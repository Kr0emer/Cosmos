/**********************************************************
        物理内存区间文件memarea_t.h
***********************************************************
                
**********************************************************/
#ifndef _MEMAREA_T_H
#define _MEMAREA_T_H

#define MMSTUS_ERR (0)
#define MMSTUS_OK (1)



typedef struct s_ARCLST
{
	list_h_t al_lru1;
	list_h_t al_lru2;
	uint_t al_lru1nr;
	uint_t al_lru2nr;
}arclst_t;

typedef struct s_MMAFRETS
{
	msadsc_t* mat_fist;
	uint_t mat_sz;
	uint_t mat_phyadr;
	u16_t mat_runmode;
	u16_t mat_gen;
	u32_t mat_mask;
}__attribute__((packed)) mmafrets_t;



struct s_MEMAREA;
typedef struct s_MAFUNCOBJS
{
	mmstus_t (*mafo_init)(struct s_MEMAREA* memarea,void* valp,uint_t val);
	mmstus_t (*mafo_exit)(struct s_MEMAREA* memarea);
	mmstus_t (*mafo_aloc)(struct s_MEMAREA* memarea,mmafrets_t* mafrspack,void* valp,uint_t val);
	mmstus_t (*mafo_free)(struct s_MEMAREA* memarea,mmafrets_t* mafrspack,void* valp,uint_t val);
	mmstus_t (*mafo_recy)(struct s_MEMAREA* memarea,mmafrets_t* mafrspack,void* valp,uint_t val);

}mafuncobjs_t;

// bafhlst_t 结构体的状态标志
#define BAFH_STUS_INIT 0    // 初始化状态（未就绪）
#define BAFH_STUS_ONEM 1    // 单块模式（可能用于特殊分配）
#define BAFH_STUS_DIVP 2    // 分裂中（块正在被分割）
#define BAFH_STUS_DIVM 3    // 合并中（块正在被合并）

typedef struct s_BAFHLST
{
	spinlock_t af_lock; //保护自身结构的自旋锁
	u32_t af_stus; //状态
	uint_t af_oder; //页面数的位移量
	uint_t af_oderpnr; //oder对应的页面数比如 oder为2那就是1<<2=4
	uint_t af_fobjnr; //多少个空闲msadsc_t结构，即空闲页面
	//uint_t af_aobjnr;
	uint_t af_mobjnr; //此结构的连入的内存块总数
	uint_t af_alcindx; //此结构的分配计数
	uint_t af_freindx; //此结构的释放计数
	list_h_t af_frelst; //挂载此结构的空闲msadsc_t结构
	list_h_t af_alclst; //挂载此结构已经分配的msadsc_t结构
	list_h_t af_ovelst;//伙伴系统中用于临时存放待合并内存块的链表（待商榷）
}bafhlst_t;

// memdivmer_t 结构体的数组大小限制
#define MDIVMER_ARR_LMAX 52  // 最大层级数（如 0~51 阶，支持 2^51 的块大小）
#define MDIVMER_ARR_BMAX 11  // 可能为“块链表”最大数（未直接使用）
#define MDIVMER_ARR_OMAX 9   // 可能为“操作链表”最大数（未直接使用）

typedef struct s_MEMDIVMER
{
    spinlock_t dm_lock;           // 自旋锁，保护整个内存划分合并器的并发访问
    u32_t dm_stus;                // 全局状态（如初始化完成、错误状态）
    uint_t dm_dmmaxindx;          // 当前支持的最大阶数（如 MDIVMER_ARR_LMAX-1）
    uint_t dm_phydmindx;          // 物理内存的最高有效阶数（由实际内存大小决定）
    uint_t dm_predmindx;          // 预测下一次分配的最佳阶数（用于优化）
    uint_t dm_divnr;              // 分裂操作的总次数
    uint_t dm_mernr;              // 合并操作的总次数
    //bafhlst_t dm_mdmonelst[MDIVMER_ARR_OMAX];
	//bafhlst_t dm_mdmblklst[MDIVMER_ARR_BMAX];
	bafhlst_t dm_mdmlielst[MDIVMER_ARR_LMAX]; // 按阶组织的内存块管理链表数组（核心数据结构）
    bafhlst_t dm_onemsalst;       // 进程私有内存链表
} memdivmer_t;

// 内存区域类型定义
#define MA_TYPE_INIT 0    // 初始/未定义类型
#define MA_TYPE_HWAD 1    // 硬件相关内存区（如 DMA 区域）
#define MA_TYPE_KRNL 2    // 内核专用内存区
#define MA_TYPE_PROC 3    // 用户进程内存区
#define MA_TYPE_SHAR 4    // 共享内存区
#define MEMAREA_MAX 4     // 最大内存区域类型编号

// 硬件相关内存区（HWAD）的地址范围定义
#define MA_HWAD_LSTART 0                // 起始地址：0
#define MA_HWAD_LSZ 0x2000000           // 大小：32MB
#define MA_HWAD_LEND (MA_HWAD_LSTART + MA_HWAD_LSZ - 1) // 结束地址：0x1FFFFFF

// 内核内存区（KRNL）的地址范围定义
#define MA_KRNL_LSTART 0x2000000        // 起始地址：32MB
#define MA_KRNL_LSZ (0x400000000 - 0x2000000) // 大小：约 15.75GB（从32MB到16GB）
#define MA_KRNL_LEND (MA_KRNL_LSTART + MA_KRNL_LSZ - 1) // 结束地址：0x3FFFFFFFF

// 进程内存区（PROC）的地址范围定义
#define MA_PROC_LSTART 0x400000000      // 起始地址：16GB
#define MA_PROC_LSZ (0xffffffffffffffff - 0x400000000) // 大小：极大值（剩余全部地址空间）
#define MA_PROC_LEND (MA_PROC_LSTART + MA_PROC_LSZ)    // 结束地址：理论上限
typedef struct s_MEMAREA
{
    // -------------------- 基础管理字段 --------------------
    list_h_t ma_list;             // 内存区链表节点，用于将多个内存区链接成全局链表
    spinlock_t ma_lock;           // 保护此内存区的自旋锁，防止并发访问冲突
    uint_t ma_stus;               // 内存区当前状态（如激活、禁用、错误等）
    uint_t ma_flgs;               // 内存区标志位（如只读、可回收等）
    uint_t ma_type;               // 内存区类型（对应 MA_TYPE_* 宏，如内核区、进程区）
    sem_t ma_sem;                 // 信号量，用于同步操作（如页面分配时的阻塞等待）
    wait_l_head_t ma_waitlst;     // 等待队列头，记录因内存不足而阻塞的进程/线程

    // -------------------- 页面统计信息 --------------------
    uint_t ma_maxpages;    // 内存区总页面数（根据逻辑地址范围计算）
    uint_t ma_allocpages;  // 已分配页面数
    uint_t ma_freepages;   // 空闲页面数
    uint_t ma_resvpages;   // 预留页面数（不可分配，用于紧急情况）
    uint_t ma_horizline;   // 内存分配水位线，用于触发内存回收或警告（类似 Linux 的 watermark）

    // -------------------- 逻辑地址范围 --------------------
    adr_t ma_logicstart;   // 内存区的逻辑起始地址（如 MA_HWAD_LSTART）
    adr_t ma_logicend;     // 内存区的逻辑结束地址（如 MA_HWAD_LEND）
    uint_t ma_logicsz;     // 内存区总大小（字节数，等于 logicend - logicstart + 1）

    // -------------------- 有效地址范围 --------------------
    adr_t ma_effectstart;  // 实际可分配的有效起始地址（可能因对齐或保留区域小于逻辑范围）
    adr_t ma_effectend;    // 实际可分配的有效结束地址
    uint_t ma_effectsz;    // 有效可分配区域大小

    // -------------------- 页面管理结构 --------------------
    list_h_t ma_allmsadsclst;  // 挂载此内存区所有 msadsc_t（内存页描述符）的链表
    uint_t ma_allmsadscnr;     // 此内存区中 msadsc_t 结构的总数（总页面数）
    arclst_t ma_arcpglst;      // 可能为“归档页面链表”，记录长时间未使用的页面（推测）

    // -------------------- 策略扩展接口 --------------------
    mafuncobjs_t ma_funcobj;   // 函数对象集合，包含此内存区的分配/释放算法函数指针
    memdivmer_t ma_mdmdata;    // 内存划分与合并策略的数据结构（如伙伴系统的阶数管理）
    void* ma_privp;            // 私有数据指针，供自定义分配算法使用

    // 注释说明：
    // 1. 此结构可能占用一个或多个连续页面，首地址对齐到页面起始。
    // 2. 后续空间可扩展分配算法所需的数据结构（如伙伴系统的空闲链表数组）。
    // 3. 通过替换 `ma_funcobj` 中的函数指针，可动态切换分配算法。
} memarea_t;
#endif