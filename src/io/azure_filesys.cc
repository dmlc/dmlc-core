/*!
 *  Copyright (c) 2015 by Contributors
 * \file azure_filesys.cc
 * \brief Azure access module
 * \author Kamran Munshi
 */
#include "./azure_filesys.h"
#include "stdafx.h"

#include "was/storage_account.h"
#include "was/blob.h"
#include "cpprest/filestream.h"
#include "cpprest/containerstream.h"

#include <dmlc/io.h>

namespace dmlc {
namespace io {
/*! \brief implementation of Azure(File) i/o stream */
class AzureFileStream : public SeekStream {
 public:
  explicit AzureFileStream(FILE *fp, bool use_stdio)
      : fp_(fp),
        use_stdio_(use_stdio) {
  }
  virtual ~AzureFileStream(void) {
    this->Close();
  }
  virtual size_t Read(void *ptr, size_t size) {
    return std::fread(ptr, 1, size, fp_);
  }
  virtual void Write(const void *ptr, size_t size) {
    LOG(FATAL)<< "Azure.FileStream cannot be used for write";
  }
  virtual void Seek(size_t pos) {
    std::fseek(fp_, static_cast<long>(pos), SEEK_SET);// NOLINT(*)
  }
  virtual size_t Tell(void) {
    return std::ftell(fp_);
  }
  virtual bool AtEnd(void) const {
    return std::feof(fp_) != 0;
  }
  inline void Close(void) {
    if (fp_ != NULL && !use_stdio_) {
      std::fclose(fp_); fp_ = NULL;
    }
  }

private:
  std::FILE *fp_;
  bool use_stdio_;
};

std::vector<std::string> split(std::string str, char delimiter) {
  std::vector < std::string > internal;
  std::stringstream ss(str);
  std::string tok;

  while (std::getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }
  return internal;
}

AzureFileSystem::AzureFileSystem() {
  const char *name = getenv("AZURE_STORAGE_ACCOUNT");
  const char* key = getenv("AZURE_STORAGE_ACCESS_KEY");
  CHECK_NE(name, NULL)
      << "Need to set enviroment variable AZURE_STORAGE_ACCOUNT to use Azure";
  CHECK_NE(key, NULL)
      << "Need to set enviroment variable AZURE_STORAGE_ACCESS_KEY to use Azure";
  azure_account_ = name;
  azure_key_ = key;
}

void AzureFileSystem::ListDirectory(const URI &path,
                                    std::vector<FileInfo> *out_list) {
  CHECK(path.host.length()) << "container name not specified in azure";
  out_list->clear();
  utility::string_t storage_connection_string(
      U("DefaultEndpointsProtocol=https;AccountName=" + azure_account_
          + ";AccountKey=" + azure_key_));
  std::cout << storage_connection_string;

  // Retrieve storage account from connection string.
  azure::storage::cloud_storage_account storage_account =
      azure::storage::cloud_storage_account::parse(storage_connection_string);

  // Create the blob client.
  azure::storage::cloud_blob_client blob_client = storage_account
      .create_cloud_blob_client();

  // Retrieve a reference to a previously created container.
  azure::storage::cloud_blob_container container = blob_client
      .get_container_reference(U(path.host));

  // Output URI of each item.
  azure::storage::list_blob_item_iterator end_of_results;
  for (auto it = container.list_blobs(); it != end_of_results; ++it) {
    if (it->is_blob()) {
      FileInfo info;
      info.path = path;
      size_t value = it->as_blob().properties().size();
      info.size = static_cast<size_t>(value);
      std::vector < std::string > splitVec = split(
          it->as_blob().uri().primary_uri().to_string(), '/');
      info.path.name = '/' + splitVec[splitVec.size() - 1];
      info.type = kFile;
      out_list->push_back(info);
    } else {
      FileInfo info;
      info.path = path;
      info.size = 0;
      std::vector < std::string > splitVec = split(
          it->as_directory().uri().primary_uri().to_string(), '/');
      info.path.name = '/' + splitVec[splitVec.size() - 1];
      info.type = kDirectory;
      out_list->push_back(info);
    }
  }
}

bool AzureFileSystem::TryGetPathInfo(const URI &path_, FileInfo *out_info) {
  URI path = path_;
  while (path.name.length() > 1 && *path.name.rbegin() == '/') {
    path.name.resize(path.name.length() - 1);
  }
  std::vector<FileInfo> files;
  ListDirectory(path, &files);
  std::string pdir = path.name + '/';
  for (size_t i = 0; i < files.size(); ++i) {
    if (files[i].path.name == path.name) {
      *out_info = files[i];
      return true;
    }
    if (files[i].path.name == pdir) {
      *out_info = files[i];
      return true;
    }
  }
  return false;
}

class AzureWriteStream : public Stream {
 public:
  AzureWriteStream(const URI &path, std::string azureaccount,
                   std::string azurekey)
      : path_(path),
        azure_account_(azureaccount),
        azure_key_(azurekey) {
    this->Init();
  }
  virtual size_t Read(void *ptr, size_t size) {
    LOG(FATAL)<< "Azure.WriteStream cannot be used for read";
    return 0;
  }
  virtual void Write(const void *ptr, size_t size) {
    size_t rlen = buffer_.length();
    buffer_.resize(rlen + size);
    std::memcpy(BeginPtr(buffer_) + rlen, ptr, size);
    this->AppendToFile();
  }
  // destructor (only upload on destructor)
  virtual ~AzureWriteStream() {
    fileBuffer_.close();
    buffer_.clear();
    this->Upload();
  }

private:
  // write data buffer
  std::string buffer_;

  // file buffer
  std::ofstream fileBuffer_;

  // path we are writing to
  URI path_;

  // azure credentials
  std::string azure_account_, azure_key_;

  // handle to the block blob
  azure::storage::cloud_block_blob blockBlob_;

  /*!
   * \brief initialize block blob client
   */
  void Init(void) {
    utility::string_t storage_connection_string(
        U("DefaultEndpointsProtocol=https;AccountName=" + azure_account_
            + ";AccountKey=" + azure_key_));

    // Retrieve storage account from connection string.
    azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storage_connection_string);

    // Create the blob client.
    azure::storage::cloud_blob_client blob_client = storage_account.create_cloud_blob_client();

    // Retrieve a reference to a previously created container.
    azure::storage::cloud_blob_container container = blob_client.get_container_reference(U(path_.host));
    azure::storage::cloud_block_blob blockBlob = container.get_block_blob_reference(U(path_.name));
    blockBlob_ = blockBlob;
    fileBuffer_.open("temp");
  }

  /*!
   * \brief append string buffer to file buffer
   */
  void AppendToFile(void) {
    fileBuffer_ << buffer_;
  }

  /*!
   * \brief upload file to Azure
   */
  void Upload(void) {
    std::string file = "temp";
    blockBlob_.upload_from_file(file);
  }
};

SeekStream *AzureFileSystem::OpenForRead(const URI &path, bool allow_null) {
  utility::string_t storage_connection_string(
      U("DefaultEndpointsProtocol=https;AccountName=" + azure_account_
          + ";AccountKey=" + azure_key_));

  // Retrieve storage account from connection string.
  azure::storage::cloud_storage_account storage_account =
      azure::storage::cloud_storage_account::parse(storage_connection_string);

  // Create the blob client.
  azure::storage::cloud_blob_client blob_client = storage_account
      .create_cloud_blob_client();

  // Retrieve a reference to a previously created container.
  azure::storage::cloud_blob_container container = blob_client
      .get_container_reference(U(path.host));
  azure::storage::cloud_block_blob blockBlob = container
      .get_block_blob_reference(U(path.name));

  // Download file to out.txt
  concurrency::streams::container_buffer < std::vector < uint8_t >> buffer;
  concurrency::streams::ostream output_stream(buffer);
  blockBlob.download_to_stream(output_stream);
  std::ofstream outfile("out.txt", std::ofstream::binary);
  std::vector<unsigned char>& data = buffer.collection();
  outfile.write((char *) &data[0], buffer.size());
  outfile.close();

  FILE *fp = NULL;
  std::string fn = "out.txt";
  std::string flag = "rb";
  using namespace std;
  const char *fname = fn.c_str();
  fp = fopen64(fname, flag.c_str());
  if (fp != NULL) {
    return new AzureFileStream(fp, false);
  } else {
    return NULL;
  }
}

Stream *AzureFileSystem::Open(const URI &path, const char* const mode,
                              bool allow_null) {
  std::string flag = mode;
  if (flag == "r") {
    return OpenForRead(path, allow_null);
  } else {
    return new AzureWriteStream(path, azure_account_, azure_key_);
  }
}

FileInfo AzureFileSystem::GetPathInfo(const URI &path) {
  CHECK(path.protocol == "azure://") << " AzureFileSystem.ListDirectory";
  FileInfo info;
  CHECK(TryGetPathInfo(path, &info))
      << "AzureFileSystem.GetPathInfo cannot find information about "
          + path.str();
  return info;
}

}  // namespace io
}  // namespace dmlc
