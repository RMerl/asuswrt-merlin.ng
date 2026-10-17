/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018-2020 NXP
 *
 */

#ifndef __EMC2305_H_
#define __EMC2305_H_

#define I2C_EMC2305_CONF		0x20
#define I2C_EMC2305_FAN1		0x30
#define I2C_EMC2305_FAN2		0x40
#define I2C_EMC2305_FAN3		0x50
#define I2C_EMC2305_FAN4		0x60
#define I2C_EMC2305_FAN5		0x70

#define NUM_OF_FANS			5

void emc2305_init(int chip_addr);
void set_fan_speed(u8 data, int chip_addr);

#endif  /* __EMC2305_H_ */
