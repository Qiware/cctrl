#if !defined(__REDIS_H__)
#define __REDIS_H__

#include "comm.h"
#include "ring.h"
#include <hiredis/hiredis.h>

#include "redis_hash.h"
#include "redis_list.h"

/* Redis配置 */
typedef struct
{
    char ip[IP_ADDR_MAX_LEN];           /* IP */
    int port;                           /* 端口号 */
    char passwd[PASSWD_MAX_LEN];        /* 密码 */
} redis_conf_t;

/* Redis池 */
typedef struct _redis_pool_t
{
    ring_t *ring;                       /* 连接池 */

    redisContext *(*get)(struct _redis_pool_t *pool); /* 获取一个连接 */
    int (*close)(struct _redis_pool_t *pool, redisContext *item); /* 放回一个连接 */
} redis_pool_t;

redisContext *redis_init(const redis_conf_t *conf, int sec);
#define redis_destroy(redis) redisFree(redis)

redis_pool_t *redis_pool_creat(const redis_conf_t *conf, int max);
void redis_pool_destroy(redis_pool_t *pool);

#endif /*__REDIS_H__*/
