#ifndef _ASM_X86_PERCPU_H
#define _ASM_X86_PERCPU_H
#define ARCH_GET_PERCPU_BASE
static inline unsigned long _get_cur_percpu_base()
{
    ulong r;
    asm volatile ("rdgsbase %0" : "=r"(r));
    return r;    
}
static inline unsigned long _get_percpu_base_rdmsr()
{
    u32 lo, hi;
    u32 index=0xc0000101;
    asm volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(index));
    return lo | ((u64)hi << 32);
}
#endif
