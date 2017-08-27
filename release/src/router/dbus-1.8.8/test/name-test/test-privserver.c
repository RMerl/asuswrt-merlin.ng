#include <config.h>
#include "../test-utils.h"

static void
die (const char *message, ...)
{
  va_list args;
  va_start (args, message);
  vfprintf (stderr, message, args);
  va_end (args);
  exit (1);
}

typedef struct TestServiceData TestServiceData;

struct TestServiceData
{
  DBusLoop *loop;
  char *private_addr;
};

static void
new_connection_callback (DBusServer     *server,
                         DBusConnection *new_connection,
                         void           *data)
{
  TestServiceData *testdata = data;

  if (!test_connection_setup (testdata->loop, new_connection))
    dbus_connection_close (new_connection);
}

static DBusHandlerResult 
filter_session_message (DBusConnection     *connection,
                        DBusMessage        *message,
                        void               *user_data)
{
  TestServiceData *testdata = user_data;

  if (dbus_message_is_method_call (message,
                                   "org.freedesktop.DBus.TestSuite.PrivServer",
                                   "GetPrivateAddress"))
    {
       DBusMessage *reply;
       reply = dbus_message_new_method_return (message);
       dbus_message_append_args (reply, DBUS_TYPE_STRING, 
                                 &(testdata->private_addr), DBUS_TYPE_INVALID);
       dbus_connection_send (connection, reply, NULL);
       dbus_message_unref (reply);
       return DBUS_HANDLER_RESULT_HANDLED;
    }
  else if (dbus_message_is_method_call (message,
                                   "org.freedesktop.DBus.TestSuite.PrivServer",
                                   "Quit"))
    {
      fprintf (stderr, "server exiting loop\n");
      _dbus_loop_quit (testdata->loop);
      return DBUS_HANDLER_RESULT_HANDLED;
    }
  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

int
main (int argc, char *argv[])
{
  DBusServer *server;
  DBusError error;
  DBusLoop *loop;
  DBusConnection *session;
  TestServiceData *testdata;

  dbus_error_init (&error);

  loop = _dbus_loop_new ();

  testdata = dbus_new (TestServiceData, 1);
  testdata->loop = loop;

  session = dbus_bus_get (DBUS_BUS_SESSION, &error);
  if (!session)
    die ("couldn't access session bus");

  test_connection_setup (loop, session);

  dbus_bus_request_name (session, "org.freedesktop.DBus.TestSuite.PrivServer", 0, &error);
  if (dbus_error_is_set (&error))
    die ("couldn't request name: %s", error.message);

  if (!dbus_connection_add_filter (session, filter_session_message, testdata, NULL))
    die ("couldn't add filter");

#ifdef DBUS_CMAKE
  server = dbus_server_listen (TEST_LISTEN, &error);
#else
  server = dbus_server_listen ("unix:tmpdir=/tmp", &error);
#endif
  if (!server)
    die (error.message);
  testdata->private_addr = dbus_server_get_address (server);
  printf ("test server listening on %s\n", testdata->private_addr);

  dbus_server_set_new_connection_function (server, new_connection_callback,
                                           testdata, NULL);

  if (!test_server_setup (loop, server))
    die ("server setup failed");

  fprintf (stderr, "server running mainloop\n");
  _dbus_loop_run (loop);
  fprintf (stderr, "server mainloop quit\n");

  test_server_shutdown (loop, server);

  test_connection_shutdown (loop, session);

  dbus_connection_unref (session);

  _dbus_loop_unref (loop);

  dbus_free (testdata);

  return 0;
}
