// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * Valentin Lontgchamp, Keymile AG, valentin.longchamp@keymile.com
 */

#include <common.h>
#include <i2c.h>
#include <linux/errno.h>

/* GPIO Pin from kirkwood connected to PROGRAM_B pin of the xilinx FPGA */
#define KM_XLX_PROGRAM_B_PIN    39

#define BOCO_ADDR	0x10

#define ID_REG		0x00
#define BOCO2_ID	0x5b

static int check_boco2(void)
{
	int ret;
	u8 id;

	ret = i2c_read(BOCO_ADDR, ID_REG, 1, &id, 1);
	if (ret) {
		printf("%s: error reading the BOCO id !!\n", __func__);
		return ret;
	}

	return (id == BOCO2_ID);
}

static int boco_clear_bits(u8 reg, u8 flags)
{
	int ret;
	u8 regval;

	/* give access to the EEPROM from FPGA */
	ret = i2c_read(BOCO_ADDR, reg, 1, &regval, 1);
	if (ret) {
		printf("%s: error reading the BOCO @%#x !!\n",
			__func__, reg);
		return ret;
	}
	regval &= ~flags;
	ret = i2c_write(BOCO_ADDR, reg, 1, &regval, 1);
	if (ret) {
		printf("%s: error writing the BOCO @%#x !!\n",
			__func__, reg);
		return ret;
	}

	return 0;
}

static int boco_set_bits(u8 reg, u8 flags)
{
	int ret;
	u8 regval;

	/* give access to the EEPROM from FPGA */
	ret = i2c_read(BOCO_ADDR, reg, 1, &regval, 1);
	if (ret) {
		printf("%s: error reading the BOCO @%#x !!\n",
			__func__, reg);
		return ret;
	}
	regval |= flags;
	ret = i2c_write(BOCO_ADDR, reg, 1, &regval, 1);
	if (ret) {
		printf("%s: error writing the BOCO @%#x !!\n",
			__func__, reg);
		return ret;
	}

	return 0;
}

#define SPI_REG		0x06
#define CFG_EEPROM	0x02
#define FPGA_PROG	0x04
#define FPGA_INIT_B	0x10
#define FPGA_DONE	0x20

static int fpga_done(void)
{
	int ret = 0;
	u8 regval;

	/* this is only supported with the boco2 design */
	if (!check_boco2())
		return 0;

	ret = i2c_read(BOCO_ADDR, SPI_REG, 1, &regval, 1);
	if (ret) {
		printf("%s: error reading the BOCO @%#x !!\n",
			__func__, SPI_REG);
		return 0;
	}

	return regval & FPGA_DONE ? 1 : 0;
}

int skip;

int trigger_fpga_config(void)
{
	int ret = 0;

	/* if the FPGA is already configured, we do not want to
	 * reconfigure it */
	skip = 0;
	if (fpga_done()) {
		printf("PCIe FPGA config: skipped\n");
		skip = 1;
		return 0;
	}

	if (check_boco2()) {
		/* we have a BOCO2, this has to be triggered here */

		/* make sure the FPGA_can access the EEPROM */
		ret = boco_clear_bits(SPI_REG, CFG_EEPROM);
		if (ret)
			return ret;

		/* trigger the config start */
		ret = boco_clear_bits(SPI_REG, FPGA_PROG | FPGA_INIT_B);
		if (ret)
			return ret;

		/* small delay for the pulse */
		udelay(10);

		/* up signal for pulse end */
		ret = boco_set_bits(SPI_REG, FPGA_PROG);
		if (ret)
			return ret;

		/* finally, raise INIT_B to remove the config delay */
		ret = boco_set_bits(SPI_REG, FPGA_INIT_B);
		if (ret)
			return ret;

	} else {
		/* we do it the old way, with the gpio pin */
		kw_gpio_set_valid(KM_XLX_PROGRAM_B_PIN, 1);
		kw_gpio_direction_output(KM_XLX_PROGRAM_B_PIN, 0);
		/* small delay for the pulse */
		udelay(10);
		kw_gpio_direction_input(KM_XLX_PROGRAM_B_PIN);
	}

	return 0;
}

int wait_for_fpga_config(void)
{
	int ret = 0;
	u8 spictrl;
	u32 timeout = 20000;

	if (skip)
		return 0;

	if (!check_boco2()) {
		/* we do not have BOCO2, this is not really used */
		return 0;
	}

	printf("PCIe FPGA config:");
	do {
		ret = i2c_read(BOCO_ADDR, SPI_REG, 1, &spictrl, 1);
		if (ret) {
			printf("%s: error reading the BOCO spictrl !!\n",
				__func__);
			return ret;
		}
		if (timeout-- == 0) {
			printf(" FPGA_DONE timeout\n");
			return -EFAULT;
		}
		udelay(10);
	} while (!(spictrl & FPGA_DONE));

	printf(" done\n");

	return 0;
}

#if defined(KM_PCIE_RESET_MPP7)

#define KM_PEX_RST_GPIO_PIN	7
int fpga_reset(void)
{
	if (!check_boco2()) {
		/* we do not have BOCO2, this is not really used */
		return 0;
	}

	printf("PCIe reset through GPIO7: ");
	/* apply PCIe reset via GPIO */
	kw_gpio_set_valid(KM_PEX_RST_GPIO_PIN, 1);
	kw_gpio_direction_output(KM_PEX_RST_GPIO_PIN, 1);
	kw_gpio_set_value(KM_PEX_RST_GPIO_PIN, 0);
	udelay(1000*10);
	kw_gpio_set_value(KM_PEX_RST_GPIO_PIN, 1);

	printf(" done\n");

	return 0;
}

#else

#define PRST1		0x4
#define PCIE_RST	0x10
#define TRAFFIC_RST	0x04

int fpga_reset(void)
{
	int ret = 0;
	u8 resets;

	if (!check_boco2()) {
		/* we do not have BOCO2, this is not really used */
		return 0;
	}

	/* if we have skipped, we only want to reset the PCIe part */
	resets = skip ? PCIE_RST : PCIE_RST | TRAFFIC_RST;

	ret = boco_clear_bits(PRST1, resets);
	if (ret)
		return ret;

	/* small delay for the pulse */
	udelay(10);

	ret = boco_set_bits(PRST1, resets);
	if (ret)
		return ret;

	return 0;
}
#endif

/* the FPGA was configured, we configure the BOCO2 so that the EEPROM
 * is available from the Bobcat SPI bus */
int toggle_eeprom_spi_bus(void)
{
	int ret = 0;

	if (!check_boco2()) {
		/* we do not have BOCO2, this is not really used */
		return 0;
	}

	ret = boco_set_bits(SPI_REG, CFG_EEPROM);
	if (ret)
		return ret;

	return 0;
}
