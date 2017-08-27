/*
 * utilities related header file
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: wlc_utils.h 514073 2014-11-08 20:46:19Z $
 */

#ifndef _wlc_utils_h_
#define _wlc_utils_h_

#include <typedefs.h>

struct rsn_parms {
    uint8 flags;        /* misc booleans (e.g., supported) */
    uint8 multicast;    /* multicast cipher */
    uint8 ucount;       /* count of unicast ciphers */
    uint8 unicast[4];   /* unicast ciphers */
    uint8 acount;       /* count of auth modes */
    uint8 auth[4];      /* Authentication modes */
    uint8 PAD[4];       /* padding for future growth */
    uint8 cap[4];       /* capability */
};

typedef struct rsn_parms rsn_parms_t;

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* Calculate delta between 'a' and 'b'.
 * 'a' is a 32-bit counter value taken at t1 and 'b' is a 32-bit counter value taken at t2,
 * t2 is later than t1 in absolute time.
 * the 32-bit counter may wrap more than once between t1 and t2 but we wouldn't know.
 */
#define U32_DUR(a, b) ((uint32)(b) - (uint32)(a))

extern void wlc_uint64_add(uint32* high, uint32* low, uint32 inc_high, uint32 inc_low);
extern void wlc_uint64_sub(uint32* a_high, uint32* a_low, uint32 b_high, uint32 b_low);
extern bool wlc_uint64_lt(uint32 a_high, uint32 a_low, uint32 b_high, uint32 b_low);
extern uint32 wlc_calc_tbtt_offset(uint32 bi, uint32 tsf_h, uint32 tsf_l);
extern void wlc_tsf64_to_next_tbtt64(uint32 bcn_int, uint32 *tsf_h, uint32 *tsf_l);

/* rsn params lookup */
extern bool wlc_rsn_ucast_lookup(struct rsn_parms *rsn, uint8 auth);
extern bool wlc_rsn_akm_lookup(struct rsn_parms *rsn, uint8 akm);
extern uint32 wlc_convert_rsn_to_wsec_bitmap(struct rsn_parms *rsn);
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/* Frame Type and Frame Subtype conversion */
#define FST2FT(fst) (((fst) << FC_SUBTYPE_SHIFT) & FC_SUBTYPE_MASK)
#define FST2BMP(fst) (1 << (fst))
#define FT2FST(ft) (((ft) & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT)
#define FT2BMP(ft) (1 << FT2FST(ft))

/* Map Frame Type to VNDR_IE_XXXX_FLAG */
extern uint32 wlc_ft2vieflag(uint16 ft);
/* Map Sequence Number in FC_AUTH to VNDR_IE_XXXX_FLAG */
extern uint32 wlc_auth2vieflag(int seq);

extern const uint8 wlc_802_1x_hdr[];

#ifdef BCMCCX
extern const uint8 ckip_llc_snap[];
#endif

#endif /* !_wlc_utils_h_ */
