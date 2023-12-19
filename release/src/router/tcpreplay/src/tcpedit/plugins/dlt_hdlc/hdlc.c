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

#include "hdlc.h"
#include "dlt_utils.h"
#include "tcpedit.h"
#include "tcpedit_stub.h"
#include <stdlib.h>
#include <string.h>

static char dlt_name[] = "hdlc";
static char _U_ dlt_prefix[] = "hdlc";
static uint16_t dlt_value = DLT_C_HDLC;

/*
 * Function to register ourselves.  This function is always called, regardless
 * of what DLT types are being used, so it shouldn't be allocating extra buffers
 * or anything like that (use the dlt_hdlc_init() function below for that).
 * Tasks:
 * - Create a new plugin struct
 * - Fill out the provides/requires bit masks.  Note:  Only specify which fields are
 *   actually in the header.
 * - Add the plugin to the context's plugin chain
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_hdlc_register(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    assert(ctx);

    /* create  a new plugin structure */
    plugin = tcpedit_dlt_newplugin();

    /* FIXME: set what we provide & require  */
    plugin->provides += PLUGIN_MASK_PROTO;
    plugin->
        requires
    += PLUGIN_MASK_PROTO;

    /* what is our DLT value? */
    plugin->dlt = dlt_value;

    /* set the prefix name of our plugin.  This is also used as the prefix for our options */
    plugin->name = safe_strdup(dlt_prefix);

    /*
     * Point to our functions, note, you need a function for EVERY method.
     * Even if it is only an empty stub returning success.
     */
    plugin->plugin_init = dlt_hdlc_init;
    plugin->plugin_cleanup = dlt_hdlc_cleanup;
    plugin->plugin_parse_opts = dlt_hdlc_parse_opts;
    plugin->plugin_decode = dlt_hdlc_decode;
    plugin->plugin_encode = dlt_hdlc_encode;
    plugin->plugin_proto = dlt_hdlc_proto;
    plugin->plugin_l2addr_type = dlt_hdlc_l2addr_type;
    plugin->plugin_l2len = dlt_hdlc_l2len;
    plugin->plugin_get_layer3 = dlt_hdlc_get_layer3;
    plugin->plugin_merge_layer3 = dlt_hdlc_merge_layer3;
    plugin->plugin_get_mac = dlt_hdlc_get_mac;

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
dlt_hdlc_init(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    hdlc_config_t *config;
    assert(ctx);

    if ((plugin = tcpedit_dlt_getplugin(ctx, dlt_value)) == NULL) {
        tcpedit_seterr(ctx->tcpedit, "Unable to initialize unregistered plugin %s", dlt_name);
        return TCPEDIT_ERROR;
    }

    /* allocate memory for our deocde extra data */
    if (ctx->decoded_extra_size > 0) {
        if (ctx->decoded_extra_size < sizeof(hdlc_extra_t)) {
            ctx->decoded_extra_size = sizeof(hdlc_extra_t);
            ctx->decoded_extra = safe_realloc(ctx->decoded_extra, ctx->decoded_extra_size);
        }
    } else {
        ctx->decoded_extra_size = sizeof(hdlc_extra_t);
        ctx->decoded_extra = safe_malloc(ctx->decoded_extra_size);
    }

    /* allocate memory for our config data */
    plugin->config_size = sizeof(hdlc_config_t);
    plugin->config = safe_malloc(plugin->config_size);
    config = (hdlc_config_t *)plugin->config;

    /* default to unset */
    config->address = 65535;
    config->control = 65535;
    return TCPEDIT_OK; /* success */
}

/*
 * Since this is used in a library, we should manually clean up after ourselves
 * Unless you allocated some memory in dlt_hdlc_init(), this is just an stub.
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_hdlc_cleanup(tcpeditdlt_t *ctx)
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
dlt_hdlc_parse_opts(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    hdlc_config_t *config;
    assert(ctx);

    plugin = tcpedit_dlt_getplugin(ctx, dlt_value);
    if (!plugin)
        return TCPEDIT_ERROR;

    config = plugin->config;
    if (plugin->config_size < sizeof(*config))
        return TCPEDIT_ERROR;

    if (HAVE_OPT(HDLC_CONTROL)) {
        config->control = (uint16_t)OPT_VALUE_HDLC_CONTROL;
    }

    if (HAVE_OPT(HDLC_ADDRESS)) {
        config->address = (uint16_t)OPT_VALUE_HDLC_ADDRESS;
    }

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
dlt_hdlc_decode(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    cisco_hdlc_t *hdlc;
    hdlc_extra_t *extra;
    assert(ctx);
    assert(packet);

    if ((size_t)pktlen < sizeof(*hdlc))
        return TCPEDIT_ERROR;

    if (ctx->decoded_extra_size < sizeof(*extra))
        return TCPEDIT_ERROR;

    extra = (hdlc_extra_t *)ctx->decoded_extra;

    hdlc = (cisco_hdlc_t *)packet;

    ctx->proto = hdlc->protocol;
    ctx->l2len = 4;

    extra->address = hdlc->address;
    extra->control = hdlc->control;

    return TCPEDIT_OK; /* success */
}

/*
 * Function to encode the layer 2 header back into the packet.
 * Returns: total packet len or TCPEDIT_ERROR
 */
int
dlt_hdlc_encode(tcpeditdlt_t *ctx, u_char *packet, int pktlen, _U_ tcpr_dir_t dir)
{
    cisco_hdlc_t *hdlc;
    hdlc_config_t *config = NULL;
    hdlc_extra_t *extra = NULL;
    tcpeditdlt_plugin_t *plugin = NULL;
    int newpktlen;

    assert(ctx);
    assert(packet);

    if ((size_t)pktlen < sizeof(*hdlc))
        return TCPEDIT_ERROR;

    if (ctx->decoded_extra_size < sizeof(*extra))
        return TCPEDIT_ERROR;

    /* Make room for our new l2 header if old l2len != 4 */
    if (ctx->l2len > 4) {
        memmove(packet + 4, packet + ctx->l2len, pktlen - ctx->l2len);
    } else if (ctx->l2len < 4) {
        u_char *tmpbuff = safe_malloc(pktlen);
        memcpy(tmpbuff, packet, pktlen);
        memcpy(packet + 4, (tmpbuff + ctx->l2len), pktlen - ctx->l2len);
        safe_free(tmpbuff);
    }

    /* update the total packet length */
    newpktlen = pktlen + 4 - ctx->l2len;

    /*
     * HDLC doesn't support direction, since we have no real src/dst addresses
     * to deal with, so we just use the original packet data or option data
     */
    hdlc = (cisco_hdlc_t *)packet;
    plugin = tcpedit_dlt_getplugin(ctx, dlt_value);
    if (!plugin)
        return TCPEDIT_ERROR;

    config = plugin->config;
    if (plugin->config_size < sizeof(*config))
        return TCPEDIT_ERROR;

    extra = (hdlc_extra_t *)ctx->decoded_extra;
    if (ctx->decoded_extra_size < sizeof(*extra))
        return TCPEDIT_ERROR;

    /* set the address field */
    if (config->address < 65535) {
        hdlc->address = (uint8_t)config->address;
    } else if (extra->hdlc) {
        hdlc->address = extra->hdlc;
    } else {
        tcpedit_seterr(ctx->tcpedit, "%s", "Non-HDLC packet requires --hdlc-address");
        return TCPEDIT_ERROR;
    }

    /* set the control field */
    if (config->control < 65535) {
        hdlc->control = (uint8_t)config->control;
    } else if (extra->hdlc) {
        hdlc->control = extra->hdlc;
    } else {
        tcpedit_seterr(ctx->tcpedit, "%s", "Non-HDLC packet requires --hdlc-control");
        return TCPEDIT_ERROR;
    }

    /* copy over our protocol */
    hdlc->protocol = ctx->proto;

    return newpktlen; /* success */
}

/*
 * Function returns the Layer 3 protocol type of the given packet, or TCPEDIT_ERROR on error
 */
int
dlt_hdlc_proto(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    cisco_hdlc_t *hdlc;
    assert(ctx);
    assert(packet);

    if (pktlen < 4)
        return TCPEDIT_ERROR;

    hdlc = (cisco_hdlc_t *)packet;

    return hdlc->protocol;
}

/*
 * Function returns a pointer to the layer 3 protocol header or NULL on error
 */
u_char *
dlt_hdlc_get_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen)
{
    int l2len;
    assert(ctx);
    assert(packet);

    l2len = dlt_hdlc_l2len(ctx, packet, pktlen);
    if (l2len == -1 || pktlen < l2len)
        return NULL;

    return tcpedit_dlt_l3data_copy(ctx, packet, pktlen, l2len);
}

/*
 * function merges the packet (containing L2 and old L3) with the l3data buffer
 * containing the new l3 data.  Note, if L2 % 4 == 0, then they're pointing to the
 * same buffer, otherwise there was a memcpy involved on strictly aligned architectures
 * like SPARC
 */
u_char *
dlt_hdlc_merge_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen, u_char *ipv4_data, u_char *ipv6_data)
{
    int l2len;
    assert(ctx);
    assert(packet);
    assert(ipv4_data || ipv6_data);

    l2len = dlt_hdlc_l2len(ctx, packet, pktlen);
    if (l2len == -1 || pktlen < l2len)
        return NULL;

    return tcpedit_dlt_l3data_merge(ctx, packet, pktlen, ipv4_data ?: ipv6_data, l2len);
}

/*
 * return the length of the L2 header of the current packet
 */
int
dlt_hdlc_l2len(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    assert(ctx);
    assert(packet);

    if (pktlen < 4)
        return -1;

    /* HDLC is a static 4 bytes */
    return 4;
}

/*
 * return a static pointer to the source/destination MAC address
 * return NULL on error/address doesn't exist
 */
u_char *
dlt_hdlc_get_mac(tcpeditdlt_t *ctx, tcpeditdlt_mac_type_t mac, const u_char *packet, int pktlen)
{
    assert(ctx);
    assert(packet);

    if (pktlen < 14)
        return NULL;

    /* FIXME: return a ptr to the source or dest mac address. */
    switch (mac) {
    case SRC_MAC:
        return (NULL);
    case DST_MAC:
        memcpy(ctx->dstmac, packet, 2);
        return (ctx->dstmac);
    default:
        errx(-1, "Invalid tcpeditdlt_mac_type_t: %d", mac);
    }
}

tcpeditdlt_l2addr_type_t
dlt_hdlc_l2addr_type(void)
{
    return C_HDLC;
}
