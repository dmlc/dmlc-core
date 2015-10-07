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

# ---[ Linter target
if(MSVC)
  find_package(PythonInterp 2)
  set(PYTHON2_EXECUTABLE ${PYTHON_EXECUTABLE} CACHE FILEPATH "Path to the python 2.x executable")
  find_package(PythonInterp 3)
  set(PYTHON3_EXECUTABLE ${PYTHON_EXECUTABLE} CACHE FILEPATH "Path to the python 3.x executable")
endif()
set(LINT_DIRS include src scripts)
add_custom_target(dmlccore_lint COMMAND ${CMAKE_COMMAND} -DMSVC=${MSVC} -DPYTHON2_EXECUTABLE=${PYTHON2_EXECUTABLE} -DPYTHON3_EXECUTABLE=${PYTHON3_EXECUTABLE} -DLINT_DIRS=${LINT_DIRS} -P ${PROJECT_SOURCE_DIR}/cmake/lint.cmake)
