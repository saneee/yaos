#include <yaos/types.h>
#include <yaos/module.h>
#include <yaos/printk.h>
DECLARE_MODULE(virtio_net_drivers, 0, main);
void init_virtio_net();


static int main(module_t m, ulong t, void *arg)
{
    modeventtype_t env = (modeventtype_t) t;

    if (env == MOD_BPLOAD) {
        printk("virtio_net_drivers, %lx,%lx,%lx\n", m, t, arg);
        init_virtio_net();
    }
    return MOD_ERR_OK;          //MOD_ERR_NEXTLOOP;
}
