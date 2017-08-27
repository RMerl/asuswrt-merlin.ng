/**
* Test to make sure that pending calls succeed when given a default,
* specific and infinite timeout.
**/

#include <config.h>
#include <dbus/dbus.h>
#include <dbus/dbus-sysdeps.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

static void
_method_call (DBusConnection *conn,
	      int             timeout_milliseconds)
{
  DBusPendingCall *pending;
  DBusMessage *method;
  DBusMessage *reply;
  char *echo = "echo";

  /* send the message */
  method = dbus_message_new_method_call ("org.freedesktop.DBus.TestSuiteEchoService",
                                         "/org/freedesktop/TestSuite",
                                         "org.freedesktop.TestSuite",
                                         "DelayEcho");

  dbus_message_append_args (method, DBUS_TYPE_STRING, &echo, NULL);
  dbus_connection_send_with_reply (conn, method, &pending, timeout_milliseconds);
  dbus_message_unref (method);
  
  /* block on the message */
  dbus_pending_call_block (pending);

  /* check the reply only to make sure we
     are not getting errors unrelated
     to the block in poll bug */
  reply = dbus_pending_call_steal_reply (pending);

  if (reply == NULL)
    {
      printf ("Failed: Reply is NULL ***\n");
      exit (1);
    }

  if (dbus_message_get_type (reply) == DBUS_MESSAGE_TYPE_ERROR)
    {
      printf ("Failed: Reply is error: %s ***\n", dbus_message_get_error_name (reply));
      exit (1);
    } 

  dbus_message_unref (reply);
  dbus_pending_call_unref (pending);
}

static void
_run_iteration (DBusConnection *conn)
{
  _method_call (conn, -1);
  _method_call (conn, 10000);
  _method_call (conn, INT_MAX);
}

int
main (int argc, char *argv[])
{
  long start_tv_sec, start_tv_usec;
  long end_tv_sec, end_tv_usec;
  int i;
  DBusMessage *method;
  DBusConnection *conn;
  DBusError error;

  printf ("*** Testing pending call timeouts\n");

  dbus_error_init (&error);

  conn = dbus_bus_get (DBUS_BUS_SESSION, &error);

  /* run 100 times to make sure */
  for (i = 0; i < 100; i++)
    {
      long delta;
      
      _dbus_get_monotonic_time (&start_tv_sec, &start_tv_usec);
      _run_iteration (conn);
      _dbus_get_monotonic_time (&end_tv_sec, &end_tv_usec);

      /* we just care about seconds */
      delta = end_tv_sec - start_tv_sec;
      printf ("Iter %i: %lis\n", i, delta);
    }
 
  method = dbus_message_new_method_call ("org.freedesktop.TestSuiteEchoService",
                                         "/org/freedesktop/TestSuite",
                                         "org.freedesktop.TestSuite",
                                         "Exit");
  dbus_connection_send (conn, method, NULL);
  dbus_message_unref (method);

  printf ("Success ***\n");
  exit (0);
}
