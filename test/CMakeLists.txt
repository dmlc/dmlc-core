# ---[ Google Test
if(MSVC)
  if(MSVC_VERSION LESS 1900)
    message(FATAL_ERROR "Need Visual Studio 2015 or newer to compile unit tests")
  endif()
endif()

if (UNIX)
  SET(CMAKE_EXE_LINKER_FLAGS "-pthread")
endif(UNIX)

# Compiler definitions needed to use GNU/POSIX extensions
set(ENABLE_GNU_EXTENSION_FLAGS -D_XOPEN_SOURCE=700
  -D_POSIX_SOURCE -D_POSIX_C_SOURCE=200809L -D_DARWIN_C_SOURCE)

enable_testing()
find_package(Threads REQUIRED)

file(GLOB_RECURSE UNIT_TEST_SOURCE "*.cc")
add_executable(dmlc_unit_tests ${UNIT_TEST_SOURCE})
if(MSVC)
  set_target_properties(dmlc_unit_tests PROPERTIES
    MSVC_RUNTIME_LIBRARY "${DMLC_MSVC_RUNTIME_LIBRARY}")
endif()

target_compile_definitions(dmlc_unit_tests PRIVATE -DDMLC_UNIT_TESTS_USE_CMAKE -DDMLC_CORE_USE_CMAKE ${ENABLE_GNU_EXTENSION_FLAGS})
target_include_directories(dmlc_unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(GTest 1.14.0)
if (NOT GTEST_FOUND)
  message(STATUS "GTest not found, downloading GTest.")
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
  )
  set(gtest_force_shared_crt ${DMLC_FORCE_SHARED_CRT} CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  add_library(GTest::gtest ALIAS gtest)
  add_library(GTest::gmock ALIAS gmock)
  target_compile_definitions(gtest PRIVATE ${ENABLE_GNU_EXTENSION_FLAGS})
  target_compile_definitions(gmock PRIVATE ${ENABLE_GNU_EXTENSION_FLAGS})
  target_link_libraries(dmlc_unit_tests PRIVATE
    gtest gmock dmlc Threads::Threads)
  foreach(target gtest gmock)
    target_compile_features(${target} PUBLIC cxx_std_14)
    if(MSVC)
      set_target_properties(${target} PROPERTIES
        MSVC_RUNTIME_LIBRARY "${DMLC_MSVC_RUNTIME_LIBRARY}")
    endif()
  endforeach()
else()
  target_include_directories(dmlc_unit_tests PRIVATE ${GTEST_INCLUDE_DIRS})
  foreach(target GTest::gtest GTest::gmock)
    target_compile_features(${target} INTERFACE cxx_std_14)
  endforeach()
  target_link_libraries(dmlc_unit_tests PRIVATE
    GTest::gtest GTest::gmock dmlc Threads::Threads)
endif()

if(USE_OPENMP)
  target_link_libraries(dmlc_unit_tests PRIVATE OpenMP::OpenMP_CXX)
endif()

add_test(AllTestsInDMLCUnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dmlc_unit_tests)
