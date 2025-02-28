/**********************************************************
        物理内存分割合并文件memdivmer_t.h
***********************************************************
                
**********************************************************/
#ifndef _MEMDIVMER_T_H
#define _MEMDIVMER_T_H

#define DMF_RELDIV 0 //按需分配指定页数
#define DMF_MAXDIV 1 //最大化分配可用连续块

typedef struct s_MCHKSTUC
{
	list_h_t mc_list;
	u64_t mc_phyadr;
	u64_t mc_viradr;
	u64_t mc_sz;
	u64_t mc_chkval;
	msadsc_t* mc_msa; 
	u64_t* mc_chksadr;
	u64_t* mc_chkeadr; 
}mchkstuc_t;

#endif