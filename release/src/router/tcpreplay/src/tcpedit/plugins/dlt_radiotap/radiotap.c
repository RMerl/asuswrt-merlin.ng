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

#include "radiotap.h"
#include "../dlt_ieee80211/ieee80211.h"
#include "dlt_utils.h"
#include "tcpedit.h"
#include "tcpedit_stub.h"
#include <stdlib.h>
#include <string.h>

/* edit these variables to taste */
static char dlt_name[] = "radiotap";
_U_ static char dlt_prefix[] = "radiotap";
static uint16_t dlt_value = DLT_IEEE802_11_RADIO;

/*
 * The Radiotap header plugin utilizes the 802.11 plugin internally to do all the work
 * we just eat the radiotap header itself and pass the resulting buffer to the ieee80211
 * plugin.
 */

static u_char *dlt_radiotap_get_80211(tcpeditdlt_t *ctx, const u_char *packet, int pktlen, int radiolen);

/*
 * Function to register ourselves.  This function is always called, regardless
 * of what DLT types are being used, so it shouldn't be allocating extra buffers
 * or anything like that (use the dlt_radiotap_init() function below for that).
 * Tasks:
 * - Create a new plugin struct
 * - Fill out the provides/requires bit masks.  Note:  Only specify which fields are
 *   actually in the header.
 * - Add the plugin to the context's plugin chain
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_radiotap_register(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    assert(ctx);

    /* create  a new plugin structure */
    plugin = tcpedit_dlt_newplugin();

    /* we're a decoder only plugin, copy from ieee802.11 */
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
    plugin->plugin_init = dlt_radiotap_init;
    plugin->plugin_cleanup = dlt_radiotap_cleanup;
    plugin->plugin_parse_opts = dlt_radiotap_parse_opts;
    plugin->plugin_decode = dlt_radiotap_decode;
    plugin->plugin_encode = dlt_radiotap_encode;
    plugin->plugin_proto = dlt_radiotap_proto;
    plugin->plugin_l2addr_type = dlt_radiotap_l2addr_type;
    plugin->plugin_l2len = dlt_radiotap_80211_l2len;
    plugin->plugin_get_layer3 = dlt_radiotap_get_layer3;
    plugin->plugin_merge_layer3 = dlt_radiotap_merge_layer3;
    plugin->plugin_get_mac = dlt_radiotap_get_mac;

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
dlt_radiotap_init(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    assert(ctx);

    if ((plugin = tcpedit_dlt_getplugin(ctx, dlt_value)) == NULL) {
        tcpedit_seterr(ctx->tcpedit, "Unable to initialize unregistered plugin %s", dlt_name);
        return TCPEDIT_ERROR;
    }

    /* allocate memory for our decode extra data */
    if (ctx->decoded_extra_size > 0) {
        if (ctx->decoded_extra_size < sizeof(radiotap_extra_t)) {
            ctx->decoded_extra_size = sizeof(radiotap_extra_t);
            ctx->decoded_extra = safe_realloc(ctx->decoded_extra, ctx->decoded_extra_size);
        }
    } else {
        ctx->decoded_extra_size = sizeof(radiotap_extra_t);
        ctx->decoded_extra = safe_malloc(ctx->decoded_extra_size);
    }

    /* allocate memory for our config data */
    plugin->config_size = sizeof(radiotap_config_t);
    plugin->config = safe_malloc(plugin->config_size);

    return TCPEDIT_OK; /* success */
}

/*
 * Since this is used in a library, we should manually clean up after ourselves
 * Unless you allocated some memory in dlt_radiotap_init(), this is just an stub.
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_radiotap_cleanup(tcpeditdlt_t *ctx)
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
dlt_radiotap_parse_opts(tcpeditdlt_t *ctx)
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
dlt_radiotap_decode(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    int radiolen, rcode;
    u_char *data;
    assert(ctx);
    assert(packet);

    if (pktlen < (int)sizeof(radiotap_hdr_t))
        return TCPEDIT_ERROR;

    radiolen = dlt_radiotap_l2len(ctx, packet, pktlen);
    data = dlt_radiotap_get_80211(ctx, packet, pktlen, radiolen);
    if (!data)
        return TCPEDIT_ERROR;

    /* ieee80211 decoder fills out everything */
    rcode = dlt_ieee80211_decode(ctx, data, pktlen - radiolen);

    /* need to override the ieee802.11 l2 length result */
    ctx->l2len = dlt_radiotap_80211_l2len(ctx, packet, pktlen);
    return (ctx->l2len > 0) ? rcode : TCPEDIT_ERROR;
}

/*
 * Function to encode the layer 2 header back into the packet.
 * Returns: total packet len or TCPEDIT_ERROR
 */
int
dlt_radiotap_encode(tcpeditdlt_t *ctx, u_char *packet, _U_ int pktlen, _U_ tcpr_dir_t dir)
{
    assert(ctx);
    assert(packet);

    tcpedit_seterr(ctx->tcpedit, "%s", "DLT_IEEE802_11_RADIO plugin does not support packet encoding");
    return TCPEDIT_ERROR;
}

/*
 * Function returns the Layer 3 protocol type of the given packet, or TCPEDIT_ERROR on error
 * Make sure you return this in host byte order since all the comparisons will be
 * against the ETHERTYPE_* values which are oddly in host byte order.
 */
int
dlt_radiotap_proto(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    int radiolen;
    u_char *data;
    assert(ctx);
    assert(packet);

    if (pktlen < (int)sizeof(radiotap_hdr_t))
        return TCPEDIT_ERROR;

    radiolen = dlt_radiotap_l2len(ctx, packet, pktlen);
    if (radiolen < 0 || radiolen > pktlen)
        return TCPEDIT_ERROR;

    data = dlt_radiotap_get_80211(ctx, packet, pktlen, radiolen);
    return dlt_ieee80211_proto(ctx, data, pktlen - radiolen);
}

/*
 * Function returns a pointer to the layer 3 protocol header or NULL on error
 */
u_char *
dlt_radiotap_get_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen)
{
    int radiolen, l2len;
    u_char *data;

    assert(ctx);
    assert(packet);

    radiolen = dlt_radiotap_l2len(ctx, packet, pktlen);
    data = dlt_radiotap_get_80211(ctx, packet, pktlen, radiolen);
    l2len = dlt_ieee80211_l2len(ctx, data, pktlen - radiolen);
    return tcpedit_dlt_l3data_copy(ctx, data, pktlen - radiolen, l2len);
}

/*
 * function merges the packet (containing L2 and old L3) with the l3data buffer
 * containing the new l3 data.  Note, if L2 % 4 == 0, then they're pointing to the
 * same buffer, otherwise there was a memcpy involved on strictly aligned architectures
 * like SPARC
 */
u_char *
dlt_radiotap_merge_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen, u_char *ipv4_data, u_char *ipv6_data)
{
    int radiolen, l2len;
    u_char *data;

    assert(ctx);
    assert(packet);
    assert(ipv4_data || ipv6_data);

    radiolen = dlt_radiotap_l2len(ctx, packet, pktlen);
    data = dlt_radiotap_get_80211(ctx, packet, pktlen, radiolen);
    l2len = dlt_ieee80211_l2len(ctx, data, pktlen);
    return tcpedit_dlt_l3data_merge(ctx, data, pktlen - radiolen, ipv4_data ?: ipv6_data, l2len);
}

/*
 * return a static pointer to the source/destination MAC address
 * return NULL on error/address doesn't exist
 */
u_char *
dlt_radiotap_get_mac(tcpeditdlt_t *ctx, tcpeditdlt_mac_type_t mac, const u_char *packet, int pktlen)
{
    int radiolen;
    u_char *data;

    assert(ctx);
    assert(packet);

    radiolen = dlt_radiotap_l2len(ctx, packet, pktlen);

    if (pktlen < radiolen)
        return NULL;

    data = dlt_radiotap_get_80211(ctx, packet, pktlen, radiolen);
    return dlt_ieee80211_get_mac(ctx, mac, data, pktlen - radiolen);
}

/*
 * return the length of the L2 header of the current packet
 */
int
dlt_radiotap_l2len(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    uint16_t radiolen;
    assert(ctx);
    assert(packet);

    if (pktlen < 4)
        return -1;

    memcpy(&radiolen, &packet[2], 2);
    /* little endian to host */
    radiolen = ntohs(SWAPSHORT(radiolen));
    return (int)radiolen;
}

/*
 * return the length of the L2 header w/ 802.11 header of the current packet
 */
int
dlt_radiotap_80211_l2len(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    int radiolen, res;
    u_char *data;

    radiolen = dlt_radiotap_l2len(ctx, packet, pktlen);
    if (radiolen == -1)
        return TCPEDIT_ERROR;

    data = dlt_radiotap_get_80211(ctx, packet, pktlen, radiolen);
    res = dlt_ieee80211_l2len(ctx, data, pktlen - radiolen);
    if (res == -1)
        return TCPEDIT_ERROR;

    radiolen += res;
    return radiolen;
}

tcpeditdlt_l2addr_type_t
dlt_radiotap_l2addr_type(void)
{
    /* FIXME: return the tcpeditdlt_l2addr_type_t value that this DLT uses */
    return ETHERNET;
}

/*
 * returns a buffer to the 802.11 header in the packet.
 * This does an optimization of only doing a memcpy() once per packet
 * since we track which was the last packet # we copied.
 */
static u_char *
dlt_radiotap_get_80211(tcpeditdlt_t *ctx, const u_char *packet, int pktlen, int radiolen)
{
    radiotap_extra_t *extra;
    static COUNTER lastpacket = 0;

    if (ctx->decoded_extra_size < sizeof(*extra))
        return NULL;

    extra = (radiotap_extra_t *)(ctx->decoded_extra);
    if (pktlen >= radiolen && (size_t)(pktlen - radiolen) >= sizeof(extra->packet) &&
        lastpacket != ctx->tcpedit->runtime.packetnum) {
        memcpy(extra->packet, &packet[radiolen], pktlen - radiolen);
        lastpacket = ctx->tcpedit->runtime.packetnum;
    }
    return extra->packet;
}
