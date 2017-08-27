/**
 * Test to make sure late thread initialization works
 */

#include <config.h>
#include <dbus/dbus.h>
#include <dbus/dbus-sysdeps.h>
#include <stdio.h>
#include <stdlib.h>

#include <dbus/dbus-internals.h>
#include <dbus/dbus-connection-internal.h>

static void
_run_iteration (DBusConnection *conn)
{
  DBusPendingCall *echo_pending;
  DBusPendingCall *dbus_pending;
  DBusMessage *method;
  DBusMessage *reply;
  char *echo = "echo";

  /* send the first message */
  method = dbus_message_new_method_call ("org.freedesktop.DBus.TestSuiteEchoService",
                                         "/org/freedesktop/TestSuite",
                                         "org.freedesktop.TestSuite",
                                         "Echo");

  dbus_message_append_args (method, DBUS_TYPE_STRING, &echo, NULL);
  dbus_connection_send_with_reply (conn, method, &echo_pending, -1);
  dbus_message_unref (method);
  
  /* send the second message */
  method = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
                                         DBUS_PATH_DBUS,
                                         "org.freedesktop.Introspectable",
                                         "Introspect");

  dbus_connection_send_with_reply (conn, method, &dbus_pending, -1);
  dbus_message_unref (method);

  /* block on the second message (should return immediately) */
  dbus_pending_call_block (dbus_pending);

  /* block on the first message */
  /* if it does not return immediately chances 
     are we hit the block in poll bug */
  dbus_pending_call_block (echo_pending);

  /* check the reply only to make sure we
     are not getting errors unrelated
     to the block in poll bug */
  reply = dbus_pending_call_steal_reply (echo_pending);

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
  dbus_pending_call_unref (dbus_pending);
  dbus_pending_call_unref (echo_pending);
  
}
static void
check_mutex_lock (DBusMutex *mutex1, 
                  DBusMutex *mutex2, 
                  dbus_bool_t is_same)
{
  _dbus_assert (mutex1 != NULL);
  _dbus_assert (mutex2 != NULL);
  
  if (is_same)
    {
      _dbus_assert (mutex1 == mutex2);
    }
  else
    {
      _dbus_assert (mutex1 != mutex2);
    }
}

static void
check_condvar_lock (DBusCondVar *condvar1,  
                    DBusCondVar *condvar2,   
                    dbus_bool_t is_same)
{
  _dbus_assert (condvar1 != NULL);
  _dbus_assert (condvar2 != NULL);

  if (is_same)
    {
      _dbus_assert (condvar1 == condvar2);
    }
  else
    {
      _dbus_assert (condvar1 != condvar2);
    }
}


int
main (int argc, char *argv[])
{
  DBusMessage *method;
  DBusConnection *conn;
  DBusError error;
  DBusMutex *mutex1, *dispatch_mutex1, *io_path_mutex1;
  DBusCondVar *dispatch_cond1, *io_path_cond1;
  DBusMutex *mutex2, *dispatch_mutex2, *io_path_mutex2;
  DBusCondVar *dispatch_cond2, *io_path_cond2;

  printf ("*** Testing late thread init\n");

  dbus_error_init (&error);

  conn = dbus_bus_get (DBUS_BUS_SESSION, &error);

  _dbus_connection_test_get_locks (conn, &mutex1, 
                                         &dispatch_mutex1, 
                                         &io_path_mutex1,
                                         &dispatch_cond1,
                                         &io_path_cond1);
  _run_iteration (conn);
  _dbus_connection_test_get_locks (conn, &mutex2,
                                         &dispatch_mutex2,
                                         &io_path_mutex2,
                                         &dispatch_cond2,
                                         &io_path_cond2);

  check_mutex_lock (mutex1, mutex2, TRUE);
  check_mutex_lock (dispatch_mutex1, dispatch_mutex2, TRUE);
  check_mutex_lock (io_path_mutex1, io_path_mutex2, TRUE);
  check_condvar_lock (dispatch_cond1, dispatch_cond2, TRUE);
  check_condvar_lock (io_path_cond1, io_path_cond2, TRUE);

  dbus_threads_init_default ();

  _dbus_connection_test_get_locks (conn, &mutex1,
                                         &dispatch_mutex1,
                                         &io_path_mutex1,
                                         &dispatch_cond1,
                                         &io_path_cond1);

  /* Since 1.7 it is no longer the case that mutex1 != mutex2, because
   * initializing global locks automatically initializes locks
   * in general. However, it is true that the mutex is not the dummy
   * implementation, which is what we really wanted to check here. */
  _dbus_assert (mutex1 != (DBusMutex *) 0xABCDEF);
  _dbus_assert (dispatch_mutex1 != (DBusMutex *) 0xABCDEF);
  _dbus_assert (dispatch_cond1 != (DBusCondVar *) 0xABCDEF2);
  _dbus_assert (io_path_mutex1 != (DBusMutex *) 0xABCDEF);
  _dbus_assert (io_path_cond1 != (DBusCondVar *) 0xABCDEF2);

  _run_iteration (conn);
  _dbus_connection_test_get_locks (conn, &mutex2,
                                         &dispatch_mutex2,
                                         &io_path_mutex2,
                                         &dispatch_cond2,
                                         &io_path_cond2);

  check_mutex_lock (mutex1, mutex2, TRUE);
  check_mutex_lock (dispatch_mutex1, dispatch_mutex2, TRUE);
  check_mutex_lock (io_path_mutex1, io_path_mutex2, TRUE);
  check_condvar_lock (dispatch_cond1, dispatch_cond2, TRUE);
  check_condvar_lock (io_path_cond1, io_path_cond2, TRUE);

  method = dbus_message_new_method_call ("org.freedesktop.TestSuiteEchoService",
                                         "/org/freedesktop/TestSuite",
                                         "org.freedesktop.TestSuite",
                                         "Exit");
  dbus_connection_send (conn, method, NULL);
  dbus_message_unref (method);

  printf ("Success ***\n");
  exit (0);
}
