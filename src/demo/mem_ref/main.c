#include "comm.h"
#include "ring.h"
#include "mem_ref.h"
#include "thread_pool.h"

#define NUM (100000)

void *addr[NUM];

int main()
{
    int idx, num;

    mem_ref_init();

    for (idx=0; idx<NUM; idx++) {
        addr[idx] = mem_ref_alloc(1, NULL,
                (mem_alloc_cb_t)mem_alloc,
                (mem_dealloc_cb_t)mem_dealloc);
    }

    for (idx=0; idx<NUM; idx++) {
        for (num=0; num<100; num++) {
            mem_ref_incr(addr[idx]);
        }
    }

    for (idx=0; idx<NUM; idx++) {
        num = mem_ref_check(addr[idx]);
        if (101 != num) {
            assert(0);
        }
        fprintf(stderr, "num:%d\n", num);
    }

    for (idx=NUM; idx>0; idx--) {
        for (num=0; num<100; num++) {
            mem_ref_decr(addr[idx-1]);
        }
    }

#if 1
    for (idx=0; idx<NUM; idx++) {
        num = mem_ref_check(addr[idx]);
        if (1 != num) {
            assert(0);
        }
        fprintf(stderr, "num:%d\n", num);
    }
#endif

    return 0;
}
