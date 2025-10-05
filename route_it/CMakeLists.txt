# 收集源文件（只包含需要编译的文件）
file(GLOB_RECURSE router_core_src_c ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE router_core_src_cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# 创建库（只包含源文件，不包含头文件）
add_library(rti_framework 
    STATIC 
    ${router_core_src_c}
    ${router_core_src_cpp}
)

# 设置包含目录
target_include_directories(rti_framework
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
        $<INSTALL_INTERFACE:inc>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)