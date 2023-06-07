// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  OBEX library with GLib integration
 *
 *  Copyright (C) 2011  Intel Corporation. All rights reserved.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>

#include "gobex-defs.h"

GQuark g_obex_error_quark(void)
{
	return g_quark_from_static_string("g-obex-error-quark");
}
