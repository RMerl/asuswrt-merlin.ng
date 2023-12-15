/* $Id$ */

/*
 *   Copyright (c) 2001-2010 Aaron Turner <aturner at synfin dot net>
 *   Copyright (c) 2013-2022 Fred Klassen <tcpreplay at appneta dot com> - AppNeta
 *
 *   The Tcpreplay Suite of tools is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, either version 3 of the
 *   License, or with the authors permission any later version.
 *
 *   The Tcpreplay Suite is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with the Tcpreplay Suite.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "plugins_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 802.11 packet header w/ 3 addresses (non-WDS) */
typedef struct {
    u_int16_t frame_control;
/* version is first two bytes */
#define ieee80211_FC_VERSION_MASK 0x0300

/* type is second 2 bytes */
#define ieee80211_FC_TYPE_MASK 0x0F00
#define ieee80211_FC_TYPE_DATA 0x0800
#define ieee80211_FC_TYPE_MGMT 0x0000
#define ieee80211_FC_TYPE_CONTROL 0x0400

/* subtype is the 4 high bytes */
#define ieee80211_FC_SUBTYPE_MASK 0xF000
#define ieee80211_FC_SUBTYPE_QOS 0x8000  /* high bit is QoS, but there are sub-sub types for QoS */
#define ieee80211_FC_SUBTYPE_NULL 0xC000 /* no data */

/* Direction */
#define ieee80211_FC_TO_DS_MASK 0x0001
#define ieee80211_FC_FROM_DS_MASK 0x0002

/* Flags */
#define ieee80211_FC_MORE_FRAG 0x0004
#define ieee80211_FC_RETRY_MASK 0x0008
#define ieee80211_FC_PWR_MGMT_MASK 0x0010
#define ieee80211_FC_MORE_DATA_MASK 0x0020
#define ieee80211_FC_WEP_MASK 0x0040
#define ieee80211_FC_ORDER_MASK 0x0080
    u_int16_t duration;
    u_char addr1[6];
    u_char addr2[6];
    u_char addr3[6];
    u_int16_t fragid;
} ieee80211_hdr_t;

typedef struct {
    u_int16_t frame_control;
    u_int16_t duration;
    u_char addr1[6];
    u_char addr2[6];
    u_char addr3[6];
    u_char addr4[6];
    u_int16_t fragid;
} ieee80211_addr4_hdr_t;

#define ieee80211_USE_4(frame_control)                                                                                 \
    (frame_control & (ieee80211_FC_TO_DS_MASK + ieee80211_FC_FROM_DS_MASK)) ==                                         \
            (ieee80211_FC_TO_DS_MASK + ieee80211_FC_FROM_DS_MASK)

/*
 * FIXME: structure to hold any data parsed from the packet by the decoder.
 * Example: Ethernet VLAN tag info
 */
typedef struct {
    u_char packet[MAXPACKET];
} ieee80211_extra_t;

/*
 * FIXME: structure to hold any data in the tcpeditdlt_plugin_t->config
 * Things like:
 * - Parsed user options
 * - State between packets
 * - Note, you should only use this for the encoder function, decoder functions should place
 *   "extra" data parsed from the packet in the tcpeditdlt_t->decoded_extra buffer since that
 *   is available to any encoder plugin.
 */
typedef struct {
    /* dummy entry for SunPro compiler which doesn't like empty structs */
    int dummy;
} ieee80211_config_t;

#ifdef __cplusplus
}
#endif
