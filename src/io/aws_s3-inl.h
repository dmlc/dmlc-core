/*!
 *  Copyright (c) 2015 by Contributors
 * \file aws_s3-inl.h
 * \brief S3 I/O code with libcurl and libssl
 * \author Tianqi Chen
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
#include <errno.h>
#include <curl/curl.h>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include "./line_split.h"

namespace dmlc {
namespace io {
/*! \brief file system class that handles the operations in s3 */
class S3FileSytem {  
 public:
  /*! \brief information about object in s3 */
  struct ObjectInfo {
    /*! \brief key to the object */
    std::string key;
    /*! \brief size of the object */
    size_t size;
    /*! \brief whether the it is the directory */
    bool is_dir;
  };
  /*! \brief S3 path structure */
  struct Path {
    /*! \brief bucket name */
    std::string bucket;
    /*! \brief file path */
    std::string prefix;
    // path to s3
    Path() {}
    /*!
     * \brief constructor 
     * \param uri a s3 uri
     */
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
  /*! \brief reader stream that can be used to read */
  struct ReadStream : public IStream {
   public:
    virtual ~ReadStream(void) {
      curl_multi_remove_handle(mcurl_, ecurl_);
      curl_easy_cleanup(ecurl_);
      curl_multi_cleanup(mcurl_);
    }
    virtual void Write(const void *ptr, size_t size) {
      CHECK(false) << "ReadStream cannot be used for write";
    }
    virtual size_t Read(void *ptr, size_t size) {
      size_t nleft = size;
      char *buf = reinterpret_cast<char*>(ptr);
      while (nleft != 0) {
        if (read_ptr_ == buffer_.length()) {
          read_ptr_ = 0; buffer_.clear();
          if (this->FillBuffer(nleft) == 0 && buffer_.length() == 0) {
            return size - nleft;
          }
        }
        size_t nread = std::min(nleft, buffer_.length() - read_ptr_);
        std::memcpy(buf, BeginPtr(buffer_) + read_ptr_, nread);
        buf += nread; read_ptr_ += nread; nleft -= nread;
      }
      return size;
    }

   private:
    // private constructor
    explicit ReadStream(CURL *ecurl,
                        curl_slist *slist)
        : ecurl_(ecurl), slist_(slist), read_ptr_(0) {
      mcurl_ = curl_multi_init();
      CHECK(curl_easy_setopt(ecurl, CURLOPT_WRITEFUNCTION, WriteCallback) == CURLE_OK);
      CHECK(curl_easy_setopt(ecurl, CURLOPT_WRITEDATA, &buffer_) == CURLE_OK);
      CHECK(curl_easy_setopt(ecurl, CURLOPT_HEADERFUNCTION, WriteCallback) == CURLE_OK);
      CHECK(curl_easy_setopt(ecurl, CURLOPT_HEADERDATA, &header_) == CURLE_OK);
      CHECK(curl_multi_add_handle(mcurl_, ecurl_) == CURLM_OK);
      int nrun;
      curl_multi_perform(mcurl_, &nrun);
      CHECK(nrun != 0 || header_.length() != 0 || buffer_.length() != 0);
      this->Init();
    }
    // make friend with S3FileSytem
    friend class S3FileSytem;
    // multi and easy curl handle
    CURL *mcurl_, *ecurl_;
    // slist needed by the program
    curl_slist *slist_;
    // data buffer
    std::string buffer_;
    // header buffer
    std::string header_;
    // data pointer to read position
    size_t read_ptr_;
    // ask handle to read data
    // return number of remainning actions
    inline int FillBuffer(size_t nwant) {
      int nrun = 0;
      while (buffer_.length() < nwant) {
        // wait for the event of read ready
        fd_set fdread;
        fd_set fdwrite;
        fd_set fdexcep;
        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);
        int maxfd = -1;
        timeval timeout;
        timeout.tv_sec = 60;
        timeout.tv_usec = 0;
        long curl_timeo;
        curl_multi_timeout(mcurl_, &curl_timeo);
        if (curl_timeo >= 0) {
          timeout.tv_sec = curl_timeo / 1000;
          if(timeout.tv_sec > 1) {
            timeout.tv_sec = 1;
          } else {
            timeout.tv_usec = (curl_timeo % 1000) * 1000;
          }
        }
        CHECK(curl_multi_fdset(mcurl_, &fdread, &fdwrite, &fdexcep, &maxfd) == CURLM_OK);
        int rc;
        if (maxfd == -1) {
#ifdef _WIN32
          Sleep(100);
          rc = 0;
#else
          struct timeval wait = { 0, 100 * 1000 };
          rc = select(0, NULL, NULL, NULL, &wait);
#endif
      } else {
          rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
        }
        if (rc != -1) {
          CURLMcode ret = curl_multi_perform(mcurl_, &nrun);
          if (ret ==  CURLM_CALL_MULTI_PERFORM) continue;
          CHECK(ret == CURLM_OK);
          if (nrun == 0) return 0;
        }
      }
      return nrun;
    }
    // initialize the connection
    inline void Init(void) {
      this->FillBuffer(1);
      int code;
      if (sscanf(header_.c_str(), "%*s%d", &code) != 1 ||
          (code != 206 && code != 204)) {
        while (this->FillBuffer(buffer_.length() + 256) != 0);
        Error("Error\n" + header_ + buffer_);
      }
    }
    // callback by curl
    inline static size_t WriteCallback(char *buf, size_t size, size_t count, void *fp) {
      size *= count;
      std::string *str = static_cast<std::string*>(fp);
      size_t len = str->length();
      str->resize(len + size);
      std::memcpy(BeginPtr(*str) + len, buf, size);
      return size;
    }
  };
  explicit S3FileSytem(const std::string &aws_access_id,
                       const std::string &aws_secret_key)
      : aws_access_id_(aws_access_id),
        aws_secret_key_(aws_secret_key) {
  }
  explicit S3FileSytem(void) {
    const char *keyid = getenv("AWS_ACCESS_KEY_ID");
    const char *seckey = getenv("AWS_SECRET_ACCESS_KEY");
    if (keyid == NULL) {
      Error("Need to set enviroment variable AWS_ACCESS_KEY_ID to use S3");
    }
    if (seckey == NULL) {
      Error("Need to set enviroment variable AWS_SECRET_ACCESS_KEY to use S3");
    }
    aws_access_id_ = keyid;
    aws_secret_key_ = seckey;
  }
  /*!
   * \brief list the objects in the bucket
   * \param uri the uri to the bucket
   * \paam out_list stores the output results
   */
  inline void ListObjects(const Path &path, std::vector<ObjectInfo> *out_list) {
    out_list->clear();
    std::vector<std::string> amz;
    std::string date = GetDateString();
    std::string signature = Sign("GET", "", "", date, amz,
                                 std::string("/") + path.bucket + "/");    
    
    std::stringstream sauth, sdate, surl;
    std::stringstream result;
    sauth << "Authorization: AWS " << aws_access_id_ << ":" << signature;
    sdate << "Date: " << date;
    surl << "http://" << path.bucket << ".s3.amazonaws.com"
         << "/?delimiter=/&prefix=" << path.prefix;
    // make request
    CURL *curl = curl_easy_init();
    curl_slist *slist = NULL;
    slist = curl_slist_append(slist, sdate.str().c_str());
    slist = curl_slist_append(slist, sauth.str().c_str());
    CHECK(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist) == CURLE_OK);
    CHECK(curl_easy_setopt(curl, CURLOPT_URL, surl.str().c_str()) == CURLE_OK);
    CHECK(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L) == CURLE_OK);
    CHECK(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteSStreamCallback) == CURLE_OK);
    CHECK(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result) == CURLE_OK);
    curl_easy_perform(curl);
    curl_slist_free_all(slist);
    curl_easy_cleanup(curl);
    // parse xml
    std::string ret = result.str();
    if (ret.find("<Error>") != std::string::npos) {
      Error(ret);
    }
    {// get files
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
        info.is_dir = false;
        out_list->push_back(info);
      }
    }
    {// get directories
      XMLIter xml(ret.c_str());
      XMLIter data;
      while (xml.GetNext("CommonPrefixes", &data)) {
        ObjectInfo info;
        XMLIter value;
        CHECK(data.GetNext("Prefix", &value));
        info.key = value.str();
        info.size = 0; info.is_dir = true;
        out_list->push_back(info);
      }
    }
  }
  /*!
   * \brief list path, similar behavior to filesystem
   * \param uri the uri to the bucket
   * \paam out_list stores the output results
   */
  inline void ListPath(const Path &path, std::vector<ObjectInfo> *out_list) {
    std::vector<ObjectInfo> ret;
    if (path.prefix[path.prefix.length() - 1] == '/') {
      this->ListObjects(path, out_list); return;
    }
    std::string prefp = path.prefix + '/';
    out_list->clear();
    this->ListObjects(path, &ret);
    for (size_t i = 0; i < ret.size(); ++i) {
      if (ret[i].key == path.prefix) {
        CHECK(!ret[i].is_dir);
        out_list->push_back(ret[i]);
        return;
      }
      if (ret[i].key == prefp) {
        CHECK(ret[i].is_dir);
        Path ps = path; ps.prefix = prefp;
        this->ListObjects(ps, out_list);
        return;
      }
    }
  }
  /*!
   * \brief open a new stream to read a certain object
   * the stream is still valid even when FileSystem get destructed
   * \param path the path to the object
   * \param begin_pos the beginning position to read
   */
  IStream *OpenForRead(const Path &path, size_t begin_pos) {
    std::vector<std::string> amz;
    std::string date = GetDateString();
    std::string signature = Sign("GET", "", "", date, amz,
                                 std::string("/") + path.bucket + "/" + path.prefix);
    // generate headers
    std::stringstream sauth, sdate, surl, srange;
    std::stringstream result;
    sauth << "Authorization: AWS " << aws_access_id_ << ":" << signature;
    sdate << "Date: " << date;
    surl << "http://" << path.bucket << ".s3.amazonaws.com" << "/" << path.prefix;
    srange << "Range: bytes=" << begin_pos << "-";
    // make request
    CURL *curl = curl_easy_init();
    curl_slist *slist = NULL;
    slist = curl_slist_append(slist, sdate.str().c_str());
    slist = curl_slist_append(slist, srange.str().c_str());
    slist = curl_slist_append(slist, sauth.str().c_str());
    CHECK(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist) == CURLE_OK);
    CHECK(curl_easy_setopt(curl, CURLOPT_URL, surl.str().c_str()) == CURLE_OK);
    CHECK(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L) == CURLE_OK);
    CHECK(curl_easy_setopt(curl, CURLOPT_HEADER, 0L) == CURLE_OK);
    return new ReadStream(curl, slist);
  }

  IStream *Open(const Path &path, const char * const flag) {
    if (!strcmp(flag, "r") || !strcmp(flag, "rb")) {
      return OpenForRead(path, 0);
    } else {
      CHECK(false) << "S3 flag " << flag << " was  not supported";
    }
    return NULL;
  }
 private:
  // access id to aws
  std::string aws_access_id_;
  // access key to aws
  std::string aws_secret_key_;
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

/*! \brief line split from normal file system */
class S3Provider : public LineSplitter::IFileProvider {
 public:
  explicit S3Provider(const char *uri) {
    std::vector<std::string> paths;
    LineSplitter::SplitNames(&paths, uri, "#");
    // get the files
    for (size_t  i = 0; i < paths.size(); ++i) {
      std::vector<S3FileSytem::ObjectInfo> files;
      S3FileSytem::Path ps(paths[i].c_str());
      fs_.ListPath(ps, &files);      
      for (size_t i = 0; i < files.size(); ++i) {
        if (!files[i].is_dir) {
          S3FileSytem::Path pp = ps;
          pp.prefix = files[i].key;
          fnames_.push_back(pp);
          fsize_.push_back(files[i].size);
        }
      }
    }
    if (fsize_.size() == 0) {
      Error("Path %s is not a file or directory", uri);
    }
  }
  virtual ~S3Provider(void) {
  }  
  virtual const std::vector<size_t> &ListFileSize(void) const {
    return fsize_;
  }
  virtual IStream *Open(size_t file_index, size_t begin_pos) {
    CHECK(file_index < fnames_.size()) << "file index exceed bound";
    return fs_.OpenForRead(fnames_[file_index], begin_pos);
  }
  
 private:
  // file system
  S3FileSytem fs_;
  // file sizes
  std::vector<size_t> fsize_;
  // file names
  std::vector<S3FileSytem::Path> fnames_;
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_AWS_S3_INL_H_
