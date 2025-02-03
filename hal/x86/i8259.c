#include "cosmostypes.h"
#include "cosmosmctrl.h"
// 初始化 i8259 中断控制器
void init_i8259()
{
    // 向端口发送初始化命令字，配置主、从 8259 控制器
    out_u8_p(ZIOPT, ICW1); // 发送 ICW1 到主 i8259
    out_u8_p(SIOPT, ICW1); // 发送 ICW1 到从 i8259
    out_u8_p(ZIOPT1, ZICW2); // 发送 ICW2 到主 i8259
    out_u8_p(SIOPT1, SICW2); // 发送 ICW2 到从 i8259
    out_u8_p(ZIOPT1, ZICW3); // 发送 ICW3 到主 i8259
    out_u8_p(SIOPT1, SICW3); // 发送 ICW3 到从 i8259
    out_u8_p(ZIOPT1, ICW4); // 发送 ICW4 到主 i8259
    out_u8_p(SIOPT1, ICW4); // 发送 ICW4 到从 i8259

    // 禁用所有中断
    out_u8_p(ZIOPT1, 0xff); // 禁用主 i8259 中的所有中断
    out_u8_p(SIOPT1, 0xff); // 禁用从 i8259 中的所有中断
    
    return;
}

// 发送中断结束信号 (EOI)
void i8259_send_eoi()
{
    // 向主、从 i8259 发送中断结束信号 (EOI)
    out_u8_p(_INTM_CTL, _EOI);
    out_u8_p(_INTS_CTL, _EOI);
    return;
}

// 启用指定中断线
void i8259_enabled_line(u32_t line)
{
    cpuflg_t flags;
    // 保存中断标志并禁止中断
    save_flags_cli(&flags);

    // 判断指定中断线属于哪个控制器 (主控制器 0-7 从控制器 8-15)
    if (line < 8)
    {
        u8_t amtmp = in_u8(_INTM_CTLMASK); // 读取主控制器的中断屏蔽寄存器
        amtmp &= (u8_t)(~(1 << line)); // 清除对应中断线的掩码 (启用该中断线)
        out_u8_p(_INTM_CTLMASK, amtmp); // 写回主控制器中断屏蔽寄存器
    }
    else
    {
        u8_t astmp = in_u8(_INTM_CTLMASK); // 读取主控制器的中断屏蔽寄存器
        astmp &= (u8_t)(~(1 << 2)); // 清除主控制器上的 IR2 位
        out_u8_p(_INTM_CTLMASK, astmp); // 写回主控制器中断屏蔽寄存器

        astmp = in_u8(_INTS_CTLMASK); // 读取从控制器的中断屏蔽寄存器
        astmp &= (u8_t)(~(1 << (line - 8))); // 清除从控制器的对应中断位
        out_u8_p(_INTS_CTLMASK, astmp); // 写回从控制器中断屏蔽寄存器
    }

    // 恢复中断标志并允许中断
    restore_flags_sti(&flags);
    return;
}

// 禁用指定中断线
void i8259_disable_line(u32_t line)
{
    cpuflg_t flags;
    // 保存中断标志并禁止中断
    save_flags_cli(&flags);

    // 判断指定中断线属于哪个控制器 (主控制器 0-7 从控制器 8-15)
    if (line < 8)
    {
        u8_t amtmp = in_u8(_INTM_CTLMASK); // 读取主控制器的中断屏蔽寄存器
        amtmp |= (u8_t)((1 << line)); // 设置对应中断线的掩码 (禁用该中断线)
        out_u8_p(_INTM_CTLMASK, amtmp); // 写回主控制器中断屏蔽寄存器
    }
    else
    {
        u8_t astmp = in_u8(_INTM_CTLMASK); // 读取主控制器的中断屏蔽寄存器
        astmp |= (u8_t)((1 << 2)); // 设置主控制器上的 IR2 位 (禁用从控制器)
        out_u8_p(_INTM_CTLMASK, astmp); // 写回主控制器中断屏蔽寄存器

        astmp = in_u8(_INTS_CTLMASK); // 读取从控制器的中断屏蔽寄存器
        astmp |= (u8_t)((1 << (line - 8))); // 设置从控制器对应中断位的掩码
        out_u8_p(_INTS_CTLMASK, astmp); // 写回从控制器中断屏蔽寄存器
    }

    // 恢复中断标志并允许中断
    restore_flags_sti(&flags);
    return;
}

// 保存当前中断线状态并禁用指定中断线
void i8259_save_disableline(u64_t *svline, u32_t line)
{
    u32_t intftmp;
    cpuflg_t flags;
    // 保存中断标志并禁止中断
    save_flags_cli(&flags);

    u8_t altmp = in_u8(_INTM_CTLMASK); // 读取主控制器的中断屏蔽寄存器
    intftmp = altmp;
    altmp = in_u8(_INTS_CTLMASK); // 读取从控制器的中断屏蔽寄存器
    intftmp = (intftmp << 8) | altmp; // 将两者合并为一个 32 位值

    *svline = intftmp; // 保存当前的中断状态

    // 禁用指定的中断线
    i8259_disable_line(line);

    // 恢复中断标志并允许中断
    restore_flags_sti(&flags);
    return;
}

// 恢复之前保存的中断线状态并恢复指定中断线
void i8259_rest_enabledline(u64_t *svline, u32_t line)
{
    cpuflg_t flags;
    // 保存中断标志并禁止中断
    save_flags_cli(&flags);

    u32_t intftmp = (u32_t)(*svline); // 从保存的状态恢复

    u8_t altmp = (intftmp & 0xff); // 提取从控制器的状态
    out_u8_p(_INTS_CTLMASK, altmp); // 恢复从控制器的中断掩码
    altmp = (u8_t)(intftmp >> 8); // 提取主控制器的状态
    out_u8_p(_INTM_CTLMASK, altmp); // 恢复主控制器的中断掩码

    // 恢复中断标志并允许中断
    restore_flags_sti(&flags);

    return;
}
