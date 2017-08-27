#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

int main() {
  int listen_fd;
  struct sockaddr_un addr;

  listen_fd = socket (PF_UNIX, SOCK_STREAM, 0);

  if (listen_fd < 0)
    {
      fprintf (stderr, "socket() failed: %s\n", strerror (errno));
      exit (1);
    }

  memset (&addr, '\0', sizeof (addr));
  addr.sun_family = AF_UNIX;
  strcpy (addr.sun_path, "X/tmp/dbus-fake-socket-path-used-in-configure-test");
  addr.sun_path[0] = '\0'; /* this is what makes it abstract */

  if (bind (listen_fd, (struct sockaddr*) &addr, SUN_LEN (&addr)) < 0)
    {
      fprintf (stderr, "Abstract socket namespace bind() failed: %s\n",
                strerror (errno));
      exit (1);
    }
  else
    exit (0);
}