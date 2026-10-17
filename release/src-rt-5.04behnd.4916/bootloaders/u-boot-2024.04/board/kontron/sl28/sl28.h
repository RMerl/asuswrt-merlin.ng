/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __SL28_H
#define __SL28_H

enum boot_source {
	BOOT_SOURCE_UNKNOWN,
	BOOT_SOURCE_SDHC,
	BOOT_SOURCE_MMC,
	BOOT_SOURCE_I2C,
	BOOT_SOURCE_SPI,
};

enum boot_source sl28_boot_source(void);

#endif
