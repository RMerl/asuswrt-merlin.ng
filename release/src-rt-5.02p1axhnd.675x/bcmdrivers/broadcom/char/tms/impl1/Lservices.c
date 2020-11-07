/* 
 * <:label-3PIP:2019:NONE:standard
 * 
 * :>
 */
/* 
 * 
 * Copyright 1997-2017 NComm, Inc.  All rights reserved.
 * 
 * 
 *                     *** Important Notice ***
 *            This notice may not be removed from this file.
 * 
 *  * valid license agreement between your company and NComm, Inc. The license 
 * agreement includes the definition of a PROJECT.
 * 
 *  * and within the project definition in the license. Any use beyond this 
 * scope is prohibited without executing an additional agreement with 
 * NComm, Inc. Please refer to your license agreement for the definition of 
 * the PROJECT.
 * 
 * This software may be modified for use within the above scope. All 
 * modifications must be clearly marked as non-NComm changes.
 * 
 * If you are in doubt of any of these terms, please contact NComm, Inc. 
 * at sales@ncomm.com. Verification of your company's license agreement 
 * and copies of that agreement also may be obtained from:
 *  
 * NComm, Inc.
 * 81 Main Street  
 * Suite 201
 * Kingston, NH 03848
 * 603-329-5221 
 * sales@ncomm.com
 * 
 */

/*
 * Pull in the necessary kernel header files
 */

#include "linux/version.h"	/* Kernel version information */

#include <generated/autoconf.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 4)
        #include "linux/kconfig.h"
#endif

#include "linux/version.h"	/* Kernel Version Information */
#include "linux/kthread.h"	/* Kernel threads */
#include "linux/sched.h"	/* HZ, jiffies definition, task_struct */
#include "linux/list.h"		/* linked list declaration */
#include "linux/slab.h"		/* kmalloc declaration */

#include "linux/mutex.h"

#include "linux/interrupt.h"	/* IRQ_RETVAL stuff */
#include "linux/errno.h"	/* error codes */
#include "linux/module.h"	/* module-related stuff */
#include "linux/poll.h"		/* gets the user-access macros */

#include <linux/module.h>	/* Module header files */
#include <linux/init.h>
#include <linux/kernel.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0))
#include "linux/sched/signal.h" 
#endif

extern void LservicesTaskSleep(int timeout);

typedef struct ncommWaitQ {
	int active;
	wait_queue_head_t msgQ;
} NCommWaitQ;

/*--------------------------------------------------------------------------*/
/*
 * Define the module entery and exit routines 
 */

static int __init Lservices_init(void)
{
	printk(KERN_ERR "Lservices Loaded - NComm, Inc.\n");

	return 0;
}

static void __exit Lservices_cleanup(void)
{
	printk(KERN_ERR "Lservices Exit - NComm, Inc.\n");
}

/*--------------------------------------------------------------------------*/
/*
 * Define the memory management routines
 */

void *LservicesMalloc(size_t size)
{
void *mem;

	mem = kmalloc(size, GFP_ATOMIC);
	if(mem != NULL) {
		memset(mem, 0, size);
	}
	memset(mem, 0, size);

	return(mem);
}
EXPORT_SYMBOL(LservicesMalloc);

void LservicesFree(void *memptr)
{
        kfree(memptr);
}
EXPORT_SYMBOL(LservicesFree);

/*--------------------------------------------------------------------------*/
/* 
 * Queue/mailbox routines
 */

void LservicesTWrapperEnter(char *name, void *mid)
{
struct mutex *mutexId;


	mutexId = mid;

	name = name;	/* Trash a compiler warning */

	mutex_lock(mutexId);

	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0))
	#else
		daemonize(name);
	#endif


}
EXPORT_SYMBOL(LservicesTWrapperEnter);

void LservicesTWrapperExit(void *mid)
{
struct mutex *mutexId;


	mutexId = mid;

	/* sensitize the thread to these signals
	 */
	allow_signal(SIGKILL);
	allow_signal(SIGINT);
	allow_signal(SIGTERM);

	mutex_unlock(mutexId);

}
EXPORT_SYMBOL(LservicesTWrapperExit);


/*--------------------------------------------------------------------------*/
/*
 * Define the Event management routines
 */
/*--------------------------------------------------------------------------*/
/*
 * Return the process ID
 */
int LservicesGetID(void)
{
int id;


	id = get_current()->pid;


	return(id);
}
EXPORT_SYMBOL(LservicesGetID);

/*
 * Kill a kernel thread
 */

int LservicesKillProcess(void *tid)
{
struct task_struct *taskPtr;
int rc;


	if(tid == NULL) {
		return(0);
	}

	taskPtr = tid;


	rc = (kill_pid(task_pid(taskPtr), SIGKILL, 1) == (-ESRCH)) ? 0 : 1;

	return(rc);
}
EXPORT_SYMBOL(LservicesKillProcess);

/*
 * Lock a Mutex
 */

void *LservicesMutexCreate(void)
{
struct mutex *mutexID;


	mutexID = kmalloc(sizeof(struct mutex), GFP_ATOMIC);
	if(mutexID == NULL) {
		return(NULL);
	}
	memset(mutexID, 0, sizeof(struct mutex));

	mutex_init(mutexID);

	return(mutexID);
}
EXPORT_SYMBOL(LservicesMutexCreate);

void LservicesMutexDelete(void *mid)
{

	kfree(mid);

}
EXPORT_SYMBOL(LservicesMutexDelete);


void LservicesMutexLock(void *mid)
{
struct mutex *mutexId;


	if(mid == NULL) {
		return;
	}

	mutexId = mid;

	mutex_lock(mutexId);

}
EXPORT_SYMBOL(LservicesMutexLock);

/*
 * UnLock a Mutex
 */

void LservicesMutexUNLock(void *mid)
{
struct mutex *mutexId;


	if(mid == NULL) {
		return;
	}

	mutexId = mid;

	mutex_unlock(mutexId);

}
EXPORT_SYMBOL(LservicesMutexUNLock);

int Lserviceskthread_run(void *fooPtr, void *tPtr, void *data, char *name)
{
struct task_struct **taskPtr;
int rc;


	if(tPtr == NULL) {
		return(-1);
	}

	taskPtr = tPtr;

	*taskPtr = kthread_run(fooPtr, data, name);

	rc = ((IS_ERR(*taskPtr)) ? -1 : (*taskPtr)->pid);

	return(rc);
}
EXPORT_SYMBOL(Lserviceskthread_run);

void *LservicesWaitQCreate(void)
{
NCommWaitQ *waitQ;


	waitQ = kmalloc(sizeof(NCommWaitQ), GFP_ATOMIC);
	if(waitQ == NULL) {
		return(NULL);
	}
	memset(waitQ, 0, sizeof(NCommWaitQ));

	init_waitqueue_head(&waitQ->msgQ);
	waitQ->active = 1;

	return(waitQ);
}
EXPORT_SYMBOL(LservicesWaitQCreate);

void LservicesWaitQDelete(void *waitQptr)
{
NCommWaitQ *waitQ;


	waitQ = waitQptr;
	waitQ->active = 0;

	kfree(waitQ);
}
EXPORT_SYMBOL(LservicesWaitQDelete);

/* Sleep routines - timeout is in microseconds */
void LservicesTaskSleep(int timeout)
{
unsigned long tval;

	/* Guarantee we get at least one tic
	 */
	if ((tval = msecs_to_jiffies((timeout/1000))) <= 0) {
		tval = 1;
	}

	set_current_state(TASK_INTERRUPTIBLE);

	schedule_timeout(tval);
}
EXPORT_SYMBOL(LservicesTaskSleep);


/* Timer routines */
void LservicesTimerRestart(void *tptr, unsigned long time)
{
struct timer_list *timer;

	if(tptr == NULL) {
		return;
	}

	timer = tptr;

        mod_timer(timer, time);
}
EXPORT_SYMBOL(LservicesTimerRestart);


int LservicesTimerHook(void *fooptr, 
		unsigned int microseconds,
		void *data, 
		void *timerPtr,
		unsigned long *delta, 
		unsigned long *timeout)
{
struct timer_list *timer, **t;
unsigned long del;


	if((fooptr == NULL) || (delta == NULL) || (timeout == NULL)) {
		return(0);
	}

	if(timerPtr == NULL) {
		return(0);
	}

	timer = kmalloc(sizeof(struct timer_list), GFP_ATOMIC);
	memset(timer, 0, sizeof(struct timer_list));
	if(timer == NULL) {
		return(0);
	}

	/* 
	 * Guarantee we get at least one full Delta.
	 */

	del = msecs_to_jiffies((microseconds/1000));
	if(del <= 0) {
		del = 1;
	}

	*delta = del;

	timer->function = fooptr;
	timer->expires  = jiffies + del;
	timer->data     = (unsigned long)data;

	*timeout = timer->expires;

	t = timerPtr;
	*t = timer;

	init_timer_on_stack(timer);

	add_timer(timer);

	return(1);
}
EXPORT_SYMBOL(LservicesTimerHook);

int LservicesTimerUNHook(void *tid) 
{
struct timer_list *timer = tid;

	del_timer_sync(timer);

	kfree(timer);


	return(1);
}
EXPORT_SYMBOL(LservicesTimerUNHook);

/* Time routines */

unsigned int LservicesGetMSecTics(void)
{

	return((jiffies * 1000) / HZ);
}
EXPORT_SYMBOL(LservicesGetMSecTics);

/* Semaphores */

void *LservicesSemaphoreCreate(void)
{
struct semaphore *sem;


	sem = kmalloc(sizeof(struct semaphore), GFP_ATOMIC);
	if(sem == NULL) {
		return(NULL);
	}
	memset(sem, 0, sizeof(struct semaphore));

	sema_init(sem, 1);

	return(sem);
}
EXPORT_SYMBOL(LservicesSemaphoreCreate);

void LservicesSemaphoreDelete(void *semPtr)
{
struct semaphore *sem = semPtr;


	up(sem);

	kfree(sem);

}
EXPORT_SYMBOL(LservicesSemaphoreDelete);

void LservicesSemaphoreGet(void *semPtr)
{
struct semaphore *sem = semPtr;
DECLARE_COMPLETION_ONSTACK(_taskExited);


	if(semPtr == NULL) {
		return;
	}

	if (down_interruptible(sem)) {

		complete_and_exit(&_taskExited, 0);
	}

}
EXPORT_SYMBOL(LservicesSemaphoreGet);


int LservicesSemaphorePut(void *semPtr)
{
struct semaphore *sem = semPtr;


	if(semPtr == NULL) {
		return(0);
	}

	up(sem);


	return(1);
}
EXPORT_SYMBOL(LservicesSemaphorePut);


/* Locks */

void *LservicesLockCreate(void)
{
spinlock_t *lock;


	lock = kmalloc(sizeof(spinlock_t), GFP_ATOMIC);
	memset(lock, 0, sizeof(spinlock_t));

	if(lock == NULL) {
		return(NULL);
	}

	spin_lock_init(lock);


	return(lock);
}
EXPORT_SYMBOL(LservicesLockCreate);

void LservicesLockDelete(void *lock)
{

	kfree(lock);

}
EXPORT_SYMBOL(LservicesLockDelete);

void LservicesLock(void *lockPtr, void *flagPtr)
{
spinlock_t *lock = lockPtr;
unsigned long flags, *fp;

	if((lockPtr == NULL) || (flagPtr == NULL)) {
		return;
	}

	fp = flagPtr;

	flags = *fp;

	spin_lock_irqsave(lock, flags);

	*fp = flags;
}
EXPORT_SYMBOL(LservicesLock);


void LservicesUNLock(void *lockPtr, void *flagPtr)
{
spinlock_t *lock = lockPtr;
unsigned long flags, *fp;

	if((lockPtr == NULL) || (flagPtr == NULL)) {
		return;
	}

	fp = flagPtr;

	flags = *fp;

	spin_unlock_irqrestore(lock, flags);

	*fp = flags;

	return;
}
EXPORT_SYMBOL(LservicesUNLock);

/* Events */

void *LservicesEventCreate(void)
{
NCommWaitQ *event;


	event = kmalloc(sizeof(NCommWaitQ), GFP_ATOMIC);
	if(event == NULL) {
		return(NULL);
	}
	memset(event, 0, sizeof(NCommWaitQ));

	init_waitqueue_head(&event->msgQ);
	event->active = 1;


	return(event);
}
EXPORT_SYMBOL(LservicesEventCreate);

void LservicesEventDelete(void *event)
{

	kfree(event);
}
EXPORT_SYMBOL(LservicesEventDelete);


int LservicesEventWait(void *evt, void *condPtr)
{
NCommWaitQ *event = evt;


	if((evt == NULL) || (condPtr == NULL)) {
		return(0);
	}

	if(!event->active) {
		return(0);
	}

	if (wait_event_interruptible(event->msgQ, (*(int *)condPtr != 0)) < 0)
		return(0);


	return(1);
}
EXPORT_SYMBOL(LservicesEventWait);


int LservicesEventSend(void *evt)
{
NCommWaitQ *event = evt;


	if(event == NULL) {
		return(0);
	}

	if(!event->active) {
		return(0);
	}

	wake_up_interruptible(&event->msgQ);


	return(1);
}
EXPORT_SYMBOL(LservicesEventSend);

/* Interrupts services - really, just mutual exclusion */

void LservicesIntsOff(void *flagPtr)
{
unsigned long flags, *f;


	f = flagPtr;

	local_irq_save(flags);

	*f = flags;


}
EXPORT_SYMBOL(LservicesIntsOff);

void LservicesIntsOn(void *flagPtr)
{
unsigned long flags, *f;


	f = flagPtr;

	flags = *f;

	local_irq_restore(flags);

}
EXPORT_SYMBOL(LservicesIntsOn);


/*--------------------------------------------------------------------------*/
int LservicesMboxWait(void *evt, void *condPtr)
{
NCommWaitQ *event = evt;
int rc;
DECLARE_COMPLETION_ONSTACK(_taskExited);


	if(event == NULL) {
		return(0);
	}

	if(!event->active) {
		return(0);
	}

	rc = wait_event_interruptible(event->msgQ, ((*((int *)condPtr)) != 0) );

	if(rc) {

		complete_and_exit(&_taskExited, 0);
	}
	

	return(1);
}
EXPORT_SYMBOL(LservicesMboxWait);

/*--------------------------------------------------------------------------*/
int LservicesMboxWaitTimeout(void *evt, void *condPtr, int timeout)
{
NCommWaitQ *event = evt;
int rc;
unsigned long tval;


	if(event == NULL) {
		return(0);
	}

	if(!event->active) {
		return(0);
	}

	/* Guarantee we get at least one tic
	 */
	if ((tval = msecs_to_jiffies((timeout/1000))) <= 0) {
		tval = 1;
	}


	rc = wait_event_interruptible_timeout(event->msgQ, 
			((*((int *)condPtr)) != 0),
			tval );

	return(rc);
}
EXPORT_SYMBOL(LservicesMboxWaitTimeout);


int Lserivces_copy_to_user(void *uPtr, void *kPtr, int sz)
{
	return(copy_to_user(uPtr, kPtr, sz));
}
EXPORT_SYMBOL(Lserivces_copy_to_user);

int Lserivces_copy_from_user(void *uPtr, void *kPtr, int sz)
{
	return(copy_from_user(uPtr, kPtr, sz));
}
EXPORT_SYMBOL(Lserivces_copy_from_user);

module_init(Lservices_init);
module_exit(Lservices_cleanup);

MODULE_LICENSE("Proprietary: Lservices - NComm, Inc. Copyright (C) 2018");
MODULE_AUTHOR("NComm, Inc.");
MODULE_DESCRIPTION("Lservices");

