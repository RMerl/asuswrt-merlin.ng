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
 */

#define MAIN_FILE

#include "system.h"
#include <asm/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "chilli.h"
#include "rtmon.h"

struct options_t _options;

static int debug = 1;
static int chilli_pid = 0;
static char * chilli_conf = "/tmp/local.conf";

static int proc_route(struct rtmon_t *rtmon, 
		      struct rtmon_iface *iface,
		      struct rtmon_route *route) {

  rtmon_print_ifaces(rtmon, 1);
  rtmon_print_routes(rtmon, 1);
  return 0;
}

int main(int argc, char *argv[]) {
  int keep_going = 1;
  int reload_config = 1;
  int i;

  struct rtmon_t _rtmon;

  /*int selfpipe = selfpipe_init();*/

  memset(&_options, 0, sizeof(_options));

  if (rtmon_init(&_rtmon, proc_route)) {
    err(1,"netlink");
  }

  for (i=1; i < argc; i++) {
    if (strcmp(argv[i], "-debug")==0) {
      debug = 1;
    } else if (strcmp(argv[i], "-file")==0) {
      chilli_conf = argv[i+1];
    } else if (strcmp(argv[i], "-pid")==0) {
      chilli_pid = atoi(argv[i+1]);
    }
  }

  _options.foreground = debug;
  _options.debug = debug;

  log_dbg("running");

  chilli_signals(&keep_going, &reload_config);

  rtmon_discover_ifaces(&_rtmon);
  rtmon_discover_routes(&_rtmon);

  if (debug) {
    rtmon_print_ifaces(&_rtmon, 1);
    rtmon_print_routes(&_rtmon, 1);
  }

  rtmon_check_updates(&_rtmon);
  
  while (keep_going) {
    /* select */

    /* check selfpipe */

    rtmon_read_event(&_rtmon);
  }

  /* selfpipe_finish();*/

  return 0;
}
