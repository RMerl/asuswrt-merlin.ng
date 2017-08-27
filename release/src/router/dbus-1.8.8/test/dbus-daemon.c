/* Integration tests for the dbus-daemon
 *
 * Author: Simon McVittie <simon.mcvittie@collabora.co.uk>
 * Copyright © 2010-2011 Nokia Corporation
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

#ifdef DBUS_WIN
# include <io.h>
# include <windows.h>
#else
# include <signal.h>
# include <unistd.h>
# include <sys/types.h>
#endif

#include "test-utils.h"

typedef struct {
    gboolean skip;

    TestMainContext *ctx;

    DBusError e;
    GError *ge;

    GPid daemon_pid;

    DBusConnection *left_conn;

    DBusConnection *right_conn;
    gboolean right_conn_echo;
} Fixture;

#define assert_no_error(e) _assert_no_error (e, __FILE__, __LINE__)
static void
_assert_no_error (const DBusError *e,
    const char *file,
    int line)
{
  if (G_UNLIKELY (dbus_error_is_set (e)))
    g_error ("%s:%d: expected success but got error: %s: %s",
        file, line, e->name, e->message);
}

static gchar *
spawn_dbus_daemon (gchar *binary,
    gchar *configuration,
    GPid *daemon_pid)
{
  GError *error = NULL;
  GString *address;
  gint address_fd;
  gchar *argv[] = {
      binary,
      configuration,
      "--nofork",
      "--print-address=1", /* stdout */
      NULL
  };

  g_spawn_async_with_pipes (NULL, /* working directory */
      argv,
      NULL, /* envp */
      G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH,
      NULL, /* child_setup */
      NULL, /* user data */
      daemon_pid,
      NULL, /* child's stdin = /dev/null */
      &address_fd,
      NULL, /* child's stderr = our stderr */
      &error);
  g_assert_no_error (error);

  address = g_string_new (NULL);

  /* polling until the dbus-daemon writes out its address is a bit stupid,
   * but at least it's simple, unlike dbus-launch... in principle we could
   * use select() here, but life's too short */
  while (1)
    {
      gssize bytes;
      gchar buf[4096];
      gchar *newline;

      bytes = read (address_fd, buf, sizeof (buf));

      if (bytes > 0)
        g_string_append_len (address, buf, bytes);

      newline = strchr (address->str, '\n');

      if (newline != NULL)
        {
          if ((newline > address->str) && ('\r' == newline[-1]))
            newline -= 1;
          g_string_truncate (address, newline - address->str);
          break;
        }

      g_usleep (G_USEC_PER_SEC / 10);
    }

  return g_string_free (address, FALSE);
}

static DBusConnection *
connect_to_bus (Fixture *f,
    const gchar *address)
{
  DBusConnection *conn;
  DBusError error = DBUS_ERROR_INIT;
  dbus_bool_t ok;

  conn = dbus_connection_open_private (address, &error);
  assert_no_error (&error);
  g_assert (conn != NULL);

  ok = dbus_bus_register (conn, &error);
  assert_no_error (&error);
  g_assert (ok);
  g_assert (dbus_bus_get_unique_name (conn) != NULL);

  test_connection_setup (f->ctx, conn);
  return conn;
}

static DBusHandlerResult
echo_filter (DBusConnection *connection,
    DBusMessage *message,
    void *user_data)
{
  DBusMessage *reply;

  if (dbus_message_get_type (message) != DBUS_MESSAGE_TYPE_METHOD_CALL)
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

  reply = dbus_message_new_method_return (message);

  if (reply == NULL)
    g_error ("OOM");

  if (!dbus_connection_send (connection, reply, NULL))
    g_error ("OOM");

  dbus_message_unref (reply);

  return DBUS_HANDLER_RESULT_HANDLED;
}

typedef struct {
    const char *bug_ref;
    guint min_messages;
    const char *config_file;
} Config;

static void
setup (Fixture *f,
    gconstpointer context)
{
  const Config *config = context;
  gchar *dbus_daemon;
  gchar *arg;
  gchar *address;

  f->ctx = test_main_context_get ();
  f->ge = NULL;
  dbus_error_init (&f->e);

  if (config != NULL && config->config_file != NULL)
    {
      if (g_getenv ("DBUS_TEST_DAEMON_ADDRESS") != NULL)
        {
          g_message ("SKIP: cannot use DBUS_TEST_DAEMON_ADDRESS for "
              "unusally-configured dbus-daemon");
          f->skip = TRUE;
          return;
        }

      if (g_getenv ("DBUS_TEST_DATA") == NULL)
        {
          g_message ("SKIP: set DBUS_TEST_DATA to a directory containing %s",
              config->config_file);
          f->skip = TRUE;
          return;
        }

      arg = g_strdup_printf (
          "--config-file=%s/%s",
          g_getenv ("DBUS_TEST_DATA"), config->config_file);
    }
  else if (g_getenv ("DBUS_TEST_SYSCONFDIR") != NULL)
    {
      arg = g_strdup_printf ("--config-file=%s/dbus-1/session.conf",
          g_getenv ("DBUS_TEST_SYSCONFDIR"));
    }
  else if (g_getenv ("DBUS_TEST_DATA") != NULL)
    {
      arg = g_strdup_printf (
          "--config-file=%s/valid-config-files/session.conf",
          g_getenv ("DBUS_TEST_DATA"));
    }
  else
    {
      arg = g_strdup ("--session");
    }

  dbus_daemon = g_strdup (g_getenv ("DBUS_TEST_DAEMON"));

  if (dbus_daemon == NULL)
    dbus_daemon = g_strdup ("dbus-daemon");

  if (g_getenv ("DBUS_TEST_DAEMON_ADDRESS") != NULL)
    {
      address = g_strdup (g_getenv ("DBUS_TEST_DAEMON_ADDRESS"));
    }
  else
    {
      address = spawn_dbus_daemon (dbus_daemon, arg, &f->daemon_pid);
    }

  g_free (dbus_daemon);
  g_free (arg);

  f->left_conn = connect_to_bus (f, address);
  f->right_conn = connect_to_bus (f, address);
  g_free (address);
}

static void
add_echo_filter (Fixture *f)
{
  if (!dbus_connection_add_filter (f->right_conn, echo_filter, NULL, NULL))
    g_error ("OOM");

  f->right_conn_echo = TRUE;
}

static void
pc_count (DBusPendingCall *pc,
    void *data)
{
  guint *received_p = data;

  (*received_p)++;
}

static void
test_echo (Fixture *f,
    gconstpointer context)
{
  const Config *config = context;
  guint count = 2000;
  guint sent;
  guint received = 0;
  double elapsed;

  if (f->skip)
    return;

  if (config != NULL && config->bug_ref != NULL)
    g_test_bug (config->bug_ref);

  if (g_test_perf ())
    count = 100000;

  if (config != NULL)
    count = MAX (config->min_messages, count);

  add_echo_filter (f);

  g_test_timer_start ();

  for (sent = 0; sent < count; sent++)
    {
      DBusMessage *m = dbus_message_new_method_call (
          dbus_bus_get_unique_name (f->right_conn), "/",
          "com.example", "Spam");
      DBusPendingCall *pc;

      if (m == NULL)
        g_error ("OOM");

      if (!dbus_connection_send_with_reply (f->left_conn, m, &pc,
                                            DBUS_TIMEOUT_INFINITE) ||
          pc == NULL)
        g_error ("OOM");

      if (dbus_pending_call_get_completed (pc))
        pc_count (pc, &received);
      else if (!dbus_pending_call_set_notify (pc, pc_count, &received,
            NULL))
        g_error ("OOM");

      dbus_pending_call_unref (pc);
      dbus_message_unref (m);
    }

  while (received < count)
    test_main_context_iterate (f->ctx, TRUE);

  elapsed = g_test_timer_elapsed ();

  g_test_maximized_result (count / elapsed, "%u messages / %f seconds",
      count, elapsed);
}

static void
pending_call_store_reply (DBusPendingCall *pc,
    void *data)
{
  DBusMessage **message_p = data;

  *message_p = dbus_pending_call_steal_reply (pc);
  g_assert (*message_p != NULL);
}

static void
test_creds (Fixture *f,
    gconstpointer context)
{
  const char *unique = dbus_bus_get_unique_name (f->left_conn);
  DBusMessage *m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "GetConnectionCredentials");
  DBusPendingCall *pc;
  DBusMessageIter args_iter;
  DBusMessageIter arr_iter;
  DBusMessageIter pair_iter;
  DBusMessageIter var_iter;
  enum {
      SEEN_UNIX_USER = 1,
      SEEN_PID = 2,
      SEEN_WINDOWS_SID = 4
  } seen = 0;

  if (m == NULL)
    g_error ("OOM");

  if (!dbus_message_append_args (m,
        DBUS_TYPE_STRING, &unique,
        DBUS_TYPE_INVALID))
    g_error ("OOM");

  if (!dbus_connection_send_with_reply (f->left_conn, m, &pc,
                                        DBUS_TIMEOUT_USE_DEFAULT) ||
      pc == NULL)
    g_error ("OOM");

  dbus_message_unref (m);
  m = NULL;

  if (dbus_pending_call_get_completed (pc))
    pending_call_store_reply (pc, &m);
  else if (!dbus_pending_call_set_notify (pc, pending_call_store_reply,
                                          &m, NULL))
    g_error ("OOM");

  while (m == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  g_assert_cmpstr (dbus_message_get_signature (m), ==, "a{sv}");

  dbus_message_iter_init (m, &args_iter);
  g_assert_cmpuint (dbus_message_iter_get_arg_type (&args_iter), ==,
      DBUS_TYPE_ARRAY);
  g_assert_cmpuint (dbus_message_iter_get_element_type (&args_iter), ==,
      DBUS_TYPE_DICT_ENTRY);
  dbus_message_iter_recurse (&args_iter, &arr_iter);

  while (dbus_message_iter_get_arg_type (&arr_iter) != DBUS_TYPE_INVALID)
    {
      const char *name;

      dbus_message_iter_recurse (&arr_iter, &pair_iter);
      g_assert_cmpuint (dbus_message_iter_get_arg_type (&pair_iter), ==,
          DBUS_TYPE_STRING);
      dbus_message_iter_get_basic (&pair_iter, &name);
      dbus_message_iter_next (&pair_iter);
      g_assert_cmpuint (dbus_message_iter_get_arg_type (&pair_iter), ==,
          DBUS_TYPE_VARIANT);
      dbus_message_iter_recurse (&pair_iter, &var_iter);

      if (g_strcmp0 (name, "UnixUserID") == 0)
        {
#ifdef G_OS_UNIX
          guint32 u32;

          g_assert (!(seen & SEEN_UNIX_USER));
          g_assert_cmpuint (dbus_message_iter_get_arg_type (&var_iter), ==,
              DBUS_TYPE_UINT32);
          dbus_message_iter_get_basic (&var_iter, &u32);
          g_message ("%s of this process is %u", name, u32);
          g_assert_cmpuint (u32, ==, geteuid ());
          seen |= SEEN_UNIX_USER;
#else
          g_assert_not_reached ();
#endif
        }
      else if (g_strcmp0 (name, "ProcessID") == 0)
        {
          guint32 u32;

          g_assert (!(seen & SEEN_PID));
          g_assert_cmpuint (dbus_message_iter_get_arg_type (&var_iter), ==,
              DBUS_TYPE_UINT32);
          dbus_message_iter_get_basic (&var_iter, &u32);
          g_message ("%s of this process is %u", name, u32);
#ifdef G_OS_UNIX
          g_assert_cmpuint (u32, ==, getpid ());
#elif defined(G_OS_WIN32)
          g_assert_cmpuint (u32, ==, GetCurrentProcessId ());
#else
          g_assert_not_reached ();
#endif
          seen |= SEEN_PID;
        }

      dbus_message_iter_next (&arr_iter);
    }

#ifdef G_OS_UNIX
  g_assert (seen & SEEN_UNIX_USER);
  g_assert (seen & SEEN_PID);
#endif

#ifdef G_OS_WIN32
  /* FIXME: when implemented:
  g_assert (seen & SEEN_WINDOWS_SID);
   */
#endif
}

static void
teardown (Fixture *f,
    gconstpointer context G_GNUC_UNUSED)
{
  dbus_error_free (&f->e);
  g_clear_error (&f->ge);

  if (f->left_conn != NULL)
    {
      dbus_connection_close (f->left_conn);
      dbus_connection_unref (f->left_conn);
      f->left_conn = NULL;
    }

  if (f->right_conn != NULL)
    {
      if (f->right_conn_echo)
        {
          dbus_connection_remove_filter (f->right_conn, echo_filter, NULL);
          f->right_conn_echo = FALSE;
        }

      dbus_connection_close (f->right_conn);
      dbus_connection_unref (f->right_conn);
      f->right_conn = NULL;
    }

  if (f->daemon_pid != 0)
    {
#ifdef DBUS_WIN
      TerminateProcess (f->daemon_pid, 1);
#else
      kill (f->daemon_pid, SIGTERM);
#endif

      g_spawn_close_pid (f->daemon_pid);
      f->daemon_pid = 0;
    }

  test_main_context_unref (f->ctx);
}

static Config limited_config = {
    "34393", 10000, "valid-config-files/incoming-limit.conf"
};

int
main (int argc,
    char **argv)
{
  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugs.freedesktop.org/show_bug.cgi?id=");

  g_test_add ("/echo/session", Fixture, NULL, setup, test_echo, teardown);
  g_test_add ("/echo/limited", Fixture, &limited_config,
      setup, test_echo, teardown);
  g_test_add ("/creds", Fixture, NULL, setup, test_creds, teardown);

  return g_test_run ();
}
