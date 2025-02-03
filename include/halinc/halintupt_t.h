/**********************************************************
        hal层中断处理头文件halintupt_t.h
***********************************************************
                
**********************************************************/
#ifndef _HALINTUPT_T_H
#define _HALINTUPT_T_H



#ifdef CFG_X86_PLATFORM

typedef struct s_ILNEDSC
{
    u32_t ild_modflg;
    u32_t ild_devid;
    u32_t ild_physid;
    u32_t ild_clxsubinr;
}ilnedsc_t;

typedef struct s_INTFLTDSC
{
    spinlock_t  i_lock;         // 用于保护中断相关的数据结构的自旋锁，防止并发访问时发生竞态条件
    u32_t       i_flg;          // 中断标志，控制中断处理行为
    u32_t       i_stus;         // 中断状态，用于表示中断的当前处理状态
    uint_t      i_prity;        // 中断优先级，决定该中断的处理优先级，值越低优先级越高
    uint_t      i_irqnr;        // 中断号，用于标识中断来源，通常是硬件中断或软件中断的编号
    uint_t      i_deep;         // 中断嵌套深度，表示当前中断的嵌套层级，防止中断过度嵌套
    u64_t       i_indx;         // 中断计数器，用于记录中断发生的次数
    list_h_t    i_serlist;      // 中断服务例程（ISR）链表，存储与当前中断相关的所有中断服务函数
    uint_t      i_sernr;        // 当前中断服务函数的数量，表示链表中服务例程的数量
    list_h_t    i_serthrdlst;   // 中断服务线程链表，存储与中断相关的线程
    uint_t      i_serthrdnr;    // 中断服务线程的数量，表示当前与中断相关的线程数
    void*       i_onethread;    // 仅在有一个中断线程时使用，直接存储该线程的指针
    void*       i_rbtreeroot;   // 中断线程数量过多时，按优先级组织为红黑树，该字段指向红黑树的根节点
    list_h_t    i_serfisrlst;   // 中断回调函数链表，用于存储由回调机制注册的中断处理函数
    uint_t      i_serfisrnr;    // 中断回调函数的数量，表示回调函数链表中的回调函数数量
    void*       i_msgmpool;     // 中断消息池，可能用于存储与中断相关的消息（例如消息队列）
    void*       i_privp;        // 中断相关的私有数据，用于存储与该中断处理相关的额外信息
    void*       i_extp;         // 中断相关的扩展数据，用于进一步扩展中断处理信息
} intfltdsc_t;


typedef struct s_INTSERDSC
{
    list_h_t    s_list;//在中断异常描述符中的链表
    list_h_t    s_indevlst; //在设备描述描述符中的链表
    u32_t       s_flg;//控制中断处理行为
    intfltdsc_t* s_intfltp;//指向中断异常描述符 
    void*       s_device;//指向设备描述符
    uint_t      s_indx;//中断计数器
    intflthandle_t s_handle;//中断处理的回调函数指针
}intserdsc_t;

typedef struct s_KITHREAD
{
    spinlock_t  kit_lock;
    list_h_t    kit_list; 
    u32_t       kit_flg;
    u32_t       kit_stus;
    uint_t      kit_prity;
    u64_t       kit_scdidx;
    uint_t      kit_runmode;
    uint_t      kit_cpuid;
    u16_t       kit_cs;
    u16_t       kit_ss;
    uint_t      kit_nxteip;
    uint_t      kit_nxtesp;
    void*       kit_stk;
    size_t      kit_stksz;
    void*       kit_runadr;
    void*       kit_binmodadr;
    void*       kit_mmdsc;
    void*       kit_privp;
    void*       kit_extp;
}kithread_t;

#endif


#endif // HALINTUPT_T_H
