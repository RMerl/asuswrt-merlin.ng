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

#ifndef GKI_H
#define GKI_H

#include "target.h"
#include "data_types.h"

//#define TRACE_GKI_BUFFERS

/* Error codes */
#define GKI_SUCCESS         0x00
#define GKI_FAILURE         0x01
#define GKI_INVALID_TASK    0xF0
#define GKI_INVALID_POOL    0xFF



/************************************************************************
** Mailbox definitions. Each task has 4 mailboxes that are used to
** send buffers to the task.
*/
#define TASK_MBOX_0    0
#define TASK_MBOX_1    1
#define TASK_MBOX_2    2
#define TASK_MBOX_3    3

#define NUM_TASK_MBOX  4

/************************************************************************
** Event definitions.
**
** There are 4 reserved events used to signal messages rcvd in task mailboxes.
** There are 4 reserved events used to signal timeout events.
** There are 8 general purpose events available for applications.
*/
#define MAX_EVENTS             16

#define TASK_MBOX_0_EVT_MASK   0x0001
#define TASK_MBOX_1_EVT_MASK   0x0002
#define TASK_MBOX_2_EVT_MASK   0x0004
#define TASK_MBOX_3_EVT_MASK   0x0008


#define TIMER_0             0
#define TIMER_1             1
#define TIMER_2             2
#define TIMER_3             3

#define TIMER_0_EVT_MASK    0x0010
#define TIMER_1_EVT_MASK    0x0020
#define TIMER_2_EVT_MASK    0x0040
#define TIMER_3_EVT_MASK    0x0080

#define APPL_EVT_0          8
#define APPL_EVT_1          9
#define APPL_EVT_2          10
#define APPL_EVT_3          11
#define APPL_EVT_4          12
#define APPL_EVT_5          13
#define APPL_EVT_6          14
#define APPL_EVT_7          15

#define EVENT_MASK(evt)    ((UINT16)(0x0001 << (evt)))

/***********************************************************************
** This queue is a general purpose buffer queue, for application use.
*/
typedef struct 
{
    void    *p_first;
    void    *p_last;
    UINT16  count;
} BUFFER_Q;

#define GKI_IS_QUEUE_EMPTY(p_q) ((p_q)->count == 0)

/* Task constants
*/
#ifndef TASKPTR_DEF
#define TASKPTR_DEF
#if defined (LINUX_KERNEL)
typedef int (*TASKPTR)(UINT32);
#else
typedef void (*TASKPTR)(UINT32);
#endif
#endif

#define GKI_PUBLIC_POOL         0       /* General pool accessible to GKI_getbuf() */
#define GKI_RESTRICTED_POOL     1       /* Inaccessible pool to GKI_getbuf() */

/***********************************************************************
** Function prototypes
*/

#ifdef __cplusplus
extern "C" {
#endif

/* Task management */
BT_API extern void    GKI_init(void);
BT_API extern void    GKI_shutdown(void);

/* To get and release buffers, change owner and get size
*/
BT_API extern void    GKI_change_buf_owner (void *, UINT8);
BT_API extern UINT8   GKI_create_pool (UINT16, UINT16, UINT8, void *);
BT_API extern void    GKI_delete_pool (UINT8);
BT_API extern void   *GKI_find_buf_start (void *);
BT_API extern void    GKI_freebuf (void *);
#ifdef TRACE_GKI_BUFFERS
BT_API extern void   *GKI_getbuf_trace (UINT16, char *, int);
#define GKI_getbuf(_size) GKI_getbuf_trace(_size,__FILE__,__LINE__)
#else
BT_API extern void   *GKI_getbuf (UINT16);
#endif

BT_API extern UINT16  GKI_get_buf_size (void *);
BT_API extern UINT16  GKI_get_pool_bufsize (UINT8 pool_id);

// For compatibility with BTE
#define GKI_POOL_SIZE(x) GKI_get_pool_bufsize(x)


#ifdef TRACE_GKI_BUFFERS
BT_API extern void   *GKI_getpoolbuf_trace (UINT8, char *, int);
#define GKI_getpoolbuf(_pool) GKI_getpoolbuf_trace(_pool,__FILE__,__LINE__)
#else
BT_API extern void   *GKI_getpoolbuf (UINT8);
#endif

BT_API extern UINT16  GKI_poolfreecount (UINT8);
BT_API extern UINT16  GKI_poolutilization (UINT8);
BT_API extern UINT8   GKI_set_pool_permission(UINT8, UINT8);


/* User buffer queue management
*/
BT_API extern void   *GKI_dequeue  (BUFFER_Q *);
BT_API extern UINT8   GKI_buffer_status(void *p_buf);
BT_API extern void    GKI_enqueue (BUFFER_Q *, void *);
BT_API extern void    GKI_enqueue_head (BUFFER_Q *, void *);
BT_API extern void   *GKI_getfirst (BUFFER_Q *);
BT_API extern void   *GKI_getnext (void *);
BT_API extern void    GKI_init_q (BUFFER_Q *);
BT_API extern BOOLEAN GKI_queue_is_empty(BUFFER_Q *);
BT_API extern void   *GKI_remove_from_queue (BUFFER_Q *, void *);

BT_API extern BOOLEAN GKI_chk_buf_pool_damage(UINT8 pool_id);

/* Disable Interrupts, Enable Interrupts
*/
BT_API extern void    GKI_enable(void);
BT_API extern void    GKI_disable(void);

/* Exception handling
*/
BT_API extern void    GKI_exception (UINT16, const char *msg, ...);

#ifdef __cplusplus
}
#endif


#endif

