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
 * $Id: igsc.c 779297 2019-09-24 18:34:27Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmip.h>
#include <osl.h>
#include <bcmnvram.h>
#include <clist.h>
#if defined(linux)
#include <osl_linux.h>
#ifdef BCM_NBUFF_WLMCAST_IPV6
#include <igs_linux.h>
#endif // endif
#else /* !defined(linux) */
#error "Unsupported osl"
#endif /* defined(linux) */
#include "igs_cfg.h"
#include "emfc_export.h"
#include "igs_export.h"
#include "igsc_export.h"
#include "igsc.h"
#include "igsc_sdb.h"

static mc_grp_spl_t mc_addr_rsvd[] =
{
	{ 0xe0000001, 0xffffffff },		/* All hosts */
	{ 0xe0000002, 0xffffffff },		/* All routers */
	{ 0xe0000004, 0xffffffff },		/* DVMRP routers */
	{ 0xe0000005, 0xffffffff },		/* OSPF1 routers */
	{ 0xe0000006, 0xffffffff },		/* OSPF2 routers */
	{ 0xe0000009, 0xffffffff },		/* RIP v2 routers */
	{ 0xe000000d, 0xffffffff },		/* PIMd routers */
	{ 0xe0000010, 0xffffffff },		/* IGMP v3 routers */
	{ 0xe00000fb, 0xffffffff },		/* Reserved */
	{ 0xe00000fc, 0xffffffff },		/* LLMNR address */
	{ 0xe00000fd, 0xffffffff },		/* Teredo tunneling client discovery */
	{ 0xe000ff87, 0xffffffff },		/* Reserved */
	{ 0xeffffffa, 0xffffffff },		/* UPnP */
};

static bool
igsc_is_reserved(uint32 addr)
{
	int32 i;

	for (i = 0; i < sizeof(mc_addr_rsvd)/sizeof(mc_grp_spl_t); i++)
	{
		if ((addr & mc_addr_rsvd[i].mask) ==
		    (mc_addr_rsvd[i].addr & mc_addr_rsvd[i].mask))
		{
			return (TRUE);
		}
	}

	return (FALSE);
}

/*
 * Description: This function is called if the snooper doesn't receive
 *              membership query from a router for timeout interval.
 *              It deletes the router port entry from the list. If this
 *              is the last router on the interface then the EMF router
 *              port entry is also deleted.
 *
 * Input:       rtlist_ptr - Multicast router interface/port entry
 */
void
igsc_rtlist_timer(igsc_rtlist_t *rtlist_ptr)
{
	clist_head_t *ptr;
	igsc_rtlist_t *rtl_ptr;
	bool mr_port_found = FALSE;
	igsc_info_t *igsc_info;

	igsc_info = rtlist_ptr->igsc_info;

	OSL_LOCK(igsc_info->rtlist_lock);
	/* Delete the expired router port entry */
	clist_delete(&rtlist_ptr->list);

	/* Check if there are more routers on this port/interface */
	for (ptr = igsc_info->rtlist_head.next;
	     ptr != &igsc_info->rtlist_head; ptr = ptr->next)
	{
		rtl_ptr = clist_entry(ptr, igsc_rtlist_t, list);
		if (rtl_ptr->ifp == rtlist_ptr->ifp)
		{
			mr_port_found = TRUE;
			break;
		}
	}

	/* Last multicast router on this port/interface */
	if (!mr_port_found)
	{
		IGS_DEBUG("Delete the router port %p in the EMF\n",
		          rtlist_ptr->ifp);
		emfc_rtport_del(igsc_info->emf_handle, rtlist_ptr->ifp);
	}

	OSL_UNLOCK(igsc_info->rtlist_lock);
	MFREE(igsc_info->osh, rtlist_ptr, sizeof(igsc_rtlist_t));

	return;
}

/*
 * Description: This function is called to find the entry in the router
 *              port list given the IP Address and the interface pointer.
 *
 * Input:       igsc_info - IGSC Global Instance handle
 *              ifp       - Interface pointer
 *              mr_ip     - IP address of the router
 *
 * Return:      Pointer to the entry found or NULL otherwise
 */
static igsc_rtlist_t *
igsc_rtlist_find(igsc_info_t *igsc_info, void *ifp, uint32 mr_ip)
{
	clist_head_t *ptr;
	igsc_rtlist_t *rtl_ptr;

	OSL_LOCK(igsc_info->rtlist_lock);

	for (ptr = igsc_info->rtlist_head.next;
	     ptr != &igsc_info->rtlist_head; ptr = ptr->next)
	{
		rtl_ptr = clist_entry(ptr, igsc_rtlist_t, list);

		if ((rtl_ptr->ifp == ifp) && (rtl_ptr->mr_ip == mr_ip))
		{
			OSL_UNLOCK(igsc_info->rtlist_lock);
			IGS_DEBUG("Router port entry %x %p found\n",
			          mr_ip, ifp);
			return (rtl_ptr);
		}
	}

	OSL_UNLOCK(igsc_info->rtlist_lock);

	return (NULL);
}

/*
 * Description: This function is called when the snooper receives IGMP
 *              membership query on one of the ports. It adds an entry
 *              to the multicast router port list. Each entry of the
 *              list contains the IP address of the multicast router
 *              and the interface/port on which it is present. It calls
 *              the EMF function to add a router port entry to the list
 *              maintained by EMF.
 *
 * Input:       igsc_info - IGSC Global Instance handle
 *              ifp       - Interface pointer
 *              mr_ip     - IP address of the router
 *
 * Return:      SUCCESS/FAILURE
 */
int32
igsc_rtlist_add(igsc_info_t *igsc_info, void *ifp, uint32 mr_ip)
{
	igsc_rtlist_t *rtlist_ptr;

	/* If the router port list entry exists already then refresh the
	 * timer for this entry.
	 */
	if ((rtlist_ptr = igsc_rtlist_find(igsc_info, ifp, mr_ip)) != NULL)
	{
		igs_osl_timer_update(rtlist_ptr->rtlist_timer, igsc_info->query_intv, FALSE);
		IGS_DEBUG("Duplicate router port entry %x %p\n", mr_ip, ifp);
		return (FAILURE);
	}

	/* Allocate and intialize the entry */
	rtlist_ptr = MALLOC(igsc_info->osh, sizeof(igsc_rtlist_t));
	if (rtlist_ptr == NULL)
	{
		IGS_ERROR("Failed to allocate memory size %zu for rtport list\n",
		          sizeof(igsc_rtlist_t));
		return (FAILURE);
	}

	rtlist_ptr->ifp = ifp;
	rtlist_ptr->mr_ip = mr_ip;
	rtlist_ptr->igsc_info = igsc_info;

	OSL_LOCK(igsc_info->rtlist_lock);

	/* Add the entry to the EMF router port list */
	if (emfc_rtport_add(igsc_info->emf_handle, ifp) != SUCCESS)
	{
		OSL_UNLOCK(igsc_info->rtlist_lock);
		MFREE(igsc_info->osh, rtlist_ptr, sizeof(igsc_rtlist_t));
		IGS_ERROR("Failed to add EMF rtport entry for %p\n", ifp);
		return (FAILURE);
	}

	/* Add timer to delete the entry on igmp query timeout */
	rtlist_ptr->rtlist_timer = igs_osl_timer_init("MR_PORT_LIST_TIMER",
	                                          (void (*)(void *))igsc_rtlist_timer,
	                                          (void *)rtlist_ptr);

	if (rtlist_ptr->rtlist_timer == NULL)
	{
		OSL_UNLOCK(igsc_info->rtlist_lock);
		emfc_rtport_del(igsc_info->emf_handle, ifp);
		MFREE(igsc_info->osh, rtlist_ptr, sizeof(igsc_rtlist_t));
		IGS_ERROR("Failed to allocate memory size %zu for timer\n",
			sizeof(igs_osl_timer_t));
		return (FAILURE);
	}

	igs_osl_timer_add(rtlist_ptr->rtlist_timer, igsc_info->query_intv, FALSE);

	/* Add the entry to IGS router port list */
	clist_add_head(&igsc_info->rtlist_head, &rtlist_ptr->list);

	OSL_UNLOCK(igsc_info->rtlist_lock);

	return (SUCCESS);
}

/*
 * Description: This function is called to clear the IGSC router port list.
 *              It also call the API to delete the corresponding EMF router
 *              port entry.
 *
 * Input:       igsc_info - IGSC Global Instance handle
 */
static void
igsc_rtlist_clear(igsc_info_t *igsc_info)
{
	clist_head_t *ptr, *tmp;
	igsc_rtlist_t *rtl_ptr;

	OSL_LOCK(igsc_info->rtlist_lock);

	for (ptr = igsc_info->rtlist_head.next;
	     ptr != &igsc_info->rtlist_head; ptr = tmp)
	{
		rtl_ptr = clist_entry(ptr, igsc_rtlist_t, list);
		tmp = ptr->next;
		igs_osl_timer_del(rtl_ptr->rtlist_timer);
		emfc_rtport_del(igsc_info->emf_handle, rtl_ptr->ifp);
		clist_delete(ptr);
		MFREE(igsc_info->osh, rtl_ptr, sizeof(igsc_rtlist_t));
	}

	OSL_UNLOCK(igsc_info->rtlist_lock);

	return;
}

/* Add Joining Member or Update joined member */
static int32 igsc_update_join_member(igsc_info_t *igsc_info, void *ifp, uint32 dest_ip,
	uint32 mcast_ip, uint32 mh_ip)
{
	int32 ret = FAILURE;

	if (igsc_is_reserved(mcast_ip)) {
		IGSC_STATS_INCR(igsc_info, igmp_not_handled);
		IGS_DEBUG("Reserved multicast address %x frame\n",
		          mcast_ip);
		return EMF_FLOOD;
	}
	/* When a host membership report is received add/refresh the entry for the host. */
	IGS_DEBUG("Join mcast_addr=%x\n", mcast_ip);
	IGSC_STATS_INCR(igsc_info, igmp_v2reports);

	ret = igsc_sdb_member_add(igsc_info, ifp, mcast_ip, mh_ip);
	return ((ret == SUCCESS) ? EMF_FORWARD : EMF_FLOOD);
}

/* Remove leave members */
static int32 igsc_update_leave_member(igsc_info_t *igsc_info, void *ifp,
	uint32 mcast_ip, uint32 mh_ip)
{
	int32 ret = FAILURE;

	IGSC_STATS_INCR(igsc_info, igmp_leaves);
	IGS_DEBUG("Leave mcast_addr=%x\n", mcast_ip);

	ret = igsc_sdb_member_del(igsc_info, ifp, mcast_ip, mh_ip);

	if (ret != SUCCESS)
		IGS_WARN("Deleting unknown member with grp %x host %x if %p",
				mcast_ip, mh_ip, ifp);
	return ((ret == SUCCESS) ? EMF_FORWARD : EMF_FLOOD);
}

/*
 * Description: This function is called by EMFL when an IGMP frame
 *              is received. Based on the IGMP packet type it either
 *              adds, deletes or refreshes the MFDB entry.
 *
 * Input:       s       - Snooper global instance data cookie
 *              ifp     - Pointer to interface the frame arrived on.
 *              iph     - Points to start of IP header in the packet.
 *              igmph   - Points to start of IGMP header in the packet.
 *              from_rp - TRUE when received from router port.
 *
 * Return:      EMF_FLOOD  - Flood the frame.
 *              EMF_SENDUP - Deliver the packet to IP layer.
 */
static int32
igsc_input(emfc_snooper_t *s, void *ifp, uint8 *iph, uint8 *igmph, bool from_rp)
{
	int32 ret = FAILURE;
	igsc_info_t *igsc_info;
	uint32 mgrp_ip, mh_ip, dest_ip;
	uint16 grp_num = 0, offset;
	igmpv3_report_t *igmpv3h;
	igmpv3_group_t  *igmpv3g;

	ASSERT(s != NULL);

	igsc_info = IGSC_INFO(s);

	IGSC_STATS_INCR(igsc_info, igmp_packets);

	if (from_rp)
	{
		IGS_DEBUG("Ignoring IGMP frame from router port\n");
		IGSC_STATS_INCR(igsc_info, igmp_not_handled);
		return (EMF_FLOOD);
	}

	mgrp_ip = ntoh32(*((uint32 *)(igmph + IGMPV2_GRP_ADDR_OFFSET)));
	mh_ip = ntoh32(*((uint32 *)(iph + IPV4_SRC_IP_OFFSET)));
	dest_ip = ntoh32(*((uint32 *)(iph + IPV4_DEST_IP_OFFSET)));

	switch (igmph[IGMPV2_TYPE_OFFSET])
	{
		case IGMPV2_HOST_NEW_MEMBERSHIP_REPORT:
			if (!IP_ISMULTI(mgrp_ip)) {
				IGS_ERROR("igsc_input: IGMPv2 new membership report received with "
					"unicast ip - %x\n", mgrp_ip);
				break;
			} else {
				/* Move report to function igsc_update_join_member() */
				ret = igsc_update_join_member(igsc_info, ifp, dest_ip,
					mgrp_ip, mh_ip);
				break;
			}

		case IGMPV2_LEAVE_GROUP_MESSAGE:
			if (!IP_ISMULTI(mgrp_ip)) {
				IGS_ERROR("igsc_input: IGMPv2 leave group received with "
					"unicast IP - %x\n", mgrp_ip);
			} else {
				/* Move Leave to function igsc_update_leave_member() */
				ret = igsc_update_leave_member(igsc_info, ifp, mgrp_ip, mh_ip);
			}
			break;

		case IGMPV2_HOST_MEMBERSHIP_REPORT:
			IGSC_STATS_INCR(igsc_info, igmp_reports);
			break;

		case IGMPV2_HOST_MEMBERSHIP_QUERY:
			IGSC_STATS_INCR(igsc_info, igmp_queries);

			/* Update the multicast router port list */
			if (mh_ip != 0)
			{
				IGS_DEBUG("Updating router port list with %x %p\n",
				          mh_ip, ifp);
				igsc_rtlist_add(igsc_info, ifp, mh_ip);
			}
			break;
		case IGMPV3_HOST_MEMBERSHIP_REPORT:
			igmpv3h = (igmpv3_report_t *)igmph;
			igmpv3g = (igmpv3_group_t*)(igmpv3h+1);

			IGS_DEBUG("group_num=%d\n", ntoh16(*(uint16 *)&igmpv3h->group_num));

			for (grp_num = 0; grp_num < ntoh16(*(uint16 *)&igmpv3h->group_num);
				grp_num++) {
				uint16 src_num;

				src_num = ntoh16(*(uint16 *)&igmpv3g->src_num);
				mgrp_ip = ntoh32(*(uint32 *)&igmpv3g->mcast_addr);

				IGS_DEBUG("IGMPv3 grp_num %d type %d mcast_addr=%x, src_num=%x\n",
					grp_num,
					igmpv3g->type,
					mgrp_ip,
					src_num);

				switch (igmpv3g->type) {
					case IGMPV3_MODE_IS_EXCLUDE:
					case IGMPV3_BLOCK_OLD_SOURCES:
					case IGMPV3_CHANGE_TO_EXCLUDE:
						if (!IP_ISMULTI(mgrp_ip)) {
						IGS_ERROR("ignore EXCLUDE non-multicast %x\n",
								mgrp_ip);
							break;
						}

						if (src_num == 0)
							ret = igsc_update_join_member(igsc_info,
								ifp, dest_ip, mgrp_ip, mh_ip);
						else
							ret = igsc_update_leave_member(igsc_info,
								ifp, mgrp_ip, mh_ip);
						break;

					case IGMPV3_CHANGE_TO_INCLUDE:
					case IGMPV3_MODE_IS_INCLUDE:
					/* Treat ALLOW_NEW_SOURCE as leave when src_num=0 */
					case IGMPV3_ALLOW_NEW_SOURCES:
						if (!IP_ISMULTI(mgrp_ip)) {
						IGS_ERROR("ignore INCLUDE non-multicast %x\n",
								mgrp_ip);
							break;
						}

						if (src_num == 0)
							ret = igsc_update_leave_member(igsc_info,
								ifp, mgrp_ip, mh_ip);
						else
							ret = igsc_update_join_member(igsc_info,
								ifp, dest_ip, mgrp_ip, mh_ip);
						break;
					default:
						IGS_ERROR("IGMPV3 type=%x\n", igmpv3g->type);
						break;
				}

				offset = sizeof(igmpv3_group_t)+
					igmpv3g->src_num*IGMPV3_SRC_ADDR_LEN + igmpv3g->aux_len;
				igmpv3g = (igmpv3_group_t*)(((unsigned char *)igmpv3g)+offset);
			}
			break;

		default:
			IGS_WARN("IGMP type %d not handled\n", igmph[IGMPV2_TYPE_OFFSET]);
			IGSC_STATS_INCR(igsc_info, igmp_not_handled);
			break;
	}

	return (EMF_FLOOD);
}

/*
* Internet checksum routine
*/
static uint16
inet_cksum(uint8 *buf, int32 len)
{
	uint32 sum = 0;
	uint16 val;

	while (len > 1)
	{
		val = *buf++ << 8;
		val |= *buf++;
		sum += val;
		len -= 2;
	}

	if (len > 0)
	{
		sum += (*buf) << 8;
	}

	while (sum >> 16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}

	return (hton16(~sum));
}

/*
 * Description: This function adds the IP header, IGMP header and data.
 */
static void
igsc_igmp_pkt_encap(uint8 *ip, uint32 src_ip, uint32 dest_ip,
                    uint8 igmp_type, uint8 max_rsp, uint32 grp_addr)
{
	uint8 *igmp;

	/* Prepare the IP header */
	ip[IP_VER_OFFSET] = ((IP_VER_4 << 4) | (IPV4_MIN_HLEN >> 2));
	ip[IPV4_TOS_OFFSET] = 0xc0;
	*((uint16 *)(ip + IPV4_LEN_OFFSET)) = hton16(IPV4_MIN_HLEN + IGMP_HLEN);
	*((uint16 *)(ip + IPV4_ID_OFFSET)) = 0;
	*((uint16 *)(ip + IPV4_FRAG_OFFSET)) = 0;
	ip[IPV4_TTL_OFFSET] = 1;
	ip[IPV4_PROT_OFFSET] = IP_PROT_IGMP;
	*((uint16 *)(ip + IPV4_CHKSUM_OFFSET)) = 0;
	*((uint32 *)(ip + IPV4_SRC_IP_OFFSET)) = src_ip;
	*((uint32 *)(ip + IPV4_DEST_IP_OFFSET)) = dest_ip;
	*((uint16 *)(ip + IPV4_CHKSUM_OFFSET)) = inet_cksum(ip, IPV4_MIN_HLEN);

	/* Fill the IGMP header fields */
	igmp = ip + IPV4_MIN_HLEN;
	igmp[IGMPV2_TYPE_OFFSET] = igmp_type;
	igmp[IGMPV2_MAXRSP_TIME_OFFSET] = max_rsp;
	*((uint16 *)(igmp + IGMPV2_CHECKSUM_OFFSET)) = 0;
	*((uint32 *)(igmp + IGMPV2_GRP_ADDR_OFFSET)) = grp_addr;
	*((uint16 *)(igmp + IGMPV2_CHECKSUM_OFFSET)) = inet_cksum(ip,
	                                               IGMP_HLEN + IPV4_MIN_HLEN);

	return;
}

/*
 * Description: This functions generates an IGMP Query. It is used to
 *              quickly determine the group members on the attached bridge
 *              ports.
 *
 * Return:      SUCCESS or FAILURE
 */
static int32
igsc_igmp_query_send(igsc_info_t *igsc_info)
{
	uint32 ip[64];
	uint32 lan_ip = 0;

	/* Build the IGMP Query packet */
	igsc_igmp_pkt_encap((uint8 *)ip, lan_ip, hton32(IPV4_MCADDR_ALLHOSTS),
	                    IGMPV2_HOST_MEMBERSHIP_QUERY, IGMPV2_MAXRSP_TIME / 100, 0);

	IGS_DEBUG("Forwarding IGMP Query on to bridge ports\n");

	return (igsc_info->wrapper.igs_broadcast(igsc_info->igs_info, (uint8 *)ip,
	                      IGMP_HLEN + IPV4_MIN_HLEN,
	                      IPV4_MCADDR_ALLHOSTS));
}

/*
 * Description: This function is called by the EMFL when forwarding
 *              is enabled.
 *
 * Input:       snooper - Snooper gloabl instance data.
 *
 * Return:      SUCCESS or FAILURE
 */
static int32
igsc_emf_enabled(emfc_snooper_t *snooper)
{
	IGS_INFO("EMF enable callback called\n");

	/* Send an IGMP Query packet to quickly learn the members present
	 * in the network.
	 */
	if (igsc_igmp_query_send(IGSC_INFO(snooper)) != SUCCESS)
	{
		IGS_ERROR("IGMP Query send failed\n");
		return (FAILURE);
	}

	return (SUCCESS);
}

/*
 * Description: This function is called by the EMFL when forwarding
 *              is disabled.
 *
 * Input:       snooper - Snooper gloabl instance data.
 *
 * Return:      SUCCESS or FAILURE
 */
static int32
igsc_emf_disabled(emfc_snooper_t *snooper)
{
	IGS_INFO("EMF disable callback\n");
	igsc_sdb_clear(IGSC_INFO(snooper));
	igsc_rtlist_clear(IGSC_INFO(snooper));
	return (SUCCESS);
}

/*
 * Description: This function is called by the EMFL when interface is deleted.
 *              It clears interface from IGS database.
 *
 * Input:       snooper - Snooper gloabl instance data.
 *		ifp     - Pointer to interface deleted.
 *
 * Return:      SUCCESS or FAILURE
 */
static int32
igsc_emf_ifp_del(emfc_snooper_t *snooper, void *ifp)
{
	igsc_sdb_interface_del(IGSC_INFO(snooper), ifp);
	igsc_interface_rtport_del(IGSC_INFO(snooper), ifp);
	return (SUCCESS);
}

/*
 * IGSL Packet Counters/Statistics Function
 */
int32
igsc_stats_get(igsc_info_t *igsc_info, igs_stats_t *stats, uint32 size)
{
	if (igsc_info == NULL)
	{
		IGS_ERROR("Invalid IGSC handle passed\n");
		return (FAILURE);
	}

	if (stats == NULL)
	{
		IGS_ERROR("Invalid buffer input\n");
		return (FAILURE);
	}

	if (size < sizeof(igs_stats_t))
	{
		IGS_ERROR("Insufficient buffer size %d\n", size);
		return (FAILURE);
	}

	*stats = igsc_info->stats;

	return (SUCCESS);
}

/*
 * RTPORT Interface Listing
 */
static int32
igsc_rtport_list(igsc_info_t *igsc, igs_cfg_rtport_list_t *list, uint32 size)
{
	int32 index = 0, bytes = 0, if_name_size;
	clist_head_t *ptr;
	igsc_rtlist_t *rtl_ptr;

	for (ptr = igsc->rtlist_head.next; ptr != &igsc->rtlist_head; ptr = ptr->next)
	{
		rtl_ptr = clist_entry(ptr, igsc_rtlist_t, list);

		bytes += sizeof(igs_cfg_rtport_list_t);
		if (bytes > size)
			return (FAILURE);

		list->rtport_entry[index].mr_ip = rtl_ptr->mr_ip;
		if_name_size = sizeof(list->rtport_entry[index].if_name);
		strncpy(list->rtport_entry[index].if_name, DEV_IFNAME(rtl_ptr->ifp), if_name_size);
		list->rtport_entry[index].if_name[if_name_size -1] = '\0';
		index++;
	}

	/* Update the total number of entries */
	list->num_entries = index;

	return (SUCCESS);
}

void
igsc_cfg_request_process(igsc_info_t *igsc_info, igs_cfg_request_t *cfg)
{
	IGS_DEBUG("IGS command identifier: %d\n", cfg->command_id);

	switch (cfg->command_id)
	{
		case IGSCFG_CMD_IGS_STATS:
			if (igsc_stats_get(igsc_info, (igs_stats_t *)cfg->arg,
			                   cfg->size) != SUCCESS)
			{
				cfg->status = IGSCFG_STATUS_FAILURE;
				cfg->size = sprintf(cfg->arg, "IGS stats get failed\n");
				break;
			}
			cfg->status = IGSCFG_STATUS_SUCCESS;
			break;

		case IGSCFG_CMD_IGSDB_LIST:
			if (igsc_sdb_list(igsc_info, (igs_cfg_sdb_list_t *)cfg->arg,
			                  cfg->size) != SUCCESS)
			{
				cfg->status = IGSCFG_STATUS_FAILURE;
				cfg->size = sprintf(cfg->arg, "IGSDB listing failed\n");
				break;
			}
			cfg->status = IGSCFG_STATUS_SUCCESS;
			break;

#ifdef BCM_NBUFF_WLMCAST_IPV6
		case IGSCFG_CMD_IGSDB_LIST_IPV6:
			if (igsc_sdb_list_ipv6(igsc_info, (igs_cfg_sdb_list_t *)cfg->arg,
			                  cfg->size) != SUCCESS)
			{
				cfg->status = IGSCFG_STATUS_FAILURE;
				cfg->size = sprintf(cfg->arg, "IGSDB listing failed\n");
				break;
			}
			cfg->status = IGSCFG_STATUS_SUCCESS;
			break;
#endif // endif

		case IGSCFG_CMD_RTPORT_LIST:
			if (igsc_rtport_list(igsc_info, (igs_cfg_rtport_list_t *)cfg->arg,
			                     cfg->size) != SUCCESS)
			{
				cfg->status = EMFCFG_STATUS_FAILURE;
				cfg->size = sprintf(cfg->arg, "RTPORT list get failed\n");
				break;
			}
			cfg->status = EMFCFG_STATUS_SUCCESS;
			break;

		default:
			cfg->status = IGSCFG_STATUS_CMD_UNKNOWN;
			cfg->size = sprintf(cfg->arg, "Unknown command id %d\n",
			                    cfg->command_id);
			break;
	}
}
#ifdef BCM_NBUFF_WLMCAST
/* igsc_remove_sta API is an obsolete function which will not be used in
 * BRANCH_KUDU_17_10 after IPV6 mc support is checked in, it is keep here to
 * make old KUDU)_TWIG branches like TWIG_17_10_25 branches to compile as it
 * use trunk emf source
 */
static int32
igsc_remove_sta(emfc_snooper_t *s, void *ifp, uint32 src_ip)
{
	return igsc_sdb_sta_del(IGSC_INFO(s), ifp, src_ip);
}
#endif /* BCM_NBUFF_WLMCAST */

/*
 * Description: This function is called from the OS specific module
 *              init routine to initialize the IGSL. This function
 *              primarily initializes the IGSL global data and IGSDB.
 *
 * Input:       inst_id   - IGS instance identifier.
 *              igs_info  - IGSL OS Specific global data handle
 *              osh       - OS abstraction layer handle
 *              wrapper   - wrapper specific info
 *
 * Return:      igsc_info - IGSL Common code global data handle
 */
void *
igsc_init(int8 *inst_id, void *igs_info, osl_t *osh, igsc_wrapper_t *wrapper)
{
	igsc_info_t *igsc_info;

	if (inst_id == NULL)
	{
		IGS_ERROR("Instance identifier NULL\n");
		return (NULL);
	}

	if (wrapper == NULL)
	{
		IGS_ERROR("wrapper info NULL\n");
		return (NULL);
	}

	IGS_DEBUG("Initializing IGMP Snooping Layer\n");

	/* Allocate memory */
	igsc_info = MALLOC(osh, sizeof(igsc_info_t));
	if (igsc_info == NULL)
	{
		IGS_ERROR("Failed to allocated memory size %zu for MFL\n",
		          sizeof(igsc_info_t));
		return (NULL);
	}

	IGS_DEBUG("Allocated memory for IGSC info\n");

	/* Initialize the IGS global data */
	bzero(igsc_info, sizeof(igsc_info_t));
	igsc_info->osh = osh;
	igsc_info->igs_info = igs_info;
	igsc_info->grp_mem_intv = IGMPV2_GRP_MEM_INTV;
	igsc_info->query_intv = IGMPV2_QUERY_INTV;

	/* Fill in the wrapper specific data */
	igsc_info->wrapper.igs_broadcast = wrapper->igs_broadcast;

#ifdef BCM_NBUFF_WLMCAST_IPV6
	if (igsc_init_ipv6(igsc_info, osh) == NULL) {
		MFREE(osh, igsc_info, sizeof(igsc_info_t));
		return NULL;
	}
#endif // endif
	/* Initialize the IGSDB */
	igsc_sdb_init(igsc_info);

	igsc_info->sdb_lock = OSL_LOCK_CREATE("SDB Lock");

	if (igsc_info->sdb_lock == NULL)
	{
		igsc_sdb_clear(igsc_info);
		MFREE(osh, igsc_info, sizeof(igsc_info_t));
		return (NULL);
	}

	/* Register IGMP v2 Snooper with EMFL */
	igsc_info->snooper.input_fn = igsc_input;
	igsc_info->snooper.emf_enable_fn = igsc_emf_enabled;
	igsc_info->snooper.emf_disable_fn = igsc_emf_disabled;
	igsc_info->snooper.emf_ifp_del_fn = igsc_emf_ifp_del;
#ifdef BCM_NBUFF_WLMCAST
	igsc_info->snooper.remove_sta_fn = igsc_remove_sta;
#endif // endif

	if (emfc_igmp_snooper_register(inst_id, &igsc_info->emf_handle,
	                               &igsc_info->snooper) != SUCCESS)
	{
		IGS_ERROR("IGMP Snooper couldn't register with EMF\n");
		igsc_sdb_clear(igsc_info);
		OSL_LOCK_DESTROY(igsc_info->sdb_lock);
		MFREE(osh, igsc_info, sizeof(igsc_info_t));
		return (NULL);
	}

	/* Initialize router port list head */
	clist_init_head(&igsc_info->rtlist_head);

	igsc_info->rtlist_lock = OSL_LOCK_CREATE("Router List Lock");

	if (igsc_info->rtlist_lock == NULL)
	{
		emfc_igmp_snooper_unregister(igsc_info->emf_handle);
		igsc_sdb_clear(igsc_info);
		OSL_LOCK_DESTROY(igsc_info->sdb_lock);
		MFREE(osh, igsc_info, sizeof(igsc_info_t));
		return (NULL);
	}

	IGS_DEBUG("Initialized IGSDB\n");

#if defined(BCM_NBUFF_WLMCAST_IPV6) && defined(BCM_WMF_MCAST_DBG)
	if (!wrapper->cmdline_indicator) {
		igs_instance_add(inst_id, NULL, igsc_info);
	}
	strncpy(igsc_info->inst_id, inst_id, IFNAMSIZ);
#endif // endif

	return (igsc_info);
}

/*
 * Description: This function is called from OS specific module cleanup
 *              routine. This routine primarily stops the group interval
 *              timers and cleans up the IGSDB entries.
 */
void
igsc_exit(igsc_info_t *igsc_info)
{
#ifdef BCM_NBUFF_WLMCAST_IPV6
#ifdef BCM_WMF_MCAST_DBG
	{
		igs_info_t *igs_info = igs_instance_find(igsc_info->inst_id);
		if (igs_info)
			igs_instance_del(igs_info, igsc_info);
	}
#endif // endif
	igsc_exit_ipv6(igsc_info);
#endif /* BCM_NBUFF_WLMCAST_IPV6 */
	/* Cleanup the IGS router port list entries */
	igsc_rtlist_clear(igsc_info);
	OSL_LOCK_DESTROY(igsc_info->rtlist_lock);

	/* Cleanup the IGSDB */
	igsc_sdb_clear(igsc_info);
	OSL_LOCK_DESTROY(igsc_info->sdb_lock);

	MFREE(igsc_info->osh, igsc_info, sizeof(igsc_info_t));

	IGS_DEBUG("Cleaned up IGSL, exiting common code\n");

	return;
}

/*
 * Description: This function is called to delete the IGSDB
 *              rtport entries on an interface
 *
 * Input:       igsc_info - igsc info pointer
 *		ifp       - interface pointer
 *
 * Return:      SUCCESS or FAILURE
 */
int32
igsc_interface_rtport_del(igsc_info_t *igsc_info, void *ifp)
{
	clist_head_t *ptr, *tmp;
	igsc_rtlist_t *rtl_ptr;

	OSL_LOCK(igsc_info->rtlist_lock);

	for (ptr = igsc_info->rtlist_head.next;
	     ptr != &igsc_info->rtlist_head; ptr = tmp)
	{
		rtl_ptr = clist_entry(ptr, igsc_rtlist_t, list);

		tmp = ptr->next;
		if (rtl_ptr->ifp == (igsc_rtlist_t *)ifp)
		{
			IGS_DEBUG("Router port entry %p found\n");
			/* disable timer before remove rtport */
			igs_osl_timer_del(rtl_ptr->rtlist_timer);
			emfc_rtport_del(igsc_info->emf_handle, rtl_ptr->ifp);
			clist_delete(ptr);
			MFREE(igsc_info->osh, rtl_ptr, sizeof(igsc_rtlist_t));
		}
	}

	OSL_UNLOCK(igsc_info->rtlist_lock);

	return SUCCESS;
}
