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
target_link_libraries(${PROJECT_NAME}_unit_tests
        gtest
        dmlc
        Threads::Threads
        )
add_test(AllTestsIn${PROJECT_NAME}UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}_unit_tests)
