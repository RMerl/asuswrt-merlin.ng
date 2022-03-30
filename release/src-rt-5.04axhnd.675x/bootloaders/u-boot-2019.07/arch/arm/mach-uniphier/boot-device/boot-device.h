/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef _UNIPHIER_BOOT_DEVICE_H_
#define _UNIPHIER_BOOT_DEVICE_H_

struct uniphier_boot_device {
	unsigned int boot_device;
	const char *desc;
};

extern const struct uniphier_boot_device uniphier_ld4_boot_device_table[];
extern const struct uniphier_boot_device uniphier_pro5_boot_device_table[];
extern const struct uniphier_boot_device uniphier_pxs2_boot_device_table[];
extern const struct uniphier_boot_device uniphier_ld11_boot_device_table[];
extern const struct uniphier_boot_device uniphier_pxs3_boot_device_table[];

extern const unsigned int uniphier_ld4_boot_device_count;
extern const unsigned int uniphier_pro5_boot_device_count;
extern const unsigned int uniphier_pxs2_boot_device_count;
extern const unsigned int uniphier_ld11_boot_device_count;
extern const unsigned int uniphier_pxs3_boot_device_count;

int uniphier_pxs2_boot_device_is_usb(u32 pinmon);
int uniphier_ld11_boot_device_is_usb(u32 pinmon);
int uniphier_ld20_boot_device_is_usb(u32 pinmon);
int uniphier_pxs3_boot_device_is_usb(u32 pinmon);

unsigned int uniphier_pxs2_boot_device_fixup(unsigned int mode);
unsigned int uniphier_ld11_boot_device_fixup(unsigned int mode);

#endif /* _UNIPHIER_BOOT_DEVICE_H_ */
