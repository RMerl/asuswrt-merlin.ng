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
#include "hotspot.h"
#include "wapp_cmm.h"

int RTDebugLevel = RT_DEBUG_WARN;

//extern struct hotspot_drv_ops hotspot_drv_ranl_ops;

extern struct hotspot_drv_ops hotspot_drv_wext_ops;

/* hotspot parameter setting */
inline static int hs_param_setting(struct wifi_app *wapp, 
										struct wapp_conf *conf, 
										u32 param, u32 value)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	ret = wapp->hs->drv_ops->drv_hs_param_setting(wapp->drv_data, conf->iface, 
								    param, value);
	return ret;
}

int hotspot_init_param(struct wifi_app *wapp, struct wapp_conf *conf)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	/* external_anqp_server_test */
	ret = hs_param_setting(wapp, conf, PARAM_EXTERNAL_ANQP_SERVER_TEST, 
											conf->external_anqp_server_test);
	if (ret)
		return -1;
	/* DGAF */
	ret = hs_param_setting(wapp, conf, PARAM_DGAF_DISABLED, conf->DGAF_disabled & 0x01);

	if (ret)
		return -1;

	/* Proxy ARP */
	/* From Spec, if DGAF Disabel bit set to 1, Proxy ARP service shall be enabled */
	if (conf->DGAF_disabled) {
		char value[3];
		snprintf(value, sizeof(value), "%s", "1");
		conf->proxy_arp = 1;
		wapp_ctrl_set_proxy_arp_param(conf, conf->confname, value);
	}

	ret = hs_param_setting(wapp, conf, PARAM_PROXY_ARP, conf->proxy_arp);

	if (ret)
		return -1;

	/* l2_filter */
        ret = hs_param_setting(wapp, conf, PARAM_L2_FILTER, conf->l2_filter);

        if (ret)
                return -1;

        /* icmpv4_deny */
        ret = hs_param_setting(wapp, conf, PARAM_ICMPV4_DENY, conf->icmpv4_deny);

        if (ret)
                return -1;
		
	if (!conf->legacy_osu_exist) {
		char asan_enable = 1;

		//set osu interface asan enable
		ret = hotspot_set_osu_asan(wapp, conf->osu_iface, &asan_enable, 1);
	
		//get legacy osu SSID
		ret += hotspot_get_legacy_osu_ssid(wapp, conf);

		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, "%s: single SSID case , please enable MBSSID if you need OSU function\n", __FUNCTION__);					
		}
		else
		{
			/*set OSEN DGAF=1, PROXY_ARP=1 */
			//ret = wapp_cmm_param_setting(wapp, conf, PARAM_DGAF_DISABLED, 1);
			ret = wapp->hs->drv_ops->drv_hs_param_setting(wapp->drv_data, conf->osu_iface,
									PARAM_DGAF_DISABLED, 1);
			if (ret)
				return -1;	
			ret = wapp->hs->drv_ops->drv_hs_param_setting(wapp->drv_data, conf->osu_iface,
									PARAM_PROXY_ARP, 1);

			if (ret)
				return -1;
		}
	}
	else if (conf->legacy_osu_exist == 1){
		char asan_enable = 0;
		
		//set osu interface for asan disable
		ret = hotspot_set_osu_asan(wapp, conf->osu_iface, &asan_enable, 1); 		
		
		//get legacy osu SSID
		ret += hotspot_get_legacy_osu_ssid(wapp, conf);

		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, "%s: single SSID case , please enable MBSSID if you need OSU function\n", __FUNCTION__);					
		}
		else
		{
			/*set OSEN DGAF=0 */
			//ret = wapp_cmm_param_setting(wapp, conf, PARAM_DGAF_DISABLED, 1);
			ret = wapp->hs->drv_ops->drv_hs_param_setting(wapp->drv_data, conf->osu_iface,
									PARAM_DGAF_DISABLED, 0);

			if (ret)
				return -1;
			ret = wapp->hs->drv_ops->drv_hs_param_setting(wapp->drv_data, conf->osu_iface,
									PARAM_PROXY_ARP, 0);

			if (ret)
				return -1;
		}
	}
	else if (conf->legacy_osu_exist == 2) {
		//test mode, don't check osu interface
		//get legacy osu SSID
		char tmpSSID[] = "OSU";
		conf->legacy_osu_ssidlen = os_strlen(tmpSSID);
		conf->legacy_osu_ssid = os_zalloc(conf->legacy_osu_ssidlen + 1);
		os_memcpy(conf->legacy_osu_ssid, tmpSSID, conf->legacy_osu_ssidlen);	
	}				

	/* BSS Load test */
	if (conf->qload_mode)
	{
		ret = hotspot_set_bss_load(wapp, conf);

		if (ret)
			return -1;
	}
	return 0;
}

static int hotspot_drv_ops_pre_check(struct wifi_app *wapp, const char *iface)
{
	int is_found = 0;
	struct wapp_conf *conf;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found)
		return -1;

	if (conf->hotspot_onoff)
		return 0;
	else {
		DBGPRINT(RT_DEBUG_ERROR, "hs daemon disable\n");
		return -1;
	}
}

	
int hotspot_get_bssid(struct wifi_app *wapp, struct wapp_conf *conf)
{
	int ret;
	size_t hessid_len = 6;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	ret = wapp->hs->drv_ops->drv_get_bssid(wapp->drv_data, conf->iface, conf->hessid, &hessid_len);

	if (hessid_len != 6) {
		DBGPRINT(RT_DEBUG_ERROR, "hessid_len is not six\n");
		return -1;
	}

	return ret;
}


int hotspot_deinit(struct wifi_app *wapp)
{
	int ret = 0;
	//struct hs_peer_entry *peer_entry, *peer_entry_tmp;
	
	DBGPRINT(RT_DEBUG_ERROR, "%s\n", __FUNCTION__);

#if 0
	dl_list_for_each_safe(peer_entry, peer_entry_tmp, &wapp->hs->hs_peer_list,
											struct hs_peer_entry, list) {
		dl_list_del(&peer_entry->list);
		os_free(peer_entry);
	}
#endif 

	/* deinit control interface */
	wapp_ctrl_iface_deinit(wapp);
	wapp_iface_deinit(wapp);
	wapp_deinit_all_config(wapp);
	map_bss_table_release(wapp->map);

	ret = wapp->drv_ops->drv_inf_exit(wapp);

	if (ret)
		return -1;

	return 0;
}

static void hotspot_terminate(int sig, void *signal_ctx)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	eloop_terminate();
}

int hotspot_ap_reload(struct wifi_app *wapp,
					  const char *iface)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	ret = hotspot_onoff(wapp, iface, 0, EVENT_TRIGGER_ON, HS_AP_RELOAD);
	
	return ret;
}

void hotspot_run(struct wifi_app *wapp)
{

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	eloop_register_signal_terminate(hotspot_terminate, wapp->hs);

	//eloop_register_signal_reconfig(hotspot_reconfig, hs);

	eloop_run();
}

inline int hotspot_ipv4_proxy_arp_list(struct wifi_app *wapp, const char *iface,
						   			   char *reply, size_t *reply_len)
{
	int ret;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = hotspot_drv_ops_pre_check(wapp, iface);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: hotspot drv ops pre check fail\n", __FUNCTION__);
		return -1;
	}

	ret = wapp->hs->drv_ops->drv_ipv4_proxy_arp_list(wapp->drv_data, iface, reply, reply_len);
	
	return ret;
}

inline int hotspot_ipv6_proxy_arp_list(struct wifi_app *wapp, const char *iface,
									   char *reply, size_t *reply_len)
{
	int ret;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = hotspot_drv_ops_pre_check(wapp, iface);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: hotspot drv ops pre check fail\n", __FUNCTION__);
		return -1;
	}

	ret = wapp->hs->drv_ops->drv_ipv6_proxy_arp_list(wapp->drv_data, iface, reply, reply_len);
	
	return ret;
}

inline int hotspot_set_osu_asan(struct wifi_app *wapp, const char *iface, char *enable, size_t len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = wapp->hs->drv_ops->drv_set_osu_asan(wapp->drv_data, iface, enable, len);
	
	return ret;
}


inline int hotspot_set_bss_load(struct wifi_app *wapp, struct wapp_conf *conf)
{
	int ret;
	char tmpbuf[10];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	tmpbuf[0] = conf->qload_mode;
	tmpbuf[1] = conf->qload_cu;
	memcpy(&tmpbuf[2], &conf->qload_sta_cnt, 2);
	
	ret = wapp->hs->drv_ops->drv_set_bss_load(wapp->drv_data, conf->iface, tmpbuf, 4);
	
	return ret;
}

int hotspot_get_legacy_osu_ssid(struct wifi_app *wapp, struct wapp_conf *conf)
{
	int ret = 0;
	char tmpbuf[256];
	
	conf->legacy_osu_ssidlen = 255;	
	DBGPRINT(RT_DEBUG_TRACE, "%s  \n", __FUNCTION__);
	ret = wapp->hs->drv_ops->drv_get_osu_ssid(wapp->drv_data, conf->osu_iface, tmpbuf, &conf->legacy_osu_ssidlen);

	if (!ret) {
		//int tmp;
		conf->legacy_osu_ssid = os_zalloc(conf->legacy_osu_ssidlen);
		os_memcpy(conf->legacy_osu_ssid, tmpbuf, conf->legacy_osu_ssidlen);
		DBGPRINT(RT_DEBUG_TRACE, "%s  SSID:[%s] SSID_len:%zu\n"
			, __FUNCTION__,conf->legacy_osu_ssid,conf->legacy_osu_ssidlen);
	}
	
	return ret;
}

static int hotspot_set_p2p_ie(struct wifi_app *wapp, struct wapp_conf *conf)
{
	struct p2p_info_element *ie;
	struct p2p_attribute *p2p_attri;
	char *buf, *pos;
	int ie_len = 0;
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "\n");

	buf = os_zalloc(sizeof(*ie) + 4);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not availale"));
		return -1;
	}
	
	ie = (struct p2p_info_element *)buf;

	ie->eid = 0xdd;
	ie_len += 1;

	ie->oui[0] = 0x50;
	ie->oui[1] = 0x6F;
	ie->oui[2] = 0x9A;
	ie_len += 3;
	
	/* <WFA ANA> */
	ie->oui_type = 0x09;
	ie_len += 1;

	/* P2P attribute */
	p2p_attri = (struct p2p_attribute *)ie->variable;

	/* P2P manageability attribute */
	p2p_attri->attribute_id = 0x0a;
	p2p_attri->length = cpu2le16(0x0001);
	pos = p2p_attri->variable;

	/* P2P Device Management bit */
	*pos = (*pos & ~0x01) | (0x01);

	/*  Cross Connection Permitted bit */
	*pos = (*pos & ~0x02) | (conf->p2p_cross_connect_permitted & 0x01);

	ie_len += 4;
	
	ie->length = ie_len - 1;
	ie_len += 1;

	ret = wapp_set_ie(wapp, conf->iface, buf, ie_len);

	os_free(buf);

	return ret;
}

static int hotspot_set_hs_indication_ie(struct wifi_app *wapp, struct wapp_conf *conf)
{
	struct hotspot2dot0_indication_element *ie;
	char *buf;
	int ie_len = 0;
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "\n");
	
	buf = os_zalloc(sizeof(*ie));

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not availale"));
		return -1;
	}

	ie = (struct hotspot2dot0_indication_element *)buf;

	ie->eid = IE_HS2_INDICATION;
	ie_len += 1;

	ie->oi[0] = 0x50;
	ie->oi[1] = 0x6F;
	ie->oi[2] = 0x9A;
	ie_len += 3;

	/* <WFA ANA> */
	ie->type = 0x10;
	ie_len += 1;
	
	/* Hotspot Configuration */
	ie->hotspot_conf = (conf->DGAF_disabled & 0x01)|((conf->anqp_domain_id & 0x01)<<2)|((wapp->hs->version-1)<<5);

	ie_len += 1;

	ie->anqp_domain_id = conf->anqp_domain_id;
	ie_len += 2;

	ie->length = ie_len - 1;
	ie_len += 1;

	ret = wapp_set_ie(wapp, conf->iface, buf, ie_len);

	os_free(buf);

	return ret;
}

static int hotspot_set_qosmap_ie(struct wifi_app *wapp, struct wapp_conf *conf)
{
	struct qosmap_element *ie;
	char *buf, *pos;
	int ie_len = 0, varlen = 0, field = 0;
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "\n");
	
	varlen = conf->dscp_field*2;
	buf = os_zalloc(sizeof(*ie) + varlen);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not availale"));
		return -1;
	}
		
	ie = (struct qosmap_element *)buf;

	ie->eid = IE_QOS_MAP_SET;
	ie_len += 1;

	pos = (char *)ie->dscp_range;
	
	/* dscp exception */
	for (field = 0;field < conf->dscp_field;field++)
	{
		*pos = conf->dscp_exception[field] & 0xff;
		*(pos+1) = (conf->dscp_exception[field] >> 8) & 0xff;
		pos += 2;
		ie_len += 2;
	}
	
	/* dscp range */
	for (field = 0;field < 8;field++)
	{
		*pos = conf->dscp_range[field] & 0xff;
		*(pos+1) = (conf->dscp_range[field] >> 8) & 0xff;
		pos += 2;
		ie_len += 2;
	}
	
	ie->length = ie_len - 1;
	ie_len += 1;

	ret = wapp_set_ie(wapp, conf->iface, buf, ie_len);

	os_free(buf);

	return ret;
}

static int hotspot_set_interworking_ie(struct wifi_app *wapp, struct wapp_conf *conf)
{
	struct interworking_element *ie;
	char *buf, *pos;
	int ret;
	int ie_len = 0, varlen = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);	

	if (conf->is_venue_group && conf->is_venue_type)
		varlen += 2;

	if (conf->is_hessid)
		varlen += 6;

	buf = os_zalloc(sizeof(*ie) + varlen);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not available"));
		return -1;
	}

	ie = (struct interworking_element *)buf;

	ie->eid = IE_INTERWORKING;
	ie_len += 1;

	ie->access_network_options = (ie->access_network_options & ~0x0F) 
										| (conf->access_network_type & 0x0F);
	ie->access_network_options = (ie->access_network_options & ~0x10)
										| ((conf->internet << 4) & 0x10);
	ie_len += 1;

	pos = ie->variable;

	if (conf->is_venue_group && conf->is_venue_type) {
		*pos++ = conf->venue_group;
		*pos++ = conf->venue_type;
		ie_len += 2;
	}

	if (conf->is_hessid) {
		os_memcpy(pos, conf->hessid, 6);
		pos += 6;
		ie_len += 6;
	}

	ie->length = ie_len - 1;
	ie_len += 1;

	ret = wapp_set_ie(wapp, conf->iface, buf, ie_len);

	os_free(buf);
	
	return ret;
}

static int hotspot_set_advertisement_proto_ie(struct wifi_app *wapp, struct wapp_conf *conf)
{
	struct advertisement_proto_element *ie;
	char *buf, *pos;
	int ret;
	int i, ie_len = 0, varlen = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	varlen += 2 * conf->advertisement_proto_num;

	buf = os_zalloc(sizeof(*ie) + varlen);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not available"));
		return -1;
	}

	ie = (struct advertisement_proto_element *)buf;
	
	ie->eid = IE_ADVERTISEMENT_PROTO;
	ie_len += 1;

	pos  = ie->variable;

	for (i = 0; i < conf->advertisement_proto_num; i++) {
		*pos++  = 0x7F;
		*pos++ = conf->advertisement_proto[i];
		ie_len += 2; 
	}

	ie->length = ie_len - 1;
	ie_len += 1;	
	
	ret = wapp_set_ie(wapp, conf->iface, buf, ie_len);

	os_free(buf);
	
	return ret;
}

static int hotspot_set_roaming_consortium_ie(struct wifi_app *wapp, struct wapp_conf *conf)
{
	struct roaming_consortium_info_element *ie;
	char *buf, *pos;
	int ret;
	int ie_len = 0, varlen = 0;
	int i = 0;
	struct oi_duple *oiduple; 

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	dl_list_for_each(oiduple, &conf->oi_duple_list, struct oi_duple, list) {
		if (i < 3)
			varlen += oiduple->length;
		i++;
	}

	buf = os_zalloc(sizeof(*ie) + varlen);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not available"));
		return -1;
	}

	ie = (struct roaming_consortium_info_element *)buf;

	ie->eid = IE_ROAMING_CONSORTIUM;
	ie_len += 1;

	if ((i - 3) > 0)
		ie->num_anqp_oi = i - 3;
	else
		ie->num_anqp_oi = 0;
	
	ie_len += 1;

	i = 0;

	dl_list_for_each(oiduple, &conf->oi_duple_list, struct oi_duple, list) {
		if (i == 0) {
			ie->oi1_oi2_length = (ie->oi1_oi2_length & ~0x0F) | oiduple->length;
		} else if (i == 1) {
			ie->oi1_oi2_length = (ie->oi1_oi2_length & ~0xF0) |
										((oiduple->length << 4) & 0xF0);
		} else
			break;
		i++;
	}
			

	ie_len += 1;

	pos = ie->variable;

	i = 0;
	dl_list_for_each(oiduple, &conf->oi_duple_list, struct oi_duple, list) {
		if (i < 3) {
			os_memcpy(pos, oiduple->oi, oiduple->length);
			pos += oiduple->length;
			ie_len += oiduple->length;
			i++;
		} else
			break;
	}

	ie->length = ie_len - 1;
	ie_len += 1;
	
	ret = wapp_set_ie(wapp, conf->iface, buf, ie_len);

	os_free(buf);

	return ret;
}

int hotspot_set_time_zone_ie(struct wifi_app *wapp, struct wapp_conf *conf)
{
	struct time_zone_element *ie;
	char *buf, *pos;
	int ie_len = 0, varlen = 0; 
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	varlen = conf->time_zone_len;

	buf = os_zalloc(sizeof(*ie) + varlen);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "memory is not available\n");
		return -1;
	}

	ie = (struct time_zone_element *)buf;
	pos = (char *)ie;

	ie->eid = IE_TIME_ZONE;
	ie_len += 1;
	pos++;

	ie->length = conf->time_zone_len;
	ie_len += 1;
	pos++;

	os_memcpy(ie->variable, conf->time_zone, conf->time_zone_len);
	ie_len += conf->time_zone_len;
	pos += conf->time_zone_len; 

	ret = wapp_set_ie(wapp, conf->iface, buf, ie_len);

	os_free(buf);

	return ret;
}

static int hotspot_reset_ap_resource(struct wifi_app *wapp, const char *iface)
{
	int ret;

	ret = wapp->hs->drv_ops->drv_reset_resource(wapp->drv_data, iface);

	return ret;
}

int hotspot_set_ap_all_ies(struct wifi_app *wapp, const char *iface)
{
	int ret;
	struct wapp_conf *conf;
	u8 is_found = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found)
		return -1;

	ret = hotspot_set_hs_indication_ie(wapp, conf);
	
	ret += hotspot_set_p2p_ie(wapp, conf);

	ret += hotspot_set_interworking_ie(wapp, conf);

	ret += hotspot_set_advertisement_proto_ie(wapp, conf);

	if (conf->have_roaming_consortium_list) {
		ret += hotspot_set_roaming_consortium_ie(wapp, conf);
	}

    if (wapp->hs->version >= 2)
    {
		ret += hotspot_set_qosmap_ie(wapp, conf);	
    }

    if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "set IE FAILED!!!\n");
		ret = 0;
	}

	return ret;
}


int hotspot_set_ap_ifaces_all_ies(struct wifi_app *wapp)
{
	int ret = 0;
	struct wapp_conf *conf;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		ret = hotspot_set_ap_all_ies(wapp, conf->iface);
		if (ret == -1)
			return ret;		
	}

	return ret;
}

int hotspot_reset_all_ap_resource(struct wifi_app *wapp)
{
	int ret = 0;
	struct wapp_conf *conf;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		ret = hotspot_reset_ap_resource(wapp, conf->iface);
	}

	return ret;
}

int hotspot_send_qosmap_configure(struct wifi_app *wapp, const char *iface,
						 const char *peer_mac_addr,
						 const char *qosmap,
						 size_t qosmap_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = hotspot_drv_ops_pre_check(wapp, iface);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: hotspot drv ops pre check fail\n", __FUNCTION__);
		return -1;
	}

	ret = wapp->hs->drv_ops->drv_send_qosmap_configure(wapp->drv_data, iface, peer_mac_addr,
										qosmap, qosmap_len);

	return ret;
}

int hotspot_onoff(struct wifi_app *wapp, const char *iface, int enable, 
					int event_trigger, int event_type)
{
	int ret, is_found = 0;
	struct wapp_conf *conf;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found)
		return -1;

	if (!enable) 
		conf->hotspot_onoff = 0;	
	
	if (enable) {
		DBGPRINT(RT_DEBUG_OFF, "Enable hotspot2.0 feature(%s)\n", iface);
	} else
		DBGPRINT(RT_DEBUG_OFF, "Disable hotspot2.0 feature(%s)\n", iface);
	
	ret = wapp->hs->drv_ops->drv_hotspot_onoff(wapp->drv_data, iface, enable, event_trigger, event_type);

	return ret;
}

static u8 hotspot_validate_security_type(struct wifi_app *wapp, const char *iface)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	ret = wapp->hs->drv_ops->drv_validate_security_type(wapp->drv_data, iface);

	return ret;
}

int hotspot_onoff_all(struct wifi_app *wapp, int enable)
{
	int ret = 0, setting = enable;
	struct wapp_conf *conf;
	u8 valid_security_type = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {

		enable = setting;
		if (conf->hs2_openmode_test)
			valid_security_type = 1;
		else
			valid_security_type = hotspot_validate_security_type(wapp, conf->iface);

		if (enable && valid_security_type && conf->interworking) 
			enable = 1;
		else
			enable = 0;

		ret = hotspot_onoff(wapp, conf->iface, enable, EVENT_TRIGGER_ON, HS_ON_OFF_BASE);
		
		if (ret)
			return -1;

	}
	
	return ret;
}

int hotspot_collect_hs_anqp_rsp(struct wifi_app *wapp,
							struct wapp_conf *conf, char *buf)
{
	unsigned char *pos;
	struct hs_anqp_frame *hs_anqp_rsp = (struct hs_anqp_frame *)buf;
	const char wfa_oi[3] = {0x50, 0x6F, 0x9A};
	u16 tmplen = 0;	

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (conf->query_hs_capability_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query HS capability list\n");
		if (conf->have_hs_capability_list) {
			struct anqp_hs_capability *hs_capability_subtype;
			DBGPRINT(RT_DEBUG_TRACE, "Collect HS capability list\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type = WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = HS_CAPABILITY;
			tmplen += 2;
			pos = hs_anqp_rsp->variable;
			if (conf->have_hs_capability_list) {
				dl_list_for_each(hs_capability_subtype, &conf->hs_capability_list,
											struct anqp_hs_capability, list) {
					*pos = hs_capability_subtype->subtype;
					pos++;
					tmplen++;
				}
			}

			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;

		}
		conf->query_hs_capability_list = 0;
	}

	if (conf->query_operator_friendly_name) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query operator friendly name\n");
	
		if (conf->have_operator_friendly_name) {
			struct operator_name_duple *op_name_duple;
			DBGPRINT(RT_DEBUG_TRACE, "Collect operator friendly name\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type = WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = OPERATOR_FRIENDLY_NAME;
			tmplen += 2;
			pos = hs_anqp_rsp->variable;
			dl_list_for_each(op_name_duple, &conf->operator_friendly_duple_list,
										struct operator_name_duple, list) {
				*pos = op_name_duple->length;
				pos++;
				tmplen++;

				os_memcpy(pos, op_name_duple->language, 3);
				pos += 3;
				tmplen += 3;

				os_memcpy(pos, op_name_duple->operator_name, op_name_duple->length - 3);
				pos += op_name_duple->length - 3;
				tmplen += op_name_duple->length - 3;
			}

			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;
		}

		conf->query_operator_friendly_name = 0;
	}
	
	if (conf->query_wan_metrics) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query WAN Metrics\n");
		if (conf->have_wan_metrics) {
			u32 tmpspeed;
			u16 tmplmd;
			DBGPRINT(RT_DEBUG_TRACE, "Collect WAN Metrics\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type= WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = WAN_METRICS;
			tmplen += 2;
			
			pos = hs_anqp_rsp->variable;

			*pos = (*pos & ~0x03) | (conf->metrics.link_status & 0x03);
		
			if (conf->metrics.dl_speed == conf->metrics.ul_speed)
				*pos = (*pos & ~0x04) | 0x04;
			else
				*pos = (*pos & ~0x04);

			if (conf->metrics.at_capacity)
				*pos = (*pos & ~0x08) | 0x08;
			else
				*pos = *pos & ~0x08;

			pos++;
			tmplen++;

			tmpspeed = conf->metrics.dl_speed;
			tmpspeed = cpu2le32(tmpspeed);
			os_memcpy(pos, &tmpspeed, 4);
			pos += 4;
			tmplen += 4;

			tmpspeed = conf->metrics.ul_speed;
			tmpspeed = cpu2le32(tmpspeed);
			os_memcpy(pos, &tmpspeed, 4);
			pos += 4;
			tmplen += 4;

			*pos = conf->metrics.dl_load;
			pos++;
			tmplen++;

			*pos = conf->metrics.ul_load;
			pos++;
			tmplen++;

			tmplmd = conf->metrics.lmd;
			tmplmd = cpu2le16(tmplmd);
			os_memcpy(pos, &tmplmd, 2);
			pos += 2;
			tmplen += 2;
			
			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;
		}
		conf->query_wan_metrics = 0;
	}


	if (conf->query_connection_capability_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query connection capability\n");
		if (conf->have_connection_capability_list) {
			struct proto_port_tuple	*proto_port_unit;
			u16 tmpport;
			DBGPRINT(RT_DEBUG_TRACE, "Collect Connection Capability\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type= WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = CONNECTION_CAPABILITY;
			tmplen += 2;
			pos = hs_anqp_rsp->variable;
			dl_list_for_each(proto_port_unit, &conf->connection_capability_list,
									struct proto_port_tuple, list) {
				*pos = proto_port_unit->ip_protocol;
				pos++;
				tmplen++;

				tmpport = proto_port_unit->port;
				tmpport = cpu2le16(tmpport);
				os_memcpy(pos, &tmpport, 2);
				pos += 2;
				tmplen += 2;

				*pos = proto_port_unit->status;
				pos++;
				tmplen++;
			}

			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;
		}
		conf->query_connection_capability_list = 0;
	}

	if (conf->query_operating_class) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query operating class\n");
		if (conf->have_operating_class) {
			struct operating_class_unit *operating_class;
			DBGPRINT(RT_DEBUG_TRACE, "Collect operating class\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type= WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = OPERATING_CLASS;
			tmplen += 2;
			pos = hs_anqp_rsp->variable;
			dl_list_for_each(operating_class, &conf->operating_class_list,
							struct operating_class_unit, list) {
				*pos = operating_class->op_class;
				pos++;
				tmplen++;
			}
			
			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;
		}
		conf->query_operating_class = 0;
	}

	if (conf->query_osu_providers_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query osu providers list\n");
		if (conf->have_osu_providers_list) {
			struct osu_providers *providers_list;
			struct osu_friendly_name *friendly_name_list;
			struct osu_method *method_list;
			struct icon_available *icon;
			struct osu_nai *nai;
			struct osu_service_desc *service_desc;
			u16 tmp;
			
			DBGPRINT(RT_DEBUG_TRACE, "Collect osu providers list\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type= WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = OSU_PROVIDE_LIST;
			tmplen += 2;
			pos = hs_anqp_rsp->variable;

#if 0			
			if (!conf->legacy_osu_exist) {
				unsigned short nontxlen = cpu2le16(conf->nontransmitted_len);
				
				memcpy(pos, &nontxlen, 2);
				pos += 2;
				conf->nontransmitted_profile = os_zalloc(conf->nontransmitted_len);
				pos += conf->nontransmitted_len;
				*pos = 0;
				pos++;
				tmplen += 3+conf->nontransmitted_len;
			} 
			else 
#endif				
			{
				*pos = conf->legacy_osu_ssidlen;
				pos++;
				tmplen += 1;
				
				os_memcpy(pos, conf->legacy_osu_ssid, conf->legacy_osu_ssidlen);
				pos += conf->legacy_osu_ssidlen;
				tmplen += conf->legacy_osu_ssidlen;
				
				*pos = conf->osu_providers_list_nums;
				pos++;
				tmplen++;
			}
			
			dl_list_for_each(providers_list, &conf->osu_providers_list,
							struct osu_providers, list) {
				*pos = providers_list->osu_providers_list_field_len & 0xff;
				*(pos+1) = (providers_list->osu_providers_list_field_len >> 8)& 0xff;

				pos += 2;
				tmplen += 2;
				
				*pos = providers_list->osu_friendly_name_len;
//JERRY
				*(pos+1) = (providers_list->osu_friendly_name_len >> 8)& 0xff;

				tmplen += 2;
				pos += 2;
				
				dl_list_for_each(friendly_name_list, &providers_list->osu_friendly_name_list,
										struct osu_friendly_name, list) {
					*pos = friendly_name_list->len;
					tmplen += 1;
					pos++;
					
					os_memcpy(pos, friendly_name_list->language, 3);
					pos += 3;
					tmplen += 3;
					
					os_memcpy(pos, friendly_name_list->osu_friendly_name_value, friendly_name_list->len - 3);
					pos += friendly_name_list->len - 3;
					tmplen += friendly_name_list->len - 3;
				}
				
				*pos = providers_list->osu_server_uri_len;
				pos++;
				tmplen++;
				
				os_memcpy(pos, providers_list->osu_server_uri, providers_list->osu_server_uri_len);
				pos += providers_list->osu_server_uri_len;
				tmplen += providers_list->osu_server_uri_len;
				
				*pos = providers_list->osu_method_len;
				tmplen++;
				pos++;
					
				dl_list_for_each(method_list, &providers_list->osu_method_list,
										struct osu_method, list) {					
					*pos = method_list->osu_method_value;
					pos += 1;
					tmplen += 1;
				}
				
				tmp = providers_list->icon_len;
				tmp = cpu2le16(tmp);
				os_memcpy(pos, &tmp, 2);
				pos += 2;
				tmplen += 2;			
					
				dl_list_for_each(icon, &providers_list->icon_list,
										struct icon_available, list) {
					tmp = icon->weight;
					tmp = cpu2le16(tmp);
					os_memcpy(pos, &tmp, 2);
					pos += 2;
					tmplen += 2;								
					tmp = icon->height;
					tmp = cpu2le16(tmp);
					os_memcpy(pos, &tmp, 2);
					pos += 2;
					tmplen += 2;
					
					os_memcpy(pos, icon->language, 3);
					pos += 3;
					tmplen += 3;
					
					*pos = icon->type_len;
					pos += 1;
					tmplen += 1;
					
					os_memcpy(pos, icon->icon_buf, icon->type_len);
					pos += icon->type_len;
					tmplen += icon->type_len;
					
					*pos = icon->filename_len;
					pos += 1;
					tmplen += 1;
					
					os_memcpy(pos, &icon->icon_buf[icon->type_len], icon->filename_len);
					pos += icon->filename_len;
					tmplen += icon->filename_len;
				}
				
				*pos = providers_list->osu_nai_len;
				pos++;
				tmplen++;
				
				dl_list_for_each(nai, &providers_list->osu_nai_list,
										struct osu_nai, list) {
					os_memcpy(pos, nai->osu_nai_value, nai->len);
					pos += nai->len;
					tmplen += nai->len;
				}
				
				*pos = providers_list->osu_service_len;
//JERRY
				*(pos+1) = (providers_list->osu_service_len >> 8)& 0xff;

				tmplen += 2;
				pos += 2;
				
				dl_list_for_each(service_desc, &providers_list->osu_service_desc_list,
										struct osu_service_desc, list) {
					*pos = service_desc->len;
					tmplen += 1;
					pos++;
					
					os_memcpy(pos, service_desc->language, 3);
					pos += 3;
					tmplen += 3;
					
					os_memcpy(pos, service_desc->osu_service_desc_value, service_desc->len - 3);
					pos += service_desc->len - 3;
					tmplen += service_desc->len - 3;
				}
			}
			
			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;
		}
		conf->query_osu_providers_list = 0;
	}
	
	if (conf->query_anonymous_nai) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query anonymous nai\n");
		if (conf->have_anonymous_nai) {
			DBGPRINT(RT_DEBUG_TRACE, "Collect anonymous nai\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type= WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = ANONYMOUS_NAI;
			tmplen += 2;
			pos = hs_anqp_rsp->variable;
			memcpy(pos, conf->anonymous_nai, conf->anonymous_nai_len);
			pos += conf->anonymous_nai_len;
			tmplen += conf->anonymous_nai_len;			
			
			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;
		}
		conf->query_anonymous_nai = 0;
	}

	if (conf->query_icon_metadata) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query operator icon metadata\n");
		if (conf->have_icon_metadata) {
			struct operator_icon_metadata *oim_data;
			u16 tmp;
			DBGPRINT(RT_DEBUG_TRACE, "Collect operator icon metadata\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type= WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = ICON_METADATA;
			tmplen += 2;
			pos = hs_anqp_rsp->variable;
			dl_list_for_each(oim_data, &conf->icon_metadata_list,
							struct operator_icon_metadata, list) {
				tmp = oim_data->weight;
				tmp = cpu2le16(tmp);
				os_memcpy(pos, &tmp, 2);
				pos += 2;
				tmplen += 2;
				tmp = oim_data->height;
				tmp = cpu2le16(tmp);
				os_memcpy(pos, &tmp, 2);
				pos += 2;
				tmplen += 2;

				os_memcpy(pos, oim_data->language, 3);
				pos += 3;
				tmplen += 3;

				*pos = oim_data->type_len;
				pos += 1;
				tmplen += 1;

				os_memcpy(pos, oim_data->icon_buf, oim_data->type_len);
				pos += oim_data->type_len;
				tmplen += oim_data->type_len;

				*pos = oim_data->filename_len;
				pos += 1;
				tmplen += 1;

				os_memcpy(pos, &oim_data->icon_buf[oim_data->type_len], oim_data->filename_len);
				pos += oim_data->filename_len;
				tmplen += oim_data->filename_len;
			}

			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;
		}
		conf->query_icon_metadata = 0;
	}

	if (conf->query_osu_providers_nai_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query osu providers nai list");

		if (conf->have_osu_providers_nai_list) {
			struct osu_providers_nai_duple *osu_prov_nai_duple;
			DBGPRINT(RT_DEBUG_TRACE, "Collect osu providers nai list");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type = WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = OSU_PROVIDERS_NAI_LIST;
			tmplen += 2;

			pos = hs_anqp_rsp->variable;
			dl_list_for_each(osu_prov_nai_duple, &conf->osu_providers_nai_duple_list,
										struct osu_providers_nai_duple, list) {
				*pos = osu_prov_nai_duple->length;
				pos++;
				tmplen++;

				os_memcpy(pos, osu_prov_nai_duple->osu_providers_nai_list, osu_prov_nai_duple->length);
				pos += osu_prov_nai_duple->length;
				tmplen += osu_prov_nai_duple->length;
			}

			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;
		}
		conf->query_osu_providers_nai_list = 0;
	}
	return 0;
}

int hotspot_collect_nai_home_realm_anqp_rsp(struct wifi_app *wapp,
							struct wapp_conf *conf, char *buf)
{
	char *pos;
	struct anqp_frame *anqp_rsp = (struct anqp_frame *)buf;
	struct nai_home_realm_data_query *home_realm_data_query, *home_realm_data_query_tmp;
	u16 tmplen = 0;
	u16 nai_realm_count  = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	DBGPRINT(RT_DEBUG_TRACE, "STA query NAI home realm list\n");
	DBGPRINT(RT_DEBUG_TRACE, "Collect NAI home realm list\n");

	tmplen = 0;
	anqp_rsp->info_id = cpu2le16(NAI_REALM_LIST);
	pos = anqp_rsp->variable + 2;
	tmplen += 2; /* NAI Realm Count length */

	if (conf->have_nai_realm_list) {
		struct nai_realm_data *realm_data;
		u16 nai_realm_data_field_len_tmp;
		struct eap_method *eapmethod;
		struct auth_param *authparam;
		dl_list_for_each(home_realm_data_query, &conf->nai_home_realm_name_query_list,
								 struct nai_home_realm_data_query, list) {
			dl_list_for_each(realm_data, &conf->nai_realm_list,
									 struct nai_realm_data, list) {
				if (strncasecmp(home_realm_data_query->nai_home_realm, realm_data->nai_realm,
						  		home_realm_data_query->nai_home_realm_len) == 0) {
					DBGPRINT(RT_DEBUG_TRACE, "home realm = %s\n", home_realm_data_query->nai_home_realm);
					DBGPRINT(RT_DEBUG_TRACE, "home realm len = %d\n", home_realm_data_query->nai_home_realm_len);
					nai_realm_count++;
					nai_realm_data_field_len_tmp = cpu2le16(realm_data->nai_realm_data_field_len);
					os_memcpy(pos, &nai_realm_data_field_len_tmp, 2);
					tmplen += 2;
					pos += 2;
				
					*pos = realm_data->nai_realm_encoding;
					tmplen += 1;
					pos++;
				
					*pos = realm_data->nai_realm_len;
					tmplen += 1;
					pos++;
				
					os_memcpy(pos, realm_data->nai_realm, realm_data->nai_realm_len);
					tmplen += realm_data->nai_realm_len;
					pos += realm_data->nai_realm_len;

					*pos = realm_data->eap_method_count;
					tmplen += 1;
					pos++;

					dl_list_for_each(eapmethod, &realm_data->eap_method_list,
												struct eap_method, list) {
						*pos = eapmethod->len;
						tmplen += 1;
						pos++;

						*pos = eapmethod->eap_method;
						tmplen += 1;
						pos++;

						*pos = eapmethod->auth_param_count;
						tmplen += 1;
						pos++;

						dl_list_for_each(authparam, &eapmethod->auth_param_list,
												struct auth_param, list) {
							*pos = authparam->id;
							tmplen += 1;
							pos++;

							*pos = authparam->len;
							tmplen += 1;
							pos++;

							os_memcpy(pos, authparam->auth_param_value, authparam->len);
							tmplen += authparam->len;
							pos += authparam->len;
						}
	
					}
				}
			}
		}
	} else
		DBGPRINT(RT_DEBUG_TRACE, "AP does not have nai realm list info\n");
		

	DBGPRINT(RT_DEBUG_TRACE, "NAI Realm Count = %d\n", nai_realm_count);
	nai_realm_count = cpu2le16(nai_realm_count);
	os_memcpy(anqp_rsp->variable, &nai_realm_count, 2);
	anqp_rsp->length = cpu2le16(tmplen);
	
	/* Clear home_realm_name_query_list */
	dl_list_for_each_safe(home_realm_data_query, home_realm_data_query_tmp,
		&conf->nai_home_realm_name_query_list, struct nai_home_realm_data_query, list) {

		dl_list_del(&home_realm_data_query->list);
		os_free(home_realm_data_query);
	}

	dl_list_init(&conf->nai_home_realm_name_query_list);

	conf->query_nai_home_realm = 0;

	return 0;
}

int hotspot_collect_icon_binary_file(struct wifi_app *wapp,
							struct wapp_conf *conf, char *buf)
{
	u8 *pos;
	struct hs_anqp_frame *hs_anqp_rsp = (struct hs_anqp_frame *)buf;
	const char wfa_oi[3] = {0x50, 0x6F, 0x9A};
	u16 tmplen = 0;	
	struct icon_binary *icon_binary_data, *icon_binary_data_tmp;
	struct icon_available *icon;
	struct osu_providers *providers_list;
	
	DBGPRINT(RT_DEBUG_TRACE, "Collect HS icon binary file\n");
	
	tmplen = 0;
	hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
	os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
	tmplen += 3;
	hs_anqp_rsp->type = WFA_TIA_HS;
	tmplen++;
	hs_anqp_rsp->subtype = ICON_BINARY_FILE;
	tmplen += 2;
	pos = hs_anqp_rsp->variable;
	
	if (conf->calc_hs_icon_file_len == 14) {
		*pos = FILE_NOT_FOUND;
		pos++;
		tmplen++;
		os_memset(pos, 0, 3);
		pos += 3;
		tmplen += 3;
		DBGPRINT(RT_DEBUG_OFF, ("file not found\n"));
	}
	else if (conf->have_osu_providers_list) {
		dl_list_for_each(icon_binary_data, &conf->icon_file_list,
									  struct icon_binary, list) {
			dl_list_for_each(providers_list, &conf->osu_providers_list, struct osu_providers, list) {
	        	dl_list_for_each(icon, &providers_list->icon_list, struct icon_available, list) {
	        		#if 0
	        		{
	        			unsigned char *tmp1= icon->filename;
	        			unsigned char *tmp2= icon_binary_data->filename;
	        			int u;
	        			printf("cmp:%d\n", icon->filename_len);
	        			for(u=0;u<icon->filename_len;u++)
	        				printf("%02x:", icon->filename[u]);
	        			printf("\n");
	        			for(u=0;u<icon->filename_len;u++)
	        				printf("%02x:", icon_binary_data->filename[u]);
	        			printf("\n");		
	        		}
	        		#endif
	        		char *filename= &icon->icon_buf[icon->type_len];
					if (strncasecmp(filename, icon_binary_data->filename, icon->filename_len) == 0) {
						if (icon_binary_data->filesize != 0) {
							FILE *file;
							char tmpfile[256];
							int filelen = 0;//, total = 0;
						
							*pos = SUCCESS;
							pos++;
							tmplen++;
							*pos = icon->type_len;
							pos++;
							tmplen++;
							//os_memcpy(pos, icon->type, icon->type_len);
							os_memcpy(pos, icon->icon_buf, icon->type_len);
							pos += icon->type_len;
							tmplen += icon->type_len;
							
							os_memcpy(tmpfile, conf->iconfile_path, conf->iconfile_path_len);
							os_memcpy(&tmpfile[conf->iconfile_path_len], icon_binary_data->filename, icon->filename_len);
							tmpfile[conf->iconfile_path_len+icon->filename_len] = '\0';
							
							file = fopen(tmpfile, "r");
							if (!file) {
								DBGPRINT(RT_DEBUG_ERROR, "open icon binary file(%s) fail\n", icon_binary_data->filename);
								icon_binary_data->filesize = 0;	
								os_memcpy(pos, &icon_binary_data->filesize, 2);
								pos += 2;
								tmplen += 2;
							}
							else {
								char ret=0;
								icon_binary_data->filesize = cpu2le16(icon_binary_data->filesize);
								os_memcpy(pos, &icon_binary_data->filesize, 2);
								pos += 2;
								tmplen += 2;
								#if 0
								while(fread(&tmp,sizeof(tmp),1,file) != 0) {
									*pos = tmp;
									pos += 1;
									tmplen += 1;
								}    
								#else
								fseek( file, 0, SEEK_END);
								filelen = ftell(file);
								if (filelen < 0) {
									DBGPRINT(RT_DEBUG_ERROR, "(%s) ftell error\n", __FUNCTION__);
									fclose(file);
									return -1;
								}
								fseek( file, 0, SEEK_SET);
								ret = fread( pos, 1, filelen, file );
								if(ret)
									DBGPRINT(RT_DEBUG_ERROR, "(%s) fread error %d\n", __FUNCTION__,ret);
								
								pos += filelen;
								tmplen += filelen;
								#endif
								fclose(file);
							}
						}
						else {
							DBGPRINT(RT_DEBUG_TRACE, ("file not found\n"));
							*pos = FILE_NOT_FOUND;
							pos++;
							tmplen++;
							os_memset(pos, 0, 3);
							pos += 3;
							tmplen += 3;
						}
						goto check_file_end;
					}
				}
			}
		}
	}
	else {
		*pos = UNSPECIFIC_FILE_ERROR;
		pos++;
		tmplen++;
		os_memset(pos, 0, 3);
		pos += 3;
		tmplen += 3;
	}

check_file_end:		
	hs_anqp_rsp->length = cpu2le16(tmplen);
	hs_anqp_rsp = (struct hs_anqp_frame *)pos;
	
	/* Clear icon_file_list */
	dl_list_for_each_safe(icon_binary_data, icon_binary_data_tmp,
		&conf->icon_file_list, struct icon_binary, list) {

		dl_list_del(&icon_binary_data->list);
		os_free(icon_binary_data);
	}

	dl_list_init(&conf->icon_file_list);
	
	conf->query_icon_binary_file = 0;
	
	return 0;
}

size_t hotspot_calc_nai_home_realm_anqp_rsp_len(struct wifi_app *wapp,
													   struct wapp_conf *conf,
													   size_t nai_home_realm_anqp_req_len,
													   const char *curpos)
{
	size_t varlen = 0, curlen = 0;
	u8 nai_home_realm_count = 0, i;
	u8 nai_home_realm_encoding;
	u8 nai_home_realm_name_len = 0;
	struct nai_realm_data *realm_data;
	struct nai_home_realm_data_query *home_realm_data_query; 

	DBGPRINT(RT_DEBUG_TRACE, "\n");

	while (curlen < nai_home_realm_anqp_req_len) {
		nai_home_realm_count = *curpos;
		curpos++;
		curlen++;

		/* Info ID, Lenth, and NAI Realm Count */
		varlen += 6;

		for (i = 0; i < nai_home_realm_count; i++) {
			/* NAI Realm Encoding */
			nai_home_realm_encoding = *curpos;
			curpos++;
			curlen++;

			/* NAI Home Realm Name Length */
			nai_home_realm_name_len = *curpos;
			curpos++;
			curlen++;

			/* Add to nai_home_realm_name_query_list */
			home_realm_data_query = os_zalloc(sizeof(*home_realm_data_query) + 
										nai_home_realm_name_len);

			home_realm_data_query->nai_home_realm_encoding = nai_home_realm_encoding;
			home_realm_data_query->nai_home_realm_len = nai_home_realm_name_len;
			os_memcpy(home_realm_data_query->nai_home_realm, curpos, nai_home_realm_name_len);
			dl_list_add_tail(&conf->nai_home_realm_name_query_list, &home_realm_data_query->list);

			DBGPRINT(RT_DEBUG_TRACE, "nai_home_realm_encoding = %d\n",  home_realm_data_query->nai_home_realm_encoding);
			DBGPRINT(RT_DEBUG_TRACE, "nai_home_realm_len = %d\n",  home_realm_data_query->nai_home_realm_len);
			DBGPRINT(RT_DEBUG_TRACE, "nai_home_realm = %s\n",  home_realm_data_query->nai_home_realm);

			/* Filter if matching nai realm name, if match calc varlen */
			if (conf->have_nai_realm_list) {
				dl_list_for_each(realm_data, &conf->nai_realm_list,
								struct nai_realm_data, list) {

					if (strncasecmp(curpos, realm_data->nai_realm, nai_home_realm_name_len) == 0) {
						varlen += 2; /* NAI Realm Data Field Length */
						varlen += realm_data->nai_realm_data_field_len;

						break;
					}
				}
			}

			curpos += nai_home_realm_name_len;
			curlen += nai_home_realm_name_len;
		}
	}

	conf->query_nai_home_realm = 1;
	return varlen;
}

size_t hotspot_calc_icon_binary_file_len(struct wifi_app *wapp,
													   struct wapp_conf *conf,
													   size_t hs_anqp_req_len,
													   const char *icon_request)
{
	//size_t varlen = 0, curlen = 0;
	struct osu_providers *providers_list;
	struct icon_available *icon;
	
	DBGPRINT(RT_DEBUG_TRACE, "\n");
	
	conf->query_icon_binary_file = 1;
		
	if (conf->have_osu_providers_list) {
		dl_list_for_each(providers_list, &conf->osu_providers_list,
											  struct osu_providers, list) {
			dl_list_for_each(icon, &providers_list->icon_list,
									struct icon_available, list) {
				DBGPRINT(RT_DEBUG_TRACE, "icon->type_len=%d,file_name=%d, %zu\n", icon->type_len, icon->filename_len, hs_anqp_req_len);
				if ((strncasecmp(&icon->icon_buf[icon->type_len], icon_request, hs_anqp_req_len) == 0) && (icon->filename_len == hs_anqp_req_len)) {
					FILE *file;
					int filesize = 0;
					char tmpfile[256];
					struct icon_binary *icon_binary_data;
					
					os_memcpy(tmpfile, conf->iconfile_path, conf->iconfile_path_len);
					DBGPRINT(RT_DEBUG_TRACE, "path len=%d\n", conf->iconfile_path_len);
					os_memcpy(&tmpfile[conf->iconfile_path_len], &icon->icon_buf[icon->type_len], hs_anqp_req_len);
					tmpfile[conf->iconfile_path_len+hs_anqp_req_len] = '\0';
					DBGPRINT(RT_DEBUG_TRACE, "tmpfile=%s\n", tmpfile);
					file = fopen(tmpfile, "r");
					if (!file) {
						DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", &icon->icon_buf[icon->type_len]);
						filesize = 0;
						return 10 + 4;
					} else {
						if(fseek(file, 0, SEEK_END)) {
							filesize = 0;
						} else {
							filesize = ftell(file);
							rewind(file);
						}
						fclose(file);
					}
					DBGPRINT(RT_DEBUG_TRACE, "filesize=%d\n", filesize);
					icon_binary_data = os_zalloc(sizeof(*icon_binary_data) + 
										icon->filename_len);
					os_memcpy(icon_binary_data->filename, &icon->icon_buf[icon->type_len], icon->filename_len);
					icon_binary_data->filesize = filesize;					
					dl_list_add_tail(&conf->icon_file_list, &icon_binary_data->list);
					
					DBGPRINT(RT_DEBUG_TRACE, "icon type=%d, filesize=%d\n", icon->type_len, filesize);
					return 10 + 4 + icon->type_len + filesize;
				}
			}
		}
	}	
	
	return 10 + 4;
}


size_t hotspot_calc_hs_anqp_rsp_len(struct wifi_app *wapp,
									struct wapp_conf *conf,
									size_t hs_anqp_req_len,
									const char *curpos)
{

	size_t varlen = 0, curlen = 0;
	struct anqp_hs_capability *hs_capability_subtype;
	struct operator_name_duple *op_name_duple;
	struct proto_port_tuple *proto_port_unit;
	struct operating_class_unit *operating_class;
	struct osu_providers *providers_list;
	struct operator_icon_metadata *oim_data;
	struct osu_providers_nai_duple *osu_prov_nai_duple;

	DBGPRINT(RT_DEBUG_TRACE, "\n");
	
	while (curlen < hs_anqp_req_len) {
		switch(*curpos) {
			case HS_CAPABILITY:
				if (!conf->query_hs_capability_list) {
					conf->query_hs_capability_list = 1;
					if (conf->have_hs_capability_list) {
						varlen += 10;
						dl_list_for_each(hs_capability_subtype, &conf->hs_capability_list,
														struct anqp_hs_capability, list) {
							varlen += 1;
						}
					}
				}

				break;
			case OPERATOR_FRIENDLY_NAME:
				if (!conf->query_operator_friendly_name) {
					conf->query_operator_friendly_name = 1;
					if (conf->have_operator_friendly_name) {
						varlen += 10;
				
						dl_list_for_each(op_name_duple, &conf->operator_friendly_duple_list,
												  struct operator_name_duple, list) {

							varlen += 1;
							varlen += op_name_duple->length;
						}
					}
				}

				break;
			case WAN_METRICS:
				if (!conf->query_wan_metrics) {
					conf->query_wan_metrics = 1;
					if (conf->have_wan_metrics) {
						varlen += 10;
						varlen += 13;
					}
				}
				break;
			case CONNECTION_CAPABILITY:
				if (!conf->query_connection_capability_list) {
					conf->query_connection_capability_list = 1;
					if (conf->have_connection_capability_list) {
						varlen += 10;
						dl_list_for_each(proto_port_unit, &conf->connection_capability_list,
														struct proto_port_tuple, list)
						varlen += 4;
					}

				}
				break;
			case OPERATING_CLASS:
				if (!conf->query_operating_class) {
					conf->query_operating_class = 1;
					if (conf->have_operating_class) {
						varlen += 10;
						dl_list_for_each(operating_class, &conf->operating_class_list,
											struct operating_class_unit, list)
							varlen += 1;
					}
				}
				break;
			case OSU_PROVIDE_LIST:
				if (!conf->query_osu_providers_list && (wapp->hs->version >= 2)) {
					conf->query_osu_providers_list = 1;
					if (conf->have_osu_providers_list) {
						varlen += 10;
			
						//if (!conf->legacy_osu_exist)
						//	varlen += conf->nontransmitted_len + 2;						
						//else	
							varlen += conf->legacy_osu_ssidlen + 2;
							
						dl_list_for_each(providers_list, &conf->osu_providers_list,
										  struct osu_providers, list) {
							varlen += providers_list->osu_providers_list_field_len;
							varlen += 2;
						}
					}
				}
				break;	
			case ANONYMOUS_NAI:
				if (!conf->query_anonymous_nai && (wapp->hs->version >= 2)) {
					conf->query_anonymous_nai = 1;
					if (conf->have_anonymous_nai) {
						varlen += 10;
						varlen += conf->anonymous_nai_len;
					}
				}
				break;
			case ICON_METADATA:
				if (!conf->query_icon_metadata ) {
					conf->query_icon_metadata = 1;
					if (conf->have_icon_metadata) {
						varlen += 10;
						dl_list_for_each(oim_data, &conf->icon_metadata_list,
											struct operator_icon_metadata, list) {
							varlen += 2;
							varlen += 2;
							varlen += 3;
							varlen++;
							varlen += oim_data->type_len;
							varlen++;
							varlen += oim_data->filename_len;
						}
					}
				}
				break;
			case OSU_PROVIDERS_NAI_LIST:
				if (!conf->query_osu_providers_nai_list) {
					conf->query_osu_providers_nai_list = 1;
					if (conf->have_osu_providers_nai_list) {
						varlen += 10;
						dl_list_for_each(osu_prov_nai_duple, &conf->osu_providers_nai_duple_list,
													struct osu_providers_nai_duple, list) {
							varlen++;
							varlen += osu_prov_nai_duple->length;
						}
					}
				}
				break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, "Unknown HS2.0 subtype\n");
				break; 
		}	

		curpos ++;
		curlen ++;
	}

	return varlen;
}


static int hotspot_show_anqp_query_results(struct wifi_app *wapp,
							   struct wapp_conf *conf)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (conf->query_anqp_capability_list) {
		DBGPRINT(RT_DEBUG_OFF, "STA query ANQP capability list\n");
		if (conf->have_anqp_capability_list) {
			struct anqp_capability *anqp_capability_unit;
			DBGPRINT(RT_DEBUG_OFF, "Receive ANQP capability list from AP\n");
			dl_list_for_each(anqp_capability_unit, &conf->anqp_capability_list,
												struct anqp_capability, list) {

				DBGPRINT(RT_DEBUG_OFF, "Info ID = %d\n", anqp_capability_unit->info_id);
			}
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have ANQP capability list\n");
	}

	if (conf->query_venue_name) {
		DBGPRINT(RT_DEBUG_OFF, "STA query venue name information\n");
		if (conf->have_venue_name) {
			struct venue_name_duple *vname_duple;
			DBGPRINT(RT_DEBUG_OFF, "Receive venue name from AP\n");

			dl_list_for_each(vname_duple, &conf->venue_name_list,
								struct venue_name_duple, list) {
				DBGPRINT(RT_DEBUG_OFF, "language = %s\n", vname_duple->language);
				DBGPRINT(RT_DEBUG_OFF, "venue name = %s\n", vname_duple->venue_name);	
			}
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have venue name information\n");
	}

	if (conf->query_emergency_call_number) {
			DBGPRINT(RT_DEBUG_OFF, "STA query emergency call number\n");
		if (conf->have_emergency_call_number) {
			DBGPRINT(RT_DEBUG_OFF, "Receive emergency call number from AP\n");
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have emergency call number\n");
	}

	if (conf->query_roaming_consortium_list) {
		DBGPRINT(RT_DEBUG_OFF, "STA query roaming consortium list\n");	
		if (conf->have_roaming_consortium_list) {
			struct oi_duple *oiduple;
			int i;
			DBGPRINT(RT_DEBUG_OFF, "Receive roaming consortium list from AP\n");	
		
			dl_list_for_each(oiduple, &conf->oi_duple_list, struct oi_duple, list) {
				for (i = 0; i < oiduple->length; i++)
					DBGPRINT(RT_DEBUG_OFF, "roaming consortium_oi[%d] = 0x%02x\n", i, oiduple->oi[i]);
			}
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have roaming consortium list\n");
	}

	if (conf->query_nai_realm_list) {
		DBGPRINT(RT_DEBUG_OFF, "STA query NAI realm list\n");
		if (conf->have_nai_realm_list) {
			struct nai_realm_data *realm_data;
			struct eap_method *eapmethod;
			struct auth_param *authparam;
			DBGPRINT(RT_DEBUG_OFF, "Receive NAI realm list from AP\n");	

			dl_list_for_each(realm_data, &conf->nai_realm_list, struct nai_realm_data, list) {
				DBGPRINT(RT_DEBUG_OFF, "NAI realm = %s\n", realm_data->nai_realm);
				DBGPRINT(RT_DEBUG_OFF, "EAP method count = %d\n", realm_data->eap_method_count);

				dl_list_for_each(eapmethod, &realm_data->eap_method_list,
												struct eap_method, list) {
					DBGPRINT(RT_DEBUG_OFF, "EAPMethod = %d\n", eapmethod->eap_method);
					DBGPRINT(RT_DEBUG_OFF, "Auth Param Count = %d\n", eapmethod->auth_param_count);

					dl_list_for_each(authparam, &eapmethod->auth_param_list, 
												struct auth_param, list) {
						DBGPRINT(RT_DEBUG_OFF, "Auth ID = %d\n", authparam->id);
						if (authparam->len == 1) {
							DBGPRINT(RT_DEBUG_OFF, "Auth Value = %d\n", *(authparam->auth_param_value));
						} else {
							/* TODO */
						} 
					}
				}

			}

		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have NAI realm list\n");
	}
	
	if (conf->query_3gpp_network_info) {
		DBGPRINT(RT_DEBUG_OFF, "STA query 3gpp network information\n");

		if (conf->have_3gpp_network_info) {
			struct plmn *plmn_unit;
			int i, j = 0;
			DBGPRINT(RT_DEBUG_OFF, "Receive 3gpp network information from AP\n");	
			DBGPRINT(RT_DEBUG_OFF, "GUD = %d\n", conf->gud);
			DBGPRINT(RT_DEBUG_OFF, "UDHL = %d\n", conf->udhl);			

			dl_list_for_each(plmn_unit, &conf->plmn_list, struct plmn, list) {
				DBGPRINT(RT_DEBUG_OFF, "PLMN(%d):\n", j);
				DBGPRINT(RT_DEBUG_OFF, "MCC:");
				for (i = 2; i >= 0; i--)
					printf("%d", plmn_unit->mcc[i]);

				printf("\n");

				DBGPRINT(RT_DEBUG_OFF, "MNC:");

				for (i = 2; i >= 0; i--)
					printf("%d", plmn_unit->mnc[i]);

				printf("\n");
				j++;
			}
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have 3gpp network information\n");
	}	

	if (conf->query_domain_name_list) {
		DBGPRINT(RT_DEBUG_OFF, "STA query domain name list\n");
		if (conf->have_domain_name_list) {
			struct domain_name_field *dname_field;
			DBGPRINT(RT_DEBUG_OFF, "Receive domain name list from AP\n");	
	
			dl_list_for_each(dname_field, &conf->domain_name_list, struct domain_name_field, list) {
				DBGPRINT(RT_DEBUG_OFF, "domain name = %s\n", dname_field->domain_name);
			}
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have domain name list\n");
	}

	return ret;
}

static int hotspot_show_anqp_hs_query_results(struct wifi_app *wapp,
							   struct wapp_conf *conf)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	/* Following are HS2.0 elements */
	if (conf->query_hs_capability_list) {
		DBGPRINT(RT_DEBUG_OFF, "STA query HS capability list\n");
		if (conf->have_hs_capability_list) {
			struct anqp_hs_capability *hs_capability;
			DBGPRINT(RT_DEBUG_OFF, "Receive HS capability list from AP\n");	

			dl_list_for_each(hs_capability, &conf->hs_capability_list,
									struct anqp_hs_capability, list) {
				DBGPRINT(RT_DEBUG_OFF, "subtype = %d\n", hs_capability->subtype);
			}
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have HS capability list\n");
	}

	if (conf->query_operator_friendly_name) {
		DBGPRINT(RT_DEBUG_OFF, "STA query Operator friendly name\n");
		if (conf->have_operator_friendly_name) {
			struct operator_name_duple *op_name_duple;
			DBGPRINT(RT_DEBUG_OFF, "Receive Operator friendly name from AP\n");

			dl_list_for_each(op_name_duple, &conf->operator_friendly_duple_list,
								struct operator_name_duple, list) {
				DBGPRINT(RT_DEBUG_OFF, "length = %d\n", op_name_duple->length);
				DBGPRINT(RT_DEBUG_OFF, "language = %s\n", op_name_duple->language);
				DBGPRINT(RT_DEBUG_OFF, "operator_name = %s\n", op_name_duple->operator_name);	
			}
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have HS capability list\n");
	}

	return ret;
}

static int hotspot_clear_query_results(struct wifi_app *wapp,
								struct wapp_conf *conf)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (conf->have_anqp_capability_list) {
		struct anqp_capability *anqp_capability_unit, *anqp_capability_unit_tmp;
		
		dl_list_for_each_safe(anqp_capability_unit, anqp_capability_unit_tmp,
							  &conf->anqp_capability_list, struct anqp_capability, list) {
			dl_list_del(&anqp_capability_unit->list);
			os_free(anqp_capability_unit);
		}
		
		conf->have_anqp_capability_list = 0;
	}

	if (conf->have_roaming_consortium_list) {
		struct oi_duple *oiduple, *oiduple_tmp;
		
		dl_list_for_each_safe(oiduple, oiduple_tmp, &conf->oi_duple_list,
							  struct oi_duple, list) {
			dl_list_del(&oiduple->list);
			os_free(oiduple);
		}
		
		conf->have_roaming_consortium_list = 0;
	}

	if (conf->have_venue_name) {
		struct venue_name_duple *vname_duple, *vname_duple_tmp;

		dl_list_for_each_safe(vname_duple, vname_duple_tmp, &conf->venue_name_list,
							  struct venue_name_duple, list) {
			dl_list_del(&vname_duple->list);
			os_free(vname_duple);
		}

		conf->have_venue_name = 0;
	}

	if (conf->have_nai_realm_list) {
		struct nai_realm_data *realm_data, *realm_data_tmp;
		struct eap_method *eapmethod, *eapmethod_tmp;
		struct auth_param *authparam, *authparam_tmp;

		dl_list_for_each_safe(realm_data, realm_data_tmp,
								&conf->nai_realm_list, struct nai_realm_data, list) {
			dl_list_del(&realm_data->list);
			
			dl_list_for_each_safe(eapmethod, eapmethod_tmp,
									&realm_data->eap_method_list, struct eap_method, list) {
				dl_list_del(&eapmethod->list);
					dl_list_for_each_safe(authparam, authparam_tmp,
										&eapmethod->auth_param_list, struct auth_param, list) {
						dl_list_del(&authparam->list);
						os_free(authparam);
					}
				os_free(eapmethod);
			}
			os_free(realm_data);
		}
		conf->have_nai_realm_list = 0;
	}

	if (conf->have_3gpp_network_info) {
		struct plmn *plmn_unit, *plmn_unit_tmp;
		
		dl_list_for_each_safe(plmn_unit, plmn_unit_tmp,
								&conf->plmn_list, struct plmn, list) {
			dl_list_del(&plmn_unit->list);
			os_free(plmn_unit);
		}

		conf->have_3gpp_network_info = 0;
	}

	if (conf->have_domain_name_list) {
		struct domain_name_field *dname_field, *dname_field_tmp;
		
		dl_list_for_each_safe(dname_field, dname_field_tmp, &conf->domain_name_list,
							  struct domain_name_field, list) {
			dl_list_del(&dname_field->list);
			os_free(dname_field);
		}
		
		conf->have_domain_name_list = 0;
	}

	if (conf->have_hs_capability_list) {
		struct anqp_hs_capability *hs_capability, *hs_capability_tmp;

		dl_list_for_each_safe(hs_capability, hs_capability_tmp, &conf->hs_capability_list,
								struct anqp_hs_capability, list) {
			dl_list_del(&hs_capability->list);
			os_free(hs_capability);
		}

		conf->have_hs_capability_list = 0;
	}

	if (conf->have_operator_friendly_name) {
		struct operator_name_duple *op_name_duple, *op_name_duple_tmp;

		dl_list_for_each_safe(op_name_duple, op_name_duple_tmp, &conf->operator_friendly_duple_list,
								struct operator_name_duple, list) {
			dl_list_del(&op_name_duple->list);
			os_free(op_name_duple);
		}

		conf->have_operator_friendly_name = 0;
	}

	if (conf->have_connection_capability_list) {

	}

	if (conf->have_nai_home_realm_query) {
		struct nai_home_realm_data_query *home_realm_data_query, *home_realm_data_query_tmp;

		dl_list_for_each_safe(home_realm_data_query, home_realm_data_query_tmp,
			&conf->nai_home_realm_name_query_list, struct nai_home_realm_data_query, list) {

			dl_list_del(&home_realm_data_query->list);
			os_free(home_realm_data_query);
		}

		conf->have_nai_home_realm_query = 0;
	}

	return ret;
}

int hotspot_event_ap_reload(struct wifi_app *wapp,
							const char *iface)
{
	char confname[256];
	int ret = 0, is_found = 0;
	struct wapp_conf *conf;
	u8 valid_security_type = 0;	

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			sprintf(confname,"%s", conf->confname);
			break;
		}
	}

	if (!is_found)
		return -1;
	
	/* remove and deinit configuration from hs_conf_list */
	wapp_deinit_config(wapp, conf);
	dl_list_del(&conf->list);
	os_free(conf);
	is_found = 0;

	/* Reload configuration file to hotspot configuration */
	ret = wapp_init_ap_config(wapp, confname);

	if (ret) 
		return -1;

	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found)
		return -1;

	/* Reset all driver hs resource */
	ret = hotspot_reset_ap_resource(wapp, iface);

	if (ret)
		return -1;

	/* Set hotspot IE */
	ret = hotspot_set_ap_all_ies(wapp, iface);

	if (ret)
		return -1;

	if (conf->hs2_openmode_test)
		valid_security_type = 1;
	else
		valid_security_type = hotspot_validate_security_type(wapp, conf->iface);

	DBGPRINT(RT_DEBUG_TRACE, "%s: interworking = %d, valid_security_type = %d\n", __FUNCTION__,
					conf->interworking, valid_security_type);

	if (conf->interworking && valid_security_type)
		ret = hotspot_onoff(wapp, iface, 1, EVENT_TRIGGER_ON, HS_ON_OFF_BASE);
	else
		ret = hotspot_onoff(wapp, iface, 0, EVENT_TRIGGER_ON, HS_ON_OFF_BASE);
		
	return ret;
}

int hotspot_event_hs_onoff(struct wifi_app *wapp,
						   const char *iface,
						   int enable)
{
	int ret = 0, is_found = 0;
	struct wapp_conf *conf;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}
	
	if (!is_found)
		return -1;

	if (enable)
			conf->hotspot_onoff = 1;

	return ret;
}

int hotspot_event_get_location_IE(struct wifi_app *wapp,						   
						   char *buf)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_ERROR, "%s\n", __FUNCTION__);
	struct location_IE *location = (struct location_IE *)buf;

	/*dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}
	
	if (!is_found)
		return -1;
	*/
	if (location->len != 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, "GOT location IE type: %d len: %d\n", location->type, location->len);
		switch(location->type)
		{
			case AP_GEOSPATIAL_LOCATION:
				os_memset(wapp->hs->lci_IE, 0, LOCATION_IE_LEN);				
				os_memcpy(wapp->hs->lci_IE,location->location_buf,location->len);
				wapp->hs->lci_IE_len = location->len;
				hex_dump("civic_IE == ", (UCHAR *)wapp->hs->lci_IE,wapp->hs->lci_IE_len);
			break;
			case AP_CIVIC_LOCATION:
				os_memset(wapp->hs->civic_IE, 0, LOCATION_IE_LEN);
				os_memcpy(wapp->hs->civic_IE,location->location_buf,location->len);
				wapp->hs->civic_IE_len = location->len;
			break;
			case AP_LOCATION_PUBLIC_IDENTIFIER_URI:
				os_memset(wapp->hs->public_id_uri, 0, LOCATION_IE_LEN);
				os_memcpy(wapp->hs->public_id_uri,location->location_buf,location->len);
				wapp->hs->public_id_uri_len = location->len;
			break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, "unknown location IE type: %d len: %d\n", location->type, location->len);
		}
	}

	return ret;
}

size_t hotspot_calc_btm_req_len(struct wifi_app *wapp,
							 	struct wapp_conf *conf)
{
	struct bss_transition_candi_preference_unit *preference_unit;
	size_t btm_req_len = 0;
#if 1 /* mbo */
	int num_nr_list = wapp->daemon_nr_list.CurrListNum;
#endif

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	btm_req_len += 4;
	
	if (conf->have_bss_termination_duration)
		btm_req_len += 12;

	if (conf->have_session_info_url)
		btm_req_len += conf->session_info_url_len + 1;
	
	if (conf->have_bss_transition_candi_list) {
		dl_list_for_each(preference_unit, &conf->bss_transition_candi_list,
							struct bss_transition_candi_preference_unit, list)
			btm_req_len += 18;

	}
#if 1 /* mbo */
	else if (num_nr_list > 0) {
		btm_req_len += (18 * num_nr_list);
	}
#endif


	return btm_req_len;
}

int hotspot_collect_btm_req(struct wifi_app *wapp,
							struct wapp_conf *conf,
							const u8 *peer_addr,
							char *btm_req)
{
	struct btm_payload *frame;
	char *pos = btm_req;
	struct bss_transition_candi_preference_unit *preference_unit;
	struct neighbor_report_element *report_element;
	size_t report_element_len = 0;
	struct neighbor_report_subelement *report_subelement;
#if 1 /* mbo */
	int num_nr_list = wapp->daemon_nr_list.CurrListNum;
#endif

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	frame = (struct btm_payload *)btm_req;

	frame->u.btm_req.request_mode = (frame->u.btm_req.request_mode & ~0x01) | 
									(conf->preferred_candi_list_included);

	frame->u.btm_req.request_mode = (frame->u.btm_req.request_mode & ~0x02) |
									(conf->abridged << 1);

	frame->u.btm_req.request_mode = (frame->u.btm_req.request_mode & ~0x04) |
									(conf->disassociation_imminent << 2);

	frame->u.btm_req.request_mode = (frame->u.btm_req.request_mode & ~0x08) |
									(conf->bss_termination_included << 3);

	frame->u.btm_req.request_mode = (frame->u.btm_req.request_mode & ~0x10) |
									(conf->ess_disassociation_imminent << 4);

	pos += 1;

	frame->u.btm_req.disassociation_timer = cpu2le16(conf->disassociation_timer);
	pos += 2;

	frame->u.btm_req.validity_interval = conf->validity_interval;
	pos += 1;


	if (conf->have_bss_termination_duration) {
		report_subelement = (struct neighbor_report_subelement *)pos;
		report_subelement->subelement_id = BSS_TERMINATION_DURATION;
		report_subelement->length = 10;
		report_subelement->u.bss_termination_duration.bss_termination_tsf = 
											cpu2le64(conf->bss_termination_tsf);
		report_subelement->u.bss_termination_duration.duration = 
											cpu2le16(conf->bss_termination_duration);
		pos += 12;
	}

	if (conf->have_session_info_url) {
		/* session url length */
		*pos = conf->session_info_url_len;
		pos++;
		
		/* session url */
		os_memcpy(pos, conf->session_info_url, conf->session_info_url_len);
		pos += conf->session_info_url_len;
	}
	
	if (conf->have_bss_transition_candi_list) {
			report_element = (struct neighbor_report_element *)pos;
			report_element->eid = IE_NEIGHBOR_REPORT;
			pos += 2;
			
			os_memcpy(report_element->bssid, peer_addr, 6);
			pos += 6;
			report_element_len += 6;

			os_memset(&report_element->bss_info, 0, 4);
			pos += 4;
			report_element_len += 4;

			report_element->regulatory_class = 12;
			pos++;
			report_element_len++;

			report_element->channel_number = 11;
			pos++;
			report_element_len++;

			report_element->phy_type = 4;
			pos++;
			report_element_len++;

			dl_list_for_each(preference_unit, &conf->bss_transition_candi_list,
						struct bss_transition_candi_preference_unit, list) {
				report_subelement = (struct neighbor_report_subelement *)pos;
				report_subelement->subelement_id = BSS_TRANSITION_CANDIDATE_PREFERENCE;
				report_subelement->length = 1;
				report_subelement->u.bss_transition_candi_preference.preference = 
												preference_unit->preference;
				pos += 3;
				report_element_len += 3;
			}
			report_element->length = report_element_len;
	}
#if 1 /* mbo */
	else if (num_nr_list > 0) {
		u16 tmplen;
		u8 disassoc_imnt = FALSE;
		u8 is_steer_to_cell = FALSE;
		u8 bss_term = FALSE;
		struct wapp_sta *sta = NULL;
		
		sta = wdev_ap_client_list_lookup_for_all_bss(wapp, peer_addr);
		mbo_check_sta_preference_and_append_nr_list( wapp,
		                                             sta,
                                                     pos,
                                                     &tmplen,
                                                     MBO_FRAME_BTM,
                                                     disassoc_imnt,
                                                     bss_term,
                                                     is_steer_to_cell);
		frame->u.btm_req.request_mode |= 1;
	}
#endif


	return 0;
}

static void hotspot_proxy_arp_ipv4(char *buf,
								   const char *source_mac_addr,
								   const char *source_ip_addr,
								   const char *target_mac_addr,
								   const char *target_ip_addr,
								   unsigned char	IsDAD)
{
	char *pos;
	u16 protocol_type = cpu2be16(0x0806);
	u16 hw_address_type = cpu2be16(0x0001);
	u16 protocol_address_type;
	u16 arp_operation = cpu2be16(0x0002);;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	pos = buf;
	
	/* DA */
	os_memcpy(pos, source_mac_addr, 6);
	pos += 6;

	/* SA */
	os_memcpy(pos, target_mac_addr, 6);
	pos += 6;

	/* Protocol type */
	os_memcpy(pos, &protocol_type, 2);
	pos += 2;

	/* HW address yype */
	os_memcpy(pos, &hw_address_type, 2);
	pos += 2;

	/* Protocol address type */
	protocol_address_type = cpu2be16(0x0800);
	os_memcpy(pos, &protocol_address_type, 2);
	pos += 2;

	/* HW address size */
	*pos = 0x06;
	pos++;

	/* Protocol address size */
	*pos = 0x04;
	pos++;

	/* arp operation */
	os_memcpy(pos, &arp_operation, 2);
	pos += 2;
	
	/* Sender MAC address */
	os_memcpy(pos, target_mac_addr, 6);
	pos += 6;

	/* Sender IP address */
	os_memcpy(pos, target_ip_addr, 4);
	pos += 4;

	/* Target MAC address */
	os_memcpy(pos, source_mac_addr, 6);
	pos += 6;

	/* Target IP address */
	//if (IsDAD == 1)
	//	os_memcpy(pos, target_ip_addr, 4);
	//else	
	os_memcpy(pos, source_ip_addr, 4);
	pos += 4;
}

static u16 icmpv6_csum(const char *saddr,
					   const char *daddr,
					   u16 len,
					   u8 proto,
					   const char *icmp_msg)
{
	struct _ipv6_addr *sa_ipv6_addr = (struct _ipv6_addr *)saddr;
	struct _ipv6_addr *da_ipv6_addr = (struct _ipv6_addr *)daddr;
	u32 carry, ulen, uproto;
	u32 i;
	u32 csum = 0x00;
	u16 chksum;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (len % 4)
		return 0;
	
	for( i = 0; i < 4; i++)
	{
		csum += sa_ipv6_addr->ipv6_addr32[i];
		carry = (csum < sa_ipv6_addr->ipv6_addr32[i]);
		csum += carry;
	}

	for( i = 0; i < 4; i++)
	{
		csum += da_ipv6_addr->ipv6_addr32[i];
		carry = (csum < da_ipv6_addr->ipv6_addr32[i]);
		csum += carry;
	}

	ulen = htonl((u32)len);
	csum += ulen;
	carry = (csum < ulen);
	csum += carry;

	uproto = htonl((u32)proto);
	csum += uproto;
	carry = (csum < uproto);
	csum += carry;
	
	for (i = 0; i < len; i += 4)
	{
		csum += *((u32 *)(&icmp_msg[i]));
		carry = (csum < (*((u32 *)(&icmp_msg[i]))));
		csum += carry;
	}

	while (csum>>16)
		csum = (csum & 0xffff) + (csum >> 16);

	chksum = ~csum;
	
	return chksum;
}

static void hotspot_proxy_arp_ipv6(char *buf,
								   const char *source_mac_addr,
								   const char *source_ip_addr,
								   const char *target_mac_addr,
								   const char *target_ip_addr,
								   unsigned char IsDAD)
{

	char *pos, *pcsum, *icmpv6hdr;
	char DadDestAddr[16]={0xff,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01};
	u16 protocol_type = cpu2be16(0x86dd);
	u16 payload_len = cpu2be16(0x0020);
	u16	checksum = 0;
	u32 icmpmsglen = 0x20;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	pos = buf;
	
	/* DA */
//JERRY
#if 0
	if (IsDAD == 1) {
		*pos = 0x33;pos++;
		*pos = 0x33;pos++;
		*pos = 0x00;pos++;
		*pos = 0x00;pos++;
		*pos = 0x00;pos++;
		*pos = 0x01;pos++;
	}
	else {
#endif
		os_memcpy(pos, source_mac_addr, 6);
		pos += 6;
//	}

	/* SA */
	os_memcpy(pos, target_mac_addr, 6);
	pos += 6;
	
	/* Protocol type */
	os_memcpy(pos, &protocol_type, 2);
	pos += 2;

	/* Version, Traffic Class, Flow label */
	*pos = 0x60;
	pos++;

	*pos = 0x00;
	pos++;

	*pos = 0x00;
	pos++;

	*pos = 0x00;
	pos++;

	/* payload length */
	os_memcpy(pos, &payload_len, 2);
	pos += 2;

	/* Next header */
	*pos = 0x3a;
	pos++;

	/* Hop limit */
	*pos = 0xff;
	pos++;

	/* source ip address */
	os_memcpy(pos, target_ip_addr, 16);
	pos += 16;

	/* destination ip address */
	if (IsDAD == 1) {
		*pos = 0xff;pos++;
		*pos = 0x02;pos++;
		pos = pos + 13;
		*pos = 0x01;pos++;
		
		DBGPRINT(RT_DEBUG_OFF, ("ipv6 dad.....\n"));
	}
	else {
		os_memcpy(pos, source_ip_addr, 16);
		pos += 16;
	}

	/* ICMP field */
	icmpv6hdr = pos;
	/* Type */
	*pos = 0x88;
	pos++;

	/* Code */
	*pos = 0x00;
	pos++;

	/* Checksum */
	pcsum = pos;
	os_memcpy(pos, &checksum, 2);
	pos += 2;

	/* flags */
	*pos = 0x60;
	pos++;

	*pos = 0x00;
	pos++;

	*pos = 0x00;
	pos++;

	*pos = 0x00;
	pos++;

	/* targer address */
	os_memcpy(pos, target_ip_addr, 16);
	pos += 16;

	/* Possible options */
	/* target linker-layerr address type */
	*pos = 0x02;
	pos++; 

	/* length */
	*pos = 0x01;
	pos++;

	/* target link-layer address */
	os_memcpy(pos, target_mac_addr, 6);
	pos += 6;

	/* re-calculate checksum */
	if (IsDAD == 1)
		checksum = icmpv6_csum(target_ip_addr, DadDestAddr, icmpmsglen, 0x3a, icmpv6hdr);
	else
		checksum = icmpv6_csum(target_ip_addr, source_ip_addr, icmpmsglen, 0x3a, icmpv6hdr);
	os_memcpy(pcsum, &checksum, 2);
}

static int hotspot_event_proxy_arp(struct wifi_app *wapp,
								   const int ifindex,
								   u8 ip_type,
								   u8 from_ds,
								   const char *source_mac_addr,
								   const char *source_ip_addr,
								   const char *target_mac_addr,
								   const char *target_ip_addr,
								   unsigned char IsDAD)
{
	int sock;
	struct sockaddr_ll sll;
	char *buf;
	u8 bufsize;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	/* send arp response on behalf of target */
	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (sock < 0) {
		DBGPRINT(RT_DEBUG_OFF, "%s: socket fail\n", __FUNCTION__);
		return -1;
	}

	memset(&sll, 0, sizeof(sll));
	
	if (from_ds) {
		sll.sll_ifindex = if_nametoindex("eth0");
		DBGPRINT(RT_DEBUG_TRACE, "Send Proxy ARP Packet to eth0\n");
	
	} else {
		sll.sll_ifindex = ifindex;
		DBGPRINT(RT_DEBUG_TRACE, "Send Proxy ARP Packet to ra0(%d)\n", ifindex);
	}

	if (ip_type == IPV4)
		bufsize = 60;
	else
		bufsize = 86;

	buf = os_zalloc(bufsize);

	if (ip_type == IPV4)
		hotspot_proxy_arp_ipv4(buf, source_mac_addr, source_ip_addr, 
										target_mac_addr, target_ip_addr, IsDAD);
	else
		hotspot_proxy_arp_ipv6(buf, source_mac_addr, source_ip_addr,
										target_mac_addr, target_ip_addr, IsDAD);

	if (sendto(sock, buf, bufsize, 0, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Send ARP response failed\n");
		close(sock);		
		os_free(buf);
		return -1;
	}

	DBGPRINT(RT_DEBUG_TRACE, "%s:Send Proxy ARP Pakcet\n", __FUNCTION__);

	close(sock);

	os_free(buf);

	return 0;
}


/* ============ STA PART ======================== */
int hotspot_set_sta_ifaces_all_ies(struct wifi_app *wapp)
{
	int ret = 0;
	struct wapp_conf *conf;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	 
	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		ret = hotspot_set_interworking_ie(wapp, conf);
	
		if (ret)
			return -1;
	}

	return ret;
}

static int hotspot_anqp_query_test(struct wifi_app *wapp, struct wapp_conf *conf)
{
	struct anqp_frame *anqp_req;
	char *buf, *pos;
	size_t anqp_req_len = 0, varlen = 0;
	u16 tmp;
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (conf->query_anqp_capability_list)
		varlen += 2;
	
	if (conf->query_venue_name)
		varlen += 2;

	if (conf->query_emergency_call_number)
		varlen += 2;

	if (conf->query_network_auth_type)
		varlen += 2;

	if (conf->query_roaming_consortium_list)
		varlen += 2;

	if (conf->query_ip_address_type)
		varlen += 2;

	if (conf->query_nai_realm_list)
		varlen += 2;

	if (conf->query_3gpp_network_info)
		varlen += 2;

	if (conf->query_ap_geospatial_location)
		varlen += 2;

	if (conf->query_ap_civic_location)
		varlen += 2;

	if (conf->query_ap_location_public_uri)
		varlen += 2;

	if (conf->query_domain_name_list)
		varlen += 2;

	if (conf->query_emergency_alert_uri)
		varlen += 2;

	if (conf->query_emergency_nai)
		varlen += 2;

	buf = os_zalloc(sizeof(*anqp_req) + varlen);

	anqp_req = (struct anqp_frame *)buf;

	anqp_req->info_id = cpu2le16(ANQP_QUERY_LIST);
	anqp_req_len += 2;

	anqp_req->length = cpu2le16(varlen);
	anqp_req_len += 2;

	pos = anqp_req->variable;

	if (conf->query_anqp_capability_list) {
		tmp = cpu2le16(ANQP_CAPABILITY);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}
	
	if (conf->query_venue_name) {
		tmp = cpu2le16(VENUE_NAME_INFO);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_emergency_call_number) {
		tmp = cpu2le16(EMERGENCY_CALL_NUMBER_INFO);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_network_auth_type) {
		tmp = cpu2le16(NETWORK_AUTH_TYPE_INFO);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_roaming_consortium_list) {
		tmp = cpu2le16(ROAMING_CONSORTIUM_LIST);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_ip_address_type) {
		tmp = cpu2le16(IP_ADDRESS_TYPE_AVAILABILITY_INFO);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_nai_realm_list) {
		tmp = cpu2le16(NAI_REALM_LIST);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_3gpp_network_info) {
		tmp = cpu2le16(ThirdGPP_CELLULAR_NETWORK_INFO);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_ap_geospatial_location) {
		tmp = cpu2le16(AP_GEOSPATIAL_LOCATION);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_ap_civic_location) {
		tmp = cpu2le16(AP_CIVIC_LOCATION);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_ap_location_public_uri) {
		tmp = cpu2le16(AP_LOCATION_PUBLIC_IDENTIFIER_URI);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_domain_name_list) {
		tmp = cpu2le16(DOMAIN_NAME_LIST);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_emergency_alert_uri) {
		tmp = cpu2le16(EMERGENCY_ALERT_IDENTIFIER_URI);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_emergency_nai) {
		tmp = cpu2le16(EMERGENCY_NAI);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}
	
	anqp_req_len += varlen;
	
	ret = wapp_send_anqp_req(wapp, conf->iface, conf->hs_peer_mac, buf, anqp_req_len);

	os_free(buf);

	return ret;
}

static int hotspot_anqp_hs_query_test(struct wifi_app *wapp, struct wapp_conf *conf)
{

	struct hs_anqp_frame *hs_anqp_req;
	char *buf;
	u8 	 *pos;
	size_t hs_anqp_req_len = 0, varlen = 0;
	const char wfa_oi[3] = {0x50, 0x6F, 0x9A};
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (conf->query_hs_capability_list)
		varlen++;

	if (conf->query_operator_friendly_name)
		varlen++;

	if (conf->query_wan_metrics)
		varlen++;

	if (conf->query_connection_capability_list)
		varlen++;

	if (conf->query_nai_home_realm)
		varlen++;


	buf = os_zalloc(sizeof(*hs_anqp_req) + varlen);

	hs_anqp_req = (struct hs_anqp_frame *)buf;

	hs_anqp_req->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
	hs_anqp_req_len += 2;

	hs_anqp_req->length = 5 + varlen;
	hs_anqp_req_len += 2;

	os_memcpy(hs_anqp_req->oi, wfa_oi, 3);
	hs_anqp_req_len += 3;

	hs_anqp_req->type = WFA_TIA_HS;
	hs_anqp_req_len++;

	hs_anqp_req->subtype = HS_QUERY_LIST;
	hs_anqp_req_len++;

	pos = hs_anqp_req->variable;

	if (conf->query_hs_capability_list) {
		*pos = HS_CAPABILITY;
		pos++;
	}

	if (conf->query_operator_friendly_name) {
		*pos = OPERATOR_FRIENDLY_NAME;
		pos++;
	}

	if (conf->query_wan_metrics) {
		*pos = WAN_METRICS;
		pos++;
	}

	if (conf->query_connection_capability_list) {
		*pos = CONNECTION_CAPABILITY;
		pos++;
	}

	if (conf->query_nai_home_realm) {
		*pos = NAI_HOME_REALM_QUERY;
		pos++;
	}

	hs_anqp_req_len += varlen;
	
	ret = wapp_send_anqp_req(wapp, conf->iface, conf->hs_peer_mac, buf, hs_anqp_req_len);
	
	os_free(buf);

	return ret;
}

int hotspot_sta_test(struct wifi_app *wapp, const char *iface)
{
	int ret = 0;

	struct wapp_conf *conf;
	
	DBGPRINT(RT_DEBUG_TRACE, "\n");

	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0)
			break;
	}

	if (conf->anqp_req_type == GAS_ANQP_QUERY)
		ret = hotspot_anqp_query_test(wapp, conf);
	else if(conf->anqp_req_type == GAS_ANQP_HS_QUERY)
		ret = hotspot_anqp_hs_query_test(wapp, conf);

	return ret;
}
static int hotspot_receive_anqp_rsp(struct wifi_app *wapp,
							 struct wapp_conf *conf,
							 const char *pos,
							 size_t buflen,
							 u16 info_id)
{
	u16 tmp_info_id;
	int i, j, k, ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	switch (info_id) {
	case ANQP_CAPABILITY:
		while (buflen > 0) {
			struct anqp_capability *anqp_capability_unit;
			anqp_capability_unit = os_zalloc(sizeof(*anqp_capability_unit));
			memcpy(&tmp_info_id, pos, 2);
			anqp_capability_unit->info_id = le2cpu16(tmp_info_id);
			dl_list_add_tail(&conf->anqp_capability_list, &anqp_capability_unit->list);
			pos += 2;
			buflen -= 2;
		}

		if (!dl_list_empty(&conf->anqp_capability_list)) {
			conf->have_anqp_capability_list = 1;
		}

		break;
	case VENUE_NAME_INFO:
		if (buflen > 0) {
			conf->venue_group = *pos;
			pos++;
			buflen--;

			conf->venue_type = *pos;
			pos++;
			buflen--;
		}

		while (buflen > 0) {
			struct venue_name_duple *vname_duple, *vname_duple_new;
			vname_duple = os_zalloc(sizeof(*vname_duple));

			vname_duple->length = *pos;
			pos++;
			buflen--;

			os_memcpy(vname_duple->language, pos, 3);
			pos += 3;
			buflen -= 3;
			
			vname_duple_new = os_realloc(vname_duple, sizeof(*vname_duple) 
										+ vname_duple->length - 3);

			os_memcpy(vname_duple_new->venue_name, pos, vname_duple_new->length - 3);
			pos += (vname_duple_new->length - 3);
			buflen -= (vname_duple_new->length - 3);

			dl_list_add_tail(&conf->venue_name_list, &vname_duple_new->list);
		}
		
		if (!dl_list_empty(&conf->venue_name_list)) {
			conf->have_venue_name = 1;
		}

		break;
	case ROAMING_CONSORTIUM_LIST:
		while (buflen > 0 ) {
			struct oi_duple *oiduple;
			u8 oi_len = *pos;
			pos++;
			buflen--;
			oiduple = os_zalloc(sizeof(*oiduple) + oi_len);
			oiduple->length = oi_len;
			os_memcpy(oiduple->oi, pos, oi_len);
			dl_list_add_tail(&conf->oi_duple_list, &oiduple->list);
			pos += oi_len;
			buflen-= oi_len;
		}

		if (!dl_list_empty(&conf->oi_duple_list)) {
			conf->have_roaming_consortium_list = 1;
		}

		break;
	case NAI_REALM_LIST: 
		while (buflen > 0) {
			struct nai_realm_data *realm_data, *realm_data_new;
			u16 nai_realm_count;
			struct eap_method *eapmethod;
			struct auth_param *authparam, *authparam_new;
			u16 i;
			u16 tmp16;
					
			os_memcpy(&tmp16, pos, 2);
			nai_realm_count = le2cpu16(tmp16);
			pos += 2;
			buflen -= 2;

			DBGPRINT(RT_DEBUG_TRACE, "NAI realm count = %d\n", nai_realm_count);

			for (i = 0; i < nai_realm_count; i++) {
				realm_data = os_zalloc(sizeof(*realm_data));
				dl_list_init(&realm_data->eap_method_list);

				os_memcpy(&tmp16, pos, 2);
					
				realm_data->nai_realm_data_field_len = le2cpu16(tmp16);
				pos += 2;
				buflen -= 2;
					
				realm_data->nai_realm_encoding = *pos;
				pos++;
				buflen--;
					
				realm_data->nai_realm_len = *pos;
				pos++;
				buflen--;
					
				realm_data_new = os_realloc(realm_data, 
									sizeof(*realm_data) + realm_data->nai_realm_len);

				dl_list_init(&realm_data_new->eap_method_list);
				os_memcpy(realm_data_new->nai_realm, pos, realm_data_new->nai_realm_len);
				pos += realm_data_new->nai_realm_len;
				buflen -= realm_data_new->nai_realm_len;

				DBGPRINT(RT_DEBUG_TRACE, "%d\n", realm_data_new->nai_realm_len);

				realm_data_new->eap_method_count = *pos;
				pos++;
				buflen--;

				dl_list_add_tail(&conf->nai_realm_list, &realm_data_new->list);

				DBGPRINT(RT_DEBUG_TRACE, "eap method count = %d\n", realm_data_new->eap_method_count);

				for (j = 0; j < realm_data_new->eap_method_count; j++) {
					eapmethod = os_zalloc(sizeof(*eapmethod));
					dl_list_init(&eapmethod->auth_param_list);
					eapmethod->len = *pos;
					pos++;
					buflen--;

					eapmethod->eap_method = *pos;
					DBGPRINT(RT_DEBUG_TRACE, "EAP method = %d\n", eapmethod->eap_method);
					pos++;
					buflen--;

					eapmethod->auth_param_count = *pos;
					pos++;
					buflen--;

					dl_list_add_tail(&realm_data_new->eap_method_list, &eapmethod->list);

					DBGPRINT(RT_DEBUG_TRACE, "auth param count = %d\n", eapmethod->auth_param_count);
					for (k = 0; k < eapmethod->auth_param_count; k++) {
						authparam = os_zalloc(sizeof(*authparam));
						authparam->id = *pos;
						DBGPRINT(RT_DEBUG_TRACE, "id = %d\n", authparam->id);
						pos++;
						buflen--;

						authparam->len = *pos;
						DBGPRINT(RT_DEBUG_TRACE, "len = %d\n", authparam->len);
						pos++;
						buflen--;

						authparam_new = os_realloc(authparam, 
							sizeof(*authparam_new) + authparam->len);
								
						os_memcpy(authparam_new->auth_param_value, pos, authparam_new->len);
						pos += authparam_new->len;
						buflen -= authparam_new->len;
						dl_list_add_tail(&eapmethod->auth_param_list, &authparam_new->list);
					}
				}
			}
		}

		if (!dl_list_empty(&conf->nai_realm_list)) {
			conf->have_nai_realm_list = 1;
		}
		break;
	case ThirdGPP_CELLULAR_NETWORK_INFO:
		while (buflen > 0) {
			struct plmn *plmn_unit;
			conf->gud = *pos;
			pos++;
			buflen--;
			conf->udhl = *pos;
			pos++;
			buflen--;

			if (*pos == IEI_PLMN) {
				//u8 plmn_value_len;
				u8 plmn_num;
				pos++;
				buflen--;
				
				/* Length of PLMN list value contents */
				//plmn_value_len = *pos;
				pos++;
				buflen--;

				/* Number of PLMNs */
				plmn_num = *pos;
				pos++;
				buflen--;

				for (i = 0; i < plmn_num; i++) {
					plmn_unit = os_zalloc(sizeof(*plmn_unit));
					plmn_unit->mcc[0] = *pos & 0x0f;
					plmn_unit->mcc[1] = (*pos & 0xf0) >> 4; 
					pos++;
					buflen--;

					plmn_unit->mcc[2] = *pos & 0x0f;
					plmn_unit->mnc[2] = (*pos & 0xf0) >> 4;
					pos++;
					buflen--;

					plmn_unit->mnc[0] = *pos & 0x0f;
					plmn_unit->mnc[1] = (*pos & 0xf0) >> 4;
					pos++;
					buflen--;
					dl_list_add_tail(&conf->plmn_list, &plmn_unit->list);
				}
				
			}
			
		}

		if (!dl_list_empty(&conf->plmn_list)) {
			conf->have_3gpp_network_info = 1;
		}
		break;
	case DOMAIN_NAME_LIST: 				
		while (buflen > 0 ) {
			struct domain_name_field *dname_field;
			u8 dname_len = *pos;
			pos++;
			buflen--;
			dname_field = os_zalloc(sizeof(*dname_field) + dname_len);
			dname_field->length = dname_len;
			os_memcpy(dname_field->domain_name, pos, dname_len);
			dl_list_add_tail(&conf->domain_name_list, &dname_field->list);
			pos += dname_len;
			buflen-= dname_len;
		}

		if (!dl_list_empty(&conf->domain_name_list)) {
			conf->have_domain_name_list = 1;
		}

		break;
	}

	return ret;
}

static int hotspot_receive_anqp_hs_rsp(struct wifi_app *wapp,
								struct wapp_conf *conf,
							 	const char *pos,
							 	size_t buflen,
								u8 subtype)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	switch (subtype) {
	case HS_CAPABILITY:
		while (buflen > 0) {
			struct anqp_hs_capability *anqp_hs_capability_unit;
			anqp_hs_capability_unit = os_zalloc(sizeof(*anqp_hs_capability_unit));
			anqp_hs_capability_unit->subtype = *pos;
			pos++;
			buflen--;
			dl_list_add_tail(&conf->hs_capability_list, &anqp_hs_capability_unit->list);
		}

		if (!dl_list_empty(&conf->hs_capability_list)) {
			conf->have_hs_capability_list = 1;
		}

		break;
	case OPERATOR_FRIENDLY_NAME:
		while (buflen > 0) {
			struct operator_name_duple *op_name_duple, *op_name_duple_new;
			op_name_duple = os_zalloc(sizeof(*op_name_duple));
			op_name_duple->length = *pos;
			pos++;
			buflen--;
			os_memcpy(op_name_duple->language, pos, 3);
			pos += 3;
			buflen -= 3;
			
			op_name_duple_new = os_realloc(op_name_duple, sizeof(*op_name_duple) 
										+ op_name_duple->length - 3);
			os_memcpy(op_name_duple_new->operator_name, pos, op_name_duple_new->length - 3);
			pos += (op_name_duple_new->length - 3);
			buflen -= (op_name_duple_new->length - 3);
			dl_list_add_tail(&conf->operator_friendly_duple_list, &op_name_duple_new->list);
		}

		if (!dl_list_empty(&conf->operator_friendly_duple_list)) {
			conf->have_operator_friendly_name = 1;
		}
				
		break;

	case WAN_METRICS:

		break;
	case CONNECTION_CAPABILITY:

		break;

	default:

		break;
	}

	return ret;
}

int hotspot_event_anqp_rsp(struct wifi_app *wapp,
						   const char *iface,
						   const u8 *peer_mac_addr,
						   int status,
						   const char *anqp_rsp,
						   size_t anqp_rsp_len)
{
	const char *pos = anqp_rsp;
	size_t buflen;
	u16 info_id;
	struct wapp_conf *conf;
	const char wfa_oi[3] = {0x50, 0x6F, 0x9A};
	u8 subtype;

	DBGPRINT(RT_DEBUG_TRACE,"\n");

	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0)
			break;
	}

	if (status == 0) {
		while (anqp_rsp_len > 0) {
			os_memcpy(&info_id, pos, 2);
			info_id = le2cpu16(info_id);
			pos += 2;
			anqp_rsp_len -= 2;
			switch (info_id) {
			case ANQP_CAPABILITY:
			case VENUE_NAME_INFO:
			case EMERGENCY_CALL_NUMBER_INFO:
			case NETWORK_AUTH_TYPE_INFO:
			case ROAMING_CONSORTIUM_LIST:
			case IP_ADDRESS_TYPE_AVAILABILITY_INFO:
			case NAI_REALM_LIST:
			case ThirdGPP_CELLULAR_NETWORK_INFO:
			case AP_GEOSPATIAL_LOCATION:
			case AP_CIVIC_LOCATION:
			case AP_LOCATION_PUBLIC_IDENTIFIER_URI:
			case DOMAIN_NAME_LIST:
			case EMERGENCY_ALERT_IDENTIFIER_URI:
			case EMERGENCY_NAI:
				os_memcpy(&buflen, pos, 2);
				buflen = le2cpu16(buflen);
				pos += 2;
				anqp_rsp_len -= 2;				

				hotspot_receive_anqp_rsp(wapp, conf, pos, buflen, info_id);
				pos += buflen;
				anqp_rsp_len -= buflen;
	
				break;

			case ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST:
				os_memcpy(&buflen, pos, 2);
				buflen = le2cpu16(buflen);
				pos += 2;
				anqp_rsp_len -= 2;
	
				if (os_memcmp(pos, wfa_oi, 3) == 0) {
					pos += 3;
					anqp_rsp_len -= 3;
					buflen -= 3 ;

					if (*pos == WFA_TIA_HS) {
						pos++;
						anqp_rsp_len--;
						buflen--;

						subtype = *pos;
						pos++;
						anqp_rsp_len--;
						buflen--;
						hotspot_receive_anqp_hs_rsp(wapp, conf, pos, buflen, subtype);
						pos += buflen;
						anqp_rsp_len -= buflen;
					} else
						DBGPRINT(RT_DEBUG_TRACE, "Unknown type field\n");

				} else
					DBGPRINT(RT_DEBUG_TRACE, "Unknown OI field\n");

				break;
			default:
				DBGPRINT(RT_DEBUG_TRACE, "Unknown info ID(%d) from AP\n", info_id);	
				goto clear_query_results;
				break;	
			}
		}
	
		/* Show query results */
		if (conf->anqp_req_type == GAS_ANQP_QUERY)
			hotspot_show_anqp_query_results(wapp, conf);
		else if (conf->anqp_req_type == GAS_ANQP_HS_QUERY)
			hotspot_show_anqp_hs_query_results(wapp, conf);
		else
			DBGPRINT(RT_DEBUG_ERROR, "Unknow anqp request type\n");

clear_query_results:
 		/* Clear query results */
		hotspot_clear_query_results(wapp, conf);

	} else if (status == TIMEOUT) {
		DBGPRINT(RT_DEBUG_OFF, "Timeout\n");
	}
	
	/* Send again to test */
	hotspot_sta_test(wapp, iface);

	return 0;
}

/* 
 * hotspot event callback 
 *
 * general feature
 * event_test: test if daemon can receive driver event
 * event_hs_on_off:
 *
 * station mode
 * event_apqp_rsp:
 *
 * ap mode
 * event_anqp_req : 
 */
struct hotspot_event_ops hs_event_ops = {	
	.event_hs_onoff = hotspot_event_hs_onoff,	
	.event_proxy_arp = hotspot_event_proxy_arp,
	.event_ap_reload = hotspot_event_ap_reload,
	.event_get_location_IE = hotspot_event_get_location_IE,	
};

int anqp_init(struct wifi_app *wapp, 
				 struct hotspot_event_ops *event_ops,				
				 int version)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);	

	/* use ralink wireless extension */
	wapp->hs->drv_ops = &hotspot_drv_wext_ops;	

	wapp->hs->event_ops = event_ops;

	wapp->hs->version = version;

	dl_list_init(&wapp->conf_list);

	dl_list_init(&wapp->hs->hs_peer_list);	
	
	return 0;
}
