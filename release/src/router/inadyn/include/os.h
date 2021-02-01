/*
 * Copyright (C) 2003-2004  Narcis Ilisei
 * Copyright (C) 2006       Steve Horbachuk
 * Copyright (C) 2010-2020  Joachim Nilsson <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifndef INADYN_OS_H_
#define INADYN_OS_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <syslog.h>

#ifndef IN_PRIVATE
#define IN_PRIVATE(addr) (((addr & IN_CLASSA_NET) == 0x0a000000) ||  \
                          ((addr & 0xfff00000)    == 0xac100000) ||  \
                          ((addr & IN_CLASSB_NET) == 0xc0a80000))
#endif

#ifndef IN_LINKLOCAL
#define IN_LINKLOCALNETNUM	0xa9fe0000
#define IN_LINKLOCAL(addr) ((addr & IN_CLASSB_NET) == IN_LINKLOCALNETNUM)
#endif

#ifndef IN_LOOPBACK
#define IN_LOOPBACK(addr) ((addr & IN_CLASSA_NET) == 0x7f000000)
#endif

#ifndef IN_ZERONET
#define IN_ZERONET(addr) ((addr & IN_CLASSA_NET) == 0)
#endif

#include "error.h"

int os_install_signal_handler (void *ctx);
int os_check_perms            (void);
int os_shell_execute          (char *cmd, char *ip, char *hostname, char *event, int error);

#endif /* INADYN_OS_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
