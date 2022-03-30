// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

/* Power-One ZM7300 DPM */
#include "zm7300.h"

#define DPM_WP 0x96
#define WRP_OPCODE 0x01
#define WRM_OPCODE 0x02
#define RRP_OPCODE 0x11

#define DPM_SUCCESS 0x01
#define DPM_EXEC_FAIL 0x00

static const uint16_t hex_to_1_10mv[] = {
	5000,
	5125,
	5250,
	5375,
	5500,
	5625,
	5750,
	5875,
	6000,
	6125,
	6250,
	6375,
	6500,
	6625,
	6750,
	6875,
	7000,
	7125,
	7250,
	7375,
	7500,
	7625,
	7750,
	7875,
	8000,
	8125,
	8250,
	8375,
	8500,
	8625,
	8750,
	8875,
	9000,
	9125,
	9250,
	9375,
	9500,  /* 0.95mV */
	9625,
	9750,
	9875,
	10000,  /* 1.0V */
	10125,
	10250,
	10375,
	10500,
	10625,
	10750,
	10875,
	11000,
	11125,
	11250,
	11375,
	11500,
	11625,
	11750,
	11875,
	12000,
	12125,
	12250,
	12375,
	0,	/* reserved */
};


/* Read Data d from Register r of POL p */
u8 dpm_rrp(uchar r)
{
	u8 ret[5];

	ret[0] = RRP_OPCODE;
	/* POL is 0 */
	ret[1] = 0;
	ret[2] = r;
	i2c_read(I2C_DPM_ADDR, 0, -3, ret, 2);
	if (ret[1] == DPM_SUCCESS) { /* the DPM returned success as status */
		debug("RRP_OPCODE returned success data is %x\n", ret[0]);
		return ret[0];
	} else {
		return -1;
	}
}

/* Write Data d into DPM register r (RAM) */
int dpm_wrm(u8 r, u8 d)
{
	u8 ret[5];

	ret[0] = WRM_OPCODE;
	ret[1] = r;
	ret[2] = d;
	i2c_read(I2C_DPM_ADDR, 0, -3, ret, 1);
	if (ret[0] == DPM_SUCCESS) { /* the DPM returned success as status */
		debug("WRM_OPCODE returned success data is %x\n", ret[0]);
		return ret[0];
	} else {
		return -1;
	}
}

/* Write Data d into Register r of POL(s) a */
int dpm_wrp(u8 r, u8 d)
{
	u8 ret[7];

	ret[0] = WRP_OPCODE;
	/* only POL0 is present */
	ret[1] = 0x01;
	ret[2] = 0x00;
	ret[3] = 0x00;
	ret[4] = 0x00;
	ret[5] = r;
	ret[6] = d;
	i2c_read(I2C_DPM_ADDR, 0, -7, ret, 1);
	if (ret[0] == DPM_SUCCESS) { /* the DPM returned success as status */
		debug("WRP_OPCODE returned success data is %x\n", ret[0]);
		return 0;
	} else {
		return -1;
	}
}

/* Uses the DPM command RRP */
u8 zm_read(uchar reg)
{
	return dpm_rrp(reg);
}

/* ZM_write --
	Steps:
	a. Write data to the register
	b. Read data from register and compare to written value
	c. Return return_code & voltage_read
*/
u8 zm_write(u8 reg, u8 data)
{
	u8 d;

	/* write data to register */
	dpm_wrp(reg, data);

	/* read register and compare to written value */
	d = dpm_rrp(reg);
	if (d != data) {
		printf("zm_write : Comparison register data failed\n");
		return -1;
	}

	return d;
}

/* zm_write_out_voltage
 * voltage in 1/10 mV
 */
int zm_write_voltage(int voltage)
{
	u8 reg = 0x7, vid;
	uint16_t voltage_read;
	u8 ret;

	vid =  (voltage - 5000) / ZM_STEP;

	ret = zm_write(reg, vid);
	if (ret != -1) {
		voltage_read = hex_to_1_10mv[ret];
		debug("voltage set to %dmV\n", voltage_read/10);
		return voltage_read;
	}
	return -1;
}

/* zm_read_out_voltage
 * voltage in 1/10 mV
 */
int zm_read_voltage(void)
{
	u8 reg = 0x7;
	u8 ret;
	int voltage;

	ret = zm_read(reg);
	if (ret != -1) {
		voltage =  hex_to_1_10mv[ret];
		debug("Voltage read is %dmV\n", voltage/10);
		return voltage;
	} else {
		return -1;
	}
}

int zm_disable_wp()
{
	u8 new_wp_value;

	/* Disable using Write-Protect register 0x96 */
	new_wp_value = 0x8;
	if ((dpm_wrm(DPM_WP, new_wp_value)) < 0) {
		printf("Disable Write-Protect register failed\n");
		return -1;
	}
	return 0;
}

int zm_enable_wp()
{
	u8 orig_wp_value;
	orig_wp_value = 0x0;

	/* Enable using Write-Protect register 0x96 */
	if ((dpm_wrm(DPM_WP, orig_wp_value)) < 0) {
		printf("Enable Write-Protect register failed\n");
		return -1;
	}
	return 0;
}

