#ifndef _YAOS_CONCURRENT_QUEUE_H
#define _YAOS_CONCURRENT_QUEUE_H
struct con_queue_node_t;
struct con_queue_node_t {
    struct con_queue_node_t *pnext;
};
struct con_queue_t {
    struct con_queue_node_t *phead;
    struct con_queue_node_t *ptail;
};
static inline void init_con_queue(struct con_queue_t *q,
                                  struct con_queue_node_t *first)
{
    first->pnext = NULL;
    q->phead = q->ptail = first;
}

static inline void enqueue(struct con_queue_t *q, struct con_queue_node_t *node)
{
    while (true) {
        con_queue_node_t *tail, *next;

        tail = q->ptail;
        next = tail->pnext;
        if (tail == q->tail) {
            if (next == NULL) {
                if (cmpxchg(&tail->pnext, next, node))
                    break;
            }
            else {
                cmpxchg(&q->tail, tail, next);
            }
        }
    }
    cmpxchg(&q->tail, tail, node);
}

static inline void *dequeue(struct non_queue_t *q)
{
    struct con_queue_node_t *head;
    struct con_queue_node_t *tail;
    struct con_queue_node_t *next;

    while (true) {
        head = q->phead;
        tail = q->ptail;
        next = head->pnext;
        if (head == q->phead) {
            if (head == tail) {
                if (next == NULL)
                    return NULL;
                cmpxchg(&q->tail, tail, next);
            }
            else {
            if (cmpxchg(&q->phead, head, next) break;}
                }
                }
                return head;}
#endif
