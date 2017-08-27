#undef G_DISABLE_ASSERT
#undef G_LOG_DOMAIN

/* We are testing some deprecated APIs here */
#define GLIB_DISABLE_DEPRECATION_WARNINGS

#include "glib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <time.h>

static void
test_basic (void)
{
  g_assert_cmpint (sizeof (GDate), <,  9);
  g_assert (!g_date_valid_month (G_DATE_BAD_MONTH));
  g_assert (!g_date_valid_month (13));
  g_assert (!g_date_valid_day (G_DATE_BAD_DAY));
  g_assert (!g_date_valid_day (32));
  g_assert (!g_date_valid_year (G_DATE_BAD_YEAR));
  g_assert (!g_date_valid_julian (G_DATE_BAD_JULIAN));
  g_assert (!g_date_valid_weekday (G_DATE_BAD_WEEKDAY));
  g_assert (g_date_is_leap_year (2000));
  g_assert (!g_date_is_leap_year (1999));
  g_assert (g_date_is_leap_year (1996));
  g_assert (g_date_is_leap_year (1600));
  g_assert (!g_date_is_leap_year (2100));
  g_assert (!g_date_is_leap_year (1800));
}

static void
test_empty_constructor (void)
{
  GDate *d;

  d = g_date_new ();
  g_assert (!g_date_valid (d));
  g_date_free (d);
}

static void
test_dmy_constructor (void)
{
  GDate *d;
  guint32 j;

  d = g_date_new_dmy (1, 1, 1);
  g_assert (g_date_valid (d));
  j = g_date_get_julian (d);
  g_assert_cmpint (j, ==, 1);
  g_assert_cmpint (g_date_get_month (d), ==, G_DATE_JANUARY);
  g_assert_cmpint (g_date_get_day (d), ==, 1);
  g_assert_cmpint (g_date_get_year (d), ==, 1);
  g_date_free (d);
}

static void
test_julian_constructor (void)
{
  GDate *d1;
  GDate *d2;

  d1 = g_date_new_julian (4000);
  d2 = g_date_new_julian (5000);
  g_assert_cmpint (g_date_get_julian (d1), ==, 4000);
  g_assert_cmpint (g_date_days_between (d1, d2), ==, 1000);
  g_date_free (d1);
  g_date_free (d2);
}

static void
test_dates (void)
{
  GDate *d;
  GTimeVal tv;

  d = g_date_new ();

  /* today */
  g_date_set_time (d, time (NULL));
  g_assert (g_date_valid (d));

  /* Unix epoch */
  g_date_set_time (d, 1);
  g_assert (g_date_valid (d));

  tv.tv_sec = 0;
  tv.tv_usec = 0;
  g_date_set_time_val (d, &tv);
  g_assert (g_date_valid (d));

  /* Julian day 1 */
  g_date_set_julian (d, 1);
  g_assert (g_date_valid (d));

  g_date_set_year (d, 3);
  g_date_set_day (d, 3);
  g_date_set_month (d, 3);
  g_assert (g_date_valid (d));
  g_assert_cmpint (g_date_get_year (d), ==, 3);
  g_assert_cmpint (g_date_get_month (d), ==, 3);
  g_assert_cmpint (g_date_get_day (d), ==, 3);
  g_assert (!g_date_is_first_of_month (d));
  g_assert (!g_date_is_last_of_month (d));
  g_date_set_day (d, 1);
  g_assert (g_date_is_first_of_month (d));
  g_date_subtract_days (d, 1);
  g_assert (g_date_is_last_of_month (d));

  g_date_free (d);
}

static void
test_parse (void)
{
  GDate *d;
  gchar buf[101];

  d = g_date_new ();

  g_date_set_dmy (d, 10, 1, 2000);
  g_date_strftime (buf, 100, "%x", d);

  g_date_set_parse (d, buf);
  g_assert (g_date_valid (d));
  g_assert_cmpint (g_date_get_month (d), ==, 1);
  g_assert_cmpint (g_date_get_day (d), ==, 10);
  g_assert_cmpint (g_date_get_year (d), ==, 2000);

  g_date_free (d);
}

static void
test_year (gconstpointer t)
{
  GDateYear y = GPOINTER_TO_INT (t);
  GDateMonth m;
  GDateDay day;
  guint32 j;
  GDate *d;
  gint i;
  GDate tmp;

  guint32 first_day_of_year = G_DATE_BAD_JULIAN;
  guint16 days_in_year = g_date_is_leap_year (y) ? 366 : 365;
  guint   sunday_week_of_year = 0;
  guint   sunday_weeks_in_year = g_date_get_sunday_weeks_in_year (y);
  guint   monday_week_of_year = 0;
  guint   monday_weeks_in_year = g_date_get_monday_weeks_in_year (y);
  guint   iso8601_week_of_year = 0;

  g_assert (g_date_valid_year (y));
  /* Years ought to have roundabout 52 weeks */
  g_assert (sunday_weeks_in_year == 52 || sunday_weeks_in_year == 53);
  g_assert (monday_weeks_in_year == 52 || monday_weeks_in_year == 53);

  m = 1;
  while (m < 13)
    {
      guint8 dim = g_date_get_days_in_month (m, y);
      GDate days[31];

      g_date_clear (days, 31);

      g_assert (dim > 0 && dim < 32);
      g_assert (g_date_valid_month (m));

      day = 1;
      while (day <= dim)
        {
          g_assert (g_date_valid_dmy (day, m, y));

          d = &days[day - 1];
          //g_assert (!g_date_valid (d));

          g_date_set_dmy (d, day, m, y);

          g_assert (g_date_valid (d));

          if (m == G_DATE_JANUARY && day == 1)
            first_day_of_year = g_date_get_julian (d);

          g_assert (first_day_of_year != G_DATE_BAD_JULIAN);

          g_assert_cmpint (g_date_get_month (d), ==, m);
          g_assert_cmpint (g_date_get_year (d), ==, y);
          g_assert_cmpint (g_date_get_day (d), ==, day);

          g_assert (g_date_get_julian (d) + 1 - first_day_of_year ==
                    g_date_get_day_of_year (d));

          if (m == G_DATE_DECEMBER && day == 31)
            g_assert_cmpint (g_date_get_day_of_year (d), ==, days_in_year);

          g_assert_cmpint (g_date_get_day_of_year (d), <=, days_in_year);
          g_assert_cmpint (g_date_get_monday_week_of_year (d), <=, monday_weeks_in_year);
          g_assert_cmpint (g_date_get_monday_week_of_year (d), >=, monday_week_of_year);

          if (g_date_get_weekday(d) == G_DATE_MONDAY)
            {
              g_assert_cmpint (g_date_get_monday_week_of_year (d) - monday_week_of_year, ==, 1);
              if ((m == G_DATE_JANUARY && day <= 4) ||
                  (m == G_DATE_DECEMBER && day >= 29))
                 g_assert_cmpint (g_date_get_iso8601_week_of_year (d), ==, 1);
              else
                g_assert_cmpint (g_date_get_iso8601_week_of_year (d) - iso8601_week_of_year, ==, 1);
            }
          else
            {
              g_assert_cmpint (g_date_get_monday_week_of_year(d) - monday_week_of_year, ==, 0);
              if (!(day == 1 && m == G_DATE_JANUARY))
                g_assert_cmpint (g_date_get_iso8601_week_of_year(d) - iso8601_week_of_year, ==, 0);
            }

          monday_week_of_year = g_date_get_monday_week_of_year (d);
          iso8601_week_of_year = g_date_get_iso8601_week_of_year (d);

          g_assert_cmpint (g_date_get_sunday_week_of_year (d), <=, sunday_weeks_in_year);
          g_assert_cmpint (g_date_get_sunday_week_of_year (d), >=, sunday_week_of_year);
          if (g_date_get_weekday(d) == G_DATE_SUNDAY)
            g_assert_cmpint (g_date_get_sunday_week_of_year (d) - sunday_week_of_year, ==, 1);
          else
            g_assert_cmpint (g_date_get_sunday_week_of_year (d) - sunday_week_of_year, ==, 0);

          sunday_week_of_year = g_date_get_sunday_week_of_year (d);

          g_assert_cmpint (g_date_compare (d, d), ==, 0);

          i = 1;
          while (i < 402) /* Need to get 400 year increments in */
            {
              tmp = *d;
              g_date_add_days (d, i);
              g_assert_cmpint (g_date_compare (d, &tmp), >, 0);
              g_date_subtract_days (d, i);
              g_assert_cmpint (g_date_get_day (d), ==, day);
              g_assert_cmpint (g_date_get_month (d), ==, m);
              g_assert_cmpint (g_date_get_year (d), ==, y);

              tmp = *d;
              g_date_add_months (d, i);
              g_assert_cmpint (g_date_compare (d, &tmp), >, 0);
              g_date_subtract_months (d, i);
              g_assert_cmpint (g_date_get_month (d), ==, m);
              g_assert_cmpint (g_date_get_year (d), ==, y);

              if (day < 29)
                g_assert_cmpint (g_date_get_day (d), ==, day);
              else
                g_date_set_day (d, day);

              tmp = *d;
              g_date_add_years (d, i);
              g_assert_cmpint (g_date_compare (d, &tmp), >, 0);
              g_date_subtract_years (d, i);
              g_assert_cmpint (g_date_get_month (d), ==, m);
              g_assert_cmpint (g_date_get_year (d), ==, y);

              if (m != 2 && day != 29)
                g_assert_cmpint (g_date_get_day (d), ==, day);
              else
                g_date_set_day (d, day); /* reset */

              i += 10;
            }

          j = g_date_get_julian (d);

          ++day;
        }
      ++m;
   }

  /* at this point, d is the last day of year y */
  g_date_set_dmy (&tmp, 1, 1, y + 1);
  g_assert_cmpint (j + 1, ==, g_date_get_julian (&tmp));

  g_date_add_days (&tmp, 1);
  g_assert_cmpint (j + 2, ==, g_date_get_julian (&tmp));
}

static void
test_clamp (void)
{
  GDate d1, d2, d, o;

  g_date_set_dmy (&d1, 1, 1, 1970);
  g_date_set_dmy (&d2, 1, 1, 1980);
  g_date_set_dmy (&d, 1, 1, 1);

  o = d;
  g_date_clamp (&o, NULL, NULL);
  g_assert (g_date_compare (&o, &d) == 0);

  g_date_clamp (&o,  &d1, &d2);
  g_assert (g_date_compare (&o, &d1) == 0);

  g_date_set_dmy (&o, 1, 1, 2000);

  g_date_clamp (&o,  &d1, &d2);
  g_assert (g_date_compare (&o, &d2) == 0);
}

static void
test_order (void)
{
  GDate d1, d2;

  g_date_set_dmy (&d1, 1, 1, 1970);
  g_date_set_dmy (&d2, 1, 1, 1980);

  g_assert (g_date_compare (&d1, &d2) == -1);
  g_date_order (&d2, &d1);
  g_assert (g_date_compare (&d1, &d2) == 1);
}

int
main (int argc, char** argv)
{
  gchar *path;
  gint i;

  /* Try to get all the leap year cases. */
  int check_years[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 98, 99, 100, 101, 102, 103, 397,
    398, 399, 400, 401, 402, 403, 404, 405, 406,
    1598, 1599, 1600, 1601, 1602, 1650, 1651,
    1897, 1898, 1899, 1900, 1901, 1902, 1903,
    1961, 1962, 1963, 1964, 1965, 1967,
    1968, 1969, 1970, 1971, 1972, 1973, 1974, 1975, 1976,
    1977, 1978, 1979, 1980, 1981, 1982, 1983, 1984, 1985,
    1986, 1987, 1988, 1989, 1990, 1991, 1992, 1993, 1994,
    1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003,
    2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012,
    3000, 3001, 3002, 3998, 3999, 4000, 4001, 4002, 4003
  };

  g_setenv ("LANG", "en_US.utf-8", TRUE);
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/date/basic", test_basic);
  g_test_add_func ("/date/empty", test_empty_constructor);
  g_test_add_func ("/date/dmy", test_dmy_constructor);
  g_test_add_func ("/date/julian", test_julian_constructor);
  g_test_add_func ("/date/dates", test_dates);
  g_test_add_func ("/date/parse", test_parse);
  g_test_add_func ("/date/clamp", test_clamp);
  g_test_add_func ("/date/order", test_order);
  for (i = 0; i < G_N_ELEMENTS (check_years); i++)
    {
      path = g_strdup_printf ("/date/year/%d", check_years[i]);
      g_test_add_data_func (path, GINT_TO_POINTER(check_years[i]), test_year);
      g_free (path);
    }

  return g_test_run ();
}


