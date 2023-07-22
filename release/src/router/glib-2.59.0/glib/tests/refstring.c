/* refstring.c: Reference counted strings
 *
 * Copyright 2018  Emmanuele Bassi
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <string.h>

/* test_refstring_base: Test the base API of GRefString */
static void
test_refstring_base (void)
{
  char *s = g_ref_string_new ("hello, world");

  g_test_message ("s = '%s' (%p)", s, s);
  g_assert_cmpint (strcmp (s, "hello, world"), ==, 0);
  g_assert_cmpint (strlen (s), ==, strlen ("hello, world"));
  g_assert_cmpuint (g_ref_string_length (s), ==, strlen ("hello, world"));

  g_assert_true (g_ref_string_acquire (s) == s);
  g_ref_string_release (s);

  g_ref_string_release (s);
}

/* test_refstring_length: Test the _len variant */
static void
test_refstring_length (void)
{
  char buf[] = {'h', 'e', 'l', 'l', 'o'}; /* no NUL */
  char *s = g_ref_string_new_len (buf, 5);

  g_assert_cmpstr (s, ==, "hello");
  g_assert_cmpint (strlen (s), ==, strlen ("hello"));
  g_assert_cmpuint (g_ref_string_length (s), ==, strlen ("hello"));
  g_ref_string_release (s);
}

/* test_refstring_length: Test the _len variant with no size set */
static void
test_refstring_length_auto (void)
{
  char *s = g_ref_string_new_len ("hello", -1);
  g_assert_cmpstr (s, ==, "hello");
  g_assert_cmpuint (g_ref_string_length (s), ==, strlen ("hello"));
  g_ref_string_release (s);
}

/* test_refstring_length_nuls: Test the _len variant */
static void
test_refstring_length_nuls (void)
{
  char buf[] = {'h', 'e', '\0', 'l', 'o'}; /* no NUL */
  char *s = g_ref_string_new_len (buf, 5);

  g_assert_cmpstr (s, ==, "he");
  g_assert_cmpint (memcmp (s, "he\0lo", 5), ==, 0);
  g_assert_cmpuint (g_ref_string_length (s), ==, 5);
  g_ref_string_release (s);
}

/* test_refstring_intern: Test the interning API of GRefString */
static void
test_refstring_intern (void)
{
  char *s = g_ref_string_new_intern ("hello, world");
  char *p;

  g_test_message ("s = '%s' (%p)", s, s);
  g_assert_cmpstr (s, ==, "hello, world");

  p = g_ref_string_new_intern ("hello, world");
  g_test_message ("p = s = '%s' (%p)", p, p);
  g_assert_true (s == p);

  g_test_message ("releasing p[%p] ('%s')", p, p);
  g_ref_string_release (p);

  p = g_ref_string_new_intern ("goodbye, world");
  g_test_message ("p = '%s' (%p)", p, p);
  g_assert_false (s == p);

  g_test_message ("releasing p[%p] ('%s')", p, p);
  g_ref_string_release (p);

  g_test_message ("releasing s[%p] ('%s')", s, s);
  g_ref_string_release (s);
}

int
main (int   argc,
      char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/refstring/base", test_refstring_base);
  g_test_add_func ("/refstring/length", test_refstring_length);
  g_test_add_func ("/refstring/length-auto", test_refstring_length_auto);
  g_test_add_func ("/refstring/length-nuls", test_refstring_length_nuls);
  g_test_add_func ("/refstring/intern", test_refstring_intern);

  return g_test_run ();
}

