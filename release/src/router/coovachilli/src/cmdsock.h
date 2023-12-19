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
#ifndef CMDSOCK_H
#define CMDSOCK_H

typedef enum {
  CMDSOCK_DHCP_LIST=0,
  CMDSOCK_DHCP_RELEASE,
  CMDSOCK_LIST,
  CMDSOCK_SHOW,
  CMDSOCK_AUTHORIZE,
  CMDSOCK_DHCP_DROP,
  CMDSOCK_RELOAD,
  CMDSOCK_PROCS,
  CMDSOCK_UPDATE,
  CMDSOCK_LOGIN,
  CMDSOCK_LOGOUT,
  CMDSOCK_LIST_IPPOOL,
  CMDSOCK_LIST_RADQUEUE,
  CMDSOCK_LIST_GARDEN,
#ifdef ENABLE_STATFILE
  CMDSOCK_STATUSFILE,
#endif
#ifdef ENABLE_CLUSTER
  CMDSOCK_PEERS,
  CMDSOCK_PEER_SET,
#endif
#ifdef ENABLE_MULTIROUTE
  CMDSOCK_ROUTE,
  CMDSOCK_ROUTE_SET,
  CMDSOCK_ROUTE_GW,
#endif
  CMDSOCK_ADD_GARDEN,
  CMDSOCK_REM_GARDEN,
#ifdef ENABLE_INSPECT
  CMDSOCK_INSPECT,
#endif
#if defined(ENABLE_LOCATION) && defined(HAVE_AVL)
 CMDSOCK_LISTLOC,
 CMDSOCK_LISTLOCSUM,
#endif
} chilli_cmdtype;
#define  CMDSOCK_OPT_JSON      (1)

#include "pkt.h"
#include "session.h"

struct cmdsock_request { 
  uint16_t type;
  uint16_t options;
  unsigned char mac[PKT_ETH_ALEN];
  struct in_addr ip;
  union {
    struct cmdsock_session {
      char username[256];
      char password[256];
      char sessionid[17];
#ifdef ENABLE_LOCATION
      char location[MAX_LOCATION_LENGTH];
#endif
      struct session_params params;
    } sess;
    char data[1024];
  } d;
}  __attribute__((packed));

typedef struct cmdsock_request CMDSOCK_REQUEST;

#endif /* CMDSOCK */
