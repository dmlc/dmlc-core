set(dmlccore_LINKER_LIBS "")

dmlccore_option(USE_HDFS "Build with HDFS support" OFF)
dmlccore_option(USE_S3 "Build with S3 support" OFF)
dmlccore_option(USE_AZURE "Build with AZURE support" OFF)


if(USE_HDFS)
 find_package(HDF5 COMPONENTS C CXX HL REQUIRED)
 include_directories(SYSTEM ${HDF5_INCLUDE_DIRS} ${HDF5_HL_INCLUDE_DIR})
 list(APPEND dmlccore_LINKER_LIBS ${HDF5_LIBRARIES})
 add_definitions(-DDMLC_USE_HDFS=1)
else()
 add_definitions(-DDMLC_USE_HDFS=0)
endif()


if(USE_S3)
 find_package(CURL REQUIRED)
 include_directories(SYSTEM ${CURL_INCLUDE_DIR})
 list(APPEND dmlccore_LINKER_LIBS ${CURL_LIBRARY})
 
 find_package(OpenSSL REQUIRED)
 include_directories(SYSTEM ${OPENSSL_INCLUDE_DIR})
 list(APPEND dmlccore_LINKER_LIBS ${OPENSSL_LIBRARY})
 
 find_package(CRYPTO REQUIRED)
 include_directories(SYSTEM ${CRYPTO_INCLUDE_DIR})
 list(APPEND dmlccore_LINKER_LIBS ${CRYPTO_LIBRARY})
 
 add_definitions(-DDMLC_USE_S3=1)
else()
 add_definitions(-DDMLC_USE_S3=0)
endif()

if(USE_AZURE)
  add_definitions(-DDMLC_USE_AZURE=1)
else()
  add_definitions(-DDMLC_USE_AZURE=0)
endif()