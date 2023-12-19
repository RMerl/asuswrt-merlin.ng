/*
 * mod_print.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "config.h"
#include "mod.h"
#include "pkt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif

#define EXTRACT_16BITS(p) ((uint16_t)ntohs(*(uint16_t *)(p)))
#define EXTRACT_32BITS(p) ((uint32_t)ntohl(*(uint32_t *)(p)))

/* XXX - _print_* routines adapted from tcpdump */

static void
print_icmp(u_char *p, _U_ int length)
{
    struct ip_hdr *ip;
    struct icmp_hdr *icmp;

    ip = (struct ip_hdr *)p;
    icmp = (struct icmp_hdr *)(p + (ip->ip_hl * 4));

    /* XXX - truncation? */
    printf("%s > %s:", ip_ntoa(&ip->ip_src), ip_ntoa(&ip->ip_dst));
    printf(" icmp: type %d code %d", icmp->icmp_type, icmp->icmp_code);
}

static void
print_icmp6(u_char *p, _U_ int length)
{
    struct ip6_hdr *ip6;
    struct icmp_hdr *icmp;

    ip6 = (struct ip6_hdr *)p;
    icmp = (struct icmp_hdr *)(p + IP6_HDR_LEN);

    /* XXX - truncation? */
    printf("%s > %s:", ip6_ntoa(&ip6->ip6_src), ip6_ntoa(&ip6->ip6_dst));
    printf(" icmp: type %hhu code %hhu", icmp->icmp_type, icmp->icmp_code);
}

void
print_tcp(int family, unsigned char *p, int length)
{
    struct tcp_hdr *tcp;
    u_short sport, dport, win, urp;
    u_long seq, ack;
    int len, tcp_hl;
    register char ch;

    char src[INET6_ADDRSTRLEN], dst[INET6_ADDRSTRLEN];

    if (family == AF_INET6) {
        struct ip6_hdr *ip6 = (struct ip6_hdr *)p;
        tcp = (struct tcp_hdr *)(p + IP6_HDR_LEN);
        len = length;

        ip6_ntop(&ip6->ip6_src, src, sizeof(src));
        ip6_ntop(&ip6->ip6_dst, dst, sizeof(dst));
    } else {
        struct ip_hdr *ip;

        ip = (struct ip_hdr *)p;
        tcp = (struct tcp_hdr *)(p + (ip->ip_hl * 4));
        len = length - (ip->ip_hl * 4);

        ip_ntop(&ip->ip_src, src, sizeof(src));
        ip_ntop(&ip->ip_dst, dst, sizeof(dst));
    }

    if (len < TCP_HDR_LEN) {
        printf("truncated-tcp %d", len);
        return;
    }
    sport = ntohs(tcp->th_sport);
    dport = ntohs(tcp->th_dport);
    seq = ntohl(tcp->th_seq);
    ack = ntohl(tcp->th_ack);
    win = ntohs(tcp->th_win);
    urp = ntohs(tcp->th_urp);
    tcp_hl = tcp->th_off * 4;

    printf("%s.%d > %s.%d: ", src, sport, dst, dport);

    if (tcp->th_flags & (TH_SYN | TH_FIN | TH_RST | TH_PUSH)) {
        if (tcp->th_flags & TH_SYN)
            putchar('S');
        if (tcp->th_flags & TH_FIN)
            putchar('F');
        if (tcp->th_flags & TH_RST)
            putchar('R');
        if (tcp->th_flags & TH_PUSH)
            putchar('P');
    } else
        putchar('.');

    if (tcp_hl > len) {
        printf(" [bad hdr length]");
        return;
    }
    len -= tcp_hl;

    if (len > 0 || tcp->th_flags & (TH_SYN | TH_FIN | TH_RST))
        printf(" %lu:%lu(%d)", seq, seq + len, len);

    if (tcp->th_flags & TH_ACK)
        printf(" ack %lu", ack);
    printf(" win %d", win);
    if (tcp->th_flags & TH_URG)
        printf(" urg %d", urp);

    /* Handle options. */
    if ((tcp_hl -= TCP_HDR_LEN) > 0) {
        register const u_char *cp;
        register int i, opt, len, datalen;

        cp = (const u_char *)tcp + TCP_HDR_LEN;
        putchar(' ');
        ch = '<';

        while (tcp_hl > 0) {
            putchar(ch);
            opt = *cp++;
            if (TCP_OPT_TYPEONLY(opt)) {
                len = 1;
            } else {
                len = *cp++; /* total including type, len */
                if (len < 2 || len > tcp_hl)
                    goto bad;
                --tcp_hl; /* account for length byte */
            }
            --tcp_hl; /* account for type byte */
            datalen = 0;

/* Bail if "l" bytes of data are not left or were not captured  */
#define LENCHECK(l)                                                                                                    \
    {                                                                                                                  \
        if ((l) > tcp_hl)                                                                                              \
            goto bad;                                                                                                  \
    }

            switch (opt) {
            case TCP_OPT_MSS:
                printf("mss");
                datalen = 2;
                LENCHECK(datalen);
                printf(" %u", EXTRACT_16BITS(cp));
                break;
            case TCP_OPT_EOL:
                printf("eol");
                break;
            case TCP_OPT_NOP:
                printf("nop");
                break;
            case TCP_OPT_WSCALE:
                printf("wscale");
                datalen = 1;
                LENCHECK(datalen);
                printf(" %u", *cp);
                break;
            case TCP_OPT_SACKOK:
                printf("sackOK");
                if (len != 2)
                    printf("[len %d]", len);
                break;
            case TCP_OPT_SACK:
                datalen = len - 2;
                if ((datalen % 8) != 0 || !(tcp->th_flags & TH_ACK)) {
                    printf("malformed sack ");
                    printf("[len %d] ", datalen);
                    break;
                }
                printf("sack %d ", datalen / 8);
                break;
            case TCP_OPT_ECHO:
                printf("echo");
                datalen = 4;
                LENCHECK(datalen);
                printf(" %u", EXTRACT_32BITS(cp));
                break;
            case TCP_OPT_ECHOREPLY:
                printf("echoreply");
                datalen = 4;
                LENCHECK(datalen);
                printf(" %u", EXTRACT_32BITS(cp));
                break;
            case TCP_OPT_TIMESTAMP:
                printf("timestamp");
                datalen = 8;
                LENCHECK(4);
                printf(" %u", EXTRACT_32BITS(cp));
                LENCHECK(datalen);
                printf(" %u", EXTRACT_32BITS(cp + 4));
                break;
            case TCP_OPT_CC:
                printf("cc");
                datalen = 4;
                LENCHECK(datalen);
                printf(" %u", EXTRACT_32BITS(cp));
                break;
            case TCP_OPT_CCNEW:
                printf("ccnew");
                datalen = 4;
                LENCHECK(datalen);
                printf(" %u", EXTRACT_32BITS(cp));
                break;
            case TCP_OPT_CCECHO:
                printf("ccecho");
                datalen = 4;
                LENCHECK(datalen);
                printf(" %u", EXTRACT_32BITS(cp));
                break;
            default:
                printf("opt-%d:", opt);
                datalen = len - 2;
                for (i = 0; i < datalen; ++i) {
                    LENCHECK(i);
                    printf("%02x", cp[i]);
                }
                break;
            }
            /* Account for data printed */
            cp += datalen;
            tcp_hl -= datalen;

            /* Check specification against observed length */
            ++datalen; /* option octet */
            if (!TCP_OPT_TYPEONLY(opt))
                ++datalen; /* size octet */
            if (datalen != len)
                printf("[len %d]", len);
            ch = ',';
            if (opt == TCP_OPT_EOL)
                break;
        }
        putchar('>');
    }
    return;
bad:
    fputs("[bad opt]", stdout);
    if (ch != '\0')
        putchar('>');
    return;
}

static void
print_udp(int family, u_char *p, _U_ int length)
{
    struct udp_hdr *udp;
    char src[INET6_ADDRSTRLEN], dst[INET6_ADDRSTRLEN];

    if (family == AF_INET6) {
        struct ip6_hdr *ip6 = (struct ip6_hdr *)p;
        udp = (struct udp_hdr *)(p + IP6_HDR_LEN);

        ip6_ntop(&ip6->ip6_src, src, sizeof(src));
        ip6_ntop(&ip6->ip6_dst, dst, sizeof(dst));
    } else {
        struct ip_hdr *ip;

        ip = (struct ip_hdr *)p;
        udp = (struct udp_hdr *)(p + (ip->ip_hl * 4));

        ip_ntop(&ip->ip_src, src, sizeof(src));
        ip_ntop(&ip->ip_dst, dst, sizeof(dst));
    }

    /* XXX - truncation? */
    printf("%s.%d > %s.%d:", src, ntohs(udp->uh_sport), dst, ntohs(udp->uh_dport));
    printf(" udp %d", ntohs(udp->uh_ulen) - UDP_HDR_LEN);
}

static void
print_frag6(u_char *p, _U_ int length)
{
    struct ip6_hdr *ip6;
    struct ip6_ext_hdr *ext;
    int off;

    ip6 = (struct ip6_hdr *)p;
    ext = (struct ip6_ext_hdr *)(p + IP6_HDR_LEN);

    off = htons(ext->ext_data.fragment.offlg & IP6_OFF_MASK);

    printf("%s > %s:", ip6_ntoa(&ip6->ip6_src), ip6_ntoa(&ip6->ip6_dst));
    printf(" fragment: next %hhu offset %d%s ident 0x%08x",
           ext->ext_nxt,
           off,
           (ext->ext_data.fragment.offlg & IP6_MORE_FRAG) ? " MF" : "",
           htonl(ext->ext_data.fragment.ident));
}

static void
print_ip(u_char *p, int length)
{
    struct ip_hdr *ip;
    int ip_off, ip_hl, ip_len;

    ip = (struct ip_hdr *)p;

    if (length < IP_HDR_LEN) {
        printf("truncated-ip %d", length);
        return;
    }
    ip_hl = ip->ip_hl * 4;
    ip_len = ntohs(ip->ip_len);

    if (length < ip_len) {
        printf("truncated-ip - %d bytes missing!", ip_len - length);
        return;
    }
    ip_off = ntohs(ip->ip_off);

    /* Handle first fragment. */
    if ((ip_off & IP_OFFMASK) == 0) {
        switch (ip->ip_p) {
        case IP_PROTO_TCP:
            print_tcp(AF_INET, p, ip_len);
            break;
        case IP_PROTO_UDP:
            print_udp(AF_INET, p, ip_len);
            break;
        case IP_PROTO_ICMP:
            print_icmp(p, ip_len);
            break;
        default:
            printf("%s > %s:", ip_ntoa(&ip->ip_src), ip_ntoa(&ip->ip_dst));
            printf(" ip-proto-%d %d", ip->ip_p, ip_len);
            break;
        }
    }
    /* Handle more frags. */
    if (ip_off & (IP_MF | IP_OFFMASK)) {
        if (ip_off & IP_OFFMASK)
            printf("%s > %s:", ip_ntoa(&ip->ip_src), ip_ntoa(&ip->ip_dst));
        printf(" (frag %d:%d@%d%s)",
               ntohs(ip->ip_id),
               ip_len - ip_hl,
               (ip_off & IP_OFFMASK) << 3,
               (ip_off & IP_MF) ? "+" : "");
    } else if (ip_off & IP_DF)
        printf(" (DF)");

    if (ip->ip_tos)
        printf(" [tos 0x%x]", ip->ip_tos);
    if (ip->ip_ttl <= 1)
        printf(" [ttl %d]", ip->ip_ttl);
}

static void
print_ip6(u_char *p, int length)
{
    struct ip6_hdr *ip6;
    int plen;

    ip6 = (struct ip6_hdr *)p;

    if (length < IP6_HDR_LEN) {
        printf("truncated-ip6 %d", length);
        return;
    }

    plen = htons(ip6->ip6_plen);

    switch (ip6->ip6_nxt) {
    case IP_PROTO_TCP:
        print_tcp(AF_INET6, p, plen);
        break;
    case IP_PROTO_UDP:
        print_udp(AF_INET6, p, plen);
        break;
    case IP_PROTO_ICMPV6:
        print_icmp6(p, plen);
        break;
    case IP_PROTO_FRAGMENT:
        print_frag6(p, plen);
        break;
    default:
        printf("%s > %s:", ip6_ntoa(&ip6->ip6_src), ip6_ntoa(&ip6->ip6_dst));
        printf(" ip-proto-%hhu ttl %hhu  payload len %d", ip6->ip6_nxt, ip6->ip6_hlim, plen);
        break;
    }

    if (ip6->ip6_hlim <= 1)
        printf(" [ttl %d]", ip6->ip6_hlim);
}

static void
print_eth(struct eth_hdr *e, int length)
{
    char d[20], s[20];
    eth_ntop(&e->eth_dst, &d[0], sizeof(d));
    eth_ntop(&e->eth_src, &s[0], sizeof(s));

    printf("%s > %s type 0x%04hx length %d", d, s, htons(e->eth_type), length);
}

static char *
timerntoa(struct timeval *tv)
{
    static char buf[128];
    uint64_t usec;

    usec = (tv->tv_sec * 1000000) + tv->tv_usec;

    snprintf(buf, sizeof(buf), "%d.%03d ms", (int)(usec / 1000), (int)(usec % 1000));

    return (buf);
}

int
print_apply(_U_ void *d, struct pktq *pktq)
{
    struct pkt *pkt;

    TAILQ_FOREACH(pkt, pktq, pkt_next)
    {
        uint16_t eth_type = htons(pkt->pkt_eth->eth_type);

        if (eth_type == ETH_TYPE_IP)
            print_ip(pkt->pkt_eth_data, pkt->pkt_end - pkt->pkt_eth_data);
        else if (eth_type == ETH_TYPE_IPV6)
            print_ip6(pkt->pkt_eth_data, pkt->pkt_end - pkt->pkt_eth_data);
        else
            print_eth(pkt->pkt_eth, pkt->pkt_end - pkt->pkt_data);
        if (timerisset(&pkt->pkt_ts))
            printf(" [delay %s]", timerntoa(&pkt->pkt_ts));
        printf("\n");
    }
    return (0);
}

struct mod mod_print = {
        "print",     /* name */
        "print",     /* usage */
        NULL,        /* init */
        print_apply, /* apply */
        NULL         /* close */
};
