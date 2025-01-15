#include "cosmostypes.h"
#include "cosmosmctrl.h"

void kprint(const char_t *fmt, ...)
{
	char_t buf[512];
	va_list ap;
	va_start(ap, fmt);
	vsprintfk(buf, fmt, ap);
	gstr_write(&kdftgh, buf);
	va_end(ap);
	return;
}


// 将字符串从 str_s 复制到 buf
// 参数：
// buf - 目标缓冲区指针
// str_s - 源字符串指针
// 返回值：复制操作后 buf 的末尾位置
char_t *strcopyk(char_t *buf, char_t *str_s)
{
    // 遍历源字符串，逐字符复制到目标缓冲区
    while (*str_s)
    {
        *buf = *str_s; // 复制字符
        buf++;         // 移动目标指针
        str_s++;       // 移动源指针
    }
    return buf; // 返回复制后的缓冲区末尾位置
}

// 格式化字符串并写入缓冲区
// 参数：
// buf - 目标缓冲区指针
// fmt - 格式化字符串
// args - 可变参数列表
void vsprintfk(char_t *buf, const char_t *fmt, va_list args)
{
    char_t *p = buf; // 指向缓冲区的指针
    // 遍历格式化字符串
    while (*fmt)
    {
        if (*fmt != '%') // 如果当前字符不是格式符
        {
            *p++ = *fmt++; // 直接写入到缓冲区
            continue;
        }
        fmt++; // 跳过 '%'
        switch (*fmt)
        {
        case 'x': // 十六进制格式
            p = numberk(p, va_arg(args, uint_t), 16); // 调用 numberk 格式化数字
            fmt++; // 跳过格式符
            break;
        case 'd': // 十进制格式
            p = numberk(p, va_arg(args, uint_t), 10); // 调用 numberk 格式化数字
            fmt++; // 跳过格式符
            break;
        case 's': // 字符串格式
            p = strcopyk(p, (char_t *)va_arg(args, uint_t)); // 调用 strcopyk 复制字符串
            fmt++; // 跳过格式符
            break;
        default: // 不支持的格式符，直接跳过
            break;
        }
    }
    *p = 0; // 在缓冲区末尾添加字符串结束符
    return;
}

// 将数字格式化为字符串
// 参数：
// str - 目标缓冲区指针
// n - 要格式化的数字
// base - 数字的进制（10 或 16）
// 返回值：格式化后的字符串末尾位置
char_t *numberk(char_t *str, uint_t n, sint_t base)
{
    register char_t *p;      // 临时指针
    char_t strbuf[36];       // 临时缓冲区，存储数字字符串
    p = &strbuf[36];         // 指向缓冲区末尾
    *--p = 0;                // 添加字符串结束符

    if (n == 0) // 如果数字为 0
    {
        *--p = '0'; // 直接添加字符 '0'
    }
    else // 如果数字非零
    {
        do
        {
            // 根据余数从字符表中取出对应的字符
            *--p = "0123456789abcdef"[n % base];
        } while (n /= base); // 不断除以进制，直到数字为 0
    }

    // 将格式化后的数字字符串复制到目标缓冲区
    while (*p != 0)
    {
        *str++ = *p++;
    }

    return str; // 返回缓冲区末尾位置
}
