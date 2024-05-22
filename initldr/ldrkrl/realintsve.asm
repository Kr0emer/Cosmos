%include "ldrasm.inc"
global _start
[section .text]
[bits 16]
_start:
_16_mode:
    mov bp, 0x20 ;0000 0000 0010 0000，索引为4，16位数据段描述符
    mov ds, bp
    mov es, bp
    mov ss, bp
    mov ebp, cr0
    and ebp, 0xfffffffe
    mov cr0, ebp ;CR0.P=0 关闭保护模式
    jmp 0:real_entry ;刷新CS影子寄存器，真正进入实模式
real_entry:
    mov bp,cs
    mov ds, bp
    mov es, bp
    mov ss, bp
    mov sp, 08000h ;设置栈顶
    mov bp, func_table
    add bp, ax
    call [bp] ;调用函数表中的汇编函数，ax是C函数(使用realadr_call_entry符号传递参数)中传递进来的，根据不同的eax来控制调用不同函数
    cli
    call disable_nmi
    mov ebp, cr0
    or ebp, 1
    mov cr0, ebp ;CR0.P=1 开启保护模式
    jmp dword 0x8 : _32bits_mode


[bits 32]
_32bits_mode:
    mov bp, 0x10;0000 0000 0001 0000，索引为2，32位数据段描述符
    mov ds, bp
    mov ss, bp ;重新设置保护模式下的段寄存器为32位数据段描述符
	mov esi,[PM32_EIP_OFF];加载先前保存的EIP
	mov esp,[PM32_ESP_OFF];加载先前保存的ESP
	jmp esi;eip=esi 回到了realadr_call_entry函数中

_getmmap:
    push ds
    push es
    push ss
    mov esi,0
    mov dword[E80MAP_NR],esi//E80MAP_NR设为0
    mov dword[E80MAP_ADRADR],E80MAP_ADR;e820map结构体开始地址
    xor ebx,ebx //ebx设置为0
    mov edi,E80MAP_ADR//将E80MAP_ADR传递给edi,存储e820map结构的初始位置
loop:
    mov eax,0e820h;获取e820map结构参数
	mov ecx,20;;输出结果数据项的大小为20字节：8字节内存基地址，8字节内存长度，4字节内存类型
	mov edx,0534d4150h;获取e820map结构参数必须是这个数据
	int 15h;BIOS的15h中断
	jc .1;如果进位标志被设置（表示出现错误），跳转到标签 .1
	add edi,20;将 edi 的值增加 20（移动到下一个内存位置）
	cmp edi,E80MAP_ADR+0x1000;将 edi 与 E80MAP_ADR + 0x1000 进行比较
	jg .1;如果 edi 大于 E80MAP_ADR + 0x1000，跳转到标签 .1
	inc esi;增加 esi 的值（循环计数器）
	cmp ebx,0;将 ebx 与零进行比较
	jne loop;如果 ebx 不等于零，则重复循环
	jmp .2

.1:
	mov esi,0;将 esi 的值重置为零

.2:
	mov dword[E80MAP_NR],esi;将最终的 esi 值存储到由 E80MAP_NR 指向的内存位置
	pop ss
	pop es
	pop ds
	ret    

func_table: ;函数表，每项占两个字节
	dw _getmmap;获取内存布局视图的函数
	dw _read ;读取硬盘的函数
    dw _getvbemode ;获取显卡VBE模式
    dw _getvbeonemodeinfo ;获取显卡VBE模式的数据
    dw _setvbemode ;设置显卡VBE模式

disable_nmi:
	push ax
	in al, 0x70 ; port 0x70NMI_EN_PORT
	or al, 0x80	; disable all NMI source
	out 0x70,al	; NMI_EN_PORT
	pop ax
	ret