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
    mbsp->mb_e820padr = (u64_t)((u32_t)(retemp));//把e820map_t结构数组的首地址传给mbsp->mb_e820padr
    mbsp->mb_e820nr = (u64_t)retemnr;//把e820map_t结构数组元素个数传给mbsp->mb_e820nr
    mbsp->mb_e820sz = retemnr * (sizeof(e820map_t));//把e820map_t结构数组大小传给mbsp->mb_e820sz
    mbsp->mb_memsz = get_memsize(retemp, retemnr);//根据e820map_t结构数据计算内存大小。
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



u64_t get_memsize(e820map_t *e8p, u32_t enr)//对所有内存求和获取总内存
{
    u64_t len = 0;
    if (enr == 0 || e8p == NULL)
    {
        return 0;
    }
    for (u32_t i = 0; i < enr; i++)
    {
        if (e8p[i].type == RAM_USABLE)
        {
            len += e8p[i].lsize;
        }
    }
    return len;
}

PUBLIC void init_acpi(machbstart_t *mbsp)
{
    mrsdp_t *rdp = NULL;
    rdp = find_acpi_rsdp();//尝试通过 find_acpi_rsdp 函数查找 ACPI 的根系统描述指针
    if (NULL == rdp)//检测指针是否存在
    {
        kerror("Your computer is not support ACPI!!");
    }
    m2mcopy(rdp, &mbsp->mb_mrsdp, (sint_t)((sizeof(mrsdp_t))));//将结构体复制过去
    if (acpi_rsdp_isok(&mbsp->mb_mrsdp) == NULL)//函数检查复制的数据是否有效
    {
        kerror("Your computer is not support ACPI!!");
    }
    return;
}

PUBLIC mrsdp_t *find_acpi_rsdp()
{

    void *fndp = (void *)acpi_get_bios_ebda();//获取BIOS的Extended BIOS Data Area (EBDA) 的地址
    mrsdp_t *rdp = findacpi_rsdp_core(fndp, 1024);//在EBDA中搜索RSDP，会检查给定的内存范围（在这里是1024字节）以查找RSDP的签名
    if (NULL != rdp)//如果在EBDA中找到了RSDP，函数就会返回RSDP的地址
    {
        return rdp;
    }
    //0E0000h和0FFFFFH
    fndp = (void *)(0xe0000);
    rdp = findacpi_rsdp_core(fndp, (0xfffff - 0xe0000));//如果在EBDA中没有找到RSDP，函数会继续在0xE0000到0xFFFFF的内存范围内搜索
    if (NULL != rdp)
    {
        return rdp;
    }
    return NULL;//如果两个范围都没有找到RSDP，函数最终会返回NULL
}

mrsdp_t *findacpi_rsdp_core(void *findstart, u32_t findlen)
{
    if (NULL == findstart || 1024 > findlen)//函数首先检查findstart是否为NULL，以及findlen是否小于1024字节。如果任一条件为真，函数立即返回NULL，因为这意味着没有足够的数据来查找RSDP
    {
        return NULL;
    }

    u8_t *tmpdp = (u8_t *)findstart;//将findstart指针从void*转换为u8_t*，这样可以按字节操作内存

    mrsdp_t *retdrp = NULL;
    for (u64_t i = 0; i <= findlen; i++)//函数使用一个循环来遍历从findstart开始的findlen字节的内存区域。循环变量i从0开始，直到findlen
    {

        if (('R' == tmpdp[i]) && ('S' == tmpdp[i + 1]) && ('D' == tmpdp[i + 2]) && (' ' == tmpdp[i + 3]) &&
            ('P' == tmpdp[i + 4]) && ('T' == tmpdp[i + 5]) && ('R' == tmpdp[i + 6]) && (' ' == tmpdp[i + 7]))//在每个循环迭代中，函数检查当前位置的8个连续字节是否匹配RSDP的签名。RSDP的签名是"RSD PTR "，包括空格
        {
            retdrp = acpi_rsdp_isok((mrsdp_t *)(&tmpdp[i]));//验证找到的RSDP是否有效
            if (NULL != retdrp)
            {
                return retdrp;//如果验证通过，返回指向有效RSDP的指针
            }
        }
    }
    return NULL;
}

mrsdp_t *acpi_rsdp_isok(mrsdp_t *rdp)
{

    if (rdp->rp_len == 0 || rdp->rp_revn == 0)// 检查RSDP的长度和修订版本是否为0，如果是，则返回NULL
    {
        return NULL;
    }
    if (0 == acpi_checksum((unsigned char *)rdp, (s32_t)rdp->rp_len))// 使用acpi_checksum函数来计算RSDP的校验和
    {

        return rdp;//如果校验和为0，说明RSDP没有错误，函数返回RSDP的指针
    }

    return NULL;// 如果校验和不为0，说明RSDP有错误，函数返回NULL
}

int acpi_checksum(unsigned char *ap, s32_t len)//计算校验和
{
    int sum = 0;
    while (len--)
    {
        sum += *ap++;
    }
    return sum & 0xFF;
}