/******************************************************************************
 * Copyright(C) 2014-2024 Xundao All Right Reserved 
 * FileName: log.h
 * Version:  1. 0
 * Description: 打印日志信息
 * Author:   # Qifeng.zou # 2014.08.27 #
 ******************************************************************************/
#if !defined(__LOG_H__)
#define __LOG_H__

#if defined(__ASYNC_LOG__)

#include <sys/timeb.h>

#include "common.h"

/* 宏定义 */
#define LOG_FILE_MAX_NUM        (512)       /* 日志文件最大个数 */
#define LOG_FILE_CACHE_SIZE     (64*1024)   /* 日志文件的缓存SIZE */
#define LOG_SHM_SIZE                        /* 日志共享内存SIZE */\
    (LOG_FILE_MAX_NUM * LOG_FILE_CACHE_SIZE)
#define LOG_SYNC_TIMEOUT        (3)         /* 日志超时同步时间 */

#define LOG_SHM_KEY             (0x32313123)/* 共享内存KEY */

#define LOG_MSG_MAX_LEN         (2048)      /* 日志行最大长度 */
#define LOG_SVR_THREAD_NUM      (1)         /* 服务线程数 */
#define LOG_FILE_MAX_SIZE       (8*1024*1024)/* 单个日志文件的最大SIZE */

#define LOG_SUFFIX              ".log"      /* 日志文件后缀 */
#define LOG_DEFAULT_TRCLOG      "trc.log"   /* 默认日志名 */
#define LogTrclogDefPath(path, size)        /* 默认日志文件路径 */ \
    snprintf(path, size, "../logs/%s", LOG_DEFAULT_TRCLOG);
#define LOG_LOCK_FILE           ".log.lock" /* 日志锁 */
#define LogGetLockPath(path, size)          /* 日志锁路径 */ \
    snprintf(path, size, "../tmp/log/%s", LOG_LOCK_FILE)

/* DUMP设置 */
#define LOG_DUMP_COL_NUM        (16)        /* DUMP列数 */
#define LOG_DUMP_PAGE_MAX_ROWS  (20)        /* DUMP页最大行数 */
#define LOG_DUMP_PAGE_MAX_SIZE  (2048)      /* DUMP页最大SIZE */
#define LOG_DUMP_HEAD_STR     \
    "\nDisplacement -1--2--3--4--5--6--7--8-Hex-0--1--2--3--4--5--6  --ASCII Value--\n"

#define log_is_timeout(diff_time) (diff_time >= LOG_SYNC_TIMEOUT)

/* 日志级别 */
#define LOG_LEVEL_FATAL         (0x0001)    /* 严重级别 */
#define LOG_LEVEL_ERROR         (0x0002)    /* 错误级别 */
#define LOG_LEVEL_WARN          (0x0004)    /* 警告级别 */
#define LOG_LEVEL_INFO          (0x0008)    /* 信息级别 */
#define LOG_LEVEL_DEBUG         (0x0010)    /* 调试级别 */
#define LOG_LEVEL_TRACE         (0x0020)    /* 跟踪级别 */
#define LOG_LEVEL_TOTAL         (6)         /* 级别总数 */

/* 日志信息 */
typedef struct
{
    int idx;                                /* 日志索引号 */
    char path[FILE_NAME_MAX_LEN];           /* 文件路径(路径+名) */
    size_t in_offset;                       /* 输入偏移 */
    size_t out_offset;                      /* 输出偏移 */
    pid_t pid;                              /* 被进程PID占用 */
    struct timeb sync_tm;                   /* 上次同步时间 */
}log_file_info_t;

/* 日志对象 */
typedef struct _log_cycle_t
{
    int fd;                                 /* 文件描述符 */
    log_file_info_t *file;                  /* 文件信息 */
    pid_t pid;                              /* 进程PID */
    int (*action)(
        struct _log_cycle_t *cycle, int level, /* 回调函数 */
        const void *dump, int dumplen,
        const char *errmsg, const struct timeb *curr_time);
}log_cycle_t;

/* 外部接口 */
void log_set_level(int level);
void log_core(int level,
            const char *fname, int lineno,
            const void *dump, int dumplen,
            const char *fmt, ...);

/* 内部接口 */
int log_set_path(const char *path);
int log_trclog_sync(log_file_info_t *file);
void log_set_max_size(size_t size);
void log_abnormal(const char *fname, int lineno, const char *fmt, ...);

#if 0
/* 日志模块接口 */
#define log_fatal(log, ...) /* 撰写FATAL级别日志 */\
    if ((log)->level & LOG_LEVEL_FATAL) \
        log_core(log, LOG_LEVEL_FATAL, __FILE__, __LINE__, NULL, 0, __VA_ARGS__)
#define log_error(log, ...) /* 撰写ERROR级别日志 */\
    if ((log)->level & LOG_LEVEL_ERROR) \
        log_core(log, LOG_LEVEL_ERROR, __FILE__, __LINE__, NULL, 0, __VA_ARGS__)
#define log_warn(log, ...)  /* 撰写WARN级别日志 */\
    if ((log)->level & LOG_LEVEL_WARN) \
        log_core(log, LOG_LEVEL_WARN, __FILE__, __LINE__, NULL, 0, __VA_ARGS__)
#define log_info(log, ...)  /* 撰写INFO级别日志 */\
    if ((log)->level & LOG_LEVEL_INFO) \
        log_core(log, LOG_LEVEL_INFO, __FILE__, __LINE__, NULL, 0, __VA_ARGS__)
#define log_debug(log, ...) /* 撰写DEBUG级别日志 */\
    if ((log)->level & LOG_LEVEL_DEBUG) \
        log_core(log, LOG_LEVEL_DEBUG, __FILE__, __LINE__, NULL, 0, __VA_ARGS__)
#define log_trace(log, ...) /* 撰写TRACE级别日志 */\
    if ((log)->level & LOG_LEVEL_TRACE) \
        log_core(log, LOG_LEVEL_TRACE, __FILE__, __LINE__, NULL, 0, __VA_ARGS__)
#define log_bin(log, addr, len, ...)   /* 撰写MEM-DUMP日志 */\
    if ((log)->level & LOG_LEVEL_INFO) \
        log_core(log, LOG_LEVEL_INFO, __FILE__, __LINE__, addr, len, __VA_ARGS__)
#else
#define log_fatal(...)
#define log_error(...)
#define log_warn(...)
#define log_info(...)
#define log_debug(...)
#define log_bin(...)
#endif
#endif /*__ASYNC_LOG__*/
#endif /*__LOG_H__*/