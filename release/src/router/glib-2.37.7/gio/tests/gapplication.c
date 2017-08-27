#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gdbus-tests.h"
#include "gdbus-sessionbus.h"

static gint outstanding_watches;
static GMainLoop *main_loop;

typedef struct
{
  const gchar *expected_stdout;
  gint stdout_pipe;
} ChildData;

static void
child_quit (GPid     pid,
            gint     status,
            gpointer data)
{
  ChildData *child = data;
  gssize expected, actual;
  gchar *buffer;

  g_assert_cmpint (status, ==, 0);

  if (--outstanding_watches == 0)
    g_main_loop_quit (main_loop);

  expected = strlen (child->expected_stdout);
  buffer = g_alloca (expected + 100);
  actual = read (child->stdout_pipe, buffer, expected + 100);
  close (child->stdout_pipe);

  g_assert_cmpint (actual, >=, 0);

  if (actual != expected ||
      memcmp (buffer, child->expected_stdout, expected) != 0)
    {
      buffer[MIN(expected + 100, actual)] = '\0';

      g_error ("\nExpected\n-----\n%s-----\nGot (%s)\n-----\n%s-----\n",
               child->expected_stdout,
               (actual > expected) ? "truncated" : "full", buffer);
    }

  g_slice_free (ChildData, child);
}

static void
spawn (const gchar *expected_stdout,
       const gchar *first_arg,
       ...)
{
  GError *error = NULL;
  const gchar *arg;
  GPtrArray *array;
  ChildData *data;
  gchar **args;
  va_list ap;
  GPid pid;
  GPollFD fd;

  va_start (ap, first_arg);
  array = g_ptr_array_new ();
  g_ptr_array_add (array, g_test_build_filename (G_TEST_BUILT, "basic-application", NULL));
  for (arg = first_arg; arg; arg = va_arg (ap, const gchar *))
    g_ptr_array_add (array, g_strdup (arg));
  g_ptr_array_add (array, NULL);
  args = (gchar **) g_ptr_array_free (array, FALSE);

  va_end (ap);

  data = g_slice_new (ChildData);
  data->expected_stdout = expected_stdout;

  g_spawn_async_with_pipes (NULL, args, NULL,
                            G_SPAWN_DO_NOT_REAP_CHILD,
                            NULL, NULL, &pid, NULL,
                            &data->stdout_pipe, NULL, &error);
  g_assert_no_error (error);

  g_child_watch_add (pid, child_quit, data);
  outstanding_watches++;

  /* we block until the children write to stdout to make sure
   * they have started, as they need to be executed in order;
   * see https://bugzilla.gnome.org/show_bug.cgi?id=664627
   */
  fd.fd = data->stdout_pipe;
  fd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
  g_poll (&fd, 1, -1);
}

static void
basic (void)
{
  GDBusConnection *c;

  session_bus_up ();
  c = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);

  main_loop = g_main_loop_new (NULL, 0);

  /* spawn the master */
  spawn ("activated\n"
         "open file:///a file:///b\n"
         "cmdline '40 +' '2'\n"
         "exit status: 0\n",
         "./app", NULL);

  /* send it some files */
  spawn ("exit status: 0\n",
         "./app", "/a", "/b", NULL);

  spawn ("40 + 2 = 42\n"
         "exit status: 42\n",
         "./cmd", "40 +", "2", NULL);

  g_main_loop_run (main_loop);

  g_object_unref (c);
  session_bus_down ();
}


#if 0
/* Now that we register non-unique apps on the bus we need to fix the
 * following test not to assume that it's safe to create multiple instances
 * of the same app in one process.
 *
 * See https://bugzilla.gnome.org/show_bug.cgi?id=647986 for the patch that
 * introduced this problem.
 */

static GApplication *recently_activated;
static GMainLoop *loop;

static void
nonunique_activate (GApplication *application)
{
  recently_activated = application;

  if (loop != NULL)
    g_main_loop_quit (loop);
}

static GApplication *
make_app (gboolean non_unique)
{
  GApplication *app;
  gboolean ok;

  app = g_application_new ("org.gtk.Test-Application",
                           non_unique ? G_APPLICATION_NON_UNIQUE : 0);
  g_signal_connect (app, "activate", G_CALLBACK (nonunique_activate), NULL);
  ok = g_application_register (app, NULL, NULL);
  if (!ok)
    {
      g_object_unref (app);
      return NULL;
    }

  g_application_activate (app);

  return app;
}

static void
test_nonunique (void)
{
  GApplication *first, *second, *third, *fourth;

  session_bus_up ();

  first = make_app (TRUE);
  /* non-remote because it is non-unique */
  g_assert (!g_application_get_is_remote (first));
  g_assert (recently_activated == first);
  recently_activated = NULL;

  second = make_app (FALSE);
  /* non-remote because it is first */
  g_assert (!g_application_get_is_remote (second));
  g_assert (recently_activated == second);
  recently_activated = NULL;

  third = make_app (TRUE);
  /* non-remote because it is non-unique */
  g_assert (!g_application_get_is_remote (third));
  g_assert (recently_activated == third);
  recently_activated = NULL;

  fourth = make_app (FALSE);
  /* should have failed to register due to being
   * unable to register the object paths
   */
  g_assert (fourth == NULL);
  g_assert (recently_activated == NULL);

  g_object_unref (first);
  g_object_unref (second);
  g_object_unref (third);

  session_bus_down ();
}
#endif

static void
properties (void)
{
  GDBusConnection *c;
  GObject *app;
  gchar *id;
  GApplicationFlags flags;
  gboolean registered;
  guint timeout;
  gboolean remote;
  gboolean ret;
  GError *error = NULL;

  session_bus_up ();
  c = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);

  app = g_object_new (G_TYPE_APPLICATION,
                      "application-id", "org.gtk.TestApplication",
                      NULL);

  g_object_get (app,
                "application-id", &id,
                "flags", &flags,
                "is-registered", &registered,
                "inactivity-timeout", &timeout,
                NULL);

  g_assert_cmpstr (id, ==, "org.gtk.TestApplication");
  g_assert_cmpint (flags, ==, G_APPLICATION_FLAGS_NONE);
  g_assert (!registered);
  g_assert_cmpint (timeout, ==, 0);

  ret = g_application_register (G_APPLICATION (app), NULL, &error);
  g_assert (ret);
  g_assert_no_error (error);

  g_object_get (app,
                "is-registered", &registered,
                "is-remote", &remote,
                NULL);

  g_assert (registered);
  g_assert (!remote);

  g_object_set (app,
                "inactivity-timeout", 1000,
                NULL);

  g_application_quit (G_APPLICATION (app));

  g_object_unref (c);
  g_object_unref (app);
  g_free (id);

  session_bus_down ();
}

static void
appid (void)
{
  gchar *id;

  g_assert (!g_application_id_is_valid (""));
  g_assert (!g_application_id_is_valid ("."));
  g_assert (!g_application_id_is_valid ("a"));
  g_assert (!g_application_id_is_valid ("abc"));
  g_assert (!g_application_id_is_valid (".abc"));
  g_assert (!g_application_id_is_valid ("abc."));
  g_assert (!g_application_id_is_valid ("a..b"));
  g_assert (!g_application_id_is_valid ("a/b"));
  g_assert (!g_application_id_is_valid ("a\nb"));
  g_assert (!g_application_id_is_valid ("a\nb"));
  g_assert (!g_application_id_is_valid ("_a.b"));
  g_assert (!g_application_id_is_valid ("-a.b"));
  id = g_new0 (gchar, 261);
  memset (id, 'a', 260);
  id[1] = '.';
  id[260] = 0;
  g_assert (!g_application_id_is_valid (id));
  g_free (id);

  g_assert (g_application_id_is_valid ("a.b"));
  g_assert (g_application_id_is_valid ("A.B"));
  g_assert (g_application_id_is_valid ("A-.B"));
  g_assert (g_application_id_is_valid ("a_b.c-d"));
  g_assert (g_application_id_is_valid ("org.gnome.SessionManager"));
}

static gboolean nodbus_activated;

static gboolean
release_app (gpointer user_data)
{
  g_application_release (user_data);
  return G_SOURCE_REMOVE;
}

static void
nodbus_activate (GApplication *app)
{
  nodbus_activated = TRUE;
  g_application_hold (app);

  g_assert (g_application_get_dbus_connection (app) == NULL);
  g_assert (g_application_get_dbus_object_path (app) == NULL);

  g_idle_add (release_app, app);
}

static void
test_nodbus (void)
{
  char *binpath = g_test_build_filename (G_TEST_BUILT, "unimportant", NULL);
  gchar *argv[] = { binpath, NULL };
  GApplication *app;

  app = g_application_new ("org.gtk.Unimportant", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (nodbus_activate), NULL);
  g_application_run (app, 1, argv);
  g_object_unref (app);

  g_assert (nodbus_activated);
  g_free (binpath);
}

static gboolean noappid_activated;

static void
noappid_activate (GApplication *app)
{
  noappid_activated = TRUE;
  g_application_hold (app);

  g_assert (g_application_get_flags (app) & G_APPLICATION_NON_UNIQUE);

  g_idle_add (release_app, app);
}

/* test that no appid -> non-unique */
static void
test_noappid (void)
{
  char *binpath = g_test_build_filename (G_TEST_BUILT, "unimportant", NULL);
  gchar *argv[] = { binpath, NULL };
  GApplication *app;

  app = g_application_new (NULL, G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (noappid_activate), NULL);
  g_application_run (app, 1, argv);
  g_object_unref (app);

  g_assert (noappid_activated);
  g_free (binpath);
}


static gboolean
quit_app (gpointer user_data)
{
  g_application_quit (user_data);
  return G_SOURCE_REMOVE;
}

static gboolean quit_activated;

static void
quit_activate (GApplication *app)
{
  quit_activated = TRUE;
  g_application_hold (app);

  g_assert (g_application_get_dbus_connection (app) != NULL);
  g_assert (g_application_get_dbus_object_path (app) != NULL);

  g_idle_add (quit_app, app);
}

static void
test_quit (void)
{
  GDBusConnection *c;
  char *binpath = g_test_build_filename (G_TEST_BUILT, "unimportant", NULL);
  gchar *argv[] = { binpath, NULL };
  GApplication *app;

  session_bus_up ();
  c = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);

  app = g_application_new ("org.gtk.Unimportant",
                           G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (quit_activate), NULL);
  g_application_run (app, 1, argv);
  g_object_unref (app);
  g_object_unref (c);

  g_assert (quit_activated);

  session_bus_down ();
  g_free (binpath);
}

static void
on_activate (GApplication *app)
{
  gchar **actions;
  GAction *action;
  GVariant *state;

  g_assert (!g_application_get_is_remote (app));

  actions = g_action_group_list_actions (G_ACTION_GROUP (app));
  g_assert (g_strv_length (actions) == 0);
  g_strfreev (actions);

  action = (GAction*)g_simple_action_new_stateful ("test", G_VARIANT_TYPE_BOOLEAN, g_variant_new_boolean (FALSE));
  g_action_map_add_action (G_ACTION_MAP (app), action);

  actions = g_action_group_list_actions (G_ACTION_GROUP (app));
  g_assert (g_strv_length (actions) == 1);
  g_strfreev (actions);

  g_action_group_change_action_state (G_ACTION_GROUP (app), "test", g_variant_new_boolean (TRUE));
  state = g_action_group_get_action_state (G_ACTION_GROUP (app), "test");
  g_assert (g_variant_get_boolean (state) == TRUE);

  g_action_map_remove_action (G_ACTION_MAP (app), "test");

  actions = g_action_group_list_actions (G_ACTION_GROUP (app));
  g_assert (g_strv_length (actions) == 0);
  g_strfreev (actions);

  g_idle_add (quit_app, app);
}

static void
test_actions (void)
{
  char *binpath = g_test_build_filename (G_TEST_BUILT, "unimportant", NULL);
  gchar *argv[] = { binpath, NULL };
  GApplication *app;

  g_unsetenv ("DBUS_SESSION_BUS_ADDRESS");

  app = g_application_new ("org.gtk.Unimportant",
                           G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);
  g_application_run (app, 1, argv);
  g_object_unref (app);
  g_free (binpath);
}

typedef GApplication TestLocCmdApp;
typedef GApplicationClass TestLocCmdAppClass;

static GType test_loc_cmd_app_get_type (void);
G_DEFINE_TYPE (TestLocCmdApp, test_loc_cmd_app, G_TYPE_APPLICATION)

static void
test_loc_cmd_app_init (TestLocCmdApp *app)
{
}

static void
test_loc_cmd_app_startup (GApplication *app)
{
  g_assert_not_reached ();
}

static void
test_loc_cmd_app_shutdown (GApplication *app)
{
  g_assert_not_reached ();
}

static gboolean
test_loc_cmd_app_local_command_line (GApplication   *application,
                                     gchar        ***arguments,
                                     gint           *exit_status)
{
  return TRUE;
}

static void
test_loc_cmd_app_class_init (TestLocCmdAppClass *klass)
{
  G_APPLICATION_CLASS (klass)->startup = test_loc_cmd_app_startup;
  G_APPLICATION_CLASS (klass)->shutdown = test_loc_cmd_app_shutdown;
  G_APPLICATION_CLASS (klass)->local_command_line = test_loc_cmd_app_local_command_line;
}

static void
test_local_command_line (void)
{
  char *binpath = g_test_build_filename (G_TEST_BUILT, "unimportant", NULL);
  gchar *argv[] = { binpath, "-invalid", NULL };
  GApplication *app;

  g_unsetenv ("DBUS_SESSION_BUS_ADDRESS");

  app = g_object_new (test_loc_cmd_app_get_type (),
                      "application-id", "org.gtk.Unimportant",
                      "flags", G_APPLICATION_FLAGS_NONE,
                      NULL);
  g_application_run (app, 1, argv);
  g_object_unref (app);
  g_free (binpath);
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_dbus_unset ();

  g_test_add_func ("/gapplication/no-dbus", test_nodbus);
  g_test_add_func ("/gapplication/basic", basic);
  g_test_add_func ("/gapplication/no-appid", test_noappid);
/*  g_test_add_func ("/gapplication/non-unique", test_nonunique); */
  g_test_add_func ("/gapplication/properties", properties);
  g_test_add_func ("/gapplication/app-id", appid);
  g_test_add_func ("/gapplication/quit", test_quit);
  g_test_add_func ("/gapplication/actions", test_actions);
  g_test_add_func ("/gapplication/local-command-line", test_local_command_line);

  return g_test_run ();
}
