#ifndef _IO_H
#define _IO_H


KLINE void out_u8(const u16_t port, const u8_t val)//向port写入一字节val
{

    __asm__ __volatile__("outb  %1, %0\n"
             :
             : "dN"(port), "a"(val));
}

KLINE u8_t in_u8(const u16_t port)//从port读入一字节
{
    u8_t tmp;
    __asm__ __volatile__("inb %1, %0\n"
             : "=a"(tmp)
             : "dN"(port));
    return tmp;
}


KLINE void out_u16(const u16_t port, const u16_t val)//向port写入一字val
{

    __asm__ __volatile__("outw  %1, %0\n"
             :
             : "dN"(port), "a"(val));
}


KLINE u16_t in_u16(const u16_t port)//从port读入一字
{
    u16_t tmp;

    __asm__ __volatile__("inw %1, %0\n"
             : "=a"(tmp)
             : "dN"(port));
    return tmp;
};

//内存拷贝函数
KLINE sint_t m2mcopy(void *sadr,void *dadr ,sint_t len)
{
    //如果源地址或目标地址为NULL，或者长度小于1，则不进行复制，返回0。
    if (sadr == NULL || dadr == NULL || len < 1)
    {
        return 0;
    }
    u8_t *s = (u8_t *)sadr,*d = (u8_t *)dadr;//将sadr和dadr的地址类型转化为逐字节的u8_t便于复制
    //如果源地址小于目的地址，从后往前复制，防止重叠的时候数据覆盖
    if (s < d)
    {
        for (sint_t i = (len - 1); i>=0 ; i--)
        {
            d[i] = s[i];
        }
        return len;
    }
    //如果源地址大于目的地址，从前向后复制
    if (s > d)
    {
        for (sint_t j = 0 ; j < len ; j++)
        {
            d[j] = s[j];
        }
        return len;
    }
    if (s == d)
    {
        return len;
    }
    //如果以上条件都不满足则返回0
    return 0;
}
KLINE void memset(void* src,u8_t val,uint_t count)
{

    u8_t* ss=src;
    for(uint_t i=0;i<count;i++)
    {
        ss[i]=val;
    }
    return;
}


#endif //_IO_H