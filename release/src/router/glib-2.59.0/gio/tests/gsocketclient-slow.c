/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2018 Igalia S.L.
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
 * Public License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <gio/gio.h>

static void
on_connected (GObject      *source_object,
              GAsyncResult *result,
              gpointer      user_data)
{
  GSocketConnection *conn;
  GError *error = NULL;

  conn = g_socket_client_connect_to_uri_finish (G_SOCKET_CLIENT (source_object), result, &error);
  g_assert_no_error (error);

  g_object_unref (conn);
  g_main_loop_quit (user_data);
}

static void
test_happy_eyeballs (void)
{
  GSocketClient *client;
  GSocketService *service;
  GError *error = NULL;
  guint16 port;
  GMainLoop *loop;

  loop = g_main_loop_new (NULL, FALSE);

  service = g_socket_service_new ();
  port = g_socket_listener_add_any_inet_port (G_SOCKET_LISTENER (service), NULL, &error);
  g_assert_no_error (error);
  g_socket_service_start (service);

  /* All of the magic here actually happens in slow-connect-preload.c
   * which as you would guess is preloaded. So this is just making a
   * normal connection that happens to take 600ms each time. This will
   * trigger the logic to make multiple parallel connections.
   */
  client = g_socket_client_new ();
  g_socket_client_connect_to_host_async (client, "localhost", port, NULL, on_connected, loop);
  g_main_loop_run (loop);

  g_main_loop_unref (loop);
  g_object_unref (service);
  g_object_unref (client);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/socket-client/happy-eyeballs", test_happy_eyeballs);

  return g_test_run ();
}