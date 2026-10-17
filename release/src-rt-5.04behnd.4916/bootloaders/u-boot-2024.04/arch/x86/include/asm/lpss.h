/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2019 Google LLC
 */

#ifndef __ASM_LPSS_H
#define __ASM_LPSS_H

struct udevice;

/* D0 and D3 enable config */
enum lpss_pwr_state {
	STATE_D0 = 0,
	STATE_D3 = 3
};

/**
 * lpss_reset_release() - Release device from reset
 *
 * This is used for devices which have LPSS support.
 *
 * @regs: Pointer to device registers
 */
void lpss_reset_release(void *regs);

/**
 * lpss_set_power_state() - Change power state of a device
 *
 * This is used for devices which have LPSS support.
 *
 * @dev: Device to update
 * @state: New power state to set
 */
void lpss_set_power_state(struct udevice *dev, enum lpss_pwr_state state);

#endif
