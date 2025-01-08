#ifndef _TYPE_H
#define _TYPE_H

typedef unsigned char u8_t;//__attribute__((__mode__(__QI__))) u8_t;
typedef unsigned short u16_t;//__attribute__((__mode__(__HI__))) u16_t;
typedef unsigned int u32_t;//__attribute__((__mode__(__SI__))) u32_t;
typedef unsigned long long u64_t; //__attribute__((__mode__(__DI__))) u64_t;

typedef char s8_t;
typedef short s16_t;
typedef int s32_t;
typedef long long s64_t;

typedef long sint_t;//有符号整型
typedef unsigned long uint_t;//无符号整型

typedef u64_t cpuflg_t;

typedef unsigned long adr_t;
typedef unsigned long size_t;

typedef u64_t bool_t;
typedef u64_t dev_t;
typedef const char* str_t;
typedef char char_t;
typedef unsigned long drv_t; 
typedef unsigned long mrv_t;
typedef sint_t drvstus_t;
#define KLINE static inline

typedef void (*inthandler_t)();
typedef drv_t (*i_handle_t)(uint_t int_nr);
typedef drv_t (*f_handle_t)(uint_t int_nr,void* sframe);
typedef void (*syscall_t)();
typedef drv_t (*dev_op_t)();
typedef mrv_t (*msg_op_t)();
typedef void* exception_t;
typedef char_t* va_list_t;



#define PUBLIC
#define private	static
#define EXTERN extern
#define KEXTERN extern
#define NULL	0
#define TRUE    1
#define	FALSE	0
#define REGCALL __attribute__((regparm(3)))//表示会用 3 个寄存器来传递参数（EAX、EDX 和 ECX），其余的参数通过堆栈来传递,__attribute__必须配合使用
#define ALIGN(x, a)     (((x) + (a) - 1) & ~((a) - 1))//将任何给定的数 x 向上对齐到最接近的 a 的倍数


#define P4K_ALIGN(x) ALIGN(x,0x1000)//向上对齐到 4KB 的倍数
#endif//_TYPE_H