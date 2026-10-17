/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef	_IDE_H
#define _IDE_H

#include <blk.h>

#define IDE_BUS(dev)	(dev / (CONFIG_SYS_IDE_MAXDEVICE / CONFIG_SYS_IDE_MAXBUS))

/**
 * ide_set_reset() - Assert or de-assert reset for the IDE device
 *
 * This is provided by boards which need to reset the device through another
 * means, e.g. a GPIO.
 *
 * @idereset: 1 to assert reset, 0 to de-assert it
 */
void ide_set_reset(int idereset);

#endif /* _IDE_H */
