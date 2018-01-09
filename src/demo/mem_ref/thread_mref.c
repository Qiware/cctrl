#include "comm.h"
#include "ring.h"
#include "mem_ref.h"
#include "thread_pool.h"

#define NUM (1000)

void *addr[NUM];
ring_t *q;

void *productor(void *arg)
{
    int idx, n;

    for (idx=0; idx<NUM; idx++) {
        for (n=0; n<100; n++) {
            mem_ref_incr(addr[idx]);
            if (ring_push(q, addr[idx])) {
                mem_ref_decr(addr[idx]);
            }
        }
    }
    return NULL;
}

void *customer(void *arg)
{
    void *data[1024];
    int num, idx, cnt;

    while (1) {
        num = MIN(ring_used(q), 1024);

        num = ring_mpop(q, data, num);
        if (0 == num) {
            sleep(1);
            continue;
        }

        for (idx=0; idx<num; idx++) {
            mem_ref_decr(data[idx]);
        }

        for (idx=0; idx<NUM; idx++) {
            cnt = mem_ref_check(addr[idx]);
            fprintf(stderr, "Idx:%d Count:%d\n", idx, cnt);
        }
    }
    return NULL;
}

int main()
{
    int idx;
    thread_pool_t *t;

    mem_ref_init();

    for (idx=0; idx<NUM; idx++) {
        addr[idx] = mem_ref_alloc(1024, NULL,
                (mem_alloc_cb_t)mem_alloc,
                (mem_dealloc_cb_t)mem_dealloc);
    }

    q = ring_creat(1000000000);

    t = thread_pool_init(5, NULL, NULL);
    if (NULL == t) {
        fprintf(stderr, "Init thread pool failed!\n");
        return -1;
    }

    thread_pool_add_worker(t, customer, addr);
    thread_pool_add_worker(t, customer, addr);

    thread_pool_add_worker(t, productor, addr);
    thread_pool_add_worker(t, productor, addr);
    thread_pool_add_worker(t, productor, addr);

    while (1) {
        sleep(1);
    }

    return 0;
}
