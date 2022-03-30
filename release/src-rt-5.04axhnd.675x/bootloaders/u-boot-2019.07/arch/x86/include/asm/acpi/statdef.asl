/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 * Copyright (C) 2016 Bin Meng <bmeng.cn@gmail.com>
 *
 * Modified from coreboot src/arch/x86/acpi/statdef.asl
 */

/* Status and notification definitions */

#define STA_MISSING		0x00
#define STA_PRESENT		0x01
#define STA_ENABLED		0x03
#define STA_DISABLED		0x09
#define STA_INVISIBLE		0x0b
#define STA_UNAVAILABLE		0x0d
#define STA_VISIBLE		0x0f

/* SMBus status codes */
#define SMB_OK			0x00
#define SMB_UNKNOWN_FAIL	0x07
#define SMB_DEV_ADDR_NAK	0x10
#define SMB_DEVICE_ERROR	0x11
#define SMB_DEV_CMD_DENIED	0x12
#define SMB_UNKNOWN_ERR		0x13
#define SMB_DEV_ACC_DENIED	0x17
#define SMB_TIMEOUT		0x18
#define SMB_HST_UNSUPP_PROTOCOL	0x19
#define SMB_BUSY		0x1a
#define SMB_PKT_CHK_ERROR	0x1f

/* Device Object Notification Values */
#define NOTIFY_BUS_CHECK	0x00
#define NOTIFY_DEVICE_CHECK	0x01
#define NOTIFY_DEVICE_WAKE	0x02
#define NOTIFY_EJECT_REQUEST	0x03
#define NOTIFY_DEVICE_CHECK_JR	0x04
#define NOTIFY_FREQUENCY_ERROR	0x05
#define NOTIFY_BUS_MODE		0x06
#define NOTIFY_POWER_FAULT	0x07
#define NOTIFY_CAPABILITIES	0x08
#define NOTIFY_PLD_CHECK	0x09
#define NOTIFY_SLIT_UPDATE	0x0b
#define NOTIFY_SRA_UPDATE	0x0d

/* Battery Device Notification Values */
#define NOTIFY_BAT_STATUSCHG	0x80
#define NOTIFY_BAT_INFOCHG	0x81
#define NOTIFY_BAT_MAINTDATA	0x82

/* Power Source Object Notification Values */
#define NOTIFY_PWR_STATUSCHG	0x80
#define NOTIFY_PWR_INFOCHG	0x81

/* Thermal Zone Object Notification Values */
#define NOTIFY_TZ_STATUSCHG	0x80
#define NOTIFY_TZ_TRIPPTCHG	0x81
#define NOTIFY_TZ_DEVLISTCHG	0x82
#define NOTIFY_TZ_RELTBLCHG	0x83

/* Power Button Notification Values */
#define NOTIFY_POWER_BUTTON	0x80

/* Sleep Button Notification Values */
#define NOTIFY_SLEEP_BUTTON	0x80

/* Lid Notification Values */
#define NOTIFY_LID_STATUSCHG	0x80

/* Processor Device Notification Values */
#define NOTIFY_CPU_PPCCHG	0x80
#define NOTIFY_CPU_CSTATECHG	0x81
#define NOTIFY_CPU_THROTLCHG	0x82

/* User Presence Device Notification Values */
#define NOTIFY_USR_PRESNCECHG	0x80

/* Ambient Light Sensor Notification Values */
#define NOTIFY_ALS_ILLUMCHG	0x80
#define NOTIFY_ALS_COLORTMPCHG	0x81
#define NOTIFY_ALS_RESPCHG	0x82
