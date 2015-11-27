/*zhuhanjun 2015/11/11
*/
#include <asm/cpu.h>
#include <yaos/printk.h>
#include <asm/irq.h>
#include <asm/pm64.h>
typedef void (*TRAP_FUNC) (struct trapframe *);
extern TRAP_FUNC c_vectors[256];
void ttt(struct trapframe *tf)
{
    c_vectors[tf->trapno] (tf);

}

static ulong m_time = 0;
void default_trap(struct trapframe *tf)
{
    if (tf->trapno != LOCAL_TIMER_VECTOR) {
        ulong *p;
        int stacknr=0;
        printk("Exception EIP:%lx,RBP:%lx,Error:%lx\n", tf->rip, tf->rbp,
               tf->trapno);
        printk("RAX:%lx,RBX:%lx,RCX:%lx\n", tf->rax, tf->rbx, tf->rcx);
        printk("RDX:%lx,RSI:%lx,RDI:%lx\n", tf->rdx, tf->rsi, tf->rdi);
        printk("eflag:%lx,rsp:%lx,err:%lx\n", tf->eflags, tf->rsp, tf->err);
        //p = (ulong *) tf->rsp;
        
       // for (int i = -10; i < 28; i += 3)
       //     printf("addr:%lx v:%lx %lx %lx\n", &p[i], p[i], p[i + 1], p[i + 2]);
        p = (ulong *) tf->rbp;
        printk("call stack:\n\%lx,rbp:%lx\n", *(p + 1), p);
        while ((ulong) p >= tf->rbp && (ulong) p < tf->rbp + 0x4000 && ++stacknr<10) {
            p = (ulong *) * p;
            printk("%lx,rbp:%lx\n", *(p + 1), p);

        }
       if(tf->trapno<15)for(;;)sti_hlt();

    }
    else {
        if (++m_time == 1) {
            printf("__TIME__\n");
            m_time = 0;
        }
    }
}

void trap_switch(struct trapframe *tf)
{

}

void default_exception(struct trapframe *tf)
{
    ulong *p;
    int stacknr=0;
    printk("Exception EIP:%lx,RBP:%lx,Error:%lx\n", tf->rip, tf->rbp,
           tf->trapno);
    printk("RAX:%lx,RBX:%lx,RCX:%lx\n", tf->rax, tf->rbx, tf->rcx);
    printk("RDX:%lx,RSI:%lx,RDI:%lx\n", tf->rdx, tf->rsi, tf->rdi);
    printk("eflag:%lx,rsp:%lx,err:%lx\n", tf->eflags, tf->rsp, tf->err);
   // p = (ulong *) tf->rsp;
    //for (int i = -10; i < 28; i += 3)
      //  printf("addr:%lx v:%lx %lx %lx\n", &p[i], p[i], p[i + 1], p[i + 2]);
    p = (ulong *) tf->rbp;
    printk("call stack:\n\%lx,rbp:%lx\n", *(p + 1), p);
    while ((ulong) p >= tf->rbp && (ulong) p < tf->rbp + 0x4000 && ++stacknr<10) {
        p = (ulong *) * p;
        printk("%lx,rbp:%lx\n", *(p + 1), p);

    }
    if(tf->trapno<=15)for(;;)sti_hlt();
    for (;;)
        sti_hlt();
}

void default_syscall(struct trapframe *tf)
{

    ulong *p;

    printk("SYSCALL EIP:%lx,RBP:%lx,Error:%lx\n", tf->rip, tf->rbp,
           tf->trapno);
    printk("RAX:%lx,RBX:%lx,RCX:%lx\n", tf->rax, tf->rbx, tf->rcx);
    printk("RDX:%lx,RSI:%lx,RDI:%lx\n", tf->rdx, tf->rsi, tf->rdi);
    printk("eflag:%lx,rsp:%lx,err:%lx\n", tf->eflags, tf->rsp, tf->err);
    p = (ulong *) tf->rsp;
    for (int i = -10; i < 28; i += 3)
        printf("addr:%lx v:%lx %lx %lx\n", &p[i], p[i], p[i + 1], p[i + 2]);
    p = (ulong *) tf->rbp;
    printk("call stack:\n\%lx,rbp:%lx\n", *(p + 1), p);
    while ((ulong) p >= tf->rbp && (ulong) p < tf->rbp + 0x4000){
         p = (ulong *) * p;
        printk("%lx,rbp:%lx\n", *(p + 1), p);

    }

}
