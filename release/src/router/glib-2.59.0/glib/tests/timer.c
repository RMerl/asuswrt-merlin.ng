/* Unit tests for GTimer
 * Copyright (C) 2013 Red Hat, Inc.
 *
 * This work is provided "as is"; redistribution and modification
 * in whole or in part, in any medium, physical or electronic is
 * permitted without restriction.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * In no event shall the authors or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 * Author: Matthias Clasen
 */

#include "glib.h"

static void
test_timer_basic (void)
{
  GTimer *timer;
  gdouble elapsed;
  gulong micros;

  timer = g_timer_new ();

  elapsed = g_timer_elapsed (timer, &micros);

  g_assert_cmpfloat (elapsed, <, 1.0);
  g_assert_cmpuint (micros, ==, ((guint64)(elapsed * 1e6)) % 1000000);

  g_timer_destroy (timer);
}

static void
test_timer_stop (void)
{
  GTimer *timer;
  gdouble elapsed, elapsed2;

  timer = g_timer_new ();

  g_timer_stop (timer);

  elapsed = g_timer_elapsed (timer, NULL);
  g_usleep (100);
  elapsed2 = g_timer_elapsed (timer, NULL);

  g_assert_cmpfloat (elapsed, ==, elapsed2);

  g_timer_destroy (timer);
}

static void
test_timer_continue (void)
{
  GTimer *timer;
  gdouble elapsed, elapsed2;

  timer = g_timer_new ();
  g_usleep (100);
  g_timer_stop (timer);

  elapsed = g_timer_elapsed (timer, NULL);
  g_timer_continue (timer);
  g_usleep (100);
  elapsed2 = g_timer_elapsed (timer, NULL);

  g_assert_cmpfloat (elapsed, <, elapsed2);

  g_timer_destroy (timer);
}

static void
test_timer_reset (void)
{
  GTimer *timer;
  gdouble elapsed, elapsed2;

  timer = g_timer_new ();
  g_usleep (100);
  g_timer_stop (timer);

  elapsed = g_timer_elapsed (timer, NULL);
  g_timer_reset (timer);
  elapsed2 = g_timer_elapsed (timer, NULL);

  g_assert_cmpfloat (elapsed, >, elapsed2);

  g_timer_destroy (timer);
}

static void
test_timeval_add (void)
{
  GTimeVal time = { 1, 0 };

  g_time_val_add (&time, 10);

  g_assert_cmpint (time.tv_sec, ==, 1); 
  g_assert_cmpint (time.tv_usec, ==, 10); 

  g_time_val_add (&time, -500);
  g_assert_cmpint (time.tv_sec, ==, 0); 
  g_assert_cmpint (time.tv_usec, ==, G_USEC_PER_SEC - 490); 

  g_time_val_add (&time, 1000);
  g_assert_cmpint (time.tv_sec, ==, 1); 
  g_assert_cmpint (time.tv_usec, ==, 510);

  g_time_val_add (&time, 0);
  g_assert_cmpint (time.tv_sec, ==, 1);
  g_assert_cmpint (time.tv_usec, ==, 510);

  g_time_val_add (&time, -210);
  g_assert_cmpint (time.tv_sec, ==, 1);
  g_assert_cmpint (time.tv_usec, ==, 300);
}

typedef struct {
  gboolean success;
  const gchar *in;
  GTimeVal val;
} TimeValParseTest;

static void
test_timeval_from_iso8601 (void)
{
  gchar *old_tz = g_strdup (g_getenv ("TZ"));
  TimeValParseTest tests[] = {
    { TRUE, "1990-11-01T10:21:17Z", { 657454877, 0 } },
    { TRUE, "19901101T102117Z", { 657454877, 0 } },
    { TRUE, "19901101T102117+5", { 657454577, 0 } },
    { TRUE, "19901101T102117+3:15", { 657443177, 0 } },
    { TRUE, "  1990-11-01T10:21:17Z  ", { 657454877, 0 } },
    { TRUE, "1970-01-01T00:00:17.12Z", { 17, 120000 } },
    { TRUE, "1970-01-01T00:00:17.1234Z", { 17, 123400 } },
    { TRUE, "1970-01-01T00:00:17.123456Z", { 17, 123456 } },
    { TRUE, "1980-02-22T12:36:00+02:00", { 320063760, 0 } },
    { FALSE, "   ", { 0, 0 } },
    { FALSE, "x", { 0, 0 } },
    { FALSE, "123x", { 0, 0 } },
    { FALSE, "2001-10+x", { 0, 0 } },
    { FALSE, "1980-02-22T", { 0, 0 } },
    { FALSE, "2001-10-08Tx", { 0, 0 } },
    { FALSE, "2001-10-08T10:11x", { 0, 0 } },
    { FALSE, "Wed Dec 19 17:20:20 GMT 2007", { 0, 0 } },
    { FALSE, "1980-02-22T10:36:00Zulu", { 0, 0 } },
    { FALSE, "2T0+819855292164632335", { 0, 0 } },
    { TRUE, "2018-08-03T14:08:05.446178377+01:00", { 1533301685, 446178 } },
    { FALSE, "2147483648-08-03T14:08:05.446178377+01:00", { 0, 0 } },
    { FALSE, "2018-13-03T14:08:05.446178377+01:00", { 0, 0 } },
    { FALSE, "2018-00-03T14:08:05.446178377+01:00", { 0, 0 } },
    { FALSE, "2018-08-00T14:08:05.446178377+01:00", { 0, 0 } },
    { FALSE, "2018-08-32T14:08:05.446178377+01:00", { 0, 0 } },
    { FALSE, "2018-08-03T24:08:05.446178377+01:00", { 0, 0 } },
    { FALSE, "2018-08-03T14:60:05.446178377+01:00", { 0, 0 } },
    { FALSE, "2018-08-03T14:08:63.446178377+01:00", { 0, 0 } },
    { FALSE, "2018-08-03T14:08:05.446178377+100:00", { 0, 0 } },
    { FALSE, "2018-08-03T14:08:05.446178377+01:60", { 0, 0 } },
    { TRUE, "20180803T140805.446178377+0100", { 1533301685, 446178 } },
    { FALSE, "21474836480803T140805.446178377+0100", { 0, 0 } },
    { FALSE, "20181303T140805.446178377+0100", { 0, 0 } },
    { FALSE, "20180003T140805.446178377+0100", { 0, 0 } },
    { FALSE, "20180800T140805.446178377+0100", { 0, 0 } },
    { FALSE, "20180832T140805.446178377+0100", { 0, 0 } },
    { FALSE, "20180803T240805.446178377+0100", { 0, 0 } },
    { FALSE, "20180803T146005.446178377+0100", { 0, 0 } },
    { FALSE, "20180803T140863.446178377+0100", { 0, 0 } },
    { FALSE, "20180803T140805.446178377+10000", { 0, 0 } },
    { FALSE, "20180803T140805.446178377+0160", { 0, 0 } },
    { TRUE, "+1980-02-22T12:36:00+02:00", { 320063760, 0 } },
    { FALSE, "-0005-01-01T00:00:00Z", { 0, 0 } },
    { FALSE, "2018-08-06", { 0, 0 } },
    { FALSE, "2018-08-06 13:51:00Z", { 0, 0 } },
    { TRUE, "20180803T140805,446178377+0100", { 1533301685, 446178 } },
    { TRUE, "2018-08-03T14:08:05.446178377-01:00", { 1533308885, 446178 } },
    { FALSE, "2018-08-03T14:08:05.446178377 01:00", { 0, 0 } },
    { TRUE, "1990-11-01T10:21:17", { 657454877, 0 } },
    { TRUE, "1990-11-01T10:21:17     ", { 657454877, 0 } },
  };
  GTimeVal out;
  gboolean success;
  gint i;

  /* Always run in UTC so the comparisons of parsed values are valid. */
  if (!g_setenv ("TZ", "UTC", TRUE))
    {
      g_test_skip ("Failed to set TZ=UTC");
      return;
    }

  for (i = 0; i < G_N_ELEMENTS (tests); i++)
    {
      out.tv_sec = 0;
      out.tv_usec = 0;
      success = g_time_val_from_iso8601 (tests[i].in, &out);
      g_assert (success == tests[i].success);
      if (tests[i].success)
        {
          g_assert_cmpint (out.tv_sec, ==, tests[i].val.tv_sec);
          g_assert_cmpint (out.tv_usec, ==, tests[i].val.tv_usec);
        }
    }

  if (old_tz != NULL)
    g_assert_true (g_setenv ("TZ", old_tz, TRUE));
  else
    g_unsetenv ("TZ");

  g_free (old_tz);
}

typedef struct {
  GTimeVal val;
  const gchar *expected;
} TimeValFormatTest;

static void
test_timeval_to_iso8601 (void)
{
  TimeValFormatTest tests[] = {
    { { 657454877, 0 }, "1990-11-01T10:21:17Z" },
    { { 17, 123400 }, "1970-01-01T00:00:17.123400Z" }
  };
  gint i;
  gchar *out;
  GTimeVal val;
  gboolean ret;

  g_unsetenv ("TZ");

  for (i = 0; i < G_N_ELEMENTS (tests); i++)
    {
      out = g_time_val_to_iso8601 (&(tests[i].val));
      g_assert_cmpstr (out, ==, tests[i].expected);

      ret = g_time_val_from_iso8601 (out, &val);
      g_assert (ret);
      g_assert_cmpint (val.tv_sec, ==, tests[i].val.tv_sec);
      g_assert_cmpint (val.tv_usec, ==, tests[i].val.tv_usec);
      g_free (out);
    }
}

/* Test error handling for g_time_val_to_iso8601() on dates which are too large. */
static void
test_timeval_to_iso8601_overflow (void)
{
  GTimeVal val;
  gchar *out = NULL;

  if ((glong) G_MAXINT == G_MAXLONG)
    {
      g_test_skip ("G_MAXINT == G_MAXLONG - we can't make g_time_val_to_iso8601() overflow.");
      return;
    }

  g_unsetenv ("TZ");

  val.tv_sec = G_MAXLONG;
  val.tv_usec = G_USEC_PER_SEC - 1;

  out = g_time_val_to_iso8601 (&val);
  g_assert_null (out);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/timer/basic", test_timer_basic);
  g_test_add_func ("/timer/stop", test_timer_stop);
  g_test_add_func ("/timer/continue", test_timer_continue);
  g_test_add_func ("/timer/reset", test_timer_reset);
  g_test_add_func ("/timeval/add", test_timeval_add);
  g_test_add_func ("/timeval/from-iso8601", test_timeval_from_iso8601);
  g_test_add_func ("/timeval/to-iso8601", test_timeval_to_iso8601);
  g_test_add_func ("/timeval/to-iso8601/overflow", test_timeval_to_iso8601_overflow);

  return g_test_run ();
}
