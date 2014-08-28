#include "shm_slab.h"
#include "xdt_log.h"

/* �궨�� */
#define SHM_SLAB_PAGE_SIZE  (4096)  /* ҳ�ߴ� */
#define SHM_SLAB_MIN_SIZE   (8)     /* ��С����ߴ� */
#define SHM_SLAB_MIN_SHIFT  (3)     /* ��С����ߴ��λ�� */
#define SHM_SLAB_BITMAP_BITS (32)   /* λͼ��λ */
#define SHM_SLAB_BITMAP_SHIFT (5)   /* λͼ��λ��Ӧ��λ�� */

#define SHM_SLAB_NULL_PAGE  (-1)    /* ��ҳ */
#define SHM_SLAB_FREE_PAGE  (-2)    /* FREE���� */
#define SHM_SLAB_BUSY_BITMAP  (0xffffffff)  /* ��ҳ�ڴ涼�ѱ�ռ�� */

#define shm_slab_is_busy_bitmap(bitmap) (SHM_SLAB_BUSY_BITMAP == (bitmap))
#define shm_slab_is_null_page(page_idx) (SHM_SLAB_NULL_PAGE == (page_idx))
#define shm_slab_is_free_page(page_idx) (SHM_SLAB_FREE_PAGE == (page_idx))

/* ��̬ȫ�ֱ��� */
static size_t g_shm_slab_page_size = 0; /* ÿһҳ�ڴ�Ĵ�С */
static int g_shm_slab_page_shift = 0;   /* ÿһҳ�ڴ��С��Ӧ��λ�� */
static size_t g_shm_slab_max_size = 0;  /* SLOT����ڴ���䵥Ԫ */
static int g_shm_slab_max_shift = 0;    /* SLOT����ڴ���䵥Ԫ��Ӧ��λ�� */
static size_t g_shm_slab_exact_size = 0;/* SLOT��ȷ�ڴ���䵥Ԫ */
static int g_shm_slab_exact_shift = 0;  /* SLOT��ȷ�ڴ���䵥Ԫ��Ӧ��λ�� */

#define shm_slab_page_size() (g_shm_slab_page_size)
#define shm_slab_page_shift() (g_shm_slab_page_shift)
#define shm_slab_max_size() (g_shm_slab_max_size)
#define shm_slab_max_shift() (g_shm_slab_max_shift)
#define shm_slab_exact_size() (g_shm_slab_exact_size)
#define shm_slab_exact_shift() (g_shm_slab_exact_shift)

#define shm_slab_set_page_size(size) (g_shm_slab_page_size = (size))
#define shm_slab_set_page_shift(shift) (g_shm_slab_page_shift = (shift))
#define shm_slab_set_max_size(size) (g_shm_slab_max_size = (size))
#define shm_slab_set_max_shift(shift)  (g_shm_slab_max_shift = (shift))
#define shm_slab_set_exact_size(size) (g_shm_slab_exact_size = (size))
#define shm_slab_set_exact_shift(shift) (g_shm_slab_exact_shift = (shift))


/* ��̬�������� */
static void shm_slab_init_param(void);
static shm_slab_page_t *shm_slab_alloc_pages(shm_slab_pool_t *pool, int pages);
static int shm_slab_free_pages(shm_slab_pool_t *pool, shm_slab_page_t *page);
static void *shm_slab_alloc_slot(shm_slab_pool_t *pool, size_t size);
static void *_shm_slab_alloc_slot(shm_slab_pool_t *pool, int slot_idx, int type);
#if defined(__XDT_NOT_USED__)
static int shm_slab_slot_add_page(
    shm_slab_pool_t *pool, shm_slab_slot_t *slot, shm_slab_page_t *page);
#endif /*__XDT_NOT_USED__*/
static int shm_slab_slot_remove_page(
    shm_slab_pool_t *pool, shm_slab_slot_t *slot, shm_slab_page_t *page);

/******************************************************************************
 **Name  : shm_slab_init
 **Func  : Initialize share-memory slab pool
 **Input : 
 **     pool: Object of slab pool.
 **     base_offset: Base offset.
 **Output: NONE
 **Return: 0:Success    !0:Failed
 **Desc  : 
 **     |<-               HEAD               ->|
 **     |<- POOL ->|<-   SLOT   ->|<-  PAGE  ->|<-            DATA          ->|
 **     -----------------------------------------------------------------------
 **     |          |              |            |      |      |       |        |
 **     |   Pool   |     Slot     |    Page    |  P1  |  P2  |  ...  |   Pn   |
 **     |          |              |            |      |      |       |        |
 **     -----------------------------------------------------------------------
 **     ^          ^              ^            ^                              ^
 **     |          |              |            |                              |
 **    addr       slot           page         data                           end
 **Note  : 
 **     addr: Ϊƫ�����Ļ�ַ
 **Author: # Qifeng.zou # 2013.07.12 #
 ******************************************************************************/
int shm_slab_init(shm_slab_pool_t *pool)
{
    void *addr = NULL;
    size_t min_size = 0, left_size = 0;
    int idx = 0, slot_num = 0, page_num = 0;
    shm_slab_slot_t *slot = NULL;
    shm_slab_page_t *page = NULL;

    shm_slab_init_param();

    slot_num = shm_slab_max_shift() - SHM_SLAB_MIN_SHIFT + 1;
    min_size = sizeof(shm_slab_pool_t) + slot_num * sizeof(shm_slab_page_t);
    
    if(pool->pool_size < min_size)
    {
        return -1;  /* Not enough memory */
    }

    addr = (void *)pool;

    pool->min_size = SHM_SLAB_MIN_SIZE;
    pool->min_shift = SHM_SLAB_MIN_SHIFT;

    pool->slot_offset = sizeof(shm_slab_pool_t);
    pool->page_offset = pool->slot_offset + slot_num * sizeof(shm_slab_page_t);
    pool->end_offset = pool->pool_size;

    slot = (shm_slab_slot_t *)(addr + pool->slot_offset);
    for(idx=0; idx<slot_num; idx++)
    {
        slot[idx].index = idx;
        slot[idx].page_idx = SHM_SLAB_NULL_PAGE;
    }

    if(pool->end_offset < pool->page_offset)
    {
        log_error("Not enough memory!");
        return -1;  /* Not enough memory */
    }

    left_size = pool->end_offset - pool->page_offset;
    page_num = left_size / (shm_slab_page_size() + sizeof(shm_slab_page_t));
    if(page_num <= 0)
    {
        log_error("Not enough memory!");
        return -1;  /* Not enough memory */
    }

    log_info("page_num:%d!", page_num);
    
    page = (shm_slab_page_t *)(addr + pool->page_offset);
    for(idx=0; idx<page_num; idx++)
    {
        page[idx].index = idx;
        page[idx].shift = shm_slab_page_shift();
        page[idx].type = SHM_SLAB_ALLOC_UNKNOWN;
    }

    pool->free.pages = page_num;
    pool->free.prev_idx = SHM_SLAB_NULL_PAGE;
    pool->free.next_idx = 0;

    page->pages = page_num;
    page->bitmap = 0;
    page->next_idx = SHM_SLAB_NULL_PAGE;
    page->prev_idx = SHM_SLAB_FREE_PAGE;

    pool->data_offset = pool->page_offset + page_num * sizeof(shm_slab_page_t);
    
    return 0;
}

/******************************************************************************
 **Name  : shm_slab_init_param
 **Func  : Initialize global parmaters
 **Input : NONE
 **Output: NONE
 **Return: 0:Success    !0:Failed
 **Desc  : 
 **Note  : 
 **Author: # Qifeng.zou # 2013.07.15 #
 ******************************************************************************/
static void shm_slab_init_param(void)
{
    int shift = 0;
    size_t size = 0;
    
    
    if(0 == shm_slab_page_size())
    {
        /* Set page size */
        shm_slab_set_page_size(SHM_SLAB_PAGE_SIZE);
        for(size=shm_slab_page_size(), shift=0; size>>=1; shift++)
        {
            /* Do nothing */
        }
        shm_slab_set_page_shift(shift);

        /* Set max size */
        shm_slab_set_max_size(shm_slab_page_size() >> 1);
        for(size=shm_slab_max_size(), shift=0; size>>=1; shift++)
        {
            /* Do nothing */
        }
        shm_slab_set_max_shift(shift);

        /* Set exact size */
        shm_slab_set_exact_size(shm_slab_page_size()/(8 * sizeof(int)));
        for(size=shm_slab_exact_size(), shift=0; size>>=1; shift++)
        {
            /* Do nothing */
        }
        shm_slab_set_exact_shift(shift);
    }
    
    return;
}

/******************************************************************************
 **Name  : shm_slab_get_alloc_type
 **Func  : Get alloc type. 
 **Input : 
 **     size: Alloc size of memory
 **Output: NONE
 **Return: Type of alloced. range: SHM_SLAB_ALLOC_TYPE
 **Desc  : 
 **Note  : 
 **Author: # Qifeng.zou # 2013.07.16 #
 ******************************************************************************/
static int shm_slab_get_alloc_type(int size)
{
    if(size < shm_slab_exact_size())
    {
        return SHM_SLAB_ALLOC_SMALL;
    }
    else if(size == shm_slab_exact_size())
    {
        return SHM_SLAB_ALLOC_EXACT;
    }
    else if(size > shm_slab_exact_size() && size < shm_slab_max_size())
    {
        return SHM_SLAB_ALLOC_LARGE;
    }

    return SHM_SLAB_ALLOC_PAGES;
}

/******************************************************************************
 **Name  : shm_slab_alloc
 **Func  : Alloc special size of memory from slab pool.
 **Input : 
 **     pool: Object of slab pool.
 **     size: Alloc special size of memory.
 **Output: NONE
 **Return: 0:Success    !0:Failed
 **Desc  : 
 **     1. Alloc large memory from slab pool.
 **     2. Alloc small memory from slab pool.
 **Note  : 
 **Author: # Qifeng.zou # 2013.07.12 #
 ******************************************************************************/
void *shm_slab_alloc(shm_slab_pool_t *pool, size_t size)
{
    int pages = 0;
    void *addr = NULL, *p = NULL;
    shm_slab_page_t *page = NULL;

    if(0 == size)
    {
        log_error("Size is incorrect!");
        return NULL;
    }
    
    /* 1. Alloc large memory */
    if(size >= shm_slab_max_size())
    {
        addr = (void *)pool;
        pages = (size + shm_slab_page_size() - 1) >> shm_slab_page_shift();
        
        page = shm_slab_alloc_pages(pool, pages);
        if(NULL == page)
        {
            log_error("Alloc pages failed!");
            return NULL;
        }
        
        p = (addr + pool->data_offset + (page->index << shm_slab_page_shift()));

        memset(p, 0, size);
        return p;
    }

    /* 2. Alloc small memory */
    return shm_slab_alloc_slot(pool, size);
}

/******************************************************************************
 **Name  : shm_slab_alloc_pages
 **Func  : Alloc special pages of memory from slab pool.
 **Input : 
 **     pool: Object of slab pool.
 **     pages: Alloc special pages of memory.
 **Output: NONE
 **Return: Start offset of alloced pages.
 **Desc  : 
 **Note  : 
 **Author: # Qifeng.zou # 2013.07.12 #
 ******************************************************************************/
static shm_slab_page_t *shm_slab_alloc_pages(shm_slab_pool_t *pool, int pages)
{
    void *addr = NULL;
    shm_slab_page_t *start_page = NULL, *page = NULL,
                    *next = NULL, *prev = NULL;

    addr = (void *)pool;
    page = &pool->free;
    start_page = (shm_slab_page_t *)(addr + pool->page_offset);


    while(!shm_slab_is_null_page(page->next_idx))
    {
        page = &start_page[page->next_idx];        
        if(page->pages > pages)
        {
            next = page + pages;
            
            next->pages = page->pages - pages;

            if(shm_slab_is_free_page(page->prev_idx))
            {
                prev = &pool->free;
            }
            else
            {
                prev = &start_page[page->prev_idx];
            }
            
            next->prev_idx = page->prev_idx;
            next->next_idx = page->next_idx;

            prev->next_idx = next - start_page;

            page->pages = pages;
            page->shift = shm_slab_page_shift();
            page->type = SHM_SLAB_ALLOC_PAGES;
            page->next_idx = SHM_SLAB_NULL_PAGE;
            page->prev_idx = SHM_SLAB_NULL_PAGE;

            return page;
        }
        else if(pages == page->pages)
        {
            if(shm_slab_is_null_page(page->next_idx))
            {
                if(shm_slab_is_free_page(page->prev_idx))
                {
                    prev = &pool->free;
                }
                else
                {
                    prev = &start_page[page->prev_idx];
                }
                prev->next_idx = SHM_SLAB_NULL_PAGE;
            }
            else
            {
                if(shm_slab_is_free_page(page->prev_idx))
                {
                    prev = &pool->free;
                }
                else
                {
                    prev = &start_page[page->prev_idx];
                }
                next = &start_page[page->next_idx];

                prev->next_idx = page->next_idx;
                next->prev_idx = page->prev_idx;
            }
            
            page->type = SHM_SLAB_ALLOC_PAGES;
            page->next_idx = SHM_SLAB_NULL_PAGE;
            page->prev_idx = SHM_SLAB_NULL_PAGE;

            return page;
        }
    }

    log_error("Alloc pages failed!");
    
    return NULL;
}

/******************************************************************************
 **Name  : shm_slab_alloc_slot
 **Func  : Alloc special size of memory from slab pool.
 **Input : 
 **     pool: Object of slab pool.
 **     size: Alloc special size of memory.
 **Output: NONE
 **Return: Start offset of alloced pages.
 **Desc  : 
 **Note  : 
 **Author: # Qifeng.zou # 2013.07.12 #
 ******************************************************************************/
static void *shm_slab_alloc_slot(shm_slab_pool_t *pool, size_t size)
{
    void *p = NULL;
    int shift = 0, idx = 0, s = 0;
    SHM_SLAB_ALLOC_TYPE type = SHM_SLAB_ALLOC_UNKNOWN;
    
    /* 1. Make sure use which slot */
    if(size > pool->min_size)
    {
        shift = 1;
        for(s = size-1; s >>= 1; shift++)
        {
            /* Do nothing */
        }
        idx = shift - pool->min_shift;
    }
    else
    {
        size = pool->min_size;
        shift = pool->min_shift;
        idx = 0;
    }

    type = shm_slab_get_alloc_type(size);

    p = _shm_slab_alloc_slot(pool, idx, type);
    if(NULL == p)
    {
        return NULL;
    }

    memset(p, 0, size);

    return p;
}

/******************************************************************************
 **Name  : _shm_slab_alloc_slot
 **Func  : Alloc small slot of memory from slab pool.
 **Input :
 **     pool: Object of slab pool.
 **     slot_idx: Index of slob array.
 **     type: Type of alloc. range: SHM_SLAB_ALLOC_TYPE
 **Output: NONE 
 **Return: 0:Success    !0:Failed
 **Desc  : 
 **     |<-               HEAD               ->|
 **     |<- POOL ->|<-   SLOT   ->|<-  PAGE  ->|<-            DATA          ->|
 **     -----------------------------------------------------------------------
 **     |          |              |            |      |      |       |        |
 **     |   Pool   |     Slot     |    Page    |  P1  |  P2  |  ...  |   Pn   |
 **     |          |              |            |      |      |       |        |
 **     -----------------------------------------------------------------------
 **     ^          ^              ^            ^                              ^
 **     |          |              |            |                              |
 **    addr       slot           page         data                           end
 **Note  :
 **Author: # Qifeng.zou # 2013.07.15 #
 ******************************************************************************/
static void *_shm_slab_alloc_slot(shm_slab_pool_t *pool, int slot_idx, int type)
{
    int *bitmap = NULL;
    shm_slab_slot_t *slot = NULL;
    shm_slab_page_t *start_page = NULL, *page = NULL, *new_page = NULL;
    void *addr = NULL, *data = NULL, *ptr = NULL;
    int bits = 0, exp_bitmaps = 0, i = 0, idx = 0, shift = 0;

    addr = (void *)pool;
    data = addr + pool->data_offset;
    start_page = (shm_slab_page_t *)(addr + pool->page_offset);
    
    shift = slot_idx + pool->min_shift;			/* SIZE��λ�� */
    bits = (shm_slab_page_size() >> shift);		/* ������Ҫ����bit�ܱ�ʾslotռ����� */
    exp_bitmaps = (bits >> SHM_SLAB_BITMAP_SHIFT) - 1;	/* (1).��>0ʱ����ʾ��Ҫ��չλͼ
                                                           (2).��<=0ʱ����ʾ����Ҫ��չλͼ */

    /* 1. Get address of page */
    slot = (shm_slab_slot_t *)(addr + pool->slot_offset);
    if(shm_slab_is_null_page(slot[slot_idx].page_idx))
    {
        page = shm_slab_alloc_pages(pool, 1);
        if(NULL == page)
        {
            log_error("Alloc pages failed!");
            return NULL;
        }

        page->type = type;
        page->shift = shift;
        slot[slot_idx].page_idx = page - start_page;

        /* Set bitmap */
        if(exp_bitmaps > 0)
        {
            /* 1. ��Ҫ��չλͼ: ������Ϊ��չλͼ��SLOT�Ķ�Ӧbit��1 */
            for(i=0; i<exp_bitmaps; i++)
            {
                page->bitmap |= (1 << i);
            }
        }
        else
        {
            /* 2. ����Ҫ��չλͼ: �������ڵ�SLOT��Ӧ��bit��1 */
            for(i=bits; i<SHM_SLAB_BITMAP_BITS; i++)
            {
                 page->bitmap |= (1 << i);
            }
        }
        page->rbitmap = ~(page->bitmap);
    }
    else
    {
        page = &start_page[slot[slot_idx].page_idx];
    }

    /* 2. Alloc small slot of memory from slab pool. */
    do
    {
        if(!shm_slab_is_busy_bitmap(page->bitmap))
        {
            i = (exp_bitmaps > 0)?exp_bitmaps:0;

            for(/* nothing */; i<SHM_SLAB_BITMAP_BITS; i++)
            {
                if((page->bitmap >> i) & 1)
                {
                    continue;   /* Used */
                }

                page->bitmap |= (1 << i);
                ptr = data + (page->index << shm_slab_page_shift());
                ptr += (i * (1 << shift));
                return ptr;
            }
        }
        else if(exp_bitmaps > 0)
        {
            /* �ɷ���ռ��ǰ����int����λͼ: ��չλͼ */
            bitmap = (int *)(data + (page->index << shm_slab_page_shift()));
            for(idx=0; idx<exp_bitmaps; idx++)
            {
                if(shm_slab_is_busy_bitmap(bitmap[idx]))
                {
                    continue;
                }
                
                for(i=0; i<SHM_SLAB_BITMAP_BITS; i++)
                {
                    if((bitmap[idx] >> i) & 1)
                    {
                        continue;   /* Used */
                    }

                    bitmap[idx] |= (1 << i);
                    ptr = data + (page->index << shm_slab_page_shift());
                    ptr += ((((idx+1) << SHM_SLAB_BITMAP_SHIFT) + i) * (1 << shift));
                    return ptr;
                }
            }
        }

        if(shm_slab_is_null_page(page->next_idx))
        {
            new_page = shm_slab_alloc_pages(pool, 1);
            if(NULL == new_page)
            {
                log_error("Alloc pages failed!");
                return NULL;
            }

            new_page->type = type;
            new_page->shift = shift;

            page->next_idx = new_page - start_page;
            new_page->prev_idx = page - start_page;

            page = new_page;

            /* Set bitmap */
            if(exp_bitmaps > 0)
            {
                for(i=0; i<exp_bitmaps; i++)
                {
                    page->bitmap |= (1 << i);   /* ��λ��1 */
                }
            }
            else
            {
                for(i=bits; i<SHM_SLAB_BITMAP_BITS; i++)
                {
                    page->bitmap |= (1 << i);   /* ��λ��1 */
                }
            }
            page->rbitmap = ~(page->bitmap);
            continue;
        }

        page = &start_page[page->next_idx];
    }while(1);

    log_error("Alloc slot failed!");
    return NULL;
}

/******************************************************************************
 **Name  : shm_slab_free_pages
 **Func  : Free special pages of slab pool.
 **Input : 
 **     pool: Object of slab pool.
 **     page: Specail pages of memory.
 **Output: NONE
 **Return: 0:Success    !0:Failed
 **Desc  : 
 **Note  : 
 **Author: # Qifeng.zou # 2013.07.15 #
 ******************************************************************************/
static int shm_slab_free_pages(shm_slab_pool_t *pool, shm_slab_page_t *page)
{
    int page_idx = 0;
    void *addr = NULL, *data = NULL;
    shm_slab_page_t *start_page = NULL, *free = NULL, *next = NULL;


    addr = (void *)pool;
    page->bitmap = 0;
    free = &pool->free;
    start_page = (shm_slab_page_t *)(addr + pool->page_offset);
    page_idx = page - start_page;
    data = addr + pool->data_offset + (page_idx << shm_slab_page_shift());

    memset(data, 0, shm_slab_page_size());

    if(shm_slab_is_null_page(free->next_idx))
    {
        free->next_idx = page_idx;
        page->prev_idx = SHM_SLAB_FREE_PAGE;
        return 0;
    }
    
    next = &start_page[free->next_idx];

    page->shift = shm_slab_page_shift();
    page->type = SHM_SLAB_ALLOC_UNKNOWN;
    page->next_idx = free->next_idx;
    page->prev_idx = next->prev_idx;
    next->prev_idx = page_idx;
    free->next_idx = page_idx;

    return 0;
}

#if defined(__XDT_NOT_USED__)
/******************************************************************************
 **Name  : shm_slab_slot_add_page
 **Func  : Add page into slot link.
 **Input : 
 **     pool: Object of slab pool.
 **     slot: Slot link.
 **     page: Object of added.
 **Output: 
 **Return: 0:Success    !0:Failed
 **Desc  : 
 **Note  : 
 **Author: # Qifeng.zou # 2013.07.17 #
 ******************************************************************************/
static int shm_slab_slot_add_page(
    shm_slab_pool_t *pool, shm_slab_slot_t *slot, shm_slab_page_t *page)
{
    void *addr = NULL;
    int page_idx = 0;
    shm_slab_page_t *start_page = NULL, *next = NULL;

    addr = (void *)pool;
    start_page = (shm_slab_page_t *)(addr + pool->page_offset);
    page_idx = page - start_page;
    
    if(shm_slab_is_null_page(slot->page_idx))
    {
        slot->page_idx = page_idx;
        return 0;
    }

    next = &start_page[slot->page_idx];
    page->next_idx = slot->page_idx;
    next->prev_idx = page_idx;
    slot->page_idx = page_idx;

    return 0;
}
#endif /*__XDT_NOT_USED__*/

/******************************************************************************
 **Name  : shm_slab_slot_remove_page
 **Func  : Remove page from slot link.
 **Input : 
 **     pool: Object of slab pool.
 **     slot: Slot link.
 **     page: Object of removed.
 **Output: 
 **Return: 0:Success    !0:Failed
 **Desc  : 
 **Note  : 
 **Author: # Qifeng.zou # 2013.07.17 #
 ******************************************************************************/
static int shm_slab_slot_remove_page(
    shm_slab_pool_t *pool, shm_slab_slot_t *slot, shm_slab_page_t *page)
{
    void *addr = NULL;
    shm_slab_page_t *start_page = NULL, *next = NULL, *prev = NULL;

    if(shm_slab_is_null_page(slot->page_idx))
    {
        log_error("Remove page failed!");
        return -1;
    }

    addr = (void *)pool;
    start_page = (shm_slab_page_t *)(addr + pool->page_offset);
    
    prev = &start_page[slot->page_idx];
    if(prev == page)
    {
        slot->page_idx = page->next_idx;
        if(!shm_slab_is_null_page(page->next_idx))
        {
            next = &start_page[page->next_idx];
            next->prev_idx = page->prev_idx;
        }

        page->prev_idx = SHM_SLAB_NULL_PAGE;
        page->next_idx = SHM_SLAB_NULL_PAGE;

        return 0;
    }

    while(!shm_slab_is_null_page(prev->next_idx))
    {
        next = &start_page[prev->next_idx];
        if(next != page)
        {
            prev = next;
            continue;
        }

        if(!shm_slab_is_null_page(page->next_idx))
        {
            next = &start_page[page->next_idx];
            next->prev_idx = page->prev_idx;
        }
        prev->next_idx = page->next_idx;
        page->next_idx = SHM_SLAB_NULL_PAGE;
        page->prev_idx = SHM_SLAB_NULL_PAGE;

        return 0;
    }

    log_error("Search page failed!");
    return -1;
}

/******************************************************************************
 **Name  : shm_slab_free
 **Func  : Free special memory.
 **Input : 
 **     pool: Object of slab pool.
 **     p: Object which is will be freed.
 **Output: 
 **Return: 0:Success    !0:Failed
 **Desc  : 
 **Note  : 
 **Author: # Qifeng.zou # 2013.07.12 #
 ******************************************************************************/
int shm_slab_free(shm_slab_pool_t *pool, void *p)
{
    int ret = 0, page_idx = 0,
        idx = 0, is_free = 1,
        bits = 0, bit_idx = 0, bitmap_idx = 0,
        slot_idx = 0, exp_bitmaps = 0;
    int *bitmap = NULL;
    size_t offset = 0;
    shm_slab_slot_t *slot = NULL;
    shm_slab_page_t *page = NULL;
    void *addr = NULL, *start = NULL;

    addr = (void *)pool;
    offset = (void *)p - addr;
    
    if((offset < pool->data_offset) || (offset > pool->end_offset))
    {
        log_error("Pointer address is incorrect!");
        return -1;
    }

    slot = (shm_slab_slot_t *)(addr + pool->slot_offset);

    page_idx = ((offset - pool->data_offset) >> shm_slab_page_shift());
    start = (addr + pool->data_offset) + (page_idx << shm_slab_page_shift());
    page = (shm_slab_page_t *)(addr + pool->page_offset
                                    + page_idx * sizeof(shm_slab_page_t));
    switch(page->type)
    {
        case SHM_SLAB_ALLOC_SMALL:
        {
            bit_idx = (p - start) >> page->shift;
            bitmap = (int *)start;
            bitmap_idx = (bit_idx >> SHM_SLAB_BITMAP_SHIFT);
            bits = (shm_slab_page_size() >> page->shift);
            exp_bitmaps = (bits >> SHM_SLAB_BITMAP_SHIFT) - 1;

            memset(p, 0, 1 << page->shift);

            if(0 == bitmap_idx)
            {
                page->bitmap &= ~(1 << bit_idx);
            }
            else
            {
                bitmap[bitmap_idx-1] &= ~(1 << bit_idx);
            }

            for(idx=0; idx<exp_bitmaps; idx++)
            {
                if(0 != bitmap[idx])
                {
                    is_free = 0;
                    break;
                }
            }
            
            if((1 == is_free) && !(page->bitmap & page->rbitmap))
            {
                slot_idx = page->shift - pool->min_shift;
                
                /* Remove from slot[idx] link */
                ret = shm_slab_slot_remove_page(pool, slot+slot_idx, page);
                if(ret < 0)
                {
                    log_error("Remove page failed!");
                    return -1;
                }

                /* Add into free link */
                return shm_slab_free_pages(pool, page);
            }
            return 0;
        }
        case SHM_SLAB_ALLOC_EXACT:
        case SHM_SLAB_ALLOC_LARGE:
        {
            bit_idx = (p - start) >> page->shift;

            memset(p, 0, 1 << page->shift);
            
            page->bitmap &= ~(1 << bit_idx);
            
            if(!(page->bitmap & page->rbitmap))
            {
                slot_idx = page->shift - pool->min_shift;
                
                /* Remove from slot[idx] link */
                ret = shm_slab_slot_remove_page(pool, slot+slot_idx, page);
                if(ret < 0)
                {
                    log_error("Remove page failed!");
                    return -1;
                }

                /* Add into free link */
                return shm_slab_free_pages(pool, page);
            }
            return 0;
        }
        case SHM_SLAB_ALLOC_PAGES:
        {
            return shm_slab_free_pages(pool, page);
        }
        default:
        {
            log_error("Type is unknown! [%d]", page->type);
            return -1;
        }
    }

    return 0;
}

/******************************************************************************
 **Name  : shm_slab_head_size
 **Func  : Compute the head size of slab pool.
 **Input : 
 **     size: Slab pool size.
 **Output: 
 **Return: The head size of slab pool.
 **Desc  : 
 **Note  : 
 **Author: # Qifeng.zou # 2013.07.18 #
 ******************************************************************************/
size_t shm_slab_head_size(size_t size)
{
    return sizeof(shm_slab_pool_t)
            + (shm_slab_max_shift() - SHM_SLAB_MIN_SHIFT) * sizeof(shm_slab_slot_t)
            + (size >> shm_slab_page_shift()) * sizeof(shm_slab_page_t);
}
