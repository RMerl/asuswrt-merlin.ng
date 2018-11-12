/*-*- Mode: C; c-basic-offset: 8; indent-tabs-mode: nil -*-*/

#ifndef SNMPD_SD_DAEMON_H
#define SNMPD_SD_DAEMON_H

/***
  Copyright 2010 Lennart Poettering

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation files
  (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge,
  publish, distribute, sublicense, and/or sell copies of the Software,
  and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
***/

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
  Reference implementation of a few systemd related interfaces for
  writing daemons. These interfaces are trivial to implement. To
  simplify porting we provide this reference implementation.
  Applications are welcome to reimplement the algorithms described
  here if they do not want to include these two source files.

  The following functionality is provided:

  - Support for logging with log levels on stderr
  - File descriptor passing for socket-based activation
  - Daemon startup and status notification
  - Detection of systemd boots

  You may compile this with -DDISABLE_SYSTEMD to disable systemd
  support. This makes all those calls NOPs that are directly related to
  systemd (i.e. only sd_is_xxx() will stay useful).

  Since this is drop-in code we don't want any of our symbols to be
  exported in any case. Hence we declare hidden visibility for all of
  them.

  You may find an up-to-date version of these source files online:

  http://cgit.freedesktop.org/systemd/plain/src/sd-daemon.h
  http://cgit.freedesktop.org/systemd/plain/src/sd-daemon.c

  This should compile on non-Linux systems, too, but with the
  exception of the sd_is_xxx() calls all functions will become NOPs.

  See sd-daemon(7) for more information.
*/

/*
  Returns how many file descriptors have been passed, or a negative
  errno code on failure. Optionally, removes the $LISTEN_FDS and
  $LISTEN_PID file descriptors from the environment (recommended, but
  problematic in threaded environments). If r is the return value of
  this function you'll find the file descriptors passed as fds
  SD_LISTEN_FDS_START to SD_LISTEN_FDS_START+r-1. Returns a negative
  errno style error code on failure. This function call ensures that
  the FD_CLOEXEC flag is set for the passed file descriptors, to make
  sure they are not passed on to child processes. If FD_CLOEXEC shall
  not be set, the caller needs to unset it after this call for all file
  descriptors that are used.

  See sd_listen_fds(3) for more information.
*/
int netsnmp_sd_listen_fds(int unset_environment);

/*
  Informs systemd about changed daemon state. This takes a number of
  newline separated environment-style variable assignments in a
  string. The following variables are known:

     READY=1      Tells systemd that daemon startup is finished (only
                  relevant for services of Type=notify). The passed
                  argument is a boolean "1" or "0". Since there is
                  little value in signaling non-readiness the only
                  value daemons should send is "READY=1".

     STATUS=...   Passes a single-line status string back to systemd
                  that describes the daemon state. This is free-from
                  and can be used for various purposes: general state
                  feedback, fsck-like programs could pass completion
                  percentages and failing programs could pass a human
                  readable error message. Example: "STATUS=Completed
                  66% of file system check..."

     ERRNO=...    If a daemon fails, the errno-style error code,
                  formatted as string. Example: "ERRNO=2" for ENOENT.

     BUSERROR=... If a daemon fails, the D-Bus error-style error
                  code. Example: "BUSERROR=org.freedesktop.DBus.Error.TimedOut"

     MAINPID=...  The main pid of a daemon, in case systemd did not
                  fork off the process itself. Example: "MAINPID=4711"

  Daemons can choose to send additional variables. However, it is
  recommended to prefix variable names not listed above with X_.

  Returns a negative errno-style error code on failure. Returns > 0
  if systemd could be notified, 0 if it couldn't possibly because
  systemd is not running.

  Example: When a daemon finished starting up, it could issue this
  call to notify systemd about it:

     sd_notify(0, "READY=1");

  See sd_notifyf() for more complete examples.

  See sd_notify(3) for more information.
*/
int netsnmp_sd_notify(int unset_environment, const char *state);

/**
 * Find an socket with given parameters. See man sd_is_socket_inet for
 * description of the arguments.
 *
 * Returns the file descriptor if it is found, 0 otherwise.
 */
int netsnmp_sd_find_inet_socket(int family, int type, int listening, int port);

/**
 * Find an unix socket with given parameters. See man sd_is_socket_unix for
 * description of the arguments.
 *
 * Returns the file descriptor if it is found, 0 otherwise.
 */
int
netsnmp_sd_find_unix_socket(int type, int listening, const char *path);

#ifdef __cplusplus
}
#endif

#endif /* SNMPD_SD_DAEMON_H */
