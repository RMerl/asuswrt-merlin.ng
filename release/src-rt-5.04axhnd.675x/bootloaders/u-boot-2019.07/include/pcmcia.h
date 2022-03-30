/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _PCMCIA_H
#define _PCMCIA_H

#include <common.h>
#include <config.h>

/*
 * Allow configuration to select PCMCIA slot,
 * or try to generate a useful default
 */
#if defined(CONFIG_CMD_PCMCIA)

#if !defined(CONFIG_PCMCIA_SLOT_A) && !defined(CONFIG_PCMCIA_SLOT_B)
# error "PCMCIA Slot not configured"
#endif /* !defined(CONFIG_PCMCIA_SLOT_A) && !defined(CONFIG_PCMCIA_SLOT_B) */

/* Make sure exactly one slot is defined - we support only one for now */
#if !defined(CONFIG_PCMCIA_SLOT_A) && !defined(CONFIG_PCMCIA_SLOT_B)
#error Neither CONFIG_PCMCIA_SLOT_A nor CONFIG_PCMCIA_SLOT_B configured
#endif
#if defined(CONFIG_PCMCIA_SLOT_A) && defined(CONFIG_PCMCIA_SLOT_B)
#error Both CONFIG_PCMCIA_SLOT_A and CONFIG_PCMCIA_SLOT_B configured
#endif

#ifndef PCMCIA_SOCKETS_NO
#define PCMCIA_SOCKETS_NO	1
#endif
#ifndef PCMCIA_MEM_WIN_NO
#define PCMCIA_MEM_WIN_NO	4
#endif
#define PCMCIA_IO_WIN_NO	2

/* define _slot_ to be able to optimize macros */
#ifdef CONFIG_PCMCIA_SLOT_A
# define _slot_			0
# define PCMCIA_SLOT_MSG	"slot A"
# define PCMCIA_SLOT_x		PCMCIA_PSLOT_A
#else
# define _slot_			1
# define PCMCIA_SLOT_MSG	"slot B"
# define PCMCIA_SLOT_x		PCMCIA_PSLOT_B
#endif

/*
 * This structure is used to address each window in the PCMCIA controller.
 *
 * Keep in mind that we assume that pcmcia_win_t[n+1] is mapped directly
 * after pcmcia_win_t[n]...
 */

typedef struct {
	ulong	br;
	ulong	or;
} pcmcia_win_t;

/**********************************************************************/

/*
 * CIS Tupel codes
 */
#define CISTPL_NULL		0x00
#define CISTPL_DEVICE		0x01
#define CISTPL_LONGLINK_CB	0x02
#define CISTPL_INDIRECT		0x03
#define CISTPL_CONFIG_CB	0x04
#define CISTPL_CFTABLE_ENTRY_CB 0x05
#define CISTPL_LONGLINK_MFC	0x06
#define CISTPL_BAR		0x07
#define CISTPL_PWR_MGMNT	0x08
#define CISTPL_EXTDEVICE	0x09
#define CISTPL_CHECKSUM		0x10
#define CISTPL_LONGLINK_A	0x11
#define CISTPL_LONGLINK_C	0x12
#define CISTPL_LINKTARGET	0x13
#define CISTPL_NO_LINK		0x14
#define CISTPL_VERS_1		0x15
#define CISTPL_ALTSTR		0x16
#define CISTPL_DEVICE_A		0x17
#define CISTPL_JEDEC_C		0x18
#define CISTPL_JEDEC_A		0x19
#define CISTPL_CONFIG		0x1a
#define CISTPL_CFTABLE_ENTRY	0x1b
#define CISTPL_DEVICE_OC	0x1c
#define CISTPL_DEVICE_OA	0x1d
#define CISTPL_DEVICE_GEO	0x1e
#define CISTPL_DEVICE_GEO_A	0x1f
#define CISTPL_MANFID		0x20
#define CISTPL_FUNCID		0x21
#define CISTPL_FUNCE		0x22
#define CISTPL_SWIL		0x23
#define CISTPL_END		0xff

/*
 * CIS Function ID codes
 */
#define CISTPL_FUNCID_MULTI	0x00
#define CISTPL_FUNCID_MEMORY	0x01
#define CISTPL_FUNCID_SERIAL	0x02
#define CISTPL_FUNCID_PARALLEL	0x03
#define CISTPL_FUNCID_FIXED	0x04
#define CISTPL_FUNCID_VIDEO	0x05
#define CISTPL_FUNCID_NETWORK	0x06
#define CISTPL_FUNCID_AIMS	0x07
#define CISTPL_FUNCID_SCSI	0x08

/*
 * Fixed Disk FUNCE codes
 */
#define CISTPL_IDE_INTERFACE	0x01

#define CISTPL_FUNCE_IDE_IFACE	0x01
#define CISTPL_FUNCE_IDE_MASTER	0x02
#define CISTPL_FUNCE_IDE_SLAVE	0x03

/* First feature byte */
#define CISTPL_IDE_SILICON	0x04
#define CISTPL_IDE_UNIQUE	0x08
#define CISTPL_IDE_DUAL		0x10

/* Second feature byte */
#define CISTPL_IDE_HAS_SLEEP	0x01
#define CISTPL_IDE_HAS_STANDBY	0x02
#define CISTPL_IDE_HAS_IDLE	0x04
#define CISTPL_IDE_LOW_POWER	0x08
#define CISTPL_IDE_REG_INHIBIT	0x10
#define CISTPL_IDE_HAS_INDEX	0x20
#define CISTPL_IDE_IOIS16	0x40

#endif

#endif /* _PCMCIA_H */
