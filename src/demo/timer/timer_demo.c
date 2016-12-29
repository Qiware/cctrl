/******************************************************************************
 ** Coypright(C) 2014-2024 Qiware technology Co., Ltd
 **
 ** 文件名: timer_test.c
 ** 版本号: 1.0
 ** 描  述: 
 ** 作  者: # Qifeng.zou # 2016年01月08日 星期五 09时02分24秒 #
 ******************************************************************************/
#include "comm.h"
#include "timer.h"

void proc(void *idx)
{
    time_t ctm = time(NULL);
    fprintf(stderr, "%s: tm:%lu idx:%lu\n", __func__, ctm, (uint64_t)idx);
}

int main(void)
{
    int idx;
    timer_cntx_t *timer;
    timer_task_t task[10];

    timer = timer_cntx_init();
    if (NULL == timer) {
        fprintf(stderr, "Initialize timer context failed!");
        return -1;
    }

    for (idx=0; idx<10; idx+=1) {
        timer_task_init(&task[idx], proc, 1, 2, (void *)((uint64_t)idx));

        timer_task_add(timer, &task[idx]);
    }

    timer_task_routine((void *)timer);

    return 0;
}

