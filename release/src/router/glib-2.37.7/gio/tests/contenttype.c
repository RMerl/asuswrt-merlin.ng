#include <gio/gio.h>
#include <string.h>

static void
test_guess (void)
{
  gchar *res;
  gchar *expected;
  gboolean uncertain;
  guchar data[] =
    "[Desktop Entry]\n"
    "Type=Application\n"
    "Name=appinfo-test\n"
    "Exec=./appinfo-test --option\n";

  res = g_content_type_guess ("/etc/", NULL, 0, &uncertain);
  expected = g_content_type_from_mime_type ("inode/directory");
  g_assert (g_content_type_equals (expected, res));
  g_assert (uncertain);
  g_free (res);
  g_free (expected);

  res = g_content_type_guess ("foo.txt", NULL, 0, &uncertain);
  expected = g_content_type_from_mime_type ("text/plain");
  g_assert (g_content_type_equals (expected, res));
  g_assert (!uncertain);
  g_free (res);
  g_free (expected);

  res = g_content_type_guess ("foo.desktop", data, sizeof (data) - 1, &uncertain);
  expected = g_content_type_from_mime_type ("application/x-desktop");
  g_assert (g_content_type_equals (expected, res));
  g_assert (!uncertain);
  g_free (res);
  g_free (expected);

  res = g_content_type_guess ("foo.txt", data, sizeof (data) - 1, &uncertain);
  expected = g_content_type_from_mime_type ("text/plain");
  g_assert (g_content_type_equals (expected, res));
  g_assert (!uncertain);
  g_free (res);
  g_free (expected);

  res = g_content_type_guess ("foo", data, sizeof (data) - 1, &uncertain);
  expected = g_content_type_from_mime_type ("text/plain");
  g_assert (g_content_type_equals (expected, res));
  g_assert (!uncertain);
  g_free (res);
  g_free (expected);

  res = g_content_type_guess (NULL, data, sizeof (data) - 1, &uncertain);
  expected = g_content_type_from_mime_type ("application/x-desktop");
  g_assert (g_content_type_equals (expected, res));
  g_assert (!uncertain);
  g_free (res);
  g_free (expected);

  /* this is potentially ambiguous: it does not match the PO template format,
   * but looks like text so it can't be Powerpoint */
  res = g_content_type_guess ("test.pot", (guchar *)"ABC abc", 7, &uncertain);
  expected = g_content_type_from_mime_type ("text/x-gettext-translation-template");
  g_assert (g_content_type_equals (expected, res));
  g_assert (!uncertain);
  g_free (res);
  g_free (expected);

  res = g_content_type_guess ("test.pot", (guchar *)"msgid \"", 7, &uncertain);
  expected = g_content_type_from_mime_type ("text/x-gettext-translation-template");
  g_assert (g_content_type_equals (expected, res));
  g_assert (!uncertain);
  g_free (res);
  g_free (expected);

  res = g_content_type_guess ("test.pot", (guchar *)"\xCF\xD0\xE0\x11", 4, &uncertain);
  expected = g_content_type_from_mime_type ("application/vnd.ms-powerpoint");
  g_assert (g_content_type_equals (expected, res));
  /* we cannot reliably detect binary powerpoint files as long as there is no
   * defined MIME magic, so do not check uncertain here */
  g_free (res);
  g_free (expected);

  res = g_content_type_guess ("test.otf", (guchar *)"OTTO", 4, &uncertain);
  expected = g_content_type_from_mime_type ("application/x-font-otf");
  g_assert (g_content_type_equals (expected, res));
  g_assert (!uncertain);
  g_free (res);
  g_free (expected);
}

static void
test_unknown (void)
{
  gchar *unknown;
  gchar *str;

  unknown = g_content_type_from_mime_type ("application/octet-stream");
  g_assert (g_content_type_is_unknown (unknown));
  str = g_content_type_get_mime_type (unknown);
  g_assert_cmpstr (str, ==, "application/octet-stream");
  g_free (str);
  g_free (unknown);
}

static void
test_subtype (void)
{
  gchar *plain;
  gchar *xml;

  plain = g_content_type_from_mime_type ("text/plain");
  xml = g_content_type_from_mime_type ("application/xml");

  g_assert (g_content_type_is_a (xml, plain));

  g_free (plain);
  g_free (xml);
}

static gint
find_mime (gconstpointer a, gconstpointer b)
{
  if (g_content_type_equals (a, b))
    return 0;
  return 1;
}

static void
test_list (void)
{
  GList *types;
  gchar *plain;
  gchar *xml;

  plain = g_content_type_from_mime_type ("text/plain");
  xml = g_content_type_from_mime_type ("application/xml");

  types = g_content_types_get_registered ();

  g_assert (g_list_length (types) > 1);

  /* just check that some types are in the list */
  g_assert (g_list_find_custom (types, plain, find_mime) != NULL);
  g_assert (g_list_find_custom (types, xml, find_mime) != NULL);

  g_list_free_full (types, g_free);

  g_free (plain);
  g_free (xml);
}

static void
test_executable (void)
{
  gchar *type;

  type = g_content_type_from_mime_type ("application/x-executable");
  g_assert (g_content_type_can_be_executable (type));
  g_free (type);

  type = g_content_type_from_mime_type ("text/plain");
  g_assert (g_content_type_can_be_executable (type));
  g_free (type);

  type = g_content_type_from_mime_type ("image/png");
  g_assert (!g_content_type_can_be_executable (type));
  g_free (type);
}

static void
test_description (void)
{
  gchar *type;
  gchar *desc;

  type = g_content_type_from_mime_type ("text/plain");
  desc = g_content_type_get_description (type);
  g_assert (desc != NULL);

  g_free (desc);
  g_free (type);
}

static void
test_icon (void)
{
  gchar *type;
  GIcon *icon;

  type = g_content_type_from_mime_type ("text/plain");
  icon = g_content_type_get_icon (type);
  g_assert (G_IS_ICON (icon));
  g_object_unref (icon);
  g_free (type);

  type = g_content_type_from_mime_type ("application/rtf");
  icon = g_content_type_get_icon (type);
  g_assert (G_IS_ICON (icon));
  g_object_unref (icon);
  g_free (type);
}


int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/contenttype/guess", test_guess);
  g_test_add_func ("/contenttype/unknown", test_unknown);
  g_test_add_func ("/contenttype/subtype", test_subtype);
  g_test_add_func ("/contenttype/list", test_list);
  g_test_add_func ("/contenttype/executable", test_executable);
  g_test_add_func ("/contenttype/description", test_description);
  g_test_add_func ("/contenttype/icon", test_icon);

  return g_test_run ();
}
