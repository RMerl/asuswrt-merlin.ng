/*
 * Copyright (c) 2018 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php or from the Free Software
 * Foundation at https://www.gnu.org/licenses/ .
 */

#ifndef _BRCMSTB_AVS_DVFS_H
#define _BRCMSTB_AVS_DVFS_H

#include <linux/types.h>
#include <linux/errno.h>
struct platform_device;

/* AVS Commands */
#define AVS_CMD_AVAILABLE	0x00
#define AVS_CMD_DISABLE		0x10
#define AVS_CMD_ENABLE		0x11
#define AVS_CMD_S2_ENTER	0x12
#define AVS_CMD_S2_EXIT		0x13
#define AVS_CMD_BBM_ENTER	0x14
#define AVS_CMD_BBM_EXIT	0x15
#define AVS_CMD_S3_ENTER	0x16
#define AVS_CMD_S3_EXIT		0x17
#define AVS_CMD_BALANCE		0x18
/* PMAP and P-STATE commands */
#define AVS_CMD_GET_PMAP	0x30
#define AVS_CMD_SET_PMAP	0x31
#define AVS_CMD_GET_PSTATE	0x40
#define AVS_CMD_SET_PSTATE	0x41
/* Read sensor/debug */
#define AVS_CMD_READ_SENSOR	0x50
#define AVS_CMD_READ_DEBUG	0x51
#define AVS_CMD_CALC_FREQ	0x52
#define AVS_CMD_GET_PMIC_INFO	0x53
#define AVS_CMD_SET_PMIC_CONFIG	0x54
#define AVS_CMD_GET_PMIC_STATUS	0x55
#define AVS_CMD_GET_PMIC_REG_INFO	0x56
#define AVS_CMD_SET_PMIC_REG_CONFIG	0x57
#define AVS_CMD_GET_PMIC_REG_STATUS	0x58
#define AVS_CMD_SET_PMIC_IRQ_ENABLE	0x59
#define AVS_CMD_GET_PMIC_IRQ_STATUS	0x5A
#define AVS_CMD_SET_PMIC_IRQ_CLEAR	0x5B
#define AVS_CMD_GET_PMIC_USB_CC	0x5C
#define AVS_CMD_SET_PMIC_GPO_CFG	0x5D
#define AVS_CMD_GET_PMIC_GPO_STATUS	0x5E
#define AVS_CMD_GET_PMIC_ALL_REGS	0x5F
#define AVS_CMD_ACCESS_RAW_I2C		0x60
#define AVS_CMD_DEBUG_ACCESS	0x61

/* AVS function return status definitions */
#define AVS_STATUS_CLEAR	0x00
#define AVS_STATUS_SUCCESS	0xf0
#define AVS_STATUS_FAILURE	0xff
#define AVS_STATUS_INVALID	0xf1
#define AVS_STATUS_NO_SUPP	0xf2
#define AVS_STATUS_NO_MAP	0xf3
#define AVS_STATUS_MAP_SET	0xf4

#define AVS_MAX_PARAMS		0x0c

static inline
int brcmstb_issue_avs_command(struct platform_device *pdev, unsigned int cmd,
			      unsigned int num_in, unsigned int num_out,
			      u32 args[])
{
	return -ENODEV;
}

struct brcmstb_avs_pmic_ext_info {
	__u8	i2c_addr;
	/* A non-zero chip_id means that information is valid */
	__u8	chip_id;
#define AVS_PMIC_DIE		(1 << 0)
#define AVS_PMIC_EXT_THERM	(1 << 1)
#define AVS_PMIC_OTP_SHIFT	2
#define AVS_PMIC_OTP_MASK	0x3
	__u8	caps;
};

struct brcmstb_avs_pmic_ext_info2 {
	__u8	version;
	__u8	variant;
	__u8	gid;
	__u8	esd;
};

struct brcmstb_avs_pmic_info {
	__u8	num_pmic_devices;
	__u8	num_regulators;
	__u8	num_gpios;
	__u8	reserved;
	struct brcmstb_avs_pmic_ext_info ext_infos[4];
	__u8	otp_strapping;
	struct brcmstb_avs_pmic_ext_info2 ext_infos2[4];
};

#endif /* _BRCMSTB_AVS_DVFS_H */
