/**********************************************************
        物理内存空间数组文件msadsc_t.h
***********************************************************
                
**********************************************************/
#ifndef _MSADSC_T_H
#define _MSADSC_T_H

#define MSAD_PAGE_MAX (8)


#define MF_OLKTY_INIT (0)// 初始未链接状态
#define MF_OLKTY_ODER (1)// 内存块头节点标志
#define MF_OLKTY_BAFH (2)// 内存块尾节点标志
#define MF_OLKTY_TOBJ (3) //类型化对象链表标志

#define MF_LSTTY_LIST (0)// 内存对象已挂载至链表

#define MF_MOCTY_FREE (0)// 内存块处于空闲状态
#define MF_MOCTY_KRNL (1)// 内存块被内核占用
#define MF_MOCTY_USER (2)// 内存块被用户态应用占用

#define MF_MRV1_VAL (0) // / 保留位标志

#define MF_UINDX_INIT (0)// 分配计数器初始值
#define MF_UINDX_MAX (0xffffff)// 分配计数器最大值

#define MF_MARTY_INIT (0) // 未分配区域
#define MF_MARTY_HWD (1)  // 硬件相关内存区
#define MF_MARTY_KRL (2) // 内核核心内存区
#define MF_MARTY_PRC (3) // 用户进程私有内存区  
#define MF_MARTY_SHD (4)  // 共享内存映射区
//内存空间地址描述符标志
typedef struct s_MSADFLGS
{
    u32_t mf_olkty:2;    //块位置类型
    u32_t mf_lstty:1;    //是否挂入链表
    u32_t mf_mocty:2;    //分配类型，被谁占用了，内核还是应用或者空闲
    u32_t mf_marty:3;    //属于哪个区
    u32_t mf_uindx:24;   //分配计数
}__attribute__((packed)) msadflgs_t; 


#define  PAF_NO_ALLOC (0) // 页帧未分配（空闲状态）
#define  PAF_ALLOC (1)  // 页帧已分配（占用状态）
#define  PAF_NO_SHARED (0)// 禁止共享
#define  PAF_NO_SWAP (0) // 禁止换出到交换空间
#define  PAF_NO_CACHE (0)// 禁用CPU缓存
#define  PAF_NO_KMAP (0) // 禁止内核直接映射
#define  PAF_NO_LOCK (0)   // 无锁访问模式
#define  PAF_NO_DIRTY (0)  // 页帧内容为干净
#define  PAF_NO_BUSY (0) // 页帧未被占用
#define  PAF_RV2_VAL (0)  // 保留位2的默认值
#define  PAF_INIT_PADRS (0)  // 初始物理地址


//物理地址和标志  4KB 12bit
typedef struct s_PHYADRFLGS
{
    u64_t paf_alloc:1;     //分配位
    u64_t paf_shared:1;    //共享位
    u64_t paf_swap:1;      //交换位
    u64_t paf_cache:1;     //缓存位
    u64_t paf_kmap:1;      //映射位
    u64_t paf_lock:1;      //锁定位
    u64_t paf_dirty:1;     //脏位
    u64_t paf_busy:1;      //忙位
    u64_t paf_rv2:4;       //保留位
    u64_t paf_padrs:52;    //页物理地址位
}__attribute__((packed)) phyadrflgs_t;


/*内存空间地址页描述符*/
typedef struct s_MSADSC
{
	list_h_t md_list;//16 链表
	spinlock_t md_lock; //4 保护自身的自旋锁
	msadflgs_t md_indxflgs;//4 内存空间地址描述符标志
	phyadrflgs_t md_phyadrs;//8 物理地址和标志
	void* md_odlink;//8 连续块内msadsc的指针，确定每块的最后一个页描述符
}__attribute__((packed)) msadsc_t;//16+24 = 40 


#endif