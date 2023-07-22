/* rcbox.c: Reference counted data
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

typedef struct {
  float x, y;
} Point;

static Point *global_point;

/* test_rcbox_new: Test g_rc_box_new() */
static void
test_rcbox_new (void)
{
  Point *a = g_rc_box_new (Point);

  g_assert_nonnull (a);
  g_assert_cmpuint (g_rc_box_get_size (a), ==, sizeof (Point));

  g_rc_box_release (a);

  a = g_rc_box_new0 (Point);
  g_assert_nonnull (a);
  g_assert_cmpfloat (a->x, ==, 0.f);
  g_assert_cmpfloat (a->y, ==, 0.f);

  g_rc_box_release (a);
}

/* test_atomic_rcbox_new: Test g_atomic_rc_box_new() */
static void
test_atomic_rcbox_new (void)
{
  Point *a = g_atomic_rc_box_new (Point);

  g_assert_nonnull (a);
  g_assert_cmpuint (g_atomic_rc_box_get_size (a), ==, sizeof (Point));

  g_atomic_rc_box_release (a);

  a = g_atomic_rc_box_new0 (Point);
  g_assert_nonnull (a);
  g_assert_cmpfloat (a->x, ==, 0.f);
  g_assert_cmpfloat (a->y, ==, 0.f);

  g_atomic_rc_box_release (a);
}

static void
point_clear (Point *p)
{
  g_assert_nonnull (p);
  g_assert_true (global_point == p);

  g_assert_cmpfloat (p->x, ==, 42.0f);
  g_assert_cmpfloat (p->y, ==, 47.0f);

  g_test_message ("global_point = %p", p);
  global_point = NULL;
}

/* test_rcbox_release_full: Verify that g_rc_box_release_full() calls
 * the clear function only when the last reference is released
 */
static void
test_rcbox_release_full (void)
{
  Point *p = g_rc_box_new (Point);

  g_assert_nonnull (p);
  global_point = p;

  p->x = 42.0f;
  p->y = 47.0f;

  g_assert_true (g_rc_box_acquire (p) == p);

  g_rc_box_release_full (p, (GDestroyNotify) point_clear);
  g_assert_nonnull (global_point);
  g_assert_true (p == global_point);

  g_rc_box_release_full (p, (GDestroyNotify) point_clear);
  g_assert_null (global_point);
}

/* test_atomic_rcbox_release_full: Verify that g_atomic_rc_box_release_full()
 * calls the clear function only when the last reference is released
 */
static void
test_atomic_rcbox_release_full (void)
{
  Point *p = g_atomic_rc_box_new (Point);

  g_assert_nonnull (p);
  global_point = p;

  p->x = 42.0f;
  p->y = 47.0f;

  g_assert_true (g_atomic_rc_box_acquire (p) == p);

  g_atomic_rc_box_release_full (p, (GDestroyNotify) point_clear);
  g_assert_nonnull (global_point);
  g_assert_true (p == global_point);

  g_atomic_rc_box_release_full (p, (GDestroyNotify) point_clear);
  g_assert_null (global_point);
}

static Point *global_point_a;
static Point *global_point_b;

static void
point_clear_dup_a (Point *a)
{
  g_assert_true (a == global_point_a);

  g_test_message ("global_point_a = %p", a);
  global_point_a = NULL;
}

static void
point_clear_dup_b (Point *b)
{
  g_assert_true (b == global_point_b);

  g_test_message ("global_point_b = %p", b);
  global_point_b = NULL;
}

/* test_rcbox_dup: Verify that g_rc_box_dup() copies only the
 * data and does not change the reference count of the original
 */
static void
test_rcbox_dup (void)
{
  Point *a, *b;

  a = g_rc_box_new (Point);
  a->x = 10.f;
  a->y = 5.f;

  b = g_rc_box_dup (sizeof (Point), a);
  g_assert_true (a != b);
  g_assert_cmpfloat (a->x, ==, b->x);
  g_assert_cmpfloat (a->y, ==, b->y);

  global_point_a = a;
  global_point_b = b;

  a->x = 1.f;
  a->y = 1.f;
  g_assert_cmpfloat (a->x, !=, b->x);
  g_assert_cmpfloat (a->y, !=, b->y);

  b->x = 5.f;
  b->y = 10.f;
  g_assert_cmpfloat (a->x, !=, b->x);
  g_assert_cmpfloat (a->y, !=, b->y);

  g_rc_box_release_full (a, (GDestroyNotify) point_clear_dup_a);
  g_assert_null (global_point_a);
  g_assert_nonnull (global_point_b);

  g_rc_box_release_full (b, (GDestroyNotify) point_clear_dup_b);
  g_assert_null (global_point_b);
}

/* test_atomic_rcbox_dup: Verify that g_atomic_rc_box_dup() copies
 * only the data and does not change the reference count of the original
 */
static void
test_atomic_rcbox_dup (void)
{
  Point *a, *b;

  a = g_atomic_rc_box_new (Point);
  a->x = 10.f;
  a->y = 5.f;

  b = g_atomic_rc_box_dup (sizeof (Point), a);
  g_assert_true (a != b);
  g_assert_cmpfloat (a->x, ==, b->x);
  g_assert_cmpfloat (a->y, ==, b->y);

  global_point_a = a;
  global_point_b = b;

  a->x = 1.f;
  a->y = 1.f;
  g_assert_cmpfloat (a->x, !=, b->x);
  g_assert_cmpfloat (a->y, !=, b->y);

  b->x = 5.f;
  b->y = 10.f;
  g_assert_cmpfloat (a->x, !=, b->x);
  g_assert_cmpfloat (a->y, !=, b->y);

  g_atomic_rc_box_release_full (a, (GDestroyNotify) point_clear_dup_a);
  g_assert_null (global_point_a);
  g_assert_nonnull (global_point_b);

  g_atomic_rc_box_release_full (b, (GDestroyNotify) point_clear_dup_b);
  g_assert_null (global_point_b);
}

/* The expected alignment of the refcounted data, absent any other
 * alignment requirement, is `2 * sizeof(void*)`; GLib only really
 * supports void* sized 8 or 4 (see the comment in gatomic.h)
 */
#if GLIB_SIZEOF_VOID_P == 8
static const gsize rcbox_alignment = 16;
#else
static const gsize rcbox_alignment = 8;
#endif

/* verify that the refcounted allocation is properly aligned */
static void
test_rcbox_alignment (void)
{
  const gsize block_sizes[] = {
    1,
    2,
    4,
    sizeof (gint32) * 3,
  };

  int i;

  for (i = 0; i < G_N_ELEMENTS (block_sizes); i++)
    {
      gpointer p = g_rc_box_alloc0 (block_sizes[i]);

      g_assert_nonnull (p);
      g_assert_true (((guintptr) p & (rcbox_alignment - 1)) == 0);

      g_rc_box_release (p);
    }
}

/* verify that the atomically refcounted allocation is properly aligned */
static void
test_atomic_rcbox_alignment (void)
{
  const gsize block_sizes[] = {
    1,
    2,
    4,
    sizeof (gint32) * 3,
  };

  int i;

  for (i = 0; i < G_N_ELEMENTS (block_sizes); i++)
    {
      gpointer p = g_atomic_rc_box_alloc0 (block_sizes[i]);

      g_assert_nonnull (p);
      g_assert_true (((guintptr) p & (rcbox_alignment - 1)) == 0);

      g_atomic_rc_box_release (p);
    }
}

int
main (int   argc,
      char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/rcbox/new", test_rcbox_new);
  g_test_add_func ("/rcbox/release-full", test_rcbox_release_full);
  g_test_add_func ("/rcbox/dup", test_rcbox_dup);
  g_test_add_func ("/rcbox/alignment", test_rcbox_alignment);

  g_test_add_func ("/atomic-rcbox/new", test_atomic_rcbox_new);
  g_test_add_func ("/atomic-rcbox/release-full", test_atomic_rcbox_release_full);
  g_test_add_func ("/atomic-rcbox/dup", test_atomic_rcbox_dup);
  g_test_add_func ("/atomic-rcbox/alignment", test_atomic_rcbox_alignment);

  return g_test_run ();
}
