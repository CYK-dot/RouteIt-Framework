# projects
cmake_minimum_required(VERSION 3.14)
project(RouteItFramework_Test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# gtest-framework
enable_testing()
option(BUILD_TESTS "Build tests" ON)
add_subdirectory(extern/googletest)

# rti-framework
add_subdirectory(../route_it
    rti_build
)

# testcase
add_executable(RouteItFramework_Test
    cases/test_vlanid.cpp
)
target_link_libraries(RouteItFramework_Test
    gtest
    gtest_main
    rti_framework
)
target_link_directories(RouteItFramework_Test
    PRIVATE ${CMAKE_SOURCE_DIR}/../route_it/ld
)
target_link_options(RouteItFramework_Test
    PRIVATE -T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
    PRIVATE -Wl,--verbose
)
