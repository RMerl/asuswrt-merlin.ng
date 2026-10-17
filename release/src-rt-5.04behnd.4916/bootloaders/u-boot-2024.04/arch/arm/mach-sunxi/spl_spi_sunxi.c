// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Siarhei Siamashka <siarhei.siamashka@gmail.com>
 */

#include <image.h>
#include <log.h>
#include <spl.h>
#include <asm/arch/spl.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/libfdt.h>
#include <sunxi_gpio.h>

#ifdef CONFIG_SPL_OS_BOOT
#error CONFIG_SPL_OS_BOOT is not supported yet
#endif

/*
 * This is a very simple U-Boot image loading implementation, trying to
 * replicate what the boot ROM is doing when loading the SPL. Because we
 * know the exact pins where the SPI Flash is connected and also know
 * that the Read Data Bytes (03h) command is supported, the hardware
 * configuration is very simple and we don't need the extra flexibility
 * of the SPI framework. Moreover, we rely on the default settings of
 * the SPI controler hardware registers and only adjust what needs to
 * be changed. This is good for the code size and this implementation
 * adds less than 400 bytes to the SPL.
 *
 * There are two variants of the SPI controller in Allwinner SoCs:
 * A10/A13/A20 (sun4i variant) and everything else (sun6i variant).
 * Both of them are supported.
 *
 * The pin mixing part is SoC specific and only A10/A13/A20/H3/A64 are
 * supported at the moment.
 */

/*****************************************************************************/
/* SUN4I variant of the SPI controller                                       */
/*****************************************************************************/

#define SUN4I_SPI0_CCTL             0x1C
#define SUN4I_SPI0_CTL              0x08
#define SUN4I_SPI0_RX               0x00
#define SUN4I_SPI0_TX               0x04
#define SUN4I_SPI0_FIFO_STA         0x28
#define SUN4I_SPI0_BC               0x20
#define SUN4I_SPI0_TC               0x24

#define SUN4I_CTL_ENABLE            BIT(0)
#define SUN4I_CTL_MASTER            BIT(1)
#define SUN4I_CTL_TF_RST            BIT(8)
#define SUN4I_CTL_RF_RST            BIT(9)
#define SUN4I_CTL_XCH               BIT(10)

/*****************************************************************************/
/* SUN6I variant of the SPI controller                                       */
/*****************************************************************************/

#define SUN6I_SPI0_CCTL             0x24
#define SUN6I_SPI0_GCR              0x04
#define SUN6I_SPI0_TCR              0x08
#define SUN6I_SPI0_FIFO_STA         0x1C
#define SUN6I_SPI0_MBC              0x30
#define SUN6I_SPI0_MTC              0x34
#define SUN6I_SPI0_BCC              0x38
#define SUN6I_SPI0_TXD              0x200
#define SUN6I_SPI0_RXD              0x300

#define SUN6I_CTL_ENABLE            BIT(0)
#define SUN6I_CTL_MASTER            BIT(1)
#define SUN6I_CTL_SRST              BIT(31)
#define SUN6I_TCR_XCH               BIT(31)

/*****************************************************************************/

#define CCM_AHB_GATING0             (0x01C20000 + 0x60)
#define CCM_H6_SPI_BGR_REG          (0x03001000 + 0x96c)
#ifdef CONFIG_SUN50I_GEN_H6
#define CCM_SPI0_CLK                (0x03001000 + 0x940)
#else
#define CCM_SPI0_CLK                (0x01C20000 + 0xA0)
#endif
#define SUN6I_BUS_SOFT_RST_REG0     (0x01C20000 + 0x2C0)

#define AHB_RESET_SPI0_SHIFT        20
#define AHB_GATE_OFFSET_SPI0        20

#define SPI0_CLK_DIV_BY_2           0x1000
#define SPI0_CLK_DIV_BY_4           0x1001
#define SPI0_CLK_DIV_BY_32          0x100f

/*****************************************************************************/

/*
 * Allwinner A10/A20 SoCs were using pins PC0,PC1,PC2,PC23 for booting
 * from SPI Flash, everything else is using pins PC0,PC1,PC2,PC3.
 * The H6 uses PC0, PC2, PC3, PC5, the H616 PC0, PC2, PC3, PC4.
 */
static void spi0_pinmux_setup(unsigned int pin_function)
{
	/* All chips use PC0 and PC2. */
	sunxi_gpio_set_cfgpin(SUNXI_GPC(0), pin_function);
	sunxi_gpio_set_cfgpin(SUNXI_GPC(2), pin_function);

	/* All chips except H6 and H616 use PC1. */
	if (!IS_ENABLED(CONFIG_SUN50I_GEN_H6))
		sunxi_gpio_set_cfgpin(SUNXI_GPC(1), pin_function);

	if (IS_ENABLED(CONFIG_MACH_SUN50I_H6))
		sunxi_gpio_set_cfgpin(SUNXI_GPC(5), pin_function);
	if (IS_ENABLED(CONFIG_MACH_SUN50I_H616))
		sunxi_gpio_set_cfgpin(SUNXI_GPC(4), pin_function);

	/* Older generations use PC23 for CS, newer ones use PC3. */
	if (IS_ENABLED(CONFIG_MACH_SUN4I) || IS_ENABLED(CONFIG_MACH_SUN7I) ||
	    IS_ENABLED(CONFIG_MACH_SUN8I_R40))
		sunxi_gpio_set_cfgpin(SUNXI_GPC(23), pin_function);
	else
		sunxi_gpio_set_cfgpin(SUNXI_GPC(3), pin_function);
}

static bool is_sun6i_gen_spi(void)
{
	return IS_ENABLED(CONFIG_SUNXI_GEN_SUN6I) ||
	       IS_ENABLED(CONFIG_SUN50I_GEN_H6);
}

static uintptr_t spi0_base_address(void)
{
	if (IS_ENABLED(CONFIG_MACH_SUN8I_R40))
		return 0x01C05000;

	if (IS_ENABLED(CONFIG_SUN50I_GEN_H6))
		return 0x05010000;

	if (!is_sun6i_gen_spi() ||
	    IS_ENABLED(CONFIG_MACH_SUNIV))
		return 0x01C05000;

	return 0x01C68000;
}

/*
 * Setup 6 MHz from OSC24M (because the BROM is doing the same).
 */
static void spi0_enable_clock(void)
{
	uintptr_t base = spi0_base_address();

	/* Deassert SPI0 reset on SUN6I */
	if (IS_ENABLED(CONFIG_SUN50I_GEN_H6))
		setbits_le32(CCM_H6_SPI_BGR_REG, (1U << 16) | 0x1);
	else if (is_sun6i_gen_spi())
		setbits_le32(SUN6I_BUS_SOFT_RST_REG0,
			     (1 << AHB_RESET_SPI0_SHIFT));

	/* Open the SPI0 gate */
	if (!IS_ENABLED(CONFIG_SUN50I_GEN_H6))
		setbits_le32(CCM_AHB_GATING0, (1 << AHB_GATE_OFFSET_SPI0));

	if (IS_ENABLED(CONFIG_MACH_SUNIV)) {
		/* Divide by 32, clock source is AHB clock 200MHz */
		writel(SPI0_CLK_DIV_BY_32, base + SUN6I_SPI0_CCTL);
	} else {
		/* Divide by 4 */
		writel(SPI0_CLK_DIV_BY_4, base + (is_sun6i_gen_spi() ?
					  SUN6I_SPI0_CCTL : SUN4I_SPI0_CCTL));
		/* 24MHz from OSC24M */
		writel((1 << 31), CCM_SPI0_CLK);
	}

	if (is_sun6i_gen_spi()) {
		/* Enable SPI in the master mode and do a soft reset */
		setbits_le32(base + SUN6I_SPI0_GCR, SUN6I_CTL_MASTER |
			     SUN6I_CTL_ENABLE | SUN6I_CTL_SRST);
		/* Wait for completion */
		while (readl(base + SUN6I_SPI0_GCR) & SUN6I_CTL_SRST)
			;
	} else {
		/* Enable SPI in the master mode and reset FIFO */
		setbits_le32(base + SUN4I_SPI0_CTL, SUN4I_CTL_MASTER |
						    SUN4I_CTL_ENABLE |
						    SUN4I_CTL_TF_RST |
						    SUN4I_CTL_RF_RST);
	}
}

static void spi0_disable_clock(void)
{
	uintptr_t base = spi0_base_address();

	/* Disable the SPI0 controller */
	if (is_sun6i_gen_spi())
		clrbits_le32(base + SUN6I_SPI0_GCR, SUN6I_CTL_MASTER |
					     SUN6I_CTL_ENABLE);
	else
		clrbits_le32(base + SUN4I_SPI0_CTL, SUN4I_CTL_MASTER |
					     SUN4I_CTL_ENABLE);

	/* Disable the SPI0 clock */
	if (!IS_ENABLED(CONFIG_MACH_SUNIV))
		writel(0, CCM_SPI0_CLK);

	/* Close the SPI0 gate */
	if (!IS_ENABLED(CONFIG_SUN50I_GEN_H6))
		clrbits_le32(CCM_AHB_GATING0, (1 << AHB_GATE_OFFSET_SPI0));

	/* Assert SPI0 reset on SUN6I */
	if (IS_ENABLED(CONFIG_SUN50I_GEN_H6))
		clrbits_le32(CCM_H6_SPI_BGR_REG, (1U << 16) | 0x1);
	else if (is_sun6i_gen_spi())
		clrbits_le32(SUN6I_BUS_SOFT_RST_REG0,
			     (1 << AHB_RESET_SPI0_SHIFT));
}

static void spi0_init(void)
{
	unsigned int pin_function = SUNXI_GPC_SPI0;

	if (IS_ENABLED(CONFIG_MACH_SUN50I) ||
	    IS_ENABLED(CONFIG_SUN50I_GEN_H6))
		pin_function = SUN50I_GPC_SPI0;
	else if (IS_ENABLED(CONFIG_MACH_SUNIV))
		pin_function = SUNIV_GPC_SPI0;

	spi0_pinmux_setup(pin_function);
	spi0_enable_clock();
}

static void spi0_deinit(void)
{
	/* New SoCs can disable pins, older could only set them as input */
	unsigned int pin_function = SUNXI_GPIO_INPUT;

	if (is_sun6i_gen_spi())
		pin_function = SUNXI_GPIO_DISABLE;

	spi0_disable_clock();
	spi0_pinmux_setup(pin_function);
}

/*****************************************************************************/

#define SPI_READ_MAX_SIZE 60 /* FIFO size, minus 4 bytes of the header */

static void sunxi_spi0_read_data(u8 *buf, u32 addr, u32 bufsize,
				 ulong spi_ctl_reg,
				 ulong spi_ctl_xch_bitmask,
				 ulong spi_fifo_reg,
				 ulong spi_tx_reg,
				 ulong spi_rx_reg,
				 ulong spi_bc_reg,
				 ulong spi_tc_reg,
				 ulong spi_bcc_reg)
{
	writel(4 + bufsize, spi_bc_reg); /* Burst counter (total bytes) */
	writel(4, spi_tc_reg);           /* Transfer counter (bytes to send) */
	if (spi_bcc_reg)
		writel(4, spi_bcc_reg);  /* SUN6I also needs this */

	/* Send the Read Data Bytes (03h) command header */
	writeb(0x03, spi_tx_reg);
	writeb((u8)(addr >> 16), spi_tx_reg);
	writeb((u8)(addr >> 8), spi_tx_reg);
	writeb((u8)(addr), spi_tx_reg);

	/* Start the data transfer */
	setbits_le32(spi_ctl_reg, spi_ctl_xch_bitmask);

	/* Wait until everything is received in the RX FIFO */
	while ((readl(spi_fifo_reg) & 0x7F) < 4 + bufsize)
		;

	/* Skip 4 bytes */
	readl(spi_rx_reg);

	/* Read the data */
	while (bufsize-- > 0)
		*buf++ = readb(spi_rx_reg);

	/* tSHSL time is up to 100 ns in various SPI flash datasheets */
	udelay(1);
}

static void spi0_read_data(void *buf, u32 addr, u32 len)
{
	u8 *buf8 = buf;
	u32 chunk_len;
	uintptr_t base = spi0_base_address();

	while (len > 0) {
		chunk_len = len;
		if (chunk_len > SPI_READ_MAX_SIZE)
			chunk_len = SPI_READ_MAX_SIZE;

		if (is_sun6i_gen_spi()) {
			sunxi_spi0_read_data(buf8, addr, chunk_len,
					     base + SUN6I_SPI0_TCR,
					     SUN6I_TCR_XCH,
					     base + SUN6I_SPI0_FIFO_STA,
					     base + SUN6I_SPI0_TXD,
					     base + SUN6I_SPI0_RXD,
					     base + SUN6I_SPI0_MBC,
					     base + SUN6I_SPI0_MTC,
					     base + SUN6I_SPI0_BCC);
		} else {
			sunxi_spi0_read_data(buf8, addr, chunk_len,
					     base + SUN4I_SPI0_CTL,
					     SUN4I_CTL_XCH,
					     base + SUN4I_SPI0_FIFO_STA,
					     base + SUN4I_SPI0_TX,
					     base + SUN4I_SPI0_RX,
					     base + SUN4I_SPI0_BC,
					     base + SUN4I_SPI0_TC,
					     0);
		}

		len  -= chunk_len;
		buf8 += chunk_len;
		addr += chunk_len;
	}
}

static ulong spi_load_read(struct spl_load_info *load, ulong sector,
			   ulong count, void *buf)
{
	spi0_read_data(buf, sector, count);

	return count;
}

/*****************************************************************************/

static int spl_spi_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	int ret = 0;
	struct legacy_img_hdr *header;
	uint32_t load_offset = sunxi_get_spl_size();

	header = (struct legacy_img_hdr *)CONFIG_TEXT_BASE;
	load_offset = max_t(uint32_t, load_offset, CONFIG_SYS_SPI_U_BOOT_OFFS);

	spi0_init();

	spi0_read_data((void *)header, load_offset, 0x40);

        if (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
		image_get_magic(header) == FDT_MAGIC) {
		struct spl_load_info load;

		debug("Found FIT image\n");
		spl_set_bl_len(&load, 1);
		load.read = spi_load_read;
		ret = spl_load_simple_fit(spl_image, &load,
					  load_offset, header);
	} else {
		ret = spl_parse_image_header(spl_image, bootdev, header);
		if (ret)
			return ret;

		spi0_read_data((void *)spl_image->load_addr,
			       load_offset, spl_image->size);
	}

	spi0_deinit();

	return ret;
}
/* Use priorty 0 to override the default if it happens to be linked in */
SPL_LOAD_IMAGE_METHOD("sunxi SPI", 0, BOOT_DEVICE_SPI, spl_spi_load_image);
