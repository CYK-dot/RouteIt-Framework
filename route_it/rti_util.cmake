set(RTI_CMAKE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

function(check_python_environment)
    # 查找 Python（指定最低版本）
    find_package(Python 3.6 REQUIRED COMPONENTS Interpreter)
    
    if(NOT Python_FOUND)
        message(FATAL_ERROR "Python 3.6+ is required but not found")
    endif()
    
    message(STATUS "Python executable: ${Python_EXECUTABLE}")
    message(STATUS "Python version: ${Python_VERSION}")
    
    # 检查 Jinja2 并获取版本
    execute_process(
        COMMAND ${Python_EXECUTABLE} -c "
try:
    import jinja2
    print(jinja2.__version__)
except ImportError:
    print('NOT_FOUND')
except Exception as e:
    print(f'ERROR: {e}')
"
        OUTPUT_VARIABLE jinja2_output
        RESULT_VARIABLE jinja2_result
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    if(jinja2_result EQUAL 0 AND NOT jinja2_output STREQUAL "NOT_FOUND" AND NOT jinja2_output MATCHES "ERROR")
        message(STATUS "Jinja2 found: ${jinja2_output}")
        set(JINJA2_FOUND TRUE PARENT_SCOPE)
        set(JINJA2_VERSION ${jinja2_output} PARENT_SCOPE)
    else()
        message(WARNING "Jinja2 is not installed")
        set(JINJA2_FOUND FALSE PARENT_SCOPE)
    endif()
    
    set(PYTHON_FOUND ${Python_FOUND} PARENT_SCOPE)
    set(PYTHON_EXECUTABLE ${Python_EXECUTABLE} PARENT_SCOPE)
endfunction()

function(rti_add_library_dependency target_name)
    target_link_libraries(${target_name} PRIVATE
        rti_framework
    )
endfunction()

function(rti_add_exec_dependency target_name is_isolate_script)
    # 添加链接库依赖
    target_link_libraries(${target_name} PRIVATE
        rti_framework
    )
    target_link_directories(${target_name}
        PRIVATE ${RTI_CMAKE_ROOT_DIR}/ld
    )
    # 添加链接脚本依赖
    string(TOUPPER "${is_isolate_script}" _isolate_upper)
    if(_isolate_upper STREQUAL "YES")
        if(WIN32)
            target_link_options(${target_name} PRIVATE
                -Wl,--subsystem,console
                -Wl,--file-alignment,4096
                -Wl,--section-alignment,4096
            )
            target_link_options(${target_name}
                PRIVATE -Wl,-T${RTI_CMAKE_ROOT_DIR}/ld/rti_isolate_linker_windows.ld
            )
        else()
            target_link_options(${target_name}
                PRIVATE -Wl,-T${RTI_CMAKE_ROOT_DIR}/ld/rti_isolate_linker_gcc.ld
            )
        endif()
    endif()
endfunction()