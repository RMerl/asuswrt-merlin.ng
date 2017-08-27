/* Simple sanity-check for loopback through TCP and Unix sockets.
 *
 * Author: Simon McVittie <simon.mcvittie@collabora.co.uk>
 * Copyright © 2010-2012 Nokia Corporation
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <config.h>

#include <glib.h>

#include <dbus/dbus.h>

#include <string.h>

#include "test-utils.h"

typedef struct {
    TestMainContext *ctx;
    DBusError e;

    DBusServer *server;
    DBusConnection *server_conn;
    /* queue of DBusMessage */
    GQueue server_messages;

    DBusConnection *client_conn;
} Fixture;

static void
assert_no_error (const DBusError *e)
{
  if (G_UNLIKELY (dbus_error_is_set (e)))
    g_error ("expected success but got error: %s: %s", e->name, e->message);
}

static DBusHandlerResult
server_message_cb (DBusConnection *server_conn,
    DBusMessage *message,
    void *data)
{
  Fixture *f = data;

  g_assert (server_conn == f->server_conn);
  g_queue_push_tail (&f->server_messages, dbus_message_ref (message));

  return DBUS_HANDLER_RESULT_HANDLED;
}

static void
new_conn_cb (DBusServer *server,
    DBusConnection *server_conn,
    void *data)
{
  Fixture *f = data;
  dbus_bool_t have_mem;

  g_assert (f->server_conn == NULL);
  f->server_conn = dbus_connection_ref (server_conn);
  test_connection_setup (f->ctx, server_conn);

  have_mem = dbus_connection_add_filter (server_conn,
      server_message_cb, f, NULL);
  g_assert (have_mem);
}

static void
setup (Fixture *f,
    gconstpointer addr)
{
  f->ctx = test_main_context_get ();
  dbus_error_init (&f->e);
  g_queue_init (&f->server_messages);

  f->server = dbus_server_listen (addr, &f->e);
  assert_no_error (&f->e);
  g_assert (f->server != NULL);

  dbus_server_set_new_connection_function (f->server,
      new_conn_cb, f, NULL);
  test_server_setup (f->ctx, f->server);
}

static void
test_connect (Fixture *f,
    gconstpointer addr G_GNUC_UNUSED)
{
  g_assert (f->server_conn == NULL);

  f->client_conn = dbus_connection_open_private (
      dbus_server_get_address (f->server), &f->e);
  assert_no_error (&f->e);
  g_assert (f->client_conn != NULL);
  test_connection_setup (f->ctx, f->client_conn);

  while (f->server_conn == NULL)
    {
      g_print (".");
      test_main_context_iterate (f->ctx, TRUE);
    }
}

static void
test_bad_guid (Fixture *f,
    gconstpointer addr G_GNUC_UNUSED)
{
  DBusMessage *incoming;
  gchar *address = g_strdup (dbus_server_get_address (f->server));
  gchar *guid;

  g_test_bug ("39720");

  g_assert (f->server_conn == NULL);

  g_assert (strstr (address, "guid=") != NULL);
  guid = strstr (address, "guid=");
  g_assert_cmpuint (strlen (guid), >=, 5 + 32);

  /* Change the first char of the guid to something different */
  if (guid[5] == '0')
    guid[5] = 'f';
  else
    guid[5] = '0';

  f->client_conn = dbus_connection_open_private (address, &f->e);
  assert_no_error (&f->e);
  g_assert (f->client_conn != NULL);
  test_connection_setup (f->ctx, f->client_conn);

  while (f->server_conn == NULL)
    {
      g_print (".");
      test_main_context_iterate (f->ctx, TRUE);
    }

  /* We get disconnected */

  while (g_queue_is_empty (&f->server_messages))
    {
      g_print (".");
      test_main_context_iterate (f->ctx, TRUE);
    }

  g_assert_cmpuint (g_queue_get_length (&f->server_messages), ==, 1);

  incoming = g_queue_pop_head (&f->server_messages);

  g_assert (!dbus_message_contains_unix_fds (incoming));
  g_assert_cmpstr (dbus_message_get_destination (incoming), ==, NULL);
  g_assert_cmpstr (dbus_message_get_error_name (incoming), ==, NULL);
  g_assert_cmpstr (dbus_message_get_interface (incoming), ==,
      DBUS_INTERFACE_LOCAL);
  g_assert_cmpstr (dbus_message_get_member (incoming), ==, "Disconnected");
  g_assert_cmpstr (dbus_message_get_sender (incoming), ==, NULL);
  g_assert_cmpstr (dbus_message_get_signature (incoming), ==, "");
  g_assert_cmpstr (dbus_message_get_path (incoming), ==, DBUS_PATH_LOCAL);

  dbus_message_unref (incoming);

  g_free (address);
}

static void
test_message (Fixture *f,
    gconstpointer addr)
{
  dbus_bool_t have_mem;
  dbus_uint32_t serial;
  DBusMessage *outgoing, *incoming;

  test_connect (f, addr);

  outgoing = dbus_message_new_signal ("/com/example/Hello",
      "com.example.Hello", "Greeting");
  g_assert (outgoing != NULL);

  have_mem = dbus_connection_send (f->client_conn, outgoing, &serial);
  g_assert (have_mem);
  g_assert (serial != 0);

  while (g_queue_is_empty (&f->server_messages))
    {
      g_print (".");
      test_main_context_iterate (f->ctx, TRUE);
    }

  g_assert_cmpuint (g_queue_get_length (&f->server_messages), ==, 1);

  incoming = g_queue_pop_head (&f->server_messages);

  g_assert (!dbus_message_contains_unix_fds (incoming));
  g_assert_cmpstr (dbus_message_get_destination (incoming), ==, NULL);
  g_assert_cmpstr (dbus_message_get_error_name (incoming), ==, NULL);
  g_assert_cmpstr (dbus_message_get_interface (incoming), ==,
      "com.example.Hello");
  g_assert_cmpstr (dbus_message_get_member (incoming), ==, "Greeting");
  g_assert_cmpstr (dbus_message_get_sender (incoming), ==, NULL);
  g_assert_cmpstr (dbus_message_get_signature (incoming), ==, "");
  g_assert_cmpstr (dbus_message_get_path (incoming), ==, "/com/example/Hello");
  g_assert_cmpuint (dbus_message_get_serial (incoming), ==, serial);

  dbus_message_unref (incoming);

  dbus_message_unref (outgoing);
}

static void
teardown (Fixture *f,
    gconstpointer addr G_GNUC_UNUSED)
{
  if (f->client_conn != NULL)
    {
      dbus_connection_close (f->client_conn);
      dbus_connection_unref (f->client_conn);
      f->client_conn = NULL;
    }

  if (f->server_conn != NULL)
    {
      dbus_connection_close (f->server_conn);
      dbus_connection_unref (f->server_conn);
      f->server_conn = NULL;
    }

  if (f->server != NULL)
    {
      dbus_server_disconnect (f->server);
      dbus_server_unref (f->server);
      f->server = NULL;
    }

  test_main_context_unref (f->ctx);
}

int
main (int argc,
    char **argv)
{
  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugs.freedesktop.org/show_bug.cgi?id=");

  g_test_add ("/connect/tcp", Fixture, "tcp:host=127.0.0.1", setup,
      test_connect, teardown);
  g_test_add ("/message/tcp", Fixture, "tcp:host=127.0.0.1", setup,
      test_message, teardown);

  g_test_add ("/connect/nonce-tcp", Fixture, "nonce-tcp:host=127.0.0.1", setup,
      test_connect, teardown);
  g_test_add ("/message/nonce-tcp", Fixture, "nonce-tcp:host=127.0.0.1", setup,
      test_message, teardown);

#ifdef DBUS_UNIX
  g_test_add ("/connect/unix", Fixture, "unix:tmpdir=/tmp", setup,
      test_connect, teardown);
  g_test_add ("/message/unix", Fixture, "unix:tmpdir=/tmp", setup,
      test_message, teardown);
#endif

  g_test_add ("/message/bad-guid", Fixture, "tcp:host=127.0.0.1", setup,
      test_bad_guid, teardown);

  return g_test_run ();
}
