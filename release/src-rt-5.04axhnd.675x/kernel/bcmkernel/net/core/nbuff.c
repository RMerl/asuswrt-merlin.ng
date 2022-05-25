/*
 * <:copyright-BRCM:2009:GPL/GPL:standard
 * 
 *    Copyright (c) 2009 Broadcom 
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

#define FKB_IMPLEMENTATION_FILE

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/nbuff.h>
#include <linux/export.h>
#if IS_ENABLED(CONFIG_BCM_BPM)
#include <linux/gbpm.h>
#endif

#ifdef CC_CONFIG_FKB_COLOR
#define COLOR(clr_code)     clr_code
#else
#define COLOR(clr_code)
#endif
#define CLRb                COLOR("\e[0;34m")       /* blue */
#define CLRc                COLOR("\e[0;36m")       /* cyan */
#define CLRn                COLOR("\e[0m")          /* normal */
#define CLRerr              COLOR("\e[0;33;41m")    /* yellow on red */
#define CLRN                CLRn"\n"                /* normal newline */

int nbuff_dbg = 0;
#if defined(CC_CONFIG_FKB_DEBUG)
#define fkb_print(fmt, arg...)                                          \
    printk( CLRc "FKB %s :" fmt CLRN, __FUNCTION__, ##arg )
#define fkb_assertv(cond)                                               \
    if ( !cond ) {                                                      \
        printk( CLRerr "FKB ASSERT %s : " #cond CLRN, __FUNCTION__ );   \
        return;                                                         \
    }
#define fkb_assertr(cond, rtn)                                          \
    if ( !cond ) {                                                      \
        printk( CLRerr "FKB ASSERT %s : " #cond CLRN, __FUNCTION__ );   \
        return rtn;                                                     \
    }
#else
#define fkb_print(fmt, arg...)  NULL_STMT
#define fkb_assertv(cond)       NULL_STMT
#define fkb_assertr(cond, rtn)  NULL_STMT
#endif

/*
 *------------------------------------------------------------------------------
 * Test whether an FKB may be translated onto a skb.
 *------------------------------------------------------------------------------
 */
int fkb_in_skb_test(int fkb_offset, int word0_offset, int blog_p_offset,
		    int data_offset, int len_word_offset, int queue_offset,
		    int priority_offset, int recycle_hook_offset,
		    int recycle_context_offset)
{
#undef OFFSETOF
#define OFFSETOF(stype, member)	((size_t)&((struct stype *)0)->member)       
#define FKBOFFSETOF(member)	(member##_offset)
#define SKBOFFSETOF(member)	(((size_t)&((struct sk_buff *)0)->member)-fkb_offset)
#define FKBSIZEOF(member)	(sizeof(((struct fkbuff *)0)->member))
#define SKBSIZEOF(member)	(sizeof(((struct sk_buff *)0)->member))
#define FKBINSKB_TEST(member)	((FKBOFFSETOF(member) == SKBOFFSETOF(member)) \
		 		  && (FKBSIZEOF(member) == SKBSIZEOF(member)))

	if (OFFSETOF(sk_buff, fkbInSkb) != fkb_offset)
		return 0;

	if (!FKBINSKB_TEST(word0))
		return 0;

	if (!FKBINSKB_TEST(blog_p))
		return 0;

	if (!FKBINSKB_TEST(data))
		return 0;

	if (!FKBINSKB_TEST(len_word))
		return 0;

	if (!FKBINSKB_TEST(queue))
		return 0;

	if (!FKBINSKB_TEST(priority))
		return 0;

	if (!FKBINSKB_TEST(recycle_hook))
		return 0;

	if (!FKBINSKB_TEST(recycle_context))
		return 0;

	/* to ensure that fkbuff is not larger than 2 cache lines */
	if (sizeof(struct fkbuff) > (2 * cache_line_size()))
		return 0;

	/* this is to ensure fkb is not placed across different cache line
	 * in 32 bit architecture (ARM and MIPS), we have 16, 32, and 64
	 * byte cache line sizes.  In 64 bit architecture, we have 64 byte
	 * cache line sizes. The following will ensure fkb_offset is at
	 * 32 byte aligned for 32 bit system, and 64 byte aligned for
	 * 64 bit system. */
	if ((fkb_offset & (sizeof(unsigned long) * 8 - 1)) != 0)
		return 0;

	return 1;
}

/*
 *------------------------------------------------------------------------------
 * Object detailing info related to both Master and cloned type FkbObjs.
 * If CONFIG_BCM_NBUFF_FKB_POOL is enabled then it will maintain a
 * pre-allocated Pool of Cloned and Master FkBuff_t objects.
 *------------------------------------------------------------------------------ */

typedef struct fkbObjInfo {
#if defined(CONFIG_BCM_NBUFF_FKB_POOL)
    FkBuff_t  * freelist_p;         /* List of free objects                   */
    uint32_t    extends;            /* Number of pool extensions performed    */

    /* Pool dimensioning parameters */
    uint32_t    pool_size;          /* Startup default pool size              */
    uint32_t    extend_size;        /* Number of objects per extension        */
    uint32_t    extend_max;         /* Maximum number of extensions permitted */
#endif   //CONFIG_BCM_NBUFF_FKB_POOL
    uint32_t    object_size;        /* Size of each object in the pool        */
    char        name[8];

#if defined(CC_CONFIG_FKB_STATS)
    int         cnt_free;           /* Number of free objects in pool / total freed so far    */
    int         cnt_used;           /* Number of in use objects / total used so far           */
    int         cnt_failed;         /* Failure due to out of memory                           */
#if defined(CONFIG_BCM_NBUFF_FKB_POOL)
    int         cnt_hwm;            /* In use high water mark for engineering                 */
#endif   //CONFIG_BCM_NBUFF_FKB_POOL
#endif
} FkbObjInfo_t;

/*
 *------------------------------------------------------------------------------
 * Global fkbObjInfo for Cloned and Master FKB Objects. 
 *------------------------------------------------------------------------------
 */
FkbObjInfo_t fkb_obj_g[ FkbMaxType_e ] = {
    {
#if defined(CONFIG_BCM_NBUFF_FKB_POOL)
        .freelist_p     = FKB_NULL,
        .extends        = 0,
        .pool_size      = FKBM_POOL_SIZE_ENGG,
        .extend_size    = FKBM_EXTEND_SIZE_ENGG,
        .extend_max     = FKBM_EXTEND_MAX_ENGG,
#endif
#if !((defined(CONFIG_BCM94912) || defined(CONFIG_BCM96855)) && defined(CONFIG_BCM_JUMBO_FRAME))
        .object_size    = BCM_PKTBUF_SIZE,     /* Rx Buffer wth in place FKB */
#endif
        .name           = "Master",
#if defined(CC_CONFIG_FKB_STATS)
        .cnt_free = 0, .cnt_used = 0, .cnt_failed = 0,
#if defined(CONFIG_BCM_NBUFF_FKB_POOL)
        .cnt_hwm = 0, 
#endif
#endif
    }
    ,
    {
#if defined(CONFIG_BCM_NBUFF_FKB_POOL)
        .freelist_p     = FKB_NULL,
        .extends        = 0,
        .pool_size      = FKBC_POOL_SIZE_ENGG,
        .extend_size    = FKBC_EXTEND_SIZE_ENGG,
        .extend_max     = FKBC_EXTEND_MAX_ENGG,
#endif
        .object_size    = sizeof(FkBuff_t),     /* Only FKB object */
        .name           = "Cloned",
#if defined(CC_CONFIG_FKB_STATS)
        .cnt_free = 0, .cnt_used = 0, .cnt_failed = 0,
#if defined(CONFIG_BCM_NBUFF_FKB_POOL)
        .cnt_hwm = 0,
#endif
#endif
    }
};

/*
 *------------------------------------------------------------------------------
 * Statistics collection for engineering free pool parameters.
 *------------------------------------------------------------------------------
 */
void fkb_stats(void)
{
    int pool;
    FkbObjInfo_t *objinfo_p;
    for (pool = 0; pool < FkbMaxType_e; pool++ )
    {
        objinfo_p = &fkb_obj_g[pool];
#if defined(CONFIG_BCM_NBUFF_FKB_POOL)
        printk("FKB %s Pool: extends<%u>\n", objinfo_p->name, objinfo_p->extends );

        FKB_STATS(
            printk("\t free<%d> used<%d> HWM<%d> failed<%d>\n",
                   objinfo_p->cnt_free,
                   objinfo_p->cnt_used, objinfo_p->cnt_hwm, objinfo_p->cnt_failed ); );
#else
        printk("FKB %s \n", objinfo_p->name );

        FKB_STATS(
            printk("\t free<%d> used<%d> failed<%d>\n",
                   objinfo_p->cnt_free,
                   objinfo_p->cnt_used, objinfo_p->cnt_failed ); );
#endif
    }
}

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#include "linux/spinlock.h"
static DEFINE_SPINLOCK(fkb_pool_lock_g);   /* FkBuff packet flow */
#define FKB_POOL_LOCK()     spin_lock_irqsave(&fkb_pool_lock_g, lock_flags)
#define FKB_POOL_UNLOCK()   spin_unlock_irqrestore(&fkb_pool_lock_g, lock_flags)
#else
#define FKB_POOL_LOCK()     local_irq_disable()
#define FKB_POOL_UNLOCK()   local_irq_enable()
#endif

#if defined(CC_CONFIG_FKB_AUDIT)
void fkb_audit(const char * function, int line)
{ /* place any audits here */ }
EXPORT_SYMBOL(fkb_audit);
#define FKB_AUDIT_RUN()     fkb_audit(__FUNCTION__,__LINE__)  
#else
#define FKB_AUDIT_RUN()     NULL_STMT
#endif

#if defined(CONFIG_BCM_NBUFF_FKB_POOL)
static uint32_t fkb_extend(uint32_t num, FkbObject_t object);

static void release_fkb(FkBuff_t *fkb_p, FkbObject_t object, FkbObjInfo_t *objinfo_p)
{
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
    unsigned long lock_flags;
#endif

    FKB_POOL_LOCK();

    FKB_STATS( objinfo_p->cnt_used--; objinfo_p->cnt_free++; );

    fkb_p->list = objinfo_p->freelist_p;  /* resets master_p */
    objinfo_p->freelist_p = fkb_p;        /* link into Cloned free pool */

    FKB_POOL_UNLOCK();
}

static FkBuff_t *allocate_fkb(FkbObject_t object, FkbObjInfo_t *objinfo_p)
{
    register FkBuff_t *fkb_p;
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
    unsigned long lock_flags;
#endif

    FKB_POOL_LOCK();    /* DO NOT USE fkb_assertr() until FKB_POOL_UNLOCK() */

    if ( unlikely(objinfo_p->freelist_p == FKB_NULL) )
    {
#ifdef SUPPORT_FKB_EXTEND
        /* Try extending free pool */
        if ( (objinfo_p->extends >= objinfo_p->extend_max)
          || (fkb_extend( objinfo_p->extend_size, object ) != objinfo_p->extend_size))
        {
            fkb_print( "WARNING: FKB Pool %s exhausted", objinfo_p->name );
        }
#else
        if ( fkb_extend( objinfo_p->extend_size, object ) == 0 )
        {
            fkb_print( "WARNING: FKB Pool %s out of memory", objinfo_p->name );
        }
#endif
        if (objinfo_p->freelist_p == FKB_NULL)
        {
            fkb_p = FKB_NULL;
            goto fkb_alloc_return;
        }
    }

    FKB_STATS(
        objinfo_p->cnt_free--;
        if ( ++objinfo_p->cnt_used > objinfo_p->cnt_hwm )
            objinfo_p->cnt_hwm = objinfo_p->cnt_used;
        );

    /* Delete an FkBuff from the pool */
    fkb_p = objinfo_p->freelist_p;
    objinfo_p->freelist_p = objinfo_p->freelist_p->list;

    // fkb_set_ref(fkb_p, 0);
    fkb_p->list = FKB_NULL;   /* resets list, master_p to NULL , users to 0 */
fkb_alloc_return:

    FKB_POOL_UNLOCK();  /* May use kb_assertr() now onwards */
    return fkb_p;
}
#else
void fkbM_recycle(pNBuff_t pNBuff, unsigned long context, unsigned flags);

static void release_fkb(FkBuff_t *fkb_p, FkbObject_t object, FkbObjInfo_t *objinfo_p)
{
#if (defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)) && defined(CC_CONFIG_FKB_STATS)
    unsigned long lock_flags;
#endif

    if ( object == FkbMaster_e )
    {
#if IS_ENABLED(CONFIG_BCM_BPM)
        gbpm_free_buf(PFKBUFF_TO_PDATA(fkb_p,BCM_PKT_HEADROOM));
#else
        kfree(fkb_p);
#endif
    }
    else if (object == FkbCloned_e)
    {
        kfree(fkb_p);
    }
    else
    {
        printk("Invalid object type\n");
    }

#if defined(CC_CONFIG_FKB_STATS)
    FKB_POOL_LOCK();

    FKB_STATS( objinfo_p->cnt_free++; );

    FKB_POOL_UNLOCK();
#endif
}

static FkBuff_t *allocate_fkb(FkbObject_t object, FkbObjInfo_t *objinfo_p)
{
    register FkBuff_t *fkb_p;
#if (defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)) && defined(CC_CONFIG_FKB_STATS)
    unsigned long lock_flags;
#endif

    if ( object == FkbMaster_e )
    {
#if IS_ENABLED(CONFIG_BCM_BPM)
        fkb_p = gbpm_alloc_buf();
        if ( fkb_p != NULL )
            fkb_p = (FkBuff_t *)PDATA_TO_PFKBUFF(fkb_p, BCM_PKT_HEADROOM);
        else
            printk("failed to allocate buffer from BPM\n");
#else
        fkb_p = kmalloc(objinfo_p->object_size, GFP_ATOMIC);
#endif
        if ( fkb_p != NULL )
        {
            fkb_p->recycle_hook = (RecycleFuncP)fkbM_recycle;
            fkb_p->recycle_context = (unsigned long)objinfo_p;
            fkb_p->list = FKB_NULL;
        }
        else
            printk("Failed to allocate Master FKB\n");
    }
    else if (object == FkbCloned_e)
    {
        fkb_p = (FkBuff_t *)kmalloc(objinfo_p->object_size,GFP_ATOMIC);
        if ( fkb_p != NULL )
        {
            fkb_p->recycle_hook = (RecycleFuncP)NULL;
            fkb_p->recycle_context = (unsigned long)objinfo_p;
            fkb_p->list = FKB_NULL;
        }
        else
            printk("Failed to allocate clone FKB\n");
    }
    else
    {
        printk("Invalid object type\n");
    }

#if defined(CC_CONFIG_FKB_STATS)
    FKB_POOL_LOCK();    /* DO NOT USE fkb_assertr() until FKB_POOL_UNLOCK() */

    FKB_STATS(
        if ( fkb_p != NULL )  
            objinfo_p->cnt_used++;
        else
            objinfo_p->cnt_failed++;
        );

    FKB_POOL_UNLOCK();  /* May use kb_assertr() now onwards */
#endif
    return fkb_p;
}
#endif
/*
 *------------------------------------------------------------------------------
 * Function   : fkbM_recycle
 * Description: Recycling a Master FKB that was allocated from Master FKB Pool.
 * Parameters :
 *   pNBuff   : pointer to a network buffer
 *   context  : registered context argument with network buffer.
 *   flags    : unused by fkb recycling.
 *------------------------------------------------------------------------------
 */
void fkbM_recycle(pNBuff_t pNBuff, unsigned long context, unsigned flags)
{
    register FkBuff_t  * fkbM_p;
    register FkbObjInfo_t * objinfo_p = (FkbObjInfo_t *)((unsigned long)context);

    fkb_assertv( (objinfo_p == &fkb_obj_g[FkbMaster_e]) ); 

    if ( IS_SKBUFF_PTR(pNBuff) )
    {
        struct sk_buff * skb_p = (struct sk_buff *)PNBUFF_2_SKBUFF(pNBuff);
        fkb_assertv( (flags & SKB_DATA_RECYCLE) );
        fkbM_p = (FkBuff_t *)((uintptr_t)(skb_p->head) - PFKBUFF_PHEAD_OFFSET); 
    }
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
    else
        fkbM_p = PNBUFF_2_FKBUFF(pNBuff);

    fkb_dbg(1, "fkbM_p<%px>", fkbM_p);

    FKB_AUDIT(
        if ( fkbM_p->list != NULL )
            printk("FKB ASSERT cpu<%u> %s(%px) list<%px> recycle<%pS>\n",
                   smp_processor_id(), __FUNCTION__,
                   fkbM_p, fkbM_p->list, fkbM_p->recycle_hook);
        if ( fkbM_p->recycle_hook != (RecycleFuncP)fkbM_recycle )
            printk("FKB ASSERT cpu<%u> %s <%px>.recycle<%pS>\n",
                   smp_processor_id(), __FUNCTION__,
                   fkbM_p, fkbM_p->recycle_hook); );
            
    FKB_AUDIT_RUN();

    release_fkb(fkbM_p, FkbMaster_e, objinfo_p);

    FKB_AUDIT_RUN();
}

/*
 *------------------------------------------------------------------------------
 * Function   : fkbC_recycle
 * Description: Recycling a Cloned FKB back to the Cloned FKB Pool or kernel.
 * Parameters :
 *   fkbC_p   : Pointer to a Cloned FKB Object.
 *------------------------------------------------------------------------------
 */
void fkbC_recycle(FkBuff_t * fkbC_p)
{
    register FkbObjInfo_t * objinfo_p;

    objinfo_p = &fkb_obj_g[ FkbCloned_e ];

    fkb_dbg(2, "fkb_p<%px>", fkbC_p);

    FKB_AUDIT(
        if ( fkbC_p->recycle_hook != (RecycleFuncP)NULL )
            printk("FKB ASSERT cpu<%u> %s <0x%08x>.recycle<%pS>\n",
                   smp_processor_id(), __FUNCTION__,
                   (int)fkbC_p, fkbC_p->recycle_hook); );

    FKB_AUDIT_RUN();

    release_fkb(fkbC_p, FkbCloned_e, objinfo_p);

    FKB_AUDIT_RUN();
}

#if defined(CONFIG_BCM_NBUFF_FKB_POOL)
/*
 *------------------------------------------------------------------------------
 * Function   : fkb_extend
 * Description: Create a pool of FKB objects. When a pool is exhausted
 *              this function may be invoked to extend the pool.
 *              All objects in the pool are chained in a single linked list.
 * Parameters :
 *   num      : Number of FKB objects to be allocated.
 *   object   : Object type to locate pool
 * Returns    : Number of FKB objects allocated.
 *------------------------------------------------------------------------------
 */
static uint32_t fkb_extend(uint32_t num, FkbObject_t object)
{
    register int i;
    register FkBuff_t  * list_p, * fkb_p, * fkbNext_p;
    register FkbObjInfo_t *objinfo_p;

    fkb_assertr( (object < FkbMaxType_e), 0 );

    objinfo_p = &fkb_obj_g[object];

    list_p = (FkBuff_t *) kmalloc( num * objinfo_p->object_size, GFP_ATOMIC);
 
    fkb_print( "fkb_extend %u FKB %s objects <%px> .. <%px>",
               num, objinfo_p->name, list_p,
               (FkBuff_t*)((uintptr_t)list_p + ((num-1) * objinfo_p->object_size)));
 
    if ( unlikely(list_p == FKB_NULL) )   /* may fail if in_interrupt or oom */
    {
        FKB_STATS( objinfo_p->cnt_failed++; );
        fkb_print( "WARNING: Failure to initialize %d FKB %s objects",
                    num, objinfo_p->name );
        return 0;
    }
    objinfo_p->extends++;
 
    /* memset( (void *)list, 0, ( num * objinfo_p->object_size ) ); */
 
    /* Link all allocated objects together */
    fkb_p = FKB_NULL;
    fkbNext_p = list_p;
    for ( i = 0; i < num; i++ )
    {
        fkb_p = fkbNext_p;
        fkbNext_p = (FkBuff_t *)( (uintptr_t)fkb_p + objinfo_p->object_size );
 
        if ( object == FkbCloned_e )
        {
            fkb_p->recycle_hook = (RecycleFuncP)NULL;
            fkb_p->recycle_context = (unsigned long)&fkb_obj_g[FkbCloned_e];
        }
        else
        {
            // fkb_set_ref(fkb_p, 0);   ... see fkb_p->list
            fkb_p->recycle_hook = (RecycleFuncP)fkbM_recycle;
            fkb_p->recycle_context = (unsigned long)&fkb_obj_g[FkbMaster_e];
        }
 
        fkb_p->list = fkbNext_p;        /* link each FkBuff */
    }
 
    FKB_STATS( objinfo_p->cnt_free += num; );
 
    /* link allocated list into FKB free pool */
    fkb_p->list = objinfo_p->freelist_p;  /* chain last FKB object */
    objinfo_p->freelist_p = list_p;       /* head of allocated list */
    return num;
}
#endif

/*
 *------------------------------------------------------------------------------
 * Function     : fkb_construct
 * Description  : Incarnates the FKB system pools during kernel/module init
 *------------------------------------------------------------------------------
 */
int fkb_construct(int fkb_in_skb_offset)
{
#if (defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)) && defined(CONFIG_BCM_NBUFF_FKB_POOL)
    unsigned long lock_flags;
#endif
#undef FKBOFFSETOF
#define FKBOFFSETOF(member)   ((size_t)&((struct fkbuff*)0)->member)
    if ( fkb_in_skb_test(fkb_in_skb_offset,
                FKBOFFSETOF(word0), FKBOFFSETOF(blog_p), FKBOFFSETOF(data),
                FKBOFFSETOF(len_word), FKBOFFSETOF(mark), FKBOFFSETOF(priority),
                FKBOFFSETOF(recycle_hook), FKBOFFSETOF(recycle_context)) == 0 )
        return -1;
    else
        FKB_DBG( printk(CLRb "FKB compatible with SKB" CLRN); );

#if defined(CONFIG_BCM_NBUFF_FKB_POOL)
    FKB_POOL_LOCK();

    /* Prepare a free pool for Cloned FkBuffs */
    fkb_extend( fkb_obj_g[FkbCloned_e].pool_size, FkbCloned_e );

    /* Prepare a free pool for Master FkBuffs */
    fkb_extend( fkb_obj_g[FkbMaster_e].pool_size, FkbMaster_e );

    FKB_POOL_UNLOCK();

    FKB_AUDIT_RUN();

    FKB_DBG( printk(CLRb "NBUFF nbuff_dbg<%px> = %d\n"
                         "\t Pool FkBuff %s size<%u> num<%u>\n"
                         "\t Pool FkBuff %s size<%u> num<%u>" CLRN,
                         &nbuff_dbg, nbuff_dbg,
                         fkb_obj_g[FkbCloned_e].name,
                         fkb_obj_g[FkbCloned_e].object_size,
                         fkb_obj_g[FkbCloned_e].pool_size,
                         fkb_obj_g[FkbMaster_e].name,
                         fkb_obj_g[FkbMaster_e].object_size,
                         fkb_obj_g[FkbMaster_e].pool_size );
           );
#endif
    printk( CLRb "NBUFF %s Initialized" CLRN, NBUFF_VERSION );

    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function     : fkb_alloc
 * Description  : Allocate an FKB from one of the pools
 *  object      : Type of FkbObject, to identify Free Pool
 * Returns      : Pointer to an FKB, or NULL on pool depletion.
 *------------------------------------------------------------------------------
 */
FkBuff_t * fkb_alloc( FkbObject_t object )
{
    FkBuff_t  * fkb_p;
    FkbObjInfo_t * objinfo_p;

    fkb_assertr( (object < FkbMaxType_e), FKB_NULL );

    FKB_AUDIT(
        if ( smp_processor_id() )
            printk("FKB ASSERT %s not supported on CP 1\n", __FUNCTION__); );

    FKB_AUDIT_RUN();

    objinfo_p = &fkb_obj_g[object];

    fkb_dbg(2, "%s", objinfo_p->name);
    
    fkb_p = allocate_fkb(object, objinfo_p);

    FKB_AUDIT_RUN();

    fkb_dbg(1, "fkb_p<%px>", fkb_p);

    return fkb_p;
}
/*
 *------------------------------------------------------------------------------
 * Function     : fkb_free
 * Description  : Free an FKB and associated buffer if reference count of the
 *                buffer is 0. 
 *                All cloned fkb's are freed to the global Cloned free pool or
 *                kernel depending on CONFIG_BCM_NBUFF_FKB_POOL
 *                Master FKBs will be recycled into the appropriate network
 *                device driver's rx pool or the MasterFkb recycle handler.
 * Parameters   :
 *   fkb_p      : Pointer to a FKB to be freed.
 *------------------------------------------------------------------------------
 */
void fkb_free(FkBuff_t * fkb_p)
{
    register FkBuff_t  * fkbM_p;

    FKB_AUDIT_RUN();

    fkb_assertv( (fkb_p!=FKB_NULL) );
    fkb_dbg(1, "fkb_p<%px>", fkb_p);

    /* FKB should never point to a Blog, so no need to free fkb_p->blog_p */
    fkb_assertv( (fkb_p!=FKB_NULL) );

    /* Implementation Note: list_p, master_p and users union.
       If it is a cloned fkb, then fkb_p->master_p is a KPTR. If a double free
       is invoked on the same fkb_p, then list_p will also be a KPTR! */

    if ( _is_fkb_cloned_pool_(fkb_p) )
    {
        fkbM_p = fkb_p->master_p;
        fkbC_recycle(fkb_p);
    }
    else
        fkbM_p = fkb_p;

    fkb_assertv( (_get_master_users_(fkbM_p) > 0) );

    /* API atomic_dec_and_test: After decrement, return true if result is 0 */
    if ( likely(atomic_long_dec_and_test(&fkbM_p->users)) )
    {
        /* No fkbs are referring to master, so master and buffer recyclable */
        fkbM_p->recycle_hook(FKBUFF_2_PNBUFF(fkbM_p),
                             fkbM_p->recycle_context, 0);
    }

    FKB_AUDIT_RUN();

    fkb_dbg(2, "fkbM_p<%px>", fkbM_p);
}
/*
 *------------------------------------------------------------------------------
 * Function   : fkb_unshare
 * Description: Returns a pointer to a Master FKB with a single reference to the
 *              packet. A Cloned FKB with a single reference will result in the
 *              Clone's Master FKB being returned, and the Cloned FKB Object is
 *              recycled.
 *------------------------------------------------------------------------------
 */
FkBuff_t * fkb_unshare(FkBuff_t * fkb_p)
{
    register FkBuff_t * fkbM_p;
    uint8_t * dirty_p;

    FKB_AUDIT(
        if ( smp_processor_id() )
            printk("FKB ASSERT %s not supported on CP 1\n", __FUNCTION__); );

    FKB_AUDIT_RUN();

    if ( unlikely(_is_fkb_cloned_pool_(fkb_p)) )     /* Cloned FKB */
    {
        /* If master is also referenced by other FkBuffs */
        if ( _get_master_users_(fkb_p->master_p) > 1 )
        {
            /* Real unsharing, by allocating new master FkBuff */
            fkbM_p = fkb_alloc( FkbMaster_e );
            if (fkbM_p == FKB_NULL)
            {
                fkb_dbg(1, "fkb_unshare Cloned FKB fkb_alloc failure");
                return FKB_NULL;
            }
            fkb_set_ref(fkbM_p, 1);

            /* Setup FkBuff context */
            fkbM_p->data = (uint8_t*)(fkbM_p)
                         + ((uintptr_t)fkb_p->data - (uintptr_t)fkb_p->master_p);
            fkbM_p->len_word = fkb_p->len_word;
            fkbM_p->mark = fkb_p->mark;
            fkbM_p->priority = fkb_p->priority;

            fkbM_p->dirty_p = _to_dptr_from_kptr_(fkbM_p->data + fkbM_p->len);

            /* Copy from original clone FkBuff */
            memcpy(fkbM_p->data, fkb_p->data, fkb_p->len);

            dirty_p = _to_dptr_from_kptr_(fkb_p->data  + fkb_p->len);
            if ( fkb_p->master_p->dirty_p < dirty_p )
                fkb_p->master_p->dirty_p = dirty_p;

            fkb_dec_ref(fkb_p->master_p); /* decrement masters user count */
            fkb_dbg(1, "cloned fkb_p with multiple ref master");
        }
        else
        {
            fkb_dbg(1, "cloned fkb_p with single ref master");
            fkbM_p = fkb_p->master_p;

            // Move clone context to master and return master
            fkbM_p->data = fkb_p->data;
            fkbM_p->len_word = fkb_p->len_word;
            fkbM_p->mark = fkb_p->mark;
            fkbM_p->priority = fkb_p->priority;

            if ( fkbM_p->dirty_p < fkb_p->dirty_p )
                fkbM_p->dirty_p = fkb_p->dirty_p;
        }

        fkb_dbg(2, "fkbM_p<%px> fkbM_data<%px> dirty_p<%px> len<%d>",
            fkbM_p, fkbM_p->data, fkbM_p->dirty_p, fkbM_p->len);
        fkb_dbg(2, "fkb_p<%px> fkb_data<%px> dirty_p<%px> len<%d>",
            fkb_p, fkb_p->data, fkb_p->dirty_p, fkb_p->len);
        fkbC_recycle(fkb_p);    /* always recycle original clone fkb */
        return fkbM_p;  /* return new allocate master FkBuff */
    }
    else    /* Original is a Master */
    {
        /* Single reference, no need to unshare */
        if ( _get_master_users_(fkb_p) == 1 )
        {
            fkb_dbg(1, "master fkb_p with single ref ");
            fkb_dbg(2, "fkb_p<%px> fkb_data<%px> dirty_p<%px> len<%d>",
                fkb_p, fkb_p->data, fkb_p->dirty_p, fkb_p->len);
            return fkb_p;
        }

        /* Allocate a master FkBuff with associated data buffer */
        fkbM_p = fkb_alloc( FkbMaster_e );
        if (fkbM_p == FKB_NULL)
        {
            fkb_dbg(1, "fkb_unshare Master Fkb fkb_alloc failure");
            return FKB_NULL;
        }
        fkb_set_ref(fkbM_p, 1);

        /* Setup FkBuff context */
        fkbM_p->data = (uint8_t*)(fkbM_p)
                       + ((uintptr_t)fkb_p->data - (uintptr_t)fkb_p);
        fkbM_p->len_word = fkb_p->len_word;
        fkbM_p->mark = fkb_p->mark;
        fkbM_p->priority = fkb_p->priority;

        fkbM_p->dirty_p = _to_dptr_from_kptr_(fkbM_p->data + fkbM_p->len);

        /* Copy original FkBuff's data into new allocated master FkBuff */
        memcpy(fkbM_p->data, fkb_p->data, fkb_p->len);

        dirty_p = _to_dptr_from_kptr_(fkb_p->data  + fkb_p->len);
        if ( fkb_p->dirty_p < dirty_p )
            fkb_p->dirty_p = dirty_p;

        /* unshare by decrementing reference count */
        fkb_dec_ref(fkb_p);

        fkb_dbg(1, "master fkb_p with multiple ref");
        fkb_dbg(2, "fkbM_p<%px> fkbM_data<%px> dirty_p<%px> len<%d>",
            fkbM_p, fkbM_p->data, fkbM_p->dirty_p, fkbM_p->len);
        fkb_dbg(2, "fkb_p<%px> fkb_data<%px> dirty_p<%px> len<%d>",
            fkb_p, fkb_p->data, fkb_p->dirty_p, fkb_p->len);
        return fkbM_p;  /* return new allocate master FkBuff */
    }
}

/*
 *------------------------------------------------------------------------------
 * Function   : fkbM_borrow
 * Description: Fetch a Master FKB from the Master FKB pool. A Master FKB Object
 *              Pool can serve as a network device driver's preallocated buffer
 *              pool overflow.
 *------------------------------------------------------------------------------
 */
FkBuff_t * fkbM_borrow(void)
{
    FkBuff_t * fkbM_p;

    fkbM_p = fkb_alloc( FkbMaster_e );

    fkb_dbg(1, "fkbM_p<%px>", fkbM_p);
    return fkbM_p;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fkb_xlate
 * Description: Translates an FKB to an SKB allocated from kernel SKB cache.
 *              If the FKB is refering to a packet with multiple FKB references
 *              to it then it will be first unshared, before it is translated
 *              to a SKB. Unsharing is done by allocating a Master FKB from the
 *              Master FKB Pool.
 *------------------------------------------------------------------------------
 */
extern struct sk_buff * skb_xlate_dp(FkBuff_t * fkb_p, uint8_t *dirty_p);

struct sk_buff * fkb_xlate(FkBuff_t * fkb_p)
{
    struct sk_buff * skb_p;
    uint8_t *dirty_p;

    FKB_AUDIT(
        if ( smp_processor_id() )
            printk("FKB ASSERT %s not supported on CP 1\n", __FUNCTION__); );

    FKB_AUDIT_RUN();

    if ( unlikely(fkb_p == FKB_NULL) )
        return (struct sk_buff *)NULL;

    fkb_assertr( (!_IS_BPTR_(fkb_p->ptr)), (struct sk_buff *)NULL );

        /* Ensure that only a single reference exists to the FKB */
    fkb_p = fkb_unshare(fkb_p);
    if ( unlikely(fkb_p == FKB_NULL) )
        goto clone_fail;

    /* carry the dirty_p to the skb */
    dirty_p = (is_dptr_tag_(fkb_p->dirty_p)) ?
               _to_kptr_from_dptr_(fkb_p->dirty_p) : NULL;

        /* Now translate the fkb_p to a skb_p */
    skb_p = skb_xlate_dp(fkb_p, dirty_p);

    if ( unlikely(skb_p == (struct sk_buff *)NULL) )
        goto clone_fail;

        /* pNBuff may not be used henceforth */
    return skb_p;

clone_fail:
    return (struct sk_buff *)NULL;
}


/*
 *------------------------------------------------------------------------------
 * Function     : nbuff_align_data
 * Description  : Aligns NBUFF data to a byte boundary defined by alignMask
 *                This function can be called ONLY by driver Transmit functions
 *------------------------------------------------------------------------------
 */
pNBuff_t nbuff_align_data(pNBuff_t pNBuff, uint8_t **data_pp,
                          uint32_t len, unsigned long alignMask)
{
    fkb_dbg(1, "pNBuff<%px>", pNBuff);

    FKB_AUDIT(
        if ( smp_processor_id() )
            printk("FKB ASSERT %s not supported on CP 1\n", __FUNCTION__); );

    FKB_AUDIT_RUN();

    if ( IS_SKBUFF_PTR(pNBuff) )
    {
        struct sk_buff * skb_p = PNBUFF_2_SKBUFF(pNBuff);
        uint32_t headroom;
        uint8_t *skb_data_p;

        headroom = (uintptr_t)(skb_p->data) & alignMask;

        if(headroom == 0)
        {
            /* data is already aligned */
            goto out;
        }

        if(skb_cow(skb_p, headroom) < 0)
        {
            kfree_skb(skb_p);

            pNBuff = NULL;
            goto out;
        }

        skb_data_p = (uint8_t *)((uintptr_t)(skb_p->data) & ~alignMask);

        memcpy(skb_data_p, skb_p->data, len);

        skb_p->data = skb_data_p;
        *data_pp = skb_data_p;
    }
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
    else
    {
        FkBuff_t * fkb_p = (FkBuff_t *)PNBUFF_2_PBUF(pNBuff);
        FkBuff_t * fkb2_p;
        uint32_t headroom;
        uint8_t *fkb_data_p;

        headroom = (uintptr_t)(fkb_p->data) & alignMask;

        if(headroom == 0)
        {
            /* data is already aligned */
            goto out;
        }

        if(fkb_headroom(fkb_p) < headroom)
        {
            fkb_dbg(1, "FKB has no headroom  "
                       "(fkb_p<%px>, fkb_p->data<%px>)",
                       fkb_p, fkb_p->data);

            goto out;
        }

        fkb2_p = fkb_unshare(fkb_p);
        if (fkb2_p == FKB_NULL)
        {
            fkb_free(fkb_p);
            pNBuff = NULL;
            goto out;
        }
        pNBuff = FKBUFF_2_PNBUFF(fkb2_p);

        fkb_data_p = (uint8_t *)((uintptr_t)(fkb2_p->data) & ~alignMask);

        memcpy(fkb_data_p, fkb2_p->data, len);

        fkb2_p->data = fkb_data_p;
        *data_pp = fkb_data_p;

#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
        {
            uint8_t * tail_p = fkb2_p->data + len; 
            fkb2_p->dirty_p = _to_dptr_from_kptr_(tail_p);
        }
#endif
    }

    fkb_dbg(2, "<<");

out:
    FKB_AUDIT_RUN();

    return pNBuff;
}

/*
 * Function   : nbuff_flush
 * Description: Flush (Hit_Writeback_Inv_D) a network buffer's packet data.
 */
void nbuff_flush(pNBuff_t pNBuff, uint8_t * data, int len)
{
    fkb_dbg(1, "pNBuff<%p> data<%p> len<%d>",
            pNBuff, data, len);
    if ( IS_SKBUFF_PTR(pNBuff) )
    {
#if !defined(CONFIG_BCM_GLB_COHERENCY)
/* Optimized flush for BPM buffers */
#if IS_ENABLED(CONFIG_BCM_BPM)
        struct sk_buff *skb = (struct sk_buff *)pNBuff;
        /* Use the  dirty pointer cache flush optimization when the BPM is in
           PRISTINE state which means the BPM is allocated by WFD/WLAN */
        if (skb->recycle_flags & SKB_BPM_PRISTINE) {
            uint8_t *dirty_p = (uint8_t*)skb_shinfo(skb)->dirty_p;
            if ((dirty_p != NULL) && (dirty_p > (uint8_t *)data))
            {
                len = (uint8_t *)(dirty_p) - (uint8_t *)data;
                len = len > BCM_MAX_PKT_LEN ? BCM_MAX_PKT_LEN : len;
            }
        }
#endif /* CONFIG_BCM_BPM || CONFIG_BCM_BPM_MODULE */
        if (len > 0) {
            cache_flush_len(data, len);
        }
#endif /* !CONFIG_BCM_GLB_COHERENCY */
    }
    else
    {
        FkBuff_t * fkb_p = (FkBuff_t *)PNBUFF_2_PBUF(pNBuff);
        fkb_flush(fkb_p, data, len, FKB_CACHE_FLUSH); 
    }
    fkb_dbg(2, "<<");
}

/*
 * Function   : nbuff_free
 * Description: Free/recycle a network buffer and associated data
 *
 * Freeing may involve a recyling of the network buffer into its respective
 * pool (per network device driver pool, kernel cache or FKB pool). Likewise
 * the associated buffer may be recycled if there are no other network buffers
 * referencing it.
 */
void nbuff_free_ex(pNBuff_t pNBuff, int in_thread)
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    fkb_dbg(1, "pNBuff<%p> pBuf<%p>", pNBuff, pBuf);
    if ( IS_SKBUFF_PTR(pNBuff) )
    {
#ifdef CONFIG_BCM_SKB_FREE_THREAD
        if(!in_thread)
            dev_kfree_skb_any((struct sk_buff *)pBuf);
#endif
        else
            dev_kfree_skb_thread((struct sk_buff *)pBuf);
    }
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
    else
        fkb_free(pBuf);
    fkb_dbg(2, "<<");
}

void nbuff_free(pNBuff_t pNBuff)
{
#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM_HNDROUTER) || defined(CONFIG_BCM947189)
    nbuff_free_ex(pNBuff, 0);
#else
    nbuff_free_ex(pNBuff, 1);
#endif
}

/*
 * Function   : nbuff_flushfree
 * Description: Flush (Hit_Writeback_Inv_D) and free/recycle a network buffer.
 * If the data buffer was referenced by a single network buffer, then the data
 * buffer will also be freed/recycled. 
 */
static inline void nbuff_flushfree_ex(pNBuff_t pNBuff, int in_thread)
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    fkb_dbg(1, "pNBuff<%px> pBuf<%px>", pNBuff, pBuf);
    if ( IS_SKBUFF_PTR(pNBuff) )
    {
        cache_flush_len(((struct sk_buff *)pBuf)->data, ((struct sk_buff *)pBuf)->len);
#ifdef CONFIG_BCM_SKB_FREE_THREAD
        if(in_thread)
            dev_kfree_skb_thread((struct sk_buff *)pBuf);
        else
#endif
            dev_kfree_skb_irq((struct sk_buff *)pBuf);
    }
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
    else
    {
        FkBuff_t * fkb_p = (FkBuff_t *)pBuf;
        fkb_flush(fkb_p, fkb_p->data, fkb_p->len, FKB_CACHE_FLUSH);
        fkb_free(fkb_p);
    }
    fkb_dbg(2, "<<");
}


void nbuff_flushfree(pNBuff_t pNBuff)
{
#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM_HNDROUTER) || defined(CONFIG_BCM947189)
    nbuff_flushfree_ex(pNBuff, 0);
#else
    nbuff_flushfree_ex(pNBuff, 1);
#endif
}

/*
 * Function   : fkb_flush
 * Description: Flush a FKB from current data or received packet data upto
 * the dirty_p. When Flush Optimization is disabled, the entire length.
 */
void fkb_flush(FkBuff_t * fkb_p, uint8_t * data_p, int len, int cache_op)
{
    uint8_t * fkb_data_p;

    if ( _is_fkb_cloned_pool_(fkb_p) )
        fkb_data_p = PFKBUFF_TO_PDATA(fkb_p->master_p, BCM_PKT_HEADROOM);
    else
        fkb_data_p = PFKBUFF_TO_PDATA(fkb_p, BCM_PKT_HEADROOM);

    /* headers may have been popped */
    if ( (uintptr_t)data_p < (uintptr_t)fkb_data_p )
        fkb_data_p = data_p;

    {
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
    uint8_t * dirty_p;  /* Flush only L1 dirty cache lines */
    dirty_p = _to_kptr_from_dptr_(fkb_p->dirty_p);  /* extract kernel pointer */

    fkb_dbg(1, "fkb_p<%p> fkb_data<%p> dirty_p<%p> len<%d>",
            fkb_p, fkb_data_p, dirty_p, len);

    if (cache_op == FKB_CACHE_FLUSH)
        cache_flush_region(fkb_data_p, dirty_p);
    else
        cache_invalidate_region(fkb_data_p, dirty_p);
#else
    uint32_t data_offset;
    data_offset = (uintptr_t)data_p - (uintptr_t)fkb_data_p;

    fkb_dbg(1, "fkb_p<%p> fkb_data<%p> data_offset<%d> len<%d>",
            fkb_p, fkb_data_p, data_offset, len);

    if (cache_op == FKB_CACHE_FLUSH)
        cache_flush_len(fkb_data_p, data_offset + len);
    else
        cache_invalidate_len(fkb_data_p, data_offset + len);
#endif
    }
}

/*
 *------------------------------------------------------------------------------
 * Function Name: cache_flush_data_len
 * Description  : Flush Cache
 *------------------------------------------------------------------------------
 */
void cache_flush_data_len(void *addr, int len)
{
    cache_flush_len(addr, len);
}


EXPORT_SYMBOL(nbuff_dbg);

EXPORT_SYMBOL(fkb_in_skb_test);
EXPORT_SYMBOL(fkb_construct);
EXPORT_SYMBOL(fkb_stats);

EXPORT_SYMBOL(fkb_alloc);
EXPORT_SYMBOL(fkb_free);

EXPORT_SYMBOL(fkb_unshare);

EXPORT_SYMBOL(fkbM_borrow);

EXPORT_SYMBOL(fkb_xlate);
EXPORT_SYMBOL(nbuff_align_data);
EXPORT_SYMBOL(nbuff_flush);
EXPORT_SYMBOL(nbuff_free_ex);
EXPORT_SYMBOL(nbuff_free);
EXPORT_SYMBOL(nbuff_flushfree);
EXPORT_SYMBOL(fkb_flush);
EXPORT_SYMBOL(cache_flush_data_len);
