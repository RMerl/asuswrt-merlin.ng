// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <dm.h>
#include <dm/device-internal.h>
#include <dm/devres.h>
#include <dm/of_access.h>
#include <malloc.h>
#include <memalign.h>
#include <nand.h>
#include <pci.h>
#include <time.h>
#include <linux/bitfield.h>
#include <linux/ctype.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <linux/libfdt.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand_bch.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/rawnand.h>
#include <linux/time.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/types.h>
#include <asm/dma-mapping.h>
#include <asm/arch/clock.h>
#include "octeontx_bch.h"

/*
 * The NDF_CMD queue takes commands between 16 - 128 bit.
 * All commands must be 16 bit aligned and are little endian.
 * WAIT_STATUS commands must be 64 bit aligned.
 * Commands are selected by the 4 bit opcode.
 *
 * Available Commands:
 *
 * 16 Bit:
 *   NOP
 *   WAIT
 *   BUS_ACQ, BUS_REL
 *   CHIP_EN, CHIP_DIS
 *
 * 32 Bit:
 *   CLE_CMD
 *   RD_CMD, RD_EDO_CMD
 *   WR_CMD
 *
 * 64 Bit:
 *   SET_TM_PAR
 *
 * 96 Bit:
 *   ALE_CMD
 *
 * 128 Bit:
 *   WAIT_STATUS, WAIT_STATUS_ALE
 */

/* NDF Register offsets */
#define NDF_CMD			0x0
#define NDF_MISC		0x8
#define NDF_ECC_CNT		0x10
#define NDF_DRBELL		0x30
#define NDF_ST_REG		0x38	/* status */
#define NDF_INT			0x40
#define NDF_INT_W1S		0x48
#define NDF_DMA_CFG		0x50
#define NDF_DMA_ADR		0x58
#define NDF_INT_ENA_W1C		0x60
#define NDF_INT_ENA_W1S		0x68

/* NDF command opcodes */
#define NDF_OP_NOP		0x0
#define NDF_OP_SET_TM_PAR	0x1
#define NDF_OP_WAIT		0x2
#define NDF_OP_CHIP_EN_DIS	0x3
#define NDF_OP_CLE_CMD		0x4
#define NDF_OP_ALE_CMD		0x5
#define NDF_OP_WR_CMD		0x8
#define NDF_OP_RD_CMD		0x9
#define NDF_OP_RD_EDO_CMD	0xa
#define NDF_OP_WAIT_STATUS	0xb	/* same opcode for WAIT_STATUS_ALE */
#define NDF_OP_BUS_ACQ_REL	0xf

#define NDF_BUS_ACQUIRE		1
#define NDF_BUS_RELEASE		0

#define DBGX_EDSCR(X)		(0x87A008000088 + (X) * 0x80000)

struct ndf_nop_cmd {
	u16 opcode:	4;
	u16 nop:	12;
};

struct ndf_wait_cmd {
	u16 opcode:4;
	u16 r_b:1;		/* wait for one cycle or PBUS_WAIT deassert */
	u16:3;
	u16 wlen:3;		/* timing parameter select */
	u16:5;
};

struct ndf_bus_cmd {
	u16 opcode:4;
	u16 direction:4;	/* 1 = acquire, 0 = release */
	u16:8;
};

struct ndf_chip_cmd {
	u16 opcode:4;
	u16 chip:3;		/* select chip, 0 = disable */
	u16 enable:1;		/* 1 = enable, 0 = disable */
	u16 bus_width:2;	/* 10 = 16 bit, 01 = 8 bit */
	u16:6;
};

struct ndf_cle_cmd {
	u32 opcode:4;
	u32:4;
	u32 cmd_data:8;		/* command sent to the PBUS AD pins */
	u32 clen1:3;		/* time between PBUS CLE and WE asserts */
	u32 clen2:3;		/* time WE remains asserted */
	u32 clen3:3;		/* time between WE deassert and CLE */
	u32:7;
};

/* RD_EDO_CMD uses the same layout as RD_CMD */
struct ndf_rd_cmd {
	u32 opcode:4;
	u32 data:16;		/* data bytes */
	u32 rlen1:3;
	u32 rlen2:3;
	u32 rlen3:3;
	u32 rlen4:3;
};

struct ndf_wr_cmd {
	u32 opcode:4;
	u32 data:16;		/* data bytes */
	u32:4;
	u32 wlen1:3;
	u32 wlen2:3;
	u32:3;
};

struct ndf_set_tm_par_cmd {
	u64 opcode:4;
	u64 tim_mult:4;	/* multiplier for the seven parameters */
	u64 tm_par1:8;	/* --> Following are the 7 timing parameters that */
	u64 tm_par2:8;	/*     specify the number of coprocessor cycles.  */
	u64 tm_par3:8;	/*     A value of zero means one cycle.		  */
	u64 tm_par4:8;	/*     All values are scaled by tim_mult	  */
	u64 tm_par5:8;	/*     using tim_par * (2 ^ tim_mult).		  */
	u64 tm_par6:8;
	u64 tm_par7:8;
};

struct ndf_ale_cmd {
	u32 opcode:4;
	u32:4;
	u32 adr_byte_num:4;	/* number of address bytes to be sent */
	u32:4;
	u32 alen1:3;
	u32 alen2:3;
	u32 alen3:3;
	u32 alen4:3;
	u32:4;
	u8 adr_byt1;
	u8 adr_byt2;
	u8 adr_byt3;
	u8 adr_byt4;
	u8 adr_byt5;
	u8 adr_byt6;
	u8 adr_byt7;
	u8 adr_byt8;
};

struct ndf_wait_status_cmd {
	u32 opcode:4;
	u32:4;
	u32 data:8;		/** data */
	u32 clen1:3;
	u32 clen2:3;
	u32 clen3:3;
	u32:8;
	/** set to 5 to select WAIT_STATUS_ALE command */
	u32 ale_ind:8;
	/** ALE only: number of address bytes to be sent */
	u32 adr_byte_num:4;
	u32:4;
	u32 alen1:3;	/* ALE only */
	u32 alen2:3;	/* ALE only */
	u32 alen3:3;	/* ALE only */
	u32 alen4:3;	/* ALE only */
	u32:4;
	u8 adr_byt[4];		/* ALE only */
	u32 nine:4;	/* set to 9 */
	u32 and_mask:8;
	u32 comp_byte:8;
	u32 rlen1:3;
	u32 rlen2:3;
	u32 rlen3:3;
	u32 rlen4:3;
};

union ndf_cmd {
	u64 val[2];
	union {
		struct ndf_nop_cmd		nop;
		struct ndf_wait_cmd		wait;
		struct ndf_bus_cmd		bus_acq_rel;
		struct ndf_chip_cmd		chip_en_dis;
		struct ndf_cle_cmd		cle_cmd;
		struct ndf_rd_cmd		rd_cmd;
		struct ndf_wr_cmd		wr_cmd;
		struct ndf_set_tm_par_cmd	set_tm_par;
		struct ndf_ale_cmd		ale_cmd;
		struct ndf_wait_status_cmd	wait_status;
	} u;
};

/** Disable multi-bit error hangs */
#define NDF_MISC_MB_DIS		BIT_ULL(27)
/** High watermark for NBR FIFO or load/store operations */
#define NDF_MISC_NBR_HWM	GENMASK_ULL(26, 24)
/** Wait input filter count */
#define NDF_MISC_WAIT_CNT	GENMASK_ULL(23, 18)
/** Unfilled NFD_CMD queue bytes */
#define NDF_MISC_FR_BYTE	GENMASK_ULL(17, 7)
/** Set by HW when it reads the last 8 bytes of NDF_CMD */
#define NDF_MISC_RD_DONE	BIT_ULL(6)
/** Set by HW when it reads. SW read of NDF_CMD clears it */
#define NDF_MISC_RD_VAL		BIT_ULL(5)
/** Let HW read NDF_CMD queue. Cleared on SW NDF_CMD write */
#define NDF_MISC_RD_CMD		BIT_ULL(4)
/** Boot disable */
#define NDF_MISC_BT_DIS		BIT_ULL(2)
/** Stop command execution after completing command queue */
#define NDF_MISC_EX_DIS		BIT_ULL(1)
/** Reset fifo */
#define NDF_MISC_RST_FF		BIT_ULL(0)

/** DMA engine enable */
#define NDF_DMA_CFG_EN		BIT_ULL(63)
/** Read or write */
#define NDF_DMA_CFG_RW		BIT_ULL(62)
/** Terminates DMA and clears enable bit */
#define NDF_DMA_CFG_CLR		BIT_ULL(61)
/** 32-bit swap enable */
#define NDF_DMA_CFG_SWAP32	BIT_ULL(59)
/** 16-bit swap enable */
#define NDF_DMA_CFG_SWAP16	BIT_ULL(58)
/** 8-bit swap enable */
#define NDF_DMA_CFG_SWAP8	BIT_ULL(57)
/** Endian mode */
#define NDF_DMA_CFG_CMD_BE	BIT_ULL(56)
/** Number of 64 bit transfers */
#define NDF_DMA_CFG_SIZE	GENMASK_ULL(55, 36)

/** Command execution status idle */
#define NDF_ST_REG_EXE_IDLE	BIT_ULL(15)
/** Command execution SM states */
#define NDF_ST_REG_EXE_SM	GENMASK_ULL(14, 11)
/** DMA and load SM states */
#define NDF_ST_REG_BT_SM	GENMASK_ULL(10, 7)
/** Queue read-back SM bad state */
#define NDF_ST_REG_RD_FF_BAD	BIT_ULL(6)
/** Queue read-back SM states */
#define NDF_ST_REG_RD_FF	GENMASK_ULL(5, 4)
/** Main SM is in a bad state */
#define NDF_ST_REG_MAIN_BAD	BIT_ULL(3)
/** Main SM states */
#define NDF_ST_REG_MAIN_SM	GENMASK_ULL(2, 0)

#define MAX_NAND_NAME_LEN	64
#if (defined(NAND_MAX_PAGESIZE) && (NAND_MAX_PAGESIZE > 4096)) ||	\
	!defined(NAND_MAX_PAGESIZE)
# undef NAND_MAX_PAGESIZE
# define NAND_MAX_PAGESIZE	4096
#endif
#if (defined(NAND_MAX_OOBSIZE) && (NAND_MAX_OOBSIZE > 256)) ||		\
	!defined(NAND_MAX_OOBSIZE)
# undef NAND_MAX_OOBSIZE
# define NAND_MAX_OOBSIZE	256
#endif

#define OCTEONTX_NAND_DRIVER_NAME	"octeontx_nand"

#define NDF_TIMEOUT		1000	/** Timeout in ms */
#ifndef NAND_MAX_CHIPS
# define NAND_MAX_CHIPS		8	/** Linux compatibility */
#endif

struct octeontx_nand_chip {
	struct list_head node;
	struct nand_chip nand;
	struct ndf_set_tm_par_cmd timings;
	int cs;
	int selected_page;
	int iface_mode;
	int row_bytes;
	int col_bytes;
	bool oob_only;
	bool iface_set;
};

struct octeontx_nand_buf {
	u8 *dmabuf;
	dma_addr_t dmaaddr;
	int dmabuflen;
	int data_len;
	int data_index;
};

/** NAND flash controller (NDF) related information */
struct octeontx_nfc {
	struct nand_hw_control controller;
	struct udevice *dev;
	void __iomem *base;
	struct list_head chips;
	int selected_chip;      /* Currently selected NAND chip number */

	/*
	 * Status is separate from octeontx_nand_buf because
	 * it can be used in parallel and during init.
	 */
	u8 *stat;
	dma_addr_t stat_addr;
	bool use_status;

	struct octeontx_nand_buf buf;
	union bch_resp *bch_resp;
	dma_addr_t bch_rhandle;

	/* BCH of all-0xff, so erased pages read as error-free */
	unsigned char *eccmask;
};

/* settable timings - 0..7 select timing of alen1..4/clen1..3/etc */
enum tm_idx {
	t0, /* fixed at 4<<mult cycles */
	t1, t2, t3, t4, t5, t6, t7, /* settable per ONFI-timing mode */
};

struct octeontx_probe_device {
	struct list_head list;
	struct udevice *dev;
};

static struct bch_vf *bch_vf;
/** Deferred devices due to BCH not being ready */
static LIST_HEAD(octeontx_pci_nand_deferred_devices);

/** default parameters used for probing chips */
#define MAX_ONFI_MODE	5

static int default_onfi_timing;
static int slew_ns = 2; /* default timing padding */
static int def_ecc_size = 512; /* 1024 best for sw_bch, <= 4095 for hw_bch */
static int default_width = 1; /* 8 bit */
static int default_page_size = 2048;
static struct ndf_set_tm_par_cmd default_timing_parms;

/** Port from Linux */
#define readq_poll_timeout(addr, val, cond, delay_us, timeout_us)	\
({									\
	ulong __start = get_timer(0);					\
	void *__addr = (addr);						\
	const ulong __timeout_ms = timeout_us / 1000;			\
	do {								\
		(val) = readq(__addr);					\
		if (cond)						\
			break;						\
		if (timeout_us && get_timer(__start) > __timeout_ms) {	\
			(val) = readq(__addr);				\
			break;						\
		}							\
		if (delay_us)						\
			udelay(delay_us);				\
	} while (1);							\
	(cond) ? 0 : -ETIMEDOUT;					\
})

/** Ported from Linux 4.9.0 include/linux/of.h for compatibility */
static inline int of_get_child_count(const ofnode node)
{
	return fdtdec_get_child_count(gd->fdt_blob, ofnode_to_offset(node));
}

/**
 * Linux compatibility from Linux 4.9.0 drivers/mtd/nand/nand_base.c
 */
static int nand_ooblayout_ecc_lp(struct mtd_info *mtd, int section,
				 struct mtd_oob_region *oobregion)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct nand_ecc_ctrl *ecc = &chip->ecc;

	if (section || !ecc->total)
		return -ERANGE;

	oobregion->length = ecc->total;
	oobregion->offset = mtd->oobsize - oobregion->length;

	return 0;
}

/**
 * Linux compatibility from Linux 4.9.0 drivers/mtd/nand/nand_base.c
 */
static int nand_ooblayout_free_lp(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *oobregion)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct nand_ecc_ctrl *ecc = &chip->ecc;

	if (section)
		return -ERANGE;

	oobregion->length = mtd->oobsize - ecc->total - 2;
	oobregion->offset = 2;

	return 0;
}

static const struct mtd_ooblayout_ops nand_ooblayout_lp_ops = {
	.ecc = nand_ooblayout_ecc_lp,
	.rfree = nand_ooblayout_free_lp,
};

static inline struct octeontx_nand_chip *to_otx_nand(struct nand_chip *nand)
{
	return container_of(nand, struct octeontx_nand_chip, nand);
}

static inline struct octeontx_nfc *to_otx_nfc(struct nand_hw_control *ctrl)
{
	return container_of(ctrl, struct octeontx_nfc, controller);
}

static int octeontx_nand_calc_ecc_layout(struct nand_chip *nand)
{
	struct nand_ecclayout *layout = nand->ecc.layout;
	struct octeontx_nfc *tn = to_otx_nfc(nand->controller);
	struct mtd_info *mtd = &nand->mtd;
	int oobsize = mtd->oobsize;
	int i;
	bool layout_alloc = false;

	if (!layout) {
		layout = devm_kzalloc(tn->dev, sizeof(*layout), GFP_KERNEL);
		if (!layout)
			return -ENOMEM;
		nand->ecc.layout = layout;
		layout_alloc = true;
	}
	layout->eccbytes = nand->ecc.steps * nand->ecc.bytes;
	/* Reserve 2 bytes for bad block marker */
	if (layout->eccbytes + 2 > oobsize) {
		pr_err("No suitable oob scheme available for oobsize %d eccbytes %u\n",
		       oobsize, layout->eccbytes);
		goto fail;
	}
	/* put ecc bytes at oob tail */
	for (i = 0; i < layout->eccbytes; i++)
		layout->eccpos[i] = oobsize - layout->eccbytes + i;
	layout->oobfree[0].offset = 2;
	layout->oobfree[0].length = oobsize - 2 - layout->eccbytes;
	nand->ecc.layout = layout;
	return 0;

fail:
	if (layout_alloc)
		kfree(layout);
	return -1;
}

/*
 * Read a single byte from the temporary buffer. Used after READID
 * to get the NAND information and for STATUS.
 */
static u8 octeontx_nand_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct octeontx_nfc *tn = to_otx_nfc(nand->controller);

	if (tn->use_status) {
		tn->use_status = false;
		return *tn->stat;
	}

	if (tn->buf.data_index < tn->buf.data_len)
		return tn->buf.dmabuf[tn->buf.data_index++];

	dev_err(tn->dev, "No data to read, idx: 0x%x, len: 0x%x\n",
		tn->buf.data_index, tn->buf.data_len);

	return 0xff;
}

/*
 * Read a number of pending bytes from the temporary buffer. Used
 * to get page and OOB data.
 */
static void octeontx_nand_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct octeontx_nfc *tn = to_otx_nfc(nand->controller);

	if (len > tn->buf.data_len - tn->buf.data_index) {
		dev_err(tn->dev, "Not enough data for read of %d bytes\n", len);
		return;
	}

	memcpy(buf, tn->buf.dmabuf + tn->buf.data_index, len);
	tn->buf.data_index += len;
}

static void octeontx_nand_write_buf(struct mtd_info *mtd,
				    const u8 *buf, int len)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct octeontx_nfc *tn = to_otx_nfc(nand->controller);

	memcpy(tn->buf.dmabuf + tn->buf.data_len, buf, len);
	tn->buf.data_len += len;
}

/* Overwrite default function to avoid sync abort on chip = -1. */
static void octeontx_nand_select_chip(struct mtd_info *mtd, int chip)
{
}

static inline int timing_to_cycle(u32 psec, unsigned long clock)
{
	unsigned int ns;
	int ticks;

	ns = DIV_ROUND_UP(psec, 1000);
	ns += slew_ns;

	/* no rounding needed since clock is multiple of 1MHz */
	clock /= 1000000;
	ns *= clock;

	ticks = DIV_ROUND_UP(ns, 1000);

	/* actual delay is (tm_parX+1)<<tim_mult */
	if (ticks)
		ticks--;

	return ticks;
}

static void set_timings(struct octeontx_nand_chip *chip,
			struct ndf_set_tm_par_cmd *tp,
			const struct nand_sdr_timings *timings,
			unsigned long sclk)
{
	/* scaled coprocessor-cycle values */
	u32 s_wh, s_cls, s_clh, s_rp, s_wb, s_wc;

	tp->tim_mult = 0;
	s_wh = timing_to_cycle(timings->tWH_min, sclk);
	s_cls = timing_to_cycle(timings->tCLS_min, sclk);
	s_clh = timing_to_cycle(timings->tCLH_min, sclk);
	s_rp = timing_to_cycle(timings->tRP_min, sclk);
	s_wb = timing_to_cycle(timings->tWB_max, sclk);
	s_wc = timing_to_cycle(timings->tWC_min, sclk);

	tp->tm_par1 = s_wh;
	tp->tm_par2 = s_clh;
	tp->tm_par3 = s_rp + 1;
	tp->tm_par4 = s_cls - s_wh;
	tp->tm_par5 = s_wc - s_wh + 1;
	tp->tm_par6 = s_wb;
	tp->tm_par7 = 0;
	tp->tim_mult++; /* overcompensate for bad math */

	/* TODO: comment parameter re-use */

	pr_debug("%s: tim_par: mult: %d  p1: %d  p2: %d  p3: %d\n",
		 __func__, tp->tim_mult, tp->tm_par1, tp->tm_par2, tp->tm_par3);
	pr_debug("                 p4: %d  p5: %d  p6: %d  p7: %d\n",
		 tp->tm_par4, tp->tm_par5, tp->tm_par6, tp->tm_par7);
}

static int set_default_timings(struct octeontx_nfc *tn,
			       const struct nand_sdr_timings *timings)
{
	unsigned long sclk = octeontx_get_io_clock();

	set_timings(NULL, &default_timing_parms, timings, sclk);
	return 0;
}

static int octeontx_nfc_chip_set_timings(struct octeontx_nand_chip *chip,
					 const struct nand_sdr_timings *timings)
{
	/*struct octeontx_nfc *tn = to_otx_nfc(chip->nand.controller);*/
	unsigned long sclk = octeontx_get_io_clock();

	set_timings(chip, &chip->timings, timings, sclk);
	return 0;
}

/* How many bytes are free in the NFD_CMD queue? */
static int ndf_cmd_queue_free(struct octeontx_nfc *tn)
{
	u64 ndf_misc;

	ndf_misc = readq(tn->base + NDF_MISC);
	return FIELD_GET(NDF_MISC_FR_BYTE, ndf_misc);
}

/* Submit a command to the NAND command queue. */
static int ndf_submit(struct octeontx_nfc *tn, union ndf_cmd *cmd)
{
	int opcode = cmd->val[0] & 0xf;

	switch (opcode) {
	/* All these commands fit in one 64bit word */
	case NDF_OP_NOP:
	case NDF_OP_SET_TM_PAR:
	case NDF_OP_WAIT:
	case NDF_OP_CHIP_EN_DIS:
	case NDF_OP_CLE_CMD:
	case NDF_OP_WR_CMD:
	case NDF_OP_RD_CMD:
	case NDF_OP_RD_EDO_CMD:
	case NDF_OP_BUS_ACQ_REL:
		if (ndf_cmd_queue_free(tn) < 8)
			goto full;
		writeq(cmd->val[0], tn->base + NDF_CMD);
		break;
	case NDF_OP_ALE_CMD:
		/* ALE commands take either one or two 64bit words */
		if (cmd->u.ale_cmd.adr_byte_num < 5) {
			if (ndf_cmd_queue_free(tn) < 8)
				goto full;
			writeq(cmd->val[0], tn->base + NDF_CMD);
		} else {
			if (ndf_cmd_queue_free(tn) < 16)
				goto full;
			writeq(cmd->val[0], tn->base + NDF_CMD);
			writeq(cmd->val[1], tn->base + NDF_CMD);
		}
		break;
	case NDF_OP_WAIT_STATUS: /* Wait status commands take two 64bit words */
		if (ndf_cmd_queue_free(tn) < 16)
			goto full;
		writeq(cmd->val[0], tn->base + NDF_CMD);
		writeq(cmd->val[1], tn->base + NDF_CMD);
		break;
	default:
		dev_err(tn->dev, "%s: unknown command: %u\n", __func__, opcode);
		return -EINVAL;
	}
	return 0;

full:
	dev_err(tn->dev, "%s: no space left in command queue\n", __func__);
	return -ENOMEM;
}

/**
 * Wait for the ready/busy signal. First wait for busy to be valid,
 * then wait for busy to de-assert.
 */
static int ndf_build_wait_busy(struct octeontx_nfc *tn)
{
	union ndf_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.u.wait.opcode = NDF_OP_WAIT;
	cmd.u.wait.r_b = 1;
	cmd.u.wait.wlen = t6;

	if (ndf_submit(tn, &cmd))
		return -ENOMEM;
	return 0;
}

static bool ndf_dma_done(struct octeontx_nfc *tn)
{
	u64 dma_cfg;

	/* Enable bit should be clear after a transfer */
	dma_cfg = readq(tn->base + NDF_DMA_CFG);
	if (!(dma_cfg & NDF_DMA_CFG_EN))
		return true;

	return false;
}

static int ndf_wait(struct octeontx_nfc *tn)
{
	ulong start = get_timer(0);
	bool done;

	while (!(done = ndf_dma_done(tn)) && get_timer(start) < NDF_TIMEOUT)
		;

	if (!done) {
		dev_err(tn->dev, "%s: timeout error\n", __func__);
		return -ETIMEDOUT;
	}
	return 0;
}

static int ndf_wait_idle(struct octeontx_nfc *tn)
{
	u64 val;
	u64 dval = 0;
	int rc;
	int pause = 100;
	u64 tot_us = USEC_PER_SEC / 10;

	rc = readq_poll_timeout(tn->base + NDF_ST_REG,
				val, val & NDF_ST_REG_EXE_IDLE, pause, tot_us);
	if (!rc)
		rc = readq_poll_timeout(tn->base + NDF_DMA_CFG,
					dval, !(dval & NDF_DMA_CFG_EN),
					pause, tot_us);

	return rc;
}

/** Issue set timing parameters */
static int ndf_queue_cmd_timing(struct octeontx_nfc *tn,
				struct ndf_set_tm_par_cmd *timings)
{
	union ndf_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.u.set_tm_par.opcode = NDF_OP_SET_TM_PAR;
	cmd.u.set_tm_par.tim_mult = timings->tim_mult;
	cmd.u.set_tm_par.tm_par1 = timings->tm_par1;
	cmd.u.set_tm_par.tm_par2 = timings->tm_par2;
	cmd.u.set_tm_par.tm_par3 = timings->tm_par3;
	cmd.u.set_tm_par.tm_par4 = timings->tm_par4;
	cmd.u.set_tm_par.tm_par5 = timings->tm_par5;
	cmd.u.set_tm_par.tm_par6 = timings->tm_par6;
	cmd.u.set_tm_par.tm_par7 = timings->tm_par7;
	return ndf_submit(tn, &cmd);
}

/** Issue bus acquire or release */
static int ndf_queue_cmd_bus(struct octeontx_nfc *tn, int direction)
{
	union ndf_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.u.bus_acq_rel.opcode = NDF_OP_BUS_ACQ_REL;
	cmd.u.bus_acq_rel.direction = direction;
	return ndf_submit(tn, &cmd);
}

/* Issue chip select or deselect */
static int ndf_queue_cmd_chip(struct octeontx_nfc *tn, int enable, int chip,
			      int width)
{
	union ndf_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.u.chip_en_dis.opcode = NDF_OP_CHIP_EN_DIS;
	cmd.u.chip_en_dis.chip = chip;
	cmd.u.chip_en_dis.enable = enable;
	cmd.u.chip_en_dis.bus_width = width;
	return ndf_submit(tn, &cmd);
}

static int ndf_queue_cmd_wait(struct octeontx_nfc *tn, int t_delay)
{
	union ndf_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.u.wait.opcode = NDF_OP_WAIT;
	cmd.u.wait.wlen = t_delay;
	return ndf_submit(tn, &cmd);
}

static int ndf_queue_cmd_cle(struct octeontx_nfc *tn, int command)
{
	union ndf_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.u.cle_cmd.opcode = NDF_OP_CLE_CMD;
	cmd.u.cle_cmd.cmd_data = command;
	cmd.u.cle_cmd.clen1 = t4;
	cmd.u.cle_cmd.clen2 = t1;
	cmd.u.cle_cmd.clen3 = t2;
	return ndf_submit(tn, &cmd);
}

static int ndf_queue_cmd_ale(struct octeontx_nfc *tn, int addr_bytes,
			     struct nand_chip *nand, u64 page,
			     u32 col, int page_size)
{
	struct octeontx_nand_chip *octeontx_nand = (nand) ?
						to_otx_nand(nand) : NULL;
	union ndf_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.u.ale_cmd.opcode = NDF_OP_ALE_CMD;
	cmd.u.ale_cmd.adr_byte_num = addr_bytes;

	/* set column bit for OOB area, assume OOB follows page */
	if (octeontx_nand && octeontx_nand->oob_only)
		col += page_size;

	/* page is u64 for this generality, even if cmdfunc() passes int */
	switch (addr_bytes) {
	/* 4-8 bytes: page, then 2-byte col */
	case 8:
		cmd.u.ale_cmd.adr_byt8 = (page >> 40) & 0xff;
		fallthrough;
	case 7:
		cmd.u.ale_cmd.adr_byt7 = (page >> 32) & 0xff;
		fallthrough;
	case 6:
		cmd.u.ale_cmd.adr_byt6 = (page >> 24) & 0xff;
		fallthrough;
	case 5:
		cmd.u.ale_cmd.adr_byt5 = (page >> 16) & 0xff;
		fallthrough;
	case 4:
		cmd.u.ale_cmd.adr_byt4 = (page >> 8) & 0xff;
		cmd.u.ale_cmd.adr_byt3 = page & 0xff;
		cmd.u.ale_cmd.adr_byt2 = (col >> 8) & 0xff;
		cmd.u.ale_cmd.adr_byt1 =  col & 0xff;
		break;
	/* 1-3 bytes: just the page address */
	case 3:
		cmd.u.ale_cmd.adr_byt3 = (page >> 16) & 0xff;
		fallthrough;
	case 2:
		cmd.u.ale_cmd.adr_byt2 = (page >> 8) & 0xff;
		fallthrough;
	case 1:
		cmd.u.ale_cmd.adr_byt1 = page & 0xff;
		break;
	default:
		break;
	}

	cmd.u.ale_cmd.alen1 = t3;
	cmd.u.ale_cmd.alen2 = t1;
	cmd.u.ale_cmd.alen3 = t5;
	cmd.u.ale_cmd.alen4 = t2;
	return ndf_submit(tn, &cmd);
}

static int ndf_queue_cmd_write(struct octeontx_nfc *tn, int len)
{
	union ndf_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.u.wr_cmd.opcode = NDF_OP_WR_CMD;
	cmd.u.wr_cmd.data = len;
	cmd.u.wr_cmd.wlen1 = t3;
	cmd.u.wr_cmd.wlen2 = t1;
	return ndf_submit(tn, &cmd);
}

static int ndf_build_pre_cmd(struct octeontx_nfc *tn, int cmd1,
			     int addr_bytes, u64 page, u32 col, int cmd2)
{
	struct nand_chip *nand = tn->controller.active;
	struct octeontx_nand_chip *octeontx_nand;
	struct ndf_set_tm_par_cmd *timings;
	int width, page_size, rc;

	/* Also called before chip probing is finished */
	if (!nand) {
		timings = &default_timing_parms;
		page_size = default_page_size;
		width = default_width;
	} else {
		octeontx_nand = to_otx_nand(nand);
		timings = &octeontx_nand->timings;
		page_size = nand->mtd.writesize;
		if (nand->options & NAND_BUSWIDTH_16)
			width = 2;
		else
			width = 1;
	}
	rc = ndf_queue_cmd_timing(tn, timings);
	if (rc)
		return rc;

	rc = ndf_queue_cmd_bus(tn, NDF_BUS_ACQUIRE);
	if (rc)
		return rc;

	rc = ndf_queue_cmd_chip(tn, 1, tn->selected_chip, width);
	if (rc)
		return rc;

	rc = ndf_queue_cmd_wait(tn, t1);
	if (rc)
		return rc;

	rc = ndf_queue_cmd_cle(tn, cmd1);
	if (rc)
		return rc;

	if (addr_bytes) {
		rc = ndf_build_wait_busy(tn);
		if (rc)
			return rc;

		rc = ndf_queue_cmd_ale(tn, addr_bytes, nand,
				       page, col, page_size);
		if (rc)
			return rc;
	}

	/* CLE 2 */
	if (cmd2) {
		rc = ndf_build_wait_busy(tn);
		if (rc)
			return rc;

		rc = ndf_queue_cmd_cle(tn, cmd2);
		if (rc)
			return rc;
	}
	return 0;
}

static int ndf_build_post_cmd(struct octeontx_nfc *tn, int hold_time)
{
	int rc;

	/* Deselect chip */
	rc = ndf_queue_cmd_chip(tn, 0, 0, 0);
	if (rc)
		return rc;

	rc = ndf_queue_cmd_wait(tn, t2);
	if (rc)
		return rc;

	/* Release bus */
	rc = ndf_queue_cmd_bus(tn, 0);
	if (rc)
		return rc;

	rc = ndf_queue_cmd_wait(tn, hold_time);
	if (rc)
		return rc;

	/*
	 * Last action is ringing the doorbell with number of bus
	 * acquire-releases cycles (currently 1).
	 */
	writeq(1, tn->base + NDF_DRBELL);
	return 0;
}

/* Setup the NAND DMA engine for a transfer. */
static void ndf_setup_dma(struct octeontx_nfc *tn, int is_write,
			  dma_addr_t bus_addr, int len)
{
	u64 dma_cfg;

	dma_cfg = FIELD_PREP(NDF_DMA_CFG_RW, is_write) |
		  FIELD_PREP(NDF_DMA_CFG_SIZE, (len >> 3) - 1);
	dma_cfg |= NDF_DMA_CFG_EN;
	writeq(bus_addr, tn->base + NDF_DMA_ADR);
	writeq(dma_cfg, tn->base + NDF_DMA_CFG);
}

static int octeontx_nand_reset(struct octeontx_nfc *tn)
{
	int rc;

	rc = ndf_build_pre_cmd(tn, NAND_CMD_RESET, 0, 0, 0, 0);
	if (rc)
		return rc;

	rc = ndf_build_wait_busy(tn);
	if (rc)
		return rc;

	rc = ndf_build_post_cmd(tn, t2);
	if (rc)
		return rc;

	return 0;
}

static int ndf_read(struct octeontx_nfc *tn, int cmd1, int addr_bytes,
		    u64 page, u32 col, int cmd2, int len)
{
	dma_addr_t bus_addr = tn->use_status ? tn->stat_addr : tn->buf.dmaaddr;
	struct nand_chip *nand = tn->controller.active;
	int timing_mode, bytes, rc;
	union ndf_cmd cmd;
	u64 start, end;

	pr_debug("%s(%p, 0x%x, 0x%x, 0x%llx, 0x%x, 0x%x, 0x%x)\n", __func__,
		 tn, cmd1, addr_bytes, page, col, cmd2, len);
	if (!nand)
		timing_mode = default_onfi_timing;
	else
		timing_mode = nand->onfi_timing_mode_default;

	/* Build the command and address cycles */
	rc = ndf_build_pre_cmd(tn, cmd1, addr_bytes, page, col, cmd2);
	if (rc) {
		dev_err(tn->dev, "Build pre command failed\n");
		return rc;
	}

	/* This waits for some time, then waits for busy to be de-asserted. */
	rc = ndf_build_wait_busy(tn);
	if (rc) {
		dev_err(tn->dev, "Wait timeout\n");
		return rc;
	}

	memset(&cmd, 0, sizeof(cmd));

	if (timing_mode < 4)
		cmd.u.rd_cmd.opcode = NDF_OP_RD_CMD;
	else
		cmd.u.rd_cmd.opcode = NDF_OP_RD_EDO_CMD;

	cmd.u.rd_cmd.data = len;
	cmd.u.rd_cmd.rlen1 = t7;
	cmd.u.rd_cmd.rlen2 = t3;
	cmd.u.rd_cmd.rlen3 = t1;
	cmd.u.rd_cmd.rlen4 = t7;
	rc = ndf_submit(tn, &cmd);
	if (rc) {
		dev_err(tn->dev, "Error submitting command\n");
		return rc;
	}

	start = (u64)bus_addr;
	ndf_setup_dma(tn, 0, bus_addr, len);

	rc = ndf_build_post_cmd(tn, t2);
	if (rc) {
		dev_err(tn->dev, "Build post command failed\n");
		return rc;
	}

	/* Wait for the DMA to complete */
	rc = ndf_wait(tn);
	if (rc) {
		dev_err(tn->dev, "DMA timed out\n");
		return rc;
	}

	end = readq(tn->base + NDF_DMA_ADR);
	bytes = end - start;

	/* Make sure NDF is really done */
	rc = ndf_wait_idle(tn);
	if (rc) {
		dev_err(tn->dev, "poll idle failed\n");
		return rc;
	}

	pr_debug("%s: Read %d bytes\n", __func__, bytes);
	return bytes;
}

static int octeontx_nand_get_features(struct mtd_info *mtd,
				      struct nand_chip *chip, int feature_addr,
				      u8 *subfeature_para)
{
	struct nand_chip *nand = chip;
	struct octeontx_nfc *tn = to_otx_nfc(nand->controller);
	int len = 8;
	int rc;

	pr_debug("%s: feature addr: 0x%x\n", __func__, feature_addr);
	memset(tn->buf.dmabuf, 0xff, len);
	tn->buf.data_index = 0;
	tn->buf.data_len = 0;
	rc = ndf_read(tn, NAND_CMD_GET_FEATURES, 1, feature_addr, 0, 0, len);
	if (rc)
		return rc;

	memcpy(subfeature_para, tn->buf.dmabuf, ONFI_SUBFEATURE_PARAM_LEN);

	return 0;
}

static int octeontx_nand_set_features(struct mtd_info *mtd,
				      struct nand_chip *chip, int feature_addr,
				      u8 *subfeature_para)
{
	struct nand_chip *nand = chip;
	struct octeontx_nfc *tn = to_otx_nfc(nand->controller);
	const int len = ONFI_SUBFEATURE_PARAM_LEN;
	int rc;

	rc = ndf_build_pre_cmd(tn, NAND_CMD_SET_FEATURES,
			       1, feature_addr, 0, 0);
	if (rc)
		return rc;

	memcpy(tn->buf.dmabuf, subfeature_para, len);
	memset(tn->buf.dmabuf + len, 0, 8 - len);

	ndf_setup_dma(tn, 1, tn->buf.dmaaddr, 8);

	rc = ndf_queue_cmd_write(tn, 8);
	if (rc)
		return rc;

	rc = ndf_build_wait_busy(tn);
	if (rc)
		return rc;

	rc = ndf_build_post_cmd(tn, t2);
	if (rc)
		return rc;

	return 0;
}

/*
 * Read a page from NAND. If the buffer has room, the out of band
 * data will be included.
 */
static int ndf_page_read(struct octeontx_nfc *tn, u64 page, int col, int len)
{
	debug("%s(%p, 0x%llx, 0x%x, 0x%x) active: %p\n", __func__,
	      tn, page, col, len, tn->controller.active);
	struct nand_chip *nand = tn->controller.active;
	struct octeontx_nand_chip *chip = to_otx_nand(nand);
	int addr_bytes = chip->row_bytes + chip->col_bytes;

	memset(tn->buf.dmabuf, 0xff, len);
	return ndf_read(tn, NAND_CMD_READ0, addr_bytes,
		    page, col, NAND_CMD_READSTART, len);
}

/* Erase a NAND block */
static int ndf_block_erase(struct octeontx_nfc *tn, u64 page_addr)
{
	struct nand_chip *nand = tn->controller.active;
	struct octeontx_nand_chip *chip = to_otx_nand(nand);
	int addr_bytes = chip->row_bytes;
	int rc;

	rc = ndf_build_pre_cmd(tn, NAND_CMD_ERASE1, addr_bytes,
			       page_addr, 0, NAND_CMD_ERASE2);
	if (rc)
		return rc;

	/* Wait for R_B to signal erase is complete  */
	rc = ndf_build_wait_busy(tn);
	if (rc)
		return rc;

	rc = ndf_build_post_cmd(tn, t2);
	if (rc)
		return rc;

	/* Wait until the command queue is idle */
	return ndf_wait_idle(tn);
}

/*
 * Write a page (or less) to NAND.
 */
static int ndf_page_write(struct octeontx_nfc *tn, int page)
{
	int len, rc;
	struct nand_chip *nand = tn->controller.active;
	struct octeontx_nand_chip *chip = to_otx_nand(nand);
	int addr_bytes = chip->row_bytes + chip->col_bytes;

	len = tn->buf.data_len - tn->buf.data_index;
	chip->oob_only = (tn->buf.data_index >= nand->mtd.writesize);
	WARN_ON_ONCE(len & 0x7);

	ndf_setup_dma(tn, 1, tn->buf.dmaaddr + tn->buf.data_index, len);
	rc = ndf_build_pre_cmd(tn, NAND_CMD_SEQIN, addr_bytes, page, 0, 0);
	if (rc)
		return rc;

	rc = ndf_queue_cmd_write(tn, len);
	if (rc)
		return rc;

	rc = ndf_queue_cmd_cle(tn, NAND_CMD_PAGEPROG);
	if (rc)
		return rc;

	/* Wait for R_B to signal program is complete  */
	rc = ndf_build_wait_busy(tn);
	if (rc)
		return rc;

	rc = ndf_build_post_cmd(tn, t2);
	if (rc)
		return rc;

	/* Wait for the DMA to complete */
	rc = ndf_wait(tn);
	if (rc)
		return rc;

	/* Data transfer is done but NDF is not, it is waiting for R/B# */
	return ndf_wait_idle(tn);
}

static void octeontx_nand_cmdfunc(struct mtd_info *mtd, unsigned int command,
				  int column, int page_addr)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct octeontx_nand_chip *octeontx_nand = to_otx_nand(nand);
	struct octeontx_nfc *tn = to_otx_nfc(nand->controller);
	int rc;

	tn->selected_chip = octeontx_nand->cs;
	if (tn->selected_chip < 0 || tn->selected_chip >= NAND_MAX_CHIPS) {
		dev_err(tn->dev, "invalid chip select\n");
		return;
	}

	tn->use_status = false;

	pr_debug("%s(%p, 0x%x, 0x%x, 0x%x) cs: %d\n", __func__, mtd, command,
		 column, page_addr, tn->selected_chip);
	switch (command) {
	case NAND_CMD_READID:
		tn->buf.data_index = 0;
		octeontx_nand->oob_only = false;
		rc = ndf_read(tn, command, 1, column, 0, 0, 8);
		if (rc < 0)
			dev_err(tn->dev, "READID failed with %d\n", rc);
		else
			tn->buf.data_len = rc;
		break;

	case NAND_CMD_READOOB:
		octeontx_nand->oob_only = true;
		tn->buf.data_index = 0;
		tn->buf.data_len = 0;
		rc = ndf_page_read(tn, page_addr, column, mtd->oobsize);
		if (rc < mtd->oobsize)
			dev_err(tn->dev, "READOOB failed with %d\n",
				tn->buf.data_len);
		else
			tn->buf.data_len = rc;
		break;

	case NAND_CMD_READ0:
		octeontx_nand->oob_only = false;
		tn->buf.data_index = 0;
		tn->buf.data_len = 0;
		rc = ndf_page_read(tn, page_addr, column,
				   mtd->writesize + mtd->oobsize);

		if (rc < mtd->writesize + mtd->oobsize)
			dev_err(tn->dev, "READ0 failed with %d\n", rc);
		else
			tn->buf.data_len = rc;
		break;

	case NAND_CMD_STATUS:
		/* used in oob/not states */
		tn->use_status = true;
		rc = ndf_read(tn, command, 0, 0, 0, 0, 8);
		if (rc < 0)
			dev_err(tn->dev, "STATUS failed with %d\n", rc);
		break;

	case NAND_CMD_RESET:
		/* used in oob/not states */
		rc = octeontx_nand_reset(tn);
		if (rc < 0)
			dev_err(tn->dev, "RESET failed with %d\n", rc);
		break;

	case NAND_CMD_PARAM:
		octeontx_nand->oob_only = false;
		tn->buf.data_index = 0;
		rc = ndf_read(tn, command, 1, 0, 0, 0,
			      min(tn->buf.dmabuflen, 3 * 512));
		if (rc < 0)
			dev_err(tn->dev, "PARAM failed with %d\n", rc);
		else
			tn->buf.data_len = rc;
		break;

	case NAND_CMD_RNDOUT:
		tn->buf.data_index = column;
		break;

	case NAND_CMD_ERASE1:
		if (ndf_block_erase(tn, page_addr))
			dev_err(tn->dev, "ERASE1 failed\n");
		break;

	case NAND_CMD_ERASE2:
		/* We do all erase processing in the first command, so ignore
		 * this one.
		 */
		break;

	case NAND_CMD_SEQIN:
		octeontx_nand->oob_only = (column >= mtd->writesize);
		tn->buf.data_index = column;
		tn->buf.data_len = column;

		octeontx_nand->selected_page = page_addr;
		break;

	case NAND_CMD_PAGEPROG:
		rc = ndf_page_write(tn, octeontx_nand->selected_page);
		if (rc)
			dev_err(tn->dev, "PAGEPROG failed with %d\n", rc);
		break;

	case NAND_CMD_SET_FEATURES:
		octeontx_nand->oob_only = false;
		/* assume tn->buf.data_len == 4 of data has been set there */
		rc = octeontx_nand_set_features(mtd, nand,
						page_addr, tn->buf.dmabuf);
		if (rc)
			dev_err(tn->dev, "SET_FEATURES failed with %d\n", rc);
		break;

	case NAND_CMD_GET_FEATURES:
		octeontx_nand->oob_only = false;
		rc = octeontx_nand_get_features(mtd, nand,
						page_addr, tn->buf.dmabuf);
		if (!rc) {
			tn->buf.data_index = 0;
			tn->buf.data_len = 4;
		} else {
			dev_err(tn->dev, "GET_FEATURES failed with %d\n", rc);
		}
		break;

	default:
		WARN_ON_ONCE(1);
		dev_err(tn->dev, "unhandled nand cmd: %x\n", command);
	}
}

static int octeontx_nand_waitfunc(struct mtd_info *mtd, struct nand_chip *chip)
{
	struct octeontx_nfc *tn = to_otx_nfc(chip->controller);
	int ret;

	ret = ndf_wait_idle(tn);
	return (ret < 0) ? -EIO : 0;
}

/* check compatibility with ONFI timing mode#N, and optionally apply */
/* TODO: Implement chipnr support? */
static int octeontx_nand_setup_dat_intf(struct mtd_info *mtd, int chipnr,
					const struct nand_data_interface *conf)
{
	static const bool check_only;
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct octeontx_nand_chip *chip = to_otx_nand(nand);
	static u64 t_wc_n[MAX_ONFI_MODE + 2]; /* cache a mode signature */
	int mode; /* deduced mode number, for reporting and restricting */
	int rc;

	/*
	 * Cache timing modes for reporting, and reducing needless change.
	 *
	 * Challenge: caller does not pass ONFI mode#, but reporting the mode
	 * and restricting to a maximum, or a list, are useful for diagnosing
	 * new hardware.  So use tWC_min, distinct and monotonic across modes,
	 * to discover the requested/accepted mode number
	 */
	for (mode = MAX_ONFI_MODE; mode >= 0 && !t_wc_n[0]; mode--) {
		const struct nand_sdr_timings *t;

		t = onfi_async_timing_mode_to_sdr_timings(mode);
		if (!t)
			continue;
		t_wc_n[mode] = t->tWC_min;
	}

	if (!conf) {
		rc = -EINVAL;
	} else if (check_only) {
		rc = 0;
	} else if (nand->data_interface &&
			chip->iface_set && chip->iface_mode == mode) {
		/*
		 * Cases:
		 * - called from nand_reset, which clears DDR timing
		 *   mode back to SDR.  BUT if we're already in SDR,
		 *   timing mode persists over resets.
		 *   While mtd/nand layer only supports SDR,
		 *   this is always safe. And this driver only supports SDR.
		 *
		 * - called from post-power-event nand_reset (maybe
		 *   NFC+flash power down, or system hibernate.
		 *   Address this when CONFIG_PM support added
		 */
		rc = 0;
	} else {
		rc = octeontx_nfc_chip_set_timings(chip, &conf->timings.sdr);
		if (!rc) {
			chip->iface_mode = mode;
			chip->iface_set = true;
		}
	}
	return rc;
}

static void octeontx_bch_reset(void)
{
}

/*
 * Given a page, calculate the ECC code
 *
 * chip:	Pointer to NAND chip data structure
 * buf:		Buffer to calculate ECC on
 * code:	Buffer to hold ECC data
 *
 * Return 0 on success or -1 on failure
 */
static int octeontx_nand_bch_calculate_ecc_internal(struct mtd_info *mtd,
						    dma_addr_t ihandle,
						    u8 *code)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct octeontx_nfc *tn = to_otx_nfc(nand->controller);
	int rc;
	int i;
	static u8 *ecc_buffer;
	static int ecc_size;
	static unsigned long ecc_handle;
	union bch_resp *r = tn->bch_resp;

	if (!ecc_buffer || ecc_size < nand->ecc.size) {
		ecc_size = nand->ecc.size;
		ecc_buffer = dma_alloc_coherent(ecc_size,
						(unsigned long *)&ecc_handle);
	}

	memset(ecc_buffer, 0, nand->ecc.bytes);

	r->u16 = 0;
	__iowmb(); /* flush done=0 before making request */

	rc = octeontx_bch_encode(bch_vf, ihandle, nand->ecc.size,
				 nand->ecc.strength,
				 (dma_addr_t)ecc_handle, tn->bch_rhandle);

	if (!rc) {
		octeontx_bch_wait(bch_vf, r, tn->bch_rhandle);
	} else {
		dev_err(tn->dev, "octeontx_bch_encode failed\n");
		return -1;
	}

	if (!r->s.done || r->s.uncorrectable) {
		dev_err(tn->dev,
			"%s timeout, done:%d uncorr:%d corr:%d erased:%d\n",
			__func__, r->s.done, r->s.uncorrectable,
			r->s.num_errors, r->s.erased);
		octeontx_bch_reset();
		return -1;
	}

	memcpy(code, ecc_buffer, nand->ecc.bytes);

	for (i = 0; i < nand->ecc.bytes; i++)
		code[i] ^= tn->eccmask[i];

	return tn->bch_resp->s.num_errors;
}

/*
 * Given a page, calculate the ECC code
 *
 * mtd:        MTD block structure
 * dat:        raw data (unused)
 * ecc_code:   buffer for ECC
 */
static int octeontx_nand_bch_calculate(struct mtd_info *mtd,
				       const u8 *dat, u8 *ecc_code)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	dma_addr_t handle = dma_map_single((u8 *)dat,
					   nand->ecc.size, DMA_TO_DEVICE);
	int ret;

	ret = octeontx_nand_bch_calculate_ecc_internal(mtd, handle,
						       (void *)ecc_code);

	return ret;
}

/*
 * Detect and correct multi-bit ECC for a page
 *
 * mtd:        MTD block structure
 * dat:        raw data read from the chip
 * read_ecc:   ECC from the chip (unused)
 * isnull:     unused
 *
 * Returns number of bits corrected or -1 if unrecoverable
 */
static int octeontx_nand_bch_correct(struct mtd_info *mtd, u_char *dat,
				     u_char *read_ecc, u_char *isnull)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct octeontx_nfc *tn = to_otx_nfc(nand->controller);
	int i = nand->ecc.size + nand->ecc.bytes;
	static u8 *data_buffer;
	static dma_addr_t ihandle;
	static int buffer_size;
	dma_addr_t ohandle;
	union bch_resp *r = tn->bch_resp;
	int rc;

	if (i > buffer_size) {
		if (buffer_size)
			free(data_buffer);
		data_buffer = dma_alloc_coherent(i,
						 (unsigned long *)&ihandle);
		if (!data_buffer) {
			dev_err(tn->dev,
				"%s: Could not allocate %d bytes for buffer\n",
				__func__, i);
			goto error;
		}
		buffer_size = i;
	}

	memcpy(data_buffer, dat, nand->ecc.size);
	memcpy(data_buffer + nand->ecc.size, read_ecc, nand->ecc.bytes);

	for (i = 0; i < nand->ecc.bytes; i++)
		data_buffer[nand->ecc.size + i] ^= tn->eccmask[i];

	r->u16 = 0;
	__iowmb(); /* flush done=0 before making request */

	ohandle = dma_map_single(dat, nand->ecc.size, DMA_FROM_DEVICE);
	rc = octeontx_bch_decode(bch_vf, ihandle, nand->ecc.size,
				 nand->ecc.strength, ohandle, tn->bch_rhandle);

	if (!rc)
		octeontx_bch_wait(bch_vf, r, tn->bch_rhandle);

	if (rc) {
		dev_err(tn->dev, "octeontx_bch_decode failed\n");
		goto error;
	}

	if (!r->s.done) {
		dev_err(tn->dev, "Error: BCH engine timeout\n");
		octeontx_bch_reset();
		goto error;
	}

	if (r->s.erased) {
		debug("Info: BCH block is erased\n");
		return 0;
	}

	if (r->s.uncorrectable) {
		debug("Cannot correct NAND block, response: 0x%x\n",
		      r->u16);
		goto error;
	}

	return r->s.num_errors;

error:
	debug("Error performing bch correction\n");
	return -1;
}

void octeontx_nand_bch_hwctl(struct mtd_info *mtd, int mode)
{
	/* Do nothing. */
}

static int octeontx_nand_hw_bch_read_page(struct mtd_info *mtd,
					  struct nand_chip *chip, u8 *buf,
					  int oob_required, int page)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct octeontx_nfc *tn = to_otx_nfc(nand->controller);
	int i, eccsize = chip->ecc.size, ret;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	u8 *p;
	u8 *ecc_code = chip->buffers->ecccode;
	unsigned int max_bitflips = 0;

	/* chip->read_buf() insists on sequential order, we do OOB first */
	memcpy(chip->oob_poi, tn->buf.dmabuf + mtd->writesize, mtd->oobsize);

	/* Use private buffer as input for ECC correction */
	p = tn->buf.dmabuf;

	ret = mtd_ooblayout_get_eccbytes(mtd, ecc_code, chip->oob_poi, 0,
					 chip->ecc.total);
	if (ret)
		return ret;

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		int stat;

		debug("Correcting block offset %lx, ecc offset %x\n",
		      p - buf, i);
		stat = chip->ecc.correct(mtd, p, &ecc_code[i], NULL);

		if (stat < 0) {
			mtd->ecc_stats.failed++;
			debug("Cannot correct NAND page %d\n", page);
		} else {
			mtd->ecc_stats.corrected += stat;
			max_bitflips = max_t(unsigned int, max_bitflips, stat);
		}
	}

	/* Copy corrected data to caller's buffer now */
	memcpy(buf, tn->buf.dmabuf, mtd->writesize);

	return max_bitflips;
}

static int octeontx_nand_hw_bch_write_page(struct mtd_info *mtd,
					   struct nand_chip *chip,
					   const u8 *buf, int oob_required,
					   int page)
{
	struct octeontx_nfc *tn = to_otx_nfc(chip->controller);
	int i, eccsize = chip->ecc.size, ret;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	const u8 *p;
	u8 *ecc_calc = chip->buffers->ecccalc;

	debug("%s(buf?%p, oob%d p%x)\n",
	      __func__, buf, oob_required, page);
	for (i = 0; i < chip->ecc.total; i++)
		ecc_calc[i] = 0xFF;

	/* Copy the page data from caller's buffers to private buffer */
	chip->write_buf(mtd, buf, mtd->writesize);
	/* Use private date as source for ECC calculation */
	p = tn->buf.dmabuf;

	/* Hardware ECC calculation */
	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		int ret;

		ret = chip->ecc.calculate(mtd, p, &ecc_calc[i]);

		if (ret < 0)
			debug("calculate(mtd, p?%p, &ecc_calc[%d]?%p) returned %d\n",
			      p, i, &ecc_calc[i], ret);

		debug("block offset %lx, ecc offset %x\n", p - buf, i);
	}

	ret = mtd_ooblayout_set_eccbytes(mtd, ecc_calc, chip->oob_poi, 0,
					 chip->ecc.total);
	if (ret)
		return ret;

	/* Store resulting OOB into private buffer, will be sent to HW */
	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);

	return 0;
}

/**
 * nand_write_page_raw - [INTERN] raw page write function
 * @mtd: mtd info structure
 * @chip: nand chip info structure
 * @buf: data buffer
 * @oob_required: must write chip->oob_poi to OOB
 * @page: page number to write
 *
 * Not for syndrome calculating ECC controllers, which use a special oob layout.
 */
static int octeontx_nand_write_page_raw(struct mtd_info *mtd,
					struct nand_chip *chip,
					const u8 *buf, int oob_required,
					int page)
{
	chip->write_buf(mtd, buf, mtd->writesize);
	if (oob_required)
		chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);

	return 0;
}

/**
 * octeontx_nand_write_oob_std - [REPLACEABLE] the most common OOB data write
 *                             function
 * @mtd: mtd info structure
 * @chip: nand chip info structure
 * @page: page number to write
 */
static int octeontx_nand_write_oob_std(struct mtd_info *mtd,
				       struct nand_chip *chip,
				       int page)
{
	int status = 0;
	const u8 *buf = chip->oob_poi;
	int length = mtd->oobsize;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, mtd->writesize, page);
	chip->write_buf(mtd, buf, length);
	/* Send command to program the OOB data */
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

	status = chip->waitfunc(mtd, chip);

	return status & NAND_STATUS_FAIL ? -EIO : 0;
}

/**
 * octeontx_nand_read_page_raw - [INTERN] read raw page data without ecc
 * @mtd: mtd info structure
 * @chip: nand chip info structure
 * @buf: buffer to store read data
 * @oob_required: caller requires OOB data read to chip->oob_poi
 * @page: page number to read
 *
 * Not for syndrome calculating ECC controllers, which use a special oob layout.
 */
static int octeontx_nand_read_page_raw(struct mtd_info *mtd,
				       struct nand_chip *chip,
				       u8 *buf, int oob_required, int page)
{
	chip->read_buf(mtd, buf, mtd->writesize);
	if (oob_required)
		chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);
	return 0;
}

static int octeontx_nand_read_oob_std(struct mtd_info *mtd,
				      struct nand_chip *chip,
				      int page)

{
	chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page);
	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);
	return 0;
}

static int octeontx_nand_calc_bch_ecc_strength(struct nand_chip *nand)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct nand_ecc_ctrl *ecc = &nand->ecc;
	struct octeontx_nfc *tn = to_otx_nfc(nand->controller);
	int nsteps = mtd->writesize / ecc->size;
	int oobchunk = mtd->oobsize / nsteps;

	/* ecc->strength determines ecc_level and OOB's ecc_bytes. */
	const u8 strengths[]  = {4, 8, 16, 24, 32, 40, 48, 56, 60, 64};
	/* first set the desired ecc_level to match strengths[] */
	int index = ARRAY_SIZE(strengths) - 1;
	int need;

	while (index > 0 && !(ecc->options & NAND_ECC_MAXIMIZE) &&
	       strengths[index - 1] >= ecc->strength)
		index--;

	do {
		need = DIV_ROUND_UP(15 * strengths[index], 8);
		if (need <= oobchunk - 2)
			break;
	} while (index > 0);

	debug("%s: steps ds: %d, strength ds: %d\n", __func__,
	      nand->ecc_step_ds, nand->ecc_strength_ds);
	ecc->strength = strengths[index];
	ecc->bytes = need;
	debug("%s: strength: %d, bytes: %d\n", __func__, ecc->strength,
	      ecc->bytes);

	if (!tn->eccmask)
		tn->eccmask = devm_kzalloc(tn->dev, ecc->bytes, GFP_KERNEL);
	if (!tn->eccmask)
		return -ENOMEM;

	return 0;
}

/* sample the BCH signature of an erased (all 0xff) page,
 * to XOR into all page traffic, so erased pages have no ECC errors
 */
static int octeontx_bch_save_empty_eccmask(struct nand_chip *nand)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct octeontx_nfc *tn = to_otx_nfc(nand->controller);
	unsigned int eccsize = nand->ecc.size;
	unsigned int eccbytes = nand->ecc.bytes;
	u8 erased_ecc[eccbytes];
	unsigned long erased_handle;
	unsigned char *erased_page = dma_alloc_coherent(eccsize,
							&erased_handle);
	int i;
	int rc = 0;

	if (!erased_page)
		return -ENOMEM;

	memset(erased_page, 0xff, eccsize);
	memset(erased_ecc, 0, eccbytes);

	rc = octeontx_nand_bch_calculate_ecc_internal(mtd,
						      (dma_addr_t)erased_handle,
						      erased_ecc);

	free(erased_page);

	for (i = 0; i < eccbytes; i++)
		tn->eccmask[i] = erased_ecc[i] ^ 0xff;

	return rc;
}

static void octeontx_nfc_chip_sizing(struct nand_chip *nand)
{
	struct octeontx_nand_chip *chip = to_otx_nand(nand);
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct nand_ecc_ctrl *ecc = &nand->ecc;

	chip->row_bytes = nand->onfi_params.addr_cycles & 0xf;
	chip->col_bytes = nand->onfi_params.addr_cycles >> 4;
	debug("%s(%p) row bytes: %d, col bytes: %d, ecc mode: %d\n",
	      __func__, nand, chip->row_bytes, chip->col_bytes, ecc->mode);

	/*
	 * HW_BCH using OcteonTX BCH engine, or SOFT_BCH laid out in
	 * HW_BCH-compatible fashion, depending on devtree advice
	 * and kernel config.
	 * BCH/NFC hardware capable of subpage ops, not implemented.
	 */
	mtd_set_ooblayout(mtd, &nand_ooblayout_lp_ops);
	nand->options |= NAND_NO_SUBPAGE_WRITE;
	debug("%s: start steps: %d, size: %d, bytes: %d\n",
	      __func__, ecc->steps, ecc->size, ecc->bytes);
	debug("%s: step ds: %d, strength ds: %d\n", __func__,
	      nand->ecc_step_ds, nand->ecc_strength_ds);

	if (ecc->mode != NAND_ECC_NONE) {
		int nsteps = ecc->steps ? ecc->steps : 1;

		if (ecc->size && ecc->size != mtd->writesize)
			nsteps = mtd->writesize / ecc->size;
		else if (mtd->writesize > def_ecc_size &&
			 !(mtd->writesize & (def_ecc_size - 1)))
			nsteps = mtd->writesize / def_ecc_size;
		ecc->steps = nsteps;
		ecc->size = mtd->writesize / nsteps;
		ecc->bytes = mtd->oobsize / nsteps;

		if (nand->ecc_strength_ds)
			ecc->strength = nand->ecc_strength_ds;
		if (nand->ecc_step_ds)
			ecc->size = nand->ecc_step_ds;
		/*
		 * no subpage ops, but set subpage-shift to match ecc->steps
		 * so mtd_nandbiterrs tests appropriate boundaries
		 */
		if (!mtd->subpage_sft && !(ecc->steps & (ecc->steps - 1)))
			mtd->subpage_sft = fls(ecc->steps) - 1;

		if (IS_ENABLED(CONFIG_NAND_OCTEONTX_HW_ECC)) {
			debug("%s: ecc mode: %d\n", __func__, ecc->mode);
			if (ecc->mode != NAND_ECC_SOFT &&
			    !octeontx_nand_calc_bch_ecc_strength(nand)) {
				struct octeontx_nfc *tn =
					to_otx_nfc(nand->controller);

				debug("Using hardware BCH engine support\n");
				ecc->mode = NAND_ECC_HW_SYNDROME;
				ecc->read_page = octeontx_nand_hw_bch_read_page;
				ecc->write_page =
					octeontx_nand_hw_bch_write_page;
				ecc->read_page_raw =
					octeontx_nand_read_page_raw;
				ecc->write_page_raw =
					octeontx_nand_write_page_raw;
				ecc->read_oob = octeontx_nand_read_oob_std;
				ecc->write_oob = octeontx_nand_write_oob_std;

				ecc->calculate = octeontx_nand_bch_calculate;
				ecc->correct = octeontx_nand_bch_correct;
				ecc->hwctl = octeontx_nand_bch_hwctl;

				debug("NAND chip %d using hw_bch\n",
				      tn->selected_chip);
				debug(" %d bytes ECC per %d byte block\n",
				      ecc->bytes, ecc->size);
				debug(" for %d bits of correction per block.",
				      ecc->strength);
				octeontx_nand_calc_ecc_layout(nand);
				octeontx_bch_save_empty_eccmask(nand);
			}
		}
	}
}

static int octeontx_nfc_chip_init(struct octeontx_nfc *tn, struct udevice *dev,
				  ofnode node)
{
	struct octeontx_nand_chip *chip;
	struct nand_chip *nand;
	struct mtd_info *mtd;
	int ret;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	debug("%s: Getting chip select\n", __func__);
	ret = ofnode_read_s32(node, "reg", &chip->cs);
	if (ret) {
		dev_err(dev, "could not retrieve reg property: %d\n", ret);
		return ret;
	}

	if (chip->cs >= NAND_MAX_CHIPS) {
		dev_err(dev, "invalid reg value: %u (max CS = 7)\n", chip->cs);
		return -EINVAL;
	}
	debug("%s: chip select: %d\n", __func__, chip->cs);
	nand = &chip->nand;
	nand->controller = &tn->controller;
	if (!tn->controller.active)
		tn->controller.active = nand;

	debug("%s: Setting flash node\n", __func__);
	nand_set_flash_node(nand, node);

	nand->options = 0;
	nand->select_chip = octeontx_nand_select_chip;
	nand->cmdfunc = octeontx_nand_cmdfunc;
	nand->waitfunc = octeontx_nand_waitfunc;
	nand->read_byte = octeontx_nand_read_byte;
	nand->read_buf = octeontx_nand_read_buf;
	nand->write_buf = octeontx_nand_write_buf;
	nand->onfi_set_features = octeontx_nand_set_features;
	nand->onfi_get_features = octeontx_nand_get_features;
	nand->setup_data_interface = octeontx_nand_setup_dat_intf;

	mtd = nand_to_mtd(nand);
	debug("%s: mtd: %p\n", __func__, mtd);
	mtd->dev->parent = dev;

	debug("%s: NDF_MISC: 0x%llx\n", __func__,
	      readq(tn->base + NDF_MISC));

	/* TODO: support more then 1 chip */
	debug("%s: Scanning identification\n", __func__);
	ret = nand_scan_ident(mtd, 1, NULL);
	if (ret)
		return ret;

	debug("%s: Sizing chip\n", __func__);
	octeontx_nfc_chip_sizing(nand);

	debug("%s: Scanning tail\n", __func__);
	ret = nand_scan_tail(mtd);
	if (ret) {
		dev_err(dev, "nand_scan_tail failed: %d\n", ret);
		return ret;
	}

	debug("%s: Registering mtd\n", __func__);
	ret = nand_register(0, mtd);

	debug("%s: Adding tail\n", __func__);
	list_add_tail(&chip->node, &tn->chips);
	return 0;
}

static int octeontx_nfc_chips_init(struct octeontx_nfc *tn)
{
	struct udevice *dev = tn->dev;
	ofnode node = dev_ofnode(dev);
	ofnode nand_node;
	int nr_chips = of_get_child_count(node);
	int ret;

	debug("%s: node: %s\n", __func__, ofnode_get_name(node));
	debug("%s: %d chips\n", __func__, nr_chips);
	if (nr_chips > NAND_MAX_CHIPS) {
		dev_err(dev, "too many NAND chips: %d\n", nr_chips);
		return -EINVAL;
	}

	if (!nr_chips) {
		debug("no DT NAND chips found\n");
		return -ENODEV;
	}

	pr_info("%s: scanning %d chips DTs\n", __func__, nr_chips);

	ofnode_for_each_subnode(nand_node, node) {
		debug("%s: Calling octeontx_nfc_chip_init(%p, %s, %ld)\n",
		      __func__, tn, dev->name, nand_node.of_offset);
		ret = octeontx_nfc_chip_init(tn, dev, nand_node);
		if (ret)
			return ret;
	}
	return 0;
}

/* Reset NFC and initialize registers. */
static int octeontx_nfc_init(struct octeontx_nfc *tn)
{
	const struct nand_sdr_timings *timings;
	u64 ndf_misc;
	int rc;

	/* Initialize values and reset the fifo */
	ndf_misc = readq(tn->base + NDF_MISC);

	ndf_misc &= ~NDF_MISC_EX_DIS;
	ndf_misc |= (NDF_MISC_BT_DIS | NDF_MISC_RST_FF);
	writeq(ndf_misc, tn->base + NDF_MISC);
	debug("%s: NDF_MISC: 0x%llx\n", __func__, readq(tn->base + NDF_MISC));

	/* Bring the fifo out of reset */
	ndf_misc &= ~(NDF_MISC_RST_FF);

	/* Maximum of co-processor cycles for glitch filtering */
	ndf_misc |= FIELD_PREP(NDF_MISC_WAIT_CNT, 0x3f);

	writeq(ndf_misc, tn->base + NDF_MISC);

	/* Set timing parameters to onfi mode 0 for probing */
	timings = onfi_async_timing_mode_to_sdr_timings(0);
	if (IS_ERR(timings))
		return PTR_ERR(timings);
	rc = set_default_timings(tn, timings);
	if (rc)
		return rc;

	return 0;
}

static int octeontx_pci_nand_probe(struct udevice *dev)
{
	struct octeontx_nfc *tn = dev_get_priv(dev);
	int ret;
	static bool probe_done;

	debug("%s(%s) tn: %p\n", __func__, dev->name, tn);
	if (probe_done)
		return 0;

	if (IS_ENABLED(CONFIG_NAND_OCTEONTX_HW_ECC)) {
		bch_vf = octeontx_bch_getv();
		if (!bch_vf) {
			struct octeontx_probe_device *probe_dev;

			debug("%s: bch not yet initialized\n", __func__);
			probe_dev = calloc(sizeof(*probe_dev), 1);
			if (!probe_dev) {
				printf("%s: Out of memory\n", __func__);
				return -ENOMEM;
			}
			probe_dev->dev = dev;
			INIT_LIST_HEAD(&probe_dev->list);
			list_add_tail(&probe_dev->list,
				      &octeontx_pci_nand_deferred_devices);
			debug("%s: Defering probe until after BCH initialization\n",
			      __func__);
			return 0;
		}
	}

	tn->dev = dev;
	INIT_LIST_HEAD(&tn->chips);

	tn->base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE, PCI_REGION_MEM);
	if (!tn->base) {
		ret = -EINVAL;
		goto release;
	}
	debug("%s: bar at %p\n", __func__, tn->base);
	tn->buf.dmabuflen = NAND_MAX_PAGESIZE + NAND_MAX_OOBSIZE;
	tn->buf.dmabuf = dma_alloc_coherent(tn->buf.dmabuflen,
					    (unsigned long *)&tn->buf.dmaaddr);
	if (!tn->buf.dmabuf) {
		ret = -ENOMEM;
		debug("%s: Could not allocate DMA buffer\n", __func__);
		goto unclk;
	}

	/* one hw-bch response, for one outstanding transaction */
	tn->bch_resp = dma_alloc_coherent(sizeof(*tn->bch_resp),
					  (unsigned long *)&tn->bch_rhandle);

	tn->stat = dma_alloc_coherent(8, (unsigned long *)&tn->stat_addr);
	if (!tn->stat || !tn->bch_resp) {
		debug("%s: Could not allocate bch status or response\n",
		      __func__);
		ret = -ENOMEM;
		goto unclk;
	}

	debug("%s: Calling octeontx_nfc_init()\n", __func__);
	octeontx_nfc_init(tn);
	debug("%s: Initializing chips\n", __func__);
	ret = octeontx_nfc_chips_init(tn);
	debug("%s: init chips ret: %d\n", __func__, ret);
	if (ret) {
		if (ret != -ENODEV)
			dev_err(dev, "failed to init nand chips\n");
		goto unclk;
	}
	dev_info(dev, "probed\n");
	return 0;

unclk:
release:
	return ret;
}

int octeontx_pci_nand_disable(struct udevice *dev)
{
	struct octeontx_nfc *tn = dev_get_priv(dev);
	u64 dma_cfg;
	u64 ndf_misc;

	debug("%s: Disabling NAND device %s\n", __func__, dev->name);
	dma_cfg = readq(tn->base + NDF_DMA_CFG);
	dma_cfg &= ~NDF_DMA_CFG_EN;
	dma_cfg |= NDF_DMA_CFG_CLR;
	writeq(dma_cfg, tn->base + NDF_DMA_CFG);

	/* Disable execution and put FIFO in reset mode */
	ndf_misc = readq(tn->base + NDF_MISC);
	ndf_misc |= NDF_MISC_EX_DIS | NDF_MISC_RST_FF;
	writeq(ndf_misc, tn->base + NDF_MISC);
	ndf_misc &= ~NDF_MISC_RST_FF;
	writeq(ndf_misc, tn->base + NDF_MISC);
#ifdef DEBUG
	printf("%s: NDF_MISC: 0x%llx\n", __func__, readq(tn->base + NDF_MISC));
#endif
	/* Clear any interrupts and enable bits */
	writeq(~0ull, tn->base + NDF_INT_ENA_W1C);
	writeq(~0ull, tn->base + NDF_INT);
	debug("%s: NDF_ST_REG: 0x%llx\n", __func__,
	      readq(tn->base + NDF_ST_REG));
	return 0;
}

/**
 * Since it's possible (and even likely) that the NAND device will be probed
 * before the BCH device has been probed, we may need to defer the probing.
 *
 * In this case, the initial probe returns success but the actual probing
 * is deferred until the BCH VF has been probed.
 *
 * Return:	0 for success, otherwise error
 */
int octeontx_pci_nand_deferred_probe(void)
{
	int rc = 0;
	struct octeontx_probe_device *pdev;

	debug("%s: Performing deferred probing\n", __func__);
	list_for_each_entry(pdev, &octeontx_pci_nand_deferred_devices, list) {
		debug("%s: Probing %s\n", __func__, pdev->dev->name);
		dev_get_flags(pdev->dev) &= ~DM_FLAG_ACTIVATED;
		rc = device_probe(pdev->dev);
		if (rc && rc != -ENODEV) {
			printf("%s: Error %d with deferred probe of %s\n",
			       __func__, rc, pdev->dev->name);
			break;
		}
	}
	return rc;
}

static const struct pci_device_id octeontx_nfc_pci_id_table[] = {
	{ PCI_VDEVICE(CAVIUM, 0xA04F) },
	{}
};

static int octeontx_nand_of_to_plat(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id octeontx_nand_ids[] = {
	{ .compatible = "cavium,cn8130-nand" },
	{ },
};

U_BOOT_DRIVER(octeontx_pci_nand) = {
	.name	= OCTEONTX_NAND_DRIVER_NAME,
	.id	= UCLASS_MTD,
	.of_match = of_match_ptr(octeontx_nand_ids),
	.of_to_plat = octeontx_nand_of_to_plat,
	.probe = octeontx_pci_nand_probe,
	.priv_auto	= sizeof(struct octeontx_nfc),
	.remove = octeontx_pci_nand_disable,
	.flags = DM_FLAG_OS_PREPARE,
};

U_BOOT_PCI_DEVICE(octeontx_pci_nand, octeontx_nfc_pci_id_table);

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	if (IS_ENABLED(CONFIG_NAND_OCTEONTX_HW_ECC)) {
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(octeontx_pci_bchpf),
						  &dev);
		if (ret && ret != -ENODEV) {
			pr_err("Failed to initialize OcteonTX BCH PF controller. (error %d)\n",
			       ret);
		}
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(octeontx_pci_bchvf),
						  &dev);
		if (ret && ret != -ENODEV) {
			pr_err("Failed to initialize OcteonTX BCH VF controller. (error %d)\n",
			       ret);
		}
	}

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(octeontx_pci_nand),
					  &dev);
	if (ret && ret != -ENODEV)
		pr_err("Failed to initialize OcteonTX NAND controller. (error %d)\n",
		       ret);
}
