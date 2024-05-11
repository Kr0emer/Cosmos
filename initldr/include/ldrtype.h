#ifndef _LDRTYPE_H
#define _LDRTYPE_H


#define FHDSC_NMAX 192 //文件名长度
#define MDC_ENDGIC 0xaaffaaffaaffaaff//映像文件的结束标识
#define MDC_RVGIC 0xffaaffaaffaaffaa//映像文件的版本标识
#define REALDRV_PHYADR 0x1000//真实驱动器物理地址
#define IMGFILE_PHYADR 0x4000000//映像文件物理地址
#define IMGKRNL_PHYADR 0x2000000//映像内核物理地址
#define LDRFILEADR IMGFILE_PHYADR//加载文件的地址，设置为映像文件的物理地址。

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

#endif //_LDRTYPE_H