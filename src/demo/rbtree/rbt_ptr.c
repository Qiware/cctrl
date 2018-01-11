/******************************************************************************
 ** Coypright(C) 2014-2024 Qiware technology Co., Ltd
 **
 ** 文件名: rbt_test.c
 ** 版本号: 1.0
 ** 描  述: 
 ** 作  者: # Qifeng.zou # 2016年01月08日 星期五 09时02分24秒 #
 ******************************************************************************/
#include "comm.h"
#include "list.h"
#include "rb_tree.h"
#include "hash_tab.h"

typedef struct
{
    void *addr;
} item_t;

void rbt_trav_print_hdl(item_t *item, void *args)
{
    fprintf(stderr, "addr:%p\n", item->addr);
}

void rbt_print_cb(item_t *item)
{
    fprintf(stderr, "addr:%p", item->addr);
}

static int64_t rbt_item_cmp_cb(const item_t *item1, const item_t *item2)
{
    return (int64_t)(item1->addr - item2->addr);
}

int main(int argc, char *argv[])
{
    rbt_tree_t *rbt;
    item_t *item, key;

    rbt = rbt_creat(NULL, (cmp_cb_t)rbt_item_cmp_cb);
    if (NULL == rbt) {
        return -1;
    }

    // 1
    item = (item_t *)calloc(1, sizeof(item_t));

    item->addr = (void *)0x7f6c0c000030;

    rbt_insert(rbt, item);

    // 2
    item = (item_t *)calloc(1, sizeof(item_t));

    item->addr = (void *)0x7f6bc5900080;

    rbt_insert(rbt, item);

    // 3
    fprintf(stderr, "Line:%d\n", __LINE__);
    rbt_print(rbt, (print_cb_t)rbt_print_cb);

    key.addr = (void *)0x7f6c0c000030;
    rbt_delete(rbt, &key, (void **)&item);
    if ((NULL == item) || (item->addr != key.addr)) {
        assert(0);
    }

    // 4
    item = (item_t *)calloc(1, sizeof(item_t));
    item->addr = (void *)0x7f6b7f2000d0;

    rbt_insert(rbt, item);

    // 5
    item = (item_t *)calloc(1, sizeof(item_t));

    item->addr = (void *)0x7f6c0c000030;

    rbt_insert(rbt, item);

    // 6
    key.addr = (void *)0x7f6bc5900080;
    rbt_delete(rbt, &key, (void **)&item);
    if ((NULL == item) || (item->addr != key.addr)) {
        assert(0);
    }

    // 6
    key.addr = (void *)0x7f6c0c000030;
    rbt_delete(rbt, &key, (void **)&item);
    if ((NULL == item) || (item->addr != key.addr)) {
        assert(0);
    }

    // 7
    item = (item_t *)calloc(1, sizeof(item_t));

    item->addr = (void *)0x7f6c0c000030;

    rbt_insert(rbt, item);

    // 8
    key.addr = (void *)0x7f6b7f2000d0;
    rbt_delete(rbt, &key, (void **)&item);
    if ((NULL == item) || (item->addr != key.addr)) {
        assert(0);
    }

    // 7
    item = (item_t *)calloc(1, sizeof(item_t));

    item->addr = (void *)0x7f6b7f2000d0;

    rbt_insert(rbt, item);

    // 8
    key.addr = (void *)0x7f6c0c000030;
    rbt_delete(rbt, &key, (void **)&item);
    if ((NULL == item) || (item->addr != key.addr)) {
        assert(0);
    }

    // 9
    item = (item_t *)calloc(1, sizeof(item_t));

    item->addr = (void *)0x7f6c0c000030;

    rbt_insert(rbt, item);

    // 10
    key.addr = (void *)0x7f6b7f2000d0;
    rbt_delete(rbt, &key, (void **)&item);
    if ((NULL == item) || (item->addr != key.addr)) {
        assert(0);
    }

    // 11
    key.addr = (void *)0x7f6c0c000030;
    rbt_delete(rbt, &key, (void **)&item);
    if ((NULL == item) || (item->addr != key.addr)) {
        assert(0);
    }

    // 12
    item = (item_t *)calloc(1, sizeof(item_t));

    item->addr = (void *)0x7f6b7f2000d0;

    rbt_insert(rbt, item);

    // 13
    item = (item_t *)calloc(1, sizeof(item_t));

    item->addr = (void *)0x7f6c0c000030;

    rbt_insert(rbt, item);

    // 14
    key.addr = (void *)0x7f6b7f2000d0;
    rbt_delete(rbt, &key, (void **)&item);
    if ((NULL == item) || (item->addr != key.addr)) {
        assert(0);
    }

    // 15
    item = (item_t *)calloc(1, sizeof(item_t));

    item->addr = (void *)0x7f6b7f2000d0;

    rbt_insert(rbt, item);

    // 16
    key.addr = (void *)0x7f6c0c000030;
    rbt_delete(rbt, &key, (void **)&item);
    if ((NULL == item) || (item->addr != key.addr)) {
        assert(0);
    }

    fprintf(stderr, "Line:%d\n", __LINE__);
    rbt_print(rbt, (print_cb_t)rbt_print_cb);

    // 17
    item = (item_t *)calloc(1, sizeof(item_t));

    item->addr = (void *)0x7f6c0c000030;

    rbt_insert(rbt, item);

    fprintf(stderr, "Line:%d\n", __LINE__);
    rbt_print(rbt, (print_cb_t)rbt_print_cb);

    // 18
    item = (item_t *)calloc(1, sizeof(item_t));

    item->addr = (void *)0x7f6bc5900080;

    rbt_insert(rbt, item);

    fprintf(stderr, "Line:%d\n", __LINE__);
    rbt_print(rbt, (print_cb_t)rbt_print_cb);

    // 19
    key.addr = (void *)0x7f6b7f2000d0;
    rbt_delete(rbt, &key, (void **)&item);
    if ((NULL == item) || (item->addr != key.addr)) {
        assert(0);
    }

    fprintf(stderr, "Line:%d\n", __LINE__);
    rbt_print(rbt, (print_cb_t)rbt_print_cb);

    // 20
    key.addr = (void *)0x7f6c0c000030;
    rbt_delete(rbt, &key, (void **)&item);
    if ((NULL == item) || (item->addr != key.addr)) {
        assert(0);
    }

    // finish
    rbt_print(rbt, (print_cb_t)rbt_print_cb);

    return 0;
}
