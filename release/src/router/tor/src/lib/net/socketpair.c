/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */

/**
 * @file socketpair.c
 * @brief Replacement socketpair() for systems that lack it
 **/

#include "lib/cc/torint.h"
#include "lib/net/socketpair.h"
#include "lib/net/inaddr_st.h"
#include "lib/arch/bytes.h"

#include <errno.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#define socket_errno() (WSAGetLastError())
#define SOCKET_EPROTONOSUPPORT WSAEPROTONOSUPPORT
#else /* !defined(_WIN32) */
#define closesocket(x) close(x)
#define socket_errno() (errno)
#define SOCKET_EPROTONOSUPPORT EPROTONOSUPPORT
#endif /* defined(_WIN32) */

#ifdef NEED_ERSATZ_SOCKETPAIR

// Avoid warning about call to memcmp.
#define raw_memcmp memcmp

/**
 * Return a new socket that is bound and listening on the loopback interface
 * of family <b>family</b> for a socket of type <b>type</b>. On failure return
 * TOR_INVALID_SOCKET.
 */
static tor_socket_t
get_local_listener(int family, int type)
{
  struct sockaddr_in sin;
  struct sockaddr_in6 sin6;
  struct sockaddr *sa;
  int len;

  memset(&sin, 0, sizeof(sin));
  memset(&sin6, 0, sizeof(sin6));

  tor_socket_t sock = TOR_INVALID_SOCKET;
  sock = socket(family, type, 0);
  if (!SOCKET_OK(sock)) {
    return TOR_INVALID_SOCKET;
  }

  if (family == AF_INET) {
    sa = (struct sockaddr *) &sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = tor_htonl(0x7f000001);
    len = sizeof(sin);
  } else {
    sa = (struct sockaddr *) &sin6;
    sin6.sin6_family = AF_INET6;
    sin6.sin6_addr.s6_addr[15] = 1;
    len = sizeof(sin6);
  }

  if (bind(sock, sa, len) == -1)
    goto err;
  if (listen(sock, 1) == -1)
    goto err;

  return sock;
 err:
  closesocket(sock);
  return TOR_INVALID_SOCKET;
}

/**
 * Return true iff sa1 and sa2 are equivalent AF_INET or AF_INET6 addresses.
 */
static int
sockaddr_eq(struct sockaddr *sa1, struct sockaddr *sa2)
{
  if (sa1->sa_family != sa2->sa_family)
    return 0;

  if (sa1->sa_family == AF_INET6) {
    struct sockaddr_in6 *sin6_1 = (struct sockaddr_in6 *) sa1;
    struct sockaddr_in6 *sin6_2 = (struct sockaddr_in6 *) sa2;
    return sin6_1->sin6_port == sin6_2->sin6_port &&
      0==raw_memcmp(sin6_1->sin6_addr.s6_addr, sin6_2->sin6_addr.s6_addr, 16);
  } else if (sa1->sa_family == AF_INET) {
    struct sockaddr_in *sin_1 = (struct sockaddr_in *) sa1;
    struct sockaddr_in *sin_2 = (struct sockaddr_in *) sa2;
    return sin_1->sin_port == sin_2->sin_port &&
      sin_1->sin_addr.s_addr == sin_2->sin_addr.s_addr;
  } else {
    return 0;
  }
}

/**
 * Helper used to implement socketpair on systems that lack it, by
 * making a direct connection to localhost.
 *
 * See tor_socketpair() for details.
 *
 * The direct connection defaults to IPv4, but falls back to IPv6 if
 * IPv4 is not supported.
 **/
int
tor_ersatz_socketpair(int family, int type, int protocol, tor_socket_t fd[2])
{
  /* This socketpair does not work when localhost is down. So
   * it's really not the same thing at all. But it's close enough
   * for now, and really, when localhost is down sometimes, we
   * have other problems too.
   */
  tor_socket_t listener = TOR_INVALID_SOCKET;
  tor_socket_t connector = TOR_INVALID_SOCKET;
  tor_socket_t acceptor = TOR_INVALID_SOCKET;
  struct sockaddr_storage accepted_addr_ss;
  struct sockaddr_storage connect_addr_ss;
  struct sockaddr *connect_addr = (struct sockaddr *) &connect_addr_ss;
  struct sockaddr *accepted_addr = (struct sockaddr *) &accepted_addr_ss;
  socklen_t size;
  int saved_errno = -1;
  int ersatz_domain = AF_INET;
  socklen_t addrlen = sizeof(struct sockaddr_in);

  memset(&accepted_addr_ss, 0, sizeof(accepted_addr_ss));
  memset(&connect_addr_ss, 0, sizeof(connect_addr_ss));

  if (protocol
#ifdef AF_UNIX
      || family != AF_UNIX
#endif
      ) {
#ifdef _WIN32
    return -WSAEAFNOSUPPORT;
#else
    return -EAFNOSUPPORT;
#endif
  }
  if (!fd) {
    return -EINVAL;
  }

  listener = get_local_listener(ersatz_domain, type);
  if (!SOCKET_OK(listener)) {
    int first_errno = socket_errno();
    if (first_errno == SOCKET_EPROTONOSUPPORT) {
      /* Assume we're on an IPv6-only system */
      ersatz_domain = AF_INET6;
      addrlen = sizeof(struct sockaddr_in6);
      listener = get_local_listener(ersatz_domain, type);
    }
    if (!SOCKET_OK(listener)) {
      /* Keep the previous behaviour, which was to return the IPv4 error.
       * (This may be less informative on IPv6-only systems.)
       * XX/teor - is there a better way to decide which errno to return?
       * (I doubt we care much either way, once there is an error.)
       */
      return -first_errno;
    }
  }

  connector = socket(ersatz_domain, type, 0);
  if (!SOCKET_OK(connector))
    goto tidy_up_and_fail;
  /* We want to find out the port number to connect to.  */
  size = sizeof(connect_addr_ss);
  if (getsockname(listener, connect_addr, &size) == -1)
    goto tidy_up_and_fail;
  if (size != addrlen)
    goto abort_tidy_up_and_fail;
  if (connect(connector, connect_addr, size) == -1)
    goto tidy_up_and_fail;

  size = sizeof(accepted_addr_ss);
  acceptor = accept(listener, accepted_addr, &size);
  if (!SOCKET_OK(acceptor))
    goto tidy_up_and_fail;
  if (size != addrlen)
    goto abort_tidy_up_and_fail;
  /* Now check we are talking to ourself by matching port and host on the
     two sockets.  */
  if (getsockname(connector, connect_addr, &size) == -1)
    goto tidy_up_and_fail;
  /* Set *_tor_addr and *_port to the address and port that was used */
  if (!sockaddr_eq(accepted_addr, connect_addr))
    goto abort_tidy_up_and_fail;
  closesocket(listener);
  fd[0] = connector;
  fd[1] = acceptor;
  return 0;

 abort_tidy_up_and_fail:
#ifdef _WIN32
  saved_errno = WSAECONNABORTED;
#else
  saved_errno = ECONNABORTED; /* I hope this is portable and appropriate.  */
#endif
 tidy_up_and_fail:
  if (saved_errno < 0)
    saved_errno = errno;
  if (SOCKET_OK(listener))
    closesocket(listener);
  if (SOCKET_OK(connector))
    closesocket(connector);
  if (SOCKET_OK(acceptor))
    closesocket(acceptor);
  return -saved_errno;
}

#endif /* defined(NEED_ERSATZ_SOCKETPAIR) */
