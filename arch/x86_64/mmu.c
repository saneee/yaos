#include <yaos/types.h>
#include <asm/bitops.h>
#include <yaos/printk.h>
#include <asm/pgtable.h>
static ulong test[] =
    { 8, 16, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 4, 8, 16, 32, 64, 128,
    256, 512
};

extern ulong __max_phy_addr;
void init_mmu()
{
    ulong bits = __max_phy_addr / PAGE_SIZE;
    int i;

    for (i = 0; i < 1280; i++) {
        test_and_set_bit(i, &test[0]);
//printf("bit:%d,%x\n",i,test_and_set_bit(i,&test[0]));
    }
    printf("%lx,%lx,%lx,%lx\n", test[0], test[1], test[2], bits);
}
