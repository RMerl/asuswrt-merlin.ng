/*
 * Broadcom chipcommon NAND flash interface
 *
 * Copyright (C) 2019, Broadcom. All Rights Reserved.
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
 * $Id: nflash.c 667654 2016-10-28 02:10:45Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <hndsoc.h>
#include <sbhndcpu.h>
#include <sbchipc.h>
#include <bcmdevs.h>
#include <nflash.h>
#include <hndpmu.h>

#ifdef BCMDBG
#define	NFL_MSG(args)	printf args
#else
#define	NFL_MSG(args)
#endif	/* BCMDBG */

#define NF_RETRIES	1000000

#define NF_SMALL_BADBLOCK_POS	5
#define NF_LARGE_BADBLOCK_POS	0

/* Private global state */
static hndnand_t nflash;

/* Prototype */
static int nflash_poll(si_t *sih, chipcregs_t *cc);
static int nflash_readst(si_t *sih, chipcregs_t *cc, uint8 *status);

hndnand_t *nflash_init(si_t *sih);
static void nflash_enable(hndnand_t *nfl, int enable);
static int nflash_read(hndnand_t *nfl, uint64 offset, uint len, uchar *buf);
static int nflash_write(hndnand_t *nfl, uint64 offset, uint len, const uchar *buf);
static int nflash_erase(hndnand_t *nfl, uint64 offset);
static int nflash_checkbadb(hndnand_t *nfl, uint64 offset);
static int nflash_mark_badb(hndnand_t *nfl, uint64 offset);

static void
nflash_enable(hndnand_t *nfl, int enable)
{
	si_t *sih = nfl->sih;

	if (sih->ccrev == 38) {
		/* BCM5357 NAND boot */
		if ((sih->chipst & (1 << 4)) != 0)
			return;

		if (enable)
			si_pmu_chipcontrol(sih, 1, CCTRL5357_NFLASH, CCTRL5357_NFLASH);
		else
			si_pmu_chipcontrol(sih, 1, CCTRL5357_NFLASH, 0);
	}
}

/* Issue a nand flash command */
static INLINE void
nflash_cmd(osl_t *osh, chipcregs_t *cc, uint opcode)
{
	W_REG(osh, &cc->nand_cmd_start, opcode);
	/* read after write to flush the command */
	R_REG(osh, &cc->nand_cmd_start);
}

static bool firsttime = TRUE;

static char *
nflash_check_id(uint8 *id)
{
	char *name = NULL;

	switch (id[0]) {
	case NFL_VENDOR_AMD:
		name = "AMD";
		break;
	case NFL_VENDOR_NUMONYX:
		name = "Numonyx";
		break;
	case NFL_VENDOR_MICRON:
		name = "Micron";
		break;
	case NFL_VENDOR_TOSHIBA:
		name = "Toshiba";
		break;
	case NFL_VENDOR_HYNIX:
		name = "Hynix";
		break;
	case NFL_VENDOR_SAMSUNG:
		name = "Samsung";
		break;
	case NFL_VENDOR_ESMT:
		name = "Esmt";
		break;
	case NFL_VENDOR_MXIC:
		name = "Mxic";
		break;
	case NFL_VENDOR_ZENTEL_ESMT:
		name = "Zentel/Esmt";
		break;
	default:
		printf("No NAND flash type found\n");
		break;
	}

	return name;
}

/* Initialize nand flash access */
hndnand_t *
nflash_init(si_t *sih)
{
	chipcregs_t *cc;
	uint32 id, id2;
	char *name = "";
	osl_t *osh;
	int i;
	uint32 ncf, val;
	uint32 acc_control;

	ASSERT(sih);

	if ((cc = (chipcregs_t *)si_setcoreidx(sih, SI_CC_IDX)) == NULL)
		return NULL;

	/* Only support chipcommon revision == 38 for now */
	if (sih->ccrev != 38)
		return NULL;

	/* Check if nand flash is mounted */
	if ((sih->cccaps & CC_CAP_NFLASH) != CC_CAP_NFLASH)
		return NULL;

	if (!firsttime && nflash.size)
		return &nflash;

	osh = si_osh(sih);
	bzero(&nflash, sizeof(nflash));

	nflash.sih = sih;
	nflash.core = (void *)cc;
	nflash.enable = nflash_enable;
	nflash.read = nflash_read;
	nflash.write = nflash_write;
	nflash.erase = nflash_erase;
	nflash.checkbadb = nflash_checkbadb;
	nflash.markbadb = nflash_mark_badb;

	nflash_enable(&nflash, 1);
	nflash_cmd(osh, cc, NCMD_ID_RD);
	if (nflash_poll(sih, cc) < 0) {
		nflash_enable(&nflash, 0);
		return NULL;
	}
	nflash_enable(&nflash, 0);
	id = R_REG(osh, &cc->nand_devid);
	id2 = R_REG(osh, &cc->nand_devid_x);
	for (i = 0; i < 5; i++) {
		if (i < 4)
			nflash.id[i] = (id >> (8*i)) & 0xff;
		else
			nflash.id[i] = id2 & 0xff;
	}

	name = nflash_check_id(nflash.id);
	if (name == NULL)
		return NULL;
	nflash.type = nflash.id[0];

	ncf = R_REG(osh, &cc->nand_config);
	/*  Page size (# of bytes) */
	val = (ncf & NCF_PAGE_SIZE_MASK) >> NCF_PAGE_SIZE_SHIFT;
	switch (val) {
	case 0:
		nflash.pagesize = 512;
		break;
	case 1:
		nflash.pagesize = (1 << 10) * 2;
		break;
		case 2:
		nflash.pagesize = (1 << 10) * 4;
		break;
	case 3:
		nflash.pagesize = (1 << 10) * 8;
		break;
	}
	/* Block size (# of bytes) */
	val = (ncf & NCF_BLOCK_SIZE_MASK) >> NCF_BLOCK_SIZE_SHIFT;
	switch (val) {
	case 0:
		nflash.blocksize = (1 << 10) * 16;
		break;
	case 1:
		nflash.blocksize = (1 << 10) * 128;
		break;
	case 2:
		nflash.blocksize = (1 << 10) * 8;
		break;
	case 3:
		nflash.blocksize = (1 << 10) * 512;
		break;
	case 4:
		nflash.blocksize = (1 << 10) * 256;
		break;
	default:
		printf("Unknown block size\n");
		return NULL;
	}
	/* NAND flash size in MBytes */
	val = (ncf & NCF_DEVICE_SIZE_MASK) >> NCF_DEVICE_SIZE_SHIFT;
	if (val == 0) {
		printf("Unknown flash size\n");
		return NULL;
	}
	nflash.size = (1 << (val - 1)) * 8;

	/* More attribues for ECC functions */
	acc_control = R_REG(osh, &cc->nand_acc_control);
	nflash.ecclevel = (acc_control & NAC_ECC_LEVEL_MASK) >> NAC_ECC_LEVEL_SHIFT;
	nflash.ecclevel0 = (acc_control & NAC_ECC_LEVEL0_MASK) >> NAC_ECC_LEVEL0_SHIFT;
	/* make sure that block-0 and block-n use the same ECC level */
	if (nflash.ecclevel != nflash.ecclevel0) {
		acc_control &= ~(NAC_ECC_LEVEL_MASK | NAC_ECC_LEVEL0_MASK);
		acc_control |=
			(nflash.ecclevel0 << NAC_ECC_LEVEL0_SHIFT) |
			(nflash.ecclevel0 << NAC_ECC_LEVEL_SHIFT);
		W_REG(osh, &cc->nand_acc_control, acc_control);
		nflash.ecclevel = nflash.ecclevel0;
	}
	nflash.phybase = SI_FLASH1;

	nflash.numblocks = (nflash.size * (1 << 10)) / (nflash.blocksize >> 10);
	if (firsttime)
		printf("Found a %s NAND flash with %uB pages or %dKB blocks; total size %dMB\n",
		       name, nflash.pagesize, (nflash.blocksize >> 10), nflash.size);
	firsttime = FALSE;
	return nflash.size ? &nflash : NULL;
}

/* Read len bytes starting at offset into buf. Returns number of bytes read. */
static int
nflash_read(hndnand_t *nfl, uint64 offset, uint len, uchar *buf)
{
	si_t *sih = nfl->sih;
	chipcregs_t *cc = (chipcregs_t *)nfl->core;
	uint32 mask;
	osl_t *osh;
	int i;
	uint32 *to;
	uint32 val;
	uint res;

	ASSERT(sih);
	mask = NFL_SECTOR_SIZE - 1;
	if ((offset & mask) != 0 || (len & mask) != 0)
		return 0;
	if ((((offset + len) >> 20) > nflash.size) ||
	    ((((offset + len) >> 20) == nflash.size) &&
	     (((offset + len) & ((1 << 20) - 1)) != 0)))
		return 0;
	osh = si_osh(sih);
	to = (uint32 *)buf;
	res = len;

	nflash_enable(nfl, 1);
	while (res > 0) {
		W_REG(osh, &cc->nand_cmd_addr, offset);
		nflash_cmd(osh, cc, NCMD_PAGE_RD);
		if (nflash_poll(sih, cc) < 0)
			break;
		if (((val = R_REG(osh, &cc->nand_intfc_status)) & NIST_CACHE_VALID) == 0)
			break;
		W_REG(osh, &cc->nand_cache_addr, 0);
		for (i = 0; i < NFL_SECTOR_SIZE; i += 4, to++) {
			*to = R_REG(osh, &cc->nand_cache_data);
		}

		res -= NFL_SECTOR_SIZE;
		offset += NFL_SECTOR_SIZE;
	}
	nflash_enable(nfl, 0);

	return (len - res);
}

/* Poll for command completion. Returns zero when complete. */
static int
nflash_poll(si_t *sih, chipcregs_t *cc)
{
	osl_t *osh;
	int i;
	uint32 pollmask;

	ASSERT(sih);
	osh = si_osh(sih);

	pollmask = NIST_CTRL_READY|NIST_FLASH_READY;
	for (i = 0; i < NF_RETRIES; i++) {
		if ((R_REG(osh, &cc->nand_intfc_status) & pollmask) == pollmask) {
			return 0;
		}
	}

	printf("nflash_poll: not ready\n");
	return -1;
}

/* Write len bytes starting at offset into buf. Returns number of bytes
 * written.
 */
static int
nflash_write(hndnand_t *nfl, uint64 offset, uint len, const uchar *buf)
{
	si_t *sih = nfl->sih;
	chipcregs_t *cc = (chipcregs_t *)nfl->core;
	uint32 mask;
	osl_t *osh;
	int i;
	uint32 *from;
	uint res;
	uint32 reg;
	int ret = 0;
	uint8 status;

	ASSERT(sih);
	mask = nflash.pagesize - 1;
	/* Check offset and length */
	if ((offset & mask) != 0 || (len & mask) != 0)
		return 0;
	if ((((offset + len) >> 20) > nflash.size) ||
	    ((((offset + len) >> 20) == nflash.size) &&
	     (((offset + len) & ((1 << 20) - 1)) != 0)))
		return 0;
	osh = si_osh(sih);

	from = (uint32 *)buf;
	res = len;

	nflash_enable(nfl, 1);
	/* disable partial page enable */
	reg = R_REG(osh, &cc->nand_acc_control);
	reg &= ~NAC_PARTIAL_PAGE_EN;
	W_REG(osh, &cc->nand_acc_control, reg);

	while (res > 0) {
		W_REG(osh, &cc->nand_cache_addr, 0);
		for (i = 0; i < nflash.pagesize; i += 4, from++) {
			if (i % 512 == 0)
				W_REG(osh, &cc->nand_cmd_addr, i);
			W_REG(osh, &cc->nand_cache_data, *from);
		}
		W_REG(osh, &cc->nand_cmd_addr, offset + nflash.pagesize - 512);
		nflash_cmd(osh, cc, NCMD_PAGE_PROG);
		if (nflash_poll(sih, cc) < 0)
			break;

		/* Check status */
		W_REG(osh, &cc->nand_cmd_start, NCMD_STATUS_RD);
		if (nflash_poll(sih, cc) < 0)
			break;
		status = R_REG(osh, &cc->nand_intfc_status) & NIST_STATUS;
		if (status & 1) {
			ret = -1;
			break;
		}
		res -= nflash.pagesize;
		offset += nflash.pagesize;
	}

	nflash_enable(nfl, 0);
	if (ret)
		return ret;

	return (len - res);
}

/* Erase a region. Returns number of bytes scheduled for erasure.
 * Caller should poll for completion.
 */
static int
nflash_erase(hndnand_t *nfl, uint64 offset)
{
	si_t *sih = nfl->sih;
	chipcregs_t *cc = (chipcregs_t *)nfl->core;
	osl_t *osh;
	int ret = 0;
	uint8 status = 0;

	ASSERT(sih);

	osh = si_osh(sih);
	if ((offset >> 20) >= nflash.size)
		return -1;
	if ((offset & (nflash.blocksize - 1)) != 0) {
		return -1;
	}

	nflash_enable(nfl, 1);
	W_REG(osh, &cc->nand_cmd_addr, offset);
	nflash_cmd(osh, cc, NCMD_BLOCK_ERASE);
	if (nflash_poll(sih, cc) < 0) {
		ret = -1;
		goto err;
	}
	/* Check status */
	W_REG(osh, &cc->nand_cmd_start, NCMD_STATUS_RD);
	if (nflash_poll(sih, cc) < 0) {
		ret = -1;
		goto err;
	}
	status = R_REG(osh, &cc->nand_intfc_status) & NIST_STATUS;
	if (status & 1)
		ret = -1;
err:
	nflash_enable(nfl, 0);

	return ret;
}

static int
nflash_checkbadb(hndnand_t *nfl, uint64 offset)
{
	si_t *sih = nfl->sih;
	chipcregs_t *cc = (chipcregs_t *)nfl->core;
	osl_t *osh;
	int i;
	uint off;
	uint32 nand_intfc_status;
	int ret = 0;

	ASSERT(sih);

	osh = si_osh(sih);
	if ((offset >> 20) >= nflash.size)
		return -1;
	if ((offset & (nflash.blocksize - 1)) != 0) {
		return -1;
	}

	nflash_enable(nfl, 1);
	for (i = 0; i < 2; i++) {
		off = offset + (nflash.pagesize * i);
		W_REG(osh, &cc->nand_cmd_addr, off);
		nflash_cmd(osh, cc, NCMD_SPARE_RD);
		if (nflash_poll(sih, cc) < 0) {
			ret = -1;
			goto err;
		}
		nand_intfc_status = R_REG(osh, &cc->nand_intfc_status) & NIST_SPARE_VALID;
		if (nand_intfc_status != NIST_SPARE_VALID) {
			ret = -1;
			goto err;
		}
		if ((R_REG(osh, &cc->nand_spare_rd0) & 0xff) != 0xff) {
			ret = -1;
			goto err;
		}
	}
err:
	nflash_enable(nfl, 0);
	return ret;
}

static int
nflash_mark_badb(hndnand_t *nfl, uint64 offset)
{
	si_t *sih = nfl->sih;
	chipcregs_t *cc = (chipcregs_t *)nfl->core;
	osl_t *osh;
	uint off;
	int i, ret = 0;
	uint32 reg;

	ASSERT(sih);

	osh = si_osh(sih);
	if ((offset >> 20) >= nflash.size)
		return -1;
	if ((offset & (nflash.blocksize - 1)) != 0) {
		return -1;
	}

	nflash_enable(nfl, 1);
	/* Erase block */
	W_REG(osh, &cc->nand_cmd_addr, offset);
	nflash_cmd(osh, cc, NCMD_BLOCK_ERASE);
	if (nflash_poll(sih, cc) < 0) {
		ret = -1;
		goto err;
	}

	/*
	 * Enable partial page programming and disable ECC checkbit generation
	 * for PROGRAM_SPARE_AREA
	 */
	reg = R_REG(osh, &cc->nand_acc_control);
	reg |= NAC_PARTIAL_PAGE_EN;
	reg &= ~NAC_WR_ECC_EN;
	W_REG(osh, &cc->nand_acc_control, reg);

	for (i = 0; i < 2; i++) {
		off = offset + (nflash.pagesize * i);
		W_REG(osh, &cc->nand_cmd_addr, off);

		W_REG(osh, &cc->nand_spare_wr0, 0);
		W_REG(osh, &cc->nand_spare_wr4, 0);
		W_REG(osh, &cc->nand_spare_wr8, 0);
		W_REG(osh, &cc->nand_spare_wr12, 0);

		nflash_cmd(osh, cc, NCMD_SPARE_PROG);
		if (nflash_poll(sih, cc) < 0) {
			ret = -1;
			goto err;
		}
	}
err:
	/* Restore the default value for spare area write registers */
	W_REG(osh, &cc->nand_spare_wr0, 0xffffffff);
	W_REG(osh, &cc->nand_spare_wr4, 0xffffffff);
	W_REG(osh, &cc->nand_spare_wr8, 0xffffffff);
	W_REG(osh, &cc->nand_spare_wr12, 0xffffffff);

	/*
	 * Disable partial page programming and enable ECC checkbit generation
	 * for PROGRAM_SPARE_AREA
	 */
	reg = R_REG(osh, &cc->nand_acc_control);
	reg &= ~NAC_PARTIAL_PAGE_EN;
	reg |= NAC_WR_ECC_EN;
	W_REG(osh, &cc->nand_acc_control, reg);

	nflash_enable(nfl, 0);

	return ret;
}

static int
nflash_readst(si_t *sih, chipcregs_t *cc, uint8 *status)
{
	osl_t *osh;
	int ret = 0;

	ASSERT(sih);

	osh = si_osh(sih);

	nflash_enable(&nflash, 1);
	W_REG(osh, &cc->nand_cmd_start, NCMD_STATUS_RD);

	if (nflash_poll(sih, cc) < 0)
		ret = -1;

	else
		*status = (uint8)(R_REG(osh, &cc->nand_intfc_status) & NIST_STATUS);

	nflash_enable(&nflash, 0);
	return ret;
}
