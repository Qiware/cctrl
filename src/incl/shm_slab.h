#if !defined(__SHM_SLAB__)
#define __SHM_SLAB__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
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
    uint32_t index;         /* slot���� */
    int32_t page_idx;       /* slot�����ҳ���� */
}shm_slab_slot_t;

/* PAGE���� */
typedef struct
{
    uint32_t index;         /* ҳ���� */
    uint32_t pages;         /* ��¼ҳ�����ҳ�� */
    uint32_t shift:16;      /* ҳ����Ĵ�С��Ӧ��λ�� */
    uint32_t type:16;       /* ҳ���������: ȡֵ��ΧSHM_SLAB_ALLOC_TYPE */
    uint32_t bitmap;        /* ҳ��ʹ��λͼ */
    uint32_t rbitmap;       /* ��ʼλͼ��ȡ��ֵ */
    int32_t next_idx;       /* ��һҳ������ */
    int32_t prev_idx;       /* ��һҳ������ */
}shm_slab_page_t;

typedef struct
{
    size_t pool_size;       /* �ڴ�ռ��ܴ�С */
    
    int32_t min_size;       /* ��С���䵥Ԫ */
    int32_t min_shift;      /* ��С���䵥Ԫ��λ�� */

    size_t base_offset;     /* ����ƫ���� */
    size_t data_offset;     /* �ɷ���ռ����ʼƫ���� */
    size_t end_offset;      /* �ɷ���ռ�Ľ���ƫ���� */

    size_t slot_offset;     /* SLOT�������ʼƫ���� */
    size_t page_offset;     /* PAGE�������ʼƫ���� */
	
    shm_slab_page_t free;   /* ����ҳ���� */
}shm_slab_pool_t;

int32_t shm_slab_init(shm_slab_pool_t *pool);
void *shm_slab_alloc(shm_slab_pool_t *pool, size_t size);
int32_t shm_slab_free(shm_slab_pool_t *pool, void *p);

size_t shm_slab_head_size(size_t size);
#endif /*__SHM_SLAB__*/
