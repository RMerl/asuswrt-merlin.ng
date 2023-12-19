/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
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
 */

#include "chilli.h"
#include "debug.h"
#ifdef ENABLE_MODULES
#include "chilli_module.h"
#endif

int safe_accept(int fd, struct sockaddr *sa, socklen_t *lenptr) {
  int ret;
  do {
    ret = accept(fd, sa, lenptr);
  } while (ret == -1 && errno == EINTR);
  return ret;
}

int safe_select(int nfds, fd_set *readfds, fd_set *writefds,
		fd_set *exceptfds, struct timeval *timeout) {
  int ret;
  do {
    ret = select(nfds, readfds, writefds, exceptfds, timeout);
  } while (ret == -1 && errno == EINTR);
  return ret;
}

#ifdef USING_POLL
#ifdef HAVE_SYS_EPOLL_H
int safe_epoll_wait(int epfd, struct epoll_event *events,
		    int maxevents, int timeout) {
  int ret;
  do {
    ret = epoll_wait(epfd, events, maxevents, timeout);
  } while (ret == -1 && errno == EINTR);
  return ret;
}
#else
int safe_poll(struct pollfd *fds, nfds_t nfds, int timeout) {
  int ret;
  do {
    ret = poll(fds, nfds, timeout);
  } while (ret == -1 && errno == EINTR);
  return ret;
}
#endif
#endif

int safe_connect(int s, struct sockaddr *sock, size_t len) {
  int ret;
  do {
    ret = connect(s, sock, len);
  } while (ret == -1 && errno == EINTR);
  return ret;
}

int safe_read(int s, void *b, size_t blen) {
  int ret;
  do {
    ret = read(s, b, blen);
  } while (ret == -1 && errno == EINTR);
  return ret;
}

int safe_write(int s, void *b, size_t blen) {
  int ret;
  do {
    ret = write(s, b, blen);
  } while (ret == -1 && errno == EINTR);
  return ret;
}

int safe_recv(int sockfd, void *buf, size_t len, int flags) {
  int ret;
  do {
    ret = recv(sockfd, buf, len, flags);
  } while (ret == -1 && errno == EINTR);
  return ret;
}

int safe_send(int sockfd, void *buf, size_t len, int flags) {
  int ret;
  do {
    ret = send(sockfd, buf, len, flags);
  } while (ret == -1 && errno == EINTR);
  return ret;
}

int safe_recvfrom(int sockfd, void *buf, size_t len, int flags,
		  struct sockaddr *src_addr, socklen_t *addrlen) {
  int ret;
  do {
    ret = recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
  } while (ret == -1 && errno == EINTR);
  return ret;
}

int safe_recvmsg(int sockfd, struct msghdr *msg, int flags) {
  int ret;
  do {
    ret = recvmsg(sockfd, msg, flags);
  } while (ret == -1 && errno == EINTR);
  return ret;
}

int safe_sendto(int s, const void *b, size_t blen, int flags,
		const struct sockaddr *dest_addr, socklen_t addrlen) {
  int ret;
  do {
    ret = sendto(s, b, blen, flags, dest_addr, addrlen);
  } while (ret == -1 && errno == EINTR);
  return ret;
}

int safe_close (int fd) {
  int ret;
  do {
    ret = close(fd);
  } while (ret == -1 && errno == EINTR);
  return ret;
}

pid_t safe_fork() {
  pid_t pid;
  do {
    pid = fork();
  } while (pid == -1 && errno == EINTR);
  return pid;
}
