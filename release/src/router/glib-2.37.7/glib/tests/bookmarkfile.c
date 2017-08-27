#undef G_DISABLE_ASSERT

#include <glib.h>
#include <time.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_URI_0 	"file:///abc/defgh/ijklmnopqrstuvwxyz"
#define TEST_URI_1 	"file:///test/uri/1"
#define TEST_URI_2 	"file:///test/uri/2"

#define TEST_MIME 	"text/plain"

#define TEST_APP_NAME 	"bookmarkfile-test"
#define TEST_APP_EXEC 	"bookmarkfile-test %f"

static gboolean
test_load (GBookmarkFile *bookmark,
           const gchar   *filename)
{
  GError *error = NULL;
  gboolean res;
  
  res = g_bookmark_file_load_from_file (bookmark, filename, &error);
  if (error && g_test_verbose ())
    g_print ("Load error: %s\n", error->message);

  g_clear_error (&error);
  return res;
}

static void
test_query (GBookmarkFile *bookmark)
{
  gint size;
  gchar **uris;
  gsize uris_len, i;
  gchar *mime;
  GError *error;

  size = g_bookmark_file_get_size (bookmark);
  uris = g_bookmark_file_get_uris (bookmark, &uris_len);

  g_assert_cmpint (uris_len, ==, size);

  for (i = 0; i < uris_len; i++)
    {
      g_assert (g_bookmark_file_has_item (bookmark, uris[i]));
      error = NULL;
      mime = g_bookmark_file_get_mime_type (bookmark, uris[i], &error);
      g_assert (mime != NULL);
      g_assert_no_error (error);
      g_free (mime);
    }
  g_strfreev (uris);

  g_assert (!g_bookmark_file_has_item (bookmark, "file:///no/such/uri"));
  error = NULL;
  mime = g_bookmark_file_get_mime_type (bookmark, "file:///no/such/uri", &error);
  g_assert (mime == NULL);
  g_assert_error (error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND);
  g_error_free (error);
  g_free (mime);
}

static gboolean
test_modify (GBookmarkFile *bookmark)
{
  gchar *text;
  guint count;
  time_t stamp;
  time_t now;
  GError *error = NULL;
  gchar **groups;
  gsize length;
  gchar **apps;
  gchar *icon;
  gchar *mime;

  if (g_test_verbose ())
    g_print ("\t=> check global title/description...");
  g_bookmark_file_set_title (bookmark, NULL, "a file");
  g_bookmark_file_set_description (bookmark, NULL, "a bookmark file");

  text = g_bookmark_file_get_title (bookmark, NULL, &error);
  g_assert_no_error (error);
  g_assert_cmpstr (text, ==, "a file");
  g_free (text);

  text = g_bookmark_file_get_description (bookmark, NULL, &error);
  g_assert_no_error (error);
  g_assert_cmpstr (text, ==, "a bookmark file");
  g_free (text);
  if (g_test_verbose ())
    g_print ("ok\n");

  if (g_test_verbose ())
    g_print ("\t=> check bookmark title/description...");
  g_bookmark_file_set_title (bookmark, TEST_URI_0, "a title");
  g_bookmark_file_set_description (bookmark, TEST_URI_0, "a description");
  g_bookmark_file_set_is_private (bookmark, TEST_URI_0, TRUE);
  time (&now);
  g_bookmark_file_set_added (bookmark, TEST_URI_0, now);
  g_bookmark_file_set_modified (bookmark, TEST_URI_0, now);
  g_bookmark_file_set_visited (bookmark, TEST_URI_0, now);
  g_bookmark_file_set_icon (bookmark, TEST_URI_0, "testicon", "image/png");

  text = g_bookmark_file_get_title (bookmark, TEST_URI_0, &error);
  g_assert_no_error (error);
  g_assert_cmpstr (text, ==, "a title");
  g_free (text);
  text = g_bookmark_file_get_description (bookmark, TEST_URI_0, &error);
  g_assert_no_error (error);
  g_assert_cmpstr (text, ==, "a description");
  g_free (text);
  g_assert (g_bookmark_file_get_is_private (bookmark, TEST_URI_0, &error));
  g_assert_no_error (error);
  stamp = g_bookmark_file_get_added (bookmark, TEST_URI_0, &error);
  g_assert_no_error (error);
  g_assert (stamp == now);
  stamp = g_bookmark_file_get_modified (bookmark, TEST_URI_0, &error);
  g_assert_no_error (error);
  g_assert (stamp == now);
  stamp = g_bookmark_file_get_visited (bookmark, TEST_URI_0, &error);
  g_assert_no_error (error);
  g_assert (stamp == now);
  g_assert (g_bookmark_file_get_icon (bookmark, TEST_URI_0, &icon, &mime, &error));
  g_assert_no_error (error);
  g_assert_cmpstr (icon, ==, "testicon");
  g_assert_cmpstr (mime, ==, "image/png");
  g_free (icon);
  g_free (mime);
  if (g_test_verbose ())
    g_print ("ok\n");

  if (g_test_verbose ())
    g_print ("\t=> check non existing bookmark...");
  g_bookmark_file_get_description (bookmark, TEST_URI_1, &error);
  g_assert_error (error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND);
  g_clear_error (&error);
  g_bookmark_file_get_is_private (bookmark, TEST_URI_1, &error);
  g_assert_error (error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND);
  g_clear_error (&error);
  g_bookmark_file_get_added (bookmark, TEST_URI_1, &error);
  g_assert_error (error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND);
  g_clear_error (&error);
  g_bookmark_file_get_modified (bookmark, TEST_URI_1, &error);
  g_assert_error (error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND);
  g_clear_error (&error);
  g_bookmark_file_get_visited (bookmark, TEST_URI_1, &error);
  g_assert_error (error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND);
  g_clear_error (&error);
  if (g_test_verbose ())
    g_print ("ok\n");

  if (g_test_verbose ())
    g_print ("\t=> check application...");
  g_bookmark_file_set_mime_type (bookmark, TEST_URI_0, TEST_MIME);
  g_assert (!g_bookmark_file_has_application (bookmark, TEST_URI_0, TEST_APP_NAME, NULL));
  g_bookmark_file_add_application (bookmark, TEST_URI_0,
				   TEST_APP_NAME,
				   TEST_APP_EXEC);
  g_assert (g_bookmark_file_has_application (bookmark, TEST_URI_0, TEST_APP_NAME, NULL));
  g_bookmark_file_get_app_info (bookmark, TEST_URI_0, TEST_APP_NAME,
		  		&text,
				&count,
				&stamp,
				&error);
  g_assert_no_error (error);
  g_assert (count == 1);
  g_assert (stamp == g_bookmark_file_get_modified (bookmark, TEST_URI_0, NULL));
  g_free (text);
  g_assert (g_bookmark_file_remove_application (bookmark, TEST_URI_0, TEST_APP_NAME, &error));
  g_assert_no_error (error);
  g_bookmark_file_add_application (bookmark, TEST_URI_0, TEST_APP_NAME, TEST_APP_EXEC);
  apps = g_bookmark_file_get_applications (bookmark, TEST_URI_0, &length, &error);
  g_assert_no_error (error);
  g_assert_cmpint (length, ==, 1);
  g_assert_cmpstr (apps[0], ==, TEST_APP_NAME);
  g_strfreev (apps);

  g_bookmark_file_get_app_info (bookmark, TEST_URI_0, "fail",
		  		&text,
				&count,
				&stamp,
				&error);
  g_assert_error (error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_APP_NOT_REGISTERED);
  g_clear_error (&error);

  if (g_test_verbose ())
    g_print ("ok\n"); 

  if (g_test_verbose ())
    g_print ("\t=> check groups...");
  g_assert (!g_bookmark_file_has_group (bookmark, TEST_URI_1, "Test", NULL));
  g_bookmark_file_add_group (bookmark, TEST_URI_1, "Test");
  g_assert (g_bookmark_file_has_group (bookmark, TEST_URI_1, "Test", NULL));
  g_assert (!g_bookmark_file_has_group (bookmark, TEST_URI_1, "Fail", NULL));
  g_assert (g_bookmark_file_remove_group (bookmark, TEST_URI_1, "Test", &error));
  g_assert_no_error (error);
  groups = g_bookmark_file_get_groups (bookmark, TEST_URI_1, NULL, &error);
  g_assert_cmpint (g_strv_length (groups), ==, 0);
  g_strfreev (groups);
  groups = g_new0 (gchar *, 3);
  groups[0] = "Group1";
  groups[1] = "Group2";
  groups[2] = NULL;
  g_bookmark_file_set_groups (bookmark, TEST_URI_1, (const gchar **)groups, 2);
  g_free (groups);
  groups = g_bookmark_file_get_groups (bookmark, TEST_URI_1, &length, &error);
  g_assert_cmpint (length, ==, 2);
  g_strfreev (groups);
  g_assert_no_error (error);

  if (g_test_verbose ())
    g_print ("ok\n");

  if (g_test_verbose ())
    g_print ("\t=> check remove...");
  g_assert (g_bookmark_file_remove_item (bookmark, TEST_URI_1, &error) == TRUE);
  g_assert_no_error (error);
  g_assert (g_bookmark_file_remove_item (bookmark, TEST_URI_1, &error) == FALSE);
  g_assert_error (error, G_BOOKMARK_FILE_ERROR, G_BOOKMARK_FILE_ERROR_URI_NOT_FOUND);
  g_clear_error (&error);
  if (g_test_verbose ())
    g_print ("ok\n");
  
  return TRUE;
}

static void
test_file (gconstpointer d)
{
  const gchar *filename = d;
  GBookmarkFile *bookmark_file;
  gboolean success;
  gchar *data;
  GError *error;

  bookmark_file = g_bookmark_file_new ();
  g_assert (bookmark_file != NULL);

  success = test_load (bookmark_file, filename);

  if (success)
    {
      test_query (bookmark_file);
      test_modify (bookmark_file);

      error = NULL;
      data = g_bookmark_file_to_data (bookmark_file, NULL, &error);
      g_assert_no_error (error);
      /* FIXME do some checks on data */
      g_free (data);
    }

  g_bookmark_file_free (bookmark_file);

  g_assert (success == (strstr (filename, "fail") == NULL));
}

int
main (int argc, char *argv[])
{
  GDir *dir;
  GError *error;
  const gchar *name;
  gchar *path;

  g_test_init (&argc, &argv, NULL);

  if (argc > 1)
    {
      test_file (argv[1]);
      return 0;
    }

  error = NULL;
  path = g_test_build_filename (G_TEST_DIST, "bookmarks", NULL);
  dir = g_dir_open (path, 0, &error);
  g_free (path);
  g_assert_no_error (error);
  while ((name = g_dir_read_name (dir)) != NULL)
    {
      path = g_strdup_printf ("/bookmarks/parse/%s", name);
      g_test_add_data_func_full (path, g_test_build_filename (G_TEST_DIST, "bookmarks", name, NULL),
                                 test_file, g_free);
      g_free (path);
    }
  g_dir_close (dir);

  return g_test_run ();
}
