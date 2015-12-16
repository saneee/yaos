#ifndef _ASM_MSI_H
#define _ASM_MSI_H
struct pci_device;
struct msix_vector {
    unsigned _vector;
    void (*_handler) ();
    struct pci_device *_dev;
};
typedef msix_vector *msix_vector_t;
#endif
