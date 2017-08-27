#include <stdlib.h>
#include <gio/gio.h>

int
main (int argc, char *argv[])
{
  const gchar *envvar;
  gint pid_from_env;

  envvar = g_getenv ("GIO_LAUNCHED_DESKTOP_FILE_PID");
  g_assert (envvar != NULL);
  pid_from_env = atoi (envvar);
  g_assert_cmpint (pid_from_env, ==, getpid ());

  envvar = g_getenv ("GIO_LAUNCHED_DESKTOP_FILE");
  g_assert_cmpstr (envvar, ==, g_test_get_filename (G_TEST_DIST, "appinfo-test.desktop", NULL));

  return 0;
}

