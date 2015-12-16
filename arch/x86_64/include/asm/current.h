#ifndef _ASM_CURRENT_H
#define _ASM_CURRENT_H
#include <yaos/compiler.h>
#include <asm/percpu.h>
#ifndef __ASSEMBLY__
struct thread_struct;
DECLARE_PER_CPU(struct thread_struct *, current_thread);
static __always_inline struct thread_struct *get_current(void)
{
    return this_cpu_read_stable(current_thread);
}

#define current get_current()

#endif
#endif
