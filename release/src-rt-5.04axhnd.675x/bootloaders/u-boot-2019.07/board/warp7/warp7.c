// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 NXP Semiconductors
 * Author: Fabio Estevam <fabio.estevam@nxp.com>
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx7-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/hab.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/io.h>
#include <common.h>
#include <asm/arch/crm_regs.h>
#include <netdev.h>
#include <power/pmic.h>
#include <power/pfuze3000_pmic.h>
#include "../freescale/common/pfuze.h"
#include <asm/setup.h>
#include <asm/bootm.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_DSE_3P3V_49OHM | PAD_CTL_PUS_PU100KOHM | \
			PAD_CTL_HYS)

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_SIZE;

	/* Subtract the defined OPTEE runtime firmware length */
#ifdef CONFIG_OPTEE_TZDRAM_SIZE
		gd->ram_size -= CONFIG_OPTEE_TZDRAM_SIZE;
#endif

	return 0;
}

static iomux_v3_cfg_t const wdog_pads[] = {
	MX7D_PAD_GPIO1_IO00__WDOG1_WDOG_B | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const uart1_pads[] = {
	MX7D_PAD_UART1_TX_DATA__UART1_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX7D_PAD_UART1_RX_DATA__UART1_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
};

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

#ifdef CONFIG_DM_PMIC
int power_init_board(void)
{
	struct udevice *dev;
	int ret, dev_id, rev_id;

	ret = pmic_get("pfuze3000", &dev);
	if (ret == -ENODEV)
		return 0;
	if (ret != 0)
		return ret;

	dev_id = pmic_reg_read(dev, PFUZE3000_DEVICEID);
	rev_id = pmic_reg_read(dev, PFUZE3000_REVID);
	printf("PMIC: PFUZE3000 DEV_ID=0x%x REV_ID=0x%x\n", dev_id, rev_id);

	/* disable Low Power Mode during standby mode */
	pmic_reg_write(dev, PFUZE3000_LDOGCTL, 1);

	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	int ret = 0;

#ifdef CONFIG_USB_ETHER
	ret = usb_eth_initialize(bis);
	if (ret < 0)
		printf("Error %d registering USB ether.\n", ret);
#endif

	return ret;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int checkboard(void)
{
	char *mode;

	if (IS_ENABLED(CONFIG_ARMV7_BOOT_SEC_DEFAULT))
		mode = "secure";
	else
		mode = "non-secure";

#ifdef CONFIG_OPTEE_TZDRAM_SIZE
	unsigned long optee_start, optee_end;

	optee_end = PHYS_SDRAM + PHYS_SDRAM_SIZE;
	optee_start = optee_end - CONFIG_OPTEE_TZDRAM_SIZE;

	printf("Board: WARP7 in %s mode OPTEE DRAM 0x%08lx-0x%08lx\n",
	       mode, optee_start, optee_end);
#else
	printf("Board: WARP7 in %s mode\n", mode);
#endif

	return 0;
}

int board_late_init(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;
#ifdef CONFIG_SERIAL_TAG
	struct tag_serialnr serialnr;
	char serial_string[0x20];
#endif

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	/*
	 * Do not assert internal WDOG_RESET_B_DEB(controlled by bit 4),
	 * since we use PMIC_PWRON to reset the board.
	 */
	clrsetbits_le16(&wdog->wcr, 0, 0x10);

#ifdef CONFIG_SECURE_BOOT
	/* Determine HAB state */
	env_set_ulong(HAB_ENABLED_ENVNAME, imx_hab_is_enabled());
#else
	env_set_ulong(HAB_ENABLED_ENVNAME, 0);
#endif

#ifdef CONFIG_SERIAL_TAG
	/* Set serial# standard environment variable based on OTP settings */
	get_board_serial(&serialnr);
	snprintf(serial_string, sizeof(serial_string), "WaRP7-0x%08x%08x",
		 serialnr.low, serialnr.high);
	env_set("serial#", serial_string);
#endif

	return 0;
}
