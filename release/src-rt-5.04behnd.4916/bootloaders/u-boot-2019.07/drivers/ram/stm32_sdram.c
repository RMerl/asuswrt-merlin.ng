// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <ram.h>
#include <asm/io.h>

#define MEM_MODE_MASK	GENMASK(2, 0)
#define SWP_FMC_OFFSET 10
#define SWP_FMC_MASK	GENMASK(SWP_FMC_OFFSET+1, SWP_FMC_OFFSET)
#define NOT_FOUND	0xff

struct stm32_fmc_regs {
	/* 0x0 */
	u32 bcr1;	/* NOR/PSRAM Chip select control register 1 */
	u32 btr1;	/* SRAM/NOR-Flash Chip select timing register 1 */
	u32 bcr2;	/* NOR/PSRAM Chip select Control register 2 */
	u32 btr2;	/* SRAM/NOR-Flash Chip select timing register 2 */
	u32 bcr3;	/* NOR/PSRAMChip select Control register 3 */
	u32 btr3;	/* SRAM/NOR-Flash Chip select timing register 3 */
	u32 bcr4;	/* NOR/PSRAM Chip select Control register 4 */
	u32 btr4;	/* SRAM/NOR-Flash Chip select timing register 4 */
	u32 reserved1[24];

	/* 0x80 */
	u32 pcr;	/* NAND Flash control register */
	u32 sr;		/* FIFO status and interrupt register */
	u32 pmem;	/* Common memory space timing register */
	u32 patt;	/* Attribute memory space timing registers  */
	u32 reserved2[1];
	u32 eccr;	/* ECC result registers */
	u32 reserved3[27];

	/* 0x104 */
	u32 bwtr1;	/* SRAM/NOR-Flash write timing register 1 */
	u32 reserved4[1];
	u32 bwtr2;	/* SRAM/NOR-Flash write timing register 2 */
	u32 reserved5[1];
	u32 bwtr3;	/* SRAM/NOR-Flash write timing register 3 */
	u32 reserved6[1];
	u32 bwtr4;	/* SRAM/NOR-Flash write timing register 4 */
	u32 reserved7[8];

	/* 0x140 */
	u32 sdcr1;	/* SDRAM Control register 1 */
	u32 sdcr2;	/* SDRAM Control register 2 */
	u32 sdtr1;	/* SDRAM Timing register 1 */
	u32 sdtr2;	/* SDRAM Timing register 2 */
	u32 sdcmr;	/* SDRAM Mode register */
	u32 sdrtr;	/* SDRAM Refresh timing register */
	u32 sdsr;	/* SDRAM Status register */
};

/*
 * NOR/PSRAM Control register BCR1
 * FMC controller Enable, only availabe for H7
 */
#define FMC_BCR1_FMCEN		BIT(31)

/* Control register SDCR */
#define FMC_SDCR_RPIPE_SHIFT	13	/* RPIPE bit shift */
#define FMC_SDCR_RBURST_SHIFT	12	/* RBURST bit shift */
#define FMC_SDCR_SDCLK_SHIFT	10	/* SDRAM clock divisor shift */
#define FMC_SDCR_WP_SHIFT	9	/* Write protection shift */
#define FMC_SDCR_CAS_SHIFT	7	/* CAS latency shift */
#define FMC_SDCR_NB_SHIFT	6	/* Number of banks shift */
#define FMC_SDCR_MWID_SHIFT	4	/* Memory width shift */
#define FMC_SDCR_NR_SHIFT	2	/* Number of row address bits shift */
#define FMC_SDCR_NC_SHIFT	0	/* Number of col address bits shift */

/* Timings register SDTR */
#define FMC_SDTR_TMRD_SHIFT	0	/* Load mode register to active */
#define FMC_SDTR_TXSR_SHIFT	4	/* Exit self-refresh time */
#define FMC_SDTR_TRAS_SHIFT	8	/* Self-refresh time */
#define FMC_SDTR_TRC_SHIFT	12	/* Row cycle delay */
#define FMC_SDTR_TWR_SHIFT	16	/* Recovery delay */
#define FMC_SDTR_TRP_SHIFT	20	/* Row precharge delay */
#define FMC_SDTR_TRCD_SHIFT	24	/* Row-to-column delay */

#define FMC_SDCMR_NRFS_SHIFT	5

#define FMC_SDCMR_MODE_NORMAL		0
#define FMC_SDCMR_MODE_START_CLOCK	1
#define FMC_SDCMR_MODE_PRECHARGE	2
#define FMC_SDCMR_MODE_AUTOREFRESH	3
#define FMC_SDCMR_MODE_WRITE_MODE	4
#define FMC_SDCMR_MODE_SELFREFRESH	5
#define FMC_SDCMR_MODE_POWERDOWN	6

#define FMC_SDCMR_BANK_1		BIT(4)
#define FMC_SDCMR_BANK_2		BIT(3)

#define FMC_SDCMR_MODE_REGISTER_SHIFT	9

#define FMC_SDSR_BUSY			BIT(5)

#define FMC_BUSY_WAIT(regs)	do { \
		__asm__ __volatile__ ("dsb" : : : "memory"); \
		while (regs->sdsr & FMC_SDSR_BUSY) \
			; \
	} while (0)

struct stm32_sdram_control {
	u8 no_columns;
	u8 no_rows;
	u8 memory_width;
	u8 no_banks;
	u8 cas_latency;
	u8 sdclk;
	u8 rd_burst;
	u8 rd_pipe_delay;
};

struct stm32_sdram_timing {
	u8 tmrd;
	u8 txsr;
	u8 tras;
	u8 trc;
	u8 trp;
	u8 twr;
	u8 trcd;
};
enum stm32_fmc_bank {
	SDRAM_BANK1,
	SDRAM_BANK2,
	MAX_SDRAM_BANK,
};

enum stm32_fmc_family {
	STM32F7_FMC,
	STM32H7_FMC,
};

struct bank_params {
	struct stm32_sdram_control *sdram_control;
	struct stm32_sdram_timing *sdram_timing;
	u32 sdram_ref_count;
	enum stm32_fmc_bank target_bank;
};

struct stm32_sdram_params {
	struct stm32_fmc_regs *base;
	u8 no_sdram_banks;
	struct bank_params bank_params[MAX_SDRAM_BANK];
	enum stm32_fmc_family family;
};

#define SDRAM_MODE_BL_SHIFT	0
#define SDRAM_MODE_CAS_SHIFT	4
#define SDRAM_MODE_BL		0

int stm32_sdram_init(struct udevice *dev)
{
	struct stm32_sdram_params *params = dev_get_platdata(dev);
	struct stm32_sdram_control *control;
	struct stm32_sdram_timing *timing;
	struct stm32_fmc_regs *regs = params->base;
	enum stm32_fmc_bank target_bank;
	u32 ctb; /* SDCMR register: Command Target Bank */
	u32 ref_count;
	u8 i;

	/* disable the FMC controller */
	if (params->family == STM32H7_FMC)
		clrbits_le32(&regs->bcr1, FMC_BCR1_FMCEN);

	for (i = 0; i < params->no_sdram_banks; i++) {
		control = params->bank_params[i].sdram_control;
		timing = params->bank_params[i].sdram_timing;
		target_bank = params->bank_params[i].target_bank;
		ref_count = params->bank_params[i].sdram_ref_count;

		writel(control->sdclk << FMC_SDCR_SDCLK_SHIFT
			| control->cas_latency << FMC_SDCR_CAS_SHIFT
			| control->no_banks << FMC_SDCR_NB_SHIFT
			| control->memory_width << FMC_SDCR_MWID_SHIFT
			| control->no_rows << FMC_SDCR_NR_SHIFT
			| control->no_columns << FMC_SDCR_NC_SHIFT
			| control->rd_pipe_delay << FMC_SDCR_RPIPE_SHIFT
			| control->rd_burst << FMC_SDCR_RBURST_SHIFT,
			&regs->sdcr1);

		if (target_bank == SDRAM_BANK2)
			writel(control->cas_latency << FMC_SDCR_CAS_SHIFT
				| control->no_banks << FMC_SDCR_NB_SHIFT
				| control->memory_width << FMC_SDCR_MWID_SHIFT
				| control->no_rows << FMC_SDCR_NR_SHIFT
				| control->no_columns << FMC_SDCR_NC_SHIFT,
				&regs->sdcr2);

		writel(timing->trcd << FMC_SDTR_TRCD_SHIFT
			| timing->trp << FMC_SDTR_TRP_SHIFT
			| timing->twr << FMC_SDTR_TWR_SHIFT
			| timing->trc << FMC_SDTR_TRC_SHIFT
			| timing->tras << FMC_SDTR_TRAS_SHIFT
			| timing->txsr << FMC_SDTR_TXSR_SHIFT
			| timing->tmrd << FMC_SDTR_TMRD_SHIFT,
			&regs->sdtr1);

		if (target_bank == SDRAM_BANK2)
			writel(timing->trcd << FMC_SDTR_TRCD_SHIFT
				| timing->trp << FMC_SDTR_TRP_SHIFT
				| timing->twr << FMC_SDTR_TWR_SHIFT
				| timing->trc << FMC_SDTR_TRC_SHIFT
				| timing->tras << FMC_SDTR_TRAS_SHIFT
				| timing->txsr << FMC_SDTR_TXSR_SHIFT
				| timing->tmrd << FMC_SDTR_TMRD_SHIFT,
				&regs->sdtr2);

		if (target_bank == SDRAM_BANK1)
			ctb = FMC_SDCMR_BANK_1;
		else
			ctb = FMC_SDCMR_BANK_2;

		writel(ctb | FMC_SDCMR_MODE_START_CLOCK, &regs->sdcmr);
		udelay(200);	/* 200 us delay, page 10, "Power-Up" */
		FMC_BUSY_WAIT(regs);

		writel(ctb | FMC_SDCMR_MODE_PRECHARGE, &regs->sdcmr);
		udelay(100);
		FMC_BUSY_WAIT(regs);

		writel((ctb | FMC_SDCMR_MODE_AUTOREFRESH | 7 << FMC_SDCMR_NRFS_SHIFT),
		       &regs->sdcmr);
		udelay(100);
		FMC_BUSY_WAIT(regs);

		writel(ctb | (SDRAM_MODE_BL << SDRAM_MODE_BL_SHIFT
		       | control->cas_latency << SDRAM_MODE_CAS_SHIFT)
		       << FMC_SDCMR_MODE_REGISTER_SHIFT | FMC_SDCMR_MODE_WRITE_MODE,
		       &regs->sdcmr);
		udelay(100);
		FMC_BUSY_WAIT(regs);

		writel(ctb | FMC_SDCMR_MODE_NORMAL, &regs->sdcmr);
		FMC_BUSY_WAIT(regs);

		/* Refresh timer */
		writel(ref_count << 1, &regs->sdrtr);
	}

	/* enable the FMC controller */
	if (params->family == STM32H7_FMC)
		setbits_le32(&regs->bcr1, FMC_BCR1_FMCEN);

	return 0;
}

static int stm32_fmc_ofdata_to_platdata(struct udevice *dev)
{
	struct stm32_sdram_params *params = dev_get_platdata(dev);
	struct bank_params *bank_params;
	struct ofnode_phandle_args args;
	u32 *syscfg_base;
	u32 mem_remap;
	u32 swp_fmc;
	ofnode bank_node;
	char *bank_name;
	u8 bank = 0;
	int ret;

	ret = dev_read_phandle_with_args(dev, "st,syscfg", NULL, 0, 0,
						 &args);
	if (ret) {
		dev_dbg(dev, "%s: can't find syscon device (%d)\n", __func__, ret);
	} else {
		syscfg_base = (u32 *)ofnode_get_addr(args.node);

		mem_remap = dev_read_u32_default(dev, "st,mem_remap", NOT_FOUND);
		if (mem_remap != NOT_FOUND) {
			/* set memory mapping selection */
			clrsetbits_le32(syscfg_base, MEM_MODE_MASK, mem_remap);
		} else {
			dev_dbg(dev, "%s: cannot find st,mem_remap property\n", __func__);
		}
		
		swp_fmc = dev_read_u32_default(dev, "st,swp_fmc", NOT_FOUND);
		if (swp_fmc != NOT_FOUND) {
			/* set fmc swapping selection */
			clrsetbits_le32(syscfg_base, SWP_FMC_MASK, swp_fmc << SWP_FMC_OFFSET);
		} else {
			dev_dbg(dev, "%s: cannot find st,swp_fmc property\n", __func__);
		}

		dev_dbg(dev, "syscfg %x = %x\n", (u32)syscfg_base, *syscfg_base);
	}

	dev_for_each_subnode(bank_node, dev) {
		/* extract the bank index from DT */
		bank_name = (char *)ofnode_get_name(bank_node);
		strsep(&bank_name, "@");
		if (!bank_name) {
			pr_err("missing sdram bank index");
			return -EINVAL;
		}

		bank_params = &params->bank_params[bank];
		strict_strtoul(bank_name, 10,
			       (long unsigned int *)&bank_params->target_bank);

		if (bank_params->target_bank >= MAX_SDRAM_BANK) {
			pr_err("Found bank %d , but only bank 0 and 1 are supported",
			      bank_params->target_bank);
			return -EINVAL;
		}

		debug("Find bank %s %u\n", bank_name, bank_params->target_bank);

		params->bank_params[bank].sdram_control =
			(struct stm32_sdram_control *)
			 ofnode_read_u8_array_ptr(bank_node,
						  "st,sdram-control",
						  sizeof(struct stm32_sdram_control));

		if (!params->bank_params[bank].sdram_control) {
			pr_err("st,sdram-control not found for %s",
			      ofnode_get_name(bank_node));
			return -EINVAL;
		}


		params->bank_params[bank].sdram_timing =
			(struct stm32_sdram_timing *)
			 ofnode_read_u8_array_ptr(bank_node,
						  "st,sdram-timing",
						  sizeof(struct stm32_sdram_timing));

		if (!params->bank_params[bank].sdram_timing) {
			pr_err("st,sdram-timing not found for %s",
			      ofnode_get_name(bank_node));
			return -EINVAL;
		}


		bank_params->sdram_ref_count = ofnode_read_u32_default(bank_node,
						"st,sdram-refcount", 8196);
		bank++;
	}

	params->no_sdram_banks = bank;
	debug("%s, no of banks = %d\n", __func__, params->no_sdram_banks);

	return 0;
}

static int stm32_fmc_probe(struct udevice *dev)
{
	struct stm32_sdram_params *params = dev_get_platdata(dev);
	int ret;
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	params->base = (struct stm32_fmc_regs *)addr;
	params->family = dev_get_driver_data(dev);

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
	ret = stm32_sdram_init(dev);
	if (ret)
		return ret;

	return 0;
}

static int stm32_fmc_get_info(struct udevice *dev, struct ram_info *info)
{
	return 0;
}

static struct ram_ops stm32_fmc_ops = {
	.get_info = stm32_fmc_get_info,
};

static const struct udevice_id stm32_fmc_ids[] = {
	{ .compatible = "st,stm32-fmc", .data = STM32F7_FMC },
	{ .compatible = "st,stm32h7-fmc", .data = STM32H7_FMC },
	{ }
};

U_BOOT_DRIVER(stm32_fmc) = {
	.name = "stm32_fmc",
	.id = UCLASS_RAM,
	.of_match = stm32_fmc_ids,
	.ops = &stm32_fmc_ops,
	.ofdata_to_platdata = stm32_fmc_ofdata_to_platdata,
	.probe = stm32_fmc_probe,
	.platdata_auto_alloc_size = sizeof(struct stm32_sdram_params),
};
