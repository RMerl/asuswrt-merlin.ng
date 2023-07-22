/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
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

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */

#undef G_DISABLE_ASSERT
#undef G_LOG_DOMAIN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "glib.h"

/* Test data to be passed to any function which calls g_array_new(), providing
 * the parameters for that call. Most #GArray tests should be repeated for all
 * possible values of #ArrayTestData. */
typedef struct
{
  gboolean zero_terminated;
  gboolean clear_;
} ArrayTestData;

/* Assert that @garray contains @n_expected_elements as given in @expected_data.
 * @garray must contain #gint elements. */
static void
assert_int_array_equal (GArray     *garray,
                        const gint *expected_data,
                        gsize       n_expected_elements)
{
  gsize i;

  g_assert_cmpuint (garray->len, ==, n_expected_elements);
  for (i = 0; i < garray->len; i++)
    g_assert_cmpint (g_array_index (garray, gint, i), ==, expected_data[i]);
}

/* Iff config->zero_terminated is %TRUE, assert that the final element of
 * @garray is zero. @garray must contain #gint elements. */
static void
assert_int_array_zero_terminated (const ArrayTestData *config,
                                  GArray              *garray)
{
  if (config->zero_terminated)
    {
      gint *data = (gint *) garray->data;
      g_assert_cmpint (data[garray->len], ==, 0);
    }
}

static void
sum_up (gpointer data,
	gpointer user_data)
{
  gint *sum = (gint *)user_data;

  *sum += GPOINTER_TO_INT (data);
}

/* Check that expanding an array with g_array_set_size() clears the new elements
 * if @clear_ was specified during construction. */
static void
array_set_size (gconstpointer test_data)
{
  const ArrayTestData *config = test_data;
  GArray *garray;
  gsize i;

  garray = g_array_new (config->zero_terminated, config->clear_, sizeof (gint));
  g_assert_cmpuint (garray->len, ==, 0);
  assert_int_array_zero_terminated (config, garray);

  g_array_set_size (garray, 5);
  g_assert_cmpuint (garray->len, ==, 5);
  assert_int_array_zero_terminated (config, garray);

  if (config->clear_)
    for (i = 0; i < 5; i++)
      g_assert_cmpint (g_array_index (garray, gint, i), ==, 0);

  g_array_unref (garray);
}

/* As with array_set_size(), but with a sized array. */
static void
array_set_size_sized (gconstpointer test_data)
{
  const ArrayTestData *config = test_data;
  GArray *garray;
  gsize i;

  garray = g_array_sized_new (config->zero_terminated, config->clear_, sizeof (gint), 10);
  g_assert_cmpuint (garray->len, ==, 0);
  assert_int_array_zero_terminated (config, garray);

  g_array_set_size (garray, 5);
  g_assert_cmpuint (garray->len, ==, 5);
  assert_int_array_zero_terminated (config, garray);

  if (config->clear_)
    for (i = 0; i < 5; i++)
      g_assert_cmpint (g_array_index (garray, gint, i), ==, 0);

  g_array_unref (garray);
}

/* Check that a zero-terminated array does actually have a zero terminator. */
static void
array_new_zero_terminated (void)
{
  GArray *garray;
  gchar *out_str = NULL;

  garray = g_array_new (TRUE, FALSE, sizeof (gchar));
  g_assert_cmpuint (garray->len, ==, 0);

  g_array_append_vals (garray, "hello", strlen ("hello"));
  g_assert_cmpuint (garray->len, ==, 5);
  g_assert_cmpstr (garray->data, ==, "hello");

  out_str = g_array_free (garray, FALSE);
  g_assert_cmpstr (out_str, ==, "hello");
  g_free (out_str);
}

/* Check that g_array_append_val() works correctly for various #GArray
 * configurations. */
static void
array_append_val (gconstpointer test_data)
{
  const ArrayTestData *config = test_data;
  GArray *garray;
  gint i;
  gint *segment;

  garray = g_array_new (config->zero_terminated, config->clear_, sizeof (gint));
  for (i = 0; i < 10000; i++)
    g_array_append_val (garray, i);
  assert_int_array_zero_terminated (config, garray);

  for (i = 0; i < 10000; i++)
    g_assert_cmpint (g_array_index (garray, gint, i), ==, i);

  segment = (gint*)g_array_free (garray, FALSE);
  for (i = 0; i < 10000; i++)
    g_assert_cmpint (segment[i], ==, i);
  if (config->zero_terminated)
    g_assert_cmpint (segment[10000], ==, 0);

  g_free (segment);
}

/* Check that g_array_prepend_val() works correctly for various #GArray
 * configurations. */
static void
array_prepend_val (gconstpointer test_data)
{
  const ArrayTestData *config = test_data;
  GArray *garray;
  gint i;

  garray = g_array_new (config->zero_terminated, config->clear_, sizeof (gint));
  for (i = 0; i < 100; i++)
    g_array_prepend_val (garray, i);
  assert_int_array_zero_terminated (config, garray);

  for (i = 0; i < 100; i++)
    g_assert_cmpint (g_array_index (garray, gint, i), ==, (100 - i - 1));

  g_array_free (garray, TRUE);
}

/* Test that g_array_prepend_vals() works correctly with various array
 * configurations. */
static void
array_prepend_vals (gconstpointer test_data)
{
  const ArrayTestData *config = test_data;
  GArray *garray, *garray_out;
  const gint vals[] = { 0, 1, 2, 3, 4 };
  const gint expected_vals1[] = { 0, 1 };
  const gint expected_vals2[] = { 2, 0, 1 };
  const gint expected_vals3[] = { 3, 4, 2, 0, 1 };

  /* Set up an array. */
  garray = g_array_new (config->zero_terminated, config->clear_, sizeof (gint));
  assert_int_array_zero_terminated (config, garray);

  /* Prepend several values to an empty array. */
  garray_out = g_array_prepend_vals (garray, vals, 2);
  g_assert_true (garray == garray_out);
  assert_int_array_equal (garray, expected_vals1, G_N_ELEMENTS (expected_vals1));
  assert_int_array_zero_terminated (config, garray);

  /* Prepend a single value. */
  garray_out = g_array_prepend_vals (garray, vals + 2, 1);
  g_assert_true (garray == garray_out);
  assert_int_array_equal (garray, expected_vals2, G_N_ELEMENTS (expected_vals2));
  assert_int_array_zero_terminated (config, garray);

  /* Prepend several values to a non-empty array. */
  garray_out = g_array_prepend_vals (garray, vals + 3, 2);
  g_assert_true (garray == garray_out);
  assert_int_array_equal (garray, expected_vals3, G_N_ELEMENTS (expected_vals3));
  assert_int_array_zero_terminated (config, garray);

  /* Prepend no values. */
  garray_out = g_array_prepend_vals (garray, vals, 0);
  g_assert_true (garray == garray_out);
  assert_int_array_equal (garray, expected_vals3, G_N_ELEMENTS (expected_vals3));
  assert_int_array_zero_terminated (config, garray);

  /* Prepend no values with %NULL data. */
  garray_out = g_array_prepend_vals (garray, NULL, 0);
  g_assert_true (garray == garray_out);
  assert_int_array_equal (garray, expected_vals3, G_N_ELEMENTS (expected_vals3));
  assert_int_array_zero_terminated (config, garray);

  g_array_free (garray, TRUE);
}

/* Test that g_array_insert_vals() works correctly with various array
 * configurations. */
static void
array_insert_vals (gconstpointer test_data)
{
  const ArrayTestData *config = test_data;
  GArray *garray, *garray_out;
  gsize i;
  const gint vals[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
  const gint expected_vals1[] = { 0, 1 };
  const gint expected_vals2[] = { 0, 2, 3, 1 };
  const gint expected_vals3[] = { 0, 2, 3, 1, 4 };
  const gint expected_vals4[] = { 5, 0, 2, 3, 1, 4 };
  const gint expected_vals5[] = { 5, 0, 2, 3, 1, 4, 0, 0, 0, 0, 6, 7 };

  /* Set up an array. */
  garray = g_array_new (config->zero_terminated, config->clear_, sizeof (gint));
  assert_int_array_zero_terminated (config, garray);

  /* Insert several values at the beginning. */
  garray_out = g_array_insert_vals (garray, 0, vals, 2);
  g_assert_true (garray == garray_out);
  assert_int_array_equal (garray, expected_vals1, G_N_ELEMENTS (expected_vals1));
  assert_int_array_zero_terminated (config, garray);

  /* Insert some more part-way through. */
  garray_out = g_array_insert_vals (garray, 1, vals + 2, 2);
  g_assert_true (garray == garray_out);
  assert_int_array_equal (garray, expected_vals2, G_N_ELEMENTS (expected_vals2));
  assert_int_array_zero_terminated (config, garray);

  /* And at the end. */
  garray_out = g_array_insert_vals (garray, garray->len, vals + 4, 1);
  g_assert_true (garray == garray_out);
  assert_int_array_equal (garray, expected_vals3, G_N_ELEMENTS (expected_vals3));
  assert_int_array_zero_terminated (config, garray);

  /* Then back at the beginning again. */
  garray_out = g_array_insert_vals (garray, 0, vals + 5, 1);
  g_assert_true (garray == garray_out);
  assert_int_array_equal (garray, expected_vals4, G_N_ELEMENTS (expected_vals4));
  assert_int_array_zero_terminated (config, garray);

  /* Insert zero elements. */
  garray_out = g_array_insert_vals (garray, 0, vals, 0);
  g_assert_true (garray == garray_out);
  assert_int_array_equal (garray, expected_vals4, G_N_ELEMENTS (expected_vals4));
  assert_int_array_zero_terminated (config, garray);

  /* Insert zero elements with a %NULL pointer. */
  garray_out = g_array_insert_vals (garray, 0, NULL, 0);
  g_assert_true (garray == garray_out);
  assert_int_array_equal (garray, expected_vals4, G_N_ELEMENTS (expected_vals4));
  assert_int_array_zero_terminated (config, garray);

  /* Insert some elements off the end of the array. The behaviour here depends
   * on whether the array clears entries. */
  garray_out = g_array_insert_vals (garray, garray->len + 4, vals + 6, 2);
  g_assert_true (garray == garray_out);

  g_assert_cmpuint (garray->len, ==, G_N_ELEMENTS (expected_vals5));
  for (i = 0; i < G_N_ELEMENTS (expected_vals5); i++)
    {
      if (config->clear_ || i < 6 || i > 9)
        g_assert_cmpint (g_array_index (garray, gint, i), ==, expected_vals5[i]);
    }

  assert_int_array_zero_terminated (config, garray);

  g_array_free (garray, TRUE);
}

/* Check that g_array_remove_index() works correctly for various #GArray
 * configurations. */
static void
array_remove_index (gconstpointer test_data)
{
  const ArrayTestData *config = test_data;
  GArray *garray;
  gint i;
  gint prev, cur;

  garray = g_array_new (config->zero_terminated, config->clear_, sizeof (gint));
  for (i = 0; i < 100; i++)
    g_array_append_val (garray, i);
  assert_int_array_zero_terminated (config, garray);

  g_assert_cmpint (garray->len, ==, 100);

  g_array_remove_index (garray, 1);
  g_array_remove_index (garray, 3);
  g_array_remove_index (garray, 21);
  g_array_remove_index (garray, 57);

  g_assert_cmpint (garray->len, ==, 96);
  assert_int_array_zero_terminated (config, garray);

  prev = -1;
  for (i = 0; i < garray->len; i++)
    {
      cur = g_array_index (garray, gint, i);
      g_assert (cur != 1 &&  cur != 4 && cur != 23 && cur != 60);
      g_assert_cmpint (prev, <, cur);
      prev = cur;
    }

  g_array_free (garray, TRUE);
}

/* Check that g_array_remove_index_fast() works correctly for various #GArray
 * configurations. */
static void
array_remove_index_fast (gconstpointer test_data)
{
  const ArrayTestData *config = test_data;
  GArray *garray;
  gint i;
  gint prev, cur;

  garray = g_array_new (config->zero_terminated, config->clear_, sizeof (gint));
  for (i = 0; i < 100; i++)
    g_array_append_val (garray, i);

  g_assert_cmpint (garray->len, ==, 100);
  assert_int_array_zero_terminated (config, garray);

  g_array_remove_index_fast (garray, 1);
  g_array_remove_index_fast (garray, 3);
  g_array_remove_index_fast (garray, 21);
  g_array_remove_index_fast (garray, 57);

  g_assert_cmpint (garray->len, ==, 96);
  assert_int_array_zero_terminated (config, garray);

  prev = -1;
  for (i = 0; i < garray->len; i++)
    {
      cur = g_array_index (garray, gint, i);
      g_assert (cur != 1 &&  cur != 3 && cur != 21 && cur != 57);
      if (cur < 96)
        {
          g_assert_cmpint (prev, <, cur);
          prev = cur;
        }
    }

  g_array_free (garray, TRUE);
}

/* Check that g_array_remove_range() works correctly for various #GArray
 * configurations. */
static void
array_remove_range (gconstpointer test_data)
{
  const ArrayTestData *config = test_data;
  GArray *garray;
  gint i;
  gint prev, cur;

  garray = g_array_new (config->zero_terminated, config->clear_, sizeof (gint));
  for (i = 0; i < 100; i++)
    g_array_append_val (garray, i);

  g_assert_cmpint (garray->len, ==, 100);
  assert_int_array_zero_terminated (config, garray);

  g_array_remove_range (garray, 31, 4);

  g_assert_cmpint (garray->len, ==, 96);
  assert_int_array_zero_terminated (config, garray);

  prev = -1;
  for (i = 0; i < garray->len; i++)
    {
      cur = g_array_index (garray, gint, i);
      g_assert (cur < 31 || cur > 34);
      g_assert_cmpint (prev, <, cur);
      prev = cur;
    }

  /* Ensure the entire array can be cleared, even when empty. */
  g_array_remove_range (garray, 0, garray->len);

  g_assert_cmpint (garray->len, ==, 0);
  assert_int_array_zero_terminated (config, garray);

  g_array_remove_range (garray, 0, garray->len);

  g_assert_cmpint (garray->len, ==, 0);
  assert_int_array_zero_terminated (config, garray);

  g_array_free (garray, TRUE);
}

static void
array_ref_count (void)
{
  GArray *garray;
  GArray *garray2;
  gint i;

  garray = g_array_new (FALSE, FALSE, sizeof (gint));
  g_assert_cmpint (g_array_get_element_size (garray), ==, sizeof (gint));
  for (i = 0; i < 100; i++)
    g_array_prepend_val (garray, i);

  /* check we can ref, unref and still access the array */
  garray2 = g_array_ref (garray);
  g_assert (garray == garray2);
  g_array_unref (garray2);
  for (i = 0; i < 100; i++)
    g_assert_cmpint (g_array_index (garray, gint, i), ==, (100 - i - 1));

  /* garray2 should be an empty valid GArray wrapper */
  garray2 = g_array_ref (garray);
  g_array_free (garray, TRUE);

  g_assert_cmpint (garray2->len, ==, 0);
  g_array_unref (garray2);
}

static int
int_compare (gconstpointer p1, gconstpointer p2)
{
  const gint *i1 = p1;
  const gint *i2 = p2;

  return *i1 - *i2;
}

static int
int_compare_data (gconstpointer p1, gconstpointer p2, gpointer data)
{
  const gint *i1 = p1;
  const gint *i2 = p2;

  return *i1 - *i2;
}

/* Check that g_array_sort() works correctly for various #GArray
 * configurations. */
static void
array_sort (gconstpointer test_data)
{
  const ArrayTestData *config = test_data;
  GArray *garray;
  gint i;
  gint prev, cur;

  garray = g_array_new (config->zero_terminated, config->clear_, sizeof (gint));
  for (i = 0; i < 10000; i++)
    {
      cur = g_random_int_range (0, 10000);
      g_array_append_val (garray, cur);
    }
  assert_int_array_zero_terminated (config, garray);

  g_array_sort (garray, int_compare);
  assert_int_array_zero_terminated (config, garray);

  prev = -1;
  for (i = 0; i < garray->len; i++)
    {
      cur = g_array_index (garray, gint, i);
      g_assert_cmpint (prev, <=, cur);
      prev = cur;
    }

  g_array_free (garray, TRUE);
}

/* Check that g_array_sort_with_data() works correctly for various #GArray
 * configurations. */
static void
array_sort_with_data (gconstpointer test_data)
{
  const ArrayTestData *config = test_data;
  GArray *garray;
  gint i;
  gint prev, cur;

  garray = g_array_new (config->zero_terminated, config->clear_, sizeof (gint));
  for (i = 0; i < 10000; i++)
    {
      cur = g_random_int_range (0, 10000);
      g_array_append_val (garray, cur);
    }
  assert_int_array_zero_terminated (config, garray);

  g_array_sort_with_data (garray, int_compare_data, NULL);
  assert_int_array_zero_terminated (config, garray);

  prev = -1;
  for (i = 0; i < garray->len; i++)
    {
      cur = g_array_index (garray, gint, i);
      g_assert_cmpint (prev, <=, cur);
      prev = cur;
    }

  g_array_free (garray, TRUE);
}

static gint num_clear_func_invocations = 0;

static void
my_clear_func (gpointer data)
{
  num_clear_func_invocations += 1;
}

static void
array_clear_func (void)
{
  GArray *garray;
  gint i;
  gint cur;

  garray = g_array_new (FALSE, FALSE, sizeof (gint));
  g_array_set_clear_func (garray, my_clear_func);

  for (i = 0; i < 10; i++)
    {
      cur = g_random_int_range (0, 100);
      g_array_append_val (garray, cur);
    }

  g_array_remove_index (garray, 9);
  g_assert_cmpint (num_clear_func_invocations, ==, 1);

  g_array_remove_range (garray, 5, 3);
  g_assert_cmpint (num_clear_func_invocations, ==, 4);

  g_array_remove_index_fast (garray, 4);
  g_assert_cmpint (num_clear_func_invocations, ==, 5);

  g_array_free (garray, TRUE);
  g_assert_cmpint (num_clear_func_invocations, ==, 10);
}

static void
pointer_array_add (void)
{
  GPtrArray *gparray;
  gint i;
  gint sum = 0;
  gpointer *segment;

  gparray = g_ptr_array_sized_new (1000);

  for (i = 0; i < 10000; i++)
    g_ptr_array_add (gparray, GINT_TO_POINTER (i));

  for (i = 0; i < 10000; i++)
    g_assert (g_ptr_array_index (gparray, i) == GINT_TO_POINTER (i));
  
  g_ptr_array_foreach (gparray, sum_up, &sum);
  g_assert (sum == 49995000);

  segment = g_ptr_array_free (gparray, FALSE);
  for (i = 0; i < 10000; i++)
    g_assert (segment[i] == GINT_TO_POINTER (i));
  g_free (segment);
}

static void
pointer_array_insert (void)
{
  GPtrArray *gparray;
  gint i;
  gint sum = 0;
  gint index;

  gparray = g_ptr_array_sized_new (1000);

  for (i = 0; i < 10000; i++)
    {
      index = g_random_int_range (-1, i + 1);
      g_ptr_array_insert (gparray, index, GINT_TO_POINTER (i));
    }

  g_ptr_array_foreach (gparray, sum_up, &sum);
  g_assert (sum == 49995000);

  g_ptr_array_free (gparray, TRUE);
}

static void
pointer_array_ref_count (void)
{
  GPtrArray *gparray;
  GPtrArray *gparray2;
  gint i;
  gint sum = 0;

  gparray = g_ptr_array_new ();
  for (i = 0; i < 10000; i++)
    g_ptr_array_add (gparray, GINT_TO_POINTER (i));

  /* check we can ref, unref and still access the array */
  gparray2 = g_ptr_array_ref (gparray);
  g_assert (gparray == gparray2);
  g_ptr_array_unref (gparray2);
  for (i = 0; i < 10000; i++)
    g_assert (g_ptr_array_index (gparray, i) == GINT_TO_POINTER (i));

  g_ptr_array_foreach (gparray, sum_up, &sum);
  g_assert (sum == 49995000);

  /* gparray2 should be an empty valid GPtrArray wrapper */
  gparray2 = g_ptr_array_ref (gparray);
  g_ptr_array_free (gparray, TRUE);

  g_assert_cmpint (gparray2->len, ==, 0);
  g_ptr_array_unref (gparray2);
}

static gint num_free_func_invocations = 0;

static void
my_free_func (gpointer data)
{
  num_free_func_invocations++;
  g_free (data);
}

static void
pointer_array_free_func (void)
{
  GPtrArray *gparray;
  GPtrArray *gparray2;
  gchar **strv;
  gchar *s;

  num_free_func_invocations = 0;
  gparray = g_ptr_array_new_with_free_func (my_free_func);
  g_ptr_array_unref (gparray);
  g_assert_cmpint (num_free_func_invocations, ==, 0);

  gparray = g_ptr_array_new_with_free_func (my_free_func);
  g_ptr_array_free (gparray, TRUE);
  g_assert_cmpint (num_free_func_invocations, ==, 0);

  num_free_func_invocations = 0;
  gparray = g_ptr_array_new_with_free_func (my_free_func);
  g_ptr_array_add (gparray, g_strdup ("foo"));
  g_ptr_array_add (gparray, g_strdup ("bar"));
  g_ptr_array_add (gparray, g_strdup ("baz"));
  g_ptr_array_remove_index (gparray, 0);
  g_assert_cmpint (num_free_func_invocations, ==, 1);
  g_ptr_array_remove_index_fast (gparray, 1);
  g_assert_cmpint (num_free_func_invocations, ==, 2);
  s = g_strdup ("frob");
  g_ptr_array_add (gparray, s);
  g_assert (g_ptr_array_remove (gparray, s));
  g_assert (!g_ptr_array_remove (gparray, "nuun"));
  g_assert (!g_ptr_array_remove_fast (gparray, "mlo"));
  g_assert_cmpint (num_free_func_invocations, ==, 3);
  s = g_strdup ("frob");
  g_ptr_array_add (gparray, s);
  g_ptr_array_set_size (gparray, 1);
  g_assert_cmpint (num_free_func_invocations, ==, 4);
  g_ptr_array_ref (gparray);
  g_ptr_array_unref (gparray);
  g_assert_cmpint (num_free_func_invocations, ==, 4);
  g_ptr_array_unref (gparray);
  g_assert_cmpint (num_free_func_invocations, ==, 5);

  num_free_func_invocations = 0;
  gparray = g_ptr_array_new_full (10, my_free_func);
  g_ptr_array_add (gparray, g_strdup ("foo"));
  g_ptr_array_add (gparray, g_strdup ("bar"));
  g_ptr_array_add (gparray, g_strdup ("baz"));
  g_ptr_array_set_size (gparray, 20);
  g_ptr_array_add (gparray, NULL);
  gparray2 = g_ptr_array_ref (gparray);
  strv = (gchar **) g_ptr_array_free (gparray, FALSE);
  g_assert_cmpint (num_free_func_invocations, ==, 0);
  g_strfreev (strv);
  g_ptr_array_unref (gparray2);
  g_assert_cmpint (num_free_func_invocations, ==, 0);

  num_free_func_invocations = 0;
  gparray = g_ptr_array_new_with_free_func (my_free_func);
  g_ptr_array_add (gparray, g_strdup ("foo"));
  g_ptr_array_add (gparray, g_strdup ("bar"));
  g_ptr_array_add (gparray, g_strdup ("baz"));
  g_ptr_array_remove_range (gparray, 1, 1);
  g_ptr_array_unref (gparray);
  g_assert_cmpint (num_free_func_invocations, ==, 3);

  num_free_func_invocations = 0;
  gparray = g_ptr_array_new_with_free_func (my_free_func);
  g_ptr_array_add (gparray, g_strdup ("foo"));
  g_ptr_array_add (gparray, g_strdup ("bar"));
  g_ptr_array_add (gparray, g_strdup ("baz"));
  g_ptr_array_free (gparray, TRUE);
  g_assert_cmpint (num_free_func_invocations, ==, 3);

  num_free_func_invocations = 0;
  gparray = g_ptr_array_new_with_free_func (my_free_func);
  g_ptr_array_add (gparray, "foo");
  g_ptr_array_add (gparray, "bar");
  g_ptr_array_add (gparray, "baz");
  g_ptr_array_set_free_func (gparray, NULL);
  g_ptr_array_free (gparray, TRUE);
  g_assert_cmpint (num_free_func_invocations, ==, 0);
}

static gint
ptr_compare (gconstpointer p1, gconstpointer p2)
{
  gpointer i1 = *(gpointer*)p1;
  gpointer i2 = *(gpointer*)p2;

  return GPOINTER_TO_INT (i1) - GPOINTER_TO_INT (i2);
}

static gint
ptr_compare_data (gconstpointer p1, gconstpointer p2, gpointer data)
{
  gpointer i1 = *(gpointer*)p1;
  gpointer i2 = *(gpointer*)p2;

  return GPOINTER_TO_INT (i1) - GPOINTER_TO_INT (i2);
}

static void
pointer_array_sort (void)
{
  GPtrArray *gparray;
  gint i;
  gint val;
  gint prev, cur;

  gparray = g_ptr_array_new ();
  for (i = 0; i < 10000; i++)
    {
      val = g_random_int_range (0, 10000);
      g_ptr_array_add (gparray, GINT_TO_POINTER (val));
    }

  g_ptr_array_sort (gparray, ptr_compare);

  prev = -1;
  for (i = 0; i < 10000; i++)
    {
      cur = GPOINTER_TO_INT (g_ptr_array_index (gparray, i));
      g_assert_cmpint (prev, <=, cur);
      prev = cur;
    }

  g_ptr_array_free (gparray, TRUE);
}

static void
pointer_array_sort_with_data (void)
{
  GPtrArray *gparray;
  gint i;
  gint prev, cur;

  gparray = g_ptr_array_new ();
  for (i = 0; i < 10000; i++)
    g_ptr_array_add (gparray, GINT_TO_POINTER (g_random_int_range (0, 10000)));

  g_ptr_array_sort_with_data (gparray, ptr_compare_data, NULL);

  prev = -1;
  for (i = 0; i < 10000; i++)
    {
      cur = GPOINTER_TO_INT (g_ptr_array_index (gparray, i));
      g_assert_cmpint (prev, <=, cur);
      prev = cur;
    }

  g_ptr_array_free (gparray, TRUE);
}

static void
pointer_array_find_empty (void)
{
  GPtrArray *array;
  guint idx;

  array = g_ptr_array_new ();

  g_assert_false (g_ptr_array_find (array, "some-value", NULL));  /* NULL index */
  g_assert_false (g_ptr_array_find (array, "some-value", &idx));  /* non-NULL index */
  g_assert_false (g_ptr_array_find_with_equal_func (array, "some-value", g_str_equal, NULL));  /* NULL index */
  g_assert_false (g_ptr_array_find_with_equal_func (array, "some-value", g_str_equal, &idx));  /* non-NULL index */

  g_ptr_array_free (array, TRUE);
}

static void
pointer_array_find_non_empty (void)
{
  GPtrArray *array;
  guint idx;
  const gchar *str_pointer = "static-string";

  array = g_ptr_array_new ();

  g_ptr_array_add (array, "some");
  g_ptr_array_add (array, "random");
  g_ptr_array_add (array, "values");
  g_ptr_array_add (array, "some");
  g_ptr_array_add (array, "duplicated");
  g_ptr_array_add (array, (gpointer) str_pointer);

  g_assert_true (g_ptr_array_find_with_equal_func (array, "random", g_str_equal, NULL));  /* NULL index */
  g_assert_true (g_ptr_array_find_with_equal_func (array, "random", g_str_equal, &idx));  /* non-NULL index */
  g_assert_cmpuint (idx, ==, 1);

  g_assert_true (g_ptr_array_find_with_equal_func (array, "some", g_str_equal, &idx));  /* duplicate element */
  g_assert_cmpuint (idx, ==, 0);

  g_assert_false (g_ptr_array_find_with_equal_func (array, "nope", g_str_equal, NULL));

  g_assert_true (g_ptr_array_find_with_equal_func (array, str_pointer, g_str_equal, &idx));
  g_assert_cmpuint (idx, ==, 5);
  idx = G_MAXUINT;
  g_assert_true (g_ptr_array_find_with_equal_func (array, str_pointer, NULL, &idx));  /* NULL equal func */
  g_assert_cmpuint (idx, ==, 5);
  idx = G_MAXUINT;
  g_assert_true (g_ptr_array_find (array, str_pointer, &idx));  /* NULL equal func */
  g_assert_cmpuint (idx, ==, 5);

  g_ptr_array_free (array, TRUE);
}

static void
steal_destroy_notify (gpointer data)
{
  guint *counter = data;
  *counter = *counter + 1;
}

/* Test that g_ptr_array_steal_index() and g_ptr_array_steal_index_fast() can
 * remove elements from a pointer array without the #GDestroyNotify being called. */
static void
pointer_array_steal (void)
{
  guint i1 = 0, i2 = 0, i3 = 0, i4 = 0;
  gpointer out1, out2;
  GPtrArray *array = g_ptr_array_new_with_free_func (steal_destroy_notify);

  g_ptr_array_add (array, &i1);
  g_ptr_array_add (array, &i2);
  g_ptr_array_add (array, &i3);
  g_ptr_array_add (array, &i4);

  g_assert_cmpuint (array->len, ==, 4);

  /* Remove a single element. */
  out1 = g_ptr_array_steal_index (array, 0);
  g_assert_true (out1 == &i1);
  g_assert_cmpuint (i1, ==, 0);  /* should not have been destroyed */

  /* Following elements should have been moved down. */
  g_assert_cmpuint (array->len, ==, 3);
  g_assert_true (g_ptr_array_index (array, 0) == &i2);
  g_assert_true (g_ptr_array_index (array, 1) == &i3);
  g_assert_true (g_ptr_array_index (array, 2) == &i4);

  /* Remove another element, quickly. */
  out2 = g_ptr_array_steal_index_fast (array, 0);
  g_assert_true (out2 == &i2);
  g_assert_cmpuint (i2, ==, 0);  /* should not have been destroyed */

  /* Last element should have been swapped in place. */
  g_assert_cmpuint (array->len, ==, 2);
  g_assert_true (g_ptr_array_index (array, 0) == &i4);
  g_assert_true (g_ptr_array_index (array, 1) == &i3);

  /* Check that destroying the pointer array doesn’t affect the stolen elements. */
  g_ptr_array_unref (array);

  g_assert_cmpuint (i1, ==, 0);
  g_assert_cmpuint (i2, ==, 0);
  g_assert_cmpuint (i3, ==, 1);
  g_assert_cmpuint (i4, ==, 1);
}

static void
byte_array_append (void)
{
  GByteArray *gbarray;
  gint i;
  guint8 *segment;

  gbarray = g_byte_array_sized_new (1000);
  for (i = 0; i < 10000; i++)
    g_byte_array_append (gbarray, (guint8*) "abcd", 4);

  for (i = 0; i < 10000; i++)
    {
      g_assert (gbarray->data[4*i] == 'a');
      g_assert (gbarray->data[4*i+1] == 'b');
      g_assert (gbarray->data[4*i+2] == 'c');
      g_assert (gbarray->data[4*i+3] == 'd');
    }

  segment = g_byte_array_free (gbarray, FALSE);

  for (i = 0; i < 10000; i++)
    {
      g_assert (segment[4*i] == 'a');
      g_assert (segment[4*i+1] == 'b');
      g_assert (segment[4*i+2] == 'c');
      g_assert (segment[4*i+3] == 'd');
    }

  g_free (segment);
}

static void
byte_array_prepend (void)
{
  GByteArray *gbarray;
  gint i;

  gbarray = g_byte_array_new ();
  g_byte_array_set_size (gbarray, 1000);

  for (i = 0; i < 10000; i++)
    g_byte_array_prepend (gbarray, (guint8*) "abcd", 4);

  for (i = 0; i < 10000; i++)
    {
      g_assert (gbarray->data[4*i] == 'a');
      g_assert (gbarray->data[4*i+1] == 'b');
      g_assert (gbarray->data[4*i+2] == 'c');
      g_assert (gbarray->data[4*i+3] == 'd');
    }

  g_byte_array_free (gbarray, TRUE);
}

static void
byte_array_ref_count (void)
{
  GByteArray *gbarray;
  GByteArray *gbarray2;
  gint i;

  gbarray = g_byte_array_new ();
  for (i = 0; i < 10000; i++)
    g_byte_array_append (gbarray, (guint8*) "abcd", 4);

  gbarray2 = g_byte_array_ref (gbarray);
  g_assert (gbarray2 == gbarray);
  g_byte_array_unref (gbarray2);
  for (i = 0; i < 10000; i++)
    {
      g_assert (gbarray->data[4*i] == 'a');
      g_assert (gbarray->data[4*i+1] == 'b');
      g_assert (gbarray->data[4*i+2] == 'c');
      g_assert (gbarray->data[4*i+3] == 'd');
    }

  gbarray2 = g_byte_array_ref (gbarray);
  g_assert (gbarray2 == gbarray);
  g_byte_array_free (gbarray, TRUE);
  g_assert_cmpint (gbarray2->len, ==, 0);
  g_byte_array_unref (gbarray2);
}

static void
byte_array_remove (void)
{
  GByteArray *gbarray;
  gint i;

  gbarray = g_byte_array_new ();
  for (i = 0; i < 100; i++)
    g_byte_array_append (gbarray, (guint8*) "abcd", 4);

  g_assert_cmpint (gbarray->len, ==, 400);

  g_byte_array_remove_index (gbarray, 4);
  g_byte_array_remove_index (gbarray, 4);
  g_byte_array_remove_index (gbarray, 4);
  g_byte_array_remove_index (gbarray, 4);

  g_assert_cmpint (gbarray->len, ==, 396);

  for (i = 0; i < 99; i++)
    {
      g_assert (gbarray->data[4*i] == 'a');
      g_assert (gbarray->data[4*i+1] == 'b');
      g_assert (gbarray->data[4*i+2] == 'c');
      g_assert (gbarray->data[4*i+3] == 'd');
    }

  g_byte_array_free (gbarray, TRUE);
}

static void
byte_array_remove_fast (void)
{
  GByteArray *gbarray;
  gint i;

  gbarray = g_byte_array_new ();
  for (i = 0; i < 100; i++)
    g_byte_array_append (gbarray, (guint8*) "abcd", 4);

  g_assert_cmpint (gbarray->len, ==, 400);

  g_byte_array_remove_index_fast (gbarray, 4);
  g_byte_array_remove_index_fast (gbarray, 4);
  g_byte_array_remove_index_fast (gbarray, 4);
  g_byte_array_remove_index_fast (gbarray, 4);

  g_assert_cmpint (gbarray->len, ==, 396);

  for (i = 0; i < 99; i++)
    {
      g_assert (gbarray->data[4*i] == 'a');
      g_assert (gbarray->data[4*i+1] == 'b');
      g_assert (gbarray->data[4*i+2] == 'c');
      g_assert (gbarray->data[4*i+3] == 'd');
    }

  g_byte_array_free (gbarray, TRUE);
}

static void
byte_array_remove_range (void)
{
  GByteArray *gbarray;
  gint i;

  gbarray = g_byte_array_new ();
  for (i = 0; i < 100; i++)
    g_byte_array_append (gbarray, (guint8*) "abcd", 4);

  g_assert_cmpint (gbarray->len, ==, 400);

  g_byte_array_remove_range (gbarray, 12, 4);

  g_assert_cmpint (gbarray->len, ==, 396);

  for (i = 0; i < 99; i++)
    {
      g_assert (gbarray->data[4*i] == 'a');
      g_assert (gbarray->data[4*i+1] == 'b');
      g_assert (gbarray->data[4*i+2] == 'c');
      g_assert (gbarray->data[4*i+3] == 'd');
    }

  /* Ensure the entire array can be cleared, even when empty. */
  g_byte_array_remove_range (gbarray, 0, gbarray->len);
  g_byte_array_remove_range (gbarray, 0, gbarray->len);

  g_byte_array_free (gbarray, TRUE);
}

static int
byte_compare (gconstpointer p1, gconstpointer p2)
{
  const guint8 *i1 = p1;
  const guint8 *i2 = p2;

  return *i1 - *i2;
}

static int
byte_compare_data (gconstpointer p1, gconstpointer p2, gpointer data)
{
  const guint8 *i1 = p1;
  const guint8 *i2 = p2;

  return *i1 - *i2;
}

static void
byte_array_sort (void)
{
  GByteArray *gbarray;
  gint i;
  guint8 val;
  guint8 prev, cur;

  gbarray = g_byte_array_new ();
  for (i = 0; i < 100; i++)
    {
      val = 'a' + g_random_int_range (0, 26);
      g_byte_array_append (gbarray, (guint8*) &val, 1);
    }

  g_byte_array_sort (gbarray, byte_compare);

  prev = 'a';
  for (i = 0; i < gbarray->len; i++)
    {
      cur = gbarray->data[i];
      g_assert_cmpint (prev, <=, cur);
      prev = cur;
    }

  g_byte_array_free (gbarray, TRUE);
}

static void
byte_array_sort_with_data (void)
{
  GByteArray *gbarray;
  gint i;
  guint8 val;
  guint8 prev, cur;

  gbarray = g_byte_array_new ();
  for (i = 0; i < 100; i++)
    {
      val = 'a' + g_random_int_range (0, 26);
      g_byte_array_append (gbarray, (guint8*) &val, 1);
    }

  g_byte_array_sort_with_data (gbarray, byte_compare_data, NULL);

  prev = 'a';
  for (i = 0; i < gbarray->len; i++)
    {
      cur = gbarray->data[i];
      g_assert_cmpint (prev, <=, cur);
      prev = cur;
    }

  g_byte_array_free (gbarray, TRUE);
}

static void
byte_array_new_take (void)
{
  GByteArray *gbarray;
  guint8 *data;

  data = g_memdup ("woooweeewow", 11);
  gbarray = g_byte_array_new_take (data, 11);
  g_assert (gbarray->data == data);
  g_assert_cmpuint (gbarray->len, ==, 11);
  g_byte_array_free (gbarray, TRUE);
}

static void
byte_array_free_to_bytes (void)
{
  GByteArray *gbarray;
  gpointer memory;
  GBytes *bytes;
  gsize size;

  gbarray = g_byte_array_new ();
  g_byte_array_append (gbarray, (guint8 *)"woooweeewow", 11);
  memory = gbarray->data;

  bytes = g_byte_array_free_to_bytes (gbarray);
  g_assert (bytes != NULL);
  g_assert_cmpuint (g_bytes_get_size (bytes), ==, 11);
  g_assert (g_bytes_get_data (bytes, &size) == memory);
  g_assert_cmpuint (size, ==, 11);

  g_bytes_unref (bytes);
}

static void
add_array_test (const gchar         *test_path,
                const ArrayTestData *config,
                GTestDataFunc        test_func)
{
  gchar *test_name = NULL;

  test_name = g_strdup_printf ("%s/%s-%s",
                               test_path,
                               config->zero_terminated ? "zero-terminated" : "non-zero-terminated",
                               config->clear_ ? "clear" : "no-clear");
  g_test_add_data_func (test_name, config, test_func);
  g_free (test_name);
}

int
main (int argc, char *argv[])
{
  /* Test all possible combinations of g_array_new() parameters. */
  const ArrayTestData array_configurations[] =
    {
      { FALSE, FALSE },
      { FALSE, TRUE },
      { TRUE, FALSE },
      { TRUE, TRUE },
    };
  gsize i;

  g_test_init (&argc, &argv, NULL);

  g_test_bug_base ("https://bugzilla.gnome.org/");

  /* array tests */
  g_test_add_func ("/array/new/zero-terminated", array_new_zero_terminated);
  g_test_add_func ("/array/ref-count", array_ref_count);
  g_test_add_func ("/array/clear-func", array_clear_func);

  for (i = 0; i < G_N_ELEMENTS (array_configurations); i++)
    {
      add_array_test ("/array/set-size", &array_configurations[i], array_set_size);
      add_array_test ("/array/set-size/sized", &array_configurations[i], array_set_size_sized);
      add_array_test ("/array/append-val", &array_configurations[i], array_append_val);
      add_array_test ("/array/prepend-val", &array_configurations[i], array_prepend_val);
      add_array_test ("/array/prepend-vals", &array_configurations[i], array_prepend_vals);
      add_array_test ("/array/insert-vals", &array_configurations[i], array_insert_vals);
      add_array_test ("/array/remove-index", &array_configurations[i], array_remove_index);
      add_array_test ("/array/remove-index-fast", &array_configurations[i], array_remove_index_fast);
      add_array_test ("/array/remove-range", &array_configurations[i], array_remove_range);
      add_array_test ("/array/sort", &array_configurations[i], array_sort);
      add_array_test ("/array/sort-with-data", &array_configurations[i], array_sort_with_data);
    }

  /* pointer arrays */
  g_test_add_func ("/pointerarray/add", pointer_array_add);
  g_test_add_func ("/pointerarray/insert", pointer_array_insert);
  g_test_add_func ("/pointerarray/ref-count", pointer_array_ref_count);
  g_test_add_func ("/pointerarray/free-func", pointer_array_free_func);
  g_test_add_func ("/pointerarray/sort", pointer_array_sort);
  g_test_add_func ("/pointerarray/sort-with-data", pointer_array_sort_with_data);
  g_test_add_func ("/pointerarray/find/empty", pointer_array_find_empty);
  g_test_add_func ("/pointerarray/find/non-empty", pointer_array_find_non_empty);
  g_test_add_func ("/pointerarray/steal", pointer_array_steal);

  /* byte arrays */
  g_test_add_func ("/bytearray/append", byte_array_append);
  g_test_add_func ("/bytearray/prepend", byte_array_prepend);
  g_test_add_func ("/bytearray/remove", byte_array_remove);
  g_test_add_func ("/bytearray/remove-fast", byte_array_remove_fast);
  g_test_add_func ("/bytearray/remove-range", byte_array_remove_range);
  g_test_add_func ("/bytearray/ref-count", byte_array_ref_count);
  g_test_add_func ("/bytearray/sort", byte_array_sort);
  g_test_add_func ("/bytearray/sort-with-data", byte_array_sort_with_data);
  g_test_add_func ("/bytearray/new-take", byte_array_new_take);
  g_test_add_func ("/bytearray/free-to-bytes", byte_array_free_to_bytes);

  return g_test_run ();
}

