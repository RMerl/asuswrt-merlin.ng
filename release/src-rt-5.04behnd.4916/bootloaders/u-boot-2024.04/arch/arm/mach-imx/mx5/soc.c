// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/cache.h>

#include <linux/errno.h>
#include <asm/io.h>
#include <asm/mach-imx/boot_mode.h>

#if !(defined(CONFIG_MX51) || defined(CONFIG_MX53))
#error "CPU_TYPE not defined"
#endif

u32 get_cpu_rev(void)
{
#ifdef CONFIG_MX51
	int system_rev = 0x51000;
#else
	int system_rev = 0x53000;
#endif
	int reg = __raw_readl(ROM_SI_REV);

#if defined(CONFIG_MX51)
	switch (reg) {
	case 0x02:
		system_rev |= CHIP_REV_1_1;
		break;
	case 0x10:
		if ((__raw_readl(GPIO1_BASE_ADDR + 0x0) & (0x1 << 22)) == 0)
			system_rev |= CHIP_REV_2_5;
		else
			system_rev |= CHIP_REV_2_0;
		break;
	case 0x20:
		system_rev |= CHIP_REV_3_0;
		break;
	default:
		system_rev |= CHIP_REV_1_0;
		break;
	}
#else
	if (reg < 0x20)
		system_rev |= CHIP_REV_1_0;
	else
		system_rev |= reg;
#endif
	return system_rev;
}

#ifdef CONFIG_REVISION_TAG
u32 __weak get_board_rev(void)
{
	return get_cpu_rev();
}
#endif

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

#if defined(CONFIG_FEC_MXC)
void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	int i;
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE;
	struct fuse_bank *bank = &iim->bank[1];
	struct fuse_bank1_regs *fuse =
			(struct fuse_bank1_regs *)bank->fuse_regs;

	for (i = 0; i < 6; i++)
		mac[i] = readl(&fuse->mac_addr[i]) & 0xff;
}
#endif

#ifdef CONFIG_MX53
#define IMX53_SRTC_LPGR_PERSIST_SECONDARY_BOOT	BIT(30)

void boot_mode_apply(unsigned cfg_val)
{
	void *lpgr = &((struct srtc_regs *)SRTC_BASE_ADDR)->lpgr;

	if (cfg_val == MAKE_CFGVAL_PRIMARY_BOOT)
		clrbits_le32(lpgr, IMX53_SRTC_LPGR_PERSIST_SECONDARY_BOOT);
	else if (cfg_val == MAKE_CFGVAL_SECONDARY_BOOT)
		setbits_le32(lpgr, IMX53_SRTC_LPGR_PERSIST_SECONDARY_BOOT);
	else
		writel(cfg_val, lpgr);
}

int boot_mode_getprisec(void)
{
	void *lpgr = &((struct srtc_regs *)SRTC_BASE_ADDR)->lpgr;

	return !!(readl(lpgr) & IMX53_SRTC_LPGR_PERSIST_SECONDARY_BOOT);
}

/*
 * cfg_val will be used for
 * Boot_cfg3[7:0]:Boot_cfg2[7:0]:Boot_cfg1[7:0]
 *
 * If bit 28 of LPGR is set upon watchdog reset,
 * bits[25:0] of LPGR will move to SBMR.
 */
const struct boot_mode soc_boot_modes[] = {
	{"normal",	MAKE_CFGVAL(0x00, 0x00, 0x00, 0x00)},
	/* usb or serial download */
	{"usb",		MAKE_CFGVAL(0x00, 0x00, 0x00, 0x13)},
	{"sata",	MAKE_CFGVAL(0x28, 0x00, 0x00, 0x12)},
	{"escpi1:0",	MAKE_CFGVAL(0x38, 0x20, 0x00, 0x12)},
	{"escpi1:1",	MAKE_CFGVAL(0x38, 0x20, 0x04, 0x12)},
	{"escpi1:2",	MAKE_CFGVAL(0x38, 0x20, 0x08, 0x12)},
	{"escpi1:3",	MAKE_CFGVAL(0x38, 0x20, 0x0c, 0x12)},
	/* 4 bit bus width */
	{"esdhc1",	MAKE_CFGVAL(0x40, 0x20, 0x00, 0x12)},
	{"esdhc2",	MAKE_CFGVAL(0x40, 0x20, 0x08, 0x12)},
	{"esdhc3",	MAKE_CFGVAL(0x40, 0x20, 0x10, 0x12)},
	{"esdhc4",	MAKE_CFGVAL(0x40, 0x20, 0x18, 0x12)},
	{"primary",	MAKE_CFGVAL_PRIMARY_BOOT},
	{"secondary",	MAKE_CFGVAL_SECONDARY_BOOT},
	{NULL,		0},
};
#endif
