/*
 * Configuration-related definitions for
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlc_cfg.h 732650 2017-11-20 10:14:47Z $
 */

#ifndef _wlc_cfg_h_
#define _wlc_cfg_h_

#define NEW_TXQ

#if defined(__NetBSD__) || defined(__FreeBSD__)
#include <opt_bcm.h>
#include <opt_bwl.h>
#endif /* defined(__NetBSD__) || defined(__FreeBSD__) */

/**************************************************
 * Get customized tunables to override the default*
 * ************************************************
 */
#ifndef LINUX_HYBRID
#include "wlconf.h"
#endif // endif

/***********************************************
 * Feature-related macros to optimize out code *
 * *********************************************
 */

/* WL_ENAB_RUNTIME_CHECK may be set based upon the #define below (for ROM builds). It may also
 * be defined via makefiles (e.g. ROM auto abandon unoptimized compiles).
 */
#if defined(BCMROMBUILD)
	#ifndef WL_ENAB_RUNTIME_CHECK
		#define WL_ENAB_RUNTIME_CHECK
	#endif
#endif /* BCMROMBUILD */

#if (!defined(PHYCAL_CACHING) && !defined(BCMROMBUILD))
#if (defined(SRHWVSDB) && !defined(SRHWVSDB_DISABLED)) || defined(WLAWDL) || \
	(defined(WLMCHAN) && !defined(WLMCHAN_DISABLED)) || defined(BCMPHYCAL_CACHING)
#define PHYCAL_CACHING
#endif // endif
#endif /* !PHYCAL_CACHING && !BCMROMBUILD */

#if defined(WLOFFLD) && defined(MBSS)
#error "WLOFFLD is not compatible with MBSS"
#endif // endif

#if defined(NLO) && !defined(WLPFN)
#error "NLO needs WLPFN to be defined."
#endif // endif

/* DUALBAND Support */
#ifdef DBAND
#define NBANDS(wlc) ((wlc)->pub->_nbands)
#define NBANDS_PUB(pub) ((pub)->_nbands)
#define NBANDS_HW(hw) ((hw)->_nbands)
#else
#define NBANDS(wlc) 1
#define NBANDS_PUB(wlc) 1
#define NBANDS_HW(hw) 1
#endif /* DBAND */

#define _IS_SINGLEBAND_5G(device) \
	((device == BCM4306_D11A_ID) || \
	 (device == BCM4311_D11A_ID) || \
	 (device == BCM4318_D11A_ID) || \
	 (device == BCM4321_D11N5G_ID) || \
	 (device == BCM4328_D11A_ID) || \
	 (device == BCM4325_D11A_ID) || \
	 (device == BCM4322_D11N5G_ID) || \
	 (device == BCM43222_D11N5G_ID) || \
	 (device == BCM43228_D11N5G_ID) || \
	 (device == BCM43236_D11N5G_ID) || \
	 (device == BCM4331_D11N5G_ID) || \
	 (device == BCM4360_D11AC5G_ID) || \
	 (device == BCM4335_D11AC5G_ID) || \
	 (device == BCM4345_D11AC5G_ID) || \
	 (device == BCM4352_D11AC5G_ID) || \
	 (device == BCM4350_D11AC5G_ID) || \
	 (device == BCM43556_D11AC5G_ID) || \
	 (device == BCM43558_D11AC5G_ID) || \
	 (device == BCM6362_D11N5G_ID) || \
	 (device == BCM4345_D11AC5G_ID) || \
	 (device == BCM43602_D11AC5G_ID) || \
	 (device == BCM4365_D11AC5G_ID) || \
	 (device == BCM4366_D11AC5G_ID) || \
	0)

int wlc_is_singleband_5g(unsigned int device);
int wlc_bmac_is_singleband_5g(unsigned int device);
#if defined(WLC_HIGH)
	#define IS_SINGLEBAND_5G(device)	(bool)wlc_is_singleband_5g(device)
#elif defined(WLC_LOW)
	#define IS_SINGLEBAND_5G(device)	(bool)wlc_bmac_is_singleband_5g(device)
#endif // endif

	#define WLNINTENDO_ENAB(wlc)	0
	#define WLNITRO_ENAB(wlc)	0

#if defined(WLC_HIGH) && !defined(WLC_LOW)
#define WLC_HIGH_ONLY
#endif // endif

#if !defined(WLC_HIGH) && defined(WLC_LOW)
#define WLC_LOW_ONLY
#endif // endif

#if !defined(WLC_HIGH) || !defined(WLC_LOW)
#define WLC_SPLIT
#endif // endif

#ifndef WLRXEXTHDROOM
#if defined(WLC_LOW_ONLY)
#define WLRXEXTHDROOM 32 /* RPC header */
#elif defined(DONGLEBUILD)
#define WLRXEXTHDROOM -1 /* keep -1 default BCMEXTRAHDROOM for full-dongle */
#else
#define WLRXEXTHDROOM 0
#endif // endif
#endif /* WLRXEXTHDROOM */

/* **** Core type/rev defaults **** */
#define D11_DEFAULT	0xffffffb0	/* Supported  D11 revs: 4, 5, 7-31 */
#ifdef UNRELEASEDCHIP
#define D11_DEFAULT2	0x007fff87	/* Supported  D11 revs: 32-34, 39, 40-54 */
#define D11_DEFAULT3	0x00000003	/* Supported  D11 revs: 64-65 */
#else
#define D11_DEFAULT2	0x000bff07	/* Supported  D11 revs: 32-34, 40-49, 51 */
#define D11_DEFAULT3	0x00000003	/* Supported  D11 revs: 64-65 */
#endif  /* UNRELEASEDCHIP */

/*
 * The supported PHYs are either specified explicitly in the wltunable_xxx.h file, or the
 * defaults are used.
 * If PHYCONF_DEFAULTS is set to 1, then all defaults will be used for PHYs that are not predefined.
 * If PHYCONF_DEFAULTS is set to 0 (or non-existent), then all PHYs that are not predefined will
 * be disabled.
 */
#ifndef PHYCONF_DEFAULTS	/* possibly defined in wltunable_xxx.h, to use defaults */
#  if (defined(ACCONF) || defined(ACCONF2) || defined(LCN40CONF) || defined(LCNCONF) || \
	defined(SSLPNCONF) || defined(LPCONF) || defined(NCONF) || defined(HTCONF) || \
	defined(ACONF) || defined(GCONF) || defined(LCN20CONF))
	/* do not use default phy configs since specific phy support is specified */
#    define PHYCONF_DEFAULTS	0
#  else
	/* use default phy configs since nothing was specified */
#    define PHYCONF_DEFAULTS	1
#  endif /* ACCONF || ACCONF2 || LCN40CONF || LCNCONF || SSLPNCONF || LPCONF ||
	 ** NCONF || HTCONF || ACONF || GCONF || LCN20CONF
	 */
#endif /* !PHYCONF_DEFAULTS */

#define APHY_DEFAULT	0x000001ec	/* Supported aphy revs:
					 *	2	4306b0
					 *	3	4306c0, 4712a0/a1/a2/b0
					 *	5	4320a2
					 *	6	4318b0
					 *	7	5352a0, 4311a0
					 *	8	4311b0
					 */
#define GPHY_DEFAULT	0x000003c6	/* Supported gphy revs:
					 *	1	4306b0
					 *	2	4306c0, 4712a0/a1/a2/b0
					 *	6	4320a2
					 *	7	4318b0, 5352a0
					 *	8	4311a0
					 *	9	4311b0
					 */
#define NPHY_DEFAULT	0x005f07ff	/* Supported nphy revs:
					 *	0	4321a0
					 *	1	4321a1
					 *	2	4321b0/b1/c0/c1
					 *	3	4322a0
					 *	4	4322a1
					 *	5	4716a0
					 *	6	43222a0, 43224a0
					 *	7	43226a0
					 *	8	5357a0, 43236a0
					 *	9	5357b0, 43236b0
					 *      10      43237a0
					 *	16	43228a0
					 *	17	53572a0
					 *	18	43239a0
					 *	19	4324a0
					 *	20	4324b0
					 *	22	43242a1
					 */
#define HTPHY_DEFAULT	0x00000003	/* Supported htphy revs:
					 *	0	4331a0
					 *	1	4331a1
					 */
#define LPPHY_DEFAULT	0x0000001f	/* Supported lpphy revs:
					 *	0	4328a0, 5354a0, 5354a1
					 *	1	4312a0/a1
					 *	2	4325a0
					 *	3	4325b0
					 *	4	4315a0
					 */

#define SSLPNPHY_DEFAULT 0x0000000f	/* Supported sslpnphy revs:
					 *	0	4329a0/k0
					 *	1	4329b0/4329C0
					 *	2	4319a0
					 *	3	5356a0
					 */

#define LCNPHY_DEFAULT	0x0000000f	/* Supported lcnphy revs:
					 *	0	4313a0, 4336a0, 4330a0
					 *	1
					 *	2 	4330a0
					 *	3 	4330b0
					 */

#define LCN40PHY_DEFAULT	0x000000ff	/* Supported lcn40phy revs:
						 *	0	4334a0
						 *	1	4314a0
						 *
						 *	3	4324a0
						 *
						 *
						 *	6	43143a0
						 *	7	43341a0
						 */

#define LCN20PHY_DEFAULT	0x00000001	/* Supported lcn20phy revs:
						 *	0	43430a0
						 */

#ifdef UNRELEASEDCHIP

#define ACPHY_DEFAULT		0x0f17fbff      /* Supported acphy revs:
						 *
						 *
						 *
						 *
						 *
						 *
						 *
						 *	7	4345a0
						 *	8	4350c0
						 *	9	43602a0
						 *
						 *	11	43457a0
						 *	12	4349a0
						 *	13	4345b0/b1
						 *	14	4350c2
						 *	16	43909a0
						 *	17	4354a2
						 *	18  43602a1
						 *	20  4345c0
						 *    24   4349b0
						 *    25   4347TC2a0
						 *    27	4364a0
						 */
#define ACPHY_DEFAULT2		0x00000113	/*
						 *	32	4365a0/b0/b1
						 *      33      4365c0
						 *	36	4347a0 (1x1)
						 *	40	4347a0 (2x2)
						 */

#else

#define ACPHY_DEFAULT		0x106a9ff       /* Supported acphy revs:
						 *
						 *
						 *
						 *	7	4345a0
						 *	8	4350c0
						 *
						 *
						 *
						 *	12	4349a0
						 *	13	4345b0/b1
						 *	14	4350c2
						 *
						 *
						 *	17	4354a2
						 *	18      43602a1
						 *	24  4355b0
						 *      25      4347TC2a0
						 */
#define ACPHY_DEFAULT2		0x00000003	/*
						 *	32	4365a0/b0/b1
						 *      33      4365c0
						 */
#endif /* UNRELEASEDCHIP */

#define D11CONF1_BASE		0
#define D11CONF2_BASE		32
#define D11CONF3_BASE		64
#define D11CONF_WIDTH		32
#define D11CONF_CHK(val, base)	(((val) >= (base)) && ((val) < ((base) + D11CONF_WIDTH)))

/* Windows needs D11CONF_REV():
 *   "shift count negative or too big, undefined behavior"
 */
#define D11CONF_REV(val, base)	(D11CONF_CHK(val, base) ? ((val) - (base)) : (0))

/* We need similar macros for ACPHY */
#define ACCONF1_BASE		0
#define ACCONF2_BASE		32
#define ACCONF_WIDTH		32
#define ACCONF_CHK(val, base)	(((val) >= (base)) && ((val) < ((base) + ACCONF_WIDTH)))
#define ACCONF_REV(val, base)	(ACCONF_CHK(val, base) ? ((val) - (base)) : (0))

/* To avoid compile warnings, we cannot compare uint with 0, which is always true */
#define ACCONF_CHK0(val, base)	((val) < ((base) + ACCONF_WIDTH))
#define ACCONF_REV0(val, base)	(ACCONF_CHK0(val, base) ? ((val) - (base)) : (0))

/* For undefined values, use defaults */
#ifndef D11CONF
#define D11CONF	D11_DEFAULT
#endif // endif
#ifndef D11CONF2
#define D11CONF2 D11_DEFAULT2
#endif // endif
#ifndef D11CONF3
#define D11CONF3 D11_DEFAULT3
#endif // endif

#if PHYCONF_DEFAULTS
/* Use default configurations for phy configs that are not specified */
#  ifndef ACONF
#    define ACONF       APHY_DEFAULT
#  endif /* !ACONF */
#  ifndef GCONF
#    define GCONF	GPHY_DEFAULT
#  endif /* !GCONF */
#  ifndef NCONF
#    define NCONF	NPHY_DEFAULT
#  endif /* !NCONF */
#  ifndef LPCONF
#    define LPCONF	LPPHY_DEFAULT
#  endif /* !LPCONF */
#  ifndef SSLPNCONF
#    define SSLPNCONF	SSLPNPHY_DEFAULT
#  endif /* !SSLPNCONF */
#  ifndef LCNCONF
#    define LCNCONF	LCNPHY_DEFAULT
#  endif /* !LCNCONF */
#  ifndef LCN40CONF
#    define LCN40CONF	LCN40PHY_DEFAULT
#  endif /* !LCN40CONF */
#  ifndef LCN20CONF
#    define LCN20CONF	LCN20PHY_DEFAULT
#  endif /* !LCN20CONF */
#  ifndef HTCONF
#    define HTCONF	HTPHY_DEFAULT
#  endif /* !HTCONF */
#  ifndef ACCONF
#    define ACCONF	ACPHY_DEFAULT
#  endif /* !ACCONF */
#  ifndef ACCONF2
#    define ACCONF2	ACPHY_DEFAULT2
#  endif /* !ACCONF2 */
#else /* PHYCONF_DEFAULTS == 0 */
/* Do not configure any phy support by default. Only specified phy configs will be non-zero. */
#  ifndef ACONF
#    define ACONF       0
#  endif /* !ACONF */
#  ifndef GCONF
#    define GCONF	0
#  endif /* !GCONF */
#  ifndef NCONF
#    define NCONF	0
#  endif /* !NCONF */
#  ifndef LPCONF
#    define LPCONF	0
#  endif /* !LPCONF */
#  ifndef SSLPNCONF
#    define SSLPNCONF	0
#  endif /* !SSLPNCONF */
#  ifndef LCNCONF
#    define LCNCONF	0
#  endif /* !LCNCONF */
#  ifndef LCN40CONF
#    define LCN40CONF	0
#  endif /* !LCN40CONF */
#  ifndef LCN20CONF
#    define LCN20CONF	0
#  endif /* !LCN20CONF */
#  ifndef HTCONF
#    define HTCONF	0
#  endif /* !HTCONF */
#  ifndef ACCONF
#    define ACCONF	0
#  endif /* !ACCONF */
#  ifndef ACCONF2
#    define ACCONF2	0
#  endif /* !ACCONF */
#endif /* PHYCONF_DEFAULTS */

/* support 2G band */
#if GCONF || NCONF || LPCONF || SSLPNCONF || LCNCONF || HTCONF || LCN40CONF || ACCONF \
	|| ACCONF2 || LCN20CONF

#define BAND2G
#endif // endif

/* support 5G band */
#if ACONF || defined(DBAND)
#define BAND5G
#endif // endif

#ifdef WL11N
#if NCONF || HTCONF || ACCONF || ACCONF2
#define WLANTSEL	1
#endif // endif
#if defined(WLAMSDU) && !defined(DONGLEBUILD)
#define WLAMSDU_TX      1
#endif // endif
#endif /* WL11N */

/***********************************
 * Some feature consistency checks *
 * *********************************
 */
#if ((defined(BCMSUP_PSK) || defined(BCMCCX)) && defined(LINUX_CRYPTO))
/* testing #error	"Only one supplicant can be defined; BCMSUP_PSK or LINUX_CRYPTO" */
#endif // endif

#if (defined(BCMSUP_PSK) || defined(BCMCCX)) && !defined(STA)
#error	"STA must be defined when BCMSUP_PSK and/or BCMCCX is defined"
#endif // endif

#if defined(BCMCCX) && !defined(WLRM)
#error	"WLRM must be defined when BCMCCX is defined"
#endif // endif

#if defined(CCX_SDK)
#if !defined(BCMCCX) || !defined(EXT_STA) || (!defined(NDIS60) && !defined(NDIS620) && \
	!defined(NDIS630) && !defined(NDIS640)) || !defined(BCMCCMP) || !defined(WLCNT)
#error "BCMCCX, EXT_STA, NDIS6x0, BCMCCMP and WLCNT must be defined for CCX_SDK"
#endif // endif
#endif /* CCX_SDK */

#if defined(BCMCCX) && !defined(WME)
#error	"BCMCCX should be used with WME"
#endif // endif

#if defined(WET) && !defined(STA)
#error	"STA must be defined when WET is defined"
#endif // endif

#if (!defined(AP) || !defined(STA)) && defined(APSTA)
#error "APSTA feature requested without both AP and STA defined"
#endif // endif

#if defined(WOWL) && !defined(STA)
#error "STA should be defined for WOWL"
#endif // endif

#if defined(EXT_STA) && !defined(WL_ASSOC_RECREATE)
#error "WL_ASSOC_RECREATE should be defined for EXT_STA"
#endif // endif

#if !defined(WLPLT)

#if defined(WLC_HIGH)
#if !defined(AP) && !defined(STA)
#error	"Neither AP nor STA were defined"
#endif // endif
#endif /* defined(WLC_HIGH) */
#if defined(BAND5G) && !defined(WL11H)
#error	"WL11H is required when 5G band support is configured"
#endif // endif

#endif /* !WLPLT */

#if !defined(WME) && defined(WLCAC)
#error	"WME support required"
#endif // endif

/* RXCHAIN_PWRSAVE/WL11N consistency check */
#if defined(RXCHAIN_PWRSAVE) && !defined(WL11N)
#error "WL11N should be defined for RXCHAIN_PWRSAVE"
#endif // endif

/* AP TPC and 11h consistency check */
#if defined(WL_AP_TPC) && !defined(WL11H)
#error "WL11H should be defined for WL_AP_TPC"
#endif // endif

/* AMPDU/WL11N consistency check */
#if defined(WLAMPDU) && !defined(WL11N)
#error "WL11N should be defined for WLAMPDU"
#endif // endif

/* AMSDU/WL11N consistency check */
#if defined(WLAMSDU) && !defined(WL11N)
#error "WL11N should be defined for WLAMSDU"
#endif // endif

/* WL11N/WL11AC consistency check */
#if defined(WL11AC) && !defined(WL11N)
#error "WL11N should be defined for WL11AC"
#endif // endif

#if defined(WLBTAMP) && defined(WLPKTDLYSTAT)
#error "BTAMP and Delay stats should not be defined simultaneously"
#endif // endif

#if defined(BCMWAPI_WAI) && defined(WL11K)
/* #error "BCMWAPI_WAI and WL11K can't be defined at the same time" */
#endif // endif

#if defined(DWDS) && !defined(WDS)
#error "WDS should be defined for DWDS"
#endif // endif
/* AMPDU Host reorder config checks */
#ifdef WLAMPDU_HOSTREORDER
#if !defined(BRCMAPIVTW)
#error "WLAMPDU_HOSTREORDER feature depends on BRCMAPIVTW feature"
#endif // endif
#if !defined(DONGLEBUILD)
#error "WLAMPDU_HOSTREORDER feature is a valid feature only for full dongle builds"
#endif // endif
#if !(defined(BCMPCIEDEV) || defined(PROP_TXSTATUS))
#error "WLAMPDU_HOSTREORDER requires PROP_TXSTATUS for non PCIE DEV builds"
#endif // endif
#if !defined(WLAMPDU_HOSTREORDER_DISABLED) && !(defined(PROP_TXSTATUS_ENABLED) || \
	defined(BCMPCIEDEV_ENABLED))
#error "WLAMPDU_HOSTREORDER requires PROP_TXSTATUS_ENABLED for non PCIE FULL Dongle "
#endif // endif
#endif /* WLAMPDU_HOSTREORDER */

#if defined(MBSS) && !defined(MBSS_DISABLED) && defined(WLP2P_UCODE_ONLY)
#error "MBSS requires non-P2P ucode"
#endif // endif

#ifdef WL_BCNTRIM
#if !(defined(WLC_HIGH) && defined(WLC_LOW))
#error "WL_BCNTRIM is only available to full dongle"
#endif // endif
#endif /* WL_BCNTRIM */

#if defined(BCM_DMA_CT) && !defined(BCM_DMA_INDIRECT)
#error "BCM_DMA_CT feature depends on BCM_DMA_INDIRECT"
#endif // endif

#if defined(WL_MU_TX) && !defined(BCM_DMA_INDIRECT)
#error "WL_MU_TX feature depends on BCM_DMA_INDIRECT"
#endif // endif

#if defined(BCM_DMA_CT) && defined(DMA_TX_FREE) && !defined(DMA_TX_FREE_DISABLED)
#error "DMA_TX_FREE must not be defined along with BCM_DMA_CT"
#endif // endif

#if defined(WLMCNX) && defined(DONGLEBUILD)
	#ifndef WLP2P_UCODE
		#error WLP2P_UCODE is required for MCNX on dongles
	#endif	/* !WLP2P_UCODE */
	#if !defined(WL_ENAB_RUNTIME_CHECK) && !defined(WLP2P_UCODE_ONLY) && \
	!defined(WLMCNX_DISABLED)
		#error WLP2P_UCODE_ONLY is required for MCNX on dongles
	#endif  /* !WLP2P_UCODE_ONLY */
#endif /* WLMCNX && DONGLEBUILD */

/* MBSS Utility Macros */
#if defined(MBSS)
/* Test if chip supports 4 or 16 MBSS */
#define D11REV_ISMBSS4(rev)  (D11REV_GE(rev, 9) && D11REV_LE(rev, 14) && !D11REV_IS(rev, 13))
#define D11REV_ISMBSS16(rev)  (D11REV_GE(rev, 13) && !D11REV_IS(rev, 14))
#else
#define D11REV_ISMBSS4(rev)    (0)
#define D11REV_ISMBSS16(rev)    (0)
#endif /* MBSS */

/********************************************************************
 * Phy/Core Configuration.  Defines macros to to check core phy/rev *
 * compile-time configuration.  Defines default core support.       *
 * ******************************************************************
 */

/* Basic macros to check a configuration bitmask */

#define IS_MULTI_REV(config)	((config) & ((config) - 1))	/* is more than one bit set? */

#define IS_MULTI_REV2(config1, config2)	(IS_MULTI_REV(config1) || IS_MULTI_REV(config2) || \
	((config1) && (config2)))

#define CONF_HAS(config, val)	((config) & (1U << (val)))
#define CONF_MSK(config, mask)	((config) & (mask))
#define MSK_RANGE(low, hi)	((1U << ((hi) + 1)) - (1U << (low)))
#define CONF_RANGE(config, low, hi) (CONF_MSK(config, MSK_RANGE(low, high)))

#define CONF_IS(config, val)	((config) == (uint)(1U << (val)))
#define CONF_GE(config, val)	((config) & (0 - (1U << (val))))
#define CONF_GT(config, val)	((config) & (0 - 2 * (1U << (val))))
#define CONF_LT(config, val)	((config) & ((1U << (val)) - 1))
#define CONF_LE(config, val)	((config) & (2 * (1U << (val)) - 1))

/* Wrappers for some of the above, specific to config constants */

#define ACONF_HAS(val)	CONF_HAS(ACONF, val)
#define ACONF_MSK(mask)	CONF_MSK(ACONF, mask)
#define ACONF_IS(val)	CONF_IS(ACONF, val)
#define ACONF_GE(val)	CONF_GE(ACONF, val)
#define ACONF_GT(val)	CONF_GT(ACONF, val)
#define ACONF_LT(val)	CONF_LT(ACONF, val)
#define ACONF_LE(val)	CONF_LE(ACONF, val)

#define GCONF_HAS(val)	CONF_HAS(GCONF, val)
#define GCONF_MSK(mask)	CONF_MSK(GCONF, mask)
#define GCONF_IS(val)	CONF_IS(GCONF, val)
#define GCONF_GE(val)	CONF_GE(GCONF, val)
#define GCONF_GT(val)	CONF_GT(GCONF, val)
#define GCONF_LT(val)	CONF_LT(GCONF, val)
#define GCONF_LE(val)	CONF_LE(GCONF, val)

#define NCONF_HAS(val)	CONF_HAS(NCONF, val)
#define NCONF_MSK(mask)	CONF_MSK(NCONF, mask)
#define NCONF_IS(val)	CONF_IS(NCONF, val)
#define NCONF_GE(val)	CONF_GE(NCONF, val)
#define NCONF_GT(val)	CONF_GT(NCONF, val)
#define NCONF_LT(val)	CONF_LT(NCONF, val)
#define NCONF_LE(val)	CONF_LE(NCONF, val)

#define LPCONF_HAS(val)	CONF_HAS(LPCONF, val)
#define LPCONF_MSK(mask) CONF_MSK(LPCONF, mask)
#define LPCONF_IS(val)	CONF_IS(LPCONF, val)
#define LPCONF_GE(val)	CONF_GE(LPCONF, val)
#define LPCONF_GT(val)	CONF_GT(LPCONF, val)
#define LPCONF_LT(val)	CONF_LT(LPCONF, val)
#define LPCONF_LE(val)	CONF_LE(LPCONF, val)

#define SSLPNCONF_HAS(val)	CONF_HAS(SSLPNCONF, val)
#define SSLPNCONF_MSK(mask)	CONF_MSK(SSLPNCONF, mask)
#define SSLPNCONF_IS(val)	CONF_IS(SSLPNCONF, val)
#define SSLPNCONF_GE(val)	CONF_GE(SSLPNCONF, val)
#define SSLPNCONF_GT(val)	CONF_GT(SSLPNCONF, val)
#define SSLPNCONF_LT(val)	CONF_LT(SSLPNCONF, val)
#define SSLPNCONF_LE(val)	CONF_LE(SSLPNCONF, val)

#define LCNCONF_HAS(val)	CONF_HAS(LCNCONF, val)
#define LCNCONF_MSK(mask)	CONF_MSK(LCNCONF, mask)
#define LCNCONF_IS(val)		CONF_IS(LCNCONF, val)
#define LCNCONF_GE(val)		CONF_GE(LCNCONF, val)
#define LCNCONF_GT(val)		CONF_GT(LCNCONF, val)
#define LCNCONF_LT(val)		CONF_LT(LCNCONF, val)
#define LCNCONF_LE(val)		CONF_LE(LCNCONF, val)

#define LCN40CONF_HAS(val)	CONF_HAS(LCN40CONF, val)
#define LCN40CONF_MSK(mask)	CONF_MSK(LCN40CONF, mask)
#define LCN40CONF_IS(val)	CONF_IS(LCN40CONF, val)
#define LCN40CONF_GE(val)	CONF_GE(LCN40CONF, val)
#define LCN40CONF_GT(val)	CONF_GT(LCN40CONF, val)
#define LCN40CONF_LT(val)	CONF_LT(LCN40CONF, val)
#define LCN40CONF_LE(val)	CONF_LE(LCN40CONF, val)

#define LCN20CONF_HAS(val)	CONF_HAS(LCN20CONF, val)
#define LCN20CONF_MSK(mask)	CONF_MSK(LCN20CONF, mask)
#define LCN20CONF_IS(val)	CONF_IS(LCN20CONF, val)
#define LCN20CONF_GE(val)	CONF_GE(LCN20CONF, val)
#define LCN20CONF_GT(val)	CONF_GT(LCN20CONF, val)
#define LCN20CONF_LT(val)	CONF_LT(LCN20CONF, val)
#define LCN20CONF_LE(val)	CONF_LE(LCN20CONF, val)

#define HTCONF_HAS(val)		CONF_HAS(HTCONF, val)
#define HTCONF_MSK(mask)	CONF_MSK(HTCONF, mask)
#define HTCONF_IS(val)		CONF_IS(HTCONF, val)
#define HTCONF_GE(val)		CONF_GE(HTCONF, val)
#define HTCONF_GT(val)		CONF_GT(HTCONF, val)
#define HTCONF_LT(val)		CONF_LT(HTCONF, val)
#define HTCONF_LE(val)		CONF_LE(HTCONF, val)

#define ACCONF_MSK(mask)	CONF_MSK(ACCONF, mask)
#define ACCONF2_MSK(mask)	CONF_MSK(ACCONF2, mask)
#define ACCONF_HAS(val) (\
	(ACCONF_CHK0(val, ACCONF1_BASE) && CONF_HAS(ACCONF, ACCONF_REV0(val, ACCONF1_BASE))) || \
	(ACCONF_CHK(val, ACCONF2_BASE) && CONF_HAS(ACCONF2, ACCONF_REV(val, ACCONF2_BASE))))
#define ACCONF_IS(val) (\
	(ACCONF_CHK0(val, ACCONF1_BASE) &&	\
	 !ACCONF2 && CONF_IS(ACCONF, ACCONF_REV0(val, ACCONF1_BASE))) ||	\
	(ACCONF_CHK(val, ACCONF2_BASE) &&	\
	 !ACCONF && CONF_IS(ACCONF2, ACCONF_REV(val, ACCONF2_BASE))))
#define ACCONF_GE(val) (\
	(ACCONF_CHK0(val, ACCONF1_BASE) && CONF_GE(ACCONF, ACCONF_REV0(val, ACCONF1_BASE))) || \
	(ACCONF_CHK0(val, ACCONF1_BASE) && ACCONF2) || \
	(ACCONF_CHK(val, ACCONF2_BASE) && CONF_GE(ACCONF2, ACCONF_REV(val, ACCONF2_BASE))))
#define ACCONF_GT(val) (\
	(ACCONF_CHK0(val, ACCONF1_BASE) && CONF_GT(ACCONF, ACCONF_REV0(val, ACCONF1_BASE))) || \
	(ACCONF_CHK0(val, ACCONF1_BASE) && ACCONF2) || \
	(ACCONF_CHK(val, ACCONF2_BASE) && CONF_GT(ACCONF2, ACCONF_REV(val, ACCONF2_BASE))))
#define ACCONF_LT(val) (\
	(ACCONF_CHK0(val, ACCONF1_BASE) && CONF_LT(ACCONF, ACCONF_REV0(val, ACCONF1_BASE))) || \
	(ACCONF_CHK(val, ACCONF2_BASE) && CONF_LT(ACCONF2, ACCONF_REV(val, ACCONF2_BASE))) || \
	(ACCONF_CHK(val, ACCONF2_BASE) && ACCONF))
#define ACCONF_LE(val) (\
	(ACCONF_CHK0(val, ACCONF1_BASE) && CONF_LE(ACCONF, ACCONF_REV0(val, ACCONF1_BASE))) || \
	(ACCONF_CHK(val, ACCONF2_BASE) && CONF_LE(ACCONF2, ACCONF_REV(val, ACCONF2_BASE))) || \
	(ACCONF_CHK(val, ACCONF2_BASE) && ACCONF))

#define D11CONF_MSK(mask) CONF_MSK(D11CONF, mask)
#define D11CONF_MSK2(lo, hi) (\
	(((D11CONF_CHK(lo, D11CONF1_BASE)) && (D11CONF_CHK(hi, D11CONF1_BASE))) && \
		CONF_MSK(D11CONF, MSK_RANGE(\
			D11CONF_REV(lo, D11CONF1_BASE), D11CONF_REV(hi, D11CONF1_BASE)))) || \
	(((D11CONF_CHK(lo, D11CONF2_BASE)) && (D11CONF_CHK(hi, D11CONF2_BASE))) && \
		CONF_MSK(D11CONF2, MSK_RANGE(\
			D11CONF_REV(lo, D11CONF2_BASE), D11CONF_REV(hi, D11CONF2_BASE)))) || \
	(((D11CONF_CHK(lo, D11CONF1_BASE)) && (D11CONF_CHK(hi, D11CONF2_BASE))) && \
		(CONF_MSK(D11CONF, MSK_RANGE(\
			D11CONF_REV(lo, D11CONF1_BASE), (D11CONF_WIDTH - 1))) || \
		CONF_MSK(D11CONF2, MSK_RANGE(0, D11CONF_REV(hi, D11CONF2_BASE))))))
#define D11CONF_HAS(val) (\
	(D11CONF_CHK(val, D11CONF1_BASE) && CONF_HAS(D11CONF, D11CONF_REV(val, D11CONF1_BASE))) ||\
	(D11CONF_CHK(val, D11CONF2_BASE) && CONF_HAS(D11CONF2, D11CONF_REV(val, D11CONF2_BASE))) ||\
	(D11CONF_CHK(val, D11CONF3_BASE) && CONF_HAS(D11CONF3, D11CONF_REV(val, D11CONF3_BASE))))
#define D11CONF_IS(val) (\
	(D11CONF_CHK(val, D11CONF1_BASE) &&	\
	 !D11CONF2 && !D11CONF3 && CONF_IS(D11CONF, D11CONF_REV(val, D11CONF1_BASE))) ||	\
	(D11CONF_CHK(val, D11CONF2_BASE) &&	\
	 !D11CONF && ! D11CONF3 && CONF_IS(D11CONF2, D11CONF_REV(val, D11CONF2_BASE))) ||	\
	(D11CONF_CHK(val, D11CONF3_BASE) &&	\
	 !D11CONF && ! D11CONF2 && CONF_IS(D11CONF3, D11CONF_REV(val, D11CONF3_BASE))))
#define D11CONF_GE(val) (\
	(D11CONF_CHK(val, D11CONF1_BASE) && CONF_GE(D11CONF, D11CONF_REV(val, D11CONF1_BASE))) || \
	(D11CONF_CHK(val, D11CONF1_BASE) && (D11CONF2 || D11CONF3)) || \
	(D11CONF_CHK(val, D11CONF2_BASE) && CONF_GE(D11CONF2, D11CONF_REV(val, D11CONF2_BASE))) || \
	(D11CONF_CHK(val, D11CONF2_BASE) && D11CONF3) || \
	(D11CONF_CHK(val, D11CONF3_BASE) && CONF_GE(D11CONF3, D11CONF_REV(val, D11CONF3_BASE))))
#define D11CONF_GT(val) (\
	(D11CONF_CHK(val, D11CONF1_BASE) && CONF_GT(D11CONF, D11CONF_REV(val, D11CONF1_BASE))) || \
	(D11CONF_CHK(val, D11CONF1_BASE) && (D11CONF2 || D11CONF3)) || \
	(D11CONF_CHK(val, D11CONF2_BASE) && CONF_GT(D11CONF2, D11CONF_REV(val, D11CONF2_BASE))) || \
	(D11CONF_CHK(val, D11CONF2_BASE) && D11CONF3) || \
	(D11CONF_CHK(val, D11CONF3_BASE) && CONF_GT(D11CONF3, D11CONF_REV(val, D11CONF3_BASE))))
#define D11CONF_LT(val) (\
	(D11CONF_CHK(val, D11CONF1_BASE) && CONF_LT(D11CONF, D11CONF_REV(val, D11CONF1_BASE))) || \
	(D11CONF_CHK(val, D11CONF2_BASE) && CONF_LT(D11CONF2, D11CONF_REV(val, D11CONF2_BASE))) || \
	(D11CONF_CHK(val, D11CONF3_BASE) && CONF_LT(D11CONF3, D11CONF_REV(val, D11CONF3_BASE))) || \
	(D11CONF_CHK(val, D11CONF2_BASE) && D11CONF) || \
	(D11CONF_CHK(val, D11CONF3_BASE) && (D11CONF || D11CONF2)))
#define D11CONF_LE(val) (\
	(D11CONF_CHK(val, D11CONF1_BASE) && CONF_LE(D11CONF, D11CONF_REV(val, D11CONF1_BASE))) || \
	(D11CONF_CHK(val, D11CONF2_BASE) && CONF_LE(D11CONF2, D11CONF_REV(val, D11CONF2_BASE))) || \
	(D11CONF_CHK(val, D11CONF3_BASE) && CONF_LE(D11CONF3, D11CONF_REV(val, D11CONF3_BASE))) || \
	(D11CONF_CHK(val, D11CONF2_BASE) && D11CONF) || \
	(D11CONF_CHK(val, D11CONF3_BASE) && (D11CONF || D11CONF2)))

#define PHYCONF_HAS(val) CONF_HAS(PHYTYPE, val)
#define PHYCONF_IS(val)	CONF_IS(PHYTYPE, val)

/* Macros to check (but override) a run-time value; compile-time
 * override allows unconfigured code to be optimized out.
 *
 * Note: the XXCONF values contain 0 or more bits, each of which represents a supported revision
 */

#if IS_MULTI_REV(ACONF)
#define AREV_IS(var, val)	(ACONF_HAS(val) && (var) == (val))
#define AREV_GE(var, val)	(!ACONF_LT(val) || (var) >= (val))
#define AREV_GT(var, val)	(!ACONF_LE(val) || (var) > (val))
#define AREV_LT(var, val)	(!ACONF_GE(val) || (var) < (val))
#define AREV_LE(var, val)	(!ACONF_GT(val) || (var) <= (val))
#else
#define AREV_IS(var, val)	ACONF_HAS(val)
#define AREV_GE(var, val)	ACONF_GE(val)
#define AREV_GT(var, val)	ACONF_GT(val)
#define AREV_LT(var, val)	ACONF_LT(val)
#define AREV_LE(var, val)	ACONF_LE(val)
#endif	/* IS_MULTI_REV(ACONF) */

#if IS_MULTI_REV(GCONF)
#define GREV_IS(var, val)	(GCONF_HAS(val) && (var) == (val))
#define GREV_GE(var, val)	(!GCONF_LT(val) || (var) >= (val))
#define GREV_GT(var, val)	(!GCONF_LE(val) || (var) > (val))
#define GREV_LT(var, val)	(!GCONF_GE(val) || (var) < (val))
#define GREV_LE(var, val)	(!GCONF_GT(val) || (var) <= (val))
#else
#define GREV_IS(var, val)	GCONF_HAS(val)
#define GREV_GE(var, val)	GCONF_GE(val)
#define GREV_GT(var, val)	GCONF_GT(val)
#define GREV_LT(var, val)	GCONF_LT(val)
#define GREV_LE(var, val)	GCONF_LE(val)
#endif	/* IS_MULTI_REV(GCONF) */

#if IS_MULTI_REV(NCONF)
#define NREV_IS(var, val)	(NCONF_HAS(val) && (var) == (val))
#define NREV_GE(var, val)	(!NCONF_LT(val) || (var) >= (val))
#define NREV_GT(var, val)	(!NCONF_LE(val) || (var) > (val))
#define NREV_LT(var, val)	(!NCONF_GE(val) || (var) < (val))
#define NREV_LE(var, val)	(!NCONF_GT(val) || (var) <= (val))
#else
#define NREV_IS(var, val)	NCONF_HAS(val)
#define NREV_GE(var, val)	NCONF_GE(val)
#define NREV_GT(var, val)	NCONF_GT(val)
#define NREV_LT(var, val)	NCONF_LT(val)
#define NREV_LE(var, val)	NCONF_LE(val)
#endif	/* IS_MULTI_REV(NCONF) */

#if IS_MULTI_REV(HTCONF)
#define HTREV_IS(var, val)	(HTCONF_HAS(val) && (var) == (val))
#define HTREV_GE(var, val)	(!HTCONF_LT(val) || (var) >= (val))
#define HTREV_GT(var, val)	(!HTCONF_LE(val) || (var) > (val))
#define HTREV_LT(var, val)	(!HTCONF_GE(val) || (var) < (val))
#define HTREV_LE(var, val)	(!HTCONF_GT(val) || (var) <= (val))
#else
#define HTREV_IS(var, val)	HTCONF_HAS(val)
#define HTREV_GE(var, val)	HTCONF_GE(val)
#define HTREV_GT(var, val)	HTCONF_GT(val)
#define HTREV_LT(var, val)	HTCONF_LT(val)
#define HTREV_LE(var, val)	HTCONF_LE(val)
#endif	/* IS_MULTI_REV(HTCONF) */

#if IS_MULTI_REV(LPCONF)
#define LPREV_IS(var, val)	(LPCONF_HAS(val) && (var) == (val))
#define LPREV_GE(var, val)	(!LPCONF_LT(val) || (var) >= (val))
#define LPREV_GT(var, val)	(!LPCONF_LE(val) || (var) > (val))
#define LPREV_LT(var, val)	(!LPCONF_GE(val) || (var) < (val))
#define LPREV_LE(var, val)	(!LPCONF_GT(val) || (var) <= (val))
#else
#define LPREV_IS(var, val)	LPCONF_HAS(val)
#define LPREV_GE(var, val)	LPCONF_GE(val)
#define LPREV_GT(var, val)	LPCONF_GT(val)
#define LPREV_LT(var, val)	LPCONF_LT(val)
#define LPREV_LE(var, val)	LPCONF_LE(val)
#endif	/* IS_MULTI_REV(LPCONF) */

#if IS_MULTI_REV(SSLPNCONF)
#define SSLPNREV_IS(var, val)	(SSLPNCONF_HAS(val) && (var) == (val))
#define SSLPNREV_GE(var, val)	(!SSLPNCONF_LT(val) || (var) >= (val))
#define SSLPNREV_GT(var, val)	(!SSLPNCONF_LE(val) || (var) > (val))
#define SSLPNREV_LT(var, val)	(!SSLPNCONF_GE(val) || (var) < (val))
#define SSLPNREV_LE(var, val)	(!SSLPNCONF_GT(val) || (var) <= (val))
#else
#define SSLPNREV_IS(var, val)	SSLPNCONF_HAS(val)
#define SSLPNREV_GE(var, val)	SSLPNCONF_GE(val)
#define SSLPNREV_GT(var, val)	SSLPNCONF_GT(val)
#define SSLPNREV_LT(var, val)	SSLPNCONF_LT(val)
#define SSLPNREV_LE(var, val)	SSLPNCONF_LE(val)
#endif	/* IS_MULTI_REV(SSLPNCONF) */

#if IS_MULTI_REV(LCNCONF)
#define LCNREV_IS(var, val)	(LCNCONF_HAS(val) && (var) == (val))
#define LCNREV_GE(var, val)	(!LCNCONF_LT(val) || (var) >= (val))
#define LCNREV_GT(var, val)	(!LCNCONF_LE(val) || (var) > (val))
#define LCNREV_LT(var, val)	(!LCNCONF_GE(val) || (var) < (val))
#define LCNREV_LE(var, val)	(!LCNCONF_GT(val) || (var) <= (val))
#else
#define LCNREV_IS(var, val)	LCNCONF_HAS(val)
#define LCNREV_GE(var, val)	LCNCONF_GE(val)
#define LCNREV_GT(var, val)	LCNCONF_GT(val)
#define LCNREV_LT(var, val)	LCNCONF_LT(val)
#define LCNREV_LE(var, val)	LCNCONF_LE(val)
#endif	/* IS_MULTI_REV(LCNCONF) */

#if IS_MULTI_REV(LCN40CONF)
#define LCN40REV_IS(var, val)	(LCN40CONF_HAS(val) && (var) == (val))
#define LCN40REV_GE(var, val)	(!LCN40CONF_LT(val) || (var) >= (val))
#define LCN40REV_GT(var, val)	(!LCN40CONF_LE(val) || (var) > (val))
#define LCN40REV_LT(var, val)	(!LCN40CONF_GE(val) || (var) < (val))
#define LCN40REV_LE(var, val)	(!LCN40CONF_GT(val) || (var) <= (val))
#else
#define LCN40REV_IS(var, val)	LCN40CONF_HAS(val)
#define LCN40REV_GE(var, val)	LCN40CONF_GE(val)
#define LCN40REV_GT(var, val)	LCN40CONF_GT(val)
#define LCN40REV_LT(var, val)	LCN40CONF_LT(val)
#define LCN40REV_LE(var, val)	LCN40CONF_LE(val)
#endif	/* IS_MULTI_REV(LCN40CONF) */

#if IS_MULTI_REV(LCN20CONF)
#define LCN20REV_IS(var, val)	(LCN20CONF_HAS(val) && (var) == (val))
#define LCN20REV_GE(var, val)	(!LCN20CONF_LT(val) || (var) >= (val))
#define LCN20REV_GT(var, val)	(!LCN20CONF_LE(val) || (var) > (val))
#define LCN20REV_LT(var, val)	(!LCN20CONF_GE(val) || (var) < (val))
#define LCN20REV_LE(var, val)	(!LCN20CONF_GT(val) || (var) <= (val))
#else
#define LCN20REV_IS(var, val)	LCN20CONF_HAS(val)
#define LCN20REV_GE(var, val)	LCN20CONF_GE(val)
#define LCN20REV_GT(var, val)	LCN20CONF_GT(val)
#define LCN20REV_LT(var, val)	LCN20CONF_LT(val)
#define LCN20REV_LE(var, val)	LCN20CONF_LE(val)
#endif	/* IS_MULTI_REV(LCN20CONF) */

#if IS_MULTI_REV2(ACCONF, ACCONF2)
#define ACREV_IS(var, val)	(ACCONF_HAS(val) && (var) == (val))
#define ACREV_GE(var, val)	(!ACCONF_LT(val) || (var) >= (val))
#define ACREV_GT(var, val)	(!ACCONF_LE(val) || (var) > (val))
#define ACREV_LT(var, val)	(!ACCONF_GE(val) || (var) < (val))
#define ACREV_LE(var, val)	(!ACCONF_GT(val) || (var) <= (val))
#else
#define ACREV_IS(var, val)	ACCONF_HAS(val)
#define ACREV_GE(var, val)	ACCONF_GE(val)
#define ACREV_GT(var, val)	ACCONF_GT(val)
#define ACREV_LT(var, val)	ACCONF_LT(val)
#define ACREV_LE(var, val)	ACCONF_LE(val)
#endif	/* IS_MULTI_REV(ACCONF) */

#define D11REV_IS(var, val)	(D11CONF_HAS(val) && (D11CONF_IS(val) || ((var) == (val))))
#define D11REV_GE(var, val)	(D11CONF_GE(val) && (!D11CONF_LT(val) || ((var) >= (val))))
#define D11REV_GT(var, val)	(D11CONF_GT(val) && (!D11CONF_LE(val) || ((var) > (val))))
#define D11REV_LT(var, val)	(D11CONF_LT(val) && (!D11CONF_GE(val) || ((var) < (val))))
#define D11REV_LE(var, val)	(D11CONF_LE(val) && (!D11CONF_GT(val) || ((var) <= (val))))

#define PHYTYPE_IS(var, val)	(PHYCONF_HAS(val) && (PHYCONF_IS(val) || ((var) == (val))))

#if D11CONF_GE(64)
#define VASIP_HW_SUPPORT
#define VASIP_SPECTRUM_ANALYSIS
#define WL_HWKTAB
#define WL_PSMX
#define WL_AUXPMQ

/* MU-PKTENG only for NIC Build */

#if defined(WLC_HIGH) && defined(WLC_LOW) && !defined(DONGLEBUILD)
#if defined(AP) && defined(WL_BEAMFORMING) && defined(WL_MU_TX) && (defined(WLTEST) || \
	defined(WLPKTENG))
#define WL_MUPKTENG
#endif // endif
#endif // endif
#else
#ifdef WL_MU_TX
#undef WL_MU_TX
#endif // endif
#endif /* D11CONF_GE(64) */

#define VASIP_PRESENT(rev)	D11REV_GE(rev, 64)

/* Finally, early-exit from switch case if anyone wants it... */

#define CASECHECK(config, val)	if (!(CONF_HAS(config, val))) break
#define CASEMSK(config, mask)	if (!(CONF_MSK(config, mask))) break

#if (D11CONF ^ (D11CONF & D11_DEFAULT))
#error "Unsupported MAC revision configured"
#endif // endif
#if (ACONF ^ (ACONF & APHY_DEFAULT))
#error "Unsupported APHY revision configured"
#endif // endif
#if (GCONF ^ (GCONF & GPHY_DEFAULT))
#error "Unsupported GPHY revision configured"
#endif // endif
#if (NCONF ^ (NCONF & NPHY_DEFAULT))
#error "Unsupported NPHY revision configured"
#endif // endif
#if (LPCONF ^ (LPCONF & LPPHY_DEFAULT))
#error "Unsupported LPPHY revision configured"
#endif // endif
#if (LCNCONF ^ (LCNCONF & LCNPHY_DEFAULT))
#error "Unsupported LCNPHY revision configured"
#endif // endif
#if (HTCONF ^ (HTCONF & HTPHY_DEFAULT))
#error "Unsupported HTPHY revision configured"
#endif // endif
#if (LCN40CONF ^ (LCN40CONF & LCN40PHY_DEFAULT))
#error "Unsupported LCN40PHY revision configured"
#endif // endif
#if (LCN20CONF ^ (LCN20CONF & LCN20PHY_DEFAULT))
#error "Unsupported LCN20PHY revision configured"
#endif // endif
#if (ACCONF ^ (ACCONF & ACPHY_DEFAULT)) || (ACCONF2 ^ (ACCONF2 & ACPHY_DEFAULT2))
#error "Unsupported ACPHY revision configured"
#endif // endif

/* *** Consistency checks *** */
#if !D11CONF && !D11CONF2 && !D11CONF3
#error "No MAC revisions configured!"
#endif // endif

#if !ACONF && !GCONF && !NCONF && !LPCONF && !SSLPNCONF && !LCNCONF && !HTCONF && \
	!LCN40CONF && !ACCONF && !ACCONF2 && !LCN20CONF
#error "No PHY configured!"
#endif // endif

/* Set up PHYTYPE automatically: (depends on PHY_TYPE_X, from d11.h) */
#if ACONF
#define _PHYCONF_A (1U << PHY_TYPE_A)
#else
#define _PHYCONF_A 0
#endif /* ACONF */

#if GCONF
#define _PHYCONF_G (1U << PHY_TYPE_G)
#else
#define _PHYCONF_G 0
#endif /* GCONF */

#if NCONF
#define _PHYCONF_N (1U << PHY_TYPE_N)
#else
#define _PHYCONF_N 0
#endif /* NCONF */

#if LPCONF
#define _PHYCONF_LP (1U << PHY_TYPE_LP)
#else
#define _PHYCONF_LP 0
#endif /* LPCONF */

#if SSLPNCONF
#define _PHYCONF_SSLPN (1U << PHY_TYPE_SSN)
#else
#define _PHYCONF_SSLPN 0
#endif /* SSLPNCONF */

#if LCNCONF
#define _PHYCONF_LCN (1U << PHY_TYPE_LCN)
#else
#define _PHYCONF_LCN 0
#endif /* LCNCONF */

#if LCN40CONF
#define _PHYCONF_LCN40 (1U << PHY_TYPE_LCN40)
#else
#define _PHYCONF_LCN40 0
#endif /* LCN40CONF */

#if LCN20CONF
#define _PHYCONF_LCN20 (1U << PHY_TYPE_LCN20)
#else
#define _PHYCONF_LCN20 0
#endif /* LCN20CONF */

#if HTCONF
#define _PHYCONF_HT (1U << PHY_TYPE_HT)
#else
#define _PHYCONF_HT 0
#endif /* HTCONF */

#if ACCONF || ACCONF2
#define _PHYCONF_AC (1U << PHY_TYPE_AC)
#else
#define _PHYCONF_AC 0
#endif /* ACCONF || ACCONF2 */

#define PHYTYPE (_PHYCONF_A | _PHYCONF_G | _PHYCONF_N | _PHYCONF_LP | \
	_PHYCONF_SSLPN | _PHYCONF_LCN | _PHYCONF_HT | _PHYCONF_LCN40 |  \
	_PHYCONF_AC | _PHYCONF_LCN20)

/* Utility macro to identify 802.11n (HT) capable PHYs */
#define PHYTYPE_11N_CAP(phytype) \
	(PHYTYPE_IS(phytype, PHY_TYPE_N) ||	\
	 PHYTYPE_IS(phytype, PHY_TYPE_SSN) ||	\
	 PHYTYPE_IS(phytype, PHY_TYPE_LCN) ||	\
	 PHYTYPE_IS(phytype, PHY_TYPE_LCN40) ||	\
	 PHYTYPE_IS(phytype, PHY_TYPE_LCN20) ||	\
	 PHYTYPE_IS(phytype, PHY_TYPE_AC) ||	\
	 PHYTYPE_IS(phytype, PHY_TYPE_HT))

/* Utility macro to identify MIMO capable PHYs */
#define PHYTYPE_MIMO_CAP(phytype) \
	(PHYTYPE_11N_CAP(phytype) && \
	 !PHYTYPE_IS(phytype, PHY_TYPE_SSN) && \
	 !PHYTYPE_IS(phytype, PHY_TYPE_LCN) && \
	 !PHYTYPE_IS(phytype, PHY_TYPE_LCN40))

/* Utility macro to identify 802.11ac (VHT) capable PHYs */
#define PHYTYPE_VHT_CAP(phytype) \
	(PHYTYPE_IS(phytype, PHY_TYPE_AC))

#define PHYTYPE_HT_CAP(band) \
	(PHYTYPE_IS((band)->phytype, PHY_TYPE_HT) ||	\
	 PHYTYPE_IS((band)->phytype, PHY_TYPE_AC))

/* Last but not least: shorter wlc-specific var checks */
#define WLCISAPHY(band)		PHYTYPE_IS((band)->phytype, PHY_TYPE_A)
#define WLCISGPHY(band)		PHYTYPE_IS((band)->phytype, PHY_TYPE_G)
#define WLCISNPHY(band)		PHYTYPE_IS((band)->phytype, PHY_TYPE_N)
#define WLCISLPPHY(band)	PHYTYPE_IS((band)->phytype, PHY_TYPE_LP)
#define WLCISSSLPNPHY(band)	PHYTYPE_IS((band)->phytype, PHY_TYPE_SSN)
#define WLCISLCNPHY(band)	PHYTYPE_IS((band)->phytype, PHY_TYPE_LCN)
#define WLCISHTPHY(band)	PHYTYPE_IS((band)->phytype, PHY_TYPE_HT)
#define WLCISLCN40PHY(band)	PHYTYPE_IS((band)->phytype, PHY_TYPE_LCN40)
#define WLCISLCN20PHY(band)	PHYTYPE_IS((band)->phytype, PHY_TYPE_LCN20)
#define WLCISACPHY(band)	PHYTYPE_IS((band)->phytype, PHY_TYPE_AC)

#define WLC_PHY_11N_CAP(band)	PHYTYPE_11N_CAP((band)->phytype)
#define WLC_PHY_VHT_CAP(band)	PHYTYPE_VHT_CAP((band)->phytype)

/**********************************************************************
 * ------------- End of Core phy/rev configuration. ----------------- *
 * ********************************************************************
 */

/*************************************************
 * Defaults for tunables (e.g. sizing constants)
 *
 * For each new tunable, add a member to the end
 * of wlc_tunables_t in wlc_pub.h to enable
 * runtime checks of tunable values. (Directly
 * using the macros in code invalidates ROM code)
 *
 * ***********************************************
 */
#ifndef NTXD
#define NTXD		512   /* Max # of entries in Tx FIFO. 512 maximum */
#endif /* NTXD */
#ifndef NRXD
#define NRXD		256   /* Max # of entries in Rx FIFO. 512 maximum */
#endif /* NRXD */

/* Separate tunable for DMA descriptor ring size for cores
 * with large descriptor ring support (4k descriptors)
 */
#ifndef NTXD_LARGE
#define NTXD_LARGE		NTXD		/* TX descriptor ring */
#endif // endif

#ifndef NRXD_LARGE
#define NRXD_LARGE		NRXD		/* RX descriptor ring */
#endif // endif

#ifndef NRXBUFPOST
#define	NRXBUFPOST	64		/* try to keep this # rbufs posted to the chip */
#endif /* NRXBUFPOST */

#ifdef TXQ_MUX

/* Water marks used in MUX SCB implementation. LOW_TXQ only implementation uses txmaxpkts */
#ifndef WLC_TXQ_HIGHWATER
#define	WLC_TXQ_HIGHWATER	5000	/* TxQ max desired level in usec for flow control */
#endif /* WLC_TXQ_HIGHWATER */

#ifndef WLC_TXQ_LOWWATER
#define	WLC_TXQ_LOWWATER	2500	/* TxQ min desired level in usec for flow control */
#endif /* WLC_TXQ_LOWWATER */

#endif /* TXQ_MUX */

#ifndef TXMR
#define TXMR			2	/* number of outstanding reads */
#endif // endif

#ifndef TXPREFTHRESH
#define TXPREFTHRESH		8	/* prefetch threshold */
#endif // endif

#ifndef TXPREFCTL
#define TXPREFCTL		16	/* max descr allowed in prefetch request */
#endif // endif

#ifndef TXBURSTLEN
#define TXBURSTLEN		1024	/* burst length for dma reads */
#endif // endif

#ifndef RXPREFTHRESH
#define RXPREFTHRESH		8	/* prefetch threshold */
#endif // endif

#ifndef RXPREFCTL
#define RXPREFCTL		16	/* max descr allowed in prefetch request */
#endif // endif

#ifndef RXBURSTLEN
#define RXBURSTLEN		128	/* burst length for dma writes */
#endif // endif

#ifndef MRRS
#define MRRS			AUTO	/* Max read request size */
#endif // endif

#ifndef TXMR_AC2
#define TXMR_AC2		12	/* number of outstanding reads */
#endif // endif

#ifndef TXPREFTHRESH_AC2
#define TXPREFTHRESH_AC2	8	/* prefetch threshold */
#endif // endif

#ifndef TXPREFCTL_AC2
#define TXPREFCTL_AC2		16	/* max descr allowed in prefetch request */
#endif // endif

#ifndef TXBURSTLEN_AC2
#define TXBURSTLEN_AC2		1024	/* burst length for dma reads */
#endif // endif

#ifndef RXPREFTHRESH_AC2
#define RXPREFTHRESH_AC2	8	/* prefetch threshold */
#endif // endif

#ifndef RXPREFCTL_AC2
#define RXPREFCTL_AC2		16	/* max descr allowed in prefetch request */
#endif // endif

#ifndef RXBURSTLEN_AC2
#define RXBURSTLEN_AC2		128	/* burst length for dma writes */
#endif // endif

#ifndef MRRS_AC2
#define MRRS_AC2		1024	/* Max read request size */
#endif // endif

#ifndef WLC_MAXMODULES
#define WLC_MAXMODULES		73	/* max #  wlc_module_register() calls */
#endif // endif

#ifndef MAXSCB				/* station control blocks in cache */
#ifdef AP
#define	MAXSCB		128		/* Maximum SCBs in cache for AP */
#else
#define MAXSCB		32		/* Maximum SCBs in cache for STA */
#endif /* AP */
#endif /* MAXSCB */

#ifndef MAXSCBCUBBIES
#define MAXSCBCUBBIES		32	/* max number of cubbies in scb container */
#endif // endif

#ifndef MAXBSSCFGCUBBIES
#define MAXBSSCFGCUBBIES	32	/* max number of cubbies in bsscfg container */
#endif // endif

#ifdef AMPDU_NUM_MPDU			/* Used for the memory limited dongles. */
#define AMPDU_NUM_MPDU_1SS_D11LEGACY	AMPDU_NUM_MPDU /* Small fifo D11 Maccore < 16 */
#define AMPDU_NUM_MPDU_2SS_D11LEGACY	AMPDU_NUM_MPDU /* Small fifo D11 Maccore < 16 */
#define AMPDU_NUM_MPDU_3SS_D11LEGACY	AMPDU_NUM_MPDU /* Small fifo D11 Maccore < 16 */
#define AMPDU_NUM_MPDU_1SS_D11HT	AMPDU_NUM_MPDU /* Medium fifo 16 < D11 Maccore < 40 */
#define AMPDU_NUM_MPDU_2SS_D11HT	AMPDU_NUM_MPDU /* Medium fifo 16 < D11 Maccore < 40 */
#define AMPDU_NUM_MPDU_3SS_D11HT	AMPDU_NUM_MPDU /* Medium fifo 16 < D11 Maccore < 40 */
#define AMPDU_NUM_MPDU_1SS_D11AQM	AMPDU_NUM_MPDU /* Large fifo D11 Maccore > 40 */
#define AMPDU_NUM_MPDU_2SS_D11AQM	AMPDU_NUM_MPDU /* Large fifo D11 Maccore > 40 */
#define AMPDU_NUM_MPDU_3SS_D11AQM	AMPDU_NUM_MPDU /* Large fifo D11 Maccore > 40 */
#define AMPDU_NUM_MPDU_3STREAMS		AMPDU_NUM_MPDU
#define AMPDU_MAX_MPDU			AMPDU_NUM_MPDU
#define AMPDU_NUM_MPDU_LEGACY		AMPDU_NUM_MPDU
#else
#ifndef AMPDU_NUM_MPDU_1SS_D11LEGACY
#define AMPDU_NUM_MPDU_1SS_D11LEGACY	16 /* Small fifo D11 Maccore < 16 */
#endif // endif

#ifndef AMPDU_NUM_MPDU_2SS_D11LEGACY
#define AMPDU_NUM_MPDU_2SS_D11LEGACY	16 /* Small fifo D11 Maccore < 16 */
#endif // endif

#ifndef AMPDU_NUM_MPDU_3SS_D11LEGACY
#define AMPDU_NUM_MPDU_3SS_D11LEGACY	16 /* Small fifo D11 Maccore < 16 */
#endif // endif

#ifndef AMPDU_NUM_MPDU_1SS_D11HT
#define AMPDU_NUM_MPDU_1SS_D11HT	32 /* Medium fifo 16 < D11 Maccore < 40 */
#endif // endif

#ifndef AMPDU_NUM_MPDU_2SS_D11HT
#define AMPDU_NUM_MPDU_2SS_D11HT	32 /* Medium fifo 16 < D11 Maccore < 40 */
#endif // endif

#ifndef AMPDU_NUM_MPDU_3SS_D11HT
#define AMPDU_NUM_MPDU_3SS_D11HT	32 /* Medium fifo 16 < D11 Maccore < 40 */
#endif // endif

#ifndef AMPDU_NUM_MPDU_1SS_D11AQM
#define AMPDU_NUM_MPDU_1SS_D11AQM	32 /* Large fifo D11 Maccore > 40 */
#endif // endif

#ifndef AMPDU_NUM_MPDU_2SS_D11AQM
#define AMPDU_NUM_MPDU_2SS_D11AQM	48 /* Large fifo D11 Maccore > 40 */
#endif // endif

#ifndef AMPDU_NUM_MPDU_3SS_D11AQM
#define AMPDU_NUM_MPDU_3SS_D11AQM	64 /* Large fifo D11 Maccore > 40 */
#endif // endif

#ifndef AMPDU_NUM_MPDU_3STREAMS
#define AMPDU_NUM_MPDU_3STREAMS		AMPDU_NUM_MPDU_3SS_D11LEGACY
#endif // endif

#define AMPDU_NUM_MPDU			AMPDU_NUM_MPDU_2SS_D11LEGACY
#endif /* AMPDU_NUM_MPDU */

#ifndef AMPDU_PKTQ_LEN
#define AMPDU_PKTQ_LEN		1024
#endif // endif

#ifndef AMPDU_PKTQ_FAVORED_LEN
#define AMPDU_PKTQ_FAVORED_LEN	1024
#endif // endif

#ifndef MAXPCBCDS
/* size of packet class callback class descriptor array */
#define MAXPCBCDS 4
#endif // endif

#ifndef MAXCD1PCBS
/* max # class 1 packet class callbacks */
#define MAXCD1PCBS 16
#endif // endif

#ifndef MAXCD2PCBS
/* max # class 2 packet class callbacks */
#define MAXCD2PCBS 4
#endif // endif

#ifndef MAXCD3PCBS
/* max # class 3 packet class callbacks */
#define MAXCD3PCBS 2
#endif // endif

#ifndef MAXCD4PCBS
/* max # class 4 packet class callbacks */
#define MAXCD4PCBS 2
#endif // endif

#ifndef AMPDU_MAX_MPDU
#if D11CONF_GE(40)
#define AMPDU_MAX_MPDU		64 /* max number of mpdus in an ampdu */
#else
#define AMPDU_MAX_MPDU		32 /* max number of mpdus in an ampdu */
#endif // endif
#endif /* AMPDU_MAX_MPDU */

#ifndef AMPDU_NUM_MPDU_LEGACY
#define AMPDU_NUM_MPDU_LEGACY	16
#endif // endif

#define D11HT_CORE	16
#define D11AQM_CORE	40 /* MAC supports hardware qggregation */

/* Count of packet callback structures. either of following
 * 1. Set to the number of SCBs since a STA
 * can queue up a rate callback for each IBSS STA it knows about, and an AP can
 * queue up an "are you there?" Null Data callback for each associated STA
 * 2. controlled by tunable config file
 */
#ifndef MAXPKTCB
#define MAXPKTCB	MAXSCB	/* Max number of packet callbacks */
#endif /* MAXPKTCB */

#ifndef WLC_MAXTDLS
#ifdef WLTDLS
#define WLC_MAXTDLS	4		/* Max # of TDLS links */
#define WLC_MAXDLS_TIMERS	10	/* TDLS_LOOKASIDE_SIZE */
#else
#define WLC_MAXTDLS	0		/* Max # of TDLS links */
#define WLC_MAXDLS_TIMERS	0
#endif /* WLTDLS */
#endif /* WLC_MAXTDLS */

#ifndef WLC_MAXMFPS
#ifdef MFP
#if defined(NDIS630) || defined(NDIS640)
#define WLC_MAXMFPS	1	/* Win8 onwards only supports 802.11w on primary port */
#else
#define WLC_MAXMFPS	8	/* in general max times for MFP */
#endif /* NDIS630 || NDIS640 */
#else /* !MFP */
#define WLC_MAXMFPS	0
#endif /* MFP */
#endif /* WLC_MAXMFPS */

#ifndef CTFPOOLSZ
#define CTFPOOLSZ       128
#endif /* CTFPOOLSZ */

/* NetBSD also needs to keep track of this */
#ifndef WLC_MAX_UCODE_BSS
#define WLC_MAX_UCODE_BSS	(16)		/* Number of BSS handled in ucode bcn/prb */
#endif /* WLC_MAX_UCODE_BSS */
#define WLC_MAX_UCODE_BSS4	(4)		/* Number of BSS handled in sw bcn/prb */
#ifndef WLC_MAXBSSCFG
#ifdef PSTA
#define WLC_MAXBSSCFG		(1 + 50)		/* max # BSS configs */
#else /* PSTA */
#ifdef AP
#define WLC_MAXBSSCFG		(WLC_MAX_UCODE_BSS)	/* max # BSS configs */
#else
#define WLC_MAXBSSCFG		(1 )	/* max # BSS configs */
#endif /* AP */
#endif /* PSTA */
#endif /* WLC_MAXBSSCFG */

#ifndef MAXBSS
#define MAXBSS		128	/* max # available networks */
#endif /* MAXBSS */

#ifdef DONGLEBUILD
#define MAXUSCANBSS	16	/* reduce active limit for user scans */
#if (MAXBSS < MAXUSCANBSS)
#undef MAXUSCANBSS
#endif // endif
#endif /* DONGLEBUILD */
#ifndef MAXUSCANBSS
#define MAXUSCANBSS MAXBSS
#endif // endif

/* Max delay for timer interrupt. Platform specific and hence is a tunable */
#ifndef MCHAN_TIMER_DELAY_US
#ifdef WLC_HIGH_ONLY
#define MCHAN_TIMER_DELAY_US 5000
#else
#define MCHAN_TIMER_DELAY_US 200
#endif /* WLC_HIGH_ONLY */
#endif /* MCHAN_TIMER_DELAY_US */

#ifndef WLC_DATAHIWAT
#define WLC_DATAHIWAT		50	/* data msg txq hiwat mark */
#endif /* WLC_DATAHIWAT */

#ifndef WLC_AMPDUDATAHIWAT
#define WLC_AMPDUDATAHIWAT	128	/* enough to cover tow full size aggr */
#endif /* WLC_AMPDUDATAHIWAT */

/* bounded rx loops */
#ifndef RXBND
#define RXBND		8	/* max # frames to process in wlc_recv() */
#endif	/* RXBND */

#ifndef TXSBND
#define TXSBND		8	/* max # tx status to process in wlc_txstatus() */
#endif	/* TXSBND */

#ifndef PKTCBND
#define PKTCBND		8	/* max # frames to chain in wlc_recv() */
#endif	/* PKTCBND */

/* VHT 3x3 tunable value defaults */
#ifndef NTXD_AC3X3
#define NTXD_AC3X3		NTXD	/* TX descriptor ring */
#endif // endif

#ifndef NRXD_AC3X3
#define NRXD_AC3X3		NRXD	/* RX descriptor ring */
#endif // endif

#ifndef NTXD_LARGE_AC3X3
#define NTXD_LARGE_AC3X3	NTXD_LARGE
#endif // endif

#ifndef NRXD_LARGE_AC3X3
#define NRXD_LARGE_AC3X3	NRXD_LARGE
#endif // endif

#ifndef NRXBUFPOST_AC3X3
#define NRXBUFPOST_AC3X3	NRXBUFPOST	/* # rx buffers posted */
#endif // endif

#ifndef RXBND_AC3X3
#define RXBND_AC3X3		RXBND	/* max # rx frames to process */
#endif // endif

#ifndef PKTCBND_AC3X3
#define PKTCBND_AC3X3		PKTCBND	/* max # frames to chain in wlc_recv() */
#endif // endif

#ifndef CTFPOOLSZ_AC3X3
#define CTFPOOLSZ_AC3X3		CTFPOOLSZ	/* max buffers in ctfpool */
#endif // endif

#ifdef BCMSUP_PSK
#define IDSUP_NOTIF_SRVR_OBJS	1
#define IDSUP_NOTIF_CLNT_OBJS	5
#else
#define IDSUP_NOTIF_SRVR_OBJS	0
#define IDSUP_NOTIF_CLNT_OBJS	0
#endif // endif

/* Maximum number of notification servers. */
#ifndef MAX_NOTIF_SERVERS
#define MAX_NOTIF_SERVERS	(18+IDSUP_NOTIF_SRVR_OBJS)
#endif // endif

/* Maximum number of notification clients. */
#ifndef MAX_NOTIF_CLIENTS
#define MAX_NOTIF_CLIENTS	(34+IDSUP_NOTIF_CLNT_OBJS)
#endif // endif

/* Maximum number of memory pools. */
#ifndef MAX_MEMPOOLS
#define MAX_MEMPOOLS	5
#endif // endif

/* Maximum # of IE build callbacks */
#ifndef MAXIEBUILDCBS
#define MAXIEBUILDCBS 176
#endif // endif

/* Maximum # of Vendor Specif IE build callbacks */
#ifndef MAXVSIEBUILDCBS
#define MAXVSIEBUILDCBS 64
#endif // endif

/* Maximum # of IE parse callbacks */
#ifndef MAXIEPARSECBS
#define MAXIEPARSECBS 98
#endif // endif

/* Maximum # of Vendor Specific IE parse callbacks */
#ifndef MAXVSIEPARSECBS
#define MAXVSIEPARSECBS 48
#endif // endif

/* Maximum # of IE registries */
#ifndef MAXIEREGS
#define MAXIEREGS 8
#endif // endif

#ifdef PROP_TXSTATUS
/* FIFO credit values given to host */
#ifndef WLFCFIFOCREDITAC0
#define WLFCFIFOCREDITAC0 2
#endif // endif

#ifndef WLFCFIFOCREDITAC1
#if defined(BCMTPOPT_TXMID)  /* memredux14 */
#define WLFCFIFOCREDITAC1 10
#elif defined(BCMTPOPT_TXHI) /* memredux16 */
#define WLFCFIFOCREDITAC1 12
#else  /* memredux or default */
#define WLFCFIFOCREDITAC1 8
#endif // endif
#endif /* WLFCFIFOCREDITAC1 */

#ifndef WLFCFIFOCREDITAC2
#define WLFCFIFOCREDITAC2 2
#endif // endif

#ifndef WLFCFIFOCREDITAC3
#define WLFCFIFOCREDITAC3 2
#endif // endif

#ifndef WLFCFIFOCREDITBCMC
#define WLFCFIFOCREDITBCMC 2
#endif // endif

#ifndef WLFCFIFOCREDITOTHER
#define WLFCFIFOCREDITOTHER 2
#endif // endif

#ifndef WLFC_INDICATION_TRIGGER
#define WLFC_INDICATION_TRIGGER 1	/* WLFC_CREDIT_TRIGGER or WLFC_TXSTATUS_TRIGGER */
#endif // endif

/* total credits to max pending credits ratio in borrow case */
#ifndef WLFC_FIFO_BO_CR_RATIO
#define WLFC_FIFO_BO_CR_RATIO 3 /* pending cr thresh = total_credits/WLFC_FIFO_BO_CR_RATIO */
#endif // endif

/* Pending Compressed Txstatus Thresholds */
#ifndef WLFC_COMP_TXSTATUS_THRESH
#define WLFC_COMP_TXSTATUS_THRESH 8
#endif // endif

/* FIFO Credit Pending Thresholds */
#ifndef WLFC_FIFO_CR_PENDING_THRESH_AC_BK
#define WLFC_FIFO_CR_PENDING_THRESH_AC_BK 2
#endif // endif

#ifndef WLFC_FIFO_CR_PENDING_THRESH_AC_BE
#define WLFC_FIFO_CR_PENDING_THRESH_AC_BE 4
#endif // endif

#ifndef WLFC_FIFO_CR_PENDING_THRESH_AC_VI
#define WLFC_FIFO_CR_PENDING_THRESH_AC_VI 3
#endif // endif

#ifndef WLFC_FIFO_CR_PENDING_THRESH_AC_VO
#define WLFC_FIFO_CR_PENDING_THRESH_AC_VO 2
#endif // endif

#ifndef WLFC_FIFO_CR_PENDING_THRESH_BCMC
#define WLFC_FIFO_CR_PENDING_THRESH_BCMC  2
#endif // endif
#endif /* PROP_TXSTATUS */

#if !defined(WLC_DISABLE_DFS_RADAR_SUPPORT)
/* Radar support AP, STA */
#if defined(WL11H) && defined(BAND5G)
#define RADAR
#endif /* WL11H && BAND5G */
/* DFS AP, STA */
#if (defined(AP) || defined(SLAVE_RADAR)) && defined(RADAR)
#define WLDFS
#endif /* AP && RADAR */
#if defined(RSDB_DFS_SCAN) && defined(BGDFS)
#error "RSDB_DFS_SCAN and BGDFS are mutually exclusive features (at present)"
#endif // endif
#endif /* !WLC_DISABLE_DFS_RADAR_SUPPORT */

#if defined(BAND5G)
#define BAND_5G(bt)	((bt) == WLC_BAND_5G)
#else
#define BAND_5G(bt)	((void)(bt), 0)
#endif // endif

#if defined(BAND2G)
#define BAND_2G(bt)	((bt) == WLC_BAND_2G)
#else
#define BAND_2G(bt)	((void)(bt), 0)
#endif // endif

/* Some phy initialization code/data can't be reclaimed in dualband mode */
#if defined(DBAND) || LPCONF_HAS(0)
#define WLBANDINITDATA(_data)	_data
#define WLBANDINITFN(_fn)	_fn
#else
#define WLBANDINITDATA(_data)	BCMINITDATA(_data)
#define WLBANDINITFN(_fn)	BCMINITFN(_fn)
#endif // endif

/* FIPS support */
#ifdef WLFIPS
#define FIPS_ENAB(wlc) ((wlc)->cfg->wsec & FIPS_ENABLED)
#else
#define FIPS_ENAB(wlc) 0
#endif // endif

#ifdef WLPLT
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLPLT_ENAB(wlc_pub)     (wlc_pub->_plt)
	#elif defined(WLPLT_ENABLED)
		#define WLPLT_ENAB(wlc_pub)     1
	#else
		#define WLPLT_ENAB(wlc_pub)     0
	#endif
#else
	#define WLPLT_ENAB(wlc_pub)     0
#endif /* WLPLT */

#ifdef WLANTSEL
#define WLANTSEL_ENAB(wlc)	1
#else
#define WLANTSEL_ENAB(wlc)	0
#endif /* WLANTSEL */

#ifdef BCMWAPI_WPI
#define WAPI_HW_WPI_CAP(wlc)       D11REV_GE((wlc)->pub->corerev, 40)
#else
#define WAPI_HW_WPI_CAP(wlc)       0
#endif // endif

/* sdio-bmac */
#if defined(BCMSDIO) && defined(BCMDBUS) && defined(WLC_HIGH) && !defined(WLC_LOW)
#define OSLREGOPS
#endif // endif

#if D11CONF_GE(40)
	#if ACCONF || ACCONF2
	#define D11AC_TXD
	#endif	/* ACCONF || ACCONF2 */
#define WLTOEHW		/* TOE should always be on for all chips with D11 coreref >= 40 */
#endif /* D11CONF >= 40 */

#ifdef BAND5G
#define WL_NUMCHANSPECS_5G_20	29
#define WL_NUMCHANSPECS_5G_2P5	WL_NUMCHANSPECS_5G_20
#define WL_NUMCHANSPECS_5G_5	WL_NUMCHANSPECS_5G_20
#define WL_NUMCHANSPECS_5G_10	WL_NUMCHANSPECS_5G_20
#ifdef WL11N
#define WL_NUMCHANSPECS_5G_40	24
#else
#define WL_NUMCHANSPECS_5G_40	0
#endif /* WL11N */
#ifdef WL11AC
#define WL_NUMCHANSPECS_5G_80	24
#else
#define WL_NUMCHANSPECS_5G_80	0
#endif /* WL11AC */
#ifdef WL11AC
#define WL_NUMCHANSPECS_5G_8080	96
#else
#define WL_NUMCHANSPECS_5G_8080	0
#endif /* WL11AC */
#ifdef WL11AC
#define WL_NUMCHANSPECS_5G_160 16
#else
#define WL_NUMCHANSPECS_5G_160	0
#endif /* WL11AC */
#else
#define WL_NUMCHANSPECS_5G_20	0
#define WL_NUMCHANSPECS_5G_2P5	0
#define WL_NUMCHANSPECS_5G_5	0
#define WL_NUMCHANSPECS_5G_10	0
#define WL_NUMCHANSPECS_5G_40	0
#define WL_NUMCHANSPECS_5G_80	0
#define WL_NUMCHANSPECS_5G_8080	0
#define WL_NUMCHANSPECS_5G_160	0
#endif /* band5g */

#ifdef BAND2G
#define WL_NUMCHANSPECS_2G_20	14
#define WL_NUMCHANSPECS_2G_2P5	WL_NUMCHANSPECS_2G_20
#define WL_NUMCHANSPECS_2G_5	WL_NUMCHANSPECS_2G_20
#define WL_NUMCHANSPECS_2G_10	WL_NUMCHANSPECS_2G_20
#ifdef WL11N
#define WL_NUMCHANSPECS_2G_40	18
#else
#define WL_NUMCHANSPECS_2G_40	0
#endif /* WL11N */
#else
#define WL_NUMCHANSPECS_2G_20	0
#define WL_NUMCHANSPECS_2G_2P5	0
#define WL_NUMCHANSPECS_2G_5	0
#define WL_NUMCHANSPECS_2G_10	0
#define WL_NUMCHANSPECS_2G_40	0
#endif /* band 2g */

#ifdef WL11ULB
#define WL_NUMCHANSPECS_2G_ULB	(WL_NUMCHANSPECS_2G_2P5 + WL_NUMCHANSPECS_2G_5 +\
		WL_NUMCHANSPECS_2G_10)
#define WL_NUMCHANSPECS_5G_ULB	(WL_NUMCHANSPECS_5G_2P5 + WL_NUMCHANSPECS_5G_5 +\
		WL_NUMCHANSPECS_5G_10)
#else
#define WL_NUMCHANSPECS_2G_ULB	0
#define WL_NUMCHANSPECS_5G_ULB	0
#endif // endif

#define WL_NUMCHANSPECS_2G (WL_NUMCHANSPECS_2G_20 + WL_NUMCHANSPECS_2G_40 +\
		WL_NUMCHANSPECS_2G_ULB)

#define WL_NUMCHANSPECS_5G (WL_NUMCHANSPECS_5G_20 + WL_NUMCHANSPECS_5G_40 +\
		WL_NUMCHANSPECS_5G_80 + WL_NUMCHANSPECS_5G_8080 +\
		WL_NUMCHANSPECS_5G_160 + WL_NUMCHANSPECS_5G_ULB)

/* max number of chanspecs include 2g and 5g */
#define WL_NUMCHANSPEC_ALL (WL_NUMCHANSPECS_2G + WL_NUMCHANSPECS_5G)

/* Wave2 devices */
#define IS_AC2_DEV(d) (((d) == BCM4366_D11AC_ID) || \
	                 ((d) == BCM4366_D11AC2G_ID) || \
	                 ((d) == BCM4366_D11AC5G_ID) || \
	                 ((d) == BCM4365_D11AC_ID) || \
	                 ((d) == BCM4365_D11AC2G_ID) || \
	                 ((d) == BCM4365_D11AC5G_ID))

#define IS_DEV_AC3X3(d) (((d) == BCM4360_D11AC_ID) || \
	                 ((d) == BCM4360_D11AC2G_ID) || \
	                 ((d) == BCM4360_D11AC5G_ID) || \
	                 ((d) == BCM43602_D11AC_ID) || \
	                 ((d) == BCM43602_D11AC2G_ID) || \
	                 ((d) == BCM43602_D11AC5G_ID))

#define IS_DEV_AC2X2(d) (((d) == BCM4352_D11AC_ID) ||	\
	                 ((d) == BCM4352_D11AC2G_ID) || \
	                 ((d) == BCM4352_D11AC5G_ID) || \
	                 ((d) == BCM4350_D11AC_ID) || \
	                 ((d) == BCM4350_D11AC2G_ID) || \
	                 ((d) == BCM4350_D11AC5G_ID) || \
	                 ((d) == BCM43556_D11AC_ID) || \
	                 ((d) == BCM43556_D11AC2G_ID) || \
	                 ((d) == BCM43556_D11AC5G_ID) || \
	                 ((d) == BCM43558_D11AC_ID) || \
	                 ((d) == BCM43558_D11AC2G_ID) || \
	                 ((d) == BCM43558_D11AC5G_ID) || \
	                 ((d) == BCM43566_D11AC_ID) || \
	                 ((d) == BCM43566_D11AC2G_ID) || \
	                 ((d) == BCM43566_D11AC5G_ID) || \
	                 ((d) == BCM43568_D11AC_ID) || \
	                 ((d) == BCM43568_D11AC2G_ID) || \
	                 ((d) == BCM43568_D11AC5G_ID) || \
	                 ((d) == BCM43569_D11AC_ID) || \
	                 ((d) == BCM43569_D11AC2G_ID) || \
	                 ((d) == BCM43569_D11AC5G_ID) || \
	                 ((d) == BCM4354_D11AC_ID) || \
	                 ((d) == BCM4354_D11AC2G_ID) || \
	                 ((d) == BCM4354_D11AC5G_ID) || \
	                 ((d) == BCM4356_D11AC_ID) || \
	                 ((d) == BCM4356_D11AC2G_ID) || \
	                 ((d) == BCM4356_D11AC5G_ID) || \
	                 ((d) == BCM4358_D11AC_ID) || \
	                 ((d) == BCM4358_D11AC2G_ID) || \
	                 ((d) == BCM4358_D11AC5G_ID))

/* Airtime fairness */
#ifdef WLATF
#ifndef WLC_ATF_ENABLE_DEFAULT
#define WLC_ATF_ENABLE_DEFAULT	0
#endif /* WLC_ATF_ENABLE_DEFAULT */
#endif /* WLATF */
#ifndef NTXD_LFRAG
#define NTXD_LFRAG	1024
#endif // endif
#ifndef NRXD_FIFO1
#define NRXD_FIFO1	32
#endif // endif
#ifndef NRXD_CLASSIFIED_FIFO
#define NRXD_CLASSIFIED_FIFO 32
#endif // endif

#ifndef NRXBUFPOST_CLASSIFIED_FIFO
#define NRXBUFPOST_CLASSIFIED_FIFO 6
#endif // endif

#ifndef NRXBUFPOST_FIFO1
#define NRXBUFPOST_FIFO1	6
#endif // endif

#ifndef	NRXBUFPOST_FIFO2
#define NRXBUFPOST_FIFO2	6
#endif // endif
#ifndef PKT_CLASSIFY_FIFO
#define PKT_CLASSIFY_FIFO 2
#endif // endif

#ifndef SPLIT_RXMODE
#define SPLIT_RXMODE 0
#endif // endif
#ifndef COPY_CNT_BYTES
#define COPY_CNT_BYTES	64
#endif // endif

#ifndef MIN_SCBALLOC_MEM
#define MIN_SCBALLOC_MEM	0
#endif // endif

/* Derive WL_MACDBG */
#if defined(BCM_MACDBG) || defined(BCMDBG)
#define WL_MACDBG 1
#else
#define WL_MACDBG 0
#endif /* defined(BCM_MACDBG) || defined(BCMDBG) */

#endif /* _wlc_cfg_h_ */
