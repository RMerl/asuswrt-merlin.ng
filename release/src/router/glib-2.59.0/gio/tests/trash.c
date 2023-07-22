/*
 * Copyright (C) 2018 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * licence, or (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>

#ifndef G_OS_UNIX
#error This is a Unix-specific test
#endif

#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gunixmounts.h>

/* Test that g_file_trash() returns G_IO_ERROR_NOT_SUPPORTED for files on system mounts. */
static void
test_trash_not_supported (void)
{
  GFile *file;
  GFileIOStream *stream;
  GUnixMountEntry *mount;
  GFileInfo *info;
  GError *error = NULL;
  gboolean ret;
  gchar *parent_dirname;
  GStatBuf parent_stat, home_stat;

  g_test_bug ("251");

  /* The test assumes that tmp file is located on system internal mount. */
  file = g_file_new_tmp ("test-trashXXXXXX", &stream, &error);
  parent_dirname = g_path_get_dirname (g_file_peek_path (file));
  g_assert_no_error (error);
  g_assert_cmpint (g_stat (parent_dirname, &parent_stat), ==, 0);
  g_test_message ("File: %s (parent st_dev: %" G_GUINT64_FORMAT ")",
                  g_file_peek_path (file), (guint64) parent_stat.st_dev);

  g_assert_cmpint (g_stat (g_get_home_dir (), &home_stat), ==, 0);
  g_test_message ("Home: %s (st_dev: %" G_GUINT64_FORMAT ")",
                  g_get_home_dir (), (guint64) home_stat.st_dev);

  if (parent_stat.st_dev == home_stat.st_dev)
    {
      g_test_skip ("The file has to be on another filesystem than the home trash to run this test");

      g_free (parent_dirname);
      g_object_unref (stream);
      g_object_unref (file);

      return;
    }

  mount = g_unix_mount_for (g_file_peek_path (file), NULL);
  g_assert_true (mount == NULL || g_unix_mount_is_system_internal (mount));
  g_test_message ("Mount: %s", (mount != NULL) ? g_unix_mount_get_mount_path (mount) : "(null)");
  g_clear_pointer (&mount, g_unix_mount_free);

  /* g_file_trash() shouldn't be supported on system internal mounts,
   * because those are not monitored by gvfsd-trash.
   */
  ret = g_file_trash (file, NULL, &error);
  g_assert_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED);
  g_test_message ("Error: %s", error->message);
  g_assert_false (ret);
  g_clear_error (&error);

  info = g_file_query_info (file,
                            G_FILE_ATTRIBUTE_ACCESS_CAN_TRASH,
                            G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                            NULL,
                            &error);
  g_assert_no_error (error);

  g_assert_false (g_file_info_get_attribute_boolean (info,
                                                     G_FILE_ATTRIBUTE_ACCESS_CAN_TRASH));

  g_io_stream_close (G_IO_STREAM (stream), NULL, &error);
  g_assert_no_error (error);

  g_free (parent_dirname);
  g_object_unref (info);
  g_object_unref (stream);
  g_object_unref (file);
}

/* Test that symlinks are properly expaned when looking for topdir (e.g. for trash folder). */
static void
test_trash_symlinks (void)
{
  GFile *symlink;
  GUnixMountEntry *target_mount, *tmp_mount, *symlink_mount, *target_over_symlink_mount;
  gchar *target, *tmp, *target_over_symlink;
  GError *error = NULL;

  g_test_bug ("1522");

  /* The test assumes that ~/.local always exists. */
  target = g_build_filename (g_get_home_dir (), ".local", NULL);
  target_mount = g_unix_mount_for (target, NULL);
  g_assert_nonnull (target_mount);
  g_test_message ("Target: %s (mount: %s)", target, g_unix_mount_get_mount_path (target_mount));

  tmp = g_dir_make_tmp ("test-trashXXXXXX", &error);
  tmp_mount = g_unix_mount_for (tmp, NULL);
  g_assert_nonnull (tmp_mount);
  g_test_message ("Tmp: %s (mount: %s)", tmp, g_unix_mount_get_mount_path (tmp_mount));

  if (g_unix_mount_compare (target_mount, tmp_mount) == 0)
    {
      g_test_skip ("The tmp has to be on another mount than the home to run this test");

      g_unix_mount_free (tmp_mount);
      g_free (tmp);
      g_unix_mount_free (target_mount);
      g_free (target);

      return;
    }

  symlink = g_file_new_build_filename (tmp, "symlink", NULL);
  g_file_make_symbolic_link (symlink, g_get_home_dir (), NULL, &error);
  g_assert_no_error (error);

  symlink_mount = g_unix_mount_for (g_file_peek_path (symlink), NULL);
  g_assert_nonnull (symlink_mount);
  g_test_message ("Symlink: %s (mount: %s)", g_file_peek_path (symlink), g_unix_mount_get_mount_path (symlink_mount));

  g_assert_cmpint (g_unix_mount_compare (symlink_mount, tmp_mount), ==, 0);

  target_over_symlink = g_build_filename (g_file_peek_path (symlink),
                                          ".local",
                                          NULL);
  target_over_symlink_mount = g_unix_mount_for (target_over_symlink, NULL);
  g_assert_nonnull (symlink_mount);
  g_test_message ("Target over symlink: %s (mount: %s)", target_over_symlink, g_unix_mount_get_mount_path (target_over_symlink_mount));

  g_assert_cmpint (g_unix_mount_compare (target_over_symlink_mount, target_mount), ==, 0);

  g_unix_mount_free (target_over_symlink_mount);
  g_unix_mount_free (symlink_mount);
  g_free (target_over_symlink);
  g_object_unref (symlink);
  g_unix_mount_free (tmp_mount);
  g_free (tmp);
  g_unix_mount_free (target_mount);
  g_free (target);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_bug_base ("https://gitlab.gnome.org/GNOME/glib/issues/");

  g_test_add_func ("/trash/not-supported", test_trash_not_supported);
  g_test_add_func ("/trash/symlinks", test_trash_symlinks);

  return g_test_run ();
}

