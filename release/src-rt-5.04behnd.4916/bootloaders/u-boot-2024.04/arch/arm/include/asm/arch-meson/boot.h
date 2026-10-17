/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef __MESON_BOOT_H__
#define __MESON_BOOT_H__

#include <linux/types.h>

/* Boot device */
#define BOOT_DEVICE_RESERVED    0
#define BOOT_DEVICE_EMMC        1
#define BOOT_DEVICE_NAND        2
#define BOOT_DEVICE_SPI         3
#define BOOT_DEVICE_SD          4
#define BOOT_DEVICE_USB         5

int meson_get_boot_device(void);

int meson_get_soc_rev(char *buff, size_t buff_len);

#endif /* __MESON_BOOT_H__ */
