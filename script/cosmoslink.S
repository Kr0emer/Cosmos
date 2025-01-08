#include "config.h"  
// 包含配置文件，可能定义了一些平台相关的宏或配置信息。

#ifdef CFG_X86_PLATFORM
// 如果定义了 CFG_X86_PLATFORM（表示当前为 x86 平台），则编译下面的代码。

#include "buildfile.h"  
// 包含 buildfile.h 文件，可能定义了构建相关的宏或配置。

LINEAR_ADDRESS = 0x2000000;  
// 定义线性地址的起始值，表示程序加载的物理地址为 0x2000000。

VIRTUAL_ADDRESS = 0xffff800000000000;  
// 定义虚拟地址的基地址，用于 x86-64 平台的虚拟内存管理。

OUTPUT_ARCH(i386:x86-64)  
// 设置目标架构为 x86-64（64 位 x86 架构）。

OUTPUT_FORMAT(elf64-x86-64)  
// 设置目标文件格式为 ELF 64 位 x86 格式。

ENTRY(_start)  
// 定义程序入口点为 `_start`，通常是启动代码的入口。

SECTIONS  
{  
    // 定义链接脚本中的段布局，用于指定程序的各部分在内存中的位置。
    
	. =LINEAR_ADDRESS;  
    // 将当前地址设置为 LINEAR_ADDRESS 的起始值（0x2000000）。
	
	__start_kernel = . + VIRTUAL_ADDRESS;  
    // 定义内核启动的虚拟地址，等于当前地址 + 虚拟地址偏移量。

    __begin_start_text = .;  
    // 定义 `.start.text` 段的起始地址。
    
    .start.text : ALIGN(4) { *(.start.text) }  
    // 定义 `.start.text` 段的对齐方式为 4 字节，存储 `.start.text` 中的内容。
    
	__end_start_text = .;  
    // 定义 `.start.text` 段的结束地址。
    
    __begin_start_data = .;  
    // 定义 `.start.data` 段的起始地址。
    
    .start.data : ALIGN(0x4) { *(.start.data) }  
    // 定义 `.start.data` 段的对齐方式为 4 字节，存储 `.start.data` 中的内容。
    
	__end_start_data = .;  
    // 定义 `.start.data` 段的结束地址。

    __begin_start_data_pml4 = .;  
    // 定义 `.start.data.pml4` 段的起始地址。
    
    .start.data.pml4 : ALIGN(0x1000) { *(.start.data.pml4) }  
    // 定义 `.start.data.pml4` 段的对齐方式为 4 KB，存储 `.start.data.pml4` 中的内容。
    
	__end_start_data_pml4 = .;  
    // 定义 `.start.data.pml4` 段的结束地址。

	. +=VIRTUAL_ADDRESS;  
    // 将当前地址加上虚拟地址偏移量，用于转换到新的虚拟内存空间。

	__begin_entry64_text = .;  
    // 定义 `.entry64.text` 段的起始地址。
    
    .entry64.text ALIGN(4) : AT(ADDR(.entry64.text)-VIRTUAL_ADDRESS) { *(.entry64.text) }  
    // 定义 `.entry64.text` 段的对齐方式为 4 字节，存储 `.entry64.text` 中的内容，物理地址为虚拟地址减去偏移量。
    
	__end_entry64_text = .;  
    // 定义 `.entry64.text` 段的结束地址。

    // 类似的定义方式重复应用于以下各段：
    // .hwint.text、.sirq.text、.scall.text、.sched.text、.fs.text、.drive.text
    // 这些段分别对应硬件中断、软中断、系统调用、调度器、文件系统、驱动程序等代码段。
    __begin_hwint_text = .;
    .hwint.text ALIGN(4) : AT(ADDR(.hwint.text)-VIRTUAL_ADDRESS) { *(.hwint.text) }
    __end_hwint_text = .;

    __begin_sirq_text = .;
    .sirq.text ALIGN(4) : AT(ADDR(.sirq.text)-VIRTUAL_ADDRESS) { *(.sirq.text) }
    __end_sirq_text = .;

    __begin_scall_text = .;
    .scall.text ALIGN(4) : AT(ADDR(.scall.text)-VIRTUAL_ADDRESS) { *(.scall.text) }
    __end_scall_text = .;

    __begin_sched_text = .;
    .sched.text ALIGN(4) : AT(ADDR(.sched.text)-VIRTUAL_ADDRESS) { *(.sched.text) }
    __end_sched_text = .;

    __begin_fs_text = .;
    .fs.text ALIGN(4) : AT(ADDR(.fs.text)-VIRTUAL_ADDRESS) { *(.fs.text) }
    __end_fs_text = .;

    __begin_drive_text = .;
    .drive.text ALIGN(4) : AT(ADDR(.drive.text)-VIRTUAL_ADDRESS) { *(.drive.text) }
    __end_drive_text = .;

    __begin_text = .;
    .text ALIGN(4) : AT(ADDR(.text)-VIRTUAL_ADDRESS) { *(.text) }
    __end_text = .;

    __begin_spinlock_text = .;
    .spinlock.text ALIGN(4) : AT(ADDR(.spinlock.text)-VIRTUAL_ADDRESS) { *(.spinlock.text) }
    __end_spinlock_text = .;

    __begin_init_text = .;
    .init.text ALIGN(4) : AT(ADDR(.init.text)-VIRTUAL_ADDRESS) { *(.init.text) }
    __end_init_text = .;

    __begin_driver_entry_text = .;
    .drventry.text ALIGN(4) : AT(ADDR(.drventry.text)-VIRTUAL_ADDRESS) { *(.drventry.text) }
    __end_driver_entry_text = .;

    // 数据段和只读数据段：
    __begin_data = .;
    .data ALIGN(4) : AT(ADDR(.data)-VIRTUAL_ADDRESS) { *(.data) *(.bss) }
    __end_data = .;

    __begin_rodata = .;
    .rodata ALIGN(4) : AT(ADDR(.rodata)-VIRTUAL_ADDRESS) { *(.rodata) *(.rodata.*) }
    __end_rodata = .;

    __begin_kstrtab = .;
    .kstrtab ALIGN(4) : AT(ADDR(.kstrtab)-VIRTUAL_ADDRESS) { *(.kstrtab) }
    __end_kstrtab = .;

    // AP 启动代码段和数据段：
    __begin_apboot_text = .;
    .apboot.text ALIGN(4) : AT(ADDR(.apboot.text)-VIRTUAL_ADDRESS) { *(.apboot.text) }
    __end_apboot_text = .;

    __begin_apboot_data = .;
    .apboot.data ALIGN(4) : AT(ADDR(.apboot.data)-VIRTUAL_ADDRESS) { *(.apboot.data) }
    __end_apboot_data = .;

    // 内核模块属性数据段：
    __begin_kmodsattrb_data = .;
    .kmodsattrbdata ALIGN(4) : AT(ADDR(.kmodsattrbdata)-VIRTUAL_ADDRESS) { *(.kmodsattrbdata) }
    __end_kmodsattrb_data = .;

    // 内核结束标志：
    .krnl.krnend ALIGN(4) : AT(ADDR(.krnl.krnend)-VIRTUAL_ADDRESS) { *(.krnl.krnend) }

    __end_kernel = .;
    // 内核的结束地址。
}

#endif
// 结束 x86 平台的条件编译部分。
