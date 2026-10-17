// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <ahci.h>
#include <cpu_func.h>
#include <init.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/mbus.h>
#include <asm/io.h>
#include <asm/pl310.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/spl.h>
#include <sdhci.h>

#define DDR_BASE_CS_OFF(n)	(0x0000 + ((n) << 3))
#define DDR_SIZE_CS_OFF(n)	(0x0004 + ((n) << 3))

static const struct mbus_win windows[] = {
	/* SPI */
	{ MBUS_SPI_BASE, MBUS_SPI_SIZE,
	  CPU_TARGET_DEVICEBUS_BOOTROM_SPI, CPU_ATTR_SPIFLASH },

	/* BootROM */
	{ MBUS_BOOTROM_BASE, MBUS_BOOTROM_SIZE,
	  CPU_TARGET_DEVICEBUS_BOOTROM_SPI, CPU_ATTR_BOOTROM },

#ifdef CONFIG_ARMADA_MSYS
	/* DFX */
	{ MBUS_DFX_BASE, MBUS_DFX_SIZE, CPU_TARGET_DFX, 0 },
#endif
};

/* SPI0 CS0 Flash of size MBUS_SPI_SIZE is mapped to address MBUS_SPI_BASE */
#if CONFIG_ENV_SPI_BUS == 0 && CONFIG_ENV_SPI_CS == 0 && \
    CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE <= MBUS_SPI_SIZE
void *env_sf_get_env_addr(void)
{
	return (void *)MBUS_SPI_BASE + CONFIG_ENV_OFFSET;
}
#endif

void lowlevel_init(void)
{
	/*
	 * Dummy implementation, we only need LOWLEVEL_INIT
	 * on Armada to configure CP15 in start.S / cpu_init_cp15()
	 */
}

void reset_cpu(void)
{
	struct mvebu_system_registers *reg =
		(struct mvebu_system_registers *)MVEBU_SYSTEM_REG_BASE;

	writel(readl(&reg->rstoutn_mask) | 1, &reg->rstoutn_mask);
	writel(readl(&reg->sys_soft_rst) | 1, &reg->sys_soft_rst);
	while (1)
		;
}

u32 get_boot_device(void)
{
	u32 val;
	u32 boot_device;
	u32 boot_err_mode;
#ifdef CONFIG_ARMADA_38X
	u32 boot_err_code;
#endif

	/*
	 * First check, if UART boot-mode is active. This can only
	 * be done, via the bootrom error register. Here the
	 * MSB marks if the UART mode is active.
	 */
	val = readl(BOOTROM_ERR_REG);
	boot_err_mode = (val & BOOTROM_ERR_MODE_MASK) >> BOOTROM_ERR_MODE_OFFS;
	debug("BOOTROM_ERR_REG=0x%08x boot_err_mode=0x%x\n", val, boot_err_mode);
	if (boot_err_mode == BOOTROM_ERR_MODE_UART)
		return BOOT_DEVICE_UART;

#ifdef CONFIG_ARMADA_38X
	/*
	 * If the bootrom error code contains any other than zeros it's an
	 * error condition and the bootROM has fallen back to UART boot
	 */
	boot_err_code = (val & BOOTROM_ERR_CODE_MASK) >> BOOTROM_ERR_CODE_OFFS;
	debug("boot_err_code=0x%x\n", boot_err_code);
	if (boot_err_code)
		return BOOT_DEVICE_UART;
#endif

	/*
	 * Now check the SAR register for the strapped boot-device
	 */
	val = readl(CFG_SAR_REG);	/* SAR - Sample At Reset */
	boot_device = (val & BOOT_DEV_SEL_MASK) >> BOOT_DEV_SEL_OFFS;
	debug("SAR_REG=0x%08x boot_device=0x%x\n", val, boot_device);
#ifdef BOOT_FROM_NAND
	if (BOOT_FROM_NAND(boot_device))
		return BOOT_DEVICE_NAND;
#endif
#ifdef BOOT_FROM_MMC
	if (BOOT_FROM_MMC(boot_device))
		return BOOT_DEVICE_MMC1;
#endif
#ifdef BOOT_FROM_UART
	if (BOOT_FROM_UART(boot_device))
		return BOOT_DEVICE_UART;
#endif
#ifdef BOOT_FROM_SATA
	if (BOOT_FROM_SATA(boot_device))
		return BOOT_DEVICE_SATA;
#endif
#ifdef BOOT_FROM_SPI
	if (BOOT_FROM_SPI(boot_device))
		return BOOT_DEVICE_SPI;
#endif
	return BOOT_DEVICE_BOOTROM;
}

#if defined(CONFIG_DISPLAY_CPUINFO)

#if defined(CONFIG_ARMADA_375)
/* SAR frequency values for Armada 375 */
static const struct sar_freq_modes sar_freq_tab[] = {
	{  0,  0x0,  266,  133,  266 },
	{  1,  0x0,  333,  167,  167 },
	{  2,  0x0,  333,  167,  222 },
	{  3,  0x0,  333,  167,  333 },
	{  4,  0x0,  400,  200,  200 },
	{  5,  0x0,  400,  200,  267 },
	{  6,  0x0,  400,  200,  400 },
	{  7,  0x0,  500,  250,  250 },
	{  8,  0x0,  500,  250,  334 },
	{  9,  0x0,  500,  250,  500 },
	{ 10,  0x0,  533,  267,  267 },
	{ 11,  0x0,  533,  267,  356 },
	{ 12,  0x0,  533,  267,  533 },
	{ 13,  0x0,  600,  300,  300 },
	{ 14,  0x0,  600,  300,  400 },
	{ 15,  0x0,  600,  300,  600 },
	{ 16,  0x0,  666,  333,  333 },
	{ 17,  0x0,  666,  333,  444 },
	{ 18,  0x0,  666,  333,  666 },
	{ 19,  0x0,  800,  400,  267 },
	{ 20,  0x0,  800,  400,  400 },
	{ 21,  0x0,  800,  400,  534 },
	{ 22,  0x0,  900,  450,  300 },
	{ 23,  0x0,  900,  450,  450 },
	{ 24,  0x0,  900,  450,  600 },
	{ 25,  0x0, 1000,  500,  500 },
	{ 26,  0x0, 1000,  500,  667 },
	{ 27,  0x0, 1000,  333,  500 },
	{ 28,  0x0,  400,  400,  400 },
	{ 29,  0x0, 1100,  550,  550 },
	{ 0xff, 0xff,    0,   0,   0 }	/* 0xff marks end of array */
};
#elif defined(CONFIG_ARMADA_38X)
/* SAR frequency values for Armada 38x */
static const struct sar_freq_modes sar_freq_tab[] = {
	{  0x0,  0x0,  666,  333, 333 },
	{  0x2,  0x0,  800,  400, 400 },
	{  0x4,  0x0, 1066,  533, 533 },
	{  0x6,  0x0, 1200,  600, 600 },
	{  0x8,  0x0, 1332,  666, 666 },
	{  0xc,  0x0, 1600,  800, 800 },
	{ 0x10,  0x0, 1866,  933, 933 },
	{ 0x13,  0x0, 2000, 1000, 933 },
	{ 0xff, 0xff,    0,    0,   0 }	/* 0xff marks end of array */
};
#elif defined(CONFIG_ARMADA_MSYS)
static const struct sar_freq_modes sar_freq_tab[] = {
	{  0x0,	0x0,  400,  400, 400 },
	{  0x2, 0x0,  667,  333, 667 },
	{  0x3, 0x0,  800,  400, 800 },
	{  0x5, 0x0,  800,  400, 800 },
	{ 0xff, 0xff,    0,   0,   0 }	/* 0xff marks end of array */
};
#else
/* SAR frequency values for Armada XP */
static const struct sar_freq_modes sar_freq_tab[] = {
	{  0xa,  0x5,  800, 400, 400 },
	{  0x1,  0x5, 1066, 533, 533 },
	{  0x2,  0x5, 1200, 600, 600 },
	{  0x2,  0x9, 1200, 600, 400 },
	{  0x3,  0x5, 1333, 667, 667 },
	{  0x4,  0x5, 1500, 750, 750 },
	{  0x4,  0x9, 1500, 750, 500 },
	{  0xb,  0x9, 1600, 800, 533 },
	{  0xb,  0xa, 1600, 800, 640 },
	{  0xb,  0x5, 1600, 800, 800 },
	{ 0xff, 0xff,    0,   0,   0 }	/* 0xff marks end of array */
};
#endif

void get_sar_freq(struct sar_freq_modes *sar_freq)
{
	u32 val;
	u32 freq;
	int i;

#if defined(CONFIG_ARMADA_375) || defined(CONFIG_ARMADA_MSYS)
	val = readl(CFG_SAR2_REG);	/* SAR - Sample At Reset */
#else
	val = readl(CFG_SAR_REG);	/* SAR - Sample At Reset */
#endif
	freq = (val & SAR_CPU_FREQ_MASK) >> SAR_CPU_FREQ_OFFS;
#if defined(SAR2_CPU_FREQ_MASK)
	/*
	 * Shift CPU0 clock frequency select bit from SAR2 register
	 * into correct position
	 */
	freq |= ((readl(CFG_SAR2_REG) & SAR2_CPU_FREQ_MASK)
		 >> SAR2_CPU_FREQ_OFFS) << 3;
#endif
	for (i = 0; sar_freq_tab[i].val != 0xff; i++) {
		if (sar_freq_tab[i].val == freq) {
#if defined(CONFIG_ARMADA_375) || defined(CONFIG_ARMADA_38X) || defined(CONFIG_ARMADA_MSYS)
			*sar_freq = sar_freq_tab[i];
			return;
#else
			int k;
			u8 ffc;

			ffc = (val & SAR_FFC_FREQ_MASK) >>
				SAR_FFC_FREQ_OFFS;
			for (k = i; sar_freq_tab[k].ffc != 0xff; k++) {
				if (sar_freq_tab[k].ffc == ffc) {
					*sar_freq = sar_freq_tab[k];
					return;
				}
			}
			i = k;
#endif
		}
	}

	/* SAR value not found, return 0 for frequencies */
	*sar_freq = sar_freq_tab[i - 1];
}

int print_cpuinfo(void)
{
	u16 devid = (readl(MVEBU_REG_PCIE_DEVID) >> 16) & 0xffff;
	u8 revid = readl(MVEBU_REG_PCIE_REVID) & 0xff;
	struct sar_freq_modes sar_freq;

	puts("SoC:   ");

	switch (devid) {
	case SOC_MV78230_ID:
		puts("MV78230-");
		break;
	case SOC_MV78260_ID:
		puts("MV78260-");
		break;
	case SOC_MV78460_ID:
		puts("MV78460-");
		break;
	case SOC_88F6720_ID:
		puts("MV88F6720-");
		break;
	case SOC_88F6810_ID:
		puts("MV88F6810-");
		break;
	case SOC_88F6820_ID:
		puts("MV88F6820-");
		break;
	case SOC_88F6828_ID:
		puts("MV88F6828-");
		break;
	case SOC_98DX3236_ID:
		puts("98DX3236-");
		break;
	case SOC_98DX3336_ID:
		puts("98DX3336-");
		break;
	case SOC_98DX4251_ID:
		puts("98DX4251-");
		break;
	default:
		puts("Unknown-");
		break;
	}

	switch (devid) {
	case SOC_MV78230_ID:
	case SOC_MV78260_ID:
	case SOC_MV78460_ID:
		switch (revid) {
		case 1:
			puts("A0");
			break;
		case 2:
			puts("B0");
			break;
		default:
			printf("?? (%x)", revid);
			break;
		}
		break;

	case SOC_88F6720_ID:
		switch (revid) {
		case MV_88F67XX_A0_ID:
			puts("A0");
			break;
		default:
			printf("?? (%x)", revid);
			break;
		}
		break;

	case SOC_88F6810_ID:
	case SOC_88F6820_ID:
	case SOC_88F6828_ID:
		switch (revid) {
		case MV_88F68XX_Z1_ID:
			puts("Z1");
			break;
		case MV_88F68XX_A0_ID:
			puts("A0");
			break;
		case MV_88F68XX_B0_ID:
			puts("B0");
			break;
		default:
			printf("?? (%x)", revid);
			break;
		}
		break;

	case SOC_98DX3236_ID:
	case SOC_98DX3336_ID:
	case SOC_98DX4251_ID:
		switch (revid) {
		case 3:
			puts("A0");
			break;
		case 4:
			puts("A1");
			break;
		default:
			printf("?? (%x)", revid);
			break;
		}
		break;

	default:
		printf("?? (%x)", revid);
		break;
	}

	get_sar_freq(&sar_freq);
	printf(" at %d MHz\n", sar_freq.p_clk);

	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

/*
 * This function initialize Controller DRAM Fastpath windows.
 * It takes the CS size information from the 0x1500 scratch registers
 * and sets the correct windows sizes and base addresses accordingly.
 *
 * These values are set in the scratch registers by the Marvell
 * DDR3 training code, which is executed by the SPL before the
 * main payload (U-Boot) is executed.
 */
static void update_sdram_window_sizes(void)
{
	u64 base = 0;
	u32 size, temp;
	int i;

	for (i = 0; i < SDRAM_MAX_CS; i++) {
		size = readl((MVEBU_SDRAM_SCRATCH + (i * 8))) & SDRAM_ADDR_MASK;
		if (size != 0) {
			size |= ~(SDRAM_ADDR_MASK);

			/* Set Base Address */
			temp = (base & 0xFF000000ll) | ((base >> 32) & 0xF);
			writel(temp, MVEBU_SDRAM_BASE + DDR_BASE_CS_OFF(i));

			/*
			 * Check if out of max window size and resize
			 * the window
			 */
			temp = (readl(MVEBU_SDRAM_BASE + DDR_SIZE_CS_OFF(i)) &
				~(SDRAM_ADDR_MASK)) | 1;
			temp |= (size & SDRAM_ADDR_MASK);
			writel(temp, MVEBU_SDRAM_BASE + DDR_SIZE_CS_OFF(i));

			base += ((u64)size + 1);
		} else {
			/*
			 * Disable window if not used, otherwise this
			 * leads to overlapping enabled windows with
			 * pretty strange results
			 */
			clrbits_le32(MVEBU_SDRAM_BASE + DDR_SIZE_CS_OFF(i), 1);
		}
	}
}

#ifdef CONFIG_ARCH_CPU_INIT
#define MV_USB_PHY_BASE			(MVEBU_AXP_USB_BASE + 0x800)
#define MV_USB_PHY_PLL_REG(reg)		(MV_USB_PHY_BASE | (((reg) & 0xF) << 2))
#define MV_USB_X3_BASE(addr)		(MVEBU_AXP_USB_BASE | BIT(11) | \
					 (((addr) & 0xF) << 6))
#define MV_USB_X3_PHY_CHANNEL(dev, reg)	(MV_USB_X3_BASE((dev) + 1) |	\
					 (((reg) & 0xF) << 2))

static void setup_usb_phys(void)
{
	int dev;

	/*
	 * USB PLL init
	 */

	/* Setup PLL frequency */
	/* USB REF frequency = 25 MHz */
	clrsetbits_le32(MV_USB_PHY_PLL_REG(1), 0x3ff, 0x605);

	/* Power up PLL and PHY channel */
	setbits_le32(MV_USB_PHY_PLL_REG(2), BIT(9));

	/* Assert VCOCAL_START */
	setbits_le32(MV_USB_PHY_PLL_REG(1), BIT(21));

	mdelay(1);

	/*
	 * USB PHY init (change from defaults) specific for 40nm (78X30 78X60)
	 */

	for (dev = 0; dev < 3; dev++) {
		setbits_le32(MV_USB_X3_PHY_CHANNEL(dev, 3), BIT(15));

		/* Assert REG_RCAL_START in channel REG 1 */
		setbits_le32(MV_USB_X3_PHY_CHANNEL(dev, 1), BIT(12));
		udelay(40);
		clrbits_le32(MV_USB_X3_PHY_CHANNEL(dev, 1), BIT(12));
	}
}

/*
 * This function is not called from the SPL U-Boot version
 */
int arch_cpu_init(void)
{
	/*
	 * We need to call mvebu_mbus_probe() before calling
	 * update_sdram_window_sizes() as it disables all previously
	 * configured mbus windows and then configures them as
	 * required for U-Boot. Calling update_sdram_window_sizes()
	 * without this configuration will not work, as the internal
	 * registers can't be accessed reliably because of potenial
	 * double mapping.
	 * After updating the SDRAM access windows we need to call
	 * mvebu_mbus_probe() again, as this now correctly configures
	 * the SDRAM areas that are later used by the MVEBU drivers
	 * (e.g. USB, NETA).
	 */

	/*
	 * First disable all windows
	 */
	mvebu_mbus_probe(NULL, 0);

	if (IS_ENABLED(CONFIG_ARMADA_XP)) {
		/*
		 * Now the SDRAM access windows can be reconfigured using
		 * the information in the SDRAM scratch pad registers
		 */
		update_sdram_window_sizes();
	}

	/*
	 * Finally the mbus windows can be configured with the
	 * updated SDRAM sizes
	 */
	mvebu_mbus_probe(windows, ARRAY_SIZE(windows));

	if (IS_ENABLED(CONFIG_ARMADA_XP)) {
		/* Enable GBE0, GBE1, LCD and NFC PUP */
		clrsetbits_le32(ARMADA_XP_PUP_ENABLE, 0,
				GE0_PUP_EN | GE1_PUP_EN | LCD_PUP_EN |
				NAND_PUP_EN | SPI_PUP_EN);

		/* Configure USB PLL and PHYs on AXP */
		setup_usb_phys();
	}

	/* Enable NAND and NAND arbiter */
	clrsetbits_le32(MVEBU_SOC_DEV_MUX_REG, 0, NAND_EN | NAND_ARBITER_EN);

	/* Disable MBUS error propagation */
	clrsetbits_le32(SOC_COHERENCY_FABRIC_CTRL_REG, MBUS_ERR_PROP_EN, 0);

	return 0;
}
#endif /* CONFIG_ARCH_CPU_INIT */

u32 mvebu_get_nand_clock(void)
{
	u32 reg;

	if (IS_ENABLED(CONFIG_ARMADA_38X))
		reg = MVEBU_DFX_DIV_CLK_CTRL(1);
	else if (IS_ENABLED(CONFIG_ARMADA_MSYS))
		reg = MVEBU_DFX_DIV_CLK_CTRL(8);
	else
		reg = MVEBU_CORE_DIV_CLK_CTRL(1);

	return CONFIG_SYS_MVEBU_PLL_CLOCK /
		((readl(reg) &
		  NAND_ECC_DIVCKL_RATIO_MASK) >> NAND_ECC_DIVCKL_RATIO_OFFS);
}

#if defined(CONFIG_MMC_SDHCI_MV) && !defined(CONFIG_DM_MMC)
int board_mmc_init(struct bd_info *bis)
{
	mv_sdh_init(MVEBU_SDIO_BASE, 0, 0,
		    SDHCI_QUIRK_32BIT_DMA_ADDR | SDHCI_QUIRK_WAIT_SEND_CMD);

	return 0;
}
#endif

#define AHCI_VENDOR_SPECIFIC_0_ADDR	0xa0
#define AHCI_VENDOR_SPECIFIC_0_DATA	0xa4

#define AHCI_WINDOW_CTRL(win)		(0x60 + ((win) << 4))
#define AHCI_WINDOW_BASE(win)		(0x64 + ((win) << 4))
#define AHCI_WINDOW_SIZE(win)		(0x68 + ((win) << 4))

static void ahci_mvebu_mbus_config(void __iomem *base)
{
	const struct mbus_dram_target_info *dram;
	int i;

	/* mbus is not initialized in SPL; keep the ROM settings */
	if (IS_ENABLED(CONFIG_SPL_BUILD))
		return;

	dram = mvebu_mbus_dram_info();

	for (i = 0; i < 4; i++) {
		writel(0, base + AHCI_WINDOW_CTRL(i));
		writel(0, base + AHCI_WINDOW_BASE(i));
		writel(0, base + AHCI_WINDOW_SIZE(i));
	}

	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;

		writel((cs->mbus_attr << 8) |
		       (dram->mbus_dram_target_id << 4) | 1,
		       base + AHCI_WINDOW_CTRL(i));
		writel(cs->base >> 16, base + AHCI_WINDOW_BASE(i));
		writel(((cs->size - 1) & 0xffff0000),
		       base + AHCI_WINDOW_SIZE(i));
	}
}

static void ahci_mvebu_regret_option(void __iomem *base)
{
	/*
	 * Enable the regret bit to allow the SATA unit to regret a
	 * request that didn't receive an acknowlegde and avoid a
	 * deadlock
	 */
	writel(0x4, base + AHCI_VENDOR_SPECIFIC_0_ADDR);
	writel(0x80, base + AHCI_VENDOR_SPECIFIC_0_DATA);
}

int board_ahci_enable(void)
{
	ahci_mvebu_mbus_config((void __iomem *)MVEBU_SATA0_BASE);
	ahci_mvebu_regret_option((void __iomem *)MVEBU_SATA0_BASE);

	return 0;
}

#ifdef CONFIG_USB_XHCI_MVEBU
#define USB3_MAX_WINDOWS        4
#define USB3_WIN_CTRL(w)        (0x0 + ((w) * 8))
#define USB3_WIN_BASE(w)        (0x4 + ((w) * 8))

static void xhci_mvebu_mbus_config(void __iomem *base,
			const struct mbus_dram_target_info *dram)
{
	int i;

	for (i = 0; i < USB3_MAX_WINDOWS; i++) {
		writel(0, base + USB3_WIN_CTRL(i));
		writel(0, base + USB3_WIN_BASE(i));
	}

	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;

		/* Write size, attributes and target id to control register */
		writel(((cs->size - 1) & 0xffff0000) | (cs->mbus_attr << 8) |
			(dram->mbus_dram_target_id << 4) | 1,
			base + USB3_WIN_CTRL(i));

		/* Write base address to base register */
		writel((cs->base & 0xffff0000), base + USB3_WIN_BASE(i));
	}
}

int board_xhci_enable(fdt_addr_t base)
{
	const struct mbus_dram_target_info *dram;

	printf("MVEBU XHCI INIT controller @ 0x%llx\n", (fdt64_t)base);

	dram = mvebu_mbus_dram_info();
	xhci_mvebu_mbus_config((void __iomem *)base, dram);

	return 0;
}
#endif

void enable_caches(void)
{
	/* Avoid problem with e.g. neta ethernet driver */
	invalidate_dcache_all();

	/*
	 * Armada 375 still has some problems with d-cache enabled in the
	 * ethernet driver (mvpp2). So lets keep the d-cache disabled
	 * until this is solved.
	 */
	if (!IS_ENABLED(CONFIG_ARMADA_375)) {
		/* Enable D-cache. I-cache is already enabled in start.S */
		dcache_enable();
	}
}

void v7_outer_cache_enable(void)
{
	struct pl310_regs *const pl310 =
		(struct pl310_regs *)CFG_SYS_PL310_BASE;

	/* The L2 cache is already disabled at this point */

	/*
	 * For now L2 cache will be enabled only for Armada XP and Armada 38x.
	 * It can be enabled also for other SoCs after testing that it works fine.
	 */
	if (!IS_ENABLED(CONFIG_ARMADA_XP) && !IS_ENABLED(CONFIG_ARMADA_38X))
		return;

	if (IS_ENABLED(CONFIG_ARMADA_XP)) {
		u32 u;

		/*
		 * For Aurora cache in no outer mode, enable via the CP15
		 * coprocessor broadcasting of cache commands to L2.
		 */
		asm volatile("mrc p15, 1, %0, c15, c2, 0" : "=r" (u));
		u |= BIT(8);		/* Set the FW bit */
		asm volatile("mcr p15, 1, %0, c15, c2, 0" : : "r" (u));

		isb();
	}

	/* Enable the L2 cache */
	setbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);
}

void v7_outer_cache_disable(void)
{
	struct pl310_regs *const pl310 =
		(struct pl310_regs *)CFG_SYS_PL310_BASE;

	clrbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);
}
