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

struct obex_plugin_desc {
	const char *name;
	int (*init) (void);
	void (*exit) (void);
};

#ifdef OBEX_PLUGIN_BUILTIN
#define OBEX_PLUGIN_DEFINE(name, init, exit) \
		struct obex_plugin_desc __obex_builtin_ ## name = { \
			#name, init, exit \
		};
#else
#define OBEX_PLUGIN_DEFINE(name,init,exit) \
		extern struct obex_plugin_desc obex_plugin_desc \
				__attribute__ ((visibility("default"))); \
		struct obex_plugin_desc obex_plugin_desc = { \
			#name, init, exit \
		};
#endif
