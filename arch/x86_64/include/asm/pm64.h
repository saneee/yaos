/* zhuhanjun 2015/10/29 
per cpu struct
*/
#ifndef ARCH_X86_64_PM64_H
#define ARCH_X86_64_PM64_H 1
#include <yaos/types.h>
#include <yaos/string.h>
#include <yaos/kheap.h>
#include <yaos/printk.h>
#include <asm/cpu.h>
#include <asm/percpu.h>
#include <yaos/thread.h>
#include <yaos/cpupm.h>
struct trapframe {
  u64 rax;      
  u64 rbx;
  u64 rcx;
  u64 rdx;
  u64 rbp;
  u64 rsi;
  u64 rdi;
  u64 r8;
  u64 r9;
  u64 r10;
  u64 r11;
  u64 r12;
  u64 r13;
  u64 r14;
  u64 r15;

  u64 trapno;
  u64 err;

  u64 rip;     // rip
  u64 cs;
  u64 eflags;  // rflags
  u64 rsp;     // rsp
  u64 ds;      // ss
};
extern ret_t switch_to(ulong stack, ulong * poldstack, ulong arg);
extern ret_t switch_first(struct thread_t *, void *, ulong, ulong *, ulong);  

static inline ret_t arch_switch_first(pthread p,void *thread_main,pthread old,ulong arg)
{
    __thread ulong stack_point;
    stack_point = p->stack_addr + p->stack_size - 8;
    return switch_first(p, thread_main, stack_point, &old->rsp, arg);
}
static inline ret_t arch_switch_to(pthread p,pthread old,ulong arg)
{
    return switch_to(p->rsp,&old->rsp,arg);
}
static inline bool is_bp_cpu(cpu_p cpu)
{
    return (cpu->arch_cpu.apic_id==0);
}


#endif
