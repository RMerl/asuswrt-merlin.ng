/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2011 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include "gnetworking.h"

/**
 * SECTION:gnetworking
 * @title: gnetworking.h
 * @short_description: System networking includes
 * @include: gio/gnetworking.h
 *
 * The <literal>gnetworking.h</literal> header can be included to get
 * various low-level networking-related system headers, automatically
 * taking care of certain portability issues for you.
 *
 * This can be used, for example, if you want to call setsockopt()
 * on a #GSocket.
 *
 * Note that while WinSock has many of the same APIs as the
 * traditional UNIX socket API, most of them behave at least slightly
 * differently (particularly with respect to error handling). If you
 * want your code to work under both UNIX and Windows, you will need
 * to take these differences into account.
 *
 * Also, under glibc, certain non-portable functions are only visible
 * in the headers if you define <literal>_GNU_SOURCE</literal> before
 * including them. Note that this symbol must be defined before
 * including <emphasis>any</emphasis> headers, or it may not take
 * effect.
 */

/**
 * g_networking_init:
 *
 * Initializes the platform networking libraries (eg, on Windows, this
 * calls WSAStartup()). GLib will call this itself if it is needed, so
 * you only need to call it if you directly call system networking
 * functions (without calling any GLib networking functions first).
 *
 * Since: 2.36
 */
void
g_networking_init (void)
{
#ifdef G_OS_WIN32
  static volatile gsize inited = 0;

  if (g_once_init_enter (&inited))
    {
      WSADATA wsadata;

      if (WSAStartup (MAKEWORD (2, 0), &wsadata) != 0)
	g_error ("Windows Sockets could not be initialized");
      
      g_once_init_leave (&inited, 1);
    }
#endif
}
