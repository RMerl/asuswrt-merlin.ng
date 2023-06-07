/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */
#define BLUETOOTH_PLUGIN_PRIORITY_LOW      -100
#define BLUETOOTH_PLUGIN_PRIORITY_DEFAULT     0
#define BLUETOOTH_PLUGIN_PRIORITY_HIGH      100

struct bluetooth_plugin_desc {
	const char *name;
	const char *version;
	int priority;
	int (*init) (void);
	void (*exit) (void);
	void *debug_start;
	void *debug_stop;
};

#ifdef BLUETOOTH_PLUGIN_BUILTIN
#define BLUETOOTH_PLUGIN_DEFINE(name, version, priority, init, exit) \
		struct bluetooth_plugin_desc __bluetooth_builtin_ ## name = { \
			#name, version, priority, init, exit \
		};
#else
#define BLUETOOTH_PLUGIN_DEFINE(name, version, priority, init, exit) \
		extern struct btd_debug_desc __start___debug[] \
				__attribute__ ((weak, visibility("hidden"))); \
		extern struct btd_debug_desc __stop___debug[] \
				__attribute__ ((weak, visibility("hidden"))); \
		extern struct bluetooth_plugin_desc bluetooth_plugin_desc \
				__attribute__ ((visibility("default"))); \
		struct bluetooth_plugin_desc bluetooth_plugin_desc = { \
			#name, version, priority, init, exit, \
			__start___debug, __stop___debug \
		};
#endif
