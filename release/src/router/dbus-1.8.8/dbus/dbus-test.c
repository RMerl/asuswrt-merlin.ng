/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-test.c  Program to run all tests
 *
 * Copyright (C) 2002, 2003, 2004, 2005  Red Hat Inc.
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
#include "dbus-test.h"
#include "dbus-sysdeps.h"
#include "dbus-internals.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
static void
die (const char *failure)
{
  fprintf (stderr, "Unit test failed: %s\n", failure);
  exit (1);
}

static void
check_memleaks (void)
{
  dbus_shutdown ();

  printf ("%s: checking for memleaks\n", "test-dbus");
  if (_dbus_get_malloc_blocks_outstanding () != 0)
    {
      _dbus_warn ("%d dbus_malloc blocks were not freed\n",
                  _dbus_get_malloc_blocks_outstanding ());
      die ("memleaks");
    }
}

typedef dbus_bool_t (*TestFunc)(void);
typedef dbus_bool_t (*TestDataFunc)(const char *data);

static void
run_test (const char             *test_name,
	  const char             *specific_test,
	  TestFunc                test)
{
  if (!specific_test || strcmp (specific_test, test_name) == 0)
    {
      printf ("%s: running %s tests\n", "test-dbus", test_name);
      if (!test ())
	die (test_name);

      check_memleaks ();
    }
}

static void
run_data_test (const char             *test_name,
	       const char             *specific_test,
	       TestDataFunc            test,
	       const char             *test_data_dir)
{
  if (!specific_test || strcmp (specific_test, test_name) == 0)
    {
      printf ("%s: running %s tests\n", "test-dbus", test_name);
      if (!test (test_data_dir))
	die (test_name);

      check_memleaks ();
    }
}

#endif /* DBUS_ENABLE_EMBEDDED_TESTS */

/**
 * An exported symbol to be run in order to execute
 * unit tests. Should not be used by
 * any app other than our test app, this symbol
 * won't exist in some builds of the library.
 * (with --enable-tests=no)
 *
 * @param test_data_dir the directory with test data (test/data normally)
 * @param specific_test run specific test or #NULL to run all tests
 */
void
dbus_internal_do_not_use_run_tests (const char *test_data_dir, const char *specific_test)
{
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
  if (!_dbus_threads_init_debug ())
    die ("debug threads init");
  
  if (test_data_dir == NULL)
    test_data_dir = _dbus_getenv ("DBUS_TEST_DATA");

  if (test_data_dir != NULL)
    printf ("Test data in %s\n", test_data_dir);
  else
    printf ("No test data!\n");

  run_test ("string", specific_test, _dbus_string_test);
  
  run_test ("sysdeps", specific_test, _dbus_sysdeps_test);
  
  run_test ("data-slot", specific_test, _dbus_data_slot_test);

  run_test ("misc", specific_test, _dbus_misc_test);
  
  run_test ("address", specific_test, _dbus_address_test);

  run_test ("server", specific_test, _dbus_server_test);

  run_test ("object-tree", specific_test, _dbus_object_tree_test);

  run_test ("signature", specific_test, _dbus_signature_test);
  
  run_test ("marshalling", specific_test, _dbus_marshal_test);

  run_test ("marshal-recursive", specific_test, _dbus_marshal_recursive_test);

  run_test ("byteswap", specific_test, _dbus_marshal_byteswap_test);
  
  run_test ("memory", specific_test, _dbus_memory_test);

#if 1
  run_test ("mem-pool", specific_test, _dbus_mem_pool_test);
#endif
  
  run_test ("list", specific_test, _dbus_list_test);

  run_test ("marshal-validate", specific_test, _dbus_marshal_validate_test);

  run_data_test ("message", specific_test, _dbus_message_test, test_data_dir);
  
  run_test ("hash", specific_test, _dbus_hash_test);

#if !defined(DBUS_WINCE)
  run_data_test ("spawn", specific_test, _dbus_spawn_test, test_data_dir);
#endif
  
  run_data_test ("credentials", specific_test, _dbus_credentials_test, test_data_dir);

#ifdef DBUS_UNIX
  run_data_test ("userdb", specific_test, _dbus_userdb_test, test_data_dir);

  run_test ("transport-unix", specific_test, _dbus_transport_unix_test);
#endif
  
  run_test ("keyring", specific_test, _dbus_keyring_test);

  run_data_test ("sha", specific_test, _dbus_sha_test, test_data_dir);
  
  run_data_test ("auth", specific_test, _dbus_auth_test, test_data_dir);

  printf ("%s: completed successfully\n", "test-dbus");
#else
  printf ("Not compiled with unit tests, not running any\n");
#endif
}

