/*
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2018, Mediatek, Inc.
 * *
 * * All rights reserved. Mediatek's source code is an unpublished work and the
 * * use of a copyright notice does not imply otherwise. This source code
 * * contains confidential trade secret material of Ralink Tech. Any attemp
 * * or participation in deciphering, decoding, reverse engineering or in any
 * * way altering the source code is stricitly prohibited, unless the prior
 * * written consent of Mediatek, Inc. is obtained.
 * ***************************************************************************
 *
 *  Module Name:
 *  metrics query/response
 *
 *  Abstract:
 *  metrics query/response
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Kapil.Gupta 2018/05/02    First implementation of the metrics query/response
 * */
#include "includes.h"
#ifdef __linux__
#include <fcntl.h>
#endif				/* __linux__ */

#include "common.h"
#include <sys/un.h>

#include "interface.h"
#include "./../1905_local_lib/data_def.h"
#include "client_db.h"
#include "mapd_i.h"
#include "topologySrv.h"
#include "eloop.h"
#include "wapp_if.h"
#include "tlv_parsor.h"
#include "1905_if.h"
#include "1905_map_interface.h"
#include "ap_est.h"
#include "wapp_if.h"
#include "client_mon.h"
#include "ap_cent_str.h"
#include "apSelection.h"

void infra_metrics_srv_send_link_metrics_query(void *eloop_ctx, void *timeout_ctx);
void infra_metrics_srv_send_cb_infra_metrics(void *eloop_ctx, void *timeout_ctx);

int parse_associated_sta_link_metrics_rsp_msg(struct own_1905_device *ctx, struct _1905_map_device *dev, unsigned char *buf)
{
        int length = 0;
        unsigned char *temp_buf;

        temp_buf = buf;
        info(" ");
        while (1) {
                if (*temp_buf == ASSOC_STA_LINK_METRICS_TYPE) {
                        length = parse_assoc_sta_link_metrics_tlv(ctx, dev, temp_buf);
                        if(length < 0) {
                                err("error associated sta link metrics query tlv");
                                return -1;
                        }
                        temp_buf += length;
                } else if (*temp_buf == END_OF_TLV_TYPE) {
                        break;
                } else {
                        length = get_cmdu_tlv_length(temp_buf);
                        temp_buf += length;
                }
        }

        return 0;
}

/**
* @brief Fn to send link metrics query to every 1905 device as part of infra metrics service
*
* @param eloop_ctx eloop ctx
* @param timeout_ctx timeout ctx
*/
void infra_metrics_srv_send_link_metrics_query(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct own_1905_device *own_dev = &global->dev;
	struct _1905_map_device *dev;
	unsigned char metrics = BOTH_TX_AND_RX_METRICS;
	char neighbor_almac[ETH_ALEN] = { 0 };
	unsigned char neighbor = 0x00;

	debug("trace");
	SLIST_FOREACH(dev, &own_dev->_1905_dev_head, next_1905_device) {
		if (dev == topo_srv_get_1905_device(own_dev, NULL))
			continue;
		map_1905_Send_Link_Metric_Query_Message(global->_1905_ctrl, (char *)dev->_1905_info.al_mac_addr, neighbor, neighbor_almac,
							metrics);
	}

	/* start timer to shoot every 30 sec for each 1905 to collect link metrics */
	eloop_register_timeout(30, 0, infra_metrics_srv_send_link_metrics_query, global, own_dev);
}

/**
* @brief fill combined infra metrics to be set for 1905
*
* @param ctx own 1905 ctx
* @param ap_met_cnt ap metrics count
* @param ap_metrics_ptr ap metrics pointet
* @param link_cnt link count
* @param tx_ap_ptr tx ap metrics pointer
* @param tx_sta_ptr tx sta metics pointer
* @param rx_ap_ptr rx ap metrics pointer
* @param rx_sta_ptr rx sta metics pointer
*
* @return 0 if success 
*/
unsigned short combined_infrastructure_metrics_message(struct own_1905_device *ctx, int *ap_met_cnt,
						       struct ap_metrics_info_lib *ap_metrics_ptr, int *link_cnt,
						       struct tx_link_metrics *tx_ap_ptr,
						       struct tx_link_metrics *tx_sta_ptr,
						       struct rx_link_metrics *rx_ap_ptr,
						       struct rx_link_metrics *rx_sta_ptr)
{
	int is_ap;
	unsigned char *ap_metrics_buf, *tx_ap_buf, *rx_ap_buf, *tx_sta_buf, *rx_sta_buf;

	struct bss_info_db *bss = NULL;
	struct _1905_map_device *tmp_dev;
	struct map_neighbor_info *neighbor;
	int bh_link_cnt = 0, ap_metrics_cnt = 0, count;
	struct ap_metrics_info_lib *valid_ap_metric = NULL;

	debug("enter");
	ap_metrics_buf = (unsigned char *)ap_metrics_ptr;
	tx_ap_buf = (unsigned char *)tx_ap_ptr;
	tx_sta_buf = (unsigned char *)tx_sta_ptr;
	rx_ap_buf = (unsigned char *)rx_ap_ptr;
	rx_sta_buf = (unsigned char *)rx_sta_ptr;

	SLIST_FOREACH(tmp_dev, &ctx->_1905_dev_head, next_1905_device) {
		SLIST_FOREACH(bss, &tmp_dev->first_bss, next_bss) {
			/* TODO make a proper check */
			if (bss->esp_cnt != 4) {
				debug("skipping bssid(%02x:%02x:%02x:%02x:%02x:%02x) ch_uti=%d, assoc_sta_cnt=%d, valid_esp_cnt=%d",
					PRINT_MAC(bss->bssid),bss->ch_util,
					bss->assoc_sta_cnt, bss->esp_cnt);
				continue;
			}
			append_ap_metrics_info(ap_metrics_buf, bss);
			valid_ap_metric = (struct ap_metrics_info_lib *)ap_metrics_buf;
			if (valid_ap_metric->valid_esp_count > 0) {
				ap_metrics_cnt++;
				ap_metrics_buf += sizeof(struct ap_metrics_info_lib);
			}
		}

		if (SLIST_EMPTY(&tmp_dev->neighbors_entry))
			continue;

		neighbor = SLIST_FIRST(&tmp_dev->neighbors_entry);

		SLIST_FOREACH(neighbor, &tmp_dev->neighbors_entry, next_neighbor) {
			count =
			    append_link_metric_info(tx_ap_buf, rx_ap_buf, tx_sta_buf, rx_sta_buf, tmp_dev, neighbor,
						    &is_ap);
			if (count > 0) {
				bh_link_cnt++;
				if (is_ap) {
					tx_ap_buf +=sizeof(struct tx_link_metrics);
					rx_ap_buf +=sizeof(struct rx_link_metrics);
				} else {
					tx_sta_buf +=sizeof(struct tx_link_metrics);
					rx_sta_buf +=sizeof(struct rx_link_metrics);
			}
			}
		}
	}
	/* if odd, then we missed something */
	debug("link count =%d", bh_link_cnt);
	bh_link_cnt = bh_link_cnt / 2;

	*ap_met_cnt = ap_metrics_cnt;
	*link_cnt = bh_link_cnt;

	return 0;
}

/**
* @brief Fn to set combined infra metrics info in 1905 daemon
*
* @param eloop_ctx
* @param timeout_ctx
*/
void infra_metrics_srv_send_cb_infra_metrics(void *eloop_ctx, void *timeout_ctx)
{
#define MAX_SIZE 2048
#define MAX_AP_METRICS_SIZE 4096
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct own_1905_device *own_dev = &global->dev;
	struct _1905_map_device *dev;
	struct ap_metrics_info_lib *ap_metrics;
	struct tx_link_metrics *tx_sta_buf, *tx_ap_buf;
	struct rx_link_metrics *rx_sta_buf, *rx_ap_buf;
	int bh_link_cnt, ap_metrics_cnt;

	//TODO should we use realloc here??
	ap_metrics = os_zalloc(MAX_AP_METRICS_SIZE);
	tx_sta_buf = os_zalloc(MAX_SIZE);
	tx_ap_buf = os_zalloc(MAX_SIZE);
	rx_sta_buf = os_zalloc(MAX_SIZE);
	rx_ap_buf = os_zalloc(MAX_SIZE);


	if(!ap_metrics || !tx_sta_buf || !tx_ap_buf || !rx_sta_buf || !rx_ap_buf){
		err("MAlloc Failed !!!!!!!!!!!!!!!!");
		if(ap_metrics)
			os_free(ap_metrics);
		if(tx_sta_buf)
			os_free(tx_sta_buf);
		if(tx_ap_buf)
			os_free(tx_ap_buf);
		if(rx_sta_buf)
			os_free(rx_sta_buf);
		if(rx_ap_buf)
			os_free(rx_ap_buf);

		eloop_register_timeout(60, 0, infra_metrics_srv_send_cb_infra_metrics, global, own_dev);
		return;
	}
#if 1
	topo_srv_cont_update_ap_metrics(own_dev);
	topo_srv_cont_update_link_metrics(own_dev);
#undef MAX_SIZE
	combined_infrastructure_metrics_message(own_dev, &ap_metrics_cnt, ap_metrics, &bh_link_cnt,
						tx_ap_buf, tx_sta_buf, rx_ap_buf, rx_sta_buf);
	if (bh_link_cnt > 0) {
		SLIST_FOREACH(dev, &own_dev->_1905_dev_head, next_1905_device) {
			if (dev == topo_srv_get_1905_device(own_dev, NULL))
				continue;
			map_1905_Send_Combined_Infrastructure_Metrics_Message(global->_1905_ctrl, (char *)dev->_1905_info.al_mac_addr,
									      ap_metrics_cnt, ap_metrics, bh_link_cnt,
									      tx_ap_buf, tx_sta_buf, rx_ap_buf, rx_sta_buf);
		}
	}
#endif
	free(ap_metrics);
	free(tx_sta_buf);
	free(tx_ap_buf);
	free(rx_sta_buf);
	free(rx_ap_buf);
	/* start timer to send combined infra every 60 sec?? */
	eloop_register_timeout(60, 0, infra_metrics_srv_send_cb_infra_metrics, global, own_dev);
}

/**
* @brief Fn to start combined infra metrics
*
* @param ctx own 1905 device ctx
*
* @return 0 
*/
int topo_srv_start_combined_infra_metrics_srv(struct own_1905_device *ctx)
{

	struct mapd_global *global = ctx->back_ptr;

	if(global->params.Certification) {
		return 0;
	}

	if (ctx->device_role != DEVICE_ROLE_CONTROLLER)	/* controller */
		return 0;

	/* Standalone MAPD without 1905 */
	if (is_1905_present() == FALSE)
		return 0;

	info("started combined infra metric service");
	/* start timer to shoot every 30 sec for each 1905 to collect link metrics */
	eloop_register_timeout(30, 0, infra_metrics_srv_send_link_metrics_query, ctx->back_ptr, ctx);
	/* start timer to send combined infra every 60 sec?? */
	eloop_register_timeout(60, 0, infra_metrics_srv_send_cb_infra_metrics, ctx->back_ptr, ctx);

	return 0;
}

/**
* @brief Fn to append link metrics info
*
* @param ap_tx_link ap tx link info
* @param ap_rx_link ap rx link info
* @param sta_tx_link sta tx link info
* @param sta_rx_link sta rx link info
* @param dev 1905 map device pointer
* @param neighbor map neighbor pointer
* @param is_ap whether its an ap or sta
*
* @return 0 if success else -1
*/
int append_link_metric_info(unsigned char *ap_tx_link, unsigned char *ap_rx_link, unsigned char *sta_tx_link,
			    unsigned char *sta_rx_link, struct _1905_map_device *dev,
			    struct map_neighbor_info *neighbor, int *is_ap)
{
	int i = 0, link_count = 0;
	struct backhaul_link_info *link;
	struct tx_link_metrics_sub *tx_link_sub;
	struct tx_link_metrics tx_link;
	struct rx_link_metrics_sub *rx_link_sub;
	struct rx_link_metrics rx_link;
	struct iface_info *iface = NULL;

	/*fill into local abstration layer mac addr */
	memcpy(tx_link.almac, dev->_1905_info.al_mac_addr, ETH_ALEN);
	memcpy(rx_link.almac, dev->_1905_info.al_mac_addr, ETH_ALEN);

	/*fill into neighbor abstration layer mac addr */
	memcpy(tx_link.neighbor_almac, neighbor->n_almac, ETH_ALEN);
	memcpy(rx_link.neighbor_almac, neighbor->n_almac, ETH_ALEN);

	SLIST_FOREACH(link, &neighbor->bh_head, next_bh) {
		iface = topo_srv_get_interface(NULL, dev, link->connected_iface_addr);
		if (!iface) {
			err("failed to get interface");
			return -1;
		}
		//role = iface->media_info.role & 0x40;
		if ((*((char *)(&iface->media_info) + 6) == 0x40) || (*((char *)(&iface->media_info) + 6) == 0x00)) {
			link_count++;
		} else
			continue;

		//TODO memory allocation here
		tx_link_sub = &tx_link.metrics[i];
		/*fill into local interface mac addr */
		memcpy(tx_link_sub->mac_address, link->connected_iface_addr, ETH_ALEN);
		/*fill into neighbor interface mac addr */
		memcpy(tx_link_sub->mac_address_neighbor, link->neighbor_iface_addr, ETH_ALEN);
		/*fill into interface media type */
		tx_link_sub->intftype = link->tx.iface_type;
		/*fill into bridge flag */
		tx_link_sub->ieee80211_bridgeflg = link->tx.is_80211_bridge;
		/*fill into tx packets error */
		tx_link_sub->packetErrors = link->tx.pkt_err;
		/*fill into total transmitted packets */
		tx_link_sub->transmittedPackets = link->tx.tx_packet;
		/*fill into max throughput capability */
		tx_link_sub->macThroughputCapacity = link->tx.mac_throughput;
		/*fill into link availability field */
		tx_link_sub->linkAvailability = link->tx.link_availability;
		/* fill into phy rate */
		tx_link_sub->phyRate = link->tx.phy_rate;

		debug("Tx link=%d mac(%02x:%02x:%02x:%02x:%02x:%02x) n_mac(%02x:%02x:%02x:%02x:%02x:%02x) intftype=%d, ieee80211_bridgeflg=%d, packetErrors=%d, transmittedPackets=%d, macThroughputCapacity=%d, linkAvailability=%d, phyRate=%d", link_count,
			PRINT_MAC(tx_link_sub->mac_address), PRINT_MAC(tx_link_sub->mac_address_neighbor), tx_link_sub->intftype,
			tx_link_sub->ieee80211_bridgeflg, tx_link_sub->packetErrors, tx_link_sub->transmittedPackets,
			tx_link_sub->macThroughputCapacity, tx_link_sub->linkAvailability, tx_link_sub->phyRate);
		/* Rx link */
		rx_link_sub = &rx_link.metrics[i];
		/*fill into local interface mac addr */
		memcpy(rx_link_sub->mac_address, link->connected_iface_addr, ETH_ALEN);
		/*fill into neighbor interface mac addr */
		memcpy(rx_link_sub->mac_address_neighbor, link->neighbor_iface_addr, ETH_ALEN);
		/*fill into interface media type */
		rx_link_sub->intftype = link->rx.iface_type;
		/*fill into rx packets error */
		rx_link_sub->packetErrors = link->rx.pkt_err;
		/*fill into total received packets */
		rx_link_sub->packetsReceived = link->rx.pkt_received;
		/* fill rssi */
		rx_link_sub->rssi = rssi_to_rcpi(link->rx.rssi);

		debug("Rx link=%d, mac(%02x:%02x:%02x:%02x:%02x:%02x) n_mac(%02x:%02x:%02x:%02x:%02x:%02x) intftype=%d, packetErrors=%d, packetsReceived=%d, rssi=%d",
		link_count, PRINT_MAC(rx_link_sub->mac_address), PRINT_MAC(rx_link_sub->mac_address_neighbor), rx_link_sub->intftype,
		rx_link_sub->packetErrors, rx_link_sub->packetsReceived, rx_link_sub->rssi);
		/* next backhaul between same neighbors */
		i++;
	}
	//TODO correct this based on new structure
	tx_link.link_pair_cnt=link_count;
	rx_link.link_pair_cnt=link_count;
	if (iface) {
		if ((*((char *)(&iface->media_info) + 6) == 0x40)) {
			os_memcpy(sta_tx_link, &tx_link, sizeof(tx_link));
			os_memcpy(sta_rx_link, &rx_link, sizeof(rx_link));
			*is_ap = 0;
		} else if ((*((char *)(&iface->media_info) + 6) == 0x00)) {
			os_memcpy(ap_tx_link, &tx_link, sizeof(tx_link));
			os_memcpy(ap_rx_link, &rx_link, sizeof(rx_link));
			*is_ap = 1;
		}
	}
	return i;
}

u8 is_op_class_2g(u8 op_class) {
	switch(op_class) {
		case 81:
		case 82:
		case 83:
		case 84:
		case 103:
		case 113:
		case 114:
			return TRUE;
		default:
			return FALSE;
		
	}
}
#ifdef CENT_STR
u8 is_op_class_5g(u8 op_class) {
	switch(op_class) {
		case 94:
		case 95:
		case 96:
		case 104:
		case 105:
		case 106:
		case 107:
		case 108:
		case 109:
		case 110:
		case 111:
		case 115:
		case 116:
		case 117:
		case 118:
		case 119:
		case 120:
		case 121:
		case 122:
		case 123:
		case 124:
		case 125:
		case 126:
		case 127:
		case 128:
		case 129:
		case 130:
			return TRUE;
		default:
			return FALSE;
	}
}
#endif

void topo_srv_update_wireless_mode(struct _1905_map_device *dev)
{
	
	struct radio_info_db *radio = NULL;
	struct basic_cap_db *cap;
	
	struct ap_radio_basic_capability *bcap;
	SLIST_FOREACH(radio, &dev->first_radio, next_radio){
		radio->wireless_mode =0;
		bcap = &radio->radio_capability.basic_caps;
		SLIST_FOREACH(cap, &bcap->bcap_head,basic_cap_entry) {
			if(is_op_class_2g(cap->op_class)) {
				radio->wireless_mode |= (WMODE_B | WMODE_G);
				if(radio->radio_capability.ht_cap.valid == 1)
					radio->wireless_mode |= WMODE_GN;
				if(radio->radio_capability.he_cap.valid == 1)
					radio->wireless_mode |= WMODE_AX_24G;
			} else {
				radio->wireless_mode |= (WMODE_A);
				if(radio->radio_capability.ht_cap.valid == 1)
					radio->wireless_mode |= WMODE_AN;
				if(radio->radio_capability.vht_cap.valid == 1)
					radio->wireless_mode |= (WMODE_AC | WMODE_AN);
				if(radio->radio_capability.he_cap.valid == 1)
					radio->wireless_mode |= WMODE_AX_5G;
			}
		}
	}
}

/**
* @brief Fn to parse ap caps report
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
*
* @return 0 if success else -1
*/
int topo_srv_prase_ap_cap_report(struct own_1905_device * ctx, unsigned char *buf)
{
	int length = 0;
	unsigned char *temp_buf;
	struct _1905_map_device *dev;
	struct ap_radio_basic_cap *bcap;
	struct ap_capability ap_cap;
	struct ap_ht_capability ht_cap;
	struct ap_vht_capability vht_cap;
	struct ap_he_capability he_cap;
#ifdef MAP_R2
	struct channel_scan_capab ch_scan_cap;
#endif

	temp_buf = buf;
	dev = topo_srv_get_1905_device(ctx, temp_buf);
	if (!dev) {
		err("failed to get 1905 device");
		return -1;
	}
	temp_buf += ETH_ALEN;

	while (1) {
		if (*temp_buf == AP_CAPABILITY_TYPE) {
			length = parse_ap_cap_tlv(temp_buf, dev, &ap_cap);
			if (length < 0) {
				err("error parse ap cap tlv");
				return -1;
			}
			topo_srv_update_1905_ap_cap(dev, &ap_cap);
			temp_buf += length;
		} else if (*temp_buf == AP_RADIO_BASIC_CAPABILITY_TYPE) {
			unsigned char bcap_buf[512];
			bcap = (struct ap_radio_basic_cap *)bcap_buf;
			length = parse_basic_radio_cap_tlv(temp_buf, dev, bcap);
			if (length < 0) {
				err("error basic ap cap tlv");
				return -1;
			}
			topo_srv_update_radio_basic_cap(ctx, dev , bcap);
			temp_buf += length;
		} else if (*temp_buf == AP_HT_CAPABILITY_TYPE) {
			length = parse_ap_ht_cap_tlv(temp_buf, &ht_cap);
			if (length < 0) {
				err("error ap ht cap tlv");
				return -1;
			}
			topo_srv_update_ap_ht_cap(ctx, dev, &ht_cap);
			temp_buf += length;
		} else if (*temp_buf == AP_VHT_CAPABILITY_TYPE) {
			length = parse_ap_vht_cap_tlv(temp_buf, &vht_cap);
			if (length < 0) {
				err("error ap vht cap tlv");
				return -1;
			}
			topo_srv_update_ap_vht_cap(ctx, dev, &vht_cap);
			temp_buf += length;
		} else if (*temp_buf == AP_HE_CAPABILITY_TYPE) {
			length = parse_ap_he_cap_tlv(temp_buf, &he_cap);
			if (length < 0) {
				err("error ap he cap tlv");
				return -1;
			}
			topo_srv_update_ap_he_cap(ctx, dev, &he_cap);
			temp_buf += length;
		}
#ifdef MAP_R2
		else if (*temp_buf == CHANNEL_SCAN_CAPABILITY_TYPE) {
			length = parse_ap_scan_cap_tlv(temp_buf,dev, &ch_scan_cap);
			if (length < 0) {
				err("error channel scan capability TLV");
				return -1;
			}
			topo_srv_update_dev_ch_scan_cap(ctx, dev, &ch_scan_cap);
			temp_buf += length;
		}
		/* One CAC Capabilities TLV */
		else if (*temp_buf == CAC_CAPABILITIES_TYPE) {
			length = parse_ap_cac_cap_tlv(temp_buf,dev);
			if (length < 0) {
				err("error channel scan capability TLV");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == METRIC_COLLECTION_INTERVAL_TYPE) {
			length = parse_metric_collection_intv_tlv(temp_buf, dev);
			if (length < 0) {
				err("error channel scan capability TLV");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == R2_AP_CAPABILITY_TYPE) {
			length = parse_r2_cap_tlv(temp_buf, dev);
			dev->map_version = DEV_TYPE_R2;
			temp_buf += length;
		}
#endif
		else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
	/*check integrity */
	topo_srv_update_wireless_mode(dev);

	excess_debug("exit");

	return 0;

}
/**
* @brief Fn to parse combined infra msg
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
*
* @return 0 if success else -1
*/
int topo_srv_parse_combined_infra_msg(struct own_1905_device * ctx, unsigned char *buf)
{
	int length = 0;
	unsigned char *temp_buf;
	unsigned char integrity = 0;
	struct _1905_map_device *dev = NULL;

	temp_buf = buf;
	while (1) {
		if (*temp_buf == AP_METRICS_TYPE) {
			integrity |= 0x1;
			//err("parse combined infra metric");
			//mapd_hexdump(MSG_ERROR, "parse combined infra metric", temp_buf, 44);
			dev = topo_srv_get_1905_device(ctx, temp_buf + 3);
			if(dev == NULL)
				info("error dev not found");
			length = parse_ap_metrics_tlv(ctx, dev, temp_buf);
			if (length < 0) {
				err("error parse ap metrics tlv");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == TRANSMITTER_LINK_METRIC_TYPE) {
			integrity |= 0x1;
			dev = topo_srv_get_1905_device(ctx, temp_buf + 3);
			if (!dev) {
				err("failed to get tx metric dev");
				mapd_hexdump(MSG_ERROR, "topo_srv_parse_combined_infra_msg", temp_buf, 44);
				return -1;
			}
			length = parse_tx_link_metrics_tlv(temp_buf, dev);
			if (length < 0) {
				err("error tx link metrics tlv");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == RECEIVER_LINK_METRIC_TYPE) {
			integrity |= 0x1;
			dev = topo_srv_get_1905_device(ctx, temp_buf + 3);
			if (!dev) {
				err("failed to get rx metric dev");
				mapd_hexdump(MSG_ERROR, "topo_srv_parse_combined_infra_msg", temp_buf, 38);
				return -1;
			}
			length = parse_rx_link_metrics_tlv(temp_buf, dev);
			if (length < 0) {
				err("error tx link metrics tlv");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
	/*check integrity */
	if (integrity != 0x1) {
		err("no backhaul steering response tlv");
		return -1;
	}
	excess_debug("exit");

	return 0;
}

/**
* @brief Fn to delete current metrics policy
*
* @param mpolicy policy to be delelted
*
* @return -1 if error else 0
*/
int delete_exist_metrics_policy(struct metrics_policy *mpolicy)
{
	struct metric_policy_db *policy = NULL, *policy_tmp = NULL;

	excess_debug("delete_steering_policy");

	policy = SLIST_FIRST(&mpolicy->policy_head);
	while (policy != NULL) {
		excess_debug("metric_policy_db identifier(%02x:%02x:%02x:%02x:%02x:%02x)",
			     PRINT_MAC(policy->identifier));
		excess_debug("rssi_thres=%d, hysteresis_margin=%d, ch_util_thres=%d"
			     "rssi_thres=%d, hysteresis_margin=%d",
			     policy->rssi_thres, policy->hysteresis_margin, policy->ch_util_thres,
			     policy->sta_stats_inclusion, policy->sta_metrics_inclusion);
		policy_tmp = SLIST_NEXT(policy, policy_entry);
		free(policy);
		policy = policy_tmp;
	}
	SLIST_INIT(&mpolicy->policy_head);
	mpolicy->report_interval = 0;
	mpolicy->radio_num = 0;

	return 0;
}

/**
* @brief 
*
* @param ctx
* @param msg_buf
* @param len
*
* @return 
*/
int topo_srv_handle_assoc_link_metrics_rsp(struct own_1905_device *ctx, unsigned char *buf, int len)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, buf);
	
	if(!dev){
		err("dev is NULL");
		return -1;
	}
	info("got ASSOC_LINK_METRICS_RESPONSE from (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(buf));
	/*parse assoc link metrics response message*/
	if (0 > parse_associated_sta_link_metrics_rsp_msg(ctx, dev, buf + ETH_ALEN)) {
		debug("error! parse assoc sta link metrics response message");
	}

	return 0;
}

/**
* @brief Fn to handle assoc link metrics resp
*
* @param ctx own 1905 device ctx
* @param msg_buf msg buffer
* @param len msg len
*
* @return 0
*/
int topo_srv_handle_unassoc_link_metrics_rsp(struct own_1905_device *ctx, unsigned char *msg_buf, int len)
{
	ap_est_handle_unassoc_sta_link_metric_rsp(ctx->back_ptr,(struct unassoc_link_metric_tlv_rsp *) msg_buf, len);
	return 0;
}


#ifdef CENT_STR
int parse_beacon_metrics_rsp_tlv(struct own_1905_device * ctx, struct _1905_map_device *dev, unsigned char *buf)
{
	unsigned char *temp_buf;
	unsigned short length = 0,rpt_len = 0;
	unsigned char *bcn_rpt, bcn_rpt_num;
	unsigned char isSuccess = 1, islastreport= 0;
	unsigned char sta_mac[ETH_ALEN] = {0};
	PEID_STRUCT eid_ptr = NULL;
	prrm_beacon_rep_info pBcnRep = NULL;
	int bcnrpt_cnt = 0;

	temp_buf = buf;

	if((*temp_buf) == BEACON_METRICS_RESPONSE_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}
	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	//Copy sta mac address
	os_memcpy(sta_mac, temp_buf, ETH_ALEN);	
	temp_buf += ETH_ALEN;		

	//skip resvd field
	temp_buf += 1;
	
	bcn_rpt_num = *temp_buf;
	temp_buf++;

	if(length > 0)
	rpt_len = length - 8;
	else
		rpt_len = 0;


	bcn_rpt = (unsigned char *)temp_buf;	


	if(rpt_len > 0) {
		eid_ptr = (PEID_STRUCT)bcn_rpt;
		while (((u8 *)eid_ptr + eid_ptr->Len + 1) < ((u8 *)bcn_rpt + rpt_len)){
			switch (eid_ptr->Eid) {
				case IE_MEASUREMENT_REPORT:
					/*Skip measurement report token and type*/
					if(eid_ptr->Len - 3 > 0){
						pBcnRep = (prrm_beacon_rep_info)((u8*)eid_ptr->Octet + 3);
						err("Chan No:%d,rcpi:%d,bssid:"MACSTR,pBcnRep->ch_number,pBcnRep->rcpi,MAC2STR(pBcnRep->bssid));
						bcnrpt_cnt++;

						if(bcnrpt_cnt == bcn_rpt_num)
							islastreport = 1;

						ap_est_handle_11k_report(ctx->back_ptr, sta_mac, isSuccess,
							pBcnRep->bssid, pBcnRep->ch_number,
							pBcnRep->rcpi, islastreport);
					} else {
						isSuccess = 0;
						ap_est_handle_11k_report(ctx->back_ptr, sta_mac, isSuccess,
									NULL, 0, 0, 0);
					}
					break;
				default:
					break;

			}
			eid_ptr = (PEID_STRUCT)((u8 *)eid_ptr + 2 + eid_ptr->Len);
		}
	
	}


	return (length+3);
}

int parse_beacon_metrics_rsp_msg(struct own_1905_device *ctx, struct _1905_map_device *dev, unsigned char *buf)
{
        int length = 0;
        unsigned char *temp_buf;

        temp_buf = buf;
        info(" ");
        while (1) {
                if (*temp_buf == BEACON_METRICS_RESPONSE_TYPE) {
                        length = parse_beacon_metrics_rsp_tlv(ctx, dev, temp_buf);
                        if(length < 0) {
                                err("error associated sta link metrics query tlv");
                                return -1;
                        }
                        temp_buf += length;
                } else if (*temp_buf == END_OF_TLV_TYPE) {
                        break;
                } else {
                        length = get_cmdu_tlv_length(temp_buf);
                        temp_buf += length;
                }
        }

        return 0;
}


#endif


/**
* @brief Fn to handle beacon metrics resp event
*
* @param ctx own 1905 device ctx
* @param msg_buf msg buffer
* @param len msg length
*
* @return 0
*/
int topo_srv_handle_beacon_metrics_rsp_event(struct own_1905_device *ctx, unsigned char *msg_buf, int len)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, msg_buf);
	
	info("got BEACON METRICS RESPONSE from (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(msg_buf));
	/*parse beacon metrics response message*/
	if (0 > parse_beacon_metrics_rsp_msg (ctx, dev, msg_buf + ETH_ALEN)) {
		debug("error! parse beacon metrics response message");
	}

	return 0;
}

/**
* @brief Fn to delete existing unlink metrics info
*
* @param unlink_metrics unlink metrics to be deleted
*
* @return -1 if error else 0
*/
int delete_exist_unlink_metrics_rsp(struct unlink_metrics_info *unlink_metrics)
{
	struct unlink_metrics_db *metrics = NULL, *metrics_tmp = NULL;

	excess_debug("delete_exist_unlink_metrics_info");

	unlink_metrics->oper_class = 0;
	unlink_metrics->sta_num = 0;
	metrics = SLIST_FIRST(&unlink_metrics->unlink_metrics_head);
	while (metrics != NULL) {
		metrics_tmp = SLIST_NEXT(metrics, unlink_metrics_entry);
		excess_debug("sta mac=%02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(metrics->mac));
		excess_debug("ch=%d, time_delta=%d, uplink_rssi=%d",
			     metrics->ch, metrics->time_delta, metrics->uplink_rssi);
		free(metrics);
		metrics = metrics_tmp;
	}
	SLIST_INIT(&unlink_metrics->unlink_metrics_head);

	return 0;
}

/**
* @brief Fn to update unlink metrics resp
*
* @param unlink_metrics_ctx unlink mterics context
* @param unlink_metrics unlink metrics response
*
* @return -1 if error else 0
*/
int update_unlink_metrics_rsp(struct unlink_metrics_info *unlink_metrics_ctx, struct unlink_metrics_rsp *unlink_metrics)
{
	struct unlink_metrics_db *metrics = NULL;
	struct unlink_rsp_sta *info = NULL;
	int i = 0;

	excess_debug("enter");

	delete_exist_unlink_metrics_rsp(unlink_metrics_ctx);

	unlink_metrics_ctx->oper_class = unlink_metrics->oper_class;
	unlink_metrics_ctx->sta_num = unlink_metrics->sta_num;
	excess_debug("oper_class=%d, sta_num=%d",
		     unlink_metrics_ctx->oper_class, unlink_metrics_ctx->sta_num);

	SLIST_INIT(&unlink_metrics_ctx->unlink_metrics_head);

	for (i = 0; i < unlink_metrics->sta_num; i++) {
		metrics = (struct unlink_metrics_db *)
		    os_zalloc(sizeof(struct unlink_metrics_db));
		if (!metrics) {
			excess_debug("alloc struct unlink_metrics_db fail");
			return -1;
		}
		info = &unlink_metrics->info[i];
		memcpy(metrics->mac, info->mac, ETH_ALEN);
		metrics->ch = info->ch;
		metrics->time_delta = info->time_delta;
		metrics->uplink_rssi = info->uplink_rssi;
		info->uplink_rssi = rssi_to_rcpi(info->uplink_rssi);
		SLIST_INSERT_HEAD(&unlink_metrics_ctx->unlink_metrics_head, metrics, unlink_metrics_entry);
		excess_debug("sta mac=%02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(metrics->mac));
		excess_debug("ch=%d, time_delta=%d, uplink_rssi=%d",
			     metrics->ch, metrics->time_delta, metrics->uplink_rssi);
	}
	return 0;

}

/**
* @brief Fn to update one station link metrics
*
* @param ctx own 1905 device ctx
* @param metrics link_metrics for sta
*
* @return -1 if error else 0
*/
int update_one_sta_link_metrics_info(struct own_1905_device *ctx, struct link_metrics *metrics)
{
	struct metrics_db *metrics_sta = &ctx->metric_entry.assoc_sta_link_metrics;

	excess_debug("enter");

	memset(metrics_sta, 0, sizeof(struct metrics_db));
	memcpy(metrics_sta->mac, metrics->mac, ETH_ALEN);
	memcpy(metrics_sta->bssid, metrics->bssid, ETH_ALEN);
	metrics_sta->time_delta = metrics->time_delta;
	metrics_sta->erate_downlink = metrics->erate_downlink;
	metrics_sta->erate_uplink = metrics->erate_uplink;
	metrics_sta->rssi_uplink = metrics->rssi_uplink;

	excess_debug("insert struct link_metrics_db");
	excess_debug("sta mac(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(metrics_sta->mac));
	excess_debug("bssid(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(metrics_sta->bssid));
	excess_debug
	    ("time_delta=%d, erate_downlink=%d erate_uplink=%d rssi_uplink=%d",
	     metrics_sta->time_delta, metrics_sta->erate_downlink, metrics_sta->erate_uplink, metrics_sta->rssi_uplink);

	return 0;

}
#ifdef MAP_R2
int update_one_sta_link_ext_metrics_info(struct own_1905_device *ctx, struct ext_link_metrics *metrics)
{
	struct metrics_db *metrics_sta = &ctx->metric_entry.assoc_sta_link_metrics;

	debug("enter");

	memset(metrics_sta, 0, sizeof(struct metrics_db));
	memcpy(metrics_sta->mac, metrics->mac, ETH_ALEN);
	memcpy(metrics_sta->bssid, metrics->bssid, ETH_ALEN);

	metrics_sta->sta_ext_info.last_data_dl_rate = metrics->last_data_dl_rate;
	metrics_sta->sta_ext_info.last_data_ul_rate = metrics->last_data_ul_rate;
	metrics_sta->sta_ext_info.utilization_rx = metrics->utilization_rx;
	metrics_sta->sta_ext_info.utilization_tx = metrics->utilization_tx;

	debug("insert struct link_metrics_db");
	debug("sta mac(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(metrics_sta->mac));
	debug("bssid(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(metrics_sta->bssid));
	debug("dl rate: %d, ul rate: %d", metrics_sta->sta_ext_info.last_data_dl_rate, metrics_sta->sta_ext_info.last_data_ul_rate);
	debug("rx rate: %d, tx rate: %d", metrics_sta->sta_ext_info.utilization_rx, metrics_sta->sta_ext_info.utilization_tx);

	return 0;

}

#endif

/**
* @brief Fn to add new link metrics info
*
* @param ctx own 1905 device ctx
* @param metrics_info sta_link_metrics to be added
*
* @return 0 if success else -1
*/
int topo_srv_create_new_sta(struct own_1905_device *ctx, struct link_metrics *cinfo)
{
	struct _1905_map_device *device = topo_srv_get_1905_device(ctx, NULL);
	struct connected_clients *client = NULL;
	struct associated_clients *assoc_client = NULL;
	struct bss_info_db *bss = NULL;
	uint32_t client_id;
	err("create client");
	// TODO move connected clients to bss basis
	client = (struct connected_clients *)os_zalloc(sizeof(struct connected_clients));
	assoc_client = (struct associated_clients *)os_zalloc(sizeof(struct associated_clients));
	if (!client || !assoc_client) {
		err("mem assoc failed");
		if (client)
			os_free(client);
		if (assoc_client)
			os_free(assoc_client);
		return -1;
	}
	os_memcpy(client->client_addr, cinfo->mac, ETH_ALEN);
	os_memcpy(client->_1905_iface_addr, cinfo->bssid, ETH_ALEN);
	os_memcpy(assoc_client->client_addr, cinfo->mac, ETH_ALEN);
	client->is_APCLI = cinfo->is_APCLI;
	assoc_client->is_APCLI = cinfo->is_APCLI;
	
	bss = topo_srv_get_bss_by_bssid(ctx, device, cinfo->bssid);
	assoc_client->bss = bss;
	if(assoc_client->bss != NULL) {
		os_memcpy(assoc_client->bss->bssid, cinfo->bssid, ETH_ALEN);
		os_memcpy(client->bss_addr, cinfo->bssid, ETH_ALEN);
	} else {
		err("There is no BSS");
		os_free(client);
		os_free(assoc_client);
		return -1;
	}

	assoc_client->last_assoc_time = 0;

	err("insert sta(%02x:%02x:%02x:%02x:%02x:%02x) in bss(%02x:%02x:%02x:%02x:%02x:%02x)",
		PRINT_MAC(cinfo->mac), PRINT_MAC(cinfo->bssid));
	if (!ctx->dual_bh_en)
		remove_duplicate_cli_single_bh(device, ctx, cinfo->mac);
	SLIST_INSERT_HEAD(&(device->wlan_clients), client, next_client);
	SLIST_INSERT_HEAD(&(device->assoc_clients), assoc_client, next_client);
	duplicate_sta_check_for_1905_device(ctx, device);

	if (!cinfo->is_APCLI) {
		client_id = client_mon_handle_local_join(ctx->back_ptr, cinfo->mac,
						cinfo->bssid, 0, 0, 0, 0, 0, 0, 0, NULL);
		if (client_id == (uint32_t)-1) {
			mapd_printf(MSG_ERROR, "new assoc seen-->But no more room");
			return -1;
		}
		mapd_printf(MSG_WARNING, "new assoc seen-->assigned_id=%d",
					client_id);
	}
	return 0;
}
int insert_new_link_metrics_info(struct _1905_context *_1905_ctrl,struct own_1905_device *ctx, struct sta_link_metrics *metrics_info)
{
#ifdef CENT_STR
	struct _1905_map_device *own_dev = topo_srv_get_1905_device(ctx,NULL);
	int found_sta = 0;
	struct link_metrics *info_to_send = NULL;
#endif

	if (is_1905_present()) {
		struct link_metrics *info = NULL;
		struct associated_clients *metrics_ctx = NULL;
		int i = 0;

		excess_debug("enter");
#ifdef CENT_STR
		info_to_send = os_zalloc(sizeof(struct link_metrics) * metrics_info->sta_cnt);
		if (info_to_send == NULL)
			return -1;
#endif
		for (i = 0; i < metrics_info->sta_cnt; i++) {
		info = &metrics_info->info[i];
		metrics_ctx = topo_srv_get_associate_client(ctx, NULL, info->mac);
		if (!metrics_ctx) {
			err("No associated client found");
			if (info->mac != NULL) {
				topo_srv_create_new_sta(ctx, info);
				err("Creating new STA info");
			}
			continue;
		}
		metrics_ctx->time_delta = info->time_delta;
		metrics_ctx->erate_downlink = info->erate_downlink;
		metrics_ctx->erate_uplink = info->erate_uplink;
		metrics_ctx->rssi_uplink = info->rssi_uplink;
		info->rssi_uplink = rssi_to_rcpi((signed char)info->rssi_uplink);

		debug("insert struct metrics_db");
		debug("time_delta=%d, erate_downlink=%d erate_uplink=%d rssi_uplink=%d",
		     metrics_ctx->time_delta, metrics_ctx->erate_downlink,
			     metrics_ctx->erate_uplink, metrics_ctx->rssi_uplink);

#ifdef CENT_STR
		if(ctx->cent_str_en && own_dev->device_role == DEVICE_ROLE_CONTROLLER && !metrics_ctx->is_bh_link){
			if(link_metrics_mon_rcpi_at_controller(metrics_ctx, ctx)) {
					/*Get the client db entry for this sta.*/
					struct client *cli = client_db_get_client_from_sta_mac(ctx->back_ptr, metrics_ctx->client_addr);

			

					if(cli && !metrics_ctx->is_bh_link) {
					cli->dl_phy_rate = metrics_ctx->erate_downlink;
					cent_str_select_on_demand_str_method(ctx,cli);
					}

			}			

		} else {	
#endif				

			if(send_link_metrics_selective(metrics_ctx,ctx)) {
#ifdef CENT_STR
				if (ctx->cent_str_en) {
					os_memcpy(&info_to_send[found_sta], info, sizeof(struct link_metrics));
					found_sta++;
					err("STA link rsp afterwards, cent steering");
				} else {
#endif
#ifdef MAP_R2
					map_1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(_1905_ctrl, 1,
							info, 0, NULL, NULL, 0);
#else
					map_1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(_1905_ctrl, 1, info, NULL, 0);
#endif
#ifdef CENT_STR
			}
#endif
		}
		}
		}
#ifdef CENT_STR
		if (found_sta && ctx->cent_str_en) {
			err("Sending STA link metrics rsp");
#ifdef MAP_R2
			map_1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(_1905_ctrl, found_sta,
			info_to_send, 0, NULL, NULL, 0);
#else
			map_1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(_1905_ctrl, found_sta, info_to_send, NULL, 0);
#endif
		}
		os_free(info_to_send);
#endif
	}
	return 0;
}

#ifdef MAP_R2
int insert_new_ext_link_metrics_info(struct own_1905_device *ctx, struct ext_sta_link_metrics *metrics_info)
{
	if (is_1905_present()) {
		//struct link_metrics_db *link_metrics_ctx = NULL;
		struct associated_clients *metrics_ctx = NULL;
		struct ext_link_metrics *info = NULL;
		int i = 0;

		debug("enter");

		for (i = 0; i < metrics_info->sta_cnt; i++) {
			info = &metrics_info->info[i];
			metrics_ctx = topo_srv_get_associate_client(ctx, NULL, info->mac);
			if (!metrics_ctx) {
				err("No associated client found");
				return -1;
			}
			metrics_ctx->sta_ext_info.last_data_dl_rate = info->last_data_dl_rate;
			metrics_ctx->sta_ext_info.last_data_ul_rate = info->last_data_ul_rate;
			metrics_ctx->sta_ext_info.utilization_rx = info->utilization_rx;
			metrics_ctx->sta_ext_info.utilization_tx = info->utilization_tx;
			debug("insert struct link_metrics_db");
			debug("dl rate: %d, ul rate: %d", metrics_ctx->sta_ext_info.last_data_dl_rate, metrics_ctx->sta_ext_info.last_data_ul_rate);
			debug("rx rate: %d, tx rate: %d", metrics_ctx->sta_ext_info.utilization_rx, metrics_ctx->sta_ext_info.utilization_tx);

		}
	}
	return 0;
}
#endif


/**
* @brief Fn to delete a ap metrics info
*
* @param ctx own 1905 device ctx
* @param bssid bssid if ap
*
* @return -1 if error else 0
*/
int delete_exist_ap_metrics_info(struct own_1905_device *ctx, unsigned char *bssid)
{
	struct esp_db *esp = NULL, *esp_tmp = NULL;
	struct bss_info_db *bss = topo_srv_get_bss_by_bssid(ctx, NULL, bssid);

	debug("delete_exist_ap_metrics_info");
	if (!bss) {
		err("bss not found");
		return -1;
	}
	debug("esp_cnt=%d", bss->esp_cnt);
	if (!SLIST_EMPTY(&(bss->esp_head))) {
		esp = SLIST_FIRST(&(bss->esp_head));
		while (esp) {
			debug("delete_exist struct esp_db");
			debug("ac=%d, format=%d ba_win_size=%d", esp->ac, esp->format, esp->ba_win_size);
			debug("e_air_time_fraction=%d, ppdu_dur_target=%d",
				esp->e_air_time_fraction, esp->ppdu_dur_target);

			esp_tmp = SLIST_NEXT(esp, esp_entry);
			SLIST_REMOVE(&(bss->esp_head), esp, esp_db, esp_entry);
			free(esp);
			esp = esp_tmp;
		}
	}
	debug("delete_exist struct mrsp_db");
	debug("bssid(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bss->bssid));
	debug("ch_uti=%d, assoc_sta_cnt=%d", bss->ch_util, bss->assoc_sta_cnt);

	return 0;
}

/**
* @brief Fn to delete a traffic stats info
*
* @param ctx own 1905 device ctx
* @param identifier radio identifier
*
* @return 0 if success else -1
*/
int delete_exist_traffic_stats_info(struct own_1905_device *ctx, unsigned char *identifier)
{
	struct traffic_stats_db *traffic_stats = NULL;
	struct stats_db *stats = NULL, *stats_tmp = NULL;

	debug("delete_exist_traffic_stats_info");

	SLIST_FOREACH(traffic_stats, &ctx->metric_entry.traffic_stats_head, traffic_stats_entry) {
		if (!memcmp(traffic_stats->identifier, identifier, ETH_ALEN)) {
			excess_debug("sta_cnt=%d", traffic_stats->sta_cnt);

			stats = SLIST_FIRST(&traffic_stats->stats_head);
			while (stats) {
				excess_debug("delete_exist struct stats_db");
				excess_debug("sta mac(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(stats->mac));
				excess_debug("bytes_sent=%d, bytes_received=%d"
					     "packets_sent=%d, packets_received=%d"
					     "tx_packets_errors=%d, rx_packets_errors=%d"
					     "retransmission_count=%d",
					     stats->bytes_sent,
					     stats->bytes_received,
					     stats->packets_sent,
					     stats->packets_received,
					     stats->tx_packets_errors,
					     stats->rx_packets_errors, stats->retransmission_count);

				stats_tmp = SLIST_NEXT(stats, stats_entry);
				SLIST_REMOVE(&traffic_stats->stats_head, stats, stats_db, stats_entry);
				free(stats);
				stats = stats_tmp;
			}

			excess_debug("delete_exist struct traffic_stats_db");
			excess_debug
			    ("identifier(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(traffic_stats->identifier));
			SLIST_REMOVE(&ctx->metric_entry.traffic_stats_head,
				     traffic_stats, traffic_stats_db, traffic_stats_entry);
			free(traffic_stats);
			break;
		}
	}

	return 0;
}

/**
* @brief Fn to add new traffic stats
*
* @param ctx own 1905 device ctx
* @param traffic_stats sta_traffic_stats to be added
*
* @return 0 if success else -1
*/
int insert_new_traffic_stats_info(struct own_1905_device *ctx, struct sta_traffic_stats *traffic_stats)
{
	struct traffic_stats_db *tstats = NULL;
	struct stats_db *stats = NULL;
	struct stat_info *info = NULL;
	int i = 0;

	excess_debug("enter");
	tstats = (struct traffic_stats_db *)os_zalloc(sizeof(struct traffic_stats_db));
	if (!tstats) {
		err("alloc struct traffic_stats_db fail");
		return -1;
	}
	memset(tstats, 0, sizeof(struct traffic_stats_db));
	memcpy(tstats->identifier, traffic_stats->identifier, ETH_ALEN);
	tstats->sta_cnt = traffic_stats->sta_cnt;
	SLIST_INIT(&tstats->stats_head);
	SLIST_INSERT_HEAD(&(ctx->metric_entry.traffic_stats_head), tstats, traffic_stats_entry);

	excess_debug("insert struct traffic_stats_db");
	excess_debug("identifier(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(tstats->identifier));
	excess_debug("sta_cnt=%d", tstats->sta_cnt);

	for (i = 0; i < traffic_stats->sta_cnt; i++) {
		info = &traffic_stats->stats[i];
		stats = (struct stats_db *)os_zalloc(sizeof(struct stats_db));
		if (!stats) {
			err("alloc struct stats_db fail");
			return -1;
		}
		memcpy(stats->mac, info->mac, ETH_ALEN);
		stats->bytes_sent = info->bytes_sent;
		stats->bytes_received = info->bytes_received;
		stats->packets_sent = info->packets_sent;
		stats->packets_received = info->packets_received;
		stats->tx_packets_errors = info->tx_packets_errors;
		stats->rx_packets_errors = info->rx_packets_errors;
		stats->retransmission_count = info->retransmission_count;
		SLIST_INSERT_HEAD(&tstats->stats_head, stats, stats_entry);

		excess_debug("insert struct stats_db");
		excess_debug("sta mac(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(stats->mac));
		excess_debug("bytes_sent=%d, bytes_received=%d packets_sent=%d"
			     "packets_received=%d, tx_packets_errors=%d rx_packets_errors=%d"
			     "retransmission_count=%d",
			     stats->bytes_sent, stats->bytes_received,
			     stats->packets_sent, stats->packets_received,
			     stats->tx_packets_errors, stats->rx_packets_errors, stats->retransmission_count);
	}

	return 0;

}

/**
* @brief Fn to parse link metrics response
*
* @param dev _1905_map_device pointer
* @param buf msg buffer
*
* @return -1 if error else 0
*/
int parse_link_metrics_response_message(struct _1905_map_device *dev, unsigned char *buf)
{
	int length = 0;
	unsigned char *temp_buf;

	temp_buf = buf;

	while (1) {
		if (*temp_buf == TRANSMITTER_LINK_METRIC_TYPE) {
			length = parse_tx_link_metrics_tlv(temp_buf, dev);
			if (length < 0) {
				err("error tx link metrics tlv");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == RECEIVER_LINK_METRIC_TYPE) {
			length = parse_rx_link_metrics_tlv(temp_buf, dev);
			if (length < 0) {
				err("error rx link metrics tlv");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}

	return 0;
}

/* handle link metric report msgs */
int topo_srv_update_link_metric_report(struct _1905_map_device *_1905_device, char *msg)
{
	return 0;
}

/* handle ap metric msgs */
int topo_srv_update_ap_metric(struct _1905_map_device *_1905_device, char *msg)
{
	return 0;
}

/**
* @brief Fn to parse unass sta link query msg
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
*
* @return -1 if error else 0
*/
int parse_unassociated_sta_link_metrics_query_message(struct own_1905_device
						      *ctx, unsigned char *buf, struct unlink_metrics_query *unlink_query)
{
	int length = 0;
	unsigned char *temp_buf;
	unsigned char integrity = 0;

	temp_buf = buf;

	while (1) {
		if (*temp_buf == UNASSOC_STA_LINK_METRICS_QUERY_TYPE) {
			integrity |= 0x1;
			length =
			    parse_unassociated_sta_link_metrics_query_tlv(temp_buf, unlink_query);
			if (length < 0) {
				err("error unassociated sta link metrics query tlv");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
	/*check integrity */
	if (integrity != 0x1) {
		err("no unassicated sta link metrics query tlv");
		return -1;
	}
	excess_debug("exit");

	return 0;
}

/**
* @brief Fn to handle usassoc link metrics query msg from 1905
*
* @param ctx own 1905 device
* @param buf msg buffer
*
* @return -1 if error else 0
*/
int topo_srv_handle_unassoc_sta_link_metrics_query(struct own_1905_device *ctx, unsigned char *buf)
{
	int len = 0;
	struct mapd_global *global = (struct mapd_global *)ctx->back_ptr;
	u8 unlink_query[50];
	struct unlink_metrics_query *p_unlink_query = (struct unlink_metrics_query *)unlink_query;
	debug("got UNASSOC_STA_LINK_METRICS_QUERY");
	mapd_hexdump(MSG_DEBUG, "UNASSOC_STA_LINK_METRICS_QUERY", (buf - ETH_ALEN), len);

	/*parse AP_LINK_METRICS_QUERY msg */
	/*TODO hanlde unlink Query Properly in case of multiple STA Query
		Handle 7621 Stack corruption issue due to the unlink_query present in ctx*/
	if (0 > parse_unassociated_sta_link_metrics_query_message(ctx, buf, p_unlink_query)) {
		err("error! no need to response this unassociated sta link metrics query message");
		return -1;
	}

	/*query ap metrics response info from wapp */
	len = sizeof(struct unlink_metrics_query) + (p_unlink_query->sta_num)* ETH_ALEN;
	if(global->params.Certification){
	if (0 >
	    map_get_info_from_wapp(ctx,
				   WAPP_USER_GET_UNASSOC_STA_LINK_METRICS,
				   WAPP_UNASSOC_STA_LINK_METRICS, NULL, NULL,
				   (void *)p_unlink_query, len)) {
		err("error! wapp_get_unassoc_sta_link_metrics");
		return -1;
		}
	} else {
		if (0 >
		    map_get_info_from_wapp(ctx,
					   WAPP_USER_SET_AIR_MONITOR_REQUEST,
					   WAPP_AIR_MONITOR_REPORT, NULL, NULL,
					   (void *)unlink_query, len)) {
			err("error! wapp_get_unassoc_sta_link_metrics");
			return -1;
		}
	}
	//os_free(ctx->metric_entry.unlink_query);

	//ctx->metric_entry.unlink_query = NULL;
	return 0;
}

/**
* @brief Fn to parse beacon metrics query msg
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
*
* @return -1 if error else 0
*/
int parse_beacon_metrics_query_message(struct own_1905_device *ctx, unsigned char *buf)
{
	int length = 0;
	unsigned char *temp_buf;
	unsigned char integrity = 0;

	temp_buf = buf;

	while (1) {
		if (*temp_buf == BEACON_METRICS_QUERY_TYPE) {
			integrity |= 0x1;
			length = parse_beacon_metrics_query_tlv(temp_buf, &ctx->metric_entry.bcn_query);
			if (length < 0) {
				err("error beacon metrics query tlv");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
	/*check integrity */
	if (integrity != 0x1) {
		err("no beacon metrics query tlv");
		return -1;
	}
	excess_debug("exit");

	return 0;
}

/**
* @brief Fn to handle beacon metrics query from 1905 and send it to wapp for query
*
* @param ctx own 1905 device ctx
* @param temp_buf msg buffer
*
* @return -1 if error else 0
*/
int topo_srv_handle_beacon_metrics_query(struct own_1905_device *ctx, unsigned char *temp_buf)
{
	int datalen = 0, status;
	unsigned char assoc_bssid[ETH_ALEN] = {0};

	excess_debug("got BEACON_METRICS_QUERY");

	/*parse AP_LINK_METRICS_QUERY msg */
	if (0 > parse_beacon_metrics_query_message(ctx, temp_buf)) {
		err("error! no need to response this beacon metrics query message");
		return -1;
	}

	status = topo_srv_get_bssid_of_sta(ctx, ctx->metric_entry.bcn_query, assoc_bssid);

	/*query ap metrics response info from wapp */
	datalen = sizeof(struct beacon_metrics_query) +
	    ctx->metric_entry.bcn_query->ap_ch_rpt_num * sizeof(struct ap_chn_rpt);

	if (status == 0)
		map_get_info_from_wapp(ctx, WAPP_USER_SET_BEACON_METRICS_QRY,
				   0, assoc_bssid, NULL, (void *)ctx->metric_entry.bcn_query, datalen);
	else
		map_get_info_from_wapp(ctx, WAPP_USER_SET_BEACON_METRICS_QRY,
				   0, NULL, NULL, (void *)ctx->metric_entry.bcn_query, datalen);

	free(ctx->metric_entry.bcn_query);
	ctx->metric_entry.bcn_query = NULL;

	return 0;
}

/**
* @brief Fn to parse ap metrics query msg
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
*
* @return -1 if error else 0
*/
int parse_ap_metrics_query_message(struct own_1905_device *ctx, unsigned char *buf)
{
	int length = 0;
	unsigned char *temp_buf;
	unsigned int integrity = 0;
#ifdef MAP_R2
	unsigned short *band_idx = &ctx->metric_entry.total_radio_band;
	*band_idx = 0;
#endif
	temp_buf = buf;

	while (1) {
		if (*temp_buf == AP_METRICS_QUERY_TYPE) {
			integrity |= (1 << AP_METRICS_QUERY_TYPE_CHECK);
			length = parse_ap_metrics_query_tlv(temp_buf, ctx);
			if (length < 0) {
				err("error ap metrics query tlv");
				return -1;
			}
			temp_buf += length;
		} 
#ifdef MAP_R2

else if (*temp_buf == AP_RADIO_IDENTIFIER_TYPE) {
			integrity |= (1 << AP_RADIO_IDENTIFIER_TYPE_CHECK);
			length = parse_ap_radio_identifier_tlv(temp_buf, ctx, *band_idx);
			if (length < 0) {
				err("error radio identifier type tlv");
				return -1;
			}
			*band_idx+=1;

			//code to remove
			//return 0;
			temp_buf += length;
		} 
#endif
                else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
	/*check integrity */
	if ((integrity & (1 << AP_METRICS_QUERY_TYPE_CHECK)) == 0) {
		err("incomplete ap metrics query message 0x%x 0x%x",
			     integrity, ((1 << AP_METRICS_QUERY_TYPE_CHECK) | (1 << AP_RADIO_IDENTIFIER_TYPE_CHECK)));
		return -1;
	}
	excess_debug("exit");

	return 0;
}

/**
* @brief Fn to parse link metrics query msg
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
* @param target neighbor target
* @param type tx/rx/both
*
* @return -1 if error else 0
*/
static int parse_link_metric_query_message(struct own_1905_device *ctx,
					   unsigned char *buf, unsigned char *target, unsigned char *type)
{
	int length = 0;
	unsigned char *temp_buf;

	temp_buf = buf;

	length = parse_link_metric_query_type_tlv(temp_buf, target, type);

	if (length < 0) {
		err("error link metric query tlv ");
		return -1;
	}
	temp_buf += length;

	return 0;
}

/**
* @brief Fn to update link metrics info to 1905
*
* @param ctx own 1905 device ctx
* @param target neighbor target
* @param type rx/tx/both
*
* @return -1 if error else 0
*/
int send_link_metrics_response_message(struct own_1905_device *ctx, unsigned char *target, int type)
{
	int count, is_ap;
	struct tx_link_metrics *tx_sta, *tx_ap;
	struct rx_link_metrics *rx_sta, *rx_ap;
	unsigned char *tx_ap_buf, *rx_ap_buf, *tx_sta_buf, *rx_sta_buf;
	unsigned char zero_bssid[ETH_ALEN] = { 0 };

	struct _1905_map_device *tmp_dev = topo_srv_get_1905_device(ctx, NULL);
	struct map_neighbor_info *neighbor;
	struct link_stat_query lsq;
	struct iface_info *ifc_info = SLIST_FIRST(&(tmp_dev->_1905_info.first_iface));
	int tx_metrics_cnt = 0, rx_metrics_cnt = 0;
	struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;

#define MAX_SIZE 1024
	tx_sta = os_zalloc(MAX_SIZE);
	tx_ap= os_zalloc(MAX_SIZE);
	rx_sta= os_zalloc(MAX_SIZE);
	rx_ap = os_zalloc(MAX_SIZE);
	excess_debug("enter");

	if((tx_sta == NULL) || (tx_ap == NULL) || (rx_sta == NULL) || (rx_ap == NULL)){
		if(tx_sta){
			os_free(tx_sta);
		}
		if(tx_ap){
			os_free(tx_ap);
		}
		if(rx_sta){
			os_free(rx_sta);
		}
		if(rx_ap){
			os_free(rx_ap);
		}
		return -1;
	}

	//TODO should we allocate memory here?
	tx_ap_buf = (unsigned char *)tx_ap;
	rx_ap_buf = (unsigned char *)rx_ap;
	tx_sta_buf = (unsigned char *)tx_sta;
	rx_sta_buf = (unsigned char *)rx_sta;

	if (SLIST_EMPTY(&tmp_dev->neighbors_entry)) {
		os_free(tx_sta);
		os_free(tx_ap);
		os_free(rx_sta);
		os_free(rx_ap);
		return -1;
	}
	lsq.media_type = ifc_info->media_type;
	if (!memcmp(zero_bssid, target, ETH_ALEN)) {
		SLIST_FOREACH(neighbor, &tmp_dev->neighbors_entry, next_neighbor) {
			if (!neighbor) {
				err("neighbor not found for link metrics");
				os_free(tx_sta);
				os_free(tx_ap);
				os_free(rx_sta);
				os_free(rx_ap);
				return -1;
			}
			ctx->metric_entry.bh = SLIST_FIRST(&neighbor->bh_head);
			if ((memcmp(zero_bssid ,ctx->metric_entry.bh->connected_iface_addr, ETH_ALEN) == 0) ||
				memcmp(zero_bssid ,ctx->metric_entry.bh->neighbor_iface_addr, ETH_ALEN) == 0) {
				err("bh entry is not valid yet");
				continue;
			}
			ifc_info = topo_srv_get_iface(tmp_dev, ctx->metric_entry.bh->connected_iface_addr);
			if(!ifc_info) {
				err("interface not found");
				continue;
			}
			lsq.media_type = ifc_info->media_type;

			os_memcpy(lsq.local_if, ctx->metric_entry.bh->connected_iface_addr, ETH_ALEN);
			os_memcpy(lsq.neighbor_if, ctx->metric_entry.bh->neighbor_iface_addr, ETH_ALEN);
			wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_TX_LINK_STATISTICS,
					WAPP_TX_LINK_STATISTICS, target, NULL, &lsq, sizeof(struct link_stat_query), 0, 1, 0);
			wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_RX_LINK_STATISTICS,
					WAPP_RX_LINK_STATISTICS, target, NULL, &lsq, sizeof(struct link_stat_query), 0, 1, 0);
			ctx->metric_entry.bh = NULL;

			count =
				append_link_metric_info(tx_ap_buf, rx_ap_buf, tx_sta_buf, rx_sta_buf, tmp_dev, neighbor, &is_ap);
			if (count > 0) {
				if (!is_ap) {
					memcpy(tx_ap_buf, tx_sta_buf, sizeof(struct tx_link_metrics));
					memcpy(rx_ap_buf, rx_sta_buf, sizeof(struct rx_link_metrics));
				}
				tx_metrics_cnt++;
				rx_metrics_cnt++;
				tx_ap_buf += sizeof(struct tx_link_metrics);
				tx_sta_buf += sizeof(struct tx_link_metrics);
				rx_ap_buf += sizeof(struct rx_link_metrics);
				rx_sta_buf += sizeof(struct rx_link_metrics);
			}
		}
	} else {

		SLIST_FOREACH(neighbor, &tmp_dev->neighbors_entry, next_neighbor) {
			if (memcmp(neighbor->n_almac, target, ETH_ALEN)){
				continue;
			}

			ctx->metric_entry.bh = SLIST_FIRST(&neighbor->bh_head);
			if ((memcmp(zero_bssid ,ctx->metric_entry.bh->connected_iface_addr, ETH_ALEN) == 0) ||
				(memcmp(zero_bssid ,ctx->metric_entry.bh->neighbor_iface_addr, ETH_ALEN) == 0)) {
				continue;
			}
			ifc_info = topo_srv_get_iface(tmp_dev, ctx->metric_entry.bh->connected_iface_addr);
			if(!ifc_info) {
				err("interface not found");
				continue;
			}
			lsq.media_type = ifc_info->media_type;
			os_memcpy(lsq.local_if, ctx->metric_entry.bh->connected_iface_addr, ETH_ALEN);
			os_memcpy(lsq.neighbor_if, ctx->metric_entry.bh->neighbor_iface_addr, ETH_ALEN);
			wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_TX_LINK_STATISTICS,
					WAPP_TX_LINK_STATISTICS, target, NULL, &lsq, sizeof(struct link_stat_query), 0, 1, 0);
			wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_RX_LINK_STATISTICS,
					WAPP_RX_LINK_STATISTICS, target, NULL, &lsq, sizeof(struct link_stat_query), 0, 1, 0);
			ctx->metric_entry.bh = NULL;

			count =
				append_link_metric_info(tx_ap_buf, rx_ap_buf, tx_sta_buf, rx_sta_buf, tmp_dev, neighbor, &is_ap);
			if (count > 0) {
				tx_metrics_cnt++;
				rx_metrics_cnt++;
				if (is_ap) {
					tx_ap_buf += sizeof(struct tx_link_metrics);
					rx_ap_buf += sizeof(struct rx_link_metrics);
				} else {
					memcpy(tx_ap_buf, tx_sta_buf, sizeof(struct tx_link_metrics));
					memcpy(rx_ap_buf, rx_sta_buf, sizeof(struct rx_link_metrics));
					tx_ap_buf += sizeof(struct tx_link_metrics);
					rx_ap_buf += sizeof(struct rx_link_metrics);
				}
			}
			break;
		}
		if (!neighbor) {
			err("failed to get the specified neighbor");
			os_free(tx_sta);
			os_free(tx_ap);
			os_free(rx_sta);
			os_free(rx_ap);
			return -1;
		}
	}
	if (type == TX_METRICS_ONLY)
		rx_metrics_cnt = 0;
	else if (type == RX_METRICS_ONLY)
		tx_metrics_cnt = 0;

	if (mapd_ctx)
		map_1905_Set_Link_Metrics_Rsp_Info(mapd_ctx->_1905_ctrl,
			tx_metrics_cnt, tx_ap,
			rx_metrics_cnt, rx_ap);

	os_free(tx_sta);
	os_free(tx_ap);
	os_free(rx_sta);
	os_free(rx_ap);


	return 0;
}

/**
* @brief Fn to handle link metrics query
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
*
* @return -1 if error else 0
*/
int topo_srv_handle_link_metrics_query(struct own_1905_device *ctx, unsigned char *buf)
{
	unsigned char target[ETH_ALEN], type;

	if (0 > parse_link_metric_query_message(ctx, buf, target, &type)) {
		err("receive error link metric query message");
		return -1;
	}
	send_link_metrics_response_message(ctx, target, type);

	return 0;
}

/**
* @brief Fn to prase assoc sta metrics query and send it to wapp
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
* @param len msg len
*
* @return 0 if success else error
*/
int topo_srv_handle_assoc_sta_metrics_query(struct own_1905_device *ctx, unsigned char *buf, int len)
{
	struct mapd_global *global = (struct mapd_global *)ctx->back_ptr;
	struct link_metrics sta_metrics;
	excess_debug("got ASSOC_STA_LINK_METRICS_QUERY");
	hex_dump("ASSOC_STA_LINK_METRICS_QUERY", (buf - ETH_HLEN), len);

	/*parse AP_LINK_METRICS_QUERY msg */
	if (0 > parse_associated_sta_link_metrics_query_message(ctx, buf)) {
		err("error! no need to response this associated sta link metrics query message");
		return -1;
	}

	/*query ap metrics response info from wapp */
	if (topo_srv_get_associate_client(ctx, NULL, ctx->metric_entry.assoc_sta) == NULL) {
		err("STA is not present"MACSTR, MAC2STR(ctx->metric_entry.assoc_sta));
		os_memcpy(sta_metrics.mac, ctx->metric_entry.assoc_sta, ETH_ALEN);
#ifdef MAP_R2
		map_1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(global->_1905_ctrl, 1, &sta_metrics,
					0, NULL, ctx->metric_entry.assoc_sta, 2);
#else
		map_1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(global->_1905_ctrl, 0, NULL, ctx->metric_entry.assoc_sta, 2);
#endif
		return -1;
	}
	wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_ONE_ASSOC_STA_LINK_METRICS,
		WAPP_ONE_ASSOC_STA_LINK_METRICS, NULL, ctx->metric_entry.assoc_sta, NULL,
			0, 1, 1, 0);
#ifdef MAP_R2
	wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_ONE_ASSOC_STA_EXTENDED_LINK_METRICS,
		WAPP_ONE_ASSOC_STA_EXTENDED_LINK_METRICS, NULL, ctx->metric_entry.assoc_sta, NULL,
			0, 1, 1, 0);
#endif
	return 0;
}

/**
* @brief Fn to parse metrics query and send it to wapp
*
* @param ctx own 1905 device
* @param buf msg buffer
* @param len msg len
*
* @return -1 if error else 0
*/
int topo_srv_handle_metrics_query(struct own_1905_device *ctx, unsigned char *buf, int len, unsigned char periodic)
{
	int ret = 0;

	ret = parse_ap_metrics_query_message(ctx, buf);

	if (ret < 0) {
		err("this shouldn't happen");
		return -1;
	}
	/*query ap metrics response info from wapp */
	topo_srv_get_ap_metrics_info(ctx);
	topo_srv_get_assoc_sta_traffic_stats(ctx);
	topo_srv_get_all_assoc_sta_link_metrics(ctx);
#ifdef MAP_R2
	if(!periodic)
		topo_srv_get_radio_metrics_info(ctx);
	else
		topo_srv_get_all_radio_metrics_info(ctx);
#endif
	return 0;
}

int topo_srv_cont_update_ap_metrics(struct own_1905_device *ctx)
{
       /*query ap metrics response info from wapp */
       topo_srv_get_own_metrics_info(ctx);

       /* TODO later */
#if 0
       topo_srv_get_assoc_sta_traffic_stats(ctx);
       topo_srv_get_all_assoc_sta_link_metrics(ctx);
#endif
       return 0;
}

int topo_srv_cont_update_link_metrics(struct own_1905_device *ctx)
{
       /*query ap metrics response info from wapp */
       topo_srv_get_own_link_metrics_info(ctx);

       /* TODO later */
#if 0
       topo_srv_get_assoc_sta_traffic_stats(ctx);
       topo_srv_get_all_assoc_sta_link_metrics(ctx);
#endif
       return 0;
}

/**
* @brief Fn to append metrics info
*
* @param pkt buffer where metrics needs to be appended
* @param mrsp bss info db
*
* @return -1 if error else 0
*/
unsigned short append_ap_metrics_info(unsigned char *pkt, struct bss_info_db *mrsp)
{
	struct esp_db *esp = NULL;
	struct ap_metrics_info_lib *ap_metrics;
	struct esp_info *esp_lib;

	ap_metrics = (struct ap_metrics_info_lib *)pkt;

	memcpy(ap_metrics->bssid, mrsp->bssid, ETH_ALEN);

	ap_metrics->ch_util = mrsp->ch_util;
	ap_metrics->assoc_sta_cnt = mrsp->assoc_sta_cnt;
	ap_metrics->valid_esp_count = 0;
	esp_lib = ap_metrics->esp;
	debug("insert struct mrsp_db append_ap_metrics_info");
	debug("bssid(%02x:%02x:%02x:%02x:%02x:%02x) ch_uti=%d, assoc_sta_cnt=%d, valid_esp_cnt=%d",
			PRINT_MAC(ap_metrics->bssid),ap_metrics->ch_util, ap_metrics->assoc_sta_cnt, mrsp->esp_cnt);

	SLIST_FOREACH(esp, &(mrsp->esp_head), esp_entry) {
		ap_metrics->valid_esp_count++;
		esp_lib->ac = esp->ac;
		esp_lib->format = esp->format;
		esp_lib->ba_win_size = esp->ba_win_size;
		esp_lib->e_air_time_fraction = esp->e_air_time_fraction;
		esp_lib->ppdu_dur_target = esp->ppdu_dur_target;
		debug("insert struct esp_db append_ap_metrics_info");
		debug("ac=%d, format=%d ba_win_size=%d", esp_lib->ac, esp_lib->format, esp_lib->ba_win_size);
		debug("e_air_time_fraction=%d, ppdu_dur_target=%d esp =%p",
			esp_lib->e_air_time_fraction, esp_lib->ppdu_dur_target, esp_lib);
		esp_lib = &ap_metrics->esp[ap_metrics->valid_esp_count];
	}

	return 0;
}

/**
* @brief Fn to append ap metrics tlv
*
* @param pkt msg buffer
* @param mrsp bss info
*
* @return -1 if error else 0
*/
unsigned short append_ap_metrics_tlv(unsigned char *pkt, struct bss_info_db *mrsp)
{
	struct esp_db *esp = NULL;
	struct ap_metrics_info_lib *ap_metrics;
	struct esp_info *esp_lib;
	unsigned short len = 0;

	ap_metrics = (struct ap_metrics_info_lib *)pkt;

	memcpy(ap_metrics->bssid, mrsp->bssid, ETH_ALEN);
	len += ETH_ALEN;

	ap_metrics->ch_util = mrsp->ch_util;
	len += sizeof(char);
	ap_metrics->assoc_sta_cnt = mrsp->assoc_sta_cnt;
	len += sizeof(short);
	ap_metrics->valid_esp_count = 0;
	len += sizeof(char);
	esp_lib = ap_metrics->esp;
	SLIST_FOREACH(esp, &(mrsp->esp_head), esp_entry) {
		ap_metrics->valid_esp_count++;
		esp_lib->ac = esp->ac;
		esp_lib->format = esp->format;
		esp_lib->ba_win_size = esp->ba_win_size;
		esp_lib->e_air_time_fraction = esp->e_air_time_fraction;
		esp_lib->ppdu_dur_target = esp->ppdu_dur_target;
		debug(" esplib =%p ac=%d, format=%d, ba_win=%d, airtime=%d, target=%d valid_esp_count=%d", esp_lib, esp_lib->ac,
				esp_lib->format, esp_lib->ba_win_size,
				esp_lib->e_air_time_fraction, esp_lib->ppdu_dur_target, ap_metrics->valid_esp_count);
		esp_lib = &ap_metrics->esp[ap_metrics->valid_esp_count];
	}
	len += ap_metrics->valid_esp_count * sizeof(struct esp_info);

	debug("len =%d", len);
	return len;
}

#ifdef MAP_R2
unsigned short append_radio_metrics_tlv(unsigned char *pkt, struct mapd_radio_info *radio_info)
{
	struct radio_metrics_lib *radio_metrics;
	unsigned short len = 0;

	radio_metrics = (struct radio_metrics_lib *)pkt;

	os_memcpy(radio_metrics->identifier, radio_info->radio_metrics.ra_id, ETH_ALEN);
	err("radio ID "MACSTR"",MAC2STR(radio_info->radio_metrics.ra_id));
	len += ETH_ALEN;

	radio_metrics->noise = radio_info->radio_metrics.cu_noise;
	len += sizeof(char);
	radio_metrics->transmit = radio_info->radio_metrics.cu_tx;
	len += sizeof(short);
	radio_metrics->receive_self = radio_info->radio_metrics.cu_rx;
	len += sizeof(char);
	radio_metrics->receive_other= radio_info->radio_metrics.cu_other;
	len += sizeof(short);
	debug("len =%d", len);
	err("radio_metric cu_other %d", radio_info->radio_metrics.cu_other);
	return len;
}
unsigned short append_ch_util_tlv(unsigned char *pkt, struct mapd_radio_info *radio_info)
{
	struct ch_util_lib *ch_util;
	unsigned short len = 0;

	ch_util = (struct ch_util_lib *)pkt;

	ch_util->ch_num = radio_info->channel;
	ch_util->edcca = radio_info->radio_metrics.edcca;
	len = sizeof(struct ch_util_lib);
	debug("len =%d", len);
	return len;
}
#endif

/**
* @brief Fn to append sta traffic stats
*
* @param pkt msg buffer where sta traffic stats to be appended
* @param stats stats_db
*
* @return 0 if success else -1
*/
unsigned short append_sta_traffic_stats_tlv(unsigned char *pkt, struct stats_db *stats, int byte_cnt)
{
	struct stat_info *stats_lib;

	stats_lib = (struct stat_info *)pkt;

	memcpy(stats_lib->mac, stats->mac, ETH_ALEN);
	stats_lib->bytes_sent = stats->bytes_sent/power(1024, byte_cnt);
	stats_lib->bytes_received = stats->bytes_received/power(1024, byte_cnt);
	stats_lib->bytes_sent = stats->bytes_sent;
	stats_lib->bytes_received = stats->bytes_received;
	stats_lib->packets_sent = stats->packets_sent;
	stats_lib->packets_received = stats->packets_received;
	stats_lib->tx_packets_errors = stats->tx_packets_errors;
	stats_lib->rx_packets_errors = stats->rx_packets_errors;
	stats_lib->retransmission_count = stats->retransmission_count;

	return 0;
}

/**
* @brief Fn to append sta link metrics
*
* @param pkt buffer where data needs to be appended
* @param metrics sta link metrics db
*
* @return -1 if error else 0
*/
unsigned short append_sta_link_metrics_tlv(unsigned char *pkt, struct associated_clients *metrics)
{
	struct link_metrics *link_met;

	link_met = (struct link_metrics *)pkt;

	memcpy(link_met->mac, metrics->client_addr, ETH_ALEN);
	memcpy(link_met->bssid, metrics->bss->bssid, ETH_ALEN);

	link_met->time_delta = metrics->time_delta;
	link_met->erate_downlink = metrics->erate_downlink;
	link_met->erate_uplink = metrics->erate_uplink;
	link_met->time_delta = metrics->time_delta;
	link_met->rssi_uplink = rssi_to_rcpi((signed char)metrics->rssi_uplink);

	return 0;
}
unsigned short append_one_sta_link_metrics_tlv(unsigned char *pkt, struct metrics_db*metrics)
{
	struct link_metrics *link_met;

	link_met = (struct link_metrics *)pkt;

	memcpy(link_met->mac, metrics->mac, ETH_ALEN);
	memcpy(link_met->bssid, metrics->bssid, ETH_ALEN);

	link_met->time_delta = metrics->time_delta;
	link_met->erate_downlink = metrics->erate_downlink;
	link_met->erate_uplink = metrics->erate_uplink;
	link_met->time_delta = metrics->time_delta;
	link_met->rssi_uplink = metrics->rssi_uplink;

	return 0;
}


#ifdef MAP_R2
unsigned short append_ap_ext_metrics_tlv(unsigned char *pkt, struct bss_info_db *metrics)
{
	struct ap_extended_metrics_lib *link_met = NULL;

	link_met = (struct ap_extended_metrics_lib *)pkt;

	os_memcpy(link_met->bssid, metrics->bssid, ETH_ALEN);
	link_met->bc_rx = metrics->bc_rx;
	link_met->bc_tx = metrics->bc_tx;
	link_met->mc_rx = metrics->mc_rx;
	link_met->mc_tx = metrics->mc_tx;
	link_met->uc_rx = metrics->uc_rx;
	link_met->uc_tx = metrics->uc_tx;
	debug("bc_rx: %d, bc_tx: %d, mc_rx: %d, mc_tx: %d, uc_rx: %d, uc_tx: %d",
		link_met->bc_rx,
		link_met->bc_tx,
		link_met->mc_rx,
		link_met->mc_tx,
		link_met->uc_rx,
		link_met->uc_tx);

	return 0;
}
unsigned short append_sta_link_ext_metrics_tlv(unsigned char *pkt, struct associated_clients *metrics)
{
	struct sta_extended_metrics_lib *link_met = NULL;

	link_met = (struct sta_extended_metrics_lib *)pkt;

	memcpy(link_met->sta_mac, metrics->client_addr, ETH_ALEN);
	memcpy(link_met->metric_info[0].bssid, metrics->bss->bssid, ETH_ALEN);

	link_met->extended_metric_cnt = 1;
	link_met->metric_info[0].last_data_dl_rate = metrics->sta_ext_info.last_data_dl_rate;
	link_met->metric_info[0].last_data_ul_rate = metrics->sta_ext_info.last_data_ul_rate;
	link_met->metric_info[0].utilization_rx = metrics->sta_ext_info.utilization_rx;
	link_met->metric_info[0].utilization_tx = metrics->sta_ext_info.utilization_tx;

	return 0;
}
unsigned short append_one_sta_link_ext_metrics_tlv(unsigned char *pkt, struct metrics_db *metrics)
{
	struct sta_extended_metrics_lib *link_met = NULL;

	link_met = (struct sta_extended_metrics_lib *)pkt;

	memcpy(link_met->sta_mac, metrics->mac, ETH_ALEN);
	memcpy(link_met->metric_info[0].bssid, metrics->bssid, ETH_ALEN);

	link_met->extended_metric_cnt = 1;
	link_met->metric_info[0].last_data_dl_rate = metrics->sta_ext_info.last_data_dl_rate;
	link_met->metric_info[0].last_data_ul_rate = metrics->sta_ext_info.last_data_ul_rate;
	link_met->metric_info[0].utilization_rx = metrics->sta_ext_info.utilization_rx;
	link_met->metric_info[0].utilization_tx = metrics->sta_ext_info.utilization_tx;

	return 0;
}



/**
* @brief Fn to fill ap metrics rsp message
*
* @param ctx own 1905 device ctx
* @param info ap_metrics_info_lib to be filled
* @param ap_metrics_info_cnt ap metrics count
* @param sta_stats sta stats to be filled
* @param sta_stats_cnt sta stats count
* @param sta_metrics sta metrics to be filled
* @param sta_metrics_cnt sta metrics count
*
* @return -1 if error else 0
*/
unsigned short topo_srv_sta_metrics_rsp_message(struct own_1905_device *ctx,
				struct link_metrics **sta_metrics, unsigned char *sta_metrics_cnt
#ifdef MAP_R2
				, struct sta_extended_metrics_lib **ext_sta_metric, unsigned char *ext_sta_met_cnt
#endif
			)
{
	*sta_metrics_cnt = 1;
	*sta_metrics = os_zalloc(sizeof(struct link_metrics) * (*sta_metrics_cnt));

	append_one_sta_link_metrics_tlv((unsigned char *)*sta_metrics, &ctx->metric_entry.assoc_sta_link_metrics);
	debug("sta metics info cnt=%d", *sta_metrics_cnt);

#ifdef MAP_R2
	if (ctx->map_version == DEV_TYPE_R2) {
		*ext_sta_met_cnt = 1;
		*ext_sta_metric = os_zalloc((sizeof(struct sta_extended_metrics_lib) + sizeof(struct extended_metrics_info))* (*ext_sta_met_cnt));

		append_one_sta_link_ext_metrics_tlv((unsigned char *)*ext_sta_metric, &ctx->metric_entry.assoc_sta_link_metrics);
		err("ext sta metics info cnt=%d", *ext_sta_met_cnt);
	}
#endif
	return 0;

}
#endif


/**
* @brief Fn to fill ap metrics rsp message
*
* @param ctx own 1905 device ctx
* @param info ap_metrics_info_lib to be filled
* @param ap_metrics_info_cnt ap metrics count
* @param sta_stats sta stats to be filled
* @param sta_stats_cnt sta stats count
* @param sta_metrics sta metrics to be filled
* @param sta_metrics_cnt sta metrics count
*
* @return -1 if error else 0
*/
unsigned short topo_srv_ap_metrics_rsp_message(struct own_1905_device *ctx,
					       struct ap_metrics_info_lib **info, int *ap_metrics_info_cnt,
					       struct stat_info **sta_stats, int *sta_stats_cnt,
					       struct link_metrics **sta_metrics, int *sta_metrics_cnt
#ifdef MAP_R2					       
					       , struct ap_extended_metrics_lib **ext_ap_metric, int *ext_ap_met_cnt,
					       struct sta_extended_metrics_lib **ext_sta_metric, int *ext_sta_met_cnt,
							struct radio_metrics_lib **info_radio, int *radio_metrics_info_cnt,
							struct ch_util_lib **ch_util, int *ch_util_cnt, unsigned char periodic
#endif
							)
{
	struct bss_info_db *mrsp = NULL;
	struct traffic_stats_db *traffic_stats = NULL;
	struct stats_db *stats = NULL;
#ifdef MAP_R2
	//struct metrics_db *metrics = NULL;
#endif
	unsigned char *tlv_temp_buf;
	unsigned char tmp_buf[3000] = {0};
	struct _1905_map_device *device = topo_srv_get_1905_device(ctx, NULL);
	struct bss_db *bss = NULL;
	struct associated_clients *client = NULL;
	struct metric_policy_db *policy = NULL;
#ifdef MAP_R2
	struct mapd_radio_info *radio_info = NULL;
	struct mapd_global *global = ctx->back_ptr;
	int i;
	struct tlv_head vs;
	struct radio_info_db *radio_db = NULL;
	struct _1905_map_device *cont = topo_srv_get_controller_device(ctx);
#endif
	int byte_cnt = 0;

#ifdef MAP_R2
	if (cont->map_version == DEV_TYPE_R2 && device->map_version == DEV_TYPE_R2)
		byte_cnt = ctx->r2_ap_capab->byte_counter_units;
#endif
	tlv_temp_buf = tmp_buf;
	SLIST_FOREACH(bss, &(ctx->metric_entry.metrics_query_head), bss_entry) {
		mrsp = topo_srv_get_bss_by_bssid(ctx, topo_srv_get_1905_device(ctx, NULL), bss->bssid);
		if (!mrsp) {
			err("failed to find AP with bssid %02x:%02x:%02x:%02x:%02x:%02x",
					PRINT_MAC(bss->bssid));
			continue;
		}
		append_ap_metrics_tlv(tlv_temp_buf, mrsp);
		tlv_temp_buf += sizeof(struct ap_metrics_info_lib);
		*ap_metrics_info_cnt = *ap_metrics_info_cnt + 1;
	}
	if (*ap_metrics_info_cnt) {
		*info = os_zalloc(sizeof(struct ap_metrics_info_lib) * (*ap_metrics_info_cnt));
		os_memcpy(*info, tmp_buf, sizeof(struct ap_metrics_info_lib) * *ap_metrics_info_cnt);
	}

	tlv_temp_buf = tmp_buf;
	
	SLIST_FOREACH(traffic_stats, &ctx->metric_entry.traffic_stats_head, traffic_stats_entry) {
		SLIST_FOREACH(policy, &ctx->map_policy.mpolicy.policy_head, policy_entry) {
			if (os_memcmp(policy->identifier,traffic_stats->identifier,ETH_ALEN) == 0 &&
				policy->sta_stats_inclusion) {
				SLIST_FOREACH(stats, &traffic_stats->stats_head, stats_entry) {
					append_sta_traffic_stats_tlv(tlv_temp_buf, stats, byte_cnt);
					tlv_temp_buf += sizeof(struct stat_info);
					*sta_stats_cnt = *sta_stats_cnt + 1;
					debug("sta stats info cnt=%d", *sta_stats_cnt);
				}
			}
		}
	}
	if (*sta_stats_cnt) {
		*sta_stats = os_zalloc(sizeof(struct stat_info) * (*sta_stats_cnt));
		os_memcpy(*sta_stats, tmp_buf, (sizeof(struct stat_info) * (*sta_stats_cnt)));
	}
	tlv_temp_buf = tmp_buf;
	SLIST_FOREACH(client, &device->assoc_clients, next_client) {
		SLIST_FOREACH(policy, &ctx->map_policy.mpolicy.policy_head, policy_entry) {
			if (os_memcmp(policy->identifier, client->bss->radio->identifier, ETH_ALEN) == 0 &&
				policy->sta_metrics_inclusion) {
				append_sta_link_metrics_tlv(tlv_temp_buf, client);
				tlv_temp_buf += sizeof(struct link_metrics);
				*sta_metrics_cnt = *sta_metrics_cnt + 1;
				debug("sta metics info cnt=%d", *sta_metrics_cnt);
			}
		}
	}
	if (*sta_metrics_cnt) {
		*sta_metrics = os_zalloc(sizeof(struct link_metrics) * (*sta_metrics_cnt));
		os_memcpy(*sta_metrics, tmp_buf, (sizeof(struct link_metrics) * (*sta_metrics_cnt)));
	}
#ifdef MAP_R2
	tlv_temp_buf = tmp_buf;
	SLIST_FOREACH(bss, &(ctx->metric_entry.metrics_query_head), bss_entry) {
		mrsp = topo_srv_get_bss_by_bssid(ctx, topo_srv_get_1905_device(ctx, NULL), bss->bssid);
		if (!mrsp) {
			err("failed to find AP with bssid %02x:%02x:%02x:%02x:%02x:%02x",
					PRINT_MAC(bss->bssid));
			continue;
		}
		append_ap_ext_metrics_tlv(tlv_temp_buf, mrsp);
		tlv_temp_buf += sizeof(struct ap_extended_metrics_lib);
		*ext_ap_met_cnt = *ext_ap_met_cnt + 1;
		debug("ap ext metics info cnt=%d", *ext_ap_met_cnt);
	}
	if (*ext_ap_met_cnt) {
		*ext_ap_metric = os_zalloc(sizeof(struct ap_extended_metrics_lib) * (*ext_ap_met_cnt));
		os_memcpy(*ext_ap_metric, tmp_buf, (sizeof(struct ap_extended_metrics_lib) * (*ext_ap_met_cnt)));
	}

	tlv_temp_buf = tmp_buf;
	if(periodic) {
		/*need to send:
		include one Radio Metrics TLV for each radio which it is operating*/
		SLIST_FOREACH(radio_db,&device->first_radio,next_radio) {
			radio_info = get_radio_info_by_radio_id(global,radio_db->identifier);
			if(!radio_info)
				continue;
			debug("@@@MAP@@@ channel = %d", radio_info->channel);
			append_radio_metrics_tlv(tlv_temp_buf, radio_info);
			mapd_hexdump(MSG_OFF, "append_radio_metrics_tlv", tlv_temp_buf,sizeof(struct radio_metrics_lib));
			tlv_temp_buf += sizeof(struct radio_metrics_lib);
			*radio_metrics_info_cnt = *radio_metrics_info_cnt + 1;
			debug("radio metrics info cnt=%d", *radio_metrics_info_cnt);
		}
	} else {
		for(i = 0; i < ctx->metric_entry.total_radio_band; i++) {
			radio_info = get_radio_info_by_radio_id(global, ctx->metric_entry.radio_id[i].identifier);
			if(!radio_info)
				continue;
			debug("@@@MAP@@@ radio_idx = %d", radio_info->radio_idx);
			append_radio_metrics_tlv(tlv_temp_buf, radio_info);
			mapd_hexdump(MSG_OFF, "append_radio_metrics_tlv", tlv_temp_buf,sizeof(struct radio_metrics_lib));
			tlv_temp_buf += sizeof(struct radio_metrics_lib);
			*radio_metrics_info_cnt = *radio_metrics_info_cnt + 1;
			debug("radio metrics info cnt=%d", *radio_metrics_info_cnt);
		}
	}
	if (*radio_metrics_info_cnt) {
		debug("radio_metrics_info_cnt memcpy");
		*info_radio = os_zalloc(sizeof(struct radio_metrics_lib) * (*radio_metrics_info_cnt));
		os_memcpy(*info_radio, tmp_buf, sizeof(struct radio_metrics_lib) * *radio_metrics_info_cnt);
	}
	tlv_temp_buf = tmp_buf;
	SLIST_FOREACH(client, &device->assoc_clients, next_client) {
		SLIST_FOREACH(policy, &ctx->map_policy.mpolicy.policy_head, policy_entry) {
			if (os_memcmp(policy->identifier, client->bss->radio->identifier, ETH_ALEN) == 0 &&
				policy->sta_metrics_inclusion) {
					append_sta_link_ext_metrics_tlv(tlv_temp_buf, client);
					mapd_hexdump(MSG_OFF, "append_sta_link_ext_metrics_tlv", tlv_temp_buf,sizeof(struct sta_extended_metrics_lib) + sizeof(struct extended_metrics_info));
					tlv_temp_buf += (sizeof(struct sta_extended_metrics_lib) + sizeof(struct extended_metrics_info));
					*ext_sta_met_cnt = *ext_sta_met_cnt + 1;
					debug("ext sta metics info cnt=%d", *ext_sta_met_cnt);
			}
		}
	}
	if (*ext_sta_met_cnt) {
		*ext_sta_metric = os_zalloc((sizeof(struct sta_extended_metrics_lib) + sizeof(struct extended_metrics_info)) * (*ext_sta_met_cnt));
		os_memcpy(*ext_sta_metric, tmp_buf, ((sizeof(struct sta_extended_metrics_lib) + sizeof(struct extended_metrics_info)) * (*ext_sta_met_cnt)));
	}
#if 1 
	tlv_temp_buf = tmp_buf;
	if(periodic) {
		SLIST_FOREACH(radio_db,&device->first_radio,next_radio) {
			radio_info = get_radio_info_by_radio_id(global,radio_db->identifier);
			if(!radio_info)
				continue;
			debug("@@@MAP@@@ radio_idx = %d", radio_info->radio_idx);
			append_ch_util_tlv(tlv_temp_buf, radio_info);
			mapd_hexdump(MSG_OFF, "ch util lib", tlv_temp_buf,sizeof(struct ch_util_lib));
			tlv_temp_buf += sizeof(struct ch_util_lib);
			*ch_util_cnt = *ch_util_cnt + 1;
			debug("ch util cnt=%d", *ch_util_cnt);
		}
	} else {
		for(i = 0; i < ctx->metric_entry.total_radio_band; i++) {
			radio_info = get_radio_info_by_radio_id(global, ctx->metric_entry.radio_id[i].identifier);
			if(!radio_info)
				continue;
			debug("@@@MAP@@@ radio_idx = %d", radio_info->radio_idx);
			append_ch_util_tlv(tlv_temp_buf, radio_info);
			mapd_hexdump(MSG_OFF, "ch util lib", tlv_temp_buf,sizeof(struct ch_util_lib));
			tlv_temp_buf += sizeof(struct ch_util_lib);
			*ch_util_cnt = *ch_util_cnt + 1;
			debug("ch util cnt=%d", *ch_util_cnt);
		}
	}
	if (*ch_util_cnt) {
		*ch_util = os_zalloc(sizeof(struct ch_util_lib) * (*ch_util_cnt) + sizeof(struct tlv_head));
		if (*ch_util == NULL) {
			*ch_util_cnt = 0;
			return -1;
		}
		vs.tlv_type = VENDOR_SPECIFIC_TLV_TYPE;
		vs.func_type = FUNC_VENDOR_CHANNEL_UTIL_RSP;
		os_memcpy(vs.oui, MTK_OUI, OUI_LEN);
		vs.tlv_len = OUI_LEN + sizeof(struct ch_util_lib) * (*ch_util_cnt) + 1;//1 is for function type len
		vs.tlv_len = host_to_be16(vs.tlv_len);
		char *temp_ch_util = (char *)*ch_util;
		os_memcpy(temp_ch_util, &vs, sizeof(struct tlv_head)) ;
		os_memcpy(temp_ch_util + sizeof(struct tlv_head), tmp_buf, sizeof(struct ch_util_lib) * *ch_util_cnt);
		mapd_hexdump(MSG_ERROR, "ch_util", *ch_util, sizeof(struct tlv_head) + (sizeof(struct ch_util_lib) * (*ch_util_cnt)));
	}
	#endif
#endif
	return 0;

}
