/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-userdb-util.c Would be in dbus-userdb.c, but not used in libdbus
 * 
 * Copyright (C) 2003, 2004, 2005  Red Hat, Inc.
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
#include <unistd.h>
#define DBUS_USERDB_INCLUDES_PRIVATE 1
#include "dbus-userdb.h"
#include "dbus-test.h"
#include "dbus-internals.h"
#include "dbus-protocol.h"
#include <string.h>

#if HAVE_SYSTEMD
#include <systemd/sd-login.h>
#endif

/**
 * @addtogroup DBusInternalsUtils
 * @{
 */

/**
 * Checks to see if the UID sent in is the console user
 *
 * @param uid UID of person to check 
 * @param error return location for errors
 * @returns #TRUE if the UID is the same as the console user and there are no errors
 */
dbus_bool_t
_dbus_is_console_user (dbus_uid_t uid,
		       DBusError *error)
{

  DBusUserDatabase *db;
  const DBusUserInfo *info;
  dbus_bool_t result = FALSE;

#ifdef HAVE_SYSTEMD
  /* check if we have logind */
  if (access ("/run/systemd/seats/", F_OK) >= 0)
    {
      int r;

      /* Check whether this user is logged in on at least one physical
         seat */
      r = sd_uid_get_seats (uid, 0, NULL);
      if (r < 0)
        {
          dbus_set_error (error, _dbus_error_from_errno (-r),
                          "Failed to determine seats of user \"" DBUS_UID_FORMAT "\": %s",
                          uid,
                          _dbus_strerror (-r));
          return FALSE;
        }

      return (r > 0);
    }
#endif

#ifdef HAVE_CONSOLE_OWNER_FILE

  DBusString f;
  DBusStat st;

  if (!_dbus_string_init (&f))
    {
      _DBUS_SET_OOM (error);
      return FALSE;
    }

  if (!_dbus_string_append(&f, DBUS_CONSOLE_OWNER_FILE))
    {
      _dbus_string_free(&f);
      _DBUS_SET_OOM (error);
      return FALSE;
    }

  if (_dbus_stat(&f, &st, NULL) && (st.uid == uid))
    {
      _dbus_string_free(&f);
      return TRUE;
    }

  _dbus_string_free(&f);

#endif /* HAVE_CONSOLE_OWNER_FILE */

  if (!_dbus_user_database_lock_system ())
    {
      _DBUS_SET_OOM (error);
      return FALSE;
    }

  db = _dbus_user_database_get_system ();
  if (db == NULL)
    {
      dbus_set_error (error, DBUS_ERROR_FAILED, "Could not get system database.");
      _dbus_user_database_unlock_system ();
      return FALSE;
    }

  /* TPTD: this should be cache-safe, we've locked the DB and
    _dbus_user_at_console doesn't pass it on. */
  info = _dbus_user_database_lookup (db, uid, NULL, error);

  if (info == NULL)
    {
      _dbus_user_database_unlock_system ();
       return FALSE;
    }

  result = _dbus_user_at_console (info->username, error);

  _dbus_user_database_unlock_system ();

  return result;
}

/**
 * Gets user ID given username
 *
 * @param username the username
 * @param uid return location for UID
 * @returns #TRUE if username existed and we got the UID
 */
dbus_bool_t
_dbus_get_user_id (const DBusString  *username,
                   dbus_uid_t        *uid)
{
  return _dbus_get_user_id_and_primary_group (username, uid, NULL);
}

/**
 * Gets group ID given groupname
 *
 * @param groupname the groupname
 * @param gid return location for GID
 * @returns #TRUE if group name existed and we got the GID
 */
dbus_bool_t
_dbus_get_group_id (const DBusString  *groupname,
                    dbus_gid_t        *gid)
{
  DBusUserDatabase *db;
  const DBusGroupInfo *info;

  /* FIXME: this can't distinguish ENOMEM from other errors */
  if (!_dbus_user_database_lock_system ())
    return FALSE;

  db = _dbus_user_database_get_system ();
  if (db == NULL)
    {
      _dbus_user_database_unlock_system ();
      return FALSE;
    }

  if (!_dbus_user_database_get_groupname (db, groupname,
                                          &info, NULL))
    {
      _dbus_user_database_unlock_system ();
      return FALSE;
    }

  *gid = info->gid;
  
  _dbus_user_database_unlock_system ();
  return TRUE;
}

/**
 * Gets user ID and primary group given username
 *
 * @param username the username
 * @param uid_p return location for UID
 * @param gid_p return location for GID
 * @returns #TRUE if username existed and we got the UID and GID
 */
dbus_bool_t
_dbus_get_user_id_and_primary_group (const DBusString  *username,
                                     dbus_uid_t        *uid_p,
                                     dbus_gid_t        *gid_p)
{
  DBusUserDatabase *db;
  const DBusUserInfo *info;

  /* FIXME: this can't distinguish ENOMEM from other errors */
  if (!_dbus_user_database_lock_system ())
    return FALSE;

  db = _dbus_user_database_get_system ();
  if (db == NULL)
    {
      _dbus_user_database_unlock_system ();
      return FALSE;
    }

  if (!_dbus_user_database_get_username (db, username,
                                         &info, NULL))
    {
      _dbus_user_database_unlock_system ();
      return FALSE;
    }

  if (uid_p)
    *uid_p = info->uid;
  if (gid_p)
    *gid_p = info->primary_gid;
  
  _dbus_user_database_unlock_system ();
  return TRUE;
}

/**
 * Looks up a gid or group name in the user database.  Only one of
 * name or GID can be provided. There are wrapper functions for this
 * that are better to use, this one does no locking or anything on the
 * database and otherwise sort of sucks.
 *
 * @param db the database
 * @param gid the group ID or #DBUS_GID_UNSET
 * @param groupname group name or #NULL 
 * @param error error to fill in
 * @returns the entry in the database
 */
DBusGroupInfo*
_dbus_user_database_lookup_group (DBusUserDatabase *db,
                                  dbus_gid_t        gid,
                                  const DBusString *groupname,
                                  DBusError        *error)
{
  DBusGroupInfo *info;

  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

   /* See if the group is really a number */
   if (gid == DBUS_UID_UNSET)
    {
      unsigned long n;

      if (_dbus_is_a_number (groupname, &n))
        gid = n;
    }

  if (gid != DBUS_GID_UNSET)
    info = _dbus_hash_table_lookup_uintptr (db->groups, gid);
  else
    info = _dbus_hash_table_lookup_string (db->groups_by_name,
                                           _dbus_string_get_const_data (groupname));
  if (info)
    {
      _dbus_verbose ("Using cache for GID "DBUS_GID_FORMAT" information\n",
                     info->gid);
      return info;
    }
  else
    {
      if (gid != DBUS_GID_UNSET)
	_dbus_verbose ("No cache for GID "DBUS_GID_FORMAT"\n",
		       gid);
      else
	_dbus_verbose ("No cache for groupname \"%s\"\n",
		       _dbus_string_get_const_data (groupname));
      
      info = dbus_new0 (DBusGroupInfo, 1);
      if (info == NULL)
        {
          dbus_set_error (error, DBUS_ERROR_NO_MEMORY, NULL);
          return NULL;
        }

      if (gid != DBUS_GID_UNSET)
        {
          if (!_dbus_group_info_fill_gid (info, gid, error))
            {
              _DBUS_ASSERT_ERROR_IS_SET (error);
              _dbus_group_info_free_allocated (info);
              return NULL;
            }
        }
      else
        {
          if (!_dbus_group_info_fill (info, groupname, error))
            {
              _DBUS_ASSERT_ERROR_IS_SET (error);
              _dbus_group_info_free_allocated (info);
              return NULL;
            }
        }

      /* don't use these past here */
      gid = DBUS_GID_UNSET;
      groupname = NULL;

      if (!_dbus_hash_table_insert_uintptr (db->groups, info->gid, info))
        {
          dbus_set_error (error, DBUS_ERROR_NO_MEMORY, NULL);
          _dbus_group_info_free_allocated (info);
          return NULL;
        }


      if (!_dbus_hash_table_insert_string (db->groups_by_name,
                                           info->groupname,
                                           info))
        {
          _dbus_hash_table_remove_uintptr (db->groups, info->gid);
          dbus_set_error (error, DBUS_ERROR_NO_MEMORY, NULL);
          return NULL;
        }
      
      return info;
    }
}


/**
 * Gets the user information for the given group name,
 * returned group info should not be freed. 
 *
 * @param db user database
 * @param groupname the group name
 * @param info return location for const ref to group info
 * @param error error location
 * @returns #FALSE if error is set
 */
dbus_bool_t
_dbus_user_database_get_groupname (DBusUserDatabase     *db,
                                   const DBusString     *groupname,
                                   const DBusGroupInfo **info,
                                   DBusError            *error)
{
  *info = _dbus_user_database_lookup_group (db, DBUS_GID_UNSET, groupname, error);
  return *info != NULL;
}

/**
 * Gets the user information for the given GID,
 * returned group info should not be freed. 
 *
 * @param db user database
 * @param gid the group ID
 * @param info return location for const ref to group info
 * @param error error location
 * @returns #FALSE if error is set
 */
dbus_bool_t
_dbus_user_database_get_gid (DBusUserDatabase     *db,
                             dbus_gid_t            gid,
                             const DBusGroupInfo **info,
                             DBusError            *error)
{
  *info = _dbus_user_database_lookup_group (db, gid, NULL, error);
  return *info != NULL;
}


/**
 * Gets all groups  corresponding to the given UID. Returns #FALSE
 * if no memory, or user isn't known, but always initializes
 * group_ids to a NULL array. 
 *
 * @param uid the UID
 * @param group_ids return location for array of group IDs
 * @param n_group_ids return location for length of returned array
 * @returns #TRUE if the UID existed and we got some credentials
 */
dbus_bool_t
_dbus_groups_from_uid (dbus_uid_t         uid,
                       dbus_gid_t       **group_ids,
                       int               *n_group_ids)
{
  DBusUserDatabase *db;
  const DBusUserInfo *info;
  *group_ids = NULL;
  *n_group_ids = 0;

  /* FIXME: this can't distinguish ENOMEM from other errors */
  if (!_dbus_user_database_lock_system ())
    return FALSE;

  db = _dbus_user_database_get_system ();
  if (db == NULL)
    {
      _dbus_user_database_unlock_system ();
      return FALSE;
    }

  if (!_dbus_user_database_get_uid (db, uid,
                                    &info, NULL))
    {
      _dbus_user_database_unlock_system ();
      return FALSE;
    }

  _dbus_assert (info->uid == uid);
  
  if (info->n_group_ids > 0)
    {
      *group_ids = dbus_new (dbus_gid_t, info->n_group_ids);
      if (*group_ids == NULL)
        {
	  _dbus_user_database_unlock_system ();
          return FALSE;
        }

      *n_group_ids = info->n_group_ids;

      memcpy (*group_ids, info->group_ids, info->n_group_ids * sizeof (dbus_gid_t));
    }

  _dbus_user_database_unlock_system ();
  return TRUE;
}
/** @} */

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
#include <stdio.h>

/**
 * Unit test for dbus-userdb.c.
 * 
 * @returns #TRUE on success.
 */
dbus_bool_t
_dbus_userdb_test (const char *test_data_dir)
{
  const DBusString *username;
  const DBusString *homedir;
  dbus_uid_t uid;
  unsigned long *group_ids;
  int n_group_ids, i;
  DBusError error;

  if (!_dbus_username_from_current_process (&username))
    _dbus_assert_not_reached ("didn't get username");

  if (!_dbus_homedir_from_current_process (&homedir))
    _dbus_assert_not_reached ("didn't get homedir");  

  if (!_dbus_get_user_id (username, &uid))
    _dbus_assert_not_reached ("didn't get uid");

  if (!_dbus_groups_from_uid (uid, &group_ids, &n_group_ids))
    _dbus_assert_not_reached ("didn't get groups");

  printf ("    Current user: %s homedir: %s gids:",
          _dbus_string_get_const_data (username),
          _dbus_string_get_const_data (homedir));

  for (i=0; i<n_group_ids; i++)
      printf(" %ld", group_ids[i]);

  printf ("\n");

  dbus_error_init (&error);
  printf ("Is Console user: %i\n",
          _dbus_is_console_user (uid, &error));
  printf ("Invocation was OK: %s\n", error.message ? error.message : "yes");
  dbus_error_free (&error);
  printf ("Is Console user 4711: %i\n",
          _dbus_is_console_user (4711, &error));
  printf ("Invocation was OK: %s\n", error.message ? error.message : "yes");
  dbus_error_free (&error);

  dbus_free (group_ids);

  return TRUE;
}
#endif /* DBUS_ENABLE_EMBEDDED_TESTS */
