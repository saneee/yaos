#include <asm/cpu.h>
void halt(void)
{
    while (true) {
        cli_hlt();
    }
}

void poweroff(void)
{

}

void reboot(void)
{
    struct desc_ptr desc;

    desc.limit = 0;
    desc.addr = 0;
    lidt(&desc);

    outb(1, 0x92);
    outb(0xfe, 0x64);
    lidt(&desc);
    __asm__ __volatile__("int3");

}
