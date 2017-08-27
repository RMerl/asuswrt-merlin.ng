/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-monitor.c  Utility program to monitor messages on the bus
 *
 * Copyright (C) 2003 Philip Blundell <philb@gnu.org>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DBUS_WIN
#include <winsock2.h>
#undef interface
#else
#include <sys/time.h>
#endif

#include <time.h>

#include "dbus-print-message.h"

#define EAVESDROPPING_RULE "eavesdrop=true"

#ifdef DBUS_WIN

/* gettimeofday is not defined on windows */
#define DBUS_SECONDS_SINCE_1601 11644473600LL
#define DBUS_USEC_IN_SEC        1000000LL

#ifdef DBUS_WINCE

#ifndef _IOLBF
#define _IOLBF 0x40
#endif
#ifndef _IONBF
#define _IONBF 0x04
#endif

void
GetSystemTimeAsFileTime (LPFILETIME ftp)
{
  SYSTEMTIME st;
  GetSystemTime (&st);
  SystemTimeToFileTime (&st, ftp);
}
#endif

static int
gettimeofday (struct timeval *__p,
	      void *__t)
{
  union {
      unsigned long long ns100; /*time since 1 Jan 1601 in 100ns units */
      FILETIME           ft;
    } now;

  GetSystemTimeAsFileTime (&now.ft);
  __p->tv_usec = (long) ((now.ns100 / 10LL) % DBUS_USEC_IN_SEC);
  __p->tv_sec  = (long)(((now.ns100 / 10LL) / DBUS_SECONDS_SINCE_1601) - DBUS_SECONDS_SINCE_1601);

  return 0;
}
#endif

inline static void
oom (const char *doing)
{
  fprintf (stderr, "OOM while %s\n", doing);
  exit (1);
}

static DBusHandlerResult
monitor_filter_func (DBusConnection     *connection,
		     DBusMessage        *message,
		     void               *user_data)
{
  print_message (message, FALSE);
  
  if (dbus_message_is_signal (message,
                              DBUS_INTERFACE_LOCAL,
                              "Disconnected"))
    exit (0);
  
  /* Conceptually we want this to be
   * DBUS_HANDLER_RESULT_NOT_YET_HANDLED, but this raises
   * some problems.  See bug 1719.
   */
  return DBUS_HANDLER_RESULT_HANDLED;
}

#ifdef __APPLE__
#define PROFILE_TIMED_FORMAT "%s\t%lu\t%d"
#elif defined(__NetBSD__)
#include <inttypes.h>
#define PROFILE_TIMED_FORMAT "%s\t%" PRId64 "\t%d"
#else
#define PROFILE_TIMED_FORMAT "%s\t%lu\t%lu"
#endif
#define TRAP_NULL_STRING(str) ((str) ? (str) : "<none>")

typedef enum
{
  PROFILE_ATTRIBUTE_FLAG_SERIAL = 1,
  PROFILE_ATTRIBUTE_FLAG_REPLY_SERIAL = 2,
  PROFILE_ATTRIBUTE_FLAG_SENDER = 4,
  PROFILE_ATTRIBUTE_FLAG_DESTINATION = 8,
  PROFILE_ATTRIBUTE_FLAG_PATH = 16,
  PROFILE_ATTRIBUTE_FLAG_INTERFACE = 32,
  PROFILE_ATTRIBUTE_FLAG_MEMBER = 64,
  PROFILE_ATTRIBUTE_FLAG_ERROR_NAME = 128
} ProfileAttributeFlags;

static void
profile_print_with_attrs (const char *type, DBusMessage *message,
  struct timeval *t, ProfileAttributeFlags attrs)
{
  printf (PROFILE_TIMED_FORMAT, type, t->tv_sec, t->tv_usec);

  if (attrs & PROFILE_ATTRIBUTE_FLAG_SERIAL)
    printf ("\t%u", dbus_message_get_serial (message));

  if (attrs & PROFILE_ATTRIBUTE_FLAG_REPLY_SERIAL)
    printf ("\t%u", dbus_message_get_reply_serial (message));

  if (attrs & PROFILE_ATTRIBUTE_FLAG_SENDER)
    printf ("\t%s", TRAP_NULL_STRING (dbus_message_get_sender (message)));

  if (attrs & PROFILE_ATTRIBUTE_FLAG_DESTINATION)
    printf ("\t%s", TRAP_NULL_STRING (dbus_message_get_destination (message)));

  if (attrs & PROFILE_ATTRIBUTE_FLAG_PATH)
    printf ("\t%s", TRAP_NULL_STRING (dbus_message_get_path (message)));

  if (attrs & PROFILE_ATTRIBUTE_FLAG_INTERFACE)
    printf ("\t%s", TRAP_NULL_STRING (dbus_message_get_interface (message)));

  if (attrs & PROFILE_ATTRIBUTE_FLAG_MEMBER)
    printf ("\t%s", TRAP_NULL_STRING (dbus_message_get_member (message)));

  if (attrs & PROFILE_ATTRIBUTE_FLAG_ERROR_NAME)
    printf ("\t%s", TRAP_NULL_STRING (dbus_message_get_error_name (message)));

  printf ("\n");
}

static void
print_message_profile (DBusMessage *message)
{
  struct timeval t;

  if (gettimeofday (&t, NULL) < 0)
    {
      printf ("un\n");
      return;
    }

  switch (dbus_message_get_type (message))
    {
      case DBUS_MESSAGE_TYPE_METHOD_CALL:
	profile_print_with_attrs ("mc", message, &t,
	  PROFILE_ATTRIBUTE_FLAG_SERIAL |
	  PROFILE_ATTRIBUTE_FLAG_SENDER |
	  PROFILE_ATTRIBUTE_FLAG_PATH |
	  PROFILE_ATTRIBUTE_FLAG_INTERFACE |
	  PROFILE_ATTRIBUTE_FLAG_MEMBER);
	break;
      case DBUS_MESSAGE_TYPE_METHOD_RETURN:
	profile_print_with_attrs ("mr", message, &t,
	  PROFILE_ATTRIBUTE_FLAG_SERIAL |
	  PROFILE_ATTRIBUTE_FLAG_DESTINATION |
	  PROFILE_ATTRIBUTE_FLAG_REPLY_SERIAL);
	break;
      case DBUS_MESSAGE_TYPE_ERROR:
	profile_print_with_attrs ("err", message, &t,
	  PROFILE_ATTRIBUTE_FLAG_SERIAL |
	  PROFILE_ATTRIBUTE_FLAG_DESTINATION |
	  PROFILE_ATTRIBUTE_FLAG_REPLY_SERIAL);
	break;
      case DBUS_MESSAGE_TYPE_SIGNAL:
	profile_print_with_attrs ("sig", message, &t,
	  PROFILE_ATTRIBUTE_FLAG_SERIAL |
	  PROFILE_ATTRIBUTE_FLAG_PATH |
	  PROFILE_ATTRIBUTE_FLAG_INTERFACE |
	  PROFILE_ATTRIBUTE_FLAG_MEMBER);
	break;
      default:
	printf (PROFILE_TIMED_FORMAT "\n", "tun", t.tv_sec, t.tv_usec);
	break;
    }
}

static DBusHandlerResult
profile_filter_func (DBusConnection	*connection,
		     DBusMessage	*message,
		     void		*user_data)
{
  print_message_profile (message);

  if (dbus_message_is_signal (message,
                              DBUS_INTERFACE_LOCAL,
                              "Disconnected"))
    exit (0);

  return DBUS_HANDLER_RESULT_HANDLED;
}

static void
usage (char *name, int ecode)
{
  fprintf (stderr, "Usage: %s [--system | --session | --address ADDRESS] [--monitor | --profile ] [watch expressions]\n", name);
  exit (ecode);
}

static void
only_one_type (dbus_bool_t *seen_bus_type,
               char        *name)
{
  if (*seen_bus_type)
    {
      fprintf (stderr, "I only support monitoring one bus at a time!\n");
      usage (name, 1);
    }
  else
    {
      *seen_bus_type = TRUE;
    }
}

int
main (int argc, char *argv[])
{
  DBusConnection *connection;
  DBusError error;
  DBusBusType type = DBUS_BUS_SESSION;
  DBusHandleMessageFunction filter_func = monitor_filter_func;
  char *address = NULL;
  dbus_bool_t seen_bus_type = FALSE;
  
  int i = 0, j = 0, numFilters = 0;
  char **filters = NULL;

  /* Set stdout to be unbuffered; this is basically so that if people
   * do dbus-monitor > file, then send SIGINT via Control-C, they
   * don't lose the last chunk of messages.
   */

#ifdef DBUS_WIN
  setvbuf (stdout, NULL, _IONBF, 0);
#else
  setvbuf (stdout, NULL, _IOLBF, 0);
#endif

  for (i = 1; i < argc; i++)
    {
      char *arg = argv[i];

      if (!strcmp (arg, "--system"))
        {
          only_one_type (&seen_bus_type, argv[0]);
          type = DBUS_BUS_SYSTEM;
        }
      else if (!strcmp (arg, "--session"))
        {
          only_one_type (&seen_bus_type, argv[0]);
          type = DBUS_BUS_SESSION;
        }
      else if (!strcmp (arg, "--address"))
        {
          only_one_type (&seen_bus_type, argv[0]);

          if (i+1 < argc)
            {
              address = argv[i+1];
              i++;
            }
          else
            usage (argv[0], 1);
        }
      else if (!strcmp (arg, "--help"))
	usage (argv[0], 0);
      else if (!strcmp (arg, "--monitor"))
	filter_func = monitor_filter_func;
      else if (!strcmp (arg, "--profile"))
	filter_func = profile_filter_func;
      else if (!strcmp (arg, "--"))
	continue;
      else if (arg[0] == '-')
	usage (argv[0], 1);
      else {
          unsigned int filter_len;
          numFilters++;
          /* Prepend a rule (and a comma) to enable the monitor to eavesdrop.
           * Prepending allows the user to add eavesdrop=false at command line
           * in order to disable eavesdropping when needed */
          filter_len = strlen (EAVESDROPPING_RULE) + 1 + strlen (arg) + 1;

          filters = (char **) realloc (filters, numFilters * sizeof (char *));
          if (filters == NULL)
            oom ("adding a new filter slot");
          filters[j] = (char *) malloc (filter_len);
          if (filters[j] == NULL)
            oom ("adding a new filter");
          snprintf (filters[j], filter_len, "%s,%s", EAVESDROPPING_RULE, arg);
          j++;
      }
    }

  dbus_error_init (&error);
  
  if (address != NULL)
    {
      connection = dbus_connection_open (address, &error);
      if (connection)
        {
          if (!dbus_bus_register (connection, &error))
      	    {
              fprintf (stderr, "Failed to register connection to bus at %s: %s\n",
      	               address, error.message);
              dbus_error_free (&error);
              exit (1);
      	    }
        }
    }
  else
    connection = dbus_bus_get (type, &error);
  if (connection == NULL)
    {
      const char *where;
      if (address != NULL)
        where = address;
      else
        {
          switch (type)
            {
            case DBUS_BUS_SYSTEM:
              where = "system bus";
              break;
            case DBUS_BUS_SESSION:
              where = "session bus";
              break;
            default:
              where = "";
            }
        }
      fprintf (stderr, "Failed to open connection to %s: %s\n",
               where,
               error.message);
      dbus_error_free (&error);
      exit (1);
    }

  if (numFilters)
    {
      size_t offset = 0;
      for (i = 0; i < j; i++)
        {
          dbus_bus_add_match (connection, filters[i] + offset, &error);
          if (dbus_error_is_set (&error) && i == 0 && offset == 0)
            {
              /* We might be talking to a pre-1.5.6 dbus-daemon
              * which wouldn't understand eavesdrop=true.
              * If this works, carry on with offset > 0
              * on the remaining iterations. */
              offset = strlen (EAVESDROPPING_RULE) + 1;
              dbus_error_free (&error);
              dbus_bus_add_match (connection, filters[i] + offset, &error);
            }

	  if (dbus_error_is_set (&error))
            {
              fprintf (stderr, "Failed to setup match \"%s\": %s\n",
                       filters[i], error.message);
              dbus_error_free (&error);
              exit (1);
            }
          free(filters[i]);
        }
    }
  else
    {
      dbus_bus_add_match (connection,
                          EAVESDROPPING_RULE,
                          &error);
      if (dbus_error_is_set (&error))
        {
          dbus_error_free (&error);
          dbus_bus_add_match (connection,
                              "",
                              &error);
          if (dbus_error_is_set (&error))
            goto lose;
        }
    }

  if (!dbus_connection_add_filter (connection, filter_func, NULL, NULL)) {
    fprintf (stderr, "Couldn't add filter!\n");
    exit (1);
  }

  while (dbus_connection_read_write_dispatch(connection, -1))
    ;
  exit (0);
 lose:
  fprintf (stderr, "Error: %s\n", error.message);
  exit (1);
}

