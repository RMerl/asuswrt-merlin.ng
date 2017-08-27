/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * dbus-socket-set.h - used to bolt file descriptors onto a bus
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

#ifndef DBUS_SOCKET_SET_H
#define DBUS_SOCKET_SET_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <dbus/dbus.h>

typedef struct {
    int fd;
    unsigned int flags;
} DBusSocketEvent;

typedef struct DBusSocketSet DBusSocketSet;

typedef struct DBusSocketSetClass DBusSocketSetClass;
struct DBusSocketSetClass {
    void            (*free)     (DBusSocketSet   *self);
    dbus_bool_t     (*add)      (DBusSocketSet   *self,
                                 int              fd,
                                 unsigned int     flags,
                                 dbus_bool_t      enabled);
    void            (*remove)   (DBusSocketSet   *self,
                                 int              fd);
    void            (*enable)   (DBusSocketSet   *self,
                                 int              fd,
                                 unsigned int     flags);
    void            (*disable)  (DBusSocketSet   *self,
                                 int              fd);
    int             (*poll)     (DBusSocketSet   *self,
                                 DBusSocketEvent *revents,
                                 int              max_events,
                                 int              timeout_ms);
};

struct DBusSocketSet {
    DBusSocketSetClass *cls;
};

DBusSocketSet *_dbus_socket_set_new           (int               size_hint);

static inline void
_dbus_socket_set_free (DBusSocketSet *self)
{
  (self->cls->free) (self);
}

static inline dbus_bool_t
_dbus_socket_set_add (DBusSocketSet *self,
                      int            fd,
                      unsigned int   flags,
                      dbus_bool_t    enabled)
{
  return (self->cls->add) (self, fd, flags, enabled);
}

static inline void
_dbus_socket_set_remove (DBusSocketSet *self,
                         int            fd)
{
  (self->cls->remove) (self, fd);
}

static inline void
_dbus_socket_set_enable (DBusSocketSet *self,
                         int            fd,
                         unsigned int   flags)
{
  (self->cls->enable) (self, fd, flags);
}

static inline void
_dbus_socket_set_disable (DBusSocketSet *self,
                          int            fd)
{
  (self->cls->disable) (self, fd);
}


static inline int
_dbus_socket_set_poll (DBusSocketSet    *self,
                       DBusSocketEvent  *revents,
                       int               max_events,
                       int               timeout_ms)
{
  return (self->cls->poll) (self, revents, max_events, timeout_ms);
}

/* concrete implementations, not necessarily built on all platforms */

extern DBusSocketSetClass _dbus_socket_set_poll_class;
extern DBusSocketSetClass _dbus_socket_set_epoll_class;

DBusSocketSet *_dbus_socket_set_poll_new  (int  size_hint);
DBusSocketSet *_dbus_socket_set_epoll_new (void);

#endif /* !DOXYGEN_SHOULD_SKIP_THIS */
#endif /* multiple-inclusion guard */
