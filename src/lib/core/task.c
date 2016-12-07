#include "comm.h"
#include "list.h"
#include "task.h"

/* 比较回调 */
static int task_cmp_cb(task_item_t *item1, task_item_t *item2)
{
    return item1->seq - item2->seq;
}

/******************************************************************************
 **函数名称: task_init
 **功    能: 初始化定时任务表
 **输入参数: NONE
 **输出参数: NONE
 **返    回: 任务管理对象
 **实现描述: 初始化读写锁、创建红黑树
 **注意事项: 
 **作    者: # Qifeng.zou # 2016.12.06 22:34:25 #
 ******************************************************************************/
task_t *task_init(void)
{
    task_t *task;

    task = (task_t *)calloc(1, sizeof(task_t));
    if (NULL == task) {
        return NULL;
    }

    task->seq = 0;
    pthread_rwlock_init(&task->lock, NULL);
    task->list = rbt_creat(NULL, (cmp_cb_t)task_cmp_cb);
    if (NULL == task->list) {
        pthread_rwlock_destroy(&task->lock);
        free(task);
        return NULL;
    }

    return NULL;
}

/******************************************************************************
 **函数名称: task_add
 **功    能: 增加定时任务
 **输入参数:
 **     task: 任务表
 **     proc: 任务回调
 **     start: 第一次执行的间隔
 **     interval: 执行执行的间隔
 **     param: 附件参数
 **输出参数:
 **返    回: VOID
 **实现描述: 
 **注意事项: 
 **作    者: # Qifeng.zou # 2016.12.06 21:57:30 #
 ******************************************************************************/
int task_add(task_t *task, void (*proc)(void *param), int start, int interval, int times, void *param)
{
    task_item_t *item;

    item = (task_item_t *)calloc(1, sizeof(task_item_t));
    if (NULL == item) {
        return -1;
    }

    item->interval = interval;
    item->is_limit = (0 == times)? false : true;
    item->times = times;
    item->last = (0 == start)? 0 : time(NULL);

    item->proc = proc;
    item->param = param;

    /* 插入定时任务表 */
    pthread_rwlock_wrlock(&task->lock);
    item->seq = ++task->seq;
    if (rbt_insert(task->list, item)) {
        pthread_rwlock_unlock(&task->lock);
        free(item);
        return 0;
    }
    pthread_rwlock_unlock(&task->lock);

    return 0;
}

/******************************************************************************
 **函数名称: task_exec_cb
 **功    能: 执行定时任务
 **输入参数:
 **     item: 定时任务
 **     timeout_list: 超时列表
 **输出参数:
 **返    回: VOID
 **实现描述: 
 **注意事项: 
 **作    者: # Qifeng.zou # 2016.12.06 21:57:30 #
 ******************************************************************************/
static int task_exec_cb(task_item_t *item, list_t *timeout_list)
{
    time_t ctm = time(NULL);

    if ((ctm - item->last) >= item->interval) {
        item->proc(item->param); // 执行任务
        item->last = ctm;
        if (item->is_limit) {
            --item->times;
            if (0 == item->times) {
                list_rpush(timeout_list, (void *)item->seq);
            }
        }
    }

    return 0;
}

/******************************************************************************
 **函数名称: task_routine
 **功    能: 定时任务处理
 **输入参数:
 **     task: 定时任务
 **输出参数:
 **返    回: VOID
 **实现描述: 
 **注意事项: 
 **作    者: # Qifeng.zou # 2016.12.06 21:57:30 #
 ******************************************************************************/
void *task_routine(void *_task)
{
    void *addr;
    list_t *timeout_list;
    task_item_t *item, key;
    task_t *task = (task_t *)_task;

    timeout_list = list_creat(NULL);
    if (NULL == timeout_list) {
        return (void *)-1;
    }

    for (;;) {
        /* > 执行定时任务 */
        pthread_rwlock_rdlock(&task->lock);
        rbt_trav(task->list, (trav_cb_t)task_exec_cb, (void *)timeout_list);
        pthread_rwlock_unlock(&task->lock);

        /* > 清理定时任务 */
        pthread_rwlock_wrlock(&task->lock);
        for (;;) {
            addr = (void *)list_lpop(timeout_list);
            if (NULL == addr) {
                break;
            }

            key.seq = (uint64_t)((uint64_t *)addr);

            rbt_delete(task->list, (void *)&key, (void **)&item);
            if (NULL == item) {
                continue;
            }
            free(item);
        }
        pthread_rwlock_unlock(&task->lock);
        sleep(1);
    }

    return NULL;
}
