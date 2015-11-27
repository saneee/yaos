/* zhuhanjun 2015/11/23
cpupm struct
*/
#ifndef ARCH_X86_64_CPUPM_H
#define ARCH_X86_64_CPUPM_H 1
#include <yaos/types.h>
#include <yaos/string.h>
#include <yaos/kheap.h>
#include <yaos/printk.h>
#include <asm/cpu.h>
#include <asm/percpu.h>
#include <yaos/rett.h>
#define INIT_STACK_SIZE 4096
#define EXCEPTION_STACK_SIZE 4096
#define INTERRUPT_STACK_SIZE 4096
#define NR_EXCEPTION_NEST 3
#define CPU_RUNNING 1
extern ulong __kernel_end;
enum {
    gdt_null = 0,
    gdt_cs = 1,
    gdt_ds = 2,
    gdt_cs32 =3,
    gdt_tss =4,
    gdt_tssx =5,
    nr_gdt
};
struct task_state_segment {
    u32 reserved0;
    u64 rsp[3];
    u64 ist[8];   // ist[0] is reserved
    u32 reserved1;
    u32 reserved2;
    u16 reserved3;
    u16 io_bitmap_base;
} __attribute__((packed));

struct aligned_task_state_segment {
    u32 pad;  // force 64-bit structures to be aligned
    struct task_state_segment tss;
} __attribute__((packed, aligned(8)));

struct cpu_stack;
struct cpu_stack {
    char init_stack[INIT_STACK_SIZE] __attribute__((aligned(16)));
    char interrupt_stack[INTERRUPT_STACK_SIZE] __attribute__((aligned(16)));
    char exception_stack[EXCEPTION_STACK_SIZE*NR_EXCEPTION_NEST] __attribute__((aligned(16)));
} __attribute__((packed));

struct thread_t;
struct arch_cpu{
    struct aligned_task_state_segment atss;
    struct cpu_stack stack;

    u32 apic_id;
    u32 acpi_id;
    u64 status;
    u64 rsp;
    u64 gdt[nr_gdt];

};
static inline void set_ist_entry(struct arch_cpu *p,unsigned ist,char *base,size_t size)
{
    p->atss.tss.ist[ist]=(u64)(base+size);	
}
static inline char* get_ist_entry(struct arch_cpu *p,unsigned ist)
{
    return (char *)(p->atss.tss.ist[ist]);
}
static inline void set_exception_stack(struct arch_cpu *p)
{
    char *base = p->stack.exception_stack;
    set_ist_entry(p,1, base, EXCEPTION_STACK_SIZE*NR_EXCEPTION_NEST);
}
static inline void set_interrupt_stack(struct arch_cpu *p)
{
    char *base = p->stack.interrupt_stack;
    set_ist_entry(p,2, base, INTERRUPT_STACK_SIZE);
}
static inline struct arch_cpu *get_current_arch_cpu()
{
    ulong base=_get_cur_percpu_base(); 
    return (struct arch_cpu *)base;
}
static inline bool arch_is_bp()
{
   struct arch_cpu *p=get_current_arch_cpu();
   return p->apic_id==0; 
}
struct thread_t;
void add_int_vector(unsigned n,unsigned ist,void *handler);
extern void bootup_aps(void);//pm64.c
static inline void arch_start_aps(void)
{
   bootup_aps(); 
}
#endif
