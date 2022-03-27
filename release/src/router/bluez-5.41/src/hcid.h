/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

typedef enum {
	BT_MODE_DUAL,
	BT_MODE_BREDR,
	BT_MODE_LE,
} bt_mode_t;

struct main_opts {
	char		*name;
	uint32_t	class;
	uint16_t	autoto;
	uint32_t	pairto;
	uint32_t	discovto;
	gboolean	reverse_sdp;
	gboolean	name_resolv;
	gboolean	debug_keys;
	gboolean	fast_conn;

	uint16_t	did_source;
	uint16_t	did_vendor;
	uint16_t	did_product;
	uint16_t	did_version;

	bt_mode_t	mode;
};

extern struct main_opts main_opts;

gboolean plugin_init(const char *enable, const char *disable);
void plugin_cleanup(void);

void rfkill_init(void);
void rfkill_exit(void);

GKeyFile *btd_get_main_conf(void);

void btd_exit(void);
