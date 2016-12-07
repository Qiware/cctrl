#if !defined(__TASK_H__)
#define __TASK_H__

#include "comm.h"
#include "rb_tree.h"

/* 定时任务项 */
typedef struct
{
    uint64_t seq;                   /* 序列号 */

    int interval;                   /* 每次执行的间隔 */
    bool is_limit;                  /* 执行次数是否有限制 */
    int times;                      /* 执行次数 */
    time_t last;                    /* 上一次执行的时间 */

    void (*proc)(void *param);      /* 处理回调 */
    void *param;                    /* 附加参数 */
} task_item_t;

/* 定时任务 */
typedef struct
{
    uint64_t seq;                   /* 序列号 */
    pthread_rwlock_t lock;          /* 读写锁 */
    rbt_tree_t *list;               /* 定时任务表 */
} task_t;

task_t *task_init(void);
int task_add(task_t *task, void (*proc)(void *param), int start, int interval, int times, void *param);
void *task_routine(void *_task);

#endif /*__TASK_H__*/
