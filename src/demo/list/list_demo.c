#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "list.h"
#include "redo.h"
#include "rb_tree.h"

#define LIST_NUM    (1000000)

typedef struct
{
    int idx;
} list_data_t;

static int rbt_cmp_cb(list_data_t *d1, list_data_t *d2)
{
    return d1->idx - d2->idx;
}

int main(void)
{
    int idx, total;
    list_t *list;
    list_data_t *data, key;
    rbt_tree_t *rbt;

    list = list_creat(NULL);
    rbt = rbt_creat(NULL, (cmp_cb_t)rbt_cmp_cb);

    total = 0;
    for (idx=0; idx<LIST_NUM; ++idx) {
        data = (list_data_t *)calloc(1, sizeof(list_data_t));
        if (NULL == data) {
            fprintf(stderr, "errmsg:[%d] %s!", errno, strerror(errno));
            return -1;
        }

        data->idx = idx;

        list_rpush(list, data);
        rbt_insert(rbt, data);
        ++total;
    }

    /* 删除测试 */
    for (idx=LIST_NUM-1; idx >=0; --idx) {
        key.idx = idx;
        rbt_delete(rbt, &key, (void **)&data);
        if (NULL == data) {
            assert(0);
        }
        list_remove(list, data);
        //fprintf(stderr, "Delete idx:%d\n", data->idx);
        free(data);
    }

    while (1) {
        data = list_lpop(list);
        if (NULL == data) {
            break;
        }
        fprintf(stdout, "Left data! idx:%d\n", data->idx);
        free(data);
    }

    return 0;
}
