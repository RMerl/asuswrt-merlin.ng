/*
 * Sequence commands.
 *
 * Command sequencing is a mechanism whereby multiple IOCTL commands, with
 * possible delays inserted, are sent to the driver. The driver doesn't
 * actually run the IOCTLs until it has received the entire list.
 *
 * The start of a sequence is indicated by the seq_start IOVAR, and the end
 * of the sequence by the seq_stop IOVAR. The driver queues all IOCTLs/IOVARs
 * it receives until the seq_stop is received.
 *
 * When a seq_stop is received:
 *		-	The firmware will begin at the head of the queue and issue ioctls
 *			to the wlc code sequentially.
 * 	- 	If a seq_delay is encountered in the queue, it will spin for the
 *			indicated amount of time, at which point the next ioctl will be
 *			fetched and passed to the wlc code
 * 	- 	As each ioctl is completed, the associated list element that was
 *			malloc'd will be free'd.
 * 	- 	If any of the individual batch commands fail, the sequence will
 *			be aborted, the remaining queued ioctls will be free'd and an
 *			error indication will be passed back for the entire sequence.
 * 	- 	If all batched commands are processed successfully, then a success
 *			indication will be returned and the driver will return to normal
 *			IOCTL processing mode.
 *
 *	In order to reduce the number of transactions, the seq_list may be used
 * as a container IOVAR. It is used to concatenate a series of IOCTLs/IOVARs
 * into a single contiguous buffer and pass the entire sequence to the driver
 * using a single transaction.
 *
 * Because of the nature of the batching, it is suited towards a series of
 * set commands. IOCTLs/IOVARS that query information are not supported.
 *
 *   Copyright 2020 Broadcom
 *
 *   This program is the proprietary software of Broadcom and/or
 *   its licensors, and may only be used, duplicated, modified or distributed
 *   pursuant to the terms and conditions of a separate, written license
 *   agreement executed between you and Broadcom (an "Authorized License").
 *   Except as set forth in an Authorized License, Broadcom grants no license
 *   (express or implied), right to use, or waiver of any kind with respect to
 *   the Software, and Broadcom expressly reserves all rights in and to the
 *   Software and all intellectual property rights therein.  IF YOU HAVE NO
 *   AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 *   WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 *   THE SOFTWARE.
 *
 *   Except as expressly set forth in the Authorized License,
 *
 *   1. This program, including its structure, sequence and organization,
 *   constitutes the valuable trade secrets of Broadcom, and you shall use
 *   all reasonable efforts to protect the confidentiality thereof, and to
 *   use this information only in connection with your use of Broadcom
 *   integrated circuit products.
 *
 *   2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *   "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 *   REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 *   OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 *   DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 *   NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 *   ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 *   CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 *   OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *   3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 *   BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 *   SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 *   IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *   IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 *   ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 *   OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 *   NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *   $Id: wlc_seq_cmds.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_seq_cmds_h_
#define _wlc_seq_cmds_h_

/* ---- Include Files ---------------------------------------------------- */
/* ---- Constants and Types ---------------------------------------------- */

/* Return codes. */
#define SEQ_CMDS_ERROR			-1
#define SEQ_CMDS_BUFFERED		0
#define SEQ_CMDS_NOT_BUFFERED	1

/* Forward declaration */

/* Callback function registered with sequence command engine. It will
 * be invoked for each queued command for IOCTL processing.
 */
typedef int (*wlc_seq_cmds_ioctl_cb)
(
	wlc_info_t	*wlc,
	int 			cmd,
	void 			*buf,
	int 			len,
	wlc_if_t		*wlcif
);

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#ifdef SEQ_CMDS

/*
*****************************************************************************
* Function:   wl_seq_cmds_attach
*
* Purpose:    Initialize sequence commands private context.
*
* Parameters: wlc			(mod)	Common driver context.
*             callback	(in)	Callback invoked for each queued command
*                             for IOCTL processing.
*
* Returns:    Pointer to the packet filter private context. Returns NULL on error.
*****************************************************************************
*/
extern wlc_seq_cmds_info_t *wlc_seq_cmds_attach
(
	wlc_info_t 					*wlc,
	wlc_seq_cmds_ioctl_cb	callback
);

/*
*****************************************************************************
* Function:   wl_seq_cmds_detach
*
* Purpose:    Cleanup sequence commands private context.
*
* Parameters: info	(mod)	Sequence commands private context.
*
* Returns:    Nothing.
*****************************************************************************
*/
extern void wlc_seq_cmds_detach(wlc_seq_cmds_info_t *info);

/*
*****************************************************************************
* Function:   wl_seq_cmds_process
*
* Purpose:    Determine if IOCTL should be queued for later execution, or
*             processed immediately.
*
* Parameters: info	(mod)	Sequence commands private context.
*             cmd    (in)  IOCTL cmd to process.
*             arg    (in)  IOCTL cmd argument.
*             len    (in)  IOCTL cmd argement length.
*             wlcif  (in)  IOCTL wl interface.
*
* Returns:
*             - SEQ_CMDS_BUFFERED if the IOCTL was queued. Calling function
*               should not process IOCTL.
*             - SEQ_CMDS_NOT_BUFFERED if the IOCTL was not queued. Calling
*               function should process IOCTL, as normal.
*             - SEQ_CMDS_ERROR on error.
*****************************************************************************
*/
extern int wlc_seq_cmds_process
(
	wlc_seq_cmds_info_t	*info,
	int 						cmd,
	void 						*arg,
	int 						len,
	wlc_if_t					*wlcif
);

#else	/* stubs */

#define wlc_seq_cmds_attach(a, b)				(wlc_seq_cmds_info_t *)0x0dadbeef
#define wlc_seq_cmds_detach(a)					do {} while (0)
#define wlc_seq_cmds_process(a, b, c, d, e)	(SEQ_CMDS_NOT_BUFFERED)

#endif /* SEQ_CMDS */

#endif	/* _wlc_seq_cmds_h_ */
