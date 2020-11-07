/*
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
 * $Id: emf_linux.h 732217 2017-11-16 06:10:54Z $
 */

#ifndef _EMF_LINUX_H_
#define _EMF_LINUX_H_

#define EMF_MAX_INST          8

#ifdef EMFDBG
#define EMF_DUMP_PKT(data) \
{ \
	int32 i; \
	for (i = 0; i < 64; i++) \
		printk("%02x ", (data)[i]); \
	printk("\n"); \
}
#else /* EMFDBG */
#define EMF_DUMP_PKT(data)
#endif /* EMFDBG */

#define EMF_BRPORT_STATE(if)  (((br_port_t *)((if)->br_port))->state)

typedef struct emf_iflist
{
	struct emf_iflist  *next;        /* Next pointer */
	struct net_device  *if_ptr;      /* Interface pointer */
} emf_iflist_t;

typedef struct emf_info
{
	struct emf_info    *next;        /* Next pointer */
	int8               inst_id[16];  /* EMF Instance identifier */
	osl_t              *osh;         /* OS layer handle */
	struct net_device  *br_dev;      /* Bridge device pointer */
	struct emfc_info   *emfci;       /* EMFC Global data handle */
	uint32             hooks_reg;    /* EMF Hooks registration */
	emf_iflist_t       *iflist_head; /* EMF interfaces list */
} emf_info_t;

typedef struct emf_struct
{
	struct sock *nl_sk;              /* Netlink socket */
	emf_info_t  *list_head;          /* EMF instance list */
	osl_lock_t  lock;                /* EMF locking */
	int32       hooks_reg;           /* EMF hooks registration ref count */
	int32       inst_count;          /* EMF instance count */
} emf_struct_t;

typedef struct br_port
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
	/* Not used for this version of Linux */
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	struct net_bridge	*br;
	struct net_device	*dev;
	struct list_head	list;
	/* STP */
	u8			priority;
	u8			state;
#else /* LINUX_VERSION_CODE >= 2.6.0 */
	struct br_port     *next;
	struct net_bridge  *br;
	struct net_device  *dev;
	int32              port_no;
	uint16             port_id;
	int32              state;
#endif /* LINUX_VERSION_CODE >= 2.6.0 */
} br_port_t;

#endif /* _EMF_LINUX_H_ */
