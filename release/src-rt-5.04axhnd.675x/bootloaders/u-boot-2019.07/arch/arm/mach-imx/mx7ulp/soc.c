// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/hab.h>

static char *get_reset_cause(char *);

#if defined(CONFIG_SECURE_BOOT)
struct imx_sec_config_fuse_t const imx_sec_config_fuse = {
	.bank = 29,
	.word = 6,
};
#endif

u32 get_cpu_rev(void)
{
	/* Temporally hard code the CPU rev to 0x73, rev 1.0. Fix it later */
	return (MXC_CPU_MX7ULP << 12) | (1 << 4);
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
	return 0;
}

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
	writel(UNLOCK_WORD0, (wdog_base + 0x04));
	writel(UNLOCK_WORD1, (wdog_base + 0x04));
	writel(0x0, (wdog_base + 0x0C)); /* Set WIN to 0 */
	writel(0x400, (wdog_base + 0x08)); /* Set timeout to default 0x400 */
	writel(0x120, (wdog_base + 0x00)); /* Disable it and set update */

	writel(REFRESH_WORD0, (wdog_base + 0x04)); /* Refresh the CNT */
	writel(REFRESH_WORD1, (wdog_base + 0x04));
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


void s_init(void)
{
	/* Disable wdog */
	init_wdog();

	/* clock configuration. */
	clock_init();

	return;
}

#ifndef CONFIG_ULP_WATCHDOG
void reset_cpu(ulong addr)
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
