#include "config.h"

#include <gio/gio.h>
#include <gio/gnetworking.h>

static void
test_basic (void)
{
  GNetworkAddress *address;
  guint port;
  gchar *hostname;
  gchar *scheme;

  address = (GNetworkAddress*)g_network_address_new ("www.gnome.org", 8080);

  g_assert_cmpstr (g_network_address_get_hostname (address), ==, "www.gnome.org");
  g_assert_cmpint (g_network_address_get_port (address), ==, 8080);

  g_object_get (address, "hostname", &hostname, "port", &port, "scheme", &scheme, NULL);
  g_assert_cmpstr (hostname, ==, "www.gnome.org");
  g_assert_cmpint (port, ==, 8080);
  g_assert (scheme == NULL);
  g_free (hostname);

  g_object_unref (address);
}

typedef struct {
  const gchar *input;
  const gchar *scheme;
  const gchar *hostname;
  guint16 port;
  gint error_code;
} ParseTest;

static ParseTest uri_tests[] = {
  { "http://www.gnome.org:2020/start", "http", "www.gnome.org", 2020, -1 },
  { "ftp://joe~:(*)%46@ftp.gnome.org:2020/start", "ftp", "ftp.gnome.org", 2020, -1 },
  { "ftp://[fec0::abcd]/start", "ftp", "fec0::abcd", 8080, -1 },
  { "ftp://[fec0::abcd]:999/start", "ftp", "fec0::abcd", 999, -1 },
  { "ftp://joe%x-@ftp.gnome.org:2020/start", NULL, NULL, 0, G_IO_ERROR_INVALID_ARGUMENT },
  { "http://[fec0::abcd%em1]/start", "http", "fec0::abcd%em1", 8080, -1 },
  { "http://[fec0::abcd%25em1]/start", "http", "fec0::abcd%em1", 8080, -1 },
  { "http://[fec0::abcd%10]/start", "http", "fec0::abcd%10", 8080, -1 },
  { "http://[fec0::abcd%25em%31]/start", NULL, NULL, 0, G_IO_ERROR_INVALID_ARGUMENT }
};

static void
test_parse_uri (gconstpointer d)
{
  const ParseTest *test = d;
  GNetworkAddress *address;
  GError *error;

  error = NULL;
  address = (GNetworkAddress*)g_network_address_parse_uri (test->input, 8080, &error);

  if (address)
    {
      g_assert_cmpstr (g_network_address_get_scheme (address), ==, test->scheme);
      g_assert_cmpstr (g_network_address_get_hostname (address), ==, test->hostname);
      g_assert_cmpint (g_network_address_get_port (address), ==, test->port);
      g_assert_no_error (error);
    }
  else
    g_assert_error (error, G_IO_ERROR, test->error_code);

  if (address)
    g_object_unref (address);
  if (error)
    g_error_free (error);
}

static ParseTest host_tests[] =
{
  { "www.gnome.org", NULL, "www.gnome.org", 1234, -1 },
  { "www.gnome.org:8080", NULL, "www.gnome.org", 8080, -1 },
  { "[2001:db8::1]", NULL, "2001:db8::1", 1234, -1 },
  { "[2001:db8::1]:888", NULL, "2001:db8::1", 888, -1 },
  { "[2001:db8::1%em1]", NULL, "2001:db8::1%em1", 1234, -1 },
  { "[hostname", NULL, NULL, 0, G_IO_ERROR_INVALID_ARGUMENT },
  { "[hostnam]e", NULL, NULL, 0, G_IO_ERROR_INVALID_ARGUMENT },
  { "hostname:", NULL, NULL, 0, G_IO_ERROR_INVALID_ARGUMENT },
  { "hostname:-1", NULL, NULL, 0, G_IO_ERROR_INVALID_ARGUMENT },
  { "hostname:9999999", NULL, NULL, 0, G_IO_ERROR_INVALID_ARGUMENT }
};

static void
test_parse_host (gconstpointer d)
{
  const ParseTest *test = d;
  GNetworkAddress *address;
  GError *error;

  error = NULL;
  address = (GNetworkAddress*)g_network_address_parse (test->input, 1234, &error);

  if (address)
    {
      g_assert_null (g_network_address_get_scheme (address));
      g_assert_cmpstr (g_network_address_get_hostname (address), ==, test->hostname);
      g_assert_cmpint (g_network_address_get_port (address), ==, test->port);
      g_assert_no_error (error);
    }
  else
    {
      g_assert_error (error, G_IO_ERROR, test->error_code);
    }

  if (address)
    g_object_unref (address);
  if (error)
    g_error_free (error);
}

#define SCOPE_ID_TEST_ADDR "fe80::42"
#define SCOPE_ID_TEST_PORT 99

#ifdef HAVE_IF_INDEXTONAME
static char SCOPE_ID_TEST_IFNAME[IF_NAMESIZE];
static int SCOPE_ID_TEST_INDEX;
#else
#define SCOPE_ID_TEST_IFNAME "1"
#define SCOPE_ID_TEST_IFINDEX 1
#endif

static void
find_ifname_and_index (void)
{
  if (SCOPE_ID_TEST_INDEX != 0)
    return;

#ifdef HAVE_IF_INDEXTONAME
  for (SCOPE_ID_TEST_INDEX = 1; SCOPE_ID_TEST_INDEX < 255; SCOPE_ID_TEST_INDEX++) {
    if (if_indextoname (SCOPE_ID_TEST_INDEX, SCOPE_ID_TEST_IFNAME))
      break;
  }
  g_assert_cmpstr (SCOPE_ID_TEST_IFNAME, !=, "");
#endif
}

static void
test_scope_id (GSocketConnectable *addr)
{
  GSocketAddressEnumerator *addr_enum;
  GSocketAddress *saddr;
  GInetSocketAddress *isaddr;
  GInetAddress *iaddr;
  char *tostring;
  GError *error = NULL;

  addr_enum = g_socket_connectable_enumerate (addr);
  saddr = g_socket_address_enumerator_next (addr_enum, NULL, &error);
  g_assert_no_error (error);

  g_assert (saddr != NULL);
  g_assert (G_IS_INET_SOCKET_ADDRESS (saddr));

  isaddr = G_INET_SOCKET_ADDRESS (saddr);
  g_assert_cmpint (g_inet_socket_address_get_scope_id (isaddr), ==, SCOPE_ID_TEST_INDEX);
  g_assert_cmpint (g_inet_socket_address_get_port (isaddr), ==, SCOPE_ID_TEST_PORT);

  iaddr = g_inet_socket_address_get_address (isaddr);
  tostring = g_inet_address_to_string (iaddr);
  g_assert_cmpstr (tostring, ==, SCOPE_ID_TEST_ADDR);
  g_free (tostring);

  g_object_unref (saddr);
  saddr = g_socket_address_enumerator_next (addr_enum, NULL, &error);
  g_assert_no_error (error);
  g_assert (saddr == NULL);

  g_object_unref (addr_enum);
}

static void
test_host_scope_id (void)
{
  GSocketConnectable *addr;
  char *str;

  find_ifname_and_index ();

  str = g_strdup_printf ("%s%%%s", SCOPE_ID_TEST_ADDR, SCOPE_ID_TEST_IFNAME);
  addr = g_network_address_new (str, SCOPE_ID_TEST_PORT);
  g_free (str);

  test_scope_id (addr);
  g_object_unref (addr);
}

static void
test_uri_scope_id (void)
{
  GSocketConnectable *addr;
  char *uri;
  GError *error = NULL;

  find_ifname_and_index ();

  uri = g_strdup_printf ("http://[%s%%%s]:%d/foo",
                         SCOPE_ID_TEST_ADDR,
                         SCOPE_ID_TEST_IFNAME,
                         SCOPE_ID_TEST_PORT);
  addr = g_network_address_parse_uri (uri, 0, &error);
  g_free (uri);
  g_assert_no_error (error);

  test_scope_id (addr);
  g_object_unref (addr);

  uri = g_strdup_printf ("http://[%s%%25%s]:%d/foo",
                         SCOPE_ID_TEST_ADDR,
                         SCOPE_ID_TEST_IFNAME,
                         SCOPE_ID_TEST_PORT);
  addr = g_network_address_parse_uri (uri, 0, &error);
  g_free (uri);
  g_assert_no_error (error);

  test_scope_id (addr);
  g_object_unref (addr);
}

int
main (int argc, char *argv[])
{
  gint i;
  gchar *path;

  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/network-address/basic", test_basic);

  for (i = 0; i < G_N_ELEMENTS (host_tests); i++)
    {
      path = g_strdup_printf ("/network-address/parse-host/%d", i);
      g_test_add_data_func (path, &host_tests[i], test_parse_host);
      g_free (path);
    }

  for (i = 0; i < G_N_ELEMENTS (uri_tests); i++)
    {
      path = g_strdup_printf ("/network-address/parse-uri/%d", i);
      g_test_add_data_func (path, &uri_tests[i], test_parse_uri);
      g_free (path);
    }

  g_test_add_func ("/network-address/scope-id", test_host_scope_id);
  g_test_add_func ("/network-address/uri-scope-id", test_uri_scope_id);

  return g_test_run ();
}
