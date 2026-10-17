// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 NXP Semiconductors
 */

#include <init.h>
#include <net.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx7-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/io.h>
#include <common.h>
#include <miiphy.h>
#include <power/pmic.h>
#include <power/pfuze3000_pmic.h>
#include "../../freescale/common/pfuze.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_DSE_3P3V_49OHM | \
	PAD_CTL_PUS_PU100KOHM | PAD_CTL_HYS)

#define PICO_MMC0 0
#define PICO_MMC0_BLK 2
#define PICO_MMC1 1
#define PICO_MMC1_BLK 0

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	/* Subtract the defined OPTEE runtime firmware length */
#ifdef CONFIG_OPTEE_TZDRAM_SIZE
		gd->ram_size -= CONFIG_OPTEE_TZDRAM_SIZE;
#endif

	return 0;
}

#if CONFIG_IS_ENABLED(DM_PMIC)
int power_init_board(void)
{
	struct udevice *dev;
	int reg, rev_id;
	int ret;

	ret = pmic_get("pfuze3000@8", &dev);
	if (ret == -ENODEV)
		return 0;
	if (ret != 0)
		return ret;

	reg = pmic_reg_read(dev, PFUZE3000_DEVICEID);
	rev_id = pmic_reg_read(dev, PFUZE3000_REVID);
	printf("PMIC: PFUZE3000 DEV_ID=0x%x REV_ID=0x%x\n", reg, rev_id);

	/* disable Low Power Mode during standby mode */
	reg = pmic_reg_read(dev, PFUZE3000_LDOGCTL);
	reg |= 0x1;
	pmic_reg_write(dev, PFUZE3000_LDOGCTL, reg);

	/* SW1A/1B mode set to APS/APS */
	reg = 0x8;
	pmic_reg_write(dev, PFUZE3000_SW1AMODE, reg);
	pmic_reg_write(dev, PFUZE3000_SW1BMODE, reg);

	/* SW1A/1B standby voltage set to 1.025V */
	reg = 0xd;
	pmic_reg_write(dev, PFUZE3000_SW1ASTBY, reg);
	pmic_reg_write(dev, PFUZE3000_SW1BSTBY, reg);

	/* decrease SW1B normal voltage to 0.975V */
	reg = pmic_reg_read(dev, PFUZE3000_SW1BVOLT);
	reg &= ~0x1f;
	reg |= PFUZE3000_SW1AB_SETP(975);
	pmic_reg_write(dev, PFUZE3000_SW1BVOLT, reg);

	return 0;
}
#endif

static iomux_v3_cfg_t const wdog_pads[] = {
	MX7D_PAD_GPIO1_IO00__WDOG1_WDOG_B | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const uart5_pads[] = {
	MX7D_PAD_I2C4_SCL__UART5_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX7D_PAD_I2C4_SDA__UART5_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#ifdef CONFIG_FEC_MXC
static int setup_fec(void)
{
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs
		= (struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Use 125M anatop REF_CLK1 for ENET1, clear gpr1[13], gpr1[17] */
	clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
			(IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL_MASK |
			IOMUXC_GPR_GPR1_GPR_ENET1_CLK_DIR_MASK), 0);

	return set_clk_enet(ENET_125MHZ);
}
#endif

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart5_pads, ARRAY_SIZE(uart5_pads));
}

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

#ifdef CONFIG_VIDEO
void setup_lcd(void)
{
	gpio_request(IMX_GPIO_NR(1, 11), "lcd_brightness");
	gpio_request(IMX_GPIO_NR(1, 6), "lcd_enable");
	/* Set Brightness to high */
	gpio_direction_output(IMX_GPIO_NR(1, 11) , 1);
	/* Set LCD enable to high */
	gpio_direction_output(IMX_GPIO_NR(1, 6) , 1);
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_VIDEO
	setup_lcd();
#endif
#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

	return 0;
}

int board_late_init(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

#if CONFIG_IS_ENABLED(FSL_ESDHC_IMX)
#if CONFIG_IS_ENABLED(ENV_IS_IN_MMC) || CONFIG_IS_ENABLED(ENV_IS_NOWHERE)
	board_late_mmc_env_init();
#endif /* CONFIG_ENV_IS_IN_MMC or CONFIG_ENV_IS_NOWHERE */
#endif

	/*
	 * Do not assert internal WDOG_RESET_B_DEB(controlled by bit 4),
	 * since we use PMIC_PWRON to reset the board.
	 */
	clrsetbits_le16(&wdog->wcr, 0, 0x10);

	return 0;
}

int checkboard(void)
{
	puts("Board: i.MX7D PICOSOM\n");

	return 0;
}

static iomux_v3_cfg_t const usb_otg2_pads[] = {
	MX7D_PAD_UART3_CTS_B__USB_OTG2_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
};

int board_ehci_hcd_init(int port)
{
	switch (port) {
	case 0:
		break;
	case 1:
		imx_iomux_v3_setup_multiple_pads(usb_otg2_pads,
						 ARRAY_SIZE(usb_otg2_pads));
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

#if CONFIG_IS_ENABLED(FSL_ESDHC_IMX)
#if CONFIG_IS_ENABLED(ENV_IS_IN_MMC) || CONFIG_IS_ENABLED(ENV_IS_NOWHERE)
int board_mmc_get_env_dev(int devno)
{
	int dev_env = 0;

	switch (get_boot_device()) {
	case SD3_BOOT:
	case MMC3_BOOT:
		env_set("bootdev", "MMC3");
		dev_env = PICO_MMC0;
		break;
	case SD1_BOOT:
		env_set("bootdev", "SD1");
		dev_env = PICO_MMC1;
		break;
	default:
		printf("Wrong boot device!");
	}

	return dev_env;
}

int mmc_map_to_kernel_blk(int dev_no)
{
	int blk_no = 0;

	switch (dev_no) {
	case PICO_MMC0:
		blk_no = PICO_MMC0_BLK;
		break;
	case PICO_MMC1:
		blk_no = PICO_MMC1_BLK;
		break;
	default:
		printf("Invalid MMC device!");
	}

	return blk_no;
}
#endif

#if CONFIG_IS_ENABLED(ENV_IS_NOWHERE)
int mmc_get_env_dev(void)
{
	return board_mmc_get_env_dev(0);
}
#endif
#endif /* CONFIG_FSL_ESDHC_IMX */
