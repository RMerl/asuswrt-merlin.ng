/*
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
 * $Id: emf_export.h 780195 2019-10-17 19:00:17Z $
 */

#ifndef _EMF_EXPORT_H_
#define _EMF_EXPORT_H_

struct emf_info;

/*
 * Description: This function allows EMFL to start receiving frames for
 *              its processing and/or forwarding. It registers OS specific
 *              hook functions.  These functions are called from the
 *              bridge or ip forwarding path when packets reach respective
 *              bridge pre or ip post hook points. This function can be
 *              called when multicast forwarding is enabled by user.
 *
 * Input:       emfi - EMFL instance handle.
 *
 * Return:      Registration status, SUCCESS or FAILURE.
 */
extern int32 emf_hooks_register(struct emf_info *emfi);

/*
 * Description: This function causes EMFL to stop receiving frames. It
 *              unregisters the OS specific hook functions. This function
 *              can be called when multicast forwarding is disabled by
 *              user.
 *
 * Input:       emfi - Global EMF instance handle.
 */
extern void emf_hooks_unregister(struct emf_info *emfi);

/*
 * Description: This function is called by EMFL common code when it wants
 *              to forward the packet on to a specific port. It adds the
 *              MAC header to the frame if not present and queues the
 *              frame to the destination interface indicated by the input
 *              paramter (txif).
 *
 * Input:       emfi    - EMFL instance handle
 *              sdu     - Pointer to the packet buffer.
 *              mgrp_ip - Multicast destination address.
 *              txif    - Interface to send the frame on.
 *              rt_port - TRUE when the packet is received from IP stack
 *                        (router port), FALSE otherwise.
 *
 * Return:      SUCCESS or FAILURE.
 */
extern int32 emf_forward(struct emf_info *emfi, void *sdu, uint32 mgrp_ip,
                         void *txif, int rt_port);

/*
 * Description: This function is called to send the packet buffer up
 *              to the IP stack.
 *
 * Input:       emfi - EMF instance information
 *              sdu  - Pointer to the packet buffer.
 */
extern void emf_sendup(struct emf_info *emfi, void *sdu);

#endif /* _EMF_EXPORT_H_ */
