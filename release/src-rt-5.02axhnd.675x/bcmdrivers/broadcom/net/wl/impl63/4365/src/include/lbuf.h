/*
 * Linked buffer (lbuf) pool, with static buffer size.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 * $Id: lbuf.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _LBUF_H_
#define _LBUF_H_

#ifdef __cplusplus
extern "C" {
#endif // endif

/* ---- Include Files ---------------------------------------------------- */
#include <osl_ext.h>

/* ---- Constants and Types ---------------------------------------------- */

#define LBUFSZ		2048	/* largest reasonable packet buffer, driver uses for ethernet MTU */

#define	NTXBUF		16	/* # local transmit buffers */
#define	NRXBUF		16	/* # local receive buffers */

/* Forward declaration */
struct lbfree;

/* Linked buffer. */
struct lbuf {
	struct lbuf	*next;			/* pointer to next lbuf if in a chain */
	struct lbuf	*link;			/* pointer to next lbuf if in a list */
	uchar       	*head;			/* start of buffer */
	uchar		*end;			/* end of buffer */
	uchar		*data;			/* start of data */
	uchar		*tail;			/* end of data */
	uint		len;			/* nbytes of data */
	uint		priority;		/* packet priority */
	uint16		flags;
	uchar		pkttag[OSL_PKTTAG_SZ];	/* pkttag area  */
	struct lbfree	*list;			/* lbuf's parent freelist */
	void		*native_pkt;		/* native pkt associated with lbuf. */
};

/* Simple, static list of free lbufs. */
typedef struct lbfree {
	struct lbuf	*free;		/* the linked list */
	uint		total;		/* # total packets */
	uint		count;		/* # packets currently on free */
	uint		headroom;
	uint		size;		/* # bytes packet buffer memory */
	uchar		*dbuf;
	uchar 		*sbuf;
	osl_ext_mutex_t	mutex;
} lbfree_struct;

typedef struct lbuf_info {
	void 		*osh;		/* pointer to OS handle */
	struct lbfree	txfree;		/* tx packet freelist */
	struct lbfree	rxfree;		/* rx packet freelist */
} lbuf_info_t;

/****************************************************************************
* Function:   lbuf_alloc_list
*
* Purpose:    Allocate global pool of linked buffers.
*
* Parameters: info  (mod) Linked buffers info structure.
*             list  (mod) Pointer to list of linked buffers to allocate.
*             total (in)  Number of buffers to allocate.
*
* Returns:    TRUE on success, else FALSE.
*****************************************************************************
*/
bool
lbuf_alloc_list(lbuf_info_t *info, struct lbfree *list, uint total);

/****************************************************************************
* Function:   lbuf_free_list
*
* Purpose:    Free global pool of linked buffers.
*
* Parameters: info (mod) Linked buffers info structure.
*             list (mod) List of linked buffer to free.
*
* Returns:    Nothing.
*****************************************************************************
*/
void
lbuf_free_list(lbuf_info_t *info, struct lbfree *list);

/****************************************************************************
* Function:   lbuf_get
*
* Purpose:    Get a buffer from list of free buffers.
*
* Parameters: list (mod) List to get buffer from.
*
* Returns:    Buffer.
*****************************************************************************
*/
struct lbuf *
lbuf_get(struct lbfree *list);

/****************************************************************************
* Function:   lbuf_put
*
* Purpose:    Return buffer to linked list of free buffers.
*
* Parameters: list (mod) List to return buffer to.
*             lb   (mod) Buffer to return.
*
* Returns:    Nothing.
*****************************************************************************
*/
void
lbuf_put(struct lbfree *list, struct lbuf *lb);

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#ifdef __cplusplus
	}
#endif // endif

#endif  /* _LBUF_H_  */
