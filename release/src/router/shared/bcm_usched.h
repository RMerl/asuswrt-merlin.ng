/*
 * Broadcom micro scheduler library include file
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
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
 * $Id: bcm_usched.h 623877 2016-03-09 13:21:51Z $
 */

#ifndef __BCM_USCHED_H__
#define __BCM_USCHED_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BCM_USCHED_VERSION	1

/* Define Error Codes */
#define BCM_USCHEDE_OK			0	/* Successfull */
#define BCM_USCHEDE_FAIL		-1	/* Failed */
#define BCM_USCHEDE_INV_HDL		-2	/* Invalid Handle of the scheduler */
#define BCM_USCHEDE_MEMORY		-3	/* Memory allocation failure */
#define BCM_USCHEDE_SCHEDULER_EMPTY	-4	/* No FD's and Timers are added */
#define BCM_USCHEDE_NO_TIMERS		-5	/* No timers are added */
#define BCM_USCHEDE_NO_FDS		-6	/* No FD's added */
#define BCM_USCHEDE_TIMER_EXISTS	-7	/* Timer already exists */
#define BCM_USCHEDE_FD_EXISTS		-8	/* FD already exists */
#define BCM_USCHEDE_NOT_FOUND		-9	/* FD or Timer not found */

/* Define different status of the scheduler */
#define BCM_USCHED_INITIALIZED		1	/* Scheduling is initialized */
#define BCM_USCHED_RUNNING		2	/* Scheduling is running */
#define BCM_USCHED_STOPPED		3	/* Scheduling is stopped */

typedef int BCM_USCHED_STATUS;

typedef void bcm_usched_handle;

#define BCM_USCHED_MASK_READFD		0x0001 /* Check for READ in the FD */
#define BCM_USCHED_MASK_WRITEFD		0x0002 /* Check for WRITE in the FD */
#define BCM_USCHED_MASK_EXCEPTIONFD	0x0004 /* Check for Exception in the FD */

/* Use this macro to set BCM_USCHED_MASK_READFD, BCM_USCHED_MASK_WRITEFD,
 * BCM_USCHED_MASK_EXCEPTIONFD bits to check while adding the FD using bcm_usched_add_fd_schedule()
 * call
 */
#define BCM_USCHED_SETFDMASK(x, bit)	((x) |= (bit))
#define BCM_USCHED_ISFDMASKSET(x, bit)	((x) & (bit))

/* structure for holding the fd's information and send it in callback to user */
typedef struct bcm_usched_fds_entry {
	/* FD which is set */
	int fd;
	/* READ, WRITE or EXCEPTION FD's set. This will be either BCM_USCHED_MASK_READFD,
	 * BCM_USCHED_MASK_WRITEFD or BCM_USCHED_MASK_EXCEPTIONFD
	 */
	int fdbits;
} bcm_usched_fds_entry_t;

/**
 * Callback function for timers
 *
 * @param handle	handle to the library
 * @param arg		Argument passed while adding timer for scheduling
 */
typedef void bcm_usched_timerscbfn(bcm_usched_handle *handle, void *arg);

/**
 * Callback function for File descriptor scheduling
 *
 * @param handle	handle to the library
 * @param arg		Argument passed while adding FD for scheduling
 * @param entry		File descriptor informations in a structure
 */
typedef void bcm_usched_fdscbfn(bcm_usched_handle *handle, void *arg,
	bcm_usched_fds_entry_t *entry);

/**
 * Initialize the scheduler. This initializes the memory for the handle and this
 * handle should be used in all the functions
 *
 * @return	handle to the library
 */
bcm_usched_handle* bcm_usched_init();

/**
 * DeInitialize the scheduler. Call this only after the bcm_usched_run() function returns
 * or after calling bcm_usched_stop()
 *
 * @param handle	Handle to the library which is returned while initializing
 *
 * @return		status of the call
 */
BCM_USCHED_STATUS bcm_usched_deinit(bcm_usched_handle *handle);

/**
 * Add an timer. Dont add two timers with same callback and same optional argument. Either one
 * should be different, callback or optional argument
 *
 * @param handle	Handle to the library which is returned while initializing
 * @param interval	interval of the timer in microseconds
 * @param repeat_flag	Whether the timer should repeat
 * @param cbfn		users call back function
 * @param arg		Optional argument to be passed in users callback function when it is called
 *
 * @return		status of the call
 */
BCM_USCHED_STATUS bcm_usched_add_timer(bcm_usched_handle *handle, unsigned long interval,
	short int repeat_flag, bcm_usched_timerscbfn *cbfn, void *arg);

/**
 * Remove an timer from the list of timers provided the timer ID.
 * This can be called from any callback.
 *
 * @param handle	Handle to the library which is returned while initializing
 * @param cbfn		Callback function used while adding timer
 * @param arg		Same arg used while adding timer. If passed NULL while adding, pass NULL
 *			here too
 *
 * @return		status of the call
 */
BCM_USCHED_STATUS bcm_usched_remove_timer(bcm_usched_handle *handle, bcm_usched_timerscbfn *cbfn,
	void *arg);

/**
 * Add an fd scheduler. Dont add two FD's with same callback and optional argument. Either one
 * should be different, callback or optional argument
 *
 * @param handle	Handle to the library which is returned while initializing
 * @param fd		File descriptor to be checked
 * @param fdmask	This can be BCM_USCHED_MASK_READFD, BCM_USCHED_MASK_WRITEFD,
 *			BCM_USCHED_MASK_EXCEPTIONFD. If you want to monitor all pass
 *			bitwise OR of all.
 * @param cbfn		users call back function
 * @param arg		Optional argument to be passed in users callback function when it is called
 *
 * @return		status of the call
 */
BCM_USCHED_STATUS bcm_usched_add_fd_schedule(bcm_usched_handle *handle, int fd, int fdbits,
	bcm_usched_fdscbfn *cbfn, void *arg);

/**
 * Remove scheduling provided the schedule ID. This can be called from any callback.
 *
 * @param handle	Handle to the library which is returned while initializing
 * @param fd		FD to be removed
 *
 * @return		status of the call
 */
BCM_USCHED_STATUS bcm_usched_remove_fd_schedule(bcm_usched_handle *handle, int fd);

/**
 * Run the scheduler, This will return only if the user calls bcm_usched_stop function or if
 * there is no timer or fd entries added. The user can call bcm_usched_stop function from any
 * callback which gets called from the scheduler.
 *
 * @param handle	Handle to the library which is returned while initializing
 *
 * @return		status of the call
 */
BCM_USCHED_STATUS bcm_usched_run(bcm_usched_handle *handle);

/**
 * Stop the scheduler. This can be called from any callback
 *
 * @param handle	Handle to the library which is returned while initializing
 *
 * @return		status of the call
 */
BCM_USCHED_STATUS bcm_usched_stop(bcm_usched_handle *handle);

/**
 * Status of the Scheduler
 *
 * @param handle	Handle to the library which is returned while initializing
 *
 * @return		status of the function (Initialized, running or stopped)
 */
BCM_USCHED_STATUS bcm_usched_get_status(bcm_usched_handle *handle);

/**
 * Return error string for the error code
 *
 * @param errorcode	Error code
 *
 * @return		Error string corresponding to the error code
 */
const char* bcm_usched_strerror(BCM_USCHED_STATUS errorcode);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BCM_USCHED_H__ */
