/*
 * Copyright © 2010 Codethink Limited
 * Copyright © 2011 Canonical Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the licence or (at
 * your option) any later version.
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
 *
 * Authors: Ryan Lortie <desrt@desrt.ca>
 */

#ifndef __G_DBUS_ACTION_GROUP_PRIVATE_H__
#define __G_DBUS_ACTION_GROUP_PRIVATE_H__

#include "gdbusactiongroup.h"

G_BEGIN_DECLS

gboolean
g_dbus_action_group_sync (GDBusActionGroup  *group,
                          GCancellable      *cancellable,
                          GError           **error);

G_END_DECLS

#endif 
