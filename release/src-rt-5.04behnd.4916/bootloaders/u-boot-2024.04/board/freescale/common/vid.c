// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2020-21 NXP
 * Copyright 2020 Stephen Carlson <stcarlso@linux.microsoft.com>
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <i2c.h>
#include <irq_func.h>
#include <log.h>
#include <asm/io.h>
#ifdef CONFIG_FSL_LSCH2
#include <asm/arch/immap_lsch2.h>
#elif defined(CONFIG_FSL_LSCH3)
#include <asm/arch/immap_lsch3.h>
#else
#include <asm/immap_85xx.h>
#endif
#include <linux/delay.h>
#include "i2c_common.h"
#include "vid.h"

#ifndef I2C_VOL_MONITOR_BUS
#define I2C_VOL_MONITOR_BUS                    0
#endif

/* Voltages are generally handled in mV to keep them as integers */
#define MV_PER_V 1000

/*
 * Select the channel on the I2C mux (on some NXP boards) that contains
 * the voltage regulator to use for VID. Return 0 for success or nonzero
 * for failure.
 */
int __weak i2c_multiplexer_select_vid_channel(u8 channel)
{
	return 0;
}

/*
 * Compensate for a board specific voltage drop between regulator and SoC.
 * Returns the voltage offset in mV.
 */
int __weak board_vdd_drop_compensation(void)
{
	return 0;
}

/*
 * Performs any board specific adjustments after the VID voltage has been
 * set. Return 0 for success or nonzero for failure.
 */
int __weak board_adjust_vdd(int vdd)
{
	return 0;
}

/*
 * Processor specific method of converting the fuse value read from VID
 * registers into the core voltage to supply. Return the voltage in mV.
 */
u16 __weak soc_get_fuse_vid(int vid_index)
{
	/* Default VDD for Layerscape Chassis 1 devices */
	static const u16 vdd[32] = {
		0,      /* unused */
		9875,   /* 0.9875V */
		9750,
		9625,
		9500,
		9375,
		9250,
		9125,
		9000,
		8875,
		8750,
		8625,
		8500,
		8375,
		8250,
		8125,
		10000,  /* 1.0000V */
		10125,
		10250,
		10375,
		10500,
		10625,
		10750,
		10875,
		11000,
		0,      /* reserved */
	};
	return vdd[vid_index];
}

#ifndef I2C_VOL_MONITOR_ADDR
#define I2C_VOL_MONITOR_ADDR                    0
#endif

#if defined(CONFIG_VOL_MONITOR_IR36021_SET) || \
	defined(CONFIG_VOL_MONITOR_IR36021_READ)
/*
 * Get the i2c address configuration for the IR regulator chip
 *
 * There are some variance in the RDB HW regarding the I2C address configuration
 * for the IR regulator chip, which is likely a problem of external resistor
 * accuracy. So we just check each address in a hopefully non-intrusive mode
 * and use the first one that seems to work
 *
 * The IR chip can show up under the following addresses:
 * 0x08 (Verified on T1040RDB-PA,T4240RDB-PB,X-T4240RDB-16GPA)
 * 0x09 (Verified on T1040RDB-PA)
 * 0x38 (Verified on T2080QDS, T2081QDS, T4240RDB)
 */
static int find_ir_chip_on_i2c(void)
{
	int i2caddress, ret, i;
	u8 mfrID;
	const int ir_i2c_addr[] = {0x38, 0x08, 0x09};
	DEVICE_HANDLE_T dev;

	/* Check all the address */
	for (i = 0; i < (sizeof(ir_i2c_addr)/sizeof(ir_i2c_addr[0])); i++) {
		i2caddress = ir_i2c_addr[i];
		ret = fsl_i2c_get_device(i2caddress, I2C_VOL_MONITOR_BUS, &dev);
		if (!ret) {
			ret = I2C_READ(dev, IR36021_MFR_ID_OFFSET,
				       (void *)&mfrID, sizeof(mfrID));
			/* If manufacturer ID matches the IR36021 */
			if (!ret && mfrID == IR36021_MFR_ID)
				return i2caddress;
		}
	}
	return -1;
}
#endif

/* Maximum loop count waiting for new voltage to take effect */
#define MAX_LOOP_WAIT_NEW_VOL		100
/* Maximum loop count waiting for the voltage to be stable */
#define MAX_LOOP_WAIT_VOL_STABLE	100
/*
 * read_voltage from sensor on I2C bus
 * We use average of 4 readings, waiting for WAIT_FOR_ADC before
 * another reading
 */
#define NUM_READINGS    4       /* prefer to be power of 2 for efficiency */

/* If an INA220 chip is available, we can use it to read back the voltage
 * as it may have a higher accuracy than the IR chip for the same purpose
 */
#ifdef CONFIG_VOL_MONITOR_INA220
#define WAIT_FOR_ADC	532	/* wait for 532 microseconds for ADC */
#define ADC_MIN_ACCURACY	4
#else
#define WAIT_FOR_ADC	138	/* wait for 138 microseconds for ADC */
#define ADC_MIN_ACCURACY	4
#endif

#ifdef CONFIG_VOL_MONITOR_INA220
static int read_voltage_from_INA220(int i2caddress)
{
	int i, ret, voltage_read = 0;
	u16 vol_mon;
	u8 buf[2];
	DEVICE_HANDLE_T dev;

	/* Open device handle */
	ret = fsl_i2c_get_device(i2caddress, I2C_VOL_MONITOR_BUS, &dev);
	if (ret)
		return ret;

	for (i = 0; i < NUM_READINGS; i++) {
		ret = I2C_READ(dev, I2C_VOL_MONITOR_BUS_V_OFFSET,
			       (void *)&buf[0], sizeof(buf));
		if (ret) {
			printf("VID: failed to read core voltage\n");
			return ret;
		}

		vol_mon = (buf[0] << 8) | buf[1];
		if (vol_mon & I2C_VOL_MONITOR_BUS_V_OVF) {
			printf("VID: Core voltage sensor error\n");
			return -1;
		}

		debug("VID: bus voltage reads 0x%04x\n", vol_mon);
		/* LSB = 4mv */
		voltage_read += (vol_mon >> I2C_VOL_MONITOR_BUS_V_SHIFT) * 4;
		udelay(WAIT_FOR_ADC);
	}

	/* calculate the average */
	voltage_read /= NUM_READINGS;

	return voltage_read;
}
#endif

#ifdef CONFIG_VOL_MONITOR_IR36021_READ
/* read voltage from IR */
static int read_voltage_from_IR(int i2caddress)
{
	int i, ret, voltage_read = 0;
	u16 vol_mon;
	u8 buf;
	DEVICE_HANDLE_T dev;

	/* Open device handle */
	ret = fsl_i2c_get_device(i2caddress, I2C_VOL_MONITOR_BUS, &dev);
	if (ret)
		return ret;

	for (i = 0; i < NUM_READINGS; i++) {
		ret = I2C_READ(dev, IR36021_LOOP1_VOUT_OFFSET, (void *)&buf,
			       sizeof(buf));
		if (ret) {
			printf("VID: failed to read core voltage\n");
			return ret;
		}
		vol_mon = buf;
		if (!vol_mon) {
			printf("VID: Core voltage sensor error\n");
			return -1;
		}
		debug("VID: bus voltage reads 0x%02x\n", vol_mon);
		/* Resolution is 1/128V. We scale up here to get 1/128mV
		 * and divide at the end
		 */
		voltage_read += vol_mon * MV_PER_V;
		udelay(WAIT_FOR_ADC);
	}
	/* Scale down to the real mV as IR resolution is 1/128V, rounding up */
	voltage_read = DIV_ROUND_UP(voltage_read, 128);

	/* calculate the average */
	voltage_read /= NUM_READINGS;

	/* Compensate for a board specific voltage drop between regulator and
	 * SoC before converting into an IR VID value
	 */
	voltage_read -= board_vdd_drop_compensation();

	return voltage_read;
}
#endif

#if defined(CONFIG_VOL_MONITOR_ISL68233_READ) || \
	defined(CONFIG_VOL_MONITOR_LTC3882_READ) || \
	defined(CONFIG_VOL_MONITOR_ISL68233_SET) || \
	defined(CONFIG_VOL_MONITOR_LTC3882_SET)

/*
 * The message displayed if the VOUT exponent causes a resolution
 * worse than 1.0 V (if exponent is >= 0).
 */
#define VOUT_WARNING "VID: VOUT_MODE exponent has resolution worse than 1 V!\n"

/* Checks the PMBus voltage monitor for the format used for voltage values */
static int get_pmbus_multiplier(DEVICE_HANDLE_T dev)
{
	u8 mode;
	int exponent, multiplier, ret;

	ret = I2C_READ(dev, PMBUS_CMD_VOUT_MODE, &mode, sizeof(mode));
	if (ret) {
		printf("VID: unable to determine voltage multiplier\n");
		return 1;
	}

	/* Upper 3 bits is mode, lower 5 bits is exponent */
	exponent = (int)mode & 0x1F;
	mode >>= 5;
	switch (mode) {
	case 0:
		/* Linear, 5 bit twos component exponent */
		if (exponent & 0x10) {
			multiplier = 1 << (16 - (exponent & 0xF));
		} else {
			/* If exponent is >= 0, then resolution is 1 V! */
			printf(VOUT_WARNING);
			multiplier = 1;
		}
		break;
	case 1:
		/* VID code identifier */
		printf("VID: custom VID codes are not supported\n");
		multiplier = MV_PER_V;
		break;
	default:
		/* Direct, in mV */
		multiplier = MV_PER_V;
		break;
	}

	debug("VID: calculated multiplier is %d\n", multiplier);
	return multiplier;
}
#endif

#if defined(CONFIG_VOL_MONITOR_ISL68233_READ) || \
	defined(CONFIG_VOL_MONITOR_LTC3882_READ)
static int read_voltage_from_pmbus(int i2caddress)
{
	int ret, multiplier, vout;
	u8 channel = PWM_CHANNEL0;
	u16 vcode;
	DEVICE_HANDLE_T dev;

	/* Open device handle */
	ret = fsl_i2c_get_device(i2caddress, I2C_VOL_MONITOR_BUS, &dev);
	if (ret)
		return ret;

	/* Select the right page */
	ret = I2C_WRITE(dev, PMBUS_CMD_PAGE, &channel, sizeof(channel));
	if (ret) {
		printf("VID: failed to select VDD page %d\n", channel);
		return ret;
	}

	/* VOUT is little endian */
	ret = I2C_READ(dev, PMBUS_CMD_READ_VOUT, (void *)&vcode, sizeof(vcode));
	if (ret) {
		printf("VID: failed to read core voltage\n");
		return ret;
	}

	/* Scale down to the real mV */
	multiplier = get_pmbus_multiplier(dev);
	vout = (int)vcode;
	/* Multiplier 1000 (direct mode) requires no change to convert */
	if (multiplier != MV_PER_V)
		vout = DIV_ROUND_UP(vout * MV_PER_V, multiplier);
	return vout - board_vdd_drop_compensation();
}
#endif

static int read_voltage(int i2caddress)
{
	int voltage_read;
#ifdef CONFIG_VOL_MONITOR_INA220
	voltage_read = read_voltage_from_INA220(I2C_VOL_MONITOR_ADDR);
#elif defined CONFIG_VOL_MONITOR_IR36021_READ
	voltage_read = read_voltage_from_IR(i2caddress);
#elif defined(CONFIG_VOL_MONITOR_ISL68233_READ) || \
	  defined(CONFIG_VOL_MONITOR_LTC3882_READ)
	voltage_read = read_voltage_from_pmbus(i2caddress);
#else
	voltage_read = -1;
#endif
	return voltage_read;
}

#ifdef CONFIG_VOL_MONITOR_IR36021_SET
/*
 * We need to calculate how long before the voltage stops to drop
 * or increase. It returns with the loop count. Each loop takes
 * several readings (WAIT_FOR_ADC)
 */
static int wait_for_new_voltage(int vdd, int i2caddress)
{
	int timeout, vdd_current;

	vdd_current = read_voltage(i2caddress);
	/* wait until voltage starts to reach the target. Voltage slew
	 * rates by typical regulators will always lead to stable readings
	 * within each fairly long ADC interval in comparison to the
	 * intended voltage delta change until the target voltage is
	 * reached. The fairly small voltage delta change to any target
	 * VID voltage also means that this function will always complete
	 * within few iterations. If the timeout was ever reached, it would
	 * point to a serious failure in the regulator system.
	 */
	for (timeout = 0;
	     abs(vdd - vdd_current) > (IR_VDD_STEP_UP + IR_VDD_STEP_DOWN) &&
	     timeout < MAX_LOOP_WAIT_NEW_VOL; timeout++) {
		vdd_current = read_voltage(i2caddress);
	}
	if (timeout >= MAX_LOOP_WAIT_NEW_VOL) {
		printf("VID: Voltage adjustment timeout\n");
		return -1;
	}
	return timeout;
}

/*
 * Blocks and reads the VID voltage until it stabilizes, or the
 * timeout expires
 */
static int wait_for_voltage_stable(int i2caddress)
{
	int timeout, vdd_current, vdd;

	vdd = read_voltage(i2caddress);
	udelay(NUM_READINGS * WAIT_FOR_ADC);

	vdd_current = read_voltage(i2caddress);
	/*
	 * The maximum timeout is
	 * MAX_LOOP_WAIT_VOL_STABLE * NUM_READINGS * WAIT_FOR_ADC
	 */
	for (timeout = MAX_LOOP_WAIT_VOL_STABLE;
	     abs(vdd - vdd_current) > ADC_MIN_ACCURACY &&
	     timeout > 0; timeout--) {
		vdd = vdd_current;
		udelay(NUM_READINGS * WAIT_FOR_ADC);
		vdd_current = read_voltage(i2caddress);
	}
	if (timeout == 0)
		return -1;
	return vdd_current;
}

/* Sets the VID voltage using the IR36021 */
static int set_voltage_to_IR(int i2caddress, int vdd)
{
	int wait, vdd_last;
	int ret;
	u8 vid;
	DEVICE_HANDLE_T dev;

	/* Open device handle */
	ret = fsl_i2c_get_device(i2caddress, I2C_VOL_MONITOR_BUS, &dev);
	if (ret)
		return ret;

	/* Compensate for a board specific voltage drop between regulator and
	 * SoC before converting into an IR VID value
	 */
	vdd += board_vdd_drop_compensation();
#ifdef CONFIG_FSL_LSCH2
	vid = DIV_ROUND_UP(vdd - 265, 5);
#else
	vid = DIV_ROUND_UP(vdd - 245, 5);
#endif

	ret = I2C_WRITE(dev, IR36021_LOOP1_MANUAL_ID_OFFSET, (void *)&vid,
			sizeof(vid));
	if (ret) {
		printf("VID: failed to write new voltage\n");
		return -1;
	}
	wait = wait_for_new_voltage(vdd, i2caddress);
	if (wait < 0)
		return -1;
	debug("VID: Waited %d us\n", wait * NUM_READINGS * WAIT_FOR_ADC);

	vdd_last = wait_for_voltage_stable(i2caddress);
	if (vdd_last < 0)
		return -1;
	debug("VID: Current voltage is %d mV\n", vdd_last);
	return vdd_last;
}
#endif

#if defined(CONFIG_VOL_MONITOR_ISL68233_SET) || \
	defined(CONFIG_VOL_MONITOR_LTC3882_SET)
static int set_voltage_to_pmbus(int i2caddress, int vdd)
{
	int ret, vdd_last, vdd_target = vdd;
	int count = MAX_LOOP_WAIT_NEW_VOL, temp = 0, multiplier;
	unsigned char value;

	/* The data to be sent with the PMBus command PAGE_PLUS_WRITE */
	u8 buffer[5] = { 0x04, PWM_CHANNEL0, PMBUS_CMD_VOUT_COMMAND, 0, 0 };
	DEVICE_HANDLE_T dev;

	/* Open device handle */
	ret = fsl_i2c_get_device(i2caddress, I2C_VOL_MONITOR_BUS, &dev);
	if (ret)
		return ret;

	/* Scale up to the proper value for the VOUT command, little endian */
	multiplier = get_pmbus_multiplier(dev);
	vdd += board_vdd_drop_compensation();
	if (multiplier != MV_PER_V)
		vdd = DIV_ROUND_UP(vdd * multiplier, MV_PER_V);
	buffer[3] = vdd & 0xFF;
	buffer[4] = (vdd & 0xFF00) >> 8;

	/* Check write protect state */
	ret = I2C_READ(dev, PMBUS_CMD_WRITE_PROTECT, (void *)&value,
		       sizeof(value));
	if (ret)
		goto exit;

	if (value != EN_WRITE_ALL_CMD) {
		value = EN_WRITE_ALL_CMD;
		ret = I2C_WRITE(dev, PMBUS_CMD_WRITE_PROTECT,
				(void *)&value, sizeof(value));
		if (ret)
			goto exit;
	}

	/* Write the desired voltage code to the regulator */
	ret = I2C_WRITE(dev, PMBUS_CMD_PAGE_PLUS_WRITE, (void *)&buffer[0],
			sizeof(buffer));
	if (ret) {
		printf("VID: I2C failed to write to the voltage regulator\n");
		return -1;
	}

exit:
	/* Wait for the voltage to get to the desired value */
	do {
		vdd_last = read_voltage_from_pmbus(i2caddress);
		if (vdd_last < 0) {
			printf("VID: Couldn't read sensor abort VID adjust\n");
			return -1;
		}
		count--;
		temp = vdd_last - vdd_target;
	} while ((abs(temp) > 2)  && (count > 0));

	return vdd_last;
}
#endif

static int set_voltage(int i2caddress, int vdd)
{
	int vdd_last = -1;

#ifdef CONFIG_VOL_MONITOR_IR36021_SET
	vdd_last = set_voltage_to_IR(i2caddress, vdd);
#elif defined(CONFIG_VOL_MONITOR_ISL68233_SET) || \
	  defined(CONFIG_VOL_MONITOR_LTC3882_SET)
	vdd_last = set_voltage_to_pmbus(i2caddress, vdd);
#else
	#error Specific voltage monitor must be defined
#endif
	return vdd_last;
}

int adjust_vdd(ulong vdd_override)
{
	int re_enable = disable_interrupts();
#if defined(CONFIG_FSL_LSCH2) || defined(CONFIG_FSL_LSCH3)
	struct ccsr_gur *gur = (void *)(CFG_SYS_FSL_GUTS_ADDR);
#else
	ccsr_gur_t __iomem *gur =
		(void __iomem *)(CFG_SYS_MPC85xx_GUTS_ADDR);
#endif
	u8 vid;
	u32 fusesr;
	int vdd_current, vdd_last, vdd_target;
	int ret, i2caddress = I2C_VOL_MONITOR_ADDR;
	unsigned long vdd_string_override;
	char *vdd_string;

#if defined(CONFIG_VOL_MONITOR_IR36021_SET) || \
	defined(CONFIG_VOL_MONITOR_IR36021_READ)
	u8 buf;
	DEVICE_HANDLE_T dev;
#endif

	/*
	 * VID is used according to the table below
	 *                ---------------------------------------
	 *                |                DA_V                 |
	 *                |-------------------------------------|
	 *                | 5b00000 | 5b00001-5b11110 | 5b11111 |
	 * ---------------+---------+-----------------+---------|
	 * | D | 5b00000  | NO VID  | VID = DA_V      | NO VID  |
	 * | A |----------+---------+-----------------+---------|
	 * | _ | 5b00001  |VID =    | VID =           |VID =    |
	 * | V |   ~      | DA_V_ALT|   DA_V_ALT      | DA_A_VLT|
	 * | _ | 5b11110  |         |                 |         |
	 * | A |----------+---------+-----------------+---------|
	 * | L | 5b11111  | No VID  | VID = DA_V      | NO VID  |
	 * | T |          |         |                 |         |
	 * ------------------------------------------------------
	 */
#if defined(CONFIG_FSL_LSCH3)
	fusesr = in_le32(&gur->dcfg_fusesr);
	vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_ALTVID_SHIFT) &
	       FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK;
	if (vid == 0 || vid == FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK) {
		vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_VID_SHIFT) &
		       FSL_CHASSIS3_DCFG_FUSESR_VID_MASK;
	}
#elif defined(CONFIG_FSL_LSCH2)
	fusesr = in_be32(&gur->dcfg_fusesr);
	vid = (fusesr >> FSL_CHASSIS2_DCFG_FUSESR_ALTVID_SHIFT) &
	       FSL_CHASSIS2_DCFG_FUSESR_ALTVID_MASK;
	if (vid == 0 || vid == FSL_CHASSIS2_DCFG_FUSESR_ALTVID_MASK) {
		vid = (fusesr >> FSL_CHASSIS2_DCFG_FUSESR_VID_SHIFT) &
		       FSL_CHASSIS2_DCFG_FUSESR_VID_MASK;
	}
#else
	fusesr = in_be32(&gur->dcfg_fusesr);
	vid = (fusesr >> FSL_CORENET_DCFG_FUSESR_ALTVID_SHIFT) &
	       FSL_CORENET_DCFG_FUSESR_ALTVID_MASK;
	if (vid == 0 || vid == FSL_CORENET_DCFG_FUSESR_ALTVID_MASK) {
		vid = (fusesr >> FSL_CORENET_DCFG_FUSESR_VID_SHIFT) &
		       FSL_CORENET_DCFG_FUSESR_VID_MASK;
	}
#endif
	vdd_target = soc_get_fuse_vid((int)vid);

	ret = i2c_multiplexer_select_vid_channel(I2C_MUX_CH_VOL_MONITOR);
	if (ret) {
		debug("VID: I2C failed to switch channel\n");
		ret = -1;
		goto exit;
	}

#if defined(CONFIG_VOL_MONITOR_IR36021_SET) || \
	defined(CONFIG_VOL_MONITOR_IR36021_READ)
	ret = find_ir_chip_on_i2c();
	if (ret < 0) {
		printf("VID: Could not find voltage regulator on I2C.\n");
		ret = -1;
		goto exit;
	} else {
		i2caddress = ret;
		debug("VID: IR Chip found on I2C address 0x%02x\n", i2caddress);
	}

	ret = fsl_i2c_get_device(i2caddress, I2C_VOL_MONITOR_BUS, &dev);
	if (ret)
		return ret;

	/* check IR chip work on Intel mode */
	ret = I2C_READ(dev, IR36021_INTEL_MODE_OFFSET, (void *)&buf,
		       sizeof(buf));
	if (ret) {
		printf("VID: failed to read IR chip mode.\n");
		ret = -1;
		goto exit;
	}
	if ((buf & IR36021_MODE_MASK) != IR36021_INTEL_MODE) {
		printf("VID: IR Chip is not used in Intel mode.\n");
		ret = -1;
		goto exit;
	}
#endif

	/* check override variable for overriding VDD */
	vdd_string = env_get(CONFIG_VID_FLS_ENV);
	debug("VID: Initial VDD value is %d mV\n",
	      DIV_ROUND_UP(vdd_target, 10));
	if (vdd_override == 0 && vdd_string &&
	    !strict_strtoul(vdd_string, 10, &vdd_string_override))
		vdd_override = vdd_string_override;
	if (vdd_override >= VDD_MV_MIN && vdd_override <= VDD_MV_MAX) {
		vdd_target = vdd_override * 10; /* convert to 1/10 mV */
		debug("VID: VDD override is %lu\n", vdd_override);
	} else if (vdd_override != 0) {
		printf("VID: Invalid VDD value.\n");
	}
	if (vdd_target == 0) {
		debug("VID: VID not used\n");
		ret = 0;
		goto exit;
	} else {
		/* divide and round up by 10 to get a value in mV */
		vdd_target = DIV_ROUND_UP(vdd_target, 10);
		debug("VID: vid = %d mV\n", vdd_target);
	}

	/*
	 * Read voltage monitor to check real voltage.
	 */
	vdd_last = read_voltage(i2caddress);
	if (vdd_last < 0) {
		printf("VID: Couldn't read sensor abort VID adjustment\n");
		ret = -1;
		goto exit;
	}
	vdd_current = vdd_last;
	debug("VID: Core voltage is currently at %d mV\n", vdd_last);

#if defined(CONFIG_VOL_MONITOR_LTC3882_SET) || \
	defined(CONFIG_VOL_MONITOR_ISL68233_SET)
	/* Set the target voltage */
	vdd_current = set_voltage(i2caddress, vdd_target);
	vdd_last = vdd_current;
#else
	/*
	  * Adjust voltage to at or one step above target.
	  * As measurements are less precise than setting the values
	  * we may run through dummy steps that cancel each other
	  * when stepping up and then down.
	  */
	while (vdd_last > 0 &&
	       vdd_last < vdd_target) {
		vdd_current += IR_VDD_STEP_UP;
		vdd_last = set_voltage(i2caddress, vdd_current);
	}
	while (vdd_last > 0 &&
	       vdd_last > vdd_target + (IR_VDD_STEP_DOWN - 1)) {
		vdd_current -= IR_VDD_STEP_DOWN;
		vdd_last = set_voltage(i2caddress, vdd_current);
	}
#endif

	/* Board specific adjustments */
	if (board_adjust_vdd(vdd_target) < 0) {
		ret = -1;
		goto exit;
	}

	if (vdd_last > 0)
		printf("VID: Core voltage after adjustment is at %d mV\n",
		       vdd_last);
	else
		ret = -1;
exit:
	if (re_enable)
		enable_interrupts();

	i2c_multiplexer_select_vid_channel(I2C_MUX_CH_DEFAULT);

	return ret;
}

static int print_vdd(void)
{
	int vdd_last, ret, i2caddress = I2C_VOL_MONITOR_ADDR;

	ret = i2c_multiplexer_select_vid_channel(I2C_MUX_CH_VOL_MONITOR);
	if (ret) {
		debug("VID : I2c failed to switch channel\n");
		return -1;
	}
#if defined(CONFIG_VOL_MONITOR_IR36021_SET) || \
	defined(CONFIG_VOL_MONITOR_IR36021_READ)
	ret = find_ir_chip_on_i2c();
	if (ret < 0) {
		printf("VID: Could not find voltage regulator on I2C.\n");
		goto exit;
	} else {
		i2caddress = ret;
		debug("VID: IR Chip found on I2C address 0x%02x\n", i2caddress);
	}
#endif

	/*
	 * Read voltage monitor to check real voltage.
	 */
	vdd_last = read_voltage(i2caddress);
	if (vdd_last < 0) {
		printf("VID: Couldn't read sensor abort VID adjustment\n");
		goto exit;
	}
	printf("VID: Core voltage is at %d mV\n", vdd_last);
exit:
	i2c_multiplexer_select_vid_channel(I2C_MUX_CH_DEFAULT);

	return ret < 0 ? -1 : 0;
}

static int do_vdd_override(struct cmd_tbl *cmdtp,
			   int flag, int argc,
			   char *const argv[])
{
	ulong override;
	int ret = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strict_strtoul(argv[1], 10, &override)) {
		ret = adjust_vdd(override);
		if (ret < 0)
			return CMD_RET_FAILURE;
	} else
		return CMD_RET_USAGE;
	return 0;
}

static int do_vdd_read(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	if (argc < 1)
		return CMD_RET_USAGE;
	print_vdd();

	return 0;
}

U_BOOT_CMD(
	vdd_override, 2, 0, do_vdd_override,
	"override VDD",
	" - override with the voltage specified in mV, eg. 1050"
);

U_BOOT_CMD(
	vdd_read, 1, 0, do_vdd_read,
	"read VDD",
	" - Read the voltage specified in mV"
);
