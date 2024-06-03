#include <net/dst.h>

/*------------------------------------------------------------------------------------
 *BLOG dst id map table and its related functions, adopted from bcmnetdev
 *-----------------------------------------------------------------------------------*/
#define BLOG_DSTENTRY_MAX_BITS      (7)
#define BLOG_DSTENTRY_MAX_ENTRIES   (1 << BLOG_DSTENTRY_MAX_BITS)

#define BLOG_DSTENTRY_IDX_MASK      (BLOG_DSTENTRY_MAX_ENTRIES - 1)
#define BLOG_DSTENTRY_INCARN_BITS   (4)
#define AVAIL_BITMAP_NUM_WORDS         (BLOG_DSTENTRY_MAX_ENTRIES / (sizeof(unsigned long) * 8))
#define BLOG_DSTENTRY_RESERVE_BITS  ((sizeof(uint16_t) * 8) - (BLOG_DSTENTRY_MAX_BITS + BLOG_DSTENTRY_INCARN_BITS))

#define blog_dstentry_id_get_dstid(x)     (x.dstid)
#define blog_dstentry_id_get_idx(x)       (x.bmp.idx)
#define blog_dstentry_id_get_incarn(x)    (x.bmp.incarn)

typedef struct {
    union {
       struct {
           uint16_t idx:BLOG_DSTENTRY_MAX_BITS;
           uint16_t incarn:BLOG_DSTENTRY_INCARN_BITS;
           uint16_t reserved:BLOG_DSTENTRY_RESERVE_BITS;
       }bmp;
       uint16_t dstid;
    };
}blog_dstentry_id_t;

typedef struct {
    atomic_t user_count;
    blog_dstentry_id_t dstid;
    struct dst_entry *dst;
}blog_dstentry_dstid_map_ent_t;

typedef struct {
    spinlock_t lock;
    unsigned long free_idx_bitmap[AVAIL_BITMAP_NUM_WORDS];/* bitmap for availability */
    blog_dstentry_dstid_map_ent_t ent[BLOG_DSTENTRY_MAX_ENTRIES];
	/* TODO maintain an last alloc index, so we try to allocate incrementally irrespective of alloc/free  */
}blog_dstentry_dstid_tbl_t;

static blog_dstentry_dstid_tbl_t blog_dstentry_id_tbl;
static blog_dstentry_dstid_tbl_t *blog_dstentry_id_tbl_p = NULL;

#define BLOG_DSTENTRY_LOCK(lock)         spin_lock_bh(lock)
#define BLOG_DSTENTRY_UNLOCK(lock)       spin_unlock_bh(lock)

static inline int get_idx(uint16_t *pdx)
{
    static int offset = 1;
    uint16_t idx = 0;
    volatile unsigned long *addr;
    addr = &blog_dstentry_id_tbl_p->free_idx_bitmap[0];

    idx = find_next_bit((const unsigned long *)addr, BLOG_DSTENTRY_MAX_ENTRIES, offset);
    if((idx == BLOG_DSTENTRY_MAX_ENTRIES) && (offset !=1))
    {
        /* if entry not found and we did not start from beginning(offset 1)
         * do a second lookup, to find any free entires from 1 - offset 
         */
        offset = 1;
        idx = find_next_bit((const unsigned long *)addr, BLOG_DSTENTRY_MAX_ENTRIES, offset);
    }

    if ((idx > 0) && (idx < BLOG_DSTENTRY_MAX_ENTRIES))
    {
        *pdx = idx;
        clear_bit(*pdx, addr);

        /* update offset for next allocation */
        if (idx == (BLOG_DSTENTRY_MAX_ENTRIES - 1))
            offset = 1;
        else
            offset = idx + 1;

        return 0;
    }
    return -1;
}

static void put_idx(uint16_t *pdx)
{
    volatile unsigned long *addr;
    if (*pdx < BLOG_DSTENTRY_MAX_ENTRIES)
    {
        addr = &blog_dstentry_id_tbl_p->free_idx_bitmap[0];
        set_bit(*pdx, addr);
    }
}



/*
 * Allocates an entry in blog_dstentry_id_tbl_t to create a dstentry  mapping.
 */
static inline int alloc_dstid(struct dst_entry *dst_p, uint16_t *dstid_p)
{
    int status = 0;
    uint16_t idx = 0;
    blog_dstentry_dstid_map_ent_t *pentry;
		
    BLOG_DSTENTRY_LOCK(&blog_dstentry_id_tbl_p->lock);
    if (get_idx(&idx))
    {
        status = -1;
        printk(KERN_ERR "%s:invalid idx for dstentry dstid mapping [%d]\n",__FUNCTION__,idx);
        goto assign_exit;
    }
    pentry = &blog_dstentry_id_tbl_p->ent[idx];
    if (atomic_read(&pentry->user_count) == 0)
    {
        dst_hold(dst_p);
        pentry->dst = dst_p;
        atomic_inc(&pentry->user_count);
        *dstid_p = pentry->dstid.dstid;
    }
    else
    {
        status = -1;
        printk(KERN_ERR "user count is not zero something is wrong\n");
    }
    //printk(KERN_INFO "Assigning idx[%d:%d] \n",pentry->dstid.bmp.incarn, pentry->dstid.bmp.idx);
assign_exit:
    BLOG_DSTENTRY_UNLOCK(&blog_dstentry_id_tbl_p->lock);
    return status;
}

static inline void free_dstentry_entry(blog_dstentry_dstid_map_ent_t *pentry, uint16_t idx)
{
    struct dst_entry *dst_p;
    BLOG_DSTENTRY_LOCK(&blog_dstentry_id_tbl_p->lock);
    put_idx(&idx);
    dst_p = pentry->dst;
    pentry->dst = NULL;
    BLOG_DSTENTRY_UNLOCK(&blog_dstentry_id_tbl_p->lock);
    dst_release(dst_p);
}
/*
 *  Frees the entry associated to the dstid once the user_count reaches zero.
 */
static inline int free_dstid(uint16_t dstid)
{
    int status = 0;
    blog_dstentry_dstid_map_ent_t *pentry;
    uint16_t idx    = (dstid & BLOG_DSTENTRY_IDX_MASK);
    if (unlikely((idx == 0) || (idx >= BLOG_DSTENTRY_MAX_ENTRIES)))
    {
        status = -1;
        printk(KERN_ERR "%s:invalid idx for dstentry dstid mapping [%d]\n",__FUNCTION__,idx);
        goto free_exit;
    }
    pentry = &blog_dstentry_id_tbl_p->ent[idx];
    if (unlikely(dstid != pentry->dstid.dstid))
    {
        status = -1;
        printk(KERN_ERR "%s:dstid[%d] doesn't match with dstid in table[%d]\n",__FUNCTION__,
                                                                     dstid,
                                                                     pentry->dstid.dstid);
        goto free_exit;
    }
    if (atomic_read(&pentry->user_count) > 0)
    {
        if (atomic_dec_and_test(&pentry->user_count))
        {
            /*printk(KERN_INFO "Freeing dstid[%d:%d] for %px\n",pentry->dstid.bmp.incarn,
					pentry->dstid.bmp.incarn,pentry->dst);*/
            /*users is zero now delete this entry*/
            pentry->dstid.bmp.incarn++;
            if (pentry->dstid.bmp.incarn == 0)
                pentry->dstid.bmp.incarn = 1;
            free_dstentry_entry(pentry, idx);
        }
    }
    else
    {
		printk(KERN_ERR "%s:usercount is invalid for dstid %d\n",__FUNCTION__,
				pentry->dstid.dstid);
    }
free_exit:
    return status;
}

static uint16_t blog_dstid_lookup(struct dst_entry *dst_p)
{
    int idx;
    blog_dstentry_dstid_map_ent_t *pentry;
    unsigned long *addr = &blog_dstentry_id_tbl_p->free_idx_bitmap[0];

    /* bit value of zero indicates used here */
    for (idx = find_next_zero_bit(addr, BLOG_DSTENTRY_MAX_ENTRIES, 1);
            idx < BLOG_DSTENTRY_MAX_ENTRIES;
            idx = find_next_zero_bit(addr, BLOG_DSTENTRY_MAX_ENTRIES, idx+1))
    {
        pentry = &blog_dstentry_id_tbl_p->ent[idx];
        if (atomic_read(&pentry->user_count) > 0)
        {	
            if((pentry->dst == dst_p) && atomic_inc_not_zero(&pentry->user_count))
            {
                return pentry->dstid.dstid;
            }
        }
    }
    return 0;
}
/*
 *  Given a dstentry pointer allocate a dstid 
 */
int blog_get_dstentry_id(struct dst_entry *dst_p, uint16_t *dstid_p)
{
	uint16_t dstid = blog_dstid_lookup(dst_p);
	int ret;
	
	if(dstid)
	{
		*dstid_p = dstid;
		ret = 0;
	} 
	else
		ret = alloc_dstid(dst_p, dstid_p);

    if(0)
    {
        blog_dstentry_id_t id_map;
        id_map.dstid = *dstid_p;
        printk(KERN_INFO "Assigning idx[%d:%d] \n", id_map.bmp.incarn, id_map.bmp.idx);
    }
    return ret;
}
EXPORT_SYMBOL(blog_get_dstentry_id);
/*
 *  Given a dstid locates mapped dstentry mapping entry and frees it.
 */
void blog_put_dstentry_id(uint16_t dstid)
{
    free_dstid(dstid);
}
EXPORT_SYMBOL(blog_put_dstentry_id);

/*
 *  Given a dstid locates mapped dstentry pointer and also increases its
 *  user_count. user of this API should takecare of dst_release once done.
 */
struct dst_entry *blog_get_dstentry_by_id(uint16_t dstid)
{
    blog_dstentry_dstid_map_ent_t *pentry;
    uint16_t idx    = (dstid & BLOG_DSTENTRY_IDX_MASK);
    pentry = &blog_dstentry_id_tbl_p->ent[idx];
    if (likely(pentry->dstid.dstid == dstid))
    {
        if (atomic_inc_not_zero(&pentry->user_count)) 
        {
            struct dst_entry *dst = pentry->dst;
            dst_hold(dst);
            atomic_dec(&pentry->user_count);
            return dst;
        }
        else
            printk_ratelimited(KERN_DEBUG "%s:Trying to get dstentry pointer for idx[%d] dstid[%d] but zero user_count\n",
                    __FUNCTION__, idx, dstid);
    }
    else
    {
        printk_ratelimited(KERN_DEBUG "%s:dstid[%d] match fails entry.dstid[%d]\n",__FUNCTION__,
                dstid, pentry->dstid.dstid);
    }
    return NULL;
}
EXPORT_SYMBOL(blog_get_dstentry_by_id);

/*
 * Initializes the blog_blog_dstentry_id_tbl.  
 */
int blog_dstentry_id_tbl_init(void)
{
    int idx;
    blog_dstentry_id_tbl_p = &blog_dstentry_id_tbl;
    memset(blog_dstentry_id_tbl_p, 0, sizeof(blog_dstentry_dstid_tbl_t));
    spin_lock_init(&blog_dstentry_id_tbl_p->lock);
    for (idx=0; idx < BLOG_DSTENTRY_MAX_ENTRIES; idx++)
    {
        blog_dstentry_id_tbl_p->ent[idx].dstid.bmp.idx     = idx;
        blog_dstentry_id_tbl_p->ent[idx].dstid.bmp.incarn = 1;
        blog_dstentry_id_tbl_p->ent[idx].dst    = (struct dst_entry *)NULL;
        atomic_set(&blog_dstentry_id_tbl_p->ent[idx].user_count,0);
        /*Mark this ent to be available for mapping */
        set_bit(idx,&blog_dstentry_id_tbl_p->free_idx_bitmap[0]);
    }
    /* Mark the very first index with dstid as 0 to be non-available */
    clear_bit(0,&blog_dstentry_id_tbl_p->free_idx_bitmap[0]);
    return 0;
}

#if 0
/*
 * Function to dump mapping table. 
 */
static void blog_dstid_dump(void)
{
    int idx =0;
    blog_dstentry_dstid_map_ent_t *pentry;
    for (idx=0;idx < BLOG_DSTENTRY_MAX_ENTRIES; idx++)
    {
        pentry = &blog_dstentry_id_tbl_p->ent[idx];
        if (atomic_read(&pentry->user_count) > 0)
        {
            printk("dstid:[%d] idx:[%d] dst:%px \n",pentry->dstid.dstid,
                                                           pentry->dstid.bmp.idx,
                                                           pentry->dst);
        }
    }
    printk("\n");
    return;
}
#endif
