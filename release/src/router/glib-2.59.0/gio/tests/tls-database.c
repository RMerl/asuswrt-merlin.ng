/* GLib testing framework examples and tests
 *
 * Copyright (C) Matthew Waters <matthew@centricular.com>.
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

#include "config.h"

#include <gio/gio.h>

#include "gtesttlsbackend.h"

static void
set_default_database (void)
{
  GTlsBackend *backend;
  GTlsDatabase *default_db, *file_db, *test_db;
  GError *error = NULL;
  gchar *path;

  backend = g_tls_backend_get_default ();
  g_assert_nonnull (backend);

  default_db = g_tls_backend_get_default_database (backend);
  g_assert_nonnull (default_db);

  path = g_test_build_filename (G_TEST_DIST, "cert-tests", "cert1.pem", NULL);
  file_db = g_tls_file_database_new (path, &error);
  g_assert_no_error (error);
  g_assert_nonnull (file_db);

  /* setting a default database makes get_default_database return that database */
  g_tls_backend_set_default_database (backend, file_db);
  test_db = g_tls_backend_get_default_database (backend);
  g_assert_nonnull (test_db);
  g_assert_true (test_db == file_db);
  g_object_unref (test_db);

  /* setting a NULL default database returns the original default database */
  g_tls_backend_set_default_database (backend, NULL);
  test_db = g_tls_backend_get_default_database (backend);
  g_assert_nonnull (test_db);
  g_assert_true (test_db == default_db);

  g_object_unref (default_db);
  g_object_unref (file_db);
  g_object_unref (test_db);
  g_free (path);
}

int
main (int   argc,
      char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  _g_test_tls_backend_get_type ();

  g_test_add_func ("/tls-backend/set-default-database",
                   set_default_database);

  return g_test_run();
}
