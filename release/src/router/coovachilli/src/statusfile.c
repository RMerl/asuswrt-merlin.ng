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

extern time_t mainclock;
extern struct ippool_t *ippool;

#define MARK_START 0x00
#define MARK_NEXT  0x34 /* arbitrary */

#ifdef ENABLE_BINSTATFILE
static int has_loaded = 0;

int loadstatus() {
  char filedest[512];
  FILE *file;
  char c;

  struct dhcp_conn_t dhcpconn;
  struct app_conn_t appconn;

  time_t r_wall, r_rt, r_rtoffset;
  time_t wall, rt, rtoffset;

  has_loaded = 1;

  if (!_options.usestatusfile) 
    return 1;

  statedir_file(filedest, sizeof(filedest), _options.usestatusfile, 0);

  log_dbg("Loading file %s", filedest);

  file = fopen(filedest, "r");
  if (!file) { log_err(errno, "could not open file %s", filedest); return -1; }

  while ((c = fgetc(file)) != MARK_START) {
    if (c == EOF) { 
      log_err(errno, "end of file");
      fclose(file); 
      return -1; 
    }
  }

  time(&wall);
  if (fread(&r_wall, sizeof(time_t), 1, file) != 1) {
    log_err(errno, "bad binary file");
    if (c == EOF) { fclose(file); return -1; }
  }

  rt = mainclock_tick();
  if (fread(&r_rt, sizeof(time_t), 1, file) != 1) {
    log_err(errno, "bad binary file");
    if (c == EOF) { fclose(file); return -1; }
  }

  if ((c = fgetc(file)) != MARK_START) {
    log_err(errno, "bad binary file");
    fclose(file); 
    return -1; 
  }

  rtoffset = wall - rt;
  log_dbg("now: wall = %d, rt = %d, wall at rt=0 %d",
	  (int)wall, (int)rt, (int)rtoffset);

  r_rtoffset = r_wall - r_rt;
  log_dbg("file: wall = %d, rt = %d, wall at rt=0 %d",
	  (int)r_wall, (int)r_rt, (int)r_rtoffset);

  while (fread(&dhcpconn, sizeof(struct dhcp_conn_t), 1, file) == 1) {
    struct dhcp_conn_t *conn = 0;
    struct ippoolm_t *newipm = 0;
    int n;

    /* todo: read a md5 checksum or magic token */

    if ((c = fgetc(file)) != MARK_NEXT) {
      log_err(errno, "bad binary file");
      fclose(file); 
      return -1;
    }

    if (dhcp_hashget(dhcp, &conn, dhcpconn.hismac)) {

      log_info("Loading dhcp connection %.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
	       dhcpconn.hismac[0], dhcpconn.hismac[1],
	       dhcpconn.hismac[2], dhcpconn.hismac[3],
	       dhcpconn.hismac[4], dhcpconn.hismac[5]);

      /* not already known */
      dhcp_lnkconn(dhcp, &conn);
      
      /* set/copy all the pointers */
      dhcpconn.nexthash = conn->nexthash;
      dhcpconn.next = conn->next;
      dhcpconn.prev = conn->prev;
      dhcpconn.parent = dhcp;

      dhcpconn.is_reserved = 0; /* never a reserved ip if added here */

      /*
       * Fix time_t values:
       *  lasttime
       */

#define localizetime(t) \
      /* to it's local real time */ \
      t = r_rtoffset + t; \
      /* now to our local rt offset */ \
      t = t - rtoffset; \
      if (t < 0) t = 0;

      localizetime(dhcpconn.lasttime);

      /* initialize dhcp_conn_t */
      memcpy(conn, &dhcpconn, sizeof(struct dhcp_conn_t));
      
      for (n=0; n < DHCP_DNAT_MAX; n++) {
	memset(conn->dnat[n].mac, 0, PKT_ETH_ALEN); 
      }
      
      log_dbg("checking IP %s", inet_ntoa(dhcpconn.hisip));

      /* add into ippool */
      if (ippool_getip(ippool, &newipm, &dhcpconn.hisip)) {
	if (ippool_newip(ippool, &newipm, &dhcpconn.hisip, 1)) {
	  if (ippool_newip(ippool, &newipm, &dhcpconn.hisip, 0)) {
	    log_err(0, "Failed to allocate either static or dynamic IP address");
	    conn->hisip.s_addr = 0;
	  }
	}
      }
      
      dhcp_hashadd(dhcp, conn);
      
      if (conn->peer) {
	conn->peer = 0;
	
	if (fread(&appconn, sizeof(struct app_conn_t), 1, file) == 1) {
	  struct app_conn_t *aconn = 0;
	  
	  if ((c = fgetc(file)) != MARK_NEXT) {
	    log_err(errno, "bad binary file");
	    fclose(file); 
	    return -1;
	  }
	  
	  if (chilli_new_conn(&aconn) == 0) {
	    /* set/copy all the pointers/internals */
	    appconn.unit = aconn->unit;
	    appconn.next = aconn->next;
	    appconn.prev = aconn->prev;
	    appconn.uplink = newipm;
	    appconn.dnlink = conn;
	    
	    /*
	     * Fix time_t values:
	     *  start_time, interim_time,
	     *  last_sent_time, last_time,
	     *  uamtime
	     */
	    localizetime(appconn.s_state.start_time);
	    localizetime(appconn.s_state.interim_time);
	    localizetime(appconn.s_state.last_sent_time);
	    localizetime(appconn.s_state.last_time);
	    localizetime(appconn.s_state.uamtime);

	    /* initialize app_conn_t */
	    memcpy(aconn, &appconn, sizeof(struct app_conn_t));
	    conn->peer = aconn;

	    if (newipm) {
	      newipm->peer = aconn;
	      
#ifdef ENABLE_UAMANYIP
	      if (aconn->natip.s_addr)
		chilli_assign_snat(aconn, 1);
#endif
	      
	      dhcp_set_addrs(conn, 
			     &newipm->addr, &_options.mask, 
			     &aconn->ourip, &aconn->mask,
			     &_options.dns1, &_options.dns2);
	    }

#if defined(ENABLE_SESSGARDEN) && defined(HAVE_PATRICIA)
	    if (aconn->s_params.pass_through_count) {
	      garden_patricia_load_list(&aconn->ptree,
					aconn->s_params.pass_throughs, 
					aconn->s_params.pass_through_count);
	    }
#endif
	  }
	  
	  /* todo: read a md5 checksum or magic token */
	}
	else {
	  log_err(errno, "Problem loading state file %s",filedest);
	  break;
	}
      }

    } else {

      log_info("Known dhcp connection %.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
	       dhcpconn.hismac[0], dhcpconn.hismac[1],
	       dhcpconn.hismac[2], dhcpconn.hismac[3],
	       dhcpconn.hismac[4], dhcpconn.hismac[5]);

      conn->authstate = dhcpconn.authstate;
      
      if (dhcpconn.peer) {

	log_info("Reading appconn (peer)");

	if (fread(&appconn, sizeof(struct app_conn_t), 1, file) == 1) {

	  if ((c = fgetc(file)) != MARK_NEXT) {
	    log_err(errno, "bad binary file");
	    fclose(file); 
	    return -1;
	  }

	  if (conn->peer) {
	    /*
	     * Already have an appconn.
	     */
	    struct app_conn_t *aconn = (struct app_conn_t*) conn->peer;
	    
	    log_info("Overwriting existing appconn %d", appconn.s_state.authenticated);
	    
	    memcpy(&aconn->s_params, &appconn.s_params, sizeof(struct session_params));
	    memcpy(&aconn->s_state, &appconn.s_state, sizeof(struct session_state));
	    
	  } else {
	    /*
	     * No peer (appconn), then create it just as above.
	     */
	    struct app_conn_t *aconn = 0;
	    
	    log_info("Creating new appconn (peer)");
	    
	    if (ippool_getip(ippool, &newipm, &conn->hisip)) {
	      if (ippool_newip(ippool, &newipm, &conn->hisip, 1)) {
		if (ippool_newip(ippool, &newipm, &conn->hisip, 0)) {
		  log_err(0, "Failed to allocate either static or dynamic IP address");
		  fclose(file); 
		  return -1;
		}
	      }
	    }
	    
	    if (chilli_new_conn(&aconn) == 0) {
	      /* set/copy all the pointers/internals */
	      appconn.unit = aconn->unit;
	      appconn.next = aconn->next;
	      appconn.prev = aconn->prev;
	      appconn.uplink = newipm;
	      appconn.dnlink = conn;
	      
	      /* initialize app_conn_t */
	      memcpy(aconn, &appconn, sizeof(struct app_conn_t));
	      conn->peer = aconn;
	      newipm->peer = aconn;
	      
#ifdef ENABLE_UAMANYIP
	      if (appconn.natip.s_addr)
		chilli_assign_snat(aconn, 1);
#endif
	      
	      dhcp_set_addrs(conn, 
			     &newipm->addr, &_options.mask, 
			     &aconn->ourip, &aconn->mask,
			     &_options.dns1, &_options.dns2);
	    }
	  }
	}
      }
    }
  } 

  fclose(file);
  printstatus();
  return 0;
}

int printstatus() {
  char filedest[512];
  time_t tm;
  FILE *file;

  struct dhcp_conn_t *dhcpconn = dhcp->firstusedconn;
  struct app_conn_t *appconn;

  if (!has_loaded)
    return 0;

  if (!_options.usestatusfile) 
    return 0;

  statedir_file(filedest, sizeof(filedest), _options.usestatusfile, 0);

  log_dbg("Writing status file: %s", filedest);

  file = fopen(filedest, "w");
  if (!file) { log_err(errno, "could not open file %s", filedest); return -1; }
  fprintf(file, "#CoovaChilli-Version: %s\n", VERSION);
  fprintf(file, "#Timestamp: %d\n", (int) mainclock);

  /* marker */
  fputc(MARK_START, file);

  time(&tm); /* wall time */
  fwrite(&tm, sizeof(time_t), 1, file);

  tm = mainclock_tick(); /* rt time */
  fwrite(&tm, sizeof(time_t), 1, file);

  fputc(MARK_START, file);

  while (dhcpconn) {

    switch(dhcpconn->authstate) {
    case DHCP_AUTH_DROP:
    case DHCP_AUTH_PASS:
    case DHCP_AUTH_DNAT:
    case DHCP_AUTH_SPLASH:
#ifdef ENABLE_LAYER3
    case DHCP_AUTH_ROUTER:
#endif
      log_dbg("Saving dhcp connection %.2X-%.2X-%.2X-%.2X-%.2X-%.2X %s",
	      dhcpconn->hismac[0], dhcpconn->hismac[1],
	      dhcpconn->hismac[2], dhcpconn->hismac[3],
	      dhcpconn->hismac[4], dhcpconn->hismac[5],
	      inet_ntoa(dhcpconn->hisip));
      
      fwrite(dhcpconn, sizeof(struct dhcp_conn_t), 1, file);
      fputc(MARK_NEXT, file);
      appconn = (struct app_conn_t *)dhcpconn->peer;
      if (appconn) {
	fwrite(appconn, sizeof(struct app_conn_t), 1, file);
	fputc(MARK_NEXT, file);
      }
      break;
    }

    dhcpconn = dhcpconn->next;
  }

  fclose(file);
  return 0;
}
#else
#ifdef ENABLE_STATFILE
int loadstatus() {
  printstatus();
  return 0;
}

int printstatus() {
  FILE *file;
  char filedest[512];
  
  struct dhcp_conn_t *dhcpconn = dhcp->firstusedconn;
  struct app_conn_t *appconn;

  if (!_options.usestatusfile) 
    return 0;

  statedir_file(filedest, sizeof(filedest), _options.usestatusfile, 0);

  file = fopen(filedest, "w");
  if (!file) { log_err(errno, "could not open file %s", filedest); return -1; }

  fprintf(file, "#Version:1.1\n");
  fprintf(file, "#SessionID = SID\n#Start-Time = ST\n");
  fprintf(file, "#SessionTimeOut = STO\n#SessionTerminateTime = STT\n");
  fprintf(file, "#Timestamp: %d\n", (int) mainclock);
  fprintf(file, "#User, IP, MAC, SID, ST, STO, STT\n");

  while(dhcpconn) {
    appconn = (struct app_conn_t *)dhcpconn->peer;
    if (appconn && appconn->s_state.authenticated == 1) {
      fprintf(file, "%s, %s, %.2X-%.2X-%.2X-%.2X-%.2X-%.2X, %s, %d, %d, %d\n",
	      appconn->s_state.redir.username,
	      inet_ntoa(appconn->hisip),
	      appconn->hismac[0], appconn->hismac[1],
	      appconn->hismac[2], appconn->hismac[3],
	      appconn->hismac[4], appconn->hismac[5],
	      appconn->s_state.sessionid,
	      appconn->s_state.start_time,
	      appconn->s_params.sessiontimeout,
	      appconn->s_params.sessionterminatetime);
    }

    dhcpconn = dhcpconn->next;
  }
  
  fclose(file);
  return 0;
}
#endif
#endif
