/*
 * Broadcom Dongle Host Driver (DHD) - CM WiFI header file
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */
#ifndef __DHD_CMWIFI_H__
#define __DHD_CMWIFI_H__

/* DHD CM utility wrappers */
#define DHD_CM_CTX(dhdp)               dhd_cm_get_ctx(dhdp)
#define DHD_CM_SET_CTX(dhdp, cm_ctx)   dhd_cm_set_ctx(dhdp, cm_ctx)
#define DHD_IFP(dhdp, idx)             dhd_cm_get_ifp(dhdp, idx)
#define DHD_IFP_AP_ISOLATE(dhdp, idx)  dhd_if_ap_isolate(dhdp, idx)
#define DHD_IF_BSS_UP(dhdp, idx)       dhd_if_bss_up(dhdp, idx)
#define DHD_IF_WMF_ENABLE(dhdp, idx)   dhd_if_bss_wmf_enabled(dhdp, idx)

typedef struct dhd_cm_ctx {
	dhd_pub_t *pub;	/* common DHD handle */
	int cm_dor_ver;
	int cm_dor_state;
	uint32 cm_dor_wmf_pkt_cnt;
	struct sk_buff_head *cm_dor_wmf_txq; /* WMF converted list of unicast pkt frames */
} dhd_cm_ctx_t;

void dhd_offload_wmf_init(dhd_pub_t *dhdp);
void dhd_dor_wmf_txq_reset(dhd_pub_t *dhdp);
void dhd_offload_wmf_dump(dhd_pub_t *dhdp);
void dhd_offload_wmf_deinit(dhd_pub_t *dhdp);

/* DoR CM V1 APIs */
int dhd_offload_map_flowring(void *ctx, u8 *buf, int buf_len, unsigned char priority,
	int if_idx, int *radio, int *flowring);
void * dhd_find_intrabss_sta(void *ctx, int ifidx, void *ea);

/* DoR CM V2 APIs */
/* CM Runner team's new APIs replacing dhd_offload_map_flowring(),
 * and dhd_find_intrabss_sta()
 */
int dhd_tx_skb_nethook(void *ctx, int ifidx, void *p, struct sk_buff_head *txq);
int dhd_rx_skb_nethook(void *ctx, int ifidx, void *p);

int BCMFASTPATH dhd_offload_sendpkt(dhd_pub_t *dhdp, int ifidx, void *pktbuf);

/* DHD CM utility functions */
dhd_cm_ctx_t *dhd_cm_get_ctx(dhd_pub_t *dhdp);
void dhd_cm_set_ctx(dhd_pub_t *dhdp, dhd_cm_ctx_t *cm_ctx);
void *dhd_cm_get_ifp(dhd_pub_t *dhdp, int ifidx);
bool dhd_if_ap_isolate(dhd_pub_t *dhdp, int ifidx);
bool dhd_if_bss_up(dhd_pub_t *dhdp, int ifidx);
bool dhd_if_bss_wmf_enabled(dhd_pub_t *dhdp, int ifidx);

int dhd_get_flowid(dhd_pub_t *dhdp, uint8 ifindex,
                                uint8 prio, char *sa, char *da, uint16 *flowid);

#endif /* __DHD_CMWIFI_H__ */
