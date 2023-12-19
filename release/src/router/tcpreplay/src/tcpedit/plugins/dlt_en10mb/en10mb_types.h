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

typedef struct {
    int vlan; /* set to 1 for vlan_ fields being filled out */

    u_int32_t vlan_offset;
    u_int16_t vlan_tag;
    u_int16_t vlan_pri;
    u_int16_t vlan_cfi;
    u_int16_t vlan_proto;
    bool src_modified;
    bool dst_modified;
} en10mb_extra_t;

typedef enum {
    TCPEDIT_MAC_MASK_SMAC1 = 1,
    TCPEDIT_MAC_MASK_SMAC2 = 2,
    TCPEDIT_MAC_MASK_DMAC1 = 4,
    TCPEDIT_MAC_MASK_DMAC2 = 8
} tcpedit_mac_mask;

typedef enum {
    TCPEDIT_VLAN_OFF = 0,
    TCPEDIT_VLAN_DEL, /* strip 802.1q and rewrite as standard 802.3 Ethernet */
    TCPEDIT_VLAN_ADD  /* add/replace 802.1q vlan tag */
} tcpedit_vlan;

typedef struct {
    tcpr_macaddr_t target;
    tcpr_macaddr_t rewrite;
} en10mb_sub_entry_t;

typedef struct {
    int count;
    en10mb_sub_entry_t *entries;
} en10mb_sub_conf_t;

typedef struct {
    uint32_t set;
    int keep;
    tcpr_macaddr_t mask;
} en10mb_random_conf_t;

typedef struct {
    /* values to rewrite src/dst MAC addresses */
    tcpr_macaddr_t intf1_dmac;
    tcpr_macaddr_t intf1_smac;
    tcpr_macaddr_t intf2_dmac;
    tcpr_macaddr_t intf2_smac;

    en10mb_sub_conf_t subs;
    en10mb_random_conf_t random;

    /* we use the mask to say which are valid values */
    tcpedit_mac_mask mac_mask;

    /* 802.1q VLAN tag stuff */
    tcpedit_vlan vlan;

    /* user defined values, -1 means unset! */
    u_int16_t vlan_tag;
    u_int8_t vlan_pri;
    u_int8_t vlan_cfi;

    /* 802.1Q/802.1ad VLAN Q-in-Q - 0 means 802.1Q */
    u_int16_t vlan_proto;
} en10mb_config_t;

#ifdef __cplusplus
}
#endif
