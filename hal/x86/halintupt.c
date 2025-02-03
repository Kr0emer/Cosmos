#include "cosmostypes.h"
#include "cosmosmctrl.h"

// 初始化中断异常描述符（intfltdsc_t）结构体
void intfltdsc_t_init(intfltdsc_t *initp, u32_t flg, u32_t sts, uint_t prity, uint_t irq)
{
    // 初始化自旋锁
    hal_spinlock_init(&initp->i_lock);
    // 设置标志位、状态、优先级和中断号
    initp->i_flg = flg;
    initp->i_stus = sts;
    initp->i_prity = prity;
    initp->i_irqnr = irq;
    // 初始化深度和索引
    initp->i_deep = 0;
    initp->i_indx = 0;
    // 初始化相关链表
    list_init(&initp->i_serlist);
    initp->i_sernr = 0;
    list_init(&initp->i_serthrdlst);
    initp->i_serthrdnr = 0;
    initp->i_onethread = NULL;
    initp->i_rbtreeroot = NULL;
    list_init(&initp->i_serfisrlst);
    initp->i_serfisrnr = 0;
    initp->i_msgmpool = NULL;
    initp->i_privp = NULL;
    initp->i_extp = NULL;
    return;
}

// 初始化所有的中断异常描述符
void init_intfltdsc()
{
    for (uint_t i = 0; i < IDTMAX; i++)
    {
        intfltdsc_t_init(&machintflt[i], 0, 0, i, i);
    }
    return;
}

// 初始化硬件中断管理
PUBLIC void init_halintupt()
{
    init_descriptor();           // 初始化描述符
    init_idt_descriptor();       // 初始化IDT描述符
    init_intfltdsc();            // 
    init_i8259();                // 初始化i8259中断控制器
    i8259_enabled_line(0);       // 启用特定的中断线
    return;
}

// 根据中断号返回对应的中断描述符
PUBLIC intfltdsc_t *hal_retn_intfltdsc(uint_t irqnr)
{
    if (irqnr > IDTMAX)
    {
        return NULL;  // 如果中断号超出范围，返回NULL
    }
    return &machintflt[irqnr];
}

// 初始化中断服务描述符（intserdsc_t）结构体
void intserdsc_t_init(intserdsc_t *initp, u32_t flg, intfltdsc_t *intfltp, void *device, intflthandle_t handle)
{
    // 初始化链表
    list_init(&initp->s_list);
    list_init(&initp->s_indevlst);
    // 设置标志位、关联的中断描述符、设备和处理函数
    initp->s_flg = flg;
    initp->s_intfltp = intfltp;
    initp->s_indx = 0;
    initp->s_device = device;
    initp->s_handle = handle;
    return;
}

// 将中断服务描述符添加到中断描述符的服务链表中
bool_t hal_add_ihandle(intfltdsc_t *intdscp, intserdsc_t *serdscp)
{
    if (intdscp == NULL || serdscp == NULL)
    {
        return FALSE;  // 如果输入为空，返回FALSE
    }
    cpuflg_t cpuflg;
    // 保存自旋锁并禁用中断
    hal_spinlock_saveflg_cli(&intdscp->i_lock, &cpuflg);
    // 将服务描述符添加到中断描述符的服务链表
    list_add(&serdscp->s_list, &intdscp->i_serlist);
    intdscp->i_sernr++;
    // 释放自旋锁并恢复中断
    hal_spinunlock_restflg_sti(&intdscp->i_lock, &cpuflg);
    return TRUE;  // 成功返回TRUE
}

// 启用中断线
drvstus_t hal_enable_intline(uint_t ifdnr)
{
    if (20 > ifdnr || 36 < ifdnr)
    {
        return DFCERRSTUS;  // 如果中断线不在有效范围内，返回错误状态
    }
    i8259_enabled_line((u32_t)ifdnr);  // 启用中断线
    return DFCOKSTUS;  // 返回成功状态
}

// 禁用中断线
drvstus_t hal_disable_intline(uint_t ifdnr)
{
    if (20 > ifdnr || 36 < ifdnr)
    {
        return DFCERRSTUS;  // 如果中断线不在有效范围内，返回错误状态
    }
    i8259_disable_line((u32_t)ifdnr);  // 禁用中断线
    return DFCOKSTUS;  // 返回成功状态
}

// 默认中断处理函数
drvstus_t hal_intflt_default(uint_t ift_nr, void *sframe)
{
    if (ift_nr == 0xffffffff || sframe == NULL)
    {
        return DFCERRSTUS;  // 如果中断号无效或帧指针为空，返回错误状态
    }
    return DFCOKSTUS;  // 返回成功状态
}

// 执行中断处理程序
void hal_run_intflthandle(uint_t ifdnr, void *sframe)
{
    intserdsc_t *isdscp;
    list_h_t *lst;
    // 获取中断描述符
    intfltdsc_t *ifdscp = hal_retn_intfltdsc(ifdnr);
    if (ifdscp == NULL)
    {
        hal_sysdie("hal_run_intfdsc err");  // 如果中断描述符为空，终止程序
        return;
    }

    // 遍历服务链表并调用服务处理函数
    list_for_each(lst, &ifdscp->i_serlist)
    {
        isdscp = list_entry(lst, intserdsc_t, s_list);
        isdscp->s_handle(ifdnr, isdscp->s_device, sframe);
    }

    return;
}

// 执行硬件中断处理
void hal_do_hwint(uint_t intnumb, void *krnlsframp)//krnlsframp为之前传入的寄存器组
{
    intfltdsc_t *ifdscp = NULL;
    cpuflg_t cpuflg;
    if (intnumb > IDTMAX || krnlsframp == NULL)
    {
        hal_sysdie("hal_do_hwint fail\n");  // 如果中断号无效或帧指针为空，终止程序
        return;
    }
    // 获取中断异常描述符
    ifdscp = hal_retn_intfltdsc(intnumb);
    if (ifdscp == NULL)
    {
        hal_sysdie("hal_do_hwint ifdscp NULL\n");  // 如果中断异常描述符为空，终止程序
        return;
    }
    // 获取自旋锁并禁用中断
    hal_spinlock_saveflg_cli(&ifdscp->i_lock, &cpuflg);
    ifdscp->i_indx++;  // 增加中断索引
    ifdscp->i_deep++;  // 增加深度
    // 执行中断处理函数
    hal_run_intflthandle(intnumb, krnlsframp);
    ifdscp->i_deep--;  // 恢复深度
    // 释放自旋锁并恢复中断
    hal_spinunlock_restflg_sti(&ifdscp->i_lock, &cpuflg);
    return;
}

/*TO DO*/
/*后期可以在这里修改为正常的异常处理*/
// 错误处理函数，用于处理错误的故障
void hal_fault_allocator(uint_t faultnumb, void *krnlsframp)
{
    kprint("faultnumb:%x\n", faultnumb);  // 打印故障编号
    for (;;)
        ;  // 挂起程序
    return;
}

// 系统调用分配器，当前返回0
sysstus_t hal_syscl_allocator(uint_t sys_nr,void* msgp)
{
    return 0;
}

// 执行硬件中断分配
void hal_hwint_allocator(uint_t intnumb, void *krnlsframp)
{
    i8259_send_eoi();  // 发送EOI信号
    hal_do_hwint(intnumb, krnlsframp);  // 执行硬件中断处理
    return;
}
