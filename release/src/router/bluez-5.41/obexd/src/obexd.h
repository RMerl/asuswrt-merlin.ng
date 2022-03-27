/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
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

#define OBEX_OPP	(1 << 1)
#define OBEX_FTP	(1 << 2)
#define OBEX_BIP	(1 << 3)
#define OBEX_PBAP	(1 << 4)
#define OBEX_IRMC	(1 << 5)
#define OBEX_PCSUITE	(1 << 6)
#define OBEX_SYNCEVOLUTION	(1 << 7)
#define OBEX_MAS	(1 << 8)
#define OBEX_MNS	(1 << 9)

gboolean plugin_init(const char *pattern, const char *exclude);
void plugin_cleanup(void);

gboolean manager_init(void);
void manager_cleanup(void);

gboolean obex_option_auto_accept(void);
const char *obex_option_root_folder(void);
gboolean obex_option_symlinks(void);
const char *obex_option_capability(void);
