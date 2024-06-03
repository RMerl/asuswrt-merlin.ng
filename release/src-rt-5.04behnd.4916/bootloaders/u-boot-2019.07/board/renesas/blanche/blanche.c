// SPDX-License-Identifier: GPL-2.0
/*
 * board/renesas/blanche/blanche.c
 *     This file is blanche board support.
 *
 * Copyright (C) 2016 Renesas Electronics Corporation
 */

#include <common.h>
#include <asm/arch/mmc.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/sh_sdhi.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/processor.h>
#include <dm.h>
#include <dm/platform_data/serial_sh.h>
#include <environment.h>
#include <i2c.h>
#include <linux/errno.h>
#include <malloc.h>
#include <miiphy.h>
#include <mmc.h>
#include <netdev.h>
#include "qos.h"

DECLARE_GLOBAL_DATA_PTR;

#define	CPG_PLL1CR	0xE6150028
#define	CPG_PLL3CR	0xE61500DC

#define TMU0_MSTP125	BIT(25)
#define QSPI_MSTP917	BIT(17)

struct reg_config {
	u16	off;
	u32	val;
};

static void blanche_init_sys(void)
{
	struct rcar_rwdt *rwdt = (struct rcar_rwdt *)RWDT_BASE;
	struct rcar_swdt *swdt = (struct rcar_swdt *)SWDT_BASE;
	u32 cpu_type;

	cpu_type = rmobile_get_cpu_type();
	if (cpu_type == 0x4A) {
		writel(0x4D000000, CPG_PLL1CR);
		writel(0x4F000000, CPG_PLL3CR);
	}

	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);
}

static void blanche_init_pfc(void)
{
	static const struct reg_config pfc_with_unlock[] = {
		{ 0x0004, 0x0bffffff },
		{ 0x0008, 0x002fffff },
		{ 0x0014, 0x00000fff },
		{ 0x0018, 0x00010fff },
		{ 0x001c, 0x00010fff },
		{ 0x0020, 0x00010fff },
		{ 0x0024, 0x00010fff },
		{ 0x0028, 0x00010fff },
		{ 0x002c, 0x04006000 },
		{ 0x0030, 0x303fefe0 },
		{ 0x0058, 0x0002000e },
	};

	static const struct reg_config pfc_without_unlock[] = {
		{ 0x0108, 0x00000000 },
		{ 0x010c, 0x0803FF40 },
		{ 0x0110, 0x0000FFFF },
		{ 0x0114, 0x00010FFF },
		{ 0x011c, 0x0001AFFF },
		{ 0x0124, 0x0001CFFF },
		{ 0x0128, 0xC0438001 },
		{ 0x012c, 0x0FC00007 },
	};

	static const u32 pfc_base = 0xe6060000;

	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(pfc_with_unlock); i++) {
		writel(~pfc_with_unlock[i].val, pfc_base);
		writel(pfc_with_unlock[i].val,
		       pfc_base | pfc_with_unlock[i].off);
	}

	for (i = 0; i < ARRAY_SIZE(pfc_without_unlock); i++)
		writel(pfc_without_unlock[i].val,
		       pfc_base | pfc_without_unlock[i].off);
}

static void blanche_init_lbsc(void)
{
	static const struct reg_config lbsc_config[] = {
		{ 0x00, 0x00000020 },
		{ 0x08, 0x00002020 },
		{ 0x30, 0x2a103320 },
		{ 0x38, 0x19102110 },
	};

	static const u32 lbsc_base = 0xfec00200;

	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(lbsc_config); i++) {
		writel(lbsc_config[i].val,
		       lbsc_base | lbsc_config[i].off);
		writel(lbsc_config[i].val,
		       lbsc_base | (lbsc_config[i].off + 4));
	}
}

#if defined(CONFIG_MTD_NOR_FLASH)
static void dbsc_wait(u16 reg)
{
	static const u32 dbsc3_0_base = DBSC3_0_BASE;

	while (!(readl(dbsc3_0_base + reg) & BIT(0)))
		;
}

static void blanche_init_dbsc(void)
{
	static const struct reg_config dbsc_config1[] = {
		{ 0x0280, 0x0000a55a },
		{ 0x0018, 0x21000000 },
		{ 0x0018, 0x11000000 },
		{ 0x0018, 0x10000000 },
		{ 0x0290, 0x00000001 },
		{ 0x02a0, 0x80000000 },
		{ 0x0290, 0x00000004 },
	};

	static const struct reg_config dbsc_config2[] = {
		{ 0x0290, 0x00000006 },
		{ 0x02a0, 0x0001c000 },
	};

	static const struct reg_config dbsc_config4[] = {
		{ 0x0290, 0x0000000f },
		{ 0x02a0, 0x00181ee4 },
		{ 0x0290, 0x00000010 },
		{ 0x02a0, 0xf00464db },
		{ 0x0290, 0x00000061 },
		{ 0x02a0, 0x0000008d },
		{ 0x0290, 0x00000001 },
		{ 0x02a0, 0x00000073 },
		{ 0x0020, 0x00000007 },
		{ 0x0024, 0x0f030a02 },
		{ 0x0030, 0x00000001 },
		{ 0x00b0, 0x00000000 },
		{ 0x0040, 0x0000000b },
		{ 0x0044, 0x00000008 },
		{ 0x0048, 0x00000000 },
		{ 0x0050, 0x0000000b },
		{ 0x0054, 0x000c000b },
		{ 0x0058, 0x00000027 },
		{ 0x005c, 0x0000001c },
		{ 0x0060, 0x00000006 },
		{ 0x0064, 0x00000020 },
		{ 0x0068, 0x00000008 },
		{ 0x006c, 0x0000000c },
		{ 0x0070, 0x00000009 },
		{ 0x0074, 0x00000012 },
		{ 0x0078, 0x000000d0 },
		{ 0x007c, 0x00140005 },
		{ 0x0080, 0x00050004 },
		{ 0x0084, 0x70233005 },
		{ 0x0088, 0x000c0000 },
		{ 0x008c, 0x00000300 },
		{ 0x0090, 0x00000040 },
		{ 0x0100, 0x00000001 },
		{ 0x00c0, 0x00020001 },
		{ 0x00c8, 0x20082004 },
		{ 0x0380, 0x00020002 },
		{ 0x0390, 0x0000001f },
	};

	static const struct reg_config dbsc_config5[] = {
		{ 0x0244, 0x00000011 },
		{ 0x0290, 0x00000003 },
		{ 0x02a0, 0x0300c4e1 },
		{ 0x0290, 0x00000023 },
		{ 0x02a0, 0x00fcdb60 },
		{ 0x0290, 0x00000011 },
		{ 0x02a0, 0x1000040b },
		{ 0x0290, 0x00000012 },
		{ 0x02a0, 0x9d9cbb66 },
		{ 0x0290, 0x00000013 },
		{ 0x02a0, 0x1a868400 },
		{ 0x0290, 0x00000014 },
		{ 0x02a0, 0x300214d8 },
		{ 0x0290, 0x00000015 },
		{ 0x02a0, 0x00000d70 },
		{ 0x0290, 0x00000016 },
		{ 0x02a0, 0x00000004 },
		{ 0x0290, 0x00000017 },
		{ 0x02a0, 0x00000018 },
		{ 0x0290, 0x0000001a },
		{ 0x02a0, 0x910035c7 },
		{ 0x0290, 0x00000004 },
	};

	static const struct reg_config dbsc_config6[] = {
		{ 0x0290, 0x00000001 },
		{ 0x02a0, 0x00000181 },
		{ 0x0018, 0x11000000 },
		{ 0x0290, 0x00000004 },
	};

	static const struct reg_config dbsc_config7[] = {
		{ 0x0290, 0x00000001 },
		{ 0x02a0, 0x0000fe01 },
		{ 0x0304, 0x00000000 },
		{ 0x00f4, 0x01004c20 },
		{ 0x00f8, 0x014000aa },
		{ 0x00e0, 0x00000140 },
		{ 0x00e4, 0x00081860 },
		{ 0x00e8, 0x00010000 },
		{ 0x0290, 0x00000004 },
	};

	static const struct reg_config dbsc_config8[] = {
		{ 0x0014, 0x00000001 },
		{ 0x0010, 0x00000001 },
		{ 0x0280, 0x00000000 },
	};

	static const u32 dbsc3_0_base = DBSC3_0_BASE;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(dbsc_config1); i++)
		writel(dbsc_config1[i].val, dbsc3_0_base | dbsc_config1[i].off);

	dbsc_wait(0x2a0);

	for (i = 0; i < ARRAY_SIZE(dbsc_config2); i++)
		writel(dbsc_config2[i].val, dbsc3_0_base | dbsc_config2[i].off);

	for (i = 0; i < ARRAY_SIZE(dbsc_config4); i++)
		writel(dbsc_config4[i].val, dbsc3_0_base | dbsc_config4[i].off);

	dbsc_wait(0x240);

	for (i = 0; i < ARRAY_SIZE(dbsc_config5); i++)
		writel(dbsc_config5[i].val, dbsc3_0_base | dbsc_config5[i].off);

	dbsc_wait(0x2a0);

	for (i = 0; i < ARRAY_SIZE(dbsc_config6); i++)
		writel(dbsc_config6[i].val, dbsc3_0_base | dbsc_config6[i].off);

	dbsc_wait(0x2a0);

	for (i = 0; i < ARRAY_SIZE(dbsc_config7); i++)
		writel(dbsc_config7[i].val, dbsc3_0_base | dbsc_config7[i].off);

	dbsc_wait(0x2a0);

	for (i = 0; i < ARRAY_SIZE(dbsc_config8); i++)
		writel(dbsc_config8[i].val, dbsc3_0_base | dbsc_config8[i].off);

}

static void s_init_wait(volatile unsigned int cnt)
{
	volatile u32 i = cnt * 0x10000;

	while (i-- > 0)
		;
}
#endif

void s_init(void)
{
	blanche_init_sys();
	qos_init();
	blanche_init_pfc();
	blanche_init_lbsc();
#if defined(CONFIG_MTD_NOR_FLASH)
	s_init_wait(10);
	blanche_init_dbsc();
#endif /* CONFIG_MTD_NOR_FLASH */
}

int board_early_init_f(void)
{
	/* TMU0 */
	mstp_clrbits_le32(MSTPSR1, SMSTPCR1, TMU0_MSTP125);
	/* QSPI */
	mstp_clrbits_le32(MSTPSR9, SMSTPCR9, QSPI_MSTP917);

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

/* Added for BLANCHE(R-CarV2H board) */
int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_SMC911X
	struct eth_device *dev;
	uchar eth_addr[6];

	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);

	if (!eth_env_get_enetaddr("ethaddr", eth_addr)) {
		dev = eth_get_dev_by_index(0);
		if (dev) {
			eth_env_set_enetaddr("ethaddr", dev->enetaddr);
		} else {
			printf("blanche: Couldn't get eth device\n");
			rc = -1;
		}
	}

#endif

	return rc;
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

void reset_cpu(ulong addr)
{
}
