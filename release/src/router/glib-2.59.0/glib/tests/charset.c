/*
 * Copyright 2018 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * See the included COPYING file for more information.
 */

#include "glib.h"

#define TEST_LOCALE "fr_FR.UTF-8@latin:en_US.UTF-8"

const gchar *TEST_RESULT[] = {
  "fr_FR.UTF-8@latin",
  "fr_FR@latin",
  "fr.UTF-8@latin",
  "fr@latin",
  "fr_FR.UTF-8",
  "fr_FR",
  "fr.UTF-8",
  "fr",
  "en_US.UTF-8",
  "en_US",
  "en.UTF-8",
  "en",
  "C",
  NULL
};

const gchar *TEST_TABLE[] = {
  "LANGUAGE",
  "LC_ALL",
  "LC_CTYPE",
  "LANG",
  NULL
};

static void
test_language_names_with_category (void)
{
  const gchar * const *language_names = NULL;
  gsize i, j;

  for (i = 0; TEST_TABLE[i]; ++i)
    {
      g_test_message ("Test %" G_GSIZE_FORMAT, i);
      g_assert_true (g_setenv (TEST_TABLE[i], TEST_LOCALE, TRUE));
      language_names = g_get_language_names_with_category ("LC_CTYPE");
      g_assert_cmpuint (g_strv_length ((gchar **)language_names), ==, g_strv_length ((gchar **)TEST_RESULT));

      for (j = 0; language_names[j]; ++j)
        {
          g_assert_cmpstr (language_names[j], ==, TEST_RESULT[j]);
        }
      g_unsetenv (TEST_TABLE[i]);
    }
}

static void
test_language_names_with_category_async (void)
{
  g_thread_join (g_thread_new (
      NULL, (GThreadFunc)g_get_language_names_with_category, "LC_CTYPE"));

  /* g_get_language_names_with_category returns a pointer to a memory
     which is owned by a thread it has been called from. The thread is dead now,
     therefore returned pointer can't be used at this stage.
  */
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_bug_base ("http://bugs.gnome.org/");

  g_test_add_func ("/charset/language_names_with_category", test_language_names_with_category);
  g_test_add_func ("/charset/language_names_with_category_async", test_language_names_with_category_async);

  return g_test_run ();
}
