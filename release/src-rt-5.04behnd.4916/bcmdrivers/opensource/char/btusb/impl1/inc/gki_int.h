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

#ifndef GKI_INT_H
#define GKI_INT_H

#include "gki.h"

/* Task States: (For OSRdyTbl) */
// TASK_DEAD is already defined by Linux => undefine it
#ifdef TASK_DEAD
#undef TASK_DEAD
#endif

#define TASK_DEAD       0   /* b0000 */
#define TASK_READY      1   /* b0001 */
#define TASK_WAIT       2   /* b0010 */
#define TASK_DELAY      4   /* b0100 */
#define TASK_SUSPEND    8   /* b1000 */


/********************************************************************
**  Internal Error codes
*********************************************************************/
#define GKI_ERROR_BUF_CORRUPTED         0xFFFF
#define GKI_ERROR_NOT_BUF_OWNER         0xFFFE
#define GKI_ERROR_FREEBUF_BAD_QID       0xFFFD
#define GKI_ERROR_FREEBUF_BUF_LINKED    0xFFFC
#define GKI_ERROR_SEND_MSG_BAD_DEST     0xFFFB
#define GKI_ERROR_SEND_MSG_BUF_LINKED   0xFFFA
#define GKI_ERROR_ENQUEUE_BUF_LINKED    0xFFF9
#define GKI_ERROR_DELETE_POOL_BAD_QID   0xFFF8
#define GKI_ERROR_BUF_SIZE_TOOBIG       0xFFF7
#define GKI_ERROR_BUF_SIZE_ZERO         0xFFF6
#define GKI_ERROR_ADDR_NOT_IN_BUF       0xFFF5
#define GKI_ERROR_BUF_POOL_CORRUPT      0xFFF4
#define GKI_ERROR_BUFFER_TRACE_ON       0xFFF3
#define GKI_ERROR_SEGS_EXCEEDED         0xFFF2
#define GKI_ERROR_NO_MEMORY_FOR_SEG     0xFFF1


/********************************************************************
**  Buffer Management Data Structures
*********************************************************************/

typedef struct _buffer_hdr
{
    struct _buffer_hdr *p_next;
    UINT8   q_id;
    UINT8   task_id;
    UINT8   status;
    UINT8   Type;
#ifdef TRACE_GKI_BUFFERS
    struct _buffer_hdr *p_next_all;
    char    *pFile;
    int     linenum;
    UINT32  times_alloc;
#endif
} BUFFER_HDR_T;

/* Allocate each buffer pool in up to 4 memory segments, for efficiency
*/
#define MAX_BUFFPOOL_SEGS   4

typedef struct _free_queue
{
    UINT8       *seg_start[MAX_BUFFPOOL_SEGS];
    UINT8       *seg_end[MAX_BUFFPOOL_SEGS];

    BUFFER_HDR_T *p_first;
    BUFFER_HDR_T *p_last;
    UINT16       total;
    UINT16       cur_cnt;
    UINT16       max_cnt;
#ifdef TRACE_GKI_BUFFERS
    BUFFER_HDR_T *p_first_all;
#endif
} FREE_QUEUE_T;


/* Buffer related defines
*/
#define BUFFER_HDR_SIZE     (sizeof(BUFFER_HDR_T))                  /* Offset past header */
#define BUFFER_PADDING_SIZE (sizeof(BUFFER_HDR_T) + sizeof(UINT32)) /* Header + Magic Number */
#define MAX_USER_BUF_SIZE   ((UINT16)0xffff - BUFFER_PADDING_SIZE)  /* pool size must allow for header */
#define MAGIC_NO            0xDDBADDBA

#define BUF_STATUS_FREE     0
#define BUF_STATUS_UNLINKED 1
#define BUF_STATUS_QUEUED   2

/* Exception related structures
*/
#define MAX_EXCEPTION 8
#define MAX_EXCEPTION_MSGLEN 64

typedef struct
{
    UINT16  type;
    UINT8   taskid;
    UINT8   msg[MAX_EXCEPTION_MSGLEN];
} EXCEPTION_T;


/* Put all GKI variables into one control block
*/
typedef struct
{
    /* Task management variables
    */
    /* The stack and stack size are not used on Windows
    */
#if (GKI_USE_DYNAMIC_BUFFERS == FALSE)

#if (GKI_NUM_FIXED_BUF_POOLS > 0)
    UINT8 bufpool0[(GKI_BUF0_SIZE + BUFFER_PADDING_SIZE) * GKI_BUF0_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 1)
    UINT8 bufpool1[(GKI_BUF1_SIZE + BUFFER_PADDING_SIZE) * GKI_BUF1_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 2)
    UINT8 bufpool2[(GKI_BUF2_SIZE + BUFFER_PADDING_SIZE) * GKI_BUF2_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 3)
    UINT8 bufpool3[(GKI_BUF3_SIZE + BUFFER_PADDING_SIZE) * GKI_BUF3_MAX];
#endif

#endif

    INT     IsRunning;

    /* Define the buffer pool management variables
    */
    FREE_QUEUE_T    freeq[GKI_NUM_TOTAL_BUF_POOLS];

    UINT16   pool_buf_size[GKI_NUM_TOTAL_BUF_POOLS];
    UINT16   pool_max_count[GKI_NUM_TOTAL_BUF_POOLS];
    UINT16   pool_additions[GKI_NUM_TOTAL_BUF_POOLS];

    UINT16      ExceptionCnt;
    EXCEPTION_T Exception[MAX_EXCEPTION];

    /* Define the buffer pool access control variables */
    UINT16      pool_access_mask;
    UINT8       pool_list[GKI_NUM_TOTAL_BUF_POOLS]; /* buffer pools arranged in the order of size */
    int         curr_total_no_of_pools;
} tGKI_CB;


void gki_buffer_init(void);
BOOLEAN gki_chk_buf_damage(void *p_buf);
void *gki_reserve_os_memory (UINT32 size);
void gki_release_os_memory (void *p_mem);

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _BT_DYNAMIC_MEMORY
extern tGKI_CB  gki_cb;
#else
extern tGKI_CB *gp_gki_cb;
#define gki_cb (*gp_gki_cb)
#endif


#ifdef __cplusplus
}
#endif

#endif
