/* Unix pipe-to-self. This is a utility module for tests, not a test.
 *
 * Copyright © 2008-2010 Red Hat, Inc.
 * Copyright © 2011 Nokia Corporation
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
 * Author: Simon McVittie <simon.mcvittie@collabora.co.uk>
 */

#ifndef TEST_PIPE_UNIX_H
#define TEST_PIPE_UNIX_H

#include <gio/gio.h>

gboolean test_pipe (GInputStream  **is,
                    GOutputStream **os,
                    GError        **error);

gboolean test_bidi_pipe (GIOStream **left,
                         GIOStream **right,
                         GError    **error);

#endif /* guard */
