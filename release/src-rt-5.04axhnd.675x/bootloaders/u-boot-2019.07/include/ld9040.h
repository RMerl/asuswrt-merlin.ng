/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ld9040 AMOLED LCD panel driver.
 *
 * Copyright (C) 2012 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 */

#ifndef __LD9040_H_
#define __LD9040_H_

void ld9040_cfg_ldo(void);
void ld9040_enable_ldo(unsigned int onoff);

#endif /* __LD9040_H_ */
