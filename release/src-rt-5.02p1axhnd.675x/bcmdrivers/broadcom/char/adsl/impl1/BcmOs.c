/* 
* <:copyright-BRCM:2002:proprietary:standard
* 
*    Copyright (c) 2002 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/

//**************************************************************************
// File Name  : BcmOs.c
//
// Description: This file contains wrapper for Linux OS calls
//
//**************************************************************************
#include "BcmOs.h"
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#include <linux/malloc.h>
#else
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
#include <linux/kthread.h>
#include <linux/semaphore.h>
#else
#include <asm/semaphore.h>
#endif
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#include <bcmnetlink.h>	/* kerSysSendtoMonitorTask */
#else
#include <board.h>	/* kerSysWakeupMonitorTask */
#endif

#define index_of( array, elem )     ({                      \
                                        int i;              \
                                        i = (elem) >= (array) ? (long int)((char *)(elem)-(char *)(array)) / sizeof((array)[0]) : -1;            \
                                        i = (i!=-1) && (((long)((char *)(elem) - (char *)(array)) - sizeof((array)[0]) * i) == 0) ? i : -1; \
                                        i; \
                                    })




#define MAX_SEMS            50
#define MAX_DPC				4

/* Static */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
LOCAL struct semaphore g_Sems[MAX_SEMS];
LOCAL int g_isSemTaken[MAX_SEMS] = {0};
#else
LOCAL struct semaphore g_Sems[MAX_SEMS] = {0};
#endif

typedef struct _g_Dpc_t{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
    struct tq_struct		taskQueue;
#else
    struct tasklet_struct   task;
#endif
    struct timer_list       timerList;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
    void                    (*timer_cb_func)(unsigned long arg);
    unsigned long           timer_cb_arg;
#endif
    unsigned long           ulTimeout;
} g_Dpc_t;

LOCAL g_Dpc_t g_Dpc[MAX_DPC];

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
typedef struct _bcm_sema_timer_list{
   struct timer_list to_timer;
   OS_SEMID semid;
}bcm_sema_timer_list;
#endif

/*****************************************************************************
** FUNCTION:   bcmOsTaskCreate
**
** PURPOSE:    Create a task.
**
** PARAMETERS: None
**
** RETURNS:    OS_STATUS_OK - success
**             error code   - failure
**
** NOTE:
******************************************************************************/
OS_STATUS BcmOsInitialize( void )
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
    int i;
    memset( g_isSemTaken, 0x00, sizeof(g_isSemTaken) );
    for (i = 0; i < MAX_SEMS; i++)
    {
        sema_init(&g_Sems[i], 1);
    }
    
#else //(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
    /* Initialize semaphore structures to all bits on in order to determine
     * a not in use state.*/
    memset( g_Sems, 0xff, sizeof(g_Sems) );
#endif
	memset( g_Dpc, 0x00, sizeof(g_Dpc) );

	return OS_STATUS_OK;
}

OS_STATUS BcmOsUninitialize( void )
{
	return OS_STATUS_OK;
}

/*****************************************************************************
** FUNCTION:   bcmOsTaskCreate
**
** PURPOSE:    Create a task.
**
** PARAMETERS: name - name of the of the task
**             stackSize - task stack size
**             priority - task priority
**             taskentry - the task entry point
**             argument - the task argument
**             taskId - task identifier
**
** RETURNS:    OS_STATUS_OK - success
**             error code   - failure
**
** NOTE:
******************************************************************************/
OS_STATUS bcmOsTaskCreate( char* name, int stackSize, int priority,
                       void* taskEntry, OS_TASKARG argument, OS_TASKID *taskId )
{
    typedef int (*FN_GENERIC_INT) (void *);
    struct task_struct * task;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
    task = kthread_run( (FN_GENERIC_INT) taskEntry, (void *) argument, name );
    *taskId = (OS_TASKID)task;
    return( IS_ERR(task) ? OS_STATUS_FAIL : OS_STATUS_OK );
#else
    task = kthread_create( (FN_GENERIC_INT) taskEntry, (void *) argument, name );
    if(!IS_ERR(task)) {
        wake_up_process(task);
        *taskId = (OS_TASKID)task;
    }
    return( IS_ERR(task) ? OS_STATUS_FAIL : OS_STATUS_OK );
#endif

}

/*****************************************************************************
** FUNCTION:   bcmOsTaskDelete
**
** PURPOSE:    deletes a task
**
** PARAMETERS: taskID - task identification
**
**
** RETURNS:    OS_STATUS_OK - success
**             error code   - failure
**
** NOTE:
******************************************************************************/
OS_STATUS bcmOsTaskDelete( OS_TASKID taskId )
{
	return OS_STATUS_OK;
}

/*****************************************************************************
** FUNCTION:   bcmOsSemCreate
**
** PURPOSE:    Create a binary semaphore
**
** PARAMETERS: semName - semaphore name
**             semId - pointer to the semaphore handle
**
** RETURNS:    OS_STATUS_OK - success
**             error code   - failure
**
** NOTE:
******************************************************************************/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
/* The old code was actually not concurrently safe, so 
   I'm rewriting it from scratch for the new kernel */
OS_STATUS bcmOsSemCreate(char *semName, OS_SEMID *semId)
{
    int i;
    struct semaphore *pSem = NULL;
    
    for (i = 0; i < MAX_SEMS; i++)
    {
        if (g_isSemTaken[i] == 0) // quick test outside of the lock to prevent unnecessary locking
        {
            pSem = &g_Sems[i];
            // we're going to lock the semaphore to take it:
            if (down_trylock(pSem ) != 0) {
                continue; //grabbing semaphore was unsuccesful -- go to next semaphore
            }
            else
            {
                // Semaphore is locked.  We have to check taken again now that we're locked
                if ( unlikely(g_isSemTaken[i] == 1))
                {
                    // semaphore already taken, move to next:
                    up(pSem);
                    continue;
                }
                else
                {
                    g_isSemTaken[i] = 1;
                    // up(pSem);  don't have to do this because of the init...
                    sema_init( pSem, 0 );
                    *semId = (OS_SEMID)pSem;
                    return OS_STATUS_OK;
                }
            }
            
        }
        
    }

    // no free semaphores -- failing:
    *semId = (OS_SEMID)NULL;
    return OS_STATUS_FAIL;
    
}
  
#else // (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
OS_STATUS bcmOsSemCreate(char *semName, OS_SEMID *semId)
{
    int i;
    struct semaphore *pSem = NULL;

    /* Find an unused semaphore structure. */
    for( i = 0, pSem = g_Sems; i < MAX_SEMS; i++, pSem++ )
    {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
        if( (pSem->count.counter & pSem->waking.counter) == ~0 )
#else
        if( (pSem->count.counter) == ~0 )
#endif
        {
            /* An unused semaphore is found.  Initialize it. */
            memset( pSem, 0x00, sizeof(struct semaphore) );
            sema_init( pSem, 0 );
            *semId = (OS_SEMID)pSem;
            break;
        }
    }

	return OS_STATUS_OK;
}
#endif //(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)

/***************************************************************************
 * Function Name: bcmOsSemTakeTo
 * Description  : Timeout handler for bcmOsSemTake
 * Returns      : 0 if successful.
 ***************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
void bcmOsSemTakeTo( unsigned long ulSem )
#else
void bcmOsSemTakeTo( struct timer_list *timer)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
   bcm_sema_timer_list *sema_to_timer;
   OS_SEMID *ulSem;
   sema_to_timer = from_timer(sema_to_timer,timer,to_timer);
   ulSem = (OS_SEMID *)(sema_to_timer->semid);
#endif
    up( (struct semaphore *) ulSem );
} /* XtmOsRequestSemTo */
/*****************************************************************************
** FUNCTION:   bcmOsSemDelete
**
** PURPOSE:    deletes a semaphore
**
** PARAMETERS: semID - semaphore identification
**
** RETURNS:    OS_STATUS_OK - success
**             error code   - failure
**
** NOTE:
******************************************************************************/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
OS_STATUS bcmOsSemDelete( OS_SEMID semId )
{
    int i;

    // convert to index
    i = index_of(g_Sems, (struct semaphore *) semId);
    if (i == -1 || i >= MAX_SEMS)
        return OS_STATUS_FAIL; // bad sem Id;

    sema_init( (struct semaphore *) semId, 1 );
    g_isSemTaken[i] = 0;

    return OS_STATUS_OK;
}
  
#else // (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
OS_STATUS bcmOsSemDelete( OS_SEMID semId )
{
    /* Mark semaphore unused. */
    memset( (char *) semId, 0xff, sizeof(struct semaphore) );
	return OS_STATUS_OK;
}
#endif

/*****************************************************************************
** FUNCTION:   bcmOsSemTake
**
** PURPOSE:    Take a binary semaphore
**
** PARAMETERS: semId - Handle of the semaphore
**             timeout_ticks - timeout in ticks
**
** RETURNS:    OS_STATUS_OK - success
**             error code   - failure
**
** NOTE:       this function ASSERTs that the operation was successful
******************************************************************************/
OS_STATUS bcmOsSemTake(OS_SEMID semId, OS_TIME timeout)
{
    OS_STATUS nRet = OS_STATUS_OK;
    //typedef void  (*FN_TIMER) (unsigned long);

    if( timeout == OS_WAIT_FOREVER ) {
        if (down_interruptible( (struct semaphore *) semId ) != 0) {
            nRet = OS_STATUS_FAIL;
        }
    }
    else {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
        struct timer_list Timer;
        init_timer (&Timer);
        Timer.function = bcmOsSemTakeTo;
        Timer.data = semId;
        Timer.expires = jiffies + timeout;
        add_timer (&Timer);

        if (down_interruptible( (struct semaphore *) semId ) != 0) {
            /* Error on event */
            del_timer_sync( &Timer );
            nRet = OS_STATUS_FAIL;
        }
        else if (!del_timer_sync( &Timer)) {
            /* Timer already timeout */
            nRet = OS_STATUS_TIMEOUT;
        }
#else
        bcm_sema_timer_list Timer;
        timer_setup(&Timer.to_timer, bcmOsSemTakeTo, 0);
        Timer.semid = semId;
        Timer.to_timer.expires = jiffies + timeout;
        add_timer (&Timer.to_timer);

        if (down_interruptible( (struct semaphore *) semId ) != 0) {
            /* Error on event */
            del_timer_sync( &Timer.to_timer );
            nRet = OS_STATUS_FAIL;
        }
        else if (!del_timer_sync( &Timer.to_timer )) {
            /* Timer already timeout */
            nRet = OS_STATUS_TIMEOUT;
        }
#endif
    }
    return (nRet);
}

/*****************************************************************************
** FUNCTION:   bcmOsSemGive
**
** PURPOSE:    Give a semaphore
**
** PARAMETERS: semId - Handle of the semaphore
**
** RETURNS:    OS_STATUS_OK - success
**             error code   - failure
**
** NOTE:
******************************************************************************/
OS_STATUS bcmOsSemGive( OS_SEMID semId )
{
    up( (struct semaphore *) semId );
    return OS_STATUS_OK;
}

/*****************************************************************************
** FUNCTION:   bcmOsGetTime
**
** PURPOSE:    get current system time in ticks.
**
** PARAMETERS: osTime - pointer to OS_TICKS (output)
**
** RETURNS:    OS_STATUS_OK - success
**             error code   - failure
**
** NOTE:       This function is only used to measure relative time for the
**             purposes of timing out syncronous requests
*****************************************************************************/
OS_STATUS bcmOsGetTime(OS_TICKS *osTime)
{
	*osTime = jiffies;
	return OS_STATUS_OK;
}

/***************************************************************************
** FUNCTION:   bcmOsSleep
**
** PURPOSE:    Delays a specified number of jiffies.
**
** PARAMETERS: timeout - number of jiffies to wait
**
** RETURNS:    None.
 ***************************************************************************/
OS_STATUS bcmOsSleep( OS_TIME timeout )
{
    wait_queue_head_t wait;
    init_waitqueue_head(&wait);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
    interruptible_sleep_on_timeout(&wait, timeout);
#else
    wait_event_interruptible_timeout(wait, 1 == 0, timeout); 
#endif
    return OS_STATUS_OK;
} /* bcmOsSleep */

void *calloc( unsigned long num, unsigned long size )
{
	return kmalloc (size*num, GFP_KERNEL);
}

void free( void *memblock )
{
	kfree(memblock);
}

/*****************************************************************************
** FUNCTION:   bcmOsDpcCreate
**
** PURPOSE:    Create a DPC.
**
** PARAMETERS: dpcEntry - the task entry point
**             argument - the task argument
**
** RETURNS:    dpc handle
**             NULL - failure
**
******************************************************************************/
void * bcmOsDpcCreate(void* dpcEntry, void * arg)
{
	int		i = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
    while (NULL != g_Dpc[i].taskQueue.routine)
#else
    while (NULL != g_Dpc[i].task.func)
#endif
	{
		i++;
		if (MAX_DPC == i)
			return NULL;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	INIT_LIST_HEAD(&g_Dpc[i].taskQueue.list);
    g_Dpc[i].taskQueue.sync = 0;
    g_Dpc[i].taskQueue.routine = (void (*)(void *)) dpcEntry;
    g_Dpc[i].taskQueue.data = arg;
	return &g_Dpc[i].taskQueue;
#else
    tasklet_init(&g_Dpc[i].task, dpcEntry, (unsigned long)arg);	
    return &g_Dpc[i].task;
#endif
}

void bcmOsDpcEnqueue(void* dpcHandle)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	queue_task((struct tq_struct *) dpcHandle, &tq_immediate);
	mark_bh(IMMEDIATE_BH);
#else
    tasklet_schedule((struct tasklet_struct *) dpcHandle);	
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
void bcmOsTimerCb(struct timer_list *timer)
{
   g_Dpc_t *pg_Dpc;
   pg_Dpc = (g_Dpc_t *)from_timer(pg_Dpc,timer,timerList);
   if(pg_Dpc != NULL)
   {
      pg_Dpc->timer_cb_func(pg_Dpc->timer_cb_arg);
   }
}
#endif
/*****************************************************************************
** FUNCTION:   bcmOsTimerCreate
**
** PURPOSE:    Create a Timer DPC.
**
** PARAMETERS: timerEntry - the task entry point
**             argument - the task argument
**
** RETURNS:    timer handle
**             NULL - failure
**
******************************************************************************/

void bcmOsTimerStart(void* timerHandle, int toMs)
{
    struct timer_list	*timerList = (struct timer_list	*) timerHandle;

	if (0 == (toMs = (toMs * HZ) / 1000))
		toMs = 1;
	timerList->expires = jiffies + toMs;
	add_timer(timerList);
}

void bcmOsTimerStop(void *timerHandle)
{
	struct timer_list *timerList = (struct timer_list *)timerHandle;
	del_timer_sync(timerList);
}

void * bcmOsTimerCreate(void* timerEntry, void * arg)
{
	int		i = 0;

    while (NULL != g_Dpc[i].timerList.function) {
		i++;
		if (MAX_DPC == i)
			return NULL;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
	init_timer(&g_Dpc[i].timerList);
	g_Dpc[i].timerList.function = timerEntry;
	g_Dpc[i].timerList.data = (unsigned long) arg;
#else
   g_Dpc[i].timer_cb_func = timerEntry;
   g_Dpc[i].timer_cb_arg = (unsigned long)arg;
   timer_setup(&g_Dpc[i].timerList, bcmOsTimerCb, 0);
#endif

	return &g_Dpc[i].timerList;
}

void bcmOsDelay(unsigned long timeMs)
{
	OS_TICKS	tick0, tick1;

	bcmOsGetTime(&tick0);
	do {
		bcmOsGetTime(&tick1);
		tick1 = (tick1 - tick0) * BCMOS_MSEC_PER_TICK;
	} while (tick1 < timeMs);
}

void bcmOsWakeupMonitorTask(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_WAKEUP_MONITOR_TASK,NULL,0);
#else
	kerSysWakeupMonitorTask();
#endif
}

