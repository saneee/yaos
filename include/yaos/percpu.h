#ifndef __YAOS_PERCPU_H
#define __YAOS_PERCPU_H
#include <asm/percpu.h>

#ifndef PERCPU
#define PERCPU(type, var) type var __attribute__((section(".data..percpu"))) 
#endif
#ifndef __percpu_data
#define __percpu_data __attribute__((section(".data..percpu")))
#endif
#include <yaos/cpupm.h>
extern char __per_cpu_start[];
extern char __per_cpu_end[];
extern struct cpu the_cpu;
static inline char *new_percpu()
{
    char *p= kalloc((__per_cpu_end-__per_cpu_start));
    memcpy(p,(char *)__per_cpu_start,(__per_cpu_end-__per_cpu_start));
    return p;
}
static inline ulong cpu_percpu_valp(cpu_p pcpu,void *p)
{
     return ((ulong)p+(ulong)pcpu->percpu_base-(ulong)(__per_cpu_start));
}
static inline cpu_p base_to_cpu(char * base)
{
   return (cpu_p)(((ulong)(&the_cpu))+(ulong)base-(ulong)(__per_cpu_start));
}
static inline ulong base_to_percpu(char* base,void *p)
{
   return ((ulong)p+(ulong)base-(ulong)(__per_cpu_start));
}
#define BASE_TO_PERCPU(base,p) \
    (typeof p)base_to_percpu(base,p)
extern char *cpu_base[];
static inline ulong cpuid_to_percpu(int cpu,void *p)
{
    return ((ulong)p+(ulong)cpu_base[cpu]-(ulong)__per_cpu_start);
}
#define CPUID_TO_PERCPU(cpu,p) \
      (typeof p)cpuid_to_percpu(cpu,p)

static inline ulong get_percpu_valp(void *p)
{

    ulong base=_get_cur_percpu_base();
    return base_to_percpu((char*)base,p);
}
#define PERCPU_PTR(p) (typeof(p))(get_percpu_valp(p))

#endif
