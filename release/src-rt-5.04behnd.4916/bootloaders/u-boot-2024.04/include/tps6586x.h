/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TPS6586X_H_
#define _TPS6586X_H_

struct udevice;

enum {
	/* SM0-2 PWM/PFM Mode Selection */
	TPS6586X_PWM_SM0	= 1 << 0,
	TPS6586X_PWM_SM1	= 1 << 1,
	TPS6586X_PWM_SM2	= 1 << 2,
};

/**
 * Enable PWM mode for selected SM0-2
 *
 * @param mask	Mask of synchronous converter to enable (TPS6586X_PWM_...)
 * Return: 0 if ok, -1 on error
 */
int tps6586x_set_pwm_mode(int mask);

/**
 * Adjust SM0 and SM1 voltages to the given targets in incremental steps.
 *
 * @param sm0_target	Target voltage for SM0 in 25mW units, 0=725mV, 31=1.5V
 * @param sm1_target	Target voltage for SM1 in 25mW units, 0=725mV, 31=1.5V
 * @param step		Amount to change voltage in each step, in 25mW units
 * @param rate		Slew ratein mV/us: 0=instantly, 1=0.11, 2=0.22,
 *			3=0.44, 4=0.88, 5=1.76, 6=3.52, 7=7.04
 * @param min_sm0_over_sm1	Minimum amount by which sm0 must exceed sm1.
 *			If this condition is not met, no adjustment will be
 *			done and an error will be reported. Use -1 to skip
 *			this check.
 * Return: 0 if ok, -1 on error
 */
int tps6586x_adjust_sm0_sm1(int sm0_target, int sm1_target, int step, int rate,
			    int min_sm0_over_sm1);

/**
 * Set up the TPS6586X I2C bus number. This will be used for all operations
 * on the device. This function must be called before using other functions.
 *
 * @param bus	I2C bus containing the TPS6586X chip
 * Return: 0 (always succeeds)
 */
int tps6586x_init(struct udevice *bus);

#endif	/* _TPS6586X_H_ */
