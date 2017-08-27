/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2006-2007 Red Hat, Inc.
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
 * Author: Alexander Larsson <alexl@redhat.com>
 */

#include "config.h"
#include <string.h>
#include "gvfs.h"
#include "glib-private.h"
#include "glocalvfs.h"
#include "gresourcefile.h"
#include "giomodule-priv.h"
#include "glibintl.h"


/**
 * SECTION:gvfs
 * @short_description: Virtual File System
 * @include: gio/gio.h
 *
 * Entry point for using GIO functionality.
 *
 */

G_DEFINE_TYPE (GVfs, g_vfs, G_TYPE_OBJECT);

static void
g_vfs_class_init (GVfsClass *klass)
{
}

static void
g_vfs_init (GVfs *vfs)
{
}

/**
 * g_vfs_is_active:
 * @vfs: a #GVfs.
 *
 * Checks if the VFS is active.
 *
 * Returns: %TRUE if construction of the @vfs was successful
 *     and it is now active.
 */
gboolean
g_vfs_is_active (GVfs *vfs)
{
  GVfsClass *class;

  g_return_val_if_fail (G_IS_VFS (vfs), FALSE);

  class = G_VFS_GET_CLASS (vfs);

  return (* class->is_active) (vfs);
}


/**
 * g_vfs_get_file_for_path:
 * @vfs: a #GVfs.
 * @path: a string containing a VFS path.
 *
 * Gets a #GFile for @path.
 *
 * Returns: (transfer full): a #GFile.
 *     Free the returned object with g_object_unref().
 */
GFile *
g_vfs_get_file_for_path (GVfs       *vfs,
                         const char *path)
{
  GVfsClass *class;
 
  g_return_val_if_fail (G_IS_VFS (vfs), NULL);
  g_return_val_if_fail (path != NULL, NULL);

  class = G_VFS_GET_CLASS (vfs);

  return (* class->get_file_for_path) (vfs, path);
}

/**
 * g_vfs_get_file_for_uri:
 * @vfs: a#GVfs.
 * @uri: a string containing a URI
 *
 * Gets a #GFile for @uri.
 *
 * This operation never fails, but the returned object
 * might not support any I/O operation if the URI
 * is malformed or if the URI scheme is not supported.
 *
 * Returns: (transfer full): a #GFile.
 *     Free the returned object with g_object_unref().
 */
GFile *
g_vfs_get_file_for_uri (GVfs       *vfs,
                        const char *uri)
{
  GVfsClass *class;
 
  g_return_val_if_fail (G_IS_VFS (vfs), NULL);
  g_return_val_if_fail (uri != NULL, NULL);

  class = G_VFS_GET_CLASS (vfs);

  /* This is an unfortunate placement, but we really
   * need to check this before chaining to the vfs,
   * because we want to support resource uris for
   * all vfs:es, even those that predate resources.
   */
  if (g_str_has_prefix (uri, "resource:"))
    return _g_resource_file_new (uri);

  return (* class->get_file_for_uri) (vfs, uri);
}

/**
 * g_vfs_get_supported_uri_schemes:
 * @vfs: a #GVfs.
 *
 * Gets a list of URI schemes supported by @vfs.
 *
 * Returns: (transfer none): a %NULL-terminated array of strings.
 *     The returned array belongs to GIO and must
 *     not be freed or modified.
 */
const gchar * const *
g_vfs_get_supported_uri_schemes (GVfs *vfs)
{
  GVfsClass *class;

  g_return_val_if_fail (G_IS_VFS (vfs), NULL);

  class = G_VFS_GET_CLASS (vfs);

  return (* class->get_supported_uri_schemes) (vfs);
}

/**
 * g_vfs_parse_name:
 * @vfs: a #GVfs.
 * @parse_name: a string to be parsed by the VFS module.
 *
 * This operation never fails, but the returned object might
 * not support any I/O operations if the @parse_name cannot
 * be parsed by the #GVfs module.
 *
 * Returns: (transfer full): a #GFile for the given @parse_name.
 *     Free the returned object with g_object_unref().
 */
GFile *
g_vfs_parse_name (GVfs       *vfs,
                  const char *parse_name)
{
  GVfsClass *class;

  g_return_val_if_fail (G_IS_VFS (vfs), NULL);
  g_return_val_if_fail (parse_name != NULL, NULL);

  class = G_VFS_GET_CLASS (vfs);

  if (g_str_has_prefix (parse_name, "resource:"))
    return _g_resource_file_new (parse_name);

  return (* class->parse_name) (vfs, parse_name);
}

/**
 * g_vfs_get_default:
 *
 * Gets the default #GVfs for the system.
 *
 * Returns: (transfer none): a #GVfs.
 */
GVfs *
g_vfs_get_default (void)
{
  if (GLIB_PRIVATE_CALL (g_check_setuid) ())
    return g_vfs_get_local ();
  return _g_io_module_get_default (G_VFS_EXTENSION_POINT_NAME,
                                   "GIO_USE_VFS",
                                   (GIOModuleVerifyFunc)g_vfs_is_active);
}

/**
 * g_vfs_get_local:
 *
 * Gets the local #GVfs for the system.
 *
 * Returns: (transfer none): a #GVfs.
 */
GVfs *
g_vfs_get_local (void)
{
  static gsize vfs = 0;

  if (g_once_init_enter (&vfs))
    g_once_init_leave (&vfs, (gsize)_g_local_vfs_new ());

  return G_VFS (vfs);
}
