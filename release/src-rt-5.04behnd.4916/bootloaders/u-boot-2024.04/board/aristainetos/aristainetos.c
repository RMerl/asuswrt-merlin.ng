// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 */

#include <common.h>
#include <bmp_layout.h>
#include <command.h>
#include <image.h>
#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/global_data.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/video.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/sections.h>
#include <bmp_logo.h>
#include <dm/root.h>
#include <env.h>
#include <i2c_eeprom.h>
#include <i2c.h>
#include <micrel.h>
#include <miiphy.h>
#include <led.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/da9063_pmic.h>
#include <splash.h>
#include <video.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	BOARD_TYPE_4 = 4,
	BOARD_TYPE_7 = 7,
};

#define ARI_BT_4 "aristainetos2_4@2"
#define ARI_BT_7 "aristainetos2_7@1"

int board_phy_config(struct phy_device *phydev)
{
	/* control data pad skew - devaddr = 0x02, register = 0x04 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* rx data pad skew - devaddr = 0x02, register = 0x05 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* tx data pad skew - devaddr = 0x02, register = 0x06 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* gtx and rx clock pad skew - devaddr = 0x02, register = 0x08 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_CLOCK_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x03FF);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

static int rotate_logo_one(unsigned char *out, unsigned char *in)
{
	int   i, j;

	for (i = 0; i < BMP_LOGO_WIDTH; i++)
		for (j = 0; j < BMP_LOGO_HEIGHT; j++)
			out[j * BMP_LOGO_WIDTH + BMP_LOGO_HEIGHT - 1 - i] =
			in[i * BMP_LOGO_WIDTH + j];
	return 0;
}

/*
 * Rotate the BMP_LOGO (only)
 * Will only work, if the logo is square, as
 * BMP_LOGO_HEIGHT and BMP_LOGO_WIDTH are defines, not variables
 */
void rotate_logo(int rotations)
{
	unsigned char out_logo[BMP_LOGO_WIDTH * BMP_LOGO_HEIGHT];
	struct bmp_header *header;
	unsigned char *in_logo;
	int   i, j;

	if (BMP_LOGO_WIDTH != BMP_LOGO_HEIGHT)
		return;

	header = (struct bmp_header *)bmp_logo_bitmap;
	in_logo = bmp_logo_bitmap + header->data_offset;

	/* one 90 degree rotation */
	if (rotations == 1  ||  rotations == 2  ||  rotations == 3)
		rotate_logo_one(out_logo, in_logo);

	/* second 90 degree rotation */
	if (rotations == 2  ||  rotations == 3)
		rotate_logo_one(in_logo, out_logo);

	/* third 90 degree rotation */
	if (rotations == 3)
		rotate_logo_one(out_logo, in_logo);

	/* copy result back to original array */
	if (rotations == 1  ||  rotations == 3)
		for (i = 0; i < BMP_LOGO_WIDTH; i++)
			for (j = 0; j < BMP_LOGO_HEIGHT; j++)
				in_logo[i * BMP_LOGO_WIDTH + j] =
				out_logo[i * BMP_LOGO_WIDTH + j];
}

static void enable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;
	s32 timeout = 100000;

	/* set PLL5 clock */
	reg = readl(&ccm->analog_pll_video);
	reg |= BM_ANADIG_PLL_VIDEO_POWERDOWN;
	writel(reg, &ccm->analog_pll_video);

	/* set PLL5 to 232720000Hz */
	reg &= ~BM_ANADIG_PLL_VIDEO_DIV_SELECT;
	reg |= BF_ANADIG_PLL_VIDEO_DIV_SELECT(0x26);
	reg &= ~BM_ANADIG_PLL_VIDEO_POST_DIV_SELECT;
	reg |= BF_ANADIG_PLL_VIDEO_POST_DIV_SELECT(0);
	writel(reg, &ccm->analog_pll_video);

	writel(BF_ANADIG_PLL_VIDEO_NUM_A(0xC0238),
	       &ccm->analog_pll_video_num);
	writel(BF_ANADIG_PLL_VIDEO_DENOM_B(0xF4240),
	       &ccm->analog_pll_video_denom);

	reg &= ~BM_ANADIG_PLL_VIDEO_POWERDOWN;
	writel(reg, &ccm->analog_pll_video);

	while (timeout--)
		if (readl(&ccm->analog_pll_video) & BM_ANADIG_PLL_VIDEO_LOCK)
			break;
	if (timeout < 0)
		printf("Warning: video pll lock timeout!\n");

	reg = readl(&ccm->analog_pll_video);
	reg |= BM_ANADIG_PLL_VIDEO_ENABLE;
	reg &= ~BM_ANADIG_PLL_VIDEO_BYPASS;
	writel(reg, &ccm->analog_pll_video);

	/* set LDB0, LDB1 clk select to 000/000 (PLL5 clock) */
	reg = readl(&ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		 | MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (0 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET)
		| (0 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &ccm->cs2cdr);

	reg = readl(&ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV;
	writel(reg, &ccm->cscmr2);

	reg = readl(&ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	writel(reg, &ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	      | IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH
	      | IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_HIGH
	      | IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	      | IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT
	      | IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED
	      | IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~IOMUXC_GPR3_LVDS0_MUX_CTL_MASK)
	       | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
		  << IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);
}

static void setup_display(void)
{
	enable_ipu_clock();
}

static void set_gpr_register(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	writel(IOMUXC_GPR1_APP_CLK_REQ_N | IOMUXC_GPR1_PCIE_RDY_L23 |
	       IOMUXC_GPR1_EXC_MON_SLVE |
	       (2 << IOMUXC_GPR1_ADDRS0_OFFSET) |
	       IOMUXC_GPR1_ACT_CS0,
	       &iomuxc_regs->gpr[1]);
	writel(0x0, &iomuxc_regs->gpr[8]);
	writel(IOMUXC_GPR12_ARMP_IPG_CLK_EN | IOMUXC_GPR12_ARMP_AHB_CLK_EN |
	       IOMUXC_GPR12_ARMP_ATB_CLK_EN | IOMUXC_GPR12_ARMP_APB_CLK_EN,
	       &iomuxc_regs->gpr[12]);
}

int board_early_init_f(void)
{
	select_ldb_di_clock_source(MXC_PLL5_CLK);
	set_gpr_register();

	/*
	 * clear bss here, so we can use spi driver
	 * before relocation and read Environment
	 * from spi flash.
	 */
	memset(__bss_start, 0x00, __bss_end - __bss_start);

	return 0;
}

static void setup_one_led(char *label, int state)
{
	struct udevice *dev;
	int ret;

	ret = led_get_by_label(label, &dev);
	if (ret == 0)
		led_set_state(dev, state);
}

static void setup_board_gpio(void)
{
	setup_one_led("led_ena", LEDST_ON);
	/* switch off Status LEDs */
	setup_one_led("led_yellow", LEDST_OFF);
	setup_one_led("led_red", LEDST_OFF);
	setup_one_led("led_green", LEDST_OFF);
	setup_one_led("led_blue", LEDST_OFF);
}

static void aristainetos_run_rescue_command(int reason)
{
	char rescue_reason_command[20];

	sprintf(rescue_reason_command, "setenv rreason %d", reason);
	run_command(rescue_reason_command, 0);
}

static int aristainetos_bootmode_settings(void)
{
	struct gpio_desc *desc;
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	unsigned int sbmr1 = readl(&psrc->sbmr1);
	char *my_bootdelay;
	char bootmode = 0;
	int ret;
	struct udevice *dev;
	int off;
	u8 data[0x10];
	u8 rescue_reason;

	/* jumper controlled reset of the environment */
	ret = gpio_hog_lookup_name("env_reset", &desc);
	if (!ret) {
		if (dm_gpio_get_value(desc)) {
			printf("\nReset u-boot environment (jumper)\n");
			run_command("run default_env; saveenv; saveenv", 0);
		}
	}

	off = fdt_path_offset(gd->fdt_blob, "eeprom0");
	if (off < 0) {
		printf("%s: No eeprom0 path offset\n", __func__);
		return off;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_I2C_EEPROM, off, &dev);
	if (ret) {
		printf("%s: Could not find EEPROM\n", __func__);
		return ret;
	}

	ret = i2c_set_chip_offset_len(dev, 2);
	if (ret)
		return ret;

	ret = i2c_eeprom_read(dev, 0x1ff0, (uint8_t *)data, sizeof(data));
	if (ret) {
		printf("%s: Could not read EEPROM\n", __func__);
		return ret;
	}

	/* software controlled reset of the environment (EEPROM magic) */
	if (strncmp((char *)data, "DeF", 3) == 0) {
		memset(data, 0xff, 3);
		i2c_eeprom_write(dev, 0x1ff0, (uint8_t *)data, 3);
		printf("\nReset u-boot environment (EEPROM)\n");
		run_command("run default_env; saveenv; saveenv", 0);
	}

	if (sbmr1 & 0x40) {
		env_set("bootmode", "1");
		printf("SD bootmode jumper set!\n");
	} else {
		env_set("bootmode", "0");
	}

	/*
	 * Check the boot-source. If booting from NOR Flash,
	 * disable bootdelay
	 */
	ret = gpio_hog_lookup_name("bootsel0", &desc);
	if (!ret)
		bootmode |= (dm_gpio_get_value(desc) ? 1 : 0) << 0;
	ret = gpio_hog_lookup_name("bootsel1", &desc);
	if (!ret)
		bootmode |= (dm_gpio_get_value(desc) ? 1 : 0) << 1;
	ret = gpio_hog_lookup_name("bootsel2", &desc);
	if (!ret)
		bootmode |= (dm_gpio_get_value(desc) ? 1 : 0) << 2;

	if (bootmode == 7) {
		my_bootdelay = env_get("nor_bootdelay");
		if (my_bootdelay)
			env_set("bootdelay", my_bootdelay);
		else
			env_set("bootdelay", "-2");
	}

	/* jumper controlled boot of the rescue system */
	ret = gpio_hog_lookup_name("boot_rescue", &desc);
	if (!ret) {
		if (dm_gpio_get_value(desc)) {
			printf("\nBooting into Rescue System (jumper)\n");
			aristainetos_run_rescue_command(16);
			run_command("run rescue_xload_boot", 0);
		}
	}

	/* software controlled boot of the rescue system (EEPROM magic) */
	if (strncmp((char *)&data[3], "ReScUe", 6) == 0) {
		rescue_reason = *(uint8_t *)&data[9];
		memset(&data[3], 0xff, 7);
		i2c_eeprom_write(dev, 0x1ff0, (uint8_t *)&data[3], 7);
		printf("\nBooting into Rescue System (EEPROM)\n");
		aristainetos_run_rescue_command(rescue_reason);
		run_command("run rescue_xload_boot", 0);
	}

	return 0;
}

#if defined(CONFIG_DM_PMIC_DA9063)
/*
 * On the aristainetos2c boards the PMIC needs to be initialized,
 * because the Ethernet PHY uses a different regulator that is not
 * setup per hardware default. This does not influence the other versions
 * as this regulator isn't used there at all.
 *
 * Unfortunately we have not yet a interface to setup all
 * values we need.
 */
static int setup_pmic_voltages(void)
{
	struct udevice *dev;
	int off;
	int ret;

	off = fdt_path_offset(gd->fdt_blob, "pmic0");
	if (off < 0) {
		printf("%s: No pmic path offset\n", __func__);
		return off;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_PMIC, off, &dev);
	if (ret) {
		printf("%s: Could not find PMIC\n", __func__);
		return ret;
	}

	pmic_reg_write(dev, DA9063_REG_PAGE_CON, 0x01);
	pmic_reg_write(dev, DA9063_REG_BPRO_CFG, 0xc1);
	ret = pmic_reg_read(dev, DA9063_REG_BUCK_ILIM_B);
	if (ret < 0) {
		printf("%s: error %d get register\n", __func__, ret);
		return ret;
	}
	ret &= 0xf0;
	ret |= 0x09;
	pmic_reg_write(dev, DA9063_REG_BUCK_ILIM_B, ret);
	pmic_reg_write(dev, DA9063_REG_VBPRO_A, 0x43);
	pmic_reg_write(dev, DA9063_REG_VBPRO_B, 0xc3);

	return 0;
}
#else
static int setup_pmic_voltages(void)
{
	return 0;
}
#endif

int board_late_init(void)
{
	int x, y;
	int ret;

	splash_get_pos(&x, &y);
	bmp_display((ulong)&bmp_logo_bitmap[0], x, y);

	ret = aristainetos_bootmode_settings();
	if (ret)
		return ret;

	/* set board_type */
	if (gd->board_type == BOARD_TYPE_4)
		env_set("board_type", ARI_BT_4);
	else
		env_set("board_type", ARI_BT_7);

	if (setup_pmic_voltages())
		printf("Error setup PMIC\n");

	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

struct display_info_t const displays[] = {
	{
		.bus	= -1,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= enable_lvds,
		.mode	= {
			.name           = "lb07wv8",
			.refresh        = 60,
			.xres           = 800,
			.yres           = 480,
			.pixclock       = 30066,
			.left_margin    = 88,
			.right_margin   = 88,
			.upper_margin   = 20,
			.lower_margin   = 20,
			.hsync_len      = 80,
			.vsync_len      = 5,
			.sync           = FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	}
};
size_t display_count = ARRAY_SIZE(displays);

int board_init(void)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	setup_board_gpio();
	setup_display();

	/* GPIO_1 for USB_OTG_ID */
	clrsetbits_le32(&iomux->gpr[1], IOMUXC_GPR1_USB_OTG_ID_SEL_MASK, 0);
	return 0;
}

int board_fit_config_name_match(const char *name)
{
	if (gd->board_type == BOARD_TYPE_4 &&
	    strchr(name, 0x34))
		return 0;

	if (gd->board_type == BOARD_TYPE_7 &&
	    strchr(name, 0x37))
		return 0;

	return -1;
}

static void do_board_detect(void)
{
	int ret;
	char s[30];

	/* default use board type 7 */
	gd->board_type = BOARD_TYPE_7;
	if (env_init())
		return;

	ret = env_get_f("panel", s, sizeof(s));
	if (ret < 0)
		return;

	if (!strncmp("lg4573", s, 6))
		gd->board_type = BOARD_TYPE_4;
}

#ifdef CONFIG_DTB_RESELECT
int embedded_dtb_select(void)
{
	int rescan;

	do_board_detect();
	fdtdec_resetup(&rescan);

	return 0;
}
#endif
