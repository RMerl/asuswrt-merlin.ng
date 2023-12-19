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

#include "en10mb.h"
#include "../ethernet.h"
#include "dlt_utils.h"
#include "tcpedit.h"
#include "tcpedit_stub.h"
#include <stdlib.h>
#include <string.h>

static char dlt_name[] = "en10mb";
static char dlt_prefix[] = "enet";
static uint16_t dlt_value = DLT_EN10MB;

/*
 * Function to register ourselves.  This function is always called, regardless
 * of what DLT types are being used, so it shouldn't be allocating extra buffers
 * or anything like that (use the dlt_en10mb_init() function below for that).
 * Tasks:
 * - Create a new plugin struct
 * - Fill out the provides/requires bit masks.  Note:  Only specify which fields are
 *   actually in the header.
 * - Add the plugin to the context's plugin chain
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_en10mb_register(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    assert(ctx);

    /* create  a new plugin structure */
    plugin = tcpedit_dlt_newplugin();

    /* set what we provide & require */
    plugin->provides += PLUGIN_MASK_PROTO + PLUGIN_MASK_SRCADDR + PLUGIN_MASK_DSTADDR;
    plugin->requires += PLUGIN_MASK_PROTO + PLUGIN_MASK_SRCADDR + PLUGIN_MASK_DSTADDR;

    /* what is our dlt type? */
    plugin->dlt = dlt_value;

    /* set the prefix name of our plugin.  This is also used as the prefix for our options */
    plugin->name = safe_strdup(dlt_prefix);

    /*
     * Point to our functions, note, you need a function for EVERY method.
     * Even if it is only an empty stub returning success.
     */
    plugin->plugin_init = dlt_en10mb_init;
    plugin->plugin_cleanup = dlt_en10mb_cleanup;
    plugin->plugin_parse_opts = dlt_en10mb_parse_opts;
    plugin->plugin_decode = dlt_en10mb_decode;
    plugin->plugin_encode = dlt_en10mb_encode;
    plugin->plugin_proto = dlt_en10mb_proto;
    plugin->plugin_l2addr_type = dlt_en10mb_l2addr_type;
    plugin->plugin_l2len = dlt_en10mb_l2len;
    plugin->plugin_get_layer3 = dlt_en10mb_get_layer3;
    plugin->plugin_merge_layer3 = dlt_en10mb_merge_layer3;
    plugin->plugin_get_mac = dlt_en10mb_get_mac;

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
dlt_en10mb_init(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    en10mb_config_t *config;
    assert(ctx);

    /* vlan tags need an additional 4 bytes */
    if ((plugin = tcpedit_dlt_getplugin(ctx, dlt_value)) == NULL) {
        tcpedit_seterr(ctx->tcpedit, "%s", "Unable to initialize unregistered plugin en10mb");
        return TCPEDIT_ERROR;
    }

    if (ctx->decoded_extra_size > 0) {
        if (ctx->decoded_extra_size < sizeof(en10mb_extra_t)) {
            ctx->decoded_extra_size = sizeof(en10mb_extra_t);
            ctx->decoded_extra = safe_realloc(ctx->decoded_extra, ctx->decoded_extra_size);
        }
    } else {
        ctx->decoded_extra_size = sizeof(en10mb_extra_t);
        ctx->decoded_extra = safe_malloc(ctx->decoded_extra_size);
    }

    plugin->config_size = sizeof(en10mb_config_t);
    plugin->config = safe_malloc(plugin->config_size);
    config = (en10mb_config_t *)plugin->config;

    /* init vlan user values to -1 to indicate not set */
    config->vlan_tag = 65535;
    config->vlan_pri = 255;
    config->vlan_cfi = 255;

    /* VLAN protocol = 802.1q */
    config->vlan_proto = ETHERTYPE_VLAN;

    return TCPEDIT_OK; /* success */
}

/*
 * Since this is used in a library, we should manually clean up after ourselves
 * Unless you allocated some memory in dlt_en10mb_init(), this is just an stub.
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_en10mb_cleanup(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;

    assert(ctx);

    if ((plugin = tcpedit_dlt_getplugin(ctx, dlt_value)) == NULL) {
        tcpedit_seterr(ctx->tcpedit, "Unable to cleanup unregistered plugin %s", dlt_name);
        return TCPEDIT_ERROR;
    }

    safe_free(plugin->name);
    plugin->name = NULL;
    if (plugin->config != NULL) {
        en10mb_config_t *config = (en10mb_config_t *)plugin->config;
        safe_free(config->subs.entries);
        safe_free(plugin->config);
        plugin->config = NULL;
        plugin->config_size = 0;
    }

    return TCPEDIT_OK; /* success */
}

int
dlt_en10mb_parse_subsmac_entry(const char *raw, en10mb_sub_entry_t *entry)
{
    char *candidate = safe_strdup(raw);
    int parse_result = dualmac2hex(candidate, entry->target, entry->rewrite, SUBSMAC_ENTRY_LEN);

    free(candidate);

    return parse_result;
}

en10mb_sub_entry_t *
dlt_en10mb_realloc_merge(en10mb_sub_conf_t config, en10mb_sub_entry_t *new_entries, int entries_count)
{
    int i;

    config.entries = safe_realloc(config.entries, (config.count + entries_count) * sizeof(en10mb_sub_entry_t));

    for (i = 0; i < entries_count; i++) {
        config.entries[config.count + i] = new_entries[i];
    }

    return config.entries;
}

int
dlt_en10mb_parse_subsmac(tcpeditdlt_t *ctx, en10mb_config_t *config, const char *input)
{
    size_t input_len = strlen(input);
    size_t possible_entries_number = (input_len / (SUBSMAC_ENTRY_LEN + 1)) + 1;
    int entry;

    en10mb_sub_entry_t *entries = safe_malloc(possible_entries_number * sizeof(en10mb_sub_entry_t));

    for (entry = 0; entry < possible_entries_number; entry++) {
        const size_t read_offset = entry + entry * SUBSMAC_ENTRY_LEN;

        if (input_len - read_offset < SUBSMAC_ENTRY_LEN) {
            free(entries);
            tcpedit_seterr(ctx->tcpedit, "Unable to parse --enet-subsmac=%s", input);
            return TCPEDIT_ERROR;
        }

        switch (dlt_en10mb_parse_subsmac_entry(input + read_offset, &entries[entry])) {
        case 3:
            /* Both read; This is what we want */
            break;
        default:
            free(entries);
            tcpedit_seterr(ctx->tcpedit, "Unable to parse --enet-subsmac=%s", input);
            return TCPEDIT_ERROR;
        }
    }

    config->subs.entries = dlt_en10mb_realloc_merge(config->subs, entries, (int)possible_entries_number);
    config->subs.count += (int)possible_entries_number;

    free(entries);

    return TCPEDIT_OK;
}

/*
 * This is where you should define all your AutoGen AutoOpts option parsing.
 * Any user specified option should have it's bit turned on in the 'provides'
 * bit mask.
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_en10mb_parse_opts(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    en10mb_config_t *config;
    assert(ctx);

    plugin = tcpedit_dlt_getplugin(ctx, dlt_value);
    if (!plugin)
        return TCPEDIT_ERROR;

    config = (en10mb_config_t *)plugin->config;
    if (plugin->config_size < sizeof(*config))
        return TCPEDIT_ERROR;

    /* --subsmacs */
    if (HAVE_OPT(ENET_SUBSMAC)) {
        int i, count = STACKCT_OPT(ENET_SUBSMAC);
        char **list = (char **)STACKLST_OPT(ENET_SUBSMAC);
        for (i = 0; i < count; i++) {
            int parse_result = dlt_en10mb_parse_subsmac(ctx, config, list[i]);
            if (parse_result == TCPEDIT_ERROR) {
                return TCPEDIT_ERROR;
            }
        }
    }

    /* --mac-seed */
    if (HAVE_OPT(ENET_MAC_SEED)) {
        int i, j;

        config->random.set = OPT_VALUE_ENET_MAC_SEED;

        for (i = 0; i < 6; i++) {
            config->random.mask[i] = (u_char)tcpr_random(&config->random.set) % 256;
            /* only unique numbers */
            for (j = 0; j < i; j++) {
                if (config->random.mask[i] == config->random.mask[j]) {
                    i--;
                    break;
                }
            }
        }

        if (HAVE_OPT(ENET_MAC_SEED_KEEP_BYTES)) {
            config->random.keep = OPT_VALUE_ENET_MAC_SEED_KEEP_BYTES;
        }
    }

    /* --dmac */
    if (HAVE_OPT(ENET_DMAC)) {
        int macparse;
        macparse = dualmac2hex(OPT_ARG(ENET_DMAC),
                               config->intf1_dmac,
                               config->intf2_dmac,
                               (int)strlen(OPT_ARG(ENET_DMAC)));
        switch (macparse) {
        case 1:
            config->mac_mask += TCPEDIT_MAC_MASK_DMAC1;
            break;
        case 2:
            config->mac_mask += TCPEDIT_MAC_MASK_DMAC2;
            break;
        case 3:
            config->mac_mask += TCPEDIT_MAC_MASK_DMAC1;
            config->mac_mask += TCPEDIT_MAC_MASK_DMAC2;
            break;
        case 0:
            /* nothing to do */
            break;
        default:
            tcpedit_seterr(ctx->tcpedit, "Unable to parse --enet-dmac=%s", OPT_ARG(ENET_DMAC));
            return TCPEDIT_ERROR;
        }

        plugin->requires -= PLUGIN_MASK_DSTADDR;
    }

    /* --smac */
    if (HAVE_OPT(ENET_SMAC)) {
        int macparse;
        macparse = dualmac2hex(OPT_ARG(ENET_SMAC),
                               config->intf1_smac,
                               config->intf2_smac,
                               (int)strlen(OPT_ARG(ENET_SMAC)));
        switch (macparse) {
        case 1:
            config->mac_mask += TCPEDIT_MAC_MASK_SMAC1;
            break;
        case 2:
            config->mac_mask += TCPEDIT_MAC_MASK_SMAC2;
            break;
        case 3:
            config->mac_mask += TCPEDIT_MAC_MASK_SMAC1;
            config->mac_mask += TCPEDIT_MAC_MASK_SMAC2;
            break;
        case 0:
            /* nothing to do */
            break;
        default:
            tcpedit_seterr(ctx->tcpedit, "Unable to parse --enet-smac=%s", OPT_ARG(ENET_SMAC));
            return TCPEDIT_ERROR;
        }

        plugin->requires -= PLUGIN_MASK_SRCADDR;
    }

    /*
     * Validate 802.1q vlan args and populate tcpedit->vlan_record
     */
    if (HAVE_OPT(ENET_VLAN)) {
        if (strcmp(OPT_ARG(ENET_VLAN), "add") == 0) { /* add or change */
            config->vlan = TCPEDIT_VLAN_ADD;
        } else if (strcmp(OPT_ARG(ENET_VLAN), "del") == 0) {
            config->vlan = TCPEDIT_VLAN_DEL;
        } else {
            tcpedit_seterr(ctx->tcpedit, "Invalid --enet-vlan=%s", OPT_ARG(ENET_VLAN));
            return -1;
        }

        if (config->vlan != TCPEDIT_VLAN_OFF) {
            if (config->vlan == TCPEDIT_VLAN_ADD) {
                if (!HAVE_OPT(ENET_VLAN_TAG)) {
                    tcpedit_seterr(ctx->tcpedit,
                                   "%s",
                                   "Must specify a new 802.1 VLAN tag if vlan "
                                   "mode is add");
                    return TCPEDIT_ERROR;
                }

                /*
                 * fill out the 802.1q header
                 */
                config->vlan_tag = OPT_VALUE_ENET_VLAN_TAG;

                dbgx(1, "We will %s 802.1q headers", config->vlan == TCPEDIT_VLAN_DEL ? "delete" : "add/modify");

                if (HAVE_OPT(ENET_VLAN_PRI))
                    config->vlan_pri = OPT_VALUE_ENET_VLAN_PRI;

                if (HAVE_OPT(ENET_VLAN_CFI))
                    config->vlan_cfi = OPT_VALUE_ENET_VLAN_CFI;
            }

            if (HAVE_OPT(ENET_VLAN_PROTO)) {
                if (strcasecmp(OPT_ARG(ENET_VLAN_PROTO), "802.1q") == 0) {
                    config->vlan_proto = ETHERTYPE_VLAN;
                } else if (strcasecmp(OPT_ARG(ENET_VLAN_PROTO), "802.1ad") == 0) {
                    config->vlan_proto = ETHERTYPE_Q_IN_Q;
                } else {
                    tcpedit_seterr(ctx->tcpedit, "VLAN protocol \"%s\" is invalid", OPT_ARG(ENET_VLAN_PROTO));
                    return TCPEDIT_ERROR;
                }
            }
        }
    }

    return TCPEDIT_OK; /* success */
}

/*
 * Function to decode the layer 2 header in the packet
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_en10mb_decode(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    struct tcpr_ethernet_hdr *eth_hdr;
    uint32_t pkt_len = pktlen;
    en10mb_extra_t *extra;
    uint32_t vlan_offset;
    uint16_t protcol;
    uint32_t l2offset;
    uint32_t l2len;
    int res;

    assert(ctx);
    assert(packet);

    extra = (en10mb_extra_t *)ctx->decoded_extra;
    if (ctx->decoded_extra_size < sizeof(*extra)) {
        tcpedit_seterr(ctx->tcpedit, "Decode extra size too short! %d < %u", ctx->decoded_extra_size, sizeof(*extra));
        return TCPEDIT_ERROR;
    }

    res = get_l2len_protocol(packet, pkt_len, dlt_value, &protcol, &l2len, &l2offset, &vlan_offset);
    if (res == -1)
        return TCPEDIT_ERROR;

    if (pkt_len < TCPR_802_3_H + l2offset)
        return TCPEDIT_ERROR;

    eth_hdr = (struct tcpr_ethernet_hdr *)(packet + l2offset);
    protcol = ntohs(eth_hdr->ether_type);
    memcpy(&(ctx->dstaddr.ethernet), eth_hdr, ETHER_ADDR_LEN);
    memcpy(&(ctx->srcaddr.ethernet), &(eth_hdr->ether_shost), ETHER_ADDR_LEN);
    ctx->proto_vlan_tag = ntohs(eth_hdr->ether_type);

    if (vlan_offset != 0) {
        if (vlan_offset == l2offset + sizeof(*eth_hdr)) {
            vlan_hdr_t *vlan_hdr;
            vlan_hdr = (vlan_hdr_t *)(packet + vlan_offset);
            if (pkt_len < vlan_offset + sizeof(*vlan_hdr)) {
                tcpedit_seterr(ctx->tcpedit,
                               "Frame is too short for VLAN headers! %d < %d",
                               pktlen,
                               vlan_offset + sizeof(*vlan_hdr));
                return TCPEDIT_ERROR;
            }

            extra->vlan = 1;
            extra->vlan_offset = vlan_offset;
            extra->vlan_proto = ntohs(vlan_hdr->vlan_tpid);
            extra->vlan_tag = htons(vlan_hdr->vlan_tci) & TCPR_802_1Q_VIDMASK;
            extra->vlan_pri = htons(vlan_hdr->vlan_tci) & TCPR_802_1Q_PRIMASK;
            extra->vlan_cfi = htons(vlan_hdr->vlan_tci) & TCPR_802_1Q_CFIMASK;

        } else {
            /* VLAN headers cannot be after MPLS. There are other protocols such
             * Cisco Metatdata that could be before VLAN, but we don't support
             * them yet.
             */
            return TCPEDIT_ERROR;
        }
    } else {
        extra->vlan = 0;
        extra->vlan_offset = l2offset + sizeof(*eth_hdr);
        extra->vlan_proto = protcol;
    }

    ctx->proto = ntohs(protcol);
    ctx->l2offset = (int)l2offset;
    ctx->l2len = (int)l2len;

    return TCPEDIT_OK; /* success */
}

/*
 * Function to encode the layer 2 header back into the packet.
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_en10mb_encode(tcpeditdlt_t *ctx, u_char *packet, int pktlen, tcpr_dir_t dir)
{
    struct tcpr_802_1q_hdr *vlan_hdr;
    struct tcpr_ethernet_hdr *eth;
    tcpeditdlt_plugin_t *plugin;
    en10mb_config_t *config;
    en10mb_extra_t *extra;
    uint32_t newl2len = 0;
    uint32_t oldl2len = 0;

    assert(ctx);
    assert(packet);

    if (pktlen < TCPR_802_3_H) {
        tcpedit_seterr(ctx->tcpedit,
                       "Unable to process packet #" COUNTER_SPEC " since it is less then 14 bytes.",
                       ctx->tcpedit->runtime.packetnum);
        return TCPEDIT_ERROR;
    }

    plugin = tcpedit_dlt_getplugin(ctx, dlt_value);
    if (!plugin)
        return TCPEDIT_ERROR;

    config = plugin->config;
    if (plugin->config_size < sizeof(*config))
        return TCPEDIT_ERROR;

    extra = (en10mb_extra_t *)ctx->decoded_extra;
    if (ctx->decoded_extra_size < sizeof(*extra))
        return TCPEDIT_ERROR;

    /* figure out the new layer2 length, first for the case: ethernet -> ethernet? */
    if (ctx->decoder->dlt == dlt_value) {
        switch (config->vlan) {
        case TCPEDIT_VLAN_ADD:
            oldl2len = extra->vlan_offset;
            newl2len = extra->vlan_offset + sizeof(vlan_hdr_t);
            break;
        case TCPEDIT_VLAN_DEL:
            if (extra->vlan) {
                oldl2len = extra->vlan_offset + sizeof(vlan_hdr_t);
                newl2len = extra->vlan_offset;
            }
            break;
        case TCPEDIT_VLAN_OFF:
            if (extra->vlan) {
                oldl2len = extra->vlan_offset;
                newl2len = extra->vlan_offset;
            }
            break;
        }
    }

    /* newl2len for some other DLT -> ethernet */
    else if (config->vlan == TCPEDIT_VLAN_ADD) {
        /* if add a vlan then 18, */
        newl2len = TCPR_802_1Q_H;
    }

    if (pktlen < newl2len || pktlen + newl2len - ctx->l2len > MAXPACKET) {
        tcpedit_seterr(ctx->tcpedit,
                       "Unable to process packet #" COUNTER_SPEC " since its new length is %d bytes.",
                       ctx->tcpedit->runtime.packetnum,
                       newl2len);
        return TCPEDIT_ERROR;
    }

    if (pktlen < ctx->l2len) {
        tcpedit_seterr(ctx->tcpedit,
                       "Unable to process packet #" COUNTER_SPEC " since its new length less then %d L2 bytes.",
                       ctx->tcpedit->runtime.packetnum,
                       ctx->l2len);
        return TCPEDIT_ERROR;
    }

    /* Make space for our new L2 header */
    if (newl2len > 0 && newl2len != oldl2len) {
        if (pktlen + (newl2len - oldl2len) > MAXPACKET) {
            tcpedit_seterr(ctx->tcpedit,
                           "New frame too big, new length %d exceeds %d",
                           pktlen + (newl2len - ctx->l2len),
                           MAXPACKET);
            return TCPEDIT_ERROR;
        }

        memmove(packet + newl2len, packet + oldl2len, pktlen - oldl2len);
    }

    /* update the total packet length */
    pktlen += (int)(newl2len - oldl2len);
    ctx->l2len += (int)(newl2len - oldl2len);

    /* set the src & dst address as the first 12 bytes */
    eth = (struct tcpr_ethernet_hdr *)(packet + ctx->l2offset);

    if (dir == TCPR_DIR_C2S) {
        /* copy user supplied SRC MAC if provided or from original packet */
        if (config->mac_mask & TCPEDIT_MAC_MASK_SMAC1) {
            if ((ctx->addr_type == ETHERNET &&
                 ((ctx->skip_broadcast && is_unicast_ethernet(ctx, ctx->srcaddr.ethernet)) || !ctx->skip_broadcast)) ||
                ctx->addr_type != ETHERNET) {
                memcpy(eth->ether_shost, config->intf1_smac, ETHER_ADDR_LEN);
            } else {
                memcpy(eth->ether_shost, ctx->srcaddr.ethernet, ETHER_ADDR_LEN);
            }
        } else if (ctx->addr_type == ETHERNET) {
            extra->src_modified = memcmp(eth->ether_shost, ctx->srcaddr.ethernet, ETHER_ADDR_LEN);
            memcpy(eth->ether_shost, ctx->srcaddr.ethernet, ETHER_ADDR_LEN);
        } else {
            tcpedit_seterr(ctx->tcpedit, "%s", "Please provide a source address");
            return TCPEDIT_ERROR;
        }

        /* copy user supplied DMAC MAC if provided or from original packet */
        if (config->mac_mask & TCPEDIT_MAC_MASK_DMAC1) {
            if ((ctx->addr_type == ETHERNET &&
                 ((ctx->skip_broadcast && is_unicast_ethernet(ctx, ctx->dstaddr.ethernet)) || !ctx->skip_broadcast)) ||
                ctx->addr_type != ETHERNET) {
                memcpy(eth->ether_dhost, config->intf1_dmac, ETHER_ADDR_LEN);
            } else {
                memcpy(eth->ether_dhost, ctx->dstaddr.ethernet, ETHER_ADDR_LEN);
            }
        } else if (ctx->addr_type == ETHERNET) {
            extra->dst_modified = memcmp(eth->ether_dhost, ctx->dstaddr.ethernet, ETHER_ADDR_LEN);
            memcpy(eth->ether_dhost, ctx->dstaddr.ethernet, ETHER_ADDR_LEN);
        } else {
            tcpedit_seterr(ctx->tcpedit, "%s", "Please provide a destination address");
            return TCPEDIT_ERROR;
        }

    } else if (dir == TCPR_DIR_S2C) {
        /* copy user supplied SRC MAC if provided or from original packet */
        if (config->mac_mask & TCPEDIT_MAC_MASK_SMAC2) {
            if ((ctx->addr_type == ETHERNET &&
                 ((ctx->skip_broadcast && is_unicast_ethernet(ctx, ctx->srcaddr.ethernet)) || !ctx->skip_broadcast)) ||
                ctx->addr_type != ETHERNET) {
                memcpy(eth->ether_shost, config->intf2_smac, ETHER_ADDR_LEN);
            } else {
                memcpy(eth->ether_shost, ctx->srcaddr.ethernet, ETHER_ADDR_LEN);
            }
        } else if (ctx->addr_type == ETHERNET) {
            memcpy(eth->ether_shost, ctx->srcaddr.ethernet, ETHER_ADDR_LEN);
        } else {
            tcpedit_seterr(ctx->tcpedit, "%s", "Please provide a source address");
            return TCPEDIT_ERROR;
        }

        /* copy user supplied DMAC MAC if provided or from original packet */
        if (config->mac_mask & TCPEDIT_MAC_MASK_DMAC2) {
            if ((ctx->addr_type == ETHERNET &&
                 ((ctx->skip_broadcast && is_unicast_ethernet(ctx, ctx->dstaddr.ethernet)) || !ctx->skip_broadcast)) ||
                ctx->addr_type != ETHERNET) {
                memcpy(eth->ether_dhost, config->intf2_dmac, ETHER_ADDR_LEN);
            } else {
                memcpy(eth->ether_dhost, ctx->dstaddr.ethernet, ETHER_ADDR_LEN);
            }
        } else if (ctx->addr_type == ETHERNET) {
            memcpy(eth->ether_dhost, ctx->dstaddr.ethernet, ETHER_ADDR_LEN);
        } else {
            tcpedit_seterr(ctx->tcpedit, "%s", "Please provide a destination address");
            return TCPEDIT_ERROR;
        }

    } else {
        tcpedit_seterr(ctx->tcpedit, "%s", "Encoders only support C2S or C2S!");
        return TCPEDIT_ERROR;
    }

    if (config->subs.entries) {
        int entry = 0;
        for (entry = 0; entry < config->subs.count; entry++) {
            en10mb_sub_entry_t *current = &config->subs.entries[entry];

            if (!memcmp(eth->ether_dhost, current->target, ETHER_ADDR_LEN)) {
                memcpy(eth->ether_dhost, current->rewrite, ETHER_ADDR_LEN);
            }

            if (!memcmp(eth->ether_shost, current->target, ETHER_ADDR_LEN)) {
                memcpy(eth->ether_shost, current->rewrite, ETHER_ADDR_LEN);
            }
        }
    }

    if (config->random.set) {
        int unicast_src = is_unicast_ethernet(ctx, eth->ether_shost);
        int unicast_dst = is_unicast_ethernet(ctx, eth->ether_dhost);

        int i = config->random.keep;
        for (; i < ETHER_ADDR_LEN; i++) {
            eth->ether_shost[i] = MAC_MASK_APPLY(eth->ether_shost[i], config->random.mask[i], unicast_src);
            eth->ether_dhost[i] = MAC_MASK_APPLY(eth->ether_dhost[i], config->random.mask[i], unicast_dst);
        }

        /* avoid making unicast packets multicast */
        if (!config->random.keep) {
            eth->ether_shost[0] &= ~(0x01 * unicast_src);
            eth->ether_dhost[0] &= ~(0x01 * unicast_dst);
        }
    }

    if (config->vlan == TCPEDIT_VLAN_ADD || (config->vlan == TCPEDIT_VLAN_OFF && extra->vlan)) {
        vlan_hdr = (struct tcpr_802_1q_hdr *)(packet + extra->vlan_offset);
        if (config->vlan == TCPEDIT_VLAN_ADD) {
            struct tcpr_ethernet_hdr *eth_hdr;
            eth_hdr = (struct tcpr_ethernet_hdr *)(packet + ctx->l2offset);
            eth_hdr->ether_type = htons(config->vlan_proto);
            vlan_hdr->vlan_tpid = htons(ctx->proto_vlan_tag);
        }

        /* are we changing VLAN info? */
        if (config->vlan_tag < 65535) {
            vlan_hdr->vlan_tci = htons((uint16_t)config->vlan_tag & TCPR_802_1Q_VIDMASK);
        } else if (extra->vlan) {
            vlan_hdr->vlan_tci = htons(extra->vlan_tag);
        } else {
            tcpedit_seterr(ctx->tcpedit, "%s", "Non-VLAN tagged packet requires --enet-vlan-tag");
            return TCPEDIT_ERROR;
        }

        if (config->vlan_pri < 255) {
            vlan_hdr->vlan_tci += htons((uint16_t)config->vlan_pri << 13);
        } else if (extra->vlan) {
            vlan_hdr->vlan_tci += htons(extra->vlan_pri);
        } else {
            tcpedit_seterr(ctx->tcpedit, "%s", "Non-VLAN tagged packet requires --enet-vlan-pri");
            return TCPEDIT_ERROR;
        }

        if (config->vlan_cfi < 255) {
            vlan_hdr->vlan_tci += htons((uint16_t)config->vlan_cfi << 12);
        } else if (extra->vlan) {
            vlan_hdr->vlan_tci += htons(extra->vlan_cfi);
        } else {
            tcpedit_seterr(ctx->tcpedit, "%s", "Non-VLAN tagged packet requires --enet-vlan-cfi");
            return TCPEDIT_ERROR;
        }

    } else if (config->vlan == TCPEDIT_VLAN_DEL && newl2len > 0) {
        /* all we need for 802.3 is the proto */
        eth->ether_type = htons(extra->vlan_proto);
    }

    return pktlen;
}

/*
 * Function returns the Layer 3 protocol type of the given packet, or TCPEDIT_ERROR on error
 */
int
dlt_en10mb_proto(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    uint32_t _U_ vlan_offset;
    uint16_t ether_type;
    uint32_t _U_ l2offset;
    uint32_t _U_ l2len;
    int res;

    assert(ctx);
    assert(packet);
    if (pktlen < (int)sizeof(struct tcpr_ethernet_hdr)) {
        tcpedit_seterr(ctx->tcpedit, "Ethernet packet length too short: %d", pktlen);
        return TCPEDIT_ERROR;
    }

    res = get_l2len_protocol(packet, pktlen, dlt_value, &ether_type, &l2len, &l2offset, &vlan_offset);
    if (res == -1)
        return TCPEDIT_ERROR;

    return htons(ether_type);
}

/*
 * Function returns a pointer to the layer 3 protocol header or NULL on error
 */
u_char *
dlt_en10mb_get_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen)
{
    int l2len;
    assert(ctx);
    assert(packet);

    l2len = dlt_en10mb_l2len(ctx, packet, pktlen);
    if (l2len == -1 || pktlen < l2len)
        return NULL;

    return tcpedit_dlt_l3data_copy(ctx, packet, pktlen, l2len);
}

/*
 * Modify MAC address if IPv4 address is Multicast (#563)
 * ip: 32-bit IP address in network order
 * mac: pointer to packet ethernet source or destination address (6 bytes)
 */
static void
dlt_en10mb_ipv4_multicast_mac_update(const uint32_t ip, uint8_t mac[])
{
    /* only modify multicast packets */
    if ((ntohl(ip) & 0xf0000000) != 0xe0000000)
        return;

    mac[2] = (ntohl(ip) & 0x7fffff);
    mac[0] = 0x01;
    mac[1] = 0x0;
    mac[2] = 0x5e;
}

/*
 * Modify MAC address if IPv4 address is Multicast (#563)
 * ip6: 128-bit IPv6 address in network order
 * mac: pointer to packet ethernet source or destination address (6 bytes)
 */
static void
dlt_en10mb_ipv6_multicast_mac_update(const struct tcpr_in6_addr *ip6, uint8_t mac[])
{
    /* only modify multicast packets */
    if (ip6->tcpr_s6_addr[0] != 0xff)
        return;

    mac[0] = 0x33;
    mac[1] = 0x33;
    mac[2] = ip6->tcpr_s6_addr[12];
    mac[3] = ip6->tcpr_s6_addr[13];
    mac[4] = ip6->tcpr_s6_addr[14];
    mac[5] = ip6->tcpr_s6_addr[15];
}

/*
 * function merges the packet (containing L2 and old L3) with the l3data buffer
 * containing the new l3 data.  Note, if L2 % 4 == 0, then they're pointing to the
 * same buffer, otherwise there was a memcpy involved on strictly aligned architectures
 * like SPARC
 */
u_char *
dlt_en10mb_merge_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen, u_char *ipv4_data, u_char *ipv6_data)
{
    en10mb_extra_t *extra;
    struct tcpr_ethernet_hdr *eth;

    int l2len;

    assert(ctx);
    assert(packet);

    l2len = dlt_en10mb_l2len(ctx, packet, pktlen);
    if (l2len == -1 || pktlen < l2len)
        return NULL;

    assert(ctx->decoded_extra_size == sizeof(*extra));
    extra = (en10mb_extra_t *)ctx->decoded_extra;
    eth = (struct tcpr_ethernet_hdr *)(packet + ctx->l2offset);
    assert(eth);
    /* modify source/destination MAC if source/destination IP is multicast */
    if (ipv4_data) {
        if ((size_t)pktlen >= sizeof(*eth) + sizeof(struct tcpr_ipv4_hdr)) {
            struct tcpr_ipv4_hdr *ip_hdr = (struct tcpr_ipv4_hdr *)ipv4_data;
            if (!extra->src_modified)
                dlt_en10mb_ipv4_multicast_mac_update(ip_hdr->ip_src.s_addr, eth->ether_shost);

            if (!extra->dst_modified)
                dlt_en10mb_ipv4_multicast_mac_update(ip_hdr->ip_dst.s_addr, eth->ether_dhost);
        }
        return tcpedit_dlt_l3data_merge(ctx, packet, pktlen, ipv4_data, l2len);
    } else if (ipv6_data) {
        if ((size_t)pktlen >= sizeof(*eth) + sizeof(struct tcpr_ipv6_hdr)) {
            struct tcpr_ipv6_hdr *ip6_hdr = (struct tcpr_ipv6_hdr *)ipv6_data;
            if (!extra->src_modified)
                dlt_en10mb_ipv6_multicast_mac_update(&ip6_hdr->ip_src, eth->ether_shost);

            if (!extra->dst_modified)
                dlt_en10mb_ipv6_multicast_mac_update(&ip6_hdr->ip_dst, eth->ether_dhost);
        }
        return tcpedit_dlt_l3data_merge(ctx, packet, pktlen, ipv6_data, l2len);
    }

    return NULL;
}

/*
 * return a static pointer to the source/destination MAC address
 * return NULL on error/address doesn't exist
 */
u_char *
dlt_en10mb_get_mac(tcpeditdlt_t *ctx, tcpeditdlt_mac_type_t mac, const u_char *packet, int pktlen)
{
    assert(ctx);
    assert(packet);
    if (pktlen < 14)
        return NULL;

    /* FIXME: return a ptr to the source or dest mac address. */
    switch (mac) {
    case SRC_MAC:
        memcpy(ctx->srcmac, &packet[6], ETHER_ADDR_LEN);
        return (ctx->srcmac);
    case DST_MAC:
        memcpy(ctx->dstmac, packet, ETHER_ADDR_LEN);
        return (ctx->dstmac);
    default:
        errx(1, "Invalid tcpeditdlt_mac_type_t: %d", mac);
    }
}

/*
 * return the length of the L2 header of the current packet
 */
int
dlt_en10mb_l2len(tcpeditdlt_t *ctx, const u_char *packet, int pktlen)
{
    int l2len;

    assert(ctx);
    assert(packet);

    if (pktlen < (int)sizeof(eth_hdr_t)) {
        tcpedit_seterr(ctx->tcpedit, "dlt_en10mb_l2len: pktlen=%u is less than size of Ethernet header", pktlen);
        return TCPEDIT_ERROR;
    }

    l2len = get_l2len(packet, pktlen, dlt_value);
    if (l2len > 0) {
        if (pktlen < l2len) {
            /* can happen if fuzzing is enabled */
            tcpedit_seterr(ctx->tcpedit, "dlt_en10mb_l2len: pktlen=%u is less than l2len=%u", pktlen, l2len);
            return TCPEDIT_ERROR;
        }

        return l2len;
    }

    tcpedit_seterr(ctx->tcpedit, "dlt_en10mb_l2len: %s", "Whoops!  Bug in my code!");
    return TCPEDIT_ERROR;
}

tcpeditdlt_l2addr_type_t
dlt_en10mb_l2addr_type(void)
{
    return ETHERNET;
}
