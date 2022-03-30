/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */

#ifndef _OTP_HW_H
#define _OTP_HW_H

#include "otp_hw_map.h"

/*
 * includes common OTP controller offsets
 * */

#define OTP_STATUS_CMD_DONE_TMO_CNT 		65536
#define OTP_CPU_LOCK_TMO_CNT 			65536


#if defined (CONFIG_OTP_V1)
/* 40nm SoC */

#define OTP_CTRL0_ACCESS_MODE			(0x2 << 22)
#define OTP_CTRL0_PROG_EN			(0x1 << 21)
#define OTP_CTRL0_START				(0x1 << 0)
#define OTP_CTRL0_CMD_OTP_PROG_EN		(0x2 << 1)
#define OTP_CTRL0_CMD_PROG			(0xA << 1)
#define OTP_CTRL0_CMD_PROG_LOCK			(0x19 << 1)

#define OTP_CTRL0_PROG_MODE_ENABLE		(OTP_CTRL0_START|OTP_CTRL0_PROG_EN|OTP_CTRL0_CMD_OTP_PROG_EN)
#define OTP_CTRL0_PROG_CMD_START		(OTP_CTRL0_START|OTP_CTRL0_CMD_PROG|OTP_CTRL0_ACCESS_MODE|OTP_CTRL0_PROG_EN)

#define OTP_CTRL0_OFFSET			0x0

#define OTP_CTRL1_CPU_MODE			(1 << 0)
#define OTP_CTRL1_OFFSET			0x4
#define OTP_CTRL2_OFFSET			0x8
#define OTP_CTRL3_OFFSET			0xC

#define OTP_STATUS0_OFFSET			0x14

#define OTP_STATUS1_PROG_OK			(1 << 2)
#define OTP_STATUS1_CMD_DONE			(1 << 1)
#define OTP_STATUS1_OFFSET			0x18

#elif defined(CONFIG_OTP_V2) || defined(CONFIG_OTP_V3)

/*  28-16nm SoC */
#define OTP_CTRL0_START				(0x1 << 0)
#define OTP_CTRL0_CMD_OTP_PROG_EN		(0x2 << 1)
#define OTP_CTRL0_CMD_PROG			(0xA << 1)
#define OTP_CTRL0_CMD_PROG_LOCK			(0x19 << 1)
#define OTP_CTRL0_PROG_MODE_ENABLE		(OTP_CTRL0_START|OTP_CTRL0_CMD_OTP_PROG_EN)
#define OTP_CTRL0_PROG_CMD_START		(OTP_CTRL0_START|OTP_CTRL0_CMD_PROG) 
#define OTP_CTRL0_OFFSET			0x0

#define OTP_CTRL1_CPU_MODE			(1 << 0)
#define OTP_CTRL1_OFFSET			0x4
#define OTP_CTRL2_OFFSET			0x8
#define OTP_CTRL2_HI_OFFSET			0xC
#define OTP_CTRL3_OFFSET			0x10

#define OTP_STATUS0_OFFSET			0x18
#define OTP_STATUS0_HI_OFFSET			0x1C

#define OTP_STATUS1_PROG_OK			(1 << 2)
#define OTP_STATUS1_CMD_DONE			(1 << 1)
#define OTP_STATUS1_OFFSET			0x20


#define OTP_CPU_LOCK_SHIFT			0x0
#define OTP_CPU_LOCK_MASK			0x1


#if defined(CONFIG_OTP_LOCK)

#if defined(CONFIG_OTP_V3)
/* 16nm SoC*/
#define OTP_CPU_LOCK_OFFSET			0x54
#else
#define OTP_CPU_LOCK_OFFSET			0x70
#endif

#endif
#else

#error OTP controller is not defined  

#endif

#endif
