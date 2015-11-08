#include <asm/apic.h>
#include <asm/cpu.h>
#include <asm/irq.h>
#include <asm/pgtable.h>
#include <asm/bitops.h>
#include <yaos/printk.h>
#include <asm/cpu.h>
#include <asm/apic.h>
#define CMOS_PORT 0x70
bool is_xapic = true;
bool is_x2apic = false;
u32 *lapic_base;                //set by acpi.c
void probe_apic()
{
    u32 eax = 1;
    u32 ecx = 0;
    u32 ebx, edx;

    native_cpuid(&eax, &ebx, &ecx, &edx);
    if (test_bit(9, (ulong *) & edx)) {
        is_xapic = true;
        printk("xAPIC ok %lx %lx %lx %lx\n", eax, ebx, ecx, edx);
    }
    if (test_bit(21, (ulong *) & ecx)) {
        is_x2apic = true;
        printk("x2APIC ok\n");
    }
    eax = 0;
    native_cpuid(&eax, &ebx, &ecx, &edx);
    printk("Obf:%lx,%lx,%lx,%lx\n", eax, ebx, ecx, edx);
    eax = 0x0b;
    native_cpuid(&eax, &ebx, &ecx, &edx);
    printk("Obf:%lx,%lx,%lx,%lx\n", eax, ebx, ecx, edx);

}

static void microdelay(ulong m)
{
}

void lapic_start_ap(uint apicid, uint addr)
{
    ushort *wrv;

    // "The BSP must initialize CMOS shutdown code to 0AH
    // and the warm reset vector (DWORD based at 40:67) to point at
    // the AP startup code prior to the [universal startup algorithm]."
    outb(0xF, CMOS_PORT);       // offset 0xF is shutdown code
    outb(0x0A, CMOS_PORT + 1);
    wrv = (ushort *) P2V((0x40 << 4 | 0x67));	// Warm reset vector
    wrv[0] = 0;
    wrv[1] = addr >> 4;
    x2apic_enable();
    // "Universal startup algorithm."
    // Send INIT (level-triggered) interrupt to reset other CPU.
    lapic_write(APIC_ICR2, apicid << 24);
    lapic_write(APIC_ICR, APIC_DM_INIT | APIC_INT_LEVELTRIG | APIC_INT_ASSERT);
    microdelay(200);
    lapic_write(APIC_ICR, APIC_DM_INIT | APIC_INT_LEVELTRIG);
    microdelay(10000);
 for(int i = 0; i < 2; i++){
    lapic_write(APIC_ICR2, apicid<<24);
    lapic_write(APIC_ICR, APIC_DM_STARTUP | (addr>>12));
    microdelay(200);
  }

}

void init_lapic(void)
{
    // Enable local APIC; set spurious interrupt vector.
    lapic_write(APIC_SPIV, APIC_SPIV_APIC_ENABLED | SPURIOUS_APIC_VECTOR);
    // The timer repeatedly counts down at bus frequency
    // from lapic[TICR] and then issues an interrupt.
    // If xv6 cared more about precise timekeeping,
    // TICR would be calibrated using an external time source.
    lapic_write(APIC_TDCR, APIC_TDR_DIV_1);
    lapic_write(APIC_LVTT, APIC_LVT_TIMER_PERIODIC | LOCAL_TIMER_VECTOR);
    lapic_write(APIC_TMICT, 10000000);

    // Disable logical interrupt lines.
    lapic_write(APIC_LVT0, APIC_LVT_MASKED);
    lapic_write(APIC_LVT1, APIC_LVT_MASKED);

// Disable performance counter overflow interrupts
    // on machines that provide that interrupt entry.
    if (GET_APIC_MAXLVT(lapic_read(APIC_LVR) >= 4)) {
        lapic_write(APIC_LVTPC, APIC_LVT_MASKED);
    }

    // Map error interrupt to IRQ_ERROR.
    lapic_write(APIC_LVTERR, ERROR_APIC_VECTOR);

    // Clear error status register (requires back-to-back writes).
    lapic_write(APIC_ESR, 0);
    lapic_write(APIC_ESR, 0);

    // Ack any outstanding interrupts.
    lapic_write(APIC_EOI, 0);

    // Send an Init Level De-Assert to synchronise arbitration ID's.
    lapic_write(APIC_ICR2, 0);
    lapic_write(APIC_ICR, APIC_DEST_ALLINC | APIC_DM_INIT | APIC_INT_LEVELTRIG);
    while (lapic_read(APIC_ICR) & APIC_ICR_BUSY) {
    }

    lapic_write(APIC_TASKPRI, 0);
}
