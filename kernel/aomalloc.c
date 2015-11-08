#include <yaos/aomalloc.h>
#include <yaos/assert.h>
struct alloc_only_head;
struct alloc_only_head {
    ulong size;
    ulong start;
    ulong freesize;
    struct alloc_only_head *pnext;
};
typedef struct alloc_only_head ao_head_t;
static ao_head_t *phead = 0;
void aomem_init(struct ao_malloc *p)
{
    p->phead = 0;
}

void aomem_free(struct ao_malloc *pobj, void *p, ulong size)
{
    ASSERT(size > sizeof(ao_head_t));
    ao_head_t *ptr = (ao_head_t *) p;

    size -= sizeof(ao_head_t);
    ptr->size = size;
    ptr->start = (ulong) p + sizeof(ao_head_t);
    ptr->freesize = size;
    ptr->pnext = (ao_head_t *) pobj->phead;

    phead = ptr;

}

void *aomem_alloc(struct ao_malloc *pobj, ulong size)
{
    ao_head_t *ptr = (ao_head_t *) pobj->phead;
    ulong ret;

    while (ptr && ptr->freesize < size)
        ptr = ptr->pnext;
    if (!ptr)
        return (void *)ptr;
    ptr->freesize -= size;
    ret = ptr->start;
    ptr->start += size;
    ASSERT(ptr->start < (ulong) ptr + size);
    return (void *)ret;
}
