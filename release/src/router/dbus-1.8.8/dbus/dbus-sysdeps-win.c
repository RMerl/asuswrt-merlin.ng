/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-sysdeps.c Wrappers around system/libc features (internal to D-BUS implementation)
 * 
 * Copyright (C) 2002, 2003  Red Hat, Inc.
 * Copyright (C) 2003 CodeFactory AB
 * Copyright (C) 2005 Novell, Inc.
 * Copyright (C) 2006 Peter Kümmel  <syntheticpp@gmx.net>
 * Copyright (C) 2006 Christian Ehrlicher <ch.ehrlicher@gmx.de>
 * Copyright (C) 2006-2013 Ralf Habacker <ralf.habacker@freenet.de>
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

#define STRSAFE_NO_DEPRECATE

#ifndef DBUS_WINCE
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#endif

#include "dbus-internals.h"
#include "dbus-sha.h"
#include "dbus-sysdeps.h"
#include "dbus-threads.h"
#include "dbus-protocol.h"
#include "dbus-string.h"
#include "dbus-sysdeps.h"
#include "dbus-sysdeps-win.h"
#include "dbus-protocol.h"
#include "dbus-hash.h"
#include "dbus-sockets-win.h"
#include "dbus-list.h"
#include "dbus-nonce.h"
#include "dbus-credentials.h"

#include <windows.h>
#include <ws2tcpip.h>
#include <wincrypt.h>
#include <iphlpapi.h>

/* Declarations missing in mingw's and windows sdk 7.0 headers */
extern BOOL WINAPI ConvertStringSidToSidA (LPCSTR  StringSid, PSID *Sid);
extern BOOL WINAPI ConvertSidToStringSidA (PSID Sid, LPSTR *StringSid);

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#if HAVE_ERRNO_H
#include <errno.h>
#endif
#ifndef DBUS_WINCE
#include <mbstring.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifdef HAVE_WS2TCPIP_H
/* getaddrinfo for Windows CE (and Windows).  */
#include <ws2tcpip.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef PROCESS_QUERY_LIMITED_INFORMATION
/* MinGW32 < 4 does not define this value in its headers */
#define PROCESS_QUERY_LIMITED_INFORMATION (0x1000)
#endif

typedef int socklen_t;


void
_dbus_win_set_errno (int err)
{
#ifdef DBUS_WINCE
  SetLastError (err);
#else
  errno = err;
#endif
}

static BOOL is_winxp_sp3_or_lower();

/*
 * _MIB_TCPROW_EX and friends are not available in system headers
 *  and are mapped to attribute identical ...OWNER_PID typedefs.
 */
typedef MIB_TCPROW_OWNER_PID _MIB_TCPROW_EX;
typedef MIB_TCPTABLE_OWNER_PID MIB_TCPTABLE_EX;
typedef PMIB_TCPTABLE_OWNER_PID PMIB_TCPTABLE_EX;
typedef DWORD (WINAPI *ProcAllocateAndGetTcpExtTableFromStack)(PMIB_TCPTABLE_EX*,BOOL,HANDLE,DWORD,DWORD);
static ProcAllocateAndGetTcpExtTableFromStack lpfnAllocateAndGetTcpExTableFromStack = NULL;

/**
 * AllocateAndGetTcpExTableFromStack() is undocumented and not exported,
 * but is the only way to do this in older XP versions.
 * @return true if the procedures could be loaded
 */
static BOOL
load_ex_ip_helper_procedures(void)
{
  HMODULE hModule = LoadLibrary ("iphlpapi.dll");
  if (hModule == NULL)
    {
      _dbus_verbose ("could not load iphlpapi.dll\n");
      return FALSE;
    }

  lpfnAllocateAndGetTcpExTableFromStack = (ProcAllocateAndGetTcpExtTableFromStack)GetProcAddress (hModule, "AllocateAndGetTcpExTableFromStack");
  if (lpfnAllocateAndGetTcpExTableFromStack == NULL)
    {
      _dbus_verbose ("could not find function AllocateAndGetTcpExTableFromStack in iphlpapi.dll\n");
      return FALSE;
    }
  return TRUE;
}

/**
 * get pid from localhost tcp connection using peer_port
 * This function is available on WinXP >= SP3
 * @param peer_port peers tcp port
 * @return process id or 0 if connection has not been found
 */
static dbus_pid_t
get_pid_from_extended_tcp_table(int peer_port)
{
  dbus_pid_t result;
  DWORD errorCode, size, i;
  MIB_TCPTABLE_OWNER_PID *tcp_table;

  if ((errorCode =
       GetExtendedTcpTable (NULL, &size, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0)) == ERROR_INSUFFICIENT_BUFFER)
    {
      tcp_table = (MIB_TCPTABLE_OWNER_PID *) dbus_malloc (size);
      if (tcp_table == NULL)
        {
          _dbus_verbose ("Error allocating memory\n");
          return 0;
        }
    }
  else
    {
      _dbus_win_warn_win_error ("unexpected error returned from GetExtendedTcpTable", errorCode);
      return 0;
    }

  if ((errorCode = GetExtendedTcpTable (tcp_table, &size, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0)) != NO_ERROR)
    {
      _dbus_verbose ("Error fetching tcp table %d\n", (int)errorCode);
      dbus_free (tcp_table);
      return 0;
    }

  result = 0;
  for (i = 0; i < tcp_table->dwNumEntries; i++)
    {
      MIB_TCPROW_OWNER_PID *p = &tcp_table->table[i];
      int local_address = ntohl (p->dwLocalAddr);
      int local_port = ntohs (p->dwLocalPort);
      if (p->dwState == MIB_TCP_STATE_ESTAB
          && local_address == INADDR_LOOPBACK && local_port == peer_port)
         result = p->dwOwningPid;
    }

  dbus_free (tcp_table);
  _dbus_verbose ("got pid %lu\n", result);
  return result;
}

/**
 * get pid from localhost tcp connection using peer_port
 * This function is available on all WinXP versions, but
 * not in wine (at least version <= 1.6.0)
 * @param peer_port peers tcp port
 * @return process id or 0 if connection has not been found
 */
static dbus_pid_t
get_pid_from_tcp_ex_table(int peer_port)
{
  dbus_pid_t result;
  DWORD errorCode, i;
  PMIB_TCPTABLE_EX tcp_table = NULL;

  if (!load_ex_ip_helper_procedures ())
    {
      _dbus_verbose
        ("Error not been able to load iphelper procedures\n");
      return 0;
    }

  errorCode = lpfnAllocateAndGetTcpExTableFromStack (&tcp_table, TRUE, GetProcessHeap(), 0, 2);

  if (errorCode != NO_ERROR)
    {
      _dbus_verbose
        ("Error not been able to call AllocateAndGetTcpExTableFromStack()\n");
      return 0;
    }

  result = 0;
  for (i = 0; i < tcp_table->dwNumEntries; i++)
    {
      _MIB_TCPROW_EX *p = &tcp_table->table[i];
      int local_port = ntohs (p->dwLocalPort);
      int local_address = ntohl (p->dwLocalAddr);
      if (local_address == INADDR_LOOPBACK && local_port == peer_port)
        {
          result = p->dwOwningPid;
          break;
        }
    }

  HeapFree (GetProcessHeap(), 0, tcp_table);
  _dbus_verbose ("got pid %lu\n", result);
  return result;
}

/**
 * @brief return peer process id from tcp handle for localhost connections
 * @param handle tcp socket descriptor
 * @return process id or 0 in case the process id could not be fetched
 */
static dbus_pid_t
_dbus_get_peer_pid_from_tcp_handle (int handle)
{
  struct sockaddr_storage addr;
  socklen_t len = sizeof (addr);
  int peer_port;

  dbus_pid_t result;
  dbus_bool_t is_localhost = FALSE;

  getpeername (handle, (struct sockaddr *) &addr, &len);

  if (addr.ss_family == AF_INET)
    {
      struct sockaddr_in *s = (struct sockaddr_in *) &addr;
      peer_port = ntohs (s->sin_port);
      is_localhost = (ntohl (s->sin_addr.s_addr) == INADDR_LOOPBACK);
    }
  else if (addr.ss_family == AF_INET6)
    {
      _dbus_verbose ("FIXME [61922]: IPV6 support not working on windows\n");
      return 0;
      /*
         struct sockaddr_in6 *s = (struct sockaddr_in6 * )&addr;
         peer_port = ntohs (s->sin6_port);
         is_localhost = (memcmp(s->sin6_addr.s6_addr, in6addr_loopback.s6_addr, 16) == 0);
         _dbus_verbose ("IPV6 %08x %08x\n", s->sin6_addr.s6_addr, in6addr_loopback.s6_addr);
       */
    }
  else
    {
      _dbus_verbose ("no idea what address family %d is\n", addr.ss_family);
      return 0;
    }

  if (!is_localhost)
    {
      _dbus_verbose ("could not fetch process id from remote process\n");
      return 0;
    }

  if (peer_port == 0)
    {
      _dbus_verbose
        ("Error not been able to fetch tcp peer port from connection\n");
      return 0;
    }

  _dbus_verbose ("trying to get peers pid");

  result = get_pid_from_extended_tcp_table (peer_port);
  if (result > 0)
      return result;
  result = get_pid_from_tcp_ex_table (peer_port);
  return result;
}

/* Convert GetLastError() to a dbus error.  */
const char*
_dbus_win_error_from_last_error (void)
{
  switch (GetLastError())
    {
    case 0:
      return DBUS_ERROR_FAILED;
    
    case ERROR_NO_MORE_FILES:
    case ERROR_TOO_MANY_OPEN_FILES:
      return DBUS_ERROR_LIMITS_EXCEEDED; /* kernel out of memory */

    case ERROR_ACCESS_DENIED:
    case ERROR_CANNOT_MAKE:
      return DBUS_ERROR_ACCESS_DENIED;

    case ERROR_NOT_ENOUGH_MEMORY:
      return DBUS_ERROR_NO_MEMORY;

    case ERROR_FILE_EXISTS:
      return DBUS_ERROR_FILE_EXISTS;

    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
      return DBUS_ERROR_FILE_NOT_FOUND;
    }
  
  return DBUS_ERROR_FAILED;
}


char*
_dbus_win_error_string (int error_number)
{
  char *msg;

  FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_IGNORE_INSERTS |
                  FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, error_number, 0,
                  (LPSTR) &msg, 0, NULL);

  if (msg[strlen (msg) - 1] == '\n')
    msg[strlen (msg) - 1] = '\0';
  if (msg[strlen (msg) - 1] == '\r')
    msg[strlen (msg) - 1] = '\0';

  return msg;
}

void
_dbus_win_free_error_string (char *string)
{
  LocalFree (string);
}

/**
 * Socket interface
 *
 */

/**
 * Thin wrapper around the read() system call that appends
 * the data it reads to the DBusString buffer. It appends
 * up to the given count, and returns the same value
 * and same errno as read(). The only exception is that
 * _dbus_read_socket() handles EINTR for you. 
 * _dbus_read_socket() can return ENOMEM, even though 
 * regular UNIX read doesn't.
 *
 * @param fd the file descriptor to read from
 * @param buffer the buffer to append data to
 * @param count the amount of data to read
 * @returns the number of bytes read or -1
 */

int
_dbus_read_socket (int               fd,
                   DBusString       *buffer,
                   int               count)
{
  int bytes_read;
  int start;
  char *data;

  _dbus_assert (count >= 0);

  start = _dbus_string_get_length (buffer);

  if (!_dbus_string_lengthen (buffer, count))
    {
      _dbus_win_set_errno (ENOMEM);
      return -1;
    }

  data = _dbus_string_get_data_len (buffer, start, count);

 again:
 
  _dbus_verbose ("recv: count=%d fd=%d\n", count, fd);
  bytes_read = recv (fd, data, count, 0);
  
  if (bytes_read == SOCKET_ERROR)
	{
	  DBUS_SOCKET_SET_ERRNO();
	  _dbus_verbose ("recv: failed: %s (%d)\n", _dbus_strerror (errno), errno);
	  bytes_read = -1;
	}
	else
	  _dbus_verbose ("recv: = %d\n", bytes_read);

  if (bytes_read < 0)
    {
      if (errno == EINTR)
        goto again;
      else    	
        {
          /* put length back (note that this doesn't actually realloc anything) */
          _dbus_string_set_length (buffer, start);
          return -1;
        }
    }
  else
    {
      /* put length back (doesn't actually realloc) */
      _dbus_string_set_length (buffer, start + bytes_read);

#if 0
      if (bytes_read > 0)
        _dbus_verbose_bytes_of_string (buffer, start, bytes_read);
#endif

      return bytes_read;
    }
}

/**
 * Thin wrapper around the write() system call that writes a part of a
 * DBusString and handles EINTR for you.
 * 
 * @param fd the file descriptor to write
 * @param buffer the buffer to write data from
 * @param start the first byte in the buffer to write
 * @param len the number of bytes to try to write
 * @returns the number of bytes written or -1 on error
 */
int
_dbus_write_socket (int               fd,
                    const DBusString *buffer,
                    int               start,
                    int               len)
{
  const char *data;
  int bytes_written;

  data = _dbus_string_get_const_data_len (buffer, start, len);

 again:

  _dbus_verbose ("send: len=%d fd=%d\n", len, fd);
  bytes_written = send (fd, data, len, 0);

  if (bytes_written == SOCKET_ERROR)
    {
      DBUS_SOCKET_SET_ERRNO();
      _dbus_verbose ("send: failed: %s\n", _dbus_strerror_from_errno ());
      bytes_written = -1;
    }
    else
      _dbus_verbose ("send: = %d\n", bytes_written);

  if (bytes_written < 0 && errno == EINTR)
    goto again;
    
#if 0
  if (bytes_written > 0)
    _dbus_verbose_bytes_of_string (buffer, start, bytes_written);
#endif

  return bytes_written;
}


/**
 * Closes a file descriptor.
 *
 * @param fd the file descriptor
 * @param error error object
 * @returns #FALSE if error set
 */
dbus_bool_t
_dbus_close_socket (int        fd,
                    DBusError *error)
{
  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

 again:
  if (closesocket (fd) == SOCKET_ERROR)
    {
      DBUS_SOCKET_SET_ERRNO ();
      
      if (errno == EINTR)
        goto again;
        
      dbus_set_error (error, _dbus_error_from_errno (errno),
                      "Could not close socket: socket=%d, , %s",
                      fd, _dbus_strerror_from_errno ());
      return FALSE;
    }
  _dbus_verbose ("_dbus_close_socket: socket=%d, \n", fd);

  return TRUE;
}

/**
 * Sets the file descriptor to be close
 * on exec. Should be called for all file
 * descriptors in D-Bus code.
 *
 * @param handle the Windows HANDLE
 */
void
_dbus_fd_set_close_on_exec (intptr_t handle)
{
  if ( !SetHandleInformation( (HANDLE) handle,
                        HANDLE_FLAG_INHERIT | HANDLE_FLAG_PROTECT_FROM_CLOSE,
                        0 /*disable both flags*/ ) )
    {
      _dbus_win_warn_win_error ("Disabling socket handle inheritance failed:", GetLastError());
    }
}

/**
 * Sets a file descriptor to be nonblocking.
 *
 * @param handle the file descriptor.
 * @param error address of error location.
 * @returns #TRUE on success.
 */
dbus_bool_t
_dbus_set_fd_nonblocking (int             handle,
                          DBusError      *error)
{
  u_long one = 1;

  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  if (ioctlsocket (handle, FIONBIO, &one) == SOCKET_ERROR)
    {
      DBUS_SOCKET_SET_ERRNO ();
      dbus_set_error (error, _dbus_error_from_errno (errno),
                      "Failed to set socket %d:%d to nonblocking: %s", handle,
                      _dbus_strerror_from_errno ());
      return FALSE;
    }

  return TRUE;
}


/**
 * Like _dbus_write() but will use writev() if possible
 * to write both buffers in sequence. The return value
 * is the number of bytes written in the first buffer,
 * plus the number written in the second. If the first
 * buffer is written successfully and an error occurs
 * writing the second, the number of bytes in the first
 * is returned (i.e. the error is ignored), on systems that
 * don't have writev. Handles EINTR for you.
 * The second buffer may be #NULL.
 *
 * @param fd the file descriptor
 * @param buffer1 first buffer
 * @param start1 first byte to write in first buffer
 * @param len1 number of bytes to write from first buffer
 * @param buffer2 second buffer, or #NULL
 * @param start2 first byte to write in second buffer
 * @param len2 number of bytes to write in second buffer
 * @returns total bytes written from both buffers, or -1 on error
 */
int
_dbus_write_socket_two (int               fd,
                        const DBusString *buffer1,
                        int               start1,
                        int               len1,
                        const DBusString *buffer2,
                        int               start2,
                        int               len2)
{
  WSABUF vectors[2];
  const char *data1;
  const char *data2;
  int rc;
  DWORD bytes_written;

  _dbus_assert (buffer1 != NULL);
  _dbus_assert (start1 >= 0);
  _dbus_assert (start2 >= 0);
  _dbus_assert (len1 >= 0);
  _dbus_assert (len2 >= 0);


  data1 = _dbus_string_get_const_data_len (buffer1, start1, len1);

  if (buffer2 != NULL)
    data2 = _dbus_string_get_const_data_len (buffer2, start2, len2);
  else
    {
      data2 = NULL;
      start2 = 0;
      len2 = 0;
    }

  vectors[0].buf = (char*) data1;
  vectors[0].len = len1;
  vectors[1].buf = (char*) data2;
  vectors[1].len = len2;

 again:
 
  _dbus_verbose ("WSASend: len1+2=%d+%d fd=%d\n", len1, len2, fd);
  rc = WSASend (fd, 
                vectors,
                data2 ? 2 : 1, 
                &bytes_written,
                0, 
                NULL, 
                NULL);
                
  if (rc == SOCKET_ERROR)
    {
      DBUS_SOCKET_SET_ERRNO ();
      _dbus_verbose ("WSASend: failed: %s\n", _dbus_strerror_from_errno ());
      bytes_written = -1;
    }
  else
    _dbus_verbose ("WSASend: = %ld\n", bytes_written);
    
  if (bytes_written < 0 && errno == EINTR)
    goto again;
      
  return bytes_written;
}

dbus_bool_t
_dbus_socket_is_invalid (int fd)
{
    return fd == INVALID_SOCKET ? TRUE : FALSE;
}

#if 0

/**
 * Opens the client side of a Windows named pipe. The connection D-BUS
 * file descriptor index is returned. It is set up as nonblocking.
 * 
 * @param path the path to named pipe socket
 * @param error return location for error code
 * @returns connection D-BUS file descriptor or -1 on error
 */
int
_dbus_connect_named_pipe (const char     *path,
                          DBusError      *error)
{
  _dbus_assert_not_reached ("not implemented");
}

#endif

/**
 * @returns #FALSE if no memory
 */
dbus_bool_t
_dbus_win_startup_winsock (void)
{
  /* Straight from MSDN, deuglified */

  /* Protected by _DBUS_LOCK_sysdeps */
  static dbus_bool_t beenhere = FALSE;

  WORD wVersionRequested;
  WSADATA wsaData;
  int err;

  if (!_DBUS_LOCK (sysdeps))
    return FALSE;

  if (beenhere)
    goto out;

  wVersionRequested = MAKEWORD (2, 0);

  err = WSAStartup (wVersionRequested, &wsaData);
  if (err != 0)
    {
      _dbus_assert_not_reached ("Could not initialize WinSock");
      _dbus_abort ();
    }

  /* Confirm that the WinSock DLL supports 2.0.  Note that if the DLL
   * supports versions greater than 2.0 in addition to 2.0, it will
   * still return 2.0 in wVersion since that is the version we
   * requested.
   */
  if (LOBYTE (wsaData.wVersion) != 2 ||
      HIBYTE (wsaData.wVersion) != 0)
    {
      _dbus_assert_not_reached ("No usable WinSock found");
      _dbus_abort ();
    }

  beenhere = TRUE;

out:
  _DBUS_UNLOCK (sysdeps);
  return TRUE;
}









/************************************************************************
 
 UTF / string code
 
 ************************************************************************/

/**
 * Measure the message length without terminating nul 
 */
int _dbus_printf_string_upper_bound (const char *format,
                                     va_list args)
{
  /* MSVCRT's vsnprintf semantics are a bit different */
  char buf[1024];
  int bufsize;
  int len;
  va_list args_copy;

  bufsize = sizeof (buf);
  DBUS_VA_COPY (args_copy, args);
  len = _vsnprintf (buf, bufsize - 1, format, args_copy);
  va_end (args_copy);

  while (len == -1) /* try again */
    {
      char *p;

      bufsize *= 2;

      p = malloc (bufsize);

      if (p == NULL)
        return -1;

      DBUS_VA_COPY (args_copy, args);
      len = _vsnprintf (p, bufsize - 1, format, args_copy);
      va_end (args_copy);
      free (p);
    }

  return len;
}


/**
 * Returns the UTF-16 form of a UTF-8 string. The result should be
 * freed with dbus_free() when no longer needed.
 *
 * @param str the UTF-8 string
 * @param error return location for error code
 */
wchar_t *
_dbus_win_utf8_to_utf16 (const char *str,
                         DBusError  *error)
{
  DBusString s;
  int n;
  wchar_t *retval;

  _dbus_string_init_const (&s, str);

  if (!_dbus_string_validate_utf8 (&s, 0, _dbus_string_get_length (&s)))
    {
      dbus_set_error_const (error, DBUS_ERROR_FAILED, "Invalid UTF-8");
      return NULL;
    }

  n = MultiByteToWideChar (CP_UTF8, 0, str, -1, NULL, 0);

  if (n == 0)
    {
      _dbus_win_set_error_from_win_error (error, GetLastError ());
      return NULL;
    }

  retval = dbus_new (wchar_t, n);

  if (!retval)
    {
      _DBUS_SET_OOM (error);
      return NULL;
    }

  if (MultiByteToWideChar (CP_UTF8, 0, str, -1, retval, n) != n)
    {
      dbus_free (retval);
      dbus_set_error_const (error, DBUS_ERROR_FAILED, "MultiByteToWideChar inconsistency");
      return NULL;
    }

  return retval;
}

/**
 * Returns the UTF-8 form of a UTF-16 string. The result should be
 * freed with dbus_free() when no longer needed.
 *
 * @param str the UTF-16 string
 * @param error return location for error code
 */
char *
_dbus_win_utf16_to_utf8 (const wchar_t *str,
                         DBusError     *error)
{
  int n;
  char *retval;

  n = WideCharToMultiByte (CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);

  if (n == 0)
    {
      _dbus_win_set_error_from_win_error (error, GetLastError ());
      return NULL;
    }

  retval = dbus_malloc (n);

  if (!retval)
    {
      _DBUS_SET_OOM (error);
      return NULL;
    }

  if (WideCharToMultiByte (CP_UTF8, 0, str, -1, retval, n, NULL, NULL) != n)
    {
      dbus_free (retval);
      dbus_set_error_const (error, DBUS_ERROR_FAILED, "WideCharToMultiByte inconsistency");
      return NULL;
    }

  return retval;
}






/************************************************************************
 
 
 ************************************************************************/

dbus_bool_t
_dbus_win_account_to_sid (const wchar_t *waccount,
                          void      	 **ppsid,
                          DBusError 	  *error)
{
  dbus_bool_t retval = FALSE;
  DWORD sid_length, wdomain_length;
  SID_NAME_USE use;
  wchar_t *wdomain;

  *ppsid = NULL;

  sid_length = 0;
  wdomain_length = 0;
  if (!LookupAccountNameW (NULL, waccount, NULL, &sid_length,
                           NULL, &wdomain_length, &use) &&
      GetLastError () != ERROR_INSUFFICIENT_BUFFER)
    {
      _dbus_win_set_error_from_win_error (error, GetLastError ());
      return FALSE;
    }

  *ppsid = dbus_malloc (sid_length);
  if (!*ppsid)
    {
      _DBUS_SET_OOM (error);
      return FALSE;
    }

  wdomain = dbus_new (wchar_t, wdomain_length);
  if (!wdomain)
    {
      _DBUS_SET_OOM (error);
      goto out1;
    }

  if (!LookupAccountNameW (NULL, waccount, (PSID) *ppsid, &sid_length,
                           wdomain, &wdomain_length, &use))
    {
      _dbus_win_set_error_from_win_error (error, GetLastError ());
      goto out2;
    }

  if (!IsValidSid ((PSID) *ppsid))
    {
      dbus_set_error_const (error, DBUS_ERROR_FAILED, "Invalid SID");
      goto out2;
    }

  retval = TRUE;

out2:
  dbus_free (wdomain);
out1:
  if (!retval)
    {
      dbus_free (*ppsid);
      *ppsid = NULL;
    }

  return retval;
}

/** @} end of sysdeps-win */


/**
 * The only reason this is separate from _dbus_getpid() is to allow it
 * on Windows for logging but not for other purposes.
 * 
 * @returns process ID to put in log messages
 */
unsigned long
_dbus_pid_for_log (void)
{
  return _dbus_getpid ();
}

#ifndef DBUS_WINCE

static BOOL is_winxp_sp3_or_lower()
{
   OSVERSIONINFOEX osvi;
   DWORDLONG dwlConditionMask = 0;
   int op=VER_LESS_EQUAL;

   // Initialize the OSVERSIONINFOEX structure.

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   osvi.dwMajorVersion = 5;
   osvi.dwMinorVersion = 1;
   osvi.wServicePackMajor = 3;
   osvi.wServicePackMinor = 0;

   // Initialize the condition mask.

   VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, op );
   VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, op );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, op );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMINOR, op );

   // Perform the test.

   return VerifyVersionInfo(
      &osvi,
      VER_MAJORVERSION | VER_MINORVERSION |
      VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
      dwlConditionMask);
}

/** Gets our SID
 * @param sid points to sid buffer, need to be freed with LocalFree()
 * @param process_id the process id for which the sid should be returned
 * @returns process sid
 */
static dbus_bool_t
_dbus_getsid(char **sid, dbus_pid_t process_id)
{
  HANDLE process_token = INVALID_HANDLE_VALUE;
  TOKEN_USER *token_user = NULL;
  DWORD n;
  PSID psid;
  int retval = FALSE;

  HANDLE process_handle = OpenProcess(is_winxp_sp3_or_lower() ? PROCESS_QUERY_INFORMATION : PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process_id);

  if (!OpenProcessToken (process_handle, TOKEN_QUERY, &process_token))
    {
      _dbus_win_warn_win_error ("OpenProcessToken failed", GetLastError ());
      goto failed;
    }
  if ((!GetTokenInformation (process_token, TokenUser, NULL, 0, &n)
            && GetLastError () != ERROR_INSUFFICIENT_BUFFER)
           || (token_user = alloca (n)) == NULL
           || !GetTokenInformation (process_token, TokenUser, token_user, n, &n))
    {
      _dbus_win_warn_win_error ("GetTokenInformation failed", GetLastError ());
      goto failed;
    }
  psid = token_user->User.Sid;
  if (!IsValidSid (psid))
    {
      _dbus_verbose("%s invalid sid\n",__FUNCTION__);
      goto failed;
    }
  if (!ConvertSidToStringSidA (psid, sid))
    {
      _dbus_verbose("%s invalid sid\n",__FUNCTION__);
      goto failed;
    }
//okay:
  retval = TRUE;

failed:
  CloseHandle (process_handle);
  if (process_token != INVALID_HANDLE_VALUE)
    CloseHandle (process_token);

  _dbus_verbose("_dbus_getsid() got '%s' and returns %d\n", *sid, retval);
  return retval;
}
#endif

/************************************************************************
 
 pipes
 
 ************************************************************************/

/**
 * Creates a full-duplex pipe (as in socketpair()).
 * Sets both ends of the pipe nonblocking.
 *
 * @param fd1 return location for one end
 * @param fd2 return location for the other end
 * @param blocking #TRUE if pipe should be blocking
 * @param error error return
 * @returns #FALSE on failure (if error is set)
 */
dbus_bool_t
_dbus_full_duplex_pipe (int        *fd1,
                        int        *fd2,
                        dbus_bool_t blocking,
                        DBusError  *error)
{
  SOCKET temp, socket1 = -1, socket2 = -1;
  struct sockaddr_in saddr;
  int len;
  u_long arg;

  if (!_dbus_win_startup_winsock ())
    {
      _DBUS_SET_OOM (error);
      return FALSE;
    }

  temp = socket (AF_INET, SOCK_STREAM, 0);
  if (temp == INVALID_SOCKET)
    {
      DBUS_SOCKET_SET_ERRNO ();
      goto out0;
    }

  _DBUS_ZERO (saddr);
  saddr.sin_family = AF_INET;
  saddr.sin_port = 0;
  saddr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

  if (bind (temp, (struct sockaddr *)&saddr, sizeof (saddr)) == SOCKET_ERROR)
    {
      DBUS_SOCKET_SET_ERRNO ();
      goto out0;
    }

  if (listen (temp, 1) == SOCKET_ERROR)
    {
      DBUS_SOCKET_SET_ERRNO ();
      goto out0;
    }

  len = sizeof (saddr);
  if (getsockname (temp, (struct sockaddr *)&saddr, &len) == SOCKET_ERROR)
    {
      DBUS_SOCKET_SET_ERRNO ();
      goto out0;
    }

  socket1 = socket (AF_INET, SOCK_STREAM, 0);
  if (socket1 == INVALID_SOCKET)
    {
      DBUS_SOCKET_SET_ERRNO ();
      goto out0;
    }

  if (connect (socket1, (struct sockaddr  *)&saddr, len) == SOCKET_ERROR)
    {
      DBUS_SOCKET_SET_ERRNO ();
      goto out1;
    }

  socket2 = accept (temp, (struct sockaddr *) &saddr, &len);
  if (socket2 == INVALID_SOCKET)
    {
      DBUS_SOCKET_SET_ERRNO ();
      goto out1;
    }

  if (!blocking)
    {
      arg = 1;
      if (ioctlsocket (socket1, FIONBIO, &arg) == SOCKET_ERROR)
        {
          DBUS_SOCKET_SET_ERRNO ();
          goto out2;
        }

      arg = 1;
      if (ioctlsocket (socket2, FIONBIO, &arg) == SOCKET_ERROR)
        {
          DBUS_SOCKET_SET_ERRNO ();
          goto out2;
        }
    }

  *fd1 = socket1;
  *fd2 = socket2;

  _dbus_verbose ("full-duplex pipe %d:%d <-> %d:%d\n",
                 *fd1, socket1, *fd2, socket2);

  closesocket (temp);

  return TRUE;

out2:
  closesocket (socket2);
out1:
  closesocket (socket1);
out0:
  closesocket (temp);

  dbus_set_error (error, _dbus_error_from_errno (errno),
                  "Could not setup socket pair: %s",
                  _dbus_strerror_from_errno ());

  return FALSE;
}

/**
 * Wrapper for poll().
 *
 * @param fds the file descriptors to poll
 * @param n_fds number of descriptors in the array
 * @param timeout_milliseconds timeout or -1 for infinite
 * @returns numbers of fds with revents, or <0 on error
 */
int
_dbus_poll (DBusPollFD *fds,
            int         n_fds,
            int         timeout_milliseconds)
{
#define USE_CHRIS_IMPL 0

#if USE_CHRIS_IMPL

#define DBUS_POLL_CHAR_BUFFER_SIZE 2000
  char msg[DBUS_POLL_CHAR_BUFFER_SIZE];
  char *msgp;

  int ret = 0;
  int i;
  struct timeval tv;
  int ready;

#define DBUS_STACK_WSAEVENTS 256
  WSAEVENT eventsOnStack[DBUS_STACK_WSAEVENTS];
  WSAEVENT *pEvents = NULL;
  if (n_fds > DBUS_STACK_WSAEVENTS)
    pEvents = calloc(sizeof(WSAEVENT), n_fds);
  else
    pEvents = eventsOnStack;


#ifdef DBUS_ENABLE_VERBOSE_MODE
  msgp = msg;
  msgp += sprintf (msgp, "WSAEventSelect: to=%d\n\t", timeout_milliseconds);
  for (i = 0; i < n_fds; i++)
    {
      DBusPollFD *fdp = &fds[i];


      if (fdp->events & _DBUS_POLLIN)
        msgp += sprintf (msgp, "R:%d ", fdp->fd);

      if (fdp->events & _DBUS_POLLOUT)
        msgp += sprintf (msgp, "W:%d ", fdp->fd);

      msgp += sprintf (msgp, "E:%d\n\t", fdp->fd);

      // FIXME: more robust code for long  msg
      //        create on heap when msg[] becomes too small
      if (msgp >= msg + DBUS_POLL_CHAR_BUFFER_SIZE)
        {
          _dbus_assert_not_reached ("buffer overflow in _dbus_poll");
        }
    }

  msgp += sprintf (msgp, "\n");
  _dbus_verbose ("%s",msg);
#endif
  for (i = 0; i < n_fds; i++)
    {
      DBusPollFD *fdp = &fds[i];
      WSAEVENT ev;
      long lNetworkEvents = FD_OOB;

      ev = WSACreateEvent();

      if (fdp->events & _DBUS_POLLIN)
        lNetworkEvents |= FD_READ | FD_ACCEPT | FD_CLOSE;

      if (fdp->events & _DBUS_POLLOUT)
        lNetworkEvents |= FD_WRITE | FD_CONNECT;

      WSAEventSelect(fdp->fd, ev, lNetworkEvents);

      pEvents[i] = ev;
    }


  ready = WSAWaitForMultipleEvents (n_fds, pEvents, FALSE, timeout_milliseconds, FALSE);

  if (DBUS_SOCKET_API_RETURNS_ERROR (ready))
    {
      DBUS_SOCKET_SET_ERRNO ();
      if (errno != WSAEWOULDBLOCK)
        _dbus_verbose ("WSAWaitForMultipleEvents: failed: %s\n", _dbus_strerror_from_errno ());
      ret = -1;
    }
  else if (ready == WSA_WAIT_TIMEOUT)
    {
      _dbus_verbose ("WSAWaitForMultipleEvents: WSA_WAIT_TIMEOUT\n");
      ret = 0;
    }
  else if (ready >= WSA_WAIT_EVENT_0 && ready < (int)(WSA_WAIT_EVENT_0 + n_fds))
    {
      msgp = msg;
      msgp += sprintf (msgp, "WSAWaitForMultipleEvents: =%d\n\t", ready);

      for (i = 0; i < n_fds; i++)
        {
          DBusPollFD *fdp = &fds[i];
          WSANETWORKEVENTS ne;

          fdp->revents = 0;

          WSAEnumNetworkEvents(fdp->fd, pEvents[i], &ne);

          if (ne.lNetworkEvents & (FD_READ | FD_ACCEPT | FD_CLOSE))
            fdp->revents |= _DBUS_POLLIN;

          if (ne.lNetworkEvents & (FD_WRITE | FD_CONNECT))
            fdp->revents |= _DBUS_POLLOUT;

          if (ne.lNetworkEvents & (FD_OOB))
            fdp->revents |= _DBUS_POLLERR;

          if (ne.lNetworkEvents & (FD_READ | FD_ACCEPT | FD_CLOSE))
              msgp += sprintf (msgp, "R:%d ", fdp->fd);

          if (ne.lNetworkEvents & (FD_WRITE | FD_CONNECT))
              msgp += sprintf (msgp, "W:%d ", fdp->fd);

          if (ne.lNetworkEvents & (FD_OOB))
              msgp += sprintf (msgp, "E:%d ", fdp->fd);

          msgp += sprintf (msgp, "lNetworkEvents:%d ", ne.lNetworkEvents);

          if(ne.lNetworkEvents)
            ret++;

          WSAEventSelect(fdp->fd, pEvents[i], 0);
        }

      msgp += sprintf (msgp, "\n");
      _dbus_verbose ("%s",msg);
    }
  else
    {
      _dbus_verbose ("WSAWaitForMultipleEvents: failed for unknown reason!");
      ret = -1;
    }

  for(i = 0; i < n_fds; i++)
    {
      WSACloseEvent(pEvents[i]);
    }

  if (n_fds > DBUS_STACK_WSAEVENTS)
    free(pEvents);

  return ret;

#else   /* USE_CHRIS_IMPL */

#define DBUS_POLL_CHAR_BUFFER_SIZE 2000
  char msg[DBUS_POLL_CHAR_BUFFER_SIZE];
  char *msgp;

  fd_set read_set, write_set, err_set;
  int max_fd = 0;
  int i;
  struct timeval tv;
  int ready;

  FD_ZERO (&read_set);
  FD_ZERO (&write_set);
  FD_ZERO (&err_set);


#ifdef DBUS_ENABLE_VERBOSE_MODE
  msgp = msg;
  msgp += sprintf (msgp, "select: to=%d\n\t", timeout_milliseconds);
  for (i = 0; i < n_fds; i++)
    {
      DBusPollFD *fdp = &fds[i];


      if (fdp->events & _DBUS_POLLIN)
        msgp += sprintf (msgp, "R:%d ", fdp->fd);

      if (fdp->events & _DBUS_POLLOUT)
        msgp += sprintf (msgp, "W:%d ", fdp->fd);

      msgp += sprintf (msgp, "E:%d\n\t", fdp->fd);

      // FIXME: more robust code for long  msg
      //        create on heap when msg[] becomes too small
      if (msgp >= msg + DBUS_POLL_CHAR_BUFFER_SIZE)
        {
          _dbus_assert_not_reached ("buffer overflow in _dbus_poll");
        }
    }

  msgp += sprintf (msgp, "\n");
  _dbus_verbose ("%s",msg);
#endif
  for (i = 0; i < n_fds; i++)
    {
      DBusPollFD *fdp = &fds[i]; 

      if (fdp->events & _DBUS_POLLIN)
        FD_SET (fdp->fd, &read_set);

      if (fdp->events & _DBUS_POLLOUT)
        FD_SET (fdp->fd, &write_set);

      FD_SET (fdp->fd, &err_set);

      max_fd = MAX (max_fd, fdp->fd);
    }

  // Avoid random lockups with send(), for lack of a better solution so far
  tv.tv_sec = timeout_milliseconds < 0 ? 1 : timeout_milliseconds / 1000;
  tv.tv_usec = timeout_milliseconds < 0 ? 0 : (timeout_milliseconds % 1000) * 1000;

  ready = select (max_fd + 1, &read_set, &write_set, &err_set, &tv);

  if (DBUS_SOCKET_API_RETURNS_ERROR (ready))
    {
      DBUS_SOCKET_SET_ERRNO ();
      if (errno != WSAEWOULDBLOCK)
        _dbus_verbose ("select: failed: %s\n", _dbus_strerror_from_errno ());
    }
  else if (ready == 0)
    _dbus_verbose ("select: = 0\n");
  else
    if (ready > 0)
      {
#ifdef DBUS_ENABLE_VERBOSE_MODE
        msgp = msg;
        msgp += sprintf (msgp, "select: = %d:\n\t", ready);

        for (i = 0; i < n_fds; i++)
          {
            DBusPollFD *fdp = &fds[i];

            if (FD_ISSET (fdp->fd, &read_set))
              msgp += sprintf (msgp, "R:%d ", fdp->fd);

            if (FD_ISSET (fdp->fd, &write_set))
              msgp += sprintf (msgp, "W:%d ", fdp->fd);

            if (FD_ISSET (fdp->fd, &err_set))
              msgp += sprintf (msgp, "E:%d\n\t", fdp->fd);
          }
        msgp += sprintf (msgp, "\n");
        _dbus_verbose ("%s",msg);
#endif

        for (i = 0; i < n_fds; i++)
          {
            DBusPollFD *fdp = &fds[i];

            fdp->revents = 0;

            if (FD_ISSET (fdp->fd, &read_set))
              fdp->revents |= _DBUS_POLLIN;

            if (FD_ISSET (fdp->fd, &write_set))
              fdp->revents |= _DBUS_POLLOUT;

            if (FD_ISSET (fdp->fd, &err_set))
              fdp->revents |= _DBUS_POLLERR;
          }
      }
  return ready;
#endif  /* USE_CHRIS_IMPL */
}




/******************************************************************************
 
Original CVS version of dbus-sysdeps.c
 
******************************************************************************/
/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-sysdeps.c Wrappers around system/libc features (internal to D-Bus implementation)
 * 
 * Copyright (C) 2002, 2003  Red Hat, Inc.
 * Copyright (C) 2003 CodeFactory AB
 * Copyright (C) 2005 Novell, Inc.
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


/**
 * Exit the process, returning the given value.
 *
 * @param code the exit code
 */
void
_dbus_exit (int code)
{
  _exit (code);
}

/**
 * Creates a socket and connects to a socket at the given host 
 * and port. The connection fd is returned, and is set up as
 * nonblocking.
 *
 * @param host the host name to connect to
 * @param port the port to connect to
 * @param family the address family to listen on, NULL for all
 * @param error return location for error code
 * @returns connection file descriptor or -1 on error
 */
int
_dbus_connect_tcp_socket (const char     *host,
                          const char     *port,
                          const char     *family,
                          DBusError      *error)
{
  return _dbus_connect_tcp_socket_with_nonce (host, port, family, (const char*)NULL, error);
}

int
_dbus_connect_tcp_socket_with_nonce (const char     *host,
                                     const char     *port,
                                     const char     *family,
                                     const char     *noncefile,
                                     DBusError      *error)
{
  int fd = -1, res;
  struct addrinfo hints;
  struct addrinfo *ai, *tmp;

  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  if (!_dbus_win_startup_winsock ())
    {
      _DBUS_SET_OOM (error);
      return -1;
    }

  _DBUS_ZERO (hints);

  if (!family)
    hints.ai_family = AF_UNSPEC;
  else if (!strcmp(family, "ipv4"))
    hints.ai_family = AF_INET;
  else if (!strcmp(family, "ipv6"))
    hints.ai_family = AF_INET6;
  else
    {
      dbus_set_error (error,
                      DBUS_ERROR_INVALID_ARGS,
                      "Unknown address family %s", family);
      return -1;
    }
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_socktype = SOCK_STREAM;
#ifdef AI_ADDRCONFIG
  hints.ai_flags = AI_ADDRCONFIG;
#else
  hints.ai_flags = 0;
#endif

  if ((res = getaddrinfo(host, port, &hints, &ai)) != 0 || !ai)
    {
      dbus_set_error (error,
                      _dbus_error_from_errno (res),
                      "Failed to lookup host/port: \"%s:%s\": %s (%d)",
                      host, port, _dbus_strerror(res), res);
      return -1;
    }

  tmp = ai;
  while (tmp)
    {
      if ((fd = socket (tmp->ai_family, SOCK_STREAM, 0)) == INVALID_SOCKET)
        {
          DBUS_SOCKET_SET_ERRNO ();
          dbus_set_error (error,
                          _dbus_error_from_errno (errno),
                          "Failed to open socket: %s",
                          _dbus_strerror_from_errno ());
          freeaddrinfo(ai);
          return -1;
        }
      _DBUS_ASSERT_ERROR_IS_CLEAR(error);

      if (connect (fd, (struct sockaddr*) tmp->ai_addr, tmp->ai_addrlen) == SOCKET_ERROR)
        {
          DBUS_SOCKET_SET_ERRNO ();
          closesocket(fd);
          fd = -1;
          tmp = tmp->ai_next;
          continue;
        }

      break;
    }
  freeaddrinfo(ai);

  if (fd == -1)
    {
      dbus_set_error (error,
                      _dbus_error_from_errno (errno),
                      "Failed to connect to socket \"%s:%s\" %s",
                      host, port, _dbus_strerror_from_errno ());
      return -1;
    }

  if (noncefile != NULL)
    {
      DBusString noncefileStr;
      dbus_bool_t ret;
      if (!_dbus_string_init (&noncefileStr) ||
          !_dbus_string_append(&noncefileStr, noncefile))
        {
          closesocket (fd);
          dbus_set_error (error, DBUS_ERROR_NO_MEMORY, NULL);
          return -1;
        }

      ret = _dbus_send_nonce (fd, &noncefileStr, error);

      _dbus_string_free (&noncefileStr);

      if (!ret)
        {
          closesocket (fd);
          return -1;
        }
    }

  _dbus_fd_set_close_on_exec (fd);

  if (!_dbus_set_fd_nonblocking (fd, error))
    {
      closesocket (fd);
      return -1;
    }

  return fd;
}

/**
 * Creates a socket and binds it to the given path, then listens on
 * the socket. The socket is set to be nonblocking.  In case of port=0
 * a random free port is used and returned in the port parameter.
 * If inaddr_any is specified, the hostname is ignored.
 *
 * @param host the host name to listen on
 * @param port the port to listen on, if zero a free port will be used 
 * @param family the address family to listen on, NULL for all
 * @param retport string to return the actual port listened on
 * @param fds_p location to store returned file descriptors
 * @param error return location for errors
 * @returns the number of listening file descriptors or -1 on error
 */

int
_dbus_listen_tcp_socket (const char     *host,
                         const char     *port,
                         const char     *family,
                         DBusString     *retport,
                         int           **fds_p,
                         DBusError      *error)
{
  int nlisten_fd = 0, *listen_fd = NULL, res, i, port_num = -1;
  struct addrinfo hints;
  struct addrinfo *ai, *tmp;

  // On Vista, sockaddr_gen must be a sockaddr_in6, and not a sockaddr_in6_old
  //That's required for family == IPv6(which is the default on Vista if family is not given)
  //So we use our own union instead of sockaddr_gen:

  typedef union {
	struct sockaddr Address;
	struct sockaddr_in AddressIn;
	struct sockaddr_in6 AddressIn6;
  } mysockaddr_gen;

  *fds_p = NULL;
  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  if (!_dbus_win_startup_winsock ())
    {
      _DBUS_SET_OOM (error);
      return -1;
    }

  _DBUS_ZERO (hints);

  if (!family)
    hints.ai_family = AF_UNSPEC;
  else if (!strcmp(family, "ipv4"))
    hints.ai_family = AF_INET;
  else if (!strcmp(family, "ipv6"))
    hints.ai_family = AF_INET6;
  else
    {
      dbus_set_error (error,
                      DBUS_ERROR_INVALID_ARGS,
                      "Unknown address family %s", family);
      return -1;
    }

  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_socktype = SOCK_STREAM;
#ifdef AI_ADDRCONFIG
  hints.ai_flags = AI_ADDRCONFIG | AI_PASSIVE;
#else
  hints.ai_flags = AI_PASSIVE;
#endif

 redo_lookup_with_port:
  if ((res = getaddrinfo(host, port, &hints, &ai)) != 0 || !ai)
    {
      dbus_set_error (error,
                      _dbus_error_from_errno (res),
                      "Failed to lookup host/port: \"%s:%s\": %s (%d)",
                      host ? host : "*", port, _dbus_strerror(res), res);
      return -1;
    }

  tmp = ai;
  while (tmp)
    {
      int fd = -1, *newlisten_fd;
      if ((fd = socket (tmp->ai_family, SOCK_STREAM, 0)) == INVALID_SOCKET)
        {
          DBUS_SOCKET_SET_ERRNO ();
          dbus_set_error (error,
                          _dbus_error_from_errno (errno),
                         "Failed to open socket: %s",
                         _dbus_strerror_from_errno ());
          goto failed;
        }
      _DBUS_ASSERT_ERROR_IS_CLEAR(error);

      if (bind (fd, (struct sockaddr*) tmp->ai_addr, tmp->ai_addrlen) == SOCKET_ERROR)
        {
          DBUS_SOCKET_SET_ERRNO ();
          dbus_set_error (error, _dbus_error_from_errno (errno),
                          "Failed to bind socket \"%s:%s\": %s",
                          host ? host : "*", port, _dbus_strerror_from_errno ());
          closesocket (fd);
          goto failed;
    }

      if (listen (fd, 30 /* backlog */) == SOCKET_ERROR)
        {
          DBUS_SOCKET_SET_ERRNO ();
          dbus_set_error (error, _dbus_error_from_errno (errno),
                          "Failed to listen on socket \"%s:%s\": %s",
                          host ? host : "*", port, _dbus_strerror_from_errno ());
          closesocket (fd);
          goto failed;
        }

      newlisten_fd = dbus_realloc(listen_fd, sizeof(int)*(nlisten_fd+1));
      if (!newlisten_fd)
        {
          closesocket (fd);
          dbus_set_error (error, DBUS_ERROR_NO_MEMORY,
                          "Failed to allocate file handle array");
          goto failed;
        }
      listen_fd = newlisten_fd;
      listen_fd[nlisten_fd] = fd;
      nlisten_fd++;

      if (!_dbus_string_get_length(retport))
        {
          /* If the user didn't specify a port, or used 0, then
             the kernel chooses a port. After the first address
             is bound to, we need to force all remaining addresses
             to use the same port */
          if (!port || !strcmp(port, "0"))
            {
              mysockaddr_gen addr;
              socklen_t addrlen = sizeof(addr);
              char portbuf[10];

              if (getsockname(fd, &addr.Address, &addrlen) == SOCKET_ERROR)
                {
                  DBUS_SOCKET_SET_ERRNO ();
                  dbus_set_error (error, _dbus_error_from_errno (errno),
                                  "Failed to resolve port \"%s:%s\": %s",
                                  host ? host : "*", port, _dbus_strerror_from_errno());
                  goto failed;
                }
              snprintf( portbuf, sizeof( portbuf ) - 1, "%d", addr.AddressIn.sin_port );
              if (!_dbus_string_append(retport, portbuf))
                {
                  dbus_set_error (error, DBUS_ERROR_NO_MEMORY, NULL);
                  goto failed;
                }

              /* Release current address list & redo lookup */
              port = _dbus_string_get_const_data(retport);
              freeaddrinfo(ai);
              goto redo_lookup_with_port;
            }
          else
            {
              if (!_dbus_string_append(retport, port))
                {
                    dbus_set_error (error, DBUS_ERROR_NO_MEMORY, NULL);
                    goto failed;
                }
            }
        }
  
      tmp = tmp->ai_next;
    }
  freeaddrinfo(ai);
  ai = NULL;

  if (!nlisten_fd)
    {
      _dbus_win_set_errno (WSAEADDRINUSE);
      dbus_set_error (error, _dbus_error_from_errno (errno),
                      "Failed to bind socket \"%s:%s\": %s",
                      host ? host : "*", port, _dbus_strerror_from_errno ());
      return -1;
    }

  sscanf(_dbus_string_get_const_data(retport), "%d", &port_num);

  for (i = 0 ; i < nlisten_fd ; i++)
    {
      _dbus_fd_set_close_on_exec (listen_fd[i]);
      if (!_dbus_set_fd_nonblocking (listen_fd[i], error))
        {
          goto failed;
        }
    }

  *fds_p = listen_fd;

  return nlisten_fd;

 failed:
  if (ai)
    freeaddrinfo(ai);
  for (i = 0 ; i < nlisten_fd ; i++)
    closesocket (listen_fd[i]);
  dbus_free(listen_fd);
  return -1;
}


/**
 * Accepts a connection on a listening socket.
 * Handles EINTR for you.
 *
 * @param listen_fd the listen file descriptor
 * @returns the connection fd of the client, or -1 on error
 */
int
_dbus_accept  (int listen_fd)
{
  int client_fd;

 retry:
  client_fd = accept (listen_fd, NULL, NULL);

  if (DBUS_SOCKET_IS_INVALID (client_fd))
    {
      DBUS_SOCKET_SET_ERRNO ();
      if (errno == EINTR)
        goto retry;
    }

  _dbus_verbose ("client fd %d accepted\n", client_fd);
  
  return client_fd;
}




dbus_bool_t
_dbus_send_credentials_socket (int            handle,
                        DBusError      *error)
{
/* FIXME: for the session bus credentials shouldn't matter (?), but
 * for the system bus they are presumably essential. A rough outline
 * of a way to implement the credential transfer would be this:
 *
 * client waits to *read* a byte.
 *
 * server creates a named pipe with a random name, sends a byte
 * contining its length, and its name.
 *
 * client reads the name, connects to it (using Win32 API).
 *
 * server waits for connection to the named pipe, then calls
 * ImpersonateNamedPipeClient(), notes its now-current credentials,
 * calls RevertToSelf(), closes its handles to the named pipe, and
 * is done. (Maybe there is some other way to get the SID of a named
 * pipe client without having to use impersonation?)
 *
 * client closes its handles and is done.
 * 
 * Ralf: Why not sending credentials over the given this connection ?
 * Using named pipes makes it impossible to be connected from a unix client.  
 *
 */
  int bytes_written;
  DBusString buf; 

  _dbus_string_init_const_len (&buf, "\0", 1);
again:
  bytes_written = _dbus_write_socket (handle, &buf, 0, 1 );

  if (bytes_written < 0 && errno == EINTR)
    goto again;

  if (bytes_written < 0)
    {
      dbus_set_error (error, _dbus_error_from_errno (errno),
                      "Failed to write credentials byte: %s",
                     _dbus_strerror_from_errno ());
      return FALSE;
    }
  else if (bytes_written == 0)
    {
      dbus_set_error (error, DBUS_ERROR_IO_ERROR,
                      "wrote zero bytes writing credentials byte");
      return FALSE;
    }
  else
    {
      _dbus_assert (bytes_written == 1);
      _dbus_verbose ("wrote 1 zero byte, credential sending isn't implemented yet\n");
      return TRUE;
    }
  return TRUE;
}

/**
 * Reads a single byte which must be nul (an error occurs otherwise),
 * and reads unix credentials if available. Fills in pid/uid/gid with
 * -1 if no credentials are available. Return value indicates whether
 * a byte was read, not whether we got valid credentials. On some
 * systems, such as Linux, reading/writing the byte isn't actually
 * required, but we do it anyway just to avoid multiple codepaths.
 *
 * Fails if no byte is available, so you must select() first.
 *
 * The point of the byte is that on some systems we have to
 * use sendmsg()/recvmsg() to transmit credentials.
 *
 * @param handle the client file descriptor
 * @param credentials struct to fill with credentials of client
 * @param error location to store error code
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_read_credentials_socket  (int              handle,
                                DBusCredentials *credentials,
                                DBusError       *error)
{
  int bytes_read = 0;
  DBusString buf;

  char *sid = NULL;
  dbus_pid_t pid;
  int retval = FALSE;

  // could fail due too OOM
  if (_dbus_string_init (&buf))
    {
      bytes_read = _dbus_read_socket (handle, &buf, 1 );

      if (bytes_read > 0) 
        _dbus_verbose ("got one zero byte from server\n");

      _dbus_string_free (&buf);
    }

  pid = _dbus_get_peer_pid_from_tcp_handle (handle);
  if (pid == 0)
    return TRUE;

  _dbus_credentials_add_pid (credentials, pid);

  if (_dbus_getsid (&sid, pid))
    {
      if (!_dbus_credentials_add_windows_sid (credentials, sid))
        goto out;
    }

  retval = TRUE;

out:
  if (sid)
    LocalFree (sid);

  return retval;
}

/**
* Checks to make sure the given directory is 
* private to the user 
*
* @param dir the name of the directory
* @param error error return
* @returns #FALSE on failure
**/
dbus_bool_t
_dbus_check_dir_is_private_to_user (DBusString *dir, DBusError *error)
{
  /* TODO */
  _DBUS_ASSERT_ERROR_IS_CLEAR (error);
  return TRUE;
}


/**
 * Appends the given filename to the given directory.
 *
 * @todo it might be cute to collapse multiple '/' such as "foo//"
 * concat "//bar"
 *
 * @param dir the directory name
 * @param next_component the filename
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_concat_dir_and_file (DBusString       *dir,
                           const DBusString *next_component)
{
  dbus_bool_t dir_ends_in_slash;
  dbus_bool_t file_starts_with_slash;

  if (_dbus_string_get_length (dir) == 0 ||
      _dbus_string_get_length (next_component) == 0)
    return TRUE;

  dir_ends_in_slash =
    ('/' == _dbus_string_get_byte (dir, _dbus_string_get_length (dir) - 1) ||
     '\\' == _dbus_string_get_byte (dir, _dbus_string_get_length (dir) - 1));

  file_starts_with_slash =
    ('/' == _dbus_string_get_byte (next_component, 0) ||
     '\\' == _dbus_string_get_byte (next_component, 0));

  if (dir_ends_in_slash && file_starts_with_slash)
    {
      _dbus_string_shorten (dir, 1);
    }
  else if (!(dir_ends_in_slash || file_starts_with_slash))
    {
      if (!_dbus_string_append_byte (dir, '\\'))
        return FALSE;
    }

  return _dbus_string_copy (next_component, 0, dir,
                            _dbus_string_get_length (dir));
}

/*---------------- DBusCredentials ----------------------------------*/

/**
 * Adds the credentials corresponding to the given username.
 *
 * @param credentials credentials to fill in 
 * @param username the username
 * @returns #TRUE if the username existed and we got some credentials
 */
dbus_bool_t
_dbus_credentials_add_from_user (DBusCredentials  *credentials,
                                     const DBusString *username)
{
  return _dbus_credentials_add_windows_sid (credentials,
                    _dbus_string_get_const_data(username));
}

/**
 * Adds the credentials of the current process to the
 * passed-in credentials object.
 *
 * @param credentials credentials to add to
 * @returns #FALSE if no memory; does not properly roll back on failure, so only some credentials may have been added
 */

dbus_bool_t
_dbus_credentials_add_from_current_process (DBusCredentials *credentials)
{
  dbus_bool_t retval = FALSE;
  char *sid = NULL;

  if (!_dbus_getsid(&sid, _dbus_getpid()))
    goto failed;

  if (!_dbus_credentials_add_pid (credentials, _dbus_getpid()))
    goto failed;

  if (!_dbus_credentials_add_windows_sid (credentials,sid))
    goto failed;

  retval = TRUE;
  goto end;
failed:
  retval = FALSE;
end:
  if (sid)
    LocalFree(sid);

  return retval;
}

/**
 * Append to the string the identity we would like to have when we
 * authenticate, on UNIX this is the current process UID and on
 * Windows something else, probably a Windows SID string.  No escaping
 * is required, that is done in dbus-auth.c. The username here
 * need not be anything human-readable, it can be the machine-readable
 * form i.e. a user id.
 * 
 * @param str the string to append to
 * @returns #FALSE on no memory
 * @todo to which class belongs this 
 */
dbus_bool_t
_dbus_append_user_from_current_process (DBusString *str)
{
  dbus_bool_t retval = FALSE;
  char *sid = NULL;

  if (!_dbus_getsid(&sid, _dbus_getpid()))
    return FALSE;

  retval = _dbus_string_append (str,sid);

  LocalFree(sid);
  return retval;
}

/**
 * Gets our process ID
 * @returns process ID
 */
dbus_pid_t
_dbus_getpid (void)
{
  return GetCurrentProcessId ();
}

/** nanoseconds in a second */
#define NANOSECONDS_PER_SECOND       1000000000
/** microseconds in a second */
#define MICROSECONDS_PER_SECOND      1000000
/** milliseconds in a second */
#define MILLISECONDS_PER_SECOND      1000
/** nanoseconds in a millisecond */
#define NANOSECONDS_PER_MILLISECOND  1000000
/** microseconds in a millisecond */
#define MICROSECONDS_PER_MILLISECOND 1000

/**
 * Sleeps the given number of milliseconds.
 * @param milliseconds number of milliseconds
 */
void
_dbus_sleep_milliseconds (int milliseconds)
{
  Sleep (milliseconds);
}


/**
 * Get current time, as in gettimeofday(). Never uses the monotonic
 * clock.
 *
 * @param tv_sec return location for number of seconds
 * @param tv_usec return location for number of microseconds
 */
void
_dbus_get_real_time (long *tv_sec,
                     long *tv_usec)
{
  FILETIME ft;
  dbus_uint64_t time64;

  GetSystemTimeAsFileTime (&ft);

  memcpy (&time64, &ft, sizeof (time64));

  /* Convert from 100s of nanoseconds since 1601-01-01
  * to Unix epoch. Yes, this is Y2038 unsafe.
  */
  time64 -= DBUS_INT64_CONSTANT (116444736000000000);
  time64 /= 10;

  if (tv_sec)
    *tv_sec = time64 / 1000000;

  if (tv_usec)
    *tv_usec = time64 % 1000000;
}

/**
 * Get current time, as in gettimeofday(). Use the monotonic clock if
 * available, to avoid problems when the system time changes.
 *
 * @param tv_sec return location for number of seconds
 * @param tv_usec return location for number of microseconds
 */
void
_dbus_get_monotonic_time (long *tv_sec,
                          long *tv_usec)
{
  /* no implementation yet, fall back to wall-clock time */
  _dbus_get_real_time (tv_sec, tv_usec);
}

/**
 * signal (SIGPIPE, SIG_IGN);
 */
void
_dbus_disable_sigpipe (void)
{
}

/**
 * Creates a directory; succeeds if the directory
 * is created or already existed.
 *
 * @param filename directory filename
 * @param error initialized error object
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_create_directory (const DBusString *filename,
                        DBusError        *error)
{
  const char *filename_c;

  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  filename_c = _dbus_string_get_const_data (filename);

  if (!CreateDirectoryA (filename_c, NULL))
    {
      if (GetLastError () == ERROR_ALREADY_EXISTS)
        return TRUE;

      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "Failed to create directory %s: %s\n",
                      filename_c, _dbus_strerror_from_errno ());
      return FALSE;
    }
  else
    return TRUE;
}


/**
 * Generates the given number of random bytes,
 * using the best mechanism we can come up with.
 *
 * @param str the string
 * @param n_bytes the number of random bytes to append to string
 * @returns #TRUE on success, #FALSE if no memory
 */
dbus_bool_t
_dbus_generate_random_bytes (DBusString *str,
                             int         n_bytes)
{
  int old_len;
  char *p;
  HCRYPTPROV hprov;

  old_len = _dbus_string_get_length (str);

  if (!_dbus_string_lengthen (str, n_bytes))
    return FALSE;

  p = _dbus_string_get_data_len (str, old_len, n_bytes);

  if (!CryptAcquireContext (&hprov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    return FALSE;

  if (!CryptGenRandom (hprov, n_bytes, p))
    {
      CryptReleaseContext (hprov, 0);
      return FALSE;
    }

  CryptReleaseContext (hprov, 0);

  return TRUE;
}

/**
 * Gets the temporary files directory by inspecting the environment variables 
 * TMPDIR, TMP, and TEMP in that order. If none of those are set "/tmp" is returned
 *
 * @returns location of temp directory, or #NULL if no memory for locking
 */
const char*
_dbus_get_tmpdir(void)
{
  /* Protected by _DBUS_LOCK_sysdeps */
  static const char* tmpdir = NULL;
  static char buf[1000];

  if (!_DBUS_LOCK (sysdeps))
    return NULL;

  if (tmpdir == NULL)
    {
      char *last_slash;

      if (!GetTempPathA (sizeof (buf), buf))
        {
          _dbus_warn ("GetTempPath failed\n");
          _dbus_abort ();
        }

      /* Drop terminating backslash or slash */
      last_slash = _mbsrchr (buf, '\\');
      if (last_slash > buf && last_slash[1] == '\0')
        last_slash[0] = '\0';
      last_slash = _mbsrchr (buf, '/');
      if (last_slash > buf && last_slash[1] == '\0')
        last_slash[0] = '\0';

      tmpdir = buf;
    }

  _DBUS_UNLOCK (sysdeps);

  _dbus_assert(tmpdir != NULL);

  return tmpdir;
}


/**
 * Deletes the given file.
 *
 * @param filename the filename
 * @param error error location
 * 
 * @returns #TRUE if unlink() succeeded
 */
dbus_bool_t
_dbus_delete_file (const DBusString *filename,
                   DBusError        *error)
{
  const char *filename_c;

  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  filename_c = _dbus_string_get_const_data (filename);

  if (DeleteFileA (filename_c) == 0)
    {
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "Failed to delete file %s: %s\n",
                      filename_c, _dbus_strerror_from_errno ());
      return FALSE;
    }
  else
    return TRUE;
}

#if !defined (DBUS_DISABLE_ASSERT) || defined(DBUS_ENABLE_EMBEDDED_TESTS)

#if defined(_MSC_VER) || defined(DBUS_WINCE)
# ifdef BACKTRACES
#  undef BACKTRACES
# endif
#else
# define BACKTRACES
#endif

#ifdef BACKTRACES
/*
 * Backtrace Generator
 *
 * Copyright 2004 Eric Poech
 * Copyright 2004 Robert Shearman
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <winver.h>
#include <imagehlp.h>
#include <stdio.h>

#define DPRINTF _dbus_warn

#ifdef _MSC_VER
#define BOOL int

#define __i386__
#endif

//#define MAKE_FUNCPTR(f) static typeof(f) * p##f

//MAKE_FUNCPTR(StackWalk);
//MAKE_FUNCPTR(SymGetModuleBase);
//MAKE_FUNCPTR(SymFunctionTableAccess);
//MAKE_FUNCPTR(SymInitialize);
//MAKE_FUNCPTR(SymGetSymFromAddr);
//MAKE_FUNCPTR(SymGetModuleInfo);
static BOOL (WINAPI *pStackWalk)(
  DWORD MachineType,
  HANDLE hProcess,
  HANDLE hThread,
  LPSTACKFRAME StackFrame,
  PVOID ContextRecord,
  PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
  PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
  PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
  PTRANSLATE_ADDRESS_ROUTINE TranslateAddress
);
#ifdef _WIN64
static DWORD64 (WINAPI *pSymGetModuleBase)(
  HANDLE hProcess,
  DWORD64 dwAddr
);
static PVOID  (WINAPI *pSymFunctionTableAccess)(
  HANDLE hProcess,
  DWORD64 AddrBase
);
#else
static DWORD (WINAPI *pSymGetModuleBase)(
  HANDLE hProcess,
  DWORD dwAddr
);
static PVOID  (WINAPI *pSymFunctionTableAccess)(
  HANDLE hProcess,
  DWORD AddrBase
);
#endif
static BOOL  (WINAPI *pSymInitialize)(
  HANDLE hProcess,
  PSTR UserSearchPath,
  BOOL fInvadeProcess
);
static BOOL  (WINAPI *pSymGetSymFromAddr)(
  HANDLE hProcess,
  DWORD Address,
  PDWORD Displacement,
  PIMAGEHLP_SYMBOL Symbol
);
static BOOL  (WINAPI *pSymGetModuleInfo)(
  HANDLE hProcess,
  DWORD dwAddr,
  PIMAGEHLP_MODULE ModuleInfo
);
static DWORD  (WINAPI *pSymSetOptions)(
  DWORD SymOptions
);


static BOOL init_backtrace()
{
    HMODULE hmodDbgHelp = LoadLibraryA("dbghelp");
/*
    #define GETFUNC(x) \
    p##x = (typeof(x)*)GetProcAddress(hmodDbgHelp, #x); \
    if (!p##x) \
    { \
        return FALSE; \
    }
    */


//    GETFUNC(StackWalk);
//    GETFUNC(SymGetModuleBase);
//    GETFUNC(SymFunctionTableAccess);
//    GETFUNC(SymInitialize);
//    GETFUNC(SymGetSymFromAddr);
//    GETFUNC(SymGetModuleInfo);

#define FUNC(x) #x

      pStackWalk = (BOOL  (WINAPI *)(
DWORD MachineType,
HANDLE hProcess,
HANDLE hThread,
LPSTACKFRAME StackFrame,
PVOID ContextRecord,
PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
PTRANSLATE_ADDRESS_ROUTINE TranslateAddress
))GetProcAddress (hmodDbgHelp, FUNC(StackWalk));
#ifdef _WIN64
    pSymGetModuleBase=(DWORD64  (WINAPI *)(
  HANDLE hProcess,
  DWORD64 dwAddr
))GetProcAddress (hmodDbgHelp, FUNC(SymGetModuleBase));
    pSymFunctionTableAccess=(PVOID  (WINAPI *)(
  HANDLE hProcess,
  DWORD64 AddrBase
))GetProcAddress (hmodDbgHelp, FUNC(SymFunctionTableAccess));
#else
    pSymGetModuleBase=(DWORD  (WINAPI *)(
  HANDLE hProcess,
  DWORD dwAddr
))GetProcAddress (hmodDbgHelp, FUNC(SymGetModuleBase));
    pSymFunctionTableAccess=(PVOID  (WINAPI *)(
  HANDLE hProcess,
  DWORD AddrBase
))GetProcAddress (hmodDbgHelp, FUNC(SymFunctionTableAccess));
#endif
    pSymInitialize = (BOOL  (WINAPI *)(
  HANDLE hProcess,
  PSTR UserSearchPath,
  BOOL fInvadeProcess
))GetProcAddress (hmodDbgHelp, FUNC(SymInitialize));
    pSymGetSymFromAddr = (BOOL  (WINAPI *)(
  HANDLE hProcess,
  DWORD Address,
  PDWORD Displacement,
  PIMAGEHLP_SYMBOL Symbol
))GetProcAddress (hmodDbgHelp, FUNC(SymGetSymFromAddr));
    pSymGetModuleInfo = (BOOL  (WINAPI *)(
  HANDLE hProcess,
  DWORD dwAddr,
  PIMAGEHLP_MODULE ModuleInfo
))GetProcAddress (hmodDbgHelp, FUNC(SymGetModuleInfo));
pSymSetOptions = (DWORD  (WINAPI *)(
DWORD SymOptions
))GetProcAddress (hmodDbgHelp, FUNC(SymSetOptions));


    pSymSetOptions(SYMOPT_UNDNAME);

    pSymInitialize(GetCurrentProcess(), NULL, TRUE);

    return TRUE;
}

static void dump_backtrace_for_thread(HANDLE hThread)
{
    STACKFRAME sf;
    CONTEXT context;
    DWORD dwImageType;

    if (!pStackWalk)
        if (!init_backtrace())
            return;

    /* can't use this function for current thread as GetThreadContext
     * doesn't support getting context from current thread */
    if (hThread == GetCurrentThread())
        return;

    DPRINTF("Backtrace:\n");

    _DBUS_ZERO(context);
    context.ContextFlags = CONTEXT_FULL;

    SuspendThread(hThread);

    if (!GetThreadContext(hThread, &context))
    {
        DPRINTF("Couldn't get thread context (error %ld)\n", GetLastError());
        ResumeThread(hThread);
        return;
    }

    _DBUS_ZERO(sf);

#ifdef __i386__
    sf.AddrFrame.Offset = context.Ebp;
    sf.AddrFrame.Mode = AddrModeFlat;
    sf.AddrPC.Offset = context.Eip;
    sf.AddrPC.Mode = AddrModeFlat;
    dwImageType = IMAGE_FILE_MACHINE_I386;
#elif _M_X64
  dwImageType                = IMAGE_FILE_MACHINE_AMD64;
  sf.AddrPC.Offset    = context.Rip;
  sf.AddrPC.Mode      = AddrModeFlat;
  sf.AddrFrame.Offset = context.Rsp;
  sf.AddrFrame.Mode   = AddrModeFlat;
  sf.AddrStack.Offset = context.Rsp;
  sf.AddrStack.Mode   = AddrModeFlat;
#elif _M_IA64
  dwImageType                 = IMAGE_FILE_MACHINE_IA64;
  sf.AddrPC.Offset    = context.StIIP;
  sf.AddrPC.Mode      = AddrModeFlat;
  sf.AddrFrame.Offset = context.IntSp;
  sf.AddrFrame.Mode   = AddrModeFlat;
  sf.AddrBStore.Offset= context.RsBSP;
  sf.AddrBStore.Mode  = AddrModeFlat;
  sf.AddrStack.Offset = context.IntSp;
  sf.AddrStack.Mode   = AddrModeFlat;
#else
# error You need to fill in the STACKFRAME structure for your architecture
#endif

    while (pStackWalk(dwImageType, GetCurrentProcess(),
                     hThread, &sf, &context, NULL, pSymFunctionTableAccess,
                     pSymGetModuleBase, NULL))
    {
        BYTE buffer[256];
        IMAGEHLP_SYMBOL * pSymbol = (IMAGEHLP_SYMBOL *)buffer;
        DWORD dwDisplacement;

        pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        pSymbol->MaxNameLength = sizeof(buffer) - sizeof(IMAGEHLP_SYMBOL) + 1;

        if (!pSymGetSymFromAddr(GetCurrentProcess(), sf.AddrPC.Offset,
                                &dwDisplacement, pSymbol))
        {
            IMAGEHLP_MODULE ModuleInfo;
            ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);

            if (!pSymGetModuleInfo(GetCurrentProcess(), sf.AddrPC.Offset,
                                   &ModuleInfo))
                DPRINTF("1\t%p\n", (void*)sf.AddrPC.Offset);
            else
                DPRINTF("2\t%s+0x%lx\n", ModuleInfo.ImageName,
                    sf.AddrPC.Offset - ModuleInfo.BaseOfImage);
        }
        else if (dwDisplacement)
            DPRINTF("3\t%s+0x%lx\n", pSymbol->Name, dwDisplacement);
        else
            DPRINTF("4\t%s\n", pSymbol->Name);
    }

    ResumeThread(hThread);
}

static DWORD WINAPI dump_thread_proc(LPVOID lpParameter)
{
    dump_backtrace_for_thread((HANDLE)lpParameter);
    return 0;
}

/* cannot get valid context from current thread, so we have to execute
 * backtrace from another thread */
static void dump_backtrace()
{
    HANDLE hCurrentThread;
    HANDLE hThread;
    DWORD dwThreadId;
    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
        GetCurrentProcess(), &hCurrentThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
    hThread = CreateThread(NULL, 0, dump_thread_proc, (LPVOID)hCurrentThread,
        0, &dwThreadId);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(hCurrentThread);
}
#endif
#endif /* asserts or tests enabled */

#ifdef BACKTRACES
void _dbus_print_backtrace(void)
{
  init_backtrace();
  dump_backtrace();
}
#else
void _dbus_print_backtrace(void)
{
  _dbus_verbose ("  D-Bus not compiled with backtrace support\n");
}
#endif

static dbus_uint32_t fromAscii(char ascii)
{
    if(ascii >= '0' && ascii <= '9')
        return ascii - '0';
    if(ascii >= 'A' && ascii <= 'F')
        return ascii - 'A' + 10;
    if(ascii >= 'a' && ascii <= 'f')
        return ascii - 'a' + 10;
    return 0;    
}

dbus_bool_t _dbus_read_local_machine_uuid   (DBusGUID         *machine_id,
                                             dbus_bool_t       create_if_not_found,
                                             DBusError        *error)
{
#ifdef DBUS_WINCE
	return TRUE;
  // TODO
#else
    HW_PROFILE_INFOA info;
    char *lpc = &info.szHwProfileGuid[0];
    dbus_uint32_t u;

    //  the hw-profile guid lives long enough
    if(!GetCurrentHwProfileA(&info))
      {
        dbus_set_error (error, DBUS_ERROR_NO_MEMORY, NULL); // FIXME
        return FALSE;  
      }

    // Form: {12340001-4980-1920-6788-123456789012}
    lpc++;
    // 12340001
    u = ((fromAscii(lpc[0]) <<  0) |
         (fromAscii(lpc[1]) <<  4) |
         (fromAscii(lpc[2]) <<  8) |
         (fromAscii(lpc[3]) << 12) |
         (fromAscii(lpc[4]) << 16) |
         (fromAscii(lpc[5]) << 20) |
         (fromAscii(lpc[6]) << 24) |
         (fromAscii(lpc[7]) << 28));
    machine_id->as_uint32s[0] = u;

    lpc += 9;
    // 4980-1920
    u = ((fromAscii(lpc[0]) <<  0) |
         (fromAscii(lpc[1]) <<  4) |
         (fromAscii(lpc[2]) <<  8) |
         (fromAscii(lpc[3]) << 12) |
         (fromAscii(lpc[5]) << 16) |
         (fromAscii(lpc[6]) << 20) |
         (fromAscii(lpc[7]) << 24) |
         (fromAscii(lpc[8]) << 28));
    machine_id->as_uint32s[1] = u;
    
    lpc += 10;
    // 6788-1234
    u = ((fromAscii(lpc[0]) <<  0) |
         (fromAscii(lpc[1]) <<  4) |
         (fromAscii(lpc[2]) <<  8) |
         (fromAscii(lpc[3]) << 12) |
         (fromAscii(lpc[5]) << 16) |
         (fromAscii(lpc[6]) << 20) |
         (fromAscii(lpc[7]) << 24) |
         (fromAscii(lpc[8]) << 28));
    machine_id->as_uint32s[2] = u;
    
    lpc += 9;
    // 56789012
    u = ((fromAscii(lpc[0]) <<  0) |
         (fromAscii(lpc[1]) <<  4) |
         (fromAscii(lpc[2]) <<  8) |
         (fromAscii(lpc[3]) << 12) |
         (fromAscii(lpc[4]) << 16) |
         (fromAscii(lpc[5]) << 20) |
         (fromAscii(lpc[6]) << 24) |
         (fromAscii(lpc[7]) << 28));
    machine_id->as_uint32s[3] = u;
#endif
    return TRUE;
}

static
HANDLE _dbus_global_lock (const char *mutexname)
{
  HANDLE mutex;
  DWORD gotMutex;

  mutex = CreateMutexA( NULL, FALSE, mutexname );
  if( !mutex )
    {
      return FALSE;
    }

   gotMutex = WaitForSingleObject( mutex, INFINITE );
   switch( gotMutex )
     {
       case WAIT_ABANDONED:
               ReleaseMutex (mutex);
               CloseHandle (mutex);
               return 0;
       case WAIT_FAILED:
       case WAIT_TIMEOUT:
               return 0;
     }

   return mutex;
}

static
void _dbus_global_unlock (HANDLE mutex)
{
  ReleaseMutex (mutex);
  CloseHandle (mutex); 
}

// for proper cleanup in dbus-daemon
static HANDLE hDBusDaemonMutex = NULL;
static HANDLE hDBusSharedMem = NULL;
// sync _dbus_daemon_publish_session_bus_address, _dbus_daemon_unpublish_session_bus_address and _dbus_daemon_already_runs
static const char *cUniqueDBusInitMutex = "UniqueDBusInitMutex";
// sync _dbus_get_autolaunch_address
static const char *cDBusAutolaunchMutex = "DBusAutolaunchMutex";
// mutex to determine if dbus-daemon is already started (per user)
static const char *cDBusDaemonMutex = "DBusDaemonMutex";
// named shm for dbus adress info (per user)
static const char *cDBusDaemonAddressInfo = "DBusDaemonAddressInfo";

static dbus_bool_t
_dbus_get_install_root_as_hash(DBusString *out)
{
    DBusString install_path;

    char path[MAX_PATH*2];
    int path_size = sizeof(path);

    if (!_dbus_get_install_root(path,path_size))
        return FALSE;

    _dbus_string_init(&install_path);
    _dbus_string_append(&install_path,path);

    _dbus_string_init(out);
    _dbus_string_tolower_ascii(&install_path,0,_dbus_string_get_length(&install_path));

    if (!_dbus_sha_compute (&install_path, out))
        return FALSE;

    return TRUE;
}

static dbus_bool_t
_dbus_get_address_string (DBusString *out, const char *basestring, const char *scope)
{
  _dbus_string_init(out);
  _dbus_string_append(out,basestring);

  if (!scope)
    {
      return TRUE;
    }
  else if (strcmp(scope,"*install-path") == 0
        // for 1.3 compatibility
        || strcmp(scope,"install-path") == 0)
    {
      DBusString temp;
      if (!_dbus_get_install_root_as_hash(&temp))
        {
          _dbus_string_free(out);
           return FALSE;
        }
      _dbus_string_append(out,"-");
      _dbus_string_append(out,_dbus_string_get_const_data(&temp));
      _dbus_string_free(&temp);
    }
  else if (strcmp(scope,"*user") == 0)
    {
      _dbus_string_append(out,"-");
      if (!_dbus_append_user_from_current_process(out))
        {
           _dbus_string_free(out);
           return FALSE;
        }
    }
  else if (strlen(scope) > 0)
    {
      _dbus_string_append(out,"-");
      _dbus_string_append(out,scope);
      return TRUE;
    }
  return TRUE;
}

static dbus_bool_t
_dbus_get_shm_name (DBusString *out,const char *scope)
{
  return _dbus_get_address_string (out,cDBusDaemonAddressInfo,scope);
}

static dbus_bool_t
_dbus_get_mutex_name (DBusString *out,const char *scope)
{
  return _dbus_get_address_string (out,cDBusDaemonMutex,scope);
}

dbus_bool_t
_dbus_daemon_is_session_bus_address_published (const char *scope)
{
  HANDLE lock;
  DBusString mutex_name;

  if (!_dbus_get_mutex_name(&mutex_name,scope))
    {
      _dbus_string_free( &mutex_name );
      return FALSE;
    }

  if (hDBusDaemonMutex)
      return TRUE;

  // sync _dbus_daemon_publish_session_bus_address, _dbus_daemon_unpublish_session_bus_address and _dbus_daemon_already_runs
  lock = _dbus_global_lock( cUniqueDBusInitMutex );

  // we use CreateMutex instead of OpenMutex because of possible race conditions,
  // see http://msdn.microsoft.com/en-us/library/ms684315%28VS.85%29.aspx
  hDBusDaemonMutex = CreateMutexA( NULL, FALSE, _dbus_string_get_const_data(&mutex_name) );

  /* The client uses mutex ownership to detect a running server, so the server should do so too.
     Fortunally the client deletes the mutex in the lock protected area, so checking presence 
     will work too.  */

  _dbus_global_unlock( lock );

  _dbus_string_free( &mutex_name );

  if (hDBusDaemonMutex  == NULL)
      return FALSE;
  if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
      CloseHandle(hDBusDaemonMutex);
      hDBusDaemonMutex = NULL;
      return TRUE;
    }
  // mutex wasn't created before, so return false.
  // We leave the mutex name allocated for later reusage
  // in _dbus_daemon_publish_session_bus_address.
  return FALSE;
}

dbus_bool_t
_dbus_daemon_publish_session_bus_address (const char* address, const char *scope)
{
  HANDLE lock;
  char *shared_addr = NULL;
  DBusString shm_name;
  DBusString mutex_name;

  _dbus_assert (address);

  if (!_dbus_get_mutex_name(&mutex_name,scope))
    {
      _dbus_string_free( &mutex_name );
      return FALSE;
    }

  // sync _dbus_daemon_publish_session_bus_address, _dbus_daemon_unpublish_session_bus_address and _dbus_daemon_already_runs
  lock = _dbus_global_lock( cUniqueDBusInitMutex );

  if (!hDBusDaemonMutex)
    {
      hDBusDaemonMutex = CreateMutexA( NULL, FALSE, _dbus_string_get_const_data(&mutex_name) );
    }
  _dbus_string_free( &mutex_name );

  // acquire the mutex
  if (WaitForSingleObject( hDBusDaemonMutex, 10 ) != WAIT_OBJECT_0)
    {
      _dbus_global_unlock( lock );
      CloseHandle( hDBusDaemonMutex );
      return FALSE;
    }

  if (!_dbus_get_shm_name(&shm_name,scope))
    {
      _dbus_string_free( &shm_name );
      _dbus_global_unlock( lock );
      return FALSE;
    }

  // create shm
  hDBusSharedMem = CreateFileMappingA( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                       0, strlen( address ) + 1, _dbus_string_get_const_data(&shm_name) );
  _dbus_assert( hDBusSharedMem );

  shared_addr = MapViewOfFile( hDBusSharedMem, FILE_MAP_WRITE, 0, 0, 0 );

  _dbus_assert (shared_addr);

  strcpy( shared_addr, address);

  // cleanup
  UnmapViewOfFile( shared_addr );

  _dbus_global_unlock( lock );
  _dbus_verbose( "published session bus address at %s\n",_dbus_string_get_const_data (&shm_name) );

  _dbus_string_free( &shm_name );
  return TRUE;
}

void
_dbus_daemon_unpublish_session_bus_address (void)
{
  HANDLE lock;

  // sync _dbus_daemon_publish_session_bus_address, _dbus_daemon_unpublish_session_bus_address and _dbus_daemon_already_runs
  lock = _dbus_global_lock( cUniqueDBusInitMutex );

  CloseHandle( hDBusSharedMem );

  hDBusSharedMem = NULL;

  ReleaseMutex( hDBusDaemonMutex );

  CloseHandle( hDBusDaemonMutex );

  hDBusDaemonMutex = NULL;

  _dbus_global_unlock( lock );
}

static dbus_bool_t
_dbus_get_autolaunch_shm (DBusString *address, DBusString *shm_name)
{
  HANDLE sharedMem;
  char *shared_addr;
  int i;

  // read shm
  for(i=0;i<20;++i) {
      // we know that dbus-daemon is available, so we wait until shm is available
      sharedMem = OpenFileMappingA( FILE_MAP_READ, FALSE, _dbus_string_get_const_data(shm_name));
      if( sharedMem == 0 )
          Sleep( 100 );
      if ( sharedMem != 0)
          break;
  }

  if( sharedMem == 0 )
      return FALSE;

  shared_addr = MapViewOfFile( sharedMem, FILE_MAP_READ, 0, 0, 0 );

  if( !shared_addr )
      return FALSE;

  _dbus_string_init( address );

  _dbus_string_append( address, shared_addr );

  // cleanup
  UnmapViewOfFile( shared_addr );

  CloseHandle( sharedMem );

  return TRUE;
}

static dbus_bool_t
_dbus_daemon_already_runs (DBusString *address, DBusString *shm_name, const char *scope)
{
  HANDLE lock;
  HANDLE daemon;
  DBusString mutex_name;
  dbus_bool_t bRet = TRUE;

  if (!_dbus_get_mutex_name(&mutex_name,scope))
    {
      _dbus_string_free( &mutex_name );
      return FALSE;
    }

  // sync _dbus_daemon_publish_session_bus_address, _dbus_daemon_unpublish_session_bus_address and _dbus_daemon_already_runs
  lock = _dbus_global_lock( cUniqueDBusInitMutex );

  // do checks
  daemon = CreateMutexA( NULL, FALSE, _dbus_string_get_const_data(&mutex_name) );
  if(WaitForSingleObject( daemon, 10 ) != WAIT_TIMEOUT)
    {
      ReleaseMutex (daemon);
      CloseHandle (daemon);

      _dbus_global_unlock( lock );
      _dbus_string_free( &mutex_name );
      return FALSE;
    }

  // read shm
  bRet = _dbus_get_autolaunch_shm( address, shm_name );

  // cleanup
  CloseHandle ( daemon );

  _dbus_global_unlock( lock );
  _dbus_string_free( &mutex_name );

  return bRet;
}

dbus_bool_t
_dbus_get_autolaunch_address (const char *scope, DBusString *address,
                              DBusError *error)
{
  HANDLE mutex;
  STARTUPINFOA si;
  PROCESS_INFORMATION pi;
  dbus_bool_t retval = FALSE;
  LPSTR lpFile;
  char dbus_exe_path[MAX_PATH];
  char dbus_args[MAX_PATH * 2];
  const char * daemon_name = DBUS_DAEMON_NAME ".exe";
  DBusString shm_name;

  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  if (!_dbus_get_shm_name(&shm_name,scope))
    {
        dbus_set_error_const (error, DBUS_ERROR_FAILED, "could not determine shm name");
        return FALSE;
    }

  mutex = _dbus_global_lock ( cDBusAutolaunchMutex );

  if (_dbus_daemon_already_runs(address,&shm_name,scope))
    {
        _dbus_verbose( "found running dbus daemon at %s\n",
                       _dbus_string_get_const_data (&shm_name) );
        retval = TRUE;
        goto out;
    }

  if (!SearchPathA(NULL, daemon_name, NULL, sizeof(dbus_exe_path), dbus_exe_path, &lpFile))
    {
      // Look in directory containing dbus shared library
      HMODULE hmod;
      char dbus_module_path[MAX_PATH];
      DWORD rc;

      _dbus_verbose( "did not found dbus daemon executable on default search path, "
            "trying path where dbus shared library is located");

      hmod = _dbus_win_get_dll_hmodule();
      rc = GetModuleFileNameA(hmod, dbus_module_path, sizeof(dbus_module_path));
      if (rc <= 0)
        {
          dbus_set_error_const (error, DBUS_ERROR_FAILED, "could not retrieve dbus shared library file name");
          retval = FALSE;
          goto out;
        }
      else
        {
          char *ext_idx = strrchr(dbus_module_path, '\\');
          if (ext_idx)
          *ext_idx = '\0';
          if (!SearchPathA(dbus_module_path, daemon_name, NULL, sizeof(dbus_exe_path), dbus_exe_path, &lpFile))
            {
              dbus_set_error_const (error, DBUS_ERROR_FAILED, "could not find dbus-daemon executable");
              retval = FALSE;
              printf ("please add the path to %s to your PATH environment variable\n", daemon_name);
              printf ("or start the daemon manually\n\n");
              goto out;
            }
          _dbus_verbose( "found dbus daemon executable at %s",dbus_module_path);
        }
    }


  // Create process
  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( &pi, sizeof(pi) );

  _snprintf(dbus_args, sizeof(dbus_args) - 1, "\"%s\" %s", dbus_exe_path,  " --session");

//  argv[i] = "--config-file=bus\\session.conf";
//  printf("create process \"%s\" %s\n", dbus_exe_path, dbus_args);
  if(CreateProcessA(dbus_exe_path, dbus_args, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
      CloseHandle (pi.hThread);
      CloseHandle (pi.hProcess);
      retval = _dbus_get_autolaunch_shm( address, &shm_name );
      if (retval == FALSE)
        dbus_set_error_const (error, DBUS_ERROR_FAILED, "Failed to get autolaunch address from launched dbus-daemon");
    }
  else
    {
      dbus_set_error_const (error, DBUS_ERROR_FAILED, "Failed to launch dbus-daemon");
      retval = FALSE;
    }

out:
  if (retval)
    _DBUS_ASSERT_ERROR_IS_CLEAR (error);
  else
    _DBUS_ASSERT_ERROR_IS_SET (error);
  
  _dbus_global_unlock (mutex);

  return retval;
 }


/** Makes the file readable by every user in the system.
 *
 * @param filename the filename
 * @param error error location
 * @returns #TRUE if the file's permissions could be changed.
 */
dbus_bool_t
_dbus_make_file_world_readable(const DBusString *filename,
                               DBusError *error)
{
  // TODO
  return TRUE;
}

/**
 * Atomically increments an integer
 *
 * @param atomic pointer to the integer to increment
 * @returns the value before incrementing
 *
 */
dbus_int32_t
_dbus_atomic_inc (DBusAtomic *atomic)
{
  // +/- 1 is needed here!
  // no volatile argument with mingw
  return InterlockedIncrement (&atomic->value) - 1;
}

/**
 * Atomically decrement an integer
 *
 * @param atomic pointer to the integer to decrement
 * @returns the value before decrementing
 *
 */
dbus_int32_t
_dbus_atomic_dec (DBusAtomic *atomic)
{
  // +/- 1 is needed here!
  // no volatile argument with mingw
  return InterlockedDecrement (&atomic->value) + 1;
}

/**
 * Atomically get the value of an integer. It may change at any time
 * thereafter, so this is mostly only useful for assertions.
 *
 * @param atomic pointer to the integer to get
 * @returns the value at this moment
 */
dbus_int32_t
_dbus_atomic_get (DBusAtomic *atomic)
{
  /* In this situation, GLib issues a MemoryBarrier() and then returns
   * atomic->value. However, mingw from mingw.org (not to be confused with
   * mingw-w64 from mingw-w64.sf.net) does not have MemoryBarrier in its
   * headers, so we have to get a memory barrier some other way.
   *
   * InterlockedIncrement is older, and is documented on MSDN to be a full
   * memory barrier, so let's use that.
   */
  long dummy = 0;

  InterlockedExchange (&dummy, 1);

  return atomic->value;
}

/**
 * Called when the bus daemon is signaled to reload its configuration; any
 * caches should be nuked. Of course any caches that need explicit reload
 * are probably broken, but c'est la vie.
 *
 * 
 */
void
_dbus_flush_caches (void)
{
}

/**
 * See if errno is EAGAIN or EWOULDBLOCK (this has to be done differently
 * for Winsock so is abstracted)
 *
 * @returns #TRUE if errno == EAGAIN or errno == EWOULDBLOCK
 */
dbus_bool_t
_dbus_get_is_errno_eagain_or_ewouldblock (void)
{
  return errno == WSAEWOULDBLOCK;
}

/**
 * return the absolute path of the dbus installation 
 *
 * @param prefix buffer for installation path
 * @param len length of buffer
 * @returns #FALSE on failure
 */
dbus_bool_t
_dbus_get_install_root(char *prefix, int len)
{
    //To find the prefix, we cut the filename and also \bin\ if present
    DWORD pathLength;
    char *lastSlash;
    SetLastError( 0 );
    pathLength = GetModuleFileNameA(_dbus_win_get_dll_hmodule(), prefix, len);
    if ( pathLength == 0 || GetLastError() != 0 ) {
        *prefix = '\0';
        return FALSE;
    }
    lastSlash = _mbsrchr(prefix, '\\');
    if (lastSlash == NULL) {
        *prefix = '\0';
        return FALSE;
    }
    //cut off binary name
    lastSlash[1] = 0;

    //cut possible "\\bin"

    //this fails if we are in a double-byte system codepage and the
    //folder's name happens to end with the *bytes*
    //"\\bin"... (I.e. the second byte of some Han character and then
    //the Latin "bin", but that is not likely I think...
    if (lastSlash - prefix >= 4 && strnicmp(lastSlash - 4, "\\bin", 4) == 0)
        lastSlash[-3] = 0;
    else if (lastSlash - prefix >= 10 && strnicmp(lastSlash - 10, "\\bin\\debug", 10) == 0)
        lastSlash[-9] = 0;
    else if (lastSlash - prefix >= 12 && strnicmp(lastSlash - 12, "\\bin\\release", 12) == 0)
        lastSlash[-11] = 0;

    return TRUE;
}

/** 
  find config file either from installation or build root according to 
  the following path layout 
    install-root/
      bin/dbus-daemon[d].exe
      etc/<config-file>.conf *or* etc/dbus-1/<config-file>.conf
      (the former above is what dbus4win uses, the latter above is
      what a "normal" Unix-style "make install" uses)

    build-root/
      bin/dbus-daemon[d].exe
      bus/<config-file>.conf 
*/
dbus_bool_t 
_dbus_get_config_file_name(DBusString *config_file, char *s)
{
  char path[MAX_PATH*2];
  int path_size = sizeof(path);

  if (!_dbus_get_install_root(path,path_size))
    return FALSE;

  if(strlen(s) + 4 + strlen(path) > sizeof(path)-2)
    return FALSE;
  strcat(path,"etc\\");
  strcat(path,s);
  if (_dbus_file_exists(path)) 
    {
      // find path from executable 
      if (!_dbus_string_append (config_file, path))
        return FALSE;
    }
  else 
    {
      if (!_dbus_get_install_root(path,path_size))
        return FALSE;
      if(strlen(s) + 11 + strlen(path) > sizeof(path)-2)
        return FALSE;
      strcat(path,"etc\\dbus-1\\");
      strcat(path,s);
  
      if (_dbus_file_exists(path)) 
        {
          if (!_dbus_string_append (config_file, path))
            return FALSE;
        }
      else
        {
          if (!_dbus_get_install_root(path,path_size))
            return FALSE;
          if(strlen(s) + 4 + strlen(path) > sizeof(path)-2)
            return FALSE;
          strcat(path,"bus\\");
          strcat(path,s);
          
          if (_dbus_file_exists(path)) 
            {
              if (!_dbus_string_append (config_file, path))
                return FALSE;
            }
        }
    }
  return TRUE;
}    

/* See comment in dbus-sysdeps-unix.c */
dbus_bool_t
_dbus_lookup_session_address (dbus_bool_t *supported,
                              DBusString  *address,
                              DBusError   *error)
{
  /* Probably fill this in with something based on COM? */
  *supported = FALSE;
  return TRUE;
}

/**
 * Appends the directory in which a keyring for the given credentials
 * should be stored.  The credentials should have either a Windows or
 * UNIX user in them.  The directory should be an absolute path.
 *
 * On UNIX the directory is ~/.dbus-keyrings while on Windows it should probably
 * be something else, since the dotfile convention is not normal on Windows.
 * 
 * @param directory string to append directory to
 * @param credentials credentials the directory should be for
 *  
 * @returns #FALSE on no memory
 */
dbus_bool_t
_dbus_append_keyring_directory_for_credentials (DBusString      *directory,
                                                DBusCredentials *credentials)
{
  DBusString homedir;
  DBusString dotdir;
  const char *homepath;
  const char *homedrive;

  _dbus_assert (credentials != NULL);
  _dbus_assert (!_dbus_credentials_are_anonymous (credentials));
  
  if (!_dbus_string_init (&homedir))
    return FALSE;

  homedrive = _dbus_getenv("HOMEDRIVE");
  if (homedrive != NULL && *homedrive != '\0')
    {
      _dbus_string_append(&homedir,homedrive);
    }

  homepath = _dbus_getenv("HOMEPATH");
  if (homepath != NULL && *homepath != '\0')
    {
      _dbus_string_append(&homedir,homepath);
    }
  
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
  {
    const char *override;
    
    override = _dbus_getenv ("DBUS_TEST_HOMEDIR");
    if (override != NULL && *override != '\0')
      {
        _dbus_string_set_length (&homedir, 0);
        if (!_dbus_string_append (&homedir, override))
          goto failed;

        _dbus_verbose ("Using fake homedir for testing: %s\n",
                       _dbus_string_get_const_data (&homedir));
      }
    else
      {
        /* Not strictly thread-safe, but if we fail at thread-safety here,
         * the worst that will happen is some extra warnings. */
        static dbus_bool_t already_warned = FALSE;
        if (!already_warned)
          {
            _dbus_warn ("Using your real home directory for testing, set DBUS_TEST_HOMEDIR to avoid\n");
            already_warned = TRUE;
          }
      }
  }
#endif

#ifdef DBUS_WINCE
  /* It's not possible to create a .something directory in Windows CE
     using the file explorer.  */
#define KEYRING_DIR "dbus-keyrings"
#else
#define KEYRING_DIR ".dbus-keyrings"
#endif

  _dbus_string_init_const (&dotdir, KEYRING_DIR);
  if (!_dbus_concat_dir_and_file (&homedir,
                                  &dotdir))
    goto failed;
  
  if (!_dbus_string_copy (&homedir, 0,
                          directory, _dbus_string_get_length (directory))) {
    goto failed;
  }

  _dbus_string_free (&homedir);
  return TRUE;
  
 failed: 
  _dbus_string_free (&homedir);
  return FALSE;
}

/** Checks if a file exists
*
* @param file full path to the file
* @returns #TRUE if file exists
*/
dbus_bool_t 
_dbus_file_exists (const char *file)
{
  DWORD attributes = GetFileAttributesA (file);

  if (attributes != INVALID_FILE_ATTRIBUTES && GetLastError() != ERROR_PATH_NOT_FOUND)
    return TRUE;
  else
    return FALSE;  
}

/**
 * A wrapper around strerror() because some platforms
 * may be lame and not have strerror().
 *
 * @param error_number errno.
 * @returns error description.
 */
const char*
_dbus_strerror (int error_number)
{
#ifdef DBUS_WINCE
  // TODO
  return "unknown";
#else
  const char *msg;

  switch (error_number)
    {
    case WSAEINTR:
      return "Interrupted function call";
    case WSAEACCES:
      return "Permission denied";
    case WSAEFAULT:
      return "Bad address";
    case WSAEINVAL:
      return "Invalid argument";
    case WSAEMFILE:
      return "Too many open files";
    case WSAEWOULDBLOCK:
      return "Resource temporarily unavailable";
    case WSAEINPROGRESS:
      return "Operation now in progress";
    case WSAEALREADY:
      return "Operation already in progress";
    case WSAENOTSOCK:
      return "Socket operation on nonsocket";
    case WSAEDESTADDRREQ:
      return "Destination address required";
    case WSAEMSGSIZE:
      return "Message too long";
    case WSAEPROTOTYPE:
      return "Protocol wrong type for socket";
    case WSAENOPROTOOPT:
      return "Bad protocol option";
    case WSAEPROTONOSUPPORT:
      return "Protocol not supported";
    case WSAESOCKTNOSUPPORT:
      return "Socket type not supported";
    case WSAEOPNOTSUPP:
      return "Operation not supported";
    case WSAEPFNOSUPPORT:
      return "Protocol family not supported";
    case WSAEAFNOSUPPORT:
      return "Address family not supported by protocol family";
    case WSAEADDRINUSE:
      return "Address already in use";
    case WSAEADDRNOTAVAIL:
      return "Cannot assign requested address";
    case WSAENETDOWN:
      return "Network is down";
    case WSAENETUNREACH:
      return "Network is unreachable";
    case WSAENETRESET:
      return "Network dropped connection on reset";
    case WSAECONNABORTED:
      return "Software caused connection abort";
    case WSAECONNRESET:
      return "Connection reset by peer";
    case WSAENOBUFS:
      return "No buffer space available";
    case WSAEISCONN:
      return "Socket is already connected";
    case WSAENOTCONN:
      return "Socket is not connected";
    case WSAESHUTDOWN:
      return "Cannot send after socket shutdown";
    case WSAETIMEDOUT:
      return "Connection timed out";
    case WSAECONNREFUSED:
      return "Connection refused";
    case WSAEHOSTDOWN:
      return "Host is down";
    case WSAEHOSTUNREACH:
      return "No route to host";
    case WSAEPROCLIM:
      return "Too many processes";
    case WSAEDISCON:
      return "Graceful shutdown in progress";
    case WSATYPE_NOT_FOUND:
      return "Class type not found";
    case WSAHOST_NOT_FOUND:
      return "Host not found";
    case WSATRY_AGAIN:
      return "Nonauthoritative host not found";
    case WSANO_RECOVERY:
      return "This is a nonrecoverable error";
    case WSANO_DATA:
      return "Valid name, no data record of requested type";
    case WSA_INVALID_HANDLE:
      return "Specified event object handle is invalid";
    case WSA_INVALID_PARAMETER:
      return "One or more parameters are invalid";
    case WSA_IO_INCOMPLETE:
      return "Overlapped I/O event object not in signaled state";
    case WSA_IO_PENDING:
      return "Overlapped operations will complete later";
    case WSA_NOT_ENOUGH_MEMORY:
      return "Insufficient memory available";
    case WSA_OPERATION_ABORTED:
      return "Overlapped operation aborted";
#ifdef WSAINVALIDPROCTABLE

    case WSAINVALIDPROCTABLE:
      return "Invalid procedure table from service provider";
#endif
#ifdef WSAINVALIDPROVIDER

    case WSAINVALIDPROVIDER:
      return "Invalid service provider version number";
#endif
#ifdef WSAPROVIDERFAILEDINIT

    case WSAPROVIDERFAILEDINIT:
      return "Unable to initialize a service provider";
#endif

    case WSASYSCALLFAILURE:
      return "System call failure";
    }
  msg = strerror (error_number);
  if (msg == NULL)
    msg = "unknown";

  return msg;
#endif //DBUS_WINCE
}

/**
 * Assigns an error name and message corresponding to a Win32 error
 * code to a DBusError. Does nothing if error is #NULL.
 *
 * @param error the error.
 * @param code the Win32 error code
 */
void
_dbus_win_set_error_from_win_error (DBusError *error,
                                    int        code)
{
  char *msg;

  /* As we want the English message, use the A API */
  FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_IGNORE_INSERTS |
                  FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, code, MAKELANGID (LANG_ENGLISH, SUBLANG_ENGLISH_US),
                  (LPSTR) &msg, 0, NULL);
  if (msg)
    {
      char *msg_copy;

      msg_copy = dbus_malloc (strlen (msg));
      strcpy (msg_copy, msg);
      LocalFree (msg);

      dbus_set_error (error, "win32.error", "%s", msg_copy);
    }
  else
    dbus_set_error (error, "win32.error", "Unknown error code %d or FormatMessage failed", code);
}

void
_dbus_win_warn_win_error (const char *message,
                          unsigned long code)
{
  DBusError error;

  dbus_error_init (&error);
  _dbus_win_set_error_from_win_error (&error, code);
  _dbus_warn ("%s: %s\n", message, error.message);
  dbus_error_free (&error);
}

/**
 * Removes a directory; Directory must be empty
 *
 * @param filename directory filename
 * @param error initialized error object
 * @returns #TRUE on success
 */
dbus_bool_t
_dbus_delete_directory (const DBusString *filename,
                        DBusError        *error)
{
  const char *filename_c;

  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  filename_c = _dbus_string_get_const_data (filename);

  if (RemoveDirectoryA (filename_c) == 0)
    {
      char *emsg = _dbus_win_error_string (GetLastError ());
      dbus_set_error (error, _dbus_win_error_from_last_error (),
                      "Failed to remove directory %s: %s",
                      filename_c, emsg);
      _dbus_win_free_error_string (emsg);
      return FALSE;
    }

  return TRUE;
}

/**
 * Checks whether the filename is an absolute path
 *
 * @param filename the filename
 * @returns #TRUE if an absolute path
 */
dbus_bool_t
_dbus_path_is_absolute (const DBusString *filename)
{
  if (_dbus_string_get_length (filename) > 0)
    return _dbus_string_get_byte (filename, 1) == ':'
           || _dbus_string_get_byte (filename, 0) == '\\'
           || _dbus_string_get_byte (filename, 0) == '/';
  else
    return FALSE;
}

dbus_bool_t
_dbus_check_setuid (void)
{
  return FALSE;
}

/** @} end of sysdeps-win */
/* tests in dbus-sysdeps-util.c */

