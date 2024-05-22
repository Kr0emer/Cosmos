#include "cmctl.h"

u64_t get_wt_imgfilesz(machbstart_t *mbsp)
{
    u64_t retsz = LDRFILEADR; // 初始化返回的文件大小为加载文件地址
    mlosrddsc_t *mrddadrs = MRDDSC_ADR;// 获取内存中文件描述符的地址
    if (mrddadrs->mdc_endgic != MDC_ENDGIC ||
        mrddadrs->mdc_rv != MDC_RVGIC ||
        mrddadrs->mdc_fhdnr < 2 ||
        mrddadrs->mdc_filnr < 2)//检查基本信息，如果不正确，则返回0
    {
        return 0;
    }
    if (mrddadrs->mdc_filbk_e < 0x4000)//检查文件块结束地址是否有效
    {
        return 0;
    }
    retsz += mrddadrs->mdc_filbk_e;
    retsz -= LDRFILEADR;
    mbsp->mb_imgpadr = LDRFILEADR;
    mbsp->mb_imgsz = retsz;
    return retsz;
}

int move_krlimg(machbstart_t *mbsp, u64_t cpyadr, u64_t cpysz)
{

    if (0xffffffff <= (cpyadr + cpysz) || 1 > cpysz)// 检查源地址和大小是否合法
    {
        return 0;
    }
    void *toadr = (void *)((u32_t)(P4K_ALIGN(cpyadr + cpysz)));// 计算目标地址（向上对齐到4KB的倍数）
    sint_t tosz = (sint_t)mbsp->mb_imgsz;// 获取地址空间的大小
    if (0 != adrzone_is_ok(mbsp->mb_imgpadr, mbsp->mb_imgsz, cpyadr, cpysz))// 检查源地址和大小是否在有效的地址范围内
    {
        if (NULL == chk_memsize((e820map_t *)((u32_t)(mbsp->mb_e820padr)), (u32_t)mbsp->mb_e820nr, (u64_t)((u32_t)toadr), (u64_t)tosz))//检查内存大小是否足够
        {
            return -1;
        }
        m2mcopy((void *)((u32_t)mbsp->mb_imgpadr), toadr, tosz);// 复制数据
        mbsp->mb_imgpadr = (u64_t)((u32_t)toadr);// 更新地址空间的起始地址
        return 1;
    }
    return 2;// 源地址和大小不在有效的地址范围内
}