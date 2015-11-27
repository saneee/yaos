#ifndef _ASM_PHYMEM_H
#define _ASM_PHYMEM_H
ulong alloc_phy_page();
void free_phy_one_page(ulong addr);
void free_phy_pages(ulong addr, size_t size);
#endif
