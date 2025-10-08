# projects
cmake_minimum_required(VERSION 3.14)
project(RouteItFramework_Test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

include(../route_it/rti_util.cmake)

# functions
function(create_split_exec target_name source_file definitions)
    message(STATUS "Creating target ${target_name} with definitions: ${definitions}")

    add_executable(${target_name} ${source_file})
    target_compile_definitions(${target_name} PRIVATE ${definitions})
    target_compile_options(${target_name} PRIVATE
        -O0
        -ggdb3
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
        -fno-inline
        -fno-elide-constructors
    )
    target_link_libraries(${target_name} PRIVATE
        gtest
        gtest_main
    )
    rti_add_exec_dependency(${target_name} "YES")
endfunction()

# gtest-framework
enable_testing()
option(BUILD_TESTS "Build tests" ON)
add_subdirectory(extern/googletest)

# rti-framework
add_subdirectory(../route_it
    rti_build
)
if(TARGET rti_framework)
    target_compile_options(rti_framework PRIVATE -ggdb3 -O0)
endif()

# testcase files
file(GLOB_RECURSE testcases_cpp ${CMAKE_CURRENT_SOURCE_DIR}/cases/*.cpp)

# testcase executable
create_split_exec(RouteItFramework_Test
    "${testcases_cpp}"
    "RTI_TEST_COMMON"
)
create_split_exec(RouteItFramework_TestVlanEmpty
    "${testcases_cpp}"
    "RTI_TEST_VLAN_TABLE_EMPTY"
)
create_split_exec(RouteItFramework_TestVlanCommon
    "${testcases_cpp}"
    "RTI_TEST_VLAN_TABLE_COMMON"
)
create_split_exec(RouteItFramework_TestVlanGenerate
    "${testcases_cpp}"
    "RTI_TEST_VLAN_TABLE_GENERATE"
)