#include "cosmostypes.h"
#include "cosmosmctrl.h"


bool_t ret_msadsc_vadrandsz(machbstart_t *mbsp, msadsc_t **retmasvp, u64_t *retmasnr)
{
    // 检查传入参数是否为空
    if (NULL == mbsp || NULL == retmasvp || NULL == retmasnr)
    {
        return FALSE;  // 如果有空指针，返回FALSE
    }
    
    // 检查e820扩展区的信息是否有效
    if (mbsp->mb_e820exnr < 1 || NULL == mbsp->mb_e820expadr || (mbsp->mb_e820exnr * sizeof(phymmarge_t)) != mbsp->mb_e820exsz)
    {
        *retmasvp = NULL;  // 如果无效，设置返回指针为NULL
        *retmasnr = 0;     // 设置返回数量为0
        return FALSE;      // 返回FALSE
    }
    
    // 将物理地址转换为虚拟地址
    phymmarge_t *pmagep = (phymmarge_t *)phyadr_to_viradr((adr_t)mbsp->mb_e820expadr);
    
    u64_t usrmemsz = 0, msadnr = 0;
    
    // 遍历e820扩展区的所有内存区域
    for (u64_t i = 0; i < mbsp->mb_e820exnr; i++)
    {
        // 寻找"可用的物理内存区域"的区域
        if (PMR_T_OSAPUSERRAM == pmagep[i].pmr_type)
        {
            usrmemsz += pmagep[i].pmr_lsize;   // 累加用户内存大小
            msadnr += (pmagep[i].pmr_lsize >> 12); // 累加对应页数（假设页大小为4KB）
        }
    }
    
    // 检查计算的用户内存是否有效
    if (0 == usrmemsz || (usrmemsz >> 12) < 1 || msadnr < 1)
    {
        *retmasvp = NULL;  // 如果无效，设置返回指针为NULL
        *retmasnr = 0;     // 设置返回数量为0
        return FALSE;      // 返回FALSE
    }

    // 检查内存区域是否有效
    if (0 != initchkadr_is_ok(mbsp, mbsp->mb_nextwtpadr, (msadnr * sizeof(msadsc_t))))
    {
        // 如果无效，输出系统错误
        system_error("ret_msadsc_vadrandsz initchkadr_is_ok err\n");
    }

    // 将物理地址转换为虚拟地址并赋值给返回指针
    *retmasvp = (msadsc_t *)phyadr_to_viradr((adr_t)mbsp->mb_nextwtpadr);
    *retmasnr = msadnr;  // 返回计算的页数
    return TRUE;  // 返回TRUE表示成功
}


// 初始化内存页描述符（msadsc_t 结构体）
void msadsc_t_init(msadsc_t *initp)
{
    // 初始化链表节点（用于将描述符链接到内存管理数据结构中）
    list_init(&initp->md_list);

    // 初始化自旋锁（保护该描述符的并发访问）
    knl_spinlock_init(&initp->md_lock);

    // 初始化标志位字段：md_indxflgs
    initp->md_indxflgs.mf_olkty = MF_OLKTY_INIT;  // 内存页的“所有者类型”初始状态
    initp->md_indxflgs.mf_lstty = MF_LSTTY_LIST;   // 内存页的“链表类型”标记（如空闲链表）
    initp->md_indxflgs.mf_mocty = MF_MOCTY_FREE;   // 内存页的“管理操作类型”标记为“空闲”
    initp->md_indxflgs.mf_marty = MF_MARTY_INIT;   // 内存页的“映射类型”初始状态
    initp->md_indxflgs.mf_uindx = MF_UINDX_INIT;   // 用户自定义索引或标记初始值

    // 初始化物理地址状态字段：md_phyadrs
    initp->md_phyadrs.paf_alloc = PAF_NO_ALLOC;    // 页未被分配
    initp->md_phyadrs.paf_shared = PAF_NO_SHARED;  // 页未被共享
    initp->md_phyadrs.paf_swap = PAF_NO_SWAP;      // 页未被交换到磁盘
    initp->md_phyadrs.paf_cache = PAF_NO_CACHE;    // 页未被缓存
    initp->md_phyadrs.paf_kmap = PAF_NO_KMAP;      // 页未映射到内核地址空间
    initp->md_phyadrs.paf_lock = PAF_NO_LOCK;      // 页未被锁定
    initp->md_phyadrs.paf_dirty = PAF_NO_DIRTY;    // 页未被标记为“脏”
    initp->md_phyadrs.paf_busy = PAF_NO_BUSY;      // 页未被标记为“忙”
    initp->md_phyadrs.paf_rv2 = PAF_RV2_VAL;       // 保留字段2的默认值
    initp->md_phyadrs.paf_padrs = PAF_INIT_PADRS;  // 物理地址的初始值

    // 初始化其他字段
    initp->md_odlink = NULL;  // 对象依赖链接指针（如关联的进程或驱动对象）初始化为空

    return;
}

void write_one_msadsc(msadsc_t *msap, u64_t phyadr)
{
    msadsc_t_init(msap);
    phyadrflgs_t *tmp = (phyadrflgs_t *)(&phyadr);
    msap->md_phyadrs.paf_padrs = tmp->paf_padrs;
    return;
}

u64_t init_msadsc_core(machbstart_t *mbsp, msadsc_t *msavstart, u64_t msanr)
{
    phymmarge_t *pmagep = (phymmarge_t *)phyadr_to_viradr((adr_t)mbsp->mb_e820expadr);

    u64_t mdindx = 0;
    // 遍历e820扩展区的所有内存区域
    for (u64_t i = 0; i < mbsp->mb_e820exnr; i++)
    {
        // 寻找"可用的物理内存区域"的区域
        if (PMR_T_OSAPUSERRAM == pmagep[i].pmr_type)
        {
            //以4KB遍历地址
            for (u64_t start = pmagep[i].pmr_saddr; start < pmagep[i].pmr_end; start += 4096)
            {
                // 确保当前页的结束地址不超过区域边界
                if ((start + 4096 - 1) <= pmagep[i].pmr_end)
                {
                    // 将当前页的信息写入内存描述符数组
                    write_one_msadsc(&msavstart[mdindx], start);
                    mdindx++; // 移动到下一个描述符位置
                }

            }
        }
    }
    return mdindx;
}


void init_msadsc()
{
    u64_t coremdnr = 0, msadscnr = 0;
    msadsc_t *msadscvp = NULL;
    machbstart_t *mbsp = &kmachbsp;
    if(ret_msadsc_vadrandsz(mbsp,&msadscvp,&msadscnr) == FALSE)//遍历e820表寻找可用的物理内存区域，并返回地址和页数
    {
        system_error("init_msadsc ret_msadsc_vadrandsz err\n");
    }
    coremdnr = init_msadsc_core(mbsp, msadscvp, msadscnr);
    if(coremdnr != msadscnr)
    {
		system_error("init_msadsc init_msadsc_core err\n");
	}
    mbsp->mb_memmappadr = viradr_to_phyadr((adr_t)msadscvp);
	mbsp->mb_memmapnr = coremdnr;
	mbsp->mb_memmapsz = coremdnr * sizeof(msadsc_t);
	mbsp->mb_nextwtpadr = PAGE_ALIGN(mbsp->mb_memmappadr + mbsp->mb_memmapsz);
    return;
}




// 搜索占用的MSADSC段，返回页数目
u64_t search_segment_occupymsadsc(msadsc_t *msastart, u64_t msanr, u64_t ocpystat, u64_t ocpyend) {
    u64_t mphyadr = 0, fsmsnr = 0;
    // 用于临时存储找到的内存描述结构的指针
    msadsc_t *fstatmp = NULL;
    // 遍历内存描述结构数组
    for (u64_t mnr = 0; mnr < msanr; mnr++) {
        // 检查当前内存段的物理地址是否与要查找的起始地址匹配
        if ((msastart[mnr].md_phyadrs.paf_padrs << PSHRSIZE) == ocpystat) {
            fstatmp = &msastart[mnr];
            // 找到匹配的起始页的描述符，跳转到step1
            goto step1;
        }
    }
step1:
    fsmsnr = 0;
    // 如果没有找到匹配的起始段，返回0
    if (NULL == fstatmp) {
        return 0;
    }
    // 遍历从起始地址到结束地址的范围，以页大小为步长
    for (u64_t tmpadr = ocpystat; tmpadr < ocpyend; tmpadr += PAGESIZE, fsmsnr++) {
        // 计算每页的起始地址
        mphyadr = fstatmp[fsmsnr].md_phyadrs.paf_padrs << PSHRSIZE;
        // 如果计算出的页起始与当前遍历的地址不匹配，返回0
        if (mphyadr!= tmpadr) {
            return 0;
        }
        // 检查当前内存段的状态标志，如果不符合要求，（不是 MF_MOCTY_FREE（表示空闲），或者分配计数不为 0，或者物理地址分配状态不是 PAF_NO_ALLOC（表示未分配）），返回0
        if (MF_MOCTY_FREE!= fstatmp[fsmsnr].md_indxflgs.mf_mocty ||
            0!= fstatmp[fsmsnr].md_indxflgs.mf_uindx ||
            PAF_NO_ALLOC!= fstatmp[fsmsnr].md_phyadrs.paf_alloc) {
            return 0;
        }
        // 更新内存段的状态标志
        fstatmp[fsmsnr].md_indxflgs.mf_mocty = MF_MOCTY_KRNL;//标记为被内核占用
        fstatmp[fsmsnr].md_indxflgs.mf_uindx++;//分配计数增加
        fstatmp[fsmsnr].md_phyadrs.paf_alloc = PAF_ALLOC;//标记为被分配
    }
    // 计算要查找的内存段的大小
    u64_t ocpysz = ocpyend - ocpystat;
    // 如果内存段大小不是页大小的整数倍
    if ((ocpysz & 0xfff)!= 0) {
        // 检查段数量是否正确
        if (((ocpysz >> PSHRSIZE) + 1)!= fsmsnr) {
            return 0;
        }
        return fsmsnr;
    }
    // 如果内存段大小是页大小的整数倍，检查段数量是否正确
    if ((ocpysz >> PSHRSIZE)!= fsmsnr) {
        return 0;
    }
    return fsmsnr;
}



// 函数 search_krloccupymsadsc_core 用于搜索并检查指定内存段是否在 MSADSC（可能是某种内存状态描述结构）中被占用
// 参数 mbsp 是一个指向 machbstart_t 结构体的指针，该结构体包含了启动相关的内存信息
bool_t search_krloccupymsadsc_core(machbstart_t *mbsp)
{
    // 定义一个 64 位无符号整数 retschmnr，用于存储搜索结果
    u64_t retschmnr = 0;

    // 将 mbsp 结构体中的物理内存映射地址转换为虚拟地址，并将其强制转换为 msadsc_t 类型的指针
    // msadsc_t 可能是用于描述内存状态的结构体
    msadsc_t *msadstat = (msadsc_t *)phyadr_to_viradr((adr_t)mbsp->mb_memmappadr);

    // 从 mbsp 结构体中获取内存映射编号，存储在 msanr 中
    u64_t msanr = mbsp->mb_memmapnr;

    // 调用 search_segment_occupymsadsc 函数，搜索从地址 0 到 0x1000 的内存段 既搜索BIOS中断表占用的内存页所对应msadsc_t结构
    // 参数分别为：msadsc 结构体指针、内存映射编号、起始地址、结束地址
    retschmnr = search_segment_occupymsadsc(msadstat, msanr, 0, 0x1000);
    // 如果搜索结果为 0，表示该页未被占用，返回 FALSE
    if (0 == retschmnr)
    {
        return FALSE;
    }

    // 调用 search_segment_occupymsadsc 函数，搜索内核栈占用的内存页所对应msadsc_t结构
    // mbsp->mb_krlinitstack & (~(0xfffUL)) 用于将 mbsp->mb_krlinitstack 按页对齐
    retschmnr = search_segment_occupymsadsc(msadstat, msanr, mbsp->mb_krlinitstack & (~(0xfffUL)), mbsp->mb_krlinitstack);
    // 如果搜索结果为 0，表示该页未被占用，返回 FALSE
    if (0 == retschmnr)
    {
        return FALSE;
    }

    // 调用 search_segment_occupymsadsc 函数，搜索内核占用的内存页所对应msadsc_t结构
    retschmnr = search_segment_occupymsadsc(msadstat, msanr, mbsp->mb_krlimgpadr, mbsp->mb_nextwtpadr);
    // 如果搜索结果为 0，表示该页未被占用，返回 FALSE
    if (0 == retschmnr)
    {
        return FALSE;
    }

    // 调用 search_segment_occupymsadsc 函数，搜索从 mbsp->mb_imgpadr 到 mbsp->mb_imgpadr + mbsp->mb_imgsz 的内存段是否被占用
    // mbsp->mb_imgsz 表示图像的大小
    retschmnr = search_segment_occupymsadsc(msadstat, msanr, mbsp->mb_imgpadr, mbsp->mb_imgpadr + mbsp->mb_imgsz);
    // 如果搜索结果为 0，表示该内存段未被占用，返回 FALSE
    if (0 == retschmnr)
    {
        return FALSE;
    }

    // 如果以上所有内存段都被占用，返回 TRUE
    return TRUE;
}

void init_search_krloccupymm(machbstart_t *mbsp)
{
	if (search_krloccupymsadsc_core(mbsp) == FALSE)
	{
		system_error("search_krloccupymsadsc_core fail\n");
	}
	return;
}
