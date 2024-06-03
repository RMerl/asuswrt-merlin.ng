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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "wapp_cmm.h"


#include "hotspot.h"

int wapp_event_get_neighbor_report_list(struct wifi_app *wapp,	 const char *iface, const char *neighbor_list_req, size_t neighbor_list_len)
{
	int ret = 0;
	int i = 0;
	DAEMON_NR_MSG *msg = (DAEMON_NR_MSG *)neighbor_list_req;
	DAEMON_EVENT_NR_LIST NRList;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	os_memcpy(&NRList, &msg->evt_nr_list, sizeof(DAEMON_EVENT_NR_LIST));	

	DBGPRINT(RT_DEBUG_ERROR, "%s NRList.Newlist %d  NRList.CurrNum %d\n",
		__FUNCTION__,NRList.Newlist,NRList.CurrNum);

	if (NRList.Newlist)
	{
		os_memset(&wapp->daemon_nr_list, 0, sizeof(DAEMON_NR_LIST));
	}

	if (NRList.CurrNum > 0)
	{
		u8 offset = wapp->daemon_nr_list.CurrListNum;
        wapp_nr_info *nr_entry = NULL;
        int entry_idx = NR_ENTRY_NOT_FOUND;
		DBGPRINT(RT_DEBUG_ERROR, "GOT neighbor list num: %d\n", NRList.CurrNum);		
		
		if(wapp->daemon_nr_list.CurrListNum + NRList.CurrNum <= MBO_NEIGHBOR_REPORT_MAX_LEN)
		{
            for (i = 0; i < NRList.CurrNum; i++)
            {
                nr_entry = &NRList.EvtNRInfo[i];
			
                entry_idx = nr_list_lookup_by_mac_addr(wapp, nr_entry->Bssid);
				//printf("\033[1;33m %s, %u, %d  entry_idx = %u, bssid = %02x:%02x:%02x:%02x:%02x:%02x\033, CurrListNum = %u, offset = %u[0m\n", __FUNCTION__, __LINE__,i, entry_idx, PRINT_MAC(nr_entry->Bssid), wapp->daemon_nr_list.CurrListNum, offset);  /* Haipin Debug Print (Y)*/
				if (entry_idx == NR_ENTRY_NOT_FOUND)
                {
                    /* copy to the new entry */
					if (offset >= DAEMON_NEIGHBOR_REPORT_MAX_NUM) {
						DBGPRINT(RT_DEBUG_ERROR, "WAPP:ERROR! [%s](%d): offset(%d) is over than %d;\n",
						__func__, __LINE__, offset, DAEMON_NEIGHBOR_REPORT_MAX_NUM);
						return -1;
					}
			        os_memcpy(&wapp->daemon_nr_list.NRInfo[offset], &NRList.EvtNRInfo[i],
			                  sizeof(wapp_nr_info));
                    offset++;
			        wapp->daemon_nr_list.CurrListNum++;
                } else {
                    /* update the existing entry */
			        os_memcpy(&wapp->daemon_nr_list.NRInfo[entry_idx], &NRList.EvtNRInfo[i],
			                  sizeof(wapp_nr_info));
                }
            }
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, "%s: overflow !!! currNum %d, listNum %d\n",
				__FUNCTION__, wapp->daemon_nr_list.CurrListNum, NRList.CurrNum);
		}
			
	}	
	wapp_send_reduced_nr_list_by_inf(wapp, iface);

	return ret;
}

int nr_list_lookup_by_mac_addr(struct wifi_app *wapp, u8* mac_addr)
{
	u16 i = 0;
	u8 ret = NR_ENTRY_NOT_FOUND;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	for(i = 0; i < wapp->daemon_nr_list.CurrListNum; i++) {
		if(	MAC_ADDR_EQUAL(mac_addr, &wapp->daemon_nr_list.NRInfo[i].Bssid[0])) {
			ret = i;
		        break;
        	}
	}
	DBGPRINT(RT_DEBUG_TRACE, "%s ret index %d\n", __FUNCTION__,ret);

	return ret;	
}

int nr_entry_add(struct wifi_app *wapp, wapp_nr_info* nr_info)
{
	int nr_index = 0;
	wapp_nr_info* nr_entry = NULL;
	
	/* if there is already the same bssid in the nr_list, use it. */
	/* if no the same bssid found, append an entry to the nr_list */
	nr_index = nr_list_lookup_by_mac_addr(wapp, nr_info->Bssid);
	DBGPRINT(RT_DEBUG_TRACE, GRN("%s nr_index %d wapp->daemon_nr_list.CurrListNum %d\n"), __func__,nr_index,wapp->daemon_nr_list.CurrListNum);

	if (nr_index < NR_ENTRY_NOT_FOUND)  /* found exist entry */
		nr_entry = &wapp->daemon_nr_list.NRInfo[nr_index];
	else {								/* create new entry */
		if (wapp->daemon_nr_list.CurrListNum < (DAEMON_NEIGHBOR_REPORT_MAX_NUM - 1)) {
			nr_index = wapp->daemon_nr_list.CurrListNum;
			nr_entry = &wapp->daemon_nr_list.NRInfo[nr_index];
			COPY_MAC_ADDR(nr_entry->Bssid, nr_info->Bssid);
			nr_entry->CandidatePrefSubID = 0x3;
			nr_entry->CandidatePrefSubLen = 1;
			wapp->daemon_nr_list.CurrListNum++;
		} else {
			DBGPRINT(RT_DEBUG_ERROR, "%s: list is full\n", __func__);
			return WAPP_RESOURCE_ALLOC_FAIL;
		}
	}

	if (nr_info->RegulatoryClass)
		nr_entry->RegulatoryClass = nr_info->RegulatoryClass;
	if (nr_info->ChNum)
		nr_entry->ChNum = nr_info->ChNum;
	nr_entry->CandidatePref = nr_info->CandidatePref;
	
	DBGPRINT(RT_DEBUG_ERROR, GRN("nr_entry_add done\n"));

	return WAPP_SUCCESS;
}

int nr_peer_btm_query_action(struct wifi_app *wapp, const char *btm_query,
						    size_t btm_query_len)
{
	u8 	*pos = (u8 *)btm_query;
	u8 	ret = 0;
	u16 parsed_len = 0;
	u8 query_reason = 0;
	u8 tmp_len = 0;
	u8 mac_addr[MAC_ADDR_LEN] = {0};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	/*nr list full, return*/
	if(wapp->daemon_nr_list.CurrListNum >= DAEMON_NEIGHBOR_REPORT_MAX_NUM - 1)
		return ret;

	DBGPRINT(RT_DEBUG_ERROR, "%s: query_len %zu , nrlist.CurrListNum %d\n",__FUNCTION__, btm_query_len,wapp->daemon_nr_list.CurrListNum);
	hex_dump("======btm query =======\n",pos,btm_query_len);

	/* dialog token */
	pos++;
	query_reason = *pos;
	pos++;
	parsed_len++;

	if(query_reason != 19)
		DBGPRINT(RT_DEBUG_ERROR, "%s: query_reason %d , suppose to be 19...\n",__FUNCTION__, query_reason);
	
	
	while(parsed_len < btm_query_len) {
		if(*pos == IE_NEIGHBOR_REPORT) { 			
			pos++;
			parsed_len++;

			tmp_len = *pos;
			pos++;
			parsed_len++;

			if(parsed_len + tmp_len > btm_query_len) {
				DBGPRINT(RT_DEBUG_ERROR, "%s: malformed: sub-element lenth overflow, stop parsing...\n",__FUNCTION__);
				break;
			}

			/* add new neighbor report to nr_list if we don't have it */

			COPY_MAC_ADDR(mac_addr,pos);
			if(nr_list_lookup_by_mac_addr(wapp,mac_addr) == NR_ENTRY_NOT_FOUND 
			&& (wapp->daemon_nr_list.CurrListNum + 1 < DAEMON_NEIGHBOR_REPORT_MAX_NUM)
			) {
				wapp_nr_info *nr_entry = &wapp->daemon_nr_list.NRInfo[wapp->daemon_nr_list.CurrListNum];
				NdisZeroMemory(nr_entry, sizeof(wapp_nr_info));
				os_memcpy(nr_entry, pos, NEIGHBOR_REPORT_IE_SIZE);				
				DBGPRINT(RT_DEBUG_TRACE, "%s: add nr[%d] bssid %02x:%02x:%02x:%02x:%02x:%02x\n",
					__FUNCTION__,wapp->daemon_nr_list.CurrListNum,PRINT_MAC(nr_entry->Bssid));
				wapp->daemon_nr_list.CurrListNum++;
			}
			pos += tmp_len;
			parsed_len += tmp_len;
		}
		else {
			DBGPRINT(RT_DEBUG_ERROR, "%s: unrecognized sub-element ID %d ...\n",__FUNCTION__, *pos);
			break;
		}
	}

	return ret;
}

int nr_peer_btm_req_action(struct wifi_app *wapp, const char *btm_req,
		size_t btm_req_len,
		enum bss_trans_mgmt_status_code *status)
{
	u8 *pos = (u8 *)btm_req;
	u8 *end = (u8 *)btm_req + btm_req_len;
	u8 ret = 0, req_mode = 0;
	u16 parsed_len = 0, disassoc_timer = 0;
	u8 val_interval = 0;

	DBGPRINT(RT_DEBUG_TRACE, " %s: WNM: request_len %zu\n", __FUNCTION__, btm_req_len);
	hex_dump("======btm Request =======\n", pos, btm_req_len);

	req_mode = *pos;
	pos++;
	parsed_len++;

	disassoc_timer = *pos;
	pos += 2;
	parsed_len += 2;

	val_interval = *pos;
	pos++;
	parsed_len++;

	DBGPRINT(RT_DEBUG_TRACE, "%s: WNM: BTM Request: req_mode %d disassoc_timer %d val_interval %d\n",
			__FUNCTION__, req_mode, disassoc_timer, val_interval);

	if (req_mode & (1<<BSS_TERM_INCLUDED_BIT_MAP)) {
		if (end - pos < 12) {
			*status = WNM_BSS_TM_REJECT_UNSPECIFIED;
			DBGPRINT(RT_DEBUG_ERROR, "%s: WNM: Too short BSS TM Request\n", __FUNCTION__);
			return ret;
		}
		/* BSS Termination Duration */
		pos += 12;
	}

	if (req_mode & (1<<ESS_DISASSOC_IMNT_BIT_MAP)) {
		if (end - pos < 1 || 1 + pos[0] > end - pos) {
			*status = WNM_BSS_TM_REJECT_UNSPECIFIED;
			DBGPRINT(RT_DEBUG_ERROR, "%s: WNM: Invalid BTM Request (URL)\n", __FUNCTION__);
			return ret;
		}
		pos += 1 + pos[0];
	}

	if (req_mode & (1<<CAND_LIST_INCLUDED_BIT_MAP)) {
		DBGPRINT(RT_DEBUG_TRACE, "%s: WNM: Preferred List Available, Neighbor report tag %d\n",
			__FUNCTION__, *pos);

		while(parsed_len < btm_req_len) {
			if (*pos == IE_NEIGHBOR_REPORT) {
				ret = wapp_parse_nr_elem(wapp, (void *)pos);

			} else {
				DBGPRINT(RT_DEBUG_TRACE, "%s: WNM: unrecognized sub-element ID %d ...\n", __FUNCTION__, *pos);
			}
			parsed_len +=  2 + pos[1];
			pos += 2 + pos[1];
		}
		if (ret > 0) {
			*status = WNM_BSS_TM_REJECT_NO_SUITABLE_CANDIDATES;
			DBGPRINT(RT_DEBUG_TRACE, "%s: WNM: Candidate list included bit is set, but no candidates found \n", __FUNCTION__);
			return ret;
		}
	}

	if ((req_mode & (1<<DISASSOC_IMNT_BIT_MAP)) || (req_mode & (1<<ESS_DISASSOC_IMNT_BIT_MAP)) ||
			(req_mode & (1<<BSS_TERM_INCLUDED_BIT_MAP))) {
		*status = WNM_BSS_TM_ACCEPT;
	} else {
		*status = WNM_BSS_TM_REJECT_UNSPECIFIED;
	}

	return ret;
}


