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
#include "common.h"
#include "edit_packet.h"
#include "fuzzing.h"
#include "incremental_checksum.h"
#include "parse_args.h"
#include "portmap.h"
#include "rewrite_sequence.h"
#include "tcpedit_stub.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

/**
 * \brief Edit the given packet
 *
 * Process a given packet and edit the pkthdr/pktdata structures
 * according to the rules in tcpedit
 * Returns: TCPEDIT_ERROR on error
 *          TCPEDIT_SOFT_ERROR on remove packet
 *          0 on no change
 *          1 on change
 */
int
tcpedit_packet(tcpedit_t *tcpedit, struct pcap_pkthdr **pkthdr, u_char **pktdata, tcpr_dir_t direction)
{
    bool fuzz_once = tcpedit->fuzz_seed != 0;
    ipv4_hdr_t *ip_hdr;
    ipv6_hdr_t *ip6_hdr;
    arp_hdr_t *arp_hdr;
    int l2len, l2proto, retval;
    int dst_dlt, src_dlt, pktlen, lendiff;
    uint32_t ipflags, tclass;
    int needtorecalc; /* did the packet change? if so, checksum */
    u_char *packet;

    assert(tcpedit);
    assert(pkthdr);
    assert(*pkthdr);
    assert(pktdata);
    assert(*pktdata);
    assert(tcpedit->validated);

    packet = *pktdata;

    tcpedit->runtime.packetnum++;

    dbgx(3, "packet " COUNTER_SPEC " caplen %d", tcpedit->runtime.packetnum, (*pkthdr)->caplen);

    /*
     * remove the Ethernet FCS (checksum)?
     * note that this feature requires the end user to be smart and
     * only set this flag IFF the pcap has the FCS.  If not, then they
     * just removed 2 bytes of ACTUAL PACKET DATA.  Sucks to be them.
     */
    if (tcpedit->efcs > 0 && (*pkthdr)->len > 4) {
        if ((*pkthdr)->caplen == (*pkthdr)->len) {
            (*pkthdr)->caplen -= 4;
        }

        (*pkthdr)->len -= 4;
    }

    src_dlt = tcpedit_dlt_src(tcpedit->dlt_ctx);

    needtorecalc = 0;
again:
    ip_hdr = NULL;
    ip6_hdr = NULL;
    arp_hdr = NULL;
    retval = 0;
    ipflags = 0;
    /* not everything has a L3 header, so check for errors.  returns proto in network byte order */
    if ((l2proto = tcpedit_dlt_proto(tcpedit->dlt_ctx, src_dlt, packet, (int)(*pkthdr)->caplen)) < 0) {
        dbgx(2, "Packet has no L3+ header: %s", tcpedit_geterr(tcpedit));
        return TCPEDIT_SOFT_ERROR;
    } else {
        dbgx(2, "Layer 3 protocol type is: 0x%04x", ntohs(l2proto));
    }

    /* rewrite Layer 2 */
    if ((pktlen = tcpedit_dlt_process(tcpedit->dlt_ctx, pktdata, (int)(*pkthdr)->caplen, direction)) < 0) {
        /* unable to edit packet, most likely 802.11 management or data QoS frame */
        dbgx(3, "Failed to edit DLT: %s", tcpedit_geterr(tcpedit));
        return TCPEDIT_SOFT_ERROR;
    }

    /* update our packet lengths (real/captured) based on L2 length changes */
    lendiff = pktlen - (int)(*pkthdr)->caplen;
    (*pkthdr)->caplen += lendiff;
    (*pkthdr)->len += lendiff;

    dst_dlt = tcpedit_dlt_dst(tcpedit->dlt_ctx);
    l2len = tcpedit_dlt_l2len(tcpedit->dlt_ctx, dst_dlt, packet, (int)(*pkthdr)->caplen);
    if (l2len == -1)
        return TCPEDIT_SOFT_ERROR;

    dbgx(2, "dst_dlt = %04x\tsrc_dlt = %04x\tproto = %04x\tl2len = %d", dst_dlt, src_dlt, ntohs(l2proto), l2len);

    /* does packet have an IP header?  if so set our pointer to it */
    if (l2proto == htons(ETHERTYPE_IP)) {
        u_char *p;

        if ((*pkthdr)->caplen < l2len + sizeof(*ip_hdr)) {
            tcpedit_seterr(tcpedit,
                           "Packet length %d is too short to contain a layer IP header for DLT 0x%04x",
                           pktlen,
                           dst_dlt);
            return TCPEDIT_SOFT_ERROR;
        }

        ip_hdr = (ipv4_hdr_t *)tcpedit_dlt_l3data(tcpedit->dlt_ctx, dst_dlt, packet, (int)(*pkthdr)->caplen);
        if (ip_hdr == NULL)
            return TCPEDIT_SOFT_ERROR;

        p = get_layer4_v4(ip_hdr, (u_char *)ip_hdr + (*pkthdr)->caplen - l2len);
        if (!p) {
            tcpedit_seterr(tcpedit,
                           "Packet length %d is too short to contain a layer %d byte IP header for DLT 0x%04x",
                           pktlen,
                           ip_hdr->ip_hl << 2,
                           dst_dlt);
            return TCPEDIT_SOFT_ERROR;
        }

        dbgx(3, "Packet has an IPv4 header: 0x%p...", ip_hdr);
    } else if (l2proto == htons(ETHERTYPE_IP6)) {
        u_char *p;

        if ((*pkthdr)->caplen < l2len + sizeof(*ip6_hdr)) {
            tcpedit_seterr(tcpedit,
                           "Packet length %d is too short to contain a layer IPv6 header for DLT 0x%04x",
                           pktlen,
                           dst_dlt);
            return TCPEDIT_SOFT_ERROR;
        }

        ip6_hdr = (ipv6_hdr_t *)tcpedit_dlt_l3data(tcpedit->dlt_ctx, dst_dlt, packet, (int)(*pkthdr)->caplen);
        if (ip6_hdr == NULL)
            return TCPEDIT_SOFT_ERROR;

        p = get_layer4_v6(ip6_hdr, (u_char *)ip6_hdr + (*pkthdr)->caplen - l2len);
        if (!p) {
            tcpedit_seterr(tcpedit,
                           "Packet length %d is too short to contain an IPv6 header for DLT 0x%04x",
                           pktlen,
                           dst_dlt);
            return TCPEDIT_SOFT_ERROR;
        }

        dbgx(3, "Packet has an IPv6 header: 0x%p...", ip6_hdr);
    } else {
        dbgx(3, "Packet isn't IPv4 or IPv6: 0x%04x", l2proto);
        /* non-IP packets have a NULL ip_hdr struct */
        ip_hdr = NULL;
        ip6_hdr = NULL;
    }

    /* The following edits only apply for IPv4 */
    if (ip_hdr != NULL) {
        /* set TOS ? */
        if (tcpedit->tos > -1) {
            volatile uint16_t oldval = *((uint16_t *)ip_hdr);
            volatile uint16_t newval;

            ip_hdr->ip_tos = tcpedit->tos;
            newval = *((uint16_t *)ip_hdr);
            csum_replace2(&ip_hdr->ip_sum, oldval, newval);
        }

        /* rewrite the TTL */
        needtorecalc += rewrite_ipv4_ttl(tcpedit, ip_hdr);

        /* rewrite TCP/UDP ports */
        if (tcpedit->portmap != NULL) {
            if ((retval = rewrite_ipv4_ports(tcpedit, &ip_hdr, (int)(*pkthdr)->caplen - l2len)) < 0)
                return TCPEDIT_ERROR;
            needtorecalc += retval;
        }

        if (tcpedit->tcp_sequence_enable)
            rewrite_ipv4_tcp_sequence(tcpedit, &ip_hdr, (int)(*pkthdr)->caplen - l2len);
    }

    /* IPv6 edits */
    else if (ip6_hdr != NULL) {
        /* rewrite the hop limit */
        needtorecalc += rewrite_ipv6_hlim(tcpedit, ip6_hdr);

        /* set traffic class? */
        if (tcpedit->tclass > -1) {
            /* calculate the bits */
            tclass = tcpedit->tclass << 20;

            /* convert our 4 bytes to an int */
            memcpy(&ipflags, &ip6_hdr->ip_flags, 4);

            /* strip out the old tclass bits */
            ipflags = ntohl(ipflags) & 0xf00fffff;

            /* add the tclass bits back */
            ipflags += tclass;
            ipflags = htonl(ipflags);
            memcpy(&ip6_hdr->ip_flags, &ipflags, 4);
        }

        /* set the flow label? */
        if (tcpedit->flowlabel > -1) {
            memcpy(&ipflags, &ip6_hdr->ip_flags, 4);
            ipflags = ntohl(ipflags) & 0xfff00000;
            ipflags += tcpedit->flowlabel;
            ipflags = htonl(ipflags);
            memcpy(&ip6_hdr->ip_flags, &ipflags, 4);
        }

        /* rewrite TCP/UDP ports */
        if (tcpedit->portmap != NULL) {
            if ((retval = rewrite_ipv6_ports(tcpedit, &ip6_hdr, (int)(*pkthdr)->caplen - l2len)) < 0)
                return TCPEDIT_ERROR;
            needtorecalc += retval;
        }

        if (tcpedit->tcp_sequence_enable)
            rewrite_ipv6_tcp_sequence(tcpedit, &ip6_hdr, (int)(*pkthdr)->caplen - l2len);
    }

    if (fuzz_once) {
        fuzz_once = false;
        retval = fuzzing(tcpedit, *pkthdr, pktdata);
        if (retval < 0) {
            return TCPEDIT_ERROR;
        }
        needtorecalc += retval;
        goto again;
    }

    /* (Un)truncate or MTU truncate packet? */
    if (tcpedit->fixlen || tcpedit->mtu_truncate) {
        if ((retval = untrunc_packet(tcpedit, *pkthdr, pktdata, ip_hdr, ip6_hdr)) < 0)
            return TCPEDIT_ERROR;
        needtorecalc += retval;
    }

    /* rewrite IP addresses in IPv4/IPv6 or ARP */
    if (tcpedit->rewrite_ip) {
        /* IP packets */
        if (ip_hdr != NULL) {
            if ((retval = rewrite_ipv4l3(tcpedit, ip_hdr, direction, (int)(*pkthdr)->caplen - l2len)) < 0)
                return TCPEDIT_ERROR;
            needtorecalc += retval;
        } else if (ip6_hdr != NULL) {
            if ((retval = rewrite_ipv6l3(tcpedit, ip6_hdr, direction, (int)(*pkthdr)->caplen - l2len)) < 0)
                return TCPEDIT_ERROR;
            needtorecalc += retval;
        }

        /* ARP packets */
        else if (l2proto == htons(ETHERTYPE_ARP)) {
            arp_hdr = (arp_hdr_t *)&(packet[l2len]);
            /* unlike, rewrite_ipl3, we don't care if the packet changed
             * because we never need to recalc the checksums for an ARP
             * packet.  So ignore the return value
             */
            if (rewrite_iparp(tcpedit, arp_hdr, direction) < 0)
                return TCPEDIT_ERROR;
        }
    }

    /* do we need to spoof the src/dst IP address in IPv4 or ARP? */
    if (tcpedit->seed) {
        /* IPv4 Packets */
        if (ip_hdr != NULL) {
            if ((retval = randomize_ipv4(tcpedit, *pkthdr, packet, ip_hdr, (int)(*pkthdr)->caplen - l2len)) < 0)
                return TCPEDIT_ERROR;
            needtorecalc += retval;

        } else if (ip6_hdr != NULL) {
            if ((retval = randomize_ipv6(tcpedit, *pkthdr, packet, ip6_hdr, (int)(*pkthdr)->caplen - l2len)) < 0)
                return TCPEDIT_ERROR;
            needtorecalc += retval;

            /* ARP packets */
        } else if (l2proto == htons(ETHERTYPE_ARP)) {
            if (direction == TCPR_DIR_C2S) {
                if (randomize_iparp(tcpedit, *pkthdr, packet, tcpedit->runtime.dlt1, (int)(*pkthdr)->caplen - l2len) <
                    0)
                    return TCPEDIT_ERROR;
            } else {
                if (randomize_iparp(tcpedit, *pkthdr, packet, tcpedit->runtime.dlt2, (int)(*pkthdr)->caplen - l2len) <
                    0)
                    return TCPEDIT_ERROR;
            }
        }
    }

    /* ensure IP header length is correct */
    if (ip_hdr != NULL) {
        fix_ipv4_length(*pkthdr, ip_hdr, l2len);
        needtorecalc = 1;
    } else if (ip6_hdr != NULL) {
        needtorecalc |= fix_ipv6_length(*pkthdr, ip6_hdr, l2len);
    }

    /* do we need to fix checksums? -- must always do this last! */
    if ((tcpedit->fixcsum || needtorecalc > 0)) {
        if (ip_hdr != NULL) {
            dbgx(3, "doing IPv4 checksum: needtorecalc=%d", needtorecalc);
            retval = fix_ipv4_checksums(tcpedit, *pkthdr, ip_hdr, l2len);
        } else if (ip6_hdr != NULL) {
            dbgx(3, "doing IPv6 checksum: needtorecalc=%d", needtorecalc);
            retval = fix_ipv6_checksums(tcpedit, *pkthdr, ip6_hdr, l2len);
        } else {
            dbgx(3, "checksum not performed: needtorecalc=%d", needtorecalc);
            retval = TCPEDIT_OK;
        }
        if (retval < 0) {
            return TCPEDIT_ERROR;
        } else if (retval == TCPEDIT_WARN) {
            warnx("%s", tcpedit_getwarn(tcpedit));
        }
    }

    tcpedit_dlt_merge_l3data(tcpedit->dlt_ctx,
                             dst_dlt,
                             packet,
                             (int)(*pkthdr)->caplen,
                             (u_char *)ip_hdr,
                             (u_char *)ip6_hdr);

    tcpedit->runtime.total_bytes += (*pkthdr)->caplen;
    tcpedit->runtime.pkts_edited++;
    return retval;
}

/**
 * initializes the tcpedit library.  returns 0 on success, -1 on error.
 */
int
tcpedit_init(tcpedit_t **tcpedit_ex, int dlt)
{
    tcpedit_t *tcpedit;

    *tcpedit_ex = safe_malloc(sizeof(tcpedit_t));
    tcpedit = *tcpedit_ex;

    if ((tcpedit->dlt_ctx = tcpedit_dlt_init(tcpedit, dlt)) == NULL)
        return TCPEDIT_ERROR;

    tcpedit->mtu = DEFAULT_MTU; /* assume 802.3 Ethernet */

    tcpedit->fuzz_factor = DEFAULT_FUZZ_FACTOR;

    /* disabled by default */
    tcpedit->tos = -1;
    tcpedit->tclass = -1;
    tcpedit->flowlabel = -1;
    tcpedit->editdir = TCPEDIT_EDIT_BOTH;

    memset(&(tcpedit->runtime), 0, sizeof(tcpedit_runtime_t));
    tcpedit->runtime.dlt1 = dlt;
    tcpedit->runtime.dlt2 = dlt;

    dbgx(1, "Input file (1) datalink type is %s", pcap_datalink_val_to_name(dlt));

#ifdef FORCE_ALIGN
    tcpedit->runtime.l3buff = (u_char *)safe_malloc(MAXPACKET);
#endif

    return TCPEDIT_OK;
}

/**
 * return the output DLT type
 */
int
tcpedit_get_output_dlt(tcpedit_t *tcpedit)
{
    assert(tcpedit);
    return tcpedit_dlt_output_dlt(tcpedit->dlt_ctx);
}

/**
 * \brief tcpedit option validator.  Call after tcpedit_init()
 *
 * Validates that given the current state of tcpedit that the given
 * pcap source and destination (based on DLT) can be properly rewritten
 * return 0 on success
 * return -1 on error
 * DO NOT USE!
 */
int
tcpedit_validate(tcpedit_t *tcpedit)
{
    assert(tcpedit);
    tcpedit->validated = 1;

    /* we used to do a bunch of things here, but not anymore...
     * maybe I should find something to do or just get ride of it
     */
    return 0;
}

/**
 * return the error string when a tcpedit() function returns
 * TCPEDIT_ERROR
 */
char *
tcpedit_geterr(tcpedit_t *tcpedit)
{
    assert(tcpedit);
    return tcpedit->runtime.errstr;
}

/**
 * \brief Internal function to set the tcpedit error string
 *
 * Used to set the error string when there is an error, result is retrieved
 * using tcpedit_geterr().  You shouldn't ever actually call this, but use
 * tcpedit_seterr() which is a macro wrapping this instead.
 */
void
__tcpedit_seterr(tcpedit_t *tcpedit, const char *func, int line, const char *file, const char *fmt, ...)
{
    va_list ap;
    char errormsg[TCPEDIT_ERRSTR_LEN - 32];

    assert(tcpedit);

    va_start(ap, fmt);
    if (fmt != NULL) {
        (void)vsnprintf(errormsg, sizeof(errormsg), fmt, ap);
    }

    va_end(ap);

    snprintf(tcpedit->runtime.errstr,
             sizeof(tcpedit->runtime.errstr),
             "From %s:%s() line %d:\n%s",
             file,
             func,
             line,
             errormsg);
}

/**
 * return the warning string when a tcpedit() function returns
 * TCPEDIT_WARN
 */
char *
tcpedit_getwarn(tcpedit_t *tcpedit)
{
    assert(tcpedit);

    return tcpedit->runtime.warnstr;
}

/**
 * used to set the warning string when there is an warning
 */
void
tcpedit_setwarn(tcpedit_t *tcpedit, const char *fmt, ...)
{
    va_list ap;
    assert(tcpedit);

    va_start(ap, fmt);
    if (fmt != NULL)
        (void)vsnprintf(tcpedit->runtime.warnstr, sizeof(tcpedit->runtime.warnstr), fmt, ap);

    va_end(ap);
}

/**
 * \brief Checks the given error code and does the right thing
 *
 * Generic function which checks the TCPEDIT_* error code
 * and always returns OK or ERROR.  For warnings, prints the
 * warning message and returns OK.  For any other value, fails with
 * an assert.
 *
 * prefix is a string prepended to the error/warning
 */
int
tcpedit_checkerror(tcpedit_t *tcpedit, int rcode, const char *prefix)
{
    assert(tcpedit);

    switch (rcode) {
    case TCPEDIT_OK:
    case TCPEDIT_ERROR:
        return rcode;
    case TCPEDIT_SOFT_ERROR:
        if (prefix != NULL) {
            fprintf(stderr, "Error %s: %s\n", prefix, tcpedit_geterr(tcpedit));
        } else {
            fprintf(stderr, "Error: %s\n", tcpedit_geterr(tcpedit));
        }
        break;
    case TCPEDIT_WARN:
        if (prefix != NULL) {
            fprintf(stderr, "Warning %s: %s\n", prefix, tcpedit_getwarn(tcpedit));
        } else {
            fprintf(stderr, "Warning: %s\n", tcpedit_getwarn(tcpedit));
        }
        return TCPEDIT_OK;
    default:
        assert(0 == 1); /* this should never happen! */
        break;
    }
    return TCPEDIT_ERROR;
}

/**
 * \brief Cleans up after ourselves.  Return 0 on success.
 *
 * Clean up after ourselves and free the ptr.
 */
int
tcpedit_close(tcpedit_t **tcpedit_ex)
{
    assert(*tcpedit_ex);
    tcpedit_t *tcpedit;

    tcpedit = *tcpedit_ex;

    dbgx(1,
         "tcpedit processed " COUNTER_SPEC " bytes in " COUNTER_SPEC " packets.",
         tcpedit->runtime.total_bytes,
         tcpedit->runtime.pkts_edited);

    /* free if required */
    if (tcpedit->dlt_ctx) {
        tcpedit_dlt_cleanup(tcpedit->dlt_ctx);
        tcpedit->dlt_ctx = NULL;
    }

    if (tcpedit->cidrmap1) {
        destroy_cidr(tcpedit->cidrmap1->from);
        tcpedit->cidrmap1->from = NULL;
        destroy_cidr(tcpedit->cidrmap1->to);
        tcpedit->cidrmap1->to = NULL;
    }

    if (tcpedit->cidrmap2 && tcpedit->cidrmap2 != tcpedit->cidrmap1) {
        destroy_cidr(tcpedit->cidrmap2->from);
        tcpedit->cidrmap2->from = NULL;
        destroy_cidr(tcpedit->cidrmap2->to);
        tcpedit->cidrmap2->to = NULL;
        safe_free(tcpedit->cidrmap2);
        tcpedit->cidrmap2 = NULL;
    }

    safe_free(tcpedit->cidrmap1);
    tcpedit->cidrmap1 = NULL;

    if (tcpedit->srcipmap) {
        destroy_cidr(tcpedit->srcipmap->from);
        tcpedit->srcipmap->from = NULL;
        destroy_cidr(tcpedit->srcipmap->to);
        tcpedit->srcipmap->to = NULL;
    }

    if (tcpedit->dstipmap && tcpedit->dstipmap != tcpedit->srcipmap) {
        destroy_cidr(tcpedit->dstipmap->from);
        tcpedit->dstipmap->from = NULL;
        destroy_cidr(tcpedit->dstipmap->to);
        tcpedit->dstipmap->to = NULL;
        safe_free(tcpedit->dstipmap);
        tcpedit->dstipmap = NULL;
    }

    safe_free(tcpedit->srcipmap);
    tcpedit->srcipmap = NULL;

    if (tcpedit->portmap) {
        free_portmap(tcpedit->portmap);
        tcpedit->portmap = NULL;
    }

#ifdef FORCE_ALIGN
    safe_free(tcpedit->runtime.l3buff);
    tcpedit->runtime.l3buff = NULL;
#endif

    safe_free(*tcpedit_ex);
    *tcpedit_ex = NULL;

    return 0;
}

/**
 * Return a ptr to the Layer 3 data.  Returns TCPEDIT_ERROR on error
 */
const u_char *
tcpedit_l3data(tcpedit_t *tcpedit, tcpedit_coder code, u_char *packet, int pktlen)
{
    u_char *result = NULL;
    if (code == BEFORE_PROCESS) {
        result = tcpedit_dlt_l3data(tcpedit->dlt_ctx, tcpedit->dlt_ctx->decoder->dlt, packet, pktlen);
    } else {
        result = tcpedit_dlt_l3data(tcpedit->dlt_ctx, tcpedit->dlt_ctx->encoder->dlt, packet, pktlen);
    }
    return result;
}

/**
 * Returns the layer 3 type, often encoded as the layer2.proto field
 */
int
tcpedit_l3proto(tcpedit_t *tcpedit, tcpedit_coder code, const u_char *packet, int pktlen)
{
    int result;
    if (code == BEFORE_PROCESS) {
        result = tcpedit_dlt_proto(tcpedit->dlt_ctx, tcpedit->dlt_ctx->decoder->dlt, packet, pktlen);
    } else {
        result = tcpedit_dlt_proto(tcpedit->dlt_ctx, tcpedit->dlt_ctx->encoder->dlt, packet, pktlen);
    }
    return ntohs(result);
}
