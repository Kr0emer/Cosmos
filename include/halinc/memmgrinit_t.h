/**********************************************************
        物理内存管理器初始化文件memmgrinit_t.h
***********************************************************
                 
**********************************************************/
#ifndef _MEMMGRINIT_T_H
#define _MEMMGRINIT_T_H

typedef struct s_MEMMGROB {
    // 管理对象链表锚点，用于全局管理多个内存管理实例
    list_h_t mo_list;        
    // 自旋锁，保护本结构体的多核并发访问            
    spinlock_t mo_lock;      
    // 运行时状态位图（初始化/激活/维护中等状态编码）       
    uint_t mo_stus;          
    // 功能特性标志位（控制分配策略/统计模式等）            
    uint_t mo_flgs;          
    // 资源访问同步信号量，协调多线程内存请求          
    sem_t mo_sem;            
    // 托管物理内存总量（以字节为单位）            
    u64_t mo_memsz;          
    // 系统最大可管理的物理页框数量（基于PAGE_SIZE）         
    u64_t mo_maxpages;       
    // 当前空闲页框计数器（实时反映可用物理页数量）        
    u64_t mo_freepages;      
    // 已分配页框计数器（包含内核和用户态分配）          
    u64_t mo_alocpages;      
    // 预留页框数量（用于紧急操作或特权请求）        
    u64_t mo_resvpages;      
    // 内存警戒水位线（触发回收机制的阈值页数）       
    u64_t mo_horizline;      
    // 物理内存段描述符数组（记录不同类型物理地址范围）
    phymmarge_t* mo_pmagestat;  
    // 物理内存段数量（即phymmarge数组元素个数）     
    u64_t mo_pmagenr;        
    // 页帧元数据数组（每个元素对应一个物理页的管理信息）   
    msadsc_t* mo_msadscstat;   
    // 页帧元数据总数（通常等于系统物理页总数）       
    u64_t mo_msanr;          
    // 内存分区管理数组（按用途划分：DMA/内核/用户等区域）  
    memarea_t* mo_mareastat;   
    // 内存分区数量（不同架构分区策略不同）         
    u64_t mo_mareanr;        
    // 内核专用小对象分配器控制头（SLAB/SLUB分配器元数据）
    kmsobmgrhed_t mo_kmsobmgr; 
    // 模块私有数据挂载点（扩展特定内存管理功能）        
    void* mo_privp;          
    // 预留扩展接口指针（未来兼容性设计）          
    void* mo_extp;           
} memmgrob_t;


#endif