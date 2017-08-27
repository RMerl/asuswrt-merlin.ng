/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-sysdeps.h Wrappers around system/libc features (internal to D-Bus implementation)
 * 
 * Copyright (C) 2002, 2003  Red Hat, Inc.
 * Copyright (C) 2003 CodeFactory AB
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

#ifndef DBUS_SYSDEPS_H
#define DBUS_SYSDEPS_H

#include "config.h"

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include <dbus/dbus-errors.h>
#include <dbus/dbus-file.h>
#include <dbus/dbus-string.h>

/* this is perhaps bogus, but strcmp() etc. are faster if we use the
 * stuff straight out of string.h, so have this here for now.
 */
#include <string.h>
#include <stdarg.h>

/* AIX sys/poll.h does #define events reqevents, and other
 * wonderousness, so must include sys/poll before declaring
 * DBusPollFD
 */ 
#ifdef HAVE_POLL
#include <sys/poll.h>
#endif

#ifdef DBUS_WINCE
/* Windows CE lacks some system functions (such as errno and clock).
   We bring them in here.  */
#include "dbus-sysdeps-wince-glue.h"
#endif

DBUS_BEGIN_DECLS

#ifdef DBUS_WIN
#define _DBUS_PATH_SEPARATOR ";"
#else
#define _DBUS_PATH_SEPARATOR ":"
#endif

/* Forward declarations */


/** An opaque list type */
typedef struct DBusList DBusList;

/** Object that contains a list of credentials such as UNIX or Windows user ID */
typedef struct DBusCredentials DBusCredentials;

/** A wrapper around a pipe descriptor or handle */
typedef struct DBusPipe DBusPipe;

/**
 * @addtogroup DBusSysdeps
 *
 * @{
 */

void _dbus_abort (void) _DBUS_GNUC_NORETURN;

dbus_bool_t _dbus_check_setuid (void);
const char* _dbus_getenv (const char *varname);
dbus_bool_t _dbus_clearenv (void);
char **     _dbus_get_environment (void);

/** A process ID */
typedef unsigned long dbus_pid_t;
/** A user ID */
typedef unsigned long dbus_uid_t;
/** A group ID */
typedef unsigned long dbus_gid_t;

/** an invalid PID used to represent an uninitialized dbus_pid_t field */
#define DBUS_PID_UNSET ((dbus_pid_t) -1)
/** an invalid UID used to represent an uninitialized dbus_uid_t field */
#define DBUS_UID_UNSET ((dbus_uid_t) -1)
/** an invalid GID used to represent an uninitialized dbus_gid_t field */
#define DBUS_GID_UNSET ((dbus_gid_t) -1)

/** an appropriate printf format for dbus_pid_t */
#define DBUS_PID_FORMAT "%lu"
/** an appropriate printf format for dbus_uid_t */
#define DBUS_UID_FORMAT "%lu"
/** an appropriate printf format for dbus_gid_t */
#define DBUS_GID_FORMAT "%lu"


/**
 * Socket interface
 *
 *  @todo Use for the file descriptors a struct
 *           - struct DBusSocket{ int d; }; -
 *        instead of int to get type-safety which 
 *        will be checked by the compiler.
 * 
 */

dbus_bool_t _dbus_close_socket     (int               fd,
                                    DBusError        *error);
int         _dbus_read_socket      (int               fd,
                                    DBusString       *buffer,
                                    int               count);
int         _dbus_write_socket     (int               fd,
                                    const DBusString *buffer,
                                    int               start,
                                    int               len);
int         _dbus_write_socket_two (int               fd,
                                    const DBusString *buffer1,
                                    int               start1,
                                    int               len1,
                                    const DBusString *buffer2,
                                    int               start2,
                                    int               len2);

int _dbus_read_socket_with_unix_fds      (int               fd,
                                          DBusString       *buffer,
                                          int               count,
                                          int              *fds,
                                          int              *n_fds);
int _dbus_write_socket_with_unix_fds     (int               fd,
                                          const DBusString *buffer,
                                          int               start,
                                          int               len,
                                          const int        *fds,
                                          int               n_fds);
int _dbus_write_socket_with_unix_fds_two (int               fd,
                                          const DBusString *buffer1,
                                          int               start1,
                                          int               len1,
                                          const DBusString *buffer2,
                                          int               start2,
                                          int               len2,
                                          const int        *fds,
                                          int               n_fds);

dbus_bool_t _dbus_socket_is_invalid (int              fd);

int _dbus_connect_tcp_socket  (const char     *host,
                               const char     *port,
                               const char     *family,
                               DBusError      *error);
int _dbus_connect_tcp_socket_with_nonce  (const char     *host,
                                          const char     *port,
                                          const char     *family,
                                          const char     *noncefile,
                                          DBusError      *error);
int _dbus_listen_tcp_socket   (const char     *host,
                               const char     *port,
                               const char     *family,
                               DBusString     *retport,
                               int           **fds_p,
                               DBusError      *error);
int _dbus_accept              (int             listen_fd);


dbus_bool_t _dbus_read_credentials_socket (int               client_fd,
                                           DBusCredentials  *credentials,
                                           DBusError        *error);
dbus_bool_t _dbus_send_credentials_socket (int              server_fd,
                                           DBusError       *error);

dbus_bool_t _dbus_credentials_add_from_user            (DBusCredentials  *credentials,
                                                        const DBusString *username);
dbus_bool_t _dbus_credentials_add_from_current_process (DBusCredentials  *credentials);
dbus_bool_t _dbus_append_user_from_current_process     (DBusString        *str);

dbus_bool_t _dbus_parse_unix_user_from_config   (const DBusString  *username,
                                                 dbus_uid_t        *uid_p);
dbus_bool_t _dbus_parse_unix_group_from_config  (const DBusString  *groupname,
                                                 dbus_gid_t        *gid_p);
dbus_bool_t _dbus_unix_groups_from_uid          (dbus_uid_t         uid,
                                                 dbus_gid_t       **group_ids,
                                                 int               *n_group_ids);
dbus_bool_t _dbus_unix_user_is_at_console       (dbus_uid_t         uid,
                                                 DBusError         *error);
dbus_bool_t _dbus_unix_user_is_process_owner    (dbus_uid_t         uid);
dbus_bool_t _dbus_windows_user_is_process_owner (const char        *windows_sid);

dbus_bool_t _dbus_append_keyring_directory_for_credentials (DBusString      *directory,
                                                            DBusCredentials *credentials);

dbus_bool_t _dbus_daemon_is_session_bus_address_published (const char *scope);

dbus_bool_t _dbus_daemon_publish_session_bus_address (const char* address, const char* shm_name);

void _dbus_daemon_unpublish_session_bus_address (void);

dbus_bool_t _dbus_socket_can_pass_unix_fd(int fd);

/** Opaque type representing an atomically-modifiable integer
 * that can be used from multiple threads.
 */
typedef struct DBusAtomic DBusAtomic;

/**
 * An atomic integer safe to increment or decrement from multiple threads.
 */
struct DBusAtomic
{
#ifdef DBUS_WIN
  volatile long value; /**< Value of the atomic integer. */
#else
  volatile dbus_int32_t value; /**< Value of the atomic integer. */
#endif
};

/* The value we get from autofoo is in the form of a cpp expression;
 * convert that to a conventional defined/undef switch. (We can't get
 * the conventional defined/undef because of multiarch builds only running
 * ./configure once, on Darwin.) */
#if DBUS_HAVE_ATOMIC_INT_COND
#   define DBUS_HAVE_ATOMIC_INT 1
#else
#   undef DBUS_HAVE_ATOMIC_INT
#endif

dbus_int32_t _dbus_atomic_inc (DBusAtomic *atomic);
dbus_int32_t _dbus_atomic_dec (DBusAtomic *atomic);
dbus_int32_t _dbus_atomic_get (DBusAtomic *atomic);


/* AIX uses different values for poll */

#ifdef _AIX
/** There is data to read */
#define _DBUS_POLLIN      0x0001
/** There is urgent data to read */
#define _DBUS_POLLPRI     0x0004
/** Writing now will not block */
#define _DBUS_POLLOUT     0x0002
/** Error condition */
#define _DBUS_POLLERR     0x4000
/** Hung up */
#define _DBUS_POLLHUP     0x2000
/** Invalid request: fd not open */
#define _DBUS_POLLNVAL    0x8000
#elif defined(__HAIKU__)
/** There is data to read */
#define _DBUS_POLLIN      0x0001
/** Writing now will not block */
#define _DBUS_POLLOUT     0x0002
/** Error condition */
#define _DBUS_POLLERR     0x0004
/** There is urgent data to read */
#define _DBUS_POLLPRI     0x0020
/** Hung up */
#define _DBUS_POLLHUP     0x0080
/** Invalid request: fd not open */
#define _DBUS_POLLNVAL    0x1000
#elif defined(__QNX__)
/** Writing now will not block */
#define _DBUS_POLLOUT     0x0002
/** There is data to read */
#define _DBUS_POLLIN      0x0005
/** There is urgent data to read */
#define _DBUS_POLLPRI     0x0008
/** Error condition */
#define _DBUS_POLLERR     0x0020
/** Hung up */
#define _DBUS_POLLHUP     0x0040
/** Invalid request: fd not open */
#define _DBUS_POLLNVAL    0x1000
#else
/** There is data to read */
#define _DBUS_POLLIN      0x0001
/** There is urgent data to read */
#define _DBUS_POLLPRI     0x0002
/** Writing now will not block */
#define _DBUS_POLLOUT     0x0004
/** Error condition */
#define _DBUS_POLLERR     0x0008
/** Hung up */
#define _DBUS_POLLHUP     0x0010
/** Invalid request: fd not open */
#define _DBUS_POLLNVAL    0x0020
#endif

/**
 * A portable struct pollfd wrapper. 
 */
typedef struct
{
  int fd;            /**< File descriptor */
  short events;      /**< Events to poll for */
  short revents;     /**< Events that occurred */
} DBusPollFD;

int _dbus_poll (DBusPollFD *fds,
                int         n_fds,
                int         timeout_milliseconds);

void _dbus_sleep_milliseconds (int milliseconds);

void _dbus_get_monotonic_time (long *tv_sec,
                               long *tv_usec);

void _dbus_get_real_time (long *tv_sec,
                          long *tv_usec);

/**
 * directory interface
 */
dbus_bool_t    _dbus_create_directory        (const DBusString *filename,
                                              DBusError        *error);
dbus_bool_t    _dbus_delete_directory        (const DBusString *filename,
					      DBusError        *error);

dbus_bool_t _dbus_concat_dir_and_file (DBusString       *dir,
                                       const DBusString *next_component);
dbus_bool_t _dbus_string_get_dirname  (const DBusString *filename,
                                       DBusString       *dirname);
dbus_bool_t _dbus_path_is_absolute    (const DBusString *filename);

dbus_bool_t _dbus_get_standard_session_servicedirs (DBusList **dirs);
dbus_bool_t _dbus_get_standard_system_servicedirs (DBusList **dirs);

dbus_bool_t _dbus_append_system_config_file  (DBusString *str);
dbus_bool_t _dbus_append_session_config_file (DBusString *str);

/** Opaque type for reading a directory listing */
typedef struct DBusDirIter DBusDirIter;

DBusDirIter* _dbus_directory_open          (const DBusString *filename,
                                            DBusError        *error);
dbus_bool_t  _dbus_directory_get_next_file (DBusDirIter      *iter,
                                            DBusString       *filename,
                                            DBusError        *error);
void         _dbus_directory_close         (DBusDirIter      *iter);

dbus_bool_t  _dbus_check_dir_is_private_to_user    (DBusString *dir,
                                                    DBusError *error);

void _dbus_fd_set_close_on_exec (intptr_t fd);

const char* _dbus_get_tmpdir      (void);

/**
 * Random numbers 
 */
void        _dbus_generate_pseudorandom_bytes_buffer (char *buffer,
                                                      int   n_bytes);
void        _dbus_generate_random_bytes_buffer (char       *buffer,
                                                int         n_bytes);
dbus_bool_t _dbus_generate_random_bytes        (DBusString *str,
                                                int         n_bytes);
dbus_bool_t _dbus_generate_random_ascii        (DBusString *str,
                                                int         n_bytes);

const char* _dbus_error_from_errno (int error_number);
const char* _dbus_error_from_system_errno (void);

void        _dbus_set_errno_to_zero                  (void);
dbus_bool_t _dbus_get_is_errno_nonzero               (void);
dbus_bool_t _dbus_get_is_errno_eagain_or_ewouldblock (void);
dbus_bool_t _dbus_get_is_errno_enomem                (void);
dbus_bool_t _dbus_get_is_errno_eintr                 (void);
dbus_bool_t _dbus_get_is_errno_epipe                 (void);
dbus_bool_t _dbus_get_is_errno_etoomanyrefs           (void);
const char* _dbus_strerror_from_errno                (void);

void _dbus_disable_sigpipe (void);


void _dbus_exit (int code) _DBUS_GNUC_NORETURN;

int _dbus_printf_string_upper_bound (const char *format,
                                     va_list args);


/**
 * Portable struct with stat() results
 */
typedef struct
{
  unsigned long mode;  /**< File mode */
  unsigned long nlink; /**< Number of hard links */
  dbus_uid_t    uid;   /**< User owning file */
  dbus_gid_t    gid;   /**< Group owning file */
  unsigned long size;  /**< Size of file */
  unsigned long atime; /**< Access time */
  unsigned long mtime; /**< Modify time */
  unsigned long ctime; /**< Creation time */
} DBusStat;

dbus_bool_t _dbus_stat             (const DBusString *filename,
                                    DBusStat         *statbuf,
                                    DBusError        *error);
dbus_bool_t _dbus_full_duplex_pipe (int              *fd1,
                                    int              *fd2,
                                    dbus_bool_t       blocking,
                                    DBusError        *error);

void        _dbus_print_backtrace  (void);

dbus_bool_t _dbus_become_daemon   (const DBusString *pidfile,
                                   DBusPipe         *print_pid_pipe,
                                   DBusError        *error,
                                   dbus_bool_t       keep_umask);

dbus_bool_t _dbus_verify_daemon_user    (const char *user);
dbus_bool_t _dbus_change_to_daemon_user (const char *user,
                                         DBusError  *error);

dbus_bool_t _dbus_write_pid_to_file_and_pipe (const DBusString *pidfile,
                                              DBusPipe         *print_pid_pipe,
                                              dbus_pid_t        pid_to_write,
                                              DBusError        *error);

dbus_bool_t _dbus_command_for_pid (unsigned long  pid,
                                   DBusString    *str,
                                   int            max_len,
                                   DBusError     *error);

/** A UNIX signal handler */
typedef void (* DBusSignalHandler) (int sig);

void _dbus_set_signal_handler (int               sig,
                               DBusSignalHandler handler);

dbus_bool_t _dbus_user_at_console (const char *username,
                                   DBusError  *error);

void _dbus_init_system_log (dbus_bool_t is_daemon);

typedef enum {
  DBUS_SYSTEM_LOG_INFO,
  DBUS_SYSTEM_LOG_SECURITY,
  DBUS_SYSTEM_LOG_FATAL
} DBusSystemLogSeverity;

void _dbus_system_log (DBusSystemLogSeverity severity, const char *msg, ...) _DBUS_GNUC_PRINTF (2, 3);
void _dbus_system_logv (DBusSystemLogSeverity severity, const char *msg, va_list args);

/* Define DBUS_VA_COPY() to do the right thing for copying va_list variables.
 * config.h may have already defined DBUS_VA_COPY as va_copy or __va_copy.
 */
#if !defined (DBUS_VA_COPY)
#  if defined (__GNUC__) && defined (__PPC__) && (defined (_CALL_SYSV) || defined (_WIN32))
#    define DBUS_VA_COPY(ap1, ap2)   (*(ap1) = *(ap2))
#  elif defined (DBUS_VA_COPY_AS_ARRAY)
#    define DBUS_VA_COPY(ap1, ap2)   memcpy ((ap1), (ap2), sizeof (va_list))
#  else /* va_list is a pointer */
#    define DBUS_VA_COPY(ap1, ap2)   ((ap1) = (ap2))
#  endif /* va_list is a pointer */
#endif /* !DBUS_VA_COPY */


/**
 * Casts a primitive C type to a byte array and then indexes
 * a particular byte of the array.
 */
#define _DBUS_BYTE_OF_PRIMITIVE(p, i) \
    (((const char*)&(p))[(i)])
/** On x86 there is an 80-bit FPU, and if you do "a == b" it may have a
 * or b in an 80-bit register, thus failing to compare the two 64-bit
 * doubles for bitwise equality. So this macro compares the two doubles
 * bitwise.
 */
#define _DBUS_DOUBLES_BITWISE_EQUAL(a, b)                                       \
     (_DBUS_BYTE_OF_PRIMITIVE (a, 0) == _DBUS_BYTE_OF_PRIMITIVE (b, 0) &&       \
      _DBUS_BYTE_OF_PRIMITIVE (a, 1) == _DBUS_BYTE_OF_PRIMITIVE (b, 1) &&       \
      _DBUS_BYTE_OF_PRIMITIVE (a, 2) == _DBUS_BYTE_OF_PRIMITIVE (b, 2) &&       \
      _DBUS_BYTE_OF_PRIMITIVE (a, 3) == _DBUS_BYTE_OF_PRIMITIVE (b, 3) &&       \
      _DBUS_BYTE_OF_PRIMITIVE (a, 4) == _DBUS_BYTE_OF_PRIMITIVE (b, 4) &&       \
      _DBUS_BYTE_OF_PRIMITIVE (a, 5) == _DBUS_BYTE_OF_PRIMITIVE (b, 5) &&       \
      _DBUS_BYTE_OF_PRIMITIVE (a, 6) == _DBUS_BYTE_OF_PRIMITIVE (b, 6) &&       \
      _DBUS_BYTE_OF_PRIMITIVE (a, 7) == _DBUS_BYTE_OF_PRIMITIVE (b, 7))

dbus_bool_t _dbus_get_autolaunch_address (const char *scope,
                                          DBusString *address,
					                      DBusError  *error);

dbus_bool_t _dbus_lookup_session_address (dbus_bool_t *supported,
                                          DBusString  *address,
                                          DBusError   *error);

/** Type representing a universally unique ID
 * @todo rename to UUID instead of GUID
 */
typedef union DBusGUID DBusGUID;

dbus_bool_t _dbus_read_local_machine_uuid   (DBusGUID         *machine_id,
                                             dbus_bool_t       create_if_not_found,
                                             DBusError        *error);

/**
 * Initialize threads as in dbus_threads_init_default(), appropriately
 * for the platform.
 * @returns #FALSE if no memory
 */
dbus_bool_t _dbus_threads_init_platform_specific (void);

/**
 * Lock a static mutex used to protect _dbus_threads_init_platform_specific().
 */
void _dbus_threads_lock_platform_specific (void);

/**
 * Undo _dbus_threads_lock_platform_specific().
 */
void _dbus_threads_unlock_platform_specific (void);

dbus_bool_t _dbus_split_paths_and_append (DBusString *dirs, 
                                          const char *suffix, 
                                          DBusList **dir_list);

unsigned long _dbus_pid_for_log (void);

/* FIXME move back to dbus-sysdeps-unix.h probably -
 * the PID file handling just needs a little more abstraction
 * in the bus daemon first.
 */
dbus_pid_t    _dbus_getpid (void);

dbus_bool_t _dbus_change_to_daemon_user (const char *user,
                                         DBusError  *error);

void _dbus_flush_caches (void);

void _dbus_request_file_descriptor_limit (unsigned int limit);

/*
 * replaces the term DBUS_PREFIX in configure_time_path by the
 * current dbus installation directory. On unix this function is a noop
 *
 * @param configure_time_path
 * @return real path
 */
const char *
_dbus_replace_install_prefix (const char *configure_time_path);

/* Do not set this too high: it is a denial-of-service risk.
 * See <https://bugs.freedesktop.org/show_bug.cgi?id=82820>
 *
 * (This needs to be in the non-Unix-specific header so that
 * the config-parser can use it.)
 */
#define DBUS_DEFAULT_MESSAGE_UNIX_FDS 16

/** @} */

DBUS_END_DECLS


#ifdef DBUS_WIN
#include "dbus-sysdeps-win.h"
#endif

#endif /* DBUS_SYSDEPS_H */
