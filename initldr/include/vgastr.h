#ifndef _VGASTR_H
#define _VGASTR_H

void kprint(const char_t *fmt, ...);
void init_curs();
void close_curs();
void clear_screen(u16_t srrv);
char_t *numberk(char_t *str, uint_t n, sint_t base);
char_t *strcopy(char_t *buf, char_t *str_s);
void GxH_strwrite(char_t *str, cursor_t *cursptr);


KLINE void current_curs(cursor_t *cp, u32_t c_flg)
{

	if (c_flg == VGACHAR_LR_CFLG)// 如果传入的标志是VGACHAR_LR_CFLG，表示需要换行
	{
		cp->y++;// 光标的y坐标加1，即移动到下一行
		cp->x = 0;// 光标的x坐标重置为0，即行首
		if (cp->y > 24)// 如果光标的y坐标超过24（VGA文本模式下屏幕的最大行数为25行）
		{
			cp->y = 0;// 光标的y坐标重置为0，即屏幕顶部
			clear_screen(VGADP_DFVL);// 清除屏幕
		}

		return;
	}
	if (c_flg == VGACHAR_DF_CFLG)// 如果传入的标志是VGACHAR_DF_CFLG，表示在当前行继续写入
	{
		cp->x += 2;// 光标的x坐标加2（因为VGA文本模式下每个字符占两个字节）
		if (cp->x > 159) // 如果光标的x坐标超过159（VGA文本模式下每行的最大字符数为80个，每个字符占两个字节）
		{
			cp->x = 0;// 光标的x坐标重置为0，即行首
			cp->y++;// 光标的y坐标加1，即移动到下一行
			if (cp->y > 24) // 如果光标的y坐标超过24
			{
				cp->y = 0;// 光标的y坐标重置为0，即屏幕顶部
				clear_screen(VGADP_DFVL);// 清除屏幕
			}
			return;
		}
	}
	return;
}
#endif
