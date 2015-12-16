#include <asm/msi.h>
#include <asm/pci.h>
void msix_vector_init(msix_vector_t t, pci_device_t pci)
{
    t->_dev = pci;
}
