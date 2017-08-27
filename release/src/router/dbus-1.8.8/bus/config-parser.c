/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* config-parser.c  XML-library-agnostic configuration file parser
 *
 * Copyright (C) 2003, 2004 Red Hat, Inc.
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
#include "config-parser-common.h"
#include "config-parser.h"
#include "test.h"
#include "utils.h"
#include "policy.h"
#include "selinux.h"
#include <dbus/dbus-list.h>
#include <dbus/dbus-internals.h>
#include <dbus/dbus-misc.h>
#include <dbus/dbus-sysdeps.h>
#include <string.h>

typedef enum
{
  /* we ignore policies for unknown groups/users */
  POLICY_IGNORED,

  /* non-ignored */
  POLICY_DEFAULT,
  POLICY_MANDATORY,
  POLICY_USER,
  POLICY_GROUP,
  POLICY_CONSOLE
} PolicyType;

typedef struct
{
  ElementType type;

  unsigned int had_content : 1;

  union
  {
    struct
    {
      unsigned int ignore_missing : 1;
      unsigned int if_selinux_enabled : 1;
      unsigned int selinux_root_relative : 1;
    } include;

    struct
    {
      PolicyType type;
      unsigned long gid_uid_or_at_console;      
    } policy;

    struct
    {
      char *name;
      long value;
    } limit;
    
  } d;

} Element;

/**
 * Parser for bus configuration file. 
 */
struct BusConfigParser
{
  int refcount;        /**< Reference count */

  DBusString basedir;  /**< Directory we resolve paths relative to */
  
  DBusList *stack;     /**< stack of Element */

  char *user;          /**< user to run as */

  char *servicehelper; /**< location of the setuid helper */

  char *bus_type;          /**< Message bus type */
  
  DBusList *listen_on; /**< List of addresses to listen to */

  DBusList *mechanisms; /**< Auth mechanisms */

  DBusList *service_dirs; /**< Directories to look for session services in */

  DBusList *conf_dirs;   /**< Directories to look for policy configuration in */

  BusPolicy *policy;     /**< Security policy */

  BusLimits limits;      /**< Limits */

  char *pidfile;         /**< PID file */

  DBusList *included_files;  /**< Included files stack */

  DBusHashTable *service_context_table; /**< Map service names to SELinux contexts */

  unsigned int fork : 1; /**< TRUE to fork into daemon mode */

  unsigned int syslog : 1; /**< TRUE to enable syslog */
  unsigned int keep_umask : 1; /**< TRUE to keep original umask when forking */

  unsigned int is_toplevel : 1; /**< FALSE if we are a sub-config-file inside another one */

  unsigned int allow_anonymous : 1; /**< TRUE to allow anonymous connections */
};

static Element*
push_element (BusConfigParser *parser,
              ElementType      type)
{
  Element *e;

  _dbus_assert (type != ELEMENT_NONE);
  
  e = dbus_new0 (Element, 1);
  if (e == NULL)
    return NULL;

  if (!_dbus_list_append (&parser->stack, e))
    {
      dbus_free (e);
      return NULL;
    }
  
  e->type = type;

  return e;
}

static void
element_free (Element *e)
{
  if (e->type == ELEMENT_LIMIT)
    dbus_free (e->d.limit.name);
  
  dbus_free (e);
}

static void
pop_element (BusConfigParser *parser)
{
  Element *e;

  e = _dbus_list_pop_last (&parser->stack);
  
  element_free (e);
}

static Element*
peek_element (BusConfigParser *parser)
{
  Element *e;

  e = _dbus_list_get_last (&parser->stack);

  return e;
}

static ElementType
top_element_type (BusConfigParser *parser)
{
  Element *e;

  e = _dbus_list_get_last (&parser->stack);

  if (e)
    return e->type;
  else
    return ELEMENT_NONE;
}

static dbus_bool_t
merge_service_context_hash (DBusHashTable *dest,
			    DBusHashTable *from)
{
  DBusHashIter iter;
  char *service_copy;
  char *context_copy;

  service_copy = NULL;
  context_copy = NULL;

  _dbus_hash_iter_init (from, &iter);
  while (_dbus_hash_iter_next (&iter))
    {
      const char *service = _dbus_hash_iter_get_string_key (&iter);
      const char *context = _dbus_hash_iter_get_value (&iter);

      service_copy = _dbus_strdup (service);
      if (service_copy == NULL)
        goto fail;
      context_copy = _dbus_strdup (context);
      if (context_copy == NULL)
        goto fail; 
      
      if (!_dbus_hash_table_insert_string (dest, service_copy, context_copy))
        goto fail;

      service_copy = NULL;
      context_copy = NULL;    
    }

  return TRUE;

 fail:
  if (service_copy)
    dbus_free (service_copy);

  if (context_copy)
    dbus_free (context_copy);

  return FALSE;
}

static dbus_bool_t
service_dirs_find_dir (DBusList **service_dirs,
                       const char *dir)
{
  DBusList *link;

  _dbus_assert (dir != NULL);

  for (link = *service_dirs; link; link = _dbus_list_get_next_link(service_dirs, link))
    {
      const char *link_dir;
      
      link_dir = (const char *)link->data;
      if (strcmp (dir, link_dir) == 0)
        return TRUE;
    }

  return FALSE;
}

static dbus_bool_t
service_dirs_append_unique_or_free (DBusList **service_dirs,
                                    char *dir)
{
  if (!service_dirs_find_dir (service_dirs, dir))
    return _dbus_list_append (service_dirs, dir);  

  dbus_free (dir);
  return TRUE;
}

static void 
service_dirs_append_link_unique_or_free (DBusList **service_dirs,
                                         DBusList *dir_link)
{
  if (!service_dirs_find_dir (service_dirs, dir_link->data))
    {
      _dbus_list_append_link (service_dirs, dir_link);
    }
  else
    {
      dbus_free (dir_link->data);
      _dbus_list_free_link (dir_link);
    }
}

static dbus_bool_t
merge_included (BusConfigParser *parser,
                BusConfigParser *included,
                DBusError       *error)
{
  DBusList *link;

  if (!bus_policy_merge (parser->policy,
                         included->policy))
    {
      BUS_SET_OOM (error);
      return FALSE;
    }

  if (!merge_service_context_hash (parser->service_context_table,
				   included->service_context_table))
    {
      BUS_SET_OOM (error);
      return FALSE;
    }
  
  if (included->user != NULL)
    {
      dbus_free (parser->user);
      parser->user = included->user;
      included->user = NULL;
    }

  if (included->bus_type != NULL)
    {
      dbus_free (parser->bus_type);
      parser->bus_type = included->bus_type;
      included->bus_type = NULL;
    }
  
  if (included->fork)
    parser->fork = TRUE;

  if (included->keep_umask)
    parser->keep_umask = TRUE;

  if (included->allow_anonymous)
    parser->allow_anonymous = TRUE;

  if (included->pidfile != NULL)
    {
      dbus_free (parser->pidfile);
      parser->pidfile = included->pidfile;
      included->pidfile = NULL;
    }

  if (included->servicehelper != NULL)
    {
      dbus_free (parser->servicehelper);
      parser->servicehelper = included->servicehelper;
      included->servicehelper = NULL;
    }

  while ((link = _dbus_list_pop_first_link (&included->listen_on)))
    _dbus_list_append_link (&parser->listen_on, link);

  while ((link = _dbus_list_pop_first_link (&included->mechanisms)))
    _dbus_list_append_link (&parser->mechanisms, link);

  while ((link = _dbus_list_pop_first_link (&included->service_dirs)))
    service_dirs_append_link_unique_or_free (&parser->service_dirs, link);

  while ((link = _dbus_list_pop_first_link (&included->conf_dirs)))
    _dbus_list_append_link (&parser->conf_dirs, link);
  
  return TRUE;
}

static dbus_bool_t
seen_include (BusConfigParser  *parser,
	      const DBusString *file)
{
  DBusList *iter;

  iter = parser->included_files;
  while (iter != NULL)
    {
      if (! strcmp (_dbus_string_get_const_data (file), iter->data))
	return TRUE;

      iter = _dbus_list_get_next_link (&parser->included_files, iter);
    }

  return FALSE;
}

BusConfigParser*
bus_config_parser_new (const DBusString      *basedir,
                       dbus_bool_t            is_toplevel,
                       const BusConfigParser *parent)
{
  BusConfigParser *parser;

  parser = dbus_new0 (BusConfigParser, 1);
  if (parser == NULL)
    return NULL;

  parser->is_toplevel = !!is_toplevel;
  
  if (!_dbus_string_init (&parser->basedir))
    {
      dbus_free (parser);
      return NULL;
    }

  if (((parser->policy = bus_policy_new ()) == NULL) ||
      !_dbus_string_copy (basedir, 0, &parser->basedir, 0) ||
      ((parser->service_context_table = _dbus_hash_table_new (DBUS_HASH_STRING,
							      dbus_free,
							      dbus_free)) == NULL))
    {
      if (parser->policy)
        bus_policy_unref (parser->policy);
      
      _dbus_string_free (&parser->basedir);

      dbus_free (parser);
      return NULL;
    }

  if (parent != NULL)
    {
      /* Initialize the parser's limits from the parent. */
      parser->limits = parent->limits;

      /* Use the parent's list of included_files to avoid
	 circular inclusions. */
      parser->included_files = parent->included_files;
    }
  else
    {

      /* Make up some numbers! woot! */
      parser->limits.max_incoming_bytes = _DBUS_ONE_MEGABYTE * 127;
      parser->limits.max_outgoing_bytes = _DBUS_ONE_MEGABYTE * 127;
      parser->limits.max_message_size = _DBUS_ONE_MEGABYTE * 32;

      /* We set relatively conservative values here since due to the
      way SCM_RIGHTS works we need to preallocate an array for the
      maximum number of file descriptors we can receive. Picking a
      high value here thus translates directly to more memory
      allocation. */
      parser->limits.max_incoming_unix_fds = DBUS_DEFAULT_MESSAGE_UNIX_FDS*4;
      parser->limits.max_outgoing_unix_fds = DBUS_DEFAULT_MESSAGE_UNIX_FDS*4;
      parser->limits.max_message_unix_fds = DBUS_DEFAULT_MESSAGE_UNIX_FDS;
      
      /* Making this long means the user has to wait longer for an error
       * message if something screws up, but making it too short means
       * they might see a false failure.
       */
      parser->limits.activation_timeout = 25000; /* 25 seconds */

      /* Making this long risks making a DOS attack easier, but too short
       * and legitimate auth will fail.  If interactive auth (ask user for
       * password) is allowed, then potentially it has to be quite long.
       */
      parser->limits.auth_timeout = 5000; /* 5 seconds */

      /* Do not allow a fd to stay forever in dbus-daemon
       * https://bugs.freedesktop.org/show_bug.cgi?id=80559
       */
      parser->limits.pending_fd_timeout = 150000; /* 2.5 minutes */
      
      parser->limits.max_incomplete_connections = 64;
      parser->limits.max_connections_per_user = 256;
      
      /* Note that max_completed_connections / max_connections_per_user
       * is the number of users that would have to work together to
       * DOS all the other users.
       */
      parser->limits.max_completed_connections = 2048;
      
      parser->limits.max_pending_activations = 512;
      parser->limits.max_services_per_connection = 512;

      /* For this one, keep in mind that it isn't only the memory used
       * by the match rules, but slowdown from linearly walking a big
       * list of them. A client adding more than this is almost
       * certainly a bad idea for that reason, and should change to a
       * smaller number of wider-net match rules - getting every last
       * message to the bus is probably better than having a thousand
       * match rules.
       */
      parser->limits.max_match_rules_per_connection = 512;
      
      parser->limits.reply_timeout = -1; /* never */

      /* this is effectively a limit on message queue size for messages
       * that require a reply
       */
      parser->limits.max_replies_per_connection = 128;
    }
      
  parser->refcount = 1;
      
  return parser;
}

BusConfigParser *
bus_config_parser_ref (BusConfigParser *parser)
{
  _dbus_assert (parser->refcount > 0);

  parser->refcount += 1;

  return parser;
}

void
bus_config_parser_unref (BusConfigParser *parser)
{
  _dbus_assert (parser->refcount > 0);

  parser->refcount -= 1;

  if (parser->refcount == 0)
    {
      while (parser->stack != NULL)
        pop_element (parser);

      dbus_free (parser->user);
      dbus_free (parser->servicehelper);
      dbus_free (parser->bus_type);
      dbus_free (parser->pidfile);
      
      _dbus_list_foreach (&parser->listen_on,
                          (DBusForeachFunction) dbus_free,
                          NULL);

      _dbus_list_clear (&parser->listen_on);

      _dbus_list_foreach (&parser->service_dirs,
                          (DBusForeachFunction) dbus_free,
                          NULL);

      _dbus_list_clear (&parser->service_dirs);

      _dbus_list_foreach (&parser->conf_dirs,
                          (DBusForeachFunction) dbus_free,
                          NULL);

      _dbus_list_clear (&parser->conf_dirs);

      _dbus_list_foreach (&parser->mechanisms,
                          (DBusForeachFunction) dbus_free,
                          NULL);

      _dbus_list_clear (&parser->mechanisms);
      
      _dbus_string_free (&parser->basedir);

      if (parser->policy)
        bus_policy_unref (parser->policy);

      if (parser->service_context_table)
        _dbus_hash_table_unref (parser->service_context_table);
      
      dbus_free (parser);
    }
}

dbus_bool_t
bus_config_parser_check_doctype (BusConfigParser   *parser,
                                 const char        *doctype,
                                 DBusError         *error)
{
  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  if (strcmp (doctype, "busconfig") != 0)
    {
      dbus_set_error (error,
                      DBUS_ERROR_FAILED,
                      "Configuration file has the wrong document type %s",
                      doctype);
      return FALSE;
    }
  else
    return TRUE;
}

typedef struct
{
  const char  *name;
  const char **retloc;
} LocateAttr;

static dbus_bool_t
locate_attributes (BusConfigParser  *parser,
                   const char       *element_name,
                   const char      **attribute_names,
                   const char      **attribute_values,
                   DBusError        *error,
                   const char       *first_attribute_name,
                   const char      **first_attribute_retloc,
                   ...)
{
  va_list args;
  const char *name;
  const char **retloc;
  int n_attrs;
#define MAX_ATTRS 24
  LocateAttr attrs[MAX_ATTRS];
  dbus_bool_t retval;
  int i;

  _dbus_assert (first_attribute_name != NULL);
  _dbus_assert (first_attribute_retloc != NULL);

  retval = TRUE;

  n_attrs = 1;
  attrs[0].name = first_attribute_name;
  attrs[0].retloc = first_attribute_retloc;
  *first_attribute_retloc = NULL;

  va_start (args, first_attribute_retloc);

  name = va_arg (args, const char*);
  retloc = va_arg (args, const char**);

  while (name != NULL)
    {
      _dbus_assert (retloc != NULL);
      _dbus_assert (n_attrs < MAX_ATTRS);

      attrs[n_attrs].name = name;
      attrs[n_attrs].retloc = retloc;
      n_attrs += 1;
      *retloc = NULL;

      name = va_arg (args, const char*);
      retloc = va_arg (args, const char**);
    }

  va_end (args);

  i = 0;
  while (attribute_names[i])
    {
      int j;
      dbus_bool_t found;
      
      found = FALSE;
      j = 0;
      while (j < n_attrs)
        {
          if (strcmp (attrs[j].name, attribute_names[i]) == 0)
            {
              retloc = attrs[j].retloc;

              if (*retloc != NULL)
                {
                  dbus_set_error (error, DBUS_ERROR_FAILED,
                                  "Attribute \"%s\" repeated twice on the same <%s> element",
                                  attrs[j].name, element_name);
                  retval = FALSE;
                  goto out;
                }

              *retloc = attribute_values[i];
              found = TRUE;
            }

          ++j;
        }

      if (!found)
        {
          dbus_set_error (error, DBUS_ERROR_FAILED,
                          "Attribute \"%s\" is invalid on <%s> element in this context",
                          attribute_names[i], element_name);
          retval = FALSE;
          goto out;
        }

      ++i;
    }

 out:
  return retval;
}

static dbus_bool_t
check_no_attributes (BusConfigParser  *parser,
                     const char       *element_name,
                     const char      **attribute_names,
                     const char      **attribute_values,
                     DBusError        *error)
{
  if (attribute_names[0] != NULL)
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "Attribute \"%s\" is invalid on <%s> element in this context",
                      attribute_names[0], element_name);
      return FALSE;
    }

  return TRUE;
}

static dbus_bool_t
start_busconfig_child (BusConfigParser   *parser,
                       const char        *element_name,
                       const char       **attribute_names,
                       const char       **attribute_values,
                       DBusError         *error)
{
  ElementType element_type;

  element_type = bus_config_parser_element_name_to_type (element_name);

  if (element_type == ELEMENT_USER)
    {
      if (!check_no_attributes (parser, "user", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_USER) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      return TRUE;
    }
  else if (element_type == ELEMENT_CONFIGTYPE)
    {
      if (!check_no_attributes (parser, "type", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_CONFIGTYPE) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      return TRUE;
    }
  else if (element_type == ELEMENT_FORK)
    {
      if (!check_no_attributes (parser, "fork", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_FORK) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      parser->fork = TRUE;
      
      return TRUE;
    }
  else if (element_type == ELEMENT_SYSLOG)
    {
      if (!check_no_attributes (parser, "syslog", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_SYSLOG) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }
      
      parser->syslog = TRUE;
      
      return TRUE;
    }
  else if (element_type == ELEMENT_KEEP_UMASK)
    {
      if (!check_no_attributes (parser, "keep_umask", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_KEEP_UMASK) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      parser->keep_umask = TRUE;
      
      return TRUE;
    }
  else if (element_type == ELEMENT_PIDFILE)
    {
      if (!check_no_attributes (parser, "pidfile", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_PIDFILE) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      return TRUE;
    }
  else if (element_type == ELEMENT_LISTEN)
    {
      if (!check_no_attributes (parser, "listen", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_LISTEN) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      return TRUE;
    }
  else if (element_type == ELEMENT_AUTH)
    {
      if (!check_no_attributes (parser, "auth", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_AUTH) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }
      
      return TRUE;
    }
  else if (element_type == ELEMENT_SERVICEHELPER)
    {
      if (!check_no_attributes (parser, "servicehelper", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_SERVICEHELPER) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      return TRUE;
    }
  else if (element_type == ELEMENT_INCLUDEDIR)
    {
      if (!check_no_attributes (parser, "includedir", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_INCLUDEDIR) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      return TRUE;
    }
  else if (element_type == ELEMENT_STANDARD_SESSION_SERVICEDIRS)
    {
      DBusList *link;
      DBusList *dirs;
      dirs = NULL;

      if (!check_no_attributes (parser, "standard_session_servicedirs", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_STANDARD_SESSION_SERVICEDIRS) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      if (!_dbus_get_standard_session_servicedirs (&dirs))
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

        while ((link = _dbus_list_pop_first_link (&dirs)))
          service_dirs_append_link_unique_or_free (&parser->service_dirs, link);

      return TRUE;
    }
  else if (element_type == ELEMENT_STANDARD_SYSTEM_SERVICEDIRS)
    {
      DBusList *link;
      DBusList *dirs;
      dirs = NULL;

      if (!check_no_attributes (parser, "standard_system_servicedirs", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_STANDARD_SYSTEM_SERVICEDIRS) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      if (!_dbus_get_standard_system_servicedirs (&dirs))
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

        while ((link = _dbus_list_pop_first_link (&dirs)))
          service_dirs_append_link_unique_or_free (&parser->service_dirs, link);

      return TRUE;
    }
  else if (element_type == ELEMENT_ALLOW_ANONYMOUS)
    {
      if (!check_no_attributes (parser, "allow_anonymous", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_ALLOW_ANONYMOUS) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      parser->allow_anonymous = TRUE;
      return TRUE;
    }
  else if (element_type == ELEMENT_SERVICEDIR)
    {
      if (!check_no_attributes (parser, "servicedir", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_SERVICEDIR) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      return TRUE;
    }
  else if (element_type == ELEMENT_INCLUDE)
    {
      Element *e;
      const char *if_selinux_enabled;
      const char *ignore_missing;
      const char *selinux_root_relative;

      if ((e = push_element (parser, ELEMENT_INCLUDE)) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      e->d.include.ignore_missing = FALSE;
      e->d.include.if_selinux_enabled = FALSE;
      e->d.include.selinux_root_relative = FALSE;

      if (!locate_attributes (parser, "include",
                              attribute_names,
                              attribute_values,
                              error,
                              "ignore_missing", &ignore_missing,
                              "if_selinux_enabled", &if_selinux_enabled,
                              "selinux_root_relative", &selinux_root_relative,
                              NULL))
        return FALSE;

      if (ignore_missing != NULL)
        {
          if (strcmp (ignore_missing, "yes") == 0)
            e->d.include.ignore_missing = TRUE;
          else if (strcmp (ignore_missing, "no") == 0)
            e->d.include.ignore_missing = FALSE;
          else
            {
              dbus_set_error (error, DBUS_ERROR_FAILED,
                              "ignore_missing attribute must have value \"yes\" or \"no\"");
              return FALSE;
            }
        }

      if (if_selinux_enabled != NULL)
        {
          if (strcmp (if_selinux_enabled, "yes") == 0)
            e->d.include.if_selinux_enabled = TRUE;
          else if (strcmp (if_selinux_enabled, "no") == 0)
            e->d.include.if_selinux_enabled = FALSE;
          else
            {
              dbus_set_error (error, DBUS_ERROR_FAILED,
                              "if_selinux_enabled attribute must have value"
                              " \"yes\" or \"no\"");
              return FALSE;
	    }
        }
      
      if (selinux_root_relative != NULL)
        {
          if (strcmp (selinux_root_relative, "yes") == 0)
            e->d.include.selinux_root_relative = TRUE;
          else if (strcmp (selinux_root_relative, "no") == 0)
            e->d.include.selinux_root_relative = FALSE;
          else
            {
              dbus_set_error (error, DBUS_ERROR_FAILED,
                              "selinux_root_relative attribute must have value"
                              " \"yes\" or \"no\"");
              return FALSE;
	    }
        }

      return TRUE;
    }
  else if (element_type == ELEMENT_POLICY)
    {
      Element *e;
      const char *context;
      const char *user;
      const char *group;
      const char *at_console;

      if ((e = push_element (parser, ELEMENT_POLICY)) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      e->d.policy.type = POLICY_IGNORED;
      
      if (!locate_attributes (parser, "policy",
                              attribute_names,
                              attribute_values,
                              error,
                              "context", &context,
                              "user", &user,
                              "group", &group,
                              "at_console", &at_console,
                              NULL))
        return FALSE;

      if (((context && user) ||
           (context && group) ||
           (context && at_console)) ||
           ((user && group) ||
           (user && at_console)) ||
           (group && at_console) ||
          !(context || user || group || at_console))
        {
          dbus_set_error (error, DBUS_ERROR_FAILED,
                          "<policy> element must have exactly one of (context|user|group|at_console) attributes");
          return FALSE;
        }

      if (context != NULL)
        {
          if (strcmp (context, "default") == 0)
            {
              e->d.policy.type = POLICY_DEFAULT;
            }
          else if (strcmp (context, "mandatory") == 0)
            {
              e->d.policy.type = POLICY_MANDATORY;
            }
          else
            {
              dbus_set_error (error, DBUS_ERROR_FAILED,
                              "context attribute on <policy> must have the value \"default\" or \"mandatory\", not \"%s\"",
                              context);
              return FALSE;
            }
        }
      else if (user != NULL)
        {
          DBusString username;
          _dbus_string_init_const (&username, user);

          if (_dbus_parse_unix_user_from_config (&username,
                                                 &e->d.policy.gid_uid_or_at_console))
            e->d.policy.type = POLICY_USER;
          else
            _dbus_warn ("Unknown username \"%s\" in message bus configuration file\n",
                        user);
        }
      else if (group != NULL)
        {
          DBusString group_name;
          _dbus_string_init_const (&group_name, group);

          if (_dbus_parse_unix_group_from_config (&group_name,
                                                  &e->d.policy.gid_uid_or_at_console))
            e->d.policy.type = POLICY_GROUP;
          else
            _dbus_warn ("Unknown group \"%s\" in message bus configuration file\n",
                        group);          
        }
      else if (at_console != NULL)
        {
           dbus_bool_t t;
           t = (strcmp (at_console, "true") == 0);
           if (t || strcmp (at_console, "false") == 0)
             {
               e->d.policy.gid_uid_or_at_console = t; 
               e->d.policy.type = POLICY_CONSOLE;
             }  
           else
             {
               dbus_set_error (error, DBUS_ERROR_FAILED,
                              "Unknown value \"%s\" for at_console in message bus configuration file",
                              at_console);

               return FALSE;
             }
        }
      else
        {
          _dbus_assert_not_reached ("all <policy> attributes null and we didn't set error");
        }
      
      return TRUE;
    }
  else if (element_type == ELEMENT_LIMIT)
    {
      Element *e;
      const char *name;

      if ((e = push_element (parser, ELEMENT_LIMIT)) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }
      
      if (!locate_attributes (parser, "limit",
                              attribute_names,
                              attribute_values,
                              error,
                              "name", &name,
                              NULL))
        return FALSE;

      if (name == NULL)
        {
          dbus_set_error (error, DBUS_ERROR_FAILED,
                          "<limit> element must have a \"name\" attribute");
          return FALSE;
        }

      e->d.limit.name = _dbus_strdup (name);
      if (e->d.limit.name == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      return TRUE;
    }
  else if (element_type == ELEMENT_SELINUX)
    {
      if (!check_no_attributes (parser, "selinux", attribute_names, attribute_values, error))
        return FALSE;

      if (push_element (parser, ELEMENT_SELINUX) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      return TRUE;
    }
  else
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "Element <%s> not allowed inside <%s> in configuration file",
                      element_name, "busconfig");
      return FALSE;
    }
}

static dbus_bool_t
append_rule_from_element (BusConfigParser   *parser,
                          const char        *element_name,
                          const char       **attribute_names,
                          const char       **attribute_values,
                          dbus_bool_t        allow,
                          DBusError         *error)
{
  const char *log;
  const char *send_interface;
  const char *send_member;
  const char *send_error;
  const char *send_destination;
  const char *send_path;
  const char *send_type;
  const char *receive_interface;
  const char *receive_member;
  const char *receive_error;
  const char *receive_sender;
  const char *receive_path;
  const char *receive_type;
  const char *eavesdrop;
  const char *send_requested_reply;
  const char *receive_requested_reply;
  const char *own;
  const char *own_prefix;
  const char *user;
  const char *group;

  BusPolicyRule *rule;
  
  if (!locate_attributes (parser, element_name,
                          attribute_names,
                          attribute_values,
                          error,
                          "send_interface", &send_interface,
                          "send_member", &send_member,
                          "send_error", &send_error,
                          "send_destination", &send_destination,
                          "send_path", &send_path,
                          "send_type", &send_type,
                          "receive_interface", &receive_interface,
                          "receive_member", &receive_member,
                          "receive_error", &receive_error,
                          "receive_sender", &receive_sender,
                          "receive_path", &receive_path,
                          "receive_type", &receive_type,
                          "eavesdrop", &eavesdrop,
                          "send_requested_reply", &send_requested_reply,
                          "receive_requested_reply", &receive_requested_reply,
                          "own", &own,
                          "own_prefix", &own_prefix,
                          "user", &user,
                          "group", &group,
                          "log", &log,
                          NULL))
    return FALSE;

  if (!(send_interface || send_member || send_error || send_destination ||
        send_type || send_path ||
        receive_interface || receive_member || receive_error || receive_sender ||
        receive_type || receive_path || eavesdrop ||
        send_requested_reply || receive_requested_reply ||
        own || own_prefix || user || group))
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "Element <%s> must have one or more attributes",
                      element_name);
      return FALSE;
    }

  if ((send_member && (send_interface == NULL && send_path == NULL)) ||
      (receive_member && (receive_interface == NULL && receive_path == NULL)))
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "On element <%s>, if you specify a member you must specify an interface or a path. Keep in mind that not all messages have an interface field.",
                      element_name);
      return FALSE;
    }
  
  /* Allowed combinations of elements are:
   *
   *   base, must be all send or all receive:
   *     nothing
   *     interface
   *     interface + member
   *     error
   * 
   *   base send_ can combine with send_destination, send_path, send_type, send_requested_reply
   *   base receive_ with receive_sender, receive_path, receive_type, receive_requested_reply, eavesdrop
   *
   *   user, group, own, own_prefix must occur alone
   *
   * Pretty sure the below stuff is broken, FIXME think about it more.
   */

  if ((send_interface && (send_error ||
                          receive_interface ||
                          receive_member ||
                          receive_error ||
                          receive_sender ||
                          receive_requested_reply ||
                          own || own_prefix ||
                          user ||
                          group)) ||

      (send_member && (send_error ||
                       receive_interface ||
                       receive_member ||
                       receive_error ||
                       receive_sender ||
                       receive_requested_reply ||
                       own || own_prefix ||
                       user ||
                       group)) ||

      (send_error && (receive_interface ||
                      receive_member ||
                      receive_error ||
                      receive_sender ||
                      receive_requested_reply ||
                      own || own_prefix ||
                      user ||
                      group)) ||

      (send_destination && (receive_interface ||
                            receive_member ||
                            receive_error ||
                            receive_sender ||
                            receive_requested_reply ||
                            own || own_prefix ||
                            user ||
                            group)) ||

      (send_type && (receive_interface ||
                     receive_member ||
                     receive_error ||
                     receive_sender ||
                     receive_requested_reply ||
                     own || own_prefix ||
                     user ||
                     group)) ||

      (send_path && (receive_interface ||
                     receive_member ||
                     receive_error ||
                     receive_sender ||
                     receive_requested_reply ||
                     own || own_prefix ||
                     user ||
                     group)) ||

      (send_requested_reply && (receive_interface ||
                                receive_member ||
                                receive_error ||
                                receive_sender ||
                                receive_requested_reply ||
                                own || own_prefix ||
                                user ||
                                group)) ||

      (receive_interface && (receive_error ||
                             own || own_prefix ||
                             user ||
                             group)) ||

      (receive_member && (receive_error ||
                          own || own_prefix ||
                          user ||
                          group)) ||

      (receive_error && (own || own_prefix ||
                         user ||
                         group)) ||

      (eavesdrop && (own || own_prefix ||
                     user ||
                     group)) ||

      (receive_requested_reply && (own || own_prefix ||
                                   user ||
                                   group)) ||

      (own && (own_prefix || user || group)) ||

      (own_prefix && (own || user || group)) ||

      (user && group))
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "Invalid combination of attributes on element <%s>",
                      element_name);
      return FALSE;
    }
  
  rule = NULL;

  /* In BusPolicyRule, NULL represents wildcard.
   * In the config file, '*' represents it.
   */
#define IS_WILDCARD(str) ((str) && ((str)[0]) == '*' && ((str)[1]) == '\0')

  if (send_interface || send_member || send_error || send_destination ||
      send_path || send_type || send_requested_reply)
    {
      int message_type;
      
      if (IS_WILDCARD (send_interface))
        send_interface = NULL;
      if (IS_WILDCARD (send_member))
        send_member = NULL;
      if (IS_WILDCARD (send_error))
        send_error = NULL;
      if (IS_WILDCARD (send_destination))
        send_destination = NULL;
      if (IS_WILDCARD (send_path))
        send_path = NULL;
      if (IS_WILDCARD (send_type))
        send_type = NULL;

      message_type = DBUS_MESSAGE_TYPE_INVALID;
      if (send_type != NULL)
        {
          message_type = dbus_message_type_from_string (send_type);
          if (message_type == DBUS_MESSAGE_TYPE_INVALID)
            {
              dbus_set_error (error, DBUS_ERROR_FAILED,
                              "Bad message type \"%s\"",
                              send_type);
              return FALSE;
            }
        }

      if (eavesdrop &&
          !(strcmp (eavesdrop, "true") == 0 ||
            strcmp (eavesdrop, "false") == 0))
        {
          dbus_set_error (error, DBUS_ERROR_FAILED,
                          "Bad value \"%s\" for %s attribute, must be true or false",
                          "eavesdrop", eavesdrop);
          return FALSE;
        }

      if (send_requested_reply &&
          !(strcmp (send_requested_reply, "true") == 0 ||
            strcmp (send_requested_reply, "false") == 0))
        {
          dbus_set_error (error, DBUS_ERROR_FAILED,
                          "Bad value \"%s\" for %s attribute, must be true or false",
                          "send_requested_reply", send_requested_reply);
          return FALSE;
        }
      
      rule = bus_policy_rule_new (BUS_POLICY_RULE_SEND, allow); 
      if (rule == NULL)
        goto nomem;
      
      if (eavesdrop)
        rule->d.send.eavesdrop = (strcmp (eavesdrop, "true") == 0);

      if (log)
        rule->d.send.log = (strcmp (log, "true") == 0);

      if (send_requested_reply)
        rule->d.send.requested_reply = (strcmp (send_requested_reply, "true") == 0);

      rule->d.send.message_type = message_type;
      rule->d.send.path = _dbus_strdup (send_path);
      rule->d.send.interface = _dbus_strdup (send_interface);
      rule->d.send.member = _dbus_strdup (send_member);
      rule->d.send.error = _dbus_strdup (send_error);
      rule->d.send.destination = _dbus_strdup (send_destination);
      if (send_path && rule->d.send.path == NULL)
        goto nomem;
      if (send_interface && rule->d.send.interface == NULL)
        goto nomem;
      if (send_member && rule->d.send.member == NULL)
        goto nomem;
      if (send_error && rule->d.send.error == NULL)
        goto nomem;
      if (send_destination && rule->d.send.destination == NULL)
        goto nomem;
    }
  else if (receive_interface || receive_member || receive_error || receive_sender ||
           receive_path || receive_type || eavesdrop || receive_requested_reply)
    {
      int message_type;
      
      if (IS_WILDCARD (receive_interface))
        receive_interface = NULL;
      if (IS_WILDCARD (receive_member))
        receive_member = NULL;
      if (IS_WILDCARD (receive_error))
        receive_error = NULL;
      if (IS_WILDCARD (receive_sender))
        receive_sender = NULL;
      if (IS_WILDCARD (receive_path))
        receive_path = NULL;
      if (IS_WILDCARD (receive_type))
        receive_type = NULL;

      message_type = DBUS_MESSAGE_TYPE_INVALID;
      if (receive_type != NULL)
        {
          message_type = dbus_message_type_from_string (receive_type);
          if (message_type == DBUS_MESSAGE_TYPE_INVALID)
            {
              dbus_set_error (error, DBUS_ERROR_FAILED,
                              "Bad message type \"%s\"",
                              receive_type);
              return FALSE;
            }
        }


      if (eavesdrop &&
          !(strcmp (eavesdrop, "true") == 0 ||
            strcmp (eavesdrop, "false") == 0))
        {
          dbus_set_error (error, DBUS_ERROR_FAILED,
                          "Bad value \"%s\" for %s attribute, must be true or false",
                          "eavesdrop", eavesdrop);
          return FALSE;
        }

      if (receive_requested_reply &&
          !(strcmp (receive_requested_reply, "true") == 0 ||
            strcmp (receive_requested_reply, "false") == 0))
        {
          dbus_set_error (error, DBUS_ERROR_FAILED,
                          "Bad value \"%s\" for %s attribute, must be true or false",
                          "receive_requested_reply", receive_requested_reply);
          return FALSE;
        }
      
      rule = bus_policy_rule_new (BUS_POLICY_RULE_RECEIVE, allow); 
      if (rule == NULL)
        goto nomem;

      if (eavesdrop)
        rule->d.receive.eavesdrop = (strcmp (eavesdrop, "true") == 0);

      if (receive_requested_reply)
        rule->d.receive.requested_reply = (strcmp (receive_requested_reply, "true") == 0);
      
      rule->d.receive.message_type = message_type;
      rule->d.receive.path = _dbus_strdup (receive_path);
      rule->d.receive.interface = _dbus_strdup (receive_interface);
      rule->d.receive.member = _dbus_strdup (receive_member);
      rule->d.receive.error = _dbus_strdup (receive_error);
      rule->d.receive.origin = _dbus_strdup (receive_sender);

      if (receive_path && rule->d.receive.path == NULL)
        goto nomem;
      if (receive_interface && rule->d.receive.interface == NULL)
        goto nomem;
      if (receive_member && rule->d.receive.member == NULL)
        goto nomem;
      if (receive_error && rule->d.receive.error == NULL)
        goto nomem;
      if (receive_sender && rule->d.receive.origin == NULL)
        goto nomem;
    }
  else if (own || own_prefix)
    {
      rule = bus_policy_rule_new (BUS_POLICY_RULE_OWN, allow); 
      if (rule == NULL)
        goto nomem;

      if (own)
        {
          if (IS_WILDCARD (own))
            own = NULL;
      
          rule->d.own.prefix = 0;
          rule->d.own.service_name = _dbus_strdup (own);
          if (own && rule->d.own.service_name == NULL)
            goto nomem;
        }
      else
        {
          rule->d.own.prefix = 1;
          rule->d.own.service_name = _dbus_strdup (own_prefix);
          if (rule->d.own.service_name == NULL)
            goto nomem;
        }
    }
  else if (user)
    {      
      if (IS_WILDCARD (user))
        {
          rule = bus_policy_rule_new (BUS_POLICY_RULE_USER, allow); 
          if (rule == NULL)
            goto nomem;

          rule->d.user.uid = DBUS_UID_UNSET;
        }
      else
        {
          DBusString username;
          dbus_uid_t uid;
          
          _dbus_string_init_const (&username, user);
      
          if (_dbus_parse_unix_user_from_config (&username, &uid))
            {
              rule = bus_policy_rule_new (BUS_POLICY_RULE_USER, allow); 
              if (rule == NULL)
                goto nomem;

              rule->d.user.uid = uid;
            }
          else
            {
              _dbus_warn ("Unknown username \"%s\" on element <%s>\n",
                          user, element_name);
            }
        }
    }
  else if (group)
    {
      if (IS_WILDCARD (group))
        {
          rule = bus_policy_rule_new (BUS_POLICY_RULE_GROUP, allow); 
          if (rule == NULL)
            goto nomem;

          rule->d.group.gid = DBUS_GID_UNSET;
        }
      else
        {
          DBusString groupname;
          dbus_gid_t gid;
          
          _dbus_string_init_const (&groupname, group);
          
          if (_dbus_parse_unix_group_from_config (&groupname, &gid))
            {
              rule = bus_policy_rule_new (BUS_POLICY_RULE_GROUP, allow); 
              if (rule == NULL)
                goto nomem;

              rule->d.group.gid = gid;
            }
          else
            {
              _dbus_warn ("Unknown group \"%s\" on element <%s>\n",
                          group, element_name);
            }
        }
    }
  else
    _dbus_assert_not_reached ("Did not handle some combination of attributes on <allow> or <deny>");

  if (rule != NULL)
    {
      Element *pe;
      
      pe = peek_element (parser);      
      _dbus_assert (pe != NULL);
      _dbus_assert (pe->type == ELEMENT_POLICY);

      switch (pe->d.policy.type)
        {
        case POLICY_IGNORED:
          /* drop the rule on the floor */
          break;
          
        case POLICY_DEFAULT:
          if (!bus_policy_append_default_rule (parser->policy, rule))
            goto nomem;
          break;
        case POLICY_MANDATORY:
          if (!bus_policy_append_mandatory_rule (parser->policy, rule))
            goto nomem;
          break;
        case POLICY_USER:
          if (!BUS_POLICY_RULE_IS_PER_CLIENT (rule))
            {
              dbus_set_error (error, DBUS_ERROR_FAILED,
                              "<%s> rule cannot be per-user because it has bus-global semantics",
                              element_name);
              goto failed;
            }
          
          if (!bus_policy_append_user_rule (parser->policy, pe->d.policy.gid_uid_or_at_console,
                                            rule))
            goto nomem;
          break;
        case POLICY_GROUP:
          if (!BUS_POLICY_RULE_IS_PER_CLIENT (rule))
            {
              dbus_set_error (error, DBUS_ERROR_FAILED,
                              "<%s> rule cannot be per-group because it has bus-global semantics",
                              element_name);
              goto failed;
            }
          
          if (!bus_policy_append_group_rule (parser->policy, pe->d.policy.gid_uid_or_at_console,
                                             rule))
            goto nomem;
          break;
        

        case POLICY_CONSOLE:
          if (!bus_policy_append_console_rule (parser->policy, pe->d.policy.gid_uid_or_at_console,
                                               rule))
            goto nomem;
          break;
        }
 
      bus_policy_rule_unref (rule);
      rule = NULL;
    }
  
  return TRUE;

 nomem:
  BUS_SET_OOM (error);
 failed:
  if (rule)
    bus_policy_rule_unref (rule);
  return FALSE;
}

static dbus_bool_t
start_policy_child (BusConfigParser   *parser,
                    const char        *element_name,
                    const char       **attribute_names,
                    const char       **attribute_values,
                    DBusError         *error)
{
  if (strcmp (element_name, "allow") == 0)
    {
      if (!append_rule_from_element (parser, element_name,
                                     attribute_names, attribute_values,
                                     TRUE, error))
        return FALSE;
      
      if (push_element (parser, ELEMENT_ALLOW) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }
      
      return TRUE;
    }
  else if (strcmp (element_name, "deny") == 0)
    {
      if (!append_rule_from_element (parser, element_name,
                                     attribute_names, attribute_values,
                                     FALSE, error))
        return FALSE;
      
      if (push_element (parser, ELEMENT_DENY) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }
      
      return TRUE;
    }
  else
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "Element <%s> not allowed inside <%s> in configuration file",
                      element_name, "policy");
      return FALSE;
    }
}

static dbus_bool_t
start_selinux_child (BusConfigParser   *parser,
                     const char        *element_name,
                     const char       **attribute_names,
                     const char       **attribute_values,
                     DBusError         *error)
{
  char *own_copy;
  char *context_copy;

  own_copy = NULL;
  context_copy = NULL;

  if (strcmp (element_name, "associate") == 0)
    {
      const char *own;
      const char *context;
      
      if (!locate_attributes (parser, "associate",
                              attribute_names,
                              attribute_values,
                              error,
                              "own", &own,
                              "context", &context,
                              NULL))
        return FALSE;
      
      if (push_element (parser, ELEMENT_ASSOCIATE) == NULL)
        {
          BUS_SET_OOM (error);
          return FALSE;
        }

      if (own == NULL || context == NULL)
        {
          dbus_set_error (error, DBUS_ERROR_FAILED,
                          "Element <associate> must have attributes own=\"<servicename>\" and context=\"<selinux context>\"");
          return FALSE;
        }

      own_copy = _dbus_strdup (own);
      if (own_copy == NULL)
        goto oom;
      context_copy = _dbus_strdup (context);
      if (context_copy == NULL)
        goto oom;

      if (!_dbus_hash_table_insert_string (parser->service_context_table,
					   own_copy, context_copy))
        goto oom;

      return TRUE;
    }
  else
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "Element <%s> not allowed inside <%s> in configuration file",
                      element_name, "selinux");
      return FALSE;
    }

 oom:
  if (own_copy)
    dbus_free (own_copy);

  if (context_copy)  
    dbus_free (context_copy);

  BUS_SET_OOM (error);
  return FALSE;
}

dbus_bool_t
bus_config_parser_start_element (BusConfigParser   *parser,
                                 const char        *element_name,
                                 const char       **attribute_names,
                                 const char       **attribute_values,
                                 DBusError         *error)
{
  ElementType t;

  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  /* printf ("START: %s\n", element_name); */
  
  t = top_element_type (parser);

  if (t == ELEMENT_NONE)
    {
      if (strcmp (element_name, "busconfig") == 0)
        {
          if (!check_no_attributes (parser, "busconfig", attribute_names, attribute_values, error))
            return FALSE;
          
          if (push_element (parser, ELEMENT_BUSCONFIG) == NULL)
            {
              BUS_SET_OOM (error);
              return FALSE;
            }

          return TRUE;
        }
      else
        {
          dbus_set_error (error, DBUS_ERROR_FAILED,
                          "Unknown element <%s> at root of configuration file",
                          element_name);
          return FALSE;
        }
    }
  else if (t == ELEMENT_BUSCONFIG)
    {
      return start_busconfig_child (parser, element_name,
                                    attribute_names, attribute_values,
                                    error);
    }
  else if (t == ELEMENT_POLICY)
    {
      return start_policy_child (parser, element_name,
                                 attribute_names, attribute_values,
                                 error);
    }
  else if (t == ELEMENT_SELINUX)
    {
      return start_selinux_child (parser, element_name,
                                  attribute_names, attribute_values,
                                  error);
    }
  else
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "Element <%s> is not allowed in this context",
                      element_name);
      return FALSE;
    }  
}

static dbus_bool_t
set_limit (BusConfigParser *parser,
           const char      *name,
           long             value,
           DBusError       *error)
{
  dbus_bool_t must_be_positive;
  dbus_bool_t must_be_int;

  must_be_int = FALSE;
  must_be_positive = FALSE;
  
  if (strcmp (name, "max_incoming_bytes") == 0)
    {
      must_be_positive = TRUE;
      parser->limits.max_incoming_bytes = value;
    }
  else if (strcmp (name, "max_incoming_unix_fds") == 0)
    {
      must_be_positive = TRUE;
      parser->limits.max_incoming_unix_fds = value;
    }
  else if (strcmp (name, "max_outgoing_bytes") == 0)
    {
      must_be_positive = TRUE;
      parser->limits.max_outgoing_bytes = value;
    }
  else if (strcmp (name, "max_outgoing_unix_fds") == 0)
    {
      must_be_positive = TRUE;
      parser->limits.max_outgoing_unix_fds = value;
    }
  else if (strcmp (name, "max_message_size") == 0)
    {
      must_be_positive = TRUE;
      parser->limits.max_message_size = value;
    }
  else if (strcmp (name, "max_message_unix_fds") == 0)
    {
      must_be_positive = TRUE;
      parser->limits.max_message_unix_fds = value;
    }
  else if (strcmp (name, "service_start_timeout") == 0)
    {
      must_be_positive = TRUE;
      must_be_int = TRUE;
      parser->limits.activation_timeout = value;
    }
  else if (strcmp (name, "auth_timeout") == 0)
    {
      must_be_positive = TRUE;
      must_be_int = TRUE;
      parser->limits.auth_timeout = value;
    }
  else if (strcmp (name, "pending_fd_timeout") == 0)
    {
      must_be_positive = TRUE;
      must_be_int = TRUE;
      parser->limits.pending_fd_timeout = value;
    }
  else if (strcmp (name, "reply_timeout") == 0)
    {
      must_be_positive = TRUE;
      must_be_int = TRUE;
      parser->limits.reply_timeout = value;
    }
  else if (strcmp (name, "max_completed_connections") == 0)
    {
      must_be_positive = TRUE;
      must_be_int = TRUE;
      parser->limits.max_completed_connections = value;
    }
  else if (strcmp (name, "max_incomplete_connections") == 0)
    {
      must_be_positive = TRUE;
      must_be_int = TRUE;
      parser->limits.max_incomplete_connections = value;
    }
  else if (strcmp (name, "max_connections_per_user") == 0)
    {
      must_be_positive = TRUE;
      must_be_int = TRUE;
      parser->limits.max_connections_per_user = value;
    }
  else if (strcmp (name, "max_pending_service_starts") == 0)
    {
      must_be_positive = TRUE;
      must_be_int = TRUE;
      parser->limits.max_pending_activations = value;
    }
  else if (strcmp (name, "max_names_per_connection") == 0)
    {
      must_be_positive = TRUE;
      must_be_int = TRUE;
      parser->limits.max_services_per_connection = value;
    }
  else if (strcmp (name, "max_match_rules_per_connection") == 0)
    {
      must_be_positive = TRUE;
      must_be_int = TRUE;
      parser->limits.max_match_rules_per_connection = value;
    }
  else if (strcmp (name, "max_replies_per_connection") == 0)
    {
      must_be_positive = TRUE;
      must_be_int = TRUE;
      parser->limits.max_replies_per_connection = value;
    }
  else
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "There is no limit called \"%s\"\n",
                      name);
      return FALSE;
    }
  
  if (must_be_positive && value < 0)
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "<limit name=\"%s\"> must be a positive number\n",
                      name);
      return FALSE;
    }

  if (must_be_int &&
      (value < _DBUS_INT_MIN || value > _DBUS_INT_MAX))
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "<limit name=\"%s\"> value is too large\n",
                      name);
      return FALSE;
    }

  return TRUE;  
}

dbus_bool_t
bus_config_parser_end_element (BusConfigParser   *parser,
                               const char        *element_name,
                               DBusError         *error)
{
  ElementType t;
  const char *n;
  Element *e;

  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  /* printf ("END: %s\n", element_name); */
  
  t = top_element_type (parser);

  if (t == ELEMENT_NONE)
    {
      /* should probably be an assertion failure but
       * being paranoid about XML parsers
       */
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "XML parser ended element with no element on the stack");
      return FALSE;
    }

  n = bus_config_parser_element_type_to_name (t);
  _dbus_assert (n != NULL);
  if (strcmp (n, element_name) != 0)
    {
      /* should probably be an assertion failure but
       * being paranoid about XML parsers
       */
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "XML element <%s> ended but topmost element on the stack was <%s>",
                      element_name, n);
      return FALSE;
    }

  e = peek_element (parser);
  _dbus_assert (e != NULL);

  switch (e->type)
    {
    case ELEMENT_NONE:
      _dbus_assert_not_reached ("element in stack has no type");
      break;

    case ELEMENT_INCLUDE:
    case ELEMENT_USER:
    case ELEMENT_CONFIGTYPE:
    case ELEMENT_LISTEN:
    case ELEMENT_PIDFILE:
    case ELEMENT_AUTH:
    case ELEMENT_SERVICEDIR:
    case ELEMENT_SERVICEHELPER:
    case ELEMENT_INCLUDEDIR:
    case ELEMENT_LIMIT:
      if (!e->had_content)
        {
          dbus_set_error (error, DBUS_ERROR_FAILED,
                          "XML element <%s> was expected to have content inside it",
                          bus_config_parser_element_type_to_name (e->type));
          return FALSE;
        }

      if (e->type == ELEMENT_LIMIT)
        {
          if (!set_limit (parser, e->d.limit.name, e->d.limit.value,
                          error))
            return FALSE;
        }
      break;

    case ELEMENT_BUSCONFIG:
    case ELEMENT_POLICY:
    case ELEMENT_ALLOW:
    case ELEMENT_DENY:
    case ELEMENT_FORK:
    case ELEMENT_SYSLOG:
    case ELEMENT_KEEP_UMASK:
    case ELEMENT_SELINUX:
    case ELEMENT_ASSOCIATE:
    case ELEMENT_STANDARD_SESSION_SERVICEDIRS:
    case ELEMENT_STANDARD_SYSTEM_SERVICEDIRS:
    case ELEMENT_ALLOW_ANONYMOUS:
      break;
    }

  pop_element (parser);

  return TRUE;
}

static dbus_bool_t
all_whitespace (const DBusString *str)
{
  int i;

  _dbus_string_skip_white (str, 0, &i);

  return i == _dbus_string_get_length (str);
}

static dbus_bool_t
make_full_path (const DBusString *basedir,
                const DBusString *filename,
                DBusString       *full_path)
{
  if (_dbus_path_is_absolute (filename))
    {
      return _dbus_string_copy (filename, 0, full_path, 0);
    }
  else
    {
      if (!_dbus_string_copy (basedir, 0, full_path, 0))
        return FALSE;
      
      if (!_dbus_concat_dir_and_file (full_path, filename))
        return FALSE;

      return TRUE;
    }
}

static dbus_bool_t
include_file (BusConfigParser   *parser,
              const DBusString  *filename,
              dbus_bool_t        ignore_missing,
              DBusError         *error)
{
  /* FIXME good test case for this would load each config file in the
   * test suite both alone, and as an include, and check
   * that the result is the same
   */
  BusConfigParser *included;
  const char *filename_str;
  DBusError tmp_error;
        
  dbus_error_init (&tmp_error);

  filename_str = _dbus_string_get_const_data (filename);

  /* Check to make sure this file hasn't already been included. */
  if (seen_include (parser, filename))
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
		      "Circular inclusion of file '%s'",
		      filename_str);
      return FALSE;
    }
  
  if (! _dbus_list_append (&parser->included_files, (void *) filename_str))
    {
      BUS_SET_OOM (error);
      return FALSE;
    }

  /* Since parser is passed in as the parent, included
     inherits parser's limits. */
  included = bus_config_load (filename, FALSE, parser, &tmp_error);

  _dbus_list_pop_last (&parser->included_files);

  if (included == NULL)
    {
      _DBUS_ASSERT_ERROR_IS_SET (&tmp_error);

      if (dbus_error_has_name (&tmp_error, DBUS_ERROR_FILE_NOT_FOUND) &&
          ignore_missing)
        {
          dbus_error_free (&tmp_error);
          return TRUE;
        }
      else
        {
          dbus_move_error (&tmp_error, error);
          return FALSE;
        }
    }
  else
    {
      _DBUS_ASSERT_ERROR_IS_CLEAR (&tmp_error);

      if (!merge_included (parser, included, error))
        {
          bus_config_parser_unref (included);
          return FALSE;
        }

      /* Copy included's limits back to parser. */
      parser->limits = included->limits;

      bus_config_parser_unref (included);
      return TRUE;
    }
}

static dbus_bool_t
servicehelper_path (BusConfigParser   *parser,
                    const DBusString  *filename,
                    DBusError         *error)
{
  const char *filename_str;
  char *servicehelper;

  filename_str = _dbus_string_get_const_data (filename);

  /* copy to avoid overwriting with NULL on OOM */
  servicehelper = _dbus_strdup (filename_str);

  /* check for OOM */
  if (servicehelper == NULL)
    {
      BUS_SET_OOM (error);
      return FALSE;
    }

  /* save the latest servicehelper only if not OOM */
  dbus_free (parser->servicehelper);
  parser->servicehelper = servicehelper;

  /* We don't check whether the helper exists; instead we
   * would just fail to ever activate anything if it doesn't.
   * This allows an admin to fix the problem if it doesn't exist.
   * It also allows the parser test suite to successfully parse
   * test cases without installing the helper. ;-)
   */
  
  return TRUE;
}

static dbus_bool_t
include_dir (BusConfigParser   *parser,
             const DBusString  *dirname,
             DBusError         *error)
{
  DBusString filename;
  dbus_bool_t retval;
  DBusError tmp_error;
  DBusDirIter *dir;
  char *s;
  
  if (!_dbus_string_init (&filename))
    {
      BUS_SET_OOM (error);
      return FALSE;
    }

  retval = FALSE;
  
  dir = _dbus_directory_open (dirname, error);

  if (dir == NULL)
    goto failed;

  dbus_error_init (&tmp_error);
  while (_dbus_directory_get_next_file (dir, &filename, &tmp_error))
    {
      DBusString full_path;

      if (!_dbus_string_init (&full_path))
        {
          BUS_SET_OOM (error);
          goto failed;
        }

      if (!_dbus_string_copy (dirname, 0, &full_path, 0))
        {
          BUS_SET_OOM (error);
          _dbus_string_free (&full_path);
          goto failed;
        }      

      if (!_dbus_concat_dir_and_file (&full_path, &filename))
        {
          BUS_SET_OOM (error);
          _dbus_string_free (&full_path);
          goto failed;
        }
      
      if (_dbus_string_ends_with_c_str (&full_path, ".conf"))
        {
          if (!include_file (parser, &full_path, TRUE, error))
            {
              if (dbus_error_is_set (error))
                {
                  /* We log to syslog unconditionally here, because this is
                   * the configuration parser, so we don't yet know whether
                   * this bus is going to want to write to syslog! (There's
                   * also some layer inversion going on, if we want to use
                   * the bus context.) */
                  _dbus_system_log (DBUS_SYSTEM_LOG_INFO,
                                    "Encountered error '%s' while parsing '%s'\n",
                                    error->message,
                                    _dbus_string_get_const_data (&full_path));
                  dbus_error_free (error);
                }
            }
        }

      _dbus_string_free (&full_path);
    }

  if (dbus_error_is_set (&tmp_error))
    {
      dbus_move_error (&tmp_error, error);
      goto failed;
    }


  if (!_dbus_string_copy_data (dirname, &s))
    {
      BUS_SET_OOM (error);
      goto failed;
    }

  if (!_dbus_list_append (&parser->conf_dirs, s))
    {
      dbus_free (s);
      BUS_SET_OOM (error);
      goto failed;
    }

  retval = TRUE;
  
 failed:
  _dbus_string_free (&filename);
  
  if (dir)
    _dbus_directory_close (dir);

  return retval;
}

dbus_bool_t
bus_config_parser_content (BusConfigParser   *parser,
                           const DBusString  *content,
                           DBusError         *error)
{
  Element *e;

  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

#if 0
  {
    const char *c_str;
    
    _dbus_string_get_const_data (content, &c_str);

    printf ("CONTENT %d bytes: %s\n", _dbus_string_get_length (content), c_str);
  }
#endif
  
  e = peek_element (parser);
  if (e == NULL)
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "Text content outside of any XML element in configuration file");
      return FALSE;
    }
  else if (e->had_content)
    {
      _dbus_assert_not_reached ("Element had multiple content blocks");
      return FALSE;
    }

  switch (top_element_type (parser))
    {
    case ELEMENT_NONE:
      _dbus_assert_not_reached ("element at top of stack has no type");
      return FALSE;

    case ELEMENT_BUSCONFIG:
    case ELEMENT_POLICY:
    case ELEMENT_ALLOW:
    case ELEMENT_DENY:
    case ELEMENT_FORK:
    case ELEMENT_SYSLOG:
    case ELEMENT_KEEP_UMASK:
    case ELEMENT_STANDARD_SESSION_SERVICEDIRS:    
    case ELEMENT_STANDARD_SYSTEM_SERVICEDIRS:    
    case ELEMENT_ALLOW_ANONYMOUS:
    case ELEMENT_SELINUX:
    case ELEMENT_ASSOCIATE:
      if (all_whitespace (content))
        return TRUE;
      else
        {
          dbus_set_error (error, DBUS_ERROR_FAILED,
                          "No text content expected inside XML element %s in configuration file",
                          bus_config_parser_element_type_to_name (top_element_type (parser)));
          return FALSE;
        }

    case ELEMENT_PIDFILE:
      {
        char *s;

        e->had_content = TRUE;
        
        if (!_dbus_string_copy_data (content, &s))
          goto nomem;
          
        dbus_free (parser->pidfile);
        parser->pidfile = s;
      }
      break;

    case ELEMENT_INCLUDE:
      {
        DBusString full_path, selinux_policy_root;

        e->had_content = TRUE;

	if (e->d.include.if_selinux_enabled
	    && !bus_selinux_enabled ())
	  break;

        if (!_dbus_string_init (&full_path))
          goto nomem;

        if (e->d.include.selinux_root_relative)
	  {
            if (!bus_selinux_get_policy_root ())
	      {
		dbus_set_error (error, DBUS_ERROR_FAILED,
				"Could not determine SELinux policy root for relative inclusion");
		_dbus_string_free (&full_path);
		return FALSE;
	      }
            _dbus_string_init_const (&selinux_policy_root,
                                     bus_selinux_get_policy_root ());
            if (!make_full_path (&selinux_policy_root, content, &full_path))
              {
                _dbus_string_free (&full_path);
                goto nomem;
              }
          }
        else if (!make_full_path (&parser->basedir, content, &full_path))
          {
            _dbus_string_free (&full_path);
            goto nomem;
          }

        if (!include_file (parser, &full_path,
                           e->d.include.ignore_missing, error))
          {
            _dbus_string_free (&full_path);
            return FALSE;
          }

        _dbus_string_free (&full_path);
      }
      break;

    case ELEMENT_SERVICEHELPER:
      {
        DBusString full_path;
        
        e->had_content = TRUE;

        if (!_dbus_string_init (&full_path))
          goto nomem;
        
        if (!make_full_path (&parser->basedir, content, &full_path))
          {
            _dbus_string_free (&full_path);
            goto nomem;
          }

        if (!servicehelper_path (parser, &full_path, error))
          {
            _dbus_string_free (&full_path);
            return FALSE;
          }

        _dbus_string_free (&full_path);
      }
      break;
      
    case ELEMENT_INCLUDEDIR:
      {
        DBusString full_path;
        
        e->had_content = TRUE;

        if (!_dbus_string_init (&full_path))
          goto nomem;
        
        if (!make_full_path (&parser->basedir, content, &full_path))
          {
            _dbus_string_free (&full_path);
            goto nomem;
          }
        
        if (!include_dir (parser, &full_path, error))
          {
            _dbus_string_free (&full_path);
            return FALSE;
          }

        _dbus_string_free (&full_path);
      }
      break;
      
    case ELEMENT_USER:
      {
        char *s;

        e->had_content = TRUE;
        
        if (!_dbus_string_copy_data (content, &s))
          goto nomem;
          
        dbus_free (parser->user);
        parser->user = s;
      }
      break;

    case ELEMENT_CONFIGTYPE:
      {
        char *s;

        e->had_content = TRUE;

        if (!_dbus_string_copy_data (content, &s))
          goto nomem;
        
        dbus_free (parser->bus_type);
        parser->bus_type = s;
      }
      break;
      
    case ELEMENT_LISTEN:
      {
        char *s;

        e->had_content = TRUE;
        
        if (!_dbus_string_copy_data (content, &s))
          goto nomem;

        if (!_dbus_list_append (&parser->listen_on,
                                s))
          {
            dbus_free (s);
            goto nomem;
          }
      }
      break;

    case ELEMENT_AUTH:
      {
        char *s;
        
        e->had_content = TRUE;

        if (!_dbus_string_copy_data (content, &s))
          goto nomem;

        if (!_dbus_list_append (&parser->mechanisms,
                                s))
          {
            dbus_free (s);
            goto nomem;
          }
      }
      break;

    case ELEMENT_SERVICEDIR:
      {
        char *s;
        DBusString full_path;
        
        e->had_content = TRUE;

        if (!_dbus_string_init (&full_path))
          goto nomem;
        
        if (!make_full_path (&parser->basedir, content, &full_path))
          {
            _dbus_string_free (&full_path);
            goto nomem;
          }
        
        if (!_dbus_string_copy_data (&full_path, &s))
          {
            _dbus_string_free (&full_path);
            goto nomem;
          }

        /* _only_ extra session directories can be specified */
        if (!service_dirs_append_unique_or_free (&parser->service_dirs, s))
          {
            _dbus_string_free (&full_path);
            dbus_free (s);
            goto nomem;
          }

        _dbus_string_free (&full_path);
      }
      break;

    case ELEMENT_LIMIT:
      {
        long val;

        e->had_content = TRUE;

        val = 0;
        if (!_dbus_string_parse_int (content, 0, &val, NULL))
          {
            dbus_set_error (error, DBUS_ERROR_FAILED,
                            "<limit name=\"%s\"> element has invalid value (could not parse as integer)",
                            e->d.limit.name);
            return FALSE;
          }

        e->d.limit.value = val;

        _dbus_verbose ("Loaded value %ld for limit %s\n",
                       e->d.limit.value,
                       e->d.limit.name);
      }
      break;
    }

  _DBUS_ASSERT_ERROR_IS_CLEAR (error);
  return TRUE;

 nomem:
  BUS_SET_OOM (error);
  return FALSE;
}

dbus_bool_t
bus_config_parser_finished (BusConfigParser   *parser,
                            DBusError         *error)
{
  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  if (parser->stack != NULL)
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "Element <%s> was not closed in configuration file",
                      bus_config_parser_element_type_to_name (top_element_type (parser)));

      return FALSE;
    }

  if (parser->is_toplevel && parser->listen_on == NULL)
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "Configuration file needs one or more <listen> elements giving addresses"); 
      return FALSE;
    }
  
  return TRUE;
}

const char*
bus_config_parser_get_user (BusConfigParser *parser)
{
  return parser->user;
}

const char*
bus_config_parser_get_type (BusConfigParser *parser)
{
  return parser->bus_type;
}

DBusList**
bus_config_parser_get_addresses (BusConfigParser *parser)
{
  return &parser->listen_on;
}

DBusList**
bus_config_parser_get_mechanisms (BusConfigParser *parser)
{
  return &parser->mechanisms;
}

DBusList**
bus_config_parser_get_service_dirs (BusConfigParser *parser)
{
  return &parser->service_dirs;
}

DBusList**
bus_config_parser_get_conf_dirs (BusConfigParser *parser)
{
  return &parser->conf_dirs;
}

dbus_bool_t
bus_config_parser_get_fork (BusConfigParser   *parser)
{
  return parser->fork;
}

dbus_bool_t
bus_config_parser_get_syslog (BusConfigParser   *parser)
{
  return parser->syslog;
}

dbus_bool_t
bus_config_parser_get_keep_umask (BusConfigParser   *parser)
{
  return parser->keep_umask;
}

dbus_bool_t
bus_config_parser_get_allow_anonymous (BusConfigParser   *parser)
{
  return parser->allow_anonymous;
}

const char *
bus_config_parser_get_pidfile (BusConfigParser   *parser)
{
  return parser->pidfile;
}

const char *
bus_config_parser_get_servicehelper (BusConfigParser   *parser)
{
  return parser->servicehelper;
}

BusPolicy*
bus_config_parser_steal_policy (BusConfigParser *parser)
{
  BusPolicy *policy;

  _dbus_assert (parser->policy != NULL); /* can only steal the policy 1 time */
  
  policy = parser->policy;

  parser->policy = NULL;

  return policy;
}

/* Overwrite any limits that were set in the configuration file */
void
bus_config_parser_get_limits (BusConfigParser *parser,
                              BusLimits       *limits)
{
  *limits = parser->limits;
}

DBusHashTable*
bus_config_parser_steal_service_context_table (BusConfigParser *parser)
{
  DBusHashTable *table;

  _dbus_assert (parser->service_context_table != NULL); /* can only steal once */

  table = parser->service_context_table;

  parser->service_context_table = NULL;

  return table;
}

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
#include <stdio.h>

typedef enum
{
  VALID,
  INVALID,
  UNKNOWN
} Validity;

static dbus_bool_t
do_check_own_rules (BusPolicy  *policy)
{
  const struct {
    char *name;
    dbus_bool_t allowed;
  } checks[] = {
    {"org.freedesktop", FALSE},
    {"org.freedesktop.ManySystem", FALSE},
    {"org.freedesktop.ManySystems", TRUE},
    {"org.freedesktop.ManySystems.foo", TRUE},
    {"org.freedesktop.ManySystems.foo.bar", TRUE},
    {"org.freedesktop.ManySystems2", FALSE},
    {"org.freedesktop.ManySystems2.foo", FALSE},
    {"org.freedesktop.ManySystems2.foo.bar", FALSE},
    {NULL, FALSE}
  };
  int i = 0;

  while (checks[i].name)
    {
      DBusString service_name;
      dbus_bool_t ret;

      if (!_dbus_string_init (&service_name))
        _dbus_assert_not_reached ("couldn't init string");
      if (!_dbus_string_append (&service_name, checks[i].name))
        _dbus_assert_not_reached ("couldn't append string");

      ret = bus_policy_check_can_own (policy, &service_name);
      printf ("        Check name %s: %s\n", checks[i].name,
              ret ? "allowed" : "not allowed");
      if (checks[i].allowed && !ret)
        {
          _dbus_warn ("Cannot own %s\n", checks[i].name);
          return FALSE;
        }
      if (!checks[i].allowed && ret)
        {
          _dbus_warn ("Can own %s\n", checks[i].name);
          return FALSE;
        }
      _dbus_string_free (&service_name);

      i++;
    }

  return TRUE;
}

static dbus_bool_t
do_load (const DBusString *full_path,
         Validity          validity,
         dbus_bool_t       oom_possible,
         dbus_bool_t       check_own_rules)
{
  BusConfigParser *parser;
  DBusError error;

  dbus_error_init (&error);

  parser = bus_config_load (full_path, TRUE, NULL, &error);
  if (parser == NULL)
    {
      _DBUS_ASSERT_ERROR_IS_SET (&error);

      if (oom_possible &&
          dbus_error_has_name (&error, DBUS_ERROR_NO_MEMORY))
        {
          _dbus_verbose ("Failed to load valid file due to OOM\n");
          dbus_error_free (&error);
          return TRUE;
        }
      else if (validity == VALID)
        {
          _dbus_warn ("Failed to load valid file but still had memory: %s\n",
                      error.message);

          dbus_error_free (&error);
          return FALSE;
        }
      else
        {
          dbus_error_free (&error);
          return TRUE;
        }
    }
  else
    {
      _DBUS_ASSERT_ERROR_IS_CLEAR (&error);

      if (check_own_rules && do_check_own_rules (parser->policy) == FALSE)
        {
          return FALSE;
        }

      bus_config_parser_unref (parser);

      if (validity == INVALID)
        {
          _dbus_warn ("Accepted invalid file\n");
          return FALSE;
        }

      return TRUE;
    }
}

typedef struct
{
  const DBusString *full_path;
  Validity          validity;
  dbus_bool_t       check_own_rules;
} LoaderOomData;

static dbus_bool_t
check_loader_oom_func (void *data)
{
  LoaderOomData *d = data;

  return do_load (d->full_path, d->validity, TRUE, d->check_own_rules);
}

static dbus_bool_t
process_test_valid_subdir (const DBusString *test_base_dir,
                           const char       *subdir,
                           Validity          validity)
{
  DBusString test_directory;
  DBusString filename;
  DBusDirIter *dir;
  dbus_bool_t retval;
  DBusError error;

  retval = FALSE;
  dir = NULL;

  if (!_dbus_string_init (&test_directory))
    _dbus_assert_not_reached ("didn't allocate test_directory\n");

  _dbus_string_init_const (&filename, subdir);

  if (!_dbus_string_copy (test_base_dir, 0,
                          &test_directory, 0))
    _dbus_assert_not_reached ("couldn't copy test_base_dir to test_directory");

  if (!_dbus_concat_dir_and_file (&test_directory, &filename))
    _dbus_assert_not_reached ("couldn't allocate full path");

  _dbus_string_free (&filename);
  if (!_dbus_string_init (&filename))
    _dbus_assert_not_reached ("didn't allocate filename string\n");

  dbus_error_init (&error);
  dir = _dbus_directory_open (&test_directory, &error);
  if (dir == NULL)
    {
      _dbus_warn ("Could not open %s: %s\n",
                  _dbus_string_get_const_data (&test_directory),
                  error.message);
      dbus_error_free (&error);
      goto failed;
    }

  if (validity == VALID)
    printf ("Testing valid files:\n");
  else if (validity == INVALID)
    printf ("Testing invalid files:\n");
  else
    printf ("Testing unknown files:\n");

 next:
  while (_dbus_directory_get_next_file (dir, &filename, &error))
    {
      DBusString full_path;
      LoaderOomData d;

      if (!_dbus_string_init (&full_path))
        _dbus_assert_not_reached ("couldn't init string");

      if (!_dbus_string_copy (&test_directory, 0, &full_path, 0))
        _dbus_assert_not_reached ("couldn't copy dir to full_path");

      if (!_dbus_concat_dir_and_file (&full_path, &filename))
        _dbus_assert_not_reached ("couldn't concat file to dir");

      if (!_dbus_string_ends_with_c_str (&full_path, ".conf"))
        {
          _dbus_verbose ("Skipping non-.conf file %s\n",
                         _dbus_string_get_const_data (&filename));
          _dbus_string_free (&full_path);
          goto next;
        }

      printf ("    %s\n", _dbus_string_get_const_data (&filename));

      _dbus_verbose (" expecting %s\n",
                     validity == VALID ? "valid" :
                     (validity == INVALID ? "invalid" :
                      (validity == UNKNOWN ? "unknown" : "???")));

      d.full_path = &full_path;
      d.validity = validity;
      d.check_own_rules = _dbus_string_ends_with_c_str (&full_path,
          "check-own-rules.conf");

      /* FIXME hackaround for an expat problem, see
       * https://bugzilla.redhat.com/bugzilla/show_bug.cgi?id=124747
       * http://freedesktop.org/pipermail/dbus/2004-May/001153.html
       */
      /* if (!_dbus_test_oom_handling ("config-loader", check_loader_oom_func, &d)) */
      if (!check_loader_oom_func (&d))
        _dbus_assert_not_reached ("test failed");
      
      _dbus_string_free (&full_path);
    }

  if (dbus_error_is_set (&error))
    {
      _dbus_warn ("Could not get next file in %s: %s\n",
                  _dbus_string_get_const_data (&test_directory),
                  error.message);
      dbus_error_free (&error);
      goto failed;
    }

  retval = TRUE;

 failed:

  if (dir)
    _dbus_directory_close (dir);
  _dbus_string_free (&test_directory);
  _dbus_string_free (&filename);

  return retval;
}

static dbus_bool_t
bools_equal (dbus_bool_t a,
	     dbus_bool_t b)
{
  return a ? b : !b;
}

static dbus_bool_t
strings_equal_or_both_null (const char *a,
                            const char *b)
{
  if (a == NULL || b == NULL)
    return a == b;
  else
    return !strcmp (a, b);
}

static dbus_bool_t
elements_equal (const Element *a,
		const Element *b)
{
  if (a->type != b->type)
    return FALSE;

  if (!bools_equal (a->had_content, b->had_content))
    return FALSE;

  switch (a->type)
    {

    case ELEMENT_INCLUDE:
      if (!bools_equal (a->d.include.ignore_missing,
			b->d.include.ignore_missing))
	return FALSE;
      break;

    case ELEMENT_POLICY:
      if (a->d.policy.type != b->d.policy.type)
	return FALSE;
      if (a->d.policy.gid_uid_or_at_console != b->d.policy.gid_uid_or_at_console)
	return FALSE;
      break;

    case ELEMENT_LIMIT:
      if (strcmp (a->d.limit.name, b->d.limit.name))
	return FALSE;
      if (a->d.limit.value != b->d.limit.value)
	return FALSE;
      break;

    default:
      /* do nothing */
      break;
    }

  return TRUE;

}

static dbus_bool_t
lists_of_elements_equal (DBusList *a,
			 DBusList *b)
{
  DBusList *ia;
  DBusList *ib;

  ia = a;
  ib = b;
  
  while (ia != NULL && ib != NULL)
    {
      if (elements_equal (ia->data, ib->data))
	return FALSE;
      ia = _dbus_list_get_next_link (&a, ia);
      ib = _dbus_list_get_next_link (&b, ib);
    }

  return ia == NULL && ib == NULL;
}

static dbus_bool_t
lists_of_c_strings_equal (DBusList *a,
			  DBusList *b)
{
  DBusList *ia;
  DBusList *ib;

  ia = a;
  ib = b;
  
  while (ia != NULL && ib != NULL)
    {
      if (strcmp (ia->data, ib->data))
	return FALSE;
      ia = _dbus_list_get_next_link (&a, ia);
      ib = _dbus_list_get_next_link (&b, ib);
    }

  return ia == NULL && ib == NULL;
}

static dbus_bool_t
limits_equal (const BusLimits *a,
	      const BusLimits *b)
{
  return
    (a->max_incoming_bytes == b->max_incoming_bytes
     || a->max_incoming_unix_fds == b->max_incoming_unix_fds
     || a->max_outgoing_bytes == b->max_outgoing_bytes
     || a->max_outgoing_unix_fds == b->max_outgoing_unix_fds
     || a->max_message_size == b->max_message_size
     || a->max_message_unix_fds == b->max_message_unix_fds
     || a->activation_timeout == b->activation_timeout
     || a->auth_timeout == b->auth_timeout
     || a->pending_fd_timeout == b->pending_fd_timeout
     || a->max_completed_connections == b->max_completed_connections
     || a->max_incomplete_connections == b->max_incomplete_connections
     || a->max_connections_per_user == b->max_connections_per_user
     || a->max_pending_activations == b->max_pending_activations
     || a->max_services_per_connection == b->max_services_per_connection
     || a->max_match_rules_per_connection == b->max_match_rules_per_connection
     || a->max_replies_per_connection == b->max_replies_per_connection
     || a->reply_timeout == b->reply_timeout);
}

static dbus_bool_t
config_parsers_equal (const BusConfigParser *a,
                      const BusConfigParser *b)
{
  if (!_dbus_string_equal (&a->basedir, &b->basedir))
    return FALSE;

  if (!lists_of_elements_equal (a->stack, b->stack))
    return FALSE;

  if (!strings_equal_or_both_null (a->user, b->user))
    return FALSE;

  if (!lists_of_c_strings_equal (a->listen_on, b->listen_on))
    return FALSE;

  if (!lists_of_c_strings_equal (a->mechanisms, b->mechanisms))
    return FALSE;

  if (!lists_of_c_strings_equal (a->service_dirs, b->service_dirs))
    return FALSE;
  
  /* FIXME: compare policy */

  /* FIXME: compare service selinux ID table */

  if (! limits_equal (&a->limits, &b->limits))
    return FALSE;

  if (!strings_equal_or_both_null (a->pidfile, b->pidfile))
    return FALSE;

  if (! bools_equal (a->fork, b->fork))
    return FALSE;

  if (! bools_equal (a->keep_umask, b->keep_umask))
    return FALSE;

  if (! bools_equal (a->is_toplevel, b->is_toplevel))
    return FALSE;

  return TRUE;
}

static dbus_bool_t
all_are_equiv (const DBusString *target_directory)
{
  DBusString filename;
  DBusDirIter *dir;
  BusConfigParser *first_parser;
  BusConfigParser *parser;
  DBusError error;
  dbus_bool_t equal;
  dbus_bool_t retval;

  dir = NULL;
  first_parser = NULL;
  parser = NULL;
  retval = FALSE;

  if (!_dbus_string_init (&filename))
    _dbus_assert_not_reached ("didn't allocate filename string");

  dbus_error_init (&error);
  dir = _dbus_directory_open (target_directory, &error);
  if (dir == NULL)
    {
      _dbus_warn ("Could not open %s: %s\n",
		  _dbus_string_get_const_data (target_directory),
		  error.message);
      dbus_error_free (&error);
      goto finished;
    }

  printf ("Comparing equivalent files:\n");

 next:
  while (_dbus_directory_get_next_file (dir, &filename, &error))
    {
      DBusString full_path;

      if (!_dbus_string_init (&full_path))
	_dbus_assert_not_reached ("couldn't init string");

      if (!_dbus_string_copy (target_directory, 0, &full_path, 0))
        _dbus_assert_not_reached ("couldn't copy dir to full_path");

      if (!_dbus_concat_dir_and_file (&full_path, &filename))
        _dbus_assert_not_reached ("couldn't concat file to dir");

      if (!_dbus_string_ends_with_c_str (&full_path, ".conf"))
        {
          _dbus_verbose ("Skipping non-.conf file %s\n",
                         _dbus_string_get_const_data (&filename));
	  _dbus_string_free (&full_path);
          goto next;
        }

      printf ("    %s\n", _dbus_string_get_const_data (&filename));

      parser = bus_config_load (&full_path, TRUE, NULL, &error);

      if (parser == NULL)
	{
	  _dbus_warn ("Could not load file %s: %s\n",
		      _dbus_string_get_const_data (&full_path),
		      error.message);
          _dbus_string_free (&full_path);
	  dbus_error_free (&error);
	  goto finished;
	}
      else if (first_parser == NULL)
	{
          _dbus_string_free (&full_path);
	  first_parser = parser;
	}
      else
	{
          _dbus_string_free (&full_path);
	  equal = config_parsers_equal (first_parser, parser);
	  bus_config_parser_unref (parser);
	  if (! equal)
	    goto finished;
	}
    }

  retval = TRUE;

 finished:
  _dbus_string_free (&filename);
  if (first_parser)
    bus_config_parser_unref (first_parser);
  if (dir)
    _dbus_directory_close (dir);

  return retval;
  
}

static dbus_bool_t
process_test_equiv_subdir (const DBusString *test_base_dir,
			   const char       *subdir)
{
  DBusString test_directory;
  DBusString filename;
  DBusDirIter *dir;
  DBusError error;
  dbus_bool_t equal;
  dbus_bool_t retval;

  dir = NULL;
  retval = FALSE;

  if (!_dbus_string_init (&test_directory))
    _dbus_assert_not_reached ("didn't allocate test_directory");

  _dbus_string_init_const (&filename, subdir);

  if (!_dbus_string_copy (test_base_dir, 0,
			  &test_directory, 0))
    _dbus_assert_not_reached ("couldn't copy test_base_dir to test_directory");

  if (!_dbus_concat_dir_and_file (&test_directory, &filename))
    _dbus_assert_not_reached ("couldn't allocate full path");

  _dbus_string_free (&filename);
  if (!_dbus_string_init (&filename))
    _dbus_assert_not_reached ("didn't allocate filename string");

  dbus_error_init (&error);
  dir = _dbus_directory_open (&test_directory, &error);
  if (dir == NULL)
    {
      _dbus_warn ("Could not open %s: %s\n",
		  _dbus_string_get_const_data (&test_directory),
		  error.message);
      dbus_error_free (&error);
      goto finished;
    }

  while (_dbus_directory_get_next_file (dir, &filename, &error))
    {
      DBusString full_path;

      /* Skip CVS's magic directories! */
      if (_dbus_string_equal_c_str (&filename, "CVS"))
	continue;

      if (!_dbus_string_init (&full_path))
	_dbus_assert_not_reached ("couldn't init string");

      if (!_dbus_string_copy (&test_directory, 0, &full_path, 0))
        _dbus_assert_not_reached ("couldn't copy dir to full_path");

      if (!_dbus_concat_dir_and_file (&full_path, &filename))
        _dbus_assert_not_reached ("couldn't concat file to dir");
      
      equal = all_are_equiv (&full_path);
      _dbus_string_free (&full_path);

      if (!equal)
	goto finished;
    }

  retval = TRUE;

 finished:
  _dbus_string_free (&test_directory);
  _dbus_string_free (&filename);
  if (dir)
    _dbus_directory_close (dir);

  return retval;
  
}

static const char *test_session_service_dir_matches[] = 
        {
#ifdef DBUS_UNIX
         "/testhome/foo/.testlocal/testshare/dbus-1/services",
         "/testusr/testlocal/testshare/dbus-1/services",
         "/testusr/testshare/dbus-1/services",
         DBUS_DATADIR"/dbus-1/services",
#endif
/* will be filled in test_default_session_servicedirs() */
#ifdef DBUS_WIN
         NULL,
         NULL,
#endif
         NULL
        };

static dbus_bool_t
test_default_session_servicedirs (void)
{
  DBusList *dirs;
  DBusList *link;
  DBusString progs;
  int i;

#ifdef DBUS_WIN
  const char *common_progs;
  char buffer[1024];

  if (_dbus_get_install_root(buffer, sizeof(buffer)))
    {
      strcat(buffer,DBUS_DATADIR);
      strcat(buffer,"/dbus-1/services");
      test_session_service_dir_matches[0] = buffer;
    }
#endif

  /* On Unix we don't actually use this variable, but it's easier to handle the
   * deallocation if we always allocate it, whether needed or not */
  if (!_dbus_string_init (&progs))
    _dbus_assert_not_reached ("OOM allocating progs");

#ifndef DBUS_UNIX
  common_progs = _dbus_getenv ("CommonProgramFiles");

  if (common_progs) 
    {
      if (!_dbus_string_append (&progs, common_progs)) 
        {
          _dbus_string_free (&progs);
          return FALSE;
        }

      if (!_dbus_string_append (&progs, "/dbus-1/services")) 
        {
          _dbus_string_free (&progs);
          return FALSE;
        }
      test_session_service_dir_matches[1] = _dbus_string_get_const_data(&progs);
    }
#endif
  dirs = NULL;

  printf ("Testing retrieving the default session service directories\n");
  if (!_dbus_get_standard_session_servicedirs (&dirs))
    _dbus_assert_not_reached ("couldn't get stardard dirs");

  /* make sure our defaults end with share/dbus-1/service */
  while ((link = _dbus_list_pop_first_link (&dirs)))
    {
      DBusString path;
      
      printf ("    default service dir: %s\n", (char *)link->data);
      _dbus_string_init_const (&path, (char *)link->data);
      if (!_dbus_string_ends_with_c_str (&path, "dbus-1/services"))
        {
          printf ("error with default session service directories\n");
	      dbus_free (link->data);
    	  _dbus_list_free_link (link);
          _dbus_string_free (&progs);
          return FALSE;
        }
 
      dbus_free (link->data);
      _dbus_list_free_link (link);
    }

#ifdef DBUS_UNIX
  if (!dbus_setenv ("XDG_DATA_HOME", "/testhome/foo/.testlocal/testshare"))
    _dbus_assert_not_reached ("couldn't setenv XDG_DATA_HOME");

  if (!dbus_setenv ("XDG_DATA_DIRS", ":/testusr/testlocal/testshare: :/testusr/testshare:"))
    _dbus_assert_not_reached ("couldn't setenv XDG_DATA_DIRS");
#endif
  if (!_dbus_get_standard_session_servicedirs (&dirs))
    _dbus_assert_not_reached ("couldn't get stardard dirs");

  /* make sure we read and parse the env variable correctly */
  i = 0;
  while ((link = _dbus_list_pop_first_link (&dirs)))
    {
      printf ("    test service dir: %s\n", (char *)link->data);
      if (test_session_service_dir_matches[i] == NULL)
        {
          printf ("more directories parsed than in match set\n");
          dbus_free (link->data);
          _dbus_list_free_link (link);
          _dbus_string_free (&progs);
          return FALSE;
        }
 
      if (strcmp (test_session_service_dir_matches[i], 
                  (char *)link->data) != 0)
        {
          printf ("%s directory does not match %s in the match set\n", 
                  (char *)link->data,
                  test_session_service_dir_matches[i]);
          dbus_free (link->data);
          _dbus_list_free_link (link);
          _dbus_string_free (&progs);
          return FALSE;
        }

      ++i;

      dbus_free (link->data);
      _dbus_list_free_link (link);
    }
  
  if (test_session_service_dir_matches[i] != NULL)
    {
      printf ("extra data %s in the match set was not matched\n",
              test_session_service_dir_matches[i]);

      _dbus_string_free (&progs);
      return FALSE;
    }
    
  _dbus_string_free (&progs);
  return TRUE;
}

static const char *test_system_service_dir_matches[] = 
        {
#ifdef DBUS_UNIX
         "/usr/local/share/dbus-1/system-services",
         "/usr/share/dbus-1/system-services",
#endif
         DBUS_DATADIR"/dbus-1/system-services",
#ifdef DBUS_UNIX
         "/lib/dbus-1/system-services",
#endif

#ifdef DBUS_WIN
         NULL,
#endif
         NULL
        };

static dbus_bool_t
test_default_system_servicedirs (void)
{
  DBusList *dirs;
  DBusList *link;
  DBusString progs;
#ifndef DBUS_UNIX
  const char *common_progs;
#endif
  int i;

  /* On Unix we don't actually use this variable, but it's easier to handle the
   * deallocation if we always allocate it, whether needed or not */
  if (!_dbus_string_init (&progs))
    _dbus_assert_not_reached ("OOM allocating progs");

#ifndef DBUS_UNIX
  common_progs = _dbus_getenv ("CommonProgramFiles");

  if (common_progs) 
    {
      if (!_dbus_string_append (&progs, common_progs)) 
        {
          _dbus_string_free (&progs);
          return FALSE;
        }

      if (!_dbus_string_append (&progs, "/dbus-1/system-services")) 
        {
          _dbus_string_free (&progs);
          return FALSE;
        }
      test_system_service_dir_matches[1] = _dbus_string_get_const_data(&progs);
    }
#endif
  dirs = NULL;

  printf ("Testing retrieving the default system service directories\n");
  if (!_dbus_get_standard_system_servicedirs (&dirs))
    _dbus_assert_not_reached ("couldn't get stardard dirs");

  /* make sure our defaults end with share/dbus-1/system-service */
  while ((link = _dbus_list_pop_first_link (&dirs)))
    {
      DBusString path;
      
      printf ("    default service dir: %s\n", (char *)link->data);
      _dbus_string_init_const (&path, (char *)link->data);
      if (!_dbus_string_ends_with_c_str (&path, "dbus-1/system-services"))
        {
          printf ("error with default system service directories\n");
	      dbus_free (link->data);
    	  _dbus_list_free_link (link);
          _dbus_string_free (&progs);
          return FALSE;
        }
 
      dbus_free (link->data);
      _dbus_list_free_link (link);
    }

#ifdef DBUS_UNIX
  if (!dbus_setenv ("XDG_DATA_HOME", "/testhome/foo/.testlocal/testshare"))
    _dbus_assert_not_reached ("couldn't setenv XDG_DATA_HOME");

  if (!dbus_setenv ("XDG_DATA_DIRS", ":/testusr/testlocal/testshare: :/testusr/testshare:"))
    _dbus_assert_not_reached ("couldn't setenv XDG_DATA_DIRS");
#endif
  if (!_dbus_get_standard_system_servicedirs (&dirs))
    _dbus_assert_not_reached ("couldn't get stardard dirs");

  /* make sure we read and parse the env variable correctly */
  i = 0;
  while ((link = _dbus_list_pop_first_link (&dirs)))
    {
      printf ("    test service dir: %s\n", (char *)link->data);
      if (test_system_service_dir_matches[i] == NULL)
        {
          printf ("more directories parsed than in match set\n");
          dbus_free (link->data);
          _dbus_list_free_link (link);
          _dbus_string_free (&progs);
          return FALSE;
        }
 
      if (strcmp (test_system_service_dir_matches[i], 
                  (char *)link->data) != 0)
        {
          printf ("%s directory does not match %s in the match set\n", 
                  (char *)link->data,
                  test_system_service_dir_matches[i]);
          dbus_free (link->data);
          _dbus_list_free_link (link);
          _dbus_string_free (&progs);
          return FALSE;
        }

      ++i;

      dbus_free (link->data);
      _dbus_list_free_link (link);
    }
  
  if (test_system_service_dir_matches[i] != NULL)
    {
      printf ("extra data %s in the match set was not matched\n",
              test_system_service_dir_matches[i]);

      _dbus_string_free (&progs);
      return FALSE;
    }
    
  _dbus_string_free (&progs);
  return TRUE;
}
		   
dbus_bool_t
bus_config_parser_test (const DBusString *test_data_dir)
{
  if (test_data_dir == NULL ||
      _dbus_string_get_length (test_data_dir) == 0)
    {
      printf ("No test data\n");
      return TRUE;
    }

  if (!test_default_session_servicedirs())
    return FALSE;

#ifdef DBUS_WIN
  printf("default system service dir skipped\n");
#else
  if (!test_default_system_servicedirs())
    return FALSE;
#endif

  if (!process_test_valid_subdir (test_data_dir, "valid-config-files", VALID))
    return FALSE;

  if (!process_test_valid_subdir (test_data_dir, "invalid-config-files", INVALID))
    return FALSE;

  if (!process_test_equiv_subdir (test_data_dir, "equiv-config-files"))
    return FALSE;

  return TRUE;
}

#endif /* DBUS_ENABLE_EMBEDDED_TESTS */

