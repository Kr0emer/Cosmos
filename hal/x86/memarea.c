#include "cosmostypes.h"
#include "cosmosmctrl.h"
void arclst_t_init(arclst_t *initp)//待定
{
	list_init(&initp->al_lru1);
	list_init(&initp->al_lru2);
	initp->al_lru1nr = 0;
	initp->al_lru2nr = 0;
	return;
}
/*TO DO*/
mmstus_t mafo_deft_init(struct s_MEMAREA *memarea, void *valp, uint_t val)
{
	return MMSTUS_ERR;
}

mmstus_t mafo_deft_exit(struct s_MEMAREA *memarea)
{
	return MMSTUS_ERR;
}

mmstus_t mafo_deft_afry(struct s_MEMAREA *memarea, mmafrets_t *mafrspack, void *valp, uint_t val)
{
	return MMSTUS_ERR;
}


void mafuncobjs_t_init(mafuncobjs_t *initp)
{
	initp->mafo_init = mafo_deft_init;
	initp->mafo_exit = mafo_deft_exit;
	initp->mafo_aloc = mafo_deft_afry;
	initp->mafo_free = mafo_deft_afry;
	initp->mafo_recy = mafo_deft_afry;
	return;
}

void bafhlst_t_init(bafhlst_t *initp, u32_t stus, uint_t oder, uint_t oderpnr)
{
	knl_spinlock_init(&initp->af_lock);//初始化锁
	initp->af_stus = stus;//设置状态
	initp->af_oder = oder;//页面数的位移量
	initp->af_oderpnr = oderpnr;//oder对应的页面数比如 oder为2那就是1<<2=4
	initp->af_fobjnr = 0;
	//initp->af_aobjnr=0;
	initp->af_mobjnr = 0;
	initp->af_alcindx = 0;
	initp->af_freindx = 0;
	list_init(&initp->af_frelst);
	list_init(&initp->af_alclst);
	list_init(&initp->af_ovelst);
	return;
}

void memdivmer_t_init(memdivmer_t *initp)
{
	knl_spinlock_init(&initp->dm_lock);
	initp->dm_stus = 0;
	initp->dm_dmmaxindx = 0;
	initp->dm_phydmindx = 0;
	initp->dm_predmindx = 0;
	initp->dm_divnr = 0;
	initp->dm_mernr = 0;

	for (uint_t li = 0; li < MDIVMER_ARR_LMAX; li++)
	{
		bafhlst_t_init(&initp->dm_mdmlielst[li], BAFH_STUS_DIVM, li, (1UL << li));
	}
	bafhlst_t_init(&initp->dm_onemsalst, BAFH_STUS_ONEM, 0, 1UL);
	return;
}


void memarea_t_init(memarea_t *initp)
{
    // 初始化内存区域的链表
    list_init(&initp->ma_list);
    // 初始化自旋锁
    knl_spinlock_init(&initp->ma_lock);
    // 初始化参数
    initp->ma_stus = 0;
	initp->ma_flgs = 0;
	initp->ma_type = MA_TYPE_INIT;
	initp->ma_maxpages = 0;
	initp->ma_allocpages = 0;
	initp->ma_freepages = 0;
	initp->ma_resvpages = 0;
	initp->ma_horizline = 0;
	initp->ma_logicstart = 0;
	initp->ma_logicend = 0;
	initp->ma_logicsz = 0;
	initp->ma_effectstart = 0;
	initp->ma_effectend = 0;
	initp->ma_effectsz = 0;

    list_init(&initp->ma_allmsadsclst);
    initp->ma_allmsadscnr = 0;
	arclst_t_init(&initp->ma_arcpglst);
	mafuncobjs_t_init(&initp->ma_funcobj);
	memdivmer_t_init(&initp->ma_mdmdata);
	initp->ma_privp = NULL;
    return;
}


bool_t init_memarea_core(machbstart_t *mbsp)
{
    // 从传入的结构体 `mbsp` 中获取物理内存区域的起始地址
    u64_t phymarea = mbsp->mb_nextwtpadr;

    // 检查物理内存区域的起始地址是否有效
    if (initchkadr_is_ok(mbsp, phymarea, (sizeof(memarea_t) * MEMAREA_MAX)) != 0)
    {
        // 如果内存地址无效，返回 FALSE 表示初始化失败
        return FALSE;
    }

    // 将物理地址转换为虚拟地址，并指向 `memarea_t` 类型的内存区域
    memarea_t *virmarea = (memarea_t *)phyadr_to_viradr((adr_t)phymarea);

    // 初始化 `MEMAREA_MAX` 数量的内存区域
    for (uint_t mai = 0; mai < MEMAREA_MAX; mai++)
    {
        memarea_t_init(&virmarea[mai]);
    }

    // 设置第一个内存区域的属性，标识为硬件区域
    virmarea[0].ma_type = MA_TYPE_HWAD;
    virmarea[0].ma_logicstart = MA_HWAD_LSTART;  // 硬件区域的逻辑起始地址
    virmarea[0].ma_logicend = MA_HWAD_LEND;      // 硬件区域的逻辑结束地址
    virmarea[0].ma_logicsz = MA_HWAD_LSZ;        // 硬件区域的逻辑大小

    // 设置第二个内存区域的属性，标识为内核区域
    virmarea[1].ma_type = MA_TYPE_KRNL;
    virmarea[1].ma_logicstart = MA_KRNL_LSTART;  // 内核区域的逻辑起始地址
    virmarea[1].ma_logicend = MA_KRNL_LEND;      // 内核区域的逻辑结束地址
    virmarea[1].ma_logicsz = MA_KRNL_LSZ;        // 内核区域的逻辑大小

    // 设置第三个内存区域的属性，标识为进程区域
    virmarea[2].ma_type = MA_TYPE_PROC;
    virmarea[2].ma_logicstart = MA_PROC_LSTART;  // 进程区域的逻辑起始地址
    virmarea[2].ma_logicend = MA_PROC_LEND;      // 进程区域的逻辑结束地址
    virmarea[2].ma_logicsz = MA_PROC_LSZ;        // 进程区域的逻辑大小

    // 设置第四个内存区域的属性，标识为共享区域
    virmarea[3].ma_type = MA_TYPE_SHAR;

    // 更新 `mbsp` 结构体的内存区域信息
    mbsp->mb_memznpadr = phymarea; // 物理内存区域起始地址
    mbsp->mb_memznnr = MEMAREA_MAX; // 内存区域的数量
    mbsp->mb_memznsz = sizeof(memarea_t) * MEMAREA_MAX; // 内存区域的总大小

    // 更新下一内存区域的物理地址，按照页对齐
    mbsp->mb_nextwtpadr = PAGE_ALIGN(phymarea + sizeof(memarea_t) * MEMAREA_MAX);

    // 其他操作（省略）

    // 返回 TRUE，表示内存区域初始化成功
    return TRUE;
}


/* 验证内存区域页面标志设置正确性 */
uint_t test_setflgs(memarea_t *mareap, msadsc_t *mstat, uint_t msanr)
{
    u32_t muindx = 0;         // 预期标志索引值
    msadflgs_t *mdfp = NULL; // 预期标志结构指针

    // 参数有效性检查：空指针或零长度直接返回错误码
    if (NULL == mareap || NULL == mstat || 0 == msanr) {
        return ~0UL; // 返回全1值表示非法参数错误
    }

    /* 根据内存区域类型获取预期标志值 */
    switch (mareap->ma_type) {
    case MA_TYPE_HWAD: // 硬件相关内存区域
        muindx = MF_MARTY_HWD << 5;  // 左移5位组合标志
        mdfp = (msadflgs_t *)(&muindx);
        break;
    case MA_TYPE_KRNL: // 内核内存区域
        muindx = MF_MARTY_KRL << 5;
        mdfp = (msadflgs_t *)(&muindx);
        break;
    case MA_TYPE_PROC: // 进程内存区域
        muindx = MF_MARTY_PRC << 5;
        mdfp = (msadflgs_t *)(&muindx);
        break;
    case MA_TYPE_SHAR: // 共享内存区域（直接返回0不验证）
        return 0;
    default:            // 未知类型处理
        muindx = 0;
        mdfp = NULL;
        break;
    }

    // 预期标志有效性验证
    if (0 == muindx || NULL == mdfp) {
        return ~0UL; // 返回错误码表示类型不匹配
    }

    u64_t phyadr = 0;    // 临时存储物理地址
    uint_t retnr = 0;    // 符合预期的页计数器

    /* 遍历所有内存页描述符进行验证 */
    for (uint_t mix = 0; mix < msanr; mix++) {
        // 计算物理地址（与设置函数相同的计算方式）
        phyadr = mstat[mix].md_phyadrs.paf_padrs << PSHRSIZE;

        // 验证地址是否属于当前内存区域
        if (phyadr >= mareap->ma_logicstart && 
            ((phyadr + PAGESIZE) - 1) <= mareap->ma_logicend) {
            // 检查实际标志是否与预期标志一致
            if (mstat[mix].md_indxflgs.mf_marty == mdfp->mf_marty) {
                retnr++; // 符合预期计数递增
            }
        }
    }

    return retnr; // 返回符合预期的页数量
}


/* 内存区域标志设置函数 */
uint_t merlove_setallmarflgs_onmemarea(memarea_t *mareap, msadsc_t *mstat, uint_t msanr)
{
    // 参数有效性检查
    if (NULL == mareap || NULL == mstat || 0 == msanr) {
        return ~0UL; // 返回全1值表示非法参数错误
    }

    u32_t muindx = 0;         // 内存类型标志索引
    msadflgs_t *mdfp = NULL;  // 内存页标志结构指针

    /* 根据内存区域类型生成对应标志 */
    switch (mareap->ma_type) {
    case MA_TYPE_HWAD:        // 硬件相关内存区域
        muindx = MF_MARTY_HWD << 5;  // 左移5位组合标志位
        mdfp = (msadflgs_t *)(&muindx); // 转换为标志结构指针
        break;
    case MA_TYPE_KRNL:         // 内核内存区域
        muindx = MF_MARTY_KRL << 5;
        mdfp = (msadflgs_t *)(&muindx);
        break;
    case MA_TYPE_PROC:         // 进程内存区域
        muindx = MF_MARTY_PRC << 5;
        mdfp = (msadflgs_t *)(&muindx);
        break;
    case MA_TYPE_SHAR:         // 共享内存区域（直接返回不处理）
        return 0;
    default:                   // 未知类型处理
        muindx = 0;
        mdfp = NULL;
        break;
    }

    /* 标志有效性验证 */
    if (0 == muindx || NULL == mdfp) {
        return ~0UL; // 返回错误码表示无效内存类型
    }

    u64_t phyadr = 0;    // 临时存储物理地址
    uint_t retnr = 0;    // 成功设置标志的页面计数器

    /* 遍历内存页描述符数组 */
    for (uint_t mix = 0; mix < msanr; mix++) {
        // 仅处理初始化状态的页面
        if (MF_MARTY_INIT == mstat[mix].md_indxflgs.mf_marty) {
            // 计算实际物理地址（页框号转地址）
            phyadr = mstat[mix].md_phyadrs.paf_padrs << PSHRSIZE;
            
            /* 验证地址范围有效性 */
            if (phyadr >= mareap->ma_logicstart && 
                ((phyadr + PAGESIZE) - 1) <= mareap->ma_logicend) {
                // 设置内存页类型标志
                mstat[mix].md_indxflgs.mf_marty = mdfp->mf_marty;
                retnr++; // 成功计数递增
            }
        }
    }

    return retnr; // 返回成功设置的页面数量
}


uint_t continumsadsc_is_ok(msadsc_t* prevmsa,msadsc_t* nextmsa,msadflgs_t* cmpmdfp){

    if(NULL == prevmsa || NULL == nextmsa || NULL == cmpmdfp){//检查基础参数
        return (~0UL);
    }
    //检查当前页属性
    if  (prevmsa->md_indxflgs.mf_marty == cmpmdfp->mf_marty &&  // 内存类型匹配
        0 == prevmsa->md_indxflgs.mf_uindx &&                   // 分配计数为0
        MF_MOCTY_FREE == prevmsa->md_indxflgs.mf_mocty &&       // 空闲状态
        PAF_ALLOC == prevmsa->md_phyadrs.paf_alloc)             // 未分配状态
    {
        //检查下一页属性
        if  (nextmsa->md_indxflgs.mf_marty == cmpmdfp->mf_marty &&  // 内存类型匹配
            0 == nextmsa->md_indxflgs.mf_uindx &&                   // 分配计数为0
            MF_MOCTY_FREE == nextmsa->md_indxflgs.mf_mocty &&       // 空闲状态
            PAF_ALLOC == nextmsa->md_phyadrs.paf_alloc)             // 未分配状态
        {
            u64_t now_addr = prevmsa->md_phyadrs.paf_padrs << PAGE_SIZE;
            u64_t next_addr = prevmsa->md_phyadrs.paf_padrs << PAGE_SIZE;
            if((next_addr - now_addr) == PAGE_SIZE){
                return 2; //地址连续且属性匹配
            }
            return  1;//地址不连续但属性匹配
        }
        return 1;//后一页地址和属性都不匹配
    }
    return 0;//前一页属性不匹配
}






bool_t scan_len_msadsc(msadsc_t* mstat,msadflgs_t* cmpmdfp,uint_t mnr,uint_t* retmnr){
    uint_t retclok = 0;//连续性检查结果
    uint_t retnr = 0;//连续块占据数量

    if(NULL == mstat || NULL == cmpmdfp || 0 == mnr || NULL == retmnr){//检查基本参数
        return FALSE;
    }
    
    for(uint_t mix = 0; mix < mnr - 1 ; mix++){
        retclok = continumsadsc_is_ok(&mstat[mix], &mstat[mix + 1], cmpmdfp);
        if(retclok == (~0UL)){//如果传入地址为空
            *retmnr = 0;
            return FALSE;
        }
        if(retclok == 0){//如果当前页属性错误
            *retmnr = 0;
            return FALSE;       
        }
        if(retclok == 1){//如果当前页属性正确但下一页地址错误或属性错误
            *retmnr = retnr;
            return TRUE;
        }
        if(retclok == 2){//如果当前页和下一页都符合预期
            retnr++;
        }
    }
    *retmnr = retnr;
    return TRUE;
}





/* 连续内存块扫描函数 */
bool_t merlove_scan_continumsadsc(memarea_t *mareap, msadsc_t *fmstat, uint_t *fntmsanr, uint_t fmsanr,
                                  msadsc_t **retmsastatp, msadsc_t **retmsaendp, uint_t *retfmnr)
{
    // 参数有效性检查（包含7个必要参数检查）
    if (NULL == mareap || NULL == fmstat || NULL == fntmsanr ||
        0 == fmsanr || NULL == retmsastatp || NULL == retmsaendp || NULL == retfmnr) {
        return FALSE;
    }
    // 扫描位置越界检查
    if (*fntmsanr >= fmsanr) {
        return FALSE;
    }

    u32_t muindx = 0;         // 内存类型标志索引
    msadflgs_t *mdfp = NULL;  // 目标内存标志结构指针

    /* 生成目标内存区域的特征标志 */
    switch (mareap->ma_type) {
    case MA_TYPE_HWAD:        // 硬件相关内存
        muindx = MF_MARTY_HWD << 5;  // 左移5位组合标志位
        mdfp = (msadflgs_t *)(&muindx);
        break;
    case MA_TYPE_KRNL:         // 内核内存
        muindx = MF_MARTY_KRL << 5;
        mdfp = (msadflgs_t *)(&muindx);
        break;
    case MA_TYPE_PROC:         // 进程内存
        muindx = MF_MARTY_PRC << 5;
        mdfp = (msadflgs_t *)(&muindx);
        break;
    default:                   // 不支持的内存类型
        muindx = 0;
        mdfp = NULL;
        break;
    }

    /* 标志有效性验证 */
    if (0 == muindx || NULL == mdfp) {
        return FALSE;
    }

    msadsc_t *msastat = fmstat;    // 内存页描述符组起始指针
    uint_t retfindmnr = 0;         // 找到的连续页数
    bool_t rets = FALSE;           // 扫描操作返回值
    uint_t tmidx = *fntmsanr;       // 当前扫描索引

    /* 遍历内存页数组 */
    for (; tmidx < fmsanr; tmidx++) {
        // 检查当前页是否符合目标特征
        if (msastat[tmidx].md_indxflgs.mf_marty == mdfp->mf_marty &&      // 内存类型匹配
            0 == msastat[tmidx].md_indxflgs.mf_uindx &&                  // 分配计数为0
            MF_MOCTY_FREE == msastat[tmidx].md_indxflgs.mf_mocty &&      // 空闲状态
            PAF_NO_ALLOC == msastat[tmidx].md_phyadrs.paf_alloc) {       // 未分配状态
            
            // 扫描连续内存块长度
            rets = scan_len_msadsc(
                &msastat[tmidx], //当前页描述符
                mdfp, //标志
                fmsanr - tmidx, //剩余页数目
                &retfindmnr);//连续页数目
            if (FALSE == rets) {
                system_error("scan_len_msadsc err\n");  // 连续块验证失败触发系统崩溃
            }

            /* 更新扫描位置和返回参数 */
            *fntmsanr = tmidx + retfindmnr + 1;  // 跳过已找到的连续块
            *retmsastatp = &msastat[tmidx];     // 设置连续块起始页
            *retmsaendp = &msastat[tmidx + retfindmnr];  // 设置连续块结束页,若只有一页则retfindmnr为0
            *retfmnr = retfindmnr + 1;          // 计算实际页数（包含起始页），若只有一页为1
            return TRUE;
        }
    }

    /* 遍历完成处理 */
    if (tmidx >= fmsanr) {
        *fntmsanr = fmsanr;     // 标记扫描完成
        *retmsastatp = NULL;    // 清空返回指针
        *retmsaendp = NULL;
        *retfmnr = 0;
        return TRUE;            // 正常结束返回成功
    }

    return FALSE;  // 异常路径返回
}

uint_t check_continumsadsc(memarea_t* mareap,msadsc_t* stat,msadsc_t* end,uint_t fmnr){

    if(NULL  == mareap || NULL ==stat || NULL ==end || 0 == fmnr || stat > end){
        return 0;
    }
    msadsc_t *now_start = stat, *now_end = end;
    u32_t muindx = 0;
	msadflgs_t *mdfp = NULL;

    switch (mareap->ma_type) {
        case MA_TYPE_HWAD:        // 硬件相关内存区域
            muindx = MF_MARTY_HWD << 5;  // 左移5位组合标志位
            mdfp = (msadflgs_t *)(&muindx); // 转换为标志结构指针
            break;
        case MA_TYPE_KRNL:         // 内核内存区域
            muindx = MF_MARTY_KRL << 5;
            mdfp = (msadflgs_t *)(&muindx);
            break;
        case MA_TYPE_PROC:         // 进程内存区域
            muindx = MF_MARTY_PRC << 5;
            mdfp = (msadflgs_t *)(&muindx);
            break;
        case MA_TYPE_SHAR:         // 共享内存区域（直接返回不处理）
            return 0;
        default:                   // 未知类型处理
            muindx = 0;
            mdfp = NULL;
            break;
    }
    uint_t check_num = 0;
    if(now_start == now_end){//若只有一页
        if (now_start->md_indxflgs.mf_marty != mdfp->mf_marty &&      // 内存类型匹配
            0 != now_start->md_indxflgs.mf_uindx &&                  // 分配计数为0
            MF_MOCTY_FREE != now_start->md_indxflgs.mf_mocty &&      // 空闲状态
            PAF_NO_ALLOC != now_start->md_phyadrs.paf_alloc) {       // 未分配状态
            
                return 0;
        }
        return (check_num + 1 == fmnr) ? (check_num + 1) : 0;//若计数与传入参数不同返回零，否则返回计数
    }
    for(; now_start <= now_end; now_start++){
        //检查两页的归属区域
        if  (now_start->md_indxflgs.mf_marty != mdfp->mf_marty ||
            (now_start+1)->md_indxflgs.mf_marty != mdfp->mf_marty){
                return 0;
            }
        //检查两项是否空闲
        if  (now_start->md_indxflgs.mf_mocty != MF_MOCTY_FREE ||
            (now_start+1)->md_indxflgs.mf_mocty != MF_MOCTY_FREE){
                return 0;
            }
        //检查分配计数
        if  (now_start->md_indxflgs.mf_uindx  != 0 ||
            (now_start+1)->md_indxflgs.mf_uindx  != 0){
                return 0;
            }
        //检查分配位
        if  (now_start->md_phyadrs.paf_alloc  != PAF_NO_ALLOC  ||
            (now_start+1)->md_indxflgs.mf_uindx  != PAF_NO_ALLOC ){
                return 0;
            }
        //检查连续性
        u64_t now_addr = now_start->md_phyadrs.paf_padrs << PSHRSIZE;
        u64_t next_addr = (now_start + 1)->md_phyadrs.paf_padrs << PSHRSIZE;
        if(now_start - now_addr != PAGE_SIZE){
            return 0;
        }
        check_num++;
    }
    if(check_num ==0 || (check_num +1) != fmnr){
        return 0;
    }
    return check_num;
}

bafhlst_t *find_continumsa_inbafhlst(memarea_t *mareap, uint_t fmnr){
    //检查参数有效性
    if(mareap == NULL || fmnr == 0){
        return NULL;
    }
    bafhlst_t *retbafhp = NULL;  // 返回的管理结构指针
    uint_t match_count = 0;       // 符合条件的管理结构计数
    /* 特殊内存类型处理 */
    if(mareap->ma_type == MA_TYPE_PROC){
        return &mareap->ma_mdmdata.dm_onemsalst;// 进程内存直接返回专用管理结构
    }
    if (mareap->ma_type == MA_TYPE_SHAR) {
        // 共享内存不支持此操作
        return NULL;
    }

    /* 遍历bafhlst_t数组 */
    for(uint_t i = 0; i < MDIVMER_ARR_LMAX; i++){
        bafhlst_t *current = &mareap->ma_mdmdata.dm_mdmlielst[i];
        // 检查当前结构能否容纳请求页数
        if(current->af_oderpnr <= fmnr){
            retbafhp = current;     // 记录符合条件的管理结构
            match_count++;
        }
    }
    /*有效性检查*/
    if(match_count >= MDIVMER_ARR_LMAX ||  NULL == retbafhp){
        return NULL;
    }
    return retbafhp;//返回最大的比内存区小的头结点
}

bool_t continumsadsc_add_bafhlst(memarea_t* mareap,bafhlst_t* bafhp,msadsc_t* fstat,msadsc_t* fend,uint_t fmnr){
     // 参数有效性检查
    if(NULL == mareap || NULL == bafhp || NULL == fstat || NULL == fend || 0 == fmnr){
        return FALSE;
    }
    //查实际页数是否符合管理结构预期
    if(bafhp->af_oderpnr != fmnr){
        return FALSE;
    }
    // 验证结束页是否为起始页后的第(fmnr-1)个页（确保连续性）
    if ((&fstat[fmnr - 1]) != fend) {
        return FALSE;
    }
    // 设置起始页为"头节点"（ORDER HEAD）
    fstat->md_indxflgs.mf_olkty = MF_OLKTY_ODER;//标记为起始结点
    fstat->md_odlink = fend;

    // 设置结束页为"管理结构尾节点"（BAFH TAIL）
    fend->md_indxflgs.mf_olkty = MF_OLKTY_BAFH;  // 标记为块结束节点
    fend->md_odlink = bafhp;                     // 结束页指向管理结构

    // 将起始页插入管理结构的空闲链表头部
    list_add(&fstat->md_list, &bafhp->af_frelst);

    bafhp->af_fobjnr++;   // 空闲链表对象计数+1（例如：增加一个4页块）
    bafhp->af_mobjnr++;   // 总管理对象计数+1

    mareap->ma_maxpages += fmnr;    // 区域最大可管理页数增加
    mareap->ma_freepages += fmnr;  // 区域空闲页数增加
    mareap->ma_allmsadscnr += fmnr;// 区域总页描述符计数增加

    return TRUE; // 操作成功
}

bool_t continumsadsc_add_procmareabafh(memarea_t* mareap,bafhlst_t* bafhp,msadsc_t* fstat,msadsc_t* fend,uint_t fmnr){
    // 参数有效性检查
    if(NULL == mareap || NULL == bafhp || NULL == fstat || NULL == fend || 0 == fmnr){
        return FALSE;
    }
    // 验证管理结构状态为单页模式，且内存区域类型为进程内存
    if (bafhp->af_stus != BAFH_STUS_ONEM || mareap->ma_type != MA_TYPE_PROC) {
        return FALSE; 
    }
    // 确保管理结构设置为单页模式（af_oderpnr=1）
    if (bafhp->af_oderpnr != 1) {
        return FALSE; 
    }
    // 确保结束页是起始页后的第(fmnr-1)个页（允许连续批量操作）
    if (&fstat[fmnr - 1] != fend) {
        return FALSE; 
    }
    //逐页添加至管理结构
    for (uint_t tmpnr = 0; tmpnr < fmnr; tmpnr++) {
        // 设置当前页为管理结构关联页
        fstat[tmpnr].md_indxflgs.mf_olkty = MF_OLKTY_BAFH; //标记该页已被管理结构接管
        fstat[tmpnr].md_odlink = bafhp;                   // 页直接关联管理结构
    
        // 将当前页加入空闲链表
        list_add(&fstat[tmpnr].md_list, &bafhp->af_frelst);
    
        bafhp->af_fobjnr++;    // 空闲页计数+1
        bafhp->af_mobjnr++;     // 总管理页计数+1

        mareap->ma_maxpages++;     // 最大可管理页+1
        mareap->ma_freepages++;    // 空闲页数+1
        mareap->ma_allmsadscnr++;  // 总页描述符数+1
    }
    return TRUE;
}



bool_t continumsadsc_mareabafh_core(memarea_t* mareap,msadsc_t** rfstat,msadsc_t** rfend,uint_t* rfmnr){
    // 参数有效性检查
    if(NULL == mareap || NULL == rfstat || NULL == rfend || NULL == rfmnr){
        return FALSE;
    }
    uint_t retval = *rfmnr;      // 当前剩余待处理页数
    uint_t tmpmnr = 0;          // 临时页数计数器
    msadsc_t *now_start = *rfstat;  // 当前处理块起始页
    msadsc_t *now_end = *rfend;     // 当前处理块结束页
    // 最少需要处理1页
    if(retval < 1){
        return FALSE;
    }
    // 查找合适的内存块管理结构 
    bafhlst_t *bafhp = find_continumsa_inbafhlst(mareap, retval);
    // 若返回为空
    if(bafhp == NULL){
        return FALSE;
    }
    // 若返回链表项错误
    if(retval < bafhp->af_oderpnr){
        return FALSE;
    }
    //处理可分割内存块（DIVP/DIVM状态）
    if ((bafhp->af_stus == BAFH_STUS_DIVP ||  bafhp->af_stus == BAFH_STUS_DIVM) && mareap->ma_type != MA_TYPE_PROC) {
        // 计算分割后的剩余页数
        tmpmnr = retval - bafhp->af_oderpnr; 

        // 将前N页（af_oderpnr页）加入管理结构
        // 参数说明：目标区域, 管理结构, 起始页, 结束页（now_start + N-1）, 页数
        /*
                            +--------+--------+--------+   +--------+--------+--------+
        bafhp->af_frelst -->| fstat  |  ...   |  fend  |-->| fstat  |  ...   |  fend  |....
                            +--------+--------+--------+   +--------+--------+--------+  
        +---------+
        | fstat   |
        |---------|   
        |md_odlink|-->+---------+   
        +---------+   | fend    |
                      |---------|   
                      |md_odlink|--> bafhp
                      +---------+ 
        */
        if (continumsadsc_add_bafhlst(mareap, bafhp, now_start, &now_start[bafhp->af_oderpnr - 1], bafhp->af_oderpnr) == FALSE) {
            return FALSE; // 添加失败（如链表满）
        }

        // 处理完成情况（无剩余页）
        if (tmpmnr == 0) {
            *rfmnr = 0;       // 剩余页清零
            *rfend = NULL;    // 结束指针置空
            return TRUE;
        }

       // 更新输出参数：移动起始指针到剩余块，更新剩余页数
        *rfstat = &now_start[bafhp->af_oderpnr]; // 新起始页 = 原起始页 + 分割页数
        *rfmnr = tmpmnr;                     // 剩余页数 = 原页数 - 分割页数
        return TRUE;
    }
    //处理单块（ONEM状态）
    if (bafhp->af_stus == BAFH_STUS_ONEM && 
        mareap->ma_type == MA_TYPE_PROC) {
        // 将整个块加入进程专用管理结构
        // 参数说明：进程内存区域, 管理结构, 起始页, 结束页, 总页数
    
        /*
        (dm_onemsalst)
                            +--------+      +--------+   +--------+          +--------+    +--------+      +--------+   +--------+          +--------+
        bafhp->af_frelst -->| fstat  |  --> |  f 1   |-->|   f2   |  ... --> |  fend  | -->| fstat  |  --> |  f 1   |-->|   f2   |  ... --> |  fend  |
                            +--------+      +--------+   +--------+          +--------+    +--------+      +--------+   +--------+          +--------+
        */
        if (continumsadsc_add_procmareabafh(mareap, bafhp, now_start, now_end, *rfmnr) == FALSE) {
           return FALSE; // 添加失败（如权限错误）
        }

        // 清空所有输出参数（已无剩余需要处理）
        *rfmnr = 0;     // 剩余页清零
        *rfend = NULL;  // 结束指针置空
        return TRUE;
    }

    return FALSE; // 未知状态或类型不匹配
}



bool_t merlove_continumsadsc_mareabafh(memarea_t* mareap,msadsc_t* mstat,msadsc_t* mend,uint_t mnr){ 
    // 参数有效性检查
    if(NULL == mareap || NULL == mstat || NULL == mend || mnr == 0){
        return FALSE;
    }
    uint_t mnridx = mnr;// 剩余待处理页数计数器
	msadsc_t *now_start = mstat, *now_end = mend;// 当前处理块起始页指针 当前处理块结束页指针
    /* 循环处理内存块直到完成或出错 */
    while ((mnridx > 0 && NULL != now_end)){
        // 调用核心处理函数操作当前内存块
        if (continumsadsc_mareabafh_core(mareap, &now_start, &now_end, &mnridx) == FALSE){
			system_error("continumsadsc_mareabafh_core fail\n");
		}
    }
    return TRUE; 
}



/* 内存区域初始化核心函数 */
bool_t merlove_mem_onmemarea(memarea_t *mareap, msadsc_t *mstat, uint_t msanr)
{
    // 参数有效性检查
    if (NULL == mareap || NULL == mstat || 0 == msanr) {
        return FALSE;
    }
    
    // 特殊内存类型处理
    if (MA_TYPE_SHAR == mareap->ma_type) {
        return TRUE;  // 共享内存区域无需初始化
    }
    if (MA_TYPE_INIT == mareap->ma_type) {
        return FALSE;  // 初始化状态区域视为异常
    }

    // 初始化扫描相关变量
    msadsc_t *retstatmsap = NULL;  // 连续块起始页指针
    msadsc_t *retendmsap = NULL;   // 连续块结束页指针
    msadsc_t *fntmsap = mstat;     // 页描述符首地址
    uint_t retfindmnr = 0;          // 找到的连续页数量
    uint_t fntmnr = 0;              // 已扫描页数计数器
    bool_t retscan = FALSE;         // 扫描操作返回值

    /* 主循环：遍历所有内存页 */
    for (; fntmnr < msanr;) {
        // 查找符合条件的连续空闲内存页块
        retscan = merlove_scan_continumsadsc(
            mareap,           // 目标内存区域
            fntmsap,          // 页描述符首地址
            &fntmnr,           // 已扫描页数（会被更新）
            msanr,            // 总页数
            &retstatmsap,      // 输出：连续块起始地址
            &retendmsap,       // 输出：连续块结束地址，包括此页
            &retfindmnr        // 输出：找到的连续页数，若只有一页则为1
        );

        // 扫描失败处理
        if (FALSE == retscan) {
            system_error("merlove_scan_continumsadsc fail\n"); // 致命错误直接终止
        }

        // 发现有效连续块时的处理流程
        if (NULL != retstatmsap && NULL != retendmsap) {
            /* 连续块完整性校验 */
            if (check_continumsadsc(mareap, retstatmsap, retendmsap, retfindmnr) == 0) {
                system_error("check_continumsadsc fail\n"); // 内存连续性验证失败
            }

            /* 执行内存块分配操作 */
            if (merlove_continumsadsc_mareabafh(
                mareap,        // 目标内存区域
                retstatmsap,   // 分配起始页
                retendmsap,    // 分配结束页
                retfindmnr     // 分配页数
            ) == FALSE) {
                system_error("merlove_continumsadsc_mareabafh fail\n"); // 分配操作失败
            }
        }
    }

    return TRUE;  // 区域初始化成功完成
}





bool_t merlove_mem_core(machbstart_t *mbsp) {
    // 转换物理地址为虚拟地址获取内存页描述符数组起点
    msadsc_t *mstatp = (msadsc_t *)phyadr_to_viradr((adr_t)mbsp->mb_memmappadr);
    // 获取系统内存页描述符总数
    uint_t msanr = (uint_t)mbsp->mb_memmapnr;
    // 初始化最大可用页计数器（实际未使用）
    uint_t maxp = 0;
    // 获取内存分区管理结构数组起点
    memarea_t *marea = (memarea_t *)phyadr_to_viradr((adr_t)mbsp->mb_memznpadr);
    // 初始化标志设置/验证返回值
    //0UL 表示无符号长整型的 0，取反操作符～会将 0 的所有位取反，64 位系统中则是 0xFFFFFFFFFFFFFFFF
    uint_t sretf = ~0UL, tretf = ~0UL;

    /* 第一阶段：遍历所有内存分区进行标志位操作验证 */
    for (uint_t mi = 0; mi < (uint_t)mbsp->mb_memznnr; mi++) {
        // 设置当前分区所有页面的内存管理标志
        sretf = merlove_setallmarflgs_onmemarea(&marea[mi], mstatp, msanr);
        // 异常处理：标志设置失败立即返回
        if ((~0UL) == sretf) {
            return FALSE;
        }
        // 验证设置的标志位是否符合预期
        tretf = test_setflgs(&marea[mi], mstatp, msanr);
        // 异常处理：标志验证失败立即返回
        if ((~0UL) == tretf) {
            return FALSE;
        }
        // 完整性检查：设置值与验证值必须完全一致
        if (sretf!= tretf) {
            return FALSE;
        }
    }

    /* 第二阶段：执行内存分区初始化操作 */
    for (uint_t maidx = 0; maidx < (uint_t)mbsp->mb_memznnr; maidx++) {
        // 对每个分区执行内存初始化操作
        if (merlove_mem_onmemarea(&marea[maidx], mstatp, msanr) == FALSE) {
            return FALSE;
        }
        // 累计各分区最大可用页数（注：结果未实际使用）
        maxp += marea[maidx].ma_maxpages;
    }

    // 所有操作成功完成返回TRUE
    return TRUE;
}


uint_t check_multi_msadsc(msadsc_t *mstat, bafhlst_t *bafhp, memarea_t *mareap)
{
    /*----------- 参数校验 -----------*/
    if (NULL == mstat || NULL == bafhp || NULL == mareap) {
        return 0;
    }

    /*----------- 标志位校验 -----------*/
    // 必须为块头或管理结构标记
    if (mstat->md_indxflgs.mf_olkty != MF_OLKTY_ODER &&
        mstat->md_indxflgs.mf_olkty != MF_OLKTY_BAFH) {
        return 0;
    }
    
    // 链接指针必须有效
    if (NULL == mstat->md_odlink) {
        return 0;
    }

    /*----------- 确定块范围 -----------*/
    msadsc_t *mend = NULL;
    if (mstat->md_indxflgs.mf_olkty == MF_OLKTY_ODER) {
        mend = (msadsc_t *)mstat->md_odlink; // 块结束页
    } else {
        mend = mstat; // 单页情况（自身为结束页）
    }
    if (NULL == mend) {
        return 0;
    }

    /*----------- 块尺寸校验 -----------*/
    uint_t actual_pages = (mend - mstat) + 1; // 计算实际连续页数
    if (actual_pages != bafhp->af_oderpnr) {
        return 0; // 实际页数与管理结构要求不符
    }

    /*----------- 结束页校验 -----------*/
    // 结束页必须标记为管理结构关联
    if (mend->md_indxflgs.mf_olkty != MF_OLKTY_BAFH) {
        return 0;
    }
    
    // 结束页必须正确指向管理结构
    if ((bafhlst_t *)(mend->md_odlink) != bafhp) {
        return 0;
    }

    /*----------- 单页特殊校验 -----------*/
    if (actual_pages == 1) {
        // 类型必须匹配区域类型
        if (mstat->md_indxflgs.mf_marty != (u32_t)mareap->ma_type) {
            return 0;
        }
        
        // 必须为未分配状态且用户索引为0
        if (mstat->md_phyadrs.paf_alloc != PAF_NO_ALLOC ||
            mstat->md_indxflgs.mf_uindx != 0) {
            return 0;
        }
        
        // 物理地址必须在区域地址范围内
        u64_t phy_addr = mstat->md_phyadrs.paf_padrs << PSHRSIZE;
        if (phy_addr < mareap->ma_logicstart || 
            (phy_addr + PAGESIZE - 1) > mareap->ma_logicend) {
            return 0;
        }
        return 1; // 单页检查通过
    }

    /*----------- 多页连续性校验 -----------*/
    uint_t valid_pages = 0;
    for (uint_t mi = 0; mi < actual_pages - 1; mi++) {
        // 当前页校验
        if (mstat[mi].md_indxflgs.mf_marty != (u32_t)mareap->ma_type ||
            mstat[mi].md_phyadrs.paf_alloc != PAF_NO_ALLOC ||
            mstat[mi].md_indxflgs.mf_uindx != 0) {
            return 0;
        }
        
        // 下一页校验
        if (mstat[mi + 1].md_phyadrs.paf_alloc != PAF_NO_ALLOC ||
            mstat[mi + 1].md_indxflgs.mf_uindx != 0) {
            return 0;
        }
        
        // 地址连续性校验（必须相邻）
        u64_t curr_addr = mstat[mi].md_phyadrs.paf_padrs << PSHRSIZE;
        u64_t next_addr = mstat[mi + 1].md_phyadrs.paf_padrs << PSHRSIZE;
        if ((curr_addr + PAGESIZE) != next_addr) {
            return 0;
        }
        
        // 地址范围校验
        if (curr_addr < mareap->ma_logicstart || 
            (next_addr + PAGESIZE - 1) > mareap->ma_logicend) {
            return 0;
        }
        valid_pages++;
    }
    return valid_pages + 1; // 返回实际验证通过的页数
}



bool_t check_one_bafhlst(bafhlst_t *bafhp, memarea_t *mareap)
{
    // 双指针校验
    if (NULL == bafhp || NULL == mareap) {
        return FALSE;
    }
    
    // 空链表直接通过（无管理对象）
    if (bafhp->af_mobjnr < 1 && bafhp->af_fobjnr < 1) {
        return TRUE;
    }

    uint_t list_count = 0;        // 实际遍历计数
    list_h_t *curr_node = NULL;   // 当前链表节点
    msadsc_t *curr_msd = NULL;    // 当前内存块描述符

    // 遍历空闲链表中的每个块
    list_for_each(curr_node, &bafhp->af_frelst) {
        curr_msd = list_entry(curr_node, msadsc_t, md_list);
        
        // 检查块尺寸是否符合预期（关键校验点）
        if (bafhp->af_oderpnr != check_multi_msadsc(curr_msd, bafhp, mareap)) {
            return FALSE;
        }
        list_count++;
    }

    // 验证链表节点数匹配计数器
    if (list_count != bafhp->af_fobjnr ||   // 空闲块数不匹配
        list_count != bafhp->af_mobjnr) {   // 总块数不匹配
        return FALSE;
    }

    return TRUE;
}


bool_t check_one_memarea(memarea_t *mareap)
{
    // 空指针检查
    if (NULL == mareap) {
        return FALSE;
    }
    
    // 最小页数验证（至少1页可管理）
    if (mareap->ma_maxpages < 1) {
        return TRUE; // 空区域视为有效
    }

    /* 检查所有内存块管理链表 (MDIVMER_ARR_LMAX 通常为不同块大小的管理链表数量) */
    for (uint_t li = 0; li < MDIVMER_ARR_LMAX; li++) {
        if (!check_one_bafhlst(&mareap->ma_mdmdata.dm_mdmlielst[li], mareap)) {
            return FALSE; // 块链表检查失败
        }
    }

    /* 检查单页专用链表 */
    if (!check_one_bafhlst(&mareap->ma_mdmdata.dm_onemsalst, mareap)) {
        return FALSE;
    }

    return TRUE;
}


void mem_check_mareadata(machbstart_t *mbsp)
{
    // 将物理地址转换为虚拟地址获取内存区域数组
    memarea_t *marea = (memarea_t *)phyadr_to_viradr((adr_t)mbsp->mb_memznpadr);
    
    // 遍历所有内存区域
    for (uint_t maidx = 0; maidx < mbsp->mb_memznnr; maidx++) {
        // 检查单个内存区域的完整性
        if (check_one_memarea(&marea[maidx]) == FALSE) {
            system_error("check_one_memarea fail\n");
        }
    }
    return;
}


void init_merlove_mem()
{
	if(merlove_mem_core(&kmachbsp) == FALSE)
	{
		system_error("init_merlove_mem fail");
	}
	return;
    mem_check_mareadata(&kmachbsp);
    return;
}




LKINIT void init_memarea()
{
	if (init_memarea_core(&kmachbsp) == FALSE)
	{
		system_error("init_memarea_core fail");
	}
	return;
}