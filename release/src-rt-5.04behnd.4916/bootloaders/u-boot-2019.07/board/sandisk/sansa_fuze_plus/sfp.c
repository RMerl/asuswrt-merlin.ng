// SPDX-License-Identifier: GPL-2.0+
/*
 * SanDisk Sansa Fuze Plus board
 *
 * Copyright (C) 2013 Marek Vasut <marex@denx.de>
 *
 * Hardware investigation done by:
 *
 * Amaury Pouly <amaury.pouly@gmail.com>
 */

#include <common.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/iomux-mx23.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Functions
 */
int board_early_init_f(void)
{
	/* IO0 clock at 480MHz */
	mxs_set_ioclk(MXC_IOCLK0, 480000);

	/* SSP0 clock at 96MHz */
	mxs_set_sspclk(MXC_SSPCLK0, 96000, 0);

	return 0;
}

int dram_init(void)
{
	return mxs_dram_init();
}

#ifdef	CONFIG_CMD_MMC
static int xfi3_mmc_cd(int id)
{
	switch (id) {
	case 0:
		/* The SSP_DETECT is inverted on this board. */
		return gpio_get_value(MX23_PAD_SSP1_DETECT__GPIO_2_1);
	case 1:
		/* Internal eMMC always present */
		return 1;
	default:
		return 0;
	}
}

int board_mmc_init(bd_t *bis)
{
	int ret;

	/* MicroSD slot */
	gpio_direction_input(MX23_PAD_SSP1_DETECT__GPIO_2_1);
	gpio_direction_output(MX23_PAD_GPMI_D08__GPIO_0_8, 0);
	ret = mxsmmc_initialize(bis, 0, NULL, xfi3_mmc_cd);
	if (ret)
		return ret;

	/* Internal eMMC */
	gpio_direction_output(MX23_PAD_PWM3__GPIO_1_29, 0);
	ret = mxsmmc_initialize(bis, 1, NULL, xfi3_mmc_cd);

	return ret;
}
#endif

#ifdef CONFIG_VIDEO_MXS
#define	MUX_CONFIG_LCD	(MXS_PAD_3V3 | MXS_PAD_4MA | MXS_PAD_NOPULL)
const iomux_cfg_t iomux_lcd_gpio[] = {
	MX23_PAD_LCD_D00__GPIO_1_0 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D01__GPIO_1_1 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D02__GPIO_1_2 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D03__GPIO_1_3 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D04__GPIO_1_4 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D05__GPIO_1_5 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D06__GPIO_1_6 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D07__GPIO_1_7 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D08__GPIO_1_8 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D09__GPIO_1_9 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D10__GPIO_1_10 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D11__GPIO_1_11 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D12__GPIO_1_12 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D13__GPIO_1_13 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D14__GPIO_1_14 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D15__GPIO_1_15 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D16__GPIO_1_16 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D17__GPIO_1_17 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_RESET__GPIO_1_18 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_RS__GPIO_1_19 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_WR__GPIO_1_20 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_CS__GPIO_1_21 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_ENABLE__GPIO_1_23 | MUX_CONFIG_LCD,
};

const iomux_cfg_t iomux_lcd_lcd[] = {
	MX23_PAD_LCD_D00__LCD_D00 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D01__LCD_D01 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D02__LCD_D02 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D03__LCD_D03 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D04__LCD_D04 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D05__LCD_D05 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D06__LCD_D06 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D07__LCD_D07 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D08__LCD_D08 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D09__LCD_D09 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D10__LCD_D10 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D11__LCD_D11 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D12__LCD_D12 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D13__LCD_D13 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D14__LCD_D14 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D15__LCD_D15 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D16__LCD_D16 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_D17__LCD_D17 | MUX_CONFIG_LCD,
	MX23_PAD_LCD_RESET__LCD_RESET | MUX_CONFIG_LCD,
	MX23_PAD_LCD_RS__LCD_RS | MUX_CONFIG_LCD,
	MX23_PAD_LCD_WR__LCD_WR | MUX_CONFIG_LCD,
	MX23_PAD_LCD_CS__LCD_CS | MUX_CONFIG_LCD,
	MX23_PAD_LCD_ENABLE__LCD_ENABLE | MUX_CONFIG_LCD,
	MX23_PAD_LCD_VSYNC__LCD_VSYNC | MUX_CONFIG_LCD,
};

static int mxsfb_read_register(uint32_t reg, uint32_t *value)
{
	iomux_cfg_t mux;
	uint32_t val = 0;
	int i;

	/* Mangle the register offset. */
	reg = ((reg & 0xff) << 1) | (((reg >> 8) & 0xff) << 10);

	/*
	 * The SmartLCD interface on MX233 can only do WRITE operation
	 * via the LCDIF controller. Implement the READ operation by
	 * fiddling with bits.
	 */
	mxs_iomux_setup_multiple_pads(iomux_lcd_gpio,
		ARRAY_SIZE(iomux_lcd_gpio));

	gpio_direction_output(MX23_PAD_LCD_RS__GPIO_1_19, 1);
	gpio_direction_output(MX23_PAD_LCD_CS__GPIO_1_21, 1);
	gpio_direction_output(MX23_PAD_LCD_WR__GPIO_1_20, 1);
	gpio_direction_output(MX23_PAD_LCD_ENABLE__GPIO_1_23, 1);

	for (i = 0; i < 18; i++) {
		mux = MXS_IOMUX_PAD_NAKED(1, i, PAD_MUXSEL_GPIO);
		gpio_direction_output(mux, 0);
	}

	udelay(2);
	gpio_direction_output(MX23_PAD_LCD_RS__GPIO_1_19, 0);
	udelay(1);
	gpio_direction_output(MX23_PAD_LCD_CS__GPIO_1_21, 0);
	udelay(1);
	gpio_direction_output(MX23_PAD_LCD_WR__GPIO_1_20, 0);
	udelay(1);

	for (i = 0; i < 18; i++) {
		mux = MXS_IOMUX_PAD_NAKED(1, i, PAD_MUXSEL_GPIO);
		gpio_direction_output(mux, (reg >> i) & 1);
	}
	udelay(1);

	gpio_direction_output(MX23_PAD_LCD_WR__GPIO_1_20, 1);
	udelay(3);

	for (i = 0; i < 18; i++) {
		mux = MXS_IOMUX_PAD_NAKED(1, i, PAD_MUXSEL_GPIO);
		gpio_direction_input(mux);
	}
	udelay(2);

	gpio_direction_output(MX23_PAD_LCD_ENABLE__GPIO_1_23, 0);
	udelay(1);
	gpio_direction_output(MX23_PAD_LCD_RS__GPIO_1_19, 1);
	udelay(1);
	gpio_direction_output(MX23_PAD_LCD_ENABLE__GPIO_1_23, 1);
	udelay(3);
	gpio_direction_output(MX23_PAD_LCD_ENABLE__GPIO_1_23, 0);
	udelay(2);

	for (i = 0; i < 18; i++) {
		mux = MXS_IOMUX_PAD_NAKED(1, i, PAD_MUXSEL_GPIO);
		val |= !!gpio_get_value(mux) << i;
	}
	udelay(1);

	gpio_direction_output(MX23_PAD_LCD_ENABLE__GPIO_1_23, 1);
	udelay(1);
	gpio_direction_output(MX23_PAD_LCD_CS__GPIO_1_21, 1);
	udelay(1);

	mxs_iomux_setup_multiple_pads(iomux_lcd_lcd,
		ARRAY_SIZE(iomux_lcd_lcd));

	/* Demangle the register value. */
	*value = ((val >> 1) & 0xff) | ((val >> 2) & 0xff00);

	writel(val, 0x2000);
	return 0;
}

static int mxsfb_write_byte(uint32_t payload, const unsigned int data)
{
	struct mxs_lcdif_regs *regs = (struct mxs_lcdif_regs *)MXS_LCDIF_BASE;
	const unsigned int timeout = 0x10000;

	/* What is going on here I do not know. FIXME */
	payload = ((payload & 0xff) << 1) | (((payload >> 8) & 0xff) << 10);

	if (mxs_wait_mask_clr(&regs->hw_lcdif_ctrl_reg, LCDIF_CTRL_RUN,
			      timeout))
		return -ETIMEDOUT;

	writel((1 << LCDIF_TRANSFER_COUNT_V_COUNT_OFFSET) |
		(1 << LCDIF_TRANSFER_COUNT_H_COUNT_OFFSET),
		&regs->hw_lcdif_transfer_count);

	writel(LCDIF_CTRL_DATA_SELECT | LCDIF_CTRL_RUN,
		&regs->hw_lcdif_ctrl_clr);

	if (data)
		writel(LCDIF_CTRL_DATA_SELECT, &regs->hw_lcdif_ctrl_set);

	writel(LCDIF_CTRL_RUN, &regs->hw_lcdif_ctrl_set);

	if (mxs_wait_mask_clr(&regs->hw_lcdif_lcdif_stat_reg, 1 << 29,
			      timeout))
		return -ETIMEDOUT;

	writel(payload, &regs->hw_lcdif_data);
	return mxs_wait_mask_clr(&regs->hw_lcdif_ctrl_reg, LCDIF_CTRL_RUN,
				 timeout);
}

static void mxsfb_write_register(uint32_t reg, uint32_t data)
{
	mxsfb_write_byte(reg, 0);
	mxsfb_write_byte(data, 1);
}

static const struct {
	uint8_t		reg;
	uint8_t		delay;
	uint16_t	val;
} lcd_regs[] = {
	{ 0xe5, 0  , 0x78f0 },
	{ 0xe3, 0  , 0x3008 },
	{ 0xe7, 0  , 0x0012 },
	{ 0xef, 0  , 0x1231 },
	{ 0x00, 0  , 0x0001 },
	{ 0x01, 0  , 0x0100 },
	{ 0x02, 0  , 0x0700 },
	{ 0x03, 0  , 0x1030 },
	{ 0x04, 0  , 0x0000 },
	{ 0x08, 0  , 0x0207 },
	{ 0x09, 0  , 0x0000 },
	{ 0x0a, 0  , 0x0000 },
	{ 0x0c, 0  , 0x0000 },
	{ 0x0d, 0  , 0x0000 },
	{ 0x0f, 0  , 0x0000 },
	{ 0x10, 0  , 0x0000 },
	{ 0x11, 0  , 0x0007 },
	{ 0x12, 0  , 0x0000 },
	{ 0x13, 20 , 0x0000 },
	/* Wait 20 mS here. */
	{ 0x10, 0  , 0x1290 },
	{ 0x11, 50 , 0x0007 },
	/* Wait 50 mS here. */
	{ 0x12, 50 , 0x0019 },
	/* Wait 50 mS here. */
	{ 0x13, 0  , 0x1700 },
	{ 0x29, 50 , 0x0014 },
	/* Wait 50 mS here. */
	{ 0x20, 0  , 0x0000 },
	{ 0x21, 0  , 0x0000 },
	{ 0x30, 0  , 0x0504 },
	{ 0x31, 0  , 0x0007 },
	{ 0x32, 0  , 0x0006 },
	{ 0x35, 0  , 0x0106 },
	{ 0x36, 0  , 0x0202 },
	{ 0x37, 0  , 0x0504 },
	{ 0x38, 0  , 0x0500 },
	{ 0x39, 0  , 0x0706 },
	{ 0x3c, 0  , 0x0204 },
	{ 0x3d, 0  , 0x0202 },
	{ 0x50, 0  , 0x0000 },
	{ 0x51, 0  , 0x00ef },
	{ 0x52, 0  , 0x0000 },
	{ 0x53, 0  , 0x013f },
	{ 0x60, 0  , 0xa700 },
	{ 0x61, 0  , 0x0001 },
	{ 0x6a, 0  , 0x0000 },
	{ 0x2b, 50 , 0x000d },
	/* Wait 50 mS here. */
	{ 0x90, 0  , 0x0011 },
	{ 0x92, 0  , 0x0600 },
	{ 0x93, 0  , 0x0003 },
	{ 0x95, 0  , 0x0110 },
	{ 0x97, 0  , 0x0000 },
	{ 0x98, 0  , 0x0000 },
	{ 0x07, 0  , 0x0173 },
};

void board_mxsfb_system_setup(void)
{
	struct mxs_lcdif_regs *regs = (struct mxs_lcdif_regs *)MXS_LCDIF_BASE;
	uint32_t id;
	int i;

	/* Switch the LCDIF into System-Mode */
	writel(LCDIF_CTRL_LCDIF_MASTER | LCDIF_CTRL_DOTCLK_MODE |
		LCDIF_CTRL_BYPASS_COUNT, &regs->hw_lcdif_ctrl_clr);

	/* To program the LCD, switch to 18bit bus + 18bit data. */
	clrsetbits_le32(&regs->hw_lcdif_ctrl,
		LCDIF_CTRL_WORD_LENGTH_MASK | LCDIF_CTRL_LCD_DATABUS_WIDTH_MASK,
		LCDIF_CTRL_WORD_LENGTH_18BIT |
		LCDIF_CTRL_LCD_DATABUS_WIDTH_18BIT);

	mxsfb_read_register(0, &id);
	writel(id, 0x2004);

	/* Restart the SmartLCD controller */
	mdelay(50);
	writel(1, &regs->hw_lcdif_ctrl1_set);
	mdelay(50);
	writel(1, &regs->hw_lcdif_ctrl1_clr);
	mdelay(50);
	writel(1, &regs->hw_lcdif_ctrl1_set);
	mdelay(50);

	/* Program the SmartLCD controller */
	writel(LCDIF_CTRL1_RECOVER_ON_UNDERFLOW, &regs->hw_lcdif_ctrl1_set);

	writel((0x02 << LCDIF_TIMING_CMD_HOLD_OFFSET) |
	       (0x02 << LCDIF_TIMING_CMD_SETUP_OFFSET) |
	       (0x02 << LCDIF_TIMING_DATA_HOLD_OFFSET) |
	       (0x01 << LCDIF_TIMING_DATA_SETUP_OFFSET),
	       &regs->hw_lcdif_timing);

	/*
	 * ILI9325 init and configuration sequence.
	 */
	for (i = 0; i < ARRAY_SIZE(lcd_regs); i++) {
		mxsfb_write_register(lcd_regs[i].reg, lcd_regs[i].val);
		if (lcd_regs[i].delay)
			mdelay(lcd_regs[i].delay);
	}
	/* Turn on Framebuffer Upload Mode */
	mxsfb_write_byte(0x22, 0);

	writel(LCDIF_CTRL_LCDIF_MASTER | LCDIF_CTRL_DATA_SELECT,
		&regs->hw_lcdif_ctrl_set);

	/* Operate the framebuffer in 16bit mode. */
	clrsetbits_le32(&regs->hw_lcdif_ctrl,
		LCDIF_CTRL_WORD_LENGTH_MASK | LCDIF_CTRL_LCD_DATABUS_WIDTH_MASK,
		LCDIF_CTRL_WORD_LENGTH_16BIT |
		LCDIF_CTRL_LCD_DATABUS_WIDTH_18BIT);
}
#endif

int board_init(void)
{
	/* Adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	/* Turn on PWM backlight */
	gpio_direction_output(MX23_PAD_PWM2__GPIO_1_28, 1);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	usb_eth_initialize(bis);
	return 0;
}
