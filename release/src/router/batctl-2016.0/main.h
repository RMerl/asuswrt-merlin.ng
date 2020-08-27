/*
 * Copyright (C) 2007-2016  B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>, Marek Lindner <mareklindner@neomailbox.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */

#ifndef _BATCTL_MAIN_H
#define _BATCTL_MAIN_H

#include <stdint.h>

#ifndef SOURCE_VERSION
#define SOURCE_VERSION "2016.0"
#endif

#define SOCKET_PATH_FMT "%s/batman_adv/%s/socket"

#define EXIT_NOSUCCESS 2

#define OPT_LONG_MAX_LEN 25
#define OPT_SHORT_MAX_LEN 5

#define DEBUG_TABLE_PATH_MAX_LEN 20
#define SETTINGS_PATH_MAX_LEN 25

#if BYTE_ORDER == BIG_ENDIAN
#define __BIG_ENDIAN_BITFIELD
#elif BYTE_ORDER == LITTLE_ENDIAN
#define __LITTLE_ENDIAN_BITFIELD
#else
#error "unknown endianess"
#endif

#define __packed __attribute((packed))   /* linux kernel compat */
#define BIT(nr)                 (1UL << (nr)) /* linux kernel compat */

typedef uint8_t u8; /* linux kernel compat */
typedef uint16_t u16; /* linux kernel compat */

extern char module_ver_path[];

#ifndef VLAN_VID_MASK
#define VLAN_VID_MASK   0xfff
#endif

#define BATADV_PRINT_VID(vid) (vid & BATADV_VLAN_HAS_TAG ? \
			       (int)(vid & VLAN_VID_MASK) : -1)

#endif
