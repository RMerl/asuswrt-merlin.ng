/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2023 Broadcom Ltd.
 */
#ifndef __BCM_FAILSAFE_BOOT_H
#define __BOOT_STATE_H


void bcm_failsafe_boot_check(void);
void bcm_failsafe_boot_clear(void);
void bcm_failsafe_boot_init(void);
uint32_t bcm_failsafe_boot_offset(void);

#endif
