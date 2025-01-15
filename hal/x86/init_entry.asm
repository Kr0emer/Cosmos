%define MBSP_ADR 0x100000          ; 定义多重引导信息（Multiboot Information）的地址为0x100000
%define IA32_EFER 0C0000080H       ; 定义 IA32_EFER MSR 的地址（Model-Specific Register）
%define PML4T_BADR 0x1000000       ; 定义页表的 PML4 表物理地址
%define KRLVIRADR 0xffff800000000000 ; 定义内核虚拟地址基址
%define KINITSTACK_OFF 16          ; 定义内核初始化栈的偏移量
global _start                      ; 定义全局符号 _start，为程序入口
global x64_GDT                     ; 定义全局符号 x64_GDT
extern hal_start                   ; 声明外部符号 hal_start，表示内核主函数

[section .start.text]              ; 定义代码段
[BITS 32]                          ; 指定当前代码是 32 位模式
_start:
    cli                            ; 关闭中断
    mov ax,0x10                    ; 加载数据段选择子 0x10（对应内核数据段）
    mov ds,ax                      ; 设置数据段寄存器 DS
    mov es,ax                      ; 设置附加段寄存器 ES
    mov ss,ax                      ; 设置堆栈段寄存器 SS
    mov fs,ax                      ; 设置 FS 段寄存器
    mov gs,ax                      ; 设置 GS 段寄存器
    lgdt [eGdtPtr]                 ; 加载全局描述符表（GDT），eGdtPtr 包含 GDT 地址和界限

; 开启 PAE（Physical Address Extension）
    mov eax, cr4                   ; 读取 CR4 寄存器
    bts eax, 5                     ; 设置 CR4.PAE（启用 PAE）
    mov cr4, eax                   ; 写入更新后的 CR4

    mov eax, PML4T_BADR            ; 加载 PML4 表的物理地址
    mov cr3, eax                   ; 加载 PML4 表地址到 CR3（页目录基址寄存器）

; 开启 64 位长模式（Long Mode）
    mov ecx, IA32_EFER             ; 加载 IA32_EFER MSR 地址到 ECX
    rdmsr                          ; 读取 IA32_EFER 的值到 EAX（低 32 位）和 EDX（高 32 位）
    bts eax, 8                     ; 设置 IA32_EFER.LME 位（启用长模式）
    wrmsr                          ; 写入修改后的值到 IA32_EFER MSR

; 开启 PE（保护模式）和分页（Paging）
    mov eax, cr0                   ; 读取 CR0 寄存器
    bts eax, 0                     ; 设置 CR0.PE（启用保护模式）
    bts eax, 31                    ; 设置 CR0.PG（启用分页）
; 开启 CACHE
    btr eax, 29                    ; 清除 CR0.NW（启用写回缓存）
    btr eax, 30                    ; 清除 CR0.CD（启用缓存）
    mov cr0, eax                   ; 写入更新后的 CR0 寄存器
    jmp 08:entry64                 ; 跳转到 64 位代码段（段选择子 0x08，入口地址为 entry64）

[BITS 64]                          ; 指定当前代码是 64 位模式
entry64:
    mov ax,0x10                    ; 加载数据段选择子 0x10（对应内核数据段）
    mov ds,ax                      ; 设置数据段寄存器 DS
    mov es,ax                      ; 设置附加段寄存器 ES
    mov ss,ax                      ; 设置堆栈段寄存器 SS
    mov fs,ax                      ; 设置 FS 段寄存器
    mov gs,ax                      ; 设置 GS 段寄存器
    xor rax,rax                    ; 清零 RAX 寄存器
    xor rbx,rbx                    ; 清零 RBX 寄存器
    xor rbp,rbp                    ; 清零 RBP 寄存器
    xor rcx,rcx                    ; 清零 RCX 寄存器
    xor rdx,rdx                    ; 清零 RDX 寄存器
    xor rdi,rdi                    ; 清零 RDI 寄存器
    xor rsi,rsi                    ; 清零 RSI 寄存器
    xor r8,r8                      ; 清零 R8 寄存器
    xor r9,r9                      ; 清零 R9 寄存器
    xor r10,r10                    ; 清零 R10 寄存器
    xor r11,r11                    ; 清零 R11 寄存器
    xor r12,r12                    ; 清零 R12 寄存器
    xor r13,r13                    ; 清零 R13 寄存器
    xor r14,r14                    ; 清零 R14 寄存器
    xor r15,r15                    ; 清零 R15 寄存器

    mov rbx,MBSP_ADR               ; 加载多重引导信息的地址到 RBX
    mov rax,KRLVIRADR              ; 加载内核虚拟地址基址到 RAX
    mov rcx,[rbx+KINITSTACK_OFF]   ; 从多重引导信息中读取内核栈偏移量到 RCX
    add rax,rcx                    ; 计算完整的内核栈地址
    xor rcx,rcx                    ; 清零 RCX
    xor rbx,rbx                    ; 清零 RBX
    mov rsp,rax                    ; 设置 RSP 为内核栈地址
    push 0                         ; 压入 0，表示 RFLAGS
    push 0x8                       ; 压入段选择子 0x08（内核代码段）
    mov rax,hal_start              ; 加载内核主函数的地址到 RAX
    push rax                       ; 压入内核主函数的地址
    dw 0xcb48                      ; 执行 iret（中断返回）以跳转到内核主函数
    jmp $                          ; 死循环，防止代码意外运行

[section .start.data]              ; 定义数据段
[BITS 32]                          ; 指定数据是 32 位模式
ex64_GDT:
enull_x64_dsc: dq 0                ; 空描述符（GDT 的第一个条目）
ekrnl_c64_dsc: dq 0x0020980000000000 ; 内核代码段描述符
ekrnl_d64_dsc: dq 0x0000920000000000 ; 内核数据段描述符
euser_c64_dsc: dq 0x0020f80000000000 ; 用户代码段描述符
euser_d64_dsc: dq 0x0000f20000000000 ; 用户数据段描述符
eGdtLen         equ $ - enull_x64_dsc ; GDT 长度
eGdtPtr:        dw eGdtLen - 1       ; GDT 界限（长度 - 1）
                dq ex64_GDT          ; GDT 基址
