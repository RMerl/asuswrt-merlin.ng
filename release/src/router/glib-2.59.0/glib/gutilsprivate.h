/*
 * Copyright © 2018 Endless Mobile, Inc.
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
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Philip Withnall <withnall@endlessm.com>
 */

#ifndef __G_UTILS_PRIVATE_H__
#define __G_UTILS_PRIVATE_H__

#include "gtypes.h"

G_BEGIN_DECLS

GLIB_AVAILABLE_IN_2_60
void g_set_user_dirs (const gchar *first_dir_type,
                      ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS

#endif /* __G_UTILS_PRIVATE_H__ */
