// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Synopsys, Inc. All rights reserved.
 */

#include <command.h>
#include <cpu_func.h>
#include <dwmmc.h>
#include <init.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <linux/bitops.h>

#include <asm/arcregs.h>

DECLARE_GLOBAL_DATA_PTR;

#define ARC_PERIPHERAL_BASE		0xF0000000

#define CGU_ARC_FMEAS_ARC		(void *)(ARC_PERIPHERAL_BASE + 0x84)
#define CGU_ARC_FMEAS_ARC_START		BIT(31)
#define CGU_ARC_FMEAS_ARC_DONE		BIT(30)
#define CGU_ARC_FMEAS_ARC_CNT_MASK	GENMASK(14, 0)
#define CGU_ARC_FMEAS_ARC_RCNT_OFFSET	0
#define CGU_ARC_FMEAS_ARC_FCNT_OFFSET	15

#define SDIO_BASE			(void *)(ARC_PERIPHERAL_BASE + 0x10000)

int mach_cpu_init(void)
{
	int rcnt, fcnt;
	u32 data;

	/* Start frequency measurement */
	writel(CGU_ARC_FMEAS_ARC_START, CGU_ARC_FMEAS_ARC);

	/* Poll DONE bit */
	do {
		data = readl(CGU_ARC_FMEAS_ARC);
	} while (!(data & CGU_ARC_FMEAS_ARC_DONE));

	/* Amount of reference 100 MHz clocks */
	rcnt = ((data >> CGU_ARC_FMEAS_ARC_RCNT_OFFSET) &
	       CGU_ARC_FMEAS_ARC_CNT_MASK);

	/* Amount of CPU clocks */
	fcnt = ((data >> CGU_ARC_FMEAS_ARC_FCNT_OFFSET) &
	       CGU_ARC_FMEAS_ARC_CNT_MASK);

	gd->cpu_clk = ((100 * fcnt) / rcnt) * 1000000;

	return 0;
}

int board_early_init_r(void)
{
#define EMSDP_PSRAM_BASE		0xf2001000
#define PSRAM_FLASH_CONFIG_REG_0	(void *)(EMSDP_PSRAM_BASE + 0x10)
#define PSRAM_FLASH_CONFIG_REG_1	(void *)(EMSDP_PSRAM_BASE + 0x14)
#define CRE_ENABLE			BIT(31)
#define CRE_DRIVE_CMD			BIT(6)

#define PSRAM_RCR_DPD			BIT(1)
#define PSRAM_RCR_PAGE_MODE		BIT(7)

/*
 * PSRAM_FLASH_CONFIG_REG_x[30:15] to the address lines[16:1] of flash,
 * thus "<< 1".
 */
#define PSRAM_RCR_SETUP		((PSRAM_RCR_DPD | PSRAM_RCR_PAGE_MODE) << 1)

	// Switch PSRAM controller to command mode
	writel(CRE_ENABLE | CRE_DRIVE_CMD, PSRAM_FLASH_CONFIG_REG_0);
	// Program Refresh Configuration Register (RCR) for BANK0
	writew(0, (void *)(0x10000000 + PSRAM_RCR_SETUP));
	// Switch PSRAM controller back to memory mode
	writel(0, PSRAM_FLASH_CONFIG_REG_0);


	// Switch PSRAM controller to command mode
	writel(CRE_ENABLE | CRE_DRIVE_CMD, PSRAM_FLASH_CONFIG_REG_1);
	// Program Refresh Configuration Register (RCR) for BANK1
	writew(0, (void *)(0x10800000 + PSRAM_RCR_SETUP));
	// Switch PSRAM controller back to memory mode
	writel(0, PSRAM_FLASH_CONFIG_REG_1);

	printf("PSRAM initialized.\n");

	return 0;
}

#define CREG_BASE		0xF0001000
#define CREG_BOOT		(void *)(CREG_BASE + 0x0FF0)
#define CREG_IP_SW_RESET	(void *)(CREG_BASE + 0x0FF0)
#define CREG_IP_VERSION		(void *)(CREG_BASE + 0x0FF8)

/* Bits in CREG_BOOT register */
#define CREG_BOOT_WP_BIT	BIT(8)

void reset_cpu(void)
{
	writel(1, CREG_IP_SW_RESET);
	while (1)
		; /* loop forever till reset */
}

static int do_emsdp_rom(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	u32 creg_boot = readl(CREG_BOOT);

	if (!strcmp(argv[1], "unlock"))
		creg_boot &= ~CREG_BOOT_WP_BIT;
	else if (!strcmp(argv[1], "lock"))
		creg_boot |= CREG_BOOT_WP_BIT;
	else
		return CMD_RET_USAGE;

	writel(creg_boot, CREG_BOOT);

	return CMD_RET_SUCCESS;
}

struct cmd_tbl cmd_emsdp[] = {
	U_BOOT_CMD_MKENT(rom, 2, 0, do_emsdp_rom, "", ""),
};

static int do_emsdp(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	struct cmd_tbl *c;

	c = find_cmd_tbl(argv[1], cmd_emsdp, ARRAY_SIZE(cmd_emsdp));

	/* Strip off leading 'emsdp' command */
	argc--;
	argv++;

	if (c == NULL || argc > c->maxargs)
		return CMD_RET_USAGE;

	return c->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	emsdp, CONFIG_SYS_MAXARGS, 0, do_emsdp,
	"Synopsys EMSDP specific commands",
	"rom unlock - Unlock non-volatile memory for writing\n"
	"emsdp rom lock - Lock non-volatile memory to prevent writing\n"
);

int checkboard(void)
{
	int version = readl(CREG_IP_VERSION);

	printf("Board: ARC EM Software Development Platform v%d.%d\n",
	       (version >> 16) & 0xff, version & 0xff);
	return 0;
};
