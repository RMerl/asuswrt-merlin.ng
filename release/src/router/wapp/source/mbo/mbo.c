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

#include <stdlib.h>
#include <stdio.h>
#include "interface.h"
#include "mbo.h"
#include "driver_wext.h"
#include "debug.h"
#ifdef MBO_SUPPORT

struct mbo_event_ops;
extern struct mbo_event_ops mbo_evnt_ops;

extern struct mbo_drv_ops mbo_drv_wext_ops;
extern int map_build_r2_mbo_sta_ch_pref(
	struct wifi_app *wapp, unsigned char *buf, int buf_len);
#ifdef MAP_SUPPORT
#ifdef MAP_R2
int map_build_r2_mbo_sta_ch_pref(
	struct wifi_app *wapp, unsigned char *buf, int buf_len)
{
	struct evt *map_event = os_zalloc(buf_len + sizeof(struct evt));
	int send_pkt_len = 0;

	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);
	if (!map_event)
		return -1;
	map_event->type = WAPP_MBO_STA_PREF_CH_LIST;
	map_event->length = buf_len;
	os_memcpy(map_event->buffer, buf, buf_len);

	send_pkt_len = sizeof(*map_event) + map_event->length;
	DBGPRINT(RT_DEBUG_OFF, "%s, send_pkt_len:%d, buf_len:%d\n", __func__, send_pkt_len, buf_len);
	if (0 > map_1905_send(wapp, (char *)map_event, send_pkt_len)) {
		DBGPRINT(RT_DEBUG_ERROR, "%s send map_build_r2_mbo_sta_ch_pref msg fail\n", __func__);
	}
	if (map_event)
		os_free(map_event);
	return send_pkt_len;
}

#endif
#endif
int mbo_init(struct mbo_cfg *mbo)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	mbo->event_ops = &mbo_evnt_ops;
	mbo->drv_ops = &mbo_drv_wext_ops;
	mbo->cdcp = 1;
	mbo->assoc_retry_delay = 10;
#if 0
	mbo->disassoc_imnt = 0;
	mbo->btm_bss_termination_onoff = 0;
#endif
	return MBO_SUCCESS;
}

int mbo_sta_update(
struct wifi_app *wapp,
struct wapp_sta *sta,
u8				action,
u32				len,
void			*data)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp || !sta) {
		return MBO_INVALID_ARG;
	}

	switch(action)
	{
		case MBO_CDC_UPDATE:
		{
			if (len == sizeof(u8))
				sta->cell_data_cap = *((u8 *) data);
			break;
		}

		case MBO_NPC_APPEND:
		{
			if (len == sizeof( struct non_pref_ch)){
				struct non_pref_ch_entry *npc_entry;

				npc_entry = os_zalloc(sizeof(struct non_pref_ch_entry));
				os_memcpy(&npc_entry->npc, data, len);
				dl_list_add_tail(&sta->non_pref_ch_list, &npc_entry->list);
			}
			
			break;
		}

		case MBO_BSSID_UPDATE:
		{
			if (len == MAC_ADDR_LEN){
				os_memcpy(&sta->bssid, data, len);
			}
			
			break;
		}

		case MBO_AKM_UPDATE:
		{
			if (len == sizeof(u32))
				sta->akm = *((u32 *) data);
			break;
		}
		
		case MBO_CIPHER_UPDATE:
		{
			if (len == sizeof(u32))
				sta->cipher = *((u32 *) data);
			break;
		}	
			
		default:
			break;
	}

	return MBO_SUCCESS;
}

int mbo_sta_clear_npc_list(
struct wapp_sta	*sta)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!sta) {
		return MBO_INVALID_ARG;
	}

	if (!dl_list_empty(&sta->non_pref_ch_list)) {
		struct non_pref_ch_entry *npc_entry, *npc_entry_tmp;

		dl_list_for_each_safe(npc_entry, npc_entry_tmp,
						&sta->non_pref_ch_list, struct non_pref_ch_entry, list)
		{	
			dl_list_del(&npc_entry->list);
			os_free(npc_entry);
		}
	}

	return MBO_SUCCESS;
}

BOOLEAN mbo_check_bss_sta_security_match(
	struct wapp_sta *sta,
	wapp_nr_info *nr_entry)
{
	BOOLEAN ret = FALSE;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if((nr_entry->akm & sta->akm)
	&& (nr_entry->cipher & sta->cipher))
		ret = TRUE;
	else
		ret = FALSE;

	DBGPRINT_RAW(RT_DEBUG_TRACE, 
		"%s: %02X:%02X:%02X:%02X:%02X:%02X pBssEntry %x/%x & pSTA %x/%x = (%d/%d)  ret %d\n", __FUNCTION__,
		PRINT_MAC(nr_entry->Bssid),
		nr_entry->akm ,nr_entry->cipher,
		sta->akm,sta->cipher,
		(nr_entry->akm & sta->akm),
		(nr_entry->cipher & sta->cipher),
		ret);
		
	return ret;	
}

int mbo_check_sta_preference_and_append_nr_list(
	struct wifi_app *wapp,
	struct wapp_sta *sta,
	char *frame_pos, 
	u16 *frame_len, 
	u8 frame_type,
	u8 disassoc_imnt,
	u8 bss_term,
	u8 is_steer_to_cell)
{
	u8 	list_len=0;	
	u8 	i=0;
	struct non_pref_ch_entry *npc_entry;
	u8 	btm_neighbor_report_header[2] = {0};
	//u16 anqp_neighbor_report_header[2] = {0};
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(sta == NULL){
		DBGPRINT_RAW(RT_DEBUG_OFF, "%s - can't find in sta list,"
		" please check if sta has MBO IE , fill default preference value\n", 
			__FUNCTION__);
	}
	else{
		DBGPRINT_RAW(RT_DEBUG_OFF, 
			"\033[1;32m%s %d - %02X:%02X:%02X:%02X:%02X:%02X ,"
			" BSSID %02X:%02X:%02X:%02X:%02X:%02X  disassoc_imnt %d\033[0m\n",
			__FUNCTION__,__LINE__,
			PRINT_MAC(sta->mac_addr),
			PRINT_MAC(sta->bssid),
			disassoc_imnt);
	}

	if(is_steer_to_cell)
		DBGPRINT_RAW(RT_DEBUG_ERROR, "%s This is Steer_To_Cell case !!\n", __FUNCTION__);
	
	if( wapp->daemon_nr_list.CurrListNum > 0){
		for(i=0;i<wapp->daemon_nr_list.CurrListNum;i++){
			wapp_nr_info nr_entry;			
			u16 append_entry_len = NEIGHBOR_REPORT_IE_SIZE; /* append Bssid ~ CandidatePref */

			os_memcpy(&nr_entry, &wapp->daemon_nr_list.NRInfo[i], sizeof(wapp_nr_info));
	
			if (sta){
				RRM_BSSID_INFO bss_info;				
				
				/* step 1: in steering to cellular case, only append AP's own BSS as candidate */
				if(is_steer_to_cell)
				{				
					if(!MAC_ADDR_EQUAL(nr_entry.Bssid,sta->bssid))
						continue;
					else
						DBGPRINT_RAW(RT_DEBUG_ERROR, "%s steer to cell, append ap's own bss!!\n",__FUNCTION__);
				}
				/* step 2: search STA CH list and fill preference */
				if(!dl_list_empty(&sta->non_pref_ch_list)){
					dl_list_for_each(npc_entry, &sta->non_pref_ch_list, struct non_pref_ch_entry, list) {
						
						if(npc_entry && npc_entry->npc.ch == nr_entry.ChNum)
							nr_entry.CandidatePref = npc_entry->npc.pref;					
					}					
				}else{
					if(sta->no_none_pref_ch != TRUE)
						DBGPRINT_RAW(RT_DEBUG_OFF, 
							"%s %d - uxexpected npc list empty !!\n", 
							__FUNCTION__,__LINE__);
				}	
				
				/* step 3: if disassoc imminent , set AP's own bss neigbor report preference to 0 */
				DBGPRINT_RAW(RT_DEBUG_TRACE, "\033[1;32m%s %d - frame_type==BTM? %d ,"
				" BSSID %02X:%02X:%02X:%02X:%02X:%02X  disassoc_imnt %d mac qual? %d\033[0m\n",
				__FUNCTION__,__LINE__,(frame_type == MBO_FRAME_BTM),PRINT_MAC(nr_entry.Bssid),
				disassoc_imnt,MAC_ADDR_EQUAL(nr_entry.Bssid,sta->bssid));
				if(frame_type == MBO_FRAME_BTM
				&& (disassoc_imnt || bss_term)
				&& MAC_ADDR_EQUAL(nr_entry.Bssid,sta->bssid)){
					nr_entry.CandidatePref = 0;  
					
					DBGPRINT_RAW(RT_DEBUG_OFF, "%s %d - disassoc_imnt %d set ap_own_bss_pref 0!!\n",
						__FUNCTION__,__LINE__,disassoc_imnt);
				}
				
				/* step 4: update nr_entry security bit */
				bss_info.word = nr_entry.BssidInfo;
				bss_info.field.Security = \
					(mbo_check_bss_sta_security_match(sta,&nr_entry))?1:0;
				nr_entry.BssidInfo = bss_info.word;
				
			}

#if 0
			/* append nr header base on frame type */			
			if(frame_type == MBO_FRAME_BTM){
				/* element ID: 52, len: 16 */
				btm_neighbor_report_header[0] = IE_RRM_NEIGHBOR_REP;
				btm_neighbor_report_header[1] = append_entry_len;

				os_memcpy(frame_pos, &btm_neighbor_report_header, sizeof(btm_neighbor_report_header));
				frame_pos 	+= sizeof(btm_neighbor_report_header);
				*frame_len 	+= sizeof(btm_neighbor_report_header);
				list_len 	+= sizeof(btm_neighbor_report_header);
			}
			else if(frame_type == MBO_FRAME_ANQP){
				/* element ID: 272, len: 16 */
				anqp_neighbor_report_header[0] = NEIGHBOR_REPORT;
				anqp_neighbor_report_header[1] = append_entry_len;

				os_memcpy(frame_pos, &anqp_neighbor_report_header, sizeof(anqp_neighbor_report_header));
				frame_pos 	+= sizeof(anqp_neighbor_report_header);
				*frame_len 	+= sizeof(anqp_neighbor_report_header);
				list_len 	+= sizeof(anqp_neighbor_report_header);
			}
#else
			/* element ID: 52, len: 16 */
			btm_neighbor_report_header[0] = IE_RRM_NEIGHBOR_REP;
			btm_neighbor_report_header[1] = append_entry_len;
			
			os_memcpy(frame_pos, &btm_neighbor_report_header, sizeof(btm_neighbor_report_header));
			frame_pos 	+= sizeof(btm_neighbor_report_header);
			*frame_len 	+= sizeof(btm_neighbor_report_header);
			list_len 	+= sizeof(btm_neighbor_report_header);
#endif
		
			/* append this nr_entry */
			os_memcpy(frame_pos, &nr_entry, append_entry_len);
			//hex_dump_dbg("entry", (u8 *)frame_pos, append_entry_len);
			frame_pos += append_entry_len;
			*frame_len += append_entry_len;
			list_len  += append_entry_len;

			DBGPRINT_RAW(RT_DEBUG_ERROR, 
					"append [%d] frame_pos %p frame_len %d mac %02x:%02x:%02x:%02x:%02x:%02x\n"
					,i,frame_pos,*frame_len,PRINT_MAC(nr_entry.Bssid));

		}
	}

	//hex_dump_dbg("frame", (u8 *) (frame_pos-*frame_len), *frame_len);
	return list_len;
}

int cmpfunc(
	const void * pa, 
	const void * pb)
{
	u8 la,lb;
	wapp_nr_info *a = (wapp_nr_info *)pa;
	wapp_nr_info *b = (wapp_nr_info *)pb;

	la = a->Rssi;
	lb = b->Rssi;
	if (la < lb)
		return 1;
	else
		return -1;
}

int mbo_append_reduced_nr_list(
	struct wifi_app *wapp,
	char *frame_pos,
	u16 *frame_len)
{
	u8	list_len=0;
	u8	i, j;
	u8	Band0_channel_count = 0;
	u8	Non_2G_channel_count = 0; /* Support more dual band*/
	u8	muti_band = 0;
	u8	reduced_neighbor_report_header[2] = {0};
	u16	append_entry_len;
	u8	reduced_neighbor_report_append_num = 0;

	if (wapp->daemon_nr_list.CurrListNum > 1){
		/* decide number of reduced neighbor report should be appended */
		reduced_neighbor_report_append_num = (wapp->daemon_nr_list.CurrListNum > OCE_MAX_RNR_NUM_TO_BCN) ? OCE_MAX_RNR_NUM_TO_BCN : wapp->daemon_nr_list.CurrListNum;

		/* select only 9 reduced neighbor report for beacon, OCE_MAX_RNR_NUM_TO_BCN = 9 */
		if (wapp->daemon_nr_list.CurrListNum > OCE_MAX_RNR_NUM_TO_BCN) {
			qsort(wapp->daemon_nr_list.NRInfo, wapp->daemon_nr_list.CurrListNum, sizeof(wapp_nr_info), cmpfunc);
		}

		for (i = 0; i < reduced_neighbor_report_append_num; i++){
			if (wapp->daemon_nr_list.NRInfo[i].ChNum < 15)
				Band0_channel_count++;
			else
				Non_2G_channel_count++;
		}

		if (Non_2G_channel_count > 0 && Band0_channel_count > 0)
			muti_band = TRUE;

		for (i = 0; i < reduced_neighbor_report_append_num; i++){
			wapp_nr_info reduced_nr_entry;
			os_memcpy(&reduced_nr_entry, &wapp->daemon_nr_list.NRInfo[i], sizeof(wapp_nr_info));
			/* no needs to add own bssid in RNR */
			if (reduced_nr_entry.TbttInfoSetNum == 1)
				continue;
			else if (reduced_nr_entry.TbttInfoSetNum == 2 && muti_band > 0)
				continue;

			if (reduced_nr_entry.TbttInfoSetNum > 1 && muti_band == 0)
				append_entry_len = 4 + (wapp->daemon_nr_list.NRInfo[i].TbttInfoSetNum - 1) * 11;
			else if (reduced_nr_entry.TbttInfoSetNum > 2 && muti_band == 1)
				append_entry_len = 4 + (wapp->daemon_nr_list.NRInfo[i].TbttInfoSetNum - 2) * 11;
			else
				append_entry_len = 4 + (wapp->daemon_nr_list.NRInfo[i].TbttInfoSetNum + 1) * 11;

			/* element ID: 201, len: 11 */
			reduced_neighbor_report_header[0] = IE_REDUCED_NEIGHBOR_REPORT;
			reduced_neighbor_report_header[1] = append_entry_len;

			os_memcpy(frame_pos, &reduced_neighbor_report_header, sizeof(reduced_neighbor_report_header));
			frame_pos	+= sizeof(reduced_neighbor_report_header);
			*frame_len	+= sizeof(reduced_neighbor_report_header);
			list_len	+= sizeof(reduced_neighbor_report_header);

			/* build TbttInfoHdr */
			TBTT_INFO_HEADER TbttInfoHdr;

			TbttInfoHdr.word = 0;
			TbttInfoHdr.field.TbttInfoType = 0;
			TbttInfoHdr.field.FilteredNrAP = 0;
			TbttInfoHdr.field.Reserved = 0;
			/* add MBSSID in RNR */
			if (reduced_nr_entry.TbttInfoSetNum > 1 && muti_band == 0)
				TbttInfoHdr.field.TbttInfoCount = reduced_nr_entry.TbttInfoSetNum - 2;
			else if (reduced_nr_entry.TbttInfoSetNum > 2 && muti_band == 0)
				TbttInfoHdr.field.TbttInfoCount = reduced_nr_entry.TbttInfoSetNum - 2;
			/* add scaned neighbor in RNR */
			else
				TbttInfoHdr.field.TbttInfoCount = 0;
			TbttInfoHdr.field.TbttInfoLength = 11;

			/* append this nr_entry */
			/* append TbttInfoHdr */
			os_memcpy(frame_pos, &TbttInfoHdr, 2);
			frame_pos += 2;
			*frame_len += 2;
			list_len  += 2;
			/* append RegulatoryClass */
			os_memcpy(frame_pos, &reduced_nr_entry.RegulatoryClass, 1);
			frame_pos += 1;
			*frame_len += 1;
			list_len  += 1;
			/* append ChNum */
			os_memcpy(frame_pos, &reduced_nr_entry.ChNum, 1);
			frame_pos += 1;
			*frame_len += 1;
			list_len  += 1;
			if (reduced_nr_entry.TbttInfoSetNum > 1) {
				for (j = 1; j < reduced_nr_entry.TbttInfoSetNum; j++) {
					if ((i+j) >= DAEMON_NEIGHBOR_REPORT_MAX_NUM) {
						DBGPRINT(RT_DEBUG_ERROR, "WAPP:ERROR! [%s](%d): (i+j)(%d) is over than %d;\n",
						__func__, __LINE__, i+j, DAEMON_NEIGHBOR_REPORT_MAX_NUM);
						return -1;
					}
					/* append TbttInfoSet: NrAPTbttOffset, Bssid, ShortBssid */
					os_memcpy(frame_pos, &wapp->daemon_nr_list.NRInfo[i + j].TbttInfoSet.NrAPTbttOffset, 1);
					frame_pos += 1;
					*frame_len += 1;
					list_len  += 1;
					os_memcpy(frame_pos, &wapp->daemon_nr_list.NRInfo[i + j].Bssid, 6);
					frame_pos += 6;
					*frame_len += 6;
					list_len  += 6;
					os_memcpy(frame_pos, &wapp->daemon_nr_list.NRInfo[i + j].TbttInfoSet.ShortBssid, 4);
					frame_pos += 4;
					*frame_len += 4;
					list_len  += 4;

					printf("\033[1;33m first No.%d %02X:%02X:%02X:%02X:%02X:%02X  OpClass %d ChNum %d \033[0m\n"
					,i + j
					,PRINT_MAC(wapp->daemon_nr_list.NRInfo[i + j].Bssid)
					,wapp->daemon_nr_list.NRInfo[i + j].RegulatoryClass
					,wapp->daemon_nr_list.NRInfo[i + j].ChNum);  /* Melody Debug Print (B)*/
				}

				i += (reduced_nr_entry.TbttInfoSetNum - 1);
			} else if (reduced_nr_entry.TbttInfoSetNum == 0) {
				/* append TbttInfoSet: NrAPTbttOffset, Bssid, ShortBssid */
				os_memcpy(frame_pos, &reduced_nr_entry.TbttInfoSet.NrAPTbttOffset, 1);
				frame_pos += 1;
				*frame_len += 1;
				list_len  += 1;
				os_memcpy(frame_pos, &reduced_nr_entry.Bssid, 6);
				frame_pos += 6;
				*frame_len += 6;
				list_len  += 6;
				os_memcpy(frame_pos, &reduced_nr_entry.TbttInfoSet.ShortBssid, 4);
				frame_pos += 4;
				*frame_len += 4;
				list_len  += 4;

				printf("\033[1;33m second No.%d %02X:%02X:%02X:%02X:%02X:%02X  OpClass %d ChNum %d \033[0m\n"
				,i
				,PRINT_MAC(wapp->daemon_nr_list.NRInfo[i].Bssid)
				,wapp->daemon_nr_list.NRInfo[i].RegulatoryClass
				,wapp->daemon_nr_list.NRInfo[i].ChNum);  /* Melody Debug Print (B)*/
			}
		}
	}

	frame_pos -= *frame_len;
	return list_len;
}


void mbo_make_mbo_ie_for_btm(
	struct wifi_app *wapp,
	char *frame_pos, 
	u16 *frame_len, 
	u8 b_insert_cdcp,
	u8 b_insert_tran_reason,
	u8 tran_reason,
	u8 b_insert_retry_delay)
{	
	u8 	AttrLen = 0;
	u8 	MBO_OCE_OUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x16};
	u8 *tmpbuf = NULL;
	P_MBO_ATTR_STRUCT mbo_attr = NULL;
	struct mbo_cfg	*mbo;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(!b_insert_cdcp && !b_insert_tran_reason && !b_insert_retry_delay){
		DBGPRINT(RT_DEBUG_OFF, "%s no need to add, return.\n",__FUNCTION__);
		return;
	}
	mbo = wapp->mbo;
	tmpbuf = os_zalloc(1024);

	if(tmpbuf == NULL){
		DBGPRINT(RT_DEBUG_OFF, "%s MEM ALLOC FAIL!!!!!!!\n",__FUNCTION__);
		return;
	}
	
	if(b_insert_cdcp){
		mbo_attr = (P_MBO_ATTR_STRUCT)(tmpbuf + AttrLen);
		mbo_attr->AttrID = MBO_ATTR_AP_CDCP;
		mbo_attr->AttrLen = 1;
		mbo_attr->AttrBody[0] = mbo->cdcp;
		
		AttrLen	+= 3;
	}
	if(b_insert_tran_reason){
		mbo_attr = (P_MBO_ATTR_STRUCT)(tmpbuf + AttrLen);
		mbo_attr->AttrID = MBO_ATTR_AP_TRANS_REASON;
		mbo_attr->AttrLen = 1;
		mbo_attr->AttrBody[0] = tran_reason;
		
		AttrLen	+= 3;
	}
	if(b_insert_retry_delay){
		mbo_attr = (P_MBO_ATTR_STRUCT)(tmpbuf + AttrLen);
		mbo_attr->AttrID = MBO_ATTR_AP_ASSOC_RETRY_DELAY;
		mbo_attr->AttrLen = 2;
		os_memcpy(&mbo_attr->AttrBody[0], &mbo->assoc_retry_delay, 2);		
		AttrLen	+= 4;
	}
	
	
	*frame_pos 	= IE_MBO_ELEMENT_ID;
	frame_pos	+= 1;
	*frame_len 	+= 1;

	*frame_pos 	= AttrLen + 4;
	frame_pos	+= 1;
	*frame_len 	+= 1;

	os_memcpy(frame_pos, MBO_OCE_OUIBYTE, 4);
	frame_pos	+= 4;
	*frame_len 	+= 4;

	os_memcpy(frame_pos, tmpbuf, AttrLen);
	frame_pos	+= AttrLen;
	*frame_len 	+= AttrLen;

	os_free(tmpbuf);
	
	return;
}

/* mbo parameter setting */
inline int mbo_param_setting(struct wifi_app *wapp, 
										const char *iface, 
										u32 param, u32 value)
{
	int ret;

	ret = wapp->mbo->drv_ops->drv_mbo_param_setting(wapp->drv_data, iface, 
								    param, value);
	return ret;
}

int mbo_event_sta_update(struct wifi_app *wapp, char *msg, u8 msg_type)
{
	struct wapp_sta *sta = NULL;
	int ret = MBO_SUCCESS;
	struct mbo_msg *pmsg = (struct mbo_msg *) msg;
	struct wapp_dev *wdev = NULL;
	MBO_STA_CH_PREF_CDC_INFO *info = &pmsg->body.MboEvtStaInfo;
#ifdef MAP_SUPPORT
#ifdef MAP_R2
	unsigned char *tmp_buf, *buf = NULL;
	int buf_len = 0;
#endif
#endif
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, pmsg->ifindex);

	if (!wdev)
		return MBO_LOOKUP_ENTRY_NOT_FOUND;

	sta = wdev_ap_client_list_lookup(wapp, (struct ap_dev *) wdev->p_dev, info->mac_addr);

	if (!sta)
		return MBO_LOOKUP_ENTRY_NOT_FOUND;

	switch(msg_type)
	{
		case MBO_MSG_STA_PREF_UPDATE:
		{
			int i;

			if (ret != MBO_SUCCESS)
				break;
			if (info->npc_num == 0xFF) { /* means that the sta has no none-prefer ch */
				sta->no_none_pref_ch = TRUE;
				mbo_sta_clear_npc_list(sta);
			} else if (info->npc_num) {
				
				mbo_sta_clear_npc_list(sta);
				for (i = 0; i < info->npc_num; i++) {
					mbo_sta_update(wapp,
									sta,
									MBO_NPC_APPEND,
									sizeof(struct non_pref_ch),
									(void *) &info->npc[i]);
				}
#ifdef MAP_SUPPORT
#ifdef MAP_R2
				if (sta->is_APCLI == 0) {
					buf_len = MAC_ADDR_LEN + sizeof(struct non_pref_ch) * info->npc_num;
					buf = os_zalloc(buf_len);
					if (buf == NULL)
						break;
					tmp_buf = buf;
					os_memcpy(tmp_buf, sta->mac_addr, MAC_ADDR_LEN);
					tmp_buf += MAC_ADDR_LEN;
					os_memcpy(tmp_buf, info->npc, sizeof(struct non_pref_ch) * info->npc_num);
					map_build_r2_mbo_sta_ch_pref(wapp, buf, buf_len);
					os_free(buf);
				}
#endif
#endif
			}
		}
			break;

		case MBO_MSG_CDC_UPDATE:
			mbo_sta_update(	wapp,
							sta,
							MBO_CDC_UPDATE,
							sizeof(sta->cell_data_cap),
							(void *) &info->cdc);
			break;

		case MBO_MSG_BSSID_UPDATE:
			printf("\033[1;32m %s, %u info->akm/cipher 0x%x/0x%x\033[0m\n"
				, __FUNCTION__, __LINE__,info->akm,info->cipher); 
			mbo_sta_update(	wapp,
							sta,
							MBO_BSSID_UPDATE,
							sizeof(sta->bssid),
							(void *) &info->bssid);
			mbo_sta_update(	wapp,
							sta,
							MBO_AKM_UPDATE,
							sizeof(sta->akm),
							(void *) &info->akm);
			mbo_sta_update(	wapp,
							sta,
							MBO_CIPHER_UPDATE,
							sizeof(sta->cipher),
							(void *) &info->cipher);
			break;
		default:
			break;
	}

	return MBO_SUCCESS;
}

// TODO: move to btm
int mbo_send_btm_req_by_cmd(
    struct wifi_app *wapp,
    const char *iface,
    const u8 *mac_addr)
{
	int i = 0;
	//struct wapp_conf *conf;
	struct btm_cfg *btm = NULL;
	u8 ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp)
		return MBO_INVALID_ARG;

	btm = &wapp->protocol.btm;

	/* Add the entry which is set by cmd */
	if (os_memcmp(btm->nebor_bssid , ZERO_MAC_ADDR, MAC_ADDR_LEN) != 0) {
	    /* if there is already the same bssid in the nr_list, use it. */
	    /* if no the same bssid found, append an entry to the nr_list */
	    for(i=0;i<wapp->daemon_nr_list.CurrListNum;i++){
	        if (os_memcmp(wapp->daemon_nr_list.NRInfo[i].Bssid, btm->nebor_bssid, MAC_ADDR_LEN) == 0) {
		    break;
	        }
	    }

	    /* fill the setup */
	    if (i == wapp->daemon_nr_list.CurrListNum) {
	        os_memcpy(wapp->daemon_nr_list.NRInfo[i].Bssid, btm->nebor_bssid, MAC_ADDR_LEN);
		wapp->daemon_nr_list.NRInfo[i].CandidatePrefSubID = 0x3;
		wapp->daemon_nr_list.NRInfo[i].CandidatePrefSubLen = 1;
		wapp->daemon_nr_list.CurrListNum++;
	    }
	    wapp->daemon_nr_list.NRInfo[i].RegulatoryClass = btm->nebor_op_class;
	    wapp->daemon_nr_list.NRInfo[i].ChNum = btm->nebor_op_ch;
	    wapp->daemon_nr_list.NRInfo[i].CandidatePref = btm->nebor_pref;
#if 0 /* Not filled parts */
	    //wapp->daemon_nr_list.NRInfo[i].akm = ;
	    //wapp->daemon_nr_list.NRInfo[i].cipher = ;
	    //wapp->daemon_nr_list.NRInfo[i].PhyType = ;
	    //wapp->daemon_nr_list.NRInfo[i].PhyType = ;
#endif
	}

    if (TEST_EVENT_STA_STEERING(btm)) {
	    DBGPRINT_RAW(RT_DEBUG_OFF, 
                     "TEST_EVENT_STA_STEERING, "
                     "mac addr = %02x:%02x:%02x:%02x:%02x:%02x.\n",
                     PRINT_MAC(mac_addr));
	    wapp_send_btm_req_by_case(wapp, iface, mac_addr, BTM_STA_STEERING);
	} else if (TEST_EVENT_STA_DISASSOC(btm)) {
	    DBGPRINT_RAW(RT_DEBUG_OFF, 
                     "TEST_EVENT_STA_DISASSOC, "
                     "mac addr = %02x:%02x:%02x:%02x:%02x:%02x.\n",
                     PRINT_MAC(mac_addr));
        wapp_send_btm_req_by_case(wapp, iface, mac_addr, BTM_DISASSOC_STA);
	} else if (TEST_EVENT_BSS_TERM(btm)) {
	    DBGPRINT_RAW(RT_DEBUG_OFF, 
                     "TEST_EVENT_BSS_TERM.\n");
		mbo_param_setting(wapp, iface, PARAM_MBO_AP_BSS_TERM, 3); //TODO:
    } else if (TEST_EVENT_STA_TO_CELL(btm)) {
	    DBGPRINT_RAW(RT_DEBUG_OFF, 
                     "TEST_EVENT_STA_TO_CELL. TODO\n");
    } else {
	    DBGPRINT_RAW(RT_DEBUG_OFF, "%s(), Unknown test event.\n", __FUNCTION__);
    }

	return MBO_SUCCESS;
}

struct mbo_event_ops mbo_evnt_ops = {
	.sta_update = mbo_event_sta_update,
};

int mbo_cmd_steer_sta( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 mac_addr[MAC_ADDR_LEN];
	char *token;
	int i;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	i = 0;
	token = strtok(argv[1], ":");
	while (token != NULL) {
		AtoH(token, (char *) &mac_addr[i], 1);
		i++;
		if (i >= MAC_ADDR_LEN)
			break;
		token = strtok(NULL, ":");
	}

	wapp_send_btm_req_by_case(wapp, iface, mac_addr, BTM_STA_STEERING);
	return MBO_SUCCESS;
}

int mbo_cmd_steer_sta_to_cell( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 mac_addr[MAC_ADDR_LEN];
	char *token;
	int i;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	i = 0;
	token = strtok(argv[1], ":");
	while (token != NULL) {
		AtoH(token, (char *) &mac_addr[i], 1);
		i++;
		if (i >= MAC_ADDR_LEN)
			break;
		token = strtok(NULL, ":");
	}

	wapp_send_btm_req_by_case(wapp, iface, mac_addr, BTM_STA_STEER_TO_CELL);
	return MBO_SUCCESS;
}

int mbo_cmd_disassoc_sta( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 mac_addr[MAC_ADDR_LEN];
	char *token;
	int i;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	i = 0;
	token = strtok(argv[1], ":");
	while (token != NULL) {
		AtoH(token, (char *) &mac_addr[i], 1);
		i++;
		if (i >= MAC_ADDR_LEN)
			break;
		token = strtok(NULL, ":");
	}

	wapp_send_btm_req_by_case(wapp, iface, mac_addr, BTM_DISASSOC_STA);
	return MBO_SUCCESS;
}

int mbo_cmd_bss_term( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct wapp_conf *conf;
	struct wapp_dev *wdev = NULL;
	u8 is_found=0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wapp->mbo->assoc_retry_delay = 0;
	dl_list_for_each(conf, &wapp->conf_list ,struct wapp_conf,list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	wdev = wapp_dev_list_lookup_by_ifname(wapp, iface);

	if (wdev && !is_found)
		conf = wapp->wapp_default_config;
		
	if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
		u8 i;				
		struct wapp_sta *sta;
		struct ap_dev *ap = (struct ap_dev	*)wdev->p_dev;

		for (i = 0; i < ap->num_of_clients; i++)
		{
			sta = ap->client_table[i];
			if (sta && sta->sta_status == WAPP_STA_CONNECTED) {
				wapp_send_btm_req_by_case(
						wapp,
						iface,
						sta->mac_addr,
						BTM_AP_TERMINATE);
			}
		}
	}

	return MBO_SUCCESS;
}

int mbo_cmd_show_nrlist( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 i = 0;
	printf("\033[1;36m %s, wapp->daemon_nr_list.CurrListNum %d\033[0m\n", __FUNCTION__, wapp->daemon_nr_list.CurrListNum);  /* Haipin Debug Print (B)*/

	for(i=0;i<wapp->daemon_nr_list.CurrListNum;i++){
	
		printf("\033[1;36m No.%d %02X:%02X:%02X:%02X:%02X:%02X  Pref %d BssidInfo 0x%X  ChNum %d OpClass %d PhyType %d \033[0m\n"
			,i
			,PRINT_MAC(wapp->daemon_nr_list.NRInfo[i].Bssid)
			,wapp->daemon_nr_list.NRInfo[i].CandidatePref
			,wapp->daemon_nr_list.NRInfo[i].BssidInfo
			,wapp->daemon_nr_list.NRInfo[i].ChNum
			,wapp->daemon_nr_list.NRInfo[i].RegulatoryClass
			,wapp->daemon_nr_list.NRInfo[i].PhyType);  /* Haipin Debug Print (B)*/
	}
	
	return MBO_SUCCESS;
}

int mbo_cmd_show_reduced_nrlist( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 i = 0;
	printf("\033[1;36m %s, wapp->daemon_nr_list.CurrListNum %d\033[0m\n", __FUNCTION__, wapp->daemon_nr_list.CurrListNum);  /* Haipin Debug Print (B)*/

	for(i=0;i<wapp->daemon_nr_list.CurrListNum;i++){
	
		printf("\033[1;36m No.%d %02X:%02X:%02X:%02X:%02X:%02X  OpClass %d ChNum %d \033[0m\n"
			,i
			,PRINT_MAC(wapp->daemon_nr_list.NRInfo[i].Bssid)
			,wapp->daemon_nr_list.NRInfo[i].RegulatoryClass
			,wapp->daemon_nr_list.NRInfo[i].ChNum); 
	}
	
	return MBO_SUCCESS;
}

int mbo_cmd_show_mbo_cfg( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct mbo_cfg *mbo = wapp->mbo;
	struct btm_cfg *btm = &wapp->protocol.btm;
	DBGPRINT_RAW(RT_DEBUG_OFF, "mbo cfg:\n");
	DBGPRINT_RAW(RT_DEBUG_OFF, "\t cdcp = %u\n", mbo->cdcp);
	DBGPRINT_RAW(RT_DEBUG_OFF, "\t assoc_disallow_reason = %u\n", mbo->assoc_disallow_reason);
	DBGPRINT_RAW(RT_DEBUG_OFF, "\t assoc_retry_delay = %u\n", mbo->assoc_retry_delay);
	DBGPRINT_RAW(RT_DEBUG_OFF, "\t dft_trans_reason = %u\n", mbo->dft_trans_reason);
	DBGPRINT_RAW(RT_DEBUG_OFF, "\t ap_capability = %u\n", mbo->ap_capability);
#if 0
	DBGPRINT_RAW(RT_DEBUG_OFF, "\t btm_bss_termination_duration = %u\n", mbo->btm_bss_termination_duration);
	DBGPRINT_RAW(RT_DEBUG_OFF, "\t btm_bss_termination_tsf = %u\n", mbo->btm_bss_termination_tsf);
	DBGPRINT_RAW(RT_DEBUG_OFF, "\t btm_disassociation_timer = %u\n", mbo->btm_disassociation_timer);
#endif
	DBGPRINT_RAW(RT_DEBUG_OFF, "\t btm_bss_termination_onoff = %u\n", btm->bss_termination_onoff);
	DBGPRINT_RAW(RT_DEBUG_OFF, "\t disassoc_imnt = %u\n", btm->disassoc_imnt);
	DBGPRINT_RAW(RT_DEBUG_OFF, "\t nebor_bssid = %02x:%02x:%02x:%02x:%02x:%02x\n",
                                   PRINT_MAC(btm->nebor_bssid));
	DBGPRINT_RAW(RT_DEBUG_OFF, "\t nebor_op_class = %u\n", btm->nebor_op_class);
	DBGPRINT_RAW(RT_DEBUG_OFF, "\t nebor_op_ch = %u\n", btm->nebor_op_ch);
	DBGPRINT_RAW(RT_DEBUG_OFF, "\t nebor_pref = %u\n", btm->nebor_pref);
	return MBO_SUCCESS;
}

int mbo_cmd_set_ap_cap( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct mbo_cfg *mbo = wapp->mbo;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	u8 cap = 0;

	if(argv[1])
		cap = atoi(argv[1]);
	else
		printf(" %s, NULL cap, return\n", __FUNCTION__);  

	mbo->ap_capability = cap;

	mbo_param_setting(wapp,iface,PARAM_MBO_AP_CAP,mbo->ap_capability);

	printf(" %s, set ap_capability %d\n", __FUNCTION__,cap);
	
	return MBO_SUCCESS;
}

int mbo_cmd_set_assoc_disallow( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct mbo_cfg *mbo = wapp->mbo;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	u8 cap = 0;

	if(argv[1])
		cap = atoi(argv[1]);
	else
		printf(" %s, NULL cap, return\n", __FUNCTION__);  

	mbo->assoc_disallow_reason = cap;

	/* mbo assoc disallow */
	mbo_param_setting(wapp, iface, PARAM_MBO_AP_ASSOC_DISALLOW, wapp->mbo->assoc_disallow_reason);
	
#ifdef MAP_R2
	/* Send assoc status notification to controller/agents */
	map_send_assoc_notification(wapp, iface, wapp->mbo->assoc_disallow_reason);
#endif	
	
	return MBO_SUCCESS;
}

int mbo_cmd_set_ap_cdcp( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct mbo_cfg *mbo = wapp->mbo;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	u8 cap = 0;

	if(argv[1])
		cap = atoi(argv[1]);
	else
		printf(" %s, NULL cdcp, return\n", __FUNCTION__);  

	mbo->cdcp = cap;

	/* mbo ap cdcp */
	mbo_param_setting(wapp, iface, PARAM_MBO_AP_CDCP, wapp->mbo->cdcp);
	
	printf(" %s, set cdcp %d\n", __FUNCTION__,cap);
	
	return MBO_SUCCESS;
}

int mbo_cmd_set_assoc_retry_delay( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct mbo_cfg *mbo = wapp->mbo;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	u8 cap = 0;

	if(argv[1])
		cap = atoi(argv[1]);
	else
		printf(" %s, NULL ,return\n", __FUNCTION__);  

	mbo->assoc_retry_delay = cap;
	
	printf(" %s, set assoc_retry_delay %d\n", __FUNCTION__,cap);
	
	return MBO_SUCCESS;
}

int mbo_cmd_set_bss_termination_onoff( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct btm_cfg *btm = &wapp->protocol.btm;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	u8 value = 0;
	
	printf(" %s, set btm_bss_termination_flag %d\n", __FUNCTION__,value);

	if(argv[1])
		value = atoi(argv[1]);
	else
		printf(" %s, NULL , return\n", __FUNCTION__);  

	btm->bss_termination_onoff = value;
	
	printf(" %s, set btm_bss_termination_flag %d\n", __FUNCTION__,value);
	
	return MBO_SUCCESS;
}

int mbo_cmd_set_bss_termination_duration( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 value = 0;
	struct wapp_conf *conf;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	dl_list_for_each(conf, &wapp->conf_list ,struct wapp_conf,list)
		if (os_strcmp(conf->iface, iface) == 0)
			break;


	if(argv[1])
		value = atoi(argv[1]);
	else
		printf(" %s, NULL , return\n", __FUNCTION__);  

	conf->bss_termination_duration = value;
	
	printf(" %s, set btm_bss_termination_duration %d\n", __FUNCTION__,value);
	
	return MBO_SUCCESS;
}

int mbo_cmd_set_btm_bss_termination_tsf( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 value = 0;
	struct wapp_conf *conf;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	dl_list_for_each(conf, &wapp->conf_list ,struct wapp_conf,list)
		if (os_strcmp(conf->iface, iface) == 0)
			break;

	if(argv[1])
		value = atoi(argv[1]);
	else
		printf(" %s, NULL , return\n", __FUNCTION__);  

	conf->bss_termination_tsf = value;
	
	printf(" %s, set btm_bss_termination_tsf %d\n", __FUNCTION__, value);
	
	return MBO_SUCCESS;
}

int mbo_cmd_set_btm_disassociation_timer( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 value = 0;
	struct wapp_conf *conf;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	dl_list_for_each(conf, &wapp->conf_list ,struct wapp_conf,list)
		if (os_strcmp(conf->iface, iface) == 0)
			break;

	if(argv[1])
		value = atoi(argv[1]);
	else
		printf(" %s, NULL , return\n", __FUNCTION__);  

	conf->disassociation_timer = value;
	
	printf(" %s, set btm_disassociation_timer %d\n", __FUNCTION__, value);
	
	return MBO_SUCCESS;
}

int mbo_cmd_set_ap_disassoc_imnt( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct btm_cfg *btm = &wapp->protocol.btm;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	u8 value = 0;

	if(argv[1])
		value = atoi(argv[1]);
	else
		printf(" %s, NULL avlue, return\n", __FUNCTION__);  

	btm->disassoc_imnt = value;

	printf(" %s, set disassoc_imnt %d\n", __FUNCTION__, value);
	
	return MBO_SUCCESS;
}

int mbo_cmd_set_nebor_bssid( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct btm_cfg *btm = &wapp->protocol.btm;
	u8 *mac_addr = btm->nebor_bssid;
	char *token;
	int i;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(argv[1]) {
	    i = 0;
	    token = strtok(argv[1], ":");
	    while (token != NULL) {
	        AtoH(token, (char *) &mac_addr[i], 1);
	        i++;
	        if (i >= MAC_ADDR_LEN)
	            break;
			token = strtok(NULL, ":");
	        }
	}
	else
	    printf(" %s, NULL avlue, return\n", __FUNCTION__);  

	printf(" %s, set nebor_bssid = %02x:%02x:%02x:%02x:%02x:%02x\n", 
               __FUNCTION__, PRINT_MAC(btm->nebor_bssid));
	
	return MBO_SUCCESS;
}

int mbo_cmd_set_nebor_op_class( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct btm_cfg *btm = &wapp->protocol.btm;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	u8 value = 0;

	if(argv[1])
		value = atoi(argv[1]);
	else
		printf(" %s, NULL avlue, return\n", __FUNCTION__);  

	btm->nebor_op_class = value;

	printf(" %s, set nebor_op_class %d\n", __FUNCTION__, value);
	
	return MBO_SUCCESS;
}

int mbo_cmd_set_nebor_op_ch( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct btm_cfg *btm = &wapp->protocol.btm;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	u8 value = 0;

	if(argv[1])
		value = atoi(argv[1]);
	else
		printf(" %s, NULL avlue, return\n", __FUNCTION__);  

	btm->nebor_op_ch = value;

	printf(" %s, nebor_op_ch %d\n", __FUNCTION__, value);
	
	return MBO_SUCCESS;
}

int mbo_cmd_set_nebor_pref( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct btm_cfg *btm = &wapp->protocol.btm;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	u8 value = 0;

	if(argv[1])
		value = atoi(argv[1]);
	else
		printf(" %s, NULL avlue, return\n", __FUNCTION__);  

	btm->nebor_pref = value;

	printf(" %s, nebor_pref %d\n", __FUNCTION__, value);
	
	return MBO_SUCCESS;
}

int mbo_cmd_add_test_nr( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct btm_cfg *btm = &wapp->protocol.btm;
	u8 ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	/* Add the entry which is set by cmd */
	if (os_memcmp(btm->nebor_bssid , ZERO_MAC_ADDR, MAC_ADDR_LEN) != 0) {
	    wapp_nr_info nr_info;

	    os_memset(&nr_info, 0, sizeof(wapp_nr_info));
	    COPY_MAC_ADDR(nr_info.Bssid, btm->nebor_bssid);
	    nr_info.RegulatoryClass = btm->nebor_op_class;
	    nr_info.ChNum = btm->nebor_op_ch;
	    nr_info.CandidatePref = btm->nebor_pref;
	    nr_entry_add(wapp, &nr_info);
	}
	
	printf(" %s\n", __FUNCTION__);
	return MBO_SUCCESS;
}

int mbo_cmd_send_btm_req( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 mac_addr[MAC_ADDR_LEN] = {0};
	char *token;
	int i;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if(argv[1]) {
	    i = 0;
	    token = strtok(argv[1], ":");
	    while (token != NULL) {
	        AtoH(token, (char *) &mac_addr[i], 1);
	        i++;
	        if (i >= MAC_ADDR_LEN)
	            break;
	        token = strtok(NULL, ":");
	        }
	}
	else
	    memset(mac_addr, 0, MAC_ADDR_LEN);
	    
	printf(" %s, %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__, PRINT_MAC(mac_addr));

	mbo_send_btm_req_by_cmd( wapp, iface, mac_addr);

	printf(" %s, mac addr = %02x:%02x:%02x:%02x:%02x:%02x\n", 
               __FUNCTION__, PRINT_MAC(mac_addr));
	
	return MBO_SUCCESS;
}

int mbo_cmd_reset_default( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	os_memset(&wapp->daemon_nr_list, 0, sizeof(DAEMON_NR_LIST));
	return MBO_SUCCESS;
}

struct mbo_ctrl_cmd mbo_cmd[] = {
	{"steer_sta",         mbo_cmd_steer_sta,                      "btm steer a sta"},
	{"disassoc_sta",      mbo_cmd_disassoc_sta,                   "btm disassoc a sta, disassoc imnt"},
	{"bss_term",          mbo_cmd_bss_term,                       "btm bss terminate, disassoc all sta"},
	{"steer_sta_to_cell", mbo_cmd_steer_sta_to_cell,              "btm steer a sta to cellular"},	
	{"mbo_cfg",           mbo_cmd_show_mbo_cfg,                   "show mbo cfg"},
	{"ap_cap",            mbo_cmd_set_ap_cap,                     "set mbo ap capability (runtime)"},
	{"ap_cdcp",           mbo_cmd_set_ap_cdcp,                    "set mbo ap cellular preference (runtime)"},
	{"assoc_disallow",    mbo_cmd_set_assoc_disallow,             "set mbo ap assoc disallow reason (runtime)"},
	{"retry_delay",       mbo_cmd_set_assoc_retry_delay,          "set association retry delay (second) (runtime)"},
	{"bss_term_onoff",    mbo_cmd_set_bss_termination_onoff,      "set mbo bss term flag (btm test)"},
	{"bss_term_duration", mbo_cmd_set_bss_termination_duration,   "set how long the bss will be down (minute) (runtime)"},
	{"bss_term_tsf",      mbo_cmd_set_btm_bss_termination_tsf,    "set how long before bss shutdown (tsf) (runtime)"},
	{"disassoc_timer",    mbo_cmd_set_btm_disassociation_timer,   "set how long before ap sending dis-assoctiation (100ms) (runtime)"},
	{"disassoc_imnt",     mbo_cmd_set_ap_disassoc_imnt,           "set mbo disassoc imnt bit of btm (runtime)"},
	{"nebor_bssid",       mbo_cmd_set_nebor_bssid,                "set mbo nebor bssid(runtime)"},
	{"nebor_op_class",    mbo_cmd_set_nebor_op_class,             "set mbo nebor op class (runtime)"},
	{"nebor_op_ch",       mbo_cmd_set_nebor_op_ch,                "set mbo nebor_op_class(runtime)"},
	{"nebor_pref",        mbo_cmd_set_nebor_pref,                 "set mbo nebor_pref(runtime)"},
	{"nrlist",            mbo_cmd_show_nrlist,                    "dump neighbor report list"},
	{"reduced_nrlist",    mbo_cmd_show_reduced_nrlist,           "dump reduced neighbor report list"},
	{"add_test_nr",       mbo_cmd_add_test_nr,                    "push the test nr to nr_list"},
	{"send_btm_req",      mbo_cmd_send_btm_req,                   "send a btm req for test purpose"},
	{"reset_default",     mbo_cmd_reset_default,                  "clear NR List"},
	{"help",              mbo_cmd_show_help,                      "show this help"},
	{NULL,                mbo_cmd_show_help}	
};

int mbo_cmd_show_help( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 i = 0;

	printf("\033[1;36m available cmds: \033[0m\n");
	for(i=0;(mbo_cmd[i].cmd != NULL);i++){	
		printf("\033[1;36m %20s  \t -  %s\033[0m\n",mbo_cmd[i].cmd,mbo_cmd[i].help); 
	}
	
	return MBO_SUCCESS;
}

int mbo_ctrl_interface_cmd_handle(
	struct wifi_app *wapp,
	const char *iface,
	u8 argc,
	char **argv)
{
	int i, ret;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);	
	for (i = 0; mbo_cmd[i].cmd != NULL; i++)
	{
		if (os_strncmp(mbo_cmd[i].cmd, argv[0], os_strlen(argv[0])) == 0) {
			ret = mbo_cmd[i].cmd_proc(wapp, iface, argc, argv);
			if (ret != MBO_SUCCESS)
				DBGPRINT(RT_DEBUG_ERROR, "cmd [%s] failed. ret = %d\n",	mbo_cmd[i].cmd, ret);
			break;
		}
	}

	return MBO_SUCCESS;
}


size_t wapp_calc_mbo_anqp_rsp_len(struct wifi_app *wapp,
									struct wapp_conf *conf,
									size_t mbo_anqp_req_len,
									const char *curpos)
{
	size_t varlen = 0, curlen = 0;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	while (curlen < mbo_anqp_req_len) {
		switch(*curpos) {			
			case MBO_CDCP:
				if (!conf->query_ap_cdcp) {
					DBGPRINT(RT_DEBUG_ERROR, "conf->query_ap_cdcp = 1!!\n");
					varlen = 9; /* MBO ANQP Elem Header len */					
					varlen += sizeof(wapp->mbo->cdcp);
					conf->query_ap_cdcp = 1;
				}							
				break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, "Unknown MBO subtype %d!!!\n",*curpos);
				break; 
		}	

		curpos ++;
		curlen ++;
	}

	return varlen;
}

int wapp_collect_mbo_anqp_rsp(struct wifi_app *wapp,
							struct wapp_conf *conf, char *buf)
{
	unsigned char *pos;
	struct mbo_anqp_frame *mbo_anqp_rsp = (struct mbo_anqp_frame *)buf;
	const char wfa_oi[3] = {0x50, 0x6F, 0x9A};
	u16 tmplen = 0;	

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	//MBO CDCP
	if (conf->query_ap_cdcp) {			
		DBGPRINT(RT_DEBUG_ERROR, "STA query AP CDCP - vendor specific\n");
		DBGPRINT(RT_DEBUG_ERROR, "Collect AP CDCP %d\n",wapp->mbo->cdcp);		
		tmplen = 0;
		mbo_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
		os_memcpy(mbo_anqp_rsp->oi, wfa_oi, 3);
		tmplen += 3;
		mbo_anqp_rsp->type = WFA_TIA_MBO;
		tmplen++;
		mbo_anqp_rsp->subtype = MBO_CDCP;
		tmplen += 1;

		pos = mbo_anqp_rsp->variable;
		*pos = wapp->mbo->cdcp;
		tmplen += 1;
		pos += 1;
		
		mbo_anqp_rsp->length = cpu2le16(tmplen);		
		mbo_anqp_rsp = (struct mbo_anqp_frame *)pos;
		
		conf->query_ap_cdcp = 0;

	}
	hex_dump("final_buf",(u8 *)buf,10);
	
	return 0;
}

#endif /* MBO_SUPPORT */

