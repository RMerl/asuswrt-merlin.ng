/*
    <:copyright-BRCM:2011:proprietary:standard
    
       Copyright (c) 2011 Broadcom 
       All Rights Reserved
    
     This program is the proprietary software of Broadcom and/or its
     licensors, and may only be used, duplicated, modified or distributed pursuant
     to the terms and conditions of a separate, written license agreement executed
     between you and Broadcom (an "Authorized License").  Except as set forth in
     an Authorized License, Broadcom grants no license (express or implied), right
     to use, or waiver of any kind with respect to the Software, and Broadcom
     expressly reserves all rights in and to the Software and all intellectual
     property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
     NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
     BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
    
     Except as expressly set forth in the Authorized License,
    
     1. This program, including its structure, sequence and organization,
        constitutes the valuable trade secrets of Broadcom, and you shall use
        all reasonable efforts to protect the confidentiality thereof, and to
        use this information only in connection with your use of Broadcom
        integrated circuit products.
    
     2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
        AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
        WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
        RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
        ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
        FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
        COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
        TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
        PERFORMANCE OF THE SOFTWARE.
    
     3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
        ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
        INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
        WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
        IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
        OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
        SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
        SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
        LIMITED REMEDY.
    :>    


*/
/***************************************************************************
 * File Name  : xtmoslinux.c (impl2)
 *
 * Description: This file contains Linux operation system function calls.
 *              All operating system specific functions are isolated to a
 *              source file named XtmOs"xxx" where "xxx" is the operating
 *              system name.
 ***************************************************************************/


/* Includes. */
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/types.h>
#include <asm/io.h>
#include <bcmtypes.h>
#include <atmosservices.h>
#include <bcm_map.h>
#include <bcm_intr.h>
#include <board.h>
#include <boardparms.h>
#include <bcmnetlink.h>


#define XTM_EVT_TRAFFIC_TYPE_MISMATCH              1   /* Also duplicated in xtmcfgimpl.h */
#define XTM_EVT_TRAFFIC_TYPE_MISMATCH_AND_RESTART  2   /* Also duplicated in xtmcfgimpl.h */

/* For the 63138 and 63148, implement a workaround to strip bytes and
   allow OAM traffic due to JIRA HW63138-12 */
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
#define XTM_OAM_STRIP_BYTE_WORKAROUND_ENABLED      3   /* Also duplicated in xtmcfgimpl.h */
#define XTM_OAM_STRIP_BYTE_WORKAROUND_DISABLED     4   /* Also duplicated in xtmcfgimpl.h */
#endif
#define XTM_EVT_BONDING_GROUP_ID_MISMATCH          5   /* Also duplicated in xtmcfgimpl.c */

/* typedefs. */
typedef struct
{
    UINT32 ulInUse;
    UINT32 ulTimeout;
    struct tasklet_struct Task;
    struct timer_list TimerList;
} DEFERRED_INFO, *PDEFERRED_INFO;

typedef struct
{
#if defined(CONFIG_ARM64)
    uintptr_t ulBase;
#else
    UINT32 ulBase;
#endif
    UINT32 ulOrder;
} PAGE_ALLOC_INFO, *PPAGE_ALLOC_INFO;

/* Defines. */
#define MAX_SEMS            50
#define MAX_LOCKS           50
#define MAX_PAGE_ALLOCS     50
#define MAX_INT_DATA_TIMERS 8

#define INTR_ATM            (1 << (INTERRUPT_ID_ATM - INTERNAL_ISR_TABLE_OFFSET))

#if defined(CONFIG_BCM963158)
#define index_of( array, elem )     ({                      \
                                        int i;              \
                                        i = (elem) >= (array) ? (INT32)((char *)(elem)-(char *)(array)) / sizeof((array)[0]) : -1;            \
                                        i = (i!=-1) && (((INT32)((char *)(elem) - (char *)(array)) - sizeof((array)[0]) * i) == 0) ? i : -1; \
                                        i; \
                                    })
#else
#define index_of( array, elem )     ({                      \
                                        int i;              \
                                        i = (elem) >= (array) ? (long int)((char *)(elem)-(char *)(array)) / sizeof((array)[0]) : -1;            \
                                        i = (i!=-1) && (((long)((char *)(elem) - (char *)(array)) - sizeof((array)[0]) * i) == 0) ? i : -1; \
                                        i; \
                                    })
#endif


/* Globals. */
static struct InternalData
{
    struct semaphore Sems[MAX_SEMS];
    int    isSemTaken[MAX_SEMS];
    spinlock_t Locks[MAX_LOCKS];
    int    isLockTaken[MAX_LOCKS];
    DEFERRED_INFO DeferredInfo;
    PAGE_ALLOC_INFO PageAllocInfo[MAX_PAGE_ALLOCS];
    int nDisabledBh;
    int nRequestCount;
    struct timer_list IntDataTimer[MAX_INT_DATA_TIMERS];
    void*  IntDataTimerHdlr [MAX_INT_DATA_TIMERS];
    int    IntDataTimerShutDown [MAX_INT_DATA_TIMERS]; 
    spinlock_t timerlock;
    
} InternalData;

static struct InternalData *g_pData = &InternalData;

/***************************************************************************
 * Function Name: XtmOsInitialize
 * Description  : Operating system specific function initialization.
 * Returns      : RTN_SUCCESS if successful or error status.
 ***************************************************************************/
void XtmOsInitialize( void )
{
    int i;

    memset( g_pData, 0x00, sizeof(struct InternalData) );

    for (i = 0; i < MAX_SEMS; i++)
        sema_init(&g_pData->Sems[i], 1);

    for (i = 0; i < MAX_LOCKS; i++)
        spin_lock_init(&g_pData->Locks[i]);
    memset( &g_pData->DeferredInfo, 0x00, sizeof(g_pData->DeferredInfo) );
    memset( g_pData->PageAllocInfo, 0x00, sizeof(g_pData->PageAllocInfo) );
    g_pData->nDisabledBh = 0;
    g_pData->nRequestCount = 0;
    spin_lock_init(&g_pData->timerlock);

} /* XtmOsInitialize */


/***************************************************************************
 * Function Name: XtmOsAlloc
 * Description  : Allocates kernel memory.
 * Returns      : Address of allocated memory of NULL.
 ***************************************************************************/
char *XtmOsAlloc( UINT32 ulSize )
{
    char *pRet = NULL;
    int nGfpDma = GFP_DMA;

    /* The largest block that kmalloc can allocate is 128K.  If the size is
     * greater than 128K, allocate memory pages.
     */
    if( ulSize < (1024 * 128)  )
    {
        if( in_softirq() || in_irq() )
            pRet = (char *) kmalloc(ulSize, GFP_ATOMIC);
        else
            pRet = (char *) kmalloc(ulSize, GFP_KERNEL);
    }
    else
    {
        /* Memory pages need to be allocated.  The number of pages must be an
         * exponent of 2
         */
        PPAGE_ALLOC_INFO pPai;
        UINT32 i;
        for(i=0, pPai=g_pData->PageAllocInfo; i < MAX_PAGE_ALLOCS; i++, pPai++)
        {
            if( pPai->ulBase == 0 )
            {
                pPai->ulOrder = 0;
                while( ulSize > (PAGE_SIZE * (1 << pPai->ulOrder)))
                    pPai->ulOrder++;
                pPai->ulBase = __get_free_pages(GFP_KERNEL | nGfpDma,
                    pPai->ulOrder);
                pRet = (char *) pPai->ulBase;
                break;
            }
        }
    }

    return( pRet );
} /* XtmOsAlloc */


/***************************************************************************
 * Function Name: XtmOsFree
 * Description  : Frees memory.
 * Returns      : None.
 ***************************************************************************/
void XtmOsFree( char *pBuf )
{
    PPAGE_ALLOC_INFO pPai;
    UINT32 i;

    if( pBuf )
    {
        for(i=0, pPai=g_pData->PageAllocInfo; i < MAX_PAGE_ALLOCS; i++, pPai++)
        {
            if( pPai->ulBase == (uintptr_t) pBuf )
            {
                free_pages( pPai->ulBase, pPai->ulOrder );
                pPai->ulBase = (uintptr_t)NULL;
                pPai->ulOrder = 0;
                break;
            }
        }

        if( i == MAX_PAGE_ALLOCS )
            kfree( pBuf );
    }
}


/***************************************************************************
 * Function Name: XtmOsCreateSem
 * Description  : Finds an unused semaphore and initializes it.
 * Returns      : Semaphore handle if successful or NULL.
 ***************************************************************************/
void *XtmOsCreateSem( UINT32 ulInitialState )
{
    int i;
    struct semaphore *pSem = NULL;
    
    for (i = 0; i < MAX_SEMS; i++)
    {
        if (g_pData->isSemTaken[i] == 0) // quick test outside of the lock to prevent unnecessary locking
        {
            pSem = &g_pData->Sems[i];
            // we're going to lock the actual semaphore to take it:
            if (down_trylock( pSem ) != 0)
                continue; //grabbing semaphore was unsuccesful -- go to next semaphore
            else
            {
                // Semaphore is locked.  We have to check taken again now that we're locked
                if ( unlikely(g_pData->isSemTaken[i] == 1))
                {
                    // semaphore already taken, move to next:
                    up(pSem);
                    continue;
                }
                else
                {
                    g_pData->isSemTaken[i] = 1;
                    // up(pSem); don't have to do this because of the sema_init
                    /* Note, no one else can take this semaphore at this point, so we're free to
                       reinitialize it */
                    sema_init(pSem, ulInitialState) ;                   
                    return pSem;
                }
            }
        }
    }

    // no free semaphores -- failing:
    return NULL;
}

/***************************************************************************
 * Function Name: XtmOsCreateLock
 * Description  : Finds an unused spin_lock and initializes it.
 * Returns      : spin_lock handle if successful or NULL.
 ***************************************************************************/
void *XtmOsCreateLock(void)
{
    int i;
    spinlock_t *pLock = NULL;
    
    for (i = 0; i < MAX_LOCKS; i++)
    {
        if (g_pData->isLockTaken[i] == 0) // quick test outside of the lock to prevent unnecessary locking
        {
            pLock = &g_pData->Locks[i];
            // we're going to lock the actual spinlock to take it:
            if(spin_trylock(pLock) != 1) 
                continue; //grabbing spinlock was unsuccesful -- go to next semaphore
            else
            {
                // spinlock is locked.  
                if ( unlikely(g_pData->isLockTaken[i] == 1))
                {
                    spin_unlock(pLock);
                    continue;
                }
                else
                {
                    g_pData->isLockTaken[i] = 1;
                    spin_unlock(pLock);
                    return (void *)pLock;
                }
            }
        }
    }

    // no free spinlock -- failing:
    return (void *)pLock;
}

/***************************************************************************
 * Function Name: XtmOsDeleteSem
 * Description  : Makes semaphore available.
 * Returns      : None.
 ***************************************************************************/
void XtmOsDeleteSem( void *ulSem )
{
    /* Mark semaphore unused. */
    int i;
    i = index_of(g_pData->Sems, (struct semaphore *) ulSem);
    
    if (i == -1 || i > MAX_SEMS)
    {
        printk("ERROR %s\n\n\n", __func__); // make sure someone notices!!
        return; // bad sem Id;
    }

    sema_init((struct semaphore *) ulSem, 1);
    g_pData->isSemTaken[i] = 0;

} /* XtmOsDeleteSem */

/***************************************************************************
 * Function Name: XtmOsDeleteLock
 * Description  : Makes spinlock available.
 * Returns      : None.
 ***************************************************************************/
void XtmOsDeleteLock( unsigned long pLock )
{
    /* Mark spinlock unused. */
    int i;
    i = index_of(g_pData->Locks, (spinlock_t *) pLock);
    
    if (i == -1 || i > MAX_LOCKS)
    {
        printk("ERROR %s\n\n\n", __func__); // make sure someone notices!!
        return; // bad lock Id;
    }

    spin_lock_init((spinlock_t *)pLock);
    g_pData->isLockTaken[i] = 0;

} /* XtmOsDeleteLock */

/***************************************************************************
 * Function Name: XtmOsRequestSemTo
 * Description  : Timeout handler for XtmOsRequestSem
 * Returns      : 0 if successful.
 ***************************************************************************/
void XtmOsRequestSemTo( unsigned long ulSem )
{
    up( (struct semaphore *) ulSem );
} /* XtmOsRequestSemTo */


/***************************************************************************
 * Function Name: XtmOsRequestSem
 * Description  : Requests ownership of the semaphore. ulTimeout is in ms.
 * Returns      : 0 if successful.
 ***************************************************************************/
unsigned long XtmOsRequestSem( unsigned long ulSem, UINT32 ulTimeout )
{
    int nRet = 0;

    /* A bottom half should run without any task context switches and also
     * should not block.  Therefore, just return if currently executing in a bh.
     */
    g_pData->nRequestCount++;
    if( !in_softirq() )
    {
        /* If the timeout is big, no need to start timer. */
        if( ulTimeout > 0x80000000 )
        {
            nRet = down_interruptible( (struct semaphore *) ulSem );

            /* At this point, the current thread is protected from other tasks
             * but not from a bottom half.  Disable bottom halves.  Doing this
             * assumes that the semaphore is being used as a mutex.  A future
             * change may be necessary to have both critical section functions
             * and synchronization functions that use semaphores.
             */
            if( nRet == 0 && g_pData->nDisabledBh == 0 )
            {
                g_pData->nDisabledBh = 1;
                local_bh_disable();
            }
        }
        else
        {
            struct timer_list Timer;
            int nTimerDeleted;

            /* Convert ms to jiffies.  If the timeout is less than the
             * granularity of the system clock, wait one jiffy.
             */
            if( (ulTimeout = (ulTimeout * HZ) / 1000) == 0 )
                ulTimeout = 1;

            init_timer (&Timer);
            Timer.expires = jiffies + ulTimeout;
            Timer.function = XtmOsRequestSemTo;
            Timer.data = (unsigned long)ulSem;

            add_timer (&Timer);
            nRet = down_interruptible( (struct semaphore *) ulSem );
            nTimerDeleted = del_timer( &Timer );

            /* If the timer is still active and the semaphore was not
             * interrupted, then there was a timeout.
             */
            if( nTimerDeleted == 0 && !nRet )
                nRet = -1; /* Timed out. */
        }
    }

    return( nRet );
} /* XtmOsRequestSem */


/***************************************************************************
 * Function Name: XtmOsReleaseSem
 * Description  : Releases ownership of the semaphore.
 * Returns      : None.
 ***************************************************************************/
void XtmOsReleaseSem( unsigned long ulSem )
{
    if( --g_pData->nRequestCount <= 0 )
    {
        g_pData->nRequestCount = 0;

        /* If XtmOsRequestSem had disabled bottom havles, reenable here. */
        if( g_pData->nDisabledBh == 1 )
        {
            g_pData->nDisabledBh = 0;
            local_bh_enable();
        }
    }

    if( !in_softirq() )
        up( (struct semaphore *) ulSem );
} /* XtmOsReleaseSem */

void XtmOsRequestLock(unsigned long pLock)
{
   spin_lock_bh((spinlock_t *)pLock);
}

void XtmOsReleaseLock(unsigned long pLock)
{
   spin_unlock_bh((spinlock_t *)pLock);
}

/***************************************************************************
 * Function Name: XtmOsDisableInts
 * Description  : Disables interrupts.
 * Returns      : 1 for compatibility
 ***************************************************************************/
unsigned long XtmOsDisableInts( void )
{
    unsigned long flags;

    local_save_flags(flags);
    local_irq_disable();
    return( flags );
} /* XtmOsDisableInts */


/***************************************************************************
 * Function Name: XtmOsEnableInts
 * Description  : Enables interrupts.
 * Returns      : None.
 ***************************************************************************/
void XtmOsEnableInts( unsigned long flags )
{
    local_irq_restore(flags);
} /* XtmOsEnableInts */


/***************************************************************************
 * Function Name: XtmOsDelay
 * Description  : Delays a specified number of milliseconds.
 * Returns      : None.
 ***************************************************************************/
void XtmOsDelay( UINT32 ulTimeout )
{
    if( !in_softirq() && !in_irq() )
    {
        wait_queue_head_t wait;

        /* Convert ms to jiffies.  If the timeout is less than the granularity of
         * the system clock, wait one jiffy.
         */

        if( (ulTimeout = (ulTimeout * HZ) / 1000) == 0 )
            ulTimeout = 1;

        init_waitqueue_head(&wait);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
        interruptible_sleep_on_timeout(&wait, (int) ulTimeout );
#else
        wait_event_interruptible_timeout(wait, 1 == 0, ulTimeout); 
#endif

    }
} /* XtmOsDelay */


/***************************************************************************
 * Function Name: XtmOsTickGet
 * Description  : Returns the current number of milliseconds since the board
 *                was booted.
 * Returns      : Current number of milliesconds.
 ***************************************************************************/
UINT32 XtmOsTickGet( void )
{
    return( jiffies * (1000 / HZ) );
} /* XtmOsTickGet */


#if !defined(CONFIG_BCM963158)
/***************************************************************************
 * Function Name: XtmOsTickCheck
 * Description  : Calculates if the number of milliseconds has expired.
 * Returns      : 1 if the number of milliseconds has expired, 0 if not.
 ***************************************************************************/
UINT32 XtmOsTickCheck( unsigned long ulWaitBase, unsigned long ulMsToWait )
{
    return( time_before(jiffies, ((HZ * (ulWaitBase + ulMsToWait)) / 1000))
        ? 0 : 1 );
} /* XtmOsTickCheck */
#endif	//Needs check

/***************************************************************************
 * Function Name: XtmOsDeferredTo
 * Description  : Timeout handler for XtmOsInitDeferredHandler.
 * Returns      : 0 if successful.
 ***************************************************************************/
void XtmOsDeferredTo( unsigned long ulHandle )
{
    PDEFERRED_INFO pDeferredInfo = (PDEFERRED_INFO) ulHandle;
    if( pDeferredInfo->ulInUse == 1 )
    {
        /* Schedule the bottom half. */
        tasklet_schedule(&pDeferredInfo->Task);

        /* Restart the timer. */
        pDeferredInfo->TimerList.expires = jiffies + pDeferredInfo->ulTimeout;
        pDeferredInfo->TimerList.function = XtmOsDeferredTo;
        pDeferredInfo->TimerList.data = (uintptr_t)pDeferredInfo;
        add_timer ( &pDeferredInfo->TimerList );
    }
} /* XtmOsDeferredTo */


/***************************************************************************
 * Function Name: XtmOsInitDeferredHandler
 * Description  : Sets up a function for post ISR processing.
 * Returns      : handle that is used in subsequent calls or 0.
 ***************************************************************************/
unsigned long XtmOsInitDeferredHandler( void *pFnEntry, uintptr_t ulFnParm,
    UINT32 ulTimeout )
{
    unsigned long ulHandle = 0;

    if( g_pData->DeferredInfo.ulInUse == 0 )
    {
        g_pData->DeferredInfo.ulInUse = 1;

        /* Initialize a tasklet. */
        tasklet_init(&(g_pData->DeferredInfo.Task), (void *) pFnEntry, (unsigned long) ulFnParm);

        /* Start a timer.  Convert ms to jiffies.  If the timeout is less than
         * the granularity of the system clock, wait one jiffy.
         */
        if( (ulTimeout = (ulTimeout * HZ) / 1000) == 0 )
            g_pData->DeferredInfo.ulTimeout = 1;
        else
            g_pData->DeferredInfo.ulTimeout = ulTimeout;

        init_timer( &g_pData->DeferredInfo.TimerList );
        g_pData->DeferredInfo.TimerList.expires = jiffies + g_pData->DeferredInfo.ulTimeout;
        g_pData->DeferredInfo.TimerList.function = XtmOsDeferredTo;
        g_pData->DeferredInfo.TimerList.data = (unsigned long)&g_pData->DeferredInfo;
        add_timer( &g_pData->DeferredInfo.TimerList);

        ulHandle = (unsigned long) &g_pData->DeferredInfo;
    }
    return( ulHandle );
} /* XtmOsInitDeferredHandler */


/***************************************************************************
 * Function Name: XtmOsScheduleDeferred
 * Description  : Schedules the deferred processing function to run.
 * Returns      : None.
 ***************************************************************************/
void XtmOsScheduleDeferred( void *ulHandle )
{
    PDEFERRED_INFO pDeferredInfo = (PDEFERRED_INFO) ulHandle;

    if( pDeferredInfo->ulInUse == 1 )
    {
        tasklet_schedule(&pDeferredInfo->Task);
    }
} /* XtmOsScheduleDeferred */


/***************************************************************************
 * Function Name: XtmOsUninitDeferredHandler
 * Description  : Uninitializes the deferred handler resources.
 * Returns      : handle that is used in subsequent calls or 0.
 ***************************************************************************/
void XtmOsUninitDeferredHandler( unsigned long ulHandle )
{
    PDEFERRED_INFO pDeferredInfo = (PDEFERRED_INFO) ulHandle;

    if( pDeferredInfo->ulInUse == 1 )
    {
        pDeferredInfo->ulInUse = 2;
        del_timer( &pDeferredInfo->TimerList );
        pDeferredInfo->ulInUse = 0;
    }
} /* XtmOsUninitDeferredHandler */


/***************************************************************************
 * Function Name: XtmOsStartTimer
 * Description  : Starts a timer.
 * Returns      : 0 if successful, -1 if not
 ***************************************************************************/
UINT32 XtmOsStartTimer( void *pFnEntry, unsigned long ulFnParm, UINT32 ulTimeout )
{
    UINT32 ulRet = (UINT32) -1;
    struct timer_list *pTimer;
    int i;

    /* Lock critical section to access the timer array*/
    spin_lock_bh(&g_pData->timerlock);
   
    /* Loop through timers to find if the timer is set for deletion, in which
    ** case, no need to start.
    **/
    for( i = 0, pTimer = &g_pData->IntDataTimer[0]; i < MAX_INT_DATA_TIMERS;
        i++, pTimer++ )
    {
       if (g_pData->IntDataTimerHdlr[i] == pFnEntry) {

          if (g_pData->IntDataTimerShutDown[i])
             goto _unlock_exit ;
       }
    }

    for( i = 0, pTimer = &g_pData->IntDataTimer[0]; i < MAX_INT_DATA_TIMERS;
        i++, pTimer++ )
        {
    /* Loop through timers to find the timer or a free one for (Re)start.  */
       if ((g_pData->IntDataTimerHdlr[i] == NULL) || (g_pData->IntDataTimerHdlr[i] == pFnEntry)) {

        if( !timer_pending( pTimer ) )
        {
            /* Start a timer.  Convert ms to jiffies.  If the timeout is less than
             * the granularity of the system clock, wait one jiffy.
             */
            if( (ulTimeout = (ulTimeout * HZ) / 1000) == 0 )
                ulTimeout = 1;

            init_timer( pTimer );
            pTimer->expires = jiffies + ulTimeout;
            pTimer->function = pFnEntry;
            pTimer->data = ulFnParm;
              add_timer( pTimer ) ;
              g_pData->IntDataTimerShutDown[i] = 0 ;
              g_pData->IntDataTimerHdlr[i] = pFnEntry ;
            ulRet = 0;
             goto _unlock_exit;
    }
       } /* if */
    } /* for (i) */


_unlock_exit :
    /* Unlock critical section */
    spin_unlock_bh(&g_pData->timerlock);

    /* Did we find an open timer entry? */
    if(i>=MAX_INT_DATA_TIMERS) {
        int j;  /* Counter variable for use in tiemr dump */

        /* No.  Flag an error */
        printk("ERROR(%s): Unable to schedule a timer event for routine %pS."
               "  Too many timers already allocated.\n", __func__, pFnEntry);

        /* Dump the already allocated timers */
        for( j = 0, pTimer = &g_pData->IntDataTimer[0]; j < MAX_INT_DATA_TIMERS;
            j++, pTimer++ ) {
            printk("    timer %d, pending=%c, expires=%lu, function=%pS\n", 
                   j, (timer_pending( pTimer ) ? 'Y' : 'N'), pTimer->expires, pTimer->function);
        }
    }

    return( ulRet );
} /* XtmOsStartTimer */


/***************************************************************************
 * Function Name: XtmOsStopTimer
 * Description  : Stop all pending timers for a given function.
 * Returns      : Number of timers stopped, 0 if no timers stopped
 ***************************************************************************/
int XtmOsStopTimer( void *pFnEntry )
{
    int iRet = -1;
    struct timer_list *pTimer;
    int i;

    /* Lock critical section to access the timer array*/
    spin_lock_bh(&g_pData->timerlock);
   
    /* Loop through timers to see which ones need to be deleted */
    for( i = 0, pTimer = &g_pData->IntDataTimer[0]; i < MAX_INT_DATA_TIMERS;
        i++, pTimer++ )
    {
        /* Is it a pending timer? */
        if( timer_pending( pTimer ) )
        {
            /* Yes.  Does the function match the passed parameter? */
            if(pTimer->function == pFnEntry) {
                int try = 3, delay ;
                /* Delete the timer */
                g_pData->IntDataTimerShutDown[i] = 1 ;
                spin_unlock_bh(&g_pData->timerlock);
                while (((iRet = try_to_del_timer_sync(pTimer)) <= 0)
                       &&
                       (try!=0)) {
                   try-- ;
                   for (delay=200; delay > 0; delay--)
                      ;
                }
                spin_lock_bh(&g_pData->timerlock);
                g_pData->IntDataTimerShutDown[i] = 0 ;
                if (iRet <= 0)
                   break ;
                g_pData->IntDataTimerHdlr[i] = NULL ;
            }
        }
    }

    /* Unlock critical section */
    spin_unlock_bh(&g_pData->timerlock);

    return( iRet );
} /* XtmOsStopTimer */

void DummyXtmOsPrintf( char *pFmt, ... )
{
} /* XtmOsPrintf */

/***************************************************************************
 * Function Name: XtmOsPrintf
 * Description  : Outputs text to the console.
 * Returns      : None.
 ***************************************************************************/
void XtmOsPrintf( char *pFmt, ... )
{
    va_list args;
    char buf[256];

    va_start(args, pFmt);
    vsnprintf(buf, sizeof(buf), pFmt, args);
    va_end(args);

    printk(buf);
} /* XtmOsPrintf */


/***************************************************************************
 * Function Name: XtmOsChipRev
 * Description  : Returns the DSL chip revision.
 * Returns      : None.
 ***************************************************************************/
UINT32 XtmOsChipRev( void )
{
    return( PERF->RevID & 0xfffeffff );
} /* XtmOsChipRev */


void XtmOsGetTimeOfDay(UINT8 *vp, void *dummy)
{
    struct timeval *tvp = (struct timeval *) vp ;
    do_gettimeofday(tvp);
}

void XtmOsAddTimeOfDay(UINT8 *vp, UINT32 sec) 
{
    struct timeval *tvp = (struct timeval *) vp ;
    tvp->tv_sec += sec ;
}

UINT32 XtmOsGetTimeStamp(void)
{
    struct timeval tv;

    XtmOsGetTimeOfDay((UINT8 *) &tv, NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}

UINT32 XtmOsGetTimeStampMs(void)
{
    struct timeval tv;
    UINT32 msec;

    XtmOsGetTimeOfDay((UINT8 *) &tv, NULL);
    msec = (tv.tv_usec != 0) ? tv.tv_usec/1000 : 0 ;
    return ((tv.tv_sec*1000) + msec);
}
//#define XtmOsCompareTimer(ap,bp,compare)  (((struct timeval*) (ap))->tv_sec != ((struct timeval*) (bp))->tv_sec ? ((struct timeval*) (ap))->tv_sec compare ((struct timeval*) (bp))->tv_sec : ((struct timeval*) (ap))->tv_usec compare ((struct timeval*) (bp))->tv_usec)
BOOL XtmOsIsTimerGreater(UINT8 *ap, UINT8 *bp)
{
    struct timeval*  tvp1 = (struct timeval*) ap;
    struct timeval*  tvp2 = (struct timeval*) bp;

    if( tvp1->tv_sec != tvp2->tv_sec && tvp1->tv_sec > tvp2->tv_sec &&
        tvp1->tv_usec > tvp2->tv_usec )
    {
        return TRUE;
    }

    return FALSE;
}

UINT32 XtmOsTimeInUsecs (UINT8 *vp)
{
    struct timeval *tvp = (struct timeval *) vp ;
    return (tvp->tv_sec*1000000+tvp->tv_usec) ;
}

/* This function can be used to invoke monitor task, through XTM events */
void XtmOsSendSysEvent (int EventId)
{
   UINT32  msgData ;

   switch(EventId)
   {
   case XTM_EVT_TRAFFIC_TYPE_MISMATCH:
       msgData = 0x0 ;
       kerSysSendtoMonitorTask (MSG_NETLINK_BRCM_LINK_TRAFFIC_TYPE_MISMATCH,(UINT8 *) &msgData,
                sizeof (msgData)) ;
       break;

   case XTM_EVT_TRAFFIC_TYPE_MISMATCH_AND_RESTART:
       msgData = EventId ;
       kerSysSendtoMonitorTask (MSG_NETLINK_BRCM_LINK_TRAFFIC_TYPE_MISMATCH,(UINT8 *) &msgData,
				sizeof (msgData)) ;
       break;

/* For the 63138 and 63148, implement a workaround to strip bytes and
   allow OAM traffic due to JIRA HW63138-12 */
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)

   case XTM_OAM_STRIP_BYTE_WORKAROUND_ENABLED:
       msgData = 0 ;
       kerSysSendtoMonitorTask (MSG_NETLINK_BRCM_LINK_OAM_STRIP_BYTE,(UINT8 *) &msgData,
				sizeof (msgData)) ;
       break;

   case XTM_OAM_STRIP_BYTE_WORKAROUND_DISABLED:
       msgData = EventId ;
       kerSysSendtoMonitorTask (MSG_NETLINK_BRCM_LINK_OAM_STRIP_BYTE,(UINT8 *) &msgData,
				sizeof (msgData)) ;
       break;
#endif
   case XTM_EVT_BONDING_GROUP_ID_MISMATCH:
       msgData = 0x0 ;
       kerSysSendtoMonitorTask (MSG_NETLINK_BRCM_XTM_BNDGRP_ID_MISMATCH ,(UINT8 *) &msgData,
				sizeof (msgData)) ;
       break;

   default:
       /* No such event.  Flag an error */
       printk("ERROR(%s): Unable to invoke XTM monitor task %d.", 
              __func__, EventId);
   }



	return ;
}
