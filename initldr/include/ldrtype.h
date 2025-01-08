#ifndef _LDRTYPE_H
#define _LDRTYPE_H


#define BFH_RW_R 1
#define BFH_RW_W 2

#define BFH_BUF_SZ 0x1000
#define BFH_ONERW_SZ 0x1000
#define BFH_RWONE_OK 1
#define BFH_RWONE_ER 2
#define BFH_RWALL_OK 3

#define FHDSC_NMAX 192 //文件名长度
#define FHDSC_SZMAX 256
#define MDC_ENDGIC 0xaaffaaffaaffaaff//映像文件的结束标识
#define MDC_RVGIC 0xffaaffaaffaaffaa//映像文件的版本标识
#define REALDRV_PHYADR 0x1000//真实驱动器物理地址
#define ILDRKRL_PHYADR 0x200000//二级引导器的主模块地址
#define IMGSHEL_PHYADR 0x30000
#define IKSTACK_PHYADR (0x90000-0x10)
#define IKSTACK_SIZE 0x1000
#define IMGFILE_PHYADR 0x4000000//映像文件初始物理地址
#define IMGKRNL_PHYADR 0x2000000//内核物理地址
#define KINITPAGE_PHYADR 0x1000000//内核初始化页表
#define KINITFRVM_PHYADR 0x800000//内核初始化虚拟内存的
#define KINITFRVM_SZ 0x400000
#define LDRFILEADR IMGFILE_PHYADR//加载文件的地址，设置为映像文件的物理地址

#define IKSTACK_PHYADR (0x90000-0x10) //内核栈起始地址
#define IKSTACK_SIZE 0x1000//内核栈大小

#define MLOSDSC_OFF (0x1000)//定义一个偏移量，表示到映像文件头描述符（mlosrddsc_t 结构体）的开始位置
#define MRDDSC_ADR (mlosrddsc_t*)(LDRFILEADR+MLOSDSC_OFF)//指向映像文件头描述符的指针

#define KRNL_VIRTUAL_ADDRESS_START 0xffff800000000000//内核虚拟地址的起始地址
#define KPML4_P (1<<0)//PML4（顶级页目录）条目的存在位
#define KPML4_RW (1<<1)//PML4 条目的读写权限
#define KPML4_US (1<<2)// PML4 条目的用户态访问权限
#define KPML4_PWT (1<<3)// PML4 条目的页级写回
#define KPML4_PCD (1<<4)// PML4 条目的页级缓存禁用
#define KPML4_A (1<<5)// PML4 条目的访问位

#define KPDPTE_P (1<<0)//PDPT（页目录表）条目的存在位
#define KPDPTE_RW (1<<1)//PDPT 条目的读写权限
#define KPDPTE_US (1<<2)// PDPT 条目的用户态访问权限
#define KPDPTE_PWT (1<<3)//PDPT 条目的页级写回位
#define KPDPTE_PCD (1<<4)//PDPT 条目的页级缓存禁用位
#define KPDPTE_A (1<<5)//PDPT 条目的访问位

#define KPDE_P (1<<0)//PD（页目录）条目的存在位，如果设置为 1，表示该条目对应的页表存在；如果设置为 0，表示该条目对应的页表不存在
#define KPDE_RW (1<<1)//PD 条目的读写权限，如果设置为 1，表示该页表可读写；如果设置为 0，表示只读
#define KPDE_US (1<<2)//PD 条目的用户态访问权限，如果设置为 1，表示用户态代码可以访问该页表；如果设置为 0，表示用户态代码无法访问
#define KPDE_PWT (1<<3)// PD 条目的页级写回位，如果设置为 1，表示启用写回缓存策略；如果设置为 0，表示启用写直达（Write-Through）缓存策略
#define KPDE_PCD (1<<4)// PD 条目的页级缓存禁用位，如果设置为 1，表示禁用缓存；如果设置为 0，表示启用缓存
#define KPDE_A (1<<5)// PD 条目的访问位，如果设置为 1，表示该页表条目已被访问过；如果设置为 0，表示未被访问
#define KPDE_D (1<<6)//PD 条目的脏位，表示该页表条目对应的页面已被修改过；如果设置为 0，表示未被修改
#define KPDE_PS (1<<7)//PD 条目的页面大小位为1是2mb，页面大小是4kb
#define KPDE_G (1<<8)// PD 条目的全局位，如果设置为 1，表示该页表条目是全局的，不受 CR3 寄存器的影响；如果设置为 0，表示受 CR3 寄存器的影响
#define KPDE_PAT (1<<12)// PD 条目的页属性表位，如果设置为 1，表示启用 PAT；如果设置为 0，表示不启用 PAT

#define KPML4_SHIFT 39
#define KPDPTTE_SHIFT 30
#define KPDP_SHIFT 21
#define PGENTY_SIZE 512


//引导期加载文件模块信息结构体
typedef struct s_mlosrddsc
{
    u64_t mdc_mgic;         // 魔数，用于标识该结构体的合法性（Magic Number）
    u64_t mdc_sfsum;        // 文件的总大小（File System Total Size）
    u64_t mdc_sfsoff;       // 文件起始偏移（Start Offset of File System）
    u64_t mdc_sfeoff;       // 文件结束偏移（End Offset of File System）
    u64_t mdc_sfrlsz;       // 文件的实际大小（Real Size of File System）
    
    u64_t mdc_ldrbk_s;      // 加载块的起始位置（Start of Loader Block）
    u64_t mdc_ldrbk_e;      // 加载块的结束位置（End of Loader Block）
    u64_t mdc_ldrbk_rsz;    // 加载块的实际大小（Real Size of Loader Block）
    u64_t mdc_ldrbk_sum;    // 加载块的总大小（Total Size of Loader Block）
    
    u64_t mdc_fhdbk_s;      // 文件头块的起始位置（Start of File Header Block）
    u64_t mdc_fhdbk_e;      // 文件头块的结束位置（End of File Header Block）
    u64_t mdc_fhdbk_rsz;    // 文件头块的实际大小（Real Size of File Header Block）
    u64_t mdc_fhdbk_sum;    // 文件头块的总大小（Total Size of File Header Block）
    
    u64_t mdc_filbk_s;      // 文件数据块的起始位置（Start of File Data Block）
    u64_t mdc_filbk_e;      // 文件数据块的结束位置（End of File Data Block）
    u64_t mdc_filbk_rsz;    // 文件数据块的实际大小（Real Size of File Data Block）
    u64_t mdc_filbk_sum;    // 文件数据块的总大小（Total Size of File Data Block）
    
    u64_t mdc_ldrcodenr;    // 加载器代码块的数量（Number of Loader Code Blocks）
    u64_t mdc_fhdnr;        // 文件头的数量（Number of File Headers）
    u64_t mdc_filnr;        // 文件的数量（Number of Files）
    
    u64_t mdc_endgic;       // 结束魔数，用于标识结构体的结束（End Magic Number）
    u64_t mdc_rv;           // 保留字段（Reserved Field）
} mlosrddsc_t;

//文件头描述符
typedef struct s_fhdsc
{
    u64_t fhd_type;//文件类型
    u64_t fhd_subtype;//文件子类型
    u64_t fhd_stuts;//文件状态
    u64_t fhd_id;//文件id
    u64_t fhd_intsfsoff;//文件在映像文件位置开始偏移
    u64_t fhd_intsfend;//文件在映像文件的结束偏移
    u64_t fhd_frealsz;//文件实际大小
    u64_t fhd_fsum;//文件校验和
    char   fhd_name[FHDSC_NMAX];//文件名
}fhdsc_t;


typedef struct s_MRSDP
{
    u64_t rp_sign;//签名字段，用于标识这个结构体包含的数据类型或格式
    u8_t rp_chksum;//校验和字段，用于验证结构体数据的完整性
    u8_t rp_oemid[6];//OEM（原始设备制造商）标识符，用于标识创建这个表的硬件制造商
    u8_t rp_revn;//修订号，指示这个结构体或数据表的版本
    u32_t rp_rsdtphyadr;//RSDT（Root System Description Table）的物理地址，这是ACPI中的一个表，包含了其他所有系统描述表的地址
                        //操作系统通过 rp_rsdtphyadr 找到 RSDT 表。通过 RSDT 表中的地址数组，逐一解析其他 ACPI 表，完成系统初始化。
    u32_t rp_len;//这个结构体或者相关数据表的长度
    u64_t rp_xsdtphyadr;//XSDT（Extended System Description Table）的物理地址，这是一个扩展的系统描述表，包含了更多的系统描述表地址
    u8_t rp_echksum;//扩展校验和，可能用于验证整个结构体或扩展表的完整性
    u8_t rp_resv[3];//保留字段
}__attribute__((packed)) mrsdp_t;//这里的__attribute__((packed))用于告诉编译器取消结构体在编译过程中字段紧密排列，没有内存对齐填充


typedef struct s_VBEINFO
{
        char vbesignature[4];//VBE签名，通常是 “VESA” 的ASCII码，用于标识这是一个VBE信息块
        u16_t vbeversion;//VBE版本号，以BCD（二进制编码的十进制）格式存储，例如0x0200表示VBE 2.0
        u32_t oemstringptr;//指向OEM（原始设备制造商）字符串的指针，这个字符串包含了制造商的信息
        u32_t capabilities;//描述图形卡能力的位字段，例如是否支持窗口化或线性帧缓冲区
        u32_t videomodeptr;//指向支持的视频模式列表的指针
        u16_t totalmemory;//图形卡的视频内存总量，以64KB块为单位
        u16_t oemsoftwarerev;//OEM软件的修订版本号
        u32_t oemvendornameptr;//指向OEM供应商名称的指针
        u32_t oemproductnameptr;//指向OEM产品名称的指针
        u32_t oemproductrevptr;//指向OEM产品修订号的指针
        u8_t reserved[222];//保留字段，用于未来扩展，应被设置为0
        u8_t oemdata[256];//OEM数据区，可以由OEM用来存储特定信息
}__attribute__((packed)) vbeinfo_t;//这里的__attribute__((packed))用于告诉编译器取消结构体在编译过程中字段紧密排列，没有内存对齐填充


typedef struct s_VBEOMINFO
{
    u16_t ModeAttributes;//模式属性，指示支持的特性，如是否支持图形和文本模式
    u8_t  WinAAttributes;//窗口A的属性，如是否可读可写
    u8_t  WinBAttributes;//窗口b的属性，如是否可读可写
    u16_t WinGranularity;//窗口粒度，表示窗口数据在内存中的最小移动单位
    u16_t WinSize;//窗口大小，通常以KB为单位
    u16_t WinASegment;//窗口A在内存中的段地址
    u16_t WinBSegment;//窗口B在内存中的段地址
    u32_t WinFuncPtr;//指向窗口切换函数的指针
    u16_t BytesPerScanLine;//每扫描行的字节数
    u16_t XResolution;//水平分辨率
    u16_t YResolution;//垂直分辨率
    u8_t  XCharSize;//字符的水平尺寸
    u8_t  YCharSize;//字符的垂直尺寸
    u8_t  NumberOfPlanes;//图形平面的数量
    u8_t  BitsPerPixel;//每像素的位数
    u8_t  NumberOfBanks;//内存银行的数量
    u8_t  MemoryModel;//内存模型，如直接色彩或索引色彩
    u8_t  BankSize;//每个内存银行的大小
    u8_t  NumberOfImagePages;//图像页的数量
    u8_t  Reserved;//保留字段
    u8_t  RedMaskSize;//红色掩码大小
    u8_t  RedFieldPosition;//红色的字段位置
    u8_t  GreenMaskSize;//绿色的掩码大小
    u8_t  GreenFieldPosition;//绿色的字段位置
    u8_t  BlueMaskSize;//蓝色的掩码大小
    u8_t  BlueFieldPosition;//蓝色的字段位置
    u8_t  RsvdMaskSize;//保留颜色的掩码大小
    u8_t  RsvdFieldPosition;//保留颜色的字段位置
    u8_t  DirectColorModeInfo;//直接色彩模式信息
    u32_t PhysBasePtr;//物理基址指针，指向帧缓冲区的物理地址
    u32_t Reserved1;//保留字段
    u16_t Reserved2;//保留字段
    u16_t LinBytesPerScanLine;//线性模式下每扫描行的字节数
    u8_t  BnkNumberOfImagePages;//银行模式下的图像页数量
    u8_t  LinNumberOfImagePages;//线性模式下的图像页数量
    u8_t  LinRedMaskSize;//线性模式下红色的掩码大小。
    u8_t  LinRedFieldPosition;//线性模式下红色的字段位置。
    u8_t  LinGreenMaskSize;//线性模式下绿色的掩码大小。
    u8_t  LinGreenFieldPosition;//线性模式下绿色的字段位置。
    u8_t  LinBlueMaskSize;//线性模式下蓝色的掩码大小。
    u8_t  LinBlueFieldPosition;//线性模式下蓝色的字段位置。
    u8_t  LinRsvdMaskSize;//线性模式下保留颜色的掩码大小。
    u8_t  LinRsvdFieldPosition;//线性模式下保留颜色的字段位置。
    u32_t MaxPixelClock;//最大像素时钟频率
    u8_t  Reserved3[189];//保留字段
}__attribute__((packed)) vbeominfo_t;//这里的__attribute__((packed))用于告诉编译器取消结构体在编译过程中字段紧密排列，没有内存对齐填充

typedef struct s_PIXCL
{
    u8_t cl_b;
    u8_t cl_g;
    u8_t cl_r;
    u8_t cl_a;
}__attribute__((packed)) pixcl_t;
#define BGRA(r,g,b) ((0|(r<<16)|(g<<8)|b))
typedef u32_t pixl_t;

#define VBEMODE 1
#define GPUMODE 2
#define BGAMODE 3


typedef struct s_GRAPH
{
    u32_t gh_mode;//图形模式
    u32_t gh_x;//图形的坐标，表示图形在屏幕上的位置，x轴
    u32_t gh_y;//图形的坐标，表示图形在屏幕上的位置，y轴
    u32_t gh_framphyadr;//帧缓冲区的物理地址，用于存储图形数据
    u32_t gh_onepixbits;//每个像素的位数，通常表示颜色深度
    u32_t gh_vbemodenr;//VBE（VESA BIOS Extensions）模式号，用于选择不同的图形模式
    u32_t gh_vifphyadr;//虚拟画布的起始物理地址
    u32_t gh_vmifphyadr;//虚拟画布的映射信息或管理信息的物理地址
    u32_t gh_bank;//当前激活的内存银行或页面
    u32_t gh_curdipbnk;//当前正在显示的内存银行或页面
    u32_t gh_nextbnk;//下一个将要被激活或显示的内存银行或页面
    u32_t gh_banksz;//每个内存银行或页面的大小
    u32_t gh_logophyadrs;//逻辑地址的开始
    u32_t gh_logophyadre;//逻辑地址的结束
    u32_t gh_fontadr;//字体的物理地址
    u32_t gh_ftsectadr;//字体的扇区地址
    u32_t gh_ftsectnr;//字体的扇区数量
    u32_t gh_fnthight;//字体的高度
    u32_t gh_nxtcharsx;//下一个字符的x坐标
    u32_t gh_nxtcharsy;//下一个字符的y坐标
    u32_t gh_linesz;//每行显示的像素数量或字节大小
    vbeinfo_t gh_vbeinfo;//VESA BIOS扩展（VBE）信息
    vbeominfo_t gh_vminfo;//VESA BIOS扩展（VBE）的输出模式信息块（OMIB）的详细信息
}__attribute__((packed)) graph_t;//这里的__attribute__((packed))用于告诉编译器取消结构体在编译过程中字段紧密排列，没有内存对齐填充

typedef struct s_BMFHEAD
{
    u16_t bf_tag;   //0x4d42
    u32_t bf_size;
    u16_t bf_resd1;
    u16_t bf_resd2;
    u32_t bf_off;
}__attribute__((packed)) bmfhead_t;//14

typedef struct s_BITMINFO
{
    u32_t bi_size;
    s32_t bi_w;
    s32_t bi_h;
    u16_t bi_planes;
    u16_t bi_bcount;
    u32_t bi_comp;
    u32_t bi_szimg;
    s32_t bi_xpelsper;
    s32_t bi_ypelsper;
    u32_t bi_clruserd;
    u32_t bi_clrimport;
}__attribute__((packed)) bitminfo_t;//40

typedef struct s_BMDBGR
{
    u8_t bmd_b;
    u8_t bmd_g;
    u8_t bmd_r;
}__attribute__((packed)) bmdbgr_t;

typedef struct s_UTF8
{
    u8_t utf_b1;
    u8_t utf_b2;
    u8_t utf_b3;
    u8_t utf_b4;
    u8_t utf_b5;
    u8_t utf_b6;
}__attribute__((packed)) utf8_t;

typedef struct s_FONTFHDER
{
    u8_t  fh_mgic[4];
    u32_t fh_fsize;
    u8_t  fh_sectnr;
    u8_t  fh_fhght;
    u16_t fh_wcpflg;
    u16_t fh_nchars;
    u8_t  fh_rev[2];
}__attribute__((packed)) fontfhder_t;

typedef struct s_FTSECTIF
{
    u16_t fsf_fistuc;
    u16_t fsf_lastuc;
    u32_t fsf_off;
}__attribute__((packed)) ftsectif_t;

typedef struct s_UFTINDX
{
    u32_t ui_choff:26;
    u32_t ui_chwith:6;
}__attribute__((packed)) uftindx_t;
#define MAX_CHARBITBY 32
typedef struct s_FNTDATA
{
    u8_t fntwxbyte;
    u8_t fnthx;
    u8_t fntwx;
    u8_t fntchmap[MAX_CHARBITBY];
}__attribute__((packed)) fntdata_t;


typedef struct s_KLFOCPYMBLK
{
    u64_t sphyadr;
    u64_t ocymsz;
}__attribute__((packed)) klfocpymblk_t;

//mbsp，多重引导规范 (MultiBoot Specification) 是一个由 GRUB 引导程序定义的规范，用于引导多种操作系统。用于表示内核的各种信息
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


typedef struct s_DPT {
    u8_t dp_bmgic;
    u8_t dp_pshead;
    u8_t dp_pssector;
    u8_t dp_pscyder;
    u8_t dp_ptype;
    u8_t dp_pehead;
    u8_t dp_pesector;
    u8_t dp_pecyder;
    u32_t dp_pslba;
    u32_t dp_plbasz;
} __attribute__((packed)) dpt_t;

typedef struct s_IDTR {
    u16_t idtlen;
    u32_t idtbas;
} idtr_t;

typedef struct s_MBR {
    char_t mb_byte[446];
    dpt_t mb_part[4];
    u16_t mb_endmgic;
} __attribute__((packed)) mbr_t;

typedef struct s_EBR {
    char_t eb_byte[446];
    dpt_t eb_part[4];
    u16_t eb_endmgic;
} __attribute__((packed)) ebr_t;

typedef struct s_RWHDPACK {
    u8_t rwhpk_sz;
    u8_t rwhpk_rv;
    u8_t rwhpk_sn;
    u8_t rwhpk_rv1;
    u16_t rwhpk_of;
    u16_t rwhpk_se;
    u32_t rwhpk_ll;
    u32_t rwhpk_lh;
} __attribute__((packed)) rwhdpach_t;

#define RLINTNR(x) (x*2)




#define RAM_USABLE 1 //可用内存
#define RAM_RESERV 2 //保留内存不可使用
#define RAM_ACPIREC 3 //ACPI表相关的
#define RAM_ACPINVS 4 //ACPI NVS空间
#define RAM_AREACON 5 //包含坏内存

typedef struct s_e820{
    u64_t saddr;//内存开始地址 
    u64_t lsize;//内存大小 
    u32_t type;//内存类型
}__attribute__((packed)) e820map_t;//单纯用来传送内存信息的结构体，用于存储从 BIOS 获取的物理内存布局信息

#define MBSPADR ((machbstart_t*)(0x100000))
#define MBS_MIGC (u64_t)((((u64_t)'K')<<56)|(((u64_t)'R')<<48)|(((u64_t)'0')<<40)|(((u64_t)'E')<<32)|(((u64_t)'M')<<24)|(((u64_t)'E')<<16)|(((u64_t)'R')<<8)|((u64_t)'M'))
// 定义 VBE（VESA BIOS Extensions）相关的 I/O 端口
// VBE_DISPI_IOPORT_INDEX 是索引寄存器的端口地址
#define VBE_DISPI_IOPORT_INDEX (0x01CE)
// VBE_DISPI_IOPORT_DATA 是数据寄存器的端口地址
#define VBE_DISPI_IOPORT_DATA (0x01CF)

// VBE 的索引寄存器地址定义
// VBE_DISPI_INDEX_ID：ID寄存器
#define VBE_DISPI_INDEX_ID (0)
// VBE_DISPI_INDEX_XRES：水平分辨率寄存器
#define VBE_DISPI_INDEX_XRES (1)
// VBE_DISPI_INDEX_YRES：垂直分辨率寄存器
#define VBE_DISPI_INDEX_YRES (2)
// VBE_DISPI_INDEX_BPP：每像素位数寄存器
#define VBE_DISPI_INDEX_BPP (3)
// VBE_DISPI_INDEX_ENABLE：使能寄存器
#define VBE_DISPI_INDEX_ENABLE (4)
// VBE_DISPI_INDEX_BANK：显存分页寄存器
#define VBE_DISPI_INDEX_BANK (5)
// VBE_DISPI_INDEX_VIRT_WIDTH：虚拟宽度寄存器
#define VBE_DISPI_INDEX_VIRT_WIDTH (6)
// VBE_DISPI_INDEX_VIRT_HEIGHT：虚拟高度寄存器
#define VBE_DISPI_INDEX_VIRT_HEIGHT (7)
// VBE_DISPI_INDEX_X_OFFSET：水平偏移寄存器
#define VBE_DISPI_INDEX_X_OFFSET (8)
// VBE_DISPI_INDEX_Y_OFFSET：垂直偏移寄存器
#define VBE_DISPI_INDEX_Y_OFFSET (9)

// BGA（Bochs Graphics Adapter）设备 ID 定义
// BGA_DEV_ID0：支持设置 X 和 Y 分辨率以及位深度（仅支持 8 BPP），使用分段显存模式
#define BGA_DEV_ID0 (0xB0C0)
// BGA_DEV_ID1：支持设置虚拟宽度和高度，以及 X 和 Y 偏移量
#define BGA_DEV_ID1 (0xB0C1)
// BGA_DEV_ID2：支持 15, 16, 24 和 32 BPP 模式，支持线性帧缓冲区和模式切换时保留显存内容
#define BGA_DEV_ID2 (0xB0C2)
// BGA_DEV_ID3：支持获取设备能力，支持使用 8 位 DAC（数字模拟转换器）
#define BGA_DEV_ID3 (0xB0C3)
// BGA_DEV_ID4：显存增加到 8 MB
#define BGA_DEV_ID4 (0xB0C4)
// BGA_DEV_ID5：显存增加到 16 MB（需验证，可能还有其他更改）
#define BGA_DEV_ID5 (0xB0C5)

// VBE 支持的每像素位数（BPP）定义
// 4 位颜色深度
#define VBE_DISPI_BPP_4 (0x04)
// 8 位颜色深度
#define VBE_DISPI_BPP_8 (0x08)
// 15 位颜色深度
#define VBE_DISPI_BPP_15 (0x0F)
// 16 位颜色深度
#define VBE_DISPI_BPP_16 (0x10)
// 24 位颜色深度
#define VBE_DISPI_BPP_24 (0x18)
// 32 位颜色深度
#define VBE_DISPI_BPP_32 (0x20)

// VBE 显示功能的使能/禁用标志
// 禁用显示
#define VBE_DISPI_DISABLED (0x00)
// 启用显示
#define VBE_DISPI_ENABLED (0x01)
// 启用线性帧缓冲区（LFB，Linear Frame Buffer）
#define VBE_DISPI_LFB_ENABLED (0x40)


void REGCALL realadr_call_entry(u16_t callint,u16_t val1,u16_t val2);//eax edx ecx
#endif //_LDRTYPE_H