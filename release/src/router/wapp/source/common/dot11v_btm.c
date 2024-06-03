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
#include "wapp_cmm.h"
#include "wps.h"

/* Send BTM pkt content to driver */
int wapp_send_btm_req(struct wifi_app *wapp,
						 const char *iface,
						 const u8 *peer_mac_addr,
						 const char *btm_req,
						 size_t btm_req_len)
{
	int ret;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	DBGPRINT(RT_DEBUG_ERROR, "%s  peer_mac_addr %02X:%02X:%02X:%02X:%02X:%02X len %d\n"
		, __FUNCTION__,PRINT_MAC(peer_mac_addr),(UINT32)btm_req_len);
	
	ret = wapp->drv_ops->drv_send_btm_req(wapp->drv_data, iface, peer_mac_addr,
										btm_req, btm_req_len);

	return ret;
}

/* Send BTM pkt content to driver */
int wapp_send_btm_rsp(struct wifi_app *wapp,
		struct wapp_dev *wdev,
		const char *iface,
		enum bss_trans_mgmt_status_code status_code,
		u8 bss_term_delay, const u8 *peer_mac_addr,
		const u8 *target_bssid, u8 ChNum)
{
	int ret;
	char btm_rsp_buf[1024] = {0};
	size_t btm_rsp_len = 0;
	struct btm_payload *frame;
	char *pos = btm_rsp_buf;
	char cmd[1024] = {0};
	
	DBGPRINT(RT_DEBUG_TRACE, "%s  peer_mac_addr %02X:%02X:%02X:%02X:%02X:%02X\n"
			, __FUNCTION__, PRINT_MAC(peer_mac_addr));

	frame = (struct btm_payload *)btm_rsp_buf;
	frame->u.btm_rsp.status_code = status_code;
	pos += 1;
	btm_rsp_len += 1;

	frame->u.btm_rsp.bss_termination_delay = bss_term_delay;
	pos += 1;
	btm_rsp_len += 1;

	if (target_bssid) {
		COPY_MAC_ADDR(frame->u.btm_rsp.variable, target_bssid);
		pos += MAC_ADDR_LEN;
		btm_rsp_len += MAC_ADDR_LEN;
	} else {
		COPY_MAC_ADDR(frame->u.btm_rsp.variable, "\0\0\0\0\0\0");
		pos += MAC_ADDR_LEN;
		btm_rsp_len += MAC_ADDR_LEN;
	}

	ret = wapp->drv_ops->drv_send_btm_rsp(wapp->drv_data, iface, peer_mac_addr,
			btm_rsp_buf, btm_rsp_len);

	if (target_bssid && MAC_ADDR_EQUAL(target_bssid, "\0\0\0\0\0\0"))
		return 0;

	sprintf(cmd, "iwpriv %s set ApCliEnable=0;", iface);
	system(cmd);
	os_memset(cmd, 0, sizeof(cmd));

	sprintf(cmd, "iwpriv %s set AutoRoaming=1;", iface);
	system(cmd);
	os_memset(cmd, 0, sizeof(cmd));

	if (target_bssid)
		sprintf(cmd, "iwpriv %s set ApCliBssid=%02x:%02x:%02x:%02x:%02x:%02x;",
			iface, PRINT_MAC(target_bssid));
	system(cmd);
	os_memset(cmd, 0, sizeof(cmd));

	if (ChNum)
		wdev_set_ch(wapp, wdev, ChNum, 0);

	sprintf(cmd, "iwpriv %s set ApCliEnable=1;", iface);
	system(cmd);

	return ret;
}

/*
octet	|1		 |2		|3			 |4			  |5~6				   |7				 | 0 or 12 (8~19)			 | variable				   | variable							  |
		|Category|Action|Dialog Token|Request Mode|Disassociation Timer|Validity Interval|	BSS Termination Duration | Session Information URL |BSS Transition Candidate List Entries |

*/

size_t wapp_build_btm_req(
	u8 req_mode,
	u16 disassoc_timer,
	u8 vad_intvl,
	struct neighbor_report_subelement *bss_term_dur,
	char *url,
	size_t	 url_len,
	char *cand_list,
	size_t cand_list_len,
	char *btm_req_buf)
{
	size_t btm_req_len = 0;
	struct btm_payload *frame;
	char *pos = btm_req_buf;
	struct neighbor_report_subelement *report_subelement;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	frame = (struct btm_payload *)btm_req_buf;

	frame->u.btm_req.request_mode = req_mode;
	pos += 1;
	btm_req_len += 1;

	frame->u.btm_req.disassociation_timer = cpu2le16(disassoc_timer);
	pos += 2;
	btm_req_len += 2;

	frame->u.btm_req.validity_interval = vad_intvl;
	pos += 1;
	btm_req_len += 1;


	if (bss_term_dur) {
		report_subelement = (struct neighbor_report_subelement *)pos;
		report_subelement->subelement_id = BSS_TERMINATION_DURATION;
		report_subelement->length = 10;
		report_subelement->u.bss_termination_duration.bss_termination_tsf = 
											cpu2le64(bss_term_dur->u.bss_termination_duration.bss_termination_tsf);
		report_subelement->u.bss_termination_duration.duration = 
											cpu2le16(bss_term_dur->u.bss_termination_duration.duration);
		frame->u.btm_req.request_mode |= (1 << BSS_TERM_INCLUDED_BIT_MAP);
		pos += 12;
		btm_req_len += 12;
	}

	/* URL is included only when ESS Disassociation Imminent is set to 1 */
	if ((req_mode & (1 << ESS_DISASSOC_IMNT_BIT_MAP)) && url) {
		/* session url length */
		*pos = url_len;
		pos++;
		btm_req_len++;

		/* session url */
		os_memcpy(pos, url, url_len);
		pos += url_len;
		btm_req_len += url_len;
	}
	
	if (cand_list) {
			frame->u.btm_req.request_mode |= (1 << CAND_LIST_INCLUDED_BIT_MAP);
			
			os_memcpy(pos, cand_list, cand_list_len);
			pos += cand_list_len;
			btm_req_len += cand_list_len;
	}

	return btm_req_len;
}


int wapp_send_btm_req_by_case(
	struct wifi_app *wapp,
    const char *iface,
	const u8 *mac_addr,
	u8	trans_case)
{
	struct wapp_sta *sta = NULL;
	struct wapp_conf *conf;
	u8 req_mode = 0, disassoc_imnt = 0, ess_disassoc_imnt = 0, abridged = 0;
	u16 disassoc_timer = 0;
	u8 has_trans_reason = FALSE, has_reassoc_delay = FALSE, is_steer_to_cell = FALSE;
	u8 validity_intvl = 0;
	char buf[1024] = {0};
	char cand_list[1024] = {0}, *p_cand_list = NULL;
	char *p_url = NULL;
	size_t btm_req_len = 0, cand_list_len = 0, url_len = 0;
	struct neighbor_report_subelement bss_term_dur, *p_bss_term_dur = NULL;
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	struct btm_cfg *btm = &wapp->protocol.btm;
	u8 is_found=0;
#ifdef KV_API_SUPPORT
	int status = 0;
#endif /* KV_API_SUPPORT */
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return WAPP_INVALID_ARG;

	wdev = wapp_dev_list_lookup_by_ifname(wapp, iface);

	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, 
			"wdev not found or dev type is not AP\n");
		return WAPP_LOOKUP_ENTRY_NOT_FOUND;
	}

	if (wdev->dev_type != WAPP_DEV_TYPE_AP) {
		DBGPRINT(RT_DEBUG_ERROR, 
				"dev type is not AP. (dev type = %u)\n", wdev->dev_type);
			return WAPP_UNEXP;
	}

	dl_list_for_each(conf, &wapp->conf_list ,struct wapp_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found)
		conf = wapp->wapp_default_config;

	if (!conf) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ERROR! conf is null\n", __func__, __LINE__);
		return -1;
	}
	
	ap = (struct ap_dev *) wdev->p_dev;
	sta = wdev_ap_client_list_lookup(wapp, ap, mac_addr);	
	

	if (!sta) {
		DBGPRINT(RT_DEBUG_ERROR, 
			"%02x:%02x:%02x:%02x:%02x:%02x, the sta is not in the list\n",
			PRINT_MAC(mac_addr));
		return WAPP_LOOKUP_ENTRY_NOT_FOUND;
	}
	/* Fill parameters for each case */
	switch(trans_case)
	{
		case BTM_STA_STEERING:
			//mbo_sta_steering_action(wapp, conf, iface, sta);			
			abridged = conf->abridged;
			disassoc_imnt = sta->disassoc_imnt;
			ess_disassoc_imnt = 0;	
			disassoc_timer = 0; /* 0 means that disassoc time is not set */
			validity_intvl = conf->validity_interval;
			p_bss_term_dur = NULL;
			p_url = NULL;
			url_len = 0;
			p_cand_list = cand_list;
			has_reassoc_delay = FALSE;
			has_trans_reason = TRUE;
			is_steer_to_cell = FALSE;
			break;

		case BTM_STA_STEER_TO_CELL:
			abridged = conf->abridged;
			disassoc_imnt = sta->disassoc_imnt;
			ess_disassoc_imnt = 0;	
			disassoc_timer = 0; /* 0 means that disassoc time is not set */
			validity_intvl = conf->validity_interval;
			p_bss_term_dur = NULL;
			p_url = NULL;
			url_len = 0;
			p_cand_list = cand_list;
			has_reassoc_delay = FALSE;
			has_trans_reason = TRUE;
			is_steer_to_cell = TRUE;
			break;
			
		case BTM_DISASSOC_STA:
			//mbo_disassoc_sta_btm_action(wapp, conf, iface, sta);
			abridged = conf->abridged;
			disassoc_imnt = 1;
			ess_disassoc_imnt = 0;	
			disassoc_timer = conf->disassociation_timer;
			validity_intvl = conf->validity_interval;
			p_bss_term_dur = NULL;
			p_url = NULL; /* (conf->have_session_info_url) ? conf->session_info_url : NULL */;
			url_len = 0;
			p_cand_list = cand_list;
			has_reassoc_delay = TRUE;
			has_trans_reason = TRUE;
			is_steer_to_cell = FALSE;
			break;

		case BTM_AP_TERMINATE:
			abridged = conf->abridged;
			/* Add a command "wappctrl INF mbo disassoc_imnt $vallue" in config_agent to
			 * make disassoc_imnt bit configurable and enable it only when test case 4.2.5.4
			 * is run i.e for certification only.
			 */
			if (btm->disassoc_imnt)
				disassoc_imnt = btm->disassoc_imnt;
			else
				disassoc_imnt = 0; /* changed from 1 to 0 per UCC 4.2.5.4, need confirm */
			ess_disassoc_imnt = 0;	
			disassoc_timer = conf->disassociation_timer;
			validity_intvl = conf->validity_interval;
			p_bss_term_dur = (conf->bss_termination_duration) ? 
								&bss_term_dur : NULL;
			p_url = NULL; /* (conf->have_session_info_url) ? conf->session_info_url : NULL */;
			url_len = 0;
			p_cand_list = cand_list;
			has_reassoc_delay = FALSE;
			has_trans_reason = TRUE;
			is_steer_to_cell = FALSE;
			break;		

		case BTM_QUERY_RSP:
			//mbo_sta_steering_action(wapp, conf, iface, sta);			
			abridged = conf->abridged;
			disassoc_imnt = 0;
			ess_disassoc_imnt = 0;	
			disassoc_timer = 0; /* 0 means that disassoc time is not set */
			validity_intvl = conf->validity_interval;
			p_bss_term_dur = NULL;
			p_url = NULL;
			url_len = 0;
			p_cand_list = cand_list;
			has_reassoc_delay = FALSE;
			has_trans_reason = FALSE;
			is_steer_to_cell = FALSE;
			break;


		default:
			printf("\033[1;33m %s, %u Unknown Transition Case %d\033[0m\n", __FUNCTION__, __LINE__,trans_case); 
			break;
	}

	/* build req_mode */
	/*
		Request Mode:
		|		0							|		1  |			2			 |			3				|			4				  |
		| Preferred Candidate List Included | Abridged | Disassociation Imminent | BSS Termination Included | ESS Disassociation Imminent | 
	
		Preferred Candidate List Included: set in wapp_build_btm_req() when there is cand_list.
		Abridged: align the setting in conf
		Disassociation Imminent = 0 in this case (in rps to a btm query)
		BSS Termination Included: set in wapp_build_btm_req() when there is bss_term_dur
		ESS Disassociation Imminent = 0 in this case (in rps to a btm query)
	*/
	req_mode = 	(abridged << ABIDGED_BIT_MAP) |
				(disassoc_imnt << DISASSOC_IMNT_BIT_MAP) |
				(ess_disassoc_imnt << ESS_DISASSOC_IMNT_BIT_MAP);
	
	/* build bss_term */
	if (p_bss_term_dur) {
		bss_term_dur.u.bss_termination_duration.bss_termination_tsf = \
								conf->bss_termination_tsf;
		bss_term_dur.u.bss_termination_duration.duration = \
								conf->bss_termination_duration;
	}

	if ((!disassoc_imnt) && sta->cli_caps.mbo_capable) {
		if (nr_list_lookup_by_mac_addr(wapp, sta->bssid) == NR_ENTRY_NOT_FOUND) {
			wapp_nr_info nr;

			os_memset(&nr, 0, sizeof(nr));
			COPY_MAC_ADDR(nr.Bssid, sta->bssid);
			nr.CandidatePref = 0xff;
			nr_entry_add(wapp, &nr);
		}
	}
	/* build cand_list */
	if( p_cand_list &&
		wapp->daemon_nr_list.CurrListNum > 0)
	{
		mbo_check_sta_preference_and_append_nr_list( wapp,
												sta,
												cand_list, 
												(u16 *) &cand_list_len, 
												MBO_FRAME_BTM,
												disassoc_imnt,
												(p_bss_term_dur ? TRUE:FALSE),
												is_steer_to_cell);
	}
hex_dump("1037====",(u8 *)buf , btm_req_len);
	/* build btm content */
	btm_req_len = wapp_build_btm_req(
					req_mode,
					disassoc_timer,
					validity_intvl,
					p_bss_term_dur,
					p_url,
					url_len,
					(cand_list_len) ? cand_list : NULL,
					cand_list_len,
					buf);
hex_dump("1049====",(u8 *)buf , btm_req_len);
	/* append MBO IE */
	{
		u16 ie_len = 0;
		char *pos = buf + btm_req_len;
		struct mbo_cfg *mbo = wapp->mbo;
		mbo_make_mbo_ie_for_btm(
					wapp,
					pos,
					&ie_len, 
					(sta->cell_data_cap) ? TRUE : FALSE,
					has_trans_reason, 
					(sta->trans_reason) ? sta->trans_reason : mbo->dft_trans_reason,
					has_reassoc_delay);
		btm_req_len += ie_len;
	}
hex_dump("1068====",(u8 *)buf , btm_req_len);
	/* Send this BTM Req */
#ifndef KV_API_SUPPORT
	wapp_send_btm_req(wapp, iface, sta->mac_addr, buf, btm_req_len);
	return WAPP_SUCCESS;
#else
	status = wapp_send_btm_req_11kv_api(wapp, wdev->ifname, sta->mac_addr, buf, btm_req_len);
	return status;
#endif /* KV_API_SUPPORT */
}

/* Parse BTM Request - Neighbor Report Element */
int wapp_parse_nr_elem(
		struct wifi_app *wapp, void *nr_elem)
{
	struct neighbor_report_element *nrElem =
		(struct neighbor_report_element *)nr_elem;
	struct neighbor_report_subelement *nrSubElem = NULL;
	wapp_nr_info nrInfo;
	u8 parseLen = 0;
	int ret = WAPP_SUCCESS;

	if ((nrElem == NULL) || (nrElem->length < sizeof(struct neighbor_report_element))) {
		if (nrElem) {
			DBGPRINT(RT_DEBUG_ERROR, "WAPP: %s  nrElem_len %d\n",
					__func__, (UINT32)nrElem->length);
		} else {
			DBGPRINT(RT_DEBUG_ERROR, "WAPP: %s  nrElem == NULL\n",
					__func__);
		}
		return WAPP_INVALID_ARG;
	}
	/* Subtract 2 from sizeof(struct neighbor_report_element) to skip EID & Length */
	os_memcpy(&nrInfo, &nrElem->bssid, sizeof(struct neighbor_report_element) - SKIP_EID_LEN);
	parseLen += sizeof(struct neighbor_report_element);
	/* Check if SubElement present */
	if (nrElem->length <= sizeof(struct neighbor_report_element)) {
		DBGPRINT(RT_DEBUG_ERROR, "WAPP: %s No SubElement\n", __func__);
		return WAPP_INVALID_ARG;
	}

	nrSubElem = (struct neighbor_report_subelement *)&nrElem->variable;
	while (parseLen < nrElem->length) {
		switch (nrSubElem->subelement_id) {
			case BSS_TRANSITION_CANDIDATE_PREF:
				nrInfo.CandidatePrefSubID = nrSubElem->subelement_id;
				nrInfo.CandidatePrefSubLen = nrSubElem->length;
				nrInfo.CandidatePref = nrSubElem->u.bss_transition_candi_preference.preference;
				ret = nr_entry_add(wapp, &nrInfo);
				if (ret == WAPP_RESOURCE_ALLOC_FAIL)
					return ret;
				break;
			case WIDE_BANDWIDTH_CHANNEL:
				break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, "WAPP: %s Unhandled SubElement %d\n",
						__func__, nrSubElem->subelement_id);
		}
		parseLen += 2 + nrSubElem->length;
		nrSubElem = (struct neighbor_report_subelement*)
			((u8 *)nrSubElem + 2 + nrSubElem->length);
	}

	return ret;
}

int nr_list_get_target_bss(struct wifi_app *wapp,
		u8 *target_bssid, u8 *ChNum)
{
	int i = 0;
	u8 pref = 0;

	if(wapp->daemon_nr_list.CurrListNum >= DAEMON_NEIGHBOR_REPORT_MAX_NUM - 1) {
		DBGPRINT(RT_DEBUG_ERROR, "WAPP: %s candidate list is Empty %d\n", __func__, wapp->daemon_nr_list.CurrListNum);
		return WAPP_INVALID_ARG;
	}

	for (i=0; i<wapp->daemon_nr_list.CurrListNum; i++) {
		wapp_nr_info *nr_entry = &wapp->daemon_nr_list.NRInfo[i];
		if (nr_entry->CandidatePref > pref) {
			pref = nr_entry->CandidatePref;
			COPY_MAC_ADDR(target_bssid, nr_entry->Bssid);
			*ChNum = nr_entry->ChNum;
		}
	}

	return WAPP_SUCCESS;
}

int wapp_event_btm_req(
		struct wifi_app *wapp,
		const char *iface,
		const u8 *peer_mac_addr,
		const char *btm_req,
		size_t btm_req_len)
{
	int ret;
	enum bss_trans_mgmt_status_code status;
	u8 target_bssid[MAC_ADDR_LEN] = {0};
	struct wapp_dev *wdev = NULL;
	u8 ChNum = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s  btm_req_len %d\n", __func__, (UINT32)btm_req_len);

	if (!wapp)
		return WAPP_INVALID_ARG;

	wdev = wapp_dev_list_lookup_by_ifname(wapp, iface);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_OFF, RED("%s wdev %s not found\n"), __FUNCTION__, iface);
		return WAPP_NOT_INITIALIZED;
	}

	if (wdev->dev_type != WAPP_DEV_TYPE_STA) {
		DBGPRINT(RT_DEBUG_ERROR,
				"dev type is not STA. (dev type = %u)\n", wdev->dev_type);
		return WAPP_UNEXP;
	}

	ret = nr_peer_btm_req_action(wapp, btm_req, btm_req_len, &status);

	 if (ret == WAPP_SUCCESS)
                ret = nr_list_get_target_bss(wapp, target_bssid, &ChNum);

	wapp_send_btm_rsp(wapp, wdev, iface, status, 0, peer_mac_addr, target_bssid, ChNum);

	return ret;
}

/* btm_rsp starts from dialog_token */
int wapp_event_btm_rsp(
        struct wifi_app *wapp,
        const char *iface,
        const u8 *peer_mac_addr,
        const char *btm_rsp,
        size_t btm_rsp_len)
{
	u8 dialog_token = *btm_rsp;
#ifdef MAP_SUPPORT
	struct btm_payload *payload = (struct btm_payload *)(btm_rsp);
#else
	struct btm_payload *payload = (struct btm_payload *)(btm_rsp+1);
#endif /* MAP_SUPPORT */
	struct wapp_dev *wdev = NULL;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s  btm_rsp_len %d dialog_token %d\n", __func__,(UINT32)btm_rsp_len,dialog_token);
	
	wdev = wapp_dev_list_lookup_by_ifname(wapp, iface);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_OFF, RED("%s wdev %s not found\n"), __FUNCTION__, iface);
		return WAPP_NOT_INITIALIZED;
	}

#ifdef MAP_SUPPORT
	map_btm_rsp_action(wapp, wdev, peer_mac_addr, payload);
#endif /* MAP_SUPPORT */
	
	return WAPP_SUCCESS;
}

int wapp_event_btm_query(struct wifi_app *wapp,
						    const char *iface,
						    const u8 *peer_addr,
						    const char *btm_query,
						    size_t btm_query_len)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
#ifdef MAP_R2
	map_send_btm_tunneled_message(wapp, peer_addr, btm_query, btm_query_len);
#endif /* MAP_SUPPORT */
	
	ret = nr_peer_btm_query_action(wapp, btm_query, btm_query_len);
	
	ret += wapp_send_btm_req_by_case(wapp, iface, peer_addr, BTM_QUERY_RSP);

	return ret;
}

int wapp_event_wnm_notify_req(struct wifi_app *wapp,
						 const char *iface,
						 const u8 *peer_mac_addr,
						 const char *wnm_req,
						 size_t wnm_req_len)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

#ifdef MAP_R2
	map_send_wnm_tunneled_message(wapp, peer_mac_addr, wnm_req, wnm_req_len);
#endif /* MAP_SUPPORT */

	return ret;
}

int wapp_send_wnm_notify_req(struct wifi_app *wapp, const char *iface,
						 const char *peer_mac_addr,
						 const char *wnm_req,
						 size_t wnm_req_len,
						 int	type)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = wapp->drv_ops->drv_send_wnm_notify_req(wapp->drv_data, iface, peer_mac_addr,
										wnm_req, wnm_req_len, type);

	return ret;
}

/* Send reduced neighbor report to driver */
int wapp_send_reduced_nr_list(struct wifi_app *wapp,
						 const char *iface,
						 const char *reduced_nr_list,
						 size_t reduced_nr_list_len)
{
	int ret;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	DBGPRINT(RT_DEBUG_ERROR, "%s len %d\n"
		, __FUNCTION__,(UINT32)reduced_nr_list_len);

	ret = wapp->drv_ops->drv_send_reduced_nr_list(wapp->drv_data, iface,
										reduced_nr_list, reduced_nr_list_len);

	return ret;
}

size_t wapp_build_reduced_nr_list(
	char *cand_list,
	size_t cand_list_len,
	char *reduced_nr_list_buf)
{
	size_t reduced_nr_list_len = 0;
	char *pos = reduced_nr_list_buf;

	if (cand_list) {	
		os_memcpy(pos, cand_list, cand_list_len);
		pos += cand_list_len;
		reduced_nr_list_len += cand_list_len;
	}

	return reduced_nr_list_len;
}

int wapp_send_reduced_nr_list_by_inf(
	struct wifi_app *wapp,
    const char *iface)
{
	struct wapp_conf *conf;
	char buf[1024];
	char cand_list[512];
	size_t reduced_nr_list_len = 0, cand_list_len = 0;
	struct wapp_dev *wdev = NULL;

	u8 is_found=0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return WAPP_INVALID_ARG;
	wdev = wapp_dev_list_lookup_by_ifname(wapp, iface);
	
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, 
			"wdev not found or dev type is not AP\n");
		return WAPP_LOOKUP_ENTRY_NOT_FOUND;
	}
	
	if (wdev->dev_type != WAPP_DEV_TYPE_AP) {
		DBGPRINT(RT_DEBUG_ERROR, 
				"dev type is not AP. (dev type = %u)\n", wdev->dev_type);
			return WAPP_UNEXP;
	}

	dl_list_for_each(conf, &wapp->conf_list ,struct wapp_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}
	
	if (!is_found)
		conf = wapp->wapp_default_config;

	/* build reduced neighbor report */
	{
		reduced_nr_list_len = mbo_append_reduced_nr_list( wapp,
									cand_list, 
									(u16 *) &cand_list_len);
		
	}
	/* build reduced neighbor report content */
	reduced_nr_list_len = wapp_build_reduced_nr_list(
					(cand_list_len) ? cand_list : NULL,
					cand_list_len,
					buf);
	/* Send this reduced neighbor list */
	wapp_send_reduced_nr_list(wapp, iface, buf, reduced_nr_list_len);
	return WAPP_SUCCESS;
}


