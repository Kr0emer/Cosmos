#include "cmctl.h"

u64_t get_wt_imgfilesz(machbstart_t *mbsp)
{
    u64_t retsz = LDRFILEADR; // 初始化返回的文件大小为加载文件地址
    mlosrddsc_t *mrddadrs = MRDDSC_ADR;// 获取内存中影响文件头描述符的地址
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
    retsz -= LDRFILEADR;// 将文件块结束地址加到retsz上，然后减去LDRFILEADR，得到文件的实际大小
    mbsp->mb_imgpadr = LDRFILEADR;// 设置machbstart_t结构体中的mb_imgpadr为LDRFILEADR，即映像文件的加载地址
    mbsp->mb_imgsz = retsz;// 设置machbstart_t结构体中的mb_imgsz为计算出的文件大小
    return retsz;
}
                                        //0x8f000       0x1001
int move_krlimg(machbstart_t *mbsp, u64_t cpyadr, u64_t cpysz)
{

    if (0xffffffff <= (cpyadr + cpysz) || 1 > cpysz)// 检查源地址和大小是否合法
    {
        return 0;
    }
    void *toadr = (void *)((u32_t)(P4K_ALIGN(cpyadr + cpysz)));// 计算目标地址（向上对齐到4KB的倍数）
    sint_t tosz = (sint_t)mbsp->mb_imgsz;// 获取映像文件空间的大小
    if (0 != adrzone_is_ok(mbsp->mb_imgpadr, mbsp->mb_imgsz, cpyadr, cpysz))// 检查源地址和大小是否和已经存在的内容重叠
    {     
        if (NULL == chk_memsize((e820map_t *)((u32_t)(mbsp->mb_e820padr)), (u32_t)mbsp->mb_e820nr, (u64_t)((u32_t)toadr), (u64_t)tosz))//检查内存大小是否足够
        {
            return -1;
        }
        m2mcopy((void *)((u32_t)mbsp->mb_imgpadr), toadr, tosz);// 复制映像内容到0x90000的地址
        mbsp->mb_imgpadr = (u64_t)((u32_t)toadr);// 更新映像的起始地址
        return 1;
    }
    return 2;// 源地址和大小不在有效的地址范围内
}


void init_krlfile(machbstart_t *mbsp)
{
    u64_t sz = r_file_to_padr(mbsp, IMGKRNL_PHYADR, "Cosmos.bin");////在映像中查找相应的文件，并复制到对应的地址，并返回文件的大小，这里是查找Cosmos.bin文件
    if (0 == sz)
    {
        kerror("r_file_to_padr err");
    }
    mbsp->mb_krlimgpadr = IMGKRNL_PHYADR;//填入内核起始地址
    mbsp->mb_krlsz = sz;//填入内核大小
    mbsp->mb_nextwtpadr = P4K_ALIGN(mbsp->mb_krlimgpadr + mbsp->mb_krlsz);//始终要保持指向下一段空闲内存的首地址
    mbsp->mb_kalldendpadr = mbsp->mb_krlimgpadr + mbsp->mb_krlsz;//计算内核结束地址并保存
    return;
}

u64_t r_file_to_padr(machbstart_t *mbsp, u32_t f2adr, char_t *fnm)
{
    if (NULL == f2adr || NULL == fnm || NULL == mbsp)//检查参数
    {
        return 0;
    }
    u32_t fpadr = 0, sz = 0;
    get_file_rpadrandsz(fnm, mbsp, &fpadr, &sz);//查找文件
    if (0 == fpadr || 0 == sz)
    {
        return 0;
    }
    if (NULL == chk_memsize((e820map_t *)((u32_t)mbsp->mb_e820padr), (u32_t)(mbsp->mb_e820nr), f2adr, sz))//检查内存是否足够
    {
        return 0;
    }
    if (0 != chkadr_is_ok(mbsp, f2adr, sz))//检查给定的地址范围是否有效
    {
        return 0;
    }
    m2mcopy((void *)fpadr, (void *)f2adr, (sint_t)sz);//复制文件到0x2000000
    return sz;
}

void get_file_rpadrandsz(char_t *fname, machbstart_t *mbsp, u32_t *retadr, u32_t *retsz)
{
    u64_t padr = 0, fsz = 0;
    if (NULL == fname || NULL == mbsp)//检查参数
    {
        *retadr = 0;
        return;
    }
    fhdsc_t *fhdsc = get_fileinfo(fname, mbsp);//寻找内核文件头描述符
    if (fhdsc == NULL)
    {
        *retadr = 0;
        return;
    }
    padr = fhdsc->fhd_intsfsoff + mbsp->mb_imgpadr;//获取内核文件头描述符地址
    if (padr > 0xffffffff)//检查地址是否合格
    {
        *retadr = 0;
        return;
    }
    fsz = (u32_t)fhdsc->fhd_frealsz;//获取文件大小
    if (fsz > 0xffffffff)
    {
        *retadr = 0;
        return;
    }
    *retadr = (u32_t)padr;
    *retsz = (u32_t)fsz;//得到文件地址和大小
    return;
}

fhdsc_t *get_fileinfo(char_t *fname, machbstart_t *mbsp)
{
    mlosrddsc_t *mrddadrs = (mlosrddsc_t *)((u32_t)(mbsp->mb_imgpadr + MLOSDSC_OFF));//获取映像文件头描述符地址
    if (mrddadrs->mdc_endgic != MDC_ENDGIC ||
        mrddadrs->mdc_rv != MDC_RVGIC ||
        mrddadrs->mdc_fhdnr < 2 ||
        mrddadrs->mdc_filnr < 2)//检查基本信息
    {
        kerror("no mrddsc");
    }

    s64_t rethn = -1;
    fhdsc_t *fhdscstart = (fhdsc_t *)((u32_t)(mrddadrs->mdc_fhdbk_s) + ((u32_t)(mbsp->mb_imgpadr)));//获取文件头描述符地址

    for (u64_t i = 0; i < mrddadrs->mdc_fhdnr; i++)//遍历文件头描述符
    {
        if (strcmpl(fname, fhdscstart[i].fhd_name) == 0)//寻找文件
        {
            rethn = (s64_t)i;
            break;
        }
    }
    if (rethn < 0)
    {
        kerror("not find file");
    }
    return &fhdscstart[rethn];
}