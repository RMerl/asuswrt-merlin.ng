/* GLib testing framework examples and tests
 * Copyright (C) 2008 Red Hat, Inc.
 * Authors: Tomas Bzatek <tbzatek@redhat.com>
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

#include <glib/glib.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_NAME			"Prilis zlutoucky kun"
#define TEST_DISPLAY_NAME	        "UTF-8 p\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88"
#define TEST_SIZE			0xFFFFFFF0

static void
test_assigned_values (GFileInfo *info)
{
  const char *name, *display_name, *mistake;
  guint64 size;
  GFileType type;
  
  /*  Test for attributes presence */
  g_assert (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_NAME) == TRUE);
  g_assert (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME) == TRUE);
  g_assert (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_SIZE) == TRUE);
  g_assert (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_COPY_NAME) == FALSE);
	
  /*  Retrieve data back and compare */
  
  name = g_file_info_get_attribute_byte_string (info, G_FILE_ATTRIBUTE_STANDARD_NAME);
  display_name = g_file_info_get_attribute_string (info, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
  mistake = g_file_info_get_attribute_string (info, G_FILE_ATTRIBUTE_STANDARD_COPY_NAME);
  size = g_file_info_get_attribute_uint64 (info, G_FILE_ATTRIBUTE_STANDARD_SIZE);
  type = g_file_info_get_file_type (info);
  
  g_assert_cmpstr (name, ==, TEST_NAME);
  g_assert_cmpstr (display_name, ==, TEST_DISPLAY_NAME);
  g_assert (mistake == NULL);
  g_assert_cmpint (size, ==, TEST_SIZE);
  g_assert_cmpstr (name, ==, g_file_info_get_name (info));
  g_assert_cmpstr (display_name, ==, g_file_info_get_display_name (info));
  g_assert_cmpint (size, ==, g_file_info_get_size (info)	);
  g_assert_cmpint (type, ==, G_FILE_TYPE_DIRECTORY);
}



static void
test_g_file_info (void)
{
  GFileInfo *info;
  GFileInfo *info_dup;
  GFileInfo *info_copy;
  char **attr_list;
  GFileAttributeMatcher *matcher;
  
  info = g_file_info_new ();
  
  /*  Test for empty instance */
  attr_list = g_file_info_list_attributes (info, NULL);
  g_assert (attr_list != NULL);
  g_assert (*attr_list == NULL);
  g_strfreev (attr_list);

  g_file_info_set_attribute_byte_string (info, G_FILE_ATTRIBUTE_STANDARD_NAME, TEST_NAME);
  g_file_info_set_attribute_string (info, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, TEST_DISPLAY_NAME);
  g_file_info_set_attribute_uint64 (info, G_FILE_ATTRIBUTE_STANDARD_SIZE, TEST_SIZE);
  g_file_info_set_file_type (info, G_FILE_TYPE_DIRECTORY);
	
  /*  The attr list should not be empty now */
  attr_list = g_file_info_list_attributes (info, NULL);
  g_assert (attr_list != NULL);
  g_assert (*attr_list != NULL);
  g_strfreev (attr_list);

  test_assigned_values (info);
	
  /*  Test dups */
  info_dup = g_file_info_dup (info);
  g_assert (info_dup != NULL);
  test_assigned_values (info_dup);
  
  info_copy = g_file_info_new ();
  g_file_info_copy_into (info_dup, info_copy);
  g_assert (info_copy != NULL);
  test_assigned_values (info_copy);

  /*  Test remove attribute */
  g_assert (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_SORT_ORDER) == FALSE);
  g_file_info_set_attribute_int32 (info, G_FILE_ATTRIBUTE_STANDARD_SORT_ORDER, 10);
  g_assert (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_SORT_ORDER) == TRUE);

  g_assert (g_file_info_get_attribute_type (info, G_FILE_ATTRIBUTE_STANDARD_SORT_ORDER) == G_FILE_ATTRIBUTE_TYPE_INT32);
  g_assert (g_file_info_get_attribute_status (info, G_FILE_ATTRIBUTE_STANDARD_SORT_ORDER) != G_FILE_ATTRIBUTE_STATUS_ERROR_SETTING);

  g_file_info_remove_attribute (info, G_FILE_ATTRIBUTE_STANDARD_SORT_ORDER);
  g_assert (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_SORT_ORDER) == FALSE);
  g_assert (g_file_info_get_attribute_type (info, G_FILE_ATTRIBUTE_STANDARD_SORT_ORDER) == G_FILE_ATTRIBUTE_TYPE_INVALID);

  matcher = g_file_attribute_matcher_new (G_FILE_ATTRIBUTE_STANDARD_NAME ","
                                          G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);

  g_assert (g_file_attribute_matcher_matches (matcher, G_FILE_ATTRIBUTE_STANDARD_NAME) == TRUE);
  g_assert (g_file_attribute_matcher_matches_only (matcher, G_FILE_ATTRIBUTE_STANDARD_NAME) == FALSE);
  g_assert (g_file_attribute_matcher_matches (matcher, G_FILE_ATTRIBUTE_STANDARD_SIZE) == FALSE);

  g_file_info_set_attribute_mask (info, matcher);
  g_file_attribute_matcher_unref (matcher);

  g_assert (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_SIZE) == FALSE);
  g_assert (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_NAME) == TRUE);

  g_object_unref (info);
  g_object_unref (info_dup);
  g_object_unref (info_copy);
}

int
main (int   argc,
      char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/g-file-info/test_g_file_info", test_g_file_info);
  
  return g_test_run();
}
