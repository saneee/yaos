#include <yaos/types.h>
#include <yaos/percpu.h>
#include <yaos/compiler.h>
#include <yaos/module.h>
#include <yaos/printk.h>
#include <yaos/init.h>
#include <asm/bitops.h>
#include <yaoscall/page.h>
#include <yaos/yaoscall.h>
#include "slub.h"
#define PAGE_4K_MASK 0xfff
#define PAGE_4K_SIZE 0x1000
#define ZERO_SIZE_PTR ((void *)16)
unsigned char *page_bits = NULL;

DECLARE_MODULE(slub_mm, 0, main);
struct freelink {
    void *pnext;
};
bool slub_ok = false;
__percpu_data static struct kmem_cache kmalloc_caches[KMALLOC_SHIFT_HIGH + 1];

static s8 size_index[24] = {
    3,                          /* 8 */
    4,                          /* 16 */
    5,                          /* 24 */
    5,                          /* 32 */
    6,                          /* 40 */
    6,                          /* 48 */
    6,                          /* 56 */
    6,                          /* 64 */
    1,                          /* 72 */
    1,                          /* 80 */
    1,                          /* 88 */
    1,                          /* 96 */
    7,                          /* 104 */
    7,                          /* 112 */
    7,                          /* 120 */
    7,                          /* 128 */
    2,                          /* 136 */
    2,                          /* 144 */
    2,                          /* 152 */
    2,                          /* 160 */
    2,                          /* 168 */
    2,                          /* 176 */
    2,                          /* 184 */
    2                           /* 192 */
};

static struct {
    const char *name;
    unsigned long size;
} const kmalloc_info[] __initconst = {
    {NULL, 0}, {"kmalloc-96", 96},
    {"kmalloc-192", 192}, {"kmalloc-8", 8},
    {"kmalloc-16", 16}, {"kmalloc-32", 32},
    {"kmalloc-64", 64}, {"kmalloc-128", 128},
    {"kmalloc-256", 256}, {"kmalloc-512", 512},
    {"kmalloc-1024", 1024}, {"kmalloc-2048", 2048},
    {"kmalloc-4096", 4096}, {"kmalloc-8192", 8192},
    {"kmalloc-16384", 16384}, {"kmalloc-32768", 32768},
    {"kmalloc-65536", 65536}, {"kmalloc-131072", 131072},
    {"kmalloc-262144", 262144}, {"kmalloc-524288", 524288},
    {"kmalloc-1048576", 1048576}, {"kmalloc-2097152", 2097152},
    {"kmalloc-4194304", 4194304}, {"kmalloc-8388608", 8388608},
    {"kmalloc-16777216", 16777216}, {"kmalloc-33554432", 33554432},
    {"kmalloc-67108864", 67108864}
};

static inline int size_index_elem(size_t bytes)
{
    return (bytes - 1) / 8;
}

struct kmem_cache *kmalloc_slab(size_t size)
{
    int index;

    if (unlikely(size > KMALLOC_MAX_SIZE)) {
        return NULL;
    }

    if (size <= 192) {
        if (!size)
            return ZERO_SIZE_PTR;

        index = size_index[size_index_elem(size)];
    }
    else
        index = fls(size - 1);

    return PERCPU_PTR(&kmalloc_caches[index]);
}

static void __init new_kmalloc_cache(int idx)
{
    struct kmem_cache *p = PERCPU_PTR(&kmalloc_caches[0]);

    p[idx].name = kmalloc_info[idx].name;
    p[idx].size = kmalloc_info[idx].size;
    p[idx].freehead = NULL;
    p[idx].page = NULL;
    p[idx].idx = (uchar) idx;
    ASSERT(idx < 1 << 16);      //page_bits only 4bit 
    printk("Name:%s,size:%d,idx:%d,%lx\n", p[idx].name, p[idx].size, idx, p);
}

static void __init create_kmalloc_caches()
{
    int i;

    for (i = KMALLOC_SHIFT_LOW; i <= KMALLOC_SHIFT_HIGH; i++) {
        new_kmalloc_cache(i);

        /*
         * Caches that are not of the two-to-the-power-of size.
         * These have to be created immediately after the
         * earlier power of two caches
         */
        if (KMALLOC_MIN_SIZE <= 32 && i == 6)
            new_kmalloc_cache(1);
        if (KMALLOC_MIN_SIZE <= 64 && i == 7)
            new_kmalloc_cache(2);
    }
}

/* Loop over all objects in a slab */
#define for_each_object(__p, __s, __addr, __objects) \
        for (__p = (__addr); __p < (__addr) + (__objects) * (__s)->size;\
                        __p += (__s)->size)

#define for_each_object_idx(__p, __idx, __s, __addr, __objects) \
        for (__p = (__addr), __idx = 1; __idx <= __objects;\
                        __p += (__s)->size, __idx++)

/* Determine object index from a given position */
static inline int slab_index(void *p, struct kmem_cache *s, void *addr)
{
    return (p - addr) / s->size;
}

static inline size_t slab_ksize(const struct kmem_cache *s)
{
    return s->size;
}

struct kmem_cache *get_cache_from_addr(void *addr)
{
    ulong offset = (ulong) addr >> PAGE_4K_SHIFT;
    uchar idx;

    if (offset & 1) {
        idx = page_bits[offset / 2] >> 4;
    }
    else {
        idx = page_bits[offset / 2] & 0xf;
    }
    return PERCPU_PTR(&kmalloc_caches[idx]);

}

static void __kmfree(void *p)
{
    if (p == ZERO_SIZE_PTR)
        return;
    struct kmem_cache *s = get_cache_from_addr(p);

    printk("kmfree:%lx,s:%lx,%s\n", p, s, s->name);
    struct freelink *pfree = (struct freelink *)p;

    pfree->pnext = s->freehead;
    s->freehead = pfree;

}

static bool slub_add_page(struct kmem_cache *s)
{
    void *addr = yaos_heap_alloc_4k(PAGE_4K_SIZE);

    printk("s:%lx add page:%lx,size:%lx\n", s, addr, s->size);
    if (!addr)
        return false;
    struct freelink *pfree = (struct freelink *)addr;
    struct freelink *pend = (struct freelink *)(addr + PAGE_4K_SIZE);
    struct freelink *pnext = pfree;

    pnext = (struct freelink *)((ulong) (pfree) + s->size);

    while (pnext < pend) {
        pfree->pnext = pnext;
        pfree = pnext;
        pnext = (struct freelink *)((ulong) (pfree) + s->size);

    }
    pfree->pnext = s->freehead;
    s->freehead = (struct freelink *)addr;
    ulong offset = (ulong) addr >> PAGE_4K_SHIFT;

    ASSERT(s->idx < 16);        // only 4 bit
    if (offset & 1) {
        page_bits[offset / 2] |= s->idx << 4;
    }
    else {
        page_bits[offset / 2] |= s->idx;
    }
    return true;
}

static void *__kmalloc(size_t size)
{
    struct kmem_cache *s = kmalloc_slab(size);

    if (s == ZERO_SIZE_PTR) {
        return ZERO_SIZE_PTR;
    }
    if (!s) {
        return NULL;
    }
    struct freelink *pfree = s->freehead;

    if (!pfree) {
        if (!slub_add_page(s))
            return NULL;
        pfree = s->freehead;
        ASSERT(pfree);
    }
    s->freehead = pfree->pnext;
    return pfree;
}

static ret_t slub_mfree(void *p)
{
    __kmfree(p);
    ret_t ret = { 0, YAOSCALL_OK };
    return ret;
}

static ret_t slub_malloc(size_t size)
{
    void *addr = __kmalloc(size);
    ret_t ret;

    ret.v = (ulong) addr;
    ret.e = ret.v == 0 ? ERR_YAOSCALL_NO_MEM : YAOSCALL_OK;
    return ret;
}

__init static void init_slub_bp()
{
    ulong maxaddr = yaos_max_physical_mem();
    size_t size = maxaddr >> PAGE_4K_SHIFT;

    size /= 2;                  //4bit one 4k page
    size++;
    size &= ~PAGE_4K_MASK;
    size += PAGE_4K_SIZE;
    page_bits = yaos_heap_alloc_4k(size);
    memset(page_bits, 0, size);

}

__init static void init_slub_ap()
{
    create_kmalloc_caches();
    slub_ok = true;

}

static int main(module_t m, ulong t, void *arg)
{
    modeventtype_t env = (modeventtype_t) t;

    if (env == MOD_BPLOAD) {
        slub_ok = false;
        init_slub_bp();
        init_slub_ap();
        regist_yaoscall(YAOS_malloc, slub_malloc);
        regist_yaoscall(YAOS_mfree, slub_mfree);

        printk("slub_mm, %lx,%lx,%lx\n", m, t, arg);
    }
    else if (env == MOD_APLOAD) {
        printk("slub_mm,ap load\n");
        if (!is_bp()) {//bp already done in bpload
            init_slub_ap();
        }
    }
    return MOD_ERR_OK;          //MOD_ERR_NEXTLOOP;
}
