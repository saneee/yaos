#include <asm/cpu.h>
#include <asm/bitops.h>

#include <yaos/printk.h>
#include <asm/pm64.h>
#ifndef PAGE_4K_SIZE
#define PAGE_4K_SIZE 0x1000
#endif

struct cpu_features_type cpu_features;

void init_cpuid()
{
    u32 eax = 1;
    u32 ecx = 0;
    u32 ebx, edx;
    void bzero(void *, size_t);

    bzero(&cpu_features, sizeof(cpu_features));
    native_cpuid(&eax, &ebx, &ecx, &edx);
    printk("cpu features:\n");
    if (test_bit(9, (ulong *) & edx)) {
        //xapic 
    }
    if (test_bit(0, (ulong *) & ecx)) {
        cpu_features.sse3 = true;
        printk(" sse3");
    }
    if (test_bit(9, (ulong *) & ecx)) {
        cpu_features.ssse3 = true;
        printk(" ssse3");

    }
    if (test_bit(13, (ulong *) & ecx)) {
        cpu_features.cmpxchg16b = true;
        printk(" cmpxchg16b");

    }
    if (test_bit(19, (ulong *) & ecx)) {
        cpu_features.sse4_1 = true;
        printk(" sse4_1");

    }
    if (test_bit(20, (ulong *) & ecx)) {
        cpu_features.sse4_2 = true;
        printk(" sse4_2");

    }

    if (test_bit(21, (ulong *) & ecx)) {
        cpu_features.x2apic = true;
        printk(" x2apic");
    }

    if (test_bit(24, (ulong *) & ecx)) {
        cpu_features.tsc_deadline = true;
        printk(" tsc_deadline");
    }

    if (test_bit(26, (ulong *) & ecx)) {
        cpu_features.xsave = true;
        printk(" xsave");
    }

    if (test_bit(27, (ulong *) & ecx)) {
        cpu_features.osxsave = true;

        printk(" osxsave");
    }

    if (test_bit(28, (ulong *) & ecx)) {
        cpu_features.avx = true;
        printk(" avx");
    }

    if (test_bit(30, (ulong *) & ecx)) {
        cpu_features.rdrand = true;

        printk(" rdrand");
    }
    if (test_bit(19, (ulong *) & edx)) {
        cpu_features.clflush = true;

        printk(" clflush");
    }
    eax = 7;
    native_cpuid(&eax, &ebx, &ecx, &edx);
    if (test_bit(0, (ulong *) & ebx)) {
        cpu_features.fsgsbase = true;
        write_cr4(read_cr4() | cr4_fsgsbase);
        printk(" fsgsbase");
    }

    if (test_bit(9, (ulong *) & ebx)) {
        cpu_features.repmovsb = true;

        printk(" repmovsb");
    }
    printk("\n");

}

void init_cpuid_ap()
{
    if (cpu_features.fsgsbase) {
        write_cr4(read_cr4() | cr4_fsgsbase);
    }

}

void cprint_regs(struct trapframe *tf)
{
    ulong rsp = tf->rbp;
    int stacknr = 0;
    ulong *p;

    printk("CS:%x,DS:%x,SS:%x,apic:%d,cr3:%lx\n", read_cs(), read_ds(),
           read_ss(), cpuid_apic_id(), read_cr3());
    printk("TR:%x ", read_tr());
    printk("RSP:%lx,RBP:%lx,RDX:%lx\n", tf, tf->rbp, tf->rdx);
    printk("RAX:%lx,RBX:%lx,RCX:%lx\n", tf->rax, tf->rbx, tf->rcx);
    printk("RDX:%lx,RSI:%lx,RDI:%lx\n", tf->rdx, tf->rsi, tf->rdi);
    p = (ulong *) rsp;
//    for (int i = -10; i < 28; i += 3)
//        printf("addr:%lx v:%lx %lx %lx\n", &p[i], p[i], p[i + 1], p[i + 2]);
    while ((ulong) p >= tf->rbp && (ulong) p < tf->rbp + 0x4000
           && ++stacknr < 10) {
        p = (ulong *) * p;
        printk("%lx,rbp:%lx\n", *(p + 1), p);

    }
}

void init_cput()
{

}
