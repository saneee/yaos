#include <asm/cpu.h>
#include <yaos/printk.h>
void print_regs()
{
    ulong rsp=read_rbp();
    ulong *p;
    printk("CS:%x,apic:%d,cr3:%lx\n", read_cs(),cpuid_apic_id(),read_cr3());
    printk("TR:%x\n", read_tr());
    printk("RSP:%lx\n",rsp);
    p=(ulong *)rsp;
    for(int i=-10;i<28;i+=3)printf("addr:%lx v:%lx %lx %lx\n",&p[i],p[i],p[i+1],p[i+2]);
}

void init_cput()
{

}
