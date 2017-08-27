
#include <config.h>
#include "../test-utils.h"

static DBusLoop *loop;

static void
die (const char *message)
{
  fprintf (stderr, "*** test-shutdown: %s", message);
  exit (1);
}

static void
open_destroy_shared_session_bus_connection (void)
{
  DBusError error;
  DBusConnection *connection;
  char *session_addr_no_guid;
  char *comma;
  
  dbus_error_init (&error);

  session_addr_no_guid = strdup (getenv ("DBUS_SESSION_BUS_ADDRESS"));
  comma = strchr (session_addr_no_guid, ',');
  if (comma == NULL)
    die ("Couldn't find GUID in session bus address");
  *comma = '\0';
    
  connection = dbus_connection_open (session_addr_no_guid, &error);
  free (session_addr_no_guid);
  if (connection == NULL)
    die ("Failed to open connection to temp session bus\n");

  loop = _dbus_loop_new ();
  if (loop == NULL)
    die ("No memory\n");
  
  if (!test_connection_setup (loop, connection))
    die ("No memory\n");

  test_connection_shutdown (loop, connection);
 
  _dbus_loop_unref (loop);

  dbus_connection_unref (connection); 
}

int
main (int    argc,
      char **argv)
{
  open_destroy_shared_session_bus_connection ();

  dbus_shutdown ();

  open_destroy_shared_session_bus_connection ();

  dbus_shutdown ();

  open_destroy_shared_session_bus_connection ();

  dbus_shutdown ();

  _dbus_verbose ("*** Test shutdown exiting\n");
  
  return 0;
}
