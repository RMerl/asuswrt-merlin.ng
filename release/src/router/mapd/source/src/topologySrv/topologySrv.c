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
 *  topology server
 *
 *  Abstract:
 *  topology server
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Kapil.Gupta 2018/05/02    First implementation of the 1905 topology server
 * */

#include "includes.h"
#ifdef __linux__
#include <fcntl.h>
#endif				/* __linux__ */

#include "common.h"
#include <sys/un.h>
#include "interface.h"
#include "./../1905_local_lib/data_def.h"
#include "1905_map_interface.h"
#include "client_db.h"
#include "mapd_i.h"
#include "topologySrv.h"
#include "eloop.h"
#include "tlv_parsor.h"
#include "1905_if.h"
#include "wapp_if.h"
#include "mapd_debug.h"
#include "chan_mon.h"
#include "client_mon.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "apSelection.h"
#include "ch_planning.h"
#include "network_optimization.h"
#include "mapd_user_iface.h"
#include "ctrl_iface.h"

#ifdef CENT_STR
#include "steer_action.h"
#include "ap_cent_str.h"


u8 MBO_OCE_OUI_BYTE[4] = {0x50, 0x6f, 0x9a, 0x16};
#endif
#define MAX_HYSTERESIS_MARGIN 5

int get_band_from_channel_dual_band(int chan);
struct global_oper_class {
	unsigned char opclass;	/* regulatory class */
	unsigned char channel_num;
	unsigned char channel_set[MAX_CH_NUM];	/* max 13 channels, use 0 as terminator */
};

/*Global operating classes*/
struct global_oper_class oper_class[] = {
	{0, 0, {0}},		/* Invlid entry */
	{81, 13, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}},
	{82, 1, {14}},
	{83, 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
	{84, 9, {5, 6, 7, 8, 9, 10, 11, 12, 13}},
	{94, 2, {133, 137}},
	{95, 4, {132, 134, 136, 138}},
	{96, 8, {131, 132, 133, 134, 135, 136, 137, 138}},
	{101, 2, {21, 25}},
	{102, 5, {11, 13, 15, 17, 19}},
	{103, 10, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
	{104, 2, {184, 192}},
	{105, 2, {188, 196}},
	{106, 2, {191, 195}},
	{107, 5, {189, 191, 193, 195, 197}},
	{108, 10, {188, 189, 190, 191, 192, 193, 194, 195, 196, 197}},
	{109, 4, {184, 188, 192, 196}},
	{110, 7, {183, 184, 185, 186, 187, 188, 189}},
	{111, 8, {182, 183, 184, 185, 186, 187, 188, 189}},
	{112, 3, {8, 12, 16}},
	{113, 5, {7, 8, 9, 10, 11}},
	{114, 6, {6, 7, 8, 9, 10, 11}},
	{115, 4, {36, 40, 44, 48}},
	{116, 2, {36, 44}},
	{117, 2, {40, 48}},
	{118, 4, {52, 56, 60, 64}},
	{119, 2, {52, 60}},
	{120, 2, {56, 64}},
	{121, 12, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144}},
	{122, 6, {100, 108, 116, 124, 132, 140}},
	{123, 6, {104, 112, 120, 128, 136, 144}},
	{124, 4, {149, 153, 157, 161}},
	{125, 6, {149, 153, 157, 161, 165, 169}},
	{126, 2, {149, 157}},
	{127, 2, {153, 161}},
	{128, 7, {42, 58, 106, 122, 138, 144, 155}},
	{129, 2, {50, 114}},
	{130, 6, {42, 58, 106, 122, 138, 155}},
	{0, 0, {0}}		/* end */
};

unsigned char opclass_2g[] = {
	81, 82, 83, 84, 101, 102, 103, 112, 113, 114, 0
};

unsigned char opclass_5gh[] = {
	94, 95, 96, 104, 105, 106, 107, 108, 109, 110, 111, 121, 122,
	123, 124, 125, 126, 127, 0
};

unsigned char opclass_5gl[] = {
	115, 116, 117, 118, 119, 120, 0
};

Boolean check_is_triband(struct _1905_map_device *dev)
{
	int radio_cnt=0;
	struct radio_info_db *radio;
	SLIST_FOREACH(radio, &(dev->first_radio), next_radio) {
		radio_cnt++;
	}
	if (radio_cnt>2)
		return TRUE;
	else
		return FALSE;
}
int get_band(struct _1905_map_device *dev,int chan)
{
	if(check_is_triband(dev)) {
		return get_band_from_channel(chan);
	}
	else {
		return get_band_from_channel_dual_band(chan);
	}
	return 0;
}
/**
* @brief Fn to get band from channel
*
* @param chan channel number
*
* @return band
*/
int get_band_from_channel(int chan)
{
#define MAX_CHAN_NUMBER_2G 14
#define MAX_CHAN_5GL 100
	if (chan <= MAX_CHAN_NUMBER_2G)
		return BAND_2G;
	else if (chan < MAX_CHAN_5GL)
		return BAND_5GL;
	else
		return BAND_5GH;
}

int get_band_from_channel_dual_band(int chan)
{
	if (chan <= MAX_CHAN_NUMBER_2G)
		return BAND_2G;
	else
		return BAND_5GL;
}
#undef MAX_CHAN_NUMBER_2G
#undef MAX_CHAN_5GL

int get_bandcap(unsigned char opclass, unsigned char non_opch_num, unsigned char *non_opch)
{
	unsigned char chnum = 0;
	int i = 0, j = 0, k = 0;
	struct global_oper_class *opcls = oper_class;
	unsigned char *chset = NULL;
	unsigned char cap = BAND_INVALID_CAP;

	do {
		if (opcls[i].opclass == opclass) {
			chnum = opcls[i].channel_num;
			chset = opcls[i].channel_set;
			if (non_opch_num == chnum) {
				debug("opclass(%d) all channel not used", opclass);
				cap = BAND_INVALID_CAP;
			} else {
				switch (opclass) {
				case 81:
				case 82:
				case 83:
				case 84:
				case 101:
				case 102:
				case 103:
				case 112:
				case 113:
				case 114:
					cap = BAND_2G_CAP;
					break;
				case 115:
				case 116:
				case 117:
				case 118:
				case 119:
				case 120:
					cap = BAND_5GL_CAP;
					break;
				case 128:
				case 129:
				case 130:
					for (j = 0; j < chnum; j++) {
						for (k = 0; k < non_opch_num; k++) {
							if (chset[j] == non_opch[k])
								break;
						}
						if (k == non_opch_num) {
							if (chset[j] < 100)
								cap |= BAND_5GL_CAP;
							else
								cap |= BAND_5GH_CAP;
							debug("opclass=%d channel(%d) support cap=%d", opclass,
							      chset[j], cap);
						}
					}
					break;
				default:
					err("unknown opclass %d", opclass);
					break;

				}
				debug("opclass=%d band_cap=%d", opclass, cap);
			}
			break;
		}
		i++;
	} while (opcls[i].opclass != 0);

	return cap;
}

void reset_net_opt_allowed(struct own_1905_device *ctx)
{
	ctx->network_optimization.scan_2g_allow = 0;
	ctx->network_optimization.scan_5gl_allow = 0;
	ctx->network_optimization.scan_5gh_allow = 0;
}

void net_opt_check_bh_scan_allowed(struct own_1905_device *ctx, struct bss_info_db *bss)
{
	if ((bss->radio->band == BAND_2G) && (bss->map_vendor_extn & BSS_BH)) {
			ctx->network_optimization.scan_2g_allow = 1;
	} else if ((bss->radio->band == BAND_5GL ) && (bss->map_vendor_extn & BSS_BH)) {
			ctx->network_optimization.scan_5gl_allow = 1;
	} else if ((bss->radio->band == BAND_5GH) && (bss->map_vendor_extn & BSS_BH)) {
			ctx->network_optimization.scan_5gh_allow = 1;
	}
}

/**
* @brief : Fn to update upstream device based on APCLI connection
*
* @param ctx : own 1905 device ctx
* @param tmp_dev : 1905 device
* @param mac_addr : bssid which belongs to upstream device
*
* @return : 0 if no error else -1
*/
int topo_srv_update_upstream_device(struct own_1905_device *ctx, struct _1905_map_device *tmp_dev,
				    unsigned char *mac_addr)
{
	if (!tmp_dev) {
		tmp_dev = topo_srv_get_1905_device(ctx, NULL);
	}
	tmp_dev->upstream_device = topo_srv_get_1905_by_bssid(ctx, mac_addr);

	return 0;
}
struct _1905_map_device *topo_srv_get_1905_device_from_iface(struct
							     own_1905_device
							     *ctx, u8 * iface_addr);


/**
* @brief : Fn to validate channel based on opclass and bw
*
* @param op_class : operating class
* @param ch_num : no of channel in channellist
* @param ch_list : channel list to be validated
*
* @return : 1 if any channel is valid else 0
*/
unsigned char check_invalid_channel(unsigned char op_class, unsigned char ch_num, struct channel_selection_chan_list *ch_list)
{
	struct global_oper_class *opcls = oper_class;
	int i = 0, j = 0, k = 0;
	unsigned char opcls_found = 0;

	/*check if the channel is in oper_class */
	do {
		if (opcls[i].opclass == op_class) {
			opcls_found = 1;
			for (k = 0; k < ch_num; k++) {
				for (j = 0; j < opcls[i].channel_num; j++) {
					debug("ch_set: %u, chlist_ch: %u\n", opcls[i].channel_set[j], ch_list[k].channel);
					if ((opcls[i].channel_set[j] == ch_list[k].channel) ||
						/*(ch_list[k].channel == get_primary_channel(opcls[i].channel_set[j]))*/ 
						is_valid_primary_ch_80M_160M(ch_list[k].channel, opcls[i].channel_set[j],
									opcls[i].opclass))
						break;
				}
				if (j == opcls[i].channel_num) {
					err("Invalid channel");
					return 0;	/*invalid channel */
				}
			}
			break;
		}
		i++;
	} while (opcls[i].opclass != 0);

	if (opcls_found)
		return 1;
	else
		return 0;
}
int get_op_class_channel_list(unsigned char op_class, struct prefer_info_db *perfer)
{
	struct global_oper_class *opcls = oper_class;
	u8 i =1,j=0;
	while(opcls[i].opclass != 0) {
		if(opcls[i].opclass == op_class) {
			break;
		}
		i++;
	}
	if(opcls[i].opclass == 0)
		return 0;
	else{
		for(j=0;j<opcls[i].channel_num;j++){
			perfer->ch_list[j]=opcls[i].channel_set[j];
		}
		perfer->ch_num=opcls[i].channel_num;
		debug("added %d channels to prefer list ch[0]%d", perfer->ch_num, perfer->ch_list[0]);
		return 1;
	}
}

/**
* @brief : Fn to return iface_info based on mac address
*
* @param device : 1905 device interface belongs to
* @param mac : mac address of interface
*
* @return : pointer to interface info if iface is present else NULL
*/
struct iface_info *topo_srv_get_iface(struct _1905_map_device *device, u8 * mac)
{
	struct iface_info *iface = NULL;

	/* basic validation */
	if (!device) {
		err("device is null");
		return NULL;
	}
	if (SLIST_EMPTY(&(device->_1905_info.first_iface)))
		return NULL;

	if (!mac)
		return SLIST_FIRST(&(device->_1905_info.first_iface));

	SLIST_FOREACH(iface, &(device->_1905_info.first_iface), next_iface) {
		if (os_memcmp(iface->iface_addr, mac, ETH_ALEN) == 0)
			return iface;
	}

	return NULL;
}

int topo_srv_deinit_1905dev_info(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev);
#define DEVICE_NOT_IN_NETWORK_TIME 100 /* In secs */
#define DEVICE_TIMEOUT_TIME 300 /* In secs */
/**
* @brief : Fn to kick out 1905 devices which are timeout out
*
* @param eloop_ctx : eloop context
* @param timeout_ctx : timeout context
*/
void topo_srv_start_1905_timer(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = eloop_ctx;
	struct own_1905_device *ctx = &global->dev;
	struct _1905_map_device *tmp_dev, *dev = topo_srv_get_next_1905_device(ctx, topo_srv_get_1905_device(ctx, NULL));
	struct os_time current_time;

	os_get_time(&current_time);
	while (dev) {
		tmp_dev = topo_srv_get_next_1905_device(ctx, dev);
		if ((dev->in_network == 1) && (current_time.sec - dev->last_seen.sec > DEVICE_NOT_IN_NETWORK_TIME)) {
			topo_srv_move_1905_off_nw(ctx, dev);
		}
#if 0
		if ((dev->in_network == 0) && ((current_time.sec - dev->last_seen.sec) > DEVICE_TIMEOUT_TIME)) {
			/* Memory cleanup */
			info("deleting almac (%02x:%02x:%02x:%02x:%02x:%02x) last=%lu current=%lu",
				PRINT_MAC(dev->_1905_info.al_mac_addr), dev->last_seen.sec, current_time.sec);
			topo_srv_deinit_1905dev_info(ctx, dev);
			/* kick out this node */
			SLIST_REMOVE(&ctx->_1905_dev_head, dev, _1905_map_device, next_1905_device);
			free(dev);
		}
#endif
		dev = tmp_dev;
	}
	eloop_register_timeout(5, 0, topo_srv_start_1905_timer, global, NULL);
}

/**
* @brief : Fn to get next interface of any 1905 device
*
* @param device : 1905 device pointer
* @param iface : current interface pointer
*
* @return : interface pointer if exists else NULL
*/
struct iface_info *topo_srv_get_next_iface(struct _1905_map_device *device, struct iface_info *iface)
{
	struct list_interface *iface_list = NULL;

	/* basic validation */
	if (!device) {
		err("device is null");
		return NULL;
	}

	iface_list = &device->_1905_info.first_iface;
	if (SLIST_EMPTY(iface_list))
		return NULL;

	/* Return first iface if bss is null */
	if (!iface)
		return SLIST_FIRST(iface_list);

	return SLIST_NEXT(iface, next_iface);
}

/**
* @brief : Fn to get next bss of any 1905 device based on current bss
*
* @param device : 1905 device pointer
* @param bss : current bss
*
* @return : bss pointer if exists else NULL
*/
struct bss_info_db *topo_srv_get_next_bss(struct _1905_map_device *device, struct bss_info_db *bss)
{
	struct list_bss *bss_list;

	/* basic validation */
	if (!device) {
		err("device is null");
		return NULL;
	}

	bss_list = &device->first_bss;
	if (SLIST_EMPTY(bss_list))
		return NULL;

	/* Return first bss if bss is null */
	if (!bss)
		return SLIST_FIRST(bss_list);

	return SLIST_NEXT(bss, next_bss);
}

/**
* @brief : Fn to find mac address of an interface based on iface name
*
* @param iface_name : interface name
* @param mac_addr : mac address to be filled by Fn
*
* @return : 0 if interface address is found else -1
*/
int lookup_iface_addr(char *iface_name, unsigned char *mac_addr)
{
	struct ifreq ifr;
	int sock;

	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (-1 == sock) {
		err("failed to open a socket");
		return 1;
	}

	os_memcpy(ifr.ifr_name, iface_name, IFNAMSIZ);

	if (-1 == ioctl(sock, SIOCGIFHWADDR, &ifr)) {
		err("ioctl(SIOCGIFADDR)");
		close(sock);
		return 1;
	}

	close(sock);

	memcpy(mac_addr, ifr.ifr_hwaddr.sa_data, 6);
	debug("mac addr of %s interface is " MACSTR, iface_name, MAC2STR(mac_addr));
	return 0;
}

/**
* @brief : Fn to get interface pointer based on interface address
*
* @param ctx : own 1905 device ctx
* @param dev : 1905 device to which interface belongs to
* @param ifaddr : mac address of interface
*
* @return : interface pointer if interface is found else NULL
*/
struct iface_info *topo_srv_get_interface(struct own_1905_device *ctx, struct _1905_map_device *dev,
					  unsigned char *ifaddr)
{
	struct iface_info *iface = NULL;

	do {
		iface = topo_srv_get_next_iface(dev, iface);
		if (!iface) {
			debug("next interface not found");
			return NULL;
		}
		if (os_memcmp(iface->iface_addr, ifaddr, ETH_ALEN) == 0)
			return iface;
	} while (iface);

	return NULL;
}

/**
* @brief : Fn to return bss based on bssid
*
* @param ctx : own 1905 device context
* @param dev : 1905 device pointer
* @param ifaddr : bssid of the bss
*
* @return : bss pointer if found else NULL
*/
struct bss_info_db *topo_srv_get_bss_by_bssid(struct own_1905_device *ctx, struct _1905_map_device *dev,
					      unsigned char *ifaddr)
{
	struct bss_info_db *bss = NULL;
	struct _1905_map_device *tmp_dev = NULL;

	if (dev) {
		do {
			bss = topo_srv_get_next_bss(dev, bss);
			if (!bss) {
				debug("mac not found " MACSTR, MAC2STR(ifaddr));
				return NULL;
			}
			if (os_memcmp(bss->bssid, ifaddr, ETH_ALEN) == 0)
				return bss;
		} while (bss);

		return NULL;
	}
	SLIST_FOREACH(tmp_dev, &ctx->_1905_dev_head, next_1905_device) {
		do {
			if (!tmp_dev)
				return NULL;
			bss = topo_srv_get_next_bss(tmp_dev, bss);
			if (!bss) {
				debug("mac not found " MACSTR, MAC2STR(ifaddr));
				continue;
			}
			if (os_memcmp(bss->bssid, ifaddr, ETH_ALEN) == 0)
				return bss;
		} while (bss);
	}
	return NULL;
}

/**
* @brief : Fn to return 1905 upstream device
*
* @param _1905_dev : 1905 device pointer
*
* @return : pointer to 1905 upstream device
*/
struct _1905_map_device *topo_srv_get_upstream_device(struct _1905_map_device *_1905_dev)
{
	return _1905_dev->upstream_device;
}

/**
* @brief : Fn to return 1905 device pointer based on bssid
*
* @param ctx : own 1905 device context
* @param ifaddr : bssid to be searched
*
* @return : 1905 device pointer if found else NULL
*/
struct _1905_map_device *topo_srv_get_1905_by_bssid(struct own_1905_device *ctx, unsigned char *ifaddr)
{
	struct bss_info_db *bss = NULL;
	struct _1905_map_device *tmp_dev;

	SLIST_FOREACH(tmp_dev, &ctx->_1905_dev_head, next_1905_device) {
		do {
			bss = topo_srv_get_next_bss(tmp_dev, bss);
			if (!bss) {
				debug("next bss not found");
				continue ;
			}
			if (os_memcmp(bss->bssid, ifaddr, ETH_ALEN) == 0)
				return tmp_dev;
		} while (bss);
	}
	return NULL;
}

/**
* @brief : Fn to get 1905 device based of interface mac address
*
* @param ctx : own 1905 device context
* @param ifaddr : mac address of interface
*
* @return : 1905 device pointer if found else NULL
*/
struct _1905_map_device *topo_srv_get_1905_by_iface_addr(struct own_1905_device *ctx, unsigned char *ifaddr)
{
	struct iface_info *iface = NULL;
	struct _1905_map_device *tmp_dev;

	SLIST_FOREACH(tmp_dev, &ctx->_1905_dev_head, next_1905_device) {
		do {
			iface = topo_srv_get_next_iface(tmp_dev, iface);
			if (!iface) {
				debug("next iface not found");
				continue ;
			}
			if (os_memcmp(iface->iface_addr, ifaddr, ETH_ALEN) == 0)
				return tmp_dev;
		} while (iface);
	}
	return NULL;
}

/**
* @brief : Fn to return bss by interface name
*
* @param _1905_device : 1905 device pointer
* @param iface_name : interface name
*
* @return : bss pointer if exists else NULL
*/
struct bss_info_db *topo_srv_get_bss_by_ifname(struct _1905_map_device
					       *_1905_device, char *iface_name)
{
	struct bss_info_db *bss = NULL;

	/* Get interface address by ifname, define this API */
	unsigned char iface_addr[6] = {0};

	lookup_iface_addr(iface_name, iface_addr);
	do {
		bss = topo_srv_get_next_bss(_1905_device, bss);
		if (!bss) {
			err("failed to get bss with ifname");
			return NULL;
		}
		if (os_memcmp(bss->bssid, iface_addr, ETH_ALEN) == 0)
			return bss;
	} while (bss);

	return NULL;
}

/**
* @brief : Fn to return 1905 controller device pointer
*
* @param ctx : own 1905 device context
*
* @return : 1905 device pointer if found else NULL
*/
struct _1905_map_device *topo_srv_get_controller_device(struct own_1905_device *ctx)
{
	struct _1905_map_device *tmp_dev;

	if (SLIST_EMPTY(&ctx->_1905_dev_head)) {
		err("1905 list is empty");
		return NULL;
	}

	SLIST_FOREACH(tmp_dev, &ctx->_1905_dev_head, next_1905_device) {
		if (tmp_dev->device_role == DEVICE_ROLE_CONTROLLER  ||
			tmp_dev->device_role == DEVICE_ROLE_CONTRAGENT)
			return tmp_dev;
	}
	warn("controller not found in topology db");

	return NULL;
}

/**
* @brief : Fn to return 1905 device pointer based on al mac address
*
* @param ctx : own 1905 device context
* @param al_mac : almac to be searched
*
* @return : 1905 device pointer if found else NULL
*/
struct _1905_map_device *topo_srv_get_1905_device(struct own_1905_device *ctx, u8 * al_mac)
{
	struct _1905_map_device *tmp_dev;

	if (SLIST_EMPTY(&ctx->_1905_dev_head)) {
		assert(0);
		err("1905 list is empty");
		return NULL;
	}

	if (!al_mac)
		return SLIST_FIRST(&ctx->_1905_dev_head);

	SLIST_FOREACH(tmp_dev, &ctx->_1905_dev_head, next_1905_device) {
		if (os_memcmp(tmp_dev->_1905_info.al_mac_addr, al_mac, ETH_ALEN) == 0)
			return tmp_dev;
	}
	warn("mac not found in topology db "MACSTR, MAC2STR(al_mac));

	return NULL;
}

/**
* @brief : Fn to return next 1905 device in topology
*
* @param ctx : own 1905 device context
* @param _1905_dev : current 1905 device pointer
*
* @return : 1905 device pointer or NULL
*/
struct _1905_map_device *topo_srv_get_next_1905_device(struct own_1905_device
						       *ctx, struct _1905_map_device
						       *_1905_dev)
{
	if (!ctx)
		return NULL;

	if (SLIST_EMPTY(&ctx->_1905_dev_head))
		return NULL;

	if (!_1905_dev)
		return SLIST_FIRST(&ctx->_1905_dev_head);

	return SLIST_NEXT(_1905_dev, next_1905_device);
}

/**
* @brief : Fn to return next radio of any 1905 device based on curent radio
*
* @param device : 1905 device pointer
* @param radio : current radio / NULL
*
* @return : radio pointer or NULL
*/
struct radio_info_db *topo_srv_get_next_radio(struct _1905_map_device *device, struct radio_info_db *radio)
{
	struct list_radio *radio_list;

	/* basic validation */
	if (!device) {
		err("device is null");
		return NULL;
	}

	radio_list = &device->first_radio;
	if (SLIST_EMPTY(radio_list))
		return NULL;

	/* Return first bss if bss is null */
	if (!radio)
		return SLIST_FIRST(radio_list);

	return SLIST_NEXT(radio, next_radio);
}

/**
* @brief : Fn to return radio pointer based on radio itentifier
*
* @param _1905_device : 1905 device in which radio needs to be seached
* @param identifier : radio identifier
*
* @return : radio pointer if found else NULL
*/
struct radio_info_db *topo_srv_get_radio_by_channel(
	struct _1905_map_device *_1905_device,
	unsigned char channel)
{
	struct radio_info_db *radio = NULL;

	do {
		radio = topo_srv_get_next_radio(_1905_device, radio);
		if (!radio) {
			warn("failed to find radio with given identifier\n");
			return NULL;
		}
		if ((radio->channel[0] == channel) ||
			(radio->channel[1] == channel) ||
			(radio->channel[2] == channel) ||
			(radio->channel[3] == channel) )
			return radio;
	} while (radio);

	return NULL;
}

/**
* @brief : Fn to return radio pointer based on radio itentifier
*
* @param _1905_device : 1905 device in which radio needs to be seached
* @param identifier : radio identifier
*
* @return : radio pointer if found else NULL
*/
struct radio_info_db *topo_srv_get_radio(struct _1905_map_device *_1905_device, unsigned char *identifier)
{
	struct radio_info_db *radio = NULL;

	if (identifier == NULL)
	{
		return SLIST_FIRST(&_1905_device->first_radio);
	}
	do {
		radio = topo_srv_get_next_radio(_1905_device, radio);
		if (!radio) {
			debug("failed to find radio with given identifier");
			return NULL;
		}
		if (os_memcmp(radio->identifier, identifier, ETH_ALEN) == 0)
			return radio;
	} while (radio);

	return NULL;
}

struct radio_info_db *topo_srv_get_radio_by_band(struct _1905_map_device *_1905_device, unsigned char chan)
{
	struct radio_info_db *radio = NULL;
	int radio_cnt = 0;

	radio = topo_srv_get_next_radio(_1905_device, radio);
	while(radio) {
		radio_cnt++;
		radio = topo_srv_get_next_radio(_1905_device, radio);
	};

	radio = NULL;
	do {
		radio = topo_srv_get_next_radio(_1905_device, radio);
		if (!radio) {
			warn("failed to find radio with given identifier");
			return NULL;
		}
		if (radio_cnt <= 2) {
			if (get_band_from_channel_dual_band(radio->channel[0]) == get_band_from_channel_dual_band(chan))
				return radio;
		} else {
			if (get_band_from_channel(radio->channel[0]) == get_band_from_channel(chan))
				return radio;
		}
	} while (radio);

	return NULL;
}
/**
* @brief : helper API, does blind copy without validation, caller needs to take care of validations
*
* @param bss : bss in which config copy needs to be done
* @param bss_config : wireless setting from controlller
*/
static void topo_srv_update_bss_info(struct bss_info_db *bss, struct wireless_setting *bss_config)
{
	os_memset(bss->ssid, 0, MAX_SSID_LEN);
	os_memset(bss->key, 0, 64);
	bss->auth_mode = 0;
	bss->enc_type = 0;
	bss->key_len = 0;
	bss->map_vendor_extn = 0x20;

	bss->hidden = bss_config->hidden_ssid;
	bss->auth_mode = bss_config->AuthMode;
	bss->enc_type = bss_config->EncrypType;
	bss->map_vendor_extn = bss_config->map_vendor_extension;
	os_memcpy((char *)bss->ssid, (char *)bss_config->Ssid, sizeof(bss->ssid));
	bss->ssid_len = os_strlen((char *)bss_config->Ssid);
	os_memcpy((char *)bss->key, (char *)bss_config->WPAKey, sizeof(bss->key));
	bss->key_len = os_strlen((char *)bss_config->WPAKey);
}

/**
* @brief : Fn to dump radio info
*
* @param radio : pointer to radio
*/
void topo_srv_dump_radio_info(struct radio_info_db *radio)
{
	always("identifier %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(radio->identifier));
	always("band=%u\n", radio->band);
	always("operating_class=%u\n", radio->operating_class);
	always("channel=%u\n", radio->channel[0]);
	always("channel=%u\n", radio->power);
	always("wireless_mode=%u\n", topo_srv_get_wireless_mode(radio->wireless_mode));
	//TODO ht, vt, chan pref, chan restrict later;
}

#ifdef MAP_R2
#if 0
void topo_srv_dump_ch_scan_info(struct own_1905_device *ctx, struct radio_info_db *radio)
{
	int i;
	struct neighbor_info *nb_info;
	always("OperatingClass=%u\n", radio->scan_result.oper_class);
	always("OperatingChannel=%u\n", radio->scan_result.channel);
	always("ScanStatus=%u\n", radio->scan_result.scan_status);
	always("ChannelUtilization=%u\n", radio->scan_result.utilization);
	always("ChannelNoise=%u\n", radio->scan_result.noise);
	always("ChannelScanType=%u\n", radio->scan_result.scan_type);
	always("Timestamp=%s\n", radio->scan_result.timestamp);
	always("AggScanDuration=%u\n", radio->scan_result.agg_scan_duration);
	for(i = 0;i < radio->scan_result.neighbor_num; i++) {
		nb_info = &radio->scan_result.nb_info[i];
		always("NeighborBSSID=(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(nb_info->bssid));
		always("NeighborSSID=%s\n", nb_info->ssid);
		always("NeighborRCPI=%u\n", nb_info->RCPI);
		always("NeighborBW=%s\n", nb_info->ch_bw);
		if (nb_info->cu_stacnt_present & 0x80)
			always("NeighborCU: %u\n", nb_info->cu);
		if (nb_info->cu_stacnt_present & 0x40)
			always("NeighborSTACount: %u\n", nb_info->sta_cnt);
	}
}
#endif
void de_stats_timeout(void * eloop_ctx, void *user_ctx)
{
	struct mapd_global *ctx = eloop_ctx;
	struct own_1905_device *own_dev = &ctx->dev;
	struct _1905_map_device *dev = user_ctx, *next_dev = NULL;
	u8 flag = 0;
	dev->de_done = 0;
	SLIST_FOREACH(next_dev, &own_dev->_1905_dev_head, next_1905_device) {
		if (next_dev->de_done == 1)
			flag = 1;
	}
	if (flag == 0 && ctx->dev.de_state != OFF) {
		ctx->dev.de_state = OFF;
	}
}

int topo_srv_dump_radio_de_own_info(struct own_1905_device *ctx, struct radio_info_db *radio, char** reply_buf, size_t buf_len)
{
	struct mapd_radio_info *radio_info = NULL;
	struct mapd_global *global = ctx->back_ptr;
	char *pos, *end;
	int ret = 0;

	pos = *reply_buf;
	end = pos + buf_len;

	radio_info = get_radio_info_by_radio_id(global, radio->identifier);
	if(radio_info == NULL)
		return -1;
	ret = os_snprintf(pos, end - pos, "{\n\"identifier\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n",
			PRINT_MAC(radio->identifier));
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Radio Noise\":\"%d\",\n",
		radio_info->radio_metrics.cu_noise);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Radio Transmit\":\"%d\",\n",
			radio_info->radio_metrics.cu_tx);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Radio Receive Self\":\"%d\",\n",
			radio_info->radio_metrics.cu_rx);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Radio Receive Other\":\"%d\"",
			radio_info->radio_metrics.cu_other);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	always("Noise=%u\n", radio_info->radio_metrics.cu_noise);
	always("Transmit=%u\n", radio_info->radio_metrics.cu_tx);
	always("ReceiveSelf=%u\n", radio_info->radio_metrics.cu_rx);
	always("RecieveOther=%u\n", radio_info->radio_metrics.cu_other);
	ret = os_snprintf(pos, end - pos, "},\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	*reply_buf = pos;
	return 0;
	//TODO ht, vt, chan pref, chan restrict later;
}

int topo_srv_dump_radio_de_info(struct own_1905_device *ctx, struct radio_info_db *radio, char** reply_buf, size_t buf_len)
{
	//struct mapd_radio_info *mapd_radio = NULL;
	//struct mapd_global *global = ctx->back_ptr;
	//mapd_radio = get_radio_info_by_radio_id(global, radio->identifier);
	char *pos, *end;
	int ret = 0;

	pos = *reply_buf;
	end = pos + buf_len;

	ret = os_snprintf(pos, end - pos, "{\n\"identifier\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n",
			PRINT_MAC(radio->identifier));
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Radio Noise\":\"%d\",\n",
		radio->radio_metrics.cu_noise);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Radio Transmit\":\"%d\",\n",
			radio->radio_metrics.cu_tx);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Radio Receive Self\":\"%d\",\n",
			radio->radio_metrics.cu_rx);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Radio Receive Other\":\"%d\"",
			radio->radio_metrics.cu_other);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "},\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	always("identifier %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(radio->identifier));
	always("Noise=%u\n", radio->radio_metrics.cu_noise);
	always("Transmit=%u\n", radio->radio_metrics.cu_tx);
	always("ReceiveSelf=%u\n", radio->radio_metrics.cu_rx);
	always("ReceiveOther=%u\n", radio->radio_metrics.cu_other);
	//topo_srv_dump_ch_scan_info(ctx, radio);
	//TODO ht, vt, chan pref, chan restrict later;
	*reply_buf = pos;
	return 0;
}
int topo_srv_dump_bss_ext_info(struct bss_info_db *bss, char** reply_buf, size_t buf_len)
{
	char *pos, *end;
	int ret = 0;
	pos = *reply_buf;
	end = pos + buf_len;

	ret = os_snprintf(pos, end - pos, "{\n\"BSSID\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n",
			PRINT_MAC(bss->bssid));
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"UnicastBytesSent\":\"%d\",\n",
		bss->uc_tx);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"UnicastBytesReceived\":\"%d\",\n",
		bss->uc_rx);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"MulticastBytesSent\":\"%d\",\n",
		bss->mc_tx);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"MulticastBytesReceived\":\"%d\",\n",
		bss->mc_rx);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"BroadcastBytesSent\":\"%d\",\n",
		bss->bc_tx);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"BroadcastBytesReceived\":\"%d\",\n",
		bss->bc_rx);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"AssociationStatus\":\"%d\"\n",
		!bss->status);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	always("bssid %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(bss->bssid));
	always("UnicastBytesSent= %u\n", bss->uc_tx);
	always("UnicastBytesReceived= %u\n", bss->uc_rx);
	always("MulticastBytesSent= %u\n", bss->mc_tx);
	always("MulticastBytesReceived= %u\n", bss->mc_rx);
	always("BroadcastBytesSent= %u\n", bss->bc_tx);
	always("BroadcastBytesReceived= %u\n", bss->bc_rx);
	always("AssociationStatus= %u\n", !bss->status);

	ret = os_snprintf(pos, end - pos, "\n},\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	*reply_buf = pos;
	return 0;
}

int topo_srv_dump_client_ext_metric_info(struct own_1905_device *ctx, struct associated_clients *sta, char** reply_buf, size_t buf_len)
{
	char *pos, *end;
	int ret = 0;
	pos = *reply_buf;
	end = pos + buf_len;
	u32 temp_rate_Kbps_dl = 0, temp_rate_Kbps_ul = 0;
	ret = os_snprintf(pos, end - pos, "{\n\"STA MAC\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n",
			PRINT_MAC(sta->client_addr));
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	temp_rate_Kbps_dl = sta->sta_ext_info.last_data_dl_rate * 1000;
	ret = os_snprintf(pos, end - pos, "\"LastDataDownlinkRate\":\"%d\",\n",
		temp_rate_Kbps_dl);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	temp_rate_Kbps_ul = sta->sta_ext_info.last_data_ul_rate * 1000;
	ret = os_snprintf(pos, end - pos, "\"LastDataUplinkRate\":\"%d\",\n",
		temp_rate_Kbps_ul);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"UtilizationReceive\":\"%d\",\n",
		sta->sta_ext_info.utilization_rx);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"UtilizationTransmit\":\"%d\"",
		sta->sta_ext_info.utilization_tx);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	always("STA MAC %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(sta->client_addr));
	always("LastDataDownlinkRate= %u\n", temp_rate_Kbps_dl);
	always("LastDataUplinkRate= %u\n", temp_rate_Kbps_ul);
	always("UtilizationReceive= %u\n", sta->sta_ext_info.utilization_rx);
	always("UtilizationTransmit= %u\n", sta->sta_ext_info.utilization_tx);
	ret = os_snprintf(pos, end - pos, "\n},\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	*reply_buf = pos;
	return 0;
}
#endif
int topo_srv_dump_bh_link_metrics(struct _1905_map_device *dev, char** reply_buf, size_t buf_len)
{
	char *pos, *end;
	int ret = 0;
	struct map_neighbor_info *neighbor_info = NULL;
	struct backhaul_link_info *bh_info = NULL;
	unsigned char exist = 0, exist_1 = 0;
	pos = *reply_buf;
	end = pos + buf_len;

	SLIST_FOREACH(neighbor_info, &dev->neighbors_entry, next_neighbor) {
		ret = os_snprintf(pos, end - pos, "{\n\"neighbor_al\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n\"metrics\":[",
		PRINT_MAC(neighbor_info->n_almac));
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;

		SLIST_FOREACH(bh_info, &neighbor_info->bh_head, next_bh) {
			ret = os_snprintf(pos, end - pos,
				"{\n\"local_if_mac\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n"
				"\"neighbor_if_mac\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n"
				"\"tx packet Errors\":\"%d\",\n"
				"\"transmittedPackets\":\"%d\",\n"
				"\"macThroughputCap\":\"%d\",\n"
				"\"linkAvailability\":\"%d\",\n"
				"\"phyRate\":\"%d\",\n"
				"\"rx packet Errors\":\"%d\",\n"
				"\"Packets Received\":\"%d\",\n"
				"\"RSSI\":\"%d\"\n},\n",
				PRINT_MAC(bh_info->connected_iface_addr),
				PRINT_MAC(bh_info->neighbor_iface_addr),
				bh_info->tx.pkt_err, bh_info->tx.tx_packet, bh_info->tx.mac_throughput, bh_info->tx.link_availability,
				bh_info->tx.phy_rate, bh_info->rx.pkt_err, bh_info->rx.pkt_received,
				bh_info->rx.rssi
			);
			if (ret < 0 || ret >= end - pos)
				return -1;
			pos += ret;
			exist_1 = 1;
		}

		if (exist_1 == 1) {
			pos -= 2;
			exist_1 = 0;
		}
		ret = os_snprintf(pos, end - pos, "\n]\n},\n");
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;
		exist = 1;
	}

	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	*reply_buf = pos;
	return 0;
}
int topo_srv_dump_bh_link_info(struct _1905_map_device *dev, struct backhaul_link_info *bh_entry, struct map_neighbor_info *neighbor, char** reply_buf, size_t buf_len)
{
	char *pos, *end;
	int ret = 0;

	struct iface_info *iface = NULL;
	char str[20];
	os_memset(str, '\0', sizeof(char)*20);
	iface = topo_srv_get_iface(dev , bh_entry->connected_iface_addr);

	if(!iface)
		return -1;

	if(iface->media_type == ieee_802_3_ab || iface->media_type == ieee_802_3_u) {
		if (neighbor->neighbor->root_distance > dev->root_distance) {
			always("eth connection! only show upstream connection info\n");
			return -1;
		}
		os_snprintf(str, sizeof(str), "Ethernet");
	}
	else if (*((char *)(&iface->media_info) + 6) == 0x00)
		return -1;
	if (buf_len == 0) {
		always("BH Info:\n");
		always("neighbor almac %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(neighbor->n_almac));
		always("Connected iface addr: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(bh_entry->connected_iface_addr));
	}
	if (iface->media_type & IEEE802_11_GROUP) {
		if ((iface->media_type == ieee_802_11_b) || (iface->media_type == ieee_802_11_g) ||
			(iface->media_type == ieee_802_11_n_2_4G))
			os_snprintf(str, sizeof(str), "2.4G");
		else if ((iface->media_type == ieee_802_11_a) || (iface->media_type == ieee_802_11_n_5G) ||
			(iface->media_type == ieee_802_11_ac)) {
			if(check_is_triband(dev)) {
				if (isChan5GL(iface->channel_freq_idx))
					os_snprintf(str, sizeof(str), "5GL");
				else
					os_snprintf(str, sizeof(str), "5GH");
			} else
				os_snprintf(str, sizeof(str), "5G");
		}
		else if (iface->media_type == ieee_802_11_ad)
			os_snprintf(str, sizeof(str), "60GHz");
		else if (iface->media_type == ieee_802_11_af)
			os_snprintf(str, sizeof(str), "WhiteSpace");
		else if (iface->media_type == ieee_802_11_ax) {
			if (iface->channel_freq_idx <= 14)
				os_snprintf(str, sizeof(str), "2.4G");
			else if (check_is_triband(dev)) {
				if (isChan5GL(iface->channel_freq_idx))
					os_snprintf(str, sizeof(str), "5GL");
				else
					os_snprintf(str, sizeof(str), "5GH");
			} else
					os_snprintf(str, sizeof(str), "5G");
		}
	}
	else if (iface->media_type & IEEE1901_GROUP)
		os_snprintf(str, sizeof(str), "1901 Group");
	else if (iface->media_type & MOCA_GROUP)
		os_snprintf(str, sizeof(str), "MOCA Group");
	if (buf_len == 0) {
		always("Backhaul Medium Type: %s\n", str);
		if(os_strcmp(str,"Ethernet") == 0)
			always("RSSI: NA\n");
		else
			always("RSSI: %d\n", bh_entry->rx.rssi);
	}
	if(buf_len == 0)
		return -1;
	pos = *reply_buf;
	end = pos + buf_len;
	ret = os_snprintf(pos, end - pos, "{\n\"neighbor almac addr\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n",
		PRINT_MAC(neighbor->n_almac));
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"connected iface addr\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n",
		PRINT_MAC(bh_entry->connected_iface_addr));
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Backhaul Medium Type\":\"%s\",\n",
		str);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	if(os_strcmp(str,"Ethernet") == 0)
		ret = os_snprintf(pos, end - pos, "\"RSSI\":\"NA\"},\n");
	else
		ret = os_snprintf(pos, end - pos, "\"RSSI\":\"%d\"},\n",
			bh_entry->rx.rssi);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	*reply_buf = pos;
	return ret;
}


int topo_srv_dump_bss_info_v1(struct _1905_map_device *dev, struct bss_info_db *bss, char** reply_buf, size_t buf_len, struct own_1905_device *ctx);
/**
* @brief : Fn to dump radio info
*
* @param radio : pointer to radio
*/
int topo_srv_dump_radio_info_v1(struct _1905_map_device *dev, struct radio_info_db *radio, char** reply_buf, size_t buf_len, struct own_1905_device *ctx)
{
	char *pos, *end;
	int ret = 0;
	struct bss_info_db *bss;
	unsigned char exist = 0;

	pos = *reply_buf;
	end = pos + buf_len;
	ret = os_snprintf(pos, end - pos, "{\n\"channel\":\"%d\",\n",
		radio->channel[0]);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"identifier\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n",
		PRINT_MAC(radio->identifier));
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"BW\":\"NA\",\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"wireless mode\":\"%d\",\n", topo_srv_get_wireless_mode(radio->wireless_mode));
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"Tx Spatial streams\":\"%d\",\n",
		radio->radio_capability.ht_cap.tx_stream);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"Rx Spatial streams\":\"%d\",\n\"BSSINFO\":[",
		radio->radio_capability.ht_cap.rx_stream);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;


	SLIST_FOREACH(bss, &dev->first_bss, next_bss) {
		if (bss->radio == radio) {
			exist = 1;
			if (topo_srv_dump_bss_info_v1(dev, bss, &pos, end - pos, ctx) < 0)
				return -1;
		}
	}
	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "]\n},\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	*reply_buf = pos;
	return 0;
}


/**
* @brief : Fn to dump bss info
*
* @param bss : bss pointer
*/
void topo_srv_dump_bss_info(struct bss_info_db *bss)
{
	always("bssid %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(bss->bssid));
	always("ssid=%s\n", bss->ssid);
	if(bss->auth_mode != 0) {
		always("auth_mode=%u\n", bss->auth_mode);
		always("encrypt_type=%u\n", bss->enc_type);
		always("key=%s\n", bss->key);
	}
	always("map_vendor_extn=%u\n", bss->map_vendor_extn);
}

/**
* @brief : Fn to dump bss info
*
* @param bss : bss pointer
*/
int topo_srv_dump_bss_info_v1(struct _1905_map_device *dev, struct bss_info_db *bss, char** reply_buf, size_t buf_len, struct own_1905_device *ctx)
{
	char *pos, *end;
	int ret = 0;
	struct associated_clients *client_info = NULL;
	unsigned char exist = 0;

	pos = *reply_buf;
	end = pos + buf_len;
	ret = os_snprintf(pos, end - pos, "{\n\"BSSID\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n",
		PRINT_MAC(bss->bssid));
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"SSID\":\"%s\",\n",
		bss->ssid);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	if (bss->auth_mode != 0) {
		if (ctx->config_status != DEVICE_CONFIGURED) {
			/* Assign AuthMode in WSC format if Agent is unconfigured
			 * as expected by UI */
			ret = os_snprintf(pos, end - pos, "\"Security\":\"%04x\",\n",
			WscGetAuthType(bss->auth_mode));
		}
		else {
			ret = os_snprintf(pos, end - pos, "\"Security\":\"%04x\",\n",
			bss->auth_mode);
		}
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;

		if (ctx->config_status != DEVICE_CONFIGURED) {
			/* Assign EncrypType in WSC format if Agent is unconfigured
			 * as expected by UI */
			ret = os_snprintf(pos, end - pos, "\"Encryption\":\"%04x\",\n",
			WscGetEncryType(bss->enc_type));
		}
		else {
			ret = os_snprintf(pos, end - pos, "\"Encryption\":\"%04x\",\n",
			bss->enc_type);
		}
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;

		ret = os_snprintf(pos, end - pos, "\"Pass-phrase\":\"%s\",\n",
			bss->key);
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;

		ret = os_snprintf(pos, end - pos, "\"Hidden\":\"%d\",\n",
			bss->hidden);
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;
	}

	if (bss->assoc_sta_cnt > 0) {
		ret = os_snprintf(pos, end - pos, "\"connected sta info\":[\n");
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;

		SLIST_FOREACH(client_info, &dev->assoc_clients, next_client) {
			if(client_info->bss == bss) {
				char med[10] = {0};
				struct _1905_map_device *_1905_device_ = NULL;
				struct _1905_map_device *own_dev = NULL;
				int is_1905 = 0;
				exist = 1;
				ret = os_snprintf(pos, end - pos, "{\n\"STA MAC address\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n",
					PRINT_MAC(client_info->client_addr));
				if (ret < 0 || ret >= end - pos)
					return -1;
				pos += ret;

				ret = os_snprintf(pos, end - pos, "\"last assoc time\":\"%d\",\n",
					client_info->last_assoc_time);
				if (ret < 0 || ret >= end - pos)
					return -1;
				pos += ret;
				_1905_device_ = topo_srv_get_1905_device_from_iface(ctx, client_info->client_addr);
				if(_1905_device_ != NULL)
					is_1905 = 1;
				if(is_1905 == 1) {
					ret = os_snprintf(pos, end - pos, "\"BH STA\":\"Yes\",\n");
					if (ret < 0 || ret >= end - pos)
						return -1;
					pos += ret;
				} else {
					ret = os_snprintf(pos, end - pos, "\"BH STA\":\"No\",\n");
					if (ret < 0 || ret >= end - pos)
						return -1;
					pos += ret;
				}
				if(client_info->bss->radio->band == BAND_5GH || client_info->bss->radio->band == BAND_5GL)
					os_snprintf(med, sizeof(med), "5G");
				else if(client_info->bss->radio->band == BAND_2G)
					os_snprintf(med, sizeof(med), "2.4G");
				else
					os_snprintf(med, sizeof(med), "Ethernet");
				ret = os_snprintf(pos, end - pos, "\"Medium\":\"%s\",\n",
					med);
				if (ret < 0 || ret >= end - pos)
					return -1;
				pos += ret;
				own_dev = topo_srv_get_next_1905_device(ctx, NULL);
				_1905_device_ = topo_srv_get_1905_device_from_iface(ctx, client_info->bss->bssid);
				if(_1905_device_ && own_dev) {
					if(os_memcmp(own_dev->_1905_info.al_mac_addr, _1905_device_->_1905_info.al_mac_addr, ETH_ALEN) == 0
						|| own_dev->device_role == DEVICE_ROLE_CONTROLLER) {
						ret = os_snprintf(pos, end - pos, "\"uplink rssi\":\"%d\"},\n",
								(s8)client_info->rssi_uplink);
						if (ret < 0 || ret >= end - pos)
							return -1;
						pos += ret;
					} else {
						if (own_dev->device_role == DEVICE_ROLE_AGENT) {
							ret = os_snprintf(pos, end - pos, "\"uplink rssi\":\"NA\"},\n");
							if (ret < 0 || ret >= end - pos)
								return -1;
							pos += ret;
						}
					}
				}
			}
		}
		if (exist == 1) {
			pos -= 2;
			exist = 0;
		}

		ret = os_snprintf(pos, end - pos, "\n],\n");
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;
		exist = 1;
	} else {
		pos -= 2;
	}
	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}

	ret = os_snprintf(pos, end - pos, "},\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	*reply_buf = pos;
	return 0;
}
void topo_srv_dump_client_med_info(struct own_1905_device *ctx, struct associated_clients *client_info)
{
	struct _1905_map_device *_1905_device_ = NULL;
	struct _1905_map_device *own_dev = NULL;
	int is_1905 = 0;
	_1905_device_ = topo_srv_get_1905_device_from_iface(ctx, client_info->client_addr);
	if(_1905_device_ != NULL)
		is_1905 = 1;
	always("iface addr %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(client_info->bss->bssid));
	always("client addr %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(client_info->client_addr));
	if(is_1905 == 1)
		always("BH STA: Yes\n");
	else
		always("BH STA: No\n");
	if(client_info->bss->radio->band == BAND_2G)
		always("Medium: 2.4G\n");
	else if(client_info->bss->radio->band == BAND_5GH || client_info->bss->radio->band == BAND_5GL)
		always("Medium: 5G\n");
	else
		always("Medium: Ethernet\n");
	own_dev = topo_srv_get_next_1905_device(ctx, NULL);
	_1905_device_ = topo_srv_get_1905_device_from_iface(ctx, client_info->bss->bssid);
	if(!_1905_device_ || !own_dev)
		return;
	if((os_memcmp(own_dev->_1905_info.al_mac_addr, _1905_device_->_1905_info.al_mac_addr, ETH_ALEN) == 0)
		|| own_dev->device_role == DEVICE_ROLE_CONTROLLER) {
		always("RSSI: %d\n", (s8)client_info->rssi_uplink);
	} else {
		always("RSSI: NA\n");
	}
}


/**
* @brief : Fn to dump neigbor info
*
* @param neighbor : neighbor pointer
*/
void topo_srv_dump_neighbor_info(struct map_neighbor_info *neighbor)
{
	always("neighbor almac %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(neighbor->n_almac));
}

void topo_srv_dump_bh_info(struct _1905_map_device *dev, struct map_neighbor_info *neighbor)
{
	struct backhaul_link_info *bh_entry = NULL;
	SLIST_FOREACH(bh_entry, &neighbor->bh_head, next_bh) {
		if(bh_entry) {
			topo_srv_dump_bh_link_info(dev, bh_entry, neighbor, NULL, 0);
		}
	}
}
void topo_srv_dump_1905_dev_bh_info(struct _1905_map_device *dev)
{
	struct map_neighbor_info *neighbor = NULL;
	if(dev->device_role == DEVICE_ROLE_CONTROLLER)
		return;
	always("BH Info:\n");
	always("almac %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(dev->_1905_info.al_mac_addr));
	SLIST_FOREACH(neighbor, &dev->neighbors_entry, next_neighbor) {
		topo_srv_dump_bh_info(dev, neighbor);
	}
}

/**
* @brief : Fn to dump 1905 device info
*
* @param dev : 1905 device pointer
*/
void topo_srv_dump_1905_dev_info(struct own_1905_device *ctx, struct _1905_map_device *dev)
{
	struct _1905_device *_1905_info = &dev->_1905_info;
	struct _1905_bridge *br;
	struct iface_info *iface;
	struct radio_info_db *radio;
	struct bss_info_db *bss;
	struct associated_clients *client;
	struct connected_clients *conn_client = NULL;
	struct map_neighbor_info *neighbor;

	always("=======================\n");
	always("1905 info: %p\n", dev);
	always("almac %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(_1905_info->al_mac_addr));
	SLIST_FOREACH(br, &_1905_info->first_bridge, next_bridge) {
		always("interface count=%d\n", br->interface_count);
	}
	SLIST_FOREACH(iface, &_1905_info->first_iface, next_iface) {
		always("iface addr= %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(iface->iface_addr));
		always("media type =%d\n", iface->media_type);
	}
	if(dev->device_role == DEVICE_ROLE_UNCONFIGURED)
		always("device role = unconfigured\n");
	else
		always("device role = %s\n", dev->device_role == DEVICE_ROLE_AGENT?"agent":"controller");
	always("distance from controller = %d\n", dev->root_distance);

	always("vendor role =%d\n", dev->vendor);
	always("supported services=%d\n", dev->supported_services);
	always("%s unassociated sta link metrics on operating channel\n", dev->ap_cap.sta_report_on_cop?"supported":"Unsupported");
	always("%s unassociated sta link metrics on not operating channel\n", dev->ap_cap.sta_report_not_cop?"supported":"Unsupported");
	always("%s Agent-initiated RSSI-based Steering\n", dev->ap_cap.rssi_steer?"supported":"Unsupported");

	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		topo_srv_dump_radio_info(radio);
	}
	SLIST_FOREACH(bss, &dev->first_bss, next_bss) {
		topo_srv_dump_bss_info(bss);
	}
	if (dev->upstream_device)
		always("upstream device = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(dev->upstream_device->_1905_info.al_mac_addr));
	else
		always("upstream device not found\n");
	SLIST_FOREACH(client, &dev->assoc_clients, next_client) {
		topo_srv_dump_client_med_info(ctx, client);
	}
	SLIST_FOREACH(conn_client, &dev->wlan_clients, next_client) {
		struct bss_info_db *bss = NULL;
		bss = topo_srv_get_bss_by_bssid(ctx, dev, conn_client->_1905_iface_addr); //removing wireless clients
		if(!bss) {
			always("1905 intf addr %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(conn_client->_1905_iface_addr));
			always("client addr %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(conn_client->client_addr));
			always("Medium: Ethernet\n");
		}
	}
	SLIST_FOREACH(neighbor, &dev->neighbors_entry, next_neighbor) {
		topo_srv_dump_neighbor_info(neighbor);
	}
	SLIST_FOREACH(neighbor, &dev->neighbors_entry, next_neighbor) {
		topo_srv_dump_bh_info(dev, neighbor);
	}
}

/**
* @brief : Fn to dump 1905 device info
*
* @param dev : 1905 device pointer
*/
int topo_srv_dump_1905_dev_info_v1(struct _1905_map_device *dev, char* reply_buf, size_t buf_len, struct own_1905_device *ctx)
{
	struct _1905_device *_1905_info = &dev->_1905_info;
	struct iface_info *iface;
	struct radio_info_db *radio;
	unsigned char exist = 0;
	char *pos, *end;
	int ret = 0;
	int cnt = 0;
	struct map_neighbor_info *neighbor = NULL;
	struct connected_clients *conn_client = NULL;

	pos = reply_buf;
	end = pos + buf_len;

	ret = os_snprintf(pos, end - pos, "{\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"AL MAC\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n", PRINT_MAC(_1905_info->al_mac_addr));
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"Device role\":\"%02x\",\n", dev->device_role);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"Distance from controller\":\"%d\",\n", dev->root_distance);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	if (dev->upstream_device != NULL) {
		ret = os_snprintf(pos, end - pos, "\"Upstream 1905 device\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n",
			PRINT_MAC(dev->upstream_device->_1905_info.al_mac_addr));
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;
	}

	ret = os_snprintf(pos, end - pos, "\"BH Interface(AP)\":[\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	SLIST_FOREACH(iface, &(dev->_1905_info.first_iface), next_iface) {
		if (iface->is_map_if == 1
			&& (iface->media_type >= ieee_802_11_b && iface->media_type <= ieee_802_11_ax)
			&& iface->media_info.role == 0x00) {
				ret = os_snprintf(pos, end - pos, "{\"MAC address\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n\"interface name\":\"%s\"},\n",
					PRINT_MAC(iface->iface_addr), "NA");
				if (ret < 0 || ret >= end - pos)
					return -1;
				pos += ret;
				exist = 1;
			}
	}

	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "],\n\"BH Interface(APCLI)\":[\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	SLIST_FOREACH(iface, &(dev->_1905_info.first_iface), next_iface) {
		if (iface->is_map_if == 1
			&& (iface->media_type >= ieee_802_11_b && iface->media_type <= ieee_802_11_ax)
			&& iface->media_info.role == 0x4) {
				ret = os_snprintf(pos, end - pos, "{\"MAC address\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n\"interface name\":\"%s\"},\n",
					PRINT_MAC(iface->iface_addr), "N/A");
				if (ret < 0 || ret >= end - pos)
					return -1;
				pos += ret;
				exist = 1;
		}
	}
	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}

	ret = os_snprintf(pos, end - pos, "],\n\"backhaul link metrics\":[");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;


	if (topo_srv_dump_bh_link_metrics(dev, &pos, end - pos) < 0)
	return -1;

	cnt = 0;
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		cnt++;
	}
	ret = os_snprintf(pos, end - pos, "],\n\"Number of radios\":\"%d\",\n", cnt);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"Radio Info\":[\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		if (topo_srv_dump_radio_info_v1(dev, radio, &pos, end - pos, ctx) < 0)
			return -1;
		exist = 1;
	}
	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "],\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"BH Info\":[\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	SLIST_FOREACH(neighbor, &dev->neighbors_entry, next_neighbor) {
		struct backhaul_link_info *bh_entry = NULL;
		if(dev->device_role == DEVICE_ROLE_CONTROLLER)
			continue;
		SLIST_FOREACH(bh_entry, &neighbor->bh_head, next_bh) {
			if(bh_entry) {
				if(topo_srv_dump_bh_link_info(dev, bh_entry, neighbor, &pos, end - pos) < 0)
					continue;
				exist = 1;
			}
		}
	}
	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "\n],\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	exist = 1;

	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, ",\n\"Other Clients Info\":[\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	SLIST_FOREACH(conn_client, &dev->wlan_clients, next_client) {
		struct bss_info_db *bss = NULL;
		bss = topo_srv_get_bss_by_bssid(ctx, dev, conn_client->_1905_iface_addr); //removing wireless clients
		if(bss == NULL) {
			exist = 1;
			ret = os_snprintf(pos, end - pos, "{\n\"1905 Intf address\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n",
				PRINT_MAC(conn_client->_1905_iface_addr));
			if (ret < 0 || ret >= end - pos)
				return -1;
			pos += ret;
			ret = os_snprintf(pos, end - pos, "\"Client Address\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n",
				PRINT_MAC(conn_client->client_addr));
			if (ret < 0 || ret >= end - pos)
				return -1;
			pos += ret;
			ret = os_snprintf(pos, end - pos, "\"Medium\": \"Ethernet\"},\n");
			if (ret < 0 || ret >= end - pos)
				return -1;
			pos += ret;
		}
	}
	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "\n],\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	exist = 1;

	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "\n},\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	return pos - reply_buf;
}

int topo_srv_dump_bh_all_info(struct own_1905_device *ctx)
{
	struct _1905_map_device *_1905_dev = topo_srv_get_next_1905_device(ctx, NULL);

	while (_1905_dev) {
		topo_srv_dump_1905_dev_bh_info(_1905_dev);
		_1905_dev = topo_srv_get_next_1905_device(ctx, _1905_dev);
	}
	return 0;
}

int topo_srv_dump_sta_all_info(struct own_1905_device *ctx)
{
	struct _1905_map_device *_1905_dev = topo_srv_get_next_1905_device(ctx, NULL);
	struct associated_clients *client = NULL;
	struct connected_clients *conn_client = NULL;
	while (_1905_dev) {
		SLIST_FOREACH(client, &_1905_dev->assoc_clients, next_client) {
			topo_srv_dump_client_med_info(ctx, client);
		}
		SLIST_FOREACH(conn_client, &_1905_dev->wlan_clients, next_client) {
			struct bss_info_db *bss = NULL;
			bss = topo_srv_get_bss_by_bssid(ctx, _1905_dev, conn_client->_1905_iface_addr); //removing wireless clients
			if(!bss) {
				always("1905 intf addr %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(conn_client->_1905_iface_addr));
				always("client addr %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(conn_client->client_addr));
				always("Medium: Ethernet\n");
			}
		}
		_1905_dev = topo_srv_get_next_1905_device(ctx, _1905_dev);
	}
	return 0;
}

/**
* @brief : Fn to dump whole topology
*
* @param ctx : own 1905 device context
*
* @return : 0 if success else -1
*/
int topo_srv_dump_topology(struct own_1905_device *ctx)
{
	struct _1905_map_device *_1905_dev = topo_srv_get_next_1905_device(ctx, NULL);

	while (_1905_dev) {
		topo_srv_dump_1905_dev_info(ctx, _1905_dev);
		_1905_dev = topo_srv_get_next_1905_device(ctx, _1905_dev);
	}
	return 0;
}

#ifdef ACL_CTRL
int topo_srv_dump_agent_info(struct own_1905_device *ctx)
{
	char buf[512] = {0};
	char *pos, *end;
	int ret = 0;
	struct iface_info *iface;
	struct _1905_map_device *_1905_dev;

	pos = &buf[0];
	end = pos + (sizeof(buf)-1);

	ret = os_snprintf(pos, end - pos, "\tagent info\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\tDevice  status     AlMac\t \tInfAddr's ");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	_1905_dev = topo_srv_get_next_1905_device(ctx, NULL);

	while (_1905_dev) {
		if (_1905_dev->device_role == DEVICE_ROLE_AGENT) {
			ret = os_snprintf(pos, end - pos, "\n\t%s  \t%s     %02x:%02x:%02x:%02x:%02x:%02x\t", "Agent",
				_1905_dev->in_network ? "Online":"Offline", PRINT_MAC(_1905_dev->_1905_info.al_mac_addr));
			if (ret < 0 || ret >= end - pos)
				return -1;
			pos += ret;
			SLIST_FOREACH(iface, &(_1905_dev->_1905_info.first_iface), next_iface) {
				if ((iface->media_type >= ieee_802_11_b && iface->media_type <= ieee_802_11_ax)
					&& iface->media_info.role == 0x4) {
						ret = os_snprintf(pos, end - pos, "%02x:%02x:%02x:%02x:%02x:%02x  ", PRINT_MAC(iface->iface_addr));
						if (ret < 0 || ret >= end - pos)
							return -1;
						pos += ret;
				}
			}
		}
		_1905_dev = topo_srv_get_next_1905_device(ctx, _1905_dev);
	}
	err("%s", buf);
	return 0;
}
#endif /*ACL_CTRL*/

#ifdef MAP_R2
int dump_de_per_dev(struct own_1905_device *ctx, struct _1905_map_device *dev, char* reply_buf, size_t buf_len)
{
	struct _1905_device *_1905_info = &dev->_1905_info;
	struct radio_info_db *radio = NULL;
	struct bss_info_db *bss = NULL;
	struct associated_clients *client = NULL;
	unsigned char exist = 0;
	char *pos, *end;
	int ret = 0;
	int cnt = 0;
	pos = reply_buf;
	end = pos + buf_len;

	ret = os_snprintf(pos, end - pos, "{\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	always("=======================\n");
	always("Data Elements info\n");
	always("almac %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(_1905_info->al_mac_addr));
	always("Metric Reporting Interval:%d\n", dev->metric_rep_interval);

	ret = os_snprintf(pos, end - pos, "\"AL MAC\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n", PRINT_MAC(_1905_info->al_mac_addr));
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Metric Reporting Interval\":\"%d\",\n", dev->metric_rep_interval);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	SLIST_FOREACH(radio, &dev->first_radio, next_radio)
		cnt++;
	ret = os_snprintf(pos, end - pos, "\"Number of radios\":\"%d\",\n", cnt);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"Radio Info\":[\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	radio = NULL;
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		if (os_memcmp(ctx->al_mac, dev->_1905_info.al_mac_addr, ETH_ALEN) == 0) {
			topo_srv_dump_radio_de_own_info(ctx, radio, &pos, end - pos);
		} else
			topo_srv_dump_radio_de_info(ctx, radio, &pos, end - pos);
		exist = 1;
	}
	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "],\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	cnt = 0;
	SLIST_FOREACH(bss, &dev->first_bss, next_bss)
		cnt++;
	ret = os_snprintf(pos, end - pos, "\"Number of Bss\":\"%d\",\n", cnt);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Bss Info\":[\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	SLIST_FOREACH(bss, &dev->first_bss, next_bss) {
		topo_srv_dump_bss_ext_info(bss, &pos, end - pos);
		exist = 1;
	}
	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "],\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	cnt = 0;
	SLIST_FOREACH(client, &dev->assoc_clients, next_client)
		cnt++;
	ret = os_snprintf(pos, end - pos, "\"Number of Clients\":\"%d\",\n", cnt);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Client Extended Info\":[\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	SLIST_FOREACH(client, &dev->assoc_clients, next_client) {
		topo_srv_dump_client_ext_metric_info(ctx, client, &pos, end - pos);
		exist = 1;
	}
	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "\n],\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	exist = 1;

	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "\n},\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	return pos - reply_buf;
}

int dump_de(struct own_1905_device *ctx, char *buf, char* reply_buf, size_t buf_Len)
{
	struct _1905_map_device *_1905_dev = topo_srv_get_next_1905_device(ctx, NULL);
	int len = 0, total_len = 0;
	char *pos, *end;
	int ret = 0;
	int exist = 0;
	pos = reply_buf;
	end = pos + buf_Len;

	ret = os_snprintf(pos, end - pos, "{\n\"Data Elements Information\":[ \n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	while (_1905_dev) {
		if (_1905_dev->in_network == 0) {
			_1905_dev = topo_srv_get_next_1905_device(ctx, _1905_dev);
			continue;
		}
		len = dump_de_per_dev(ctx, _1905_dev, pos, end - pos);
		pos += len;
		exist = 1;
		if (len < 0) {
			break;
		} else {
			if (total_len + len > buf_Len - 3) {
				break;
			}
			else
				total_len += len;
		}
		_1905_dev = topo_srv_get_next_1905_device(ctx, _1905_dev);
	}
	if (exist == 1)
		pos -= 2; // for truncating extra comma character in topology dump

	ret = os_snprintf(pos, end - pos, "]\n}\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	total_len  = pos - reply_buf;

	return total_len;
}
#endif
/**
* @brief : Fn to dump whole topology
*
* @param ctx : own 1905 device context
*
* @return : 0 if success else -1
*/
int topo_srv_dump_topology_v1(struct own_1905_device *ctx, char *buf, char* reply_buf, size_t buf_Len)
{
	struct _1905_map_device *_1905_dev = topo_srv_get_next_1905_device(ctx, NULL);
	struct _1905_map_device *_1905_dev_me = _1905_dev;
	int len = 0, total_len = 0, i = 0;
	char *pos, *end;
	int ret = 0;
	u8 start_index = 1;
	int exist = 0;
	pos = reply_buf;
	end = pos + buf_Len;

	if (os_strlen(buf) == 16)
		start_index = 1;
	else
		start_index = atoi(buf + 17);
	if (start_index == 0) {
		/*error when parse the start_index*/
		goto fail;
	}
	err("%s start_index=%d\n", __FUNCTION__, start_index);
	ret = os_snprintf(pos, end - pos, "{\n\"topology information\":[ \n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	while (_1905_dev) {
		i++;
		if (i < start_index) {
			_1905_dev = topo_srv_get_next_1905_device(ctx, _1905_dev);
			continue;
		}
		if (_1905_dev == NULL) {
			err("1905 dev NULL\n");
			break;
		}
		/*skip device not in network*/
		if (_1905_dev->in_network == 0 && _1905_dev != _1905_dev_me) {
			always("skip device %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(_1905_dev->_1905_info.al_mac_addr));
			_1905_dev = topo_srv_get_next_1905_device(ctx, _1905_dev);
			continue;
		}

	len = topo_srv_dump_1905_dev_info_v1(_1905_dev, pos, end - pos, ctx);
	exist = 1;
		if (len < 0) {
			break;
		} else {
			if (total_len + len > buf_Len - 3) {
				break;
			}
			else
				total_len += len;
		}
		_1905_dev = topo_srv_get_next_1905_device(ctx, _1905_dev);
		pos += len;
	}
	if (exist == 1)
		pos -= 2; // for truncating extra comma character in topology dump

	ret = os_snprintf(pos, end - pos, "]\n}\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

fail:
	if (_1905_dev == NULL)
		ret = 0;//removing end character
	else
		ret = os_snprintf(pos, end - pos, "%03d", i);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	total_len  = pos - reply_buf;

	return total_len;
}


/**
* @brief : Helper API to issue scan
*
* @param ctx : own 1905 device context
*
* @return : 0 if success else error
*/
void topo_srv_issue_scan(struct own_1905_device *ctx)
{
	err("topo_srv_issue_scan");
	/* TODO interface name from mapd_cli*/
	ap_selection_issue_scan(ctx);
}

/**
* @brief : helper Fn to get root address
*
* @param ctx : own 1905 device context
* @param _1905_dev : 1905 device pointer
*
* @return : root distance if success else -1
*/
int topo_srv_get_root_distance(struct own_1905_device *ctx, struct _1905_map_device *_1905_dev)
{
	if (!_1905_dev)
		_1905_dev = topo_srv_get_1905_device(ctx, NULL);

	if (!_1905_dev) {
		err("own 1905 dev is missing");
		assert(0);
		return -1;
	}

	return _1905_dev->root_distance;
}

void topo_srv_update_1905_dev_vendor(struct own_1905_device *ctx, struct _1905_map_device *_1905_device, enum map_vendor vendor)
{
	_1905_device->vendor = vendor;
}

struct associated_clients *topo_srv_get_associate_client(struct own_1905_device *ctx,
				struct _1905_map_device *dev, unsigned char *mac)
{
	struct associated_clients *client = NULL;
	if (dev == NULL)
		dev = topo_srv_get_1905_device(ctx, NULL);

	if (!dev) {
		err("failed to get 1905 device");
		return NULL;
	}
	SLIST_FOREACH(client, &dev->assoc_clients, next_client) {
		if (os_memcmp(client->client_addr, mac, ETH_ALEN) == 0)
			return client;
	}

	err("failed to get client");

	return NULL;
}

int topo_srv_get_bssid_of_sta(struct own_1905_device *ctx, struct beacon_metrics_query *bcn_query, unsigned char *bssid)
{
	struct _1905_map_device *dev = topo_srv_get_next_1905_device(ctx, NULL);
	struct associated_clients *client = NULL;

	SLIST_FOREACH(client, &dev->assoc_clients, next_client) {
		if (os_memcmp(client->client_addr, bcn_query->sta_mac, ETH_ALEN) == 0) {
			os_memcpy(bssid, client->bss->bssid, ETH_ALEN);
			info("found the bssid");
			return 0;
		}
	}

	err("failed to find bssid for associated station %02x:%02x:%02x:%02x:%02x:%02x",
		PRINT_MAC(bcn_query->sta_mac));

	return -1;
}


/**
* @brief Fn to return 5Ghz backhaul API
*
* @param ctx own 1905 device context
* @param _1905_dev 1905 device pointetr
* @param bssid bssid address to be filled
*
* @return 0 if sucess else -1
*/
int topo_srv_get_5g_bh_ap_channel(struct own_1905_device *ctx, struct _1905_map_device *_1905_dev, unsigned char *channel)
{
	struct bss_info_db *bss;

	*channel =0;
	if (!_1905_dev)
		_1905_dev = topo_srv_get_1905_device(ctx, NULL);

	if (!_1905_dev) {
		err("own 1905 dev is missing");
		return -1;
	}
	SLIST_FOREACH(bss, &_1905_dev->first_bss, next_bss) {
		if (!bss->radio) {
			err("radio for bss not found");
			return -1;
		}
#ifndef BH_BSS
#define BH_BSS (1 << (6))
#endif
		if (((bss->radio->band == BAND_5GL) || (bss->radio->band == BAND_5GH))) {	/* 5h 5l */
			os_memcpy(channel, bss->radio->channel, MAX_CHANNEL_BLOCKS);
			return 0;
		}
	}
	err("Couldnt find BH BSS");
	return 0;
}

/**
* @brief Fn to return 5Ghz backhaul API
*
* @param ctx own 1905 device context
* @param _1905_dev 1905 device pointetr
* @param bssid bssid address to be filled
*
* @return 0 if sucess else -1
*/
void topo_srv_get_5g_bh_ap_channel_by_band(struct own_1905_device *ctx, struct _1905_map_device *_1905_dev, unsigned char *channel, unsigned char band)
{
	struct bss_info_db *bss;

	*channel =0;
	if (!_1905_dev)
		_1905_dev = topo_srv_get_1905_device(ctx, NULL);


	SLIST_FOREACH(bss, &_1905_dev->first_bss, next_bss) {
		if (!bss->radio) {
			err("radio for bss not found");
			return;
		}
		if (((bss->radio->band == band))) {
			os_memcpy(channel, bss->radio->channel, MAX_CHANNEL_BLOCKS);
			return;
		}
	}
	return;
}


/**
* @brief Fn to return 2.4Ghz backhaul API
*
* @param ctx own 1905 device context
* @param _1905_dev 1905 device pointetr
* @param bssid bssid address to be filled
*
* @return 0 if sucess else -1
*/
int topo_srv_get_2g_bh_ap_channel(struct own_1905_device *ctx, struct _1905_map_device *_1905_dev, unsigned char *channel)
{
	struct bss_info_db *bss;

	*channel = 0;
	if (!_1905_dev)
		_1905_dev = topo_srv_get_1905_device(ctx, NULL);

	if (!_1905_dev) {
		err("own 1905 dev is missing");
		return -1;
	}
	SLIST_FOREACH(bss, &_1905_dev->first_bss, next_bss) {
		if (!bss->radio) {
			err("radio for bss not found");
			return -1;
		}
		if ((bss->radio->band == BAND_2G)) {
			*channel = bss->radio->channel[0];
			return 0;
		}
	}
	return 0;
}


/**
* @brief Fn to return 2.4Ghz backhaul API
*
* @param ctx own 1905 device context
* @param _1905_dev 1905 device pointetr
* @param bssid bssid address to be filled
*
* @return 0 if sucess else -1
*/
int topo_srv_get_2g_bh_ap_bssid(struct own_1905_device *ctx, struct _1905_map_device *_1905_dev, unsigned char *bssid)
{
	struct bss_info_db *bss;
	unsigned char zero_bssid[ETH_ALEN] = { 0 };

	memcpy(bssid, zero_bssid, ETH_ALEN);
	if (!_1905_dev)
		_1905_dev = topo_srv_get_1905_device(ctx, NULL);

	if (!_1905_dev) {
		err("own 1905 dev is missing");
		return -1;
	}
	SLIST_FOREACH(bss, &_1905_dev->first_bss, next_bss) {
		if (!bss->radio) {
			err("radio for bss not found");
			return -1;
		}
		if ((bss->map_vendor_extn & BH_BSS) &&	/* backhaul ap */
		    (bss->radio->band == BAND_2G)) {
			memcpy(bssid, bss->bssid, ETH_ALEN);
			return 0;
		}
	}

	return 0;
}

/**
* @brief Fn to return peer relation of a 1905 device from us
*
* @param ctx own 1905 device context
* @param almac other 1905 device almac
*
* @return peer relation if found else -1
*/
int topo_srv_get_peer_relation(struct own_1905_device *ctx, unsigned char *almac)
{
	struct _1905_map_device *tmp_dev, *dev;

	dev = topo_srv_get_1905_device(ctx, NULL);
	tmp_dev = topo_srv_get_1905_device(ctx, almac);
	if (!tmp_dev) {
		err("device not found");
		return -1;
	}
	if (tmp_dev->root_distance < dev->root_distance)
		return 0;
	if (tmp_dev->root_distance == dev->root_distance)
		return 1;

	return 2;
}

/**
* @brief Fn to update 1905 upstream device
*
* @param dev 1905 device pointer
*/
static void topo_srv_update_dev_upstream_device(struct _1905_map_device *dev)
{
	struct map_neighbor_info *neighbor;

	if (dev->device_role == DEVICE_ROLE_CONTROLLER)
		return;

	SLIST_FOREACH(neighbor, &dev->neighbors_entry, next_neighbor) {
		if (neighbor && neighbor->neighbor &&
			(neighbor->neighbor->root_distance < dev->root_distance)) {
			dev->upstream_device = neighbor->neighbor;
			break;
		}
	}
}

unsigned char topo_srv_wifi_uplink_exist(
		struct own_1905_device *ctx,
		struct _1905_map_device *dev,
		struct iface_info **uplink_iface,
		unsigned char *uplink_bssid)
{

	struct iface_info *iface = NULL;
	SLIST_FOREACH(iface, &dev->_1905_info.first_iface,
			next_iface)
	{
		if (iface->media_type < IEEE802_11_GROUP ||
				iface->media_type >= IEEE1901_GROUP
		   ) {
			continue;
		} else {
			if (*(((char *)(&iface->media_info)) + 6) == 0x40) {
				if (!is_zero_ether_addr(iface->media_info.network_membership))
				{
					os_memcpy(uplink_bssid,
							iface->media_info.network_membership,
							MAC_ADDR_TLV_LENGTH);
					*uplink_iface = iface;
					info("Uplink BSSID for %02x:%02x:%02x:%02x:%02x:%02x",
							PRINT_MAC(dev->_1905_info.al_mac_addr));
					info("is %02x:%02x:%02x:%02x:%02x:%02x\n",
							PRINT_MAC(uplink_bssid));
					return TRUE;
				}
			} else {
				continue;
			}
		}
	}
	err("wifi uplink does not exist for %02x:%02x:%02x:%02x:%02x:%02x\n",
			PRINT_MAC(dev->_1905_info.al_mac_addr));
	return FALSE;
}

void mapd_update_bss_info(struct mapd_global *global, struct wireless_setting * bss_config)
{
	struct mapd_bss *bss = mapd_get_bss_from_mac(global, bss_config->mac_addr);

	if(bss  == NULL) {
		/*update this bss info later in WAPP_OPERBSS_REPORT*/
		//mapd_ASSERT(0);
		return;
	}
	bss->ssid_len = 0;
	os_memset((char *)bss->ssid, 0, 32);
	os_snprintf((char *)bss->ssid,  sizeof(bss->ssid), "%s", (char *)bss_config->Ssid);
	bss->ssid_len = os_strlen((char *)bss_config->Ssid);
}

void topo_srv_update_1905_radio_capinfo(void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *ctx = (struct own_1905_device *)eloop_ctx;
 	struct radio_info_db *radio = (struct radio_info_db *)timeout_ctx;
	wlanif_get_op_chan_info(ctx->back_ptr);
	topo_srv_get_radio_capinfo(ctx, radio->identifier);
}

/**
* @brief update wireless settings of every bss as provided in msg
*
* @param ctx own 1905 device context
* @param msg_buf msg came from controller
* @param len msg len
*
* @return 0 if success else -1
*/
int topo_srv_update_wireless_setting(struct own_1905_device *ctx, unsigned char *msg_buf, int len)
{
	int i = 0;
	struct bss_info_db *bss = NULL;
	struct wsc_config *totol_conf = NULL;
	struct wireless_setting *bss_config = NULL;
	struct _1905_map_device *own_dev = topo_srv_get_next_1905_device(ctx, NULL);
	totol_conf = (struct wsc_config *)msg_buf;
	for (i = 0; i < totol_conf->num; i++) {
		bss_config = &totol_conf->setting[i];
		bss = topo_srv_get_bss_by_bssid(ctx, own_dev, bss_config->mac_addr);
		if (bss) {
			topo_srv_update_bss_info(bss, bss_config);
			mapd_update_bss_info(ctx->back_ptr, bss_config);
			if (bss->radio)
				bss->radio->is_configured = TRUE;
		}
	}
	topo_srv_update_dev_upstream_device(own_dev);
	return 0;
}

extern u8 ZERO_MAC_ADDR[ETH_ALEN];
/**
* @brief Fn to check if all interfaces of a device are mapped to a radio
* @param ctx own 1905 device context
* @param dev device for radio check is to be made
*/
void topo_srv_check_all_iface_mapped(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{
	struct iface_info *iface = NULL;

	if (dev->radio_mapping_completed)
	{
		return;
	}
	SLIST_FOREACH(iface, &(dev->_1905_info.first_iface), next_iface) {
		if (*(((char *)(&iface->media_info)) + 6) == 0x0 &&
			(iface->media_type >= IEEE802_11_GROUP) &&
			(iface->media_type < IEEE1901_GROUP) )
		{
			if (iface->radio != NULL) {
				info("Radio mapping completed for AP interface %02x:%02x:%02x:%02x:%02x:%02x",
					PRINT_MAC(iface->iface_addr));
				continue;
			} else {
				info("radio mapping pending for AP interface %02x:%02x:%02x:%02x:%02x:%02x",
					PRINT_MAC(iface->iface_addr));
				return;
			}
		} else if (*(((char *)(&iface->media_info)) + 6) == 0x40){
			if (os_memcmp(iface->media_info.network_membership,
				ZERO_MAC_ADDR, ETH_ALEN)) {
				if (iface->radio != NULL) {
					info("radio mapping completed for CLI interface %02x:%02x:%02x:%02x:%02x:%02x",
						PRINT_MAC(iface->iface_addr));
					continue;
				}
				else {
					info("radio mapping pending for CLI interface %02x:%02x:%02x:%02x:%02x:%02x",
						PRINT_MAC(iface->iface_addr));
					return;
				}
			}
		}
	}
	info("Radio mapping complete for %02x:%02x:%02x:%02x:%02x:%02x",
		PRINT_MAC(dev->_1905_info.al_mac_addr));
	dev->radio_mapping_completed = TRUE;
}


/**
* @brief Fn to gracefully remove 1905 device off network
* @param ctx own 1905 device context
* @param dev 1905 dev to be removed
*/
void topo_srv_clear_ch_planning_states (
	struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{
	struct radio_info_db *radio = NULL;
	struct radio_ch_prefer *ch_prefer = NULL;
	struct prefer_info_db *prefer_info_t = NULL;
	struct prefer_info_db *prefer_info_tmp = NULL;
	struct iface_info *iface = NULL;

	dev->channel_planning_completed = FALSE;
	dev->ch_preference_available = FALSE;
	dev->radio_mapping_completed = FALSE;
	err("for %02x:%02x:%02x:%02x:%02x:%02x",
		PRINT_MAC(dev->_1905_info.al_mac_addr));
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		err("Radio Id: %02x:%02x:%02x:%02x:%02x:%02x  Channel0:%d",
				PRINT_MAC(radio->identifier), radio->channel[0]);

		/*Fix: remove operating channel instead of channel[0], as Operating channel is filled in operating list*/
		if (radio->operating_channel != NULL) {
			err("Oper_Chnl:%d", radio->operating_channel->ch_num);
			ch_planning_remove_ch_from_operating_list(ctx, radio->operating_channel->ch_num,
				radio, radio->operating_channel->ch_num > 14);
		}
		ch_prefer = &radio->chan_preferance;
		if (!SLIST_EMPTY(&(ch_prefer->prefer_info_head))) {
			prefer_info_t = SLIST_FIRST(&(ch_prefer->prefer_info_head));
			while (prefer_info_t) {
				int i;
				for (i = 0; i < prefer_info_t->ch_num; i++) {
					ch_planning_remove_ch_from_prefered_list(
						ctx,
						prefer_info_t->ch_list[i],
						radio,
						prefer_info_t->ch_list[i] > 14);
				}
				prefer_info_tmp = SLIST_NEXT(prefer_info_t, prefer_info_entry);
				SLIST_REMOVE(&(ch_prefer->prefer_info_head), prefer_info_t, prefer_info_db, prefer_info_entry);
				free(prefer_info_t);
				prefer_info_t = prefer_info_tmp;
			}
		}
		radio->operating_channel = NULL;
		radio->uplink_bh_present = FALSE;

	}

	SLIST_FOREACH(iface, &(dev->_1905_info.first_iface), next_iface) {
		iface->radio = NULL;
	}
}

void topo_srv_ch_planning_on_agents (
	struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{
	struct map_neighbor_info *neighbor = NULL, *neighbor_tmp = NULL;
	struct backhaul_link_info *bh = NULL, *bh_tmp = NULL;

	if (ctx->device_role != DEVICE_ROLE_CONTROLLER ||
		!ctx->ch_planning.ch_planning_enabled)
		return;

	if (dev->device_role == DEVICE_ROLE_AGENT) {
		topo_srv_clear_ch_planning_states (ctx, dev);
		return;
	}

	if (dev->device_role != DEVICE_ROLE_CONTROLLER) {
		err("device-"MACSTR" role(%d) err",
			PRINT_MAC(dev->_1905_info.al_mac_addr), dev->device_role);
		return;
	}

	neighbor = SLIST_FIRST(&dev->neighbors_entry);
	while(neighbor) {
		neighbor_tmp = SLIST_NEXT(neighbor, next_neighbor);
		if (neighbor->insert_new_link) {
			topo_srv_clear_ch_planning_states (ctx, neighbor->neighbor);
		} else {
			bh = SLIST_FIRST(&neighbor->bh_head);
			while(bh) {
				bh_tmp = SLIST_NEXT(bh, next_bh);
				if (bh->is_valid == FALSE) {
					topo_srv_clear_ch_planning_states (ctx, neighbor->neighbor);
					break;
				}
				bh = bh_tmp;
			}
		}
		neighbor = neighbor_tmp;
	}
}

void topo_srv_move_1905_off_nw (
	struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{
	struct radio_info_db *radio = NULL;
	struct radio_ch_prefer *ch_prefer = NULL;
	struct prefer_info_db *prefer_info_t = NULL;
	struct prefer_info_db *prefer_info_tmp = NULL;
	struct iface_info *iface = NULL;


	dev->in_network = FALSE;
	dev->channel_planning_completed = FALSE;
	dev->ch_preference_available = FALSE;
	dev->radio_mapping_completed = FALSE;
	info("Mark device %02x:%02x:%02x:%02x:%02x:%02x off Network",
		PRINT_MAC(dev->_1905_info.al_mac_addr));
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		/*Fix: remove operating channel instead of channel[0], as Operating channel is filled in operating list*/
		if (radio->operating_channel != NULL)
			ch_planning_remove_ch_from_operating_list(ctx, radio->operating_channel->ch_num,
				radio, radio->operating_channel->ch_num > 14);

		ch_prefer = &radio->chan_preferance;
		if (!SLIST_EMPTY(&(ch_prefer->prefer_info_head))) {
			prefer_info_t = SLIST_FIRST(&(ch_prefer->prefer_info_head));
			while (prefer_info_t) {
				int i;
				for (i = 0; i < prefer_info_t->ch_num; i++) {
					ch_planning_remove_ch_from_prefered_list(
						ctx,
						prefer_info_t->ch_list[i],
						radio,
						prefer_info_t->ch_list[i] > 14);
				}
				prefer_info_tmp = SLIST_NEXT(prefer_info_t, prefer_info_entry);
				SLIST_REMOVE(&(ch_prefer->prefer_info_head), prefer_info_t, prefer_info_db, prefer_info_entry);
				free(prefer_info_t);
				prefer_info_t = prefer_info_tmp;
			}
		}
		radio->operating_channel = NULL;
		radio->uplink_bh_present = FALSE;

	}

	SLIST_FOREACH(iface, &(dev->_1905_info.first_iface), next_iface) {
		iface->radio = NULL;
	}
}
/**
* @brief Fn to remove 1905 neighbours off network recursively
* @param ctx own 1905 device context
* @param dev target 1905 whose neighbours are to be removed
*/

void topo_serv_remove_remote_peers_recurse(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct _1905_map_device *exclude_dev)
{
	struct map_neighbor_info *neighbor = NULL;
	struct map_neighbor_info *neighbor_temp = NULL;
	SLIST_FOREACH(neighbor, &dev->neighbors_entry, next_neighbor)
	{
		if (neighbor->neighbor && neighbor->neighbor != exclude_dev) {
			topo_serv_remove_remote_peers_recurse(ctx, neighbor->neighbor, dev);
			topo_srv_move_1905_off_nw(ctx, neighbor->neighbor);
		} else {
			if (neighbor->neighbor)
				info("exclude %02x:%02x:%02x:%02x:%02x:%02x",
					PRINT_MAC(neighbor->neighbor->_1905_info.al_mac_addr));
		}
		neighbor_temp = SLIST_NEXT(neighbor, next_neighbor);
		SLIST_REMOVE(&dev->neighbors_entry, neighbor, map_neighbor_info, next_neighbor);
		if (neighbor->neighbor) {
			info("remove %02x:%02x%02x:%02x:%02x:%02x",
				PRINT_MAC(neighbor->neighbor->_1905_info.al_mac_addr));
			info("From neighbour list of %02x:%02x:%02x:%02x:%02x:%02x",
				PRINT_MAC(dev->_1905_info.al_mac_addr));
		}
		os_free(neighbor);
		neighbor = neighbor_temp;
		if (neighbor == NULL)
			break;
	}
}

/**
* @brief Fn to remove 1905 neighbours off network recursively
* @param ctx own 1905 device context
* @param dev target 1905 whose neighbours are to be removed
*/

void topo_serv_update_remote_peers(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	char *change)
{
	struct map_neighbor_info *neighbor = NULL, *neighbor_tmp = NULL;
	struct backhaul_link_info *bh = NULL, *bh_tmp = NULL;
#ifdef MAP_R2
	struct mapd_global *global = ctx->back_ptr;
#endif

	if (SLIST_EMPTY(&dev->neighbors_entry)) {
		err("no Neighbors for %02x:%02x:%02x:%02x:%02x:%02x",
					PRINT_MAC(dev->_1905_info.al_mac_addr));
		return;
	}
	debug("Check Neighbors for %02x:%02x:%02x:%02x:%02x:%02x",
					PRINT_MAC(dev->_1905_info.al_mac_addr));


	neighbor = SLIST_FIRST(&dev->neighbors_entry);
	while(neighbor) {
		neighbor_tmp = SLIST_NEXT(neighbor, next_neighbor);
		/*do not use the topology rsp of my neighbor device to update all the link related
		 *to me. it may be incorrect due to the asynchronous updating neighbor time between
		 *two directly connected device
		 */
		if (!os_memcmp(neighbor->n_almac, ctx->al_mac, ETH_ALEN)) {
			debug("skip neighbor(%02x:%02x:%02x:%02x:%02x:%02x) "
				"from dev(%02x:%02x:%02x:%02x:%02x:%02x)",
				PRINT_MAC(neighbor->n_almac),
				PRINT_MAC(dev->_1905_info.al_mac_addr));
			neighbor = neighbor_tmp;
			continue;
		}
		if (neighbor->insert_new_link) {
			*change = 1;
			/* 1905 device connected reset the state in 1905 devices for Network Optmization*/
			ntwrk_opt_device_conn_disconnect_handle(ctx, DEVICE_CONNECT, neighbor->neighbor);
#ifdef MAP_R2
			if(ctx->device_role == DEVICE_ROLE_CONTROLLER) {
				err("r2_ch_st: %d, r1_ch_st: %d\n",
					global->dev.ch_planning_R2.ch_plan_state, ctx->ch_planning.ch_planning_state);
				if ((global->dev.ch_planning_R2.ch_plan_state != CHPLAN_STATE_CAC_ONGOING) &&
						(ctx->ch_planning.ch_planning_state != CHANNEL_PLANNING_CAC_START)) {
					ch_planning_R2_reset(ctx,NULL);
					handle_task_completion(ctx);
				}
				if (eloop_is_timeout_registered(ch_planning_R2_bootup_handling_restart, (void *)&global->dev,NULL)) {
					eloop_cancel_timeout(ch_planning_R2_bootup_handling_restart, (void *)&global->dev, NULL);
					eloop_register_timeout(CH_SCAN_RETRIGGER_TIMEOUT, 0, ch_planning_R2_bootup_handling_restart, (void *)&global->dev, NULL);
				}
			}
#endif
			if(ctx->device_role == DEVICE_ROLE_CONTROLLER) {
				ctx->ch_planning.need_restart_ch_plan = 1;
			}
		}

		bh = SLIST_FIRST(&neighbor->bh_head);
		while(bh) {
			bh_tmp = SLIST_NEXT(bh, next_bh);
			if (bh->is_valid == FALSE) {
				*change = 1;
				ntwrk_opt_device_conn_disconnect_handle(ctx, DEVICE_DISCONNECT,  neighbor->neighbor);
#ifdef MAP_R2
				if(ctx->device_role == DEVICE_ROLE_CONTROLLER) {
					err("r2_ch_st: %d, r1_ch_st: %d\n",
						global->dev.ch_planning_R2.ch_plan_state, ctx->ch_planning.ch_planning_state);
					if ((global->dev.ch_planning_R2.ch_plan_state != CHPLAN_STATE_CAC_ONGOING) &&
						(ctx->ch_planning.ch_planning_state != CHANNEL_PLANNING_CAC_START) &&
						(global->dev.ch_planning_R2.ch_plan_state != CHPLAN_STATE_SCAN_ONGOING)) {
						ch_planning_R2_reset(ctx,NULL);
						handle_task_completion(ctx);
					}
				}
#endif
				SLIST_REMOVE(&neighbor->bh_head, bh, backhaul_link_info, next_bh);
				err("need remove link "
						"connected_iface_addr(%02x:%02x:%02x:%02x:%02x:%02x) "
						"neighbor_iface_addr(%02x:%02x:%02x:%02x:%02x:%02x) "
						"from dev(%02x:%02x:%02x:%02x:%02x:%02x)",
						PRINT_MAC(bh->connected_iface_addr),
						PRINT_MAC(bh->neighbor_iface_addr),
						PRINT_MAC(dev->_1905_info.al_mac_addr));
				/*need remove deleted link of the peer*/
				topo_serv_remove_delete_link_peers(bh, dev, neighbor->neighbor);
				os_free(bh);
			}
			bh = bh_tmp;
		}
		if (SLIST_EMPTY(&neighbor->bh_head)) {
			SLIST_REMOVE(&dev->neighbors_entry, neighbor, map_neighbor_info, next_neighbor);
			err("need remove %02x:%02x:%02x:%02x:%02x:%02x from network",
				PRINT_MAC(neighbor->n_almac));
			/*need remove my link of the peer*/
			topo_serv_remove_neighbor_peers(dev, neighbor->neighbor);
			os_free(neighbor);
		} else {
			debug("%02x:%02x:%02x:%02x:%02x:%02x still in network",
               PRINT_MAC(neighbor->n_almac));
		}
		neighbor = neighbor_tmp;
	}
}





/*
	Field	Length	Value	Description
	Type	2 octets	_1905_SET_CHANNEL_SETTING _EVENT
	Length	2 octets	m	 Value length of bytes
		6 octets	Any EUI-48 value
	Value	Zero or more Channel Preference TLVs
		Zero or more Transmit Power Limit TLVs
*/
unsigned char find_oper_channel(unsigned char op_class,
	unsigned char ch_num, unsigned char *ch_list)
{
	struct global_oper_class *opcls = oper_class;
	int i = 1; /* skip Invlid entry */
	int j = 0, k = 0;
	unsigned char channel = 0;

	do {
		if (opcls[i].opclass == op_class) {
			for (j = 0; j < opcls[i].channel_num; j++) {
				channel = opcls[i].channel_set[j];
				for (k = 0; k < ch_num; k++) {
					if (channel == ch_list[k])
						break;
				}
				if (k == ch_num) {
					return channel;
				}
			}
			break;
		}
		i++;
	} while (opcls[i].opclass != 0);

	return 0;
}

unsigned char find_first_oper_channel(unsigned char op_class)
{
	struct global_oper_class *opcls = oper_class;
	int i = 1; /* skip Invlid entry */
	unsigned char channel = 0;

	do {
		if (opcls[i].opclass == op_class) {
			channel = opcls[i].channel_set[0];
			return channel;
		}
		i++;
	} while (opcls[i].opclass != 0);

	return 0;
}

unsigned char find_first_oper_channel_from_operclass(unsigned char op_class, struct ch_pref_sub *chan_pref)
{
	struct global_oper_class *opcls = oper_class;
	int i = 1; /* skip Invlid entry */
	int j;
	int k;
	int found  = 0;

	do {
		if (opcls[i].opclass == op_class) {
			for (j = 0; j < opcls[i].channel_num; j++) {
				for (k = 0; k < chan_pref->ch_num; k++) {
					/* TODO preference check may be */
					if ( chan_pref->chan_list[k].channel == opcls[i].channel_set[j]) {
						found = 1;
						break;
					}
				}
				if (!found)
					return opcls[i].channel_set[j];
				found = 0;
			}
		}
		i++;
	} while (opcls[i].opclass != 0);

	return 0;
}

int _1905_2_wapp_cert_channel_setting_event(struct own_1905_device *ctx, unsigned char *pkt, unsigned int pkt_len)
{
	struct ch_sel_rsp_info rsp_info[MAX_CH_NUM];
	unsigned char *ptr = pkt, *tmp;
	struct channel_setting setting;
	struct ch_config_info *chinfo= NULL;
	struct ch_pref_tlv ch_pref[MAX_CH_NUM];
	struct ch_config_info tran_power[MAX_CH_NUM];
	unsigned char *buf = NULL;
	struct cmd_to_wapp *wapp_msg = NULL;
	int invalid_ch_found = 0;
	unsigned char i = 0;
	unsigned char j = 0;
	unsigned char k = 0, chan_count = 0;
	int ret = 0;
	int body_len = 0;
	unsigned char tlv_type = 0;
	unsigned char ch_pref_cnt = 0;
	unsigned char tran_power_cnt = 0;
	unsigned short ev_len = 0;
	unsigned char preference, reason_code, best_prefered_chan, best_prefered_opclass, max_preferred;
	u8 primary_ch = 0;
	u8 primary_ch_op_class = 0;
	u8 primary_ch_reason_code = 0;
	u8 primary_ch_pref = 0;
	unsigned char primary_ch_identifier[ETH_ALEN] = {0};
	int oper_class_cnt, channel;
	struct mapd_global *ptrmapd = (struct mapd_global *)ctx->back_ptr;
	os_memset(&ch_pref, 0, sizeof(ch_pref));
	/*
		The al mac address of the 1905 device from which this message is received
	*/
	if ((pkt_len -(ptr - pkt)) < ETH_ALEN) {
		err("check packet size fail\n");
		return -1;
	}
	buf = os_zalloc(512);
	os_memcpy(setting.almac, ptr, ETH_ALEN);
	ptr += ETH_ALEN;

	debug("al mac %2x:%02x:%02x:%02x:%02x:%02x"
		, PRINT_MAC(setting.almac));

	while ((ptr - pkt) < pkt_len) {
		tlv_type = (*ptr);
		ptr += 1;
		ev_len = *((unsigned short *)ptr);
		ev_len = ntohs(ev_len);
		ptr += sizeof(unsigned short);
		debug("tlv_type %02x ev_len %d"
			, tlv_type, ev_len);

		switch (tlv_type) {
			case CH_PREFERENCE_TYPE:
				/*
					17.2.13 Channel Preference TLV format
					1 octet:	Channel Preference TLV.
					2 octets:	Number of octets in ensuing tlvValue field.
					6 octets:	Radio Unique identifier of a radio for which channel preferences are reported.
					1 octet:	Number of operating classes for which preferences are reported in this TLV.
					1 octet:	Operating Class contains an enumerated value from Table E-4 in Annex E of [1],
							specifying the global operating class in which the subsequent Channel List is valid.
					1 octet:	Number of channels specified in the Channel List
					k octets:	Channel List
							Contains a variable number of octets.
							Each octet describes a single channel number in the Operating Class.
							An empty Channel List field (k=0) indicates that
							the indicated Preference applies to all channels in the Operating Class.
					bits 7-4:	Preference.
							Indicates a preference value for the channels in the Channel List.
					bits 3-0:	Reason Code.
							Indicates the reason for the Preference
				*/

				os_memcpy(ch_pref[ch_pref_cnt].identifier, ptr, ETH_ALEN);
				ptr += ETH_ALEN;

				oper_class_cnt = (*ptr);
				ptr += 1;
				for(j = 0; j < oper_class_cnt; j++) {
					for (i = 0; i < ch_pref[ch_pref_cnt].op_class_cnt; i++) {
						if (ch_pref[ch_pref_cnt].opclass[i].op_class == (*ptr))
							break;
					}
					/* now the i pointer have the correct one */

					/* reassignment/assignment */
					if (ch_pref[ch_pref_cnt].opclass[i].op_class != (*ptr)) {
						ch_pref[ch_pref_cnt].opclass[i].op_class = (*ptr);
						ch_pref[ch_pref_cnt].op_class_cnt++;
					}

					ptr += 1;

					/* chan number in the lisr */
					chan_count = (*ptr);
					ptr += 1;

					tmp = ptr;
					tmp += chan_count;
					preference = ((*tmp)&0xF0) >> 4;
					reason_code = ((*tmp)&0x0F);
					for (k = 0; k < chan_count; k++) {

						channel = (*ptr);
						ch_pref[ch_pref_cnt].opclass[i].chan_list[ch_pref[ch_pref_cnt].opclass[i].ch_num + k].channel = channel;
						ptr += 1;
						ch_pref[ch_pref_cnt].opclass[i].chan_list[ch_pref[ch_pref_cnt].opclass[i].ch_num + k].preference = preference;
						ch_pref[ch_pref_cnt].opclass[i].chan_list[ch_pref[ch_pref_cnt].opclass[i].ch_num + k].reason_code = reason_code;
						debug("reason code: %d", reason_code);
					}
					ch_pref[ch_pref_cnt].opclass[i].ch_num = ch_pref[ch_pref_cnt].opclass[i].ch_num + chan_count;
					/* preference + reason code */
					ptr += 1;
				}
				err("tlv type %02x identifier %02x:%02x:%02x:%02x:%02x:%02x op class cnt %d"
					, tlv_type
					, PRINT_MAC(ch_pref[ch_pref_cnt].identifier)
					, ch_pref[ch_pref_cnt].op_class_cnt);
				ch_pref_cnt ++;
				break;
			case TRANSMIT_POWER_LIMIT_TYPE:
				/*
					17.2.15 Transmit Power Limit TLV format
					1 octet:	Transmit Power Limit TLV.
					2 octets(7):	Number of octets in ensuing tlvValue filed.
					6 octets:	Radio Unique identifier
					1 octet:	Transmit Power Limit
				*/
				os_memcpy(tran_power[tran_power_cnt].identifier, ptr, ETH_ALEN);
				ptr += ETH_ALEN;

				tran_power[tran_power_cnt].power = (*ptr);
				ptr += 1;

				debug("tlv type %02x identifier %2x:%02x:%02x:%02x:%02x:%02x power %d"
					, tlv_type
					, PRINT_MAC(tran_power[tran_power_cnt].identifier)
					, tran_power[tran_power_cnt].power);

				tran_power_cnt ++;
				break;
			default:
				break;
		}

	}

	if (ch_pref_cnt == 0) {
		wlanif_get_op_chan_info(ctx->back_ptr);
		os_free(buf);
		return 0;
	}
	if (ptrmapd->params.Certification)
		setting.ch_set_num = (tran_power_cnt > ch_pref_cnt)?ch_pref_cnt:tran_power_cnt;
	else
		setting.ch_set_num = ch_pref_cnt;
	// construct msg here
	body_len = sizeof(struct channel_setting) + setting.ch_set_num * sizeof(struct ch_config_info);
	wapp_msg = (struct cmd_to_wapp *)buf;


	debug("Al mac address of 1905.1 device %2x:%02x:%02x:%02x:%02x:%02x Number of channel info %d"
		, PRINT_MAC(setting.almac), setting.ch_set_num);
	os_memset(wapp_msg, 0, sizeof(struct cmd_to_wapp));
	chinfo = (struct ch_config_info *)((char *)wapp_msg + sizeof(struct cmd_to_wapp) + sizeof(struct channel_setting));

	for (i = 0; i < setting.ch_set_num; i++) {
		invalid_ch_found = 0;
		if (ptrmapd->params.Certification) {
			// 6 octets Radio identity
			os_memcpy(chinfo->identifier, tran_power[i].identifier, ETH_ALEN);
			os_memcpy(rsp_info[i].radio_indentifier, tran_power[i].identifier, ETH_ALEN);
		} else {
			// 6 octets Radio identity
			os_memcpy(chinfo->identifier, ch_pref[i].identifier, ETH_ALEN);
			os_memcpy(rsp_info[i].radio_indentifier, ch_pref[i].identifier, ETH_ALEN);
		}
		rsp_info[i].rsp_code = 0x00;
		for(j = 0; j < ch_pref[i].op_class_cnt; j++) {
			if (!check_invalid_channel(ch_pref[i].opclass[j].op_class,
					ch_pref[i].opclass[j].ch_num, ch_pref[i].opclass[j].chan_list)){
				err("opclass=%d", ch_pref[i].opclass[j].op_class);
				rsp_info[i].rsp_code = 0x02;
				invalid_ch_found = 1;
				break;
			}
		}
		if(invalid_ch_found == 0)
		{
#if 0
			for(j = 0; j < ch_pref[i].op_class_cnt; j++) {
				if (ch_pref[i].opclass[j].ch_num == 0 && ch_pref[i].opclass[j].preference == 0) {
					debug("all channels of op_class(%d) are non-operable",
							ch_pref[i].opclass[j].op_class);
				}
				else if (ch_pref[i].opclass[j].ch_num != 0 && ch_pref[i].opclass[j].preference == 0) {
					/*find an operable channel from global operating class*/
					chinfo->op_class = ch_pref[i].opclass[j].op_class;
					debug("some channels of op_class(%d) are non-operable",
							ch_pref[i].opclass[j].op_class);
					mapd_hexdump(MSG_WARNING,"invalid channel list", ch_pref[i].opclass[j].ch_list, ch_pref[i].opclass[j].ch_num);
					chinfo->channel = find_oper_channel(ch_pref[i].opclass[j].op_class,
							ch_pref[i].opclass[j].ch_num, ch_pref[i].opclass[j].ch_list);
					break;
				}
				else if (ch_pref[i].opclass[j].ch_num == 0 && ch_pref[i].opclass[j].preference != 0) {
					/*find first channel from global operating class*/
					chinfo->op_class = ch_pref[i].opclass[j].op_class;
					chinfo->channel = find_first_oper_channel(ch_pref[i].opclass[j].op_class);
					break;
				}
				else {
					/*find first channel from channel list*/
					chinfo->op_class = ch_pref[i].opclass[j].op_class;
					chinfo->channel= ch_pref[i].opclass[j].ch_list[0];
					break;
				}
			}
#endif
			/* find best prefered channel for this radio */
			best_prefered_chan = 0;
			best_prefered_opclass = 0;
			max_preferred = 0;
			reason_code = 0;
			u8  bw_curr = BW_20, curr_op_bw = BW_20;
			struct radio_info_db *radio = NULL;
			struct _1905_map_device * _1905_device = topo_srv_get_1905_device(ctx,NULL);
			radio = topo_srv_get_radio(_1905_device,ch_pref->identifier);
			if(!radio){
				err("radio is NULL");
			} else {
				curr_op_bw = chan_mon_get_bw_from_op_class(radio->operating_class);
				err("current operating BW %d, opclass %d", curr_op_bw, radio->operating_class);
			}
			debug("opclass_cnt: %u\n", ch_pref[i].op_class_cnt);
			for(j = 0; j < ch_pref[i].op_class_cnt; j++) {
				for (k = 0; k < ch_pref[i].opclass[j].ch_num; k++) {
					if (max_preferred < ch_pref[i].opclass[j].chan_list[k].preference) {
						debug("found new prefered =%d, channel=%d, opclass=%d",
							ch_pref[i].opclass[j].chan_list[k].preference,
							ch_pref[i].opclass[j].chan_list[k].channel, ch_pref[i].opclass[j].op_class);
						max_preferred = ch_pref[i].opclass[j].chan_list[k].preference;
						best_prefered_chan = ch_pref[i].opclass[j].chan_list[k].channel;
						best_prefered_opclass = ch_pref[i].opclass[j].op_class;
						reason_code = ch_pref[i].opclass[j].chan_list[k].reason_code;
						debug("reason code: %d", reason_code);
					}
					else if (max_preferred == ch_pref[i].opclass[j].chan_list[k].preference) {
						bw_curr = chan_mon_get_bw_from_op_class(ch_pref[i].opclass[j].op_class);
						if(bw_curr == curr_op_bw) {
							max_preferred = ch_pref[i].opclass[j].chan_list[k].preference;
							best_prefered_chan = ch_pref[i].opclass[j].chan_list[k].channel;
							best_prefered_opclass = ch_pref[i].opclass[j].op_class;
							reason_code = ch_pref[i].opclass[j].chan_list[k].reason_code;
							debug("At agent modify max pref based on bw since same pref opclass %d ch%d",
								best_prefered_opclass,
								best_prefered_chan);
						}
					}

					/* Using i+1 index for storing primary CH info at setting->chinfo  */
					if ((primary_ch == 0) && ((ch_pref[i].opclass[j].op_class == 128)
#ifdef MAP_160BW
						|| (ch_pref[i].opclass[j].op_class == 129)
#endif
						) && (j < (ch_pref[i].op_class_cnt - 1))) {
						primary_ch = ch_pref[i].opclass[j+1].chan_list[0].channel;
						primary_ch_pref = ch_pref[i].opclass[j+1].chan_list[0].preference;

						if ((is_valid_primary_ch_80M_160M(primary_ch, ch_pref[i].opclass[j].chan_list[0].channel,
									ch_pref[i].opclass[j].op_class)) &&
							(primary_ch_pref == ch_pref[i].opclass[j].chan_list[0].preference)) {
                                			primary_ch_op_class = ch_pref[i].opclass[j+1].op_class;
                                			os_memcpy(primary_ch_identifier, ch_pref[i].identifier, ETH_ALEN);
                                			primary_ch_reason_code = ch_pref[i].opclass[j+1].chan_list[0].reason_code;
                                			err("Primary channel info:  opclass %d channel %d",
                                        			ch_pref[i].opclass[j+1].op_class,
                                        			ch_pref[i].opclass[j+1].chan_list[0].channel);
							j++;
						} else {
							primary_ch = 0;
						}
					}
				}
			}
			if (max_preferred == 0) {
				/* find first channel from global oper class */
				for(j = 0; j < ch_pref[i].op_class_cnt; j++) {
					if (ch_pref[i].opclass[j].ch_num == 0)
						continue;
					channel = find_first_oper_channel_from_operclass(ch_pref[i].opclass[j].op_class, &ch_pref[i].opclass[j]);
					/* try next opclass */
					if (channel == 0)
						continue;
					best_prefered_chan = channel;
					best_prefered_opclass = ch_pref[i].opclass[j].op_class;
					break;
				}

			}
			chinfo->op_class = best_prefered_opclass;
			chinfo->channel= best_prefered_chan;
			chinfo->reason_code = reason_code;
			err("final agent opclass %d, chinfo->channel %d , reason code in ch info:%d",
				chinfo->op_class,chinfo->channel, chinfo->reason_code);

			if (chinfo->channel == 0) {
				err("failed to find any channel");
				rsp_info[i].rsp_code = 0x02;
			}

			// 1 octets power
			chinfo->power = tran_power[i].power;

			err("***num %d ch info as below:***"
					"Radio identity %02x:%02x:%02x:%02x:%02x:%02x:"
					"if name %s "
					"op_class %d: "
					"channel %d: "
					"power %d: "
					, i, PRINT_MAC(chinfo->identifier), chinfo->ifname
					, chinfo->op_class
					, chinfo->channel, chinfo->power);

			/* Using i+1 index for storing primary CH info at setting->chinfo  */
			if (primary_ch) {
				chinfo += 1;
				chinfo->channel = primary_ch;
				chinfo->op_class = primary_ch_op_class;
				os_memcpy(chinfo->identifier, primary_ch_identifier, ETH_ALEN);
				chinfo->reason_code = primary_ch_reason_code;
			}
		}
		chinfo += 1;
	}

	if (primary_ch) {
		setting.ch_set_num++;
		body_len = sizeof(struct channel_setting) + setting.ch_set_num * sizeof(struct ch_config_info);
	}

	wapp_msg->type = WAPP_USER_SET_CHANNEL_SETTING;
	wapp_msg->length = (&wapp_msg->staAddr[6] - &wapp_msg->band) + 1 + body_len;

	// copy config to msg->body
	os_memcpy(wapp_msg->body, &setting, sizeof(setting));

	ret = wlanif_issue_wapp_command(ctx->back_ptr, WAPP_USER_SET_CHANNEL_SETTING, 0,
			NULL, NULL, wapp_msg->body, wapp_msg->length, 0, 0, 0);

	if (ret == -1) {
		err("(_1905_SET_CHANNEL_SETTING) fail");
		os_free(buf);
		return -1;
	}

	if (primary_ch)
		setting.ch_set_num--;

	for(i = 0 ; i < setting.ch_set_num ; i++){
		struct radio_info_db *radio = topo_srv_get_radio(
				topo_srv_get_1905_device(ctx,NULL),
				chinfo[i].identifier);
		if(radio)
			eloop_register_timeout(10, 0, topo_srv_update_1905_radio_capinfo, ctx, radio);
	}
	ret = map_1905_Set_Channel_Selection_Rsp_Info(ptrmapd->_1905_ctrl,
			rsp_info,  setting.ch_set_num);
	os_free(buf);
	return ret;
}

/**
* @brief Fn to update channel settings
*
* @param ctx own 1905 device context
* @param msg_buf msg buffer
* @param len msg len
*
* @return 0 if success else -1
*/
int topo_srv_update_channel_setting(struct own_1905_device *ctx,
	unsigned char *msg_buf, int len, struct ch_sel_rsp_info *ch_sel_rsp)
{
	struct channel_setting *setting = NULL;
	struct _1905_map_device *dev;
	struct radio_info_db *radio = NULL;
	unsigned char *al_mac = msg_buf;
	int i = 0;
	unsigned char num_oper_class = 0;
	unsigned char channel = 0;
	unsigned char oper_class = 0;
	unsigned char num_channel = 0;
	unsigned char radio_count = 0;

	setting = (struct channel_setting *)os_zalloc(512);
	if(!setting) {
		err("memory allocation fail\n");
		return -1;
	}
	os_memcpy(setting->almac, al_mac, ETH_ALEN);
	dev = topo_srv_get_1905_device(ctx, al_mac);
	if (!dev) {
		err("device with given almac not found\n");
		os_free(setting);
		return -1;
	}

	msg_buf += ETH_ALEN;
	while(1)
	{
		if (*msg_buf == 0x8b)
		{
			msg_buf += 1;
			msg_buf += 2;//! we skip the len for now
			radio = topo_srv_get_radio(dev, msg_buf);
			msg_buf += 6;
			num_oper_class = *msg_buf;
			msg_buf += 1;
			channel = 0;
			i = 0;
			while(i < num_oper_class)
			{
				if (channel == 0) {
					oper_class = *msg_buf;
				}
				msg_buf++;
				num_channel = *msg_buf;
				msg_buf += 1;
				if (channel == 0) {
					channel = *msg_buf;
				}
				msg_buf += num_channel;
				msg_buf++;//! ignore TX power
				i++;
			}
			setting->chinfo[radio_count].channel = channel;
			setting->chinfo[radio_count].op_class= oper_class;
			os_memcpy(setting->chinfo[radio_count].identifier, radio->identifier, ETH_ALEN);
			os_memcpy(ch_sel_rsp[radio_count].radio_indentifier, radio->identifier, ETH_ALEN);
			radio_count++;
		} else {
			break;
		}
	}
	if (radio_count) {
		setting->ch_set_num = radio_count;
		wlanif_issue_wapp_command(ctx->back_ptr, WAPP_USER_SET_CHANNEL_SETTING, 0,
			NULL, NULL, setting, 512, 0, 0, 0);
	}
	os_free(setting);
	return radio_count;
}

#ifdef MAP_R2
void topo_srv_get_channel_scan_capinfo(struct own_1905_device *ctx)
{
	map_get_info_from_wapp(ctx, WAPP_USER_GET_SCAN_CAP, WAPP_SCAN_CAPAB,
				   NULL, NULL, NULL, 0);
}

void topo_srv_get_r2_ap_capinfo(struct own_1905_device *ctx)
{
	map_get_info_from_wapp(ctx, WAPP_USER_GET_R2_AP_CAP, WAPP_R2_AP_CAP,
				   NULL, NULL, NULL, 0);
}

#ifdef DFS_CAC_R2
void topo_srv_get_cac_capinfo(struct own_1905_device *ctx)
{
	map_get_info_from_wapp(ctx, WAPP_USER_GET_CAC_CAP, WAPP_CAC_CAPAB,
				   NULL, NULL, NULL, 0);
}
void topo_srv_get_cac_statusinfo(struct own_1905_device *ctx)
{
	map_get_info_from_wapp(ctx, WAPP_USER_GET_CAC_STATUS, WAPP_CAC_STATUS_REPORT,
				   NULL, NULL, NULL, 0);
}
#endif
#endif

/**
* @brief Fn to initialize own 1905 device info
*
* @param ctx own 1905 device context
* @param dev 1905 device pointer
*
* @return 0 if success else -1
*/
int topo_srv_init_own_1905_info(struct own_1905_device *ctx, struct _1905_map_device *dev)
{
	struct device_info *dev_info;
	struct bridge_cap *br_cap;
	struct supported_srv *srv;

	struct device_info_sub *info_sub;
	struct iface_info *iface;
	struct _1905_bridge *br;
	int i, status = -1;
	unsigned char dev_info_buf[512] = {0}, br_cap_buf[512] = {0}, srv_buf[64] = {0};
	struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;
	char send_to_1905 = 0;

	dev_info = (struct device_info *)dev_info_buf;
	br_cap = (struct bridge_cap *)br_cap_buf;
	srv = (struct supported_srv *)srv_buf;
	while (status != 0) {
		status = map_1905_Get_Local_Devinfo(mapd_ctx->_1905_ctrl, dev_info, br_cap, srv);
		/* bandsteering */
		if (status == 0xff)
			break;
		os_sleep(1, 0);
	}
	os_memcpy(dev->_1905_info.al_mac_addr, dev_info->al_mac, ETH_ALEN);
	os_memcpy(ctx->al_mac, dev_info->al_mac, ETH_ALEN);
	debug("almac %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(dev_info->al_mac));

	for (i = 0; i < dev_info->inf_num; i++) {
		info_sub = &dev_info->inf_info[i];
		iface = os_zalloc(sizeof(struct iface_info));
		memcpy(iface->iface_addr, info_sub->mac, ETH_ALEN);
		iface->media_type = info_sub->media_type;
		SLIST_INSERT_HEAD(&(dev->_1905_info.first_iface), iface, next_iface);
		iface->p1905_device = dev;
		iface->valid = TRUE;
	}

	for (i = 0; i< br_cap->br_num; i++) {
		br = os_zalloc(sizeof(struct _1905_bridge));
		struct bridge_cap_sub *bridge_info = &br_cap->bridge_info[i];
		br->interface_count = bridge_info->inf_num;
		br->interface_mac_tuple = os_zalloc(br->interface_count* ETH_ALEN);
		memcpy(br->interface_mac_tuple, bridge_info->inf_mac,
		       (br->interface_count* ETH_ALEN));

		SLIST_INSERT_HEAD(&(dev->_1905_info.first_bridge), br, next_bridge);
	}

	dev->in_network = 1;
	os_get_time(&dev->first_seen);
	dev->supported_services = srv->srv[0];
	if (status == 0xff) {
		/* we don't have 1905, put a random alc mac in 1905 device */
		os_memcpy(&dev->_1905_info.al_mac_addr[5], &status, sizeof(char));
	}
	if ((status == 0xff) || (dev->supported_services == 0)) {
		dev->supported_services = 0;

		if(status != 0xff)
			map_start_auto_role_detection_srv(ctx);
	}

	ctx->current_connect_priority = MAX_POSSIBLE_BH_PRIORITY;
	err("topo_srv_init_own_1905_info\n");
	dev->channel_planning_completed = TRUE;
	wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_WIRELESS_INF_INFO,
		WAPP_WIRELESS_INF_INFO, NULL, 0, &send_to_1905, sizeof(send_to_1905), 0, 1, 0);
	return 0;
}

/**
* @brief Fn to allocate a 1905 device
*
* @return 1905 device pointer
*/
struct _1905_map_device *create_1905_device()
{
	struct _1905_map_device *dev = os_zalloc(sizeof(*dev));
	if (!dev)
		assert(0);

	SLIST_INIT(&dev->wlan_clients);
	SLIST_INIT(&dev->assoc_clients);
	SLIST_INIT(&dev->neighbors_entry);
	SLIST_INIT(&dev->first_radio);
	SLIST_INIT(&dev->first_bss);
	SLIST_INIT(&dev->_1905_info.first_iface);
	SLIST_INIT(&dev->_1905_info.first_bridge);
	dev->upstream_device = NULL;
	dev->root_distance = 0xff;
	dev->p_current_bss_rr = NULL;
#ifdef ACL_CTRL
	dev->is_acl_sync_done = FALSE;
#endif
	dev->vendor = VENDOR_MEDIATEK;
	err("one 1905 device created");

	return dev;
}

struct _1905_map_device *topo_srv_create_1905_device(struct own_1905_device *ctx, unsigned char *almac)
{
	struct _1905_map_device *dev = create_1905_device();
	SLIST_INSERT_AFTER(topo_srv_get_1905_device(ctx, NULL), dev, next_1905_device);
	os_memcpy(dev->_1905_info.al_mac_addr, almac, ETH_ALEN);
	os_get_time(&dev->first_seen);

	return dev;
}
/**
* @brief Fn to deinitialize metrics info in own 1905 device ctx
*
* @param ctx own 1905 device context
*
* @return 0 if success else -1
*/
int topo_srv_deinit_metrics_info(struct own_1905_device *ctx)
{
	struct metrics_info *metrics = &ctx->metric_entry;
	struct bss_db *bss_tmp, *bss = SLIST_FIRST(&metrics->metrics_query_head);
	struct mrsp_db *msrp_tmp, *msrp = SLIST_FIRST(&metrics->metrics_rsp_head);
	struct esp_db *esp, *esp_tmp;
	struct traffic_stats_db *traffic_tmp, *traffic = SLIST_FIRST(&metrics->traffic_stats_head);
	struct stats_db *tmp_stats, *stats;

	while (bss != NULL) {
		bss_tmp = SLIST_NEXT(bss, bss_entry);
		free(bss);
		bss = bss_tmp;
	}

	while (msrp != NULL) {
		esp = SLIST_FIRST(&msrp->esp_head);
		while (esp) {
			esp_tmp = SLIST_NEXT(esp, esp_entry);
			free(esp);
			esp = esp_tmp;
		}
		msrp_tmp = SLIST_NEXT(msrp, mrsp_entry);
		free(msrp);
		msrp = msrp_tmp;
	}

	while(traffic != NULL) {
		stats = SLIST_FIRST(&traffic->stats_head);
		while (stats) {
			tmp_stats = SLIST_NEXT(stats, stats_entry);
			free(stats);
			stats = tmp_stats;
		}
		traffic_tmp = SLIST_NEXT(traffic, traffic_stats_entry);
		free(traffic);
		traffic = traffic_tmp;
	}

	return 0;
}
/**
* @brief Fn to initialize metrics info
*
* @param ctx : own 1905 device context
* @param dev : 1905 device pointetr
*
* @return : 0
*/
int topo_srv_init_metrics_info(struct own_1905_device *ctx, struct _1905_map_device *dev)
{
	struct metrics_info *metrics = &ctx->metric_entry;

	SLIST_INIT(&metrics->metrics_query_head);
	SLIST_INIT(&metrics->metrics_rsp_head);
	SLIST_INIT(&metrics->traffic_stats_head);

	return 0;
}

/**
* @brief Fn to deinitialize metrics info
*
* @param ctx : own 1905 device context
*
* @return : 0
*/
int topo_srv_deinit_own_info(struct own_1905_device *ctx)
{
	struct bh_link_entry *tmp_bh, *bh = SLIST_FIRST(&ctx->bh_link_head);
	struct topology_channel *tmp_chan, *chan = SLIST_FIRST(&ctx->channel_head);

	topo_srv_deinit_metrics_info(ctx);
	while (bh) {
		tmp_bh = SLIST_NEXT(bh, next_bh_link);
		free(bh);
		bh = tmp_bh;
	}
	while (chan) {
		tmp_chan = SLIST_NEXT(chan, next_channel);
		free(chan);
		chan = tmp_chan;
	}
	return 0;
}

/**
* @brief Fn to deinit a 1905 device
*
* @param dev 1905 device pointer
*
* @return 0
*/
int topo_srv_deinit_1905dev_info(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{
	struct _1905_device *_1905_info = &dev->_1905_info;
	struct _1905_bridge *tmp_br, *br = SLIST_FIRST(&_1905_info->first_bridge);
	struct iface_info *tmp_iface, *iface = SLIST_FIRST(&_1905_info->first_iface);
	struct radio_info_db *tmp_radio, *radio = SLIST_FIRST(&dev->first_radio);
	struct basic_cap_db *tmp_bcap, *bcap;
	struct N_O_link_estimate_cb *tmp_link, *link;
	struct bss_info_db *tmp_bss, *bss = SLIST_FIRST(&dev->first_bss);
	struct esp_db *esp, *tmp_esp;
	struct connected_clients *tmp_client, *client = SLIST_FIRST(&dev->wlan_clients);
	struct map_neighbor_info *tmp_nb, *nb = SLIST_FIRST(&dev->neighbors_entry);
	struct backhaul_link_info *bh, *tmp_bh;

	while (br) {
		tmp_br = SLIST_NEXT(br, next_bridge);
		if (br->interface_mac_tuple)
			free(br->interface_mac_tuple);
		free(br);
		br = tmp_br;
	}

	while (iface) {
		tmp_iface = SLIST_NEXT(iface, next_iface);
		free(iface);
		iface = tmp_iface;
	}
	delete_agent_ch_prefer_info(ctx, dev);
	while (radio) {
		bcap = SLIST_FIRST(&radio->radio_capability.basic_caps.bcap_head);
		while (bcap) {
			tmp_bcap = SLIST_NEXT(bcap, basic_cap_entry);
			free(bcap);
			bcap = tmp_bcap;
		}
		link = SLIST_FIRST(&radio->link_estimate_cb_head);
		while (link) {
			tmp_link = SLIST_NEXT(link, link_estimate_cb_entry);
			free(link);
			link = tmp_link;
		}
		tmp_radio = SLIST_NEXT(radio, next_radio);
		free(radio);
		radio = tmp_radio;
	}

	while (bss) {
		esp = SLIST_FIRST(&bss->esp_head);
		while (esp) {
			tmp_esp = SLIST_NEXT(esp, esp_entry);
			free(esp);
			esp = tmp_esp;
		}
		tmp_bss = SLIST_NEXT(bss, next_bss);
		free(bss);
		bss = tmp_bss;
	}
	while (client) {
		tmp_client = SLIST_NEXT(client, next_client);
		free(client);
		client = tmp_client;
	}
	while (nb) {
		bh = SLIST_FIRST(&nb->bh_head);
		while (bh) {
			tmp_bh = SLIST_NEXT(bh, next_bh);
			free(bh);
			bh = tmp_bh;
		}
		tmp_nb = SLIST_NEXT(nb, next_neighbor);
		free(nb);
		nb = tmp_nb;
	}

	return 0;
}

/**
* @brief Fn to deinit topology server
*
* @param ctx own 1905 device context
*
* @return 0
*/
int topo_srv_deinit_topo_srv(struct own_1905_device *ctx)
{
	struct _1905_map_device *tmp_dev, *dev = topo_srv_get_1905_device(ctx, NULL);

	topo_srv_deinit_own_info(ctx);
	while (dev) {
		tmp_dev = SLIST_NEXT(dev, next_1905_device);
		topo_srv_deinit_1905dev_info(ctx, dev);
		free(dev);
		dev = tmp_dev;
	}

	return 0;
}

int topo_srv_update_bss_role_for_controller(struct own_1905_device *dev)
{
	struct bss_role brole;
	struct bss_info_db *bss = NULL;
	struct _1905_map_device *_1905_device = topo_srv_get_next_1905_device(dev, NULL);
	SLIST_FOREACH(bss, &_1905_device->first_bss, next_bss) {
		os_memcpy(brole.bssid , bss->bssid , ETH_ALEN);
		brole.role = bss->map_vendor_extn;
		if (wlanif_issue_wapp_command((struct mapd_global *)dev->back_ptr, WAPP_USER_SET_BSS_ROLE, 0, brole.bssid,
					brole.bssid, &brole, sizeof(struct bss_role), 0, 0, 0) < 0)
			return -1;
	}
	return 0;
}

/**
* @brief Fn to init info for 1905 device
*
* @param ctx own 1905 device context
*
* @return 0 if success else -1
*/
int topo_srv_init_own_info(struct own_1905_device *ctx)
{
	struct _1905_map_device *dev = create_1905_device();
	struct radio_info_db *radio;

	debug("initializing own 1905 info");
	SLIST_INIT(&ctx->_1905_dev_head);
	SLIST_INIT(&ctx->bh_link_head);
	SLIST_INIT(&ctx->channel_head);
	SLIST_INIT(&ctx->bl_ap_list);
	SLIST_INIT(&ctx->steer_cands_head);
#ifdef CENT_STR
	STAILQ_INIT(&ctx->cent_steer_cands_head);
#endif
	ctx->device_role = DEVICE_ROLE_UNCONFIGURED;
	ctx->chan_report = NULL;
	ctx->need_to_update_wts = 1;
	info("Inserting into 1905 list %p", dev);
	SLIST_INSERT_HEAD(&ctx->_1905_dev_head, dev, next_1905_device);

	topo_srv_init_own_1905_info(ctx, dev);
	topo_srv_init_metrics_info(ctx, dev);
	wlanif_get_op_chan_info(ctx->back_ptr);
#ifdef MAP_R2
	topo_srv_get_channel_scan_capinfo(ctx);
	//topo_srv_get_r2_ap_capinfo(ctx);
	//parse_map_policy_config_request_message(ctx, abc);
	//parse_map_policy_config_request_message(ctx, abc);
	//wapp_set_unsuccessful_association_policy_setting(ctx, &ctx->map_policy.assoc_failed_policy);
	//parse_ap_metrics_query_message(ctx, abc);
	//topo_srv_get_radio_metrics_info(ctx);
#ifdef DFS_CAC_R2
	topo_srv_get_cac_capinfo(ctx);
#endif
	topo_srv_get_r2_ap_capinfo(ctx);
#endif
	radio = topo_srv_get_next_radio(dev, NULL);
	while (radio) {
		topo_srv_get_radio_capinfo(ctx, radio->identifier);
		map_get_info_from_wapp(ctx, WAPP_USER_GET_OPERATIONAL_BSS,
			WAPP_OPERBSS_REPORT, radio->identifier, NULL, NULL, 0);
		radio = topo_srv_get_next_radio(dev, radio);
	}
	if (!is_1905_present()) {
		ctx->config_status = DEVICE_CONFIGURED;
		mapd_update_controller_steer_policy(ctx->back_ptr);
		topo_srv_update_uplink_rate(ctx,dev);
		return 0;
	}

	/*Reset Previous value if any*/
	topo_srv_update_radio_config_status(ctx, NULL, FALSE);

	if (!((struct mapd_global *)ctx->back_ptr)->params.Certification)
		wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_BH_WIRELESS_SETTING,
					WAPP_MAP_BH_CONFIG, NULL, NULL, NULL, 0, 0, 1, 0);


	if (dev->device_role == DEVICE_ROLE_CONTROLLER) {
		mapd_update_controller_steer_policy(ctx->back_ptr);
		topo_srv_update_bss_role_for_controller(ctx);
	}

	return 0;
}

/**
* @brief Fn to update device role
*
* @param ctx own 1905 device context
* @param almac almac of 1905 device
* @param role role of 1905 device
*/
void topo_srv_update_device_role(struct own_1905_device *ctx, unsigned char *almac, int role)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, almac);
	if (!dev) {
		err("failed to get 1905 device");
		return;
	}

	dev->device_role = role;
}

/**
* @brief Fn to update root distance of a 1905 device
*
* @param dev 1905 device pointer
* @param root_distance root distace of previos device
* @param distance_updated cookie value to check whether distance is updated or not
*/
void topo_srv_update_root_distance(struct _1905_map_device *dev,
			unsigned char root_distance, unsigned long distance_updated)
{
	struct map_neighbor_info *neighbor = NULL;

	if ((dev == NULL) || (dev->in_network == 0))
		return;

	info("updating root distance for %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(dev->_1905_info.al_mac_addr));
	info("current =%d new=%d", dev->root_distance, root_distance);
	dev->root_distance = root_distance;
	dev->distance_updated = distance_updated;

	SLIST_FOREACH(neighbor, &dev->neighbors_entry, next_neighbor) {
		if (neighbor && neighbor->neighbor &&
						(neighbor->neighbor->distance_updated == distance_updated))
			continue;
		else
			topo_srv_update_root_distance(neighbor->neighbor, root_distance + 1, distance_updated);
	}
}
/**
* @brief Fn to update root distace for whole topology
*
* @param ctx own 1905 device context
*/
#if 0
void topo_srv_update_topo_root_distance_wireless(struct own_1905_device *ctx)
{
	int root_distance = 0;
	unsigned long distance_updated = random();
	struct _1905_map_device *dev = topo_srv_get_controller_device(ctx);
	if (dev)
		topo_srv_update_root_wireless_distance(dev, root_distance, distance_updated);
}
#endif
/**
* @brief Fn to update root distace for whole topology
*
* @param ctx own 1905 device context
*/
void topo_srv_update_topo_root_distance(struct own_1905_device *ctx)
{
	int root_distance = 0;
	unsigned long distance_updated = random();
	struct _1905_map_device *dev = topo_srv_get_controller_device(ctx);
	if (dev)
		topo_srv_update_root_distance(dev, root_distance, distance_updated);
}
/**
* @brief Fn to attch a CLI interface with its corresponding BSSID structure
*
* @param ctx own 1905 device context
* @param bss peer bss
* @param iface CLI interface
*/

void topo_srv_attach_bh_cli_to_bss(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct bss_info_db *bss,
	struct iface_info *iface)
{
	unsigned char channel = 0;
	struct radio_info_db *radio = NULL;
	info("Attach %02x:%02x:%02x:%02x:%02x:%02x to AP interface %02x:%02x:%02x:%02x:%02x:%02x\n",
		PRINT_MAC(iface->iface_addr), PRINT_MAC(bss->bssid));
	channel = bss->radio->channel[0];
	if (channel != 0) {
		iface->radio = topo_srv_get_radio_by_channel(iface->p1905_device,channel);
		radio = iface->radio;
		if (radio) {
			radio->uplink_bh_present = TRUE;
			topo_srv_check_all_iface_mapped(ctx, iface->p1905_device);
		}
	}
}
/**
* @brief Fn to map radio interface with CLI interface
* @param ctx own 1905 device context
* @param dev device for whcich mapping is to be done
*/

void topo_srv_map_radio_to_cli_iface(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{
	struct iface_info *iface = NULL;
	struct bss_info_db *peer_bss = NULL;
	struct _1905_map_device *peer_1905_dev;

	SLIST_FOREACH(iface, &(dev->_1905_info.first_iface), next_iface) {
		if ((*(((char *)(&iface->media_info)) + 6) != 0x40) ||
			(iface->media_type < IEEE802_11_GROUP) ||
			(iface->media_type >= IEEE1901_GROUP))
		{
			continue;
		}
		debug("iface address = %02x:%02x:%02x:%02x:%02x:%02x",
			PRINT_MAC(iface->iface_addr));
#if 0
		if (iface->radio != NULL)
		{
			info("Radio already mapped");
			continue;
		}
#endif
		if (os_memcmp(iface->media_info.network_membership,
			ZERO_MAC_ADDR, ETH_ALEN)) {
			debug("iface network membership = %02x:%02x:%02x:%02x:%02x:%02x",
				PRINT_MAC(iface->media_info.network_membership));
			peer_1905_dev = topo_srv_get_1905_by_bssid(ctx,iface->media_info.network_membership);
			if (peer_1905_dev != NULL) {
				peer_bss = topo_srv_get_bss_by_bssid(ctx,peer_1905_dev, iface->media_info.network_membership);
				if (peer_bss != NULL && peer_bss->radio != NULL)
					topo_srv_attach_bh_cli_to_bss(ctx, peer_1905_dev, peer_bss, iface);
			}
		} else {
			if (iface->radio)
			{
				struct radio_info_db *iface_radio = iface->radio;

				err("Remove Uplink Radio from this interface now\n");
				iface_radio->uplink_bh_present = FALSE;
			}
		}
	}
}

/**
* @brief Fn to map radio interface with AP interface
* @param ctx own 1905 device context
* @param dev device for whcich mapping is to be done
*/
void topo_srv_map_radio_to_ap_iface(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{
	struct iface_info *iface = NULL;
	struct bss_info_db *bss = NULL;

	SLIST_FOREACH(iface, &(dev->_1905_info.first_iface), next_iface) {
		if (*(((char *)(&iface->media_info)) + 6) != 0 ||
			(iface->media_type < IEEE802_11_GROUP) ||
			(iface->media_type >= IEEE1901_GROUP))
		{
			continue;
		}
		if (iface->radio != NULL)
		{
			continue;
		}
		SLIST_FOREACH(bss, &(dev->first_bss), next_bss) {
			if (!os_memcmp(iface->iface_addr, bss->bssid, ETH_ALEN))
			{
				info("radio mapped for %02x:%02x:%02x:%02x:%02x:%02x",
					PRINT_MAC(iface->iface_addr));
				iface->radio = bss->radio;
				break;
			}
		}
	}
	topo_srv_check_all_iface_mapped(ctx, dev);
}

int topo_srv_send_vendor_oui_tlv(struct own_1905_device *ctx, unsigned char *al_mac_addr)
{
	struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;
	struct tlv_head tlv;
	debug("sending vendor ie tlv");
	tlv.tlv_type = TLV_802_11_VENDOR_SPECIFIC;
	tlv.tlv_len = host_to_be16(OUI_LEN + sizeof(u8));
	os_memcpy(tlv.oui, MTK_OUI, OUI_LEN);
	tlv.func_type = FUNC_VENDOR_OUI;
	map_1905_Send_Vendor_Specific_Message(mapd_ctx->_1905_ctrl, (char *)al_mac_addr,
						(char *)&tlv, sizeof(struct tlv_head));

	return 0;
}

int topo_srv_send_vendor_chan_report_msg(struct own_1905_device *ctx, unsigned char *al_mac_addr)
{
	struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;
	struct tlv_head *tlv;
	unsigned char *buf;
	unsigned short tlv_len;

	if (!ctx->chan_report) {
		return -1;
	}
	debug("sending vendor channel report tlv");
	tlv_len = (OUI_LEN + sizeof(u8)) + sizeof(unsigned char) +
			sizeof(struct ch_rep_info) * ctx->chan_report->ch_rep_num + sizeof(unsigned char);
	buf = os_zalloc(tlv_len + 3);

	if(buf == NULL)
	{
		return -1;
	}

	tlv = (struct tlv_head *)buf;
	tlv->tlv_type = TLV_802_11_VENDOR_SPECIFIC;
	tlv->tlv_len = host_to_be16(tlv_len);

	os_memcpy(tlv->oui, MTK_OUI, OUI_LEN);
	tlv->func_type = FUNC_VENDOR_CHAN_REPORT;
	os_memcpy(buf + sizeof(struct tlv_head), ctx->chan_report, tlv_len - 5);
	map_1905_Send_Vendor_Specific_Message(mapd_ctx->_1905_ctrl, (char *)al_mac_addr,
						(char *)buf, tlv_len + 3);
	os_free(buf);

	return 0;
}

void duplicate_sta_check_for_1905_device(struct own_1905_device *ctx, struct _1905_map_device *dev)
{
	unsigned char mac[ETH_ALEN];
	struct associated_clients *client_info = NULL;
	struct connected_clients *client_for_mac = NULL;
	struct associated_clients *assoc_for_mac = NULL;
	struct _1905_map_device *_1905_device_for_mac = topo_srv_get_next_1905_device(ctx, NULL);

	SLIST_FOREACH(client_info, &dev->assoc_clients, next_client) {
		os_memcpy(mac, client_info->client_addr, ETH_ALEN);
		while(_1905_device_for_mac)
		{
			if(_1905_device_for_mac != dev)
			{
				SLIST_FOREACH(client_for_mac, &(_1905_device_for_mac->wlan_clients), next_client)
				{
					if (os_memcmp(mac, client_for_mac->client_addr, ETH_ALEN) == 0)
					{
						SLIST_REMOVE(&(_1905_device_for_mac->wlan_clients), client_for_mac, connected_clients, next_client);
						free(client_for_mac);
						break;
					}
				}
				SLIST_FOREACH(assoc_for_mac, &(_1905_device_for_mac->assoc_clients), next_client)
				{
					if (os_memcmp(mac, assoc_for_mac->client_addr, ETH_ALEN) == 0)
					{
						SLIST_REMOVE(&(_1905_device_for_mac->assoc_clients), assoc_for_mac, associated_clients, next_client);
						free(assoc_for_mac);
						break;
					}
				}
			}
			_1905_device_for_mac = topo_srv_get_next_1905_device(ctx, _1905_device_for_mac);
		}
	}
}

void duplicate_sta_check_for_notification_evt(struct own_1905_device *ctx, struct topo_notification *evt)
{
	struct _1905_map_device *dev = topo_srv_get_next_1905_device(ctx, NULL);

	while(dev)
	{
		struct connected_clients *client_for_mac = NULL;
		struct associated_clients *assoc_for_mac = NULL;
		int i;
		struct client_assoc *assoc;

		if (os_memcmp(evt->al_mac, dev->_1905_info.al_mac_addr, ETH_ALEN) == 0)
		{
			dev = topo_srv_get_next_1905_device(ctx, dev);
			continue;
		}
		for (i = 0; i < evt->assoc_cnt; i++) {
			assoc = &evt->assoc[i];
			SLIST_FOREACH(client_for_mac, &(dev->wlan_clients), next_client)
			{
				if (os_memcmp(client_for_mac->client_addr, assoc->sta_addr, ETH_ALEN) == 0)
				{
					SLIST_REMOVE(&dev->wlan_clients, client_for_mac, connected_clients, next_client);
					free(client_for_mac);
					break;
				}
			}
			SLIST_FOREACH(assoc_for_mac, &(dev->assoc_clients), next_client) {
				if (os_memcmp(assoc_for_mac->client_addr, assoc->sta_addr, ETH_ALEN) == 0) {
					SLIST_REMOVE(&dev->assoc_clients, assoc_for_mac, associated_clients, next_client);
					free(assoc_for_mac);
					break;
				}
			}
		}
		dev = topo_srv_get_next_1905_device(ctx, dev);
	}
}

int topo_srv_issue_disconnect_if_local(struct own_1905_device *ctx, struct topo_notification *evt)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	struct connected_clients *cli_wlan;
	int i;
	struct client_assoc *assoc;

	for (i = 0; i < evt->assoc_cnt; i++) {

		/* Ignore for own device */
		if (os_memcmp(evt->al_mac, dev->_1905_info.al_mac_addr, ETH_ALEN) == 0)
			continue;

		assoc = &evt->assoc[i];
		/* Ignore in case of disassoc */
		if (assoc->is_joined == 0)
			continue;
		SLIST_FOREACH(cli_wlan, &dev->wlan_clients, next_client) {
			if (os_memcmp(cli_wlan->client_addr, assoc->sta_addr, ETH_ALEN) == 0) {
				/* We have an entry for this, issue a disconnect */
				err("issuing disconnect to %02x:%02x:%02x:%02x:%02x:%02x",
					PRINT_MAC(cli_wlan->client_addr));
				wlanif_deauth_sta((struct mapd_global *)ctx->back_ptr, cli_wlan->client_addr, NULL);
			}
		}
	}

	return 0;
}

void topo_serv_update_bh_conn_assoc_client(struct own_1905_device *ctx,struct _1905_map_device *dev) {

	struct associated_clients *assoc_client = NULL;
	struct connected_clients  *wlan_client = NULL;


	if(!dev)
		return;

	SLIST_FOREACH(wlan_client, &dev->wlan_clients, next_client) {

		if(topo_srv_get_1905_by_iface_addr(ctx,wlan_client->client_addr)){
			wlan_client->is_bh_link = 1;
			err("Apcli detected: "MACSTR,MAC2STR(wlan_client->client_addr));
			assoc_client = topo_srv_get_associate_client(ctx,dev,wlan_client->client_addr);
			if(assoc_client)
				assoc_client->is_bh_link = 1;

		}

	}
	assoc_client = NULL;
	SLIST_FOREACH(assoc_client, &dev->assoc_clients, next_client) {

		if(topo_srv_get_1905_by_iface_addr(ctx,assoc_client->client_addr)){
			assoc_client->is_bh_link = 1;
			err("Apcli detected: "MACSTR,MAC2STR(assoc_client->client_addr));
		}

	}

  return;
}


/**
* @brief Fn to parse 1905 topology event resp
*
* @param ctx own 1905 device context
* @param buf msg buffer
*
* @return 0 if success else -1
*/

int topo_srv_handle_topology_event(struct own_1905_device *ctx, unsigned char *buf, int len)
{
	int length = 0;
	unsigned char *temp_buf = NULL;
	unsigned char al_mac_addr[ETH_ALEN];
	struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;
	struct map_neighbor_info *neighbor = NULL;
	struct _1905_map_device *dev, *own_dev;
	struct iface_info *iface = NULL;
	int root_distance = 0;
	unsigned char own_response = FALSE;
	struct backhaul_link_info *bh = NULL;
	char link_change = 0;
	char neighbor_found = 0;
	int parse_len = 0;
#ifdef MAP_R2
        unsigned char profile = DEV_TYPE_R1;
#endif

	if(!ctx || !buf || len < ETH_ALEN) {
		err(" invaild input params len(%d)\n", len);
		return -1;
	}

	temp_buf = (unsigned char *)buf;
	memcpy(al_mac_addr, temp_buf, ETH_ALEN);

	err("recv TOPOLOGY_response from %02x:%02x:%02x:%02x:%02x:%02x, len :%d", PRINT_MAC(al_mac_addr), len);
	dev = topo_srv_get_1905_device(ctx, al_mac_addr);
	if (!dev) {
		dev = topo_srv_create_1905_device(ctx, al_mac_addr);
		dev->in_network = 1;
	} else {
		SLIST_FOREACH(iface, &(dev->_1905_info.first_iface), next_iface) {
			iface->is_map_if = 0;	/*reset here, and it will update in parse_p1905_neighbor_device_type_tlv*/
		}
		if (dev->in_network == 0)
			os_get_time(&dev->first_seen);
	}
	own_dev = topo_srv_get_next_1905_device(ctx, NULL);
	duplicate_sta_check_for_1905_device(ctx, dev);
	if(!mapd_ctx->params.Certification && own_dev && os_memcmp(al_mac_addr,own_dev->_1905_info.al_mac_addr, ETH_ALEN)) {
		// don't send to own device.
		topo_srv_send_vendor_oui_tlv(ctx, al_mac_addr);
		topo_srv_send_vendor_chan_report_msg(ctx, al_mac_addr);
		map_1905_Send_AP_Capability_Query_Message(mapd_ctx->_1905_ctrl, (char *)al_mac_addr);
	} else if (!mapd_ctx->params.Certification){
		ctx->upstream_device_present = FALSE;
		own_response = TRUE;
	}

	/*adjust algorithm in multi-backhaul case*/
	if( len < (ETH_ALEN + ETH_ALEN)) {
		err("  len(%d) less than 12 \n", len);
		return -1;
	}
	temp_buf = buf + ETH_ALEN + ETH_ALEN;
	parse_len = parse_len +  ETH_ALEN + ETH_ALEN;
	SLIST_FOREACH(neighbor, &(dev->neighbors_entry), next_neighbor) {
		if (neighbor->neighbor)
			debug("is valid 0 for %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(neighbor->neighbor->_1905_info.al_mac_addr));
		neighbor->is_valid = 0;
		/*use insert_new_link to record if new link is established
       *maybe a new map device online or a new bh link create
       */
       neighbor->insert_new_link = 0;
       /*in multiple case, there may exist multiple wifi sta link and one eth link in one neighbor
       *use is_valid to record if the link is still exist in topology response message
       */
       SLIST_FOREACH(bh, &neighbor->bh_head, next_bh)
       {
           bh->is_valid = 0;
       }
	}
	clear_all_client_info(ctx, dev);
	while (1) {
		length = 0;

		if(parse_len > len) {
			err(" parse_len (%d) is bigger than total len (%d), drop this topo event \n", parse_len, len);
			return -1;
		}

		if (*temp_buf == DEVICE_INFO_TLV_TYPE) {
			/*then shift back to start of device info tlv, parsing it first */
			length = parse_device_info_type_tlv(ctx, temp_buf, dev);
			if (length < 0) {
				err("error device info tlv");
				return -1;
			}
			/*we have parsed this tlv, so just get length & shift */
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
			parse_len = parse_len + length;
		} else if (*temp_buf == BRIDGE_CAPABILITY_TLV_TYPE) {
			length = parse_bridge_capability_type_tlv(ctx, temp_buf, dev);
			if (length < 0) {
				err("error bridge capability tlv");
				return -1;
			}
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
			parse_len = parse_len + length;
		} else if (*temp_buf == P1905_NEIGHBOR_DEV_TLV_TYPE) {
			length = parse_p1905_neighbor_device_type_tlv(ctx, temp_buf, dev);
			if (length < 0) {
				err("error p1905.1 neighbor device tlv");
				return -1;
			}
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
			parse_len = parse_len + length;
			neighbor_found = 1;
		} else if (*temp_buf == NON_P1905_NEIGHBOR_DEV_TLV_TYPE) {
			debug("non-p1905.1 neighbor device tlv");
			length = parse_non_p1905_neighbor_device_type_tlv(ctx, temp_buf, dev);
			if (length < 0) {
				err("error non-p1905.1 neighbor device tlv");
				return -1;
			}
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
			parse_len = parse_len + length;
		} else if (*temp_buf == SUPPORTED_SERVICE_TLV_TYPE) {
			length = parse_supported_service_tlv(ctx, temp_buf, dev);
			if (length < 0) {
				err("error supported service tlv");
				return -1;
			}
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
			parse_len = parse_len + length;
		} else if (*temp_buf == AP_OPERATIONAL_BSS_TYPE) {
			length = parse_ap_operational_bss_type_tlv(ctx, temp_buf, dev);
			if (length < 0) {
				err("error ap_operational_bss tlv");
				return -1;
			}
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
			parse_len = parse_len + length;
		} else if (*temp_buf == AP_ASSOCIATED_CLIENTS_TYPE) {
			length = parse_ap_associated_clients_type_tlv(ctx, temp_buf, dev);
			if (length < 0) {
				err("error ap_associated_clients tlv");
				return -1;
			}
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
			parse_len = parse_len + length;
#ifdef MAP_R2
		} else if (*temp_buf == MULTI_AP_VERSION_TYPE) {
			length = parse_map_version_tlv(temp_buf, al_mac_addr, &profile);
			dev->map_version = profile;
			temp_buf += length;
			parse_len = parse_len + length;
#endif
		} else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			err("wrong TLV in topology response message %u", *temp_buf);
			/*ignore extra tlv */
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
			parse_len = parse_len + length;
		}
	}
	err("recv TOPOLOGY_response from %02x:%02x:%02x:%02x:%02x:%02x\n"
		"neighbor found(%d)", PRINT_MAC(al_mac_addr), neighbor_found);
	clear_invalid_client_info(ctx, dev);
	topo_serv_update_remote_peers(ctx, dev, &link_change);
	topo_srv_remove_all_invalid_bss(ctx, dev);
	topo_srv_remove_all_invalid_ap_iface(ctx, dev);
	root_distance = topo_srv_get_root_distance(ctx, NULL);
	topo_srv_update_topo_root_distance(ctx);
	topo_srv_update_dev_upstream_device(dev);
	topo_serv_update_own_topology_info(ctx);

	/* XXX: revisit if this needs to be done for every response */
	//if (ctx->device_role == DEVICE_ROLE_AGENT && (own_response || (ctrlr_found) || (link_change)))
	if (ctx->device_role == DEVICE_ROLE_AGENT && own_response) {
		topo_srv_manage_bh_links(ctx);
	}
	if (ctx->device_role == DEVICE_ROLE_AGENT && !own_response) {
		mapd_fill_secondary_channels_for_1905_dev(ctx, dev);
	}

#ifdef ACL_CTRL
	if (!dev->is_acl_sync_done && !own_response) {
		/* Add agent's apcli mac to ACL list */
		if ((ctx->device_role == DEVICE_ROLE_CONTROLLER) && (dev->device_role == DEVICE_ROLE_AGENT)) {
			err("new dev found\n");
			dev->is_acl_sync_done = TRUE;
			mapd_acl_sync_new_agent_info(ctx, dev);
		}
		else if ((ctx->device_role == DEVICE_ROLE_AGENT) && (dev->device_role == DEVICE_ROLE_CONTROLLER)) {
			err("ACL query not yet send\n");
			dev->is_acl_sync_done = TRUE;
			if (!eloop_is_timeout_registered(map_sync_acl_info, ctx->back_ptr, ctx))
				eloop_register_timeout(5, 0, map_sync_acl_info, ctx->back_ptr, ctx);
		}
	}
#endif /*ACL_CTRL*/

	topo_serv_update_bh_conn_assoc_client(ctx, dev);


	topo_srv_map_radio_to_ap_iface(ctx, dev);
	topo_srv_map_radio_to_cli_iface(ctx, dev);
	topo_srv_update_uplink_rate(ctx, dev);

	if (ctx->device_role == DEVICE_ROLE_CONTROLLER &&
		dev->radio_mapping_completed &&
		(ctx->ch_planning.ch_planning_enabled == TRUE) &&
		!dev->ch_preference_available)
	{
		struct mapd_global *global = ctx->back_ptr;

		if (dev != topo_srv_get_1905_device(ctx, NULL))
			map_1905_Send_Channel_Preference_Query_Message(global->_1905_ctrl, (char *)dev->_1905_info.al_mac_addr);
	}

	/* It could be possible that our distance from root has changed */
	if (root_distance != topo_srv_get_root_distance(ctx, NULL))
	{
		struct radio_info_db *radio = NULL;
		unsigned char all_radios_status = TRUE;

		radio = topo_srv_get_radio(topo_srv_get_1905_device(ctx, NULL), NULL);
		while (radio) {
			if (radio->config_status == FALSE)
				all_radios_status = FALSE;
			radio = topo_srv_get_next_radio(topo_srv_get_1905_device(ctx, NULL), radio);
		}
		if (all_radios_status)
			ap_selection_update_vend_ie(ctx, NULL, TRUE);
	}

	os_get_time(&dev->last_seen);

	if (link_change) {
		/*trigger arp after set primary interface to FW to avoid bc looping*/
		/*do a delay to avoid loop cause ap interface delete sta*/
		if (eloop_is_timeout_registered(register_send_garp_req_to_wapp, (void *)ctx, NULL)) {
			eloop_cancel_timeout(register_send_garp_req_to_wapp, (void *)ctx, NULL);
			eloop_register_timeout(5, 0, register_send_garp_req_to_wapp, ctx, NULL);
			err("register new send_garp_req_to_wapp");
		} else {
			eloop_register_timeout(5, 0, register_send_garp_req_to_wapp, ctx, NULL);
			err("register send_garp_req_to_wapp");
		}
		err("link chnage is 1 go ahead ");
		if (ctx->device_role != DEVICE_ROLE_CONTROLLER) {
			wlanif_get_op_chan_info(ctx->back_ptr);
		}
		topo_srv_ch_planning_on_agents(ctx, dev);
	}
	if(ctx->device_role == DEVICE_ROLE_CONTROLLER && (os_memcmp(ctx->al_mac, dev->_1905_info.al_mac_addr, ETH_ALEN) != 0)) {
		//only controller needs to send policy to its agents
		steer_msg_update_policy_config(ctx->back_ptr, dev);
	}
	return 0;
}

/**
* @brief Fn to delete current steer policy
*
* @param spolicy steer policy pointer
*
* @return  0 if success else -1
*/
int delete_exist_steering_policy(struct steer_policy *spolicy)
{
	struct sta_db *sta = NULL, *sta_tmp = NULL;
	struct radio_policy_db *policy = NULL, *policy_tmp = NULL;

	debug("delete_steering_policy");

	sta = SLIST_FIRST(&spolicy->local_disallow_head);
	while (sta != NULL) {
		debug("local_disallow_head sta_mac(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(sta->mac));
		sta_tmp = SLIST_NEXT(sta, sta_entry);
		free(sta);
		sta = sta_tmp;
	}
	SLIST_INIT(&spolicy->local_disallow_head);
	spolicy->local_disallow_count = 0;

	sta = SLIST_FIRST(&spolicy->btm_disallow_head);
	while (sta != NULL) {
		debug("btm_disallow_head sta_mac(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(sta->mac));
		sta_tmp = SLIST_NEXT(sta, sta_entry);
		free(sta);
		sta = sta_tmp;
	}
	SLIST_INIT(&spolicy->btm_disallow_head);
	spolicy->btm_disallow_count = 0;

	policy = SLIST_FIRST(&spolicy->radio_policy_head);
	while (policy != NULL) {
		debug("radio_policy_head identifier(%02x:%02x:%02x:%02x:%02x:%02x)",
			     PRINT_MAC(policy->identifier));
		debug("steer_policy=%d, ch_util_thres=%d, rssi_thres=%d",
			     policy->steer_policy, policy->ch_util_thres, policy->rssi_thres);
		policy_tmp = SLIST_NEXT(policy, radio_policy_entry);
		free(policy);
		policy = policy_tmp;
	}
	SLIST_INIT(&spolicy->radio_policy_head);
	spolicy->radios = 0;

	return 0;
}

/**
* @brief Fn to update steer policy in wapp
*
* @param ctx own 1905 device context
* @param spolicy sterr policy
*/
void wapp_set_steering_policy_setting(struct own_1905_device *ctx, struct steer_policy *spolicy)
{
	int datalen = 0;
	struct local_disallow_sta_head *sta_head = NULL;
	struct btm_disallow_sta_head *btm_sta_head = NULL;
	struct radio_policy_head *radio_head = NULL;
	struct radio_policy *policy = NULL;
	struct sta_db *sta = NULL;
	struct radio_policy_db *policy_db = NULL;
	unsigned char *mac = NULL;
	unsigned char buf[128] = { 0 };

	if (spolicy->local_disallow_count) {
		datalen = spolicy->local_disallow_count * ETH_ALEN + 1;
		sta_head = (struct local_disallow_sta_head *)buf;
		sta_head->sta_cnt = spolicy->local_disallow_count;
		mac = sta_head->sta_list;
		SLIST_FOREACH(sta, &spolicy->local_disallow_head, sta_entry) {
			memcpy(mac, sta->mac, ETH_ALEN);
			mac += ETH_ALEN;
		}
		map_get_info_from_wapp(ctx, WAPP_USER_SET_LOCAL_STEER_DISALLOW_STA, 0, NULL, NULL,
				       (void *)sta_head, datalen);
	}

	if (spolicy->btm_disallow_count) {
		datalen = spolicy->btm_disallow_count * ETH_ALEN + 1;
		btm_sta_head = (struct btm_disallow_sta_head *)buf;
		btm_sta_head->sta_cnt = spolicy->btm_disallow_count;
		mac = btm_sta_head->sta_list;
		SLIST_FOREACH(sta, &spolicy->btm_disallow_head, sta_entry) {
			memcpy(mac, sta->mac, ETH_ALEN);
			mac += ETH_ALEN;
		}
		map_get_info_from_wapp(ctx, WAPP_USER_SET_BTM_STEER_DISALLOW_STA, 0, NULL, NULL,
				       (void *)btm_sta_head, datalen);
	}

	if (spolicy->radios) {
		datalen = spolicy->radios * (sizeof(struct radio_policy_head) + sizeof(struct radio_policy));
		policy = (struct radio_policy *)buf;
		policy->radio_cnt = spolicy->radios;
		radio_head = policy->radio;
		SLIST_FOREACH(policy_db, &spolicy->radio_policy_head, radio_policy_entry) {
			memcpy(radio_head->identifier, policy_db->identifier, ETH_ALEN);
			radio_head->policy = policy_db->steer_policy;
			radio_head->ch_ultil_thres = policy_db->ch_util_thres;
			radio_head->rssi_thres = policy_db->rssi_thres;
			radio_head++;
		}
		map_get_info_from_wapp(ctx, WAPP_USER_SET_RADIO_CONTROL_POLICY, 0, NULL, NULL, (void *)policy, datalen);
	}
}

/**
* @brief Fn to check whether btm steer is allowed or not
*
* @param ctx own 1905 device context
* @param mac mac address of station
*
* @return 0/1
*/
Boolean topo_srv_is_btm_steer_disallowed(struct own_1905_device *ctx, unsigned char *mac)
{
	struct sta_db *sta;
        struct steer_policy *spolicy = &ctx->map_policy.spolicy;

	if (spolicy->btm_disallow_count) {
		SLIST_FOREACH(sta, &spolicy->btm_disallow_head, sta_entry) {
			if (os_memcmp(mac, sta->mac, ETH_ALEN) == 0)
				return TRUE;
		}
	}

	return FALSE;
}

/**
* @brief Fn to check whether local sterring is allowed or not
*
* @param ctx own 1905 device context
* @param mac station mac
*
* @return 0/1
*/
Boolean topo_srv_is_local_steer_disallowed(struct own_1905_device *ctx, unsigned char *mac)
{
	struct sta_db *sta;
        struct steer_policy *spolicy = &ctx->map_policy.spolicy;

	if (spolicy->local_disallow_count) {
		SLIST_FOREACH(sta, &spolicy->local_disallow_head, sta_entry) {
			if (os_memcmp(mac, sta->mac, ETH_ALEN) == 0)
				return TRUE;
		}
	}

	return FALSE;
}

/**
* @brief Fn to update metrics policy in wapp
*
* @param ctx own 1905 device context
* @param mpolicy metrics policy pointer
*/
void wapp_set_metrics_policy_setting(struct own_1905_device *ctx, struct metrics_policy *mpolicy)
{
	unsigned char buf[128] = { 0 };
	int datalen = 0;
	struct metric_policy_head *policy_head = NULL;
	struct metric_policy *policy = NULL;
	struct metric_policy_db *policy_db = NULL;

	datalen = 2 + mpolicy->radio_num * sizeof(struct metric_policy_db);
	policy = (struct metric_policy *)buf;
	policy->report_interval = mpolicy->report_interval;
	policy->policy_cnt = mpolicy->radio_num;
	policy_head = policy->policy;
	SLIST_FOREACH(policy_db, &(mpolicy->policy_head), policy_entry) {
		memcpy(policy_head->identifier, policy_db->identifier, ETH_ALEN);
		policy_head->rssi_thres = policy_db->rssi_thres;
		policy_head->hysteresis_margin = policy_db->hysteresis_margin;
		policy_head->ch_util_thres = policy_db->ch_util_thres;
		policy_head->sta_stats_inclusion = policy_db->sta_stats_inclusion;
		policy_head->sta_metrics_inclusion = policy_db->sta_metrics_inclusion;
		debug("MAP policy_head->ch_util_thres %d", policy_head->ch_util_thres);
		policy_head++;
	}
	map_get_info_from_wapp(ctx, WAPP_USER_SET_METIRCS_POLICY, 0, NULL, NULL, (void *)policy, datalen);

}

/**
* @brief Fn to parse map policy data
*
* @param ctx own 1905 device pointer
* @param buf msg buffer
*
* @return 0 if success else -1
*/
int parse_cli_assoc_control_request_tlv(unsigned char *buf,
	struct own_1905_device *ctx, u8 *al_mac)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == CLI_ASSOC_CONTROL_REQUEST_TYPE) {
		temp_buf++;
	}
	else {
		return -1;
	}

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);
	temp_buf += 2;
	update_client_coordination_state_for_assoc_control((struct mapd_global *)ctx->back_ptr, (struct cli_assoc_control *)temp_buf, length);
	client_mon_handle_assoc_control((struct mapd_global *)ctx->back_ptr,(struct cli_assoc_control *)temp_buf, length, al_mac);
	return (length+3);
}

int parse_cli_assoc_control_request_message(struct own_1905_device *ctx,
	unsigned char *buf, u8 len)
{
	int length =0;
	unsigned char *temp_buf;
	unsigned int integrity = 0;
	unsigned int right_integrity = 0x1;

	temp_buf = buf;
	chan_mon_check_steer_triggered((struct mapd_global *)ctx->back_ptr, buf, len);
	temp_buf += ETH_ALEN;
	while(1) {
		if (*temp_buf == CLI_ASSOC_CONTROL_REQUEST_TYPE) {
			integrity |= (1 << 0);
			length = parse_cli_assoc_control_request_tlv(temp_buf, ctx, buf);
			if(length < 0)
			{
				err("error cli assoc control request tlv\n");
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
	/*check integrity*/
	if (integrity != right_integrity) {
		err("incomplete client association control request 0x%x 0x%x\n",
			integrity, right_integrity);
		return -1;
	}

	return 0;
}

int parse_map_policy_config_request_message(struct own_1905_device *ctx, unsigned char *buf)
{
	int length = 0;
	unsigned char *temp_buf;
	unsigned int integrity = 0;

	temp_buf = buf;

	while (1) {
		if (*temp_buf == STEERING_POLICY_TYPE) {
			integrity |= (1 << STEERING_POLICY_TYPE_CHECK);

			/*delete the previously reserved steering policy */
			delete_exist_steering_policy(&ctx->map_policy.spolicy);

			length = parse_steering_policy_tlv(temp_buf, ctx);
			if (length < 0) {
				err("error steering request tlv");
				return -1;
			}


			temp_buf += length;

		} else if (*temp_buf == METRIC_REPORTING_POLICY_TYPE) {
			integrity |= (1 << METRIC_REPORTING_POLICY_TYPE_CHECK);
			/*delete the previously reserved metrics policy */
			delete_exist_metrics_policy(&ctx->map_policy.mpolicy);

			length = parse_metric_reporting_policy_tlv(temp_buf, ctx);
			if (length < 0) {
				err("error metric reporting policy tlv");
				return -1;
			}

			temp_buf += length;

#ifdef MAP_R2
		} else if (*temp_buf == CHANNEL_SCAN_REPORTING_POLICY_TYPE) {
			// TODO: Raghav. Currently don't support independent scans. so no use
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		} else if (*temp_buf == UNSUCCESSFUL_ASSOCIATION_POLICY_TYPE) {

			integrity |= (1 << 7);

			length = parse_unsuccessful_association_policy_tlv(temp_buf, ctx);
			if (length < 0) {
				err("error unsuccessful association policy tlv");
				return -1;
			}
			//code to remove
			//return 0;

			temp_buf += length;
#endif
		} else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
	/*check integrity */
#ifdef MAP_R2
	if ((integrity & ((1 << STEERING_POLICY_TYPE_CHECK) | (1 << METRIC_REPORTING_POLICY_TYPE_CHECK)
						| (1 << 7)))
	    == 0) {
		err("incomplete policy config request 0x%x 0x%x",
			     integrity,
			     ((1 << STEERING_POLICY_TYPE_CHECK) | (1 << METRIC_REPORTING_POLICY_TYPE_CHECK)
			     		| (1 << 7)));
		return -1;
	}

#else
	if ((integrity & ((1 << STEERING_POLICY_TYPE_CHECK) | (1 << METRIC_REPORTING_POLICY_TYPE_CHECK)))
	    == 0) {
		err("incomplete policy config request 0x%x 0x%x",
			     integrity,
			     ((1 << STEERING_POLICY_TYPE_CHECK) | (1 << METRIC_REPORTING_POLICY_TYPE_CHECK)));
		return -1;
	}
#endif
	return 0;
}

/**
* @brief Fn to handler btm report
*
* @param ctx own 1905 device context
* @param msg_buf msg buffer
* @param len msg len
*
* @return 0
*/
int topo_srv_handle_client_steer_btm_report(struct own_1905_device *ctx, unsigned char *msg_buf, int len)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, msg_buf);

	if (parse_client_steering_btm_report_message(ctx, dev, msg_buf + ETH_ALEN) < 0) {
		err("failed to parse resp msg");
		return -1;
	}

	return 0;
}

/**
* @brief Fn to handle steer complete msg from a 1905 device
*
* @param ctx own 1905 device
* @param msg_buf msg buffer
* @param len msg len
*
* @return 0 if success else -1
*/
int topo_srv_handle_steer_complete(struct own_1905_device *ctx, unsigned char *msg_buf, int len)
{
	struct _1905_map_device *_1905_device = topo_srv_get_1905_device(ctx, msg_buf);

	if(_1905_device == NULL) {
		mapd_printf(MSG_ERROR, "1905 device not in DB");
		return 1;
	}
#ifdef CENT_STR
	if(ctx->cent_str_en){
		mapd_printf(MSG_ERROR, "Steer complete Received...");
	}
	else
#endif
	chan_mon_handle_steer_complete(ctx,_1905_device);

	return 0;
}

/**
* @brief Fn to handle backhaul steer msg from a 1905 device
*
* @param ctx own 1905 device context
* @param msg_buf msg buffer
* @param len msg buffer len
*
* @return 0
*/
int topo_srv_handle_backhaul_steer_rsp(struct own_1905_device *ctx, unsigned char *msg_buf, int len)
{

	struct _1905_map_device *_1905_dev = NULL;
	if (ctx->network_optimization.network_optimization_enabled)
	{
		SLIST_FOREACH(_1905_dev, &ctx->_1905_dev_head, next_1905_device) {
			err("1905 dev ALMAC "MACSTR" ",MAC2STR(_1905_dev->_1905_info.al_mac_addr));
			err("network_opt_device_state %d",_1905_dev->network_opt_per1905.network_opt_device_state);
			if(_1905_dev->network_opt_per1905.network_opt_device_state == NETOPT_STATE_OPT_NET_REALIZATION_ONGOING)
			{
				_1905_dev->network_opt_per1905.network_opt_device_state = NETOPT_STATE_OPT_NET_BHSTEER_DONE;
				err("mark BH steer complete");
			}
		}
	}
	return 0;
}
#ifdef MAP_R2
void reset_de_if_needed(struct own_1905_device *ctx)
{
	struct _1905_map_device *next_dev = NULL;
	u8 flag = 0;

	SLIST_FOREACH(next_dev, &ctx->_1905_dev_head, next_1905_device) {
		if (next_dev->de_done == 1)
			flag = 1;
	}
	if (flag == 0 && ctx->de_state != OFF) {
		err("set de state off");
		ctx->de_state = OFF;
	}
}
#endif

int topo_srv_handle_ap_metrics_rsp(struct own_1905_device *ctx, unsigned char *buf, int len)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, buf);
	if (!dev) {
		err("mentioned 1905 device not found");
		return -1;
	}
	buf += ETH_ALEN;
	debug("topo_srv_handle_ap_metrics_rsp from "MACSTR"",MAC2STR(dev->_1905_info.al_mac_addr));
	if (parse_ap_metrics_response_message(ctx, dev, buf) < 0) {
		err("failed to parse resp msg");
		return -1;
	}
#ifdef MAP_R2
	dev->de_done = 0;
	if (ctx->de_state != OFF) {
		update_policy_config((struct mapd_global *)ctx->back_ptr, dev, 0);
		if (eloop_is_timeout_registered(de_stats_timeout,
				(void *)ctx->back_ptr, (void *)dev)) {
				err("Timer cancelled for: "MACSTR, MAC2STR(dev->_1905_info.al_mac_addr));
			eloop_cancel_timeout(de_stats_timeout, (void *)ctx->back_ptr, (void *)dev);
		}
		reset_de_if_needed(ctx);
	}
#endif
	return 0;
}
void mapd_user_ch_pref_send(
	struct mapd_global *global,
	unsigned char *al_mac, unsigned char *identifier, struct prefer_info_db *prefer_db)
{
	struct mapd_user_iface_ch_pref_event *client_notif = NULL;
	struct mapd_user_event *user_event = NULL;
	struct ctrl_iface_global_priv *priv = global->ctrl_iface;

	user_event = (struct mapd_user_event *)os_zalloc(sizeof(struct mapd_user_event) +
		sizeof(struct mapd_user_iface_ch_pref_event));
	if (!user_event) {
		err("mem alloc failed");
		return;
	}
	os_memset(user_event, 0, sizeof(struct mapd_user_event) +
		sizeof(struct mapd_user_iface_ch_pref_event));

	user_event->event_id = CH_PREF_NOTIF;
	client_notif = (struct mapd_user_iface_ch_pref_event *)user_event->event_body;

	os_memcpy(client_notif->almac, al_mac, ETH_ALEN);
	os_memcpy(client_notif->radio_id, identifier, ETH_ALEN);
	client_notif->ch_num = prefer_db->ch_num;
	client_notif->op_class = prefer_db->op_class;
	client_notif->perference= prefer_db->perference;
	client_notif->reason = prefer_db->reason;
	os_memcpy(client_notif->ch_list, prefer_db->ch_list, MAX_CH_NUM);
	if (!dl_list_empty(&priv->ctrl_dst)) {
		mapd_ctrl_iface_send(global,
							priv->sock,
							&priv->ctrl_dst,
							(const char *)user_event, sizeof(struct mapd_user_event) +
							sizeof(struct mapd_user_iface_ch_pref_event),
							priv);
	}
	os_free(user_event);
}

void send_user_ch_operable_results(struct own_1905_device *ctx)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	struct radio_info_db *radio_info = NULL;
	struct prefer_info_db *prefer_db = NULL;
	while (dev != NULL) {
		if (dev->in_network) {
			radio_info = topo_srv_get_radio(dev, NULL);
				while (radio_info) {
					SLIST_FOREACH(prefer_db, &radio_info->chan_preferance.prefer_info_head, prefer_info_entry) {
						mapd_user_ch_pref_send((struct mapd_global *)ctx->back_ptr,
							dev->_1905_info.al_mac_addr, radio_info->identifier,
							prefer_db);
					}
				radio_info = topo_srv_get_next_radio(dev, radio_info);
				}
			}
		dev = topo_srv_get_next_1905_device(ctx, dev);
	}
}

/**
 * @brief Fn to parse channel prefence report from a 1905 device
 *
 * @param dev 1905 device pointer
 * @param buf msg buffer
 *
 * @return 0 if success else -1
 */
int parse_channel_preference_report_message(struct own_1905_device *ctx,
				struct _1905_map_device *dev, unsigned char *buf)
{
	int length = 0;
	unsigned char *temp_buf;
	unsigned int integrity = 0;
	struct _1905_map_device *own_map_dev = topo_srv_get_1905_device(ctx,NULL);
	struct radio_info_db *radio = topo_srv_get_radio(own_map_dev,
		NULL);
	temp_buf = buf + 6;

	while (1) {
		if (*temp_buf == CH_PREFERENCE_TYPE) {
			integrity |= (1 << CH_PREFERENCE_TYPE_CHECK);
			length = parse_channel_preference_tlv(ctx, temp_buf, dev);
			if (length < 0) {
				err("error channel preference tlv");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == RADIO_OPERATION_RESTRICTION_TYPE) {
			integrity |= (1 << RADIO_OPERATION_RESTRICTION_TYPE_CHECK);
			length = parse_radio_operation_restriction_tlv(temp_buf, dev);
			if (length < 0) {
				err("error operation restriction tlv");
				return -1;
			}

			temp_buf += length;
		} else if (*temp_buf == CAC_COMPLETION_REPORT_TYPE) {
				length = parse_cac_completion_tlv(ctx,temp_buf,dev);
				if (length < 0) {
						err("error operation restriction tlv");
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
	if (ctx->ch_planning.ch_planning_enabled) {
		while (radio) {
			if (!ch_planning_check_channel_operable_wrapper(ctx, radio->channel[0])) {
#ifdef MAP_R2
				ctx->Restart_ch_planning_radar_on_agent = 1;
#endif
				mapd_reset_first_seen_for_all_dev(ctx,0);
				mapd_restart_channel_plannig(ctx->back_ptr);
				break;
			}
			radio = topo_srv_get_next_radio(own_map_dev, radio);
		}
	}
	//ch_planning_show_ch_distribution(ctx);

	return 0;
}

/**
* @brief Fn to delete an agent's channel preference
*
* @param ctx own 1905 device context
* @param dev 1905 device pointer
*
* @return 0 if success else -1
*/
int delete_agent_ch_prefer_info(struct own_1905_device *ctx, struct _1905_map_device *dev)
{
	struct prefer_info_db *prefer = NULL, *prefer_tmp = NULL;
	struct restrict_db *restrict_var = NULL, *restrict_tmp = NULL;
	int i = 0;
	struct radio_info_db *tmp_radio = topo_srv_get_radio(dev, NULL);
	struct radio_ch_prefer *ch_prefer;
	while (tmp_radio) {
		ch_prefer = &tmp_radio->chan_preferance;
		ch_prefer->is_valid = 0;

		debug("agent(%02x:%02x:%02x:%02x:%02x:%02x)",
			     PRINT_MAC(dev->_1905_info.al_mac_addr));
		prefer= SLIST_FIRST(&ch_prefer->prefer_info_head);
		while (prefer) {
			debug("opclass=%d, ch_num=%d perference=%d, reason=%d",
			     prefer->op_class, prefer->ch_num, prefer->perference, prefer->reason);
			debug("ch_list: ");
			prefer_tmp = SLIST_NEXT(prefer, prefer_info_entry);
			SLIST_REMOVE(&(ch_prefer->prefer_info_head), prefer, prefer_info_db, prefer_info_entry);
			//! below loop should be called after removing from the channel list
			for (i = 0; i < prefer->ch_num; i++) {
				debug("%d ", prefer->ch_list[i]);
				ch_planning_remove_ch_from_prefered_list(ctx,
						prefer->ch_list[i],
						tmp_radio, prefer->ch_list[i] > 14);
			}
			free(prefer);
			prefer = prefer_tmp;
		}

		restrict_var = SLIST_FIRST(&(tmp_radio->chan_restrict.restrict_head));
		while (restrict_var) {
			debug("opclass=%d ch_num=%d", restrict_var->op_class, restrict_var->ch_num);
			debug("ch_list: ");
			for (i = 0; i < restrict_var->ch_num; i++) {
				debug("%d ", restrict_var->ch_list[i]);
			}
			debug("min_fre_sep_list: ");
			for (i = 0; i < restrict_var->ch_num; i++) {
				debug("%d ", restrict_var->min_fre_sep[i]);
			}
			restrict_tmp = SLIST_NEXT(restrict_var, restrict_entry);
			SLIST_REMOVE(&(tmp_radio->chan_restrict.restrict_head), restrict_var, restrict_db, restrict_entry);
			free(restrict_var);
			restrict_var = restrict_tmp;
		}
		tmp_radio = topo_srv_get_next_radio(dev, tmp_radio);
	}
	return 0;
}
int find_bhlink_bw_ch(struct own_1905_device *ctx,
	struct radio_info_db *radio,
	struct bh_link_entry *bh_entry,
	struct channel_bw_info *ch_bw_info)
{
	struct _1905_map_device *peer_map_device = NULL;
	struct radio_info_db *peer_radio = NULL;
	//int bandwidth = BW_20;

	if(bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED)
	{
		info("the upstream dev bssid is "MACSTR"", MAC2STR(bh_entry->bssid));
		if(os_memcmp(bh_entry->bssid,ZERO_MAC_ADDR,ETH_ALEN) == 0)
		{
			err("BSSID info not yet updated");
			return -1;
		}
		peer_map_device = topo_srv_get_1905_by_bssid(ctx,bh_entry->bssid);
		peer_radio = topo_srv_get_radio(peer_map_device, radio->identifier);
		if (!peer_radio) {
			err("peer_radio not found corresponding to identifier");
			return -1;
		}
		ch_bw_info->channel_bw = chan_mon_get_bw_from_op_class(peer_radio->operating_class);

		if (peer_radio->operating_class > 127) {
			ch_bw_info->channel_num =
				ch_planning_get_centre_freq_ch(bh_entry->bh_channel, peer_radio->operating_class);
		} else {
			ch_bw_info->channel_num = bh_entry->bh_channel;
		}
		err("bw connected %d peer_radio->channel %d",ch_bw_info->channel_bw,bh_entry->bh_channel);
	}
	else
	{
		//bh is not connected , so bw is same as unconnected radio's bw
		ch_bw_info->channel_bw = chan_mon_get_bw_from_op_class(radio->operating_class);
		if (radio->operating_class > 127) {
			ch_bw_info->channel_num =
				ch_planning_get_centre_freq_ch(radio->channel[0], radio->operating_class);
		} else {
			ch_bw_info->channel_num = radio->channel[0];
		}
		err("bw not connected %d, radio->channel %d",ch_bw_info->channel_bw,radio->channel[0]);
	}
	os_memcpy(ch_bw_info->iface_addr,  bh_entry->mac_addr, ETH_ALEN);
	return 0;
}

int topo_srv_update_radio_info(struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct channel_report *chan_rpt)
{
	struct ch_rep_info *report_info;
	struct radio_info_db *radio;
	int report_cnt = 0;
	struct bss_info_db *bss = NULL;
	struct mapd_global *global = ctx->back_ptr;
	struct channel_bw_info ch_bw_info;
	struct bh_link_entry *bh_entry = NULL;
	int status = 0;

	if (!dev) {
		err("critical error");
		return -1;
	}
	radio = topo_srv_get_next_radio(dev, NULL);
	/* first time */
	if (!radio)
		SLIST_INIT(&(dev->first_radio));

	debug("total radio count=%d", chan_rpt->ch_rep_num);

	while (report_cnt < chan_rpt->ch_rep_num) {
		report_info = &chan_rpt->info[report_cnt];
		radio = topo_srv_get_radio(dev, report_info->identifier);
		if (!radio) {
			radio = os_zalloc(sizeof(*radio));
			if (!radio) {
				err("mem allocation failed");
				return -1;
			}
			memcpy(radio->identifier, report_info->identifier, ETH_ALEN);
			radio->parent_1905 = dev;
			info("%s, create new radio %p, parent device = %p\n",
				__FUNCTION__, radio, radio->parent_1905);
			radio->operating_channel = NULL;
			SLIST_INSERT_HEAD(&(dev->first_radio), radio, next_radio);
			debug("new radio interface");
			SLIST_INIT(&(radio->link_estimate_cb_head));
			SLIST_INIT(&radio->chan_preferance.prefer_info_head);
			SLIST_INIT(&radio->chan_restrict.restrict_head);
#ifdef MAP_R2
			SLIST_INIT(&radio->first_scan_result);
			SLIST_INIT(&radio->cac_cap.cac_capab_head);
			SLIST_INIT(&radio->cac_comp_status.cac_completion_opcap_head);
#endif
			radio->is_configured = FALSE;
		}
		radio->operating_class = report_info->op_class;
		os_memset(radio->channel, 0, sizeof(radio->channel));
		radio->channel[0] = report_info->channel;
		radio->prev_channel = report_info->channel;
		info("Updated Prev channel: %u", radio->prev_channel);
		/* getting Primary CH for 80MHz BW */
		if (((radio->operating_class == 128)
#ifdef MAP_160BW
			|| (radio->operating_class == 129)
#endif
			) && (report_cnt < (chan_rpt->ch_rep_num-1))) {
			if (is_valid_primary_ch_80M_160M(chan_rpt->info[report_cnt+1].channel, report_info->channel,
								report_info->op_class)) {
				radio->channel[0] = chan_rpt->info[report_cnt+1].channel;
				radio->prev_channel = report_info->channel;
				err("topo_srv_update_radio_info PRIMARY CH %d",radio->channel[0]);
				report_cnt++;
			}
		}
		mapd_fill_secondary_channels(radio->channel, radio->operating_class, 0);
		radio->band = get_band_from_channel(radio->channel[0]);
		radio->radio_capability.max_tx_pwr = report_info->tx_power;
		debug("radio operating_class=%d, channel=%d, max_tx_pwr=%d",
		      radio->operating_class, radio->channel[0], radio->radio_capability.max_tx_pwr);
		debug("radio identifier(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(radio->identifier));
		bss = topo_srv_get_next_bss(radio->parent_1905,
			bss);
		while (bss) {
			if (bss->radio == radio) {
				if (radio->operating_class > 127) {
					ch_bw_info.channel_num =
						ch_planning_get_centre_freq_ch(radio->channel[0], radio->operating_class);
				} else {
					ch_bw_info.channel_num =
						radio->channel[0];
				}
				ch_bw_info.channel_bw = chan_mon_get_bw_from_op_class(radio->operating_class);

				os_memcpy(ch_bw_info.iface_addr, bss->bssid, ETH_ALEN);
				map_1905_Set_Channel_BandWidth(global->_1905_ctrl,
					&ch_bw_info);
			}
			bss = topo_srv_get_next_bss(radio->parent_1905,
				bss);
		}

		ch_planning_update_ch_ditribution(ctx, dev,
		radio, radio->channel[0], radio->operating_class);
		SLIST_FOREACH(bh_entry,&(ctx->bh_link_head),next_bh_link)
		{
			if(os_memcmp(bh_entry->radio_identifier,radio->identifier,ETH_ALEN) == 0)
			{
				info("bh entry identifier(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(radio->identifier));
				info("bh entry mac address(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bh_entry->mac_addr));
				bh_entry->bh_channel = radio->channel[0];
				status= find_bhlink_bw_ch(ctx,radio,bh_entry,&ch_bw_info);
				if(status < 0)
					break;
				info("ch_bw_info mac address(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(ch_bw_info.iface_addr));
				info("ch_bw_info.channel_num %d",ch_bw_info.channel_num);
				info("ch_bw_info.channel_bw %d",ch_bw_info.channel_bw);
				map_1905_Set_Channel_BandWidth(global->_1905_ctrl,
						&ch_bw_info);
				radio->bh_priority = bh_entry->priority_info.priority;
				break;
			}
		}
		report_cnt++;	
	}
	return 0;
}

/**
* @brief Fn to update own 1905 radio info from wapp
*
* @param ctx own 1905 device context
* @param buf msg buffer
*
* @return 0 if success else -1
*/
int topo_srv_update_own_radio_info(struct own_1905_device *ctx, unsigned char *buf)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	struct channel_report *chan_rpt = (struct channel_report *)(buf);

	if (!dev) {
		err("critical error");
		return -1;
	}
	topo_srv_update_radio_info(ctx, dev, chan_rpt);
	return 0;
}
/**
* @brief Fn to update ap caps for a 1905 device
*
* @param dev 1905 device
* @param cap ap caps
*
* @return 0
*/
int topo_srv_update_1905_ap_cap(struct _1905_map_device *dev, struct ap_capability *cap)
{
	dev->ap_cap.sta_report_on_cop = cap->sta_report_on_cop;
	dev->ap_cap.sta_report_not_cop = cap->sta_report_not_cop;
	dev->ap_cap.rssi_steer = cap->rssi_steer;

	return 0;
}

/**
* @brief Fn to update ap caps for own 1905 device
*
* @param ctx own 1905 device ctx
* @param cap ap caps
*
* @return 0 if success else -1
*/
int topo_srv_update_ap_cap(struct own_1905_device *ctx, struct ap_capability *cap)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	if (!dev) {
		err("own 1905 device is missing");
		return -1;
	}
	topo_srv_update_1905_ap_cap(dev, cap);
	return 0;
}
#ifdef MAP_R2
int topo_srv_update_metric_rep_intv_cap(struct own_1905_device *ctx, u32 *cap)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	if (!dev) {
		err("own 1905 device is missing");
		return -1;
	}
	dev->metric_rep_interval = *cap;
	err("dev->metric_rep_interval: %d", dev->metric_rep_interval);
	return 0;
}
#endif


/**
* @brief Fn to update own bss info from wapp
*
* @param ctx own 1905 device
* @param buf msg buffer
*
* @return 0 if success else -1
*/
	/* 802.11 authentication and key management */

unsigned short WscGetEncryType(
	unsigned int encryType)
{
	if (IS_CIPHER_NONE(encryType))
		return WSC_ENCRTYPE_NONE;
	else if (IS_CIPHER_WEP(encryType))
		return WSC_ENCRTYPE_WEP;
	else if (IS_CIPHER_TKIP(encryType) && IS_CIPHER_CCMP128(encryType))
		return WSC_ENCRTYPE_AES | WSC_ENCRTYPE_TKIP;
	else if (IS_CIPHER_TKIP(encryType))
		return WSC_ENCRTYPE_TKIP;
	else if (IS_CIPHER_CCMP128(encryType))
		return WSC_ENCRTYPE_AES;
	else
		return WSC_ENCRTYPE_AES;
}

unsigned short WscGetAuthType(
	unsigned int authType)
{
	if (IS_AKM_OPEN(authType))
		return WSC_AUTHTYPE_OPEN;
	else if (IS_AKM_SHARED(authType))
		return WSC_AUTHTYPE_SHARED;
	else if (IS_AKM_WPANONE(authType))
		return WSC_AUTHTYPE_WPANONE;
	else if (IS_AKM_WPA1(authType) && IS_AKM_WPA2(authType))
		return WSC_AUTHTYPE_WPA | WSC_AUTHTYPE_WPA2;
	else if (IS_AKM_WPA1PSK(authType) && IS_AKM_WPA2PSK(authType))
		return WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK;
	else if (IS_AKM_WPA1(authType))
		return WSC_AUTHTYPE_WPA;
	else if (IS_AKM_WPA1PSK(authType))
		return WSC_AUTHTYPE_WPAPSK;
	else if (IS_AKM_WPA2(authType))
		return WSC_AUTHTYPE_WPA2;
	else if (IS_AKM_WPA2PSK(authType))
		return WSC_AUTHTYPE_WPA2PSK;
	else
		return WSC_AUTHTYPE_OPEN;
}

void send_wapp_event_wireless_settings(struct own_1905_device *ctx,
	struct bss_info_db *bss_bh)
{
	unsigned char *buffer = NULL;
	struct wsc_config *wsc_config_msg;

	buffer = os_malloc(sizeof(struct wsc_config) + sizeof(struct wireless_setting));
	wsc_config_msg = (struct wsc_config *)buffer;
	os_memset(buffer, 0, sizeof(struct wsc_config) + sizeof(struct wireless_setting));
	wsc_config_msg->num = 1;
	wsc_config_msg->setting[0].AuthMode = WscGetAuthType(bss_bh->auth_mode);
	wsc_config_msg->setting[0].EncrypType = WscGetEncryType(bss_bh->enc_type);
	os_memcpy(wsc_config_msg->setting[0].WPAKey, bss_bh->key, bss_bh->key_len);
	os_memcpy(wsc_config_msg->setting[0].Ssid, bss_bh->ssid, bss_bh->ssid_len);
	os_memcpy(wsc_config_msg->setting[0].mac_addr, bss_bh->bssid, ETH_ALEN);
	wsc_config_msg->setting[0].map_vendor_extension = bss_bh->map_vendor_extn;
	wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_BH_WIRELESS_SETTING,
		0, bss_bh->bssid, 0, wsc_config_msg, sizeof(struct wsc_config) + sizeof(struct wireless_setting), 0, 0, 0);
	os_free(buffer);
}

int topo_srv_update_own_bss_info(struct own_1905_device *ctx, unsigned char *buf)
{
	struct oper_bss_cap *bss_cap = (struct oper_bss_cap *)buf;
	struct op_bss_cap *bss_report;
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	struct bss_info_db *bss;
	int bss_cnt;
	struct radio_info_db *radio;

	if (!dev) {
		err("own 1905 device is missing");
		return -1;
	}
	radio = topo_srv_get_radio(dev, bss_cap->identifier);
	if (!radio) {
		err("radio not found corresponding to identifier");
		return -1;
	}
	radio->band = get_band_from_channel(radio->channel[0]);
	topo_srv_mark_all_oper_bss_invalid(ctx, dev, radio);
	info("received bss info for %d", bss_cap->oper_bss_num);
	if (bss_cap->oper_bss_num == 0)
		return 0;

	for (bss_cnt = 0; bss_cnt < bss_cap->oper_bss_num; bss_cnt++) {
		bss_report = &bss_cap->cap[bss_cnt];
		bss = topo_srv_get_bss_by_bssid(ctx, dev, bss_report->bssid);
		if (!bss) {
			bss = os_zalloc(sizeof(*bss));
			if (!bss) {
				err("mem allocation failed");
				return -1;
			}
			memcpy(bss->bssid, bss_report->bssid, ETH_ALEN);
			SLIST_INIT(&bss->esp_head);
			SLIST_INSERT_HEAD(&(dev->first_bss), bss, next_bss);
#ifdef ACL_CTRL
			dl_list_init(&bss->acl_cli_list);
#endif
		}
		bss->valid = TRUE;
		bss->ssid_len = bss_report->ssid_len;
		bss->radio = radio;
		debug("previous map_vendor_extn =%d,  new map_vendor_extension =%d", bss->map_vendor_extn,
				bss_report->map_vendor_extension);
		bss->map_vendor_extn = bss_report->map_vendor_extension;
		memcpy(bss->ssid, bss_report->ssid, bss->ssid_len);
		bss->auth_mode = bss_report->auth_mode;
		bss->enc_type = bss_report->enc_type;

		bss->key_len = bss_report->key_len;

		os_memcpy(bss->key, bss_report->key, bss->key_len);

		debug("bssid(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bss->bssid));
	}
	topo_srv_remove_all_invalid_bss(ctx, dev);

	return 0;
}

/**
* @brief Fn to handler link metrics event from 1905
*
* @param ctx own 1905 device context
* @param buf msg buffer
* @param len msg len
*
* @return  0 if sccess else -1
*/
int topo_srv_handle_link_metrics_rsp_event(struct own_1905_device *ctx, unsigned char *buf, int len)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, buf);

	debug("got LINK_METRICS_RESPONSE from (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(buf));
	if (!dev) {
		debug("error! no agent info exist");
		return -1;
	}
	/*parse ap metrics response message for the agent */
	if (0 > parse_link_metrics_response_message(dev, buf + ETH_ALEN)) {
		debug("error! parse link metrics response message");
	}

	return 0;
}

/**
* @brief Fn to update channel preferece
*
* @param ctx own 1905 device ctx
* @param msg_buf msg buffer
* @param len msg len
*
* @return 0 if success else -1
*/
int topo_srv_update_chan_preference(struct own_1905_device *ctx, unsigned char *msg_buf, int len)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, msg_buf);

	debug("got CHANNLE_PREFERENCE_REPORT");
	if (!dev) {
		err("failed to get device");
		return -1;
	}

	/*delete all the stored channel preference information for the agent */
	delete_agent_ch_prefer_info(ctx, dev);
	/*parse the channel preference report message for the agent */
	if (0 > parse_channel_preference_report_message(ctx, dev, msg_buf)) {
		err("error! parse channel preference report message");
		return -1;
	}

	return 0;
}

/**
* @brief Fn to update channel info
*
* @param ctx own 1905 device ctx
* @param bss bss pointer
*
* @return 0 if success else error
*/
int topo_srv_update_channel_info(struct own_1905_device *ctx, struct bss_info_db *bss)
{
	struct topology_channel *chan = NULL;
	int i = 0;

	for (i = 0; i < MAX_CHANNEL_BLOCKS; i++) {
		if (!bss->radio->channel[i])
			continue;
		SLIST_FOREACH(chan, &ctx->channel_head, next_channel) {
			if (chan->channel_no == bss->radio->channel[i]) {
				chan->channel_util = bss->ch_util;
				break;
			}
		}
		if (!chan) {
			chan = os_zalloc(sizeof(*chan));
			chan->channel_no = bss->radio->channel[i];
			chan->channel_util = bss->ch_util;
			SLIST_INSERT_HEAD(&(ctx->channel_head), chan, next_channel);
		}
	}
	return 0;
}

/**
* @brief Fn to update bss channel util
*
* @param ctx own 1905 device ctx
* @param minfo metics info pointer
*
* @return 0 if success else error
*/
int topo_srv_update_bss_chan_util(struct own_1905_device *ctx, struct ap_metrics_info *minfo)
{
	struct bss_info_db *mrsp = NULL;

	mrsp = topo_srv_get_bss_by_bssid(ctx, NULL, minfo->bssid);
	if (!mrsp) {
		err("bss not found");
		return -1;
	}
	mrsp->ch_util = minfo->ch_util;
	mrsp->assoc_sta_cnt = minfo->assoc_sta_cnt;
	topo_srv_update_channel_info(ctx, mrsp);
#ifdef MAP_R2
	debug(" own dev ap metric");
	topo_srv_update_ch_planning_info(ctx,SLIST_FIRST(&ctx->_1905_dev_head),mrsp,NULL,0);
#endif
#ifdef CENT_STR
	if(ctx->cent_str_en && ctx->device_role == DEVICE_ROLE_CONTROLLER)
		cent_str_cu_monitor(ctx,mrsp);
#endif


	return 0;
}

/**
* @brief Fn to insert new metics info
*
* @param ctx own 1905 device ctx
* @param minfo metics info pointer
*
* @return 0 if success else error
*/
int insert_new_metrics_info(struct own_1905_device *ctx, struct ap_metrics_info *minfo)
{
	struct bss_info_db *mrsp = NULL;
	struct esp_db *esp = NULL;
	struct esp_info *info = NULL;
	int i = 0;

	mrsp = topo_srv_get_bss_by_bssid(ctx, NULL, minfo->bssid);
	if (!mrsp) {
		err("bss not found");
		return -1;
	}
	mrsp->ch_util = minfo->ch_util;
	mrsp->assoc_sta_cnt = minfo->assoc_sta_cnt;
	mrsp->esp_cnt = minfo->valid_esp_count;
#ifdef MAP_R2
	//err("storing ap metric info in map");
	mrsp->bc_rx = minfo->ext_ap_metric.bc_rx;
	mrsp->bc_tx = minfo->ext_ap_metric.bc_tx;
	mrsp->mc_rx = minfo->ext_ap_metric.mc_rx;
	mrsp->mc_tx = minfo->ext_ap_metric.mc_tx;
	mrsp->uc_rx = minfo->ext_ap_metric.uc_rx;
	mrsp->uc_tx = minfo->ext_ap_metric.uc_tx;
#endif
	SLIST_INIT(&(mrsp->esp_head));

	debug("insert struct mrsp_db");
	debug("bssid(%02x:%02x:%02x:%02x:%02x:%02x) ch_uti=%d, assoc_sta_cnt=%d, esp_cnt=%d",
			PRINT_MAC(mrsp->bssid), mrsp->ch_util, mrsp->assoc_sta_cnt, mrsp->esp_cnt);

	for (i = 0; i < mrsp->esp_cnt; i++) {
		info = &minfo->esp[i];
		esp = (struct esp_db *)os_zalloc(sizeof(struct esp_db));
		if (!esp) {
			err("alloc struct esp_db fail");
			return -1;
		}
		esp->ac = info->ac;
		esp->format = info->format;
		esp->ba_win_size = info->ba_win_size;
		esp->e_air_time_fraction = info->e_air_time_fraction;
		esp->ppdu_dur_target = info->ppdu_dur_target;
		SLIST_INSERT_HEAD(&(mrsp->esp_head), esp, esp_entry);

		debug("insert struct esp_db");
		debug("ac=%d, format=%d ba_win_size=%d", esp->ac, esp->format, esp->ba_win_size);
		debug("e_air_time_fraction=%d, ppdu_dur_target=%d esp =%p",
			     esp->e_air_time_fraction, esp->ppdu_dur_target, esp);
	}
	topo_srv_update_channel_info(ctx, mrsp);
#ifdef CENT_STR
	if(ctx->cent_str_en && ctx->device_role == DEVICE_ROLE_CONTROLLER)
		cent_str_cu_monitor(ctx,mrsp);
#endif

	return 0;

}
#ifdef MAP_R2
void topo_srv_update_ch_planning_info(
	struct own_1905_device * ctx,
	struct _1905_map_device *dev,
	struct bss_info_db *bss,
	struct radio_info_db *radio,
	u8 cu_tlv_update)
{
	if(!dev) {
		return;
	}
	if((ctx->ch_planning_R2.ch_plan_enable == TRUE) &&
		(ctx->device_role == DEVICE_ROLE_CONTROLLER))
	{
		ch_planning_handle_metric_report(ctx,dev,bss,radio,cu_tlv_update,0);
	}
}
int insert_new_radio_metrics_info(struct own_1905_device *ctx, struct radio_metrics_info *minfo)
{
	struct mapd_radio_info *radio_info = NULL;
	struct mapd_global *global = ctx->back_ptr;

	debug(MACSTR"\n", MAC2STR(minfo->ra_id));
	radio_info = get_radio_info_by_radio_id(global, minfo->ra_id);
	if(radio_info == NULL)
		return -1;
	radio_info->radio_metrics.cu_noise = minfo->cu_noise;
	radio_info->radio_metrics.cu_tx = minfo->cu_tx;
	radio_info->radio_metrics.cu_rx = minfo->cu_rx;
	radio_info->radio_metrics.cu_other = minfo->cu_other;
	radio_info->radio_metrics.edcca = minfo->edcca;
	debug("### radio_info->radio_metrics.cu_rx  = %d ###\n", radio_info->radio_metrics.cu_rx);
	debug("### radio_info->radio_metrics.cu_other  = %d ###\n", radio_info->radio_metrics.cu_other);
	debug("minfo->cu_noise: %d", minfo->cu_noise);
	debug("minfo->cu_tx: %d", minfo->cu_tx);
	debug("minfo->cu_rx: %d", minfo->cu_rx);
	debug("minfo->cu_other: %d", minfo->cu_other);
	debug("minfo->edcca: %d", minfo->edcca);
	os_memcpy(radio_info->radio_metrics.ra_id, minfo->ra_id, ETH_ALEN);

	debug(MACSTR"\n", MAC2STR(radio_info->radio_metrics.ra_id));
	struct radio_info_db *radio =
		topo_srv_get_radio_by_channel(SLIST_FIRST(&ctx->_1905_dev_head),radio_info->channel);
	if(!radio) {
		err("Radio not found some issue ");
		return -1;
	}
	radio->cu_distribution.ch_num = radio_info->channel;
	radio->cu_distribution.edcca_airtime = radio_info->radio_metrics.edcca;
	radio->radio_metrics.cu_noise = radio_info->radio_metrics.cu_noise;
	radio->radio_metrics.cu_tx = radio_info->radio_metrics.cu_tx;
	radio->radio_metrics.cu_rx = radio_info->radio_metrics.cu_rx;
	radio->radio_metrics.cu_other = radio_info->radio_metrics.cu_other;
	os_memcpy(radio->radio_metrics.ra_id, radio_info->radio_metrics.ra_id, ETH_ALEN);
	debug(" owndev rx ch %d radio metric cu_other %d, cu_tx %d cu rx %d",
		radio->channel[0], radio->radio_metrics.cu_other,radio->radio_metrics.cu_tx,
		radio->radio_metrics.cu_rx);
	/* for owndev cu tlv info and radio info come in same wapp event so need to save all info */
	topo_srv_update_ch_planning_info(ctx,SLIST_FIRST(&ctx->_1905_dev_head),NULL,radio,1);
	topo_srv_update_ch_planning_info(ctx,SLIST_FIRST(&ctx->_1905_dev_head),NULL,radio,0);
	return 0;

}
#endif

/**
* @brief Fn to get a bss based on bssid
*
* @param _1905_device 1905 device pointer
* @param bssid bssid of the bss
*
* @return bss pointer if found else NULL
*/
struct bss_info_db *topo_srv_get_bss(struct _1905_map_device *_1905_device, char *bssid)
{
	struct bss_info_db *bss = NULL;

	/* Get interface address by ifname, define this API */
	do {
		bss = topo_srv_get_next_bss(_1905_device, bss);
		if (!bss) {
			err("this should have never happened");
			return NULL;
		}
		if (os_memcmp(bss->bssid, bssid, ETH_ALEN) == 0)
			return bss;

	} while (bss);

	return NULL;
}

/* TODO this API can be used to update neighbor informations */
int topo_srv_parse_backhaul_ready_evt(struct own_1905_device *ctx, struct bh_link_info *bh_info)
{
	// TODO update uplink bssid and device from here
	return 0;
}

/* TODO correct this API, discuss whether we can have a single one or multiple are required */
/**
* @brief Fn to update associated clients info
*
* @param ctx own 1905 device pointer
* @param cinfo association event pointer
*
* @return 0 if success else error
*/
int remove_duplicate_cli_single_bh(struct _1905_map_device *device, struct own_1905_device *ctx, unsigned char *ifaddr)
{
	struct _1905_map_device *_1905_dev = NULL;
	struct iface_info *iface = NULL;
	_1905_dev = topo_srv_get_1905_by_iface_addr(ctx, ifaddr);
	if(_1905_dev == NULL) {
		err("No duplicate 1905 dev");
		return -1;
	}
	if(_1905_dev != NULL) {
		err("1905 al mac: (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(_1905_dev->_1905_info.al_mac_addr));
		SLIST_FOREACH(iface, &_1905_dev->_1905_info.first_iface, next_iface) {
			if ((*((char *)(&iface->media_info) + 6) != 0x00) && (os_memcmp(iface->iface_addr, ifaddr, ETH_ALEN) != 0)) {
				err("Explicit Disconnect: (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(iface->iface_addr));
				wlanif_deauth_sta((struct mapd_global *)ctx->back_ptr, iface->iface_addr, NULL);
			}
		}
	}
	return 0;
}

int topo_srv_update_assoc_client_info(struct own_1905_device *ctx, struct map_client_association_event_local *cinfo)
{
	struct _1905_map_device *device = topo_srv_get_1905_device(ctx, NULL);
	struct connected_clients *client;
	struct associated_clients *assoc_client;
	struct bss_info_db *bss = NULL;
	debug("client notification");
	// TODO move connected clients to bss basis
	if (cinfo->assoc_evt == 0x80) {	/*station connect to bss */
		client = (struct connected_clients *)os_zalloc(sizeof(struct connected_clients));
		assoc_client = (struct associated_clients *)os_zalloc(sizeof(struct associated_clients));
		memcpy(client->client_addr, cinfo->sta_mac, ETH_ALEN);
		memcpy(client->_1905_iface_addr, cinfo->bssid, ETH_ALEN);
		memcpy(assoc_client->client_addr, cinfo->sta_mac, ETH_ALEN);
		client->is_bh_link = 0;
		assoc_client->is_bh_link = 0;
		client->is_APCLI = cinfo->is_APCLI;
		assoc_client->is_APCLI = cinfo->is_APCLI;
		bss = topo_srv_get_bss_by_bssid(ctx, device, cinfo->bssid);
		assoc_client->bss = bss;
		if(assoc_client->bss != NULL) {
			memcpy(assoc_client->bss->bssid, cinfo->bssid, ETH_ALEN);
			memcpy(client->bss_addr, cinfo->bssid, ETH_ALEN);
		}

		assoc_client->last_assoc_time = cinfo->assoc_time;

		debug("insert sta(%02x:%02x:%02x:%02x:%02x:%02x) in bss(%02x:%02x:%02x:%02x:%02x:%02x)",
			PRINT_MAC(cinfo->sta_mac), PRINT_MAC(cinfo->bssid));
		if (!ctx->dual_bh_en)
			remove_duplicate_cli_single_bh(device, ctx, cinfo->sta_mac);
		SLIST_INSERT_HEAD(&(device->wlan_clients), client, next_client);
		SLIST_INSERT_HEAD(&(device->assoc_clients), assoc_client, next_client);
		duplicate_sta_check_for_1905_device(ctx, device);
	} else {

		bss = topo_srv_get_bss_by_bssid(ctx, device, cinfo->bssid);
		if(bss == NULL) {
			err("bss is null, something wrong");
			return -1;
		}

		SLIST_FOREACH(client, &(device->wlan_clients), next_client) {
			if ((os_memcmp(client->client_addr, cinfo->sta_mac, ETH_ALEN) == 0) &&(os_memcmp(client->bss_addr, cinfo->bssid, ETH_ALEN)== 0))
				break;
		}
		if (!client) {
			err("something wrong");
			return 0;
		}
		if(os_memcmp(client->_1905_iface_addr, cinfo->bssid, ETH_ALEN) == 0) {
			debug("delete sta(%02x:%02x:%02x:%02x:%02x:%02x) in wlan_clients\n", PRINT_MAC(cinfo->sta_mac));
			SLIST_REMOVE(&device->wlan_clients, client, connected_clients, next_client);
			free(client);
		}
		else {
			err("stale dis-connect event from (%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(cinfo->bssid));
			return -1;
		}
		SLIST_FOREACH(assoc_client, &(device->assoc_clients), next_client) {
			if  ((os_memcmp(assoc_client->client_addr, cinfo->sta_mac, ETH_ALEN) == 0) &&(os_memcmp(assoc_client->bss->bssid, cinfo->bssid, ETH_ALEN)== 0))
				break;
		}
		if (!assoc_client) {
			err("something wrong");
			return 0;
		}
		if(os_memcmp(assoc_client->bss->bssid, cinfo->bssid, ETH_ALEN) == 0) {
			debug("delete sta(%02x:%02x:%02x:%02x:%02x:%02x) in assoc_clients\n", PRINT_MAC(cinfo->sta_mac));
			SLIST_REMOVE(&device->assoc_clients, assoc_client, associated_clients, next_client);
			free(assoc_client);
		}
		else {
			err("stale dis-connect event from (%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(cinfo->bssid));
			return -1;
		}
	}

	return 0;
}

int topo_srv_get_wireless_mode(int phy_mode)
{
	if (phy_mode & 0xC0)
		return MODE_HE;
	else if (phy_mode & 0x20)
		return MODE_VHT;
	else if (phy_mode & 0x10)
		return MODE_HTMIX;
	else if (phy_mode & 0x8)
		return MODE_HTMIX;
	else if (phy_mode & 0x4)
		return MODE_OFDM;
	else if (phy_mode & 0x2)
		return MODE_CCK;
	else if (phy_mode & 0x1)
		return MODE_OFDM;

	/* By default */
	return MODE_VHT;
}

/**
* @brief Fn to update basic caps of a device
*
* @param ctx own 1905 device ctx
* @param dev 1905 device pointer
* @param bcap basic caps pointer
*
* @return 0 if success else error
*/
int topo_srv_update_radio_basic_cap(struct own_1905_device *ctx,
				    struct _1905_map_device *dev, struct ap_radio_basic_cap *bcap)
{
	struct radio_basic_cap *basic_cap = NULL;
	struct ap_radio_basic_capability *bcap_db = NULL;
	struct basic_cap_db *cap = NULL;
	struct basic_cap_db *cap_tmp = NULL;
	int op_class_num = 0;
	int i = 0, j = 0;
	struct radio_info_db *radio;

	if (!dev)
		dev = topo_srv_get_1905_device(ctx, NULL);

	radio = topo_srv_get_radio(dev, bcap->identifier);
	if (!radio) {
		radio = os_zalloc(sizeof(*radio));
		if (!radio) {
			err("mem allocation failed");
			return -1;
		}
		memcpy(radio->identifier, bcap->identifier, ETH_ALEN);
		radio->parent_1905 = dev;
		SLIST_INSERT_HEAD(&(dev->first_radio), radio, next_radio);
		err("new radio interface identifier(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(radio->identifier));
		SLIST_INIT(&(radio->link_estimate_cb_head));
		SLIST_INIT(&radio->chan_preferance.prefer_info_head);
		SLIST_INIT(&radio->chan_restrict.restrict_head);
#ifdef MAP_R2
		SLIST_INIT(&radio->first_scan_result);
		SLIST_INIT(&radio->cac_cap.cac_capab_head);
		SLIST_INIT(&radio->cac_comp_status.cac_completion_opcap_head);
#endif
	}

	radio->wireless_mode = bcap->wireless_mode;

	bcap_db = &radio->radio_capability.basic_caps;

	bcap_db->max_bss_num = bcap->max_bss_num;
	bcap_db->band = bcap->band;
	bcap_db->op_class_num = bcap->op_class_num;

	//hex_dump("insert struct radio_basic_capability_db", bcap_db, 9);
	debug("insert struct radio_basic_capability_db");
	debug("max_bss_num:%d, band:%d, op_class_num:%d",
		     bcap_db->max_bss_num, bcap_db->band, bcap_db->op_class_num);

	if (!SLIST_EMPTY(&(bcap_db->bcap_head))) {
		cap = SLIST_FIRST(&(bcap_db->bcap_head));
		while(cap) {
			cap_tmp = SLIST_NEXT(cap, basic_cap_entry);
			SLIST_REMOVE(&(bcap_db->bcap_head), cap, basic_cap_db, basic_cap_entry);
			os_free(cap);
			cap = cap_tmp;
		}
	}

	SLIST_INIT(&(bcap_db->bcap_head));
	op_class_num = bcap->op_class_num;
	for (i = 0; i < op_class_num; i++) {
		basic_cap = &bcap->opcap[i];
		cap = (struct basic_cap_db *)os_zalloc(sizeof(*cap));
		memcpy(cap, basic_cap, sizeof(*basic_cap));
		SLIST_INSERT_HEAD(&(bcap_db->bcap_head), cap, basic_cap_entry);

		//hex_dump("insert struct basic_cap_db", cap, 16);
		debug("insert struct basic_cap_db");
		debug("opclass:%d, max_tx_pwr=%d, non_operch_num=%d",
			     cap->op_class, cap->max_tx_pwr, cap->non_operch_num);
		debug("non_operch_list: ");

		for (j = 0; j < cap->non_operch_num; j++)
			debug("%d ", cap->non_operch_list[j]);
	}

	return 0;

}

/**
* @brief Fn to update operation channel restirction for a device
*
* @param ctx own 1905 device ctx
* @param dev 1905 device pointer
* @param restrict_cmd restrict pointer
*
* @return 0 if success else -1
*/
int topo_srv_update_operation_restriction(struct own_1905_device *ctx,
					  struct _1905_map_device *dev, struct restriction *restrict_cmd)
{
	int db_cnt;
	struct radio_info_db *radio;
	struct oper_restrict *chan_restrict;
	struct restrict_db *rest_chan, *rest_chan_tmp;
	struct restrict_info *res_info;

	dev = topo_srv_get_1905_device(ctx, NULL);

	if (!dev) {
		err("critical error");
		return -1;
	}
	radio = topo_srv_get_radio(dev, restrict_cmd->identifier);
	if (!radio) {
		err("critical error");
		return -1;
	}
	chan_restrict = &radio->chan_restrict;
	if (!SLIST_EMPTY(&(chan_restrict->restrict_head))) {
		rest_chan = SLIST_FIRST(&(chan_restrict->restrict_head));
		while (rest_chan) {
			rest_chan_tmp = SLIST_NEXT(rest_chan, restrict_entry);
			SLIST_REMOVE(&(chan_restrict->restrict_head), rest_chan, restrict_db, restrict_entry);
			free(rest_chan);
			rest_chan = rest_chan_tmp;
		}
	}
	chan_restrict->is_valid = 1;
	chan_restrict->op_class_num = restrict_cmd->op_class_num;
	SLIST_INIT(&chan_restrict->restrict_head);

	for (db_cnt = 0; db_cnt < chan_restrict->op_class_num; db_cnt++) {
		res_info = &restrict_cmd->opinfo[db_cnt];
		rest_chan = os_zalloc(sizeof(*rest_chan));
		rest_chan->op_class = res_info->op_class;
		rest_chan->ch_num = res_info->ch_num;
		memcpy(rest_chan->ch_list, res_info->ch_list, MAX_CH_NUM);
		memcpy(rest_chan->min_fre_sep, res_info->fre_separation, MAX_CH_NUM);
		SLIST_INSERT_HEAD(&chan_restrict->restrict_head, rest_chan, restrict_entry);
	}

	return 0;
}
#if 0
static void unblock_mbh(void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *ctx = (struct own_1905_device *)eloop_ctx;
	struct bh_link_entry *bh_entry = (struct bh_link_entry *)timeout_ctx;

	bh_entry->mbh_blocked = 0;
	mapd_printf(MSG_OFF, "%s on " MACSTR " is unblocked now",
					bh_entry->ifname, MAC2STR(ctx->al_mac));
}
#endif
void mapd_handle_non_operable_channel(struct own_1905_device *ctx,
	struct radio_info_db *radio)
{
	struct bh_link_entry *bh_entry = NULL;
	unsigned char centre_freq_channel = ch_planning_get_centre_freq_ch(radio->channel[0], radio->operating_class);
	unsigned char channel_non_operable = 0;
	err("centre_freq_channel = %d, radio->operating_channel = %d",
		centre_freq_channel, radio->channel[0]);
	if (!ch_planning_check_channel_operable_wrapper(ctx, radio->channel[0])) {
		channel_non_operable = 1;
	}
	switch (ctx->current_bh_state)
	{
		case BH_STATE_DEFAULT:
		case BH_STATE_WIFI_BOOTUP:
		case BH_STATE_WIFI_LINK_FAIL:
		case BH_STATE_WIFI_BH_STEER:
		case BH_STATE_WIFI_BAND_SWITCHED:
		case BH_STATE_ETHERNET_UPLUGGED:
			always("Do Nothing, non operable channel to be handled in connection states\n");
			break;
		case BH_STATE_ETHERNET_PLUGGED:
			always("Non Operable channel in ethernet BH, inform controller\n");
#ifdef MAP_R2
			_1905_update_channel_pref_report(ctx, NULL, NULL);
#else
			_1905_update_channel_pref_report(ctx, NULL);
#endif
			break;
		case BH_STATE_WIFI_LINKUP:
			SLIST_FOREACH(bh_entry, &(ctx->bh_link_head), next_bh_link)
			{
				if (channel_non_operable) {
					if ((bh_entry->bh_channel == radio->channel[0]) &&
						bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED)
					{
						struct bh_link_entry *bh_entry_local = NULL;
						int num_active_links = 0, mbh_state = 0;
						unsigned char non_zero_prior_entry = FALSE;
						always("Non Operable channel on connected BH\n");
						/* Disconnect only the BH on non-operable channel*/
						if (ctx->dual_bh_en) {
							wlanif_disconnect_apcli(ctx->back_ptr, bh_entry->ifname);
#if 0
							bh_entry->mbh_blocked = 1; //for 120 seconds
							eloop_register_timeout(120, 0, unblock_mbh, ctx, bh_entry);
#endif
							mbh_state = get_mbh_state(ctx, &bh_entry_local, &num_active_links);
							mapd_printf(MSG_OFF, "mbh_state=%d", mbh_state);
							/* Even after block I have 1 more active link */
							if (num_active_links >= 1) {
								mapd_printf(MSG_OFF, "Duplicate Link avail; Do nothing;");
								break;
							}
							/* No Duplicate Links Present */
							mapd_printf(MSG_OFF, "No duplicate links Mimic BAND_SWITCHING");
						} else {
								wlanif_disconnect_apcli(ctx->back_ptr,NULL);
						}

						ctx->current_bh_state = BH_STATE_WIFI_BAND_SWITCHED;

						bh_entry->priority_info.priority_bkp =
							bh_entry->priority_info.priority;
						bh_entry->priority_info.priority = 0;
						SLIST_FOREACH(bh_entry_local, &(ctx->bh_link_head), next_bh_link)
						{
							if (bh_entry_local->priority_info.priority) {
									non_zero_prior_entry = TRUE;
									break;
							}
						}

						if (non_zero_prior_entry == FALSE) {
							bh_entry->priority_info.priority =
								bh_entry->priority_info.priority_bkp;
							bh_entry->priority_info.priority_bkp = 0;

						}
						break;
					}
				}
			}
			if (bh_entry == NULL)
			{
				always("Non Operable channel on inactive BH\n");
#ifdef MAP_R2
				_1905_update_channel_pref_report(ctx, NULL, NULL);
#else
				_1905_update_channel_pref_report(ctx, NULL);
#endif
			}
			break;
	}
}

void retrigger_ch_planning_post_radar(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct own_1905_device *dev = (struct own_1905_device *)&global->dev;
	err("restart ");
	mapd_reset_first_seen_for_all_dev(dev, 0);
	mapd_restart_channel_plannig(dev->back_ptr);
	dev->Restart_ch_planning_radar = 1;
}

void retrigger_ch_planning(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct own_1905_device *dev = (struct own_1905_device *)&global->dev;
	mapd_restart_channel_plannig(dev->back_ptr);
}

/**
* @brief Fn to update channel preference for an 1905 device
*
* @param ctx own 1905 device ctx
* @param dev 1905 device pointer
* @param prefer channel preference
*
* @return 0 if success else -1
*/
int topo_srv_update_channel_preference(struct own_1905_device *ctx,
				       struct _1905_map_device *dev, struct ch_prefer *prefer)
{
	int db_cnt;
	struct radio_ch_prefer *ch_prefer;
	struct prefer_info_db *prefer_info_t, *prefer_info_tmp;
	struct prefer_info *cmd_info;
	struct radio_info_db *radio;
	int i;

	if (!dev)
		dev = topo_srv_get_1905_device(ctx, NULL);

	if (!dev) {
		err("critical error");
		return -1;
	}

	radio = topo_srv_get_radio(dev, prefer->identifier);
	err("radio mac " MACSTR, MAC2STR(prefer->identifier));

	if (!radio) {
		err("critical error");
		return -1;
	}
	debug("radio operating_class=%d, channel=%d, max_tx_pwr=%d",
	      radio->operating_class, radio->channel[0], radio->radio_capability.max_tx_pwr);

	ch_prefer = &radio->chan_preferance;
	if (!SLIST_EMPTY(&(ch_prefer->prefer_info_head))) {
		prefer_info_t = SLIST_FIRST(&(ch_prefer->prefer_info_head));
		while (prefer_info_t) {
			int i;
			for (i = 0; i < prefer_info_t->ch_num; i++) {
				ch_planning_remove_ch_from_prefered_list(
					ctx,
					prefer_info_t->ch_list[i],
					radio,
					prefer_info_t->ch_list[i] > 14);
			}
			prefer_info_tmp = SLIST_NEXT(prefer_info_t, prefer_info_entry);
			SLIST_REMOVE(&(ch_prefer->prefer_info_head), prefer_info_t, prefer_info_db, prefer_info_entry);
			free(prefer_info_t);
			prefer_info_t = prefer_info_tmp;
		}
	}
	SLIST_INIT(&ch_prefer->prefer_info_head);
	ch_prefer->is_valid = 1;
	ch_prefer->op_class_num = prefer->op_class_num;
	for (db_cnt = 0; db_cnt < ch_prefer->op_class_num; db_cnt++) {
		cmd_info = &prefer->opinfo[db_cnt];
		prefer_info_tmp = os_zalloc(sizeof(*prefer_info_tmp));
		prefer_info_tmp->op_class = cmd_info->op_class;
		prefer_info_tmp->ch_num = cmd_info->ch_num;
		memcpy(prefer_info_tmp->ch_list, cmd_info->ch_list, MAX_CH_NUM);
		prefer_info_tmp->perference = cmd_info->perference;
		prefer_info_tmp->reason = cmd_info->reason;
		SLIST_INSERT_HEAD(&ch_prefer->prefer_info_head, prefer_info_tmp, prefer_info_entry);
		for (i = 0; i < prefer_info_tmp->ch_num; i++)
		{
			ch_planning_add_ch_to_prefered_list(ctx,
				prefer_info_tmp->ch_list[i],
				radio,
				prefer_info_tmp->ch_list[i] > 14,
				prefer_info_tmp->perference,
				prefer_info_tmp->reason);
		}
	}

	dev->ch_preference_available = TRUE;
	while (radio) {
		send_user_ch_operable_results(ctx);
		if (!ch_planning_check_channel_operable_wrapper(ctx, radio->channel[0])) {
			if (ctx->device_role == DEVICE_ROLE_CONTROLLER) {
				err("need to retrigger ch planning post radar");
				eloop_register_timeout(10, 0,retrigger_ch_planning_post_radar,
						ctx->back_ptr, NULL);


				break;
			}
		}
		if(ctx->device_role == DEVICE_ROLE_AGENT) {
			mapd_handle_non_operable_channel(ctx, radio);
		}
		radio = topo_srv_get_next_radio(dev, radio);
	}

	//ch_planning_show_ch_distribution(ctx);
	return 0;
}

/**
* @brief Fn to update ht caps for an 1905 device
*
* @param ctx own 1905 device ctx
* @param dev 1905 device pointer
* @param pcap ht capability
*
* @return 0 if success else -1
*/
int topo_srv_update_ap_ht_cap(struct own_1905_device *ctx, struct _1905_map_device *dev, struct ap_ht_capability *pcap)
{
	struct radio_info_db *radio;
	struct ht_caps *pdb;

	if (!dev)
		dev = topo_srv_get_1905_device(ctx, NULL);

	radio = topo_srv_get_radio(dev, pcap->identifier);
	if (!radio) {
		radio = os_zalloc(sizeof(*radio));
		if (!radio) {
			err("mem allocation failed");
			return -1;
		}
		memcpy(radio->identifier, pcap->identifier, ETH_ALEN);
		radio->parent_1905 = dev;
		debug("create new radio %p, parent device = %p",
			radio, radio->parent_1905);
		SLIST_INSERT_HEAD(&(dev->first_radio), radio, next_radio);
		debug("new radio interface");
		SLIST_INIT(&(radio->link_estimate_cb_head));
		SLIST_INIT(&radio->chan_preferance.prefer_info_head);
		SLIST_INIT(&radio->chan_restrict.restrict_head);
#ifdef MAP_R2
		SLIST_INIT(&radio->first_scan_result);
		SLIST_INIT(&radio->cac_cap.cac_capab_head);
		SLIST_INIT(&radio->cac_comp_status.cac_completion_opcap_head);
#endif
	}
	pdb = &radio->radio_capability.ht_cap;
	pdb->tx_stream = pcap->tx_stream;
	pdb->rx_stream = pcap->rx_stream;
	pdb->sgi_20 = pcap->sgi_20;
	pdb->sgi_40 = pcap->sgi_40;
	pdb->ht_40 = pcap->ht_40;
	pdb->valid = 1;

	return 0;
}

/**
* @brief Fn to update vht caps for an 1905 device
*
* @param ctx own 1905 device ctx
* @param dev 1905 device pointer
* @param pcap vht caps
*
* @return 0 if success else error
*/
int topo_srv_update_ap_vht_cap(struct own_1905_device *ctx,
			       struct _1905_map_device *dev, struct ap_vht_capability *pcap)
{
	struct radio_info_db *radio;
	struct vht_caps *pdb;

	if (!dev)
		dev = topo_srv_get_1905_device(ctx, NULL);

	radio = topo_srv_get_radio(dev, pcap->identifier);
	if (!radio) {
		radio = os_zalloc(sizeof(*radio));
		if (!radio) {
			err("mem allocation failed");
			return -1;
		}
		memcpy(radio->identifier, pcap->identifier, ETH_ALEN);
		radio->parent_1905 = dev;
		info("create new radio %p, parent device = %p",
			radio, radio->parent_1905);
		SLIST_INSERT_HEAD(&(dev->first_radio), radio, next_radio);
		debug("new radio interface");
		SLIST_INIT(&(radio->link_estimate_cb_head));
		SLIST_INIT(&radio->chan_preferance.prefer_info_head);
		SLIST_INIT(&radio->chan_restrict.restrict_head);
#ifdef MAP_R2
		SLIST_INIT(&radio->first_scan_result);
		SLIST_INIT(&radio->cac_cap.cac_capab_head);
		SLIST_INIT(&radio->cac_comp_status.cac_completion_opcap_head);
#endif
	}
	debug("radio mac " MACSTR, MAC2STR(pcap->identifier));
	debug("radio operating_class=%d, channel=%d, max_tx_pwr=%d",
	      radio->operating_class, radio->channel[0], radio->radio_capability.max_tx_pwr);

	pdb = &radio->radio_capability.vht_cap;

	pdb->vht_tx_mcs = pcap->vht_tx_mcs;
	pdb->vht_rx_mcs = pcap->vht_rx_mcs;
	pdb->mu_beamformer = pcap->mu_beamformer;
	pdb->sgi_160 = pcap->sgi_160;
	pdb->sgi_80 = pcap->sgi_80;
	pdb->vht_160 = pcap->vht_160;
	pdb->vht_8080 = pcap->vht_8080;
	pdb->tx_stream = pcap->tx_stream;
	pdb->rx_stream = pcap->rx_stream;
	pdb->su_beamformer = pcap->su_beamformer;
	pdb->valid = 1;

	return 0;
}

int topo_srv_update_ap_he_cap(struct own_1905_device *ctx,
			       struct _1905_map_device *dev, struct ap_he_capability *pcap)
{
	struct radio_info_db *radio = NULL;
	struct he_caps *pdb = NULL;
	if (!dev)
		dev = topo_srv_get_1905_device(ctx, NULL);
	radio = topo_srv_get_radio(dev, pcap->identifier);
	if (!radio) {
		radio = os_zalloc(sizeof(*radio));
		if (!radio) {
			err("mem allocation failed");
			return -1;
		}
		memcpy(radio->identifier, pcap->identifier, ETH_ALEN);
		radio->parent_1905 = dev;
		info("create new radio %p, parent device = %p",
			radio, radio->parent_1905);
		SLIST_INSERT_HEAD(&(dev->first_radio), radio, next_radio);
		debug("new radio interface");
		SLIST_INIT(&(radio->link_estimate_cb_head));
		SLIST_INIT(&radio->chan_preferance.prefer_info_head);
		SLIST_INIT(&radio->chan_restrict.restrict_head);
#ifdef MAP_R2
		SLIST_INIT(&radio->first_scan_result);
		SLIST_INIT(&radio->cac_cap.cac_capab_head);
		SLIST_INIT(&radio->cac_comp_status.cac_completion_opcap_head);
#endif
	}
	debug("radio mac " MACSTR, MAC2STR(pcap->identifier));
	debug("radio operating_class=%d, channel=%d, max_tx_pwr=%d",
	      radio->operating_class, radio->channel[0], radio->radio_capability.max_tx_pwr);
	pdb = &radio->radio_capability.he_cap;
	pdb->tx_spatial_stream= pcap->tx_stream;
	pdb->rx_spatial_streams= pcap->rx_stream;
	pdb->he_160 =  pcap->he_160;
	pdb->he_80plus80 = pcap->he_8080;
	pdb->su_beamformer = pcap->su_bf_cap;
	pdb->mu_beamformer = pcap->mu_bf_cap;
	pdb->dl_ofdma_supported = pcap->dl_ofdma_cap;
	pdb->dl_ofdma_plus_mu_mimo = pcap->dl_mu_mimo_ofdma_cap;
	pdb->ul_mi_mimo = pcap->ul_mu_mimo_cap;
	pdb->ul_ofdma_plus_mu_mimo = pcap->ul_mu_mimo_ofdma_cap;
	pdb->ul_ofdma_supported = pcap->ul_ofdma_cap;
	pdb->valid = 1;
	return 0;
}
#ifdef MAP_R2
int topo_srv_update_dev_ch_scan_cap(struct own_1905_device *ctx,
	struct _1905_map_device *dev, struct channel_scan_capab *ch_scan_cap)
{
	struct radio_info_db *radio = NULL;
	struct radio_scan_capab *radio_scan_capab_db = NULL;
	struct radio_scan_capab_db *radio_param = NULL;
	struct channel_body *ch_body = NULL;
	u8 i,j,k;
	for (i =0; i < ch_scan_cap->radio_num; i++) {
		radio_scan_capab_db = &ch_scan_cap->radio_scan_params[i];
		radio = topo_srv_get_radio(dev, ch_scan_cap->radio_scan_params[i].radio_id);
		if(!radio) {
			err("radio not found in dev database , error");
			return -1;
		}
		radio_param = &radio->radio_scan_params;
		os_memcpy(radio_param->radio_id, radio_scan_capab_db->radio_id, ETH_ALEN);
		radio_param->boot_scan_only = radio_scan_capab_db->boot_scan_only;
		radio_param->scan_impact = radio_scan_capab_db->scan_impact;
		radio_param->min_scan_interval = radio_scan_capab_db->min_scan_interval;
		radio_param->oper_class_num = radio_scan_capab_db->oper_class_num;
		for (j = 0; j < radio_scan_capab_db->oper_class_num; j++) {
			ch_body = &radio_scan_capab_db->ch_body[j];
			radio_param->ch_body[j].oper_class = ch_body->oper_class;
			radio_param->ch_body[j].ch_list_num = ch_body->ch_list_num;
			for (k = 0; k < ch_body->ch_list_num; k++) {
				radio_param->ch_body[j].ch_list[k] = ch_body->ch_list[k];
			}
		}
	}
	return 0;
}
void topo_srv_clear_cac_cap_db(
	struct radio_info_db *radio)
{
	struct cac_cap_db *entry;
	struct cac_opcap_db *entry_opcap;
	while (!SLIST_EMPTY(&(radio->cac_cap.cac_capab_head))) {
		entry = SLIST_FIRST(&(radio->cac_cap.cac_capab_head));
		while (!SLIST_EMPTY(&(entry->cac_opcap_head))) {
			entry_opcap = SLIST_FIRST(&(entry->cac_opcap_head));
			SLIST_REMOVE_HEAD(&(entry->cac_opcap_head),
						cac_opcap_entry);
			os_free(entry_opcap);
		}
		SLIST_REMOVE_HEAD(&(radio->cac_cap.cac_capab_head),
			cac_cap_entry);
		os_free(entry);
	}
}
#endif
void topo_srv_clear_cac_completion_status(
	struct radio_info_db *radio)
{
	struct cac_completion_opcap_db *entry;
	while (!SLIST_EMPTY(&(radio->cac_comp_status.cac_completion_opcap_head))) {
		entry = SLIST_FIRST(&(radio->cac_comp_status.cac_completion_opcap_head));
		SLIST_REMOVE_HEAD(&(radio->cac_comp_status.cac_completion_opcap_head),
			opcap_db_next);
		os_free(entry);
	}
}

/**
* @brief return upstream 1905 device of any 1905 device, null in case of root
*
* @param _1905_device pointer to 1905 device
*
* @return _1905_map_device if found else NULL
*/
struct _1905_map_device *topo_srv_get_upstream_1905_device(struct _1905_map_device *_1905_device)
{
	if (!_1905_device)
		return NULL;
	else
		return _1905_device->upstream_device;
}

/**
* @brief return 1905 device, from an interface address
*
* @param ctx own 1905 device ctx
* @param iface_addr mac address of interface
*
* @return _1905_map_device if found else NULL
*/
struct _1905_map_device *topo_srv_get_1905_device_from_iface(struct
							     own_1905_device
							     *ctx, u8 * iface_addr)
{
	struct list_topology *_1905_list;
	struct _1905_map_device *map_dev;
	struct _1905_device *_1905_dev;
	struct list_interface *iface_list;
	struct iface_info *iface;

	if (!iface_addr)
		return NULL;

	_1905_list = &ctx->_1905_dev_head;

	if (SLIST_EMPTY(_1905_list)) {
		err("critical err, 1905 list is empty");
		return NULL;
	}

	SLIST_FOREACH(map_dev, _1905_list, next_1905_device) {
		_1905_dev = &map_dev->_1905_info;
		iface_list = &_1905_dev->first_iface;
		SLIST_FOREACH(iface, iface_list, next_iface) {
			if (os_memcmp(iface->iface_addr, iface_addr, ETH_ALEN) == 0)
				return map_dev;
		}
	}

	return NULL;
}

/**
* @brief mark a 1905 device as controller in topology
*
* @param ctx own 1905 device ctx
* @param al_mac almac of 1905 map device
*
* @return 0 if success else -1
*/
int topo_srv_update_controller_info(struct own_1905_device *ctx, u8 * al_mac)
{
	struct _1905_map_device *map_dev = topo_srv_get_1905_device(ctx, al_mac);

	if (!map_dev) {
		err("given node not found in topology " MACSTR, MAC2STR(al_mac));
		return -1;
	}
	map_dev->device_role = DEVICE_ROLE_CONTROLLER;

	return 0;
}

/**
* @brief Fn to return neighbor based of mac of a 1905 device
*
* @param ctx own 1905 device
* @param tmp_dev 1905 map device pointer
* @param almac almac of neighbor
*
* @return map_neighbor_info if found else NULL
*/
struct map_neighbor_info *topo_srv_get_neighbor(struct own_1905_device *ctx, struct _1905_map_device *tmp_dev,
		unsigned char *almac)
{
	struct map_neighbor_info *neighbor;

	if (!tmp_dev) {
		err("dev not found");
		return NULL;
	}

	if (SLIST_EMPTY(&tmp_dev->neighbors_entry)) {
		err("neighbor list is empty");
		return NULL;
	}

	SLIST_FOREACH(neighbor, &tmp_dev->neighbors_entry, next_neighbor) {
		if (os_memcmp(neighbor->n_almac, almac, ETH_ALEN) == 0)
			return neighbor;
	}

	return NULL;
}

int topo_srv_get_1905_dev_count(struct own_1905_device *ctx)
{
		int count = 0;
		struct _1905_map_device *_1905_device = NULL;

		while((_1905_device = topo_srv_get_next_1905_device(ctx, _1905_device)) != NULL)
			count++;

		return count;
}

void topo_srv_handle_local_leave(struct own_1905_device *ctx,
	unsigned char *mac_addr)
{
	struct _1905_map_device *dev = NULL;
	struct _1905_map_device *own_1905dev = NULL;

	dev = topo_srv_get_1905_by_iface_addr(ctx, mac_addr);
	if (dev != NULL)
	{
		struct map_neighbor_info *neighbor = NULL;
		info("1905 device leaving\n");
		own_1905dev = topo_srv_get_1905_device(ctx,NULL);
		SLIST_FOREACH(neighbor, &own_1905dev->neighbors_entry, next_neighbor)
		{
			if (neighbor->neighbor && neighbor->neighbor == dev) {
				info("remove %02x:%02x%02x:%02x:%02x:%02x",
					PRINT_MAC(neighbor->neighbor->_1905_info.al_mac_addr));
				info("From neighbour list of %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(own_1905dev->_1905_info.al_mac_addr));
				SLIST_REMOVE(&own_1905dev->neighbors_entry,
					neighbor, map_neighbor_info, next_neighbor);
				os_free(neighbor);
				if(SLIST_EMPTY(&own_1905dev->neighbors_entry))
					break;
				break;
			}
		}
		/* 1905 device disconnected reset the state in 1905 devices for Network Optmization*/
		ntwrk_opt_device_conn_disconnect_handle(ctx, DEVICE_DISCONNECT,  neighbor->neighbor);
		topo_srv_move_1905_off_nw(ctx, dev);
	}
}
/**
* @brief Fn mark_all_iface invalid*
* @param ctx own 1905 device ctx
* @param buf msg buffer
* @param dev 1905 map device ptr
*
* @return -1 if error else tlv length
*/
void topo_srv_mark_all_iface_invalid(struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{

	struct iface_info *iface = topo_srv_get_iface(dev, NULL);
	while(iface)
	{
		iface->valid = FALSE;
		iface = topo_srv_get_next_iface(dev, iface);
	}
}

/**
* @brief Fn mark_all_iface invalid*
* @param ctx own 1905 device ctx
* @param buf msg buffer
* @param dev 1905 map device ptr
*
* @return -1 if error else tlv length
*/
void topo_srv_mark_all_oper_bss_invalid(struct own_1905_device *ctx,
	struct _1905_map_device *dev, struct radio_info_db *radio)
{

	struct bss_info_db *bss = topo_srv_get_next_bss(dev, NULL);
	while(bss)
	{
		if (bss->radio == radio){
			bss->valid = FALSE;
		}
		bss = topo_srv_get_next_bss(dev, bss);
	}
}

void topo_srv_remove_all_invalid_iface(struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{

	struct iface_info *iface = topo_srv_get_iface(dev, NULL);
	struct iface_info *iface_tmp = NULL;

	while(iface)
	{
		iface_tmp = topo_srv_get_next_iface(dev, iface);
		if (iface->valid == FALSE) {
			SLIST_REMOVE(&dev->_1905_info.first_iface, iface, iface_info, next_iface);
			os_free(iface);
		}
		iface = iface_tmp;
	}
}


void topo_srv_remove_all_invalid_ap_iface(struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{

	struct iface_info *iface = topo_srv_get_iface(dev, NULL);
	struct iface_info *iface_tmp = NULL;

	while(iface)
	{
		iface_tmp = topo_srv_get_next_iface(dev, iface);
		if (*(((char *)(&iface->media_info)) + 6) == 0 &&
			(iface->media_type >= IEEE802_11_GROUP) &&
			(iface->media_type < IEEE1901_GROUP) &&
			topo_srv_get_bss_by_bssid(ctx, dev, iface->iface_addr) == NULL) {
			debug("Iface %02x:%02x:%02x:%02x:%02x:%02x present in iface list but not in BSS list",
				PRINT_MAC(iface->iface_addr));
			SLIST_REMOVE(&dev->_1905_info.first_iface, iface, iface_info, next_iface);
			os_free(iface);
		}
		iface = iface_tmp;
	}
}

void topo_srv_free_esp_record(struct bss_info_db *bss)
{
	struct esp_db *esp = NULL;
	struct esp_db *esp_tmp = NULL;

	SLIST_FOREACH(esp, &(bss->esp_head), esp_entry) {
		esp_tmp = SLIST_NEXT(esp, esp_entry);
		SLIST_REMOVE(&bss->esp_head, esp, esp_db, esp_entry);
		os_free(esp);
		esp = esp_tmp;
		if (esp == NULL)
		{
			SLIST_INIT(&bss->esp_head);
			break;
		}
	}
}
void topo_srv_remove_all_assoc_clients_for_bss(struct _1905_map_device *dev,
	struct bss_info_db *bss)
{
	struct associated_clients *assoc_client = NULL;
	struct associated_clients *assoc_client_tmp = NULL;

	SLIST_FOREACH(assoc_client, &(dev->assoc_clients), next_client) {
		if (assoc_client->bss == bss)
		{
			assoc_client_tmp = SLIST_NEXT(assoc_client, next_client);
			SLIST_REMOVE(&(dev->assoc_clients), assoc_client,
				associated_clients, next_client);
			os_free(assoc_client);
			assoc_client = assoc_client_tmp;
			if (assoc_client == NULL)
				break;
		}
	}
}
void topo_srv_update_rr_states_for_bss(struct own_1905_device *ctx,
	struct _1905_map_device *dev, struct bss_info_db *bss)
{
	if (ctx->device_role == DEVICE_ROLE_CONTROLLER)
	{

		struct rr_steer_controller *rr_control = &ctx->controller_context.rr_control;
		if (rr_control->p_current_1905_rr &&
			rr_control->p_current_1905_rr->p_current_bss_rr == bss)
		{
			chan_mon_handle_steer_complete(ctx, dev);
			rr_control->p_current_1905_rr->p_current_bss_rr = NULL;
		}
		if ((rr_control->p_current_1905_rr != dev) && (dev->p_current_bss_rr == bss))
 			dev->p_current_bss_rr = NULL;
	}
}
void topo_srv_remove_all_invalid_bss(struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{

	struct bss_info_db *bss = topo_srv_get_next_bss(dev, NULL);
	struct bss_info_db *bss_tmp = NULL;

	while(bss)
	{
		bss_tmp = topo_srv_get_next_bss(dev, bss);
		if (bss->valid == FALSE) {
			info("Remove %02x:%02x:%02x:%02x:%02x:%02x from the list",
				PRINT_MAC(bss->bssid));
			SLIST_REMOVE(&dev->first_bss, bss, bss_info_db, next_bss);
			topo_srv_free_esp_record(bss);
			topo_srv_remove_all_assoc_clients_for_bss(dev, bss);
			topo_srv_update_rr_states_for_bss(ctx, dev, bss);
			os_free(bss);
		}
		bss = bss_tmp;
	}
}

int topo_srv_update_own_device_config_status(struct own_1905_device *ctx)
{
	struct radio_info_db *radio = NULL;
	unsigned char all_radios_configured = TRUE;

	radio = topo_srv_get_radio(topo_srv_get_1905_device(ctx, NULL), NULL);
	while (radio) {
		if (radio->is_configured == FALSE)
			all_radios_configured = FALSE;
		radio = topo_srv_get_next_radio(topo_srv_get_1905_device(ctx, NULL), radio);
	}
	if (all_radios_configured)
		ctx->config_status = DEVICE_CONFIGURED;
	return 0;
}

#if 0
void handle_off_ch_scan_resp(struct mapd_global *pGlobal_dev,
	unsigned char *resp,
	struct _1905_map_device *_1905_device)
{
	struct off_ch_scan_req_tlv *ntwrk_opt_rsp = (struct off_ch_scan_req_tlv *)req;
	struct own_1905_device *dev = &pGlobal_dev->dev;

}
#endif
void trigger_net_opt_scan(void *global_ctx, void *timer_ctx)
{
	struct own_1905_device *ctx = (struct own_1905_device *)global_ctx;
	struct off_ch_scan_req_msg_s *scan_msg = ctx->net_opt_scan_msg;
	map_get_info_from_wapp(ctx,
		WAPP_USER_SET_NET_OPT_SCAN_REQ, 0, NULL,
		NULL,scan_msg, sizeof(struct off_ch_scan_req_msg_s));
	err(" ");
	if(ctx->net_opt_scan_msg)
		os_free(ctx->net_opt_scan_msg);
}
/*If the given channel maps with the bh channel band then that bh priority will be given
Last condition with bh channel greater than 100 is for tri-band*/
unsigned char get_bh_priority_from_channel(struct mapd_global *pGlobal, unsigned int ch) {
	struct bh_link_entry *bh_entry = NULL;
	unsigned char priority = 0;
	SLIST_FOREACH(bh_entry, &(pGlobal->dev.bh_link_head), next_bh_link) {
		if ((bh_entry->bh_channel <= 14) &&
			(ch <= 14)) {
			priority = bh_entry->priority_info.priority;
		}
		if ((bh_entry->bh_channel > 14) &&
			(ch > 14)) {
			priority = bh_entry->priority_info.priority;
		}
		if ((bh_entry->bh_channel >= 100) &&
			(ch >= 100)){
			priority = bh_entry->priority_info.priority;
			break;
		}
	}
	return priority;
}

void handle_off_ch_scan_req(struct mapd_global *pGlobal_dev,
	unsigned char *req,
	struct _1905_map_device *_1905_device_ptr, unsigned char type)
{
	struct own_1905_device *ctx = &pGlobal_dev->dev;
	struct _1905_map_device *own_dev = topo_srv_get_1905_device(&pGlobal_dev->dev, NULL);
	struct radio_info_db *radio = NULL;
	struct off_ch_scan_req_tlv *off_ch_scan_req =
		(struct off_ch_scan_req_tlv *)req;
	struct off_ch_scan_req_msg_s *scan_msg = &off_ch_scan_req->scan_msg;
	int temp =0, idx = 0, i = 0;

	ctx->off_ch_scan_cb.off_ch_token = off_ch_scan_req->transaction_id;
	ctx->off_ch_scan_cb._1905_dev_ptr = _1905_device_ptr;
	if (ctx->scan_triggered)
	{
		if (type == FUNC_VENDOR_OFF_CH_SCAN_REQ)
			send_off_ch_scan_resp(pGlobal_dev,
			NULL);
		else
			send_net_opt_scan_resp(pGlobal_dev,
			NULL);
		return;
	}
	err("map_get_info_from_wapp , WAPP_USER_SET_OFF_CH_SCAN_REQ\n");
	if (type == FUNC_VENDOR_OFF_CH_SCAN_REQ) {
		err("normal off ch scan ");
	map_get_info_from_wapp(ctx,
		WAPP_USER_SET_OFF_CH_SCAN_REQ, 0, NULL,
		NULL, scan_msg, sizeof(struct off_ch_scan_req_msg_s));
	} else {
		err("Net opt off ch scan ");
		temp = (os_random()) % 5;
		struct off_ch_scan_req_msg_s *temp_scan_msg = NULL;
		ctx->net_opt_scan_msg = os_zalloc(sizeof(struct off_ch_scan_req_msg_s));
		temp_scan_msg = ctx->net_opt_scan_msg;
		os_memcpy(temp_scan_msg,&off_ch_scan_req->scan_msg,sizeof(struct off_ch_scan_req_msg_s));
		os_memset(ctx->net_opt_scan_msg->ch_list, 0, MAX_OFF_CH_SCAN_CH);
		for (idx = 0; idx < MAX_OFF_CH_SCAN_CH; idx ++) {
			if (off_ch_scan_req->scan_msg.ch_list[idx]) {
				radio = topo_srv_get_radio_by_band(own_dev, off_ch_scan_req->scan_msg.ch_list[idx]);
				info("cac_channel: %u", radio->cac_channel);
				if ((get_bh_priority_from_channel(ctx->back_ptr, off_ch_scan_req->scan_msg.ch_list[idx]) != 0)
					&& (radio->cac_channel == 0)){
					ctx->net_opt_scan_msg->ch_list[i] = off_ch_scan_req->scan_msg.ch_list[idx];
					i++;
				} else
					err("No need to scan %d this as priority 0", off_ch_scan_req->scan_msg.ch_list[idx]);
			}
		}
		eloop_cancel_timeout(trigger_net_opt_scan, ctx, NULL);
		eloop_register_timeout(temp,0,
								trigger_net_opt_scan,
								ctx, NULL);
#if 0
		map_get_info_from_wapp(ctx,
				WAPP_USER_SET_NET_OPT_SCAN_REQ, 0, NULL,
				NULL, scan_msg, sizeof(struct off_ch_scan_req_msg_s));
#endif
	}
}
unsigned int calculate_size (struct net_opt_scan_report_event  *scan_resp)
{
	unsigned int scan_resp_len = 0;
	u8 * buf = NULL;
	int i;
	buf = (u8 *)scan_resp->scan_result;
	scan_resp_len = sizeof(struct net_opt_scan_report_event);
	for(i = 0; i < scan_resp->scan_result_num; i++)
	{
		struct net_opt_scan_result_event *temp_scan_result = (struct net_opt_scan_result_event *)buf;
		info("scan result %d neigh_num %d\n",i, temp_scan_result->neighbor_num);
		scan_resp_len =scan_resp_len + sizeof(struct net_opt_scan_result_event)+ (temp_scan_result->neighbor_num*sizeof(struct neighbor_info));
		buf += sizeof(struct net_opt_scan_result_event)+ (temp_scan_result->neighbor_num*sizeof(struct neighbor_info));
	}
	info("total_size %d\n",scan_resp_len );
	return scan_resp_len;
}
unsigned short fill_response_buffer(struct mapd_global *pGlobal_dev,struct net_opt_scan_report_event * src,struct net_opt_scan_report_event *dst)
{
	int i,j;
	u8 * buf = NULL;
	u8 * buf_rsp = NULL;
	unsigned short new_len = 0;
	buf = (u8 *)src->scan_result;
	buf_rsp = (u8 *)dst->scan_result;
	dst->scan_result_num = 0;
	new_len = sizeof(struct net_opt_scan_resp_tlv);//sizeof(struct off_ch_scan_report_event);
	info("src->scan_result_num %d\n", src->scan_result_num);
	for(i=0; i< src->scan_result_num;i++) {
		struct net_opt_scan_result_event *scan_result = (struct net_opt_scan_result_event *)buf;
		struct net_opt_scan_result_event *scan_resp = (struct net_opt_scan_result_event *)buf_rsp;
		if(scan_result->neighbor_num > 0) {
			scan_resp->neighbor_num = 0;
			for (j=0;j<scan_result->neighbor_num;j++) {
				struct neighbor_info *nb_info = &scan_result->nb_info[j];
				struct neighbor_info *nb_info_resp = &scan_resp->nb_info[scan_resp->neighbor_num];
				if(topo_srv_get_1905_by_bssid(&pGlobal_dev->dev,(unsigned char *)scan_result->nb_info[j].bssid))
				{
					os_memcpy(&nb_info_resp->bssid,&nb_info->bssid,ETH_ALEN);
					nb_info_resp->ssid_len=nb_info->ssid_len;
					os_memset(nb_info_resp->ssid, 0, MAX_LEN_OF_SSID);
					os_memcpy(&nb_info_resp->ssid,&nb_info->ssid,nb_info_resp->ssid_len);
					nb_info_resp->ssid_len = nb_info->ssid_len;
					nb_info_resp->RCPI = nb_info->RCPI;
					nb_info_resp->ch_bw_len = nb_info->ch_bw_len;
					os_memcpy(&nb_info_resp->ch_bw,&nb_info->ch_bw,MAX_CH_BW_LEN);
					nb_info_resp->cu_stacnt_present = nb_info->cu_stacnt_present;
					nb_info_resp->cu = nb_info->cu;
					nb_info_resp->sta_cnt = nb_info->sta_cnt;
					scan_resp->neighbor_num =scan_resp->neighbor_num + 1;
				}
			}
			if(scan_resp->neighbor_num > 0)
			{
				os_memcpy(&scan_resp->radio_id,&scan_result->radio_id,ETH_ALEN);
				scan_resp->channel=scan_result->channel;
				scan_resp->utilization = scan_result->utilization;
				scan_resp->noise =scan_result->noise;
				dst->scan_result_num = dst->scan_result_num + 1;
				buf_rsp += sizeof(struct net_opt_scan_result_event)+ (scan_resp->neighbor_num*sizeof(struct neighbor_info));
				new_len = new_len + sizeof(struct net_opt_scan_result_event)+ (scan_resp->neighbor_num*sizeof(struct neighbor_info));
			}
		}
		buf += sizeof(struct net_opt_scan_result_event)+ (scan_result->neighbor_num*sizeof(struct neighbor_info));
	}
return new_len;
}
void handle_net_opt_scan_resp(struct mapd_global *pGlobal_dev,
	struct net_opt_scan_resp_tlv *resp,
	struct _1905_map_device *p1905_device)
{
	unsigned int scan_resp_len;
	unsigned char *scan_resp = NULL;
	err("Scan report received from: %02x:%02x:%02x:%02x:%02x:%02x",
		PRINT_MAC(p1905_device->_1905_info.al_mac_addr));
	if (resp->transaction_id == p1905_device->off_ch_scan_cb.off_ch_token) {
		scan_resp_len = calculate_size(&resp->off_ch_scan_rep);
		scan_resp = os_zalloc(scan_resp_len);
		fill_response_buffer(pGlobal_dev,&resp->off_ch_scan_rep,(struct net_opt_scan_report_event *)scan_resp);
		p1905_device->net_opt_scan_report =
				(struct net_opt_scan_report_event *)scan_resp;
		//dump_net_opt_off_ch_scan_rep(p1905_device->net_opt_scan_report);
		if(p1905_device->network_opt_per1905.network_opt_device_state == NETOPT_STATE_DATA_COLLECTION_ONGOING)
		p1905_device->network_opt_per1905.network_opt_device_state = NETOPT_STATE_DATA_COLLECTION_COMPLETE;
		else
		{
			err("net opt scan resp rx in wrong state\n");
			network_opt_reset(pGlobal_dev);
			pGlobal_dev->dev.network_optimization.NetOptReason = REASON_TRY_AGAIN;
#ifdef MAP_R2
			handle_task_completion(&pGlobal_dev->dev);
#endif
		}
	} else {
		err("Scan response for incorrect transaction ID");
	}
}

void handle_off_ch_scan_resp(struct mapd_global *pGlobal_dev,
	struct off_ch_scan_resp_tlv *resp,
	struct _1905_map_device *p1905_device)
{
	unsigned int scan_resp_len;
	unsigned char *scan_resp = NULL;
	err("Scan report received from: %02x:%02x:%02x:%02x:%02x:%02x",
		PRINT_MAC(p1905_device->_1905_info.al_mac_addr));
	if (resp->transaction_id == p1905_device->off_ch_scan_cb.off_ch_token) {
		scan_resp_len = sizeof(struct off_ch_scan_report_event) +
			(sizeof(struct off_ch_scan_result_event) * resp->off_ch_scan_rep.scan_result_num);
		scan_resp = os_zalloc(scan_resp_len);
		if (scan_resp) {
			os_memcpy(scan_resp, &resp->off_ch_scan_rep, scan_resp_len);
			p1905_device->off_ch_scan_report =
				(struct off_ch_scan_report_event *)scan_resp;
		}
	} else {
		err("Scan response for incorrect transaction ID");
	}
}
void modify_scan_list_basedon_dev_pref(
	struct own_1905_device *own_dev,
	struct _1905_map_device *peer_1905,
	u8 *temp_list,
	u8  bw)
{
	struct radio_info_db *radio = NULL;
	struct radio_ch_prefer *ch_prefer = NULL;
	struct prefer_info_db *prefer_db = NULL;
	u8 bandwidth = BW_20, i = 0, c = 0, found = 0;
	for (c = 0; c < MAX_OFF_CH_SCAN_CH; c++) {
		if(*(temp_list+c) == 0)
			continue;
		found = 0;
		SLIST_FOREACH(radio, &(peer_1905->first_radio), next_radio) {
			if (radio->bh_priority != 1) {
				err("For this radio %02x:%02x:%02x:%02x:%02x:%02x, BH priority is:%u",
						MAC2STR(radio->identifier), radio->bh_priority);
				continue;
			}
			ch_prefer = &radio->chan_preferance;
			SLIST_FOREACH(prefer_db, &ch_prefer->prefer_info_head, prefer_info_entry) {
				bandwidth = chan_mon_get_bw_from_op_class(prefer_db->op_class);
				if (bw != bandwidth)
					continue;
				debug("opclass %d,bw %d pref %d", prefer_db->op_class,bw, prefer_db->perference);
				for(i=0; i<prefer_db->ch_num;i++) {
					if(*(temp_list+c) == prefer_db->ch_list[i]) {
						found = 1;
					}
				}
			}
		}
		if(found == 0) {
			debug("ch %d not found",*(temp_list+c));
			*(temp_list+c) = 0;
		}
	}
	for ( i= 0;i<MAX_OFF_CH_SCAN_CH;i++) {
		if(*(temp_list+i)!=0)
			err("final list %d", *(temp_list+i));
	}

}

/*Send Network Optimization Request to 1905 device*/
void send_off_ch_scan_req(struct mapd_global *pGlobal_dev,
	struct _1905_map_device *p1905_device, unsigned char mode,
	unsigned int band, unsigned char *list, unsigned char bw, unsigned char type)
{
	struct off_ch_scan_req_tlv req;
	struct _1905_map_device *own_1905_device = NULL;
	unsigned char scan_list[MAX_OFF_CH_SCAN_CH] = {0};
	u8 *temp_list = scan_list;

	own_1905_device = topo_srv_get_next_1905_device(&pGlobal_dev->dev, NULL); /*Get own device struct.*/
	os_memset(&req, 0, sizeof(struct off_ch_scan_req_tlv));
	req.tlv.tlv_type = TLV_802_11_VENDOR_SPECIFIC;
	req.tlv.tlv_len = host_to_be16(sizeof(struct off_ch_scan_req_tlv));
	memcpy(req.tlv.oui, MTK_OUI, OUI_LEN);
	if (type == 1)
	req.tlv.func_type = FUNC_VENDOR_OFF_CH_SCAN_REQ;
	else
		req.tlv.func_type = FUNC_VENDOR_NET_OPT_SCAN_REQ;
	req.transaction_id = ++p1905_device->off_ch_scan_cb.off_ch_token;
	err("Send Off channel scan Req to ALMAC " MACSTR, MAC2STR(p1905_device->_1905_info.al_mac_addr));
	info("Token ID = %d", req.transaction_id );
	req.scan_msg.mode = mode;
	req.scan_msg.band = band;
	req.scan_msg.bw = bw;
	if (list != NULL)
		os_memcpy(scan_list, list, MAX_OFF_CH_SCAN_CH);

	modify_scan_list_basedon_dev_pref(&pGlobal_dev->dev, p1905_device, temp_list, bw);
	if (temp_list != NULL)
		os_memcpy(req.scan_msg.ch_list, scan_list, MAX_OFF_CH_SCAN_CH);

	if (own_1905_device == p1905_device) {
		info("handle_off_ch_scan_req\n");
		handle_off_ch_scan_req(pGlobal_dev,
		(unsigned char *)&req,
		p1905_device, req.tlv.func_type);
	} else {
		info("map_1905_Send_Vendor_Specific_Message\n");
		map_1905_Send_Vendor_Specific_Message(pGlobal_dev->_1905_ctrl,
			(char *)p1905_device->_1905_info.al_mac_addr,
			(char *)&req, sizeof(struct off_ch_scan_req_tlv));

	}
	//eloop_register_timeout(45, 0, off_ch_scan_req_timeout, pGlobal_dev, p1905_device);
}


/*Send Network Optimization Request to 1905 device*/
void send_net_opt_scan_resp(struct mapd_global *pGlobal_dev,
	struct net_opt_scan_report_event *scan_rep_evt)
{
	struct net_opt_scan_resp_tlv *resp;
	struct _1905_map_device *own_1905_device = NULL;
	struct own_1905_device *ctx = &pGlobal_dev->dev;
	unsigned short  total_size = 0;
	int i = 0;

	own_1905_device = topo_srv_get_next_1905_device(ctx, NULL); /*Get own device struct.*/
	if (scan_rep_evt == NULL) {
		resp = os_zalloc(sizeof(struct net_opt_scan_resp_tlv));
		total_size = sizeof(struct net_opt_scan_resp_tlv);
		resp->off_ch_scan_rep.scan_result_num = 0;
	} else {
		u8 * buf = NULL;
		buf = (u8 *)scan_rep_evt->scan_result;
		total_size = sizeof(struct net_opt_scan_resp_tlv);
		for(i = 0; i < scan_rep_evt->scan_result_num; i++)
		{
		    struct net_opt_scan_result_event *temp_scan_result = (struct net_opt_scan_result_event *)buf;
			info("scan result %d neigh_num %d\n",i, temp_scan_result->neighbor_num);
			total_size +=sizeof(struct net_opt_scan_result_event)+ temp_scan_result->neighbor_num*sizeof(struct neighbor_info);
			buf += sizeof(struct net_opt_scan_result_event)+ temp_scan_result->neighbor_num*sizeof(struct neighbor_info);
		}
		info("total_size %d\n",total_size );
		resp = os_zalloc(total_size);
		err("Send Off channel scan Resp to ALMAC " MACSTR,
			MAC2STR(ctx->off_ch_scan_cb._1905_dev_ptr->_1905_info.al_mac_addr));
		info("Token ID = %d", resp->transaction_id );
		total_size =fill_response_buffer(pGlobal_dev,scan_rep_evt,&resp->off_ch_scan_rep);
	}
	resp->tlv.tlv_type = TLV_802_11_VENDOR_SPECIFIC;
	resp->tlv.tlv_len = host_to_be16(total_size);
	info("total_size %d , resp->tlv.tlv_len %d\n",total_size, resp->tlv.tlv_len);
	memcpy(resp->tlv.oui, MTK_OUI, OUI_LEN);
	resp->tlv.func_type = FUNC_VENDOR_NET_OPT_SCAN_RESP;
	resp->transaction_id = ctx->off_ch_scan_cb.off_ch_token;
	if (own_1905_device == ctx->off_ch_scan_cb._1905_dev_ptr) {
		err("Off channel scan resp to self\n");
		handle_net_opt_scan_resp(pGlobal_dev,
			resp, ctx->off_ch_scan_cb._1905_dev_ptr);
	} else {
		err("agent MAP send Off channel scan resp to peer\n");
		map_1905_Send_Vendor_Specific_Message(pGlobal_dev->_1905_ctrl,
			(char *)ctx->off_ch_scan_cb._1905_dev_ptr->_1905_info.al_mac_addr,
			(char *)resp, total_size);
	}
	os_free(resp);
}
void send_off_ch_scan_resp(struct mapd_global *pGlobal_dev,
	struct off_ch_scan_report_event *scan_rep_evt)
{
	struct off_ch_scan_resp_tlv *resp;
	struct _1905_map_device *own_1905_device = NULL;
	struct own_1905_device *ctx = &pGlobal_dev->dev;
	unsigned int tlv_len = 0;
	int i = 0;

	if (!(ctx->off_ch_scan_cb._1905_dev_ptr))
		return;

	own_1905_device = topo_srv_get_next_1905_device(ctx, NULL); /*Get own device struct.*/
	if (scan_rep_evt == NULL) {
		resp = os_zalloc(sizeof(struct off_ch_scan_resp_tlv));
	} else {
		resp = os_zalloc(sizeof(struct off_ch_scan_resp_tlv) +
			(sizeof(struct off_ch_scan_result_event) *
			scan_rep_evt->scan_result_num));
		err("Send Off channel scan Resp to ALMAC " MACSTR,
			MAC2STR(ctx->off_ch_scan_cb._1905_dev_ptr->_1905_info.al_mac_addr));
		info ("Token ID = %d", resp->transaction_id );

		for(i = 0; i < scan_rep_evt->scan_result_num; i++)
		{
			os_memcpy(&resp->off_ch_scan_rep.scan_result[i],
				&scan_rep_evt->scan_result[i],
				sizeof(struct off_ch_scan_result_event));
		}
	}
	resp->tlv.tlv_type = TLV_802_11_VENDOR_SPECIFIC;
	if (scan_rep_evt) {
		tlv_len = sizeof(struct off_ch_scan_resp_tlv) +
			(sizeof(struct off_ch_scan_result_event) * scan_rep_evt->scan_result_num);
		resp->tlv.tlv_len = host_to_be16(tlv_len);
		resp->off_ch_scan_rep.scan_result_num =
			scan_rep_evt->scan_result_num;
	} else {
		tlv_len = sizeof(struct off_ch_scan_resp_tlv);
		resp->tlv.tlv_len = host_to_be16(tlv_len);
		resp->off_ch_scan_rep.scan_result_num = 0;
	}
	memcpy(resp->tlv.oui, MTK_OUI, OUI_LEN);
	resp->tlv.func_type = FUNC_VENDOR_OFF_CH_SCAN_RESP;
	resp->transaction_id = ctx->off_ch_scan_cb.off_ch_token;

	if (own_1905_device == ctx->off_ch_scan_cb._1905_dev_ptr) {
		err("Off channel scan resp to self\n");
		handle_off_ch_scan_resp(pGlobal_dev,
			resp,
			ctx->off_ch_scan_cb._1905_dev_ptr);
	} else {
		err("Off channel scan resp to peer\n");
		map_1905_Send_Vendor_Specific_Message(pGlobal_dev->_1905_ctrl,
			(char *)ctx->off_ch_scan_cb._1905_dev_ptr->_1905_info.al_mac_addr,
			(char *)resp,
			tlv_len);
	}
	os_free(resp);
}

void update_topology_info(struct _1905_map_device *dev)
{
   struct map_neighbor_info *neighbor = NULL;

   SLIST_FOREACH(neighbor, &dev->neighbors_entry, next_neighbor) {
       if (neighbor->neighbor->in_network == FALSE) {
		   neighbor->neighbor->in_network = TRUE;
		   debug("mark dev " MACSTR " valid", PRINT_MAC(neighbor->neighbor->_1905_info.al_mac_addr));
           update_topology_info(neighbor->neighbor);
	   }
   }
}

void topo_serv_update_own_topology_info(struct own_1905_device *ctx)
{
   struct _1905_map_device *tmp_dev = NULL, *own_dev = NULL;

   /*mark all the node invalid*/
   SLIST_FOREACH(tmp_dev, &ctx->_1905_dev_head, next_1905_device) {
       tmp_dev->in_network = FALSE;
       debug("mark dev " MACSTR " invalid", PRINT_MAC(tmp_dev->_1905_info.al_mac_addr));
   }

   /*mark the node still in network valid*/
   own_dev = topo_srv_get_next_1905_device(ctx, NULL);
   own_dev->in_network = TRUE;
   debug("mark dev " MACSTR " valid", PRINT_MAC(own_dev->_1905_info.al_mac_addr));
   update_topology_info(own_dev);
}
uint8_t min(uint8_t a, uint8_t b);

void dump_uplink_rate(struct own_1905_device *ctx)
{
	struct radio_info_db *radio = NULL;
	struct _1905_map_device *_1905_device = NULL;

		_1905_device = topo_srv_get_next_1905_device(ctx, NULL); /*Get own device struct.*/

		while(_1905_device) {
			err("almac"MACSTR ,MAC2STR(_1905_device->_1905_info.al_mac_addr));

			radio = topo_srv_get_next_radio(_1905_device, NULL);
			while (radio) {
				err("Radio Channel %d Uplink Rate %d",radio->channel[0],radio->uplink_rate);
				radio = topo_srv_get_next_radio(_1905_device, radio);
			}
			_1905_device = topo_srv_get_next_1905_device(ctx,_1905_device);
		}

}
void latch_and_compare_lastUplinkRssi(struct _1905_map_device *_1905_dev)
{
	struct radio_info_db *radio = NULL;
	info("for dev " MACSTR " ", PRINT_MAC(_1905_dev->_1905_info.al_mac_addr));
	radio = topo_srv_get_radio(_1905_dev, NULL);
	while(radio) {
		if(radio->network_opt_1905dev_radio.last_uplink_rate == 0) {
			radio->network_opt_1905dev_radio.last_uplink_rate = radio->uplink_rate;
		}
		info(" last uplink rate %d",radio->network_opt_1905dev_radio.last_uplink_rate);
		if (radio->uplink_bh_present ){
			err("radio->uplink_rate %d, radio->network_opt_1905dev_radio.last_uplink_rate %d", radio->uplink_rate, radio->network_opt_1905dev_radio.last_uplink_rate);
			if (radio->uplink_rate != (radio->network_opt_1905dev_radio.last_uplink_rate)){
				err("different");
				radio->network_opt_1905dev_radio.rate_deviate_count++;
			} else {
				info("same reset rate_deviate_count");
				radio->network_opt_1905dev_radio.rate_deviate_count = 0;
			}
		}
		radio = topo_srv_get_next_radio(_1905_dev,radio);
	}
}
void sync_score_to_all_radios(struct own_1905_device *ctx, struct _1905_map_device *dev,
	struct map_neighbor_info *neighbor, unsigned char is_eth_bh,
	unsigned char wifi_bh_found, unsigned int score_24G,
	unsigned int score_5G)
{
	struct _1905_map_device *uplink_1905_dev = NULL;
	struct radio_info_db *radio = NULL, *uplink_radio = NULL;
	unsigned int prev_uplink_rate = 0;
	u8 controller_radio = 0;
	uplink_1905_dev = dev->upstream_device;
	if(uplink_1905_dev == NULL) {
		err("Something is wrong check");
		return;
	}

	if(is_eth_bh){
		err("ETH backhaul apply uplink score - 100 to all radios");
		SLIST_FOREACH(uplink_radio, &uplink_1905_dev->first_radio, next_radio) {
			controller_radio++;
			SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
				if (get_band_from_channel(radio->channel[0]) == get_band_from_channel(uplink_radio->channel[0])) {
					prev_uplink_rate = radio->uplink_rate;
					radio->uplink_rate = uplink_radio->uplink_rate - 200;
					if (prev_uplink_rate != radio->uplink_rate)
						ap_selection_update_vend_ie(ctx, NULL, TRUE);
					info("Radio Channel %d rate %d", radio->channel[0], radio->uplink_rate);
				}
			}
		}
		if (controller_radio == 0) {
			SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
				radio->uplink_rate = MAX_RATE;
				ap_selection_update_vend_ie(ctx, NULL, TRUE);
			}
		}
	} else {
		if(wifi_bh_found == 2){
			info("Multiple Backhaul ");
			SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
				if(radio->channel[0] < 14){
					if(score_24G) {
						radio->uplink_rate = score_24G;
					} else {
						radio->uplink_rate = score_5G;
					}
				} else {
						radio->uplink_rate = score_5G;
				}
				info("Radio Channel %d rate %d", radio->channel[0], radio->uplink_rate);
			}
		} else if (wifi_bh_found == 1) {
			info("Single Backhaul apply 2.4 %d 5 %d score to all radios",score_24G, score_5G);
			SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
				if(score_24G) {
					radio->uplink_rate = score_24G;
				} else {
					radio->uplink_rate = score_5G;
				}
				info("Radio Channel %d rate %d", radio->channel[0], radio->uplink_rate);
			}
		}
	}
	if ((ctx->device_role == DEVICE_ROLE_CONTROLLER) &&
		(ctx->network_optimization.network_opt_state == NETOPT_STATE_IDLE)) {
		latch_and_compare_lastUplinkRssi(dev);
	}
}
unsigned int get_uplink_ethernet_hop_count(struct own_1905_device *ctx, struct _1905_map_device *dev)
{
	unsigned int hop_count = 0;
	struct _1905_map_device *controller_1905_dev = NULL, *uplink_1905_dev = NULL, *current_dev = NULL;
	struct iface_info *iface = NULL;
	struct backhaul_link_info *link = NULL;
	struct map_neighbor_info *neighbor = NULL;

	controller_1905_dev = topo_srv_get_controller_device(ctx);
	if(controller_1905_dev == NULL) {
		info(" Controller Not Found Return");
		return hop_count;
	}
	current_dev = dev;
	uplink_1905_dev = dev->upstream_device;
	while(uplink_1905_dev) {
		info("Uplink Dev almac"MACSTR ,MAC2STR(uplink_1905_dev->_1905_info.al_mac_addr));
		info("Current Dev almac"MACSTR ,MAC2STR(current_dev->_1905_info.al_mac_addr));
		SLIST_FOREACH(neighbor, &(current_dev->neighbors_entry), next_neighbor) {
			info("Neighbour almac"MACSTR ,MAC2STR(neighbor->n_almac));
			if(os_memcmp(uplink_1905_dev->_1905_info.al_mac_addr,neighbor->n_almac,ETH_ALEN) == 0){
				SLIST_FOREACH(link, &neighbor->bh_head, next_bh) {
					info("Connected iface"MACSTR ,MAC2STR(link->connected_iface_addr));
					if (os_memcmp(link->connected_iface_addr, ZERO_MAC_ADDR, ETH_ALEN)) {
						iface = topo_srv_get_iface(current_dev , link->connected_iface_addr);
						if(!iface) {
							err("iface not found");
							continue;
						}
						if (iface->media_type == ieee_802_3_u || iface->media_type == ieee_802_3_ab) {
							info("Ethernet Found %d ",hop_count);
							hop_count++;
						} else {
							info("WIFI LINK");
						}
					} else {
						err("BH Entry NULL");
					}
				}
			}
		}
		if((os_memcmp(controller_1905_dev->_1905_info.al_mac_addr,uplink_1905_dev->_1905_info.al_mac_addr,ETH_ALEN) == 0) ||
			(os_memcmp(dev->_1905_info.al_mac_addr,current_dev->_1905_info.al_mac_addr,ETH_ALEN) == 0)){
			err("Reached Controller ETH Hop is %d",hop_count);
			return hop_count;
		} else {
			current_dev = uplink_1905_dev;
			uplink_1905_dev = uplink_1905_dev->upstream_device;
		}
	}
	return hop_count;
}
struct map_neighbor_info *topo_serv_find_cli_neighbor_by_mac(
   struct own_1905_device *ctx, struct _1905_map_device *dev,
   unsigned char *mac)
{
   struct map_neighbor_info *neighbor = NULL;
   struct backhaul_link_info *link = NULL;

   SLIST_FOREACH(neighbor, &dev->neighbors_entry, next_neighbor)
   {
       SLIST_FOREACH(link, &neighbor->bh_head, next_bh)
       {
           if (!os_memcmp(mac, link->connected_iface_addr, ETH_ALEN))
                   return neighbor;
       }
   }

   return NULL;
}
#if 0
int self_apcli_loop_exist(struct own_1905_device *ctx)
{
	struct map_neighbor_info *neighbor = NULL;
	struct _1905_map_device *own_dev = NULL;

	own_dev = topo_srv_get_next_1905_device(ctx, NULL);

	SLIST_FOREACH(neighbor, &(own_dev->neighbors_entry), next_neighbor) {
		struct backhaul_link_info *link = NULL;
		struct iface_info *iface = NULL;
		u8 apcli_found = 0;
		u8 ap_found = 0;

		SLIST_FOREACH(link, &neighbor->bh_head, next_bh) {
				iface = topo_srv_get_iface(own_dev, link->connected_iface_addr);
				if (!iface)
					continue;
				if (iface->media_type < IEEE802_11_GROUP)
					continue;
				if (iface->media_type >= IEEE802_11_GROUP) {
					if (iface->media_info.role == 0x04)
						apcli_found = 1;
					else
						ap_found = 1;
				}
				if (apcli_found == 1 && ap_found == 1)
					return 1;
		}
	}
	return 0;
}
#endif

struct iface_info *topo_srv_find_iface_by_mac(
   struct own_1905_device *ctx, unsigned char *addr)
{
   struct iface_info *iface = NULL;
   struct _1905_map_device *dev = NULL;

   if (!addr)
       return NULL;

   SLIST_FOREACH(dev, &ctx->_1905_dev_head, next_1905_device)
   {
       SLIST_FOREACH(iface, &(dev->_1905_info.first_iface), next_iface)
       {
           if (os_memcmp(iface->iface_addr, addr, ETH_ALEN) == 0)
               return iface;
       }
   }

   return NULL;
}

void topo_serv_remove_delete_link_peers(struct backhaul_link_info *bh,
   struct _1905_map_device *dev, struct _1905_map_device *ndev)
{
   struct map_neighbor_info *neighbor = NULL;
   struct backhaul_link_info *nbh = NULL;
   unsigned char zero_mac[ETH_ALEN] = {0};

   SLIST_FOREACH(neighbor, &ndev->neighbors_entry, next_neighbor)
   {
       if (neighbor->neighbor != dev)
           continue;
       SLIST_FOREACH(nbh, &neighbor->bh_head, next_bh)
       {
           if (os_memcmp(bh->connected_iface_addr, zero_mac, ETH_ALEN)) {
               if (!os_memcmp(bh->connected_iface_addr,
                   nbh->neighbor_iface_addr, ETH_ALEN))
                   break;
           } else if (os_memcmp(bh->neighbor_iface_addr, zero_mac, ETH_ALEN)) {
               if (!os_memcmp(bh->neighbor_iface_addr,
                   nbh->connected_iface_addr, ETH_ALEN))
                   break;
           }
       }

       if (nbh) {
           SLIST_REMOVE(&neighbor->bh_head, nbh, backhaul_link_info, next_bh);
           err("need remove link\n"
               "connected_iface_addr(%02x:%02x:%02x:%02x:%02x:%02x)\n"
               "neighbor_iface_addr(%02x:%02x:%02x:%02x:%02x:%02x)\n"
               "from dev(%02x:%02x:%02x:%02x:%02x:%02x)\n"
               "to dev(%02x:%02x:%02x:%02x:%02x:%02x)\n",
               PRINT_MAC(nbh->connected_iface_addr),
			   PRINT_MAC(nbh->neighbor_iface_addr),
               PRINT_MAC(ndev->_1905_info.al_mac_addr),
               PRINT_MAC(dev->_1905_info.al_mac_addr));
           os_free(nbh);
       } else {
           err("err condition!!! cannot find remove link!!!\n"
               "connected_iface_addr(%02x:%02x:%02x:%02x:%02x:%02x)\n"
               "neighbor_iface_addr(%02x:%02x:%02x:%02x:%02x:%02x)\n"
               "from dev(%02x:%02x:%02x:%02x:%02x:%02x)\n"
               "to dev(%02x:%02x:%02x:%02x:%02x:%02x)\n",
               PRINT_MAC(bh->connected_iface_addr),
               PRINT_MAC(bh->neighbor_iface_addr),
               PRINT_MAC(dev->_1905_info.al_mac_addr),
               PRINT_MAC(ndev->_1905_info.al_mac_addr));
       }
	   break;
   }
}


void topo_serv_remove_neighbor_peers(struct _1905_map_device *dev,
   struct _1905_map_device *ndev)
{
   struct map_neighbor_info *neighbor = NULL;

   SLIST_FOREACH(neighbor, &ndev->neighbors_entry, next_neighbor)
   {
       if (neighbor->neighbor == dev)
           break;
   }

   if (neighbor) {
       SLIST_REMOVE(&ndev->neighbors_entry, neighbor, map_neighbor_info, next_neighbor);
       err("need remove neighbor(%02x:%02x:%02x:%02x:%02x:%02x)\n"
           "from dev(%02x:%02x:%02x:%02x:%02x:%02x)\n",
           PRINT_MAC(neighbor->n_almac),
           PRINT_MAC(ndev->_1905_info.al_mac_addr));
       os_free(neighbor);
   } else {
       err("err condition!!!\n"
           "cannot find neighbor from dev(%02x:%02x:%02x:%02x:%02x:%02x)\n",
           PRINT_MAC(ndev->_1905_info.al_mac_addr));
   }

}

struct backhaul_link_info *topo_srv_get_bh_uplink_metrics_info(struct own_1905_device *ctx, struct bh_link_entry *bh_entry)
{
	struct map_neighbor_info *neighbor;
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	struct iface_info *ifc_info = NULL;

	SLIST_FOREACH(neighbor, &dev->neighbors_entry, next_neighbor) {
		ctx->metric_entry.bh = SLIST_FIRST(&neighbor->bh_head);
		ifc_info = topo_srv_get_iface(dev, ctx->metric_entry.bh->connected_iface_addr);
		if(!ifc_info)
			continue;

		if(ifc_info->media_type == ieee_802_3_ab || ifc_info->media_type == ieee_802_3_u) // if eth, contine;
			continue;

		if ((memcmp(bh_entry->mac_addr, ctx->metric_entry.bh->connected_iface_addr, ETH_ALEN)) == 0) {

			//err("find the current bh link info\n");
			return ctx->metric_entry.bh;
		}
	}
	return NULL;
}

void mapd_send_onboardstatus_to_app(struct mapd_global *global, int curr_onboard_status, unsigned char bh_type)
{
	struct mapd_user_onboarding_event *client_notif = NULL;
	struct mapd_user_event *user_event = NULL;
	struct ctrl_iface_global_priv *priv = global->ctrl_iface;

	user_event = (struct mapd_user_event *)os_zalloc(sizeof(struct mapd_user_event) +
		sizeof(struct mapd_user_onboarding_event));
	if (!user_event) {
		err("mem alloc failed");
		return;
	}
	os_memset(user_event, 0, sizeof(struct mapd_user_event) +
		sizeof(struct mapd_user_onboarding_event));

	user_event->event_id = ONBOARDING_STATUS_NOTIF;
	client_notif = (struct mapd_user_onboarding_event *)user_event->event_body;
	client_notif->bh_type = bh_type;
	client_notif->onboarding_start_stop = curr_onboard_status;

	if (!dl_list_empty(&priv->ctrl_dst)) {
		mapd_ctrl_iface_send(global,
					priv->sock,
					&priv->ctrl_dst,
					(const char *)user_event, sizeof(struct mapd_user_event) +
						sizeof(struct mapd_user_onboarding_event),
					priv);
	}
	os_free(user_event);
}

#ifdef MAP_R2
enum neighbor_mode {
	NB_NONE,
	NB_OWN_SSID,
	NB_ALL
};
int topo_srv_handle_ch_scan_req(struct own_1905_device *ctx, unsigned char *buf, int len)
{

	struct channel_scan_req *scan_req = NULL;
	u8 buff[1024] = {0};
	u16 length=0;
	u8 *p = buf;
	u8 i=0, j=0, k=0;
	scan_req = (struct channel_scan_req *)buff;

	if(*p != CHANNEL_SCAN_REQUEST_TYPE) {
		err("Raghav: wrong tlv: %d", (int)*p);
		return FALSE;
	}

	//mapd_hexdump(MSG_OFF, "MAPD SCAN_REQ incoming", buf, 1000);
	p++;
	p+=2;  // skip length
	scan_req->fresh_scan = *p++;
	scan_req->radio_num = *p++;

	scan_req->neighbour_only = NB_ALL;

	for(i=0; i < scan_req->radio_num; i++) {
		os_memcpy(scan_req->body[i].radio_id,p,ETH_ALEN);
		p+=ETH_ALEN;
		scan_req->body[i].oper_class_num = *p++;
		for (j=0; j< scan_req->body[i].oper_class_num;j++) {
			scan_req->body[i].ch_body[j].oper_class = *p++;
			scan_req->body[i].ch_body[j].ch_list_num = *p++;
			for(k=0;k < scan_req->body[i].ch_body[j].ch_list_num; k++) {
				scan_req->body[i].ch_body[j].ch_list[k] = *p++;
			}
		}
	}

	length = sizeof(struct channel_scan_req) + scan_req->radio_num*sizeof(struct scan_body);
	printf("Len = %d\n", length);
	mapd_hexdump(MSG_OFF, "MAPD SCAN_REQ", buff, length);
	map_get_info_from_wapp(ctx, WAPP_USER_SET_CHANNEL_SCAN_REQ, 0, NULL, NULL, (void *)buff, length);
	return 0;
}

void topo_srv_update_ch_plan_scan_done(
	struct own_1905_device *own_dev,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
	if(own_dev->ch_planning_R2.ch_plan_enable == FALSE)
		return;
	if(own_dev->ch_planning_R2.ch_plan_state != CHPLAN_STATE_SCAN_ONGOING) {
		debug("scan is not ongoing , so return");
		return;
	}
	ch_planning_update_state_scan_done(own_dev,dev,radio);
}

int topo_srv_handle_ch_scan_report(struct mapd_global *global, unsigned char *buf, int len)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(&global->dev, buf);
	struct _1905_map_device *next_dev = NULL;
	struct own_1905_device *ctx = &global->dev;
	u8 flag = 0;
	if (!dev) {
		err("mentioned 1905 device not found");
		return -1;
	}
	buf += ETH_ALEN;

#ifdef MAP_R2
	/*if a new report is received , first delete the exiting report for all radios*/
	if(ctx->ch_planning_R2.ch_plan_enable == TRUE) {
		ch_planning_remove_radio_scan_results(ctx, dev);
	}
#endif
	if (parse_channel_scan_report_message(global, buf, dev) < 0) {
		err("failed to parse resp msg");
		return -1;
	}
#ifdef MAP_R2
	/*check if best channel selection needs to be triggered.*/
	if(ctx->ch_planning_R2.ch_plan_enable == TRUE)
		ch_planning_handle_ch_scan_rep(global);
#endif
	dev->de_done = 0;
	eloop_cancel_timeout(ch_scan_req_timeout, (void *)global, (void *)dev);
	SLIST_FOREACH(next_dev, &ctx->_1905_dev_head, next_1905_device) {
		if (next_dev->de_done == 1)
			flag = 1;
	}
	if (flag == 0 && global->dev.de_state != OFF) {
		global->dev.de_state = OFF;
	}

#ifdef MAP_R2
	if(ctx->user_triggered_scan == 1)
	{
		ctx->user_triggered_scan = 0;
		handle_task_completion(&global->dev);
	}
#endif
	return 0;
}
int topo_srv_handle_assoc_status_notif_event(struct mapd_global *global, unsigned char *buf, int len) {
	struct _1905_map_device *dev = topo_srv_get_1905_device(&global->dev, buf);

	if (!dev) {
		err("mentioned 1905 device not found");
		return -1;
	}
	buf += ETH_ALEN;

	if (parse_assoc_status_notification_message(&global->dev, dev, buf) < 0) {
		err("failed to parse resp msg");
		return -1;
	}

	return 0;
}

int topo_srv_handle_disassoc_stats_event(struct mapd_global *global, unsigned char *buf, int len) {
	struct _1905_map_device *dev = topo_srv_get_1905_device(&global->dev, buf);

	if (!dev) {
		err("mentioned 1905 device not found");
		return -1;
	}
	buf += ETH_ALEN;

	if (parse_client_disassciation_stats_message(&global->dev, dev, buf) < 0) {
		err("failed to parse resp msg");
		return -1;
	}

	return 0;
}

int topo_srv_handle_bh_sta_report(struct mapd_global *global, unsigned char *buf, int len) {
	struct _1905_map_device *dev = topo_srv_get_1905_device(&global->dev, buf);
	if (!dev) {
		err("mentioned 1905 device not found");
		return -1;
	}
	buf += ETH_ALEN;

	if (parse_bh_sta_report(&global->dev, dev, buf) < 0) {
		err("failed to parse resp msg");
		return -1;
	}

	return 0;
}


int topo_srv_handle_tunneled_msg(struct mapd_global *global, unsigned char *buf, int len) {
	struct _1905_map_device *dev = topo_srv_get_1905_device(&global->dev, buf);

	if (!dev) {
		err("mentioned 1905 device not found");
		return -1;
	}
	buf += ETH_ALEN;
	if (global->dev.device_role != DEVICE_ROLE_CONTROLLER)
		return -1;
	if (parse_tunneled_message(&global->dev, buf, dev) < 0) {
		err("failed to parse resp msg");
		return -1;
	}

	return 0;
}
void wapp_set_unsuccessful_association_policy_setting(struct own_1905_device *ctx, struct unsuccessful_association_policy *assoc_failed_policy)
{
	int datalen = 0;
	struct unsuccessful_association_policy *policy = NULL;

	debug("Enter");

	datalen = sizeof(struct unsuccessful_association_policy);
	policy = (struct unsuccessful_association_policy *)os_zalloc(sizeof(struct unsuccessful_association_policy));
	if(!policy || !assoc_failed_policy) {
		err("Invalid Fields %p %p\n", policy, assoc_failed_policy);
		if(policy){
			os_free(policy);
		}
		return;
	}
	//printf("@@@ assoc_failed_policy->report_unsuccessful_association %d\n", assoc_failed_policy->report_unsuccessful_association);
	if(policy) {
		policy->report_unsuccessful_association = assoc_failed_policy->report_unsuccessful_association;
		policy->max_supporting_rate= assoc_failed_policy->max_supporting_rate;
		debug("@@@ policy->max_supporting_rate %d\n", policy->max_supporting_rate);
		debug("@@@ policy->report_unsuccessful_association %d\n", policy->report_unsuccessful_association);
		map_get_info_from_wapp(ctx, WAPP_USER_SET_UNSUCCESSFUL_ASSOC_POLICY, 0, NULL, NULL, (void *)policy, datalen);
		os_free(policy);
	}

}

int send_channel_scan_req( struct mapd_global *global, struct _1905_map_device *dev, unsigned char *scan_ch_list)
{
	struct channel_scan_req *scan_req = NULL;
	u8 buf[8000] = {0};
	u32 len;
	struct radio_info_db *radio = NULL;
	struct radio_scan_capab_db *radio_param = NULL;
	struct scan_body *scan_req_body = NULL;
	u8 op_idx = 0, ch_idx = 0, a = 0, op_req_idx = 0, ch_req_idx = 0;

	scan_req = (struct channel_scan_req *)buf;
	always("%s %d\n", __func__, __LINE__);
	scan_req->fresh_scan = 0x80;
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		if (radio->bh_priority != 1 || radio->radio_scan_params.boot_scan_only == BOOT_ONLY_SCAN) {
			err("For this radio %02x:%02x:%02x:%02x:%02x:%02x, BH priority is:%u, bootScanOnly %d",
						MAC2STR(radio->identifier), radio->bh_priority,
							radio->radio_scan_params.boot_scan_only);
			continue;
		}
		scan_req_body = &scan_req->body[scan_req->radio_num];
		op_req_idx= 0;
		radio_param = &radio->radio_scan_params;
		for (op_idx = 0; op_idx < radio_param->oper_class_num; op_idx++) {
			debug("Scan capab radio_param->ch_body[%d].oper_class %d", op_idx,
				radio_param->ch_body[op_idx].oper_class);
			debug("radio_param->ch_body[%d].ch_list_num %d", op_idx,
				radio_param->ch_body[op_idx].ch_list_num);
			ch_req_idx = 0;
			/*If ch list num is non zero than choose from the list, otherwise it suports all channels on that opclass*/
			if (radio_param->ch_body[op_idx].ch_list_num) {
				for (ch_idx = 0; ch_idx < radio_param->ch_body[op_idx].ch_list_num; ch_idx++) {
					a = 0;
					while(scan_ch_list[a]) {
						if(radio_param->ch_body[op_idx].ch_list[ch_idx] == scan_ch_list[a]) {
							scan_req_body->ch_body[op_req_idx].ch_list[ch_req_idx] = scan_ch_list[a];
							scan_req_body->ch_body[op_req_idx].ch_list_num++;
							ch_req_idx++;
						}
						a++;
					}
				}
			} else {
				a = 0;
				while(scan_ch_list[a]) {
					if(is_channel_in_opclass(radio_param->ch_body[op_idx].oper_class,
						BW_20, scan_ch_list[a])) {
						scan_req_body->ch_body[op_req_idx].ch_list[ch_req_idx] = scan_ch_list[a];
						scan_req_body->ch_body[op_req_idx].ch_list_num++;
						ch_req_idx++;
					}
					a++;
				}
			}
			if (ch_req_idx) {
				scan_req_body->ch_body[op_req_idx].oper_class = radio_param->ch_body[op_idx].oper_class;
				scan_req_body->oper_class_num++;
				op_req_idx++;
			}
		}
		if (op_req_idx) {
			os_memcpy(scan_req_body->radio_id, radio->identifier, ETH_ALEN);
			scan_req->radio_num++;
		}
	}
	if (scan_req->radio_num == 0) {
		dev->network_opt_per1905.network_opt_device_state = NETOPT_STATE_DATA_COLLECTION_COMPLETE;
		err ("No Fresh Scan allowed on any radio for this dev %02x:%02x:%02x:%02x:%02x:%02x",
				MAC2STR(dev->_1905_info.al_mac_addr));
		return 0;
	}
	len = sizeof(struct channel_scan_req) + scan_req->radio_num * sizeof(struct scan_body);
	always("%s: scanReqLen = %d\n", __func__, len);
	map_1905_Send_Channel_Scan_Request_Message(global->_1905_ctrl, (char *)dev->_1905_info.al_mac_addr,
		scan_req->fresh_scan, scan_req->radio_num, (unsigned char *)scan_req->body);

	return 0;
}
int topo_srv_handle_ack_msg(struct mapd_global *global, unsigned char *buf, int len) {
	struct _1905_map_device *dev = topo_srv_get_1905_device(&global->dev, buf);

	if (!dev) {
		err("mentioned 1905 device not found");
		return -1;
	}
	buf += ETH_ALEN;

	if (parse_1905_ack_message(&global->dev, dev, buf) < 0) {
		err("failed to parse resp msg");
		return -1;
	}

	return 0;
}

#endif

#ifdef DFS_CAC_R2
int topo_srv_handle_cac_req(struct own_1905_device *ctx, unsigned char *buf, int len)
{
	struct cac_request *cac = NULL;
	u8 *buff = NULL;
	u16 length=0;
	u8 *p = buf;
	u8 i=0;

	buff = os_malloc(len);
	if (buff == NULL) {
		err("\n cac req allocation failed");
		return FALSE;
	}

	os_memset(buff, 0, len);

	cac = (struct cac_request *)buff;

	if(*p != CAC_REQ_RESULT_TYPE) {
		err("CAC Req: wrong tlv: %d", (int)*p);
		os_free(buff);
		return FALSE;
	}

	p++;
	p+=2;  // skip length

	cac->num_radio = *p++;

	for(i=0; i < cac->num_radio; i++) {
		os_memcpy(&cac->tlv[i].identifier,p,ETH_ALEN);
		p+=ETH_ALEN;

		cac->tlv[i].op_class_num = *p++;
		cac->tlv[i].ch_num = *p++;
		err("channel %d",cac->tlv[i].ch_num);
		cac->tlv[i].cac_method = ((*p & 0xE0) >> 5);
		err("cac method: %d", cac->tlv[i].cac_method);
		cac->tlv[i].cac_action = ((*p & 0x18) >> 3);
		err("cac action: %d", cac->tlv[i].cac_action);
	}
#ifdef MAP_R2
	if (cac->num_radio) {
		ctx->ch_planning_R2.cac_ongoing = 1;
		ctx->ch_planning_R2.CAC_on_channel = cac->tlv[0].ch_num;
	}
#endif
	length = sizeof(struct cac_request) + cac->num_radio*sizeof(struct cac_tlv);
	//mapd_hexdump(MSG_OFF, "MAPD CAC_REQ", buff, length);
	wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_CAC_REQ,
			0, NULL, 0, buff, length, 0, 0, 0);

	os_free(buff);
	return 0;
}


int topo_srv_handle_cac_terminate(struct own_1905_device *ctx, unsigned char *buf, int len)
{
	struct cac_terminate *cac_term = NULL;
	u8 *buff = NULL;
	u16 length=0;
	u8 *p = buf;
	u8 i=0;

	buff = os_malloc(len);
	if (buff == NULL)
		err("\n cac req allocation failed");

	os_memset(buff, 0, len);

	cac_term = (struct cac_terminate *)buff;

	if(*p != CAC_TERMINATE_TYPE) {
		err("CAC Req: wrong tlv: %d", (int)*p);
		os_free(buff);
		return FALSE;
	}

	p++;
	p+=2;  // skip length

	cac_term->num_radio = *p++;

	for(i=0; i < cac_term->num_radio; i++) {
		os_memcpy(&cac_term->tlv[i].identifier,p,ETH_ALEN);
		p+=ETH_ALEN;

		cac_term->tlv[i].op_class_num = *p++;
		cac_term->tlv[i].ch_num = *p++;
	}

	length = sizeof(struct cac_terminate) + cac_term->num_radio*sizeof(struct cac_term_tlv);
	printf("Len = %d\n", length);
	mapd_hexdump(MSG_OFF, "MAPD CAC_TERMINATE_REQ", buff, length);
	wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_CAC_TERMINATE_REQ,
			0, NULL, 0, buff, length, 0, 0, 0);

	os_free(buff);
	return 0;
}

#endif
#ifdef CENT_STR
int mapd_mbo_parse_sta_npc_element(u8 *buf, u8 len, struct client *cli)
{
#ifdef MAP_R2
	u8 empty_npc = 0, i = 0;
	u8 np_list_len = 0;
	if (len == 0) {
		empty_npc = 1;
		err("Empty");
		return 0;
	}
	if (len > 3)
		np_list_len = len - 3;
	if (!cli)
		return 0;
	cli->np_reason = *(buf + np_list_len);
	cli->np_pref = *(buf + np_list_len + 1);
	for (i = 0;i < np_list_len; i++) {
		cli->np_channels[i] = *(buf + i);
	}
#endif
	return 1;
}
int parse_mbo_cap(u8 *buf, u8 len, int *mbo_capable, struct client *cli)
{
	unsigned short length = 0;
	int indicate_cdc = 0;
	int indicate_npc = 0;
	u8 *pos = NULL;
	u8 ParsedLen = 0;
	PEID_STRUCT eid_ptr;

	pos = buf;
	pos += 4;
	ParsedLen += 4;

	eid_ptr = (PEID_STRUCT)pos;
	//shift to tlv value field
	err("here");
	while ((ParsedLen+2) <= len) {
		switch (eid_ptr->Eid) {
		case MBO_ATTR_STA_CDC:
				indicate_cdc = 1;
				break;
		case MBO_ATTR_STA_NOT_PREFER_CH_REP:
				indicate_npc = 1;
#ifdef MAP_R2
				mapd_mbo_parse_sta_npc_element(&eid_ptr->Octet[0], eid_ptr->Len, cli);
#endif
				break;
		default:
				break;
		}

		ParsedLen += (2 + eid_ptr->Len);
		eid_ptr = (PEID_STRUCT)((u8 *)eid_ptr + 2 + eid_ptr->Len);
	}
	if (indicate_cdc || indicate_npc) {
		err("here");
		*mbo_capable = 1;
	}
	return (length+3);
}

int parse_ext_cap(EID_STRUCT *buf, int *btm_capable)
{
	unsigned short length = 0;
	EXT_CAP_INFO_ELEMENT ext_cap;

	// get val long long val = *temp_buf;
	if (buf->Len > sizeof(EXT_CAP_INFO_ELEMENT))
		os_memcpy(&ext_cap, buf->Octet, sizeof(EXT_CAP_INFO_ELEMENT));
	else
		os_memcpy(&ext_cap, buf->Octet, buf->Len);

	if (ext_cap.BssTransitionManmt == 1) {
		*btm_capable = 1;
	}

	return (length+3);
}

int parse_sup_op_class_cap(EID_STRUCT *buf, unsigned int *dual_band)
{
	unsigned short length = 0;
	int i = 0;
	int is_2g_band = 0, is_5g_band = 0;

	length = buf->Len;
	for (i = 0;i < length; i++) {
		if (is_op_class_2g(buf->Octet[i]))
			is_2g_band = 1;
		else if (is_op_class_5g(buf->Octet[i]))
			is_5g_band = 1;
	}
	if (is_2g_band && is_5g_band)
		*dual_band = BAND_SUPPORT_DUAL;
	else if (is_2g_band || is_5g_band)
		*dual_band = BAND_SUPPORT_SINGLE;
	else
		*dual_band = BAND_SUPPORT_AMBIGUOUS;
	return (length + 2);
}

int parse_rrm_cap(EID_STRUCT *buf, int *rrm_capable)
{
	unsigned short length = 0;
	RRM_EN_CAP_IE rrm_cap;

	os_memcpy(&rrm_cap.word, buf->Octet, sizeof(u64));
	if (rrm_cap.field.BeaconPassiveMeasureCap ||
		 rrm_cap.field.BeaconActiveMeasureCap) {
		 *rrm_capable = 1;
	}
	return length;
}
#if 0
int parse_supp_ch_cap(unsigned char *buf)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
	u8 SupportedChl[MAX_LEN_OF_SUPPORTED_CHL];
	unsigned short SupportedChlLen = 0;


	temp_buf = buf;

	if((*temp_buf) == IE_SUPP_CHANNELS) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	if (length > MAX_LEN_OF_SUPPORTED_CHL || (length % 2)) {
		err("wrong IE supported");
	} else if (length + SupportedChlLen <= MAX_LEN_OF_SUPPORTED_CHL) {
		u32 _ChlIdx = SupportedChlLen %
						 MAX_LEN_OF_SUPPORTED_CHL;
		os_memcpy(&SupportedChl[_ChlIdx], temp_buf,
					   length);
		SupportedChlLen += length;
	} else {
		os_memcpy(&SupportedChl[SupportedChlLen], temp_buf,
					   MAX_LEN_OF_SUPPORTED_CHL - SupportedChlLen);
		SupportedChlLen = MAX_LEN_OF_SUPPORTED_CHL;
	}

	if (SupportedChlLen > MAX_LEN_OF_SUPPORTED_CHL)
		SupportedChlLen = MAX_LEN_OF_SUPPORTED_CHL;

	return (length+3);
}
#endif
int parse_assoc_req
(
	struct mapd_global *global,
	unsigned char *sta_mac,
	unsigned char *bssid,
	unsigned char *assoc_req,
	unsigned int assoc_len,
	unsigned char channel,
	unsigned char assoc_req_format
)
{
	PFRAME_802_11	Fr = (PFRAME_802_11)assoc_req;
	PEID_STRUCT eid_ptr = NULL;
	int length = 0;
	unsigned char *temp_buf = NULL;
	int isReassoc = 0;
	uint32_t client_id = (uint32_t)-1;
	u8 already_seen = 0;
	int minus_len = 0;
	int rrm_capable = 0;
	int mbo_capable = 0;
	int btm_capable = 0;
	unsigned char band_idx = 0;
	struct client *cli = NULL;
	u8 is_80211 = 0;
	unsigned int dual_band = 0, supp_reg_present = 0;

	if(os_memcmp(Fr->Hdr.Addr2,sta_mac,ETH_ALEN) == 0){
		is_80211 = 1;
		err("Assoc Req contains the 802.11 header");
	}
	if(is_80211){
		if (Fr->Hdr.FC.SubType != SUBTYPE_ASSOC_REQ)
			isReassoc = 1;

		if (isReassoc) {
				err("Is Reassoc");
			eid_ptr = (PEID_STRUCT) &Fr->Octet[10];
			minus_len = sizeof(HEADER_802_11) + 9;
		} else {
			eid_ptr = (PEID_STRUCT) &Fr->Octet[4];
			minus_len = sizeof(HEADER_802_11) + 3;
		}
	} else {
		//Determine if client cap report contains assoc or reassoc req
		//Assume its an assoc req and check if the next IE after ssid IE is the supported cap IE.
		eid_ptr = (PEID_STRUCT) (&assoc_req[4]);
		temp_buf = 	(u8 *)((u8 *)eid_ptr + 2 + eid_ptr->Len);
		if (assoc_req_format == ASSOC_REQ_REASSOC)
			isReassoc = 1;
		else if (assoc_req_format == ASSOC_REQ_AMBIGUOUS) {
			if(*temp_buf != 1){
				err("Is Reassoc");
				isReassoc = 1;
			}
		}

		if (isReassoc) {
			eid_ptr = (PEID_STRUCT) &assoc_req[10];
			minus_len = 9;
		} else {
			eid_ptr = (PEID_STRUCT) &assoc_req[4];
		minus_len = 3;
		}

	}
	mapd_hexdump(MSG_ERROR,"ClicapAssocReq",(unsigned char *)eid_ptr,assoc_len - minus_len);

//	temp_buf = (unsigned char *)eid_ptr;
	client_id = client_db_track_add(global, sta_mac, &already_seen);
	if (client_id == (uint32_t)-1) {
		 mapd_printf(MSG_ERROR, "No more room to accomodate" MACSTR
					", that is discovered", MAC2STR(sta_mac));
		 return -1;
	}
	if (already_seen != 1) {
		 mapd_printf(MSG_DEBUG, "New Client discovered"
					MACSTR, MAC2STR(sta_mac));
	}
	if (channel < 14)
		band_idx = BAND_2G_IDX;
	else
		band_idx = BAND_5G_IDX;
	client_db_update_cli_ht_vht_cap(global, client_id,(u8 *) eid_ptr, assoc_len - minus_len, band_idx);
	/* get variable fields from payload and advance the pointer */


	while (((u8 *)eid_ptr + eid_ptr->Len + 1) < ((u8 *)assoc_req + assoc_len)) {
		switch (eid_ptr->Eid) {
			case IE_RRM_EN_CAP:
				length = parse_rrm_cap(eid_ptr, &rrm_capable);
				break;
			case IE_WPA:
				if (os_memcmp(eid_ptr->Octet, MBO_OCE_OUI_BYTE, sizeof(MBO_OCE_OUI_BYTE)) == 0 && (eid_ptr->Len >= 5)) {
					length = parse_mbo_cap(eid_ptr->Octet, eid_ptr->Len, &mbo_capable, client_db_get_client_from_client_id(global, client_id));
				}
				break;
			case IE_EXT_CAPABILITY:
				length = parse_ext_cap(eid_ptr, &btm_capable);
				break;
			case IE_SUPP_REG_CLASS:
				supp_reg_present = 1;
				length = parse_sup_op_class_cap(eid_ptr, &dual_band);
				break;
			default:
				break;
		}
		eid_ptr = (PEID_STRUCT)((u8 *)eid_ptr + 2 + eid_ptr->Len);
	}
	err("BTM: %d RRM: %d MBO: %d", btm_capable, rrm_capable, mbo_capable);
	client_db_set_known_channels(global, client_id, channel);
	client_db_set_capab(global, client_id, btm_capable, rrm_capable, mbo_capable);
	client_db_set_curr_channel(global, client_id, channel);
	cli = client_db_get_client_from_client_id(global, client_id);
	if (supp_reg_present == 1)
		cli->dual_band = dual_band;
	else
		cli->dual_band = BAND_SUPPORT_AMBIGUOUS;
	steer_action_sta_join(global, cli, 0);
	/* Trigger FSM */
	mapd_printf(MSG_OFF, "Trigger FSM (REMOTE_TOPOLOGY_NOTIFICATION)");
	steer_fsm_trigger(global, client_id, CLIENT_ASSOCIATED, NULL);
	return length;

}

int topo_srv_handle_client_cap_report(struct mapd_global *global, unsigned char *buf, unsigned short len)
{
	unsigned char temp_bssid[ETH_ALEN] = {0};
	unsigned char temp_sta[ETH_ALEN] = {0};
	unsigned char assoc_req[1024] = {0};
	unsigned int assoc_len = 0;
	unsigned char channel = 0;
	struct bss_info_db *bss_db = NULL;
	struct _1905_map_device *dev = topo_srv_get_1905_device(&global->dev, buf);
	struct associated_clients *sta = NULL;

	if (!dev) {
		err("failed to get device");
		return -1;
	}
	if (global->dev.device_role == DEVICE_ROLE_AGENT) {
		err("We are at the agent");
		return -1;
	}
	if (parse_client_capability_report_message(&global->dev, buf + ETH_ALEN, temp_bssid, temp_sta, assoc_req, &assoc_len) < 0) {
		err("error! parse client capability report message");
		return -1;
	}
	bss_db = topo_srv_get_bss(dev,(char*)temp_bssid);
	channel = bss_db->radio->channel[0];
	// Extract AL mac Cli mac --> done
	client_mon_handle_remote_join(global, temp_sta, temp_bssid, dev->_1905_info.al_mac_addr);
	// Handled remote join
	// Parsing Assoc Request
	parse_assoc_req(global, temp_sta, temp_bssid, assoc_req, assoc_len, channel, ASSOC_REQ_AMBIGUOUS);
	// Update known channels
	// Call client_db_update_cli_ht_vht_cap() to update client db's.
	sta = topo_srv_get_associate_client(&global->dev, dev, temp_sta);
	if(sta) {
		os_get_time(&(sta->stat_db.last_traffic_stats_time));
		sta->stat_db.last_bytes_recieved = 0;
		sta->stat_db.last_bytes_sent = 0;
	}

	return 0;
}
#endif

void topo_srv_update_radio_config_status(struct own_1905_device *ctx, unsigned char *radio_id, char flag)
{
	struct radio_info_db *radio = NULL;
	unsigned char ra_config_num = 0;
	unsigned char ra_non_config_num = 0;
	unsigned char total_ra_num = 0;

	radio = topo_srv_get_radio(topo_srv_get_1905_device(ctx, NULL), NULL);
	while (radio) {
		if(radio_id) {
			if(!os_memcmp(radio->identifier, radio_id, ETH_ALEN))
			{

				radio->config_status = flag;
			}
		} else {
			radio->config_status = flag;
		}
		err("radio (%d) config_status set as %d\n", radio->channel[0], radio->config_status);
		if (radio->config_status == TRUE)
			ra_config_num++;
		else
			ra_non_config_num++;
		total_ra_num++;
		radio = topo_srv_get_next_radio(topo_srv_get_1905_device(ctx, NULL), radio);
	}
	if (total_ra_num && (ra_config_num == total_ra_num))
		ap_selection_update_vend_ie(ctx, NULL, TRUE);
	else if (total_ra_num && (ra_non_config_num == total_ra_num))
		ap_selection_update_vend_ie(ctx, NULL, FALSE);
}
void send_vs_bh_priority(struct own_1905_device *ctx)
{
	struct bh_priority_msg message;
	struct bh_link_entry *bh_entry = NULL;
	struct _1905_map_device *p1905_device = topo_srv_get_controller_device(ctx);
	struct mapd_global *pGlobal_dev = (struct mapd_global *)ctx->back_ptr;
	u8 i = 0;

	os_memset(&message, '\0', sizeof(struct bh_priority_msg));

	if (p1905_device == NULL)
		return;
	SLIST_FOREACH(bh_entry, &(ctx->bh_link_head), next_bh_link) {
		os_memcpy(message.bh_tlv[i].radio_id, bh_entry->radio_identifier, ETH_ALEN);
		message.bh_tlv[i].bh_priority = bh_entry->priority_info.priority;
		i++;
	}
	if (i != 0) {
		os_memcpy(message.tlv.oui, MTK_OUI, OUI_LEN);
		message.tlv.tlv_type = TLV_802_11_VENDOR_SPECIFIC;
		message.tlv.func_type = FUNC_BH_PRIORITY_INFO;
		message.tlv.tlv_len = host_to_be16(sizeof(struct bh_priority_msg) - sizeof(struct tlv_head));
		err("Send BH priority info to almac " MACSTR, MAC2STR(p1905_device->_1905_info.al_mac_addr));
		map_1905_Send_Vendor_Specific_Message(pGlobal_dev->_1905_ctrl,
			(char *)p1905_device->_1905_info.al_mac_addr,
			(char *)&message, sizeof(struct bh_priority_msg));
	}
	return;
}
void handle_bh_priority_info_from_agent(struct mapd_global *pGlobal_dev,
	struct bh_priority_msg *bh_info,
	struct _1905_map_device *p1905_device)
{
	struct radio_info_db *radio = NULL;
	u8 i = 0;

	if (p1905_device == NULL)
		return;
	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		SLIST_FOREACH(radio, &p1905_device->first_radio, next_radio) {
			if (os_memcmp(radio->identifier, bh_info->bh_tlv[i].radio_id, ETH_ALEN) == 0) {
				radio->bh_priority = bh_info->bh_tlv[i].bh_priority;
			}
		}
	}
	return;
}

void send_cac_start(struct mapd_global *pGlobal_dev)
{
	struct cac_start_tlv req;
	struct _1905_map_device *p1905_device = topo_srv_get_controller_device(&pGlobal_dev->dev);
	if (!p1905_device)
		return;
	struct _1905_map_device *own_dev = topo_srv_get_1905_device(&pGlobal_dev->dev, NULL);
	if (!own_dev)
		return;
	struct radio_info_db *radio = topo_srv_get_radio(own_dev, NULL);
	if (!radio)
		return;
	while (radio) {
		if (radio->cac_enable && radio->cac_timer && !radio->send_cac_start_event) {
			if (pGlobal_dev->dev.current_bh_state == BH_STATE_WIFI_LINKUP) {
				os_memset(&req, 0, sizeof(struct cac_start_tlv));
				req.tlv.tlv_type = TLV_802_11_VENDOR_SPECIFIC;
				req.tlv.tlv_len = host_to_be16(sizeof(struct cac_start_tlv));
				memcpy(req.tlv.oui, MTK_OUI, OUI_LEN);
				req.tlv.func_type = FUNC_CAC_START;
				req.cac_start.cac_channel = radio->cac_channel;
				req.cac_start.cac_enable = radio->cac_enable;
				req.cac_start.cac_timer = radio->cac_timer;
				err("Send CAC START to almac " MACSTR, MAC2STR(p1905_device->_1905_info.al_mac_addr));
					info("map_1905_Send_Vendor_Specific_Message\n");
					map_1905_Send_Vendor_Specific_Message(pGlobal_dev->_1905_ctrl,
						(char *)p1905_device->_1905_info.al_mac_addr,
						(char *)&req, sizeof(struct cac_start_tlv));
				radio->send_cac_start_event = 1;
				radio->cac_timer--;
			} else {
				radio->cac_timer--;
			}
		} else if ((radio->send_cac_start_event == 1) && (radio->cac_timer == 0)) {
			radio->send_cac_start_event = 0;
		} else if (radio->cac_enable && radio->cac_timer)
			radio->cac_timer--;
		radio = topo_srv_get_next_radio(own_dev, radio);
	}
}

void handle_cac_start_from_agent(struct mapd_global *pGlobal_dev,
	struct cac_start_tlv *cac_tlv,
	struct _1905_map_device *p1905_device)
{
	struct radio_info_db *radio = NULL;

	radio = topo_srv_get_radio_by_band(p1905_device, cac_tlv->cac_start.cac_channel);
	radio->cac_channel = cac_tlv->cac_start.cac_channel;
	radio->cac_enable = cac_tlv->cac_start.cac_enable;
	radio->cac_timer = cac_tlv->cac_start.cac_timer;
	err("CAC ongoing on" MACSTR, MAC2STR(p1905_device->_1905_info.al_mac_addr));
	err("Channel %d, Timer %d", radio->cac_channel, radio->cac_timer);
#if 0
	eloop_cancel_timeout(trigger_net_opt, (void *)&pGlobal_dev->dev, NULL);
	network_opt_reset(pGlobal_dev);
	eloop_register_timeout(radio->cac_timer + pGlobal_dev->dev.network_optimization.post_cac_trigger_time, 0, trigger_net_opt, &pGlobal_dev->dev, NULL);
#endif
}

int send_link_metrics_selective(struct associated_clients *metrics_ctx,struct own_1905_device *ctx)

{
	struct metric_policy_db *policy_db = NULL;
	struct metrics_policy *mpolicy = &ctx->map_policy.mpolicy;
	uint8_t HysteresisInPolicy = 0, HysteresisMargin, fStaRssiAlert =0;
	uint8_t RssiThInPolicy = 0,CompareThreshold = 0;


	SLIST_FOREACH(policy_db, &(mpolicy->policy_head), policy_entry){
		if(os_memcmp(policy_db->identifier,metrics_ctx->bss->radio->identifier,ETH_ALEN) == 0){
			RssiThInPolicy = policy_db->rssi_thres;
			HysteresisInPolicy = policy_db->hysteresis_margin;
			break;
		}
	}

	if( -127 == (signed char)metrics_ctx->rssi_uplink) {
		 mapd_printf(MSG_ERROR, "Rssi is -127 that is inavlid");
		 return 0;
	}



	debug("Final RssiThInPolicy %d, HysteresisInPolicy %d", RssiThInPolicy,HysteresisInPolicy);
	if(RssiThInPolicy == 0){
		debug("Avoid STA monitor Radio Policy->RSSI_Th is 0");
		return 0;
	}
	if(HysteresisInPolicy > 0){
		HysteresisMargin = HysteresisInPolicy;
	}else{
		HysteresisMargin = MAX_HYSTERESIS_MARGIN;
	}
	metrics_ctx->rssi_uplink = rssi_to_rcpi((signed char)metrics_ctx->rssi_uplink);
	CompareThreshold = RssiThInPolicy;
	debug("CompareTH %d,HM %d,cliUL %d,last %d",CompareThreshold,HysteresisMargin,metrics_ctx->rssi_uplink, metrics_ctx->LastReportedUlRssi);
	if(metrics_ctx->rssi_uplink <= (CompareThreshold + HysteresisMargin)){
		debug("BAD ZONE");
		metrics_ctx->MonitorRcpi = 1;
		if(metrics_ctx->LastReportedUlRssi == 0) {
			debug("NEED to send");
			fStaRssiAlert = 1;
			metrics_ctx->LastReportedUlRssi = metrics_ctx->rssi_uplink;
		}else if(metrics_ctx->LastReportedUlRssi > metrics_ctx ->rssi_uplink){
			if((metrics_ctx->LastReportedUlRssi - metrics_ctx ->rssi_uplink) > HysteresisMargin){
				debug("NEED to send");
				fStaRssiAlert = 1;
				metrics_ctx->LastReportedUlRssi = metrics_ctx->rssi_uplink;
			}
		}else{
			if((metrics_ctx->rssi_uplink - metrics_ctx->LastReportedUlRssi) > HysteresisMargin){
				debug("NEED to send");
				fStaRssiAlert = 1;
				metrics_ctx->LastReportedUlRssi = metrics_ctx->rssi_uplink;
			}
		}
	}
	if((metrics_ctx->MonitorRcpi == 1) &&
		(metrics_ctx->rssi_uplink > (CompareThreshold + HysteresisMargin)) &&
		(metrics_ctx->rssi_uplink > (metrics_ctx->LastReportedUlRssi + HysteresisMargin))) {
		debug("NEED to send just entering good zone by a good margin");
		fStaRssiAlert = 1;
		metrics_ctx->MonitorRcpi = 0;
	}
metrics_ctx->rssi_uplink = rcpi_to_rssi(metrics_ctx->rssi_uplink);
return fStaRssiAlert;
}

#ifdef CENT_STR
int link_metrics_mon_rcpi_at_controller(struct associated_clients *metrics_ctx,struct own_1905_device *ctx)

{
	uint8_t HysteresisInPolicy = 0, HysteresisMargin, fStaRssiAlert =0;
	uint8_t RssiThInPolicy = 0,CompareThreshold = 0;
	struct _1905_map_device * own_device = NULL;
	struct radio_info_db * radio_tmp = NULL;
	u8 band = 0;

	if(!metrics_ctx)
		return 0;

	if(!metrics_ctx->bss)
		return 0;

	/*Get controller 1905 device*/
	own_device = topo_srv_get_1905_by_bssid(ctx , metrics_ctx->bss->bssid);

	if(!own_device) {
		err("own device is NULL");
		return 0;
	}

	if( -127 == (signed char)metrics_ctx->rssi_uplink) {
		 mapd_printf(MSG_ERROR, "Rssi is -127 that is inavlid");
		 return 0;
	}


	SLIST_FOREACH(radio_tmp, &(own_device->first_radio), next_radio){

		if(radio_tmp->channel[0] == 0)
			continue;

		if(os_memcmp(radio_tmp->identifier,metrics_ctx->bss->radio->identifier,ETH_ALEN) == 0){

			if(isChan5GH(radio_tmp->channel[0])) {
				band = BAND_5GH;
			} else if (isChan5GL(radio_tmp->channel[0])) {
				band = BAND_5GL;

			} else if(radio_tmp->channel[0] <= 14) {
				band = BAND_2G;
			}
			if(band > 0) {
				RssiThInPolicy = ctx->controller_context.ap_metric_policy.policy_params[band-1].MetricPolicyRcpi;
				HysteresisInPolicy = ctx->controller_context.ap_metric_policy.policy_params[band-1].MetricPolicyHys;
			}
			break;
		}
	}




	debug("Final RssiThInPolicy %d, HysteresisInPolicy %d", RssiThInPolicy,HysteresisInPolicy);
	if(RssiThInPolicy == 0){
		debug("Avoid STA monitor Radio Policy->RSSI_Th is 0");
		return 0;
	}
	if(HysteresisInPolicy > 0){
		HysteresisMargin = HysteresisInPolicy;
	}else{
		HysteresisMargin = MAX_HYSTERESIS_MARGIN;
	}
	metrics_ctx->rssi_uplink = rssi_to_rcpi((signed char)metrics_ctx->rssi_uplink);
	CompareThreshold = RssiThInPolicy;
	debug("CompareTH %d,HM %d,cliUL %d,last %d",CompareThreshold,HysteresisMargin,metrics_ctx->rssi_uplink, metrics_ctx->LastReportedUlRssi);
	if(metrics_ctx->rssi_uplink <= (CompareThreshold + HysteresisMargin)){
		debug("BAD ZONE");
		metrics_ctx->MonitorRcpi = 1;
		if(metrics_ctx->LastReportedUlRssi == 0) {
			debug("NEED to send");
			fStaRssiAlert = 1;
			metrics_ctx->LastReportedUlRssi = metrics_ctx->rssi_uplink;
		}else if(metrics_ctx->LastReportedUlRssi > metrics_ctx ->rssi_uplink){
			if((metrics_ctx->LastReportedUlRssi - metrics_ctx ->rssi_uplink) > HysteresisMargin){
				debug("NEED to send");
				fStaRssiAlert = 1;
				metrics_ctx->LastReportedUlRssi = metrics_ctx->rssi_uplink;
			}
		}else{
			if((metrics_ctx->rssi_uplink - metrics_ctx->LastReportedUlRssi) > HysteresisMargin){
				debug("NEED to send");
				fStaRssiAlert = 1;
				metrics_ctx->LastReportedUlRssi = metrics_ctx->rssi_uplink;
			}
		}
	}
	if((metrics_ctx->MonitorRcpi == 1) &&
		(metrics_ctx->rssi_uplink > (CompareThreshold + HysteresisMargin)) &&
		(metrics_ctx->rssi_uplink > (metrics_ctx->LastReportedUlRssi + HysteresisMargin))) {
		debug("NEED to send just entering good zone by a good margin");
		fStaRssiAlert = 1;
		metrics_ctx->MonitorRcpi = 0;
	}
	metrics_ctx->rssi_uplink = rcpi_to_rssi(metrics_ctx->rssi_uplink);
return fStaRssiAlert;
}

#endif

void topo_srv_update_wts_config(struct own_1905_device *ctx)
{
	map_get_info_from_wapp(ctx, WAPP_USER_GET_WTS_CONFIG, WAPP_WTS_CONFIG,
				   NULL, NULL, NULL, 0);
	return;
}


