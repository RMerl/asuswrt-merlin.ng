/*
 * FCBS core module impl
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: fcbs.c 692261 2017-03-27 10:20:22Z $
 */

#ifdef DONGLEBUILD

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <d11.h>
#include <hndd11.h>
#include <fcbs.h>
#include <fcbsutils.h>
#include <osl.h>

/* prints */
#define FCBS_DBG_BMDUMP 0

#define FCBS_DYN_SEQ	{FCBS_BLK_DYNAMIC, {NULL}}
#define FCBS_ROM_SEQ(n)	{FCBS_BLK_ROM, {n}}

#define FCBS_DS0_BLK(block, halt_on_err, core) \
		{block, ARRAYSIZE(block), NULL, halt_on_err, FCBS_DS0, core}
#define FCBS_DS1_BLK(block, halt_on_err, core) \
		{block, ARRAYSIZE(block), NULL, halt_on_err, FCBS_DS1, core}

#define FCBS_CMD_PTR_LSB_SZ		16
#define FCBS_CMD_PTR_LSB_MASK		0xFFFF
#define FCBS_CMD_PTR_MSB_SHIFT		0
#define FCBS_CMD_PTR_MSB_MASK		0x7

#define FCBS_DAT_PTR_LSB_SZ		16
#define FCBS_DAT_PTR_LSB_MASK		0xFFFF
#define FCBS_DAT_PTR_MSB_SHIFT		3
#define FCBS_DAT_PTR_MSB_MASK		0x7

#define FCBS_CTRL_HALTONERR_SHIFT	14
#define FCBS_CTRL_HALTONERR_MASK	0x1

#define FCBS_CTRL_PHYBACKTOBACKEN_SHIFT	11
#define FCBS_CTRL_PHYBACKTOBACKEN_MASK	0x1

#define FCBS_MAX_WAIT_TIME		100
#define	FCBS_CTRL_STATUS_MAGIC		0xABBA
#define FCBS_ACT_CMPL_TIME_MAGIC	0xCDDC

/* Enabling the FCBS sequnces */
#define	FCBS_DS1_MAC_INIT_BLOCK_ENABLE	1
#define	FCBS_DS1_PHY_RADIO_BLOCK_ENABLE	1
#define	FCBS_DS1_RADIO_PD_BLOCK_ENABLE	1
#define	FCBS_DS1_EXIT_BLOCK_ENABLE	1
#define	FCBS_DS0_RADIO_PU_BLOCK_ENABLE	1
#define FCBS_DS0_RADIO_PD_BLOCK_ENABLE	1

#define HALTONERR	1
#define NO_HALTONERR	0

typedef enum fcbs_cores {
	FCBS_MAIN_CORE,
	FCBS_AUXI_CORE,
	MAX_FCBS_CORES
} fcbs_cores_t;

typedef enum fcbs_blk_type {
	FCBS_BLK_DYNAMIC,	/* 0 */
	FCBS_BLK_ROM	/* 1 */
} fcbs_blk_type_t;

typedef struct fcbs_block {
	fcbs_blk_type_t type;
	union {
		int rom_seq_num;
		fcbs_input_data_t *fid;
	} data;
} fcbs_seq_t;

typedef struct fcbs_block_info {
	fcbs_seq_t *seq;
	int blk_sz;
	int blk_addr;
	uint8 halt_on_err;
	uint8 ds_inx;
	uint8 core;
} fcbs_blk_info_t;

typedef struct fcbs_shmdefs {
	const shmdefs_t *shmdefs;
} fcbs_shmdefs_t;

/* private data */
struct fcbs_info {
	osl_t        *osh;	/* OS handle */
	si_t         *sih;	/* SB handle */
	d11regs_info_t *reginfo;	/* d11 regs  */

	uint32       cmdptr_curr_offset[FCBS_DS_MAX];
	uint32       datptr_curr_offset[FCBS_DS_MAX];

	uint32       bm_usage;
	uint8 core;
};

#ifdef BCMULP
/* === Chip spedicific data: For Chip:43012 Rev: A0 [START] === */
/* M_FCBS_DS1_MAC_INIT_BLOCK */
static fcbs_seq_t fcbs_ds1_mac_init_blk[] = {
#if FCBS_DS1_MAC_INIT_BLOCK_ENABLE
	FCBS_ROM_SEQ(FCBSROM_ULP_INITVALS),		/* 0 ULP initvals - ordered */
	FCBS_ROM_SEQ(FCBSROM_ULP_BSINITVALS),	/* 1 ULP bsinitvals - ordered */
	FCBS_DYN_SEQ,				/* 2 AMT */
#endif /* FCBS_DS1_MAC_INIT_BLOCK_ENABLE */
};

/* M_FCBS_DS1_PHY_RADIO_BLOCK */
static fcbs_seq_t fcbs_ds1_phy_radio_blk[] = {
#if FCBS_DS1_PHY_RADIO_BLOCK_ENABLE
	FCBS_ROM_SEQ(FUNC_PHY_RESET),	/* 0 */
	FCBS_ROM_SEQ(FUNC_TBL_SETUP),	/* 1 */
	FCBS_ROM_SEQ(INIT_PHY_REGS),	/* 2 */
	FCBS_ROM_SEQ(INIT_PHY_TBLS),	/* 3 */
	FCBS_ROM_SEQ(FUNC_NAPPING),		/* 4 */
	FCBS_DYN_SEQ,			/* 5 upd CHANSPEC_PHY_RADIO if array idx changes */
	FCBS_DYN_SEQ,			/* 6 upd CALCACHE_PHY_RADIO if array idx changes */
	FCBS_ROM_SEQ(FUNC_TBL_CLEANUP),	/* 7 */
	FCBS_ROM_SEQ(FUNC_RETURN_FROM_DEAF),	/* 8 */
	FCBS_DYN_SEQ,			/* 9 */
	FCBS_DYN_SEQ,			/* 10 */
	FCBS_DYN_SEQ			/* 11 */
#endif /* FCBS_DS1_PHY_RADIO_BLOCK_ENABLE */
};

/* M_FCBS_DS1_RADIO_PD_BLOCK */
static fcbs_seq_t fcbs_ds1_radio_pd_blk[] = {
#if FCBS_DS1_RADIO_PD_BLOCK_ENABLE
	FCBS_ROM_SEQ(PWR_DN_RADIO)
#endif /* FCBS_DS1_RADIO_PD_BLOCK_ENABLE */
};

/* M_FCBS_DS1_EXIT_BLOCK */
static fcbs_seq_t fcbs_ds1_exit_blk[] = {
#if FCBS_DS1_EXIT_BLOCK_ENABLE
	FCBS_DYN_SEQ,		/* 0 SDIO regs */
#endif /* FCBS_DS1_EXIT_BLOCK_ENABLE */
};
#endif /* BCMULP */

/* M_FCBS_DS0_RADIO_PU_BLOCK for D11 core 0 */
static fcbs_seq_t fcbs_ds0_radio_pu_blk_core0[] = {
#if FCBS_DS0_RADIO_PU_BLOCK_ENABLE
	FCBS_DYN_SEQ,	/* 0 Minipmu PU */
	FCBS_DYN_SEQ,	/* 1 PLL, LOGEN PU and VCO Cal */
	FCBS_DYN_SEQ	/* 2 Chan tune */
#endif /* */
};

/* M_FCBS_DS0_RADIO_PD_BLOCK for D11 core 0 */
static fcbs_seq_t fcbs_ds0_radio_pd_blk_core0[] = {
#if FCBS_DS0_RADIO_PD_BLOCK_ENABLE
#ifdef USE_FCBS_ROM
	FCBS_ROM_SEQ(PWR_DN_RADIO)
#else
	FCBS_DYN_SEQ  /* 0 Radio PD */
#endif /* USE_FCBS_ROM */
#endif /* FCBS_DS0_RADIO_PD_BLOCK_ENABLE */
};

/* M_FCBS_DS0_RADIO_PU_BLOCK for D11 core 1 */
static fcbs_seq_t fcbs_ds0_radio_pu_blk_core1[] = {
#if FCBS_DS0_RADIO_PU_BLOCK_ENABLE
	FCBS_DYN_SEQ,	/* 0 Minipmu PU */
	FCBS_DYN_SEQ,	/* 1 PLL, LOGEN PU and VCO Cal */
	FCBS_DYN_SEQ	/* 2 Chan tune */
#endif /* */
};

/* M_FCBS_DS0_RADIO_PD_BLOCK for D11 core 1 */
static fcbs_seq_t fcbs_ds0_radio_pd_blk_core1[] = {
#if FCBS_DS0_RADIO_PD_BLOCK_ENABLE
#ifdef USE_FCBS_ROM
	FCBS_ROM_SEQ(PWR_DN_RADIO)
#else
	FCBS_DYN_SEQ,			/* Radio PD Dynamic sequence 0 */
#endif /* USE_FCBS_ROM */
#endif /* FCBS_DS0_RADIO_PD_BLOCK_ENABLE */
};

enum {
	FCBS_DS0_BLK_RADIO_PD_CORE0 = 0,
	FCBS_DS0_BLK_RADIO_PU_CORE0 = 1,
	FCBS_DS0_BLK_RADIO_PD_CORE1 = 2,
	FCBS_DS0_BLK_RADIO_PU_CORE1 = 3,
#ifdef BCMULP
	FCBS_DS1_BLK_MAC_INIT = 4,
	FCBS_DS1_BLK_PHY_RADIO = 5,
	FCBS_DS1_BLK_RADIO_PD = 6,
	FCBS_DS1_BLK_DS1_EXIT = 7,
#endif /* BCMULP */
	};

static fcbs_blk_info_t _blocks[] = {
	FCBS_DS0_BLK(fcbs_ds0_radio_pd_blk_core0, HALTONERR, FCBS_MAIN_CORE),
	FCBS_DS0_BLK(fcbs_ds0_radio_pu_blk_core0, HALTONERR, FCBS_MAIN_CORE),
	FCBS_DS0_BLK(fcbs_ds0_radio_pd_blk_core1, HALTONERR, FCBS_AUXI_CORE),
	FCBS_DS0_BLK(fcbs_ds0_radio_pu_blk_core1, HALTONERR, FCBS_AUXI_CORE),
#ifdef BCMULP
	FCBS_DS1_BLK(fcbs_ds1_mac_init_blk, HALTONERR),
	FCBS_DS1_BLK(fcbs_ds1_phy_radio_blk, HALTONERR),
	FCBS_DS1_BLK(fcbs_ds1_radio_pd_blk, HALTONERR),
	FCBS_DS1_BLK(fcbs_ds1_exit_blk, NO_HALTONERR),
#endif /* BCMULP */
};

/* === Chip spedicific data: For Chip:43012 Rev: A0 [END] === */
static fcbs_blk_info_t*
BCMRAMFN(get_fcbs_blocks)(uint *size)
{
	*size = ARRAYSIZE(_blocks);
	return _blocks;
}

#ifdef USE_FCBS_ROM
static uchar*
BCMRAMFN(get_fcbs_metadata)(void)
{
	return fcbs_metadata;
}

static uchar*
BCMRAMFN(get_fcbs_ram_data)(void)
{
	return fcbs_ram_data;
}

void
fcbs_update_rom_seq(int blk_num, int seq_num, int rom_seq_num)
{
	uint size;
	fcbs_blk_info_t *blocks = get_fcbs_blocks(&size);

	/* Skip the blocks which are disabled or blk_sz = 0 */
	if (blocks[blk_num].blk_sz) {
		blocks[blk_num].seq[seq_num].data.rom_seq_num = rom_seq_num;
	}
}

static fcbs_out_t
copy_tuples_to_ram(fcbs_info_t* fi, fcbs_stage_md_t fsmd, int ds_inx)
{
	fcbs_input_data_t fid;
	fcbs_tuples_t *ft;
	fcbs_out_t fo = {0, 0};
	uchar *_fcbs_ram_data = get_fcbs_ram_data();
	ft = (fcbs_tuples_t *) MALLOCZ(fi->osh, sizeof(*ft));
	if (!ft) {
		FCBS_DBG(("%s: failed to allocate memory for ft\n", __func__));
		goto ret;
	}

	ft->cmd_ptr = (uint16 *) &_fcbs_ram_data[fsmd.ram_cmd_ptr];
	ft->dat_ptr = (uint16 *) &_fcbs_ram_data[fsmd.ram_dat_ptr];
	ft->cmd_size = fsmd.ram_cmd_sz;
	ft->dat_size = fsmd.ram_dat_sz;
	ft->cmd_buf_size = 0;
	ft->dat_buf_size = 0;

	fid.data_size = 0;
	fid.flags = FCBS_DATA_TUPLE;
	fid.data = ft;

	fo = fcbs_populate(fi, &fid, 1, ds_inx);
ret:
	return fo;
}
#endif /* USE_FCBS_ROM */

void
fcbs_add_dynamic_seq(fcbs_input_data_t *fid, int blk_num, int seq_num)
{
	uint size;
	fcbs_blk_info_t *blocks = get_fcbs_blocks(&size);

	FCBS_DBG(("%s: blk_num: %d, seq_num: %d\n",
			__func__, blk_num, seq_num));
	/* Skip the blocks which are disabled or blk_sz = 0 */
	if (blocks[blk_num].blk_sz) {
		blocks[blk_num].seq[seq_num].data.fid = fid;
	}
}

static void
copy_seqdata_to_shm(fcbs_info_t *fi, int blk_addr, uint32 cmd_start, uint32 dat_start,
	uint8 halt_on_err)
{
	uint16 ctrl_word, val;

	ctrl_word =
		((cmd_start >> (FCBS_CMD_PTR_LSB_SZ + FCBS_CMD_PTR_MSB_SHIFT)) &
		FCBS_CMD_PTR_MSB_MASK) << FCBS_CMD_PTR_MSB_SHIFT;
	ctrl_word |=
		(((dat_start >> (FCBS_DAT_PTR_LSB_SZ + FCBS_DAT_PTR_MSB_SHIFT)) &
		FCBS_DAT_PTR_MSB_MASK) << FCBS_DAT_PTR_MSB_SHIFT) |
		(FCBS_CTRL_PHYBACKTOBACKEN_MASK << FCBS_CTRL_PHYBACKTOBACKEN_SHIFT);

	if (halt_on_err) {
		ctrl_word |= (FCBS_CTRL_HALTONERR_MASK << FCBS_CTRL_HALTONERR_SHIFT);
	}

	/* FCBS Sequence - Command Pointer */
	val = (uint16) (cmd_start & SHM_ENTRY_MASK);
	hndd11_copyto_shm(fi->sih, fi->core,
			(blk_addr + SHM_FCBS_SEQ_CMD_PTR_INX * SHM_ENTRY_SIZE),
			&val, SHM_ENTRY_SIZE);

	/* FCBS Sequence - Data Pointer */
	val = (uint16) (dat_start & SHM_ENTRY_MASK);
	hndd11_copyto_shm(fi->sih, fi->core,
			(blk_addr + SHM_FCBS_SEQ_DAT_PTR_INX * SHM_ENTRY_SIZE),
			&val, SHM_ENTRY_SIZE);

	/* FCBS Sequence - Control Word */
	val = (uint16) (ctrl_word & SHM_ENTRY_MASK);
	hndd11_copyto_shm(fi->sih, fi->core,
			(blk_addr + SHM_FCBS_SEQ_CTL_WRD_INX * SHM_ENTRY_SIZE),
			&val, SHM_ENTRY_SIZE);

	/* FCBS Sequence - Wait Time */
	val = (uint16) (FCBS_MAX_WAIT_TIME & SHM_ENTRY_MASK);
	hndd11_copyto_shm(fi->sih, fi->core,
			(blk_addr + SHM_FCBS_SEQ_WT_TIME_INX * SHM_ENTRY_SIZE),
			&val, 2);

	/* FCBS Sequence - Control Status */
	val = (uint16) (FCBS_CTRL_STATUS_MAGIC & SHM_ENTRY_MASK);
	hndd11_copyto_shm(fi->sih, fi->core,
			(blk_addr + SHM_FCBS_SEQ_CTRL_ST_INX * SHM_ENTRY_SIZE),
			&val, 2);

	/* FCBS Sequence - Actual Completion Time */
	val = (uint16) (FCBS_ACT_CMPL_TIME_MAGIC & SHM_ENTRY_MASK);
	hndd11_copyto_shm(fi->sih, fi->core,
			(blk_addr + SHM_FCBS_SEQ_ACT_TM_INX * SHM_ENTRY_SIZE),
			&val, 2);
}

#ifdef USE_FCBS_ROM
/*
 * FCBS ROM start address = BM base address + BM size
 * FCBS ROM start address = 0xE8400000 + 96K
 */
#define FCBS_ROM_BASE (0xE8400000 + 96 * 1024)

#define ROMSEQ(offset) ((FCBS_ROM_BASE + offset) >> 2)

static int
process_fcbs_rom_block(fcbs_info_t *fi, osl_t *osh, fcbs_stage_md_t *md,
		int blk_num, int seq_num, int rom_seq_num, uint16 *blk_addr, int ds_inx)
{
	uint sz, valid_seq = 1;
	fcbs_blk_info_t *blocks = get_fcbs_blocks(&sz);
	fcbs_out_t fo = {0, 0};

	FCBS_DBG(("%s: blk_num: %d, seq_num: %d, rom_seq_num: %d\n",
			__func__, blk_num, seq_num, rom_seq_num));

	if (rom_seq_num == FCBS_ROM_SEQ_DISABLE) {
		valid_seq = 0;
		FCBS_DBG(("FCBS ROM sequence is disabled!\n"));
		goto exit;
	}

	if (!(IS_WORD_ALIGN(md[rom_seq_num].rom_cmd_ptr))) {
		FCBS_DBG(("%s: rom_cmd_ptr not word aligned \n", __func__));
	}

	if (!(IS_WORD_ALIGN(md[rom_seq_num].rom_dat_ptr))) {
		FCBS_DBG(("%s: rom_dat_ptr not word aligned \n", __func__));
	}

	/* Copy FCBS ROM cmd and dat ptr to SHM */
	if (md[rom_seq_num].rom_cmd_ptr || md[rom_seq_num].rom_dat_ptr) {
		FCBS_DBG(("%s: FCBS ROM seq: cmd: 0x%08x, dat: 0x%08x\n",
				__func__,
				ROMSEQ(md[rom_seq_num].rom_cmd_ptr),
				ROMSEQ(md[rom_seq_num].rom_dat_ptr)));
		copy_seqdata_to_shm(fi, *blk_addr,
				ROMSEQ(md[rom_seq_num].rom_cmd_ptr),
				ROMSEQ(md[rom_seq_num].rom_dat_ptr), blocks[blk_num].halt_on_err);
		*blk_addr += FCBS_SHM_SEQ_SZ;
	}

	/* Copy FCBS RAM data to BM */
	if (md[rom_seq_num].ram_cmd_sz) {
		fo = copy_tuples_to_ram(fi, md[rom_seq_num], ds_inx);
	}

	FCBS_DBG(("(rom blk) RAM cmd and dat ptr: 0x%08x, 0x%08x\n",
			fo.cmd_start, fo.dat_start));

	/* Copy FCBS RAM cmd and dat ptr to SHM */
	if (fo.cmd_start || fo.dat_start) {
		copy_seqdata_to_shm(fi, *blk_addr, fo.cmd_start, fo.dat_start,
				blocks[blk_num].halt_on_err);
		*blk_addr += FCBS_SHM_SEQ_SZ;
	}

exit:
	return valid_seq;
}
#endif /* USE_FCBS_ROM */

static int
process_fcbs_dyn_data(fcbs_info_t *fi, int blk_num, int seq_num,
	fcbs_input_data_t *fid, uint16 *blk_addr, int ds_inx)
{
	uint i, sz;
	int seq_size = 0, valid_seq = 1;
	fcbs_out_t fo;
	fcbs_blk_info_t *blocks = get_fcbs_blocks(&sz);

	FCBS_DBG(("%s: blk_num: %d, seq_num: %d\n", __func__,
			blk_num, seq_num));

	if (!fid) {
		FCBS_DBG(("Unused dynamic sequence!\n"));
		valid_seq = 0;
		goto exit;
	}

	if (fid == (fcbs_input_data_t *)FCBS_DYN_SEQ_SKIP) {
		*blk_addr += FCBS_SHM_SEQ_SZ;
		valid_seq = 1;
		goto exit;
	}

	/* Calculate the number of sub-sequences in a sequence */
	for (i = 0; i < FCBS_MAX_ROM_SUB_SEQ; i++) {
		FCBS_DBG(("%s: fid[i].type: %d\n", __func__, fid[i].type));

		if (fid[i].type == FCBS_TYPE_MAX) {
			break;
		}
		seq_size++;
	}
	FCBS_DBG(("%s: Seq size: %d\n", __func__, seq_size));

	fo = fcbs_populate(fi, fid, seq_size, ds_inx);
	FCBS_DBG(("\n(dynamic) RAM cmd and dat ptr: 0x%08x, 0x%08x\n",
			fo.cmd_start, fo.dat_start));

	/* Copy FCBS RAM cmd and dat ptr to SHM */
	if (fo.cmd_start || fo.dat_start) {
		copy_seqdata_to_shm(fi, *blk_addr, fo.cmd_start, fo.dat_start,
				blocks[blk_num].halt_on_err);
		*blk_addr += FCBS_SHM_SEQ_SZ;
	}
exit:
	return valid_seq;
}

static void
fcbs_shm_init(fcbs_info_t* fi, int ds_inx, uint16 *drv_ucode_if_blk_addr)
{
	int d11rev;
	uint size;
	uint origidx = 0;
	fcbs_shmdefs_t _ufs;
	fcbs_shmdefs_t *ufs = &_ufs;
	fcbs_blk_info_t *blocks = get_fcbs_blocks(&size);

	origidx = si_coreidx(fi->sih);
	si_setcore(fi->sih, D11_CORE_ID, 0);
	d11rev = si_corerev(fi->sih);

	if ((ds_inx == FCBS_DS0) && (fi->core == FCBS_MAIN_CORE)) {
		d11shm_select_ucode_std(&ufs->shmdefs, d11rev);
		blocks[FCBS_DS0_BLK_RADIO_PU_CORE0].blk_addr =
			M_FCBS_DS0_RADIO_PU_BLOCK_OFFSET(ufs);
		blocks[FCBS_DS0_BLK_RADIO_PD_CORE0].blk_addr =
			M_FCBS_DS0_RADIO_PD_BLOCK_OFFSET(ufs);
	}
	else if ((ds_inx == FCBS_DS0) && (fi->core == FCBS_AUXI_CORE)) {
		d11shm_select_ucode_std(&ufs->shmdefs, d11rev);
		blocks[FCBS_DS0_BLK_RADIO_PU_CORE1].blk_addr =
			M_FCBS_DS0_RADIO_PU_BLOCK_OFFSET(ufs);
		blocks[FCBS_DS0_BLK_RADIO_PD_CORE1].blk_addr =
			M_FCBS_DS0_RADIO_PD_BLOCK_OFFSET(ufs);
	}
#ifdef BCMULP
	else {
		d11shm_select_ucode_ulp(&ufs->shmdefs, d11rev);
		blocks[FCBS_DS1_BLK_MAC_INIT].blk_addr = M_FCBS_DS1_MAC_INIT_BLOCK_OFFSET(ufs);
		blocks[FCBS_DS1_BLK_PHY_RADIO].blk_addr = M_FCBS_DS1_PHY_RADIO_BLOCK_OFFSET(ufs);
		blocks[FCBS_DS1_BLK_RADIO_PD].blk_addr = M_FCBS_DS1_RADIO_PD_BLOCK_OFFSET(ufs);
		blocks[FCBS_DS1_BLK_DS1_EXIT].blk_addr = M_FCBS_DS1_EXIT_BLOCK_OFFSET(ufs);
	}
#endif // endif
	/* Reading driver-ucode interface block addr in shm */
	hndd11_read_shm(fi->sih, fi->core, M_DRVR_UCODE_IF_PTR(ufs), drv_ucode_if_blk_addr);
	FCBS_DBG(("[FCBS] driver-ucode interface block addr: 0x%08x\n",
			*drv_ucode_if_blk_addr));

	si_setcoreidx(fi->sih, origidx);
}

#ifdef USE_FCBS_ROM
/* If the FCBS sequence is in FCBS ROM then cmd_ptr and dat_ptr points to the addresses in
 * FCBS ROM. In case, if the sequence got abandoned, then cmd_ptr and dat_ptr points to
 * the addresses of the abandoned data present in ARM RAM.
 */
int
fcbs_rom_metadata(int rom_seq_num, fcbs_tuples_t *ft)
{
	uint ret = -1;
	uchar *_fcbs_ram_data = get_fcbs_ram_data();
	uint *rom_seq_count = (uint *) get_fcbs_metadata();
	fcbs_stage_md_t *md = (fcbs_stage_md_t *)(rom_seq_count + 1);

	if (rom_seq_num >= *rom_seq_count) {
		FCBS_ERR(("Sequence number doesn't exist!\n"));
		goto exit;
	}

	if (md[rom_seq_num].rom_cmd_ptr || md[rom_seq_num].rom_dat_ptr) {
		ft->cmd_ptr = (uint16 *) md[rom_seq_num].rom_cmd_ptr;
		ft->dat_ptr = (uint16 *) md[rom_seq_num].rom_dat_ptr;
		ret = FCBS_SEQ_IN_ROM;
	} else if (md[rom_seq_num].ram_cmd_sz || md[rom_seq_num].ram_dat_sz) {
		ft->cmd_ptr = (uint16 *) &_fcbs_ram_data[md[rom_seq_num].ram_cmd_ptr];
		ft->dat_ptr = (uint16 *) &_fcbs_ram_data[md[rom_seq_num].ram_dat_ptr];
		ft->cmd_size = md[rom_seq_num].ram_cmd_sz;
		ft->dat_size = md[rom_seq_num].ram_dat_sz;
		ret = FCBS_SEQ_IN_RAM;
	}

exit:
	return ret;
}
#endif /* USE_FCBS_ROM */

int
fcbs_init(fcbs_info_t* fi, int ds_inx)
{
	uint i, j, size;
	int err = BCME_OK;
	uint16 blk_sz, blk_sz_shm, blk_addr, drv_ucode_if_blk_addr;
	fcbs_seq_t seq;
	fcbs_blk_info_t *blocks = get_fcbs_blocks(&size);
#ifdef USE_FCBS_ROM
	uint *rom_seq_count = (uint *) get_fcbs_metadata();

	fcbs_stage_md_t *md = (fcbs_stage_md_t *)(rom_seq_count + 1);
#endif /* USE_FCBS_ROM */

	if (fi == NULL) {
		FCBS_DBG(("fcbs is not initialized\n"));
		err = BCME_NOTREADY;
		goto exit;
	}

	fcbs_shm_init(fi, ds_inx, &drv_ucode_if_blk_addr);

#ifdef USE_FCBS_ROM
	FCBS_DBG(("Number of FCBS ROM sequences: %d\n", *rom_seq_count));

	for (i = 0; i < *rom_seq_count; i++) {
		FCBS_DBG(("FCBS ROM Seq - %02d: ", i));
		FCBS_DBG(("rom_cmd_ptr: %04x, rom_dat_ptr: %04x ",
				md[i].rom_cmd_ptr, md[i].rom_dat_ptr));
		FCBS_DBG(("ram_cmd_ptr: %04x ram_cmd_sz: %04x ",
				md[i].ram_cmd_ptr, md[i].ram_cmd_sz));
		FCBS_DBG(("ram_dat_ptr: %04x ram_dat_sz: %04x\n",
				md[i].ram_dat_ptr, md[i].ram_dat_sz));
	}
#endif /* USE_FCBS_ROM */

	for (i = 0; i < size; i++) {
		if (blocks[i].ds_inx != ds_inx) {
			continue;
		}

		if (blocks[i].core != fi->core) {
			continue;
		}

		blk_addr = (drv_ucode_if_blk_addr * SHM_ENTRY_SIZE) +
			blocks[i].blk_addr;

		FCBS_DBG(("===============================================\n"));
		FCBS_DBG(("\n %s: blk[%d]: blk_addr (offset): 0x%08x\n",
				__func__, i, blocks[i].blk_addr));
		FCBS_DBG(("\n %s: blk[%d]: blk_addr (absolute): 0x%08x\n",
				__func__, i, blk_addr));

		/* Populate number of valid sequences */
		blk_sz = 0;
		blk_sz_shm = blk_addr;
		blk_addr += SHM_ENTRY_SIZE;

		for (j = 0; j < blocks[i].blk_sz; j++) {
			FCBS_DBG(("\n FCBS_BLOCK %d %d\n", i, j));
			seq = blocks[i].seq[j];

			FCBS_DBG(("\n %s: blk[%d] seq[%d]: blk_addr (absolute): 0x%08x\n",
					__func__, i, j, blk_addr));
			switch (seq.type) {
			case FCBS_BLK_DYNAMIC:
				blk_sz += process_fcbs_dyn_data(fi, i, j,
						seq.data.fid, &blk_addr, ds_inx);
				break;
			case FCBS_BLK_ROM:
#ifdef USE_FCBS_ROM
				blk_sz += process_fcbs_rom_block(fi, osh, md, i, j,
						seq.data.rom_seq_num, &blk_addr, ds_inx);
#else
				FCBS_ERR(("FCBS ROM seq shouldn't used!"));
				ROMMABLE_ASSERT(0);
#endif /* USE_FCBS_ROM */
				break;
			default:
				FCBS_DBG(("Invalid fcbs blk type!"));
				err = BCME_BADOPTION;
			}
		}
		hndd11_copyto_shm(fi->sih, fi->core, blk_sz_shm, &blk_sz, SHM_ENTRY_SIZE);
	}

exit:
	return err;
}

int
fcbs_reset_cmd_dat_ptrs(void* ctx, int ds_inx, uint32 cmd_ptr, uint32 data_ptr)
{
	fcbs_info_t *fi = (fcbs_info_t *)ctx;
	int err = BCME_OK;

	if ((ds_inx >= FCBS_DS_MAX) || ((fi == NULL) || (fi->core >= MAX_FCBS_CORES))) {
		FCBS_ERR(("%s: Wrong DS index! %d\n", __FUNCTION__, ds_inx));
		err = BCME_BADOPTION;
	} else {
		fi->cmdptr_curr_offset[ds_inx] = cmd_ptr;
		fi->datptr_curr_offset[ds_inx] = data_ptr;
	}

	fi->bm_usage = 0;
	return err;
}

/* attach & detach */
fcbs_info_t *
BCMATTACHFN(fcbs_attach)(osl_t *osh, si_t *sih, d11regs_t *regs, uint8 core)
{
	fcbs_info_t *fi;
	uint32 ds0_cmdptr_base[MAX_FCBS_CORES] =
		{FCBS_DS0_BM_CMDPTR_BASE_CORE0, FCBS_DS0_BM_CMDPTR_BASE_CORE1};
	uint32 ds0_datptr_base[MAX_FCBS_CORES] =
		{FCBS_DS0_BM_DATPTR_BASE_CORE0, FCBS_DS0_BM_DATPTR_BASE_CORE1};
#ifdef BCMULP
	uint32 ds1_cmdptr_base[MAX_D11_CORES] =
		{FCBS_DS1_BM_CMDPTR_BASE_CORE0, FCBS_DS1_BM_CMDPTR_BASE_CORE1};
	uint32 ds1_datptr_base[MAX_D11_CORES] =
		{FCBS_DS1_BM_DATPTR_BASE_CORE0, FCBS_DS1_BM_DATPTR_BASE_CORE1};
#endif /* BCMULP */

	if ((fi = MALLOCZ(osh, sizeof(fcbs_info_t) + sizeof(d11regs_info_t))) == NULL) {
		FCBS_ERR(("%s: out of memory, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(osh)));
		goto fail;
	}
	fi->osh = osh;
	fi->sih = sih;
	fi->core = core;
	fi->reginfo = (d11regs_info_t *)(fi + 1);

	if (BCME_OK != hndd11_get_reginfo(sih, fi->reginfo, 0)) {
		goto fail;
	}

	/* reset cmd and dat ptr offsets */
	if ((BCME_OK != fcbs_reset_cmd_dat_ptrs(fi, FCBS_DS0, ds0_cmdptr_base[fi->core],
		ds0_datptr_base[fi->core]))) {
		FCBS_ERR(("%s: Failed to reset FCBS cmd/data ptrs\n", __FUNCTION__));
		goto fail;
	}

#ifdef BCMULP
	if ((BCME_OK != fcbs_reset_cmd_dat_ptrs(fi, FCBS_DS1, ds1_cmdptr_base[fi->core],
		ds1_datptr_base[fi->core]))) {
		FCBS_ERR(("%s: Failed to reset FCBS cmd/data ptrs\n", __FUNCTION__));
		goto fail;
	}
#endif /* BCMULP */

	return fi;
fail:
	fcbs_detach(fi);
	return NULL;
}

void
BCMATTACHFN(fcbs_detach)(fcbs_info_t *fi)
{
	if (fi) {
		MFREE(fi->osh, fi, sizeof(fcbs_info_t));
	}
}

static uint32
fcbs_get_tuple_size(fcbs_tuples_t *ft[], int num_tuples, fcbs_tpl_type_t tpl_type)
{
	int i = 0, sz = 0;

	for (i = 0; i < num_tuples; i++) {
		if (tpl_type == CMD_TUPLE)
			sz += ft[i]->cmd_size;
		else
			sz += ft[i]->dat_size;
	}

	/* Head room for appending NULL cmd */
	if (tpl_type == CMD_TUPLE)
		sz += 1;

	/* return size as uint32 type */
	return ((sz >> 1) + (sz % 2));
}

static void
fcbs_fill_buffer(fcbs_tuples_t *ft[], int num_tuples, bool select, uint32 *buf)
{
	int i = 0, len = 0;
	uint16 *ptr = NULL;

	for (i = 0; i < num_tuples; i++) {
		if (select == CMD_TUPLE) {
			len = ft[i]->cmd_size;
			ptr = ft[i]->cmd_ptr;
		} else {
			len = ft[i]->dat_size;
			ptr = ft[i]->dat_ptr;
		}

		memcpy(buf, ptr, len * FCBS_TUPLE_NUM_BYTES);
		buf = (uint32 *)(((uint8 *)buf) + len * FCBS_TUPLE_NUM_BYTES);
	}

	/* Stage Ends: append NULL command */
	if (select == CMD_TUPLE) {
		*((uint16 *) buf) = FCBS_NULL;
	}
}

static void
fcbs_transfer_tuples(fcbs_info_t *fi, fcbs_tuples_t *ft[], int num_tuples, int ds_inx)
{
	uint32 sz = 0, dump_addr = 0;
	uint32 *buf = NULL;

	FCBS_DBG(("---- %s ----\n", __FUNCTION__));

	/* transfer cmd tuples to bm */
	sz = fcbs_get_tuple_size(ft, num_tuples, CMD_TUPLE);
	buf = (uint32 *)MALLOCZ(fi->osh, sizeof(uint32) * sz);
	if (!buf) {
		FCBS_ERR(("(cmd) Mem alloc failed\n"));
		goto exit;
	}

	fcbs_fill_buffer(ft, num_tuples, CMD_TUPLE, buf);
	dump_addr = fi->cmdptr_curr_offset[ds_inx];
	fi->cmdptr_curr_offset[ds_inx] = hndd11_bm_write(fi->osh, fi->reginfo,
			fi->cmdptr_curr_offset[ds_inx], sz, buf);

	FCBS_DBG((" CMD tuple size : %d bytes\n", sz * 4));
	fi->bm_usage += sz;
	hndd11_bm_dump(fi->osh, fi->reginfo, dump_addr, sz);

	MFREE(fi->osh, buf, sizeof(uint32) * sz); buf = NULL;

	/* transfer data tuples to bm */
	sz = fcbs_get_tuple_size(ft, num_tuples, DAT_TUPLE);
	buf = (uint32 *)MALLOCZ(fi->osh, sizeof(uint32) * sz);
	if (!buf) {
		FCBS_ERR(("(dat) Mem alloc failed\n"));
		goto exit;
	}

	fcbs_fill_buffer(ft, num_tuples, DAT_TUPLE, buf);
	dump_addr = fi->datptr_curr_offset[ds_inx];
	fi->datptr_curr_offset[ds_inx] = hndd11_bm_write(fi->osh, fi->reginfo,
			fi->datptr_curr_offset[ds_inx],	sz, buf);

	FCBS_DBG(("DATA tuple size : %d bytes\n", sz * 4));
	fi->bm_usage += sz;
	hndd11_bm_dump(fi->osh, fi->reginfo, dump_addr, sz);

	MFREE(fi->osh, buf, sizeof(uint32) * sz); buf = NULL;

	FCBS_DBG(("Total BM used %d bytes\n", fi->bm_usage * 4));

exit:
	return;

}

fcbs_out_t
fcbs_populate(fcbs_info_t* fi, fcbs_input_data_t *input, int num_tuples, int ds_inx)
{
	int i = 0;

	fcbs_out_t ret = {0, 0};
	fcbs_tuples_t **tuple;
	fcbs_tuples_t **ft = NULL;

	tuple = (fcbs_tuples_t **)MALLOCZ(fi->osh, sizeof(fcbs_tuples_t *) * num_tuples);
	if (!tuple) {
		FCBS_ERR(("%s: Failed to allocate memory for tuples\n", __func__));
		goto exit;
	}

	for (i = 0; i < num_tuples; i++, input++) {
		if (input->flags & FCBS_DATA_TUPLE) {
			tuple[i] = ((fcbs_tuples_t *)(input->data));
		} else {
			tuple[i] = fcbs_create_tuples(fi->osh, input);
		}
		fcbs_print_tuples(tuple[i]);
	}

	ft = tuple;

	if (!IS_WORD_ALIGN(fi->cmdptr_curr_offset[ds_inx])) {
		WORD_ALIGN(fi->cmdptr_curr_offset[ds_inx]);
	}

	if (!IS_WORD_ALIGN(fi->datptr_curr_offset[ds_inx])) {
		WORD_ALIGN(fi->datptr_curr_offset[ds_inx]);
	}

	/* start addresses of cmd and data tuples in BM */
	ret.cmd_start = fi->cmdptr_curr_offset[ds_inx] >> 2;
	ret.dat_start = fi->datptr_curr_offset[ds_inx] >> 2;

	fcbs_transfer_tuples(fi, ft, num_tuples, ds_inx);

	for (i = 0; i < num_tuples; i++)
		fcbs_free_tuples(fi->osh, tuple[i]);

	MFREE(fi->osh, tuple, sizeof(fcbs_tuples_t *) * num_tuples);

	FCBS_DBG(("%s: cmd_start = 0x%08x(0x%08x) data_start = 0x%08x(0x%08x)\n\n",
		__FUNCTION__, ret.cmd_start, ret.cmd_start << 2,
		ret.dat_start, ret.dat_start << 2));

exit:
	return ret;
}

#endif /* DONGLEBUILD */
