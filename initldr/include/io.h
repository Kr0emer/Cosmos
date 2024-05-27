#ifndef _IO_H
#define _IO_H


KLINE void out_u8(const u16_t port, const u8_t val)//port 表示要写入数据的硬件端口号 val 表示要发送到端口的值
{

    __asm__ __volatile__("outb  %1, %0\n"//用于将数据从CPU发送到硬件端口
             :
             : "dN"(port), "a"(val));
}

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