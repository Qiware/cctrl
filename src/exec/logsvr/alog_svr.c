/*******************************************************************************
 * ģ��: �첽��־ģ�� - ����˴���
 * ����: 
 *      1) �������ڴ�ĳ�ʼ��
 *      2) ����������Ŀͻ��˽��̵��ڻ�������־ͬ�����ļ�
 * ����: # Qifeng.zou # 2013.11.07 #
 ******************************************************************************/
#include <sys/shm.h>
#include <sys/types.h>

#include "alog.h"
#include "common.h"

#if defined(__ASYNC_LOG__) || defined(__ASYNC_ULOG__)

/* ������̻�����·�� */
#define ALOG_SVR_PROC_LOCK  ".alogsvr.lck"
#define AlogSvrGetProcLockPath(path, size) \
    snprintf(path, size, "../tmp/%s",  ALOG_SVR_PROC_LOCK)
#define AlogSvrTryLockProc(fd) proc_try_wrlock(fd)

/* ���������־�ļ�·�� */
#define ALOG_SVR_LOG_NAME   "swalogsvr.log"
#define AlogSvrGetLogPath(path, size) \
    snprintf(path, size, "../logs/%s", ALOG_SVR_LOG_NAME)

static alog_svr_t g_logsvr;     /* ��־������� */

static int alog_svr_init(alog_svr_t *logsvr);
static void *alog_svr_timeout_routine(void *args);
int alog_svr_sync_work(int idx, alog_svr_t *logsvr);
static int alog_svr_proc_lock(void);

static char *alog_svr_creat_shm(int fd);

/******************************************************************************
 **��������: main 
 **��    ��: ��־����������
 **�������: NONE
 **�������: NONE
 **��    ��: 0:success !0:failed
 **ʵ������: 
 **     1. �����־ģ��ĳ�ʼ��
 **     2. ����������պͳ�ʱ����
 **ע������: 
 **��    ��: # Qifeng.zou # 2013.10.28 #
 ******************************************************************************/
int main(void)
{
    int ret;
    alog_svr_t *logsvr = &g_logsvr;

    memset(logsvr, 0, sizeof(alog_svr_t));

    daemon(0, 0);

    /* 1. ��ʼ����־ϵͳ */
    ret = alog_svr_init(logsvr);
    if(ret < 0)
    {
        fprintf(stderr, "Init log failed!");
        return -1;
    }

    thread_pool_add_worker(logsvr->pool, alog_svr_timeout_routine, logsvr);

    while(1){ pause(); }
    return 0;
}

/******************************************************************************
 **��������: alog_svr_proc_lock
 **��    ��: ��־�������������ֹͬʱ���������������
 **�������: 
 **�������: NONE
 **��    ��: 0:success !0:failed
 **ʵ������: 
 **ע������: 
 **��    ��: # Qifeng.zou # 2013.11.06 #
 ******************************************************************************/
int alog_svr_proc_lock(void)
{
    int ret = 0, fd = 0;
    char path[FILE_PATH_MAX_LEN] = {0};

    /* 1. ��ȡ����������ļ�·�� */
    AlogSvrGetProcLockPath(path, sizeof(path));

    Mkdir2(path, ALOG_DIR_MODE);

    /* 2. �򿪷���������ļ� */
    fd = Open(path, ALOG_OPEN_FLAGS, ALOG_OPEN_MODE);
    if(fd < 0)
    {
        fprintf(stderr, "errmsg:[%d]%s! path:[%s]", errno, strerror(errno), path);
        return -1;
    }

    /* 3. ���Լ��� */
    ret = AlogSvrTryLockProc(fd);
    if(ret < 0)
    {
        fprintf(stderr, "errmsg:[%d]%s! path:[%s]", errno, strerror(errno), path);
        Close(fd);
        return -1;
    }

    return 0;
}

/******************************************************************************
 **��������: alog_svr_init
 **��    ��: ��ʼ������
 **�������: 
 **�������: NONE
 **��    ��: 0:success !0:failed
 **ʵ������: 
 **ע������: 
 **��    ��: # Qifeng.zou # 2013.10.28 #
 ******************************************************************************/
static int alog_svr_init(alog_svr_t *logsvr)
{
    int ret = 0;
    char path[FILE_PATH_MAX_LEN] = {0};

    /* ���ø�����־·�� */
    AlogSvrGetLogPath(path, sizeof(path));

    alog_set_path(ALOG_PARAM_TRC, path);
    
    /* 1. �ӷ�������� */
    ret = alog_svr_proc_lock();
    if(ret < 0)
    {
        fprintf(stderr, "Alog server is already running...");
        return -1;    /* ��־���������������... */
    }

    /* 2. ���ļ������� */
    AlogGetLockPath(path, sizeof(path));

    Mkdir2(path, ALOG_DIR_MODE);

    logsvr->fd = Open(path, ALOG_OPEN_FLAGS, ALOG_OPEN_MODE);
    if(logsvr->fd < 0)
    {
        fprintf(stderr, "errmsg:[%d] %s! path:[%s]", errno, strerror(errno), path);
        return -1;
    }

    /* 3. ����/���ӹ����ڴ� */
    logsvr->addr = alog_svr_creat_shm(logsvr->fd);
    if(NULL == logsvr->addr)
    {
        fprintf(stderr, "Create SHM failed!");
        return -1;
    }

    /* 4. ��������߳� */
    ret = thread_pool_init(&logsvr->pool, ALOG_SVR_THREAD_NUM);
    if(ret < 0)
    {
        thread_pool_destroy(logsvr->pool);
        logsvr->pool = NULL;
        fprintf(stderr, "errmsg:[%d]%s!", errno, strerror(errno));
        return -1;
    }

    return 0;
}

/******************************************************************************
 **��������: alog_svr_creat_shm
 **��    ��: ���������ӹ����ڴ�
 **�������: 
 **�������: 
 **     cfg: ��־������Ϣ
 **��    ��: Address of SHM
 **ʵ������: 
 **     1. ���������ڴ�
 **     2. ���ӹ����ڴ�
 **ע������: 
 **��    ��: # Qifeng.zou # 2013.10.28 #
 ******************************************************************************/
static char *alog_svr_creat_shm(int fd)
{
    int idx, shmid;
    void *addr = NULL, *p = NULL;
    alog_file_t *file = NULL;

    /* 1. ���������ڴ� */
    /* 1.1 �ж��Ƿ��Ѿ����� */
    shmid = shmget(ALOG_SHM_KEY, 0, 0666);
    if(shmid >= 0)
    {
        return shmat(shmid, NULL, 0);  /* �Ѵ��� */
    }

    /* 1.2 �쳣�����˳����� */
    if(ENOENT != errno)
    {
        return NULL;
    }

    /* 1.3 ���������ڴ� */
    shmid = shmget(ALOG_SHM_KEY, ALOG_SHM_SIZE, IPC_CREAT|0660);
    if(shmid < 0)
    {
        return NULL;
    }

    /* 2. ATTACH�����ڴ� */
    addr = (void *)shmat(shmid, NULL, 0);
    if((void *)-1 == addr)
    {
        fprintf(stderr, "Attach shm failed! shmid:[%d] key:[0x%x]", shmid, ALOG_SHM_KEY);
        return NULL;
    }

    /* 3. ��ʼ�������ڴ� */
    p = addr;
    for(idx=0; idx<ALOG_FILE_MAX_NUM; idx++)
    {
        file = (alog_file_t *)(p + idx * ALOG_FILE_CACHE_SIZE);

        proc_wrlock_b(fd, idx+1);
        
        file->idx = idx;
        file->pid = ALOG_INVALID_PID;

        proc_unlock_b(fd, idx+1);
    }

    return addr;
}

/******************************************************************************
 **��������: alog_svr_timeout_routine
 **��    ��: ��־��ʱ����
 **�������: 
 **     args: ����
 **�������: NONE
 **��    ��: 0:success !0:failed
 **ʵ������: 
 **     1. ˯��ָ��ʱ��
 **     2. ���α�����־����
 **     3. ���г�ʱ�ж�
 **     4. ���л���ͬ������
 **ע������: 
 **��    ��: # Qifeng.zou # 2013.10.25 #
 ******************************************************************************/
static void *alog_svr_timeout_routine(void *args)
{
    int ret = 0, idx = 0;
    struct timeb curr_time;
    alog_file_t *file = NULL;
    alog_svr_t *logsvr = (alog_svr_t *)args;


    while(1)
    {
        memset(&curr_time, 0, sizeof(curr_time));

        ftime(&curr_time);

        for(idx=0; idx<ALOG_FILE_MAX_NUM; idx++)
        {
            /* 1. ���Լ��� */
            ret = proc_wrlock_b(logsvr->fd, idx+1);
            if(ret < 0)
            {
                continue;
            }

            /* 2. ·��Ϊ�գ�����ͬ�� */
            file = (alog_file_t *)(logsvr->addr + idx*ALOG_FILE_CACHE_SIZE);
            if('\0' == file->path[0])
            {
                proc_unlock_b(logsvr->fd, idx+1);
                continue;
            }

        #if !defined(__ALOG_SVR_SYNC__)
            /* 3. ������־�Ľ�����Ȼ�����У�����ͬ�� */
            if((file->pid <= 0)
                || (0 == proc_is_exist(file->pid)))
            {
                proc_unlock_b(logsvr->fd, idx+1);
                continue;
            }
        #endif /*!__ALOG_SVR_SYNC__*/

            alog_trclog_sync(file);
        
        #if defined(__ALOG_SVR_SYNC__)
            /* �ж��ļ��Ƿ������еĽ�������ʹ���ļ����� */
            if(!alog_file_is_pid_live(file))
         #endif /*__ALOG_SVR_SYNC__*/
            {
                memset(file, 0, sizeof(alog_file_t));
                alog_file_reset_pid(file);
            }
            file->idx = idx;
            
            proc_unlock_b(logsvr->fd, idx+1);
        }
        
        Sleep(ALOG_SYNC_TIMEOUT);
    }

    return (void *)-1;
}

/******************************************************************************
 **��������: alog_svr_sync_work
 **��    ��: ��־ͬ������
 **�������: 
 **     idx: ��������
 **     logsvr: ��־�������
 **�������: NONE
 **��    ��: 0:success !0:failed
 **ʵ������: 
 **     1. �������
 **     2. д���ļ�
 **     3. �������
 **ע������: 
 **��    ��: # Qifeng.zou # 2013.10.28 #
 ******************************************************************************/
int alog_svr_sync_work(int idx, alog_svr_t *logsvr)
{
    alog_file_t *file = NULL;

    file = (alog_file_t *)(logsvr->addr + idx*ALOG_FILE_CACHE_SIZE);
    
    alog_trclog_sync(file);
    
    return 0;
}
#endif /*__ASYNC_LOG__ || __ASYNC_ULOG__*/
