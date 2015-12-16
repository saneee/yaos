#ifndef __YAOS_PERCPU_H
#define __YAOS_PERCPU_H
#include <string.h>
#include <asm/percpu.h>
#include <yaos/kheap.h>
#include <yaos/compiler.h>
#include <yaos/printk.h>
#include <yaos/percpu_defs.h>
#ifndef PERCPU
#define PERCPU(type, var) type var __attribute__((section(".data..percpu")))
#endif
#ifndef __percpu_data
#define __percpu_data __attribute__((section(".data..percpu")))
#endif

#define per_cpu_offset(x) ((unsigned long)cpu_base[x])

extern char __per_cpu_start[];
extern char __per_cpu_end[];
extern char __per_cpu_load[];
extern struct cpu the_cpu;
extern char *cpu_base[];
extern int nr_cpu;
DECLARE_PER_CPU(int, cpu_number);
static inline char *new_percpu()
{
    char *p = kalloc((__per_cpu_end - __per_cpu_start));

    memcpy(p, (char *)__per_cpu_load, (__per_cpu_end - __per_cpu_start));
    return p;
}

static inline char *get_percpu_base(int cpu)
{
    return cpu_base[cpu];
}

/*
static inline ulong cpu_percpu_valp(cpu_p pcpu,void *p)
{
     return ((ulong)p+(ulong)pcpu->percpu_base-(ulong)(__per_cpu_start));
}
*/
static inline struct cpu *base_to_cpu(char *base)
{
    return (struct cpu *)(((ulong) (&the_cpu)) + (ulong) base -
                          (ulong) (__per_cpu_start));
}

static inline ulong base_to_percpu(char *base, void *p)
{
    return ((ulong) p + (ulong) base - (ulong) (__per_cpu_start));
}

#define BASE_TO_PERCPU(base,p) \
    (typeof p)base_to_percpu(base,p)
static inline ulong cpuid_to_percpu(int cpu, void *p)
{
    return ((ulong) p + (ulong) cpu_base[cpu] - (ulong) __per_cpu_start);
}

#define CPUID_TO_PERCPU(cpu,p) \
      (typeof p)cpuid_to_percpu(cpu,p)

static inline ulong get_percpu_valp(void *p)
{

    ulong base = _get_cur_percpu_base();

    return base_to_percpu((char *)base, p);
}

static inline int get_cpu_number()
{
    char *base = (char *)_get_cur_percpu_base();

    for (int i = 0; i < nr_cpu; i++) {
        if (base == cpu_base[i])
            return i;
    }
    panic("can't get cpu_number:%lx\n", base);
    return 0;
}

static inline void *get_cur_percpu_base()
{
    return (void *)_get_cur_percpu_base();
}

static inline int this_cpu_number()
{
    return *((int *)get_percpu_valp(&cpu_number));
}

#define PERCPU_PTR(p) (typeof(p))(get_percpu_valp(p))

#define __this_cpu_rw_init() do{}while(0)

static inline ulong _per_cpu_ptr(void *p, int cpu)
{
    char *base = cpu_base[cpu];

    return base_to_percpu(base, p);
}

#define for_each_possible_cpu(cpu) \
    for(cpu=0;cpu<nr_cpu;cpu++)
#endif
