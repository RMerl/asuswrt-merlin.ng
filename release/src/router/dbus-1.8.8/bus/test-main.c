/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
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
#include <dbus/dbus-message-internal.h>
#include "selinux.h"

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

static DBusInitialFDs *initial_fds = NULL;

static void
test_pre_hook (void)
{
  
  if (_dbus_getenv ("DBUS_TEST_SELINUX")
      && (!bus_selinux_pre_init ()
	  || !bus_selinux_full_init ()))
    die ("could not init selinux support");

  initial_fds = _dbus_check_fdleaks_enter ();
}

static char *progname = "";

static void
test_post_hook (void)
{
  if (_dbus_getenv ("DBUS_TEST_SELINUX"))
    bus_selinux_shutdown ();
  check_memleaks (progname);

  _dbus_check_fdleaks_leave (initial_fds);
  initial_fds = NULL;
}

int
main (int argc, char **argv)
{
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
  const char *dir;
  const char *only;
  DBusString test_data_dir;

  progname = argv[0];

  if (argc > 1)
    dir = argv[1];
  else
    dir = _dbus_getenv ("DBUS_TEST_DATA");

  if (argc > 2)
    only = argv[2];
  else
    only = NULL;

  if (dir == NULL)
    {
      fprintf (stderr, "Must specify test data directory as argv[1] or in DBUS_TEST_DATA env variable\n");
      return 1;
    }

  _dbus_string_init_const (&test_data_dir, dir);

  if (!_dbus_threads_init_debug ())
    die ("initializing debug threads");

  if (only == NULL || strcmp (only, "expire-list") == 0)
    {
      test_pre_hook ();
      printf ("%s: Running expire list test\n", argv[0]);
      if (!bus_expire_list_test (&test_data_dir))
        die ("expire list");
      test_post_hook ();
    }

  if (only == NULL || strcmp (only, "config-parser") == 0)
    {
      test_pre_hook ();
      printf ("%s: Running config file parser test\n", argv[0]);
      if (!bus_config_parser_test (&test_data_dir))
        die ("parser");
      test_post_hook ();
    }

  if (only == NULL || strcmp (only, "signals") == 0)
    {
      test_pre_hook ();
      printf ("%s: Running signals test\n", argv[0]);
      if (!bus_signals_test (&test_data_dir))
        die ("signals");
      test_post_hook ();
    }

  if (only == NULL || strcmp (only, "dispatch-sha1") == 0)
    {
      test_pre_hook ();
      printf ("%s: Running SHA1 connection test\n", argv[0]);
      if (!bus_dispatch_sha1_test (&test_data_dir))
        die ("sha1");
      test_post_hook ();
    }

  if (only == NULL || strcmp (only, "dispatch") == 0)
    {
      test_pre_hook ();
      printf ("%s: Running message dispatch test\n", argv[0]);
      if (!bus_dispatch_test (&test_data_dir)) 
        die ("dispatch");
      test_post_hook ();
    }

  if (only == NULL || strcmp (only, "activation-service-reload") == 0)
    {
      test_pre_hook ();
      printf ("%s: Running service files reloading test\n", argv[0]);
      if (!bus_activation_service_reload_test (&test_data_dir))
        die ("service reload");
      test_post_hook ();
    }

#ifdef HAVE_UNIX_FD_PASSING
  if (only == NULL || strcmp (only, "unix-fds-passing") == 0)
    {
      test_pre_hook ();
      printf ("%s: Running unix fd passing test\n", argv[0]);
      if (!bus_unix_fds_passing_test (&test_data_dir))
        die ("unix fd passing");
      test_post_hook ();
    }
#endif

  printf ("%s: Success\n", argv[0]);

  
  return 0;
#else /* DBUS_ENABLE_EMBEDDED_TESTS */

  printf ("Not compiled with test support\n");
  
  return 0;
#endif
}
