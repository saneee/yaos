#include <yaos/types.h>
#include <yaos/module.h>
#include <yaos/printk.h>
#include <yaoscall/malloc.h>
DECLARE_MODULE(pci_bus_drivers, 0, main);

static int main(module_t m, ulong t, void *arg)
{
    modeventtype_t env = (modeventtype_t) t;

    if (env == MOD_BPLOAD) {
        printk("pci.bus.drivers module, %lx,%lx,%lx\n", m, t, arg);
        void *p=yaos_malloc(0x10);
        if(!p)return MOD_ERR_NEXTLOOP;//malloc not ready
    }
    return MOD_ERR_OK;          //MOD_ERR_NEXTLOOP;
}
