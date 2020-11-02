/*
 * WLAN iovar functions.
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
 * $Id:$
 */

#ifndef _WLU_API_H_
#define _WLU_API_H_

#include "typedefs.h"
#include "wlioctl.h"

#define DEFAULT_BSSCFG_INDEX	(-1)

#define MAX_WLIF_NUM	8

void *wl(void);
void *wlif(int index);
void *wl_getifbyname(char *ifname);
void wlFree(void);

int wl_open(void **wl);
void wl_close(void);
char *wl_ifname(void *wl);

int wlu_get(void *wl, int cmd, void *buf, int len);
int wlu_set(void *wl, int cmd, void *buf, int len);

int wlu_iovar_get(void *wl, const char *iovar, void *outbuf, int len);
int wlu_iovar_set(void *wl, const char *iovar, void *param, int paramlen);

int wlu_iovar_getint(void *wl, const char *iovar, int *pval);
int wlu_iovar_setint(void *wl, const char *iovar, int val);

int wlu_iovar_setbuf(void* wl, const char *iovar,
	void *param, int paramlen, void *bufptr, int buflen);

int wlu_var_getbuf(void *wl, const char *iovar, void *param, int param_len, void **bufptr);
int wlu_var_setbuf(void *wl, const char *iovar, void *param, int param_len);

int wlu_bssiovar_setbuf(void* wl, const char *iovar, int bssidx,
	void *param, int paramlen, void *bufptr, int buflen);

int wlu_bssiovar_get(void *wl, const char *iovar, int bssidx, void *outbuf, int len);

int wl_cur_etheraddr(void *wl, int bsscfg_idx, struct ether_addr *ea);

int wl_format_ssid(char* ssid_buf, uint8* ssid, int ssid_len);

char *wl_ether_etoa(const struct ether_addr *n);

int wl_escan(void *wl, uint16 sync_id, int isActive,
	int numProbes, int activeDwellTime, int passiveDwellTime,
	int num_channels, uint16 *channels);
int wl_escan_abort(void *wl, uint16 sync_id);

int wl_scan_abort(void *wl);

int wl_actframe(void *wl, int bsscfg_idx, uint32 packet_id,
	uint32 channel, int32 dwell_time,
	struct ether_addr *BSSID, struct ether_addr *da,
	uint16 len, uint8 *data);

int wl_wifiaction(void *wl, uint32 packet_id,
	struct ether_addr *da, uint16 len, uint8 *data);

int wl_enable_event_msg(void *wl, int event);
int wl_disable_event_msg(void *wl, int event);

int wl_add_vndr_ie(void *wl, int bsscfg_idx, uint32 pktflag, int len, uchar *data);
int wl_del_vndr_ie(void *wl, int bsscfg_idx, uint32 pktflag, int len, uchar *data);
int wl_del_all_vndr_ie(void *wl, int bsscfg_idx);

int wl_ie(void *wl, uchar id, uchar len, uchar *data);

#ifdef __CONFIG_DHDAP__
int dhd_ie(void *wl, uchar id, uchar len, uchar *data);

int dhd_probe(char *name);
int dhd_bssiovar_set(char *ifname, char *iovar, int bssidx, void *param, int paramlen);
int dhd_bssiovar_setint(char *ifname, char *iovar, int bssidx, int val);
#endif /* __CONFIG_DHDAP__ */

int wl_bssiovar_setint(char *ifname, char *iovar, int bssidx, int val);
int get_ifname_unit(const char* ifname, int *unit, int *subunit);

int wl_get_channels(void *wl, int max, int *len, uint16 *channels);
int wl_is_dfs(void *wl, uint16 channel);

int wl_disassoc(void *wl);
int wl_pmf_disassoc(void *wl);

int wl_wnm_bsstrans_query(void *wl);
int wl_wnm_bsstrans_req(void *wl, uint8 reqmode, uint16 tbtt, uint16 dur, uint8 unicast);

int wl_tdls_enable(void *wl, int enable);
int wl_tdls_endpoint(void *wl, char *cmd, struct ether_addr *ea);

int wl_status(void *wl, int *isAssociated, int biBufferSize, wl_bss_info_t *biBuffer);

int wl_grat_arp(void *wl, int enable);
int wl_bssload(void *wl, int enable);
int wl_dls(void *wl, int enable);
int wl_wnm(void *wl, int mask);
int wl_wnm_get(void *wl, int *mask);
int wl_wnm_parp_discard(void *wl, int enable);
int wl_wnm_parp_allnode(void *wl, int enable);
int wl_interworking(void *wl, int enable);
int wl_probresp_sw(void *wl, int enable);
int wl_block_ping(void *wl, int enable);
int wl_block_sta(void *wl, int enable);
int wl_ap_isolate(void *wl, int enable);
int wl_proxy_arp(void *wl, int enable);
int wl_block_tdls(void *wl, int enable);
int wl_dls_reject(void *wl, int enable);
int wl_dhcp_unicast(void *wl, int enable);
int wl_wmf_bss_enable(void *wl, int enable);
int wl_block_multicast(void *wl, int enable);
int wl_gtk_per_sta(void *wl, int enable);
int wl_wnm_url(void *wl, uchar datalen, uchar *url_data);
int wl_pmf(void *wl, int mode);
int wl_mac(void *wl, int count, struct ether_addr *bssid);
int wl_macmode(void *wl, int mode);
int wl_osen(void *wl, int enable);
int wl_send_frame(void *wl, int len, uchar *frame);
int wl_bssload_static(void *wl, bool is_static, uint16 sta_count,
	uint8 chan_util, uint16 aac);

int wl_p2p_disc(void *wl, int enable);
int wl_p2p_state(void *wl, uint8 state, chanspec_t chspec, uint16 dwell);
int wl_p2p_scan(void *wl, uint16 sync_id, int isActive,
	int numProbes, int activeDwellTime, int passiveDwellTime,
	int num_channels, uint16 *channels);
int wl_p2p_if(void *wl, struct ether_addr *ea, int *bsscfgIndex);
int wl_p2p_dev(void *wl, int *bsscfgIndex);

int wl_lci(void *wl, int bsscfg_idx, uint16 *buflen, uint8 *buf);
int wl_civic(void *wl, int bsscfg_idx, uint16 *buflen, uint8 *buf);

int wl_app_serve_anqp_rqst(void *wl, uint8 enable);
int wl_get_nbr_list(void *wl, uint8* buf, uint16 len, uint16 *nbytes_rd);
#endif /* _WLU_API_H_ */
