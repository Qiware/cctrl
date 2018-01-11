#include "comm.h"
#include "ring.h"
#include "mref.h"
#include "queue.h"
#include "thread_pool.h"

#define LEN (5 * MB)

queue_t *q;
int NUM = 1;
int MOD = 1;

typedef struct
{
    int idx;
    void *base;
    void *data;
} item_t;

void *productor(void *arg)
{
    int n;
    void *addr;
    item_t *item;

    for (;;) {
        addr = mref_alloc(LEN, NULL,
                (mem_alloc_cb_t)mem_alloc,
                (mem_dealloc_cb_t)mem_dealloc);
        for (n=0; n<LEN; n+=(random()%MOD + 1)) {
            item = queue_malloc(q, sizeof(item_t));
            if (NULL == item) {
                continue;
            }
            item->idx = n;
            item->base = addr;
            item->data = addr + n;
            mref_inc(item->base);

            queue_push(q, item);
        }
        mref_dec(addr);
    }
    return NULL;
}

void *customer(void *arg)
{
    int num, idx;
    item_t *item[1024];

    while (1) {
        num = MIN(queue_used(q), 1024);

        num = queue_mpop(q, (void **)item, num);
        if (0 == num) {
            usleep(0);
            continue;
        }

        fprintf(stderr, "Pop num:%d!\n", num);

        for (idx=0; idx<num; idx++) {
            fprintf(stderr, "Mem ref:%d!\n", mref_check(item[idx]->base));
            mref_dec(item[idx]->base);
            fprintf(stderr, "Mem ref:%d!\n", mref_check(item[idx]->base));
            queue_dealloc(q, item[idx]);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    thread_pool_t *t;

    if (3 != argc) {
        fprintf(stderr, "Paramter isn't right!\n");
        return -1;
    }

    NUM = atoi(argv[1]);
    MOD = atoi(argv[2]);

    mref_init();

    q = queue_creat(100000, sizeof(item_t));

    t = thread_pool_init(5, NULL, NULL);
    if (NULL == t) {
        fprintf(stderr, "Init thread pool failed!\n");
        return -1;
    }

    thread_pool_add_worker(t, customer, NULL);
    thread_pool_add_worker(t, customer, NULL);

    thread_pool_add_worker(t, productor, NULL);
    thread_pool_add_worker(t, productor, NULL);
    thread_pool_add_worker(t, productor, NULL);

    while (1) {
        sleep(1);
    }

    return 0;
}
