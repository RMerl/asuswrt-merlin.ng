/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
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

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */

#undef G_DISABLE_ASSERT
#undef G_LOG_DOMAIN

/* We are testing some deprecated APIs here */
#define GLIB_DISABLE_DEPRECATION_WARNINGS

#include <stdio.h>
#include <string.h>
#include "glib.h"


static gint
my_compare (gconstpointer a,
            gconstpointer b)
{
  const char *cha = a;
  const char *chb = b;

  return *cha - *chb;
}

static gint
my_compare_with_data (gconstpointer a,
                      gconstpointer b,
                      gpointer      user_data)
{
  const char *cha = a;
  const char *chb = b;

  /* just check that we got the right data */
  g_assert (GPOINTER_TO_INT(user_data) == 123);

  return *cha - *chb;
}

static gint
my_search (gconstpointer a,
           gconstpointer b)
{
  return my_compare (b, a);
}

static gpointer destroyed_key = NULL;
static gpointer destroyed_value = NULL;

static void
my_key_destroy (gpointer key)
{
  destroyed_key = key;
}

static void
my_value_destroy (gpointer value)
{
  destroyed_value = value;
}

static gint
my_traverse (gpointer key,
             gpointer value,
             gpointer data)
{
  char *ch = key;
  g_assert ((*ch) > 0);
  return FALSE;
}

char chars[] =
  "0123456789"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz";

char chars2[] =
  "0123456789"
  "abcdefghijklmnopqrstuvwxyz";

static gint
check_order (gpointer key,
             gpointer value,
             gpointer data)
{
  char **p = data;
  char *ch = key;
 
  g_assert (**p == *ch);

  (*p)++;

  return FALSE;
}

static void
test_tree_search (void)
{
  gint i;
  GTree *tree;
  gboolean removed;
  gchar c;
  gchar *p, *d;

  tree = g_tree_new_with_data (my_compare_with_data, GINT_TO_POINTER(123));

  for (i = 0; chars[i]; i++)
    g_tree_insert (tree, &chars[i], &chars[i]);

  g_tree_foreach (tree, my_traverse, NULL);

  g_assert_cmpint (g_tree_nnodes (tree), ==, strlen (chars));
  g_assert_cmpint (g_tree_height (tree), ==, 6);
 
  p = chars;
  g_tree_foreach (tree, check_order, &p);

  for (i = 0; i < 26; i++)
    {
      removed = g_tree_remove (tree, &chars[i + 10]);
      g_assert (removed);
    }

  c = '\0';
  removed = g_tree_remove (tree, &c);
  g_assert (!removed);

  g_tree_foreach (tree, my_traverse, NULL);

  g_assert_cmpint (g_tree_nnodes (tree), ==, strlen (chars2));
  g_assert_cmpint (g_tree_height (tree), ==, 6);

  p = chars2;
  g_tree_foreach (tree, check_order, &p);

  for (i = 25; i >= 0; i--)
    g_tree_insert (tree, &chars[i + 10], &chars[i + 10]);

  p = chars;
  g_tree_foreach (tree, check_order, &p);

  c = '0';
  p = g_tree_lookup (tree, &c);
  g_assert (p && *p == c);
  g_assert (g_tree_lookup_extended (tree, &c, (gpointer *)&d, (gpointer *)&p));
  g_assert (c == *d && c == *p);

  c = 'A';
  p = g_tree_lookup (tree, &c);
  g_assert (p && *p == c);

  c = 'a';
  p = g_tree_lookup (tree, &c);
  g_assert (p && *p == c);

  c = 'z';
  p = g_tree_lookup (tree, &c);
  g_assert (p && *p == c);

  c = '!';
  p = g_tree_lookup (tree, &c);
  g_assert (p == NULL);

  c = '=';
  p = g_tree_lookup (tree, &c);
  g_assert (p == NULL);

  c = '|';
  p = g_tree_lookup (tree, &c);
  g_assert (p == NULL);

  c = '0';
  p = g_tree_search (tree, my_search, &c);
  g_assert (p && *p == c);

  c = 'A';
  p = g_tree_search (tree, my_search, &c);
  g_assert (p && *p == c);

  c = 'a';
  p = g_tree_search (tree, my_search, &c);
  g_assert (p &&*p == c);

  c = 'z';
  p = g_tree_search (tree, my_search, &c);
  g_assert (p && *p == c);

  c = '!';
  p = g_tree_search (tree, my_search, &c);
  g_assert (p == NULL);

  c = '=';
  p = g_tree_search (tree, my_search, &c);
  g_assert (p == NULL);

  c = '|';
  p = g_tree_search (tree, my_search, &c);
  g_assert (p == NULL);

  g_tree_destroy (tree);
}

static void
test_tree_remove (void)
{
  GTree *tree;
  char c, d;
  gint i;
  gboolean removed;
  gchar *remove;

  tree = g_tree_new_full ((GCompareDataFunc)my_compare, NULL,
                          my_key_destroy,
                          my_value_destroy);

  for (i = 0; chars[i]; i++)
    g_tree_insert (tree, &chars[i], &chars[i]);

  c = '0';
  g_tree_insert (tree, &c, &c);
  g_assert (destroyed_key == &c);
  g_assert (destroyed_value == &chars[0]);
  destroyed_key = NULL;
  destroyed_value = NULL;

  d = '1';
  g_tree_replace (tree, &d, &d);
  g_assert (destroyed_key == &chars[1]);
  g_assert (destroyed_value == &chars[1]);
  destroyed_key = NULL;
  destroyed_value = NULL;

  c = '2';
  removed = g_tree_remove (tree, &c);
  g_assert (removed);
  g_assert (destroyed_key == &chars[2]);
  g_assert (destroyed_value == &chars[2]);
  destroyed_key = NULL;
  destroyed_value = NULL;

  c = '3';
  removed = g_tree_steal (tree, &c);
  g_assert (removed);
  g_assert (destroyed_key == NULL);
  g_assert (destroyed_value == NULL);

  remove = "omkjigfedba";
  for (i = 0; remove[i]; i++)
    {
      removed = g_tree_remove (tree, &remove[i]);
      g_assert (removed);
    }

  g_tree_destroy (tree);
}

static void
test_tree_destroy (void)
{
  GTree *tree;
  gint i;

  tree = g_tree_new (my_compare);

  for (i = 0; chars[i]; i++)
    g_tree_insert (tree, &chars[i], &chars[i]);

  g_assert_cmpint (g_tree_nnodes (tree), ==, strlen (chars));

  g_tree_ref (tree);
  g_tree_destroy (tree);

  g_assert_cmpint (g_tree_nnodes (tree), ==, 0);

  g_tree_unref (tree);
}

static gboolean
traverse_func (gpointer key, gpointer value, gpointer data)
{
  gchar *c = value;
  gchar **p = data;

  **p = *c;
  (*p)++;

  return FALSE;
}

static void
test_tree_traverse (void)
{
  GTree *tree;
  gint i;
  gchar *p, *result;

  tree = g_tree_new (my_compare);

  for (i = 0; chars[i]; i++)
    g_tree_insert (tree, &chars[i], &chars[i]);

  result = g_new0 (gchar, strlen (chars) + 1);

  p = result;
  g_tree_traverse (tree, traverse_func, G_IN_ORDER, &p);
  g_assert_cmpstr (result, ==, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

  p = result;
  g_tree_traverse (tree, traverse_func, G_PRE_ORDER, &p);
  g_assert_cmpstr (result, ==, "VF73102546B98ADCENJHGILKMRPOQTSUldZXWYbachfegjiktpnmorqsxvuwyz");

  p = result;
  g_tree_traverse (tree, traverse_func, G_POST_ORDER, &p);
  g_assert_cmpstr (result, ==, "02146538A9CEDB7GIHKMLJOQPSUTRNFWYXacbZegfikjhdmonqsrpuwvzyxtlV");

  g_tree_unref (tree);
  g_free (result);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/tree/search", test_tree_search);
  g_test_add_func ("/tree/remove", test_tree_remove);
  g_test_add_func ("/tree/destroy", test_tree_destroy);
  g_test_add_func ("/tree/traverse", test_tree_traverse);

  return g_test_run ();
}

