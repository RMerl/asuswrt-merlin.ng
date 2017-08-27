/* Integration tests for the eavesdrop=true|false keyword in DBusMatchRule
 *
 * Author: Cosimo Alfarano <cosimo.alfarano@collabora.co.uk>
 * Based on: tests/dbus-daemon.c by Simon McVittie
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
#endif

#include "test-utils.h"

#define SENDER_NAME "test.eavesdrop.sender"
#define SENDER_PATH "/test/eavesdrop/sender"
#define SENDER_IFACE SENDER_NAME
#define SENDER_SIGNAL_NAME "Signal"
#define SENDER_STOPPER_NAME "Stopper"

/* This rule is equivalent to the one added to a proxy connecting to
 * SENDER_NAME+SENDER_IFACE, plus restricting on signal name.
 * Being more restrictive, if the connection receives what we need, for sure
 * the original proxy rule will match it */
#define RECEIVER_RULE "sender='" SENDER_NAME "'," \
  "interface='" SENDER_IFACE "'," \
  "type='signal'," \
  "member='" SENDER_SIGNAL_NAME "'"
#define POLITELISTENER_RULE RECEIVER_RULE
#define EAVESDROPPER_RULE RECEIVER_RULE ",eavesdrop=true"

#define STOPPER_RULE "sender='" SENDER_NAME \
  "',interface='" SENDER_IFACE "',type='signal',member='" SENDER_STOPPER_NAME "'"

/* a connection received a signal to whom? */
typedef enum {
  NONE_YET = 0,
  TO_ME,
  TO_OTHER,
  BROADCAST,
} SignalDst;

typedef struct {
    TestMainContext *ctx;
    DBusError e;
    GError *ge;

    GPid daemon_pid;

    /* eavedrop keyword tests */
    DBusConnection *sender;
    DBusConnection *receiver;
    SignalDst receiver_dst;
    dbus_bool_t receiver_got_stopper;
    DBusConnection *eavesdropper;
    SignalDst eavesdropper_dst;
    dbus_bool_t eavesdropper_got_stopper;
    DBusConnection *politelistener;
    SignalDst politelistener_dst;
    dbus_bool_t politelistener_got_stopper;
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

/* send a unicast signal to <self> to ensure that no other connection
 * listening is the actual recipient for the signal */
static DBusHandlerResult
sender_send_unicast_to_sender (Fixture *f)
{
  DBusMessage *signal;

  signal = dbus_message_new_signal (SENDER_PATH, SENDER_IFACE,
      SENDER_SIGNAL_NAME);
  dbus_message_set_destination (signal, dbus_bus_get_unique_name (f->sender));

  if (signal == NULL)
    g_error ("OOM");

  if (!dbus_connection_send (f->sender, signal, NULL))
    g_error ("OOM");

  dbus_message_unref (signal);

  return DBUS_HANDLER_RESULT_HANDLED;
}

/* send a unicast signal to <receiver>, making <politelistener> and
 * <eavesdropper> not a actual recipient for it */
static DBusHandlerResult
sender_send_unicast_to_receiver (Fixture *f)
{
  DBusMessage *signal;

  signal = dbus_message_new_signal (SENDER_PATH, SENDER_IFACE, SENDER_SIGNAL_NAME);
  dbus_message_set_destination (signal, dbus_bus_get_unique_name (f->receiver));

  if (signal == NULL)
    g_error ("OOM");

  if (!dbus_connection_send (f->sender, signal, NULL))
    g_error ("OOM");

  dbus_message_unref (signal);

  return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
sender_send_broadcast (Fixture *f)
{
  DBusMessage *signal;

  signal = dbus_message_new_signal (SENDER_PATH, SENDER_IFACE, SENDER_SIGNAL_NAME);
  dbus_message_set_destination (signal, NULL);

  if (signal == NULL)
    g_error ("OOM");

  if (!dbus_connection_send (f->sender, signal, NULL))
    g_error ("OOM");

  dbus_message_unref (signal);

  return DBUS_HANDLER_RESULT_HANDLED;
}

/* Send special broadcast signal to indicate that the connections can "stop"
 * listening and check their results.
 * DBus does not re-order messages, so when the three connections have received
 * this signal, we are sure that any message sent before it has also been
 * dispatched. */
static DBusHandlerResult
sender_send_stopper (Fixture *f)
{
  DBusMessage *signal;

  signal = dbus_message_new_signal (SENDER_PATH, SENDER_IFACE, SENDER_STOPPER_NAME);
  dbus_message_set_destination (signal, NULL);

  if (signal == NULL)
    g_error ("OOM");

  if (!dbus_connection_send (f->sender, signal, NULL))
    g_error ("OOM");

  dbus_message_unref (signal);

  return DBUS_HANDLER_RESULT_HANDLED;
}

/* Ignore NameAcquired, then depending on the signal received:
 * - updates f-><conn>_dst based on the destination of the message
 * - asserts that <conn> received the stop signal
 */
static DBusHandlerResult
signal_filter (DBusConnection *connection,
    DBusMessage *message,
    void *user_data)
{
  Fixture *f = user_data;
  SignalDst *dst = NULL;
  DBusConnection **conn;
  dbus_bool_t *got_stopper;

  if (0 == strcmp (dbus_message_get_member (message), "NameAcquired"))
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

  if (connection == f->receiver)
    {
      dst = &(f->receiver_dst);
      conn = &(f->receiver);
      got_stopper = &(f->receiver_got_stopper);
    }
  else if (connection == f->eavesdropper)
    {
      dst = &(f->eavesdropper_dst);
      conn = &(f->eavesdropper);
      got_stopper = &(f->eavesdropper_got_stopper);
    }
  else if (connection == f->politelistener)
    {
      dst = &(f->politelistener_dst);
      conn = &(f->politelistener);
      got_stopper = &(f->politelistener_got_stopper);
    }
  else
    {
      g_error ("connection not matching");
    }

  if (0 == strcmp (dbus_message_get_member (message), SENDER_SIGNAL_NAME))
    {
      if (dbus_message_get_destination (message) == NULL)
        *dst = BROADCAST;
      else if (0 == strcmp (dbus_message_get_destination (message), dbus_bus_get_unique_name (*conn)))
        *dst = TO_ME;
      else /* if (dbus_message_get_destination (message) != NULL) */
        *dst = TO_OTHER;
    }
  else if (0 == strcmp (dbus_message_get_member (message), SENDER_STOPPER_NAME))
    {
      *got_stopper = TRUE;
    }
  else
    {
      g_error ("got unknown member from message: %s",
          dbus_message_get_member (message));
    }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
add_receiver_filter (Fixture *f)
{
  DBusError e = DBUS_ERROR_INIT;

  dbus_bus_add_match (f->receiver, RECEIVER_RULE, &e);
  assert_no_error (&e);
  dbus_bus_add_match (f->receiver, STOPPER_RULE, &e);
  assert_no_error (&e);

  if (!dbus_connection_add_filter (f->receiver,
        signal_filter, f, NULL))
    g_error ("OOM");
}

static void
add_eavesdropper_filter (Fixture *f)
{
  DBusError e = DBUS_ERROR_INIT;

  dbus_bus_add_match (f->eavesdropper, EAVESDROPPER_RULE, &e);
  assert_no_error (&e);
  dbus_bus_add_match (f->eavesdropper, STOPPER_RULE, &e);
  assert_no_error (&e);

  if (!dbus_connection_add_filter (f->eavesdropper,
        signal_filter, f, NULL))
    g_error ("OOM");
}

static void
add_politelistener_filter (Fixture *f)
{
  DBusError e = DBUS_ERROR_INIT;

  dbus_bus_add_match (f->politelistener, POLITELISTENER_RULE, &e);
  assert_no_error (&e);
  dbus_bus_add_match (f->politelistener, STOPPER_RULE, &e);
  assert_no_error (&e);

  if (!dbus_connection_add_filter (f->politelistener,
        signal_filter, f, NULL))
    g_error ("OOM");
}

static void
setup (Fixture *f,
    gconstpointer context G_GNUC_UNUSED)
{
  gchar *dbus_daemon;
  gchar *config;
  gchar *address;

  f->ctx = test_main_context_get ();

  f->ge = NULL;
  dbus_error_init (&f->e);

  dbus_daemon = g_strdup (g_getenv ("DBUS_TEST_DAEMON"));

  if (dbus_daemon == NULL)
    dbus_daemon = g_strdup ("dbus-daemon");

  if (g_getenv ("DBUS_TEST_SYSCONFDIR") != NULL)
    {
      config = g_strdup_printf ("--config-file=%s/dbus-1/session.conf",
          g_getenv ("DBUS_TEST_SYSCONFDIR"));
    }
  else if (g_getenv ("DBUS_TEST_DATA") != NULL)
    {
      config = g_strdup_printf (
          "--config-file=%s/valid-config-files/session.conf",
          g_getenv ("DBUS_TEST_DATA"));
    }
  else
    {
      config = g_strdup ("--session");
    }

  if (g_getenv ("DBUS_TEST_DAEMON_ADDRESS") != NULL)
    {
      address = g_strdup (g_getenv ("DBUS_TEST_DAEMON_ADDRESS"));
    }
  else
    {
      address = spawn_dbus_daemon (dbus_daemon, config, &f->daemon_pid);
    }

  g_free (dbus_daemon);
  g_free (config);

  f->sender = connect_to_bus (f, address);
  dbus_bus_request_name (f->sender, SENDER_NAME, DBUS_NAME_FLAG_DO_NOT_QUEUE,
      &(f->e));
  f->receiver = connect_to_bus (f, address);
  f->eavesdropper = connect_to_bus (f, address);
  f->politelistener = connect_to_bus (f, address);
  add_receiver_filter (f);
  add_politelistener_filter (f);
  add_eavesdropper_filter (f);

  g_free (address);
}

static void
test_eavesdrop_broadcast (Fixture *f,
    gconstpointer context G_GNUC_UNUSED)
{
  sender_send_broadcast (f);
  sender_send_stopper (f);

  while (!f->receiver_got_stopper ||
      !f->politelistener_got_stopper ||
      !f->eavesdropper_got_stopper)
    test_main_context_iterate (f->ctx, TRUE);

  /* all the three connection can receive a broadcast */
  g_assert_cmpint (f->receiver_dst, ==, BROADCAST);
  g_assert_cmpint (f->politelistener_dst, ==, BROADCAST);
  g_assert_cmpint (f->eavesdropper_dst, ==, BROADCAST);
}

/* a way to say that none of the listening connection are destination of the
 * signal */
static void
test_eavesdrop_unicast_to_sender (Fixture *f,
    gconstpointer context G_GNUC_UNUSED)
{
  sender_send_unicast_to_sender (f);
  sender_send_stopper (f);

  while (!f->receiver_got_stopper ||
      !f->politelistener_got_stopper ||
      !f->eavesdropper_got_stopper)
    test_main_context_iterate (f->ctx, TRUE);

  /* not directed to it and not broadcasted, they cannot receive it */
  g_assert_cmpint (f->receiver_dst, ==, NONE_YET);
  g_assert_cmpint (f->politelistener_dst, ==, NONE_YET);
  /* eavesdrop=true, it will receive the signal even though it's not directed
   * to it */
  g_assert_cmpint (f->eavesdropper_dst, ==, TO_OTHER);
}

static void
test_eavesdrop_unicast_to_receiver (Fixture *f,
    gconstpointer context G_GNUC_UNUSED)
{
  sender_send_unicast_to_receiver (f);
  sender_send_stopper (f);

  while (!f->receiver_got_stopper ||
      !f->politelistener_got_stopper ||
      !f->eavesdropper_got_stopper)
    test_main_context_iterate (f->ctx, TRUE);

  /* direct to him */
  g_assert_cmpint (f->receiver_dst, ==, TO_ME);
  /* not directed to it and not broadcasted, it cannot receive it */
  g_assert_cmpint (f->politelistener_dst, ==, NONE_YET);
  /* eavesdrop=true, it will receive the signal even though it's not directed
   * to it */
  g_assert_cmpint (f->eavesdropper_dst, ==, TO_OTHER);
}

static void
teardown (Fixture *f,
    gconstpointer context G_GNUC_UNUSED)
{
  dbus_error_free (&f->e);
  g_clear_error (&f->ge);

  if (f->sender != NULL)
    {
      dbus_connection_close (f->sender);
      dbus_connection_unref (f->sender);
      f->sender = NULL;
    }

  if (f->receiver != NULL)
    {
      dbus_connection_remove_filter (f->receiver,
          signal_filter, f);

      dbus_connection_close (f->receiver);
      dbus_connection_unref (f->receiver);
      f->receiver = NULL;
    }

  if (f->politelistener != NULL)
    {
      dbus_connection_remove_filter (f->politelistener,
          signal_filter, f);

      dbus_connection_close (f->politelistener);
      dbus_connection_unref (f->politelistener);
      f->politelistener = NULL;
    }

  if (f->eavesdropper != NULL)
    {
      dbus_connection_remove_filter (f->eavesdropper,
          signal_filter, f);

      dbus_connection_close (f->eavesdropper);
      dbus_connection_unref (f->eavesdropper);
      f->eavesdropper = NULL;
    }

#ifdef DBUS_WIN
  TerminateProcess (f->daemon_pid, 1);
#else
  kill (f->daemon_pid, SIGTERM);
#endif

  g_spawn_close_pid (f->daemon_pid);

  test_main_context_unref (f->ctx);
}

int
main (int argc,
    char **argv)
{
  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugs.freedesktop.org/show_bug.cgi?id=");

  g_test_add ("/eavedrop/match_keyword/broadcast", Fixture, NULL,
      setup, test_eavesdrop_broadcast, teardown);
  g_test_add ("/eavedrop/match_keyword/unicast_to_receiver", Fixture, NULL,
      setup, test_eavesdrop_unicast_to_receiver,
      teardown);
  g_test_add ("/eavedrop/match_keyword/unicast_to_sender", Fixture, NULL,
      setup, test_eavesdrop_unicast_to_sender, teardown);

  return g_test_run ();
}
