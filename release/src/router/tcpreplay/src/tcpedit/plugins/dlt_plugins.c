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

#include "config.h"
#include "dlt_utils.h"
#include "plugins.h"
#include "tcpedit.h"
#include "tcpedit_stub.h"
#include <stdlib.h>

/**
 * Include plugin header files here...
 */
#include "dlt_en10mb/en10mb.h"
#include "dlt_hdlc/hdlc.h"
#include "dlt_ieee80211/ieee80211.h"
#include "dlt_jnpr_ether/jnpr_ether.h"
#include "dlt_linuxsll/linuxsll.h"
#include "dlt_loop/loop.h"
#include "dlt_null/null.h"
#include "dlt_pppserial/pppserial.h"
#include "dlt_radiotap/radiotap.h"
#include "dlt_raw/raw.h"
#include "dlt_user/user.h"

/**
 * Everyone writing a DLT plugin, must add their registration function
 * here.
 */
int
tcpedit_dlt_register(tcpeditdlt_t *ctx)
{
    int retcode = 0;
    assert(ctx);

    retcode += dlt_en10mb_register(ctx);
    retcode += dlt_hdlc_register(ctx);
    retcode += dlt_user_register(ctx);
    retcode += dlt_raw_register(ctx);
    retcode += dlt_null_register(ctx);
    retcode += dlt_loop_register(ctx);
    retcode += dlt_linuxsll_register(ctx);
    retcode += dlt_ieee80211_register(ctx);
    retcode += dlt_radiotap_register(ctx);
    retcode += dlt_jnpr_ether_register(ctx);
    retcode += dlt_pppserial_register(ctx);

    if (retcode < 0)
        return TCPEDIT_ERROR;

    return TCPEDIT_OK;
}

/********************************************************************
 * People writing DLT plugins should stop editing here!
 *
 * Well actually, that's true most of the time, but feel free to take
 * a look!
 ********************************************************************/

/*
 * mapping for bit_mask to bit_info.  If you're making changes here
 * then you almost certainly need to modify tcpeditdlt_t in dlt_plugins-int.h
 */
const uint32_t tcpeditdlt_bit_map[] = {PLUGIN_MASK_PROTO, PLUGIN_MASK_SRCADDR, PLUGIN_MASK_DSTADDR};

/* Meanings of the above map */
const char *tcpeditdlt_bit_info[] = {"Missing required Layer 3 protocol.",
                                     "Missing required Layer 2 source address.",
                                     "Missing required Layer 2 destination address."};

/*********************************************************************
 * Internal functions
 ********************************************************************/

/*********************************************************************
 * Public functions
 ********************************************************************/

/**
 * initialize our plugin library.  Pass the DLT of the source pcap handle.
 * Actions:
 * - Create new tcpeditdlt_t context
 * - Link tcpedit to new context
 * - Register plugins
 * - Select decoder plugin using srcdlt
 * - Select encoder plugin using destination name
 * - Initialize decoder/encoder plugins
 * - Parse options for encoder plugin
 * - Validate provides/reqiures + user options
 */
tcpeditdlt_t *
tcpedit_dlt_init(tcpedit_t *tcpedit, const int srcdlt)
{
    tcpeditdlt_t *ctx;
    int rcode;

    assert(tcpedit);
    assert(srcdlt >= 0);

    ctx = (tcpeditdlt_t *)safe_malloc(sizeof(tcpeditdlt_t));

    /* do we need a side buffer for L3 data? */
#ifdef FORCE_ALIGN
    ctx->l3buff = (u_char *)safe_malloc(MAXPACKET);
#endif

    /* copy our tcpedit context */
    ctx->tcpedit = tcpedit;

    /* register all our plugins */
    if (tcpedit_dlt_register(ctx) != TCPEDIT_OK) {
        tcpedit_dlt_cleanup(ctx);
        return NULL;
    }

    /* Choose decode plugin */
    if ((ctx->decoder = tcpedit_dlt_getplugin(ctx, srcdlt)) == NULL) {
        tcpedit_seterr(tcpedit, "No DLT plugin available for source DLT: 0x%x", srcdlt);
        tcpedit_dlt_cleanup(ctx);
        return NULL;
    }

    /* set our dlt type */
    ctx->dlt = srcdlt;

    /* set our address type */
    ctx->addr_type = ctx->decoder->plugin_l2addr_type();

    /* initialize decoder plugin */
    rcode = ctx->decoder->plugin_init(ctx);
    if (tcpedit_checkerror(ctx->tcpedit, rcode, NULL) != TCPEDIT_OK) {
        tcpedit_dlt_cleanup(ctx);
        return NULL;
    }

    /* we're OK */
    return ctx;
}

/**
 * \brief Call this to parse AutoOpts arguments for the DLT encoder plugin
 *
 * Was previously part of tcpedit_dlt_init(), but moved into it's own function
 * to allow a full programtic API.  Basically, if you're not using this function
 * you'll need to roll your own!
 * Returns 0 on success, -1 on error
 */
int
tcpedit_dlt_post_args(tcpedit_t *tcpedit)
{
    tcpeditdlt_t *ctx;
    const char *dst_dlt_name;

    assert(tcpedit);
    ctx = tcpedit->dlt_ctx;
    assert(ctx);

    /* Select the encoder plugin */
    dst_dlt_name = OPT_ARG(DLT) ? OPT_ARG(DLT) : ctx->decoder->name;
    if ((ctx->encoder = tcpedit_dlt_getplugin_byname(ctx, dst_dlt_name)) == NULL) {
        tcpedit_seterr(tcpedit, "No output DLT plugin available for: %s", dst_dlt_name);
        return TCPEDIT_ERROR;
    }

    /* Figure out if we're skipping braodcast & multicast */
    if (HAVE_OPT(SKIPL2BROADCAST))
        ctx->skip_broadcast = 1;

    /* init encoder plugin if it's not the decoder plugin */
    if (ctx->encoder->dlt != ctx->decoder->dlt) {
        if (ctx->encoder->plugin_init(ctx) != TCPEDIT_OK) {
            /* plugin should generate the error */
            return TCPEDIT_ERROR;
        }
    }

    /* parse the DLT specific options */
    if (tcpedit_dlt_parse_opts(ctx) != TCPEDIT_OK) {
        /* parser should generate the error */
        return TCPEDIT_ERROR;
    }

    /* we're OK */
    return tcpedit_dlt_post_init(ctx);
}

/**
 * This is the recommended method to edit a packet.  Returns (new) total packet length
 */
int
tcpedit_dlt_process(tcpeditdlt_t *ctx, u_char **packet, int pktlen, tcpr_dir_t direction)
{
    int rcode;

    assert(ctx);
    assert(packet);
    assert(direction == TCPR_DIR_C2S || direction == TCPR_DIR_S2C || direction == TCPR_DIR_NOSEND);

    /* nothing to do here */
    if (direction == TCPR_DIR_NOSEND)
        return pktlen;

    /* decode packet */
    if ((rcode = tcpedit_dlt_decode(ctx, *packet, pktlen)) == TCPEDIT_ERROR) {
        return TCPEDIT_ERROR;
    } else if (rcode == TCPEDIT_WARN) {
        warnx("Warning decoding packet: %s", tcpedit_getwarn(ctx->tcpedit));
    } else if (rcode == TCPEDIT_SOFT_ERROR) {
        return rcode; /* can't edit the packet */
    }

    /* encode packet */
    if ((rcode = tcpedit_dlt_encode(ctx, *packet, pktlen, direction)) == TCPEDIT_ERROR) {
        return TCPEDIT_ERROR;
    } else if (rcode == TCPEDIT_WARN) {
        warnx("Warning encoding packet: %s", tcpedit_getwarn(ctx->tcpedit));
    }

    return rcode;
}

/**
 * \brief Call after tcpedit_dlt_post_args() to allow plugins to do special things
 *
 * Useful for plugins to initialize sub-plugins and what not.
 * Returns the standard TCPEDIT_OK|ERROR|WARN
 */
int
tcpedit_dlt_post_init(tcpeditdlt_t *tcpedit)
{
    /* first init decoder */
    if (tcpedit->decoder->plugin_post_init != NULL)
        if (tcpedit->decoder->plugin_post_init(tcpedit) == TCPEDIT_ERROR)
            return TCPEDIT_ERROR;

    /* the encoder */
    if (tcpedit->encoder->plugin_post_init != NULL)
        if (tcpedit->encoder->plugin_post_init(tcpedit) == TCPEDIT_ERROR)
            return TCPEDIT_ERROR;

    return TCPEDIT_OK;
}

/**
 * What is the output DLT type???
 */
int
tcpedit_dlt_output_dlt(tcpeditdlt_t *ctx)
{
    uint16_t dlt;
    assert(ctx);

    /*
     * usually we just return the DLT value of the decoder, but for DLT_USER0
     * we return a user-specified value via --user-dlt
     */
    if (ctx->encoder->dlt == DLT_USER0) {
        dlt = dlt_user_get_output_dlt(ctx);
    } else {
        dlt = ctx->encoder->dlt;
    }
    return dlt;
}

/**
 * Get the layer 2 length of the packet using the DLT plugin currently in
 * place
 */
int
tcpedit_dlt_l2len(tcpeditdlt_t *ctx, int dlt, const u_char *packet, const int pktlen)
{
    tcpeditdlt_plugin_t *plugin;
    int res;

    assert(ctx);
    assert(dlt >= 0);
    assert(packet);

    if ((plugin = tcpedit_dlt_getplugin(ctx, dlt)) == NULL) {
        tcpedit_seterr(ctx->tcpedit, "Unable to find plugin for DLT 0x%04x", dlt);
        return -1;
    }

    res = plugin->plugin_l2len(ctx, packet, pktlen);
    if (res == -1) {
        tcpedit_seterr(ctx->tcpedit,
                       "Packet length %d is to short to contain a layer 2 header for DLT 0x%04x",
                       pktlen,
                       dlt);
        return -1;
    }

    return res;
}

/**
 * Get the L3 type.  Returns -1 on error.  Get error via tcpedit->geterr()
 */
int
tcpedit_dlt_proto(tcpeditdlt_t *ctx, int dlt, const u_char *packet, const int pktlen)
{
    tcpeditdlt_plugin_t *plugin;

    assert(ctx);
    assert(dlt >= 0);
    assert(packet);

    if ((plugin = tcpedit_dlt_getplugin(ctx, dlt)) == NULL) {
        tcpedit_seterr(ctx->tcpedit, "Unable to find plugin for DLT 0x%04x", dlt);
        return -1;
    }

    return plugin->plugin_proto(ctx, packet, pktlen);
}

/**
 * Get the L3 data.  Returns NULL on error.  Get error via tcpedit->geterr()
 */
u_char *
tcpedit_dlt_l3data(tcpeditdlt_t *ctx, int dlt, u_char *packet, const int pktlen)
{
    tcpeditdlt_plugin_t *plugin;
    u_char *res;

    assert(ctx);
    assert(dlt >= 0);
    assert(packet);

    if ((plugin = tcpedit_dlt_getplugin(ctx, dlt)) == NULL) {
        tcpedit_seterr(ctx->tcpedit, "Unable to find plugin for DLT 0x%04x", dlt);
        return NULL;
    }

    res = plugin->plugin_get_layer3(ctx, packet, pktlen);
    if (res == NULL)
        tcpedit_seterr(ctx->tcpedit,
                       "Packet length %d is to short to contain a layer 3 header for DLT 0x%04x",
                       pktlen,
                       dlt);

    return res;
}

/**
 * \brief Merge the Layer 3 data back onto the mainbuffer so it's immediately
 *   after the layer 2 header
 *
 * Since some L2 headers aren't strictly aligned, we need to "merge" the packet w/ L2 data
 * and the L3 buffer.  This is basically a NO-OP for things like vlan tagged ethernet (16 byte) header
 * or Cisco HDLC (4 byte header) but is critical for std ethernet (12 byte header)
 */
u_char *
tcpedit_dlt_merge_l3data(tcpeditdlt_t *ctx,
                         int dlt,
                         u_char *packet,
                         const int pktlen,
                         u_char *ipv4_data,
                         u_char *ipv6_data)
{
    tcpeditdlt_plugin_t *plugin;
    u_char *res;

    assert(ctx);
    assert(dlt >= 0);
    assert(packet);

    if (ipv4_data == NULL && ipv6_data == NULL)
        return packet;

    if ((plugin = tcpedit_dlt_getplugin(ctx, dlt)) == NULL) {
        tcpedit_seterr(ctx->tcpedit, "Unable to find plugin for DLT 0x%04x", dlt);
        return NULL;
    }

    res = plugin->plugin_merge_layer3(ctx, packet, pktlen, ipv4_data, ipv6_data);
    if (res == NULL)
        tcpedit_seterr(ctx->tcpedit, "Packet length %d is to short for layer 3 merge for DLT 0x%04x", pktlen, dlt);

    return res;
}

/**
 * Call the specific plugin decode() method
 */
int
tcpedit_dlt_decode(tcpeditdlt_t *ctx, const u_char *packet, const int pktlen)
{
    return ctx->decoder->plugin_decode(ctx, packet, pktlen);
}

/**
 * Call the specific plugin encode() method
 */
int
tcpedit_dlt_encode(tcpeditdlt_t *ctx, u_char *packet, int pktlen, tcpr_dir_t direction)
{
    return ctx->encoder->plugin_encode(ctx, packet, pktlen, direction);
}

/**
 * what is the source (decoder) DLT type?
 */
int
tcpedit_dlt_src(tcpeditdlt_t *ctx)
{
    assert(ctx);
    return ctx->decoder->dlt;
}

/**
 * What is the destination (encoder) DLT type
 */
int
tcpedit_dlt_dst(tcpeditdlt_t *ctx)
{
    assert(ctx);
    return ctx->encoder->dlt;
}

/**
 * cleanup after ourselves: destroys our context and all plugin data
 */
void
tcpedit_dlt_cleanup(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;

    assert(ctx);

    plugin = ctx->plugins;
    while (plugin != NULL) {
        plugin->plugin_cleanup(ctx);
        plugin = plugin->next;
    }

    plugin = ctx->plugins;
    while (plugin != NULL) {
        tcpeditdlt_plugin_t *plugin_next = plugin->next;
        safe_free(plugin);
        plugin = plugin_next;
    }

#ifdef FORCE_ALIGN
    safe_free(ctx->l3buff);
#endif

    if (ctx->decoded_extra != NULL) {
        safe_free(ctx->decoded_extra);
        ctx->decoded_extra = NULL;
        ctx->decoded_extra_size = 0;
    }

    safe_free(ctx);
}
