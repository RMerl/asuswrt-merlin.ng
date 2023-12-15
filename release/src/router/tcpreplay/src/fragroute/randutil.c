/*
 * randutil.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "config.h"

#ifdef HAVE_LIBDNET
/* need to undef these which are pulled in via defines.h, prior to importing dnet.h */
#undef icmp_id
#undef icmp_seq
#undef icmp_data
#undef icmp_mask
#ifdef HAVE_DNET_H
#include <dnet.h>
#endif
#ifdef HAVE_DUMBNET_H
#include <dumbnet.h>
#endif
#endif

#include "randutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void
rand_strset(rand_t *r, void *buf, size_t len)
{
    uint32_t u;
    char *p;
    int i;

    p = (char *)buf;
    i = (len + 3) / 4;
    u = rand_uint32(r);

    /* XXX - more Duff's device tomfoolery. */
    switch (len % 4) {
    case 0:
        do {
            u = rand_uint32(r);
            *p++ = base64[(u >> 18) & 0x3f];
            /* fall through */
        case 3:
            *p++ = base64[(u >> 12) & 0x3f];
            /* fall through */
        case 2:
            *p++ = base64[(u >> 6) & 0x3f];
            /* fall through */
        case 1:
            *p++ = base64[(u >> 0) & 0x3f];
        } while (--i > 0);
    }
    p[-1] = '\0';
}
