/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2020 NXP
 */

#ifndef __VID_H_
#define __VID_H_

/* IR36021 command codes */
#define IR36021_LOOP1_MANUAL_ID_OFFSET	0x6A
#define IR36021_LOOP1_VOUT_OFFSET	0x9A
#define IR36021_MFR_ID_OFFSET		0x92
#define IR36021_MFR_ID			0x43
#define IR36021_INTEL_MODE_OFFSET	0x14
#define IR36021_MODE_MASK		0x20
#define IR36021_INTEL_MODE		0x00
#define IR36021_AMD_MODE		0x20

/* Step the IR regulator in 5mV increments */
#define IR_VDD_STEP_DOWN		5
#define IR_VDD_STEP_UP			5

/* LTC3882 */
#define PMBUS_CMD_WRITE_PROTECT         0x10
/*
 * WRITE_PROTECT command supported values
 * 0x80: Disable all writes except WRITE_PROTECT, PAGE,
 *       STORE_USER_ALL and MFR_EE_UNLOCK commands.
 * 0x40: Disable all writes except WRITE_PROTECT, PAGE, STORE_USER_ALL,
 *       MFR_EE_UNLOCK, OPERATION, CLEAR_PEAKS and CLEAR_FAULTS commands.
 *       Individual faults can also be cleared by writing a 1 to the
 *       respective status bit.
 * 0x20: Disable all writes except WRITE_PROTECT, PAGE, STORE_USER_ ALL,
 *       MFR_EE_UNLOCK, OPERATION, CLEAR_PEAKS, CLEAR_FAULTS, ON_OFF_CONFIG
 *       and VOUT_COMMAND commands. Individual faults can be cleared by
 *       writing a 1 to the respective status bit.
 * 0x00: Enables write to all commands
 */
#define EN_WRITE_ALL_CMD (0)

#ifdef CONFIG_TARGET_LX2160ARDB
/* The lowest and highest voltage allowed*/
#define VDD_MV_MIN			775
#define VDD_MV_MAX			855
#endif

#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
/* The lowest and highest voltage allowed*/
#define VDD_MV_MIN			775
#define VDD_MV_MAX			925
#endif

/* PM Bus commands code for LTC3882*/
#define PWM_CHANNEL0                    0x0
#define PMBUS_CMD_PAGE                  0x0
#define PMBUS_CMD_READ_VOUT             0x8B
#define PMBUS_CMD_VOUT_MODE			0x20
#define PMBUS_CMD_VOUT_COMMAND          0x21
#define PMBUS_CMD_PAGE_PLUS_WRITE       0x05

#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS) || \
defined(CONFIG_TARGET_LX2160ARDB)
/* Voltage monitor on channel 2*/
#define I2C_VOL_MONITOR_BUS_V_OFFSET	0x2
#define I2C_VOL_MONITOR_BUS_V_OVF	0x1
#define I2C_VOL_MONITOR_BUS_V_SHIFT	3
#define I2C_VOL_MONITOR_ADDR            0x63
#define I2C_MUX_CH_VOL_MONITOR		0xA
#endif

int adjust_vdd(ulong vdd_override);
u16 soc_get_fuse_vid(int vid_index);

#endif  /* __VID_H_ */
