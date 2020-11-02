/*
 * ACPHY PHYTableInit module implementation
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id$
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_tbl.h"
#include <phy_ac.h>
#include <phy_ac_tbl.h>
#include <phy_utils_reg.h>
#include <phy_ac_info.h>
#include <wlc_phyreg_ac.h>
#include <wlc_phy_ac.h>
#include <bcmotp.h>
#include <hndpmu.h>
#include <wlc_phytbl_ac.h>
#include <phy_ac_chanmgr.h>
#include <phy_rxgcrs_api.h>

#include "wlc_phy_radio.h"
#include "wlc_radioreg_20691.h"
#include "wlc_radioreg_20693.h"

/* module private states */
struct phy_ac_tbl_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_tbl_info_t *ti;
};

/* local functions */
static int phy_ac_tbl_init(phy_type_tbl_ctx_t *ctx);
#ifdef WL11ULB
static bool wlc_phy_ac_ulb_10_capable(phy_info_t *pi);
static bool wlc_phy_ac_ulb_5_capable(phy_info_t *pi);
static bool wlc_phy_ac_ulb_2P5_capable(phy_info_t *pi);
#endif /* WL11ULB */

#if ((defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)) || \
	defined(BCMDBG_PHYDUMP)
static void phy_ac_tbl_read_table(phy_type_tbl_ctx_t *ctx,
	phy_table_info_t *ti, uint addr, uint16 *val, uint16 *qval);
static int phy_ac_tbl_dump1(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b);
static int phy_ac_tbl_dump2(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b);
static int phy_ac_tbl_dump3(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b);
static int phy_ac_tbl_dump4(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b);
#else
#define phy_ac_tbl_read_table NULL
#define phy_ac_tbl_dump1 NULL
#define phy_ac_tbl_dump2 NULL
#define phy_ac_tbl_dump3 NULL
#define phy_ac_tbl_dump4 NULL
#endif // endif

/* Register/unregister ACPHY specific implementation to common layer. */
phy_ac_tbl_info_t *
BCMATTACHFN(phy_ac_tbl_register_impl)(phy_info_t *pi, phy_ac_info_t *aci, phy_tbl_info_t *ti)
{
	phy_ac_tbl_info_t *info;
	phy_type_tbl_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_ac_tbl_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->aci = aci;
	info->ti = ti;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.init = phy_ac_tbl_init;
	fns.readtbl = phy_ac_tbl_read_table;
	fns.dump[0] = phy_ac_tbl_dump1;
	fns.dump[1] = phy_ac_tbl_dump2;
	fns.dump[2] = phy_ac_tbl_dump3;
	fns.dump[3] = phy_ac_tbl_dump4;
	fns.ctx = info;

	/* PHY-Feature specific parameter initialization */
	/* Read RFLDO from OTP */
	/* ac_info->rfldo = wlc_phy_rfldo_trim_value(pi); */

	phy_tbl_register_impl(ti, &fns);

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ac_tbl_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_tbl_unregister_impl)(phy_ac_tbl_info_t *info)
{
	phy_info_t *pi;
	phy_tbl_info_t *ti;

	ASSERT(info);
	pi = info->pi;
	ti = info->ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_tbl_unregister_impl(ti);

	phy_mfree(pi, info, sizeof(phy_ac_tbl_info_t));
}

/* h/w init/down */
static int
WLBANDINITFN(phy_ac_tbl_init)(phy_type_tbl_ctx_t *ctx)
{
	phy_ac_tbl_info_t *ti = (phy_ac_tbl_info_t *)ctx;
	phy_info_t *pi = ti->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	wlc_phy_init_acphy(pi);

	return BCME_OK;
}

#if ((defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)) || \
	defined(BCMDBG_PHYDUMP)

static phy_table_info_t acphy_tables_rev32_part1[] = {
	{  2,  0,     40},
	{  3,  1,    256},
	{  4,  0,    256},
	{  5,  1,     22},
	{  6,  0,     72},
	{  7,  0,   1407},
	{  8,  3,      9},
	{  9,  0,     64},
	{ 10,  0,   1024},
	{ 13,  1,     68},
	{ 14,  1,    512},
	{ 19,  0,    800},
	{ 20,  2,    128},
	{ 21,  0,     96},
	{ 25,  1,     64},
	{ 29,  1,     35},
	{ 31,  1,     35},
	{ 32,  1,    128},
	{ 33,  1,    128},
	{ 34,  0,    128},
	{ 35,  0,    128},
	{ 37,  1,     77},
	{ 38,  1,     35},
	{ 39,  1,     35},
	{ 40,  1,      5},
	{ 50,  0,    160},
	{ 64,  1,    128},
	{ 65,  1,    128},
	{ 66,  0,    128},
	{ 67,  0,    128},
	{0xff, 0,      0}
};

static phy_table_info_t acphy_tables_rev32_part2[] = {
	{ 68,  0,    119},
	{ 69,  0,    119},
	{ 70,  1,     22},
	{ 72,  1,    128},
	{ 78,  1,    256},
	{ 88,  0,     72},
	{ 89,  0,     16},
	{ 91,  0,     64},
	{ 92,  0,     64},
	{ 94,  0,    105},
	{ 96,  1,    128},
	{ 97,  1,    128},
	{ 98,  0,    128},
	{ 99,  0,    128},
	{100,  0,    119},
	{101,  0,    119},
	{102,  1,     22},
	{104,  1,    128},
	{110,  1,    256},
	{114,  1,    183},
	{115,  1,      8},
	{126,  0,    105},
	{127,  1,     11},
	{128,  1,    128},
	{129,  1,    128},
	{130,  0,    128},
	{131,  0,    128},
	{132,  0,    119},
	{133,  0,    119},
	{134,  1,     22},
	{136,  1,    128},
	{0xff, 0,      0}
};

static phy_table_info_t acphy_tables_rev32_part3[] = {
	{142,  1,    256},
	{146,  1,    183},
	{147,  1,      8},
	{158,  0,    105},
	{159,  1,     11},
	{160,  1,    128},
	{161,  1,    128},
	{162,  0,    128},
	{163,  0,    128},
	{164,  0,    119},
	{165,  0,    119},
	{166,  1,     22},
	{168,  1,    128},
	{174,  1,    256},
	{178,  1,    183},
	{179,  1,      8},
	{190,  0,    105},
	{191,  1,     11},
	{210,  1,    183},
	{211,  1,      8},
	{223,  1,     11},
	{224,  1,     64},
	{225,  1,     64},
	{226,  0,     24},
	{0xff, 0,      0}
};

static phy_table_info_t acphy_tables_rev33_part1[] = {
	{  2,  0,     40},
	{  3,  1,    256},
	{  4,  0,    256},
	{  5,  1,     22},
	{  6,  0,     72},
	{  7,  0,   1407},
	{  8,  3,      9},
	{  9,  0,     64},
	{ 10,  0,   1024},
	{ 13,  1,     68},
	{ 14,  1,    512},
	{ 19,  0,    800},
	{ 20,  2,    128},
	{ 21,  0,     96},
	{ 25,  1,     64},
	{ 29,  1,     35},
	{ 31,  1,     35},
	{ 37,  1,     77},
	{ 38,  1,     35},
	{ 39,  1,     35},
	{ 40,  1,      5},
	{ 50,  0,    160},
	{ 64,  1,    128},
	{ 65,  1,    128},
	{ 66,  0,    128},
	{ 67,  0,    128},
	{0xff, 0,      0}
};

static phy_table_info_t acphy_tables_rev33_part2[] = {
	{ 68,  0,    119},
	{ 69,  0,    119},
	{ 70,  1,     22},
	{ 72,  1,    128},
	{ 78,  1,    256},
	{ 88,  0,     72},
	{ 89,  0,     16},
	{ 91,  0,     64},
	{ 92,  0,     64},
	{ 94,  0,    105},
	{ 96,  1,    128},
	{ 97,  1,    128},
	{ 98,  0,    128},
	{ 99,  0,    128},
	{100,  0,    119},
	{101,  0,    119},
	{102,  1,     22},
	{104,  1,    128},
	{110,  1,    256},
	{113,  1,     64},
	{115,  1,      8},
	{126,  0,    105},
	{127,  1,     11},
	{128,  1,    128},
	{129,  1,    128},
	{130,  0,    128},
	{131,  0,    128},
	{132,  0,    119},
	{133,  0,    119},
	{134,  1,     22},
	{136,  1,    128},
	{0xff, 0,      0}
};

static phy_table_info_t acphy_tables_rev33_part3[] = {
	{142,  1,    256},
	{147,  1,      8},
	{158,  0,    105},
	{159,  1,     11},
	{160,  1,    128},
	{161,  1,    128},
	{162,  0,    128},
	{163,  0,    128},
	{164,  0,    119},
	{165,  0,    119},
	{166,  1,     22},
	{168,  1,    128},
	{174,  1,    256},
	{179,  1,      8},
	{190,  0,    105},
	{191,  1,     11},
	{211,  1,      8},
	{223,  1,     11},
	{224,  1,     64},
	{225,  1,     64},
	{226,  0,     24},
	{11,   1,     22},
	{15,   1,    128},
	{54,   1,    128},
	{55,   0,     16},
	{56,   0,    128},
	{57,   0,     96},
	{58,   0,     24},
	{59,   0,     11},
	{60,   0,     77},
	{61,   1,     16},
	{0xff, 0,      0}
};

static phy_table_info_t acphy_tables_rev33_part4[] = {
	{320,  0,    247},
	{321,  1,     36},
	{322,  1,     24},
	{323,  0,     24},
	{324,  0,    105},
	{325,  0,    119},
	{326,  0,    119},
	{327,  1,     22},
	{328,  1,     11},
	{352,  0,    247},
	{353,  1,     36},
	{354,  1,     24},
	{355,  1,     24},
	{356,  0,    105},
	{357,  0,    119},
	{358,  0,    119},
	{359,  1,     22},
	{360,  1,     11},
	{384,  0,    247},
	{385,  1,     36},
	{386,  1,     24},
	{387,  0,     24},
	{388,  0,    105},
	{389,  0,    119},
	{390,  0,    119},
	{391,  1,     22},
	{392,  1,     11},
	{416,  0,    247},
	{417,  1,     36},
	{418,  1,     24},
	{419,  0,     24},
	{420,  0,    105},
	{421,  0,    119},
	{422,  0,    119},
	{423,  1,     22},
	{424,  1,     11},
	{0xff, 0,      0}
};

static phy_table_info_t acphy_tables_rev12_rsdb[] = {
	{2,	0,	40},
	{3,	1,	256},
	{4,	0,	256},
	{5,	1,	22},
	{6,	0,	72},
	{7,	0,	1136},
	{8,	3,	9},
	{9,	0,	48},
	{10,	0,	768},
	{12,	0,	160},
	{13,	1,	68},
	{14,	1,	512},
	{19,	0,	800},
	{20,	2,	128},
	{24,	1,	520},
	{25,	1,	64},
	{29,	1,	35},
	{31,	1,	35},
	{35,	1,	77},
	{36,	1,	35},
	{37,	1,	35},
	{38,	1,	5},
	{64,	1,	128},
	{65,	1,	128},
	{66,	0,	128},
	{67,	0,	128},
	{68,	0,	119},
	{69,	0,	119},
	{70,	1,	22},
	{71,	1,	512},
	{72,	1,	128},
	{73,	1,	1024},
	{78,	1,	256},
	{79,	2,	64},
	{80,	0,	8},
	{85,	0,	128},
	{86,	0,	128},
	{87,	0,	128},
	{88,	0,	72},
	{89,	0,	16},
	{90,	0,	64},
	{91,	0,	64},
	{92,	0,	64},
	{93,	0,	128},
	{94,	0,	105},
	{105,	1,	1024},
	{113,	1,	64},
	{114,	0,	24},
	{115,	1,	8},
	{116,	1,	64},
	{128,	2,	128},
	{129,	1,	64},
	{148,	1,	64},
	{0xff,	0,	0}
};

static phy_table_info_t acphy_tables_rev12_mimo_80p80[] = {
	{2,	0,	40},
	{3,	1,	256},
	{4,	0,	256},
	{5,	1,	22},
	{6,	0,	72},
	{7,	0,	1136},
	{8,	3,	9},
	{9,	0,	48},
	{10,	0,	768},
	{12,	0,	160},
	{13,	1,	68},
	{14,	1,	512},
	{19,	0,	800},
	{20,	2,	128},
	{23,	1,	520},
	{24,	1,	520},
	{25,	1,	64},
	{27,	0,	46},
	{29,	1,	35},
	{30,	1,	64},
	{31,	1,	35},
	{35,	1,	77},
	{36,	1,	35},
	{37,	1,	35},
	{38,	1,	5},
	{44,	0,	160},
	{64,	1,	128},
	{65,	1,	128},
	{66,	0,	128},
	{67,	0,	128},
	{68,	0,	119},
	{69,	0,	119},
	{70,	1,	22},
	{71,	1,	512},
	{72,	1,	128},
	{73,	1,	1024},
	{78,	1,	256},
	{79,	2,	64},
	{80,	0,	8},
	{85,	0,	128},
	{86,	0,	128},
	{87,	0,	128},
	{88,	0,	72},
	{89,	0,	16},
	{90,	0,	64},
	{91,	0,	64},
	{92,	0,	64},
	{93,	0,	128},
	{94,	0,	105},
	{96,	1,	128},
	{97,	1,	128},
	{98,	0,	128},
	{99,	0,	128},
	{100,	0,	119},
	{101,	0,	119},
	{102,	1,	22},
	{103,	1,	512},
	{104,	1,	128},
	{105,	1,	1024},
	{110,	1,	256},
	{111,	2,	64},
	{113,	1,	64},
	{114,	0,	24},
	{115,	1,	8},
	{116,	1,	64},
	{117,	0,	128},
	{118,	0,	128},
	{119,	0,	128},
	{120,	0,	72},
	{121,	0,	16},
	{122,	0,	64},
	{123,	0,	64},
	{124,	0,	64},
	{125,	0,	128},
	{126,	0,	105},
	{128,	2,	128},
	{129,	1,	64},
	{147,	1,	8},
	{148,	1,	64},
	{160,	2,	128},
	{161,	1,	64},
	{0xff,	0,	0}
};

static phy_table_info_t acphy_tables_rev4[] = {
	{  1, 0,  128},
	{  2, 0,   40},
	{  3, 1,  256},
	{  4, 0,  256},
	{  5, 1,   22},
	{  6, 0,   72},
	{  7, 0, 1136},
	{  8, 3,    9},
	{  9, 0,   48},
	{ 10, 0,  768},
	{ 12, 0,  160},
	{ 13, 1,   68},
	{ 14, 1,  512},
	{ 15, 1,  128},
	{ 16, 1, 8784},
	{ 17, 2, 464},
	{ 18, 1, 1920},
	{ 19, 0,  800},
	{ 20, 2,  128},
	{ 21, 0,   72},
	{ 22, 0,   64},
	{ 23, 1,  520},
	{ 24, 1,  520},
	{ 25, 1,  64},
	{ 26, 1,  22},
	{ 27, 0,   46},
	{ 28, 0,   46},
	{ 29, 1,   35},
	{ 30, 1,   64},
	{ 31, 1,   35},
	{ 32, 2,  128},
	{ 33, 1,   24},
	{ 34, 2,   512},
	{ 35, 1,   77},
	{ 64, 0,  128},
	{ 65, 1,  128},
	{ 66, 0,  128},
	{ 67, 0,  128},
	{ 68, 0,  119},
	{ 69, 0,  119},
	{ 70, 1,   22},
	{ 71, 1,  512},
	{ 72, 1,  128},
	{ 73, 1, 1024},
	{ 80, 0,    8},
	{ 81, 0,  128},
	{ 82, 0,  128},
	{ 83, 0,  128},
	{ 84, 0,  128},
	{ 85, 0,  128},
	{ 86, 0,  128},
	{ 87, 0,  128},
	{ 88, 0,  72},
	{ 89, 0,  16},
	{ 90, 0,  64},
	{ 91, 0,  64},
	{ 92, 0,  64},
	{ 93, 0,  128},
	{ 94, 0,  105},
	{ 96, 0,  128},
	{ 97, 1,  128},
	{ 98, 0,  128},
	{ 99, 0,  128},
	{100, 0,  119},
	{101, 0,  119},
	{102, 1,   22},
	{103, 1,  512},
	{104, 1,  128},
	{105, 1, 1024},
	{112, 1,   64},
	{113, 1,   64},
	{117, 0,   128},
	{118, 0,   128},
	{119, 0,   128},
	{120, 0,  72},
	{121, 0,  16},
	{122, 0,  64},
	{123, 0,  64},
	{124, 0,  64},
	{125, 0,  128},
	{126, 0,  105},
	{128, 0,  128},
	{129, 1,  128},
	{130, 0,  128},
	{131, 0,  128},
	{132, 0,  119},
	{133, 0,  119},
	{134, 1,   22},
	{135, 1,  512},
	{136, 1,  128},
	{137, 1, 1024},
	{149, 0,   128},
	{150, 0,   128},
	{151, 0,   128},
	{152, 0,   72},
	{153, 0,   16},
	{154, 0,   64},
	{155, 0,   64},
	{156, 0,   64},
	{157, 0,   128},
	{158, 0,   105},
	{ 0xff,	0,	0 }
};

static phy_table_info_t acphy_tables_rev3[] = {
	{  1, 0,  128},
	{  2, 0,   40},
	{  3, 0,  256},
	{  4, 0,  256},
	{  5, 1,   22},
	{  6, 0,   72},
	{  7, 0, 1136},
	{  8, 3,    9},
	{  9, 0,   48},
	{ 10, 0,  320},
	{ 11, 0,  105},
	{ 12, 0,  160},
	{ 13, 1,   68},
	{ 14, 1,  512},
	{ 15, 1,  128},
	{ 16, 1, 8784},
	{ 18, 1, 1920},
	{ 19, 0,  512},
	{ 20, 2,  128},
	{ 21, 0,   72},
	{ 22, 0,   64},
	{ 23, 1,  520},
	{ 24, 1,  520},
	{ 27, 0,   46},
	{ 28, 0,   46},
	{ 29, 1,   35},
	{ 30, 1,   64},
	{ 32, 2,  128},
	{ 33, 1,   24},
	{ 64, 0,  128},
	{ 65, 1,  128},
	{ 66, 0,  128},
	{ 67, 0,  128},
	{ 68, 0,  119},
	{ 69, 0,  119},
	{ 70, 1,   22},
	{ 71, 1,  128},
	{ 72, 1,  128},
	{ 73, 1, 1024},
	{ 80, 0,    8},
	{ 81, 0,  128},
	{ 82, 0,  128},
	{ 83, 0,  128},
	{ 84, 0,  128},
	{ 96, 0,  128},
	{ 97, 1,  128},
	{ 98, 0,  128},
	{ 99, 0,  128},
	{100, 0,  119},
	{101, 0,  119},
	{102, 1,   22},
	{103, 1,  128},
	{104, 1,  128},
	{105, 1, 1024},
	{ 0xff,	0,	0 }
};
static phy_table_info_t acphy2_tables_1[] = {
	{ 2, 0,   40},
	{ 3, 1,  256},
	{ 4, 0,  256},
	{ 7, 0, 1136},
	{ 8, 3,    9},
	{ 9, 0,   16},
	{10, 0,  256},
	{11, 0,  105},
	{12, 0,  160},
	{13, 1,   68},
	{14, 1,  512},
	{19, 0,  512},
	{20, 2,  128},
	{21, 0,   24},
	{22, 0,   35},
	{25, 1,   64},
	{26, 1,   22},
	{29, 1,   35},
	{31, 1,   35},
	{32, 2,  128},
	{33, 0,   24},
	{34, 0,  512},

	{0xff, 0, 0}
};
static phy_table_info_t acphy2_tables_2[] = {
	{64, 0,  128},
	{65, 1,  128},
	{66, 0,  128},
	{67, 0,  128},
	{68, 0,  119},
	{69, 0,  119},
	{70, 1,   19},
	{71, 1,   64},
	{72, 1,   64},
	{73, 1, 1024},
	{80, 0,    8},
	{81, 0,  128},
	{82, 0,  128},
	{83, 0,  128},
	{84, 0,  128},

	{0xff, 0, 0}
};
static phy_table_info_t acphy_tables[] = {
	{  1, 0,  128},
	{  2, 0,   40},
	{  3, 1,  256},
	{  4, 0,  256},
	{  5, 1,   22},
	{  6, 0,   72},
	{  7, 0, 1136},
	{  8, 3,    9},
	{  9, 0,   48},
	{ 10, 0,   96},
	{ 11, 0,  105},
	{ 12, 0,  160},
	{ 13, 1,   68},
	{ 14, 1,  512},
	{ 15, 1,  128},
	{ 16, 1, 8784},
	{ 17, 2,  464},
	{ 18, 1, 1920},
	{ 19, 0,  512},
	{ 20, 2,  128},
	{ 21, 0,   72},
	{ 22, 0,   64},
	{ 23, 1,  520},
	{ 24, 1,  520},
	{ 32, 2,  128},
	{ 33, 1,   24},
	{ 64, 0,  128},
	{ 65, 1,  128},
	{ 66, 0,  128},
	{ 67, 0,  128},
	{ 68, 0,  119},
	{ 69, 0,  119},
	{ 70, 1,   19},
	{ 71, 1,   64},
	{ 72, 1,   64},
	{ 73, 1, 1024},
	{ 80, 0,    8},
	{ 81, 0,  128},
	{ 82, 0,  128},
	{ 83, 0,  128},
	{ 84, 0,  128},
	{ 96, 0,  128},
	{ 97, 1,  128},
	{ 98, 0,  128},
	{ 99, 0,  128},
	{100, 0,  119},
	{101, 0,  119},
	{102, 1,   19},
	{103, 1,   64},
	{104, 1,   64},
	{105, 1, 1024},
	{128, 0,  128},
	{129, 1,  128},
	{130, 0,  128},
	{131, 0,  128},
	{132, 0,  119},
	{133, 0,  119},
	{134, 1,   19},
	{135, 1,   64},
	{136, 1,   64},
	{137, 1, 1024},
	{ 0xff,	0,	0 }
};

static void
phy_ac_tbl_read_table(phy_type_tbl_ctx_t *ctx,
	phy_table_info_t *ti, uint addr, uint16 *val, uint16 *qval)
{
	phy_info_t *pi = (phy_info_t *)ctx;

	phy_utils_write_phyreg(pi, ACPHY_TableID(pi->pubpi.phy_rev), (uint16)ti->table);
	phy_utils_write_phyreg(pi, ACPHY_TableOffset(pi->pubpi.phy_rev), (uint16)addr);

	if (ti->q == 3) {
		*qval++ = phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi.phy_rev));
		*qval++ = phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi.phy_rev));
		*qval = phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi.phy_rev));
		*val = phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi.phy_rev));
	} else if (ti->q == 2) {
		*qval++ = phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi.phy_rev));
		*qval = phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi.phy_rev));
		*val = phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi.phy_rev));
	} else if (ti->q == 1) {
		*qval = phy_utils_read_phyreg(pi, ACPHY_TableDataLo(pi->pubpi.phy_rev));
		*val = phy_utils_read_phyreg(pi, ACPHY_TableDataHi(pi->pubpi.phy_rev));
	} else {
		*val = phy_utils_read_phyreg(pi, ACPHY_TableDataLo(pi->pubpi.phy_rev));
	}
}

static int
phy_ac_tbl_dumptbl(phy_ac_tbl_info_t *aci, uint tbl, struct bcmstrbuf *b)
{
	phy_info_t *pi = aci->pi;
	phy_table_info_t *ti = NULL;

	wlc_phy_stay_in_carriersearch_acphy(pi, TRUE);

	if (!(ACMAJORREV_1(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
		ACMAJORREV_33(pi->pubpi.phy_rev)) && tbl != 1) {
		PHY_ERROR(("There is only one table\n"));
		return BCME_UNSUPPORTED;
	}

	if (ACMAJORREV_32(pi->pubpi.phy_rev)) {
		if (tbl == 1) {
			ti = acphy_tables_rev32_part1;
		} else if (tbl == 2) {
			ti = acphy_tables_rev32_part2;
		} else if (tbl == 3) {
			ti = acphy_tables_rev32_part3;
		} else {
			ti = acphy_tables_rev32_part1;
		}
	} else if (ACMAJORREV_33(pi->pubpi.phy_rev)) {
		if (tbl == 1) {
			ti = acphy_tables_rev33_part1;
		} else if (tbl == 2) {
			ti = acphy_tables_rev33_part2;
		} else if (tbl == 3) {
			ti = acphy_tables_rev33_part3;
		} else if (tbl == 4) {
			ti = acphy_tables_rev33_part4;
		} else {
			ti = acphy_tables_rev33_part1;
		}
	} else if (ACMAJORREV_5(pi->pubpi.phy_rev)) {
		ti = acphy_tables_rev4;
	} else if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
		uint16 phymode = phy_get_phymode(pi);
		if (phymode == PHYMODE_RSDB)
			ti = acphy_tables_rev12_rsdb;
		else if ((phymode == PHYMODE_MIMO) || (phymode == PHYMODE_80P80))
			ti = acphy_tables_rev12_mimo_80p80;
		else
			ASSERT(0);
	} else if (ACMAJORREV_2(pi->pubpi.phy_rev)) {
			ti = acphy_tables_rev3;
	} else if (ACMAJORREV_1(pi->pubpi.phy_rev)) {
		if (tbl == 1) {
			ti = acphy2_tables_1;
		} else if (tbl == 2) {
			ti = acphy2_tables_2;
		}
	} else {
		ti = acphy_tables;
	}

	phy_tbl_do_dumptbl(aci->ti, ti, b);

	if (ACMAJORREV_1(pi->pubpi.phy_rev) && tbl == 1) {
		bcm_bprintf(b, "Please run \"wl dump phytbl2\" to get the remaining tables\n");
	} else if (ACMAJORREV_32(pi->pubpi.phy_rev) && tbl < 3) {
		bcm_bprintf(b, "Please run \"wl dump phytbl2; wl dump phytbl3\"");
		bcm_bprintf(b, "to get the complete tables\n");
	} else if (ACMAJORREV_33(pi->pubpi.phy_rev) && tbl < 4) {
		bcm_bprintf(b, "Please run \"wl dump phytbl2; wl dump phytbl3; wl dump phytbl4\"");
		bcm_bprintf(b, "to get the complete tables\n");
	}

	wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);

	return BCME_OK;
}

static int
phy_ac_tbl_dump1(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b)
{
	return phy_ac_tbl_dumptbl((phy_ac_tbl_info_t *)ctx, 1, b);
}

static int
phy_ac_tbl_dump2(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b)
{
	return phy_ac_tbl_dumptbl((phy_ac_tbl_info_t *)ctx, 2, b);
}

static int
phy_ac_tbl_dump3(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b)
{
	return phy_ac_tbl_dumptbl((phy_ac_tbl_info_t *)ctx, 3, b);
}

static int
phy_ac_tbl_dump4(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b)
{
	return phy_ac_tbl_dumptbl((phy_ac_tbl_info_t *)ctx, 4, b);
}
#endif // endif

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */
static void wlc_phy_edcrs_thresh_acphy(phy_info_t *pi);
static bool wlc_phy_ac_proprietary_rates(phy_info_t *pi);
static bool wlc_phy_ac_stbc_capable(phy_info_t *pi);
#ifdef WL11AC_160
static bool wlc_phy_ac_160_capable(phy_info_t *pi);
#endif // endif
#ifdef WL11AC_80P80
static bool wlc_phy_ac_80p80_capable(phy_info_t *pi);
#endif // endif

#ifdef WL_PROXDETECT
static void wlc_phy_tof_reset_acphy(phy_info_t *pi);
#endif // endif

static void
WLBANDINITFN(wlc_phy_edcrs_thresh_acphy)(phy_info_t *pi)
{
	int32 assert_dBm;
	phy_info_acphy_t *pi_ac;
	pi_ac = pi->u.pi_acphy;

	pi_ac->sromi->ed_thresh_default = -69; /* EU Adaptivity */
	if (pi_ac->sromi->ed_thresh2g) {
		assert_dBm = pi_ac->sromi->ed_thresh2g;
	} else {
		assert_dBm = pi_ac->sromi->ed_thresh_default;
	}

	wlc_phy_adjust_ed_thres_acphy(pi, &assert_dBm, TRUE);
}

/** Returns TRUE if PHY is capable of VHT Proprietary Rates */
static bool
wlc_phy_ac_proprietary_rates(phy_info_t *pi)
{
	return ((ACREV_IS(pi->pubpi.phy_rev, 1)) ||
		(ACREV_IS(pi->pubpi.phy_rev, 3)) ||
		(ACREV_GE(pi->pubpi.phy_rev, 6)));
}

/** Returns TRUE if PHY is capable of 11n Proprietary Rates */
static bool
wlc_phy_ac_ht_proprietary_rates(phy_info_t *pi)
{
	/* 43602 : no capability register available
	 *
	 *  4349 : miscSigCtrl.brcm_11n_256qam_support must be set in order to use 11n 256Q rates
	 *         Currently, 4349 supports 11n 256Q rates only on 2G 20Mhz channels
	 */

	return (CHIPID(pi->sh->chip) == BCM43602_CHIP_ID) ||
		(CHIPID(pi->sh->chip) == BCM43462_CHIP_ID) ||
		BCM4349_CHIP(pi->sh->chip);
}

/** Return True if PHY is capable of STBC. Currently returns 1 for all ACPHY except 4335A0/B0 */
static bool
wlc_phy_ac_stbc_capable(phy_info_t *pi)
{
	return !(ACMAJORREV_1(pi->pubpi.phy_rev) &&
	(ACMINORREV_0(pi) || ACMINORREV_1(pi)));
}

#ifdef WL11AC_160
/** Return True if PHY is capable of 160 */
static bool wlc_phy_ac_160_capable(phy_info_t *pi)
{
	/* Disable 160Mhz support for 4365 3x3 */
	if (BCM4365_CHIP(pi->sh->chip) &&
		pi->sromi->sr13_en_sw_txrxchain_mask &&
		(pi->sromi->sw_txchain_mask != 0xf) &&
		(pi->sromi->sw_rxchain_mask != 0xf)) {
		return FALSE;
	} else {
		return (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_GE33(pi->pubpi.phy_rev))
			? TRUE: FALSE;
	}
}
#endif /* WL11AC_160 */

#ifdef WL11AC_80P80
/** Return True if PHY is capable of 80p80 */
static bool wlc_phy_ac_80p80_capable(phy_info_t *pi)
{
	/* Disable 160Mhz support for 4365 3x3 */
	if (BCM4365_CHIP(pi->sh->chip) &&
		pi->sromi->sr13_en_sw_txrxchain_mask &&
		(pi->sromi->sw_txchain_mask != 0xf) &&
		(pi->sromi->sw_rxchain_mask != 0xf)) {
		return FALSE;
	} else {
		return (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_GE33(pi->pubpi.phy_rev))
			? TRUE: FALSE;
	}
}
#endif /* WL11AC_80P80 */

static bool wlc_phy_ac_mu_bfr_capable(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	return (pi_ac->phy_caps & PHY_CAP_MU_BFR) ? TRUE : FALSE;
}

static bool wlc_phy_ac_mu_bfe_capable(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	return (pi_ac->phy_caps & PHY_CAP_MU_BFE) ? TRUE : FALSE;
}

static bool wlc_phy_ac_1024qam_capable(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	return (pi_ac->phy_caps & PHY_CAP_1024QAM) ? TRUE : FALSE;
}

#ifdef WL11ULB
static bool wlc_phy_ac_ulb_10_capable(phy_info_t *pi)
{
	if ((ACMAJORREV_4(pi->pubpi.phy_rev)) ||
		(ACMAJORREV_32(pi->pubpi.phy_rev)) ||
		(ACMAJORREV_33(pi->pubpi.phy_rev)) ||
	(ACMAJORREV_3(pi->pubpi.phy_rev) && ACMINORREV_6(pi)))
		/* No phycapability bit indicating ulb capable for 4349 */
		/* 10MHz mode is supported in 4349B0 and 4364 1x1 PHYs */
		return TRUE;
	else
		return FALSE;
}

static bool wlc_phy_ac_ulb_5_capable(phy_info_t *pi)
{
	if ((ACMAJORREV_4(pi->pubpi.phy_rev)) ||
		(ACMAJORREV_32(pi->pubpi.phy_rev)) ||
		(ACMAJORREV_33(pi->pubpi.phy_rev)) ||
	(ACMAJORREV_3(pi->pubpi.phy_rev) && ACMINORREV_6(pi)))
		/* No phycapability bit indicating ulb capable for 4349 */
		/* 5MHz mode is supported in 4349B0 and 4364 1x1 PHYs */
		return TRUE;
	else
		return FALSE;
}

static bool wlc_phy_ac_ulb_2P5_capable(phy_info_t *pi)
{
	if (ACMAJORREV_3(pi->pubpi.phy_rev) && ACMINORREV_6(pi))
		/* 2.5MHz mode is supported in 4364 1x1 PHY */
		return TRUE;
	else
		/* 4349B0 doesnot support 2.5 MHz */
		return FALSE;
}
#endif /* WL11ULB */

#ifdef WL_PROXDETECT
void wlc_phy_tof_reset_acphy(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;

	pi_ac->tof_setup_done = FALSE;
	pi_ac->tof_active = FALSE;
	pi_ac->tof_smth_forced = FALSE;
	pi_ac->tof_rfseq_bundle_offset = 0;
	pi_ac->tof_shm_ptr = (wlapi_bmac_read_shm(pi->sh->physhim, M_TOF_PTR) * 2);
}
#endif // endif

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
void
BCMATTACHFN(wlc_phy_rfldo_trim_value)(phy_info_t *pi)
{

	uint8 otp_select;
	uint16 otp = 0;

	uint32 sromctl;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	sromctl = si_get_sromctl(pi->sh->sih);
	otp_select = (sromctl >> 4) & 0x1;
	if (otp_select == 0)
		si_set_sromctl(pi->sh->sih, sromctl | (1 << 4));
	otp_read_word(pi->sh->sih, 16, &otp);
	if (otp_select == 0)
		si_set_sromctl(pi->sh->sih, sromctl);
	otp = (otp >> 8) & 0x1f;

	pi->u.pi_acphy->rfldo = otp;
}

void
WLBANDINITFN(wlc_phy_init_acphy)(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	uint8 region_group = wlc_phy_get_locale(pi->rxgcrsi);
	uint32 rfldo = 0;
	uint8 phyver;
#ifdef PREASSOC_PWRCTRL
	uint8 core;
#endif // endif
#ifdef ENABLE_FCBS
	int chanidx;
	uint8 iqlocal_tbl_id = wlc_phy_get_tbl_id_iqlocal(pi, 0);
	uint32 tmp_val[160];
	uint8 stall_val;
	chanidx = 0;
	tmp_val[0] = 0;
	stall_val = 0;
#endif // endif
#if defined(WL_BEAMFORMING)
	uint32 txbf_stall_val;
	uint32 mlbf_lut[64] = {0x19003af, 0x19003af, 0x1d10390, 0x8f03f6, 0x12b03d3, 0x8f03f6,
		0xe603e6, 0x11a03d8, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x400, 0x400,
		0x400, 0x400, 0x19003af, 0x19003af, 0x1d10390, 0x8f03f6, 0x12b03d3, 0x8f03f6,
		0xe603e6, 0x11a03d8, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x400, 0x400,
		0x400, 0x400, 0x18003b5, 0x18003b5, 0x1a003a7, 0xb203f0, 0xf803e2, 0x8f03f6,
		0xc303ed, 0xe603e6, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x400, 0x400,
		0x400, 0x400, 0x1d10390, 0x1d10390, 0x2d402d4, 0x400, 0x23d0351, 0x12b03d3,
		0x1f00380, 0x22e035b, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x400,
		0x400, 0x400, 0x400};
	uint32 bfmuserdx_lut[8] = {0x11141000, 0x133c1228, 0x15641450, 0x178c1678, 0x19b418a0,
		0x1bdc1ac8, 0x1e041cf0, 0x1f18};
	uint32 mu_vmaddr[1] = {0x0e000c00};
#endif // endif
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* update corenum and coremask state variables */
	if (ACMAJORREV_4(pi->pubpi.phy_rev))
		phy_ac_update_phycorestate(pi);

#ifdef WL_PROXDETECT
	wlc_phy_tof_reset_acphy(pi);
#endif // endif

	if (ACMAJORREV_5(pi->pubpi.phy_rev) ||
	    (ACMAJORREV_2(pi->pubpi.phy_rev) && !ACMINORREV_0(pi)) ||
	    ACMAJORREV_GE32(pi->pubpi.phy_rev)) {
		ACPHY_ENABLE_STALL(pi, 0);
	}

	/* Enable VHT prorietary rates if the PHY supports it */
	if (wlc_phy_ac_proprietary_rates(pi)) {
		MOD_PHYREG(pi, HTSigTones, ldpc_proprietary_mcs_vht, 1);
	}

	phyver = READ_PHYREGFLD(pi, Version, version);
	if (phyver <= 1) {
		if (phyver == 0) {
			/* 4360a0 */
			if (pi_ac->rfldo == 0) {
				/* Use rfldo = 1.26 V for phyver = 0 by default */
				rfldo = 5;
			} else {
				rfldo = pi_ac->rfldo;
			}
		} else {
			/* 4360b0 */
			if (pi_ac->rfldo <= 3) {
				rfldo = 0;
			} else {
				rfldo = pi_ac->rfldo - 3;
			}
		}

		rfldo = rfldo << 20;
		si_pmu_regcontrol(pi->sh->sih, 0, 0x1f00000, rfldo);
	}

#ifdef PREASSOC_PWRCTRL
	FOREACH_CORE(pi, core) {
		pi_ac->pwr_ctrl_save->status_idx_carry_2g[core] = FALSE;
		pi_ac->pwr_ctrl_save->status_idx_carry_5g[core] = FALSE;
	}
#endif // endif
	/* Check if board uses internal 3.3V LDOs to supply the iPAs
	 * and enable the LDOs if used
	 */
	if ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_PALDO) != 0) {
		if (BCM4350_CHIP(pi->sh->chip)) {
			si_pmu_regcontrol(pi->sh->sih, 0x7, 0x100, 0x100);
		}
	}

	/* Start with PHY not controlling any gpio's */
	si_gpiocontrol(pi->sh->sih, 0xffff, 0, GPIO_DRV_PRIORITY);

	if (!ACMAJORREV_3(pi->pubpi.phy_rev)) {
		/* Only supported in ucode for mac revid 40 and 42 */
		/* ucode hirssi detect - bypass lna1 to save it */
		wlc_phy_hirssi_elnabypass_init_acphy(pi);
	}

	if ((CHIPID(pi->sh->chip) == BCM43602_CHIP_ID) ||
	    (CHIPID(pi->sh->chip) == BCM43462_CHIP_ID)) {
		/* JIRA:HW43602-197 WAR:
		 * start with disabling PAVREF programming by ucode;
		 * it will be enabled for MCH2/MCH5 boards later on
		 */
		W_REG(pi->sh->osh, &pi->regs->psm_int_sel_1, 0x0);
	}
	/* Initialize deaf_count */
	pi_ac->deaf_count = 0;

	/* Init regs/tables only once that do not get reset on phy_reset */
	wlc_phy_set_regtbl_on_pwron_acphy(pi);

	/* Call chan_change with default chan */
	pi_ac->init = TRUE;
	pi_ac->bt_sw_state = AUTO;

	if (pi->phy_init_por) {
		pi_ac->mdgain_trtx_allowed = FALSE;
	}

	wlc_phy_chanspec_set_acphy(pi, pi->radio_chanspec);

	pi_ac->init = FALSE;
	pi_ac->init_done = TRUE;
	/* Sets Assert and Deassert thresholds for all 20MHz subbands for EDCRS */

	/* edcrs phyreg is shdowed.
	 * For shadowed reg/table, initialed values have to be put on both sets.
	 */
#ifdef ENABLE_FCBS
	if (IS_FCBS(pi)) {
		for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
			wlc_phy_prefcbsinit_acphy(pi, chanidx);
			wlc_phy_edcrs_thresh_acphy(pi);
		}
		wlc_phy_prefcbsinit_acphy(pi, 0);
	} else {
		wlc_phy_edcrs_thresh_acphy(pi);
	}
#else
	wlc_phy_edcrs_thresh_acphy(pi);
#endif // endif

	if (region_group ==  REGION_EU)
	    wlc_phy_set_srom_eu_edthresh_acphy(pi);

	if (ACMAJORREV_0(pi->pubpi.phy_rev) || ACMAJORREV_3(pi->pubpi.phy_rev) ||
	    (ACMAJORREV_1(pi->pubpi.phy_rev) && ACMINORREV_2(pi)) ||
	    ACMAJORREV_5(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
	    ACMAJORREV_33(pi->pubpi.phy_rev)) {
		/* SWWLAN-58194 */
		MOD_PHYREG(pi, timeoutEn, cckpaydecodetimeoutEn, 1);
		MOD_PHYREG(pi, timeoutEn, ofdmpaydecodetimeoutEn, 1);
		MOD_PHYREG(pi, timeoutEn, resetRxontimeout, 1);
		WRITE_PHYREG(pi, ofdmpaydecodetimeoutlen, 0x7d0);
		WRITE_PHYREG(pi, cckpaydecodetimeoutlen, 0x7d0);
	}

	if (pi->phy_init_por)
		pi->interf->curr_home_channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	if (!(ACMAJORREV_4(pi->pubpi.phy_rev))) {
		if (TINY_RADIO(pi)) {
			wlc_phy_tiny_rfseq_mode_set(pi, 1);
		}

		wlc_phy_txpwrctrl_idle_tssi_meas_acphy(pi);

		if (TINY_RADIO(pi)) {
			wlc_phy_tiny_rfseq_mode_set(pi, 0);
		}
	}

#ifdef ENABLE_FCBS
	/* For shadowed reg/table, initialed values have to be put on both sets.
	 * Here I copy initialed values from setA to setB
	 */
	if (IS_FCBS(pi) && !ACMAJORREV_32(pi->pubpi.phy_rev) && !ACMAJORREV_33(pi->pubpi.phy_rev)) {
		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		ACPHY_DISABLE_STALL(pi);

		wlc_phy_table_read_acphy(pi, iqlocal_tbl_id,
			160, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, iqlocal_tbl_id,
			160, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_PAPR,
			68, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR,
			68, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS0,
			128, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS0,
			128, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RSSICLIPGAIN0,
			19, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RSSICLIPGAIN0,
			19, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS1,
			128, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS1,
			128, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RSSICLIPGAIN1,
			19, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RSSICLIPGAIN1,
			19, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS2,
			128, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS2,
			128, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RSSICLIPGAIN2,
			19, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RSSICLIPGAIN2,
			19, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		ACPHY_ENABLE_STALL(pi, stall_val);
	}
#endif /* ENABLE_FCBS */
	/* SW4345-273 Add TXBF support for 4345 */
#if defined(WL_BEAMFORMING)
	if ((ACMAJORREV_3(pi->pubpi.phy_rev) && ACMINORREV_3(pi)))
		MOD_PHYREG(pi, FrontEndDebug, txbfReportReadEn, 0x01);

	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		txbf_stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		ACPHY_DISABLE_STALL(pi);

		WRITE_PHYREG(pi, BfrMuConfigReg1, 0x1000);
		WRITE_PHYREG(pi, BfmMuConfig3, 0x1000);
		WRITE_PHYREG(pi, BfeMuConfigReg1, 0x1000);
		WRITE_PHYREG(pi, BfrMuConfigReg2, 0x1f92);
		WRITE_PHYREG(pi, BfmMuConfig4, 0x1f92);
		WRITE_PHYREG(pi, BfeMuConfigReg2, 0x1f92);
		MOD_PHYREG(pi, BfrMuConfigReg0, useTxbfIndexAddr, 0);
		MOD_PHYREG(pi, BfeMuConfigReg0, useTxbfIndexAddr, 0);

		MOD_PHYREG(pi, BfmMuConfig0, mlbfEnable, 1);

		/* Enable column-wise phase-alignment */
		if (ACMAJORREV_33(pi->pubpi.phy_rev) ||
			(ACMAJORREV_32(pi->pubpi.phy_rev) && ACMINORREV_2(pi))) {
			MOD_PHYREG(pi, BfmMuConfig0, colPhaseAlignEnable, 1);
		}

		/* Enable BFR spatial expansion */
		if (ACMAJORREV_33(pi->pubpi.phy_rev)) {
			//MOD_PHYREG(pi, BfrMuConfigReg0, bfr_config_spatialExpansion, 1);
			//MOD_PHYREG(pi, BfrMuConfigReg0, bfr_config_phyNumTx,
			//	PHY_BITSCNT(pi->sh->phytxchain) - 1);
		}

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFMUSERINDEX, 8, 0x1000, 32,
			bfmuserdx_lut);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFMUSERINDEX, 64, 0x1040, 32, mlbf_lut);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFMUSERINDEX, 1, 0x1020, 32, mu_vmaddr);
		ACPHY_ENABLE_STALL(pi, txbf_stall_val);
	}
#endif /* WL_BEAMFORMING */

	if (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
		ACMAJORREV_33(pi->pubpi.phy_rev)) {
		MOD_PHYREG(pi, RfseqMode, Trigger_override, 0);
	}
}

uint32
wlc_phy_ac_caps(phy_info_t *pi)
{
	uint32 cap = (PHY_CAP_40MHZ | PHY_CAP_80MHZ | PHY_CAP_SGI | PHY_CAP_LDPC);
	cap |= wlc_phy_ac_proprietary_rates(pi) ? PHY_CAP_VHT_PROP_RATES : 0;
	cap |= wlc_phy_ac_ht_proprietary_rates(pi) ? PHY_CAP_HT_PROP_RATES : 0;
	cap |= wlc_phy_ac_stbc_capable(pi) ? PHY_CAP_STBC : 0;
#ifdef WL11AC_160
	cap |= wlc_phy_ac_160_capable(pi) ? PHY_CAP_160MHZ : 0;
#endif /* WL11AC_160 */
#ifdef WL11AC_80P80
	cap |= wlc_phy_ac_80p80_capable(pi) ? PHY_CAP_8080MHZ : 0;
#endif /* WL11AC_80P80 */
	cap |= (PHY_CAP_SU_BFR | PHY_CAP_SU_BFE);
	cap |= wlc_phy_ac_mu_bfr_capable(pi) ? PHY_CAP_MU_BFR : 0;
	cap |= wlc_phy_ac_mu_bfe_capable(pi) ? PHY_CAP_MU_BFE : 0;
	cap |= wlc_phy_ac_1024qam_capable(pi) ? PHY_CAP_1024QAM : 0;
#ifdef WL11ULB
	cap |= wlc_phy_ac_ulb_10_capable(pi) ? PHY_CAP_10MHZ : 0;
	cap |= wlc_phy_ac_ulb_5_capable(pi) ? PHY_CAP_5MHZ : 0;
	cap |= wlc_phy_ac_ulb_2P5_capable(pi) ? PHY_CAP_2P5MHZ : 0;
#endif /* WL11ULB */
	return cap;
}

void
wlc_phy_table_write_ext_acphy(phy_info_t *pi, const acphytbl_info_t *ptbl_info)
{
	uint8 trig_disable;

	/* Disable any ongoing ACI/FCBS if in progress */
	trig_disable = (ACPHY_ENABLE_FCBS_HWACI(pi)) ? wlc_phy_disable_hwaci_fcbs_trig(pi) : 0;

	phy_utils_write_phytable_ext(pi, ptbl_info, ACPHY_TableID(pi->pubpi.phy_rev),
		ACPHY_TableOffset(pi->pubpi.phy_rev), ACPHY_TableDataWide(pi->pubpi.phy_rev),
		ACPHY_TableDataHi(pi->pubpi.phy_rev), ACPHY_TableDataLo(pi->pubpi.phy_rev));

	/* Restore FCBS Trigger if needed */
	if (ACPHY_ENABLE_FCBS_HWACI(pi)) {
		wlc_phy_restore_hwaci_fcbs_trig(pi, trig_disable);
	} else {
		BCM_REFERENCE(trig_disable);
	}
}

void
wlc_phy_table_write_acphy(phy_info_t *pi, uint32 id, uint32 len, uint32 offset, uint32 width,
                          const void *data)
{
	acphytbl_info_t tbl;
	uint8 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);

	if (stall_val == 0)
		ACPHY_DISABLE_STALL(pi);

	ASSERT(READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls) == 1);

	/*
	 * PHY_TRACE(("wlc_phy_table_write_acphy, id %d, len %d, offset %d, width %d\n",
	 * 	id, len, offset, width));
	*/
	tbl.tbl_id = id;
	tbl.tbl_len = len;
	tbl.tbl_offset = offset;
	tbl.tbl_width = width;
	tbl.tbl_ptr = data;

	wlc_phy_table_write_ext_acphy(pi, &tbl);

	if (stall_val == 0)
		ACPHY_ENABLE_STALL(pi, stall_val);
}

void
wlc_phy_table_read_ext_acphy(phy_info_t *pi, const acphytbl_info_t *ptbl_info)
{
	uint8 trig_disable;

	/* Disable any ongoing ACI/FCBS if in progress */
	trig_disable = (ACPHY_ENABLE_FCBS_HWACI(pi)) ? wlc_phy_disable_hwaci_fcbs_trig(pi) : 0;

	phy_utils_read_phytable_ext(pi, ptbl_info, ACPHY_TableID(pi->pubpi.phy_rev),
		ACPHY_TableOffset(pi->pubpi.phy_rev), ACPHY_TableDataWide(pi->pubpi.phy_rev),
		ACPHY_TableDataHi(pi->pubpi.phy_rev), ACPHY_TableDataLo(pi->pubpi.phy_rev));

	/* Restore FCBS Trigger if needed */
	if (ACPHY_ENABLE_FCBS_HWACI(pi)) {
		wlc_phy_restore_hwaci_fcbs_trig(pi, trig_disable);
	} else {
		BCM_REFERENCE(trig_disable);
	}
}

void
wlc_phy_table_read_acphy(phy_info_t *pi, uint32 id, uint32 len, uint32 offset, uint32 width,
                         void *data)
{
	acphytbl_info_t tbl;
	uint8 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);

	if (stall_val == 0)
		ACPHY_DISABLE_STALL(pi);

	ASSERT(READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls) == 1);

	/*	PHY_TRACE(("wlc_phy_table_read_acphy, id %d, len %d, offset %d, width %d\n",
	 *	id, len, offset, width));
	 */
	tbl.tbl_id = id;
	tbl.tbl_len = len;
	tbl.tbl_offset = offset;
	tbl.tbl_width = width;
	tbl.tbl_ptr = data;

	wlc_phy_table_read_ext_acphy(pi, &tbl);

	if (stall_val == 0)
		ACPHY_ENABLE_STALL(pi, stall_val);
}

/* Use this proc to write tables which need dac clk power up WAR.
 * Current list of tables which need this WAR are
 * epsilon0, epsilon1, bbpdepsiloni0, bbpdepsiloni1, bbpdepsilonq0, bbpdepsilonq1
 * Assuming that the function is called only for the above tables
 */
void wlc_phy_table_write_acphy_dac_war(phy_info_t *pi, uint32 id, uint32 len, uint32 offset,
	uint32 width, void *data, uint8 core)
{
	uint16 orig_dac_clk_pu = 0, orig_ovr_dac_clk_pu = 0;
	bool dacclk_saved = FALSE;

	/* Implement the dac_war for tiny chips.For others simply call table read/write functions */
	if (TINY_RADIO(pi))
		dacclk_saved = wlc_phy_poweron_dac_clocks(pi, core, &orig_dac_clk_pu,
			&orig_ovr_dac_clk_pu);

	/* Now Call table write */
	wlc_phy_table_write_acphy(pi, id, len, offset, width, data);

	/* Restore dac clocks after the table access */
	if (dacclk_saved == TRUE)
		wlc_phy_restore_dac_clocks(pi, core, orig_dac_clk_pu, orig_ovr_dac_clk_pu);
}

/* Use this proc to read tables which need dac clk power up WAR.
 * Current list of tables which need this WAR are
 * epsilon0, epsilon1, bbpdepsiloni0, bbpdepsiloni1, bbpdepsilonq0, bbpdepsilonq1
 * Assuming that the function is called only for the above tables
 */
void wlc_phy_table_read_acphy_dac_war(phy_info_t *pi, uint32 id, uint32 len, uint32 offset,
	uint32 width, void *data, uint8 core)
{
	uint16 orig_dac_clk_pu = 0, orig_ovr_dac_clk_pu = 0;
	bool dacclk_saved = FALSE;

	/* Implement the dac_war for tiny chips.For others simply call table read/write functions */
	if (TINY_RADIO(pi))
		dacclk_saved = wlc_phy_poweron_dac_clocks(pi, core, &orig_dac_clk_pu,
			&orig_ovr_dac_clk_pu);

	/* Now Call table read */
	wlc_phy_table_read_acphy(pi, id, len, offset, width, data);

	/* Restore dac clocks after the table access */
	if (dacclk_saved == TRUE)
		wlc_phy_restore_dac_clocks(pi, core, orig_dac_clk_pu, orig_ovr_dac_clk_pu);
}

/* Use this proc to access tables channel smoothing filter tables in 4349.
 * These tables require the MAC_CLK_EN to be forced through PHYCTL
 */
void wlc_phy_table_write_tiny_chnsmth(phy_info_t *pi, uint32 id, uint32 len, uint32 offset,
	uint32 width, const void *data)
{
	uint16 phyctl_save_val = 0, phyctl_saved = 0;

	if (ACMAJORREV_4(pi->pubpi.phy_rev) && (id == ACPHY_TBL_ID_CORE1CHANSMTH_FLTR)) {
		phyctl_saved = 1;
		wlc_phy_force_mac_clk(pi, &phyctl_save_val);
	}

	/* Now Call table read or write */
	wlc_phy_table_write_acphy(pi, id, len, offset, width, data);

	if (phyctl_saved) {
		W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, phyctl_save_val);
	}
}

/* Proc to force mac_clk in phyctl register */
void wlc_phy_force_mac_clk(phy_info_t *pi, uint16 *orig_phy_ctl)
{
	*orig_phy_ctl = R_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param);
	W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param,
		(*orig_phy_ctl | MAC_PHY_FORCE_CLK));
}

void
wlc_phy_clear_static_table_acphy(phy_info_t *pi, const phytbl_info_t *ptbl_info,
	const uint32 tbl_info_cnt)
{
	uint32 idx, dpth_idx;
	/* Used 2 uint32 zeros because max table width we have is 64 */
	uint32 zeros[2] = { 0, 0};
	for (idx = 0; idx < tbl_info_cnt; idx++) {
		for (dpth_idx = 0; dpth_idx < ptbl_info[idx].tbl_len; dpth_idx++) {
			wlc_phy_table_write_acphy(pi, ptbl_info[idx].tbl_id, 1,
				dpth_idx, ptbl_info[idx].tbl_width, zeros);
		}
	}
}
