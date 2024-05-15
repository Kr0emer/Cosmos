#ifndef _VGASTR_T_H
#define _VGASTR_T_H


typedef struct s_CURSOR
{
    //spinlock_t c_lock;
    //spinlock_t p_lock;
    uint_t vmem_s;
    uint_t vmem_e;
    uint_t cvmemadr;
	
    uint_t x;
    uint_t y;
}cursor_t; 

#endif