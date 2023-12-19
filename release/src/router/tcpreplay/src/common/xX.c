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

/*
 * xX stands for "include or exclude" which is used with the
 * -x and -X flags
 *
 * Functions for use to process args for or check data against in
 * tcpreplay/do_packets and tcpprep.
 */

#include "defines.h"
#include "config.h"
#include "common.h"
#include <stdlib.h>

/**
 * returns the include_exclude_mode on success placing the CIDR or LIST in mybuf
 * but on failure, returns xXError
 */
int
parse_xX_str(tcpr_xX_t *xX, char *str, tcpr_bpf_t *bpf)
{
    int out;

    dbgx(1, "Parsing string: %s", str);
    dbgx(1, "Switching on: %c", str[0]);

    switch (str[0]) {
    case 'B': /* both ip's */
        str = str + 2;
        out = xXBoth;
        if (!parse_cidr(&(xX->cidr), str, ","))
            return xXError;
        break;

    case 'D': /* dst ip */
        str = str + 2;
        out = xXDest;
        if (!parse_cidr(&(xX->cidr), str, ","))
            return xXError;
        break;

    case 'E': /* either ip */
        str = str + 2;
        out = xXEither;
        if (!parse_cidr(&(xX->cidr), str, ","))
            return xXError;
        break;

    case 'F': /* bpf filter */
        str = str + 2;
        out = xXBPF;
        bpf->filter = safe_strdup(str);
        /*
         * note: it's temping to compile the BPF here, but we don't
         * yet know what the link type is for the file, so we have
         * to compile the BPF once we open the pcap file
         */
        break;

    case 'P': /* packet id */
        str = str + 2;
        out = xXPacket;
        if (!parse_list(&(xX->list), str))
            return xXError;
        break;

    case 'S': /* source ip */
        str = str + 2;
        out = xXSource;
        if (!parse_cidr(&(xX->cidr), str, ","))
            return xXError;
        break;

    default:
        errx(-1, "Invalid -%c option: %c", xX->mode, *str);
    }

    if (xX->mode == 'X') { /* run in exclude mode */
        out += xXExclude;
        if (bpf->filter != NULL)
            err(-1,
                "Using a BPF filter with -X doesn't work.\n"
                "Try using -xF:\"not <filter>\" instead");
    }

    xX->mode = out;
    return xX->mode;
}

/**
 * compare the source/destination IP address according to the mode
 * and return 1 if we should send the packet or 0 if not
 */
int
process_xX_by_cidr_ipv4(int mode, tcpr_cidr_t *cidr, ipv4_hdr_t *ip_hdr)
{
    if (mode & xXExclude) {
        /* Exclude mode */
        switch (mode ^ xXExclude) {
        case xXSource:
            /* note: check_ip_cidr() returns TCPR_DIR_C2S for true, TCPR_DIR_S2C for false
             * and NOT true/false or 1/0, etc!
             */
            return check_ip_cidr(cidr, ip_hdr->ip_src.s_addr) ? DONT_SEND : SEND;
        case xXDest:
            return check_ip_cidr(cidr, ip_hdr->ip_dst.s_addr) ? DONT_SEND : SEND;

        case xXBoth:
            return (check_ip_cidr(cidr, ip_hdr->ip_dst.s_addr) && check_ip_cidr(cidr, ip_hdr->ip_src.s_addr))
                           ? DONT_SEND
                           : SEND;
        case xXEither:
            return (check_ip_cidr(cidr, ip_hdr->ip_dst.s_addr) || check_ip_cidr(cidr, ip_hdr->ip_src.s_addr))
                           ? DONT_SEND
                           : SEND;
        }
    } else {
        /* Include Mode */
        switch (mode) {
        case xXSource:
            return check_ip_cidr(cidr, ip_hdr->ip_src.s_addr) ? SEND : DONT_SEND;
        case xXDest:
            return check_ip_cidr(cidr, ip_hdr->ip_dst.s_addr) ? SEND : DONT_SEND;
        case xXBoth:
            return (check_ip_cidr(cidr, ip_hdr->ip_dst.s_addr) && check_ip_cidr(cidr, ip_hdr->ip_src.s_addr))
                           ? SEND
                           : DONT_SEND;
        case xXEither:
            return (check_ip_cidr(cidr, ip_hdr->ip_dst.s_addr) || check_ip_cidr(cidr, ip_hdr->ip_src.s_addr))
                           ? SEND
                           : DONT_SEND;
        default:
            assert(false);
        }
    }

    /* total failure */
    if (mode & xXExclude) {
        warn("Unable to determine action in CIDR filter mode.  Default: Don't Send.");
        return DONT_SEND;
    } else {
        warn("Unable to determine action in CIDR filter mode.  Default: Send.");
        return SEND;
    }
}

int
process_xX_by_cidr_ipv6(int mode, tcpr_cidr_t *cidr, ipv6_hdr_t *ip6_hdr)
{
    if (mode & xXExclude) {
        /* Exclude mode */
        switch (mode ^ xXExclude) {
        case xXSource:
            /* note: check_ip_cidr() returns TCPR_DIR_C2S for true, TCPR_DIR_S2C for false
             * and NOT true/false or 1/0, etc!
             */
            return check_ip6_cidr(cidr, &ip6_hdr->ip_src) ? DONT_SEND : SEND;
        case xXDest:
            return check_ip6_cidr(cidr, &ip6_hdr->ip_dst) ? DONT_SEND : SEND;

        case xXBoth:
            return (check_ip6_cidr(cidr, &ip6_hdr->ip_dst) && check_ip6_cidr(cidr, &ip6_hdr->ip_src)) ? DONT_SEND
                                                                                                      : SEND;
        case xXEither:
            return (check_ip6_cidr(cidr, &ip6_hdr->ip_dst) || check_ip6_cidr(cidr, &ip6_hdr->ip_src)) ? DONT_SEND
                                                                                                      : SEND;
        }
    } else {
        /* Include Mode */
        switch (mode) {
        case xXSource:
            return check_ip6_cidr(cidr, &ip6_hdr->ip_src) ? SEND : DONT_SEND;
        case xXDest:
            return check_ip6_cidr(cidr, &ip6_hdr->ip_dst) ? SEND : DONT_SEND;
        case xXBoth:
            return (check_ip6_cidr(cidr, &ip6_hdr->ip_dst) && check_ip6_cidr(cidr, &ip6_hdr->ip_src)) ? SEND
                                                                                                      : DONT_SEND;
        case xXEither:
            return (check_ip6_cidr(cidr, &ip6_hdr->ip_dst) || check_ip6_cidr(cidr, &ip6_hdr->ip_src)) ? SEND
                                                                                                      : DONT_SEND;
        default:
            assert(false);
        }
    }

    /* total failure */
    if (mode & xXExclude) {
        warn("Unable to determine action in CIDR filter mode.  Default: Don't Send.");
        return DONT_SEND;
    } else {
        warn("Unable to determine action in CIDR filter mode.  Default: Send.");
        return SEND;
    }
}
