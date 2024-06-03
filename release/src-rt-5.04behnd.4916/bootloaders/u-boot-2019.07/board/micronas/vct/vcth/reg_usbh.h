/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Stefan Roese <sr@denx.de>, DENX Software Engineering
 */

#define USBH_BASE		0x00080000

/* Relative offsets of the register adresses */

#define USBH_CAPLENGTH_OFFS	0x00000100
#define USBH_CAPLENGTH(base)	((base) + USBH_CAPLENGTH_OFFS)
#define USBH_USBCMD_OFFS	0x00000140
#define USBH_USBCMD(base)	((base) + USBH_USBCMD_OFFS)
#define USBH_BURSTSIZE_OFFS	0x00000160
#define USBH_BURSTSIZE(base)	((base) + USBH_BURSTSIZE_OFFS)
#define USBH_USBMODE_OFFS	0x000001A8
#define USBH_USBMODE(base)	((base) + USBH_USBMODE_OFFS)
#define USBH_USBHMISC_OFFS	0x00000200
#define USBH_USBHMISC(base)	((base) + USBH_USBHMISC_OFFS)
