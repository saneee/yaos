#ifndef _YAOS_CPUPM_H
#define _YAOS_CPUPM_H
#include <asm/cpupm.h>
struct thread_t;
struct cpu{
struct arch_cpu arch_cpu;
char *percpu_base;
struct thread_t *current_thread;
unsigned long status;
};
typedef struct cpu* cpu_p;
static inline cpu_p arch_cpu_po_cpu(struct arch_cpu * p)
{
    return (cpu_p)((ulong)p-offsetof(struct cpu,arch_cpu));
}
static inline  cpu_p get_current_cpu()
{
    return arch_cpu_po_cpu(get_current_arch_cpu());
}
static inline bool is_bp()
{
    return arch_is_bp();
}
#endif
