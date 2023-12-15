/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2003-2005 Mondru AB., 
 * Copyright (C) 2006 PicoPoint B.V.
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

#include "chilli.h"

#define ADMIN_TIMEOUT 10

struct options_t _options;

static int chilliauth_cb(struct radius_t *radius,
			 struct radius_packet_t *pack,
			 struct radius_packet_t *pack_req, void *cbp) {
  struct radius_attr_t *attr = NULL;
  /*char attrs[RADIUS_ATTR_VLEN+1];*/
  size_t offset = 0;

  if (!pack) { 
    log_err(0, "Radius request timed out");
    return 0;
  }

  if ((pack->code != RADIUS_CODE_ACCESS_REJECT) && 
      (pack->code != RADIUS_CODE_ACCESS_CHALLENGE) &&
      (pack->code != RADIUS_CODE_ACCESS_ACCEPT)) {
    log_err(0, "Unknown radius access reply code %d", pack->code);
    return 0;
  }

  /* ACCESS-ACCEPT */
  if (pack->code != RADIUS_CODE_ACCESS_ACCEPT) {
    log_err(0, "Administrative-User Login Failed");
    return 0;
  }

  while (!radius_getnextattr(pack, &attr, 
			     RADIUS_ATTR_VENDOR_SPECIFIC,
			     RADIUS_VENDOR_CHILLISPOT,
			     RADIUS_ATTR_CHILLISPOT_CONFIG, 
			     0, &offset)) {
    printf("%.*s\n", attr->l - 2, (const char *)attr->v.t);
  }
  
  return 0;
}

int static chilliauth() {
  unsigned char hwaddr[6];
  struct radius_t *radius=0;
  struct timeval idleTime;
  int endtime, now;
  int maxfd = 0;
  fd_set fds;
  int status;
  int ret=-1;

  if (!_options.adminuser || !_options.adminpasswd) {
    log_err(0, "Must be used with --adminuser and --adminpasswd");
    return 1;
  }

  if (radius_new(&radius, &_options.radiuslisten, 0, 0, 0) ||
      radius_init_q(radius, 4)) {
    log_err(0, "Failed to create radius");
    return ret;
  }

  /* get dhcpif mac */
  memset(hwaddr, 0, sizeof(hwaddr));

#ifdef SIOCGIFHWADDR
  if (!_options.nasmac && _options.dhcpif) {
    struct ifreq ifr;
    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
      memset(&ifr, 0, sizeof(ifr));
      safe_strncpy(ifr.ifr_name, _options.dhcpif, IFNAMSIZ);
      if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
	log_err(errno, "ioctl(d=%d, request=%d) failed", fd, SIOCGIFHWADDR);
      }
      memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, PKT_ETH_ALEN);
      close(fd);
    }
  }
#endif

  radius_set(radius, hwaddr, (_options.debug & DEBUG_RADIUS));
  radius_set_cb_auth_conf(radius, chilliauth_cb); 

  ret = chilli_auth_radius(radius);

  if (radius->fd <= 0) {
    log_err(0, "not a valid socket!");
    return ret;
  } 

  maxfd = radius->fd;

  now = time(NULL);
  endtime = now + ADMIN_TIMEOUT; 

  while (endtime > now) {

    FD_ZERO(&fds);
    FD_SET(radius->fd, &fds);
    
    idleTime.tv_sec = 0;
    idleTime.tv_usec = REDIR_RADIUS_SELECT_TIME;
    radius_timeleft(radius, &idleTime);

    switch (status = select(maxfd + 1, &fds, NULL, NULL, &idleTime)) {
    case -1:
      log_err(errno, "select() returned -1!");
      break;  
    case 0:
      radius_timeout(radius);
    default:
      break;
    }

    if (status > 0) {
      if (FD_ISSET(radius->fd, &fds)) {
	if (radius_decaps(radius, 0) < 0) {
	  log_err(0, "radius_ind() failed!");
	}
	else {
	  ret = 0;
	}
	break;
      }
    }

    now = time(NULL);
  }  

  radius_free(radius);
  return ret;
}

int main(int argc, char **argv)
{
  int ret;
  options_init();

  if (process_options(argc, argv, 1)) {
    log_err(errno, "Exiting...");
    exit(1);
  }
  
  ret = chilliauth();
  options_cleanup();
  return ret;
}
