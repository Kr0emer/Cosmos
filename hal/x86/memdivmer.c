#include "cosmostypes.h"
#include "cosmosmctrl.h"




memarea_t *onmrtype_retn_marea(memmgrob_t *mmobjp, uint_t mrtype) 
{
    // 遍历内存区域数组，数组长度由mo_mareanr字段指定
    for (uint_t mi = 0; mi < mmobjp->mo_mareanr; mi++)
    {
        // 比对当前内存区域的类型标识
        if (mrtype == mmobjp->mo_mareastat[mi].ma_type)
        {
            // 返回匹配区域的结构体地址
            return &mmobjp->mo_mareastat[mi];
        }
    }
    // 遍历完成后未找到匹配类型
    return NULL;
}

//返回当前可以分配的最大的空闲链表头管理结构体
bafhlst_t* onma_retn_maxbafhlst(memarea_t* malckp){
    for(uint_t li = (MDIVMER_ARR_LMAX - 1); li >=0; li++){
        if(malckp->ma_mdmdata.dm_mdmlielst[li].af_fobjnr > 0){
            return &malckp->ma_mdmdata.dm_mdmlielst[li];
        }
    }
    return NULL;
}

//从块管理链表中获取内存块
bool_t mm_retnmsaob_onbafhlst(bafhlst_t* bafhp,msadsc_t** retmstat,msadsc_t** retmend){
    if(bafhp == NULL || retmstat == NULL || retmend == NULL){
        return FALSE;
    }
    // 确保管理结构中有可用块（总块数和空闲块数均需≥1）
    if (bafhp->af_mobjnr < 1 || bafhp->af_fobjnr < 1) {
        *retmstat = NULL; // 清空输出参数
        *retmend = NULL;
        return FALSE;     // 可用内存块
    }

    // 检测空闲链表是否实际为空
    if (list_is_empty_careful(&bafhp->af_frelst) == TRUE) {
        *retmstat = NULL;
        *retmend = NULL;
        return FALSE;     //链表状态不一致
    }

    // 获取链表页描述符结点
    msadsc_t *tmp = list_entry(bafhp->af_frelst.next, msadsc_t, md_list);

    list_del(&tmp->md_list); //从链表中取出
    bafhp->af_mobjnr--;   // 总管理块数减1
    bafhp->af_fobjnr--;   // 空闲块数减1
    bafhp->af_freindx++;  // 释放计数加1

    *retmstat = tmp; // 起始页的描述符
    *retmend = (msadsc_t *)tmp->md_odlink;// 结束页的描述符

    if(tmp->md_indxflgs.mf_olkty == MF_OLKTY_BAFH)//如果只有一页既首页既为尾
    {
        *retmend = tmp;
    }
    return TRUE;
}
//校验以及设置msadsc_t参数
msadsc_t *mm_divpages_opmsadsc(msadsc_t *msastat, uint_t mnr)
{
    /*参数有效性校验 */
    if (msastat == NULL || mnr == 0) {
        return NULL; // 快速失败路径：非法输入
    }

    /*页描述符状态验证 */
    // 验证块类型必须为头或尾节点
    if ((msastat->md_indxflgs.mf_olkty != MF_OLKTY_ODER && 
         msastat->md_indxflgs.mf_olkty != MF_OLKTY_BAFH) ||
        // odlink指针必须有效
        msastat->md_odlink == NULL || 
        // 页帧必须处于未分配状态
        msastat->md_phyadrs.paf_alloc != PAF_NO_ALLOC) {
        system_error("mm_divpages_opmsadsc err1\n");
    }

    /*确定块结束位置 */
    msadsc_t *mend = (msadsc_t *)msastat->md_odlink;
    // 处理单页块特殊情况（头尾为同一页）
    if (msastat->md_indxflgs.mf_olkty == MF_OLKTY_BAFH) {
        mend = msastat; 
    }

    /*地址范围校验 */
    if (mend < msastat) { // 结束地址不能小于起始地址
        system_error("mm_divpages_opmsadsc err2\n");
    }

    // 验证实际页数是否匹配请求
    if ((uint_t)((mend - msastat) + 1) != mnr) {
        system_error("mm_divpages_opmsadsc err3\n");
    }

    /*分配计数器校验 */
    if (msastat->md_indxflgs.mf_uindx > (MF_UINDX_MAX - 1) || 
        mend->md_indxflgs.mf_uindx > (MF_UINDX_MAX - 1) ||
        msastat->md_indxflgs.mf_uindx != mend->md_indxflgs.mf_uindx) {
        system_error("mm_divpages_opmsadsc err4");
    }

    /*单页块处理 */
    if (mend == msastat) {
        msastat->md_indxflgs.mf_uindx++;       // 递增分配计数
        msastat->md_phyadrs.paf_alloc = PAF_ALLOC; // 标记为已分配
        msastat->md_indxflgs.mf_olkty = MF_OLKTY_ODER; // 转换为头节点
        msastat->md_odlink = mend;            // 维护自环指针
        return msastat; 
    }

    /*多页块处理 */
    //更新起始页元数据
    msastat->md_indxflgs.mf_uindx++;       // 头页分配计数+1
    msastat->md_phyadrs.paf_alloc = PAF_ALLOC; 
    //更新结束页元数据
    mend->md_indxflgs.mf_uindx++;          // 尾页分配计数+1 
    mend->md_phyadrs.paf_alloc = PAF_ALLOC;
    //设置块头标志和尾页指针
    //msastat->md_indxflgs.mf_olkty = MF_OLKTY_ODER; 
    //msastat->md_odlink = mend; 

    return msastat; // 返回起始页描述符
}


//分配当前区域能提供的最大连续块
msadsc_t* mm_maxdivpages_onmarea(memarea_t* malckp,uint_t* retrelpnr){
    if( malckp == NULL || retrelpnr == NULL ){//基础参数校验
        //*retrelpnr = 0;
        return NULL;
    }
    /*获取当前能获得的最大链表头管理结构体*/
    bafhlst_t *bafhp = onma_retn_maxbafhlst(malckp);
    if(bafhp == NULL){//是否有返回对应的空闲链表头
        *retrelpnr = 0;
        return NULL;
    }

    msadsc_t *retmsa = NULL;
    msadsc_t *retmstat = NULL, *retmend = NULL;
    /*bafhp中从获取内存块*/
    bool_t rets = mm_retnmsaob_onbafhlst(bafhp, &retmstat, &retmend);

    /*判断返回参数是否正确*/
    if(rets == FALSE || retmstat == NULL || retmend == NULL){
        *retrelpnr = 0;
        return NULL;
    }
    /*校验以及设置msadsc_t参数*/
    retmsa = mm_divpages_opmsadsc(retmstat, bafhp->af_oderpnr);

    if(retmsa == NULL){
        *retrelpnr = 0;
        return NULL;
    }
    *retrelpnr = bafhp->af_oderpnr;
    return retmsa;
}

 
bool_t scan_mapgsalloc_ok(memarea_t* malckp, uint_t pages) {
    // 参数有效性检查
    if (malckp == NULL || pages < 1) {
        return FALSE; // 无效参数（空指针或请求0页）
    }

    // 内存区域容量双重验证：
    // 1. 检查当前空闲页数是否足够 (ma_freepages)
    // 2. 检查区域最大容量是否允许 (ma_maxpages)
    if (malckp->ma_freepages >= pages &&    // 当前空闲页足够
        malckp->ma_maxpages >= pages) {     // 且不超过区域总容量
        return TRUE;  // 满足分配条件
    }

    // 若任意一个条件不满足
    return FALSE; // 资源不足或超过区域容量限制
}


KLINE sint_t retn_divoder(uint_t pages)  // 
{
    // 通过位扫描函数找到最高有效位的位置，并减1得到初始位数值
    // 例如：pages=8(1000) 时，search_64rlbits可能返回4，减1得到3（即log2(8)）
    sint_t pbits = search_64rlbits((uint_t)pages) - 1;    

    // 检测页数是否为2的幂：
    // 当 pages & (pages-1) != 0 时，说明存在多个低位1，不是2的幂
    if (pages & (pages - 1)) // 经典的2的幂检测方法
    {
        pbits++; // 非精确2的幂时需要增加一位来处理余数   
    }

    return pbits; // 返回最终计算出的位宽值  
}



bool_t onmpgs_retn_bafhlst(memarea_t *malckp, uint_t pages, bafhlst_t **retrelbafh, bafhlst_t **retdivbafh)
{
    // 前置防御性检查
    if (NULL == malckp || 1 > pages || NULL == retrelbafh || NULL == retdivbafh) {
        return FALSE; // 参数合法性检查失败
    }

    // 获取空闲链表数组（按内存块阶数组织）
    bafhlst_t *bafhstat = malckp->ma_mdmdata.dm_mdmlielst;

    // 计算请求页数对应的基础阶数（内存块大小级别）
    sint_t dividx = retn_divoder(pages);
    if (0 > dividx || MDIVMER_ARR_LMAX <= dividx) { //阶数索引越界检查
        *retrelbafh = NULL;
        *retdivbafh = NULL;
        return FALSE; //无效的阶数计算结果
    }

    // 检查请求页数是否超过当前阶的承载能力
    if (pages > bafhstat[dividx].af_oderpnr) { 
        *retrelbafh = NULL;
        *retdivbafh = NULL;
        return FALSE; // 当前阶无法满足需求
    }

    // 自计算阶开始向上搜索可用内存块
    for (sint_t idx = dividx; idx < MDIVMER_ARR_LMAX; idx++) {
        // 同时满足两个条件：
        // 1. 当前阶总页数足够（af_oderpnr）
        // 2. 当前阶有空闲内存块（af_fobjnr）
        if (bafhstat[idx].af_oderpnr >= pages && 0 < bafhstat[idx].af_fobjnr) {
            // 设置返回指针：
            // retrelbafh - 原始计算阶链表（可能用于后续内存分割）
            // retdivbafh - 实际分配阶链表
            *retrelbafh = &bafhstat[dividx];
            *retdivbafh = &bafhstat[idx];
            return TRUE; // 找到合适的内存块
        }
    }

    // 遍历完所有阶仍未找到合适块
    *retrelbafh = NULL;
    *retdivbafh = NULL;
    return FALSE;
}



bool_t mrdmb_add_msa_bafh(bafhlst_t *bafhp, msadsc_t *msastat, msadsc_t *msaend) 
{
    //--------------------- 1. 参数合法性检查 ---------------------
    if (NULL == bafhp || NULL == msastat || NULL == msaend) 
    {
        return FALSE; // 任何关键指针为NULL则直接拒绝
    }

    //--------------------- 2. 计算内存块数量并验证 -----------------
    uint_t mnr = (msaend - msastat) + 1;  // 计算从msastat到msaend的连续块数
    if (mnr != (uint_t)bafhp->af_oderpnr) 
    {
        return FALSE; // 内存块数量必须与分配器预期的块数严格一致
    }

    //--------------------- 3. 检查内存块使用状态 -------------------
    if (0 < msastat->md_indxflgs.mf_uindx || 0 < msaend->md_indxflgs.mf_uindx) 
    {
        return FALSE; // 起始或结束块若已被使用（mf_uindx>0），则不能加入空闲链表
    }

    //--------------------- 4. 验证物理地址分配状态 -----------------
    if (PAF_NO_ALLOC != msastat->md_phyadrs.paf_alloc ||
        PAF_NO_ALLOC != msaend->md_phyadrs.paf_alloc) 
    {
        return FALSE; // 起始或结束块必须处于未分配状态（PAF_NO_ALLOC）
    }

    //--------------------- 5. 处理单页的特殊情况 -------------------
    if (msastat == msaend && (1 != mnr || 1 != bafhp->af_oderpnr)) 
    {
        return FALSE; // 若为单页块，则数量和分配器预期值必须均为1
    }

    //--------------------- 6. 设置内存块链表关系 -------------------
    // 起始块标记为“顺序块”，并指向结束块（用于遍历）
    msastat->md_indxflgs.mf_olkty = MF_OLKTY_ODER; 
    msastat->md_odlink = msaend;

    // 结束块标记为“关联分配器”，并指向分配器头（便于归属判断）
    msaend->md_indxflgs.mf_olkty = MF_OLKTY_BAFH; 
    msaend->md_odlink = bafhp;

    //--------------------- 7. 更新分配器链表和统计 -----------------
    list_add(&msastat->md_list, &bafhp->af_frelst); // 将内存块插入空闲链表头部
    bafhp->af_mobjnr++;  // 总内存对象数+1
    bafhp->af_fobjnr++;  // 空闲内存对象数+1

    return TRUE; // 所有检查通过，操作成功
}


// 内存页面分割与回收的核心操作函数
msadsc_t *mm_reldpgsdivmsa_bafhl(
    memarea_t *malckp,     // 内存区域控制块
    uint_t pages,          // 请求操作页数
    uint_t *retrelpnr,     // 输出实际释放页数的指针
    bafhlst_t *relbfl,     // 理想状态
    bafhlst_t *divbfl)     // 实际现存的
{
    msadsc_t *retmsa = NULL;    // 最终返回的msadsc结构指针
    bool_t rets = FALSE;        // 临时操作状态
    msadsc_t *retmstat = NULL,  // 操作起始msadsc指针
            *retmend = NULL;    // 操作结束msadsc指针

    /* 参数有效性检查 */
    if (NULL == malckp || 1 > pages || 
        NULL == retrelpnr || NULL == relbfl || NULL == divbfl) {
        return NULL;
    }

    /* 链表顺序保护检查（回收链表不能高于分割链表） */
    if (relbfl > divbfl) {
        *retrelpnr = 0;         // 设置输出参数为0页
        return NULL;
    }

    /* 当目标链表与源链表相同时的直接处理 */
    if (relbfl == divbfl) {
        // 尝试从目标链表获取连续的msadsc块
        rets = mm_retnmsaob_onbafhlst(relbfl, &retmstat, &retmend);
        if (FALSE == rets || NULL == retmstat || NULL == retmend) {
            *retrelpnr = 0;
            return NULL;
        }

        // 验证获取的msadsc块数量是否符合链表描述
        if ((uint_t)((retmend - retmstat) + 1) != relbfl->af_oderpnr) {
            *retrelpnr = 0;
            return NULL;
        }

        // 校验以及设置msadsc_t参数
        retmsa = mm_divpages_opmsadsc(retmstat, relbfl->af_oderpnr);
        if (NULL == retmsa) {
            *retrelpnr = 0;
            return NULL;
        }

        *retrelpnr = relbfl->af_oderpnr; // 设置实际释放页数
        return retmsa;
    }

    /* 处理不同链表的分割操作 */
    // 从分割链表获取连续的msadsc块
    rets = mm_retnmsaob_onbafhlst(divbfl, &retmstat, &retmend);
    if (FALSE == rets || NULL == retmstat || NULL == retmend) {
        *retrelpnr = 0;
        return NULL;
    }

    // 验证获取块的数量一致性
    if ((uint_t)((retmend - retmstat) + 1) != divbfl->af_oderpnr) {
        *retrelpnr = 0;
        return NULL;
    }

    uint_t divnr = divbfl->af_oderpnr; // 获取分割链表的标准页数


    /* 逆向遍历链表层级，逐级分解内存块 */
    for (bafhlst_t *tmpbfl = divbfl - 1; tmpbfl >= relbfl; tmpbfl--) {   //32  16   8  *8
        // 分割后内存块并添加到对应层级的链表
        if (mrdmb_add_msa_bafh(tmpbfl, 
                             &retmstat[tmpbfl->af_oderpnr],
                             (msadsc_t *)retmstat->md_odlink) == FALSE) {
            system_error("mrdmb_add_msa_bafh fail\n"); // 系统级错误处理
        }
        // 更新当前块的链接指针
        retmstat->md_odlink = &retmstat[tmpbfl->af_oderpnr - 1];
        divnr -= tmpbfl->af_oderpnr; // 调整剩余需要处理的页数
    }

    /* 校验以及设置msadsc_t参数 */
    retmsa = mm_divpages_opmsadsc(retmstat, divnr);
    if (NULL == retmsa) {
        *retrelpnr = 0;
        return NULL;
    }

    *retrelpnr = relbfl->af_oderpnr; // 设置实际释放的页数
    return retmsa;
}

//分配指定页数的连续块
msadsc_t *mm_reldivpages_onmarea(memarea_t *malckp, uint_t pages, uint_t *retrelpnr)
{
    // 参数有效性检查
    if (NULL == malckp || 1 > pages || NULL == retrelpnr) {
        return NULL; // 存在无效参数直接返回空
    }

    // 检查内存区域是否可以分配指定数量的连续页面
    if (scan_mapgsalloc_ok(malckp, pages) == FALSE) {
        *retrelpnr = 0; // 设置返回页数为0
        return NULL;     // 分配检查不通过返回空
    }

    // 声明两个空闲链表指针
    bafhlst_t *retrelbhl = NULL, *retdivbhl = NULL;

    // 获取适合当前请求的空闲链表
    bool_t rets = onmpgs_retn_bafhlst(malckp, pages, &retrelbhl, &retdivbhl);
    if (FALSE == rets) {
        *retrelpnr = 0; // 设置返回页数为0
        return NULL;     // 获取空闲链表失败返回空
    }

    // 实际执行内存分割/释放操作
    uint_t retpnr = 0; // 实际操作的页数
    msadsc_t *retmsa = mm_reldpgsdivmsa_bafhl(malckp, pages, &retpnr, retrelbhl, retdivbhl);
    
    if (NULL == retmsa) { // 操作失败处理
        *retrelpnr = 0;   // 设置返回页数为0
        return NULL;
    }

    *retrelpnr = retpnr; // 通过参数返回实际操作的页数
    return retmsa;       // 返回操作后的内存描述符指针
}


void mm_update_memarea(memarea_t *malokp, uint_t pgnr, uint_t flgs)
{
    // 检查空指针，避免非法访问
    if (NULL == malokp)
    {
        return;
    }

    // 根据标志调整统计值
    if (0 == flgs)
    {
        // 分配页：减少空闲页计数，增加已分配页计数
        malokp->ma_freepages -= pgnr;
        malokp->ma_allocpages += pgnr;
    }
    if (1 == flgs)
    {
        // 释放页：增加空闲页计数，减少已分配页计数
        malokp->ma_freepages += pgnr;
        malokp->ma_allocpages -= pgnr;
    }
    return; // 显式返回（实际可省略）
}


void mm_update_memmgrob(uint_t realpnr, uint_t flgs)
{
    cpuflg_t cpuflg; // 用于保存中断状态

    if (0 == flgs)
    {
        // 分配页操作：加锁、更新统计值、解锁（防止多线程竞争）
        knl_spinlock_cli(&memmgrob.mo_lock, &cpuflg);  // 禁用中断并获取锁
        memmgrob.mo_alocpages += realpnr;             // 增加全局分配页计数
        memmgrob.mo_freepages -= realpnr;             // 减少全局空闲页计数
        knl_spinunlock_sti(&memmgrob.mo_lock, &cpuflg); // 释放锁并恢复中断
    }
    if (1 == flgs)
    {
        // 释放页操作：加锁、更新统计值、解锁
        knl_spinlock_cli(&memmgrob.mo_lock, &cpuflg);
        memmgrob.mo_alocpages -= realpnr;             // 减少全局分配页计数
        memmgrob.mo_freepages += realpnr;             // 增加全局空闲页计数
        knl_spinunlock_sti(&memmgrob.mo_lock, &cpuflg);
    }
    return; // 显式返回（实际可省略）
}



msadsc_t *mm_divpages_core(memarea_t *mareap, uint_t pages, 
                          uint_t *retrealpnr, uint_t flgs)
{
    uint_t retpnr = 0;           // 实际分配的页数
    msadsc_t *retmsa = NULL;     // 返回的页描述符指针
    cpuflg_t cpuflg;             // CPU中断状态保存变量

    /*-- 分配标志合法性检查 --*/
    if (DMF_RELDIV != flgs && DMF_MAXDIV != flgs) {
        *retrealpnr = 0;         // 清零输出参数
        return NULL;             // 无效分配策略
    }

    /*-- 内存区域类型检查 --*/
    // 仅允许内核(MA_TYPE_KRNL)和硬件相关(MA_TYPE_HWAD)内存区域
    if (MA_TYPE_KRNL != mareap->ma_type && 
        MA_TYPE_HWAD != mareap->ma_type) {
        *retrealpnr = 0;
        return NULL;
    }

    /*-- 临界区保护 --*/
    knl_spinlock_cli(&mareap->ma_lock, &cpuflg); // 关中断获取自旋锁

    /*-- 分配策略路由 --*/
    if (DMF_MAXDIV == flgs) {
        // 最大化分配模式：分配当前区域能提供的最大连续块
        retmsa = mm_maxdivpages_onmarea(mareap, &retpnr);
        goto ret_step;
    }
    if (DMF_RELDIV == flgs) {
        // 按需分配模式：尝试分配指定页数的连续块
        retmsa = mm_reldivpages_onmarea(mareap, pages, &retpnr);
        goto ret_step;
    }

    /*-- 默认失败路径 --*/
    retmsa = NULL;
    retpnr = 0;

ret_step:
    /*-- 分配成功后的更新操作 --*/
    if (NULL != retmsa && 0 != retpnr) {
        // 更新内存区域统计信息
        mm_update_memarea(mareap, retpnr, 0);    // 参数0表示减少可用页数
        // 更新全局内存管理统计
        mm_update_memmgrob(retpnr, 0);           // 参数0表示增加已分配页数
    }

    /*-- 释放锁并恢复中断 --*/
    knl_spinunlock_sti(&mareap->ma_lock, &cpuflg);

    /*-- 设置返回参数 --*/
    *retrealpnr = retpnr;    // 通过指针返回实际分配的页数
    return retmsa;           // 返回首个页描述符指针
}







//内存分配页面框架函数
msadsc_t *mm_divpages_fmwk(memmgrob_t *mmobjp, uint_t pages, 
                          uint_t *retrelpnr, uint_t mrtype, uint_t flgs)
{
    /*根据内存类型获取内存区域*/
    // 通过内存类型（如内核/进程）找到对应的内存区域管理结构
    memarea_t *marea = onmrtype_retn_marea(mmobjp, mrtype);
    
    // 内存区域不存在时立即返回失败
    if (marea == NULL) {
        *retrelpnr = 0;      // 明确设置输出参数
        return NULL;         // 错误码：无效内存类型
    }

    /*=============== 调用核心分配函数 ===============*/
    uint_t retpnr = 0;       // 实际分配页数（临时变量）
    msadsc_t *retmsa = mm_divpages_core(marea, pages, &retpnr, flgs);
    
    // 分配失败处理
    if (retmsa == NULL) {
        *retrelpnr = 0;      // 清零输出参数
        return NULL;         // 错误码：内存不足或其他错误
    }

    /*=============== 设置返回值 ===============*/
    *retrelpnr = retpnr;     // 通过指针返回实际分配的页数
    return retmsa;           // 返回首个页描述符指针
}









//mmobjp->内存管理数据结构指针
//pages->请求分配的内存页面数
//retrealpnr->存放实际分配内存页面数的指针
//mrtype->请求的分配内存页面的内存区类型
//flgs->请求分配的内存页面的标志位

//内存分配页面接口
msadsc_t *mm_division_pages(memmgrob_t *mmobjp, uint_t pages, 
                           uint_t *retrealpnr, uint_t mrtype, uint_t flgs)
{
    /*=============== 参数校验 ===============*/
    // 检查必要参数的有效性
    if (mmobjp == NULL || retrealpnr == NULL || mrtype == 0) {
        // 错误码说明：
        // - mmobjp为空：内存管理对象未初始化
        // - retrealpnr为空：调用方未提供结果存储指针
        // - mrtype为0：未指定有效内存类型
        return NULL;
    }

    uint_t retpnr = 0;       // 实际分配页数（临时变量）
    msadsc_t *retmsa = NULL; // 分配结果指针

    /*=============== 调用框架函数 ===============*/
    retmsa = mm_divpages_fmwk(mmobjp, pages, &retpnr, mrtype, flgs);

    /*=============== 分配结果处理 ===============*/
    if (retmsa == NULL) {
        // 分配失败时明确返回参数状态
        *retrealpnr = 0;     // 输出参数清零
        return NULL;
    }

    // 分配成功时设置输出参数
    *retrealpnr = retpnr;    // 返回实际获得的页数
    
    return retmsa;           // 返回首个页描述符
}



bool_t scan_freemsa_isok(msadsc_t *freemsa, uint_t freepgs)
{
    // 检查输入参数是否有效
    if (NULL == freemsa || 1 > freepgs)
    {
        return FALSE;
    }

    // 检查 freemsa 的类型是否为 MF_OLKTY_ODER，且 md_odlink 是否有效，且 mf_uindx 是否大于 0
    if (MF_OLKTY_ODER != freemsa->md_indxflgs.mf_olkty ||
        NULL == freemsa->md_odlink || 1 > freemsa->md_indxflgs.mf_uindx)
    {
        return FALSE;
    }

    // 获取 freemsa 的结束描述符
    msadsc_t *end = (msadsc_t *)freemsa->md_odlink;

    // 检查 freemsa 和 end 的物理地址分配状态是否为 PAF_ALLOC
    if (PAF_ALLOC != freemsa->md_phyadrs.paf_alloc ||
        PAF_ALLOC != end->md_phyadrs.paf_alloc ||
        1 > end->md_indxflgs.mf_uindx)
    {
        return FALSE;
    }

    // 检查 freemsa 和 end 之间的距离是否等于 freepgs
    if (((uint_t)((end - freemsa) + 1)) != freepgs)
    {
        return FALSE;
    }

    // 如果所有检查都通过，返回 TRUE
    return TRUE;
}




/* 内存页面合并操作核心函数 */
sint_t mm_merpages_opmsadsc(bafhlst_t *bafh, msadsc_t *freemsa, uint_t freepgs)
{
    // 基础参数有效性检查
    if (NULL == bafh || NULL == freemsa || 1 > freepgs)
    {
        return 0; // 参数非法直接返回失败
    }

    /*---------------------
     * 内存块结构验证阶段
     * 注意：以下错误直接触发系统级错误中断
     --------------------*/
    
    // 校验内存块是否在有序链表中（要求OLKTY_ODER类型且存在后续链接）
    if (MF_OLKTY_ODER != freemsa->md_indxflgs.mf_olkty ||
        NULL == freemsa->md_odlink)
    {
        system_error("mm_merpages_opmsadsc err1\n"); // 错误点1：内存块链接类型异常
    }

    // 获取内存块末端描述符并验证地址顺序
    msadsc_t *fmend = (msadsc_t *)freemsa->md_odlink;
    if (fmend < freemsa)
    {
        system_error("mm_merpages_opmsadsc err2\n"); // 错误点2：地址倒置错误
    }

    // 验证页面数量一致性
    if (bafh->af_oderpnr != freepgs ||
        ((uint_t)(fmend - freemsa) + 1) != freepgs)
    {
        system_error("mm_merpages_opmsadsc err3\n"); // 错误点3：页面数不一致
    }

    // 验证起始页分配状态和引用计数
    if (PAF_NO_ALLOC == freemsa->md_phyadrs.paf_alloc ||
        1 > freemsa->md_indxflgs.mf_uindx)
    {
        system_error("mm_merpages_opmsadsc err4\n"); // 错误点4：起始页状态异常
    }

    // 验证结束页分配状态和引用计数
    if (PAF_NO_ALLOC == fmend->md_phyadrs.paf_alloc ||
        1 > fmend->md_indxflgs.mf_uindx)
    {
        system_error("mm_merpages_opmsadsc err5\n"); // 错误点5：结束页状态异常
    }

    // 验证首尾页引用计数一致性
    if (freemsa->md_indxflgs.mf_uindx != fmend->md_indxflgs.mf_uindx)
    {
        system_error("mm_merpages_opmsadsc err6\n"); // 错误点6：引用计数不匹配
    }

    /*---------------------
     * 内存块合并操作阶段
     * 注意：此处开始执行实际内存状态修改
     --------------------*/
    
    // 单一页面处理分支
    if (freemsa == fmend)
    {
        freemsa->md_indxflgs.mf_uindx--; //减少引用计数
        
        // 判断是否还存在引用
        if (0 < freemsa->md_indxflgs.mf_uindx)
        {
            return 1; // 仍有引用不释放
        }
        
        // 标记为未分配状态，转为自由链表管理
        freemsa->md_phyadrs.paf_alloc = PAF_NO_ALLOC;
        freemsa->md_indxflgs.mf_olkty = MF_OLKTY_BAFH;
        freemsa->md_odlink = bafh;
        return 2; // 需要执行合并操作
    }

    //--------------- 多页面处理分支 ---------------//
    // 减少首尾页引用计数
    freemsa->md_indxflgs.mf_uindx--;
    fmend->md_indxflgs.mf_uindx--;

    // 判断是否全页面均无引用
    if (0 < freemsa->md_indxflgs.mf_uindx)
    {
        return 1; // 仍有引用保持状态
    }

    // 完全释放内存块的重置操作
    freemsa->md_phyadrs.paf_alloc = PAF_NO_ALLOC;
    fmend->md_phyadrs.paf_alloc = PAF_NO_ALLOC;
    
    // 重构内存块链接关系
    freemsa->md_indxflgs.mf_olkty = MF_OLKTY_ODER;
    freemsa->md_odlink = fmend;
    fmend->md_indxflgs.mf_olkty = MF_OLKTY_BAFH;
    fmend->md_odlink = bafh;
    
    return 2; // 触发合并流程
}




bool_t mpobf_add_msadsc(bafhlst_t *bafhp, msadsc_t *freemstat, msadsc_t *freemend)
{
    //============ 参数校验阶段 ============//
    // 空指针保护
    if (NULL == bafhp || NULL == freemstat || NULL == freemend) {
        return FALSE;
    }
    
    // 地址范围有效性验证（起始地址不能大于结束地址）
    if (freemend < freemstat) {
        return FALSE;
    }
    
    // 验证内存块跨度与链表记录是否匹配（例如链表记录管理4页内存块，实际跨度也必须为4页）
    if (bafhp->af_oderpnr != ((uint_t)(freemend - freemstat) + 1)) {
        return FALSE;
    }

    //============ 计数器溢出保护 ============//
    // 检查管理链表对象计数是否达到最大值（~0UL表示无符号长整型的最大值）
    if ((~0UL) <= bafhp->af_fobjnr || (~0UL) <= bafhp->af_mobjnr) {
        system_error("(~0UL)<=bafhp->af_fobjnr\n"); // 系统级致命错误
        return FALSE;
    }

    //============ 内存块标记阶段 ============//
    // 设置内存块起始描述符
    freemstat->md_indxflgs.mf_olkty = MF_OLKTY_ODER; // 标记为有序内存块
    freemstat->md_odlink = freemend;                 // 指向结束描述符
    
    // 设置内存块结束描述符
    freemend->md_indxflgs.mf_olkty = MF_OLKTY_BAFH; // 标记属于buddy链表管理
    freemend->md_odlink = bafhp;                    // 指向所属管理链表

    //============ 链表操作阶段 ============//
    // 将起始描述符加入管理链表的空闲列表（假设list_add为内核链表操作函数）
    list_add(&freemstat->md_list, &bafhp->af_frelst);
    
    //============ 统计更新阶段 ============//
    bafhp->af_fobjnr++; // 增加空闲对象计数（free object number）
    bafhp->af_mobjnr++; // 增加总对象计数（managed object number）
    
    return TRUE;
}
bool_t onfpgs_retn_bafhlst(memarea_t *malckp, uint_t freepgs, bafhlst_t **retrelbf, bafhlst_t **retmerbf)
{
    //============ 前置校验阶段 ============//
    // 空指针及非法页数检测
    if (NULL == malckp || 1 > freepgs || NULL == retrelbf || NULL == retmerbf)
    {
        return FALSE;
    }

    //============ 获取管理链表数组 ============//
    // 从内存区域获取伙伴系统分级管理链表数组
    bafhlst_t *bafhstat = malckp->ma_mdmdata.dm_mdmlielst; // 分级链表起点
    
    //============ 确定内存块级别 ============//
    sint_t dividx = retn_divoder(freepgs); // 核心分阶算法（假设返回层级索引）
    if (0 > dividx || MDIVMER_ARR_LMAX <= dividx) // 指数边界检查
    {
        *retrelbf = NULL; // 清除输出参数
        *retmerbf = NULL;
        return FALSE;
    }

    //============ 计数器容灾保护 ============//
    // 预防计数器溢出（例如长期运行系统的计数器回卷）
    if ((~0UL) <= bafhstat[dividx].af_mobjnr) // 总对象数即将溢出
    {
        system_error("onfpgs_retn_bafhlst af_mobjnr max"); // 系统级告警
    }
    if ((~0UL) <= bafhstat[dividx].af_fobjnr) // 空闲对象数即将溢出
    {
        system_error("onfpgs_retn_bafhlst af_fobjnr max");
    }

    //============ 一致性验证阶段 ============//
    // 验证页数和管理层记录是否匹配
    if (freepgs != bafhstat[dividx].af_oderpnr)
    {
        *retrelbf = NULL;
        *retmerbf = NULL;
        return FALSE;
    }

    //============ 设置输出参数 ============//
    *retrelbf = &bafhstat[dividx];            // 当前释放级别链表
    *retmerbf = &bafhstat[MDIVMER_ARR_LMAX - 1]; // 最大合并级别链表
    
    return TRUE;
}

/*
 * 单块内存状态校验函数
 * 作用：验证一个内存块是否符合伙伴系统合并要求
 * 参数：
 *   bafh  - 所属的buddy管理链表
 *   _1ms  - 内存块起始描述符
 *   _1me  - 内存块结束描述符
 * 返回值：
 *   0 - 非法内存块
 *   2 - 合法空闲可合并内存块
 */
sint_t mm_cmsa1blk_isok(bafhlst_t *bafh, msadsc_t *_1ms, msadsc_t *_1me)
{
    /*======= 基础参数校验 =======*/
    // 输入指针非空检查
    if (NULL == bafh || NULL == _1ms || NULL == _1me) { 
        return 0; 
    }
    
    // 地址顺序检查（结束地址不能大于起始地址）
    if (_1me < _1ms) { 
        return 0; 
    }

    /*======= 单页内存块处理 =======*/
    if (_1ms == _1me) { 
        //-- 标记类型验证：必须是自由链表块类型 --//
        if (MF_OLKTY_BAFH != _1me->md_indxflgs.mf_olkty) { 
            return 0;
        }
        
        //-- 归属链表验证：必须属于当前管理链表 --//
        if (bafh != (bafhlst_t *)_1me->md_odlink) { 
            return 0;
        }
        
        //-- 分配状态验证：必须处于未分配状态 --//
        if (PAF_NO_ALLOC != _1me->md_phyadrs.paf_alloc) {
            return 0; 
        }
        
        //-- 引用计数验证：引用必须归零 --//
        if (0 != _1me->md_indxflgs.mf_uindx) { 
            return 0; 
        }
        
        //-- 地址跨度验证：物理地址差 = 描述符索引差 --//
        if ((_1me->md_phyadrs.paf_padrs - _1ms->md_phyadrs.paf_padrs) 
            != (uint_t)(_1me - _1ms)) {
            return 0;
        }
        return 2; // 单页内存块校验通过
    }

    /*======= 多页内存块处理 =======*/
    //-- 头描述符验证 --//
    // 类型必须为有序块
    if (MF_OLKTY_ODER != _1ms->md_indxflgs.mf_olkty) { 
        return 0; 
    }
    
    // 必须正确链接到结尾描述符
    if (_1me != (msadsc_t *)_1ms->md_odlink) { 
        return 0; 
    }
    
    // 头页必须处于未分配状态
    if (PAF_NO_ALLOC != _1ms->md_phyadrs.paf_alloc) {
        return 0;
    }
    
    // 头页引用必须为零
    if (0 != _1ms->md_indxflgs.mf_uindx) { 
        return 0; 
    }

    //-- 尾描述符验证 --//
    // 类型必须为buddy管理块
    if (MF_OLKTY_BAFH != _1me->md_indxflgs.mf_olkty) { 
        return 0; 
    }
    
    // 必须属于当前管理链表
    if (bafh != (bafhlst_t *)_1me->md_odlink) { 
        return 0; 
    }
    
    // 尾页必须处于未分配状态
    if (PAF_NO_ALLOC != _1me->md_phyadrs.paf_alloc) {
        return 0;
    }
    
    // 尾页引用必须为零
    if (0 != _1me->md_indxflgs.mf_uindx) { 
        return 0;
    }
    
    //-- 地址连续性二次验证 --
    if ((_1me->md_phyadrs.paf_padrs - _1ms->md_phyadrs.paf_padrs)
        != (uint_t)(_1me - _1ms)) {
        return 0;
    }
    return 2; // 多页内存块校验通过
}

sint_t mm_cmsa2blk_isok(bafhlst_t *bafh, msadsc_t *_1ms, msadsc_t *_1me, 
                        msadsc_t *_2ms, msadsc_t *_2me)
{
    //======= 参数有效性护盾 =======//
    if (NULL == bafh || NULL == _1ms || NULL == _1me || 
        NULL == _2ms || NULL == _2me || _1ms == _2ms || _1me == _2me)
    {
        return 0; // 防御性设计阻止无效参数
    }

    //======= 内存块状态双重验证 =======//
    sint_t ret1s = 0, ret2s = 0;
    ret1s = mm_cmsa1blk_isok(bafh, _1ms, _1me); // 块1状态校验
    if (0 == ret1s) {
        system_error("块1地址区间验证失败\n");    // 触发内核级错误记录
    }
    
    ret2s = mm_cmsa1blk_isok(bafh, _2ms, _2me); // 块2状态校验
    if (0 == ret2s) {
        system_error("块2地址区间验证失败\n");
    }

    //======= 合并条件核心算法 =======//
    if (2 == ret1s && 2 == ret2s) { // 两内存块均为空闲可合并状态
        /* 参数含义：
         * ret1s=2 表示块1是可合并的空闲块
         * ret2s=2 表示块2是可合并的空闲块 */

        //---- 块1在低地址的情况 ----//
        if (_1ms < _2ms && _1me < _2me) { 
            // 虚拟地址连续性校验
            if ((_1me + 1) != _2ms) { 
                return 1; // 虚拟地址不连续
            }
            // 物理地址连续性校验
            if ((_1me->md_phyadrs.paf_padrs + 1) != _2ms->md_phyadrs.paf_padrs) {
                return 1; // 物理地址不连续
            }
            return 2; // 符合前向合并条件
        }

        //---- 块2在低地址的情况 ----//
        if (_1ms > _2ms && _1me > _2me) {
            // 虚拟地址连续性校验
            if ((_2me + 1) != _1ms) { 
                return 1;
            }
            // 物理地址连续性校验
            if ((_2me->md_phyadrs.paf_padrs + 1) != _1ms->md_phyadrs.paf_padrs) {
                return 1;
            }
            return 4; // 符合后向合并条件
        }
    }
    
    return 0; // 其他所有不满足合并条件的情况
}

bool_t mm_clear_2msaolflg(bafhlst_t *bafh, msadsc_t *_1ms, msadsc_t *_1me, 
                          msadsc_t *_2ms, msadsc_t *_2me)
{
    //=============== 防御式编程 ===============//
    // 校验所有输入指针的有效性
    if (NULL == bafh || NULL == _1ms || NULL == _1me || 
        NULL == _2ms || NULL == _2me || 
        _1ms == _2ms || _1me == _2me)
    {
        return FALSE; // 防止空指针或重叠块导致内存损坏
    }

    //============ 中间描述符重置阶段 =============//
    // 清除原块1尾部的链表信息（MF_OLKTY_INIT表示中间过渡态）
    _1me->md_indxflgs.mf_olkty = MF_OLKTY_INIT; // 类型重置
    _1me->md_odlink = NULL;                      // 断开原有链接
    
    // 清除原块2头部的链表信息（准备被合并的部分）
    _2ms->md_indxflgs.mf_olkty = MF_OLKTY_INIT; // 类型重置
    _2ms->md_odlink = NULL;                      // 断开原有链接

    //============ 新合并块构建阶段 =============//
    /* 新合并块的描述符结构：
     * _1ms -> [新模式头部]  类型：MF_OLKTY_ODER（有序块开始头）
     * _2me -> [新管理块尾]  类型：MF_OLKTY_BAFH（连接到管理链表）
     */
    // 设置新块起始描述符
    _1ms->md_indxflgs.mf_olkty = MF_OLKTY_ODER; // 标记为有序内存块起始
    _1ms->md_odlink = _2me;                     // 直指新合并块的终点
    
    // 设置新块结束描述符
    _2me->md_indxflgs.mf_olkty = MF_OLKTY_BAFH; // 标记为链表管理节点
    _2me->md_odlink = bafh;                     // 链接到所在管理链表

    return TRUE; // 成功完成链接重构
}

/*
 * 合并后的内存块完整性校验函数
 * 功能：验证合并后的内存块描述符状态是否正确
 * 返回：TRUE-验证通过 / FALSE-存在异常
 */
bool_t chek_cl2molkflg(bafhlst_t *bafh, msadsc_t *_1ms, msadsc_t *_1me,
                       msadsc_t *_2ms, msadsc_t *_2me)
{
    //======= 防御性参数检查 =======//
    /* 检查所有关键指针有效性 */
    if (NULL == bafh || NULL == _1ms || NULL == _1me ||
        NULL == _2ms || NULL == _2me ||
        _1ms == _2ms || _1me == _2me)
    {
        return FALSE; // 基本参数不合法
    }

    //======= 合并尺寸验证 =======//
    /* 通过地址差计算实际内存块大小 */
    uint_t merge_pages = (uint_t)(_2me - _1ms) + 1;
    /* 对比管理链表的预期块大小 */
    if (merge_pages != bafh->af_oderpnr) {
        return FALSE; // 合并后的页数与当前阶不匹配
    }

    //======= 单页块合并特判 =======//
    if (_1ms == _1me && _2ms == _2me) { // 两个都是单页块
        // 验证新合并块起始描述符属性
        if ((_1ms->md_indxflgs.mf_olkty != MF_OLKTY_ODER) ||  // 必须是链表起始标志
            ((msadsc_t*)_1ms->md_odlink != _2me))             // 必须指向正确的尾部
        {
            return FALSE;
        }
        
        // 验证新合并块尾部描述符属性
        if ((_2me->md_indxflgs.mf_olkty != MF_OLKTY_BAFH) ||  // 必须是链表归属标志
            ((bafhlst_t*)_2me->md_odlink != bafh))            // 必须属于当前管理链表
        {
            return FALSE;
        }
        return TRUE; // 单页合并验证通过
    }

    //======= 多页块通用验证 =======//
    /* 块首属性校验 */
    if ((_1ms->md_indxflgs.mf_olkty != MF_OLKTY_ODER) ||      // 起始标志校验
        ((msadsc_t*)_1ms->md_odlink != _2me))                 // odlink指向合并后的新尾
    {
        return FALSE;
    }

    /* 中间标志清除校验（块1尾） */
    if ((_1me->md_indxflgs.mf_olkty != MF_OLKTY_INIT) ||      // 旧尾部应置为初始化态
        (_1me->md_odlink != NULL))                            // 链接必须断开
    {
        return FALSE;
    }

    /* 中间标志清除校验（块2首） */
    if ((_2ms->md_indxflgs.mf_olkty != MF_OLKTY_INIT) ||      // 旧头应置为初始化态
        (_2ms->md_odlink != NULL))                            // 链接必须断开
    {
        return FALSE;
    }

    /* 新尾部属性校验 */
    if ((_2me->md_indxflgs.mf_olkty != MF_OLKTY_BAFH) ||      // 必须为链表尾标志
        ((bafhlst_t*)_2me->md_odlink != bafh))                // 必须指向正确链表
    {
        return FALSE;
    }

    return TRUE; // 多页合并验证通过
}

sint_t mm_find_cmsa2blk(bafhlst_t *fbafh, msadsc_t **rfnms, msadsc_t **rfnme)
{
    //============ 参数校验 ============//
    if (NULL == fbafh || NULL == rfnms || NULL == rfnme) 
    {
        return 0; // 基础参数异常
    }
    
    //============ 初始化指针 ============//
    msadsc_t *freemstat = *rfnms;   // 待合并块起始指针
    msadsc_t *freemend = *rfnme;    // 待合并块结束指针
    
    //============ 快速失败检查 ============//
    if (fbafh->af_fobjnr < 1)       // 当前阶无空闲块可合并
    { 
        return 1; 
    }

    //============ 遍历空闲链表 ============//
    list_h_t *tmplst = NULL;        // 临时链表指针
    msadsc_t *tmpmsa = NULL;        // 临时内存描述符指针
    msadsc_t *blkms = NULL, *blkme = NULL; // 找到的伙伴块起止地址
    sint_t rets = 0;                // 操作结果缓存
    
    list_for_each(tmplst, &fbafh->af_frelst)  // 遍历当前阶的空闲链表
    { 
        tmpmsa = list_entry(tmplst, msadsc_t, md_list); // 获取描述符结构
        
        //============ 核心检测调用 ============//
        rets = mm_cmsa2blk_isok(fbafh, freemstat, freemend, 
                                tmpmsa, &tmpmsa[fbafh->af_oderpnr - 1]);
        
        //============ 处理检测结果 ============//
        if (2 == rets || 4 == rets) {    // 寻址到低地址或高地址伙伴
            blkms = tmpmsa;              // 记录伙伴块地址
            blkme = &tmpmsa[fbafh->af_oderpnr - 1]; // 计算结束地址
            
            //---------- 链表操作 ----------//
            list_del(&tmpmsa->md_list);  // 从当前阶链表移除伙伴块
            fbafh->af_fobjnr--;          // 更新空闲块计数
            fbafh->af_mobjnr--;          // 更新总块计数
            goto step1;                  // 跳转合并处理
        }
    }

step1:
    //============ 合并结果分支 ============//
    if (0 == rets || 1 == rets)         // 无合适块或结构异常 
    { 
        return 1; 
    }

    //--------------- 前向合并处理（新增块在目标块低地址） ---------------//
    if (2 == rets) {
        if (mm_clear_2msaolflg(fbafh + 1, freemstat, freemend, blkms, blkme)
            && chek_cl2molkflg(fbafh + 1, freemstat, freemend, blkms, blkme)) 
        {
            *rfnms = freemstat;  // 新块起始地址不变
            *rfnme = blkme;      // 结束地址扩展到伙伴块尾
            return 2;
        }
        return 0;                // 标志清除失败
    }

    //--------------- 后向合并处理（新增块在目标块高地址） ---------------//
    if (4 == rets) {
        if (mm_clear_2msaolflg(fbafh + 1, blkms, blkme, freemstat, freemend) 
            && chek_cl2molkflg(fbafh + 1, blkms, blkme, freemstat, freemend)) 
        {
            *rfnms = blkms;      // 新块起始地址调整为伙伴块首
            *rfnme = freemend;   // 结束地址保持不变
            return 2;
        }
        return 0;                // 标志清除失败
    }

    return 0; // 未预料的返回值
}



bool_t mm_merpages_onbafhlst(msadsc_t *freemsa, uint_t freepgs, 
                            bafhlst_t *relbf, bafhlst_t *merbf)
{
    sint_t rets = 0;
    /* 计算内存块物理边界 */
    msadsc_t *mnxs = freemsa;                    // 起始描述符指针
    msadsc_t *mnxe = &freemsa[freepgs - 1];      // 结束描述符指针
    
    /* 从当前分配链表开始遍历到最大可合并链表 */
    bafhlst_t *tmpbf = relbf;                    // 当前处理的管理链表指针
    for (; tmpbf < merbf; tmpbf++)                // 逐级提升合并层级
    {
        /* 在当前层级寻找可合并区块 */
        rets = mm_find_cmsa2blk(tmpbf, &mnxs, &mnxe);
        if (1 == rets) {                        // 成功找到合并位置
            break;
        }
        if (0 == rets) {                        // 出现内部数据错误
            system_error("内存块结构验证失败\n"); // 严重数据损坏告警
        }
    }

    /* 将最终合并的区块添加到对应管理链表 */
    if (mpobf_add_msadsc(tmpbf, mnxs, mnxe) == FALSE) {
        return FALSE;                           // 链表操作失败
    }
    return TRUE;                                // 合并操作完成
}



// 将指定数量的空闲内存页面合并到所属的内存区域中
// 参数：
// malckp  - 目标内存区域指针
// freemsa - 待合并的空闲内存页面描述符数组
// freepgs - 待合并的页面数量
// 返回值：合并是否成功
bool_t mm_merpages_onmarea(memarea_t* malckp, msadsc_t* freemsa, uint_t freepgs)
{
    //========== 参数有效性校验 ==========//
    if (NULL == malckp || NULL == freemsa || 1 > freepgs) {
        return FALSE; // 非法参数直接返回失败
    }

    //========== 处理进程类型内存区域 ==========//
    bafhlst_t* prcbf = NULL;  // 页面管理链表指针
    sint_t pocs = 0;          // 页面操作状态

    if (MA_TYPE_PROC == malckp->ma_type) {  // 如果是进程类型的内存区域
        // 获取单个内存页面的管理链表
        prcbf = &malckp->ma_mdmdata.dm_onemsalst;  
        // 查看是否可以收回并设置值
        pocs = mm_merpages_opmsadsc(prcbf, freemsa, freepgs);

        
        if (2 == pocs) {  // 需要执行合并操作的情况
            // 将内存块添加到链表（包含起点和终点页）
            if (mpobf_add_msadsc(prcbf, freemsa, &freemsa[freepgs - 1]) == FALSE) {
                system_error("mm_merpages_onmarea proc memarea merge fail\n");  // 严重错误上报
            }
            // 更新内存区域统计信息（增加可用页数）
            mm_update_memarea(malckp, freepgs, 1);
            // 更新全局内存管理统计
            mm_update_memmgrob(freepgs, 1);
            return TRUE;
        } else if (1 == pocs) {  // 内存块已被合并的情况
            return TRUE;
        } else if (0 == pocs) {  // 合并失败的情况
            return FALSE;
        }
        return FALSE;  // 非预期返回值兜底
    }

    //========== 处理非进程类型内存区域 ==========//
    bafhlst_t *retrelbf = NULL, *retmerbf = NULL;  // 释放/合并链表指针
    bool_t rets = FALSE;  // 操作结果

    // 获取对应的内存管理链表（根据页面数确定链表层级）
    rets = onfpgs_retn_bafhlst(malckp, freepgs, &retrelbf, &retmerbf);
    if (FALSE == rets || NULL == retrelbf || NULL == retmerbf) {
        return FALSE;  // 获取链表失败直接返回
    }

    // 尝试合并页面到链表
    sint_t mopms = mm_merpages_opmsadsc(retrelbf, freemsa, freepgs);
    if (2 == mopms) {  // 需要执行合并操作的情况
        // 在管理链表上执行合并操作
        rets = mm_merpages_onbafhlst(freemsa, freepgs, retrelbf, retmerbf);
        if (TRUE == rets) {
            // 维护内存区域和全局管理信息
            mm_update_memarea(malckp, freepgs, 1);
            mm_update_memmgrob(freepgs, 1);
        }
        return rets;
    } else if (1 == mopms) {  // 内存块已被合并的情况
        return TRUE;
    }

    // 处理失败情况（0或其他不可预期返回值）
    return FALSE;
}




// mm_merpages_core 函数：合并内存区域中的页面
bool_t mm_merpages_core(memarea_t *marea, msadsc_t *freemsa, uint_t freepgs)
{
    // 检查输入参数是否有效
    if (NULL == marea || NULL == freemsa || 1 > freepgs)
    {
        return FALSE;
    }
    // 检查 freemsa 是否有效
    if (scan_freemsa_isok(freemsa, freepgs) == FALSE)
    {
        return FALSE;
    }
    bool_t rets = FALSE;
    cpuflg_t cpuflg;

    // 获取内存区域的锁
    knl_spinlock_cli(&marea->ma_lock, &cpuflg);

    // 调用 mm_merpages_onmarea 函数合并页面
    rets = mm_merpages_onmarea(marea, freemsa, freepgs);

    // 释放内存区域的锁
    knl_spinunlock_sti(&marea->ma_lock, &cpuflg);
    return rets;
}

memarea_t *onfrmsa_retn_marea(memmgrob_t *mmobjp, msadsc_t *freemsa, uint_t freepgs)
{
    // 检查 freemsa 的类型是否为 MF_OLKTY_ODER，且 md_odlink 是否有效
    if (MF_OLKTY_ODER != freemsa->md_indxflgs.mf_olkty || NULL == freemsa->md_odlink)
    {
        return NULL;
    }

    // 获取 freemsa 的结束描述符
    msadsc_t *fmend = (msadsc_t *)freemsa->md_odlink;

    // 检查 freemsa 和 fmend 之间的距离是否等于 freepgs
    if (((uint_t)(fmend - freemsa) + 1) != freepgs)
    {
        return NULL;
    }

    // 检查 freemsa 和 fmend 的类型是否相同
    if (freemsa->md_indxflgs.mf_marty != fmend->md_indxflgs.mf_marty)
    {
        return NULL;
    }

    // 遍历内存管理对象中的内存区域，找到匹配的内存区域
    for (uint_t mi = 0; mi < mmobjp->mo_mareanr; mi++)
    {
        if ((uint_t)(freemsa->md_indxflgs.mf_marty) == mmobjp->mo_mareastat[mi].ma_type)
        {
            return &mmobjp->mo_mareastat[mi];
        }
    }

    // 如果没有找到匹配的内存区域，返回 NULL
    return NULL;
}


// mm_merpages_fmwk 函数：框架函数，用于处理内存管理对象中的页面合并
bool_t mm_merpages_fmwk(memmgrob_t *mmobjp, msadsc_t *freemsa, uint_t freepgs)
{
    // 获取内存区域
    memarea_t *marea = onfrmsa_retn_marea(mmobjp, freemsa, freepgs);
    if (NULL == marea)
    {
        return FALSE;
    }

    // 调用核心函数进行页面合并
    bool_t rets = mm_merpages_core(marea, freemsa, freepgs);
    if (FALSE == rets)
    {
        return FALSE;
    }
    return rets;
}

// mm_merge_pages 函数：合并内存管理对象中的页面的接口函数
bool_t mm_merge_pages(memmgrob_t *mmobjp, msadsc_t *freemsa, uint_t freepgs)
{
    // 检查输入参数是否有效
    if (NULL == mmobjp || NULL == freemsa || 1 > freepgs)
    {
        return FALSE;
    }

    // 调用框架函数进行页面合并
    bool_t rets = mm_merpages_fmwk(mmobjp, freemsa, freepgs);
    if (FALSE == rets)
    {
        return FALSE;
    }
    return rets;
}