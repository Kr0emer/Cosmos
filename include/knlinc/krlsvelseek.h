/**********************************************************
        内核服务头文件krlsvelseek.h
***********************************************************
                
**********************************************************/
#ifndef _KRLSVELSEEK_H
#define _KRLSVELSEEK_H
sysstus_t krlsvetabl_lseek(uint_t swinr,stkparame_t* stkparv);
sysstus_t krlsve_lseek(hand_t fhand,uint_t lofset,uint_t flgs);
sysstus_t krlsve_core_lseek(hand_t fhand,uint_t lofset,uint_t flgs);
#endif // KRLSVELSEEK_H
