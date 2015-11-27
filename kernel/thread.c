#include <yaos/rett.h>          //double_ret
#include <yaos/thread.h>
#include <yaos/vm.h>
#include <asm/cpu.h>
#include <asm/pm64.h>
#include <asm/pgtable.h>
#include <yaos/percpu.h>
#include <yaos/init.h>
#include <yaos/atomic.h>
#include <yaoscall/malloc.h>
#ifndef MAX_THREADS_PER_CPU
#define MAX_THREADS_PER_CPU 1000
#endif
#if 1
#define DEBUG_PRINT printk
#else
#define DEBUG_PRINT inline_printk

#endif
PERCPU(static struct thread_t, threads[MAX_THREADS_PER_CPU]);
PERCPU(static pthread, p_free);
PERCPU(static pthread, p_head);

PERCPU(static pthread, p_idle);
extern ret_t switch_to(ulong stack, ulong * poldstack, ulong arg);
extern ret_t switch_first(pthread, void *, ulong, ulong *, ulong);	//switch stack
ret_t switch_thread(pthread p, ulong arg);

ret_t yield_thread(ulong);
pthread create_thread(ulong stack_size, void *main);
ret_t resume_thread(pthread p, ulong arg);
ret_t yield_thread(ulong arg);
atomic_t idlenr = ATOMIC_INIT(0);
static void thread_main(pthread p, ulong arg)
{
    p->status = THREAD_RUN;
    p->main(arg);
}

static ret_t test_thread(ulong arg1)
{
    ret_t ret;

    printk("test_thread 1start %lx,%lx\n", arg1);
    ret = yield_thread(ret.v * 16);
    printk("test_thread 2 start %lx\n", ret.v);
    ret = yield_thread(arg1 * 16);
    printk("test_thread 3 start %lx\n", ret.v);

    ret = yield_thread(arg1 * 256);
    printk("test_thread 3 start %lx\n", ret.v);
    ret.v = 0x87654321;
    ret.e = 0;
    return ret;

}

static ret_t idle_main(ulong arg1)
{
    pthread p;
    ret_t result;
    __thread cpu_p pcpu = get_current_cpu();

    printk("idle_main start %lx\n", arg1);
    atomic_inc(&idlenr);
    if (is_bp_cpu(pcpu)) {
        void *p2 = 0;

        for (size_t i = 8; i < 2096; i += 8) {

            void *p = yaos_malloc(i);

            printk("malloc:return %lx,size:%lx\n", p, i);
            if (p2 != 0) {
                yaos_mfree(p);
                yaos_mfree(p2);
                p2 = 0;
            }
            else
                p2 = p;
        }
    }
    for (;;)
        sti_hlt();
    p = create_thread(PAGE_SIZE, test_thread);
    result = resume_thread(p, 0x123);
    printk("resume return %lx\n", result.v);
    result = resume_thread(p, 0x456);
    printk("resume return %lx\n", result.v);
    result = resume_thread(p, 0x789);
    printk("resume return %lx\n", result.v);
    result = resume_thread(p, 0x9999);
    printk("resume return:%lx\n", result.v);
    printk("thread status:%d,%lx\n", p->status, p);
    result = resume_thread(p, 0x8888);
    printk("resume return:%lx\n", result.v);
    printk("hello world\n");
    for (;;)
        sti_hlt();
    result.v = 0;
    result.e = 0;
    return result;
}

void goto_idle(void)
{
    idle_main(0);
}

ret_t exit_thread(ulong arg)
{
    __thread pthread p;
    __thread cpu_p pcpu = get_current_cpu();
    __thread pthread pold = pcpu->current_thread;

    p = pold->father;
    pold->status = THREAD_DONE;
    printk("thread exist:%lx,status:%d,%lx,%lx\n", arg, pold->status, pold, p);
    print_regs();
    if (unlikely(!p)) {
        panic("no father thread running\n");
    }
    else {
        if (p->status == THREAD_READY) {

            p->status = THREAD_RUN;
            return arch_switch_first(p, thread_main, pold, arg);
            //return switch_first(p, thread_main, stack_point, &pold->rsp, arg);                                        //no return

        }
        else {
            p->status = THREAD_RUN;
            return arch_switch_to(p, pold, arg);
        }
    }
    printk("exec exist thread\n");
    for (;;)
        sti_hlt();
}

ret_t resume_thread(pthread p, ulong arg)
{
    __thread pthread pold;
    __thread cpu_p pcpu = get_current_cpu();

    if (p->status == THREAD_DONE) {
        printk("resume done thread\n");
        ret_t t = { 0, -1 };
        return t;
    }
    pold = pcpu->current_thread;
    p->father = pold;
    ASSERT(pold != p);
    pcpu->current_thread = p;
    if (p->status == THREAD_READY) {

        p->status = THREAD_RUN;
        return arch_switch_first(p, thread_main, pold, arg);

    }
    else {
        //p->status = THREAD_RUN;
        return arch_switch_to(p, pold, arg);
    }

}

ret_t yield_thread(ulong arg)
{
    __thread pthread p;
    __thread cpu_p pcpu = get_current_cpu();
    __thread pthread pold = pcpu->current_thread;

    p = pold->father;
    ASSERT(p != pold);
    if (unlikely(!p)) {
        panic("no father thread running\n");
    }
    else {
        pcpu->current_thread = p;
        if (p->status == THREAD_READY) {

            p->status = THREAD_RUN;
            return arch_switch_first(p, thread_main, pold, arg);

        }
        else {
            p->status = THREAD_RUN;
            return arch_switch_to(p, pold, arg);
        }

    }
    ret_t t = { 0, -1 };
    return t;
}

pthread create_thread(ulong stack_size, void *main)
{
    cpu_p pcpu = get_current_cpu();
    pthread p;
    pthread *ptr;

    ptr = (pthread *) cpu_percpu_valp(pcpu, &p_free);
    p = *ptr;
    *ptr = p->pnext;
    p->main = main;
    p->stack_size = stack_size;
    if (likely(stack_size)) {
        ulong stack_addr = (ulong) alloc_vm_stack(stack_size);

        if (unlikely(!stack_addr)) {
            printk("no memory alloc stack:%lx\n", stack_size);
            *ptr = p;
            return (pthread) 0;
        }
        p->stack_addr = stack_addr;
    }
    else {
        ASSERT(idle_main == main);	//only idle thread can use init stack
        p->stack_addr = (ulong) pcpu->arch_cpu.stack.init_stack;	//use init stack
        p->stack_size = INIT_STACK_SIZE;
    }
    p->status = THREAD_READY;
    ptr = (pthread *) cpu_percpu_valp(pcpu, &p_head);
    p->pnext = *ptr;
    *ptr = p;
    printk("create_thread ptr:%lx,%lx\n", ptr, *(ulong *) ptr);
    return p;
}

ret_t switch_thread(pthread p, ulong arg)
{
    pthread pold;
    cpu_p pcpu = get_current_cpu();

    pold = pcpu->current_thread;
    pcpu->current_thread = p;
    if (p->status == THREAD_READY) {

        p->status = THREAD_RUN;
        return arch_switch_first(p, thread_main, pold, arg);

    }
    else {
        p->status = THREAD_RUN;
        return arch_switch_to(p, pold, arg);
    }
}

void run_thread(pthread p, ulong arg1)
{
    ulong stack_point = p->stack_addr + p->stack_size - 8;
    ulong nouse = 0;

    switch_first(p, thread_main, stack_point, &nouse, arg1);

}

void kernel_thread(void *func, ulong stacksize, void *param)
{
    pthread p = create_thread(stacksize, func);

    resume_thread(p, (ulong) param);
}

void attach_thread(cpu_p cpu, pthread p)
{
    ASSERT(!cpu->current_thread);	//only once
    cpu->current_thread = p;
    p->status = THREAD_RUN;
}

__init void init_thread_ap(void)
{
    cpu_p pcpu = get_current_cpu();
    pthread p = (pthread) cpu_percpu_valp(pcpu, &threads[0]);
    __thread pthread *ptr;

    DEBUG_PRINT("pcpu:%lx,id:%d,p:%lx,", pcpu, pcpu->arch_cpu.apic_id, p);

    for (int i = 0; i < MAX_THREADS_PER_CPU; i++) {
        p[i].status = THREAD_INIT;
        p[i].pnext = &p[i + 1];
    }
    p[MAX_THREADS_PER_CPU - 1].pnext = (pthread) 0;
    ptr = (pthread *) cpu_percpu_valp(pcpu, &p_free);
    *ptr = (pthread) cpu_percpu_valp(pcpu, threads);
    DEBUG_PRINT(" pfree:%lx", ptr);
    ptr = (pthread *) cpu_percpu_valp(pcpu, &p_head);
    //*ptr = (pthread) 0;
    DEBUG_PRINT(" pfirst:%lx\n", ptr);
    p = create_thread(0, idle_main);
    ptr = (pthread *) cpu_percpu_valp(pcpu, &p_idle);
    *ptr = p;
    pcpu->current_thread = (pthread) 0;
    attach_thread(pcpu, p);

//    if (!is_bp_cpu(pcpu)) {     //ap not return
//        idle_main((ulong) pcpu);
//    }

    //run_thread(p, (ulong) pcpu);
}

__init int static init_thread_call(bool isbp)
{
    init_thread_ap();
    
    return 0;
}

early_initcall(init_thread_call);
