#include "cosmostypes.h"
#include "cosmosmctrl.h"


void init_memmgr(){
    init_msadsc();//初始化msadsc_t结构体
    init_memarea();//初始化memarea_t结构体
    init_copy_pagesfvm();//迁移页表以及字体文件
    init_search_krloccupymm(&kmachbsp);//初始化搜索内核占用的内存页面
    init_merlove_mem();//合并空闲内存区域并挂载
    init_memmgrob();//初始化内存管理全局对象
}


/* 初始化内存管理全局对象 */
void init_memmgrob()
{
    // 获取机器启动参数和内存管理对象指针
    machbstart_t *mbsp = &kmachbsp;          // 内核机器启动参数结构
    memmgrob_t *mobp = &memmgrob;           // 全局内存管理对象
    
    // 初始化内存管理对象为默认值
    memmgrob_t_init(mobp);

    /*=============== 启动参数有效性验证 ===============*/
    // 检查E820扩展内存表参数
    if (mbsp->mb_e820expadr == NULL || mbsp->mb_e820exnr == 0) {
        system_error("E820内存表参数无效\n");
    }
    // 检查内存页描述符表参数
    if (mbsp->mb_memmappadr == NULL || mbsp->mb_memmapnr == 0) {
        system_error("内存页描述符表参数无效\n");
    }
    // 检查内存区域描述符表参数
    if (mbsp->mb_memznpadr == NULL || mbsp->mb_memznnr == 0) {
        system_error("内存区域描述符表参数无效\n");
    }

    /*=============== 地址转换与字段初始化 ===============*/
    // 物理地址转虚拟地址并填充管理对象
    mobp->mo_pmagestat = (phymmarge_t*)phyadr_to_viradr((adr_t)mbsp->mb_e820expadr); // 物理内存段数组
    mobp->mo_pmagenr = mbsp->mb_e820exnr;         // 物理内存段数量
    
    mobp->mo_msadscstat = (msadsc_t*)phyadr_to_viradr((adr_t)mbsp->mb_memmappadr); // 内存页描述符数组
    mobp->mo_msanr = mbsp->mb_memmapnr;            // 总页描述符数量
    
    mobp->mo_mareastat = (memarea_t*)phyadr_to_viradr((adr_t)mbsp->mb_memznpadr); // 内存区域数组
    mobp->mo_mareanr = mbsp->mb_memznnr;           // 内存区域数量
    
    // 计算内存总大小和页数
    mobp->mo_memsz = mbsp->mb_memmapnr << PSHRSIZE; // 总物理内存大小 = 页数 * 页大小
    mobp->mo_maxpages = mbsp->mb_memmapnr;          // 最大可管理页数

    /*=============== 统计初始内存状态 ===============*/
    uint_t allocated_pages = 0;
    // 遍历所有页描述符，统计已分配的内核页
    for (uint_t i = 0; i < mobp->mo_msanr; i++) {
        // 判断条件：用户索引为1（内核保留）、内存占用类型为内核、分配状态为已分配
        if (mobp->mo_msadscstat[i].md_indxflgs.mf_uindx == 1 &&
            mobp->mo_msadscstat[i].md_indxflgs.mf_mocty == MF_MOCTY_KRNL &&
            mobp->mo_msadscstat[i].md_phyadrs.paf_alloc == PAF_ALLOC) {
            allocated_pages++;
        }
    }
    
    // 更新管理对象统计信息
    mobp->mo_alocpages = allocated_pages;          // 已分配页数
    mobp->mo_freepages = mobp->mo_maxpages - allocated_pages; // 空闲页数

    return;
}

/* 内存管理对象初始化函数 */
void memmgrob_t_init(memmgrob_t *initp)
{
    // 初始化链表和锁
    list_init(&initp->mo_list);          // 管理链表初始化
    knl_spinlock_init(&initp->mo_lock);   // 初始化自旋锁
    
    // 状态标志初始化
    initp->mo_stus = 0;                  // 状态字清零
    initp->mo_flgs = 0;                  // 标志位清零
    
    // 内存统计信息初始化
    initp->mo_memsz = 0;                  // 总内存大小清零
    initp->mo_maxpages = 0;               // 最大页数清零
    initp->mo_freepages = 0;             // 空闲页数清零
    initp->mo_alocpages = 0;             // 已分配页数清零
    initp->mo_resvpages = 0;             // 保留页数清零
    initp->mo_horizline = 0;             // 内存水位线清零
    
    // 指针和数组初始化
    initp->mo_pmagestat = NULL;            // 物理内存段数组指针置空
    initp->mo_pmagenr = 0;                 // 物理内存段数量清零
    initp->mo_msadscstat = NULL;           // 页描述符数组指针置空
    initp->mo_msanr = 0;                  // 页描述符数量清零
    initp->mo_mareastat = NULL;           // 内存区域数组指针置空
    initp->mo_mareanr = 0;                // 内存区域数量清零
    
    // 扩展字段初始化
    initp->mo_privp = NULL;                // 私有数据指针置空
    initp->mo_extp = NULL;                // 扩展数据指针置空
    
    return;
}



// 复制页面数据的函数
bool_t copy_pages_data(machbstart_t *mbsp)
{
    // 获取当前的页面地址
    uint_t topgadr = mbsp->mb_nextwtpadr;

    // 检查初始化地址是否有效
    if (initchkadr_is_ok(mbsp, topgadr, mbsp->mb_subpageslen) != 0)
    {
        return FALSE; // 如果初始化地址无效，则返回失败
    }

    // 将物理地址转换为虚拟地址，并为页表相关结构分配空间
    uint_t *p = (uint_t *)phyadr_to_viradr((adr_t)topgadr);
    uint_t *pdpte = (uint_t *)(((uint_t)p) + 0x1000);
    uint_t *pde = (uint_t *)(((uint_t)p) + 0x2000);

    // 初始化页表条目为0
    for (uint_t mi = 0; mi < PGENTY_SIZE; mi++)
    {
        p[mi] = 0;
        pdpte[mi] = 0;
    }

    uint_t adr = 0;
    uint_t pdepd = 0;

    // 填充每个页目录条目
    for (uint_t pdei = 0; pdei < 16; pdei++)
    {
        // 获取页目录物理地址并更新页目录指针
        pdepd = (uint_t)viradr_to_phyadr((adr_t)pde);
        pdpte[pdei] = (uint_t)(pdepd | KPDPTE_RW | KPDPTE_P);

        // 填充每个页目录条目，设置页表条目的相关标志
        for (uint_t pdeii = 0; pdeii < PGENTY_SIZE; pdeii++)
        {
            pde[pdeii] = 0 | adr | KPDE_PS | KPDE_RW | KPDE_P;
            adr += 0x200000; // 地址增加
        }
        pde = (uint_t *)((uint_t)pde + 0x1000); // 更新页目录地址
    }

    // 获取页目录指针表的物理地址并更新页表的最高位
    uint_t pdptepd = (uint_t)viradr_to_phyadr((adr_t)pdpte);
    p[((KRNL_VIRTUAL_ADDRESS_START) >> KPML4_SHIFT) & 0x1ff] = (uint_t)(pdptepd | KPML4_RW | KPML4_P);
    p[0] = (uint_t)(pdptepd | KPML4_RW | KPML4_P);

    // 更新页表结构相关的信息
    mbsp->mb_pml4padr = topgadr;
    mbsp->mb_subpageslen = (uint_t)(0x1000 * 16 + 0x2000);
    mbsp->mb_kpmapphymemsz = (uint_t)(0x400000000); // 物理内存大小
    mbsp->mb_nextwtpadr = PAGE_ALIGN(mbsp->mb_pml4padr + mbsp->mb_subpageslen);

    return TRUE; // 返回成功
}

// 复制FVM（固件虚拟内存）数据的函数
bool_t copy_fvm_data(machbstart_t *mbsp, dftgraph_t *dgp)
{
    // 获取复制目标地址
    u64_t tofvadr = mbsp->mb_nextwtpadr;

    // 检查目标地址是否有效
    if (initchkadr_is_ok(mbsp, tofvadr, dgp->gh_fvrmsz) != 0)
    {
        return FALSE; // 如果目标地址无效，返回失败
    }

    // 执行内存从源到目标的复制操作
    sint_t retcl = m2mcopy((void *)((uint_t)dgp->gh_fvrmphyadr), (void *)phyadr_to_viradr((adr_t)(tofvadr)), (sint_t)dgp->gh_fvrmsz);
    if (retcl != (sint_t)dgp->gh_fvrmsz)
    {
        return FALSE; // 如果复制的字节数不匹配，返回失败
    }

    // 更新源数据的物理地址
    dgp->gh_fvrmphyadr = phyadr_to_viradr((adr_t)tofvadr);
    mbsp->mb_fvrmphyadr = tofvadr;

    // 更新下一次写入的目标地址
    mbsp->mb_nextwtpadr = PAGE_ALIGN(tofvadr + dgp->gh_fvrmsz);

    return TRUE; // 返回成功
}

// 设置内存管理单元（MMU）可修改的函数
void memi_set_mmutabl(uint_t tblpadr, void *edatap)
{
    set_cr3(tblpadr); // 设置CR3寄存器为指定的表格地址
    return;
}

// 初始化复制页面和FVM数据的函数
void init_copy_pagesfvm()
{
    // 复制页表数据
    if (copy_pages_data(&kmachbsp) == FALSE)
    {
        system_error("copy_pages_data fail"); // 如果复制页面数据失败，报错
    }

    // 复制图像相关数据
    if (copy_fvm_data(&kmachbsp, &kdftgh) == FALSE)
    {
        system_error("copy_fvm_data fail"); // 如果复制FVM数据失败，报错
    }

    // 设置新的页表为系统页表
    memi_set_mmutabl(kmachbsp.mb_pml4padr, NULL);

    return;
}
