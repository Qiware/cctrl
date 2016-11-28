#include "mesg.h"
#include "search.h"
#include "mem_ref.h"
#include "test_mesg.h"
#include "rtmq_recv.h"

/* 回调函数 */
static int rtmq_work_def_hdl(int type, int nid, char *buff, size_t len, void *args)
{
    char mesg[1024];

    memset(mesg, 0, sizeof(mesg));

    strncpy(mesg, buff, sizeof(mesg)-1>len?len:sizeof(mesg)-1);

    fprintf(stderr, "type:%d nid:%d buff:[%s] len:%ld args:%p\n", type, nid, mesg, len, args);
    return 0;
}

/* 配置RTMQ */
static void rtmq_setup_conf(rtmq_conf_t *conf, int port)
{
    rtmq_auth_t *auth;
    snprintf(conf->path, sizeof(conf->path), "./");

    conf->nid = 20000;
    conf->port = port;
    conf->recv_thd_num = 1;
    conf->work_thd_num = 1;
    conf->recvq_num = 3;
    conf->recvq.max = 1024;
    conf->recvq.size = 40960;
    conf->distq_num = 3;
    conf->distq.max = 1024;
    conf->distq.size = 40960;
    conf->sendq.max = 1024;
    conf->sendq.size = 40960;
 

    conf->auth = list_creat(NULL);

    auth = (rtmq_auth_t *)calloc(1, sizeof(rtmq_auth_t));
    snprintf(auth->usr, sizeof(auth->usr), "qifeng");
    snprintf(auth->passwd, sizeof(auth->passwd), "111111");
    list_rpush(conf->auth, auth);
}

int main(int argc, const char *argv[])
{
    int ret, port;
    rtmq_cntx_t *ctx;
    log_cycle_t *log;
    rtmq_conf_t conf;

    memset(&conf, 0, sizeof(conf));

    if (2 != argc) {
        fprintf(stderr, "Didn't special port!");
        return -1;
    }

    mem_ref_init();

    port = atoi(argv[1]);

    rtmq_setup_conf(&conf, port);

    signal(SIGPIPE, SIG_IGN);
                                       
    log = log_init(LOG_LEVEL_DEBUG, "./rtmq.log");
    if (NULL == log) {
        fprintf(stderr, "Initialize log failed!");
        return RTMQ_ERR;
    }

    /* 1. 接收端初始化 */
    ctx = rtmq_init(&conf, log);
    if (NULL == ctx) {
        fprintf(stderr, "Initialize rcvsvr failed!");
        return RTMQ_ERR;
    }

    rtmq_register(ctx, MSG_SEARCH_REQ, rtmq_work_def_hdl, NULL);

    /* 2. 接收服务端工作 */
    ret = rtmq_launch(ctx);
    if (0 != ret) {
        fprintf(stderr, "Work failed!");
    }

    while(1) pause();

    return 0;
}
