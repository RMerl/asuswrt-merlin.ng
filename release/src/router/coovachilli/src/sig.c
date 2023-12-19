/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * modified from public domain.
 */

#include "chilli.h"

static int selfpipe[2] = { -1, -1 };

static void _trigger (int s) {
  char c = (char)s;
  /*log_dbg("PID %d SIG Trigger %d", getpid(), s);*/
  safe_write(selfpipe[1], &c, 1);
}

static void _ignore (int s) {
  /*log_dbg("PID %d SIG Ignore %d", getpid(), s);*/
}

static void selfpipe_close (void) {
  register int e = errno;
  safe_close(selfpipe[1]);
  safe_close(selfpipe[0]);
  selfpipe[0] = selfpipe[1] = -1;
  errno = e;
}

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

int ndelay_on (int fd) {
  register int got = fcntl(fd, F_GETFL);
  return (got == -1) ? -1 : fcntl(fd, F_SETFL, got | O_NONBLOCK);
}

int ndelay_off (int fd) {
  register int got = fcntl(fd, F_GETFL);
  return (got == -1) ? -1 : fcntl(fd, F_SETFL, got & ~O_NONBLOCK);
}

int coe (int fd) {
   register int flags = fcntl(fd, F_GETFD, 0);
   if (flags == -1) return -1;
   return fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
}

int selfpipe_init (void) {
  if (selfpipe[0] >= 0) return (errno = EBUSY, -1);
  if (pipe(selfpipe) == -1) return -1;
  if ((ndelay_on(selfpipe[1]) == -1)
   || (coe(selfpipe[1]) == -1)
   || (ndelay_on(selfpipe[0]) == -1)
   || (coe(selfpipe[0]) == -1))
    selfpipe_close();
  return selfpipe[0];
}

int selfpipe_read (void) {
  char c;
  int r = safe_read(selfpipe[0], &c, 1);
  return (r <= 0) ? r : (int)c;
}

void selfpipe_finish (void) {
  selfpipe_close();
}

int set_signal (int signo, void (*func)(int)) {
  struct sigaction act;
  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  if (signo == SIGALRM) {
#ifdef  SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;
#endif
  } else {
#ifdef  SA_RESTART
    act.sa_flags |= SA_RESTART;
#endif
  }
  return sigaction(signo, &act, NULL);
}

int selfpipe_trap (int signo) {
  return set_signal(signo, _trigger);
}

int selfpipe_ignore (int signo) {
  return set_signal(signo, _ignore);
}
