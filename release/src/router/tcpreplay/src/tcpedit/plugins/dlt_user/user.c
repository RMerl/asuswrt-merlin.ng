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

#include "user.h"
#include "dlt_utils.h"
#include "tcpedit.h"
#include "tcpedit_stub.h"
#include <stdlib.h>
#include <string.h>

static char dlt_name[] = "user";
static char _U_ dlt_prefix[] = "user";
static uint16_t dlt_value = DLT_USER0;

/*
 * Function to register ourselves.  This function is always called, regardless
 * of what DLT types are being used, so it shouldn't be allocating extra buffers
 * or anything like that (use the dlt_user_init() function below for that).
 * Tasks:
 * - Create a new plugin struct
 * - Fill out the provides/requires bit masks.  Note:  Only specify which fields are
 *   actually in the header.
 * - Add the plugin to the context's plugin chain
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_user_register(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    assert(ctx);

    /* create  a new plugin structure */
    plugin = tcpedit_dlt_newplugin();

    /* FIXME: set what we provide & require */
    plugin->provides += PLUGIN_MASK_PROTO + PLUGIN_MASK_SRCADDR + PLUGIN_MASK_DSTADDR;
    plugin->
        requires
    += 0; // PLUGIN_MASK_PROTO + PLUGIN_MASK_SRCADDR + PLUGIN_MASK_DSTADDR;

    /* what is our DLT value? */
    plugin->dlt = dlt_value;

    /* set the prefix name of our plugin.  This is also used as the prefix for our options */
    plugin->name = safe_strdup(dlt_prefix);

    /*
     * Point to our functions, note, you need a function for EVERY method.
     * Even if it is only an empty stub returning success.
     */
    plugin->plugin_init = dlt_user_init;
    plugin->plugin_cleanup = dlt_user_cleanup;
    plugin->plugin_parse_opts = dlt_user_parse_opts;
    plugin->plugin_decode = dlt_user_decode;
    plugin->plugin_encode = dlt_user_encode;
    plugin->plugin_proto = dlt_user_proto;
    plugin->plugin_l2addr_type = dlt_user_l2addr_type;
    plugin->plugin_l2len = dlt_user_l2len;
    plugin->plugin_get_layer3 = dlt_user_get_layer3;
    plugin->plugin_merge_layer3 = dlt_user_merge_layer3;
    plugin->plugin_get_mac = dlt_user_get_mac;

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
dlt_user_init(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    user_config_t *config;
    assert(ctx);

    if ((plugin = tcpedit_dlt_getplugin(ctx, dlt_value)) == NULL) {
        tcpedit_seterr(ctx->tcpedit, "Unable to initialize unregistered plugin %s", dlt_name);
        return TCPEDIT_ERROR;
    }

    /* allocate memory for our decode extra data - plus some space for
     * other DLT decodes
     */
    if (ctx->decoded_extra_size > 0) {
        if (ctx->decoded_extra_size < USER_L2MAXLEN) {
            ctx->decoded_extra_size = USER_L2MAXLEN;
            ctx->decoded_extra = safe_realloc(ctx->decoded_extra, ctx->decoded_extra_size);
        }
    } else {
        ctx->decoded_extra_size = USER_L2MAXLEN;
        ctx->decoded_extra = safe_malloc(ctx->decoded_extra_size);
    }

    /* allocate memory for our config data */
    plugin->config_size = sizeof(user_config_t);
    plugin->config = safe_malloc(plugin->config_size);

    config = (user_config_t *)plugin->config;
    config->length = -1;

    /* do nothing */
    return TCPEDIT_OK; /* success */
}

/*
 * Since this is used in a library, we should manually clean up after ourselves
 * Unless you allocated some memory in dlt_user_init(), this is just an stub.
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_user_cleanup(tcpeditdlt_t *ctx)
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
dlt_user_parse_opts(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    user_config_t *config;
    assert(ctx);

    plugin = tcpedit_dlt_getplugin(ctx, dlt_value);
    if (!plugin)
        return TCPEDIT_ERROR;

    config = plugin->config;
    if (plugin->config_size < sizeof(*config))
        return TCPEDIT_ERROR;

    /*
     * --user-dlt will override the output DLT type, otherwise we'll use
     * the DLT of the decoder
     */
    if (HAVE_OPT(USER_DLT)) {
        config->dlt = OPT_VALUE_USER_DLT;
    } else {
        config->dlt = ctx->decoder->dlt;
    }

    /* --user-dlink */
    if (HAVE_OPT(USER_DLINK)) {
        int ct = STACKCT_OPT(USER_DLINK);
        char **list = (char **)STACKLST_OPT(USER_DLINK);
        int first = 1;

        do {
            char *p = *list++;
            if (first) {
                config->length = read_hexstring(p, config->l2server, USER_L2MAXLEN);
                memcpy(config->l2client, config->l2server, config->length);
            } else {
                if (config->length != read_hexstring(p, config->l2client, USER_L2MAXLEN)) {
                    tcpedit_seterr(ctx->tcpedit, "%s", "both --dlink's must contain the same number of bytes");
                    return TCPEDIT_ERROR;
                }
            }

            first = 0;
        } while (--ct > 0);
    }

    return TCPEDIT_OK; /* success */
}

/* you should never decode packets with this plugin! */
int
dlt_user_decode(tcpeditdlt_t *ctx, const u_char *packet, int _U_ pktlen)
{
    assert(ctx);
    assert(packet);

    tcpedit_seterr(ctx->tcpedit, "%s", "DLT_USER0 plugin does not support packet decoding");
    return TCPEDIT_ERROR;
}

/*
 * Function to encode the layer 2 header back into the packet.
 * Returns: total packet len or TCPEDIT_ERROR
 */
int
dlt_user_encode(tcpeditdlt_t *ctx, u_char *packet, int pktlen, tcpr_dir_t dir)
{
    user_config_t *config;
    tcpeditdlt_plugin_t *plugin;

    assert(ctx);
    assert(packet);

    if (pktlen == 0)
        return TCPEDIT_ERROR;

    plugin = tcpedit_dlt_getplugin(ctx, dlt_value);
    if (!plugin)
        return TCPEDIT_ERROR;

    config = plugin->config;
    if (plugin->config_size < sizeof(*config))
        return TCPEDIT_ERROR;

    /* Make room for our new l2 header if l2len != config->length */
    if (ctx->l2len > config->length) {
        memmove(packet + config->length, packet + ctx->l2len, pktlen - ctx->l2len);
    } else if (ctx->l2len < config->length) {
        u_char *tmpbuff = safe_malloc(pktlen);
        memcpy(tmpbuff, packet, pktlen);
        memcpy(packet + config->length, (tmpbuff + ctx->l2len), pktlen - ctx->l2len);
        safe_free(tmpbuff);
    }

    /* update the total packet length */
    pktlen += config->length - ctx->l2len;

    if (dir == TCPR_DIR_C2S) {
        memcpy(packet, config->l2client, config->length);
    } else if (dir == TCPR_DIR_S2C) {
        memcpy(packet, config->l2server, config->length);
    } else {
        tcpedit_seterr(ctx->tcpedit, "%s", "Encoders only support C2S or C2S!");
        return TCPEDIT_ERROR;
    }

    return pktlen; /* success */
}

/*
 * Function returns the Layer 3 protocol type of the given packet, or TCPEDIT_ERROR on error
 */
int
dlt_user_proto(tcpeditdlt_t *ctx, const u_char *packet, int _U_ pktlen)
{
    assert(ctx);
    assert(packet);

    /* calling this for DLT_USER0 is broken */
    tcpedit_seterr(ctx->tcpedit, "%s", "Nonsensical calling of dlt_user_proto()");
    return TCPEDIT_ERROR;
}

/*
 * Function returns a pointer to the layer 3 protocol header or NULL on error
 */
u_char *
dlt_user_get_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen)
{
    int l2len;
    assert(ctx);
    assert(packet);

    /* FIXME: Is there anything else we need to do?? */
    l2len = dlt_user_l2len(ctx, packet, pktlen);

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
dlt_user_merge_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen, u_char *ipv4_data, u_char *ipv6_data)
{
    int l2len;
    assert(ctx);
    assert(packet);
    assert(ipv4_data || ipv6_data);

    /* FIXME: Is there anything else we need to do?? */
    l2len = dlt_user_l2len(ctx, packet, pktlen);
    if (l2len == TCPEDIT_ERROR || pktlen < l2len)
        return NULL;

    return tcpedit_dlt_l3data_merge(ctx, packet, pktlen, ipv4_data ?: ipv6_data, l2len);
}

/*
 * return the length of the L2 header of the current packet
 */
int
dlt_user_l2len(tcpeditdlt_t *ctx, const u_char *packet, int _U_ pktlen)
{
    tcpeditdlt_plugin_t *plugin;
    user_config_t *config;
    assert(ctx);
    assert(packet);

    plugin = tcpedit_dlt_getplugin(ctx, dlt_value);
    if (!plugin)
        return TCPEDIT_ERROR;

    config = plugin->config;
    if (plugin->config_size < sizeof(*config))
        return TCPEDIT_ERROR;

    return config->length;
}

/*
 * return a static pointer to the source/destination MAC address
 * return NULL on error/address doesn't exist
 */
u_char *
dlt_user_get_mac(tcpeditdlt_t *ctx, _U_ tcpeditdlt_mac_type_t mac, const u_char *packet, int _U_ pktlen)
{
    assert(ctx);
    assert(packet);

    /* we don't know the format of USER DLT, hence always return NULL */
    return (NULL);
}

tcpeditdlt_l2addr_type_t
dlt_user_l2addr_type(void)
{
    return NONE;
}

/*
 * Need this special function for dlt_plugins.c:tcpedit_dlt_output_dlt()
 */

uint16_t
dlt_user_get_output_dlt(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    user_config_t *config;
    assert(ctx);

    plugin = tcpedit_dlt_getplugin(ctx, dlt_value);
    config = plugin->config;
    return config->dlt;
}
