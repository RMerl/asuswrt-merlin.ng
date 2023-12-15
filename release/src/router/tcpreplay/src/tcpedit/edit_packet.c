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

#include "edit_packet.h"
#include "config.h"
#include "checksum.h"
#include "dlt.h"
#include "incremental_checksum.h"
#include "tcpedit.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

static uint32_t randomize_ipv4_addr(tcpedit_t *tcpedit, uint32_t ip);
static uint32_t remap_ipv4(tcpedit_t *tcpedit, tcpr_cidr_t *cidr, uint32_t original);
static int is_unicast_ipv4(tcpedit_t *tcpedit, uint32_t ip);

static void randomize_ipv6_addr(tcpedit_t *tcpedit, struct tcpr_in6_addr *addr);
static int remap_ipv6(tcpedit_t *tcpedit, tcpr_cidr_t *cidr, struct tcpr_in6_addr *addr);
static int is_multicast_ipv6(tcpedit_t *tcpedit, struct tcpr_in6_addr *addr);
static int ipv6_header_length(ipv6_hdr_t const *ip6_hdr, size_t pkt_len, size_t l2len);

/**
 * this code re-calcs the IP and Layer 4 checksums
 * the IMPORTANT THING is that the Layer 4 header
 * is contiguous in memory after *ip_hdr we're actually
 * writing to the layer 4 header via the ip_hdr ptr.
 * (Yes, this sucks, but that's the way libnet works, and
 * I was too lazy to re-invent the wheel.
 * Returns 0 on success, -1 on error
 */
int
fix_ipv4_checksums(tcpedit_t *tcpedit, struct pcap_pkthdr *pkthdr, ipv4_hdr_t *ip_hdr, size_t l2len)
{
    int ret1 = 0;
    int ret2;
    int ip_len;
    assert(tcpedit);
    assert(pkthdr);
    assert(ip_hdr);

    if (pkthdr->caplen < (sizeof(*ip_hdr) + l2len)) {
        tcpedit_setwarn(tcpedit,
                        "caplen too small to read IPv4 header: caplen=%u: pkt=" COUNTER_SPEC,
                        pkthdr->caplen,
                        tcpedit->runtime.packetnum);
        return TCPEDIT_WARN;
    }

    if (ip_hdr->ip_v != 4) {
        tcpedit_seterr(tcpedit,
                       "Invalid packet: Expected IPv4 packet: got %u: pkt=" COUNTER_SPEC,
                       ip_hdr->ip_v,
                       tcpedit->runtime.packetnum);
        return TCPEDIT_ERROR;
    }

    ip_len = (int)ntohs(ip_hdr->ip_len);
    /* calc the L4 checksum if we have the whole packet && not a frag or first frag */
    if (pkthdr->caplen == pkthdr->len && (htons(ip_hdr->ip_off) & (IP_MF | IP_OFFMASK)) == 0) {
        if (ip_len != (int)(pkthdr->caplen - l2len)) {
            tcpedit_seterr(tcpedit,
                           "caplen minus L2 length %u does IPv4 header length %u: pkt=" COUNTER_SPEC,
                           pkthdr->caplen - l2len,
                           ip_len,
                           tcpedit->runtime.packetnum);
            return TCPEDIT_ERROR;
        }
        ret1 = do_checksum(tcpedit,
                           (u_char *)ip_hdr,
                           ip_hdr->ip_p,
                           ip_len - (ip_hdr->ip_hl << 2),
                           (u_char *)ip_hdr + pkthdr->caplen - l2len);
        if (ret1 < 0)
            return TCPEDIT_ERROR;
    }

    /* calc IP checksum */
    ret2 = do_checksum(tcpedit, (u_char *)ip_hdr, IPPROTO_IP, ip_len, (u_char *)ip_hdr + pkthdr->caplen - l2len);
    if (ret2 < 0)
        return TCPEDIT_ERROR;

    /* what do we return? */
    if (ret1 == TCPEDIT_WARN || ret2 == TCPEDIT_WARN)
        return TCPEDIT_WARN;

    return TCPEDIT_OK;
}

/**
 * Returns ipv6 header length with all ipv6 options on success
 *         -1 on error
 */
static int
ipv6_header_length(ipv6_hdr_t const *ip6_hdr, size_t pkt_len, size_t l2len)
{
    struct tcpr_ipv6_ext_hdr_base const *nhdr;
    uint8_t next_header;
    int offset;

    offset = sizeof(*ip6_hdr);
    next_header = ip6_hdr->ip_nh;

    while (sizeof(*nhdr) + offset + l2len < (size_t)pkt_len) {
        if (next_header != TCPR_IPV6_NH_HBH && next_header != TCPR_IPV6_NH_ROUTING &&
            next_header != TCPR_IPV6_NH_FRAGMENT) {
            return offset;
        }

        nhdr = (struct tcpr_ipv6_ext_hdr_base const *)(((uint8_t const *)ip6_hdr) + offset);
        next_header = nhdr->ip_nh;
        offset += ((nhdr->ip_len + 1) << 3);
    }

    return -1;
}

int
fix_ipv6_checksums(tcpedit_t *tcpedit, struct pcap_pkthdr *pkthdr, ipv6_hdr_t *ip6_hdr, size_t l2len)
{
    int ret = 0;
    assert(tcpedit);
    assert(pkthdr);
    assert(ip6_hdr);

    if (pkthdr->caplen < (sizeof(*ip6_hdr) + l2len)) {
        tcpedit_setwarn(tcpedit,
                        "caplen too small to read IPv6 header: caplen=%u pkt=" COUNTER_SPEC,
                        pkthdr->caplen,
                        tcpedit->runtime.packetnum);
        return TCPEDIT_WARN;
    }

    ipv4_hdr_t *ip_hdr = (ipv4_hdr_t *)ip6_hdr;
    if (ip_hdr->ip_v != 6) {
        tcpedit_seterr(tcpedit, "Invalid packet: Expected IPv6 packet: got %u", ip_hdr->ip_v);
        return TCPEDIT_ERROR;
    }

    /* calc the L4 checksum if we have the whole packet && not a frag or first frag */
    if (pkthdr->caplen == pkthdr->len) {
        int ip6_len = ipv6_header_length(ip6_hdr, pkthdr->len, l2len);
        if (ip6_hdr->ip_len < ip6_len) {
            tcpedit_setwarn(tcpedit,
                            "Unable to checksum IPv6 packet with invalid: pkt=" COUNTER_SPEC " IP length=%u caplen=%u",
                            tcpedit->runtime.packetnum,
                            ip6_hdr->ip_len,
                            pkthdr->caplen);
            return TCPEDIT_WARN;
        }
        ret = do_checksum(tcpedit,
                          (u_char *)ip6_hdr,
                          ip6_hdr->ip_nh,
                          htons(ip6_hdr->ip_len),
                          (u_char *)ip6_hdr + pkthdr->caplen - l2len);
        if (ret < 0)
            return TCPEDIT_ERROR;
    }

    /* what do we return? */
    if (ret == TCPEDIT_WARN)
        return TCPEDIT_WARN;

    return TCPEDIT_OK;
}

static void
ipv4_l34_csum_replace(uint8_t *data, uint8_t protocol, uint32_t old, uint32_t new)
{
    ipv4_hdr_t *ipv4;
    tcp_hdr_t *tcp_hdr;
    udp_hdr_t *udp_hdr;

    assert(data);

    switch (protocol) {
    case IPPROTO_IP:
        ipv4 = (ipv4_hdr_t *)data;
        csum_replace4(&ipv4->ip_sum, old, new);
        break;

    case IPPROTO_TCP:
        tcp_hdr = (tcp_hdr_t *)data;
        csum_replace4(&tcp_hdr->th_sum, old, new);
        break;

    case IPPROTO_UDP:
        udp_hdr = (udp_hdr_t *)data;
        if (udp_hdr->uh_sum)
            csum_replace4(&udp_hdr->uh_sum, old, new);
        break;

    default:
        assert(false);
    }
}

static void
ipv6_l34_csum_replace(uint8_t *data, uint8_t protocol, uint32_t *old, uint32_t *new)
{
    tcp_hdr_t *tcp_hdr;
    udp_hdr_t *udp_hdr;
    icmpv4_hdr_t *icmp;
    icmpv6_hdr_t *icmp6;

    assert(data);

    switch (protocol) {
    case IPPROTO_TCP:
        tcp_hdr = (tcp_hdr_t *)data;
        csum_replace16(&tcp_hdr->th_sum, old, new);
        break;

    case IPPROTO_UDP:
        udp_hdr = (udp_hdr_t *)data;
        if (udp_hdr->uh_sum)
            csum_replace16(&udp_hdr->uh_sum, old, new);
        break;

    case IPPROTO_ICMP:
        icmp = (icmpv4_hdr_t *)data;
        csum_replace16(&icmp->icmp_sum, old, new);
        break;

    case IPPROTO_ICMP6:
        icmp6 = (icmpv6_hdr_t *)data;
        csum_replace16(&icmp6->icmp_sum, old, new);
        break;

    default:
        assert(false);
    }
}

static void
ipv4_addr_csum_replace(ipv4_hdr_t *ip_hdr, uint32_t old_ip, uint32_t new_ip, int l3len)
{
    uint8_t *l4, protocol;
    int len = l3len;

    assert(ip_hdr);

    if ((size_t)len < sizeof(*ip_hdr))
        return;

    ipv4_l34_csum_replace((uint8_t *)ip_hdr, IPPROTO_IP, old_ip, new_ip);

    protocol = ip_hdr->ip_p;
    switch (protocol) {
    case IPPROTO_UDP:
        l4 = get_layer4_v4(ip_hdr, (u_char *)ip_hdr + l3len);
        len -= ip_hdr->ip_hl << 2;
        len -= TCPR_UDP_H;
        break;

    case IPPROTO_TCP:
        l4 = get_layer4_v4(ip_hdr, (u_char *)ip_hdr + l3len);
        len -= ip_hdr->ip_hl << 2;
        len -= TCPR_TCP_H;
        break;

    default:
        l4 = NULL;
    }

    if (!l4 || len < 0)
        return;

    /* if this is a fragment, don't attempt to checksum the Layer4 header */
    if ((htons(ip_hdr->ip_off) & IP_OFFMASK) == 0)
        ipv4_l34_csum_replace(l4, protocol, old_ip, new_ip);
}

static void
ipv6_addr_csum_replace(ipv6_hdr_t *ip6_hdr, struct tcpr_in6_addr *old_ip, struct tcpr_in6_addr *new_ip, int l3len)
{
    uint8_t *l4, protocol;

    assert(ip6_hdr);

    if ((size_t)l3len < sizeof(*ip6_hdr))
        return;

    protocol = get_ipv6_l4proto(ip6_hdr, (u_char *)ip6_hdr + l3len);
    switch (protocol) {
    case IPPROTO_UDP:
    case IPPROTO_TCP:
        l4 = get_layer4_v6(ip6_hdr, (u_char *)ip6_hdr + l3len);
        break;
    default:
        l4 = NULL;
    }

    if (!l4)
        return;

    ipv6_l34_csum_replace(l4, protocol, (uint32_t *)old_ip, (uint32_t *)new_ip);
}

/**
 * returns a new 32bit integer which is the randomized IP
 * based upon the user specified seed
 */
static uint32_t
randomize_ipv4_addr(tcpedit_t *tcpedit, uint32_t ip)
{
    assert(tcpedit);

    /* don't rewrite broadcast addresses */
    if (tcpedit->skip_broadcast && !is_unicast_ipv4(tcpedit, ip))
        return ip;

    return ((ip ^ htonl(tcpedit->seed)) - (ip & htonl(tcpedit->seed)));
}

static void
randomize_ipv6_addr(tcpedit_t *tcpedit, struct tcpr_in6_addr *addr)
{
    uint32_t *p;
    int i;
    u_char was_multicast;

    assert(tcpedit);

    p = &addr->__u6_addr.__u6_addr32[0];

    was_multicast = is_multicast_ipv6(tcpedit, addr);

    for (i = 0; i < 4; ++i) {
        p[i] = ((p[i] ^ htonl(tcpedit->seed)) - (p[i] & htonl(tcpedit->seed)));
    }

    if (was_multicast) {
        addr->tcpr_s6_addr[0] = 0xff;
    } else if (is_multicast_ipv6(tcpedit, addr)) {
        addr->tcpr_s6_addr[0] = 0xaa;
    }
}

int
fix_ipv4_length(struct pcap_pkthdr *pkthdr, ipv4_hdr_t *ip_hdr, size_t l2len)
{
    int ip_len = (int)ntohs(ip_hdr->ip_len);
    int ip_len_want = (int)(pkthdr->len - l2len);

    if (pkthdr->caplen < l2len + sizeof(*ip_hdr))
        return -1;

    if ((htons(ip_hdr->ip_off) & (IP_MF | IP_OFFMASK)) == 0 && ip_len != ip_len_want) {
        ip_hdr->ip_len = htons(ip_len_want);
        return 1;
    }

    return 0;
}

int
fix_ipv6_length(struct pcap_pkthdr *pkthdr, ipv6_hdr_t *ip6_hdr, size_t l2len)
{
    int ip_len = ntohs((uint16_t)ip6_hdr->ip_len);
    int ip_len_want = (int)(pkthdr->len - l2len - sizeof(*ip6_hdr));

    if (pkthdr->caplen < l2len + sizeof(*ip6_hdr))
        return -1;

    if (ip_len != ip_len_want) {
        ip6_hdr->ip_len = htons((uint16_t)ip_len_want);
        return 1;
    }

    return 0;
}

/**
 * randomizes the source and destination IP addresses based on a
 * pseudo-random number which is generated via the seed.
 * return 1 since we changed one or more IP addresses
 */
int
randomize_ipv4(tcpedit_t *tcpedit, struct pcap_pkthdr *pkthdr, const u_char *pktdata, ipv4_hdr_t *ip_hdr, int l3len)
{
#ifdef DEBUG
    char srcip[16], dstip[16];
#endif
    assert(tcpedit);
    assert(pkthdr);
    assert(pktdata);
    assert(ip_hdr);

#ifdef DEBUG
    strlcpy(srcip, get_addr2name4(ip_hdr->ip_src.s_addr, RESOLVE), 16);
    strlcpy(dstip, get_addr2name4(ip_hdr->ip_dst.s_addr, RESOLVE), 16);
#endif

    /* randomize IP addresses based on the value of random */
    dbgx(1, "Old Src IP: %s\tOld Dst IP: %s", srcip, dstip);

    if (l3len < (int)ip_hdr->ip_hl << 2) {
        tcpedit_seterr(tcpedit, "Unable to randomize IP header due to packet capture snap length %u", pkthdr->caplen);
        return TCPEDIT_ERROR;
    }

    /* don't rewrite broadcast addresses */
    if ((tcpedit->skip_broadcast && is_unicast_ipv4(tcpedit, (u_int32_t)ip_hdr->ip_dst.s_addr)) ||
        !tcpedit->skip_broadcast) {
        uint32_t old_ip = ip_hdr->ip_dst.s_addr;
        ip_hdr->ip_dst.s_addr = randomize_ipv4_addr(tcpedit, ip_hdr->ip_dst.s_addr);
        ipv4_addr_csum_replace(ip_hdr, old_ip, ip_hdr->ip_dst.s_addr, l3len);
    }

    if ((tcpedit->skip_broadcast && is_unicast_ipv4(tcpedit, (u_int32_t)ip_hdr->ip_src.s_addr)) ||
        !tcpedit->skip_broadcast) {
        uint32_t old_ip = ip_hdr->ip_src.s_addr;
        ip_hdr->ip_src.s_addr = randomize_ipv4_addr(tcpedit, ip_hdr->ip_src.s_addr);
        ipv4_addr_csum_replace(ip_hdr, old_ip, ip_hdr->ip_src.s_addr, l3len);
    }

#ifdef DEBUG
    strlcpy(srcip, get_addr2name4(ip_hdr->ip_src.s_addr, RESOLVE), 16);
    strlcpy(dstip, get_addr2name4(ip_hdr->ip_dst.s_addr, RESOLVE), 16);
#endif

    dbgx(1, "New Src IP: %s\tNew Dst IP: %s\n", srcip, dstip);

    return 0;
}

int
randomize_ipv6(tcpedit_t *tcpedit, struct pcap_pkthdr *pkthdr, const u_char *pktdata, ipv6_hdr_t *ip6_hdr, int l3len)
{
#ifdef DEBUG
    char srcip[INET6_ADDRSTRLEN], dstip[INET6_ADDRSTRLEN];
#endif
    assert(tcpedit);
    assert(pkthdr);
    assert(pktdata);
    assert(ip6_hdr);

#ifdef DEBUG
    strlcpy(srcip, get_addr2name6(&ip6_hdr->ip_src, RESOLVE), INET6_ADDRSTRLEN);
    strlcpy(dstip, get_addr2name6(&ip6_hdr->ip_dst, RESOLVE), INET6_ADDRSTRLEN);
#endif

    /* randomize IP addresses based on the value of random */
    dbgx(1, "Old Src IP: %s\tOld Dst IP: %s", srcip, dstip);
    if (l3len < (int)sizeof(ipv6_hdr_t)) {
        tcpedit_seterr(tcpedit,
                       "Unable to randomize IPv6 header due to packet capture snap length %u: pkt=" COUNTER_SPEC,
                       pkthdr->caplen,
                       tcpedit->runtime.packetnum);
        return TCPEDIT_ERROR;
    }

    /* don't rewrite broadcast addresses */
    if ((tcpedit->skip_broadcast && !is_multicast_ipv6(tcpedit, &ip6_hdr->ip_dst)) || !tcpedit->skip_broadcast) {
        struct tcpr_in6_addr old_ip6;
        memcpy(&old_ip6, &ip6_hdr->ip_dst, sizeof(old_ip6));
        randomize_ipv6_addr(tcpedit, &ip6_hdr->ip_dst);
        ipv6_addr_csum_replace(ip6_hdr, &old_ip6, &ip6_hdr->ip_dst, l3len);
    }

    if ((tcpedit->skip_broadcast && !is_multicast_ipv6(tcpedit, &ip6_hdr->ip_src)) || !tcpedit->skip_broadcast) {
        struct tcpr_in6_addr old_ip6;
        memcpy(&old_ip6, &ip6_hdr->ip_src, sizeof(old_ip6));
        randomize_ipv6_addr(tcpedit, &ip6_hdr->ip_src);
        ipv6_addr_csum_replace(ip6_hdr, &old_ip6, &ip6_hdr->ip_src, l3len);
    }

#ifdef DEBUG
    strlcpy(srcip, get_addr2name6(&ip6_hdr->ip_src, RESOLVE), INET6_ADDRSTRLEN);
    strlcpy(dstip, get_addr2name6(&ip6_hdr->ip_dst, RESOLVE), INET6_ADDRSTRLEN);
#endif

    dbgx(1, "New Src IP: %s\tNew Dst IP: %s\n", srcip, dstip);

    return 0;
}

/**
 * this code will untruncate a packet via padding it with null
 * or resetting the actual IPv4 packet len to the snaplen - L2 header.
 * return 0 if no change, 1 if change, -1 on error.
 */

int
untrunc_packet(tcpedit_t *tcpedit,
               struct pcap_pkthdr *pkthdr,
               u_char **pktdata,
               ipv4_hdr_t *ip_hdr,
               ipv6_hdr_t *ip6_hdr)
{
    int l2len;
    int chksum = 1;
    u_char *packet;
    udp_hdr_t *udp_hdr;
    u_char *dataptr = NULL;

    assert(tcpedit);
    assert(pkthdr);
    assert(pktdata);

    packet = *pktdata;
    assert(packet);

    /* if actual len == cap len or there's no IP header, don't do anything */
    if ((pkthdr->caplen == pkthdr->len) || (ip_hdr == NULL && ip6_hdr == NULL)) {
        /* unless we're in MTU truncate mode */
        if (!tcpedit->mtu_truncate)
            return (0);
    }

    if ((l2len = layer2len(tcpedit, packet, pkthdr->caplen)) < 0) {
        tcpedit_seterr(tcpedit, "Non-sensical layer 2 length: %d", l2len);
        return -1;
    }

    /*
     * cannot checksum fragments, but we can do some
     * work on UDP fragments. Setting checksum to 0
     * means checksum will be ignored.
     */
    if (ip_hdr) {
        if ((htons(ip_hdr->ip_off) & IP_OFFMASK) != 0) {
            chksum = 0;
        } else if (ip_hdr->ip_p == IPPROTO_UDP && (htons(ip_hdr->ip_off) & IP_MF) != 0) {
            dataptr = (u_char *)ip_hdr;
            dataptr += ip_hdr->ip_hl << 2;
            udp_hdr = (udp_hdr_t *)dataptr;
            udp_hdr->uh_sum = 0;
            chksum = 0;
        }
    }

    /* Pad packet or truncate it */
    if (tcpedit->fixlen == TCPEDIT_FIXLEN_PAD) {
        /*
         * this should be an unnecessary check
         * but I've gotten a report that sometimes the caplen > len
         * which seems like a corrupted pcap
         */
        if (pkthdr->len > pkthdr->caplen) {
            packet = safe_realloc(packet, pkthdr->len + PACKET_HEADROOM);
            memset(packet + pkthdr->caplen, '\0', pkthdr->len - pkthdr->caplen);
            pkthdr->caplen = pkthdr->len;
        } else if (pkthdr->len < pkthdr->caplen) {
            /* i guess this is necessary if we've got a bogus pcap */
            // ip_hdr->ip_len = htons(pkthdr->caplen - l2len);
            tcpedit_seterr(tcpedit, "%s", "WTF?  Why is your packet larger then the capture len?");
            chksum = -1;
            goto done;
        }
    } else if (tcpedit->fixlen == TCPEDIT_FIXLEN_TRUNC) {
        if (ip_hdr && pkthdr->len != pkthdr->caplen)
            ip_hdr->ip_len = htons(pkthdr->caplen - l2len);
        pkthdr->len = pkthdr->caplen;
    } else if (tcpedit->mtu_truncate) {
        if (pkthdr->len > (uint32_t)(tcpedit->mtu + l2len)) {
            /* first truncate the packet */
            pkthdr->len = pkthdr->caplen = l2len + tcpedit->mtu;

            /* if ip_hdr exists, update the length */
            if (ip_hdr) {
                ip_hdr->ip_len = htons(tcpedit->mtu);
            } else if (ip6_hdr) {
                ip6_hdr->ip_len = htons(tcpedit->mtu - sizeof(*ip6_hdr));
            } else {
                /* for non-IP frames, don't try to fix checksums */
                chksum = 0;
                goto done;
            }
        }
    } else {
        tcpedit_seterr(tcpedit, "Invalid fixlen value: 0x%x", tcpedit->fixlen);
        chksum = -1;
        goto done;
    }

done:
    *pktdata = packet;
    return chksum;
}

/**
 * rewrites an IPv4 packet's TTL based on the rules
 * return 0 if no change, 1 if changed
 */
int
rewrite_ipv4_ttl(tcpedit_t *tcpedit, ipv4_hdr_t *ip_hdr)
{
    volatile uint16_t oldval, newval;

    assert(tcpedit);

    /* make sure there's something to edit */
    if (ip_hdr == NULL || tcpedit->ttl_mode == false)
        return (0);

    oldval = (uint16_t)ip_hdr->ip_ttl;
    switch (tcpedit->ttl_mode) {
    case TCPEDIT_TTL_MODE_SET:
        if (ip_hdr->ip_ttl == tcpedit->ttl_value)
            return 0; /* no change required */
        ip_hdr->ip_ttl = tcpedit->ttl_value;
        break;
    case TCPEDIT_TTL_MODE_ADD:
        if (((int)ip_hdr->ip_ttl + tcpedit->ttl_value) > 255) {
            ip_hdr->ip_ttl = 255;
        } else {
            ip_hdr->ip_ttl += tcpedit->ttl_value;
        }
        break;
    case TCPEDIT_TTL_MODE_SUB:
        if (ip_hdr->ip_ttl <= tcpedit->ttl_value) {
            ip_hdr->ip_ttl = 1;
        } else {
            ip_hdr->ip_ttl -= tcpedit->ttl_value;
        }
        break;
    default:
        errx(1, "invalid ttl_mode: %d", tcpedit->ttl_mode);
    }

    newval = (uint16_t)ip_hdr->ip_ttl;
    csum_replace2(&ip_hdr->ip_sum, oldval, newval);

    return 1;
}

/**
 * rewrites an IPv6 packet's hop limit based on the rules
 * return 0 if no change, 1 if changed
 */
int
rewrite_ipv6_hlim(tcpedit_t *tcpedit, ipv6_hdr_t *ip6_hdr)
{
    assert(tcpedit);

    /* make sure there's something to edit */
    if (ip6_hdr == NULL || tcpedit->ttl_mode == TCPEDIT_TTL_MODE_OFF)
        return (0);

    switch (tcpedit->ttl_mode) {
    case TCPEDIT_TTL_MODE_SET:
        if (ip6_hdr->ip_hl == tcpedit->ttl_value)
            return (0); /* no change required */
        ip6_hdr->ip_hl = tcpedit->ttl_value;
        break;
    case TCPEDIT_TTL_MODE_ADD:
        if (((int)ip6_hdr->ip_hl + tcpedit->ttl_value) > 255) {
            ip6_hdr->ip_hl = 255;
        } else {
            ip6_hdr->ip_hl += tcpedit->ttl_value;
        }
        break;
    case TCPEDIT_TTL_MODE_SUB:
        if (ip6_hdr->ip_hl <= tcpedit->ttl_value) {
            ip6_hdr->ip_hl = 1;
        } else {
            ip6_hdr->ip_hl -= tcpedit->ttl_value;
        }
        break;
    default:
        errx(1, "invalid ttl_mode: %d", tcpedit->ttl_mode);
    }
    return 1;
}

/**
 * takes a CIDR notation netblock and uses that to "remap" given IP
 * onto that netblock.  ie: 10.0.0.0/8 and 192.168.55.123 -> 10.168.55.123
 * while 10.150.9.0/24 and 192.168.55.123 -> 10.150.9.123
 */
static uint32_t
remap_ipv4(tcpedit_t *tcpedit, tcpr_cidr_t *cidr, uint32_t original)
{
    uint32_t ipaddr, network, mask, result;

    assert(tcpedit);
    assert(cidr);

    if (cidr->family != AF_INET) {
        return 0;
    }

    /* don't rewrite broadcast addresses */
    if (tcpedit->skip_broadcast && !is_unicast_ipv4(tcpedit, original))
        return original;

    mask = 0xffffffff; /* turn on all the bits */

    /* shift over by correct # of bits */
    mask = mask << (32 - cidr->masklen);

    /* apply the mask to the network */
    network = htonl(cidr->u.network) & mask;

    /* apply the reverse of the mask to the IP */
    mask = mask ^ 0xffffffff;
    ipaddr = ntohl(original) & mask;

    /* merge the network portion and ip portions */
    result = network ^ ipaddr;

    /* return the result in network byte order */
    return (htonl(result));
}

static int
remap_ipv6(tcpedit_t *tcpedit, tcpr_cidr_t *cidr, struct tcpr_in6_addr *addr)
{
    uint32_t i, j, k;

    assert(tcpedit);
    assert(cidr);

    if (cidr->family != AF_INET6) {
        return 0;
    }

    /* don't rewrite broadcast addresses */
    if (tcpedit->skip_broadcast && is_multicast_ipv6(tcpedit, addr))
        return 0;

    j = cidr->masklen / 8;

    for (i = 0; i < j; i++)
        addr->tcpr_s6_addr[i] = cidr->u.network6.tcpr_s6_addr[i];

    if ((k = cidr->masklen % 8) == 0)
        return 1;

    k = (uint32_t)~0 << (8 - k);
    i = addr->tcpr_s6_addr[i] & k;

    addr->tcpr_s6_addr[i] =
            (cidr->u.network6.tcpr_s6_addr[j] & (0xff << (8 - k))) | (addr->tcpr_s6_addr[i] & (0xff >> k));

    return 1;
}

/**
 * rewrite IP address (layer3)
 * uses -N to rewrite (map) one subnet onto another subnet
 * also support --srcipmap and --dstipmap
 * return 0 if no change, 1 or 2 if changed
 */
int
rewrite_ipv4l3(tcpedit_t *tcpedit, ipv4_hdr_t *ip_hdr, tcpr_dir_t direction, int len)
{
    tcpr_cidrmap_t *cidrmap1 = NULL, *cidrmap2 = NULL;
    int didsrc = 0, diddst = 0, loop = 1;
    tcpr_cidrmap_t *ipmap;

    assert(tcpedit);
    assert(ip_hdr);

    /* first check the src/dst IP maps */
    ipmap = tcpedit->srcipmap;
    while (ipmap != NULL) {
        if (ip_in_cidr(ipmap->from, ip_hdr->ip_src.s_addr)) {
            uint32_t old_ip = ip_hdr->ip_src.s_addr;
            ip_hdr->ip_src.s_addr = remap_ipv4(tcpedit, ipmap->to, ip_hdr->ip_src.s_addr);
            ipv4_addr_csum_replace(ip_hdr, old_ip, ip_hdr->ip_src.s_addr, len);
            dbgx(2, "Remapped src addr to: %s", get_addr2name4(ip_hdr->ip_src.s_addr, RESOLVE));
            break;
        }
        ipmap = ipmap->next;
    }

    ipmap = tcpedit->dstipmap;
    while (ipmap != NULL) {
        if (ip_in_cidr(ipmap->from, ip_hdr->ip_dst.s_addr)) {
            uint32_t old_ip = ip_hdr->ip_dst.s_addr;
            ip_hdr->ip_dst.s_addr = remap_ipv4(tcpedit, ipmap->to, ip_hdr->ip_dst.s_addr);
            ipv4_addr_csum_replace(ip_hdr, old_ip, ip_hdr->ip_dst.s_addr, len);
            dbgx(2, "Remapped dst addr to: %s", get_addr2name4(ip_hdr->ip_dst.s_addr, RESOLVE));
            break;
        }
        ipmap = ipmap->next;
    }

    /* anything else to rewrite? */
    if (tcpedit->cidrmap1 == NULL)
        return (0);

    /* don't play with the main pointers */
    if (direction == TCPR_DIR_C2S) {
        cidrmap1 = tcpedit->cidrmap1;
        cidrmap2 = tcpedit->cidrmap2;
    } else {
        cidrmap1 = tcpedit->cidrmap2;
        cidrmap2 = tcpedit->cidrmap1;
    }

    /* loop through the cidrmap to rewrite */
    do {
        if ((!diddst) && ip_in_cidr(cidrmap2->from, ip_hdr->ip_dst.s_addr)) {
            uint32_t old_ip = ip_hdr->ip_dst.s_addr;
            ip_hdr->ip_dst.s_addr = remap_ipv4(tcpedit, cidrmap2->to, ip_hdr->ip_dst.s_addr);
            ipv4_addr_csum_replace(ip_hdr, old_ip, ip_hdr->ip_dst.s_addr, len);
            dbgx(2, "Remapped dst addr to: %s", get_addr2name4(ip_hdr->ip_dst.s_addr, RESOLVE));
            diddst = 1;
        }
        if ((!didsrc) && ip_in_cidr(cidrmap1->from, ip_hdr->ip_src.s_addr)) {
            uint32_t old_ip = ip_hdr->ip_src.s_addr;
            ip_hdr->ip_src.s_addr = remap_ipv4(tcpedit, cidrmap1->to, ip_hdr->ip_src.s_addr);
            ipv4_addr_csum_replace(ip_hdr, old_ip, ip_hdr->ip_src.s_addr, len);
            dbgx(2, "Remapped src addr to: %s", get_addr2name4(ip_hdr->ip_src.s_addr, RESOLVE));
            didsrc = 1;
        }

        /*
         * loop while we haven't modified both src/dst AND
         * at least one of the cidr maps have a next pointer
         */
        if ((!(diddst && didsrc)) && (!((cidrmap1->next == NULL) && (cidrmap2->next == NULL)))) {
            /* increment our ptr's if possible */
            if (cidrmap1->next != NULL)
                cidrmap1 = cidrmap1->next;

            if (cidrmap2->next != NULL)
                cidrmap2 = cidrmap2->next;

        } else {
            loop = 0;
        }

        /* Later on we should support various IP protocols which embed
         * the IP address in the application layer.  Things like
         * DNS and FTP.
         */

    } while (loop);

    /* return how many changes we require checksum updates
     * (none required - checksum is already updated)
     */
    return 0;
}

int
rewrite_ipv6l3(tcpedit_t *tcpedit, ipv6_hdr_t *ip6_hdr, tcpr_dir_t direction, int l3len)
{
    tcpr_cidrmap_t *cidrmap1 = NULL, *cidrmap2 = NULL;
    int didsrc = 0, diddst = 0, loop = 1;
    tcpr_cidrmap_t *ipmap;

    assert(tcpedit);
    assert(ip6_hdr);

    /* first check the src/dst IP maps */
    ipmap = tcpedit->srcipmap;
    while (ipmap != NULL) {
        if (ip6_in_cidr(ipmap->from, &ip6_hdr->ip_src)) {
            struct tcpr_in6_addr old_ip6;
            memcpy(&old_ip6, &ip6_hdr->ip_src, sizeof(old_ip6));
            remap_ipv6(tcpedit, ipmap->to, &ip6_hdr->ip_src);
            ipv6_addr_csum_replace(ip6_hdr, &old_ip6, &ip6_hdr->ip_src, l3len);
            dbgx(2, "Remapped src addr to: %s", get_addr2name6(&ip6_hdr->ip_src, RESOLVE));
            break;
        }
        ipmap = ipmap->next;
    }

    ipmap = tcpedit->dstipmap;
    while (ipmap != NULL) {
        if (ip6_in_cidr(ipmap->from, &ip6_hdr->ip_dst)) {
            struct tcpr_in6_addr old_ip6;
            memcpy(&old_ip6, &ip6_hdr->ip_dst, sizeof(old_ip6));
            remap_ipv6(tcpedit, ipmap->to, &ip6_hdr->ip_dst);
            ipv6_addr_csum_replace(ip6_hdr, &old_ip6, &ip6_hdr->ip_dst, l3len);
            dbgx(2, "Remapped dst addr to: %s", get_addr2name6(&ip6_hdr->ip_dst, RESOLVE));
            break;
        }
        ipmap = ipmap->next;
    }

    /* anything else to rewrite? */
    if (tcpedit->cidrmap1 == NULL)
        return (0);

    /* don't play with the main pointers */
    if (direction == TCPR_DIR_C2S) {
        cidrmap1 = tcpedit->cidrmap1;
        cidrmap2 = tcpedit->cidrmap2;
    } else {
        cidrmap1 = tcpedit->cidrmap2;
        cidrmap2 = tcpedit->cidrmap1;
    }

    /* loop through the cidrmap to rewrite */
    do {
        if ((!diddst) && ip6_in_cidr(cidrmap2->from, &ip6_hdr->ip_dst)) {
            struct tcpr_in6_addr old_ip6;
            memcpy(&old_ip6, &ip6_hdr->ip_dst, sizeof(old_ip6));
            remap_ipv6(tcpedit, cidrmap2->to, &ip6_hdr->ip_dst);
            ipv6_addr_csum_replace(ip6_hdr, &old_ip6, &ip6_hdr->ip_dst, l3len);
            dbgx(2, "Remapped dst addr to: %s", get_addr2name6(&ip6_hdr->ip_dst, RESOLVE));
            diddst = 1;
        }
        if ((!didsrc) && ip6_in_cidr(cidrmap1->from, &ip6_hdr->ip_src)) {
            struct tcpr_in6_addr old_ip6;
            memcpy(&old_ip6, &ip6_hdr->ip_src, sizeof(old_ip6));
            remap_ipv6(tcpedit, cidrmap1->to, &ip6_hdr->ip_src);
            ipv6_addr_csum_replace(ip6_hdr, &old_ip6, &ip6_hdr->ip_src, l3len);
            dbgx(2, "Remapped src addr to: %s", get_addr2name6(&ip6_hdr->ip_src, RESOLVE));
            didsrc = 1;
        }

        /*
         * loop while we haven't modified both src/dst AND
         * at least one of the cidr maps have a next pointer
         */
        if ((!(diddst && didsrc)) && (!((cidrmap1->next == NULL) && (cidrmap2->next == NULL)))) {
            /* increment our ptr's if possible */
            if (cidrmap1->next != NULL)
                cidrmap1 = cidrmap1->next;

            if (cidrmap2->next != NULL)
                cidrmap2 = cidrmap2->next;

        } else {
            loop = 0;
        }

        /* Later on we should support various IP protocols which embed
         * the IP address in the application layer.  Things like
         * DNS and FTP.
         */

    } while (loop);

    /* return how many changes we require checksum updates
     * (none required - checksum is already updated)
     */
    return 0;
}

/**
 * Randomize the IP addresses in an ARP packet based on the user seed
 * return 0 if no change, or 1 for a change
 */
int
randomize_iparp(tcpedit_t *tcpedit, struct pcap_pkthdr *pkthdr, const u_char *pktdata, int datalink, int l3len)
{
    arp_hdr_t *arp_hdr;
    int l2len;
#ifdef FORCE_ALIGN
    uint32_t iptemp;
#endif

    assert(tcpedit);
    assert(pkthdr);
    assert(pktdata);

    if (l3len < (int)sizeof(arp_hdr_t)) {
        tcpedit_seterr(tcpedit, "Unable to randomize ARP packet due to packet capture snap length %u", pkthdr->caplen);
        return TCPEDIT_ERROR;
    }

    l2len = get_l2len(pktdata, (int)pkthdr->caplen, datalink);

    arp_hdr = (arp_hdr_t *)(pktdata + l2len);

    /*
     * only rewrite IP addresses from REPLY/REQUEST's
     */
    if ((ntohs(arp_hdr->ar_pro) == ETHERTYPE_IP) &&
        ((ntohs(arp_hdr->ar_op) == ARPOP_REQUEST) || (ntohs(arp_hdr->ar_op) == ARPOP_REPLY))) {
        /* jump to the addresses */
        uint32_t *ip;
        u_char *add_hdr = ((u_char *)arp_hdr) + sizeof(arp_hdr_t) + arp_hdr->ar_hln;

#ifdef FORCE_ALIGN
        /* copy IP to a temporary buffer for processing */
        memcpy(&iptemp, add_hdr, sizeof(uint32_t));
        ip = &iptemp;
#else
        ip = (uint32_t *)add_hdr;
#endif
        *ip = randomize_ipv4_addr(tcpedit, *ip);
#ifdef FORCE_ALIGN
        memcpy(add_hdr, &iptemp, sizeof(uint32_t));
#endif

        add_hdr += arp_hdr->ar_pln + arp_hdr->ar_hln;
#ifdef FORCE_ALIGN
        /* copy IP2 to a temporary buffer for processing */
        memcpy(&iptemp, add_hdr, sizeof(uint32_t));
        ip = &iptemp;
#else
        ip = (uint32_t *)add_hdr;
#endif
        *ip = randomize_ipv4_addr(tcpedit, *ip);
#ifdef FORCE_ALIGN
        memcpy(add_hdr, &iptemp, sizeof(uint32_t));
#endif
    }

    return 1; /* yes we changed the packet */
}

/**
 * rewrite IP address (arp)
 * uses -a to rewrite (map) one subnet onto another subnet
 * pointer must point to the WHOLE and CONTIGUOUS memory buffer
 * because the arp_hdr_t doesn't have the space for the IP/MAC
 * addresses
 * return 0 if no change, 1 or 2 if changed
 */
int
rewrite_iparp(tcpedit_t *tcpedit, arp_hdr_t *arp_hdr, int cache_mode)
{
    u_char *add_hdr = NULL;
    uint32_t *ip1 = NULL, *ip2 = NULL;
    uint32_t newip = 0;
    tcpr_cidrmap_t *cidrmap1 = NULL, *cidrmap2 = NULL;
    int didsrc = 0, diddst = 0, loop = 1;
#ifdef FORCE_ALIGN
    uint32_t iptemp;
#endif

    assert(tcpedit);
    assert(arp_hdr);

    /* figure out what mapping to use */
    if (cache_mode == TCPR_DIR_C2S) {
        cidrmap1 = tcpedit->cidrmap1;
        cidrmap2 = tcpedit->cidrmap2;
    } else if (cache_mode == TCPR_DIR_S2C) {
        cidrmap1 = tcpedit->cidrmap2;
        cidrmap2 = tcpedit->cidrmap1;
    }

    /* anything to rewrite? */
    if (cidrmap1 == NULL || cidrmap2 == NULL)
        return (0);

    /*
     * must be IPv4 and request or reply
     * Do other op codes use the same subheader stub?
     * If so we won't need to check the op code.
     */
    if ((ntohs(arp_hdr->ar_pro) == ETHERTYPE_IP) &&
        ((ntohs(arp_hdr->ar_op) == ARPOP_REQUEST) || (ntohs(arp_hdr->ar_op) == ARPOP_REPLY))) {
        /* jump to the addresses */
        add_hdr = (u_char *)arp_hdr;
        add_hdr += sizeof(arp_hdr_t) + arp_hdr->ar_hln;
        ip1 = (uint32_t *)add_hdr;
        add_hdr += arp_hdr->ar_pln + arp_hdr->ar_hln;
#ifdef FORCE_ALIGN
        /* copy IP2 to a temporary buffer for processing */
        memcpy(&iptemp, add_hdr, sizeof(uint32_t));
        ip2 = &iptemp;
#else
        ip2 = (uint32_t *)add_hdr;
#endif

        /* loop through the cidrmap to rewrite */
        do {
            /* arp request ? */
            if (ntohs(arp_hdr->ar_op) == ARPOP_REQUEST) {
                if ((!diddst) && ip_in_cidr(cidrmap2->from, *ip1)) {
                    newip = remap_ipv4(tcpedit, cidrmap2->to, *ip1);
                    memcpy(ip1, &newip, 4);
                    diddst = 1;
                }
                if ((!didsrc) && ip_in_cidr(cidrmap1->from, *ip2)) {
                    newip = remap_ipv4(tcpedit, cidrmap1->to, *ip2);
                    memcpy(ip2, &newip, 4);
                    didsrc = 1;
                }
            }
            /* else it's an arp reply */
            else {
                if ((!diddst) && ip_in_cidr(cidrmap2->from, *ip2)) {
                    newip = remap_ipv4(tcpedit, cidrmap2->to, *ip2);
                    memcpy(ip2, &newip, 4);
                    diddst = 1;
                }
                if ((!didsrc) && ip_in_cidr(cidrmap1->from, *ip1)) {
                    newip = remap_ipv4(tcpedit, cidrmap1->to, *ip1);
                    memcpy(ip1, &newip, 4);
                    didsrc = 1;
                }
            }

#ifdef FORCE_ALIGN
            /* copy temporary IP to IP2 location in buffer */
            memcpy(add_hdr, &iptemp, sizeof(uint32_t));
#endif

            /*
             * loop while we haven't modified both src/dst AND
             * at least one of the cidr maps have a next pointer
             */
            if ((!(diddst && didsrc)) && (!((cidrmap1->next == NULL) && (cidrmap2->next == NULL)))) {
                /* increment our ptr's if possible */
                if (cidrmap1->next != NULL)
                    cidrmap1 = cidrmap1->next;

                if (cidrmap2->next != NULL)
                    cidrmap2 = cidrmap2->next;

            } else {
                loop = 0;
            }

        } while (loop);

    } else {
        warn("ARP packet isn't for IPv4!  Can't rewrite IP's");
    }

    return (didsrc + diddst);
}

/**
 * returns 1 if the IP address is a unicast address, otherwise, returns 0
 * for broadcast/multicast addresses.  Returns -1 on error
 */
static int
is_unicast_ipv4(tcpedit_t *tcpedit, uint32_t ip)
{
    assert(tcpedit);

    /* multicast/broadcast is 224.0.0.0 to 239.255.255.255 */
    if ((ntohl(ip) & 0xf0000000) == 0xe0000000)
        return 0;

    return 1;
}

/**
 * returns 1 if the IPv6 address is a multicast address, otherwise, returns 0
 * for unicast/anycast addresses.  Returns -1 on error
 */
static int
is_multicast_ipv6(tcpedit_t *tcpedit, struct tcpr_in6_addr *addr)
{
    assert(tcpedit);

    if (addr->tcpr_s6_addr[0] == 0xff)
        return 1;

    return 0;
}
