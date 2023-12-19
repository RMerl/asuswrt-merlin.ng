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

#include "defines.h"
#include "config.h"
#include "dlt_utils.h"
#include "portmap.h"
#include "tcpedit.h"
#include <sys/types.h>

/**
 * Set output DLT plugin by using it's DLT_<type>.  Note that the user plugin
 * is DLT_USER0.
 */
int
tcpedit_set_encoder_dltplugin_byid(tcpedit_t *tcpedit, int dlt)
{
    tcpeditdlt_plugin_t *plugin;
    tcpeditdlt_t *ctx;

    assert(tcpedit);

    ctx = tcpedit->dlt_ctx;
    assert(ctx);

    if (ctx->encoder) {
        tcpedit_seterr(tcpedit, "You have already selected a DLT encoder: %s", ctx->encoder->name);
        return TCPEDIT_ERROR;
    }

    plugin = tcpedit_dlt_getplugin(ctx, dlt);
    if (plugin == NULL) {
        tcpedit_seterr(tcpedit, "No output DLT plugin decoder with DLT type: 0x%04x", dlt);
        return TCPEDIT_ERROR;
    }

    ctx->encoder = plugin;

    /* init the encoder plugin if it's not the decoder plugin too */
    if (ctx->encoder->dlt != ctx->decoder->dlt) {
        if (ctx->encoder->plugin_init(ctx) != TCPEDIT_OK) {
            /* plugin should generate the error */
            return TCPEDIT_ERROR;
        }
    }

    return TCPEDIT_OK;
}

/**
 * same as tcpedit_set_encoder_plugin_byid() except we take the DLT_<name>
 * as a string to select it
 */
int
tcpedit_set_encoder_dltplugin_byname(tcpedit_t *tcpedit, const char *name)
{
    tcpeditdlt_plugin_t *plugin;
    tcpeditdlt_t *ctx;

    assert(tcpedit);

    ctx = tcpedit->dlt_ctx;
    assert(ctx);

    if (ctx->encoder) {
        tcpedit_seterr(tcpedit, "You have already selected a DLT encoder: %s", ctx->encoder->name);
        return TCPEDIT_ERROR;
    }

    plugin = tcpedit_dlt_getplugin_byname(ctx, name);
    if (plugin == NULL) {
        tcpedit_seterr(tcpedit, "No output DLT plugin available for: %s", name);
        return TCPEDIT_ERROR;
    }

    ctx->encoder = plugin;

    /* init the encoder plugin if it's not the decoder plugin too */
    if (ctx->encoder->dlt != ctx->decoder->dlt) {
        if (ctx->encoder->plugin_init(ctx) != TCPEDIT_OK) {
            /* plugin should generate the error */
            return TCPEDIT_ERROR;
        }
    }

    return TCPEDIT_OK;
}

/**
 * Set whether we should edit broadcast & multicast IP addresses
 */
int
tcpedit_set_skip_broadcast(tcpedit_t *tcpedit, bool value)
{
    assert(tcpedit);
    tcpedit->skip_broadcast = value;
    return TCPEDIT_OK;
}

/**
 * \brief force fixing L3 & L4 data by padding or truncating packets
 */
int
tcpedit_set_fixlen(tcpedit_t *tcpedit, tcpedit_fixlen value)
{
    assert(tcpedit);
    tcpedit->fixlen = value;
    return TCPEDIT_OK;
}

/**
 * \brief should we always recalculate L3 & L4 checksums?
 */
int
tcpedit_set_fixcsum(tcpedit_t *tcpedit, bool value)
{
    assert(tcpedit);
    tcpedit->fixcsum = value;
    return TCPEDIT_OK;
}

/**
 * \brief should we remove the EFCS from the frame?
 */
int
tcpedit_set_efcs(tcpedit_t *tcpedit, bool value)
{
    assert(tcpedit);
    tcpedit->efcs = value;
    return TCPEDIT_OK;
}

/**
 * \brief set the IPv4 TTL mode
 */
int
tcpedit_set_ttl_mode(tcpedit_t *tcpedit, tcpedit_ttl_mode value)
{
    assert(tcpedit);
    tcpedit->ttl_mode = value;
    return TCPEDIT_OK;
}

/**
 * \brief set the IPv4 ttl value
 */
int
tcpedit_set_ttl_value(tcpedit_t *tcpedit, uint8_t value)
{
    assert(tcpedit);
    tcpedit->ttl_value = value;
    return TCPEDIT_OK;
}

/**
 * \brief set the IPv4 TOS/DiffServ/ECN byte value
 */
int
tcpedit_set_tos(tcpedit_t *tcpedit, uint8_t value)
{
    assert(tcpedit);
    tcpedit->tos = value;
    return TCPEDIT_OK;
}

/**
 * \brief set the IPv6 Traffic Class byte value
 */
int
tcpedit_set_tclass(tcpedit_t *tcpedit, uint8_t value)
{
    assert(tcpedit);
    tcpedit->tclass = value;
    return TCPEDIT_OK;
}

/**
 * \brief set the IPv6 Flow Label 20bit value
 */
int
tcpedit_set_flowlabel(tcpedit_t *tcpedit, uint32_t value)
{
    assert(tcpedit);
    tcpedit->flowlabel = (int)value;
    return TCPEDIT_OK;
}

/**
 * Set the IPv4 IP address randomization seed
 */
int
tcpedit_set_seed(tcpedit_t *tcpedit)
{
    assert(tcpedit);

    tcpedit->rewrite_ip = true;
    tcpedit->seed = 1;
    tcpedit->seed = tcpr_random(&tcpedit->seed);

    return TCPEDIT_OK;
}

/**
 * Set the MTU of the frames
 */
int
tcpedit_set_mtu(tcpedit_t *tcpedit, int value)
{
    assert(tcpedit);
    tcpedit->mtu = value;
    return TCPEDIT_OK;
}

/**
 * Enable truncating packets to the MTU length
 */
int
tcpedit_set_mtu_truncate(tcpedit_t *tcpedit, bool value)
{
    assert(tcpedit);
    tcpedit->mtu_truncate = value;
    return TCPEDIT_OK;
}

/**
 * Set the maxpacket- currently not supported
 */
int
tcpedit_set_maxpacket(tcpedit_t *tcpedit, int value)
{
    assert(tcpedit);
    tcpedit->maxpacket = value;
    return TCPEDIT_OK;
}

/**
 * \brief Set the server to client (primary) CIDR map (Pseudo NAT)
 *
 * Set the server to client (primary) CIDR map using the given string
 * which is in the format of:
 * <match cidr>:<target cidr>,...
 * 192.168.0.0/16:10.77.0.0/16,172.16.0.0/12:10.1.0.0/24
 */
int
tcpedit_set_cidrmap_s2c(tcpedit_t *tcpedit, char *value)
{
    assert(tcpedit);

    tcpedit->rewrite_ip = true;
    if (!parse_cidr_map(&tcpedit->cidrmap1, value)) {
        tcpedit_seterr(tcpedit, "Unable to parse: %s", value);
        return TCPEDIT_ERROR;
    }
    return TCPEDIT_OK;
}

/**
 * \brief Set the client to server (secondary) CIDR map (Pseudo NAT)
 *
 * Set the client to server (secondary) CIDR map using the given string
 * which is in the format of:
 * <match cidr>:<target cidr>,...
 * 192.168.0.0/16:10.77.0.0/16,172.16.0.0/12:10.1.0.0/24
 */
int
tcpedit_set_cidrmap_c2s(tcpedit_t *tcpedit, char *value)
{
    assert(tcpedit);

    tcpedit->rewrite_ip = true;
    if (!parse_cidr_map(&tcpedit->cidrmap2, value)) {
        tcpedit_seterr(tcpedit, "Unable to parse: %s", value);
        return TCPEDIT_ERROR;
    }
    return TCPEDIT_OK;
}

/**
 * Rewrite the Source IP of any packet
 */
int
tcpedit_set_srcip_map(tcpedit_t *tcpedit, char *value)
{
    assert(tcpedit);

    tcpedit->rewrite_ip = true;
    if (!parse_cidr_map(&tcpedit->srcipmap, value)) {
        tcpedit_seterr(tcpedit, "Unable to parse source ip map: %s", value);
        return TCPEDIT_ERROR;
    }
    return TCPEDIT_OK;
}

/**
 * Rewrite the Destination IP of any packet
 */
int
tcpedit_set_dstip_map(tcpedit_t *tcpedit, char *value)
{
    assert(tcpedit);

    tcpedit->rewrite_ip = true;

    if (!parse_cidr_map(&tcpedit->dstipmap, value)) {
        tcpedit_seterr(tcpedit, "Unable to parse destination ip map: %s", value);
        return TCPEDIT_ERROR;
    }
    return TCPEDIT_OK;
}

/**
 * Rewrite TCP/UDP ports using the following format:
 * <src>:<dst>,...
 */
int
tcpedit_set_port_map(tcpedit_t *tcpedit, char *value)
{
    assert(tcpedit);

    if (!parse_portmap(&tcpedit->portmap, value)) {
        tcpedit_seterr(tcpedit, "Unable to parse portmap: %s", value);
        return TCPEDIT_ERROR;
    }
    return TCPEDIT_OK;
}
