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

#define SID_BEG     (0x317fd0640)
#define NUM         (10000)

typedef struct
{
    uint64_t *id;
    uint32_t flag;
} rbt_data_t;

void rbt_trav_print_hdl(rbt_data_t *data, void *args)
{
    static uint32_t idx = 0;

    fprintf(stderr, "idx:%u id:%p\n", ++idx, data->id);
}

void rbt_print_cb(rbt_data_t *data)
{
    fprintf(stderr, "id:%p", data->id);
}

static int rbt_data_cmp_cb(const rbt_data_t *data1, const rbt_data_t *data2)
{
    return (int)((uint64_t)data1->id - (uint64_t)data2->id);
}

#if 1
static int rbt_trav_insert_list(rbt_data_t *data, list_t *list)
{
    if (!data->flag) {
        assert(0);
    }
    if ((uint64_t)data->id%9) {
        if (rand()%2) {
            list_rpush(list, data);
        } else {
            list_lpush(list, data);
        }
    }
    return 0;
}

int main(void)
{
    int idx, list_len;
    list_t *list;
    rbt_tree_t *rbt;
    rbt_data_t *data, *temp;

    rbt = rbt_creat(NULL, (cmp_cb_t)rbt_data_cmp_cb);
    if (NULL == rbt) {
        return -1;
    }

    do {
        list = list_creat(NULL);
        if (NULL == list) {
            return -1;
        }

        for (idx=0; idx<NUM; ++idx) {
            data = (rbt_data_t *)calloc(1, sizeof(rbt_data_t));
            data->id = (uint64_t *)(SID_BEG + idx);
            if (rbt_insert(rbt, data)) {
                free(data);
            }
            data->flag = 1;
        }

        rbt_trav(rbt, (trav_cb_t)rbt_trav_insert_list, (void *)list);

        list_len = list_length(list);
        while ((data = (rbt_data_t *)list_lpop(list))) {
            temp = NULL;
            rbt_delete(rbt, (void *)data, (void **)&temp);
            if (NULL == temp || data != temp) {
                assert(0);
            }
            data->flag = 0;
            free(data);
        }


        if (NUM != list_len + rbt_num(rbt)) {
            assert(0);
        }

        list_destroy(list, (mem_dealloc_cb_t)mem_dummy_dealloc, NULL);

        ////////////////////////////
        list = list_creat(NULL);
        if (NULL == list) {
            return -1;
        }

        rbt_trav(rbt, (trav_cb_t)rbt_trav_insert_list, (void *)list);

        list_len = list_length(list);
        while ((data = (rbt_data_t *)list_lpop(list))) {
            temp = NULL;
            rbt_delete(rbt, (void *)data, (void **)&temp);
            if (NULL == temp || data != temp) {
                assert(0);
            }
            data->flag = 0;
            free(data);
        }

        list_destroy(list, (mem_dealloc_cb_t)mem_dummy_dealloc, NULL);
    } while(1);

    return 0;
}
#else
static uint64_t hash_cb(const rbt_data_t *data)
{
    return data->id;
}

static int htab_trav_insert_rbt(rbt_data_t *data, rbt_tree_t *rbt)
{
    rbt_insert(rbt, data);
    return 0;
}

static int htab_trav_delete_rbt(rbt_data_t *data, rbt_tree_t *rbt)
{
    rbt_data_t *item;

    if (0 == data->id % 3) {
        item = rbt_query(rbt, data);
        if (NULL == item) {
            assert(0);
        }

        rbt_delete(rbt, (void *)data, (void **)&item);
        if (NULL == item) {
            assert(0);
        }

        item = rbt_query(rbt, data);
        if (NULL != item) {
            assert(0);
        }
    }

    return 0;
}
int main(void)
{
    int idx, n;
    uint64_t last;
    rbt_tree_t *rbt;
    hash_tab_t *tab;
    rbt_data_t *data, key;

    tab = hash_tab_creat(555, (hash_cb_t)hash_cb, (cmp_cb_t)rbt_data_cmp_cb, NULL);
    if (NULL == tab) {
        return -1;
    }

    for (idx=0; idx<NUM; ++idx) {
        data = (rbt_data_t *)calloc(1, sizeof(rbt_data_t));
        data->id = SID_BEG + idx;
        hash_tab_insert(tab, data, NONLOCK);
    }

    rbt = rbt_creat(NULL, (cmp_cb_t)rbt_data_cmp_cb);
    if (NULL == rbt) {
        return -1;
    }

    hash_tab_trav(tab, (trav_cb_t)htab_trav_insert_rbt, rbt, NONLOCK);
    hash_tab_trav(tab, (trav_cb_t)htab_trav_delete_rbt, rbt, NONLOCK);

    n = 0;
    last = SID_BEG - 1;
    for (idx=0; idx<NUM; ++idx) {
        key.id = SID_BEG + idx;
        data = rbt_query(rbt, &key);
        if (NULL == data) {
            continue;
            assert(0);
        } else if (last + 1 != data->id) {
            //assert(0);
        }
        last = data->id;
        fprintf(stderr, "idx:%d data:%p id:%lu\n", ++n, data, data->id);
    }

    //rbt_trav(rbt, (trav_cb_t)rbt_trav_print_hdl, NULL);
    //rbt_print(rbt, (print_cb_t)rbt_print_cb);


    return 0;
}
#endif
