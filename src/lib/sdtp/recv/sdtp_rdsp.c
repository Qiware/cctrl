/******************************************************************************
 ** Coypright(C) 2014-2024 Xundao technology Co., Ltd
 **
 ** 文件名: sdtp_rdsp.c
 ** 版本号: 1.0
 ** 描  述: 负责接收端发送数据的分发处理
 ** 作  者: # Qifeng.zou # Fri 15 May 2015 11:45:31 PM CST #
 ******************************************************************************/
#include "mesg.h"
#include "sdtp_cmd.h"
#include "sdtp_comm.h"
#include "sdtp_recv.h"

/******************************************************************************
 **函数名称: sdtp_disp_routine
 **功    能: 运行分发线程
 **输入参数:
 **     ctx: 全局信息
 **输出参数: NONE
 **返    回: 分发对象
 **实现描述:
 **注意事项:
 **作    者: # Qifeng.zou # 2015.05.15 #
 ******************************************************************************/
void *sdtp_disp_routine(void *_ctx)
{
    int idx;
    void *addr, *addr2;
    mesg_route_t *route;
    sdtp_rctx_t *ctx = (sdtp_rctx_t *)_ctx;

    while (1)
    {
        /* > 弹出发送数据 */
        addr = shm_queue_pop(ctx->shm_sendq);
        if (NULL == addr)
        {
            usleep(500); /* TODO: 可使用消息机制减少CPU的消耗 */
            continue;
        }

        /* > 获取发送队列 */
        route = (mesg_route_t *)addr;

        idx = sdtp_dev_svr_map_rand(ctx, route->dest_devid);

        /* > 获取发送队列 */
        addr2 = queue_malloc(ctx->sendq[idx]);
        if (NULL == addr2)
        {
            shm_queue_dealloc(ctx->shm_sendq, addr);
            continue;
        }

        memcpy(addr2, addr, route->length);

        if (queue_push(ctx->sendq[idx], addr2))
        {
            queue_dealloc(ctx->sendq[idx], addr2);
        }

        shm_queue_dealloc(ctx->shm_sendq, addr);
    }

    return (void *)-1;
}
