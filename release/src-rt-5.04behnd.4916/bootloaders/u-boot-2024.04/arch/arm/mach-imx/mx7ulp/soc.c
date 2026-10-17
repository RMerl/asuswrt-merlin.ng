// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 * Copyright 2021 NXP
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/hab.h>
#include <asm/mach-imx/sys_proto.h>
#include <asm/setup.h>
#include <linux/bitops.h>
#include <dm.h>

#define PMC0_BASE_ADDR		0x410a1000
#define PMC0_CTRL		0x28
#define PMC0_CTRL_LDOEN		BIT(31)
#define PMC0_CTRL_LDOOKDIS	BIT(30)
#define PMC0_CTRL_PMC1ON	BIT(24)
#define PMC1_BASE_ADDR		0x40400000
#define PMC1_RUN		0x8
#define PMC1_STOP		0x10
#define PMC1_VLPS		0x14
#define PMC1_LDOVL_SHIFT	16
#define PMC1_LDOVL_MASK		(0x3f << PMC1_LDOVL_SHIFT)
#define PMC1_LDOVL_900		0x1e
#define PMC1_LDOVL_950		0x23
#define PMC1_STATUS		0x20
#define PMC1_STATUS_LDOVLF	BIT(8)

static char *get_reset_cause(char *);

#if defined(CONFIG_IMX_HAB)
struct imx_sec_config_fuse_t const imx_sec_config_fuse = {
	.bank = 29,
	.word = 6,
};
#endif

#define ROM_VERSION_ADDR 0x80
u32 get_cpu_rev(void)
{
	/* Check the ROM version for cpu revision */
	u32 rom_version = readl((void __iomem *)ROM_VERSION_ADDR);

	return (MXC_CPU_MX7ULP << 12) | (rom_version & 0xFF);
}

#ifdef CONFIG_REVISION_TAG
u32 __weak get_board_rev(void)
{
	return get_cpu_rev();
}
#endif

enum bt_mode get_boot_mode(void)
{
	u32 bt0_cfg = 0;

	bt0_cfg = readl(CMC0_RBASE + 0x40);
	bt0_cfg &= (BT0CFG_LPBOOT_MASK | BT0CFG_DUALBOOT_MASK);

	if (!(bt0_cfg & BT0CFG_LPBOOT_MASK)) {
		/* No low power boot */
		if (bt0_cfg & BT0CFG_DUALBOOT_MASK)
			return DUAL_BOOT;
		else
			return SINGLE_BOOT;
	}

	return LOW_POWER_BOOT;
}

int arch_cpu_init(void)
{
	enable_ca7_smp();
	return 0;
}

#if defined(CONFIG_ARCH_MISC_INIT)
int arch_misc_init(void)
{
	if (IS_ENABLED(CONFIG_FSL_CAAM)) {
		struct udevice *dev;
		int ret;

		ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(caam_jr), &dev);
		if (ret)
			printf("Failed to initialize caam_jr: %d\n", ret);
	}

	return 0;
}
#endif

#ifdef CONFIG_BOARD_POSTCLK_INIT
int board_postclk_init(void)
{
	return 0;
}
#endif

#define UNLOCK_WORD0 0xC520 /* 1st unlock word */
#define UNLOCK_WORD1 0xD928 /* 2nd unlock word */
#define REFRESH_WORD0 0xA602 /* 1st refresh word */
#define REFRESH_WORD1 0xB480 /* 2nd refresh word */

static void disable_wdog(u32 wdog_base)
{
	u32 val_cs = readl(wdog_base + 0x00);

	if (!(val_cs & 0x80))
		return;

	dmb();
	__raw_writel(REFRESH_WORD0, (wdog_base + 0x04)); /* Refresh the CNT */
	__raw_writel(REFRESH_WORD1, (wdog_base + 0x04));
	dmb();

	if (!(val_cs & 800)) {
		dmb();
		__raw_writel(UNLOCK_WORD0, (wdog_base + 0x04));
		__raw_writel(UNLOCK_WORD1, (wdog_base + 0x04));
		dmb();

		while (!(readl(wdog_base + 0x00) & 0x800));
	}
	dmb();
	__raw_writel(0x0, wdog_base + 0x0C); /* Set WIN to 0 */
	__raw_writel(0x400, wdog_base + 0x08); /* Set timeout to default 0x400 */
	__raw_writel(0x120, wdog_base + 0x00); /* Disable it and set update */
	dmb();

	while (!(readl(wdog_base + 0x00) & 0x400));
}

void init_wdog(void)
{
	/*
	 * ROM will configure WDOG1, disable it or enable it
	 * depending on FUSE. The update bit is set for reconfigurable.
	 * We have to use unlock sequence to reconfigure it.
	 * WDOG2 is not touched by ROM, so it will have default value
	 * which is enabled. We can directly configure it.
	 * To simplify the codes, we still use same reconfigure
	 * process as WDOG1. Because the update bit is not set for
	 * WDOG2, the unlock sequence won't take effect really.
	 * It actually directly configure the wdog.
	 * In this function, we will disable both WDOG1 and WDOG2,
	 * and set update bit for both. So that kernel can reconfigure them.
	 */
	disable_wdog(WDG1_RBASE);
	disable_wdog(WDG2_RBASE);
}

static bool ldo_mode_is_enabled(void)
{
	unsigned int reg;

	reg = readl(PMC0_BASE_ADDR + PMC0_CTRL);
	if (reg & PMC0_CTRL_LDOEN)
		return true;
	else
		return false;
}

#if !defined(CONFIG_SPL) || (defined(CONFIG_SPL) && defined(CONFIG_SPL_BUILD))
#if defined(CONFIG_LDO_ENABLED_MODE)
static void init_ldo_mode(void)
{
	unsigned int reg;

	if (ldo_mode_is_enabled())
		return;

	/* Set LDOOKDIS */
	setbits_le32(PMC0_BASE_ADDR + PMC0_CTRL, PMC0_CTRL_LDOOKDIS);

	/* Set LDOVL to 0.95V in PMC1_RUN */
	reg = readl(PMC1_BASE_ADDR + PMC1_RUN);
	reg &= ~PMC1_LDOVL_MASK;
	reg |= (PMC1_LDOVL_950 << PMC1_LDOVL_SHIFT);
	writel(PMC1_BASE_ADDR + PMC1_RUN, reg);

	/* Wait for LDOVLF to be cleared */
	reg = readl(PMC1_BASE_ADDR + PMC1_STATUS);
	while (reg & PMC1_STATUS_LDOVLF)
		;

	/* Set LDOVL to 0.95V in PMC1_STOP */
	reg = readl(PMC1_BASE_ADDR + PMC1_STOP);
	reg &= ~PMC1_LDOVL_MASK;
	reg |= (PMC1_LDOVL_950 << PMC1_LDOVL_SHIFT);
	writel(PMC1_BASE_ADDR + PMC1_STOP, reg);

	/* Set LDOVL to 0.90V in PMC1_VLPS */
	reg = readl(PMC1_BASE_ADDR + PMC1_VLPS);
	reg &= ~PMC1_LDOVL_MASK;
	reg |= (PMC1_LDOVL_900 << PMC1_LDOVL_SHIFT);
	writel(PMC1_BASE_ADDR + PMC1_VLPS, reg);

	/* Set LDOEN bit */
	setbits_le32(PMC0_BASE_ADDR + PMC0_CTRL, PMC0_CTRL_LDOEN);

	/* Set the PMC1ON bit */
	setbits_le32(PMC0_BASE_ADDR + PMC0_CTRL, PMC0_CTRL_PMC1ON);
}
#endif

void s_init(void)
{
	/* Disable wdog */
	init_wdog();

	/* clock configuration. */
	clock_init();

	if (soc_rev() < CHIP_REV_2_0) {
		/* enable dumb pmic */
		writel((readl(SNVS_LP_LPCR) | SNVS_LPCR_DPEN), SNVS_LP_LPCR);
	}

#if defined(CONFIG_LDO_ENABLED_MODE)
	init_ldo_mode();
#endif
	return;
}
#endif

#ifndef CONFIG_ULP_WATCHDOG
void reset_cpu(void)
{
	setbits_le32(SIM0_RBASE, SIM_SOPT1_A7_SW_RESET);
	while (1)
		;
}
#endif

#if defined(CONFIG_DISPLAY_CPUINFO)
const char *get_imx_type(u32 imxtype)
{
	return "7ULP";
}

int print_cpuinfo(void)
{
	u32 cpurev;
	char cause[18];

	cpurev = get_cpu_rev();

	printf("CPU:   Freescale i.MX%s rev%d.%d at %d MHz\n",
	       get_imx_type((cpurev & 0xFF000) >> 12),
	       (cpurev & 0x000F0) >> 4, (cpurev & 0x0000F) >> 0,
	       mxc_get_clock(MXC_ARM_CLK) / 1000000);

	printf("Reset cause: %s\n", get_reset_cause(cause));

	printf("Boot mode: ");
	switch (get_boot_mode()) {
	case LOW_POWER_BOOT:
		printf("Low power boot\n");
		break;
	case DUAL_BOOT:
		printf("Dual boot\n");
		break;
	case SINGLE_BOOT:
	default:
		printf("Single boot\n");
		break;
	}

	if (ldo_mode_is_enabled())
		printf("PMC1:  LDO enabled mode\n");
	else
		printf("PMC1:  LDO bypass mode\n");

	return 0;
}
#endif

#define CMC_SRS_TAMPER                    (1 << 31)
#define CMC_SRS_SECURITY                  (1 << 30)
#define CMC_SRS_TZWDG                     (1 << 29)
#define CMC_SRS_JTAG_RST                  (1 << 28)
#define CMC_SRS_CORE1                     (1 << 16)
#define CMC_SRS_LOCKUP                    (1 << 15)
#define CMC_SRS_SW                        (1 << 14)
#define CMC_SRS_WDG                       (1 << 13)
#define CMC_SRS_PIN_RESET                 (1 << 8)
#define CMC_SRS_WARM                      (1 << 4)
#define CMC_SRS_HVD                       (1 << 3)
#define CMC_SRS_LVD                       (1 << 2)
#define CMC_SRS_POR                       (1 << 1)
#define CMC_SRS_WUP                       (1 << 0)

static u32 reset_cause = -1;

static char *get_reset_cause(char *ret)
{
	u32 cause1, cause = 0, srs = 0;
	u32 *reg_ssrs = (u32 *)(SRC_BASE_ADDR + 0x28);
	u32 *reg_srs = (u32 *)(SRC_BASE_ADDR + 0x20);

	if (!ret)
		return "null";

	srs = readl(reg_srs);
	cause1 = readl(reg_ssrs);
	writel(cause1, reg_ssrs);

	reset_cause = cause1;

	cause = cause1 & (CMC_SRS_POR | CMC_SRS_WUP | CMC_SRS_WARM);

	switch (cause) {
	case CMC_SRS_POR:
		sprintf(ret, "%s", "POR");
		break;
	case CMC_SRS_WUP:
		sprintf(ret, "%s", "WUP");
		break;
	case CMC_SRS_WARM:
		cause = cause1 & (CMC_SRS_WDG | CMC_SRS_SW |
			CMC_SRS_JTAG_RST);
		switch (cause) {
		case CMC_SRS_WDG:
			sprintf(ret, "%s", "WARM-WDG");
			break;
		case CMC_SRS_SW:
			sprintf(ret, "%s", "WARM-SW");
			break;
		case CMC_SRS_JTAG_RST:
			sprintf(ret, "%s", "WARM-JTAG");
			break;
		default:
			sprintf(ret, "%s", "WARM-UNKN");
			break;
		}
		break;
	default:
		sprintf(ret, "%s-%X", "UNKN", cause1);
		break;
	}

	debug("[%X] SRS[%X] %X - ", cause1, srs, srs^cause1);
	return ret;
}

#ifdef CONFIG_ENV_IS_IN_MMC
__weak int board_mmc_get_env_dev(int devno)
{
	return CONFIG_SYS_MMC_ENV_DEV;
}

int mmc_get_env_dev(void)
{
	int devno = 0;
	u32 bt1_cfg = 0;

	/* If not boot from sd/mmc, use default value */
	if (get_boot_mode() == LOW_POWER_BOOT)
		return CONFIG_SYS_MMC_ENV_DEV;

	bt1_cfg = readl(CMC1_RBASE + 0x40);
	devno = (bt1_cfg >> 9) & 0x7;

	return board_mmc_get_env_dev(devno);
}
#endif

enum boot_device get_boot_device(void)
{
	struct bootrom_sw_info **p =
		(struct bootrom_sw_info **)ROM_SW_INFO_ADDR;

	enum boot_device boot_dev = SD1_BOOT;
	u8 boot_type = (*p)->boot_dev_type;
	u8 boot_instance = (*p)->boot_dev_instance;

	switch (boot_type) {
	case BOOT_TYPE_SD:
		boot_dev = boot_instance + SD1_BOOT;
		break;
	case BOOT_TYPE_MMC:
		boot_dev = boot_instance + MMC1_BOOT;
		break;
	case BOOT_TYPE_USB:
		boot_dev = USB_BOOT;
		break;
	default:
		break;
	}

	return boot_dev;
}

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
/*
 * OCOTP_CFG (SJC CHALLENGE, Unique ID)
 * i.MX 7ULP Applications Processor Reference Manual, Rev. 0, 09/2020
 *
 * OCOTP_CFG0 offset 0x4B0: 15:0 -> 15:0  bits of Unique ID
 * OCOTP_CFG1 offset 0x4C0: 15:0 -> 31:16 bits of Unique ID
 * OCOTP_CFG2 offset 0x4D0: 15:0 -> 47:32 bits of Unique ID
 * OCOTP_CFG3 offset 0x4E0: 15:0 -> 63:48 bits of Unique ID
 */
void get_board_serial(struct tag_serialnr *serialnr)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[1];
	struct fuse_bank1_regs *fuse =
		(struct fuse_bank1_regs *)bank->fuse_regs;

	serialnr->low = (fuse->cfg0 & 0xFFFF) + ((fuse->cfg1 & 0xFFFF) << 16);
	serialnr->high = (fuse->cfg2 & 0xFFFF) + ((fuse->cfg3 & 0xFFFF) << 16);
}
#endif /* CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG */
