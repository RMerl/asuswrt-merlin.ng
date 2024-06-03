/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * thor.h -- USB THOR Downloader protocol
 *
 * Copyright (C) 2013 Samsung Electronics
 * Lukasz Majewski  <l.majewski@samsung.com>
 *
 */

#ifndef __THOR_H_
#define __THOR_H_

#include <linux/usb/composite.h>

int thor_handle(void);
int thor_init(void);
int thor_add(struct usb_configuration *c);
#endif /* __THOR_H_ */
