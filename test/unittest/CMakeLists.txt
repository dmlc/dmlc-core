# ---[ Google Test
if (UNIX)
    SET(CMAKE_EXE_LINKER_FLAGS "-pthread")
endif(UNIX)
enable_testing()
find_package(Threads REQUIRED)
file(GLOB_RECURSE UNIT_TEST_SOURCE "*.cc")
add_executable(${PROJECT_NAME}_unit_tests ${UNIT_TEST_SOURCE})
set_property(TARGET ${PROJECT_NAME}_unit_tests
        PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PRIVATE_RUNTIME_DIR})

# File downloads
message(STATUS "Downloading http://data.mxnet.io/data/cifar10/cifar10_val.rec...") 
file(DOWNLOAD "http://data.mxnet.io/data/cifar10/cifar10_val.rec"
     ${CMAKE_CURRENT_SOURCE_DIR}/cifar10_val.rec
     INACTIVITY_TIMEOUT 30
     TIMEOUT 300
     SHOW_PROGRESS
     EXPECTED_HASH SHA512=48c532f7b6a50c41b3117a2b2e6311a9061d05326757f03da5a7df191764b3a79f17afed8b5a76aee300ffca65519fb4dd45382898a3d1c83ddd8718bc6ca566)

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/build_config.h.in -> ${CMAKE_CURRENT_SOURCE_DIR}/build_config.h")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/build_config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/build_config.h")

target_link_libraries(${PROJECT_NAME}_unit_tests
        gtest
        dmlc
        Threads::Threads
        )
target_compile_definitions(${PROJECT_NAME}_unit_tests PRIVATE -DDMLC_UNIT_TESTS_USE_CMAKE)
target_include_directories(${PROJECT_NAME}_unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(AllTestsIn${PROJECT_NAME}UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}_unit_tests)
