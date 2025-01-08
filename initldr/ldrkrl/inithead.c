#include"cmctl.h"

void inithead_entry()
{ 
    init_curs();//初始化cursor_t结构体
    close_curs();//关闭光标
    clear_screen(VGADP_DFVL);//清除屏幕

    write_realintsvefile(); 
    write_ldrkrlfile(); 
    return;
}

//写initldrsve.bin文件到特定的内存中
void write_realintsvefile()
{
    fhdsc_t* fhdscstart = find_file("initldrsve.bin");
    if (fhdscstart == NULL)
    {
        error("not file initldrsve.bin");
    }

    //将文件拷贝到映像文件的特定位置
    m2mcopy((void *)(LDRFILEADR+(u32_t)(fhdscstart->fhd_intsfsoff)),(void *)REALDRV_PHYADR, (sint_t)fhdscstart->fhd_frealsz);
    return;
}

void write_ldrkrlfile()
{
    fhdsc_t* fhdscstart = find_file("initldrkrl.bin");
    if (fhdscstart == NULL)
    {
        error("not file initldrkrl.bin");
    }

    //将文件拷贝到映像文件的特定位置
    m2mcopy((void *)(LDRFILEADR+(u32_t)(fhdscstart->fhd_intsfsoff)),(void *)ILDRKRL_PHYADR, (sint_t)fhdscstart->fhd_frealsz);
    return;
}

fhdsc_t *find_file(char_t *fname)
{
    mlosrddsc_t *mrddadr = MRDDSC_ADR;
    // 检查映像文件的结束标识、版本号以及文件头描述符和文件头数量是否符合预期
    if (mrddadr->mdc_endgic != MDC_ENDGIC || mrddadr->mdc_rv !=MDC_RVGIC || mrddadr->mdc_fhdnr < 2 || mrddadr->mdc_filnr <2 )
    {
        error("no mrddsc");
    }

    // 初始化返回的文件头描述符索引号为 -1，表示未找到
    s64_t rethn = -1;

    //计算文件头描述符数组的起始地址
    fhdsc_t *fhdscstart = (fhdsc_t *)(LDRFILEADR+(u32_t)(mrddadr->mdc_fhdbk_s));

    //遍历文件头描述符
    for (u64_t i = 0; i < mrddadr->mdc_fhdnr; i++)
    {
        if (strcmpl(fname,fhdscstart[i].fhd_name) == 0)
        {
            rethn = (s64_t)i;
            goto ok_l;
        }
    }
    rethn = -1;
ok_l:
    if (rethn < 0)
    {
        error("not find file");
    }
    return &fhdscstart[rethn];
}

int strcmpl(const char *a, const char *b)
{

    while (*b && *a && (*b == *a))
    {

        b++;
        a++;
    }

    return *b - *a;
}

void error(char_t *estr)
{
    kprint("INITLDR DIE ERROR:%s\n", estr);
    for (;;)
        ;
    return;
}
