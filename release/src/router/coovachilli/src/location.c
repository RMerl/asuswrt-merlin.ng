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

#include "chilli.h"
#ifdef ENABLE_MODULES
#include "chilli_module.h"
#endif

#ifdef HAVE_AVL
struct avl_tree loc_search_tree;
static int location_count=0;

void location_close_conn(struct app_conn_t *conn, int close) {

  log_dbg("removing(%s) one of %d sessions from %s",
	  close ? "closing" : "roaming out",
	  conn->loc_search_node->total_sess_count,
	  conn->loc_search_node->value);
  
  conn->loc_search_node->total_sess_count--;
  
  if (close) conn->loc_search_node->closed_sess_count++;
  else conn->loc_search_node->roamed_out_sess_count++;
  
  conn->loc_search_node->closed_bytes_up +=
    (conn->s_state.output_octets_old - 
     conn->s_state.output_octets);
  
  conn->loc_search_node->closed_bytes_down +=
    (conn->s_state.input_octets_old - 
     conn->s_state.input_octets);
  
  conn->s_state.output_octets_old =
    conn->s_state.output_octets;
  conn->s_state.input_octets_old =
    conn->s_state.input_octets;
  
#ifdef ENABLE_GARDENACCOUNTING
  if (_options.uamgardendata) {
    conn->loc_search_node->garden_closed_bytes_up +=
      (conn->s_state.garden_output_octets_old - 
       conn->s_state.garden_output_octets);
    
    conn->loc_search_node->garden_closed_bytes_down += 
      (conn->s_state.garden_input_octets_old - 
       conn->s_state.garden_input_octets);
    
    conn->s_state.garden_output_octets_old =
      conn->s_state.garden_output_octets;

    conn->s_state.garden_input_octets_old =
      conn->s_state.garden_input_octets;

    conn->loc_search_node->other_closed_bytes_up +=
      (conn->s_state.other_output_octets_old -
       conn->s_state.other_output_octets);

    conn->loc_search_node->other_closed_bytes_down +=
      (conn->s_state.other_input_octets_old -
       conn->s_state.other_input_octets);

    conn->s_state.other_output_octets_old =
      conn->s_state.other_output_octets;

    conn->s_state.other_input_octets_old = 
      conn->s_state.other_input_octets;
  }
#endif

  list_remove(&conn->loc_sess);
}

static int
avl_comp(const void *k1, const void *k2, 
	 void *ptr __attribute__ ((unused))) {
  int result = strncmp(k1, k2, MAX_LOCATION_LENGTH);
  log_dbg("%s result %d",__FUNCTION__,result);
  /* log_dbg("k1: %s k2: %s",k1,k2); */
  return result;
}

struct loc_search_t *location_find(char *loc) {
  log_dbg("looking for location: %s", loc);
  return (struct loc_search_t *)avl_find(&loc_search_tree, loc);
}

void location_add_conn(struct app_conn_t *appconn, char *loc) {
  struct loc_search_t *loc_search;

  if (appconn->loc_search_node!=NULL) {

    location_close_conn(appconn,0);

  } else list_init_node(&appconn->loc_sess);

  loc_search = (struct loc_search_t *)avl_find(&loc_search_tree, loc);

  log_dbg("checking location: %s", loc);
  if (loc_search == NULL) {
    location_count++;
    log_dbg("creating tree entry %d for location: %s",
	    location_count, loc);
    loc_search=calloc(1, sizeof(*loc_search));
    memcpy(loc_search->value,loc,sizeof(loc_search->value));
    loc_search->node.key = loc_search->value;
    loc_search->last_queried = mainclock_now();
    /*loc_search->total_sess_count=0;
      loc_search->loc_closed_bytes_up=0;
      loc_search->loc_closed_bytes_down=0;*/
    list_init_head(&loc_search->loc_sess_head);
    if (avl_insert(&loc_search_tree, &loc_search->node)!=0) {
      log_err(0, "unexpected avl error!");
    }
  }

  list_add_head(&loc_search->loc_sess_head, &appconn->loc_sess); 
  loc_search->total_sess_count++;
  if (appconn->s_state.location_changes>1)
    loc_search->roamed_in_sess_count++;
  else loc_search->new_sess_count++;

  appconn->loc_search_node=loc_search;
  log_dbg("location '%s' now has %d sessions attached",
	  loc,loc_search->total_sess_count);
}

void location_printlist(bstring s, char *loc, int json, int list) {
  bstring tmp = bfromcstr("");
  struct loc_search_t * loc_search;
  time_t act_mainclock = mainclock_now();
  time_t act_time = time(NULL);
  
  bassignformat(tmp,json ? 
		"{\"location\":\"%s\"" :
		"location = %s",
		loc);
  bconcat(s,tmp);
  
  loc_search= location_find(loc);

  if (loc_search) {
    int total_bytes_up=0,total_bytes_down=0;
#ifdef ENABLE_GARDENACCOUNTING
    int garden_total_bytes_up=0,garden_total_bytes_down=0;
    int other_total_bytes_up=0,other_total_bytes_down=0;
#endif
    int active_users=0,internet_users=0,active_internet_users=0;
    int timespan=(int)(act_mainclock-loc_search->last_queried);
    
    if (timespan >= 1) {
      
      log_dbg("roamed_in_session_count %d, out %d",
	      loc_search->roamed_in_sess_count,
	      loc_search->roamed_out_sess_count);
      log_dbg("new_session_count %d, closed %d",
	      loc_search->new_sess_count,
	      loc_search->closed_sess_count);
      
      bassignformat(tmp,json ?
		    ",\"sessions_roamed_in\":%d,"
		    "\"sessions_roamed_out\":%d,"
		    "\"sessions_new\":%d,"
		    "\"sessions_closed\":%d" :
		    "\n\tsessions_roamed_in = %d"
		    "\n\tsessions_roamed_out = %d"
		    "\n\tsessions_new = %d"
		    "\n\tsessions_closed = %d",
		    loc_search->roamed_in_sess_count, 
		    loc_search->roamed_out_sess_count,
		    loc_search->new_sess_count, 
		    loc_search->closed_sess_count);
      bconcat(s,tmp);
      
      if (loc_search->total_sess_count > 0) {
	struct app_conn_t * appconn;
	struct list_entity * ln;
	int bytes_up, bytes_down, swap;
#ifdef ENABLE_GARDENACCOUNTING
	int garden_bytes_up=0, garden_bytes_down=0;
	int other_bytes_up=0, other_bytes_down=0;
#endif
	int last_sent;
	
	log_dbg("location has %d sessions attached! ",loc_search->total_sess_count);
	log_dbg("(last queried %d seconds ago)\n",(act_mainclock-loc_search->last_queried));
	
	bassignformat(tmp,json ?
		      ",\"session_count\":%d,\"seconds_elapsed\":%d" :
		      "\n\tsession_count = %d\n\tseconds_elapsed = %d",
		      loc_search->total_sess_count, timespan);
	bconcat(s,tmp);
	
	if (list) {
	  if (json)
	    bassignformat(tmp,",\"sessions\":[");
	  else
	    bassignformat(tmp,"\n\tsessions:");
	  bconcat(s,tmp);
	}
	
	ln = loc_search->loc_sess_head.next;
	while (ln != &loc_search->loc_sess_head) {
	  appconn = container_of(ln,struct app_conn_t,loc_sess);

	  bytes_up = 
	    appconn->s_state.output_octets - 
	    appconn->s_state.output_octets_old;
	  
	  bytes_down = 
	    appconn->s_state.input_octets - 
	    appconn->s_state.input_octets_old;

	  if (_options.swapoctets) {
	    swap = bytes_up;
	    bytes_up = bytes_down;
	    bytes_down = swap;
	  }
	  
	  total_bytes_up += bytes_up;
	  total_bytes_down += bytes_down;
	  
#ifdef ENABLE_GARDENACCOUNTING
	  if (_options.uamgardendata) {
	    garden_bytes_up= (appconn->s_state.garden_output_octets-appconn->s_state.garden_output_octets_old);
	    garden_bytes_down= (appconn->s_state.garden_input_octets-appconn->s_state.garden_input_octets_old);
	    if (_options.swapoctets) {
	      swap = garden_bytes_up;
	      garden_bytes_up = garden_bytes_down;
	      garden_bytes_down = swap;
	    }
	    garden_total_bytes_up += garden_bytes_up;
	    garden_total_bytes_down += garden_bytes_down;
	    other_bytes_up = (appconn->s_state.other_output_octets-appconn->s_state.other_output_octets_old);
	    other_bytes_down = (appconn->s_state.other_input_octets-appconn->s_state.other_input_octets_old);
	    if (_options.swapoctets) {
	      swap = other_bytes_up;
	      other_bytes_up = other_bytes_down;
	      other_bytes_down = swap;
	    }
	    other_total_bytes_up += other_bytes_up;
	    other_total_bytes_down += other_bytes_down;
	  }
#endif

	  if (appconn->s_state.last_sent_time) {
	    last_sent=act_mainclock-appconn->s_state.last_sent_time;
	  } else last_sent=-1;

	  if (list) {
	    log_dbg("mac: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X up: %d down: %d\n",
		    appconn->hismac[0], appconn->hismac[1], appconn->hismac[2], 
		    appconn->hismac[3], appconn->hismac[4], appconn->hismac[5],
		    bytes_up,bytes_down);
	    
	    bassignformat(tmp,json ? 
			  "{\"mac\":\"%.2X-%.2X-%.2X-%.2X-%.2X-%.2X\","
			  "\"session_id\":\"%s\","
			  "\"last_sent\":%d,\"session_age\":%d,\"bytes_up\":%d,"
			  "\"kbps_up\":%d,\"bytes_down\":%d,\"kbps_down\":%d,"
			  "\"total_bytes_up\":%d,\"total_bytes_down\":%d" :
			  "\n\t\tmac = %.2X-%.2X-%.2X-%.2X-%.2X-%.2X"
			  "\n\t\tsession id = %s\n\t\tlast_sent = %d"
			  "\n\t\tsession_age = %d\n\t\tbytes_up = %d\n\t\tkbps_up = %d\n\t\tbytes_down = %d"
			  "\n\t\tkbps_down = %d\n\t\ttotal_bytes_up = %d\n\t\ttotal_bytes_down = %d",
			  appconn->hismac[0], appconn->hismac[1], appconn->hismac[2], 
			  appconn->hismac[3], appconn->hismac[4], appconn->hismac[5],
			  appconn->s_state.sessionid,
			  last_sent, act_time-appconn->rt,
			  bytes_up,(bytes_up*8)/(1024*timespan),
			  bytes_down,(bytes_down*8)/(1024*timespan),
			  appconn->s_state.output_octets, 
			  appconn->s_state.input_octets);
	    bconcat(s,tmp);

#ifdef ENABLE_GARDENACCOUNTING
	    if (_options.uamgardendata) {
	      bassignformat(tmp,json ?
			    ",\"garden_bytes_up\":%d,\"garden_kbps_up\":%d,"
			    "\"garden_bytes_down\":%d,\"garden_kbps_down\":%d,"
			    "\"garden_total_bytes_up\":%d,\"garden_total_bytes_down\":%d" :
			    "\n\t\tgarden_bytes_up = %d\n\t\tgarden_kbps_up = %d"
			    "\n\t\tgarden_bytes_down = %d\n\t\tgarden_kbps_down = %d"
			    "\n\t\tgarden_total_bytes_up = %d\n\t\tgarden_total_bytes_down = %d",
			    garden_bytes_up, (garden_bytes_up*8)/(1024*timespan),
			    garden_bytes_down, (garden_bytes_down*8)/(1024*timespan),
			    appconn->s_state.garden_output_octets,
			    appconn->s_state.garden_input_octets);

	      bconcat(s,tmp);
	      bassignformat(tmp,json
			    ? ",\"other_bytes_up\":%d,\"other_kbps_up\":%d,\"other_bytes_down\":%d,\"other_kbps_down\":%d,\"other_total_bytes_up\":%d,\"other_total_bytes_down\":%d"
			    : "\n\t\tother_bytes_up = %d\n\t\tother_kbps_up = %d\n\t\tother_bytes_down = %d\n\t\tother_kbps_down = %d\n\t\tother_total_bytes_up = %d\n\t\tother_total_bytes_down = %d"
			    , other_bytes_up, (other_bytes_up*8)/(1024*timespan)
			    , other_bytes_down, (other_bytes_down*8)/(1024*timespan)
			    , appconn->s_state.other_output_octets, appconn->s_state.other_input_octets);
	      bconcat(s,tmp);
	    }
#endif

	    if (last_sent <= 2 && last_sent >= 0) {
	      active_users++;
	      if (appconn->s_state.authenticated) active_internet_users++;
	    }
	    if (appconn->s_state.authenticated) {
	      internet_users++;
	      bassignformat(tmp,json
			    ? ",\"authenticated\":1,\"authenticated_since\":%d}"
			    : "\n\t\tauthenticated = 1\n\t\tauthenticated_since = %d\n"
			    , act_mainclock-appconn->s_state.start_time);
	    } else bassignformat(tmp,json
				 ? ",\"authenticated\":0,\"authenticated_since\":-1}"
				 : "\n\t\tauthenticated = 0\n\t\tauthenticated_since = -1\n");
	    bconcat(s,tmp);
	  }
	  
	  /*copy actual counter to "old", for future bandwith calculations*/
	  appconn->s_state.output_octets_old = appconn->s_state.output_octets;
	  appconn->s_state.input_octets_old = appconn->s_state.input_octets;

#ifdef ENABLE_GARDENACCOUNTING
	  if (_options.uamgardendata) {
	    appconn->s_state.garden_output_octets_old = appconn->s_state.garden_output_octets;
	    appconn->s_state.garden_input_octets_old = appconn->s_state.garden_input_octets;
	    appconn->s_state.other_output_octets_old = appconn->s_state.other_output_octets;
	    appconn->s_state.other_input_octets_old = appconn->s_state.other_input_octets;
	  }
#endif
	  ln = ln->next;
	  if (json && list && (ln != &loc_search->loc_sess_head)) {
	    bassignformat(tmp,",");
	    bconcat(s,tmp);
	  }
	}
	if (json && list) {
	  bassignformat(tmp,"]");
	  bconcat(s,tmp);
	}
      }
      else {
	fprintf(stderr,"location has no sessions attached!\n");
	bassignformat(tmp,json
		      ? ",\"session_count\":0,\"seconds_elapsed\":%d"
		      : "\n\tsession_count = 0\n\tseconds_elapsed = %d"
		      , timespan);
	bconcat(s,tmp);
      }

      if (list) {
	bassignformat(tmp,json
		      ? ",\"closed_sessions\":{\"closed_bytes_up\":%d,\"closed_kbps_up\":%d,\"closed_bytes_down\":%d,\"closed_kbps_down\":%d"
		      : "\n\tclosed_bytes_up = %d\n\tclosed_kbps_up = %d\n\tclosed_bytes_down = %d\n\tclosed_kbps_down = %d"
		      , loc_search->closed_bytes_up,(loc_search->closed_bytes_up*8)/(1024*timespan)
		      , loc_search->closed_bytes_down,(loc_search->closed_bytes_down*8)/(1024*timespan));
	bconcat(s,tmp);

#ifdef ENABLE_GARDENACCOUNTING
	if (_options.uamgardendata) {
	  bassignformat(tmp,json
			? ",\"garden_closed_bytes_up\":%d,\"garden_closed_kbps_up\":%d,\"garden_closed_bytes_down\":%d,\"garden_closed_kbps_down\":%d"
			: "\n\tgarden_closed_bytes_up = %d\n\tgarden_closed_kbps_up = %d\n\tgarden_closed_bytes_down = %d\n\tgarden_closed_kbps_down = %d"
			, loc_search->garden_closed_bytes_up,(loc_search->garden_closed_bytes_up*8)/(1024*timespan)
			, loc_search->garden_closed_bytes_down,(loc_search->garden_closed_bytes_down*8)/(1024*timespan));
	  bconcat(s,tmp);
	  bassignformat(tmp,json
			? ",\"other_closed_bytes_up\":%d,\"other_closed_kbps_up\":%d,\"other_closed_bytes_down\":%d,\"other_closed_kbps_down\":%d"
			: "\n\tother_closed_bytes_up = %d\n\tother_closed_kbps_up = %d\n\tother_closed_bytes_down = %d\n\tother_closed_kbps_down = %d"
			, loc_search->other_closed_bytes_up,(loc_search->other_closed_bytes_up*8)/(1024*timespan)
			, loc_search->other_closed_bytes_down,(loc_search->other_closed_bytes_down*8)/(1024*timespan));
	  bconcat(s,tmp);
	}
#endif
	if (json) {
	  bassignformat(tmp,"}");
	  bconcat(s,tmp);
	}
      }
      
      /*  add closed counter to sums (and set them back to 0)  */
      total_bytes_up += loc_search->closed_bytes_up;
      total_bytes_down += loc_search->closed_bytes_down;
      loc_search->closed_bytes_up = loc_search->closed_bytes_down=0;

#ifdef ENABLE_GARDENACCOUNTING
      if (_options.uamgardendata) {
	garden_total_bytes_up += loc_search->garden_closed_bytes_up;
	garden_total_bytes_down += loc_search->garden_closed_bytes_down;
	other_total_bytes_up += loc_search->other_closed_bytes_up;
	other_total_bytes_down += loc_search->other_closed_bytes_down;

	loc_search->garden_closed_bytes_up = 
	  loc_search->garden_closed_bytes_down = 
	  loc_search->other_closed_bytes_up = 
	  loc_search->other_closed_bytes_down = 0;
      }
#endif
      
      bassignformat(tmp,json ?
		    ",\"active_users\":%d,\"internet_users\":%d,"
		    "\"active_internet_users\":%d,\"total_bytes_up\":%d,"
		    "\"total_kbps_up\":%d,\"total_bytes_down\":%d,"
		    "\"total_kbps_down\":%d" :
		    "\n\tactive_users = %d\n\tinternet_users = %d"
		    "\n\tactive_internet_users = %d\n\ttotal_bytes_up = %d"
		    "\n\ttotal_kbps_up = %d\n\ttotal_bytes_down = %d"
		    "\n\ttotal_kbps_down = %d",
		    active_users, internet_users, active_internet_users,
		    total_bytes_up,(total_bytes_up*8)/(1024*timespan),
		    total_bytes_down,(total_bytes_down*8)/(1024*timespan));
      bconcat(s,tmp);
#ifdef ENABLE_GARDENACCOUNTING
      if (_options.uamgardendata) {
	bassignformat(tmp,json ?
		      ",\"garden_total_bytes_up\":%d,"
		      "\"garden_total_kbps_up\":%d,"
		      "\"garden_total_bytes_down\":%d,"
		      "\"garden_total_kbps_down\":%d" :
		      "\n\tgarden_total_bytes_up = %d"
		      "\n\tgarden_total_kbps_up = %d"
		      "\n\tgarden_total_bytes_down = %d"
		      "\n\tgarden_total_kbps_down = %d", 
		      garden_total_bytes_up,
		      (garden_total_bytes_up*8)/(1024*timespan),
		      garden_total_bytes_down,
		      (garden_total_bytes_down*8)/(1024*timespan));
	bconcat(s,tmp);
	bassignformat(tmp,json ?
		      ",\"other_total_bytes_up\":%d,"
		      "\"other_total_kbps_up\":%d,"
		      "\"other_total_bytes_down\":%d,"
		      "\"other_total_kbps_down\":%d" :
		      "\n\tother_total_bytes_up = %d"
		      "\n\tother_total_kbps_up = %d"
		      "\n\tother_total_bytes_down = %d"
		      "\n\tother_total_kbps_down = %d",
		      other_total_bytes_up,
		      (other_total_bytes_up*8)/(1024*timespan),
		      other_total_bytes_down,
		      (other_total_bytes_down*8)/(1024*timespan));
	bconcat(s,tmp);
      }
#endif
      /*set timestamp*/
      loc_search->last_queried = act_mainclock;
      /*reset traffic counter*/
      loc_search->new_sess_count
	= loc_search->closed_sess_count
	= loc_search->roamed_in_sess_count
	= loc_search->roamed_out_sess_count = 0;

    } else { /*query too short after the last*/
      log_dbg("last query less than 1 second ago!!\n");
      bassignformat(tmp,json
		    ? ",\"session_count\":-2"
		    : "\n\tsession_count = -2");
      bconcat(s,tmp);
    }
  } else {
    log_dbg("location (%s) not found!", loc);
    bassignformat(tmp,json ? 
		  ",\"session_count\":-1"
		  ",\"location_count\":%d" :
		  "\n\tsession_count = -1"
		  "\n\tlocation_count = %d",
		  location_count);
    bconcat(s,tmp);
    if (location_count > 0) {
      struct loc_search_t *node;
      int cnt = 0;
      bassignformat(tmp,json ? 
		    ",\"locations\":[" :
		    "\n\tlocations:");
      bconcat(s,tmp);
      avl_for_each_element(&loc_search_tree, node, node) {
	if (json && cnt++ > 0) {
	  bassignformat(tmp,",");
	  bconcat(s,tmp);
	}
	bassignformat(tmp,json ? 
		      "{\"name\":\"%s\"}" :
		      "\n\t\tlocation = %s",
		      node->value);
	bconcat(s,tmp);
      }
      if (json) {
	bassignformat(tmp,"]");
	bconcat(s,tmp);
      }
    }
  }
  bassignformat(tmp,json? "}": "\n");
  bconcat(s,tmp);
  bdestroy(tmp);
}

#endif

void location_init() {
#ifdef HAVE_AVL
  struct app_conn_t *conn = firstusedconn;
  memset(&loc_search_tree, 0, sizeof(loc_search_tree));
  avl_init(&loc_search_tree, avl_comp, false, NULL);
  while (conn) {
    log_dbg("restoring location (%s) of conn %X-%X-%X-%X-%X-%X\n",
	    conn->s_state.location,
	    conn->hismac[0],conn->hismac[1],conn->hismac[2],
	    conn->hismac[3],conn->hismac[4],conn->hismac[5]);
    conn->loc_search_node = NULL;
    list_init_node(&conn->loc_sess);
    if (strlen(conn->s_state.location) > 0) 
      location_add_conn(conn, conn->s_state.location);
    conn = conn->next;
  }
#endif
}

