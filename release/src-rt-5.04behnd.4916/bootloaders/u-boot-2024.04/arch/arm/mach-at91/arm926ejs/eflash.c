// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Reinhard Meyer, EMK Elektronik, reinhard.meyer@emk-elektronik.de
 */

/*
 * this driver supports the enhanced embedded flash in the Atmel
 * AT91SAM9XE devices with the following geometry:
 *
 * AT91SAM9XE128: 1 plane of  8 regions of 32 pages (total  256 pages)
 * AT91SAM9XE256: 1 plane of 16 regions of 32 pages (total  512 pages)
 * AT91SAM9XE512: 1 plane of 32 regions of 32 pages (total 1024 pages)
 * (the exact geometry is read from the flash at runtime, so any
 *  future devices should already be covered)
 *
 * Regions can be write/erase protected.
 * Whole (!) pages can be individually written with erase on the fly.
 * Writing partial pages will corrupt the rest of the page.
 *
 * The flash is presented to u-boot with each region being a sector,
 * having the following effects:
 * Each sector can be hardware protected (protect on/off).
 * Each page in a sector can be rewritten anytime.
 * Since pages are erased when written, the "erase" does nothing.
 * The first "CONFIG_EFLASH_PROTSECTORS" cannot be unprotected
 * by u-Boot commands.
 *
 * Note: Redundant environment will not work in this flash since
 * it does use partial page writes. Make sure the environment spans
 * whole pages!
 */

/*
 * optional TODOs (nice to have features):
 *
 * make the driver coexist with other NOR flash drivers
 *	(use an index into flash_info[], requires work
 *	in those other drivers, too)
 * Make the erase command fill the sectors with 0xff
 *	(if the flashes grow larger in the future and
 *	someone puts a jffs2 into them)
 * do a read-modify-write for partially programmed pages
 */
#include <common.h>
#include <display_options.h>
#include <flash.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_eefc.h>
#include <asm/arch/at91_dbu.h>

/* checks to detect configuration errors */
#if CONFIG_SYS_MAX_FLASH_BANKS!=1
#error eflash: this driver can only handle 1 bank
#endif

/* global structure */
flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];
static u32 pagesize;

unsigned long flash_init(void)
{
	at91_eefc_t *eefc = (at91_eefc_t *) ATMEL_BASE_EEFC;
	at91_dbu_t *dbu = (at91_dbu_t *) ATMEL_BASE_DBGU;
	u32 id, size, nplanes, planesize, nlocks;
	u32 addr, i, tmp=0;

	debug("eflash: init\n");

	flash_info[0].flash_id = FLASH_UNKNOWN;

	/* check if its an AT91ARM9XE SoC */
	if ((readl(&dbu->cidr) & AT91_DBU_CID_ARCH_MASK) != AT91_DBU_CID_ARCH_9XExx) {
		puts("eflash: not an AT91SAM9XE\n");
		return 0;
	}

	/* now query the eflash for its structure */
	writel(AT91_EEFC_FCR_KEY | AT91_EEFC_FCR_FCMD_GETD, &eefc->fcr);
	while ((readl(&eefc->fsr) & AT91_EEFC_FSR_FRDY) == 0)
		;
	id = readl(&eefc->frr);		/* word 0 */
	size = readl(&eefc->frr);	/* word 1 */
	pagesize = readl(&eefc->frr);	/* word 2 */
	nplanes = readl(&eefc->frr);	/* word 3 */
	planesize = readl(&eefc->frr);	/* word 4 */
	debug("id=%08x size=%u pagesize=%u planes=%u planesize=%u\n",
		id, size, pagesize, nplanes, planesize);
	for (i=1; i<nplanes; i++) {
		tmp = readl(&eefc->frr);	/* words 5..4+nplanes-1 */
	};
	nlocks = readl(&eefc->frr);	/* word 4+nplanes */
	debug("nlocks=%u\n", nlocks);
	/* since we are going to use the lock regions as sectors, check count */
	if (nlocks > CONFIG_SYS_MAX_FLASH_SECT) {
		printf("eflash: number of lock regions(%u) "\
			"> CONFIG_SYS_MAX_FLASH_SECT. reducing...\n",
			nlocks);
		nlocks = CONFIG_SYS_MAX_FLASH_SECT;
	}
	flash_info[0].size = size;
	flash_info[0].sector_count = nlocks;
	flash_info[0].flash_id = id;

	addr = ATMEL_BASE_FLASH;
	for (i=0; i<nlocks; i++) {
		tmp = readl(&eefc->frr);	/* words 4+nplanes+1.. */
		flash_info[0].start[i] = addr;
		flash_info[0].protect[i] = 0;
		addr += tmp;
	};

	/* now read the protection information for all regions */
	writel(AT91_EEFC_FCR_KEY | AT91_EEFC_FCR_FCMD_GLB, &eefc->fcr);
	while ((readl(&eefc->fsr) & AT91_EEFC_FSR_FRDY) == 0)
		;
	for (i=0; i<flash_info[0].sector_count; i++) {
		if (i%32 == 0)
			tmp = readl(&eefc->frr);
		flash_info[0].protect[i] = (tmp >> (i%32)) & 1;
#if CONFIG_VAL(EFLASH_PROTSECTORS)
		if (i < CONFIG_EFLASH_PROTSECTORS)
			flash_info[0].protect[i] = 1;
#endif
	}

	return size;
}

void flash_print_info(flash_info_t *info)
{
	int i;

	puts("AT91SAM9XE embedded flash\n  Size: ");
	print_size(info->size, " in ");
	printf("%d Sectors\n", info->sector_count);

	printf("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf("\n   ");
		printf(" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     "
		);
	}
	printf ("\n");
	return;
}

int flash_real_protect (flash_info_t *info, long sector, int prot)
{
	at91_eefc_t *eefc = (at91_eefc_t *) ATMEL_BASE_EEFC;
	u32 pagenum = (info->start[sector]-ATMEL_BASE_FLASH)/pagesize;
	u32 i, tmp=0;

	debug("protect sector=%ld prot=%d\n", sector, prot);

#if CONFIG_VAL(EFLASH_PROTSECTORS)
	if (sector < CONFIG_EFLASH_PROTSECTORS) {
		if (!prot) {
			printf("eflash: sector %lu cannot be unprotected\n",
				sector);
		}
		return 1; /* return anyway, caller does not care for result */
	}
#endif
	if (prot) {
		writel(AT91_EEFC_FCR_KEY | AT91_EEFC_FCR_FCMD_SLB |
			(pagenum << AT91_EEFC_FCR_FARG_SHIFT), &eefc->fcr);
	} else {
		writel(AT91_EEFC_FCR_KEY | AT91_EEFC_FCR_FCMD_CLB |
			(pagenum << AT91_EEFC_FCR_FARG_SHIFT), &eefc->fcr);
	}
	while ((readl(&eefc->fsr) & AT91_EEFC_FSR_FRDY) == 0)
		;
	/* now re-read the protection information for all regions */
	writel(AT91_EEFC_FCR_KEY | AT91_EEFC_FCR_FCMD_GLB, &eefc->fcr);
	while ((readl(&eefc->fsr) & AT91_EEFC_FSR_FRDY) == 0)
		;
	for (i=0; i<info->sector_count; i++) {
		if (i%32 == 0)
			tmp = readl(&eefc->frr);
		info->protect[i] = (tmp >> (i%32)) & 1;
	}
	return 0;
}

static u32 erase_write_page (u32 pagenum)
{
	at91_eefc_t *eefc = (at91_eefc_t *) ATMEL_BASE_EEFC;

	debug("erase+write page=%u\n", pagenum);

	/* give erase and write page command */
	writel(AT91_EEFC_FCR_KEY | AT91_EEFC_FCR_FCMD_EWP |
		(pagenum << AT91_EEFC_FCR_FARG_SHIFT), &eefc->fcr);
	while ((readl(&eefc->fsr) & AT91_EEFC_FSR_FRDY) == 0)
		;
	/* return status */
	return readl(&eefc->fsr)
		& (AT91_EEFC_FSR_FCMDE | AT91_EEFC_FSR_FLOCKE);
}

int flash_erase(flash_info_t *info, int s_first, int s_last)
{
	debug("erase first=%d last=%d\n", s_first, s_last);
	puts("this flash does not need and support erasing!\n");
	return 0;
}

/*
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 */

int write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	u32 pagenum;
	u32 *src32, *dst32;
	u32 i;

	debug("write src=%08lx addr=%08lx cnt=%lx\n",
		(ulong)src, addr, cnt);

	/* REQUIRE addr to be on a page start, abort if not */
	if (addr % pagesize) {
		printf ("eflash: start %08lx is not on page start\n"\
			"        write aborted\n", addr);
		return 1;
	}

	/* now start copying data */
	pagenum = (addr-ATMEL_BASE_FLASH)/pagesize;
	src32 = (u32 *) src;
	dst32 = (u32 *) addr;
	while (cnt > 0) {
		i = pagesize / 4;
		/* fill page buffer */
		while (i--)
			*dst32++ = *src32++;
		/* write page */
		if (erase_write_page(pagenum))
			return 1;
		pagenum++;
		if (cnt > pagesize)
			cnt -= pagesize;
		else
			cnt = 0;
	}
	return 0;
}
