#include "defines.h"
#include "config.h"
#include "common.h"
#ifdef HAVE_DNET_H
#include <dnet.h>
#endif
#ifdef HAVE_DUMBNET_H
#include <dumbnet.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <string.h>

static ssize_t inet_add_option_6(void *buf, size_t len, int proto, const void *optbuf, size_t optlen);

ssize_t
inet_add_option(uint16_t eth_type, void *buf, size_t len, int proto, const void *optbuf, size_t optlen)
{
    if (eth_type == ETH_TYPE_IP) {
        return ip_add_option(buf, len, proto, optbuf, optlen);
    } else if (eth_type == ETH_TYPE_IPV6) {
        return inet_add_option_6(buf, len, proto, optbuf, optlen);
    } else {
        errno = EINVAL;
        return -1;
    }
}

/*
 * IPv6 version of libdnet's ip_add_option():
 */
ssize_t
inet_add_option_6(void *buf, size_t len, int proto, const void *optbuf, size_t optlen)
{
    struct ip6_hdr *ip6;
    struct tcp_hdr *tcp = NULL;
    u_char *p;
    int hl, datalen, padlen;

    if (proto != IP_PROTO_TCP) {
        errno = EINVAL;
        return (-1);
    }

    ip6 = (struct ip6_hdr *)buf;
    p = (u_char *)buf + IP6_HDR_LEN;

    tcp = (struct tcp_hdr *)p;
    hl = tcp->th_off << 2;
    p = (u_char *)tcp + hl;

    datalen = ntohs(ip6->ip6_plen) + IP6_HDR_LEN - (int)(p - (u_char *)buf);

    /* Compute padding to next word boundary. */
    if ((padlen = 4 - (int)(optlen % 4)) == 4)
        padlen = 0;

    /* XXX - IP_HDR_LEN_MAX == TCP_HDR_LEN_MAX */
    if (hl + optlen + padlen > IP_HDR_LEN_MAX || ntohs(ip6->ip6_plen) + IP6_HDR_LEN + optlen + padlen > len) {
        errno = EINVAL;
        return (-1);
    }

    /* Shift any existing data. */
    if (datalen) {
        memmove(p + optlen + padlen, p, datalen);
    }
    /* XXX - IP_OPT_NOP == TCP_OPT_NOP */
    if (padlen) {
        memset(p, IP_OPT_NOP, padlen);
        p += padlen;
    }
    memmove(p, optbuf, optlen);
    p += optlen;
    optlen += padlen;

    tcp->th_off = (p - (u_char *)tcp) >> 2;

    ip6->ip6_plen = htons(ntohs(ip6->ip6_plen) + optlen);

    return (ssize_t)optlen;
}

void
inet_checksum(uint16_t eth_type, void *buf, size_t len)
{
    if (eth_type == ETH_TYPE_IP) {
        return ip_checksum(buf, len);
    } else if (eth_type == ETH_TYPE_IPV6) {
        return ip6_checksum(buf, len);
    }
}

int
raw_ip_opt_parse(int argc, char *argv[], uint8_t *opt_type, uint8_t *opt_len, uint8_t *buff, int buff_len)
{
    int i, j;

    if (sscanf(argv[0], "%hhx", opt_type) != 1) {
        warn("invalid opt_type");
        return -1;
    }
    if (sscanf(argv[1], "%hhx", opt_len) != 1) {
        warn("invalid opt_len");
        return -1;
    }
    j = 0;
    for (i = 2; i < argc && j < buff_len; ++i, ++j) {
        if (sscanf(argv[i], "%hhx", &buff[j]) != 1) {
            warn("invalid opt_data");
            return -1;
        }
    }
    if (*opt_len != j + 2) {
        warnx("invalid opt->len (%d) doesn't match data length (%d)", *opt_len, j);
        return -1;
    }
    return 0;
}

int
raw_ip6_opt_parse(int argc, char *argv[], uint8_t *proto, int *len, uint8_t *buff, int buff_len)
{
    int i, j;

    if (sscanf(argv[0], "%hhx", proto) != 1) {
        warn("invalid protocol");
        return -1;
    }

    j = 0;
    for (i = 1; i < argc && j < buff_len; ++i, ++j) {
        if (sscanf(argv[i], "%hhx", &buff[j]) != 1) {
            warn("invalid opt_data");
            return -1;
        }
    }
    *len = j;
    if ((j + 2) % 8 != 0) {
        warnx("(opt_len (%d) + 2) %% 8 != 0", j);
        return -1;
    }
    return 0;
}
