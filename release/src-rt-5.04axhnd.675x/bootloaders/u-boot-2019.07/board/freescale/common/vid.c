// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/io.h>
#ifdef CONFIG_FSL_LSCH2
#include <asm/arch/immap_lsch2.h>
#elif defined(CONFIG_FSL_LSCH3)
#include <asm/arch/immap_lsch3.h>
#else
#include <asm/immap_85xx.h>
#endif
#include "vid.h"

int __weak i2c_multiplexer_select_vid_channel(u8 channel)
{
	return 0;
}

/*
 * Compensate for a board specific voltage drop between regulator and SoC
 * return a value in mV
 */
int __weak board_vdd_drop_compensation(void)
{
	return 0;
}

/*
 * Board specific settings for specific voltage value
 */
int __weak board_adjust_vdd(int vdd)
{
	return 0;
}

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
	int i2caddress;
	int ret;
	u8 byte;
	int i;
	const int ir_i2c_addr[] = {0x38, 0x08, 0x09};

	/* Check all the address */
	for (i = 0; i < (sizeof(ir_i2c_addr)/sizeof(ir_i2c_addr[0])); i++) {
		i2caddress = ir_i2c_addr[i];
		ret = i2c_read(i2caddress,
			       IR36021_MFR_ID_OFFSET, 1, (void *)&byte,
			       sizeof(byte));
		if ((ret >= 0) && (byte == IR36021_MFR_ID))
			return i2caddress;
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

	for (i = 0; i < NUM_READINGS; i++) {
		ret = i2c_read(I2C_VOL_MONITOR_ADDR,
			       I2C_VOL_MONITOR_BUS_V_OFFSET, 1,
			       (void *)&buf, 2);
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

/* read voltage from IR */
#ifdef CONFIG_VOL_MONITOR_IR36021_READ
static int read_voltage_from_IR(int i2caddress)
{
	int i, ret, voltage_read = 0;
	u16 vol_mon;
	u8 buf;

	for (i = 0; i < NUM_READINGS; i++) {
		ret = i2c_read(i2caddress,
			       IR36021_LOOP1_VOUT_OFFSET,
			       1, (void *)&buf, 1);
		if (ret) {
			printf("VID: failed to read vcpu\n");
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
		voltage_read += vol_mon * 1000;
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

#ifdef CONFIG_VOL_MONITOR_LTC3882_READ
/* read the current value of the LTC Regulator Voltage */
static int read_voltage_from_LTC(int i2caddress)
{
	int  ret, vcode = 0;
	u8 chan = PWM_CHANNEL0;

	/* select the PAGE 0 using PMBus commands PAGE for VDD*/
	ret = i2c_write(I2C_VOL_MONITOR_ADDR,
			PMBUS_CMD_PAGE, 1, &chan, 1);
	if (ret) {
		printf("VID: failed to select VDD Page 0\n");
		return ret;
	}

	/*read the output voltage using PMBus command READ_VOUT*/
	ret = i2c_read(I2C_VOL_MONITOR_ADDR,
		       PMBUS_CMD_READ_VOUT, 1, (void *)&vcode, 2);
	if (ret) {
		printf("VID: failed to read the volatge\n");
		return ret;
	}

	/* Scale down to the real mV as LTC resolution is 1/4096V,rounding up */
	vcode = DIV_ROUND_UP(vcode * 1000, 4096);

	return vcode;
}
#endif

static int read_voltage(int i2caddress)
{
	int voltage_read;
#ifdef CONFIG_VOL_MONITOR_INA220
	voltage_read = read_voltage_from_INA220(i2caddress);
#elif defined CONFIG_VOL_MONITOR_IR36021_READ
	voltage_read = read_voltage_from_IR(i2caddress);
#elif defined CONFIG_VOL_MONITOR_LTC3882_READ
	voltage_read = read_voltage_from_LTC(i2caddress);
#else
	return -1;
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
 * this function keeps reading the voltage until it is stable or until the
 * timeout expires
 */
static int wait_for_voltage_stable(int i2caddress)
{
	int timeout, vdd_current, vdd;

	vdd = read_voltage(i2caddress);
	udelay(NUM_READINGS * WAIT_FOR_ADC);

	/* wait until voltage is stable */
	vdd_current = read_voltage(i2caddress);
	/* The maximum timeout is
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

/* Set the voltage to the IR chip */
static int set_voltage_to_IR(int i2caddress, int vdd)
{
	int wait, vdd_last;
	int ret;
	u8 vid;

	/* Compensate for a board specific voltage drop between regulator and
	 * SoC before converting into an IR VID value
	 */
	vdd += board_vdd_drop_compensation();
#ifdef CONFIG_FSL_LSCH2
	vid = DIV_ROUND_UP(vdd - 265, 5);
#else
	vid = DIV_ROUND_UP(vdd - 245, 5);
#endif

	ret = i2c_write(i2caddress, IR36021_LOOP1_MANUAL_ID_OFFSET,
			1, (void *)&vid, sizeof(vid));
	if (ret) {
		printf("VID: failed to write VID\n");
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

#ifdef CONFIG_VOL_MONITOR_LTC3882_SET
/* this function sets the VDD and returns the value set */
static int set_voltage_to_LTC(int i2caddress, int vdd)
{
	int ret, vdd_last, vdd_target = vdd;
	int count = 100, temp = 0;

	/* Scale up to the LTC resolution is 1/4096V */
	vdd = (vdd * 4096) / 1000;

	/* 5-byte buffer which needs to be sent following the
	 * PMBus command PAGE_PLUS_WRITE.
	 */
	u8 buff[5] = {0x04, PWM_CHANNEL0, PMBUS_CMD_VOUT_COMMAND,
			vdd & 0xFF, (vdd & 0xFF00) >> 8};

	/* Write the desired voltage code to the regulator */
	ret = i2c_write(I2C_VOL_MONITOR_ADDR,
			PMBUS_CMD_PAGE_PLUS_WRITE, 1, (void *)&buff, 5);
	if (ret) {
		printf("VID: I2C failed to write to the volatge regulator\n");
		return -1;
	}

	/* Wait for the volatge to get to the desired value */
	do {
		vdd_last = read_voltage_from_LTC(i2caddress);
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
#elif defined CONFIG_VOL_MONITOR_LTC3882_SET
	vdd_last = set_voltage_to_LTC(i2caddress, vdd);
#else
	#error Specific voltage monitor must be defined
#endif
	return vdd_last;
}

#ifdef CONFIG_FSL_LSCH3
int adjust_vdd(ulong vdd_override)
{
	int re_enable = disable_interrupts();
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 fusesr;
#if defined(CONFIG_VOL_MONITOR_IR36021_SET) || \
	defined(CONFIG_VOL_MONITOR_IR36021_READ)
	u8 vid, buf;
#else
	u8 vid;
#endif
	int vdd_target, vdd_current, vdd_last;
	int ret, i2caddress;
	unsigned long vdd_string_override;
	char *vdd_string;
#ifdef CONFIG_ARCH_LX2160A
	static const u16 vdd[32] = {
		8250,
		7875,
		7750,
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		8000,
		8125,
		8250,
		0,      /* reserved */
		8500,
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
	};
#else
#ifdef CONFIG_ARCH_LS1088A
	static const uint16_t vdd[32] = {
		10250,
		9875,
		9750,
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		9000,
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		10000,  /* 1.0000V */
		10125,
		10250,
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
	};

#else
	static const uint16_t vdd[32] = {
		10500,
		0,      /* reserved */
		9750,
		0,      /* reserved */
		9500,
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		9000,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		10000,  /* 1.0000V */
		0,      /* reserved */
		10250,
		0,      /* reserved */
		10500,
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
	};
#endif
#endif
	struct vdd_drive {
		u8 vid;
		unsigned voltage;
	};

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

	/* check IR chip work on Intel mode*/
	ret = i2c_read(i2caddress,
		       IR36021_INTEL_MODE_OOFSET,
		       1, (void *)&buf, 1);
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

	/* get the voltage ID from fuse status register */
	fusesr = in_le32(&gur->dcfg_fusesr);
	vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_ALTVID_SHIFT) &
		FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK;
	if ((vid == 0) || (vid == FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK)) {
		vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_VID_SHIFT) &
			FSL_CHASSIS3_DCFG_FUSESR_VID_MASK;
	}
	vdd_target = vdd[vid];

	/* check override variable for overriding VDD */
	vdd_string = env_get(CONFIG_VID_FLS_ENV);
	if (vdd_override == 0 && vdd_string &&
	    !strict_strtoul(vdd_string, 10, &vdd_string_override))
		vdd_override = vdd_string_override;

	if (vdd_override >= VDD_MV_MIN && vdd_override <= VDD_MV_MAX) {
		vdd_target = vdd_override * 10; /* convert to 1/10 mV */
		debug("VDD override is %lu\n", vdd_override);
	} else if (vdd_override != 0) {
		printf("Invalid value.\n");
	}

	/* divide and round up by 10 to get a value in mV */
	vdd_target = DIV_ROUND_UP(vdd_target, 10);
	if (vdd_target == 0) {
		debug("VID: VID not used\n");
		ret = 0;
		goto exit;
	} else if (vdd_target < VDD_MV_MIN || vdd_target > VDD_MV_MAX) {
		/* Check vdd_target is in valid range */
		printf("VID: Target VID %d mV is not in range.\n",
		       vdd_target);
		ret = -1;
		goto exit;
	} else {
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

#ifdef CONFIG_VOL_MONITOR_LTC3882_SET
	/* Set the target voltage */
	vdd_last = vdd_current = set_voltage(i2caddress, vdd_target);
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
#else /* !CONFIG_FSL_LSCH3 */
int adjust_vdd(ulong vdd_override)
{
	int re_enable = disable_interrupts();
#if defined(CONFIG_FSL_LSCH2)
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
#else
	ccsr_gur_t __iomem *gur =
		(void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#endif
	u32 fusesr;
	u8 vid, buf;
	int vdd_target, vdd_current, vdd_last;
	int ret, i2caddress;
	unsigned long vdd_string_override;
	char *vdd_string;
	static const uint16_t vdd[32] = {
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
	struct vdd_drive {
		u8 vid;
		unsigned voltage;
	};

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

	/* check IR chip work on Intel mode*/
	ret = i2c_read(i2caddress,
		       IR36021_INTEL_MODE_OOFSET,
		       1, (void *)&buf, 1);
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

	/* get the voltage ID from fuse status register */
	fusesr = in_be32(&gur->dcfg_fusesr);
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
#ifdef CONFIG_FSL_LSCH2
	vid = (fusesr >> FSL_CHASSIS2_DCFG_FUSESR_ALTVID_SHIFT) &
		FSL_CHASSIS2_DCFG_FUSESR_ALTVID_MASK;
	if ((vid == 0) || (vid == FSL_CHASSIS2_DCFG_FUSESR_ALTVID_MASK)) {
		vid = (fusesr >> FSL_CHASSIS2_DCFG_FUSESR_VID_SHIFT) &
			FSL_CHASSIS2_DCFG_FUSESR_VID_MASK;
	}
#else
	vid = (fusesr >> FSL_CORENET_DCFG_FUSESR_ALTVID_SHIFT) &
		FSL_CORENET_DCFG_FUSESR_ALTVID_MASK;
	if ((vid == 0) || (vid == FSL_CORENET_DCFG_FUSESR_ALTVID_MASK)) {
		vid = (fusesr >> FSL_CORENET_DCFG_FUSESR_VID_SHIFT) &
			FSL_CORENET_DCFG_FUSESR_VID_MASK;
	}
#endif
	vdd_target = vdd[vid];

	/* check override variable for overriding VDD */
	vdd_string = env_get(CONFIG_VID_FLS_ENV);
	if (vdd_override == 0 && vdd_string &&
	    !strict_strtoul(vdd_string, 10, &vdd_string_override))
		vdd_override = vdd_string_override;
	if (vdd_override >= VDD_MV_MIN && vdd_override <= VDD_MV_MAX) {
		vdd_target = vdd_override * 10; /* convert to 1/10 mV */
		debug("VDD override is %lu\n", vdd_override);
	} else if (vdd_override != 0) {
		printf("Invalid value.\n");
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
#endif

static int print_vdd(void)
{
	int vdd_last, ret, i2caddress;

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

static int do_vdd_override(cmd_tbl_t *cmdtp,
			   int flag, int argc,
			   char * const argv[])
{
	ulong override;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strict_strtoul(argv[1], 10, &override))
		adjust_vdd(override);   /* the value is checked by callee */
	else
		return CMD_RET_USAGE;
	return 0;
}

static int do_vdd_read(cmd_tbl_t *cmdtp,
			 int flag, int argc,
			 char * const argv[])
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
)
