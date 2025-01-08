#include "cmctl.h"

int strcmpl(const char *a, const char *b)
{

    while (*b && *a && (*b == *a))
    {

        b++;

        a++;
    }

    return *b - *a;
}
u64_t get_wt_imgfilesz(machbstart_t *mbsp)
{
    u64_t retsz = LDRFILEADR; // 初始化返回值，设置为加载文件的起始地址
    mlosrddsc_t *mrddadrs = MRDDSC_ADR; // 获取映像文件头描述符在内存中的地址

    // 检查文件头描述符的有效性
    if (mrddadrs->mdc_endgic != MDC_ENDGIC || // 结束标志必须匹配
        mrddadrs->mdc_rv != MDC_RVGIC ||     // 描述符的版本号必须匹配
        mrddadrs->mdc_fhdnr < 2 ||           // 文件头数量必须至少为 2
        mrddadrs->mdc_filnr < 2)             // 文件数量必须至少为 2
    {
        return 0; // 如果描述符无效，返回 0
    }

    // 检查文件块结束地址是否有效
    if (mrddadrs->mdc_filbk_e < 0x4000)
    {
        return 0; // 如果文件块结束地址无效，返回 0
    }

    // 计算文件实际大小
    retsz += mrddadrs->mdc_filbk_e; // 将文件块结束地址加到 retsz
    retsz -= LDRFILEADR;            // 减去加载地址，得到实际文件大小

    // 更新 machbstart_t 结构体中的相关字段
    mbsp->mb_imgpadr = LDRFILEADR;  // 设置映像文件的加载地址
    mbsp->mb_imgsz = retsz;         // 设置映像文件的大小

    return retsz; // 返回文件大小
}

                                        
int move_krlimg(machbstart_t *mbsp, u64_t cpyadr, u64_t cpysz)
{
    // 检查传入的目标地址和大小是否合法：
    // 1. (cpyadr + cpysz) 不能超过 4GB（0xffffffff），避免越界。
    // 2. cpysz（要复制的大小）必须大于 0。
    if (0xffffffff <= (cpyadr + cpysz) || 1 > cpysz)
    {
        return 0; // 如果不合法，直接返回 0，表示失败。
    }

    // 计算目标地址 toadr，将 (cpyadr + cpysz) 向上对齐到 4KB（页面大小）的倍数。
    void *toadr = (void *)((u32_t)(P4K_ALIGN(cpyadr + cpysz)));

    // 获取映像文件的大小（单位为字节），从 machbstart_t 结构的 mb_imgsz 字段读取。
    sint_t tosz = (sint_t)mbsp->mb_imgsz;

    // 检查源地址区域是否和目标地址区域重叠：
    // 如果 adrzone_is_ok 返回非 0，表示地址区域重叠。
    if (0 != adrzone_is_ok(mbsp->mb_imgpadr, mbsp->mb_imgsz, cpyadr, cpysz))
    {
        // 如果地址区域发生重叠，检查目标地址区域的内存是否足够：
        // 调用 chk_memsize 结合 e820 内存映射表，验证内存是否满足要求。
        // 参数：
        // - e820map_t：内存映射表起始地址。
        // - e820nr：e820 表的条目数。
        // - toadr：目标地址。
        // - tosz：需要的内存大小。
        if (NULL == chk_memsize((e820map_t *)((u32_t)(mbsp->mb_e820padr)), (u32_t)mbsp->mb_e820nr, (u64_t)((u32_t)toadr), (u64_t)tosz))
        {
            return -1; // 如果内存不足，返回 -1，表示移动失败。
        }

        // 如果内存检查通过，执行内存复制：
        // 将映像从源地址 (mb_imgpadr) 复制到目标地址 (toadr)。
        m2mcopy((void *)((u32_t)mbsp->mb_imgpadr), toadr, tosz);

        // 更新 machbstart_t 结构中的映像起始地址 (mb_imgpadr)，设置为目标地址。
        mbsp->mb_imgpadr = (u64_t)((u32_t)toadr);

        return 1; // 返回 1，表示移动成功。
    }

    // 如果地址区域不重叠，则无需移动，直接返回 2。
    return 2;
}

void init_defutfont(machbstart_t *mbsp)
{
    u32_t dfadr = (u32_t)mbsp->mb_nextwtpadr;
    u64_t sz = r_file_to_padr(mbsp, dfadr, "font.fnt");//在映像中查找相应的文件，并复制到对应的地址，并返回文件的大小，这里是查找Cosmos.bin文件
    if (0 == sz)
    {
        kerror("r_file_to_padr err");
    }
    mbsp->mb_bfontpadr = mbsp->mb_nextwtpadr;//填入字库文件起始地址
    mbsp->mb_bfontsz = sz;//填入字库文件大小
    mbsp->mb_nextwtpadr = P4K_ALIGN(mbsp->mb_nextwtpadr + sz);//始终要保持指向下一段空闲内存的首地址
    mbsp->mb_kalldendpadr = mbsp->mb_bfontpadr + mbsp->mb_bfontsz;//计算内核结束地址并保存
    return;
}

void init_krlfile(machbstart_t *mbsp)
{
    u64_t sz = r_file_to_padr(mbsp, IMGKRNL_PHYADR, "Cosmos.bin");//在映像中查找相应的文件，并复制到对应的地址，并返回文件的大小，这里是查找Cosmos.bin文件
    if (0 == sz)
    {
        kerror("r_file_to_padr err");
    }
    mbsp->mb_krlimgpadr = IMGKRNL_PHYADR;//填入内核映像起始地址
    mbsp->mb_krlsz = sz;//填入内核核心大小
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
    fhdsc_t *fhdsc = get_fileinfo(fname, mbsp);//寻找文件头描述符
    if (fhdsc == NULL)
    {
        *retadr = 0;
        return;
    }
    padr = fhdsc->fhd_intsfsoff + mbsp->mb_imgpadr;//获取文件所在内存的位置
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
    mlosrddsc_t *mrddadrs = (mlosrddsc_t *)((u32_t)(mbsp->mb_imgpadr + MLOSDSC_OFF));//获取引导加载文件信息结构体地址
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
            goto ok_l;//修改
        }
    }
    rethn = -1;
ok_l:
    if (rethn < 0)
    {
        kerror("not find file");
    }
    return &fhdscstart[rethn];
}


//新添加
u64_t get_filesz(char_t *filenm, machbstart_t *mbsp)
{
    if (filenm == NULL || mbsp == NULL)
    {
        return 0;
    }
    fhdsc_t *fhdscstart = get_fileinfo(filenm, mbsp);
    if (fhdscstart == NULL)
    {
        return 0;
    }
    return fhdscstart->fhd_frealsz;
}


u64_t ret_imgfilesz()
{
    u64_t retsz = LDRFILEADR;
    mlosrddsc_t *mrddadrs = MRDDSC_ADR;
    if (mrddadrs->mdc_endgic != MDC_ENDGIC ||
        mrddadrs->mdc_rv != MDC_RVGIC ||
        mrddadrs->mdc_fhdnr < 2 ||
        mrddadrs->mdc_filnr < 2)
    {
        kerror("no mrddsc");
    }
    if (mrddadrs->mdc_filbk_e < 0x4000)
    {
        kerror("imgfile error");
    }
    retsz += mrddadrs->mdc_filbk_e;
    retsz -= LDRFILEADR;
    return retsz;
}
