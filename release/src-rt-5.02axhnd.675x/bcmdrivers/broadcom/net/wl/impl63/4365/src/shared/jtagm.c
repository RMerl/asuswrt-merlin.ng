/*
 * JTAG master (Broadcom chipcommon) access interface
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
 * $Id: jtagm.c 708017 2017-06-29 14:11:45Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#ifdef BCMDBG_ERR
#define JTM_MSG(x) printf x
#else
#define JTM_MSG(x)
#endif // endif
#else	/* BCMDRIVER */
#include <stdio.h>
#include <stdlib.h>
#define JTM_MSG(x) printf x
#endif	/* BCMDRIVER */
#include <bcmdevs.h>
#include <sbchipc.h>
#include <hndsoc.h>
#ifndef BCMDRIVER
#include "remotepci.h"
#endif	/* BCMDRIVER */
#include "ejtag.h"
#include "jtagm.h"

/* Jtag master config */

#define JTAGM_RETRIES	10000 /* # of retries for Jtag master */

unsigned short jtagm_pciid = 0;		/* PCI id of the jtag master */
unsigned short jtagm_chipid = 0;	/* Chipcommon chipid of the jtag master */
unsigned short jtagm_crev = 0;		/* Chipcommon rev */
chipcregs_t *jtagm_regs = NULL;		/* For local (pci) access */
uint jtagm_pcicfg = 0xffffffff;		/* For local (pci config) access */
uint jtagm_rembase = 0;			/* For use with tcp/ip */
uint jtagm_clkd = 0;			/* Jtag clock divisor */
uint jtagm_inttap = 0;			/* Internal TAP or external targets? */

uint32
jtagm_rreg(uint reg)
{
	uint32 data;

#ifndef BCMDRIVER
	if (jtagm_rembase) {
		if (rempci_read(jtagm_rembase + reg, (uchar *)&data, 4, REMPCI_MEM) != 0) {
			JTM_MSG(("%s: Error reading remote jtagm reg 0x%x\n", __FUNCTION__, reg));
			return -1;
		}
	}
	else
#endif // endif
	{
		data = *((uint32 *)((uintptr)jtagm_regs + reg));
	}

	if (jtag_trace & JTAG_TRACE_REGS) {
		JTM_MSG((" reg: 0x%x => 0x%x\n", reg, data));
	}

	return data;
}

void
jtagm_wreg(uint reg, uint32 data)
{
	if (jtag_trace & JTAG_TRACE_REGS) {
		JTM_MSG((" reg: 0x%x <= 0x%x\n", reg, data));
	}

#ifndef BCMDRIVER
	if (jtagm_rembase) {
		if (rempci_write(jtagm_rembase + reg, (uchar *)&data, 4, REMPCI_MEM) != 0) {
			JTM_MSG(("%s: Error writing remote jtagm reg 0x%x\n", __FUNCTION__, reg));
			return;
		}
	}
	else
#endif // endif
	{
		*((uint32 *)((uintptr)jtagm_regs + reg)) = data;
	}
}

uint
jtagm_scmd(uint ir, uint irw, uint dr, uint drw)
{
	uint32 data, i;

	if (jtag_trace & JTAG_TRACE_SIGNALS) {
		JTM_MSG((" ir <= 0x%x/%d,\tdr <= 0x%x/%d\n", ir, irw, dr, drw));
	}

	jtagm_wreg(CC_JTAGIR, ir);
	jtagm_wreg(CC_JTAGDR, dr);
	jtagm_wreg(CC_JTAGCMD, (JCMD_START |
	                        ((jtagm_crev == 10) ? JCMD0_ACC_IRDR : JCMD_ACC_IRDR) |
	                        ((irw - 1) << JCMD_IRW_SHIFT) | (drw - 1)));

	i = 0;
	while (((jtagm_rreg(CC_JTAGCMD) & JCMD_BUSY) == JCMD_BUSY) &&
	       (i < JTAGM_RETRIES)) {
		/* usleep(1) */;
		i++;
	}

	if (i == JTAGM_RETRIES)
		data = 0xffffffff;
	else
		data = jtagm_rreg(CC_JTAGDR);

	if (jtag_trace & JTAG_TRACE_SIGNALS) {
		JTM_MSG(("\t\tdr => 0x%x\n", data));
	}
	return data;
}

int
jtagm_init(uint16 devid, uint32 sbidh, void *regsva, bool remote)
{
	uint32 clkdiv, cap, tap;

	/* Check that it really is a chipc core, rev >= 10 */
	if ((((sbidh & SBIDH_CC_MASK) >> SBIDH_CC_SHIFT) != CC_CORE_ID) ||
	    (SBCOREREV(sbidh) < 10)) {
		JTM_MSG(("jtagm_init: rev < 10\n"));
		return -1;
	}

	if (remote) {
#ifdef BCMDRIVER
		return -1;
#else
		jtagm_rembase = (uint)regsva;
#endif // endif
	} else
		jtagm_regs = (chipcregs_t *)regsva;
	jtagm_pciid = devid;

	/* check if JTAGM present? */
	cap = jtagm_rreg(CC_CAPABILITIES);
	if ((cap & CC_CAP_JTAGP) == 0) {
		JTM_MSG(("jtagm_init: (cap & CC_CAP_JTAGP) == 0\n"));
		jtagm_regs = NULL;
		jtagm_rembase = 0;
		return -1;
	}

	/* config JTAGM */
	jtagm_chipid = jtagm_rreg(CC_CHIPID);
	jtagm_crev = SBCOREREV(sbidh);

	if (jtagm_clkd == 0) {
		if (jtagm_pciid == FPGA_JTAGM_ID) {
			/* If this is the fpga, set the default clkd to 2 */
			jtagm_clkd = 2;
		} else if ((jtagm_chipid & CID_ID_MASK) == BCM4318_CHIP_ID) {
			/* A 4318, set clkdiv to 11 to get 8Mhz */
			jtagm_clkd = 11;
		}
	}

	clkdiv = jtagm_rreg(CC_CLKDIV);
	if (jtagm_clkd == 0)
		jtagm_clkd = (clkdiv & CLKD_JTAG) >> CLKD_JTAG_SHIFT;
	jtagm_wreg(CC_CLKDIV, (clkdiv & ~CLKD_JTAG) | (jtagm_clkd << CLKD_JTAG_SHIFT));
	/* Make sure we run at HT */
	if (jtagm_crev < 20)
		jtagm_wreg(CC_SYS_CLK_CTL, 0);
	else
		jtagm_wreg(CC_CLK_CTL_ST, jtagm_rreg(CC_CLK_CTL_ST) | CCS_FORCEHT);

	tap = jtagm_inttap ? JCTRL_EN : (JCTRL_EXT_EN | JCTRL_EN);
	jtagm_wreg(CC_JTAGCTRL, tap);

	/* now we are ready to go! */
	JTM_MSG(("JTAG master pciid 0x%x, rev %d, clkd=%d, %s tap\n",
		jtagm_pciid, jtagm_crev, jtagm_clkd,
		jtagm_inttap ? "internal" : "external"));

	return 0;
}
