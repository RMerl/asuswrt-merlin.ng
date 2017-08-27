/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*-
 * selinux.c  SELinux security checks for D-Bus
 *
 * Author: Matthew Rickard <mjricka@epoch.ncsc.mil>
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
#include <dbus/dbus-internals.h>
#include <dbus/dbus-string.h>
#ifndef DBUS_WIN
#include <dbus/dbus-userdb.h>
#endif
#include "selinux.h"
#include "services.h"
#include "policy.h"
#include "utils.h"
#include "config-parser.h"

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_SELINUX
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <syslog.h>
#include <selinux/selinux.h>
#include <selinux/avc.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <grp.h>
#endif /* HAVE_SELINUX */
#ifdef HAVE_LIBAUDIT
#include <cap-ng.h>
#include <libaudit.h>
#endif /* HAVE_LIBAUDIT */

#define BUS_SID_FROM_SELINUX(sid)  ((BusSELinuxID*) (sid))
#define SELINUX_SID_FROM_BUS(sid)  ((security_id_t) (sid))

#ifdef HAVE_SELINUX
/* Store the value telling us if SELinux is enabled in the kernel. */
static dbus_bool_t selinux_enabled = FALSE;

/* Store an avc_entry_ref to speed AVC decisions. */
static struct avc_entry_ref aeref;

/* Store the SID of the bus itself to use as the default. */
static security_id_t bus_sid = SECSID_WILD;

/* Thread to listen for SELinux status changes via netlink. */
static pthread_t avc_notify_thread;

/* Prototypes for AVC callback functions.  */
static void log_callback (const char *fmt, ...);
static void log_audit_callback (void *data, security_class_t class, char *buf, size_t bufleft);
static void *avc_create_thread (void (*run) (void));
static void avc_stop_thread (void *thread);
static void *avc_alloc_lock (void);
static void avc_get_lock (void *lock);
static void avc_release_lock (void *lock);
static void avc_free_lock (void *lock);

/* AVC callback structures for use in avc_init.  */
static const struct avc_memory_callback mem_cb =
{
  .func_malloc = dbus_malloc,
  .func_free = dbus_free
};
static const struct avc_log_callback log_cb =
{
  .func_log = log_callback,
  .func_audit = log_audit_callback
};
static const struct avc_thread_callback thread_cb =
{
  .func_create_thread = avc_create_thread,
  .func_stop_thread = avc_stop_thread
};
static const struct avc_lock_callback lock_cb =
{
  .func_alloc_lock = avc_alloc_lock,
  .func_get_lock = avc_get_lock,
  .func_release_lock = avc_release_lock,
  .func_free_lock = avc_free_lock
};
#endif /* HAVE_SELINUX */

/**
 * Log callback to log denial messages from the AVC.
 * This is used in avc_init.  Logs to both standard
 * error and syslogd.
 *
 * @param fmt the format string
 * @param variable argument list
 */
#ifdef HAVE_SELINUX

#ifdef HAVE_LIBAUDIT
static int audit_fd = -1;
#endif

void
bus_selinux_audit_init(void)
{
#ifdef HAVE_LIBAUDIT  
  audit_fd = audit_open ();

  if (audit_fd < 0)
    {
      /* If kernel doesn't support audit, bail out */
      if (errno == EINVAL || errno == EPROTONOSUPPORT || errno == EAFNOSUPPORT)
        return;
      /* If user bus, bail out */
      if (errno == EPERM && getuid() != 0)
        return;
      _dbus_warn ("Failed opening connection to the audit subsystem");
    }
#endif /* HAVE_LIBAUDIT */
}

static void 
log_callback (const char *fmt, ...) 
{
  va_list ap;

  va_start(ap, fmt);

#ifdef HAVE_LIBAUDIT
  if (audit_fd >= 0)
  {
    capng_get_caps_process();
    if (capng_have_capability(CAPNG_EFFECTIVE, CAP_AUDIT_WRITE))
    {
      char buf[PATH_MAX*2];
    
      /* FIXME: need to change this to show real user */
      vsnprintf(buf, sizeof(buf), fmt, ap);
      audit_log_user_avc_message(audit_fd, AUDIT_USER_AVC, buf, NULL, NULL,
                               NULL, getuid());
      return;
    }
  }
#endif /* HAVE_LIBAUDIT */
  
  vsyslog (LOG_USER | LOG_INFO, fmt, ap);
  va_end(ap);
}

/**
 * On a policy reload we need to reparse the SELinux configuration file, since
 * this could have changed.  Send a SIGHUP to reload all configs.
 */
static int
policy_reload_callback (u_int32_t event, security_id_t ssid, 
                        security_id_t tsid, security_class_t tclass, 
                        access_vector_t perms, access_vector_t *out_retained)
{
  if (event == AVC_CALLBACK_RESET)
    return raise (SIGHUP);
  
  return 0;
}

/**
 * Log any auxiliary data 
 */
static void
log_audit_callback (void *data, security_class_t class, char *buf, size_t bufleft)
{
  DBusString *audmsg = data;

  if (bufleft > (size_t) _dbus_string_get_length(audmsg))
    {
      _dbus_string_copy_to_buffer_with_nul (audmsg, buf, bufleft);
    }
  else
    {
      DBusString s;

      _dbus_string_init_const(&s, "Buffer too small for audit message");

      if (bufleft > (size_t) _dbus_string_get_length(&s))
        _dbus_string_copy_to_buffer_with_nul (&s, buf, bufleft);
    }
}

/**
 * Create thread to notify the AVC of enforcing and policy reload
 * changes via netlink.
 *
 * @param run the thread run function
 * @return pointer to the thread
 */
static void *
avc_create_thread (void (*run) (void))
{
  int rc;

  rc = pthread_create (&avc_notify_thread, NULL, (void *(*) (void *)) run, NULL);
  if (rc != 0)
    {
      _dbus_warn ("Failed to start AVC thread: %s\n", _dbus_strerror (rc));
      exit (1);
    }
  return &avc_notify_thread;
}

/* Stop AVC netlink thread.  */
static void
avc_stop_thread (void *thread)
{
  pthread_cancel (*(pthread_t *) thread);
}

/* Allocate a new AVC lock.  */
static void *
avc_alloc_lock (void)
{
  pthread_mutex_t *avc_mutex;

  avc_mutex = dbus_new (pthread_mutex_t, 1);
  if (avc_mutex == NULL)
    {
      _dbus_warn ("Could not create mutex: %s\n", _dbus_strerror (errno));
      exit (1);
    }
  pthread_mutex_init (avc_mutex, NULL);

  return avc_mutex;
}

/* Acquire an AVC lock.  */
static void
avc_get_lock (void *lock)
{
  pthread_mutex_lock (lock);
}

/* Release an AVC lock.  */
static void
avc_release_lock (void *lock)
{
  pthread_mutex_unlock (lock);
}

/* Free an AVC lock.  */
static void
avc_free_lock (void *lock)
{
  pthread_mutex_destroy (lock);
  dbus_free (lock);
}
#endif /* HAVE_SELINUX */

/**
 * Return whether or not SELinux is enabled; must be
 * called after bus_selinux_init.
 */
dbus_bool_t
bus_selinux_enabled (void)
{
#ifdef HAVE_SELINUX
  return selinux_enabled;
#else
  return FALSE;
#endif /* HAVE_SELINUX */
}

/**
 * Do early initialization; determine whether SELinux is enabled.
 */
dbus_bool_t
bus_selinux_pre_init (void)
{
#ifdef HAVE_SELINUX
  int r;
  _dbus_assert (bus_sid == SECSID_WILD);
  
  /* Determine if we are running an SELinux kernel. */
  r = is_selinux_enabled ();
  if (r < 0)
    {
      _dbus_warn ("Could not tell if SELinux is enabled: %s\n",
                  _dbus_strerror (errno));
      return FALSE;
    }

  selinux_enabled = r != 0;
  return TRUE;
#else
  return TRUE;
#endif
}

/*
 * Private Flask definitions; the order of these constants must
 * exactly match that of the structure array below!
 */
/* security dbus class constants */
#define SECCLASS_DBUS       1

/* dbus's per access vector constants */
#define DBUS__ACQUIRE_SVC   1
#define DBUS__SEND_MSG      2

#ifdef HAVE_SELINUX
static struct security_class_mapping dbus_map[] = {
  { "dbus", { "acquire_svc", "send_msg", NULL } },
  { NULL }
};
#endif /* HAVE_SELINUX */

/**
 * Establish dynamic object class and permission mapping and
 * initialize the user space access vector cache (AVC) for D-Bus and set up
 * logging callbacks.
 */
dbus_bool_t
bus_selinux_full_init (void)
{
#ifdef HAVE_SELINUX
  char *bus_context;

  _dbus_assert (bus_sid == SECSID_WILD);
  
  if (!selinux_enabled)
    {
      _dbus_verbose ("SELinux not enabled in this kernel.\n");
      return TRUE;
    }

  _dbus_verbose ("SELinux is enabled in this kernel.\n");

  if (selinux_set_mapping (dbus_map) < 0)
    {
      _dbus_warn ("Failed to set up security class mapping (selinux_set_mapping():%s).\n",
                   strerror (errno));
      return FALSE; 
    }

  avc_entry_ref_init (&aeref);
  if (avc_init ("avc", &mem_cb, &log_cb, &thread_cb, &lock_cb) < 0)
    {
      _dbus_warn ("Failed to start Access Vector Cache (AVC).\n");
      return FALSE;
    }
  else
    {
      _dbus_verbose ("Access Vector Cache (AVC) started.\n");
    }

  if (avc_add_callback (policy_reload_callback, AVC_CALLBACK_RESET,
                       NULL, NULL, 0, 0) < 0)
    {
      _dbus_warn ("Failed to add policy reload callback: %s\n",
                  _dbus_strerror (errno));
      avc_destroy ();
      return FALSE;
    }

  bus_context = NULL;
  bus_sid = SECSID_WILD;

  if (getcon (&bus_context) < 0)
    {
      _dbus_verbose ("Error getting context of bus: %s\n",
                     _dbus_strerror (errno));
      return FALSE;
    }
      
  if (avc_context_to_sid (bus_context, &bus_sid) < 0)
    {
      _dbus_verbose ("Error getting SID from bus context: %s\n",
                     _dbus_strerror (errno));
      freecon (bus_context);
      return FALSE;
    }

  freecon (bus_context);
  
#endif /* HAVE_SELINUX */
  return TRUE;
}

/**
 * Decrement SID reference count.
 * 
 * @param sid the SID to decrement
 */
void
bus_selinux_id_unref (BusSELinuxID *sid)
{
#ifdef HAVE_SELINUX
  if (!selinux_enabled)
    return;

  _dbus_assert (sid != NULL);
  
  sidput (SELINUX_SID_FROM_BUS (sid));
#endif /* HAVE_SELINUX */
}

void
bus_selinux_id_ref (BusSELinuxID *sid)
{
#ifdef HAVE_SELINUX
  if (!selinux_enabled)
    return;

  _dbus_assert (sid != NULL);
  
  sidget (SELINUX_SID_FROM_BUS (sid));
#endif /* HAVE_SELINUX */
}

/**
 * Determine if the SELinux security policy allows the given sender
 * security context to go to the given recipient security context.
 * This function determines if the requested permissions are to be
 * granted from the connection to the message bus or to another
 * optionally supplied security identifier (e.g. for a service
 * context).  Currently these permissions are either send_msg or
 * acquire_svc in the dbus class.
 *
 * @param sender_sid source security context
 * @param override_sid is the target security context.  If SECSID_WILD this will
 *        use the context of the bus itself (e.g. the default).
 * @param target_class is the target security class.
 * @param requested is the requested permissions.
 * @returns #TRUE if security policy allows the send.
 */
#ifdef HAVE_SELINUX
static dbus_bool_t
bus_selinux_check (BusSELinuxID        *sender_sid,
                   BusSELinuxID        *override_sid,
                   security_class_t     target_class,
                   access_vector_t      requested,
		   DBusString          *auxdata)
{
  if (!selinux_enabled)
    return TRUE;

  /* Make the security check.  AVC checks enforcing mode here as well. */
  if (avc_has_perm (SELINUX_SID_FROM_BUS (sender_sid),
                    override_sid ?
                    SELINUX_SID_FROM_BUS (override_sid) :
                    SELINUX_SID_FROM_BUS (bus_sid), 
                    target_class, requested, &aeref, auxdata) < 0)
    {
    switch (errno)
      {
      case EACCES:
        _dbus_verbose ("SELinux denying due to security policy.\n");
        return FALSE;
      case EINVAL:
        _dbus_verbose ("SELinux denying due to invalid security context.\n");
        return FALSE;
      default:
        _dbus_verbose ("SELinux denying due to: %s\n", _dbus_strerror (errno));
        return FALSE;
      }
    }
  else
    return TRUE;
}
#endif /* HAVE_SELINUX */

/**
 * Returns true if the given connection can acquire a service,
 * assuming the given security ID is needed for that service.
 *
 * @param connection connection that wants to own the service
 * @param service_sid the SID of the service from the table
 * @returns #TRUE if acquire is permitted.
 */
dbus_bool_t
bus_selinux_allows_acquire_service (DBusConnection     *connection,
                                    BusSELinuxID       *service_sid,
				    const char         *service_name,
				    DBusError          *error)
{
#ifdef HAVE_SELINUX
  BusSELinuxID *connection_sid;
  unsigned long spid;
  DBusString auxdata;
  dbus_bool_t ret;
  
  if (!selinux_enabled)
    return TRUE;
  
  connection_sid = bus_connection_get_selinux_id (connection);
  if (!dbus_connection_get_unix_process_id (connection, &spid))
    spid = 0;

  if (!_dbus_string_init (&auxdata))
    goto oom;
 
  if (!_dbus_string_append (&auxdata, "service="))
    goto oom;

  if (!_dbus_string_append (&auxdata, service_name))
    goto oom;

  if (spid)
    {
      if (!_dbus_string_append (&auxdata, " spid="))
	goto oom;

      if (!_dbus_string_append_uint (&auxdata, spid))
	goto oom;
    }
  
  ret = bus_selinux_check (connection_sid,
			   service_sid,
			   SECCLASS_DBUS,
			   DBUS__ACQUIRE_SVC,
			   &auxdata);

  _dbus_string_free (&auxdata);
  return ret;

 oom:
  _dbus_string_free (&auxdata);
  BUS_SET_OOM (error);
  return FALSE;

#else
  return TRUE;
#endif /* HAVE_SELINUX */
}

/**
 * Check if SELinux security controls allow the message to be sent to a
 * particular connection based on the security context of the sender and
 * that of the receiver. The destination connection need not be the
 * addressed recipient, it could be an "eavesdropper"
 *
 * @param sender the sender of the message.
 * @param proposed_recipient the connection the message is to be sent to.
 * @returns whether to allow the send
 */
dbus_bool_t
bus_selinux_allows_send (DBusConnection     *sender,
                         DBusConnection     *proposed_recipient,
			 const char         *msgtype,
			 const char         *interface,
			 const char         *member,
			 const char         *error_name,
			 const char         *destination,
			 DBusError          *error)
{
#ifdef HAVE_SELINUX
  BusSELinuxID *recipient_sid;
  BusSELinuxID *sender_sid;
  unsigned long spid, tpid;
  DBusString auxdata;
  dbus_bool_t ret;
  dbus_bool_t string_alloced;

  if (!selinux_enabled)
    return TRUE;

  if (!sender || !dbus_connection_get_unix_process_id (sender, &spid))
    spid = 0;
  if (!proposed_recipient || !dbus_connection_get_unix_process_id (proposed_recipient, &tpid))
    tpid = 0;

  string_alloced = FALSE;
  if (!_dbus_string_init (&auxdata))
    goto oom;
  string_alloced = TRUE;

  if (!_dbus_string_append (&auxdata, "msgtype="))
    goto oom;

  if (!_dbus_string_append (&auxdata, msgtype))
    goto oom;

  if (interface)
    {
      if (!_dbus_string_append (&auxdata, " interface="))
	goto oom;
      if (!_dbus_string_append (&auxdata, interface))
	goto oom;
    }

  if (member)
    {
      if (!_dbus_string_append (&auxdata, " member="))
	goto oom;
      if (!_dbus_string_append (&auxdata, member))
	goto oom;
    }

  if (error_name)
    {
      if (!_dbus_string_append (&auxdata, " error_name="))
	goto oom;
      if (!_dbus_string_append (&auxdata, error_name))
	goto oom;
    }

  if (destination)
    {
      if (!_dbus_string_append (&auxdata, " dest="))
	goto oom;
      if (!_dbus_string_append (&auxdata, destination))
	goto oom;
    }

  if (spid)
    {
      if (!_dbus_string_append (&auxdata, " spid="))
	goto oom;

      if (!_dbus_string_append_uint (&auxdata, spid))
	goto oom;
    }

  if (tpid)
    {
      if (!_dbus_string_append (&auxdata, " tpid="))
	goto oom;

      if (!_dbus_string_append_uint (&auxdata, tpid))
	goto oom;
    }

  sender_sid = bus_connection_get_selinux_id (sender);
  /* A NULL proposed_recipient means the bus itself. */
  if (proposed_recipient)
    recipient_sid = bus_connection_get_selinux_id (proposed_recipient);
  else
    recipient_sid = BUS_SID_FROM_SELINUX (bus_sid);

  ret = bus_selinux_check (sender_sid, 
			   recipient_sid,
			   SECCLASS_DBUS, 
			   DBUS__SEND_MSG,
			   &auxdata);

  _dbus_string_free (&auxdata);

  return ret;

 oom:
  if (string_alloced)
    _dbus_string_free (&auxdata);
  BUS_SET_OOM (error);
  return FALSE;
  
#else
  return TRUE;
#endif /* HAVE_SELINUX */
}

dbus_bool_t
bus_selinux_append_context (DBusMessage    *message,
			    BusSELinuxID   *sid,
			    DBusError      *error)
{
#ifdef HAVE_SELINUX
  char *context;

  if (avc_sid_to_context (SELINUX_SID_FROM_BUS (sid), &context) < 0)
    {
      if (errno == ENOMEM)
        BUS_SET_OOM (error);
      else
        dbus_set_error (error, DBUS_ERROR_FAILED,
                        "Error getting context from SID: %s\n",
			_dbus_strerror (errno));
      return FALSE;
    }
  if (!dbus_message_append_args (message,
				 DBUS_TYPE_ARRAY,
				 DBUS_TYPE_BYTE,
				 &context,
				 strlen (context),
				 DBUS_TYPE_INVALID))
    {
      _DBUS_SET_OOM (error);
      return FALSE;
    }
  freecon (context);
  return TRUE;
#else
  return TRUE;
#endif
}

/**
 * Gets the security context of a connection to the bus. It is up to
 * the caller to freecon() when they are done. 
 *
 * @param connection the connection to get the context of.
 * @param con the location to store the security context.
 * @returns #TRUE if context is successfully obtained.
 */
#ifdef HAVE_SELINUX
static dbus_bool_t
bus_connection_read_selinux_context (DBusConnection     *connection,
                                     char              **con)
{
  int fd;

  if (!selinux_enabled)
    return FALSE;

  _dbus_assert (connection != NULL);
  
  if (!dbus_connection_get_unix_fd (connection, &fd))
    {
      _dbus_verbose ("Failed to get file descriptor of socket.\n");
      return FALSE;
    }
  
  if (getpeercon (fd, con) < 0)
    {
      _dbus_verbose ("Error getting context of socket peer: %s\n",
                     _dbus_strerror (errno));
      return FALSE;
    }
  
  _dbus_verbose ("Successfully read connection context.\n");
  return TRUE;
}
#endif /* HAVE_SELINUX */

/**
 * Read the SELinux ID from the connection.
 *
 * @param connection the connection to read from
 * @returns the SID if successfully determined, #NULL otherwise.
 */
BusSELinuxID*
bus_selinux_init_connection_id (DBusConnection *connection,
                                DBusError      *error)
{
#ifdef HAVE_SELINUX
  char *con;
  security_id_t sid;
  
  if (!selinux_enabled)
    return NULL;

  if (!bus_connection_read_selinux_context (connection, &con))
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "Failed to read an SELinux context from connection");
      _dbus_verbose ("Error getting peer context.\n");
      return NULL;
    }

  _dbus_verbose ("Converting context to SID to store on connection\n");

  if (avc_context_to_sid (con, &sid) < 0)
    {
      if (errno == ENOMEM)
        BUS_SET_OOM (error);
      else
        dbus_set_error (error, DBUS_ERROR_FAILED,
                        "Error getting SID from context \"%s\": %s\n",
			con, _dbus_strerror (errno));
      
      _dbus_warn ("Error getting SID from context \"%s\": %s\n",
		  con, _dbus_strerror (errno));
      
      freecon (con);
      return NULL;
    }
 
  freecon (con); 
  return BUS_SID_FROM_SELINUX (sid);
#else
  return NULL;
#endif /* HAVE_SELINUX */
}


/**
 * Function for freeing hash table data.  These SIDs
 * should no longer be referenced.
 */
static void
bus_selinux_id_table_free_value (BusSELinuxID *sid)
{
#ifdef HAVE_SELINUX
  /* NULL sometimes due to how DBusHashTable works */
  if (sid)
    bus_selinux_id_unref (sid);
#endif /* HAVE_SELINUX */
}

/**
 * Creates a new table mapping service names to security ID.
 * A security ID is a "compiled" security context, a security
 * context is just a string.
 *
 * @returns the new table or #NULL if no memory
 */
DBusHashTable*
bus_selinux_id_table_new (void)
{
  return _dbus_hash_table_new (DBUS_HASH_STRING,
                               (DBusFreeFunction) dbus_free,
                               (DBusFreeFunction) bus_selinux_id_table_free_value);
}

/** 
 * Hashes a service name and service context into the service SID
 * table as a string and a SID.
 *
 * @param service_name is the name of the service.
 * @param service_context is the context of the service.
 * @param service_table is the table to hash them into.
 * @return #FALSE if not enough memory
 */
dbus_bool_t
bus_selinux_id_table_insert (DBusHashTable *service_table,
                             const char    *service_name,
                             const char    *service_context)
{
#ifdef HAVE_SELINUX
  dbus_bool_t retval;
  security_id_t sid;
  char *key;

  if (!selinux_enabled)
    return TRUE;

  sid = SECSID_WILD;
  retval = FALSE;

  key = _dbus_strdup (service_name);
  if (key == NULL)
    return retval;
  
  if (avc_context_to_sid ((char *) service_context, &sid) < 0)
    {
      if (errno == ENOMEM)
        {
	  dbus_free (key);
          return FALSE;
	}

      _dbus_warn ("Error getting SID from context \"%s\": %s\n",
		  (char *) service_context,
                  _dbus_strerror (errno));
      goto out;
    }

  if (!_dbus_hash_table_insert_string (service_table,
                                       key,
                                       BUS_SID_FROM_SELINUX (sid)))
    goto out;

  _dbus_verbose ("Parsed \tservice: %s \n\t\tcontext: %s\n",
                  key, 
                  sid->ctx);

  /* These are owned by the hash, so clear them to avoid unref */
  key = NULL;
  sid = SECSID_WILD;

  retval = TRUE;
  
 out:
  if (sid != SECSID_WILD)
    sidput (sid);

  if (key)
    dbus_free (key);

  return retval;
#else
  return TRUE;
#endif /* HAVE_SELINUX */
}


/**
 * Find the security identifier associated with a particular service
 * name.  Return a pointer to this SID, or #NULL/SECSID_WILD if the
 * service is not found in the hash table.  This should be nearly a
 * constant time operation.  If SELinux support is not available,
 * always return NULL.
 *
 * @param service_table the hash table to check for service name.
 * @param service_name the name of the service to look for.
 * @returns the SELinux ID associated with the service
 */
BusSELinuxID*
bus_selinux_id_table_lookup (DBusHashTable    *service_table,
                             const DBusString *service_name)
{
#ifdef HAVE_SELINUX
  security_id_t sid;

  sid = SECSID_WILD;     /* default context */

  if (!selinux_enabled)
    return NULL;
  
  _dbus_verbose ("Looking up service SID for %s\n",
                 _dbus_string_get_const_data (service_name));

  sid = _dbus_hash_table_lookup_string (service_table,
                                        _dbus_string_get_const_data (service_name));

  if (sid == SECSID_WILD)
    _dbus_verbose ("Service %s not found\n", 
                   _dbus_string_get_const_data (service_name));
  else
    _dbus_verbose ("Service %s found\n", 
                   _dbus_string_get_const_data (service_name));

  return BUS_SID_FROM_SELINUX (sid);
#endif /* HAVE_SELINUX */
  return NULL;
}

/**
 * Get the SELinux policy root.  This is used to find the D-Bus
 * specific config file within the policy.
 */
const char *
bus_selinux_get_policy_root (void)
{
#ifdef HAVE_SELINUX
  return selinux_policy_root ();
#else
  return NULL;
#endif /* HAVE_SELINUX */
} 

/**
 * For debugging:  Print out the current hash table of service SIDs.
 */
void
bus_selinux_id_table_print (DBusHashTable *service_table)
{
#if defined (DBUS_ENABLE_VERBOSE_MODE) && defined (HAVE_SELINUX)
  DBusHashIter iter;

  if (!selinux_enabled)
    return;
  
  _dbus_verbose ("Service SID Table:\n");
  _dbus_hash_iter_init (service_table, &iter);
  while (_dbus_hash_iter_next (&iter))
    {
      const char *key = _dbus_hash_iter_get_string_key (&iter);
      security_id_t sid = _dbus_hash_iter_get_value (&iter);
      _dbus_verbose ("The key is %s\n", key);
      _dbus_verbose ("The context is %s\n", sid->ctx);
      _dbus_verbose ("The refcount is %d\n", sid->refcnt);
    }
#endif /* DBUS_ENABLE_VERBOSE_MODE && HAVE_SELINUX */
}


/**
 * Print out some AVC statistics.
 */
#ifdef HAVE_SELINUX
static void
bus_avc_print_stats (void)
{
#ifdef DBUS_ENABLE_VERBOSE_MODE
  struct avc_cache_stats cstats;

  if (!selinux_enabled)
    return;
  
  _dbus_verbose ("AVC Statistics:\n");
  avc_cache_stats (&cstats);
  avc_av_stats ();
  _dbus_verbose ("AVC Cache Statistics:\n");
  _dbus_verbose ("Entry lookups: %d\n", cstats.entry_lookups);
  _dbus_verbose ("Entry hits: %d\n", cstats.entry_hits);
  _dbus_verbose ("Entry misses %d\n", cstats.entry_misses);
  _dbus_verbose ("Entry discards: %d\n", cstats.entry_discards);
  _dbus_verbose ("CAV lookups: %d\n", cstats.cav_lookups);
  _dbus_verbose ("CAV hits: %d\n", cstats.cav_hits);
  _dbus_verbose ("CAV probes: %d\n", cstats.cav_probes);
  _dbus_verbose ("CAV misses: %d\n", cstats.cav_misses);
#endif /* DBUS_ENABLE_VERBOSE_MODE */
}
#endif /* HAVE_SELINUX */

/**
 * Destroy the AVC before we terminate.
 */
void
bus_selinux_shutdown (void)
{
#ifdef HAVE_SELINUX
  if (!selinux_enabled)
    return;

  _dbus_verbose ("AVC shutdown\n");

  if (bus_sid != SECSID_WILD)
    {
      sidput (bus_sid);
      bus_sid = SECSID_WILD;

      bus_avc_print_stats ();

      avc_destroy ();
#ifdef HAVE_LIBAUDIT
      audit_close (audit_fd);
#endif /* HAVE_LIBAUDIT */
    }
#endif /* HAVE_SELINUX */
}

/* The !HAVE_LIBAUDIT case lives in dbus-sysdeps-util-unix.c */
#ifdef HAVE_LIBAUDIT
/**
 * Changes the user and group the bus is running as.
 *
 * @param user the user to become
 * @param error return location for errors
 * @returns #FALSE on failure
 */
dbus_bool_t
_dbus_change_to_daemon_user  (const char    *user,
                              DBusError     *error)
{
  dbus_uid_t uid;
  dbus_gid_t gid;
  DBusString u;

  _dbus_string_init_const (&u, user);

  if (!_dbus_get_user_id_and_primary_group (&u, &uid, &gid))
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "User '%s' does not appear to exist?",
                      user);
      return FALSE;
    }

  /* If we were root */
  if (_dbus_geteuid () == 0)
    {
      int rc;
      int have_audit_write;

      have_audit_write = capng_have_capability (CAPNG_PERMITTED, CAP_AUDIT_WRITE);
      capng_clear (CAPNG_SELECT_BOTH);
      /* Only attempt to retain CAP_AUDIT_WRITE if we had it when
       * starting.  See:
       * https://bugs.freedesktop.org/show_bug.cgi?id=49062#c9
       */
      if (have_audit_write)
        capng_update (CAPNG_ADD, CAPNG_EFFECTIVE | CAPNG_PERMITTED,
                      CAP_AUDIT_WRITE);
      rc = capng_change_id (uid, gid, CAPNG_DROP_SUPP_GRP);
      if (rc)
        {
          switch (rc) {
            default:
              dbus_set_error (error, DBUS_ERROR_FAILED,
                              "Failed to drop capabilities: %s\n",
                              _dbus_strerror (errno));
              break;
            case -4:
              dbus_set_error (error, _dbus_error_from_errno (errno),
                              "Failed to set GID to %lu: %s", gid,
                              _dbus_strerror (errno));
              break;
            case -5:
              _dbus_warn ("Failed to drop supplementary groups: %s\n",
                          _dbus_strerror (errno));
              break;
            case -6:
              dbus_set_error (error, _dbus_error_from_errno (errno),
                              "Failed to set UID to %lu: %s", uid,
                              _dbus_strerror (errno));
              break;
            case -7:
              dbus_set_error (error, _dbus_error_from_errno (errno),
                              "Failed to unset keep-capabilities: %s\n",
                              _dbus_strerror (errno));
              break;
          }
          return FALSE;
        }
    }

 return TRUE;
}
#endif
