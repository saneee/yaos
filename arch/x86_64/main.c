#include <asm/cpu.h>
#include "pm64.h"
#include "yaos.h"
#include <yaos/printk.h>
#include <yaos/kheap.h>
#include <asm/pgtable.h>
#include <asm/cpu.h>
#include <asm/apic.h>
#include <asm/irq.h>
extern void uart_early_init();
void init_bootload(u32 addr, u32 magic);
void init_cpu(u32);
void init_bss();
void init_mmu();
void init_e820();
void init_page_map();
void init_phy_mem();
void init_pgtable();
void init_pgtable_ap();
void init_idt();
void init_acpi();
void init_kheap();
void init_lapic();
void init_cpu_bp();
void bootup_cpu_ap();
void start_aps();
void init_yaos()
{
    char *ptr = (char *)&g_yaos;
    int i;

    for (i = 0; i < sizeof(g_yaos); i++) {
        *ptr++ = 0;
    }
}

void trap(struct trapframe *tf)
{
    if(tf->trapno!=LOCAL_TIMER_VECTOR){
    tf->rip++;
    printk("EIP:%lx,Error:%lx\n", tf->rip, tf->trapno);
print_regs();
for(;;)
    cli_hlt();
    }
}

void bp_main(u32 info_addr, u32 magic)
{
    extern ulong vectors[];
    extern u64 get_pte_with_addr(u64 addr);
extern void test(void);
extern void kheap_debug(void);
    extern void probe_apic();
    extern u32 *lapic_base;
    u64 addr;
    void (*pfunc) (void);

    init_yaos();
    uart_early_init();
    init_kheap();               //set up kheap manager
    init_bss();                 //free 64k heap first
printk("rsp:%lx,%lx\n",read_rsp(),(ulong)&the_cpu.stack.init_stack);
    init_bootload(info_addr, magic);	//setup __max_phy_addr
printk("rsp:%lx\n",read_rsp());
    init_phy_mem();             //set up phy memory alloc bitsmap before bootload 
    init_e820();                //free phy memory,free more kheap
    init_pgtable();
    init_page_map();
    init_cpu_bp();//load bp seg
    kheap_debug();
    init_acpi();
    addr = get_pte_with_addr((ulong) lapic_base);
    printk("\nbase:%lx,%lx\n", addr, *(ulong *) addr);
    init_lapic();


    start_aps();
    probe_apic();
    printf("apicid:%lx,%lx\n", lapic_id(), lapic_read(0x30));
cli_hlt();
    asm volatile ("int $0x80");

    asm volatile ("int3");

    pfunc = (void (*)(void))(vectors[1]);
    (*pfunc) ();
    cli_hlt();
    asm volatile ("int3");

    for (;;) ;
}
void ap_main(void)
{
    printf("ap starting...%d\n",cpuid_apic_id());
print_regs();
    init_pgtable_ap();
    init_lapic();
    bootup_cpu_ap();
    for(;;)sti_hlt();
}
