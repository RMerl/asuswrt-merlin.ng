/**
 * @file
 * @brief
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
 *
 * $Id: wlc_seq_cmds.c 708017 2017-06-29 14:11:45Z $
 *
 */

/* ---- Include Files ---------------------------------------------------- */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>

#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc_pio.h>
#include <wlc.h>
#ifdef WLP2P
#include <proto/p2p.h>
#endif // endif

#include <wl_export.h>
#include <wlc_seq_cmds.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#define LOCAL_DEBUG	0

#if LOCAL_DEBUG
#undef WL_TRACE
#define WL_TRACE(arg)	printf arg
#endif // endif

/* This is useful during development to avoid compiler warnings/errors about
 * un-used static functions.
 */
#ifndef STATIC
#define STATIC	static
#endif /* STATIC */

/* wlc_pub_t struct access macros */
#define WLCUNIT(info)	((info)->wlc->pub->unit)
#define WLCOSH(info)		((info)->wlc->osh)

/* Bounds on delay command times. */
#define MIN_DELAY_MSEC	1
#define MAX_DELAY_MSEC	10000

/* IOVar table */
enum {

	/* Add delay between queued IOCTLS. */
	IOV_SEQ_CMDS_DELAY,

	/* Retrieve index of command that resulted in sequence error. */
	IOV_SEQ_CMDS_ERROR_INDEX,

	/* Concatenation of multiple IOCTLs/IOVARs into a single IOVAR. */
	IOV_SEQ_CMDS_LIST,

	/* Start to queue subsequent IOCTLs for subsequent processing. */
	IOV_SEQ_CMDS_START,

	/* Execute queued IOCTLS. */
	IOV_SEQ_CMDS_STOP

};

/* Sequence command state. */
typedef enum seq_cmds_state {

	/* seq_start has been received, and received IOCTL will be queued. */
	SEQ_CMDS_STATE_BUFFERING,

	/* Received IOCTLs should be processed immediately, as normal. */
	SEQ_CMDS_STATE_NOT_BUFFERING,

	/* Queued IOCTLs are being executed. */
	SEQ_CMDS_STATE_RUNNING

} seq_cmds_state_t;

/* Struct for individual IOCTL in a sequence. */
typedef struct seq_cmd {

	/* Pointer to next cmd in list, NULL if no more cmds. */
	struct seq_cmd		*next;

	/* IOCTL parameters. */
	int 					cmd;
	void 					*buf;
	int 					len;
	wlc_if_t				*wlcif;

} seq_cmd_t;

/* Linked list of sequence commands. */
typedef struct seq_cmd_list {

	/* Pointers to front and back of list. NULL if empty. */
	seq_cmd_t	*head;
	seq_cmd_t	*tail;

} seq_cmd_list_t;

/* Sequence commands private info structure. */
struct seq_cmds_info {

	/* Pointer back to wlc structure */
	wlc_info_t					*wlc;

	/* Indicates if IOCTLS should be buffered. */
	seq_cmds_state_t			state;

	/* Callback function to invoke to performed buffered IOCTL processing. */
	wlc_seq_cmds_ioctl_cb	ioctl_callback;

	/* Linked list of buffered commands. */
	seq_cmd_list_t				cmd_list;

	/* Index of command that failed in sequence. */
	uint32						error_index;
};

/* ---- Private Variables ------------------------------------------------ */

#define SEQ_LIST_STR			"seq_list"
#define SEQ_LIST_STRLEN		9				/* Includes NULL terminator. */

static const bcm_iovar_t seq_cmds_iovars[] = {
	{
		"seq_delay",
		IOV_SEQ_CMDS_DELAY,
		(0),
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		"seq_error_index",
		IOV_SEQ_CMDS_ERROR_INDEX,
		(0),
		IOVT_UINT32,
		sizeof(uint32)
	},

	{
		SEQ_LIST_STR,
		IOV_SEQ_CMDS_LIST,
		(0),
		IOVT_BUFFER,
		sizeof(wl_seq_cmd_ioctl_t)
	},

	{
		"seq_start",
		IOV_SEQ_CMDS_START,
		(0),
		IOVT_VOID,
		0
	},

	{
		"seq_stop",
		IOV_SEQ_CMDS_STOP,
		(0),
		IOVT_VOID,
		0
	},

	{NULL, 0, 0, 0, 0 }
};

/* ---- Private Function Prototypes -------------------------------------- */

STATIC void
add_cmd_to_list(seq_cmd_list_t *list, seq_cmd_t *cmd);

STATIC int
seq_cmds_doiovar
(
	void 					*hdl,
	const bcm_iovar_t	*vi,
	uint32 				actionid,
	const char 			*name,
	void 					*p,
	uint					plen,
	void 					*a,
	int 					alen,
	int 					vsize,
	struct wlc_if 		*wlcif
);

STATIC int
run_buffered_cmds(wlc_seq_cmds_info_t *info);

STATIC int
run_concat_cmds(wlc_seq_cmds_info_t *info, const uint8 *buf, int buflen);

STATIC void
free_buffered_cmds(wlc_seq_cmds_info_t *info);

/* ---- Functions -------------------------------------------------------- */

/* ----------------------------------------------------------------------- */
wlc_seq_cmds_info_t * BCMATTACHFN(wlc_seq_cmds_attach)
(
	wlc_info_t 					*wlc,
	wlc_seq_cmds_ioctl_cb	callback
)
{
	wlc_seq_cmds_info_t *info;

	/* Allocate sequence commands private info struct. */
	info = MALLOCZ(wlc->osh, sizeof(wlc_seq_cmds_info_t));
	if (info == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	/* Init command sequence private info struct. */
	info->ioctl_callback	= callback;
	info->wlc 				= wlc;
	info->state 			= SEQ_CMDS_STATE_NOT_BUFFERING;

	/* Register this module. */
	if (wlc_module_register(wlc->pub,
	                        seq_cmds_iovars,
	                        "seq_cmds",
	                        info,
	                        seq_cmds_doiovar,
	                        NULL,
	                        NULL,
	                        NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	return (info);

fail:
	if (NULL != info)
		MFREE(WLCOSH(info), info, sizeof(wlc_seq_cmds_info_t));

	return (NULL);
}

/* ----------------------------------------------------------------------- */
void
BCMATTACHFN(wlc_seq_cmds_detach)(wlc_seq_cmds_info_t *info)
{
	if (info == NULL)
		return;

	/* De-register this module. */
	wlc_module_unregister(info->wlc->pub, "seq_cmds", info);

	/* Free allocated memory for buffered IOCTLS. */
	free_buffered_cmds(info);

	/* Free allocated sequence commands private info struct. */
	MFREE(WLCOSH(info), info, sizeof(wlc_seq_cmds_info_t));
}

/* ----------------------------------------------------------------------- */
int
wlc_seq_cmds_process
(
	wlc_seq_cmds_info_t	*info,
	int 						cmd,
	void 						*buf,
	int 						len,
	wlc_if_t					*wlcif
)
{
	int 				rc = SEQ_CMDS_NOT_BUFFERED;
	seq_cmd_t			*seq_cmd = NULL;
	unsigned int 		i;
	static const char	*exclude_iovars[] =
	{
		"seq_start",
		"seq_stop",
		"wifiaction",
		"remote",
		"bsscfg",
		"extlog",
		"assoc_preserved",
		NULL
	};

	/* This is a hack - we need to intercept various IOCTLs. These are issued
	 * implicitly by wl.exe each time it is invoked. We never want to buffer
	 * these, or else wl.exe will stop working.
	 */
	if (WL_SEQ_CMDS_GET_IOCTL_FILTER(cmd))
		return (SEQ_CMDS_NOT_BUFFERED);

	/* Only buffer wl IOCTLs, not other IOCTLs (e.g. NDIS OIDs, iwconfig cmds) */
	if (cmd >= WLC_LAST)
		return (SEQ_CMDS_NOT_BUFFERED);

	/* Intercept the seq_start and seq_stop IOVARs. Never buffer these, or
	 * else we will never detect the end of the command sequence.
	 */
	i = 0;
	while (exclude_iovars[i] != NULL) {
		if (((cmd == WLC_GET_VAR) || (cmd == WLC_SET_VAR)) && (buf != NULL)) {
			if (!strncmp(exclude_iovars[i], buf, strlen(exclude_iovars[i])))
				return (SEQ_CMDS_NOT_BUFFERED);
		}
		i++;
	}

	/* Check if we should buffer the received IOCTL... */
	if (SEQ_CMDS_STATE_BUFFERING == info->state) {

		/* Get IOCTLs are not supported in command sequences. */
		if (cmd == WLC_GET_VAR)
			goto fail;

		/* Allocate sequence command for buffering. */
		seq_cmd = MALLOC(WLCOSH(info), sizeof(seq_cmd_t));
		if (seq_cmd == NULL) {
			WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
			          WLCUNIT(info), __FUNCTION__, MALLOCED(WLCOSH(info))));
			goto fail;
		}

		/* Copy IOCTL state into alloc'ed buffer. */
		bzero(seq_cmd, sizeof(seq_cmd_t));
		seq_cmd->cmd 	= cmd;
		seq_cmd->len 	= len;
		seq_cmd->wlcif	= wlcif;

		/* Allocate and copy variable length IOCTL parameters. */
		if (0 != len) {
			seq_cmd->buf = MALLOC(WLCOSH(info), len);
			if (seq_cmd->buf == NULL) {
				WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
				          WLCUNIT(info), __FUNCTION__, MALLOCED(WLCOSH(info))));
				goto fail;
			}

			bcopy(buf, seq_cmd->buf, len);
		}

		/* Add IOCTL to queue for later processing. */
		add_cmd_to_list(&info->cmd_list, seq_cmd);
		rc = SEQ_CMDS_BUFFERED;

		WL_TRACE(("wl%d: %s: Add ioctl (%d) to queue\n",
		          WLCUNIT(info), __FUNCTION__, cmd));
	}

	return (rc);

fail:
	if (seq_cmd && (NULL != seq_cmd->buf))
		MFREE(WLCOSH(info), seq_cmd->buf, len);

	if (NULL != seq_cmd)
		MFREE(WLCOSH(info), seq_cmd, sizeof(seq_cmd_t));

	info->state = SEQ_CMDS_STATE_NOT_BUFFERING;
	free_buffered_cmds(info);

	return (SEQ_CMDS_ERROR);
}

/*
*****************************************************************************
* Function:		add_cmd_to_list
*
* Purpose:		Add individual IOCTL to end of sequence command list.
*
* Parameters:	list	(mod) List to add to.
*              cmd	(mod) Command to add.
*
* Returns:		Nothing.
*****************************************************************************
*/
STATIC void
add_cmd_to_list(seq_cmd_list_t *list, seq_cmd_t *cmd)
{
	/* Add command to the end of the list. */
	if (list->tail != NULL)
		list->tail->next = cmd;
	else
		list->head = cmd;

	list->tail     = cmd;
	cmd->next  = NULL;
}

/*
*****************************************************************************
* Function:   seq_cmds_doiovar
*
* Purpose:    Handles command sequencing related IOVars.
*
* Parameters:
*
* Returns:    BCME_OK on success.
*****************************************************************************
*/
STATIC int
seq_cmds_doiovar
(
	void 					*hdl,
	const bcm_iovar_t	*vi,
	uint32 				actionid,
	const char 			*name,
	void 					*p,
	uint 					plen,
	void 					*a,
	int 					alen,
	int 					vsize,
	struct wlc_if 		*wlcif
)
{
	wlc_seq_cmds_info_t	*info;
	int 						err = BCME_OK;

	info = hdl;

	switch (actionid) {

		case IOV_SVAL(IOV_SEQ_CMDS_DELAY):
		{
			uint32 		delay_msec;

			/* Validate that command buffering was previously enabled. */
			if (SEQ_CMDS_STATE_RUNNING != info->state) {
				err = BCME_ERROR;
				break;
			}

			bcopy(a, &delay_msec, sizeof(delay_msec));

			/* Validate requested delay is within bounds. */
			if ((delay_msec > MAX_DELAY_MSEC) || (delay_msec < MIN_DELAY_MSEC)) {
				err = BCME_BADARG;
				break;
			}

			/* Spin for specified delay. */
			OSL_DELAY(delay_msec * 1000);
		}
		break;

		case IOV_GVAL(IOV_SEQ_CMDS_ERROR_INDEX):
		{
			if (alen < sizeof(info->error_index)) {
				err = BCME_BUFTOOSHORT;
				break;
			}

			bcopy(&info->error_index, a, sizeof(info->error_index));
		}
		break;

		case IOV_SVAL(IOV_SEQ_CMDS_LIST):
		{
			uint8	*buf_start = (uint8 *)a - SEQ_LIST_STRLEN;

			/* The packet format of concatenated IOCTLs mandates that individual
			 * commands internally maintain a 32-bit aligned pattern, but if
			 * that pattern (i.e. the very first command) does not start on a
			 * 32-bit boundary, none of the others will.
			 */
			if (!ISALIGNED((int)(buf_start - (uint8 *)0), WL_SEQ_CMD_ALIGN_BYTES)) {
				err = BCME_BADADDR;
				break;
			}

			/* Run commands contained in concatenated command list. */
			err = run_concat_cmds(info, a, alen);
		}
		break;

		case IOV_SVAL(IOV_SEQ_CMDS_START):
		{
			/* Validate that command buffering isn't already enabled. */
			if (SEQ_CMDS_STATE_BUFFERING == info->state) {

				info->state = SEQ_CMDS_STATE_NOT_BUFFERING;

				/* Free allocated memory for buffered IOCTLS. */
				free_buffered_cmds(info);

				err = BCME_ERROR;
				break;
			}

			/* Buffer subsequent IOCTL commands. */
			info->state = SEQ_CMDS_STATE_BUFFERING;
		}
		break;

		case IOV_SVAL(IOV_SEQ_CMDS_STOP):
		{
			/* Validate that command buffering was previously enabled. */
			if (SEQ_CMDS_STATE_BUFFERING != info->state) {
				err = BCME_ERROR;
				break;
			}

			/* Process buffered IOCTL commands. */
			err = run_buffered_cmds(info);

			/* Stop buffering IOCTL commands. */
			info->state = SEQ_CMDS_STATE_NOT_BUFFERING;

			/* Free allocated memory for buffered IOCTLS. */
			free_buffered_cmds(info);
		}
		break;

		default:
		{
			err = BCME_UNSUPPORTED;
		}
		break;
	}

	return (err);
}

/*
*****************************************************************************
* Function:		run_buffered_cmds
*
* Purpose:		Execute each of the stored IOCTLs in the command sequence queue.
*
* Parameters:	info	(mod)	Sequence commands private context.
*
* Returns:		BCME_OK on success, else BCME_xxx error code.
*****************************************************************************
*/
STATIC int
run_buffered_cmds(wlc_seq_cmds_info_t *info)
{
	seq_cmd_t 			*cmd;
	int					rc;
	seq_cmds_state_t	old_state;

	rc		= BCME_OK;

	old_state 			= info->state;
	info->state 		= SEQ_CMDS_STATE_RUNNING;

	info->error_index	= 0;

	cmd = info->cmd_list.head;
	while (NULL != cmd) {

		info->error_index++;

		WL_TRACE(("wl%d: %s: Run ioctl (%d)\n",
		          WLCUNIT(info), __FUNCTION__, cmd->cmd));

		/* Invoke user registered wl_ioctl callback. */
		rc = info->ioctl_callback(info->wlc, cmd->cmd, cmd->buf, cmd->len, cmd->wlcif);
		if (BCME_OK != rc)
			break;

		cmd = cmd->next;
	}

	info->state = old_state;
	return (rc);
}

/*
*****************************************************************************
* Function:		run_concat_cmds
*
* Purpose:		Execute each of the IOCTLs concatenated in a contiguous buffer.
*
* Parameters:	info		(mod)	Sequence commands private context.
*              buf		(in)  Contiguous buffer that contains 1 or more
*									   concatenated IOCTLs.
*					buflen	(in)	Length of contiguous buffer.
*
* Returns:		BCME_OK on success, else BCME_xxx error code.
*****************************************************************************
*/
STATIC int
run_concat_cmds(wlc_seq_cmds_info_t *info, const uint8 *buf, int buflen)
{
	int						rc;
	void						*ioctl_buf;
	const uint8				*end_buf;
	seq_cmds_state_t		old_state;
	wl_seq_cmd_ioctl_t 	*seq_ioc;

	rc		= BCME_OK;
	ioctl_buf = NULL;

	if (SEQ_CMDS_STATE_RUNNING != info->state)
		info->error_index = 1;

	old_state 	= info->state;
	info->state	= SEQ_CMDS_STATE_RUNNING;

	/* 'buf' is a contiguous buffer of 1 or more IOCTLs. The length of
	 * the contiguous buffer is 'buflen'. Find the end of the buffer - i.e.
	 * the point were we should stop parsing IOCTLs - 'end_buf'.
	 */
	end_buf = &buf[buflen];

	/* Skip padding such that IOCTL struct is 32-bit aligned. */
	buf = ALIGN_ADDR(buf, WL_SEQ_CMD_ALIGN_BYTES);

	while (end_buf > buf) {

		seq_ioc = (wl_seq_cmd_ioctl_t *) buf;

		info->error_index++;

		/* Get IOVARs are not supported in command sequences. */
		if (seq_ioc->cmd == WLC_GET_VAR) {
			rc = BCME_BADARG;
			break;
		}

		/* Create a wl IOCTL from the seq_cmd IOCTL. */
		if (seq_ioc->len != 0)
			ioctl_buf = (void *) &seq_ioc[1];

		WL_TRACE(("wl%d: %s: Run ioctl (%d)\n",
		          WLCUNIT(info), __FUNCTION__, seq_ioc->cmd));

		/* Invoke user registered wl_ioctl callback. */
		rc = info->ioctl_callback(info->wlc, seq_ioc->cmd, ioctl_buf, seq_ioc->len, NULL);
		if (BCME_OK != rc)
			break;

		/* Skip to next IOCTL in contiguous buffer. Remember to
		 * skip padding such that IOCTL struct is 32-bit aligned.
		 */
		buf += (sizeof(wl_seq_cmd_ioctl_t) + seq_ioc->len);
		buf  = ALIGN_ADDR(buf, WL_SEQ_CMD_ALIGN_BYTES);
	}

	info->state = old_state;
	return (rc);
}

/*
*****************************************************************************
* Function:		free_buffered_cmds
*
* Purpose:		Free all allocated memory for each IOCTL in command sequence Q.
*
* Parameters:	info	(mod)	Sequence commands private context.
*
* Returns:		Nothing.
*****************************************************************************
*/
STATIC void
free_buffered_cmds(wlc_seq_cmds_info_t *info)
{
	seq_cmd_t 	*cmd;
	seq_cmd_t	*next_cmd;

	cmd = info->cmd_list.head;
	while (NULL != cmd) {

		next_cmd = cmd->next;

		if (NULL != cmd->buf)
			MFREE(WLCOSH(info), cmd->buf, cmd->len);

		MFREE(WLCOSH(info), cmd, sizeof(seq_cmd_t));

		cmd = next_cmd;
	}

	bzero(&info->cmd_list, sizeof(info->cmd_list));
}
