/* GLib testing framework examples and tests
 *
 * Copyright (C) 2012 Collabora Ltd. <http://www.collabora.co.uk/>
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
 *
 * Author: Xavier Claessens <xavier.claessens@collabora.co.uk>
 */

#ifndef __SESSION_BUS_H__
#define __SESSION_BUS_H__

#include <gio/gio.h>

G_BEGIN_DECLS

void session_bus_up   (void);
void session_bus_stop (void);
void session_bus_down (void);

G_END_DECLS

#endif /* __SESSION_BUS_H__ */
