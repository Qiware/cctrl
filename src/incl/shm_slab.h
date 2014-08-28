#if !defined(__SHM_SLAB__)
#define __SHM_SLAB__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>

/* �ڴ���䷽ʽ */
typedef enum
{
    SHM_SLAB_ALLOC_UNKNOWN, /* δ֪��ʽ */
    SHM_SLAB_ALLOC_SMALL,   /* С��128�ֽ� */
    SHM_SLAB_ALLOC_EXACT,   /* ����128�ֽ� */
    SHM_SLAB_ALLOC_LARGE,   /* ����128�ֽڣ�С��2048�ֽ� */
    SHM_SLAB_ALLOC_PAGES,   /* ����ҳ���� */

    SHM_SLAB_ALLOC_TOTAL
}SHM_SLAB_ALLOC_TYPE;

/* SLOT���� */
typedef struct
{
    unsigned int index;     /* slot���� */
    int page_idx;           /* slot�����ҳ���� */
}shm_slab_slot_t;

/* PAGE���� */
typedef struct
{
    unsigned int index;     /* ҳ���� */
    unsigned int pages;     /* ��¼ҳ�����ҳ�� */
    unsigned int shift:16;  /* ҳ����Ĵ�С��Ӧ��λ�� */
    unsigned int type:16;   /* ҳ���������: ȡֵ��ΧSHM_SLAB_ALLOC_TYPE */
    unsigned int bitmap;    /* ҳ��ʹ��λͼ */
    unsigned int rbitmap;   /* ��ʼλͼ��ȡ��ֵ */
    int next_idx;           /* ��һҳ������ */
    int prev_idx;           /* ��һҳ������ */
}shm_slab_page_t;

typedef struct
{
    size_t pool_size;       /* �ڴ�ռ��ܴ�С */
    
    int min_size;           /* ��С���䵥Ԫ */
    int min_shift;          /* ��С���䵥Ԫ��λ�� */

    size_t base_offset;     /* ����ƫ���� */
    size_t data_offset;     /* �ɷ���ռ����ʼƫ���� */
    size_t end_offset;      /* �ɷ���ռ�Ľ���ƫ���� */

    size_t slot_offset;     /* SLOT�������ʼƫ���� */
    size_t page_offset;     /* PAGE�������ʼƫ���� */
	
    shm_slab_page_t free;   /* ����ҳ���� */
}shm_slab_pool_t;

extern int shm_slab_init(shm_slab_pool_t *pool);
extern void *shm_slab_alloc(shm_slab_pool_t *pool, size_t size);
extern int shm_slab_free(shm_slab_pool_t *pool, void *p);

extern size_t shm_slab_head_size(size_t size);

#endif /*__SHM_SLAB__*/
