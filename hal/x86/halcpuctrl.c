#include "cosmostypes.h"
#include "cosmosmctrl.h"

void hal_spinlock_init(spinlock_t *lock)
{
    lock->lock = 0;
    return;
}

void hal_spinlock_lock(spinlock_t *lock)
{
    __asm__ __volatile__(
        "1:         \n"
        "lock; xchg  %0, %1 \n"
        "cmpl   $0, %0      \n"
        "jnz    2f      \n"
        ".section .spinlock.text,"
        "\"ax\""
        "\n"                    // 重新定义一个代码段所以jnz 2f下面并不是
        "2:         \n"         //cmpl $0,%1 事实上下面的代码不会常常执行,
        "cmpl   $0, %1      \n" //这是为了不在cpu指令高速缓存中填充无用代码
        "jne    2b      \n"     //要知道那可是用每位6颗晶体管做的双极性静态
        "jmp    1b      \n"     //储存器,比内存条快几千个数量级
        ".previous      \n"
        :
        : "r"(1), "m"(*lock));
    return;
}

void hal_spinlock_unlock(spinlock_t *lock)
{
    __asm__ __volatile__(
        "movl   $0, %0\n"
        :
        : "m"(*lock));
    return;
}

void hal_spinlock_saveflg_cli(spinlock_t *lock, cpuflg_t *cpuflg)
{
    __asm__ __volatile__(
        "pushfq             \n\t"//压入标志寄存器
        "cli                \n\t"//关中断
        "popq %0            \n\t"//将之前保存的标志寄存器值（中断状态）弹出到参数cpuflg中
        // 自旋锁获取操作
        "1:                 \n\t"
        "lock; xchg  %1, %2 \n\t"// 使用xchg指令进行原子交换，尝试获取锁（%2是锁，%1是值1）
        "cmpl   $0,%1       \n\t"// 比较锁的值是否为0，如果是0表示锁未被占用
        "jnz    2f          \n\t"// 如果锁已被占用（%1 != 0），跳转到2f处
        ".section .spinlock.text,"
        "\"ax\""
        "\n\t"                    //重新定义一个代码段所以jnz 2f下面并不是
        "2:                 \n\t" //cmpl $0,%1 事实上下面的代码不会常常执行,
        "cmpl   $0,%2       \n\t" //这是为了不在cpu指令高速缓存中填充无用代码
        "jne    2b          \n\t"// 如果锁值不是0，跳回2b，继续检查
        "jmp    1b          \n\t"// 如果锁值为0，跳回1b，重新尝试获取锁
        ".previous          \n\t"
        : "=m"(*cpuflg)
        : "r"(1), "m"(*lock));
    return;
}

void hal_spinunlock_restflg_sti(spinlock_t *lock, cpuflg_t *cpuflg)
{
    __asm__ __volatile__(
        "movl   $0, %0\n\t"// 将锁（%0）设置为0，表示释放锁（锁的值设为0）
        "pushq %1 \n\t"// 将之前保存的中断标志（%1）压入栈中
        "popfq \n\t"// 将栈中的值恢复到标志寄存器，恢复中断状态（中断开启或关闭）
        :
        : "m"(*lock), "m"(*cpuflg));// 输入操作数：锁的地址和保存的中断标志寄存器状态
    return;
}

void knl_spinlock_init(spinlock_t *lock)
{
    lock->lock = 0;
    return;
}

void knl_spinlock_lock(spinlock_t *lock)
{
    __asm__ __volatile__(
        "1:         \n"
        "lock; xchg  %0, %1 \n"
        "cmpl   $0, %0      \n"
        "jnz    2f      \n"
        ".section .spinlock.text,"
        "\"ax\""
        "\n"                    // 重新定义一个代码段所以jnz 2f下面并不是
        "2:         \n"         //cmpl $0,%1 事实上下面的代码不会常常执行,
        "cmpl   $0, %1      \n" //这是为了不在cpu指令高速缓存中填充无用代码
        "jne    2b      \n"     //要知道那可是用每位6颗晶体管做的双极性静态
        "jmp    1b      \n"     //储存器,比内存条快几千个数量级
        ".previous      \n"
        :
        : "r"(1), "m"(*lock));
    return;
}

void knl_spinlock_unlock(spinlock_t *lock)
{
    __asm__ __volatile__(
        "movl   $0, %0\n"
        :
        : "m"(*lock));
    return;
}

void knl_spinlock_cli(spinlock_t *lock, cpuflg_t *cpuflg)
{
    __asm__ __volatile__(
        "pushfq             \n\t"
        "cli                \n\t"
        "popq %0            \n\t"

        "1:                 \n\t"
        "lock; xchg  %1, %2 \n\t"
        "cmpl   $0,%1       \n\t"
        "jnz    2f          \n\t"
        ".section .spinlock.text,"
        "\"ax\""
        "\n\t"                    //重新定义一个代码段所以jnz 2f下面并不是
        "2:                 \n\t" //cmpl $0,%1 事实上下面的代码不会常常执行,
        "cmpl   $0,%2       \n\t" //这是为了不在cpu指令高速缓存中填充无用代码
        "jne    2b          \n\t"
        "jmp    1b          \n\t"
        ".previous          \n\t"
        : "=m"(*cpuflg)
        : "r"(1), "m"(*lock));
    return;
}

void knl_spinunlock_sti(spinlock_t *lock, cpuflg_t *cpuflg)
{
    __asm__ __volatile__(
        "movl   $0, %0\n\t"
        "pushq %1 \n\t"
        "popfq \n\t"
        :
        : "m"(*lock), "m"(*cpuflg));
    return;
}

void hal_memset(void *setp, size_t n, u8_t setval)
{
    u8_t *_p = (u8_t *)setp;
    for (uint_t i = 0; i < n; i++)
    {
        _p[i] = setval;
    }
    return;
}

void hal_memcpy(void *src, void *dst, size_t n)
{
    u8_t *_s = (u8_t *)src, *_d = (u8_t *)dst;
    for (uint_t i = 0; i < n; i++)
    {
        _d[i] = _s[i];
    }
    return;
}




void hal_sysdie(char_t *errmsg)
{
    kprint("COSMOS SYSTEM IS DIE %s", errmsg);
    for (;;)
        ;
    return;
}

void system_error(char_t *errmsg)
{
    hal_sysdie(errmsg);
    return;
}

uint_t hal_retn_cpuid()
{
    return 0;
}