#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>
#include <dbus/dbus-connection-internal.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

static void
die (const char *message)
{
  fprintf (stderr, "*** test-ids: %s", message);
  exit (1);
}

int
main (int    argc,
      char **argv)
{
  DBusError error;
  DBusConnection *connection;
  char *id;
  char *server_id;
  
  dbus_error_init (&error);
  connection = dbus_bus_get (DBUS_BUS_SESSION, &error);
  if (connection == NULL)
    {
      fprintf (stderr, "*** Failed to open connection to system bus: %s\n",
               error.message);
      dbus_error_free (&error);
      return 1;
    }

  server_id = dbus_connection_get_server_id (connection);
  if (server_id == NULL)
    die ("No bus server ID retrieved\n");
  /* printf("'%s'\n", server_id); */
  if (strlen (server_id) != 32)
    die ("Bus server id should have length 32\n");
  dbus_free (server_id);

  id = dbus_bus_get_id (connection, NULL);
  if (id == NULL)
    die ("No bus ID retrieved\n");
  /* printf("'%s'\n", id); */
  if (strlen (id) != 32)
    die ("Bus ID should have length 32\n");
  dbus_free (id);  
  
  _dbus_verbose ("*** Test IDs exiting\n");
  
  return 0;
}
