/*
 * Copyright © 2007, 2008 Ryan Lortie
 * Copyright © 2009, 2010 Codethink Limited
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
 * Author: Ryan Lortie <desrt@desrt.ca>
 */


/* The purpose of this header is to allow certain internal symbols of
 * GVariant to be put under test cases.
 */

#ifndef __G_VARIANT_INTERNAL_H__
#define __G_VARIANT_INTERNAL_H__

/* Hack */
#define __GLIB_H_INSIDE__

#include <glib/gvarianttype.h>
#include <glib/gtypes.h>

#include "gvariant-serialiser.h"
#include "gvarianttypeinfo.h"

#undef __GLIB_H_INSIDE__

GLIB_AVAILABLE_IN_ALL
gboolean                        g_variant_format_string_scan            (const gchar          *string,
                                                                         const gchar          *limit,
                                                                         const gchar         **endptr);

GLIB_AVAILABLE_IN_ALL
GVariantType *                  g_variant_format_string_scan_type       (const gchar          *string,
                                                                         const gchar          *limit,
                                                                         const gchar         **endptr);

/* The maximum number of levels of nested container which this implementation
 * of #GVariant will handle.
 *
 * The limit must be at least 64 + 1, to allow D-Bus messages to be wrapped in
 * a top-level #GVariant. This comes from the D-Bus specification (§(Valid
 * Signatures)), but also seems generally reasonable. #GDBusMessage wraps its
 * payload in a top-level tuple.
 *
 * The limit is actually set to be a lot greater than 64, to allow much greater
 * nesting of values off D-Bus. It cannot be set over around 80000, or the risk
 * of overflowing the stack when parsing type strings becomes too great.
 *
 * Aside from those constraints, the choice of this value is arbitrary. The
 * only restrictions on it from the API are that it has to be greater than 64
 * (due to D-Bus).
*/
#define G_VARIANT_MAX_RECURSION_DEPTH ((gsize) 128)

#endif /* __G_VARIANT_INTERNAL_H__ */
