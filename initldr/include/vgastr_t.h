#ifndef _VGASTR_T_H
#define _VGASTR_T_H
#define VGASTR_RAM_BASE (0xb8000)
#define VGASTR_RAM_END  (0xbffff)
#define VGADP_DFVL 0x0700
#define VGADP_HLVL 0x0f00
#define VGACTRL_REG_ADR 0x3d4
#define VGACTRL_REG_DAT 0x3d5
#define VGACURS_REG_INX 0x0a
#define VGACURS_CLOSE 0x20
#define VGACHAR_LR_CFLG 10
#define VGACHAR_DF_CFLG 0

#define VGASADH_REG_INX 0x0c
#define VGASADL_REG_INX 0x0d
typedef struct s_CURSOR//表示光标的数据结构
{
    //spinlock_t c_lock;
    //spinlock_t p_lock;
    uint_t vmem_s;//内存的起始地址
    uint_t vmem_e;//内存的结束地址
    uint_t cvmemadr;
	
    uint_t x;
    uint_t y;//这两个变量用于表示坐标
}cursor_t; 

#endif