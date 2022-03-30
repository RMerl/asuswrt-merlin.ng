/*
 * <:copyright-BRCM:2015:GPL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 */
 
#include "gki_int.h"
#include "btusb.h"

#if (GKI_NUM_TOTAL_BUF_POOLS > 16)
#error Number of pools out of range (16 Max)!
#endif

static void gki_add_to_pool_list(UINT8 pool_id);
static void gki_remove_from_pool_list(UINT8 pool_id);

static BOOL _b_in_gki_getbuf = FALSE;

/*******************************************************************************
**
** Function         gki_init_free_queue
**
** Description      Internal function called at startup to initialize a free
**                  queue. It is called once for each free queue.
**
** Returns          void
**
*******************************************************************************/
static BOOL gki_init_free_queue(UINT8 id, UINT16 size, UINT16 total, void *p_mem)
{
    UINT16           i;
    UINT16           act_size;
    BUFFER_HDR_T    *hdr;
    BUFFER_HDR_T    *hdr1 = NULL;
    UINT32          *magic;
    int              seg_inx = gki_cb.pool_additions[id];
    FREE_QUEUE_T    *p_freeq = &gki_cb.freeq[id];

    if (seg_inx >= MAX_BUFFPOOL_SEGS)
    {
        GKI_exception(GKI_ERROR_SEGS_EXCEEDED, "Max segs exceeded");
        return(FALSE);
    }

    act_size = (UINT16)(size + BUFFER_PADDING_SIZE);

    if (!p_mem)
        p_mem = gki_reserve_os_memory(act_size * total);

    if (!p_mem)
    {
        GKI_exception(GKI_ERROR_NO_MEMORY_FOR_SEG, "No memory for segment");
        return(FALSE);
    }

    /* Remember pool start and end addresses */
    p_freeq->seg_start[seg_inx] = (UINT8 *)p_mem;
    p_freeq->seg_end[seg_inx]   = (UINT8 *)p_mem + (act_size * total);

    if (seg_inx == 0)
    {
        gki_cb.freeq[id].total     = total;
        gki_cb.freeq[id].cur_cnt   = 0;
        gki_cb.freeq[id].max_cnt   = 0;
    }
    else
        gki_cb.freeq[id].total     += total;

    /* Initialize  index table */
    hdr = (BUFFER_HDR_T *)p_mem;
    p_freeq->p_first = hdr;

#ifdef TRACE_GKI_BUFFERS
    gki_cb.freeq[id].p_first_all = hdr;
#endif

    for (i = 0; i < total; i++)
    {
        hdr->task_id = GKI_INVALID_TASK;
        hdr->q_id    = id;
        hdr->status  = BUF_STATUS_FREE;
        magic        = (UINT32 *)((UINT8 *)hdr + BUFFER_HDR_SIZE + size);
        *magic       = MAGIC_NO;
        hdr1         = hdr;
        hdr          = (BUFFER_HDR_T *)((UINT8 *)hdr + act_size);
        hdr1->p_next = hdr;
#ifdef TRACE_GKI_BUFFERS
        hdr1->p_next_all = hdr;
        hdr1->pFile = NULL;
        hdr1->linenum = 0;
        hdr1->times_alloc = 0;
#endif
    }

    hdr1->p_next = NULL;
    p_freeq->p_last = hdr1;

    gki_cb.pool_additions[id]++;

    return(TRUE);
}


/*******************************************************************************
**
** Function         gki_buffer_init
**
** Description      Called once internally by GKI at startup to initialize all
**                  buffers and free buffer pools.
**
** Returns          void
**
*******************************************************************************/
void gki_buffer_init(void)
{
    UINT8   tt;
    UINT16  access_mask = GKI_DEF_BUFPOOL_PERM_MASK;

#ifdef TRACE_GKI_BUFFERS
    char    aString[200];
    sprintf(aString,"%s: %d: WARNING! Running with GKI buffer tracing enabled!!",__FILE__,__LINE__);
    GKI_exception(GKI_ERROR_BUFFER_TRACE_ON,aString);
#endif

    for (tt = 0; tt < GKI_NUM_TOTAL_BUF_POOLS; tt++)
    {
        memset(&gki_cb.freeq[tt], 0, sizeof(FREE_QUEUE_T));

        gki_cb.pool_buf_size[tt]  = 0;
        gki_cb.pool_max_count[tt] = 0;
        gki_cb.pool_additions[tt] = 0;
    }

    /* Use default from target.h */
    gki_cb.pool_access_mask = GKI_DEF_BUFPOOL_PERM_MASK;

#if (GKI_USE_DYNAMIC_BUFFERS == TRUE)

#if (GKI_NUM_FIXED_BUF_POOLS > 0)
    GKI_create_pool(GKI_BUF0_SIZE, GKI_BUF0_MAX, (UINT8) (access_mask & 1), NULL);
    access_mask = access_mask >> 1;
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 1)
    GKI_create_pool(GKI_BUF1_SIZE, GKI_BUF1_MAX, (UINT8) (access_mask & 1), NULL);
    access_mask = access_mask >> 1;
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 2)
    GKI_create_pool(GKI_BUF2_SIZE, GKI_BUF2_MAX, (UINT8) (access_mask & 1), NULL);
    access_mask = access_mask >> 1;
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 3)
    GKI_create_pool(GKI_BUF3_SIZE, GKI_BUF3_MAX, (UINT8) (access_mask & 1), NULL);
#endif

#else

/* Static buffers */

#if (GKI_NUM_FIXED_BUF_POOLS > 0)
    GKI_create_pool(GKI_BUF0_SIZE, GKI_BUF0_MAX, (UINT8) (access_mask & 1), gki_cb.bufpool0);
    access_mask = access_mask >> 1;
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 1)
    GKI_create_pool(GKI_BUF1_SIZE, GKI_BUF1_MAX, (UINT8) (access_mask & 1), gki_cb.bufpool1);
    access_mask = access_mask >> 1;
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 2)
    GKI_create_pool(GKI_BUF2_SIZE, GKI_BUF2_MAX, (UINT8) (access_mask & 1), gki_cb.bufpool2);
    access_mask = access_mask >> 1;
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 3)
    GKI_create_pool(GKI_BUF3_SIZE, GKI_BUF3_MAX, (UINT8) (access_mask & 1), gki_cb.bufpool3);
#endif

#endif
}


/*******************************************************************************
**
** Function         GKI_set_pool_permission
**
** Description      This function is called to set or change the permissions for
**                  the specified pool ID.
**
** Parameters       pool_id -       (input) pool ID to be set or changed
**                  permission -    (input) GKI_PUBLIC_POOL or GKI_RESTRICTED_POOL
**
** Returns          GKI_SUCCESS if successful
**                  GKI_INVALID_POOL if unsuccessful
**
*******************************************************************************/
UINT8 GKI_set_pool_permission(UINT8 pool_id, UINT8 permission)
{
    if (pool_id < GKI_NUM_TOTAL_BUF_POOLS)
    {
        if (permission == GKI_RESTRICTED_POOL)
            gki_cb.pool_access_mask |= (1 << pool_id);
        else    /* mark the pool as public */
            gki_cb.pool_access_mask &= ~(1 << pool_id);
        
        return(GKI_SUCCESS);
    }
    else
        return(GKI_INVALID_POOL);
}


/*******************************************************************************
**
** Function         gki_add_to_pool_list
**
** Description      Adds pool to the pool list which is arranged in the 
**                  order of size
**
** Returns          void
**
*******************************************************************************/
static void gki_add_to_pool_list(UINT8 pool_id)
{
    INT32 i, j;

     /* Find the position where the specified pool should be inserted into the list */
    for (i = 0; i < gki_cb.curr_total_no_of_pools; i++) 
    {
        if (gki_cb.pool_buf_size[pool_id] <= gki_cb.pool_buf_size[gki_cb.pool_list[i]])
            break;
    }

    /* Insert the new buffer pool ID into the list of pools */
    for (j = gki_cb.curr_total_no_of_pools; j > i; j--)
    {
        gki_cb.pool_list[j] = gki_cb.pool_list[j-1];
    }

    /* Prevent warning */
    if ( i < GKI_NUM_TOTAL_BUF_POOLS)
    {
        gki_cb.pool_list[i] = pool_id;
    }
}


/*******************************************************************************
**
** Function         gki_remove_from_pool_list
**
** Description      Removes pool from the pool list. Called when a pool is deleted 
**
** Returns          void
**
*******************************************************************************/
static void gki_remove_from_pool_list(UINT8 pool_id)
{
    INT8   i;
 
    for (i = 0; i < gki_cb.curr_total_no_of_pools; i++)
    {
        if (pool_id == gki_cb.pool_list[i])
            break;
    }

/* Prevent warning.
 * Since GKI_NUM_TOTAL_BUF_POOLS value is 1, this code was not needed */
/*
    while (i < (GKI_NUM_TOTAL_BUF_POOLS - 1))
    {
        gki_cb.pool_list[i] = gki_cb.pool_list[i+1];
        i++;
    }
*/
    return;
}


/*******************************************************************************
**
** Function         GKI_init_q
**
** Description      Called by an application to initialize a buffer queue.
**
** Returns          void
**
*******************************************************************************/
void GKI_init_q(BUFFER_Q *p_q)
{
    p_q->p_first = p_q->p_last = NULL;
    p_q->count   = 0;

    return;
}


/*******************************************************************************
**
** Function         GKI_getbuf
**
** Description      Called by an application to get a free buffer which
**                  is of size greater or equal to the requested size.
**
**                  Note: This routine only takes buffers from public pools.
**                        It will not use any buffers from pools
**                        marked GKI_RESTRICTED_POOL.
**
** Parameters       size - (input) number of bytes needed.
**
** Returns          A pointer to the buffer, or NULL if none available
**
*******************************************************************************/
#ifdef TRACE_GKI_BUFFERS
void *GKI_getbuf_trace(UINT16 size, char *pFile, int linenum)
#else
void *GKI_getbuf(UINT16 size)
#endif
{
    UINT8           i, pool_id;
    void            *pp;

    if (unlikely(size == 0))
    {
        GKI_exception(GKI_ERROR_BUF_SIZE_ZERO, "getbuf: Size is zero");
        return(NULL);
    }

    /* Find the first buffer pool that is public that can hold the desired size */
    for (i = 0; i < gki_cb.curr_total_no_of_pools; i++)
    {
        if (gki_cb.pool_buf_size[gki_cb.pool_list[i]] >= size)
            break;
    }

    if (i == gki_cb.curr_total_no_of_pools)
    {
        char buff[40];
        snprintf(buff, sizeof(buff), "getbuf: Size: %u is too big", size);
        GKI_exception(GKI_ERROR_BUF_SIZE_TOOBIG, buff);
        return(NULL);
    }

    _b_in_gki_getbuf = TRUE;

    /* search the public buffer pools that are big enough to hold the size
     * until a free buffer is found */ 
    for ( ; i < gki_cb.curr_total_no_of_pools; i++)
    {
        pool_id = gki_cb.pool_list[i];

        /* Only look at PUBLIC buffer pools (bypass RESTRICTED pools) */
        if (((UINT16)1 << pool_id) & gki_cb.pool_access_mask)
            continue;

        if ((pp = GKI_getpoolbuf(pool_id)) != NULL)
        {
            _b_in_gki_getbuf = FALSE;
            return(pp);
        }
    }

    _b_in_gki_getbuf = FALSE;

    return(NULL);
}


/*******************************************************************************
**
** Function         GKI_getpoolbuf
**
** Description      Called by an application to get a free buffer from
**                  a specific buffer pool.
**
**                  Note: If there are no more buffers available from the pool,
**                        the public buffers are searched for an available buffer.
**
** Parameters       pool_id - (input) pool ID to get a buffer out of.
**
** Returns          A pointer to the buffer, or NULL if none available
**
*******************************************************************************/
#ifdef TRACE_GKI_BUFFERS
void *GKI_getpoolbuf_trace(UINT8 pool_id, char *pFile, int linenum)
#else
void *GKI_getpoolbuf(UINT8 pool_id)
#endif
{
    FREE_QUEUE_T  *Q;
    BUFFER_HDR_T  *p_hdr;

    if (pool_id >= GKI_NUM_TOTAL_BUF_POOLS)
        return(NULL);

    /* Make sure the buffers aren't disturbed til finished with allocation */
    GKI_disable();

    Q = &gki_cb.freeq[pool_id];

    /* If we have no buffers left, see if we can allocate another segment */
    if ((Q->cur_cnt >= Q->total) && (gki_cb.pool_additions[pool_id] < MAX_BUFFPOOL_SEGS))
    {
        UINT16 count = (gki_cb.pool_max_count[pool_id] + MAX_BUFFPOOL_SEGS - 1) / MAX_BUFFPOOL_SEGS;

        gki_init_free_queue(pool_id, gki_cb.pool_buf_size[pool_id], count, NULL);
    }

    if (Q->cur_cnt < Q->total)
    {
        p_hdr = Q->p_first;
        Q->p_first = p_hdr->p_next;

        if (!Q->p_first)
            Q->p_last = NULL;

        if (++Q->cur_cnt > Q->max_cnt)
            Q->max_cnt = Q->cur_cnt;

        p_hdr->status  = BUF_STATUS_UNLINKED;
        p_hdr->p_next  = NULL;
        p_hdr->Type    = 0;

        GKI_enable();
#ifdef TRACE_GKI_BUFFERS
        p_hdr->pFile = pFile;
        p_hdr->linenum = linenum;
        p_hdr->times_alloc++;
        /* The following is here to allow the test engineer to recognize and */
        /* set a breakpoint when a particular buffer is allocated for the nth time. */
        /* Simply change the address and allocation number in the following 'if' */
        /* statement to reflect the buffer and occurance desired. */
        if ((p_hdr == (BUFFER_HDR_T *)0x12345678) && (p_hdr->times_alloc == 123))
            p_hdr->times_alloc = 123;
#endif

        return((void *) ((UINT8 *)p_hdr + BUFFER_HDR_SIZE));
    }

    /* If here, no buffers in the specified pool */
    GKI_enable();

    /* Try for free buffers in public pools. NOTE - no recursion allowed */
    if (!_b_in_gki_getbuf)
        return(GKI_getbuf(gki_cb.pool_buf_size[pool_id]));
    else
        return(NULL);
}


/*******************************************************************************
**
** Function         GKI_freebuf
**
** Description      Called by an application to return a buffer to the free pool.
**
** Parameters       p_buf - (input) address of the beginning of a buffer.
**
** Returns          void
**
*******************************************************************************/
void GKI_freebuf(void *p_buf)
{
    FREE_QUEUE_T *Q;
    BUFFER_HDR_T *p_hdr;
    void         *pNextBuf;

#if (GKI_ENABLE_BUF_CORRUPTION_CHECK == TRUE)
    if (!p_buf || gki_chk_buf_damage(p_buf))
    {
        GKI_exception(GKI_ERROR_BUF_CORRUPTED, "Free - Buf Corrupted");
       return;
    }
#endif

    p_hdr = (BUFFER_HDR_T *) ((UINT8 *)p_buf - BUFFER_HDR_SIZE);

    if (p_hdr->status != BUF_STATUS_UNLINKED)
    {
        GKI_exception(GKI_ERROR_FREEBUF_BUF_LINKED, "Freeing Linked Buf");
        return;
    }

    if (p_hdr->q_id >= GKI_NUM_TOTAL_BUF_POOLS)
    {
        GKI_exception(GKI_ERROR_FREEBUF_BAD_QID, "Bad Buf QId");
        return;
    }

#ifdef TRACE_GKI_BUFFERS
    if ((p_hdr->pFile == NULL) || (p_hdr->linenum == 0))
    {
        GKI_exception(GKI_ERROR_BUF_CORRUPTED, "Free - no file or line number");
    }
#endif

    GKI_disable();

    /*
    ** Releasing all buffers in the linked list
    */
    while (p_buf)
    {
        p_hdr = (BUFFER_HDR_T *) ((UINT8 *)p_buf - BUFFER_HDR_SIZE);

        pNextBuf = NULL;
        Q  = &gki_cb.freeq[p_hdr->q_id];
        if (Q->p_last)
            Q->p_last->p_next = p_hdr;
        else
            Q->p_first = p_hdr;

        Q->p_last      = p_hdr;
        p_hdr->p_next  = NULL;
        p_hdr->status  = BUF_STATUS_FREE;
        p_hdr->task_id = GKI_INVALID_TASK;
#ifdef TRACE_GKI_BUFFERS
        p_hdr->pFile = NULL;
        p_hdr->linenum = 0;
#endif
        if (Q->cur_cnt > 0)
            Q->cur_cnt--;

        p_buf = pNextBuf;
    }

    GKI_enable();

    return;
}

#if (GKI_ENABLE_BUF_CORRUPTION_CHECK == TRUE)
/*******************************************************************************
**
** Function         GKI_get_buf_size
**
** Description      Called by an application to get the size of a buffer.
**
** Parameters       p_buf - (input) address of the beginning of a buffer.
**
** Returns          the size of the buffer
**
*******************************************************************************/
UINT16 GKI_get_buf_size(void *p_buf)
{
    BUFFER_HDR_T *p_hdr;

    p_hdr = (BUFFER_HDR_T *)((UINT8 *) p_buf - BUFFER_HDR_SIZE);

    if ((uintptr_t)p_hdr & 1)
    {
        return(0);
    }

    if (p_hdr->q_id < GKI_NUM_TOTAL_BUF_POOLS)
    {
        return(gki_cb.pool_buf_size[p_hdr->q_id]);
    }
    else if (p_hdr->q_id == GKI_NUM_TOTAL_BUF_POOLS)
    {
        return offsetof(struct btusb_rx_trans, magic) - offsetof(struct btusb_rx_trans, bt_hdr);
    }
    else if (p_hdr->q_id == (GKI_NUM_TOTAL_BUF_POOLS + 1))
    {
        return offsetof(struct btusb_voice_pkt, magic) - offsetof(struct btusb_voice_pkt, bt_hdr);
    }


    return(0);
}
#endif

/*******************************************************************************
**
** Function         GKI_get_pool_bufsize
**
** Description      Called by an application to get the size of buffers in a pool
**
** Parameters       Pool ID.
**
** Returns          the size of buffers in the pool
**
*******************************************************************************/
UINT16 GKI_get_pool_bufsize(UINT8 pool_id)
{
    if (pool_id < GKI_NUM_TOTAL_BUF_POOLS)
        return(gki_cb.pool_buf_size[pool_id]);

    return(0);
}

/*******************************************************************************
**
** Function         GKI_poolfreecount
**
** Description      Called by an application to get the number of free buffers
**                  in the specified buffer pool.
**
** Parameters       pool_id - (input) pool ID to get the free count of.
**
** Returns          the number of free buffers in the pool
**
*******************************************************************************/
UINT16 GKI_poolfreecount(UINT8 pool_id)
{
    FREE_QUEUE_T  *Q;

    if (pool_id >= GKI_NUM_TOTAL_BUF_POOLS)
        return(0);

    Q  = &gki_cb.freeq[pool_id];

    return((UINT16)(Q->total - Q->cur_cnt));
}


/*******************************************************************************
**
** Function         GKI_poolutilization
**
** Description      Called by an application to get the buffer utilization
**                  in the specified buffer pool.
**
** Parameters       pool_id - (input) pool ID to get the free count of.
**
** Returns          % of buffers used from 0 to 100
**
*******************************************************************************/
UINT16 GKI_poolutilization(UINT8 pool_id)
{
    FREE_QUEUE_T  *Q;

    if (pool_id >= GKI_NUM_TOTAL_BUF_POOLS)
        return(100);

    Q  = &gki_cb.freeq[pool_id];

    if (Q->total == 0)
        return(100);

    return((Q->cur_cnt * 100) / Q->total);
}



/*******************************************************************************
**
** Function         gki_chk_buf_damage
**
** Description      Called internally by OSS to check for buffer corruption.
**
** Returns          TRUE if there is a problem, else FALSE
**
*******************************************************************************/
BOOLEAN gki_chk_buf_damage(void *p_buf)
{
#if (GKI_ENABLE_BUF_CORRUPTION_CHECK == TRUE)

    UINT32 *magic;
    magic  = (UINT32 *)((UINT8 *) p_buf + GKI_get_buf_size(p_buf));

    if ((uintptr_t)magic & 1)
        return(TRUE);

    if (*magic == MAGIC_NO)
        return(FALSE);

    return(TRUE);

#else

    return(FALSE);

#endif
}

/*******************************************************************************
**
** Function         gki_chk_buf_owner
**
** Description      Called internally by OSS to check if the current task
**                  is the owner of the buffer.
**
** Returns          TRUE if not owner, else FALSE
**
*******************************************************************************/
BOOLEAN gki_chk_buf_owner(void *p_buf)
{
    return(FALSE);
}


/*******************************************************************************
**
** Function         GKI_change_buf_owner
**
** Description      Called to change the task ownership of a buffer.
**
** Parameters:      p_buf   - (input) pointer to the buffer
**                  task_id - (input) task id to change ownership to
**
** Returns          void
**
*******************************************************************************/
void GKI_change_buf_owner(void *p_buf, UINT8 task_id)
{
    BUFFER_HDR_T    *p_hdr = (BUFFER_HDR_T *) ((UINT8 *) p_buf - BUFFER_HDR_SIZE);

    p_hdr->task_id = task_id;

    return;
}



/*******************************************************************************
**
** Function         GKI_buffer_status
**
** Description      check status of the buffer to see if it is linked
**
** Parameters:      p_buf - (input) address of the buffer to enqueue
**
** Returns          state
*        BUF_STATUS_FREE     0
*        BUF_STATUS_UNLINKED 1
*        BUF_STATUS_QUEUED   2
**
*******************************************************************************/
UINT8 GKI_buffer_status(void *p_buf)
{
    BUFFER_HDR_T *p_hdr;

    p_hdr = (BUFFER_HDR_T *) ((UINT8 *) p_buf - BUFFER_HDR_SIZE);
 
    return(p_hdr->status);
}

/*******************************************************************************
**
** Function         GKI_enqueue
**
** Description      Enqueue a buffer at the tail of the queue
**
** Parameters:      p_q  -  (input) pointer to a queue.
**                  p_buf - (input) address of the buffer to enqueue
**
** Returns          void
**
*******************************************************************************/
void GKI_enqueue(BUFFER_Q *p_q, void *p_buf)
{
    BUFFER_HDR_T *p_hdr;

#if (GKI_ENABLE_BUF_CORRUPTION_CHECK == TRUE)
    if (gki_chk_buf_damage(p_buf))
    {
        //printk("Enqueue - Buffer corrupted\n");
        GKI_exception(GKI_ERROR_BUF_CORRUPTED, "Enqueue - Buffer corrupted");
        return;
    }
#endif

    p_hdr = (BUFFER_HDR_T *) ((UINT8 *) p_buf - BUFFER_HDR_SIZE);

    if (p_hdr->status != BUF_STATUS_UNLINKED)
    {
        printk("Enqueue - buf already linked\n");
        GKI_exception(GKI_ERROR_ENQUEUE_BUF_LINKED, "Enqueue - buf already linked : p_hdr->status: %d, buffer: 0x%p",p_hdr->status, p_buf);
        return;
    }

    GKI_disable();

    /* Since the queue is exposed (C vs C++), keep the pointers in exposed format */
    /* check p_last != NULL before dereferencing it and if p_last == NULL then p_first == NULL */
    if (p_q->p_last)
    {
        BUFFER_HDR_T *p_last_hdr = (BUFFER_HDR_T *)((UINT8 *)p_q->p_last - BUFFER_HDR_SIZE);
        p_last_hdr->p_next = p_hdr;

        /* sanity check, this should not happen */
        if (p_q->p_first == NULL)
        {
            printk("ERROR: Enqueue - first == NULL , last != NULL (0x%p)\n", p_q->p_last);
        }
    }
    else
    {
        /* sanity check, this should not happen */
        if (p_q->p_first != NULL)
        {
            printk("ERROR: Enqueue - first != NULL (0x%p), last == NULL\n", p_q->p_first);
        }
        
        p_q->p_first = p_buf;
    }

    p_q->p_last = p_buf;
    p_q->count++;

    p_hdr->p_next = NULL;
    p_hdr->status = BUF_STATUS_QUEUED;

    GKI_enable();
    // printk("Enqueue: out from GKI_enqueue\n");

    return;
}


/*******************************************************************************
**
** Function         GKI_enqueue_head
**
** Description      Enqueue a buffer at the head of the queue
**
** Parameters:      p_q  -  (input) pointer to a queue.
**                  p_buf - (input) address of the buffer to enqueue
**
** Returns          void
**
*******************************************************************************/
void GKI_enqueue_head(BUFFER_Q *p_q, void *p_buf)
{
    BUFFER_HDR_T *p_hdr;

#if (GKI_ENABLE_BUF_CORRUPTION_CHECK == TRUE)
    if (gki_chk_buf_damage(p_buf))
    {
        GKI_exception(GKI_ERROR_BUF_CORRUPTED, "Enqueue - Buffer corrupted");
        return;
    }
#endif

    p_hdr = (BUFFER_HDR_T *) ((UINT8 *) p_buf - BUFFER_HDR_SIZE);

    if (p_hdr->status != BUF_STATUS_UNLINKED)
    {
        GKI_exception(GKI_ERROR_ENQUEUE_BUF_LINKED, "Enqeueue head - buf already linked");
        return;
    }

    GKI_disable();

    if (p_q->p_first)
    {
        p_hdr->p_next = (BUFFER_HDR_T *)((UINT8 *)p_q->p_first - BUFFER_HDR_SIZE);
        p_q->p_first = p_buf;
    }
    else
    {
        p_q->p_first = p_buf;
        p_q->p_last  = p_buf;
        p_hdr->p_next = NULL;
    }
    p_q->count++;

    p_hdr->status = BUF_STATUS_QUEUED;

    GKI_enable();

    return;
}


/*******************************************************************************
**
** Function         GKI_dequeue
**
** Description      Dequeues a buffer from the head of a queue
**
** Parameters:      p_q  - (input) pointer to a queue.
**
** Returns          NULL if queue is empty, else buffer
**
*******************************************************************************/
void *GKI_dequeue(BUFFER_Q *p_q)
{
    BUFFER_HDR_T *p_hdr;

    GKI_disable();

    if (!p_q || !p_q->count)
    {
        GKI_enable();
        return(NULL);
    }

    p_hdr = (BUFFER_HDR_T *)((UINT8 *)p_q->p_first - BUFFER_HDR_SIZE);

    /* Keep buffers such that GKI header is invisible */
    if (p_hdr->p_next)
        p_q->p_first = ((UINT8 *)p_hdr->p_next + BUFFER_HDR_SIZE);
    else
    {
        p_q->p_first = NULL;
        p_q->p_last  = NULL;
    }

    p_q->count--;

    p_hdr->p_next = NULL;
    p_hdr->status = BUF_STATUS_UNLINKED;

    GKI_enable();

    return((UINT8 *)p_hdr + BUFFER_HDR_SIZE);
}


/*******************************************************************************
**
** Function         GKI_remove_from_queue
**
** Description      Dequeue a buffer from the middle of the queue
**
** Parameters:      p_q  - (input) pointer to a queue.
**                  p_buf - (input) address of the buffer to enqueue
**
** Returns          NULL if queue is empty, else buffer
**
*******************************************************************************/
void *GKI_remove_from_queue(BUFFER_Q *p_q, void *p_buf)
{
    BUFFER_HDR_T *p_prev;
    BUFFER_HDR_T *p_buf_hdr;

    GKI_disable();

    if (p_buf == p_q->p_first)
    {
        GKI_enable();
        return(GKI_dequeue(p_q));
    }

    p_buf_hdr = (BUFFER_HDR_T *)((UINT8 *)p_buf - BUFFER_HDR_SIZE);
    p_prev    = (BUFFER_HDR_T *)((UINT8 *)p_q->p_first - BUFFER_HDR_SIZE);

    for ( ; p_prev; p_prev = p_prev->p_next)
    {
        /* If the previous points to this one, move the pointers around */
        if (p_prev->p_next == p_buf_hdr)
        {
            p_prev->p_next = p_buf_hdr->p_next;

            /* If we are removing the last guy in the queue, update p_last */
            if (p_buf == p_q->p_last)
                p_q->p_last = p_prev + 1;

            /* One less in the queue */
            p_q->count--;

            /* The buffer is now unlinked */
            p_buf_hdr->p_next = NULL;
            p_buf_hdr->status = BUF_STATUS_UNLINKED;

            GKI_enable();
            return(p_buf);
        }
    }

    GKI_enable();
    return(NULL);
}


/*******************************************************************************
**
** Function         GKI_getfirst
**
** Description      Return a pointer to the first buffer in a queue
**
** Parameters:      p_q  - (input) pointer to a queue.
**
** Returns          NULL if queue is empty, else buffer address
**
*******************************************************************************/
void *GKI_getfirst(BUFFER_Q *p_q)
{
    return(p_q->p_first);
}


/*******************************************************************************
**
** Function         GKI_getnext
**
** Description      Return a pointer to the next buffer in a queue
**
** Parameters:      p_buf  - (input) pointer to the buffer to find the next one from.
**
** Returns          NULL if no more buffers in the queue, else next buffer address
**
*******************************************************************************/
void *GKI_getnext(void *p_buf)
{
    BUFFER_HDR_T *p_hdr;

    p_hdr = (BUFFER_HDR_T *) ((UINT8 *) p_buf - BUFFER_HDR_SIZE);

    if (p_hdr->p_next)
        return((UINT8 *)p_hdr->p_next + BUFFER_HDR_SIZE);
    else
        return(NULL);
}



/*******************************************************************************
**
** Function         GKI_queue_is_empty
**
** Description      Check the status of a queue.
**
** Parameters:      p_q  - (input) pointer to a queue.
**
** Returns          TRUE if queue is empty, else FALSE
**
*******************************************************************************/
BOOLEAN GKI_queue_is_empty(BUFFER_Q *p_q)
{
    return((BOOLEAN)(p_q->count == 0));
}

/*******************************************************************************
**
** Function         GKI_find_buf_start
**
** Description      This function is called with an address inside a buffer,
**                  and returns the start address of the buffer.
**
**                  The buffer should be one allocated from one of GKI's pools.
**
** Parameters:      p_user_area - (input) address of anywhere in a GKI buffer.
**
** Returns          void * - Address of the beginning of the specified buffer if successful,
**                          otherwise NULL if unsuccessful
**
*******************************************************************************/
void *GKI_find_buf_start(void *p_user_area)
{
    int         pool_id, seg_inx;
    UINT16      size;
    UINT32      yy;
    UINT8      *p_ua = (UINT8 *)p_user_area;
    FREE_QUEUE_T *p_fq = gki_cb.freeq;

    for (pool_id = 0; pool_id < gki_cb.curr_total_no_of_pools; pool_id++, p_fq++)
    {
        for (seg_inx = 0; (seg_inx < MAX_BUFFPOOL_SEGS) && (p_fq->seg_start[seg_inx] != NULL); seg_inx++)
        {
            if ((p_ua > p_fq->seg_start[seg_inx]) && (p_ua < p_fq->seg_end[seg_inx]))
            {
                yy = (UINT32)(p_ua - p_fq->seg_start[seg_inx]);

                size = gki_cb.pool_buf_size[pool_id] + BUFFER_PADDING_SIZE;

                yy = (yy / size) * size;

                return((void *) (p_fq->seg_start[seg_inx] + yy + sizeof(BUFFER_HDR_T)) );
            }
        }
    }

    /* If here, invalid address - not in one of our buffers */
    GKI_exception(GKI_ERROR_BUF_SIZE_ZERO, "GKI_get_buf_start:: bad addr");

    return(NULL);
}


/*******************************************************************************
**
** Function         GKI_create_pool
**
** Description      Called by applications to create a buffer pool.
**
** Parameters:      size        - (input) length (in bytes) of each buffer in the pool
**                  count       - (input) number of buffers to allocate for the pool
**                  permission  - (input) restricted or public access?
**                                        (GKI_PUBLIC_POOL or GKI_RESTRICTED_POOL)
**                  p_mem_pool  - (input) pointer to an OS memory pool, NULL if not provided
**
** Returns          the buffer pool ID, which should be used in calls to
**                  GKI_getpoolbuf(). If a pool could not be created, this
**                  function returns 0xff.
**
*******************************************************************************/
UINT8 GKI_create_pool(UINT16 size, UINT16 count, UINT8 permission, void *p_mem_pool)
{
    int xx;

    /* First make sure the size of each pool has a valid size with room for the header info */
    if (size > MAX_USER_BUF_SIZE)
        return(GKI_INVALID_POOL);

    /* First, look for an unused pool */
    for (xx = 0; xx < GKI_NUM_TOTAL_BUF_POOLS; xx++)
    {
        if (!gki_cb.pool_buf_size[xx])
            break;
    }

    if (xx == GKI_NUM_TOTAL_BUF_POOLS)
        return(GKI_INVALID_POOL);

    GKI_disable();

    /* Ensure an even number of longwords */
    size = ((size + 3) / 4) * 4;

    gki_cb.pool_buf_size[xx]  = size;
    gki_cb.pool_max_count[xx] = count;
    gki_cb.pool_additions[xx] = 0;

    /* If memory was not passed in, create the pool in segments */
    if (!p_mem_pool)
        count = (count + MAX_BUFFPOOL_SEGS - 1) / MAX_BUFFPOOL_SEGS;

    /* Initialize the new pool */
    if (gki_init_free_queue(xx, size, count, p_mem_pool))
    {
        gki_add_to_pool_list(xx);

        (void) GKI_set_pool_permission(xx, permission);
        gki_cb.curr_total_no_of_pools++;
    }
    else
    {
        /* Failed to create the pool ? */
        gki_cb.pool_buf_size[xx] = 0;
        GKI_enable();
        return(GKI_INVALID_POOL);
    }

    /* If memory was passed in, no pool additions allowed */
    if (p_mem_pool)
        gki_cb.pool_additions[xx] = MAX_BUFFPOOL_SEGS;

    GKI_enable();
    return(xx);
}


/*******************************************************************************
**
** Function         GKI_delete_pool
**
** Description      Called by applications to delete a buffer pool.  The function
**                  calls the operating specific function to free the actual memory.
**                  An exception is generated if an error is detected.
**
** Parameters:      pool_id - (input) Id of the poll being deleted.
**
** Returns          void
**
*******************************************************************************/
void GKI_delete_pool(UINT8 pool_id)
{
    FREE_QUEUE_T    *Q;
    int             xx;

    if ( (pool_id >= GKI_NUM_TOTAL_BUF_POOLS) || (!gki_cb.pool_buf_size[pool_id]) )
        return;

    GKI_disable();

    Q = &gki_cb.freeq[pool_id];

    Q->total     = 0;
    Q->cur_cnt   = 0;
    Q->max_cnt   = 0;
    Q->p_first   = NULL;
    Q->p_last    = NULL;

    for (xx = 0; xx < MAX_BUFFPOOL_SEGS; xx++)
    {
        if (Q->seg_start[xx])
            gki_release_os_memory(Q->seg_start[xx]);

        Q->seg_start[xx] = NULL;
        Q->seg_end[xx]   = NULL;
    }

    gki_cb.pool_buf_size[pool_id]  = 0;

    gki_remove_from_pool_list(pool_id);
    gki_cb.curr_total_no_of_pools--;

    GKI_enable();
    return;
}

/*******************************************************************************
**
** Function         GKI_chk_buf_pool_damage
**
** Description      Called internally by OSS to check for buffer queue corruption.
**
** Returns          TRUE if there is a problem, else FALSE
**
*******************************************************************************/
BOOLEAN GKI_chk_buf_pool_damage(UINT8 pool_id)
{
#if (GKI_ENABLE_BUF_CORRUPTION_CHECK == TRUE)
    int i;
    FREE_QUEUE_T  *Q;
    BUFFER_HDR_T  *p_hdr;
    UINT8         *p_buf;

    if (pool_id >= GKI_NUM_TOTAL_BUF_POOLS)
    {
        GKI_exception(GKI_ERROR_BUF_POOL_CORRUPT, "pool_id out of range");
        return(TRUE);
    }

    /* Make sure the buffers aren't disturbed til finished with checking */
    GKI_disable();

    Q = &gki_cb.freeq[pool_id];
    if (!Q->p_first)
    {
        if (Q->cur_cnt != Q->total)
        {
            GKI_enable();
            GKI_exception(GKI_ERROR_BUF_POOL_CORRUPT, "p_first is NULL in non-empty pool");
            return(TRUE);
        }
        return(FALSE);
    }
    p_hdr = Q->p_first;
    i = 1;
    while (p_hdr->p_next)
    {
        p_hdr = p_hdr->p_next;
        p_buf = ((UINT8 *)p_hdr + BUFFER_HDR_SIZE);
        if (gki_chk_buf_damage(p_buf))
        {
            GKI_enable();
            GKI_exception(GKI_ERROR_BUF_CORRUPTED, "CHk Pool - Buf Corrupted");
            return(TRUE);
        }
        i++;
    }
    if (p_hdr != Q->p_last)
    {
        GKI_enable();
        GKI_exception(GKI_ERROR_BUF_POOL_CORRUPT, "last buffer in chain != p_last");
        return(TRUE);
    }
    if (i != (Q->total - Q->cur_cnt))
    {
        GKI_enable();
        GKI_exception(GKI_ERROR_BUF_POOL_CORRUPT, "cur_cnt != number of buffers in pool");
        return(TRUE);
    }

    GKI_enable();
    return(FALSE);
    
#else

    return(FALSE);

#endif
}

