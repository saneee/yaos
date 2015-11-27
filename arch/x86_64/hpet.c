#include <asm/hpet.h>
#include <asm/pgtable.h>
#define HPET_MASK			CLOCKSOURCE_MASK(32)

/* FSEC = 10^-15
   NSEC = 10^-9 */
#define FSEC_PER_NSEC			1000000L

#define HPET_DEV_USED_BIT		2
#define HPET_DEV_USED			(1 << HPET_DEV_USED_BIT)
#define HPET_DEV_VALID			0x8
#define HPET_DEV_FSB_CAP		0x1000
#define HPET_DEV_PERI_CAP		0x2000

#define HPET_MIN_CYCLES			128
#define HPET_MIN_PROG_DELTA		(HPET_MIN_CYCLES + (HPET_MIN_CYCLES >> 1))

#define HPET_BASE_ADDR 0xfed00000
/*
 * HPET address is set in acpi/boot.c, when an ACPI entry exists
 */

unsigned long hpet_address;
static void __iomem *hpet_virt_address;

inline struct hpet_dev *EVT_TO_HPET_DEV(struct clock_event_device *evtdev)
{
    return container_of(evtdev, struct hpet_dev, evt);
}

inline unsigned int hpet_readl(unsigned int a)
{
    return readl(hpet_virt_address + a);
}

static inline void hpet_writel(unsigned int d, unsigned int a)
{
    writel(d, hpet_virt_address + a);
}


static inline void hpet_set_mapping(void)
{
    hpet_virt_address = ioremap_nocache(hpet_address, HPET_MMAP_SIZE);
}
void init_hpet(void)
{
    ioremap_nocache(HPET_BASEADDR,HPT_MMAP_SIZE);
}
