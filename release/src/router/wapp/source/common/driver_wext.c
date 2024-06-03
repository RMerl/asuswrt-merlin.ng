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

#include "hotspot.h"
#include "driver_wext.h"
#include "priv_netlink.h"
#include "wireless_copy.h"
#include "netlink.h"
#include <sys/ioctl.h>



static void event_anqp_rsp(struct driver_wext_data *drv_data, char *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;	
	struct anqp_rsp_data *rsp_data = (struct anqp_rsp_data *)buf;
	char ifname[IFNAMSIZ];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if_indextoname(rsp_data->ifindex, ifname);

	wapp->event_ops->event_anqp_rsp(wapp,
							 	  ifname,
								  rsp_data->peer_mac_addr,					  
							 	  rsp_data->status,
							 	  rsp_data->anqp_rsp,	
							 	  rsp_data->anqp_rsp_len);
}

static void event_anqp_req(struct driver_wext_data *drv_data, char *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;	
	struct anqp_req_data *req_data = (struct anqp_req_data *)buf;
	char ifname[IFNAMSIZ];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if_indextoname(req_data->ifindex, ifname);
	
	wapp->event_ops->event_anqp_req(wapp,
							 	  ifname,
								  req_data->peer_mac_addr,					  
							 	  req_data->anqp_req,	
							 	  req_data->anqp_req_len);
}

static void event_hs_onoff(struct driver_wext_data *drv_data, char *buf)
{

	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;	
	struct hs_onoff *onoff = (struct hs_onoff *)buf;
	char ifname[IFNAMSIZ];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if_indextoname(onoff->ifindex, ifname);

	wapp->hs->event_ops->event_hs_onoff(wapp,
							 	  ifname,
							 	  onoff->hs_onoff);

}

static void event_btm_query(struct driver_wext_data *drv_data, char *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;
	struct btm_query_data *query_data = (struct btm_query_data *)buf;
	char ifname[IFNAMSIZ];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if_indextoname(query_data->ifindex, ifname);

	wapp->event_ops->event_btm_query(wapp,
								   ifname,
								   query_data->peer_mac_addr,
								   query_data->btm_query,
								   query_data->btm_query_len);
}

static void event_btm_req(struct driver_wext_data *drv_data, char *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;
	struct btm_req_data *req_data = (struct btm_req_data *)buf;
	char ifname[IFNAMSIZ];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if_indextoname(req_data->ifindex, ifname);

	wapp->event_ops->event_btm_req(wapp,
			ifname,
			req_data->peer_mac_addr,
			req_data->btm_req,
			req_data->btm_req_len);
}

static void event_btm_rsp(struct driver_wext_data *drv_data, char *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;
	struct btm_rsp_data *rsp_data = (struct btm_rsp_data *)buf;
	char ifname[IFNAMSIZ];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if_indextoname(rsp_data->ifindex, ifname);

	wapp->event_ops->event_btm_rsp(wapp,
								 ifname,
								 rsp_data->peer_mac_addr,
								 rsp_data->btm_rsp,
								 rsp_data->btm_rsp_len);
}

static void event_proxy_arp(struct driver_wext_data *drv_data, char *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;
	struct proxy_arp_entry *arp_entry = (struct proxy_arp_entry *)buf;
	char ifname[IFNAMSIZ];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if_indextoname(arp_entry->ifindex, ifname);

	if (arp_entry->ip_type == IPV4) {
		wapp->hs->event_ops->event_proxy_arp(wapp,
									   arp_entry->ifindex,
									   arp_entry->ip_type,
									   arp_entry->from_ds,
									   arp_entry->source_mac_addr,
									   arp_entry->ip_addr,
									   arp_entry->target_mac_addr,
									   arp_entry->ip_addr + 4,
									   arp_entry->IsDAD);
	} else {
		wapp->hs->event_ops->event_proxy_arp(wapp,
									   arp_entry->ifindex,
									   arp_entry->ip_type,
									   arp_entry->from_ds,
									   arp_entry->source_mac_addr,
									   arp_entry->ip_addr,
									   arp_entry->target_mac_addr,
									   arp_entry->ip_addr + 16,
									   arp_entry->IsDAD);
	}
}

static void event_hs_ap_reload(struct driver_wext_data *drv_data, char *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;
	struct hs_onoff *onoff= (struct hs_onoff *)buf;
	char ifname[IFNAMSIZ];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if_indextoname(onoff->ifindex, ifname);

	wapp->hs->event_ops->event_ap_reload(wapp, ifname);
}

static void event_neighbor_report_pool_update(struct driver_wext_data *drv_data, char *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;
	struct neighbor_list_data *nr_list_data = (struct neighbor_list_data *)buf;
	char ifname[IFNAMSIZ];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if_indextoname(nr_list_data->ifindex, ifname);
	
	wapp->event_ops->event_get_mbo_neighbor_report(wapp,
												 ifname,
												 nr_list_data->neighbor_list_req,
												 nr_list_data->neighbor_list_len);
}

static void event_hs_driver_inform_location_ie(struct driver_wext_data *drv_data, char *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;
	//struct location_IE *loc_buf = (struct location_IE *)buf;
	//char ifname[IFNAMSIZ];

	//if_indextoname(onoff->ifindex, ifname);
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp->hs->event_ops->event_get_location_IE(wapp, buf);
}

static void event_mbo_msg_handle(struct driver_wext_data *drv_data, char *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;
	struct mbo_msg *msg = (struct mbo_msg *)buf;
	struct mbo_cfg *mbo = wapp->mbo;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	switch(msg->type)
	{
		case MBO_MSG_STA_PREF_UPDATE:
		case MBO_MSG_CDC_UPDATE:
		case MBO_MSG_BSSID_UPDATE:
			wapp->mbo->event_ops->sta_update(wapp, buf, msg->type);
		break;

		case MBO_MSG_STA_STEERING:
		{
			char ifname[IFNAMSIZ];
		
			if_indextoname(msg->ifindex, ifname);
			wapp_send_btm_req_by_case(
					wapp,
					ifname,
					msg->body.MboEvtStaInfo.mac_addr, // TODO: take mac_addr from the right msg
					BTM_STA_STEERING);
		}
		break;

		case MBO_MSG_DISASSOC_STA:
		{
			char ifname[IFNAMSIZ];

			if_indextoname(msg->ifindex, ifname);
			wapp_send_btm_req_by_case(
					wapp,
									    ifname,
										msg->body.MboEvtStaInfo.mac_addr, // TODO: take mac_addr from the right msg
					BTM_DISASSOC_STA);
		}
		break;

		case MBO_MSG_AP_TERMINATION:
		{ /* write a function for it if grows larger */
			struct wapp_conf *conf;
			char ifname[IFNAMSIZ] = {0};
			struct wapp_dev *wdev = NULL;
			u8 is_found=0;

			mbo->assoc_retry_delay = 0;
			if_indextoname(msg->ifindex, ifname);
			dl_list_for_each(conf, &wapp->conf_list ,struct wapp_conf,list) {
				if (os_strcmp(conf->iface, ifname) == 0) {
					is_found = 1;
					break;
				}
			}

			wdev = wapp_dev_list_lookup_by_ifname(wapp, ifname);
			if (wdev && !is_found)
				conf = wapp->wapp_default_config;
			if (!conf) {
				DBGPRINT(RT_DEBUG_ERROR, "[%s](%d): ERROR! conf is null\n", __func__, __LINE__);
				break;
			}
			if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
				u8 i;				
				struct wapp_sta *sta;
				struct ap_dev *ap = (struct ap_dev	*)wdev->p_dev;

			conf->bss_termination_tsf = msg->body.MboEvtBssTermTsf.TsfHighPart;
			conf->bss_termination_tsf = conf->bss_termination_tsf<<32;
			conf->bss_termination_tsf += msg->body.MboEvtBssTermTsf.TsfLowPart;

				for (i = 0; i < ap->num_of_clients; i++)
				{
					sta = ap->client_table[i];
					if (sta && sta->sta_status == WAPP_STA_CONNECTED) {
						wapp_send_btm_req_by_case(
								wapp,
									    ifname,
										sta->mac_addr,
								BTM_AP_TERMINATE);
					}
				}
			}
		}
			break;

		default:
			DBGPRINT(RT_DEBUG_TRACE, "unknown MBO_MSG_TYPE %d\n", msg->type);
			break;
	}
}

static void event_oce_msg_handle(struct driver_wext_data *drv_data, char *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;
	struct oce_msg *msg = (struct oce_msg *)buf;
	/*struct oce_cfg *oce = wapp->oce;*/

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	switch(msg->type)
	{
		case OCE_MSG_INFO_UPDATE:
			wapp->oce->event_ops->sta_update(wapp, buf, msg->type);
			break;

		default:
			DBGPRINT(RT_DEBUG_TRACE, "unknown OCE_MSG_TYPE %d\n", msg->type);
			break;
	}
}

static void event_wapp_event_handle(struct driver_wext_data *drv_data, char *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;
	struct wapp_event *event = (struct wapp_event *)buf;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp->event_ops->event_handle(wapp, event);
}
static void event_handle_offchannel_info(struct driver_wext_data *drv_data, char *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wapp->event_ops->event_offch_info(wapp, (u8*)buf);
}
#ifdef MAP_R2
static void event_wnm_notify_handle(struct driver_wext_data *drv_data, char *buf)
{
	struct wifi_app *wapp = (struct wifi_app *)drv_data->priv;	
	struct wnn_notify_req_data *req_data = (struct wnn_notify_req_data *)buf;
	char ifname[IFNAMSIZ];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if_indextoname(req_data->ifindex, ifname);

	wapp->event_ops->event_wnm_notify(wapp,
				 	  ifname,
					  req_data->peer_mac_addr,					  
				 	  req_data->wnm_req,	
				 	  req_data->wnm_req_len);
}
#endif

static void driver_wext_event_wireless(struct driver_wext_data *drv,
                 void *ctx, char *data, int len)
{               
    struct iw_event iwe_buf, *iwe = &iwe_buf;
    char *pos, *end, *custom, *buf /*,*assoc_info_buf, *info_pos */;
    /* info_pos = NULL; */
	/* assoc_info_buf = NULL; */

	pos = data;
    end = data + len;   
    
    while (pos + IW_EV_LCP_LEN <= end) {
        /* 
 		 * Event data may be unaligned, so make a local, aligned copy
         * before processing. 
         */
        os_memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);
		
		DBGPRINT(RT_DEBUG_INFO, "cmd = 0x%x len = %d\n", iwe->cmd, iwe->len);
		if (iwe->len <= IW_EV_LCP_LEN)
            return;

        custom = pos + IW_EV_POINT_LEN;

        //if (drv->we_version_compiled > 18 && iwe->cmd == IWEVCUSTOM) {
            /* WE-19 removed the pointer from struct iw_point */
            char *dpos = (char *) &iwe_buf.u.data.length;
            int dlen = dpos - (char *) &iwe_buf;
            os_memcpy(dpos, pos + IW_EV_LCP_LEN,
                  sizeof(struct iw_event) - dlen);
        //} else {
            //os_memcpy(&iwe_buf, pos, sizeof(struct iw_event));
            //custom += IW_EV_POINT_OFF;
		//}
		
		switch (iwe->cmd) {
        case IWEVCUSTOM:
			if (custom + iwe->u.data.length > end)
               	return;
           	buf = os_malloc(iwe->u.data.length + 1);
            if (buf == NULL)
                return;
            os_memcpy(buf, custom, iwe->u.data.length);
            buf[iwe->u.data.length] = '\0';

            switch (iwe->u.data.flags) {			
			case OID_802_11_HS_ANQP_REQ:
				event_anqp_req(drv, buf);
				break;
			case OID_802_11_HS_ANQP_RSP:
				event_anqp_rsp(drv, buf);
				break;
			case OID_802_11_HS_ONOFF:
				event_hs_onoff(drv, buf);
				break;
			case OID_802_11_WNM_BTM_QUERY:
				event_btm_query(drv, buf);
				break;
			case OID_802_11_WNM_BTM_RSP:
				event_btm_rsp(drv, buf);
				break;
			case OID_802_11_WNM_BTM_REQ:
				event_btm_req(drv, buf);
				break;
			case OID_802_11_WNM_PROXY_ARP:
				event_proxy_arp(drv, buf);
				break;
			case OID_802_11_HS_AP_RELOAD:
				event_hs_ap_reload(drv, buf);
				break;
			case OID_802_11_HS_LOCATION_DRV_INFORM_IE:
				event_hs_driver_inform_location_ie(drv, buf);
				break;
			case OID_802_11_MBO_MSG:
				event_mbo_msg_handle(drv, buf);
				break;
			case OID_NEIGHBOR_REPORT:
				event_neighbor_report_pool_update(drv,buf);
				break;
			case OID_OFFCHANNEL_INFO:
				event_handle_offchannel_info(drv, buf);
				break;
			case OID_802_11_OCE_MSG:
				event_oce_msg_handle(drv, buf);
				break;
			case OID_WAPP_EVENT:
				event_wapp_event_handle(drv, buf);
				break;
#ifdef MAP_R2
			case OID_802_11_WNM_NOTIFY_REQ:
				event_wnm_notify_handle(drv, buf);
				break;
#endif
			default:
				DBGPRINT(RT_DEBUG_TRACE, "%s: unkwnon event type(0x%x)\n", __FUNCTION__, iwe->u.data.flags);
				break;
			}

           	os_free(buf);
            break;
        }

        pos += iwe->len;
    }
}
static void driver_wext_event_rtm_newlink(void *ctx, struct ifinfomsg *ifi,
                    					  u8 *buf, size_t len)
{       
    struct driver_wext_data *drv = ctx;
    int attrlen, rta_len;
    struct rtattr *attr;

	attrlen = len;

   	DBGPRINT(RT_DEBUG_INFO, "attrlen=%d", attrlen);
	attr = (struct rtattr *) buf;
    rta_len = RTA_ALIGN(sizeof(struct rtattr));
    while (RTA_OK(attr, attrlen)) {
        DBGPRINT(RT_DEBUG_INFO, "rta_type=%02x\n", attr->rta_type);
        if (attr->rta_type == IFLA_WIRELESS) {
            driver_wext_event_wireless(
                drv, ctx,
                ((char *) attr) + rta_len,
                attr->rta_len - rta_len);
        }
        attr = RTA_NEXT(attr, attrlen);
    }
}

#if 0//defined but not used
static int driver_wext_get_we_version_compiled(struct driver_wext_data *drv_wext_data,
												const char *ifname)
{
 	struct iwreq iwr;
    int we_version_compiled = 0;
    
    os_memset(&iwr, 0, sizeof(iwr));
    os_strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
    iwr.u.data.pointer = (caddr_t) &we_version_compiled;
    iwr.u.data.flags = RT_OID_WE_VERSION_COMPILED;

    if (ioctl(drv_wext_data->ioctl_sock, RT_PRIV_IOCTL, &iwr) < 0) {
        DBGPRINT(RT_DEBUG_ERROR, "%s: failed", __FUNCTION__);
        return -1;
    }

	return we_version_compiled; 
}
#endif

static void *driver_wext_init(struct wifi_app *wapp, 
						      const int opmode,
							  const int drv_mode)
{
	struct driver_wext_data *drv_wext_data;
	struct netlink_config *cfg;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	drv_wext_data = calloc(1, sizeof(*drv_wext_data));
	
	if (!drv_wext_data) {
		DBGPRINT(RT_DEBUG_ERROR, "No avaliable memory for driver_wext_data\n");
		goto err1;

	}

	DBGPRINT(RT_DEBUG_OFF, "Initialize ralink wext interface\n");

	drv_wext_data->ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	
	if (drv_wext_data->ioctl_sock < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "socket(PF_INET,SOCK_DGRAM)");
		goto err2;
	}

	cfg = os_zalloc(sizeof(*cfg));

    if (!cfg) {
		DBGPRINT(RT_DEBUG_ERROR, "No avaliable memory for netlink cfg\n");
        goto err3;
    }

	cfg->ctx = drv_wext_data;
	cfg->newlink_cb = driver_wext_event_rtm_newlink;

	drv_wext_data->netlink = netlink_init(cfg);

	if (!drv_wext_data->netlink) {
		DBGPRINT(RT_DEBUG_ERROR, "wext netlink init fail\n");
		goto err3;
	}

	drv_wext_data->opmode = opmode;

	drv_wext_data->drv_mode = drv_mode;

	drv_wext_data->priv = (void *)wapp;

	return (void *)drv_wext_data;

err3:
	close(drv_wext_data->ioctl_sock);
err2:
	os_free(drv_wext_data);
err1:
	return NULL;
}

static int driver_wext_exit(struct wifi_app *wapp)
{
	struct driver_wext_data *drv_wext_data = wapp->drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	netlink_deinit(drv_wext_data->netlink);
    close(drv_wext_data->ioctl_sock);
	
	os_free(drv_wext_data);

	return 0;
}

static int driver_wext_set_oid(struct driver_wext_data *drv_data, const char *ifname,
              				   unsigned short oid, char *data, size_t len)    
{
    char *buf;                             
    struct iwreq iwr;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
    buf = os_zalloc(len);                  

    os_memset(&iwr, 0, sizeof(iwr));
    snprintf(iwr.ifr_name, IFNAMSIZ, "%s", ifname);
    iwr.u.data.flags = oid;
    iwr.u.data.flags |= OID_GET_SET_TOGGLE;

    if (data)
        os_memcpy(buf, data, len);

	if (buf) {
    	iwr.u.data.pointer = (caddr_t)buf;    
    	iwr.u.data.length = len;
	} else {
    	iwr.u.data.pointer = NULL;    
    	iwr.u.data.length = 0;
	}

    if (ioctl(drv_data->ioctl_sock, RT_PRIV_IOCTL, &iwr) < 0) {
        DBGPRINT(RT_DEBUG_ERROR, "%s: oid=0x%x len (%zu) failed\n",
               __FUNCTION__, oid, len);
        os_free(buf);
        return -1;
    }

    os_free(buf);
    return 0;
}

static int driver_wext_get_oid(struct driver_wext_data *drv_data, const char *ifname,
								   unsigned short oid, char *data, size_t *len)
{
	struct iwreq iwr;
	unsigned char *buf;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	os_memset(&iwr, 0, sizeof(iwr));
	snprintf(iwr.ifr_name, IFNAMSIZ, "%s", ifname);

	if (*len > 4096) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: len = %zu\n", __FUNCTION__, *len);
		return -1;
	}

	buf = os_zalloc(4096);


	iwr.u.data.pointer = (void *)buf;
	iwr.u.data.flags = oid;
	iwr.u.data.length = *len;

	if (ioctl(drv_data->ioctl_sock, RT_PRIV_IOCTL, &iwr) < 0) {
        DBGPRINT(RT_DEBUG_ERROR, "%s: oid=0x%x len (%zu) failed\n",
               __FUNCTION__, oid, *len);
		*len = 0;
		os_free(buf);
		return -1;
	}
	
	if (iwr.u.data.length < *len) {
		os_memcpy(data, buf, iwr.u.data.length);
		*len = iwr.u.data.length;
	} else
		os_memcpy(data, buf, *len);

	os_free(buf);	

	return 0;
}

static int driver_wext_get_misc_cap(void *drv_data, const char *ifname, char *buf, 
				       		  size_t *buf_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_GET_MISC_CAP, buf, buf_len);

	return ret;

}

static int driver_wext_get_ht_cap(void *drv_data, const char *ifname, char *buf, 
				       		  size_t *buf_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_GET_HT_CAP, buf, buf_len);

	return ret;

}

static int driver_wext_get_vht_cap(void *drv_data, const char *ifname, char *buf, 
				       		  size_t *buf_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_GET_VHT_CAP, buf, buf_len);

	return ret;

}

static int driver_wext_get_chan_list(void *drv_data, const char *ifname, char *buf, 
				       		  size_t *buf_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_GET_CHAN_LIST, buf, buf_len);

	return ret;

}

static int driver_wext_get_op_class(void *drv_data, const char *ifname, char *buf, 
				       		  size_t *buf_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_GET_OP_CLASS, buf, buf_len);

	return ret;

}

static int driver_wext_get_bss_info(void *drv_data, const char *ifname, char *buf, 
				       		  size_t *buf_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_GET_BSS_INFO, buf, buf_len);

	return ret;

}

static int driver_wext_get_ap_metrics(void *drv_data, const char *ifname, char *buf, 
				       		  size_t *buf_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_GET_AP_METRICS, buf, buf_len);

	return ret;

}

static int driver_wext_get_nop_list(void *drv_data, const char *ifname, char *buf, 
				       		  size_t *buf_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_GET_NOP_CHANNEL_LIST, buf, buf_len);

	return ret;

}


static int driver_wext_get_he_cap(void *drv_data, const char *ifname, char *buf, 
				       		  size_t *buf_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_GET_HE_CAP, buf, buf_len);

	return ret;

}


/*
 * Set information element to driver 
 */
static int driver_wext_set_ie(void *drv_data, const char *ifname, char *ie, 
				       		  size_t ie_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_WAPP_IE, ie, ie_len);

	return ret;
}

/*
 * Send ANQP request to driver
 */
static int driver_wext_send_anqp_req(void *drv_data, const char *ifname, 
							  		 const char *peer_mac_addr, const char *anqp_req, 
							 	 	 size_t anqp_req_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	struct anqp_req_data *req_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(*req_data) + anqp_req_len;	
	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	req_data = (struct anqp_req_data *)buf;
	
	os_memcpy(req_data->peer_mac_addr, peer_mac_addr, 6);
	
	req_data->anqp_req_len = anqp_req_len;
	os_memcpy(req_data->anqp_req, anqp_req, anqp_req_len);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_HS_ANQP_REQ, buf, len);


	os_free(buf);

	return ret;
}

static int driver_wext_cancel_roc(void *drv_data, const char *ifname)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_CANCEL_ROC, buf, len);

	os_free(buf);

	return ret;

}

static int driver_wext_start_roc(void *drv_data, const char *ifname, unsigned int chan,
                                      unsigned int wait_time)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	struct roc_req *req_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(*req_data);	
	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	req_data = (struct roc_req *)buf;

	req_data->chan = chan;
	req_data->wait_time = wait_time;

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_START_ROC, buf, len);

	os_free(buf);

	return ret;
}

static int driver_wext_send_action_frm(void *drv_data, const char *ifname, unsigned int chan,
                                      unsigned int wait_time,
                                      const u8 *dst, const u8 *src,
                                      const u8 *bssid,
                                      const u8 *data, size_t data_len,
				      u16 seq_no,
                                      int no_cck)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	struct action_frm_data *req_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(*req_data) + data_len;
	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	req_data = (struct action_frm_data *)buf;

	os_memcpy(req_data->destination_addr, dst, MAC_ADDR_LEN);
	os_memcpy(req_data->transmitter_addr, src, MAC_ADDR_LEN);
	os_memcpy(req_data->bssid, bssid, MAC_ADDR_LEN);
	DBGPRINT(RT_DEBUG_TRACE, " %s da = %02x%02x%02x%02x%02x%02x\033[0m\n", __FUNCTION__, PRINT_MAC(req_data->destination_addr));
	DBGPRINT(RT_DEBUG_TRACE, " %s ta = %02x%02x%02x%02x%02x%02x\033[0m\n", __FUNCTION__, PRINT_MAC(req_data->transmitter_addr));
	DBGPRINT(RT_DEBUG_TRACE, " %s bssid = %02x%02x%02x%02x%02x%02x\033[0m\n", __FUNCTION__, PRINT_MAC(req_data->bssid));

	req_data->frm_len = data_len;
	req_data->no_cck = no_cck;
	req_data->wait_time = wait_time;

	req_data->chan = chan;
	req_data->seq_no = seq_no;
	os_memcpy(req_data->frm, data, data_len);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_SEND_OFFCHAN_ACTION_FRAME, buf, len);

	os_free(buf);

	return ret;
}
#ifdef DPP_SUPPORT
static int driver_wext_set_pmk(void *drv_data, const char *ifname,
                     const u8 *pmk, size_t pmk_len, const u8 *pmkid,
                     const u8 *authenticator_addr, const u8 *supplicant_addr, int session_timeout,
                     int akmp)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	struct pmk_req *req_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(*req_data);
	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	req_data = (struct pmk_req *)buf;

	req_data->pmk_len = pmk_len;
	os_memcpy(req_data->pmk, pmk, LEN_PMK);
	os_memcpy(req_data->pmkid, pmkid, LEN_PMKID);
	os_memcpy(req_data->authenticator_addr, authenticator_addr, 6);
	os_memcpy(req_data->supplicant_addr, supplicant_addr, 6);
	req_data->timeout = session_timeout;
	req_data->akmp = akmp;

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_SET_PMK, buf, len);

	os_free(buf);

	return ret;
}
#endif /*DPP_SUPPORT*/

/*
 * Send ANQP response to driver
 */
static int driver_wext_send_anqp_rsp(void *drv_data, const char *ifname, 
							  		 const u8 *peer_mac_addr, const char *anqp_rsp, 
							  		 size_t anqp_rsp_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	struct anqp_rsp_data *rsp_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(*rsp_data) + anqp_rsp_len;
	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	rsp_data = (struct anqp_rsp_data *)buf;

	os_memcpy(rsp_data->peer_mac_addr, peer_mac_addr, 6);
	
	rsp_data->anqp_rsp_len = anqp_rsp_len;
	os_memcpy(rsp_data->anqp_rsp, anqp_rsp, anqp_rsp_len);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_HS_ANQP_RSP, buf, len);

	os_free(buf);

	return ret;
}

/*
 * Enable/Disable hotspot feature
 */
static int driver_wext_hotspot_onoff(void *drv_data, const char *ifname, 
					                 int enable, int event_trigger, int event_type)
{
	int ret;

	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	struct hs_onoff *onoff;
	char *buf;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	buf = os_zalloc(sizeof(*onoff));

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	onoff = (struct hs_onoff *)buf;

	onoff->hs_onoff = enable;
	onoff->event_trigger  = event_trigger;
	onoff->event_type = event_type;

	//drv_wext_data->we_version_compiled = driver_wext_get_we_version_compiled(drv_wext_data, ifname);
	//ret = driver_wext_set_oid(drv_data, ifname, OID_802_11_HS_IPCTYPE, &drv_wext_data->drv_mode, 1);
		
	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_HS_ONOFF, buf, sizeof(*onoff));

	os_free(buf);

	return ret;	
}

#ifdef MAP_R2

/*
 * Send ch scan req to driver
 */


static int driver_wext_ch_scan_req(void *drv_data, const char *ifname, const char *scan_msg,
									size_t msg_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(OFFCHANNEL_SCAN_MSG);
	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	os_memcpy(buf, scan_msg, msg_len);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_OFFCHANNEL_INFO, buf, len);

	os_free(buf);

	return ret;
}
#ifdef DFS_CAC_R2
int driver_wext_cac_req(void *drv_data, const char *ifname,
			 u32 param, u32 value)
{
	/* wext */
	int ret = 0;
	struct wapp_param_setting param_setting;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	param_setting.param = param;
	param_setting.value = value;


	DBGPRINT(RT_DEBUG_OFF, "%s\n", __FUNCTION__);

	if(param == WAPP_SET_CAC_STOP) {
		ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_CAC_STOP, 
									(char *)&param_setting, sizeof(param_setting));
	}
	return ret;
}
int driver_wext_mon_ch_assign(void *drv_data, const char *ifname,
			 char *buf, size_t buf_len)
{
	/* wext */
	int ret = 0;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_OFF, "%s\n", __FUNCTION__);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_DFS_ZERO_WAIT, 
									buf, buf_len);
	return ret;
}

#endif
#endif

/*
 * Send ch scan req to driver
 */


static int driver_wext_off_ch_scan_req(void *drv_data, const char *ifname, const char *scan_msg,
									size_t msg_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(OFFCHANNEL_SCAN_MSG);
	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	os_memcpy(buf, scan_msg, msg_len);
	printf("from WAPP sending OID_OFFCHANNEL_INFO\n");
	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_OFFCHANNEL_INFO, buf, len);

	os_free(buf);

	return ret;
}

/*
 * Send BTM query to driver
 */
static int driver_wext_send_btm_query(void *drv_data, const char *ifname, 
							  		  const char *peer_mac_addr, const char *btm_query, 
							  		  size_t btm_query_len)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	return ret;
}

/*
 * Send BTM request to driver
 */
static int driver_wext_send_btm_req(void *drv_data, const char *ifname,
									const u8 *peer_mac_addr, const char *btm_req,
									size_t btm_req_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	struct btm_req_data *req_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(*req_data) + btm_req_len;
	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	req_data = (struct btm_req_data *)buf;

	os_memcpy(req_data->peer_mac_addr, peer_mac_addr, 6);
	
	req_data->btm_req_len = btm_req_len;
	os_memcpy(req_data->btm_req, btm_req, btm_req_len);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_WNM_BTM_REQ, buf, len);

	os_free(buf);

	return ret;
}

/*
 * Send BTM response to driver
 */
static int driver_wext_send_btm_rsp(void *drv_data, const char *ifname,
									const u8 *peer_mac_addr, const char *btm_rsp,
									size_t btm_rsp_len)
{
	int ret = 0;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	struct btm_rsp_data *rsp_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(*rsp_data) + btm_rsp_len;
	buf = os_zalloc(len);
	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	rsp_data = (struct btm_rsp_data *)buf;
	os_memcpy(rsp_data->peer_mac_addr, peer_mac_addr, 6);
	rsp_data->btm_rsp_len = btm_rsp_len;
	os_memcpy(rsp_data->btm_rsp, btm_rsp, btm_rsp_len);
	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_WNM_BTM_RSP, buf, len);
	os_free(buf);

	return ret;
}

/*
 * Send reduced neighbor report list to driver
 */
static int driver_wext_send_reduced_nr_list(void *drv_data, const char *ifname,
									const char *reduced_nr_list,
									size_t reduced_nr_list_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	struct reduced_neighbor_list_data *reduced_nr_list_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(*reduced_nr_list_data) + reduced_nr_list_len;
	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	reduced_nr_list_data = (struct reduced_neighbor_list_data *)buf;

	reduced_nr_list_data->reduced_neighbor_list_len = reduced_nr_list_len;
	os_memcpy(reduced_nr_list_data->reduced_neighbor_list_req, reduced_nr_list, reduced_nr_list_len);
	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_OCE_REDUCED_NEIGHBOR_REPORT, buf, len);

	os_free(buf);

	return ret;
	
}

/*
 * Send Qos Map Configure frame to driver
 */
static int driver_wext_send_qosmap_configure(void *drv_data, const char *ifname,
									const char *peer_mac_addr, const char *qosmap,
									size_t qosmap_len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	struct qosmap_data *req_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(*req_data) + qosmap_len;
	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	req_data = (struct qosmap_data *)buf;

	os_memcpy(req_data->peer_mac_addr, peer_mac_addr, 6);
	
	req_data->qosmap_len = qosmap_len;
	os_memcpy(req_data->qosmap, qosmap, qosmap_len);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_QOSMAP_CONFIGURE, buf, len);

	os_free(buf);

	return ret;
}

/*
 * Send WNM request to driver
 */
static int driver_wext_send_wnm_notify_req(void *drv_data, const char *ifname,
									const char *peer_mac_addr, const char *wnm_req,
									size_t wnm_req_len, int type)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char *buf;
	size_t len = 0;
	struct wnm_req_data *req_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	len = sizeof(*req_data) + wnm_req_len;
	buf = os_zalloc(len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:Not available memory\n", __FUNCTION__);
		return -1;
	}

	req_data = (struct wnm_req_data *)buf;

	os_memcpy(req_data->peer_mac_addr, peer_mac_addr, 6);
	
	req_data->wnm_req_len = wnm_req_len;
	req_data->type = type;
	os_memcpy(req_data->wnm_req, wnm_req, wnm_req_len);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_WNM_NOTIFY_REQ, buf, len);

	os_free(buf);

	return ret;
}
/*
 * wapp Parameter Setting
 */
static int driver_wext_wapp_param_setting(void *drv_data, const char *ifname,
									 u32 param, u32 value)
{
	int ret;
	struct wapp_param_setting param_setting;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	param_setting.param = param;
	param_setting.value = value;


	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_WAPP_PARAM_SETTING, 
									(char *)&param_setting, sizeof(param_setting));
	
	return ret;
}

/*
 * Hotspot 2.0 Parameter Setting
 */
static int driver_wext_hs_param_setting(void *drv_data, const char *ifname,
									 u32 param, u32 value)
{
	int ret;
	struct wapp_param_setting param_setting;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	param_setting.param = param;
	param_setting.value = value;


	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_WAPP_PARAM_SETTING, 
									(char *)&param_setting, sizeof(param_setting));
	
	return ret;
}

/*
 * WiFi driver verion
 */
static int driver_wext_wifi_version(void *drv_data, const char *ifname,
							   		char *ver, size_t *len)
{
	int ret;

	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_802_11_WIFI_VER, ver, len);

	return ret;
}	

static int driver_wext_wapp_support_version(void *drv_data, const char *ifname)
{
	int ret;

	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;	
	char ver[16] = {0};
	size_t len = sizeof(ver);
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	DBGPRINT_RAW(RT_DEBUG_OFF, GRN("=====\nWAPP DAEMON VER:%s\n=====\n"),WAPP_VERSION);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_802_11_WAPP_SUPPORT_VER, ver, &len);
	if (ret != 0) {
		DBGPRINT(RT_DEBUG_ERROR, RED("driver only support old WAPP (ver < v2.0.0)\n"));
	} else {
		DBGPRINT(RT_DEBUG_OFF, GRN("\ndriver_support_wapp_ver=[%s] \n"),ver);
		if(os_memcmp(WAPP_VERSION, ver, os_strlen(WAPP_VERSION)) != 0)
			DBGPRINT(RT_DEBUG_ERROR, RED("driver support WAPP ver (%s) < cur WAPP ver (%s)\n")
				, ver, WAPP_VERSION);
	}

	return ret;
}	


/*
 * Get IPv4 Proxy ARP list
 */
static int driver_wext_ipv4_proxy_arp_list(void *drv_data, const char *ifname,
									  	   char *proxy_arp_list, size_t *proxy_arp_list_len)
{
	int ret;

	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_802_11_WNM_IPV4_PROXY_ARP_LIST,
									proxy_arp_list, proxy_arp_list_len);

	return ret;
}

/*
 * Get IPv6 Proxy ARP list
 */
static int driver_wext_ipv6_proxy_arp_list(void *drv_data, const char *ifname,
									  	   char *proxy_arp_list, size_t *proxy_arp_list_len)
{
	int ret;

	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_802_11_WNM_IPV6_PROXY_ARP_LIST,
									proxy_arp_list, proxy_arp_list_len);

	return ret;
}

static int driver_wext_validate_security_type(void *drv_data, const char *ifname)
{
	
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	struct security_type sec_type;
	size_t security_type_len = sizeof(sec_type);

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	os_memset(&sec_type, 0, sizeof(sec_type));

	DBGPRINT(RT_DEBUG_TRACE, "%s: sec_type_len = %u\n", __FUNCTION__, (UINT32)sizeof(sec_type));
	
	driver_wext_get_oid(drv_wext_data, ifname, OID_802_11_SECURITY_TYPE, (char *)&sec_type, &security_type_len);

	DBGPRINT(RT_DEBUG_TRACE, "%s: auth_mode = %d, encryp_type = %d\n", __FUNCTION__, sec_type.auth_mode,
								sec_type.encryp_type);

	if (((sec_type.auth_mode == 6) || (sec_type.auth_mode == 8)) && 
							((sec_type.encryp_type == 6) || (sec_type.encryp_type == 8)))
		return 1;
	else
		return 0;
}

static int driver_wext_reset_resource(void *drv_data, const char *ifname)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_HS_RESET_RESOURCE, NULL, 0);

	return ret;
}

static int driver_wext_get_bssid(void *drv_data, const char *ifname, char *bssid, size_t *bssid_len)
{

	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_802_11_HS_BSSID, bssid, bssid_len);

	return ret;
}

int driver_wext_get_bss_coex(void *drv_data, const char *ifname, char *bss_coex)
{

	int ret;
	size_t len = 4;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname,
		OID_802_11_COEXISTENCE, bss_coex, &len);
	return ret;
}

int driver_wext_get_set_uuid(void *drv_data, const char *ifname, char *uuid, BOOLEAN set)
{

	int ret;
	size_t len = 16;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	if (set)
		ret = driver_wext_set_oid(drv_wext_data, ifname,
		OID_WSC_UUID, uuid, 16);
	else
		ret = driver_wext_get_oid(drv_wext_data, ifname,
		OID_WSC_UUID, uuid, &len);
	return ret;
}

int driver_wext_set_ssid(void *drv_data, const char *ifname, char *ssid)
{

	int ret,len,i,j;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char cmd[MAX_CMD_MSG_LEN];
	char lssid[64];
	char dquote=0x22;
	char squote=0x27;
	char backslash = 0x5C;
	char bquote = 0x60;

	DBGPRINT(RT_DEBUG_OFF, "%s : %s\n", __FUNCTION__, ssid);

	for(i=0,j=0;i<strlen(ssid);i++)
	{
		if(*(ssid+i) == dquote) {
			lssid[j] = backslash;
			j++;
			lssid[j] = dquote;
			j++;
		} else if (*(ssid+i) == squote){
			lssid[j] = backslash;
			j++;
			lssid[j] = squote;
			j++;
		} else if (*(ssid+i) == bquote){
			lssid[j] = backslash;
			j++;
			lssid[j] = bquote;
			j++;
		} else {
			lssid[j] = *(ssid+i);
			j++;
		}
	}
	lssid[j] = '\0';

	ret = driver_wext_set_oid(drv_wext_data, ifname,
		OID_SET_SSID, ssid, strlen(ssid));

	os_memset(cmd,0,sizeof(cmd));
	sprintf(cmd, "wifi_config_save %s SSID ", ifname);
	len=strlen(cmd);
	cmd[len]='"';
	cmd[len+1]='\0';
	strncat(cmd,lssid,strlen(lssid));
	len=strlen(cmd);
	cmd[len]='"';
	cmd[len+1]='\0';
	ret = system(cmd);
	DBGPRINT(RT_DEBUG_OFF," [%s] Send SSID Command: %s, ret = %d\n", __func__, cmd, ret);
	return ret;
}
int driver_wext_set_psk(void *drv_data, const char *ifname, char *psk)
{

	int ret,len;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	char cmd[MAX_CMD_MSG_LEN];

	DBGPRINT(RT_DEBUG_OFF, "%s : %s\n", __FUNCTION__, psk);

	ret = driver_wext_set_oid(drv_wext_data, ifname,
		OID_SET_PSK, psk, strlen(psk));

	os_memset(cmd,0,sizeof(cmd));
	sprintf(cmd, "wifi_config_save %s WPAPSK ", ifname);
	len=strlen(cmd);
	cmd[len]='"';
	cmd[len+1]='\0';
	strncat(cmd,psk,strlen(psk));
	len=strlen(cmd);
	cmd[len]='"';
	cmd[len+1]='\0';
	ret = system(cmd);
	DBGPRINT(RT_DEBUG_OFF," [%s] Send WPAPSK Command: %s, ret = %d\n", __func__, cmd, ret);

	return ret;
}

static int driver_wext_get_osu_ssid(void *drv_data, const char *ifname, char *ssid, size_t *ssid_len)
{

	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = driver_wext_get_oid(drv_wext_data, ifname, OID_802_11_HS_OSU_SSID, ssid, ssid_len);

	return ret;
}

static int driver_wext_set_osu_asan(void *drv_data, const char *ifname, char *enable, size_t len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_HS_SASN_ENABLE, enable, len);

	return ret;
}

static int driver_wext_set_bss_load(void *drv_data, const char *ifname, char *buf, size_t len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_BSS_LOAD, buf, len);

	return ret;
}

static int driver_wext_set_interworking(void *drv_data, const char *ifname, char *enable, size_t len)
{
	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_INTERWORKING_ENABLE, enable, len);

	return ret;
}

int driver_wext_mbo_param_setting(void *drv_data, const char *ifname,
									 u32 param, u32 value)
{
	/* wext */
	int ret;
	struct wapp_param_setting param_setting;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	param_setting.param = param;
	param_setting.value = value;


	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = driver_wext_set_oid(drv_wext_data, ifname, OID_802_11_MBO_MSG, 
									(char *)&param_setting, sizeof(param_setting));
	
	return ret;
}

static int driver_wext_send_wapp_req(
				void *drv_data,
				const char *ifname,
				struct wapp_req *req)
{
	int ret;
	struct driver_wext_data *drv_wext_data = \
					(struct driver_wext_data *)drv_data;	

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_set_oid(
				drv_wext_data,
				ifname,
				OID_WAPP_EVENT,
				(char *) req,
				(sizeof(struct wapp_req)));

	return ret;
}
int driver_wext_get_wsc_profiles(void *drv_data, const char *ifname, char *wsc_profile_data, int *length)
{
	int ret;
	size_t len = 512;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	ret = driver_wext_get_oid(drv_wext_data, ifname,
	    	OID_GET_WSC_PROFILES, wsc_profile_data, &len );
	*length=(int)len;
	return ret;
}

int driver_wext_get_chip_id(void *drv_data, const char *ifname, char *buf, 
				       		  size_t *buf_len)
{

	int ret;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	ret = driver_wext_get_oid(drv_wext_data, ifname,
		OID_MTK_CHIP_ID, buf, buf_len);
	return ret;
}

int driver_wext_set_get_oid(struct driver_wext_data *drv_data, const char *ifname,
                                                                   unsigned short oid, char *data, size_t *len)
{
        struct iwreq iwr;
        unsigned char *buf;

        DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
        os_memset(&iwr, 0, sizeof(iwr));
        snprintf(iwr.ifr_name, IFNAMSIZ, "%s", ifname);

        if (*len > 4096) {
                DBGPRINT(RT_DEBUG_ERROR, "%s: len = %zu\n", __FUNCTION__, *len);
                return -1;
        }

        buf = os_zalloc(4096);

         if (data)
                os_memcpy(buf, data, *len);

        iwr.u.data.pointer = (void *)buf;
        iwr.u.data.flags = oid;
        iwr.u.data.length = *len;

        if (ioctl(drv_data->ioctl_sock, RT_PRIV_IOCTL, &iwr) < 0) {
        DBGPRINT(RT_DEBUG_ERROR, "%s: oid=0x%x len (%zu) failed\n",
               __FUNCTION__, oid, *len);
                *len = 0;
                os_free(buf);
                return -1;
        }

        if (iwr.u.data.length < *len) {
                if (data)
                        os_memcpy(data, buf, iwr.u.data.length);
                *len = iwr.u.data.length;
        } else {
                if (data)
                        os_memcpy(data, buf, *len);
        }

        os_free(buf);

        return 0;
}

#ifdef DPP_SUPPORT
int driver_wext_get_dpp_frame(void *drv_data, const char *ifname, char *frame, int length)
{
	int ret;
	size_t len = length;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	ret = driver_wext_set_get_oid(drv_wext_data, ifname,
			OID_802_11_GET_DPP_FRAME, frame, &len);
	length=(int)len;
	return ret;
}
#endif /* DPP_SUPPORT */

int driver_wext_get_assoc_req_frame(void *drv_data, const char *ifname, char *assoc_data, int length)
{
	int ret;
	size_t len = length;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	// printf("MACSTR in wext-%02x:%02x:%02x:%02x:%02x:%02x\n", assoc_data[0],assoc_data[1], assoc_data[2], assoc_data[3],assoc_data[4],assoc_data[5]);	
	ret = driver_wext_set_get_oid(drv_wext_data, ifname,
			OID_GET_ASSOC_REQ_FRAME, assoc_data, &len);
	length=(int)len;
	return ret;
}
#ifdef MAP_R2

#ifdef DFS_CAC_R2
int driver_wext_get_cac_capability(void *drv_data, const char *ifname, char *buf, unsigned int length)
{
	int ret;
	//size_t len = 512;
	size_t len = length;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
//	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	ret = driver_wext_get_oid(drv_wext_data, ifname,
			OID_GET_CAC_CAP, buf, &len);
	length=(unsigned int)len;
	return ret;
}
int driver_wext_get_avail_channel(void *drv_data, const char *ifname, char *buf, unsigned int length)
{
	int ret;
	size_t len = length;
	struct driver_wext_data *drv_wext_data = (struct driver_wext_data *)drv_data;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __FUNCTION__);
	ret = driver_wext_get_oid(drv_wext_data, ifname,
			OID_DFS_ZERO_WAIT, buf, &len);
	length = (unsigned int)len;
	return ret;
}


#endif/* DFS_CAC_R2 */
#endif/* MAP_R2 */

#ifdef KV_API_SUPPORT
int driver_rrm_onoff(void *drv_data, const char *ifname, int onoff)
{
	struct rrm_command_s *cmd = NULL;
	int cmd_len = 0;

	DBGPRINT(RT_DEBUG_OFF, "%s\n", __FUNCTION__);

	cmd_len = sizeof(struct rrm_command_s) + 1;
	cmd = (struct rrm_command_s *)os_zalloc(cmd_len);

	if (!cmd) {
		DBGPRINT(RT_DEBUG_OFF, "%s: cmd_data alloc fail\n", __FUNCTION__);
		return 0;
	}

	cmd->command_id = OID_802_11_RRM_CMD_ENABLE;
	cmd->command_len = 1;
	cmd->command_body[0] = onoff;

	driver_wext_set_oid(drv_data, ifname, OID_802_11_RRM_COMMAND, (char *)cmd, cmd_len);

	os_free(cmd);
	return 0;
}

int driver_rrm_send_bcn_req_param(void *drv_data, const char *ifname,
							const char *bcn_req_param, u32 param_len)
{
	int len;
	p_rrm_command_t cmd_data = NULL;

	DBGPRINT(RT_DEBUG_OFF, "%s\n", __FUNCTION__);

	len = sizeof(*cmd_data) + param_len;
	cmd_data = (p_rrm_command_t)os_zalloc(len);

	if (!cmd_data) {
		DBGPRINT(RT_DEBUG_OFF, "%s: cmd_data alloc fail\n", __FUNCTION__);
		return 0;
	}

	cmd_data->command_id = OID_802_11_RRM_CMD_SET_BEACON_REQ_PARAM;
	cmd_data->command_len = param_len;

	DBGPRINT(RT_DEBUG_OFF, "444\n");

	os_memcpy(cmd_data->command_body, bcn_req_param, param_len);

	driver_wext_set_oid(drv_data, ifname, OID_802_11_RRM_COMMAND, (char *)cmd_data, len);

	DBGPRINT(RT_DEBUG_OFF, "555\n")
	os_free(cmd_data);
	return 0;
}

int driver_wnm_btm_onoff(void * drv_data, const char *ifname, int onoff)
{
	struct wnm_command *cmd = NULL;
	int cmd_len;

	DBGPRINT(RT_DEBUG_OFF, "%s\n", __FUNCTION__);

	cmd_len = sizeof(struct wnm_command) + 2;
	cmd = (struct wnm_command *)os_zalloc(cmd_len);

	if (!cmd) {
		DBGPRINT(RT_DEBUG_OFF, "%s: cmd alloc fail\n", __FUNCTION__);
		return 0;
	}

	cmd->command_id = OID_802_11_WNM_CMD_CAP;
	cmd->command_len = 2;
	cmd->command_body[0] |= onoff ;
	cmd->command_body[1]= 0;

	DBGPRINT(RT_DEBUG_OFF, "%s: cmd->command_body[0] = %d\n", __FUNCTION__, cmd->command_body[0]);
	driver_wext_set_oid(drv_data,ifname,OID_802_11_WNM_COMMAND, (char *)cmd, cmd_len);

	os_free(cmd);
	return 0;
}

int driver_wnm_send_btm_req_param(void *drv_data, const char *ifname,
							const char *btm_req_param, u32 param_len)
{
	int len;
	struct wnm_command *cmd_data = NULL;

	DBGPRINT(RT_DEBUG_OFF, "%s\n", __FUNCTION__);

	len = sizeof(struct wnm_command)+ param_len;
	cmd_data = (struct wnm_command *)os_zalloc(len);

	if (!cmd_data) {
		DBGPRINT(RT_DEBUG_OFF, "%s: cmd_data alloc fail\n", __FUNCTION__);
		return 0;
	}

	cmd_data->command_id = OID_802_11_WNM_CMD_SET_BTM_REQ_PARAM;
	cmd_data->command_len = param_len;

	os_memcpy(cmd_data->command_body, btm_req_param,param_len);

	driver_wext_set_oid(drv_data,ifname, OID_802_11_WNM_COMMAND, (char *)cmd_data, len);
	os_free(cmd_data);
	return 0;
}

int driver_wnm_send_btm_req_raw(void *drv_data, const char *ifname,
							const char *btm_req_raw, u32 param_len)
{
	int len;
	struct wnm_command *cmd_data = NULL;

	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);

	len = sizeof(struct wnm_command)+ param_len;
	cmd_data = (struct wnm_command *)os_zalloc(len);

	if (!cmd_data) {
		DBGPRINT(RT_DEBUG_OFF, "%s: cmd_data alloc fail\n", __FUNCTION__);
		return 0;
	}

	cmd_data->command_id = OID_802_11_WNM_CMD_SEND_BTM_REQ_IE;
	cmd_data->command_len = param_len;

	os_memcpy(cmd_data->command_body, btm_req_raw,param_len);

	driver_wext_set_oid(drv_data,ifname, OID_802_11_WNM_COMMAND, (char *)cmd_data, len);
	os_free(cmd_data);
	return 0;
}

int wapp_send_btm_req_11kv_api(struct wifi_app *wapp,
						 const char *ifname,
						 const u8 *peer_mac_addr,
						 const char *btm_req,
						 size_t btm_req_len)
{
	p_btm_req_ie_data_t p_btm_req_data = NULL;
	unsigned int len = 0;

	DBGPRINT(RT_DEBUG_ERROR, "%s  peer_mac_addr %02X:%02X:%02X:%02X:%02X:%02X\n",
			__func__, PRINT_MAC(peer_mac_addr));

	len = btm_req_len + sizeof(*p_btm_req_data);
	p_btm_req_data = (p_btm_req_ie_data_t)os_zalloc(len);
	if(!p_btm_req_data) {
		DBGPRINT(RT_DEBUG_ERROR, "btm_req mem alloc fail\n");
		return WAPP_NOT_INITIALIZED;
	}

	COPY_MAC_ADDR(p_btm_req_data->peer_mac_addr, peer_mac_addr);
	memcpy(p_btm_req_data->btm_req, btm_req, btm_req_len);
	p_btm_req_data->btm_req_len = btm_req_len;
	p_btm_req_data->dialog_token = 1;

	driver_wnm_send_btm_req_raw(wapp->drv_data, ifname, (char *)p_btm_req_data, len);

	os_free(p_btm_req_data);
	return WAPP_SUCCESS;
}
#endif /* KV_API_SUPPORT */

const struct wapp_drv_ops wapp_drv_wext_ops = {
	.drv_inf_init = driver_wext_init,
	.drv_inf_exit = driver_wext_exit,
	.drv_wifi_version = driver_wext_wifi_version,
	.drv_wapp_version_check = driver_wext_wapp_support_version,
	.drv_wapp_req = driver_wext_send_wapp_req,
	.drv_wapp_param_setting = driver_wext_wapp_param_setting,
	.drv_send_wnm_notify_req = driver_wext_send_wnm_notify_req,
	.drv_send_btm_req = driver_wext_send_btm_req,
	.drv_send_btm_query = driver_wext_send_btm_query,
	.drv_send_btm_rsp = driver_wext_send_btm_rsp,
	.drv_send_reduced_nr_list = driver_wext_send_reduced_nr_list,
	.drv_set_interworking = driver_wext_set_interworking,
	.drv_send_anqp_req = driver_wext_send_anqp_req,
	.drv_send_anqp_rsp = driver_wext_send_anqp_rsp,
	.drv_set_ie = driver_wext_set_ie,
#ifdef MAP_R2
	.drv_ch_scan_req = driver_wext_ch_scan_req,
#ifdef DFS_CAC_R2
	.drv_cac_req = driver_wext_cac_req,
	.drv_set_monitor_ch_assign = driver_wext_mon_ch_assign,
	.drv_get_ch_avail_list = driver_wext_get_avail_channel,
#endif
#endif
	.drv_get_misc_cap = driver_wext_get_misc_cap,
	.drv_get_ht_cap = driver_wext_get_ht_cap,
	.drv_get_vht_cap = driver_wext_get_vht_cap,
	.drv_get_he_cap = driver_wext_get_he_cap,
	.drv_get_chan_list = driver_wext_get_chan_list,
	.drv_get_op_class = driver_wext_get_op_class,
	.drv_get_bss_info = driver_wext_get_bss_info,
	.drv_get_ap_metrics = driver_wext_get_ap_metrics,
	.drv_off_ch_scan_req = driver_wext_off_ch_scan_req,
	.drv_get_nop_channels = driver_wext_get_nop_list,	
	.drv_get_chip_id = driver_wext_get_chip_id,
	.drv_set_ssid = driver_wext_set_ssid,
	.drv_set_psk = driver_wext_set_psk,
	.send_action = driver_wext_send_action_frm,
	.drv_cancel_roc = driver_wext_cancel_roc,
	.drv_start_roc = driver_wext_start_roc,
#ifdef DPP_SUPPORT
	.drv_set_pmk = driver_wext_set_pmk,
#endif /*DPP_SUPPORT*/
};

const struct hotspot_drv_ops hotspot_drv_wext_ops = {	
	.drv_hotspot_onoff = driver_wext_hotspot_onoff,	
	.drv_hs_param_setting = driver_wext_hs_param_setting,	
	.drv_ipv4_proxy_arp_list = driver_wext_ipv4_proxy_arp_list,
	.drv_ipv6_proxy_arp_list = driver_wext_ipv6_proxy_arp_list,
	.drv_validate_security_type = driver_wext_validate_security_type,
	.drv_reset_resource = driver_wext_reset_resource,
	.drv_get_bssid = driver_wext_get_bssid,
	.drv_get_osu_ssid = driver_wext_get_osu_ssid,
	.drv_set_osu_asan = driver_wext_set_osu_asan,	
	.drv_send_qosmap_configure = driver_wext_send_qosmap_configure,
	.drv_set_bss_load = driver_wext_set_bss_load,	
};

const struct mbo_drv_ops mbo_drv_wext_ops = {
	.drv_mbo_param_setting = driver_wext_mbo_param_setting,
};

