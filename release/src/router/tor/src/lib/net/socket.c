/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file socket.c
 * \brief Compatibility and utility functions for working with network
 *    sockets.
 **/

#include "lib/net/socket.h"
#include "lib/net/socketpair.h"
#include "lib/net/address.h"
#include "lib/cc/compat_compiler.h"
#include "lib/err/torerr.h"
#include "lib/lock/compat_mutex.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <stddef.h>
#include <string.h>
#ifdef __FreeBSD__
#include <sys/sysctl.h>
#endif

/** Called before we make any calls to network-related functions.
 * (Some operating systems require their network libraries to be
 * initialized.) */
int
network_init(void)
{
#ifdef _WIN32
  /* This silly exercise is necessary before windows will allow
   * gethostbyname to work. */
  WSADATA WSAData;
  int r;
  r = WSAStartup(0x101,&WSAData);
  if (r) {
    log_warn(LD_NET,"Error initializing windows network layer: code was %d",r);
    return -1;
  }
  if (sizeof(SOCKET) != sizeof(tor_socket_t)) {
    log_warn(LD_BUG,"The tor_socket_t type does not match SOCKET in size; Tor "
             "might not work. (Sizes are %d and %d respectively.)",
             (int)sizeof(tor_socket_t), (int)sizeof(SOCKET));
  }
  /* WSAData.iMaxSockets might show the max sockets we're allowed to use.
   * We might use it to complain if we're trying to be a server but have
   * too few sockets available. */
#endif /* defined(_WIN32) */
  return 0;
}

/**
 * Warn the user if any system network parameters should be changed.
 */
void
check_network_configuration(bool server_mode)
{
#ifdef __FreeBSD__
  if (server_mode) {
    int random_id_state;
    size_t state_size = sizeof(random_id_state);

    if (sysctlbyname("net.inet.ip.random_id", &random_id_state,
                     &state_size, NULL, 0)) {
      log_warn(LD_CONFIG,
               "Failed to figure out if IP ids are randomized.");
    } else if (random_id_state == 0) {
      log_warn(LD_CONFIG, "Looks like IP ids are not randomized. "
               "Please consider setting the net.inet.ip.random_id sysctl, "
               "so your relay makes it harder to figure out how busy it is.");
    }
  }
#else /* !defined(__FreeBSD__) */
  (void) server_mode;
#endif /* defined(__FreeBSD__) */
}

/* When set_max_file_sockets() is called, update this with the max file
 * descriptor value so we can use it to check the limit when opening a new
 * socket. Default value is what Debian sets as the default hard limit. */
static int max_sockets = 1024;

/** Return the maximum number of allowed sockets. */
int
get_max_sockets(void)
{
  return max_sockets;
}

/** Set the maximum number of allowed sockets to <b>n</b> */
void
set_max_sockets(int n)
{
  max_sockets = n;
}

#undef DEBUG_SOCKET_COUNTING
#ifdef DEBUG_SOCKET_COUNTING
#include "lib/container/bitarray.h"

/** A bitarray of all fds that should be passed to tor_socket_close(). Only
 * used if DEBUG_SOCKET_COUNTING is defined. */
static bitarray_t *open_sockets = NULL;
/** The size of <b>open_sockets</b>, in bits. */
static int max_socket = -1;
#endif /* defined(DEBUG_SOCKET_COUNTING) */

/** Count of number of sockets currently open.  (Undercounts sockets opened by
 * eventdns and libevent.) */
static int n_sockets_open = 0;

/** Mutex to protect open_sockets, max_socket, and n_sockets_open. */
static tor_mutex_t *socket_accounting_mutex = NULL;

/** Helper: acquire the socket accounting lock. */
static inline void
socket_accounting_lock(void)
{
  if (PREDICT_UNLIKELY(!socket_accounting_mutex))
    socket_accounting_mutex = tor_mutex_new();
  tor_mutex_acquire(socket_accounting_mutex);
}

/** Helper: release the socket accounting lock. */
static inline void
socket_accounting_unlock(void)
{
  tor_mutex_release(socket_accounting_mutex);
}

/** As close(), but guaranteed to work for sockets across platforms (including
 * Windows, where close()ing a socket doesn't work.  Returns 0 on success and
 * the socket error code on failure. */
int
tor_close_socket_simple(tor_socket_t s)
{
  int r = 0;

  /* On Windows, you have to call close() on fds returned by open(),
  * and closesocket() on fds returned by socket().  On Unix, everything
  * gets close()'d.  We abstract this difference by always using
  * tor_close_socket to close sockets, and always using close() on
  * files.
  */
  #if defined(_WIN32)
    r = closesocket(s);
  #else
    r = close(s);
  #endif

  if (r != 0) {
    int err = tor_socket_errno(-1);
    log_info(LD_NET, "Close returned an error: %s", tor_socket_strerror(err));
    return err;
  }

  return r;
}

/** @{ */
#ifdef DEBUG_SOCKET_COUNTING
/** Helper: if DEBUG_SOCKET_COUNTING is enabled, remember that <b>s</b> is
 * now an open socket. */
static inline void
mark_socket_open(tor_socket_t s)
{
  /* XXXX This bitarray business will NOT work on windows: sockets aren't
     small ints there. */
  if (s > max_socket) {
    if (max_socket == -1) {
      open_sockets = bitarray_init_zero(s+128);
      max_socket = s+128;
    } else {
      open_sockets = bitarray_expand(open_sockets, max_socket, s+128);
      max_socket = s+128;
    }
  }
  if (bitarray_is_set(open_sockets, s)) {
    log_warn(LD_BUG, "I thought that %d was already open, but socket() just "
             "gave it to me!", s);
  }
  bitarray_set(open_sockets, s);
}
static inline void
mark_socket_closed(tor_socket_t s)
{
  if (s > max_socket || ! bitarray_is_set(open_sockets, s)) {
    log_warn(LD_BUG, "Closing a socket (%d) that wasn't returned by tor_open_"
             "socket(), or that was already closed or something.", s);
  } else {
    tor_assert(open_sockets && s <= max_socket);
    bitarray_clear(open_sockets, s);
  }
}
#else /* !defined(DEBUG_SOCKET_COUNTING) */
#define mark_socket_open(s) ((void) (s))
#define mark_socket_closed(s) ((void) (s))
#endif /* defined(DEBUG_SOCKET_COUNTING) */
/** @} */

/** As tor_close_socket_simple(), but keeps track of the number
 * of open sockets. Returns 0 on success, -1 on failure. */
MOCK_IMPL(int,
tor_close_socket,(tor_socket_t s))
{
  int r = tor_close_socket_simple(s);

  socket_accounting_lock();
  mark_socket_closed(s);
  if (r == 0) {
    --n_sockets_open;
  } else {
#ifdef _WIN32
    if (r != WSAENOTSOCK)
      --n_sockets_open;
#else
    if (r != EBADF)
      --n_sockets_open; // LCOV_EXCL_LINE -- EIO and EINTR too hard to force.
#endif /* defined(_WIN32) */
    r = -1;
  }

  tor_assert_nonfatal(n_sockets_open >= 0);
  socket_accounting_unlock();
  return r;
}

/** As socket(), but counts the number of open sockets. */
MOCK_IMPL(tor_socket_t,
tor_open_socket,(int domain, int type, int protocol))
{
  return tor_open_socket_with_extensions(domain, type, protocol, 1, 0);
}

/** Mockable wrapper for connect(). */
MOCK_IMPL(tor_socket_t,
tor_connect_socket,(tor_socket_t sock, const struct sockaddr *address,
                     socklen_t address_len))
{
  return connect(sock,address,address_len);
}

/** As socket(), but creates a nonblocking socket and
 * counts the number of open sockets. */
tor_socket_t
tor_open_socket_nonblocking(int domain, int type, int protocol)
{
  return tor_open_socket_with_extensions(domain, type, protocol, 1, 1);
}

/** As socket(), but counts the number of open sockets and handles
 * socket creation with either of SOCK_CLOEXEC and SOCK_NONBLOCK specified.
 * <b>cloexec</b> and <b>nonblock</b> should be either 0 or 1 to indicate
 * if the corresponding extension should be used.*/
tor_socket_t
tor_open_socket_with_extensions(int domain, int type, int protocol,
                                int cloexec, int nonblock)
{
  tor_socket_t s;

  /* We are about to create a new file descriptor so make sure we have
   * enough of them. */
  if (get_n_open_sockets() >= max_sockets - 1) {
#ifdef _WIN32
    WSASetLastError(WSAEMFILE);
#else
    errno = EMFILE;
#endif
    return TOR_INVALID_SOCKET;
  }

#if defined(SOCK_CLOEXEC) && defined(SOCK_NONBLOCK)
  int ext_flags = (cloexec ? SOCK_CLOEXEC : 0) |
                  (nonblock ? SOCK_NONBLOCK : 0);
  s = socket(domain, type|ext_flags, protocol);
  if (SOCKET_OK(s))
    goto socket_ok;
  /* If we got an error, see if it is EINVAL. EINVAL might indicate that,
   * even though we were built on a system with SOCK_CLOEXEC and SOCK_NONBLOCK
   * support, we are running on one without. */
  if (errno != EINVAL)
    return s;
#endif /* defined(SOCK_CLOEXEC) && defined(SOCK_NONBLOCK) */

  s = socket(domain, type, protocol);
  if (! SOCKET_OK(s))
    return s;

#if defined(FD_CLOEXEC)
  if (cloexec) {
    if (fcntl(s, F_SETFD, FD_CLOEXEC) == -1) {
      log_warn(LD_FS,"Couldn't set FD_CLOEXEC: %s", strerror(errno));
      tor_close_socket_simple(s);
      return TOR_INVALID_SOCKET;
    }
  }
#else /* !defined(FD_CLOEXEC) */
  (void)cloexec;
#endif /* defined(FD_CLOEXEC) */

  if (nonblock) {
    if (set_socket_nonblocking(s) == -1) {
      tor_close_socket_simple(s);
      return TOR_INVALID_SOCKET;
    }
  }

  goto socket_ok; /* So that socket_ok will not be unused. */

 socket_ok:
  tor_take_socket_ownership(s);
  return s;
}

/**
 * For socket accounting: remember that we are the owner of the socket
 * <b>s</b>. This will prevent us from overallocating sockets, and prevent us
 * from asserting later when we close the socket <b>s</b>.
 */
void
tor_take_socket_ownership(tor_socket_t s)
{
  socket_accounting_lock();
  ++n_sockets_open;
  mark_socket_open(s);
  socket_accounting_unlock();
}

/**
 * For socket accounting: declare that we are no longer the owner of the
 * socket <b>s</b>. This will prevent us from overallocating sockets, and
 * prevent us from asserting later when we close the socket <b>s</b>.
 */
void
tor_release_socket_ownership(tor_socket_t s)
{
  socket_accounting_lock();
  --n_sockets_open;
  mark_socket_closed(s);
  socket_accounting_unlock();
}

/** As accept(), but counts the number of open sockets. */
tor_socket_t
tor_accept_socket(tor_socket_t sockfd, struct sockaddr *addr, socklen_t *len)
{
  return tor_accept_socket_with_extensions(sockfd, addr, len, 1, 0);
}

/** As accept(), but returns a nonblocking socket and
 * counts the number of open sockets. */
tor_socket_t
tor_accept_socket_nonblocking(tor_socket_t sockfd, struct sockaddr *addr,
                              socklen_t *len)
{
  return tor_accept_socket_with_extensions(sockfd, addr, len, 1, 1);
}

/** As accept(), but counts the number of open sockets and handles
 * socket creation with either of SOCK_CLOEXEC and SOCK_NONBLOCK specified.
 * <b>cloexec</b> and <b>nonblock</b> should be either 0 or 1 to indicate
 * if the corresponding extension should be used.*/
tor_socket_t
tor_accept_socket_with_extensions(tor_socket_t sockfd, struct sockaddr *addr,
                                 socklen_t *len, int cloexec, int nonblock)
{
  tor_socket_t s;

  /* We are about to create a new file descriptor so make sure we have
   * enough of them. */
  if (get_n_open_sockets() >= max_sockets - 1) {
#ifdef _WIN32
    WSASetLastError(WSAEMFILE);
#else
    errno = EMFILE;
#endif
    return TOR_INVALID_SOCKET;
  }

#if defined(HAVE_ACCEPT4) && defined(SOCK_CLOEXEC) \
  && defined(SOCK_NONBLOCK)
  int ext_flags = (cloexec ? SOCK_CLOEXEC : 0) |
                  (nonblock ? SOCK_NONBLOCK : 0);
  s = accept4(sockfd, addr, len, ext_flags);
  if (SOCKET_OK(s))
    goto socket_ok;
  /* If we got an error, see if it is ENOSYS. ENOSYS indicates that,
   * even though we were built on a system with accept4 support, we
   * are running on one without. Also, check for EINVAL, which indicates that
   * we are missing SOCK_CLOEXEC/SOCK_NONBLOCK support. */
  if (errno != EINVAL && errno != ENOSYS)
    return s;
#endif /* defined(HAVE_ACCEPT4) && defined(SOCK_CLOEXEC) ... */

  s = accept(sockfd, addr, len);
  if (!SOCKET_OK(s))
    return s;

#if defined(FD_CLOEXEC)
  if (cloexec) {
    if (fcntl(s, F_SETFD, FD_CLOEXEC) == -1) {
      log_warn(LD_NET, "Couldn't set FD_CLOEXEC: %s", strerror(errno));
      tor_close_socket_simple(s);
      return TOR_INVALID_SOCKET;
    }
  }
#else /* !defined(FD_CLOEXEC) */
  (void)cloexec;
#endif /* defined(FD_CLOEXEC) */

  if (nonblock) {
    if (set_socket_nonblocking(s) == -1) {
      tor_close_socket_simple(s);
      return TOR_INVALID_SOCKET;
    }
  }

  goto socket_ok; /* So that socket_ok will not be unused. */

 socket_ok:
  tor_take_socket_ownership(s);
  return s;
}

/** Return the number of sockets we currently have opened. */
int
get_n_open_sockets(void)
{
  int n;
  socket_accounting_lock();
  n = n_sockets_open;
  socket_accounting_unlock();
  return n;
}

/**
 * Allocate a pair of connected sockets.  (Like socketpair(family,
 * type,protocol,fd), but works on systems that don't have
 * socketpair.)
 *
 * Currently, only (AF_UNIX, SOCK_STREAM, 0) sockets are supported.
 *
 * Note that on systems without socketpair, this call will fail if
 * localhost is inaccessible (for example, if the networking
 * stack is down). And even if it succeeds, the socket pair will not
 * be able to read while localhost is down later (the socket pair may
 * even close, depending on OS-specific timeouts). The socket pair
 * should work on IPv4-only, IPv6-only, and dual-stack systems, as long
 * as they have the standard localhost addresses.
 *
 * Returns 0 on success and -errno on failure; do not rely on the value
 * of errno or WSAGetLastError().
 **/
/* It would be nicer just to set errno, but that won't work for windows. */
int
tor_socketpair(int family, int type, int protocol, tor_socket_t fd[2])
{
  int r;
//don't use win32 socketpairs (they are always bad)
#if defined(HAVE_SOCKETPAIR) && !defined(_WIN32)

#ifdef SOCK_CLOEXEC
  r = socketpair(family, type|SOCK_CLOEXEC, protocol, fd);
  if (r == 0)
    goto sockets_ok;
  /* If we got an error, see if it is EINVAL. EINVAL might indicate that,
   * even though we were built on a system with SOCK_CLOEXEC support, we
   * are running on one without. */
  if (errno != EINVAL)
    return -errno;
#endif /* defined(SOCK_CLOEXEC) */

  r = socketpair(family, type, protocol, fd);
  if (r < 0)
    return -errno;
#else /* !(defined(HAVE_SOCKETPAIR) && !defined(_WIN32)) */
  r = tor_ersatz_socketpair(family, type, protocol, fd);
  if (r < 0)
    return -r;
#endif /* defined(HAVE_SOCKETPAIR) && !defined(_WIN32) */

#if defined(FD_CLOEXEC)
  if (SOCKET_OK(fd[0])) {
    r = fcntl(fd[0], F_SETFD, FD_CLOEXEC);
    if (r == -1) {
      close(fd[0]);
      close(fd[1]);
      return -errno;
    }
  }
  if (SOCKET_OK(fd[1])) {
    r = fcntl(fd[1], F_SETFD, FD_CLOEXEC);
    if (r == -1) {
      close(fd[0]);
      close(fd[1]);
      return -errno;
    }
  }
#endif /* defined(FD_CLOEXEC) */
  goto sockets_ok; /* So that sockets_ok will not be unused. */

 sockets_ok:
  socket_accounting_lock();
  if (SOCKET_OK(fd[0])) {
    ++n_sockets_open;
    mark_socket_open(fd[0]);
  }
  if (SOCKET_OK(fd[1])) {
    ++n_sockets_open;
    mark_socket_open(fd[1]);
  }
  socket_accounting_unlock();

  return 0;
}

/** Mockable wrapper for getsockname(). */
MOCK_IMPL(int,
tor_getsockname,(tor_socket_t sock, struct sockaddr *address,
                 socklen_t *address_len))
{
   return getsockname(sock, address, address_len);
}

/**
 * Find the local address associated with the socket <b>sock</b>, and
 * place it in *<b>addr_out</b>.  Return 0 on success, -1 on failure.
 *
 * (As tor_getsockname, but instead places the result in a tor_addr_t.) */
int
tor_addr_from_getsockname(struct tor_addr_t *addr_out, tor_socket_t sock)
{
  struct sockaddr_storage ss;
  socklen_t ss_len = sizeof(ss);
  memset(&ss, 0, sizeof(ss));

  if (tor_getsockname(sock, (struct sockaddr *) &ss, &ss_len) < 0)
    return -1;

  return tor_addr_from_sockaddr(addr_out, (struct sockaddr *)&ss, NULL);
}

/** Turn <b>socket</b> into a nonblocking socket. Return 0 on success, -1
 * on failure.
 */
int
set_socket_nonblocking(tor_socket_t sock)
{
#if defined(_WIN32)
  unsigned long nonblocking = 1;
  ioctlsocket(sock, FIONBIO, (unsigned long*) &nonblocking);
#else
  int flags;

  flags = fcntl(sock, F_GETFL, 0);
  if (flags == -1) {
    log_warn(LD_NET, "Couldn't get file status flags: %s", strerror(errno));
    return -1;
  }
  flags |= O_NONBLOCK;
  if (fcntl(sock, F_SETFL, flags) == -1) {
    log_warn(LD_NET, "Couldn't set file status flags: %s", strerror(errno));
    return -1;
  }
#endif /* defined(_WIN32) */

  return 0;
}

/** Read from <b>sock</b> to <b>buf</b>, until we get <b>count</b> bytes or
 * reach the end of the file.  Return the number of bytes read, or -1 on
 * error. Only use if fd is a blocking fd. */
ssize_t
read_all_from_socket(tor_socket_t sock, char *buf, size_t count)
{
  size_t numread = 0;
  ssize_t result;

  if (count > SIZE_T_CEILING || count > SSIZE_MAX) {
    errno = EINVAL;
    return -1;
  }

  while (numread < count) {
    result = tor_socket_recv(sock, buf+numread, count-numread, 0);
    if (result<0)
      return -1;
    else if (result == 0)
      break;
    numread += result;
  }
  return (ssize_t)numread;
}

/** Write <b>count</b> bytes from <b>buf</b> to <b>sock</b>. Return the number
 * of bytes written, or -1 on error.  Only use if fd is a blocking fd.  */
ssize_t
write_all_to_socket(tor_socket_t fd, const char *buf, size_t count)
{
  size_t written = 0;
  ssize_t result;
  raw_assert(count < SSIZE_MAX);

  while (written != count) {
    result = tor_socket_send(fd, buf+written, count-written, 0);
    if (result<0)
      return -1;
    written += result;
  }
  return (ssize_t)count;
}

/**
 * On Windows, WSAEWOULDBLOCK is not always correct: when you see it,
 * you need to ask the socket for its actual errno.  Also, you need to
 * get your errors from WSAGetLastError, not errno.  (If you supply a
 * socket of -1, we check WSAGetLastError, but don't correct
 * WSAEWOULDBLOCKs.)
 *
 * The upshot of all of this is that when a socket call fails, you
 * should call tor_socket_errno <em>at most once</em> on the failing
 * socket to get the error.
 */
#if defined(_WIN32)
int
tor_socket_errno(tor_socket_t sock)
{
  int optval, optvallen=sizeof(optval);
  int err = WSAGetLastError();
  if (err == WSAEWOULDBLOCK && SOCKET_OK(sock)) {
    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void*)&optval, &optvallen))
      return err;
    if (optval)
      return optval;
  }
  return err;
}
#endif /* defined(_WIN32) */

#if defined(_WIN32)
#define E(code, s) { code, (s " [" #code " ]") }
struct { int code; const char *msg; } windows_socket_errors[] = {
  E(WSAEINTR, "Interrupted function call"),
  E(WSAEACCES, "Permission denied"),
  E(WSAEFAULT, "Bad address"),
  E(WSAEINVAL, "Invalid argument"),
  E(WSAEMFILE, "Too many open files"),
  E(WSAEWOULDBLOCK,  "Resource temporarily unavailable"),
  E(WSAEINPROGRESS, "Operation now in progress"),
  E(WSAEALREADY, "Operation already in progress"),
  E(WSAENOTSOCK, "Socket operation on nonsocket"),
  E(WSAEDESTADDRREQ, "Destination address required"),
  E(WSAEMSGSIZE, "Message too long"),
  E(WSAEPROTOTYPE, "Protocol wrong for socket"),
  E(WSAENOPROTOOPT, "Bad protocol option"),
  E(WSAEPROTONOSUPPORT, "Protocol not supported"),
  E(WSAESOCKTNOSUPPORT, "Socket type not supported"),
  /* What's the difference between NOTSUPP and NOSUPPORT? :) */
  E(WSAEOPNOTSUPP, "Operation not supported"),
  E(WSAEPFNOSUPPORT,  "Protocol family not supported"),
  E(WSAEAFNOSUPPORT, "Address family not supported by protocol family"),
  E(WSAEADDRINUSE, "Address already in use"),
  E(WSAEADDRNOTAVAIL, "Cannot assign requested address"),
  E(WSAENETDOWN, "Network is down"),
  E(WSAENETUNREACH, "Network is unreachable"),
  E(WSAENETRESET, "Network dropped connection on reset"),
  E(WSAECONNABORTED, "Software caused connection abort"),
  E(WSAECONNRESET, "Connection reset by peer"),
  E(WSAENOBUFS, "No buffer space available"),
  E(WSAEISCONN, "Socket is already connected"),
  E(WSAENOTCONN, "Socket is not connected"),
  E(WSAESHUTDOWN, "Cannot send after socket shutdown"),
  E(WSAETIMEDOUT, "Connection timed out"),
  E(WSAECONNREFUSED, "Connection refused"),
  E(WSAEHOSTDOWN, "Host is down"),
  E(WSAEHOSTUNREACH, "No route to host"),
  E(WSAEPROCLIM, "Too many processes"),
  /* Yes, some of these start with WSA, not WSAE. No, I don't know why. */
  E(WSASYSNOTREADY, "Network subsystem is unavailable"),
  E(WSAVERNOTSUPPORTED, "Winsock.dll out of range"),
  E(WSANOTINITIALISED, "Successful WSAStartup not yet performed"),
  E(WSAEDISCON, "Graceful shutdown now in progress"),
#ifdef WSATYPE_NOT_FOUND
  E(WSATYPE_NOT_FOUND, "Class type not found"),
#endif
  E(WSAHOST_NOT_FOUND, "Host not found"),
  E(WSATRY_AGAIN, "Nonauthoritative host not found"),
  E(WSANO_RECOVERY, "This is a nonrecoverable error"),
  E(WSANO_DATA, "Valid name, no data record of requested type)"),

  /* There are some more error codes whose numeric values are marked
   * <b>OS dependent</b>. They start with WSA_, apparently for the same
   * reason that practitioners of some craft traditions deliberately
   * introduce imperfections into their baskets and rugs "to allow the
   * evil spirits to escape."  If we catch them, then our binaries
   * might not report consistent results across versions of Windows.
   * Thus, I'm going to let them all fall through.
   */
  { -1, NULL },
};
/** There does not seem to be a strerror equivalent for Winsock errors.
 * Naturally, we have to roll our own.
 */
const char *
tor_socket_strerror(int e)
{
  int i;
  for (i=0; windows_socket_errors[i].code >= 0; ++i) {
    if (e == windows_socket_errors[i].code)
      return windows_socket_errors[i].msg;
  }
  return strerror(e);
}
#endif /* defined(_WIN32) */
