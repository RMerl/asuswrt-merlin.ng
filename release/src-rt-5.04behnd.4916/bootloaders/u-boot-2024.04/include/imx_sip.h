/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 */

#ifndef _IMX_SIP_H__
#define _IMX_SIP_H__

#define IMX_SIP_GPC		0xC2000000
#define IMX_SIP_GPC_PM_DOMAIN	0x03

#define IMX_SIP_BUILDINFO			0xC2000003
#define IMX_SIP_BUILDINFO_GET_COMMITHASH	0x00

#define IMX_SIP_SRC		0xC2000005
#define IMX_SIP_SRC_MCU_START	0x00
#define IMX_SIP_SRC_MCU_STARTED	0x01
#define	IMX_SIP_SRC_MCU_STOP	0x02

#endif
