// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010 Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 * Based on original Kirkwood support which is
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include <common.h>
#include <netdev.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <u-boot/md5.h>
#include <asm/arch/cpu.h>

#define BUFLEN	16

void reset_cpu(unsigned long ignored)
{
	struct orion5x_cpu_registers *cpureg =
	    (struct orion5x_cpu_registers *)ORION5X_CPU_REG_BASE;

	writel(readl(&cpureg->rstoutn_mask) | (1 << 2),
		&cpureg->rstoutn_mask);
	writel(readl(&cpureg->sys_soft_rst) | 1,
		&cpureg->sys_soft_rst);
	while (1)
		;
}

/*
 * Compute Window Size field value from size expressed in bytes
 * Used with the Base register to set the address window size and location.
 * Must be programmed from LSB to MSB as sequence of ones followed by
 * sequence of zeros. The number of ones specifies the size of the window in
 * 64 KiB granularity (e.g., a value of 0x00FF specifies 256 = 16 MiB).
 * NOTES:
 * 1) A sizeval equal to 0x0 specifies 4 GiB.
 * 2) A return value of 0x0 specifies 64 KiB.
 */
unsigned int orion5x_winctrl_calcsize(unsigned int sizeval)
{
	/*
	 * Calculate the number of 64 KiB blocks needed minus one (rounding up).
	 * For sizeval > 0 this is equivalent to:
	 * sizeval = (u32) ceil((double) sizeval / 65536.0) - 1
	 */
	sizeval = (sizeval - 1) >> 16;

	/*
	 * Propagate 'one' bits to the right by 'oring' them.
	 * We need only treat bits 15-0.
	 */
	sizeval |= sizeval >> 1;  /* 'Or' bit 15 onto bit 14 */
	sizeval |= sizeval >> 2;  /* 'Or' bits 15-14 onto bits 13-12 */
	sizeval |= sizeval >> 4;  /* 'Or' bits 15-12 onto bits 11-8 */
	sizeval |= sizeval >> 8;  /* 'Or' bits 15-8 onto bits 7-0*/

	return sizeval;
}

/*
 * orion5x_config_adr_windows - Configure address Windows
 *
 * There are 8 address windows supported by Orion5x Soc to addess different
 * devices. Each window can be configured for size, BAR and remap addr
 * Below configuration is standard for most of the cases
 *
 * If remap function not used, remap_lo must be set as base
 *
 * NOTES:
 *
 * 1) in order to avoid windows with inconsistent control and base values
 *    (which could prevent access to BOOTCS and hence execution from FLASH)
 *    always disable window before writing the base value then reenable it
 *    by writing the control value.
 *
 * 2) in order to avoid losing access to BOOTCS when disabling window 7,
 *    first configure window 6 for BOOTCS, then configure window 7 for BOOTCS,
 *    then configure windows 6 for its own target.
 *
 * Reference Documentation:
 * Mbus-L to Mbus Bridge Registers Configuration.
 * (Sec 25.1 and 25.3 of Datasheet)
 */
int orion5x_config_adr_windows(void)
{
	struct orion5x_win_registers *winregs =
		(struct orion5x_win_registers *)ORION5X_CPU_WIN_BASE;

/* Disable window 0, configure it for its intended target, enable it. */
	writel(0, &winregs[0].ctrl);
	writel(ORION5X_ADR_PCIE_MEM, &winregs[0].base);
	writel(ORION5X_ADR_PCIE_MEM_REMAP_LO, &winregs[0].remap_lo);
	writel(ORION5X_ADR_PCIE_MEM_REMAP_HI, &winregs[0].remap_hi);
	writel(ORION5X_CPU_WIN_CTRL_DATA(ORION5X_SZ_PCIE_MEM,
		ORION5X_TARGET_PCIE, ORION5X_ATTR_PCIE_MEM,
		ORION5X_WIN_ENABLE), &winregs[0].ctrl);
/* Disable window 1, configure it for its intended target, enable it. */
	writel(0, &winregs[1].ctrl);
	writel(ORION5X_ADR_PCIE_IO, &winregs[1].base);
	writel(ORION5X_ADR_PCIE_IO_REMAP_LO, &winregs[1].remap_lo);
	writel(ORION5X_ADR_PCIE_IO_REMAP_HI, &winregs[1].remap_hi);
	writel(ORION5X_CPU_WIN_CTRL_DATA(ORION5X_SZ_PCIE_IO,
		ORION5X_TARGET_PCIE, ORION5X_ATTR_PCIE_IO,
		ORION5X_WIN_ENABLE), &winregs[1].ctrl);
/* Disable window 2, configure it for its intended target, enable it. */
	writel(0, &winregs[2].ctrl);
	writel(ORION5X_ADR_PCI_MEM, &winregs[2].base);
	writel(ORION5X_CPU_WIN_CTRL_DATA(ORION5X_SZ_PCI_MEM,
		ORION5X_TARGET_PCI, ORION5X_ATTR_PCI_MEM,
		ORION5X_WIN_ENABLE), &winregs[2].ctrl);
/* Disable window 3, configure it for its intended target, enable it. */
	writel(0, &winregs[3].ctrl);
	writel(ORION5X_ADR_PCI_IO, &winregs[3].base);
	writel(ORION5X_CPU_WIN_CTRL_DATA(ORION5X_SZ_PCI_IO,
		ORION5X_TARGET_PCI, ORION5X_ATTR_PCI_IO,
		ORION5X_WIN_ENABLE), &winregs[3].ctrl);
/* Disable window 4, configure it for its intended target, enable it. */
	writel(0, &winregs[4].ctrl);
	writel(ORION5X_ADR_DEV_CS0, &winregs[4].base);
	writel(ORION5X_CPU_WIN_CTRL_DATA(ORION5X_SZ_DEV_CS0,
		ORION5X_TARGET_DEVICE, ORION5X_ATTR_DEV_CS0,
		ORION5X_WIN_ENABLE), &winregs[4].ctrl);
/* Disable window 5, configure it for its intended target, enable it. */
	writel(0, &winregs[5].ctrl);
	writel(ORION5X_ADR_DEV_CS1, &winregs[5].base);
	writel(ORION5X_CPU_WIN_CTRL_DATA(ORION5X_SZ_DEV_CS1,
		ORION5X_TARGET_DEVICE, ORION5X_ATTR_DEV_CS1,
		ORION5X_WIN_ENABLE), &winregs[5].ctrl);
/* Disable window 6, configure it for FLASH, enable it. */
	writel(0, &winregs[6].ctrl);
	writel(ORION5X_ADR_BOOTROM, &winregs[6].base);
	writel(ORION5X_CPU_WIN_CTRL_DATA(ORION5X_SZ_BOOTROM,
		ORION5X_TARGET_DEVICE, ORION5X_ATTR_BOOTROM,
		ORION5X_WIN_ENABLE), &winregs[6].ctrl);
/* Disable window 7, configure it for FLASH, enable it. */
	writel(0, &winregs[7].ctrl);
	writel(ORION5X_ADR_BOOTROM, &winregs[7].base);
	writel(ORION5X_CPU_WIN_CTRL_DATA(ORION5X_SZ_BOOTROM,
		ORION5X_TARGET_DEVICE, ORION5X_ATTR_BOOTROM,
		ORION5X_WIN_ENABLE), &winregs[7].ctrl);
/* Disable window 6, configure it for its intended target, enable it. */
	writel(0, &winregs[6].ctrl);
	writel(ORION5X_ADR_DEV_CS2, &winregs[6].base);
	writel(ORION5X_CPU_WIN_CTRL_DATA(ORION5X_SZ_DEV_CS2,
		ORION5X_TARGET_DEVICE, ORION5X_ATTR_DEV_CS2,
		ORION5X_WIN_ENABLE), &winregs[6].ctrl);

	return 0;
}

/*
 * Orion5x identification is done through PCIE space.
 */

u32 orion5x_device_id(void)
{
	return readl(PCIE_DEV_ID_OFF) >> 16;
}

u32 orion5x_device_rev(void)
{
	return readl(PCIE_DEV_REV_OFF) & 0xff;
}

#if defined(CONFIG_DISPLAY_CPUINFO)

/* Display device and revision IDs.
 * This function must cover all known device/revision
 * combinations, not only the one for which u-boot is
 * compiled; this way, one can identify actual HW in
 * case of a mismatch.
 */
int print_cpuinfo(void)
{
	char dev_str[7]; /* room enough for 0x0000 plus null byte */
	char rev_str[5]; /* room enough for 0x00 plus null byte */
	char *dev_name = NULL;
	char *rev_name = NULL;

	u32 dev = orion5x_device_id();
	u32 rev = orion5x_device_rev();

	if (dev == MV88F5181_DEV_ID) {
		dev_name = "MV88F5181";
		if (rev == MV88F5181_REV_B1)
			rev_name = "B1";
		else if (rev == MV88F5181L_REV_A1) {
			dev_name = "MV88F5181L";
			rev_name = "A1";
		} else if (rev == MV88F5181L_REV_A0) {
			dev_name = "MV88F5181L";
			rev_name = "A0";
		}
	} else if (dev == MV88F5182_DEV_ID) {
		dev_name = "MV88F5182";
		if (rev == MV88F5182_REV_A2)
			rev_name = "A2";
	} else if (dev == MV88F5281_DEV_ID) {
		dev_name = "MV88F5281";
		if (rev == MV88F5281_REV_D2)
			rev_name = "D2";
		else if (rev == MV88F5281_REV_D1)
			rev_name = "D1";
		else if (rev == MV88F5281_REV_D0)
			rev_name = "D0";
	} else if (dev == MV88F6183_DEV_ID) {
		dev_name = "MV88F6183";
		if (rev == MV88F6183_REV_B0)
			rev_name = "B0";
	}
	if (dev_name == NULL) {
		sprintf(dev_str, "0x%04x", dev);
		dev_name = dev_str;
	}
	if (rev_name == NULL) {
		sprintf(rev_str, "0x%02x", rev);
		rev_name = rev_str;
	}

	printf("SoC:   Orion5x %s-%s\n", dev_name, rev_name);

	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

#ifdef CONFIG_ARCH_CPU_INIT
int arch_cpu_init(void)
{
	/* Enable and invalidate L2 cache in write through mode */
	invalidate_l2_cache();

#ifdef CONFIG_SPL_BUILD
	orion5x_config_adr_windows();
#endif

	return 0;
}
#endif /* CONFIG_ARCH_CPU_INIT */

/*
 * SOC specific misc init
 */
#if defined(CONFIG_ARCH_MISC_INIT)
int arch_misc_init(void)
{
	u32 temp;

	/*CPU streaming & write allocate */
	temp = readfr_extra_feature_reg();
	temp &= ~(1 << 28);	/* disable wr alloc */
	writefr_extra_feature_reg(temp);

	temp = readfr_extra_feature_reg();
	temp &= ~(1 << 29);	/* streaming disabled */
	writefr_extra_feature_reg(temp);

	/* L2Cache settings */
	temp = readfr_extra_feature_reg();
	/* Disable L2C pre fetch - Set bit 24 */
	temp |= (1 << 24);
	/* enable L2C - Set bit 22 */
	temp |= (1 << 22);
	writefr_extra_feature_reg(temp);

	icache_enable();
	/* Change reset vector to address 0x0 */
	temp = get_cr();
	set_cr(temp & ~CR_V);

	/* Set CPIOs and MPPs - values provided by board
	   include file */
	writel(ORION5X_MPP0_7, ORION5X_MPP_BASE+0x00);
	writel(ORION5X_MPP8_15, ORION5X_MPP_BASE+0x04);
	writel(ORION5X_MPP16_23, ORION5X_MPP_BASE+0x50);
	writel(ORION5X_GPIO_OUT_VALUE, ORION5X_GPIO_BASE+0x00);
	writel(ORION5X_GPIO_OUT_ENABLE, ORION5X_GPIO_BASE+0x04);
	writel(ORION5X_GPIO_IN_POLARITY, ORION5X_GPIO_BASE+0x0c);

	/* initialize timer */
	timer_init_r();
	return 0;
}
#endif /* CONFIG_ARCH_MISC_INIT */

#ifdef CONFIG_MVGBE
int cpu_eth_init(bd_t *bis)
{
	mvgbe_initialize(bis);
	return 0;
}
#endif
