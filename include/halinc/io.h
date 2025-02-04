/**********************************************************
		输入/输出头文件io.h
***********************************************************
				
**********************************************************/
#ifndef _IO_H
#define _IO_H


/*D7~D5 固定为0    D4 固定为1 ICW1标识     D3 0 边沿触发 1 电平触发        D2 x86无需设置      D1 1表示单片 0表示级联     D0  IC4 ICW4是否启用 1启用*/
//写入端口 0x20 0xA0
#define ICW1 0x11 //0001 0001  边沿触发 级联 需要ICW4 

/*D7~D3  中断向量号的高 5 位，低 3 位由 IRQ 编号自动填充    D2-D0 由硬件自动填充为 IRQ 号*/
//写入端口 0x21 0xA1
#define ZICW2 0x20
#define SICW2 0x28

/*主片标记哪些 IRQ 连接了从片，主片是位图；从片声明自己连接到主片的哪个 IRQ，从片低三位表示*/
//写入端口 0x21 0xA1
#define ZICW3 0x04//IRQ2链接从片
#define SICW3 0x02//连主片IRQ2

//配置中断结束方式、缓冲模式、特殊全嵌套模式等
/*D7~D5：保留（置 0） D4 (SFNM)：特殊全嵌套模式（1=启用，仅主片需配置以支持从片中断嵌套）  D3 (BUF)：缓冲模式（1=启用，用于总线缓冲，需配合 D2 使用）   
D2 (M/S)：缓冲模式下主/从片标识（BUF=1 时有效：1=主片，0=从片） D1 (AEOI)：自动结束中断（1=自动发送 EOI，慎用，可能破坏中断优先级）   
D0 (μPM)：微处理器模式（1=8086/88，必须设为 1；0=8080/85）*/
#define ICW4 0x01//微处理器模式

#define ZIOPT 0x20
#define ZIOPT1 0x21

#define SIOPT 0xA0
#define SIOPT1 0xA1

#define _INTM_CTL 0x20     // I/O port for interrupt controller         <Master>
#define _INTM_CTLMASK 0x21 // setting bits in this port disables ints   <Master>
#define _INTS_CTL 0xA0     //; I/O port for second interrupt controller  <Slave>
#define _INTS_CTLMASK 0xA1 //; setting bits in this port disables ints   <Slave>


/*向偶地址写入若D4=1则是写入ICW1，若 D4=0 且 D3=0则为OCW2，若为D4=0 且 D3=1则OCW3*/
/*D7 (R)：优先级循环模式，D6 (SL)：指定 IRQ 级别（需与 D7 配合使用） D5 (EOI)：中断结束命令（1=发送 EOI）
D2-D0 (L0-L2)：指定具体 IRQ 编号（仅当 SL=1 时有效）*/
#define _EOI 0x20 //0010 0000,写入OCW2，D5 = 1 发送 EOI

#define PTIPROT1 0x40
#define PTIPROT2 0x41
#define PTIPROT3 0x42
#define PTIPROTM 0x43

#define TIMEMODE 0x34      //;00-11-010-0
#define TIMEJISU 1194000UL //1193182UL
#define HZ 1000UL          //0x3e8

#define HZLL ((TIMEJISU / HZ) & 0xff)        //0xa9//0x9b     //;1001  1011
#define HZHH (((TIMEJISU / HZ) >> 8) & 0xff) //0x04//0x2e     //;0010  1110

#define NORETURN __attribute__((noreturn))
#define SYSRCALL __attribute__((regparm(3)))
#define HINTCALL __attribute__((regparm(2)))
#define FAUTCALL __attribute__((regparm(2)))

#define REGCALL __attribute__((regparm(3))) //让GCC使用(EAX, EDX, ECX)寄存器传递参数
#define __SAVE_FLAGS(x) __asm__ __volatile__("pushfl ; popl %0" \
                                             : "=g"(x)          \
                                             : /* no input */)
#define __RESTORE_FLAGS(x) __asm__ __volatile__("pushl %0 ; popfl" \
                                                : /* no output */  \
                                                : "g"(x)           \
                                                : "memory")
#define CLI() __asm__ __volatile__("cli" \
                                   :     \
                                   :     \
                                   : "memory")
#define STI() __asm__ __volatile__("sti" \
                                   :     \
                                   :     \
                                   : "memory")
/*used in he idle loop; sti takes one instruction cycle to complete */
#define HALT() __asm__ __volatile__("hlt" \
                                    :     \
                                    :     \
                                    : "memory")
#define STI_HALT() __asm__ __volatile__("sti; hlt" \
                                        :          \
                                        :          \
                                        : "memory")
#define CLI_HALT() __asm__ __volatile__("cli; hlt" \
                                        :          \
                                        :          \
                                        : "memory")

KLINE void out_u8(const u16_t port, const u8_t val)
{

    __asm__ __volatile__("outb  %1, %0\n"
                         :
                         : "dN"(port), "a"(val));
}

KLINE void out_u8_p(const u16_t port, const u8_t val)
{

    __asm__ __volatile__("outb  %1, %0\n\t"
                         "nop \n\t"
                         "nop \n\t"

                         :
                         : "dN"(port), "a"(val));
}

KLINE u8_t in_u8(const u16_t port)
{
    u8_t tmp;
    /* GCC can optimize here if constant */
    __asm__ __volatile__("inb %1, %0\n"
                         : "=a"(tmp)
                         : "dN"(port));
    return tmp;
}

KLINE void out_u16(const u16_t port, const u16_t val)
{

    __asm__ __volatile__("outw  %1, %0\n"
                         :
                         : "dN"(port), "a"(val));
}

KLINE u16_t in_u16(const u16_t port)
{
    u16_t tmp;

    __asm__ __volatile__("inw %1, %0\n"
                         : "=a"(tmp)
                         : "dN"(port));
    return tmp;
};

KLINE void out_u32(const u16_t port, const u32_t val)
{

    __asm__ __volatile__("outl  %1, %0\n"
                         :
                         : "dN"(port), "a"(val));
}

KLINE u32_t in_u32(const u16_t port)
{
    u32_t tmp;

    __asm__ __volatile__("inl %1, %0\n"
                         : "=a"(tmp)
                         : "dN"(port));
    return tmp;
}

KLINE u64_t read_msr(const u32_t reg)
{
    u32_t eax, edx;

    __asm__ __volatile__(
        "rdmsr"
        : "=a"(eax), "=d"(edx)
        : "c"(reg));

    return (((u64_t)edx) << 32) | (u64_t)eax;
}

KLINE void write_msr(const u32_t reg, const u64_t val)
{
    __asm__ __volatile__(
        "wrmsr"
        :
        : "a"((u32_t)val), "d"((u32_t)(val >> 32)), "c"(reg));
}

KLINE void memcopy(void *src, void *dest, uint_t count)
{

    u8_t *ss = src, *sd = dest;
    for (uint_t i = 0; i < count; i++)
    {
        sd[i] = ss[i];
    }
    return;
}

KLINE sint_t m2mcopy(void *sadr, void *dadr, sint_t len)
{
    if (NULL == sadr || NULL == dadr || 1 > len)
    {
        return 0;
    }
    u8_t *s = (u8_t *)sadr, *d = (u8_t *)dadr;
    if (s < d)
    {
        for (sint_t i = (len - 1); i >= 0; i--)
        {
            d[i] = s[i];
        }
        return len;
    }
    if (s > d)
    {
        for (sint_t j = 0; j < len; j++)
        {
            d[j] = s[j];
        }
        return len;
    }
    if (s == d)
    {
        return len;
    }
    return 0;
}

KLINE void memset(void *s, u8_t c, uint_t count)
{
    u8_t *st = s;
    for (uint_t i = 0; i < count; i++)
    {
        st[i] = c;
    }
    return;
}

KLINE void save_flags_cli(cpuflg_t *flags)
{
    __asm__ __volatile__(
        "pushfq \t\n"
        "cli    \t\n"
        "popq %0 \t\n"

        : "=m"(*flags)
        :
        : "memory");
    return;
}

KLINE void restore_flags_sti(cpuflg_t *flagsres)
{
    __asm__ __volatile__(
        "pushq %0 \t\n"
        "popfq \t\n"
        :
        : "m"(*flagsres)
        : "memory");
    return;
}

KLINE u64_t x86_rdpmc(const int ctrsel)
{
    u32_t eax, edx;

    __asm__ __volatile__(
        "rdpmc"
        : "=a"(eax), "=d"(edx)
        : "c"(ctrsel));

    return (((u64_t)edx) << 32) | (u64_t)eax;
}

KLINE u64_t x86_rdtsc(void)
{
    u32_t leax, ledx;

    __asm__ __volatile__(
        "rdtsc"
        : "=a"(leax), "=d"(ledx));

    return (((u64_t)ledx) << 32) | (u64_t)leax;
}

KLINE sint_t search_64rlbits(u64_t val)
{
    sint_t retbitnr = -1;
    __asm__ __volatile__(
        "bsrq %1,%q0 \t\n"
        : "+r"(retbitnr)
        : "rm"(val));
    return retbitnr + 1;
}

KLINE sint_t search_32rlbits(u32_t val)
{
    sint_t retbitnr = -1;
    __asm__ __volatile__(
        "bsrl %1,%0 \t\n"
        : "+r"(retbitnr)
        : "rm"(val));
    return retbitnr + 1;
}

KLINE u32_t read_kesp()
{
    u32_t esp;

    __asm__ __volatile__(
        "movl %%esp,%0"
        : "=g"(esp)
        :
        : "memory");

    return esp;
}

KLINE u32_t read_kcr2()
{
    u32_t cr2;

    __asm__ __volatile__(
        "movl %%cr2,%0"
        : "=g"(cr2)
        :
        : "memory");

    return cr2;
}
KLINE void set_cr3(u64_t pl4adr)
{
    __asm__ __volatile__(

        "movq %0,%%cr3 \n\t"
        :
        : "r"(pl4adr)
        : "memory" //, "edx"
    );
    return;
}

#endif
