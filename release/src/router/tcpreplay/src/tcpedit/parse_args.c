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

#include "parse_args.h"
#include "config.h"
#include "portmap.h"
#include "tcpedit.h"
#include "tcpedit_stub.h"
#include <stdlib.h>
#include <string.h>

/**
 * returns 0 for success w/o errors
 * returns 1 for success w/ warnings
 * returns -1 for error
 */
int
tcpedit_post_args(tcpedit_t *tcpedit)
{
    int rcode;
    int i;
    uint32_t seed = 1, rand_num;

    assert(tcpedit);

    /* --pnat */
    if (HAVE_OPT(PNAT)) {
        int ct = STACKCT_OPT(PNAT);
        char **list = (char **)STACKLST_OPT(PNAT);
        int first = 1;

        tcpedit->rewrite_ip = true;

        do {
            char *p = *list++;
            if (first) {
                if (!parse_cidr_map(&tcpedit->cidrmap1, p)) {
                    tcpedit_seterr(tcpedit, "Unable to parse first --pnat=%s", p);
                    return -1;
                }
            } else {
                if (!parse_cidr_map(&tcpedit->cidrmap2, p)) {
                    tcpedit_seterr(tcpedit, "Unable to parse second --pnat=%s", p);
                    return -1;
                }
            }

            first = 0;
        } while (--ct > 0);
    }

    /* --srcipmap */
    if (HAVE_OPT(SRCIPMAP)) {
        tcpedit->rewrite_ip = true;
        if (!parse_cidr_map(&tcpedit->srcipmap, OPT_ARG(SRCIPMAP))) {
            tcpedit_seterr(tcpedit, "Unable to parse --srcipmap=%s", OPT_ARG(SRCIPMAP));
            return -1;
        }
    }

    /* --dstipmap */
    if (HAVE_OPT(DSTIPMAP)) {
        tcpedit->rewrite_ip = true;
        if (!parse_cidr_map(&tcpedit->dstipmap, OPT_ARG(DSTIPMAP))) {
            tcpedit_seterr(tcpedit, "Unable to parse --dstipmap=%s", OPT_ARG(DSTIPMAP));
            return -1;
        }
    }

    /*
     * If we have one and only one -N, then use the same map data
     * for both interfaces/files
     */
    if ((tcpedit->cidrmap1 != NULL) && (tcpedit->cidrmap2 == NULL))
        tcpedit->cidrmap2 = tcpedit->cidrmap1;

    /* --fixcsum */
    if (HAVE_OPT(FIXCSUM))
        tcpedit->fixcsum = true;

    /* --efcs */
    if (HAVE_OPT(EFCS))
        tcpedit->efcs = true;

    /* --ttl */
    if (HAVE_OPT(TTL)) {
        long ttl;

        if (strchr(OPT_ARG(TTL), '+')) {
            tcpedit->ttl_mode = TCPEDIT_TTL_MODE_ADD;
        } else if (strchr(OPT_ARG(TTL), '-')) {
            tcpedit->ttl_mode = TCPEDIT_TTL_MODE_SUB;
        } else {
            tcpedit->ttl_mode = TCPEDIT_TTL_MODE_SET;
        }

        ttl = strtol(OPT_ARG(TTL), (char **)NULL, 10);
        if (ttl < 0)
            ttl *= -1; /* convert to positive value */

        if (ttl > 255) {
            tcpedit_seterr(tcpedit, "Invalid --ttl value (must be 0-255): %ld", ttl);
            return -1;
        }

        tcpedit->ttl_value = (u_int8_t)ttl;
    }

    /* --tos */
    if (HAVE_OPT(TOS))
        tcpedit->tos = OPT_VALUE_TOS;

    /* --tclass */
    if (HAVE_OPT(TCLASS))
        tcpedit->tclass = OPT_VALUE_TCLASS;

    /* --flowlabel */
    if (HAVE_OPT(FLOWLABEL))
        tcpedit->flowlabel = OPT_VALUE_FLOWLABEL;

    /* --mtu */
    if (HAVE_OPT(MTU))
        tcpedit->mtu = OPT_VALUE_MTU;

    /* --mtu-trunc */
    if (HAVE_OPT(MTU_TRUNC))
        tcpedit->mtu_truncate = true;

    /* --skipbroadcast */
    if (HAVE_OPT(SKIPBROADCAST))
        tcpedit->skip_broadcast = true;

    /* --fixlen */
    if (HAVE_OPT(FIXLEN)) {
        if (strcmp(OPT_ARG(FIXLEN), "pad") == 0) {
            tcpedit->fixlen = TCPEDIT_FIXLEN_PAD;
        } else if (strcmp(OPT_ARG(FIXLEN), "trunc") == 0) {
            tcpedit->fixlen = TCPEDIT_FIXLEN_TRUNC;
        } else if (strcmp(OPT_ARG(FIXLEN), "del") == 0) {
            tcpedit->fixlen = TCPEDIT_FIXLEN_DEL;
        } else {
            tcpedit_seterr(tcpedit, "Invalid --fixlen=%s", OPT_ARG(FIXLEN));
            return -1;
        }
    }

    /* --rewrite-sequence */
    if (HAVE_OPT(TCP_SEQUENCE)) {
        tcpedit->tcp_sequence_enable = 1;
        seed = OPT_VALUE_TCP_SEQUENCE;
        for (i = 0; i < 5; ++i)
            rand_num = tcpr_random(&seed);

        tcpedit->tcp_sequence_adjust = rand_num;
    }

    /* TCP/UDP port rewriting */
    if (HAVE_OPT(PORTMAP)) {
        int ct = STACKCT_OPT(PORTMAP);
        char **list = (char **)STACKLST_OPT(PORTMAP);
        int first = 1;
        tcpedit_portmap_t *portmap_head, *portmap;

        do {
            char *p = *list++;
            if (first) {
                if (!parse_portmap(&tcpedit->portmap, p)) {
                    tcpedit_seterr(tcpedit, "Unable to parse --portmap=%s", p);
                    return -1;
                }
            } else {
                if (!parse_portmap(&portmap, p)) {
                    tcpedit_seterr(tcpedit, "Unable to parse --portmap=%s", p);
                    return -1;
                }

                /* append to end of tcpedit->portmap linked list */
                portmap_head = tcpedit->portmap;
                while (portmap_head->next != NULL)
                    portmap_head = portmap_head->next;
                portmap_head->next = portmap;
            }
            first = 0;
        } while (--ct > 0);
    }

    /*
     * IP address rewriting processing. Update seed by making
     * 5 calls to tcpr_random() as our mixer for randomizing.  This should
     * work better since most people aren't going to write out values
     * close to 32bit integers.
     */
    if (HAVE_OPT(SEED)) {
        tcpedit->rewrite_ip = true;
        seed = OPT_VALUE_SEED;
    } else if (HAVE_OPT(FUZZ_SEED)) {
        /* --fuzz-seed */
        seed = OPT_VALUE_FUZZ_SEED;
        tcpedit->fuzz_factor = OPT_VALUE_FUZZ_FACTOR;
    }

    for (i = 0; i < 5; ++i)
        rand_num = tcpr_random(&seed);

    srandom(rand_num);
    if (HAVE_OPT(SEED)) {
        tcpedit->rewrite_ip = true;
        tcpedit->seed = seed;
    }

    if (HAVE_OPT(FUZZ_SEED)) {
        /* --fuzz-seed */
        tcpedit->fuzz_seed = seed;
    }

    if (HAVE_OPT(ENDPOINTS)) {
        tcpedit->rewrite_ip = true;
        if (!parse_endpoints(&tcpedit->cidrmap1, &tcpedit->cidrmap2, OPT_ARG(ENDPOINTS))) {
            tcpedit_seterr(tcpedit, "Unable to parse --endpoints=%s", OPT_ARG(ENDPOINTS));
            return -1;
        }
    }

    /* parse the tcpedit dlt args */
    rcode = tcpedit_dlt_post_args(tcpedit);
    if (rcode < 0) {
        errx(-1, "Unable to parse args: %s", tcpedit_geterr(tcpedit));
    } else if (rcode == 1) {
        warnx("%s", tcpedit_geterr(tcpedit));
    }

    return 0;
}
