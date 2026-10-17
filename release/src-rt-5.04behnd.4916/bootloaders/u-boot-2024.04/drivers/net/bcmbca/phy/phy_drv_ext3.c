// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/*
 *	Created on: Jan 2016
 *		Author: yuval.raviv@broadcom.com
 */

/*
 * Phy driver for external 1G/2.5G/5G/10G phys: BCM8486x, BCM8488x, BCM8489x, BCM5499x
 */

/*
 * PHY firmware versions:
 * MAKO A0			1.0.03
 * ORCA A0			1.1.02
 * ORCA B0			2.3.13
 * BLACKFIN A0		0.2.04
 * BLACKFIN B0		2.3.05
 * SHORTFIN B0		2.3.05
 * LONGFIN A0/B0	2.3.01
 * LANAI A0			1.0.03
 * KAUAI_A0			1.0.04
 * XGPHY16			0.0.14
 * XGPHY8			0.0.03
 * MGPHY8			0.0.03
 * NIIHAU_A0		1.0.01
 */

#include <linux/delay.h>
#include <dm/device_compat.h>
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "xrdp_led_init.h"
#include "dt_access.h"
#ifdef MACSEC_SUPPORT
#include "phy_macsec_api.h"
#endif

typedef struct firmware_s firmware_t;
typedef struct phy_desc_s phy_desc_t;

struct firmware_s {
	char *name;
	u32 *data;
	u32 size;
	int (*load)(firmware_t *firmware);
	u32 map;
};

struct phy_desc_s {
	u16 phyid1;
	u16 phyid2;
	char *name;
	firmware_t *firmware;
	u32 inter_phy_types;
    int flag;
	u16 packageid1;
	u16 packageid2;
#define PHY_DESC_FLAG_VALID_PACKAGEID  (1<<0)
};

#if defined(CONFIG_BCM_PHY_MAKO_A0)
static int load_mako(firmware_t *firmware);
#endif
#if defined(CONFIG_BCM_PHY_ORCA_A0) || defined(CONFIG_BCM_PHY_ORCA_B0)
static int load_orca(firmware_t *firmware);
#endif
#if defined(CONFIG_BCM_PHY_BLACKFIN_A0) || defined(CONFIG_BCM_PHY_BLACKFIN_B0) || \
	defined(CONFIG_BCM_PHY_LONGFIN_A0) || defined(CONFIG_BCM_PHY_LONGFIN_B0) || \
	defined(CONFIG_BCM_PHY_XPHY_16)
static int load_blackfin(firmware_t *firmware);
#endif
#if defined(CONFIG_BCM_PHY_XPHY_8)
static int load_xphy_8(firmware_t *firmware);
#endif
#ifdef CONFIG_BCM_PHY_SHORTFIN_B0
static int load_shortfin(firmware_t *firmware);
#endif
#ifdef CONFIG_BCM_PHY_KAUAI_A0
static int load_kauai(firmware_t *firmware);
#endif
#ifdef CONFIG_BCM_PHY_LANAI_A0
static int load_lanai(firmware_t *firmware);
#endif
#ifdef CONFIG_BCM_PHY_NIIHAU_A0
static int load_niihau(firmware_t *firmware);
#endif

#ifdef CONFIG_BCM_PHY_MAKO_A0
#include "mako_a0_firmware.h"
firmware_t mako_a0 = { "mako_a0", mako_a0_firmware, sizeof(mako_a0_firmware), &load_mako, 0 };
#endif
#ifdef CONFIG_BCM_PHY_ORCA_A0
#include "orca_a0_firmware.h"
firmware_t orca_a0 = { "orca_a0", orca_a0_firmware, sizeof(orca_a0_firmware), &load_orca, 0 };
#endif
#ifdef CONFIG_BCM_PHY_ORCA_B0
#include "orca_b0_firmware.h"
firmware_t orca_b0 = { "blackfin_b0", orca_b0_firmware, sizeof(orca_b0_firmware), &load_orca, 0 };
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_A0
#include "blackfin_a0_firmware.h"
firmware_t blackfin_a0 = { "blackfin_a0", blackfin_a0_firmware, sizeof(blackfin_a0_firmware), &load_blackfin, 0 };
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_B0
#include "blackfin_b0_firmware.h"
firmware_t blackfin_b0 = { "blackfin_b0", blackfin_b0_firmware, sizeof(blackfin_b0_firmware), &load_blackfin, 0 };
#endif
#ifdef CONFIG_BCM_PHY_SHORTFIN_B0
#include "shortfin_b0_firmware.h"
firmware_t shortfin_b0 = { "shortfin_b0", shortfin_b0_firmware, sizeof(shortfin_b0_firmware), &load_shortfin, 0 };
#endif
#ifdef CONFIG_BCM_PHY_KAUAI_A0
#include "kauai_a0_firmware.h"
firmware_t kauai_a0 = { "kauai_a0", kauai_a0_firmware, sizeof(kauai_a0_firmware), &load_kauai, 0 };
#endif
#ifdef CONFIG_BCM_PHY_LANAI_A0
#include "lanai_a0_firmware.h"
firmware_t lanai_a0 = { "lanai_a0", lanai_a0_firmware, sizeof(lanai_a0_firmware), &load_lanai, 0 };
firmware_t lanai_a0_m = { "lanai_a0", lanai_a0_firmware, sizeof(lanai_a0_firmware), &load_lanai, 0 };
#endif
#ifdef CONFIG_BCM_PHY_NIIHAU_A0
#include "niihau_a0_firmware.h"
firmware_t niihau_a0 = { "niihau_a0", niihau_a0_firmware, sizeof(niihau_a0_firmware), &load_niihau, 0 };
firmware_t niihau_a0_m = { "niihau_a0", niihau_a0_firmware, sizeof(niihau_a0_firmware), &load_niihau, 0 };
#endif
#if defined(CONFIG_BCM_PHY_LONGFIN_A0) || defined(CONFIG_BCM_PHY_LONGFIN_B0)
// longfin A0 and B0 use same firmware dated April 2020 or later
#include "longfin_a0_firmware.h"
firmware_t longfin_a0 = { "longfin_a0", longfin_a0_firmware, sizeof(longfin_a0_firmware), &load_blackfin, 0 };
firmware_t longfin_a0_m = { "longfin_a0", longfin_a0_firmware, sizeof(longfin_a0_firmware), &load_blackfin, 0 };
#endif
#ifdef CONFIG_BCM_PHY_XPHY_16
#include "xgphy_16_firmware.h"
firmware_t xgphy_16 = { "xgphy_16", xgphy_16_firmware, sizeof(xgphy_16_firmware), &load_blackfin, 0 };
#endif
#ifdef CONFIG_BCM_PHY_XPHY_8
#include "mgphy_8_firmware.h"
#include "xgphy_8_firmware.h"
firmware_t mgphy_8 = { "mgphy_8", mgphy_8_firmware, sizeof(mgphy_8_firmware), &load_xphy_8, 0 };
firmware_t xgphy_8 = { "xgphy_8", xgphy_8_firmware, sizeof(xgphy_8_firmware), &load_xphy_8, 0 };
#endif

#if defined(CONFIG_BCM_PHY_SHORTFIN_B0) || defined(CONFIG_BCM_PHY_KAUAI_A0)
#define USXGMII_MP_PHY
#endif

static firmware_t *firmware_list[] = {
#ifdef CONFIG_BCM_PHY_MAKO_A0
	&mako_a0,
#endif
#ifdef CONFIG_BCM_PHY_ORCA_A0
	&orca_a0,
#endif
#ifdef CONFIG_BCM_PHY_ORCA_B0
	&orca_b0,
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_A0
	&blackfin_a0,
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_B0
	&blackfin_b0,
#endif
#if defined(CONFIG_BCM_PHY_LONGFIN_A0) || defined(CONFIG_BCM_PHY_LONGFIN_B0)
	&longfin_a0,
	&longfin_a0_m,
#endif
#if defined(CONFIG_BCM_PHY_SHORTFIN_B0)
	&shortfin_b0,
#endif
#if defined(CONFIG_BCM_PHY_KAUAI_A0)
	&kauai_a0,
#endif
#if defined(CONFIG_BCM_PHY_LANAI_A0)
	&lanai_a0,
	&lanai_a0_m,
#endif
#if defined(CONFIG_BCM_PHY_NIIHAU_A0)
	&niihau_a0,
	&niihau_a0_m,
#endif
#ifdef CONFIG_BCM_PHY_XPHY_16
	&xgphy_16,
#endif
#ifdef CONFIG_BCM_PHY_XPHY_8
	&mgphy_8,
	&xgphy_8,
#endif
};

typedef struct loading_reg_s {
	u32 ram_addr;
	u16 devid;
	u16 addr_low;
	u16 addr_high;
	u16 data_low;
	u16 data_high;
	u16 ctrl;
} loading_reg_t;

static loading_reg_t default_load_reg = {
	.ram_addr	= 0x00000000,
	.devid		= 0x01,
	.ctrl		= 0xa817,
	.addr_low	= 0xa819,
	.addr_high	= 0xa81a,
	.data_high	= 0xa81c,
	.data_low	= 0xa81b,
};

static phy_desc_t phy_desc[] = {
#ifdef CONFIG_BCM_PHY_MAKO_A0
	{ 0xae02, 0x5048, "84860    A0", &mako_a0, INTER_PHY_TYPES_S1K2XI5I_M },
	{ 0xae02, 0x5040, "84861    A0", &mako_a0, INTER_PHY_TYPES_S1K2XI5I_M },
#endif
#ifdef CONFIG_BCM_PHY_ORCA_A0
	{ 0xae02, 0x5158, "84880    A0", &orca_a0, INTER_PHY_TYPES_US1K2XI5KI_M },
	{ 0xae02, 0x5150, "84881    A0", &orca_a0, INTER_PHY_TYPES_US1K2XI5KI10R_M },
	{ 0xae02, 0x5148, "84884    A0", &orca_a0, INTER_PHY_TYPES_US1K2XI5KI_M },
	{ 0xae02, 0x5168, "84884E   A0", &orca_a0, INTER_PHY_TYPES_US1K2XI5KI_M },
	{ 0xae02, 0x5178, "84885    A0", &orca_a0, INTER_PHY_TYPES_US1K2XI5KI_M },
	{ 0xae02, 0x5170, "84886    A0", &orca_a0, INTER_PHY_TYPES_US1K2XI5KI10R_M },
	{ 0xae02, 0x5144, "84887    A0", &orca_a0, INTER_PHY_TYPES_US1K2XI5KI10R_M },
	{ 0xae02, 0x5140, "84888    A0", &orca_a0, INTER_PHY_TYPES_US1K2XI5KI10R_M },
	{ 0xae02, 0x5160, "84888E   A0", &orca_a0, INTER_PHY_TYPES_US1K2XI5KI10R_M },
	{ 0xae02, 0x5174, "84888S   A0", &orca_a0, INTER_PHY_TYPES_US1K2XI5KI10R_M },
#endif
#ifdef CONFIG_BCM_PHY_ORCA_B0
	{ 0xae02, 0x5159, "84880    B0", &orca_b0, INTER_PHY_TYPES_US1K2XI5KI_M },
	{ 0xae02, 0x5151, "84881    B0", &orca_b0, INTER_PHY_TYPES_US1K2XI5KI10R_M },
	{ 0xae02, 0x5149, "84884    B0", &orca_b0, INTER_PHY_TYPES_US1K2XI5KI_M },
	{ 0xae02, 0x5169, "84884E   B0", &orca_b0, INTER_PHY_TYPES_US1K2XI5KI_M },
	{ 0xae02, 0x5179, "84885    B0", &orca_b0, INTER_PHY_TYPES_US1K2XI5KI_M },
	{ 0xae02, 0x5171, "84886    B0", &orca_b0, INTER_PHY_TYPES_US1K2XI5KI10R_M },
	{ 0xae02, 0x5145, "84887    B0", &orca_b0, INTER_PHY_TYPES_US1K2XI5KI10R_M },
	{ 0xae02, 0x5141, "84888    B0", &orca_b0, INTER_PHY_TYPES_US1K2XI5KI10R_M },
	{ 0xae02, 0x5161, "84888E   B0", &orca_b0, INTER_PHY_TYPES_US1K2XI5KI10R_M },
	{ 0xae02, 0x5175, "84888S   B0", &orca_b0, INTER_PHY_TYPES_US1K2XI5KI10R_M },
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_A0
	{ 0x3590, 0x5090, "84891    A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x5094, "54991    A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR5KIR_M },
	{ 0x3590, 0x5098, "54991E   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x5080, "84891L   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x5084, "54991L   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR5KIR_M },
	{ 0x3590, 0x5088, "54991EL  A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x50a0, "84892    A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x50a4, "54992    A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR5KIR_M },
	{ 0x3590, 0x50a8, "54992E   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x50b0, "84894    A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x50b4, "54994    A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR5KIR_M },
	{ 0x3590, 0x50b8, "54994E   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x50d0, "54991H   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x50f0, "54994H   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x50c8, "50991EL  A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x50f8, "50994E   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2XIR_M },
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_B0
	{ 0x3590, 0x5091, "84891    B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x5095, "54991    B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR5KIR_M },
	{ 0x3590, 0x5099, "54991E   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x5081, "84891L   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x5085, "54991L   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR5KIR_M },
	{ 0x3590, 0x5089, "54991EL  B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x50c9, "50991EL  B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x50a1, "84892    B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x50a5, "54992    B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR5KIR_M },
	{ 0x3590, 0x50a9, "54992E   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x50b1, "84894    B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x50b5, "54994    B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR5KIR_M },
	{ 0x3590, 0x50b9, "54994E   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x50d1, "54991H   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x50f1, "54994H   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x50f9, "50994E   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2XIR_M },
#endif
#ifdef CONFIG_BCM_PHY_SHORTFIN_B0
	{ 0x3590, 0x5001, "84898    B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
	{ 0x3590, 0x5005, "84896    B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
	{ 0x3590, 0x5009, "54998S   B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
	{ 0x3590, 0x500d, "54998ES  B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
	{ 0x3590, 0x5011, "54998    B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
	{ 0x3590, 0x5015, "54998E   B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
	{ 0x3590, 0x5019, "54994L   B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
	{ 0x3590, 0x501d, "54994EL  B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
#endif
#ifdef CONFIG_BCM_PHY_KAUAI_A0
	{ 0x3590, 0x53c0, "54904EL  A0", &kauai_a0, INTER_PHY_TYPE_USXGMII_MP_M },
    { 0x3590, 0x53c1, "54904EL  A1", &kauai_a0, INTER_PHY_TYPE_USXGMII_MP_M, PHY_DESC_FLAG_VALID_PACKAGEID, 0x002a, 0x0000 },
    { 0x3590, 0x53c1, "50904EL  A1", &kauai_a0, INTER_PHY_TYPE_USXGMII_MP_M, PHY_DESC_FLAG_VALID_PACKAGEID, 0x003a, 0x0000 },
#endif
#ifdef CONFIG_BCM_PHY_LANAI_A0
	{ 0xf7a6, 0x1c14, "50901E   A0", &lanai_a0, INTER_PHY_TYPES_QS1K2XR_M },
    { 0xf7a6, 0x1c10, "54901E   A0", &lanai_a0, INTER_PHY_TYPES_QS1K2XR_M, PHY_DESC_FLAG_VALID_PACKAGEID, 0x0000, 0x0000 },
    { 0xf7a6, 0x1c10, "54901EM  A0", &lanai_a0, INTER_PHY_TYPES_QS1K2XR_M, PHY_DESC_FLAG_VALID_PACKAGEID, 0x0003, 0x0000 },
    { 0xf7a6, 0x1c10, "54901EMX A0", &lanai_a0, INTER_PHY_TYPES_QS1K2XR_M, PHY_DESC_FLAG_VALID_PACKAGEID, 0x0013, 0x0000 },
	{ 0xf7a6, 0x1c18, "54110    A0", &lanai_a0, INTER_PHY_TYPES_QS1K_M },
#endif
#ifdef CONFIG_BCM_PHY_NIIHAU_A0
    { 0xf7a6, 0x1c40, "84991    A0", &niihau_a0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M, PHY_DESC_FLAG_VALID_PACKAGEID, 0x0000, 0x0000 },
    { 0xf7a6, 0x1c40, "84991    A0", &niihau_a0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M, PHY_DESC_FLAG_VALID_PACKAGEID, 0x0008, 0x0000 },
    { 0xf7a6, 0x1c40, "84991M   A0", &niihau_a0, INTER_PHY_TYPES_US1K2XIR5KIR10R_M, PHY_DESC_FLAG_VALID_PACKAGEID, 0x0028, 0x0000 },
#endif
#ifdef CONFIG_BCM_PHY_LONGFIN_A0
	{ 0x3590, 0x5180, "84891LM  A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x5184, "54991LM  A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x5190, "84891M   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x5194, "54991M   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR_M },
	{ 0x3590, 0x5198, "54991EM  A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x5188, "54991ELM A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x51a0, "84892M   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x51a4, "54992M   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR_M },
	{ 0x3590, 0x51a8, "54992EM  A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x51b0, "84894M   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x51b4, "54994M   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR_M },
	{ 0x3590, 0x51b8, "54994EM  A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x51d0, "54991H   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x51f0, "54994H   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
#endif
#ifdef CONFIG_BCM_PHY_LONGFIN_B0
	{ 0x3590, 0x5181, "84891LM  B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x5185, "54991LM  B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x5191, "84891M   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x5195, "54991M   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR_M },
	{ 0x3590, 0x5199, "54991EM  B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x5189, "54991ELM B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x518d, "50991ELM B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x51a1, "84892M   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x51a5, "54992M   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR_M },
	{ 0x3590, 0x51a9, "54992EM  B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x51b1, "84894M   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x51b5, "54994M   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR_M },
	{ 0x3590, 0x51b9, "54994EM  B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR_M },
	{ 0x3590, 0x50c1, "49418      ", &longfin_a0,   INTER_PHY_TYPES_US1K2XIR5KIR_M },	// 4912 integrated XGPHY
	{ 0x3590, 0x51c1, "49418M     ", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR_M },	// 4912 integrated XGPHY
	{ 0x3590, 0x50cd, "4912       ", &longfin_a0,   INTER_PHY_TYPES_US1K2XIR_M },		// 4912 integrated XGPHY
	{ 0x3590, 0x51cd, "4912M      ", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR_M },		// 4912 integrated XGPHY
	{ 0x3590, 0x51d1, "54991H   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x51d5, "54991SK  B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x51f1, "54994H   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
	{ 0x3590, 0x51f5, "54994SK  B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2XIR5KIR10R_M },
#endif
#ifdef CONFIG_BCM_PHY_XPHY_16
	{ 0x3590, 0x50c0, "XPHY6888_X A0", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x50c4, "XPHY6888_5 A0", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x50c8, "XPHY6888_2 A0", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x50cc, "XPHY6888_1 A0", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x5371, "XPHY6888_X B0", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x5375, "XPHY6888_5 B0", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x5379, "XPHY6888_2 B0", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x537d, "XPHY6888_1 B0", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x50e0, "XPHY6813_X   ", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x50e4, "XPHY6813_5   ", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x50e8, "XPHY6813_2   ", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x50ec, "XPHY6813_1   ", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x50e1, "XPHY4916_X   ", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x50e5, "XPHY4916_5   ", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x50e9, "XPHY4916_2   ", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x50ed, "XPHY4916_1   ", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x51e0, "XPHY6765_X   ", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x51e4, "XPHY6765_5   ", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x51e8, "XPHY6765_2   ", &xgphy_16, 0 },							// integrated xgphy_16
	{ 0x3590, 0x51ec, "XPHY6765_1   ", &xgphy_16, 0 },							// integrated xgphy_16
#endif
#ifdef CONFIG_BCM_PHY_XPHY_8
	{ 0xf7a6, 0x1d30, "XGPHY68375_X ", &xgphy_8, 0, 0x0000, 0x0000 },			// integrated xgphy_8
	{ 0xf7a6, 0x1db0, "XGPHY68375_X ", &xgphy_8, 0, 0x0000, 0x0000 },			// integrated xgphy_8
	{ 0xf7a6, 0x1db4, "XGPHY68375_5 ", &xgphy_8, 0, 0x0000, 0x0000 },			// integrated xgphy_8
	{ 0xf7a6, 0x1db8, "XGPHY68375_2 ", &xgphy_8, 0, 0x0000, 0x0000 },			// integrated xgphy_8
	{ 0xf7a6, 0x1dbc, "XGPHY68375_1 ", &xgphy_8, 0, 0x0000, 0x0000 },			// integrated xgphy_8
	{ 0xf7a6, 0x1d38, "MGPHY68375_2 ", &mgphy_8, 0, 0x0000, 0x0000 },			// integrated xgphy_8
	{ 0xf7a6, 0x1d3c, "MGPHY68375_1 ", &mgphy_8, 0, 0x0000, 0x0000 },			// integrated xgphy_8
#endif
};

static u32 enabled_phys;
bus_drv_t *bus_drv;

static inline int _bus_read(u32 addr, u16 dev, u16 reg, u16 *val)
{
	return bus_drv && bus_drv->c45_read(addr, dev, reg, val);
}

static inline int _bus_write(u32 addr, u16 dev, u16 reg, u16 val)
{
	return bus_drv && bus_drv->c45_write(addr, dev, reg, val);
}

#define BUS_READ(a, b, c, d)					if ((ret = _bus_read(a, b, c, d))) goto Exit;
#define BUS_WRITE(a, b, c, d)					if ((ret = _bus_write(a, b, c, d))) goto Exit;
#define BUS_WRITE_ALL(a, b, c, d)				if ((ret = _bus_write_all(a, b, c, d))) goto Exit;
#define BUS_WRITE_AND_VERIFY_ALL(a, b, c, d, e)	if ((ret = _bus_write_and_verify_all(a, b, c, d, e))) goto Exit;

#define PHY_READ(a, b, c, d)					if ((ret = phy_bus_c45_read(a, b, c, d))) goto Exit;
#define PHY_WRITE(a, b, c, d)					if ((ret = phy_bus_c45_write(a, b, c, d))) goto Exit;
#define PHY_COMP_READ(a, b, c, d, ri, ro)		if ((ret = phy_bus_c45_comp_read(a, b, c, d, ri, ro))) goto Exit;
#define PHY_COMP_WRITE(a, b, c, d, ri, ro)		if ((ret = phy_bus_c45_comp_write(a, b, c, d, ri, ro))) goto Exit;

static phy_desc_t *_phy_get_desc_from_phydev(phy_dev_t *phy_dev)
{
    int ret;
    int i;
    phy_desc_t *_phy_desc, *phy_desc_candidate = NULL;

	if (phy_dev->descriptor)
        return phy_dev->descriptor;

	if (phy_dev->phy_id1 == 0 && phy_dev->phy_id2 == 0)
    {
        PHY_READ(phy_dev, 0x01, 0x0002, &phy_dev->phy_id1);
        PHY_READ(phy_dev, 0x01, 0x0003, &phy_dev->phy_id2);
        PHY_READ(phy_dev, 0x01, 0x000e, &phy_dev->pkg_id1);
        PHY_READ(phy_dev, 0x01, 0x000f, &phy_dev->pkg_id2);
    }

	for (i = 0; i < (int)ARRAY_SIZE(phy_desc); i++)
    {
        _phy_desc = &phy_desc[i];
	    if (_phy_desc->phyid1 != phy_dev->phy_id1 || _phy_desc->phyid2 != phy_dev->phy_id2)
            continue;
	    if (!(_phy_desc->flag & PHY_DESC_FLAG_VALID_PACKAGEID) || 
            (_phy_desc->packageid1 == phy_dev->pkg_id1 && _phy_desc->packageid2 == phy_dev->pkg_id2))
        {
            phy_dev->descriptor = _phy_desc;
            break;
        }
        phy_desc_candidate = _phy_desc;
    }

    if (!phy_dev->descriptor && phy_desc_candidate)
    {
        phy_dev->descriptor = phy_desc_candidate;
        phy_dev->flag |= PHY_FLAG_IGNORE_PKGID;
    }

Exit:
	return phy_dev->descriptor;
}

static inline int _phy_use_firmware(phy_dev_t *phy_dev, firmware_t *firmware)
{
    phy_desc_t *phy_desc = _phy_get_desc_from_phydev(phy_dev);

	if (!phy_desc)
		return 0;

	return phy_desc->firmware == firmware;
}

/* Command codes */
#define CMD_GET_PAIR_SWAP                           0x8000
#define CMD_SET_PAIR_SWAP                           0x8001
#define CMD_GET_1588_ENABLE                         0x8004
#define CMD_SET_1588_ENABLE                         0x8005
#define CMD_GET_SHORT_REACH_MODE_ENABLE             0x8006
#define CMD_SET_SHORT_REACH_MODE_ENABLE             0x8007
#define CMD_GET_EEE_MODE                            0x8008
#define CMD_SET_EEE_MODE                            0x8009
#define CMD_GET_EMI_MODE_ENABLE                     0x800A
#define CMD_SET_EMI_MODE_ENABLE                     0x800B
#define CMD_GET_SUB_LF_RF_STATUS                    0x800D
#define CMD_GET_SERDES_KR_MODE_ENABLE               0x800E
#define CMD_SET_SERDES_KR_MODE_ENABLE               0x800F
#define CMD_CLEAR_SUB_LF_RF                         0x8010
#define CMD_SET_SUB_LF_RF                           0x8011
#define CMD_READ_INDIRECT_GPHY_REG_BITS             0x8014
#define CMD_WRITE_INDIRECT_GPHY_REG_BITS            0x8015
#define CMD_GET_XFI_2P5G_5G_MODE                    0x8016
#define CMD_SET_XFI_2P5G_5G_MODE                    0x8017
#define CMD_GET_TWO_PAIR_1G_MODE                    0x8018
#define CMD_SET_TWO_PAIR_1G_MODE                    0x8019
#define CMD_SET_EEE_STATISTICS                      0x801A
#define CMD_GET_EEE_STATISTICS                      0x801B
#define CMD_SET_JUMBO_PACKET                        0x801C
#define CMD_GET_JUMBO_PACKET                        0x801D
#define CMD_GET_MSE                                 0x801E
#define CMD_GET_PAUSE_FRAME_MODE                    0x801F
#define CMD_SET_PAUSE_FRAME_MODE                    0x8020
#define CMD_GET_LED_TYPE                            0x8021
#define CMD_SET_LED_TYPE                            0x8022
#define CMD_GET_MGBASE_T_802_3BZ_TYPE               0x8023
#define CMD_SET_MGBASE_T_802_3BZ_TYPE               0x8024
#define CMD_GET_MSE_GPHY                            0x8025
#define CMD_SET_USXGMII                             0x8026
#define CMD_GET_USXGMII                             0x8027
#define CMD_GET_XL_MODE                             0x8029
#define CMD_SET_XL_MODE                             0x802A
#define CMD_GET_XFI_TX_FILTERS                      0x802B
#define CMD_SET_XFI_TX_FILTERS                      0x802C
#define CMD_GET_XFI_POLARITY                        0x802D
#define CMD_SET_XFI_POLARITY                        0x802E
#define CMD_GET_CURRENT_VOLTAGE                     0x802F
#define CMD_GET_SNR                                 0x8030
#define CMD_GET_CURRENT_TEMP                        0x8031
#define CMD_SET_UPPER_TEMP_WARNING_LEVEL            0x8032
#define CMD_GET_UPPER_TEMP_WARNING_LEVEL            0x8033
#define CMD_SET_LOWER_TEMP_WARNING_LEVEL            0x8034
#define CMD_GET_LOWER_TEMP_WARNING_LEVEL            0x8035
#define CMD_GET_HW_FR_EMI_MODE_ENABLE               0x803A
#define CMD_SET_HW_FR_EMI_MODE_ENABLE               0x803B
#define CMD_GET_CUSTOMER_REQUESTED_TX_PWR_ADJUST2   0x803c
#define CMD_SET_CUSTOMER_REQUESTED_TX_PWR_ADJUST2   0x803d
#define CMD_GET_CUSTOMER_REQUESTED_TX_PWR_ADJUST1   0x8040
#define CMD_SET_CUSTOMER_REQUESTED_TX_PWR_ADJUST1   0x8041
#define CMD_GET_DYNAMIC_PARTITION_SELECT            0x8042
#define CMD_SET_DYNAMIC_PARTITION_SELECT            0x8043
#define CMD_SET_WOL_ENABLE                          0x805A
#define CMD_GET_WOL_ENABLE                          0x805B
#define CMD_SET_MACSEC_ENABLE                       0x805E
#define CMD_GET_MACSEC_ENABLE                       0x805F
#define CMD_RESET_STAT_LOG                          0xC017

/* Command hanlder status codes */
#define CMD_RECEIVED                                0x0001
#define CMD_IN_PROGRESS                             0x0002
#define CMD_COMPLETE_PASS                           0x0004
#define CMD_COMPLETE_ERROR                          0x0008
#define CMD_SYSTEM_BUSY                             0xBBBB

/* Fixups for 5499x phys */
#define ID1_5499X                                   0x35900000
#define ID1_50901E                                  0xf7a60000
#define ID1_84991                                   0xf7a60000
#define ID1_MASK                                    0xffff0000
#define SUPER_I_DEFAULT                             BIT(15)
#define SUPER_I_BLACKFIN                            BIT(8)
#define SUPER_I_LANAI                               BIT(10)
#define SUPER_I_NIIHAU                              BIT(10)
#define CHANGE_STRAP_STATUS                         BIT(1)

#define STEPS                                       10

static int _wait_for_cmd_ready(phy_dev_t *phy_dev)
{
	int ret, i;
	u16 val;

	for (i = 0; i < 1000; i++) {
		/* Read status of command */
		PHY_READ(phy_dev, 0x1e, 0x4037, &val);

		if (val != CMD_IN_PROGRESS && val != CMD_SYSTEM_BUSY)
			return 0;

		udelay(2000);
	}

	printk("Timed out waiting for command ready");

Exit:
	return -1;
}

static u16 _wait_for_cmd_complete(phy_dev_t *phy_dev)
{
	int ret, i;
	u16 val = 0;

	for (i = 0; i < 1000; i++) {
		/* Read status of command */
		PHY_READ(phy_dev, 0x1e, 0x4037, &val);

		if (val == CMD_COMPLETE_PASS || val == CMD_COMPLETE_ERROR)
			goto Exit;

		udelay(2000);
	}

	printk("Timed out waiting for command complete\n");

Exit:
	return val;
}

int cmd_handler(phy_dev_t *phy_dev, u16 cmd_code, u16 *data1, u16 *data2, u16 *data3, u16 *data4, u16 *data5)
{
	int ret = 0;
	u16 cmd_status = 0;

#if defined(CONFIG_BCM_PHY_XPHY_16)
	if (_phy_use_firmware(phy_dev, &xgphy_16))
		ret += 1;
#endif
#if defined(CONFIG_BCM_PHY_XPHY_8)
	if (_phy_use_firmware(phy_dev, &mgphy_8) || _phy_use_firmware(phy_dev, &xgphy_8))
		ret += 1;
#endif

	if (ret) {
		switch (cmd_code) {
		case CMD_GET_XFI_2P5G_5G_MODE:
		case CMD_SET_XFI_2P5G_5G_MODE:
		case CMD_GET_USXGMII:
		case CMD_SET_USXGMII:
		case CMD_GET_XFI_POLARITY:
		case CMD_SET_XFI_POLARITY:
		case CMD_GET_XFI_TX_FILTERS:
		case CMD_SET_XFI_TX_FILTERS:
			return 0;
		default:
			break;
		}
	}

	/* Make sure command interface is open */
	ret = _wait_for_cmd_ready(phy_dev);
	if (ret)
		goto Exit;

	switch (cmd_code) {
	case CMD_SET_PAIR_SWAP:
	case CMD_SET_1588_ENABLE:
	case CMD_SET_SHORT_REACH_MODE_ENABLE:
	case CMD_SET_EEE_MODE:
	case CMD_SET_EMI_MODE_ENABLE:
	case CMD_SET_SERDES_KR_MODE_ENABLE:
	case CMD_CLEAR_SUB_LF_RF:
	case CMD_SET_SUB_LF_RF:
	case CMD_WRITE_INDIRECT_GPHY_REG_BITS:
	case CMD_SET_XFI_2P5G_5G_MODE:
	case CMD_SET_TWO_PAIR_1G_MODE:
	case CMD_SET_PAUSE_FRAME_MODE:
	case CMD_SET_LED_TYPE:
	case CMD_SET_MGBASE_T_802_3BZ_TYPE:
	case CMD_SET_USXGMII:
	case CMD_SET_EEE_STATISTICS:
	case CMD_SET_JUMBO_PACKET:
	case CMD_SET_XL_MODE:
	case CMD_SET_XFI_TX_FILTERS:
	case CMD_SET_XFI_POLARITY:
	case CMD_SET_UPPER_TEMP_WARNING_LEVEL:
	case CMD_SET_LOWER_TEMP_WARNING_LEVEL:
	case CMD_SET_HW_FR_EMI_MODE_ENABLE:
	case CMD_SET_CUSTOMER_REQUESTED_TX_PWR_ADJUST1:
	case CMD_SET_CUSTOMER_REQUESTED_TX_PWR_ADJUST2:
	case CMD_SET_DYNAMIC_PARTITION_SELECT:
	case CMD_SET_WOL_ENABLE:
	case CMD_SET_MACSEC_ENABLE: {
			if (data1)
				PHY_WRITE(phy_dev, 0x1e, 0x4038, *data1);
			if (data2)
				PHY_WRITE(phy_dev, 0x1e, 0x4039, *data2);
			if (data3)
				PHY_WRITE(phy_dev, 0x1e, 0x403a, *data3);
			if (data4)
				PHY_WRITE(phy_dev, 0x1e, 0x403b, *data4);
			if (data5)
				PHY_WRITE(phy_dev, 0x1e, 0x403c, *data5);

			PHY_WRITE(phy_dev, 0x1e, 0x4005, cmd_code);
			cmd_status = _wait_for_cmd_complete(phy_dev);

			break;
		}
	case CMD_GET_PAIR_SWAP:
	case CMD_GET_1588_ENABLE:
	case CMD_GET_SHORT_REACH_MODE_ENABLE:
	case CMD_GET_EEE_MODE:
	case CMD_GET_EMI_MODE_ENABLE:
	case CMD_GET_SERDES_KR_MODE_ENABLE:
	case CMD_GET_SUB_LF_RF_STATUS:
	case CMD_GET_XFI_2P5G_5G_MODE:
	case CMD_GET_TWO_PAIR_1G_MODE:
	case CMD_GET_PAUSE_FRAME_MODE:
	case CMD_GET_LED_TYPE:
	case CMD_GET_MGBASE_T_802_3BZ_TYPE:
	case CMD_GET_MSE_GPHY:
	case CMD_GET_USXGMII:
	case CMD_GET_JUMBO_PACKET:
	case CMD_GET_MSE:
	case CMD_GET_XL_MODE:
	case CMD_GET_XFI_TX_FILTERS:
	case CMD_GET_XFI_POLARITY:
	case CMD_GET_CURRENT_VOLTAGE:
	case CMD_GET_SNR:
	case CMD_GET_CURRENT_TEMP:
	case CMD_GET_UPPER_TEMP_WARNING_LEVEL:
	case CMD_GET_LOWER_TEMP_WARNING_LEVEL:
	case CMD_GET_HW_FR_EMI_MODE_ENABLE:
	case CMD_GET_CUSTOMER_REQUESTED_TX_PWR_ADJUST1:
	case CMD_GET_CUSTOMER_REQUESTED_TX_PWR_ADJUST2:
	case CMD_GET_DYNAMIC_PARTITION_SELECT:
	case CMD_GET_WOL_ENABLE:
	case CMD_GET_MACSEC_ENABLE:
	case CMD_RESET_STAT_LOG: {
			PHY_WRITE(phy_dev, 0x1e, 0x4005, cmd_code);
			cmd_status = _wait_for_cmd_complete(phy_dev);

			if (data1)
				PHY_READ(phy_dev, 0x1e, 0x4038, data1);
			if (data2)
				PHY_READ(phy_dev, 0x1e, 0x4039, data2);
			if (data3)
				PHY_READ(phy_dev, 0x1e, 0x403a, data3);
			if (data4)
				PHY_READ(phy_dev, 0x1e, 0x403b, data4);
			if (data5)
				PHY_READ(phy_dev, 0x1e, 0x403c, data5);

			break;
		}
	case CMD_READ_INDIRECT_GPHY_REG_BITS: {
			if (data1)
				PHY_WRITE(phy_dev, 0x1e, 0x4038, *data1);
			if (data2)
				PHY_WRITE(phy_dev, 0x1e, 0x4039, *data2);
			if (data3)
				PHY_WRITE(phy_dev, 0x1e, 0x403a, *data3);
			if (data4)
				PHY_WRITE(phy_dev, 0x1e, 0x403b, *data4);

			PHY_WRITE(phy_dev, 0x1e, 0x4005, cmd_code);
			cmd_status = _wait_for_cmd_complete(phy_dev);

			if (data5)
				PHY_READ(phy_dev, 0x1e, 0x403c, data5);

			break;
		}
	case CMD_GET_EEE_STATISTICS: {
			if (data1)
				PHY_WRITE(phy_dev, 0x1e, 0x4038, *data1);

			PHY_WRITE(phy_dev, 0x1e, 0x4005, cmd_code);
			cmd_status = _wait_for_cmd_complete(phy_dev);

			if (data2)
				PHY_READ(phy_dev, 0x1e, 0x4039, data2);
			if (data3)
				PHY_READ(phy_dev, 0x1e, 0x403a, data3);
			if (data4)
				PHY_READ(phy_dev, 0x1e, 0x403b, data4);
			if (data5)
				PHY_READ(phy_dev, 0x1e, 0x403c, data5);

			break;
		}
	default:
		printk("Unsupported cmd code: 0x%x\n", cmd_code);
		break;
	}

	if (cmd_status != CMD_COMPLETE_PASS) {
		printk("Failed to execute cmd code: 0x%x\n", cmd_code);
		return 0;
	}

Exit:
	return ret;
}

int _phy_tx_pwr_get(phy_dev_t *phy_dev, int *enable, int *tx_pwr)
{
	int command_handler;
	u16 _enable = 0;
	u16 _tx_pwr = 0;
	int ret;

#ifdef CONFIG_BCM_PHY_MAKO_A0
	if (_phy_use_firmware(phy_dev, &mako_a0))
		command_handler = CMD_GET_CUSTOMER_REQUESTED_TX_PWR_ADJUST1;
	else
#endif
		command_handler = CMD_GET_CUSTOMER_REQUESTED_TX_PWR_ADJUST2;
	ret = cmd_handler(phy_dev, command_handler, &_enable, &_tx_pwr, NULL, NULL, NULL);
	if (ret)
		goto Exit;

	*enable = _enable;
	*tx_pwr = _tx_pwr;
Exit:
	return ret;
}

int _phy_tx_pwr_set(phy_dev_t *phy_dev, int enable, int tx_pwr)
{
	int command_handler;
	u16 _enable = enable;
	u16 _tx_pwr = tx_pwr;
	int ret;

#ifdef CONFIG_BCM_PHY_MAKO_A0
	if (_phy_use_firmware(phy_dev, &mako_a0))
		command_handler = CMD_SET_CUSTOMER_REQUESTED_TX_PWR_ADJUST1;
	else
#endif
		command_handler = CMD_SET_CUSTOMER_REQUESTED_TX_PWR_ADJUST2;

	ret = cmd_handler(phy_dev, command_handler, &_enable, &_tx_pwr, NULL, NULL, NULL);
	if (ret)
		goto Exit;

	phy_dev_an_restart(phy_dev);
Exit:
	return ret;
}

#if defined(CONFIG_BCM_PHY_SHORTFIN_B0) || defined(CONFIG_BCM_PHY_BLACKFIN_A0) || \
	defined(CONFIG_BCM_PHY_BLACKFIN_B0) || defined(CONFIG_BCM_PHY_KAUAI_A0) || \
	defined(CONFIG_BCM_PHY_LANAI_A0) || defined(CONFIG_BCM_PHY_NIIHAU_A0)
static int xfiRegIn[] = {0x1e, 0x4110, 0x2004, 0x1e, 0x4111, 0x2004, 0x1e, 0x4113, 0x2004, -1};
static int xfiRegOut[] = {0x1e, 0x4110, 0x0001, 0x1e, 0x4111, 0x0001, 0x1e, 0x4113, 0x1002, -1};
static void serdes_register_read(phy_dev_t *phy_dev, int dev, int reg, u16 *val)
{
	int ret;

	PHY_COMP_READ(phy_dev, dev, reg, val, xfiRegIn, xfiRegOut);
Exit:
	return;
}

static void serdes_register_write(phy_dev_t *phy_dev, int dev, int reg, u16 val)
{
	int ret;

	PHY_COMP_WRITE(phy_dev, dev, reg, val, xfiRegIn, xfiRegOut);
Exit:
	return;
}

void phy_shortfin_short_amble_workaround(phy_dev_t *phy_dev)
{
	u16 val;
	static u16 reg_save, reg_1c600h_default, reg_1c6e2h_default;
	int rc = 0;

#ifdef CONFIG_BCM_PHY_SHORTFIN_B0
	rc += _phy_use_firmware(phy_dev, &shortfin_b0);
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_A0
	rc += _phy_use_firmware(phy_dev, &blackfin_a0);
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_B0
	rc += _phy_use_firmware(phy_dev, &blackfin_b0);
#endif

	if (!rc)
		return;

	if (!reg_save) {
		serdes_register_read(phy_dev, 0x01, 0xc600, &reg_1c600h_default);
		serdes_register_read(phy_dev, 0x01, 0xc6e2, &reg_1c6e2h_default);
		reg_save = 1;
	}

	if (!phy_dev->link)
		return;

	if (phy_dev->speed == PHY_SPEED_1000) {
		serdes_register_read(phy_dev, 0x01, 0xc600, &val);
		val &= ~0x000f;
		/* (en_1588_rxtx_phy.3) */
		val |= 0x0003;
		/* (sel_int_extclk phy,0) */
		val |= 0x0004;
		serdes_register_write(phy_dev, 0x01, 0xc600, val);

		serdes_register_read(phy_dev, 0x01, 0xc6e2, &val);

		/* (en_preabmle_fix phy ,1) */
		val &= ~0x0cc0;
		val |= ((0x0 & 0x3) << 6);
		val |= ((0x3 & 0x3) << 10);
		serdes_register_write(phy_dev, 0x01, 0xc6e2, val);
	} else {
		serdes_register_write(phy_dev, 0x01, 0xc600, reg_1c600h_default);
		serdes_register_write(phy_dev, 0x01, 0xc6e2, reg_1c6e2h_default);
	}
	return;
}
#else
void phy_shortfin_short_amble_workaround(phy_dev_t *phy_dev)
{
}
#endif

static int _phy_power_get(phy_dev_t *phy_dev, int *enable)
{
	u16 val;
	int ret;

	PHY_READ(phy_dev, 0x1e, 0x401a, &val);

	*enable = (val & (1 << 7)) ? 0 : 1; /* Copper disable bit */

Exit:
	return ret;
}

static int _phy_power_set(phy_dev_t *phy_dev, int enable)
{
	u16 val;
	int ret;

	PHY_READ(phy_dev, 0x1e, 0x401a, &val);

	if (enable)
		val &= ~(1 << 7); /* Copper enable */
	else
		val |= (1 << 7); /* Copper disable */

	PHY_WRITE(phy_dev, 0x1e, 0x401a, val);

Exit:
	return ret;
}

static int get_usxgmii_m_core_id(int phy_addr)
{
	u16 v16;
	int core_id;

	_bus_read(phy_addr, 0x1e, 0x401c, &v16);
	core_id = (v16 >> 5) & 0x0007;
	return core_id;
}

static int get_usxgmii_m_aggregated_port(phy_dev_t *phy_dev)
{
	int core_id = get_usxgmii_m_core_id(phy_dev->addr);

#ifdef CONFIG_BCM_PHY_KAUAI_A0
	if (_phy_use_firmware(phy_dev, &kauai_a0))
		return (phy_dev->addr - core_id + 2);
#endif
	return (phy_dev->addr - core_id);
}

#define PHY_FIRMWARE_WORD(main, branch, build)	((main << 11) | (branch << 4) | build)
#define BLACKFIN_B0_USXGMII_DATA5_HANDLING		PHY_FIRMWARE_WORD(2, 3, 5)	/* 2.3.05 */
#define LONGFIN_B0_USXGMII_DATA5_HANDLING		PHY_FIRMWARE_WORD(2, 3, 1)	/* 2.3.01 */

static int get_firmware_version_digits(int phy_addr, u16 *fw_version, u16 *fw_main, u16 *fw_build, u16 *fw_branch)
{
	int ret = 0;

	BUS_READ(phy_addr, 0x1e, 0x400f, fw_version);

	*fw_branch = (*fw_version >> 0) & 0x7f;
	*fw_main = (*fw_version >> 7) & 0x1f;
	*fw_build = (*fw_version >> 12) & 0xf;
	*fw_version = PHY_FIRMWARE_WORD(*fw_main, *fw_branch, *fw_build);
Exit:
	return ret;
}

static void phy_usxgmii_data5_setting(phy_dev_t *phy_dev, u16 *data5)
{
	u16 fw_version, fw_build, fw_main, fw_branch;

#ifdef CONFIG_BCM_PHY_BLACKFIN_B0
	if (_phy_use_firmware(phy_dev, &blackfin_b0)) {
		get_firmware_version_digits(phy_dev->addr, &fw_version, &fw_main, &fw_build, &fw_branch);
		if (fw_version >= BLACKFIN_B0_USXGMII_DATA5_HANDLING)
			goto set_data5;
	}
#endif

#if defined(CONFIG_BCM_PHY_LONGFIN_A0) || defined(CONFIG_BCM_PHY_LONGFIN_B0)
	if (_phy_use_firmware(phy_dev, &longfin_a0) || _phy_use_firmware(phy_dev, &longfin_a0_m)) {
		get_firmware_version_digits(phy_dev->addr, &fw_version, &fw_main, &fw_build, &fw_branch);
		if (fw_version >= LONGFIN_B0_USXGMII_DATA5_HANDLING)
			goto set_data5;
	}
#endif
	*data5 = 0;
	return;

set_data5:
	if (phy_dev->shared_ref_clk_mhz) /* Shared clock */ {
		*data5 = 0;
	} else {
		*data5 = 1;
	}

	return;
}

#define XFI_MODE_IDLE_STUFFING		0 /* Idle Stuffing mode over XFI interface */
#define XFI_MODE_BASE_X				1 /* 2.5GBase-X or 5GBase-X */
#define XFI_MODE_BASE_R				2 /* 2.5GBase-R or 5GBase-R */

static int _phy_inter_phy_types_set(phy_dev_t *phy_dev, int inter_phy_types)
{
	int ret;
	u16 data1, data2, data3, data4, data5;

	data1 = XFI_MODE_BASE_X ; /* XFI mode in 2.5G speed */
	data2 = XFI_MODE_BASE_X ; /* XFI mode in 5G speed */

	if (inter_phy_types & INTER_PHY_TYPE_2P5GBASE_R_M)
		data1 = XFI_MODE_BASE_R;

	if (inter_phy_types & INTER_PHY_TYPE_2P5GIDLE_M)
		data1 = XFI_MODE_IDLE_STUFFING;

	if (inter_phy_types & INTER_PHY_TYPE_5GBASE_R_M)
		data2 = XFI_MODE_BASE_R;

	if (inter_phy_types & INTER_PHY_TYPE_5GIDLE_M)
		data2 = XFI_MODE_IDLE_STUFFING;

#if defined(CONFIG_BCM_PHY_LANAI_A0)
	/* Disable SGMII_AN for LANAI family */
	if (_phy_use_firmware(phy_dev, &lanai_a0))
		data2 = 0;
#endif

#if defined(CONFIG_BCM_PHY_NIIHAU_A0)
	/* Disable SGMII_AN for NIIHAU family */
	if (_phy_use_firmware(phy_dev, &niihau_a0))
		data2 = 0;
#endif

	/* Configure XFI modes for 2.5G and 5G */
	ret = cmd_handler(phy_dev, CMD_SET_XFI_2P5G_5G_MODE, &data1, &data2, NULL, NULL, NULL);
	if (ret)
		goto Exit;

	/* Configure USXGMII mode */
	if (inter_phy_types & (INTER_PHY_TYPE_USXGMII_M | INTER_PHY_TYPE_USXGMII_MP_M)) {
		data1 = 1; /* USXGMII Enable */
		data2 = 0; /* AN disabled */
#ifdef CONFIG_BCM_PHY_KAUAI_A0
		if (_phy_use_firmware(phy_dev, &kauai_a0))
			data3 = 0x100;
		else
#endif
			data3 = 0x124; /* baud rate 10G, quad speed 2.5G */
		data4 = 0; /* 0 = Broadcom mode */
		phy_usxgmii_data5_setting(phy_dev, &data5);

		ret = cmd_handler(phy_dev, CMD_SET_USXGMII, &data1, &data2, &data3, &data4, &data5);
		if (ret)
			goto Exit;
	}

Exit:
	return ret;
}

static int _phy_led_control_mode_set(phy_dev_t *phy_dev, int user_control)
{
	int ret = 0;
	/* u16 val = user_control ? 1 : 0; */

	/* Set led control to user or firmware */
	/*if ((ret = cmd_handler(phy_dev, CMD_SET_LED_TYPE, &val, NULL, NULL, NULL, NULL)))
	  goto Exit;

Exit:*/
	return ret;
}

static int _phy_mdix_mode_set(phy_dev_t *phy_dev, phy_mdix_t mdix_mode)
{
	int ret;
	u16 val;
	int auto_mdix, force_mdix;

	if (mdix_mode == PHY_MDIX_AUTO) {
		auto_mdix = 1;
		force_mdix = 0;
	} else {
		auto_mdix = 0;
		force_mdix = mdix_mode == PHY_MDIX_ON ? 1 : 0;
	}

	PHY_READ(phy_dev, 0x07, 0x9000, &val);

	if (auto_mdix)
		val &= ~(1 << 14); /* Enable Auto MDIX */
	else
		val |= (1 << 14); /* Disable Auto MDIX */

	PHY_WRITE(phy_dev, 0x07, 0x9000, val);

	PHY_READ(phy_dev, 0x07, 0x900e, &val);

	if (force_mdix)
		val |= (1 << 7); /* Enable MDIX Swap */
	else
		val &= ~(1 << 7); /* Disable MDIX Swap */

	PHY_WRITE(phy_dev, 0x07, 0x900e, val);

Exit:
	return ret;
}

static int _phy_mdix_mode_get(phy_dev_t *phy_dev, phy_mdix_t *mdix_mode)
{
	int ret;
	u16 val1, val2;

	*mdix_mode = PHY_MDIX_UNKNOWN;

	PHY_READ(phy_dev, 0x07, 0x9000, &val1);
	PHY_READ(phy_dev, 0x07, 0x900e, &val2);

	if (val1 & (1 << 14))
		*mdix_mode = val2 & (1 << 7) ? PHY_MDIX_ON : PHY_MDIX_OFF; /* Force MDIX */
	else
		*mdix_mode = PHY_MDIX_AUTO; /* Auto MDIX Enabled */

Exit:
	return ret;
}

static int _phy_mdix_status_get(phy_dev_t *phy_dev, phy_mdix_t *mdix_mode)
{
	int ret;
	u16 val;

	PHY_READ(phy_dev, 0x07, 0x9001, &val);

	*mdix_mode = val & (1 << 13) ? PHY_MDIX_ON : PHY_MDIX_OFF; /* MDIX Enabled */

Exit:
	return ret;
}

static int _phy_force_auto_mdix_set(phy_dev_t *phy_dev, int enable)
{
	int ret;
	u16 val;

	PHY_READ(phy_dev, 0x07, 0x902f, &val);

	if (enable)
		val |= (1 << 9); /* Auto-MDIX enabled */
	else
		val &= ~(1 << 9); /* Auto-MDIX disabled */

	PHY_WRITE(phy_dev, 0x07, 0x902f, val);

Exit:
	return ret;
}

#if 0
static int _phy_force_auto_mdix_get(phy_dev_t *phy_dev, int *enable)
{
	int ret;
	u16 val;

	PHY_READ(phy_dev, 0x07, 0x902f, &val);

	*enable = val & (1 << 9) ? 1 : 0; /* Force Auto MDIX Enabled */

Exit:
	return ret;
}
#endif

static int _phy_eth_wirespeed_set(phy_dev_t *phy_dev, int enable)
{
	int ret;
	u16 val;

	PHY_READ(phy_dev, 0x07, 0x902f, &val);

	if (enable)
		val |= (1 << 4); /* Ethernet@Wirespeed enabled */
	else
		val &= ~(1 << 4); /* Ethernet@Wirespeed disabled */

	PHY_WRITE(phy_dev, 0x07, 0x902f, val);

Exit:
	return ret;
}

static int _phy_eth_wirespeed_get(phy_dev_t *phy_dev, int *enable)
{
	int ret;
	u16 val;

	PHY_READ(phy_dev, 0x07, 0x902f, &val);

	*enable = val & (1 << 4) ? 1 : 0; /* Ethernet@Wirespeed Enabled */

Exit:
	return ret;
}

static int _phy_pair_swap_set(phy_dev_t *phy_dev, int enable)
{
	int ret;
	u16 data;

	if (enable)
		data = 0x1b;
	else
		data = 0xe4;

	ret = cmd_handler(phy_dev, CMD_SET_PAIR_SWAP, NULL, &data, NULL, NULL, NULL);
	if (ret)
		goto Exit;

Exit:
	return ret;
}

static int _phy_xfi_txrx_polarity_set(phy_dev_t *phy_dev, int inverse, int tx_dir)
{
	int ret;
	u16 data1, data2, data3, data4, data5;
	phy_dev_t _phy_dev = *phy_dev;
	int is_lanai_class = 0;

#if defined(CONFIG_BCM_PHY_LANAI_A0)
	if (_phy_use_firmware(phy_dev, &lanai_a0))
		is_lanai_class = 1;
#endif

#if defined(CONFIG_BCM_PHY_NIIHAU_A0)
	if (_phy_use_firmware(phy_dev, &niihau_a0))
		is_lanai_class = 1;
#endif

	if (phy_dev_is_mphy(phy_dev))
		_phy_dev.addr = get_usxgmii_m_aggregated_port(phy_dev);

	if (is_lanai_class)
		phy_dev_super_isolate_phy(phy_dev, 1);

	ret = cmd_handler(&_phy_dev, CMD_GET_XFI_POLARITY, &data1, &data2, &data3, &data4, &data5);
	if (ret)
		goto Exit;

	if (is_lanai_class) {
		if (tx_dir)
			data1 = inverse > 0;
		else
			data2 = inverse > 0;
	} else {
		if (tx_dir)
			data2 = inverse > 0;
		else
			data3 = inverse > 0;

		data1 = data4 = data5 = 0;
	}

	ret = cmd_handler(&_phy_dev, CMD_SET_XFI_POLARITY, &data1, &data2, &data3, &data4, &data5);
	if (ret)
		goto Exit;

Exit:
	if (is_lanai_class)
		phy_dev_super_isolate_phy(phy_dev, 0);

	return ret;
}

static int _phy_xfi_tx_polarity_set(phy_dev_t *phy_dev, int inverse)
{
	return _phy_xfi_txrx_polarity_set(phy_dev, inverse, 1);
}

static int _phy_xfi_rx_polarity_set(phy_dev_t *phy_dev, int inverse)
{
	return _phy_xfi_txrx_polarity_set(phy_dev, inverse, 0);
}

static int _phy_jumbo_packet_set(phy_dev_t *phy_dev, int enable)
{
	int ret = 0;
#if defined(CONFIG_BCM_JUMBO_FRAME)
	u16 data1 = enable ? 1 : 0;

	ret = cmd_handler(phy_dev, CMD_SET_JUMBO_PACKET, &data1, NULL, NULL, NULL, NULL);
	if (ret)
		goto Exit;

Exit:
#endif
	return ret;
}

static int _phy_apd_get(phy_dev_t *phy_dev, int *enable)
{
	int ret;
	u16 val;

	PHY_READ(phy_dev, 0x07, 0x901a, &val);
	*enable = val & (1 << 5) ? 1 : 0; /* Auto power-down mode enabled */

Exit:
	return ret;
}

static int _phy_apd_set(phy_dev_t *phy_dev, int enable)
{
	int ret;
	u16 val;

	/* Auto-Power Down */
	PHY_READ(phy_dev, 0x07, 0x901a, &val);

	if (enable) {
		val |= (1 << 5); /* Auto power-down mode enabled */
		val |= (1 << 8); /* Enable energy detect single link pulse */
	} else {
		val &= ~(1 << 5); /* Auto power-down mode disabled */
		val &= ~(1 << 8); /* Disable energy detect single link pulse */
	}

	PHY_WRITE(phy_dev, 0x07, 0x901a, val);

	/* Reserved Control 3 */
	PHY_READ(phy_dev, 0x07, 0x9015, &val);

Exit:
	return ret;
}

#define EEE_MODE_DISABLED				0 /* EEE Disabled*/
#define EEE_MODE_NATIVE_EEE				1 /* Native EEE */
#define EEE_MODE_AUTOGREEEN_FIXED		2 /* AutoGrEEEn Fixed Latency */
#define EEE_MODE_AUTOGREEEN_VARIABLE	3 /* AutoGrEEEn Variable Latency */

/* Note: AutoGrEEEn mode is not supported when idle stuffing is enabled on 2.5/5G rates */

static int _phy_eee_modes_get(phy_dev_t *phy_dev, u16 *modes)
{
	int ret;
	u16 data1, data2, data3, data4;

	/* Get EEE modes */
	ret = cmd_handler(phy_dev, CMD_GET_EEE_MODE, &data1, &data2, &data3, &data4, NULL);
	if (ret)
		goto Exit;

	*modes = data1;

Exit:
	return ret;
}

static int _phy_eee_get(phy_dev_t *phy_dev, int *enable)
{
	int ret;
	u16 val;

	/* local EEE Status */
	ret = _phy_eee_modes_get(phy_dev, &val);
	if (ret)
		goto Exit;

	*enable = !!val;

Exit:
	return ret;
}

static int _phy_eee_resolution_get(phy_dev_t *phy_dev, int *enable)
{
	int ret;
	u16 val;

	/* EEE Resolution Status */
	PHY_READ(phy_dev, 7, 0x803e, &val);

	/* Check if the link partner auto-negotiated EEE capability */
	*enable = val & 0x3e ? 1 : 0;

Exit:
	return ret;
}

static int _phy_eee_autogreeen_read(phy_dev_t *phy_dev, int *autogreeen)
{
	int ret;
	u16 mode, modes;

	/* Read configured EEE modes */
	ret = _phy_eee_modes_get(phy_dev, &modes);
	if (ret)
		goto Exit;

	switch (phy_dev->speed) {
	case PHY_SPEED_10000:
		mode = (modes >> 0) & 0x3;
		break;
	case PHY_SPEED_5000:
		mode = (modes >> 6) & 0x3;
		break;
	case PHY_SPEED_2500:
		mode = (modes >> 4) & 0x3;
		break;
	case PHY_SPEED_1000:
	case PHY_SPEED_100:
		mode = (modes >> 2) & 0x3;
		break;
	default:
		mode = 0;
		break;
	}

	*autogreeen = mode >= EEE_MODE_AUTOGREEEN_FIXED ? 1 : 0;

Exit:
	return ret;
}

static int _phy_eee_mode_set(phy_dev_t *phy_dev, u32 caps)
{
	int ret;
	u16 modes = 0, data1, data2, data3, data4;
	u8 mode = phy_dev->autogreeen ? EEE_MODE_AUTOGREEEN_FIXED : EEE_MODE_NATIVE_EEE;

#ifdef CONFIG_BCM_PHY_XPHY_8
	if (_phy_use_firmware(phy_dev, &mgphy_8) || _phy_use_firmware(phy_dev, &xgphy_8))
		return 0;
#endif

	/* 54904EL_A0 chip has issue on 2.5G link when EEE is on, will be fixed in 54904EL_A1 chip */
	{
#define PHY_ID_54904EL_A0 0x359053c0
		u32 phy_id;

		phy_dev_phyid_get(phy_dev, &phy_id);
		if (phy_id == PHY_ID_54904EL_A0) {
			printk("ERROR: EEE is not supported in 54904EL_A0 at this firmware version yet.\n");
			return 0;
		}
	}

	data1 = XFI_MODE_BASE_X;
	data2 = XFI_MODE_BASE_X;

	ret = cmd_handler(phy_dev, CMD_GET_XFI_2P5G_5G_MODE, &data1, &data2, NULL, NULL, NULL);
	if (ret)
		goto Exit;

	/* 10G			bits 0:1 */
	/* 10M/100M/1G	bits 2:3 */
	/* 2.5G			bits 4:5 */
	/* 5G			bits 6:7 */
	modes |= ((caps & PHY_CAP_10_HALF) || (caps & PHY_CAP_10_FULL)) ? (mode << 2) : 0;
	modes |= ((caps & PHY_CAP_100_HALF) || (caps & PHY_CAP_100_FULL)) ? (mode << 2) : 0;
	modes |= ((caps & PHY_CAP_1000_HALF) || (caps & PHY_CAP_1000_FULL)) ? (mode << 2) : 0;
	modes |= ((caps & PHY_CAP_2500) ? ((data1 == XFI_MODE_IDLE_STUFFING ? EEE_MODE_NATIVE_EEE : mode) << 4) : 0);
	modes |= ((caps & PHY_CAP_5000) ? ((data2 == XFI_MODE_IDLE_STUFFING ? EEE_MODE_NATIVE_EEE : mode) << 6) : 0);
	modes |= ((caps & PHY_CAP_10000)) ? (mode << 0) : 0;

	data1 = modes;		/* Bitmap of EEE modes per speed */
	data2 = 0x0000;		/* AutoGrEEEn High Threshold */
	data3 = 0x7a12;		/* AutoGrEEEn Low Threshold */
	data4 = 0x0480;		/* AutoGrEEEn Latency */

	/* Set EEE mode */
	ret = cmd_handler(phy_dev, CMD_SET_EEE_MODE, &data1, &data2, &data3, &data4, NULL);
	if (ret)
		goto Exit;

	PHY_WRITE(phy_dev, 0x01, 0xa88c, data2);	/* AutoGrEEEn High Threshold */
	PHY_WRITE(phy_dev, 0x01, 0xa88d, data3);	/* AutoGrEEEn Low Threshold */
	PHY_WRITE(phy_dev, 0x01, 0xa88e, data4);	/* AutoGrEEEn Latency */

	phy_dev_super_isolate_phy(phy_dev, 1);		/* Requested in datasheet, or EEE will fail in power on link up */
	udelay(2000);
	phy_dev_super_isolate_phy(phy_dev, 0);

Exit:
	return ret;
}

static int _phy_an_restart(phy_dev_t *phy_dev)
{
	int ret;
	u16 val;

	PHY_READ(phy_dev, 0x07, 0xffe0, &val);
	val |= (1 << 12) | (1 << 9);
	PHY_WRITE(phy_dev, 0x07, 0xffe0, val);

Exit:
	return ret;
}

static int _phy_eee_set(phy_dev_t *phy_dev, int enable)
{
	int ret;
	u32 caps;

	ret = phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps);
	if (ret)
		goto Exit;

	ret = _phy_eee_mode_set(phy_dev, enable ? caps : 0);
	if (ret)
		goto Exit;

	/* Restart auto negotiation to kick off EEE settings */
	ret = _phy_an_restart(phy_dev);

Exit:
	return ret;
}

static int _phy_read_status(phy_dev_t *phy_dev)
{
	int ret = 0, mode;
	u16 val;
	int org_link = phy_dev->link;

	/* Read the status register */
	PHY_READ(phy_dev, 0x1e, 0x400d, &val);

	/* Copper link status */
	phy_dev->link = ((val >> 5) & 0x1);

	if (!phy_dev->link)
		goto Exit;

	/* Copper speed */
	mode = ((val >> 2) & 0x7);

	if (mode == 7)
		phy_dev->speed = PHY_SPEED_10;
	else if (mode == 2)
		phy_dev->speed = PHY_SPEED_100;
	else if (mode == 4)
		phy_dev->speed = PHY_SPEED_1000;
	else if (mode == 1)
		phy_dev->speed = PHY_SPEED_2500;
	else if (mode == 3)
		phy_dev->speed = PHY_SPEED_5000;
	else if (mode == 6)
		phy_dev->speed = PHY_SPEED_10000;
	else
		goto Exit;

	/* Read the 1000BASE-T auxiliary status summary register */
	PHY_READ(phy_dev, 0x07, 0xfff9, &val);

	if (phy_dev->speed > PHY_SPEED_1000) {
		phy_dev->duplex = PHY_DUPLEX_FULL;
	} else {
		mode = ((val >> 8) & 0x7);

		if (mode == 1 || mode == 3 || mode == 6)
			phy_dev->duplex = PHY_DUPLEX_HALF;
		else if (mode == 2 || mode == 5 || mode == 7)
			phy_dev->duplex = PHY_DUPLEX_FULL;
	}

	phy_dev->pause_rx = ((val >> 1) & 0x1);
	phy_dev->pause_tx = ((val >> 0) & 0x1);

	if (!org_link && phy_dev->link)
		phy_shortfin_short_amble_workaround(phy_dev);

	return ret;

Exit:
	phy_dev->link = 0;
	phy_dev->speed = PHY_SPEED_UNKNOWN;
	phy_dev->duplex = PHY_DUPLEX_UNKNOWN;
	phy_dev->pause_rx = 0;
	phy_dev->pause_tx = 0;
	return ret;
}

static int _phy_support_10m(phy_dev_t *phy_dev)
{
	int ret = 0;

#if defined(DSL_DEVICES)	/* DSL uses run time checking */
	return 1;
#endif

#ifdef CONFIG_BCM_PHY_KAUAI_A0
	if (_phy_use_firmware(phy_dev, &kauai_a0))
		ret = 1;
#endif

#ifdef CONFIG_BCM_PHY_LANAI_A0
	if (_phy_use_firmware(phy_dev, &lanai_a0))
		ret = 1;
#endif

	return ret;
}

int _phy_caps_get(phy_dev_t *phy_dev, int caps_type, u32 *pcaps)
{
	int ret = 0;
	u16 val = 0;
	u32 caps = 0;

	if (caps_type == CAPS_TYPE_SUPPORTED) {
		caps |= PHY_CAP_AUTONEG | PHY_CAP_PAUSE | PHY_CAP_PAUSE_ASYM | PHY_CAP_REPEATER;
		caps |= PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000;

		/* MII status */
		PHY_READ(phy_dev, 0x07, 0xffe1, &val);

		if (_phy_support_10m(phy_dev)) {
			if (val & (1 << 11))
				caps |= PHY_CAP_10_HALF;

			if (val & (1 << 12))
				caps |= PHY_CAP_10_FULL;
		}

		if (val & (1 << 13))
			caps |= PHY_CAP_100_HALF;

		if (val & (1 << 14))
			caps |= PHY_CAP_100_FULL;

		/* MII extended status */
		PHY_READ(phy_dev, 0x07, 0xffef, &val);

		if (val & (1 << 12))
			caps |= PHY_CAP_1000_HALF;

		if (val & (1 << 13))
			caps |= PHY_CAP_1000_FULL;

		*pcaps = caps;

		return 0;
	}

	if (caps_type == CAPS_TYPE_LP_ADVERTISED) {
		u16 lpa = 0, stat1000 = 0, an_stat = 0;

		PHY_READ(phy_dev, 0x07, 0xffea, &stat1000);
		PHY_READ(phy_dev, 0x07, 0xffe5, &lpa);
		PHY_READ(phy_dev, 0x07, 0x0021, &an_stat);

		if (!(lpa & (1 << 14)))
			goto Exit;

		caps |= PHY_CAP_AUTONEG;

		if (lpa & (1 << 10))
			caps |= PHY_CAP_PAUSE;

		if (lpa & (1 << 11))
			caps |= PHY_CAP_PAUSE_ASYM;

		if (lpa & (1 << 5))
			caps |= PHY_CAP_10_HALF;

		if (lpa & (1 << 6))
			caps |= PHY_CAP_10_FULL;

		if (lpa & (1 << 7))
			caps |= PHY_CAP_100_HALF;

		if (lpa & (1 << 8))
			caps |= PHY_CAP_100_FULL;

		if (stat1000 & (1 << 10))
			caps |= PHY_CAP_1000_HALF;

		if (stat1000 & (1 << 11))
			caps |= PHY_CAP_1000_FULL;

		if (an_stat & (1 << 5))
			caps |= PHY_CAP_2500;

		if (an_stat & (1 << 6))
			caps |= PHY_CAP_5000;

		if (an_stat & (1 << 11))
			caps |= PHY_CAP_10000;
	}

	if (caps_type == CAPS_TYPE_ADVERTISE) {
		/* 1000Base-T/100Base-TX MII control */
		PHY_READ(phy_dev, 0x07, 0xffe0, &val);

		if (val & (1 << 12))
			caps |= PHY_CAP_AUTONEG;

		/* Copper auto-negotiation advertisement */
		PHY_READ(phy_dev, 0x07, 0xffe4, &val);

		if (val & (1 << 10))
			caps |= PHY_CAP_PAUSE;

		if (val & (1 << 11))
			caps |= PHY_CAP_PAUSE_ASYM;

		if (_phy_support_10m(phy_dev)) {
			if (val & (1 << 5))
				caps |= PHY_CAP_10_HALF;

			if (val & (1 << 6))
				caps |= PHY_CAP_10_FULL;
		}

		if (val & (1 << 7))
			caps |= PHY_CAP_100_HALF;

		if (val & (1 << 8))
			caps |= PHY_CAP_100_FULL;

		/* 1000Base-T control */
		PHY_READ(phy_dev, 0x07, 0xffe9, &val);

		if (val & (1 << 8))
			caps |= PHY_CAP_1000_HALF;

		if (val & (1 << 9))
			caps |= PHY_CAP_1000_FULL;

		if (val & (1 << 10))
			caps |= PHY_CAP_REPEATER;

		/* 10GBase-T AN control */
		PHY_READ(phy_dev, 0x07, 0x0020, &val);

		if (val & (1 << 7))
			caps |= PHY_CAP_2500;

		if (val & (1 << 8))
			caps |= PHY_CAP_5000;

		if (val & (1 << 12))
			caps |= PHY_CAP_10000;
	}

	*pcaps = caps;

Exit:
	return ret;
}

int _phy_caps_set(phy_dev_t *phy_dev, u32 caps)
{
	int ret;
	u16 val, modes;

	if (caps & (PHY_CAP_1000_HALF | PHY_CAP_1000_FULL | PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000))
		caps |= PHY_CAP_AUTONEG;

	/* Copper auto-negotiation advertisement */
	PHY_READ(phy_dev, 0x07, 0xffe4, &val);

	val &= ~((1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 10) | (1 << 11));

	if (_phy_support_10m(phy_dev)) {
		if (caps & PHY_CAP_10_HALF)
			val |= (1 << 5);

		if (caps & PHY_CAP_10_FULL)
			val |= (1 << 6);
	}

	if (caps & PHY_CAP_100_HALF)
		val |= (1 << 7);

	if (caps & PHY_CAP_100_FULL)
		val |= (1 << 8);

	if (caps & PHY_CAP_PAUSE)
		val |= (1 << 10);

	if (caps & PHY_CAP_PAUSE_ASYM)
		val |= (1 << 11);

	PHY_WRITE(phy_dev, 0x07, 0xffe4, val);

	/* 1000Base-T control */
	PHY_READ(phy_dev, 0x07, 0xffe9, &val);

	val &= ~((1 << 8) | (1 << 9) | (1 << 10));

	if (caps & PHY_CAP_1000_HALF)
		val |= (1 << 8);

	if (caps & PHY_CAP_1000_FULL)
		val |= (1 << 9);

	if (caps & PHY_CAP_REPEATER)
		val |= (1 << 10);

	PHY_WRITE(phy_dev, 0x07, 0xffe9, val);

	/* 10GBase-T AN control */
	PHY_READ(phy_dev, 0x07, 0x0020, &val);

	val &= ~((1 << 7) | (1 << 8) | (1 << 12) | (1 << 13));

	if (caps & PHY_CAP_2500)
		val |= (1 << 7);

	if (caps & PHY_CAP_5000)
		val |= (1 << 8);

	if (caps & PHY_CAP_10000)
		val |= (1 << 12);

	if (caps & PHY_CAP_REPEATER)
		val |= (1 << 13);

	PHY_WRITE(phy_dev, 0x07, 0x0020, val);

	/* 1000Base-T/100Base-TX MII control */
	PHY_READ(phy_dev, 0x07, 0xffe0, &val);

	val &= ~((1 << 6) | (1 << 8) | (1 << 12) | (1 << 13));

	if (caps & (PHY_CAP_1000_HALF | PHY_CAP_1000_FULL))
		val |= (1 << 6);
	else if (caps & (PHY_CAP_100_HALF | PHY_CAP_100_FULL))
		val |= (1 << 13);

	if (caps & (PHY_CAP_10_FULL | PHY_CAP_100_FULL | PHY_CAP_1000_FULL))
		val |= (1 << 8);

	PHY_WRITE(phy_dev, 0x07, 0xffe0, val);

	if (!(caps & PHY_CAP_AUTONEG))
		goto Exit;

	/* Check if EEE mode is configured */
	ret = _phy_eee_modes_get(phy_dev, &modes);
	if (ret)
		goto Exit;

	/* Reset the EEE mode according to the phy capabilities, if it was set before */
	ret = _phy_eee_mode_set(phy_dev, caps);
	if (modes && ret)
		goto Exit;

	/* Restart auto negotiation */
	ret = _phy_an_restart(phy_dev);

Exit:
	return ret;
}

static int _phy_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
	int ret;
	u32 caps;

	if (speed == PHY_SPEED_UNKNOWN) {
		speed = PHY_SPEED_10000;
		duplex = PHY_DUPLEX_FULL;
	}

	ret = phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps);
	if (ret)
		goto Exit;

	caps &= ~(PHY_CAP_10_HALF | PHY_CAP_10_FULL |
		PHY_CAP_100_HALF | PHY_CAP_100_FULL |
		PHY_CAP_1000_HALF | PHY_CAP_1000_FULL |
		PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000);

	switch (speed) {
	case PHY_SPEED_10000:
		caps |= PHY_CAP_10000;
		__attribute__ ((__fallthrough__));
	case PHY_SPEED_5000:
		caps |= PHY_CAP_5000;
		__attribute__ ((__fallthrough__));
	case PHY_SPEED_2500:
		caps |= PHY_CAP_2500;
		__attribute__ ((__fallthrough__));
	case PHY_SPEED_1000:
		caps |= PHY_CAP_1000_HALF | ((duplex == PHY_DUPLEX_FULL) ? PHY_CAP_1000_FULL : 0);
		__attribute__ ((__fallthrough__));
	case PHY_SPEED_100:
		caps |= PHY_CAP_100_HALF | ((duplex == PHY_DUPLEX_FULL) ? PHY_CAP_100_FULL : 0);
		__attribute__ ((__fallthrough__));
	case PHY_SPEED_10:
		caps |= PHY_CAP_10_HALF | ((duplex == PHY_DUPLEX_FULL) ? PHY_CAP_10_FULL : 0);
		break;
	default:
		printk("Ignoring unsupported speed\n");
		goto Exit;
		break;
	}

	ret = phy_dev_caps_set(phy_dev, caps);
	if (ret)
		goto Exit;

Exit:
	return ret;
}

static int _phy_phyid_get(phy_dev_t *phy_dev, u32 *phyid)
{
	int ret;
	u16 phyid1, phyid2;

	PHY_READ(phy_dev, 0x01, 0x0002, &phyid1);
	PHY_READ(phy_dev, 0x01, 0x0003, &phyid2);

	*phyid = phyid1 << 16 | phyid2;

Exit:
	return ret;
}

static int _phy_super_isolate_5499x(phy_dev_t *phy_dev, int isolate)
{
	int ret;
	u16 data;

	/* Read the status register */
	PHY_READ(phy_dev, 0x1e, 0x401c, &data);

	if (isolate)
		data |= SUPER_I_BLACKFIN;
	else
		data &= ~SUPER_I_BLACKFIN;

	PHY_WRITE(phy_dev, 0x1e, 0x401c, data);

	return 0;
Exit:
	return ret;
}

static int _phy_super_isolate_lanai(phy_dev_t *phy_dev, int isolate)
{
	int ret;
	u16 data;

	/* Read the status register */
	PHY_READ(phy_dev, 0x1e, 0x401c, &data);

	if (isolate)
		data |= SUPER_I_LANAI;
	else
		data &= ~SUPER_I_LANAI;

	PHY_WRITE(phy_dev, 0x1e, 0x401c, data);

	return 0;
Exit:
	return ret;
}

static int _phy_super_isolate_niihau(phy_dev_t *phy_dev, int isolate)
{
	int ret;
	u16 data;

	/* Read the status register */
	PHY_READ(phy_dev, 0x1e, 0x401c, &data);

	if (isolate)
		data |= SUPER_I_NIIHAU;
	else
		data &= ~SUPER_I_NIIHAU;

	PHY_WRITE(phy_dev, 0x1e, 0x401c, data);

	return 0;
Exit:
	return ret;
}

static int _phy_super_isolate_default(phy_dev_t *phy_dev, int isolate)
{
	int ret;
	u16 data;

	/* Read the status register */
	PHY_READ(phy_dev, 0x1e, 0x401a, &data);

	if (isolate)
		data |= SUPER_I_DEFAULT;
	else
		data &= ~SUPER_I_DEFAULT;

	PHY_WRITE(phy_dev, 0x1e, 0x401a, data);

	return 0;
Exit:
	return ret;
}

static int _phy_isolate(phy_dev_t *phy_dev, int isolate)
{
	u16 val;
	int ret;

	PHY_READ(phy_dev, 0x7, 0xffe0, &val);
	if (isolate) {
		val |= BMCR_ISOLATE;
	} else {
		val &= ~BMCR_ISOLATE;
	}
	PHY_WRITE(phy_dev, 0x7, 0xffe0, val);

Exit:
	return ret;
}

static int _phy_super_isolate(phy_dev_t *phy_dev, int isolate)
{
	int ret, i = 1000;
	u16 data;
	u32 phyid;

	_phy_phyid_get(phy_dev, &phyid);

	if ((phyid & ID1_MASK) == ID1_5499X)
		ret = _phy_super_isolate_5499x(phy_dev, isolate);
	else if ((phyid & ID1_MASK) == ID1_50901E)
		ret = _phy_super_isolate_lanai(phy_dev, isolate);
	else if ((phyid & ID1_MASK) == ID1_84991)
		ret = _phy_super_isolate_niihau(phy_dev, isolate);
	else
		ret = _phy_super_isolate_default(phy_dev, isolate);

	if (ret)
		goto Exit;

	do {
		udelay(1000);
		PHY_READ(phy_dev, 0x1e, 0x400e, &data);
	} while (i-- && (data & CHANGE_STRAP_STATUS));

	if (data & CHANGE_STRAP_STATUS)
		printk("ERROR: PHY address: %d, hardware failed to clear SUPER_I boot strap status 0x%x\n",
			phy_dev->addr, data);

	return 0;
Exit:
	return ret;
}

static int _phy_set_mode(phy_dev_t *phy_dev, int line_side)
{
	int ret;
	u16 val1, val2, val3;

	val1 = line_side ? 0x0001 : 0x2004;
	val2 = line_side ? 0x0001 : 0x2004;
	val3 = line_side ? 0x1002 : 0x2004;

	/* Set base-pointer mode */
	PHY_WRITE(phy_dev, 0x1e, 0x4110, val1);
	PHY_WRITE(phy_dev, 0x1e, 0x4111, val2);
	PHY_WRITE(phy_dev, 0x1e, 0x4113, val3);

Exit:
	return ret;
}

#if defined(RESCAL_BY_XPHY)
static void __iomem *xphy_rescal_reg;

#define RESCAL_RESISTOR_VALUE_S		(4)
#define RESCAL_RESISTOR_VALUE_M		(0xf << RESCAL_RESISTOR_VALUE_S)
#define RESCAL_OVERRIDE				BIT(3)
static int xphy_rescal_probe(dt_device_t *pdev)
{
	int ret;

	xphy_rescal_reg = dt_dev_remap(pdev, 0);
	if (IS_ERR_OR_NULL(xphy_rescal_reg)) {
		ret = PTR_ERR(xphy_rescal_reg);
		xphy_rescal_reg = NULL;
		dev_err(pdev, "Missing exphy_rescal entry\n");
		goto Exit;
	}

	dev_dbg(pdev, "xphy_rescal_reg =0x%p\n", xphy_rescal_reg);
	dev_info(pdev, "registered\n");

	return 0;

Exit:
	return ret;
}

static const struct udevice_id xphy_rescal_id[] = {
	{ .compatible = "brcm,xphy-rescal" },
	{ /* end of list */ },
};

U_BOOT_DRIVER(xphy_rescal_drv) = {
	.name = "xphy-rescal",
	.id = UCLASS_MISC,
	.of_match = xphy_rescal_id,
	.probe = xphy_rescal_probe,
};

static int GetRCalSetting_1UM_VERT(int phy_addr)
{
	u16 val;
	int i;

	for (i = 20; i > 0; i--) {
		_bus_read(phy_addr, 0x1e, 0x8103, &val);
		if ((val & (1 << 5)))
			break;
		udelay(1000);
	}

	if ((val & (1 << 5)))
		return val & 0xf;
	else
		return 7;
}

static void xphy_rescal(int phy_addr)
{
	static int init;
	volatile u32 *reg = (volatile u32 *)xphy_rescal_reg;
	u32 val, reg_val;

	if (init)
		return;

	if (!xphy_rescal_reg) {
		printk("WARNING: xphy_rescal_reg is not set!!\n");
		return;
	}

	init = 1;
	val = GetRCalSetting_1UM_VERT(phy_addr);
	printk("Setting Serdes Calibration value to 0x%x from XPHY at address %d\n", val, phy_addr);

	reg_val = *reg;
	reg_val &= ~RESCAL_RESISTOR_VALUE_M;
	reg_val |= (val << RESCAL_RESISTOR_VALUE_S) | RESCAL_OVERRIDE;

	*reg = reg_val;
}
#endif

#if defined(DSL_DEVICES)
#ifdef CONFIG_BCM_PHY_XPHY_16
static int dsl_xphy_extra_init(phy_dev_t *phy_dev)
{
	int ret;

	printk("Initialize XPHY at %d LED registers.\n", phy_dev->addr);
	PHY_WRITE(phy_dev, 0x1, 0xa838, 0x0080);
	PHY_WRITE(phy_dev, 0x1, 0xa839, 0x0000);
	PHY_WRITE(phy_dev, 0x1, 0xa8f3, 0x0000);

	PHY_WRITE(phy_dev, 0x1, 0xa835, 0x0000);
	PHY_WRITE(phy_dev, 0x1, 0xa836, 0x0000);
	PHY_WRITE(phy_dev, 0x1, 0xa8f2, 0x0008);

	PHY_WRITE(phy_dev, 0x1, 0xa832, 0x0000);
	PHY_WRITE(phy_dev, 0x1, 0xa833, 0x0000);
	PHY_WRITE(phy_dev, 0x1, 0xa8f1, 0x0004);

	PHY_WRITE(phy_dev, 0x1, 0xa82f, 0x0000);
	PHY_WRITE(phy_dev, 0x1, 0xa830, 0x0000);
	PHY_WRITE(phy_dev, 0x1, 0xa8f0, 0x0010);

	PHY_WRITE(phy_dev, 0x1, 0xa82c, 0x0000);
	PHY_WRITE(phy_dev, 0x1, 0xa82d, 0x0000);
	PHY_WRITE(phy_dev, 0x1, 0xa8ef, 0x0020);

	PHY_WRITE(phy_dev, 0x1, 0xa83b, 0x2492);
	phy_dev->flag |= PHY_FLAG_FORCE_2P5G_XGMII;
Exit:
	return ret;
}
#endif

extern int dsl_runner_ext3_phy_init(phy_dev_t *phy_dev);
static int _phy_init_dsl(phy_dev_t *phy_dev)
{
	int ret = dsl_runner_ext3_phy_init(phy_dev);

	phy_dev->autogreeen = 1;	/* By default, we use AutoGreeen mode for EEE */

	if (ret)
		goto Exit;

	/* Enable jumbo packet support */
	ret = _phy_jumbo_packet_set(phy_dev, 1);
	if (ret)
		goto Exit;

#ifdef CONFIG_BCM_PHY_XPHY_16
	if (_phy_use_firmware(phy_dev, &xgphy_16))
		dsl_xphy_extra_init(phy_dev);
#endif

	if (phy_dev->tx_power != -1)
		phy_dev_tx_pwr_set(phy_dev, 1, phy_dev->tx_power);

#ifdef MACSEC_SUPPORT
#if defined(CONFIG_BCM_PHY_LONGFIN_A0) || defined(CONFIG_BCM_PHY_LONGFIN_B0)
	if (_phy_use_firmware(phy_dev, &longfin_a0_m)) {
		printk("phy 0x%x is macsec capable, initializing macsec module\n", phy_dev->addr);
		ret = phy_macsec_pu_init(phy_dev);
	}
#endif
#endif
	phy_dev_cable_diag_init(phy_dev);

Exit:
	return ret;
}

static int _phy_init_pon(phy_dev_t *phy_dev);
static int _phy_init(phy_dev_t *phy_dev)
{
	if ((phy_dev->flag & PHY_FLAG_ON_MEZZANINE) && (phy_dev->flag & PHY_FLAG_NOT_PRESENTED))
		return 0;

	if (phy_is_pon_wan_ae_serdes(phy_dev->cascade_prev))
		return _phy_init_pon(phy_dev);
	else
		return _phy_init_dsl(phy_dev);
}
#endif

#if defined(DSL_DEVICES)
static int _phy_init_pon(phy_dev_t *phy_dev)
#else
static int _phy_init(phy_dev_t *phy_dev)
#endif
{
	int ret, i;
	u16 val;

	phy_dev->autogreeen = 1;	/* By default, we use AutoGreeen mode for EEE */

	/* Reset the phy */
	PHY_WRITE(phy_dev, 0x01, 0x0000, 0x8000);

	/* Verify that the processor is running */
	i = 1000;
	do {
		udelay(2000);
		PHY_READ(phy_dev, 0x01, 0x0000, &val);
		ret = (val != 0x2040);
	} while (ret && i--);

	if (ret)
		goto Exit;

	/* Set base-pointer mode to line side to access PMD/PCS/AN for BASE-T */
	ret = _phy_set_mode(phy_dev, 1);
	if (ret)
		goto Exit;

	/* Set SerDes mode for 2.5G/5G speeds */
	ret = _phy_inter_phy_types_set(phy_dev, phy_dev->inter_phy_types);
	if (ret)
		goto Exit;

	/* Enable Force Auto-MDIX mode */
	ret = _phy_force_auto_mdix_set(phy_dev, 1);
	if (ret)
		goto Exit;

	/* Enable Ethernet@Wirespeed mode */
	ret = _phy_eth_wirespeed_set(phy_dev, 0);
	if (ret)
		goto Exit;

	/* Set pair-swap mode */
	ret = _phy_pair_swap_set(phy_dev, phy_dev->swap_pair);
	if (ret)
		goto Exit;

	/* Set XFI TX polarity */
	ret = _phy_xfi_tx_polarity_set(phy_dev, phy_dev->xfi_tx_polarity_inverse);
	if (ret)
		goto Exit;

	/* Set XFI RX polarity */
	ret = _phy_xfi_rx_polarity_set(phy_dev, phy_dev->xfi_rx_polarity_inverse);
	if (ret)
		goto Exit;

	/* Enable jumbo packet support */
	ret = _phy_jumbo_packet_set(phy_dev, 1);
	if (ret)
		goto Exit;

	ret = phy_dev_caps_set(phy_dev, PHY_CAP_PAUSE | PHY_CAP_REPEATER);
	if (ret)
		goto Exit;

	ret = _phy_speed_set(phy_dev, PHY_SPEED_10000, PHY_DUPLEX_FULL);
	if (ret)
		goto Exit;

	/* Set LED control type after AN is enabled */
	ret = _phy_led_control_mode_set(phy_dev, 0);
	if (ret)
		goto Exit;

#ifdef MACSEC_SUPPORT
#if defined(CONFIG_BCM_PHY_LONGFIN_A0) || defined(CONFIG_BCM_PHY_LONGFIN_B0)
	if (_phy_use_firmware(phy_dev, &longfin_a0_m)) {
		printk("phy 0x%x is macsec capable, initializing macsec module\n", phy_dev->addr);
		ret = phy_macsec_pu_init(phy_dev);
	}
#endif
#endif

	phy_dev_cable_diag_init(phy_dev);

Exit:
	return ret;
}

u32 _bus_read_all(u32 phy_map, u16 dev, u16 reg, u16 val, u32 mask)
{
	u32 i, ret = 0;
	u16 _val;

	for (i = 0; i < 32; i++) {
		if (!(phy_map & (1 << i)))
			continue;

		if ((_bus_read(i, dev, reg, &_val)))
			continue;

		if ((_val & mask) == val)
			ret |= (1 << i);
	}

	return ret;
}

int _bus_write_all(u32 phy_map, u16 dev, u16 reg, u16 val)
{
	int ret;
	u32 i;

	for (i = 0; i < 32; i++) {
		if (!(phy_map & (1 << i)))
			continue;

		ret = _bus_write(i, dev, reg, val);
		if (ret)
			return ret;
	}

	return 0;
}

int _bus_write_and_verify_all(u32 phy_map, u16 dev, u16 reg, u16 val, u32 mask)
{
	int ret;
	u32 i, j;
	u16 _val;

	for (i = 0; i < 32; i++) {
		if (!(phy_map & (1 << i)))
			continue;

		j = 1000;
		do {
			udelay(1000);
			ret = _bus_write(i, dev, reg, val);
			if (ret)
				return ret;

			ret = _bus_read(i, dev, reg, &_val);
			if (ret)
				return ret;
		} while (((_val & 0xffff) != val) && j--);

		if (j == 0) {
			printk("MDIO Bus Write and Verify is failed\n");
			return -1;
		}
	}

	return 0;
}

static int phy_count(u32 phy_map)
{
	int cnt;

	for (cnt = 0; phy_map != 0; phy_map &= (phy_map - 1))
		cnt++;

	return cnt;
}

static u32 get_base_phy_addr(u32 phy_map)
{
	int flag = 1;
	int cnt = 0;

	while ((flag & phy_map) == 0) {
		flag <<= 1;
		cnt++;
	}

	return cnt;
}

static inline u32 bcast_phy_map(u32 phy_map)
{
	return (phy_map & ~(1 << get_base_phy_addr(phy_map)));
}

#if defined(CONFIG_BCM_PHY_SHORTFIN_B0) || defined(CONFIG_BCM_PHY_MAKO_A0) || \
	defined(CONFIG_BCM_PHY_ORCA_A0) || defined(CONFIG_BCM_PHY_ORCA_B0) || \
	defined(CONFIG_BCM_PHY_BLACKFIN_A0) || defined(CONFIG_BCM_PHY_BLACKFIN_B0) || \
	defined(CONFIG_BCM_PHY_LONGFIN_A0) || defined(CONFIG_BCM_PHY_LONGFIN_B0) || \
	defined(CONFIG_BCM_PHY_XPHY_16) || defined(CONFIG_BCM_PHY_KAUAI_A0) || \
	defined(CONFIG_BCM_PHY_LANAI_A0) || defined(CONFIG_BCM_PHY_NIIHAU_A0)
static int verify_firmware_file(int phy_addr, firmware_t *firmware, loading_reg_t *load_reg)
{
	u32 data, data_ram;
	int errors = 0;
	u32 *firmware_data, firmware_size;
	int i, step, dword_size, ram_addr;
	u16 data_hi, data_lo;

	if (!load_reg)
		load_reg = &default_load_reg;

	ram_addr = load_reg->ram_addr;
	firmware_data = firmware->data;
	firmware_size = firmware->size;
	dword_size = firmware_size / sizeof(u32);
	step = dword_size / STEPS;

	printk("Verifying the firmware loaded in the PHY at %d, size=%d bytes, ram_addr=0x%08x\n",
		phy_addr, firmware_size, ram_addr);

	_bus_write(phy_addr, load_reg->devid, load_reg->ctrl, 0x0000);

	_bus_write(phy_addr, load_reg->devid, load_reg->addr_low, 0 & 0xffff);
	_bus_write(phy_addr, load_reg->devid, load_reg->addr_high, 0 >> 16);
	_bus_write(phy_addr, load_reg->devid, load_reg->ctrl, 0x002a);

	_bus_read(phy_addr, load_reg->devid, load_reg->data_high, &data_hi);
	_bus_read(phy_addr, load_reg->devid, load_reg->data_low, &data_lo);
	_bus_write(phy_addr, load_reg->devid, load_reg->ctrl, 0x0038);

	for (i = 0; i < dword_size; i++) {
		data = firmware_data[i];

		_bus_read(phy_addr, load_reg->devid, load_reg->data_high, &data_hi);
		_bus_read(phy_addr, load_reg->devid, load_reg->data_low, &data_lo);

		data_ram = (data_hi << 16) | data_lo;

		if (data != data_ram && errors++ < 32) {
			printk("mismatching: ram_addr: *0x%08x = 0x%08x, firmware offset: *0x%08x = 0x%08x\n",
				ram_addr + i * 4, data_ram, i * 4, data);
		}

		if (i == i / step * step)
			printk("\r%d%%", i * STEPS / step);
	}
	printk("\n");
	_bus_write(phy_addr, load_reg->devid, load_reg->ctrl, 0x0000);

	if (errors)
		printk("Comparison failed with %d dword mismatching\n", errors);
	else
		printk("Comparison succeeded\n");

	return errors;
}

static inline void bus_fast_mode(int enabled)
{
	bus_cfg_t bus_cfg;

	bus_cfg_get(bus_drv, &bus_cfg);
	bus_cfg.fast_mode = enabled;
	bus_cfg_set(bus_drv, &bus_cfg);
}

static int _load_firmware_file(int master_addr, firmware_t *firmware, loading_reg_t *load_reg)
{
	int ret = 0;
	u32 *firmware_data, firmware_size;
	int dword_size, step;
	int i;

	if (!load_reg)
		load_reg = &default_load_reg;

	printk("Registers: devid:%02x, ctrl:%02x, ram_addr:%08x\n",
		load_reg->devid, load_reg->ctrl, load_reg->ram_addr);
	printk("           addr_low:%04x, addr_high:%04x, data_low:%04x, data_high:%04x",
		load_reg->addr_low, load_reg->addr_high, load_reg->data_low, load_reg->data_high);

	firmware_data = firmware->data;
	firmware_size = firmware->size;
	dword_size = firmware_size / sizeof(u32);
	step = dword_size / STEPS;

	BUS_WRITE(master_addr, load_reg->devid, load_reg->addr_low, load_reg->ram_addr & 0xffff);
	BUS_WRITE(master_addr, load_reg->devid, load_reg->addr_high, (load_reg->ram_addr >> 16));
	BUS_WRITE(master_addr, load_reg->devid, load_reg->ctrl, 0x0038);

	bus_fast_mode(1);
	for (i = 0; i < dword_size; i++) {
		BUS_WRITE(master_addr, load_reg->devid, load_reg->data_high, firmware_data[i] >> 16); /* upper 16 bits */
		BUS_WRITE(master_addr, load_reg->devid, load_reg->data_low, firmware_data[i] & 0xffff); /* lower 16 bits */

		if (i == i / step * step)
			printk("\r%d%%", i * STEPS / step);
	}
	bus_fast_mode(0);
	printk("\n");
	BUS_WRITE(master_addr, load_reg->devid, load_reg->ctrl, 0x0000);
Exit:
	return ret;
}

static unsigned long _jiffies;
#define jiffies (_jiffies++)
#define msecs_to_jiffies(j) ((j) * 10)

static int load_firmware_file(int phy_addr, firmware_t *firmware, loading_reg_t *load_reg)
{
#ifdef DEBUG_FW_LOADING
	u32 start_time, end_time;

	start_time = jiffies;
#endif
	if (!load_reg)
		load_reg = &default_load_reg;

	printk("Registers: devid:%02x, ctrl:%02x, ram_addr:%08x\n",
		load_reg->devid, load_reg->ctrl, load_reg->ram_addr);
	printk("           addr_low:%04x, addr_high:%04x, data_low:%04x, data_high:%04x",
		load_reg->addr_low, load_reg->addr_high, load_reg->data_low, load_reg->data_high);

	if (_load_firmware_file(phy_addr, firmware, load_reg))
		return -1;

#ifdef DEBUG_FW_LOADING
	end_time = jiffies;

	printk("Firmwre loading time: %dms\n", end_time - start_time);

	start_time = jiffies;

	if (verify_firmware_file(phy_addr, firmware, load_reg))
		return -1;

	end_time = jiffies;

	printk("Firmwre verifying time: %dms\n", end_time - start_time);
#endif

	return 0;
}
#endif

static char *get_firmware_version(int phy_addr)
{
	u16 fw_version, fw_build, fw_main, fw_branch;
	static char buf[256];

	buf[0] = 0;
	if (get_firmware_version_digits(phy_addr, &fw_version, &fw_main, &fw_build, &fw_branch) ||
		fw_version == 0)
		goto Exit;
	sprintf(buf, "(%d.%d.%d)", fw_main, fw_branch, fw_build);
Exit:
	return buf;
}

int print_firmware_version(int phy_addr)
{
	printk("Firmware loading completed successfully, version: %s\n\n", get_firmware_version(phy_addr));
	return 0;
}

#if defined(CONFIG_BCM_PHY_MAKO_A0)
static int load_mako(firmware_t *firmware)
{
	int i, ret, base_phy_addr, phy_cnt;
	u32 phy_map, b_phy_map;

	phy_map = firmware->map;
	base_phy_addr = get_base_phy_addr(phy_map); /* select the min PHY address for broadcast operation */
	b_phy_map = bcast_phy_map(phy_map); /* phy map for broadcast configuration - exclude the min PHY address */
	phy_cnt = phy_count(phy_map);

	/* 1. If more than one PHY, turn on broadcast mode to accept write operations for addr = base_phy_addr */
	if (phy_cnt > 1) {
		printk("Turn on broadcast mode to accept write operations\n");
		BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4107, 0x0401 | ((base_phy_addr & 0x1f) << 5));
		BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4117, 0xf001);
	}

	/* 2. Halt the Mako processors operation */
	printk("Halt the PHYs processors operation\n");

	BUS_WRITE_ALL(phy_map, 0x1e, 0x4188, 0x48f0);
	BUS_WRITE_ALL(phy_map, 0x1e, 0x4186, 0x8000);
	BUS_WRITE_ALL(phy_map, 0x1e, 0x4181, 0x017c);
	BUS_WRITE_ALL(phy_map, 0x1e, 0x4181, 0x0040);

	BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0000);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xc300);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x0010);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0x0000);
	BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

	BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0000);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x1018);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0xe59f);
	BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

	BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0004);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x1f11);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0xee09);
	BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

	BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0008);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x0000);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0xe3a0);
	BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

	BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x000c);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x1806);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0xe3a0);
	BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

	BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0010);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x0002);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0xe8a0);
	BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

	BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0014);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x0001);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0xe150);
	BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

	BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0018);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0xfffc);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0x3aff);
	BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

	BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x001c);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0xfffe);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0xeaff);
	BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

	BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0020);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x0021);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0x0004);
	BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

	BUS_WRITE_ALL(phy_map, 0x1e, 0x4181, 0x0000);

	/* 3. Upload the firmware into the on-chip memory of the devices */
	ret = load_firmware_file(base_phy_addr, firmware, 0);
	if (ret)
		goto Exit;

	BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0000);
	BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0000);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xc300);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x0000);
	BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0x0000);
	BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

	/* 4. Reset the processors to start execution of the code in the on-chip memory */
	printk("Reset the processors to start execution of the code in the on-chip memory\n");

	BUS_WRITE(base_phy_addr, 0x01, 0xa008, 0x0000);
	BUS_WRITE_ALL(phy_map, 0x1e, 0x8004, 0x5555);
	BUS_WRITE(base_phy_addr, 0x01, 0x0000, 0x8000);

	/* 5. Verify that the processors are running */
	pr_cont("Verify that the processors are running: ");

	i = 1000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map, 0x01, 0x0000, 0x2040, 0xffff);
	} while ((ret != phy_map) && i--);

	pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");

	ret = ret != phy_map;
	if (ret)
		goto Exit;

	/* 6. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
	pr_cont("Verify that the firmware has been loaded with good CRC: ");

	i = 1000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map, 0x1e, 0x400d, 0x4000, 0xc000);
	} while ((ret != phy_map) && i--);

	pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");

	ret = ret != phy_map;
	if (ret)
		goto Exit;

	print_firmware_version(base_phy_addr);

Exit:
	if (ret)
		verify_firmware_file(base_phy_addr, firmware, 0);
	return ret;
}
#endif

#if defined(CONFIG_BCM_PHY_ORCA_A0) || defined(CONFIG_BCM_PHY_ORCA_B0)
static int load_orca(firmware_t *firmware)
{
	int i, ret, base_phy_addr, phy_cnt;
	u32 phy_map, b_phy_map;

	phy_map = firmware->map;
	base_phy_addr = get_base_phy_addr(phy_map); /* select the min PHY address for broadcast operation */
	b_phy_map = bcast_phy_map(phy_map); /* phy map for broadcast configuration - exclude the min PHY address */
	phy_cnt = phy_count(phy_map);

	/* 1. Halt the Orca processors operation */
	printk("Halt the PHYs processors operation\n");

	BUS_WRITE_ALL(phy_map, 0x1e, 0x418c, 0x0000);
	BUS_WRITE_ALL(phy_map, 0x1e, 0x4188, 0x48f0);
	BUS_WRITE_ALL(phy_map, 0x1e, 0x4181, 0x017c);
	BUS_WRITE_ALL(phy_map, 0x1e, 0x4186, 0x8000);

	BUS_WRITE_AND_VERIFY_ALL(phy_map, 0x1e, 0x4181, 0x0040, 0xffff);

	BUS_WRITE_ALL(phy_map, 0x1e, 0x4186, 0x8000);

	BUS_WRITE_ALL(phy_map, 0x01, 0xa819, 0x0000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa81a, 0xc300);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa81b, 0x0001);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa81c, 0x0000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa817, 0x0009);

	BUS_WRITE_ALL(phy_map, 0x1e, 0x4181, 0x0000);

	/* 2. If more than one PHY, turn on broadcast mode to accept write operations for addr = base_phy_addr */
	if (phy_cnt > 1) {
		printk("Turn on broadcast mode to accept write operations\n");
		BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4107, 0x0401 | ((base_phy_addr & 0x1f) << 5));
		BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4117, 0xf001);
	}

	/* 3. Upload the firmware into the on-chip memory of the devices */
	ret = load_firmware_file(base_phy_addr, firmware, 0);
	if (ret)
		goto Exit;

	/* 4. Reset the processors to start execution of the code in the on-chip memory */
	printk("Reset the processors to start execution of the code in the on-chip memory\n");

	BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0000);
	BUS_WRITE(base_phy_addr, 0x01, 0xa008, 0x0000);
	BUS_WRITE_ALL(phy_map, 0x1e, 0x8004, 0x5555);
	BUS_WRITE(base_phy_addr, 0x1, 0x8003, 0x0000);
	BUS_WRITE(base_phy_addr, 0x01, 0x0000, 0x8000);

	/* 5. Verify that the processors are running */
	pr_cont("Verify that the processors are running: ");

	i = 1000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map, 0x01, 0x0000, 0x2040, 0xffff);
	} while ((ret != phy_map) && i--);

	pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");

	ret = ret != phy_map;
	if (ret)
		goto Exit;

	/* 6. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
	pr_cont("Verify that the firmware has been loaded with good CRC: ");

	i = 1000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map, 0x1e, 0x400d, 0x4000, 0xc000);
	} while ((ret != phy_map) && i--);

	pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");

	ret = ret != phy_map;
	if (ret)
		goto Exit;

	print_firmware_version(base_phy_addr);

Exit:
	if (ret)
		verify_firmware_file(base_phy_addr, firmware, 0);
	return ret;
}
#endif

#if defined(CONFIG_BCM_PHY_SHORTFIN_B0) || defined(CONFIG_BCM_PHY_KAUAI_A0) || defined(CONFIG_BCM_PHY_LANAI_A0) || defined(CONFIG_BCM_PHY_NIIHAU_A0)
static int bus_write_reg32(u32 addr, u32 reg, u32 val, u16 ctrl, loading_reg_t *load_reg)
{
	if (!load_reg)
		load_reg = &default_load_reg;

	_bus_write(addr, load_reg->devid, load_reg->addr_low, reg & 0xffff);
	_bus_write(addr, load_reg->devid, load_reg->addr_high, (reg >> 16) & 0xffff);
	_bus_write(addr, load_reg->devid, load_reg->data_low, val & 0xffff);
	_bus_write(addr, load_reg->devid, load_reg->data_high, (val >> 16) & 0xffff);
	_bus_write(addr, load_reg->devid, load_reg->ctrl, ctrl);
	return 0;
}

static int bus_write_reg32_all(u32 phy_map, u32 reg, u32 val, u16 ctrl, loading_reg_t *load_reg)
{
	int i;

	for (i = 0; i < 32; i++) {
		if (!((1 << i) & phy_map))
			continue;
		bus_write_reg32(i, reg, val, ctrl, load_reg);
	}
	return 0;
}
#endif

#if defined(USXGMII_MP_PHY)
static int get_usxgmii_m_base_addr(int phy_addr)
{
	return phy_addr - get_usxgmii_m_core_id(phy_addr);
}

/* We need to get usxgmii_m_base address even it is not in board design map */
static u32 get_usxgmii_m_base_map(u32 phy_map)
{
	u32 usxgmii_base_map = 0;
	int i;

	for (i = 0; i < 32; i++) {
		if (((1 << i) & phy_map) == 0)
			continue;
		usxgmii_base_map |= (1 << get_usxgmii_m_base_addr(i));
	}

	return usxgmii_base_map;
}
#endif

#if defined(CONFIG_BCM_PHY_SHORTFIN_B0)
#define PHY_ADDR_PHYSICAL(x) x
void plp_shortfin_set_broadcast_mode(u32 phy_map_with_base, u32 devad, uint8_t ena)
{
	u32 prt_idx;
	u32 base_phy_addr = -1;

	for (prt_idx = 0; prt_idx < 32; prt_idx++) {
		if (!(phy_map_with_base & (1 << prt_idx)))
			continue;

		if (base_phy_addr == -1) {
			base_phy_addr = get_usxgmii_m_base_addr(prt_idx);
			continue;
		}

		if (ena) {
			/* change xgp table to acept mdio writes to port st_prtad */
			_bus_write(prt_idx, 0x1e, 0x4107, (0x0400 | ((PHY_ADDR_PHYSICAL(base_phy_addr) & 0x1f) << 5)) | (devad & 0x1f));
			_bus_write(prt_idx, 0x1e, 0x4117, 0xF001);
			/* These setting would be automatically cleared once do soft reset */
		} else {
			/* change xgp table to acept mdio writes to port st_prtad */
			_bus_write(prt_idx, 0x1e, 0x4107, 0x0000);
			_bus_write(prt_idx, 0x1e, 0x4117, 0x0000);
			/* These setting would be automatically cleared once do soft reset */
		}
	}
}

static int load_shortfin(firmware_t *firmware)
{
	/*
	 * phy_map is the ports defined by DT which might not contain base port.
	 * while phy_map_with_base contains missing base port if DT does not define it.
	 */
	int i, ret, master_addr, phy_cnt;
	u32 phy_map, mphy_base_map, phy_map_with_base, mphy_non_master_base_map;

	phy_map = firmware->map;
	mphy_base_map = get_usxgmii_m_base_map(phy_map);
	phy_map_with_base = phy_map | mphy_base_map;
	master_addr = get_base_phy_addr(phy_map_with_base); /* select the min PHY address for broadcast operation */
	mphy_non_master_base_map = mphy_base_map & ~(1 << master_addr);

	phy_cnt = phy_count(phy_map_with_base);

	printk("phy_map:%08x, mphy_base_map:%08x, phy_map_with_base:%08x, mphy_non_master_base_map:%08x, master_addr:%d, phy_cnt:%d\n",
		phy_map, mphy_base_map, phy_map_with_base, mphy_non_master_base_map, master_addr, phy_cnt);

	/*
	 * 0. Set BUSY_IN pin to 0;
	 * This is a work around for floating BUSY_IN pin on 54994EL daugter card,
	 * but it is also a correct configuration for any correct designed EXT3 PHY board with BUSY_IN grounded.
	 */
	printk("Step1. Set BUSY_IN pin to 0;\n");
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x80b5, 0x0001);

	printk("Step2. Reset and clear Watchdog timer and enable clocks\n");
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x418c, 0x0000);
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x4188, 0x48f0);
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x4110, 0x0001);

	printk("Step3. Put All CPUs in halt\n");
	bus_write_reg32_all(mphy_base_map, 0xf0003000, 0x00000121, 9, 0);

	printk("Step4. For Shortfin or Broadfin clear IOPAD_CFG_SPARE_REG5_P0 so FW will re-do pll configuration\n");
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x80a6, 0x0000);

	printk("Step5. Clear COMMON_MAILBOX_9(SERDES loader done status), so FW can re-do SERDES loader\n");
	BUS_WRITE_ALL(phy_map_with_base, 0x1, 0xa010, 0x0000);

	printk("Step6. Issue soft reset on all ports\n");
	BUS_WRITE_ALL(phy_map_with_base, 0x01, 0x0000, 0x8000);

	printk("Step7. Enable broadcast mode for device 1\n");
	BUS_WRITE_ALL(phy_map_with_base, 0x1e, 0x4110, 0x0001);
	if (phy_cnt > 1)
		plp_shortfin_set_broadcast_mode(phy_map_with_base, 1, 1);

	printk("Step8. Upload the firmware file content into the on-chip memory of the device\n");
	ret = load_firmware_file(master_addr, firmware, 0);	/* Load to single address */
	if (ret)
		goto Exit;

	printk("Step9. Disable broadcast mode for device 1\n");
	if (phy_cnt > 1)
		plp_shortfin_set_broadcast_mode(phy_map_with_base, 1, 0);

	printk("Step10. Bring CPU0 out of halt\n");
	printk("Step11. Finished uploading Firmware, CORTEXR4 jumping to address 0x00000000.\n");
	bus_write_reg32_all(mphy_base_map, 0xf0003000, 0x00000020, 9, 0);
	BUS_WRITE_ALL(mphy_base_map, 0x01, 0xa817, 0x0000);

	/* 6. Verify that the processors are running */
	pr_cont("Verify that the processors are running: ");
	i = 1000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map_with_base, 0x01, 0x0000, 0x2040, 0xffff);
	} while ((ret != phy_map_with_base) && i--);
	pr_cont("%s\n", ret != phy_map_with_base ? "Failed" : "OK");

	ret = ret != phy_map_with_base;
	if (ret)
		goto Exit;

	/* 7. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
	pr_cont("Verify that the firmware has been loaded with good CRC: ");

	i = 1000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map_with_base, 0x1e, 0x400d, 0x4000, 0xc000);
	} while ((ret != phy_map_with_base) && i--);

	pr_cont("%s\n", ret != phy_map_with_base ? "Failed" : "OK");

	ret = ret != phy_map_with_base;
	if (ret)
		goto Exit;

	print_firmware_version(master_addr);

Exit:
	if (ret)
		verify_firmware_file(master_addr, firmware, 0);
	return ret;
}
#endif

void phy_config_shared_ref_clk_map(int phy_map);
#if defined(CONFIG_BCM_PHY_BLACKFIN_A0) || defined(CONFIG_BCM_PHY_BLACKFIN_B0) || \
	defined(CONFIG_BCM_PHY_LONGFIN_A0) || defined(CONFIG_BCM_PHY_LONGFIN_B0) || \
	defined(CONFIG_BCM_PHY_XPHY_16)
static int load_blackfin(firmware_t *firmware)
{
	int i, ret, base_phy_addr, phy_cnt;
	u32 phy_map, b_phy_map;

	phy_map = firmware->map;
	base_phy_addr = get_base_phy_addr(phy_map); /* select the min PHY address for broadcast operation */
	b_phy_map = bcast_phy_map(phy_map); /* phy map for broadcast configuration - exclude the min PHY address */
	phy_cnt = phy_count(phy_map);

	/* 1. Halt the Blackfin processors operation */
	printk("Halt the PHYs processors operation\n");

	BUS_WRITE_ALL(phy_map, 0x1e, 0x4110, 0x0001);

	BUS_WRITE_ALL(phy_map, 0x1e, 0x418c, 0x0000);
	BUS_WRITE_ALL(phy_map, 0x1e, 0x4188, 0x48f0);

	BUS_WRITE_ALL(phy_map, 0x01, 0xa81a, 0xf000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa819, 0x3000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa81c, 0x0000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa81b, 0x0121);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa817, 0x0009);

	BUS_WRITE_ALL(phy_map, 0x1e, 0x80a6, 0x0000);
	BUS_WRITE_ALL(phy_map, 0x1, 0xa010, 0x0000);
	BUS_WRITE_ALL(phy_map, 0x1, 0x0000, 0x8000);

	udelay(1000);
	BUS_WRITE_ALL(phy_map, 0x1e, 0x4110, 0x0001);
	udelay(1000);

	/* 2. If more than one PHY, turn on broadcast mode to accept write operations for addr = base_phy_addr */
	if (phy_cnt > 1) {
		printk("Turn on broadcast mode to accept write operations\n");
		BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4107, 0x0401 | ((base_phy_addr & 0x1f) << 5));
		BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4117, 0xf001);
	}

	/* 3. Upload the firmware into the on-chip memory of the devices */
	ret = load_firmware_file(base_phy_addr, firmware, 0);
	if (ret)
		goto Exit;

	/* 4. Disable broadcast if phy cnt > 1 */
	if (phy_cnt > 1) {
		BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4107, 0x0000);
		BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4117, 0x0000);
	}

	/* 4.5 Write Shared Clock again here */
	phy_config_shared_ref_clk_map(phy_map);

	/* 5. Reset the processors to start execution of the code in the on-chip memory */
	printk("Reset the processors to start execution of the code in the on-chip memory\n");

	BUS_WRITE_ALL(phy_map, 0x01, 0xa81a, 0xf000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa819, 0x3000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa81c, 0x0000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa81b, 0x0020);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa817, 0x0009);

	udelay(2000);

	/* 6. Verify that the processors are running */
	pr_cont("Verify that the processors are running: ");

	i = 1000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map, 0x01, 0x0000, 0x2040, 0xffff);
	} while ((ret != phy_map) && i--);

	pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");

	ret = ret != phy_map;
	if (ret)
		goto Exit;

	/* 7. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
	pr_cont("Verify that the firmware has been loaded with good CRC: ");

	i = 1000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map, 0x1e, 0x400d, 0x4000, 0xc000);
	} while ((ret != phy_map) && i--);

	pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");

	ret = ret != phy_map;
	if (ret)
		goto Exit;

	print_firmware_version(base_phy_addr);

Exit:
	if (ret)
		verify_firmware_file(base_phy_addr, firmware, 0);
	return ret;
}
#endif

#if defined(CONFIG_BCM_PHY_XPHY_8)
static int load_xphy_8(firmware_t *firmware)
{
	int i, ret, base_phy_addr, phy_cnt;
	u32 phy_map, b_phy_map;

	phy_map = firmware->map;
	base_phy_addr = get_base_phy_addr(phy_map); /* select the min PHY address for broadcast operation */
	b_phy_map = bcast_phy_map(phy_map); /* phy map for broadcast configuration - exclude the min PHY address */
	phy_cnt = phy_count(phy_map);

	/* 1. Halt the Blackfin processors operation */
	printk("Halt the PHYs processors operation\n");

	BUS_WRITE_ALL(phy_map, 0x1e, 0x418c, 0x0000);
	BUS_WRITE_ALL(phy_map, 0x1e, 0x4188, 0x48f0);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa81a, 0xf000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa819, 0x3000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa81c, 0x0000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa81b, 0x0121);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa817, 0x0009);
	BUS_WRITE_ALL(phy_map, 0x01, 0x0000, 0x8000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa020, 0x000f);

	/* 2. If more than one PHY, turn on broadcast mode to accept write operations for addr = base_phy_addr */
	if (phy_cnt > 1) {
		printk("Turn on broadcast mode to accept write operations\n");
		BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4107, 0x0401 | ((base_phy_addr & 0x1f) << 5));
		BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4117, 0xf001);
	}

	/* 3. Upload the firmware into the on-chip memory of the devices */
	ret = load_firmware_file(base_phy_addr, firmware, 0);
	if (ret)
		goto Exit;

	/* 4. Disable broadcast if phy cnt > 1 */
	if (phy_cnt > 1) {
		BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4107, 0x0000);
		BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4117, 0x0000);
	}

	/* 5. Reset the processors to start execution of the code in the on-chip memory */
	printk("Reset the processors to start execution of the code in the on-chip memory\n");

	BUS_WRITE_ALL(phy_map, 0x01, 0xa81a, 0xf000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa819, 0x3000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa81c, 0x0000);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa81b, 0x0020);
	BUS_WRITE_ALL(phy_map, 0x01, 0xa817, 0x0009);

	udelay(2000);

	/* 6. Verify that the processors are running */
	pr_cont("Verify that the processors are running: ");

	i = 1000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map, 0x01, 0x0000, 0x2040, 0xffff);
	} while ((ret != phy_map) && i--);

	pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");

	ret = ret != phy_map;
	if (ret)
		goto Exit;

	/* 7. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
	pr_cont("Verify that the firmware has been loaded with good CRC: ");

	i = 1000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map, 0x1e, 0x400d, 0x4000, 0xc000);
	} while ((ret != phy_map) && i--);

	pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");

	ret = ret != phy_map;
	if (ret)
		goto Exit;

	print_firmware_version(base_phy_addr);

Exit:
	if (ret)
		verify_firmware_file(base_phy_addr, firmware, 0);

	return ret;
}
#endif

#if defined(CONFIG_BCM_PHY_KAUAI_A0)
static int load_kauai(firmware_t *firmware)
{
	/*
	 * phy_map is the ports defined by DT which might not contain base port.
	 * while phy_map_with_base contains missing base port if DT does not define it.
	 */
	int i, ret, master_addr, phy_cnt;
	u32 phy_map, mphy_base_map, phy_map_with_base, mphy_non_master_base_map;
	static loading_reg_t load_reg = {
		.ram_addr	= 0xf7900000,
		.devid		= 0x1e,
		.ctrl		= 0x4138,
		.addr_low	= 0x413a,
		.addr_high	= 0x413b,
		.data_low	= 0x413c,
		.data_high	= 0x413d,
	};

	phy_map = firmware->map;
	mphy_base_map = get_usxgmii_m_base_map(phy_map);
	phy_map_with_base = phy_map | mphy_base_map;
	master_addr = get_base_phy_addr(phy_map_with_base); /* select the min PHY address for broadcast operation */
	mphy_non_master_base_map = mphy_base_map & ~(1 << master_addr);

	phy_cnt = phy_count(phy_map_with_base);

	printk("phy_map:%08x, mphy_base_map:%08x, phy_map_with_base:%08x, mphy_non_master_base_map:%08x, master_addr:%d, phy_cnt:%d\n",
		phy_map, mphy_base_map, phy_map_with_base, mphy_non_master_base_map, master_addr, phy_cnt);

	/* Step1. Set in Chip broadcast */
	printk("Step1. Set in Chip broadcast.\n");
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x8914, 0xffff);
	printk(" Set XGP table for mdio2arm_devad (for the 1st port of each chip except the Master.\n");
	BUS_WRITE_ALL(mphy_non_master_base_map, 0x1e, 0x4107, (0x0400 | (master_addr & 0x1f) << 5) | (0x1e));
	BUS_WRITE_ALL(mphy_non_master_base_map, 0x1e, 0x4117, 0xf001);

	printk("Step2. Reset and clear Watchdog timer and enable clocks.\n");
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x418c, 0x0000);
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x4188, 0x48f0);

	printk("Step3. Put All CPUs in halt\n");
	bus_write_reg32_all(mphy_base_map, 0xf4003000, 0x00000121, 9, &load_reg);

	printk("step4: for Kauai, IOPAD_CFG_SPARE_REG5[1:0] = [serdes done, PLL done].\n");
	{
		u16 v16;

		BUS_READ(master_addr, 0x1e, 0x88a6, &v16);
		v16 &= 0x0004;
		BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x88a6, v16);
	}

	printk("Step5. Clear COMMON_MAILBOX_9(SERDES loader done status), so FW can re-do SERDES loader\n");
	bus_write_reg32_all(mphy_base_map, 0xf6034020, 0x00000000, 5, &load_reg);

	printk("Step5.2: For KAUAI clear serdes PMI_ADDR_CTL");
	bus_write_reg32_all(mphy_base_map, 0xf7074006, 0x00000000, 5, &load_reg);

	printk("Step6. Issue soft reset on all ports\n");
	bus_write_reg32_all(mphy_base_map, 0xf6020000, 0x00008000, 5, &load_reg);

	printk("Step7. Turn on broadcast mode for non-master ports\n");
	BUS_WRITE_ALL(mphy_non_master_base_map, 0x1e, 0x4107, 0x0400 | ((master_addr & 0x1f) << 5) | 0x1e);
	BUS_WRITE_ALL(mphy_non_master_base_map, 0x1e, 0x4117, 0xf001);

	printk("Step8. Upload the firmware file content into the on-chip memory of the device\n");
	ret = load_firmware_file(master_addr, firmware, &load_reg);	/* Load to single address */
	if (ret)
		goto Exit;

	printk("Step10. Bring CPU0 out of halt\n");
	printk("Step11. Finished uploading Firmware, CORTEXR4 jumping to address 0x00000000.\n");
	bus_write_reg32_all(mphy_base_map, 0xf0003000, 0x00000020, 9, &load_reg);

	printk("Step12. Finishing uploading, turn off in chip broadcasting.\n");
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x8914, 0x0000);

	/* 6. Verify that the processors are running */
	pr_cont("Verify that the processors are running: ");
	i = 1000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map_with_base, 0x01, 0x0000, 0x2040, 0xffff);
	} while ((ret != phy_map_with_base) && i--);
	pr_cont("%s\n", ret != phy_map_with_base ? "Failed" : "OK");

	ret = ret != phy_map_with_base;
	if (ret)
		goto Exit;

	/* 7. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
	printk("Verify that the firmware has been loaded with good CRC: ");
	i = 8000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map_with_base, 0x1e, 0x400d, 0x4000, 0xc000);
	} while ((ret != phy_map_with_base) && i--);

	pr_cont("%s\n", ret != phy_map_with_base ? "Failed" : "OK");

	ret = ret != phy_map_with_base;
	if (ret)
		goto Exit;

	print_firmware_version(master_addr);

Exit:
	if (ret)
		verify_firmware_file(master_addr, firmware, &load_reg);

	return ret;
}
#endif

#if defined(CONFIG_BCM_PHY_LANAI_A0)
static int load_lanai(firmware_t *firmware)
{
	/*
	 * phy_map is the ports defined by DT which might not contain base port.
	 * while phy_map_with_base contains missing base port if DT does not define it.
	 */
	int i, ret, master_addr, phy_cnt;
	u32 phy_map, mphy_base_map, phy_map_with_base, mphy_non_master_base_map;

	static loading_reg_t load_reg = {
		.ram_addr	= 0xf7900000,
		.devid		= 0x1e,
		.ctrl		= 0x4138,
		.addr_low	= 0x413a,
		.addr_high	= 0x413b,
		.data_low	= 0x413c,
		.data_high	= 0x413d,
	};

	phy_map = firmware->map;
	mphy_base_map = phy_map;
	phy_map_with_base = phy_map | mphy_base_map;
	master_addr = get_base_phy_addr(phy_map_with_base); /* select the min PHY address for broadcast operation */
	mphy_non_master_base_map = mphy_base_map & ~(1 << master_addr);

	phy_cnt = phy_count(phy_map_with_base);

	printk("phy_map:%08x, mphy_base_map:%08x, phy_map_with_base:%08x, mphy_non_master_base_map:%08x, master_addr:%d, phy_cnt:%d\n",
		phy_map, mphy_base_map, phy_map_with_base, mphy_non_master_base_map, master_addr, phy_cnt);

#if 0
	printk("Step1. Set in Chip broadcast.\n");
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x8914, 0xffff);
	printk(" Set XGP table for mdio2arm_devad (for the 1st port of each chip except the Master.\n");
	BUS_WRITE_ALL(mphy_non_master_base_map, 0x1e, 0x4107, (0x0400 | (master_addr & 0x1f) << 5) | (0x1e));
	BUS_WRITE_ALL(mphy_non_master_base_map, 0x1e, 0x4117, 0xf001);
#endif

#if 0
	printk("Step2. Reset and clear Watchdog timer and enable clocks.\n");
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x418c, 0x0000);
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x4188, 0x48f0);
#endif

	printk("Step3. Put All CPUs in halt\n");
	bus_write_reg32_all(mphy_base_map, 0xf0003000, 0x00000121, 9, &load_reg);

	printk("Step4. Set 0x1e.88a6 to 0\n");
	BUS_WRITE_ALL(phy_map_with_base, 0x1e, 0x88a6, 0);

	printk("Step6. Issue soft reset on all ports\n");
	BUS_WRITE_ALL(phy_map_with_base, 0x1, 0x0000, 0x8000);

	printk("Step6.2. Clear TMUX_MODE to 0\n");
	BUS_WRITE_ALL(phy_map_with_base, 0x1e, 0x8a81, 0x0000);

	printk("Step7. Step7. Enable broadcast mode for device 1\n");
	BUS_WRITE_ALL((phy_map_with_base & ~(1 << master_addr)), 0x1e, 0x4107, 0x0400 | ((master_addr & 0x1f) << 5) | 0x1e);
	BUS_WRITE_ALL((phy_map_with_base & ~(1 << master_addr)), 0x1e, 0x4117, 0xf001);

	printk("Step8. Upload the firmware file content into the on-chip memory of the device\n");
	ret = load_firmware_file(master_addr, firmware, &load_reg);
	if (ret)
		goto Exit;

	printk("Step9. Step9. Disable broadcast mode for device 1\n");
	BUS_WRITE_ALL((phy_map_with_base & ~(1 << master_addr)), 0x1e, 0x4107, 0x0000);
	BUS_WRITE_ALL((phy_map_with_base & ~(1 << master_addr)), 0x1e, 0x4117, 0x0000);

	printk("Step10. Bring CPU0 out of halt\n");
	printk("Step11. Finished uploading Firmware, CORTEXR4 jumping to address 0x00000000.\n");
	bus_write_reg32_all(mphy_base_map, 0xf0003000, 0x00000020, 9, &load_reg);

	/* 6. Verify that the processors are running */
	pr_cont("Verify that the processors are running: ");
	i = 1000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map_with_base, 0x01, 0x0000, 0x2040, 0xffff);
	} while ((ret != phy_map_with_base) && i--);
	pr_cont("%s\n", ret != phy_map_with_base ? "Failed" : "OK");

	ret = ret != phy_map_with_base;
	if (ret)
		goto Exit;

	/* 7. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
	printk("Verify that the firmware has been loaded with good CRC: ");
	i = 8000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map_with_base, 0x1e, 0x400d, 0x4000, 0xc000);
	} while ((ret != phy_map_with_base) && i--);

	pr_cont("%s\n", ret != phy_map_with_base ? "Failed" : "OK");

	ret = ret != phy_map_with_base;
	if (ret)
		goto Exit;

	print_firmware_version(master_addr);

Exit:
	if (ret)
		verify_firmware_file(master_addr, firmware, &load_reg);
	return ret;
}
#endif

#if defined(CONFIG_BCM_PHY_NIIHAU_A0)
static int load_niihau(firmware_t *firmware)
{
	/*
	 * phy_map is the ports defined by DT which might not contain base port.
	 * while phy_map_with_base contains missing base port if DT does not define it.
	 */
	int i, ret, master_addr, phy_cnt;
	u32 phy_map, mphy_base_map, phy_map_with_base, mphy_non_master_base_map;

	static loading_reg_t load_reg = {
		.ram_addr	= 0x00000000,
		.devid		= 0x1e,
		.ctrl		= 0x4138,
		.addr_low	= 0x413a,
		.addr_high	= 0x413b,
		.data_low	= 0x413c,
		.data_high	= 0x413d,
	};

	phy_map = firmware->map;
	mphy_base_map = phy_map;
	phy_map_with_base = phy_map | mphy_base_map;
	master_addr = get_base_phy_addr(phy_map_with_base); /* select the min PHY address for broadcast operation */
	mphy_non_master_base_map = mphy_base_map & ~(1 << master_addr);

	phy_cnt = phy_count(phy_map_with_base);

	printk("phy_map:%08x, mphy_base_map:%08x, phy_map_with_base:%08x, mphy_non_master_base_map:%08x, master_addr:%d, phy_cnt:%d\n",
		phy_map, mphy_base_map, phy_map_with_base, mphy_non_master_base_map, master_addr, phy_cnt);

#if 0
	printk("Step1. Set in Chip broadcast.\n");
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x8914, 0xffff);
	printk(" Set XGP table for mdio2arm_devad (for the 1st port of each chip except the Master.\n");
	BUS_WRITE_ALL(mphy_non_master_base_map, 0x1e, 0x4107, (0x0400 | (master_addr & 0x1f) << 5) | (0x1e));
	BUS_WRITE_ALL(mphy_non_master_base_map, 0x1e, 0x4117, 0xf001);
#endif

	printk("Step2. Reset and clear Watchdog timer and enable clocks.\n");
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x418c, 0x0000);
	BUS_WRITE_ALL(mphy_base_map, 0x1e, 0x4188, 0x48f0);

	printk("Step3. Put All CPUs in halt\n");
	bus_write_reg32_all(mphy_base_map, 0xf0003000, 0x00000121, 9, &load_reg);

	printk("Step4. Set 0x1e.88a6 to 0 to re-do PLL\n");
	BUS_WRITE_ALL(phy_map_with_base, 0x1e, 0x88a6, 0);

	printk("Step6. Issue soft reset on all ports\n");
	BUS_WRITE_ALL(phy_map_with_base, 0x1, 0x0000, 0x8000);

	printk("Step6.2. Clear TMUX_MODE to 0\n");
	BUS_WRITE_ALL(phy_map_with_base, 0x1e, 0x8a81, 0x0000);

	printk("Step7. Step7. Enable broadcast mode for device 1\n");
	BUS_WRITE_ALL((phy_map_with_base & ~(1 << master_addr)), 0x1e, 0x4107, 0x0400 | ((master_addr & 0x1f) << 5) | 0x1e);
	BUS_WRITE_ALL((phy_map_with_base & ~(1 << master_addr)), 0x1e, 0x4117, 0xf001);

	printk("Step8. Upload the firmware file content into the on-chip memory of the device\n");
	ret = load_firmware_file(master_addr, firmware, &load_reg);
	if (ret)
		goto Exit;

	printk("Step9. Step9. Disable broadcast mode for device 1\n");
	BUS_WRITE_ALL((phy_map_with_base & ~(1 << master_addr)), 0x1e, 0x4107, 0x0000);
	BUS_WRITE_ALL((phy_map_with_base & ~(1 << master_addr)), 0x1e, 0x4117, 0x0000);

	printk("Step10. Bring CPU0 out of halt\n");
	printk("Step11. Finished uploading Firmware, CORTEXR4 jumping to address 0x00000000.\n");
	bus_write_reg32_all(mphy_base_map, 0xf0003000, 0x00000020, 9, &load_reg);

	/* 6. Verify that the processors are running */
	pr_cont("Verify that the processors are running: ");
	i = 1000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map_with_base, 0x01, 0x0000, 0x2040, 0xffff);
	} while ((ret != phy_map_with_base) && i--);
	pr_cont("%s\n", ret != phy_map_with_base ? "Failed" : "OK");

	ret = ret != phy_map_with_base;
	if (ret)
		goto Exit;

	/* 7. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
	printk("Verify that the firmware has been loaded with good CRC: ");
	i = 8000;
	do {
		udelay(2000);
		ret = _bus_read_all(phy_map_with_base, 0x1e, 0x400d, 0x4000, 0xc000);
	} while ((ret != phy_map_with_base) && i--);

	pr_cont("%s\n", ret != phy_map_with_base ? "Failed" : "OK");

	ret = ret != phy_map_with_base;
	if (ret)
		goto Exit;

	print_firmware_version(master_addr);

Exit:
	if (ret)
		verify_firmware_file(master_addr, firmware, &load_reg);
	return ret;
}
#endif

static void phy_reset_lift(phy_dev_t *phy_dev)
{
	if (dt_gpio_exists(phy_dev->gpiod_phy_power)) {
		printk("Power up PHY at address 0x%x\n", phy_dev->addr);
		dt_gpio_set_value(phy_dev->gpiod_phy_power, 1);
		mdelay(10);
	}

	if (dt_gpio_exists(phy_dev->gpiod_phy_reset)) {
		printk("Un-reset PHY at address 0x%x\n", phy_dev->addr);
		dt_gpio_set_value(phy_dev->gpiod_phy_reset, 0);
		mdelay(10);
	}
}

static void phys_reset_lift(u32 *phy_map)
{
	int i;
	phy_dev_t *phy_dev;

	for (i = 0; i < 32; i++) {
		if (!(*phy_map & (1 << i)))
			continue;

		phy_dev = phy_dev_get(PHY_TYPE_EXT3, i);
		if (!phy_dev || ((phy_dev->flag & PHY_FLAG_ON_MEZZANINE) && (phy_dev->flag & PHY_FLAG_NOT_PRESENTED)))
			*phy_map &= ~(1 << i);
		else
			phy_reset_lift(phy_dev);
	}
}

static int phy_config_is_shared_ref_clk_set(phy_dev_t *phy_dev)
{
	u16 v16;

	/* Check firmware CRC bit to see if the PHY has been initialized in BSP */
	if (phy_bus_c45_read(phy_dev, 0x1e, 0x400d, &v16))
		return -1;

	return (v16 & 0xc000) == 0x4000;
}

static void phy_config_shared_ref_clk(phy_dev_t *phy_dev)
{
	u16 v16;

	if (!phy_dev->shared_ref_clk_mhz)
		return;

	/* Only BlackFin/LongFin family needs frequency adjustment */
	if (1
#if defined(CONFIG_BCM_PHY_LONGFIN_A0) || defined(CONFIG_BCM_PHY_LONGFIN_B0)
		&& !_phy_use_firmware(phy_dev, &longfin_a0) && !_phy_use_firmware(phy_dev, &longfin_a0_m)
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_B0
		&& !_phy_use_firmware(phy_dev, &blackfin_b0)
#endif
	   ) {
		printk("%d MHZ shared reference clock on XGPHY at address %d is supported natively\n",
			phy_dev->shared_ref_clk_mhz, phy_dev->addr);
		return;
	}

	if (phy_bus_c45_read(phy_dev, 0x1e, 0x80a8, &v16))
		return;

	switch (phy_dev->shared_ref_clk_mhz) {
	case 80:
		v16 |= 1;
		phy_bus_c45_write(phy_dev, 0x1e, 0x80a8, v16);
		break;
	default:
		printk("%d MHz Shared Reference Clock is natively supported by PHY at %d\n", phy_dev->shared_ref_clk_mhz, phy_dev->addr);
		return;
	}

	PhySetSharedRefClk(phy_dev);
	printk("Set %d MHZ clock sharing on XGPHY at address %d\n",
		phy_dev->shared_ref_clk_mhz, phy_dev->addr);
}

void phy_config_shared_ref_clk_map(int phy_map)
{
	phy_dev_t *phy_dev;
	int i;

	for (i = 0; i < 32; i++) {
		if (!(phy_map & (1 << i)))
			continue;
		phy_dev = phy_dev_get(PHY_TYPE_EXT3, i);
		phy_config_shared_ref_clk(phy_dev);
	}
}

static int _phy_shared_clock_set(phy_dev_t *phy_dev)
{
	phy_dev_t *phy_prev;

	if (!phy_dev->shared_ref_clk_mhz)
		return 0;

	phy_prev = cascade_phy_get_prev(phy_dev);
	phy_dev_shared_clock_set(phy_prev);

	if (phy_config_is_shared_ref_clk_set(phy_dev)) {
		printk("80 MHZ clock sharing on XGPHY at address %d was set in BSP already\n", phy_dev->addr);
		return 0;
	} else {
		/* Extra Reset is needed here for Blackfin 80MHz REF clock at least */
		phy_bus_c45_write(phy_dev, 0x1e, 0x8000, 0x8007);
		udelay(2000);
		phy_bus_c45_write(phy_dev, 0x1e, 0x8000, 0x8004);
		udelay(2000);
	}

	phy_config_shared_ref_clk(phy_dev);
	return 0;
}

static inline char *print_phy_map(u32 phy_map)
{
	int i, j;
	static char buf[256];

	for (i = 0, j = 0; i < 32; i++)
		if ((1 << i) & phy_map)
			j += sprintf(buf + j, " %d", i);
	return buf;
}

static inline char *print_phy_names(u32 phy_map)
{
	int i, j;
	static char buf[256];
	phy_desc_t *phy_desc;
	phy_dev_t *phy_dev;
	char *cur_name = 0;

	for (i = 0, j = 0; i < 32; i++) {
		if (((1 << i) & phy_map) == 0)
			continue;
		phy_dev = phy_dev_get(PHY_TYPE_EXT3, i);
        phy_desc = _phy_get_desc_from_phydev(phy_dev);
		if (cur_name != phy_desc->name)
			j += sprintf(buf + j, " %s,", phy_desc->name);
		cur_name = phy_desc->name;
	}
	return buf;
}

static int _phy_cfg(void)
{
    int i, ret = 0;
    phy_dev_t *phy_dev;
    uint32_t phy_map;
    phy_desc_t *phy_desc;

	printk("\nDetecting PHYs...\n");

	phys_reset_lift(&enabled_phys);

	if (!enabled_phys)
		return 0;

    for (i = 0; i < 32; i++, ret = 0)
    {
        if (!(enabled_phys & (1 << i)))
            continue;

        phy_dev = phy_dev_get(PHY_TYPE_UNKNOWN, i);
        if (!phy_dev)
            continue;

        phy_desc = _phy_get_desc_from_phydev(phy_dev);
        if (!phy_desc)
        {
            printk("No PHY descriptor found for PHY at %d; %04x:%04x;%04x:%04x\n", 
                    phy_dev->addr, phy_dev->phy_id1, phy_dev->phy_id2, phy_dev->pkg_id1, phy_dev->pkg_id2);
            continue;
        }
        phy_desc->firmware->map |= (1 << i);
 
#if defined(RESCAL_BY_XPHY)
        if (phy_desc->firmware == &xgphy_16)
            xphy_rescal(i);
#endif
        printk("    0x%x --> %s %04x:%04x,%04x:%04x\n", i, phy_desc->name, 
                phy_desc->phyid1, phy_desc->phyid2, phy_desc->packageid1, phy_desc->packageid2);
	}

	for (i = 0; i < (int)ARRAY_SIZE(firmware_list); i++) {
		phy_map = firmware_list[i]->map;

		if (!phy_map)
			continue;

		printk("Loading firmware into detected PHYs: map=0x%x count=%d\n", phy_map, phy_count(phy_map));
		printk("    PHY Addresses at %s, PHY Names: %s Firmware Name: %s\n", print_phy_map(phy_map), print_phy_names(phy_map),
			firmware_list[i]->name);

		ret += firmware_list[i]->load(firmware_list[i]);
		printk("\n");
	}

	return ret;
}

static int _phy_dev_add(phy_dev_t *phy_dev)
{
	bus_drv = phy_dev->bus_drv;
	enabled_phys |= (1 << (phy_dev->addr));

	return 0;
}

static int _phy_dev_del(phy_dev_t *phy_dev)
{
	enabled_phys &= ~(1 << (phy_dev->addr));

	return 0;
}

static int _phy_drv_init(phy_drv_t *phy_drv)
{
#if defined(DSL_DEVICES)
	extern void phy_bus_probe(bus_drv_t *bus_drv);

	phy_bus_probe(bus_drv);
#endif
	if (_phy_cfg()) {
		printk("Failed to initialize the driver\n");
	}

	phy_drv->initialized = 1;

	return 0;
}

static char *_phy_get_phy_name(phy_dev_t *phy_dev)
{
	static char buf[256];
	phy_desc_t *phy_desc = _phy_get_desc_from_phydev(phy_dev);

	if (!phy_desc)
		return "No Descriptor Found."; // bug

	sprintf(buf, "%s:%-s", phy_desc->name, get_firmware_version(phy_dev->addr));

	return buf;
}

#define EXT3_ECD_CTRL_STATUS_D			0x1e
#define EXT3_ECD_CTRL_STATUS_R			0x4006
#define EXT3_ECDRUN_IMMEDIATE			BIT(15)
#define EXT3_ECDBREAK_LINK				BIT(12)
#define EXT3_ECDDIAG_IN_PROG			BIT(11)

#define EXT3_ECD_RESULTS_D				0x1
#define EXT3_ECD_RESULTS_R				0xa896
#define EXT3_PACD_CODE_SHIFT			4
#define EXT3_PACD_CODE_MASK				0xf
#define EXT3_PACD_CODE_INVALID			0x0
#define EXT3_PACD_CODE_PAIR_OK			0x1
#define EXT3_PACD_CODE_PAIR_OPEN		0x2
#define EXT3_PACD_CODE_PAIR_INTRA_SHORT	0x3
#define EXT3_PACD_CODE_PAIR_INTER_SHORT	0x4
#define EXT3_PACD_CODE_PAIR_GET(v, p)	(((v) >> ((p) * 4)) & 0xf)
#define EXT3_PACD_CODE_PAIR_SET(v, p)	(((v) & 0xf) << ((p) * 4))
#define EXT3_PACD_CODE_PAIR_ALL_OK		0x1111
#define EXT3_PACD_CODE_PAIR_ALL_OPEN	0x2222

#define EXT3_ECD_CABLE_LEN_D 0x1
#define EXT3_ECD_CABLE_LEN_R 0xa897

/*
 * Work around some hardware inconsistency
 * Pick up the most popular length from 4 pairs
 */
static void cable_length_pick_link_up(phy_dev_t *phy_dev, int excluded_pair)
{
	int len[4] = {0};
	int i, j, k, m;
	int *pair_len = phy_dev->ethcd.pair_len_cm;

	for (i = 0, k = 0; i < 4; i++) {
		if (excluded_pair & (1 << i))	/* Exclude failed CD pair */
			continue;

		for (j = 0; j < k; j++)
			if (pair_len[j] == pair_len[i])
				break;

		if (j == k)
			k++;
		len[j]++;
	}

	for (i = 0, j = 0, m = 0; i < k; i++) {
		if (len[i] == 0)	/* If result is zero, exclude the pair from picking */
			continue;

		if (len[i] > j) {
			j = len[i];
			m = i;
		}
	}

	m = pair_len[m];
	for (i = 0; i < 4; i++)
		pair_len[i] = m;
}

int _phy_enhanced_cable_diag_run(phy_dev_t *phy_dev)
{
	u16 v16, ctrl_status;
	int i, j, ret = 0, result, excluded_pair = 0;
	int apd_enabled, phy_link;
	unsigned long jiffie;
	ethcd_t *ethcd = &phy_dev->ethcd;
	int pair_len[4];
	int retries = 0;
#define EXT3_ECDCHECK_SECS 3
#define EXT3_ECDMAX_RETRIES 3

	phy_dev_apd_get(phy_dev, &apd_enabled);
	if (apd_enabled)
		phy_dev_apd_set(phy_dev, 0);

	v16 = EXT3_ECDRUN_IMMEDIATE;

	phy_link = phy_dev->link;	/* Save initial PHY link status */

	if (phy_link) { /* If link is up, Write RUN first and wait until link goes down */
		PHY_WRITE(phy_dev, EXT3_ECD_CTRL_STATUS_D, EXT3_ECD_CTRL_STATUS_R, v16);
		for (;;) {
			phy_dev_read_status(phy_dev);
			if (!phy_dev->link)
				break;
		}
	}

TryAgain:
	if (retries)
		for (i = 0, jiffie = jiffies; jiffies < (jiffie + msecs_to_jiffies(2 * 1000));)
			;

	if (++retries > EXT3_ECDMAX_RETRIES)	/* If we did retry more than certain time, declares it as faiure */
		goto end;

	PHY_WRITE(phy_dev, EXT3_ECD_CTRL_STATUS_D, EXT3_ECD_CTRL_STATUS_R, v16);
	ret = phy_bus_c45_read(phy_dev, EXT3_ECD_CTRL_STATUS_D, EXT3_ECD_CTRL_STATUS_R, &v16);
	for (i = 0, jiffie = jiffies; jiffies < (jiffie + msecs_to_jiffies(EXT3_ECDCHECK_SECS * 1000)); ) {
		PHY_READ(phy_dev, EXT3_ECD_CTRL_STATUS_D, EXT3_ECD_CTRL_STATUS_R, &v16);
		if (!(v16 & EXT3_ECDDIAG_IN_PROG)) {
			ret = 0;
			i = 1;
			break;
		}
	}
	ctrl_status = v16;

	if (!i) {	/* If CD is still in progress after certain time, retry it */
		result = EXT3_PACD_CODE_INVALID;
		ret = -1;
		goto TryAgain;
	}

	for (i = 0, jiffie = jiffies; jiffies < (jiffie + msecs_to_jiffies(EXT3_ECDCHECK_SECS * 1000)); ) {
		PHY_READ(phy_dev, EXT3_ECD_RESULTS_D, EXT3_ECD_RESULTS_R, &v16);
		result = v16;
		excluded_pair = 0;
		for (j = 0; j < 4; j++) { /* Check if all four pairs of diags are done */
			if (EXT3_PACD_CODE_PAIR_GET(result, j) > EXT3_PACD_CODE_PAIR_INTER_SHORT)
				break;

			/* If link is up, excluded failed measuring result */
			if (phy_link && (EXT3_PACD_CODE_PAIR_GET(result, j) != EXT3_PACD_CODE_PAIR_OK))
				excluded_pair |= (1 << j);
		}

		/* If all pair of diags finish, check the results */
		if (j == 4) {
			/* If in link up, all pair diag failed, try again */
			if (result == EXT3_PACD_CODE_INVALID || excluded_pair == 0xf)
				goto TryAgain;
			/* Otherwise, we are done with CD */
			i = 1;
			break;
		}
	}

	if (phy_link)
		result = EXT3_PACD_CODE_PAIR_ALL_OK;

	if (result == EXT3_PACD_CODE_INVALID || !i) {	/* If CD ends with INVALID result, retry it */
		result = EXT3_PACD_CODE_INVALID;
		ret = -1;
		goto TryAgain;
	}

#define EXT3_CABLE_LEN_OFFSET_LINK_DOWN 200
	for (i = 0; i < 4; i++) {
		PHY_READ(phy_dev, EXT3_ECD_CABLE_LEN_D, (EXT3_ECD_CABLE_LEN_R + i), &v16);
		if (result == EXT3_PACD_CODE_PAIR_ALL_OPEN)
			pair_len[i] = (v16 > EXT3_CABLE_LEN_OFFSET_LINK_DOWN ? v16 - EXT3_CABLE_LEN_OFFSET_LINK_DOWN : 0); /* To guarrantee no cable result correct based on testing */
		else if (result == EXT3_PACD_CODE_PAIR_ALL_OK)
			pair_len[i] = v16 + EXT3_CABLE_LEN_OFFSET_LINK_DOWN;
		else
			pair_len[i] = v16;
	}

	/* If link is up, but all pair length is zero, try again */
	if (phy_link && (pair_len[0] + pair_len[1] + pair_len[2] + pair_len[3] == 0))
		goto TryAgain;

end:
	if (phy_link)
		cable_length_pick_link_up(phy_dev, excluded_pair);

	if (apd_enabled)
		phy_dev_apd_set(phy_dev, apd_enabled);

	/* Restart AN for potential link up during CD */
	phy_dev->link = 0;
	phy_dev_an_restart(phy_dev);

	memcpy(ethcd->pair_len_cm, pair_len, sizeof(pair_len));

	ethcd->return_value = ETHCD_OK;
	switch (result) {
	case EXT3_PACD_CODE_INVALID:
		ethcd->cable_code = ETHCD_STATUS_INVALID;
		ethcd->return_value = ETHCD_FAILED;
		break;
	case EXT3_PACD_CODE_PAIR_ALL_OK:
		ethcd->cable_code = ETHCD_STATUS_GOOD_CONNECTED;
		break;
	case EXT3_PACD_CODE_PAIR_ALL_OPEN:
		if ((pair_len[0] + pair_len[1] + pair_len[2] + pair_len[3]) == 0)
			ethcd->cable_code = ETHCD_STATUS_NO_CABLE;
		else if (pair_len[0] == pair_len[1] && pair_len[0] == pair_len[2] && pair_len[0] == pair_len[3])
			ethcd->cable_code = ETHCD_STATUS_GOOD_OPEN;
		else
			ethcd->cable_code = ETHCD_STATUS_BAD_OPEN;
		break;
	default:
		ethcd->cable_code = ETHCD_STATUS_MIXED_BAD;
		for (i = 0; i < 4; i++) {
			switch (EXT3_PACD_CODE_PAIR_GET(result, i)) {
			case EXT3_PACD_CODE_PAIR_OK:
				ethcd->pair_code[i] = ETHCD_PAIR_OK;
				break;
			case EXT3_PACD_CODE_PAIR_OPEN:
				ethcd->pair_code[i] = ETHCD_PAIR_OPEN;
				break;
			case EXT3_PACD_CODE_PAIR_INTRA_SHORT:
				ethcd->pair_code[i] = ETHCD_PAIR_INTRA_SHORT;
				break;
			case EXT3_PACD_CODE_PAIR_INTER_SHORT:
				ethcd->pair_code[i] = ETHCD_PAIR_INTER_SHORT;
				break;
			default:
				ethcd->pair_code[i] = ETHCD_PAIR_UNKNOWN;
				break;
			}
		}
		break;
	}

	ethcd->link = phy_link;
	ethcd->time_stamp = get_epoch_time64();
	ethcd->flag |= ETHCD_FLAG_DATA_VALID;
	return 0;
Exit:
	ethcd->cable_code = ETHCD_FAILED;
	return -1;
}

#if defined(MAC_LPORT)
extern int lport_led_init(void *leds_info);
#endif

static int _phy_leds_init(phy_dev_t *phy_dev, void *leds_info)
{
#if defined(MAC_LPORT)
	return lport_led_init(leds_info);
#else
	return xrdp_leds_init(leds_info);
#endif
}

static int _phy_diag(phy_dev_t *phy_dev, int level)
{
	int ret;
	u32 i;
	u16 data1, data2, data3;
	u16 val, high, low;
	static u32 data[2048] = {};

	printk("Type: %s\n", _phy_get_phy_name(phy_dev));

	ret = cmd_handler(phy_dev, CMD_GET_CURRENT_TEMP, &data1, NULL, NULL, NULL, NULL);
	if (ret)
		goto Exit;

	printk("Temperature: %d\n", data1);

	ret = cmd_handler(phy_dev, CMD_GET_CURRENT_VOLTAGE, &data1, &data2, &data3, NULL, NULL);
	if (ret)
		goto Exit;

	printk("Voltage: %d %d %d\n", data1, data2, data3);

	PHY_READ(phy_dev, 0x01, 0xa811, &val);
	PHY_WRITE(phy_dev, 0x01, 0xa819, 0xd000);
	PHY_WRITE(phy_dev, 0x01, 0xa81a, 0x0005);
	PHY_WRITE(phy_dev, 0x01, 0xa817, 0x003a);

	printk("\nLog\n===\n");

	for (i = 0; i < 2048; i++) {
		PHY_READ(phy_dev, 0x01, 0xa81c, &high);
		PHY_READ(phy_dev, 0x01, 0xa81b, &low);

		data[i] = high << 16 | low;
	}

	for (i = 0; i < 256; i++) {
		printk("0x%08x    %08x %08x %08x %08x %08x %08x %08x %08x\n", i * 8,
			data[8 * i + 0], data[8 * i + 1], data[8 * i + 2], data[8 * i + 3],
			data[8 * i + 4], data[8 * i + 5], data[8 * i + 6], data[8 * i + 7]);
	}

	PHY_READ(phy_dev, 0x01, 0xa811, &val);
	PHY_WRITE(phy_dev, 0x01, 0xa819, 0x7ff4);
	PHY_WRITE(phy_dev, 0x01, 0xa81a, 0x0005);
	PHY_WRITE(phy_dev, 0x01, 0xa817, 0x000a);

	PHY_READ(phy_dev, 0x01, 0xa81c, &high);
	PHY_READ(phy_dev, 0x01, 0xa81b, &low);

	PHY_WRITE(phy_dev, 0x01, 0xa819, 0xc000);
	PHY_WRITE(phy_dev, 0x01, 0xa81a, 0x0005);
	PHY_WRITE(phy_dev, 0x01, 0xa817, 0x003a);

	printk("\nTrace\n=====\n");

	for (i = 0; i < 1024; i++) {
		PHY_READ(phy_dev, 0x01, 0xa81c, &high);
		PHY_READ(phy_dev, 0x01, 0xa81b, &low);

		data[i] = high << 16 | low;
	}

	for (i = 0; i < 128; i++) {
		printk("0x%08x    %08x %08x %08x %08x %08x %08x %08x %08x\n", i * 8,
			data[8 * i + 0], data[8 * i + 1], data[8 * i + 2], data[8 * i + 3],
			data[8 * i + 4], data[8 * i + 5], data[8 * i + 6], data[8 * i + 7]);
	}

Exit:
	return ret;
}

static int _phy_read(phy_dev_t *phy_dev, u16 reg, u16 *val)
{
	return -EOPNOTSUPP;
}

static int _phy_write(phy_dev_t *phy_dev, u16 reg, u16 val)
{
	return -EOPNOTSUPP;
}

#define WOL_CTRL				0xd800
#define WOL_PKT1_15_0			0xd804
#define WOL_PKT1_31_16			0xd805
#define WOL_PKT1_47_32			0xd806
#define WOL_PKT2_15_0			0xd807
#define WOL_PKT2_31_16			0xd808
#define WOL_PKT2_47_32			0xd809
#define WOL_PKT_CNT				0xd812
#define WOL_INTR_MASK			0xd813
#define WOL_INTR_STATUS			0xd814

#define CTL_LED1_MASK_LOW		0xa82c
#define CTL_LED1_MASK_HIGH		0xa82d
#define CTL_LED1_MASK_EXT		0xa8ef
#define CTL_LED1_BLINK			0xa82e

#define CTL_LED2_MASK_LOW		0xa82f
#define CTL_LED2_MASK_HIGH		0xa830
#define CTL_LED2_MASK_EXT		0xa8f0
#define CTL_LED2_BLINK			0xa831

#define CTL_LED3_MASK_LOW		0xa832
#define CTL_LED3_MASK_HIGH		0xa833
#define CTL_LED3_MASK_EXT		0xa8f1
#define CTL_LED3_BLINK			0xa834

#define CTL_LED4_MASK_LOW		0xa835
#define CTL_LED4_MASK_HIGH		0xa836
#define CTL_LED4_MASK_EXT		0xa8f2
#define CTL_LED4_BLINK			0xa837

#define CTL_LED5_MASK_LOW		0xa838
#define CTL_LED5_MASK_HIGH		0xa839
#define CTL_LED5_MASK_EXT		0xa8f3
#define CTL_LED5_BLINK			0xa83a

#define CTL_LED_CONTROL			0xa83b
#define CTL_LED_CONTROL_1		0xa8ec
#define CTL_LED_CTRL_SRC_LOW	0xa83c
#define CTL_LED_CTRL_SRC_HIGH	0xa83d

#define CORE_CFG_LED_CONTROL	0x406d
#define CORE_CFG_HP_LED_CTL		0x407b
#define CORE_CFG_LED1_OVERRIDE	0x4080
#define CORE_CFG_LED2_OVERRIDE	0x407f
#define CORE_CFG_LED3_OVERRIDE	0x407e
#define CORE_CFG_LED4_OVERRIDE	0x407d
#define CORE_CFG_LED5_OVERRIDE	0x407c

/* LED3 is used to signal a magic packet detection event */
static int _phy_wol_enable_mpd(phy_dev_t *phy_dev, wol_params_t *wol_params)
{
	int ret;

	/* Enable MPD in PHY, apply mask to DA only */
	PHY_WRITE(phy_dev, 0x01, WOL_CTRL, 0x8401);

	/* Configure MPD PKT1: destination address */
	PHY_WRITE(phy_dev, 0x01, WOL_PKT1_15_0, (wol_params->mac_addr[4] << 8) | wol_params->mac_addr[5]);
	PHY_WRITE(phy_dev, 0x01, WOL_PKT1_31_16, (wol_params->mac_addr[2] << 8) | wol_params->mac_addr[3]);
	PHY_WRITE(phy_dev, 0x01, WOL_PKT1_47_32, (wol_params->mac_addr[0] << 8) | wol_params->mac_addr[1]);

	/* Configure MPD PKT2 */
	PHY_WRITE(phy_dev, 0x01, WOL_PKT2_15_0, 0xffff);
	PHY_WRITE(phy_dev, 0x01, WOL_PKT2_31_16, 0xffff);
	PHY_WRITE(phy_dev, 0x01, WOL_PKT2_47_32, 0xffff);

	/* LED configuration */
	PHY_WRITE(phy_dev, 0x01, CTL_LED3_MASK_LOW, 0x0100); /* Set WoL signal as LED source (bit 8) */
	PHY_WRITE(phy_dev, 0x01, CTL_LED_CONTROL, 0x80c0); /* Set LED3_CTL to Open-drain */
	PHY_WRITE(phy_dev, 0x01, CTL_LED_CONTROL_1, 0x001f); /* Set LED_STRETCH_EN to 0 */
	PHY_WRITE(phy_dev, 0x01, CORE_CFG_LED_CONTROL, 0x00c0); /* Set LED output enable */
	PHY_WRITE(phy_dev, 0x01, CORE_CFG_HP_LED_CTL, 0x0000);
	PHY_WRITE(phy_dev, 0x01, CORE_CFG_LED3_OVERRIDE, 0x0000);

	/* Enable MPD interrupt mask */
	PHY_WRITE(phy_dev, 0x01, WOL_INTR_MASK, 0);

Exit:
	return ret;
}

/* LED4 is used to siganl a link up event */
static int _phy_wol_enable_link(phy_dev_t *phy_dev, wol_params_t *wol_params)
{
	int ret;

	/* LED configuration */
	PHY_WRITE(phy_dev, 0x01, CTL_LED4_MASK_LOW, 0x0040); /* Set link up signal as LED source (bit 6) */
	PHY_WRITE(phy_dev, 0x01, CTL_LED_CONTROL, 0x8400); /* Set LED4_CTL to ON based on source */
	PHY_WRITE(phy_dev, 0x01, CTL_LED_CONTROL_1, 0x001f); /* Set LED_STRETCH_EN to 0 */
	PHY_WRITE(phy_dev, 0x01, CORE_CFG_LED_CONTROL, 0x00c0); /* Set LED output enable */
	PHY_WRITE(phy_dev, 0x01, CORE_CFG_HP_LED_CTL, 0x0000);
	PHY_WRITE(phy_dev, 0x01, CORE_CFG_LED4_OVERRIDE, 0x0000);

Exit:
	return ret;
}

static int _phy_wol_enable(phy_dev_t *phy_dev, wol_params_t *wol_params)
{
	u16 data = 1;
	int ret;

	/* Clear all LEDs sources */
	PHY_WRITE(phy_dev, 0x01, CTL_LED1_MASK_LOW, 0x0000);
	PHY_WRITE(phy_dev, 0x01, CTL_LED1_MASK_HIGH, 0x0000);
	PHY_WRITE(phy_dev, 0x01, CTL_LED1_MASK_EXT, 0x0000);

	PHY_WRITE(phy_dev, 0x01, CTL_LED2_MASK_LOW, 0x0000);
	PHY_WRITE(phy_dev, 0x01, CTL_LED2_MASK_HIGH, 0x0000);
	PHY_WRITE(phy_dev, 0x01, CTL_LED2_MASK_EXT, 0x0000);

	PHY_WRITE(phy_dev, 0x01, CTL_LED3_MASK_LOW, 0x0000);
	PHY_WRITE(phy_dev, 0x01, CTL_LED3_MASK_HIGH, 0x0000);
	PHY_WRITE(phy_dev, 0x01, CTL_LED3_MASK_EXT, 0x0000);

	PHY_WRITE(phy_dev, 0x01, CTL_LED4_MASK_LOW, 0x0000);
	PHY_WRITE(phy_dev, 0x01, CTL_LED4_MASK_HIGH, 0x0000);
	PHY_WRITE(phy_dev, 0x01, CTL_LED4_MASK_EXT, 0x0000);

	PHY_WRITE(phy_dev, 0x01, CTL_LED5_MASK_LOW, 0x0000);
	PHY_WRITE(phy_dev, 0x01, CTL_LED5_MASK_HIGH, 0x0000);
	PHY_WRITE(phy_dev, 0x01, CTL_LED5_MASK_EXT, 0x0000);

	if (wol_params->en_mpd)
		ret = _phy_wol_enable_mpd(phy_dev, wol_params);
	else if (wol_params->en_lnk)
		ret = _phy_wol_enable_link(phy_dev, wol_params);
	else
		ret = -1;

	if (ret)
		goto Exit;

	/* Enable Wake-on-LAN mode */
	ret = cmd_handler(phy_dev, CMD_SET_WOL_ENABLE, &data, NULL, NULL, NULL, NULL);
	if (ret)
		goto Exit;

	/* SerDes programming */
	PHY_WRITE(phy_dev, 0x1e, 0x4110, 0x2004);
	PHY_WRITE(phy_dev, 0x01, 0xd0c1, 0x0188);
	PHY_WRITE(phy_dev, 0x1e, 0x4110, 0x0001);

	PHY_WRITE(phy_dev, 0x1e, 0x4110, 0x2004);
	PHY_WRITE(phy_dev, 0x03, 0x9109, 0x0010);
	PHY_WRITE(phy_dev, 0x1e, 0x4110, 0x0001);

Exit:
	return ret;
}

static int _phy_txfir_cfg_get(phy_dev_t *phy_dev, int8_t *pre, int8_t *main, int8_t *post1, int8_t *post2, int8_t *hpf)
{
	int ret;
	u16 data1, data2, data3, data4, data5;
	phy_dev_t _phy_dev = *phy_dev;

	if (phy_dev_is_mphy(phy_dev))
		_phy_dev.addr = get_usxgmii_m_aggregated_port(phy_dev);

	ret = cmd_handler(&_phy_dev, CMD_GET_XFI_TX_FILTERS, &data1, &data2, &data3, &data4, &data5);
	if (ret)
		goto Exit;

	*pre = data2 & 0xff;
	*main = (data2 >> 8) & 0xff;
	*post1 = data3 & 0xff;
	*post2 = (data3 >> 8) & 0xff;
	*hpf = (data4 >> 8) & 0xff;

Exit:
	return ret;
}

static int _phy_txfir_cfg_set(phy_dev_t *phy_dev, int8_t pre, int8_t main, int8_t post1, int8_t post2, int8_t hpf)
{
	int ret;
	u16 data1, data2, data3, data4, data5;
	phy_dev_t _phy_dev = *phy_dev;

	if (phy_dev_is_mphy(phy_dev))
		_phy_dev.addr = get_usxgmii_m_aggregated_port(phy_dev);

	data1 = 0;
	data2 = pre | main << 8;
	data3 = post1 | post2 << 8;
	data4 = hpf << 8;
	data5 = 0;

	ret = cmd_handler(&_phy_dev, CMD_SET_XFI_TX_FILTERS, &data1, &data2, &data3, &data4, &data5);
	if (ret)
		goto Exit;

Exit:
	return ret;
}

static int inter_phy_type_usxgmii_m_set(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, int type)
{
	int rc = 0, i;
	u16 data1, data2, data3, data4, data5;
	int total_ports, base_port;
	int addr = phy_dev->addr;
	int ret = 0;

	total_ports = usxgmii_m_total_ports(phy_dev->usxgmii_m_type);
	base_port = phy_dev->addr - get_usxgmii_m_core_id(phy_dev->addr);

	for (i = base_port; i < base_port + total_ports; i++) {
		phy_dev->addr = i;
		ret += phy_dev_super_isolate_phy(phy_dev, 1);
	}

	data1 = type == INTER_PHY_TYPE_USXGMII; /* Enable/Disable bit */
	data2 = 0x11;	/* AN, only on */
	switch (phy_dev->usxgmii_m_type) {
	case USXGMII_M_10G_Q:
	case USXGMII_M_10G_D:
	case USXGMII_M_10G_S:
#ifdef CONFIG_BCM_PHY_KAUAI_A0
		if (_phy_use_firmware(phy_dev, &kauai_a0))
			data3 = 0x100;
		else
#endif
			data3 = 0x124;
		data4 = 0; /* 0 = Broadcom mode */
		data5 = 0; /* 0 = MAC/PHY frequency is locked without PPM offset */
		break;
	default:
		printk("No supported USXGMII mode: %d\n", phy_dev->usxgmii_m_type);
		return -1;
	}

	phy_usxgmii_data5_setting(phy_dev, &data5);
	for (i = base_port; i < base_port + total_ports; i++) {
		phy_dev->addr = i;
		rc += cmd_handler(phy_dev, CMD_SET_USXGMII, &data1, &data2, &data3, &data4, &data5);
		phy_dev_xfi_tx_polarity_set(phy_dev, phy_dev->xfi_tx_polarity_inverse);
		phy_dev_xfi_rx_polarity_set(phy_dev, phy_dev->xfi_rx_polarity_inverse);
	}

	for (i = base_port; i < base_port + total_ports; i++) {
		phy_dev->addr = i;
		ret += phy_dev_super_isolate_phy(phy_dev, 0);
	}
	phy_dev->addr = addr;
	return 0;
}

static int inter_phy_type_usxgmii_set(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, int type)
{
	int rc = 0;
	u16 data1, data2, data3, data4, data5;

	data1 = data2 = data3 = data4 = data5 = 0;
	if ((1 << type) & (INTER_PHY_TYPE_USXGMII_M | INTER_PHY_TYPE_SXGMII_M | INTER_PHY_TYPE_USXGMII_MP_M)) {
		data1 = 1;
		switch (type) {
		case INTER_PHY_TYPE_USXGMII:
			data2 = 1;	/* AN, only on */
			data3 = 4;
			break;

		case INTER_PHY_TYPE_SXGMII:
			data2 = 0x11;
			data3 = 1;
			break;

		case INTER_PHY_TYPE_USXGMII_MP:
			switch (phy_dev->usxgmii_m_type) {
				/* data sheet values */
				/* PLP script values */
			case USXGMII_M_10G_Q:
			case USXGMII_M_10G_D:
			case USXGMII_M_10G_S:
				return inter_phy_type_usxgmii_m_set(phy_dev, if_dir, type);
			default:
				printk("No supported USXGMII mode: %d\n", phy_dev->usxgmii_m_type);
				return -1;
			}
			break;
		}
	}

	phy_usxgmii_data5_setting(phy_dev, &data5);
	rc += cmd_handler(phy_dev, CMD_SET_USXGMII, &data1, &data2, &data3, &data4, &data5);

	return rc;
}

static int _phy_inter_phy_types_get(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, u32 *inter_phy_types)
{
	u16 phyid1, phyid2;
	int j, ret;

	*inter_phy_types = 0;
	PHY_READ(phy_dev, 0x01, 0x0002, &phyid1);
	PHY_READ(phy_dev, 0x01, 0x0003, &phyid2);

	for (j = 0; j < (int)ARRAY_SIZE(phy_desc); j++) {
		if (phy_desc[j].phyid1 != phyid1 || phy_desc[j].phyid2 != phyid2)
			continue;

		*inter_phy_types = phy_desc[j].inter_phy_types;
		if (*inter_phy_types == INTER_PHY_TYPE_UNKNOWN)
			*inter_phy_types = INTER_PHY_TYPE_UNKNOWN_M;
		phy_dev->inter_phy_types = *inter_phy_types;
		return 0;
	}

	/* Should not happen */
	printk("No EXT3 device found for %d\n", phy_dev->addr);
Exit:
	return -1;
}

static int inter_phy_type_2p5g5g_set(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, int type)
{
	int rc = 0;
	u16 data1, data2, data3, data4, data5;

	rc = cmd_handler(phy_dev, CMD_GET_XFI_2P5G_5G_MODE, &data1, &data2, NULL, NULL, NULL);

	switch (type) {
	case INTER_PHY_TYPE_2P5GIDLE:
		data1 = XFI_MODE_IDLE_STUFFING;
		break;
	case INTER_PHY_TYPE_2P5GBASE_X:
		data1 = XFI_MODE_BASE_X;
		break;
	case INTER_PHY_TYPE_2P5GBASE_R:
		data1 = XFI_MODE_BASE_R;
		break;
	case INTER_PHY_TYPE_5GIDLE:
		data2 = XFI_MODE_IDLE_STUFFING;
		break;
	case INTER_PHY_TYPE_5000BASE_X:
		data2 = XFI_MODE_BASE_X;
		break;
	case INTER_PHY_TYPE_5GBASE_R:
		data2 = XFI_MODE_BASE_R;
		break;
	default:
		/* do nothing */
		break;
	}
	data3 = data4 = data5 = 0;

#if defined(CONFIG_BCM_PHY_LANAI_A0)
	/* Disable SGMII_AN for LANAI family */
	if (_phy_use_firmware(phy_dev, &lanai_a0) || _phy_use_firmware(phy_dev, &lanai_a0_m))
		data2 = 0;
#endif

#if defined(CONFIG_BCM_PHY_NIIHAU_A0)
	/* Disable SGMII_AN for NIIHAU family */
	if (_phy_use_firmware(phy_dev, &niihau_a0) || _phy_use_firmware(phy_dev, &niihau_a0_m))
		data2 = 0;
#endif

	rc += cmd_handler(phy_dev, CMD_SET_XFI_2P5G_5G_MODE, &data1, &data2, &data3, &data3, &data5);

	return rc;
}

/* Setting PHY with multiple XFI modes, HW has capability to do so with internal priority */
static int _phy_configured_inter_phy_types_set(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, u32 types)
{
	int rc = 0;
	int best_type;

	phy_dev_configured_current_inter_phy_type_set(phy_dev, INTER_PHY_TYPE_AUTO);
	if (phy_dev->link == 0) {
		best_type = phy_get_best_inter_phy_configure_type(phy_dev, types, PHY_SPEED_2500);
		/* USXGMII family as an AN protocol needs to be set before link up, or XFI will not link up */
		if (best_type == INTER_PHY_TYPE_USXGMII || best_type == INTER_PHY_TYPE_USXGMII_MP || best_type == INTER_PHY_TYPE_SXGMII)
			goto end;

		/* 2.5G */
		if (best_type != INTER_PHY_TYPE_UNKNOWN)
			rc += inter_phy_type_2p5g5g_set(phy_dev, if_dir, best_type);

		best_type = phy_get_best_inter_phy_configure_type(phy_dev, types, PHY_SPEED_5000);
		if (best_type != INTER_PHY_TYPE_UNKNOWN)
			rc += inter_phy_type_2p5g5g_set(phy_dev, if_dir, best_type);
	} else {
		best_type = phy_get_best_inter_phy_configure_type(phy_dev, types, phy_dev->speed);
		if (best_type == phy_dev->current_inter_phy_type)
			return 0;

		if (best_type != INTER_PHY_TYPE_USXGMII && best_type != INTER_PHY_TYPE_USXGMII_MP && best_type != INTER_PHY_TYPE_SXGMII)
			rc += inter_phy_type_2p5g5g_set(phy_dev, if_dir, best_type);
	}
end:
	if (best_type != phy_dev->current_inter_phy_type) {
		phy_dev_current_inter_phy_types_set(phy_dev, INTER_PHY_TYPE_UP, best_type);
		rc += inter_phy_type_usxgmii_set(phy_dev, if_dir, best_type);
	}
	return rc;
}

/* Setting PHY with exlusive XFI single mode for manual configuration */
static int _phy_configured_speed_type(phy_dev_t *phy_dev, int adv_phy_caps, phy_duplex_t duplex,
	inter_phy_types_dir_t if_dir, int type, int cfg_an_enable)
{
	int rc = 0;
	int type_adv_caps;

	/* Override AN flag if it is not auto */
	switch (cfg_an_enable) {
	case PHY_CFG_AN_AUTO:
		break; /* Do not change AN */
	case PHY_CFG_AN_OFF:
		adv_phy_caps &= ~(PHY_CAP_AUTONEG);
		phy_dev->an_enabled = 0;
		break;
	case PHY_CFG_AN_ON:
		adv_phy_caps |= PHY_CAP_AUTONEG;
		phy_dev->an_enabled = 1;
		break;
	}

	if (type != INTER_PHY_TYPE_AUTO && !((1 << type) & phy_dev->common_inter_phy_types))
		return -1;

	if (type == INTER_PHY_TYPE_AUTO && adv_phy_caps == 0)
		return -1;

	if (type == INTER_PHY_TYPE_AUTO) {
		rc += phy_dev_configured_inter_phy_types_set(phy_dev, if_dir, phy_dev->common_inter_phy_types);
		rc += phy_dev_caps_set(phy_dev, adv_phy_caps);
	} else {
		rc += inter_phy_type_usxgmii_set(phy_dev, if_dir, type);
		rc += inter_phy_type_2p5g5g_set(phy_dev, if_dir, type);

		get_inter_phy_supported_speed_caps((1 << type), &type_adv_caps);
		if (adv_phy_caps == PHY_CAP_AUTONEG)
			adv_phy_caps = type_adv_caps;
		else
			adv_phy_caps &= type_adv_caps;
		rc += phy_dev_caps_set(phy_dev, adv_phy_caps);
	}

	phy_dev_configured_current_inter_phy_type_set(phy_dev, type);
	phy_dev_current_inter_phy_types_set(phy_dev, INTER_PHY_TYPE_UP, type);

	return rc;
}

static int inter_phy_type_usxgmii_get(phy_dev_t *phy_dev)
{
	u16 data1, data2, data3;

	cmd_handler(phy_dev, CMD_GET_USXGMII, &data1, &data2, &data3, NULL, NULL);

	return data1 > 0;
}

static int inter_type_max_speed(phy_dev_t *phy_dev)
{
	u32 caps;

	phy_dev_caps_get(phy_dev, CAPS_TYPE_SUPPORTED, &caps);
	return phy_caps_to_max_speed(caps);
}

static int inter_phy_current_types_2p5g_5g_get(phy_dev_t *phy_dev, u32 *types)
{
	int rc = 0;
	u16 data1, data2;

	*types = 0;
	rc = cmd_handler(phy_dev, CMD_GET_XFI_2P5G_5G_MODE, &data1, &data2, NULL, NULL, NULL);

	switch (data1) {
	case 0:
		*types |= INTER_PHY_TYPE_2P5GIDLE_M;
		break;
	case 1:
		*types |= INTER_PHY_TYPE_2P5GBASE_X_M;
		break;
	case 2:
		*types |= INTER_PHY_TYPE_2P5GBASE_R_M;
		break;
	}

	switch (data2) {
	case 0:
		*types |= INTER_PHY_TYPE_5GIDLE_M;
		break;
	case 1:
		*types |= INTER_PHY_TYPE_5000BASE_X_M;
		break;
	case 2:
		*types |= INTER_PHY_TYPE_5GBASE_R_M;
		break;
	}

	return 0;
}

static int _phy_current_inter_phy_type_get(phy_dev_t *phy_dev)
{
	u32 sw_types, hw_types;
	u32 types;
	int config_current_type;

	phy_dev_inter_phy_types_get(phy_dev, INTER_PHY_TYPE_UP, &sw_types);
	if (sw_types == INTER_PHY_TYPE_UNKNOWN_M)
		return INTER_PHY_TYPE_UNKNOWN_M;

	config_current_type = phy_dev_configured_current_inter_phy_type_get(phy_dev);
	if (config_current_type != INTER_PHY_TYPE_AUTO)
		return config_current_type;

	phy_dev_configured_inter_phy_types_get(phy_dev, &sw_types);

	if (inter_phy_type_usxgmii_get(phy_dev)) {
		phy_dev_inter_phy_types_get(phy_dev, INTER_PHY_TYPE_UP, &types);
		if (types & INTER_PHY_TYPE_USXGMII_MP_M)
			return INTER_PHY_TYPE_USXGMII_MP;
		else if (types & INTER_PHY_TYPE_SXGMII_M)
			return INTER_PHY_TYPE_SXGMII;
		else
			return INTER_PHY_TYPE_USXGMII;
	}

	if (inter_type_max_speed(phy_dev) <= PHY_SPEED_1000 &&
		sw_types & INTER_PHY_TYPE_SGMII_M)
		return INTER_PHY_TYPE_SGMII;

	if (!phy_dev->link)
		return INTER_PHY_TYPE_UNKNOWN;

	inter_phy_current_types_2p5g_5g_get(phy_dev, &hw_types);

	switch (phy_dev->speed) {
	case PHY_SPEED_10:
	case PHY_SPEED_100:
	case PHY_SPEED_1000:
		if (sw_types & INTER_PHY_TYPE_SGMII_M)
			return INTER_PHY_TYPE_SGMII;
		return INTER_PHY_TYPE_UNKNOWN;	/* Bug */

	case PHY_SPEED_2500:
		if (hw_types & INTER_PHY_TYPE_2P5GBASE_R_M)
			return INTER_PHY_TYPE_2P5GBASE_R;
		if (hw_types & INTER_PHY_TYPE_2P5GBASE_X_M)
			return INTER_PHY_TYPE_2P5GBASE_X;
		if (hw_types & INTER_PHY_TYPE_2P5GIDLE_M)
			return INTER_PHY_TYPE_2P5GIDLE;
		return INTER_PHY_TYPE_UNKNOWN;	/* Bug */

	case PHY_SPEED_5000:
		if (hw_types & INTER_PHY_TYPE_5GBASE_R_M)
			return INTER_PHY_TYPE_5GBASE_R;
		if (hw_types & INTER_PHY_TYPE_5000BASE_X_M)
			return INTER_PHY_TYPE_5000BASE_X;
		if (hw_types & INTER_PHY_TYPE_5GIDLE_M)
			return INTER_PHY_TYPE_5GIDLE;
		return INTER_PHY_TYPE_UNKNOWN;	/* Bug */

	case PHY_SPEED_10000:
		return INTER_PHY_TYPE_10GBASE_R;

	default:
		break;
	}

	return INTER_PHY_TYPE_UNKNOWN;	/* Bug */
}

phy_drv_t phy_drv_ext3 = {

	.phy_type = PHY_TYPE_EXT3,
	.name = "EXT3",
	.read = _phy_read,
	.write = _phy_write,
	.power_get = _phy_power_get,
	.power_set = _phy_power_set,
	.apd_get = _phy_apd_get,
	.apd_set = _phy_apd_set,
	.eee_get = _phy_eee_get,
	.eee_set = _phy_eee_set,
	.eee_mode_set = phy_dev_eee_mode_set,
	.eee_mode_get = _phy_eee_autogreeen_read,
	.eee_resolution_get = _phy_eee_resolution_get,
	.read_status = _phy_read_status,
	.speed_set = _phy_speed_set,
	.caps_get = _phy_caps_get,
	.caps_set = _phy_caps_set,
	.phyid_get = _phy_phyid_get,
	.mdix_mode_set = _phy_mdix_mode_set,
	.mdix_mode_get = _phy_mdix_mode_get,
	.mdix_status_get = _phy_mdix_status_get,
	.wirespeed_set = _phy_eth_wirespeed_set,
	.wirespeed_get = _phy_eth_wirespeed_get,
	.init = _phy_init,
	.dev_add = _phy_dev_add,
	.dev_del = _phy_dev_del,
	.drv_init = _phy_drv_init,
	.pair_swap_set = _phy_pair_swap_set,
	.isolate_phy = _phy_isolate,
	.super_isolate_phy = _phy_super_isolate,
	.get_phy_name = _phy_get_phy_name,
	.cable_diag_run = _phy_enhanced_cable_diag_run,
	.leds_init = _phy_leds_init,
	.diag = _phy_diag,
	.an_restart = _phy_an_restart,
	.wol_enable = _phy_wol_enable,
	.txfir_cfg_get = _phy_txfir_cfg_get,
	.txfir_cfg_set = _phy_txfir_cfg_set,
	.xfi_tx_polarity_inverse_set = _phy_xfi_tx_polarity_set,
	.xfi_rx_polarity_inverse_set = _phy_xfi_rx_polarity_set,
	.shared_clock_set = _phy_shared_clock_set,
	.tx_pwr_get = _phy_tx_pwr_get,
	.tx_pwr_set = _phy_tx_pwr_set,
#ifdef MACSEC_SUPPORT
	.macsec_oper = phy_macsec_oper,
#endif
	.inter_phy_types_get = _phy_inter_phy_types_get,
	.configured_inter_phy_types_set = _phy_configured_inter_phy_types_set,
	.configured_inter_phy_speed_type_set = _phy_configured_speed_type,
	.current_inter_phy_type_get = _phy_current_inter_phy_type_get,
};
