// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2022 ATMEL
 * Copyright 2017 Free Electrons
 *
 * Author: Boris Brezillon <boris.brezillon@free-electrons.com>
 *
 * Derived from the atmel_nand.c driver which contained the following
 * copyrights:
 *
 *   Copyright 2003 Rick Bronson
 *
 *   Derived from drivers/mtd/nand/autcpu12.c (removed in v3.8)
 *	Copyright 2001 Thomas Gleixner (gleixner@autronix.de)
 *
 *   Derived from drivers/mtd/spia.c (removed in v3.8)
 *	Copyright 2000 Steven J. Hill (sjhill@cotw.com)
 *
 *
 *   Add Hardware ECC support for AT91SAM9260 / AT91SAM9263
 *	Richard Genoud (richard.genoud@gmail.com), Adeneo Copyright 2007
 *
 *   Derived from Das U-Boot source code
 *	(u-boot-1.1.5/board/atmel/at91sam9263ek/nand.c)
 *	Copyright 2006 ATMEL Rousset, Lacressonniere Nicolas
 *
 *   Add Programmable Multibit ECC support for various AT91 SoC
 *	Copyright 2012 ATMEL, Hong Xu
 *
 *   Add Nand Flash Controller support for SAMA5 SoC
 *	Copyright 2013 ATMEL, Josh Wu (josh.wu@atmel.com)
 *
 *   Port from Linux
 *	Balamanikandan Gunasundar(balamanikandan.gunasundar@microchip.com)
 *	Copyright (C) 2022 Microchip Technology Inc.
 *
 * A few words about the naming convention in this file. This convention
 * applies to structure and function names.
 *
 * Prefixes:
 *
 * - atmel_nand_: all generic structures/functions
 * - atmel_smc_nand_: all structures/functions specific to the SMC interface
 *		      (at91sam9 and avr32 SoCs)
 * - atmel_hsmc_nand_: all structures/functions specific to the HSMC interface
 *		       (sama5 SoCs and later)
 * - atmel_nfc_: all structures/functions used to manipulate the NFC sub-block
 *		 that is available in the HSMC block
 * - <soc>_nand_: all SoC specific structures/functions
 */

#include <asm-generic/gpio.h>
#include <clk.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/of_addr.h>
#include <dm/of_access.h>
#include <dm/uclass.h>
#include <linux/completion.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/ioport.h>
#include <linux/mfd/syscon/atmel-matrix.h>
#include <linux/mfd/syscon/atmel-smc.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/mtd.h>
#include <linux/time.h>
#include <mach/at91_sfr.h>
#include <nand.h>
#include <regmap.h>
#include <syscon.h>

#include "pmecc.h"

#define ATMEL_HSMC_NFC_CFG			0x0
#define ATMEL_HSMC_NFC_CFG_SPARESIZE(x)		(((x) / 4) << 24)
#define ATMEL_HSMC_NFC_CFG_SPARESIZE_MASK	GENMASK(30, 24)
#define ATMEL_HSMC_NFC_CFG_DTO(cyc, mul)	(((cyc) << 16) | ((mul) << 20))
#define ATMEL_HSMC_NFC_CFG_DTO_MAX		GENMASK(22, 16)
#define ATMEL_HSMC_NFC_CFG_RBEDGE		BIT(13)
#define ATMEL_HSMC_NFC_CFG_FALLING_EDGE		BIT(12)
#define ATMEL_HSMC_NFC_CFG_RSPARE		BIT(9)
#define ATMEL_HSMC_NFC_CFG_WSPARE		BIT(8)
#define ATMEL_HSMC_NFC_CFG_PAGESIZE_MASK	GENMASK(2, 0)
#define ATMEL_HSMC_NFC_CFG_PAGESIZE(x)		(fls((x) / 512) - 1)

#define ATMEL_HSMC_NFC_CTRL			0x4
#define ATMEL_HSMC_NFC_CTRL_EN			BIT(0)
#define ATMEL_HSMC_NFC_CTRL_DIS			BIT(1)

#define ATMEL_HSMC_NFC_SR			0x8
#define ATMEL_HSMC_NFC_IER			0xc
#define ATMEL_HSMC_NFC_IDR			0x10
#define ATMEL_HSMC_NFC_IMR			0x14
#define ATMEL_HSMC_NFC_SR_ENABLED		BIT(1)
#define ATMEL_HSMC_NFC_SR_RB_RISE		BIT(4)
#define ATMEL_HSMC_NFC_SR_RB_FALL		BIT(5)
#define ATMEL_HSMC_NFC_SR_BUSY			BIT(8)
#define ATMEL_HSMC_NFC_SR_WR			BIT(11)
#define ATMEL_HSMC_NFC_SR_CSID			GENMASK(14, 12)
#define ATMEL_HSMC_NFC_SR_XFRDONE		BIT(16)
#define ATMEL_HSMC_NFC_SR_CMDDONE		BIT(17)
#define ATMEL_HSMC_NFC_SR_DTOE			BIT(20)
#define ATMEL_HSMC_NFC_SR_UNDEF			BIT(21)
#define ATMEL_HSMC_NFC_SR_AWB			BIT(22)
#define ATMEL_HSMC_NFC_SR_NFCASE		BIT(23)
#define ATMEL_HSMC_NFC_SR_ERRORS		(ATMEL_HSMC_NFC_SR_DTOE | \
						 ATMEL_HSMC_NFC_SR_UNDEF | \
						 ATMEL_HSMC_NFC_SR_AWB | \
						 ATMEL_HSMC_NFC_SR_NFCASE)
#define ATMEL_HSMC_NFC_SR_RBEDGE(x)		BIT((x) + 24)

#define ATMEL_HSMC_NFC_ADDR			0x18
#define ATMEL_HSMC_NFC_BANK			0x1c

#define ATMEL_NFC_MAX_RB_ID			7

#define ATMEL_NFC_SRAM_SIZE			0x2400

#define ATMEL_NFC_CMD(pos, cmd)			((cmd) << (((pos) * 8) + 2))
#define ATMEL_NFC_VCMD2				BIT(18)
#define ATMEL_NFC_ACYCLE(naddrs)		((naddrs) << 19)
#define ATMEL_NFC_CSID(cs)			((cs) << 22)
#define ATMEL_NFC_DATAEN			BIT(25)
#define ATMEL_NFC_NFCWR				BIT(26)

#define ATMEL_NFC_MAX_ADDR_CYCLES		5

#define ATMEL_NAND_ALE_OFFSET			BIT(21)
#define ATMEL_NAND_CLE_OFFSET			BIT(22)

#define DEFAULT_TIMEOUT_MS			1000
#define MIN_DMA_LEN				128

static struct nand_ecclayout atmel_pmecc_oobinfo;

struct nand_controller_ops {
	int (*attach_chip)(struct nand_chip *chip);
	int (*setup_data_interface)(struct mtd_info *mtd, int chipnr,
				    const struct nand_data_interface *conf);
};

struct nand_controller {
	const struct nand_controller_ops *ops;
};

enum atmel_nand_rb_type {
	ATMEL_NAND_NO_RB,
	ATMEL_NAND_NATIVE_RB,
	ATMEL_NAND_GPIO_RB,
};

struct atmel_nand_rb {
	enum atmel_nand_rb_type type;
	union {
		struct gpio_desc gpio;
		int id;
	};
};

struct atmel_nand_cs {
	int id;
	struct atmel_nand_rb rb;
	struct gpio_desc csgpio;
	struct {
		void __iomem *virt;
		dma_addr_t dma;
	} io;

	struct atmel_smc_cs_conf smcconf;
};

struct atmel_nand {
	struct list_head node;
	struct udevice *dev;
	struct nand_chip base;
	struct atmel_nand_cs *activecs;
	struct atmel_pmecc_user *pmecc;
	struct gpio_desc cdgpio;
	int numcs;
	struct nand_controller *controller;
	struct atmel_nand_cs cs[];
};

static inline struct atmel_nand *to_atmel_nand(struct nand_chip *chip)
{
	return container_of(chip, struct atmel_nand, base);
}

enum atmel_nfc_data_xfer {
	ATMEL_NFC_NO_DATA,
	ATMEL_NFC_READ_DATA,
	ATMEL_NFC_WRITE_DATA,
};

struct atmel_nfc_op {
	u8 cs;
	u8 ncmds;
	u8 cmds[2];
	u8 naddrs;
	u8 addrs[5];
	enum atmel_nfc_data_xfer data;
	u32 wait;
	u32 errors;
};

struct atmel_nand_controller;
struct atmel_nand_controller_caps;

struct atmel_nand_controller_ops {
	int (*probe)(struct udevice *udev,
		     const struct atmel_nand_controller_caps *caps);
	int (*remove)(struct atmel_nand_controller *nc);
	void (*nand_init)(struct atmel_nand_controller *nc,
			  struct atmel_nand *nand);
	int (*ecc_init)(struct nand_chip *chip);
	int (*setup_data_interface)(struct atmel_nand *nand, int csline,
				    const struct nand_data_interface *conf);
};

struct atmel_nand_controller_caps {
	bool has_dma;
	bool legacy_of_bindings;
	u32 ale_offs;
	u32 cle_offs;
	const char *ebi_csa_regmap_name;
	const struct atmel_nand_controller_ops *ops;
};

struct atmel_nand_controller {
	struct nand_controller base;
	const struct atmel_nand_controller_caps *caps;
	struct udevice *dev;
	struct regmap *smc;
	struct dma_chan *dmac;
	struct atmel_pmecc *pmecc;
	struct list_head chips;
	struct clk *mck;
};

static inline struct atmel_nand_controller *
to_nand_controller(struct nand_controller *ctl)
{
	return container_of(ctl, struct atmel_nand_controller, base);
}

struct atmel_smc_nand_ebi_csa_cfg {
	u32 offs;
	u32 nfd0_on_d16;
};

struct atmel_smc_nand_controller {
	struct atmel_nand_controller base;
	struct regmap *ebi_csa_regmap;
	struct atmel_smc_nand_ebi_csa_cfg *ebi_csa;
};

static inline struct atmel_smc_nand_controller *
to_smc_nand_controller(struct nand_controller *ctl)
{
	return container_of(to_nand_controller(ctl),
			    struct atmel_smc_nand_controller, base);
}

struct atmel_hsmc_nand_controller {
	struct atmel_nand_controller base;
	struct {
		struct gen_pool *pool;
		void __iomem *virt;
		dma_addr_t dma;
	} sram;
	const struct atmel_hsmc_reg_layout *hsmc_layout;
	struct regmap *io;
	struct atmel_nfc_op op;
	struct completion complete;
	int irq;

	/* Only used when instantiating from legacy DT bindings. */
	struct clk *clk;
};

static inline struct atmel_hsmc_nand_controller *
to_hsmc_nand_controller(struct nand_controller *ctl)
{
	return container_of(to_nand_controller(ctl),
			    struct atmel_hsmc_nand_controller, base);
}

static void pmecc_config_ecc_layout(struct nand_ecclayout *layout,
				    int oobsize, int ecc_len)
{
	int i;

	layout->eccbytes = ecc_len;

	/* ECC will occupy the last ecc_len bytes continuously */
	for (i = 0; i < ecc_len; i++)
		layout->eccpos[i] = oobsize - ecc_len + i;

	layout->oobfree[0].offset = 2;
	layout->oobfree[0].length =
		oobsize - ecc_len - layout->oobfree[0].offset;
}

static bool atmel_nfc_op_done(struct atmel_nfc_op *op, u32 status)
{
	op->errors |= status & ATMEL_HSMC_NFC_SR_ERRORS;
	op->wait ^= status & op->wait;

	return !op->wait || op->errors;
}

static int atmel_nfc_wait(struct atmel_hsmc_nand_controller *nc, bool poll,
			  unsigned int timeout_ms)
{
	int ret;
	u32 status;

	if (!timeout_ms)
		timeout_ms = DEFAULT_TIMEOUT_MS;

	if (poll)
		ret = regmap_read_poll_timeout(nc->base.smc,
					       ATMEL_HSMC_NFC_SR, status,
					       atmel_nfc_op_done(&nc->op,
								 status),
					       0, timeout_ms);
	else
		return -EOPNOTSUPP;

	if (nc->op.errors & ATMEL_HSMC_NFC_SR_DTOE) {
		dev_err(nc->base.dev, "Waiting NAND R/B Timeout\n");
		ret = -ETIMEDOUT;
	}

	if (nc->op.errors & ATMEL_HSMC_NFC_SR_UNDEF) {
		dev_err(nc->base.dev, "Access to an undefined area\n");
		ret = -EIO;
	}

	if (nc->op.errors & ATMEL_HSMC_NFC_SR_AWB) {
		dev_err(nc->base.dev, "Access while busy\n");
		ret = -EIO;
	}

	if (nc->op.errors & ATMEL_HSMC_NFC_SR_NFCASE) {
		dev_err(nc->base.dev, "Wrong access size\n");
		ret = -EIO;
	}

	return ret;
}

static u8 atmel_nand_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct atmel_nand *nand = to_atmel_nand(chip);

	return ioread8(nand->activecs->io.virt);
}

static void atmel_nand_write_byte(struct mtd_info *mtd, u8 byte)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct atmel_nand *nand = to_atmel_nand(chip);

	if (chip->options & NAND_BUSWIDTH_16)
		iowrite16(byte | (byte << 8), nand->activecs->io.virt);
	else
		iowrite8(byte, nand->activecs->io.virt);
}

static void atmel_nand_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct atmel_nand *nand = to_atmel_nand(chip);

	if (chip->options & NAND_BUSWIDTH_16)
		ioread16_rep(nand->activecs->io.virt, buf, len / 2);
	else
		ioread8_rep(nand->activecs->io.virt, buf, len);
}

static void atmel_nand_write_buf(struct mtd_info *mtd, const u8 *buf, int len)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct atmel_nand *nand = to_atmel_nand(chip);

	if (chip->options & NAND_BUSWIDTH_16)
		iowrite16_rep(nand->activecs->io.virt, buf, len / 2);
	else
		iowrite8_rep(nand->activecs->io.virt, buf, len);
}

static int atmel_nand_dev_ready(struct mtd_info *mtd)
{
	struct nand_chip  *chip = mtd_to_nand(mtd);
	struct atmel_nand *nand = to_atmel_nand(chip);

	return dm_gpio_get_value(&nand->activecs->rb.gpio);
}

static void atmel_nand_select_chip(struct mtd_info *mtd, int cs)
{
	struct nand_chip *chip =  mtd_to_nand(mtd);
	struct atmel_nand *nand = to_atmel_nand(chip);

	if (cs < 0 || cs >= nand->numcs) {
		nand->activecs = NULL;
		chip->dev_ready = NULL;
		return;
	}

	nand->activecs = &nand->cs[cs];

	if (nand->activecs->rb.type == ATMEL_NAND_GPIO_RB)
		chip->dev_ready = atmel_nand_dev_ready;
}

static int atmel_hsmc_nand_dev_ready(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct atmel_hsmc_nand_controller *nc;
	u32 status;

	nc = to_hsmc_nand_controller(nand->controller);

	regmap_read(nc->base.smc, ATMEL_HSMC_NFC_SR, &status);

	return status & ATMEL_HSMC_NFC_SR_RBEDGE(nand->activecs->rb.id);
}

static void atmel_hsmc_nand_select_chip(struct mtd_info *mtd, int cs)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct atmel_hsmc_nand_controller *nc;

	nc = to_hsmc_nand_controller(nand->controller);

	atmel_nand_select_chip(mtd, cs);

	if (!nand->activecs) {
		regmap_write(nc->base.smc, ATMEL_HSMC_NFC_CTRL,
			     ATMEL_HSMC_NFC_CTRL_DIS);
		return;
	}

	if (nand->activecs->rb.type == ATMEL_NAND_NATIVE_RB)
		chip->dev_ready = atmel_hsmc_nand_dev_ready;

	regmap_update_bits(nc->base.smc, ATMEL_HSMC_NFC_CFG,
			   ATMEL_HSMC_NFC_CFG_PAGESIZE_MASK |
			   ATMEL_HSMC_NFC_CFG_SPARESIZE_MASK |
			   ATMEL_HSMC_NFC_CFG_RSPARE |
			   ATMEL_HSMC_NFC_CFG_WSPARE,
			   ATMEL_HSMC_NFC_CFG_PAGESIZE(mtd->writesize) |
			   ATMEL_HSMC_NFC_CFG_SPARESIZE(mtd->oobsize) |
			   ATMEL_HSMC_NFC_CFG_RSPARE);
	regmap_write(nc->base.smc, ATMEL_HSMC_NFC_CTRL,
		     ATMEL_HSMC_NFC_CTRL_EN);
}

static int atmel_nfc_exec_op(struct atmel_hsmc_nand_controller *nc, bool poll)
{
	u8 *addrs = nc->op.addrs;
	unsigned int op = 0;
	u32 addr, val;
	int i, ret;

	nc->op.wait = ATMEL_HSMC_NFC_SR_CMDDONE;

	for (i = 0; i < nc->op.ncmds; i++)
		op |= ATMEL_NFC_CMD(i, nc->op.cmds[i]);

	if (nc->op.naddrs == ATMEL_NFC_MAX_ADDR_CYCLES)
		regmap_write(nc->base.smc, ATMEL_HSMC_NFC_ADDR, *addrs++);

	op |= ATMEL_NFC_CSID(nc->op.cs) |
	      ATMEL_NFC_ACYCLE(nc->op.naddrs);

	if (nc->op.ncmds > 1)
		op |= ATMEL_NFC_VCMD2;

	addr = addrs[0] | (addrs[1] << 8) | (addrs[2] << 16) |
	       (addrs[3] << 24);

	if (nc->op.data != ATMEL_NFC_NO_DATA) {
		op |= ATMEL_NFC_DATAEN;
		nc->op.wait |= ATMEL_HSMC_NFC_SR_XFRDONE;

		if (nc->op.data == ATMEL_NFC_WRITE_DATA)
			op |= ATMEL_NFC_NFCWR;
	}

	/* Clear all flags. */
	regmap_read(nc->base.smc, ATMEL_HSMC_NFC_SR, &val);

	/* Send the command. */
	regmap_write(nc->io, op, addr);

	ret = atmel_nfc_wait(nc, poll, 0);
	if (ret)
		dev_err(nc->base.dev,
			"Failed to send NAND command (err = %d)!",
			ret);

	/* Reset the op state. */
	memset(&nc->op, 0, sizeof(nc->op));

	return ret;
}

static void atmel_hsmc_nand_cmd_ctrl(struct mtd_info *mtd, int dat,
				     unsigned int ctrl)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct atmel_hsmc_nand_controller *nc;

	nc = to_hsmc_nand_controller(nand->controller);

	if (ctrl & NAND_ALE) {
		if (nc->op.naddrs == ATMEL_NFC_MAX_ADDR_CYCLES)
			return;

		nc->op.addrs[nc->op.naddrs++] = dat;
	} else if (ctrl & NAND_CLE) {
		if (nc->op.ncmds > 1)
			return;

		nc->op.cmds[nc->op.ncmds++] = dat;
	}

	if (dat == NAND_CMD_NONE) {
		nc->op.cs = nand->activecs->id;
		atmel_nfc_exec_op(nc, true);
	}
}

static void atmel_nand_cmd_ctrl(struct mtd_info *mtd, int cmd,
				unsigned int ctrl)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct atmel_nand_controller *nc;

	nc = to_nand_controller(nand->controller);

	if ((ctrl & NAND_CTRL_CHANGE) &&
	    dm_gpio_is_valid(&nand->activecs->csgpio)) {
		if (ctrl & NAND_NCE)
			dm_gpio_set_value(&nand->activecs->csgpio, 0);
		else
			dm_gpio_set_value(&nand->activecs->csgpio, 1);
	}

	if (ctrl & NAND_ALE)
		writeb(cmd, nand->activecs->io.virt + nc->caps->ale_offs);
	else if (ctrl & NAND_CLE)
		writeb(cmd, nand->activecs->io.virt + nc->caps->cle_offs);
}

static void atmel_nfc_copy_to_sram(struct nand_chip *chip, const u8 *buf,
				   bool oob_required)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct atmel_hsmc_nand_controller *nc;
	int ret = -EIO;

	nc = to_hsmc_nand_controller(nand->controller);

	if (ret)
		memcpy_toio(nc->sram.virt, buf, mtd->writesize);

	if (oob_required)
		memcpy_toio(nc->sram.virt + mtd->writesize, chip->oob_poi,
			    mtd->oobsize);
}

static void atmel_nfc_copy_from_sram(struct nand_chip *chip, u8 *buf,
				     bool oob_required)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct atmel_hsmc_nand_controller *nc;
	int ret = -EIO;

	nc = to_hsmc_nand_controller(nand->controller);

	if (ret)
		memcpy_fromio(buf, nc->sram.virt, mtd->writesize);

	if (oob_required)
		memcpy_fromio(chip->oob_poi, nc->sram.virt + mtd->writesize,
			      mtd->oobsize);
}

static void atmel_nfc_set_op_addr(struct nand_chip *chip, int page, int column)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct atmel_hsmc_nand_controller *nc;

	nc = to_hsmc_nand_controller(nand->controller);

	if (column >= 0) {
		nc->op.addrs[nc->op.naddrs++] = column;

		/*
		 * 2 address cycles for the column offset on large page NANDs.
		 */
		if (mtd->writesize > 512)
			nc->op.addrs[nc->op.naddrs++] = column >> 8;
	}

	if (page >= 0) {
		nc->op.addrs[nc->op.naddrs++] = page;
		nc->op.addrs[nc->op.naddrs++] = page >> 8;

		if (chip->options & NAND_ROW_ADDR_3)
			nc->op.addrs[nc->op.naddrs++] = page >> 16;
	}
}

static int atmel_nand_pmecc_enable(struct nand_chip *chip, int op, bool raw)
{
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct atmel_nand_controller *nc;
	int ret;

	nc = to_nand_controller(nand->controller);

	if (raw)
		return 0;

	ret = atmel_pmecc_enable(nand->pmecc, op);
	if (ret)
		dev_err(nc->dev,
			"Failed to enable ECC engine (err = %d)\n", ret);

	return ret;
}

static void atmel_nand_pmecc_disable(struct nand_chip *chip, bool raw)
{
	struct atmel_nand *nand = to_atmel_nand(chip);

	if (!raw)
		atmel_pmecc_disable(nand->pmecc);
}

static int atmel_nand_pmecc_generate_eccbytes(struct nand_chip *chip, bool raw)
{
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct atmel_nand_controller *nc;
	struct mtd_oob_region oobregion;
	void *eccbuf;
	int ret, i;

	nc = to_nand_controller(nand->controller);

	if (raw)
		return 0;

	ret = atmel_pmecc_wait_rdy(nand->pmecc);
	if (ret) {
		dev_err(nc->dev,
			"Failed to transfer NAND page data (err = %d)\n",
			ret);
		return ret;
	}

	mtd_ooblayout_ecc(mtd, 0, &oobregion);
	eccbuf = chip->oob_poi + oobregion.offset;

	for (i = 0; i < chip->ecc.steps; i++) {
		atmel_pmecc_get_generated_eccbytes(nand->pmecc, i,
						   eccbuf);
		eccbuf += chip->ecc.bytes;
	}

	return 0;
}

static int atmel_nand_pmecc_correct_data(struct nand_chip *chip, void *buf,
					 bool raw)
{
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct atmel_nand_controller *nc;
	struct mtd_oob_region oobregion;
	int ret, i, max_bitflips = 0;
	void *databuf, *eccbuf;

	nc = to_nand_controller(nand->controller);

	if (raw)
		return 0;

	ret = atmel_pmecc_wait_rdy(nand->pmecc);
	if (ret) {
		dev_err(nc->dev,
			"Failed to read NAND page data (err = %d)\n", ret);
		return ret;
	}

	mtd_ooblayout_ecc(mtd, 0, &oobregion);
	eccbuf = chip->oob_poi + oobregion.offset;
	databuf = buf;

	for (i = 0; i < chip->ecc.steps; i++) {
		ret = atmel_pmecc_correct_sector(nand->pmecc, i, databuf,
						 eccbuf);
		if (ret < 0 && !atmel_pmecc_correct_erased_chunks(nand->pmecc))
			ret = nand_check_erased_ecc_chunk(databuf,
							  chip->ecc.size,
							  eccbuf,
							  chip->ecc.bytes,
							  NULL, 0,
							  chip->ecc.strength);

		if (ret >= 0)
			max_bitflips = max(ret, max_bitflips);
		else
			mtd->ecc_stats.failed++;

		databuf += chip->ecc.size;
		eccbuf += chip->ecc.bytes;
	}

	return max_bitflips;
}

static int atmel_nand_pmecc_write_pg(struct nand_chip *chip, const u8 *buf,
				     bool oob_required, int page, bool raw)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct atmel_nand *nand = to_atmel_nand(chip);
	int ret;

	nand_prog_page_begin_op(chip, page, 0, NULL, 0);

	ret = atmel_nand_pmecc_enable(chip, NAND_ECC_WRITE, raw);
	if (ret)
		return ret;

	atmel_nand_write_buf(mtd, buf, mtd->writesize);

	ret = atmel_nand_pmecc_generate_eccbytes(chip, raw);
	if (ret) {
		atmel_pmecc_disable(nand->pmecc);
		return ret;
	}

	atmel_nand_pmecc_disable(chip, raw);

	atmel_nand_write_buf(mtd, chip->oob_poi, mtd->oobsize);

	return nand_prog_page_end_op(chip);
}

static int atmel_nand_pmecc_write_page(struct mtd_info *mtd,
				       struct nand_chip *chip, const u8 *buf,
				       int oob_required, int page)
{
	return atmel_nand_pmecc_write_pg(chip, buf, oob_required, page, false);
}

static int atmel_nand_pmecc_write_page_raw(struct mtd_info *mtd,
					   struct nand_chip *chip,
					   const u8 *buf, int oob_required,
					   int page)
{
	return atmel_nand_pmecc_write_pg(chip, buf, oob_required, page, true);
}

static int atmel_nand_pmecc_read_pg(struct nand_chip *chip, u8 *buf,
				    bool oob_required, int page, bool raw)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	int ret;

	nand_read_page_op(chip, page, 0, NULL, 0);

	ret = atmel_nand_pmecc_enable(chip, NAND_ECC_READ, raw);
	if (ret)
		return ret;

	atmel_nand_read_buf(mtd, buf, mtd->writesize);
	atmel_nand_read_buf(mtd, chip->oob_poi, mtd->oobsize);

	ret = atmel_nand_pmecc_correct_data(chip, buf, raw);

	atmel_nand_pmecc_disable(chip, raw);

	return ret;
}

static int atmel_nand_pmecc_read_page(struct mtd_info *mtd,
				      struct nand_chip *chip, u8 *buf,
				      int oob_required, int page)
{
	return atmel_nand_pmecc_read_pg(chip, buf, oob_required, page, false);
}

static int atmel_nand_pmecc_read_page_raw(struct mtd_info *mtd,
					  struct nand_chip *chip, u8 *buf,
					  int oob_required, int page)
{
	return atmel_nand_pmecc_read_pg(chip, buf, oob_required, page, true);
}

static int atmel_hsmc_nand_pmecc_write_pg(struct nand_chip *chip,
					  const u8 *buf, bool oob_required,
					  int page, bool raw)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct atmel_hsmc_nand_controller *nc;
	int ret, status;

	nc = to_hsmc_nand_controller(nand->controller);

	atmel_nfc_copy_to_sram(chip, buf, false);

	nc->op.cmds[0] = NAND_CMD_SEQIN;
	nc->op.ncmds = 1;
	atmel_nfc_set_op_addr(chip, page, 0x0);
	nc->op.cs = nand->activecs->id;
	nc->op.data = ATMEL_NFC_WRITE_DATA;

	ret = atmel_nand_pmecc_enable(chip, NAND_ECC_WRITE, raw);
	if (ret)
		return ret;

	ret = atmel_nfc_exec_op(nc, true);
	if (ret) {
		atmel_nand_pmecc_disable(chip, raw);
		dev_err(nc->base.dev,
			"Failed to transfer NAND page data (err = %d)\n",
			ret);
		return ret;
	}

	ret = atmel_nand_pmecc_generate_eccbytes(chip, raw);

	atmel_nand_pmecc_disable(chip, raw);

	if (ret)
		return ret;

	atmel_nand_write_buf(mtd, chip->oob_poi, mtd->oobsize);

	nc->op.cmds[0] = NAND_CMD_PAGEPROG;
	nc->op.ncmds = 1;
	nc->op.cs = nand->activecs->id;
	ret = atmel_nfc_exec_op(nc, true);
	if (ret)
		dev_err(nc->base.dev, "Failed to program NAND page (err = %d)\n",
			ret);

	status = chip->waitfunc(mtd, chip);
	if (status & NAND_STATUS_FAIL)
		return -EIO;

	return ret;
}

static int
atmel_hsmc_nand_pmecc_write_page(struct mtd_info *mtd, struct nand_chip *chip,
				 const u8 *buf, int oob_required,
				 int page)
{
	return atmel_hsmc_nand_pmecc_write_pg(chip, buf, oob_required, page,
					      false);
}

static int
atmel_hsmc_nand_pmecc_write_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
				     const u8 *buf,
				     int oob_required, int page)
{
	return atmel_hsmc_nand_pmecc_write_pg(chip, buf, oob_required, page,
					      true);
}

static int atmel_hsmc_nand_pmecc_read_pg(struct nand_chip *chip, u8 *buf,
					 bool oob_required, int page,
					 bool raw)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct atmel_hsmc_nand_controller *nc;
	int ret;

	nc = to_hsmc_nand_controller(nand->controller);

	/*
	 * Optimized read page accessors only work when the NAND R/B pin is
	 * connected to a native SoC R/B pin. If that's not the case, fallback
	 * to the non-optimized one.
	 */
	if (nand->activecs->rb.type != ATMEL_NAND_NATIVE_RB) {
		nand_read_page_op(chip, page, 0, NULL, 0);

		return atmel_nand_pmecc_read_pg(chip, buf, oob_required, page,
						raw);
	}

	nc->op.cmds[nc->op.ncmds++] = NAND_CMD_READ0;

	if (mtd->writesize > 512)
		nc->op.cmds[nc->op.ncmds++] = NAND_CMD_READSTART;

	atmel_nfc_set_op_addr(chip, page, 0x0);
	nc->op.cs = nand->activecs->id;
	nc->op.data = ATMEL_NFC_READ_DATA;

	ret = atmel_nand_pmecc_enable(chip, NAND_ECC_READ, raw);
	if (ret)
		return ret;

	ret = atmel_nfc_exec_op(nc, true);
	if (ret) {
		atmel_nand_pmecc_disable(chip, raw);
		dev_err(nc->base.dev,
			"Failed to load NAND page data (err = %d)\n",
			ret);
		return ret;
	}

	atmel_nfc_copy_from_sram(chip, buf, true);

	ret = atmel_nand_pmecc_correct_data(chip, buf, raw);

	atmel_nand_pmecc_disable(chip, raw);

	return ret;
}

static int atmel_hsmc_nand_pmecc_read_page(struct mtd_info *mtd,
					   struct nand_chip *chip, u8 *buf,
					   int oob_required, int page)
{
	return atmel_hsmc_nand_pmecc_read_pg(chip, buf, oob_required, page,
					     false);
}

static int atmel_hsmc_nand_pmecc_read_page_raw(struct mtd_info *mtd,
					       struct nand_chip *chip,
					       u8 *buf, int oob_required,
					       int page)
{
	return atmel_hsmc_nand_pmecc_read_pg(chip, buf, oob_required, page,
					     true);
}

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

const struct mtd_ooblayout_ops *nand_get_large_page_ooblayout(void)
{
	return &nand_ooblayout_lp_ops;
}

static int atmel_nand_pmecc_init(struct nand_chip *chip)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct atmel_nand_controller *nc;
	struct atmel_pmecc_user_req req;

	nc = to_nand_controller(nand->controller);

	if (!nc->pmecc) {
		dev_err(nc->dev, "HW ECC not supported\n");
		return -EOPNOTSUPP;
	}

	if (nc->caps->legacy_of_bindings) {
		u32 val;

		if (!ofnode_read_u32(nc->dev->node_, "atmel,pmecc-cap", &val))
			chip->ecc.strength = val;

		if (!ofnode_read_u32(nc->dev->node_,
				     "atmel,pmecc-sector-size",
				     &val))
			chip->ecc.size = val;
	}

	if (chip->ecc.options & NAND_ECC_MAXIMIZE)
		req.ecc.strength = ATMEL_PMECC_MAXIMIZE_ECC_STRENGTH;
	else if (chip->ecc.strength)
		req.ecc.strength = chip->ecc.strength;
	else
		req.ecc.strength = ATMEL_PMECC_MAXIMIZE_ECC_STRENGTH;

	if (chip->ecc.size)
		req.ecc.sectorsize = chip->ecc.size;
	else
		req.ecc.sectorsize = ATMEL_PMECC_SECTOR_SIZE_AUTO;

	req.pagesize = mtd->writesize;
	req.oobsize = mtd->oobsize;

	if (mtd->writesize <= 512) {
		req.ecc.bytes = 4;
		req.ecc.ooboffset = 0;
	} else {
		req.ecc.bytes = mtd->oobsize - 2;
		req.ecc.ooboffset = ATMEL_PMECC_OOBOFFSET_AUTO;
	}

	nand->pmecc = atmel_pmecc_create_user(nc->pmecc, &req);
	if (IS_ERR(nand->pmecc))
		return PTR_ERR(nand->pmecc);

	chip->ecc.algo = NAND_ECC_BCH;
	chip->ecc.size = req.ecc.sectorsize;
	chip->ecc.bytes = req.ecc.bytes / req.ecc.nsectors;
	chip->ecc.strength = req.ecc.strength;

	chip->options |= NAND_NO_SUBPAGE_WRITE;

	mtd_set_ooblayout(mtd, nand_get_large_page_ooblayout());
	pmecc_config_ecc_layout(&atmel_pmecc_oobinfo,
				mtd->oobsize,
				chip->ecc.bytes);
	chip->ecc.layout = &atmel_pmecc_oobinfo;

	return 0;
}

static int atmel_nand_ecc_init(struct nand_chip *chip)
{
	struct atmel_nand_controller *nc;
	struct atmel_nand *nand = to_atmel_nand(chip);
	int ret;

	nc = to_nand_controller(nand->controller);

	switch (chip->ecc.mode) {
	case NAND_ECC_NONE:
	case NAND_ECC_SOFT:
		/*
		 * Nothing to do, the core will initialize everything for us.
		 */
		break;

	case NAND_ECC_HW:
		ret = atmel_nand_pmecc_init(chip);
		if (ret)
			return ret;

		chip->ecc.read_page = atmel_nand_pmecc_read_page;
		chip->ecc.write_page = atmel_nand_pmecc_write_page;
		chip->ecc.read_page_raw = atmel_nand_pmecc_read_page_raw;
		chip->ecc.write_page_raw = atmel_nand_pmecc_write_page_raw;
		break;

	default:
		/* Other modes are not supported. */
		dev_err(nc->dev, "Unsupported ECC mode: %d\n",
			chip->ecc.mode);
		return -EOPNOTSUPP;
	}

	return 0;
}

static int atmel_hsmc_nand_ecc_init(struct nand_chip *chip)
{
	int ret;

	ret = atmel_nand_ecc_init(chip);
	if (ret)
		return ret;

	if (chip->ecc.mode != NAND_ECC_HW)
		return 0;

	/* Adjust the ECC operations for the HSMC IP. */
	chip->ecc.read_page = atmel_hsmc_nand_pmecc_read_page;
	chip->ecc.write_page = atmel_hsmc_nand_pmecc_write_page;
	chip->ecc.read_page_raw = atmel_hsmc_nand_pmecc_read_page_raw;
	chip->ecc.write_page_raw = atmel_hsmc_nand_pmecc_write_page_raw;

	return 0;
}

static int atmel_smc_nand_prepare_smcconf(struct atmel_nand *nand,
					  const struct nand_data_interface *conf,
					  struct atmel_smc_cs_conf *smcconf)
{
	u32 ncycles, totalcycles, timeps, mckperiodps;
	struct atmel_nand_controller *nc;
	int ret;

	nc = to_nand_controller(nand->controller);

	/* DDR interface not supported. */
	if (conf->type != NAND_SDR_IFACE)
		return -EOPNOTSUPP;

	/*
	 * tRC < 30ns implies EDO mode. This controller does not support this
	 * mode.
	 */
	if (conf->timings.sdr.tRC_min < 30000)
		return -EOPNOTSUPP;

	atmel_smc_cs_conf_init(smcconf);

	mckperiodps = NSEC_PER_SEC / clk_get_rate(nc->mck);
	mckperiodps *= 1000;

	/*
	 * Set write pulse timing. This one is easy to extract:
	 *
	 * NWE_PULSE = tWP
	 */
	ncycles = DIV_ROUND_UP(conf->timings.sdr.tWP_min, mckperiodps);
	totalcycles = ncycles;
	ret = atmel_smc_cs_conf_set_pulse(smcconf, ATMEL_SMC_NWE_SHIFT,
					  ncycles);
	if (ret)
		return ret;

	/*
	 * The write setup timing depends on the operation done on the NAND.
	 * All operations goes through the same data bus, but the operation
	 * type depends on the address we are writing to (ALE/CLE address
	 * lines).
	 * Since we have no way to differentiate the different operations at
	 * the SMC level, we must consider the worst case (the biggest setup
	 * time among all operation types):
	 *
	 * NWE_SETUP = max(tCLS, tCS, tALS, tDS) - NWE_PULSE
	 */
	timeps = max3(conf->timings.sdr.tCLS_min, conf->timings.sdr.tCS_min,
		      conf->timings.sdr.tALS_min);
	timeps = max(timeps, conf->timings.sdr.tDS_min);
	ncycles = DIV_ROUND_UP(timeps, mckperiodps);
	ncycles = ncycles > totalcycles ? ncycles - totalcycles : 0;
	totalcycles += ncycles;
	ret = atmel_smc_cs_conf_set_setup(smcconf, ATMEL_SMC_NWE_SHIFT,
					  ncycles);
	if (ret)
		return ret;

	/*
	 * As for the write setup timing, the write hold timing depends on the
	 * operation done on the NAND:
	 *
	 * NWE_HOLD = max(tCLH, tCH, tALH, tDH, tWH)
	 */
	timeps = max3(conf->timings.sdr.tCLH_min, conf->timings.sdr.tCH_min,
		      conf->timings.sdr.tALH_min);
	timeps = max3(timeps, conf->timings.sdr.tDH_min,
		      conf->timings.sdr.tWH_min);
	ncycles = DIV_ROUND_UP(timeps, mckperiodps);
	totalcycles += ncycles;

	/*
	 * The write cycle timing is directly matching tWC, but is also
	 * dependent on the other timings on the setup and hold timings we
	 * calculated earlier, which gives:
	 *
	 * NWE_CYCLE = max(tWC, NWE_SETUP + NWE_PULSE + NWE_HOLD)
	 */
	ncycles = DIV_ROUND_UP(conf->timings.sdr.tWC_min, mckperiodps);
	ncycles = max(totalcycles, ncycles);
	ret = atmel_smc_cs_conf_set_cycle(smcconf, ATMEL_SMC_NWE_SHIFT,
					  ncycles);
	if (ret)
		return ret;

	/*
	 * We don't want the CS line to be toggled between each byte/word
	 * transfer to the NAND. The only way to guarantee that is to have the
	 * NCS_{WR,RD}_{SETUP,HOLD} timings set to 0, which in turn means:
	 *
	 * NCS_WR_PULSE = NWE_CYCLE
	 */
	ret = atmel_smc_cs_conf_set_pulse(smcconf, ATMEL_SMC_NCS_WR_SHIFT,
					  ncycles);
	if (ret)
		return ret;

	/*
	 * As for the write setup timing, the read hold timing depends on the
	 * operation done on the NAND:
	 *
	 * NRD_HOLD = max(tREH, tRHOH)
	 */
	timeps = max(conf->timings.sdr.tREH_min, conf->timings.sdr.tRHOH_min);
	ncycles = DIV_ROUND_UP(timeps, mckperiodps);
	totalcycles = ncycles;

	/*
	 * TDF = tRHZ - NRD_HOLD
	 */
	ncycles = DIV_ROUND_UP(conf->timings.sdr.tRHZ_max, mckperiodps);
	ncycles -= totalcycles;

	/*
	 * In ONFI 4.0 specs, tRHZ has been increased to support EDO NANDs and
	 * we might end up with a config that does not fit in the TDF field.
	 * Just take the max value in this case and hope that the NAND is more
	 * tolerant than advertised.
	 */
	if (ncycles > ATMEL_SMC_MODE_TDF_MAX)
		ncycles = ATMEL_SMC_MODE_TDF_MAX;
	else if (ncycles < ATMEL_SMC_MODE_TDF_MIN)
		ncycles = ATMEL_SMC_MODE_TDF_MIN;

	smcconf->mode |= ATMEL_SMC_MODE_TDF(ncycles) |
			 ATMEL_SMC_MODE_TDFMODE_OPTIMIZED;

	/*
	 * Read pulse timing directly matches tRP:
	 *
	 * NRD_PULSE = tRP
	 */
	ncycles = DIV_ROUND_UP(conf->timings.sdr.tRP_min, mckperiodps);
	totalcycles += ncycles;
	ret = atmel_smc_cs_conf_set_pulse(smcconf, ATMEL_SMC_NRD_SHIFT,
					  ncycles);
	if (ret)
		return ret;

	/*
	 * The write cycle timing is directly matching tWC, but is also
	 * dependent on the setup and hold timings we calculated earlier,
	 * which gives:
	 *
	 * NRD_CYCLE = max(tRC, NRD_PULSE + NRD_HOLD)
	 *
	 * NRD_SETUP is always 0.
	 */
	ncycles = DIV_ROUND_UP(conf->timings.sdr.tRC_min, mckperiodps);
	ncycles = max(totalcycles, ncycles);
	ret = atmel_smc_cs_conf_set_cycle(smcconf, ATMEL_SMC_NRD_SHIFT,
					  ncycles);
	if (ret)
		return ret;

	/*
	 * We don't want the CS line to be toggled between each byte/word
	 * transfer from the NAND. The only way to guarantee that is to have
	 * the NCS_{WR,RD}_{SETUP,HOLD} timings set to 0, which in turn means:
	 *
	 * NCS_RD_PULSE = NRD_CYCLE
	 */
	ret = atmel_smc_cs_conf_set_pulse(smcconf, ATMEL_SMC_NCS_RD_SHIFT,
					  ncycles);
	if (ret)
		return ret;

	/* Txxx timings are directly matching tXXX ones. */
	ncycles = DIV_ROUND_UP(conf->timings.sdr.tCLR_min, mckperiodps);
	ret = atmel_smc_cs_conf_set_timing(smcconf,
					   ATMEL_HSMC_TIMINGS_TCLR_SHIFT,
					   ncycles);
	if (ret)
		return ret;

	ncycles = DIV_ROUND_UP(conf->timings.sdr.tADL_min, mckperiodps);
	ret = atmel_smc_cs_conf_set_timing(smcconf,
					   ATMEL_HSMC_TIMINGS_TADL_SHIFT,
					   ncycles);
	/*
	 * Version 4 of the ONFI spec mandates that tADL be at least 400
	 * nanoseconds, but, depending on the master clock rate, 400 ns may not
	 * fit in the tADL field of the SMC reg. We need to relax the check and
	 * accept the -ERANGE return code.
	 *
	 * Note that previous versions of the ONFI spec had a lower tADL_min
	 * (100 or 200 ns). It's not clear why this timing constraint got
	 * increased but it seems most NANDs are fine with values lower than
	 * 400ns, so we should be safe.
	 */
	if (ret && ret != -ERANGE)
		return ret;

	ncycles = DIV_ROUND_UP(conf->timings.sdr.tAR_min, mckperiodps);
	ret = atmel_smc_cs_conf_set_timing(smcconf,
					   ATMEL_HSMC_TIMINGS_TAR_SHIFT,
					   ncycles);
	if (ret)
		return ret;

	ncycles = DIV_ROUND_UP(conf->timings.sdr.tRR_min, mckperiodps);
	ret = atmel_smc_cs_conf_set_timing(smcconf,
					   ATMEL_HSMC_TIMINGS_TRR_SHIFT,
					   ncycles);
	if (ret)
		return ret;

	ncycles = DIV_ROUND_UP(conf->timings.sdr.tWB_max, mckperiodps);
	ret = atmel_smc_cs_conf_set_timing(smcconf,
					   ATMEL_HSMC_TIMINGS_TWB_SHIFT,
					   ncycles);
	if (ret)
		return ret;

	/* Attach the CS line to the NFC logic. */
	smcconf->timings |= ATMEL_HSMC_TIMINGS_NFSEL;

	/* Set the appropriate data bus width. */
	if (nand->base.options & NAND_BUSWIDTH_16)
		smcconf->mode |= ATMEL_SMC_MODE_DBW_16;

	/* Operate in NRD/NWE READ/WRITEMODE. */
	smcconf->mode |= ATMEL_SMC_MODE_READMODE_NRD |
			 ATMEL_SMC_MODE_WRITEMODE_NWE;

	return 0;
}

static int
atmel_smc_nand_setup_data_interface(struct atmel_nand *nand,
				    int csline,
				    const struct nand_data_interface *conf)
{
	struct atmel_nand_controller *nc;
	struct atmel_smc_cs_conf smcconf;
	struct atmel_nand_cs *cs;
	int ret;

	nc = to_nand_controller(nand->controller);

	ret = atmel_smc_nand_prepare_smcconf(nand, conf, &smcconf);
	if (ret)
		return ret;

	if (csline == NAND_DATA_IFACE_CHECK_ONLY)
		return 0;

	cs = &nand->cs[csline];
	cs->smcconf = smcconf;

	atmel_smc_cs_conf_apply(nc->smc, cs->id, &cs->smcconf);

	return 0;
}

static int
atmel_hsmc_nand_setup_data_interface(struct atmel_nand *nand,
				     int csline,
				     const struct nand_data_interface *conf)
{
	struct atmel_hsmc_nand_controller *nc;
	struct atmel_smc_cs_conf smcconf;
	struct atmel_nand_cs *cs;
	int ret;

	nc = to_hsmc_nand_controller(nand->controller);

	ret = atmel_smc_nand_prepare_smcconf(nand, conf, &smcconf);
	if (ret)
		return ret;

	if (csline == NAND_DATA_IFACE_CHECK_ONLY)
		return 0;

	cs = &nand->cs[csline];
	cs->smcconf = smcconf;

	if (cs->rb.type == ATMEL_NAND_NATIVE_RB)
		cs->smcconf.timings |= ATMEL_HSMC_TIMINGS_RBNSEL(cs->rb.id);

	atmel_hsmc_cs_conf_apply(nc->base.smc, nc->hsmc_layout, cs->id,
				 &cs->smcconf);

	return 0;
}

static int atmel_nand_setup_data_interface(struct mtd_info *mtd, int csline,
					   const struct nand_data_interface *conf)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct atmel_nand_controller *nc;

	nc = to_nand_controller(nand->controller);

	if (csline >= nand->numcs ||
	    (csline < 0 && csline != NAND_DATA_IFACE_CHECK_ONLY))
		return -EINVAL;

	return nc->caps->ops->setup_data_interface(nand, csline, conf);
}

#define NAND_KEEP_TIMINGS       0x00800000

static void atmel_nand_init(struct atmel_nand_controller *nc,
			    struct atmel_nand *nand)
{
	struct nand_chip *chip = &nand->base;
	struct mtd_info *mtd = nand_to_mtd(chip);

	mtd->dev->parent = nc->dev;
	nand->controller = &nc->base;

	chip->cmd_ctrl = atmel_nand_cmd_ctrl;
	chip->read_byte = atmel_nand_read_byte;
	chip->write_byte = atmel_nand_write_byte;
	chip->read_buf = atmel_nand_read_buf;
	chip->write_buf = atmel_nand_write_buf;
	chip->select_chip = atmel_nand_select_chip;
	chip->setup_data_interface = atmel_nand_setup_data_interface;

	if (!nc->mck || !nc->caps->ops->setup_data_interface)
		chip->options |= NAND_KEEP_TIMINGS;

	/* Some NANDs require a longer delay than the default one (20us). */
	chip->chip_delay = 40;

	/* Default to HW ECC if pmecc is available. */
	if (nc->pmecc)
		chip->ecc.mode = NAND_ECC_HW;
}

static void atmel_smc_nand_init(struct atmel_nand_controller *nc,
				struct atmel_nand *nand)
{
	struct atmel_smc_nand_controller *smc_nc;
	int i;

	atmel_nand_init(nc, nand);

	smc_nc = to_smc_nand_controller(nand->controller);
	if (!smc_nc->ebi_csa_regmap)
		return;

	/* Attach the CS to the NAND Flash logic. */
	for (i = 0; i < nand->numcs; i++)
		regmap_update_bits(smc_nc->ebi_csa_regmap,
				   smc_nc->ebi_csa->offs,
				   BIT(nand->cs[i].id), BIT(nand->cs[i].id));

	if (smc_nc->ebi_csa->nfd0_on_d16)
		regmap_update_bits(smc_nc->ebi_csa_regmap,
				   smc_nc->ebi_csa->offs,
				   smc_nc->ebi_csa->nfd0_on_d16,
				   smc_nc->ebi_csa->nfd0_on_d16);
}

static void atmel_hsmc_nand_init(struct atmel_nand_controller *nc,
				 struct atmel_nand *nand)
{
	struct nand_chip *chip = &nand->base;

	atmel_nand_init(nc, nand);

	/* Overload some methods for the HSMC controller. */
	chip->cmd_ctrl = atmel_hsmc_nand_cmd_ctrl;
	chip->select_chip = atmel_hsmc_nand_select_chip;
}

static int atmel_nand_controller_remove_nand(struct atmel_nand *nand)
{
	list_del(&nand->node);

	return 0;
}

static struct atmel_nand *atmel_nand_create(struct atmel_nand_controller *nc,
					    ofnode np,
					    int reg_cells)
{
	struct atmel_nand *nand;
	ofnode n;
	int numcs = 0;
	int ret, i;
	u32 val;
	fdt32_t faddr;
	phys_addr_t base;

	/* Count num of nand nodes */
	ofnode_for_each_subnode(n, ofnode_get_parent(np))
		numcs++;
	if (numcs < 1) {
		dev_err(nc->dev, "Missing or invalid reg property\n");
		return ERR_PTR(-EINVAL);
	}

	nand = devm_kzalloc(nc->dev,
			    sizeof(struct atmel_nand) +
			    (numcs * sizeof(struct atmel_nand_cs)),
			    GFP_KERNEL);
	if (!nand) {
		dev_err(nc->dev, "Failed to allocate NAND object\n");
		return ERR_PTR(-ENOMEM);
	}

	nand->numcs = numcs;

	gpio_request_by_name_nodev(np, "det-gpios", 0, &nand->cdgpio,
				   GPIOD_IS_IN);

	for (i = 0; i < numcs; i++) {
		ret = ofnode_read_u32(np, "reg", &val);
		if (ret) {
			dev_err(nc->dev, "Invalid reg property (err = %d)\n",
				ret);
			return ERR_PTR(ret);
		}
		nand->cs[i].id = val;

		/* Read base address */
		struct resource res;

		if (ofnode_read_resource(np, 0, &res)) {
			dev_err(nc->dev, "Unable to read resource\n");
			return ERR_PTR(-ENOMEM);
		}

		faddr = cpu_to_fdt32(val);
		base = ofnode_translate_address(np, &faddr);
		nand->cs[i].io.virt = (void *)base;

		if (!ofnode_read_u32(np, "atmel,rb", &val)) {
			if (val > ATMEL_NFC_MAX_RB_ID)
				return ERR_PTR(-EINVAL);

			nand->cs[i].rb.type = ATMEL_NAND_NATIVE_RB;
			nand->cs[i].rb.id = val;
		} else {
			ret = gpio_request_by_name_nodev(np, "rb-gpios", 0,
							 &nand->cs[i].rb.gpio,
							 GPIOD_IS_IN);
			if (ret && ret != -ENOENT)
				dev_err(nc->dev, "Failed to get R/B gpio (err = %d)\n", ret);
			if (!ret)
				nand->cs[i].rb.type = ATMEL_NAND_GPIO_RB;
		}

		gpio_request_by_name_nodev(np, "cs-gpios", 0,
					   &nand->cs[i].csgpio,
					   GPIOD_IS_OUT);
	}

	nand_set_flash_node(&nand->base, np);

	return nand;
}

static int nand_attach(struct nand_chip *chip)
{
	struct atmel_nand *nand = to_atmel_nand(chip);

	if (nand->controller->ops && nand->controller->ops->attach_chip)
		return nand->controller->ops->attach_chip(chip);

	return 0;
}

int atmel_nand_scan(struct mtd_info *mtd, int maxchips)
{
	int ret;

	ret = nand_scan_ident(mtd, maxchips, NULL);
	if (ret)
		return ret;

	ret = nand_attach(mtd_to_nand(mtd));
	if (ret)
		return ret;

	ret = nand_scan_tail(mtd);
	return ret;
}

static int
atmel_nand_controller_add_nand(struct atmel_nand_controller *nc,
			       struct atmel_nand *nand)
{
	struct nand_chip *chip = &nand->base;
	struct mtd_info *mtd = nand_to_mtd(chip);
	int ret;

	/* No card inserted, skip this NAND. */
	if (dm_gpio_is_valid(&nand->cdgpio) &&
	    dm_gpio_get_value(&nand->cdgpio)) {
		dev_info(nc->dev, "No SmartMedia card inserted.\n");
		return 0;
	}

	nc->caps->ops->nand_init(nc, nand);

	ret = atmel_nand_scan(mtd, nand->numcs);
	if (ret) {
		dev_err(nc->dev, "NAND scan failed: %d\n", ret);
		return ret;
	}

	ret = nand_register(0, mtd);
	if (ret) {
		dev_err(nc->dev, "nand register failed: %d\n", ret);
		return ret;
	}

	list_add_tail(&nand->node, &nc->chips);

	return 0;
}

static int
atmel_nand_controller_remove_nands(struct atmel_nand_controller *nc)
{
	struct atmel_nand *nand, *tmp;
	int ret;

	list_for_each_entry_safe(nand, tmp, &nc->chips, node) {
		ret = atmel_nand_controller_remove_nand(nand);
		if (ret)
			return ret;
	}

	return 0;
}

static int atmel_nand_controller_add_nands(struct atmel_nand_controller *nc)
{
	ofnode np;
	ofnode nand_np;
	int ret, reg_cells;
	u32 val;

	/* TODO:
	 * Add support for legacy nands
	 */

	np = nc->dev->node_;

	ret = ofnode_read_u32(np, "#address-cells", &val);
	if (ret) {
		dev_err(nc->dev, "missing #address-cells property\n");
		return ret;
	}

	reg_cells = val;

	ret = ofnode_read_u32(np, "#size-cells", &val);
	if (ret) {
		dev_err(nc->dev, "missing #size-cells property\n");
		return ret;
	}

	reg_cells += val;

	ofnode_for_each_subnode(nand_np, np) {
		struct atmel_nand *nand;

		nand = atmel_nand_create(nc, nand_np, reg_cells);
		if (IS_ERR(nand)) {
			ret = PTR_ERR(nand);
			goto err;
		}

		ret = atmel_nand_controller_add_nand(nc, nand);
		if (ret)
			goto err;
	}

	return 0;

err:
	atmel_nand_controller_remove_nands(nc);

	return ret;
}

static const struct atmel_smc_nand_ebi_csa_cfg at91sam9260_ebi_csa = {
	.offs = AT91SAM9260_MATRIX_EBICSA,
};

static const struct atmel_smc_nand_ebi_csa_cfg at91sam9261_ebi_csa = {
	.offs = AT91SAM9261_MATRIX_EBICSA,
};

static const struct atmel_smc_nand_ebi_csa_cfg at91sam9263_ebi_csa = {
	.offs = AT91SAM9263_MATRIX_EBI0CSA,
};

static const struct atmel_smc_nand_ebi_csa_cfg at91sam9rl_ebi_csa = {
	.offs = AT91SAM9RL_MATRIX_EBICSA,
};

static const struct atmel_smc_nand_ebi_csa_cfg at91sam9g45_ebi_csa = {
	.offs = AT91SAM9G45_MATRIX_EBICSA,
};

static const struct atmel_smc_nand_ebi_csa_cfg at91sam9n12_ebi_csa = {
	.offs = AT91SAM9N12_MATRIX_EBICSA,
};

static const struct atmel_smc_nand_ebi_csa_cfg at91sam9x5_ebi_csa = {
	.offs = AT91SAM9X5_MATRIX_EBICSA,
};

static const struct atmel_smc_nand_ebi_csa_cfg sam9x60_ebi_csa = {
	.offs = AT91_SFR_CCFG_EBICSA,
	.nfd0_on_d16 = AT91_SFR_CCFG_NFD0_ON_D16,
};

static const struct udevice_id atmel_ebi_csa_regmap_of_ids[] = {
	{
		.compatible = "atmel,at91sam9260-matrix",
		.data = (ulong)&at91sam9260_ebi_csa,
	},
	{
		.compatible = "atmel,at91sam9261-matrix",
		.data = (ulong)&at91sam9261_ebi_csa,
	},
	{
		.compatible = "atmel,at91sam9263-matrix",
		.data = (ulong)&at91sam9263_ebi_csa,
	},
	{
		.compatible = "atmel,at91sam9rl-matrix",
		.data = (ulong)&at91sam9rl_ebi_csa,
	},
	{
		.compatible = "atmel,at91sam9g45-matrix",
		.data = (ulong)&at91sam9g45_ebi_csa,
	},
	{
		.compatible = "atmel,at91sam9n12-matrix",
		.data = (ulong)&at91sam9n12_ebi_csa,
	},
	{
		.compatible = "atmel,at91sam9x5-matrix",
		.data = (ulong)&at91sam9x5_ebi_csa,
	},
	{
		.compatible = "microchip,sam9x60-sfr",
		.data = (ulong)&sam9x60_ebi_csa,
	},
	{ /* sentinel */ },
};

static int atmel_nand_attach_chip(struct nand_chip *chip)
{
	struct atmel_nand *nand = to_atmel_nand(chip);
	struct atmel_nand_controller *nc = to_nand_controller(nand->controller);
	struct mtd_info *mtd = nand_to_mtd(chip);
	int ret;

	ret = nc->caps->ops->ecc_init(chip);
	if (ret)
		return ret;

	if (nc->caps->legacy_of_bindings || !ofnode_valid(nc->dev->node_)) {
		/*
		 * We keep the MTD name unchanged to avoid breaking platforms
		 * where the MTD cmdline parser is used and the bootloader
		 * has not been updated to use the new naming scheme.
		 */
		mtd->name = "atmel_nand";
	} else if (!mtd->name) {
		/*
		 * If the new bindings are used and the bootloader has not been
		 * updated to pass a new mtdparts parameter on the cmdline, you
		 * should define the following property in your nand node:
		 *
		 *	label = "atmel_nand";
		 *
		 * This way, mtd->name will be set by the core when
		 * nand_set_flash_node() is called.
		 */
		sprintf(mtd->name, "%s:nand.%d", nc->dev->name, nand->cs[0].id);
	}

	return 0;
}

static const struct nand_controller_ops atmel_nand_controller_ops = {
	.attach_chip = atmel_nand_attach_chip,
};

static int
atmel_nand_controller_init(struct atmel_nand_controller *nc,
			   struct udevice *dev,
			   const struct atmel_nand_controller_caps *caps)
{
	struct ofnode_phandle_args args;
	int ret;

	nc->base.ops = &atmel_nand_controller_ops;
	INIT_LIST_HEAD(&nc->chips);
	nc->dev = dev;
	nc->caps = caps;

	nc->pmecc = devm_atmel_pmecc_get(dev);
	if (IS_ERR(nc->pmecc)) {
		ret = PTR_ERR(nc->pmecc);
		if (ret != -EPROBE_DEFER)
			dev_err(dev, "Could not get PMECC object (err = %d)\n",
				ret);
		return ret;
	}

	/* We do not retrieve the SMC syscon when parsing old DTs. */
	if (nc->caps->legacy_of_bindings)
		return 0;

	nc->mck = devm_kzalloc(dev, sizeof(nc->mck), GFP_KERNEL);
	if (!nc->mck)
		return -ENOMEM;

	clk_get_by_index(dev->parent, 0, nc->mck);
	if (IS_ERR(nc->mck)) {
		dev_err(dev, "Failed to retrieve MCK clk\n");
		return PTR_ERR(nc->mck);
	}

	ret = ofnode_parse_phandle_with_args(dev->parent->node_,
					     "atmel,smc", NULL, 0, 0, &args);
	if (ret) {
		dev_err(dev, "Missing or invalid atmel,smc property\n");
		return -EINVAL;
	}

	nc->smc = syscon_node_to_regmap(args.node);
	if (IS_ERR(nc->smc)) {
		ret = PTR_ERR(nc->smc);
		dev_err(dev, "Could not get SMC regmap (err = %d)\n", ret);
		return 0;
	}

	return 0;
}

static int
atmel_smc_nand_controller_init(struct atmel_smc_nand_controller *nc)
{
	struct udevice *dev = nc->base.dev;
	struct ofnode_phandle_args args;
	const struct udevice_id *match = NULL;
	const char *name;
	int ret;
	int len;
	int i;

	/* We do not retrieve the EBICSA regmap when parsing old DTs. */
	if (nc->base.caps->legacy_of_bindings)
		return 0;

	ret = ofnode_parse_phandle_with_args(dev->parent->node_,
					     nc->base.caps->ebi_csa_regmap_name,
					     NULL, 0, 0, &args);
	if (ret) {
		dev_err(dev, "Unable to read ebi csa regmap\n");
		return -EINVAL;
	}

	name = ofnode_get_property(args.node, "compatible", &len);

	for (i = 0; i < ARRAY_SIZE(atmel_ebi_csa_regmap_of_ids); i++) {
		if (!strcmp(name, atmel_ebi_csa_regmap_of_ids[i].compatible)) {
			match = &atmel_ebi_csa_regmap_of_ids[i];
			break;
		}
	}

	if (!match) {
		dev_err(dev, "Unable to find ebi csa conf");
		return -EINVAL;
	}
	nc->ebi_csa = (struct atmel_smc_nand_ebi_csa_cfg *)match->data;

	nc->ebi_csa_regmap = syscon_node_to_regmap(args.node);
	if (IS_ERR(nc->ebi_csa_regmap)) {
		ret = PTR_ERR(nc->ebi_csa_regmap);
		dev_err(dev, "Could not get EBICSA regmap (err = %d)\n", ret);
		return ret;
	}

	/* TODO:
	 * The at91sam9263 has 2 EBIs, if the NAND controller is under EBI1
	 * add 4 to ->ebi_csa->offs.
	 */

	return 0;
}

static int atmel_hsmc_nand_controller_init(struct atmel_hsmc_nand_controller *nc)
{
	struct udevice *dev = nc->base.dev;
	struct ofnode_phandle_args args;
	struct clk smc_clk;
	int ret;
	u32 addr;

	ret = ofnode_parse_phandle_with_args(dev->parent->node_,
					     "atmel,smc", NULL, 0, 0, &args);
	if (ret) {
		dev_err(dev, "Missing or invalid atmel,smc property\n");
		return -EINVAL;
	}

	nc->hsmc_layout = atmel_hsmc_get_reg_layout(args.node);
	if (IS_ERR(nc->hsmc_layout)) {
		dev_err(dev, "Could not get hsmc layout\n");
		return -EINVAL;
	}

	/* Enable smc clock */
	ret = clk_get_by_index_nodev(args.node, 0, &smc_clk);
	if (ret) {
		dev_err(dev, "Unable to get smc clock (err = %d)", ret);
		return ret;
	}

	ret = clk_prepare_enable(&smc_clk);
	if (ret)
		return ret;

	ret = ofnode_parse_phandle_with_args(dev->node_,
					     "atmel,nfc-io", NULL, 0, 0, &args);
	if (ret) {
		dev_err(dev, "Missing or invalid atmel,nfc-io property\n");
		return -EINVAL;
	}

	nc->io = syscon_node_to_regmap(args.node);
	if (IS_ERR(nc->io)) {
		ret = PTR_ERR(nc->io);
		dev_err(dev, "Could not get NFC IO regmap\n");
		return ret;
	}

	ret = ofnode_parse_phandle_with_args(dev->node_,
					     "atmel,nfc-sram", NULL, 0, 0, &args);
	if (ret) {
		dev_err(dev, "Missing or invalid atmel,nfc-sram property\n");
		return ret;
	}

	ret = ofnode_read_u32(args.node, "reg", &addr);
	if (ret) {
		dev_err(dev, "Could not read reg addr of nfc sram");
		return ret;
	}
	nc->sram.virt = (void *)addr;

	return 0;
}

static int
atmel_hsmc_nand_controller_remove(struct atmel_nand_controller *nc)
{
	struct atmel_hsmc_nand_controller *hsmc_nc;
	int ret;

	ret = atmel_nand_controller_remove_nands(nc);
	if (ret)
		return ret;

	hsmc_nc = container_of(nc, struct atmel_hsmc_nand_controller, base);

	if (hsmc_nc->clk)
		clk_disable_unprepare(hsmc_nc->clk);

	return 0;
}

static int
atmel_hsmc_nand_controller_probe(struct udevice *dev,
				 const struct atmel_nand_controller_caps *caps)
{
	struct atmel_hsmc_nand_controller *nc;
	int ret;

	nc = devm_kzalloc(dev, sizeof(*nc), GFP_KERNEL);
	if (!nc)
		return -ENOMEM;

	ret = atmel_nand_controller_init(&nc->base, dev, caps);
	if (ret)
		return ret;

	ret = atmel_hsmc_nand_controller_init(nc);
	if (ret)
		return ret;

	/* Make sure all irqs are masked before registering our IRQ handler. */
	regmap_write(nc->base.smc, ATMEL_HSMC_NFC_IDR, 0xffffffff);

	/* Initial NFC configuration. */
	regmap_write(nc->base.smc, ATMEL_HSMC_NFC_CFG,
		     ATMEL_HSMC_NFC_CFG_DTO_MAX);

	ret = atmel_nand_controller_add_nands(&nc->base);
	if (ret)
		goto err;

	return 0;

err:
	atmel_hsmc_nand_controller_remove(&nc->base);

	return ret;
}

static const struct atmel_nand_controller_ops atmel_hsmc_nc_ops = {
	.probe = atmel_hsmc_nand_controller_probe,
	.remove = atmel_hsmc_nand_controller_remove,
	.ecc_init = atmel_hsmc_nand_ecc_init,
	.nand_init = atmel_hsmc_nand_init,
	.setup_data_interface = atmel_hsmc_nand_setup_data_interface,
};

static const struct atmel_nand_controller_caps atmel_sama5_nc_caps = {
	.has_dma = true,
	.ale_offs = BIT(21),
	.cle_offs = BIT(22),
	.ops = &atmel_hsmc_nc_ops,
};

static int
atmel_smc_nand_controller_probe(struct udevice *dev,
				const struct atmel_nand_controller_caps *caps)
{
	struct atmel_smc_nand_controller *nc;
	int ret;

	nc = devm_kzalloc(dev, sizeof(*nc), GFP_KERNEL);
	if (!nc)
		return -ENOMEM;

	ret = atmel_nand_controller_init(&nc->base, dev, caps);
	if (ret)
		return ret;

	ret = atmel_smc_nand_controller_init(nc);
	if (ret)
		return ret;

	return atmel_nand_controller_add_nands(&nc->base);
}

static int
atmel_smc_nand_controller_remove(struct atmel_nand_controller *nc)
{
	int ret;

	ret = atmel_nand_controller_remove_nands(nc);
	if (ret)
		return ret;

	return 0;
}

/*
 * The SMC reg layout of at91rm9200 is completely different which prevents us
 * from re-using atmel_smc_nand_setup_data_interface() for the
 * ->setup_data_interface() hook.
 * At this point, there's no support for the at91rm9200 SMC IP, so we leave
 * ->setup_data_interface() unassigned.
 */
static const struct atmel_nand_controller_ops at91rm9200_nc_ops = {
	.probe = atmel_smc_nand_controller_probe,
	.remove = atmel_smc_nand_controller_remove,
	.ecc_init = atmel_nand_ecc_init,
	.nand_init = atmel_smc_nand_init,
};

static const struct atmel_nand_controller_caps atmel_rm9200_nc_caps = {
	.ale_offs = BIT(21),
	.cle_offs = BIT(22),
	.ebi_csa_regmap_name = "atmel,matrix",
	.ops = &at91rm9200_nc_ops,
};

static const struct atmel_nand_controller_ops atmel_smc_nc_ops = {
	.probe = atmel_smc_nand_controller_probe,
	.remove = atmel_smc_nand_controller_remove,
	.ecc_init = atmel_nand_ecc_init,
	.nand_init = atmel_smc_nand_init,
	.setup_data_interface = atmel_smc_nand_setup_data_interface,
};

static const struct atmel_nand_controller_caps atmel_sam9260_nc_caps = {
	.ale_offs = BIT(21),
	.cle_offs = BIT(22),
	.ebi_csa_regmap_name = "atmel,matrix",
	.ops = &atmel_smc_nc_ops,
};

static const struct atmel_nand_controller_caps atmel_sam9261_nc_caps = {
	.ale_offs = BIT(22),
	.cle_offs = BIT(21),
	.ebi_csa_regmap_name = "atmel,matrix",
	.ops = &atmel_smc_nc_ops,
};

static const struct atmel_nand_controller_caps atmel_sam9g45_nc_caps = {
	.has_dma = true,
	.ale_offs = BIT(21),
	.cle_offs = BIT(22),
	.ebi_csa_regmap_name = "atmel,matrix",
	.ops = &atmel_smc_nc_ops,
};

static const struct atmel_nand_controller_caps microchip_sam9x60_nc_caps = {
	.has_dma = true,
	.ale_offs = BIT(21),
	.cle_offs = BIT(22),
	.ebi_csa_regmap_name = "microchip,sfr",
	.ops = &atmel_smc_nc_ops,
};

/* Only used to parse old bindings. */
static const struct atmel_nand_controller_caps atmel_rm9200_nand_caps = {
	.ale_offs = BIT(21),
	.cle_offs = BIT(22),
	.ops = &atmel_smc_nc_ops,
	.legacy_of_bindings = true,
};

static const struct udevice_id atmel_nand_controller_of_ids[] = {
	{
		.compatible = "atmel,at91rm9200-nand-controller",
		.data = (ulong)&atmel_rm9200_nc_caps,
	},
	{
		.compatible = "atmel,at91sam9260-nand-controller",
		.data = (ulong)&atmel_sam9260_nc_caps,
	},
	{
		.compatible = "atmel,at91sam9261-nand-controller",
		.data = (ulong)&atmel_sam9261_nc_caps,
	},
	{
		.compatible = "atmel,at91sam9g45-nand-controller",
		.data = (ulong)&atmel_sam9g45_nc_caps,
	},
	{
		.compatible = "atmel,sama5d3-nand-controller",
		.data = (ulong)&atmel_sama5_nc_caps,
	},
	{
		.compatible = "microchip,sam9x60-nand-controller",
		.data = (ulong)&microchip_sam9x60_nc_caps,
	},
	/* Support for old/deprecated bindings: */
	{
		.compatible = "atmel,at91rm9200-nand",
		.data = (ulong)&atmel_rm9200_nand_caps,
	},
	{
		.compatible = "atmel,sama5d4-nand",
		.data = (ulong)&atmel_rm9200_nand_caps,
	},
	{
		.compatible = "atmel,sama5d2-nand",
		.data = (ulong)&atmel_rm9200_nand_caps,
	},
	{ /* sentinel */ },
};

static int atmel_nand_controller_probe(struct udevice *dev)
{
	const struct atmel_nand_controller_caps *caps;
	struct udevice *pmecc_dev;

	caps = (struct atmel_nand_controller_caps *)dev_get_driver_data(dev);
	if (!caps) {
		printf("Could not retrieve NFC caps\n");
		return -EINVAL;
	}

	/* Probe pmecc driver */
	if (uclass_get_device(UCLASS_MTD, 1, &pmecc_dev)) {
		printf("%s: get device fail\n", __func__);
		return -EINVAL;
	}

	return caps->ops->probe(dev, caps);
}

static int atmel_nand_controller_remove(struct udevice *dev)
{
	struct atmel_nand_controller *nc;

	nc = (struct atmel_nand_controller *)dev_get_driver_data(dev);

	return nc->caps->ops->remove(nc);
}

U_BOOT_DRIVER(atmel_nand_controller) = {
	.name = "atmel-nand-controller",
	.id = UCLASS_MTD,
	.of_match = atmel_nand_controller_of_ids,
	.probe = atmel_nand_controller_probe,
	.remove = atmel_nand_controller_remove,
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(atmel_nand_controller),
					  &dev);
	if (ret && ret != -ENODEV)
		printf("Failed to initialize NAND controller. (error %d)\n",
		       ret);
}
