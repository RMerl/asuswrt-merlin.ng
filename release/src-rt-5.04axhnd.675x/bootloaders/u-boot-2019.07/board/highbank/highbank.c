// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 */

#include <common.h>
#include <ahci.h>
#include <netdev.h>
#include <scsi.h>

#include <linux/sizes.h>
#include <asm/io.h>

#define HB_AHCI_BASE			0xffe08000

#define HB_SCU_A9_PWR_STATUS		0xfff10008
#define HB_SREG_A9_PWR_REQ		0xfff3cf00
#define HB_SREG_A9_BOOT_SRC_STAT	0xfff3cf04
#define HB_SREG_A9_PWRDOM_STAT		0xfff3cf20
#define HB_SREG_A15_PWR_CTRL		0xfff3c200

#define HB_PWR_SUSPEND			0
#define HB_PWR_SOFT_RESET		1
#define HB_PWR_HARD_RESET		2
#define HB_PWR_SHUTDOWN			3

#define PWRDOM_STAT_SATA		0x80000000
#define PWRDOM_STAT_PCI			0x40000000
#define PWRDOM_STAT_EMMC		0x20000000

#define HB_SCU_A9_PWR_NORMAL		0
#define HB_SCU_A9_PWR_DORMANT		2
#define HB_SCU_A9_PWR_OFF		3

DECLARE_GLOBAL_DATA_PTR;

void cphy_disable_overrides(void);

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	icache_enable();

	return 0;
}

/* We know all the init functions have been run now */
int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_CALXEDA_XGMAC
	rc += calxedaxgmac_initialize(0, 0xfff50000);
	rc += calxedaxgmac_initialize(1, 0xfff51000);
#endif
	return rc;
}

#ifdef CONFIG_SCSI_AHCI_PLAT
void scsi_init(void)
{
	u32 reg = readl(HB_SREG_A9_PWRDOM_STAT);

	cphy_disable_overrides();
	if (reg & PWRDOM_STAT_SATA) {
		ahci_init((void __iomem *)HB_AHCI_BASE);
		scsi_scan(true);
	}
}
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	char envbuffer[16];
	u32 boot_choice;

	boot_choice = readl(HB_SREG_A9_BOOT_SRC_STAT) & 0xff;
	sprintf(envbuffer, "bootcmd%d", boot_choice);
	if (env_get(envbuffer)) {
		sprintf(envbuffer, "run bootcmd%d", boot_choice);
		env_set("bootcmd", envbuffer);
	} else
		env_set("bootcmd", "");

	return 0;
}
#endif

int dram_init(void)
{
	gd->ram_size = SZ_512M;
	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *fdt, bd_t *bd)
{
	static const char disabled[] = "disabled";
	u32 reg = readl(HB_SREG_A9_PWRDOM_STAT);

	if (!(reg & PWRDOM_STAT_SATA))
		do_fixup_by_compat(fdt, "calxeda,hb-ahci", "status",
			disabled, sizeof(disabled), 1);

	if (!(reg & PWRDOM_STAT_EMMC))
		do_fixup_by_compat(fdt, "calxeda,hb-sdhci", "status",
			disabled, sizeof(disabled), 1);

	return 0;
}
#endif

static int is_highbank(void)
{
	uint32_t midr;

	asm volatile ("mrc p15, 0, %0, c0, c0, 0\n" : "=r"(midr));

	return (midr & 0xfff0) == 0xc090;
}

void reset_cpu(ulong addr)
{
	writel(HB_PWR_HARD_RESET, HB_SREG_A9_PWR_REQ);
	if (is_highbank())
		writeb(HB_SCU_A9_PWR_OFF, HB_SCU_A9_PWR_STATUS);
	else
		writel(0x1, HB_SREG_A15_PWR_CTRL);

	wfi();
}

/*
 * turn off the override before transferring control to Linux, since Linux
 * may not support spread spectrum.
 */
void arch_preboot_os(void)
{
	cphy_disable_overrides();
}
