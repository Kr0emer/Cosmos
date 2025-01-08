%include "ldrasm.inc"
global _start
global realadr_call_entry
global IDT_PTR
extern ldrkrl_entry
[section .text]
[bits 32]
_start:
_entry:
	cli
	lgdt [GDT_PTR];加载GDTR寄存器，GDT 是一个数据结构，包含了各种段描述符，例如代码段、数据段、系统段等。每个描述符定义了一个内存段的属性，如基地址、段限制、特权级等。
	;+-----------------+------------------+
	;|   Base Address  (32 bits)         |
	;+-----------------+------------------+
	;|   Limit         (16 bits)         |
	;+-----------------+------------------+

	lidt [IDT_PTR];加载IDTR寄存器，IDT 是一个数据结构，包含了中断和异常处理程序的入口地址。每个中断或异常对应一个描述符，其中包含了处理程序的地址和特权级。
	;+-----------------+------------------+
	;|   Base Address  (32 bits)         |
	;+-----------------+------------------+
	;|   Limit         (16 bits)         |
	;+-----------------+------------------+
	jmp dword 0x8 :_32bits_mode ;0000 1000段选择子为1，32位代码段描述符

_32bits_mode:
	mov ax, 0x10	; 数据段选择子(目的)，0000 0000 0001 0000，索引为2，32位数据段描述符
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
	mov esp,0x90000
	call ldrkrl_entry
	xor ebx,ebx
	jmp 0x2000000
	jmp $


realadr_call_entry:
	pushad;将所有 32 位的通用寄存器（EAX、ECX、EDX、EBX、ESP、EBP、ESI、EDI）的值按顺序压入堆栈。
	push    ds
	push    es
	push    fs
	push    gs
	call save_eip_jmp;push eip , jmp save_eip_jmp
	pop		gs
	pop		fs
	pop		es
	pop		ds
	popad
	ret
save_eip_jmp:
	pop esi ;此时栈底是储存的是call指令执行时保存的eip既"pop gs"这条指令的地址
	mov [PM32_EIP_OFF],esi;保存eip
	mov [PM32_ESP_OFF],esp;保存esp
	jmp dword far [cpmty_mode] ;长跳转这里表示把cpmty_mode处的第一个4字节装入eip，把其后的2字节装入cs，跳转到的代码文件在realintsve.asm
cpmty_mode:
	dd 0x1000;EIP
	dw 0x18;CS 0000 0000 0001 1000,最后三位是RPL和TI，都为0 ，索引是3，;16位代码段描述符，访问位 a-e
	jmp $

GDT_START:
knull_dsc: dq 0;一个空描述符，用于保护模式下的错误检查。
kcode_dsc: dq 0x00cf9a000000ffff ;32位代码段描述符，访问位 a-e
kdata_dsc: dq 0x00cf92000000ffff ;32位数据段描述符。
k16cd_dsc: dq 0x00009a000000ffff ;16位代码段描述符，访问位 a-e
k16da_dsc: dq 0x000092000000ffff ;16位数据段描述符
GDT_END:

GDT_PTR:
GDTLEN	dw GDT_END-GDT_START-1	;GDT界限
GDTBASE	dd GDT_START ;GDT 的基地址

IDT_PTR:
IDTLEN	dw 0x3ff; IDT 的长度，这里设置为 0x3ff，即 1023。
IDTBAS	dd 0; IDT 的基地址，这里设置为 0，这是BIOS中断表的地址
