#undef G_DISABLE_ASSERT
#undef G_LOG_DOMAIN

#include <locale.h>
#include <string.h>
#include <stdio.h>
#include <glib.h>

static int depth = 0;
static GString *string;

static void
indent (int extra)
{
  int i = 0;
  while (i < depth)
    {
      g_string_append (string, "  ");
      ++i;
    }
}

static void
start_element_handler  (GMarkupParseContext *context,
                        const gchar         *element_name,
                        const gchar        **attribute_names,
                        const gchar        **attribute_values,
                        gpointer             user_data,
                        GError             **error)
{
  int i;
  
  indent (0);
  g_string_append_printf (string, "ELEMENT '%s'\n", element_name);

  i = 0;
  while (attribute_names[i] != NULL)
    {
      indent (1);

      g_string_append_printf (string, "%s=\"%s\"\n",
                              attribute_names[i],
                              attribute_values[i]);
      
      ++i;
    }
  
  ++depth;
}

static void
end_element_handler (GMarkupParseContext *context,
                     const gchar         *element_name,
                     gpointer             user_data,
                     GError             **error)
{
  --depth;
  indent (0);
  g_string_append_printf (string, "END '%s'\n", element_name);
  }

static void
text_handler (GMarkupParseContext *context,
              const gchar         *text,
              gsize                text_len,
              gpointer             user_data,
              GError             **error)
{
  indent (0);
  g_string_append_printf (string, "TEXT '%.*s'\n", (int)text_len, text);
}


static void
passthrough_handler (GMarkupParseContext *context,
                     const gchar         *passthrough_text,
                     gsize                text_len,
                     gpointer             user_data,
                     GError             **error)
{
  indent (0);

  g_string_append_printf (string, "PASS '%.*s'\n", (int)text_len, passthrough_text);
}

static void
error_handler (GMarkupParseContext *context,
               GError              *error,
               gpointer             user_data)
{
  g_string_append_printf (string, "ERROR %s\n", error->message);
}

static const GMarkupParser parser = {
  start_element_handler,
  end_element_handler,
  text_handler,
  passthrough_handler,
  error_handler
};

static const GMarkupParser silent_parser = {
  NULL,
  NULL,
  NULL,
  NULL,
  error_handler
};

static int
test_in_chunks (const gchar       *contents,
                gint               length,
                gint               chunk_size,
                GMarkupParseFlags  flags)
{
  GMarkupParseContext *context;
  int i = 0;
  
  context = g_markup_parse_context_new (&silent_parser, flags, NULL, NULL);

  while (i < length)
    {
      int this_chunk = MIN (length - i, chunk_size);

      if (!g_markup_parse_context_parse (context,
                                         contents + i,
                                         this_chunk,
                                         NULL))
        {
          g_markup_parse_context_free (context);
          return 1;
        }

      i += this_chunk;
    }
      
  if (!g_markup_parse_context_end_parse (context, NULL))
    {
      g_markup_parse_context_free (context);
      return 1;
    }

  g_markup_parse_context_free (context);

  return 0;
}

/* Load the given @filename and parse it multiple times with different chunking
 * and length handling. All results should be equal. %TRUE is returned if the
 * file was parsed successfully on every attempt; %FALSE if it failed to parse
 * on every attempt. The test aborts if some attempts succeed and some fail. */
static gboolean
test_file (const gchar       *filename,
           GMarkupParseFlags  flags)
{
  gchar *contents = NULL, *contents_unterminated = NULL;
  gsize length_bytes;
  GError *local_error = NULL;
  GMarkupParseContext *context;
  gint line, col;
  guint n_failures = 0;
  guint n_tests = 0;
  const gsize chunk_sizes_bytes[] = { 1, 2, 5, 12, 1024 };
  gsize i;
  GString *first_string = NULL;

  g_file_get_contents (filename, &contents, &length_bytes, &local_error);
  g_assert_no_error (local_error);

  /* Make a copy of the contents with no trailing nul. */
  contents_unterminated = g_malloc (length_bytes);
  if (contents_unterminated != NULL)
    memcpy (contents_unterminated, contents, length_bytes);

  /* Test with nul termination. */
  context = g_markup_parse_context_new (&parser, flags, NULL, NULL);
  g_assert (g_markup_parse_context_get_user_data (context) == NULL);
  g_markup_parse_context_get_position (context, &line, &col);
  g_assert_cmpint (line, ==, 1);
  g_assert_cmpint (col, ==, 1);

  if (!g_markup_parse_context_parse (context, contents, -1, NULL) ||
      !g_markup_parse_context_end_parse (context, NULL))
    n_failures++;
  n_tests++;

  g_markup_parse_context_free (context);

  /* FIXME: Swap out the error string so we only return one copy of it, not
   * @n_tests copies. This should be fixed properly by eliminating the global
   * state in this file. */
  first_string = g_steal_pointer (&string);
  string = g_string_new ("");

  /* With the length specified explicitly and a nul terminator present (since
   * g_file_get_contents() always adds one). */
  if (test_in_chunks (contents, length_bytes, length_bytes, flags) != 0)
    n_failures++;
  n_tests++;

  /* With the length specified explicitly and no nul terminator present. */
  if (test_in_chunks (contents_unterminated, length_bytes, length_bytes, flags) != 0)
    n_failures++;
  n_tests++;

  /* In various sized chunks. */
  for (i = 0; i < G_N_ELEMENTS (chunk_sizes_bytes); i++)
    {
      if (test_in_chunks (contents, length_bytes, chunk_sizes_bytes[i], flags) != 0)
        n_failures++;
      n_tests++;
    }

  g_free (contents);
  g_free (contents_unterminated);

  /* FIXME: Restore the error string. */
  g_string_free (string, TRUE);
  string = g_steal_pointer (&first_string);

  /* We expect the file to either always be parsed successfully, or never be
   * parsed successfully. There’s a bug in GMarkup if it sometimes parses
   * successfully depending on how you chunk or terminate the input. */
  if (n_failures > 0)
    g_assert_cmpint (n_failures, ==, n_tests);

  return (n_failures == 0);
}

static gchar *
get_expected_filename (const gchar       *filename,
                       GMarkupParseFlags  flags)
{
  gchar *f, *p, *expected;

  f = g_strdup (filename);
  p = strstr (f, ".gmarkup");
  if (p)
    *p = 0;
  if (flags == 0)
    expected = g_strconcat (f, ".expected", NULL);
  else if (flags == G_MARKUP_TREAT_CDATA_AS_TEXT)
    expected = g_strconcat (f, ".cdata-as-text", NULL);
  else
    g_assert_not_reached ();

  g_free (f);

  return expected;
}

static void
test_parse (gconstpointer d)
{
  const gchar *filename = d;
  gchar *expected_file;
  gchar *expected;
  gboolean valid_input;
  GError *error = NULL;
  gboolean res;

  valid_input = strstr (filename, "valid") != NULL;
  expected_file = get_expected_filename (filename, 0);

  depth = 0;
  string = g_string_sized_new (0);

  res = test_file (filename, 0);
  g_assert_cmpint (res, ==, valid_input);

  g_file_get_contents (expected_file, &expected, NULL, &error);
  g_assert_no_error (error);
  g_assert_cmpstr (string->str, ==, expected);
  g_free (expected);

  g_string_free (string, TRUE);

  g_free (expected_file);

  expected_file = get_expected_filename (filename, G_MARKUP_TREAT_CDATA_AS_TEXT);
  if (g_file_test (expected_file, G_FILE_TEST_EXISTS))
    {
      depth = 0;
      string = g_string_sized_new (0);

      res = test_file (filename, G_MARKUP_TREAT_CDATA_AS_TEXT);
      g_assert_cmpint (res, ==, valid_input);

      g_file_get_contents (expected_file, &expected, NULL, &error);
      g_assert_no_error (error);
      g_assert_cmpstr (string->str, ==, expected);
      g_free (expected);

      g_string_free (string, TRUE);
    }

  g_free (expected_file);
}

int
main (int argc, char *argv[])
{
  GDir *dir;
  GError *error;
  const gchar *name;
  gchar *path;

  g_setenv ("LC_ALL", "C", TRUE);
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);

  /* allow to easily generate expected output for new test cases */
  if (argc > 1)
    {
      gint arg = 1;
      GMarkupParseFlags flags = 0;

      if (strcmp (argv[1], "--cdata-as-text") == 0)
        {
          flags = G_MARKUP_TREAT_CDATA_AS_TEXT;
          arg = 2;
        }
      string = g_string_sized_new (0);
      test_file (argv[arg], flags);
      g_print ("%s", string->str);
      return 0;
    }

  error = NULL;
  path = g_test_build_filename (G_TEST_DIST, "markups", NULL);
  dir = g_dir_open (path, 0, &error);
  g_free (path);
  g_assert_no_error (error);
  while ((name = g_dir_read_name (dir)) != NULL)
    {
      if (!strstr (name, "gmarkup"))
        continue;

      path = g_strdup_printf ("/markup/parse/%s", name);
      g_test_add_data_func_full (path, g_test_build_filename (G_TEST_DIST, "markups", name, NULL),
                                 test_parse, g_free);
      g_free (path);
    }
  g_dir_close (dir);

  return g_test_run ();
}

