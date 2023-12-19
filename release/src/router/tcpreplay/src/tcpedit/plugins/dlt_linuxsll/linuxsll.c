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

#include "linuxsll.h"
#include "../ethernet.h"
#include "dlt_utils.h"
#include "tcpedit.h"
#include "tcpedit_stub.h"
#include <stdlib.h>
#include <string.h>

static char dlt_name[] = "linuxsll";
static char _U_ dlt_prefix[] = "linuxsll";
static uint16_t dlt_value = DLT_LINUX_SLL;

/*
 * Function to register ourselves.  This function is always called, regardless
 * of what DLT types are being used, so it shouldn't be allocating extra buffers
 * or anything like that (use the dlt_linuxsll_init() function below for that).
 * Tasks:
 * - Create a new plugin struct
 * - Fill out the provides/requires bit masks.  Note:  Only specify which fields are
 *   actually in the header.
 * - Add the plugin to the context's plugin chain
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_linuxsll_register(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    assert(ctx);

    /* create  a new plugin structure */
    plugin = tcpedit_dlt_newplugin();

    /* FIXME: set what we provide & require */
    plugin->provides += PLUGIN_MASK_PROTO + PLUGIN_MASK_SRCADDR;
    plugin->
        requires
    += 0;

    /* what is our DLT value? */
    plugin->dlt = dlt_value;

    /* set the prefix name of our plugin.  This is also used as the prefix for our options */
    plugin->name = safe_strdup(dlt_prefix);

    /*
     * Point to our functions, note, you need a function for EVERY method.
     * Even if it is only an empty stub returning success.
     */
    plugin->plugin_init = dlt_linuxsll_init;
    plugin->plugin_cleanup = dlt_linuxsll_cleanup;
    plugin->plugin_parse_opts = dlt_linuxsll_parse_opts;
    plugin->plugin_decode = dlt_linuxsll_decode;
    plugin->plugin_encode = dlt_linuxsll_encode;
    plugin->plugin_proto = dlt_linuxsll_proto;
    plugin->plugin_l2addr_type = dlt_linuxsll_l2addr_type;
    plugin->plugin_l2len = dlt_linuxsll_l2len;
    plugin->plugin_get_layer3 = dlt_linuxsll_get_layer3;
    plugin->plugin_merge_layer3 = dlt_linuxsll_merge_layer3;
    plugin->plugin_get_mac = dlt_linuxsll_get_mac;

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
dlt_linuxsll_init(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    assert(ctx);

    if ((plugin = tcpedit_dlt_getplugin(ctx, dlt_value)) == NULL) {
        tcpedit_seterr(ctx->tcpedit, "Unable to initialize unregistered plugin %s", dlt_name);
        return TCPEDIT_ERROR;
    }

    /* allocate memory for our decode extra data */
    if (ctx->decoded_extra_size > 0) {
        if (ctx->decoded_extra_size < sizeof(linuxsll_extra_t)) {
            ctx->decoded_extra_size = sizeof(linuxsll_extra_t);
            ctx->decoded_extra = safe_realloc(ctx->decoded_extra, ctx->decoded_extra_size);
        }
    } else {
        ctx->decoded_extra_size = sizeof(linuxsll_extra_t);
        ctx->decoded_extra = safe_malloc(ctx->decoded_extra_size);
    }

    /* allocate memory for our config data */
    plugin->config_size = sizeof(linuxsll_config_t);
    plugin->config = safe_malloc(plugin->config_size);

    return TCPEDIT_OK; /* success */
}

/*
 * Since this is used in a library, we should manually clean up after ourselves
 * Unless you allocated some memory in dlt_linuxsll_init(), this is just an stub.
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_linuxsll_cleanup(tcpeditdlt_t *ctx)
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
dlt_linuxsll_parse_opts(tcpeditdlt_t *ctx)
{
    assert(ctx);

    /* nothing to parse */
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
dlt_linuxsll_decode(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    int type;
    linux_sll_header_t *linux_sll;
    assert(ctx);
    assert(packet);

    if (pktlen < (int)sizeof(linux_sll_header_t))
        return TCPEDIT_ERROR;

    linux_sll = (linux_sll_header_t *)packet;
    ctx->proto = linux_sll->proto;
    ctx->l2len = sizeof(linux_sll_header_t);

    type = ntohs(linux_sll->type);
    if (type == ARPHRD_ETHER || type == ARPHRD_LOOPBACK) { /* ethernet or loopback */
        memcpy(&(ctx->srcaddr), linux_sll->address, ETHER_ADDR_LEN);
    } else {
        tcpedit_seterr(ctx->tcpedit, "%s", "DLT_LINUX_SLL pcap's must contain only ethernet or loopback packets");
        return TCPEDIT_ERROR;
    }

    return TCPEDIT_OK; /* success */
}

/*
 * Function to encode the layer 2 header back into the packet.
 * Returns: total packet len or TCPEDIT_ERROR
 */
int
dlt_linuxsll_encode(tcpeditdlt_t *ctx, u_char *packet, _U_ int pktlen, _U_ tcpr_dir_t dir)
{
    assert(ctx);
    assert(packet);

    tcpedit_seterr(ctx->tcpedit, "%s", "DLT_LINUX_SLL plugin does not support packet encoding");
    return TCPEDIT_ERROR;
}

/*
 * Function returns the Layer 3 protocol type of the given packet, or TCPEDIT_ERROR on error
 */
int
dlt_linuxsll_proto(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    linux_sll_header_t *linux_sll;
    assert(ctx);
    assert(packet);

    if (pktlen < (int)sizeof(linux_sll_header_t))
        return TCPEDIT_ERROR;

    linux_sll = (linux_sll_header_t *)packet;

    return linux_sll->proto;
}

/*
 * Function returns a pointer to the layer 3 protocol header or NULL on error
 */
u_char *
dlt_linuxsll_get_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen)
{
    int l2len;
    assert(ctx);
    assert(packet);

    l2len = dlt_linuxsll_l2len(ctx, packet, pktlen);
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
dlt_linuxsll_merge_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen, u_char *ipv4_data, u_char *ipv6_data)
{
    int l2len;
    assert(ctx);
    assert(packet);
    assert(ipv4_data || ipv6_data);

    l2len = dlt_linuxsll_l2len(ctx, packet, pktlen);
    if (l2len == -1 || pktlen < l2len)
        return NULL;

    return tcpedit_dlt_l3data_merge(ctx, packet, pktlen, ipv4_data ?: ipv6_data, l2len);
}

/*
 * return the length of the L2 header of the current packet
 */
int
dlt_linuxsll_l2len(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    assert(ctx);
    assert(packet);

    if (pktlen < (int)sizeof(linux_sll_header_t))
        return -1;

    return sizeof(linux_sll_header_t);
}

/*
 * return a static pointer to the source/destination MAC address
 * return NULL on error/address doesn't exist
 */
u_char *
dlt_linuxsll_get_mac(tcpeditdlt_t *ctx, tcpeditdlt_mac_type_t mac, const u_char *packet, int pktlen)
{
    assert(ctx);
    assert(packet);

    if (pktlen < 14)
        return NULL;

    /* FIXME: return a ptr to the source or dest mac address. */
    switch (mac) {
    case SRC_MAC:
        memcpy(ctx->srcmac, &packet[6], 8); /* linuxssl defines the src mac field to be 8 bytes, not 6 */
        return (ctx->srcmac);
    case DST_MAC:
        return (NULL);
    default:
        errx(-1, "Invalid tcpeditdlt_mac_type_t: %d", mac);
    }
}

tcpeditdlt_l2addr_type_t
dlt_linuxsll_l2addr_type(void)
{
    /* we only support ethernet packets */
    return ETHERNET;
}
