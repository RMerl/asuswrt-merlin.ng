/* GLib testing framework examples and tests
 *
 * Copyright (C) 2012 Collabora Ltd. <http://www.collabora.co.uk/>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Xavier Claessens <xavier.claessens@collabora.co.uk>
 */

#include "gdbus-sessionbus.h"

static GTestDBus *singleton = NULL;

void
session_bus_up (void)
{
  g_assert (singleton == NULL);
  singleton = g_test_dbus_new (G_TEST_DBUS_NONE);
  g_test_dbus_up (singleton);
}

void
session_bus_stop (void)
{
  g_assert (singleton != NULL);
  g_test_dbus_stop (singleton);
}

void
session_bus_down (void)
{
  g_assert (singleton != NULL);
  g_test_dbus_down (singleton);
  g_clear_object (&singleton);
}

