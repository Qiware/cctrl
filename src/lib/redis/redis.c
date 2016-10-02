#include "log.h"
#include "str.h"
#include "comm.h"
#include "redis.h"

redisContext *redis_pool_get(redis_pool_t *pool);
int redis_pool_close(redis_pool_t *pool, redisContext *item);

/******************************************************************************
 **函数名称: redis_init
 **功    能: 初始化Redis
 **输入参数:
 **     conf: Redis配置数组
 **输出参数:
 **返    回: Redis对象
 **实现描述:
 **注意事项:
 **作    者: # Qifeng.zou # 2014.11.04 #
 ******************************************************************************/
redisContext *redis_init(const redis_conf_t *conf, int sec)
{
    struct timeval tv;

    tv.tv_sec = sec;
    tv.tv_usec = 0;

    return redisConnectWithTimeout(conf->ip, conf->port, tv);
}

/******************************************************************************
 **函数名称: redis_pool_creat
 **功    能: 创建Redis连接池
 **输入参数:
 **     conf: Redis配置数组
 **     max: 连接池个数(必须为2^n个)
 **输出参数:
 **返    回: Redis对象
 **实现描述:
 **注意事项:
 **作    者: # Qifeng.zou # 2016.10.02 15:40:59 #
 ******************************************************************************/
redis_pool_t *redis_pool_creat(const redis_conf_t *conf, int max)
{
    int idx;
    struct timeval tv;
    redis_pool_t *pool;
    redisContext *item;

    pool = (redis_pool_t *)calloc(1, sizeof(redis_pool_t));
    if (NULL == pool) {
        return NULL;
    }

    pool->get = redis_pool_get;
    pool->close = redis_pool_close;

    pool->ring = ring_creat(max);
    if (NULL == pool->ring) {
        free(pool);
        return NULL;
    }

    for (idx=0; idx<max; ++idx) {
        tv.tv_sec = 3;
        tv.tv_usec = 0;

        item = redisConnectWithTimeout(conf->ip, conf->port, tv);
        if (item->err) {
            redis_pool_destroy(pool);
            return NULL;
        }

        ring_push(pool->ring, (void *)item);
    }

    return pool;
}

/******************************************************************************
 **函数名称: redis_pool_get
 **功    能: 获取一个Redis连接
 **输入参数:
 **输出参数:
 **返    回: Redis对象
 **实现描述:
 **注意事项:
 **作    者: # Qifeng.zou # 2016.10.02 15:40:59 #
 ******************************************************************************/
redisContext *redis_pool_get(redis_pool_t *pool)
{
    return ring_pop(pool->ring);
}

/******************************************************************************
 **函数名称: redis_pool_close
 **功    能: 回收一个Redis连接
 **输入参数:
 **输出参数:
 **返    回: Redis对象
 **实现描述:
 **注意事项:
 **作    者: # Qifeng.zou # 2016.10.02 15:40:59 #
 ******************************************************************************/
int redis_pool_close(redis_pool_t *pool, redisContext *item)
{
    return ring_push(pool->ring, item);
}

/******************************************************************************
 **函数名称: redis_pool_destroy
 **功    能: 销毁Redis连接池
 **输入参数:
 **     pool: 连接池
 **输出参数:
 **返    回:
 **实现描述: 释放所有空间
 **注意事项:
 **作    者: # Qifeng.zou # 2016.10.02 15:57:51 #
 ******************************************************************************/
void redis_pool_destroy(redis_pool_t *pool)
{
    redisContext *item;

    while (1) {
        item = (redisContext *)ring_pop(pool->ring);
        if (NULL == item) {
            break;
        }
        redisFree(item);
    }

    ring_destroy(pool->ring);
    free(pool);
}
