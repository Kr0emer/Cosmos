/**********************************************************
        物理内存管理器初始化文件memmgrinit_t.h
***********************************************************
                 
**********************************************************/
#ifndef _MEMMGRINIT_T_H
#define _MEMMGRINIT_T_H

// 定义内存管理对象结构体
typedef struct s_MEMMGROB
{
    // 链表头
    list_h_t mo_list;
    // 自旋锁
    spinlock_t mo_lock;
    // 内存管理对象的状态
    uint_t mo_stus;
    // 内存管理对象的标志
    uint_t mo_flgs;
    // 信号量，用于线程间同步，可用于控制对内存资源的访问数量，避免资源竞争
    sem_t mo_sem;
    // 管理的内存总大小
    u64_t mo_memsz;
    // 最大可管理的页面数量
    u64_t mo_maxpages;
    // 空闲页面数量
    u64_t mo_freepages;
    // 已分配页面数量
    u64_t mo_alocpages;
    // 保留页面数量
    u64_t mo_resvpages;
    // 内存分配水位线
    u64_t mo_horizline;
    // //内存空间布局结构指针
    phymmarge_t* mo_pmagestat;
    // 物理内存映射状态结构体数组的数量，表明 `mo_pmagestat` 所指向的数组中元素的个数
    u64_t mo_pmagenr;
    // 内存页面结构指针
    msadsc_t* mo_msadscstat;
    // MSADSC 状态数组的数量，即 `mo_msadscstat` 所指向的数组中元素的个数
    u64_t mo_msanr;
    // 内存页面结构指针
    memarea_t* mo_mareastat;
    // 内存区域状态结构体数组的数量，表明 `mo_mareastat` 所指向的数组中元素的个数
    u64_t mo_mareanr;
    // 内核内存对象管理头，可能用于管理内核相关的内存对象，具体功能取决于其定义和程序使用方式
    kmsobmgrhed_t mo_kmsobmgr;
    // 私有指针，可由使用该内存管理对象的模块自行使用，用于存储特定于该模块的私有数据
    void* mo_privp;
    // 扩展指针，可能用于指向一些扩展功能或数据结构，为内存管理对象提供额外的扩展性
    void* mo_extp;
}memmgrob_t;

#endif