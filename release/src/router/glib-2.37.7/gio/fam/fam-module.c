/* GIO - GLib Input, Output and Streaming Library
 * 
 * Copyright (C) 2006-2007 Red Hat, Inc.
 * Copyright (C) 2007 Sebastian Dröge.
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
 * Authors: Alexander Larsson <alexl@redhat.com>
 *          John McCutchan <john@johnmccutchan.com> 
 *          Sebastian Dröge <slomo@circular-chaos.org>
 */

#include <gio/giomodule.h>
#include "gfamdirectorymonitor.h"
#include "gfamfilemonitor.h"
#include "fam-helper.h"

void
g_io_module_load (GIOModule *module)
{
  g_fam_file_monitor_register (module);
  g_fam_directory_monitor_register (module);
}

void
g_io_module_unload (GIOModule   *module)
{
  _fam_sub_shutdown ();
}

char **
g_io_module_query (void)
{
  char *eps[] = {
    G_LOCAL_DIRECTORY_MONITOR_EXTENSION_POINT_NAME,
    G_LOCAL_FILE_MONITOR_EXTENSION_POINT_NAME,
    G_NFS_DIRECTORY_MONITOR_EXTENSION_POINT_NAME,
    G_NFS_FILE_MONITOR_EXTENSION_POINT_NAME,
    NULL
  };
  return g_strdupv (eps);
}

