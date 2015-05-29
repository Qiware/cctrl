#include "sck.h"
#include "comm.h"
#include "search.h"
#include "syscall.h"
#include "prob_agent.h"
#include "prob_listen.h"

prob_listen_t *prob_listen_init(prob_cntx_t *ctx);
static int prob_listen_accept(prob_cntx_t *ctx, prob_listen_t *lsn);

/******************************************************************************
 **函数名称: prob_listen_routine
 **功    能: 运行侦听线程
 **输入参数:
 **     _ctx: 全局信息
 **输出参数: NONE
 **返    回: 0:成功 !0:失败
 **实现描述: 
 **     1. 初始化侦听线程
 **     2. 接收网络连接
 **注意事项: 
 **作    者: # Qifeng.zou # 2014.11.18 #
 ******************************************************************************/
void *prob_listen_routine(void *_ctx)
{
    int ret, max;
    fd_set rdset;
    struct timeval tv;
    prob_listen_t *lsn;
    prob_cntx_t *ctx = (prob_cntx_t *)_ctx;

    /* 1. 初始化侦听线程 */
    lsn = prob_listen_init(ctx);
    if (NULL == lsn)
    {
        log_error(ctx->log, "Initialize listen thread failed!");
        return (void *)-1;
    }

    /* 2. 接收网络连接  */
    while (1)
    {
        FD_ZERO(&rdset);

        FD_SET(lsn->lsn_sck_id, &rdset);
        FD_SET(lsn->cmd_sck_id, &rdset);

        max = MAX(lsn->lsn_sck_id, lsn->cmd_sck_id);

        /* 2.1 等待事件通知 */
        tv.tv_sec = 30;
        tv.tv_usec = 0;
        ret = select(max+1, &rdset, NULL, NULL, &tv);
        if (ret < 0)
        {
            if (EINTR == errno)
            {
                continue;
            }

            log_error(lsn->log, "errmsg:[%d] %s!", errno, strerror(errno));
            continue;
        }
        else if (0 == ret)
        {
            continue;
        }

        /* 2.2 接收网络连接 */
        if (FD_ISSET(lsn->lsn_sck_id, &rdset))
        {
            prob_listen_accept(ctx, lsn);
        }

        /* 2.3 接收操作命令 */
        if (FD_ISSET(lsn->cmd_sck_id, &rdset))
        {
        }
    }
    return (void *)0;
}

/******************************************************************************
 **函数名称: prob_listen_init
 **功    能: 初始化侦听线程
 **输入参数:
 **     ctx: 全局信息
 **输出参数: NONE
 **返    回: LSN对象
 **实现描述: 
 **     1. 创建LSN对象
 **     2. 侦听指定端口
 **     3. 创建命令套接字
 **注意事项: 
 **作    者: # Qifeng.zou # 2014.11.19 #
 ******************************************************************************/
prob_listen_t *prob_listen_init(prob_cntx_t *ctx)
{
    prob_listen_t *lsn;

    /* 1. 创建LSN对象 */
    lsn = (prob_listen_t *)calloc(1, sizeof(prob_listen_t));
    if (NULL == lsn)
    {
        log_error(lsn->log, "errmsg:[%d] %s!", errno, strerror(errno));
        return NULL;
    }

    lsn->log = ctx->log;

    do
    {
        /* 2. 侦听指定端口 */
        lsn->lsn_sck_id = tcp_listen(ctx->conf->connections.port);
        if (lsn->lsn_sck_id < 0)
        {
            log_error(lsn->log, "errmsg:[%d] %s! port:%d",
                    errno, strerror(errno), ctx->conf->connections.port);
            break;
        }

        /* 3. 创建命令套接字 */
        lsn->cmd_sck_id = unix_udp_creat(PROB_LSN_CMD_PATH);
        if (lsn->cmd_sck_id < 0)
        {
            log_error(lsn->log, "errmsg:[%d] %s!", errno, strerror(errno));
            break;
        }
        
        return lsn;
    } while(0);

    CLOSE(lsn->lsn_sck_id);
    CLOSE(lsn->cmd_sck_id);
    free(lsn);
    return NULL;
}

/******************************************************************************
 **函数名称: prob_listen_accept
 **功    能: 接收连接请求
 **输入参数:
 **     ctx: 全局信息
 **     lsn: 侦听对象
 **输出参数: NONE
 **返    回: 0:成功 !0:失败
 **实现描述: 
 **     1. 接收连接请求
 **     2. 将通信套接字放入队列
 **注意事项: 
 **作    者: # Qifeng.zou # 2014.11.20 #
 ******************************************************************************/
static int prob_listen_accept(prob_cntx_t *ctx, prob_listen_t *lsn)
{
    int fd, tidx;
    prob_add_sck_t *add;
    struct sockaddr_in cliaddr;

    /* 1. 接收连接请求 */
    fd = tcp_accept(lsn->lsn_sck_id, (struct sockaddr *)&cliaddr);
    if (fd < 0)
    {
        log_error(lsn->log, "errmsg:[%d] %s!", errno, strerror(errno));
        return PROB_ERR;
    }

    ++lsn->serial;

    /* 2. 将通信套接字放入队列 */
    tidx = lsn->serial % ctx->conf->agent_num;

    add = queue_malloc(ctx->connq[tidx]);
    if (NULL == add)
    {
        log_error(lsn->log, "Alloc memory from queue failed! fd:%d", fd);
        CLOSE(fd);
        return PROB_ERR;
    }

    add->fd = fd;
    add->serial = lsn->serial;

    log_debug(lsn->log, "Push data! fd:%d addr:%p", fd, add);

    if (queue_push(ctx->connq[tidx], add))
    {
        log_error(lsn->log, "Push into queue failed! fd:%d", fd);
        queue_dealloc(ctx->connq[tidx], add);
        CLOSE(fd);
        return PROB_ERR;
    }

    return PROB_OK;
}