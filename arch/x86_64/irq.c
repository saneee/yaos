#include <asm/cpu.h>
#include <asm/irq.h>
#include <asm/pm64.h>
void default_irq_trap(struct trapframe *tf)
{
    ((irq_handler_t )(irq_vectors[tf->trapno]))((int)tf->trapno);
for(;;)cli_hlt();
}
void default_irq_handler(int n)
{
    printk("irq:%d\n",n);
}

