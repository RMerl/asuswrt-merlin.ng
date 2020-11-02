/*
 * IGMP Snooping Layer: IGMP Snooping module runs at layer 2. IGMP
 * Snooping layer uses the multicast information in the IGMP messages
 * exchanged between the participating hosts and multicast routers to
 * update the multicast forwarding database. This file contains the
 * common code routines of IGS module.
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
 * $Id: igsc_ipv6.c 768025 2018-10-03 06:47:52Z $
 */
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmip.h>
#include <bcmipv6.h>
#include <osl.h>
#include <bcmnvram.h>
#include <clist.h>
#if defined(linux)
#include <osl_linux.h>
#else /* defined(osl_xx) */
#error "Unsupported osl"
#endif /* defined(osl_xx) */
#include "igs_cfg.h"
#include "emfc_export.h"
#include "igs_export.h"
#include "igsc_export.h"
#include "igsc.h"
#include "igs_linux.h"
#include "igsc_sdb.h"
#include <bcm_mcast.h>
extern struct notifier_block mcast_snooping_notifier;
extern void *getprivInf(char *name, int port_no);
extern void *emfc_wmf_get_igsc(int ifindex);
extern void *emfc_wmf_scbfind(int ifindex, unsigned char *mac);

extern int igsc_mcast_snooping_event(unsigned long event, void *ptr);
int mcast_snooping_event(struct notifier_block *unused, unsigned long event, void *ptr)
{
	return igsc_mcast_snooping_event(event, ptr);
}
struct notifier_block mcast_snooping_notifier = {
	.notifier_call = mcast_snooping_event
};

int igsc_mcast_snooping_event(unsigned long event, void *ptr)
{
	t_BCM_MCAST_NOTIFY *notify = (t_BCM_MCAST_NOTIFY *)ptr;
	igsc_info_t *igsc_inf;
	void *scb = NULL;
	igsc_inf = emfc_wmf_get_igsc(notify->ifindex);
	if (igsc_inf && notify->proto == BCM_MCAST_PROTO_IPV6 &&
			(scb = emfc_wmf_scbfind(notify->ifindex, notify->repMac))) {
		switch (event) {
			case BCM_MCAST_EVT_SNOOP_ADD:
				if (igsc_sdb_member_add_ipv6(igsc_inf, scb,
						*((struct ipv6_addr *)&notify->ipv6grp),
						*((struct ipv6_addr *)&notify->ipv6rep)))
					printk("Failed add entry %pI6 \r\n", &notify->ipv6rep);
				break;
			case BCM_MCAST_EVT_SNOOP_DEL:
				if (igsc_sdb_member_del_ipv6(igsc_inf, scb,
						*((struct ipv6_addr *)&notify->ipv6grp),
						*((struct ipv6_addr *)&notify->ipv6rep)))
					printk("Failed delete entry %pI6 \r\n", &notify->ipv6rep);
				break;
		}
		IGS_IGSDB("From station: %pI6\n", notify->ipv6rep.s6_addr32);
		IGS_IGSDB("Rep Mac:0x:%pM\n", notify->repMac);
	}
	return 0;
}

uint8 igsc_instance_count = 0;

void *
igsc_init_ipv6(igsc_info_t *igsc_info, osl_t *osh)
{

	igsc_info->sdb_lock_ipv6 = OSL_LOCK_CREATE("SDB6 Lock");
	if (igsc_info->sdb_lock_ipv6 == NULL)
	{
		igsc_sdb_clear_ipv6(igsc_info);
		MFREE(osh, igsc_info, sizeof(igsc_info_t));
		return (NULL);
	}
	if (++igsc_instance_count == 1)
		bcm_mcast_notify_register(&mcast_snooping_notifier);
	IGS_IGSDB("Initialized IGSDB\n");

	return (igsc_info);
}

void
igsc_exit_ipv6(igsc_info_t *igsc_info)
{
	if (--igsc_instance_count == 0)
		bcm_mcast_notify_unregister(&mcast_snooping_notifier);
	igsc_sdb_clear_ipv6(igsc_info);
	OSL_LOCK_DESTROY(igsc_info->sdb_lock_ipv6);
return;
}
