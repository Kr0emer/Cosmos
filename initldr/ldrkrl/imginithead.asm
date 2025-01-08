MBT_HDR_FLAGS	EQU 0x00010003;flag字段，指出OS映像需要引导程序提供或支持的特性
MBT_HDR_MAGIC	EQU 0x1BADB002;多引导协议头魔数
MBT2_MAGIC	EQU 0xe85250d6;第二版多引导协议头魔数
global _start;导出_start符号
extern inithead_entry;导入外部的inithead_entry函数符号
[section .text];定义.text代码节
[bits 32];汇编成32位代码
_start:
	jmp _entry
align 4
mbt_hdr:
	DD MBT_HDR_MAGIC
	DD MBT_HDR_FLAGS
	DD -(MBT_HDR_MAGIC+MBT_HDR_FLAGS)
	DD mbt_hdr
	DD _start
	DD 0
	DD 0
	DD _entry
	;以上是GRUB所需要的头
ALIGN 8
mbhdr:
	DD	0xE85250D6
	DD	0
	DD	mhdrend - mbhdr
	DD	-(0xE85250D6 + 0 + (mhdrend - mbhdr))
	DW	2, 0
	DD	24
	DD	mbhdr
	DD	_start
	DD	0
	DD	0
	DW	3, 0
	DD	12
	DD	_entry 
	DD      0  
	DW	0, 0
	DD	8
mhdrend:
;以上是GRUB2所需要的头

_entry:
    ;关中断
	cli
    ;关不可屏蔽中断,通过设置CMOS的最高位来实现
	in al, 0x70
	or al, 0x80	
	out 0x70,al
    ;重新加载GDT
	lgdt [GDT_PTR]
	jmp dword 0x8 :_32bits_mode

_32bits_mode:
;下面初始化C语言可能会用到的寄存器
	mov ax, 0x10
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	xor eax,eax
	xor ebx,ebx
	xor ecx,ecx
	xor edx,edx
	xor edi,edi
	xor esi,esi
	xor ebp,ebp
	xor esp,esp
    ;初始化栈，C语言需要栈才能工作
	mov esp,0x7c00
    ;调用C语言函数inithead_entry
	call inithead_entry
	jmp 0x200000;inithead_entry在inithead_entry导入的initldrkrl.bin


GDT_START:
knull_dsc: dq 0
kcode_dsc: dq 0x00cf9e000000ffff
kdata_dsc: dq 0x00cf92000000ffff
k16cd_dsc: dq 0x00009e000000ffff
k16da_dsc: dq 0x000092000000ffff
GDT_END:
GDT_PTR:
GDTLEN	dw GDT_END-GDT_START-1	;GDT界限
GDTBASE	dd GDT_START
