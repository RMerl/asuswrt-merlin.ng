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
test_in_chunks (const gchar *contents,
                gint         length,
                gint         chunk_size)
{
  GMarkupParseContext *context;
  int i = 0;
  
  context = g_markup_parse_context_new (&silent_parser, 0, NULL, NULL);

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

static int
test_file (const gchar *filename)
{
  gchar *contents;
  gsize  length;
  GError *error;
  GMarkupParseContext *context;
  gint line, col;

  error = NULL;
  if (!g_file_get_contents (filename,
                            &contents,
                            &length,
                            &error))
    {
      fprintf (stderr, "%s\n", error->message);
      g_error_free (error);
      return 1;
    }

  context = g_markup_parse_context_new (&parser, 0, NULL, NULL);
  g_assert (g_markup_parse_context_get_user_data (context) == NULL);
  g_markup_parse_context_get_position (context, &line, &col);
  g_assert (line == 1 && col == 1);

  if (!g_markup_parse_context_parse (context, contents, length, NULL))
    {
      g_markup_parse_context_free (context);
      g_free (contents);
      return 1;
    }

  if (!g_markup_parse_context_end_parse (context, NULL))
    {
      g_markup_parse_context_free (context);
      g_free (contents);
      return 1;
    }

  g_markup_parse_context_free (context);

  /* A byte at a time */
  if (test_in_chunks (contents, length, 1) != 0)
    {
      g_free (contents);
      return 1;
    }

  /* 2 bytes */
  if (test_in_chunks (contents, length, 2) != 0)
    {
      g_free (contents);
      return 1;
    }

  /*5 bytes */
  if (test_in_chunks (contents, length, 5) != 0)
    {
      g_free (contents);
      return 1;
    }

  /* 12 bytes */
  if (test_in_chunks (contents, length, 12) != 0)
    {
      g_free (contents);
      return 1;
    }

  /* 1024 bytes */
  if (test_in_chunks (contents, length, 1024) != 0)
    {
      g_free (contents);
      return 1;
    }

  g_free (contents);

  return 0;
}

static gchar *
get_expected_filename (const gchar *filename)
{
  gchar *f, *p, *expected;

  f = g_strdup (filename);
  p = strstr (f, ".gmarkup");
  if (p)
    *p = 0;
  expected = g_strconcat (f, ".expected", NULL);
  g_free (f);

  return expected;
}

static void
test_parse (gconstpointer d)
{
  const gchar *filename = d;
  gchar *expected_file;
  gchar *expected;
  GError *error = NULL;
  gint res;

  depth = 0;
  string = g_string_sized_new (0);

  res = test_file (filename);

  if (strstr (filename, "valid"))
    g_assert_cmpint (res, ==, 0);
  else
    g_assert_cmpint (res, ==, 1);

  expected_file = get_expected_filename (filename);
  g_file_get_contents (expected_file, &expected, NULL, &error);
  g_assert_no_error (error);
  g_assert_cmpstr (string->str, ==, expected);
  g_free (expected);
  g_free (expected_file);

  g_string_free (string, TRUE);
}

int
main (int argc, char *argv[])
{
  GDir *dir;
  GError *error;
  const gchar *name;
  gchar *path;

  g_setenv ("LANG", "en_US.utf-8", TRUE);
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);

  /* allow to easily generate expected output for new test cases */
  if (argc > 1)
    {
      string = g_string_sized_new (0);
      test_file (argv[1]);
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
      if (strstr (name, "expected"))
        continue;

      path = g_strdup_printf ("/markup/parse/%s", name);
      g_test_add_data_func_full (path, g_test_build_filename (G_TEST_DIST, "markups", name, NULL),
                                 test_parse, g_free);
      g_free (path);
    }
  g_dir_close (dir);

  return g_test_run ();
}

