/*
 * Copyright 2018 Collabora Ltd.
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
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <glib.h>

static void
test_pass (void)
{
}

static void
test_skip (void)
{
  g_test_skip ("not enough tea");
}

static void
test_fail (void)
{
  g_test_fail ();
}

static void
test_incomplete (void)
{
  g_test_incomplete ("mind reading not implemented yet");
}

int
main (int   argc,
      char *argv[])
{
  char *argv1;

  g_return_val_if_fail (argc > 1, 1);
  argv1 = argv[1];

  if (argc > 2)
    memmove (&argv[1], &argv[2], (argc - 2) * sizeof (char *));

  argc -= 1;
  argv[argc] = NULL;

  g_test_init (&argc, &argv, NULL);
  g_test_set_nonfatal_assertions ();

  if (g_strcmp0 (argv1, "pass") == 0)
    {
      g_test_add_func ("/pass", test_pass);
    }
  else if (g_strcmp0 (argv1, "skip") == 0)
    {
      g_test_add_func ("/skip", test_skip);
    }
  else if (g_strcmp0 (argv1, "incomplete") == 0)
    {
      g_test_add_func ("/incomplete", test_incomplete);
    }
  else if (g_strcmp0 (argv1, "fail") == 0)
    {
      g_test_add_func ("/fail", test_fail);
    }
  else if (g_strcmp0 (argv1, "all-non-failures") == 0)
    {
      g_test_add_func ("/pass", test_pass);
      g_test_add_func ("/skip", test_skip);
      g_test_add_func ("/incomplete", test_incomplete);
    }
  else if (g_strcmp0 (argv1, "all") == 0)
    {
      g_test_add_func ("/pass", test_pass);
      g_test_add_func ("/skip", test_skip);
      g_test_add_func ("/incomplete", test_incomplete);
      g_test_add_func ("/fail", test_fail);
    }
  else
    {
      g_assert_not_reached ();
    }

  return g_test_run ();
}
