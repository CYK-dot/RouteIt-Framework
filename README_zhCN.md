# RouteIt-Framework
轻量级嵌入式消息路由框架，将路由决策与交付机制分离<br>
⚠️仍在开发中...

# 移植方式
## cmake工程
### 1. cmake工程配置
在你工程中的根CMakelists.txt中，将route_it文件夹加入编译<br>
```cmake
add_subdirectory(你的路径/route_it)
```
在需要使用到route_it的子CMakelists.txt中，先包含rti提供的cmake工具函数<br>
```cmake
include(你的路径/rti_functions.cmake)
```
在需要用到route_it的library附近调用工具函数rti_add_dependency，将route_it添加为依赖<br>
```cmake
rti_add_library_dependency(your_target)
```
在需要使用到route_it的executable附近调用工具函数rti_add_exec_dependency，将route_it添加为依赖<br>
```cmake
rti_add_exec_dependency(your_target, "NO")
```
如果你的项目采用的是默认链接脚本，则第二个参数填入YES。如果采用了定制全局链接脚本，则填入NO。<br>
对于填入NO的情况，除了rti_add_exec_dependency，还需额外修改全局链接脚本，新增一个.rti_vlan段<br>
以CubeMX生成的cmake工程为例，打开全局链接脚本STM32F4XX_FLASH.ld，将如下代码加入到.text的后面。
```ld
.rti_vlan :
{
    INCLUDE "rti_mbed_linker_rtivlan.ld"
} > FLASH
```
加完后长这样(局部)
```ld
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  .rti_vlan :
  {
    INCLUDE "rti_mbed_linker_rtivlan.ld"
  } > FLASH
```
### 2. rti组件配置
打开route_it/rti_global_config.json中的project_dir，将其修改为你的工程根目录<br>
- project_dir不影响cmake，只影响RouteIt的python脚本。脚本会基于该目录递归搜索rti_config.json。<br>
- 若您没有使用到rti_config.json，可将project_dir设置为当前目录"./"。<br>



