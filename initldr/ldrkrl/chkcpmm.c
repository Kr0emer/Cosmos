#include "cmctl.h"



u64_t acpi_get_bios_ebda()
{
    u64_t address = *(unsigned short *)0x40E;
    address <<= 4;
    return address;
}

void mmap(e820map_t **retemp, u32_t *retemnr)
{
    realadr_call_entry(RLINTNR(0), 0, 0); //eax = 0 edx = 0 ecx = 0
    *retemnr = *((u32_t *)(E80MAP_NR));
    *retemp = (e820map_t *)(*((u32_t *)(E80MAP_ADRADR)));
    return;
}



//通过改写Eflags寄存器的第21位，观察其位的变化判断是否支持CPUID
int chk_cpuid()
{
    int rets = 0;
    __asm__ __volatile__(//__volatile__确保每次访问该变量时都直接从内存中读取或写入值，而不是从寄存器或其他缓存中获取。
        "pushfl \n\t"//EFLAGS寄存器压栈
        "pop %%eax \n\t"//将EFLAGS保存到eax中
        "movl %%eax,%%ebx \n\t"//将EAX寄存器的值复制到EBX寄存器中，用于之后的比较
        "xorl $0x0200000,%%eax \n\t"//对EAX寄存器的值进行异或操作，改变其第21位的值
        "pushl %%eax \n\t"//将修改后的EAX值压回栈中
        "popfl \n\t"//将栈顶的值弹出到EFLAGS寄存器中，更新EFLAGS的值
        "pushfl \n\t"//再次将EFLAGS的值压入栈中
        "popl %%eax \n\t"//将栈顶的值弹出到EAX寄存器中
        "xorl %%ebx,%%eax \n\t"//将EBX和EAX寄存器的值进行异或操作，用于检测EFLAGS的第21位是否发生了变化
        "jz 1f \n\t"//如果结果为0（即第21位没有变化），跳转到标签1
        "movl $1,%0 \n\t"//给rets赋值为0
        "jmp 2f \n\t"//跳转到2标签
        "1:movl $0,%0 \n\t"//给rets赋值为1
        "2: \n\t"//
        :"=c"(rets)
        :
        :
    );
    return rets;
}

//检查CPU是否支持长模式
int chk_cpu_longmode() {
    int rets = 0; // 定义一个变量rets，用来存储检测结果，初始化为0
    __asm__ __volatile__(
        "movl $0x80000000,%%eax \n\t" // 将0x80000000移动到EAX寄存器中
        "cpuid \n\t" // 执行CPUID指令，CPUID指令会根据EAX寄存器的值返回不同的信息，当EAX为0x80000000时，执行CPUID指令后，EAX寄存器会返回一个值，这个值是CPU支持的最大扩展功能号。如果这个返回值大于0x80000000，那么就表示CPU支持一系列扩展功能
        "cmpl $0x80000001,%%eax \n\t" // 比较EAX寄存器的值和0x80000001，如果EAX小于0x80000001，那么处理器不支持0x80000001号功能
        "setnb %%al \n\t" // Set Byte if Not Below（如果不低于则设置字节），如果EAX大于或等于0x80000001，将AL设置为1
        "jb 1f \n\t" // 如果EAX小于0x80000001，跳转到标签1
        "movl $0x80000001,%%eax \n\t" // 将0x80000001移动到EAX寄存器中
        "cpuid \n\t" // 再次执行CPUID指令，这次CPUID调用是为了获取扩展功能信息
        "bt $29,%%edx \n\t" // 检查EDX寄存器的第29位（长模式支持位），如果第29位为1，表示支持长模式
        "setcb %%al \n\t" // 如果第29位为1，将AL设置为1
        "1: \n\t" // 标签1
        "movzx %%al,%%eax \n\t" // 将AL的值扩展到EAX寄存器中
        : "=a"(rets)
        : 
        : 
    );
    return rets; // 返回rets变量的值
}

void init_chkcpu(machbstart_t *mbsp)
{
    if(!chk_cpuid())//检测cpuid功能是否支持
    {
        kprint("Your CPU is not support CPUID sys is die!");
        CLI_HALT();

    }
    if (!chk_cpu_longmode())//检测是否支持长模式
    {
        kprint("Your CPU is not support 64bits mode sys is die!");
        CLI_HALT();
    }
    mbsp->mb_cpumode = 64;//如果成功则设置机器信息结构的cpu模式为64位
    return;
}



void init_krlinitstack(machbstart_t *mbsp)
{
    if (1 > move_krlimg(mbsp, (u64_t)(0x8f000), 0x1001))
    {
        kerror("iks_moveimg err");
    }
    mbsp->mb_krlinitstack = IKSTACK_PHYADR;
    mbsp->mb_krlitstacksz = IKSTACK_SIZE;
    return;
}

void init_mem(machbstart_t *mbsp)
{
    e820map_t *retemp;
    u32_t retemnr = 0;
    mbsp->mb_ebdaphyadr = acpi_get_bios_ebda();//获取 EBDA 的物理地址并存储在 mbsp->mb_ebdaphyadr 中
    mmap(&retemp, &retemnr);// 调用 mmap 函数获取 e820map_t 结构体的信息
    if (retemnr == 0)// 检查是否成功获取了 e820map
    {
        kerror("no e820map\n");
    }
    if (chk_memsize(retemp, retemnr, 0x100000, 0x8000000) == NULL)// 检查内存大小是否满足要求
    {
        kerror("Your computer is low on memory, the memory cannot be less than 128MB!");
    }
    mbsp->mb_e820padr = (u64_t)((u32_t)(retemp));
    mbsp->mb_e820nr = (u64_t)retemnr;
    mbsp->mb_e820sz = retemnr * (sizeof(e820map_t));
    mbsp->mb_memsz = get_memsize(retemp, retemnr);
    init_acpi(mbsp);//调用 init_acpi 函数进行 ACPI 初始化
    return;
}


e820map_t *chk_memsize(e820map_t *e8p, u32_t enr, u64_t sadr, u64_t size)
{
    u64_t len = sadr + size;//// 计算结束地址
    if (enr == 0 || e8p == NULL)// 如果内存映射条目数为0或者内存映射指针为空，则返回NULL
    {
        return NULL;
    }
    for (u32_t i = 0; i < enr; i++)// 遍历所有内存映射条目
    {
        if (e8p[i].type == RAM_USABLE)// 如果当前条目类型为可用RAM
        {
            if ((sadr >= e8p[i].saddr) && (len < (e8p[i].saddr + e8p[i].lsize)))// 如果指定的起始地址大于等于当前条目的起始地址,并且结束地址小于当前条目的起始地址加上长度
            {
                return &e8p[i];// 则返回当前内存映射条目的指针
            }
        }
    }
    return NULL;// 如果没有找到符合条件的内存映射条目，则返回NULL
}
