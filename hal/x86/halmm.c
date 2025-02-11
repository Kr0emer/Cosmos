#include "cosmostypes.h"
#include "cosmosmctrl.h"
void phymmarge_t_init(phymmarge_t *initp)
{
    if (NULL == initp)
    {
        return;
    }
    hal_spinlock_init(&initp->pmr_lock);
    initp->pmr_type = 0;
    initp->pmr_stype = 0;
    initp->pmr_dtype = 0;
    initp->pmr_flgs = 0;
    initp->pmr_stus = 0;
    initp->pmr_saddr = 0;
    initp->pmr_lsize = 0;
    initp->pmr_end = 0;
    initp->pmr_rrvmsaddr = 0;
    initp->pmr_rrvmend = 0;
    initp->pmr_prip = NULL;
    initp->pmr_extp = NULL;
    return;
}

void ret_phymmarge_adrandsz(machbstart_t *mbsp, phymmarge_t **retpmrvadr, u64_t *retpmrsz)
{
    // 检查输入参数是否有效
    // 如果 mbsp 为 NULL，或者 mbsp->mb_e820sz 为 0，或者 mbsp->mb_e820padr 为 NULL，或者 mbsp->mb_e820nr 为 0
    // 则认为没有有效的物理内存区域，返回 0 和 NULL
    if (NULL == mbsp || 0 == mbsp->mb_e820sz || NULL == mbsp->mb_e820padr || 0 == mbsp->mb_e820nr)
    {
        *retpmrsz = 0;  // 设置物理内存区域的大小为 0
        *retpmrvadr = NULL;  // 设置物理内存区域的地址为 NULL
        return;  // 返回
    }

    // 计算物理内存区域结构体数组的大小
    // tmpsz 表示所有phymmarge_ts数组总大小，单位是字节
    u64_t tmpsz = mbsp->mb_e820nr * sizeof(phymmarge_t);

    // 获取物理内存区域的起始地址
    u64_t tmpphyadr = mbsp->mb_nextwtpadr;

    // 检查物理地址是否合法
    // 调用 initchkadr_is_ok 函数验证该物理地址是否有效
    if (0 != initchkadr_is_ok(mbsp, tmpphyadr, tmpsz))
    {
        *retpmrsz = 0;  // 如果地址不合法，设置大小为 0
        *retpmrvadr = NULL;  // 设置地址为 NULL
        return;  // 返回
    }

    // 如果地址合法，返回物理内存区域的大小和地址
    *retpmrsz = tmpsz;  // 设置物理内存区域大小
    *retpmrvadr = (phymmarge_t *)((adr_t)tmpphyadr);  // 设置物理内存区域的地址
    return;  // 返回
}


bool_t init_one_pmrge(e820map_t *e8p, phymmarge_t *pmargep)
{
    u32_t ptype = 0, pstype = 0;

    // 检查输入参数是否有效
    // 如果 e8p 或 pmargep 为 NULL，则返回 FALSE，表示初始化失败
    if (NULL == e8p || NULL == pmargep)
    {
        return FALSE;
    }

    // 初始化 pmargep 结构体
    phymmarge_t_init(pmargep);

    // 根据 e8p->type 设置物理内存区域的类型和状态类型
    switch (e8p->type)
    {
    case RAM_USABLE:
        ptype = PMR_T_OSAPUSERRAM;  // 可用的物理内存区域
        pstype = RAM_USABLE;        // 可用内存
        break;
    case RAM_RESERV:
        ptype = PMR_T_RESERVRAM;    // 保留内存区域
        pstype = RAM_RESERV;        // 保留内存
        break;
    case RAM_ACPIREC:
        ptype = PMR_T_HWUSERRAM;   // ACPI 恢复内存区域
        pstype = RAM_ACPIREC;      // ACPI 恢复内存
        break;
    case RAM_ACPINVS:
        ptype = PMR_T_HWUSERRAM;   // ACPI 无效内存区域
        pstype = RAM_ACPINVS;      // ACPI 无效内存
        break;
    case RAM_AREACON:
        ptype = PMR_T_BUGRAM;      // 坏的内存
        pstype = RAM_AREACON;      // 坏的内存
        break;
    default:
        break;
    }

    // 如果 ptype 为 0，表示没有有效的物理内存类型，则返回 FALSE
    if (0 == ptype)
    {
        return FALSE;
    }

    // 设置 pmargep 结构体的各项参数
    pmargep->pmr_type = ptype;        // 设置物理内存区域类型
    pmargep->pmr_stype = pstype;      // 设置物理内存区域状态类型
    pmargep->pmr_flgs = PMR_F_X86_64; // 设置标志，表示这是 64 位 x86 架构的内存
    pmargep->pmr_saddr = e8p->saddr;  // 设置物理内存的起始地址
    pmargep->pmr_lsize = e8p->lsize;  // 设置物理内存区域的大小
    pmargep->pmr_end = e8p->saddr + e8p->lsize - 1; // 计算物理内存区域的结束地址

    return TRUE; // 返回 TRUE，表示初始化成功
}

// 4 5 6 2 3 8 1

// - + + - - + -

void phymmarge_swap(phymmarge_t *s, phymmarge_t *d)
{
    // 定义一个临时变量 tmp，用于交换数据
    phymmarge_t tmp;

    // 初始化临时变量 tmp
    phymmarge_t_init(&tmp);

    // 使用 memcopy 将 s 所指向的内容复制到 tmp
    memcopy(s, &tmp, sizeof(phymmarge_t));

    // 使用 memcopy 将 d 所指向的内容复制到 s
    memcopy(d, s, sizeof(phymmarge_t));

    // 使用 memcopy 将 tmp 中的内容（原 s 的内容）复制到 d
    memcopy(&tmp, d, sizeof(phymmarge_t));

    return;  // 完成交换后返回
}

void phymmarge_sort(phymmarge_t *argp, u64_t nr)
{
    u64_t i, j, k = nr - 1;  // i, j 用于循环控制，k 用于表示倒数第二个元素的索引

    // 外层循环控制排序的轮次，共需进行 nr - 1 轮比较
    for (j = 0; j < k; j++)
    {
        // 内层循环逐个比较相邻的元素
        for (i = 0; i < k - j; i++)
        {
            // 如果当前元素的起始地址大于下一个元素的起始地址，则交换它们
            if (argp[i].pmr_saddr > argp[i + 1].pmr_saddr)
            {
                // 调用 phymmarge_swap 函数交换两个元素
                phymmarge_swap(&argp[i], &argp[i + 1]);
            }
        }
    }

    return;  // 排序完成后返回
}


u64_t initpmrge_core(e820map_t *e8sp, u64_t e8nr, phymmarge_t *pmargesp)
{
    u64_t retnr = 0;  // 用于统计成功初始化的物理内存区域数量

    // 检查输入参数是否有效
    if (NULL == e8sp || NULL == pmargesp || e8nr < 1)
    {
        return 0;  // 如果任何参数无效，则返回0，表示没有成功初始化
    }

    // 遍历e8nr个物理内存区域
    for (u64_t i = 0; i < e8nr; i++)
    {
        // 调用init_one_pmrge函数初始化单个物理内存区域
        if (init_one_pmrge(&e8sp[i], &pmargesp[i]) == FALSE)
        {
            return retnr;  // 如果初始化失败，则返回当前已成功初始化的数量
        }
        retnr++;  // 成功初始化一个物理内存区域，增加计数器
    }

    return retnr;  // 返回成功初始化的物理内存区域数量
}


// 初始化物理内存区域
void init_phymmarge()
{
    machbstart_t *mbsp = &kmachbsp;  // 获取机器启动信息结构体的指针
    phymmarge_t *pmarge_adr = NULL;  // 初始化物理内存区域的地址指针
    u64_t pmrgesz = 0;  // 初始化物理内存区域的大小

    // 调用ret_phymmarge_adrandsz函数，根据机器信息结构获取物理内存区域的地址和大小
    ret_phymmarge_adrandsz(mbsp, &pmarge_adr, &pmrgesz);

    // 如果物理内存区域的地址为空或者大小为0，输出错误信息并返回
    if (NULL == pmarge_adr || 0 == pmrgesz)
    {
        system_error("init_phymmarge->NULL==pmarge_adr||0==pmrgesz\n");
        return;
    }

    u64_t tmppmrphyadr = mbsp->mb_nextwtpadr;  // 获取物理内存区域的起始物理地址

    // 如果物理内存区域的起始地址与预期的地址不一致，输出错误信息并返回
    if ((adr_t)tmppmrphyadr != ((adr_t)pmarge_adr))
    {
        system_error("init_phymmarge->tmppmrphyadr!=pmarge_adr2phyadr\n");
        return;
    }

    // 获取e820内存映射信息
    e820map_t *e8p = (e820map_t *)((adr_t)(mbsp->mb_e820padr));

    // 初始化物理内存区域，返回初始化的物理内存区域数量
    u64_t ipmgnr = initpmrge_core(e8p, mbsp->mb_e820nr, pmarge_adr);

    // 如果物理内存区域初始化失败，返回数量为0，输出错误信息并返回
    if (ipmgnr == 0)
    {
        system_error("init_phymmarge->initpmrge_core ret 0\n");
        return;
    }

    // 如果物理内存区域的数量与期望的大小不一致，输出错误信息并返回
    if ((ipmgnr * sizeof(phymmarge_t)) != pmrgesz)
    {
        system_error("init_phymmarge->ipmgnr*sizeof(phymmarge_t))!=pmrgesz\n");
        return;
    }

    // 将物理内存区域信息保存到机器启动信息结构体中
    mbsp->mb_e820expadr = tmppmrphyadr;  // 物理内存区域的起始地址
    mbsp->mb_e820exnr = ipmgnr;  // 物理内存区域的数量
    mbsp->mb_e820exsz = ipmgnr * sizeof(phymmarge_t);  // 物理内存区域的总大小
    mbsp->mb_nextwtpadr = PAGE_ALIGN(mbsp->mb_e820expadr + mbsp->mb_e820exsz);  // 更新物理内存区域后续地址

    // 对物理内存区域进行排序，确保地址从低到高
    phymmarge_sort(pmarge_adr, ipmgnr);
    return;
}

void init_halmm()
{
    init_phymmarge();
    init_memmgr();
    return;
}