#include <signal.h>
#include <sys/time.h>

#include "mesg.h"
#include "mref.h"
#include "search.h"
#include "syscall.h"
#include "test_mesg.h"
#include "rtmq_proxy.h"

#define __RTMQ_DEBUG_SEND__

static int rtmq_proxy_def_handler(int type, int nid, char *buff, size_t len, void *args);

/******************************************************************************
 **函数名称: rtmq_send_debug 
 **功    能: 发送端调试
 **输入参数: 
 **     cli: 上下文信息
 **输出参数: NONE
 **返    回: 0:Success !0:Failed
 **实现描述: 
 **注意事项: 
 **作    者: # Qifeng.zou # 2014.03.26 #
 ******************************************************************************/
#define LOOP        (10000)
#define USLEEP      (10)
#define SIZE        (4096)

int rtmq_send_debug(rtmq_proxy_t *ctx, int secs)
{
    char data[SIZE];
    size_t idx = 0, n = 0, len;
    double sleep2 = 0;
    struct timeval stime, etime;
    int total = 0, fails = 0;

    for (;;) {
        ++n;
        gettimeofday(&stime, NULL);
        sleep2 = 0;
        fails = 0;
        total = 0;
        for (idx=0; idx<LOOP; idx++) {
            len = snprintf(data, sizeof(data), "BAIDU%lu-%lu", n, idx);

            if (rtmq_proxy_async_send(ctx, MSG_SEARCH_REQ, data, len + rand()%1024)) {
                idx--;
                usleep(2);
                sleep2 += USLEEP*1000000;
                ++fails;
                continue;
            }

            total++;
        }

        gettimeofday(&etime, NULL);
        if (etime.tv_usec < stime.tv_usec) {
            etime.tv_sec--;
            etime.tv_usec += 1000000;
        }

        fprintf(stderr, "%s() %s:%d\n"
                "\tstime:%ld.%06ld etime:%ld.%06ld spend:%ld.%06ld\n"
                "\tTotal:%d fails:%d\n",
                __func__, __FILE__, __LINE__,
                stime.tv_sec, stime.tv_usec,
                etime.tv_sec, etime.tv_usec,
                etime.tv_sec - stime.tv_sec,
                etime.tv_usec - stime.tv_usec,
                total, fails);
        sleep(secs);
    }

    pause();

    return 0;
}

static void rtmq_setup_conf(rtmq_proxy_conf_t *conf, int port)
{
    conf->nid = 1;
    conf->gid = 1;

    snprintf(conf->auth.usr, sizeof(conf->auth.usr), "qifeng");
    snprintf(conf->auth.passwd, sizeof(conf->auth.passwd), "111111");

    snprintf(conf->ipaddr, sizeof(conf->ipaddr), "127.0.0.1:%d", port);

    conf->send_thd_num = 2;
    conf->work_thd_num = 4;
    conf->recv_buff_size = 2 * MB;

    conf->sendq.max = 2048;
    conf->sendq.size = 4096;

    conf->recvq.max = 2048;
    conf->recvq.size = 4096;
}

int main(int argc, const char *argv[])
{
    int port;
    log_cycle_t *log;
    rtmq_proxy_t *ctx;
    rtmq_proxy_conf_t conf;

    if (2 != argc) {
        fprintf(stderr, "Didn't special port!");
        return -1;
    }

    memset(&conf, 0, sizeof(conf));

    signal(SIGPIPE, SIG_IGN);

    nice(-20);

    port = atoi(argv[1]);
    rtmq_setup_conf(&conf, port);

    mref_init();

    log = log_init(LOG_LEVEL_ERROR, "./rtmq_ssvr.log");
    if (NULL == log) {
        fprintf(stderr, "errmsg:[%d] %s!", errno, strerror(errno));
        return -1;
    }

    ctx = rtmq_proxy_init(&conf, log);
    if (NULL == ctx) {
        fprintf(stderr, "Initialize send-server failed!");
        return -1;
    }

    rtmq_proxy_reg_add(ctx, MSG_SEARCH_REQ, rtmq_proxy_def_handler, ctx);

    if (rtmq_proxy_launch(ctx)) {
        fprintf(stderr, "Start up send-server failed!");
        return -1;
    }

#if defined(__RTMQ_DEBUG_SEND__)
    rtmq_send_debug(ctx, 1);
#endif /*__RTMQ_DEBUG_SEND__*/

    while (1) { pause(); }

    fprintf(stderr, "Exit send server!");

    return 0;
}

/* 回调函数 */
static int rtmq_proxy_def_handler(int type, int nid, char *buff, size_t len, void *args)
{
    char mesg[1024];

    memset(mesg, 0, sizeof(mesg));

    strncpy(mesg, buff, sizeof(mesg)-1>len?len:sizeof(mesg)-1);

    fprintf(stderr, "type:%d nid:%d buff:[%s] len:%ld args:%p\n", type, nid, mesg, len, args);

    return 0;
}
