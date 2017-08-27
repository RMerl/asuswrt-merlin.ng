/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-credentials-util.c Would be in dbus-credentials.c, but only used for tests/bus
 *
 * Copyright (C) 2007 Red Hat Inc.
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
#include "dbus-internals.h"
#include "dbus-test.h"
#include "dbus-credentials.h"

/**
 * @addtogroup DBusCredentials
 * @{
 */

/** @} */

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
#include "dbus-test.h"
#include <stdio.h>
#include <string.h>

static DBusCredentials*
make_credentials(dbus_uid_t  unix_uid,
                 dbus_pid_t  pid,
                 const char *windows_sid)
{
  DBusCredentials *credentials;

  credentials = _dbus_credentials_new ();

  if (unix_uid != DBUS_UID_UNSET)
    {
      if (!_dbus_credentials_add_unix_uid (credentials, unix_uid))
        {
          _dbus_credentials_unref (credentials);
          return NULL;
        }
    }

  if (pid != DBUS_PID_UNSET)
    {
      if (!_dbus_credentials_add_pid (credentials, pid))
        {
          _dbus_credentials_unref (credentials);
          return NULL;
        }
    }

  if (windows_sid != NULL)
    {
      if (!_dbus_credentials_add_windows_sid (credentials, windows_sid))
        {
          _dbus_credentials_unref (credentials);
          return NULL;
        }
    }

  return credentials;
}

#define SAMPLE_SID "whatever a windows sid looks like"
#define OTHER_SAMPLE_SID "whatever else"

dbus_bool_t
_dbus_credentials_test (const char *test_data_dir)
{
  DBusCredentials *creds;
  DBusCredentials *creds2;
  
  if (test_data_dir == NULL)
    return TRUE;

  creds = make_credentials (12, 511, SAMPLE_SID);
  if (creds == NULL)
    _dbus_assert_not_reached ("oom");

  /* test refcounting */
  _dbus_credentials_ref (creds);
  _dbus_credentials_unref (creds);
  
  _dbus_assert (_dbus_credentials_include (creds, DBUS_CREDENTIAL_UNIX_USER_ID));
  _dbus_assert (_dbus_credentials_include (creds, DBUS_CREDENTIAL_UNIX_PROCESS_ID));
  _dbus_assert (_dbus_credentials_include (creds, DBUS_CREDENTIAL_WINDOWS_SID));

  _dbus_assert (_dbus_credentials_get_unix_uid (creds) == 12);
  _dbus_assert (_dbus_credentials_get_pid (creds) == 511);
  _dbus_assert (strcmp (_dbus_credentials_get_windows_sid (creds), SAMPLE_SID) == 0);

  _dbus_assert (!_dbus_credentials_are_empty (creds));
  _dbus_assert (!_dbus_credentials_are_anonymous (creds));

  /* Test copy */
  creds2 = _dbus_credentials_copy (creds);
  if (creds2 == NULL)
    _dbus_assert_not_reached ("oom");

  _dbus_assert (_dbus_credentials_include (creds2, DBUS_CREDENTIAL_UNIX_USER_ID));
  _dbus_assert (_dbus_credentials_include (creds2, DBUS_CREDENTIAL_UNIX_PROCESS_ID));
  _dbus_assert (_dbus_credentials_include (creds2, DBUS_CREDENTIAL_WINDOWS_SID));

  _dbus_assert (_dbus_credentials_get_unix_uid (creds2) == 12);
  _dbus_assert (_dbus_credentials_get_pid (creds2) == 511);
  _dbus_assert (strcmp (_dbus_credentials_get_windows_sid (creds2), SAMPLE_SID) == 0);  

  _dbus_assert (_dbus_credentials_are_superset (creds, creds2));
  
  _dbus_credentials_unref (creds2);
  
  /* Same user if both unix and windows are the same */
  creds2 = make_credentials (12, DBUS_PID_UNSET, SAMPLE_SID);
  if (creds2 == NULL)
    _dbus_assert_not_reached ("oom");

  _dbus_assert (_dbus_credentials_same_user (creds, creds2));

  _dbus_credentials_unref (creds2);

  /* Not the same user if Windows is missing */
  creds2 = make_credentials (12, DBUS_PID_UNSET, NULL);
  if (creds2 == NULL)
    _dbus_assert_not_reached ("oom");

  _dbus_assert (!_dbus_credentials_same_user (creds, creds2));
  _dbus_assert (_dbus_credentials_are_superset (creds, creds2));
  
  _dbus_credentials_unref (creds2);

  /* Not the same user if Windows is different */
  creds2 = make_credentials (12, DBUS_PID_UNSET, OTHER_SAMPLE_SID);
  if (creds2 == NULL)
    _dbus_assert_not_reached ("oom");

  _dbus_assert (!_dbus_credentials_same_user (creds, creds2));
  _dbus_assert (!_dbus_credentials_are_superset (creds, creds2));
  
  _dbus_credentials_unref (creds2);

  /* Not the same user if Unix is missing */
  creds2 = make_credentials (DBUS_UID_UNSET, DBUS_PID_UNSET, SAMPLE_SID);
  if (creds2 == NULL)
    _dbus_assert_not_reached ("oom");

  _dbus_assert (!_dbus_credentials_same_user (creds, creds2));
  _dbus_assert (_dbus_credentials_are_superset (creds, creds2));
  
  _dbus_credentials_unref (creds2);

  /* Not the same user if Unix is different */
  creds2 = make_credentials (15, DBUS_PID_UNSET, SAMPLE_SID);
  if (creds2 == NULL)
    _dbus_assert_not_reached ("oom");

  _dbus_assert (!_dbus_credentials_same_user (creds, creds2));
  _dbus_assert (!_dbus_credentials_are_superset (creds, creds2));
  
  _dbus_credentials_unref (creds2);

  /* Not the same user if both are missing */
  creds2 = make_credentials (DBUS_UID_UNSET, DBUS_PID_UNSET, NULL);
  if (creds2 == NULL)
    _dbus_assert_not_reached ("oom");

  _dbus_assert (!_dbus_credentials_same_user (creds, creds2));
  _dbus_assert (_dbus_credentials_are_superset (creds, creds2));
  
  _dbus_credentials_unref (creds2);

  /* Clearing credentials works */
  _dbus_credentials_clear (creds);

  _dbus_assert (!_dbus_credentials_include (creds, DBUS_CREDENTIAL_UNIX_USER_ID));
  _dbus_assert (!_dbus_credentials_include (creds, DBUS_CREDENTIAL_UNIX_PROCESS_ID));
  _dbus_assert (!_dbus_credentials_include (creds, DBUS_CREDENTIAL_WINDOWS_SID));

  _dbus_assert (_dbus_credentials_get_unix_uid (creds) == DBUS_UID_UNSET);
  _dbus_assert (_dbus_credentials_get_pid (creds) == DBUS_PID_UNSET);
  _dbus_assert (_dbus_credentials_get_windows_sid (creds) == NULL);

  _dbus_assert (_dbus_credentials_are_empty (creds));
  _dbus_assert (_dbus_credentials_are_anonymous (creds));

  _dbus_credentials_unref (creds);
  
  return TRUE;
}

#endif /* DBUS_ENABLE_EMBEDDED_TESTS */
