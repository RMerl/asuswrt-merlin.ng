// SPDX-License-Identifier: GPL-2.0+
/*
 * Creative ZEN X-Fi3 board
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
		/* Phison bridge always present */
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
	gpio_direction_output(MX23_PAD_GPMI_D07__GPIO_0_7, 0);
	ret = mxsmmc_initialize(bis, 0, NULL, xfi3_mmc_cd);
	if (ret)
		return ret;

	/* Phison SD-NAND bridge */
	ret = mxsmmc_initialize(bis, 1, NULL, xfi3_mmc_cd);

	return ret;
}
#endif

#ifdef CONFIG_VIDEO_MXS
static int mxsfb_write_byte(uint32_t payload, const unsigned int data)
{
	struct mxs_lcdif_regs *regs = (struct mxs_lcdif_regs *)MXS_LCDIF_BASE;
	const unsigned int timeout = 0x10000;

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
	{ 0x01, 0,  0x001c },
	{ 0x02, 0,  0x0100 },
	/* Writing 0x30 to reg. 0x03 flips the LCD */
	{ 0x03, 0,  0x1038 },
	{ 0x08, 0,  0x0808 },
	/* This can contain 0x111 to rotate the LCD. */
	{ 0x0c, 0,  0x0000 },
	{ 0x0f, 0,  0x0c01 },
	{ 0x20, 0,  0x0000 },
	{ 0x21, 30, 0x0000 },
	/* Wait 30 mS here */
	{ 0x10, 0,  0x0a00 },
	{ 0x11, 30, 0x1038 },
	/* Wait 30 mS here */
	{ 0x12, 0,  0x1010 },
	{ 0x13, 0,  0x0050 },
	{ 0x14, 0,  0x4f58 },
	{ 0x30, 0,  0x0000 },
	{ 0x31, 0,  0x00db },
	{ 0x32, 0,  0x0000 },
	{ 0x33, 0,  0x0000 },
	{ 0x34, 0,  0x00db },
	{ 0x35, 0,  0x0000 },
	{ 0x36, 0,  0x00af },
	{ 0x37, 0,  0x0000 },
	{ 0x38, 0,  0x00db },
	{ 0x39, 0,  0x0000 },
	{ 0x50, 0,  0x0000 },
	{ 0x51, 0,  0x0705 },
	{ 0x52, 0,  0x0e0a },
	{ 0x53, 0,  0x0300 },
	{ 0x54, 0,  0x0a0e },
	{ 0x55, 0,  0x0507 },
	{ 0x56, 0,  0x0000 },
	{ 0x57, 0,  0x0003 },
	{ 0x58, 0,  0x090a },
	{ 0x59, 30, 0x0a09 },
	/* Wait 30 mS here */
	{ 0x07, 30, 0x1017 },
	/* Wait 40 mS here */
	{ 0x36, 0,  0x00af },
	{ 0x37, 0,  0x0000 },
	{ 0x38, 0,  0x00db },
	{ 0x39, 0,  0x0000 },
	{ 0x20, 0,  0x0000 },
	{ 0x21, 0,  0x0000 },
};

void mxsfb_system_setup(void)
{
	struct mxs_lcdif_regs *regs = (struct mxs_lcdif_regs *)MXS_LCDIF_BASE;
	int i;

	/* Switch the LCDIF into System-Mode */
	writel(LCDIF_CTRL_LCDIF_MASTER | LCDIF_CTRL_DOTCLK_MODE |
		LCDIF_CTRL_BYPASS_COUNT, &regs->hw_lcdif_ctrl_clr);

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

	writel((0x03 << LCDIF_TIMING_CMD_HOLD_OFFSET) |
	       (0x03 << LCDIF_TIMING_CMD_SETUP_OFFSET) |
	       (0x03 << LCDIF_TIMING_DATA_HOLD_OFFSET) |
	       (0x02 << LCDIF_TIMING_DATA_SETUP_OFFSET),
	       &regs->hw_lcdif_timing);

	/*
	 * OTM2201A init and configuration sequence.
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
