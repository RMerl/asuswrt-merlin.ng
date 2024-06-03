// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Some init for sunxi platform.
 */

#include <common.h>
#include <mmc.h>
#include <i2c.h>
#include <serial.h>
#include <spl.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/spl.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/timer.h>
#include <asm/arch/tzpc.h>
#include <asm/arch/mmc.h>

#include <linux/compiler.h>

struct fel_stash {
	uint32_t sp;
	uint32_t lr;
	uint32_t cpsr;
	uint32_t sctlr;
	uint32_t vbar;
	uint32_t cr;
};

struct fel_stash fel_stash __attribute__((section(".data")));

#ifdef CONFIG_ARM64
#include <asm/armv8/mmu.h>

static struct mm_region sunxi_mem_map[] = {
	{
		/* SRAM, MMIO regions */
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	}, {
		/* RAM */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0xC0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};
struct mm_region *mem_map = sunxi_mem_map;
#endif

static int gpio_init(void)
{
#if CONFIG_CONS_INDEX == 1 && defined(CONFIG_UART0_PORT_F)
#if defined(CONFIG_MACH_SUN4I) || \
    defined(CONFIG_MACH_SUN7I) || \
    defined(CONFIG_MACH_SUN8I_R40)
	/* disable GPB22,23 as uart0 tx,rx to avoid conflict */
	sunxi_gpio_set_cfgpin(SUNXI_GPB(22), SUNXI_GPIO_INPUT);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(23), SUNXI_GPIO_INPUT);
#endif
#if defined(CONFIG_MACH_SUN8I) && !defined(CONFIG_MACH_SUN8I_R40)
	sunxi_gpio_set_cfgpin(SUNXI_GPF(2), SUN8I_GPF_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPF(4), SUN8I_GPF_UART0);
#else
	sunxi_gpio_set_cfgpin(SUNXI_GPF(2), SUNXI_GPF_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPF(4), SUNXI_GPF_UART0);
#endif
	sunxi_gpio_set_pull(SUNXI_GPF(4), 1);
#elif CONFIG_CONS_INDEX == 1 && (defined(CONFIG_MACH_SUN4I) || \
				 defined(CONFIG_MACH_SUN7I) || \
				 defined(CONFIG_MACH_SUN8I_R40))
	sunxi_gpio_set_cfgpin(SUNXI_GPB(22), SUN4I_GPB_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(23), SUN4I_GPB_UART0);
	sunxi_gpio_set_pull(SUNXI_GPB(23), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 1 && defined(CONFIG_MACH_SUN5I)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(19), SUN5I_GPB_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(20), SUN5I_GPB_UART0);
	sunxi_gpio_set_pull(SUNXI_GPB(20), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 1 && defined(CONFIG_MACH_SUN6I)
	sunxi_gpio_set_cfgpin(SUNXI_GPH(20), SUN6I_GPH_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPH(21), SUN6I_GPH_UART0);
	sunxi_gpio_set_pull(SUNXI_GPH(21), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 1 && defined(CONFIG_MACH_SUN8I_A33)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(0), SUN8I_A33_GPB_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(1), SUN8I_A33_GPB_UART0);
	sunxi_gpio_set_pull(SUNXI_GPB(1), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 1 && defined(CONFIG_MACH_SUNXI_H3_H5)
	sunxi_gpio_set_cfgpin(SUNXI_GPA(4), SUN8I_H3_GPA_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPA(5), SUN8I_H3_GPA_UART0);
	sunxi_gpio_set_pull(SUNXI_GPA(5), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 1 && defined(CONFIG_MACH_SUN50I)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(8), SUN50I_GPB_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(9), SUN50I_GPB_UART0);
	sunxi_gpio_set_pull(SUNXI_GPB(9), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 1 && defined(CONFIG_MACH_SUN50I_H6)
	sunxi_gpio_set_cfgpin(SUNXI_GPH(0), SUN50I_H6_GPH_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPH(1), SUN50I_H6_GPH_UART0);
	sunxi_gpio_set_pull(SUNXI_GPH(1), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 1 && defined(CONFIG_MACH_SUN8I_A83T)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(9), SUN8I_A83T_GPB_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(10), SUN8I_A83T_GPB_UART0);
	sunxi_gpio_set_pull(SUNXI_GPB(10), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 1 && defined(CONFIG_MACH_SUN8I_V3S)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(8), SUN8I_V3S_GPB_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(9), SUN8I_V3S_GPB_UART0);
	sunxi_gpio_set_pull(SUNXI_GPB(9), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 1 && defined(CONFIG_MACH_SUN9I)
	sunxi_gpio_set_cfgpin(SUNXI_GPH(12), SUN9I_GPH_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPH(13), SUN9I_GPH_UART0);
	sunxi_gpio_set_pull(SUNXI_GPH(13), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 2 && defined(CONFIG_MACH_SUN5I)
	sunxi_gpio_set_cfgpin(SUNXI_GPG(3), SUN5I_GPG_UART1);
	sunxi_gpio_set_cfgpin(SUNXI_GPG(4), SUN5I_GPG_UART1);
	sunxi_gpio_set_pull(SUNXI_GPG(4), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 3 && defined(CONFIG_MACH_SUN8I)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(0), SUN8I_GPB_UART2);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(1), SUN8I_GPB_UART2);
	sunxi_gpio_set_pull(SUNXI_GPB(1), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 5 && defined(CONFIG_MACH_SUN8I)
	sunxi_gpio_set_cfgpin(SUNXI_GPL(2), SUN8I_GPL_R_UART);
	sunxi_gpio_set_cfgpin(SUNXI_GPL(3), SUN8I_GPL_R_UART);
	sunxi_gpio_set_pull(SUNXI_GPL(3), SUNXI_GPIO_PULL_UP);
#else
#error Unsupported console port number. Please fix pin mux settings in board.c
#endif

	return 0;
}

#if defined(CONFIG_SPL_BOARD_LOAD_IMAGE) && defined(CONFIG_SPL_BUILD)
static int spl_board_load_image(struct spl_image_info *spl_image,
				struct spl_boot_device *bootdev)
{
	debug("Returning to FEL sp=%x, lr=%x\n", fel_stash.sp, fel_stash.lr);
	return_to_fel(fel_stash.sp, fel_stash.lr);

	return 0;
}
SPL_LOAD_IMAGE_METHOD("FEL", 0, BOOT_DEVICE_BOARD, spl_board_load_image);
#endif

void s_init(void)
{
	/*
	 * Undocumented magic taken from boot0, without this DRAM
	 * access gets messed up (seems cache related).
	 * The boot0 sources describe this as: "config ema for cache sram"
	 */
#if defined CONFIG_MACH_SUN6I
	setbits_le32(SUNXI_SRAMC_BASE + 0x44, 0x1800);
#elif defined CONFIG_MACH_SUN8I
	__maybe_unused uint version;

	/* Unlock sram version info reg, read it, relock */
	setbits_le32(SUNXI_SRAMC_BASE + 0x24, (1 << 15));
	version = readl(SUNXI_SRAMC_BASE + 0x24) >> 16;
	clrbits_le32(SUNXI_SRAMC_BASE + 0x24, (1 << 15));

	/*
	 * Ideally this would be a switch case, but we do not know exactly
	 * which versions there are and which version needs which settings,
	 * so reproduce the per SoC code from the BSP.
	 */
#if defined CONFIG_MACH_SUN8I_A23
	if (version == 0x1650)
		setbits_le32(SUNXI_SRAMC_BASE + 0x44, 0x1800);
	else /* 0x1661 ? */
		setbits_le32(SUNXI_SRAMC_BASE + 0x44, 0xc0);
#elif defined CONFIG_MACH_SUN8I_A33
	if (version != 0x1667)
		setbits_le32(SUNXI_SRAMC_BASE + 0x44, 0xc0);
#endif
	/* A83T BSP never modifies SUNXI_SRAMC_BASE + 0x44 */
	/* No H3 BSP, boot0 seems to not modify SUNXI_SRAMC_BASE + 0x44 */
#endif

#if !defined(CONFIG_ARM_CORTEX_CPU_IS_UP) && !defined(CONFIG_ARM64)
	/* Enable SMP mode for CPU0, by setting bit 6 of Auxiliary Ctl reg */
	asm volatile(
		"mrc p15, 0, r0, c1, c0, 1\n"
		"orr r0, r0, #1 << 6\n"
		"mcr p15, 0, r0, c1, c0, 1\n"
		::: "r0");
#endif
#if defined CONFIG_MACH_SUN6I || defined CONFIG_MACH_SUN8I_H3
	/* Enable non-secure access to some peripherals */
	tzpc_init();
#endif

	clock_init();
	timer_init();
	gpio_init();
#ifndef CONFIG_DM_I2C
	i2c_init_board();
#endif
	eth_init_board();
}

/* The sunxi internal brom will try to loader external bootloader
 * from mmc0, nand flash, mmc2.
 */
uint32_t sunxi_get_boot_device(void)
{
	int boot_source;

	/*
	 * When booting from the SD card or NAND memory, the "eGON.BT0"
	 * signature is expected to be found in memory at the address 0x0004
	 * (see the "mksunxiboot" tool, which generates this header).
	 *
	 * When booting in the FEL mode over USB, this signature is patched in
	 * memory and replaced with something else by the 'fel' tool. This other
	 * signature is selected in such a way, that it can't be present in a
	 * valid bootable SD card image (because the BROM would refuse to
	 * execute the SPL in this case).
	 *
	 * This checks for the signature and if it is not found returns to
	 * the FEL code in the BROM to wait and receive the main u-boot
	 * binary over USB. If it is found, it determines where SPL was
	 * read from.
	 */
	if (!is_boot0_magic(SPL_ADDR + 4)) /* eGON.BT0 */
		return BOOT_DEVICE_BOARD;

	boot_source = readb(SPL_ADDR + 0x28);
	switch (boot_source) {
	case SUNXI_BOOTED_FROM_MMC0:
	case SUNXI_BOOTED_FROM_MMC0_HIGH:
		return BOOT_DEVICE_MMC1;
	case SUNXI_BOOTED_FROM_NAND:
		return BOOT_DEVICE_NAND;
	case SUNXI_BOOTED_FROM_MMC2:
	case SUNXI_BOOTED_FROM_MMC2_HIGH:
		return BOOT_DEVICE_MMC2;
	case SUNXI_BOOTED_FROM_SPI:
		return BOOT_DEVICE_SPI;
	}

	panic("Unknown boot source %d\n", boot_source);
	return -1;		/* Never reached */
}

#ifdef CONFIG_SPL_BUILD
u32 spl_boot_device(void)
{
	return sunxi_get_boot_device();
}

void board_init_f(ulong dummy)
{
	spl_init();
	preloader_console_init();

#ifdef CONFIG_SPL_I2C_SUPPORT
	/* Needed early by sunxi_board_init if PMU is enabled */
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif
	sunxi_board_init();
}
#endif

void reset_cpu(ulong addr)
{
#if defined(CONFIG_SUNXI_GEN_SUN4I) || defined(CONFIG_MACH_SUN8I_R40)
	static const struct sunxi_wdog *wdog =
		 &((struct sunxi_timer_reg *)SUNXI_TIMER_BASE)->wdog;

	/* Set the watchdog for its shortest interval (.5s) and wait */
	writel(WDT_MODE_RESET_EN | WDT_MODE_EN, &wdog->mode);
	writel(WDT_CTRL_KEY | WDT_CTRL_RESTART, &wdog->ctl);

	while (1) {
		/* sun5i sometimes gets stuck without this */
		writel(WDT_MODE_RESET_EN | WDT_MODE_EN, &wdog->mode);
	}
#elif defined(CONFIG_SUNXI_GEN_SUN6I) || defined(CONFIG_MACH_SUN50I_H6)
#if defined(CONFIG_MACH_SUN50I_H6)
	/* WDOG is broken for some H6 rev. use the R_WDOG instead */
	static const struct sunxi_wdog *wdog =
		(struct sunxi_wdog *)SUNXI_R_WDOG_BASE;
#else
	static const struct sunxi_wdog *wdog =
		((struct sunxi_timer_reg *)SUNXI_TIMER_BASE)->wdog;
#endif
	/* Set the watchdog for its shortest interval (.5s) and wait */
	writel(WDT_CFG_RESET, &wdog->cfg);
	writel(WDT_MODE_EN, &wdog->mode);
	writel(WDT_CTRL_KEY | WDT_CTRL_RESTART, &wdog->ctl);
	while (1) { }
#endif
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF) && !defined(CONFIG_ARM64)
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
