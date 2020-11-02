/*
 * Efficient Multicast Forwarding Layer: This module does the efficient
 * layer 2 forwarding of multicast streams, i.e., forward the streams
 * only on to the ports that have corresponding group members there by
 * reducing the bandwidth utilization and latency. It uses the information
 * updated by IGMP Snooping layer to do the optimal forwarding. This file
 * contains the common code routines of EMFL.
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
 * $Id: emfc_ipv6.c 771177 2019-01-17 06:59:30Z $
 */
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <ethernet.h>
#include <bcmip.h>
#include <bcmipv6.h>
#include <osl.h>
#if defined(linux)
#include <osl_linux.h>
#else /* defined(osl_xx) */
#error "Unsupported osl"
#endif /* defined(osl_xx) */
#include <bcmutils.h>
#include "clist.h"
#include "emf_cfg.h"
#include "emfc_export.h"
#include "emfc.h"
#include "emf_linux.h"
#include <bcm_mcast.h>

#ifndef BCM_NBUFF_WLMCAST
#define EMFC_PKTDUP(emf, skb) PKTDUP((emf)->osh, (skb))
#define EMFC_PKTDUP_CPY(emf, skb) PKTDUP_CPY((emf)->osh, (skb))
#define EMFC_PKTFREE(emf, skb, send) PKTFREE((emf)->osh, (skb), (send))
#else
#include <dhd_nic_common.h>
void  *emfc_pktdup(emfc_info_t *emfc, void *skb);
void  emfc_pktfree(emfc_info_t *emfc, void *skb, bool send);
void  *emfc_pktdup_dup(emfc_info_t *emfc, void *skb);

#define EMFC_PKTDUP(emf, skb) emfc_pktdup((emf), (skb))
#define EMFC_PKTDUP_CPY(emf, skb) emfc_pktdup_cpy((emf), (skb))
#define EMFC_PKTFREE(emf, skb, send) emfc_pktfree((emf), (skb), (send))
#endif // endif

extern int ipv6_type(const struct ipv6_addr *addr);
/*
 * MFDB Listing Function
 */
int32
emfc_mfdb_list_ipv6(emfc_info_t *emfc, emf_cfg_mfdb_list_t *list, uint32 size)
{
	clist_head_t *ptr1, *ptr2;
	emfc_mi_t *mi;
	emfc_mgrp_t *mgrp;
	int32 i, index = 0;

	if (emfc == NULL)
	{
		EMF_ERROR("Invalid EMFC handle passed\n");
		return (FAILURE);
	}

	if (list == NULL)
	{
		EMF_ERROR("Invalid buffer input\n");
		return (FAILURE);
	}

	for (i = 0; i < MFDB_HASHT_SIZE; i++)
	{
		for (ptr1 = emfc->mgrp_fdb_ipv6[i].next;
				ptr1 != &emfc->mgrp_fdb_ipv6[i]; ptr1 = ptr1->next)
		{
			mgrp = clist_entry(ptr1, emfc_mgrp_t, mgrp_hlist);
			EMF_DEBUG("emfc:%p, mgrp:%p, mgrp->mi_head:%p, mgrp->mi_head.next:%p\n",
					emfc, mgrp, &mgrp->mi_head, mgrp->mi_head.next);
			EMF_DEBUG("Multicast Group entry %pI6 found\n", mgrp->mgrp_ipv6.s6_addr32);
			for (ptr2 = mgrp->mi_head.next;
					ptr2 != &mgrp->mi_head; ptr2 = ptr2->next)
			{
				mi = clist_entry(ptr2, emfc_mi_t, mi_list);
				EMF_DEBUG("SCB index:%d, ptr2:%p:\n", index, ptr2);
				index++;
			}
		}
	}

	/* Update the total number of entries */
	list->num_entries = index;
	return (SUCCESS);
}

static emfc_mi_t *
emfc_mfdb_mi_entry_ipv6_find(emfc_info_t *emfc, emfc_mgrp_t *mgrp, void *ifp)
{
	emfc_mi_t *mi;
	clist_head_t *ptr;

	ASSERT(mgrp);

	for (ptr = mgrp->mi_head.next;
			ptr != &mgrp->mi_head; ptr = ptr->next)
	{
		mi = clist_entry(ptr, emfc_mi_t, mi_list);
		if (ifp == mi->mi_mhif->mhif_ifp)
		{
			return (mi);
		}
	}

	return (NULL);
}

/*
 * Description: This function does the MFDB lookup to locate an ipv6 multicast
 *				group entry.
 *
 * Input:		emfc	- EMFL Common code global data handle
 *				mgrp_ipv6 - Multicast group address of the entry.
 *
 * Return:		Returns NULL is no group entry is found. Otherwise
 *				returns pointer to the MFDB group entry.
 */

static emfc_mgrp_t * emfc_mfdb_group_ipv6_lookup(emfc_info_t *emfc, struct ipv6_addr mgrp_ipv6)
{
	uint32 hash;
	emfc_mgrp_t *mgrp;
	clist_head_t *ptr;

	/* Do the cache lookup first. Since the multicast video traffic
	 * is bursty in nature there is a good chance that the cache
	 * hit ratio will be good. If during the testing we find that
	 * the hit ratio is not as good then this single entry cache
	 * mechanism will be removed.
	 */
	if (ipv6_is_same(mgrp_ipv6, emfc->mgrp_cache_ipv6_addr))
	{
		EMFC_STATS_INCR_IPV6(emfc, mfdb_cache_hits);
		return (emfc->mgrp_cache_ipv6_grp);
	}

	EMFC_STATS_INCR_IPV6(emfc, mfdb_cache_misses);

	hash = MFDB_MGRP_HASH_IPV6(mgrp_ipv6);
	EMF_DEBUG(":%s:%d  Hash values is:%d \r\n", __FUNCTION__, __LINE__, hash);

	for (ptr = emfc->mgrp_fdb_ipv6[hash].next;
			ptr != &emfc->mgrp_fdb_ipv6[hash];
			ptr = ptr->next)
	{
		mgrp = clist_entry(ptr, emfc_mgrp_t, mgrp_hlist);
		/* Compare group address */
		if (ipv6_is_same(mgrp_ipv6, mgrp->mgrp_ipv6))
		{
			EMF_DEBUG("Multicast Group entry %pI6 found\n", mgrp_ipv6.s6_addr32);
			emfc->mgrp_cache_ipv6_grp = mgrp;
			memcpy(&emfc->mgrp_cache_ipv6_addr, &mgrp_ipv6, sizeof(struct ipv6_addr));
			return (mgrp);
		}
	}

	return (NULL);
}

uint32 emfc_ipv6_input(emfc_info_t *emfc, void *sdu, void *ifp, uint8 *iph, bool rt_port)
{

	struct ipv6_hdr *hdr = (struct ipv6_hdr *)iph;
	int dst_type;
	uint32 dest_ip;
	u8 *nextHdr = NULL;
	u8 snooping_enabled = 0;
	struct net_device *dev;
	//void *dev;
	if (emfc->wrapper.hooks_get_fn) {
		 dev = emfc->wrapper.hooks_get_fn(WLEMF_CMD_GETDEV, (void *)emfc->emfi, NULL, NULL);
	} else
		 return (EMF_NOP);

	if (likely(dev != NULL))
	{
		snooping_enabled = bcm_mcast_is_snooping_enabled(dev, BCM_MCAST_PROTO_IPV6);
		EMF_DEBUG(" mld is enabled:%d\n", snooping_enabled);
	} else
		return (EMF_NOP);

	dst_type = ipv6_type(&hdr->daddr);
	if (!(dst_type&IPV6_ADDR_MULTICAST) || (!emfc->emf_enable))
	{
		EMF_DEBUG("IPv6 Unicast frame recevied/EMF disabled\n");
		return (EMF_NOP);
	}
	EMF_DEBUG("Received frame with dest ip %pI6\n", (hdr->daddr.s6_addr32));

	dest_ip = ntoh32(*((uint32 *)(iph + IPV4_DEST_IP_OFFSET)));

	nextHdr = (u8 *)((u8*)hdr + sizeof(struct ipv6_hdr));
	/* Check the protocol type of multicast frame */
	if ((hdr->nexthdr == IPPROTO_HOPOPTS) &&
			(*nextHdr == IPPROTO_ICMPV6) &&
			(emfc->snooper != NULL))
	{
		EMF_DEBUG("Received MLD frame type %d\n", *(iph + IPV4_HLEN(iph)));
		EMFC_STATS_INCR_IPV6(emfc, igmp_frames);
		//All IPV6 control message will go through regular handling flow.
		//get ignored in EMF module. IPv6 EMF module has no interests on
		//control message as it depends on MCPD.
		return EMF_NOP;

	}
	else
	{
		clist_head_t *ptr;
		emfc_mgrp_t *mgrp;
		emfc_mi_t *mi;
		void *sdu_clone;

		if (!bcm_mcast_control_filter(&hdr->daddr, BCM_MCAST_PROTO_IPV6)) {
			EMF_DEBUG("LinkLocal-nodelocal etc to be flooded\n");
			return EMF_NOP;
		} else if (!snooping_enabled) {
			//snooping is not enabled for this bridge, but WMF
			//is endabled, we will unicast it to all
			//the assoicated
			/* rt_port bit 7 will indicate it should forward it to all port */
			int ret = emfc->wrapper.forward_fn(emfc->emfi, sdu, 0, ifp,
					rt_port | 0x80);
			if (ret != EMF_NOP) return ret;
		} else {
			//for now, we will unicast the packet to all other STAs
			if ((mgrp = emfc_mfdb_group_ipv6_lookup(emfc, hdr->daddr)) != NULL) {
				EMF_DEBUG("emfc:%p, mgrp at :%p and mgrp_mi_head.next:%p,
						mgrp->mi_head:%p\n", emfc, mgrp, mgrp->mi_head.next,
						&mgrp->mi_head);
				for (ptr = mgrp->mi_head.next; ptr != &mgrp->mi_head;
						ptr = ptr->next) {

					if (!ptr) break;
					mi = clist_entry(ptr, emfc_mi_t, mi_list);
					if (((ifp != NULL)&&(ifp == mi->mi_mhif->mhif_ifp)))
						continue;

					else {
						if ((ptr->next == &mgrp->mi_head)&&rt_port) {
							emfc->wrapper.forward_fn(
								emfc->emfi, sdu, 0,
								mi->mi_mhif->mhif_ifp,
								rt_port);
							return (EMF_TAKEN);
						} else {
							if ((sdu_clone = EMFC_PKTDUP(emfc, sdu))
									== NULL) {
								EMFC_STATS_INCR_IPV6(emfc,
									mcast_data_dropped);
								return (EMF_DROP);
							}
							EMF_DEBUG("Clone skb and send\n");
							emfc->wrapper.forward_fn(emfc->emfi,
								sdu_clone, 0,
								mi->mi_mhif->mhif_ifp, rt_port);
						}
					}

				}
			} else if (rt_port)
				return EMF_DROP;
		}
	}
	return EMF_NOP;
}

/*
 * Add the entry if not present otherwise return the pointer to
 * the entry.
 */
emfc_mhif_t * emfc_mfdb_mhif_add_ipv6(emfc_info_t *emfc, void *ifp)
{
	emfc_mhif_t *ptr, *mhif;

	for (ptr = emfc->mhif_head_ipv6; ptr != NULL; ptr = ptr->next)
	{
		if (ptr->mhif_ifp == ifp)
		{
			ptr->mhif_ref++;
			return (ptr);
		}
	}

	/* Add new entry */
	mhif = MALLOC(emfc->osh, sizeof(emfc_mhif_t));
	if (mhif == NULL)
	{
		EMF_ERROR("Failed to alloc mem size %d for mhif entry\n",
				(int)sizeof(emfc_mhif_t));
		return (NULL);
	}

	mhif->mhif_ref = 1;
	mhif->mhif_ifp = ifp;
	mhif->mhif_data_fwd = 0;
	mhif->prev = mhif;
	mhif->next = emfc->mhif_head_ipv6;
	if (emfc->mhif_head_ipv6)
		emfc->mhif_head_ipv6->prev = mhif;
	emfc->mhif_head_ipv6 = mhif;

	return (mhif);
}

int32 emfc_mfdb_ipv6_membership_add(emfc_info_t *emfc, void *mgrp_ip6, void *ifp)
{
	uint32 hash;
	emfc_mgrp_t *mgrp;
	emfc_mi_t *mi;
	struct ipv6_addr *mgrp_ipv6 = (struct ipv6_addr*)mgrp_ip6;

	OSL_LOCK(emfc->fdb_lock_ipv6);

	/* If the group entry doesn't exist, add a new entry and update
	 * the member/interface information.
	 */
	mgrp = emfc_mfdb_group_ipv6_lookup(emfc, *mgrp_ipv6);

	if (mgrp == NULL)
	{
		/* Allocate and initialize multicast group entry */
		mgrp = MALLOC(emfc->osh, sizeof(emfc_mgrp_t));
		if (mgrp == NULL)
		{
			EMF_ERROR("Failed to alloc mem size %d for group entry\n",
					(int)sizeof(emfc_mgrp_t));
			OSL_UNLOCK(emfc->fdb_lock_ipv6);
			return (FAILURE);
		}

		memcpy(&mgrp->mgrp_ipv6, mgrp_ipv6, sizeof(struct ipv6_addr));
		clist_init_head(&mgrp->mi_head);

		EMF_DEBUG("Adding ipv6 group entry %pI6\n", (mgrp_ipv6->s6_addr32));

		/* Add the group entry to hash table */
		hash = MFDB_MGRP_HASH_IPV6(*mgrp_ipv6);
		clist_add_head(&emfc->mgrp_fdb_ipv6[hash], &mgrp->mgrp_hlist);
	}
	else
	{
		mi = emfc_mfdb_mi_entry_ipv6_find(emfc, mgrp, ifp);
		EMF_DEBUG(":%s:%d  find mi \r\n", __FUNCTION__, __LINE__);
		/* Update the ref count */
		if (mi != NULL)
		{
			mi->mi_ref++;
			OSL_UNLOCK(emfc->fdb_lock_ipv6);
			return (SUCCESS);
		}
	}

	EMF_MFDB("Adding interface entry for interface %p\n", ifp);

	/* Allocate and initialize multicast interface entry */
	mi = MALLOC(emfc->osh, sizeof(emfc_mi_t));
	if (mi == NULL)
	{
		EMF_ERROR("Failed to allocated memory %d for interface entry\n",
				(int)sizeof(emfc_mi_t));
		if (clist_empty(&mgrp->mi_head))
		{
			clist_delete(&mgrp->mgrp_hlist);
			if (emfc->mgrp_cache_ipv6_grp != NULL &&
					(emfc->mgrp_cache_ipv6_grp == mgrp))
			{
				emfc->mgrp_cache_ipv6_grp = NULL;
				memset(&emfc->mgrp_cache_ipv6_addr, 0, sizeof(struct ipv6_addr));
			}
			MFREE(emfc->osh, mgrp, sizeof(emfc_mgrp_t));
		}
		OSL_UNLOCK(emfc->fdb_lock_ipv6);
		return (FAILURE);
	}
	/* Initialize the multicast interface list entry */
	mi->mi_ref = 1;
	mi->mi_mhif = emfc_mfdb_mhif_add_ipv6(emfc, ifp);
	mi->mi_data_fwd = 0;

	/* Add the multicast interface entry */
	clist_add_head(&mgrp->mi_head, &mi->mi_list);

	OSL_UNLOCK(emfc->fdb_lock_ipv6);
	//emfc_mfdb_list_ipv6(emfc, emfc, 3);
	return (SUCCESS);
}

int32
emfc_mfdb_ipv6_membership_del(emfc_info_t *emfc, void *mgrp_ip6, void *ifp)
{
	emfc_mi_t *mi;
	emfc_mgrp_t *mgrp;
	struct ipv6_addr *mgrp_ipv6 = (struct ipv6_addr*)mgrp_ip6;
	OSL_LOCK(emfc->fdb_lock_ipv6);

	/* Find group entry */
	mgrp = emfc_mfdb_group_ipv6_lookup(emfc, *mgrp_ipv6);

	if (mgrp == NULL)
	{
		OSL_UNLOCK(emfc->fdb_lock_ipv6);
		return (FAILURE);
	}

	/* Find interface entry */
	mi = emfc_mfdb_mi_entry_ipv6_find(emfc, mgrp, ifp);

	if (mi == NULL)
	{
		OSL_UNLOCK(emfc->fdb_lock_ipv6);
		return (FAILURE);
	}

	EMF_MFDB("Deleting MFDB interface entry for interface %p\n", ifp);

	/* Delete the interface entry when ref count reaches zero */
	mi->mi_ref--;
	EMF_DEBUG(":%s:%d  mi->ref:%d \r\n", __FUNCTION__, __LINE__, mi->mi_ref);
	EMF_MFDB("Deleting interface entry %p\n", mi->mi_mhif->mhif_ifp);
	clist_delete(&mi->mi_list);

	/* If the member being deleted is last node in the interface list,
	 * delete the group entry also.
	 */
	if (clist_empty(&mgrp->mi_head))
	{
		EMF_DEBUG("Deleting group entry \n");
		clist_delete(&mgrp->mgrp_hlist);
		memset(&emfc->mgrp_cache_ipv6_addr, 0, sizeof(struct ipv6_addr));
		MFREE(emfc->osh, mgrp, sizeof(emfc_mgrp_t));
	}

	mi->mi_mhif->mhif_ref--;
	if (mi->mi_mhif->mhif_ref == 0)  {
		if (mi->mi_mhif == emfc->mhif_head_ipv6) {
			emfc->mhif_head_ipv6 = mi->mi_mhif->next;
		}
		else {
			mi->mi_mhif->prev->next = mi->mi_mhif->next;
			if (mi->mi_mhif->next)
				mi->mi_mhif->next->prev = mi->mi_mhif->prev;
		}
		MFREE(emfc->osh, mi->mi_mhif, sizeof(emfc_mhif_t));
	}

	MFREE(emfc->osh, mi, sizeof(emfc_mi_t));

	OSL_UNLOCK(emfc->fdb_lock_ipv6);

	return (SUCCESS);
}

int32 emfc_mfdb_ipv6_membership_dev_del(void *dev, void *grp, void *ifp)
{

	emfc_info_t *emfc = emfc_instance_find(((struct net_device *)dev)->name);
	if (emfc)
		return emfc_mfdb_ipv6_membership_del(emfc, grp, ifp);
	else
		return (FAILURE);
}
