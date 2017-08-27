/* gdatetime-tests.c
 *
 * Copyright (C) 2009-2010 Christian Hergert <chris@dronelabs.com>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#include <string.h>
#include <time.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <locale.h>

#define ASSERT_DATE(dt,y,m,d) G_STMT_START { \
  g_assert_cmpint ((y), ==, g_date_time_get_year ((dt))); \
  g_assert_cmpint ((m), ==, g_date_time_get_month ((dt))); \
  g_assert_cmpint ((d), ==, g_date_time_get_day_of_month ((dt))); \
} G_STMT_END
#define ASSERT_TIME(dt,H,M,S) G_STMT_START { \
  g_assert_cmpint ((H), ==, g_date_time_get_hour ((dt))); \
  g_assert_cmpint ((M), ==, g_date_time_get_minute ((dt))); \
  g_assert_cmpint ((S), ==, g_date_time_get_second ((dt))); \
} G_STMT_END

static void
get_localtime_tm (time_t     time_,
                  struct tm *retval)
{
#ifdef HAVE_LOCALTIME_R
  localtime_r (&time_, retval);
#else
  {
    struct tm *ptm = localtime (&time_);

    if (ptm == NULL)
      {
        /* Happens at least in Microsoft's C library if you pass a
         * negative time_t. Use 2000-01-01 as default date.
         */
#ifndef G_DISABLE_CHECKS
        g_return_if_fail_warning (G_LOG_DOMAIN, G_STRFUNC, "ptm != NULL");
#endif

        retval->tm_mon = 0;
        retval->tm_mday = 1;
        retval->tm_year = 100;
      }
    else
      memcpy ((void *) retval, (void *) ptm, sizeof (struct tm));
  }
#endif /* HAVE_LOCALTIME_R */
}

static void
test_GDateTime_now (void)
{
  GDateTime *dt;
  struct tm tm;

  memset (&tm, 0, sizeof (tm));
  get_localtime_tm (time (NULL), &tm);

  dt = g_date_time_new_now_local ();

  g_assert_cmpint (g_date_time_get_year (dt), ==, 1900 + tm.tm_year);
  g_assert_cmpint (g_date_time_get_month (dt), ==, 1 + tm.tm_mon);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, tm.tm_mday);
  g_assert_cmpint (g_date_time_get_hour (dt), ==, tm.tm_hour);
  g_assert_cmpint (g_date_time_get_minute (dt), ==, tm.tm_min);
  /* XXX we need some fuzzyness here */
  g_assert_cmpint (g_date_time_get_second (dt), >=, tm.tm_sec);

  g_date_time_unref (dt);
}

static void
test_GDateTime_new_from_unix (void)
{
  GDateTime *dt;
  struct tm  tm;
  time_t     t;

  memset (&tm, 0, sizeof (tm));
  t = time (NULL);
  get_localtime_tm (t, &tm);

  dt = g_date_time_new_from_unix_local (t);
  g_assert_cmpint (g_date_time_get_year (dt), ==, 1900 + tm.tm_year);
  g_assert_cmpint (g_date_time_get_month (dt), ==, 1 + tm.tm_mon);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, tm.tm_mday);
  g_assert_cmpint (g_date_time_get_hour (dt), ==, tm.tm_hour);
  g_assert_cmpint (g_date_time_get_minute (dt), ==, tm.tm_min);
  g_assert_cmpint (g_date_time_get_second (dt), ==, tm.tm_sec);
  g_date_time_unref (dt);

  memset (&tm, 0, sizeof (tm));
  tm.tm_year = 90;
  tm.tm_mday = 1;
  tm.tm_mon = 0;
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_isdst = -1;
  t = mktime (&tm);

  dt = g_date_time_new_from_unix_local (t);
  g_assert_cmpint (g_date_time_get_year (dt), ==, 1990);
  g_assert_cmpint (g_date_time_get_month (dt), ==, 1);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, 1);
  g_assert_cmpint (g_date_time_get_hour (dt), ==, 0);
  g_assert_cmpint (g_date_time_get_minute (dt), ==, 0);
  g_assert_cmpint (g_date_time_get_second (dt), ==, 0);
  g_date_time_unref (dt);
}

static void
test_GDateTime_invalid (void)
{
  GDateTime *dt;

  g_test_bug ("702674");

  dt = g_date_time_new_utc (2013, -2147483647, 31, 17, 15, 48);
  g_assert (dt == NULL);
}

static void
test_GDateTime_compare (void)
{
  GDateTime *dt1, *dt2;
  gint       i;

  dt1 = g_date_time_new_utc (2000, 1, 1, 0, 0, 0);

  for (i = 1; i < 2000; i++)
    {
      dt2 = g_date_time_new_utc (i, 12, 31, 0, 0, 0);
      g_assert_cmpint (1, ==, g_date_time_compare (dt1, dt2));
      g_date_time_unref (dt2);
    }

  dt2 = g_date_time_new_utc (1999, 12, 31, 23, 59, 59);
  g_assert_cmpint (1, ==, g_date_time_compare (dt1, dt2));
  g_date_time_unref (dt2);

  dt2 = g_date_time_new_utc (2000, 1, 1, 0, 0, 1);
  g_assert_cmpint (-1, ==, g_date_time_compare (dt1, dt2));
  g_date_time_unref (dt2);

  dt2 = g_date_time_new_utc (2000, 1, 1, 0, 0, 0);
  g_assert_cmpint (0, ==, g_date_time_compare (dt1, dt2));
  g_date_time_unref (dt2);
  g_date_time_unref (dt1);
}

static void
test_GDateTime_equal (void)
{
  GDateTime *dt1, *dt2;
  GTimeZone *tz;

  dt1 = g_date_time_new_local (2009, 10, 19, 0, 0, 0);
  dt2 = g_date_time_new_local (2009, 10, 19, 0, 0, 0);
  g_assert (g_date_time_equal (dt1, dt2));
  g_date_time_unref (dt1);
  g_date_time_unref (dt2);

  dt1 = g_date_time_new_local (2009, 10, 18, 0, 0, 0);
  dt2 = g_date_time_new_local (2009, 10, 19, 0, 0, 0);
  g_assert (!g_date_time_equal (dt1, dt2));
  g_date_time_unref (dt1);
  g_date_time_unref (dt2);

  /* UTC-0300 and not in DST */
  tz = g_time_zone_new ("-03:00");
  dt1 = g_date_time_new (tz, 2010, 5, 24,  8, 0, 0);
  g_time_zone_unref (tz);
  g_assert_cmpint (g_date_time_get_utc_offset (dt1) / G_USEC_PER_SEC, ==, (-3 * 3600));
  /* UTC */
  dt2 = g_date_time_new_utc (2010, 5, 24, 11, 0, 0);
  g_assert_cmpint (g_date_time_get_utc_offset (dt2), ==, 0);

  g_assert (g_date_time_equal (dt1, dt2));
  g_date_time_unref (dt1);

  /* America/Recife is in UTC-0300 */
#ifdef G_OS_UNIX
  tz = g_time_zone_new ("America/Recife");
#elif defined G_OS_WIN32
  tz = g_time_zone_new ("E. South America Standard Time");
#endif
  dt1 = g_date_time_new (tz, 2010, 5, 24,  8, 0, 0);
  g_time_zone_unref (tz);
  g_assert_cmpint (g_date_time_get_utc_offset (dt1) / G_USEC_PER_SEC, ==, (-3 * 3600));
  g_assert (g_date_time_equal (dt1, dt2));
  g_date_time_unref (dt1);
  g_date_time_unref (dt2);
}

static void
test_GDateTime_get_day_of_week (void)
{
  GDateTime *dt;

  dt = g_date_time_new_local (2009, 10, 19, 0, 0, 0);
  g_assert_cmpint (1, ==, g_date_time_get_day_of_week (dt));
  g_date_time_unref (dt);

  dt = g_date_time_new_local (2000, 10, 1, 0, 0, 0);
  g_assert_cmpint (7, ==, g_date_time_get_day_of_week (dt));
  g_date_time_unref (dt);
}

static void
test_GDateTime_get_day_of_month (void)
{
  GDateTime *dt;

  dt = g_date_time_new_local (2009, 10, 19, 0, 0, 0);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, 19);
  g_date_time_unref (dt);

  dt = g_date_time_new_local (1400, 3, 12, 0, 0, 0);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, 12);
  g_date_time_unref (dt);

  dt = g_date_time_new_local (1800, 12, 31, 0, 0, 0);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, 31);
  g_date_time_unref (dt);

  dt = g_date_time_new_local (1000, 1, 1, 0, 0, 0);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, 1);
  g_date_time_unref (dt);
}

static void
test_GDateTime_get_hour (void)
{
  GDateTime *dt;

  dt = g_date_time_new_utc (2009, 10, 19, 15, 13, 11);
  g_assert_cmpint (15, ==, g_date_time_get_hour (dt));
  g_date_time_unref (dt);

  dt = g_date_time_new_utc (100, 10, 19, 1, 0, 0);
  g_assert_cmpint (1, ==, g_date_time_get_hour (dt));
  g_date_time_unref (dt);

  dt = g_date_time_new_utc (100, 10, 19, 0, 0, 0);
  g_assert_cmpint (0, ==, g_date_time_get_hour (dt));
  g_date_time_unref (dt);

  dt = g_date_time_new_utc (100, 10, 1, 23, 59, 59);
  g_assert_cmpint (23, ==, g_date_time_get_hour (dt));
  g_date_time_unref (dt);
}

static void
test_GDateTime_get_microsecond (void)
{
  GTimeVal   tv;
  GDateTime *dt;

  g_get_current_time (&tv);
  dt = g_date_time_new_from_timeval_local (&tv);
  g_assert_cmpint (tv.tv_usec, ==, g_date_time_get_microsecond (dt));
  g_date_time_unref (dt);
}

static void
test_GDateTime_get_year (void)
{
  GDateTime *dt;

  dt = g_date_time_new_local (2009, 1, 1, 0, 0, 0);
  g_assert_cmpint (2009, ==, g_date_time_get_year (dt));
  g_date_time_unref (dt);

  dt = g_date_time_new_local (1, 1, 1, 0, 0, 0);
  g_assert_cmpint (1, ==, g_date_time_get_year (dt));
  g_date_time_unref (dt);

  dt = g_date_time_new_local (13, 1, 1, 0, 0, 0);
  g_assert_cmpint (13, ==, g_date_time_get_year (dt));
  g_date_time_unref (dt);

  dt = g_date_time_new_local (3000, 1, 1, 0, 0, 0);
  g_assert_cmpint (3000, ==, g_date_time_get_year (dt));
  g_date_time_unref (dt);
}

static void
test_GDateTime_hash (void)
{
  GHashTable *h;

  h = g_hash_table_new_full (g_date_time_hash, g_date_time_equal,
                             (GDestroyNotify)g_date_time_unref,
                             NULL);
  g_hash_table_insert (h, g_date_time_new_now_local (), NULL);
  g_hash_table_remove_all (h);
  g_hash_table_destroy (h);
}

static void
test_GDateTime_new_from_timeval (void)
{
  GDateTime *dt;
  GTimeVal   tv, tv2;

  g_get_current_time (&tv);
  dt = g_date_time_new_from_timeval_local (&tv);

  if (g_test_verbose ())
    g_print ("\nDT%04d-%02d-%02dT%02d:%02d:%02d%s\n",
             g_date_time_get_year (dt),
             g_date_time_get_month (dt),
             g_date_time_get_day_of_month (dt),
             g_date_time_get_hour (dt),
             g_date_time_get_minute (dt),
             g_date_time_get_second (dt),
             g_date_time_get_timezone_abbreviation (dt));

  g_date_time_to_timeval (dt, &tv2);
  g_assert_cmpint (tv.tv_sec, ==, tv2.tv_sec);
  g_assert_cmpint (tv.tv_usec, ==, tv2.tv_usec);
  g_date_time_unref (dt);
}

static void
test_GDateTime_new_from_timeval_utc (void)
{
  GDateTime *dt;
  GTimeVal   tv, tv2;

  g_get_current_time (&tv);
  dt = g_date_time_new_from_timeval_utc (&tv);

  if (g_test_verbose ())
    g_print ("\nDT%04d-%02d-%02dT%02d:%02d:%02d%s\n",
             g_date_time_get_year (dt),
             g_date_time_get_month (dt),
             g_date_time_get_day_of_month (dt),
             g_date_time_get_hour (dt),
             g_date_time_get_minute (dt),
             g_date_time_get_second (dt),
             g_date_time_get_timezone_abbreviation (dt));

  g_date_time_to_timeval (dt, &tv2);
  g_assert_cmpint (tv.tv_sec, ==, tv2.tv_sec);
  g_assert_cmpint (tv.tv_usec, ==, tv2.tv_usec);
  g_date_time_unref (dt);
}

static void
test_GDateTime_to_unix (void)
{
  GDateTime *dt;
  time_t     t;

  t = time (NULL);
  dt = g_date_time_new_from_unix_local (t);
  g_assert_cmpint (g_date_time_to_unix (dt), ==, t);
  g_date_time_unref (dt);
}

static void
test_GDateTime_add_years (void)
{
  GDateTime *dt, *dt2;

  dt = g_date_time_new_local (2009, 10, 19, 0, 0, 0);
  dt2 = g_date_time_add_years (dt, 1);
  g_assert_cmpint (2010, ==, g_date_time_get_year (dt2));
  g_date_time_unref (dt);
  g_date_time_unref (dt2);
}

static void
test_GDateTime_add_months (void)
{
#define TEST_ADD_MONTHS(y,m,d,a,ny,nm,nd) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_utc (y, m, d, 0, 0, 0); \
  dt2 = g_date_time_add_months (dt, a); \
  ASSERT_DATE (dt2, ny, nm, nd); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_MONTHS (2009, 12, 31,    1, 2010, 1, 31);
  TEST_ADD_MONTHS (2009, 12, 31,    1, 2010, 1, 31);
  TEST_ADD_MONTHS (2009,  6, 15,    1, 2009, 7, 15);
  TEST_ADD_MONTHS (1400,  3,  1,    1, 1400, 4,  1);
  TEST_ADD_MONTHS (1400,  1, 31,    1, 1400, 2, 28);
  TEST_ADD_MONTHS (1400,  1, 31, 7200, 2000, 1, 31);
  TEST_ADD_MONTHS (2008,  2, 29,   12, 2009, 2, 28);
  TEST_ADD_MONTHS (2000,  8, 16,   -5, 2000, 3, 16);
  TEST_ADD_MONTHS (2000,  8, 16,  -12, 1999, 8, 16);
  TEST_ADD_MONTHS (2011,  2,  1,  -13, 2010, 1,  1);
  TEST_ADD_MONTHS (1776,  7,  4, 1200, 1876, 7,  4);
}

static void
test_GDateTime_add_days (void)
{
#define TEST_ADD_DAYS(y,m,d,a,ny,nm,nd) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_local (y, m, d, 0, 0, 0); \
  dt2 = g_date_time_add_days (dt, a); \
  g_assert_cmpint (ny, ==, g_date_time_get_year (dt2)); \
  g_assert_cmpint (nm, ==, g_date_time_get_month (dt2)); \
  g_assert_cmpint (nd, ==, g_date_time_get_day_of_month (dt2)); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_DAYS (2009, 1, 31, 1, 2009, 2, 1);
  TEST_ADD_DAYS (2009, 2, 1, -1, 2009, 1, 31);
  TEST_ADD_DAYS (2008, 2, 28, 1, 2008, 2, 29);
  TEST_ADD_DAYS (2008, 12, 31, 1, 2009, 1, 1);
  TEST_ADD_DAYS (1, 1, 1, 1, 1, 1, 2);
  TEST_ADD_DAYS (1955, 5, 24, 10, 1955, 6, 3);
  TEST_ADD_DAYS (1955, 5, 24, -10, 1955, 5, 14);
}

static void
test_GDateTime_add_weeks (void)
{
#define TEST_ADD_WEEKS(y,m,d,a,ny,nm,nd) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_local (y, m, d, 0, 0, 0); \
  dt2 = g_date_time_add_weeks (dt, a); \
  g_assert_cmpint (ny, ==, g_date_time_get_year (dt2)); \
  g_assert_cmpint (nm, ==, g_date_time_get_month (dt2)); \
  g_assert_cmpint (nd, ==, g_date_time_get_day_of_month (dt2)); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_WEEKS (2009, 1, 1, 1, 2009, 1, 8);
  TEST_ADD_WEEKS (2009, 8, 30, 1, 2009, 9, 6);
  TEST_ADD_WEEKS (2009, 12, 31, 1, 2010, 1, 7);
  TEST_ADD_WEEKS (2009, 1, 1, -1, 2008, 12, 25);
}

static void
test_GDateTime_add_hours (void)
{
#define TEST_ADD_HOURS(y,m,d,h,mi,s,a,ny,nm,nd,nh,nmi,ns) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_utc (y, m, d, h, mi, s); \
  dt2 = g_date_time_add_hours (dt, a); \
  g_assert_cmpint (ny, ==, g_date_time_get_year (dt2)); \
  g_assert_cmpint (nm, ==, g_date_time_get_month (dt2)); \
  g_assert_cmpint (nd, ==, g_date_time_get_day_of_month (dt2)); \
  g_assert_cmpint (nh, ==, g_date_time_get_hour (dt2)); \
  g_assert_cmpint (nmi, ==, g_date_time_get_minute (dt2)); \
  g_assert_cmpint (ns, ==, g_date_time_get_second (dt2)); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_HOURS (2009,  1,  1,  0, 0, 0, 1, 2009, 1, 1, 1, 0, 0);
  TEST_ADD_HOURS (2008, 12, 31, 23, 0, 0, 1, 2009, 1, 1, 0, 0, 0);
}

static void
test_GDateTime_add_full (void)
{
#define TEST_ADD_FULL(y,m,d,h,mi,s,ay,am,ad,ah,ami,as,ny,nm,nd,nh,nmi,ns) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_utc (y, m, d, h, mi, s); \
  dt2 = g_date_time_add_full (dt, ay, am, ad, ah, ami, as); \
  g_assert_cmpint (ny, ==, g_date_time_get_year (dt2)); \
  g_assert_cmpint (nm, ==, g_date_time_get_month (dt2)); \
  g_assert_cmpint (nd, ==, g_date_time_get_day_of_month (dt2)); \
  g_assert_cmpint (nh, ==, g_date_time_get_hour (dt2)); \
  g_assert_cmpint (nmi, ==, g_date_time_get_minute (dt2)); \
  g_assert_cmpint (ns, ==, g_date_time_get_second (dt2)); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_FULL (2009, 10, 21,  0,  0, 0,
                    1,  1,  1,  1,  1, 1,
                 2010, 11, 22,  1,  1, 1);
  TEST_ADD_FULL (2000,  1,  1,  1,  1, 1,
                    0,  1,  0,  0,  0, 0,
                 2000,  2,  1,  1,  1, 1);
  TEST_ADD_FULL (2000,  1,  1,  0,  0, 0,
                   -1,  1,  0,  0,  0, 0,
                 1999,  2,  1,  0,  0, 0);
  TEST_ADD_FULL (2010, 10, 31,  0,  0, 0,
                    0,  4,  0,  0,  0, 0,
                 2011,  2, 28,  0,  0, 0);
  TEST_ADD_FULL (2010,  8, 25, 22, 45, 0,
                    0,  1,  6,  1, 25, 0,
                 2010, 10,  2,  0, 10, 0);
}

static void
test_GDateTime_add_minutes (void)
{
#define TEST_ADD_MINUTES(i,o) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_local (2000, 1, 1, 0, 0, 0); \
  dt2 = g_date_time_add_minutes (dt, i); \
  g_assert_cmpint (o, ==, g_date_time_get_minute (dt2)); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_MINUTES (60, 0);
  TEST_ADD_MINUTES (100, 40);
  TEST_ADD_MINUTES (5, 5);
  TEST_ADD_MINUTES (1441, 1);
  TEST_ADD_MINUTES (-1441, 59);
}

static void
test_GDateTime_add_seconds (void)
{
#define TEST_ADD_SECONDS(i,o) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_local (2000, 1, 1, 0, 0, 0); \
  dt2 = g_date_time_add_seconds (dt, i); \
  g_assert_cmpint (o, ==, g_date_time_get_second (dt2)); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_SECONDS (1, 1);
  TEST_ADD_SECONDS (60, 0);
  TEST_ADD_SECONDS (61, 1);
  TEST_ADD_SECONDS (120, 0);
  TEST_ADD_SECONDS (-61, 59);
  TEST_ADD_SECONDS (86401, 1);
  TEST_ADD_SECONDS (-86401, 59);
  TEST_ADD_SECONDS (-31, 29);
  TEST_ADD_SECONDS (13, 13);
}

static void
test_GDateTime_diff (void)
{
#define TEST_DIFF(y,m,d,y2,m2,d2,u) G_STMT_START { \
  GDateTime *dt1, *dt2; \
  GTimeSpan  ts = 0; \
  dt1 = g_date_time_new_utc (y, m, d, 0, 0, 0); \
  dt2 = g_date_time_new_utc (y2, m2, d2, 0, 0, 0); \
  ts = g_date_time_difference (dt2, dt1); \
  g_assert_cmpint (ts, ==, u); \
  g_date_time_unref (dt1); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_DIFF (2009, 1, 1, 2009, 2, 1, G_TIME_SPAN_DAY * 31);
  TEST_DIFF (2009, 1, 1, 2010, 1, 1, G_TIME_SPAN_DAY * 365);
  TEST_DIFF (2008, 2, 28, 2008, 2, 29, G_TIME_SPAN_DAY);
  TEST_DIFF (2008, 2, 29, 2008, 2, 28, -G_TIME_SPAN_DAY);

  /* TODO: Add usec tests */
}

static void
test_GDateTime_get_minute (void)
{
  GDateTime *dt;

  dt = g_date_time_new_utc (2009, 12, 1, 1, 31, 0);
  g_assert_cmpint (31, ==, g_date_time_get_minute (dt));
  g_date_time_unref (dt);
}

static void
test_GDateTime_get_month (void)
{
  GDateTime *dt;

  dt = g_date_time_new_utc (2009, 12, 1, 1, 31, 0);
  g_assert_cmpint (12, ==, g_date_time_get_month (dt));
  g_date_time_unref (dt);
}

static void
test_GDateTime_get_second (void)
{
  GDateTime *dt;

  dt = g_date_time_new_utc (2009, 12, 1, 1, 31, 44);
  g_assert_cmpint (44, ==, g_date_time_get_second (dt));
  g_date_time_unref (dt);
}

static void
test_GDateTime_new_full (void)
{
  GTimeZone *tz;
  GDateTime *dt;

  dt = g_date_time_new_utc (2009, 12, 11, 12, 11, 10);
  g_assert_cmpint (2009, ==, g_date_time_get_year (dt));
  g_assert_cmpint (12, ==, g_date_time_get_month (dt));
  g_assert_cmpint (11, ==, g_date_time_get_day_of_month (dt));
  g_assert_cmpint (12, ==, g_date_time_get_hour (dt));
  g_assert_cmpint (11, ==, g_date_time_get_minute (dt));
  g_assert_cmpint (10, ==, g_date_time_get_second (dt));
  g_date_time_unref (dt);

#ifdef G_OS_UNIX
  tz = g_time_zone_new ("America/Recife");
#elif defined G_OS_WIN32
  tz = g_time_zone_new ("E. South America Standard Time");
#endif
  dt = g_date_time_new (tz, 2010, 5, 24, 8, 4, 0);
  g_time_zone_unref (tz);
  g_assert_cmpint (2010, ==, g_date_time_get_year (dt));
  g_assert_cmpint (5, ==, g_date_time_get_month (dt));
  g_assert_cmpint (24, ==, g_date_time_get_day_of_month (dt));
  g_assert_cmpint (8, ==, g_date_time_get_hour (dt));
  g_assert_cmpint (4, ==, g_date_time_get_minute (dt));
  g_assert_cmpint (0, ==, g_date_time_get_second (dt));
#ifdef G_OS_UNIX
  g_assert_cmpstr ("BRT", ==, g_date_time_get_timezone_abbreviation (dt));
#elif defined G_OS_WIN32
  g_assert_cmpstr ("E. South America Standard Time", ==,
                    g_date_time_get_timezone_abbreviation (dt));
#endif
  g_assert (!g_date_time_is_daylight_savings (dt));
  g_date_time_unref (dt);
}

static void
test_GDateTime_now_utc (void)
{
  GDateTime *dt;
  time_t     t;
  struct tm  tm;

  t = time (NULL);
#ifdef HAVE_GMTIME_R
  gmtime_r (&t, &tm);
#else
  {
    struct tm *tmp = gmtime (&t);
    /* Assume gmtime() can't fail as we got t from time(NULL). (Note
     * that on Windows, gmtime() *is* MT-safe, it uses a thread-local
     * buffer.)
     */
    memcpy (&tm, tmp, sizeof (struct tm));
  }
#endif
  dt = g_date_time_new_now_utc ();
  g_assert_cmpint (tm.tm_year + 1900, ==, g_date_time_get_year (dt));
  g_assert_cmpint (tm.tm_mon + 1, ==, g_date_time_get_month (dt));
  g_assert_cmpint (tm.tm_mday, ==, g_date_time_get_day_of_month (dt));
  g_assert_cmpint (tm.tm_hour, ==, g_date_time_get_hour (dt));
  g_assert_cmpint (tm.tm_min, ==, g_date_time_get_minute (dt));
  g_assert_cmpint (tm.tm_sec, ==, g_date_time_get_second (dt));
  g_date_time_unref (dt);
}

static void
test_GDateTime_new_from_unix_utc (void)
{
  GDateTime *dt;
  gint64 t;

  t = g_get_real_time ();

#if 0
  dt = g_date_time_new_from_unix_utc (t);
  g_assert (dt == NULL);
#endif

  t = t / 1e6;  /* oops, this was microseconds */

  dt = g_date_time_new_from_unix_utc (t);
  g_assert (dt != NULL);

  g_assert (dt == g_date_time_ref (dt));
  g_date_time_unref (dt);
  g_assert_cmpint (g_date_time_to_unix (dt), ==, t);
  g_date_time_unref (dt);
}

static void
test_GDateTime_get_utc_offset (void)
{
#if defined (HAVE_STRUCT_TM_TM_GMTOFF) || defined (HAVE_STRUCT_TM___TM_GMTOFF)
  GDateTime *dt;
  GTimeSpan ts;
  struct tm tm;

  memset (&tm, 0, sizeof (tm));
  get_localtime_tm (time (NULL), &tm);

  dt = g_date_time_new_now_local ();
  ts = g_date_time_get_utc_offset (dt);
#ifdef HAVE_STRUCT_TM_TM_GMTOFF
  g_assert_cmpint (ts, ==, (tm.tm_gmtoff * G_TIME_SPAN_SECOND));
#endif
#ifdef HAVE_STRUCT_TM___TM_GMTOFF
  g_assert_cmpint (ts, ==, (tm.__tm_gmtoff * G_TIME_SPAN_SECOND));
#endif
  g_date_time_unref (dt);
#endif
}

static void
test_GDateTime_to_timeval (void)
{
  GTimeVal tv1, tv2;
  GDateTime *dt;

  memset (&tv1, 0, sizeof (tv1));
  memset (&tv2, 0, sizeof (tv2));

  g_get_current_time (&tv1);
  dt = g_date_time_new_from_timeval_local (&tv1);
  g_date_time_to_timeval (dt, &tv2);
  g_assert_cmpint (tv1.tv_sec, ==, tv2.tv_sec);
  g_assert_cmpint (tv1.tv_usec, ==, tv2.tv_usec);
  g_date_time_unref (dt);
}

static void
test_GDateTime_to_local (void)
{
  GDateTime *utc, *now, *dt;

  utc = g_date_time_new_now_utc ();
  now = g_date_time_new_now_local ();
  dt = g_date_time_to_local (utc);

  g_assert_cmpint (g_date_time_get_year (now), ==, g_date_time_get_year (dt));
  g_assert_cmpint (g_date_time_get_month (now), ==, g_date_time_get_month (dt));
  g_assert_cmpint (g_date_time_get_day_of_month (now), ==, g_date_time_get_day_of_month (dt));
  g_assert_cmpint (g_date_time_get_hour (now), ==, g_date_time_get_hour (dt));
  g_assert_cmpint (g_date_time_get_minute (now), ==, g_date_time_get_minute (dt));
  g_assert_cmpint (g_date_time_get_second (now), ==, g_date_time_get_second (dt));

  g_date_time_unref (now);
  g_date_time_unref (utc);
  g_date_time_unref (dt);
}

static void
test_GDateTime_to_utc (void)
{
  GDateTime *dt, *dt2;
  time_t     t;
  struct tm  tm;

  t = time (NULL);
#ifdef HAVE_GMTIME_R
  gmtime_r (&t, &tm);
#else
  {
    struct tm *tmp = gmtime (&t);
    memcpy (&tm, tmp, sizeof (struct tm));
  }
#endif
  dt2 = g_date_time_new_from_unix_local (t);
  dt = g_date_time_to_utc (dt2);
  g_assert_cmpint (tm.tm_year + 1900, ==, g_date_time_get_year (dt));
  g_assert_cmpint (tm.tm_mon + 1, ==, g_date_time_get_month (dt));
  g_assert_cmpint (tm.tm_mday, ==, g_date_time_get_day_of_month (dt));
  g_assert_cmpint (tm.tm_hour, ==, g_date_time_get_hour (dt));
  g_assert_cmpint (tm.tm_min, ==, g_date_time_get_minute (dt));
  g_assert_cmpint (tm.tm_sec, ==, g_date_time_get_second (dt));
  g_date_time_unref (dt);
  g_date_time_unref (dt2);
}

static void
test_GDateTime_get_day_of_year (void)
{
#define TEST_DAY_OF_YEAR(y,m,d,o)                       G_STMT_START {  \
  GDateTime *__dt = g_date_time_new_local ((y), (m), (d), 0, 0, 0);     \
  g_assert_cmpint ((o), ==, g_date_time_get_day_of_year (__dt));        \
  g_date_time_unref (__dt);                             } G_STMT_END

  TEST_DAY_OF_YEAR (2009, 1, 1, 1);
  TEST_DAY_OF_YEAR (2009, 2, 1, 32);
  TEST_DAY_OF_YEAR (2009, 8, 16, 228);
  TEST_DAY_OF_YEAR (2008, 8, 16, 229);
}

static void
test_GDateTime_printf (void)
{
/* 64 seems big, but one zoneinfo file, Factory, has an abbreviation
 * that long, and it will cause the test to fail if dst isn't big
 * enough.
 */
  gchar dst[64];
  struct tm tt;
  time_t t;

#define TEST_PRINTF(f,o)                        G_STMT_START {  \
GDateTime *__dt = g_date_time_new_local (2009, 10, 24, 0, 0, 0);\
  gchar *__p = g_date_time_format (__dt, (f));                  \
  g_assert_cmpstr (__p, ==, (o));                               \
  g_date_time_unref (__dt);                                     \
  g_free (__p);                                 } G_STMT_END

#define TEST_PRINTF_DATE(y,m,d,f,o)             G_STMT_START {  \
  GDateTime *dt = g_date_time_new_local (y, m, d, 0, 0, 0);     \
  gchar *p = g_date_time_format (dt, (f));                      \
  g_assert_cmpstr (p, ==, (o));                                 \
  g_date_time_unref (dt);                                       \
  g_free (p);                                   } G_STMT_END

#define TEST_PRINTF_TIME(h,m,s,f,o)             G_STMT_START { \
  GDateTime *dt = g_date_time_new_local (2009, 10, 24, (h), (m), (s)); \
  gchar *p = g_date_time_format (dt, (f));                      \
  g_assert_cmpstr (p, ==, (o));                                 \
  g_date_time_unref (dt);                                       \
  g_free (p);                                   } G_STMT_END

  /*
   * This is a little helper to make sure we can compare timezones to
   * that of the generated timezone.
   */
  t = time (NULL);
  memset (&tt, 0, sizeof(tt));
  get_localtime_tm (t, &tt);
  tt.tm_year = 2009 - 1900;
  tt.tm_mon = 9; /* 0 indexed */
  tt.tm_mday = 24;
  t = mktime (&tt);
  memset (&tt, 0, sizeof(tt));
  get_localtime_tm (t, &tt);
  strftime (dst, sizeof(dst), "%Z", &tt);

  /* get current time_t for 20090924 in the local timezone */
  tt.tm_sec = 0;
  tt.tm_min = 0;
  tt.tm_hour = 0;
  t = mktime (&tt);

  TEST_PRINTF ("%a", "Sat");
  TEST_PRINTF ("%A", "Saturday");
  TEST_PRINTF ("%b", "Oct");
  TEST_PRINTF ("%B", "October");
  TEST_PRINTF ("%d", "24");
  TEST_PRINTF_DATE (2009, 1, 1, "%d", "01");
  TEST_PRINTF ("%e", "24"); // fixme
  TEST_PRINTF ("%h", "Oct");
  TEST_PRINTF ("%H", "00");
  TEST_PRINTF_TIME (15, 0, 0, "%H", "15");
  TEST_PRINTF ("%I", "12");
  TEST_PRINTF_TIME (12, 0, 0, "%I", "12");
  TEST_PRINTF_TIME (15, 0, 0, "%I", "03");
  TEST_PRINTF ("%j", "297");
  TEST_PRINTF ("%k", " 0");
  TEST_PRINTF_TIME (13, 13, 13, "%k", "13");
  TEST_PRINTF ("%l", "12");
  TEST_PRINTF_TIME (12, 0, 0, "%I", "12");
  TEST_PRINTF_TIME (13, 13, 13, "%l", " 1");
  TEST_PRINTF_TIME (10, 13, 13, "%l", "10");
  TEST_PRINTF ("%m", "10");
  TEST_PRINTF ("%M", "00");
  TEST_PRINTF ("%p", "AM");
  TEST_PRINTF_TIME (13, 13, 13, "%p", "PM");
  TEST_PRINTF ("%P", "am");
  TEST_PRINTF_TIME (13, 13, 13, "%P", "pm");
  TEST_PRINTF ("%r", "12:00:00 AM");
  TEST_PRINTF_TIME (13, 13, 13, "%r", "01:13:13 PM");
  TEST_PRINTF ("%R", "00:00");
  TEST_PRINTF_TIME (13, 13, 31, "%R", "13:13");
  TEST_PRINTF ("%S", "00");
  TEST_PRINTF ("%t", "	");
  TEST_PRINTF ("%u", "6");
  TEST_PRINTF ("%x", "10/24/09");
  TEST_PRINTF ("%X", "00:00:00");
  TEST_PRINTF_TIME (13, 14, 15, "%X", "13:14:15");
  TEST_PRINTF ("%y", "09");
  TEST_PRINTF ("%Y", "2009");
  TEST_PRINTF ("%%", "%");
  TEST_PRINTF ("%", "");
  TEST_PRINTF ("%9", NULL);
#ifdef G_OS_UNIX
  TEST_PRINTF ("%Z", dst);
#elif defined G_OS_WIN32
  TEST_PRINTF ("%Z", "Pacific Standard Time");
#endif
}

static void
test_non_utf8_printf (void)
{
  gchar *oldlocale;

  oldlocale = g_strdup (setlocale (LC_ALL, NULL));
  setlocale (LC_ALL, "ja_JP.eucjp");
  if (strstr (setlocale (LC_ALL, NULL), "ja_JP") == NULL)
    {
      g_test_message ("locale ja_JP.eucjp not available, skipping non-UTF8 tests");
      g_free (oldlocale);
      return;
    }
  if (g_get_charset (NULL))
    {
      g_test_message ("locale ja_JP.eucjp may be available, but glib seems to think that it's equivalent to UTF-8, skipping non-UTF-8 tests.");
      g_test_message ("This is a known issue on Darwin");
      setlocale (LC_ALL, oldlocale);
      g_free (oldlocale);
      return;
    }

  /* These are the outputs that ja_JP.UTF-8 generates; if everything
   * is working then ja_JP.eucjp should generate the same.
   */
  TEST_PRINTF ("%a", "\345\234\237");
  TEST_PRINTF ("%A", "\345\234\237\346\233\234\346\227\245");
#ifndef HAVE_CARBON /* OSX just returns the number */
  TEST_PRINTF ("%b", "10\346\234\210");
#endif
  TEST_PRINTF ("%B", "10\346\234\210");
  TEST_PRINTF ("%d", "24");
  TEST_PRINTF_DATE (2009, 1, 1, "%d", "01");
  TEST_PRINTF ("%e", "24"); // fixme
#ifndef HAVE_CARBON /* OSX just returns the number */
  TEST_PRINTF ("%h", "10\346\234\210");
#endif
  TEST_PRINTF ("%H", "00");
  TEST_PRINTF_TIME (15, 0, 0, "%H", "15");
  TEST_PRINTF ("%I", "12");
  TEST_PRINTF_TIME (12, 0, 0, "%I", "12");
  TEST_PRINTF_TIME (15, 0, 0, "%I", "03");
  TEST_PRINTF ("%j", "297");
  TEST_PRINTF ("%k", " 0");
  TEST_PRINTF_TIME (13, 13, 13, "%k", "13");
  TEST_PRINTF ("%l", "12");
  TEST_PRINTF_TIME (12, 0, 0, "%I", "12");
  TEST_PRINTF_TIME (13, 13, 13, "%l", " 1");
  TEST_PRINTF_TIME (10, 13, 13, "%l", "10");
  TEST_PRINTF ("%m", "10");
  TEST_PRINTF ("%M", "00");
#ifndef HAVE_CARBON /* OSX returns latin "AM", not japanese */
  TEST_PRINTF ("%p", "\345\215\210\345\211\215");
  TEST_PRINTF_TIME (13, 13, 13, "%p", "\345\215\210\345\276\214");
  TEST_PRINTF ("%P", "\345\215\210\345\211\215");
  TEST_PRINTF_TIME (13, 13, 13, "%P", "\345\215\210\345\276\214");
  TEST_PRINTF ("%r", "\345\215\210\345\211\21512\346\231\20200\345\210\20600\347\247\222");
  TEST_PRINTF_TIME (13, 13, 13, "%r", "\345\215\210\345\276\21401\346\231\20213\345\210\20613\347\247\222");
#endif
  TEST_PRINTF ("%R", "00:00");
  TEST_PRINTF_TIME (13, 13, 31, "%R", "13:13");
  TEST_PRINTF ("%S", "00");
  TEST_PRINTF ("%t", "	");
  TEST_PRINTF ("%u", "6");
#ifndef HAVE_CARBON /* OSX returns YYYY/MM/DD in ASCII */
  TEST_PRINTF ("%x", "2009\345\271\26410\346\234\21024\346\227\245");
#endif
  TEST_PRINTF ("%X", "00\346\231\20200\345\210\20600\347\247\222");
  TEST_PRINTF_TIME (13, 14, 15, "%X", "13\346\231\20214\345\210\20615\347\247\222");
  TEST_PRINTF ("%y", "09");
  TEST_PRINTF ("%Y", "2009");
  TEST_PRINTF ("%%", "%");
  TEST_PRINTF ("%", "");
  TEST_PRINTF ("%9", NULL);

  setlocale (LC_ALL, oldlocale);
  g_free (oldlocale);
}

static void
test_modifiers (void)
{
  gchar *oldlocale;

  TEST_PRINTF_DATE (2009, 1,  1,  "%d", "01");
  TEST_PRINTF_DATE (2009, 1,  1, "%_d", " 1");
  TEST_PRINTF_DATE (2009, 1,  1, "%-d", "1");
  TEST_PRINTF_DATE (2009, 1,  1, "%0d", "01");
  TEST_PRINTF_DATE (2009, 1, 21,  "%d", "21");
  TEST_PRINTF_DATE (2009, 1, 21, "%_d", "21");
  TEST_PRINTF_DATE (2009, 1, 21, "%-d", "21");
  TEST_PRINTF_DATE (2009, 1, 21, "%0d", "21");

  TEST_PRINTF_DATE (2009, 1,  1,  "%e", " 1");
  TEST_PRINTF_DATE (2009, 1,  1, "%_e", " 1");
  TEST_PRINTF_DATE (2009, 1,  1, "%-e", "1");
  TEST_PRINTF_DATE (2009, 1,  1, "%0e", "01");
  TEST_PRINTF_DATE (2009, 1, 21,  "%e", "21");
  TEST_PRINTF_DATE (2009, 1, 21, "%_e", "21");
  TEST_PRINTF_DATE (2009, 1, 21, "%-e", "21");
  TEST_PRINTF_DATE (2009, 1, 21, "%0e", "21");

  TEST_PRINTF_TIME ( 1, 0, 0,  "%H", "01");
  TEST_PRINTF_TIME ( 1, 0, 0, "%_H", " 1");
  TEST_PRINTF_TIME ( 1, 0, 0, "%-H", "1");
  TEST_PRINTF_TIME ( 1, 0, 0, "%0H", "01");
  TEST_PRINTF_TIME (21, 0, 0,  "%H", "21");
  TEST_PRINTF_TIME (21, 0, 0, "%_H", "21");
  TEST_PRINTF_TIME (21, 0, 0, "%-H", "21");
  TEST_PRINTF_TIME (21, 0, 0, "%0H", "21");

  TEST_PRINTF_TIME ( 1, 0, 0,  "%I", "01");
  TEST_PRINTF_TIME ( 1, 0, 0, "%_I", " 1");
  TEST_PRINTF_TIME ( 1, 0, 0, "%-I", "1");
  TEST_PRINTF_TIME ( 1, 0, 0, "%0I", "01");
  TEST_PRINTF_TIME (23, 0, 0,  "%I", "11");
  TEST_PRINTF_TIME (23, 0, 0, "%_I", "11");
  TEST_PRINTF_TIME (23, 0, 0, "%-I", "11");
  TEST_PRINTF_TIME (23, 0, 0, "%0I", "11");

  TEST_PRINTF_TIME ( 1, 0, 0,  "%k", " 1");
  TEST_PRINTF_TIME ( 1, 0, 0, "%_k", " 1");
  TEST_PRINTF_TIME ( 1, 0, 0, "%-k", "1");
  TEST_PRINTF_TIME ( 1, 0, 0, "%0k", "01");

  oldlocale = g_strdup (setlocale (LC_ALL, NULL));
  setlocale (LC_ALL, "fa_IR.utf-8");
  if (strstr (setlocale (LC_ALL, NULL), "fa_IR") != NULL)
    {
      TEST_PRINTF_TIME (23, 0, 0, "%OH", "\333\262\333\263");    /* '23' */
      TEST_PRINTF_TIME (23, 0, 0, "%OI", "\333\261\333\261");    /* '11' */
      TEST_PRINTF_TIME (23, 0, 0, "%OM", "\333\260\333\260");    /* '00' */

      TEST_PRINTF_DATE (2011, 7, 1, "%Om", "\333\260\333\267");  /* '07' */
      TEST_PRINTF_DATE (2011, 7, 1, "%0Om", "\333\260\333\267"); /* '07' */
      TEST_PRINTF_DATE (2011, 7, 1, "%-Om", "\333\267");         /* '7' */
      TEST_PRINTF_DATE (2011, 7, 1, "%_Om", " \333\267");        /* ' 7' */
    }
  else
    g_test_message ("locale fa_IR not available, skipping O modifier tests");
  setlocale (LC_ALL, oldlocale);
  g_free (oldlocale);
}

static void
test_GDateTime_dst (void)
{
  GDateTime *dt1, *dt2;
  GTimeZone *tz;

  /* this date has the DST state set for Europe/London */
#ifdef G_OS_UNIX
  tz = g_time_zone_new ("Europe/London");
#elif defined G_OS_WIN32
  tz = g_time_zone_new ("GMT Standard Time");
#endif
  dt1 = g_date_time_new (tz, 2009, 8, 15, 3, 0, 1);
  g_assert (g_date_time_is_daylight_savings (dt1));
  g_assert_cmpint (g_date_time_get_utc_offset (dt1) / G_USEC_PER_SEC, ==, 3600);
  g_assert_cmpint (g_date_time_get_hour (dt1), ==, 3);

  /* add 6 months to clear the DST flag but keep the same time */
  dt2 = g_date_time_add_months (dt1, 6);
  g_assert (!g_date_time_is_daylight_savings (dt2));
  g_assert_cmpint (g_date_time_get_utc_offset (dt2) / G_USEC_PER_SEC, ==, 0);
  g_assert_cmpint (g_date_time_get_hour (dt2), ==, 3);

  g_date_time_unref (dt2);
  g_date_time_unref (dt1);

  /* now do the reverse: start with a non-DST state and move to DST */
  dt1 = g_date_time_new (tz, 2009, 2, 15, 2, 0, 1);
  g_assert (!g_date_time_is_daylight_savings (dt1));
  g_assert_cmpint (g_date_time_get_hour (dt1), ==, 2);

  dt2 = g_date_time_add_months (dt1, 6);
  g_assert (g_date_time_is_daylight_savings (dt2));
  g_assert_cmpint (g_date_time_get_hour (dt2), ==, 2);

  g_date_time_unref (dt2);
  g_date_time_unref (dt1);
  g_time_zone_unref (tz);
}

static inline gboolean
is_leap_year (gint year)
{
  g_assert (1 <= year && year <= 9999);

  return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}

static inline gint
days_in_month (gint year, gint month)
{
  const gint table[2][13] = {
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
  };

  g_assert (1 <= month && month <= 12);

  return table[is_leap_year (year)][month];
}

static void
test_all_dates (void)
{
  gint year, month, day;
  GTimeZone *timezone;
  gint64 unix_time;
  gint day_of_year;
  gint week_year;
  gint week_num;
  gint weekday;

  /* save some time by hanging on to this. */
  timezone = g_time_zone_new_utc ();

  unix_time = G_GINT64_CONSTANT(-62135596800);

  /* 0001-01-01 is 0001-W01-1 */
  week_year = 1;
  week_num = 1;
  weekday = 1;


  /* The calendar makes a full cycle every 400 years, so we could
   * theoretically just test years 1 through 400.  That assumes that our
   * software has no bugs, so probably we should just test them all. :)
   */
  for (year = 1; year <= 9999; year++)
    {
      day_of_year = 1;

      for (month = 1; month <= 12; month++)
        for (day = 1; day <= days_in_month (year, month); day++)
          {
            GDateTime *dt;

            dt = g_date_time_new (timezone, year, month, day, 0, 0, 0);

#if 0
            g_print ("%04d-%02d-%02d = %04d-W%02d-%d = %04d-%03d\n",
                     year, month, day,
                     week_year, week_num, weekday,
                     year, day_of_year);
#endif

            /* sanity check */
            if G_UNLIKELY (g_date_time_get_year (dt) != year ||
                           g_date_time_get_month (dt) != month ||
                           g_date_time_get_day_of_month (dt) != day)
              g_error ("%04d-%02d-%02d comes out as %04d-%02d-%02d",
                       year, month, day,
                       g_date_time_get_year (dt),
                       g_date_time_get_month (dt),
                       g_date_time_get_day_of_month (dt));

            if G_UNLIKELY (g_date_time_get_week_numbering_year (dt) != week_year ||
                           g_date_time_get_week_of_year (dt) != week_num ||
                           g_date_time_get_day_of_week (dt) != weekday)
              g_error ("%04d-%02d-%02d should be %04d-W%02d-%d but "
                       "comes out as %04d-W%02d-%d", year, month, day,
                       week_year, week_num, weekday,
                       g_date_time_get_week_numbering_year (dt),
                       g_date_time_get_week_of_year (dt),
                       g_date_time_get_day_of_week (dt));

            if G_UNLIKELY (g_date_time_to_unix (dt) != unix_time)
              g_error ("%04d-%02d-%02d 00:00:00 UTC should have unix time %"
                       G_GINT64_FORMAT " but comes out as %"G_GINT64_FORMAT,
                       year, month, day, unix_time, g_date_time_to_unix (dt));

            if G_UNLIKELY (g_date_time_get_day_of_year (dt) != day_of_year)
              g_error ("%04d-%02d-%02d should be day of year %d"
                       " but comes out as %d", year, month, day,
                       day_of_year, g_date_time_get_day_of_year (dt));

            if G_UNLIKELY (g_date_time_get_hour (dt) != 0 ||
                           g_date_time_get_minute (dt) != 0 ||
                           g_date_time_get_seconds (dt) != 0)
              g_error ("%04d-%02d-%02d 00:00:00 UTC comes out "
                       "as %02d:%02d:%02.6f", year, month, day,
                       g_date_time_get_hour (dt),
                       g_date_time_get_minute (dt),
                       g_date_time_get_seconds (dt));
            /* done */

            /* add 24 hours to unix time */
            unix_time += 24 * 60 * 60;

            /* move day of year forward */
            day_of_year++;

            /* move the week date forward */
            if (++weekday == 8)
              {
                weekday = 1; /* Sunday -> Monday */

                /* NOTE: year/month/day is the final day of the week we
                 * just finished.
                 *
                 * If we just finished the last week of last year then
                 * we are definitely starting the first week of this
                 * year.
                 *
                 * Otherwise, if we're still in this year, but Sunday
                 * fell on or after December 28 then December 29, 30, 31
                 * could be days within the next year's first year.
                 */
                if (year != week_year || (month == 12 && day >= 28))
                  {
                    /* first week of the new year */
                    week_num = 1;
                    week_year++;
                  }
                else
                  week_num++;
              }

            g_date_time_unref (dt);
          }
    }

  g_time_zone_unref (timezone);
}

static void
test_z (void)
{
  GTimeZone *tz;
  GDateTime *dt;
  gchar *p;

  g_test_bug ("642935");

  tz = g_time_zone_new ("-08:00");
  dt = g_date_time_new (tz, 1, 1, 1, 0, 0, 0);

  p = g_date_time_format (dt, "%z");
  g_assert_cmpstr (p, ==, "-0800");
  g_free (p);

  p = g_date_time_format (dt, "%:z");
  g_assert_cmpstr (p, ==, "-08:00");
  g_free (p);

  p = g_date_time_format (dt, "%::z");
  g_assert_cmpstr (p, ==, "-08:00:00");
  g_free (p);

  p = g_date_time_format (dt, "%:::z");
  g_assert_cmpstr (p, ==, "-08");
  g_free (p);

  g_date_time_unref (dt);
  g_time_zone_unref (tz);

  tz = g_time_zone_new ("+00:00");
  dt = g_date_time_new (tz, 1, 1, 1, 0, 0, 0);
  p = g_date_time_format (dt, "%:::z");
  g_assert_cmpstr (p, ==, "+00");
  g_free (p);
  g_date_time_unref (dt);
  g_time_zone_unref (tz);

  tz = g_time_zone_new ("+08:23");
  dt = g_date_time_new (tz, 1, 1, 1, 0, 0, 0);
  p = g_date_time_format (dt, "%:::z");
  g_assert_cmpstr (p, ==, "+08:23");
  g_free (p);
  g_date_time_unref (dt);
  g_time_zone_unref (tz);

  tz = g_time_zone_new ("+08:23:45");
  dt = g_date_time_new (tz, 1, 1, 1, 0, 0, 0);
  p = g_date_time_format (dt, "%:::z");
  g_assert_cmpstr (p, ==, "+08:23:45");
  g_free (p);
  g_date_time_unref (dt);
  g_time_zone_unref (tz);
}

static void
test_strftime (void)
{
#ifdef __linux__
#define TEST_FORMAT \
  "a%a A%A b%b B%B c%c C%C d%d e%e F%F g%g G%G h%h H%H I%I j%j m%m M%M " \
  "n%n p%p r%r R%R S%S t%t T%T u%u V%V w%w x%x X%X y%y Y%Y z%z Z%Z %%"
  time_t t;

  /* 127997 is prime, 1315005118 is now */
  for (t = 0; t < 1315005118; t += 127997)
    {
      GDateTime *date_time;
      gchar c_str[1000];
      gchar *dt_str;

      date_time = g_date_time_new_from_unix_local (t);
      dt_str = g_date_time_format (date_time, TEST_FORMAT);
      strftime (c_str, sizeof c_str, TEST_FORMAT, localtime (&t));
      g_assert_cmpstr (c_str, ==, dt_str);
      g_date_time_unref (date_time);
      g_free (dt_str);
    }
#endif
}

static void
test_find_interval (void)
{
  GTimeZone *tz;
  GDateTime *dt;
  gint64 u;
  gint i1, i2;

#ifdef G_OS_UNIX
  tz = g_time_zone_new ("Canada/Eastern");
#elif defined G_OS_WIN32
  tz = g_time_zone_new ("Eastern Standard Time");
#endif
  dt = g_date_time_new_utc (2010, 11, 7, 1, 30, 0);
  u = g_date_time_to_unix (dt);

  i1 = g_time_zone_find_interval (tz, G_TIME_TYPE_STANDARD, u);
  i2 = g_time_zone_find_interval (tz, G_TIME_TYPE_DAYLIGHT, u);

  g_assert_cmpint (i1, !=, i2);

  g_date_time_unref (dt);

  dt = g_date_time_new_utc (2010, 3, 14, 2, 0, 0);
  u = g_date_time_to_unix (dt);

  i1 = g_time_zone_find_interval (tz, G_TIME_TYPE_STANDARD, u);
  g_assert_cmpint (i1, ==, -1);

  g_date_time_unref (dt);
  g_time_zone_unref (tz);
}

static void
test_adjust_time (void)
{
  GTimeZone *tz;
  GDateTime *dt;
  gint64 u, u2;
  gint i1, i2;

#ifdef G_OS_UNIX
  tz = g_time_zone_new ("Canada/Eastern");
#elif defined G_OS_WIN32
  tz = g_time_zone_new ("Eastern Standard Time");
#endif
  dt = g_date_time_new_utc (2010, 11, 7, 1, 30, 0);
  u = g_date_time_to_unix (dt);
  u2 = u;

  i1 = g_time_zone_find_interval (tz, G_TIME_TYPE_DAYLIGHT, u);
  i2 = g_time_zone_adjust_time (tz, G_TIME_TYPE_DAYLIGHT, &u2);

  g_assert_cmpint (i1, ==, i2);
  g_assert (u == u2);

  g_date_time_unref (dt);

  dt = g_date_time_new_utc (2010, 3, 14, 2, 30, 0);
  u2 = g_date_time_to_unix (dt);
  g_date_time_unref (dt);

  dt = g_date_time_new_utc (2010, 3, 14, 3, 0, 0);
  u = g_date_time_to_unix (dt);
  g_date_time_unref (dt);

  i1 = g_time_zone_adjust_time (tz, G_TIME_TYPE_DAYLIGHT, &u2);
  g_assert (u == u2);

  g_time_zone_unref (tz);
}

static void
test_no_header (void)
{
  GTimeZone *tz;

  tz = g_time_zone_new ("blabla");

  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 0), ==, "UTC");
  g_assert_cmpint (g_time_zone_get_offset (tz, 0), ==, 0);
  g_assert (!g_time_zone_is_dst (tz, 0));

  g_time_zone_unref (tz);
}

static void
test_posix_parse (void)
{
  GTimeZone *tz;
  GDateTime *gdt1, *gdt2;

  tz = g_time_zone_new ("PST");
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 0), ==, "UTC");
  g_assert_cmpint (g_time_zone_get_offset (tz, 0), ==, 0);
  g_assert (!g_time_zone_is_dst (tz, 0));
  g_time_zone_unref (tz);

  tz = g_time_zone_new ("PST8");
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 0), ==, "PST");
  g_assert_cmpint (g_time_zone_get_offset (tz, 0), ==, - 8 * 3600);
  g_assert (!g_time_zone_is_dst (tz, 0));
  g_time_zone_unref (tz);

/* This fails rules_from_identifier on Unix (though not on Windows)
 * but passes anyway because PST8PDT is a zone name.
 */
  tz = g_time_zone_new ("PST8PDT");
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 0), ==, "PST");
  g_assert_cmpint (g_time_zone_get_offset (tz, 0), ==, - 8 * 3600);
  g_assert (!g_time_zone_is_dst (tz, 0));
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 1), ==, "PDT");
  g_assert_cmpint (g_time_zone_get_offset (tz, 1), ==,- 7 * 3600);
  g_assert (g_time_zone_is_dst (tz, 1));
  g_time_zone_unref (tz);

  tz = g_time_zone_new ("PST8PDT6:32:15");
#ifdef G_OS_WIN32
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 0), ==, "PST");
  g_assert_cmpint (g_time_zone_get_offset (tz, 0), ==, - 8 * 3600);
  g_assert (!g_time_zone_is_dst (tz, 0));
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 1), ==, "PDT");
  g_assert_cmpint (g_time_zone_get_offset (tz, 1), ==, - 6 * 3600 - 32 *60 - 15);
  g_assert (g_time_zone_is_dst (tz, 1));
  gdt1 = g_date_time_new (tz, 2012, 12, 6, 11, 15, 23.0);
  gdt2 = g_date_time_new (tz, 2012, 6, 6, 11, 15, 23.0);
  g_assert (!g_date_time_is_daylight_savings (gdt1));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt1) /  1000000, ==, -28800);
  g_assert (g_date_time_is_daylight_savings (gdt2));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt2) / 1000000, ==, -23535);
  g_date_time_unref (gdt1);
  g_date_time_unref (gdt2);
#else
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 0), ==, "UTC");
  g_assert_cmpint (g_time_zone_get_offset (tz, 0), ==, 0);
  g_assert (!g_time_zone_is_dst (tz, 0));
#endif
  g_time_zone_unref (tz);

  tz = g_time_zone_new ("NZST-12:00:00NZDT-13:00:00,M10.1.0,M3.3.0");
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 0), ==, "NZST");
  g_assert_cmpint (g_time_zone_get_offset (tz, 0), ==, 12 * 3600);
  g_assert (!g_time_zone_is_dst (tz, 0));
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 1), ==, "NZDT");
  g_assert_cmpint (g_time_zone_get_offset (tz, 1), ==, 13 * 3600);
  g_assert (g_time_zone_is_dst (tz, 1));
  gdt1 = g_date_time_new (tz, 2012, 3, 18, 0, 15, 23.0);
  gdt2 = g_date_time_new (tz, 2012, 3, 18, 3, 15, 23.0);
  g_assert (g_date_time_is_daylight_savings (gdt1));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt1) / 1000000, ==, 46800);
  g_assert (!g_date_time_is_daylight_savings (gdt2));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt2) / 1000000, ==, 43200);
  g_date_time_unref (gdt1);
  g_date_time_unref (gdt2);
  gdt1 = g_date_time_new (tz, 2012, 10, 7, 3, 15, 23.0);
  gdt2 = g_date_time_new (tz, 2012, 10, 7, 1, 15, 23.0);
  g_assert (g_date_time_is_daylight_savings (gdt1));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt1) / 1000000, ==, 46800);
  g_assert (!g_date_time_is_daylight_savings (gdt2));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt2) / 1000000, ==, 43200);
  g_date_time_unref (gdt1);
  g_date_time_unref (gdt2);
  g_time_zone_unref (tz);

  tz = g_time_zone_new ("NZST-12:00:00NZDT-13:00:00,280,77");
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 0), ==, "NZST");
  g_assert_cmpint (g_time_zone_get_offset (tz, 0), ==, 12 * 3600);
  g_assert (!g_time_zone_is_dst (tz, 0));
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 1), ==, "NZDT");
  g_assert_cmpint (g_time_zone_get_offset (tz, 1), ==, 13 * 3600);
  g_assert (g_time_zone_is_dst (tz, 1));
  gdt1 = g_date_time_new (tz, 2012, 3, 18, 0, 15, 23.0);
  gdt2 = g_date_time_new (tz, 2012, 3, 18, 3, 15, 23.0);
  g_assert (g_date_time_is_daylight_savings (gdt1));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt1) / 1000000, ==, 46800);
  g_assert (!g_date_time_is_daylight_savings (gdt2));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt2) / 1000000, ==, 43200);
  g_date_time_unref (gdt1);
  g_date_time_unref (gdt2);
  gdt1 = g_date_time_new (tz, 2012, 10, 7, 3, 15, 23.0);
  gdt2 = g_date_time_new (tz, 2012, 10, 7, 1, 15, 23.0);
  g_assert (g_date_time_is_daylight_savings (gdt1));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt1) / 1000000, ==, 46800);
  g_assert (!g_date_time_is_daylight_savings (gdt2));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt2) / 1000000, ==, 43200);
  g_date_time_unref (gdt1);
  g_date_time_unref (gdt2);
  g_time_zone_unref (tz);

  tz = g_time_zone_new ("NZST-12:00:00NZDT-13:00:00,J279,J76");
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 0), ==, "NZST");
  g_assert_cmpint (g_time_zone_get_offset (tz, 0), ==, 12 * 3600);
  g_assert (!g_time_zone_is_dst (tz, 0));
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 1), ==, "NZDT");
  g_assert_cmpint (g_time_zone_get_offset (tz, 1), ==, 13 * 3600);
  g_assert (g_time_zone_is_dst (tz, 1));
  gdt1 = g_date_time_new (tz, 2012, 3, 18, 0, 15, 23.0);
  gdt2 = g_date_time_new (tz, 2012, 3, 18, 3, 15, 23.0);
  g_assert (g_date_time_is_daylight_savings (gdt1));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt1) / 1000000, ==, 46800);
  g_assert (!g_date_time_is_daylight_savings (gdt2));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt2) / 1000000, ==, 43200);
  g_date_time_unref (gdt1);
  g_date_time_unref (gdt2);
  gdt1 = g_date_time_new (tz, 2012, 10, 7, 3, 15, 23.0);
  gdt2 = g_date_time_new (tz, 2012, 10, 7, 1, 15, 23.0);
  g_assert (g_date_time_is_daylight_savings (gdt1));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt1) / 1000000, ==, 46800);
  g_assert (!g_date_time_is_daylight_savings (gdt2));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt2) / 1000000, ==, 43200);
  g_date_time_unref (gdt1);
  g_date_time_unref (gdt2);
  g_time_zone_unref (tz);

  tz = g_time_zone_new ("NZST-12:00:00NZDT-13:00:00,M10.1.0/07:00,M3.3.0/07:00");
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 0), ==, "NZST");
  g_assert_cmpint (g_time_zone_get_offset (tz, 0), ==, 12 * 3600);
  g_assert (!g_time_zone_is_dst (tz, 0));
  g_assert_cmpstr (g_time_zone_get_abbreviation (tz, 1), ==, "NZDT");
  g_assert_cmpint (g_time_zone_get_offset (tz, 1), ==, 13 * 3600);
  g_assert (g_time_zone_is_dst (tz, 1));
  gdt1 = g_date_time_new (tz, 2012, 3, 18, 5, 15, 23.0);
  gdt2 = g_date_time_new (tz, 2012, 3, 18, 8, 15, 23.0);
  g_assert (g_date_time_is_daylight_savings (gdt1));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt1) / 1000000, ==, 46800);
  g_assert (!g_date_time_is_daylight_savings (gdt2));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt2) / 1000000, ==, 43200);
  g_date_time_unref (gdt1);
  g_date_time_unref (gdt2);
  gdt1 = g_date_time_new (tz, 2012, 10, 7, 8, 15, 23.0);
  gdt2 = g_date_time_new (tz, 2012, 10, 7, 6, 15, 23.0);
  g_assert (g_date_time_is_daylight_savings (gdt1));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt1) / 1000000, ==, 46800);
  g_assert (!g_date_time_is_daylight_savings (gdt2));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt2) / 1000000, ==, 43200);
  g_date_time_unref (gdt1);
  g_date_time_unref (gdt2);
  gdt1 = g_date_time_new (tz, 1902, 10, 7, 8, 15, 23.0);
  gdt2 = g_date_time_new (tz, 1902, 10, 7, 6, 15, 23.0);
  g_assert (!g_date_time_is_daylight_savings (gdt1));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt1) / 1000000, ==, 43200);
  g_assert (!g_date_time_is_daylight_savings (gdt2));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt2) / 1000000, ==, 43200);
  g_date_time_unref (gdt1);
  g_date_time_unref (gdt2);
  gdt1 = g_date_time_new (tz, 2142, 10, 7, 8, 15, 23.0);
  gdt2 = g_date_time_new (tz, 2142, 10, 7, 6, 15, 23.0);
  g_assert (g_date_time_is_daylight_savings (gdt1));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt1) / 1000000, ==, 46800);
  g_assert (!g_date_time_is_daylight_savings (gdt2));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt2) / 1000000, ==, 43200);
  g_date_time_unref (gdt1);
  g_date_time_unref (gdt2);
  gdt1 = g_date_time_new (tz, 3212, 10, 7, 8, 15, 23.0);
  gdt2 = g_date_time_new (tz, 3212, 10, 7, 6, 15, 23.0);
  g_assert (!g_date_time_is_daylight_savings (gdt1));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt1) / 1000000, ==, 43200);
  g_assert (!g_date_time_is_daylight_savings (gdt2));
  g_assert_cmpint (g_date_time_get_utc_offset (gdt2) / 1000000, ==, 43200);
  g_date_time_unref (gdt1);
  g_date_time_unref (gdt2);
  g_time_zone_unref (tz);
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("http://bugzilla.gnome.org/");

  /* GDateTime Tests */

  g_test_add_func ("/GDateTime/invalid", test_GDateTime_invalid);
  g_test_add_func ("/GDateTime/add_days", test_GDateTime_add_days);
  g_test_add_func ("/GDateTime/add_full", test_GDateTime_add_full);
  g_test_add_func ("/GDateTime/add_hours", test_GDateTime_add_hours);
  g_test_add_func ("/GDateTime/add_minutes", test_GDateTime_add_minutes);
  g_test_add_func ("/GDateTime/add_months", test_GDateTime_add_months);
  g_test_add_func ("/GDateTime/add_seconds", test_GDateTime_add_seconds);
  g_test_add_func ("/GDateTime/add_weeks", test_GDateTime_add_weeks);
  g_test_add_func ("/GDateTime/add_years", test_GDateTime_add_years);
  g_test_add_func ("/GDateTime/compare", test_GDateTime_compare);
  g_test_add_func ("/GDateTime/diff", test_GDateTime_diff);
  g_test_add_func ("/GDateTime/equal", test_GDateTime_equal);
  g_test_add_func ("/GDateTime/get_day_of_week", test_GDateTime_get_day_of_week);
  g_test_add_func ("/GDateTime/get_day_of_month", test_GDateTime_get_day_of_month);
  g_test_add_func ("/GDateTime/get_day_of_year", test_GDateTime_get_day_of_year);
  g_test_add_func ("/GDateTime/get_hour", test_GDateTime_get_hour);
  g_test_add_func ("/GDateTime/get_microsecond", test_GDateTime_get_microsecond);
  g_test_add_func ("/GDateTime/get_minute", test_GDateTime_get_minute);
  g_test_add_func ("/GDateTime/get_month", test_GDateTime_get_month);
  g_test_add_func ("/GDateTime/get_second", test_GDateTime_get_second);
  g_test_add_func ("/GDateTime/get_utc_offset", test_GDateTime_get_utc_offset);
  g_test_add_func ("/GDateTime/get_year", test_GDateTime_get_year);
  g_test_add_func ("/GDateTime/hash", test_GDateTime_hash);
  g_test_add_func ("/GDateTime/new_from_unix", test_GDateTime_new_from_unix);
  g_test_add_func ("/GDateTime/new_from_unix_utc", test_GDateTime_new_from_unix_utc);
  g_test_add_func ("/GDateTime/new_from_timeval", test_GDateTime_new_from_timeval);
  g_test_add_func ("/GDateTime/new_from_timeval_utc", test_GDateTime_new_from_timeval_utc);
  g_test_add_func ("/GDateTime/new_full", test_GDateTime_new_full);
  g_test_add_func ("/GDateTime/now", test_GDateTime_now);
  g_test_add_func ("/GDateTime/printf", test_GDateTime_printf);
  g_test_add_func ("/GDateTime/non_utf8_printf", test_non_utf8_printf);
  g_test_add_func ("/GDateTime/strftime", test_strftime);
  g_test_add_func ("/GDateTime/modifiers", test_modifiers);
  g_test_add_func ("/GDateTime/to_local", test_GDateTime_to_local);
  g_test_add_func ("/GDateTime/to_unix", test_GDateTime_to_unix);
  g_test_add_func ("/GDateTime/to_timeval", test_GDateTime_to_timeval);
  g_test_add_func ("/GDateTime/to_utc", test_GDateTime_to_utc);
  g_test_add_func ("/GDateTime/now_utc", test_GDateTime_now_utc);
  g_test_add_func ("/GDateTime/dst", test_GDateTime_dst);
  g_test_add_func ("/GDateTime/test_z", test_z);
  g_test_add_func ("/GDateTime/test-all-dates", test_all_dates);
  g_test_add_func ("/GTimeZone/find-interval", test_find_interval);
  g_test_add_func ("/GTimeZone/adjust-time", test_adjust_time);
  g_test_add_func ("/GTimeZone/no-header", test_no_header);
  g_test_add_func ("/GTimeZone/posix-parse", test_posix_parse);

  return g_test_run ();
}
