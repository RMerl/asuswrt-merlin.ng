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

#include "ieee80211.h"
#include "dlt_utils.h"
#include "ieee80211_hdr.h"
#include "tcpedit.h"
#include "tcpedit_stub.h"
#include <stdlib.h>
#include <string.h>

/*
 * Notes about the ieee80211 plugin:
 * 802.11 is a little different from most other L2 protocols:
 * - Not all frames are data frames (control, data, management) (data frame == L3 or higher included)
 * - Not all data frames have data (QoS frames are "data" frames, but have no L3 header)
 * - L2 header is 802.11 + an 802.2/802.2SNAP header
 */
static char dlt_name[] = "ieee80211";
_U_ static char dlt_prefix[] = "ieee802_11";
static uint16_t dlt_value = DLT_IEEE802_11;

/*
 * Function to register ourselves.  This function is always called, regardless
 * of what DLT types are being used, so it shouldn't be allocating extra buffers
 * or anything like that (use the dlt_ieee80211_init() function below for that).
 * Tasks:
 * - Create a new plugin struct
 * - Fill out the provides/requires bit masks.  Note:  Only specify which fields are
 *   actually in the header.
 * - Add the plugin to the context's plugin chain
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_ieee80211_register(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    assert(ctx);

    /* create  a new plugin structure */
    plugin = tcpedit_dlt_newplugin();

    /* we're a decoder only plugin */
    plugin->provides += PLUGIN_MASK_PROTO + PLUGIN_MASK_SRCADDR + PLUGIN_MASK_DSTADDR;
    plugin->
        requires
    += 0;

    /* what is our DLT value? */
    plugin->dlt = dlt_value;

    /* set the prefix name of our plugin.  This is also used as the prefix for our options */
    plugin->name = safe_strdup(dlt_name);

    /*
     * Point to our functions, note, you need a function for EVERY method.
     * Even if it is only an empty stub returning success.
     */
    plugin->plugin_init = dlt_ieee80211_init;
    plugin->plugin_cleanup = dlt_ieee80211_cleanup;
    plugin->plugin_parse_opts = dlt_ieee80211_parse_opts;
    plugin->plugin_decode = dlt_ieee80211_decode;
    plugin->plugin_encode = dlt_ieee80211_encode;
    plugin->plugin_proto = dlt_ieee80211_proto;
    plugin->plugin_l2addr_type = dlt_ieee80211_l2addr_type;
    plugin->plugin_l2len = dlt_ieee80211_l2len;
    plugin->plugin_get_layer3 = dlt_ieee80211_get_layer3;
    plugin->plugin_merge_layer3 = dlt_ieee80211_merge_layer3;
    plugin->plugin_get_mac = dlt_ieee80211_get_mac;

    /* add it to the available plugin list */
    return tcpedit_dlt_addplugin(ctx, plugin);
}

/*
 * Initializer function.  This function is called only once, if and only if
 * this plugin will be utilized.  Remember, if you need to keep track of any state,
 * store it in your plugin->config, not a global!
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_ieee80211_init(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    assert(ctx);

    if ((plugin = tcpedit_dlt_getplugin(ctx, dlt_value)) == NULL) {
        tcpedit_seterr(ctx->tcpedit, "Unable to initialize unregistered plugin %s", dlt_name);
        return TCPEDIT_ERROR;
    }

    /* allocate memory for our decode extra data */
    if (ctx->decoded_extra_size > 0) {
        if (ctx->decoded_extra_size < sizeof(ieee80211_extra_t)) {
            ctx->decoded_extra_size = sizeof(ieee80211_extra_t);
            ctx->decoded_extra = safe_realloc(ctx->decoded_extra, ctx->decoded_extra_size);
        }
    } else {
        ctx->decoded_extra_size = sizeof(ieee80211_extra_t);
        ctx->decoded_extra = safe_malloc(ctx->decoded_extra_size);
    }

    /* allocate memory for our config data */
    plugin->config_size = sizeof(ieee80211_config_t);
    plugin->config = safe_malloc(plugin->config_size);

    /* FIXME: set default config values here */

    return TCPEDIT_OK; /* success */
}

/*
 * Since this is used in a library, we should manually clean up after ourselves
 * Unless you allocated some memory in dlt_ieee80211_init(), this is just an stub.
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_ieee80211_cleanup(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    assert(ctx);

    if ((plugin = tcpedit_dlt_getplugin(ctx, dlt_value)) == NULL) {
        tcpedit_seterr(ctx->tcpedit, "Unable to cleanup unregistered plugin %s", dlt_name);
        return TCPEDIT_ERROR;
    }

    safe_free(plugin->name);
    plugin->name = NULL;
    safe_free(plugin->config);
    plugin->config = NULL;
    plugin->config_size = 0;

    return TCPEDIT_OK; /* success */
}

/*
 * This is where you should define all your AutoGen AutoOpts option parsing.
 * Any user specified option should have it's bit turned on in the 'provides'
 * bit mask.
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_ieee80211_parse_opts(tcpeditdlt_t *ctx)
{
    assert(ctx);

    /* we have none */

    return TCPEDIT_OK; /* success */
}

/*
 * Function to decode the layer 2 header in the packet.
 * You need to fill out:
 * - ctx->l2len
 * - ctx->srcaddr
 * - ctx->dstaddr
 * - ctx->proto
 * - ctx->decoded_extra
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_ieee80211_decode(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    int l2len;

    assert(ctx);
    assert(packet);

    l2len = dlt_ieee80211_l2len(ctx, packet, pktlen);
    if (l2len == -1 || pktlen < l2len)
        return TCPEDIT_ERROR;

    dbgx(3, "Decoding 802.11 packet " COUNTER_SPEC, ctx->tcpedit->runtime.packetnum);
    if (!ieee80211_is_data(ctx, packet, pktlen)) {
        tcpedit_seterr(ctx->tcpedit,
                       "Packet " COUNTER_SPEC " is not a normal 802.11 data frame",
                       ctx->tcpedit->runtime.packetnum);
        return TCPEDIT_SOFT_ERROR;
    }

    if (ieee80211_is_encrypted(ctx, packet, pktlen)) {
        tcpedit_seterr(ctx->tcpedit,
                       "Packet " COUNTER_SPEC " is encrypted.  Unable to decode frame.",
                       ctx->tcpedit->runtime.packetnum);
        return TCPEDIT_SOFT_ERROR;
    }

    ctx->l2len = l2len;
    memcpy(&(ctx->srcaddr), ieee80211_get_src((ieee80211_hdr_t *)packet), ETHER_ADDR_LEN);
    memcpy(&(ctx->dstaddr), ieee80211_get_dst((ieee80211_hdr_t *)packet), ETHER_ADDR_LEN);
    ctx->proto = dlt_ieee80211_proto(ctx, packet, pktlen);

    return TCPEDIT_OK; /* success */
}

/*
 * Function to encode the layer 2 header back into the packet.
 * Returns: total packet len or TCPEDIT_ERROR
 */
int
dlt_ieee80211_encode(tcpeditdlt_t *ctx, u_char *packet, _U_ int pktlen, _U_ tcpr_dir_t dir)
{
    assert(ctx);
    assert(packet);

    tcpedit_seterr(ctx->tcpedit, "%s", "DLT_IEEE802_11 plugin does not support packet encoding");
    return TCPEDIT_ERROR;
}

/*
 * Function returns the Layer 3 protocol type of the given packet, or TCPEDIT_ERROR on error
 */
int
dlt_ieee80211_proto(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    int l2len;
    int hdrlen = 0;
    uint16_t *frame_control, fc;
    struct tcpr_802_2snap_hdr *hdr;

    assert(ctx);
    assert(packet);

    l2len = dlt_ieee80211_l2len(ctx, packet, pktlen);
    if (l2len == -1 || pktlen < l2len)
        return TCPEDIT_ERROR;

    /* check 802.11 frame control field */
    frame_control = (uint16_t *)packet;
    fc = ntohs(*frame_control);

    /* Not all 802.11 frames have data */
    if ((fc & ieee80211_FC_TYPE_MASK) != ieee80211_FC_TYPE_DATA)
        return TCPEDIT_SOFT_ERROR;

    /* Some data frames are QoS and have no data
        if (((fc & ieee80211_FC_SUBTYPE_MASK) & ieee80211_FC_SUBTYPE_QOS) == ieee80211_FC_SUBTYPE_QOS)
        return TCPEDIT_SOFT_ERROR;
    */
    if ((fc & ieee80211_FC_SUBTYPE_QOS) == ieee80211_FC_SUBTYPE_QOS) {
        hdrlen += 2;
    }

    /* figure out the actual header length */
    if (ieee80211_USE_4(fc)) {
        hdrlen += sizeof(ieee80211_addr4_hdr_t);
    } else {
        hdrlen += sizeof(ieee80211_hdr_t);
    }

    hdr = (struct tcpr_802_2snap_hdr *)&packet[hdrlen];

    /* verify the header is 802.2SNAP (8 bytes) not 802.2 (3 bytes) */
    if (hdr->snap_dsap == 0xAA && hdr->snap_ssap == 0xAA)
        return hdr->snap_type;

    return TCPEDIT_SOFT_ERROR; /* 802.2 has no type field */
}

/*
 * Function returns a pointer to the layer 3 protocol header or NULL on error
 */
u_char *
dlt_ieee80211_get_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen)
{
    int l2len;
    assert(ctx);
    assert(packet);

    l2len = dlt_ieee80211_l2len(ctx, packet, pktlen);
    if (l2len == -1 || pktlen < l2len)
        return NULL;

    dbgx(1, "Getting data for packet " COUNTER_SPEC " from offset: %d", ctx->tcpedit->runtime.packetnum, l2len);

    return tcpedit_dlt_l3data_copy(ctx, packet, pktlen, l2len);
}

/*
 * function merges the packet (containing L2 and old L3) with the l3data buffer
 * containing the new l3 data.  Note, if L2 % 4 == 0, then they're pointing to the
 * same buffer, otherwise there was a memcpy involved on strictly aligned architectures
 * like SPARC
 */
u_char *
dlt_ieee80211_merge_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen, u_char *ipv4_data, u_char *ipv6_data)
{
    int l2len;
    assert(ctx);
    assert(packet);
    assert(ipv4_data || ipv6_data);

    l2len = dlt_ieee80211_l2len(ctx, packet, pktlen);
    if (l2len == -1 || pktlen < l2len)
        return NULL;

    return tcpedit_dlt_l3data_merge(ctx, packet, pktlen, ipv4_data ?: ipv6_data, l2len);
}

/*
 * return the length of the L2 header of the current packet
 * based on: http://www.tcpdump.org/lists/workers/2004/07/msg00121.html
 */
int
dlt_ieee80211_l2len(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    uint16_t *frame_control, fc;
    int hdrlen;

    assert(ctx);
    assert(packet);

    if (pktlen < (int)sizeof(uint16_t))
        return 0;

    dbgx(2, "packet = %p\t\tplen = %d", packet, pktlen);

    frame_control = (uint16_t *)packet;
    fc = ntohs(*frame_control);

    if (ieee80211_USE_4(fc)) {
        hdrlen = sizeof(ieee80211_addr4_hdr_t);
    } else {
        hdrlen = sizeof(ieee80211_hdr_t);
    }

    /* if Data/QoS, then L2 len is + 2 bytes */
    if ((fc & ieee80211_FC_SUBTYPE_QOS) == ieee80211_FC_SUBTYPE_QOS) {
        dbgx(2, "total header length (fc %04x) (802.11 + QoS data): %d", fc, hdrlen + 2);
        hdrlen += 2;
    }

    if (pktlen >= (hdrlen + (int)sizeof(struct tcpr_802_2snap_hdr))) {
        struct tcpr_802_2snap_hdr *hdr;

        hdr = (struct tcpr_802_2snap_hdr *)&packet[hdrlen];

        /* verify the header is 802.2SNAP (8 bytes) not 802.2 (3 bytes) */
        if (hdr->snap_dsap == 0xAA && hdr->snap_ssap == 0xAA) {
            hdrlen += (int)sizeof(struct tcpr_802_2snap_hdr);
            dbgx(2, "total header length (802.11 + 802.2SNAP): %d", hdrlen);
        } else {
            hdrlen += (int)sizeof(struct tcpr_802_2_hdr);
            dbgx(2, "total header length (802.11 + 802.2): %d (%02x/%02x)", hdrlen, hdr->snap_dsap, hdr->snap_ssap);
        }
    }

    if (pktlen < hdrlen)
        return 0;

    dbgx(2, "header length: %d", hdrlen);
    return hdrlen;
}

/*
 * return a static pointer to the source/destination MAC address
 * return NULL on error/address doesn't exist
 */
u_char *
dlt_ieee80211_get_mac(tcpeditdlt_t *ctx, tcpeditdlt_mac_type_t mac, const u_char *packet, int pktlen)
{
    assert(ctx);
    assert(packet);
    u_char *macaddr;

    if (pktlen < 14)
        return NULL;

    switch (mac) {
    case SRC_MAC:
        macaddr = ieee80211_get_src(packet);
        memcpy(ctx->srcmac, macaddr, ETHER_ADDR_LEN);
        return (ctx->srcmac);
    case DST_MAC:
        macaddr = ieee80211_get_dst(packet);
        memcpy(ctx->dstmac, macaddr, ETHER_ADDR_LEN);
        return (ctx->dstmac);
    default:
        errx(1, "Invalid tcpeditdlt_mac_type_t: %d", mac);
    }
}

tcpeditdlt_l2addr_type_t
dlt_ieee80211_l2addr_type(void)
{
    return ETHERNET;
}
