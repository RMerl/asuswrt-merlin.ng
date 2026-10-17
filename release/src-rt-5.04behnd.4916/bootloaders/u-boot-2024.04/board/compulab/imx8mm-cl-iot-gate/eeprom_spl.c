// SPDX-License-Identifier: GPL-2.0+
/* (C) Copyright 2019 CompuLab, Ltd. <www.compulab.co.il> */

#include <common.h>
#include <i2c.h>
#include <linux/kernel.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/mach-imx/gpio.h>
#include <asm-generic/gpio.h>
#include <asm/setup.h>
#include <linux/delay.h>

#ifdef CONFIG_SPL_BUILD

#define CFG_SYS_I2C_EEPROM_ADDR_P1	0x51

static iomux_v3_cfg_t const eeprom_pads[] = {
	IMX8MQ_PAD_GPIO1_IO13__GPIO1_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

#define EEPROM_WP_GPIO IMX_GPIO_NR(1, 13)

static void cl_eeprom_we(int enable)
{
	static int done;

	if (done) {
		gpio_direction_output(EEPROM_WP_GPIO, enable);
		return;
	}

	imx_iomux_v3_setup_multiple_pads(eeprom_pads, ARRAY_SIZE(eeprom_pads));
	gpio_request(EEPROM_WP_GPIO, "eeprom_wp");
	gpio_direction_output(EEPROM_WP_GPIO, enable);
	done = 1;
}

static int cl_eeprom_read(uint offset, uchar *buf, int len)
{
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(1, CFG_SYS_I2C_EEPROM_ADDR_P1,
				      CONFIG_SYS_I2C_EEPROM_ADDR_LEN, &dev);
	if (ret) {
		printf("%s: Cannot find EEPROM: %d\n", __func__, ret);
		return ret;
	}

	return dm_i2c_read(dev, offset, buf, len);
}

static int cl_eeprom_write(uint offset, uchar *buf, int len)
{
	struct udevice *dev;
	int ret;

	cl_eeprom_we(1);

	ret = i2c_get_chip_for_busnum(1, CFG_SYS_I2C_EEPROM_ADDR_P1,
				      CONFIG_SYS_I2C_EEPROM_ADDR_LEN, &dev);
	if (ret) {
		printf("%s: Cannot find EEPROM: %d\n", __func__, ret);
		return ret;
	}

	return dm_i2c_write(dev, offset, buf, len);
}

/* Reserved for fututre use area */
#define BOARD_DDRINFO_OFFSET 0x40
#define BOARD_DDR_SIZE 4
static u32 board_ddrinfo = 0xdeadbeef;

#define BOARD_DDRSUBIND_OFFSET 0x44
#define BOARD_DDRSUBIND_SIZE 1
static u8 board_ddrsubind = 0xff;

#define BOARD_OSIZE_OFFSET 0x80
#define BOARD_OSIZE_SIZE 4
static u32 board_osize = 0xdeadbeef;

#define BOARD_DDRINFO_VALID(A) ((A) != 0xdeadbeef)

u32 cl_eeprom_get_ddrinfo(void)
{
	if (!BOARD_DDRINFO_VALID(board_ddrinfo)) {
		if (cl_eeprom_read(BOARD_DDRINFO_OFFSET, (uchar *)&board_ddrinfo, BOARD_DDR_SIZE))
			return 0;
	}
	return board_ddrinfo;
};

u32 cl_eeprom_set_ddrinfo(u32 ddrinfo)
{
	if (cl_eeprom_write(BOARD_DDRINFO_OFFSET, (uchar *)&ddrinfo, BOARD_DDR_SIZE))
		return 0;

	board_ddrinfo = ddrinfo;

	return board_ddrinfo;
};

u8 cl_eeprom_get_subind(void)
{
	if (cl_eeprom_read(BOARD_DDRSUBIND_OFFSET, (uchar *)&board_ddrsubind, BOARD_DDRSUBIND_SIZE))
		return 0xff;

	return board_ddrsubind;
};

u8 cl_eeprom_set_subind(u8 ddrsubind)
{
	if (cl_eeprom_write(BOARD_DDRSUBIND_OFFSET, (uchar *)&ddrsubind, BOARD_DDRSUBIND_SIZE))
		return 0xff;
	board_ddrsubind = ddrsubind;

	return board_ddrsubind;
};

/* override-size ifaces */
u32 cl_eeprom_get_osize(void)
{
	if (cl_eeprom_read(BOARD_OSIZE_OFFSET, (uchar *)&board_osize, BOARD_OSIZE_SIZE))
		return 0;

	return board_osize;
};
#endif
