#include <asm/cpu.h>
#include <yaos/percpu.h>
#include <yaos/cpupm.h>
#include <asm/pm64.h>
void check_stack(void)
{
    __thread cpu_p pcpu = get_current_cpu();
    extern ulong init_stack_top[];
    extern void switchtrapret(void);
    ulong min;
    ulong max;
    ulong rbp = read_rbp();

    if (is_bp_cpu(pcpu)) {
        max = (ulong) init_stack_top;
        min = max - 4 * 4096;
        printk("BP stack:min:%lx-max:%lx,irqret:%lx,rbp:%lx\n", min, max,
               switchtrapret, rbp);
    }
    else {
        min = (uint64) per_cpu_ptr(&init_stack, pcpu->cpu);
        max = min + sizeof(init_stack);
        printk("AP stack:min:%lx-max:%lx\n", min, max);

    }
    ulong *p;
    int stacknr = 0;

    p = (ulong *) rbp;
    while ((ulong) p >= min && (ulong) p < max && ++stacknr < 50) {
        if ((ulong) (switchtrapret) == *(p + 1)) {
            printk("irq:%lx,rbp:%lx,rip:%lx\n", *(p + 1), p, *(p + 19));
        }
        else
            printk("%lx,rbp:%lx\n", *(p + 1), p);

        p = (ulong *) * p;

    }

}
