#include "cmctl.h"


//初始化machbstart_t结构体，清0,并设置一个标志
void machbstart_t_init(machbstart_t* initp)
{
    memset(initp,0,sizeof(machbstart_t));
    initp->mb_migc=MBS_MIGC;//设置魔数
    return;
}
void init_bstartparm()
{
    machbstart_t* mbsp = MBSPADR;//1MB的内存地址
    machbstart_t_init(mbsp);
    init_chkcpu(mbsp);
    init_mem(mbsp);
    if (0 == get_wt_imgfilesz(mbsp))
    {
        kerror("imgfilesz 0");
    }
    return;
}

//两个内存地址区间是否有重叠
int adrzone_is_ok(u64_t sadr, u64_t slen, u64_t kadr, u64_t klen)
{
    if (kadr >= sadr && kadr <= (sadr + slen))
    {
        return -1;
    }
    if (kadr <= sadr && ((kadr + klen) >= sadr))
    {
        return -2;
    }

    return 0;
}