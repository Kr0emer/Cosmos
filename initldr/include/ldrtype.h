#ifndef _LDRTYPE_H
#define _LDRTYPE_H


#define FHDSC_NMAX 192 //文件名长度
#define MDC_ENDGIC 0xaaffaaffaaffaaff//映像文件的结束标识
#define MDC_RVGIC 0xffaaffaaffaaffaa//映像文件的版本标识
#define REALDRV_PHYADR 0x1000//真实驱动器物理地址
#define ILDRKRL_PHYADR 0x200000//二级引导器的主模块地址
#define IMGFILE_PHYADR 0x4000000//映像文件物理地址
#define IMGKRNL_PHYADR 0x2000000//映像内核物理地址
#define LDRFILEADR IMGFILE_PHYADR//加载文件的地址，设置为映像文件的物理地址

#define IKSTACK_PHYADR (0x90000-0x10) //内核栈起始地址
#define IKSTACK_SIZE 0x1000//内核栈大小

#define MLOSDSC_OFF (0x1000)//定义一个偏移量，表示到映像文件头描述符（mlosrddsc_t 结构体）的开始位置
#define MRDDSC_ADR (mlosrddsc_t*)(LDRFILEADR+MLOSDSC_OFF)//指向映像文件头描述符的指针

//映像文件头描述符
typedef struct s_mlosrddsc
{
    u64_t mdc_mgic; //映像文件标识
    u64_t mdc_sfsum;//未使用
    u64_t mdc_sfsoff;//未使用
    u64_t mdc_sfeoff;//未使用
    u64_t mdc_sfrlsz;//未使用
    u64_t mdc_ldrbk_s;//映像文件中二级引导器的开始偏移
    u64_t mdc_ldrbk_e;//映像文件中二级引导器的结束偏移
    u64_t mdc_ldrbk_rsz;//映像文件中二级引导器的实际大小
    u64_t mdc_ldrbk_sum;//映像文件中二级引导器的校验和
    u64_t mdc_fhdbk_s;//映像文件中文件头描述的开始偏移
    u64_t mdc_fhdbk_e;//映像文件中文件头描述的结束偏移
    u64_t mdc_fhdbk_rsz;//映像文件中文件头描述的实际大小
    u64_t mdc_fhdbk_sum;//映像文件中文件头描述的校验和
    u64_t mdc_filbk_s;//映像文件中文件数据的开始偏移
    u64_t mdc_filbk_e;//映像文件中文件数据的结束偏移
    u64_t mdc_filbk_rsz;//映像文件中文件数据的实际大小
    u64_t mdc_filbk_sum;//映像文件中文件数据的校验和
    u64_t mdc_ldrcodenr;//映像文件中二级引导器的文件头描述符的索引号
    u64_t mdc_fhdnr;//映像文件中文件头描述符有多少个
    u64_t mdc_filnr;//映像文件中文件头有多少个
    u64_t mdc_endgic;//映像文件结束标识
    u64_t mdc_rv;//映像文件版本
}mlosrddsc_t;


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


typedef struct s_MACHBSTART
{
    u64_t mb_migc;//魔数
    u64_t mb_chksum;//校验和
    u64_t mb_krlinitstack;//表示内核的初始堆栈指针
    u64_t mb_krlitstacksz;//表示内核初始堆栈的大小
    u64_t mb_imgpadr;//操作系统映像
    u64_t mb_imgsz;////操作系统映像大小
    u64_t mb_krlimgpadr;//内核映像的物理地址
    u64_t mb_krlsz;//内核映像的大小
    u64_t mb_krlvec;//内核向量地址
    u64_t mb_krlrunmode;//内核运行模式
    u64_t mb_kalldendpadr;//内核结束地址
    u64_t mb_ksepadrs;//内核段开始地址
    u64_t mb_ksepadre;//内核段结束地址
    u64_t mb_kservadrs;//内核服务段开始地址
    u64_t mb_kservadre;//内核服务段结束地址
    u64_t mb_nextwtpadr;//下一个等待线程的地址
    u64_t mb_bfontpadr;//字体的物理地址
    u64_t mb_bfontsz;//字体的大小
    u64_t mb_fvrmphyadr;//虚拟内存的物理地址
    u64_t mb_fvrmsz;//虚拟内存的大小
    u64_t mb_cpumode;//CPU模式
    u64_t mb_memsz;//内存大小
    u64_t mb_e820padr;//E820表的物理地址，E820表是由BIOS提供的物理内存映射表，来告诉我们那些是留给BIOS使用，哪些内存地址范围可以用
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
    u64_t mb_pml4padr;//PML4表的物理地址
    u64_t mb_subpageslen;//子页面的长度
    u64_t mb_kpmapphymemsz;//内核页面映射的物理内存大小
    u64_t mb_ebdaphyadr;//EBDA的物理地址，Extended BIOS Data Area，BIOS会使用这个区域来存储额外的系统信息和数据
    mrsdp_t mb_mrsdp;//MRSDP用于描述系统内存表的信息
    graph_t mb_ghparm;//图形参数
}__attribute__((packed)) machbstart_t;;//这里的__attribute__((packed))用于告诉编译器取消结构体在编译过程中字段紧密排列，没有内存对齐填充




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
}__attribute__((packed)) e820map_t;//用于存储从 BIOS 获取的物理内存布局信息

#define MBSPADR ((machbstart_t*)(0x100000))
#define MBS_MIGC (u64_t)((((u64_t)'K')<<56)|(((u64_t)'R')<<48)|(((u64_t)'0')<<40)|(((u64_t)'E')<<32)|(((u64_t)'M')<<24)|(((u64_t)'E')<<16)|(((u64_t)'R')<<8)|((u64_t)'M'))



void REGCALL realadr_call_entry(u16_t callint,u16_t val1,u16_t val2);//eax edx ecx
#endif //_LDRTYPE_H