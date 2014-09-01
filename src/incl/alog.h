#if !defined(__ALOG_H__)
#define __ALOG_H__

#if defined(__ASYNC_LOG__)

#include <sys/timeb.h>

#include "common.h"
#include "thread_pool.h"

/* ���ܺ꿪�� */
#define ALOG_ERR_FORCE      (__ON__)    /* ���أ����ִ�����־ʱ��ǿ��ͬ����Ӧ�������� */
#define ALOG_SVR_SYNC       (__OFF__)   /* ���أ�����������ͬʱ������־ͬ�� */

#if defined(ALOG_ERR_FORCE) && (__ON__ == ALOG_ERR_FORCE)
    #define __ALOG_ERR_FORCE__          /* ���ִ�����־ʱ��ǿ��ͬ����Ӧ�������� */
#endif
#if defined(ALOG_SVR_SYNC) && (__ON__ == ALOG_SVR_SYNC)
    #define __ALOG_SVR_SYNC__           /* ����������ͬʱ������־ͬ�� */
#endif

/* ͬ������������Ϣ */
#define ALOG_FILE_MAX_NUM       (512)           /* ��־���������� */
#define ALOG_FILE_CACHE_SIZE    (64*1024)       /* �ļ�����ߴ�(����) */
#define ALOG_SHM_SIZE           (ALOG_FILE_MAX_NUM * ALOG_FILE_CACHE_SIZE)  /* ��־�����ڴ��С */
#define ALOG_SYNC_TIMEOUT       (3)             /* ͬ����ʱʱ�� */

#define ALOG_SHM_KEY            (0x32313123)    /* �����ڴ�KEY */

#define ALOG_OPEN_MODE          (0666)          /* ������־Ȩ�� */
#define ALOG_OPEN_FLAGS         (O_CREAT|O_WRONLY|O_APPEND) /* ����־��ʶ */
#define ALOG_DIR_MODE           (0777)          /* ����Ŀ¼Ȩ�� */

#define ALOG_MSG_MAX_LEN        (2048)          /* ��־��󳤶�(����) */
#define ALOG_SVR_THREAD_NUM     (1)             /* ��־�߳���Ŀ */
#define ALOG_FILE_MAX_SIZE      (8*1024*1024)   /* ��־�ļ����ߴ� */

#define ALOG_SUFFIX             ".log"          /* ��־�ļ���׺ */
#define ALOG_DEFAULT_ERRLOG     "error.log"     /* Ĭ�ϴ�����־ */
#define ALOG_DEFAULT_TRCLOG     "trc.log"       /* Ĭ�ϸ�����־ */
#define AlogErrlogDefPath(path, size)           /* Ĭ�ϴ�����־·�� */ \
    snprintf(path, size, "../logs/syslog/%s", ALOG_DEFAULT_ERRLOG);
#define AlogTrclogDefPath(path, size)           /* Ĭ�ϸ�����־·�� */ \
    snprintf(path, size, "../logs/syslog/%s", ALOG_DEFAULT_TRCLOG);
#define ALOG_LOCK_FILE          ".alog.lck"     /* ���ļ��� */
#define AlogGetLockPath(path, size)             /* ���ļ�����·�� */ \
    snprintf(path, size, "../tmp/alog/%s", ALOG_LOCK_FILE)

#define ALOG_INVALID_PID            (-1)        /* �Ƿ�����ID */
#define ALOG_INVALID_FD             (-1)        /* �Ƿ��ļ������� */

/* ��ӡָ���ڴ������ */
#define ALOG_DUMP_COL_NUM           (16)        /* 16����: ÿ������ */
#define ALOG_DUMP_LINE_MAX_SIZE     (512)       /* 16����: ÿ�д�С */
#define ALOG_DUMP_PAGE_MAX_LINE     (20)        /* 16����: ÿҳ���� */
#define ALOG_DUMP_PAGE_MAX_SIZE     (2048)      /* 16����: ÿҳ��С */
#define ALOG_DUMP_HEAD_STR     \
    "\nDisplacement -1--2--3--4--5--6--7--8-Hex-0--1--2--3--4--5--6  --ASCII Value--\n"

#define AlogIsTimeout(diff_time) (diff_time >= ALOG_SYNC_TIMEOUT)

/* ��־���� */
typedef enum
{
    LOG_LEVEL_DEBUG,                /* ���Լ��� */
    LOG_LEVEL_INFO,                 /* ��Ϣ���� */
    LOG_LEVEL_WARNING,              /* ���漶�� */
    LOG_LEVEL_ERROR,                /* ���󼶱� */

    LOG_LEVEL_UNKNOWN,              /* δ֪���� */
    LOG_LEVEL_TOTAL
}alog_level_e;

/* ��־�������� */
typedef enum
{
    ALOG_CMD_INVALID,               /* �Ƿ����� */
    ALOG_CMD_SYNC,                  /* ͬ������ */

    ALOG_CMD_UNKNOWN,               /* δ֪���� */
    ALOG_CMD_TOTAL = ALOG_CMD_UNKNOWN /* ������Ŀ */
}alog_cmd_e;

/* ��־������Ϣ */
typedef enum
{
    ALOG_PARAM_ERR,                 /* ������־·������ */
    ALOG_PARAM_TRC,                 /* ������־·������ */
    ALOG_PARAM_SIZE,                /* ��־��С���� */

    ALOG_PARAM_UNKNOWN,             /* δ֪���� */
    ALOG_PARAM_TOTAL = ALOG_PARAM_UNKNOWN
}alog_param_e;

/* ��־����ṹ */
typedef struct
{
    int idx;                        /* �������� */
    alog_cmd_e type;                /* ��־�������� */
}alog_cmd_t;

/* �ļ�������Ϣ */
typedef struct
{
    int idx;                        /* ������ */
    char path[FILE_NAME_MAX_LEN];   /* ��־�ļ�����·�� */
    size_t in_offset;               /* д��ƫ�� */
    size_t out_offset;              /* ͬ��ƫ�� */
    pid_t pid;                      /* ʹ����־����Ľ���ID */
    struct timeb sync_time;         /* �ϴ�ͬ����ʱ�� */
}alog_file_t;

/* ��־�������� */
typedef struct _alog_cycle_t
{
    int fd;                         /* ��־������ */
    alog_file_t *file;              /* �ļ����� */
    pid_t pid;                      /* ��ǰ����ID */
    int (*action)(struct _alog_cycle_t *cycle, int level, /* ��־��Ϊ */
            const void *dump, int dumplen, const char *msg, const struct timeb *curr_time);
}alog_cycle_t;

/* ��־������� */
typedef struct
{
    int fd;                         /* �ļ���FD */
    void *addr;                     /* �����ڴ��׵�ַ */
    thread_pool_t *pool;            /* �̳߳� */
}alog_svr_t;

/* ����ӿ� */
extern void alog_set_level(alog_level_e level);
extern void log_core(int level, const char *fname, int lineno, const void *dump, int dumplen, const char *fmt, ...);

/* �ڲ��ӿ� */
int alog_set_path(int type, const char *path);
extern int alog_trclog_sync(alog_file_t *file);
extern void alog_set_max_size(size_t size);
void alog_abnormal(const char *fname, int lineno, const char *fmt, ...);

#if defined(__ALOG_SVR_SYNC__)
#define alog_file_is_pid_live(file) (0 == proc_is_exist((file)->pid))
#endif /*__ALOG_SVR_SYNC__*/
#define alog_file_reset_pid(file) ((file)->pid = ALOG_INVALID_PID)

#if 1 /* �ӿ����� */

#define log_error(...) log_core(LOG_LEVEL_ERROR, NULL, 0, NULL, 0, __VA_ARGS__)
#define log_debug(...) log_core(LOG_LEVEL_DEBUG, NULL, 0, NULL, 0, __VA_ARGS__)
#define log_info(...) log_core(LOG_LEVEL_INFO, NULL, 0, NULL, 0, __VA_ARGS__)
#define log_bin(level, addr, len, ...) log_core(level, NULL, 0, addr, len, __VA_ARGS__)

#define log_set_syslog_level(level) alog_set_level(level)
#define log_set_applog_level(level) ulog_set_level(level)
#endif

#endif /*__ASYNC_LOG__*/
#endif /*__ALOG_H__*/
