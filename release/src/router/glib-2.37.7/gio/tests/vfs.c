
/* Unit tests for GVfs
 * Copyright (C) 2011 Red Hat, Inc
 * Author: Matthias Clasen
 *
 * This work is provided "as is"; redistribution and modification
 * in whole or in part, in any medium, physical or electronic is
 * permitted without restriction.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * In no event shall the authors or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 */

#include <gio/gio.h>

static void
test_local (void)
{
  GVfs *vfs;
  GFile *file;
  gchar **schemes;

  vfs = g_vfs_get_local ();
  g_assert (g_vfs_is_active (vfs));

  file = g_vfs_get_file_for_uri (vfs, "not a good uri");
  g_assert (G_IS_FILE (file));
  g_object_unref (file);

  schemes = (gchar **)g_vfs_get_supported_uri_schemes (vfs);

  g_assert (g_strv_length (schemes) > 0);
  g_assert_cmpstr (schemes[0], ==, "file");
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/gvfs/local", test_local);

  return g_test_run ();
}
