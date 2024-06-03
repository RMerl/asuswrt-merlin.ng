// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Dirk Eibach, Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS

#include <common.h>
#include <i2c.h>

#define ADV7611_I2C_ADDR 0x4c
#define ADV7611_RDINFO 0x2051

/*
 * ADV7611 I2C Addresses in u-boot notation
 */
enum {
	CP_I2C_ADDR = 0x22,
	DPLL_I2C_ADDR = 0x26,
	KSV_I2C_ADDR = 0x32,
	HDMI_I2C_ADDR = 0x34,
	EDID_I2C_ADDR = 0x36,
	INFOFRAME_I2C_ADDR = 0x3e,
	CEC_I2C_ADDR = 0x40,
	IO_I2C_ADDR = ADV7611_I2C_ADDR,
};

/*
 * Global Control Registers
 */
enum {
	IO_RD_INFO_MSB = 0xea,
	IO_RD_INFO_LSB = 0xeb,
	IO_CEC_ADDR = 0xf4,
	IO_INFOFRAME_ADDR = 0xf5,
	IO_DPLL_ADDR = 0xf8,
	IO_KSV_ADDR = 0xf9,
	IO_EDID_ADDR = 0xfa,
	IO_HDMI_ADDR = 0xfb,
	IO_CP_ADDR = 0xfd,
};

int adv7611_i2c[] = CONFIG_SYS_ADV7611_I2C;

int adv7611_probe(unsigned int screen)
{
	int old_bus = i2c_get_bus_num();
	unsigned int rd_info;
	int res = 0;

	i2c_set_bus_num(adv7611_i2c[screen]);

	rd_info = (i2c_reg_read(IO_I2C_ADDR, IO_RD_INFO_MSB) << 8)
		  | i2c_reg_read(IO_I2C_ADDR, IO_RD_INFO_LSB);

	if (rd_info != ADV7611_RDINFO) {
		res = -1;
		goto out;
	}

	/*
	 * set I2C addresses to default values
	 */
	i2c_reg_write(IO_I2C_ADDR, IO_CEC_ADDR, CEC_I2C_ADDR << 1);
	i2c_reg_write(IO_I2C_ADDR, IO_INFOFRAME_ADDR, INFOFRAME_I2C_ADDR << 1);
	i2c_reg_write(IO_I2C_ADDR, IO_DPLL_ADDR, DPLL_I2C_ADDR << 1);
	i2c_reg_write(IO_I2C_ADDR, IO_KSV_ADDR, KSV_I2C_ADDR << 1);
	i2c_reg_write(IO_I2C_ADDR, IO_EDID_ADDR, EDID_I2C_ADDR << 1);
	i2c_reg_write(IO_I2C_ADDR, IO_HDMI_ADDR, HDMI_I2C_ADDR << 1);
	i2c_reg_write(IO_I2C_ADDR, IO_CP_ADDR, CP_I2C_ADDR << 1);

	/*
	 * do magic initialization sequence from
	 * "ADV7611 Register Settings Recommendations Revision 1.5"
	 * with most registers undocumented
	 */
	i2c_reg_write(CP_I2C_ADDR, 0x6c, 0x00);
	i2c_reg_write(HDMI_I2C_ADDR, 0x9b, 0x03);
	i2c_reg_write(HDMI_I2C_ADDR, 0x6f, 0x08);
	i2c_reg_write(HDMI_I2C_ADDR, 0x85, 0x1f);
	i2c_reg_write(HDMI_I2C_ADDR, 0x87, 0x70);
	i2c_reg_write(HDMI_I2C_ADDR, 0x57, 0xda);
	i2c_reg_write(HDMI_I2C_ADDR, 0x58, 0x01);
	i2c_reg_write(HDMI_I2C_ADDR, 0x03, 0x98);
	i2c_reg_write(HDMI_I2C_ADDR, 0x4c, 0x44);

	/*
	 * IO_REG_02, default 0xf0
	 *
	 * INP_COLOR_SPACE (IO, Address 0x02[7:4])
	 * default: 0b1111 auto
	 * set to : 0b0001 force RGB (range 0 to 255) input
	 *
	 * RGB_OUT (IO, Address 0x02[1])
	 * default: 0 YPbPr color space output
	 * set to : 1 RGB color space output
	 */
	i2c_reg_write(IO_I2C_ADDR, 0x02, 0x12);

	/*
	 * IO_REG_03, default 0x00
	 *
	 * OP_FORMAT_SEL (IO, Address 0x03[7:0])
	 * default: 0x00 8-bit SDR ITU-656 mode
	 * set to : 0x40 24-bit 4:4:4 SDR mode
	 */
	i2c_reg_write(IO_I2C_ADDR, 0x03, 0x40);

	/*
	 * IO_REG_05, default 0x2c
	 *
	 * AVCODE_INSERT_EN (IO, Address 0x05[2])
	 * default: 1 insert AV codes into data stream
	 * set to : 0 do not insert AV codes into data stream
	 */
	i2c_reg_write(IO_I2C_ADDR, 0x05, 0x28);

	/*
	 * IO_REG_0C, default 0x62
	 *
	 * POWER_DOWN (IO, Address 0x0C[5])
	 * default: 1 chip is powered down
	 * set to : 0 chip is operational
	 */
	i2c_reg_write(IO_I2C_ADDR, 0x0c, 0x42);

	/*
	 * IO_REG_15, default 0xbe
	 *
	 * TRI_SYNCS (IO, Address 0x15[3)
	 * TRI_LLC (IO, Address 0x15[2])
	 * TRI_PIX (IO, Address 0x15[1])
	 * default: 1 video output pins are tristate
	 * set to : 0 video output pins are active
	 */
	i2c_reg_write(IO_I2C_ADDR, 0x15, 0xb0);

	/*
	 * HDMI_REGISTER_02H, default 0xff
	 *
	 * CLOCK_TERMA_DISABLE (HDMI, Address 0x83[0])
	 * default: 1 disable termination
	 * set to : 0 enable termination
	 * Future options are:
	 * - use the chips automatic termination control
	 * - set this manually on cable detect
	 * but at the moment this seems a safe default.
	 */
	i2c_reg_write(HDMI_I2C_ADDR, 0x83, 0xfe);

	/*
	 * HDMI_CP_CNTRL_1, default 0x01
	 *
	 * HDMI_FRUN_EN (CP, Address 0xBA[0])
	 * default: 1 Enable the free run feature in HDMI mode
	 * set to : 0 Disable the free run feature in HDMI mode
	 */
	i2c_reg_write(CP_I2C_ADDR, 0xba, 0x00);

	/*
	 * INT1_CONFIGURATION, default 0x20
	 *
	 * INTRQ_DUR_SEL[1:0] (IO, Address 0x40[7:6])
	 * default: 00 Interrupt signal is active for 4 Xtal periods
	 * set to : 11 Active until cleared
	 *
	 * INTRQ_OP_SEL[1:0] (IO, Address 0x40[1:0])
	 * default: 00 Open drain
	 * set to : 10 Drives high when active
	 */
	i2c_reg_write(IO_I2C_ADDR, 0x40, 0xc2);

out:
	i2c_set_bus_num(old_bus);

	return res;
}

#endif /* CONFIG_GDSYS_LEGACY_DRIVERS */
