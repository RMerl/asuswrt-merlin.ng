/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2018 Igalia S.L.
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
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define MOCK_TYPE_RESOLVER (mock_resolver_get_type())
G_DECLARE_FINAL_TYPE (MockResolver, mock_resolver, MOCK, RESOLVER, GResolver)

MockResolver *mock_resolver_new (void);
void mock_resolver_set_ipv4_delay_ms (MockResolver *self, guint delay_ms);
void mock_resolver_set_ipv4_results (MockResolver *self, GList *results);
void mock_resolver_set_ipv4_error (MockResolver *self, GError *error);
void mock_resolver_set_ipv6_delay_ms (MockResolver *self, guint delay_ms);
void mock_resolver_set_ipv6_results (MockResolver *self, GList *results);
void mock_resolver_set_ipv6_error (MockResolver *self, GError *error);
G_END_DECLS
