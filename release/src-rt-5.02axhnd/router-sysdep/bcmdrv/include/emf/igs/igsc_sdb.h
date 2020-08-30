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
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: igsc_sdb.h 679290 2017-01-13 07:39:40Z $
 */

#ifndef _IGSC_SDB_H_
#define _IGSC_SDB_H_
#ifdef BCM_NBUFF_WLMCAST_IPV6
#include <bcmipv6.h>
#include <typedefs.h>
#define IGSDB_MGRP_HASH_IPV6(m) ((((m).s6_addr32[0])+((m).s6_addr32[1])+((m).s6_addr32[2])+\
			((m).s6_addr32[3])) &7)
#endif // endif
#define IGSDB_MGRP_HASH(m)     ((((m) >> 24) + ((m) >> 16) + \
				 ((m) >> 8) + ((m) & 0xff)) & 7)

/*
 * Group entry of IGSDB
 */
typedef struct igsc_mgrp
{
	clist_head_t   mgrp_hlist;   /* Multicast Groups hash list */
#ifdef BCM_NBUFF_WLMCAST_IPV6
	struct ipv6_addr mgrp_ipv6;   /* Multicast Group IPv6 address */
#endif // endif
	uint32         mgrp_ip;      /* Multicast Group IP Address */
	clist_head_t   mh_head;      /* List head of group members */
	clist_head_t   mi_head;      /* List head of interfaces */
	igsc_info_t    *igsc_info;   /* IGSC instance data */
} igsc_mgrp_t;

/*
 * Interface entry of IGSDB
 */
typedef struct igsc_mi
{
	clist_head_t   mi_list;      /* Multicast i/f list prev and next */
	void           *mi_ifp;      /* Interface pointer */
	int32          mi_ref;       /* Ref count of hosts on the i/f */
} igsc_mi_t;

/*
 * Host entry of IGSDB
 */
typedef struct igsc_mh
{
	clist_head_t   mh_list;      /* Group members list prev and next */
#ifdef BCM_NBUFF_WLMCAST_IPV6
	struct ipv6_addr mh_ipv6;
#endif // endif

	uint32         mh_ip;        /* Unicast IP address of host */
	igsc_mgrp_t    *mh_mgrp;     /* Multicast forwarding entry for the
	                              * group
				      */
	igs_osl_timer_t    *mgrp_timer;  /* Group Membership Interval timer */
	igsc_mi_t      *mh_mi;       /* Interface connected to host */
	int		missed_report_cnt;	/* No of membership report it missied */
} igsc_mh_t;

/*
 * Prototypes
 */
int32 igsc_sdb_member_add(igsc_info_t *igsc_info, void *ifp, uint32 mgrp_ip,
                          uint32 mh_ip);
int32 igsc_sdb_member_del(igsc_info_t *igsc_info, void *ifp, uint32 mgrp_ip,
                          uint32 mh_ip);
void igsc_sdb_init(igsc_info_t *igsc_info);
#ifdef BCM_NBUFF_WLMCAST
int32 igsc_sdb_sta_del(igsc_info_t *igsc_info, void *ifp, uint32 mh_ip);
#endif /* BCM_NBUFF_WLMCAST */
#ifdef BCM_NBUFF_WLMCAST_IPV6
void igsc_sdb_clear_ipv6(igsc_info_t *igsc_info);
int32 igsc_sdb_clear_group_ipv6(igsc_info_t *igsc_info, struct ipv6_addr *grp);
int32 igsc_sdb_member_add_ipv6(igsc_info_t *igsc_info, void *ifp,
		struct ipv6_addr mgrp_ip, struct ipv6_addr mh_ip);
int32 igsc_sdb_member_del_ipv6(igsc_info_t *igsc_info, void *ifp,
		struct ipv6_addr mgrp_ip, struct ipv6_addr mh_ip);
int32 igsc_sdb_interface_del_ipv6(igsc_info_t *igsc_info, void *ifp);
#endif /* BCM_NBUFF_WLMCAST_IPV6 */
#endif /* _IGSC_SDB_H_ */
