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

#include "flows.h"
#include "tcpreplay_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JUNIPER_FLAG_NO_L2          0x02     /* L2 header */
#define JUNIPER_FLAG_EXT            0x80     /* Juniper extensions present */
#define JUNIPER_PCAP_MAGIC          "MGC"

/* 5-tuple plus VLAN ID */
typedef struct flow_entry_data {
    union {
        struct in_addr in;
        struct in6_addr in6;
    } src_ip;

    union {
        struct in_addr in;
        struct in6_addr in6;
    } dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t vlan;
    uint8_t protocol;
} flow_entry_data_t;

typedef struct flow_hash_entry {
    uint32_t key;
    flow_entry_data_t data;
    struct timeval ts_last_seen;
    struct flow_hash_entry *next;
} flow_hash_entry_t;

struct flow_hash_table {
    size_t num_buckets;
    flow_hash_entry_t **buckets;
};

static bool is_power_of_2(size_t n)
{
    return (n != 0 && ((n & (n - 1)) == 0));
}

/*
 * Perl's hash function
 *
 * We do extensive hashing to prevent hash table collisions.
 * It will save time in the long run.
 */
static inline uint32_t hash_func(const void *key, size_t length)
{
    register size_t i = length;
    register uint32_t hv = 0;
    register const u_char *s = (u_char *)key;
    while (i--) {
        hv += *s++;
        hv += (hv << 10);
        hv ^= (hv >> 6);
    }
    hv += (hv << 3);
    hv ^= (hv >> 11);
    hv += (hv << 15);

    return hv;
}

/*
 * add hash value to hash table bucket
 */
static inline flow_hash_entry_t *hash_add_entry(flow_hash_table_t *fht, const uint32_t hv,
        const uint32_t key, const flow_entry_data_t *hash_entry)
{
    flow_hash_entry_t *he;

    assert(hv < fht->num_buckets);

    he = safe_malloc(sizeof (*he));
    if (!he) {
        warn("out of memory");
        return NULL;
    }

    he->key = key;
    he->next = fht->buckets[hv];
    fht->buckets[hv] = he;
    memcpy(&he->data, hash_entry, sizeof(he->data));

    return he;
}

/*
 * Search for this entry in the hash table and
 * insert it if not found. Report whether this
 * is a new, existing or expired flow.
 *
 * Only check for expiry if 'expiry' is set
 */
static inline flow_entry_type_t hash_put_data(flow_hash_table_t *fht, const uint32_t key,
        const flow_entry_data_t *hash_entry, const struct timeval *tv, const int expiry)
{
    uint32_t hash_value = key & (fht->num_buckets - 1);
    flow_hash_entry_t *he;
    flow_entry_type_t res;

    for (he = fht->buckets[hash_value]; he; he = he->next) {
        /*
         * found an existing entry with similar hash. double
         * check to see if it is our flow or just a collision
         */
        if (he->key == key && !memcmp(&he->data, hash_entry, sizeof(he->data)))
            break;
    }

    if (he) {
        /* this is not a new flow */
        if (expiry && tv->tv_sec > (expiry + he->ts_last_seen.tv_sec))
            res = FLOW_ENTRY_EXPIRED;
        else
            res = FLOW_ENTRY_EXISTING;

        if (expiry)
            TIMEVAL_SET(&he->ts_last_seen, tv);
    } else {
        /* this is a new flow */
        if ((he = hash_add_entry(fht, hash_value, key, hash_entry)) != NULL) {
            res = FLOW_ENTRY_NEW;

            if (expiry)
                TIMEVAL_SET(&he->ts_last_seen, tv);
        } else
            res = FLOW_ENTRY_INVALID;
    }

    dbgx(2, "flow type=%d", (int)res);
    return res;
}

/*
 * Decode the packet, study it's flow status and report
 */
flow_entry_type_t flow_decode(flow_hash_table_t *fht, const struct pcap_pkthdr *pkthdr,
        const u_char *pktdata, const int datalink, const int expiry)
{
    uint32_t pkt_len = pkthdr->caplen;
    const u_char *packet = pktdata;
    uint32_t _U_ vlan_offset;
    uint32_t _U_ l2offset;
    uint16_t ether_type = 0;
    ipv4_hdr_t *ip_hdr = NULL;
    ipv6_hdr_t *ip6_hdr = NULL;
    tcp_hdr_t *tcp_hdr;
    udp_hdr_t *udp_hdr;
    icmpv4_hdr_t *icmp_hdr;
    flow_entry_data_t entry;
    uint32_t l2len = 0;
    uint8_t protocol;
    uint32_t hash;
    int ip_len;
    int res;

    assert(fht);
    assert(packet);

    /*
     * extract the 5-tuple and populate the entry data
     */

    memset(&entry, 0, sizeof(entry));

    res = get_l2len_protocol(packet,
                             pkt_len,
                             datalink,
                             &ether_type,
                             &l2len,
                             &l2offset,
                             &vlan_offset);

    if (res == -1) {
        warnx("Unable to process unsupported DLT type: %s (0x%x)",
              pcap_datalink_val_to_description(datalink), datalink);
        return FLOW_ENTRY_INVALID;
    }

    if (ether_type == ETHERTYPE_IP) {
        if (pkt_len < l2len + sizeof(ipv4_hdr_t))
                return FLOW_ENTRY_INVALID;

        ip_hdr = (ipv4_hdr_t *)(packet + l2len);

        if (ip_hdr->ip_v != 4)
            return FLOW_ENTRY_NON_IP;

        ip_len = ip_hdr->ip_hl * 4;
        protocol = ip_hdr->ip_p;
        entry.src_ip.in = ip_hdr->ip_src;
        entry.dst_ip.in = ip_hdr->ip_dst;
    } else if (ether_type == ETHERTYPE_IP6) {
        if (pkt_len < l2len + sizeof(ipv6_hdr_t))
                return FLOW_ENTRY_INVALID;

        if ((packet[0] >> 4) != 6)
            return FLOW_ENTRY_NON_IP;

        ip6_hdr = (ipv6_hdr_t *)(packet + l2len);
        ip_len = sizeof(*ip6_hdr);
        protocol = ip6_hdr->ip_nh;

        if (protocol == 0) {
            struct tcpr_ipv6_ext_hdr_base *ext = (struct tcpr_ipv6_ext_hdr_base *)(ip6_hdr + 1);
            ip_len += (ext->ip_len + 1) * 8;
            protocol = ext->ip_nh;
        }
        memcpy(&entry.src_ip.in6, &ip6_hdr->ip_src, sizeof(entry.src_ip.in6));
        memcpy(&entry.dst_ip.in6, &ip6_hdr->ip_dst, sizeof(entry.dst_ip.in6));
    } else {
        return FLOW_ENTRY_NON_IP;
    }

    entry.protocol = protocol;

    switch (protocol) {
    case IPPROTO_UDP:
        if (pkt_len < (l2len + ip_len + sizeof(udp_hdr_t)))
            return FLOW_ENTRY_INVALID;
        udp_hdr = (udp_hdr_t*)(packet + ip_len + l2len);
        entry.src_port = udp_hdr->uh_sport;
        entry.dst_port = udp_hdr->uh_dport;
        break;

    case IPPROTO_TCP:
        if (pkt_len < (l2len + ip_len + sizeof(tcp_hdr_t)))
            return FLOW_ENTRY_INVALID;
        tcp_hdr = (tcp_hdr_t*)(packet + ip_len + l2len);
        entry.src_port = tcp_hdr->th_sport;
        entry.dst_port = tcp_hdr->th_dport;
        break;

    case IPPROTO_ICMP:
    case IPPROTO_ICMPV6:
        if (pkt_len < (l2len + ip_len + sizeof(icmpv4_hdr_t)))
            return FLOW_ENTRY_INVALID;
        icmp_hdr = (icmpv4_hdr_t*)(packet + ip_len + l2len);
        entry.src_port = icmp_hdr->icmp_type;
        entry.dst_port = icmp_hdr->icmp_code;
        break;
    default:
        entry.src_port = 0;
        entry.dst_port = 0;
    }

    /* hash the 5-tuple */
    hash = hash_func(&entry, sizeof(entry));

    return hash_put_data(fht, hash, &entry, &pkthdr->ts, expiry);
}

static void flow_cache_clear(flow_hash_table_t *fht)
{
    flow_hash_entry_t *fhe = NULL;
    flow_hash_entry_t *fhe_next = NULL;
    size_t i;

    for (i = 0; i < fht->num_buckets; i++) {
        if ((fhe = fht->buckets[i]) != NULL) {
            while (fhe) {
                fhe_next = fhe->next;
                safe_free(fhe);
                fhe = fhe_next;
            }
            fht->buckets[i] = NULL;
        }
    }
}

flow_hash_table_t *flow_hash_table_init(size_t n)
{
    flow_hash_table_t *fht;
    if (!is_power_of_2(n))
        errx(-1, "invalid table size: %zu", n);

    fht = safe_malloc(sizeof(*fht));
    fht->num_buckets = n;
    fht->buckets = safe_malloc(sizeof(flow_hash_entry_t) * n);

    return fht;
}

void flow_hash_table_release(flow_hash_table_t *fht)
{
    if (!fht)
        return;

    flow_cache_clear(fht);
    safe_free(fht->buckets);
    safe_free(fht);
}
