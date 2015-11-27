#include <pci_def.h>
#include <asm/cpu.h>
#include <yaos/printk.h>
#include <yaos/types.h>
#include <yaos/queue.h>
#include <asm/pci.h>

void init_virtio_net();
void init_pci()
{
      for(int b=0;b<255;b++){
         for(int s=0;s<31;s++){
           for(int f=0;f<7;f++){
              uint16_t vid=read_pci_config_word(b,s,f,0);
              uint16_t did=read_pci_config_word(b,s,f,2);
if(vid!=0xffff)printf("b:%d,s:%d,f:%d,vid:%lx,%lx\n",b,s,f,vid,did);
           } 
         } 
      }
init_virtio_net();
}
