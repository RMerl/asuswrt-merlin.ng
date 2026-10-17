// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016-2018 Toradex AG
 */

#include <common.h>
#include <cpu_func.h>
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
#include <asm/io.h>
#include <dm.h>
#include <dm/platform_data/serial_mxc.h>
#include <fdt_support.h>
#include <fsl_esdhc_imx.h>
#include <jffs2/load_kernel.h>
#include <linux/delay.h>
#include <linux/sizes.h>
#include <mmc.h>
#include <miiphy.h>
#include <mtd_node.h>
#include <netdev.h>
#include <power/pmic.h>
#include <power/rn5t567_pmic.h>
#include <usb.h>
#include <usb/ehci-ci.h>
#include "../common/tdx-common.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_DSE_3P3V_49OHM | \
	PAD_CTL_PUS_PU100KOHM | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_PU100KOHM | PAD_CTL_DSE_3P3V_49OHM)
#define ENET_PAD_CTRL_MII  (PAD_CTL_DSE_3P3V_32OHM)

#define ENET_RX_PAD_CTRL  (PAD_CTL_PUS_PU100KOHM | PAD_CTL_DSE_3P3V_49OHM)

#define LCD_PAD_CTRL    (PAD_CTL_HYS | PAD_CTL_PUS_PU100KOHM | \
	PAD_CTL_DSE_3P3V_49OHM)

#define NAND_PAD_CTRL (PAD_CTL_DSE_3P3V_49OHM | PAD_CTL_SRE_SLOW | PAD_CTL_HYS)

#define NAND_PAD_READY0_CTRL (PAD_CTL_DSE_3P3V_49OHM | PAD_CTL_PUS_PU5KOHM)

#define USB_CDET_GPIO	IMX_GPIO_NR(7, 14)

#define FLASH_DETECTION_CTRL (PAD_CTL_HYS | PAD_CTL_PUE)
#define FLASH_DET_GPIO IMX_GPIO_NR(6, 11)

static bool is_emmc;

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, imx_ddr_size());

	return 0;
}

static iomux_v3_cfg_t const flash_detection_pads[] = {
	MX7D_PAD_SD3_RESET_B__GPIO6_IO11 | MUX_PAD_CTRL(FLASH_DETECTION_CTRL) | MUX_MODE_SION,
};

static iomux_v3_cfg_t const uart1_pads[] = {
	MX7D_PAD_UART1_RX_DATA__UART1_DTE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX7D_PAD_UART1_TX_DATA__UART1_DTE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX7D_PAD_SAI2_TX_BCLK__UART1_DTE_CTS | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX7D_PAD_SAI2_TX_SYNC__UART1_DTE_RTS | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#ifdef CONFIG_USB_EHCI_MX7
static iomux_v3_cfg_t const usb_cdet_pads[] = {
	MX7D_PAD_ENET1_CRS__GPIO7_IO14 | MUX_PAD_CTRL(NO_PAD_CTRL),
};
#endif

#ifdef CONFIG_TARGET_COLIBRI_IMX7_NAND
static iomux_v3_cfg_t const gpmi_pads[] = {
	MX7D_PAD_SD3_DATA0__NAND_DATA00 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX7D_PAD_SD3_DATA1__NAND_DATA01 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX7D_PAD_SD3_DATA2__NAND_DATA02 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX7D_PAD_SD3_DATA3__NAND_DATA03 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX7D_PAD_SD3_DATA4__NAND_DATA04 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX7D_PAD_SD3_DATA5__NAND_DATA05 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX7D_PAD_SD3_DATA6__NAND_DATA06 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX7D_PAD_SD3_DATA7__NAND_DATA07 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX7D_PAD_SD3_CLK__NAND_CLE	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX7D_PAD_SD3_CMD__NAND_ALE	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX7D_PAD_SD3_STROBE__NAND_RE_B	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX7D_PAD_SD3_RESET_B__NAND_WE_B	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX7D_PAD_SAI1_RX_DATA__NAND_CE1_B	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX7D_PAD_SAI1_TX_BCLK__NAND_CE0_B	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX7D_PAD_SAI1_TX_DATA__NAND_READY_B	| MUX_PAD_CTRL(NAND_PAD_READY0_CTRL),
};

static void setup_gpmi_nand(void)
{
	imx_iomux_v3_setup_multiple_pads(gpmi_pads, ARRAY_SIZE(gpmi_pads));

	/* NAND_USDHC_BUS_CLK is set in rom */
	set_clk_nand();
}
#endif

#ifdef CONFIG_VIDEO
static iomux_v3_cfg_t const backlight_pads[] = {
	/* Backlight On */
	MX7D_PAD_SD1_WP__GPIO5_IO1 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* Backlight PWM<A> (multiplexed pin) */
	MX7D_PAD_GPIO1_IO08__GPIO1_IO8   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX7D_PAD_ECSPI2_MOSI__GPIO4_IO21 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

#define GPIO_BL_ON IMX_GPIO_NR(5, 1)
#define GPIO_PWM_A IMX_GPIO_NR(1, 8)

static int setup_lcd(void)
{
	imx_iomux_v3_setup_multiple_pads(backlight_pads, ARRAY_SIZE(backlight_pads));

	/* Set BL_ON */
	gpio_request(GPIO_BL_ON, "BL_ON");
	gpio_direction_output(GPIO_BL_ON, 1);

	/* Set PWM<A> to full brightness (assuming inversed polarity) */
	gpio_request(GPIO_PWM_A, "PWM<A>");
	gpio_direction_output(GPIO_PWM_A, 0);

	return 0;
}
#endif

/*
 * Backlight off before OS handover
 */
void board_preboot_os(void)
{
#ifdef CONFIG_VIDEO
	gpio_direction_output(GPIO_PWM_A, 1);
	gpio_direction_output(GPIO_BL_ON, 0);
#endif
}

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

#ifdef CONFIG_FEC_MXC
static int setup_fec(void)
{
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs
		= (struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

#ifndef CONFIG_COLIBRI_IMX7_EXT_PHYCLK
	/*
	 * Use 50M anatop REF_CLK1 for ENET1, clear gpr1[13], set gpr1[17]
	 * and output it on the pin
	 */
	clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
			IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL_MASK,
			IOMUXC_GPR_GPR1_GPR_ENET1_CLK_DIR_MASK);
#else
	/* Use 50M external CLK for ENET1, set gpr1[13], clear gpr1[17] */
	clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
			IOMUXC_GPR_GPR1_GPR_ENET1_CLK_DIR_MASK,
			IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL_MASK);
#endif

	return set_clk_enet(ENET_50MHZ);
}

#endif

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	/*
	 * Enable GPIO SION on NAND_WE_B/eMMC_RST with 100k pull-down. eMMC_RST
	 * is pulled high with 4.7k for eMMC devices. This allows to reliably
	 * detect eMMC vs NAND flash.
	 */
	imx_iomux_v3_setup_multiple_pads(flash_detection_pads, ARRAY_SIZE(flash_detection_pads));
	gpio_request(FLASH_DET_GPIO, "flash-detection-gpio");
	is_emmc = gpio_get_value(FLASH_DET_GPIO);
	gpio_free(FLASH_DET_GPIO);

#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

#ifdef CONFIG_TARGET_COLIBRI_IMX7_NAND
	setup_gpmi_nand();
#endif

#ifdef CONFIG_USB_EHCI_MX7
	imx_iomux_v3_setup_multiple_pads(usb_cdet_pads, ARRAY_SIZE(usb_cdet_pads));
	gpio_request(USB_CDET_GPIO, "usb-cdet-gpio");
#endif

	return 0;
}

#ifdef CONFIG_DM_PMIC
int power_init_board(void)
{
	struct udevice *dev;
	int reg, ver;
	int ret;


	ret = pmic_get("pmic@33", &dev);
	if (ret)
		return ret;
	ver = pmic_reg_read(dev, RN5T567_LSIVER);
	reg = pmic_reg_read(dev, RN5T567_OTPVER);

	printf("PMIC:  RN5T567 LSIVER=0x%02x OTPVER=0x%02x\n", ver, reg);

	/* set judge and press timer of N_OE to minimal values */
	pmic_clrsetbits(dev, RN5T567_NOETIMSETCNT, 0x7, 0);

	/* configure sleep slot for 3.3V Ethernet */
	reg = pmic_reg_read(dev, RN5T567_LDO1_SLOT);
	reg = (reg & 0xf0) | reg >> 4;
	pmic_reg_write(dev, RN5T567_LDO1_SLOT, reg);

	/* disable DCDC2 discharge to avoid backfeeding through VFB2 */
	pmic_clrsetbits(dev, RN5T567_DC2CTL, 0x2, 0);

	/* configure sleep slot for ARM rail */
	reg = pmic_reg_read(dev, RN5T567_DC2_SLOT);
	reg = (reg & 0xf0) | reg >> 4;
	pmic_reg_write(dev, RN5T567_DC2_SLOT, reg);

	/* disable LDO2 discharge to avoid backfeeding from +V3.3_SD */
	pmic_clrsetbits(dev, RN5T567_LDODIS1, 0x2, 0);

	return 0;
}

void reset_cpu(void)
{
	struct udevice *dev;

	pmic_get("pmic@33", &dev);

	/* Use PMIC to reset, set REPWRTIM to 0 and REPWRON to 1 */
	pmic_reg_write(dev, RN5T567_REPCNT, 0x1);
	pmic_reg_write(dev, RN5T567_SLPCNT, 0x1);

	/*
	 * Re-power factor detection on PMIC side is not instant. 1ms
	 * proved to be enough time until reset takes effect.
	 */
	mdelay(1);
}
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
#if defined(CONFIG_IMX_BOOTAUX) && defined(CONFIG_ARCH_FIXUP_FDT_MEMORY)
	int up;

	up = arch_auxiliary_core_check_up(0);
	if (up) {
		int ret;
		int areas = 1;
		u64 start[2], size[2];

		/*
		 * Reserve 1MB of memory for M4 (1MiB is also the minimum
		 * alignment for Linux due to MMU section size restrictions).
		 */
		start[0] = gd->bd->bi_dram[0].start;
		size[0] = SZ_256M - SZ_1M;

		/* If needed, create a second entry for memory beyond 256M */
		if (gd->bd->bi_dram[0].size > SZ_256M) {
			start[1] = gd->bd->bi_dram[0].start + SZ_256M;
			size[1] = gd->bd->bi_dram[0].size - SZ_256M;
			areas = 2;
		}

		ret = fdt_set_usable_memory(blob, start, size, areas);
		if (ret) {
			eprintf("Cannot set usable memory\n");
			return ret;
		}
	} else {
		int off;

		off = fdt_node_offset_by_compatible(blob, -1,
						    "fsl,imx7d-rpmsg");
		if (off > 0)
			fdt_status_disabled(blob, off);
	}
#endif

	return ft_common_board_setup(blob, bd);
}
#endif

#ifdef CONFIG_USB_EHCI_MX7
int board_fix_fdt(void *rw_fdt_blob)
{
	int ret;

	/* i.MX 7Solo has only one single USB OTG1 but no USB host port */
	if (is_cpu_type(MXC_CPU_MX7S)) {
		int offset = fdt_path_offset(rw_fdt_blob, "/soc/bus@30800000/usb@30b20000");

		/*
		 * We're changing from status = "okay" to status = "disabled".
		 * In this case we'll need more space, so increase the size
		 * a little bit.
		 */
		ret = fdt_increase_size(rw_fdt_blob, 32);
		if (ret < 0) {
			printf("Cannot increase FDT size: %d\n", ret);
			return ret;
		}

		return fdt_status_disabled(rw_fdt_blob, offset);
	}

	return 0;
}

#if defined(CONFIG_BOARD_LATE_INIT)
int board_late_init(void)
{
#if defined(CONFIG_VIDEO)
	setup_lcd();
#endif

	if (IS_ENABLED(CONFIG_USB) && is_boot_from_usb()) {
		env_set("bootdelay", "0");
		if (IS_ENABLED(CONFIG_CMD_USB_SDP)) {
			printf("Serial Downloader recovery mode, using sdp command\n");
			env_set("bootcmd", "sdp 0");
		} else if (IS_ENABLED(CONFIG_CMD_FASTBOOT)) {
			printf("Fastboot recovery mode, using fastboot command\n");
			env_set("bootcmd", "fastboot usb 0");
		}
	}

	if (is_emmc)
		env_set("variant", "-emmc");
	else
		env_set("variant", "");

	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */

#endif /* CONFIG_USB_EHCI_MX7 */
