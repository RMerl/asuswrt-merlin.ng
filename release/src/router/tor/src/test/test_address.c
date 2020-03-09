/* Copyright (c) 2014-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define ADDRESS_PRIVATE

#include "orconfig.h"

#ifdef _WIN32
#include <winsock2.h>
/* For access to structs needed by GetAdaptersAddresses */
#include <iphlpapi.h>
#endif

#ifdef HAVE_IFADDRS_TO_SMARTLIST
#include <net/if.h>
#include <ifaddrs.h>
#endif

#ifdef HAVE_IFCONF_TO_SMARTLIST
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#include <net/if.h>
#endif /* defined(HAVE_IFCONF_TO_SMARTLIST) */

#include "core/or/or.h"
#include "app/config/config.h"
#include "feature/dirauth/process_descs.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/nodelist.h"
#include "lib/net/address.h"
#include "test/test.h"
#include "test/log_test_helpers.h"

/** Return 1 iff <b>sockaddr1</b> and <b>sockaddr2</b> represent
 * the same IP address and port combination. Otherwise, return 0.
 */
static uint8_t
sockaddr_in_are_equal(struct sockaddr_in *sockaddr1,
                      struct sockaddr_in *sockaddr2)
{
   return ((sockaddr1->sin_family == sockaddr2->sin_family) &&
           (sockaddr1->sin_port == sockaddr2->sin_port) &&
           (sockaddr1->sin_addr.s_addr == sockaddr2->sin_addr.s_addr));
}

/** Return 1 iff <b>sockaddr1</b> and <b>sockaddr2</b> represent
 * the same IP address and port combination. Otherwise, return 0.
 */
static uint8_t
sockaddr_in6_are_equal(struct sockaddr_in6 *sockaddr1,
                       struct sockaddr_in6 *sockaddr2)
{
   return ((sockaddr1->sin6_family == sockaddr2->sin6_family) &&
           (sockaddr1->sin6_port == sockaddr2->sin6_port) &&
           (tor_memeq(sockaddr1->sin6_addr.s6_addr,
                      sockaddr2->sin6_addr.s6_addr,16)));
}

/** Create a sockaddr_in structure from IP address string <b>ip_str</b>.
 *
 * If <b>out</b> is not NULL, write the result
 * to the memory address in <b>out</b>. Otherwise, allocate the memory
 * for result. On success, return pointer to result. Otherwise, return
 * NULL.
 */
static struct sockaddr_in *
sockaddr_in_from_string(const char *ip_str, struct sockaddr_in *out)
{
  // [FIXME: add some error checking?]
  if (!out)
    out = tor_malloc_zero(sizeof(struct sockaddr_in));

  out->sin_family = AF_INET;
  out->sin_port = 0;
  tor_inet_pton(AF_INET,ip_str,&(out->sin_addr));

  return out;
}

/** Return 1 iff <b>smartlist</b> contains a tor_addr_t structure
 * that is an IPv4 or IPv6 localhost address. Otherwise, return 0.
 */
static int
smartlist_contains_localhost_tor_addr(smartlist_t *smartlist)
{
  SMARTLIST_FOREACH_BEGIN(smartlist, tor_addr_t *, tor_addr) {
    if (tor_addr_is_loopback(tor_addr)) {
      return 1;
    }
  } SMARTLIST_FOREACH_END(tor_addr);

  return 0;
}

/** Return 1 iff <b>smartlist</b> contains a tor_addr_t structure
 * that is an IPv4 or IPv6 multicast address. Otherwise, return 0.
 */
static int
smartlist_contains_multicast_tor_addr(smartlist_t *smartlist)
{
  SMARTLIST_FOREACH_BEGIN(smartlist, tor_addr_t *, tor_addr) {
    if (tor_addr_is_multicast(tor_addr)) {
      return 1;
    }
  } SMARTLIST_FOREACH_END(tor_addr);

  return 0;
}

/** Return 1 iff <b>smartlist</b> contains a tor_addr_t structure
 * that is an IPv4 or IPv6 internal address. Otherwise, return 0.
 */
static int
smartlist_contains_internal_tor_addr(smartlist_t *smartlist)
{
  SMARTLIST_FOREACH_BEGIN(smartlist, tor_addr_t *, tor_addr) {
    if (tor_addr_is_internal(tor_addr, 0)) {
      return 1;
    }
  } SMARTLIST_FOREACH_END(tor_addr);

  return 0;
}

/** Return 1 iff <b>smartlist</b> contains a tor_addr_t structure
 * that is NULL or the null tor_addr_t. Otherwise, return 0.
 */
static int
smartlist_contains_null_tor_addr(smartlist_t *smartlist)
{
  SMARTLIST_FOREACH_BEGIN(smartlist, tor_addr_t *, tor_addr) {
    if (tor_addr == NULL || tor_addr_is_null(tor_addr)) {
      return 1;
    }
  } SMARTLIST_FOREACH_END(tor_addr);

  return 0;
}

/** Return 1 iff <b>smartlist</b> contains a tor_addr_t structure
 * that is an IPv4 address. Otherwise, return 0.
 */
static int
smartlist_contains_ipv4_tor_addr(smartlist_t *smartlist)
{
  SMARTLIST_FOREACH_BEGIN(smartlist, tor_addr_t *, tor_addr) {
    if (tor_addr_is_v4(tor_addr)) {
      return 1;
    }
  } SMARTLIST_FOREACH_END(tor_addr);

  return 0;
}

/** Return 1 iff <b>smartlist</b> contains a tor_addr_t structure
 * that is an IPv6 address. Otherwise, return 0.
 */
static int
smartlist_contains_ipv6_tor_addr(smartlist_t *smartlist)
{
  SMARTLIST_FOREACH_BEGIN(smartlist, tor_addr_t *, tor_addr) {
    /* Since there's no tor_addr_is_v6, assume all non-v4s are v6 */
    if (!tor_addr_is_v4(tor_addr)) {
      return 1;
    }
  } SMARTLIST_FOREACH_END(tor_addr);

  return 0;
}

#ifdef HAVE_IFADDRS_TO_SMARTLIST
static void
test_address_ifaddrs_to_smartlist(void *arg)
{
   struct ifaddrs *ifa = NULL;
   struct ifaddrs *ifa_ipv4 = NULL;
   struct ifaddrs *ifa_ipv6 = NULL;
   struct sockaddr_in *ipv4_sockaddr_local = NULL;
   struct sockaddr_in *netmask_slash8 = NULL;
   struct sockaddr_in *ipv4_sockaddr_remote = NULL;
   struct sockaddr_in6 *ipv6_sockaddr = NULL;
   smartlist_t *smartlist = NULL;
   tor_addr_t *tor_addr = NULL;
   struct sockaddr *sockaddr_to_check = NULL;
   socklen_t addr_len;

   (void)arg;

   netmask_slash8 = sockaddr_in_from_string("255.0.0.0",NULL);
   ipv4_sockaddr_local = sockaddr_in_from_string("127.0.0.1",NULL);
   ipv4_sockaddr_remote = sockaddr_in_from_string("128.52.160.20",NULL);

   ipv6_sockaddr = tor_malloc(sizeof(struct sockaddr_in6));
   ipv6_sockaddr->sin6_family = AF_INET6;
   ipv6_sockaddr->sin6_port = 0;
   tor_inet_pton(AF_INET6, "2001:db8:8714:3a90::12",
                 &(ipv6_sockaddr->sin6_addr));

   ifa = tor_malloc(sizeof(struct ifaddrs));
   ifa_ipv4 = tor_malloc(sizeof(struct ifaddrs));
   ifa_ipv6 = tor_malloc(sizeof(struct ifaddrs));

   ifa->ifa_next = ifa_ipv4;
   ifa->ifa_name = tor_strdup("eth0");
   ifa->ifa_flags = IFF_UP | IFF_RUNNING;
   ifa->ifa_addr = (struct sockaddr *)ipv4_sockaddr_local;
   ifa->ifa_netmask = (struct sockaddr *)netmask_slash8;
   ifa->ifa_dstaddr = NULL;
   ifa->ifa_data = NULL;

   ifa_ipv4->ifa_next = ifa_ipv6;
   ifa_ipv4->ifa_name = tor_strdup("eth1");
   ifa_ipv4->ifa_flags = IFF_UP | IFF_RUNNING;
   ifa_ipv4->ifa_addr = (struct sockaddr *)ipv4_sockaddr_remote;
   ifa_ipv4->ifa_netmask = (struct sockaddr *)netmask_slash8;
   ifa_ipv4->ifa_dstaddr = NULL;
   ifa_ipv4->ifa_data = NULL;

   ifa_ipv6->ifa_next = NULL;
   ifa_ipv6->ifa_name = tor_strdup("eth2");
   ifa_ipv6->ifa_flags = IFF_UP | IFF_RUNNING;
   ifa_ipv6->ifa_addr = (struct sockaddr *)ipv6_sockaddr;
   ifa_ipv6->ifa_netmask = NULL;
   ifa_ipv6->ifa_dstaddr = NULL;
   ifa_ipv6->ifa_data = NULL;

   smartlist = ifaddrs_to_smartlist(ifa, AF_UNSPEC);

   tt_assert(smartlist);
   tt_int_op(smartlist_len(smartlist), OP_EQ, 3);

   sockaddr_to_check = tor_malloc(sizeof(struct sockaddr_in6));

   tor_addr = smartlist_get(smartlist,0);
   addr_len =
   tor_addr_to_sockaddr(tor_addr,0,sockaddr_to_check,
                        sizeof(struct sockaddr_in));

   tt_int_op(addr_len,OP_EQ,sizeof(struct sockaddr_in));
   tt_assert(sockaddr_in_are_equal((struct sockaddr_in *)sockaddr_to_check,
                                   ipv4_sockaddr_local));

   tor_addr = smartlist_get(smartlist,1);
   addr_len =
   tor_addr_to_sockaddr(tor_addr,0,sockaddr_to_check,
                        sizeof(struct sockaddr_in));

   tt_int_op(addr_len,OP_EQ,sizeof(struct sockaddr_in));
   tt_assert(sockaddr_in_are_equal((struct sockaddr_in *)sockaddr_to_check,
                                   ipv4_sockaddr_remote));

   tor_addr = smartlist_get(smartlist,2);
   addr_len =
   tor_addr_to_sockaddr(tor_addr,0,sockaddr_to_check,
                        sizeof(struct sockaddr_in6));

   tt_int_op(addr_len,OP_EQ,sizeof(struct sockaddr_in6));
   tt_assert(sockaddr_in6_are_equal((struct sockaddr_in6*)sockaddr_to_check,
                                    ipv6_sockaddr));

   done:
   tor_free(netmask_slash8);
   tor_free(ipv4_sockaddr_local);
   tor_free(ipv4_sockaddr_remote);
   tor_free(ipv6_sockaddr);
   tor_free(ifa->ifa_name);
   tor_free(ifa_ipv4->ifa_name);
   tor_free(ifa_ipv6->ifa_name);
   tor_free(ifa);
   tor_free(ifa_ipv4);
   tor_free(ifa_ipv6);
   tor_free(sockaddr_to_check);
   if (smartlist) {
     SMARTLIST_FOREACH(smartlist, tor_addr_t *, t, tor_free(t));
     smartlist_free(smartlist);
   }
   return;
}

static void
test_address_get_if_addrs_ifaddrs(void *arg)
{

  smartlist_t *results = NULL;

  (void)arg;

  results = get_interface_addresses_ifaddrs(LOG_ERR, AF_UNSPEC);

  tt_assert(results);
  /* Some FreeBSD jails don't have localhost IP address. Instead, they only
   * have the address assigned to the jail (whatever that may be).
   * And a jail without a network connection might not have any addresses at
   * all. */
  tt_assert(!smartlist_contains_null_tor_addr(results));

  /* If there are addresses, they must be IPv4 or IPv6 */
  if (smartlist_len(results) > 0) {
    tt_assert(smartlist_contains_ipv4_tor_addr(results)
              || smartlist_contains_ipv6_tor_addr(results));
  }

  done:
  if (results) {
    SMARTLIST_FOREACH(results, tor_addr_t *, t, tor_free(t));
  }
  smartlist_free(results);
  return;
}

#endif /* defined(HAVE_IFADDRS_TO_SMARTLIST) */

#ifdef HAVE_IP_ADAPTER_TO_SMARTLIST

static void
test_address_get_if_addrs_win32(void *arg)
{

  smartlist_t *results = NULL;

  (void)arg;

  results = get_interface_addresses_win32(LOG_ERR, AF_UNSPEC);

  tt_int_op(smartlist_len(results),OP_GE,1);
  tt_assert(smartlist_contains_localhost_tor_addr(results));
  tt_assert(!smartlist_contains_null_tor_addr(results));

  /* If there are addresses, they must be IPv4 or IPv6 */
  if (smartlist_len(results) > 0) {
    tt_assert(smartlist_contains_ipv4_tor_addr(results)
              || smartlist_contains_ipv6_tor_addr(results));
  }

  done:
  SMARTLIST_FOREACH(results, tor_addr_t *, t, tor_free(t));
  tor_free(results);
  return;
}

static void
test_address_ip_adapter_addresses_to_smartlist(void *arg)
{

  IP_ADAPTER_ADDRESSES *addrs1;
  IP_ADAPTER_ADDRESSES *addrs2;

  IP_ADAPTER_UNICAST_ADDRESS *unicast11;
  IP_ADAPTER_UNICAST_ADDRESS *unicast12;
  IP_ADAPTER_UNICAST_ADDRESS *unicast21;

  smartlist_t *result = NULL;

  struct sockaddr_in *sockaddr_test1;
  struct sockaddr_in *sockaddr_test2;
  struct sockaddr_in *sockaddr_localhost;
  struct sockaddr_in *sockaddr_to_check;

  tor_addr_t *tor_addr;

  (void)arg;
  (void)sockaddr_in6_are_equal;

  sockaddr_to_check = tor_malloc_zero(sizeof(struct sockaddr_in));

  addrs1 =
  tor_malloc_zero(sizeof(IP_ADAPTER_ADDRESSES));

  addrs1->FirstUnicastAddress =
  unicast11 = tor_malloc_zero(sizeof(IP_ADAPTER_UNICAST_ADDRESS));
  sockaddr_test1 = sockaddr_in_from_string("86.59.30.40",NULL);
  unicast11->Address.lpSockaddr = (LPSOCKADDR)sockaddr_test1;

  unicast11->Next = unicast12 =
  tor_malloc_zero(sizeof(IP_ADAPTER_UNICAST_ADDRESS));
  sockaddr_test2 = sockaddr_in_from_string("93.95.227.222", NULL);
  unicast12->Address.lpSockaddr = (LPSOCKADDR)sockaddr_test2;

  addrs1->Next = addrs2 =
  tor_malloc_zero(sizeof(IP_ADAPTER_ADDRESSES));

  addrs2->FirstUnicastAddress =
  unicast21 = tor_malloc_zero(sizeof(IP_ADAPTER_UNICAST_ADDRESS));
  sockaddr_localhost = sockaddr_in_from_string("127.0.0.1", NULL);
  unicast21->Address.lpSockaddr = (LPSOCKADDR)sockaddr_localhost;

  result = ip_adapter_addresses_to_smartlist(addrs1);

  tt_assert(result);
  tt_int_op(smartlist_len(result), OP_EQ, 3);

  tor_addr = smartlist_get(result,0);

  tor_addr_to_sockaddr(tor_addr,0,(struct sockaddr *)sockaddr_to_check,
                       sizeof(struct sockaddr_in));

  tt_assert(sockaddr_in_are_equal(sockaddr_test1,sockaddr_to_check));

  tor_addr = smartlist_get(result,1);

  tor_addr_to_sockaddr(tor_addr,0,(struct sockaddr *)sockaddr_to_check,
                       sizeof(struct sockaddr_in));

  tt_assert(sockaddr_in_are_equal(sockaddr_test2,sockaddr_to_check));

  tor_addr = smartlist_get(result,2);

  tor_addr_to_sockaddr(tor_addr,0,(struct sockaddr *)sockaddr_to_check,
                       sizeof(struct sockaddr_in));

  tt_assert(sockaddr_in_are_equal(sockaddr_localhost,sockaddr_to_check));

  done:
  SMARTLIST_FOREACH(result, tor_addr_t *, t, tor_free(t));
  smartlist_free(result);
  tor_free(addrs1);
  tor_free(addrs2);
  tor_free(unicast11->Address.lpSockaddr);
  tor_free(unicast11);
  tor_free(unicast12->Address.lpSockaddr);
  tor_free(unicast12);
  tor_free(unicast21->Address.lpSockaddr);
  tor_free(unicast21);
  tor_free(sockaddr_to_check);
  return;
}
#endif /* defined(HAVE_IP_ADAPTER_TO_SMARTLIST) */

#ifdef HAVE_IFCONF_TO_SMARTLIST

static void
test_address_ifreq_to_smartlist(void *arg)
{
  smartlist_t *results = NULL;
  const tor_addr_t *tor_addr = NULL;
  struct sockaddr_in *sockaddr = NULL;
  struct sockaddr_in *sockaddr_eth1 = NULL;
  struct sockaddr_in *sockaddr_to_check = NULL;

  struct ifconf *ifc;
  struct ifreq *ifr;
  struct ifreq *ifr_next;

  socklen_t addr_len;

  (void)arg;

  sockaddr_to_check = tor_malloc(sizeof(struct sockaddr_in));

  ifr = tor_malloc(sizeof(struct ifreq));
  memset(ifr,0,sizeof(struct ifreq));
  strlcpy(ifr->ifr_name,"lo",3);
  sockaddr = (struct sockaddr_in *) &(ifr->ifr_ifru.ifru_addr);
  sockaddr_in_from_string("127.0.0.1",sockaddr);

  ifc = tor_malloc(sizeof(struct ifconf));
  memset(ifc,0,sizeof(struct ifconf));
  ifc->ifc_len = sizeof(struct ifreq);
  ifc->ifc_ifcu.ifcu_req = ifr;

  results = ifreq_to_smartlist(ifc->ifc_buf,ifc->ifc_len);
  tt_int_op(smartlist_len(results),OP_EQ,1);

  tor_addr = smartlist_get(results, 0);
  addr_len =
  tor_addr_to_sockaddr(tor_addr,0,(struct sockaddr *)sockaddr_to_check,
                       sizeof(struct sockaddr_in));

  tt_int_op(addr_len,OP_EQ,sizeof(struct sockaddr_in));
  tt_assert(sockaddr_in_are_equal(sockaddr,sockaddr_to_check));

  ifr = tor_realloc(ifr,2*sizeof(struct ifreq));
  ifr_next = ifr+1;
  strlcpy(ifr_next->ifr_name,"eth1",5);
  ifc->ifc_len = 2*sizeof(struct ifreq);
  ifc->ifc_ifcu.ifcu_req = ifr;
  sockaddr = (struct sockaddr_in *) &(ifr->ifr_ifru.ifru_addr);

  sockaddr_eth1 = (struct sockaddr_in *) &(ifr_next->ifr_ifru.ifru_addr);
  sockaddr_in_from_string("192.168.10.55",sockaddr_eth1);
  SMARTLIST_FOREACH(results, tor_addr_t *, t, tor_free(t));
  smartlist_free(results);

  results = ifreq_to_smartlist(ifc->ifc_buf,ifc->ifc_len);
  tt_int_op(smartlist_len(results),OP_EQ,2);

  tor_addr = smartlist_get(results, 0);
  addr_len =
  tor_addr_to_sockaddr(tor_addr,0,(struct sockaddr *)sockaddr_to_check,
                       sizeof(struct sockaddr_in));

  tt_int_op(addr_len,OP_EQ,sizeof(struct sockaddr_in));
  tt_assert(sockaddr_in_are_equal(sockaddr,sockaddr_to_check));

  tor_addr = smartlist_get(results, 1);
  addr_len =
  tor_addr_to_sockaddr(tor_addr,0,(struct sockaddr *)sockaddr_to_check,
                       sizeof(struct sockaddr_in));

  tt_int_op(addr_len,OP_EQ,sizeof(struct sockaddr_in));
  tt_assert(sockaddr_in_are_equal(sockaddr_eth1,sockaddr_to_check));

  done:
  tor_free(sockaddr_to_check);
  SMARTLIST_FOREACH(results, tor_addr_t *, t, tor_free(t));
  smartlist_free(results);
  tor_free(ifc);
  tor_free(ifr);
  return;
}

static void
test_address_get_if_addrs_ioctl(void *arg)
{

  smartlist_t *result = NULL;

  (void)arg;

  result = get_interface_addresses_ioctl(LOG_ERR, AF_INET);

  /* On an IPv6-only system, this will fail and return NULL
  tt_assert(result);
  */

  /* Some FreeBSD jails don't have localhost IP address. Instead, they only
   * have the address assigned to the jail (whatever that may be).
   * And a jail without a network connection might not have any addresses at
   * all. */
  if (result) {
    tt_assert(!smartlist_contains_null_tor_addr(result));

    /* If there are addresses, they must be IPv4 or IPv6.
     * (AIX supports IPv6 from SIOCGIFCONF.) */
    if (smartlist_len(result) > 0) {
      tt_assert(smartlist_contains_ipv4_tor_addr(result)
                || smartlist_contains_ipv6_tor_addr(result));
    }
  }

 done:
  if (result) {
    SMARTLIST_FOREACH(result, tor_addr_t *, t, tor_free(t));
    smartlist_free(result);
  }
  return;
}

#endif /* defined(HAVE_IFCONF_TO_SMARTLIST) */

#define FAKE_SOCKET_FD (42)

static tor_socket_t
fake_open_socket(int domain, int type, int protocol)
{
  (void)domain;
  (void)type;
  (void)protocol;

  return FAKE_SOCKET_FD;
}

static int
fake_close_socket(tor_socket_t s)
{
  (void)s;
  return 0;
}

static int last_connected_socket_fd = 0;

static int connect_retval = 0;

static tor_socket_t
pretend_to_connect(tor_socket_t sock, const struct sockaddr *address,
                   socklen_t address_len)
{
  (void)address;
  (void)address_len;

  last_connected_socket_fd = sock;

  return connect_retval;
}

static struct sockaddr *mock_addr = NULL;

static int
fake_getsockname(tor_socket_t sock, struct sockaddr *address,
                 socklen_t *address_len)
{
  socklen_t bytes_to_copy = 0;
  (void) sock;

  if (!mock_addr)
    return -1;

  if (mock_addr->sa_family == AF_INET) {
    bytes_to_copy = sizeof(struct sockaddr_in);
  } else if (mock_addr->sa_family == AF_INET6) {
    bytes_to_copy = sizeof(struct sockaddr_in6);
  } else {
    return -1;
  }

  if (*address_len < bytes_to_copy) {
    return -1;
  }

  memcpy(address,mock_addr,bytes_to_copy);
  *address_len = bytes_to_copy;

  return 0;
}

static void
test_address_udp_socket_trick_whitebox(void *arg)
{
  int hack_retval;
  tor_addr_t *addr_from_hack = tor_malloc_zero(sizeof(tor_addr_t));
  struct sockaddr_in6 *mock_addr6;
  struct sockaddr_in6 *ipv6_to_check =
  tor_malloc_zero(sizeof(struct sockaddr_in6));

  (void)arg;

  MOCK(tor_open_socket,fake_open_socket);
  MOCK(tor_connect_socket,pretend_to_connect);
  MOCK(tor_getsockname,fake_getsockname);
  MOCK(tor_close_socket,fake_close_socket);

  mock_addr = tor_malloc_zero(sizeof(struct sockaddr_storage));
  sockaddr_in_from_string("23.32.246.118",(struct sockaddr_in *)mock_addr);

  hack_retval =
  get_interface_address6_via_udp_socket_hack(LOG_DEBUG,
                                             AF_INET, addr_from_hack);

  tt_int_op(hack_retval,OP_EQ,0);
  tt_assert(tor_addr_eq_ipv4h(addr_from_hack, 0x1720f676));

  /* Now, lets do an IPv6 case. */
  memset(mock_addr,0,sizeof(struct sockaddr_storage));

  mock_addr6 = (struct sockaddr_in6 *)mock_addr;
  mock_addr6->sin6_family = AF_INET6;
  mock_addr6->sin6_port = 0;
  tor_inet_pton(AF_INET6,"2001:cdba::3257:9652",&(mock_addr6->sin6_addr));

  hack_retval =
  get_interface_address6_via_udp_socket_hack(LOG_DEBUG,
                                             AF_INET6, addr_from_hack);

  tt_int_op(hack_retval,OP_EQ,0);

  tor_addr_to_sockaddr(addr_from_hack,0,(struct sockaddr *)ipv6_to_check,
                       sizeof(struct sockaddr_in6));

  tt_assert(sockaddr_in6_are_equal(mock_addr6,ipv6_to_check));

 done:
  UNMOCK(tor_open_socket);
  UNMOCK(tor_connect_socket);
  UNMOCK(tor_getsockname);
  UNMOCK(tor_close_socket);

  tor_free(ipv6_to_check);
  tor_free(mock_addr);
  tor_free(addr_from_hack);
  return;
}

static void
test_address_udp_socket_trick_blackbox(void *arg)
{
  /* We want get_interface_address6_via_udp_socket_hack() to yield
   * the same valid address that get_interface_address6() returns.
   * If the latter is unable to find a valid address, we want
   * _hack() to fail and return-1.
   *
   * Furthermore, we want _hack() never to crash, even if
   * get_interface_addresses_raw() is returning NULL.
   */

  tor_addr_t addr4;
  tor_addr_t addr4_to_check;
  tor_addr_t addr6;
  tor_addr_t addr6_to_check;
  int retval, retval_reference;

  (void)arg;

#if 0
  retval_reference = get_interface_address6(LOG_DEBUG,AF_INET,&addr4);
  retval = get_interface_address6_via_udp_socket_hack(LOG_DEBUG,
                                                      AF_INET,
                                                      &addr4_to_check);

  tt_int_op(retval,OP_EQ,retval_reference);
  tt_assert( (retval == -1 && retval_reference == -1) ||
             (tor_addr_compare(&addr4,&addr4_to_check,CMP_EXACT) == 0) );

  retval_reference = get_interface_address6(LOG_DEBUG,AF_INET6,&addr6);
  retval = get_interface_address6_via_udp_socket_hack(LOG_DEBUG,
                                                      AF_INET6,
                                                      &addr6_to_check);

  tt_int_op(retval,OP_EQ,retval_reference);
  tt_assert( (retval == -1 && retval_reference == -1) ||
             (tor_addr_compare(&addr6,&addr6_to_check,CMP_EXACT) == 0) );

#else /* !(0) */
  /* Both of the blackbox test cases fail horribly if:
   *  * The host has no external addreses.
   *  * There are multiple interfaces with either AF_INET or AF_INET6.
   *  * The last address isn't the one associated with the default route.
   *
   * The tests SHOULD be re-enabled when #12377 is fixed correctly, but till
   * then this fails a lot, in situations we expect failures due to knowing
   * about the code being broken.
   */

  (void)addr4_to_check;
  (void)addr6_to_check;
  (void)addr6;
  (void) retval_reference;
#endif /* 0 */

  /* When family is neither AF_INET nor AF_INET6, we want _hack to
   * fail and return -1.
   */

  retval = get_interface_address6_via_udp_socket_hack(LOG_DEBUG,
                                                      AF_INET+AF_INET6,&addr4);

  tt_int_op(retval, OP_EQ, -1);

  done:
  return;
}

static void
test_address_get_if_addrs_list_internal(void *arg)
{
  smartlist_t *results = NULL;

  (void)arg;

  results = get_interface_address_list(LOG_WARN, 1);

  tt_ptr_op(results, OP_NE, NULL);
  /* When the network is down, a system might not have any non-local
   * non-multicast addresseses, not even internal ones.
   * Unit tests shouldn't fail because of this. */
  tt_int_op(smartlist_len(results),OP_GE,0);

  tt_assert(!smartlist_contains_localhost_tor_addr(results));
  tt_assert(!smartlist_contains_multicast_tor_addr(results));
  /* The list may or may not contain internal addresses */
  tt_assert(!smartlist_contains_null_tor_addr(results));

  /* if there are any addresses, they must be IPv4 */
  if (smartlist_len(results) > 0) {
    tt_assert(smartlist_contains_ipv4_tor_addr(results));
  }
  tt_assert(!smartlist_contains_ipv6_tor_addr(results));

 done:
  interface_address_list_free(results);
  return;
}

static void
test_address_get_if_addrs_list_no_internal(void *arg)
{
  smartlist_t *results = NULL;

  (void)arg;

  results = get_interface_address_list(LOG_WARN, 0);

  tt_ptr_op(results, OP_NE, NULL);
  /* Work even on systems with only internal IPv4 addresses */
  tt_int_op(smartlist_len(results),OP_GE,0);

  tt_assert(!smartlist_contains_localhost_tor_addr(results));
  tt_assert(!smartlist_contains_multicast_tor_addr(results));
  tt_assert(!smartlist_contains_internal_tor_addr(results));
  tt_assert(!smartlist_contains_null_tor_addr(results));

  /* if there are any addresses, they must be IPv4 */
  if (smartlist_len(results) > 0) {
    tt_assert(smartlist_contains_ipv4_tor_addr(results));
  }
  tt_assert(!smartlist_contains_ipv6_tor_addr(results));

 done:
  interface_address_list_free(results);
  return;
}

static void
test_address_get_if_addrs6_list_internal(void *arg)
{
  smartlist_t *results = NULL;

  (void)arg;

  /* We might drop a log_err */
  setup_full_capture_of_logs(LOG_ERR);
  results = get_interface_address6_list(LOG_ERR, AF_INET6, 1);
  tt_int_op(smartlist_len(mock_saved_logs()), OP_LE, 1);
  if (smartlist_len(mock_saved_logs()) == 1) {
    expect_log_msg_containing_either4("connect() failed",
                                      "unable to create socket",
                                      "Address that we determined via UDP "
                                      "socket magic is unsuitable for public "
                                      "comms.",
                                      "getsockname() to determine interface "
                                      "failed");
  }
  teardown_capture_of_logs();

  tt_ptr_op(results, OP_NE, NULL);
  /* Work even on systems without IPv6 interfaces */
  tt_int_op(smartlist_len(results),OP_GE,0);

  tt_assert(!smartlist_contains_localhost_tor_addr(results));
  tt_assert(!smartlist_contains_multicast_tor_addr(results));
  /* The list may or may not contain internal addresses */
  tt_assert(!smartlist_contains_null_tor_addr(results));

  /* if there are any addresses, they must be IPv6 */
  tt_assert(!smartlist_contains_ipv4_tor_addr(results));
  if (smartlist_len(results) > 0) {
    tt_assert(smartlist_contains_ipv6_tor_addr(results));
  }

 done:
  interface_address6_list_free(results);
  teardown_capture_of_logs();
  return;
}

static void
test_address_get_if_addrs6_list_no_internal(void *arg)
{
  smartlist_t *results = NULL;

  (void)arg;

  /* We might drop a log_err */
  setup_full_capture_of_logs(LOG_ERR);
  results = get_interface_address6_list(LOG_ERR, AF_INET6, 0);
  tt_int_op(smartlist_len(mock_saved_logs()), OP_LE, 1);
  if (smartlist_len(mock_saved_logs()) == 1) {
    expect_log_msg_containing_either4("connect() failed",
                                      "unable to create socket",
                                      "Address that we determined via UDP "
                                      "socket magic is unsuitable for public "
                                      "comms.",
                                      "getsockname() to determine interface "
                                      "failed");
  }
  teardown_capture_of_logs();

  tt_ptr_op(results, OP_NE, NULL);
  /* Work even on systems without IPv6 interfaces */
  tt_int_op(smartlist_len(results),OP_GE,0);

  tt_assert(!smartlist_contains_localhost_tor_addr(results));
  tt_assert(!smartlist_contains_multicast_tor_addr(results));
  tt_assert(!smartlist_contains_internal_tor_addr(results));
  tt_assert(!smartlist_contains_null_tor_addr(results));

  /* if there are any addresses, they must be IPv6 */
  tt_assert(!smartlist_contains_ipv4_tor_addr(results));
  if (smartlist_len(results) > 0) {
    tt_assert(smartlist_contains_ipv6_tor_addr(results));
  }

 done:
  teardown_capture_of_logs();
  interface_address6_list_free(results);
  return;
}

static int called_get_interface_addresses_raw = 0;

static smartlist_t *
mock_get_interface_addresses_raw_fail(int severity, sa_family_t family)
{
  (void)severity;
  (void)family;

  called_get_interface_addresses_raw++;
  return smartlist_new();
}

static int called_get_interface_address6_via_udp_socket_hack = 0;

static int
mock_get_interface_address6_via_udp_socket_hack_fail(int severity,
                                                     sa_family_t family,
                                                     tor_addr_t *addr)
{
  (void)severity;
  (void)family;
  (void)addr;

  called_get_interface_address6_via_udp_socket_hack++;
  return -1;
}

static void
test_address_get_if_addrs_internal_fail(void *arg)
{
  smartlist_t *results1 = NULL, *results2 = NULL;
  int rv = 0;
  uint32_t ipv4h_addr = 0;
  tor_addr_t ipv6_addr;

  memset(&ipv6_addr, 0, sizeof(tor_addr_t));

  (void)arg;

  MOCK(get_interface_addresses_raw,
       mock_get_interface_addresses_raw_fail);
  MOCK(get_interface_address6_via_udp_socket_hack,
       mock_get_interface_address6_via_udp_socket_hack_fail);

  results1 = get_interface_address6_list(LOG_ERR, AF_INET6, 1);
  tt_ptr_op(results1, OP_NE, NULL);
  tt_int_op(smartlist_len(results1),OP_EQ,0);

  results2 = get_interface_address_list(LOG_ERR, 1);
  tt_ptr_op(results2, OP_NE, NULL);
  tt_int_op(smartlist_len(results2),OP_EQ,0);

  rv = get_interface_address6(LOG_ERR, AF_INET6, &ipv6_addr);
  tt_int_op(rv, OP_EQ, -1);

  rv = get_interface_address(LOG_ERR, &ipv4h_addr);
  tt_int_op(rv, OP_EQ, -1);

 done:
  UNMOCK(get_interface_addresses_raw);
  UNMOCK(get_interface_address6_via_udp_socket_hack);
  interface_address6_list_free(results1);
  interface_address6_list_free(results2);
  return;
}

static void
test_address_get_if_addrs_no_internal_fail(void *arg)
{
  smartlist_t *results1 = NULL, *results2 = NULL;

  (void)arg;

  MOCK(get_interface_addresses_raw,
       mock_get_interface_addresses_raw_fail);
  MOCK(get_interface_address6_via_udp_socket_hack,
       mock_get_interface_address6_via_udp_socket_hack_fail);

  results1 = get_interface_address6_list(LOG_ERR, AF_INET6, 0);
  tt_ptr_op(results1, OP_NE, NULL);
  tt_int_op(smartlist_len(results1),OP_EQ,0);

  results2 = get_interface_address_list(LOG_ERR, 0);
  tt_ptr_op(results2, OP_NE, NULL);
  tt_int_op(smartlist_len(results2),OP_EQ,0);

 done:
  UNMOCK(get_interface_addresses_raw);
  UNMOCK(get_interface_address6_via_udp_socket_hack);
  interface_address6_list_free(results1);
  interface_address6_list_free(results2);
  return;
}

static void
test_address_get_if_addrs(void *arg)
{
  int rv;
  uint32_t addr_h = 0;
  tor_addr_t tor_addr;

  (void)arg;

  rv = get_interface_address(LOG_WARN, &addr_h);

  /* When the network is down, a system might not have any non-local
   * non-multicast IPv4 addresses, not even internal ones.
   * Unit tests shouldn't fail because of this. */
  if (rv == 0) {
    tor_addr_from_ipv4h(&tor_addr, addr_h);

    tt_assert(!tor_addr_is_loopback(&tor_addr));
    tt_assert(!tor_addr_is_multicast(&tor_addr));
    /* The address may or may not be an internal address */

    tt_assert(tor_addr_is_v4(&tor_addr));
  }

 done:
  return;
}

static void
test_address_get_if_addrs6(void *arg)
{
  int rv;
  tor_addr_t tor_addr;

  (void)arg;

  rv = get_interface_address6(LOG_WARN, AF_INET6, &tor_addr);

  /* Work even on systems without IPv6 interfaces */
  if (rv == 0) {
    tt_assert(!tor_addr_is_loopback(&tor_addr));
    tt_assert(!tor_addr_is_multicast(&tor_addr));
    /* The address may or may not be an internal address */

    tt_assert(!tor_addr_is_v4(&tor_addr));
  }

 done:
  return;
}

static void
test_address_tor_addr_to_in6(void *ignored)
{
  (void)ignored;
  tor_addr_t *a = tor_malloc_zero(sizeof(tor_addr_t));
  const struct in6_addr *res;
  uint8_t expected[16] = {42, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                          10, 11, 12, 13, 14, 15};

  a->family = AF_INET;
  res = tor_addr_to_in6(a);
  tt_assert(!res);

  a->family = AF_INET6;
  memcpy(a->addr.in6_addr.s6_addr, expected, 16);
  res = tor_addr_to_in6(a);
  tt_assert(res);
  tt_mem_op(res->s6_addr, OP_EQ, expected, 16);

 done:
  tor_free(a);
}

static void
test_address_tor_addr_to_in(void *ignored)
{
  (void)ignored;
  tor_addr_t *a = tor_malloc_zero(sizeof(tor_addr_t));
  const struct in_addr *res;

  a->family = AF_INET6;
  res = tor_addr_to_in(a);
  tt_assert(!res);

  a->family = AF_INET;
  a->addr.in_addr.s_addr = 44;
  res = tor_addr_to_in(a);
  tt_assert(res);
  tt_int_op(res->s_addr, OP_EQ, 44);

 done:
  tor_free(a);
}

static void
test_address_tor_addr_to_ipv4n(void *ignored)
{
  (void)ignored;
  tor_addr_t *a = tor_malloc_zero(sizeof(tor_addr_t));
  uint32_t res;

  a->family = AF_INET6;
  res = tor_addr_to_ipv4n(a);
  tt_assert(!res);

  a->family = AF_INET;
  a->addr.in_addr.s_addr = 43;
  res = tor_addr_to_ipv4n(a);
  tt_assert(res);
  tt_int_op(res, OP_EQ, 43);

 done:
  tor_free(a);
}

static void
test_address_tor_addr_to_mapped_ipv4h(void *ignored)
{
  (void)ignored;
  tor_addr_t *a = tor_malloc_zero(sizeof(tor_addr_t));
  uint32_t res;
  uint8_t toset[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 42};

  a->family = AF_INET;
  res = tor_addr_to_mapped_ipv4h(a);
  tt_assert(!res);

  a->family = AF_INET6;

  memcpy(a->addr.in6_addr.s6_addr, toset, 16);
  res = tor_addr_to_mapped_ipv4h(a);
  tt_assert(res);
  tt_int_op(res, OP_EQ, 42);

 done:
  tor_free(a);
}

static void
test_address_tor_addr_eq_ipv4h(void *ignored)
{
  (void)ignored;
  tor_addr_t *a = tor_malloc_zero(sizeof(tor_addr_t));
  int res;

  a->family = AF_INET6;
  res = tor_addr_eq_ipv4h(a, 42);
  tt_assert(!res);

  a->family = AF_INET;
  a->addr.in_addr.s_addr = 52;
  res = tor_addr_eq_ipv4h(a, 42);
  tt_assert(!res);

  a->addr.in_addr.s_addr = 52;
  res = tor_addr_eq_ipv4h(a, ntohl(52));
  tt_assert(res);

 done:
  tor_free(a);
}

static void
test_address_tor_addr_in_same_network_family(void *ignored)
{
  (void)ignored;
  tor_addr_t a, b;

  tor_addr_parse(&a, "8.8.8.8");
  tor_addr_parse(&b, "8.8.4.4");
  tt_int_op(addrs_in_same_network_family(&a, &b), OP_EQ, 1);

  tor_addr_parse(&a, "8.8.8.8");
  tor_addr_parse(&b, "1.1.1.1");
  tt_int_op(addrs_in_same_network_family(&a, &b), OP_EQ, 0);

  tor_addr_parse(&a, "8.8.8.8");
  tor_addr_parse(&b, "2001:4860:4860::8844");
  tt_int_op(addrs_in_same_network_family(&a, &b), OP_EQ, 0);

  tor_addr_parse(&a, "2001:4860:4860::8888");
  tor_addr_parse(&b, "2001:4860:4860::8844");
  tt_int_op(addrs_in_same_network_family(&a, &b), OP_EQ, 1);

  tor_addr_parse(&a, "2001:4860:4860::8888");
  tor_addr_parse(&b, "2001:470:20::2");
  tt_int_op(addrs_in_same_network_family(&a, &b), OP_EQ, 0);

 done:
  return;
}

static node_t *
helper_create_mock_node(char id_char)
{
  node_t *node = tor_malloc_zero(sizeof(node_t));
  routerinfo_t *ri = tor_malloc_zero(sizeof(routerinfo_t));
  tor_addr_make_null(&ri->ipv6_addr, AF_INET6);
  node->ri = ri;
  memset(node->identity, id_char, sizeof(node->identity));
  return node;
}

static void
helper_free_mock_node(node_t *node)
{
  if (!node)
    return;
  tor_free(node->ri);
  tor_free(node);
}

#define NODE_SET_IPV4(node, ipv4_addr, ipv4_port) { \
    tor_addr_t addr; \
    tor_addr_parse(&addr, ipv4_addr); \
    node->ri->addr = tor_addr_to_ipv4h(&addr); \
    node->ri->or_port = ipv4_port; \
  }

#define NODE_CLEAR_IPV4(node) { \
    node->ri->addr = 0; \
    node->ri->or_port = 0; \
  }

#define NODE_SET_IPV6(node, ipv6_addr_str, ipv6_port) { \
    tor_addr_parse(&node->ri->ipv6_addr, ipv6_addr_str); \
    node->ri->ipv6_orport = ipv6_port; \
  }

static void
test_address_tor_node_in_same_network_family(void *ignored)
{
  (void)ignored;
  node_t *node_a = helper_create_mock_node('a');
  node_t *node_b = helper_create_mock_node('b');

  NODE_SET_IPV4(node_a, "8.8.8.8", 1);
  NODE_SET_IPV4(node_b, "8.8.4.4", 1);

  tt_int_op(nodes_in_same_family(node_a, node_b), OP_EQ, 1);

  NODE_SET_IPV4(node_a, "8.8.8.8", 1);
  NODE_SET_IPV4(node_b, "1.1.1.1", 1);

  tt_int_op(nodes_in_same_family(node_a, node_b), OP_EQ, 0);

  NODE_CLEAR_IPV4(node_a);
  NODE_SET_IPV6(node_a, "2001:470:20::2", 1);

  tt_int_op(nodes_in_same_family(node_a, node_b), OP_EQ, 0);

  NODE_CLEAR_IPV4(node_b);
  NODE_SET_IPV6(node_b, "2606:4700:4700::1111", 1);

  tt_int_op(nodes_in_same_family(node_a, node_b), OP_EQ, 0);

  NODE_SET_IPV6(node_a, "2606:4700:4700::1001", 1);
  tt_int_op(nodes_in_same_family(node_a, node_b), OP_EQ, 1);

 done:
  helper_free_mock_node(node_a);
  helper_free_mock_node(node_b);
}

static or_options_t mock_options;

static const or_options_t *
mock_get_options(void)
{
  return &mock_options;
}

/* Test dirserv_router_has_valid_address() on a stub routerinfo, with only its
 * address fields set. Use IPv4 ipv4_addr_str and IPv6 ipv6_addr_str.
 * Fail if it does not return rv. */
#define TEST_ROUTER_VALID_ADDRESS_HELPER(ipv4_addr_str, ipv6_addr_str, rv) \
  STMT_BEGIN \
    ri = tor_malloc_zero(sizeof(routerinfo_t)); \
    tor_addr_t addr; \
    tor_addr_parse(&addr, (ipv4_addr_str));   \
    ri->addr = tor_addr_to_ipv4h(&addr); \
    tor_addr_parse(&ri->ipv6_addr, (ipv6_addr_str)); \
    tt_int_op(dirserv_router_has_valid_address(ri), OP_EQ, (rv)); \
    tor_free(ri); \
  STMT_END

/* Like TEST_ROUTER_VALID_ADDRESS_HELPER(), but always passes a null
 * IPv6 address. */
#define CHECK_RI_ADDR(ipv4_addr_str, rv) \
  TEST_ROUTER_VALID_ADDRESS_HELPER(ipv4_addr_str, "::", rv)

/* Like TEST_ROUTER_VALID_ADDRESS_HELPER(), but always passes a non-internal
 * IPv4 address, so that the IPv6 check is reached. */
#define CHECK_RI_ADDR6(ipv6_addr_str, rv) \
  TEST_ROUTER_VALID_ADDRESS_HELPER("1.0.0.1", ipv6_addr_str, rv)

static void
test_address_dirserv_router_addr_private(void *opt_dir_allow_private)
{
  /* A stub routerinfo structure, with only its address fields set. */
  routerinfo_t *ri = NULL;
  /* The expected return value for private addresses.
   * Modified if DirAllowPrivateAddresses is 1. */
  int private_rv = -1;

  memset(&mock_options, 0, sizeof(or_options_t));
  MOCK(get_options, mock_get_options);

  if (opt_dir_allow_private) {
    mock_options.DirAllowPrivateAddresses = 1;
    private_rv = 0;
  }

  CHECK_RI_ADDR("1.0.0.1", 0);
  CHECK_RI_ADDR("10.0.0.1", private_rv);

  CHECK_RI_ADDR6("2600::1", 0);
  CHECK_RI_ADDR6("fe80::1", private_rv);

  /* Null addresses */
  /* IPv4 null fails, regardless of IPv6 */
  CHECK_RI_ADDR("0.0.0.0", private_rv);
  TEST_ROUTER_VALID_ADDRESS_HELPER("0.0.0.0", "::", private_rv);

  /* IPv6 null succeeds, because IPv4 is not null */
  CHECK_RI_ADDR6("::", 0);

  /* Byte-zeroed null addresses */
  /* IPv4 null fails, regardless of IPv6 */
  {
    ri = tor_malloc_zero(sizeof(routerinfo_t));
    tt_int_op(dirserv_router_has_valid_address(ri), OP_EQ, private_rv);
    tor_free(ri);
  }

  /* IPv6 null succeeds, because IPv4 is not internal */
  {
    ri = tor_malloc_zero(sizeof(routerinfo_t));
    ri->addr = 16777217; /* 1.0.0.1 */
    tt_int_op(dirserv_router_has_valid_address(ri), OP_EQ, 0);
    tor_free(ri);
  }

 done:
  tor_free(ri);
  UNMOCK(get_options);
}

#define ADDRESS_TEST(name, flags) \
  { #name, test_address_ ## name, flags, NULL, NULL }
#define ADDRESS_TEST_STR_ARG(name, flags, str_arg) \
  { #name "/" str_arg, test_address_ ## name, flags, &passthrough_setup, \
    (void *)(str_arg) }

struct testcase_t address_tests[] = {
  ADDRESS_TEST(udp_socket_trick_whitebox, TT_FORK),
  ADDRESS_TEST(udp_socket_trick_blackbox, TT_FORK),
  ADDRESS_TEST(get_if_addrs_list_internal, 0),
  ADDRESS_TEST(get_if_addrs_list_no_internal, 0),
  ADDRESS_TEST(get_if_addrs6_list_internal, 0),
  ADDRESS_TEST(get_if_addrs6_list_no_internal, TT_FORK),
  ADDRESS_TEST(get_if_addrs_internal_fail, 0),
  ADDRESS_TEST(get_if_addrs_no_internal_fail, 0),
  ADDRESS_TEST(get_if_addrs, 0),
  ADDRESS_TEST(get_if_addrs6, 0),
#ifdef HAVE_IFADDRS_TO_SMARTLIST
  ADDRESS_TEST(get_if_addrs_ifaddrs, TT_FORK),
  ADDRESS_TEST(ifaddrs_to_smartlist, 0),
#endif
#ifdef HAVE_IP_ADAPTER_TO_SMARTLIST
  ADDRESS_TEST(get_if_addrs_win32, TT_FORK),
  ADDRESS_TEST(ip_adapter_addresses_to_smartlist, 0),
#endif
#ifdef HAVE_IFCONF_TO_SMARTLIST
  ADDRESS_TEST(get_if_addrs_ioctl, TT_FORK),
  ADDRESS_TEST(ifreq_to_smartlist, 0),
#endif
  ADDRESS_TEST(tor_addr_to_in6, 0),
  ADDRESS_TEST(tor_addr_to_in, 0),
  ADDRESS_TEST(tor_addr_to_ipv4n, 0),
  ADDRESS_TEST(tor_addr_to_mapped_ipv4h, 0),
  ADDRESS_TEST(tor_addr_eq_ipv4h, 0),
  ADDRESS_TEST(tor_addr_in_same_network_family, 0),
  ADDRESS_TEST(tor_node_in_same_network_family, 0),
  ADDRESS_TEST(dirserv_router_addr_private, 0),
  ADDRESS_TEST_STR_ARG(dirserv_router_addr_private, 0, "allow_private"),
  END_OF_TESTCASES
};
