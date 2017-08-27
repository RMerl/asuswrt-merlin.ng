/* -*- mode: C; c-file-style: "gnu" -*- */
/* test-main.c  main() for make check
 *
 * Copyright (C) 2003 Red Hat, Inc.
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <config.h>
#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <dbus/dbus-string.h>
#include <dbus/dbus-sysdeps.h>
#include <dbus/dbus-internals.h>

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
static void
die (const char *failure)
{
  fprintf (stderr, "Unit test failed: %s\n", failure);
  exit (1);
}

static void
check_memleaks (const char *name)
{
  dbus_shutdown ();

  printf ("%s: checking for memleaks\n", name);
  if (_dbus_get_malloc_blocks_outstanding () != 0)
    {
      _dbus_warn ("%d dbus_malloc blocks were not freed\n",
                  _dbus_get_malloc_blocks_outstanding ());
      die ("memleaks");
    }
}
#endif /* DBUS_ENABLE_EMBEDDED_TESTS */

static void
test_pre_hook (void)
{
}

static char *progname = "";
static void
test_post_hook (void)
{
  check_memleaks (progname);
}

int
main (int argc, char **argv)
{
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
  const char *dir;
  DBusString test_data_dir;

  progname = argv[0];

  if (argc > 1)
    dir = argv[1];
  else
    dir = _dbus_getenv ("DBUS_TEST_DATA");

  if (dir == NULL)
    {
      fprintf (stderr, "Must specify test data directory as argv[1] or in DBUS_TEST_DATA env variable\n");
      return 1;
    }

  _dbus_string_init_const (&test_data_dir, dir);

  if (!_dbus_threads_init_debug ())
    die ("initializing debug threads");

  test_pre_hook ();
  printf ("%s: Running config file parser (trivial) test\n", argv[0]);
  if (!bus_config_parser_trivial_test (&test_data_dir))
    die ("parser");
  test_post_hook ();

  printf ("%s: Success\n", argv[0]);

  return 0;
#else /* DBUS_ENABLE_EMBEDDED_TESTS */

  printf ("Not compiled with test support\n");

  return 0;
#endif
}
