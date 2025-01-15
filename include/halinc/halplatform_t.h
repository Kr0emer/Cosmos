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


typedef struct s_MACHBSTART
{
    u64_t mb_migc;//魔数
    u64_t mb_chksum;//校验和
    u64_t mb_krlinitstack;//表示内核的初始堆栈指针
    u64_t mb_krlitstacksz;//表示内核初始堆栈的大小
    u64_t mb_imgpadr;//操作系统映像文件文件开始位置
    u64_t mb_imgsz;////操作系统映像文件文件大小
    u64_t mb_krlimgpadr;//操作系统（内核）核心文件的物理地址（COSMOS.bin）
    u64_t mb_krlsz;//操作系统（内核）核心文件的大小
    u64_t mb_krlvec;//内核向量地址
    u64_t mb_krlrunmode;//内核运行模式
    u64_t mb_kalldendpadr;//内核结束地址
    u64_t mb_ksepadrs;//内核段开始地址
    u64_t mb_ksepadre;//内核段结束地址
    u64_t mb_kservadrs;//内核服务段开始地址
    u64_t mb_kservadre;//内核服务段结束地址
    u64_t mb_nextwtpadr;//下一个空闲内存地址
    u64_t mb_bfontpadr;//字体的物理地址
    u64_t mb_bfontsz;//字体的大小
    u64_t mb_fvrmphyadr;//虚拟内存的物理地址
    u64_t mb_fvrmsz;//虚拟内存的大小
    u64_t mb_cpumode;//CPU模式
    u64_t mb_memsz;//内存大小
    u64_t mb_e820padr;//E820表的地址，E820表是由BIOS提供的物理内存映射表，来告诉我们那些是留给BIOS使用，哪些内存地址范围可以用
    u64_t mb_e820nr;//E820表的数量
    u64_t mb_e820sz;//E820表的大小
    u64_t mb_e820expadr;//扩展E820表的物理地址
    u64_t mb_e820exnr;//扩展E820表的数量
    u64_t mb_e820exsz;//扩展E820表的大小
    u64_t mb_memznpadr;//内存零页的物理地址
    u64_t mb_memznnr;//内存零页的数量
    u64_t mb_memznsz;//内存零页的大小
    u64_t mb_memznchksum;//内存零页的校验和
    u64_t mb_memmappadr;//内存映射的物理地址
    u64_t mb_memmapnr;//内存映射的数量
    u64_t mb_memmapsz;//内存映射的大小
    u64_t mb_memmapchksum;//内存映射的校验和
    u64_t mb_pml4padr;//PML4表的物理地址，x86架构中最高层页表
    u64_t mb_subpageslen;//MMU映射表的总长度
    u64_t mb_kpmapphymemsz;//内核页面映射的物理内存大小
    u64_t mb_ebdaphyadr;//EBDA的起始物理地址，Extended BIOS Data Area，BIOS会使用这个区域来存储额外的系统信息和数据
    mrsdp_t mb_mrsdp;//MRSDP用于描述系统内存表的信息
    graph_t mb_ghparm;//图形参数(分辨率，显存地址等)
}__attribute__((packed)) machbstart_t;;//这里的__attribute__((packed))用于告诉编译器取消结构体在编译过程中字段紧密排列，没有内存对齐填充



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
