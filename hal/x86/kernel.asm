; 包含内核相关的定义文件
%include "kernel.inc"

; 定义代码段，存放硬件中断和异常处理程序
[section .hwint.text]

; 指定64位模式
[BITS 64]

; 对齐到16字节，提高性能和符合硬件要求
ALIGN 16
; 除法错误异常处理程序入口
exc_divide_error:
	SRFTFAULT 0 ; 使用宏SRFTFAULT处理异常号0

ALIGN 16
; 单步异常处理程序入口
exc_single_step_exception:
	SRFTFAULT 1 ; 使用宏SRFTFAULT处理异常号1

ALIGN 16
; 非屏蔽中断（NMI）处理程序入口
exc_nmi:
	SRFTFAULT 2 ; 使用宏SRFTFAULT处理异常号2

ALIGN 16
; 断点异常处理程序入口
exc_breakpoint_exception:
	SRFTFAULT 3 ; 使用宏SRFTFAULT处理异常号3

ALIGN 16
; 溢出异常处理程序入口
exc_overflow:
	SRFTFAULT 4 ; 使用宏SRFTFAULT处理异常号4

ALIGN 16
; 边界检查异常处理程序入口
exc_bounds_check:
	SRFTFAULT 5 ; 使用宏SRFTFAULT处理异常号5

ALIGN 16
; 无效操作码异常处理程序入口
exc_inval_opcode:
	SRFTFAULT 6 ; 使用宏SRFTFAULT处理异常号6

ALIGN 16
; 协处理器不可用异常处理程序入口
exc_copr_not_available:
	SRFTFAULT 7 ; 使用宏SRFTFAULT处理异常号7

ALIGN 16
; 双重故障异常处理程序入口，带错误代码
exc_double_fault:
	SRFTFAULT_ECODE 8 ; 使用宏SRFTFAULT_ECODE处理异常号8

ALIGN 16
; 协处理器段溢出异常处理程序入口
exc_copr_seg_overrun:
	SRFTFAULT 9 ; 使用宏SRFTFAULT处理异常号9

ALIGN 16
; 无效TSS异常处理程序入口，带错误代码
exc_inval_tss:
	SRFTFAULT_ECODE 10 ; 使用宏SRFTFAULT_ECODE处理异常号10

ALIGN 16
; 段不存在异常处理程序入口，带错误代码
exc_segment_not_present:
	SRFTFAULT_ECODE 11 ; 使用宏SRFTFAULT_ECODE处理异常号11

ALIGN 16
; 堆栈异常处理程序入口，带错误代码
exc_stack_exception:
	SRFTFAULT_ECODE 12 ; 使用宏SRFTFAULT_ECODE处理异常号12

ALIGN 16
; 通用保护异常处理程序入口，带错误代码
exc_general_protection:
	SRFTFAULT_ECODE 13 ; 使用宏SRFTFAULT_ECODE处理异常号13

ALIGN 16
; 页错误异常处理程序入口，带错误代码
exc_page_fault:
	SRFTFAULT_ECODE 14 ; 使用宏SRFTFAULT_ECODE处理异常号14

ALIGN 16
; 协处理器错误异常处理程序入口
exc_copr_error:
	SRFTFAULT 16 ; 使用宏SRFTFAULT处理异常号16

ALIGN 16
; 对齐检查异常处理程序入口，带错误代码
exc_alignment_check:
	SRFTFAULT_ECODE 17 ; 使用宏SRFTFAULT_ECODE处理异常号17

ALIGN 16
; 机器检查异常处理程序入口
exc_machine_check:
	SRFTFAULT 18 ; 使用宏SRFTFAULT处理异常号18

ALIGN 16
; SIMD浮点异常处理程序入口
exc_simd_fault:
	SRFTFAULT 19 ; 使用宏SRFTFAULT处理异常号19

ALIGN 16
; 通用中断错误处理程序入口
hxi_exc_general_intpfault:
	SRFTFAULT 256 ; 使用宏SRFTFAULT处理异常号256

; 硬件中断处理程序
ALIGN 16
hxi_hwint00:
	HARWINT (INT_VECTOR_IRQ0+0) ; 处理第0号硬件中断
ALIGN 16
hxi_hwint01:
	HARWINT (INT_VECTOR_IRQ0+1) ; 处理第1号硬件中断
ALIGN 16
hxi_hwint02:
	HARWINT (INT_VECTOR_IRQ0+2) ; 处理第2号硬件中断
ALIGN 16
hxi_hwint03:
	HARWINT (INT_VECTOR_IRQ0+3) ; 处理第3号硬件中断
ALIGN 16
hxi_hwint04:
	HARWINT (INT_VECTOR_IRQ0+4) ; 处理第4号硬件中断
ALIGN 16
hxi_hwint05:
	HARWINT (INT_VECTOR_IRQ0+5) ; 处理第5号硬件中断
ALIGN 16
hxi_hwint06:
	HARWINT (INT_VECTOR_IRQ0+6) ; 处理第6号硬件中断
ALIGN 16
hxi_hwint07:
	HARWINT (INT_VECTOR_IRQ0+7) ; 处理第7号硬件中断
ALIGN 16
hxi_hwint08:
	HARWINT (INT_VECTOR_IRQ0+8) ; 处理第8号硬件中断
ALIGN 16
hxi_hwint09:
	HARWINT (INT_VECTOR_IRQ0+9) ; 处理第9号硬件中断
ALIGN 16
hxi_hwint10:
	HARWINT (INT_VECTOR_IRQ0+10) ; 处理第10号硬件中断
ALIGN 16
hxi_hwint11:
	HARWINT (INT_VECTOR_IRQ0+11) ; 处理第11号硬件中断
ALIGN 16
hxi_hwint12:
	HARWINT (INT_VECTOR_IRQ0+12) ; 处理第12号硬件中断
ALIGN 16
hxi_hwint13:
	HARWINT (INT_VECTOR_IRQ0+13) ; 处理第13号硬件中断
ALIGN 16
hxi_hwint14:
	HARWINT (INT_VECTOR_IRQ0+14) ; 处理第14号硬件中断
ALIGN 16
hxi_hwint15:
	HARWINT (INT_VECTOR_IRQ0+15) ; 处理第15号硬件中断
ALIGN 16
hxi_hwint16:
	HARWINT (INT_VECTOR_IRQ0+16) ; 处理第16号硬件中断
ALIGN 16
hxi_hwint17:
	HARWINT (INT_VECTOR_IRQ0+17) ; 处理第17号硬件中断
ALIGN 16
hxi_hwint18:
	HARWINT (INT_VECTOR_IRQ0+18) ; 处理第18号硬件中断
ALIGN 16
hxi_hwint19:
	HARWINT (INT_VECTOR_IRQ0+19) ; 处理第19号硬件中断
ALIGN 16
hxi_hwint20:
	HARWINT (INT_VECTOR_IRQ0+20) ; 处理第20号硬件中断
ALIGN 16
hxi_hwint21:
	HARWINT (INT_VECTOR_IRQ0+21) ; 处理第21号硬件中断
ALIGN 16
hxi_hwint22:
	HARWINT (INT_VECTOR_IRQ0+22) ; 处理第22号硬件中断
ALIGN 16
hxi_hwint23:
	HARWINT (INT_VECTOR_IRQ0+23) ; 处理第23号硬件中断

; APIC相关中断
ALIGN 16
hxi_apic_ipi_schedul:
	HARWINT INT_VECTOR_APIC_IPI_SCHEDUL ; 处理APIC IPI调度中断
ALIGN 16
hxi_apic_svr:
	HARWINT INT_VECTOR_APIC_SVR ; 处理APIC服务中断
ALIGN 16
hxi_apic_timer:
	HARWINT INT_VECTOR_APIC_TIMER ; 处理APIC定时器中断
ALIGN 16
hxi_apic_thermal:
	HARWINT INT_VECTOR_APIC_THERMAL ; 处理APIC温度中断
ALIGN 16
hxi_apic_performonitor:
	HARWINT INT_VECTOR_APIC_PERFORM ; 处理APIC性能监控中断
ALIGN 16
hxi_apic_lint0:
	HARWINT INT_VECTOR_APIC_LINTO ; 处理APIC LINT0中断
ALIGN 16
hxi_apic_lint1:
	HARWINT INT_VECTOR_APIC_LINTI ; 处理APIC LINT1中断
ALIGN 16
hxi_apic_error:
	HARWINT INT_VECTOR_APIC_ERROR ; 处理APIC错误中断

; 系统调用和用户模式返回
ALIGN 16
exi_sys_call:
	EXI_SCALL ; 系统调用入口
ALIGN 16
asm_ret_from_user_mode:
	RESTOREALL ; 从用户模式返回