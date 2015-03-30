/*!
 *  Copyright (c) 2015 by Contributors
 * \file aws_s3-inl.h
 * \brief S3 I/O code with libcurl and libssl
 */
#ifndef DMLC_IO_AWS_S3_INL_H_
#define DMLC_IO_AWS_S3_INL_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <dmlc/io.h>
#include <dmlc/logging.h>

namespace dmlc {
namespace io {
/*! \brief file system class that handles the operations in s3 */
class AWSS3Handle {  
 public:
  /*! \brief information about object in s3 */
  struct ObjectInfo {
    /*! \brief key to the object */
    std::string key;
    /*! \brief size of the object */    
    size_t size;
  };
  explicit AWSS3Handle(const std::string &aws_access_id,
                       const std::string &aws_secret_key)
      : aws_access_id_(aws_access_id),
        aws_secret_key_(aws_secret_key) {    
  }
  /*!
   * \brief list the objects in the bucket
   * \param uri the uri to the bucket
   * \paam out_list stores the output results
   */
  inline void ListObjects(const char *uri, std::vector<ObjectInfo> *out_list) {
    Path path(uri);
    std::vector<std::string> amz;
    std::string date = GetDateString();
    std::string signature = Sign("GET", "", "", date, amz,
                                 std::string("/") + path.bucket + "/");    
    
    std::stringstream sauth, sdate, surl;
    std::stringstream result;
    sauth << "Authorization: AWS " << aws_access_id_ << ":" << signature;
    sdate << "Date: " << date;
    surl << "http://" << path.bucket << ".s3.amazonaws.com";
    // make request
    CURL *curl = curl_easy_init();
    curl_slist *list = NULL;
    list = curl_slist_append(list, sdate.str().c_str());
    list = curl_slist_append(list, sauth.str().c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_URL, surl.str().c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteSStreamCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    curl_easy_perform(curl);  
    curl_slist_free_all(list);
    curl_easy_cleanup(curl);
    // parse xml
    std::string ret = result.str();
    if (ret.find("<Error>") != std::string::npos) {
      Error(ret);
    }
    XMLIter xml(ret.c_str());
    XMLIter data;
    CHECK(xml.GetNext("IsTruncated", &data)) << "missing IsTruncated";
    CHECK(data.str() == "false") << "the returning list is truncated";    
    while (xml.GetNext("Contents", &data)) {
      ObjectInfo info;
      XMLIter value;
      CHECK(data.GetNext("Key", &value));
      info.key = value.str();
      CHECK(data.GetNext("Size", &value));
      info.size = static_cast<size_t>(atol(value.str().c_str()));
      out_list->push_back(info);
    }
  }

 private:
  // access id to aws
  std::string aws_access_id_;
  // access key to aws
  std::string aws_secret_key_;
  // Path in s3
  struct Path {
    // bucket name
    std::string bucket;
    // prefix or name of file in uri
    std::string prefix;
    // path to s3
    explicit Path(const char *uri) {
      CHECK(!strncmp(uri, "s3://", 5)) << "s3 path must begin with s3://";
      uri += 5;
      const char *p = strchr(uri, '/');
      if (p != NULL) {
        bucket = std::string(uri, p - uri);
        prefix = std::string(p + 1);
      } else {
        bucket = std::string(uri);        
      }
    }
  };
  // callback to write to sstream
  inline static size_t WriteSStreamCallback
  (char *buf, size_t size, size_t count, void *fp) {
    static_cast<std::ostringstream*>(fp)->write(buf, size * count);
    return size * count;
  }
  // simple XML parser
  struct XMLIter {
    // content of xml
    const char *content_;
    // end of content
    const char *cend_;
    explicit XMLIter() 
        : content_(NULL), cend_(NULL) {
    }
    // constructor
    explicit XMLIter(const char *content)
        : content_(content) {
      cend_ = content_ + strlen(content_);
    }
    /*! \brief convert to string */
    inline std::string str(void) const {
      if (content_ >= cend_) return std::string("");
      return std::string(content_, cend_ - content_);
    }
    /*!
     * \brief get next value of corresponding key in xml string
     * \param key the key in xml field
     * \param value the return value if success
     * \return if the get is success
     */
    inline bool GetNext(const char *key,                          
                        XMLIter *value) {
      std::string begin = std::string("<") + key +">";
      std::string end = std::string("</") + key +">";
      const char *pbegin = strstr(content_, begin.c_str());
      if (pbegin == NULL || pbegin > cend_) return false;
      content_ = pbegin + begin.size();
      const char *pend = strstr(content_, end.c_str());
      CHECK(pend != NULL) << "bad xml format";
      value->content_ = content_;
      value->cend_ = pend;
      content_ = pend + end.size();
      return true;
    }
  };
  inline std::string Sign(const std::string &method,
                          const std::string &content_md5,
                          const std::string &content_type,
                          const std::string &date,
                          std::vector<std::string> amz_headers,
                          const std::string &resource) {
	std::ostringstream stream;
    stream << method << "\n";
    stream << content_md5 << "\n";
    stream << content_type << "\n";
    stream << date << "\n";
    std::sort(amz_headers.begin(), amz_headers.end());
    for (size_t i = 0; i < amz_headers.size(); ++i) {
      stream << amz_headers[i] << "\n";
    }
    stream << resource;
    return Sign(stream.str());
  }
  /*!
   * \brief sign given AWS secret key
   * \param secret_key the key to compute the sign
   * \param content the content to sign
   */
  inline std::string Sign(const std::string &content) {
    HMAC_CTX ctx;
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int rlen = 0;
    HMAC_CTX_init(&ctx);
    HMAC_Init(&ctx, aws_secret_key_.c_str(),
              aws_secret_key_.length(), EVP_sha1());
    HMAC_Update(&ctx,
                reinterpret_cast<const unsigned char*>(content.c_str()),
                content.length());
    HMAC_Final(&ctx, md, &rlen);
    HMAC_CTX_cleanup(&ctx);
    // encode base64
    BIO *fp = BIO_push(BIO_new(BIO_f_base64()),
                       BIO_new(BIO_s_mem()));    
    BIO_write(fp, md, rlen);
    BIO_ctrl(fp, BIO_CTRL_FLUSH, 0, NULL);    
    BUF_MEM *res;
    BIO_get_mem_ptr(fp, &res);    
    std::string ret(res->data, res->length - 1);
    BIO_free_all(fp);
    return ret;
  }
  /*!
   * \brief get the datestring needed by AWS
   * \return datestring
   */
  inline static std::string GetDateString(void) {
    time_t t = time(NULL);
    tm gmt;
    gmtime_r(&t, &gmt);
    char buf[256];
    strftime(buf, 256, "%a, %d %b %Y %H:%M:%S GMT", &gmt);
    return std::string(buf);
  }
};

class AWSS3Stream : public IStream {
 public:
  AWSS3Stream(const char *uri, size_t begin_pos) {
  }  
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_AWS_S3_INL_H_
