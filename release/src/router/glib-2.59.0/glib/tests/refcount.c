/* refcount.c: Tests for reference counting types
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

#include <stdlib.h>
#include <glib.h>

/* test_grefcount: test the behavior of the grefcount API */
static void
test_grefcount (void)
{
  grefcount a, b;

  /* init(a): 1 */
  g_ref_count_init (&a);
  if (g_test_verbose ())
    g_test_message ("init(a) := %d\n", (int) a);
  g_assert_true (g_ref_count_compare (&a, 1));

  /* inc(a): 2 */
  g_ref_count_inc (&a);
  if (g_test_verbose ())
    g_test_message ("inc(a) := %d\n", (int) a);
  g_assert_false (g_ref_count_compare (&a, 1));
  g_assert_false (g_ref_count_compare (&a, G_MAXINT));

  /* b = a = 2 */
  b = a;
  if (g_test_verbose ())
    g_test_message ("a := %d, b := %d\n", (int) a, (int) b);

  /* inc(a): 3 */
  g_ref_count_inc (&a);
  if (g_test_verbose ())
    g_test_message ("inc(a) := %d\n", (int) a);

  /* dec(b) = 1 */
  if (g_test_verbose ())
    g_test_message ("dec(b) := %d + 1\n", (int) b);
  g_assert_false (g_ref_count_dec (&b));

  /* dec(a) = 2 */
  if (g_test_verbose ())
    g_test_message ("dec(a) := %d + 1\n", (int) a);
  g_assert_false (g_ref_count_dec (&a));

  /* dec(b) = 0 */
  if (g_test_verbose ())
    g_test_message ("dec(b) := %d + 1\n", (int) b);
  g_assert_true (g_ref_count_dec (&b));

  /* dec(a) = 1 */
  if (g_test_verbose ())
    g_test_message ("dec(a) := %d + 1\n", (int) a);
  g_assert_false (g_ref_count_dec (&a));

  /* dec(a) = 0 */
  if (g_test_verbose ())
    g_test_message ("dec(a) := %d + 1\n", (int) a);
  g_assert_true (g_ref_count_dec (&a));
}

/* test_grefcount_saturation: Saturating a grefcount counter
 * does not cause an overflow; additionally, if we're building
 * with checks enabled, it'll cause a warning
 */
static void
test_grefcount_saturation (void)
{
  if (g_test_subprocess ())
    {
      grefcount a;

      /* We're breaking abstraction here for convenience */
      a = G_MININT + 1;

      g_ref_count_inc (&a);
      g_assert_true (a == G_MININT);

      g_ref_count_inc (&a);
      g_assert_true (a == G_MININT);

      exit (0);
    }

  g_test_trap_subprocess (NULL, 0, 0);

#ifndef G_DISABLE_CHECKS
  /* Ensure that we got a warning when building with checks; the
   * test will fail because of the critical warning being caught
   * by GTest
   */
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*saturation*");
#else
  /* With checks disabled we don't get any warning */
  g_test_trap_assert_passed ();
#endif
}

/* test_gatomicrefcount: test the behavior of the gatomicrefcount API */
static void
test_gatomicrefcount (void)
{
  gatomicrefcount a, b;

  /* init(a): 1 */
  g_atomic_ref_count_init (&a);
  if (g_test_verbose ())
    g_test_message ("init(a) := %d\n", (int) a);
  g_assert_true (g_atomic_ref_count_compare (&a, 1));

  /* inc(a): 2 */
  g_atomic_ref_count_inc (&a);
  if (g_test_verbose ())
    g_test_message ("inc(a) := %d\n", (int) a);
  g_assert_false (g_atomic_ref_count_compare (&a, 1));
  g_assert_false (g_atomic_ref_count_compare (&a, G_MAXINT));

  /* b = a = 2 */
  b = a;
  if (g_test_verbose ())
    g_test_message ("a := %d, b := %d\n", (int) a, (int) b);

  /* inc(a): 3 */
  g_atomic_ref_count_inc (&a);
  if (g_test_verbose ())
    g_test_message ("inc(a) := %d\n", (int) a);

  /* dec(b) = 1 */
  if (g_test_verbose ())
    g_test_message ("dec(b) := %d + 1\n", (int) b);
  g_assert_false (g_atomic_ref_count_dec (&b));

  /* dec(a) = 2 */
  if (g_test_verbose ())
    g_test_message ("dec(a) := %d + 1\n", (int) a);
  g_assert_false (g_atomic_ref_count_dec (&a));

  /* dec(b) = 0 */
  if (g_test_verbose ())
    g_test_message ("dec(b) := %d + 1\n", (int) b);
  g_assert_true (g_atomic_ref_count_dec (&b));

  /* dec(a) = 1 */
  if (g_test_verbose ())
    g_test_message ("dec(a) := %d + 1\n", (int) a);
  g_assert_false (g_atomic_ref_count_dec (&a));

  /* dec(a) = 0 */
  if (g_test_verbose ())
    g_test_message ("dec(a) := %d + 1\n", (int) a);
  g_assert_true (g_atomic_ref_count_dec (&a));
}

/* test_grefcount_saturation: Saturating a gatomicrefcount counter
 * does not cause an overflow; additionally, if we're building
 * with checks enabled, it'll cause a warning
 */
static void
test_gatomicrefcount_saturation (void)
{
  if (g_test_subprocess ())
    {
      gatomicrefcount a;

      /* We're breaking abstraction here for convenience */
      a = G_MAXINT - 1;

      g_atomic_ref_count_inc (&a);
      g_assert_true (a == G_MAXINT);

      g_atomic_ref_count_inc (&a);
      g_assert_true (a == G_MAXINT);

      exit (0);
    }

  g_test_trap_subprocess (NULL, 0, 0);

#ifndef G_DISABLE_CHECKS
  /* Ensure that we got a warning when building with checks; the
   * test will fail because of the critical warning being caught
   * by GTest
   */
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*saturation*");
#else
  /* With checks disabled we don't get any warning */
  g_test_trap_assert_passed ();
#endif
}

int
main (int   argc,
      char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/refcount/grefcount", test_grefcount);
  g_test_add_func ("/refcount/grefcount/saturation", test_grefcount_saturation);

  g_test_add_func ("/refcount/gatomicrefcount", test_gatomicrefcount);
  g_test_add_func ("/refcount/gatomicrefcount/saturation", test_gatomicrefcount_saturation);

  return g_test_run ();
}
