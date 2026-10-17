/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 NVIDIA Corporation
 */

#ifndef __POWER_AS3722_H__
#define __POWER_AS3722_H__

struct udevice;

#define AS3722_GPIO_OUTPUT_VDDH (1 << 0)
#define AS3722_GPIO_INVERT (1 << 1)

#define AS3722_DEVICE_ID 0x0c
#define AS3722_SD_VOLTAGE(n) (0x00 + (n))
#define AS3722_LDO_VOLTAGE(n) (0x10 + (n))
#define AS3722_SD_CONTROL 0x4d
#define AS3722_LDO_CONTROL0 0x4e
#define AS3722_LDO_CONTROL1 0x4f
#define AS3722_ASIC_ID1 0x90
#define AS3722_ASIC_ID2 0x91

#define AS3722_GPIO_CONTROL(n) (0x08 + (n))
#define AS3722_GPIO_SIGNAL_OUT 0x20
#define AS3722_GPIO_CONTROL_MODE_OUTPUT_VDDH (1 << 0)
#define AS3722_GPIO_CONTROL_MODE_OUTPUT_VDDL (7 << 0)
#define AS3722_GPIO_CONTROL_INVERT (1 << 7)

int as3722_sd_set_voltage(struct udevice *dev, unsigned int sd, u8 value);
int as3722_ldo_set_voltage(struct udevice *dev, unsigned int ldo, u8 value);

#endif /* __POWER_AS3722_H__ */
