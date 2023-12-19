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

#include "dlt_utils.h"
#include "tcpedit.h"
#include <string.h>

/* from dlt_plugins.c */
extern const uint32_t tcpeditdlt_bit_map[];
extern const char *tcpeditdlt_bit_info[];

/*
 * Call parse args on src & dst plugins
 */
int
tcpedit_dlt_parse_opts(tcpeditdlt_t *ctx)
{
    assert(ctx);

    if (ctx->decoder->plugin_parse_opts(ctx) != TCPEDIT_OK)
        return TCPEDIT_ERROR;

    if (ctx->decoder->dlt != ctx->encoder->dlt) {
        if (ctx->encoder->plugin_parse_opts(ctx) != TCPEDIT_OK)
            return TCPEDIT_ERROR;
    }

    return TCPEDIT_OK;
}

/*
 * find a given plugin struct in the context for a given DLT.  Returns NULL on failure
 */
tcpeditdlt_plugin_t *
tcpedit_dlt_getplugin(tcpeditdlt_t *ctx, int dlt)
{
    tcpeditdlt_plugin_t *ptr;

    assert(ctx);

    ptr = ctx->plugins;
    if (ptr == NULL)
        return NULL;

    while (ptr->dlt != dlt && ptr->next != NULL) {
        ptr = ptr->next;
    }

    if (ptr->dlt == dlt)
        return ptr;

    return NULL;
}

/*
 * find a given plugin struct in the context for a given DLT.  Returns NULL on failure
 */
tcpeditdlt_plugin_t *
tcpedit_dlt_getplugin_byname(tcpeditdlt_t *ctx, const char *name)
{
    tcpeditdlt_plugin_t *ptr;

    assert(ctx);
    assert(name);

    ptr = ctx->plugins;
    if (ptr == NULL)
        return NULL;

    while ((strcmp(ptr->name, name) != 0) && ptr->next != NULL) {
        ptr = ptr->next;
    }

    if (strcmp(ptr->name, name) == 0)
        return ptr;

    return NULL;
}

/*
 * Create a new plugin struct.  WILL NOT RETURN ON FAILURE! (out of memory is not recoverable)
 */
tcpeditdlt_plugin_t *
tcpedit_dlt_newplugin(void)
{
    tcpeditdlt_plugin_t *plugin;

    plugin = (tcpeditdlt_plugin_t *)safe_malloc(sizeof(tcpeditdlt_plugin_t));
    plugin->dlt = 0xffff; /* zero is a valid plugin, so use 0xffff */
    return plugin;
}

/*
 * Add a plugin to the plugin chain for the given context.  Return 0 on success,
 * -1 on failure
 */
int
tcpedit_dlt_addplugin(tcpeditdlt_t *ctx, tcpeditdlt_plugin_t *new)
{
    assert(ctx);
    assert(new);

    /* look for a dupe by DLT */
    if (tcpedit_dlt_getplugin(ctx, new->dlt) != NULL) {
        tcpedit_seterr(ctx->tcpedit, "Can only have one DLT plugin registered per-DLT: 0x%x", new->dlt);
        return TCPEDIT_ERROR;
    }

    /* dupe by name? */
    if (tcpedit_dlt_getplugin_byname(ctx, new->name) != NULL) {
        tcpedit_seterr(ctx->tcpedit, "Can only have one DLT plugin registered per-name: %s", new->name);
        return TCPEDIT_ERROR;
    }

    /*
     * check that the plugin is properly constructed, note that the encoder
     * and decoder are optional!
     */
    assert(new->dlt < 0xffff);
    assert(new->plugin_init);
    assert(new->plugin_cleanup);
    assert(new->plugin_parse_opts);
    assert(new->plugin_proto);
    assert(new->plugin_l2addr_type);
    assert(new->plugin_l2len);
    assert(new->plugin_get_layer3);
    assert(new->plugin_merge_layer3);

    /* add it to the end of the chain */
    if (ctx->plugins == NULL) {
        ctx->plugins = new;
    } else {
        tcpeditdlt_plugin_t *ptr = ctx->plugins;
        while (ptr->next != NULL)
            ptr = ptr->next;

        ptr->next = new;
    }

    /* we're done */
    return 0;
}

/*
 * validates that the decoder plugin provides all the fields that are required
 * by then encoding plugin. Returns TCPEDIT_OK | TCPEDIT_ERROR
 */
int
tcpedit_dlt_validate(tcpeditdlt_t *ctx)
{
    uint32_t bit;

    /* loops from 1 -> UINT32_MAX by powers of 2 */
    for (bit = 1; bit != 0; bit = bit << 2) {
        if (ctx->encoder->requires & bit && !(ctx->decoder->provides & bit)) {
            tcpedit_seterr(ctx->tcpedit, "%s", tcpeditdlt_bit_info[tcpeditdlt_bit_map[bit]]);
            return TCPEDIT_ERROR;
        }
    }

    dbgx(1, "Input linktype is %s", pcap_datalink_val_to_description(ctx->decoder->dlt));
    dbgx(1, "Output linktype is %s", pcap_datalink_val_to_description(ctx->encoder->dlt));

    return TCPEDIT_OK;
}

/*
 * Utility function to extract the Layer 3 header and beyond in a single buffer
 * Since some CPU's like UltraSPARC are strictly aligned, they really don't like
 * it when you jump to an offset which isn't on a word boundary (like ethernet)
 */
u_char *
tcpedit_dlt_l3data_copy(tcpeditdlt_t *ctx, u_char *packet, int pktlen, int l2len)
{
    u_char *ptr;
    assert(ctx);
    assert(packet);

    if (pktlen <= l2len)
        return NULL;

#ifdef FORCE_ALIGN
    /*
     * copy layer 3 and up to our temp packet buffer
     * for now on, we have to edit the packetbuff because
     * just before we send the packet, we copy the packetbuff
     * back onto the pkt.data + l2len buffer
     * we do all this work to prevent byte alignment issues
     */
    if (l2len % 4 == 0) {
        ptr = (&(packet)[l2len]);
    } else {
        ptr = ctx->l3buff;
        memcpy(ptr, (&(packet)[l2len]), pktlen - l2len);
    }
#else
    /*
     * on non-strict byte align systems, don't need to memcpy(),
     * just point to 14 bytes into the existing buffer
     */
    ptr = (&(packet)[l2len]);
#endif
    return ptr;
}

/*
 * reverse of tcpedit_dlt_l3data_copy
 */
u_char *
tcpedit_dlt_l3data_merge(tcpeditdlt_t *ctx, u_char *packet, int pktlen, const u_char *l3data, int l2len)
{
    assert(ctx);
    assert(packet);
    assert(pktlen >= 0);
    assert(l3data);
    assert(l2len >= 0);
#ifdef FORCE_ALIGN
    /*
     * put back the layer 3 and above back in the pkt.data buffer
     * we can't edit the packet at layer 3 or above beyond this point
     */
    if (l2len % 4 != 0)
        memcpy((&(packet)[l2len]), l3data, pktlen - l2len);
#endif
    return packet;
}

/**
 * When using subdecoders, we need to transfer the sub decoder state
 * to our state so that our primary encoder has it available.
 */
int
tcpedit_dlt_copy_decoder_state(tcpeditdlt_t *ctx, tcpeditdlt_t *subctx)
{
    assert(ctx);
    assert(subctx);

    memcpy(&ctx->srcaddr, &subctx->srcaddr, sizeof(ctx->srcaddr));
    memcpy(&ctx->dstaddr, &subctx->dstaddr, sizeof(ctx->dstaddr));
    ctx->proto = subctx->proto;
    memcpy(&ctx->srcmac, &subctx->srcmac, MAX_MAC_LEN);
    memcpy(&ctx->dstmac, &subctx->dstmac, MAX_MAC_LEN);

    /* just need to copy the ptr */
    ctx->decoded_extra = subctx->decoded_extra;
    ctx->decoded_extra_size = subctx->decoded_extra_size;

    /*
     * the first decoder should of already specified it's l2len, so we need to
     * add to it the l2len determined by the sub-plugin
     */
    ctx->l2len += subctx->l2len;

    return TCPEDIT_OK;
}
