/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright © 2018 Endless Mobile, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  - Philip Withnall <withnall@endlessm.com>
 */

#include "config.h"

#include "gnotificationbackend.h"

#include "giomodule-priv.h"
#include "gnotification-private.h"

#define G_TYPE_WIN32_NOTIFICATION_BACKEND  (g_win32_notification_backend_get_type ())
#define G_WIN32_NOTIFICATION_BACKEND(o)    (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_WIN32_NOTIFICATION_BACKEND, GWin32NotificationBackend))

typedef struct _GWin32NotificationBackend GWin32NotificationBackend;
typedef GNotificationBackendClass         GWin32NotificationBackendClass;

struct _GWin32NotificationBackend
{
  GNotificationBackend parent;
};

GType g_win32_notification_backend_get_type (void);

G_DEFINE_TYPE_WITH_CODE (GWin32NotificationBackend, g_win32_notification_backend, G_TYPE_NOTIFICATION_BACKEND,
  _g_io_modules_ensure_extension_points_registered ();
  g_io_extension_point_implement (G_NOTIFICATION_BACKEND_EXTENSION_POINT_NAME,
                                  g_define_type_id, "win32", 0))

static gboolean
g_win32_notification_backend_is_supported (void)
{
  /* This is the only backend supported on Windows, and always needs to be
   * present to avoid no backend being selected. */
  return TRUE;
}

static void
g_win32_notification_backend_send_notification (GNotificationBackend *backend,
                                                const gchar          *id,
                                                GNotification        *notification)
{
  static gsize warned = 0;

  /* FIXME: See https://bugzilla.gnome.org/show_bug.cgi?id=776583. This backend
   * exists purely to stop crashes when applications use g_notification*()
   * on Windows, by providing a dummy backend implementation. (The alternative
   * was to modify all of the backend call sites in g_notification*(), which
   * seemed less scalable.) */
  if (g_once_init_enter (&warned))
    {
      g_warning ("Notifications are not yet supported on Windows.");
      g_once_init_leave (&warned, 1);
    }
}

static void
g_win32_notification_backend_withdraw_notification (GNotificationBackend *backend,
                                                    const gchar          *id)
{
  /* FIXME: Nothing needs doing here until send_notification() is implemented. */
}

static void
g_win32_notification_backend_init (GWin32NotificationBackend *backend)
{
}

static void
g_win32_notification_backend_class_init (GWin32NotificationBackendClass *class)
{
  GNotificationBackendClass *backend_class = G_NOTIFICATION_BACKEND_CLASS (class);

  backend_class->is_supported = g_win32_notification_backend_is_supported;
  backend_class->send_notification = g_win32_notification_backend_send_notification;
  backend_class->withdraw_notification = g_win32_notification_backend_withdraw_notification;
}
