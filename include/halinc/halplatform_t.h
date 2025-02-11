/**********************************************************
        平台相关的宏定义文件platform_t.h
***********************************************************
                
**********************************************************/
#ifndef _PLATFORM_T_H
#define _PLATFORM_T_H
#include "bdvideo_t.h"


#ifdef CFG_X86_PLATFORM
#define CPUCORE_MAX 1
#define SDRAM_MAPVECTPHY_ADDR 0x30000000

#define KRNL_INRAM_START 0x30000000
#define LINK_VIRT_ADDR 0x30008000
#define LINK_LINE_ADDR 0x30008000
#define KERNEL_VIRT_ADDR 0x30008000
#define KERNEL_PHYI_ADDR 0x30008000
#define PAGE_TLB_DIR 0x30004000
#define PAGE_TLB_SIZE 4096
#define INIT_HEAD_STACK_ADDR 0x34000000

#define CPU_VECTOR_PHYADR 0x30000000
#define CPU_VECTOR_VIRADR 0


#define PTE_SECT_AP (3<<10)
#define PTE_SECT_DOMAIN (0<<5)
#define PTE_SECT_NOCW (0<<2)
#define PTE_SECT_BIT (2)

#define PLFM_ADRSPCE_NR 29

#define INTSRC_MAX 32

#define KRNL_MAP_VIRTADDRESS_SIZE 0x400000000
#define KRNL_VIRTUAL_ADDRESS_START 0xffff800000000000
#define KRNL_VIRTUAL_ADDRESS_END 0xffffffffffffffff
#define USER_VIRTUAL_ADDRESS_START 0
#define USER_VIRTUAL_ADDRESS_END 0x00007fffffffffff
#define KRNL_MAP_PHYADDRESS_START 0
#define KRNL_MAP_PHYADDRESS_END 0x400000000
#define KRNL_MAP_PHYADDRESS_SIZE 0x400000000
#define KRNL_MAP_VIRTADDRESS_START KRNL_VIRTUAL_ADDRESS_START
#define KRNL_MAP_VIRTADDRESS_END (KRNL_MAP_VIRTADDRESS_START+KRNL_MAP_VIRTADDRESS_SIZE)
#define KRNL_ADDR_ERROR 0xf800000000000


#define MBS_MIGC (u64_t)((((u64_t)'L')<<56)|(((u64_t)'M')<<48)|(((u64_t)'O')<<40)|(((u64_t)'S')<<32)|(((u64_t)'M')<<24)|(((u64_t)'B')<<16)|(((u64_t)'S')<<8)|((u64_t)'P'))

typedef struct s_MRSDP
{
    u64_t rp_sign;
    u8_t rp_chksum;
    u8_t rp_oemid[6];
    u8_t rp_revn;
    u32_t rp_rsdtphyadr;
    u32_t rp_len;
    u64_t rp_xsdtphyadr;
    u8_t rp_echksum;
    u8_t rp_resv[3];
}__attribute__((packed)) mrsdp_t;


//mbsp，多重引导规范 (MultiBoot Specification) 是一个由 GRUB 引导程序定义的规范，用于引导多种操作系统。用于表示内核的各种信息
typedef struct s_MACHBSTART
{
    u64_t   mb_migc;           // 机器标识码（可能是用于区分机器或平台的唯一标识符）
    u64_t   mb_chksum;         // 校验和，用于验证该结构体数据的完整性
    u64_t   mb_krlinitstack;   // 内核初始化栈的物理地址
    u64_t   mb_krlitstacksz;   // 内核初始化栈的大小
    u64_t   mb_imgpadr;        // 镜像文件的物理地址（可能是操作系统的内核镜像）
    u64_t   mb_imgsz;          // 镜像文件的大小
    u64_t   mb_krlimgpadr;     // 内核镜像的物理地址
    u64_t   mb_krlsz;          // 内核镜像的大小
    u64_t   mb_krlvec;         // 内核向量的物理地址（可能是异常处理或系统调用向量）
    u64_t   mb_krlrunmode;     // 内核运行模式标识
    u64_t   mb_kalldendpadr;   // 内核调用结束后的物理地址
    u64_t   mb_ksepadrs;       // 内核栈起始地址
    u64_t   mb_ksepadre;       // 内核栈结束地址
    u64_t   mb_kservadrs;      // 内核服务起始地址
    u64_t   mb_kservadre;      // 内核服务结束地址
    u64_t   mb_nextwtpadr;     // 下一个等待队列的物理地址
    u64_t   mb_bfontpadr;      // 基本字体的物理地址（可能用于显示）
    u64_t   mb_bfontsz;        // 基本字体的大小
    u64_t   mb_fvrmphyadr;     // 固件资源的物理地址
    u64_t   mb_fvrmsz;         // 固件资源的大小
    u64_t   mb_cpumode;        // CPU 模式标识
    u64_t   mb_memsz;          // 总物理内存大小
    u64_t   mb_e820padr;       // E820 内存映射表的物理地址（E820 是用于描述系统内存的标准）
    u64_t   mb_e820nr;         // E820 内存映射表的条目数量
    u64_t   mb_e820sz;         // E820 内存映射表的大小
    u64_t   mb_e820expadr;     // 扩展的 E820 内存映射区域的物理地址
    u64_t   mb_e820exnr;       // 扩展的 E820 内存映射区域的条目数量
    u64_t   mb_e820exsz;       // 扩展的 E820 内存映射区域的大小
    u64_t   mb_memznpadr;      // 物理内存区域起始地址
    u64_t   mb_memznnr;        // 物理内存区域的条目数量
    u64_t   mb_memznsz;        // 物理内存区域的大小
    u64_t   mb_memznchksum;    // 物理内存区域的校验和
    u64_t   mb_memmappadr;     // 内存页描述符首地址
    u64_t   mb_memmapnr;       // 内存页描述符首地址数量
    u64_t   mb_memmapsz;       // 内存映射区域的总大小
    u64_t   mb_memmapchksum;   // 内存映射的校验和
    u64_t   mb_pml4padr;       // PML4（页映射级别 4）的物理地址，涉及到虚拟内存管理
    u64_t   mb_subpageslen;    // 子页面的长度（可能与内存分配或管理有关）
    u64_t   mb_kpmapphymemsz;  // 内核的物理内存大小（可能用于内存管理或映射）
    u64_t   mb_ebdaphyadr;     // 设备的物理地址（可能是硬件设备或输入输出设备的地址）
    mrsdp_t mb_mrsdp;          // ACPI（高级配置和电源接口）描述符指针（可能用于电源管理）
    graph_t mb_ghparm;         // 图形参数，可能与显示或图形相关的硬件配置
} __attribute__((packed)) machbstart_t;


#define MBSPADR ((machbstart_t*)(0x100000))


#define BFH_RW_R 1
#define BFH_RW_W 2

#define BFH_BUF_SZ 0x1000
#define BFH_ONERW_SZ 0x1000
#define BFH_RWONE_OK 1
#define BFH_RWONE_ER 2
#define BFH_RWALL_OK 3

#define FHDSC_NMAX 192
#define FHDSC_SZMAX 256
#define MDC_ENDGIC 0xaaffaaffaaffaaff
#define MDC_RVGIC 0xffaaffaaffaaffaa

#define MLOSDSC_OFF (0x1000)
#define RAM_USABLE 1
#define RAM_RESERV 2
#define RAM_ACPIREC 3
#define RAM_ACPINVS 4
#define RAM_AREACON 5
typedef struct s_e820{
    u64_t saddr;    /* start of memory segment8 */
    u64_t lsize;    /* size of memory segment8 */
    u32_t type;    /* type of memory segment 4*/
}__attribute__((packed)) e820map_t;

typedef struct s_fhdsc
{
    u64_t fhd_type;
    u64_t fhd_subtype;
    u64_t fhd_stuts;
    u64_t fhd_id;
    u64_t fhd_intsfsoff;
    u64_t fhd_intsfend;
    u64_t fhd_frealsz;
    u64_t fhd_fsum;
    char   fhd_name[FHDSC_NMAX];
}fhdsc_t;

typedef struct s_mlosrddsc
{
    u64_t mdc_mgic;
    u64_t mdc_sfsum;
    u64_t mdc_sfsoff;
    u64_t mdc_sfeoff;
    u64_t mdc_sfrlsz;
    u64_t mdc_ldrbk_s;
    u64_t mdc_ldrbk_e;
    u64_t mdc_ldrbk_rsz;
    u64_t mdc_ldrbk_sum;
    u64_t mdc_fhdbk_s;
    u64_t mdc_fhdbk_e;
    u64_t mdc_fhdbk_rsz;
    u64_t mdc_fhdbk_sum;
    u64_t mdc_filbk_s;
    u64_t mdc_filbk_e;
    u64_t mdc_filbk_rsz;
    u64_t mdc_filbk_sum;
    u64_t mdc_ldrcodenr;
    u64_t mdc_fhdnr;
    u64_t mdc_filnr;
    u64_t mdc_endgic;
    u64_t mdc_rv;
}mlosrddsc_t;



#endif



#endif 
