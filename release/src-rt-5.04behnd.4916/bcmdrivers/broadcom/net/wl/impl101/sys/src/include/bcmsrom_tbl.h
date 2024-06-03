/*
 * Table that encodes the srom formats for PCI/PCIe NICs.
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: bcmsrom_tbl.h 831783 2023-10-25 23:45:39Z $
 */

#ifndef	_bcmsrom_tbl_h_
#define	_bcmsrom_tbl_h_

#include "sbpcmcia.h"
#include "wlioctl.h"
#include <bcmsrom_fmt.h>

typedef struct {
	const char *name;
	uint32	revmask;
	uint32	flags;
	uint16	off; /**< word (uint16) offset starting at virtual SROM13 PCIe header */
	uint16	mask;
} sromvar_t;

#define SRFL_MORE	1		/* value continues as described by the next entry */
#define	SRFL_NOFFS	2		/* value bits can't be all one's */
#define	SRFL_PRHEX	4		/* value is in hexdecimal format */
#define	SRFL_PRSIGN	8		/* value is in signed decimal format */
#define	SRFL_CCODE	0x10		/* value is in country code format */
#define	SRFL_ETHADDR	0x20		/* value is an Ethernet address */
#define SRFL_LEDDC	0x40		/* value is an LED duty cycle */
#define SRFL_NOVAR	0x80		/* do not generate a nvram param, entry is for mfgc */
#define SRFL_ARRAY	0x100		/* value is in an array. All elements EXCEPT FOR THE LAST
					 * ONE in the array should have this flag set.
					 */
#define PRHEX_N_MORE (SRFL_PRHEX | SRFL_MORE)
#define SROM_DEVID_PCIE	48

/**
 * Assumptions:
 * - Ethernet address spans across 3 consecutive words
 *
 * Table rules:
 * - Add multiple entries next to each other if a value spans across multiple words
 *   (even multiple fields in the same word) with each entry except the last having
 *   it's SRFL_MORE bit set.
 * - Ethernet address entry does not follow above rule and must not have SRFL_MORE
 *   bit set. Its SRFL_ETHADDR bit implies it takes multiple words.
 * - The last entry's name field must be NULL to indicate the end of the table. Other
 *   entries must have non-NULL name.
 */
#if !defined(SROM15_MEMOPT)
static const sromvar_t BCMATTACHDATA(pci_sromvars)[] = {
/*	name		revmask		flags		off			mask */
#if defined(CABLECPE)
	{"devid",	0xffffff00,	SRFL_PRHEX,	PCI_F0DEVID,		0xffff},
#elif defined(BCMPCIEDEV) && defined(BCMPCIEDEV_ENABLED)
	{"devid",	0xffffff00,	SRFL_PRHEX, SROM_DEVID_PCIE,		0xffff},
#else
	{"devid",	0xffffff00,	SRFL_PRHEX|SRFL_NOVAR,	PCI_F0DEVID,	0xffff},
#endif /* CABLECPE */
	{"boardrev",	0x0000000e,	SRFL_PRHEX,	SROM_AABREV,		SROM_BR_MASK},
	{"boardrev",	0x000000f0,	SRFL_PRHEX,	SROM4_BREV,		0xffff},
	{"boardrev",	0xffffff00,	SRFL_PRHEX,	SROM8_BREV,		0xffff},
	{"boardflags",	0x00000002,	SRFL_PRHEX,	SROM_BFL,		0xffff},
	{"boardflags",	0x00000004,	SRFL_PRHEX|SRFL_MORE,	SROM_BFL,	0xffff},
	{"",		0,		0,		SROM_BFL2,		0xffff},
	{"boardflags",	0x00000008,	SRFL_PRHEX|SRFL_MORE,	SROM_BFL,	0xffff},
	{"",		0,		0,		SROM3_BFL2,		0xffff},
	{"boardflags",	0x00000010,	SRFL_PRHEX|SRFL_MORE,	SROM4_BFL0,	0xffff},
	{"",		0,		0,		SROM4_BFL1,		0xffff},
	{"boardflags",	0x000000e0,	SRFL_PRHEX|SRFL_MORE,	SROM5_BFL0,	0xffff},
	{"",		0,		0,		SROM5_BFL1,		0xffff},
	{"boardflags",	0xffffff00,	SRFL_PRHEX|SRFL_MORE,	SROM8_BFL0,	0xffff},
	{"",		0,		0,		SROM8_BFL1,		0xffff},
	{"boardflags2", 0x00000010,	SRFL_PRHEX|SRFL_MORE,	SROM4_BFL2,	0xffff},
	{"",		0,		0,		SROM4_BFL3,		0xffff},
	{"boardflags2", 0x000000e0,	SRFL_PRHEX|SRFL_MORE,	SROM5_BFL2,	0xffff},
	{"",		0,		0,		SROM5_BFL3,		0xffff},
	{"boardflags2", 0xffffff00,	SRFL_PRHEX|SRFL_MORE,	SROM8_BFL2,	0xffff},
	{"",		0,		0,		SROM8_BFL3,		0xffff},
	{"boardtype",	0xfffffffc,	SRFL_PRHEX,	SROM_SSID,		0xffff},
	{"subvid",	0xfffffffc,	SRFL_PRHEX,	SROM_SVID,		0xffff},
	{"boardnum",	0x00000006,	0,		SROM_MACLO_IL0,		0xffff},
	{"boardnum",	0x00000008,	0,		SROM3_MACLO,		0xffff},
	{"boardnum",	0x00000010,	0,		SROM4_MACLO,		0xffff},
	{"boardnum",	0x000000e0,	0,		SROM5_MACLO,		0xffff},
	{"boardnum",	0x00000700,	0,		SROM8_MACLO,		0xffff},
	{"cc",		0x00000002,	0,		SROM_AABREV,		SROM_CC_MASK},
	{"regrev",	0x00000008,	0,		SROM_OPO,		0xff00},
	{"regrev",	0x00000010,	0,		SROM4_REGREV,		0xffff},
	{"regrev",	0x000000e0,	0,		SROM5_REGREV,		0xffff},
	{"regrev",	0x00000700,	0,		SROM8_REGREV,		0xffff},
	{"ledbh0",	0x0000000e,	SRFL_NOFFS,	SROM_LEDBH10,		0x00ff},
	{"ledbh1",	0x0000000e,	SRFL_NOFFS,	SROM_LEDBH10,		0xff00},
	{"ledbh2",	0x0000000e,	SRFL_NOFFS,	SROM_LEDBH32,		0x00ff},
	{"ledbh3",	0x0000000e,	SRFL_NOFFS,	SROM_LEDBH32,		0xff00},
	{"ledbh0",	0x00000010,	SRFL_NOFFS,	SROM4_LEDBH10,		0x00ff},
	{"ledbh1",	0x00000010,	SRFL_NOFFS,	SROM4_LEDBH10,		0xff00},
	{"ledbh2",	0x00000010,	SRFL_NOFFS,	SROM4_LEDBH32,		0x00ff},
	{"ledbh3",	0x00000010,	SRFL_NOFFS,	SROM4_LEDBH32,		0xff00},
	{"ledbh0",	0x000000e0,	SRFL_NOFFS,	SROM5_LEDBH10,		0x00ff},
	{"ledbh1",	0x000000e0,	SRFL_NOFFS,	SROM5_LEDBH10,		0xff00},
	{"ledbh2",	0x000000e0,	SRFL_NOFFS,	SROM5_LEDBH32,		0x00ff},
	{"ledbh3",	0x000000e0,	SRFL_NOFFS,	SROM5_LEDBH32,		0xff00},
	{"ledbh0",	0x00000700,	SRFL_NOFFS,	SROM8_LEDBH10,		0x00ff},
	{"ledbh1",	0x00000700,	SRFL_NOFFS,	SROM8_LEDBH10,		0xff00},
	{"ledbh2",	0x00000700,	SRFL_NOFFS,	SROM8_LEDBH32,		0x00ff},
	{"ledbh3",	0x00000700,	SRFL_NOFFS,	SROM8_LEDBH32,		0xff00},
	{"pa0b0",	0x0000000e,	SRFL_PRHEX,	SROM_WL0PAB0,		0xffff},
	{"pa0b1",	0x0000000e,	SRFL_PRHEX,	SROM_WL0PAB1,		0xffff},
	{"pa0b2",	0x0000000e,	SRFL_PRHEX,	SROM_WL0PAB2,		0xffff},
	{"pa0itssit",	0x0000000e,	0,		SROM_ITT,		0x00ff},
	{"pa0maxpwr",	0x0000000e,	0,		SROM_WL10MAXP,		0x00ff},
	{"pa0b0",	0x00000700,	SRFL_PRHEX,	SROM8_W0_PAB0,		0xffff},
	{"pa0b1",	0x00000700,	SRFL_PRHEX,	SROM8_W0_PAB1,		0xffff},
	{"pa0b2",	0x00000700,	SRFL_PRHEX,	SROM8_W0_PAB2,		0xffff},
	{"pa0itssit",	0x00000700,	0,		SROM8_W0_ITTMAXP,	0xff00},
	{"pa0maxpwr",	0x00000700,	0,		SROM8_W0_ITTMAXP,	0x00ff},
	{"opo",		0x0000000c,	0,		SROM_OPO,		0x00ff},
	{"opo",		0x00000700,	0,		SROM8_2G_OFDMPO,	0x00ff},
	{"aa2g",	0x0000000e,	0,		SROM_AABREV,		SROM_AA0_MASK},
	{"aa2g",	0x000000f0,	0,		SROM4_AA,		0x00ff},
	{"aa2g",	0x00000700,	0,		SROM8_AA,		0x00ff},
	{"aa5g",	0x0000000e,	0,		SROM_AABREV,		SROM_AA1_MASK},
	{"aa5g",	0x000000f0,	0,		SROM4_AA,		0xff00},
	{"aa5g",	0x00000700,	0,		SROM8_AA,		0xff00},
	{"ag0",		0x0000000e,	0,		SROM_AG10,		0x00ff},
	{"ag1",		0x0000000e,	0,		SROM_AG10,		0xff00},
	{"ag0",		0x000000f0,	0,		SROM4_AG10,		0x00ff},
	{"ag1",		0x000000f0,	0,		SROM4_AG10,		0xff00},
	{"ag2",		0x000000f0,	0,		SROM4_AG32,		0x00ff},
	{"ag3",		0x000000f0,	0,		SROM4_AG32,		0xff00},
	{"ag0",		0x00000700,	0,		SROM8_AG10,		0x00ff},
	{"ag1",		0x00000700,	0,		SROM8_AG10,		0xff00},
	{"ag2",		0x00000700,	0,		SROM8_AG32,		0x00ff},
	{"ag3",		0x00000700,	0,		SROM8_AG32,		0xff00},
	{"pa1b0",	0x0000000e,	SRFL_PRHEX,	SROM_WL1PAB0,		0xffff},
	{"pa1b1",	0x0000000e,	SRFL_PRHEX,	SROM_WL1PAB1,		0xffff},
	{"pa1b2",	0x0000000e,	SRFL_PRHEX,	SROM_WL1PAB2,		0xffff},
	{"pa1lob0",	0x0000000c,	SRFL_PRHEX,	SROM_WL1LPAB0,		0xffff},
	{"pa1lob1",	0x0000000c,	SRFL_PRHEX,	SROM_WL1LPAB1,		0xffff},
	{"pa1lob2",	0x0000000c,	SRFL_PRHEX,	SROM_WL1LPAB2,		0xffff},
	{"pa1hib0",	0x0000000c,	SRFL_PRHEX,	SROM_WL1HPAB0,		0xffff},
	{"pa1hib1",	0x0000000c,	SRFL_PRHEX,	SROM_WL1HPAB1,		0xffff},
	{"pa1hib2",	0x0000000c,	SRFL_PRHEX,	SROM_WL1HPAB2,		0xffff},
	{"pa1itssit",	0x0000000e,	0,		SROM_ITT,		0xff00},
	{"pa1maxpwr",	0x0000000e,	0,		SROM_WL10MAXP,		0xff00},
	{"pa1lomaxpwr",	0x0000000c,	0,		SROM_WL1LHMAXP,		0xff00},
	{"pa1himaxpwr",	0x0000000c,	0,		SROM_WL1LHMAXP,		0x00ff},
	{"pa1b0",	0x00000700,	SRFL_PRHEX,	SROM8_W1_PAB0,		0xffff},
	{"pa1b1",	0x00000700,	SRFL_PRHEX,	SROM8_W1_PAB1,		0xffff},
	{"pa1b2",	0x00000700,	SRFL_PRHEX,	SROM8_W1_PAB2,		0xffff},
	{"pa1lob0",	0x00000700,	SRFL_PRHEX,	SROM8_W1_PAB0_LC,	0xffff},
	{"pa1lob1",	0x00000700,	SRFL_PRHEX,	SROM8_W1_PAB1_LC,	0xffff},
	{"pa1lob2",	0x00000700,	SRFL_PRHEX,	SROM8_W1_PAB2_LC,	0xffff},
	{"pa1hib0",	0x00000700,	SRFL_PRHEX,	SROM8_W1_PAB0_HC,	0xffff},
	{"pa1hib1",	0x00000700,	SRFL_PRHEX,	SROM8_W1_PAB1_HC,	0xffff},
	{"pa1hib2",	0x00000700,	SRFL_PRHEX,	SROM8_W1_PAB2_HC,	0xffff},
	{"pa1itssit",	0x00000700,	0,		SROM8_W1_ITTMAXP,	0xff00},
	{"pa1maxpwr",	0x00000700,	0,		SROM8_W1_ITTMAXP,	0x00ff},
	{"pa1lomaxpwr",	0x00000700,	0,		SROM8_W1_MAXP_LCHC,	0xff00},
	{"pa1himaxpwr",	0x00000700,	0,		SROM8_W1_MAXP_LCHC,	0x00ff},
	{"bxa2g",	0x00000008,	0,		SROM_BXARSSI2G,		0x1800},
	{"rssisav2g",	0x00000008,	0,		SROM_BXARSSI2G,		0x0700},
	{"rssismc2g",	0x00000008,	0,		SROM_BXARSSI2G,		0x00f0},
	{"rssismf2g",	0x00000008,	0,		SROM_BXARSSI2G,		0x000f},
	{"bxa2g",	0x00000700,	0,		SROM8_BXARSSI2G,	0x1800},
	{"rssisav2g",	0x00000700,	0,		SROM8_BXARSSI2G,	0x0700},
	{"rssismc2g",	0x00000700,	0,		SROM8_BXARSSI2G,	0x00f0},
	{"rssismf2g",	0x00000700,	0,		SROM8_BXARSSI2G,	0x000f},
	{"bxa5g",	0x00000008,	0,		SROM_BXARSSI5G,		0x1800},
	{"rssisav5g",	0x00000008,	0,		SROM_BXARSSI5G,		0x0700},
	{"rssismc5g",	0x00000008,	0,		SROM_BXARSSI5G,		0x00f0},
	{"rssismf5g",	0x00000008,	0,		SROM_BXARSSI5G,		0x000f},
	{"bxa5g",	0x00000700,	0,		SROM8_BXARSSI5G,	0x1800},
	{"rssisav5g",	0x00000700,	0,		SROM8_BXARSSI5G,	0x0700},
	{"rssismc5g",	0x00000700,	0,		SROM8_BXARSSI5G,	0x00f0},
	{"rssismf5g",	0x00000700,	0,		SROM8_BXARSSI5G,	0x000f},
	{"tri2g",	0x00000008,	0,		SROM_TRI52G,		0x00ff},
	{"tri5g",	0x00000008,	0,		SROM_TRI52G,		0xff00},
	{"tri5gl",	0x00000008,	0,		SROM_TRI5GHL,		0x00ff},
	{"tri5gh",	0x00000008,	0,		SROM_TRI5GHL,		0xff00},
	{"tri2g",	0x00000700,	0,		SROM8_TRI52G,		0x00ff},
	{"tri5g",	0x00000700,	0,		SROM8_TRI52G,		0xff00},
	{"tri5gl",	0x00000700,	0,		SROM8_TRI5GHL,		0x00ff},
	{"tri5gh",	0x00000700,	0,		SROM8_TRI5GHL,		0xff00},
	{"rxpo2g",	0x00000008,	SRFL_PRSIGN,	SROM_RXPO52G,		0x00ff},
	{"rxpo5g",	0x00000008,	SRFL_PRSIGN,	SROM_RXPO52G,		0xff00},
	{"rxpo2g",	0x00000700,	SRFL_PRSIGN,	SROM8_RXPO52G,		0x00ff},
	{"rxpo5g",	0x00000700,	SRFL_PRSIGN,	SROM8_RXPO52G,		0xff00},
	{"txchain",	0x000000f0,	SRFL_NOFFS,	SROM4_TXRXC,		SROM4_TXCHAIN_MASK},
	{"rxchain",	0x000000f0,	SRFL_NOFFS,	SROM4_TXRXC,		SROM4_RXCHAIN_MASK},
	{"antswitch",	0x000000f0,	SRFL_NOFFS,	SROM4_TXRXC,		SROM4_SWITCH_MASK},
	{"txchain",	0x00000700,	SRFL_NOFFS,	SROM8_TXRXC,		SROM4_TXCHAIN_MASK},
	{"rxchain",	0x00000700,	SRFL_NOFFS,	SROM8_TXRXC,		SROM4_RXCHAIN_MASK},
	{"antswitch",	0x00000700,	SRFL_NOFFS,	SROM8_TXRXC,		SROM4_SWITCH_MASK},
	{"tssipos2g",	0x00000700,	0,		SROM8_FEM2G,	SROM8_FEM_TSSIPOS_MASK},
	{"extpagain2g",	0x00000700,	0,		SROM8_FEM2G,	SROM8_FEM_EXTPA_GAIN_MASK},
	{"pdetrange2g",	0x00000700,	0,		SROM8_FEM2G,	SROM8_FEM_PDET_RANGE_MASK},
	{"triso2g",	0x00000700,	0,		SROM8_FEM2G,	SROM8_FEM_TR_ISO_MASK},
	{"antswctl2g",	0x00000700,	0,		SROM8_FEM2G,	SROM8_FEM_ANTSWLUT_MASK},
	{"tssipos5g",	0x00000700,	0,		SROM8_FEM5G,	SROM8_FEM_TSSIPOS_MASK},
	{"extpagain5g",	0x00000700,	0,		SROM8_FEM5G,	SROM8_FEM_EXTPA_GAIN_MASK},
	{"pdetrange5g",	0x00000700,	0,		SROM8_FEM5G,	SROM8_FEM_PDET_RANGE_MASK},
	{"triso5g",	0x00000700,	0,		SROM8_FEM5G,	SROM8_FEM_TR_ISO_MASK},
	{"antswctl5g",	0x00000700,	0,		SROM8_FEM5G,	SROM8_FEM_ANTSWLUT_MASK},
	{"txpid2ga0",	0x000000f0,	0,		SROM4_TXPID2G,		0x00ff},
	{"txpid2ga1",	0x000000f0,	0,		SROM4_TXPID2G,		0xff00},
	{"txpid2ga2",	0x000000f0,	0,		SROM4_TXPID2G + 1,	0x00ff},
	{"txpid2ga3",	0x000000f0,	0,		SROM4_TXPID2G + 1,	0xff00},
	{"txpid5ga0",	0x000000f0,	0,		SROM4_TXPID5G,		0x00ff},
	{"txpid5ga1",	0x000000f0,	0,		SROM4_TXPID5G,		0xff00},
	{"txpid5ga2",	0x000000f0,	0,		SROM4_TXPID5G + 1,	0x00ff},
	{"txpid5ga3",	0x000000f0,	0,		SROM4_TXPID5G + 1,	0xff00},
	{"txpid5gla0",	0x000000f0,	0,		SROM4_TXPID5GL,		0x00ff},
	{"txpid5gla1",	0x000000f0,	0,		SROM4_TXPID5GL,		0xff00},
	{"txpid5gla2",	0x000000f0,	0,		SROM4_TXPID5GL + 1,	0x00ff},
	{"txpid5gla3",	0x000000f0,	0,		SROM4_TXPID5GL + 1,	0xff00},
	{"txpid5gha0",	0x000000f0,	0,		SROM4_TXPID5GH,		0x00ff},
	{"txpid5gha1",	0x000000f0,	0,		SROM4_TXPID5GH,		0xff00},
	{"txpid5gha2",	0x000000f0,	0,		SROM4_TXPID5GH + 1,	0x00ff},
	{"txpid5gha3",	0x000000f0,	0,		SROM4_TXPID5GH + 1,	0xff00},

	{"ccode",	0x0000000f,	SRFL_CCODE,	SROM_CCODE,		0xffff},
	{"ccode",	0x00000010,	SRFL_CCODE,	SROM4_CCODE,		0xffff},
	{"ccode",	0x000000e0,	SRFL_CCODE,	SROM5_CCODE,		0xffff},
	{"ccode",	0x00000700,	SRFL_CCODE,	SROM8_CCODE,		0xffff},
	{"macaddr",	0x00000700,	SRFL_ETHADDR,	SROM8_MACHI,		0xffff},
	{"macaddr",	0x000000e0,	SRFL_ETHADDR,	SROM5_MACHI,		0xffff},
	{"macaddr",	0x00000010,	SRFL_ETHADDR,	SROM4_MACHI,		0xffff},
	{"macaddr",	0x00000008,	SRFL_ETHADDR,	SROM3_MACHI,		0xffff},
	{"il0macaddr",	0x00000007,	SRFL_ETHADDR,	SROM_MACHI_IL0,		0xffff},
	{"et1macaddr",	0x00000007,	SRFL_ETHADDR,	SROM_MACHI_ET1,		0xffff},
	{"leddc",	0x00000700,	SRFL_NOFFS|SRFL_LEDDC,	SROM8_LEDDC,	0xffff},
	{"leddc",	0x000000e0,	SRFL_NOFFS|SRFL_LEDDC,	SROM5_LEDDC,	0xffff},
	{"leddc",	0x00000010,	SRFL_NOFFS|SRFL_LEDDC,	SROM4_LEDDC,	0xffff},
	{"leddc",	0x00000008,	SRFL_NOFFS|SRFL_LEDDC,	SROM3_LEDDC,	0xffff},

	{"tempthresh",	0x00000700,	0,		SROM8_THERMAL,		0xff00},
	{"tempoffset",	0x00000700,	0,		SROM8_THERMAL,		0x00ff},
	{"rawtempsense", 0x00000700,	SRFL_PRHEX,	SROM8_MPWR_RAWTS,	0x01ff},
	{"measpower",	0x00000700,	SRFL_PRHEX,	SROM8_MPWR_RAWTS,	0xfe00},
	{"tempsense_slope",	0x00000700,	SRFL_PRHEX, 	SROM8_TS_SLP_OPT_CORRX,	0x00ff},
	{"tempcorrx",	0x00000700,	SRFL_PRHEX, 	SROM8_TS_SLP_OPT_CORRX,	0xfc00},
	{"tempsense_option",	0x00000700,	SRFL_PRHEX,	SROM8_TS_SLP_OPT_CORRX,	0x0300},
	{"freqoffset_corr",	0x00000700,	SRFL_PRHEX,	SROM8_FOC_HWIQ_IQSWP,	0x000f},
	{"iqcal_swp_dis",	0x00000700,	SRFL_PRHEX,	SROM8_FOC_HWIQ_IQSWP,	0x0010},
	{"hw_iqcal_en",	0x00000700,	SRFL_PRHEX,	SROM8_FOC_HWIQ_IQSWP,	0x0020},
	{"elna2g",      0x00000700,     0,              SROM8_EXTLNAGAIN,       0x00ff},
	{"elna5g",      0x00000700,     0,              SROM8_EXTLNAGAIN,       0xff00},
	{"phycal_tempdelta",	0x00000700,	0,	SROM8_PHYCAL_TEMPDELTA,	0x00ff},
	{"temps_period",	0x00000700,	0,	SROM8_PHYCAL_TEMPDELTA,	0x0f00},
	{"temps_hysteresis",	0x00000700,	0,	SROM8_PHYCAL_TEMPDELTA,	0xf000},
	{"measpower1", 0x00000700,	SRFL_PRHEX, SROM8_MPWR_1_AND_2, 	0x007f},
	{"measpower2",	0x00000700, 	SRFL_PRHEX, SROM8_MPWR_1_AND_2, 	0x3f80},

	{"cck2gpo",	0x000000f0,	0,		SROM4_2G_CCKPO,		0xffff},
	{"cck2gpo",	0x00000100,	0,		SROM8_2G_CCKPO,		0xffff},
	{"ofdm2gpo",	0x000000f0,	SRFL_MORE,	SROM4_2G_OFDMPO,	0xffff},
	{"",		0,		0,		SROM4_2G_OFDMPO + 1,	0xffff},
	{"ofdm5gpo",	0x000000f0,	SRFL_MORE,	SROM4_5G_OFDMPO,	0xffff},
	{"",		0,		0,		SROM4_5G_OFDMPO + 1,	0xffff},
	{"ofdm5glpo",	0x000000f0,	SRFL_MORE,	SROM4_5GL_OFDMPO,	0xffff},
	{"",		0,		0,		SROM4_5GL_OFDMPO + 1,	0xffff},
	{"ofdm5ghpo",	0x000000f0,	SRFL_MORE,	SROM4_5GH_OFDMPO,	0xffff},
	{"",		0,		0,		SROM4_5GH_OFDMPO + 1,	0xffff},
	{"ofdm2gpo",	0x00000100,	SRFL_MORE,	SROM8_2G_OFDMPO,	0xffff},
	{"",		0,		0,		SROM8_2G_OFDMPO + 1,	0xffff},
	{"ofdm5gpo",	0x00000100,	SRFL_MORE,	SROM8_5G_OFDMPO,	0xffff},
	{"",		0,		0,		SROM8_5G_OFDMPO + 1,	0xffff},
	{"ofdm5glpo",	0x00000100,	SRFL_MORE,	SROM8_5GL_OFDMPO,	0xffff},
	{"",		0,		0,		SROM8_5GL_OFDMPO + 1,	0xffff},
	{"ofdm5ghpo",	0x00000100,	SRFL_MORE,	SROM8_5GH_OFDMPO,	0xffff},
	{"",		0,		0,		SROM8_5GH_OFDMPO + 1,	0xffff},
	{"mcs2gpo0",	0x000000f0,	0,		SROM4_2G_MCSPO,		0xffff},
	{"mcs2gpo1",	0x000000f0,	0,		SROM4_2G_MCSPO + 1,	0xffff},
	{"mcs2gpo2",	0x000000f0,	0,		SROM4_2G_MCSPO + 2,	0xffff},
	{"mcs2gpo3",	0x000000f0,	0,		SROM4_2G_MCSPO + 3,	0xffff},
	{"mcs2gpo4",	0x000000f0,	0,		SROM4_2G_MCSPO + 4,	0xffff},
	{"mcs2gpo5",	0x000000f0,	0,		SROM4_2G_MCSPO + 5,	0xffff},
	{"mcs2gpo6",	0x000000f0,	0,		SROM4_2G_MCSPO + 6,	0xffff},
	{"mcs2gpo7",	0x000000f0,	0,		SROM4_2G_MCSPO + 7,	0xffff},
	{"mcs5gpo0",	0x000000f0,	0,		SROM4_5G_MCSPO,		0xffff},
	{"mcs5gpo1",	0x000000f0,	0,		SROM4_5G_MCSPO + 1,	0xffff},
	{"mcs5gpo2",	0x000000f0,	0,		SROM4_5G_MCSPO + 2,	0xffff},
	{"mcs5gpo3",	0x000000f0,	0,		SROM4_5G_MCSPO + 3,	0xffff},
	{"mcs5gpo4",	0x000000f0,	0,		SROM4_5G_MCSPO + 4,	0xffff},
	{"mcs5gpo5",	0x000000f0,	0,		SROM4_5G_MCSPO + 5,	0xffff},
	{"mcs5gpo6",	0x000000f0,	0,		SROM4_5G_MCSPO + 6,	0xffff},
	{"mcs5gpo7",	0x000000f0,	0,		SROM4_5G_MCSPO + 7,	0xffff},
	{"mcs5glpo0",	0x000000f0,	0,		SROM4_5GL_MCSPO,	0xffff},
	{"mcs5glpo1",	0x000000f0,	0,		SROM4_5GL_MCSPO + 1,	0xffff},
	{"mcs5glpo2",	0x000000f0,	0,		SROM4_5GL_MCSPO + 2,	0xffff},
	{"mcs5glpo3",	0x000000f0,	0,		SROM4_5GL_MCSPO + 3,	0xffff},
	{"mcs5glpo4",	0x000000f0,	0,		SROM4_5GL_MCSPO + 4,	0xffff},
	{"mcs5glpo5",	0x000000f0,	0,		SROM4_5GL_MCSPO + 5,	0xffff},
	{"mcs5glpo6",	0x000000f0,	0,		SROM4_5GL_MCSPO + 6,	0xffff},
	{"mcs5glpo7",	0x000000f0,	0,		SROM4_5GL_MCSPO + 7,	0xffff},
	{"mcs5ghpo0",	0x000000f0,	0,		SROM4_5GH_MCSPO,	0xffff},
	{"mcs5ghpo1",	0x000000f0,	0,		SROM4_5GH_MCSPO + 1,	0xffff},
	{"mcs5ghpo2",	0x000000f0,	0,		SROM4_5GH_MCSPO + 2,	0xffff},
	{"mcs5ghpo3",	0x000000f0,	0,		SROM4_5GH_MCSPO + 3,	0xffff},
	{"mcs5ghpo4",	0x000000f0,	0,		SROM4_5GH_MCSPO + 4,	0xffff},
	{"mcs5ghpo5",	0x000000f0,	0,		SROM4_5GH_MCSPO + 5,	0xffff},
	{"mcs5ghpo6",	0x000000f0,	0,		SROM4_5GH_MCSPO + 6,	0xffff},
	{"mcs5ghpo7",	0x000000f0,	0,		SROM4_5GH_MCSPO + 7,	0xffff},
	{"mcs2gpo0",	0x00000100,	0,		SROM8_2G_MCSPO,		0xffff},
	{"mcs2gpo1",	0x00000100,	0,		SROM8_2G_MCSPO + 1,	0xffff},
	{"mcs2gpo2",	0x00000100,	0,		SROM8_2G_MCSPO + 2,	0xffff},
	{"mcs2gpo3",	0x00000100,	0,		SROM8_2G_MCSPO + 3,	0xffff},
	{"mcs2gpo4",	0x00000100,	0,		SROM8_2G_MCSPO + 4,	0xffff},
	{"mcs2gpo5",	0x00000100,	0,		SROM8_2G_MCSPO + 5,	0xffff},
	{"mcs2gpo6",	0x00000100,	0,		SROM8_2G_MCSPO + 6,	0xffff},
	{"mcs2gpo7",	0x00000100,	0,		SROM8_2G_MCSPO + 7,	0xffff},
	{"mcs5gpo0",	0x00000100,	0,		SROM8_5G_MCSPO,		0xffff},
	{"mcs5gpo1",	0x00000100,	0,		SROM8_5G_MCSPO + 1,	0xffff},
	{"mcs5gpo2",	0x00000100,	0,		SROM8_5G_MCSPO + 2,	0xffff},
	{"mcs5gpo3",	0x00000100,	0,		SROM8_5G_MCSPO + 3,	0xffff},
	{"mcs5gpo4",	0x00000100,	0,		SROM8_5G_MCSPO + 4,	0xffff},
	{"mcs5gpo5",	0x00000100,	0,		SROM8_5G_MCSPO + 5,	0xffff},
	{"mcs5gpo6",	0x00000100,	0,		SROM8_5G_MCSPO + 6,	0xffff},
	{"mcs5gpo7",	0x00000100,	0,		SROM8_5G_MCSPO + 7,	0xffff},
	{"mcs5glpo0",	0x00000100,	0,		SROM8_5GL_MCSPO,	0xffff},
	{"mcs5glpo1",	0x00000100,	0,		SROM8_5GL_MCSPO + 1,	0xffff},
	{"mcs5glpo2",	0x00000100,	0,		SROM8_5GL_MCSPO + 2,	0xffff},
	{"mcs5glpo3",	0x00000100,	0,		SROM8_5GL_MCSPO + 3,	0xffff},
	{"mcs5glpo4",	0x00000100,	0,		SROM8_5GL_MCSPO + 4,	0xffff},
	{"mcs5glpo5",	0x00000100,	0,		SROM8_5GL_MCSPO + 5,	0xffff},
	{"mcs5glpo6",	0x00000100,	0,		SROM8_5GL_MCSPO + 6,	0xffff},
	{"mcs5glpo7",	0x00000100,	0,		SROM8_5GL_MCSPO + 7,	0xffff},
	{"mcs5ghpo0",	0x00000100,	0,		SROM8_5GH_MCSPO,	0xffff},
	{"mcs5ghpo1",	0x00000100,	0,		SROM8_5GH_MCSPO + 1,	0xffff},
	{"mcs5ghpo2",	0x00000100,	0,		SROM8_5GH_MCSPO + 2,	0xffff},
	{"mcs5ghpo3",	0x00000100,	0,		SROM8_5GH_MCSPO + 3,	0xffff},
	{"mcs5ghpo4",	0x00000100,	0,		SROM8_5GH_MCSPO + 4,	0xffff},
	{"mcs5ghpo5",	0x00000100,	0,		SROM8_5GH_MCSPO + 5,	0xffff},
	{"mcs5ghpo6",	0x00000100,	0,		SROM8_5GH_MCSPO + 6,	0xffff},
	{"mcs5ghpo7",	0x00000100,	0,		SROM8_5GH_MCSPO + 7,	0xffff},
	{"cddpo",	0x000000f0,	0,		SROM4_CDDPO,		0xffff},
	{"stbcpo",	0x000000f0,	0,		SROM4_STBCPO,		0xffff},
	{"bw40po",	0x000000f0,	0,		SROM4_BW40PO,		0xffff},
	{"bwduppo",	0x000000f0,	0,		SROM4_BWDUPPO,		0xffff},
	{"cddpo",	0x00000100,	0,		SROM8_CDDPO,		0xffff},
	{"stbcpo",	0x00000100,	0,		SROM8_STBCPO,		0xffff},
	{"bw40po",	0x00000100,	0,		SROM8_BW40PO,		0xffff},
	{"bwduppo",	0x00000100,	0,		SROM8_BWDUPPO,		0xffff},

	/* power per rate from sromrev 9 */
	{"cckbw202gpo",		0x00000600,	0,	SROM9_2GPO_CCKBW20,		0xffff},
	{"cckbw20ul2gpo",	0x00000600,	0,	SROM9_2GPO_CCKBW20UL,		0xffff},
	{"legofdmbw202gpo",	0x00000600,	SRFL_MORE, SROM9_2GPO_LOFDMBW20,	0xffff},
	{"",			0,		0,	SROM9_2GPO_LOFDMBW20 + 1,	0xffff},
	{"legofdmbw20ul2gpo",	0x00000600,	SRFL_MORE, SROM9_2GPO_LOFDMBW20UL,	0xffff},
	{"",			0,		0,	SROM9_2GPO_LOFDMBW20UL + 1,	0xffff},
	{"legofdmbw205glpo",	0x00000600,	SRFL_MORE, SROM9_5GLPO_LOFDMBW20,	0xffff},
	{"",			0,		0,	SROM9_5GLPO_LOFDMBW20 + 1,	0xffff},
	{"legofdmbw20ul5glpo",	0x00000600,	SRFL_MORE, SROM9_5GLPO_LOFDMBW20UL,	0xffff},
	{"",			0,		0,	SROM9_5GLPO_LOFDMBW20UL + 1,	0xffff},
	{"legofdmbw205gmpo",	0x00000600,	SRFL_MORE, SROM9_5GMPO_LOFDMBW20,	0xffff},
	{"",			0,		0,	SROM9_5GMPO_LOFDMBW20 + 1,	0xffff},
	{"legofdmbw20ul5gmpo",	0x00000600,	SRFL_MORE, SROM9_5GMPO_LOFDMBW20UL,	0xffff},
	{"",			0,		0,	SROM9_5GMPO_LOFDMBW20UL + 1,	0xffff},
	{"legofdmbw205ghpo",	0x00000600,	SRFL_MORE, SROM9_5GHPO_LOFDMBW20,	0xffff},
	{"",			0,		0,	SROM9_5GHPO_LOFDMBW20 + 1,	0xffff},
	{"legofdmbw20ul5ghpo",	0x00000600,	SRFL_MORE, SROM9_5GHPO_LOFDMBW20UL,	0xffff},
	{"",			0,		0,	SROM9_5GHPO_LOFDMBW20UL + 1,	0xffff},
	{"mcsbw202gpo",		0x00000600,	SRFL_MORE, SROM9_2GPO_MCSBW20,		0xffff},
	{"",			0,		0,	SROM9_2GPO_MCSBW20 + 1,		0xffff},
	{"mcsbw20ul2gpo",      	0x00000600,	SRFL_MORE, SROM9_2GPO_MCSBW20UL,	0xffff},
	{"",			0,		0,	SROM9_2GPO_MCSBW20UL + 1,	0xffff},
	{"mcsbw402gpo",		0x00000600,	SRFL_MORE, SROM9_2GPO_MCSBW40,		0xffff},
	{"",			0,		0,	SROM9_2GPO_MCSBW40 + 1,		0xffff},
	{"mcsbw205glpo",	0x00000600,	SRFL_MORE, SROM9_5GLPO_MCSBW20,		0xffff},
	{"",			0,		0,	SROM9_5GLPO_MCSBW20 + 1,	0xffff},
	{"mcsbw20ul5glpo",	0x00000600,	SRFL_MORE, SROM9_5GLPO_MCSBW20UL,	0xffff},
	{"",			0,		0,	SROM9_5GLPO_MCSBW20UL + 1,	0xffff},
	{"mcsbw405glpo",	0x00000600,	SRFL_MORE, SROM9_5GLPO_MCSBW40,		0xffff},
	{"",			0,		0,	SROM9_5GLPO_MCSBW40 + 1,	0xffff},
	{"mcsbw205gmpo",	0x00000600,	SRFL_MORE, SROM9_5GMPO_MCSBW20,		0xffff},
	{"",			0,		0,	SROM9_5GMPO_MCSBW20 + 1,	0xffff},
	{"mcsbw20ul5gmpo",	0x00000600,	SRFL_MORE, SROM9_5GMPO_MCSBW20UL,	0xffff},
	{"",			0,		0,	SROM9_5GMPO_MCSBW20UL + 1,	0xffff},
	{"mcsbw405gmpo",	0x00000600,	SRFL_MORE, SROM9_5GMPO_MCSBW40,		0xffff},
	{"",			0,		0,	SROM9_5GMPO_MCSBW40 + 1,	0xffff},
	{"mcsbw205ghpo",	0x00000600,	SRFL_MORE, SROM9_5GHPO_MCSBW20,		0xffff},
	{"",			0,		0,	SROM9_5GHPO_MCSBW20 + 1,	0xffff},
	{"mcsbw20ul5ghpo",	0x00000600,	SRFL_MORE, SROM9_5GHPO_MCSBW20UL,	0xffff},
	{"",			0,		0,	SROM9_5GHPO_MCSBW20UL + 1,	0xffff},
	{"mcsbw405ghpo",	0x00000600,	SRFL_MORE, SROM9_5GHPO_MCSBW40,		0xffff},
	{"",			0,		0,	SROM9_5GHPO_MCSBW40 + 1,	0xffff},
	{"mcs32po",		0x00000600,	0,	SROM9_PO_MCS32,			0xffff},
	{"legofdm40duppo",	0x00000600,	0,	SROM9_PO_LOFDM40DUP,	0xffff},
	{"pcieingress_war",	0x00000700,	0,	SROM8_PCIEINGRESS_WAR,	0xf},
	{"eu_edthresh2g",	0x00000100,	0,	SROM8_EU_EDCRSTH,		0x00ff},
	{"eu_edthresh5g",	0x00000100,	0,	SROM8_EU_EDCRSTH,		0xff00},
	{"eu_edthresh2g",	0x00000600,	0,	SROM9_EU_EDCRSTH,		0x00ff},
	{"eu_edthresh5g",	0x00000600,	0,	SROM9_EU_EDCRSTH,		0xff00},
	{"rxgainerr2ga0",	0x00000700,	0,	SROM8_RXGAINERR_2G,		0x003f},
	{"rxgainerr2ga0",	0x00000700,	0,	SROM8_RXGAINERR_2G,		0x003f},
	{"rxgainerr2ga1",	0x00000700,	0,	SROM8_RXGAINERR_2G,		0x07c0},
	{"rxgainerr2ga2",	0x00000700,	0,	SROM8_RXGAINERR_2G,		0xf800},
	{"rxgainerr5gla0",	0x00000700,	0,	SROM8_RXGAINERR_5GL,	0x003f},
	{"rxgainerr5gla1",	0x00000700,	0,	SROM8_RXGAINERR_5GL,	0x07c0},
	{"rxgainerr5gla2",	0x00000700,	0,	SROM8_RXGAINERR_5GL,	0xf800},
	{"rxgainerr5gma0",	0x00000700,	0,	SROM8_RXGAINERR_5GM,	0x003f},
	{"rxgainerr5gma1",	0x00000700,	0,	SROM8_RXGAINERR_5GM,	0x07c0},
	{"rxgainerr5gma2",	0x00000700,	0,	SROM8_RXGAINERR_5GM,	0xf800},
	{"rxgainerr5gha0",	0x00000700,	0,	SROM8_RXGAINERR_5GH,	0x003f},
	{"rxgainerr5gha1",	0x00000700,	0,	SROM8_RXGAINERR_5GH,	0x07c0},
	{"rxgainerr5gha2",	0x00000700,	0,	SROM8_RXGAINERR_5GH,	0xf800},
	{"rxgainerr5gua0",	0x00000700,	0,	SROM8_RXGAINERR_5GU,	0x003f},
	{"rxgainerr5gua1",	0x00000700,	0,	SROM8_RXGAINERR_5GU,	0x07c0},
	{"rxgainerr5gua2",	0x00000700,	0,	SROM8_RXGAINERR_5GU,	0xf800},
	{"sar2g",       	0x00000600,	0,	SROM9_SAR,          	0x00ff},
	{"sar5g",           0x00000600,	0,	SROM9_SAR,	            0xff00},
	{"noiselvl2ga0",	0x00000700,	0,	SROM8_NOISELVL_2G,		0x001f},
	{"noiselvl2ga1",	0x00000700,	0,	SROM8_NOISELVL_2G,		0x03e0},
	{"noiselvl2ga2",	0x00000700,	0,	SROM8_NOISELVL_2G,		0x7c00},
	{"noiselvl5gla0",	0x00000700,	0,	SROM8_NOISELVL_5GL,		0x001f},
	{"noiselvl5gla1",	0x00000700,	0,	SROM8_NOISELVL_5GL,		0x03e0},
	{"noiselvl5gla2",	0x00000700,	0,	SROM8_NOISELVL_5GL,		0x7c00},
	{"noiselvl5gma0",	0x00000700,	0,	SROM8_NOISELVL_5GM,		0x001f},
	{"noiselvl5gma1",	0x00000700,	0,	SROM8_NOISELVL_5GM,		0x03e0},
	{"noiselvl5gma2",	0x00000700,	0,	SROM8_NOISELVL_5GM,		0x7c00},
	{"noiselvl5gha0",	0x00000700,	0,	SROM8_NOISELVL_5GH,		0x001f},
	{"noiselvl5gha1",	0x00000700,	0,	SROM8_NOISELVL_5GH,		0x03e0},
	{"noiselvl5gha2",	0x00000700,	0,	SROM8_NOISELVL_5GH,		0x7c00},
	{"noiselvl5gua0",	0x00000700,	0,	SROM8_NOISELVL_5GU,		0x001f},
	{"noiselvl5gua1",	0x00000700,	0,	SROM8_NOISELVL_5GU,		0x03e0},
	{"noiselvl5gua2",	0x00000700,	0,	SROM8_NOISELVL_5GU,		0x7c00},
	{"noisecaloffset",	0x00000300,	0,	SROM8_NOISECALOFFSET,		0x00ff},
	{"noisecaloffset5g",	0x00000300,	0,	SROM8_NOISECALOFFSET,		0xff00},
	{"subband5gver",	0x00000700,	0,	SROM8_SUBBAND_PPR,		0x7},

	{"cckPwrOffset",	0x00000400,	0,	SROM10_CCKPWROFFSET,		0xffff},

	/* swctrlmap_2g array, note that the last element doesn't have SRFL_ARRAY flag set */
	{"swctrlmap_2g", 0x00000400, SRFL_MORE|SRFL_PRHEX|SRFL_ARRAY, SROM10_SWCTRLMAP_2G, 0xffff},
	{"",	0x00000400, SRFL_ARRAY,	SROM10_SWCTRLMAP_2G + 1,			0xffff},
	{"",	0x00000400, SRFL_MORE|SRFL_PRHEX|SRFL_ARRAY, SROM10_SWCTRLMAP_2G + 2, 	0xffff},
	{"",	0x00000400, SRFL_ARRAY,	SROM10_SWCTRLMAP_2G + 3,			0xffff},
	{"",	0x00000400, SRFL_MORE|SRFL_PRHEX|SRFL_ARRAY, SROM10_SWCTRLMAP_2G + 4,	0xffff},
	{"",	0x00000400, SRFL_ARRAY,	SROM10_SWCTRLMAP_2G + 5,			0xffff},
	{"",	0x00000400, SRFL_MORE|SRFL_PRHEX|SRFL_ARRAY, SROM10_SWCTRLMAP_2G + 6,	0xffff},
	{"",	0x00000400, SRFL_ARRAY,	SROM10_SWCTRLMAP_2G + 7,			0xffff},
	{"",	0x00000400, SRFL_PRHEX,	SROM10_SWCTRLMAP_2G + 8,			0xffff},

	/* sromrev 11 */
	{"boardflags3",	0xfffff800,	SRFL_PRHEX|SRFL_MORE,	SROM11_BFL4,	0xffff},
	{"",		0,		0,			SROM11_BFL5,	0xffff},
	{"boardnum",	0xfffff800,	0,			SROM11_MACLO,	0xffff},
	{"macaddr",	0xfffff800,	SRFL_ETHADDR,		SROM11_MACHI,	0xffff},
	{"ccode",	0xfffff800,	SRFL_CCODE,		SROM11_CCODE,	0xffff},
	{"regrev",	0xfffff800,	0,			SROM11_REGREV,	0xffff},
	{"ledbh0",	0xfffff800,	SRFL_NOFFS,		SROM11_LEDBH10,	0x00ff},
	{"ledbh1",	0xfffff800,	SRFL_NOFFS,		SROM11_LEDBH10,	0xff00},
	{"ledbh2",	0xfffff800,	SRFL_NOFFS,		SROM11_LEDBH32,	0x00ff},
	{"ledbh3",	0xfffff800,	SRFL_NOFFS,		SROM11_LEDBH32,	0xff00},
	{"leddc",	0xfffff800,	SRFL_NOFFS|SRFL_LEDDC,	SROM11_LEDDC,	0xffff},
	{"aa2g",	0xfffff800,	0,			SROM11_AA,	0x00ff},
	{"aa5g",	0xfffff800,	0,			SROM11_AA,	0xff00},
	{"agbg0",	0xfffff800,	0,			SROM11_AGBG10,  0xff00},
	{"agbg1",	0xfffff800,	0,			SROM11_AGBG10,	0x00ff},
	{"agbg2",	0xfffff800,	0,			SROM11_AGBG2A0,	0xff00},
	{"aga0",	0xfffff800,	0,			SROM11_AGBG2A0,	0x00ff},
	{"aga1",	0xfffff800,	0,			SROM11_AGA21,   0xff00},
	{"aga2",	0xfffff800,	0,			SROM11_AGA21,	0x00ff},
	{"txchain",	0xfffff800,	0,			SROM11_TXRXC,	SROM4_TXCHAIN_MASK},
	{"rxchain",	0xfffff800,	0,			SROM11_TXRXC,	SROM4_RXCHAIN_MASK},
	{"antswitch",	0xfffff800,	SRFL_NOFFS,	SROM11_TXRXC,	SROM4_SWITCH_MASK},

	{"tssiposslope2g",	0xfffff800,	0,		SROM11_FEM_CFG1, 	0x0001},
	{"epagain2g",		0xfffff800,	0,		SROM11_FEM_CFG1, 	0x000e},
	{"pdgain2g",		0xfffff800,	0,		SROM11_FEM_CFG1, 	0x01f0},
	{"tworangetssi2g",	0xfffff800,	0,		SROM11_FEM_CFG1, 	0x0200},
	{"papdcap2g",		0xfffff800,	0,		SROM11_FEM_CFG1, 	0x0400},
	{"femctrl",		0xfffff800,	0,		SROM11_FEM_CFG1, 	0xf800},

	{"tssiposslope5g",	0xfffff800,	0,		SROM11_FEM_CFG2, 	0x0001},
	{"epagain5g",		0xfffff800,	0,		SROM11_FEM_CFG2, 	0x000e},
	{"pdgain5g",		0xfffff800,	0,		SROM11_FEM_CFG2, 	0x01f0},
	{"tworangetssi5g",	0xfffff800,	0,		SROM11_FEM_CFG2, 	0x0200},
	{"papdcap5g",		0xfffff800,	0,		SROM11_FEM_CFG2, 	0x0400},
	{"gainctrlsph",		0xfffff800,	0,		SROM11_FEM_CFG2, 	0xf800},

	{"tempthresh",		0xfffff800,	0,		SROM11_THERMAL,		0xff00},
	{"tempoffset",		0xfffff800,	0,		SROM11_THERMAL,		0x00ff},
	{"rawtempsense", 	0xfffff800,	SRFL_PRHEX,	SROM11_MPWR_RAWTS,	0x01ff},
	{"measpower",		0xfffff800,	SRFL_PRHEX,	SROM11_MPWR_RAWTS,	0xfe00},
	{"tempsense_slope",	0xfffff800,	SRFL_PRHEX, 	SROM11_TS_SLP_OPT_CORRX, 0x00ff},
	{"tempcorrx",		0xfffff800,	SRFL_PRHEX, 	SROM11_TS_SLP_OPT_CORRX, 0xfc00},
	{"tempsense_option",	0xfffff800,	SRFL_PRHEX,	SROM11_TS_SLP_OPT_CORRX, 0x0300},
	{"xtalfreq",		0xfffff800,	0,		SROM11_XTAL_FREQ, 	0xffff},
	{"txpwrbckof",	0x00000800,	SRFL_PRHEX,	SROM11_PATH0 + SROM11_2G_MAXP,	0xff00},
	/* Special PA Params for 4350 5G Band, 40/80 MHz BW Ant #1 */
	{"phycal_tempdelta",	0xfffff800,	0,		SROM11_PHYCAL_TEMPDELTA, 0x00ff},
	{"temps_period",	0xfffff800,	0,		SROM11_PHYCAL_TEMPDELTA, 0x0f00},
	{"temps_hysteresis",	0xfffff800,	0,		SROM11_PHYCAL_TEMPDELTA, 0xf000},
	{"measpower1", 		0xfffff800,	SRFL_PRHEX,	SROM11_MPWR_1_AND_2, 	0x007f},
	{"measpower2",		0xfffff800, 	SRFL_PRHEX,	SROM11_MPWR_1_AND_2, 	0x3f80},
	{"tssifloor2g",		0xfffff800,	SRFL_PRHEX,	SROM11_TSSIFLOOR_2G,	0x03ff},
	{"tssifloor5g",	0xfffff800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_TSSIFLOOR_5GL,	0x03ff},
	{"",		0xfffff800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_TSSIFLOOR_5GM,	0x03ff},
	{"",		0xfffff800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_TSSIFLOOR_5GH,	0x03ff},
	{"",		0xfffff800,	SRFL_PRHEX,		SROM11_TSSIFLOOR_5GU,	0x03ff},
	{"pdoffset2g40ma0",     0xfffff800, 0,      SROM11_PDOFF_2G_40M,    0x000f},
	{"pdoffset2g40ma1",     0xfffff800, 0,      SROM11_PDOFF_2G_40M,    0x00f0},
	{"pdoffset2g40ma2",     0xfffff800, 0,      SROM11_PDOFF_2G_40M,    0x0f00},
	{"pdoffset2g40mvalid",  0xfffff800, 0,      SROM11_PDOFF_2G_40M,    0x8000},
	{"subband5gver",	0xfffff800, 	SRFL_PRHEX,	SROM11_SUBBAND5GVER, 	0xffff},
	{"paparambwver",	0xfffff800, 	0,		SROM11_MCSLR5GLPO, 	0xf000},
	{"rx5ggainwar",         0xfffff800,     0,              SROM11_MCSLR5GMPO,      0x2000},
	/* Special PA Params for 4335 2G Band, CCK */
	{"pa2gccka0", 0x0003f800, SRFL_PRHEX | SRFL_ARRAY, SROM11_PATH1 + SROM11_2G_PA, 0xffff},
	{"", 0x0003f800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_PATH1 + SROM11_2G_PA + 1, 0xffff},
	{"", 0x0003f800,	SRFL_PRHEX,              SROM11_PATH1 + SROM11_2G_PA + 2, 0xffff},

	/* power offset per rate */
	{"cckbw202gpo",		0xfffff800,	0,		SROM11_CCKBW202GPO,	0xffff},
	{"cckbw20ul2gpo",	0xfffff800,	0,		SROM11_CCKBW20UL2GPO,	0xffff},
	{"mcsbw202gpo",		0xfffff800,	SRFL_MORE,	SROM11_MCSBW202GPO,	0xffff},
	{"",			0xfffff800,	0,		SROM11_MCSBW202GPO_1,	0xffff},
	{"mcsbw402gpo",		0xfffff800,	SRFL_MORE,	SROM11_MCSBW402GPO,   	0xffff},
	{"",            	0xfffff800, 	0,   		SROM11_MCSBW402GPO_1,	0xffff},
	{"dot11agofdmhrbw202gpo", 0xfffff800, 	0, 	SROM11_DOT11AGOFDMHRBW202GPO,	0xffff},
	{"ofdmlrbw202gpo",	0xfffff800, 	0, 		SROM11_OFDMLRBW202GPO,	0xffff},
	{"mcsbw205glpo",	0xfffff800,	SRFL_MORE,	SROM11_MCSBW205GLPO, 	0xffff},
	{"",           		0xfffff800, 	0,   		SROM11_MCSBW205GLPO_1, 	0xffff},
	{"mcsbw405glpo",	0xfffff800,	SRFL_MORE,	SROM11_MCSBW405GLPO, 	0xffff},
	{"",           		0xfffff800, 	0,     		SROM11_MCSBW405GLPO_1, 	0xffff},
	{"mcsbw805glpo",	0xfffff800,	SRFL_MORE,	SROM11_MCSBW805GLPO, 	0xffff},
	{"",           		0xfffff800, 	0,    		SROM11_MCSBW805GLPO_1, 	0xffff},
	{"mcsbw205gmpo",	0xfffff800,	SRFL_MORE,	SROM11_MCSBW205GMPO, 	0xffff},
	{"",           		0xfffff800, 	0,     		SROM11_MCSBW205GMPO_1, 	0xffff},
	{"mcsbw405gmpo",	0xfffff800,	SRFL_MORE,	SROM11_MCSBW405GMPO, 	0xffff},
	{"",           		0xfffff800, 	0,     		SROM11_MCSBW405GMPO_1, 	0xffff},
	{"mcsbw805gmpo",	0xfffff800,	SRFL_MORE,	SROM11_MCSBW805GMPO, 	0xffff},
	{"",           		0xfffff800, 	0,   		SROM11_MCSBW805GMPO_1, 	0xffff},
	{"mcsbw205ghpo",	0xfffff800,	SRFL_MORE,	SROM11_MCSBW205GHPO, 	0xffff},
	{"",           		0xfffff800, 	0,  		SROM11_MCSBW205GHPO_1, 	0xffff},
	{"mcsbw405ghpo",	0xfffff800,	SRFL_MORE,	SROM11_MCSBW405GHPO, 	0xffff},
	{"",           		0xfffff800, 	0,   		SROM11_MCSBW405GHPO_1, 	0xffff},
	{"mcsbw805ghpo",	0xfffff800,	SRFL_MORE,	SROM11_MCSBW805GHPO, 	0xffff},
	{"",           		0xfffff800, 	0,    		SROM11_MCSBW805GHPO_1, 	0xffff},
	{"mcslr5glpo",		0xfffff800,	0,		SROM11_MCSLR5GLPO, 	0x0fff},
	{"mcslr5gmpo",		0xfffff800,	0,		SROM11_MCSLR5GMPO, 	0xffff},
	{"mcslr5ghpo",		0xfffff800,	0,		SROM11_MCSLR5GHPO, 	0xffff},
	{"sb20in40hrpo", 	0xfffff800,	0,	SROM11_SB20IN40HRPO,		0xffff},
	{"sb20in80and160hr5glpo", 0xfffff800, 	0, 	SROM11_SB20IN80AND160HR5GLPO, 	0xffff},
	{"sb40and80hr5glpo",	  0xfffff800, 	0,	SROM11_SB40AND80HR5GLPO,	0xffff},
	{"sb20in80and160hr5gmpo", 0xfffff800, 	0,	SROM11_SB20IN80AND160HR5GMPO, 	0xffff},
	{"sb40and80hr5gmpo",	  0xfffff800, 	0,	SROM11_SB40AND80HR5GMPO,	0xffff},
	{"sb20in80and160hr5ghpo", 0xfffff800, 	0,	SROM11_SB20IN80AND160HR5GHPO, 	0xffff},
	{"sb40and80hr5ghpo",	  0xfffff800, 	0,	SROM11_SB40AND80HR5GHPO,	0xffff},
	{"sb20in40lrpo",	  0xfffff800, 	0,	SROM11_SB20IN40LRPO,		0xffff},
	{"sb20in80and160lr5glpo", 0xfffff800, 	0,	SROM11_SB20IN80AND160LR5GLPO, 	0xffff},
	{"sb40and80lr5glpo",	  0xfffff800, 	0,	SROM11_SB40AND80LR5GLPO,	0xffff},
	{"sb20in80and160lr5gmpo", 0xfffff800, 	0,	SROM11_SB20IN80AND160LR5GMPO, 	0xffff},
	{"sb40and80lr5gmpo",	  0xfffff800, 	0,	SROM11_SB40AND80LR5GMPO,	0xffff},
	{"sb20in80and160lr5ghpo", 0xfffff800, 	0,	SROM11_SB20IN80AND160LR5GHPO, 	0xffff},
	{"sb40and80lr5ghpo",	  0xfffff800, 	0,	SROM11_SB40AND80LR5GHPO,	0xffff},
	{"dot11agduphrpo",	  0xfffff800, 	0,	SROM11_DOT11AGDUPHRPO,		0xffff},
	{"dot11agduplrpo",	  0xfffff800, 	0,	SROM11_DOT11AGDUPLRPO,		0xffff},

	/* Misc */
	{"sar2g",       	0xfffff800,	0,	SROM11_SAR,          	0x00ff},
	{"sar5g",           	0xfffff800,	0,	SROM11_SAR,		0xff00},

	{"noiselvl2ga0",	0xfffff800,	0,		SROM11_NOISELVL_2G,	0x001f},
	{"noiselvl2ga1",	0xfffff800,	0,		SROM11_NOISELVL_2G,	0x03e0},
	{"noiselvl2ga2",	0xfffff800,	0,		SROM11_NOISELVL_2G,	0x7c00},
	{"noiselvl5ga0",	0xfffff800,	SRFL_ARRAY,	SROM11_NOISELVL_5GL,	0x001f},
	{"",			0xfffff800,	SRFL_ARRAY,	SROM11_NOISELVL_5GM,	0x001f},
	{"",			0xfffff800,	SRFL_ARRAY,	SROM11_NOISELVL_5GH,	0x001f},
	{"",			0xfffff800,	0,		SROM11_NOISELVL_5GU,	0x001f},
	{"noiselvl5ga1",	0xfffff800,	SRFL_ARRAY,	SROM11_NOISELVL_5GL,	0x03e0},
	{"",			0xfffff800,	SRFL_ARRAY,	SROM11_NOISELVL_5GM,	0x03e0},
	{"",			0xfffff800,	SRFL_ARRAY,	SROM11_NOISELVL_5GH,	0x03e0},
	{"",			0xfffff800,	0,		SROM11_NOISELVL_5GU,	0x03e0},
	{"noiselvl5ga2",	0xfffff800,	SRFL_ARRAY,	SROM11_NOISELVL_5GL,	0x7c00},
	{"",			0xfffff800,	SRFL_ARRAY,	SROM11_NOISELVL_5GM,	0x7c00},
	{"",			0xfffff800,	SRFL_ARRAY,	SROM11_NOISELVL_5GH,	0x7c00},
	{"",			0xfffff800,	0,		SROM11_NOISELVL_5GU,	0x7c00},
	{"eu_edthresh2g",	0xfffff800,	0,		SROM11_EU_EDCRSTH,	0x00ff},
	{"eu_edthresh5g",	0xfffff800,	0,		SROM11_EU_EDCRSTH,	0xff00},

	{"rxgainerr2ga0", 	0xfffff800, 	0,    		SROM11_RXGAINERR_2G,    0x003f},
	{"rxgainerr2ga1", 	0xfffff800, 	0,    		SROM11_RXGAINERR_2G,    0x07c0},
	{"rxgainerr2ga2", 	0xfffff800, 	0,    		SROM11_RXGAINERR_2G,    0xf800},
	{"rxgainerr5ga0",      	0xfffff800, 	SRFL_ARRAY,    	SROM11_RXGAINERR_5GL,   0x003f},
	{"",      		0xfffff800, 	SRFL_ARRAY,    	SROM11_RXGAINERR_5GM,   0x003f},
	{"",      		0xfffff800, 	SRFL_ARRAY,    	SROM11_RXGAINERR_5GH,   0x003f},
	{"",      		0xfffff800, 	0,    		SROM11_RXGAINERR_5GU,   0x003f},
	{"rxgainerr5ga1",      	0xfffff800, 	SRFL_ARRAY,    	SROM11_RXGAINERR_5GL,   0x07c0},
	{"",      		0xfffff800, 	SRFL_ARRAY,    	SROM11_RXGAINERR_5GM,   0x07c0},
	{"",      		0xfffff800, 	SRFL_ARRAY,    	SROM11_RXGAINERR_5GH,   0x07c0},
	{"",      		0xfffff800, 	0,    		SROM11_RXGAINERR_5GU,   0x07c0},
	{"rxgainerr5ga2",      	0xfffff800, 	SRFL_ARRAY,    	SROM11_RXGAINERR_5GL,   0xf800},
	{"",      		0xfffff800, 	SRFL_ARRAY,    	SROM11_RXGAINERR_5GM,   0xf800},
	{"",      		0xfffff800, 	SRFL_ARRAY,    	SROM11_RXGAINERR_5GH,   0xf800},
	{"",      		0xfffff800, 	0,    		SROM11_RXGAINERR_5GU,   0xf800},
	{"rpcal2g",      	0xfffff800, 	0,		SROM11_RPCAL_2G,        0xffff},
	{"rpcal5gb0",      	0xfffff800, 	0,		SROM11_RPCAL_5GL,       0xffff},
	{"rpcal5gb1",      	0xfffff800, 	0,		SROM11_RPCAL_5GM,       0xffff},
	{"rpcal5gb2",      	0xfffff800, 	0,		SROM11_RPCAL_5GH,       0xffff},
	{"rpcal5gb3",      	0xfffff800, 	0,		SROM11_RPCAL_5GU,       0xffff},
	{"txidxcap2g",      	0xfffff800, 	0,		SROM11_TXIDXCAP2G,      0x0ff0},
	{"txidxcap5g",      	0xfffff800, 	0,		SROM11_TXIDXCAP5G,      0x0ff0},
	{"vcocal_bp_temp", 	0xfffff800, 	0,		SROM19_VCOCAL_BP_TEMP,  0x00ff},

	/* sromrev 12 */
	{"boardflags4",		0xfffff000,	SRFL_PRHEX|SRFL_MORE,	SROM12_BFL6,	0xffff},
	{"",	0,	0,			SROM12_BFL7,	0xffff},
	{"pdoffsetcck",		0xfffff000,	0,      SROM12_PDOFF_2G_CCK,       0xffff},
	{"pdoffset20in40m5gb0",	0xfffff000,	0,      SROM12_PDOFF_20in40M_5G_B0,    0xffff},
	{"pdoffset20in40m5gb1", 0xfffff000,	0,      SROM12_PDOFF_20in40M_5G_B1,    0xffff},
	{"pdoffset20in40m5gb2", 0xfffff000,	0,      SROM12_PDOFF_20in40M_5G_B2,    0xffff},
	{"pdoffset20in40m5gb3", 0xfffff000,	0,      SROM12_PDOFF_20in40M_5G_B3,    0xffff},
	{"pdoffset20in40m5gb4", 0xfffff000,	0,      SROM12_PDOFF_20in40M_5G_B4,    0xffff},
	{"pdoffset40in80m5gb0", 0xfffff000,	0,      SROM12_PDOFF_40in80M_5G_B0,    0xffff},
	{"pdoffset40in80m5gb1", 0xfffff000,	0,      SROM12_PDOFF_40in80M_5G_B1,    0xffff},
	{"pdoffset40in80m5gb2", 0xfffff000,	0,      SROM12_PDOFF_40in80M_5G_B2,    0xffff},
	{"pdoffset40in80m5gb3", 0xfffff000,	0,      SROM12_PDOFF_40in80M_5G_B3,    0xffff},
	{"pdoffset40in80m5gb4", 0xfffff000,	0,      SROM12_PDOFF_40in80M_5G_B4,    0xffff},
	{"pdoffset20in80m5gb0", 0xfffff000,	0,      SROM12_PDOFF_20in80M_5G_B0,    0xffff},
	{"pdoffset20in80m5gb1", 0xfffff000,	0,      SROM12_PDOFF_20in80M_5G_B1,    0xffff},
	{"pdoffset20in80m5gb2", 0xfffff000,	0,      SROM12_PDOFF_20in80M_5G_B2,    0xffff},
	{"pdoffset20in80m5gb3", 0xfffff000,	0,      SROM12_PDOFF_20in80M_5G_B3,    0xffff},
	{"pdoffset20in80m5gb4", 0xfffff000,	0,      SROM12_PDOFF_20in80M_5G_B4,    0xffff},

	/* power offset per rate */
	{"mcsbw205gx1po",       0xfffff000,     SRFL_MORE,      SROM12_MCSBW205GX1PO,   0xffff},
	{"",                    0xfffff000,     0,              SROM12_MCSBW205GX1PO_1, 0xffff},
	{"mcsbw405gx1po",       0xfffff000,     SRFL_MORE,      SROM12_MCSBW405GX1PO,   0xffff},
	{"",                    0xfffff000,     0,              SROM12_MCSBW405GX1PO_1, 0xffff},
	{"mcsbw805gx1po",       0xfffff000,     SRFL_MORE,      SROM12_MCSBW805GX1PO,   0xffff},
	{"",                    0xfffff000,     0,              SROM12_MCSBW805GX1PO_1, 0xffff},
	{"mcsbw205gx2po",       0xfffff000,     SRFL_MORE,      SROM12_MCSBW205GX2PO,   0xffff},
	{"",                    0xfffff000,     0,              SROM12_MCSBW205GX2PO_1, 0xffff},
	{"mcsbw405gx2po",       0xfffff000,     SRFL_MORE,      SROM12_MCSBW405GX2PO,   0xffff},
	{"",                    0xfffff000,     0,              SROM12_MCSBW405GX2PO_1, 0xffff},
	{"mcsbw805gx2po",       0xfffff000,     SRFL_MORE,      SROM12_MCSBW805GX2PO,   0xffff},
	{"",                    0xfffff000,     0,              SROM12_MCSBW805GX2PO_1, 0xffff},

	{"sb20in80and160hr5gx1po", 0xfffff000,  0,      SROM12_SB20IN80AND160HR5GX1PO,  0xffff},
	{"sb40and80hr5gx1po",     0xfffff000,   0,      SROM12_SB40AND80HR5GX1PO,       0xffff},
	{"sb20in80and160lr5gx1po", 0xfffff000,  0,      SROM12_SB20IN80AND160LR5GX1PO,  0xffff},
	{"sb40and80hr5gx1po",     0xfffff000,   0,      SROM12_SB40AND80HR5GX1PO,       0xffff},
	{"sb20in80and160hr5gx2po", 0xfffff000,  0,      SROM12_SB20IN80AND160HR5GX2PO,  0xffff},
	{"sb40and80hr5gx2po",     0xfffff000,   0,      SROM12_SB40AND80HR5GX2PO,       0xffff},
	{"sb20in80and160lr5gx2po", 0xfffff000,  0,      SROM12_SB20IN80AND160LR5GX2PO,  0xffff},
	{"sb40and80hr5gx2po",     0xfffff000,   0,      SROM12_SB40AND80HR5GX2PO,       0xffff},

	{"rxgains5gmelnagaina0",	0xfffff000,	0,       SROM12_RXGAINS10,	0x0007},
	{"rxgains5gmelnagaina1",	0xfffff000,	0,       SROM12_RXGAINS11,	0x0007},
	{"rxgains5gmelnagaina2",	0xfffff000,	0,       SROM12_RXGAINS12,	0x0007},
	{"rxgains5gmtrisoa0",	0xfffff000,	0,       SROM12_RXGAINS10,	0x0078},
	{"rxgains5gmtrisoa1",	0xfffff000,	0,       SROM12_RXGAINS11,	0x0078},
	{"rxgains5gmtrisoa2",	0xfffff000,	0,       SROM12_RXGAINS12,	0x0078},
	{"rxgains5gmtrelnabypa0", 0xfffff000,	0,       SROM12_RXGAINS10,	0x0080},
	{"rxgains5gmtrelnabypa1", 0xfffff000,	0,       SROM12_RXGAINS11,	0x0080},
	{"rxgains5gmtrelnabypa2", 0xfffff000,	0,       SROM12_RXGAINS12,	0x0080},
	{"rxgains5ghelnagaina0",	0xfffff000,	0,       SROM12_RXGAINS10,	0x0700},
	{"rxgains5ghelnagaina1",	0xfffff000,	0,       SROM12_RXGAINS11,	0x0700},
	{"rxgains5ghelnagaina2",	0xfffff000,	0,       SROM12_RXGAINS12,	0x0700},
	{"rxgains5ghtrisoa0",	0xfffff000,	0,	 SROM12_RXGAINS10,	0x7800},
	{"rxgains5ghtrisoa1",	0xfffff000,	0,	 SROM12_RXGAINS11,	0x7800},
	{"rxgains5ghtrisoa2",	0xfffff000,	0,	 SROM12_RXGAINS12,	0x7800},
	{"rxgains5ghtrelnabypa0", 0xfffff000,	0,	 SROM12_RXGAINS10,	0x8000},
	{"rxgains5ghtrelnabypa1", 0xfffff000,	0,	 SROM12_RXGAINS11,	0x8000},
	{"rxgains5ghtrelnabypa2", 0xfffff000,	0,	 SROM12_RXGAINS12,	0x8000},

	{"gpdn",		0xfffff000,	SRFL_PRHEX|SRFL_MORE,	SROM12_GPDN_L,	0xffff},
	{"",			0,		0,			SROM12_GPDN_H,	0xffff},

	{"rpcal2gcore3",        0xffffe000, 	0,	SROM13_RPCAL2GCORE3,            0x00ff},
	{"rpcal5gb0core3",      0xffffe000, 	0,	SROM13_RPCAL5GB01CORE3,         0x00ff},
	{"rpcal5gb1core3",      0xffffe000, 	0,	SROM13_RPCAL5GB01CORE3,         0xff00},
	{"rpcal5gb2core3",      0xffffe000, 	0,	SROM13_RPCAL5GB23CORE3,         0x00ff},
	{"rpcal5gb3core3",      0xffffe000, 	0,	SROM13_RPCAL5GB23CORE3,         0xff00},

	{"sw_txchain_mask",	0xffffe000,	0,	SROM13_SW_TXRX_MASK,	0x000f},
	{"sw_rxchain_mask",	0xffffe000,	0,	SROM13_SW_TXRX_MASK,	0x00f0},

	{"agbg3",       0xffffe000,     0,              SROM13_ANTGAIN_BANDBGA, 0xff00},
	{"aga3",        0xffffe000,     0,              SROM13_ANTGAIN_BANDBGA, 0x00ff},
	{"noiselvl2ga3",        0xffffe000,     0,              SROM13_NOISELVLCORE3,   0x001f},
	{"noiselvl5ga3",        0xffffe000,     SRFL_ARRAY,     SROM13_NOISELVLCORE3,   0x03e0},
	{"",                    0xffffe000,     SRFL_ARRAY,     SROM13_NOISELVLCORE3,   0x7c00},
	{"",                    0xffffe000,     SRFL_ARRAY,     SROM13_NOISELVLCORE3_1, 0x001f},
	{"",                    0xffffe000,     0,              SROM13_NOISELVLCORE3_1, 0x03e0},
	{"rxgainerr2ga3",       0xffffe000,     0,              SROM13_RXGAINERRCORE3,  0x001f},
	{"rxgainerr5ga3",       0xffffe000,     SRFL_ARRAY,     SROM13_RXGAINERRCORE3,  0x03e0},
	{"",                    0xffffe000,     SRFL_ARRAY,     SROM13_RXGAINERRCORE3,  0x7c00},
	{"",                    0xffffe000,     SRFL_ARRAY,     SROM13_RXGAINERRCORE3_1, 0x001f},
	{"",                    0xffffe000,     0,              SROM13_RXGAINERRCORE3_1, 0x03e0},
	{"rxgains5gmelnagaina3",        0xffffe000,     0,       SROM13_RXGAINS1CORE3,  0x0007},
	{"rxgains5gmtrisoa3",   0xffffe000,     0,       SROM13_RXGAINS1CORE3,  0x0078},
	{"rxgains5gmtrelnabypa3", 0xffffe000,   0,       SROM13_RXGAINS1CORE3,  0x0080},
	{"rxgains5ghelnagaina3",        0xffffe000,     0,       SROM13_RXGAINS1CORE3,  0x0700},
	{"rxgains5ghtrisoa3",   0xffffe000,     0,       SROM13_RXGAINS1CORE3,  0x7800},
	{"rxgains5ghtrelnabypa3", 0xffffe000,   0,       SROM13_RXGAINS1CORE3,  0x8000},

	/* WBPAPD */
	{"wb_rxattn", 0xfffc0000, SRFL_PRHEX, SROM18_WBRXATTN, 0xffff},
	{"wb_tia_gain_mode", 0xfffc0000, SRFL_PRHEX, SROM18_WBTIAGAINMODE, 0xffff},
	{"wb_txbuf_offset", 0xfffc0000, SRFL_PRHEX, SROM18_WBTXBUFOFFSET, 0xffff},

	/* Regulatory limits */
	{"peak_psd_limit_6G2G", 0xfffc0000, SRFL_PRHEX, SROM18_PEAKPSDLIMIT6G2G, 0xffff},
	{"peak_psd_limit_5G0", 0xfffc0000, SRFL_PRHEX, SROM18_PEAKPSDLIMIT5G0, 0xffff},
	{"peak_psd_limit_5G1", 0xfffc0000, SRFL_PRHEX, SROM18_PEAKPSDLIMIT5G1, 0xffff},
	{"peak_psd_limit_5G2", 0xfffc0000, SRFL_PRHEX, SROM18_PEAKPSDLIMIT5G2, 0xffff},

	/* 5gext PDOFFSET */
	{"pdoffset20in40m5gb5", 0xfffc0000, 0, SROM18_PDOFFSET20IN40M5GB5, 0xffff},
	{"pdoffset20in40m5gb6", 0xfffc0000, 0, SROM18_PDOFFSET20IN40M5GB6, 0xffff},
	{"pdoffset20in80m5gb5", 0xfffc0000, 0, SROM18_PDOFFSET20IN80M5GB5, 0xffff},
	{"pdoffset20in80m5gb6", 0xfffc0000, 0, SROM18_PDOFFSET20IN80M5GB6, 0xffff},
	{"pdoffset40in80m5gb5", 0xfffc0000, 0, SROM18_PDOFFSET40IN80M5GB5, 0xffff},
	{"pdoffset40in80m5gb6", 0xfffc0000, 0, SROM18_PDOFFSET40IN80M5GB6, 0xffff},
	{"pdoffset20in40m5gcore3_2", 0xfffc0000, 0, SROM18_PDOFFSET20IN40M5GCORE3_2, 0xffff},
	{"pdoffset20in80m5gcore3_2", 0xfffc0000, 0, SROM18_PDOFFSET20IN80M5GCORE3_2, 0xffff},
	{"pdoffset40in80m5gcore3_2", 0xfffc0000, 0, SROM18_PDOFFSET40IN80M5GCORE3_2, 0xffff},
	{"pdoffset20in160m5gc4", 0xfffc0000, 0, SROM18_PDOFFSET20IN160M5GC4, 0xffff},
	{"pdoffset20in160m5gc5", 0xfffc0000, 0, SROM18_PDOFFSET20IN160M5GC5, 0xffff},
	{"pdoffset20in160m5gc6", 0xfffc0000, 0, SROM18_PDOFFSET20IN160M5GC6, 0xffff},
	{"pdoffset20in160m5gcore3_2", 0xfffc0000, 0, SROM18_PDOFFSET20IN160M5GCORE3_2, 0xffff},
	{"pdoffset40in160m5gc4", 0xfffc0000, 0, SROM18_PDOFFSET40IN160M5GC4, 0xffff},
	{"pdoffset40in160m5gc5", 0xfffc0000, 0, SROM18_PDOFFSET40IN160M5GC5, 0xffff},
	{"pdoffset40in160m5gc6", 0xfffc0000, 0, SROM18_PDOFFSET40IN160M5GC6, 0xffff},
	{"pdoffset40in160m5gcore3_2", 0xfffc0000, 0, SROM18_PDOFFSET40IN160M5GCORE3_2, 0xffff},
	{"pdoffset80in160m5gc4", 0xfffc0000, 0, SROM18_PDOFFSET80IN160M5GC4, 0xffff},
	{"pdoffset80in160m5gc5", 0xfffc0000, 0, SROM18_PDOFFSET80IN160M5GC5, 0xffff},
	{"pdoffset80in160m5gc6", 0xfffc0000, 0, SROM18_PDOFFSET80IN160M5GC6, 0xffff},
	{"pdoffset80in160m5gcore3_2", 0xfffc0000, 0, SROM18_PDOFFSET80IN160M5GCORE3_2, 0xffff},

	/* maxp5gext */
	{"maxp5gb5a0",  0xfffc0000, 0, SROM18_MAXP5GEXT_CORE0, 0x00ff},
	{"maxp5gb6a0",  0xfffc0000, 0, SROM18_MAXP5GEXT_CORE0, 0xff00},
	{"maxp5gb5a1",  0xfffc0000, 0, SROM18_MAXP5GEXT_CORE1, 0x00ff},
	{"maxp5gb6a1",  0xfffc0000, 0, SROM18_MAXP5GEXT_CORE1, 0xff00},
	{"maxp5gb5a2",  0xfffc0000, 0, SROM18_MAXP5GEXT_CORE2, 0x00ff},
	{"maxp5gb6a2",  0xfffc0000, 0, SROM18_MAXP5GEXT_CORE2, 0xff00},
	{"maxp5gb5a3",  0xfffc0000, 0, SROM18_MAXP5GEXT_CORE3, 0x00ff},
	{"maxp5gb6a3",  0xfffc0000, 0, SROM18_MAXP5GEXT_CORE3, 0xff00},

	/* fem_jtag_gpio */
	{"rfem_jtag_gpio",  0xfff80000, 0, SROM19_RFEM_JTAG_GPIO, 0xffff},
	{"rfem_pa",  0xfff80000, 0, SROM19_RFEM, 0x00ff},

	{"swctrlmap4_TXlin2g_fem3to0",  0xfff80000, 0, SROM19_SWCTRLMAP4_TXLIN2G_FEM3TO0, 0xffff},
	{"swctrlmap4_TXlin2g_fem7to4",  0xfff80000, 0, SROM19_SWCTRLMAP4_TXLIN2G_FEM7TO4, 0xffff},
	{"swctrlmap4_TXlin5g_fem3to0",  0xfff80000, 0, SROM19_SWCTRLMAP4_TXLIN5G_FEM3TO0, 0xffff},
	{"swctrlmap4_TXlin5g_fem7to4",  0xfff80000, 0, SROM19_SWCTRLMAP4_TXLIN5G_FEM7TO4, 0xffff},
	{"papd_swctrlmap_femtxlin2g",  0xfff80000, 0, SROM19_PAPD_SWCTRLMAP_FEMTXLIN2G, 0xffff},
	{"papd_swctrlmap_femtxlin5g",  0xfff80000, 0, SROM19_PAPD_SWCTRLMAP_FEMTXLIN5G, 0xffff},
	{"papd_swctrlmap_femtxlin6g",  0xfff80000, 0, SROM19_PAPD_SWCTRLMAP_FEMTXLIN6G, 0xffff},

	/* 320M edcrs edge adjustment */
	{"subband_ed_adj_320",  0xfff80000, 0, SROM19_SUBBAND_ED_ADJ_320, 0x00ff},

	/* max powers for 6G subbands 0 and 1 */
	{"maxp6gb0a0",  0xfff80000, 0, SROM19_MAXP6G_SB01_CORE0, 0x00ff}, /* subband 0, ant0 */
	{"maxp6gb1a0",  0xfff80000, 0, SROM19_MAXP6G_SB01_CORE0, 0xff00}, /* subband 1, ant0 */
	{"maxp6gb0a1",  0xfff80000, 0, SROM19_MAXP6G_SB01_CORE1, 0x00ff}, /* subband 0, ant1 */
	{"maxp6gb1a1",  0xfff80000, 0, SROM19_MAXP6G_SB01_CORE1, 0xff00}, /* subband 1, ant1 */
	{"maxp6gb0a2",  0xfff80000, 0, SROM19_MAXP6G_SB01_CORE2, 0x00ff}, /* subband 0, ant2 */
	{"maxp6gb1a2",  0xfff80000, 0, SROM19_MAXP6G_SB01_CORE2, 0xff00}, /* subband 1, ant2 */
	{"maxp6gb0a3",  0xfff80000, 0, SROM19_MAXP6G_SB01_CORE3, 0x00ff}, /* subband 0, ant3 */
	{"maxp6gb1a3",  0xfff80000, 0, SROM19_MAXP6G_SB01_CORE3, 0xff00}, /* subband 1, ant3 */
	/* max powers for 6G subbands 2 and 3 */
	{"maxp6gb2a0",  0xfff80000, 0, SROM19_MAXP6G_SB23_CORE0, 0x00ff}, /* subband 2, ant0 */
	{"maxp6gb3a0",  0xfff80000, 0, SROM19_MAXP6G_SB23_CORE0, 0xff00}, /* subband 3, ant0 */
	{"maxp6gb2a1",  0xfff80000, 0, SROM19_MAXP6G_SB23_CORE1, 0x00ff}, /* subband 2, ant1 */
	{"maxp6gb3a1",  0xfff80000, 0, SROM19_MAXP6G_SB23_CORE1, 0xff00}, /* subband 3, ant1 */
	{"maxp6gb2a2",  0xfff80000, 0, SROM19_MAXP6G_SB23_CORE2, 0x00ff}, /* subband 2, ant2 */
	{"maxp6gb3a2",  0xfff80000, 0, SROM19_MAXP6G_SB23_CORE2, 0xff00}, /* subband 3, ant2 */
	{"maxp6gb2a3",  0xfff80000, 0, SROM19_MAXP6G_SB23_CORE3, 0x00ff}, /* subband 2, ant3 */
	{"maxp6gb3a3",  0xfff80000, 0, SROM19_MAXP6G_SB23_CORE3, 0xff00}, /* subband 3, ant3 */
	/* max powers for 6G subbands 4 and 5 */
	{"maxp6gb4a0",  0xfff80000, 0, SROM19_MAXP6G_SB45_CORE0, 0x00ff}, /* subband 4, ant0 */
	{"maxp6gb5a0",  0xfff80000, 0, SROM19_MAXP6G_SB45_CORE0, 0xff00}, /* subband 5, ant0 */
	{"maxp6gb4a1",  0xfff80000, 0, SROM19_MAXP6G_SB45_CORE1, 0x00ff}, /* subband 4, ant1 */
	{"maxp6gb5a1",  0xfff80000, 0, SROM19_MAXP6G_SB45_CORE1, 0xff00}, /* subband 5, ant1 */
	{"maxp6gb4a2",  0xfff80000, 0, SROM19_MAXP6G_SB45_CORE2, 0x00ff}, /* subband 4, ant2 */
	{"maxp6gb5a2",  0xfff80000, 0, SROM19_MAXP6G_SB45_CORE2, 0xff00}, /* subband 5, ant2 */
	{"maxp6gb4a3",  0xfff80000, 0, SROM19_MAXP6G_SB45_CORE3, 0x00ff}, /* subband 4, ant3 */
	{"maxp6gb5a3",  0xfff80000, 0, SROM19_MAXP6G_SB45_CORE3, 0xff00}, /* subband 5, ant3 */
	/* max powers for 6G subband 6 */
	{"maxp6gb6a0",  0xfff80000, 0, SROM19_MAXP6G_SB6_CORE0, 0x00ff}, /* subband 6, ant0 */
	{"maxp6gb6a1",  0xfff80000, 0, SROM19_MAXP6G_SB6_CORE1, 0x00ff}, /* subband 6, ant1 */
	{"maxp6gb6a2",  0xfff80000, 0, SROM19_MAXP6G_SB6_CORE2, 0x00ff}, /* subband 6, ant2 */
	{"maxp6gb6a3",  0xfff80000, 0, SROM19_MAXP6G_SB6_CORE3, 0x00ff}, /* subband 6, ant3 */
	/* antenna gain for 6g band */
	{"antgain_band6g0", 0xfff80000, 0, SROM19_ANTGAIN_BAND6G_01, 0x00ff}, /* ant0 */
	{"antgain_band6g1", 0xfff80000, 0, SROM19_ANTGAIN_BAND6G_01, 0xff00}, /* ant1 */
	{"antgain_band6g2", 0xfff80000, 0, SROM19_ANTGAIN_BAND6G_23, 0x00ff}, /* ant2 */
	{"antgain_band6g3", 0xfff80000, 0, SROM19_ANTGAIN_BAND6G_23, 0xff00}, /* ant3 */
	/* fem_cfg3:
	 * 15:11 gainctrl_6g, 10 papd_cap_6g,9 2_range_tssi_6g,8:4 pd_gain_6g, 3:1 epa_gain_6g,
	 * 0 tssi_posslope_6g
	 * epa_gain definition:
	 * 0 = full gain PA, 1 = PA "lite", 2 = no external PA, 3 = high power external PA and
	 * B2 means non-linear FEM.
	 */
	{"tssi_posslope_6g",	0xfff80000, 0, SROM19_FEM_CFG3, 0x0001},
	{"epa_gain_6g",		0xfff80000, 0, SROM19_FEM_CFG3, 0x000e},
	{"pd_gain_6g",		0xfff80000, 0, SROM19_FEM_CFG3, 0x01f0},
	{"2_range_tssi_6g",	0xfff80000, 0, SROM19_FEM_CFG3, 0x0200},
	{"papd_cap_6g",		0xfff80000, 0, SROM19_FEM_CFG3, 0x0400},
	{"gainctrl_6g",		0xfff80000, 0, SROM19_FEM_CFG3, 0xf800},
	/* 6G power offset per QAM4096 rate, per subband */
	{"mcs4kqam6gb0po",	0xfff80000, SRFL_MORE,	SROM19_MCS4KQAM6GB0PO,   0xffff}, /* sb0 */
	{"",			0xfff80000, 0,		SROM19_MCS4KQAM6GB0PO_1, 0xffff},
	{"mcs4kqam6gb1po",	0xfff80000, SRFL_MORE,	SROM19_MCS4KQAM6GB1PO,   0xffff}, /* sb1 */
	{"",			0xfff80000, 0,		SROM19_MCS4KQAM6GB1PO_1, 0xffff},
	{"mcs4kqam6gb2po",	0xfff80000, SRFL_MORE,	SROM19_MCS4KQAM6GB2PO,   0xffff}, /* sb2 */
	{"",			0xfff80000, 0,		SROM19_MCS4KQAM6GB2PO_1, 0xffff},
	{"mcs4kqam6gb3po",	0xfff80000, SRFL_MORE,	SROM19_MCS4KQAM6GB3PO,   0xffff}, /* sb3 */
	{"",			0xfff80000, 0,		SROM19_MCS4KQAM6GB3PO_1, 0xffff},
	{"mcs4kqam6gb4po",	0xfff80000, SRFL_MORE,	SROM19_MCS4KQAM6GB4PO,   0xffff}, /* sb4 */
	{"",			0xfff80000, 0,		SROM19_MCS4KQAM6GB4PO_1, 0xffff},
	{"mcs4kqam6gb5po",	0xfff80000, SRFL_MORE,	SROM19_MCS4KQAM6GB5PO,   0xffff}, /* sb5 */
	{"",			0xfff80000, 0,		SROM19_MCS4KQAM6GB5PO_1, 0xffff},
	{"mcs4kqam6gb6po",	0xfff80000, SRFL_MORE,	SROM19_MCS4KQAM6GB6PO,   0xffff}, /* sb6 */
	{"",			0xfff80000, 0,		SROM19_MCS4KQAM6GB6PO_1, 0xffff},
	/* 6G power offset per QAM1024 rate, per subband */
	{"mcs1024qam6gb0po", 0xfff80000, SRFL_MORE, SROM19_MCS1024QAM6GB0PO,   0xffff}, /* sb0 */
	{"",                 0xfff80000, 0,	    SROM19_MCS1024QAM6GB0PO_1, 0xffff},
	{"mcs1024qam6gb1po", 0xfff80000, SRFL_MORE, SROM19_MCS1024QAM6GB1PO,   0xffff}, /* sb1 */
	{"",                 0xfff80000, 0,	    SROM19_MCS1024QAM6GB1PO_1, 0xffff},
	{"mcs1024qam6gb2po", 0xfff80000, SRFL_MORE, SROM19_MCS1024QAM6GB2PO,   0xffff}, /* sb2 */
	{"",                 0xfff80000, 0,	    SROM19_MCS1024QAM6GB2PO_1, 0xffff},
	{"mcs1024qam6gb3po", 0xfff80000, SRFL_MORE, SROM19_MCS1024QAM6GB3PO,   0xffff}, /* sb3 */
	{"",                 0xfff80000, 0,	    SROM19_MCS1024QAM6GB3PO_1, 0xffff},
	{"mcs1024qam6gb4po", 0xfff80000, SRFL_MORE, SROM19_MCS1024QAM6GB4PO,   0xffff}, /* sb4 */
	{"",                 0xfff80000, 0,	    SROM19_MCS1024QAM6GB4PO_1, 0xffff},
	{"mcs1024qam6gb5po", 0xfff80000, SRFL_MORE, SROM19_MCS1024QAM6GB5PO,   0xffff}, /* sb5 */
	{"",                 0xfff80000, 0,	    SROM19_MCS1024QAM6GB5PO_1, 0xffff},
	{"mcs1024qam6gb6po", 0xfff80000, SRFL_MORE, SROM19_MCS1024QAM6GB6PO,   0xffff}, /* sb6 */
	{"",                 0xfff80000, 0,	    SROM19_MCS1024QAM6GB6PO_1, 0xffff},
	/* 6G power offsets for BW=160MHz */
	{"mcsbw1606gb0po",   0xfff80000, SRFL_MORE, SROM19_MCSBW1606GB0PO,     0xffff}, /* sb0 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW1606GB0PO_1,   0xffff},
	{"mcsbw1606gb1po",   0xfff80000, SRFL_MORE, SROM19_MCSBW1606GB1PO,     0xffff}, /* sb1 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW1606GB1PO_1,   0xffff},
	{"mcsbw1606gb2po",   0xfff80000, SRFL_MORE, SROM19_MCSBW1606GB2PO,     0xffff}, /* sb2 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW1606GB2PO_1,   0xffff},
	{"mcsbw1606gb3po",   0xfff80000, SRFL_MORE, SROM19_MCSBW1606GB3PO,     0xffff}, /* sb3 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW1606GB3PO_1,   0xffff},
	{"mcsbw1606gb4po",   0xfff80000, SRFL_MORE, SROM19_MCSBW1606GB4PO,     0xffff}, /* sb4 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW1606GB4PO_1,   0xffff},
	{"mcsbw1606gb5po",   0xfff80000, SRFL_MORE, SROM19_MCSBW1606GB5PO,     0xffff}, /* sb5 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW1606GB5PO_1,   0xffff},
	{"mcsbw1606gb6po",   0xfff80000, SRFL_MORE, SROM19_MCSBW1606GB6PO,     0xffff}, /* sb6 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW1606GB6PO_1,   0xffff},
	/* 6G power offsets for BW=80MHz */
	{"mcsbw806gb0po",    0xfff80000, SRFL_MORE, SROM19_MCSBW806GB0PO,     0xffff}, /* sb0 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW806GB0PO_1,   0xffff},
	{"mcsbw806gb1po",    0xfff80000, SRFL_MORE, SROM19_MCSBW806GB1PO,     0xffff}, /* sb1 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW806GB1PO_1,   0xffff},
	{"mcsbw806gb2po",    0xfff80000, SRFL_MORE, SROM19_MCSBW806GB2PO,     0xffff}, /* sb2 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW806GB2PO_1,   0xffff},
	{"mcsbw806gb3po",    0xfff80000, SRFL_MORE, SROM19_MCSBW806GB3PO,     0xffff}, /* sb3 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW806GB3PO_1,   0xffff},
	{"mcsbw806gb4po",    0xfff80000, SRFL_MORE, SROM19_MCSBW806GB4PO,     0xffff}, /* sb4 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW806GB4PO_1,   0xffff},
	{"mcsbw806gb5po",    0xfff80000, SRFL_MORE, SROM19_MCSBW806GB5PO,     0xffff}, /* sb5 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW806GB5PO_1,   0xffff},
	{"mcsbw806gb6po",    0xfff80000, SRFL_MORE, SROM19_MCSBW806GB6PO,     0xffff}, /* sb6 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW806GB6PO_1,   0xffff},
	/* 6G power offsets for BW=40MHz */
	{"mcsbw406gb0po",    0xfff80000, SRFL_MORE, SROM19_MCSBW406GB0PO,     0xffff}, /* sb0 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW406GB0PO_1,   0xffff},
	{"mcsbw406gb1po",    0xfff80000, SRFL_MORE, SROM19_MCSBW406GB1PO,     0xffff}, /* sb1 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW406GB1PO_1,   0xffff},
	{"mcsbw406gb2po",    0xfff80000, SRFL_MORE, SROM19_MCSBW406GB2PO,     0xffff}, /* sb2 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW406GB2PO_1,   0xffff},
	{"mcsbw406gb3po",    0xfff80000, SRFL_MORE, SROM19_MCSBW406GB3PO,     0xffff}, /* sb3 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW406GB3PO_1,   0xffff},
	{"mcsbw406gb4po",    0xfff80000, SRFL_MORE, SROM19_MCSBW406GB4PO,     0xffff}, /* sb4 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW406GB4PO_1,   0xffff},
	{"mcsbw406gb5po",    0xfff80000, SRFL_MORE, SROM19_MCSBW406GB5PO,     0xffff}, /* sb5 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW406GB5PO_1,   0xffff},
	{"mcsbw406gb6po",    0xfff80000, SRFL_MORE, SROM19_MCSBW406GB6PO,     0xffff}, /* sb6 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW406GB6PO_1,   0xffff},
	/* 6G power offsets for BW=20MHz */
	{"mcsbw206gb0po",    0xfff80000, SRFL_MORE, SROM19_MCSBW206GB0PO,     0xffff}, /* sb0 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW206GB0PO_1,   0xffff},
	{"mcsbw206gb1po",    0xfff80000, SRFL_MORE, SROM19_MCSBW206GB1PO,     0xffff}, /* sb1 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW206GB1PO_1,   0xffff},
	{"mcsbw206gb2po",    0xfff80000, SRFL_MORE, SROM19_MCSBW206GB2PO,     0xffff}, /* sb2 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW206GB2PO_1,   0xffff},
	{"mcsbw206gb3po",    0xfff80000, SRFL_MORE, SROM19_MCSBW206GB3PO,     0xffff}, /* sb3 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW206GB3PO_1,   0xffff},
	{"mcsbw206gb4po",    0xfff80000, SRFL_MORE, SROM19_MCSBW206GB4PO,     0xffff}, /* sb4 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW206GB4PO_1,   0xffff},
	{"mcsbw206gb5po",    0xfff80000, SRFL_MORE, SROM19_MCSBW206GB5PO,     0xffff}, /* sb5 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW206GB5PO_1,   0xffff},
	{"mcsbw206gb6po",    0xfff80000, SRFL_MORE, SROM19_MCSBW206GB6PO,     0xffff}, /* sb6 */
	{"",                 0xfff80000, 0,         SROM19_MCSBW206GB6PO_1,   0xffff},
	/* 6G power offset exponents per MCS */
	{"mcs8poexp6g",      0xfff80000, SRFL_MORE, SROM19_MCS8POEXP6G,       0xffff}, /* mcs8 */
	{"",                 0xfff80000, 0,         SROM19_MCS8POEXP6G_1,     0xffff},
	{"mcs9poexp6g",      0xfff80000, SRFL_MORE, SROM19_MCS9POEXP6G,       0xffff}, /* mcs9 */
	{"",                 0xfff80000, 0,         SROM19_MCS9POEXP6G_1,     0xffff},
	{"mcs10poexp6g",     0xfff80000, SRFL_MORE, SROM19_MCS10POEXP6G,      0xffff}, /* mcs10 */
	{"",                 0xfff80000, 0,         SROM19_MCS10POEXP6G_1,    0xffff},
	{"mcs11poexp6g",     0xfff80000, SRFL_MORE, SROM19_MCS11POEXP6G,      0xffff}, /* mcs11 */
	{"",                 0xfff80000, 0,         SROM19_MCS11POEXP6G_1,    0xffff},
	{"mcs12poexp6g",     0xfff80000, SRFL_MORE, SROM19_MCS12POEXP6G,      0xffff}, /* mcs12 */
	{"",                 0xfff80000, 0,         SROM19_MCS12POEXP6G_1,    0xffff},
	{"mcs13poexp6g",     0xfff80000, SRFL_MORE, SROM19_MCS13POEXP6G,      0xffff}, /* mcs13 */
	{"",                 0xfff80000, 0,         SROM19_MCS13POEXP6G_1,    0xffff},
	/* 6G low rates power offset per subband */
	{"mcslr6gb0po",      0xfff80000, 0,         SROM19_MCSLR6GB0PO,       0x0fff}, /* sb0 */
	{"mcslr6gb1po",      0xfff80000, 0,         SROM19_MCSLR6GB1PO,       0x0fff}, /* sb1 */
	{"mcslr6gb2po",      0xfff80000, 0,         SROM19_MCSLR6GB2PO,       0x0fff}, /* sb2 */
	{"mcslr6gb3po",      0xfff80000, 0,         SROM19_MCSLR6GB3PO,       0x0fff}, /* sb3 */
	{"mcslr6gb4po",      0xfff80000, 0,         SROM19_MCSLR6GB4PO,       0x0fff}, /* sb4 */
	{"mcslr6gb5po",      0xfff80000, 0,         SROM19_MCSLR6GB5PO,       0x0fff}, /* sb5 */
	{"mcslr6gb6po",      0xfff80000, 0,         SROM19_MCSLR6GB6PO,       0x0fff}, /* sb6 */
	/* 6G high rate power offset */
	{"sb20in40hrpo6g",         0xfff80000, 0, SROM19_SB20IN40HRPO6G,         0xffff},
	{"sb20in40hrlrpo6g",       0xfff80000, 0, SROM19_SB20IN40HRLRPO6G,       0xffff},
	/* 6G high rate power offset, per subband  */
	{"sb20in80and160hr6gb0po", 0xfff80000, 0, SROM19_SB20IN80AND160HR6GB0PO, 0xffff}, /* sb0 */
	{"sb40and80hr6gb0po",      0xfff80000, 0, SROM19_SB40AND80HR6GB0PO,      0xffff},
	{"sb20in80and160hr6gb1po", 0xfff80000, 0, SROM19_SB20IN80AND160HR6GB1PO, 0xffff}, /* sb1 */
	{"sb40and80hr6gb1po",      0xfff80000, 0, SROM19_SB40AND80HR6GB1PO,      0xffff},
	{"sb20in80and160hr6gb2po", 0xfff80000, 0, SROM19_SB20IN80AND160HR6GB2PO, 0xffff}, /* sb2 */
	{"sb40and80hr6gb2po",      0xfff80000, 0, SROM19_SB40AND80HR6GB2PO,      0xffff},
	{"sb20in80and160hr6gb3po", 0xfff80000, 0, SROM19_SB20IN80AND160HR6GB3PO, 0xffff}, /* sb3 */
	{"sb40and80hr6gb3po",      0xfff80000, 0, SROM19_SB40AND80HR6GB3PO,      0xffff},
	{"sb20in80and160hr6gb4po", 0xfff80000, 0, SROM19_SB20IN80AND160HR6GB4PO, 0xffff}, /* sb4 */
	{"sb40and80hr6gb4po",      0xfff80000, 0, SROM19_SB40AND80HR6GB4PO,      0xffff},
	{"sb20in80and160hr6gb5po", 0xfff80000, 0, SROM19_SB20IN80AND160HR6GB5PO, 0xffff}, /* sb5 */
	{"sb40and80hr6gb5po",      0xfff80000, 0, SROM19_SB40AND80HR6GB5PO,      0xffff},
	{"sb20in80and160hr6gb6po", 0xfff80000, 0, SROM19_SB20IN80AND160HR6GB6PO, 0xffff}, /* sb6 */
	{"sb40and80hr6gb6po",      0xfff80000, 0, SROM19_SB40AND80HR6GB6PO,      0xffff},
	/* 6G low rate power offset, per subband  */
	{"sb20in80and160lr6gb0po", 0xfff80000, 0, SROM19_SB20IN80AND160LR6GB0PO, 0xffff}, /* sb0 */
	{"sb40and80lr6gb0po",      0xfff80000, 0, SROM19_SB40AND80LR6GB0PO,      0xffff},
	{"sb20in80and160lr6gb1po", 0xfff80000, 0, SROM19_SB20IN80AND160LR6GB1PO, 0xffff}, /* sb1 */
	{"sb40and80lr6gb1po",      0xfff80000, 0, SROM19_SB40AND80LR6GB1PO,      0xffff},
	{"sb20in80and160lr6gb2po", 0xfff80000, 0, SROM19_SB20IN80AND160LR6GB2PO, 0xffff}, /* sb2 */
	{"sb40and80lr6gb2po",      0xfff80000, 0, SROM19_SB40AND80LR6GB2PO,      0xffff},
	{"sb20in80and160lr6gb3po", 0xfff80000, 0, SROM19_SB20IN80AND160LR6GB3PO, 0xffff}, /* sb3 */
	{"sb40and80lr6gb3po",      0xfff80000, 0, SROM19_SB40AND80LR6GB3PO,      0xffff},
	{"sb20in80and160lr6gb4po", 0xfff80000, 0, SROM19_SB20IN80AND160LR6GB4PO, 0xffff}, /* sb4 */
	{"sb40and80lr6gb4po",      0xfff80000, 0, SROM19_SB40AND80LR6GB4PO,      0xffff},
	{"sb20in80and160lr6gb5po", 0xfff80000, 0, SROM19_SB20IN80AND160LR6GB5PO, 0xffff}, /* sb5 */
	{"sb40and80lr6gb5po",      0xfff80000, 0, SROM19_SB40AND80LR6GB5PO,      0xffff},
	{"sb20in80and160lr6gb6po", 0xfff80000, 0, SROM19_SB20IN80AND160LR6GB6PO, 0xffff}, /* sb6 */
	{"sb40and80lr6gb6po",      0xfff80000, 0, SROM19_SB40AND80LR6GB6PO,      0xffff},
	/* 6G .11a,.11g rate power offset */
	{"dot11agduphrpo6g",       0xfff80000, 0, SROM19_DOT11AGDUPHRPO6G,       0xffff}, /* hr */
	{"dot11agduplrpo6g",       0xfff80000, 0, SROM19_DOT11AGDUPLRPO6G,       0xffff}, /* lr */
	{"dot11agduphrpo6g_2",     0xfff80000, 0, SROM19_DOT11AGDUPHRPO6G_2,     0xffff}, /* hr */
	{"dot11agduplrpo6g_2",     0xfff80000, 0, SROM19_DOT11AGDUPLRPO6G_2,     0xffff}, /* lr */
	/* 6G energy detect crs treshold */
	{"ed_thresh6g",            0xfff80000, 0, SROM19_EU_EDCRSTH6G,           0x00ff},
	{"eu_edthresh6g",          0xfff80000, 0, SROM19_EU_EDCRSTH6G,           0xff00},
	/* 6G PDOFFSET X-IN-Y */
	{"pdoffset20in40m6gb0",    0xfff80000, 0, SROM19_PDOFFSET20IN40M6GB0,    0xffff}, /* sb0 */
	{"pdoffset20in40m6gb1",    0xfff80000, 0, SROM19_PDOFFSET20IN40M6GB1,    0xffff}, /* sb1 */
	{"pdoffset20in40m6gb2",    0xfff80000, 0, SROM19_PDOFFSET20IN40M6GB2,    0xffff}, /* sb2 */
	{"pdoffset20in40m6gb3",    0xfff80000, 0, SROM19_PDOFFSET20IN40M6GB3,    0xffff}, /* sb3 */
	{"pdoffset20in40m6gb4",    0xfff80000, 0, SROM19_PDOFFSET20IN40M6GB4,    0xffff}, /* sb4 */
	{"pdoffset20in40m6gb5",    0xfff80000, 0, SROM19_PDOFFSET20IN40M6GB5,    0xffff}, /* sb5 */
	{"pdoffset20in40m6gb6",    0xfff80000, 0, SROM19_PDOFFSET20IN40M6GB6,    0xffff}, /* sb6 */
	{"pdoffset20in80m6gb0",    0xfff80000, 0, SROM19_PDOFFSET20IN80M6GB0,    0xffff}, /* sb0 */
	{"pdoffset20in80m6gb1",    0xfff80000, 0, SROM19_PDOFFSET20IN80M6GB1,    0xffff}, /* sb1 */
	{"pdoffset20in80m6gb2",    0xfff80000, 0, SROM19_PDOFFSET20IN80M6GB2,    0xffff}, /* sb2 */
	{"pdoffset20in80m6gb3",    0xfff80000, 0, SROM19_PDOFFSET20IN80M6GB3,    0xffff}, /* sb3 */
	{"pdoffset20in80m6gb4",    0xfff80000, 0, SROM19_PDOFFSET20IN80M6GB4,    0xffff}, /* sb4 */
	{"pdoffset20in80m6gb5",    0xfff80000, 0, SROM19_PDOFFSET20IN80M6GB5,    0xffff}, /* sb5 */
	{"pdoffset20in80m6gb6",    0xfff80000, 0, SROM19_PDOFFSET20IN80M6GB6,    0xffff}, /* sb6 */
	{"pdoffset40in80m6gb0",    0xfff80000, 0, SROM19_PDOFFSET40IN80M6GB0,    0xffff}, /* sb0 */
	{"pdoffset40in80m6gb1",    0xfff80000, 0, SROM19_PDOFFSET40IN80M6GB1,    0xffff}, /* sb1 */
	{"pdoffset40in80m6gb2",    0xfff80000, 0, SROM19_PDOFFSET40IN80M6GB2,    0xffff}, /* sb2 */
	{"pdoffset40in80m6gb3",    0xfff80000, 0, SROM19_PDOFFSET40IN80M6GB3,    0xffff}, /* sb3 */
	{"pdoffset40in80m6gb4",    0xfff80000, 0, SROM19_PDOFFSET40IN80M6GB4,    0xffff}, /* sb4 */
	{"pdoffset40in80m6gb5",    0xfff80000, 0, SROM19_PDOFFSET40IN80M6GB5,    0xffff}, /* sb5 */
	{"pdoffset40in80m6gb6",    0xfff80000, 0, SROM19_PDOFFSET40IN80M6GB6,    0xffff}, /* sb6 */
	{"pdoffset20in160m6gb0",   0xfff80000, 0, SROM19_PDOFFSET20IN160M6GB0,   0xffff}, /* sb0 */
	{"pdoffset20in160m6gb1",   0xfff80000, 0, SROM19_PDOFFSET20IN160M6GB1,   0xffff}, /* sb1 */
	{"pdoffset20in160m6gb2",   0xfff80000, 0, SROM19_PDOFFSET20IN160M6GB2,   0xffff}, /* sb2 */
	{"pdoffset20in160m6gb3",   0xfff80000, 0, SROM19_PDOFFSET20IN160M6GB3,   0xffff}, /* sb3 */
	{"pdoffset20in160m6gb4",   0xfff80000, 0, SROM19_PDOFFSET20IN160M6GB4,   0xffff}, /* sb4 */
	{"pdoffset20in160m6gb5",   0xfff80000, 0, SROM19_PDOFFSET20IN160M6GB5,   0xffff}, /* sb5 */
	{"pdoffset20in160m6gb6",   0xfff80000, 0, SROM19_PDOFFSET20IN160M6GB6,   0xffff}, /* sb6 */
	{"pdoffset40in160m6gb0",   0xfff80000, 0, SROM19_PDOFFSET40IN160M6GB0,   0xffff}, /* sb0 */
	{"pdoffset40in160m6gb1",   0xfff80000, 0, SROM19_PDOFFSET40IN160M6GB1,   0xffff}, /* sb1 */
	{"pdoffset40in160m6gb2",   0xfff80000, 0, SROM19_PDOFFSET40IN160M6GB2,   0xffff}, /* sb2 */
	{"pdoffset40in160m6gb3",   0xfff80000, 0, SROM19_PDOFFSET40IN160M6GB3,   0xffff}, /* sb3 */
	{"pdoffset40in160m6gb4",   0xfff80000, 0, SROM19_PDOFFSET40IN160M6GB4,   0xffff}, /* sb4 */
	{"pdoffset40in160m6gb5",   0xfff80000, 0, SROM19_PDOFFSET40IN160M6GB5,   0xffff}, /* sb5 */
	{"pdoffset40in160m6gb6",   0xfff80000, 0, SROM19_PDOFFSET40IN160M6GB6,   0xffff}, /* sb6 */
	{"pdoffset80in160m6gb0",   0xfff80000, 0, SROM19_PDOFFSET80IN160M6GB0,   0xffff}, /* sb0 */
	{"pdoffset80in160m6gb1",   0xfff80000, 0, SROM19_PDOFFSET80IN160M6GB1,   0xffff}, /* sb1 */
	{"pdoffset80in160m6gb2",   0xfff80000, 0, SROM19_PDOFFSET80IN160M6GB2,   0xffff}, /* sb2 */
	{"pdoffset80in160m6gb3",   0xfff80000, 0, SROM19_PDOFFSET80IN160M6GB3,   0xffff}, /* sb3 */
	{"pdoffset80in160m6gb4",   0xfff80000, 0, SROM19_PDOFFSET80IN160M6GB4,   0xffff}, /* sb4 */
	{"pdoffset80in160m6gb5",   0xfff80000, 0, SROM19_PDOFFSET80IN160M6GB5,   0xffff}, /* sb5 */
	{"pdoffset80in160m6gb6",   0xfff80000, 0, SROM19_PDOFFSET80IN160M6GB6,   0xffff}, /* sb6 */
	{"pdoffset20in40m6gcore3", 0xfff80000, 0, SROM19_PDOFFSET20IN40M6GCORE3, 0xffff},
	{"pdoffset20in40m6gcore3_1",  0xfff80000, 0, SROM19_PDOFFSET20IN40M6GCORE3_1,  0xffff},
	{"pdoffset20in40m6gcore3_2",  0xfff80000, 0, SROM19_PDOFFSET20IN40M6GCORE3_2,  0xffff},
	{"pdoffset20in80m6gcore3",    0xfff80000, 0, SROM19_PDOFFSET20IN80M6GCORE3,    0xffff},
	{"pdoffset20in80m6gcore3_1",  0xfff80000, 0, SROM19_PDOFFSET20IN80M6GCORE3_1,  0xffff},
	{"pdoffset20in80m6gcore3_2",  0xfff80000, 0, SROM19_PDOFFSET20IN80M6GCORE3_2,  0xffff},
	{"pdoffset20in160m6gcore3",   0xfff80000, 0, SROM19_PDOFFSET20IN160M6GCORE3,   0xffff},
	{"pdoffset20in160m6gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET20IN160M6GCORE3_1, 0xffff},
	{"pdoffset20in160m6gcore3_2", 0xfff80000, 0, SROM19_PDOFFSET20IN160M6GCORE3_2, 0xffff},
	{"pdoffset40in80m6gcore3",    0xfff80000, 0, SROM19_PDOFFSET40IN80M6GCORE3,    0xffff},
	{"pdoffset40in80m6gcore3_1",  0xfff80000, 0, SROM19_PDOFFSET40IN80M6GCORE3_1,  0xffff},
	{"pdoffset40in80m6gcore3_2",  0xfff80000, 0, SROM19_PDOFFSET40IN80M6GCORE3_2,  0xffff},
	{"pdoffset40in160m6gcore3",   0xfff80000, 0, SROM19_PDOFFSET40IN160M6GCORE3,   0xffff},
	{"pdoffset40in160m6gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET40IN160M6GCORE3_1, 0xffff},
	{"pdoffset40in160m6gcore3_2", 0xfff80000, 0, SROM19_PDOFFSET40IN160M6GCORE3_2, 0xffff},
	{"pdoffset80in160m6gcore3",   0xfff80000, 0, SROM19_PDOFFSET80IN160M6GCORE3,   0xffff},
	{"pdoffset80in160m6gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET80IN160M6GCORE3_1, 0xffff},
	{"pdoffset80in160m6gcore3_2", 0xfff80000, 0, SROM19_PDOFFSET80IN160M6GCORE3_2, 0xffff},
	{"pdoffset20in320m6gb0",   0xfff80000, 0, SROM19_PDOFFSET20IN320M6GB0, 0xffff}, /* sb0 */
	{"pdoffset20in320m6gb1",   0xfff80000, 0, SROM19_PDOFFSET20IN320M6GB1, 0xffff}, /* sb1 */
	{"pdoffset20in320m6gb2",   0xfff80000, 0, SROM19_PDOFFSET20IN320M6GB2, 0xffff}, /* sb2 */
	{"pdoffset20in320m6gb3",   0xfff80000, 0, SROM19_PDOFFSET20IN320M6GB3, 0xffff}, /* sb3 */
	{"pdoffset20in320m6gb4",   0xfff80000, 0, SROM19_PDOFFSET20IN320M6GB4, 0xffff}, /* sb4 */
	{"pdoffset20in320m6gb5",   0xfff80000, 0, SROM19_PDOFFSET20IN320M6GB5, 0xffff}, /* sb5 */
	{"pdoffset40in320m6gb0",   0xfff80000, 0, SROM19_PDOFFSET40IN320M6GB0, 0xffff}, /* sb0 */
	{"pdoffset40in320m6gb1",   0xfff80000, 0, SROM19_PDOFFSET40IN320M6GB1, 0xffff}, /* sb1 */
	{"pdoffset40in320m6gb2",   0xfff80000, 0, SROM19_PDOFFSET40IN320M6GB2, 0xffff}, /* sb2 */
	{"pdoffset40in320m6gb3",   0xfff80000, 0, SROM19_PDOFFSET40IN320M6GB3, 0xffff}, /* sb3 */
	{"pdoffset40in320m6gb4",   0xfff80000, 0, SROM19_PDOFFSET40IN320M6GB4, 0xffff}, /* sb4 */
	{"pdoffset40in320m6gb5",   0xfff80000, 0, SROM19_PDOFFSET40IN320M6GB5, 0xffff}, /* sb5 */
	{"pdoffset80in320m6gb0",   0xfff80000, 0, SROM19_PDOFFSET80IN320M6GB0, 0xffff}, /* sb0 */
	{"pdoffset80in320m6gb1",   0xfff80000, 0, SROM19_PDOFFSET80IN320M6GB1, 0xffff}, /* sb1 */
	{"pdoffset80in320m6gb2",   0xfff80000, 0, SROM19_PDOFFSET80IN320M6GB2, 0xffff}, /* sb2 */
	{"pdoffset80in320m6gb3",   0xfff80000, 0, SROM19_PDOFFSET80IN320M6GB3, 0xffff}, /* sb3 */
	{"pdoffset80in320m6gb4",   0xfff80000, 0, SROM19_PDOFFSET80IN320M6GB4, 0xffff}, /* sb4 */
	{"pdoffset80in320m6gb5",   0xfff80000, 0, SROM19_PDOFFSET80IN320M6GB5, 0xffff}, /* sb5 */
	{"pdoffset160in320m6gb0",   0xfff80000, 0, SROM19_PDOFFSET160IN320M6GB0, 0xffff}, /* sb0 */
	{"pdoffset160in320m6gb1",   0xfff80000, 0, SROM19_PDOFFSET160IN320M6GB1, 0xffff}, /* sb1 */
	{"pdoffset160in320m6gb2",   0xfff80000, 0, SROM19_PDOFFSET160IN320M6GB2, 0xffff}, /* sb2 */
	{"pdoffset160in320m6gb3",   0xfff80000, 0, SROM19_PDOFFSET160IN320M6GB3, 0xffff}, /* sb3 */
	{"pdoffset160in320m6gb4",   0xfff80000, 0, SROM19_PDOFFSET160IN320M6GB4, 0xffff}, /* sb4 */
	{"pdoffset160in320m6gb5",   0xfff80000, 0, SROM19_PDOFFSET160IN320M6GB5, 0xffff}, /* sb5 */
	{"pdoffset20in320m6gcore3",   0xfff80000, 0, SROM19_PDOFFSET20IN320M6GCORE3,     0xffff},
	{"pdoffset20in320m6gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET20IN320M6GCORE3_1,   0xffff},
	{"pdoffset20in320m6gcore3_2", 0xfff80000, 0, SROM19_PDOFFSET20IN320M6GCORE3_2,   0xffff},
	{"pdoffset40in320m6gcore3",   0xfff80000, 0, SROM19_PDOFFSET40IN320M6GCORE3,     0xffff},
	{"pdoffset40in320m6gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET40IN320M6GCORE3_1,   0xffff},
	{"pdoffset40in320m6gcore3_2", 0xfff80000, 0, SROM19_PDOFFSET40IN320M6GCORE3_2,   0xffff},
	{"pdoffset80in320m6gcore3",   0xfff80000, 0, SROM19_PDOFFSET80IN320M6GCORE3,     0xffff},
	{"pdoffset80in320m6gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET80IN320M6GCORE3_1,   0xffff},
	{"pdoffset80in320m6gcore3_2", 0xfff80000, 0, SROM19_PDOFFSET80IN320M6GCORE3_2,   0xffff},
	{"pdoffset160in320m6gcore3",   0xfff80000, 0, SROM19_PDOFFSET160IN320M6GCORE3,   0xffff},
	{"pdoffset160in320m6gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET160IN320M6GCORE3_1, 0xffff},
	{"pdoffset160in320m6gcore3_2", 0xfff80000, 0, SROM19_PDOFFSET160IN320M6GCORE3_2, 0xffff},

	{"swctrlmap4_tx6g_fem3to0",    0xfff80000, 0, SROM19_SWCTRLMAP4_TX6G_FEM3TO0,    0xffff},
	{"swctrlmap4_rx6g_fem3to0",    0xfff80000, 0, SROM19_SWCTRLMAP4_RX6G_FEM3TO0,    0xffff},
	{"swctrlmap4_rxbyp6g_fem3to0", 0xfff80000, 0, SROM19_SWCTRLMAP4_RXBYP6G_FEM3TO0, 0xffff},
	{"swctrlmap4_misc6g_fem3to0",  0xfff80000, 0, SROM19_SWCTRLMAP4_MISC6G_FEM3TO0,  0xffff},

	{"ru106ppr_6g_0",              0xfff80000, 0, SROM19_RU106PPR_6G_0,              0xffff},
	{"ru106ppr_6g_1",              0xfff80000, 0, SROM19_RU106PPR_6G_1,              0xffff},
	{"ru106ppr_6g_2",              0xfff80000, 0, SROM19_RU106PPR_6G_2,              0xffff},
	{"ru242ppr_6g_0",              0xfff80000, 0, SROM19_RU242PPR_6G_0,              0xffff},
	{"ru242ppr_6g_1",              0xfff80000, 0, SROM19_RU242PPR_6G_1,              0xffff},
	{"ru242ppr_6g_2",              0xfff80000, 0, SROM19_RU242PPR_6G_2,              0xffff},
	{"ru484ppr_6g_0",              0xfff80000, 0, SROM19_RU484PPR_6G_0,              0xffff},
	{"ru484ppr_6g_1",              0xfff80000, 0, SROM19_RU484PPR_6G_1,              0xffff},
	{"ru484ppr_6g_2",              0xfff80000, 0, SROM19_RU484PPR_6G_2,              0xffff},
	{"ru996ppr_6g_0",              0xfff80000, 0, SROM19_RU996PPR_6G_0,              0xffff},
	{"ru996ppr_6g_1",              0xfff80000, 0, SROM19_RU996PPR_6G_1,              0xffff},
	{"ru996ppr_6g_2",              0xfff80000, 0, SROM19_RU996PPR_6G_2,              0xffff},
	{"avvmid_2g_0",                0xfff80000, 0, SROM19_AVVMID_2G_0,                0xffff},
	{"avvmid_2g_1",                0xfff80000, 0, SROM19_AVVMID_2G_1,                0xffff},
	{"avvmid_2g_2",                0xfff80000, 0, SROM19_AVVMID_2G_2,                0xffff},
	{"avvmid_2g_3",                0xfff80000, 0, SROM19_AVVMID_2G_3,                0xffff},
	{"avvmid_5gb0_0",              0xfff80000, 0, SROM19_AVVMID_5GB0_0,              0xffff},
	{"avvmid_5gb0_1",              0xfff80000, 0, SROM19_AVVMID_5GB0_1,              0xffff},
	{"avvmid_5gb0_2",              0xfff80000, 0, SROM19_AVVMID_5GB0_2,              0xffff},
	{"avvmid_5gb0_3",              0xfff80000, 0, SROM19_AVVMID_5GB0_3,              0xffff},
	{"avvmid_5gb1_0",              0xfff80000, 0, SROM19_AVVMID_5GB1_0,              0xffff},
	{"avvmid_5gb1_1",              0xfff80000, 0, SROM19_AVVMID_5GB1_1,              0xffff},
	{"avvmid_5gb1_2",              0xfff80000, 0, SROM19_AVVMID_5GB1_2,              0xffff},
	{"avvmid_5gb1_3",              0xfff80000, 0, SROM19_AVVMID_5GB1_3,              0xffff},
	{"avvmid_5gb2_0",              0xfff80000, 0, SROM19_AVVMID_5GB2_0,              0xffff},
	{"avvmid_5gb2_1",              0xfff80000, 0, SROM19_AVVMID_5GB2_1,              0xffff},
	{"avvmid_5gb2_2",              0xfff80000, 0, SROM19_AVVMID_5GB2_2,              0xffff},
	{"avvmid_5gb2_3",              0xfff80000, 0, SROM19_AVVMID_5GB2_3,              0xffff},
	{"avvmid_5gb3_0",              0xfff80000, 0, SROM19_AVVMID_5GB3_0,              0xffff},
	{"avvmid_5gb3_1",              0xfff80000, 0, SROM19_AVVMID_5GB3_1,              0xffff},
	{"avvmid_5gb3_2",              0xfff80000, 0, SROM19_AVVMID_5GB3_2,              0xffff},
	{"avvmid_5gb3_3",              0xfff80000, 0, SROM19_AVVMID_5GB3_3,              0xffff},
	{"avvmid_6gb0_0",              0xfff80000, 0, SROM19_AVVMID_6GB0_0,              0xffff},
	{"avvmid_6gb0_1",              0xfff80000, 0, SROM19_AVVMID_6GB0_1,              0xffff},
	{"avvmid_6gb0_2",              0xfff80000, 0, SROM19_AVVMID_6GB0_2,              0xffff},
	{"avvmid_6gb0_3",              0xfff80000, 0, SROM19_AVVMID_6GB0_3,              0xffff},
	{"avvmid_6gb1_0",              0xfff80000, 0, SROM19_AVVMID_6GB1_0,              0xffff},
	{"avvmid_6gb1_1",              0xfff80000, 0, SROM19_AVVMID_6GB1_1,              0xffff},
	{"avvmid_6gb1_2",              0xfff80000, 0, SROM19_AVVMID_6GB1_2,              0xffff},
	{"avvmid_6gb1_3",              0xfff80000, 0, SROM19_AVVMID_6GB1_3,              0xffff},
	{"avvmid_6gb2_0",              0xfff80000, 0, SROM19_AVVMID_6GB2_0,              0xffff},
	{"avvmid_6gb2_1",              0xfff80000, 0, SROM19_AVVMID_6GB2_1,              0xffff},
	{"avvmid_6gb2_2",              0xfff80000, 0, SROM19_AVVMID_6GB2_2,              0xffff},
	{"avvmid_6gb2_3",              0xfff80000, 0, SROM19_AVVMID_6GB2_3,              0xffff},
	{"avvmid_6gb3_0",              0xfff80000, 0, SROM19_AVVMID_6GB3_0,              0xffff},
	{"avvmid_6gb3_1",              0xfff80000, 0, SROM19_AVVMID_6GB3_1,              0xffff},
	{"avvmid_6gb3_2",              0xfff80000, 0, SROM19_AVVMID_6GB3_2,              0xffff},
	{"avvmid_6gb3_3",              0xfff80000, 0, SROM19_AVVMID_6GB3_3,              0xffff},
	{"pddet_tssidelay_2g",         0xfff80000, 0, SROM19_PDDET_TSSIDELAY_2G,         0xffff},
	{"pddet_tssidelay_5g",         0xfff80000, 0, SROM19_PDDET_TSSIDELAY_5G,         0xffff},
	{"pddet_tssidelay_6g",         0xfff80000, 0, SROM19_PDDET_TSSIDELAY_6G,         0xffff},
	{"mcs4kqam2gpo",               0xfff80000, 0, SROM19_MCS4KQAM2GPO,               0xffff},
	{"mcs4kqam5glpo",              0xfff80000, SRFL_MORE, SROM19_MCS4KQAM5GLPO,      0xffff},
	{"",                           0xfff80000, 0, SROM19_MCS4KQAM5GLPO_1,             0xffff},
	{"mcs4kqam5gmpo",              0xfff80000, SRFL_MORE, SROM19_MCS4KQAM5GMPO,      0xffff},
	{"",                           0xfff80000, 0, SROM19_MCS4KQAM5GMPO_1,            0xffff},
	{"mcs4kqam5ghpo",              0xfff80000, SRFL_MORE, SROM19_MCS4KQAM5GHPO,      0xffff},
	{"",                           0xfff80000, 0, SROM19_MCS4KQAM5GHPO_1,            0xffff},
	{"mcs4kqam5gx1po",             0xfff80000, SRFL_MORE, SROM19_MCS4KQAM5GX1PO,     0xffff},
	{"",                           0xfff80000, 0, SROM19_MCS4KQAM5GX1PO_1,           0xffff},
	{"mcs4kqam5gx2po",             0xfff80000, SRFL_MORE, SROM19_MCS4KQAM5GX2PO,     0xffff},
	{"",                           0xfff80000, 0, SROM19_MCS4KQAM5GX2PO_1,           0xffff},
	{"mcs4kqam5gx3po",             0xfff80000, SRFL_MORE, SROM19_MCS4KQAM5GX3PO,     0xffff},
	{"",                           0xfff80000, 0, SROM19_MCS4KQAM5GX3PO_1,           0xffff},
	{"mcs4kqam5gx4po",             0xfff80000, SRFL_MORE, SROM19_MCS4KQAM5GX4PO,     0xffff},
	{"",                           0xfff80000, 0, SROM19_MCS4KQAM5GX4PO_1,           0xffff},
	{"mcs12poexp",                 0xfff80000, SRFL_MORE, SROM19_MCS12POEXP,         0xffff},
	{"",                           0xfff80000, 0, SROM19_MCS12POEXP_1,               0xffff},
	{"mcs13poexp",                 0xfff80000, SRFL_MORE, SROM19_MCS13POEXP,         0xffff},
	{"",                           0xfff80000, 0, SROM19_MCS13POEXP_1,               0xffff},
	{"bw320poexp6g",               0xfff80000, 0, SROM19_BW320POEXP6G,               0xffff},
	{"bw320poexp6g_1",             0xfff80000, 0, SROM19_BW320POEXP6G_1,             0xffff},
	{"bw320poexp6g_2",             0xfff80000, 0, SROM19_BW320POEXP6G_2,             0xffff},
	{"mcsbw3206gb0po",             0xfff80000, SRFL_MORE, SROM19_MCSBW3206GB0PO,     0xffff},
	{"",                           0xfff80000, 0, SROM19_MCSBW3206GB0PO_1,           0xffff},
	{"mcsbw3206gb1po",             0xfff80000, SRFL_MORE, SROM19_MCSBW3206GB1PO,     0xffff},
	{"",                           0xfff80000, 0, SROM19_MCSBW3206GB1PO_1,           0xffff},
	{"mcsbw3206gb2po",             0xfff80000, SRFL_MORE, SROM19_MCSBW3206GB2PO,     0xffff},
	{"",                           0xfff80000, 0, SROM19_MCSBW3206GB2PO_1,           0xffff},
	{"mcsbw3206gb3po",             0xfff80000, SRFL_MORE, SROM19_MCSBW3206GB3PO,     0xffff},
	{"",                           0xfff80000, 0, SROM19_MCSBW3206GB3PO_1,           0xffff},
	{"mcsbw3206gb4po",             0xfff80000, SRFL_MORE, SROM19_MCSBW3206GB4PO,     0xffff},
	{"",                           0xfff80000, 0, SROM19_MCSBW3206GB4PO_1,           0xffff},
	{"mcsbw3206gb5po",             0xfff80000, SRFL_MORE, SROM19_MCSBW3206GB5PO,     0xffff},
	{"",                           0xfff80000, 0, SROM19_MCSBW3206GB5PO_1,           0xffff},
	{"mcsbw3201kqampo",            0xfff80000, SRFL_MORE, SROM19_MCSBW3201KQAMPO,    0xffff},
	{"",                           0xfff80000, 0, SROM19_MCSBW3201KQAMPO_1,          0xffff},
	{"mcsbw3201kqampo_h",          0xfff80000, SRFL_MORE, SROM19_MCSBW3201KQAMPO_H,  0xffff},
	{"",                           0xfff80000, 0, SROM19_MCSBW3201KQAMPO_H_1,        0xffff},
	{"mcsbw3204kqampo",            0xfff80000, SRFL_MORE, SROM19_MCSBW3204KQAMPO,    0xffff},
	{"",                           0xfff80000, 0, SROM19_MCSBW3204KQAMPO_1,          0xffff},
	{"mcsbw3204kqampo_h",          0xfff80000, SRFL_MORE, SROM19_MCSBW3204KQAMPO_H,  0xffff},
	{"",                           0xfff80000, 0, SROM19_MCSBW3204KQAMPO_H_1,        0xffff},
	{"pdoffset60in80m5gb0",   0xfff80000, 0, SROM19_PDOFFSET60IN80M5GB0, 0xffff}, /* sb0 */
	{"pdoffset60in80m5gb1",   0xfff80000, 0, SROM19_PDOFFSET60IN80M5GB1, 0xffff}, /* sb1 */
	{"pdoffset60in80m5gb2",   0xfff80000, 0, SROM19_PDOFFSET60IN80M5GB2, 0xffff}, /* sb2 */
	{"pdoffset60in80m5gb3",   0xfff80000, 0, SROM19_PDOFFSET60IN80M5GB3, 0xffff}, /* sb3 */
	{"pdoffset60in80m5gb4",   0xfff80000, 0, SROM19_PDOFFSET60IN80M5GB4, 0xffff}, /* sb4 */
	{"pdoffset60in80m5gcore3",   0xfff80000, 0, SROM19_PDOFFSET60IN80M5GCORE3,     0xffff},
	{"pdoffset60in80m5gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET60IN80M5GCORE3_1,   0xffff},
	{"pdoffset140in160m5gb0",   0xfff80000, 0, SROM19_PDOFFSET140IN160M5GB0, 0xffff}, /* sb0 */
	{"pdoffset140in160m5gb1",   0xfff80000, 0, SROM19_PDOFFSET140IN160M5GB1, 0xffff}, /* sb1 */
	{"pdoffset140in160m5gb2",   0xfff80000, 0, SROM19_PDOFFSET140IN160M5GB2, 0xffff}, /* sb2 */
	{"pdoffset140in160m5gb3",   0xfff80000, 0, SROM19_PDOFFSET140IN160M5GB3, 0xffff}, /* sb3 */
	{"pdoffset140in160m5gb4",   0xfff80000, 0, SROM19_PDOFFSET140IN160M5GB4, 0xffff}, /* sb4 */
	{"pdoffset140in160m5gcore3",   0xfff80000, 0, SROM19_PDOFFSET140IN160M5GCORE3,     0xffff},
	{"pdoffset140in160m5gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET140IN160M5GCORE3_1,   0xffff},
	{"pdoffset120in160m5gb0",   0xfff80000, 0, SROM19_PDOFFSET120IN160M5GB0, 0xffff}, /* sb0 */
	{"pdoffset120in160m5gb1",   0xfff80000, 0, SROM19_PDOFFSET120IN160M5GB1, 0xffff}, /* sb1 */
	{"pdoffset120in160m5gb2",   0xfff80000, 0, SROM19_PDOFFSET120IN160M5GB2, 0xffff}, /* sb2 */
	{"pdoffset120in160m5gb3",   0xfff80000, 0, SROM19_PDOFFSET120IN160M5GB3, 0xffff}, /* sb3 */
	{"pdoffset120in160m5gb4",   0xfff80000, 0, SROM19_PDOFFSET120IN160M5GB4, 0xffff}, /* sb4 */
	{"pdoffset120in160m5gcore3",   0xfff80000, 0, SROM19_PDOFFSET120IN160M5GCORE3,     0xffff},
	{"pdoffset120in160m5gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET120IN160M5GCORE3_1,   0xffff},
	{"pdoffset60in80m6gb0",   0xfff80000, 0, SROM19_PDOFFSET60IN80M6GB0, 0xffff}, /* sb0 */
	{"pdoffset60in80m6gb1",   0xfff80000, 0, SROM19_PDOFFSET60IN80M6GB1, 0xffff}, /* sb1 */
	{"pdoffset60in80m6gb2",   0xfff80000, 0, SROM19_PDOFFSET60IN80M6GB2, 0xffff}, /* sb2 */
	{"pdoffset60in80m6gb3",   0xfff80000, 0, SROM19_PDOFFSET60IN80M6GB3, 0xffff}, /* sb3 */
	{"pdoffset60in80m6gb4",   0xfff80000, 0, SROM19_PDOFFSET60IN80M6GB4, 0xffff}, /* sb4 */
	{"pdoffset60in80m6gb6",   0xfff80000, 0, SROM19_PDOFFSET60IN80M6GB5, 0xffff}, /* sb5 */
	{"pdoffset60in80m6gb7",   0xfff80000, 0, SROM19_PDOFFSET60IN80M6GB6, 0xffff}, /* sb6 */
	{"pdoffset60in80m6gcore3",   0xfff80000, 0, SROM19_PDOFFSET60IN80M6GCORE3,     0xffff},
	{"pdoffset60in80m6gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET60IN80M6GCORE3_1,   0xffff},
	{"pdoffset60in80m6gcore3_2", 0xfff80000, 0, SROM19_PDOFFSET60IN80M6GCORE3_2,   0xffff},
	{"pdoffset140in160m6gb0",   0xfff80000, 0, SROM19_PDOFFSET140IN160M6GB0, 0xffff}, /* sb0 */
	{"pdoffset140in160m6gb1",   0xfff80000, 0, SROM19_PDOFFSET140IN160M6GB1, 0xffff}, /* sb1 */
	{"pdoffset140in160m6gb2",   0xfff80000, 0, SROM19_PDOFFSET140IN160M6GB2, 0xffff}, /* sb2 */
	{"pdoffset140in160m6gb3",   0xfff80000, 0, SROM19_PDOFFSET140IN160M6GB3, 0xffff}, /* sb3 */
	{"pdoffset140in160m6gb4",   0xfff80000, 0, SROM19_PDOFFSET140IN160M6GB4, 0xffff}, /* sb4 */
	{"pdoffset140in160m6gb5",   0xfff80000, 0, SROM19_PDOFFSET140IN160M6GB5, 0xffff}, /* sb5 */
	{"pdoffset140in160m6gb6",   0xfff80000, 0, SROM19_PDOFFSET140IN160M6GB6, 0xffff}, /* sb6 */
	{"pdoffset140in160m6gcore3",   0xfff80000, 0, SROM19_PDOFFSET140IN160M6GCORE3,     0xffff},
	{"pdoffset140in160m6gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET140IN160M6GCORE3_1,   0xffff},
	{"pdoffset140in160m6gcore3_2", 0xfff80000, 0, SROM19_PDOFFSET140IN160M6GCORE3_2,   0xffff},
	{"pdoffset120in160m6gb0",   0xfff80000, 0, SROM19_PDOFFSET120IN160M6GB0, 0xffff}, /* sb0 */
	{"pdoffset120in160m6gb1",   0xfff80000, 0, SROM19_PDOFFSET120IN160M6GB1, 0xffff}, /* sb1 */
	{"pdoffset120in160m6gb2",   0xfff80000, 0, SROM19_PDOFFSET120IN160M6GB2, 0xffff}, /* sb2 */
	{"pdoffset120in160m6gb3",   0xfff80000, 0, SROM19_PDOFFSET120IN160M6GB3, 0xffff}, /* sb3 */
	{"pdoffset120in160m6gb4",   0xfff80000, 0, SROM19_PDOFFSET120IN160M6GB4, 0xffff}, /* sb4 */
	{"pdoffset120in160m6gb5",   0xfff80000, 0, SROM19_PDOFFSET120IN160M6GB5, 0xffff}, /* sb5 */
	{"pdoffset120in160m6gb6",   0xfff80000, 0, SROM19_PDOFFSET120IN160M6GB6, 0xffff}, /* sb6 */
	{"pdoffset120in160m6gcore3",   0xfff80000, 0, SROM19_PDOFFSET120IN160M6GCORE3,     0xffff},
	{"pdoffset120in160m6gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET120IN160M6GCORE3_1,   0xffff},
	{"pdoffset120in160m6gcore3_2", 0xfff80000, 0, SROM19_PDOFFSET120IN160M6GCORE3_2,   0xffff},
	{"pdoffset280in320m6gb0",   0xfff80000, 0, SROM19_PDOFFSET280IN320M6GB0, 0xffff}, /* sb0 */
	{"pdoffset280in320m6gb1",   0xfff80000, 0, SROM19_PDOFFSET280IN320M6GB1, 0xffff}, /* sb1 */
	{"pdoffset280in320m6gb2",   0xfff80000, 0, SROM19_PDOFFSET280IN320M6GB2, 0xffff}, /* sb2 */
	{"pdoffset280in320m6gb3",   0xfff80000, 0, SROM19_PDOFFSET280IN320M6GB3, 0xffff}, /* sb3 */
	{"pdoffset280in320m6gb4",   0xfff80000, 0, SROM19_PDOFFSET280IN320M6GB4, 0xffff}, /* sb4 */
	{"pdoffset280in320m6gb5",   0xfff80000, 0, SROM19_PDOFFSET280IN320M6GB5, 0xffff}, /* sb5 */
	{"pdoffset280in320m6gcore3",   0xfff80000, 0, SROM19_PDOFFSET280IN320M6GCORE3,     0xffff},
	{"pdoffset280in320m6gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET280IN320M6GCORE3_1,   0xffff},
	{"pdoffset280in320m6gcore3_2", 0xfff80000, 0, SROM19_PDOFFSET280IN320M6GCORE3_2,   0xffff},
	{"pdoffset240in320m6gb0",   0xfff80000, 0, SROM19_PDOFFSET240IN320M6GB0, 0xffff}, /* sb0 */
	{"pdoffset240in320m6gb1",   0xfff80000, 0, SROM19_PDOFFSET240IN320M6GB1, 0xffff}, /* sb1 */
	{"pdoffset240in320m6gb2",   0xfff80000, 0, SROM19_PDOFFSET240IN320M6GB2, 0xffff}, /* sb2 */
	{"pdoffset240in320m6gb3",   0xfff80000, 0, SROM19_PDOFFSET240IN320M6GB3, 0xffff}, /* sb3 */
	{"pdoffset240in320m6gb4",   0xfff80000, 0, SROM19_PDOFFSET240IN320M6GB4, 0xffff}, /* sb4 */
	{"pdoffset240in320m6gb5",   0xfff80000, 0, SROM19_PDOFFSET240IN320M6GB5, 0xffff}, /* sb5 */
	{"pdoffset240in320m6gcore3",   0xfff80000, 0, SROM19_PDOFFSET240IN320M6GCORE3,     0xffff},
	{"pdoffset240in320m6gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET240IN320M6GCORE3_1,   0xffff},
	{"pdoffset240in320m6gcore3_2", 0xfff80000, 0, SROM19_PDOFFSET240IN320M6GCORE3_2,   0xffff},
	{"pdoffset200in320m6gb0",   0xfff80000, 0, SROM19_PDOFFSET200IN320M6GB0, 0xffff}, /* sb0 */
	{"pdoffset200in320m6gb1",   0xfff80000, 0, SROM19_PDOFFSET200IN320M6GB1, 0xffff}, /* sb1 */
	{"pdoffset200in320m6gb2",   0xfff80000, 0, SROM19_PDOFFSET200IN320M6GB2, 0xffff}, /* sb2 */
	{"pdoffset200in320m6gb3",   0xfff80000, 0, SROM19_PDOFFSET200IN320M6GB3, 0xffff}, /* sb3 */
	{"pdoffset200in320m6gb4",   0xfff80000, 0, SROM19_PDOFFSET200IN320M6GB4, 0xffff}, /* sb4 */
	{"pdoffset200in320m6gb5",   0xfff80000, 0, SROM19_PDOFFSET200IN320M6GB5, 0xffff}, /* sb5 */
	{"pdoffset200in320m6gcore3",   0xfff80000, 0, SROM19_PDOFFSET200IN320M6GCORE3,     0xffff},
	{"pdoffset200in320m6gcore3_1", 0xfff80000, 0, SROM19_PDOFFSET200IN320M6GCORE3_1,   0xffff},
	{"pdoffset200in320m6gcore3_2", 0xfff80000, 0, SROM19_PDOFFSET200IN320M6GCORE3_2,   0xffff},
	{"ru106ppr_2g_3", 0xfff80000, 0, SROM19_RU106PPR_2G_3,   0xffff},
	{"ru242ppr_2g_3", 0xfff80000, 0, SROM19_RU242PPR_2G_3,   0xffff},
	{"ru106ppr_5g_3", 0xfff80000, 0, SROM19_RU106PPR_5G_3,   0xffff},
	{"ru242ppr_5g_3", 0xfff80000, 0, SROM19_RU242PPR_5G_3,   0xffff},
	{"ru484ppr_5g_3", 0xfff80000, 0, SROM19_RU484PPR_5G_3,   0xffff},
	{"ru996ppr_5g_3", 0xfff80000, 0, SROM19_RU996PPR_5G_3,   0xffff},
	{"ru106ppr_6g_3", 0xfff80000, 0, SROM19_RU106PPR_6G_3,   0xffff},
	{"ru242ppr_6g_3", 0xfff80000, 0, SROM19_RU242PPR_6G_3,   0xffff},
	{"ru484ppr_6g_3", 0xfff80000, 0, SROM19_RU484PPR_6G_3,   0xffff},
	{"ru996ppr_6g_3", 0xfff80000, 0, SROM19_RU996PPR_6G_3,   0xffff},
	{"ru2x996ppr_6g_0", 0xfff80000, 0, SROM19_RU2X996PPR_6G_0,   0xffff},
	{"ru2x996ppr_6g_1", 0xfff80000, 0, SROM19_RU2X996PPR_6G_1,   0xffff},
	{"ru2x996ppr_6g_2", 0xfff80000, 0, SROM19_RU2X996PPR_6G_2,   0xffff},
	{"ru2x996ppr_6g_3", 0xfff80000, 0, SROM19_RU2X996PPR_6G_3,   0xffff},
	{"ag6g0", 0xfff80000,	0, SROM19_AG6G0, 0x00ff},
	{"ag6g1", 0xfff80000,	0, SROM19_AG6G0, 0xff00},
	{"ag6g2", 0xfff80000,	0, SROM19_AG6G1, 0x00ff},
	{"ag6g3", 0xfff80000,	0, SROM19_AG6G1, 0xff00},

	/* WBPAPD */
	{"wb_rxattn_6g",               0xfff80000, SRFL_PRHEX, SROM19_WB_RXATTN_6G,     0x00ff},
	{"wb_tia_gain_mode_6g",        0xfff80000, SRFL_PRHEX, SROM19_WB_RXATTN_6G,     0xff00},
	{"wb_rxattn_pa1",              0xfff80000, SRFL_PRHEX, SROM19_WBRXATTNPA1,      0xffff},
	{"wb_tia_gain_mode_pa1",       0xfff80000, SRFL_PRHEX, SROM19_WBTIAGAINMODEPA1, 0xffff},
	{"wb_rxattn_6gpa1",            0xfff80000, SRFL_PRHEX, SROM19_WBRXATTN6GPA1,    0x00ff},
	{"wb_tia_gain_mode_6gpa1",     0xfff80000, SRFL_PRHEX, SROM19_WBRXATTN6GPA1,    0xff00},
	{"wb_txbuf_offset_5gbw160",    0xfff80000, 0, SROM19_WBTXBUFOFFSET5GBW160,      0x00ff},
	{"wb_txbuf_offset_6g",         0xfff80000, 0, SROM19_WBTXBUFOFFSET5GBW160,      0xff00},
	{"wb_extra_epsoffset2g",
	0xfff80000, SRFL_PRSIGN|SRFL_ARRAY,      SROM19_WBEXTRAEPSOFFSET2GPA0,   0x001f},
	{"", 0xfff80000, SRFL_PRSIGN,            SROM19_WBEXTRAEPSOFFSET2GPA0,   0x03e0},
	{"wb_extra_epsoffset2gpa1",
	0xfff80000, SRFL_PRSIGN|SRFL_ARRAY,      SROM19_WBEXTRAEPSOFFSET2GPA1,   0x001f},
	{"", 0xfff80000, SRFL_PRSIGN,            SROM19_WBEXTRAEPSOFFSET2GPA1,   0x03e0},
	{"wb_extra_epsoffset5g",
	0xfff80000, SRFL_PRSIGN|SRFL_ARRAY,      SROM19_WBEXTRAEPSOFFSET5GPA0,   0x001f},
	{"", 0xfff80000, SRFL_PRSIGN|SRFL_ARRAY, SROM19_WBEXTRAEPSOFFSET5GPA0,   0x03e0},
	{"", 0xfff80000, SRFL_PRSIGN|SRFL_ARRAY, SROM19_WBEXTRAEPSOFFSET5GPA0,   0x7c00},
	{"", 0xfff80000, SRFL_PRSIGN,            SROM19_WBEXTRAEPSOFFSET2GPA0,   0x7c00},
	{"wb_extra_epsoffset5gpa1",
	0xfff80000, SRFL_PRSIGN|SRFL_ARRAY,      SROM19_WBEXTRAEPSOFFSET5GPA1,   0x001f},
	{"", 0xfff80000, SRFL_PRSIGN|SRFL_ARRAY, SROM19_WBEXTRAEPSOFFSET5GPA1,   0x03e0},
	{"", 0xfff80000, SRFL_PRSIGN|SRFL_ARRAY, SROM19_WBEXTRAEPSOFFSET5GPA1,   0x7c00},
	{"", 0xfff80000, SRFL_PRSIGN,            SROM19_WBEXTRAEPSOFFSET2GPA1,   0x7c00},
	{"wb_extra_epsoffset6g",
	0xfff80000, SRFL_PRSIGN|SRFL_ARRAY,      SROM19_WBEXTRAEPSOFFSET6GPA0,   0x001f},
	{"", 0xfff80000, SRFL_PRSIGN|SRFL_ARRAY, SROM19_WBEXTRAEPSOFFSET6GPA0,   0x03e0},
	{"", 0xfff80000, SRFL_PRSIGN|SRFL_ARRAY, SROM19_WBEXTRAEPSOFFSET6GPA0,   0x7c00},
	{"", 0xfff80000, SRFL_PRSIGN|SRFL_ARRAY, SROM19_WBEXTRAEPSOFFSET6GPA0_1, 0x001f},
	{"", 0xfff80000, SRFL_PRSIGN,            SROM19_WBEXTRAEPSOFFSET6GPA0_1, 0x03e0},
	{"wb_extra_epsoffset6gpa1",
	0xfff80000, SRFL_PRSIGN|SRFL_ARRAY,      SROM19_WBEXTRAEPSOFFSET6GPA1,   0x001f},
	{"", 0xfff80000, SRFL_PRSIGN|SRFL_ARRAY, SROM19_WBEXTRAEPSOFFSET6GPA1,   0x03e0},
	{"", 0xfff80000, SRFL_PRSIGN|SRFL_ARRAY, SROM19_WBEXTRAEPSOFFSET6GPA1,   0x7c00},
	{"", 0xfff80000, SRFL_PRSIGN|SRFL_ARRAY, SROM19_WBEXTRAEPSOFFSET6GPA1_1, 0x001f},
	{"", 0xfff80000, SRFL_PRSIGN,            SROM19_WBEXTRAEPSOFFSET6GPA1_1, 0x03e0},
	{"wb_papdcalidx",              0xfff80000, SRFL_PRHEX, SROM19_WBPAPDCALIDX2G5GPA0, 0xffff},
	{"wb_papdcalidx_pa1",          0xfff80000, SRFL_PRHEX, SROM19_WBPAPDCALIDX2G5GPA1, 0xffff},
	{"wb_papdcalidx_6g",           0xfff80000, SRFL_PRHEX, SROM19_WBPAPDCALIDX6GPA0,   0x00ff},
	{"wb_papdcalidx_6gbw320",      0xfff80000, SRFL_PRHEX, SROM19_WBPAPDCALIDX6GPA0,   0xff00},
	{"wb_papdcalidx_6gpa1",        0xfff80000, SRFL_PRHEX, SROM19_WBPAPDCALIDX6GPA1,   0x00ff},
	{"wb_papdcalidx_6gbw320pa1",   0xfff80000, SRFL_PRHEX, SROM19_WBPAPDCALIDX6GPA1,   0xff00},
	{"wb_calref_db",               0xfff80000, SRFL_PRHEX, SROM19_WBCALREFDB2G5GPA0,   0xffff},
	{"wb_calref_db_pa1",           0xfff80000, SRFL_PRHEX, SROM19_WBCALREFDB2G5GPA1,   0xffff},
	{"wb_calref_db_6g",            0xfff80000, SRFL_PRHEX, SROM19_WBCALREFDB6GPA0,     0x00ff},
	{"wb_calref_db_6gbw320",       0xfff80000, SRFL_PRHEX, SROM19_WBCALREFDB6GPA0,     0xff00},
	{"wb_calref_db_6gpa1",         0xfff80000, SRFL_PRHEX, SROM19_WBCALREFDB6GPA1,     0x00ff},
	{"wb_calref_db_6gbw320pa1",    0xfff80000, SRFL_PRHEX, SROM19_WBCALREFDB6GPA1,     0xff00},
	{"wb_const_pow_scale",         0xfff80000, SRFL_ARRAY, SROM19_WBCONSTPOWSCALE,     0x00ff},
	{"",                           0xfff80000, SRFL_ARRAY, SROM19_WBCONSTPOWSCALE,     0xff00},
	{"",                           0xfff80000, 0, SROM19_WBWINDOWLENPOW,               0xff00},
	{"wb_window_len_pow",          0xfff80000, 0, SROM19_WBWINDOWLENPOW,               0x00ff},
	{"wb_target_peak_rx_2g",       0xfff80000, SRFL_ARRAY, SROM19_WBTARGETPEAKRX2GPA0, 0xffff},
	{"",                           0xfff80000, 0, SROM19_WBTARGETPEAKRX2GPA1,          0xffff},
	{"wb_target_peak_rx_5g",       0xfff80000, SRFL_ARRAY, SROM19_WBTARGETPEAKRX5GPA0, 0xffff},
	{"",                           0xfff80000, 0, SROM19_WBTARGETPEAKRX5GPA1,          0xffff},
	{"wb_target_peak_rx_6g",       0xfff80000, SRFL_ARRAY, SROM19_WBTARGETPEAKRX6GPA0, 0xffff},
	{"",                           0xfff80000, 0, SROM19_WBTARGETPEAKRX6GPA1,          0xffff},
	{"wb_target_stop_index_2g",    0xfff80000, SRFL_ARRAY, SROM19_WBTARGETSTOPINDEX2G, 0x00ff},
	{"",                           0xfff80000, 0,          SROM19_WBTARGETSTOPINDEX2G, 0xff00},
	{"wb_target_stop_index_5g",    0xfff80000, SRFL_ARRAY, SROM19_WBTARGETSTOPINDEX5G, 0x00ff},
	{"",                           0xfff80000, 0,          SROM19_WBTARGETSTOPINDEX5G, 0xff00},
	{"wb_target_stop_index_6g",    0xfff80000, SRFL_ARRAY, SROM19_WBTARGETSTOPINDEX6G, 0x00ff},
	{"",                           0xfff80000, 0,          SROM19_WBTARGETSTOPINDEX6G, 0xff00},
	{"wb_wf_mode",                 0xfff80000, 0, SROM19_WBSTOPHITBLSIZEWFMODE,        0x00ff},
	{"wb_stop_hi",                 0xfff80000, 0, SROM19_WBSTOPHITBLSIZEWFMODE,        0xf000},
	{"wb_highres_en",              0xfff80000, 0, SROM19_WBSTOPHITBLSIZEWFMODE,        0x0f00},
	{"wb_txbuf_offset_6gbw160",    0xfff80000, 0, SROM19_WBTXBUFOFFSET6GBW160320,      0x00ff},
	{"wb_txbuf_offset_6gbw320",    0xfff80000, 0, SROM19_WBTXBUFOFFSET6GBW160320,      0xff00},
	{"fem_id",                     0xfff80000, SRFL_PRHEX|SRFL_ARRAY, SROM19_FEMID,    0x001f},
	{"",                           0xfff80000, SRFL_PRHEX|SRFL_ARRAY, SROM19_FEMID,    0x03e0},
	{"",                           0xfff80000, SRFL_PRHEX,            SROM19_FEMID,    0x7c00},

	/* 6G rxgains */
	{"rxgains6gband0elnagaina0",  0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE0, 0x7},
	{"rxgains6gband0elnagaina1",  0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE1, 0x7},
	{"rxgains6gband0elnagaina2",  0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE2, 0x7},
	{"rxgains6gband0elnagaina3",  0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE3, 0x7},
	{"rxgains6gband0trisoa0",     0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE0, 0x78},
	{"rxgains6gband0trisoa1",     0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE1, 0x78},
	{"rxgains6gband0trisoa2",     0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE2, 0x78},
	{"rxgains6gband0trisoa3",     0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE3, 0x78},
	{"rxgains6gband0trelnabypa0", 0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE0, 0x80},
	{"rxgains6gband0trelnabypa1", 0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE1, 0x80},
	{"rxgains6gband0trelnabypa2", 0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE2, 0x80},
	{"rxgains6gband0trelnabypa3", 0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE3, 0x80},

	{"rxgains6gband1elnagaina0",  0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE0, 0x700},
	{"rxgains6gband1elnagaina1",  0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE1, 0x700},
	{"rxgains6gband1elnagaina2",  0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE2, 0x700},
	{"rxgains6gband1elnagaina3",  0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE3, 0x700},
	{"rxgains6gband1trisoa0",     0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE0, 0x7800},
	{"rxgains6gband1trisoa1",     0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE1, 0x7800},
	{"rxgains6gband1trisoa2",     0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE2, 0x7800},
	{"rxgains6gband1trisoa3",     0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE3, 0x7800},
	{"rxgains6gband1trelnabypa0", 0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE0, 0x8000},
	{"rxgains6gband1trelnabypa1", 0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE1, 0x8000},
	{"rxgains6gband1trelnabypa2", 0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE2, 0x8000},
	{"rxgains6gband1trelnabypa3", 0xfffc0000, 0, SROM18_RXGAINS6G_0_CORE3, 0x8000},

	{"rxgains6gband2elnagaina0",  0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE0, 0x7},
	{"rxgains6gband2elnagaina1",  0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE1, 0x7},
	{"rxgains6gband2elnagaina2",  0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE2, 0x7},
	{"rxgains6gband2elnagaina3",  0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE3, 0x7},
	{"rxgains6gband2trisoa0",     0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE0, 0x78},
	{"rxgains6gband2trisoa1",     0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE1, 0x78},
	{"rxgains6gband2trisoa2",     0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE2, 0x78},
	{"rxgains6gband2trisoa3",     0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE3, 0x78},
	{"rxgains6gband2trelnabypa0", 0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE0, 0x80},
	{"rxgains6gband2trelnabypa1", 0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE1, 0x80},
	{"rxgains6gband2trelnabypa2", 0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE2, 0x80},
	{"rxgains6gband2trelnabypa3", 0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE3, 0x80},

	{"rxgains6gband3elnagaina0",  0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE0, 0x700},
	{"rxgains6gband3elnagaina1",  0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE1, 0x700},
	{"rxgains6gband3elnagaina2",  0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE2, 0x700},
	{"rxgains6gband3elnagaina3",  0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE3, 0x700},
	{"rxgains6gband3trisoa0",     0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE0, 0x7800},
	{"rxgains6gband3trisoa1",     0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE1, 0x7800},
	{"rxgains6gband3trisoa2",     0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE2, 0x7800},
	{"rxgains6gband3trisoa3",     0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE3, 0x7800},
	{"rxgains6gband3trelnabypa0", 0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE0, 0x8000},
	{"rxgains6gband3trelnabypa1", 0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE1, 0x8000},
	{"rxgains6gband3trelnabypa2", 0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE2, 0x8000},
	{"rxgains6gband3trelnabypa3", 0xfffc0000, 0, SROM18_RXGAINS6G_1_CORE3, 0x8000},

	{"rxgains6gband4elnagaina0",  0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE0, 0x7},
	{"rxgains6gband4elnagaina1",  0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE1, 0x7},
	{"rxgains6gband4elnagaina2",  0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE2, 0x7},
	{"rxgains6gband4elnagaina3",  0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE3, 0x7},
	{"rxgains6gband4trisoa0",     0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE0, 0x78},
	{"rxgains6gband4trisoa1",     0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE1, 0x78},
	{"rxgains6gband4trisoa2",     0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE2, 0x78},
	{"rxgains6gband4trisoa3",     0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE3, 0x78},
	{"rxgains6gband4trelnabypa0", 0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE0, 0x80},
	{"rxgains6gband4trelnabypa1", 0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE1, 0x80},
	{"rxgains6gband4trelnabypa2", 0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE2, 0x80},
	{"rxgains6gband4trelnabypa3", 0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE3, 0x80},

	{"rxgains6gband5elnagaina0",  0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE0, 0x700},
	{"rxgains6gband5elnagaina1",  0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE1, 0x700},
	{"rxgains6gband5elnagaina2",  0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE2, 0x700},
	{"rxgains6gband5elnagaina3",  0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE3, 0x700},
	{"rxgains6gband5trisoa0",     0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE0, 0x7800},
	{"rxgains6gband5trisoa1",     0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE1, 0x7800},
	{"rxgains6gband5trisoa2",     0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE2, 0x7800},
	{"rxgains6gband5trisoa3",     0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE3, 0x7800},
	{"rxgains6gband5trelnabypa0", 0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE0, 0x8000},
	{"rxgains6gband5trelnabypa1", 0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE1, 0x8000},
	{"rxgains6gband5trelnabypa2", 0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE2, 0x8000},
	{"rxgains6gband5trelnabypa3", 0xfffc0000, 0, SROM18_RXGAINS6G_2_CORE3, 0x8000},

	{"rxgains6gband6elnagaina0",  0xfffc0000, 0, SROM18_RXGAINS6G_3_CORE0, 0x700},
	{"rxgains6gband6elnagaina1",  0xfffc0000, 0, SROM18_RXGAINS6G_3_CORE1, 0x700},
	{"rxgains6gband6elnagaina2",  0xfffc0000, 0, SROM18_RXGAINS6G_3_CORE2, 0x700},
	{"rxgains6gband6elnagaina3",  0xfffc0000, 0, SROM18_RXGAINS6G_3_CORE3, 0x700},
	{"rxgains6gband6trisoa0",     0xfffc0000, 0, SROM18_RXGAINS6G_3_CORE0, 0x7800},
	{"rxgains6gband6trisoa1",     0xfffc0000, 0, SROM18_RXGAINS6G_3_CORE1, 0x7800},
	{"rxgains6gband6trisoa2",     0xfffc0000, 0, SROM18_RXGAINS6G_3_CORE2, 0x7800},
	{"rxgains6gband6trisoa3",     0xfffc0000, 0, SROM18_RXGAINS6G_3_CORE3, 0x7800},
	{"rxgains6gband6trelnabypa0", 0xfffc0000, 0, SROM18_RXGAINS6G_3_CORE0, 0x8000},
	{"rxgains6gband6trelnabypa1", 0xfffc0000, 0, SROM18_RXGAINS6G_3_CORE1, 0x8000},
	{"rxgains6gband6trelnabypa2", 0xfffc0000, 0, SROM18_RXGAINS6G_3_CORE2, 0x8000},
	{"rxgains6gband6trelnabypa3", 0xfffc0000, 0, SROM18_RXGAINS6G_3_CORE3, 0x8000},

	/* 6G rxgainerr */
	{"rxgainerr6ga0",       0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB0,     0x003f},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB1,     0x003f},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB2,     0x003f},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB3,     0x003f},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB4,     0x003f},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB5,     0x003f},
	{"",                    0xfffc0000, 0,          SROM18_RXGAINERR6G_SB6,     0x003f},
	{"rxgainerr6ga1",       0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB0,     0x07c0},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB1,     0x07c0},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB2,     0x07c0},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB3,     0x07c0},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB4,     0x07c0},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB5,     0x07c0},
	{"",                    0xfffc0000, 0,          SROM18_RXGAINERR6G_SB6,     0x07c0},
	{"rxgainerr6ga2",       0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB0,     0xf800},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB1,     0xf800},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB2,     0xf800},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB3,     0xf800},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB4,     0xf800},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERR6G_SB5,     0xf800},
	{"",                    0xfffc0000, 0,          SROM18_RXGAINERR6G_SB6,     0xf800},
	{"rxgainerr6ga3",       0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERRCORE3_6G_0, 0x001f},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERRCORE3_6G_0, 0x03e0},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERRCORE3_6G_0, 0x7c00},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERRCORE3_6G_1, 0x001f},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERRCORE3_6G_1, 0x03e0},
	{"",                    0xfffc0000, SRFL_ARRAY, SROM18_RXGAINERRCORE3_6G_1, 0x7c00},
	{"",                    0xfffc0000, 0,          SROM18_RXGAINERRCORE3_6G_2, 0x001f},

	/* pdoffset */
	{"pdoffset20in40m5gcore3",   0xffffe000, 0,     SROM13_PDOFFSET20IN40M5GCORE3,   0xffff},
	{"pdoffset20in40m5gcore3_1", 0xffffe000, 0,     SROM13_PDOFFSET20IN40M5GCORE3_1, 0xffff},
	{"pdoffset20in80m5gcore3",   0xffffe000, 0,     SROM13_PDOFFSET20IN80M5GCORE3,   0xffff},
	{"pdoffset20in80m5gcore3_1", 0xffffe000, 0,     SROM13_PDOFFSET20IN80M5GCORE3_1, 0xffff},
	{"pdoffset40in80m5gcore3",   0xffffe000, 0,     SROM13_PDOFFSET40IN80M5GCORE3,   0xffff},
	{"pdoffset40in80m5gcore3_1", 0xffffe000, 0,     SROM13_PDOFFSET40IN80M5GCORE3_1, 0xffff},

	{"pdoffset20in40m2g",        0xffffe000, 0,     SROM13_PDOFFSET20IN40M2G,        0xffff},
	{"pdoffset20in40m2gcore3",   0xffffe000, 0,     SROM13_PDOFFSET20IN40M2GCORE3,   0xffff},
	{"pdoffsetcck20m",           0xffffe000, 0,     SROM13_PDOFF_2G_CCK_20M,         0xffff},
	{"pdoffsetcckch14",          0xffffe000, 0,     SROM13_PDOFF_2G_CCK_CH14,        0xffff},

	/* power per rate */
	{"mcs1024qam2gpo",      0xffffe000,     0,           SROM13_MCS1024QAM2GPO,     0xffff},
	{"mcs1024qam5glpo",     0xffffe000,     SRFL_MORE,   SROM13_MCS1024QAM5GLPO,    0xffff},
	{"",                    0xffffe000,     0,           SROM13_MCS1024QAM5GLPO_1,  0xffff},
	{"mcs1024qam5gmpo",     0xffffe000,     SRFL_MORE,   SROM13_MCS1024QAM5GMPO,    0xffff},
	{"",                    0xffffe000,     0,           SROM13_MCS1024QAM5GMPO_1,  0xffff},
	{"mcs1024qam5ghpo",     0xffffe000,     SRFL_MORE,   SROM13_MCS1024QAM5GHPO,    0xffff},
	{"",                    0xffffe000,     0,           SROM13_MCS1024QAM5GHPO_1,  0xffff},
	{"mcs1024qam5gx1po",    0xffffe000,     SRFL_MORE,   SROM13_MCS1024QAM5GX1PO,   0xffff},
	{"",                    0xffffe000,     0,           SROM13_MCS1024QAM5GX1PO_1, 0xffff},
	{"mcs1024qam5gx2po",    0xffffe000,     SRFL_MORE,   SROM13_MCS1024QAM5GX2PO,   0xffff},
	{"",                    0xffffe000,     0,           SROM13_MCS1024QAM5GX2PO_1, 0xffff},

	{"mcsbw1605glpo",       0xffffe000,     SRFL_MORE,      SROM13_MCSBW1605GLPO,   0xffff},
	{"",                    0xffffe000,     0,              SROM13_MCSBW1605GLPO_1, 0xffff},
	{"mcsbw1605gmpo",       0xffffe000,     SRFL_MORE,      SROM13_MCSBW1605GMPO,   0xffff},
	{"",                    0xffffe000,     0,              SROM13_MCSBW1605GMPO_1, 0xffff},
	{"mcsbw1605ghpo",       0xffffe000,     SRFL_MORE,      SROM13_MCSBW1605GHPO,   0xffff},
	{"",                    0xffffe000,     0,              SROM13_MCSBW1605GHPO_1, 0xffff},
	{"mcsbw1605gx1po",      0xffffe000,     SRFL_MORE,      SROM13_MCSBW1605GX1PO,  0xffff},
	{"",                    0xffffe000,     0,      SROM13_MCSBW1605GX1PO_1,        0xffff},
	{"mcsbw1605gx2po",      0xffffe000,     SRFL_MORE,      SROM13_MCSBW1605GX2PO,  0xffff},
	{"",                    0xffffe000,     0,      SROM13_MCSBW1605GX2PO_1,        0xffff},

	{"mcs8poexp",           0xffffe000,     SRFL_MORE,    SROM13_MCS8POEXP,         0xffff},
	{"",                    0xffffe000,     0,            SROM13_MCS8POEXP_1,       0xffff},
	{"mcs9poexp",           0xffffe000,     SRFL_MORE,    SROM13_MCS9POEXP,         0xffff},
	{"",                    0xffffe000,     0,            SROM13_MCS9POEXP_1,       0xffff},
	{"mcs10poexp",          0xffffe000,     SRFL_MORE,    SROM13_MCS10POEXP,        0xffff},
	{"",                    0xffffe000,     0,            SROM13_MCS10POEXP_1,      0xffff},
	{"mcs11poexp",          0xffffe000,     SRFL_MORE,    SROM13_MCS11POEXP,        0xffff},
	{"",                    0xffffe000,     0,            SROM13_MCS11POEXP_1,      0xffff},

	{"cckbw202gpodelta",    0xffffe000,     0,       SROM13_CCKBW202GPODELTA,        0xffff},

	{"sb20in40hrlrpox",     0xffffe000,     0,       SROM13_SB20IN40HRLRPOX,        0xffff},

	{"mcslr5gx1po",		0xfffffc00,	0,		SROM12_MCSLR5GX1PO,	0x0fff},
	{"mcslr5gx2po",		0xfffffc00,	0,		SROM12_MCSLR5GX2PO,	0xffff},
	{"mcslr5gx3po",		0xfffffc00,	0,		SROM18_MCSLR5GX3PO,	0xffff},
	{"mcslr5gx4po",		0xfffffc00,	0,		SROM18_MCSLR5GX4PO,	0xffff},
	{"mcsbw205gx3po",       0xffffc000,     SRFL_MORE,      SROM18_MCSBW205GX3PO,   0xffff},
	{"",                    0xffffc000,     0,              SROM18_MCSBW205GX3PO_1, 0xffff},
	{"mcsbw405gx3po",       0xffffc000,     SRFL_MORE,      SROM18_MCSBW405GX3PO,   0xffff},
	{"",                    0xffffc000,     0,              SROM18_MCSBW405GX3PO_1, 0xffff},
	{"mcsbw805gx3po",       0xffffc000,     SRFL_MORE,      SROM18_MCSBW805GX3PO,   0xffff},
	{"",                    0xffffc000,     0,              SROM18_MCSBW805GX3PO_1, 0xffff},
	{"mcsbw205gx4po",       0xffffc000,     SRFL_MORE,      SROM18_MCSBW205GX4PO,   0xffff},
	{"",                    0xffffc000,     0,              SROM18_MCSBW205GX4PO_1, 0xffff},
	{"mcsbw405gx4po",       0xffffc000,     SRFL_MORE,      SROM18_MCSBW405GX4PO,   0xffff},
	{"",                    0xffffc000,     0,              SROM18_MCSBW405GX4PO_1, 0xffff},
	{"mcsbw805gx4po",       0xffffc000,     SRFL_MORE,      SROM18_MCSBW805GX4PO,   0xffff},
	{"",                    0xffffc000,     0,              SROM18_MCSBW805GX4PO_1, 0xffff},

	{"sb20in80and160hr5gx3po", 0xffffc000,  0,      SROM18_SB20IN80AND160HR5GX3PO,  0xffff},
	{"sb40and80hr5gx3po",     0xffffc000,   0,      SROM18_SB40AND80HR5GX3PO,       0xffff},
	{"sb20in80and160lr5gx3po", 0xffffc000,  0,      SROM18_SB20IN80AND160LR5GX3PO,  0xffff},
	{"sb20in80and160hr5gx4po", 0xffffc000,  0,      SROM18_SB20IN80AND160HR5GX4PO,  0xffff},
	{"sb40and80hr5gx4po",     0xffffc000,   0,      SROM18_SB40AND80HR5GX4PO,       0xffff},
	{"sb20in80and160lr5gx4po", 0xffffc000,  0,      SROM18_SB20IN80AND160LR5GX4PO,  0xffff},
	{"mcsbw1605gx3po",      0xffffc000,     SRFL_MORE,      SROM18_MCSBW1605GX3PO,  0xffff},
	{"",                    0xffffc000,     0,      SROM18_MCSBW1605GX3PO_1,        0xffff},
	{"mcsbw1605gx4po",      0xffffc000,     SRFL_MORE,      SROM18_MCSBW1605GX4PO,  0xffff},
	{"",                    0xffffc000,     0,      SROM18_MCSBW1605GX4PO_1,        0xffff},
	{"mcs1024qam5gx3po",    0xffffc000,     SRFL_MORE,   SROM18_MCS1024QAM5GX3PO,   0xffff},
	{"",                    0xffffc000,     0,           SROM18_MCS1024QAM5GX3PO_1, 0xffff},
	{"mcs1024qam5gx4po",    0xffffc000,     SRFL_MORE,   SROM18_MCS1024QAM5GX4PO,   0xffff},
	{"",                    0xffffc000,     0,           SROM18_MCS1024QAM5GX4PO_1, 0xffff},
	{"sb40and80lr5gx1po",	  0xfffffc00,	0,	SROM12_SB40AND80LR5GX1PO,	0xffff},
	{"sb40and80lr5gx2po",	  0xfffffc00,	0,	SROM12_SB40AND80LR5GX2PO,	0xffff},
	{"sb40and80lr5gx3po",	  0xfffffc00,	0,	SROM18_SB40AND80LR5GX3PO,	0xffff},
	{"sb40and80lr5gx4po",	  0xfffffc00,	0,	SROM18_SB40AND80LR5GX4PO,	0xffff},

	{"swctrlmap4_cfg",      0xffffe000,     0,      SROM13_SWCTRLMAP4_CFG,          0xffff},
	{"swctrlmap4_TX2g_fem3to0", 0xffffe000, 0,      SROM13_SWCTRLMAP4_TX2G_FEM3TO0, 0xffff},
	{"swctrlmap4_RX2g_fem3to0", 0xffffe000, 0,      SROM13_SWCTRLMAP4_RX2G_FEM3TO0, 0xffff},
	{"swctrlmap4_RXByp2g_fem3to0", 0xffffe000, 0, SROM13_SWCTRLMAP4_RXBYP2G_FEM3TO0, 0xffff},
	{"swctrlmap4_misc2g_fem3to0", 0xffffe000, 0,  SROM13_SWCTRLMAP4_MISC2G_FEM3TO0, 0xffff},
	{"swctrlmap4_TX5g_fem3to0", 0xffffe000, 0,      SROM13_SWCTRLMAP4_TX5G_FEM3TO0, 0xffff},
	{"swctrlmap4_RX5g_fem3to0", 0xffffe000, 0,      SROM13_SWCTRLMAP4_RX5G_FEM3TO0, 0xffff},
	{"swctrlmap4_RXByp5g_fem3to0", 0xffffe000, 0, SROM13_SWCTRLMAP4_RXBYP5G_FEM3TO0, 0xffff},
	{"swctrlmap4_misc5g_fem3to0", 0xffffe000, 0,  SROM13_SWCTRLMAP4_MISC5G_FEM3TO0, 0xffff},
	{"swctrlmap4_TX2g_fem7to4", 0xffffe000, 0,      SROM13_SWCTRLMAP4_TX2G_FEM7TO4, 0xffff},
	{"swctrlmap4_RX2g_fem7to4", 0xffffe000, 0,      SROM13_SWCTRLMAP4_RX2G_FEM7TO4, 0xffff},
	{"swctrlmap4_RXByp2g_fem7to4", 0xffffe000, 0, SROM13_SWCTRLMAP4_RXBYP2G_FEM7TO4, 0xffff},
	{"swctrlmap4_misc2g_fem7to4", 0xffffe000, 0,  SROM13_SWCTRLMAP4_MISC2G_FEM7TO4, 0xffff},
	{"swctrlmap4_TX5g_fem7to4", 0xffffe000, 0,      SROM13_SWCTRLMAP4_TX5G_FEM7TO4, 0xffff},
	{"swctrlmap4_RX5g_fem7to4", 0xffffe000, 0,      SROM13_SWCTRLMAP4_RX5G_FEM7TO4, 0xffff},
	{"swctrlmap4_RXByp5g_fem7to4", 0xffffe000, 0, SROM13_SWCTRLMAP4_RXBYP5G_FEM7TO4, 0xffff},
	{"swctrlmap4_misc5g_fem7to4", 0xffffe000, 0,  SROM13_SWCTRLMAP4_MISC5G_FEM7TO4, 0xffff},
	{"swctrlmap4_RXmid2g_fem3to0", 0xffffc000, 0,  SROM18_SWCTRLMAP4_RXMID2G_FEM3TO0, 0xffff},
	{"swctrlmap4_RXmid5g_fem3to0", 0xffffc000, 0,  SROM18_SWCTRLMAP4_RXMID5G_FEM3TO0, 0xffff},
	{"rxmid2g_elnagain", 0xffffc000, 0,  SROM18_RXMID2G_ELNAGAIN, 0xffff},
	{"rxmid5g_elnagain", 0xffffc000, 0,  SROM18_RXMID5G_ELNAGAIN, 0xffff},
	{"ed_thresh2g", 0xffffc000, 0,  SROM18_EDCRSTH, 0x00ff},
	{"ed_thresh5g", 0xffffc000, 0,  SROM18_EDCRSTH, 0xff00},

	{"papd_swctrl",            0xfffc0000,	0, SROM18_PAPD_SWCTRL, 0xffff},
	{"papd_swctrl_5g6g",       0xfffc0000,	0, SROM18_PAPD_SWCTRL_5G6G, 0xffff},
	{"papd_swctrlmap_fem2g",   0xfffc0000,	0, SROM18_PAPD_SWCTRLMAP_FEM2G, 0xffff},
	{"papd_swctrlmap_fem5g",   0xfffc0000,	0, SROM18_PAPD_SWCTRLMAP_FEM5G, 0xffff},
	{"papd_swctrlmap_fem6g",   0xfffc0000,	0, SROM18_PAPD_SWCTRLMAP_FEM6G, 0xffff},
	{"papd_swctrlmap_femlp2g", 0xfffc0000,	0, SROM18_PAPD_SWCTRLMAP_FEMLP2G, 0xffff},
	{"papd_swctrlmap_femlp5g", 0xfffc0000,	0, SROM18_PAPD_SWCTRLMAP_FEMLP5G, 0xffff},
	{"papd_swctrlmap_femlp6g", 0xfffc0000,	0, SROM18_PAPD_SWCTRLMAP_FEMLP6G, 0xffff},

	{"pdoffset20in160m5gc0", 0xfffc0000,	0,	SROM18_PDOFFSET20IN160M5GC0,	0xffff},
	{"pdoffset20in160m5gc1", 0xfffc0000,	0,	SROM18_PDOFFSET20IN160M5GC1,	0xffff},
	{"pdoffset20in160m5gc2", 0xfffc0000,	0,	SROM18_PDOFFSET20IN160M5GC2,	0xffff},
	{"pdoffset20in160m5gc3", 0xfffc0000,	0,	SROM18_PDOFFSET20IN160M5GC3,	0xffff},
	{"pdoffset20in160m5gcore3", 0xfffc0000,	0,	SROM18_PDOFFSET20IN160M5GCORE3,	0xffff},
	{"pdoffset20in160m5gcore3_1", 0xfffc0000, 0, SROM18_PDOFFSET20IN160M5GCORE3_1,	0xffff},
	{"pdoffset40in160m5gc0", 0xfffc0000,	0,	SROM18_PDOFFSET40IN160M5GC0,	0xffff},
	{"pdoffset40in160m5gc1", 0xfffc0000,	0,	SROM18_PDOFFSET40IN160M5GC1,	0xffff},
	{"pdoffset40in160m5gc2", 0xfffc0000,	0,	SROM18_PDOFFSET40IN160M5GC2,	0xffff},
	{"pdoffset40in160m5gc3", 0xfffc0000,	0,	SROM18_PDOFFSET40IN160M5GC3,	0xffff},
	{"pdoffset40in160m5gcore3", 0xfffc0000, 0,	SROM18_PDOFFSET40IN160M5GCORE3, 0xffff},
	{"pdoffset40in160m5gcore3_1", 0xfffc0000, 0, SROM18_PDOFFSET40IN160M5GCORE3_1,	0xffff},
	{"pdoffset80in160m5gc0", 0xfffc0000,	0,	SROM18_PDOFFSET80IN160M5GC0,	0xffff},
	{"pdoffset80in160m5gc1", 0xfffc0000,	0,	SROM18_PDOFFSET80IN160M5GC1,	0xffff},
	{"pdoffset80in160m5gc2", 0xfffc0000,	0,	SROM18_PDOFFSET80IN160M5GC2,	0xffff},
	{"pdoffset80in160m5gc3", 0xfffc0000,	0,	SROM18_PDOFFSET80IN160M5GC3,	0xffff},
	{"pdoffset80in160m5gcore3", 0xfffc0000, 0,	SROM18_PDOFFSET80IN160M5GCORE3, 0xffff},
	{"pdoffset80in160m5gcore3_1", 0xfffc0000, 0, SROM18_PDOFFSET80IN160M5GCORE3_1,  0xffff},
	{"low_adc_rate_en", 0xfffc0000,	0,	SROM18_LOW_ADC_RATE_EN,	0x0003},
	{"ppr_backoff_2g", 0xfffc0000,	0,	SROM18_LOW_ADC_RATE_EN,	0x000c},
	{"ppr_backoff_5g", 0xfffc0000,	0,	SROM18_LOW_ADC_RATE_EN,	0x0030},
	{"dssf_dis_ch138", 0xfffc0000,	0,	SROM18_LOW_ADC_RATE_EN,	0x0040},
	{"ppr_backoff_6g", 0xfffc0000,	0,	SROM18_LOW_ADC_RATE_EN,	0x0300},
	{"low_adc_rate_en_6g", 0xfffc0000,	0,	SROM18_LOW_ADC_RATE_EN,	0x0c00},
	{"desense_lesiOff_2g", 0xfffc0000, 0, SROM18_LOW_ADC_RATE_EN, 0x1000},
	{"desense_lesiOff_5g", 0xfffc0000, 0, SROM18_LOW_ADC_RATE_EN, 0x2000},
	{"ed_thresh_gainerr_dis", 0xfffc0000, 0, SROM18_LOW_ADC_RATE_EN, 0x4000},
	{"olpc_2g_th",	0xfffc0000,	SRFL_PRSIGN,	SROM18_OLPC_2G5G_TH,	0x001f},
	{"olpc_5g_th",	0xfffc0000,	SRFL_PRSIGN,	SROM18_OLPC_2G5G_TH,	0x03e0},
	{"olpc_6g_th",	0xfffc0000,	SRFL_PRSIGN,	SROM18_OLPC_2G5G_TH,	0x7c00},
	{"coex_gpioctrl_0", 0xfffc0000,	0,	SROM18_COEX_GPIOCTRL_0,	0xffff},
	{"coex_gpioctrl_1", 0xfffc0000,	0,	SROM18_COEX_GPIOCTRL_1,	0xffff},
	{"coex_gpioctrl_2", 0xfffc0000,	0,	SROM18_COEX_GPIOCTRL_2,	0xffff},
	{"coex_gpioctrl_3", 0xfffc0000,	0,	SROM18_COEX_GPIOCTRL_3,	0xffff},
	{"ru106ppr_2g_0", 0xfffc0000,	0,	SROM18_RU106PPR_2G_0,	0xffff},
	{"ru106ppr_2g_1", 0xfffc0000,	0,	SROM18_RU106PPR_2G_1,	0xffff},
	{"ru106ppr_2g_2", 0xfffc0000,	0,	SROM18_RU106PPR_2G_2,	0xffff},
	{"ru242ppr_2g_0", 0xfffc0000,	0,	SROM18_RU242PPR_2G_0,	0xffff},
	{"ru242ppr_2g_1", 0xfffc0000,	0,	SROM18_RU242PPR_2G_1,	0xffff},
	{"ru242ppr_2g_2", 0xfffc0000,	0,	SROM18_RU242PPR_2G_2,	0xffff},
	{"ru106ppr_5g_0", 0xfffc0000,	0,	SROM18_RU106PPR_5G_0,	0xffff},
	{"ru106ppr_5g_1", 0xfffc0000,	0,	SROM18_RU106PPR_5G_1,	0xffff},
	{"ru106ppr_5g_2", 0xfffc0000,	0,	SROM18_RU106PPR_5G_2,	0xffff},
	{"ru242ppr_5g_0", 0xfffc0000,	0,	SROM18_RU242PPR_5G_0,	0xffff},
	{"ru242ppr_5g_1", 0xfffc0000,	0,	SROM18_RU242PPR_5G_1,	0xffff},
	{"ru242ppr_5g_2", 0xfffc0000,	0,	SROM18_RU242PPR_5G_2,	0xffff},
	{"ru484ppr_5g_0", 0xfffc0000,	0,	SROM18_RU484PPR_5G_0,	0xffff},
	{"ru484ppr_5g_1", 0xfffc0000,	0,	SROM18_RU484PPR_5G_1,	0xffff},
	{"ru484ppr_5g_2", 0xfffc0000,	0,	SROM18_RU484PPR_5G_2,	0xffff},
	{"ru996ppr_5g_0", 0xfffc0000,	0,	SROM18_RU996PPR_5G_0,	0xffff},
	{"ru996ppr_5g_1", 0xfffc0000,	0,	SROM18_RU996PPR_5G_1,	0xffff},
	{"ru996ppr_5g_2", 0xfffc0000,	0,	SROM18_RU996PPR_5G_2,	0xffff},
	{"txs_shaper_en_2g",          0xfffc0000, 0, SROM18_TXS_SHAPER_EN, 0x0001},
	{"txs_shaper_en_5g",          0xfffc0000, 0, SROM18_TXS_SHAPER_EN, 0x0002},
	{"txs_shaper_en_6g",          0xfffc0000, 0, SROM18_TXS_SHAPER_EN, 0x0004},
	{"txs_shaper_bypass",         0xfffc0000, 0, SROM18_TXS_SHAPER_EN, 0x0008},
	{"txs_chan_rate_en",          0xfffc0000, 0, SROM18_TXS_SHAPER_EN, 0x07f0},
	{"txs_shaper_dis_2g_nonbndg", 0xfffc0000, 0, SROM18_TXS_SHAPER_EN, 0x0800},
	{"txs_shaper_dis_5g_nonbndg", 0xfffc0000, 0, SROM18_TXS_SHAPER_EN, 0x1000},
	{"txs_shaper_en_unii3",       0xfffc0000, 0, SROM18_TXS_SHAPER_EN, 0x2000},
	{"txs_shaper_en_unii4",       0xfffc0000, 0, SROM18_TXS_SHAPER_EN, 0x4000},
	{"subband_ed_adj",            0xfffc0000, 0, SROM18_SUBBAND_ED_ADJ, 0xffff},
	/* HWRSSI Offset */
	{"avsflags",                  0xfffc0000, SRFL_PRHEX, SROM18_AVSFLAGS, 0xffff},
	{"hwrssioffset_cmn_2g_0", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_2G_0, 0x000f},
	{"hwrssioffset_trt_2g_0", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_2G_0, 0x00f0},
	{"hwrssioffset_cmn_2g_1", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_2G_0, 0x0f00},
	{"hwrssioffset_trt_2g_1", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_2G_0, 0xf000},
	{"hwrssioffset_cmn_2g_2", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_2G_1, 0x000f},
	{"hwrssioffset_trt_2g_2", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_2G_1, 0x00f0},
	{"hwrssioffset_cmn_2g_3", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_2G_1, 0x0f00},
	{"hwrssioffset_trt_2g_3", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_2G_1, 0xf000},
	{"hwrssioffset_cmn_5g_6g_0", 0xfffc0000, SRFL_ARRAY,
	SROM18_HWRSSIOFFSET_5G_6G_SB0_0, 0x000f},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB1_0, 0x000f},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB2_0, 0x000f},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB3_0, 0x000f},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB4_0, 0x000f},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB5_0, 0x000f},
	{"", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_5G_6G_SB6_0, 0x000f},
	{"hwrssioffset_cmn_5g_6g_1", 0xfffc0000, SRFL_ARRAY,
	SROM18_HWRSSIOFFSET_5G_6G_SB0_0, 0x0f00},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB1_0, 0x0f00},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB2_0, 0x0f00},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB3_0, 0x0f00},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB4_0, 0x0f00},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB5_0, 0x0f00},
	{"", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_5G_6G_SB6_0, 0x0f00},
	{"hwrssioffset_cmn_5g_6g_2", 0xfffc0000, SRFL_ARRAY,
	SROM18_HWRSSIOFFSET_5G_6G_SB0_1, 0x000f},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB1_1, 0x000f},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB2_1, 0x000f},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB3_1, 0x000f},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB4_1, 0x000f},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB5_1, 0x000f},
	{"", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_5G_6G_SB6_1, 0x000f},
	{"hwrssioffset_cmn_5g_6g_3", 0xfffc0000, SRFL_ARRAY,
	SROM18_HWRSSIOFFSET_5G_6G_SB0_1, 0x0f00},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB1_1, 0x0f00},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB2_1, 0x0f00},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB3_1, 0x0f00},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB4_1, 0x0f00},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB5_1, 0x0f00},
	{"", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_5G_6G_SB6_1, 0x0f00},
	{"hwrssioffset_trt_5g_6g_0", 0xfffc0000, SRFL_ARRAY,
	SROM18_HWRSSIOFFSET_5G_6G_SB0_0, 0x00f0},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB1_0, 0x00f0},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB2_0, 0x00f0},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB3_0, 0x00f0},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB4_0, 0x00f0},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB5_0, 0x00f0},
	{"", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_5G_6G_SB6_0, 0x00f0},
	{"hwrssioffset_trt_5g_6g_1", 0xfffc0000, SRFL_ARRAY,
	SROM18_HWRSSIOFFSET_5G_6G_SB0_0, 0xf000},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB1_0, 0xf000},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB2_0, 0xf000},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB3_0, 0xf000},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB4_0, 0xf000},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB5_0, 0xf000},
	{"", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_5G_6G_SB6_0, 0xf000},
	{"hwrssioffset_trt_5g_6g_2", 0xfffc0000, SRFL_ARRAY,
	SROM18_HWRSSIOFFSET_5G_6G_SB0_1, 0x00f0},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB1_1, 0x00f0},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB2_1, 0x00f0},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB3_1, 0x00f0},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB4_1, 0x00f0},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB5_1, 0x00f0},
	{"", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_5G_6G_SB6_1, 0x00f0},
	{"hwrssioffset_trt_5g_6g_3", 0xfffc0000, SRFL_ARRAY,
	SROM18_HWRSSIOFFSET_5G_6G_SB0_1, 0xf000},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB1_1, 0xf000},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB2_1, 0xf000},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB3_1, 0xf000},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB4_1, 0xf000},
	{"", 0xfffc0000, SRFL_ARRAY, SROM18_HWRSSIOFFSET_5G_6G_SB5_1, 0xf000},
	{"", 0xfffc0000, 0, SROM18_HWRSSIOFFSET_5G_6G_SB6_1, 0xf000},
	/* HWRSSI midgain */
	{"hwrssioffset_midgain_2g_0", 0xfff00000, 0, SROM20_HWRSSIOFFSET_MIDGAIN_2G, 0x000f},
	{"hwrssioffset_midgain_2g_1", 0xfff00000, 0, SROM20_HWRSSIOFFSET_MIDGAIN_2G, 0x00f0},
	{"hwrssioffset_midgain_2g_2", 0xfff00000, 0, SROM20_HWRSSIOFFSET_MIDGAIN_2G, 0x0f00},
	{"hwrssioffset_midgain_2g_3", 0xfff00000, 0, SROM20_HWRSSIOFFSET_MIDGAIN_2G, 0xf000},
	{"hwrssioffset_midgain_5g_6g_0", 0xfff00000, SRFL_ARRAY,
	SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB0, 0x000f},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB1, 0x000f},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB2, 0x000f},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB3, 0x000f},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB4, 0x000f},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB5, 0x000f},
	{"", 0xfff00000, 0, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB6, 0x000f},
	{"hwrssioffset_midgain_5g_6g_1", 0xfff00000, SRFL_ARRAY,
	SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB0, 0x00f0},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB1, 0x00f0},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB2, 0x00f0},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB3, 0x00f0},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB4, 0x00f0},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB5, 0x00f0},
	{"", 0xfff00000, 0, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB6, 0x00f0},
	{"hwrssioffset_midgain_5g_6g_2", 0xfff00000, SRFL_ARRAY,
	SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB0, 0x0f00},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB1, 0x0f00},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB2, 0x0f00},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB3, 0x0f00},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB4, 0x0f00},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB5, 0x0f00},
	{"", 0xfff00000, 0, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB6, 0x0f00},
	{"hwrssioffset_midgain_5g_6g_3", 0xfff00000, SRFL_ARRAY,
	SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB0, 0xf000},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB1, 0xf000},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB2, 0xf000},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB3, 0xf000},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB4, 0xf000},
	{"", 0xfff00000, SRFL_ARRAY, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB5, 0xf000},
	{"", 0xfff00000, 0, SROM20_HWRSSIOFFSET_MIDGAIN_5G_6G_SB6, 0xf000},
	{NULL,		0,		0,		0,			0}
};
#endif /* !defined(SROM15_MEMOPT) */

static const sromvar_t BCMATTACHDATA(pci_srom15vars)[] = {
	{"macaddr",	0x00008000,	SRFL_ETHADDR,		SROM15_MACHI,	0xffff},
	{"caldata_offset", 0x00008000, 0, SROM15_CAL_OFFSET_LOC, 0xffff},
	{"boardrev", 0x00008000, SRFL_PRHEX, SROM15_BRDREV, 0xffff},
	{"ccode", 0x00008000, SRFL_CCODE, SROM15_CCODE, 0xffff},
	{"regrev", 0x00008000, 0, SROM15_REGREV, 0xffff},
	{NULL,		0,		0,		0,			0}
};

static const sromvar_t BCMATTACHDATA(pci_srom16vars)[] = {
	{"macaddr",		0x00010000,	SRFL_ETHADDR,	SROM16_MACHI,		0xffff},
	{"caldata_offset",	0x00010000,	0,	SROM16_CALDATA_OFFSET_LOC,	0xffff},
	{"boardrev",		0x00010000,	0,		SROM16_BOARDREV,	0xffff},
	{"ccode",		0x00010000,	0,		SROM16_CCODE,		0xffff},
	{"regrev",		0x00010000,	0,		SROM16_REGREV,		0xffff},
	{NULL,			0,		0,		0,			0}
};

static const sromvar_t BCMATTACHDATA(pci_srom17vars)[] = {
	{"boardrev",	0x00020000,	SRFL_PRHEX,	SROM17_BRDREV,		0xffff},
	{"macaddr",	0x00020000,	SRFL_ETHADDR,	SROM17_MACADDR,		0xffff},
	{"ccode",		0x00020000,	SRFL_CCODE,	SROM17_CCODE,		0xffff},
	{"caldata_offset",	0x00020000,	0,		SROM17_CALDATA,		0xffff},
	{"gain_cal_temp",	0x00020000,	SRFL_PRHEX,	SROM17_GCALTMP,		0xffff},
	{"rssi_delta_2gb0_c0", 0x00020000,	PRHEX_N_MORE,	SROM17_C0SRD202G,	0xffff},
	{"",		0x00020000,	0,		SROM17_C0SRD202G_1,	0xffff},
	{"rssi_delta_5gl_c0", 0x00020000,	PRHEX_N_MORE,	SROM17_C0SRD205GL,	0xffff},
	{"",		0x00020000,	0,		SROM17_C0SRD205GL_1,	0xffff},
	{"rssi_delta_5gml_c0", 0x00020000,	PRHEX_N_MORE,	SROM17_C0SRD205GML,	0xffff},
	{"",		0x00020000,	0,		SROM17_C0SRD205GML_1,	0xffff},
	{"rssi_delta_5gmu_c0", 0x00020000,	PRHEX_N_MORE,	SROM17_C0SRD205GMU,	0xffff},
	{"",		0x00020000,	0,		SROM17_C0SRD205GMU_1,	0xffff},
	{"rssi_delta_5gh_c0", 0x00020000,	PRHEX_N_MORE,	SROM17_C0SRD205GH,	0xffff},
	{"",		0x00020000,	0,		SROM17_C0SRD205GH_1,	0xffff},
	{"rssi_delta_2gb0_c1", 0x00020000,	PRHEX_N_MORE,	SROM17_C1SRD202G,	0xffff},
	{"",		0x00020000,	0,		SROM17_C1SRD202G_1,	0xffff},
	{"rssi_delta_5gl_c1", 0x00020000,	PRHEX_N_MORE,	SROM17_C1SRD205GL,	0xffff},
	{"",		0x00020000,	0,		SROM17_C1SRD205GL_1,	0xffff},
	{"rssi_delta_5gml_c1", 0x00020000,	PRHEX_N_MORE,	SROM17_C1SRD205GML,	0xffff},
	{"",		0x00020000,	0,		SROM17_C1SRD205GML_1,	0xffff},
	{"rssi_delta_5gmu_c1", 0x00020000,	PRHEX_N_MORE,	SROM17_C1SRD205GMU,	0xffff},
	{"",		0x00020000,	0,		SROM17_C1SRD205GMU_1,	0xffff},
	{"rssi_delta_5gh_c1", 0x00020000,	PRHEX_N_MORE,	SROM17_C1SRD205GH,	0xffff},
	{"",		0x00020000,	0,		SROM17_C1SRD205GH_1,	0xffff},
	{"txpa_trim_magic",	0x00020000,	PRHEX_N_MORE,	SROM17_TRAMMAGIC,	0xffff},
	{"",		0x00020000,	0,		SROM17_TRAMMAGIC_1,	0xffff},
	{"txpa_trim_data",	0x00020000,	SRFL_PRHEX,	SROM17_TRAMDATA,	0xffff},
	{NULL,		0,		0,		0,			0x00}
};

static const sromvar_t BCMATTACHDATA(perpath_pci_sromvars)[] = {
	{"maxp2ga",	0x000000f0,	0,		SROM4_2G_ITT_MAXP,	0x00ff},
	{"itt2ga",	0x000000f0,	0,		SROM4_2G_ITT_MAXP,	0xff00},
	{"itt5ga",	0x000000f0,	0,		SROM4_5G_ITT_MAXP,	0xff00},
	{"pa2gw0a",	0x000000f0,	SRFL_PRHEX,	SROM4_2G_PA,		0xffff},
	{"pa2gw1a",	0x000000f0,	SRFL_PRHEX,	SROM4_2G_PA + 1,	0xffff},
	{"pa2gw2a",	0x000000f0,	SRFL_PRHEX,	SROM4_2G_PA + 2,	0xffff},
	{"pa2gw3a",	0x000000f0,	SRFL_PRHEX,	SROM4_2G_PA + 3,	0xffff},
	{"maxp5ga",	0x000000f0,	0,		SROM4_5G_ITT_MAXP,	0x00ff},
	{"maxp5gha",	0x000000f0,	0,		SROM4_5GLH_MAXP,	0x00ff},
	{"maxp5gla",	0x000000f0,	0,		SROM4_5GLH_MAXP,	0xff00},
	{"pa5gw0a",	0x000000f0,	SRFL_PRHEX,	SROM4_5G_PA,		0xffff},
	{"pa5gw1a",	0x000000f0,	SRFL_PRHEX,	SROM4_5G_PA + 1,	0xffff},
	{"pa5gw2a",	0x000000f0,	SRFL_PRHEX,	SROM4_5G_PA + 2,	0xffff},
	{"pa5gw3a",	0x000000f0,	SRFL_PRHEX,	SROM4_5G_PA + 3,	0xffff},
	{"pa5glw0a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GL_PA,		0xffff},
	{"pa5glw1a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GL_PA + 1,	0xffff},
	{"pa5glw2a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GL_PA + 2,	0xffff},
	{"pa5glw3a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GL_PA + 3,	0xffff},
	{"pa5ghw0a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GH_PA,		0xffff},
	{"pa5ghw1a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GH_PA + 1,	0xffff},
	{"pa5ghw2a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GH_PA + 2,	0xffff},
	{"pa5ghw3a",	0x000000f0,	SRFL_PRHEX,	SROM4_5GH_PA + 3,	0xffff},
	{"maxp2ga",	0x00000700,	0,		SROM8_2G_ITT_MAXP,	0x00ff},
	{"itt2ga",	0x00000700,	0,		SROM8_2G_ITT_MAXP,	0xff00},
	{"itt5ga",	0x00000700,	0,		SROM8_5G_ITT_MAXP,	0xff00},
	{"pa2gw0a",	0x00000700,	SRFL_PRHEX,	SROM8_2G_PA,		0xffff},
	{"pa2gw1a",	0x00000700,	SRFL_PRHEX,	SROM8_2G_PA + 1,	0xffff},
	{"pa2gw2a",	0x00000700,	SRFL_PRHEX,	SROM8_2G_PA + 2,	0xffff},
	{"maxp5ga",	0x00000700,	0,		SROM8_5G_ITT_MAXP,	0x00ff},
	{"maxp5gha",	0x00000700,	0,		SROM8_5GLH_MAXP,	0x00ff},
	{"maxp5gla",	0x00000700,	0,		SROM8_5GLH_MAXP,	0xff00},
	{"pa5gw0a",	0x00000700,	SRFL_PRHEX,	SROM8_5G_PA,		0xffff},
	{"pa5gw1a",	0x00000700,	SRFL_PRHEX,	SROM8_5G_PA + 1,	0xffff},
	{"pa5gw2a",	0x00000700,	SRFL_PRHEX,	SROM8_5G_PA + 2,	0xffff},
	{"pa5glw0a",	0x00000700,	SRFL_PRHEX,	SROM8_5GL_PA,		0xffff},
	{"pa5glw1a",	0x00000700,	SRFL_PRHEX,	SROM8_5GL_PA + 1,	0xffff},
	{"pa5glw2a",	0x00000700,	SRFL_PRHEX,	SROM8_5GL_PA + 2,	0xffff},
	{"pa5ghw0a",	0x00000700,	SRFL_PRHEX,	SROM8_5GH_PA,		0xffff},
	{"pa5ghw1a",	0x00000700,	SRFL_PRHEX,	SROM8_5GH_PA + 1,	0xffff},
	{"pa5ghw2a",	0x00000700,	SRFL_PRHEX,	SROM8_5GH_PA + 2,	0xffff},

	/* sromrev 11 */
	{"maxp2ga",	0xfffff800,	0,			 SROM11_2G_MAXP,	0x00ff},
	{"pa2ga",	0x00000800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_2G_PA,		0xffff},
	{"",		0x00000800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_2G_PA + 1,	0xffff},
	{"",		0x00000800,	SRFL_PRHEX,		 SROM11_2G_PA + 2,	0xffff},
	{"rxgains5gmelnagaina",	0x00000800,	0,		 SROM11_RXGAINS1,	0x0007},
	{"rxgains5gmtrisoa",	0x00000800,	0,		 SROM11_RXGAINS1,	0x0078},
	{"rxgains5gmtrelnabypa", 0x00000800,	0,		 SROM11_RXGAINS1,	0x0080},
	{"rxgains5ghelnagaina",	0x00000800,	0,		 SROM11_RXGAINS1,	0x0700},
	{"rxgains5ghtrisoa",	0x00000800,	0,		 SROM11_RXGAINS1,	0x7800},
	{"rxgains5ghtrelnabypa", 0x00000800,	0,		 SROM11_RXGAINS1,	0x8000},
	{"rxgains2gelnagaina",	0x00000800,	0,		 SROM11_RXGAINS,	0x0007},
	{"rxgains2gtrisoa",	0x00000800,	0,		 SROM11_RXGAINS,	0x0078},
	{"rxgains2gtrelnabypa",	0x00000800,	0,		 SROM11_RXGAINS,	0x0080},
	{"rxgains5gelnagaina",	0x00000800,	0,		 SROM11_RXGAINS,	0x0700},
	{"rxgains5gtrisoa",	0x00000800,	0,		 SROM11_RXGAINS,	0x7800},
	{"rxgains5gtrelnabypa",	0x00000800,	0,		 SROM11_RXGAINS,	0x8000},
	{"maxp5ga",	0x00000800,	SRFL_ARRAY,		 SROM11_5GB1B0_MAXP,	0x00ff},
	{"",		0x00000800,	SRFL_ARRAY,		 SROM11_5GB1B0_MAXP,	0xff00},
	{"",		0x00000800,	SRFL_ARRAY,		 SROM11_5GB3B2_MAXP,	0x00ff},
	{"",		0x00000800,	0,			 SROM11_5GB3B2_MAXP,	0xff00},
	{"pa5ga",	0x00000800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_5GB0_PA,	0xffff},
	{"",		0x00000800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_5GB0_PA + 1,	0xffff},
	{"",		0x00000800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_5GB0_PA + 2,	0xffff},
	{"",		0x00000800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_5GB1_PA,	0xffff},
	{"",		0x00000800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_5GB1_PA + 1,	0xffff},
	{"",		0x00000800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_5GB1_PA + 2,	0xffff},
	{"",		0x00000800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_5GB2_PA,	0xffff},
	{"",		0x00000800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_5GB2_PA + 1,	0xffff},
	{"",		0x00000800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_5GB2_PA + 2,	0xffff},
	{"",		0x00000800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_5GB3_PA,	0xffff},
	{"",		0x00000800,	SRFL_PRHEX | SRFL_ARRAY, SROM11_5GB3_PA + 1,	0xffff},
	{"",		0x00000800,	SRFL_PRHEX,		 SROM11_5GB3_PA + 2,	0xffff},

	/* sromrev 12 */
	{"maxp5gb4a",	0xfffff000,	0,		 SROM12_5GB42G_MAXP,	0x00ff00},
	{"pa2ga",	0xfffff000,	SRFL_PRHEX | SRFL_ARRAY,  SROM12_2GB0_PA_W0,	0x00ffff},
	{"",	0xfffff000,	SRFL_PRHEX | SRFL_ARRAY,	  SROM12_2GB0_PA_W1,	0x00ffff},
	{"",	0xfffff000,	SRFL_PRHEX | SRFL_ARRAY,	  SROM12_2GB0_PA_W2,	0x00ffff},
	{"",	0xfffff000,	SRFL_PRHEX,		 SROM12_2GB0_PA_W3,	0x00ffff},

	{"pa2g40a",	0xfffff000,	SRFL_PRHEX | SRFL_ARRAY,  SROM12_2G40B0_PA_W0,	0x00ffff},
	{"",	0xfffff000,	SRFL_PRHEX | SRFL_ARRAY,	  SROM12_2G40B0_PA_W1,	0x00ffff},
	{"",	0xfffff000,	SRFL_PRHEX | SRFL_ARRAY,	  SROM12_2G40B0_PA_W2,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX,		 SROM12_2G40B0_PA_W3,	0x00ffff},
	{"maxp5gb0a",	0xfffff000, 0,	     SROM12_5GB1B0_MAXP,	0x00ff},
	{"maxp5gb1a",	0xfffff000, 0,       SROM12_5GB1B0_MAXP,	0x00ff00},
	{"maxp5gb2a",	0xfffff000, 0,	     SROM12_5GB3B2_MAXP,	0x00ff},
	{"maxp5gb3a",	0xfffff000, 0,	     SROM12_5GB3B2_MAXP,	0x00ff00},

	{"pa5ga",   0xfffff000, SRFL_PRHEX | SRFL_ARRAY,   SROM12_5GB0_PA_W0,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB0_PA_W1,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,         SROM12_5GB0_PA_W2,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB0_PA_W3,		0x00ffff},

	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB1_PA_W0,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB1_PA_W1,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB1_PA_W2,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB1_PA_W3,		0x00ffff},

	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB2_PA_W0,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB2_PA_W1,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB2_PA_W2,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB2_PA_W3,		0x00ffff},

	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB3_PA_W0,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB3_PA_W1,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,         SROM12_5GB3_PA_W2,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB3_PA_W3,		0x00ffff},

	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB4_PA_W0,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB4_PA_W1,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5GB4_PA_W2,		0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX,	                   SROM12_5GB4_PA_W3,		0x00ffff},

	{"pa5g40a",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY, SROM12_5G40B0_PA_W0,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B0_PA_W1,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B0_PA_W2,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B0_PA_W3,	0x00ffff},

	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B1_PA_W0,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B1_PA_W1,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B1_PA_W2,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,         SROM12_5G40B1_PA_W3,	0x00ffff},

	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B2_PA_W0,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B2_PA_W1,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B2_PA_W2,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B2_PA_W3,	0x00ffff},

	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B3_PA_W0,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B3_PA_W1,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B3_PA_W2,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B3_PA_W3,	0x00ffff},

	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B4_PA_W0,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B4_PA_W1,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	     SROM12_5G40B4_PA_W2,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX,		             SROM12_5G40B4_PA_W3,	0x00ffff},

	{"pa5g80a",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,	 SROM12_5G80B0_PA_W0,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B0_PA_W1,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B0_PA_W2,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B0_PA_W3,	0x00ffff},

	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B1_PA_W0,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B1_PA_W1,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B1_PA_W2,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B1_PA_W3,	0x00ffff},

	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B2_PA_W0,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B2_PA_W1,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B2_PA_W2,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B2_PA_W3,	0x00ffff},

	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B3_PA_W0,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B3_PA_W1,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B3_PA_W2,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B3_PA_W3,	0x00ffff},

	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B4_PA_W0,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B4_PA_W1,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX | SRFL_ARRAY,		 SROM12_5G80B4_PA_W2,	0x00ffff},
	{"",	0xfffff000, SRFL_PRHEX,		                 SROM12_5G80B4_PA_W3,	0x00ffff},
	/* sromrev 13 */
	{"rxgains2gelnagaina",   0xffffe000,     0,       SROM13_RXGAINS,  0x0007},
	{"rxgains2gtrisoa",     0xffffe000,     0,       SROM13_RXGAINS,  0x0078},
	{"rxgains2gtrelnabypa", 0xffffe000,     0,       SROM13_RXGAINS,  0x0080},
	{"rxgains5gelnagaina",  0xffffe000,     0,       SROM13_RXGAINS,  0x0700},
	{"rxgains5gtrisoa",     0xffffe000,     0,       SROM13_RXGAINS,  0x7800},
	{"rxgains5gtrelnabypa", 0xffffe000,     0,       SROM13_RXGAINS,  0x8000},

	/* sromrev 18 */
	{"pa2g20ccka",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,	SROM18_2G20CCK_PA_W0,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_2G20CCK_PA_W1,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_2G20CCK_PA_W2,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX,				SROM18_2G20CCK_PA_W3,	0x00ffff},

	{"pa5g160a",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,	SROM18_5G160C0_PA_W0,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C0_PA_W1,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C0_PA_W2,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C0_PA_W3,	0x00ffff},

	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C1_PA_W0,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C1_PA_W1,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C1_PA_W2,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C1_PA_W3,	0x00ffff},

	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C2_PA_W0,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C2_PA_W1,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C2_PA_W2,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C2_PA_W3,	0x00ffff},

	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C3_PA_W0,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C3_PA_W1,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C3_PA_W2,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX,				SROM18_5G160C3_PA_W3,	0x00ffff},
	{"pa5gexta",   0xffffc000, SRFL_PRHEX | SRFL_ARRAY,   SROM18_5GB5_PA_W0,	0x00ffff},
	{"",	0xffffc000, SRFL_PRHEX | SRFL_ARRAY,	     SROM18_5GB5_PA_W1,		0x00ffff},
	{"",	0xffffc000, SRFL_PRHEX | SRFL_ARRAY,         SROM18_5GB5_PA_W2,		0x00ffff},
	{"",	0xffffc000, SRFL_PRHEX | SRFL_ARRAY,	     SROM18_5GB5_PA_W3,		0x00ffff},

	{"",	0xffffc000, SRFL_PRHEX | SRFL_ARRAY,	     SROM18_5GB6_PA_W0,		0x00ffff},
	{"",	0xffffc000, SRFL_PRHEX | SRFL_ARRAY,	     SROM18_5GB6_PA_W1,		0x00ffff},
	{"",	0xffffc000, SRFL_PRHEX | SRFL_ARRAY,	     SROM18_5GB6_PA_W2,		0x00ffff},
	{"",	0xffffc000, SRFL_PRHEX,			     SROM18_5GB6_PA_W3,		0x00ffff},
	{"pa5gext40a", 0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,	SROM18_5G40B5_PA_W0,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G40B5_PA_W1,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G40B5_PA_W2,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G40B5_PA_W3,	0x00ffff},

	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G40B6_PA_W0,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G40B6_PA_W1,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G40B6_PA_W2,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX,				SROM18_5G40B6_PA_W3,	0x00ffff},
	{"pa5gext80a", 0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,	SROM18_5G80B5_PA_W0,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G80B5_PA_W1,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G80B5_PA_W2,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G80B5_PA_W3,	0x00ffff},

	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G80B6_PA_W0,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G80B6_PA_W1,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G80B6_PA_W2,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX,				SROM18_5G80B6_PA_W3,	0x00ffff},
	{"pa5gext160a", 0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,	SROM18_5G160C4_PA_W0,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C4_PA_W1,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C4_PA_W2,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C4_PA_W3,	0x00ffff},

	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C5_PA_W0,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C5_PA_W1,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C5_PA_W2,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C5_PA_W3,	0x00ffff},

	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C6_PA_W0,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C6_PA_W1,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX | SRFL_ARRAY,		SROM18_5G160C6_PA_W2,	0x00ffff},
	{"",	0xfffc0000, SRFL_PRHEX,				SROM18_5G160C6_PA_W3,	0x00ffff},
	{"pa6ga",   0xfff80000, SRFL_PRHEX | SRFL_ARRAY,   SROM19_PA6G20B0W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B0W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B0W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B0W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B1W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B1W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B1W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B1W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B2W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B2W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B2W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B2W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B3W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B3W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B3W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B3W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B4W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B4W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B4W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B4W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B5W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B5W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B5W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B5W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B6W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B6W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G20B6W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX, SROM19_PA6G20B6W3A0, 0x00ffff},
	{"pa6g40a",   0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B0W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B0W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B0W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B0W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B1W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B1W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B1W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B1W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B2W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B2W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B2W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B2W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B3W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B3W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B3W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B3W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B4W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B4W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B4W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B4W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B5W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B5W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B5W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B5W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B6W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B6W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G40B6W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX, SROM19_PA6G40B6W3A0, 0x00ffff},
	{"pa6g80a",   0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B0W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B0W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B0W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B0W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B1W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B1W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B1W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B1W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B2W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B2W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B2W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B2W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B3W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B3W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B3W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B3W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B4W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B4W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B4W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B4W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B5W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B5W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B5W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B5W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B6W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B6W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G80B6W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX, SROM19_PA6G80B6W3A0, 0x00ffff},
	{"pa6g160a",   0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B0W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B0W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B0W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B0W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B1W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B1W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B1W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B1W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B2W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B2W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B2W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B2W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B3W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B3W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B3W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B3W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B4W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B4W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B4W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B4W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B5W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B5W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B5W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B5W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B6W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B6W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G160B6W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX, SROM19_PA6G160B6W3A0, 0x00ffff},
	{"pa6g320a",   0xfff80000, SRFL_PRHEX | SRFL_ARRAY,   SROM19_PA6G320B0W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B0W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B0W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B0W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B1W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B1W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B1W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B1W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B2W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B2W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B2W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B2W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B3W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B3W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B3W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B3W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B4W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B4W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B4W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B4W3A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B5W0A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B5W1A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX | SRFL_ARRAY, SROM19_PA6G320B5W2A0, 0x00ffff},
	{"",	0xfff80000, SRFL_PRHEX, SROM19_PA6G320B5W3A0, 0x00ffff},
	{NULL,		0,		0,		0,			0}
};

#if !defined(PHY_TYPE_N)
#define	PHY_TYPE_N		4	/* N-Phy value */
#endif /* !(defined(PHY_TYPE_HT) && defined(PHY_TYPE_N)) */
#if !defined(PHY_TYPE_AC)
#define	PHY_TYPE_AC		11	/* AC-Phy value */
#endif /* !defined(PHY_TYPE_AC) */
#if !defined(PHY_TYPE_LCN20)
#define	PHY_TYPE_LCN20		12	/* LCN20-Phy value */
#endif /* !defined(PHY_TYPE_LCN20) */
#if !defined(PHY_TYPE_NULL)
#define	PHY_TYPE_NULL		0xf	/* Invalid Phy value */
#endif /* !defined(PHY_TYPE_NULL) */

typedef struct {
	uint16	phy_type;
	uint16	bandrange;
	uint16	chain;
	const char *vars;
} pavars_t;

static const pavars_t pavars[] = {
	/* NPHY */
	{PHY_TYPE_N, WL_CHAN_FREQ_RANGE_2G,  0, "pa2gw0a0 pa2gw1a0 pa2gw2a0"},
	{PHY_TYPE_N, WL_CHAN_FREQ_RANGE_2G,  1, "pa2gw0a1 pa2gw1a1 pa2gw2a1"},
	{PHY_TYPE_N, WL_CHAN_FREQ_RANGE_5G_BAND0, 0, "pa5glw0a0 pa5glw1a0 pa5glw2a0"},
	{PHY_TYPE_N, WL_CHAN_FREQ_RANGE_5G_BAND0, 1, "pa5glw0a1 pa5glw1a1 pa5glw2a1"},
	{PHY_TYPE_N, WL_CHAN_FREQ_RANGE_5G_BAND1, 0, "pa5gw0a0 pa5gw1a0 pa5gw2a0"},
	{PHY_TYPE_N, WL_CHAN_FREQ_RANGE_5G_BAND1, 1, "pa5gw0a1 pa5gw1a1 pa5gw2a1"},
	{PHY_TYPE_N, WL_CHAN_FREQ_RANGE_5G_BAND2, 0, "pa5ghw0a0 pa5ghw1a0 pa5ghw2a0"},
	{PHY_TYPE_N, WL_CHAN_FREQ_RANGE_5G_BAND2, 1, "pa5ghw0a1 pa5ghw1a1 pa5ghw2a1"},
	/* ACPHY */
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  0, "pa2ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  1, "pa2ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  2, "pa2ga2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_4BAND,  0, "pa5ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_4BAND,  1, "pa5ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_4BAND,  2, "pa5ga2"},
	/* LCN20PHY */
	{PHY_TYPE_LCN20, WL_CHAN_FREQ_RANGE_2G,  0, "pa2ga0"},
	{PHY_TYPE_NULL, 0, 0, ""}
};

static const pavars_t pavars_SROM12[] = {
	/* ACPHY */
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  0, "pa2ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  1, "pa2ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  2, "pa2ga2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  0, "pa2g40a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  1, "pa2g40a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  2, "pa2g40a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  0, "pa5ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  1, "pa5ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  2, "pa5ga2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  0, "pa5g40a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  1, "pa5g40a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  2, "pa5g40a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  0, "pa5g80a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  1, "pa5g80a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  2, "pa5g80a2"},
	{PHY_TYPE_NULL, 0, 0, ""}
};

static const pavars_t pavars_SROM13[] = {
	/* ACPHY */
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  0, "pa2ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  1, "pa2ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  2, "pa2ga2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  3, "pa2ga3"},
	/* WL_CHAN_FREQ_RANGE_2G_20_CCK is for SROM18 */
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  0, "pa2g20ccka0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  1, "pa2g20ccka1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  2, "pa2g20ccka2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  3, "pa2g20ccka3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  0, "pa2g40a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  1, "pa2g40a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  2, "pa2g40a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  3, "pa2g40a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  0, "pa5ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  1, "pa5ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  2, "pa5ga2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  3, "pa5ga3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  0, "pa5g40a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  1, "pa5g40a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  2, "pa5g40a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  3, "pa5g40a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  0, "pa5g80a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  1, "pa5g80a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  2, "pa5g80a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  3, "pa5g80a3"},
	{PHY_TYPE_NULL, 0, 0, ""}
};

static const pavars_t pavars_SROM18[] = {
	/* ACPHY */
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  0, "pa2ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  1, "pa2ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  2, "pa2ga2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  3, "pa2ga3"},
	/* WL_CHAN_FREQ_RANGE_2G_20_CCK is for SROM18 */
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  0, "pa2g20ccka0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  1, "pa2g20ccka1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  2, "pa2g20ccka2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  3, "pa2g20ccka3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  0, "pa2g40a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  1, "pa2g40a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  2, "pa2g40a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  3, "pa2g40a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  0, "pa5ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  1, "pa5ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  2, "pa5ga2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  3, "pa5ga3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  0, "pa5g40a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  1, "pa5g40a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  2, "pa5g40a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  3, "pa5g40a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  0, "pa5g80a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  1, "pa5g80a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  2, "pa5g80a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  3, "pa5g80a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_160,  0, "pa5g160a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_160,  1, "pa5g160a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_160,  2, "pa5g160a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_160,  3, "pa5g160a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND,  0, "pa5gexta0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND,  1, "pa5gexta1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND,  2, "pa5gexta2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND,  3, "pa5gexta3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_40,  0, "pa5gext40a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_40,  1, "pa5gext40a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_40,  2, "pa5gext40a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_40,  3, "pa5gext40a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_80,  0, "pa5gext80a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_80,  1, "pa5gext80a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_80,  2, "pa5gext80a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_80,  3, "pa5gext80a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_160,  0, "pa5gext160a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_160,  1, "pa5gext160a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_160,  2, "pa5gext160a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_160,  3, "pa5gext160a3"},
	{PHY_TYPE_NULL, 0, 0, ""}
};

static const pavars_t pavars_SROM19[] = {
	/* ACPHY */
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  0, "pa2ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  1, "pa2ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  2, "pa2ga2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  3, "pa2ga3"},
	/* WL_CHAN_FREQ_RANGE_2G_20_CCK is for SROM18 */
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  0, "pa2g20ccka0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  1, "pa2g20ccka1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  2, "pa2g20ccka2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  3, "pa2g20ccka3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  0, "pa2g40a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  1, "pa2g40a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  2, "pa2g40a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  3, "pa2g40a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  0, "pa5ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  1, "pa5ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  2, "pa5ga2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  3, "pa5ga3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  0, "pa5g40a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  1, "pa5g40a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  2, "pa5g40a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  3, "pa5g40a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  0, "pa5g80a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  1, "pa5g80a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  2, "pa5g80a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  3, "pa5g80a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_160,  0, "pa5g160a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_160,  1, "pa5g160a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_160,  2, "pa5g160a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_160,  3, "pa5g160a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND,  0, "pa5gexta0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND,  1, "pa5gexta1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND,  2, "pa5gexta2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND,  3, "pa5gexta3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_40,  0, "pa5gext40a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_40,  1, "pa5gext40a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_40,  2, "pa5gext40a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_40,  3, "pa5gext40a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_80,  0, "pa5gext80a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_80,  1, "pa5gext80a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_80,  2, "pa5gext80a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_80,  3, "pa5gext80a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_160,  0, "pa5gext160a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_160,  1, "pa5gext160a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_160,  2, "pa5gext160a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5GEXT_5BAND_160,  3, "pa5gext160a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND,  0, "pa6ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND,  1, "pa6ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND,  2, "pa6ga2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND,  3, "pa6ga3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_40,  0, "pa6g40a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_40,  1, "pa6g40a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_40,  2, "pa6g40a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_40,  3, "pa6g40a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_80,  0, "pa6g80a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_80,  1, "pa6g80a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_80,  2, "pa6g80a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_80,  3, "pa6g80a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_160,  0, "pa6g160a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_160,  1, "pa6g160a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_160,  2, "pa6g160a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_160,  3, "pa6g160a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_320,  0, "pa6g320a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_320,  1, "pa6g320a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_320,  2, "pa6g320a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_6G_7BAND_320,  3, "pa6g320a3"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_AVVMID,  0, "avvmida0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_AVVMID,  1, "avvmida1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_AVVMID,  2, "avvmida2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_AVVMID,  3, "avvmida3"},
	{PHY_TYPE_NULL, 0, 0, ""}
};

/* pavars table when paparambwver is 1 */
static const pavars_t pavars_bwver_1[] = {
	/* ACPHY */
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  0, "pa2ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  1, "pa2gccka0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  1, "pa2ga2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_4BAND,  0, "pa5ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_4BAND,  1, "pa5gbw40a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_4BAND,  2, "pa5gbw80a0"},
	{PHY_TYPE_NULL, 0, 0, ""}
};

/* pavars table when paparambwver is 2 */
static const pavars_t pavars_bwver_2[] = {
	/* ACPHY */
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  0, "pa2ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  1, "pa2ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_4BAND,  0, "pa5ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_4BAND,  1, "pa5ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_4BAND,  2, "pa5gbw4080a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_4BAND,  3, "pa5gbw4080a1"},
	{PHY_TYPE_NULL, 0, 0, ""}
};

/* pavars table when paparambwver is 3 */
static const pavars_t pavars_bwver_3[] = {
	/* ACPHY */
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  0, "pa2ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  1, "pa2ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  2, "pa2gccka0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  3, "pa2gccka1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_4BAND,  0, "pa5ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_4BAND,  1, "pa5ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_4BAND,  2, "pa5gbw4080a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_4BAND,  3, "pa5gbw4080a1"},
	{PHY_TYPE_NULL, 0, 0, ""}
};

/* pavars table when paparambwver is 4 */
static const pavars_t pavars_bwver_4[] = {
	/* ACPHY */
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  0, "pa2ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  1, "pa2ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G,  2, "pa2ga2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  0, "pa2gccka0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  1, "pa2gccka1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_20_CCK,  2, "pa2gccka2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  0, "pa2g40a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  1, "pa2g40a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_2G_40,  2, "pa2g40a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  0, "pa5ga0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  1, "pa5ga1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND,  2, "pa5ga2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  0, "pa5g40a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  1, "pa5g40a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_40,  2, "pa5g40a2"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  0, "pa5g80a0"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  1, "pa5g80a1"},
	{PHY_TYPE_AC, WL_CHAN_FREQ_RANGE_5G_5BAND_80,  2, "pa5g80a2"},
	{PHY_TYPE_NULL, 0, 0, ""}
};

typedef struct {
	uint16	phy_type;
	uint16	bandrange;
	const char *vars;
} povars_t;

static const povars_t povars[] = {
	/* NPHY */
	{PHY_TYPE_N, WL_CHAN_FREQ_RANGE_2G,  "mcs2gpo0 mcs2gpo1 mcs2gpo2 mcs2gpo3 "
	"mcs2gpo4 mcs2gpo5 mcs2gpo6 mcs2gpo7"},
	{PHY_TYPE_N, WL_CHAN_FREQ_RANGE_5GL, "mcs5glpo0 mcs5glpo1 mcs5glpo2 mcs5glpo3 "
	"mcs5glpo4 mcs5glpo5 mcs5glpo6 mcs5glpo7"},
	{PHY_TYPE_N, WL_CHAN_FREQ_RANGE_5GM, "mcs5gpo0 mcs5gpo1 mcs5gpo2 mcs5gpo3 "
	"mcs5gpo4 mcs5gpo5 mcs5gpo6 mcs5gpo7"},
	{PHY_TYPE_N, WL_CHAN_FREQ_RANGE_5GH, "mcs5ghpo0 mcs5ghpo1 mcs5ghpo2 mcs5ghpo3 "
	"mcs5ghpo4 mcs5ghpo5 mcs5ghpo6 mcs5ghpo7"},
	{PHY_TYPE_NULL, 0, ""}
};

typedef struct {
	uint8	tag;		/* Broadcom subtag name */
	uint32	revmask;	/* Supported cis_sromrev bitmask. Some of the parameters in
				 * different tuples have the same name. Therefore, the MFGc tool
				 * needs to know which tuple to generate when seeing these
				 * parameters (given that we know sromrev from user input, like the
				 * nvram file).
				 */
	uint8	len;		/* Length field of the tuple, note that it includes the
				 * subtag name (1 byte): 1 + tuple content length
				 */
	const char *params;
} cis_tuple_t;

#define OTP_RAW		(0xff - 1)	/* Reserved tuple number for wrvar Raw input */
#define OTP_VERS_1	(0xff - 2)	/* CISTPL_VERS_1 */
#define OTP_MANFID	(0xff - 3)	/* CISTPL_MANFID */
#define OTP_RAW1	(0xff - 4)	/* Like RAW, but comes first */

/** this array is used by CIS creating/writing applications */
static const cis_tuple_t cis_hnbuvars[] = {
/*       tag			revmask   len  params */
	{OTP_RAW1,		0xffffffff, 0, ""},	/* special case */
	{OTP_VERS_1,	0xffffffff, 0, "smanf sproductname"},	/* special case (non BRCM tuple) */
	{OTP_MANFID,	0xffffffff, 4, "2manfid 2prodid"},	/* special case (non BRCM tuple) */
	/* Unified OTP: tupple to embed USB manfid inside SDIO CIS */
	{HNBU_UMANFID,		0xffffffff, 8, "8usbmanfid"},
	{HNBU_SROMREV,		0xffffffff, 2, "1sromrev"},
	/* NOTE: subdevid is also written to boardtype.
	 *       Need to write HNBU_BOARDTYPE to change it if it is different.
	 */
	{HNBU_CHIPID,		0xffffffff, 11, "2vendid 2devid 2chiprev 2subvendid 2subdevid"},
	{HNBU_BOARDREV,		0xffffffff, 3, "2boardrev"},
	{HNBU_PAPARMS,		0xffffffff, 10, "2pa0b0 2pa0b1 2pa0b2 1pa0itssit 1pa0maxpwr 1opo"},
	{HNBU_AA,		0xffffffff, 3, "1aa2g 1aa5g"},
	{HNBU_AA,		0xffffffff, 3, "1aa0 1aa1"}, /* backward compatibility */
	{HNBU_AG,		0xffffffff, 5, "1ag0 1ag1 1ag2 1ag3"},
	{HNBU_BOARDFLAGS,	0xffffffff, 21, "4boardflags 4boardflags2 4boardflags3 "
	"4boardflags4 4boardflags5 "},
	{HNBU_LEDS,		0xffffffff, 17, "1ledbh0 1ledbh1 1ledbh2 1ledbh3 1ledbh4 1ledbh5 "
	"1ledbh6 1ledbh7 1ledbh8 1ledbh9 1ledbh10 1ledbh11 1ledbh12 1ledbh13 1ledbh14 1ledbh15"},
	{HNBU_CCODE,		0xffffffff, 4, "2ccode 1cctl"},
	{HNBU_CCKPO,		0xffffffff, 3, "2cckpo"},
	{HNBU_OFDMPO,		0xffffffff, 5, "4ofdmpo"},
	{HNBU_PAPARMS5G,	0xffffffff, 23, "2pa1b0 2pa1b1 2pa1b2 2pa1lob0 2pa1lob1 2pa1lob2 "
	"2pa1hib0 2pa1hib1 2pa1hib2 1pa1itssit "
	"1pa1maxpwr 1pa1lomaxpwr 1pa1himaxpwr"},
	{HNBU_RDLID,		0xffffffff, 3, "2rdlid"},
	{HNBU_RSSISMBXA2G, 0xffffffff, 3, "0rssismf2g 0rssismc2g "
	"0rssisav2g 0bxa2g"}, /* special case */
	{HNBU_RSSISMBXA5G, 0xffffffff, 3, "0rssismf5g 0rssismc5g "
	"0rssisav5g 0bxa5g"}, /* special case */
	{HNBU_XTALFREQ,		0xffffffff, 5, "4xtalfreq"},
	{HNBU_TRI2G,		0xffffffff, 2, "1tri2g"},
	{HNBU_TRI5G,		0xffffffff, 4, "1tri5gl 1tri5g 1tri5gh"},
	{HNBU_RXPO2G,		0xffffffff, 2, "1rxpo2g"},
	{HNBU_RXPO5G,		0xffffffff, 2, "1rxpo5g"},
	{HNBU_BOARDNUM,		0xffffffff, 3, "2boardnum"},
	{HNBU_MACADDR,		0xffffffff, 7, "6macaddr"},	/* special case */
	{HNBU_RDLSN,		0xffffffff, 3, "2rdlsn"},
	{HNBU_BOARDTYPE,	0xffffffff, 3, "2boardtype"},
	{HNBU_LEDDC,		0xffffffff, 3, "2leddc"},
	{HNBU_CHAINSWITCH,	0xffffffff, 5, "1txchain 1rxchain 2antswitch"},
	{HNBU_REGREV,		0xffffffff, 3, "2regrev"},
	{HNBU_FEM,		0x000007fe, 5, "0antswctl2g 0triso2g 0pdetrange2g 0extpagain2g "
	"0tssipos2g 0antswctl5g 0triso5g 0pdetrange5g 0extpagain5g 0tssipos5g"}, /* special case */
	{HNBU_PAPARMS_C0,	0x000007fe, 31, "1maxp2ga0 1itt2ga0 2pa2gw0a0 2pa2gw1a0 "
	"2pa2gw2a0 1maxp5ga0 1itt5ga0 1maxp5gha0 1maxp5gla0 2pa5gw0a0 2pa5gw1a0 2pa5gw2a0 "
	"2pa5glw0a0 2pa5glw1a0 2pa5glw2a0 2pa5ghw0a0 2pa5ghw1a0 2pa5ghw2a0"},
	{HNBU_PAPARMS_C1,	0x000007fe, 31, "1maxp2ga1 1itt2ga1 2pa2gw0a1 2pa2gw1a1 "
	"2pa2gw2a1 1maxp5ga1 1itt5ga1 1maxp5gha1 1maxp5gla1 2pa5gw0a1 2pa5gw1a1 2pa5gw2a1 "
	"2pa5glw0a1 2pa5glw1a1 2pa5glw2a1 2pa5ghw0a1 2pa5ghw1a1 2pa5ghw2a1"},
	{HNBU_PO_CCKOFDM,	0xffffffff, 19, "2cck2gpo 4ofdm2gpo 4ofdm5gpo 4ofdm5glpo "
	"4ofdm5ghpo"},
	{HNBU_PO_MCS2G,		0xffffffff, 17, "2mcs2gpo0 2mcs2gpo1 2mcs2gpo2 2mcs2gpo3 "
	"2mcs2gpo4 2mcs2gpo5 2mcs2gpo6 2mcs2gpo7"},
	{HNBU_PO_MCS5GM,	0xffffffff, 17, "2mcs5gpo0 2mcs5gpo1 2mcs5gpo2 2mcs5gpo3 "
	"2mcs5gpo4 2mcs5gpo5 2mcs5gpo6 2mcs5gpo7"},
	{HNBU_PO_MCS5GLH,	0xffffffff, 33, "2mcs5glpo0 2mcs5glpo1 2mcs5glpo2 2mcs5glpo3 "
	"2mcs5glpo4 2mcs5glpo5 2mcs5glpo6 2mcs5glpo7 "
	"2mcs5ghpo0 2mcs5ghpo1 2mcs5ghpo2 2mcs5ghpo3 "
	"2mcs5ghpo4 2mcs5ghpo5 2mcs5ghpo6 2mcs5ghpo7"},
	{HNBU_CCKFILTTYPE,	0xffffffff, 2, "1cckdigfilttype"},
	{HNBU_PO_CDD,		0xffffffff, 3, "2cddpo"},
	{HNBU_PO_STBC,		0xffffffff, 3, "2stbcpo"},
	{HNBU_PO_40M,		0xffffffff, 3, "2bw40po"},
	{HNBU_PO_40MDUP,	0xffffffff, 3, "2bwduppo"},
	{HNBU_RDLRWU,		0xffffffff, 2, "1rdlrwu"},
	{HNBU_WPS,		0xffffffff, 3, "1wpsgpio 1wpsled"},
	{HNBU_USBFS,		0xffffffff, 2, "1usbfs"},
	{HNBU_ELNA2G,           0xffffffff, 2, "1elna2g"},
	{HNBU_ELNA5G,           0xffffffff, 2, "1elna5g"},
	{HNBU_CUSTOM1,		0xffffffff, 5, "4customvar1"},
	{OTP_RAW,		0xffffffff, 0, ""},	/* special case */
	{HNBU_OFDMPO5G,		0xffffffff, 13, "4ofdm5gpo 4ofdm5glpo 4ofdm5ghpo"},
	{HNBU_USBEPNUM,		0xffffffff, 3, "2usbepnum"},
	{HNBU_CCKBW202GPO,	0xffffffff, 7, "2cckbw202gpo 2cckbw20ul2gpo 2cckbw20in802gpo"},
	{HNBU_LEGOFDMBW202GPO,	0xffffffff, 9, "4legofdmbw202gpo 4legofdmbw20ul2gpo"},
	{HNBU_LEGOFDMBW205GPO,	0xffffffff, 25, "4legofdmbw205glpo 4legofdmbw20ul5glpo "
	"4legofdmbw205gmpo 4legofdmbw20ul5gmpo 4legofdmbw205ghpo 4legofdmbw20ul5ghpo"},
	{HNBU_MCS2GPO,	0xffffffff, 17,	"4mcsbw202gpo 4mcsbw20ul2gpo 4mcsbw402gpo 4mcsbw802gpo"},
	{HNBU_MCS5GLPO,	0xffffffff, 13,	"4mcsbw205glpo 4mcsbw20ul5glpo 4mcsbw405glpo"},
	{HNBU_MCS5GMPO,	0xffffffff, 13,	"4mcsbw205gmpo 4mcsbw20ul5gmpo 4mcsbw405gmpo"},
	{HNBU_MCS5GHPO,	0xffffffff, 13,	"4mcsbw205ghpo 4mcsbw20ul5ghpo 4mcsbw405ghpo"},
	{HNBU_MCS32PO,	0xffffffff, 3,	"2mcs32po"},
	{HNBU_LEG40DUPPO,	0xffffffff, 3,	"2legofdm40duppo"},
	{HNBU_TEMPTHRESH,	0xffffffff, 7,	"1tempthresh 0temps_period 0temps_hysteresis "
	"1tempoffset 1tempsense_slope 0tempcorrx 0tempsense_option "
	"1phycal_tempdelta"}, /* special case */
	{HNBU_MUXENAB,		0xffffffff, 2,	"1muxenab"},
	{HNBU_FEM_CFG,		0xfffff800, 5,	"0femctrl 0papdcap2g 0tworangetssi2g 0pdgain2g "
	"0epagain2g 0tssiposslope2g 0gainctrlsph 0papdcap5g 0tworangetssi5g 0pdgain5g 0epagain5g "
	"0tssiposslope5g"}, /* special case */
	{HNBU_ACPA_C0,		0x00001800, 39,	"2subband5gver 2maxp2ga0 2*3pa2ga0 "
	"1*4maxp5ga0 2*12pa5ga0"},
	{HNBU_ACPA_C1,		0x00001800, 37,	"2maxp2ga1 2*3pa2ga1 1*4maxp5ga1 2*12pa5ga1"},
	{HNBU_ACPA_C2,		0x00001800, 37,	"2maxp2ga2 2*3pa2ga2 1*4maxp5ga2 2*12pa5ga2"},
	{HNBU_MEAS_PWR,		0xfffff800, 5,	"1measpower 1measpower1 1measpower2 2rawtempsense"},
	{HNBU_PDOFF,		0xfffff800, 13,	"2pdoffset40ma0 2pdoffset40ma1 2pdoffset40ma2 "
	"2pdoffset80ma0 2pdoffset80ma1 2pdoffset80ma2"},
	{HNBU_ACPPR_2GPO,	0xfffff800, 13,	"2dot11agofdmhrbw202gpo 2ofdmlrbw202gpo "
	"2sb20in40dot11agofdm2gpo 2sb20in80dot11agofdm2gpo 2sb20in40ofdmlrbw202gpo "
	"2sb20in80ofdmlrbw202gpo"},
	{HNBU_ACPPR_5GPO,	0xfffff800, 59,	"4mcsbw805glpo 4mcsbw1605glpo 4mcsbw805gmpo "
	"4mcsbw1605gmpo 4mcsbw805ghpo 4mcsbw1605ghpo 2mcslr5glpo 2mcslr5gmpo 2mcslr5ghpo "
	"4mcsbw80p805glpo 4mcsbw80p805gmpo 4mcsbw80p805ghpo 4mcsbw80p805gx1po 2mcslr5gx1po "
	"2mcslr5g80p80po 4mcsbw805gx1po 4mcsbw1605gx1po"},
	{HNBU_MCS5Gx1PO,	0xfffff800, 9,	"4mcsbw205gx1po 4mcsbw405gx1po"},
	{HNBU_ACPPR_SBPO,	0xfffff800, 49,	"2sb20in40hrpo 2sb20in80and160hr5glpo "
	"2sb40and80hr5glpo 2sb20in80and160hr5gmpo 2sb40and80hr5gmpo 2sb20in80and160hr5ghpo "
	"2sb40and80hr5ghpo 2sb20in40lrpo 2sb20in80and160lr5glpo 2sb40and80lr5glpo "
	"2sb20in80and160lr5gmpo 2sb40and80lr5gmpo 2sb20in80and160lr5ghpo 2sb40and80lr5ghpo "
	"4dot11agduphrpo 4dot11agduplrpo 2sb20in40and80hrpo 2sb20in40and80lrpo "
	"2sb20in80and160hr5gx1po 2sb20in80and160lr5gx1po 2sb40and80hr5gx1po 2sb40and80lr5gx1po "
	},
	{HNBU_ACPPR_SB8080_PO, 0xfffff800, 23, "2sb2040and80in80p80hr5glpo "
	"2sb2040and80in80p80lr5glpo 2sb2040and80in80p80hr5gmpo "
	"2sb2040and80in80p80lr5gmpo 2sb2040and80in80p80hr5ghpo 2sb2040and80in80p80lr5ghpo "
	"2sb2040and80in80p80hr5gx1po 2sb2040and80in80p80lr5gx1po 2sb20in80p80hr5gpo "
	"2sb20in80p80lr5gpo 2dot11agduppo"},
	{HNBU_NOISELVL,		0xfffff800, 16, "1noiselvl2ga0 1noiselvl2ga1 1noiselvl2ga2 "
	"1*4noiselvl5ga0 1*4noiselvl5ga1 1*4noiselvl5ga2"},
	{HNBU_RXGAIN_ERR,	0xfffff800, 16, "1rxgainerr2ga0 1rxgainerr2ga1 1rxgainerr2ga2 "
	"1*4rxgainerr5ga0 1*4rxgainerr5ga1 1*4rxgainerr5ga2"},
	{HNBU_AGBGA,		0xfffff800, 7, "1agbg0 1agbg1 1agbg2 1aga0 1aga1 1aga2"},
	{HNBU_USBDESC_COMPOSITE, 0xffffffff, 3, "2usbdesc_composite"},
	{HNBU_UUID, 		0xffffffff, 17,	"16uuid"},
	{HNBU_WOWLGPIO,		0xffffffff, 2,  "1wowl_gpio"},
	{HNBU_ACRXGAINS_C0,	0xfffff800, 5, "0rxgains5gtrelnabypa0 0rxgains5gtrisoa0 "
	"0rxgains5gelnagaina0 0rxgains2gtrelnabypa0 0rxgains2gtrisoa0 0rxgains2gelnagaina0 "
	"0rxgains5ghtrelnabypa0 0rxgains5ghtrisoa0 0rxgains5ghelnagaina0 0rxgains5gmtrelnabypa0 "
	"0rxgains5gmtrisoa0 0rxgains5gmelnagaina0"},	/* special case */
	{HNBU_ACRXGAINS_C1,	0xfffff800, 5, "0rxgains5gtrelnabypa1 0rxgains5gtrisoa1 "
	"0rxgains5gelnagaina1 0rxgains2gtrelnabypa1 0rxgains2gtrisoa1 0rxgains2gelnagaina1 "
	"0rxgains5ghtrelnabypa1 0rxgains5ghtrisoa1 0rxgains5ghelnagaina1 0rxgains5gmtrelnabypa1 "
	"0rxgains5gmtrisoa1 0rxgains5gmelnagaina1"},	/* special case */
	{HNBU_ACRXGAINS_C2,	0xfffff800, 5, "0rxgains5gtrelnabypa2 0rxgains5gtrisoa2 "
	"0rxgains5gelnagaina2 0rxgains2gtrelnabypa2 0rxgains2gtrisoa2 0rxgains2gelnagaina2 "
	"0rxgains5ghtrelnabypa2 0rxgains5ghtrisoa2 0rxgains5ghelnagaina2 0rxgains5gmtrelnabypa2 "
	"0rxgains5gmtrisoa2 0rxgains5gmelnagaina2"},	/* special case */
	{HNBU_TXDUTY, 		0xfffff800, 9,	"2tx_duty_cycle_ofdm_40_5g "
	"2tx_duty_cycle_thresh_40_5g 2tx_duty_cycle_ofdm_80_5g 2tx_duty_cycle_thresh_80_5g"},
	{HNBU_PDOFF_2G,		0xfffff800, 3,	"0pdoffset2g40ma0 0pdoffset2g40ma1 "
	"0pdoffset2g40ma2 0pdoffset2g40mvalid"},
	{HNBU_ACPA_CCK_C0,	0xfffff800, 7,  "2*3pa2gccka0"},
	{HNBU_ACPA_CCK_C1,	0xfffff800, 7,  "2*3pa2gccka1"},
	{HNBU_ACPA_40,		0xfffff800, 25,	"2*12pa5gbw40a0"},
	{HNBU_ACPA_80,		0xfffff800, 25,	"2*12pa5gbw80a0"},
	{HNBU_ACPA_4080,	0xfffff800, 49,	"2*12pa5gbw4080a0 2*12pa5gbw4080a1"},
	{HNBU_ACPA_4X4C0,	0xffffe000, 23, "1maxp2ga0 2*4pa2ga0 2*4pa2g40a0 "
	"1maxp5gb0a0 1maxp5gb1a0 1maxp5gb2a0 1maxp5gb3a0 1maxp5gb4a0"},
	{HNBU_ACPA_4X4C1,	0xffffe000, 23, "1maxp2ga1 2*4pa2ga1 2*4pa2g40a1 "
	"1maxp5gb0a1 1maxp5gb1a1 1maxp5gb2a1 1maxp5gb3a1 1maxp5gb4a1"},
	{HNBU_ACPA_4X4C2,	0xffffe000, 23, "1maxp2ga2 2*4pa2ga2 2*4pa2g40a2 "
	"1maxp5gb0a2 1maxp5gb1a2 1maxp5gb2a2 1maxp5gb3a2 1maxp5gb4a2"},
	{HNBU_ACPA_4X4C3,	0xffffe000, 23, "1maxp2ga3 2*4pa2ga3 2*4pa2g40a3 "
	"1maxp5gb0a3 1maxp5gb1a3 1maxp5gb2a3 1maxp5gb3a3 1maxp5gb4a3"},
	{HNBU_ACPA_BW20_4X4C0,	0xffffe000, 41, "2*20pa5ga0"},
	{HNBU_ACPA_BW40_4X4C0,	0xffffe000, 41, "2*20pa5g40a0"},
	{HNBU_ACPA_BW80_4X4C0,	0xffffe000, 41, "2*20pa5g80a0"},
	{HNBU_ACPA_BW20_4X4C1,	0xffffe000, 41, "2*20pa5ga1"},
	{HNBU_ACPA_BW40_4X4C1,	0xffffe000, 41, "2*20pa5g40a1"},
	{HNBU_ACPA_BW80_4X4C1,	0xffffe000, 41, "2*20pa5g80a1"},
	{HNBU_ACPA_BW20_4X4C2,	0xffffe000, 41, "2*20pa5ga2"},
	{HNBU_ACPA_BW40_4X4C2,	0xffffe000, 41, "2*20pa5g40a2"},
	{HNBU_ACPA_BW80_4X4C2,	0xffffe000, 41, "2*20pa5g80a2"},
	{HNBU_ACPA_BW20_4X4C3,	0xffffe000, 41, "2*20pa5ga3"},
	{HNBU_ACPA_BW40_4X4C3,	0xffffe000, 41, "2*20pa5g40a3"},
	{HNBU_ACPA_BW80_4X4C3,	0xffffe000, 41, "2*20pa5g80a3"},
	{HNBU_SUBBAND5GVER,	0xfffff800, 3,	"2subband5gver"},
	{HNBU_PAPARAMBWVER,	0xfffff800, 2,	"1paparambwver"},
	{HNBU_TXBFRPCALS,  0xfffff800, 11,
	"2rpcal2g 2rpcal5gb0 2rpcal5gb1 2rpcal5gb2 2rpcal5gb3"}, /* txbf rpcalvars */
	{HNBU_GPIO_PULL_DOWN,	0xffffffff, 5, "4gpdn"},
	{HNBU_MACADDR2,		0xffffffff, 7, "6macaddr2"},	/* special case */
	{0xFF,			0xffffffff, 0, ""}
};

#endif /* _bcmsrom_tbl_h_ */
