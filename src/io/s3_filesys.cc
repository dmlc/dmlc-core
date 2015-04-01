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
#include "./s3_filesys.h"

namespace dmlc {
namespace io {
/*! \brief namespace for helper utils */
namespace s3 {
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
/*!
 * \brief sign given AWS secret key
 * \param secret_key the key to compute the sign
 * \param content the content to sign
 */
std::string Sign(const std::string &key, const std::string &content) {
  HMAC_CTX ctx;
  unsigned char md[EVP_MAX_MD_SIZE];
  unsigned int rlen = 0;
  HMAC_CTX_init(&ctx);
  HMAC_Init(&ctx, key.c_str(), key.length(), EVP_sha1());
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
// sign AWS key
std::string Sign(const std::string &key,
                 const std::string &method,
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
  return Sign(key, stream.str());
}
/*!
 * \brief get the datestring needed by AWS
 * \return datestring
 */
inline std::string GetDateString(void) {
  time_t t = time(NULL);
  tm gmt;
  gmtime_r(&t, &gmt);
  char buf[256];
  strftime(buf, 256, "%a, %d %b %Y %H:%M:%S GMT", &gmt);
  return std::string(buf);
}
// curl callback to write sstream
size_t WriteSStreamCallback(char *buf, size_t size, size_t count, void *fp) {
  static_cast<std::ostringstream*>(fp)->write(buf, size * count);
  return size * count;
}
/*!
 * \brief list the objects in the bucket with prefix specified by path.name
 * \param path the path to query
 * \param aws_id access id of aws
 * \param aws_key access key of aws
 * \paam out_list stores the output results
 */
void ListObjects(const URI &path,
                 const std::string aws_id,
                 const std::string aws_key,
                 std::vector<FileInfo> *out_list) {
  CHECK(path.host.length() != 0) << "bucket name not specified in s3";
  out_list->clear();
  std::vector<std::string> amz;
  std::string date = GetDateString();
  std::string signature = Sign(aws_key, "GET", "", "", date, amz,
                               std::string("/") + path.host + "/");    
  
  std::stringstream sauth, sdate, surl;
  std::stringstream result;
  sauth << "Authorization: AWS " << aws_id << ":" << signature;
  sdate << "Date: " << date;
  surl << "http://" << path.host << ".s3.amazonaws.com"
       << "/?delimiter=/&prefix=" << path.name;
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
      FileInfo info;
      info.path = path;
      XMLIter value;
      CHECK(data.GetNext("Key", &value));
      info.path.name = value.str();
      CHECK(data.GetNext("Size", &value));
      info.size = static_cast<size_t>(atol(value.str().c_str()));
      info.type = kFile;
      out_list->push_back(info);
    }
  }
  {// get directories
    XMLIter xml(ret.c_str());
    XMLIter data;
    while (xml.GetNext("CommonPrefixes", &data)) {
      FileInfo info;
      info.path = path;
      XMLIter value;
      CHECK(data.GetNext("Prefix", &value));
      info.path.name = value.str();
      info.size = 0; info.type = kDirectory;
      out_list->push_back(info);
    }
  }
}
}  // namespace s3

S3FileSystem::S3FileSystem() {
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

FileInfo S3FileSystem::GetPathInfo(const URI &path) {
  std::vector<FileInfo> files;
  s3::ListObjects(path,  aws_access_id_, aws_secret_key_, &files);
  std::string pdir = path.name + '/'; 
  for (size_t i = 0; i < files.size(); ++i) {
    if (files[i].path.name == path.name) return files[i];
    if (files[i].path.name == pdir) return files[i];
  }
  Error("S3FileSytem.GetPathInfo cannot find information about" + path.str());
  return files[0];
}

void S3FileSystem::ListDirectory(const URI &path, std::vector<FileInfo> *out_list) {
  if (path.name[path.name.length() - 1] == '/') {
    s3::ListObjects(path, aws_access_id_,
                    aws_secret_key_, out_list);
    return;
  }
  std::vector<FileInfo> files;
  std::string pdir = path.name + '/';
  out_list->clear();
  s3::ListObjects(path, aws_access_id_,
                  aws_secret_key_, &files);
  for (size_t i = 0; i < files.size(); ++i) {
    if (files[i].path.name == path.name) {
      CHECK(files[i].type == kFile);
      out_list->push_back(files[i]);
      return;
    }
    if (files[i].path.name == pdir) {
      CHECK(files[i].type == kDirectory);       
      s3::ListObjects(files[i].path, aws_access_id_,
                      aws_secret_key_, out_list);
      return;
    }
  }  
}
}  // namespace io
}  // namespace dmlc
