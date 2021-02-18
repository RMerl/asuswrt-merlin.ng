/*
 * Event loop
 * Copyright 2002-2003, Jouni Malinen <jkmaline@cc.hut.fi>
 * All Rights Reserved.
 *
 *
 * Copyright 2019 Broadcom
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
 *
 * $Id: eloop.h 701315 2017-05-24 13:08:15Z $
 */

#ifndef ELOOP_H
#define ELOOP_H

/* Magic number for eloop_cancel_timeout() */
#define ELOOP_ALL_CTX (void *) -1

/* Initialize global event loop data - must be called before any other eloop_*
 * function. user_data is a pointer to global data structure and will be passed
 * as eloop_ctx to signal handlers.
 */
void eloop_init(void *user_data);

/* Register handler for read event */
int eloop_register_read_sock(int sock,
		void (*handler)(int sock, void *eloop_ctx, void *sock_ctx),
		void *eloop_data, void *user_data);
int eloop_unregister_read_sock(int sock);

/* Register timeout */
int eloop_register_timeout(unsigned int secs, unsigned int usecs,
		void (*handler)(void *eloop_ctx, void *timeout_ctx),
		void *eloop_data, void *user_data);

/* Cancel timeouts matching <handler,eloop_data,user_data>.
 * ELOOP_ALL_CTX can be used as a wildcard for cancelling all timeouts
 * regardless of eloop_data/user_data.
 */
int eloop_cancel_timeout(void (*handler)(void *eloop_ctx, void *sock_ctx),
		void *eloop_data, void *user_data);

/* Register handler for signal.
 * Note: signals are 'global' events and there is no local eloop_data pointer
 * like with other handlers. The (global) pointer given to eloop_init() will be
 * used as eloop_ctx for signal handlers.
 */
int eloop_register_signal(int sock,
		void (*handler)(int sig, void *eloop_ctx, void *signal_ctx),
		void *user_data);

/* Start event loop and continue running as long as there are any registered
 * event handlers.
 */
void eloop_run(void);
/* Single run */
void eloop_read(fd_set *fdset);

/* Terminate event loop even if there are registered events. */
void eloop_terminate(void);

/* Free any reserved resources. After calling eloop_destoy(), other eloop_*
 * functions must not be called before re-running eloop_init().
 */
void eloop_destroy(void);

/* Check whether event loop has been terminated. */
int eloop_terminated(void);

/* Return user_data pointer that was registered with eloop_init() */
void * eloop_get_user_data(void);

#endif /* ELOOP_H */
