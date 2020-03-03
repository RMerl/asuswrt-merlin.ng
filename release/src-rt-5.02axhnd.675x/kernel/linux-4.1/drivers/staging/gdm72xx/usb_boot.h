/*
 * Copyright (c) 2012 GCT Semiconductor, Inc. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __GDM72XX_USB_BOOT_H__
#define __GDM72XX_USB_BOOT_H__

struct usb_device;

int usb_boot(struct usb_device *usbdev, u16 pid);
int usb_emergency(struct usb_device *usbdev);

#endif /* __GDM72XX_USB_BOOT_H__ */
