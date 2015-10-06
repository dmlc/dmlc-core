cmake_minimum_required(VERSION 2.8.7)

project(dmlccore C CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

include(cmake/Utils.cmake)
include(cmake/dmlccore.cmake)

include_directories("include")

if(MSVC)
 add_definitions(-DDMLC_USE_CXX11)
endif()


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
