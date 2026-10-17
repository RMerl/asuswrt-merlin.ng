// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <command.h>
#include <stdio.h>
#include <linux/io.h>
#include <linux/printk.h>
#include <linux/sizes.h>

#include "../soc-info.h"
#include "ddrmphy-regs.h"

/* Select either decimal or hexadecimal */
#if 1
#define PRINTF_FORMAT "%2d"
#else
#define PRINTF_FORMAT "%02x"
#endif
/* field separator */
#define FS "   "

#define ptr_to_uint(p)	((unsigned int)(unsigned long)(p))

#define UNIPHIER_MAX_NR_DDRMPHY		3

struct uniphier_ddrmphy_param {
	unsigned int soc_id;
	unsigned int nr_phy;
	struct {
		resource_size_t base;
		unsigned int nr_zq;
		unsigned int nr_dx;
	} phy[UNIPHIER_MAX_NR_DDRMPHY];
};

static const struct uniphier_ddrmphy_param uniphier_ddrmphy_param[] = {
	{
		.soc_id = UNIPHIER_PXS2_ID,
		.nr_phy = 3,
		.phy = {
			{ .base = 0x5b830000, .nr_zq = 3, .nr_dx = 4, },
			{ .base = 0x5ba30000, .nr_zq = 3, .nr_dx = 4, },
			{ .base = 0x5bc30000, .nr_zq = 2, .nr_dx = 2, },
		},
	},
	{
		.soc_id = UNIPHIER_LD6B_ID,
		.nr_phy = 3,
		.phy = {
			{ .base = 0x5b830000, .nr_zq = 3, .nr_dx = 4, },
			{ .base = 0x5ba30000, .nr_zq = 3, .nr_dx = 4, },
			{ .base = 0x5bc30000, .nr_zq = 2, .nr_dx = 2, },
		},
	},
};
UNIPHIER_DEFINE_SOCDATA_FUNC(uniphier_get_ddrmphy_param, uniphier_ddrmphy_param)

static void print_bdl(void __iomem *reg, int n)
{
	u32 val = readl(reg);
	int i;

	for (i = 0; i < n; i++)
		printf(FS PRINTF_FORMAT, (val >> i * 8) & 0x1f);
}

static void dump_loop(const struct uniphier_ddrmphy_param *param,
		      void (*callback)(void __iomem *))
{
	void __iomem *phy_base, *dx_base;
	int phy, dx;

	for (phy = 0; phy < param->nr_phy; phy++) {
		phy_base = ioremap(param->phy[phy].base, SZ_4K);
		dx_base = phy_base + MPHY_DX_BASE;

		for (dx = 0; dx < param->phy[phy].nr_dx; dx++) {
			printf("PHY%dDX%d:", phy, dx);
			(*callback)(dx_base);
			dx_base += MPHY_DX_STRIDE;
			printf("\n");
		}

		iounmap(phy_base);
	}
}

static void zq_dump(const struct uniphier_ddrmphy_param *param)
{
	void __iomem *phy_base, *zq_base;
	u32 val;
	int phy, zq, i;

	printf("\n--- Impedance Data ---\n");
	printf("           ZPD  ZPU  OPD  OPU  ZDV  ODV\n");

	for (phy = 0; phy < param->nr_phy; phy++) {
		phy_base = ioremap(param->phy[phy].base, SZ_4K);
		zq_base = phy_base + MPHY_ZQ_BASE;

		for (zq = 0; zq < param->phy[phy].nr_zq; zq++) {
			printf("PHY%dZQ%d:", phy, zq);

			val = readl(zq_base + MPHY_ZQ_DR);
			for (i = 0; i < 4; i++) {
				printf(FS PRINTF_FORMAT, val & 0x7f);
				val >>= 7;
			}

			val = readl(zq_base + MPHY_ZQ_PR);
			for (i = 0; i < 2; i++) {
				printf(FS PRINTF_FORMAT, val & 0xf);
				val >>= 4;
			}

			zq_base += MPHY_ZQ_STRIDE;
			printf("\n");
		}

		iounmap(phy_base);
	}
}

static void __wbdl_dump(void __iomem *dx_base)
{
	print_bdl(dx_base + MPHY_DX_BDLR0, 4);
	print_bdl(dx_base + MPHY_DX_BDLR1, 4);
	print_bdl(dx_base + MPHY_DX_BDLR2, 2);

	printf(FS "(+" PRINTF_FORMAT ")",
	       readl(dx_base + MPHY_DX_LCDLR1) & 0xff);
}

static void wbdl_dump(const struct uniphier_ddrmphy_param *param)
{
	printf("\n--- Write Bit Delay Line ---\n");
	printf("           DQ0  DQ1  DQ2  DQ3  DQ4  DQ5  DQ6  DQ7   DM  DQS  (WDQD)\n");

	dump_loop(param, &__wbdl_dump);
}

static void __rbdl_dump(void __iomem *dx_base)
{
	print_bdl(dx_base + MPHY_DX_BDLR3, 4);
	print_bdl(dx_base + MPHY_DX_BDLR4, 4);
	print_bdl(dx_base + MPHY_DX_BDLR5, 1);

	printf(FS "(+" PRINTF_FORMAT ")",
	       (readl(dx_base + MPHY_DX_LCDLR1) >> 8) & 0xff);

	printf(FS "(+" PRINTF_FORMAT ")",
	       (readl(dx_base + MPHY_DX_LCDLR1) >> 16) & 0xff);
}

static void rbdl_dump(const struct uniphier_ddrmphy_param *param)
{
	printf("\n--- Read Bit Delay Line ---\n");
	printf("           DQ0  DQ1  DQ2  DQ3  DQ4  DQ5  DQ6  DQ7   DM  (RDQSD) (RDQSND)\n");

	dump_loop(param, &__rbdl_dump);
}

static void __wld_dump(void __iomem *dx_base)
{
	int rank;
	u32 lcdlr0 = readl(dx_base + MPHY_DX_LCDLR0);
	u32 gtr = readl(dx_base + MPHY_DX_GTR);

	for (rank = 0; rank < 4; rank++) {
		u32 wld = (lcdlr0 >> (8 * rank)) & 0xff; /* Delay */
		u32 wlsl = (gtr >> (12 + 2 * rank)) & 0x3; /* System Latency */

		printf(FS PRINTF_FORMAT "%sT", wld,
		       wlsl == 0 ? "-1" : wlsl == 1 ? "+0" : "+1");
	}
}

static void wld_dump(const struct uniphier_ddrmphy_param *param)
{
	printf("\n--- Write Leveling Delay ---\n");
	printf("           Rank0   Rank1   Rank2   Rank3\n");

	dump_loop(param, &__wld_dump);
}

static void __dqsgd_dump(void __iomem *dx_base)
{
	int rank;
	u32 lcdlr2 = readl(dx_base + MPHY_DX_LCDLR2);
	u32 gtr = readl(dx_base + MPHY_DX_GTR);

	for (rank = 0; rank < 4; rank++) {
		u32 dqsgd = (lcdlr2 >> (8 * rank)) & 0xff; /* Delay */
		u32 dgsl = (gtr >> (3 * rank)) & 0x7; /* System Latency */

		printf(FS PRINTF_FORMAT "+%dT", dqsgd, dgsl);
	}
}

static void dqsgd_dump(const struct uniphier_ddrmphy_param *param)
{
	printf("\n--- DQS Gating Delay ---\n");
	printf("           Rank0   Rank1   Rank2   Rank3\n");

	dump_loop(param, &__dqsgd_dump);
}

static void __mdl_dump(void __iomem *dx_base)
{
	int i;
	u32 mdl = readl(dx_base + MPHY_DX_MDLR);

	for (i = 0; i < 3; i++)
		printf(FS PRINTF_FORMAT, (mdl >> (8 * i)) & 0xff);
}

static void mdl_dump(const struct uniphier_ddrmphy_param *param)
{
	printf("\n--- Master Delay Line ---\n");
	printf("          IPRD TPRD MDLD\n");

	dump_loop(param, &__mdl_dump);
}

#define REG_DUMP(x)							\
	{ int ofst = MPHY_ ## x; void __iomem *reg = phy_base + ofst;	\
		printf("%3d: %-10s: %p : %08x\n",			\
		       ofst >> MPHY_SHIFT, #x, reg, readl(reg)); }

#define DX_REG_DUMP(dx, x)						\
	{ int ofst = MPHY_DX_BASE + MPHY_DX_STRIDE * (dx) +		\
			MPHY_DX_## x;					\
		void __iomem *reg = phy_base + ofst;			\
		printf("%3d: DX%d%-7s: %p : %08x\n",			\
		       ofst >> MPHY_SHIFT, (dx), #x, reg, readl(reg)); }

static void reg_dump(const struct uniphier_ddrmphy_param *param)
{
	void __iomem *phy_base;
	int phy, dx;

	printf("\n--- DDR Multi PHY registers ---\n");

	for (phy = 0; phy < param->nr_phy; phy++) {
		phy_base = ioremap(param->phy[phy].base, SZ_4K);

		printf("== PHY%d (base: %08x) ==\n", phy,
		       ptr_to_uint(phy_base));
		printf(" No: Name      : Address  : Data\n");

		REG_DUMP(RIDR);
		REG_DUMP(PIR);
		REG_DUMP(PGCR0);
		REG_DUMP(PGCR1);
		REG_DUMP(PGCR2);
		REG_DUMP(PGCR3);
		REG_DUMP(PGSR0);
		REG_DUMP(PGSR1);
		REG_DUMP(PLLCR);
		REG_DUMP(PTR0);
		REG_DUMP(PTR1);
		REG_DUMP(PTR2);
		REG_DUMP(PTR3);
		REG_DUMP(PTR4);
		REG_DUMP(ACMDLR);
		REG_DUMP(ACBDLR0);
		REG_DUMP(DXCCR);
		REG_DUMP(DSGCR);
		REG_DUMP(DCR);
		REG_DUMP(DTPR0);
		REG_DUMP(DTPR1);
		REG_DUMP(DTPR2);
		REG_DUMP(DTPR3);
		REG_DUMP(MR0);
		REG_DUMP(MR1);
		REG_DUMP(MR2);
		REG_DUMP(MR3);

		for (dx = 0; dx < param->phy[phy].nr_dx; dx++) {
			DX_REG_DUMP(dx, GCR0);
			DX_REG_DUMP(dx, GCR1);
			DX_REG_DUMP(dx, GCR2);
			DX_REG_DUMP(dx, GCR3);
			DX_REG_DUMP(dx, GTR);
		}

		iounmap(phy_base);
	}
}

static int do_ddrm(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	const struct uniphier_ddrmphy_param *param;
	char *cmd;

	param = uniphier_get_ddrmphy_param();
	if (!param) {
		pr_err("unsupported SoC\n");
		return CMD_RET_FAILURE;
	}

	if (argc == 1)
		cmd = "all";
	else
		cmd = argv[1];

	if (!strcmp(cmd, "zq") || !strcmp(cmd, "all"))
		zq_dump(param);

	if (!strcmp(cmd, "wbdl") || !strcmp(cmd, "all"))
		wbdl_dump(param);

	if (!strcmp(cmd, "rbdl") || !strcmp(cmd, "all"))
		rbdl_dump(param);

	if (!strcmp(cmd, "wld") || !strcmp(cmd, "all"))
		wld_dump(param);

	if (!strcmp(cmd, "dqsgd") || !strcmp(cmd, "all"))
		dqsgd_dump(param);

	if (!strcmp(cmd, "mdl") || !strcmp(cmd, "all"))
		mdl_dump(param);

	if (!strcmp(cmd, "reg") || !strcmp(cmd, "all"))
		reg_dump(param);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	ddrm,	2,	1,	do_ddrm,
	"UniPhier DDR Multi PHY parameters dumper",
	"- dump all of the following\n"
	"ddrm zq - dump Impedance Data\n"
	"ddrm wbdl - dump Write Bit Delay\n"
	"ddrm rbdl - dump Read Bit Delay\n"
	"ddrm wld - dump Write Leveling\n"
	"ddrm dqsgd - dump DQS Gating Delay\n"
	"ddrm mdl - dump Master Delay Line\n"
	"ddrm reg - dump registers\n"
);
