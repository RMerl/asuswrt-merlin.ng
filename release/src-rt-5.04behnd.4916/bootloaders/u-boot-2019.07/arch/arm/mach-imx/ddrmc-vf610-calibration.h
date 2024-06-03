/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ddrmc DDR3 calibration code for NXP's VF610
 *
 * Copyright (C) 2018 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 */

#ifndef __DDRMC_VF610_CALIBRATOIN_H_
#define __DDRMC_VF610_CALIBRATOIN_H_

/*
 * Number of "samples" in the calibration bitmap
 * to be considered during calibration.
 */
#define N_SAMPLES 3

/*
 * Constants to indicate if we are looking for a rising or
 * falling edge in the calibration bitmap
 */
enum edge {
	FALLING_EDGE = 1,
	RISING_EDGE
};

/*
 * The max number of delay elements when DQS to DQ setting
 */
#define DDRMC_DQS_DQ_MAX_DELAY 0xFF

/**
 * ddrmc_calibration - Vybrid's (VF610) DDR3 calibration code
 *
 * This function is calculating proper memory controller values
 * during run time.
 *
 * @param ddrmr_regs - memory controller registers
 *
 * @return 0 on success, otherwise error code
 */
int ddrmc_calibration(struct ddrmr_regs *ddrmr);

#endif /* __DDRMC_VF610_CALIBRATOIN_H_ */
