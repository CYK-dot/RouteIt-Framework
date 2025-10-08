# route_it提供的cmake工具
include(rti_util.cmake)

# 检查环境
check_python_environment()

# 预生成脚本
add_custom_target(rti_pre_build ALL
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/tools/rti_script_collector.py
        -c ${CMAKE_CURRENT_SOURCE_DIR}/rti_global_config.json
        -o ${CMAKE_BINARY_DIR}/rti_all_config.json
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/tools/rti_script_vlanid.py
        -c ${CMAKE_BINARY_DIR}/rti_all_config.json
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Collecting RTI module configurations..."
    VERBATIM
)

# 收集源文件
file(GLOB_RECURSE router_core_src_c ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE router_core_src_cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# 创建库
add_library(rti_framework 
    STATIC 
    ${router_core_src_c}
    ${router_core_src_cpp}
)
add_dependencies(rti_framework rti_pre_build)

# 设置包含目录
target_include_directories(rti_framework
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
        $<INSTALL_INTERFACE:inc>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)


