/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2018 Igalia S.L.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <dlfcn.h>

/* This is used in gsocketclient-slow.c used to test
 * and get coverage on how GSocketClient reacts to
 * slow connections.
 */
int
connect (int                    sockfd,
         const struct sockaddr *addr,
         socklen_t              addrlen)
{
  static int (*real_connect)(int, const struct sockaddr *, socklen_t);

  if (real_connect == NULL)
    real_connect = dlsym (RTLD_NEXT, "connect");

  /* This is long enough for multiple connection attempts to be done
   * in parallel given that their timeout is 250ms */
  usleep (600 * 1000);
  return real_connect (sockfd, addr, addrlen);
}
