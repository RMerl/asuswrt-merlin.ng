/*
 * This file contains the common code routines to access/update the
 * IGMP Snooping database.
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
 * $Id: igsc_sdb_ipv6.c 771177 2019-01-17 06:59:30Z $
 */
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <ethernet.h>
#include <bcmip.h>
#include <bcmipv6.h>
#include <osl.h>
#include <clist.h>
#if defined(linux)
#include <osl_linux.h>
#else /* defined(osl_xx) */
#error "Unsupported osl"
#endif /* defined(osl_xx) */
#include "emfc_export.h"
#include "igs_cfg.h"
#include "igsc_export.h"
#include "igsc.h"
#include "igsc_sdb.h"
static int32 _igsc_sdb_interface_del_ipv6(igsc_info_t *igsc_info, void *ifp);
/*
 * Description: This function deletes the host entry of IGSDB. It also
 *				deletes the corresponding interfaces and group entries
 *				if this is the last member of the group.
 *
 * Input:		igsc_info - IGSL Common code global data handle
 *				mh		  - Pointer to Multicast host entry of IGSDB
 * return:		0 - when group is not deleted, only mh is delete
 *				1 - when group list is also deleted.
 */
static int
igsc_sdb_mh_delete_ipv6(igsc_info_t *igsc_info, igsc_mh_t *mh)
{
	int ret = 0;
	/* Delete the interface entry if no stream is going on it */
	if (--mh->mh_mi->mi_ref == 0)
	{
		IGS_DEBUG("Deleting interface entry %p\n", mh->mh_mi);

		/* Delete the MFDB entry if no more members of the group
		 * are present on the interface.
		 */
		if (emfc_mfdb_ipv6_membership_del(igsc_info->emf_handle,
				&(mh->mh_mgrp->mgrp_ipv6),
				mh->mh_mi->mi_ifp) != SUCCESS)
		{
			IGS_ERROR("Membership entry %p delete failed\n", mh->mh_mi->mi_ifp);
			return ret;
		}

		clist_delete(&mh->mh_mi->mi_list);
		MFREE(igsc_info->osh, mh->mh_mi, sizeof(igsc_mi_t));
	}

	/* Delete the host entry */
	clist_delete(&mh->mh_list);
	IGSC_STATS_IPV6_DECR(igsc_info, igmp_mcast_members);

	/* If the member being deleted is last node in the host list,
	 * delete the group entry also.
	 */
	if (clist_empty(&mh->mh_mgrp->mh_head))
	{
		IGS_IGSDB("Deleting group entry of %pI6c too\n", &(mh->mh_mgrp->mgrp_ipv6));

		clist_delete(&mh->mh_mgrp->mgrp_hlist);
		MFREE(igsc_info->osh, mh->mh_mgrp, sizeof(igsc_mgrp_t));
		IGSC_STATS_IPV6_DECR(igsc_info, igmp_mcast_groups);
		ret = 1;
	}

	MFREE(igsc_info->osh, mh, sizeof(igsc_mh_t));

	return ret;
}

/*
 * Description: This function does the IGSDB lookup to locate a multicast
 *				group entry.
 *
 * Input:		mgrp_ip - Multicast group address of the entry.
 *
 * Return:		Returns NULL is no group entry is found. Otherwise
 *				returns pointer to the IGSDB group entry.
 */
static igsc_mgrp_t *
igsc_sdb_group_find_ipv6(igsc_info_t *igsc_info, struct ipv6_addr mgrp_ip)
{
	uint32 hash;
	igsc_mgrp_t *mgrp;
	clist_head_t *ptr;

	hash = IGSDB_MGRP_HASH_IPV6(mgrp_ip);

	for (ptr = igsc_info->mgrp_sdb_ipv6[hash].next;
			ptr != &igsc_info->mgrp_sdb_ipv6[hash]; ptr = ptr->next)
	{
		mgrp = clist_entry(ptr, igsc_mgrp_t, mgrp_hlist);

		/* Compare group address */
		if (ipv6_is_same(mgrp_ip, mgrp->mgrp_ipv6))
		{
			IGS_IGSDB("Multicast Group entry %pI6c\n", &(mgrp_ip));

			return (mgrp);
		}
	}

	return (NULL);
}

/*
 * Description: This fnction does the MFDB lookup to locate host entry
 *				of the specified group.
 *
 * Input:		igsc_info - IGMP Snooper instance handle.
 *				mgrp	  - Pointer to multicast group entry of IGSDB
 *				mh_ip	  - Member IP Address to find
 *				ifp		  - Pointer to the member interface.
 *
 * Return:		Returns pointer to host entry or NULL.
 */
static igsc_mh_t *
igsc_sdb_mh_entry_find_ipv6(igsc_info_t *igsc_info, igsc_mgrp_t *mgrp,
		struct ipv6_addr mh_ip, void *ifp)
{
	igsc_mh_t *mh;
	clist_head_t *ptr;

	for (ptr = mgrp->mh_head.next;
			ptr != &mgrp->mh_head; ptr = ptr->next)
	{
		mh = clist_entry(ptr, igsc_mh_t, mh_list);
		if (ipv6_is_same(mh_ip, mh->mh_ipv6) && (ifp == mh->mh_mi->mi_ifp))
		{
			return (mh);
		}
	}

	return (NULL);
}

static igsc_mi_t *
igsc_sdb_mi_entry_find_ipv6(igsc_info_t *igsc_info, igsc_mgrp_t *mgrp, void *ifp)
{
	igsc_mi_t *mi;
	clist_head_t *ptr;

	for (ptr = mgrp->mi_head.next;
			ptr != &mgrp->mi_head; ptr = ptr->next)
	{
		mi = clist_entry(ptr, igsc_mi_t, mi_list);
		if (ifp == mi->mi_ifp)
		{
			return (mi);
		}
	}

	return (NULL);
}

/*
 * Description: This function does the IGSDB lookup to locate a host
 *				entry of the specified multicast group.
 *
 * Input:		igsc_info - IGMP Snooper instance handle.
 *				mgrp_ip   - Multicast group IP address of the entry.
 *				mh_ip	  - Multicast host address.
 *				ifp		  - Pointer to the interface on which the member
 *							is present.
 *
 * Return:		Returns NULL is no host entry is found. Otherwise
 *				returns pointer to the IGSDB host entry.
 */
static igsc_mh_t *
igsc_sdb_member_find_ipv6(igsc_info_t *igsc_info, struct ipv6_addr mgrp_ip,
		struct ipv6_addr mh_ip, void *ifp)
{
	uint32 hash;
	igsc_mgrp_t *mgrp;
	igsc_mh_t *mh;

	hash = IGSDB_MGRP_HASH_IPV6(mgrp_ip);
	mgrp = igsc_sdb_group_find_ipv6(igsc_info, mgrp_ip);
	if (mgrp != NULL)
	{
		mh = igsc_sdb_mh_entry_find_ipv6(igsc_info, mgrp, mh_ip, ifp);
		if (mh != NULL)
		{
			IGS_IGSDB(" HOST ENTRY of %pI6c -- %pI6c found\n", &mgrp_ip, &mh_ip);
			return (mh);
		}
	}
	IGS_IGSDB(" HOST ENTRY of %pI6c -- %pI6c found\n", &mgrp_ip, &mh_ip);
	return (NULL);
}

/*
 * Description: This function is called by IGMP Snooper when it wants
 *				to add IGSDB entry or refresh the entry. This function
 *				is also called by the management application to add a
 *				static IGSDB entry.
 *
 *				If the IGSDB entry is not present, it allocates group
 *				entry, host entry, interface entry and links them
 *				together. Othewise it just updates the timer for the
 *				matched host entry.
 *
 * Input:		mgrp_ip - Multicast group IP address of the entry.
 *				mh_ip	- Multicast host address. When adding static
 *						  entry this parameter is zero.
 *				ifp		- Pointer to the interface on which the member
 *						  is present.
 *
 * Return:		SUCCESS or FAILURE
 */
int32
igsc_sdb_member_add_ipv6(igsc_info_t *igsc_info, void *ifp, struct ipv6_addr mgrp_ip,
		struct ipv6_addr mh_ip)
{
	igsc_mgrp_t *mgrp;
	int32 hash, ret;
	igsc_mh_t *mh;
	igsc_mi_t *mi;
	OSL_LOCK(igsc_info->sdb_lock_ipv6);

	mgrp = igsc_sdb_group_find_ipv6(igsc_info, mgrp_ip);

	if (mgrp == NULL)
	{
		mgrp = MALLOC(igsc_info->osh, sizeof(igsc_mgrp_t));
		if (mgrp == NULL)
		{
			IGS_ERROR("Failed to alloc size %d for IGSDB group entry\n",
					(int)sizeof(igsc_mgrp_t));
			goto sdb_add_exit0;
		}

		memcpy(&mgrp->mgrp_ipv6, &mgrp_ip, sizeof(struct ipv6_addr));
		clist_init_head(&mgrp->mh_head);
		clist_init_head(&mgrp->mi_head);
		mgrp->igsc_info = igsc_info;

		IGS_IGSDB("Adding group entry %pI6c\n", &mgrp_ip);

		/* Add the group entry to hash table */
		hash = IGSDB_MGRP_HASH_IPV6(mgrp_ip);
		clist_add_head(&igsc_info->mgrp_sdb_ipv6[hash], &mgrp->mgrp_hlist);

		IGSC_STATS_IPV6_INCR(igsc_info, igmp_mcast_groups);
	}
	else
	{
		IGS_IGSDB("Refreshing FDB entry for group %pI6c--- %pI6c", &mgrp_ip, &mh_ip);

		/* Avoid adding duplicate entries */
		mh = igsc_sdb_mh_entry_find_ipv6(igsc_info, mgrp, mh_ip, ifp);
		if (mh != NULL)
		{
			mh->missed_report_cnt = 0;
			IGS_IGSDB("Found the station and update the Timer Done!!!!!\n");
			OSL_UNLOCK(igsc_info->sdb_lock_ipv6);
			return (SUCCESS);
		}
	}

	/* Allocate and initialize multicast host entry */
	mh = MALLOC(igsc_info->osh, sizeof(igsc_mh_t));
	if (mh == NULL)
	{
		IGS_ERROR("Failed to allocated memory size %d for IGSDB host entry\n",
				(int)sizeof(igsc_mh_t));
		goto sdb_add_exit1;
	}

	/* Initialize the host entry */
	memcpy(&mh->mh_ipv6, &mh_ip, sizeof(struct ipv6_addr));
	mh->mh_mgrp = mgrp;
	mh->missed_report_cnt = 0;

	IGS_IGSDB("Adding host entry %pI6c\n", &mh_ip);

	IGSC_STATS_IPV6_INCR(igsc_info, igmp_mcast_members);

	IGS_IGSDB("Added timer fo this SCB done!\n");
	/* Add the host entry to the group */
	clist_add_head(&mgrp->mh_head, &mh->mh_list);

	/* Avoid adding duplicate interface list entries */
	mi = igsc_sdb_mi_entry_find_ipv6(igsc_info, mgrp, ifp);
	if (mi != NULL)
	{
		/* Link the interface list entry */
		mh->mh_mi = mi;

		/* Increment ref count indicating a new reference from
		 * host entry.
		 */
		ASSERT(mi->mi_ref > 0);
		mi->mi_ref++;
		OSL_UNLOCK(igsc_info->sdb_lock_ipv6);
		return (SUCCESS);
	}
	ret = emfc_mfdb_ipv6_membership_add(igsc_info->emf_handle, &mgrp->mgrp_ipv6, ifp);
	if (ret != SUCCESS)
	{
		IGS_ERROR("Failed to add MFDB entry for %p\n", ifp);
		goto sdb_add_exit3;
	}

	/* Allocate and initialize multicast interface entry */
	mi = MALLOC(igsc_info->osh, sizeof(igsc_mi_t));
	if (mi == NULL)
	{
		IGS_ERROR("Failed to allocated memory size %d for interface entry\n",
				(int)sizeof(igsc_mi_t));
		goto sdb_add_exit4;
	}

	/* Initialize the multicast interface list entry */
	mi->mi_ifp = ifp;
	mi->mi_ref = 1;

	IGS_IGSDB("Adding interface entry for interface %p\n", mi->mi_ifp);

	/* Add the multicast interface entry */
	clist_add_head(&mgrp->mi_head, &mi->mi_list);

	/* Link the interface list entry to host entry */
	mh->mh_mi = mi;

	OSL_UNLOCK(igsc_info->sdb_lock_ipv6);

	return (SUCCESS);

sdb_add_exit4:
	emfc_mfdb_ipv6_membership_del(igsc_info->emf_handle, (void *)&(mgrp->mgrp_ipv6), ifp);
sdb_add_exit3:
	clist_delete(&mh->mh_list);
	MFREE(igsc_info->osh, mh, sizeof(igsc_mh_t));
sdb_add_exit1:
	if (clist_empty(&mgrp->mh_head))
	{
		clist_delete(&mgrp->mgrp_hlist);
		MFREE(igsc_info->osh, mgrp, sizeof(igsc_mgrp_t));
	}
sdb_add_exit0:
	OSL_UNLOCK(igsc_info->sdb_lock_ipv6);
	return (FAILURE);
}

/*
 * Description: This function is called by the IGMP snooper layer
 *				to delete the IGSDB host entry. It deletes the group
 *				entry also if the host entry is last in the group.
 *
 * Input:		Same as above function.
 *
 * Return:		SUCCESS or FAILURE
 */
int32
igsc_sdb_member_del_ipv6(igsc_info_t *igsc_info, void *ifp, struct ipv6_addr mgrp_ip,
		struct ipv6_addr mh_ip)
{
	int32 ret = FAILURE;
	igsc_mh_t *mh;
	OSL_LOCK(igsc_info->sdb_lock_ipv6);
	mh = igsc_sdb_member_find_ipv6(igsc_info, mgrp_ip, mh_ip, ifp);
	if (mh != NULL) {
		igsc_sdb_mh_delete_ipv6(igsc_info, mh);
		IGS_IGSDB("Deleteing host entry %pI6c\n", &mh_ip);
		ret = SUCCESS;
	}
	OSL_UNLOCK(igsc_info->sdb_lock_ipv6);
	return ret;
}

int32 igsc_sdb_clear_group_ipv6(igsc_info_t *igsc_info, struct ipv6_addr *grp)
{
	igsc_mgrp_t *mgrp;
	igsc_mh_t *mh;
	clist_head_t  *ptr2, *tmp2;
	OSL_LOCK(igsc_info->sdb_lock_ipv6);
	mgrp = igsc_sdb_group_find_ipv6(igsc_info, *grp);
	if (mgrp)
	{
		/* Delete all host entries */
		for (ptr2 = mgrp->mh_head.next; ptr2 != &mgrp->mh_head; ptr2 = tmp2)
		{
			tmp2 = ptr2->next;
			mh = clist_entry(ptr2, igsc_mh_t, mh_list);
			if (mh) {
				/* Delete the interface entry if no stream is going on it */
				if (--mh->mh_mi->mi_ref == 0)
				{
					IGS_IGSDB("Deleting interface entry %p\n", mh->mh_mi);

					/* Delete the MFDB entry if no more members of the group
					 * are present on the interface.
					 */
					emfc_mfdb_ipv6_membership_del(igsc_info->emf_handle,
						&(mh->mh_mgrp->mgrp_ipv6), mh->mh_mi->mi_ifp);
					clist_delete(&mh->mh_mi->mi_list);
					MFREE(igsc_info->osh, mh->mh_mi, sizeof(igsc_mi_t));
				}
				MFREE(igsc_info->osh, mh, sizeof(igsc_mh_t));

			}
			clist_delete(ptr2);
			IGS_IGSDB("%s:%d  clear STA entry.. \r\n", __FUNCTION__, __LINE__);
		}
		if (clist_empty(&mgrp->mh_head))
		{
			clist_delete(&mgrp->mgrp_hlist);
			MFREE(igsc_info->osh, mgrp, sizeof(igsc_mgrp_t));
		}
		OSL_UNLOCK(igsc_info->sdb_lock_ipv6);
		return (SUCCESS);
	}
	OSL_UNLOCK(igsc_info->sdb_lock_ipv6);
	return (FAILURE);
}
/*
 * Description: This function clears the group interval timers and
 *				deletes the group, host and interface entries of the
 *				IGSDB.
 */
void igsc_sdb_clear_ipv6(igsc_info_t *igsc_info)
{
	return _igsc_sdb_interface_del_ipv6(igsc_info, NULL);

}

/*
 * IGSDB Listing Function
 */
int32
igsc_sdb_list_ipv6(igsc_info_t *igsc_info, igs_cfg_sdb_list_t *list, uint32 size)
{
	clist_head_t *ptr1, *ptr2;
	igsc_mh_t *mh;
	igsc_mgrp_t *mgrp;
	int32 i, index = 0;

	if (igsc_info == NULL)
	{
		IGS_ERROR("Invalid IGSC handle passed\n");
		return (FAILURE);
	}

	if (list == NULL)
	{
		IGS_ERROR("Invalid buffer input\n");
		return (FAILURE);
	}

	for (i = 0; i < IGSDB_HASHT_SIZE; i++)
	{
		for (ptr1 = igsc_info->mgrp_sdb_ipv6[i].next;
				ptr1 != &igsc_info->mgrp_sdb_ipv6[i];
				ptr1 = ptr1->next)
		{
			mgrp = clist_entry(ptr1, igsc_mgrp_t, mgrp_hlist);
			for (ptr2 = mgrp->mh_head.next;
					ptr2 != &mgrp->mh_head; ptr2 = ptr2->next)
			{
				mh = clist_entry(ptr2, igsc_mh_t, mh_list);
				memcpy(&list->sdb_entry[index].mgrp_ipv6, &mgrp->mgrp_ipv6,
						sizeof(struct ipv6_addr));
				memcpy(&list->sdb_entry[index].mh_ipv6, &mh->mh_ipv6,
						sizeof(struct ipv6_addr));
				index++;
			}
		}
	}

	list->num_entries = index;

	return (SUCCESS);
}

int32
igsc_sdb_interface_del_ipv6(igsc_info_t *igsc_info, void *ifp)
{
	if (ifp == NULL)
	{
		IGS_ERROR("Invalid ifp passed in\n");
		return (FAILURE);
	}
	return _igsc_sdb_interface_del_ipv6(igsc_info, ifp);
}

/* * Description: This function is called to delete the IGSDB
 *				an interface
 *
 * Input:		igsc_info - igsc info pointer
 *		ifp		  - interface pointer
 *
 * Return:		SUCCESS or FAILURE
 */
static int32
_igsc_sdb_interface_del_ipv6(igsc_info_t *igsc_info, void *ifp)
{
	uint32 i;
	igsc_mgrp_t *mgrp;
	igsc_mh_t *mh = NULL;
	clist_head_t *ptr1, *ptr2, *tmp1, *tmp2;
	int ret = SUCCESS;

	if (igsc_info == NULL)
	{
		IGS_ERROR("Invalid IGSC handle passed\n");
		return (FAILURE);
	}

	OSL_LOCK(igsc_info->sdb_lock_ipv6);
	for (i = 0; i < IGSDB_HASHT_SIZE; i++)
	{
		for (ptr1 = igsc_info->mgrp_sdb_ipv6[i].next;
				ptr1 != &igsc_info->mgrp_sdb_ipv6[i]; ptr1 = tmp1)
		{
			tmp1 = ptr1->next;
			mgrp = clist_entry(ptr1, igsc_mgrp_t, mgrp_hlist);

			for (ptr2 = mgrp->mh_head.next; ptr2 != &mgrp->mh_head; ptr2 = tmp2)
			{
				tmp2 = ptr2->next;
				mh = clist_entry(ptr2, igsc_mh_t, mh_list);

				if (ifp && mh->mh_mi->mi_ifp != ifp) continue;

				igsc_sdb_mh_delete_ipv6(igsc_info, mh);
			}

		}
	}

	OSL_UNLOCK(igsc_info->sdb_lock_ipv6);

	return (ret);
}
