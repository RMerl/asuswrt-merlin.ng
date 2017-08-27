#include <stdlib.h>
#include <glib.h>

/* Test g_warn macros */
static void
test_warnings (void)
{
  g_test_expect_message (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING,
                         "*test_warnings*should not be reached*");
  g_warn_if_reached ();
  g_test_assert_expected_messages ();

  g_test_expect_message (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING,
                         "*test_warnings*runtime check failed*");
  g_warn_if_fail (FALSE);
  g_test_assert_expected_messages ();
}

static guint log_count = 0;

static void
log_handler (const gchar    *log_domain,
             GLogLevelFlags  log_level,
             const gchar    *message,
             gpointer        user_data)
{
  g_assert_cmpstr (log_domain, ==, "bu");
  g_assert_cmpint (log_level, ==, G_LOG_LEVEL_INFO);

  log_count++;
}

/* test that custom log handlers only get called for
 * their domain and level
 */
static void
test_set_handler (void)
{
  guint id;

  id = g_log_set_handler ("bu", G_LOG_LEVEL_INFO, log_handler, NULL);

  g_log ("bu", G_LOG_LEVEL_DEBUG, "message");
  g_log ("ba", G_LOG_LEVEL_DEBUG, "message");
  g_log ("bu", G_LOG_LEVEL_INFO, "message");
  g_log ("ba", G_LOG_LEVEL_INFO, "message");

  g_assert_cmpint (log_count, ==, 1);

  g_log_remove_handler ("bu", id);
}

static void
test_default_handler_error (void)
{
  g_log_set_default_handler (g_log_default_handler, NULL);
  g_error ("message1");
  exit (0);
}

static void
test_default_handler_critical (void)
{
  g_log_set_default_handler (g_log_default_handler, NULL);
  g_critical ("message2");
  exit (0);
}

static void
test_default_handler_warning (void)
{
  g_log_set_default_handler (g_log_default_handler, NULL);
  g_warning ("message3");
  exit (0);
}

static void
test_default_handler_message (void)
{
  g_log_set_default_handler (g_log_default_handler, NULL);
  g_message ("message4");
  exit (0);
}

static void
test_default_handler_info (void)
{
  g_log_set_default_handler (g_log_default_handler, NULL);
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "message5");
  exit (0);
}

static void
test_default_handler_bar_info (void)
{
  g_log_set_default_handler (g_log_default_handler, NULL);

  g_setenv ("G_MESSAGES_DEBUG", "foo bar baz", TRUE);

  g_log ("bar", G_LOG_LEVEL_INFO, "message5");
  exit (0);
}

static void
test_default_handler_baz_debug (void)
{
  g_log_set_default_handler (g_log_default_handler, NULL);

  g_setenv ("G_MESSAGES_DEBUG", "foo bar baz", TRUE);

  g_log ("baz", G_LOG_LEVEL_DEBUG, "message6");
  exit (0);
}

static void
test_default_handler_debug (void)
{
  g_log_set_default_handler (g_log_default_handler, NULL);

  g_setenv ("G_MESSAGES_DEBUG", "all", TRUE);

  g_log ("foo", G_LOG_LEVEL_DEBUG, "6");
  g_log ("bar", G_LOG_LEVEL_DEBUG, "6");
  g_log ("baz", G_LOG_LEVEL_DEBUG, "6");
  exit (0);
}

static void
test_default_handler_0x400 (void)
{
  g_log_set_default_handler (g_log_default_handler, NULL);
  g_log (G_LOG_DOMAIN, 1<<10, "message7");
  exit (0);
}

static void
test_default_handler (void)
{
  g_test_trap_subprocess ("/logging/default-handler/subprocess/error", 0, 0);
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*ERROR*message1*");

  g_test_trap_subprocess ("/logging/default-handler/subprocess/critical", 0, 0);
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*CRITICAL*message2*");

  g_test_trap_subprocess ("/logging/default-handler/subprocess/warning", 0, 0);
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*WARNING*message3*");

  g_test_trap_subprocess ("/logging/default-handler/subprocess/message", 0, 0);
  g_test_trap_assert_passed ();
  g_test_trap_assert_stderr ("*Message*message4*");

  g_test_trap_subprocess ("/logging/default-handler/subprocess/info", 0, 0);
  g_test_trap_assert_passed ();
  g_test_trap_assert_stdout_unmatched ("*INFO*message5*");

  g_test_trap_subprocess ("/logging/default-handler/subprocess/bar-info", 0, 0);
  g_test_trap_assert_passed ();
  g_test_trap_assert_stdout ("*INFO*message5*");

  g_test_trap_subprocess ("/logging/default-handler/subprocess/baz-debug", 0, 0);
  g_test_trap_assert_passed ();
  g_test_trap_assert_stdout ("*DEBUG*message6*");

  g_test_trap_subprocess ("/logging/default-handler/subprocess/debug", 0, 0);
  g_test_trap_assert_passed ();
  g_test_trap_assert_stdout ("*DEBUG*6*6*6*");

  g_test_trap_subprocess ("/logging/default-handler/subprocess/0x400", 0, 0);
  g_test_trap_assert_passed ();
  g_test_trap_assert_stdout ("*LOG-0x400*message7*");
}

/* test that setting levels as fatal works */
static void
test_fatal_log_mask_subprocess (void)
{
  g_log_set_fatal_mask ("bu", G_LOG_LEVEL_INFO);
  g_log ("bu", G_LOG_LEVEL_INFO, "fatal");
  exit (0);
}

static void
test_fatal_log_mask (void)
{
  g_test_trap_subprocess ("/logging/fatal-log-mask/subprocess", 0, 0);
  g_test_trap_assert_failed ();
  /* G_LOG_LEVEL_INFO isn't printed by default */
  g_test_trap_assert_stdout_unmatched ("*fatal*");
}

static gint my_print_count = 0;
static void
my_print_handler (const gchar *text)
{
  my_print_count++;
}

static void
test_print_handler (void)
{
  GPrintFunc old_print_handler;

  old_print_handler = g_set_print_handler (my_print_handler);
  g_assert (old_print_handler == NULL);

  my_print_count = 0;
  g_print ("bu ba");
  g_assert_cmpint (my_print_count, ==, 1);

  g_set_print_handler (NULL);
}

static void
test_printerr_handler (void)
{
  GPrintFunc old_printerr_handler;

  old_printerr_handler = g_set_printerr_handler (my_print_handler);
  g_assert (old_printerr_handler == NULL);

  my_print_count = 0;
  g_printerr ("bu ba");
  g_assert_cmpint (my_print_count, ==, 1);

  g_set_printerr_handler (NULL);
}

static char *fail_str = "foo";
static char *log_str = "bar";

static gboolean
good_failure_handler (const gchar    *log_domain,
                      GLogLevelFlags  log_level,
                      const gchar    *msg,
                      gpointer        user_data)
{
  g_test_message ("The Good Fail Message Handler\n");
  g_assert ((char *)user_data != log_str);
  g_assert ((char *)user_data == fail_str);

  return FALSE;
}

static gboolean
bad_failure_handler (const gchar    *log_domain,
                     GLogLevelFlags  log_level,
                     const gchar    *msg,
                     gpointer        user_data)
{
  g_test_message ("The Bad Fail Message Handler\n");
  g_assert ((char *)user_data == log_str);
  g_assert ((char *)user_data != fail_str);

  return FALSE;
}

static void
test_handler (const gchar    *log_domain,
              GLogLevelFlags  log_level,
              const gchar    *msg,
              gpointer        user_data)
{
  g_test_message ("The Log Message Handler\n");
  g_assert ((char *)user_data != fail_str);
  g_assert ((char *)user_data == log_str);
}

static void
bug653052 (void)
{
  g_test_bug ("653052");

  g_test_log_set_fatal_handler (good_failure_handler, fail_str);
  g_log_set_default_handler (test_handler, log_str);

  g_return_if_fail (0);

  g_test_log_set_fatal_handler (bad_failure_handler, fail_str);
  g_log_set_default_handler (test_handler, log_str);

  g_return_if_fail (0);
}

int
main (int argc, char *argv[])
{
  g_unsetenv ("G_MESSAGES_DEBUG");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("http://bugzilla.gnome.org/");

  g_test_add_func ("/logging/default-handler", test_default_handler);
  g_test_add_func ("/logging/default-handler/subprocess/error", test_default_handler_error);
  g_test_add_func ("/logging/default-handler/subprocess/critical", test_default_handler_critical);
  g_test_add_func ("/logging/default-handler/subprocess/warning", test_default_handler_warning);
  g_test_add_func ("/logging/default-handler/subprocess/message", test_default_handler_message);
  g_test_add_func ("/logging/default-handler/subprocess/info", test_default_handler_info);
  g_test_add_func ("/logging/default-handler/subprocess/bar-info", test_default_handler_bar_info);
  g_test_add_func ("/logging/default-handler/subprocess/baz-debug", test_default_handler_baz_debug);
  g_test_add_func ("/logging/default-handler/subprocess/debug", test_default_handler_debug);
  g_test_add_func ("/logging/default-handler/subprocess/0x400", test_default_handler_0x400);
  g_test_add_func ("/logging/warnings", test_warnings);
  g_test_add_func ("/logging/fatal-log-mask", test_fatal_log_mask);
  g_test_add_func ("/logging/fatal-log-mask/subprocess", test_fatal_log_mask_subprocess);
  g_test_add_func ("/logging/set-handler", test_set_handler);
  g_test_add_func ("/logging/print-handler", test_print_handler);
  g_test_add_func ("/logging/printerr-handler", test_printerr_handler);
  g_test_add_func ("/logging/653052", bug653052);

  return g_test_run ();
}

