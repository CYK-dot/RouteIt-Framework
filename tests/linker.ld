ENTRY(_start)                     /* 程序入口点 */

/* 定义内存区域 */
MEMORY
{
    RAM (rwx) : ORIGIN = 0x400000, LENGTH = 256M  /* 可自定义起始地址和大小 */
}

/* 符号定义 */
__stack_size = 1024 * 1024;       /* 栈大小 1MB */

SECTIONS
{
    /* 代码段 */
    .text : {
        *(.text .text.*)          /* 所有代码段 */
        KEEP(*(.init))            /* 保持初始化代码 */
        KEEP(*(.fini))            /* 保持终止代码 */
    } > RAM

    /* 只读数据段 */
    .rodata : {
        *(.rodata .rodata.*)      /* 只读数据 */
    } > RAM

    /* 数据段 (已初始化) */
    .data : {
        *(.data .data.*)          /* 数据段 */
    } > RAM

    /* 未初始化数据段 */
    .bss : {
        __bss_start = .;
        *(.bss .bss.*)            /* BSS段 */
        *(COMMON)                 /* 通用符号 */
        __bss_end = .;
    } > RAM

    /* 栈段 */
    .stack : {
        . = ALIGN(16);
        __stack_start = .;
        . = . + __stack_size;
        . = ALIGN(16);
        __stack_end = .;
    } > RAM

    /* 其他段 */
    .dynamic : { *(.dynamic) } > RAM
    .got : { *(.got) } > RAM
    .got.plt : { *(.got.plt) } > RAM

    /* 调试信息 (可选) */
    .debug : {
        *(.debug*)
    } > RAM

    /* 丢弃不需要的段 */
    /DISCARD/ : {
        *(.comment)
        *(.note.gnu.build-id)
        *(.eh_frame)
    }

    /* 全局符号定义 */
    __end = .;
}
INCLUDE "rti_sections.ld"