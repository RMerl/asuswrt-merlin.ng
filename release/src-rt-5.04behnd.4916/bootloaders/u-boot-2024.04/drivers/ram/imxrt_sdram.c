// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019
 * Author(s): Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <init.h>
#include <log.h>
#include <ram.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/printk.h>

/* SDRAM Command Code */
#define SD_CC_ARD		0x0     /* Master Bus (AXI) command - Read */
#define SD_CC_AWR		0x1     /* Master Bus (AXI) command - Write */
#define SD_CC_IRD		0x8     /* IP command - Read */
#define SD_CC_IWR		0x9     /* IP command - Write */
#define SD_CC_IMS		0xA     /* IP command - Set Mode Register */
#define SD_CC_IACT		0xB     /* IP command - ACTIVE */
#define SD_CC_IAF		0xC     /* IP command - Auto Refresh */
#define SD_CC_ISF		0xD     /* IP Command - Self Refresh */
#define SD_CC_IPRE		0xE     /* IP command - Precharge */
#define SD_CC_IPREA		0xF     /* IP command - Precharge ALL */

#define SEMC_MCR_MDIS		BIT(1)
#define SEMC_MCR_DQSMD		BIT(2)

#define SEMC_INTR_IPCMDERR	BIT(1)
#define SEMC_INTR_IPCMDDONE	BIT(0)

#define SEMC_IPCMD_KEY		0xA55A0000

struct imxrt_semc_regs {
	/* 0x0 */
	u32 mcr;
	u32 iocr;
	u32 bmcr0;
	u32 bmcr1;
	u32 br[9];

	/* 0x34 */
	u32 res1;
	u32 inten;
	u32 intr;
	/* 0x40 */
	u32 sdramcr0;
	u32 sdramcr1;
	u32 sdramcr2;
	u32 sdramcr3;
	/* 0x50 */
	u32 nandcr0;
	u32 nandcr1;
	u32 nandcr2;
	u32 nandcr3;
	/* 0x60 */
	u32 norcr0;
	u32 norcr1;
	u32 norcr2;
	u32 norcr3;
	/* 0x70 */
	u32 sramcr0;
	u32 sramcr1;
	u32 sramcr2;
	u32 sramcr3;
	/* 0x80 */
	u32 dbicr0;
	u32 dbicr1;
	u32 res2[2];
	/* 0x90 */
	u32 ipcr0;
	u32 ipcr1;
	u32 ipcr2;
	u32 ipcmd;
	/* 0xA0 */
	u32 iptxdat;
	u32 res3[3];
	/* 0xB0 */
	u32 iprxdat;
	u32 res4[3];
	/* 0xC0 */
	u32 sts[16];
};

#if !defined(TARGET_IMXRT1170_EVK)
#define SEMC_IOCR_MUX_A8_SHIFT		0
#define SEMC_IOCR_MUX_CSX0_SHIFT	3
#define SEMC_IOCR_MUX_CSX1_SHIFT	6
#define SEMC_IOCR_MUX_CSX2_SHIFT	9
#define SEMC_IOCR_MUX_CSX3_SHIFT	12
#define SEMC_IOCR_MUX_RDY_SHIFT		15
#else
#define SEMC_IOCR_MUX_A8_SHIFT		0
#define SEMC_IOCR_MUX_CSX0_SHIFT	4
#define SEMC_IOCR_MUX_CSX1_SHIFT	8
#define SEMC_IOCR_MUX_CSX2_SHIFT	12
#define SEMC_IOCR_MUX_CSX3_SHIFT	16
#define SEMC_IOCR_MUX_RDY_SHIFT		20
#endif

struct imxrt_sdram_mux {
	u8 a8;
	u8 csx0;
	u8 csx1;
	u8 csx2;
	u8 csx3;
	u8 rdy;
};

#define SEMC_SDRAMCR0_PS_SHIFT		0
#define SEMC_SDRAMCR0_BL_SHIFT		4
#define SEMC_SDRAMCR0_COL_SHIFT		8
#define SEMC_SDRAMCR0_CL_SHIFT		10

struct imxrt_sdram_control {
	u8 memory_width;
	u8 burst_len;
	u8 no_columns;
	u8 cas_latency;
};

#define SEMC_SDRAMCR1_PRE2ACT_SHIFT	0
#define SEMC_SDRAMCR1_ACT2RW_SHIFT	4
#define SEMC_SDRAMCR1_RFRC_SHIFT	8
#define SEMC_SDRAMCR1_WRC_SHIFT		13
#define SEMC_SDRAMCR1_CKEOFF_SHIFT	16
#define SEMC_SDRAMCR1_ACT2PRE_SHIFT	20

#define SEMC_SDRAMCR2_SRRC_SHIFT	0
#define SEMC_SDRAMCR2_REF2REF_SHIFT	8
#define SEMC_SDRAMCR2_ACT2ACT_SHIFT	16
#define SEMC_SDRAMCR2_ITO_SHIFT		24

#define SEMC_SDRAMCR3_REN		BIT(0)
#define SEMC_SDRAMCR3_REBL_SHIFT	1
#define SEMC_SDRAMCR3_PRESCALE_SHIFT	8
#define SEMC_SDRAMCR3_RT_SHIFT		16
#define SEMC_SDRAMCR3_UT_SHIFT		24

struct imxrt_sdram_timing {
	u8 pre2act;
	u8 act2rw;
	u8 rfrc;
	u8 wrc;
	u8 ckeoff;
	u8 act2pre;

	u8 srrc;
	u8 ref2ref;
	u8 act2act;
	u8 ito;

	u8 rebl;
	u8 prescale;
	u8 rt;
	u8 ut;
};

enum imxrt_semc_bank {
	SDRAM_BANK1,
	SDRAM_BANK2,
	SDRAM_BANK3,
	SDRAM_BANK4,
	MAX_SDRAM_BANK,
};

#define SEMC_BR_VLD_MASK		1
#define SEMC_BR_MS_SHIFT		1

struct bank_params {
	enum imxrt_semc_bank target_bank;
	u32 base_address;
	u32 memory_size;
};

struct imxrt_sdram_params {
	struct imxrt_semc_regs *base;

	struct imxrt_sdram_mux *sdram_mux;
	struct imxrt_sdram_control *sdram_control;
	struct imxrt_sdram_timing *sdram_timing;

	struct bank_params bank_params[MAX_SDRAM_BANK];
	u8 no_sdram_banks;
};

static int imxrt_sdram_wait_ipcmd_done(struct imxrt_semc_regs *regs)
{
	do {
		readl(&regs->intr);

		if (regs->intr & SEMC_INTR_IPCMDDONE)
			return 0;
		if (regs->intr & SEMC_INTR_IPCMDERR)
			return -EIO;

		mdelay(50);
	} while (1);
}

static int imxrt_sdram_ipcmd(struct imxrt_semc_regs *regs, u32 mem_addr,
			     u32 ipcmd, u32 wd, u32 *rd)
{
	int ret;

	if (ipcmd == SD_CC_IWR || ipcmd == SD_CC_IMS)
		writel(wd, &regs->iptxdat);

	/* set slave address for every command as specified on RM */
	writel(mem_addr, &regs->ipcr0);

	/* execute command */
	writel(SEMC_IPCMD_KEY | ipcmd, &regs->ipcmd);

	ret = imxrt_sdram_wait_ipcmd_done(regs);
	if (ret < 0)
		return ret;

	if (ipcmd == SD_CC_IRD) {
		if (!rd)
			return -EINVAL;

		*rd = readl(&regs->iprxdat);
	}

	return 0;
}

int imxrt_sdram_init(struct udevice *dev)
{
	struct imxrt_sdram_params *params = dev_get_plat(dev);
	struct imxrt_sdram_mux *mux = params->sdram_mux;
	struct imxrt_sdram_control *ctrl = params->sdram_control;
	struct imxrt_sdram_timing *time = params->sdram_timing;
	struct imxrt_semc_regs *regs = params->base;
	struct bank_params *bank_params;
	u32 rd;
	int i;

	/* enable the SEMC controller */
	clrbits_le32(&regs->mcr, SEMC_MCR_MDIS);
	/* set DQS mode from DQS pad */
	setbits_le32(&regs->mcr, SEMC_MCR_DQSMD);

	for (i = 0, bank_params = params->bank_params;
		i < params->no_sdram_banks; bank_params++,
		i++)
		writel((bank_params->base_address & 0xfffff000)
		       | bank_params->memory_size << SEMC_BR_MS_SHIFT
		       | SEMC_BR_VLD_MASK,
		       &regs->br[bank_params->target_bank]);

	writel(mux->a8 << SEMC_IOCR_MUX_A8_SHIFT
		| mux->csx0 << SEMC_IOCR_MUX_CSX0_SHIFT
		| mux->csx1 << SEMC_IOCR_MUX_CSX1_SHIFT
		| mux->csx2 << SEMC_IOCR_MUX_CSX2_SHIFT
		| mux->csx3 << SEMC_IOCR_MUX_CSX3_SHIFT
		| mux->rdy << SEMC_IOCR_MUX_RDY_SHIFT,
		&regs->iocr);

	writel(ctrl->memory_width << SEMC_SDRAMCR0_PS_SHIFT
		| ctrl->burst_len << SEMC_SDRAMCR0_BL_SHIFT
		| ctrl->no_columns << SEMC_SDRAMCR0_COL_SHIFT
		| ctrl->cas_latency << SEMC_SDRAMCR0_CL_SHIFT,
		&regs->sdramcr0);

	writel(time->pre2act << SEMC_SDRAMCR1_PRE2ACT_SHIFT
		| time->act2rw << SEMC_SDRAMCR1_ACT2RW_SHIFT
		| time->rfrc << SEMC_SDRAMCR1_RFRC_SHIFT
		| time->wrc << SEMC_SDRAMCR1_WRC_SHIFT
		| time->ckeoff << SEMC_SDRAMCR1_CKEOFF_SHIFT
		| time->act2pre << SEMC_SDRAMCR1_ACT2PRE_SHIFT,
		&regs->sdramcr1);

	writel(time->srrc << SEMC_SDRAMCR2_SRRC_SHIFT
		| time->ref2ref << SEMC_SDRAMCR2_REF2REF_SHIFT
		| time->act2act << SEMC_SDRAMCR2_ACT2ACT_SHIFT
		| time->ito << SEMC_SDRAMCR2_ITO_SHIFT,
		&regs->sdramcr2);

	writel(time->rebl << SEMC_SDRAMCR3_REBL_SHIFT
		| time->prescale << SEMC_SDRAMCR3_PRESCALE_SHIFT
		| time->rt << SEMC_SDRAMCR3_RT_SHIFT
		| time->ut << SEMC_SDRAMCR3_UT_SHIFT
		| SEMC_SDRAMCR3_REN,
		&regs->sdramcr3);

	writel(2, &regs->ipcr1);

	for (i = 0, bank_params = params->bank_params;
		i < params->no_sdram_banks; bank_params++,
		i++) {
		mdelay(250);
		imxrt_sdram_ipcmd(regs, bank_params->base_address, SD_CC_IPREA,
				  0, &rd);
		imxrt_sdram_ipcmd(regs, bank_params->base_address, SD_CC_IAF,
				  0, &rd);
		imxrt_sdram_ipcmd(regs, bank_params->base_address, SD_CC_IAF,
				  0, &rd);
		imxrt_sdram_ipcmd(regs, bank_params->base_address, SD_CC_IMS,
				  ctrl->burst_len | (ctrl->cas_latency << 4),
				  &rd);
		mdelay(250);
	}

	return 0;
}

static int imxrt_semc_of_to_plat(struct udevice *dev)
{
	struct imxrt_sdram_params *params = dev_get_plat(dev);
	ofnode bank_node;
	u8 bank = 0;

	params->sdram_mux =
		(struct imxrt_sdram_mux *)
		 dev_read_u8_array_ptr(dev,
				       "fsl,sdram-mux",
				       sizeof(struct imxrt_sdram_mux));
	if (!params->sdram_mux) {
		pr_err("fsl,sdram-mux not found");
		return -EINVAL;
	}

	params->sdram_control =
		(struct imxrt_sdram_control *)
		 dev_read_u8_array_ptr(dev,
				       "fsl,sdram-control",
				       sizeof(struct imxrt_sdram_control));
	if (!params->sdram_control) {
		pr_err("fsl,sdram-control not found");
		return -EINVAL;
	}

	params->sdram_timing =
		(struct imxrt_sdram_timing *)
		 dev_read_u8_array_ptr(dev,
				       "fsl,sdram-timing",
				       sizeof(struct imxrt_sdram_timing));
	if (!params->sdram_timing) {
		pr_err("fsl,sdram-timing not found");
		return -EINVAL;
	}

	dev_for_each_subnode(bank_node, dev) {
		struct bank_params *bank_params;
		char *bank_name;
		int ret;

		/* extract the bank index from DT */
		bank_name = (char *)ofnode_get_name(bank_node);
		strsep(&bank_name, "@");
		if (!bank_name) {
			pr_err("missing sdram bank index");
			return -EINVAL;
		}

		bank_params = &params->bank_params[bank];
		strict_strtoul(bank_name, 10,
			       (unsigned long *)&bank_params->target_bank);
		if (bank_params->target_bank >= MAX_SDRAM_BANK) {
			pr_err("Found bank %d , but only bank 0,1,2,3 are supported",
			       bank_params->target_bank);
			return -EINVAL;
		}

		ret = ofnode_read_u32(bank_node,
				      "fsl,memory-size",
				      &bank_params->memory_size);
		if (ret < 0) {
			pr_err("fsl,memory-size not found");
			return -EINVAL;
		}

		ret = ofnode_read_u32(bank_node,
				      "fsl,base-address",
				      &bank_params->base_address);
		if (ret < 0) {
			pr_err("fsl,base-address not found");
			return -EINVAL;
		}

		debug("Found bank %s %u\n", bank_name,
		      bank_params->target_bank);
		bank++;
	}

	params->no_sdram_banks = bank;
	debug("%s, no of banks = %d\n", __func__, params->no_sdram_banks);

	return 0;
}

static int imxrt_semc_probe(struct udevice *dev)
{
	struct imxrt_sdram_params *params = dev_get_plat(dev);
	int ret;
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	params->base = (struct imxrt_semc_regs *)addr;

#ifdef CONFIG_CLK
	struct clk clk;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);

	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}
#endif
	ret = imxrt_sdram_init(dev);
	if (ret)
		return ret;

	return 0;
}

static int imxrt_semc_get_info(struct udevice *dev, struct ram_info *info)
{
	return 0;
}

static struct ram_ops imxrt_semc_ops = {
	.get_info = imxrt_semc_get_info,
};

static const struct udevice_id imxrt_semc_ids[] = {
	{ .compatible = "fsl,imxrt-semc", .data = 0 },
	{ }
};

U_BOOT_DRIVER(imxrt_semc) = {
	.name = "imxrt_semc",
	.id = UCLASS_RAM,
	.of_match = imxrt_semc_ids,
	.ops = &imxrt_semc_ops,
	.of_to_plat = imxrt_semc_of_to_plat,
	.probe = imxrt_semc_probe,
	.plat_auto	= sizeof(struct imxrt_sdram_params),
};
