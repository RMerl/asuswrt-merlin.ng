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

#include "en10mb_api.h"
#include "defines.h"
#include "common.h"
#include "en10mb.h"
#include "tcpedit.h"
#include <stdlib.h>
#include <string.h>

/**
 * \brief Allows you to rewrite source & destination MAC addresses
 *
 * Pass the new MAC address in null terminated string format
 * "00:00:00:00:00:00\0" as well as the mac_mask value for which mac
 * address to rewrite.  You can call this function up to 4 times,
 * once for each mac_mask value.
 */
int
tcpedit_en10mb_set_mac(tcpedit_t *tcpedit, char *mac, tcpedit_mac_mask mask)
{
    u_char mac_addr[ETHER_ADDR_LEN];
    tcpeditdlt_t *ctx;
    tcpeditdlt_plugin_t *plugin;
    en10mb_config_t *config;

    assert(tcpedit);

    ctx = tcpedit->dlt_ctx;
    assert(ctx);
    plugin = ctx->decoder;
    assert(plugin);
    config = (en10mb_config_t *)plugin->config;

    assert(mac);

    mac2hex(mac, mac_addr, strlen(mac));

    switch (mask) {
    case TCPEDIT_MAC_MASK_DMAC1:
        config->mac_mask += TCPEDIT_MAC_MASK_DMAC1;
        memcpy(config->intf1_dmac, mac_addr, ETHER_ADDR_LEN);
        break;

    case TCPEDIT_MAC_MASK_DMAC2:
        config->mac_mask += TCPEDIT_MAC_MASK_DMAC2;
        memcpy(config->intf2_dmac, mac_addr, ETHER_ADDR_LEN);
        break;

    case TCPEDIT_MAC_MASK_SMAC1:
        config->mac_mask += TCPEDIT_MAC_MASK_SMAC1;
        memcpy(config->intf1_smac, mac_addr, ETHER_ADDR_LEN);
        break;

    case TCPEDIT_MAC_MASK_SMAC2:
        config->mac_mask += TCPEDIT_MAC_MASK_SMAC2;
        memcpy(config->intf2_smac, mac_addr, ETHER_ADDR_LEN);
        break;
    }

    switch (mask) {
    case TCPEDIT_MAC_MASK_DMAC1:
    case TCPEDIT_MAC_MASK_DMAC2:
        plugin->
            requires
        = plugin->
              requires
        & (0xffffffff ^ PLUGIN_MASK_DSTADDR);
        break;

    case TCPEDIT_MAC_MASK_SMAC1:
    case TCPEDIT_MAC_MASK_SMAC2:
        plugin->
            requires
        = plugin->
              requires
        & (0xffffffff ^ PLUGIN_MASK_SRCADDR);
        break;
    }

    return TCPEDIT_OK;
}

/**
 * Sets the 802.1q VLAN mode (add, delete, etc..)
 */
int
tcpedit_en10mb_set_vlan_mode(tcpedit_t *tcpedit, tcpedit_vlan vlan)
{
    tcpeditdlt_t *ctx;
    tcpeditdlt_plugin_t *plugin;
    en10mb_config_t *config;

    assert(tcpedit);

    ctx = tcpedit->dlt_ctx;
    assert(ctx);
    plugin = ctx->decoder;
    assert(plugin);
    config = (en10mb_config_t *)plugin->config;

    config->vlan = vlan;

    return TCPEDIT_OK;
}

/**
 * Sets the VLAN tag value in add or edit mode
 */
int
tcpedit_en10mb_set_vlan_tag(tcpedit_t *tcpedit, uint16_t tag)
{
    tcpeditdlt_t *ctx;
    tcpeditdlt_plugin_t *plugin;
    en10mb_config_t *config;

    assert(tcpedit);

    ctx = tcpedit->dlt_ctx;
    assert(ctx);
    plugin = ctx->decoder;
    assert(plugin);
    config = (en10mb_config_t *)plugin->config;

    config->vlan_tag = tag;

    return TCPEDIT_OK;
}

/**
 * Sets the VLAN priority field in add or edit mode
 */
int
tcpedit_en10mb_set_vlan_priority(tcpedit_t *tcpedit, uint8_t priority)
{
    tcpeditdlt_t *ctx;
    tcpeditdlt_plugin_t *plugin;
    en10mb_config_t *config;

    assert(tcpedit);

    ctx = tcpedit->dlt_ctx;
    assert(ctx);
    plugin = ctx->decoder;
    assert(plugin);
    config = (en10mb_config_t *)plugin->config;

    config->vlan_pri = priority;

    return TCPEDIT_OK;
}

/**
 * Sets the VLAN CFI field in add or edit mode
 */
int
tcpedit_en10mb_set_vlan_cfi(tcpedit_t *tcpedit, uint8_t cfi)
{
    tcpeditdlt_t *ctx;
    tcpeditdlt_plugin_t *plugin;
    en10mb_config_t *config;

    assert(tcpedit);

    ctx = tcpedit->dlt_ctx;
    assert(ctx);
    plugin = ctx->decoder;
    assert(plugin);
    config = (en10mb_config_t *)plugin->config;

    config->vlan_cfi = cfi;

    return TCPEDIT_OK;
}
