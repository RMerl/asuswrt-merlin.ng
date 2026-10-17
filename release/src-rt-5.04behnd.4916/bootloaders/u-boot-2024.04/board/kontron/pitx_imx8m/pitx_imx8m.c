// SPDX-License-Identifier: GPL-2.0+

#include "pitx_misc.h"
#include <common.h>
#include <efi.h>
#include <efi_loader.h>
#include <init.h>
#include <mmc.h>
#include <miiphy.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm-generic/gpio.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <linux/delay.h>
#include <linux/kernel.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)
#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE)

static iomux_v3_cfg_t const wdog_pads[] = {
	IMX8MQ_PAD_GPIO1_IO02__WDOG1_WDOG_B | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

static iomux_v3_cfg_t const uart_pads[] = {
	IMX8MQ_PAD_UART3_RXD__UART3_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART3_TXD__UART3_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_ECSPI1_SS0__UART3_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_ECSPI1_MISO__UART3_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_image fw_images[] = {
	{
		.image_type_id = KONTRON_PITX_IMX8M_FIT_IMAGE_GUID,
		.fw_name = u"KONTRON-PITX-IMX8M-UBOOT",
		.image_index = 1,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 0=flash-bin raw 0x42 0x1000 mmcpart 1",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));
	set_wdog_reset(wdog);

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	return 0;
}

int board_phys_sdram_size(phys_size_t *memsize)
{
	int variant = 0;

	variant = get_pitx_board_variant();

	switch(variant) {
	case 2:
		*memsize = 0x80000000;
		break;
	case 3:
		*memsize = 0x100000000;
		break;
	default:
		printf("Unknown DDR type!!!\n");
		*memsize = 0x40000000;
		break;
	}

	debug("Memsize: %d MiB\n", (int)(*memsize >> 20));

	return 0;
}


#ifdef CONFIG_FEC_MXC
#define FEC_RST_PAD IMX_GPIO_NR(1, 11)
static iomux_v3_cfg_t const fec1_rst_pads[] = {
	IMX8MQ_PAD_GPIO1_IO11__GPIO1_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_fec(void)
{
	imx_iomux_v3_setup_multiple_pads(fec1_rst_pads,
					 ARRAY_SIZE(fec1_rst_pads));
}

int board_phy_config(struct phy_device *phydev)
{
	unsigned int val;

	/*
	 * Set LED configuration register 1:
	 * LED2_SEL: 0b1011 (link established, blink on activity)
	 */
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x18);
	val &= 0xf0ff;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x18, val | (0xb << 8));

	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
}
#endif

int board_init(void)
{
#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_DWC3)
	init_usb_clk();
#endif

	return 0;
}

#ifdef CONFIG_MISC_INIT_R
#define TPM_RESET    IMX_GPIO_NR(3, 2)
#define USBHUB_RESET IMX_GPIO_NR(3, 4)

static void reset_device_by_gpio(const char *label, int pin, int delay_ms)
{
	gpio_request(pin, label);
	gpio_direction_output(pin, 0);
	mdelay(delay_ms);
	gpio_direction_output(pin, 1);
}

int misc_init_r(void)
{
	/*
	 * reset TPM chip (Infineon SLB9670) as required by datasheet
	 * (60ms minimum Reset Inactive Time, 70ms implemented)
	 */
	reset_device_by_gpio("tpm_reset", TPM_RESET, 70);

	/*
	 * reset USB hub as required by datasheet
	 * (3ms minimum reset duration, 10ms implemented)
	 */
	reset_device_by_gpio("usbhub_reset", USBHUB_RESET, 10);

	return 0;
}
#endif

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

uint mmc_get_env_part(struct mmc *mmc)
{
	/* part 1 for eMMC, part 1 for SD card */
	return (mmc_get_env_dev() == 0) ? 1 : 0;
}

int board_late_init(void)
{
	return 0;
}
