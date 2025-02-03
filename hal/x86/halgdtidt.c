#include "cosmostypes.h"
#include "cosmosmctrl.h"

void set_idt_desc(u8_t vector, u8_t desc_type, inthandler_t handler, u8_t privilege)// 中断号 中断描述符的类型 中断处理程序 特权级
{
    // 获取 IDT 中指定 vector 对应的描述符位置
    gate_t *p_gate = &x64_idt[vector];

    // 将中断处理程序的地址转换为64位地址并存储到 base
    u64_t base = (u64_t)handler;

    // 目标代码段偏移低16位
    p_gate->offset_low = base & 0xFFFF;

    //设置选择符为内核代码段选择子
    p_gate->selector = SELECTOR_KERNEL_CS; //1 000

    // 保留字段以及IST
    p_gate->dcount = 0;

    // 设置门描述符的属性，desc_type 是中断描述符的类型，privilege 是特权级，结合起来设置 attr 字段
    p_gate->attr = (u8_t)(desc_type | (privilege << 5));

    // 目标代码段偏移中部
    p_gate->offset_high = (u16_t)((base >> 16) & 0xFFFF);

    // 目标代码段偏移高32位
    p_gate->offset_high_h = (u32_t)((base >> 32) & 0xffffffff);

    // 保留字段设置为 0
    p_gate->offset_resv = 0;

    return;  // 设置完成后返回
}


void set_igdtr(descriptor_t *gdtptr)
{

    return;
}

void set_iidtr(gate_t *idtptr)
{
    // 将 IDT（中断描述符表）基址设置到 x64_iidt_reg.idtbass 中
    // 将 IDT 的起始地址（指针 idtptr）转换为 u64_t 类型并赋值
    // x64_iidt_reg.idtbass 存储的是 IDT 的物理基址
    x64_iidt_reg.idtbass = (u64_t)idtptr;

    // 设置 IDT 的长度。IDT 长度是 IDT 表的字节数减去 1（因为 IDT 的大小是从 0 开始的）
    // sizeof(x64_idt) 是 IDT 表的总字节数
    x64_iidt_reg.idtLen = sizeof(x64_idt) - 1;

    // 函数返回类型为 void，不需要返回值
    return;
}

void set_descriptor(descriptor_t *p_desc, u32_t base, u32_t limit, u16_t attribute)
{
    p_desc->limit_low = limit & 0x0FFFF;                                                   // 段界限 1(2 字节)
    p_desc->base_low = base & 0x0FFFF;                                                     // 段基址 1(2 字节)
    p_desc->base_mid = (base >> 16) & 0x0FF;                                               // 段基址 2(1 字节)
    p_desc->attr1 = (u8_t)(attribute & 0xFF);                                              // 属性 1
    p_desc->limit_high_attr2 = (u8_t)(((limit >> 16) & 0x0F) | ((attribute >> 8) & 0xF0)); // 段界限 2 + 属性 2
    p_desc->base_high = (u8_t)((base >> 24) & 0x0FF);                                      // 段基址 3\(1 字节)
    return;
}

void set_x64tss_descriptor(descriptor_t *p_desc, u64_t base, u32_t limit, u16_t attribute)
{
    u32_t *x64tssb_h = (u32_t *)(p_desc + 1);

    p_desc->limit_low = limit & 0x0FFFF;                                                   // 段界限 1(2 字节)
    p_desc->base_low = base & 0x0FFFF;                                                     // 段基址 1(2 字节)
    p_desc->base_mid = (base >> 16) & 0x0FF;                                               // 段基址 2(1 字节)
    p_desc->attr1 = (u8_t)(attribute & 0xFF);                                              // 属性 1
    p_desc->limit_high_attr2 = (u8_t)(((limit >> 16) & 0x0F) | ((attribute >> 8) & 0xF0)); // 段界限 2 + 属性 2
    p_desc->base_high = (u8_t)((base >> 24) & 0x0FF);

    *x64tssb_h = (u32_t)((base >> 32) & 0xffffffff); //7.0

    *(x64tssb_h + 1) = 0;//7.5
}

PUBLIC LKINIT void load_x64_gdt(igdtr_t *igdtrp)
{

    __asm__ __volatile__(

        "cli \n\t"
        "pushq %%rax \n\t"
        "lgdt (%0) \n\t"
        "movabsq $1f,%%rax \n\t"
        "pushq   $8 \n\t"
        "pushq   %%rax    \n\t"
        "lretq \n\t"
        "1:\n\t"
        "movw $0x10,%%ax\n\t"
        "movw %%ax,%%ds\n\t"
        "movw %%ax,%%es\n\t"
        "movw %%ax,%%ss\n\t"
        "movw %%ax,%%fs\n\t"
        "movw %%ax,%%gs\n\t"
        "popq %%rax \n\t"
        :
        : "r"(igdtrp)
        : "rax", "memory");
    return;
}

PUBLIC LKINIT void load_x64_idt(iidtr_t *idtptr)
{
    __asm__ __volatile__(
        "lidt (%0) \n\t"
        :
        : "r"(idtptr)
        : "memory");
    return;
}

PUBLIC LKINIT void load_x64_tr(u16_t trindx)
{
    __asm__ __volatile__(
        "ltr %0 \n\t"
        :
        : "r"(trindx)
        : "memory");
}

PUBLIC LKINIT void init_descriptor()
{
    // 遍历每个 CPU 核心进行描述符表的初始化
    for (u32_t gdtindx = 0; gdtindx < CPUCORE_MAX; gdtindx++)
    {
        // 初始化每个 GDT 段描述符为 0
        set_descriptor(&x64_gdt[gdtindx][0], 0, 0, 0);
        
        // 设置 GDT 中的代码段描述符，属性：存在、64位、特权级0
        set_descriptor(&x64_gdt[gdtindx][1], 0, 0, DA_CR | DA_64 | 0);
        
        // 设置 GDT 中的数据段描述符，属性：可读写、64位、特权级0
        set_descriptor(&x64_gdt[gdtindx][2], 0, 0, DA_DRW | DA_64 | 0);
        
        // 设置 GDT 中的用户代码段描述符，属性：存在、64位、特权级3
        set_descriptor(&x64_gdt[gdtindx][3], 0, 0, DA_CR | DA_64 | DA_DPL3 | 0); 
        
        // 设置 GDT 中的用户数据段描述符，属性：可读写、64位、特权级3
        set_descriptor(&x64_gdt[gdtindx][4], 0, 0, DA_DRW | DA_64 | DA_DPL3 | 0);
        
        // 设置 GDT 中的 TSS 描述符，属性：386 TSS（任务状态段）
        set_x64tss_descriptor(&x64_gdt[gdtindx][6], (u64_t)&x64tss[gdtindx], sizeof(x64tss[gdtindx]) - 1, DA_386TSS);

        // 初始化 GDT 基址和长度
        x64_igdt_reg[gdtindx].gdtbass = (u64_t)x64_gdt[gdtindx];
        x64_igdt_reg[gdtindx].gdtLen = sizeof(x64_gdt[gdtindx]) - 1;
    }

    // 加载 GDT
    load_x64_gdt(&x64_igdt_reg[0]);
    
    // 加载 TSS（任务状态段）选择符
    load_x64_tr(0x30);//11 0000

    return;
}

// 初始化 IDT（中断描述符表）的函数
PUBLIC LKINIT void init_idt_descriptor()
{
    // 设置所有中断向量（0-255）为默认中断处理程序 hxi_exc_general_intpfault，特权级为内核（PRIVILEGE_KRNL）
    for (u16_t intindx = 0; intindx <= 255; intindx++)
    {
        set_idt_desc((u8_t)intindx, DA_386IGate, hxi_exc_general_intpfault, PRIVILEGE_KRNL);
    }
    //硬中断的特权级一般为0，不起作用，直接看中断门中目标代码段的DPL，若CPL大于DPL，提权
    //软中断要考虑特权级，若CPL大于中断门的DPL则无法调用，抛出异常，这里都是中断门，所以进入中断门都会关中断，后续也没有开中断的过程，既不支持中断嵌套

    /*TO DO*/
    /*后期有时间可以改写为支持嵌套*/
    set_idt_desc(INT_VECTOR_DIVIDE, DA_386IGate, exc_divide_error, PRIVILEGE_KRNL); // 除零错误
    set_idt_desc(INT_VECTOR_DEBUG, DA_386IGate, exc_single_step_exception, PRIVILEGE_KRNL); // 单步调试
    set_idt_desc(INT_VECTOR_NMI, DA_386IGate, exc_nmi, PRIVILEGE_KRNL); // 非屏蔽中断
    set_idt_desc(INT_VECTOR_BREAKPOINT, DA_386IGate, exc_breakpoint_exception, PRIVILEGE_USER); // 调试断点（用户特权级,用户权限可以访问）
    set_idt_desc(INT_VECTOR_OVERFLOW, DA_386IGate, exc_overflow, PRIVILEGE_USER); // 溢出异常（用户特权级）
    set_idt_desc(INT_VECTOR_BOUNDS, DA_386IGate, exc_bounds_check, PRIVILEGE_KRNL); // 边界检查异常
    set_idt_desc(INT_VECTOR_INVAL_OP, DA_386IGate, exc_inval_opcode, PRIVILEGE_KRNL); // 无效操作码
    set_idt_desc(INT_VECTOR_COPROC_NOT, DA_386IGate, exc_copr_not_available, PRIVILEGE_KRNL); // 协处理器不可用
    set_idt_desc(INT_VECTOR_DOUBLE_FAULT, DA_386IGate, exc_double_fault, PRIVILEGE_KRNL); // 双重错误
    set_idt_desc(INT_VECTOR_COPROC_SEG, DA_386IGate, exc_copr_seg_overrun, PRIVILEGE_KRNL); // 协处理器段溢出
    set_idt_desc(INT_VECTOR_INVAL_TSS, DA_386IGate, exc_inval_tss, PRIVILEGE_KRNL); // 无效 TSS
    set_idt_desc(INT_VECTOR_SEG_NOT, DA_386IGate, exc_segment_not_present, PRIVILEGE_KRNL); // 段不存在异常
    set_idt_desc(INT_VECTOR_STACK_FAULT, DA_386IGate, exc_stack_exception, PRIVILEGE_KRNL); // 栈错误
    set_idt_desc(INT_VECTOR_PROTECTION, DA_386IGate, exc_general_protection, PRIVILEGE_KRNL); // 通用保护异常
    set_idt_desc(INT_VECTOR_PAGE_FAULT, DA_386IGate, exc_page_fault, PRIVILEGE_KRNL); // 页错误
    // 保留向量 15，通常未使用
    //set_idt_desc(15,DA_386IGate,hxi_exc_general_intpfault,PRIVILEGE_KRNL);
    set_idt_desc(INT_VECTOR_COPROC_ERR, DA_386IGate, exc_copr_error, PRIVILEGE_KRNL); // 协处理器错误
    set_idt_desc(INT_VECTOR_ALIGN_CHEK, DA_386IGate, exc_alignment_check, PRIVILEGE_KRNL); // 对齐检查
    set_idt_desc(INT_VECTOR_MACHI_CHEK, DA_386IGate, exc_machine_check, PRIVILEGE_KRNL); // 机器检查
    set_idt_desc(INT_VECTOR_SIMD_FAULT, DA_386IGate, exc_simd_fault, PRIVILEGE_KRNL); // SIMD 浮点异常

      // 设置硬件中断向量（IRQ0-IRQ23）
    set_idt_desc(INT_VECTOR_IRQ0 + 0, DA_386IGate, hxi_hwint00, PRIVILEGE_KRNL); 
    set_idt_desc(INT_VECTOR_IRQ0 + 1, DA_386IGate, hxi_hwint01, PRIVILEGE_KRNL); 
    set_idt_desc(INT_VECTOR_IRQ0 + 2, DA_386IGate, hxi_hwint02, PRIVILEGE_KRNL); 
    set_idt_desc(INT_VECTOR_IRQ0 + 3, DA_386IGate, hxi_hwint03, PRIVILEGE_KRNL); 
    set_idt_desc(INT_VECTOR_IRQ0 + 4, DA_386IGate, hxi_hwint04, PRIVILEGE_KRNL); 
    set_idt_desc(INT_VECTOR_IRQ0 + 5, DA_386IGate, hxi_hwint05, PRIVILEGE_KRNL); 
    set_idt_desc(INT_VECTOR_IRQ0 + 6, DA_386IGate, hxi_hwint06, PRIVILEGE_KRNL); 
    set_idt_desc(INT_VECTOR_IRQ0 + 7, DA_386IGate, hxi_hwint07, PRIVILEGE_KRNL); 
    set_idt_desc(INT_VECTOR_IRQ8 + 0, DA_386IGate, hxi_hwint08, PRIVILEGE_KRNL); 
    set_idt_desc(INT_VECTOR_IRQ8 + 1, DA_386IGate, hxi_hwint09, PRIVILEGE_KRNL); 
    set_idt_desc(INT_VECTOR_IRQ8 + 2, DA_386IGate, hxi_hwint10, PRIVILEGE_KRNL);
    set_idt_desc(INT_VECTOR_IRQ8 + 3, DA_386IGate, hxi_hwint11, PRIVILEGE_KRNL);
    set_idt_desc(INT_VECTOR_IRQ8 + 4, DA_386IGate, hxi_hwint12, PRIVILEGE_KRNL);
    set_idt_desc(INT_VECTOR_IRQ8 + 5, DA_386IGate, hxi_hwint13, PRIVILEGE_KRNL);
    set_idt_desc(INT_VECTOR_IRQ8 + 6, DA_386IGate, hxi_hwint14, PRIVILEGE_KRNL);
    set_idt_desc(INT_VECTOR_IRQ8 + 7, DA_386IGate, hxi_hwint15, PRIVILEGE_KRNL);
    set_idt_desc(INT_VECTOR_IRQ8 + 8, DA_386IGate, hxi_hwint16, PRIVILEGE_KRNL);
    set_idt_desc(INT_VECTOR_IRQ8 + 9, DA_386IGate, hxi_hwint17, PRIVILEGE_KRNL);
    set_idt_desc(INT_VECTOR_IRQ8 + 10, DA_386IGate, hxi_hwint18, PRIVILEGE_KRNL);
    set_idt_desc(INT_VECTOR_IRQ8 + 11, DA_386IGate, hxi_hwint19, PRIVILEGE_KRNL);
    set_idt_desc(INT_VECTOR_IRQ8 + 12, DA_386IGate, hxi_hwint20, PRIVILEGE_KRNL);
    set_idt_desc(INT_VECTOR_IRQ8 + 13, DA_386IGate, hxi_hwint21, PRIVILEGE_KRNL);
    set_idt_desc(INT_VECTOR_IRQ8 + 14, DA_386IGate, hxi_hwint22, PRIVILEGE_KRNL);
    set_idt_desc(INT_VECTOR_IRQ8 + 15, DA_386IGate, hxi_hwint23, PRIVILEGE_KRNL);

    // 设置 APIC（高级可编程中断控制器）相关中断向量
    set_idt_desc(INT_VECTOR_APIC_IPI_SCHEDUL, DA_386IGate, hxi_apic_ipi_schedul, PRIVILEGE_KRNL); // APIC 调度中断
    set_idt_desc(INT_VECTOR_APIC_SVR, DA_386IGate, hxi_apic_svr, PRIVILEGE_KRNL); // APIC 服务中断
    set_idt_desc(INT_VECTOR_APIC_TIMER, DA_386IGate, hxi_apic_timer, PRIVILEGE_KRNL); // APIC 定时器中断
    set_idt_desc(INT_VECTOR_APIC_THERMAL, DA_386IGate, hxi_apic_thermal, PRIVILEGE_KRNL); // APIC 温度传感器中断
    set_idt_desc(INT_VECTOR_APIC_PERFORM, DA_386IGate, hxi_apic_performonitor, PRIVILEGE_KRNL); // 性能监视中断
    set_idt_desc(INT_VECTOR_APIC_LINTO, DA_386IGate, hxi_apic_lint0, PRIVILEGE_KRNL); // 本地中断 0
    set_idt_desc(INT_VECTOR_APIC_LINTI, DA_386IGate, hxi_apic_lint1, PRIVILEGE_KRNL); // 本地中断 1
    set_idt_desc(INT_VECTOR_APIC_ERROR, DA_386IGate, hxi_apic_error, PRIVILEGE_KRNL); // APIC 错误中断

    // 设置系统调用中断向量（用户级特权）
    set_idt_desc(INT_VECTOR_SYSCALL, DA_386IGate, exi_sys_call, PRIVILEGE_USER);

    // 设置并加载 IDT
    set_iidtr(x64_idt);             // 初始化 IDT 寄存器结构
    load_x64_idt(&x64_iidt_reg);    // 加载 IDT 到 CPU
    return;
}

