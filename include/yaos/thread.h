#ifndef _YAOS_THREAD_H
#define _YAOS_THREAD_H
#define THREAD_INIT 0
#define THREAD_SUSPEND 1
#define THREAD_READY 2 
#define THREAD_RUN 3
#define THREAD_DONE 4
#define THREAD_REUSE 5
#include <yaos/types.h>
struct thread_t;
struct arch_cpu;
struct thread_t{
    ulong stack_addr;
    ulong stack_size;
    ulong rsp;
    u32 status;
    u32 cpuid;
    int (*main)(ulong arg);
    struct thread_t *pnext;    
    struct thread_t *father;
};
typedef struct thread_t * pthread;
static inline bool is_thread_done(pthread p)
{
    return THREAD_DONE==p->status;
}
static inline bool in_thread_run(pthread p)
{
    return THREAD_RUN==p->status;
}
void kernel_thread(void *func,ulong stacksize,void *param);
#endif
