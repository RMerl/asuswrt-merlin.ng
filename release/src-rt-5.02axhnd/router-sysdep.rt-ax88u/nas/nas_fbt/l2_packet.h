/*
 * WPA Supplicant - Layer2 packet handling
 * Copyright (c) 2003-2004, Jouni Malinen <jkmaline@cc.hut.fi>
 * Copyright (c) 2005-2006, Devicescape Software, Inc. All Rights Reserved.
 *
 *
 * Copyright 2019 Broadcom
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
 *
 * $Id: l2_packet.h 701315 2017-05-24 13:08:15Z $
 */

#ifndef L2_PACKET_H
#define L2_PACKET_H

#include <typedefs.h>

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

#ifndef ETH_P_EAPOL
#define ETH_P_EAPOL 0x888e
#endif // endif

#ifndef ETH_P_RSN_PREAUTH
#define ETH_P_RSN_PREAUTH 0x88c7
#endif // endif

#ifndef ETH_P_8021Q
#define ETH_P_8021Q 0x8100
#endif // endif

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif // endif

struct l2_packet_data {
	int fd; /* packet socket for EAPOL frames */
	char ifname[20];
	uint8 own_addr[ETH_ALEN];
	void (*rx_callback)(void *ctx, unsigned char *src_addr, unsigned char *buf, size_t len);
	void *rx_callback_ctx;
};

struct l2_ethhdr {
    uint8 h_dest[ETH_ALEN];
    uint8 h_source[ETH_ALEN];
    uint16 h_proto;
} __attribute__ ((packed));

struct l2_vlanhdr {
    uint16 vlan_id;
    uint16 h_proto;
} __attribute__ ((packed));

struct l2_packet_data * l2_packet_init(
    const char *ifname, unsigned short protocol,
    void (*rx_callback)(void *ctx, unsigned char *src_addr,
                unsigned char *buf, size_t len),
    void *rx_callback_ctx);
void l2_packet_deinit(struct l2_packet_data *l2);

int l2_packet_get_own_addr(struct l2_packet_data *l2, uint8 *addr);
int l2_packet_send(struct l2_packet_data *l2, uint8 *buf, size_t len);

#endif /* L2_PACKET_H */
