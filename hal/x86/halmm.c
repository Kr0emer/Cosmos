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
    // tmpsz 表示所有物理内存区域的总大小，单位是字节
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
    if (NULL == e8p || NULL == pmargep)
    {
        return FALSE;
    }
    phymmarge_t_init(pmargep);
    switch (e8p->type)
    {
    case RAM_USABLE:
        ptype = PMR_T_OSAPUSERRAM;
        pstype = RAM_USABLE;
        break;
    case RAM_RESERV:
        ptype = PMR_T_RESERVRAM;
        pstype = RAM_RESERV;
        break;
    case RAM_ACPIREC:
        ptype = PMR_T_HWUSERRAM;
        pstype = RAM_ACPIREC;
        break;
    case RAM_ACPINVS:
        ptype = PMR_T_HWUSERRAM;
        pstype = RAM_ACPINVS;
        break;
    case RAM_AREACON:
        ptype = PMR_T_BUGRAM;
        pstype = RAM_AREACON;
        break;
    default:
        break;
    }
    if (0 == ptype)
    {
        return FALSE;
    }
    pmargep->pmr_type = ptype;
    pmargep->pmr_stype = pstype;
    pmargep->pmr_flgs = PMR_F_X86_64;
    pmargep->pmr_saddr = e8p->saddr;
    pmargep->pmr_lsize = e8p->lsize;
    pmargep->pmr_end = e8p->saddr + e8p->lsize - 1;
    return TRUE;
}
// 4 5 6 2 3 8 1

// - + + - - + -

void phymmarge_swap(phymmarge_t *s, phymmarge_t *d)
{
    phymmarge_t tmp;
    phymmarge_t_init(&tmp);
    memcopy(s, &tmp, sizeof(phymmarge_t));
    memcopy(d, s, sizeof(phymmarge_t));
    memcopy(&tmp, d, sizeof(phymmarge_t));
    return;
}

void phymmarge_sort(phymmarge_t *argp, u64_t nr)
{
    u64_t i, j, k = nr - 1;
    for (j = 0; j < k; j++)
    {
        for (i = 0; i < k - j; i++)
        {
            if (argp[i].pmr_saddr > argp[i + 1].pmr_saddr)
            {
                phymmarge_swap(&argp[i], &argp[i + 1]);
            }
        }
    }
    return;
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
    return;
}