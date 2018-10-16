# ---[ Google Test
if(MSVC)
  if(MSVC_VERSION LESS 1900)
    message(FATAL_ERROR "Need Visual Studio 2015 or newer to compile unit tests")
  endif()
endif()

if (UNIX)
  SET(CMAKE_EXE_LINKER_FLAGS "-pthread")
endif(UNIX)

enable_testing()
find_package(Threads REQUIRED)

file(GLOB_RECURSE UNIT_TEST_SOURCE "*.cc")
add_executable(${PROJECT_NAME}_unit_tests ${UNIT_TEST_SOURCE})
set_property(TARGET ${PROJECT_NAME}_unit_tests
  PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PRIVATE_RUNTIME_DIR})

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/build_config.h.in -> ${CMAKE_CURRENT_BINARY_DIR}/build_config.h")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/build_config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/build_config.h")
target_include_directories(${PROJECT_NAME}_unit_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(${PROJECT_NAME}_unit_tests PRIVATE -DDMLC_UNIT_TESTS_USE_CMAKE)
target_include_directories(${PROJECT_NAME}_unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(GTest)
if (NOT GTEST_FOUND)
  message(STATUS "GTest not found, downloading GTest.")
  # Download and unpack googletest at configure time
  message("${CMAKE_LOCAL}/gtest_cmake.in -> ${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt")
  configure_file("${CMAKE_LOCAL}/gtest_cmake.in" "${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt")
  execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )
  execute_process(COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )
  # Prevent GoogleTest from overriding our compiler/linker options
  # when building with Visual Studio
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Add googletest directly to our build. This adds
  # the following targets: gtest, gtest_main, gmock
  # and gmock_main
  add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"
    "${CMAKE_BINARY_DIR}/googletest-build")
  include_directories("${gtest_SOURCE_DIR}/include"
    "${gmock_SOURCE_DIR}/include")
  target_link_libraries(${PROJECT_NAME}_unit_tests
    gtest
    dmlc
    Threads::Threads)
else()
  include_directories(${GTEST_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME}_unit_tests
    ${GTEST_LIBRARIES}
    dmlc
    Threads::Threads)
endif()

add_test(AllTestsIn${PROJECT_NAME}UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}_unit_tests)
