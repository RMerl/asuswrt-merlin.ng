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

#include <cpu_func.h>
#include <init.h>
#include <log.h>
#include <mmc.h>
#include <i2c.h>
#include <serial.h>
#include <spl.h>
#include <sunxi_gpio.h>
#include <asm/cache.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
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
};

struct fel_stash fel_stash __section(".data");

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
		.size = CONFIG_SUNXI_DRAM_MAX_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};
struct mm_region *mem_map = sunxi_mem_map;

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	/* Some devices (like the EMAC) have a 32-bit DMA limit. */
	if (gd->ram_top > (1ULL << 32))
		return 1ULL << 32;

	return gd->ram_top;
}
#endif /* CONFIG_ARM64 */

#ifdef CONFIG_SPL_BUILD
static int gpio_init(void)
{
	__maybe_unused uint val;
#if CONFIG_CONS_INDEX == 1 && defined(CONFIG_UART0_PORT_F)
#if defined(CONFIG_MACH_SUN4I) || \
    defined(CONFIG_MACH_SUN7I) || \
    defined(CONFIG_MACH_SUN8I_R40)
	/* disable GPB22,23 as uart0 tx,rx to avoid conflict */
	sunxi_gpio_set_cfgpin(SUNXI_GPB(22), SUNXI_GPIO_INPUT);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(23), SUNXI_GPIO_INPUT);
#endif
#if defined(CONFIG_MACH_SUN4I) || defined(CONFIG_MACH_SUN5I) || \
    defined(CONFIG_MACH_SUN7I) || defined(CONFIG_MACH_SUN8I_R40) || \
    defined(CONFIG_MACH_SUN9I)
	sunxi_gpio_set_cfgpin(SUNXI_GPF(2), SUNXI_GPF_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPF(4), SUNXI_GPF_UART0);
#else
	sunxi_gpio_set_cfgpin(SUNXI_GPF(2), SUN8I_GPF_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPF(4), SUN8I_GPF_UART0);
#endif
	sunxi_gpio_set_pull(SUNXI_GPF(4), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 1 && defined(CONFIG_MACH_SUNIV)
	sunxi_gpio_set_cfgpin(SUNXI_GPE(0), SUNIV_GPE_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPE(1), SUNIV_GPE_UART0);
	sunxi_gpio_set_pull(SUNXI_GPE(1), SUNXI_GPIO_PULL_UP);
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
#elif CONFIG_CONS_INDEX == 1 && defined(CONFIG_MACH_SUN50I_H616)
	sunxi_gpio_set_cfgpin(SUNXI_GPH(0), SUN50I_H616_GPH_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPH(1), SUN50I_H616_GPH_UART0);
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
#elif CONFIG_CONS_INDEX == 1 && defined(CONFIG_MACH_SUN8I_R528)
	sunxi_gpio_set_cfgpin(SUNXI_GPE(2), 6);
	sunxi_gpio_set_cfgpin(SUNXI_GPE(3), 6);
	sunxi_gpio_set_pull(SUNXI_GPE(3), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 2 && defined(CONFIG_MACH_SUNIV)
	sunxi_gpio_set_cfgpin(SUNXI_GPA(2), SUNIV_GPE_UART0);
	sunxi_gpio_set_cfgpin(SUNXI_GPA(3), SUNIV_GPE_UART0);
	sunxi_gpio_set_pull(SUNXI_GPA(3), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 2 && defined(CONFIG_MACH_SUN5I)
	sunxi_gpio_set_cfgpin(SUNXI_GPG(3), SUN5I_GPG_UART1);
	sunxi_gpio_set_cfgpin(SUNXI_GPG(4), SUN5I_GPG_UART1);
	sunxi_gpio_set_pull(SUNXI_GPG(4), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 3 && defined(CONFIG_MACH_SUN8I_H3)
	sunxi_gpio_set_cfgpin(SUNXI_GPA(0), SUN8I_H3_GPA_UART2);
	sunxi_gpio_set_cfgpin(SUNXI_GPA(1), SUN8I_H3_GPA_UART2);
	sunxi_gpio_set_pull(SUNXI_GPA(1), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 3 && defined(CONFIG_MACH_SUN8I)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(0), SUN8I_GPB_UART2);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(1), SUN8I_GPB_UART2);
	sunxi_gpio_set_pull(SUNXI_GPB(1), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 4 && defined(CONFIG_MACH_SUN8I_R528)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(6), 7);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(7), 7);
	sunxi_gpio_set_pull(SUNXI_GPB(7), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 5 && defined(CONFIG_MACH_SUN8I)
	sunxi_gpio_set_cfgpin(SUNXI_GPL(2), SUN8I_GPL_R_UART);
	sunxi_gpio_set_cfgpin(SUNXI_GPL(3), SUN8I_GPL_R_UART);
	sunxi_gpio_set_pull(SUNXI_GPL(3), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 2 && defined(CONFIG_MACH_SUN8I) && \
				!defined(CONFIG_MACH_SUN8I_R40)
	sunxi_gpio_set_cfgpin(SUNXI_GPG(6), SUN8I_GPG_UART1);
	sunxi_gpio_set_cfgpin(SUNXI_GPG(7), SUN8I_GPG_UART1);
	sunxi_gpio_set_pull(SUNXI_GPG(7), SUNXI_GPIO_PULL_UP);
#else
#error Unsupported console port number. Please fix pin mux settings in board.c
#endif

	/*
	 * Update PIO power bias configuration by copying the hardware
	 * detected value.
	 */
	if (IS_ENABLED(CONFIG_SUN50I_GEN_H6) ||
	    IS_ENABLED(CONFIG_SUN50I_GEN_NCAT2)) {
		val = readl(SUNXI_PIO_BASE + SUN50I_H6_GPIO_POW_MOD_VAL);
		writel(val, SUNXI_PIO_BASE + SUN50I_H6_GPIO_POW_MOD_SEL);
	}
	if (IS_ENABLED(CONFIG_SUN50I_GEN_H6)) {
		val = readl(SUNXI_R_PIO_BASE + SUN50I_H6_GPIO_POW_MOD_VAL);
		writel(val, SUNXI_R_PIO_BASE + SUN50I_H6_GPIO_POW_MOD_SEL);
	}

	return 0;
}

static int spl_board_load_image(struct spl_image_info *spl_image,
				struct spl_boot_device *bootdev)
{
	debug("Returning to FEL sp=%x, lr=%x\n", fel_stash.sp, fel_stash.lr);
	return_to_fel(fel_stash.sp, fel_stash.lr);

	return 0;
}
SPL_LOAD_IMAGE_METHOD("FEL", 0, BOOT_DEVICE_BOARD, spl_board_load_image);
#endif /* CONFIG_SPL_BUILD */

#define SUNXI_INVALID_BOOT_SOURCE	-1

static int suniv_get_boot_source(void)
{
	/* Get the last function call from BootROM's stack. */
	u32 brom_call = *(u32 *)(uintptr_t)(fel_stash.sp - 4);

	/* translate SUNIV BootROM stack to standard SUNXI boot sources */
	switch (brom_call) {
	case SUNIV_BOOTED_FROM_MMC0:
		return SUNXI_BOOTED_FROM_MMC0;
	case SUNIV_BOOTED_FROM_SPI:
		return SUNXI_BOOTED_FROM_SPI;
	case SUNIV_BOOTED_FROM_MMC1:
		return SUNXI_BOOTED_FROM_MMC2;
	/* SPI NAND is not supported yet. */
	case SUNIV_BOOTED_FROM_NAND:
		return SUNXI_INVALID_BOOT_SOURCE;
	}
	/* If we get here something went wrong try to boot from FEL.*/
	printf("Unknown boot source from BROM: 0x%x\n", brom_call);
	return SUNXI_INVALID_BOOT_SOURCE;
}

static int sunxi_egon_valid(struct boot_file_head *egon_head)
{
	return !memcmp(egon_head->magic, BOOT0_MAGIC, 8); /* eGON.BT0 */
}

static int sunxi_toc0_valid(struct toc0_main_info *toc0_info)
{
	return !memcmp(toc0_info->name, TOC0_MAIN_INFO_NAME, 8); /* TOC0.GLH */
}

static int sunxi_get_boot_source(void)
{
	struct boot_file_head *egon_head = (void *)SPL_ADDR;
	struct toc0_main_info *toc0_info = (void *)SPL_ADDR;

	/*
	 * On the ARMv5 SoCs, the SPL header in SRAM is overwritten by the
	 * exception vectors in U-Boot proper, so we won't find any
	 * information there. Also the FEL stash is only valid in the SPL,
	 * so we can't use that either. So if this is called from U-Boot
	 * proper, just return MMC0 as a placeholder, for now.
	 */
	if (IS_ENABLED(CONFIG_MACH_SUNIV) &&
	    !IS_ENABLED(CONFIG_SPL_BUILD))
		return SUNXI_BOOTED_FROM_MMC0;

	if (IS_ENABLED(CONFIG_MACH_SUNIV))
		return suniv_get_boot_source();
	if (sunxi_egon_valid(egon_head))
		return readb(&egon_head->boot_media);
	if (sunxi_toc0_valid(toc0_info))
		return readb(&toc0_info->platform[0]);

	/* Not a valid image, so we must have been booted via FEL. */
	return SUNXI_INVALID_BOOT_SOURCE;
}

/* The sunxi internal brom will try to loader external bootloader
 * from mmc0, nand flash, mmc2.
 */
uint32_t sunxi_get_boot_device(void)
{
	int boot_source = sunxi_get_boot_source();

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
	switch (boot_source) {
	case SUNXI_INVALID_BOOT_SOURCE:
		return BOOT_DEVICE_BOARD;
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
uint32_t sunxi_get_spl_size(void)
{
	struct boot_file_head *egon_head = (void *)SPL_ADDR;
	struct toc0_main_info *toc0_info = (void *)SPL_ADDR;

	if (sunxi_egon_valid(egon_head))
		return readl(&egon_head->length);
	if (sunxi_toc0_valid(toc0_info))
		return readl(&toc0_info->length);

	/* Not a valid image, so use the default U-Boot offset. */
	return 0;
}

/*
 * The eGON SPL image can be located at 8KB or at 128KB into an SD card or
 * an eMMC device. The boot source has bit 4 set in the latter case.
 * By adding 120KB to the normal offset when booting from a "high" location
 * we can support both cases.
 * Also U-Boot proper is located at least 32KB after the SPL, but will
 * immediately follow the SPL if that is bigger than that.
 */
unsigned long board_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						 unsigned long raw_sect)
{
	unsigned long spl_size = sunxi_get_spl_size();
	unsigned long sector;

	sector = max(raw_sect, spl_size / 512);

	switch (sunxi_get_boot_source()) {
	case SUNXI_BOOTED_FROM_MMC0_HIGH:
	case SUNXI_BOOTED_FROM_MMC2_HIGH:
		sector += (128 - 8) * 2;
		break;
	}

	return sector;
}

u32 spl_boot_device(void)
{
	return sunxi_get_boot_device();
}

__weak void sunxi_sram_init(void)
{
}

/*
 * When booting from an eMMC boot partition, the SPL puts the same boot
 * source code into SRAM A1 as when loading the SPL from the normal
 * eMMC user data partition: 0x2. So to know where we have been loaded
 * from, we repeat the BROM algorithm here: checking for a valid eGON boot
 * image at offset 0 of a (potentially) selected boot partition.
 * If any of the conditions is not met, it must have been the eMMC user
 * data partition.
 */
static bool sunxi_valid_emmc_boot(struct mmc *mmc)
{
	struct blk_desc *bd = mmc_get_blk_desc(mmc);
	u32 *buffer = (void *)(uintptr_t)CONFIG_TEXT_BASE;
	struct boot_file_head *egon_head = (void *)buffer;
	struct toc0_main_info *toc0_info = (void *)buffer;
	int bootpart = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	uint32_t spl_size, emmc_checksum, chksum = 0;
	ulong count;

	/* The BROM requires BOOT_ACK to be enabled. */
	if (!EXT_CSD_EXTRACT_BOOT_ACK(mmc->part_config))
		return false;

	/*
	 * The BOOT_BUS_CONDITION register must be 4-bit SDR, with (0x09)
	 * or without (0x01) high speed timings.
	 */
	if ((mmc->ext_csd[EXT_CSD_BOOT_BUS_WIDTH] & 0x1b) != 0x01 &&
	    (mmc->ext_csd[EXT_CSD_BOOT_BUS_WIDTH] & 0x1b) != 0x09)
		return false;

	/* Partition 0 is the user data partition, bootpart must be 1 or 2. */
	if (bootpart != 1 && bootpart != 2)
		return false;

	/* Failure to switch to the boot partition is fatal. */
	if (mmc_switch_part(mmc, bootpart))
		return false;

	/* Read the first block to do some sanity checks on the eGON header. */
	count = blk_dread(bd, 0, 1, buffer);
	if (count != 1)
		return false;

	if (sunxi_egon_valid(egon_head))
		spl_size = egon_head->length;
	else if (sunxi_toc0_valid(toc0_info))
		spl_size = toc0_info->length;
	else
		return false;

	/* Read the rest of the SPL now we know it's halfway sane. */
	count = blk_dread(bd, 1, DIV_ROUND_UP(spl_size, bd->blksz) - 1,
			  buffer + bd->blksz / 4);

	/* Save the checksum and replace it with the "stamp value". */
	emmc_checksum = buffer[3];
	buffer[3] = 0x5f0a6c39;

	/* The checksum is a simple ignore-carry addition of all words. */
	for (count = 0; count < spl_size / 4; count++)
		chksum += buffer[count];

	debug("eMMC boot part SPL checksum: stored: 0x%08x, computed: 0x%08x\n",
	       emmc_checksum, chksum);

	return emmc_checksum == chksum;
}

u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	static u32 result = ~0;

	if (result != ~0)
		return result;

	result = MMCSD_MODE_RAW;
	if (!IS_SD(mmc) && IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT)) {
		if (sunxi_valid_emmc_boot(mmc))
			result = MMCSD_MODE_EMMCBOOT;
		else
			mmc_switch_part(mmc, 0);
	}

	debug("%s(): %s part\n", __func__,
	      result == MMCSD_MODE_RAW ? "user" : "boot");

	return result;
}

void board_init_f(ulong dummy)
{
	sunxi_sram_init();

#if defined CONFIG_MACH_SUN6I || defined CONFIG_MACH_SUN8I_H3
	/* Enable non-secure access to some peripherals */
	tzpc_init();
#endif

	clock_init();
	timer_init();
	gpio_init();

	spl_init();
	preloader_console_init();

#if CONFIG_IS_ENABLED(I2C) && CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	/* Needed early by sunxi_board_init if PMU is enabled */
	i2c_init_board();
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif
	sunxi_board_init();
}
#endif /* CONFIG_SPL_BUILD */

#if !CONFIG_IS_ENABLED(SYSRESET)
void reset_cpu(void)
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
#elif defined(CONFIG_SUNXI_GEN_SUN6I) || defined(CONFIG_SUN50I_GEN_H6) || defined(CONFIG_SUNXI_GEN_NCAT2)
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
#endif /* CONFIG_SYSRESET */

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF) && defined(CONFIG_CPU_V7A)
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
