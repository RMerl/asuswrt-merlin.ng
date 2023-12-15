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
#include "json.h"

int http_parse_input(char *src, int len, int is_cookie) {
  bstring name = bfromcstr("");
  bstring value = bfromcstr("");
  int start_pos=0, eq_pos=0, end_pos=0;
  int i, ret = 0;

  char stop = (is_cookie) ? ';' : '&';

  for (i=0; i <= len; i++) {
    if (i == len || src[i] == stop) {  end_pos = i; }
    else if (!eq_pos && src[i] == '=') {  eq_pos = i; }

    if (eq_pos && end_pos && start_pos != eq_pos) {

      bassigncstr(name, "");
      bassigncstr(value, "");

      if (is_cookie) {
        bassigncstr(name,"COOKIE_");
        bcatblk(name,src+start_pos,eq_pos-start_pos);
      } else {
        bassigncstr(name,"CAP_");
        bcatblk(name,src+start_pos,eq_pos-start_pos);
      }

      /*bunescape(name);*/

      if ((eq_pos + 1) == end_pos || (end_pos - eq_pos) <= 1) {
	log_dbg("%s = nil", name->data);
      } else {
        bassignblk(value, src+eq_pos+1, end_pos-eq_pos-1);
        /*bunescape(value);*/
	log_dbg("%s = %s", name->data, value->data);
	setenv((char *)name->data, (char *)value->data, 1);
      }
      start_pos = end_pos + 1;
      if (start_pos < len)
        while (src[start_pos] == ' ' && i < len)
          i++, start_pos++;

      end_pos = eq_pos = 0;
    }
  }

  ret = 1;
  bdestroy(name);
  bdestroy(value);
  return ret;
}

/*
static int chilli_status(bstring s) {
  return 0;
  }*/

static int chilli_garden(bstring b) {
  return 0;
}

static int chilli_sessions(bstring b) {
  struct dhcp_conn_t *conn = dhcp->firstusedconn;

  char *state = "<font color=green>Authorized</font>";

  bstring s = bfromcstr("");
  bcatcstr(b, "{\"service\":[{ \"");
  bcatcstr(b, getenv("CAP_table"));
  bcatcstr(b, "\":[ ");

  while (conn) {
    struct app_conn_t *appconn = (struct app_conn_t *)conn->peer;
    
    if (appconn && appconn->s_state.authenticated) {
      
    } else {
      state = "<font color=red>Redirect</font>";
    }
    
    bassignformat(s,
		  "{"
		  "\"state\":\"%s\","
		  "\"macAddress\":\"%.2X-%.2X-%.2X-%.2X-%.2X-%.2X\","
		  "\"ipAddress\":\"%s\",",
		  state,
		  conn->hismac[0], conn->hismac[1], conn->hismac[2],
		  conn->hismac[3], conn->hismac[4], conn->hismac[5],
		  inet_ntoa(conn->hisip)
		  );

    conn = conn->next;
    if (appconn) {
      session_json_params(&appconn->s_state, &appconn->s_params, s, 0);
      bcatcstr(s, ",");
      session_json_acct(&appconn->s_state, &appconn->s_params, s, 0);
    }
    bcatcstr(s, "},");
    bconcat(b, s);
  }
  
  bcatcstr(b, "]} ]}");
  return 0;
}

struct ewt_service {
  char *name;
  int (* func) (bstring s);
} ewt_services[] = {
  /*  { "hot-status", chilli_status },*/
  { "chilli-garden", chilli_garden },
  { "chilli-sessions", chilli_sessions },
};

static int NUM_SERVICES = sizeof(ewt_services) / sizeof(struct ewt_service);

static void json_walk(bstring prefix, struct json_object *obj) {
  json_object_object_foreach(obj, key, val) {
    bstring tmp = bfromcstr("");
    switch(json_object_get_type(val)) {
    case json_type_object:
      bassign(tmp, prefix);
      bcatcstr(tmp, key);
      bcatcstr(tmp, "_0_");
      json_walk(tmp, val);
      break;
    case json_type_array:
      bassign(tmp, prefix);
      bcatcstr(tmp, key);
      log_dbg("a %s=%s", tmp->data, json_object_to_json_string(val));
      break;
    default:
      bassign(tmp, prefix);
      bcatcstr(tmp, key);
      log_dbg("%s=%s", tmp->data, json_object_to_json_string(val));
      break;
    }
    bdestroy(tmp);
  }
}

int ewtapi(struct redir_t *redir, 
	   struct redir_socket_t *sock, 
	   struct redir_conn_t *conn,
	   struct redir_httpreq_t *httpreq) {
  char path[1024];
  char *binqqargs[3] = { path, 0, 0 };
  bstring b = bfromcstr("");
  bstring s = bfromcstr("");
  bstring res = bfromcstr("");
  int i;

  log_dbg("EWT API Request");

  redir_getparam(redir, httpreq->qs, "s", s);
  redir_getparam(redir, httpreq->qs, "res", res);

  if (httpreq->qs) {
    http_parse_input(httpreq->qs, strlen(httpreq->qs), 0);
  }
  
  if (httpreq->clen) {
    struct json_object *obj = 0;
    bblk_fromfd(b, 0, httpreq->clen);
    if ((obj = json_tokener_parse((char *)b->data))) {
      log_dbg("obj.to_string()=%s", json_object_to_json_string(obj));
      json_object_object_foreach(obj, key, val) {
	if (!strcmp(key, (char *)s->data)) {
	  bstring tmp = bfromcstr("CAP_");
	  json_walk(tmp, val);
	  bdestroy(tmp);
	}
      }
    }
    /* delete obj */
  }

  bassignformat(b,
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"Pragma: no-cache\r\n"
		"Expires: Fri, 01 Jan 1971 00:00:00 GMT\r\n"
		"Cache-Control: no-cache, must-revalidate\r\n"
		"Content-type: application/json\r\n\r\n");
  
  if (safe_write(1, b->data, b->slen) < 0) {
    log_err(errno, "redir_write()");
  }

  safe_snprintf(path, sizeof(path),
		"/var/coova/scripts/%s.sh", s->data);

  for (i=0; i < NUM_SERVICES; i++) {
    bassigncstr(b, "");
    if (!strcmp(ewt_services[i].name, (char *)s->data)) {
      ewt_services[i].func(b);
      log_dbg("Internal EWT Service %s -> %s", ewt_services[i].name, b->data);
      if (safe_write(1, b->data, b->slen) < 0) {
	log_err(errno, "redir_write()");
      }
      bdestroy(b);
      bdestroy(s);
      bdestroy(res);
      return 0;
    }
  } 

  setenv("EWTAPI", "1", 1);
  log_dbg("EWT API Running %s", *binqqargs);
  
  execv(*binqqargs, binqqargs);
  log_err(errno, "count not exec %s", *binqqargs);
  return -1;
}
