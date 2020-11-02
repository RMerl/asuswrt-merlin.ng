/*
 * Broadcom HNDSoC utlities, only for AP router
 * File: hndsoc.c
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
 * $Id: hndsoc.c 667654 2016-10-28 02:10:45Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <siutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <sbgci.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <nand_core.h>

static int bootdev = -1;
static int knldev = -1;

int
soc_boot_dev(void *socp)
{
	si_t *sih = (si_t *)socp;
	int bootfrom = SOC_BOOTDEV_SFLASH;
	uint32 origidx;
	uint32 option;

	if (bootdev != -1)
		return bootdev;

	origidx = si_coreidx(sih);

	/* Check 4707 (NorthStar) */
	if (sih->ccrev == 42) {
		if (si_setcore(sih, NS_ROM_CORE_ID, 0) != NULL) {
			option = si_core_sflags(sih, 0, 0) & SISF_NS_BOOTDEV_MASK;
			if (option == SISF_NS_BOOTDEV_NOR) {
				bootfrom = SOC_BOOTDEV_SFLASH;
			}
			else if (option == SISF_NS_BOOTDEV_NAND) {
				bootfrom = SOC_BOOTDEV_NANDFLASH;
			}
			else {
				/* This must be SISF_NS_BOOTDEV_ROM */
				bootfrom = SOC_BOOTDEV_ROM;
			}
		}
	}
	else {
		chipcregs_t *cc;
		gciregs_t *gci;

		/* Check 5357 */
		if (sih->ccrev == 38) {
			if ((sih->chipst & (1 << 4)) != 0) {
				bootfrom = SOC_BOOTDEV_NANDFLASH;
				goto found;
			}
			else if ((sih->chipst & (1 << 5)) != 0) {
				bootfrom =  SOC_BOOTDEV_ROM;
				goto found;
			}
		}

		/* Check 53573 */
		if (sih->ccrev == 54 && (gci = (gciregs_t *)si_setcore(sih, GCI_CORE_ID, 0))) {
			uint32 bootdev = 0;

			/* 53573 bootdev[1:0].  0: boot from sflash, 1: boot from nand */
			W_REG(NULL, &gci->gci_indirect_addr, 7);
			bootdev = R_REG(NULL, &gci->gci_chipsts);
			bootdev &= SI_BCM53573_BOOTDEV_MASK;

			if (bootdev == SI_BCM53573_BOOTDEV_NOR)
				bootfrom = SOC_BOOTDEV_SFLASH;
			else
				bootfrom = SOC_BOOTDEV_NANDFLASH;

			/* 47189A0 RGMII voltage setting */
			if (sih->chippkg == BCM47189_PKG_ID && sih->chiprev == 0) {
				uint32 vdd = 0;
				W_REG(NULL, &gci->gci_indirect_addr, 7);
				vdd = R_REG(NULL, &gci->gci_chipctrl);
				vdd &= ~(SI_BCM47189_RGMII_VDD_MASK << SI_BCM47189_RGMII_VDD_SHIFT);
				vdd |= (SI_BCM47189_RGMII_VDD_2_5V << SI_BCM47189_RGMII_VDD_SHIFT);
				W_REG(NULL, &gci->gci_chipctrl, vdd);
			}

			goto found;
		}
		/* Handle old soc, 4704, 4718 */
		if ((cc = (chipcregs_t *)si_setcoreidx(sih, SI_CC_IDX))) {
			option = R_REG(NULL, &cc->capabilities) & CC_CAP_FLASH_MASK;
			if (option == PFLASH)
				bootfrom = SOC_BOOTDEV_PFLASH;
			else
				bootfrom = SOC_BOOTDEV_SFLASH;
		}
	}

found:
	si_setcoreidx(sih, origidx);

	bootdev = bootfrom;
	return bootdev;
}

int
soc_knl_dev(void *socp)
{
	si_t *sih = (si_t *)socp;
	char *val;
	int knlfrom = SOC_KNLDEV_NORFLASH;

	if (knldev != -1)
		return knldev;

	if (soc_boot_dev(socp) == SOC_BOOTDEV_NANDFLASH) {
		knlfrom = SOC_KNLDEV_NANDFLASH;
		goto found;
	}

	if ((sih->ccrev == 38) &&
		(sih->cccaps & CC_CAP_NFLASH)) {
		goto check_nv;
	}
	else if (sih->ccrev == 42 || sih->ccrev == 54) {
		uint32 origidx;
		nandregs_t *nc;
		uint32 id = 0;

		origidx = si_coreidx(sih);
		if ((nc = (nandregs_t *)si_setcore(sih, NS_NAND_CORE_ID, 0)) != NULL) {
			id = R_REG(NULL, &nc->flash_device_id);
		}
		si_setcoreidx(sih, origidx);

		if (id != 0)
			goto check_nv;
	}
	else {
		/* Break through */
	}

	/* Default set to nor boot */
	goto found;

check_nv:
	/* Check NVRAM here */
	if ((val = nvram_get("bootflags")) != NULL) {
		int bootflags;
#ifdef	linux
		bootflags = simple_strtol(val, NULL, 0);
#else
		bootflags = atoi(val);
#endif // endif
		if (bootflags & FLASH_KERNEL_NFLASH)
			knlfrom = SOC_KNLDEV_NANDFLASH;
	}

found:
	knldev = knlfrom;
	return knldev;
}
