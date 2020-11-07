/*
 * utilities related header file
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wlc_utils.h 786522 2020-04-29 20:31:58Z $
 */

#ifndef _wlc_utils_h_
#define _wlc_utils_h_

#include <typedefs.h>
#include <ethernet.h>

typedef struct _ravg {
	uint32 sum;
	uint8 exp;
} wlc_ravg_info_t;

#define RAVG_EXP(_obj_) ((_obj_)->exp)
#define RAVG_SUM(_obj_) ((_obj_)->sum)
#define RAVG_AVG(_obj_) ((_obj_)->sum >> (_obj_)->exp)

/* Basic running average algorithm:
 * Keep a running buffer of the last N values, and a running SUM of all the
 * values in the buffer. Each time a new sample comes in, subtract the oldest
 * value in the buffer from SUM, replace it with the new sample, add the new
 * sample to SUM, and output SUM/N.
 */
#define RAVG_ADD(_obj_, _sample_) \
{ \
	uint8 _exp_ = RAVG_EXP((_obj_)); \
	uint32 _sum_ = RAVG_SUM((_obj_)); \
	RAVG_SUM((_obj_)) = ((_sum_ << _exp_) - _sum_) >> _exp_; \
	RAVG_SUM((_obj_)) += (_sample_); \
}

/* Initializing running buffer with value (_sample_) */
#define RAVG_INIT(_obj_, _sample_, _exp_) \
{ \
	RAVG_SUM((_obj_)) = (_sample_) << (_exp_); \
	RAVG_EXP((_obj_)) = (_exp_); \
}

/* Calculate delta between 'a' and 'b'.
 * 'a' is a 32-bit counter value taken at t1 and 'b' is a 32-bit counter value taken at t2,
 * t2 is later than t1 in absolute time.
 * the 32-bit counter may wrap more than once between t1 and t2 but we wouldn't know.
 */
#define U32_DUR(a, b) ((uint32)(b) - (uint32)(a))

extern void wlc_uint64_add(uint32* high, uint32* low, uint32 inc_high, uint32 inc_low);
extern void wlc_uint64_sub(uint32* a_high, uint32* a_low, uint32 b_high, uint32 b_low);
extern bool wlc_uint64_lt(uint32 a_high, uint32 a_low, uint32 b_high, uint32 b_low);
extern uint32 wlc_uint64_div(uint64 a, uint64 b);
extern uint32 wlc_calc_tbtt_offset(uint32 bi, uint32 tsf_h, uint32 tsf_l);
extern void wlc_tsf64_to_next_tbtt64(uint32 bcn_int, uint32 *tsf_h, uint32 *tsf_l);

#ifndef LINUX_POSTMOGRIFY_REMOVAL
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

/* rsn params lookup */
extern bool wlc_rsn_ucast_lookup(struct rsn_parms *rsn, uint8 auth);
extern bool wlc_rsn_akm_lookup(struct rsn_parms *rsn, uint8 akm);
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
extern bool wlc_ssid_cmp(uint8 *ssid1, uint8 *ssid2, uint16 len1, uint16 len2);

/* Frame Type and Frame Subtype conversion */
#define FST2FT(fst) (((fst) << FC_SUBTYPE_SHIFT) & FC_SUBTYPE_MASK)
#define FST2BMP(fst) (1 << (fst))
#define FT2FST(ft) (((ft) & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT)
#define FT2BMP(ft) (1 << FT2FST(ft))

/* Map Frame Type to VNDR_IE_XXXX_FLAG */
extern uint32 wlc_ft2vieflag(uint16 ft);
/* Map Frame SubType to VNDR_IE_XXXX_FLAG */
extern uint32 wlc_fst2vieflag(uint16 fst);
/* Map Sequence Number in FC_AUTH to VNDR_IE_XXXX_FLAG */
extern uint32 wlc_auth2vieflag(int seq);

extern const uint8 wlc_802_1x_hdr[];

extern int32 wlc_mcast_reverse_translation(struct ether_header *eh);

bool is_igmp(struct ether_header *eh);

uint wlc_bsstype_wl2dot11(uint wl);
uint wlc_bsstype_dot112wl(uint dot11);
const char *wlc_bsstype_dot11name(uint dot11);
struct wlc_info;
struct wlc_if;
extern struct wlc_bsscfg *wlc_bsscfg_find_by_wlcif(struct wlc_info *wlc,
	struct wlc_if *wl_if);
extern struct wlc_bsscfg *bcm_iov_bsscfg_find_from_wlcif(struct wlc_info *wlc,
	struct wlc_if *wl_if);
uint8 wlc_chspec_bw2bwcap_bit(uint16 bw);
#endif /* !_wlc_utils_h_ */
