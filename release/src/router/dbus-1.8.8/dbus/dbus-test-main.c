/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-test.c  Program to run all tests
 *
 * Copyright (C) 2002  Red Hat Inc.
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
#include "dbus-types.h"
#include "dbus-test.h"
#include <stdio.h>
#include <stdlib.h>
#if HAVE_LOCALE_H
#include <locale.h>
#endif

int
main (int    argc,
      char **argv)
{
  const char *test_data_dir;
  const char *specific_test;

#if HAVE_SETLOCALE
  setlocale(LC_ALL, "");
#endif
  
  if (argc > 1)
    test_data_dir = argv[1];
  else
    test_data_dir = NULL;

  if (argc > 2)
    specific_test = argv[2];
  else
    specific_test = NULL;
  
  dbus_internal_do_not_use_run_tests (test_data_dir, specific_test);
  
  return 0;
}
