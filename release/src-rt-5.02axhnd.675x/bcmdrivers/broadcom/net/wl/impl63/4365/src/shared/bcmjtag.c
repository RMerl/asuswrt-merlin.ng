/*
 * JTAG access interface for drivers
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
 * $Id: bcmjtag.c 708017 2017-06-29 14:11:45Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <hndsoc.h>
#include <siutils.h>
#include <pcicfg.h>
#include "ejtag.h"
#include "bcmjtag.h"

/* JTAG master information block */
struct bcmjtag_info {
	osl_t *mosh;	/* handle to jtag master's osl */
	si_t *sih;	/* handle to jtag target's backplane */
	osl_t *osh;	/* handle to jtag target's osl */
	char *vars;
	int varsz;
};

#define JTAG_ADDR_MASK	0x3fffffff /* JTAG address is 30 bit long */

/* debugging macros */
#ifdef BCMDBG_ERR
#define JT_MSG(x)	printf x
#else
#define JT_MSG(x)
#endif /* BCMDBG_ERR */

/*
 * Attach to JTAG master. Allocate resources to enable access to
 * SiliconBackplane on 'remote' chip through JTAGM/JTAG.
 */
bcmjtag_info_t *
bcmjtag_attach(osl_t *osh, uint16 jtmvendorid, uint16 jtmdevid,
	void *jtmregs, uint jtmbustype, void *btparam,
	bool diffend)
{
	uint32 sbidh;
	sbconfig_t *sbc;
	bcmjtag_info_t *jtih;

	JT_MSG(("bcmjtag_attach: vendor %04x device %04x bustype %d\n",
		jtmvendorid, jtmdevid, jtmbustype));

	/* only PCI jtagm is supported now */
	if (jtmbustype != PCI_BUS) {
		JT_MSG(("bcmjtag_attach: unsupported bus type %d\n", jtmbustype));
		goto exit1;
	}

	/* alloc JTAG master handle */
	jtih = MALLOC(osh, sizeof(bcmjtag_info_t));
	if (!jtih) {
		JT_MSG(("bcmjtag_attach: out of memory, malloced %d bytes\n", MALLOCED(osh)));
		goto exit1;
	}
	bzero(jtih, sizeof(bcmjtag_info_t));
	jtih->mosh = osh;

	/* point JTAG master's PCI access to chipc */
	OSL_PCI_WRITE_CONFIG(osh, PCI_BAR0_WIN, 4, SI_ENUM_BASE);

	/* init JTAGM access */
	sbc = (sbconfig_t *)((uintptr)jtmregs + SBCONFIGOFF);
	sbidh = sbc->sbidhigh;
	if (ejtag_init(jtmdevid, sbidh, jtmregs, diffend)) {
		JT_MSG(("bcmjtag_attach: ejtag_init failed\n"));
		goto exit3;
	}

	/* attach to jtag target's osl */
	if (!(jtih->osh = osl_attach(btparam, JTAG_BUS, FALSE))) {
		JT_MSG(("bcmjtag_attach: device osl_attach failed\n"));
		goto exit4;
	}

	/* attach to jtag target's sb and cache the handle */
	if (!(jtih->sih = si_attach(jtmdevid, jtih->osh,
	                            (void *)SI_ENUM_BASE, JTAG_BUS, NULL,
	                            &jtih->vars, &jtih->varsz))) {
		JT_MSG(("bcmjtag_attach: si_attach failed\n"));
		goto exit5;
	}

	/* read nvram from flash */
	if (nvram_init((void *)jtih->sih)) {
		JT_MSG(("bcmjtag_attach: nvram_init failed\n"));
		goto exit6;
	}

	return jtih;

	/* error handling */
exit6:
	si_detach(jtih->sih);
exit5:
	osl_detach(jtih->osh);
exit4:
	ejtag_cleanup();
exit3:
	MFREE(osh, jtih, sizeof(bcmjtag_info_t));
exit1:
	return NULL;
}

/*
* Detach from JTAG master. Release resources allocated.
*/
int
bcmjtag_detach(bcmjtag_info_t *jtih)
{
	/* make sure the jtih handle is valid */
	if (!jtih) {
		JT_MSG(("bcmjtag_detach: invalid JTAGM handle\n"));
		return -1;
	}

	/* release nvram */
	nvram_exit((void *)jtih->sih);

	/* detach from si module */
	si_detach(jtih->sih);
	osl_detach(jtih->osh);

	/* release ejtag resources */
	ejtag_cleanup();

	/* free jtih handle and release all resources */
	MFREE(jtih->mosh, jtih, sizeof(bcmjtag_info_t));

	return 0;
}

/*
* Proxy 'remote' OCP/SI register read/write request thru JTAGM/JTAG
*/
uint32
bcmjtag_read(bcmjtag_info_t *jtih, uint32 addr, uint size)
{
	ulong val = 0;
	read_ejtag((ulong)addr & JTAG_ADDR_MASK, &val, size);
	return (uint32)val;
}

void
bcmjtag_write(bcmjtag_info_t *jtih, uint32 addr, uint32 val, uint size)
{
	write_ejtag((ulong)addr & JTAG_ADDR_MASK, (ulong)val, size);
}

/*
* Check if vendor/device is supported.
*/
bool
bcmjtag_chipmatch(uint16 vendor, uint16 device)
{
	if (vendor != VENDOR_BROADCOM)
		return FALSE;

	return TRUE;
}

/*
* Enumerate thru all target devices with the given 'venid' and 'devid'
* and call the user supplied callback function for each target device found.
* 'venid' and 'devid' are core ids.
*/
int bcmjtag_devattach(bcmjtag_info_t *jtih, uint16 venid, uint16 devid,
	bool (*dcb)(void *arg, uint16 venid, uint16 devid, void *devregs),
	void *arg)
{
	uint coreidx;
	uint16 devunit;
	void *devregs;
	int count;
	uint16 pcivendor, pcidevice;
	uint8 pciclass, pcisubclass, pciprogif;
	uint8 pciheader;

	coreidx = si_coreidx(jtih->sih);
	for (devunit = 0, count = 0;
	    (devregs = si_setcore(jtih->sih, devid, devunit));
	    devunit ++) {
		si_corepciid(jtih->sih, 0, &pcivendor, &pcidevice,
		             &pciclass, &pcisubclass, &pciprogif,
		             &pciheader);
		if (dcb(arg, pcivendor, pcidevice, devregs))
			count ++;
	}
	si_setcoreidx(jtih->sih, coreidx);

	return count;
}
