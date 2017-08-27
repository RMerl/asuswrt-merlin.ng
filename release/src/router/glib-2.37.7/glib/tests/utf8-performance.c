/* GLIB - Library of useful routines for C programming
 *
 * Copyright (C) 2010 Mikhail Zabaluev <mikhail.zabaluev@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>

#include <glib.h>

#define NUM_ITERATIONS 500000

static const char str_ascii[] =
    "The quick brown fox jumps over the lazy dog";

static const gchar str_latin1[] =
    "Zwölf Boxkämpfer jagen Viktor quer über den großen Sylter Deich";

/* Energizing GOELRO-talk in Russian, used by KDE */
static const char str_cyrillic[] =
    "Широкая электрификация южных губерний даст мощный толчок подъёму "
    "сельского хозяйства.";

/* First sentence from the Wikipedia article:
 * http://zh.wikipedia.org/w/index.php?title=%E6%B1%89%E5%AD%97&oldid=13053137 */
static const char str_chinese[] =
    "漢字，亦稱中文字、中国字，在台灣又被稱為國字，是漢字文化圈廣泛使用的一種文字，屬於表意文字的詞素音節文字";

typedef int (* GrindFunc) (const char *, gsize);

static int
grind_get_char (const char *str, gsize len)
{
  gunichar acc = 0;
  int i;
  for (i = 0; i < NUM_ITERATIONS; i++)
    {
      const char *p = str;
      while (*p) {
	acc += g_utf8_get_char (p);
	p = g_utf8_next_char (p);
      }
    }
  return acc;
}

static int
grind_get_char_validated (const char *str, gsize len)
{
  gunichar acc = 0;
  int i;
  for (i = 0; i < NUM_ITERATIONS; i++)
    {
      const char *p = str;
      while (*p) {
	acc += g_utf8_get_char_validated (p, -1);
	p = g_utf8_next_char (p);
      }
    }
  return acc;
}

static int
grind_utf8_to_ucs4 (const char *str, gsize len)
{
  int i;
  for (i = 0; i < NUM_ITERATIONS; i++)
    {
      gunichar *ustr;
      ustr = g_utf8_to_ucs4 (str, -1, NULL, NULL, NULL);
      g_free (ustr);
    }
  return 0;
}

static int
grind_get_char_backwards (const char *str, gsize len)
{
  gunichar acc = 0;
  int i;
  for (i = 0; i < NUM_ITERATIONS; i++)
    {
      const char *p = str + len;
      do
	{
	  p = g_utf8_prev_char (p);
	  acc += g_utf8_get_char (p);
        }
      while (p != str);
    }
  return acc;
}

static int
grind_utf8_to_ucs4_sized (const char *str, gsize len)
{
  int i;
  for (i = 0; i < NUM_ITERATIONS; i++)
    {
      gunichar *ustr;
      ustr = g_utf8_to_ucs4 (str, len, NULL, NULL, NULL);
      g_free (ustr);
    }
  return 0;
}

static int
grind_utf8_to_ucs4_fast (const char *str, gsize len)
{
  int i;
  for (i = 0; i < NUM_ITERATIONS; i++)
    {
      gunichar *ustr;
      ustr = g_utf8_to_ucs4_fast (str, -1, NULL);
      g_free (ustr);
    }
  return 0;
}

static int
grind_utf8_to_ucs4_fast_sized (const char *str, gsize len)
{
  int i;
  for (i = 0; i < NUM_ITERATIONS; i++)
    {
      gunichar *ustr;
      ustr = g_utf8_to_ucs4_fast (str, len, NULL);
      g_free (ustr);
    }
  return 0;
}

static void
perform_for (GrindFunc grind_func, const char *str, const char *label)
{
  gsize len;
  gulong bytes_ground;
  gdouble time_elapsed;
  gdouble result;

  len = strlen (str);
  bytes_ground = (gulong) len * NUM_ITERATIONS;

  g_test_timer_start ();

  grind_func (str, len);

  time_elapsed = g_test_timer_elapsed ();

  result = ((gdouble) bytes_ground / time_elapsed) * 1.0e-6;

  g_test_maximized_result (result, "%-9s %6.1f MB/s", label, result);
}

static void
perform (gconstpointer data)
{
  GrindFunc grind_func = (GrindFunc) data;

  if (!g_test_perf ())
    return;

  perform_for (grind_func, str_ascii, "ASCII:");
  perform_for (grind_func, str_latin1, "Latin-1:");
  perform_for (grind_func, str_cyrillic, "Cyrillic:");
  perform_for (grind_func, str_chinese, "Chinese:");
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  if (g_test_perf ())
    {
      g_test_add_data_func ("/utf8/perf/get_char", grind_get_char, perform);
      g_test_add_data_func ("/utf8/perf/get_char-backwards", grind_get_char_backwards, perform);
      g_test_add_data_func ("/utf8/perf/get_char_validated", grind_get_char_validated, perform);
      g_test_add_data_func ("/utf8/perf/utf8_to_ucs4", grind_utf8_to_ucs4, perform);
      g_test_add_data_func ("/utf8/perf/utf8_to_ucs4-sized", grind_utf8_to_ucs4_sized, perform);
      g_test_add_data_func ("/utf8/perf/utf8_to_ucs4_fast", grind_utf8_to_ucs4_fast, perform);
      g_test_add_data_func ("/utf8/perf/utf8_to_ucs4_fast-sized", grind_utf8_to_ucs4_fast_sized, perform);
    }

  return g_test_run ();
}
