#include "cosmostypes.h"
#include "cosmosmctrl.h"


const char *cosmos_version = "Cosmos\n内核版本:00.01\nKr0emer @ 构建于 "__DATE__
                           " "__TIME__
                           "\n";

                           
// 初始化 dftgraph_t 结构体并从 machbstart_t 中复制相关数据
PUBLIC LKINIT void init_dftgraph()
{
    // 声明指向 dftgraph_t 和 machbstart_t 结构体的指针
    dftgraph_t *kghp = &kdftgh;  // dftgraph_t 类型结构体指针
    machbstart_t *kmbsp = &kmachbsp;  // machbstart_t 类型结构体指针

    // 清空 dftgraph_t 结构体的内容
    memset(kghp, 0, sizeof(dftgraph_t));

    // 将 machbstart_t 中的参数值复制到 dftgraph_t 中
    kghp->gh_mode = kmbsp->mb_ghparm.gh_mode;  // 复制显示模式
    kghp->gh_x = kmbsp->mb_ghparm.gh_x;  // 复制图像的宽度
    kghp->gh_y = kmbsp->mb_ghparm.gh_y;  // 复制图像的高度
    
    // 将物理地址转换为虚拟地址并赋值给 gh_framphyadr 和 gh_fvrmphyadr
    kghp->gh_framphyadr = phyadr_to_viradr((adr_t)kmbsp->mb_ghparm.gh_framphyadr);
    kghp->gh_fvrmphyadr = phyadr_to_viradr((adr_t)kmbsp->mb_fvrmphyadr);

    kghp->gh_fvrmsz = kmbsp->mb_fvrmsz;  // 复制帧缓冲区大小
    kghp->gh_onepixbits = kmbsp->mb_ghparm.gh_onepixbits;  // 复制每个像素位数
    kghp->gh_onepixbyte = kmbsp->mb_ghparm.gh_onepixbits / 8;  // 计算每个像素的字节数
    kghp->gh_vbemodenr = kmbsp->mb_ghparm.gh_vbemodenr;  // 复制 VBEMODE 的编号
    kghp->gh_bank = kmbsp->mb_ghparm.gh_bank;  // 复制显示银行编号
    kghp->gh_curdipbnk = kmbsp->mb_ghparm.gh_curdipbnk;  // 当前 DIP 银行
    kghp->gh_nextbnk = kmbsp->mb_ghparm.gh_nextbnk;  // 下一个银行编号
    kghp->gh_banksz = kmbsp->mb_ghparm.gh_banksz;  // 复制银行大小

    // 处理字体相关的内容
    kghp->gh_fontadr = phyadr_to_viradr((adr_t)kmbsp->mb_bfontpadr);  // 字体的物理地址转为虚拟地址
    kghp->gh_fontsz = kmbsp->mb_bfontsz;  // 复制字体大小
    kghp->gh_fnthight = 16;  // 设置字体高度为 16
    kghp->gh_linesz = 16 + 4;  // 设置每行大小为 16 + 4

    // 设置默认字体颜色，BGRA 格式（青色，白色，红色，透明度）
    kghp->gh_deffontpx = BGRA(0xff, 0xff, 0xff);

    // 返回（函数执行完成后退出）
    return;
}





// 设置默认字体像素大小的函数
pixl_t set_deffontpx(pixl_t setpx)
{
    // 获取默认图形处理结构的指针
    dftgraph_t *kghp = &kdftgh;
    // 保存当前的默认字体像素大小
    pixl_t bkpx = kghp->gh_deffontpx;
    // 更新默认字体像素大小
    kghp->gh_deffontpx = setpx;
    // 返回之前的默认字体像素大小
    return bkpx;
}

// 设置字符显示的位置（x 和 y 坐标）的函数
void set_ncharsxy(u64_t x, u64_t y)
{
    // 获取默认图形处理结构的指针
    dftgraph_t *kghp = &kdftgh;
    // 设置下一个字符显示的 x 坐标
    kghp->gh_nxtcharsx = x;
    // 设置下一个字符显示的 y 坐标
    kghp->gh_nxtcharsy = y;
    return; // 无返回值
}

// 设置字符宽度偏移和刷新标志的函数
void set_charsdxwflush(u64_t chardxw, u64_t flush)
{
    // 更新默认图形处理结构中的字符宽度偏移
    kdftgh.gh_chardxw = chardxw;
    // 更新刷新标志
    kdftgh.gh_flush = flush;
    return; // 无返回值
}

// 显示系统版本信息的函数
void hal_dspversion()
{
    // 设置默认字体颜色为红色（BGRA 格式），并保存原来的字体像素大小
    pixl_t bkpx = set_deffontpx(BGRA(0xff, 0, 0));
    // 输出系统版本信息
    kprint(cosmos_version);
    // 输出系统处理器工作模式和物理内存大小
    kprint("系统处理器工作模式:%d位 系统物理内存大小:%dMB\n", 
           (uint_t)kmachbsp.mb_cpumode,   // CPU 工作模式（位数）
           (uint_t)(kmachbsp.mb_memsz >> 20)); // 内存大小（MB）
    // 恢复原来的字体像素大小
    set_deffontpx(bkpx);
    return; // 无返回值
}



// 填充矩形区域的函数
void fill_rect(pixl_t pix, uint_t x, uint_t y, uint_t tx, uint_t ty)
{
    // 遍历矩形区域的每一个像素位置
    for (uint_t i = y; i < y + ty; i++)
    {
        for (uint_t j = x; j < x + tx; j++)
        {
            // 在图形缓冲区中写入指定颜色
            write_pixcolor(&kdftgh, (u32_t)j, (u32_t)i, pix);
        }
    }
    return; // 无返回值
}

// 填充整个屏幕背景图像的函数
void fill_fullbitmap(char_t *bmfname)
{
    // 获取图形处理和系统启动信息结构体的指针
    dftgraph_t *kghp = &kdftgh;
    machbstart_t *kmbsp = &kmachbsp;
    u64_t fadr = 0, fsz = 0;

    // 获取文件的物理地址和大小
    get_file_rvadrandsz(bmfname, kmbsp, &fadr, &fsz);

    // 如果文件地址或大小为0，则输出错误信息
    if (0 == fadr || 0 == fsz)
    {
        system_error("not bitmap file err\n");
    }

    // 获取位图数据的起始位置
    bmdbgr_t *bmdp;
    u64_t img = fadr + sizeof(bmfhead_t) + sizeof(bitminfo_t);
    bmdp = (bmdbgr_t *)((uint_t)img);

    // 遍历位图数据，并将每个像素的颜色写入缓冲区
    pixl_t pix;
    int k = 0, l = 0;
    for (int j = 768; j >= 0; j--, l++) // 从下到上扫描
    {
        for (int i = 0; i < 1024; i++) // 从左到右扫描
        {
            pix = BGRA(bmdp[k].bmd_r, bmdp[k].bmd_g, bmdp[k].bmd_b); // 提取颜色值
            write_pixcolor(kghp, i, j, pix); // 写入像素颜色
            k++;
        }
    }

    // 刷新视频内存
    flush_videoram(kghp);
    return; // 无返回值
}

// 滚动屏幕显示内容的函数
void slou_sreen(uint_t hy)
{
    uint_t x, y, ofx, ofy;
    dftgraph_t *kghp = &kdftgh;

    // 获取当前屏幕的偏移和分辨率
    bga_get_xyoffset(kghp, &x, &y);
    bga_get_vwh(kghp, &ofx, &ofy);

    // 更新垂直偏移值
    y += hy;
    ofy += hy;

    // 设置新的分辨率和偏移
    bga_set_vwh(kghp, ofx, ofy);
    bga_set_xyoffset(kghp, x, y);
    return; // 无返回值
}

// 设置屏幕背景并显示系统版本信息的函数
void hal_background()
{
    dftgraph_t *kghp = &kdftgh;
    machbstart_t *kmbsp = &kmachbsp;
    u64_t fadr = 0, fsz = 0;

    // 获取背景图片文件的物理地址和大小
    get_file_rvadrandsz("background.bmp", kmbsp, &fadr, &fsz);

    // 如果文件地址或大小为0，则输出错误信息
    if (0 == fadr || 0 == fsz)
    {
        system_error("lmosbkgderr");
    }

    // 获取位图数据的起始位置
    bmdbgr_t *bmdp;
    u64_t img = fadr + sizeof(bmfhead_t) + sizeof(bitminfo_t);
    bmdp = (bmdbgr_t *)((uint_t)img);

    // 遍历位图数据，并将每个像素的颜色写入缓冲区
    pixl_t pix;
    int k = 0, l = 0;
    for (int j = 768; j >= 0; j--, l++) // 从下到上扫描
    {
        for (int i = 0; i < 1024; i++) // 从左到右扫描
        {
            pix = BGRA(bmdp[k].bmd_r, bmdp[k].bmd_g, bmdp[k].bmd_b); // 提取颜色值
            drxw_pixcolor(kghp, i, j, pix); // 写入像素颜色
            k++;
        }
    }

    // 显示系统版本信息
    hal_dspversion();
    return; // 无返回值
}

// 显示系统标志（logo）的函数
void hal_logo()
{
    // 获取默认图形处理结构的指针
    dftgraph_t *kghp = &kdftgh;
    // 获取系统启动参数结构的指针
    machbstart_t *kmbsp = &kmachbsp;
    // 用于存储文件地址和大小的变量
    u64_t fadr = 0, fsz = 0;

    // 获取位图文件 "logo.bmp" 的物理地址和大小
    get_file_rvadrandsz("logo.bmp", kmbsp, &fadr, &fsz);

    // 如果地址或大小为 0，表示文件不存在或错误，输出错误信息并停止执行
    if (0 == fadr || 0 == fsz)
    {
        system_error("logoerr");
    }

    // 定义位图数据指针
    bmdbgr_t *bmdp;
    // 计算位图像素数据的起始地址
    u64_t img = fadr + sizeof(bmfhead_t) + sizeof(bitminfo_t);
    bmdp = (bmdbgr_t *)((uint_t)img); // 转换为位图数据结构指针

    // 用于存储像素颜色的变量
    pixl_t pix;
    int k = 0, l = 0;

    // 遍历图像显示区域的行，从617到153（从下往上）
    for (int j = 617; j >= 153; j--, l++)
    {
        // 遍历每一行的列，从402到622（从左到右）
        for (int i = 402; i < 622; i++)
        {
            // 将当前位图数据的颜色转换为 BGRA 格式
            pix = BGRA(bmdp[k].bmd_r, bmdp[k].bmd_g, bmdp[k].bmd_b);
            // 将颜色写入指定的 (i, j) 坐标
            drxw_pixcolor(kghp, i, j, pix);
            k++; // 移动到下一个像素数据
        }

        // 如果显示的行数超过 205，则中断执行
        if (l > 205)
        {
            die(0x80); // 停止执行并退出
        }
    }
    return; // 返回，无其他操作
}



// 初始化 BGA 显示相关操作函数
void init_bga()
{
    // 声明一个指向 dftgraph_t 结构体的指针 kghp，并将其指向 kdftgh 结构体
    dftgraph_t *kghp = &kdftgh;

    // 检查当前显示模式是否为 BGA 模式，如果不是，则直接返回，不做任何初始化
    if (kghp->gh_mode != BGAMODE)
    {
        return;  // 如果不是 BGA 模式，退出函数
    }

    // 设置 BGA 显示相关的操作函数指针
    kghp->gh_opfun.dgo_read = bga_read;  // 设置读操作函数
    kghp->gh_opfun.dgo_write = bga_write;  // 设置写操作函数
    kghp->gh_opfun.dgo_ioctrl = bga_ioctrl;  // 设置 I/O 控制函数
    kghp->gh_opfun.dgo_flush = bga_flush;  // 设置刷新操作函数
    kghp->gh_opfun.dgo_set_bank = bga_set_bank;  // 设置银行切换操作函数
    kghp->gh_opfun.dgo_readpix = bga_readpix;  // 设置读取像素操作函数
    kghp->gh_opfun.dgo_writepix = bga_writepix;  // 设置写入像素操作函数
    kghp->gh_opfun.dgo_dxreadpix = bga_dxreadpix;  // 设置直接读取像素的操作函数
    kghp->gh_opfun.dgo_dxwritepix = bga_dxwritepix;  // 设置直接写入像素的操作函数
    kghp->gh_opfun.dgo_set_xy = bga_set_xy;  // 设置坐标 (x, y) 的操作函数
    kghp->gh_opfun.dgo_set_vwh = bga_set_vwh;  // 设置视窗 (width, height) 的操作函数
    kghp->gh_opfun.dgo_set_xyoffset = bga_set_xyoffset;  // 设置坐标偏移的操作函数
    kghp->gh_opfun.dgo_get_xy = bga_get_xy;  // 获取当前坐标 (x, y) 的操作函数
    kghp->gh_opfun.dgo_get_vwh = bga_get_vwh;  // 获取当前视窗 (width, height) 的操作函数
    kghp->gh_opfun.dgo_get_xyoffset = bga_get_xyoffset;  // 获取坐标偏移的操作函数

    // 返回（初始化完成）
    return;
}


// 初始化 VBE 显示相关操作函数
void init_vbe()
{
    // 声明一个指向 dftgraph_t 结构体的指针 kghp，并将其指向 kdftgh 结构体
    dftgraph_t *kghp = &kdftgh;

    // 检查当前显示模式是否为 VBE 模式，如果不是，则直接返回，不做任何初始化
    if (kghp->gh_mode != VBEMODE)
    {
        return;  // 如果不是 VBE 模式，退出函数
    }

    // 设置 VBE 显示相关的操作函数指针
    kghp->gh_opfun.dgo_read = vbe_read;  // 设置读操作函数
    kghp->gh_opfun.dgo_write = vbe_write;  // 设置写操作函数
    kghp->gh_opfun.dgo_ioctrl = vbe_ioctrl;  // 设置 I/O 控制函数
    kghp->gh_opfun.dgo_flush = vbe_flush;  // 设置刷新操作函数
    kghp->gh_opfun.dgo_set_bank = vbe_set_bank;  // 设置银行切换操作函数
    kghp->gh_opfun.dgo_readpix = vbe_readpix;  // 设置读取像素操作函数
    kghp->gh_opfun.dgo_writepix = vbe_writepix;  // 设置写入像素操作函数
    kghp->gh_opfun.dgo_dxreadpix = vbe_dxreadpix;  // 设置直接读取像素的操作函数
    kghp->gh_opfun.dgo_dxwritepix = vbe_dxwritepix;  // 设置直接写入像素的操作函数
    kghp->gh_opfun.dgo_set_xy = vbe_set_xy;  // 设置坐标 (x, y) 的操作函数
    kghp->gh_opfun.dgo_set_vwh = vbe_set_vwh;  // 设置视窗 (width, height) 的操作函数
    kghp->gh_opfun.dgo_set_xyoffset = vbe_set_xyoffset;  // 设置坐标偏移的操作函数
    kghp->gh_opfun.dgo_get_xy = vbe_get_xy;  // 获取当前坐标 (x, y) 的操作函数
    kghp->gh_opfun.dgo_get_vwh = vbe_get_vwh;  // 获取当前视窗 (width, height) 的操作函数
    kghp->gh_opfun.dgo_get_xyoffset = vbe_get_xyoffset;  // 获取坐标偏移的操作函数

    // 返回（初始化完成）
    return;
}


void init_bdvideo(){
    dftgraph_t *kghp = &kdftgh;

    init_dftgraph();
    init_bga();
    init_vbe();
    fill_graph(kghp,BGRA(0,0,0));
    set_charsdxwflush(0, 0);
    hal_background();

    return;
}




// 刷新显存内容的函数
void flush_videoram(dftgraph_t *kghp)
{
    // 调用显存刷新操作函数，执行刷新操作
    kghp->gh_opfun.dgo_flush(kghp);
    return; // 返回，无其他操作
}

// 向指定坐标写入延迟显示（双缓冲）像素颜色的函数
void drxw_pixcolor(dftgraph_t *kghp, u32_t x, u32_t y, pixl_t pix)
{
    // 调用双缓冲模式下的写入像素颜色操作函数
    // 参数包括图形处理结构指针、像素颜色值，以及目标坐标 (x, y)
    kghp->gh_opfun.dgo_dxwritepix(kghp, pix, x, y);

    return; // 返回，无其他操作
}


// 向指定位置写入像素颜色的函数
void write_pixcolor(dftgraph_t *kghp, u32_t x, u32_t y, pixl_t pix)
{
    // 调用写入像素颜色操作函数，将颜色写入指定的 (x, y) 坐标
    kghp->gh_opfun.dgo_writepix(kghp, pix, x, y);
    return; // 返回，无其他操作
}

// 用指定颜色填充整个图形区域的函数
void fill_graph(dftgraph_t *kghp, pixl_t pix)
{
    // 遍历图形区域的每一行
    for (u64_t y = 0; y < kghp->gh_y; y++)
    {
        // 遍历当前行的每一列
        for (u64_t x = 0; x < kghp->gh_x; x++)
        {
            // 向当前坐标 (x, y) 写入指定的像素颜色
            write_pixcolor(kghp, (u32_t)x, (u32_t)y, pix);
        }
    }
    // 完成所有像素填充后，刷新显存以更新屏幕显示
    flush_videoram(kghp);
    return; // 返回，无其他操作
}


// 将UTF-8编码转换为Unicode编码的函数
// 参数 utfp 是指向UTF-8编码的指针，retuib 用于返回UTF-8字符占用的字节数
u32_t utf8_to_unicode(utf8_t *utfp, int *retuib)
{
    u8_t uhd = utfp->utf_b1, ubyt = 0; // 获取第一个字节（utf_b1），初始化临时变量
    u32_t ucode = 0, tmpuc = 0;       // 定义Unicode码值和中间变量

    // 单字节编码 (0x00 - 0x7F)，即ASCII字符
    if (0x80 > uhd)
    {
        ucode = utfp->utf_b1 & 0x7f; // 提取有效的7位
        *retuib = 1;                // 设置字符占用字节数为1
        return ucode;               // 返回Unicode码值
    }

    // 双字节编码 (0xC0 - 0xDF)，即U+0080 ~ U+07FF范围的字符
    if (0xc0 <= uhd && uhd <= 0xdf)
    {
        ubyt = utfp->utf_b1 & 0x1f;     // 提取第一个字节的前5位
        tmpuc |= ubyt;                 // 保存到中间变量
        ubyt = utfp->utf_b2 & 0x3f;    // 提取第二个字节的后6位
        ucode = (tmpuc << 6) | ubyt;   // 组合为Unicode码值
        *retuib = 2;                   // 设置字符占用字节数为2
        return ucode;
    }

    // 三字节编码 (0xE0 - 0xEF)，即U+0800 ~ U+FFFF范围的字符
    if (0xe0 <= uhd && uhd <= 0xef)
    {
        ubyt = utfp->utf_b1 & 0x0f;     // 提取第一个字节的前4位
        tmpuc |= ubyt;                 // 保存到中间变量
        ubyt = utfp->utf_b2 & 0x3f;    // 提取第二个字节的后6位
        tmpuc <<= 6;                   // 左移6位准备接收新数据
        tmpuc |= ubyt;
        ubyt = utfp->utf_b3 & 0x3f;    // 提取第三个字节的后6位
        ucode = (tmpuc << 6) | ubyt;   // 组合为Unicode码值
        *retuib = 3;                   // 设置字符占用字节数为3
        return ucode;
    }

    // 四字节编码 (0xF0 - 0xF7)，即U+10000 ~ U+1FFFFF范围的字符
    if (0xf0 <= uhd && uhd <= 0xf7)
    {
        ubyt = utfp->utf_b1 & 0x7;      // 提取第一个字节的前3位
        tmpuc |= ubyt;                 // 保存到中间变量
        ubyt = utfp->utf_b2 & 0x3f;    // 提取第二个字节的后6位
        tmpuc <<= 6;                   // 左移6位
        tmpuc |= ubyt;
        ubyt = utfp->utf_b3 & 0x3f;    // 提取第三个字节的后6位
        tmpuc <<= 6;                   // 左移6位
        tmpuc |= ubyt;
        ubyt = utfp->utf_b4 & 0x3f;    // 提取第四个字节的后6位
        ucode = (tmpuc << 6) | ubyt;   // 组合为Unicode码值
        *retuib = 4;                   // 设置字符占用字节数为4
        return ucode;
    }

    // 五字节编码 (0xF8 - 0xFB)，即U+200000 ~ U+3FFFFFF范围的字符
    if (0xf8 <= uhd && uhd <= 0xfb)
    {
        ubyt = utfp->utf_b1 & 0x3;      // 提取第一个字节的前2位
        tmpuc |= ubyt;
        ubyt = utfp->utf_b2 & 0x3f;    // 提取第二个字节的后6位
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b3 & 0x3f;    // 提取第三个字节的后6位
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b4 & 0x3f;    // 提取第四个字节的后6位
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b5 & 0x3f;    // 提取第五个字节的后6位
        ucode = (tmpuc << 6) | ubyt;
        *retuib = 5;                   // 设置字符占用字节数为5
        return ucode;
    }

    // 六字节编码 (0xFC - 0xFD)，即U+4000000 ~ U+7FFFFFFF范围的字符
    if (0xfc <= uhd && uhd <= 0xfd)
    {
        ubyt = utfp->utf_b1 & 0x1;      // 提取第一个字节的前1位
        tmpuc |= ubyt;
        ubyt = utfp->utf_b2 & 0x3f;    // 提取第二个字节的后6位
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b3 & 0x3f;    // 提取第三个字节的后6位
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b4 & 0x3f;    // 提取第四个字节的后6位
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b5 & 0x3f;    // 提取第五个字节的后6位
        tmpuc <<= 6;
        tmpuc |= ubyt;
        ubyt = utfp->utf_b6 & 0x3f;    // 提取第六个字节的后6位
        ucode = (tmpuc << 6) | ubyt;
        *retuib = 6;                   // 设置字符占用字节数为6
        return ucode;
    }

    *retuib = 0; // 无效的UTF-8字节序列，返回0字节
    return 0;    // 返回默认值0
}

// 获取字符信息的函数
// 参数：
// kghp - 图形处理结构的指针
// unicode - 目标字符的Unicode编码
// retchwx - 返回字符宽度的指针
// retchhx - 返回字符高度的指针
// retlinb - 返回每行字节数的指针
// 返回值：字符位图数据的起始地址
u8_t *ret_charsinfo(dftgraph_t *kghp, u32_t unicode, int *retchwx, int *retchhx, int *retlinb)
{
    // 获取字体数据的起始地址
    fntdata_t *fntp = (fntdata_t *)((uint_t)kghp->gh_fontadr);

    // 从字体数据中提取字符的宽度（像素单位）
    *retchwx = fntp[unicode].fntwx;

    // 从字体数据中提取字符的高度（像素单位）
    *retchhx = fntp[unicode].fnthx;

    // 从字体数据中提取字符每行占用的字节数
    *retlinb = fntp[unicode].fntwxbyte;

    // 返回字符位图数据的起始地址
    return (u8_t *)(&(fntp[unicode].fntchmap[0]));
}


// 在屏幕上绘制一个字符的函数
// 参数：
// kghp - 图形处理结构的指针
// unicode - 要绘制的字符的Unicode编码
// pix - 字符颜色
void put_pixonechar(dftgraph_t *kghp, u32_t unicode, pixl_t pix)
{
    int y = 0, chwx = 0, chhx = 0, linb = 0; // 初始化字符高度、宽度及每行字节数
    u8_t *charyp = NULL;                    // 字符位图数据指针

    // 如果字符是换行符（Unicode值为10）
    if (unicode == 10)
    {
        kghp->gh_nxtcharsx = 0; // 将下一个字符的x坐标重置为0
        // 判断是否需要滚动到下一行
        if ((kghp->gh_nxtcharsy + kghp->gh_linesz) > (kghp->gh_y - kghp->gh_fnthight))
        {
            kghp->gh_nxtcharsy = 0; // 将y坐标重置为顶部
            hal_background();       // 清除屏幕背景
            return;
        }
        kghp->gh_nxtcharsy += kghp->gh_linesz; // 移动到下一行
        return;
    }

    // 获取字符的图形信息
    charyp = ret_charsinfo(kghp, unicode, &chwx, &chhx, &linb);
    if (charyp == NULL) // 如果无法获取字符信息，返回
    {
        return;
    }

    // 检查字符是否会超出当前行的宽度
    if ((kghp->gh_nxtcharsx + chwx) > (kghp->gh_x - chwx))
    {
        kghp->gh_nxtcharsx = 0; // 换行并重置x坐标
        if ((kghp->gh_nxtcharsy + kghp->gh_linesz) > (kghp->gh_y - kghp->gh_fnthight))
        {
            kghp->gh_nxtcharsy = 0; // 超出屏幕则回到顶部
        }
        else
        {
            kghp->gh_nxtcharsy += kghp->gh_linesz; // 移动到下一行
        }
    }

    // 计算字符绘制的起始位置
    u32_t wpx = (u32_t)kghp->gh_nxtcharsx, wpy = (u32_t)kghp->gh_nxtcharsy;

    // 遍历字符的位图数据，逐行绘制
    for (int hx = 0; hx < chhx; hx++) // 遍历字符高度
    {
        for (int btyi = 0, x = 0; btyi < linb; btyi++) // 遍历每行字节
        {
            u8_t bitmap = charyp[hx * linb + btyi]; // 获取当前字节的位图数据
            if (bitmap == 0) // 如果当前字节无数据，跳过8个像素
            {
                x += 8;
                continue;
            }
            for (int bi = 7; bi >= 0; bi--) // 遍历字节的每一位
            {
                if (x >= chwx) // 如果超出字符宽度，停止
                {
                    break;
                }
                if (((bitmap >> bi) & 1) == 1) // 判断当前位是否需要绘制
                {
                    write_pixcolor(kghp, wpx + x, wpy + y, pix); // 绘制一个像素
                }
                x++; // 移动到下一个像素
            }
        }
        y++; // 移动到字符的下一行
    }

    // 更新下一个字符的起始x坐标
    kghp->gh_nxtcharsx += (u64_t)chwx;

    return; // 返回，无其他操作
}


// 在屏幕上绘制一个字符（使用延迟显示的方式）
// 参数：
// kghp - 图形处理结构的指针
// unicode - 要绘制的字符的Unicode编码
// pix - 字符颜色
void drxput_pixonechar(dftgraph_t *kghp, u32_t unicode, pixl_t pix)
{
    int y = 0, chwx = 0, chhx = 0, linb = 0; // 初始化字符高度、宽度及每行字节数
    u8_t *charyp = NULL;                    // 字符位图数据指针

    // 如果字符是换行符（Unicode值为10）
    if (unicode == 10)
    {
        kghp->gh_nxtcharsx = 0; // 将下一个字符的x坐标重置为0
        // 判断是否需要滚动到下一行
        if ((kghp->gh_nxtcharsy + kghp->gh_linesz) > (kghp->gh_y - kghp->gh_fnthight))
        {
            kghp->gh_nxtcharsy = 0; // 将y坐标重置为顶部
            hal_background();       // 清除屏幕背景
            return;
        }
        kghp->gh_nxtcharsy += kghp->gh_linesz; // 移动到下一行
        return;
    }

    // 获取字符的图形信息
    charyp = ret_charsinfo(kghp, unicode, &chwx, &chhx, &linb);
    if (charyp == NULL) // 如果无法获取字符信息，返回
    {
        return;
    }

    // 检查字符是否会超出当前行的宽度
    if ((kghp->gh_nxtcharsx + chwx) > (kghp->gh_x - chwx))
    {
        kghp->gh_nxtcharsx = 0; // 换行并重置x坐标
        if ((kghp->gh_nxtcharsy + kghp->gh_linesz) > (kghp->gh_y - kghp->gh_fnthight))
        {
            kghp->gh_nxtcharsy = 0; // 超出屏幕则回到顶部
        }
        else
        {
            kghp->gh_nxtcharsy += kghp->gh_linesz; // 移动到下一行
        }
    }

    // 计算字符绘制的起始位置
    u32_t wpx = (u32_t)kghp->gh_nxtcharsx, wpy = (u32_t)kghp->gh_nxtcharsy;

    // 遍历字符的位图数据，逐行绘制
    for (int hx = 0; hx < chhx; hx++) // 遍历字符高度
    {
        for (int btyi = 0, x = 0; btyi < linb; btyi++) // 遍历每行字节
        {
            u8_t bitmap = charyp[hx * linb + btyi]; // 获取当前字节的位图数据
            if (bitmap == 0) // 如果当前字节无数据，跳过8个像素
            {
                x += 8;
                continue;
            }
            for (int bi = 7; bi >= 0; bi--) // 遍历字节的每一位
            {
                if (x >= chwx) // 如果超出字符宽度，停止
                {
                    break;
                }
                if (((bitmap >> bi) & 1) == 1) // 判断当前位是否需要绘制
                {
                    drxw_pixcolor(kghp, wpx + x, wpy + y, pix); // 延迟写入像素颜色
                }
                x++; // 移动到下一个像素
            }
        }
        y++; // 移动到字符的下一行
    }

    // 更新下一个字符的起始x坐标
    kghp->gh_nxtcharsx += (u64_t)chwx;

    return; // 返回，无其他操作
}

// 向屏幕输出字符串的函数
// 参数：
// kghp - 图形处理结构的指针
// buf - 要输出的字符串缓冲区
void gstr_write(dftgraph_t *kghp, char_t *buf)
{
    int chinx = 0;          // 字符索引
    int chretnext = 0;      // 当前UTF-8字符占用的字节数
    u32_t uc = 0;           // 当前字符的Unicode编码
    pixl_t pix = kghp->gh_deffontpx; // 获取当前默认字体颜色

    // 遍历字符串，直到遇到字符串结束符
    while (buf[chinx] != 0)
    {
        // 将UTF-8编码的字符转换为Unicode编码
        uc = utf8_to_unicode((utf8_t *)(&buf[chinx]), &chretnext);

        // 移动到下一个UTF-8字符的位置
        chinx += chretnext;

        // 判断当前显示模式：普通写入或延迟写入
        if (kghp->gh_chardxw == 1)
        {
            // 普通写入单个字符到屏幕
            put_pixonechar(kghp, uc, pix);
        }
        else
        {
            // 延迟写入单个字符到屏幕
            drxput_pixonechar(kghp, uc, pix);
        }
    }

    // 如果需要刷新显示缓冲区，则执行刷新操作
    if (kghp->gh_flush == 1)
    {
        flush_videoram(kghp);
    }

    return; // 返回，无其他操作
}


// 向BGA寄存器写入数据
void bga_write_reg(u16_t index, u16_t data)
{
    out_u16(VBE_DISPI_IOPORT_INDEX, index); // 设置寄存器索引
    out_u16(VBE_DISPI_IOPORT_DATA, data);  // 写入寄存器数据
    return;
}

// 从BGA寄存器读取数据
u16_t bga_read_reg(u16_t index)
{
    out_u16(VBE_DISPI_IOPORT_INDEX, index); // 设置寄存器索引
    return in_u16(VBE_DISPI_IOPORT_DATA);   // 返回寄存器中的数据
}

// 读取操作（未实现），仅返回读取大小
size_t bga_read(void *ghpdev, void *outp, size_t rdsz)
{
    return rdsz;
}

// 写入操作（未实现），仅返回写入大小
size_t bga_write(void *ghpdev, void *inp, size_t wesz)
{
    return wesz;
}

// IO控制操作（未实现），返回错误码
sint_t bga_ioctrl(void *ghpdev, void *outp, uint_t iocode)
{
    return -1;
}

// 返回当前显存块的基地址
u64_t *ret_vramadr_inbnk(void *ghpdev)
{
    dftgraph_t *kghp = (dftgraph_t *)ghpdev;
    u64_t *d = (u64_t *)((uint_t)(kghp->gh_framphyadr + 
              (kghp->gh_x * kghp->gh_y * kghp->gh_onepixbyte * kghp->gh_nextbnk)));
    return d;
}

// 切换到下一显存块
void bga_disp_nxtbank(void *ghpdev)
{
    dftgraph_t *kghp = (dftgraph_t *)ghpdev;
    u16_t h = (u16_t)(kghp->gh_y * kghp->gh_nextbnk + 1); // 计算虚拟高度
    u16_t ofy = (u16_t)(kghp->gh_y * (kghp->gh_nextbnk)); // 计算y偏移量
    bga_write_reg(VBE_DISPI_INDEX_VIRT_HEIGHT, h);       // 写入虚拟高度
    bga_write_reg(VBE_DISPI_INDEX_Y_OFFSET, ofy);        // 写入y偏移量
    kghp->gh_curdipbnk = kghp->gh_nextbnk;               // 更新当前显存块
    kghp->gh_nextbnk++;
    if (kghp->gh_nextbnk > kghp->gh_bank)                // 循环切换显存块
    {
        kghp->gh_nextbnk = 0;
    }
    return;
}

// 刷新显存，将显存中的数据复制到当前显存块
void bga_flush(void *ghpdev)
{
    dftgraph_t *kghp = (dftgraph_t *)ghpdev;
    u64_t *s = (u64_t *)((uint_t)kghp->gh_fvrmphyadr);    // 源显存地址
    u64_t *d = ret_vramadr_inbnk(kghp);                   // 目标显存块地址
    u64_t i = 0, j = 0;
    u64_t e = kghp->gh_x * kghp->gh_y * kghp->gh_onepixbyte; // 显存数据大小

    for (; i < e; i += 8)
    {
        d[j] = s[j]; // 数据复制
        j++;
    }
    bga_disp_nxtbank(kghp); // 切换到下一显存块
    return;
}

// 设置显存块（未实现），返回错误码
sint_t bga_set_bank(void *ghpdev, sint_t bnr)
{
    return -1;
}

// 从屏幕读取像素颜色（未实现），返回默认值
pixl_t bga_readpix(void *ghpdev, uint_t x, uint_t y)
{
    return 0;
}

// 向屏幕写入像素颜色
void bga_writepix(void *ghpdev, pixl_t pix, uint_t x, uint_t y)
{
    dftgraph_t *kghp = (dftgraph_t *)ghpdev;
    if (kghp->gh_onepixbits == 24) // 如果是24位色
    {
        u64_t p24adr = (x + (y * kghp->gh_x)) * 3; // 计算像素地址
        u8_t *p24bas = (u8_t *)((uint_t)(p24adr + kghp->gh_fvrmphyadr));
        p24bas[0] = (u8_t)(pix);        // 写入蓝色
        p24bas[1] = (u8_t)(pix >> 8);  // 写入绿色
        p24bas[2] = (u8_t)(pix >> 16); // 写入红色
        return;
    }
    u32_t *phybas = (u32_t *)((uint_t)kghp->gh_fvrmphyadr); // 如果是32位色
    phybas[x + (y * kghp->gh_x)] = pix; // 写入像素数据
    return;
}

// 延迟读取像素颜色（未实现），返回默认值
pixl_t bga_dxreadpix(void *ghpdev, uint_t x, uint_t y)
{
    return 0;
}

// 延迟向屏幕写入像素颜色
void bga_dxwritepix(void *ghpdev, pixl_t pix, uint_t x, uint_t y)
{
    dftgraph_t *kghp = (dftgraph_t *)ghpdev;
    if (kghp->gh_onepixbits == 24) // 如果是24位色
    {
        u64_t p24adr = (x + (y * kghp->gh_x)) * 3 * kghp->gh_curdipbnk; // 计算像素地址
        u8_t *p24bas = (u8_t *)((uint_t)(p24adr + kghp->gh_framphyadr));
        p24bas[0] = (u8_t)(pix);        // 写入蓝色
        p24bas[1] = (u8_t)(pix >> 8);  // 写入绿色
        p24bas[2] = (u8_t)(pix >> 16); // 写入红色
        return;
    }
    u32_t *phybas = (u32_t *)((uint_t)(kghp->gh_framphyadr + 
             (kghp->gh_x * kghp->gh_y * kghp->gh_onepixbyte * kghp->gh_curdipbnk)));
    phybas[x + (y * kghp->gh_x)] = pix; // 写入像素数据
    return;
}

// 设置屏幕分辨率
sint_t bga_set_xy(void *ghpdev, uint_t x, uint_t y)
{
    bga_write_reg(VBE_DISPI_INDEX_XRES, (u16_t)(x)); // 设置x分辨率
    bga_write_reg(VBE_DISPI_INDEX_YRES, (u16_t)(y)); // 设置y分辨率
    return 0;
}

// 设置虚拟宽高
sint_t bga_set_vwh(void *ghpdev, uint_t vwt, uint_t vhi)
{
    bga_write_reg(VBE_DISPI_INDEX_VIRT_WIDTH, (u16_t)vwt);  // 设置虚拟宽度
    bga_write_reg(VBE_DISPI_INDEX_VIRT_HEIGHT, (u16_t)vhi); // 设置虚拟高度
    return 0;
}

// 设置屏幕偏移量
sint_t bga_set_xyoffset(void *ghpdev, uint_t xoff, uint_t yoff)
{
    bga_write_reg(VBE_DISPI_INDEX_X_OFFSET, (u16_t)(xoff)); // 设置x偏移
    bga_write_reg(VBE_DISPI_INDEX_Y_OFFSET, (u16_t)(yoff)); // 设置y偏移
    return 0;
}

// 获取屏幕分辨率
sint_t bga_get_xy(void *ghpdev, uint_t *rx, uint_t *ry)
{
    if (rx == NULL || ry == NULL)
    {
        return -1;
    }
    *rx = (uint_t)bga_read_reg(VBE_DISPI_INDEX_XRES); // 读取x分辨率
    *ry = (uint_t)bga_read_reg(VBE_DISPI_INDEX_YRES); // 读取y分辨率
    return 0;
}

// 获取虚拟宽高
sint_t bga_get_vwh(void *ghpdev, uint_t *rvwt, uint_t *rvhi)
{
    if (rvwt == NULL || rvhi == NULL)
    {
        return -1;
    }
    *rvwt = (uint_t)bga_read_reg(VBE_DISPI_INDEX_VIRT_WIDTH);  // 读取虚拟宽度
    *rvhi = (uint_t)bga_read_reg(VBE_DISPI_INDEX_VIRT_HEIGHT); // 读取虚拟高度
    return 0;
}

// 获取屏幕偏移量
sint_t bga_get_xyoffset(void *ghpdev, uint_t *rxoff, uint_t *ryoff)
{
    if (rxoff == NULL || ryoff == NULL)
    {
        return -1;
    }
    *rxoff = (uint_t)bga_read_reg(VBE_DISPI_INDEX_X_OFFSET); // 读取x偏移
    *ryoff = (uint_t)bga_read_reg(VBE_DISPI_INDEX_Y_OFFSET); // 读取y偏移
    return 0;
}


// 读取操作（未实现），直接返回读取的大小
size_t vbe_read(void *ghpdev, void *outp, size_t rdsz)
{
    return rdsz; // 返回请求的读取大小
}

// 写入操作（未实现），直接返回写入的大小
size_t vbe_write(void *ghpdev, void *inp, size_t wesz)
{
    return wesz; // 返回请求的写入大小
}

// IO控制操作（未实现），返回错误码
sint_t vbe_ioctrl(void *ghpdev, void *outp, uint_t iocode)
{
    return -1; // 返回-1表示功能未实现
}

// 刷新显存，将显存中的数据复制到帧缓冲区
void vbe_flush(void *ghpdev)
{
    dftgraph_t *kghp = (dftgraph_t *)ghpdev; // 转换为图形处理结构指针

    u64_t *s = (u64_t *)((uint_t)kghp->gh_fvrmphyadr); // 源显存地址
    u64_t *d = (u64_t *)((uint_t)kghp->gh_framphyadr); // 目标帧缓冲区地址
    u64_t i = 0, j = 0;
    u64_t e = kghp->gh_x * kghp->gh_y * kghp->gh_onepixbyte; // 显存数据总大小

    for (; i < e; i += 8)
    {
        d[j] = s[j]; // 将显存数据逐字复制到帧缓冲区
        j++;
    }
    return; // 无其他操作，返回
}

// 设置显存块（未实现），返回错误码
sint_t vbe_set_bank(void *ghpdev, sint_t bnr)
{
    return -1; // 返回-1表示功能未实现
}

// 从屏幕读取像素颜色（未实现），返回默认值
pixl_t vbe_readpix(void *ghpdev, uint_t x, uint_t y)
{
    return 0; // 返回默认颜色值
}

// 向屏幕写入像素颜色
void vbe_writepix(void *ghpdev, pixl_t pix, uint_t x, uint_t y)
{
    dftgraph_t *kghp = (dftgraph_t *)ghpdev; // 转换为图形处理结构指针
    if (kghp->gh_onepixbits == 24) // 如果是24位色
    {
        u64_t p24adr = (x + (y * kghp->gh_x)) * 3; // 计算像素地址
        u8_t *p24bas = (u8_t *)((uint_t)(p24adr + kghp->gh_fvrmphyadr));
        p24bas[0] = (u8_t)(pix);        // 写入蓝色
        p24bas[1] = (u8_t)(pix >> 8);  // 写入绿色
        p24bas[2] = (u8_t)(pix >> 16); // 写入红色
        return;
    }
    u32_t *phybas = (u32_t *)((uint_t)kghp->gh_fvrmphyadr); // 如果是32位色
    phybas[x + (y * kghp->gh_x)] = pix; // 写入像素数据
    return;
}

// 延迟读取像素颜色（未实现），返回默认值
pixl_t vbe_dxreadpix(void *ghpdev, uint_t x, uint_t y)
{
    return 0; // 返回默认颜色值
}

// 延迟向屏幕写入像素颜色
void vbe_dxwritepix(void *ghpdev, pixl_t pix, uint_t x, uint_t y)
{
    dftgraph_t *kghp = (dftgraph_t *)ghpdev; // 转换为图形处理结构指针
    if (kghp->gh_onepixbits == 24) // 如果是24位色
    {
        u64_t p24adr = (x + (y * kghp->gh_x)) * 3; // 计算像素地址
        u8_t *p24bas = (u8_t *)((uint_t)(p24adr + kghp->gh_framphyadr));
        p24bas[0] = (u8_t)(pix);        // 写入蓝色
        p24bas[1] = (u8_t)(pix >> 8);  // 写入绿色
        p24bas[2] = (u8_t)(pix >> 16); // 写入红色
        return;
    }
    u32_t *phybas = (u32_t *)((uint_t)kghp->gh_framphyadr); // 如果是32位色
    phybas[x + (y * kghp->gh_x)] = pix; // 写入像素数据
    return;
}

// 设置屏幕分辨率（未实现），返回错误码
sint_t vbe_set_xy(void *ghpdev, uint_t x, uint_t y)
{
    return -1; // 返回-1表示功能未实现
}

// 设置虚拟宽高（未实现），返回错误码
sint_t vbe_set_vwh(void *ghpdev, uint_t vwt, uint_t vhi)
{
    return -1; // 返回-1表示功能未实现
}

// 设置屏幕偏移量（未实现），返回错误码
sint_t vbe_set_xyoffset(void *ghpdev, uint_t xoff, uint_t yoff)
{
    return -1; // 返回-1表示功能未实现
}

// 获取屏幕分辨率（未实现），返回错误码
sint_t vbe_get_xy(void *ghpdev, uint_t *rx, uint_t *ry)
{
    return -1; // 返回-1表示功能未实现
}

// 获取虚拟宽高（未实现），返回错误码
sint_t vbe_get_vwh(void *ghpdev, uint_t *rvwt, uint_t *rvhi)
{
    return -1; // 返回-1表示功能未实现
}

// 获取屏幕偏移量（未实现），返回错误码
sint_t vbe_get_xyoffset(void *ghpdev, uint_t *rxoff, uint_t *ryoff)
{
    return -1; // 返回-1表示功能未实现
}
