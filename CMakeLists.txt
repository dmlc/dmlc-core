cmake_minimum_required(VERSION 2.8.7)

project(dmlccore C CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

include(cmake/Utils.cmake)
#include(cmake/dmlccore.cmake)

# Options
dmlccore_option(USE_HDFS "Build with HDFS support" OFF)
dmlccore_option(USE_S3 "Build with S3 support" OFF)
dmlccore_option(USE_AZURE "Build with AZURE support" OFF)

# include path
include_directories("include")

set(dmlccore_LINKER_LIBS "")
# HDFS configurations
if(USE_HDFS)
 find_package(HDF5 COMPONENTS C CXX HL REQUIRED)
 include_directories(SYSTEM ${HDF5_INCLUDE_DIRS} ${HDF5_HL_INCLUDE_DIR})
 list(APPEND dmlccore_LINKER_LIBS ${HDF5_LIBRARIES})
 add_definitions(-DDMLC_USE_HDFS=1)
else()
 add_definitions(-DDMLC_USE_HDFS=0)
endif()
# S3 configurations
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
# Azure configurations
if(USE_AZURE)
  add_definitions(-DDMLC_USE_AZURE=1)
else()
  add_definitions(-DDMLC_USE_AZURE=0)
endif()

# compile
if(MSVC)
  add_definitions(-DDMLC_USE_CXX11)
  foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MD")
  endforeach(flag_var)
else(MSVC)
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag("-std=c++0x"    SUPPORT_CXX0X)
  check_cxx_compiler_flag("-msse2"        SUPPORT_MSSE2)
  set(CMAKE_C_FLAGS "-O3 -Wall -msse2 -Wno-unknown-pragmas -std=c++0x -fPIC")
  set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS}) 
endif(MSVC)

FILE(GLOB SOURCE "src/*.cc")
list(APPEND SOURCE "src/io/line_split.cc")
list(APPEND SOURCE "src/io/recordio_split.cc")
list(APPEND SOURCE "src/io/input_split_base.cc")
list(APPEND SOURCE "src/io/local_filesys.cc")

if(USE_HDFS)
	list(APPEND SOURCE "src/io/hdfs_filesys.cc")
endif()
if(USE_S3)
	list(APPEND SOURCE "src/io/s3_filesys.cc")
endif()
if(USE_AZURE)
	list(APPEND SOURCE "src/io/azure_filesys.cc")
endif()

add_library(dmlccore ${SOURCE}) 
target_link_libraries(dmlccore ${dmlccore_LINKER_LIBS})

# ---[ Linter target
if(MSVC)
  find_package(PythonInterp 2)
  set(PYTHON2_EXECUTABLE ${PYTHON_EXECUTABLE} CACHE FILEPATH "Path to the python 2.x executable")
  find_package(PythonInterp 3)
  set(PYTHON3_EXECUTABLE ${PYTHON_EXECUTABLE} CACHE FILEPATH "Path to the python 3.x executable")
endif()
set(LINT_DIRS include src scripts)
add_custom_target(dmlccore_lint COMMAND ${CMAKE_COMMAND} -DMSVC=${MSVC} -DPYTHON2_EXECUTABLE=${PYTHON2_EXECUTABLE} -DPYTHON3_EXECUTABLE=${PYTHON3_EXECUTABLE} -DLINT_DIRS=${LINT_DIRS} -P ${PROJECT_SOURCE_DIR}/cmake/lint.cmake)
