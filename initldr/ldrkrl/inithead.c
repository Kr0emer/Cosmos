#include"cmctl.h"

void inithead_entry()
{ 
    write_realintsvefile(); 
    write_ldrkrlfile(); 
    return;
}

//写initldrsve.bin文件到特定的内存中
void write_realintsvefile()
{
    fhdsc_t* fhdscstart = find_file("inithead.bin");
    if (fhdscstart == NULL)
    {
        error("not file inithead.bin");
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
    m2mcopy((void *)(LDRFILEADR+(u32_t)(fhdscstart->fhd_intsfsoff)),(void *)REALDRV_PHYADR, (sint_t)fhdscstart->fhd_frealsz);
    return;
}

fhdsc_t *find_file(char_t *fname)
{
    mlosrddsc_t *mrddadr = MRDDSC_ADR;
    // 检查映像文件的结束标识、版本号以及文件头描述符和文件头数量是否符合预期
    if (mrddadr->mdc_endgic != MDC_ENDGIC || mrddadr->mdc_rv !=MDC_RVGIC || mrddadr->mdc_fhdnr || mrddadr->mdc_fhdnr < 2 || mrddadr->mdc_filnr <2 )
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
            break;
        }
    }
    if(rethn < 0)
    {
        return 0;
    }
    return &fhdscstart[rethn];
}

sint_t strcmpl(const char *str1, const char *str2)
{
    while (*str1,*str2,*str1 == *str2)
    {
        str1++;
        str2++;
    }
    return str1 - str2;
}

void error(char_t *estr)
{
    kprint("INITLDR DIE ERROR:%s\n", estr);
    for (;;)
        ;
    return;
}
