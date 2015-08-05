// Copyright by Contributors
extern "C" {
#include <errno.h>
#include <curl/curl.h>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
}
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <iostream>
#include <stdio.h>

#include "./azure_filesys.h"

namespace dmlc {
	namespace io {
		namespace azure {
			/*!
			 * \brief list the objects in the container with prefix specified by path.name
			 * \param path the path to query
			 * \param azure_storage_account storage account of azure
			 * \param azure_access_key access key of azure
			 * \paam out_list stores the output results
			 */
			void ListObjects(const URI &path,
							 const std::string azure_storage_account,
							 const std::string azure_access_key,
							 std::vector<FileInfo> *out_list) {

			}
		}  // namespace azure

	AzureFileSystem::AzureFileSystem() {
	  const char *storageaccount = getenv("AZURE_STORAGE_ACCOUNT");
	  const char *accesskey = getenv("AZURE_ACCESS_KEY");
	  if (storageaccount == NULL) {
		LOG(FATAL) << "Need to set environment variable AZURE_STORAGE_ACCOUNT to use Azure";
	  }
	  if (accesskey == NULL) {
		LOG(FATAL) << "Need to set environment variable AZURE_ACCESS_KEY to use Azure";
	  }
	  azure_storage_account_name_ = storageaccount;
	  azure_access_key_ = accesskey;
	}

	bool AzureFileSystem::TryGetPathInfo(const URI &path_, FileInfo *out_info) {
	  URI path = path_;
	  while (path.name.length() > 1 &&
			 *path.name.rbegin() == '/') {
		path.name.resize(path.name.length() - 1);
	  }
	  std::vector<FileInfo> files;
	  azure::ListObjects(path,  azure_storage_account_name_, azure_access_key_, &files);
	  std::string pdir = path.name + '/';
	  for (size_t i = 0; i < files.size(); ++i) {
		if (files[i].path.name == path.name) {
		  *out_info = files[i]; return true;
		}
		if (files[i].path.name == pdir) {
		  *out_info = files[i]; return true;
		}
	  }
	  return false;
	}

	FileInfo AzureFileSystem::GetPathInfo(const URI &path) {
	  CHECK(path.protocol == "azure://")
		  << " AzureFileSystem.ListDirectory";
	  FileInfo info;
	  CHECK(TryGetPathInfo(path, &info))
		  << "AzureFileSystem.GetPathInfo cannot find information about " + path.str();
	  return info;
	}
	void AzureFileSystem::ListDirectory(const URI &path, std::vector<FileInfo> *out_list) {
	  CHECK(path.protocol == "azure://")
		  << " AzureFileSystem.ListDirectory";
	  if (path.name[path.name.length() - 1] == '/') {
		azure::ListObjects(path, azure_storage_account_name_,
						azure_access_key_, out_list);
		return;
	  }
	  std::vector<FileInfo> files;
	  std::string pdir = path.name + '/';
	  out_list->clear();
	  azure::ListObjects(path, azure_storage_account_name_,
					  azure_access_key_, &files);
	  for (size_t i = 0; i < files.size(); ++i) {
		if (files[i].path.name == path.name) {
		  CHECK(files[i].type == kFile);
		  out_list->push_back(files[i]);
		  return;
		}
		if (files[i].path.name == pdir) {
		  CHECK(files[i].type == kDirectory);
		  azure::ListObjects(files[i].path, azure_storage_account_name_,
						  azure_access_key_, out_list);
		  return;
		}
	  }
	}

	Stream *AzureFileSystem::Open(const URI &path, const char* const flag, bool allow_null) {
		return NULL;
	}

	SeekStream *AzureFileSystem::OpenForRead(const URI &path, bool allow_null) {
		return NULL;
	}
}  // namespace io
}  // namespace dmlc
