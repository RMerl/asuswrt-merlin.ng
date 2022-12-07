/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
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
