#include "cmctl.h"
__attribute__((section(".data"))) cursor_t curs;

char_t *strcopy(char_t *buf, char_t *str_s)
{
    while (*str_s)
    {
        *buf = *str_s;
        buf++;
        str_s++;
    }
    return buf;
}



char_t *numberk(char_t *str, uint_t n, sint_t base)
{
    register char_t *p;// 声明一个指针p，用于操作字符串
    char_t strbuf[36];
    p = &strbuf[36];//将指针p初始化为指向strbuf数组的末尾
    *--p = 0;// 在字符串的末尾放置一个空字符，以标记字符串的结束

    if (n == 0)// 如果数字为0，则在字符串中放置字符'0'
    {
        *--p = '0';
    }

    else
    {
        do
        {
            *--p = "0123456789abcdef"[n % base];// 将数字n转换为指定基数的字符
        } while (n /= base); // 继续这个过程，直到n为0
    }
    while (*p != 0)
    {
        *str++ = *p++;
    }
    return str;
}


void vsprintfk(char_t *buf, const char_t *fmt, va_list_t args)
{
    char_t *p = buf;
    va_list_t next_arg = args;
    while (*fmt)
    {
        if(*fmt != '%')//如果当前的fmt字符不是%，则将该字符复制到p指向的位置，并将两个指针都向前移动一个字符
        {
            *p++ = *fmt++;
            continue;
        }
        fmt++;
        switch (*fmt)
        {
        case 'x'://16进制
            p = numberk(p, *((long *)next_arg), 16);
            next_arg += sizeof(long);
            fmt++;
            break;

        case 'd'://10进制
            p = numberk(p, *((long *)next_arg), 10);
            next_arg += sizeof(long);
            fmt++;
            break;

        case 's':
            p = strcopy(p, (char_t *)(*((long *)next_arg)));
            next_arg += sizeof(long);
            fmt++;
            break;

        default:
            break;
        }
    }
    *p = 0;
    return;
}





void kprint(const char_t *fmt, ...)
{
    char_t buf[512];
    va_list_t args = (va_list_t)((char_t *)(&fmt) + sizeof(long));//这里arg指向了第一个可变参数

    vsprintfk(buf, fmt, args);
    _strwrite(buf, &curs);
    return;
}