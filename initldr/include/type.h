#ifndef _TYPE_H
#define _TYPE_H

typedef unsigned char u8_t;//__attribute__((__mode__(__QI__))) u8_t;
typedef unsigned int u32_t;//__attribute__((__mode__(__SI__))) u32_t;
typedef unsigned long long u64_t; //__attribute__((__mode__(__DI__))) u64_t;


typedef long long s64_t;
typedef long sint_t;//有符号整型
typedef unsigned long uint_t;//无符号整型


typedef char char_t; 






typedef char_t* va_list_t;

#define KLINE static inline



#define NULL 0
#endif //_TYPE_H