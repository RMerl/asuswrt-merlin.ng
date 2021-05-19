/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file alertsock.c
 *
 * \brief Use a socket to alert the main thread from a worker thread.
 *
 * Because our main loop spends all of its time in select, epoll, kqueue, or
 * etc, we need a way to wake up the main loop from another thread.  This code
 * tries to provide the fastest reasonable way to do that, depending on our
 * platform.
 **/

#include "orconfig.h"
#include "lib/net/alertsock.h"
#include "lib/net/socket.h"
#include "lib/log/util_bug.h"

#ifdef HAVE_SYS_EVENTFD_H
#include <sys/eventfd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef _WIN32
#include <winsock2.h>
#endif

#if defined(HAVE_EVENTFD) || defined(HAVE_PIPE)
/* As write(), but retry on EINTR, and return the negative error code on
 * error. */
static int
write_ni(int fd, const void *buf, size_t n)
{
  int r;
 again:
  r = (int) write(fd, buf, n);
  if (r < 0) {
    if (errno == EINTR)
      goto again;
    else
      return -errno;
  }
  return r;
}
/* As read(), but retry on EINTR, and return the negative error code on error.
 */
static int
read_ni(int fd, void *buf, size_t n)
{
  int r;
 again:
  r = (int) read(fd, buf, n);
  if (r < 0) {
    if (errno == EINTR)
      goto again;
    else
      return -errno;
  }
  return r;
}
#endif /* defined(HAVE_EVENTFD) || defined(HAVE_PIPE) */

/** As send(), but retry on EINTR, and return the negative error code on
 * error. */
static int
send_ni(int fd, const void *buf, size_t n, int flags)
{
  int r;
 again:
  r = (int) send(fd, buf, n, flags);
  if (r < 0) {
    int error = tor_socket_errno(fd);
    if (ERRNO_IS_EINTR(error))
      goto again;
    else
      return -error;
  }
  return r;
}

/** As recv(), but retry on EINTR, and return the negative error code on
 * error. */
static int
recv_ni(int fd, void *buf, size_t n, int flags)
{
  int r;
 again:
  r = (int) recv(fd, buf, n, flags);
  if (r < 0) {
    int error = tor_socket_errno(fd);
    if (ERRNO_IS_EINTR(error))
      goto again;
    else
      return -error;
  }
  return r;
}

#ifdef HAVE_EVENTFD
/* Increment the event count on an eventfd <b>fd</b> */
static int
eventfd_alert(int fd)
{
  uint64_t u = 1;
  int r = write_ni(fd, (void*)&u, sizeof(u));
  if (r < 0 && -r != EAGAIN)
    return -1;
  return 0;
}

/* Drain all events from an eventfd <b>fd</b>. */
static int
eventfd_drain(int fd)
{
  uint64_t u = 0;
  int r = read_ni(fd, (void*)&u, sizeof(u));
  if (r < 0 && -r != EAGAIN)
    return r;
  return 0;
}
#endif /* defined(HAVE_EVENTFD) */

#ifdef HAVE_PIPE
/** Send a byte over a pipe. Return 0 on success or EAGAIN; -1 on error */
static int
pipe_alert(int fd)
{
  ssize_t r = write_ni(fd, "x", 1);
  if (r < 0 && -r != EAGAIN)
    return (int)r;
  return 0;
}

/** Drain all input from a pipe <b>fd</b> and ignore it.  Return 0 on
 * success, -1 on error. */
static int
pipe_drain(int fd)
{
  char buf[32];
  ssize_t r;
  do {
    r = read_ni(fd, buf, sizeof(buf));
  } while (r > 0);
  if (r < 0 && errno != EAGAIN)
    return -errno;
  /* A value of r = 0 means EOF on the fd so successfully drained. */
  return 0;
}
#endif /* defined(HAVE_PIPE) */

/** Send a byte on socket <b>fd</b>t.  Return 0 on success or EAGAIN,
 * -1 on error. */
static int
sock_alert(tor_socket_t fd)
{
  ssize_t r = send_ni(fd, "x", 1, 0);
  if (r < 0 && !ERRNO_IS_EAGAIN(-r))
    return (int)r;
  return 0;
}

/** Drain all the input from a socket <b>fd</b>, and ignore it.  Return 0 on
 * success, -errno on error. */
static int
sock_drain(tor_socket_t fd)
{
  char buf[32];
  ssize_t r;
  do {
    r = recv_ni(fd, buf, sizeof(buf), 0);
  } while (r > 0);
  if (r < 0 && !ERRNO_IS_EAGAIN(-r))
    return (int)r;
  /* A value of r = 0 means EOF on the fd so successfully drained. */
  return 0;
}

/** Allocate a new set of alert sockets, and set the appropriate function
 * pointers, in <b>socks_out</b>. */
int
alert_sockets_create(alert_sockets_t *socks_out, uint32_t flags)
{
  tor_socket_t socks[2] = { TOR_INVALID_SOCKET, TOR_INVALID_SOCKET };

#ifdef HAVE_EVENTFD
  /* First, we try the Linux eventfd() syscall.  This gives a 64-bit counter
   * associated with a single file descriptor. */
#if defined(EFD_CLOEXEC) && defined(EFD_NONBLOCK)
  if (!(flags & ASOCKS_NOEVENTFD2))
    socks[0] = eventfd(0, EFD_CLOEXEC|EFD_NONBLOCK);
#endif
  if (socks[0] < 0 && !(flags & ASOCKS_NOEVENTFD)) {
    socks[0] = eventfd(0,0);
    if (socks[0] >= 0) {
      if (fcntl(socks[0], F_SETFD, FD_CLOEXEC) < 0 ||
          set_socket_nonblocking(socks[0]) < 0) {
        // LCOV_EXCL_START -- if eventfd succeeds, fcntl will.
        tor_assert_nonfatal_unreached();
        close(socks[0]);
        return -1;
        // LCOV_EXCL_STOP
      }
    }
  }
  if (socks[0] >= 0) {
    socks_out->read_fd = socks_out->write_fd = socks[0];
    socks_out->alert_fn = eventfd_alert;
    socks_out->drain_fn = eventfd_drain;
    return 0;
  }
#endif /* defined(HAVE_EVENTFD) */

#ifdef HAVE_PIPE2
  /* Now we're going to try pipes. First type the pipe2() syscall, if we
   * have it, so we can save some calls... */
  if (!(flags & ASOCKS_NOPIPE2) &&
      pipe2(socks, O_NONBLOCK|O_CLOEXEC) == 0) {
    socks_out->read_fd = socks[0];
    socks_out->write_fd = socks[1];
    socks_out->alert_fn = pipe_alert;
    socks_out->drain_fn = pipe_drain;
    return 0;
  }
#endif /* defined(HAVE_PIPE2) */

#ifdef HAVE_PIPE
  /* Now try the regular pipe() syscall.  Pipes have a bit lower overhead than
   * socketpairs, fwict. */
  if (!(flags & ASOCKS_NOPIPE) &&
      pipe(socks) == 0) {
    if (fcntl(socks[0], F_SETFD, FD_CLOEXEC) < 0 ||
        fcntl(socks[1], F_SETFD, FD_CLOEXEC) < 0 ||
        set_socket_nonblocking(socks[0]) < 0 ||
        set_socket_nonblocking(socks[1]) < 0) {
      // LCOV_EXCL_START -- if pipe succeeds, you can fcntl the output
      tor_assert_nonfatal_unreached();
      close(socks[0]);
      close(socks[1]);
      return -1;
      // LCOV_EXCL_STOP
    }
    socks_out->read_fd = socks[0];
    socks_out->write_fd = socks[1];
    socks_out->alert_fn = pipe_alert;
    socks_out->drain_fn = pipe_drain;
    return 0;
  }
#endif /* defined(HAVE_PIPE) */

  /* If nothing else worked, fall back on socketpair(). */
  if (!(flags & ASOCKS_NOSOCKETPAIR) &&
      tor_socketpair(AF_UNIX, SOCK_STREAM, 0, socks) == 0) {
    if (set_socket_nonblocking(socks[0]) < 0 ||
        set_socket_nonblocking(socks[1])) {
      // LCOV_EXCL_START -- if socketpair worked, you can make it nonblocking.
      tor_assert_nonfatal_unreached();
      tor_close_socket(socks[0]);
      tor_close_socket(socks[1]);
      return -1;
      // LCOV_EXCL_STOP
    }
    socks_out->read_fd = socks[0];
    socks_out->write_fd = socks[1];
    socks_out->alert_fn = sock_alert;
    socks_out->drain_fn = sock_drain;
    return 0;
  }
  return -1;
}

/** Close the sockets in <b>socks</b>. */
void
alert_sockets_close(alert_sockets_t *socks)
{
  if (socks->alert_fn == sock_alert) {
    /* they are sockets. */
    tor_close_socket(socks->read_fd);
    tor_close_socket(socks->write_fd);
  } else {
    close(socks->read_fd);
    if (socks->write_fd != socks->read_fd)
      close(socks->write_fd);
  }
  socks->read_fd = socks->write_fd = -1;
}
