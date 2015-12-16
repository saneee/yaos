#ifndef __YAOS_ALLOCONLY_H
#define __YAOS_ALLOCONLY_H
struct ao_malloc {
    void *phead;

};
void aomem_init(struct ao_malloc *p);
void aomem_free(struct ao_malloc *pobj, void *p, ulong size);
void *aomem_alloc(struct ao_malloc *pobj, ulong size);
#endif
