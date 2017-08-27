#include <gio/gunixsocketaddress.h>

static void
test_unix_socket_address_construct (void)
{
  GUnixSocketAddress *a;

  a = g_object_new (G_TYPE_UNIX_SOCKET_ADDRESS, NULL);
  g_assert_cmpint (g_unix_socket_address_get_address_type (a), ==, G_UNIX_SOCKET_ADDRESS_PATH);
  g_object_unref (a);

  /* Try passing some default values for the arguments explicitly and
   * make sure it makes no difference.
   */
  a = g_object_new (G_TYPE_UNIX_SOCKET_ADDRESS, "address-type", G_UNIX_SOCKET_ADDRESS_PATH, NULL);
  g_assert_cmpint (g_unix_socket_address_get_address_type (a), ==, G_UNIX_SOCKET_ADDRESS_PATH);
  g_object_unref (a);

  a = g_object_new (G_TYPE_UNIX_SOCKET_ADDRESS, "abstract", FALSE, NULL);
  g_assert_cmpint (g_unix_socket_address_get_address_type (a), ==, G_UNIX_SOCKET_ADDRESS_PATH);
  g_object_unref (a);

  a = g_object_new (G_TYPE_UNIX_SOCKET_ADDRESS,
                    "abstract", FALSE,
                    "address-type", G_UNIX_SOCKET_ADDRESS_PATH,
                    NULL);
  g_assert_cmpint (g_unix_socket_address_get_address_type (a), ==, G_UNIX_SOCKET_ADDRESS_PATH);
  g_object_unref (a);

  a = g_object_new (G_TYPE_UNIX_SOCKET_ADDRESS,
                    "address-type", G_UNIX_SOCKET_ADDRESS_PATH,
                    "abstract", FALSE,
                    NULL);
  g_assert_cmpint (g_unix_socket_address_get_address_type (a), ==, G_UNIX_SOCKET_ADDRESS_PATH);
  g_object_unref (a);

  /* Try explicitly setting abstract to TRUE */
  a = g_object_new (G_TYPE_UNIX_SOCKET_ADDRESS,
                    "abstract", TRUE,
                    NULL);
  g_assert_cmpint (g_unix_socket_address_get_address_type (a), ==, G_UNIX_SOCKET_ADDRESS_ABSTRACT_PADDED);
  g_object_unref (a);

  /* Try explicitly setting a different kind of address */
  a = g_object_new (G_TYPE_UNIX_SOCKET_ADDRESS,
                    "address-type", G_UNIX_SOCKET_ADDRESS_ANONYMOUS,
                    NULL);
  g_assert_cmpint (g_unix_socket_address_get_address_type (a), ==, G_UNIX_SOCKET_ADDRESS_ANONYMOUS);
  g_object_unref (a);

  /* Now try explicitly setting a different type of address after
   * setting abstract to FALSE.
   */
  a = g_object_new (G_TYPE_UNIX_SOCKET_ADDRESS,
                    "abstract", FALSE,
                    "address-type", G_UNIX_SOCKET_ADDRESS_ANONYMOUS,
                    NULL);
  g_assert_cmpint (g_unix_socket_address_get_address_type (a), ==, G_UNIX_SOCKET_ADDRESS_ANONYMOUS);
  g_object_unref (a);

  /* And the other way around */
  a = g_object_new (G_TYPE_UNIX_SOCKET_ADDRESS,
                    "address-type", G_UNIX_SOCKET_ADDRESS_ANONYMOUS,
                    "abstract", FALSE,
                    NULL);
  g_assert_cmpint (g_unix_socket_address_get_address_type (a), ==, G_UNIX_SOCKET_ADDRESS_ANONYMOUS);
  g_object_unref (a);
}

int
main (int    argc,
      char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/socket/address/unix/construct", test_unix_socket_address_construct);

  return g_test_run ();
}
