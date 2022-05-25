/*
 * Common interface to channel definitions.
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 * $Id: bcmwifi_rclass.c 796498 2021-03-05 07:03:48Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmwifi_rclass.h>
#ifndef BCMDRIVER
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define ASSERT(exp) assert(exp)
#endif /* BCMDRIVER */

struct g_bitvec_t {
	uint8 len;                         /* length of bit vector array */
	uint8 bvec[BCMWIFI_MAX_VEC_SIZE];  /* array of bits */
} g_bvec;

/* Global Operating Classes */

/* op class 81 */
static const uint8 chan_set_2g_20[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

/* op class 82 */
static const uint8 chan_set_2g_20_82[] = {
	14
};

/* op class 83 */
static const uint8 chan_set_2g_40l[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9
};

/* op class 84 */
static const uint8 chan_set_2g_40u[] = {
	5, 6, 7, 8, 9, 10, 11, 12, 13
};

/* op class 112 */
static const uint8 chan_set_5g_20_1[] = {
	8, 12, 16
};

/* op class 115 */
static const uint8 chan_set_5g_20_2[] = {
	36, 40, 44, 48
};

/* op class 116 */
static const uint8 chan_set_5g_40l_1[] = {
	36, 44
};

/* op class 117 */
static const uint8 chan_set_5g_40u_1[] = {
	40, 48
};

/* op class 118 */
static const uint8 chan_set_5g_20_3[] = {
	52, 56, 60, 64
};

/* op class 119 */
static const uint8 chan_set_5g_40l_2[] = {
	52, 60
};

/* op class 120 */
static const uint8 chan_set_5g_40u_2[] = {
	56, 64
};

/* op class 121 */
static const uint8 chan_set_5g_20_4[] = {
	100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144
};

/* op class 122 */
static const uint8 chan_set_5g_40l_3[] = {
	100, 108, 116, 124, 132, 140
};

/* op class 123 */
static const uint8 chan_set_5g_40u_3[] = {
	104, 112, 120, 128, 136, 144
};

/* op class 124 */
static const uint8 chan_set_5g_20_5[] = {
	149, 153, 157, 161
};

/* op class 125 */
static const uint8 chan_set_5g_20_6[] = {
	149, 153, 157, 161, 165, 169, 173, 177
};

/* op class 126 */
static const uint8 chan_set_5g_40l_4[] = {
	149, 157, 165, 173
};

/* op class 127 */
static const uint8 chan_set_5g_40u_4[] = {
	153, 161, 169, 177
};

/* op class 128 */
static const uint8 chan_set_5g_80_1[] = {
	42, 58, 106, 122, 138, 155, 171
};

/* op class 129 */
static const uint8 chan_set_5g_160_1[] = {
	50, 114, 163
};

/* op class 130 */
static const uint8 chan_set_5g_80_2[] = {
	42, 58, 106, 122, 138, 155, 171
};

/* op class 131 */
static const uint8 chan_set_6g_20[] = {
	1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61, 65, 69,
	73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 129,
	133, 137, 141, 145, 149, 153, 157, 161, 165, 169, 173, 177, 181, 185,
	189, 193, 197, 201, 205, 209, 213, 217, 221, 225, 229, 233
};

/* op class 132 */
static const uint8 chan_set_6g_40[] = {
	3, 11, 19, 27, 35, 43, 51, 59, 67, 75, 83, 91, 99, 107, 115, 123, 131,
	139, 147, 155, 163, 171, 179, 187, 195, 203, 211, 219, 227
};

/* op class 133 and 135 */
static const uint8 chan_set_6g_80[] = {
	7, 23, 39, 55, 71, 87, 103, 119, 135, 151, 167, 183, 199, 215
};

/* op class 134 */
static const uint8 chan_set_6g_160[] = {
	15, 47, 79, 111, 143, 175, 207
};

/* op class 136 */
static const uint8 chan_set_6g2[] = {
	2
};

static const bcmwifi_rclass_info_t rcinfo_global[] = {
	{81, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_2G,
	BCMWIFI_RCLASS_FLAGS_NONE, ARRAYSIZE(chan_set_2g_20), chan_set_2g_20},
	{82, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_2G,
	BCMWIFI_RCLASS_FLAGS_NONE, ARRAYSIZE(chan_set_2g_20_82), chan_set_2g_20_82},
	{83, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_2G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER, ARRAYSIZE(chan_set_2g_40l), chan_set_2g_40l},
	{84, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_2G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER, ARRAYSIZE(chan_set_2g_40u), chan_set_2g_40u},
	{112, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY,  BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NONE, ARRAYSIZE(chan_set_5g_20_1), chan_set_5g_20_1},
	{115, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY,  BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_EIRP, ARRAYSIZE(chan_set_5g_20_2), chan_set_5g_20_2},
	{116, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER),
	ARRAYSIZE(chan_set_5g_40l_1), chan_set_5g_40l_1},
	{117, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER),
	ARRAYSIZE(chan_set_5g_40u_1), chan_set_5g_40u_1},
	{118, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY,  BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_DFS | BCMWIFI_RCLASS_FLAGS_EIRP),
	ARRAYSIZE(chan_set_5g_20_3), chan_set_5g_20_3},
	{119, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_DFS | BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER),
	ARRAYSIZE(chan_set_5g_40l_2), chan_set_5g_40l_2},
	{120, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_DFS | BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER),
	ARRAYSIZE(chan_set_5g_40u_2), chan_set_5g_40u_2},
	{121, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY,  BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_DFS | BCMWIFI_RCLASS_FLAGS_EIRP),
	ARRAYSIZE(chan_set_5g_20_4), chan_set_5g_20_4},
	{122, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_DFS | BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER),
	ARRAYSIZE(chan_set_5g_40l_3), chan_set_5g_40l_3},
	{123, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_DFS | BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER),
	ARRAYSIZE(chan_set_5g_40u_3), chan_set_5g_40u_3},
	{124, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY,  BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_NOMADIC | BCMWIFI_RCLASS_FLAGS_EIRP),
	ARRAYSIZE(chan_set_5g_20_5), chan_set_5g_20_5},
	{125, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY,  BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_LIC_EXMPT | BCMWIFI_RCLASS_FLAGS_EIRP),
	ARRAYSIZE(chan_set_5g_20_6), chan_set_5g_20_6},
	{126, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER | BCMWIFI_RCLASS_FLAGS_EIRP),
	ARRAYSIZE(chan_set_5g_40l_4), chan_set_5g_40l_4},
	{127, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER | BCMWIFI_RCLASS_FLAGS_EIRP),
	ARRAYSIZE(chan_set_5g_40u_4), chan_set_5g_40u_4},
	{128, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_80, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_EIRP, ARRAYSIZE(chan_set_5g_80_1), chan_set_5g_80_1},
	{129, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_160, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_EIRP, ARRAYSIZE(chan_set_5g_160_1), chan_set_5g_160_1},
	{130, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_80, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_80PLUS),
	ARRAYSIZE(chan_set_5g_80_2), chan_set_5g_80_2},
	{131, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_20, BCMWIFI_BAND_6G,
	BCMWIFI_RCLASS_FLAGS_NONE, ARRAYSIZE(chan_set_6g_20), chan_set_6g_20},
	{132, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_40, BCMWIFI_BAND_6G,
	BCMWIFI_RCLASS_FLAGS_NONE, ARRAYSIZE(chan_set_6g_40), chan_set_6g_40},
	{133, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_80, BCMWIFI_BAND_6G,
	BCMWIFI_RCLASS_FLAGS_NONE, ARRAYSIZE(chan_set_6g_80), chan_set_6g_80},
	{134, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_160, BCMWIFI_BAND_6G,
	BCMWIFI_RCLASS_FLAGS_NONE, ARRAYSIZE(chan_set_6g_160), chan_set_6g_160},
	{135, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_80, BCMWIFI_BAND_6G,
	BCMWIFI_RCLASS_FLAGS_80PLUS, ARRAYSIZE(chan_set_6g_80), chan_set_6g_80},
	{136, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_20, BCMWIFI_BAND_6G,
	BCMWIFI_RCLASS_FLAGS_NONE, ARRAYSIZE(chan_set_6g2), chan_set_6g2},
};

/* US Operating Classes */

/* US op class 01 */
static const uint8 chan_set_us_5g_20_1[] = {
	36, 40, 44, 48
};

/* US op class 02 */
static const uint8 chan_set_us_5g_20_2[] = {
	52, 56, 60, 64
};

/* US op class 03 */
static const uint8 chan_set_us_5g_20_3[] = {
	149, 153, 157, 161
};

/* US op class 04 */
static const uint8 chan_set_us_5g_20_4[] = {
	100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144
};

/* US op class 05 */
static const uint8 chan_set_us_5g_20_5[] = {
	149, 153, 157, 161, 165
};

/* US op class 12 */
static const uint8 chan_set_us_2g_20[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

/* US op class 22 */
static const uint8 chan_set_us_5g_40l_1[] = {
	36, 44
};

/* US op class 23 */
static const uint8 chan_set_us_5g_40l_2[] = {
	52, 60
};

/* US op class 24 */
static const uint8 chan_set_us_5g_40l_3[] = {
	100, 108, 116, 124, 132, 140
};

/* US op class 25 */
static const uint8 chan_set_us_5g_40l_4[] = {
	149, 157
};

/* US op class 26 */
static const uint8 chan_set_us_5g_40l_5[] = {
	149, 157
};

/* US op class 27 */
static const uint8 chan_set_us_5g_40u_1[] = {
	40, 48
};

/* US op class 28 */
static const uint8 chan_set_us_5g_40u_2[] = {
	56, 64
};

/* US op class 29 */
static const uint8 chan_set_us_5g_40u_3[] = {
	104, 112, 120, 128, 136, 144
};

/* US op class 30 */
static const uint8 chan_set_us_5g_40u_4[] = {
	153, 161
};

/* US op class 31 */
static const uint8 chan_set_us_5g_40u_5[] = {
	153, 161
};

/* US op class 32 */
static const uint8 chan_set_us_2g_40l[] = {
	1, 2, 3, 4, 5, 6, 7
};

/* US op class 33 */
static const uint8 chan_set_us_2g_40u[] = {
	5, 6, 7, 8, 9, 10, 11
};

/* US op class 128 */
static const uint8 chan_set_us_5g_80_1[] = {
	42, 58, 106, 122, 138, 155
};

/* US op class 129 */
static const uint8 chan_set_us_5g_160[] = {
	50, 114
};

/* US op class 130 */
static const uint8 chan_set_us_5g_80_2[] = {
	42, 58, 106, 122, 138, 155
};

static const bcmwifi_rclass_info_t rcinfo_us[] = {
	{1, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NONE,
	ARRAYSIZE(chan_set_us_5g_20_1), chan_set_us_5g_20_1},
	{2, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_DFS,
	ARRAYSIZE(chan_set_us_5g_20_2), chan_set_us_5g_20_2},
	{3, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NOMADIC,
	ARRAYSIZE(chan_set_us_5g_20_3), chan_set_us_5g_20_3},
	{4, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY,  BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_DFS | BCMWIFI_RCLASS_FLAGS_EIRP),
	ARRAYSIZE(chan_set_us_5g_20_4), chan_set_us_5g_20_4},
	{5, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY,  BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_LIC_EXMPT,
	ARRAYSIZE(chan_set_us_5g_20_5), chan_set_us_5g_20_5},
	{12, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_2G,
	BCMWIFI_RCLASS_FLAGS_LIC_EXMPT,
	ARRAYSIZE(chan_set_us_2g_20), chan_set_us_2g_20},
	{22, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER,
	ARRAYSIZE(chan_set_us_5g_40l_1), chan_set_us_5g_40l_1},
	{23, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER,
	ARRAYSIZE(chan_set_us_5g_40l_2), chan_set_us_5g_40l_2},
	{24, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_DFS | BCMWIFI_RCLASS_FLAGS_EIRP |
	BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER),
	ARRAYSIZE(chan_set_us_5g_40l_3), chan_set_us_5g_40l_3},
	{25, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER,
	ARRAYSIZE(chan_set_us_5g_40l_4), chan_set_us_5g_40l_4},
	{26, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_LIC_EXMPT | BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER),
	ARRAYSIZE(chan_set_us_5g_40l_5), chan_set_us_5g_40l_5},
	{27, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER,
	ARRAYSIZE(chan_set_us_5g_40u_1), chan_set_us_5g_40u_1},
	{28, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER,
	ARRAYSIZE(chan_set_us_5g_40u_2), chan_set_us_5g_40u_2},
	{29, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_NOMADIC | BCMWIFI_RCLASS_FLAGS_EIRP |
	BCMWIFI_RCLASS_FLAGS_DFS | BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER),
	ARRAYSIZE(chan_set_us_5g_40u_3), chan_set_us_5g_40u_3},
	{30, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_NOMADIC | BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER),
	ARRAYSIZE(chan_set_us_5g_40u_4), chan_set_us_5g_40u_4},
	{31, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_LIC_EXMPT | BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER),
	ARRAYSIZE(chan_set_us_5g_40u_5), chan_set_us_5g_40u_5},
	{32, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_2G,
	(BCMWIFI_RCLASS_FLAGS_LIC_EXMPT | BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER),
	ARRAYSIZE(chan_set_us_2g_40l), chan_set_us_2g_40l},
	{33, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_2G,
	(BCMWIFI_RCLASS_FLAGS_LIC_EXMPT | BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER),
	ARRAYSIZE(chan_set_us_2g_40u), chan_set_us_2g_40u},
	{128, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_80, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_EIRP,
	ARRAYSIZE(chan_set_us_5g_80_1), chan_set_us_5g_80_1},
	{129, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_160, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_EIRP,
	ARRAYSIZE(chan_set_us_5g_160), chan_set_us_5g_160},
	{130, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_80, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_80PLUS),
	ARRAYSIZE(chan_set_us_5g_80_2), chan_set_us_5g_80_2},
};

/* EU Operating Classes */

/* EU op class 01 */
static const uint8 chan_set_eu_5g_20_1[] = {
	36, 40, 44, 48
};

/* EU op class 02 */
static const uint8 chan_set_eu_5g_20_2[] = {
	52, 56, 60, 64
};

/* EU op class 03 */
static const uint8 chan_set_eu_5g_20_3[] = {
	100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140
};

/* EU op class 04 */
static const uint8 chan_set_eu_2g_20[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

/* EU op class 05 */
static const uint8 chan_set_eu_5g_40l_1[] = {
	36, 44
};

/* EU op class 06 */
static const uint8 chan_set_eu_5g_40l_2[] = {
	52, 60
};

/* EU op class 07 */
static const uint8 chan_set_eu_5g_40l_3[] = {
	100, 108, 116, 124, 132
};

/* EU op class 08 */
static const uint8 chan_set_eu_5g_40u_1[] = {
	40, 48
};

/* EU op class 09 */
static const uint8 chan_set_eu_5g_40u_2[] = {
	56, 64
};

/* EU op class 10 */
static const uint8 chan_set_eu_5g_40u_3[] = {
	104, 112, 120, 128, 136
};

/* EU op class 11 */
static const uint8 chan_set_eu_2g_40l[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9
};

/* EU op class 12 */
static const uint8 chan_set_eu_2g_40u[] = {
	5, 6, 7, 8, 9, 10, 11, 12, 13
};

/* EU op class 16 */
static const uint8 chan_set_eu_5g_20_4[] = {
	100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140
};

/* EU op class 17 */
static const uint8 chan_set_eu_5g_20_5[] = {
	149, 153, 157, 161, 165, 169
};

/* EU op class 128 */
static const uint8 chan_set_eu_5g_80_1[] = {
	42, 58, 106, 122
};

/* EU op class 129 */
static const uint8 chan_set_eu_5g_160[] = {
	50, 114
};

/* EU op class 130 */
static const uint8 chan_set_eu_5g_80_2[] = {
	42, 58, 106, 122
};

static const bcmwifi_rclass_info_t rcinfo_eu[] = {
	{1, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NONE,
	ARRAYSIZE(chan_set_eu_5g_20_1), chan_set_eu_5g_20_1},
	{2, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NOMADIC,
	ARRAYSIZE(chan_set_eu_5g_20_2), chan_set_eu_5g_20_2},
	{3, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NONE,
	ARRAYSIZE(chan_set_eu_5g_20_3), chan_set_eu_5g_20_3},
	{4, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY,  BCMWIFI_BW_20, BCMWIFI_BAND_2G,
	BCMWIFI_RCLASS_FLAGS_LIC_EXMPT,
	ARRAYSIZE(chan_set_eu_2g_20), chan_set_eu_2g_20},
	{5, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY,  BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER,
	ARRAYSIZE(chan_set_eu_5g_40l_1), chan_set_eu_5g_40l_1},
	{6, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER,
	ARRAYSIZE(chan_set_eu_5g_40l_2), chan_set_eu_5g_40l_2},
	{7, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER,
	ARRAYSIZE(chan_set_eu_5g_40l_3), chan_set_eu_5g_40l_3},
	{8, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER,
	ARRAYSIZE(chan_set_eu_5g_40u_1), chan_set_eu_5g_40u_1},
	{9, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER,
	ARRAYSIZE(chan_set_eu_5g_40u_2), chan_set_eu_5g_40u_2},
	{10, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER,
	ARRAYSIZE(chan_set_eu_5g_40u_3), chan_set_eu_5g_40u_3},
	{11, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_2G,
	(BCMWIFI_RCLASS_FLAGS_LIC_EXMPT | BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER),
	ARRAYSIZE(chan_set_eu_2g_40l), chan_set_eu_2g_40l},
	{12, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_2G,
	BCMWIFI_RCLASS_FLAGS_LIC_EXMPT | BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER,
	ARRAYSIZE(chan_set_eu_2g_40u), chan_set_eu_2g_40u},
	{16, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_ITS_NONMOB_OPS |
	BCMWIFI_RCLASS_FLAGS_ITS_MOB_OPS),
	ARRAYSIZE(chan_set_eu_5g_20_4), chan_set_eu_5g_20_4},
	{17, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NONE,
	ARRAYSIZE(chan_set_eu_5g_20_5), chan_set_eu_5g_20_5},
	{128, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_80, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_EIRP,
	ARRAYSIZE(chan_set_eu_5g_80_1), chan_set_eu_5g_80_1},
	{129, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_160, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_EIRP,
	ARRAYSIZE(chan_set_eu_5g_160), chan_set_eu_5g_160},
	{130, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_80, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_80PLUS),
	ARRAYSIZE(chan_set_eu_5g_80_2), chan_set_eu_5g_80_2},
};

/* JP Operating Classes */

/* JP op class 01 */
static const uint8 chan_set_jp_5g_20_1[] = {
	34, 38, 42, 46, 36, 40, 44, 48
};

/* JP op class 02/03/04/04/05/06 */
static const uint8 chan_set_jp_5g_20_2[] = {
	8, 12, 16
};

/* JP op class 30 */
static const uint8 chan_set_jp_2g_20_1[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

/* JP op class 31 */
static const uint8 chan_set_jp_2g_20_2[] = {
	14
};

/* JP op class 32/33 */
static const uint8 chan_set_jp_5g_20_3[] = {
	52, 56, 60, 64
};

/* JP op class 34/35/58 */
static const uint8 chan_set_jp_5g_20_4[] = {
	100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140
};

/* JP op class 36 */
static const uint8 chan_set_jp_5g_40l_1[] = {
	36, 44
};

/* JP op class 37/38 */
static const uint8 chan_set_jp_5g_40l_2[] = {
	52, 60
};

/* JP op class 39/40 */
static const uint8 chan_set_jp_5g_40l_3[] = {
	100, 108, 116, 120, 124, 132
};

/* JP op class 41 */
static const uint8 chan_set_jp_5g_40u_1[] = {
	40, 48
};

/* JP op class 42/43 */
static const uint8 chan_set_jp_5g_40u_2[] = {
	56, 64
};

/* JP op class 44/45 */
static const uint8 chan_set_jp_5g_40u_3[] = {
	104, 112, 120, 128, 136
};

/* JP op class 56 */
static const uint8 chan_set_jp_2g_40l[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9
};

/* JP op class 57 */
static const uint8 chan_set_jp_2g_40u[] = {
	5, 6, 7, 8, 9, 10, 11, 12, 13
};

/* JP op class 128 */
static const uint8 chan_set_jp_5g_80_1[] = {
	42, 58, 106, 122
};

/* JP op class 129 */
static const uint8 chan_set_jp_5g_160[] = {
	50, 114
};

/* JP op class 130 */
static const uint8 chan_set_jp_5g_80_2[] = {
	42, 58, 106, 122
};

static const bcmwifi_rclass_info_t rcinfo_jp[] = {
	{1, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NONE,
	ARRAYSIZE(chan_set_jp_5g_20_1), chan_set_jp_5g_20_1},
	{2, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NONE,
	ARRAYSIZE(chan_set_jp_5g_20_2), chan_set_jp_5g_20_2},
	{3, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NONE,
	ARRAYSIZE(chan_set_jp_5g_20_2), chan_set_jp_5g_20_2},
	{4, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NONE,
	ARRAYSIZE(chan_set_jp_5g_20_2), chan_set_jp_5g_20_2},
	{5, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NONE,
	ARRAYSIZE(chan_set_jp_5g_20_2), chan_set_jp_5g_20_2},
	{6, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NONE,
	ARRAYSIZE(chan_set_jp_5g_20_2), chan_set_jp_5g_20_2},
	{30, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_2G,
	BCMWIFI_RCLASS_FLAGS_LIC_EXMPT,
	ARRAYSIZE(chan_set_jp_2g_20_1), chan_set_jp_2g_20_1},
	{31, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_2G,
	BCMWIFI_RCLASS_FLAGS_LIC_EXMPT,
	ARRAYSIZE(chan_set_jp_2g_20_2), chan_set_jp_2g_20_2},
	{32, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NONE,
	ARRAYSIZE(chan_set_jp_5g_20_3), chan_set_jp_5g_20_3},
	{33, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_NONE,
	ARRAYSIZE(chan_set_jp_5g_20_3), chan_set_jp_5g_20_3},
	{34, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_DFS,
	ARRAYSIZE(chan_set_jp_5g_20_4), chan_set_jp_5g_20_4},
	{35, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_DFS,
	ARRAYSIZE(chan_set_jp_5g_20_4), chan_set_jp_5g_20_4},
	{36, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER,
	ARRAYSIZE(chan_set_jp_5g_40l_1), chan_set_jp_5g_40l_1},
	{37, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER,
	ARRAYSIZE(chan_set_jp_5g_40l_2), chan_set_jp_5g_40l_2},
	{38, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER,
	ARRAYSIZE(chan_set_jp_5g_40l_2), chan_set_jp_5g_40l_2},
	{39, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_DFS | BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER,
	ARRAYSIZE(chan_set_jp_5g_40l_3), chan_set_jp_5g_40l_3},
	{40, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_DFS | BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER,
	ARRAYSIZE(chan_set_jp_5g_40l_3), chan_set_jp_5g_40l_3},
	{41, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER,
	ARRAYSIZE(chan_set_jp_5g_40u_1), chan_set_jp_5g_40u_1},
	{42, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER,
	ARRAYSIZE(chan_set_jp_5g_40u_2), chan_set_jp_5g_40u_2},
	{43, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER,
	ARRAYSIZE(chan_set_jp_5g_40u_2), chan_set_jp_5g_40u_2},
	{44, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_DFS | BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER,
	ARRAYSIZE(chan_set_jp_5g_40u_3), chan_set_jp_5g_40u_3},
	{45, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_DFS | BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER,
	ARRAYSIZE(chan_set_jp_5g_40u_3), chan_set_jp_5g_40u_3},
	{56, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_2G,
	BCMWIFI_RCLASS_FLAGS_LIC_EXMPT | BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER,
	ARRAYSIZE(chan_set_jp_2g_40l), chan_set_jp_2g_40l},
	{57, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_2G,
	BCMWIFI_RCLASS_FLAGS_LIC_EXMPT | BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER,
	ARRAYSIZE(chan_set_jp_2g_40u), chan_set_jp_2g_40u},
	{58, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_LIC_EXMPT | BCMWIFI_RCLASS_FLAGS_NOMADIC,
	ARRAYSIZE(chan_set_jp_5g_20_4), chan_set_jp_5g_20_4},
	{128, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_80, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_EIRP,
	ARRAYSIZE(chan_set_jp_5g_80_1), chan_set_jp_5g_80_1},
	{129, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_160, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_EIRP,
	ARRAYSIZE(chan_set_jp_5g_160), chan_set_jp_5g_160},
	{130, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_80, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_80PLUS),
	ARRAYSIZE(chan_set_jp_5g_80_2), chan_set_jp_5g_80_2},
};

/* China Operating Classes */

/* CH op class 01 */
static const uint8 chan_set_ch_5g_20_1[] = {
	36, 40, 44, 48
};

/* CH op class 02 */
static const uint8 chan_set_ch_5g_20_2[] = {
	52, 56, 60, 64
};

/* CH op class 03 */
static const uint8 chan_set_ch_5g_20_3[] = {
	149, 153, 157, 161, 165
};

/* CH op class 04 */
static const uint8 chan_set_ch_5g_40l_1[] = {
	36, 44
};

/* CH op class 05 */
static const uint8 chan_set_ch_5g_40l_2[] = {
	52, 60
};

/* CH op class 06 */
static const uint8 chan_set_ch_5g_40l_3[] = {
	149, 157
};

/* CH op class 07 */
static const uint8 chan_set_ch_2g_20[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

/* CH op class 08 */
static const uint8 chan_set_ch_2g_40l[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9
};

/* CH op class 09 */
static const uint8 chan_set_ch_2g_40u[] = {
	5, 6, 7, 8, 9, 10, 11, 12, 13
};

/* CH op class 128/130 */
static const uint8 chan_set_ch_5g_80[] = {
	42, 58, 155
};

/* CH op class 129 */
static const uint8 chan_set_ch_5g_160[] = {
	50
};

static const bcmwifi_rclass_info_t rcinfo_ch[] = {
	{1, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_EIRP,
	ARRAYSIZE(chan_set_ch_5g_20_1), chan_set_ch_5g_20_1},
	{2, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_DFS),
	ARRAYSIZE(chan_set_ch_5g_20_2), chan_set_ch_5g_20_2},
	{3, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_EIRP,
	ARRAYSIZE(chan_set_ch_5g_20_3), chan_set_ch_5g_20_3},
	{4, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER),
	ARRAYSIZE(chan_set_ch_5g_40l_1), chan_set_ch_5g_40l_1},
	{5, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_DFS |
	BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER),
	ARRAYSIZE(chan_set_ch_5g_40l_2), chan_set_ch_5g_40l_2},
	{6, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER),
	ARRAYSIZE(chan_set_ch_5g_40l_3), chan_set_ch_5g_40l_3},
	{7, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_20, BCMWIFI_BAND_2G,
	BCMWIFI_RCLASS_FLAGS_LIC_EXMPT,
	ARRAYSIZE(chan_set_ch_2g_20), chan_set_ch_2g_20},
	{8, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_2G,
	(BCMWIFI_RCLASS_FLAGS_LIC_EXMPT | BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER),
	ARRAYSIZE(chan_set_ch_2g_40l), chan_set_ch_2g_40l},
	{9, BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY, BCMWIFI_BW_40, BCMWIFI_BAND_2G,
	(BCMWIFI_RCLASS_FLAGS_LIC_EXMPT | BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER),
	ARRAYSIZE(chan_set_ch_2g_40u), chan_set_ch_2g_40u},
	{128, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_80, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_EIRP,
	ARRAYSIZE(chan_set_ch_5g_80), chan_set_ch_5g_80},
	{129, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_160, BCMWIFI_BAND_5G,
	BCMWIFI_RCLASS_FLAGS_EIRP,
	ARRAYSIZE(chan_set_ch_5g_160), chan_set_ch_5g_160},
	{130, BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ,  BCMWIFI_BW_80, BCMWIFI_BAND_5G,
	(BCMWIFI_RCLASS_FLAGS_EIRP | BCMWIFI_RCLASS_FLAGS_80PLUS),
	ARRAYSIZE(chan_set_ch_5g_80), chan_set_ch_5g_80},
};

static int
bcmwifi_rclass_get_rclass_table(bcmwifi_rclass_type_t type,
	const bcmwifi_rclass_info_t **rcinfo, uint8 *rct_len)
{
	int ret = BCME_OK;

	switch (type) {
	case BCMWIFI_RCLASS_TYPE_GBL:
		*rcinfo = rcinfo_global;
		*rct_len = (uint8)ARRAYSIZE(rcinfo_global);
		break;
	case BCMWIFI_RCLASS_TYPE_US:
		*rcinfo = rcinfo_us;
		*rct_len = (uint8)ARRAYSIZE(rcinfo_us);
		break;
	case BCMWIFI_RCLASS_TYPE_EU:
		*rcinfo = rcinfo_eu;
		*rct_len = (uint8)ARRAYSIZE(rcinfo_eu);
		break;
	case BCMWIFI_RCLASS_TYPE_JP:
		*rcinfo = rcinfo_jp;
		*rct_len = (uint8)ARRAYSIZE(rcinfo_jp);
		break;
	case BCMWIFI_RCLASS_TYPE_CH:
		*rcinfo = rcinfo_ch;
		*rct_len = (uint8)ARRAYSIZE(rcinfo_ch);
		break;
	default:
		/* Invalid argument */
		ret = BCME_BADARG;
	}

	return ret;
}

/*
 * This mapping is taken from WCC implementation. The 802.11 spec
 * does not provide a mapping requirement if the dot11CountryString
 * does not convey the table number in the 3rd octet.
 */
bcmwifi_rclass_type_t
bcmwifi_rclass_get_rclasstype_from_cntrycode(const char *ccode)
{
	static const char EU_ccodes[][3] = {
		"EU", // European Union
		"AT", // Austria
		"BE", // Belgium
		"BG", // Bulgaria
		"HR", // Croatia
		"CY", // Cyprus
		"CZ", // Czech Republic
		"DK", // Denmark
		"EE", // Estonia
		"FI", // Finland
		"FR", // France
		"DE", // Germany
		"GR", // Greece
		"HU", // Hungary
		"IE", // Ireland
		"IT", // Italy
		"LV", // Latvia
		"LT", // Lithuania
		"LU", // Luxembourg
		"MT", // Malta
		"AN", // Netherlands Antilles
		"NL", // Netherlands
		"PL", // Poland
		"PT", // Portugal
		"RO", // Romania
		"SK", // Slovakia
		"SI", // Slovenia
		"ES", // Spain
		"SE", // Sweden
		"GB", // United Kingdom
	};
	int i;

	/*
	 * dot11CountryString may contain Annex E table number
	 * in 3rd octet. Use that if it specifies a supported
	 * table number.
	 */
	if (ccode[2] == BCMWIFI_RCLASS_TYPE_US ||
	    ccode[2] == BCMWIFI_RCLASS_TYPE_JP ||
	    ccode[2] == BCMWIFI_RCLASS_TYPE_EU ||
	    ccode[2] == BCMWIFI_RCLASS_TYPE_CH ||
	    ccode[2] == BCMWIFI_RCLASS_TYPE_GBL) {
		return ccode[2];
	} else if (!strncmp("CN", ccode, 2)) {
		/* Annex E table E-5, China */
		return BCMWIFI_RCLASS_TYPE_CH;
	} else if (ccode[0] == 'J' && (ccode[1] == 'P' ||
			(ccode[1] >= '1' && ccode[1] <= '9'))) {
		/* Annex E table E-3, Japan */
		return BCMWIFI_RCLASS_TYPE_JP;
	} else {
		for (i = 0; i < ARRAYSIZE(EU_ccodes); i++) {
			if (!strncmp(EU_ccodes[i], ccode, 2))
				/* Annex E table E-2, Europe */
				return BCMWIFI_RCLASS_TYPE_EU;
		}
	}
	/* default to Annex E table E-1, US */
	return BCMWIFI_RCLASS_TYPE_US;
}

int
bcmwifi_rclass_get_chanlist(const char *abbrev, uint8 rclass,
		wl_uint32_list_t *list)
{
	int ret = BCME_OK;
	const bcmwifi_rclass_info_t *rcinfo = NULL;
	uint8 i;
	bcmwifi_rclass_type_t type;
	uint8 list_avail_count = list->count;

	list->count = 0;

	/*
	 * First check global table for provided rclass
	 * otherwise check rclass in table for provided country
	 * Return empty chan list on failure
	 */
	ret = bcmwifi_rclass_get_rclass_info(BCMWIFI_RCLASS_TYPE_GBL, rclass, &rcinfo);
	if (ret || !rcinfo) {
		type = bcmwifi_rclass_get_rclasstype_from_cntrycode(abbrev);
		/*
		 * check for rclass in country specific op class
		 */
		ret = bcmwifi_rclass_get_rclass_info(type, rclass, &rcinfo);
		if (ret || !rcinfo) {
			goto done;
		}
	}

	/* Check for enough memory availability */
	if (list_avail_count < rcinfo->chan_set_len) {
		ret = BCME_BUFTOOSHORT;
		goto done;
	}

	/* Fill the channel in the list for the op class */
	for (i = 0; i < rcinfo->chan_set_len; i++) {
		list->element[list->count++] = rcinfo->chan_set[i];
	}

done:
	return ret;
}

/**
 * Convert a regulatory operting class band value to a chanspec band value
 *
 * @param    band    bcmwifi_rclass_band_t value
 *
 * @return   Returns the corresponding chanspec_band_t value, or INVCHANSPEC on error.
 */
chanspec_band_t
bcmwifi_rclass_band_rc2chspec(bcmwifi_rclass_band_t band)
{
	chanspec_band_t cs_band = INVCHANSPEC;
	switch (band) {
	case BCMWIFI_BAND_2G:
		cs_band = WL_CHANSPEC_BAND_2G;
		break;
	case BCMWIFI_BAND_5G:
		cs_band = WL_CHANSPEC_BAND_5G;
		break;
	case BCMWIFI_BAND_6G:
		cs_band = WL_CHANSPEC_BAND_6G;
		break;
	default:
		/* should never happen */
		ASSERT(0);
		break;
	}

	return cs_band;
}

/**
 * Convert a chanspec band value to a regulatory operting class band value
 *
 * @param    band    chanspec_band_t value, e.g. CHSPEC_BAND(chanspec)
 *
 * @return   Returns the corresponding bcmwifi_rclass_band_t value, or 0 on error.
 */
bcmwifi_rclass_band_t
bcmwifi_rclass_band_chspec2rc(chanspec_band_t band)
{
	bcmwifi_rclass_band_t rc_band = 0;
	switch (band) {
	case WL_CHANSPEC_BAND_2G:
		rc_band = BCMWIFI_BAND_2G;
		break;
	case WL_CHANSPEC_BAND_5G:
		rc_band = BCMWIFI_BAND_5G;
		break;
	case WL_CHANSPEC_BAND_6G:
		rc_band = BCMWIFI_BAND_6G;
		break;
	default:
		/* should never happen */
		ASSERT(0);
		break;
	}

	return rc_band;
}

/**
 * Convert a regulatory operting class bandwidth value to a chanspec bandwidth value
 *
 * @param    bw    bcmwifi_rclass_bw_t value
 *
 * @return   Returns the corresponding chanspec_bw_t value, or INVCHANSPEC on error.
 */
chanspec_bw_t
bcmwifi_rclass_bw_rc2chspec(bcmwifi_rclass_bw_t bw)
{
	chanspec_bw_t cs_bw = INVCHANSPEC;
	switch (bw) {
	case BCMWIFI_BW_20:
		cs_bw = WL_CHANSPEC_BW_20;
		break;
	case BCMWIFI_BW_40:
		cs_bw = WL_CHANSPEC_BW_40;
		break;
	case BCMWIFI_BW_80:
		cs_bw = WL_CHANSPEC_BW_80;
		break;
	case BCMWIFI_BW_160:
		cs_bw = WL_CHANSPEC_BW_160;
		break;
	default:
		/* should never happen */
		ASSERT(0);
		break;
	}

	return cs_bw;
}

/**
 * Convert a chanspec bandwidth value to a regulatory operting class bandwidth value
 *
 * @param    bw    chanspec bandwidth value, e.g. CHSPEC_BW(chanspec)
 *
 * @return   Returns the corresponding bcmwifi_rclass_bw_t value, or 0 on error.
 */
bcmwifi_rclass_bw_t
bcmwifi_rclass_bw_chspec2rc(chanspec_bw_t bw)
{
	bcmwifi_rclass_bw_t rc_bw = 0;
	switch (bw) {
	case WL_CHANSPEC_BW_20:
		rc_bw = BCMWIFI_BW_20;
		break;
	case WL_CHANSPEC_BW_40:
		rc_bw = BCMWIFI_BW_40;
		break;
	case WL_CHANSPEC_BW_80:
		rc_bw = BCMWIFI_BW_80;
		break;
	case WL_CHANSPEC_BW_160:
		rc_bw = BCMWIFI_BW_160;
		break;
	default:
		/* no support for 80+80MHz or other bandwidths */
		break;
	}

	return rc_bw;
}

static bool
lookup_chan_in_chan_set(const bcmwifi_rclass_info_t *entry, uint8 chan)
{
	uint8 idx = 0;

	for (idx = 0; idx < entry->chan_set_len; idx++) {
		if (entry->chan_set[idx] == chan) {
			break;
		}
	}

	return (idx < entry->chan_set_len);
}

/*
 * Return the operating class for a given chanspec
 * Input: chanspec
 *        type (op class type is US, EU, JP, GBL, or CH).
 * Output: rclass (op class)
 * On success return status BCME_OK.
 * On error return status BCME_BADCHAN, BCME_OUTOFRANGECHAN, BCME_BADARG.
 */
int
bcmwifi_rclass_get_opclass(bcmwifi_rclass_type_t type, chanspec_t chanspec,
	bcmwifi_rclass_opclass_t *rclass)
{
	int ret;
	const bcmwifi_rclass_info_t *entry = NULL;
	bcmwifi_rclass_band_t band;
	bcmwifi_rclass_bw_t bw;
	bcmwifi_rclass_flags_t sb = 0;
	uint8 rct_len = 0;
	uint8 i;
	uint8 pri_chan, center_chan;

	if ((ret = bcmwifi_rclass_get_rclass_table(type, &entry, &rct_len)) != BCME_OK) {
		return ret;
	}

	center_chan = CHSPEC_CHANNEL(chanspec);
	pri_chan = wf_chspec_primary20_chan(chanspec);

	/* get the reg class band(width) we need to search for */
	band = bcmwifi_rclass_band_chspec2rc(CHSPEC_BAND(chanspec));
	bw = bcmwifi_rclass_bw_chspec2rc(CHSPEC_BW(chanspec));

	/* get the upper/lower behavior required for 40MHz channels */
	if (CHSPEC_IS40(chanspec)) {
		if (CHSPEC_CTL_SB(chanspec) == WL_CHANSPEC_CTL_SB_U) {
			sb = BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER;
		} else {
			sb = BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER;
		}
	}

	for (i = 0; entry != NULL && i < rct_len; i++, entry++) {
		if (band != entry->band) {
			continue;
		}

		/* skip if the operating class BW does not match the chanspec */
		if (bw != entry->bw) {
			continue;
		}

		switch (entry->chan_type) {
		case BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ:
			if (lookup_chan_in_chan_set(entry, center_chan))
				goto done;
			break;
		case BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY:
			/* skip if required behavior flag is not set */
			if (bw == BCMWIFI_BW_40 && sb && (entry->flags & sb) == 0)
				continue;
			if (lookup_chan_in_chan_set(entry, pri_chan))
				goto done;
			break;
		default:
			break;
		}
	}
	return BCME_ERROR;

done:
	*rclass = entry->rclass;
	return BCME_OK;
}

int
bcmwifi_rclass_info_iter_start(bcmwifi_rclass_type_t type, const bcmwifi_rclass_info_t **rc_info)
{
	uint8 rct_len;

	*rc_info = NULL;
	if (bcmwifi_rclass_get_rclass_table(type, rc_info, &rct_len) < 0) {
		return 0;
	}

	return rct_len;
}

const bcmwifi_rclass_info_t *
bcmwifi_rclass_info_iter_next(const bcmwifi_rclass_info_t *rc_info, int remain)
{
	if (remain)
		return rc_info + 1;
	else
		return NULL;
}

/*
 * Return op class info (starting freq, bandwidth, sideband, channel set and behavior)
 * for given op class and type.
 * Input(s): type (op class type - US, EU, JP, GBL, or CH)
 *           rclass (op class)
 * Output: rcinfo (op class info entry)
 * Return status BCME_OK when no error.
 * On error, BCME_ERROR, BCME_UNSUPPORTED, or BCME_BADARG.
 */
int
bcmwifi_rclass_get_rclass_info(bcmwifi_rclass_type_t type, bcmwifi_rclass_opclass_t rclass,
	const bcmwifi_rclass_info_t **rcinfo)
{
	int ret = BCME_UNSUPPORTED;
	const bcmwifi_rclass_info_t *rct = NULL;
	uint8 rct_len = 0;
	uint8 idx = 0;

	*rcinfo = NULL;
	if ((ret = bcmwifi_rclass_get_rclass_table(type, &rct, &rct_len)) != BCME_OK) {
		goto done;
	}

	for (idx = 0; idx < rct_len; idx++) {
		const bcmwifi_rclass_info_t *entry = &rct[idx];
		if (entry->rclass == rclass) {
			*rcinfo = entry;
			ret = BCME_OK;
			goto done;
		}
	}
	*rcinfo = NULL;
done:
	return ret;
}

/*
 * Make chanspec from channel index
 */
static int
bcmwifi_rclass_get_chanspec_from_chan_idx(const bcmwifi_rclass_info_t *entry, uint8 chn_idx,
	chanspec_t *cspec)
{
	int ret = BCME_OK;
	uint16 band = WL_CHANSPEC_BAND_2G;
	uint16 sb = WL_CHANSPEC_CTL_SB_NONE;
	uint8 chan = 0;

	if (chn_idx > entry->chan_set_len) {
		ret = BCME_BADARG;
		goto done;
	}

	chan = entry->chan_set[chn_idx];
	band = bcmwifi_rclass_band_rc2chspec(entry->band);

	if (entry->bw > BCMWIFI_BW_20) {
		/* Upper and lower flags are only set for 40MHz */
		if (entry->flags & BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER) {
			sb = WL_CHANSPEC_CTL_SB_L;
			chan += CH_10MHZ_APART;
		} else if (entry->flags & BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER) {
			sb = WL_CHANSPEC_CTL_SB_U;
			chan -= CH_10MHZ_APART;
		}
	}

	*cspec = (band | sb | bcmwifi_rclass_bw_rc2chspec(entry->bw) | chan);

done:
	return ret;
}

/* Find the index of the given chan in the regclass channel set.
 * If the channel is not found, the returned index will be out of
 * range, equal to entry->chan_set_len.
 */
static uint8
lookup_chanidx_in_chan_set(const bcmwifi_rclass_info_t *entry, uint8 chan)
{
	uint8 idx;

	for (idx = 0; idx < entry->chan_set_len; idx++) {
		if (entry->chan_set[idx] == chan) {
			break;
		}
	}

	return idx;
}

/*
 * Return chanspec given op class type, op class and channel number
 * Input: type (op class type US, EU, JP, GBL, CH)
 *        rclass (op class)
 *        chn (channel)
 * Output: chanspec
 * Return BCME_OK on success and appropriate
 * on error returns  BCME_ERROR, BCME_OUTOFRANGECHAN,BCME_BADARG
 */
int
bcmwifi_rclass_get_chanspec_from_chan(bcmwifi_rclass_type_t type, bcmwifi_rclass_opclass_t rclass,
	uint8 chn, chanspec_t *cspec)
{
	int ret = BCME_ERROR;
	uint8 chn_idx;
	const bcmwifi_rclass_info_t *rcinfo = NULL;
	*cspec = 0;

	/*
	 * Get table corresponding to regulatory type and rclass
	 * If rclass is not found in the given regulatory class table,
	 * try the global table, E-4.
	 * Return empty chan list on failure
	 */
	ret = bcmwifi_rclass_get_rclass_info(type, rclass, &rcinfo);
	if (ret != BCME_OK && type != BCMWIFI_RCLASS_TYPE_GBL) {
		ret = bcmwifi_rclass_get_rclass_info(BCMWIFI_RCLASS_TYPE_GBL, rclass, &rcinfo);
	}
	if (ret != BCME_OK) {
		goto done;
	}
	chn_idx = lookup_chanidx_in_chan_set(rcinfo, chn);
	/* If chan_idx is out of range, call below will return an error */
	ret = bcmwifi_rclass_get_chanspec_from_chan_idx(rcinfo,
		chn_idx, cspec);
done:
	return ret;
}

/*
 * Return chanspec given op class type, op class and channel index
 * Input: type (op class type US, EU, JP, GBL, CH)
 *        rclass (op class)
 *        chn_idx (index in channel list)
 * Output: chanspec
 * Return BCME_OK on success and BCME_ERROR on error
 */
int
bcmwifi_rclass_get_chanspec(bcmwifi_rclass_type_t type, bcmwifi_rclass_opclass_t rclass,
	uint8 chn_idx, chanspec_t *cspec)
{
	int ret = BCME_ERROR;
	const bcmwifi_rclass_info_t *rct = NULL;
	uint8 rct_len = 0;
	uint8 idx = 0;

	if ((ret = bcmwifi_rclass_get_rclass_table(type, &rct, &rct_len)) != BCME_OK) {
		goto done;
	}

	for (idx = 0; idx < rct_len; idx++) {
		const bcmwifi_rclass_info_t *entry = &rct[idx];
		if (entry->rclass != rclass) {
			continue;
		} else {
			ret = bcmwifi_rclass_get_chanspec_from_chan_idx(entry,
					chn_idx, cspec);
			goto done;
		}
	}
done:
	return ret;
}

/*
 * Return supported op class encoded as a bitvector.
 * Input: type (op class type US, EU, JP, GBL, or CH)
 *
 * Output: rclass_bvec
 * Return status BCME_OK when no error.
 * On error, return status BCME_ERROR, BCME_BADARG, or BCME_UNSUPPORTED
 */
int
bcmwifi_rclass_get_supported_opclasses(bcmwifi_rclass_type_t type,
	const bcmwifi_rclass_bvec_t **rclass_bvec)
{
	int ret = BCME_ERROR;
	const bcmwifi_rclass_info_t *rct = NULL;
	bcmwifi_rclass_bvec_t *bvec;
	uint8 rct_len = 0;
	uint8 max = 0;
	uint8 idx;

	if ((ret = bcmwifi_rclass_get_rclass_table(type, &rct, &rct_len)) != BCME_OK) {
		goto done;
	}

	bzero(&g_bvec, sizeof(g_bvec));

	bvec = (bcmwifi_rclass_bvec_t*)&g_bvec;
	for (idx = 0; idx < rct_len; idx++) {
		bcmwifi_rclass_opclass_t opclass = rct[idx].rclass;

		max = MAX(max, (uint8)opclass);
		setbit(bvec->bvec, opclass);
	}

	/* returned bitvec length is bytecount to include the highest set bit */
	bvec->len = CEIL(max, NBBY);

	*rclass_bvec = bvec;
done:
	return ret;
}
