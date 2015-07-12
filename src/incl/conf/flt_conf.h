/******************************************************************************
 ** Copyright(C) 2014-2024 Xundao technology Co., Ltd
 **
 ** 文件名: flt_conf.h
 ** 版本号: 1.0
 ** 描  述: 爬虫配置
 **         定义爬虫配置相关的结构体
 ** 作  者: # Qifeng.zou # 2014.09.04 #
 ******************************************************************************/
#if !defined(__FLT_CONF_H__)
#define __FLT_CONF_H__

#include "uri.h"
#include "comm.h"
#include "redis.h"
#include "xml_tree.h"
#include "mem_pool.h"

/* 工作配置信息 */
typedef struct
{
    int num;                                /* 工作线程数 */

    char path[FILE_PATH_MAX_LEN];           /* 工作路径 */
    char webpage_path[FILE_PATH_MAX_LEN];   /* 网页存储路径 */
    char err_path[FILE_PATH_MAX_LEN];       /* 错误数据存储路径 */
    char man_path[FILE_PATH_MAX_LEN];       /* 管理数据存储路径 */
} flt_work_conf_t;

/* Seed配置信息 */
typedef struct
{
    char uri[URI_MAX_LEN];                  /* 网页URI */
    uint32_t depth;                         /* 网页深度 */
} flt_seed_conf_t;

/* Redis配置信息 */
typedef struct
{
    int num;                                /* 配置数目 */
    redis_conf_t *conf;                     /* 配置数组(注: [0]为MASTER配置 [1~N]为副本配置 */

    char taskq[QUEUE_NAME_MAX_LEN];         /* 任务队列名 */
    char done_tab[TABLE_NAME_MAX_LEN];      /* DONE哈希表名 */
    char push_tab[TABLE_NAME_MAX_LEN];      /* PUSHED哈希表名 */
} flt_redis_conf_t;

/* 配置信息 */
typedef struct
{
    struct
    {
        int level;                          /* 日志级别 */
        char path[FILE_PATH_MAX_LEN];       /* 日志路径 */
    } log;                                  /* 日志配置 */

    struct
    {
        uint32_t depth;                     /* 最大爬取深度 */
        char path[FILE_PATH_MAX_LEN];       /* 网页存储路径 */
    } download;                             /* 下载配置 */

    int workq_count;                        /* 工作队列容量 */
    int man_port;                           /* 管理服务侦听端口 */

    flt_redis_conf_t redis;                 /* REDIS配置 */
    flt_work_conf_t work;                   /* 工作配置 */

#define FLT_SEED_MAX_NUM   (100)            /* 种子最大数 */
    uint32_t seed_num;                      /* 种子实数 */
    flt_seed_conf_t seed[FLT_SEED_MAX_NUM]; /* 种子配置 */
} flt_conf_t;

flt_conf_t *flt_conf_load(const char *path, log_cycle_t *log);
#define flt_conf_destroy(conf)              /* 销毁配置对象 */\
{ \
    free(conf); \
}

#endif /*__FLT_CONF_H__*/
