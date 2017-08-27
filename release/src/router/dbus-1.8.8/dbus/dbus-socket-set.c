/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * dbus-socket-set.c - used to bolt file descriptors onto a bus
 *
 * Copyright © 2011 Nokia Corporation
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 *
 */

#include <config.h>
#include <dbus/dbus-socket-set.h>

DBusSocketSet *
_dbus_socket_set_new (int size_hint)
{
  DBusSocketSet *ret;

#ifdef DBUS_HAVE_LINUX_EPOLL
  ret = _dbus_socket_set_epoll_new ();

  if (ret != NULL)
    return ret;
#endif

  ret = _dbus_socket_set_poll_new (size_hint);

  if (ret != NULL)
    return ret;

  return NULL;
}
