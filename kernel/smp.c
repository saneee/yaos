#include <yaos/cpupm.h>
int nr_cpu=0;
void smp_init()
{
    arch_start_aps();
}
