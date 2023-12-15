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

#ifndef _SESSION_H
#define _SESSION_H

#include "chilli_limits.h"
#include "garden.h"

struct session_params {
  uint8_t url[REDIR_USERURLSIZE];
  uint8_t filteridbuf[256];
  uint8_t filteridlen;
  uint8_t routeidx;
  uint64_t bandwidthmaxup;
  uint64_t bandwidthmaxdown;
  uint64_t maxinputoctets;
  uint64_t maxoutputoctets;
  uint64_t maxtotaloctets;
  uint64_t sessiontimeout;
  uint32_t idletimeout;
  uint16_t interim_interval;     /* Seconds. 0 = No interim accounting */
  time_t sessionterminatetime;

#define REQUIRE_UAM_AUTH   (1<<0)
#define REQUIRE_UAM_SPLASH (1<<1)
#define REQUIRE_REDIRECT   (1<<2)
#define IS_UAM_REAUTH      (1<<3)
#define NO_ACCOUNTING      (1<<4)
#define NO_SCRIPT          (1<<5)
#define ADMIN_LOGIN        (1<<6)
#define UAM_INJECT_URL     (1<<7)
#define UAM_CLEAR_URL      (1<<8)
  uint16_t flags;

#ifdef ENABLE_SESSGARDEN
  pass_through pass_throughs[SESSION_PASS_THROUGH_MAX];
  uint32_t pass_through_count;
#endif
} __attribute__((packed));


struct redir_state {

  char username[REDIR_USERNAMESIZE];
  char userurl[REDIR_USERURLSIZE];

  uint8_t uamchal[REDIR_MD5LEN];

  /* To store the RADIUS CLASS attribute received in the Access Accept */
  uint8_t classbuf[RADIUS_ATTR_VLEN];
  size_t classlen;

  /* To store the RADIUS CUI attribute received in the Access Accept */
  uint8_t cuibuf[RADIUS_ATTR_VLEN];
  size_t cuilen;
  
  /* To store the RADIUS STATE attribute between Radius requests */
  uint8_t statebuf[RADIUS_ATTR_VLEN];
  uint8_t statelen;

  /*  EAP identity of the last request sent */
  uint8_t eap_identity;

  /* UAM protocol used */
  uint8_t uamprotocol;

#ifdef ENABLE_USERAGENT
  char useragent[REDIR_USERAGENTSIZE]; 
#endif

#ifdef ENABLE_ACCEPTLANGUAGE
  char acceptlanguage[128];
#endif

#ifdef ENABLE_PROXYVSA
#define RADIUS_PROXYVSA 256
  uint8_t called[RADIUS_ATTR_VLEN];
  uint8_t calledlen;
  uint8_t vsa[RADIUS_PROXYVSA];
  size_t vsalen;
#endif

} __attribute__((packed));

struct session_state {
  struct redir_state redir;

  int authenticated; /* 1 if user was authenticated */  

  char sessionid[REDIR_SESSIONID_LEN];
#ifdef ENABLE_SESSIONID
  char chilli_sessionid[56]; 
#endif
#ifdef ENABLE_APSESSIONID
  char ap_sessionid[128]; 
#endif

  time_t start_time;
  time_t interim_time;

  time_t last_sent_time; /* Last time a packet was sent. Used for idle timeout calculations */
  time_t last_time; /* Last time a packet was received or sent */
  time_t uamtime;

  uint64_t input_packets;
  uint64_t output_packets;
  uint64_t input_octets;
  uint64_t output_octets;
  uint32_t terminate_cause;
  uint32_t session_id;

#ifdef ENABLE_GARDENACCOUNTING
  char garden_sessionid[REDIR_SESSIONID_LEN];
  time_t garden_start_time;
  time_t garden_interim_time;
  uint64_t garden_input_octets;
  uint64_t garden_output_octets;
  uint64_t other_input_octets;
  uint64_t other_output_octets;
#endif

#if defined(ENABLE_LOCATION) && defined(HAVE_AVL)
  uint64_t input_octets_old;
  uint64_t output_octets_old;
#ifdef ENABLE_GARDENACCOUNTING
  uint64_t garden_input_octets_old;
  uint64_t garden_output_octets_old;
  uint64_t other_input_octets_old;
  uint64_t other_output_octets_old;
#endif
#endif

#ifdef ENABLE_SESSIONSTATE
  uint32_t session_state;
#endif

#ifdef ENABLE_IEEE8021Q
  uint16_t tag8021q;
#endif

#ifdef ENABLE_LOCATION
#define MAX_LOCATION_LENGTH 56
  char location[MAX_LOCATION_LENGTH];
  char pending_location[MAX_LOCATION_LENGTH];
  uint16_t location_changes;
#endif

#ifdef ENABLE_MULTILAN
#define app_conn_idx(x)       ((x)->s_state.lanidx)
#define app_conn_set_idx(x,c) ((x)->s_state.lanidx = (c)->lanidx)
  int lanidx;
#else
#define app_conn_idx(x) 0
#define app_conn_set_idx(x,c) 
#endif

#ifdef ENABLE_LEAKYBUCKET
  /* Leaky bucket */
  uint64_t bucketup;
  uint64_t bucketdown;
  uint64_t bucketupsize;
  uint64_t bucketdownsize;
#endif

} __attribute__((packed));

#endif
