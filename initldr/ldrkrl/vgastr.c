/**********************************************************
		转换显示字符串文件vgastr.c
***********************************************************/


#include "cmctl.h"
__attribute__((section(".data"))) cursor_t curs;

void init_curs()//初始化cursor_t结构体
{

    curs.vmem_s = VGASTR_RAM_BASE;
    curs.vmem_e = VGASTR_RAM_END;
    curs.cvmemadr = 0;
    curs.x = 0;
    curs.y = 0;
    return;
}

void close_curs()
{
    out_u8(VGACTRL_REG_ADR, VGACURS_REG_INX);//VGA 控制器的地址寄存器发送一个值，该值指定了接下来要操作的寄存器的索引+
    out_u8(VGACTRL_REG_DAT, VGACURS_CLOSE);//向 VGA 控制器的数据寄存器发送一个值，以执行关闭光标
    return;
}

void clear_screen(u16_t srrv)
{
    curs.x = 0;
    curs.y = 0;//光标位置重置为屏幕的左上角

    u16_t *p = (u16_t *)VGASTR_RAM_BASE;//将指针化为指向 VGA 文本模式内存的基地址

    for (uint_t i = 0; i < 2001; i++)
    {
        p[i] = srrv;
    }//将屏幕上的每个字符位置设置为 srrv 的值

    close_curs();//在清除屏幕后，调用 close_curs 函数来关闭光标，以防止在屏幕更新时光标闪烁
    return;
}

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
    GxH_strwrite(buf, &curs);
    return;
}

void GxH_strwrite(char_t *str, cursor_t *cursptr)
{

    uint_t straddr = cursptr->x + cursptr->y * 80 * 2;// 计算字符串应该开始写入的内存地址
    char_t *p_strdst = (char_t *)(cursptr->vmem_s + straddr);// 将计算出的地址转换为字符指针，准备写入字符
    u32_t tfindx = FALSE;// 定义一个标志变量，用于检测是否遇到换行符
    while (*str)
    {

        if (*str == 10)// 如果当前字符是换行符（ASCII值为10)
        {
            tfindx = TRUE;// 设置标志变量为TRUE
            str++;// 移动到下一个字符
            if (*str == 0)// 如果下一个字符是字符串结束符，则退出循环
            {
                break;
            }
        }

        current_curs(cursptr, VGACHAR_DF_CFLG);// 更新光标位置

        *p_strdst = *str++;// 将当前字符写入屏幕
        p_strdst += 2;// 移动到下一个字符的位置（VGA文本模式下，每个字符占两个字节）
    }

    if (tfindx == TRUE)// 如果遇到换行符，更新光标位置
    {
        current_curs(cursptr, VGACHAR_LR_CFLG);
    }

    return;
}



