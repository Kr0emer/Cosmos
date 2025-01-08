#include "cmctl.h"

void init_graph(machbstart_t* mbsp)
{
    graph_t_init(&mbsp->mb_ghparm);//初始化图形数据结构
    init_bgadevice(mbsp);//设置BGA相关参数
    if(mbsp->mb_ghparm.gh_mode!=BGAMODE)
    {
        get_vbemode(mbsp);
        get_vbemodeinfo(mbsp);
        set_vbemodeinfo();
    }

    init_kinitfvram(mbsp);// 初始化固定显存区域（Fixed VRAM）
    logo(mbsp);
    return;
}


// write_pixcolor 函数：将一个像素颜色写入帧缓冲区
void write_pixcolor(machbstart_t *mbsp, u32_t x, u32_t y, pixl_t pix)
{
    // 定义一个指向 24 位颜色像素数据的指针
    u8_t *p24bas;
    
    // 如果每像素为 24 位颜色（即每个像素用 3 字节存储 RGB）
    if (mbsp->mb_ghparm.gh_onepixbits == 24)
    {
        // 计算当前像素在图像中的偏移地址
        // (x + y * 图像宽度) 是当前像素在图像中的索引位置
        // 每个像素使用 3 字节，因此乘以 3
        u32_t p24adr = (x + (y * mbsp->mb_ghparm.gh_x)) * 3;
        
        // 获取该像素的内存地址，指向帧缓冲区中的对应位置
        p24bas = (u8_t *)(p24adr + mbsp->mb_ghparm.gh_framphyadr);
        
        // 将像素颜色写入 24 位像素的 RGB 三个分量
        p24bas[0] = (u8_t)(pix);          // 写入 B（蓝色）分量
        p24bas[1] = (u8_t)(pix >> 8);     // 写入 G（绿色）分量
        p24bas[2] = (u8_t)(pix >> 16);    // 写入 R（红色）分量
        
        // 完成写入，返回
        return;
    }
    
    // 如果每像素为 32 位颜色（即每个像素用 4 字节存储，通常采用 ARGB 格式）
    u32_t *phybas = (u32_t *)mbsp->mb_ghparm.gh_framphyadr;
    
    // 将像素颜色写入帧缓冲区的对应位置
    // (x + y * 图像宽度) 计算得到像素在图像中的索引，直接将像素值写入
    phybas[x + (y * mbsp->mb_ghparm.gh_x)] = pix;
    
    // 完成写入，返回
    return;
}


// bmp_print 函数：将 BMP 图像文件内容打印到屏幕上
void bmp_print(void *bmfile, machbstart_t *mbsp)
{
    // 如果输入的 bmp 文件指针为空，直接返回
    if (NULL == bmfile)
    {
        return;
    }

    // 定义一个像素值变量
    pixl_t pix = 0;
    // 定义一个指向 BMP 图像数据的指针
    bmdbgr_t *bpixp = NULL;
    // 定义一个指向 BMP 文件头的指针，将 bmfile 强制转换为 bmfhead_t 类型
    bmfhead_t *bmphdp = (bmfhead_t *)bmfile;
    // 定义一个指向 BMP 信息头的指针，指向文件头后的第一个字节
    bitminfo_t *binfp = (bitminfo_t *)(bmphdp + 1);
    // 计算图像数据的起始地址
    u32_t img = (u32_t)bmfile + bmphdp->bf_off;
    // 将图像数据指针指向图像数据的起始位置
    bpixp = (bmdbgr_t *)img;
    
    // 定义行和列的计数变量
    int l = 0;
    int k = 0;
    
    // 计算每行的字节数，考虑行宽的对齐
    int ilinebc = (((binfp->bi_w * 24) + 31) >> 5) << 2;
    
    // 遍历 BMP 图像的 Y 轴，从 639 行开始，到 129 行为止
    for (int y = 639; y >= 129; y--, l++)
    {
        // 每行从 x = 322 到 x = 662 之间进行像素处理
        k = 0;
        for (int x = 322; x < 662; x++)
        {
            // 获取当前像素的 BGRA 值，使用自定义的 BGRA 宏将 RGB 值组合成一个颜色值
            pix = BGRA(bpixp[k].bmd_r, bpixp[k].bmd_g, bpixp[k].bmd_b);
            // 将当前像素的颜色值写入显示屏（调用 write_pixcolor 函数）
            write_pixcolor(mbsp, x, y, pix);
            k++;
        }
        // 每处理完一行后，移动指针到下一行的像素数据
        bpixp = (bmdbgr_t *)(((int)bpixp) + ilinebc);
    }
    // 函数执行完毕，返回
    return;
}


void logo(machbstart_t *mbsp)
{
    u32_t retadr = 0, sz = 0;
    get_file_rpadrandsz("logo.bmp", mbsp, &retadr, &sz);//获取映像文件中logo.bmp的地址和大小
    if (0 == retadr)
    {
        kerror("logo getfilerpadrsz err");
    }
    bmp_print((void *)retadr, mbsp);//将图片写入显存

    return;
}

void graph_t_init(graph_t *initp)
{
    memset(initp, 0, sizeof(graph_t));
    return;
}

// 初始化固定显存区域（Fixed VRAM）
void init_kinitfvram(machbstart_t *mbsp)
{
    // 将固定显存区域的物理地址设置为 KINITFRVM_PHYADR（固定显存起始地址）
    mbsp->mb_fvrmphyadr = KINITFRVM_PHYADR;

    // 将固定显存区域的大小设置为 KINITFRVM_SZ（固定显存的大小）
    mbsp->mb_fvrmsz = KINITFRVM_SZ;

    // 将固定显存区域初始化为全 0
    // 使用 memset 函数，将从地址 KINITFRVM_PHYADR 开始的 KINITFRVM_SZ 字节清零
    memset((void *)KINITFRVM_PHYADR, 0, KINITFRVM_SZ);

    // 函数结束，返回
    return;
}


u32_t vfartolineadr(u32_t vfar)
{
    u32_t tmps = 0, sec = 0, off = 0;
    off = vfar & 0xffff;
    tmps = (vfar >> 16) & 0xffff;
    sec = tmps << 4;
    return (sec + off);
}

void get_vbemode(machbstart_t *mbsp)
{
    realadr_call_entry(RLINTNR(2), 0, 0);
    vbeinfo_t *vbeinfoptr = (vbeinfo_t *)VBEINFO_ADR;
    u16_t *mnm;

    if (vbeinfoptr->vbesignature[0] != 'V' ||
        vbeinfoptr->vbesignature[1] != 'E' ||
        vbeinfoptr->vbesignature[2] != 'S' ||
        vbeinfoptr->vbesignature[3] != 'A')
    {
        kerror("vbe is not VESA");
    }
    kprint("vbe vbever:%x\n", vbeinfoptr->vbeversion);
    if (vbeinfoptr->vbeversion < 0x0200)
    {
        kerror("vbe version not vbe3");
    }

    if (vbeinfoptr->videomodeptr > 0xffff)
    {
        mnm = (u16_t *)vfartolineadr(vbeinfoptr->videomodeptr); //
    }
    else
    {
        mnm = (u16_t *)(vbeinfoptr->videomodeptr);
    }

    int bm = 0;
    for (int i = 0; mnm[i] != 0xffff; i++)
    {
        if (mnm[i] == 0x118)
        {
            bm = 1;
        }
        if (i > 0x1000)
        {
            break;
        }
        //kprint("vbemode:%x  ",mnm[i]);
    }

    if (bm == 0)
    {
        kerror("getvbemode not 118");
    }
    mbsp->mb_ghparm.gh_mode = VBEMODE;
    mbsp->mb_ghparm.gh_vbemodenr = 0x118;
    mbsp->mb_ghparm.gh_vifphyadr = VBEINFO_ADR;
    m2mcopy(vbeinfoptr, &mbsp->mb_ghparm.gh_vbeinfo, sizeof(vbeinfo_t));
    return;
}

void bga_write_reg(u16_t index, u16_t data)
{
    out_u16(VBE_DISPI_IOPORT_INDEX, index);//向索引寄存器索引
    out_u16(VBE_DISPI_IOPORT_DATA, data);//向数据寄存器写入数据
    return;
}

u16_t bga_read_reg(u16_t index)
{
    out_u16(VBE_DISPI_IOPORT_INDEX, index);//向索引寄存器索引
    return in_u16(VBE_DISPI_IOPORT_DATA);//向数据寄存器读入数据
}

u32_t get_bgadevice()//获取设备ID
{
    // 读取 BGA 设备的 ID
    u16_t bgaid = bga_read_reg(VBE_DISPI_INDEX_ID);
    
    // 判断读取的 BGA 设备 ID 是否在有效的范围内（BGA_DEV_ID0 到 BGA_DEV_ID5）
    if (BGA_DEV_ID0 <= bgaid && BGA_DEV_ID5 >= bgaid)
    {
        // 如果设备 ID 在有效范围内，则将其写回到设备的 ID 寄存器
        bga_write_reg(VBE_DISPI_INDEX_ID, bgaid);
        
        // 再次读取设备 ID，确认其值是否与之前一致
        if (bga_read_reg(VBE_DISPI_INDEX_ID) != bgaid)
        {
            // 如果 ID 不一致，返回 0，表示验证失败
            return 0;
        }
        
        // 如果 ID 一致，返回设备 ID（转换为 32 位）
        return (u32_t)bgaid;
    }
    
    // 如果设备 ID 不在有效范围内，返回 0，表示无效设备
    return 0;
}

u32_t chk_bgamaxver()//获取版本
{
    bga_write_reg(VBE_DISPI_INDEX_ID, BGA_DEV_ID5);
    if (bga_read_reg(VBE_DISPI_INDEX_ID) == BGA_DEV_ID5)
    {
        return (u32_t)BGA_DEV_ID5;
    }
    bga_write_reg(VBE_DISPI_INDEX_ID, BGA_DEV_ID4);
    if (bga_read_reg(VBE_DISPI_INDEX_ID) == BGA_DEV_ID4)
    {
        return (u32_t)BGA_DEV_ID4;
    }
    bga_write_reg(VBE_DISPI_INDEX_ID, BGA_DEV_ID3);
    if (bga_read_reg(VBE_DISPI_INDEX_ID) == BGA_DEV_ID3)
    {
        return (u32_t)BGA_DEV_ID3;
    }
    bga_write_reg(VBE_DISPI_INDEX_ID, BGA_DEV_ID2);
    if (bga_read_reg(VBE_DISPI_INDEX_ID) == BGA_DEV_ID2)
    {
        return (u32_t)BGA_DEV_ID2;
    }
    bga_write_reg(VBE_DISPI_INDEX_ID, BGA_DEV_ID1);
    if (bga_read_reg(VBE_DISPI_INDEX_ID) == BGA_DEV_ID1)
    {
        return (u32_t)BGA_DEV_ID1;
    }
    bga_write_reg(VBE_DISPI_INDEX_ID, BGA_DEV_ID0);
    if (bga_read_reg(VBE_DISPI_INDEX_ID) == BGA_DEV_ID0)
    {
        return (u32_t)BGA_DEV_ID0;
    }
    return 0;
}

void init_bgadevice(machbstart_t *mbsp)
{
    u32_t retdevid = get_bgadevice();//获取设备ID
    if (0 == retdevid)
    {
        return;
    }
    retdevid = chk_bgamaxver();//获取版本
    if (0 == retdevid)
    {
        return;
    }
    // 禁用 BGA 显示功能
    bga_write_reg(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    
    // 设置水平分辨率为 1024 像素
    bga_write_reg(VBE_DISPI_INDEX_XRES, 1024);
    
    // 设置垂直分辨率为 768 像素
    bga_write_reg(VBE_DISPI_INDEX_YRES, 768);
    
    // 设置每像素位深（BPP，Bits Per Pixel）为 32 位颜色模式（0x20 = 32）
    bga_write_reg(VBE_DISPI_INDEX_BPP, 0x20);
    
    // 启用 BGA 显示功能，并启用线性帧缓冲区（LFB, Linear Frame Buffer）
    bga_write_reg(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);

    // 设置图形模式为 BGAMODE（BGA 图形模式标识）
    mbsp->mb_ghparm.gh_mode = BGAMODE;
    
    // 设置 VBE 模式号为 retdevid（之前检测到的设备版本号）
    mbsp->mb_ghparm.gh_vbemodenr = retdevid;
    
    // 设置水平分辨率为 1024 像素
    mbsp->mb_ghparm.gh_x = 1024;
    
    // 设置垂直分辨率为 768 像素
    mbsp->mb_ghparm.gh_y = 768;
    
    // 设置帧缓冲区的物理地址为 0xe0000000（显存的起始地址）
    mbsp->mb_ghparm.gh_framphyadr = 0xe0000000;
    
    // 设置每像素的位深（颜色位数）为 32 位（0x20 = 32）
    mbsp->mb_ghparm.gh_onepixbits = 0x20;
    
    // 设置显存分页的总页数为 4
    mbsp->mb_ghparm.gh_bank = 4;
    
    // 设置当前显存分页为第 0 页
    mbsp->mb_ghparm.gh_curdipbnk = 0;
    
    // 设置下一个显存分页为第 0 页
    mbsp->mb_ghparm.gh_nextbnk = 0;
    
    // 设置每个显存分页的大小（以字节为单位）
    // 计算方式为：分辨率宽度（gh_x） * 分辨率宽度（gh_x） * 每像素字节数（4 字节）
    mbsp->mb_ghparm.gh_banksz = (mbsp->mb_ghparm.gh_x * mbsp->mb_ghparm.gh_x * 4);

    //test_bga();
    return;
}

void test_bga()
{
    int *p = (int *)(0xe0000000);
    int *p2 = (int *)(0xe0000000 + (1024 * 768 * 4));
    int *p3 = (int *)(0xe0000000 + (1024 * 768 * 4) * 2);

    for (int i = 0; i < (1024 * 768); i++)
    {
        p2[i] = 0x00ff00ff;
    }

    for (int i = 0; i < (1024 * 768); i++)
    {
        p[i] = 0x0000ff00;
    }
    for (int i = 0; i < (1024 * 768); i++)
    {
        p3[i] = 0x00ff0000;
    }
    for (;;)
    {
        bga_write_reg(VBE_DISPI_INDEX_X_OFFSET, 0);
        bga_write_reg(VBE_DISPI_INDEX_Y_OFFSET, 0);
        bga_write_reg(VBE_DISPI_INDEX_VIRT_WIDTH, 1024);
        bga_write_reg(VBE_DISPI_INDEX_VIRT_HEIGHT, 768);
        die(0x400);
        bga_write_reg(VBE_DISPI_INDEX_X_OFFSET, 0);
        bga_write_reg(VBE_DISPI_INDEX_Y_OFFSET, 768);
        bga_write_reg(VBE_DISPI_INDEX_VIRT_WIDTH, 1024);
        bga_write_reg(VBE_DISPI_INDEX_VIRT_HEIGHT, 768 * 2);
        die(0x400);
        bga_write_reg(VBE_DISPI_INDEX_X_OFFSET, 0);
        bga_write_reg(VBE_DISPI_INDEX_Y_OFFSET, 768 * 2);
        bga_write_reg(VBE_DISPI_INDEX_VIRT_WIDTH, 1024);
        bga_write_reg(VBE_DISPI_INDEX_VIRT_HEIGHT, 768 * 3);
        die(0x400);
    }

    for (;;)
        ;
    return;
}

void get_vbemodeinfo(machbstart_t *mbsp)
{
    realadr_call_entry(RLINTNR(3), 0, 0);
    vbeominfo_t *vomif = (vbeominfo_t *)VBEMINFO_ADR;
    u32_t x = vomif->XResolution, y = vomif->YResolution;
    u32_t *phybass = (u32_t *)(vomif->PhysBasePtr);
    if (vomif->BitsPerPixel < 24)
    {
        kerror("vomif->BitsPerPixel!=32");
    }
    if (x != 1024 || y != 768)
    {
        kerror("xy not");
    }
    if ((u32_t)phybass < 0x100000)
    {
        kerror("phybass not");
    }
    mbsp->mb_ghparm.gh_x = vomif->XResolution;
    mbsp->mb_ghparm.gh_y = vomif->YResolution;
    mbsp->mb_ghparm.gh_framphyadr = vomif->PhysBasePtr;
    mbsp->mb_ghparm.gh_onepixbits = vomif->BitsPerPixel;
    mbsp->mb_ghparm.gh_vmifphyadr = VBEMINFO_ADR;
    m2mcopy(vomif, &mbsp->mb_ghparm.gh_vminfo, sizeof(vbeominfo_t));

    return;
}

void set_vbemodeinfo()
{
    realadr_call_entry(RLINTNR(4), 0, 0);
    return;
}

u32_t utf8_to_unicode(utf8_t *utfp, int *retuib)
{
    u8_t uhd = utfp->utf_b1, ubyt = 0;
    u32_t ucode = 0, tmpuc = 0;
    if (0x80 > uhd) //0xbf&&uhd<=0xbf
    {
        ucode = utfp->utf_b1 & 0x7f;
        *retuib = 1;
        return ucode;
    }
    if (0xc0 <= uhd && uhd <= 0xdf) //0xdf
    {
        ubyt = utfp->utf_b1 & 0x1f;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b2 & 0x3f;
        ucode = (tmpuc << 6) | ubyt;
        *retuib = 2;
        return ucode;
    }
    if (0xe0 <= uhd && uhd <= 0xef) //0xef
    {
        ubyt = utfp->utf_b1 & 0x0f;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b2 & 0x3f;
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b3 & 0x3f;
        ucode = (tmpuc << 6) | ubyt;
        *retuib = 3;
        return ucode;
    }
    if (0xf0 <= uhd && uhd <= 0xf7) //0xf7
    {
        ubyt = utfp->utf_b1 & 0x7;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b2 & 0x3f;
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b3 & 0x3f;
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b4 & 0x3f;
        ucode = (tmpuc << 6) | ubyt;
        *retuib = 4;
        return ucode;
    }
    if (0xf8 <= uhd && uhd <= 0xfb) //0xfb
    {
        ubyt = utfp->utf_b1 & 0x3;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b2 & 0x3f;
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b3 & 0x3f;
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b4 & 0x3f;
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b5 & 0x3f;
        ucode = (tmpuc << 6) | ubyt;
        *retuib = 5;
        return ucode;
    }
    if (0xfc <= uhd && uhd <= 0xfd) //0xfd
    {
        ubyt = utfp->utf_b1 & 0x1;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b2 & 0x3f;
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b3 & 0x3f;
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b4 & 0x3f;
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b5 & 0x3f;
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b6 & 0x3f;
        ucode = (tmpuc << 6) | ubyt;
        *retuib = 6;
        return ucode;
    }
    *retuib = 0;
    return 0;
}
