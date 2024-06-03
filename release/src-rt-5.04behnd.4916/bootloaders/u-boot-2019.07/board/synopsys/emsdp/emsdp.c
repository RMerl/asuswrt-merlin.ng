// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Synopsys, Inc. All rights reserved.
 */

#include <common.h>
#include <dwmmc.h>
#include <malloc.h>

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

int board_mmc_init(bd_t *bis)
{
	struct dwmci_host *host = NULL;

	host = malloc(sizeof(struct dwmci_host));
	if (!host) {
		printf("dwmci_host malloc fail!\n");
		return 1;
	}

	memset(host, 0, sizeof(struct dwmci_host));
	host->name = "Synopsys Mobile storage";
	host->ioaddr = SDIO_BASE;
	host->buswidth = 4;
	host->dev_index = 0;
	host->bus_hz = 50000000;

	add_dwmci(host, host->bus_hz / 2, 400000);

	return 0;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct dwmci_host *host = mmc->priv;

	return !(dwmci_readl(host, DWMCI_CDETECT) & 1);
}

#define CREG_BASE		0xF0001000
#define CREG_BOOT		(void *)(CREG_BASE + 0x0FF0)
#define CREG_IP_SW_RESET	(void *)(CREG_BASE + 0x0FF0)
#define CREG_IP_VERSION		(void *)(CREG_BASE + 0x0FF8)

/* Bits in CREG_BOOT register */
#define CREG_BOOT_WP_BIT	BIT(8)

void reset_cpu(ulong addr)
{
	writel(1, CREG_IP_SW_RESET);
	while (1)
		; /* loop forever till reset */
}

static int do_emsdp_rom(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
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

cmd_tbl_t cmd_emsdp[] = {
	U_BOOT_CMD_MKENT(rom, 2, 0, do_emsdp_rom, "", ""),
};

static int do_emsdp(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

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
