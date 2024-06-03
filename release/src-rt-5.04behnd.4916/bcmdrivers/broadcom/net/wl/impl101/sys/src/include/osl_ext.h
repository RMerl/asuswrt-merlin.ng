/*
 * OS Abstraction Layer Extension - the APIs defined by the "extension" API
 * are only supported by a subset of all operating systems.
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: osl_ext.h 821234 2023-02-06 14:16:52Z $
 */

#ifndef _osl_ext_h_
#define _osl_ext_h_

/* ---- Include Files ---------------------------------------------------- */

#ifdef THREADX
#include <threadx_osl_ext.h>
#else
#define OSL_EXT_DISABLED
#endif /* THREADX */

/* Include base operating system abstraction. */
#include <osl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Constants and Types ---------------------------------------------- */

/* -----------------------------------------------------------------------
 * Generic OS types.
 */
typedef enum osl_ext_status_t
{
	OSL_EXT_SUCCESS,
	OSL_EXT_ERROR,
	OSL_EXT_TIMEOUT

} osl_ext_status_t;
#define OSL_EXT_STATUS_DECL(status)	osl_ext_status_t status;

#define OSL_EXT_TIME_FOREVER ((time_ms_t)(-1))

typedef time_ms_t	osl_ext_time_ms_t;
typedef time_us_t	osl_ext_time_us_t;
typedef time64_us_t	osl_ext_time64_us_t;

/* -----------------------------------------------------------------------
 * Tasks.
 */

/* Task entry argument. */
typedef void* osl_ext_task_arg_t;

/* Task entry function. */
typedef void (*osl_ext_task_entry)(osl_ext_task_arg_t arg);

/* Abstract task priority levels. */
typedef enum
{
	OSL_EXT_TASK_IDLE_PRIORITY,
	OSL_EXT_TASK_CPUUTIL_PRIORITY,
	OSL_EXT_TASK_LOW_PRIORITY,
	OSL_EXT_TASK_LOW_NORMAL_PRIORITY,
	OSL_EXT_TASK_NORMAL_PRIORITY,
	OSL_EXT_TASK_HIGH_NORMAL_PRIORITY,
	OSL_EXT_TASK_HIGHEST_PRIORITY,
	OSL_EXT_TASK_TIME_CRITICAL_PRIORITY,

	/* This must be last. */
	OSL_EXT_TASK_NUM_PRIORITES
} osl_ext_task_priority_t;

#ifndef OSL_EXT_DISABLED

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

/* --------------------------------------------------------------------------
** Semaphore
*/

/****************************************************************************
* Function:   osl_ext_sem_create
*
* Purpose:    Creates a counting semaphore object, which can subsequently be
*             used for thread notification.
*
* Parameters: name     (in)  Name to assign to the semaphore (must be unique).
*             init_cnt (in)  Initial count that the semaphore should have.
*             sem      (out) Newly created semaphore.
*
* Returns:    OSL_EXT_SUCCESS if the semaphore was created successfully, or an
*             error code if the semaphore could not be created.
*****************************************************************************
*/
osl_ext_status_t osl_ext_sem_create(char *name, int init_cnt, osl_ext_sem_t *sem);

/****************************************************************************
* Function:   osl_ext_sem_delete
*
* Purpose:    Destroys a previously created semaphore object.
*
* Parameters: sem (mod) Semaphore object to destroy.
*
* Returns:    OSL_EXT_SUCCESS if the semaphore was deleted successfully, or an
*             error code if the semaphore could not be created.
*****************************************************************************
*/
osl_ext_status_t osl_ext_sem_delete(osl_ext_sem_t *sem);

/****************************************************************************
* Function:   osl_ext_sem_give
*
* Purpose:    Increments the count associated with the semaphore. This will
*             cause one thread blocked on a take to wake up.
*
* Parameters: sem (mod) Semaphore object to give.
*
* Returns:    OSL_EXT_SUCCESS if the semaphore was given successfully, or an
*             error code if the semaphore could not be created.
*****************************************************************************
*/
osl_ext_status_t osl_ext_sem_give(osl_ext_sem_t *sem);

/****************************************************************************
* Function:   osl_ext_sem_take
*
* Purpose:    Decrements the count associated with the semaphore. If the count
*             is less than zero, then the calling task will become blocked until
*             another thread does a give on the semaphore. This function will only
*             block the calling thread for timeout_msec milliseconds, before
*             returning with OSL_EXT_TIMEOUT.
*
* Parameters: sem          (mod) Semaphore object to take.
*             timeout_msec (in)  Number of milliseconds to wait for the
*                                semaphore to enter a state where it can be
*                                taken.
*
* Returns:    OSL_EXT_SUCCESS if the semaphore was taken successfully, or an
*             error code if the semaphore could not be created.
*****************************************************************************
*/
osl_ext_status_t osl_ext_sem_take(osl_ext_sem_t *sem, osl_ext_time_ms_t timeout_msec);

/* --------------------------------------------------------------------------
** Mutex
*/

/****************************************************************************
* Function:   osl_ext_mutex_create
*
* Purpose:    Creates a mutex object, which can subsequently be used to control
*             mutually exclusion of resources.
*
* Parameters: name  (in)  Name to assign to the mutex (must be unique).
*             mutex (out) Mutex object to initialize.
*
* Returns:    OSL_EXT_SUCCESS if the mutex was created successfully, or an
*             error code if the mutex could not be created.
*****************************************************************************
*/
osl_ext_status_t osl_ext_mutex_create(char *name, osl_ext_mutex_t *mutex);

/****************************************************************************
* Function:   osl_ext_mutex_delete
*
* Purpose:    Destroys a previously created mutex object.
*
* Parameters: mutex (mod) Mutex object to destroy.
*
* Returns:    OSL_EXT_SUCCESS if the mutex was deleted successfully, or an
*             error code if the mutex could not be created.
*****************************************************************************
*/
osl_ext_status_t osl_ext_mutex_delete(osl_ext_mutex_t *mutex);

/****************************************************************************
* Function:   osl_ext_mutex_acquire
*
* Purpose:    Acquires the indicated mutual exclusion object. If the object is
*             currently acquired by another task, then this function will wait
*             for timeout_msec milli-seconds before returning with OSL_EXT_TIMEOUT.
*
* Parameters: mutex        (mod) Mutex object to acquire.
*             timeout_msec (in)  Number of milliseconds to wait for the mutex.
*
* Returns:    OSL_EXT_SUCCESS if the mutex was acquired successfully, or an
*             error code if the mutex could not be created.
*****************************************************************************
*/
osl_ext_status_t osl_ext_mutex_acquire(osl_ext_mutex_t *mutex, osl_ext_time_ms_t timeout_msec);

/****************************************************************************
* Function:   osl_ext_mutex_release
*
* Purpose:    Releases the indicated mutual exclusion object. This makes it
*             available for another task to acquire.
*
* Parameters: mutex (mod) Mutex object to release.
*
* Returns:    OSL_EXT_SUCCESS if the mutex was released successfully, or an
*             error code if the mutex could not be created.
*****************************************************************************
*/
osl_ext_status_t osl_ext_mutex_release(osl_ext_mutex_t *mutex);

/* --------------------------------------------------------------------------
** Tasks
*/

/****************************************************************************
* Function:   osl_ext_task_create
*
* Purpose:    Create a task.
*
* Parameters: name       (in)  Pointer to task string descriptor.
*             stack      (in)  Pointer to stack. NULL to allocate.
*             stack_size (in)  Stack size - in bytes.
*             priority   (in)  Abstract task priority.
*             func       (in)  A pointer to the task entry point function.
*             arg        (in)  Value passed into task entry point function.
*             task       (out) Task to create.
*
* Returns:    OSL_EXT_SUCCESS if the task was created successfully, or an
*             error code if the task could not be created.
*****************************************************************************
*/

#define osl_ext_task_create(name, stack, stack_size, priority, func, arg, task) \
	   osl_ext_task_create_ex((name), (stack), (stack_size), (priority), 0, (func), \
	   (arg), TRUE, (task))

/****************************************************************************
* Function:   osl_ext_task_create_ex
*
* Purpose:    Create a task with autostart option.
*
* Parameters: name       (in)  Pointer to task string descriptor.
*             stack      (in)  Pointer to stack. NULL to allocate.
*             stack_size (in)  Stack size - in bytes.
*             priority   (in)  Abstract task priority.
*             func       (in)  A pointer to the task entry point function.
*             arg        (in)  Value passed into task entry point function.
*             autostart  (in)  TRUE to start task after creation.
*             task       (out) Task to create.
*
* Returns:    OSL_EXT_SUCCESS if the task was created successfully, or an
*             error code if the task could not be created.
*****************************************************************************
*/

osl_ext_status_t osl_ext_task_create_ex(char* name,
	void *stack, unsigned int stack_size, osl_ext_task_priority_t priority,
	osl_ext_time_ms_t timslice_msec, osl_ext_task_entry func, osl_ext_task_arg_t arg,
	bool autostart, osl_ext_task_t *task);

/****************************************************************************
* Function:   osl_ext_task_delete
*
* Purpose:    Destroy a task.
*
* Parameters: task (mod) Task to destroy.
*
* Returns:    OSL_EXT_SUCCESS if the task was created successfully, or an
*             error code if the task could not be created.
*****************************************************************************
*/
osl_ext_status_t osl_ext_task_delete(osl_ext_task_t *task);

/****************************************************************************
* Function:   osl_ext_task_is_running
*
* Purpose:    Returns current running task.
*
* Parameters: None.
*
* Returns:    osl_ext_task_t of current running task.
*****************************************************************************
*/
osl_ext_task_t *osl_ext_task_current(void);

/****************************************************************************
* Function:   osl_ext_task_yield
*
* Purpose:    Yield the CPU to other tasks of the same priority that are
*             ready-to-run.
*
* Parameters: None.
*
* Returns:    OSL_EXT_SUCCESS if successful, else error code.
*****************************************************************************
*/
osl_ext_status_t osl_ext_task_yield(void);

/****************************************************************************
* Function:   osl_ext_task_yield
*
* Purpose:    Yield the CPU to other tasks of the same priority that are
*             ready-to-run.
*
* Parameters: None.
*
* Returns:    OSL_EXT_SUCCESS if successful, else error code.
*****************************************************************************
*/
osl_ext_status_t osl_ext_task_yield(void);

/****************************************************************************
* Function:   osl_ext_task_suspend
*
* Purpose:    Suspend a task.
*
* Parameters: task (mod) Task to suspend.
*
* Returns:    OSL_EXT_SUCCESS if the task was suspended successfully, or an
*             error code if the task could not be suspended.
*****************************************************************************
*/
osl_ext_status_t osl_ext_task_suspend(osl_ext_task_t *task);

/****************************************************************************
* Function:   osl_ext_task_resume
*
* Purpose:    Resume a task.
*
* Parameters: task (mod) Task to resume.
*
* Returns:    OSL_EXT_SUCCESS if the task was resumed successfully, or an
*             error code if the task could not be resumed.
*****************************************************************************
*/
osl_ext_status_t osl_ext_task_resume(osl_ext_task_t *task);

/****************************************************************************
* Function:   osl_ext_task_enable_stack_check
*
* Purpose:    Enable task stack checking.
*
* Parameters: None.
*
* Returns:    OSL_EXT_SUCCESS if successful, else error code.
*****************************************************************************
*/
osl_ext_status_t osl_ext_task_enable_stack_check(void);

/****************************************************************************
* Function:   osl_ext_task_priority_change
*
* Purpose:    Change task priority.
*
* Parameters: task       (mod)   Task to change.
*             new_priority (in)  New priority.
*             old_priority (out) Pointer to a location to return the task's
*                                previous priority, or NULL.
*
* Returns:    OSL_EXT_SUCCESS if the task priority was changed successfully,
*             or an error code if the task priority could not be changed.
*****************************************************************************
*/
osl_ext_status_t osl_ext_task_priority_change(osl_ext_task_t *task,
	osl_ext_task_priority_t new_priority,
	osl_ext_task_priority_t *old_priority);

/* --------------------------------------------------------------------------
** Queue
*/

/****************************************************************************
* Function:   osl_ext_queue_create
*
* Purpose:    Create a queue.
*
* Parameters: name     (in)  Name to assign to the queue (must be unique).
*             buffer   (in)  Queue buffer. NULL to allocate.
*             size     (in)  Size of the queue.
*             queue    (out) Newly created queue.
*
* Returns:    OSL_EXT_SUCCESS if the queue was created successfully, or an
*             error code if the queue could not be created.
*****************************************************************************
*/
osl_ext_status_t osl_ext_queue_create(char *name,
	void *queue_buffer, unsigned int queue_size,
	osl_ext_queue_t *queue);

/****************************************************************************
* Function:   osl_ext_queue_delete
*
* Purpose:    Destroys a previously created queue object.
*
* Parameters: queue    (mod) Queue object to destroy.
*
* Returns:    OSL_EXT_SUCCESS if the queue was deleted successfully, or an
*             error code if the queue could not be deleteed.
*****************************************************************************
*/
osl_ext_status_t osl_ext_queue_delete(osl_ext_queue_t *queue);

/****************************************************************************
* Function:   osl_ext_queue_send
*
* Purpose:    Send/add data to the queue. This function will not block the
*             calling thread if the queue is full.
*
* Parameters: queue    (mod) Queue object.
*             data     (in)  Data pointer to be queued.
*
* Returns:    OSL_EXT_SUCCESS if the data was queued successfully, or an
*             error code if the data could not be queued.
*****************************************************************************
*/
osl_ext_status_t osl_ext_queue_send(osl_ext_queue_t *queue, void *data);

/****************************************************************************
* Function:   osl_ext_queue_send_synchronous
*
* Purpose:    Send/add data to the queue. This function will block the
*             calling thread until the data is dequeued.
*
* Parameters: queue    (mod) Queue object.
*             data     (in)  Data pointer to be queued.
*
* Returns:    OSL_EXT_SUCCESS if the data was queued successfully, or an
*             error code if the data could not be queued.
*****************************************************************************
*/
osl_ext_status_t osl_ext_queue_send_synchronous(osl_ext_queue_t *queue, void *data);

/****************************************************************************
* Function:   osl_ext_queue_receive
*
* Purpose:    Receive/remove data from the queue. This function will only
*             block the calling thread for timeout_msec milliseconds, before
*             returning with OSL_EXT_TIMEOUT.
*
* Parameters: queue        (mod) Queue object.
*             timeout_msec (in)  Number of milliseconds to wait for the
*                                data from the queue.
*             data         (out) Data pointer received/removed from the queue.
*
* Returns:    OSL_EXT_SUCCESS if the data was dequeued successfully, or an
*             error code if the data could not be dequeued.
*****************************************************************************
*/
osl_ext_status_t osl_ext_queue_receive(osl_ext_queue_t *queue,
                 osl_ext_time_ms_t timeout_msec, void **data);

/****************************************************************************
* Function:   osl_ext_queue_count
*
* Purpose:    Returns the number of items in the queue.
*
* Parameters: queue        (mod) Queue object.
*             count        (out) Data pointer received/removed from the queue.
*
* Returns:    OSL_EXT_SUCCESS if the count was returned successfully, or an
*             error code if the count is invalid.
*****************************************************************************
*/
osl_ext_status_t osl_ext_queue_count(osl_ext_queue_t *queue, int *count);

/* Locking */
osl_ext_status_t osl_ext_lock_create(osl_ext_lock_t *lock, char *name);
osl_ext_status_t osl_ext_lock_delete(osl_ext_lock_t *lock);
osl_ext_status_t osl_ext_lock_acquire(osl_ext_lock_t *lock);
osl_ext_status_t osl_ext_lock_release(osl_ext_lock_t *lock);

#else

/* ---- Constants and Types ---------------------------------------------- */

/* Semaphore. */
#define osl_ext_sem_t
#define OSL_EXT_SEM_DECL(sem)

/* Mutex. */
#define osl_ext_mutex_t
#define OSL_EXT_MUTEX_DECL(mutex)

/* Task. */
//#define osl_ext_task_t void
#define OSL_EXT_TASK_DECL(task)

/* Queue. */
#define osl_ext_queue_t
#define OSL_EXT_QUEUE_DECL(queue)

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#define osl_ext_sem_create(name, init_cnt, sem)		(OSL_EXT_SUCCESS)
#define osl_ext_sem_delete(sem)				(OSL_EXT_SUCCESS)
#define osl_ext_sem_give(sem)				(OSL_EXT_SUCCESS)
#define osl_ext_sem_take(sem, timeout_msec)		(OSL_EXT_SUCCESS)

#define osl_ext_mutex_create(name, mutex)		(OSL_EXT_SUCCESS)
#define osl_ext_mutex_delete(mutex)			(OSL_EXT_SUCCESS)
#define osl_ext_mutex_acquire(mutex, timeout_msec)	(OSL_EXT_SUCCESS)
#define osl_ext_mutex_release(mutex)			(OSL_EXT_SUCCESS)

#define osl_ext_task_create(name, stack, stack_size, priority, func, arg, task) \
	(OSL_EXT_SUCCESS)
#define osl_ext_task_delete(task)			(OSL_EXT_SUCCESS)
#define osl_ext_task_current()				(NULL)
#define osl_ext_task_yield()				(OSL_EXT_SUCCESS)
#define osl_ext_task_enable_stack_check()		(OSL_EXT_SUCCESS)
#define osl_ext_task_priority_change(task, new, old)	(OSL_EXT_SUCCESS)

#define osl_ext_queue_create(name, queue_buffer, queue_size, queue) \
	(OSL_EXT_SUCCESS)
#define osl_ext_queue_delete(queue)			(OSL_EXT_SUCCESS)
#define osl_ext_queue_send(queue, data)			(OSL_EXT_SUCCESS)
#define osl_ext_queue_send_synchronous(queue, data)	(OSL_EXT_SUCCESS)
#define osl_ext_queue_receive(queue, timeout_msec, data) \
	(OSL_EXT_SUCCESS)
#define osl_ext_queue_count(queue, count)		(OSL_EXT_SUCCESS)

#endif	/* OSL_EXT_DISABLED */

#ifdef __cplusplus
}
#endif

#endif	/* _osl_ext_h_ */
