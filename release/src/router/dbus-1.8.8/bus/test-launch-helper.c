/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* test-main.c  main() for the OOM check of the launch helper
 *
 * Copyright (C) 2007 Red Hat, Inc.
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
#include "activation-helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <dbus/dbus-internals.h>
#include <dbus/dbus-misc.h>

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

static void
test_post_hook (const char *name)
{
  check_memleaks (name);
}
#endif /* DBUS_ENABLE_EMBEDDED_TESTS */


#ifdef ACTIVATION_LAUNCHER_DO_OOM

/* returns true if good things happen, or if we get OOM */
static dbus_bool_t
bus_activation_helper_oom_test (void *data)
{
  const char *service;
  DBusError error;
  dbus_bool_t retval;

  service = (const char *) data;
  retval = TRUE;

  dbus_error_init (&error);
  if (!run_launch_helper (service, &error))
    {
      _DBUS_ASSERT_ERROR_IS_SET (&error);
      /* we failed, but a OOM is good */
      if (!dbus_error_has_name (&error, DBUS_ERROR_NO_MEMORY))
        {
          _dbus_warn ("FAILED SELF TEST: Error: %s\n", error.message);
          retval = FALSE;
        }
      dbus_error_free (&error);
    }
  else
    {
      /* we succeeded, yay! */
      _DBUS_ASSERT_ERROR_IS_CLEAR (&error);
    }
  return retval;
}

#endif

int
main (int argc, char **argv)
{
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
  const char *dir;
  DBusString config_file;

  if (argc > 1)
    dir = argv[1];
  else
    dir = _dbus_getenv ("DBUS_TEST_DATA");

  if (dir == NULL)
    {
      fprintf (stderr, "Must specify test data directory as argv[1] or in DBUS_TEST_DATA env variable\n");
      return 1;
    }

  printf ("%s: Running launch helper OOM checks\n", argv[0]);

  if (!_dbus_string_init (&config_file))
    return 1;
  if (!_dbus_string_append (&config_file, dir))
    return 1;
  if (!_dbus_string_append (&config_file, "/valid-config-files-system/debug-allow-all-pass.conf"))
    return 1;

  /* use a config file that will actually work... */
  dbus_setenv ("TEST_LAUNCH_HELPER_CONFIG",
               _dbus_string_get_const_data (&config_file));

  _dbus_string_free (&config_file);

  if (!_dbus_test_oom_handling ("dbus-daemon-launch-helper",
                                bus_activation_helper_oom_test,
                                "org.freedesktop.DBus.TestSuiteEchoService"))
    die ("OOM failed");

  test_post_hook (argv[0]);

  printf ("%s: Success\n", argv[0]);

  return 0;
#else /* DBUS_ENABLE_EMBEDDED_TESTS */

  printf ("Not compiled with test support\n");
  
  return 0;
#endif
}

