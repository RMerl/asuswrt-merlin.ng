/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 */

#ifndef _X86_CHROMEBOOK_H
#define _X86_CHROMEBOOK_H

#define CFG_X86_MRC_ADDR			0xfffa0000
#define CFG_X86_REFCODE_ADDR			0xffea0000
#define CFG_X86_REFCODE_RUN_ADDR		0

#define CFG_STD_DEVICES_SETTINGS	"stdin=usbkbd,i8042-kbd,serial\0" \
					"stdout=vidconsole,serial\0" \
					"stderr=vidconsole,serial\0"

#endif
