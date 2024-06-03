/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/****************************************************************************

	Abstract:

	All related IEEE802.11f IAPP + IEEE802.11r/11k IAPP extension.

***************************************************************************/

/* Include */

#include "rt_config.h"
#include "rtmpiapp.h"
#include "iappdefs.h"
#include "debug.h"

VOID os_alloc_mem(
	UCHAR *pAd,
	UCHAR **ppMem,
	UINT32 Size)
{
	*ppMem = (UCHAR *)malloc(Size);
}

VOID os_free_mem(UCHAR *pAd, VOID *pMem)
{
	free(pMem);
}

BOOLEAN mt_iapp_get_wifi_iface_mac(
	RTMP_IAPP		*pCtrlBK)
{
#ifdef IAPP_OS_LINUX
	INT idx;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	for (idx = 0; idx < pCtrlBK->IfNameWlanCount; idx ++) {
		struct ifreq		ReqIf;

		NdisZeroMemory(pCtrlBK->IfNameWlanMAC[idx], ETH_ALEN);
		NdisCopyMemory(ReqIf.ifr_name, pCtrlBK->IfNameWlanIoctl[idx], IFNAMSIZ);
		/* get mac address of the interface */
		if (ioctl(pCtrlBK->SocketIoctl, SIOCGIFHWADDR, &ReqIf) < 0) {
			DBGPRINT(RT_DEBUG_ERROR, "iapp> %s - Fail to get MAC of IfName[%d]: %s\n",
				__FUNCTION__, idx, pCtrlBK->IfNameWlanIoctl[idx]);
			return FALSE;
		}
		else {
			NdisCopyMemory(pCtrlBK->IfNameWlanMAC[idx], &ReqIf.ifr_ifru.ifru_hwaddr.sa_data[0], ETH_ALEN);
			DBGPRINT(RT_DEBUG_OFF, "iapp> %s - IfName[%d]: %s\n",
				__FUNCTION__, idx, pCtrlBK->IfNameWlanIoctl[idx]);
			IAPP_HEX_DUMP("MAC", pCtrlBK->IfNameWlanMAC[idx], ETH_ALEN);
		}
	}
	return TRUE;
#else
	return FALSE;
#endif
}

INT32 mt_iapp_find_ifidx_by_mac(
	RTMP_IAPP		*pCtrlBK,
	UCHAR			*WifiMAC)
{
	INT32 idx = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (WifiMAC == NULL) {
		DBGPRINT(RT_DEBUG_OFF, "iapp> %s - WifiMAC is null.\n", __FUNCTION__);
		return -1;
	}

	for (idx = 0; idx < pCtrlBK->IfNameWlanCount; idx++) {
		if (NdisCompareMemory(WifiMAC, pCtrlBK->IfNameWlanMAC[idx], ETH_ALEN) == 0) {
			return idx;
		}
	}

	return -1;
}

VOID mt_iapp_set_daemon_information(
	RTMP_IAPP		*pCtrlBK,
	pid_t 			*pPidAuth)
{
	INT32 ComLen = 0;
	INT32 idx = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	for (idx = 0; idx < pCtrlBK->IfNameWlanCount; idx++) {
		ComLen = sizeof(INT32);
		IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP,
						pPidAuth, &ComLen, idx,
						RT_SET_IAPP_PID | OID_GET_SET_TOGGLE);
#ifdef FT_KDP_KEY_FROM_DAEMON
		ComLen = strlen(pCtrlBK->CommonKey);
		IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP,
							pCtrlBK->CommonKey, &ComLen, idx,
							RT_FT_KEY_SET | OID_GET_SET_TOGGLE);
#endif /* FT_KDP_KEY_FROM_DAEMON */
	}
	return;
}

VOID mt_iapp_ft_client_table_init(
	RTMP_IAPP		*pCtrlBK)
{
	INT i;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	NdisZeroMemory(&pCtrlBK->SelfFtStaTable, sizeof(pCtrlBK->SelfFtStaTable));
	for (i=0; i< HASH_TABLE_SIZE; i++)
		pCtrlBK->SelfFtStaTable.hash[i] = NULL;
	return;
}

FT_CLIENT_INFO *mt_iapp_ft_client_look_up(
	FT_CLIENT_TABLE		*pFtTable,
	UCHAR *pAddr)
{
	ULONG HashIdx;
	FT_CLIENT_INFO *ft_entry = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	ft_entry = pFtTable->hash[HashIdx];

	while (ft_entry)
	{
		if (NdisCompareMemory(ft_entry->sta_mac, pAddr, ETH_ALEN) == 0)
			break;
		else
			ft_entry = ft_entry->next;
	}

	return ft_entry;
}

FT_CLIENT_INFO *mt_iapp_ft_client_insert(
	FT_CLIENT_TABLE		*pFtTable,
	UCHAR 			*pStaAddr,
	UCHAR 			*pApAddr,
	INT32			ApIfIdx)
{
	UCHAR HashIdx;
	INT idx = 0;
	FT_CLIENT_INFO *ft_entry = NULL;
	FT_CLIENT_INFO *current_ft_entry = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!pStaAddr) {
		DBGPRINT(RT_DEBUG_ERROR, "%s pStaAddr == NULL\n", __func__);
		return ft_entry;
	}

	ft_entry = mt_iapp_ft_client_look_up(pFtTable, pStaAddr);
	if (ft_entry) {
		/* Update information */
		if (pApAddr)
			NdisCopyMemory(ft_entry->ap_mac, pApAddr, ETH_ALEN);
		ft_entry->if_idx = ApIfIdx;
		ft_entry->used = 1;
		return ft_entry;
	}

	if (pFtTable->ft_sta_table_size >= MAX_NUM_OF_CLIENT) {
		DBGPRINT(RT_DEBUG_ERROR, "iapp> %s - FT client table is full. (FtStaTableSize=%d)\n",
				__FUNCTION__, pFtTable->ft_sta_table_size);
		return NULL;
	}

	for (idx = 0; idx < MAX_NUM_OF_CLIENT; idx++) {
		ft_entry = &pFtTable->ft_sta_info[idx];
		if (ft_entry->used)
			continue;
		memset(ft_entry, 0, sizeof(FT_CLIENT_INFO));
		if (pStaAddr)
			NdisCopyMemory(ft_entry->sta_mac, pStaAddr, ETH_ALEN);
		if (pApAddr)
			NdisCopyMemory(ft_entry->ap_mac, pApAddr, ETH_ALEN);
		ft_entry->if_idx = ApIfIdx;
		ft_entry->used = 1;
		break;
	}
	pFtTable->ft_sta_table_size++;

	HashIdx = MAC_ADDR_HASH_INDEX(pStaAddr);
	ft_entry->hash_idx = HashIdx;
	if (pFtTable->hash[HashIdx] == NULL)
		pFtTable->hash[HashIdx] = ft_entry;
	else
	{
		current_ft_entry = pFtTable->hash[HashIdx];
		while (current_ft_entry->next != NULL)
			current_ft_entry = current_ft_entry->next;
		current_ft_entry->next = ft_entry;
	}
	return ft_entry;
}

VOID mt_iapp_ft_client_delete(
	FT_CLIENT_TABLE		*pFtTable,
	UCHAR 			*pStaAddr)
{
	UCHAR HashIdx = 0xFF;
	FT_CLIENT_INFO *ft_entry = NULL;
	FT_CLIENT_INFO *hash_ft_entry = NULL;
	FT_CLIENT_INFO *pre_hash_ft_entry = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	ft_entry = mt_iapp_ft_client_look_up(pFtTable, pStaAddr);

	if (ft_entry == NULL) {
		DBGPRINT(RT_DEBUG_TRACE, "iapp> %s - cannot find this entry (%02x:%02x:%02x:%02x:%02x:%02x)\n",
				__FUNCTION__,
				pStaAddr[0],
				pStaAddr[1],
				pStaAddr[2],
				pStaAddr[3],
				pStaAddr[4],
				pStaAddr[5]);
		return;
	}

	ft_entry->used = 0;
	NdisZeroMemory(ft_entry->ap_mac, ETH_ALEN);
	NdisZeroMemory(ft_entry->sta_mac, ETH_ALEN);
	ft_entry->if_idx = -1;

	HashIdx = ft_entry->hash_idx;
	hash_ft_entry = pFtTable->hash[HashIdx];
	pre_hash_ft_entry = NULL;
	if (hash_ft_entry != NULL)
	{
		/* update Hash list*/
		do
		{
			if (hash_ft_entry == ft_entry)
			{
				if (pre_hash_ft_entry == NULL)
					pFtTable->hash[HashIdx] = ft_entry->next;
				else
					pre_hash_ft_entry->next = ft_entry->next;
				break;
			}

			pre_hash_ft_entry = hash_ft_entry;
			hash_ft_entry = hash_ft_entry->next;
		} while (hash_ft_entry);
	}
	pFtTable->ft_sta_table_size--;
}

INT32 mt_iapp_find_ifidx_by_sta_mac(
	FT_CLIENT_TABLE		*pFtTable,
	UCHAR			*pStaMAC)
{
	FT_CLIENT_INFO *ft_entry = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	ft_entry = mt_iapp_ft_client_look_up(pFtTable, pStaMAC);
	if (ft_entry)
		return ft_entry->if_idx;
	else
		return -1;
}
/* End of mt_iapp_util.c */
