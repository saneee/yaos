#include <yaos/types.h>
#include <yaos/init.h>
#include <yaos/printk.h>
#include <yaos/irq.h>
#include <yaos/assert.h>
#include <yaos/cpupm.h>
#include <yaos/list.h>
#include <yaos/llist.h>

#define NR_CALL_BUF 16
//try give a chance to timer,etc
struct callback;
struct callback {
    struct callback *pnext;
    void (*callback) (void);
};
static struct callback *phead = NULL;
static struct callback callbuf[NR_CALL_BUF];
static struct callback *pfree = NULL;
void sched_yield(void)
{
    struct callback *p = phead;
    void timer_sched_callback(void);

    while (p) {
        (p->callback) ();
        ASSERT(p != p->pnext);
        p = p->pnext;
    }
}

void register_yield_callback(void (*pfunc) (void))
{
    struct callback *p = pfree;

    if (unlikely(!p))
        panic("NR_CALL_BUF too small,%s:%d\n", __func__, __LINE__);
    pfree = p->pnext;
    p->callback = pfunc;
    p->pnext = phead;
    phead = p;
}

__init int init_sched(bool isbp)
{
    if (!isbp)
        return 0;
    phead = NULL;
    for (int i = 0; i < NR_CALL_BUF - 1; i++) {
        callbuf[i].pnext = &callbuf[i + 1];
    }
    callbuf[NR_CALL_BUF - 1].pnext = NULL;

    pfree = &callbuf[0];
    return 0;
}
