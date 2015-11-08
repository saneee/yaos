/* zhuhanjun 2015/10/29 
*/
#include <asm/cpu.h>
#include "pm64.h"
#include <yaos/string.h>
#include <yaos/assert.h>
#include <yaos/printk.h>
#include <yaos/percpu.h>
#include <asm/apic.h>
#include <asm/pgtable.h>
#define MAX_CPUS	1024
struct idt_entry {
    u16 offset0;
    u16 selector;
    u8 ist:3;
    u8 res0:5;
    u8 type:4;
    u8 s:1;
    u8 dpl:2;
    u8 p:1;
    u16 offset1;
    u32 offset2;
    u32 res1;
} __attribute__ ((aligned(16)));
struct idt_entry _idt[256];
PERCPU(struct arch_cpu, the_cpu);
char *cpu_base[MAX_CPUS];
uint nr_cpu = 0;
void test()
{
print_regs();
printk("jumped to zero?\n");
for(;;);
}
void add_int_vector(unsigned n, unsigned ist, void *handler)
{
    ulong addr = (ulong) handler;
    struct idt_entry e;

    ASSERT(n < 256);
    e.offset0 = addr;
    e.selector = read_cs();
    // We can't take interrupts on the main stack due to the x86-64 redzone
    e.ist = ist;
    e.type = INTR_GATE_TYPE;
    e.s = 0;
    e.dpl = 0;
    e.p = 1;
    e.offset1 = addr >> 16;
    e.offset2 = addr >> 32;
    _idt[n] = e;
}

void init_arch_cpu(struct arch_cpu *p)
{
    p->status = 0;
    memset(&p->atss, 0, sizeof(p->atss));
    p->gdt[0] = 0;
    p->gdt[gdt_cs] = 0x00af9b000000ffff;
    p->gdt[gdt_ds] = 0x00cf93000000ffff;
    p->gdt[gdt_cs32] = 0x00cf9b000000ffff;
    p->gdt[gdt_tss] = 0x0000890000000067;
    p->gdt[nr_gdt] = 0;
    u64 tss_addr = (u64) & p->atss.tss;

    p->gdt[gdt_tss] |= (tss_addr & 0x00ffffff) << 16;
    p->gdt[gdt_tss] |= (tss_addr & 0xff000000) << 32;
    p->gdt[gdt_tssx] = tss_addr >> 32;

    set_exception_stack(p);
    set_interrupt_stack(p);
}

void bootup_cpu(struct arch_cpu *p)
{
    extern void probe_apic();
    void init_idt(void);
    struct desc_ptr desc;
printk("starting %d\n",p->apic_id);
    desc.limit = nr_gdt * 8 - 1;
    desc.addr = (ulong) (&p->gdt);
    init_idt();
    lgdt(&desc);
    ltr(gdt_tss * 8);
    p->status = CPU_RUNNING;
printk("cpu %d started,%lx\n",p->apic_id,&p->status);
}

void bootup_cpu_ap()
{
    struct arch_cpu *p;
    u32 apicid = cpuid_apic_id();


    for (int i = 0; i < nr_cpu; i++) {
        p = base_to_cpu(cpu_base[i]);

        if (p->apic_id == apicid) {
            bootup_cpu(p);
            return;
        }
    }
}

void bootup_cpu_bp()
{
uchar code[20];
ulong addr=(ulong)test-5;
uchar *ptr=(uchar *)0;
code[0]=0xe9;
code[1]=addr&0xff;
code[2]=(addr>>8)&0xff;
code[3]=(addr>>16)&0xff;
code[4]=(addr>>24)&0xff;
for(int i=0;i<5;i++){
*ptr++=code[i];
printf(" %x ",code[i]);
}
printk("test %lx\n",(ulong)test-5);
    bootup_cpu(&the_cpu);
}
void init_cpu_bp(void){
    init_arch_cpu(&the_cpu);
    bootup_cpu_bp();
}
void init_cpu(u32 apicid)
{

    struct arch_cpu *p;
    char *ptr;
    void init_idt();

    if (!nr_cpu) {              //first cpu 
        ptr = (char *)&the_cpu;
    }
    else {
        ptr = new_percpu();
    }
    cpu_base[nr_cpu++] = ptr;
    p = base_to_cpu(ptr);
    p->percpu_base = ptr;
    p->apic_id = apicid;

    printk("cpu:%x,%lx,%d\n", nr_cpu, p, p->apic_id);
    init_arch_cpu(p);
}

void init_idt()
{
    struct desc_ptr desc;
    extern ulong vectors[];

    for (int i = 0; i < 32; i++) {
        add_int_vector(i, 1, (char *)vectors[i]);
    }
    for (int i = 32; i < 256; i++) {
        add_int_vector(i, 2, (char *)vectors[i]);
    }
    desc.limit = sizeof(_idt) - 1;
    desc.addr = (ulong) & _idt;
    lidt(&desc);
    printk("sizeof(_idt):%lx,%lx,%lx\n", sizeof(_idt), desc.addr, desc.limit);
}

void start_aps(void)
{
    extern uchar _binary_out_entryother_start[], _binary_out_entryother_size[];
    extern void lapic_start_ap(uint apicid, uint addr);
    extern void entry32mp(void);
    uchar *code;
    __thread  struct arch_cpu *p;
    u32 apicid = cpuid_apic_id();

    code = (uchar *)P2V(0x7000);
    memmove(code, _binary_out_entryother_start,
            (uintp) _binary_out_entryother_size);
    for (int i = 0; i < nr_cpu; i++) {
        p = base_to_cpu(cpu_base[i]);
printk("try %d,%lx,%d\n",p->apic_id,p,nr_cpu);
        if (p->apic_id == apicid ) {
            continue;
        }
        *(uint32 *) (code - 4) = 0x7000-32;//init stack in code 16,code 32
        *(uint32 *) (code - 8) = V2P((u64)entry32mp);
        //init stack in code 64
        *(uint64 *) (code - 16) = ((uint64) (&p->stack.init_stack) )+ INIT_STACK_SIZE;

printk("startcode:%lx,stack:%lx,cr3:%lx\n",V2P((u64)entry32mp), ((uint64) (&p->stack.init_stack) )+ INIT_STACK_SIZE,read_cr3());
        lapic_start_ap(p->apic_id,(u32)((u64)code));
        printk("wait for %d,%lx\n",p->apic_id,&p->status);
        while(p->status==0);


    }

}
