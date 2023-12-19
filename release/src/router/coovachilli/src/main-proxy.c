/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2009-2012 David Bird (Coova Technologies) <support@coova.com>
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
 * Author: David Bird <david@coova.com>
 */

#define MAIN_FILE

#include "chilli.h"
#include "debug.h"

#ifdef USING_CURL
#include <curl/curl.h>
#include <curl/easy.h>
#endif

/*
 * Plans (todo):
 *  - Provide a simple RADIUS->HTTP AAA proxy (loosly based on WiFiDog). 
 *  - It should also be able to proxy to an alternate RADIUS server(s). 
 *  - It should also be able to establish and use a RadSec Tunnel. 
 *
 * Also see: http://www.coova.org/CoovaChilli/Proxy
 *
 * To enable, be sure to configure with "./configure --enable-chilliproxy [--with-curl]"
 *
 */

struct options_t _options;

typedef struct _proxy_request {
  int index;

  char reserved:4;
  char authorized:1;
  char challenge:1;
  char inuse:1;
  char past_headers:1;
  char nline:1;

  bstring url;
  bstring data;
  bstring post;
  bstring wbuf;
  bstring dbuf;

  struct radius_packet_t radius_req;
  struct radius_packet_t radius_res;

#ifdef USING_CURL
  CURL *curl;
  char error_buffer[CURL_ERROR_SIZE];
#endif

  struct conn_t conn;

  struct radius_t *radius;

  struct _proxy_request *prev, *next;

  time_t lasttime;
  
} proxy_request;

static int max_requests = 0;
static int num_requests_free = 0;
static proxy_request * requests = 0;
static proxy_request * requests_free = 0;

#ifdef USING_CURL
static CURLM * curl_multi;
static int still_running = 0;
#endif

static char nas_hwaddr[PKT_ETH_ALEN];

static void print_requests() {
  proxy_request * req = 0;
  int i;

  for (i=0; i < max_requests; i++) {
    req = &requests[i];
    log_info("%.3d. inuse=%d prev=%.3d next=%.3d url=%s fd=%d", 
	     req->index, req->inuse ? 1 : 0,
	     req->prev ? req->prev->index : -1,
	     req->next ? req->next->index : -1,
	     req->url ? (char *)req->url->data : "",
	     req->conn.sock);
  }
}

static proxy_request * get_request() {
  proxy_request * req = 0;
  int i;

  if (!requests) {

    /* Initialize linked list */
    max_requests = 16; /* hard maximum! (should be configurable) */

    requests = (proxy_request *) calloc(max_requests, sizeof(proxy_request));
    for (i=0; i < max_requests; i++) {
      requests[i].index = i;
      if (i > 0) 
	requests[i].prev = &requests[i-1];
      if ((i+1) < max_requests) 
	requests[i].next = &requests[i+1];
    }
    
    requests_free = requests;
    num_requests_free = max_requests;
  }
  
  if (requests_free) {
    req = requests_free;
    requests_free = requests_free->next;
    if (requests_free)
      requests_free->prev = 0;
    num_requests_free--;
  }
  
#if(_debug_)
  log_dbg("connections free %d", num_requests_free);
#endif
  
  if (!req) {
    /* problem */
    log_err(0,"out of connections");
    print_requests();
    return 0;
  }

  log_dbg("request index %d", req->index);
  req->lasttime = time(NULL);
  req->next = req->prev = 0;
  req->inuse = 1;
  return req;
}

static int radius_reply(struct radius_t *this,
			struct radius_packet_t *pack,
			struct sockaddr_in *peer) {
  
  size_t len = ntohs(pack->length);
  
  if (sendto(this->fd, pack, len, 0,(struct sockaddr *) peer, 
	     sizeof(struct sockaddr_in)) < 0) {
    log_err(errno, "sendto() failed!");
    return -1;
  } 
  
  return 0;
}

static void bhex(bstring src, bstring dst) {
  int i;
  char b[4];
  for (i=0; i < src->slen; i++) {
    snprintf(b, sizeof(b), "%.2X", src->data[i]);
    bcatcstr(dst, b);
  }
}

static void bunhex(bstring src, bstring dst) {
  int i;
  unsigned int ir;
  uint8_t r;
  for (i=0; i < src->slen; i+=2) {
    sscanf((char *) &src->data[i], "%2x", &ir);
    r = (uint8_t) ir;
    bcatblk(dst, &r, 1);
  }
}

static void close_request(proxy_request *req) {

  log_dbg("%s", __FUNCTION__);

  if (req->url)  bdestroy(req->url);
  if (req->data) bdestroy(req->data);
  if (req->post) bdestroy(req->post);
  if (req->wbuf) bdestroy(req->wbuf);
  if (req->dbuf) bdestroy(req->dbuf);

  req->url =
    req->data = 
    req->post = 
    req->dbuf =
    req->wbuf = 0;
  
  req->authorized = 0;
  req->challenge = 0;

  req->inuse = 0;
  if (requests_free) {
    requests_free->prev = req;
    req->next = requests_free;
  }
  requests_free = req;
  num_requests_free++;

#if(_debug_)
  log_dbg("connections free %d", num_requests_free);
#endif
}

static int http_aaa_finish(proxy_request *req) {

  struct radius_t *radius = req->radius;

#ifdef USING_CURL
#if(_debug_)
  log_dbg("calling curl_easy_cleanup()");
#endif
  if (req->curl) {
    if (req->error_buffer[0])
      log_dbg("curl error %s", req->error_buffer);
    curl_multi_remove_handle(curl_multi, req->curl);
    curl_easy_cleanup(req->curl);
    req->curl = 0;
    req->error_buffer[0] = 0;
  }
#else
  conn_close(&req->conn);
#endif

  if (req->data && req->data->slen) {
#if(_debug_)
    log_dbg("Received: %s\n",req->data->data);
#endif
    req->authorized = !memcmp(req->data->data, "Auth: 1", 7);
    req->challenge = !memcmp(req->data->data, "Auth: 2", 7);
  }

  /* initialize response packet */
  switch(req->radius_req.code) {
  case RADIUS_CODE_ACCOUNTING_REQUEST:
#if(_debug_)
    log_dbg("Accounting-Response");
#endif
    radius_default_pack(radius, &req->radius_res, RADIUS_CODE_ACCOUNTING_RESPONSE);
    break;
    
  case RADIUS_CODE_ACCESS_REQUEST:
#if(_debug_)
    log_dbg("Access-%s", req->authorized ? "Accept" : 
	    req->challenge ? "Challenge" : "Reject");
#endif
    radius_default_pack(radius, &req->radius_res, 
			req->authorized ? RADIUS_CODE_ACCESS_ACCEPT :
			req->challenge ? RADIUS_CODE_ACCESS_CHALLENGE :
			RADIUS_CODE_ACCESS_REJECT);
    break;

  default:
    radius_default_pack(radius, &req->radius_res, RADIUS_CODE_ACCESS_REJECT);
    break;
  }

  req->radius_res.id = req->radius_req.id;

  /* process attributes */
  if (req->data->slen) {
    char *parse = (char *) req->data->data;
    if (parse) {
      char *ptr;
      while ((ptr = strtok(parse,"\n"))) {
	parse = 0;

	struct {
	  char *n;
	  int a;
	  int v;
	  int va;
	  char t;
	} attrs[] = {
	  { "Idle-Timeout:", RADIUS_ATTR_IDLE_TIMEOUT, 0, 0, 0 },
	  { "Reply-Message:", RADIUS_ATTR_REPLY_MESSAGE, 0, 0, 1 },
	  { "Session-Timeout:", RADIUS_ATTR_SESSION_TIMEOUT, 0, 0, 0 },
	  { "Acct-Interim-Interval:", RADIUS_ATTR_ACCT_INTERIM_INTERVAL, 0, 0, 0 },
	  { "EAP-Message:", RADIUS_ATTR_EAP_MESSAGE, 0, 0, 2 },
	  { "ChilliSpot-Version:", 
	    RADIUS_ATTR_VENDOR_SPECIFIC, RADIUS_VENDOR_CHILLISPOT, 
	    RADIUS_ATTR_CHILLISPOT_VERSION, 1 },
	  { "ChilliSpot-Config:", 
	    RADIUS_ATTR_VENDOR_SPECIFIC, RADIUS_VENDOR_CHILLISPOT, 
	    RADIUS_ATTR_CHILLISPOT_CONFIG, 1 },
	  { "ChilliSpot-Bandwidth-Max-Up:", 
	    RADIUS_ATTR_VENDOR_SPECIFIC, RADIUS_VENDOR_CHILLISPOT, 
	    RADIUS_ATTR_CHILLISPOT_BANDWIDTH_MAX_UP, 0 },
	  { "ChilliSpot-Bandwidth-Max-Down:", 
	    RADIUS_ATTR_VENDOR_SPECIFIC, RADIUS_VENDOR_CHILLISPOT, 
	    RADIUS_ATTR_CHILLISPOT_BANDWIDTH_MAX_DOWN, 0 },
	  { "ChilliSpot-Max-Input-Octets:", 
	    RADIUS_ATTR_VENDOR_SPECIFIC, RADIUS_VENDOR_CHILLISPOT, 
	    RADIUS_ATTR_CHILLISPOT_MAX_INPUT_OCTETS, 0 },
	  { "ChilliSpot-Max-Output-Octets:", 
	    RADIUS_ATTR_VENDOR_SPECIFIC, RADIUS_VENDOR_CHILLISPOT, 
	    RADIUS_ATTR_CHILLISPOT_MAX_OUTPUT_OCTETS, 0 },
	  { "ChilliSpot-Max-Total-Octets:", 
	    RADIUS_ATTR_VENDOR_SPECIFIC, RADIUS_VENDOR_CHILLISPOT, 
	    RADIUS_ATTR_CHILLISPOT_MAX_TOTAL_OCTETS, 0 },
	  { "ChilliSpot-Max-Input-Gigawords:", 
	    RADIUS_ATTR_VENDOR_SPECIFIC, RADIUS_VENDOR_CHILLISPOT, 
	    RADIUS_ATTR_CHILLISPOT_MAX_INPUT_GIGAWORDS, 0 },
	  { "ChilliSpot-Max-Output-Gigawords:", 
	    RADIUS_ATTR_VENDOR_SPECIFIC, RADIUS_VENDOR_CHILLISPOT, 
	    RADIUS_ATTR_CHILLISPOT_MAX_OUTPUT_GIGAWORDS, 0 },
	  { "ChilliSpot-Max-Total-Gigawords:", 
	    RADIUS_ATTR_VENDOR_SPECIFIC, RADIUS_VENDOR_CHILLISPOT, 
	    RADIUS_ATTR_CHILLISPOT_MAX_TOTAL_GIGAWORDS, 0 },
	  { "WISPr-Bandwidth-Max-Up:", 
	    RADIUS_ATTR_VENDOR_SPECIFIC, RADIUS_VENDOR_WISPR, 
	    RADIUS_ATTR_WISPR_BANDWIDTH_MAX_UP, 0 },
	  { "WISPr-Bandwidth-Max-Down:", 
	    RADIUS_ATTR_VENDOR_SPECIFIC, RADIUS_VENDOR_WISPR, 
	    RADIUS_ATTR_WISPR_BANDWIDTH_MAX_DOWN, 0 },
	  { "WISPr-Redirection-URL:", 
	    RADIUS_ATTR_VENDOR_SPECIFIC, RADIUS_VENDOR_WISPR, 
	    RADIUS_ATTR_WISPR_REDIRECTION_URL, 1 },
	  { 0 }
	};
	
	int i;
	for (i=0; attrs[i].n; i++) {
	  if (!strncmp(ptr,attrs[i].n,strlen(attrs[i].n))) {
	    switch(attrs[i].t) {
	    case 0: /*integer*/
	      {
		uint32_t v = (uint32_t) atoi(ptr+strlen(attrs[i].n));
		if (v > 0) {
		  radius_addattr(radius, &req->radius_res, attrs[i].a, attrs[i].v, attrs[i].va, v, NULL, 0);
#if(_debug_)
		  log_dbg("Setting %s = %d", attrs[i].n, v);
#endif
		}
	      }
	      break;
	    case 1: /*string*/
	      {
		radius_addattr(radius, &req->radius_res, attrs[i].a, attrs[i].v, attrs[i].va, 0, 
			       (uint8_t *)ptr+strlen(attrs[i].n), strlen(ptr)-strlen(attrs[i].n));
#if(_debug_)
		log_dbg("Setting %s = %s", attrs[i].n, ptr+strlen(attrs[i].n));
#endif
	      }
	      break;
	    case 2: /*binary*/
	      {
		bstring tmp = bfromcstr("");
		bstring tmp2 = bfromcstr("");

		uint8_t *resp;
		size_t offset = 0;
		size_t resplen;
		size_t eaplen;

		bassignblk(tmp,
			   (uint8_t *)ptr+strlen(attrs[i].n), 
			   strlen(ptr)-strlen(attrs[i].n));

		bunhex(tmp, tmp2);

		resp = tmp2->data;
		resplen = tmp2->slen;

		while (offset < resplen) {
		  
		  if ((resplen - offset) > RADIUS_ATTR_VLEN)
		    eaplen = RADIUS_ATTR_VLEN;
		  else
		    eaplen = resplen - offset;
		  
		  radius_addattr(radius, &req->radius_res, attrs[i].a, 
				 attrs[i].v, attrs[i].va, 0, 
				 resp + offset,
				 eaplen);
		  offset += eaplen;
		} 
		
#if(_debug_)
		log_dbg("Setting %s = %s", attrs[i].n, ptr+strlen(attrs[i].n));
#endif
	      }
	      break;
	    }
	  }
	}
      }
    }
  }

  /* finish off RADIUS response */
  switch(req->radius_req.code) {
    
  case RADIUS_CODE_ACCESS_REQUEST:
    {
      struct radius_attr_t *ma = NULL;
      
      radius_addattr(radius, &req->radius_res, RADIUS_ATTR_MESSAGE_AUTHENTICATOR, 
		     0, 0, 0, NULL, RADIUS_MD5LEN);
      
      memset(req->radius_res.authenticator, 0, RADIUS_AUTHLEN);
      memcpy(req->radius_res.authenticator, req->radius_req.authenticator, RADIUS_AUTHLEN);
      
      if (!radius_getattr(&req->radius_res, &ma, RADIUS_ATTR_MESSAGE_AUTHENTICATOR, 0,0,0)) {
	radius_hmac_md5(radius, &req->radius_res, radius->secret, radius->secretlen, ma->v.t);
      }
      
      radius_authresp_authenticator(radius, &req->radius_res, 
				    req->radius_req.authenticator,
				    radius->secret,
				    radius->secretlen);
    }
    break;
    
  case RADIUS_CODE_ACCOUNTING_REQUEST:
    radius_authresp_authenticator(radius, &req->radius_res, 
				  req->radius_req.authenticator,
				  radius->secret,
				  radius->secretlen);
    break;
  }

  radius_reply(req->radius, &req->radius_res, &req->conn.peer);

  close_request(req);

  return 0;
}

#ifdef USING_CURL
static int bstring_data(void *ptr, size_t size, size_t nmemb, void *userdata) {
  bstring s = (bstring) userdata;
  if (size > 0 && nmemb > 0) {
    int rsize = size * nmemb;
    bcatblk(s,ptr,rsize);
    log_dbg("read %d", rsize);
    return rsize;
  }
  return 0;
}

static int http_aaa_setup(struct radius_t *radius, proxy_request *req) {
  int result = -2;
  CURL *curl;

  char *user = 0;
  char *pwd = 0;
#ifdef HAVE_OPENSSL
  char *ca = 0;
  char *cert = 0;
  char *key = 0;
  char *keypwd = 0;
#endif

  req->radius = radius;
  
  if ((curl = req->curl = curl_easy_init()) != NULL) {
    
    if (req->post) {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (char *) req->post->data);
    }
    
    if (user && pwd) {
    }
    
#ifdef HAVE_OPENSSL
    
    if (cert && strlen(cert)) {
#if(_debug_)
      log_dbg("using cert [%s]",cert);
#endif
      curl_easy_setopt(curl, CURLOPT_SSLCERT, cert);
      curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
    }

    if (key && strlen(key)) {
#if(_debug_)
      log_dbg("using key [%s]",key);
#endif
      curl_easy_setopt(curl, CURLOPT_SSLKEY, key);
      curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
      if (keypwd && strlen(keypwd)) {
#if(_debug_)
	log_dbg("using key pwd [%s]",keypwd);
#endif
#ifdef CURLOPT_SSLCERTPASSWD
	curl_easy_setopt(curl, CURLOPT_SSLCERTPASSWD, keypwd);
#else
#ifdef CURLOPT_SSLKEYPASSWD
	curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, keypwd);
#else
#ifdef CURLOPT_KEYPASSWD
	curl_easy_setopt(curl, CURLOPT_KEYPASSWD, keypwd);
#endif
#endif
#endif
      }
    }
    
    if (ca && strlen(ca)) {
#ifdef CURLOPT_ISSUERCERT
#if(_debug_)
      log_dbg("using ca [%s]",ca);
#endif
      curl_easy_setopt(curl, CURLOPT_ISSUERCERT, ca);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
#else
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
#endif
    }
    else {
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    }
    
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_SSLv3);
#endif
    
    curl_easy_setopt(curl, CURLOPT_VERBOSE, /*debug ? 1 :*/ 0);
    
    curl_easy_setopt(curl, CURLOPT_URL, req->url->data);
    
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CoovaChilli " VERSION);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1); 
    curl_easy_setopt(curl, CURLOPT_NETRC, 0);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (curl_write_callback) bstring_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (char *) req->data);
    
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, req->error_buffer);
    
    result = 0;
  }
  
  return result;
}
#else

static int http_conn_finish(struct conn_t *conn, void *ctx) {
  return http_aaa_finish((proxy_request *)ctx);
}

static int http_conn_read(struct conn_t *conn, void *ctx) {
  proxy_request *req = (proxy_request *)ctx;
  char c[1];
  int r = safe_read(conn->sock, c, 1);
  if (r == 1) {
    switch (c[0]) {
    case '\r': break;
    case '\n': 
      if (req->nline) {
	conn_bstring_readhandler(conn, req->data);
      }
      req->nline = 1;
      break;
    default:
      req->nline = 0;
      break;
    }
  }
  req->lasttime = time(NULL);
  return 0;
}

static int http_aaa_setup(struct radius_t *radius, proxy_request *req) {
  /*
   *  - parse url
   *  - fill in write_buf
   *  - connect() (non-blocking)
   *  - return
   */
  char hostname[256];
  int port = 0, uripos;

  req->radius = radius;
  req->past_headers = 0;
  req->nline = 0;
  
  if (get_urlparts((char *) req->url->data, hostname, sizeof(hostname), &port, &uripos))
    return -1;
  
  if (!req->wbuf) 
    req->wbuf = bfromcstr("");

  if (!req->dbuf) 
    req->dbuf = bfromcstr("");
  
  bassignformat(req->wbuf, 
		"GET %s HTTP/1.1\r\n"
		"Host: %s:%d\r\n"
		"User-Agent: CoovaChilli " VERSION "\r\n"
		"Connection: close\r\n"
		"\r\n",
		req->url->data + uripos, hostname, port);
  
  if (conn_setup(&req->conn, hostname, port, req->wbuf, req->dbuf))
    return -1;
  
  conn_set_readhandler(&req->conn, http_conn_read, req);
  conn_set_donehandler(&req->conn, http_conn_finish, req);
  
  return 0;
}
#endif

static int http_aaa(struct radius_t *radius, proxy_request *req) {
  
  if (http_aaa_setup(radius, req) == 0) {
    
#ifdef USING_CURL
    curl_multi_add_handle(curl_multi, req->curl);
    
    while(CURLM_CALL_MULTI_PERFORM ==
	  curl_multi_perform(curl_multi, &still_running));

#if(_debug_ > 1)
    log_dbg("curl still running %d", still_running);
#endif
#endif
    
    return 0;
  }

  return -1;
}

static void http_aaa_register(int argc, char **argv, int i) {
  proxy_request req = {0};

  bstring tmp = bfromcstr("");
  bstring tmp2 = bfromcstr("");

  /* end with removing options */
  argv[i] = 0;
  process_options(i, argv, 1);

  if (!_options.uamaaaurl) {
    log_err(0, "uamaaaurl not defined in configuration");
    exit(-1);
  }

  req.url = bfromcstr("");
  req.data = bfromcstr("");
  req.post = bfromcstr("");

  bstring_fromfd(req.post, 0);

  bassignformat(req.url, "%s%c", 
		_options.uamaaaurl,
		strchr(_options.uamaaaurl, '?') > 0 ? '&' : '?');

  bcatcstr(req.url, "stage=register");

  bcatcstr(req.url, "&ap=");
  if (_options.nasmac) {
    bcatcstr(req.url, _options.nasmac);
  } else {
    char mac[32];
    safe_snprintf(mac, sizeof(mac), "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X", 
		  nas_hwaddr[0],nas_hwaddr[1],nas_hwaddr[2],
		  nas_hwaddr[3],nas_hwaddr[4],nas_hwaddr[5]);
    bcatcstr(req.url, mac);
  }

  bcatcstr(req.url, "&nasid=");
  if (_options.radiusnasid) {
    char *nasid = _options.radiusnasid;
    bassignblk(tmp, nasid, strlen(nasid));
    redir_urlencode(tmp, tmp2);
    bconcat(req.url, tmp2);
  }
    
  for (i=i+1; i < argc; i++) {
    bcatcstr(req.url, "&");
    bcatcstr(req.url, argv[i]);
    bcatcstr(req.url, "=");
    i++;
    if (i < argc) {
      bassignblk(tmp, argv[i], strlen(argv[i]));
      redir_urlencode(tmp, tmp2);
      bconcat(req.url, tmp2);
    }
  }

  if (_options.uamsecret && _options.uamsecret[0])
    redir_md_param(req.url, _options.uamsecret, "&");

#ifdef USING_CURL
  curl_global_init(CURL_GLOBAL_ALL);
#endif

  if (http_aaa_setup(0, &req) == 0) {

#if(_debug_ > 1)
    log_dbg("==> %s\npost:%s", req.url->data, req.post->data);
#endif

#ifdef USING_CURL
    curl_easy_perform(req.curl);
#endif

#if(_debug_ > 1)
    log_dbg("<== %s", req.data->data);
#endif

#ifdef USING_CURL
    curl_easy_cleanup(req.curl);
#endif
  }

  if (req.data->slen)
    if (safe_write(1, req.data->data, req.data->slen) < 0)
      log_err(errno, "write()");

  bdestroy(req.url);
  bdestroy(req.data);
  bdestroy(req.post);
  bdestroy(tmp);
  bdestroy(tmp2);

#ifdef USING_CURL
  curl_global_cleanup();
#else
  if (req.wbuf)
    bdestroy(req.wbuf);
  if (req.dbuf)
    bdestroy(req.dbuf);
#endif
  exit(0);
}

static void process_radius(struct radius_t *radius, struct radius_packet_t *pack, struct sockaddr_in *peer) {
  struct radius_attr_t *attr = NULL; 
  char *error = 0;

  proxy_request *req = get_request();

  bstring tmp;
  bstring tmp2;

  if (!req) return;

  if (!_options.uamaaaurl) {
    log_err(0,"No --uamaaaurl parameter defined");
    return;
  }

  tmp = bfromcstralloc(10,"");
  tmp2 = bfromcstralloc(10,"");

  if (!req->url) req->url = bfromcstr("");
  if (!req->data) req->data = bfromcstr("");

  memcpy(&req->conn.peer, peer, sizeof(req->conn.peer));
  memcpy(&req->radius_req, pack, sizeof(struct radius_packet_t));
  memset(&req->radius_res, '0', sizeof(struct radius_packet_t));

  bassigncstr(req->data, "");

  bassignformat(req->url, "%s%c", 
		_options.uamaaaurl, 
		strchr(_options.uamaaaurl, '?') > 0 ? '&' : '?');

  switch(req->radius_req.code) {
  case RADIUS_CODE_ACCESS_REQUEST:
    bcatcstr(req->url, "stage=login");
    if (radius_getattr(pack, &attr, RADIUS_ATTR_SERVICE_TYPE, 0,0,0)) {
      error = "No service-type in RADIUS packet";
    } else {
      bcatcstr(req->url, "&service=");
      switch (ntohl(attr->v.i)) {
      case RADIUS_SERVICE_TYPE_LOGIN:
	bcatcstr(req->url, "login");
	break;
      case RADIUS_SERVICE_TYPE_FRAMED:
	bcatcstr(req->url, "framed");
	break;
      case RADIUS_SERVICE_TYPE_ADMIN_USER:
	bcatcstr(req->url, "admin");
	break;
      default:
	bassignformat(tmp, "%d", ntohl(attr->v.i));
	bconcat(req->url, tmp);
	break;
      }
    }
    break;
  case RADIUS_CODE_ACCOUNTING_REQUEST:
    bcatcstr(req->url, "stage=counters");
    if (radius_getattr(pack, &attr, RADIUS_ATTR_ACCT_STATUS_TYPE, 0,0,0)) {
      error = "No acct-status-type in RADIUS packet";
    } else {
      bcatcstr(req->url, "&status=");
      switch (ntohl(attr->v.i)) {
      case RADIUS_STATUS_TYPE_START:
	bcatcstr(req->url, "start");
	break;
      case RADIUS_STATUS_TYPE_STOP:
	bcatcstr(req->url, "stop");
	break;
      case RADIUS_STATUS_TYPE_INTERIM_UPDATE:
	bcatcstr(req->url, "update");
	break;
      case RADIUS_STATUS_TYPE_ACCOUNTING_ON:
	bcatcstr(req->url, "up");
	break;
      case RADIUS_STATUS_TYPE_ACCOUNTING_OFF:
	bcatcstr(req->url, "down");
	break;
      default:
	log_err(0,"unsupported acct-status-type %d",ntohl(attr->v.i));
	error = "Unsupported acct-status-type";
	break;
      }
    }
    break;
  default:
    error = "Unsupported RADIUS code";
    break;
  }

  if (!error) {
    if (!radius_getattr(pack, &attr, RADIUS_ATTR_USER_NAME, 0,0,0)) {
      bcatcstr(req->url, "&user=");
      bassignblk(tmp, attr->v.t, attr->l-2);
      redir_urlencode(tmp, tmp2);
      bconcat(req->url, tmp2);
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_USER_PASSWORD, 0,0,0)) {
      char pwd[RADIUS_ATTR_VLEN];
      size_t pwdlen = 0;
      if (!radius_pwdecode(radius, (uint8_t *) pwd, RADIUS_ATTR_VLEN, &pwdlen, 
			   attr->v.t, attr->l-2, pack->authenticator,
			   radius->secret,
			   radius->secretlen)) {
	bcatcstr(req->url, "&pass=");
	bassignblk(tmp, pwd, strlen(pwd));
	redir_urlencode(tmp, tmp2);
	bconcat(req->url, tmp2);
      }
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_CHAP_CHALLENGE, 0,0,0)) {
      char hexchal[1+(2*REDIR_MD5LEN)];
      unsigned char challenge[REDIR_MD5LEN];
      if (attr->l-2 <= sizeof(challenge)) {
	bcatcstr(req->url, "&chap_chal=");
	memcpy(challenge, attr->v.t, attr->l-2);
	redir_chartohex(challenge, hexchal, REDIR_MD5LEN);
	bcatcstr(req->url, hexchal);
      }
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_CHAP_PASSWORD, 0,0,0)) {
      char hexchal[65]; /* more than enough */
      unsigned char resp[32]; 
      if (attr->l-3 <= sizeof(resp)) {
	char chapid = attr->v.t[0];
	bcatcstr(req->url, "&chap_pass=");
	redir_chartohex(attr->v.t+1, hexchal, attr->l-3);
	bcatcstr(req->url, hexchal);
	bassignformat(tmp, "&chap_id=%d", chapid);
	bconcat(req->url, tmp);
      }
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_CALLED_STATION_ID, 0,0,0)) {
      bcatcstr(req->url, "&ap=");
      bassignblk(tmp, attr->v.t, attr->l-2);
      redir_urlencode(tmp, tmp2);
      bconcat(req->url, tmp2);
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_CALLING_STATION_ID, 0,0,0)) {
      bcatcstr(req->url, "&mac=");
      bassignblk(tmp, attr->v.t, attr->l-2);
      redir_urlencode(tmp, tmp2);
      bconcat(req->url, tmp2);
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0, 0)) {
      if ((attr->l-2) == sizeof(struct in_addr)) {
	struct in_addr ip;
	ip.s_addr = attr->v.i;
	bcatcstr(req->url, "&ip=");
	bcatcstr(req->url, inet_ntoa(ip));
      }
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_ACCT_SESSION_ID, 0,0,0)) {
      bcatcstr(req->url, "&sessionid=");
      bassignblk(tmp, attr->v.t, attr->l-2);
      redir_urlencode(tmp, tmp2);
      bconcat(req->url, tmp2);
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_NAS_IDENTIFIER, 0,0,0)) {
      bcatcstr(req->url, "&nasid=");
      bassignblk(tmp, attr->v.t, attr->l-2);
      redir_urlencode(tmp, tmp2);
      bconcat(req->url, tmp2);
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_ACCT_SESSION_TIME, 0,0,0)) {
      uint32_t val = ntohl(attr->v.i);
      bassignformat(tmp, "&duration=%ld", (long) val);
      bconcat(req->url, tmp);
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_EAP_MESSAGE, 0,0,0)) {
      int instance = 1;
      bcatcstr(req->url, "&eap=");
      bassignblk(tmp, attr->v.t, (size_t)attr->l-2);
      bassigncstr(tmp2, "");

      do {
	attr=NULL;
	if (!radius_getattr(pack, &attr, RADIUS_ATTR_EAP_MESSAGE, 0, 0, 
			    instance++) && attr) {
	  bcatblk(tmp, attr->v.t, (size_t)attr->l-2);
	}
      } while (attr);

      bhex(tmp, tmp2);
      bconcat(req->url, tmp2);
    }
    
    if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
			RADIUS_VENDOR_CHILLISPOT, 
			RADIUS_ATTR_CHILLISPOT_VLAN_ID, 0)) {
      uint32_t val = ntohl(attr->v.i);
      bassignformat(tmp, "&vlan=%ld", (long) val);
      bconcat(req->url, tmp);
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_ACCT_INPUT_OCTETS, 0,0,0)) {
      char *direction = _options.swapoctets ? "up" : "down";
      uint64_t val = (uint64_t) ntohl(attr->v.i);
      uint64_t input = val;
      if (!radius_getattr(pack, &attr, RADIUS_ATTR_ACCT_INPUT_GIGAWORDS, 0,0,0)) {
	val = (uint64_t) ntohl(attr->v.i);
	input |= (val << 32);
      }
      bassignformat(tmp, "&bytes_%s=%lld", direction, (long long) input);
      bconcat(req->url, tmp);
      if (!radius_getattr(pack, &attr, RADIUS_ATTR_ACCT_INPUT_PACKETS, 0,0,0)) {
	uint32_t sval = ntohl(attr->v.i);
	bassignformat(tmp, "&pkts_%s=%ld", direction, (long) sval);
	bconcat(req->url, tmp);
      }
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_ACCT_OUTPUT_OCTETS, 0,0,0)) {
      char *direction = _options.swapoctets ? "down" : "up";
      uint64_t val = (uint64_t) ntohl(attr->v.i);
      uint64_t output = val;
      if (!radius_getattr(pack, &attr, RADIUS_ATTR_ACCT_OUTPUT_GIGAWORDS, 0,0,0)) {
	val = (uint64_t) ntohl(attr->v.i);
	output |= (val << 32);
      }
      bassignformat(tmp, "&bytes_%s=%lld", direction, (long long) output);
      bconcat(req->url, tmp);
      if (!radius_getattr(pack, &attr, RADIUS_ATTR_ACCT_OUTPUT_PACKETS, 0,0,0)) {
	uint32_t sval = ntohl(attr->v.i);
	bassignformat(tmp, "&pkts_%s=%ld", direction, (long) sval);
	bconcat(req->url, tmp);
      }
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC, 
			RADIUS_VENDOR_CHILLISPOT,
			RADIUS_ATTR_CHILLISPOT_DHCP_HOSTNAME, 0)) {
      bcatcstr(req->url, "&dhcp_host=");
      bassignblk(tmp, attr->v.t, attr->l-2);
      redir_urlencode(tmp, tmp2);
      bconcat(req->url, tmp2);
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC, 
			RADIUS_VENDOR_CHILLISPOT,
			RADIUS_ATTR_CHILLISPOT_DHCP_PARAMETER_REQUEST_LIST, 0)) {
      uint8_t l = attr->l;
      if (l > 2) {
	uint8_t *p = attr->v.t;
	l -= 2;
	bcatcstr(req->url, "&dhcp_pql=");
	while (l--) {
	  bassignformat(tmp, "%.2X", *p++);
	  bconcat(req->url, tmp);
	}
      }
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC, 
			RADIUS_VENDOR_CHILLISPOT,
			RADIUS_ATTR_CHILLISPOT_DHCP_VENDOR_CLASS_ID, 0)) {
      uint8_t l = attr->l;
      if (l > 2) {
	uint8_t *p = attr->v.t;
	l -= 2;
	bcatcstr(req->url, "&dhcp_vcid=");
	while (l--) {
	  bassignformat(tmp, "%.2X", *p++);
	  bconcat(req->url, tmp);
	}
      }
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC, 
			RADIUS_VENDOR_CHILLISPOT,
			RADIUS_ATTR_CHILLISPOT_DHCP_CLIENT_ID, 0)) {
      uint8_t l = attr->l;
      if (l > 2) {
	uint8_t *p = attr->v.t;
	l -= 2;
	bcatcstr(req->url, "&dhcp_cid=");
	while (l--) {
	  bassignformat(tmp, "%.2X", *p++);
	  bconcat(req->url, tmp);
	}
      }
    }
  }

  if (!error) {
    if (_options.uamsecret && _options.uamsecret[0])
      redir_md_param(req->url, _options.uamsecret, "&");
    
#if(_debug_ > 1)
    log_dbg("==> %s", req->url->data);
#endif
    if (http_aaa(radius, req) < 0)
      close_request(req);
    
  } else {
    log_err(0, "problem: %s", error);
  }

  bdestroy(tmp);
  bdestroy(tmp2);
}

int main(int argc, char **argv) {
  struct radius_packet_t radius_pack;
  struct radius_t *radius_auth;
  struct radius_t *radius_acct;
  struct in_addr radiuslisten;

  struct timeval timeout;

  int maxfd = 0;
  fd_set fdread;
  fd_set fdwrite;
  fd_set fdexcep;

  int status;
  int idx, i;

#ifdef USING_CURL
  CURLMsg *msg;
  int msgs_left;
#endif

  int keep_going = 1;
  int reload_config = 1;

  int selfpipe;

  int max_conn_time = 60;

  options_init();

  selfpipe = selfpipe_init();

  chilli_signals(&keep_going, &reload_config);
  selfpipe_trap(SIGUSR2);

  /*
   *  Support a --register mode whereby all subsequent arguments are 
   *  used to create a URL for sending to the back-end. 
   */
  for (i=0; i < argc; i++) {
    if (!strcmp(argv[i],"--register")) {
      http_aaa_register(argc, argv, i);
    }
  }

  process_options(argc, argv, 1);

#if defined (__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)
  net_getmac(_options.dhcpif, nas_hwaddr);
#else
  {
    struct ifreq ifr;
    
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    safe_strncpy(ifr.ifr_name, _options.dhcpif, sizeof(ifr.ifr_name));
    
    if (ioctl(fd, SIOCGIFHWADDR, (caddr_t)&ifr) == 0) {
      memcpy(nas_hwaddr, ifr.ifr_hwaddr.sa_data, PKT_ETH_ALEN);
    }
    
    close(fd);
  }
#endif

#ifdef USING_CURL
  curl_global_init(CURL_GLOBAL_ALL);
  curl_multi = curl_multi_init();
#endif
  
  radiuslisten.s_addr = htonl(INADDR_ANY);

  if (radius_new(&radius_auth, &radiuslisten, 
		 _options.radiusauthport ? 
		 _options.radiusauthport : RADIUS_AUTHPORT, 
		 0, 0) || 
      radius_init_q(radius_auth, 0)) {
    log_err(0, "Failed to create radius");
    return -1;
  }

  if (radius_new(&radius_acct, &radiuslisten, 
		 _options.radiusacctport ? 
		 _options.radiusacctport : RADIUS_ACCTPORT, 
		 0, 0) || 
      radius_init_q(radius_acct, 0)) {
    log_err(0, "Failed to create radius");
    return -1;
  }
  
  radius_set(radius_auth, 0, 0);
  radius_set(radius_acct, 0, 0);
  
  if (_options.gid && setgid(_options.gid)) {
    log_err(errno, "setgid(%d) failed while running with gid = %d\n", 
	    _options.gid, getgid());
  }
  
  if (_options.uid && setuid(_options.uid)) {
    log_err(errno, "setuid(%d) failed while running with uid = %d\n", 
	    _options.uid, getuid());
  }

  while (keep_going) {

    int expired_time = time(NULL) - max_conn_time;

    if (reload_config) {
      reload_options(argc, argv);
      reload_config = 0;
    }

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    FD_SET(selfpipe, &fdread);
    FD_SET(radius_auth->fd, &fdread);
    FD_SET(radius_acct->fd, &fdread);

    for (idx=0; idx < max_requests; idx++) {
      if (requests[idx].inuse && 
	  requests[idx].lasttime < expired_time) {
	log_dbg("remove expired index %d", idx);
	http_aaa_finish(&requests[idx]);
      }
    }

#ifdef USING_CURL
    curl_multi_fdset(curl_multi, &fdread, &fdwrite, &fdexcep, &maxfd);
#else
    for (idx=0; idx < max_requests; idx++) {
      conn_fd(&requests[idx].conn, &fdread, &fdwrite, &fdexcep, &maxfd);
    }
#endif

    if (radius_auth->fd > maxfd)
      maxfd = radius_auth->fd;

    if (radius_acct->fd > maxfd) 
      maxfd = radius_acct->fd;
    
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    status = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);

    switch (status) {
    case -1:
      if (EINTR != errno) {
	log_err(errno, "select() returned -1!");
      }
      break;  

    case 0:
    default:

      if (status > 0) {
	struct sockaddr_in addr;
	socklen_t fromlen = sizeof(addr);
	
	if (FD_ISSET(selfpipe, &fdread)) {
	  int signo = chilli_handle_signal(0, 0);
	  if (signo) {
#if(_debug_)
	    log_dbg("main-proxy signal %d", signo);
#endif
	    switch(signo) {
	    case SIGUSR2: print_requests(); break;
	    default: break;
	    }
	  }
	}

	if (FD_ISSET(radius_auth->fd, &fdread)) {
	  /*
	   *    ---> Authentication
	   */
	  
	  if ((status = recvfrom(radius_auth->fd, 
				 &radius_pack, sizeof(radius_pack), 0, 
				 (struct sockaddr *) &addr, &fromlen)) <= 0) {
	    log_err(errno, "recvfrom() failed");
	    
	    return -1;
	  }

	  process_radius(radius_auth, &radius_pack, &addr);
	}
	
	if (FD_ISSET(radius_acct->fd, &fdread)) {
	  /*
	   *    ---> Accounting
	   */
	  
#if(_debug_)
	  log_dbg("received accounting");
#endif
	  
	  if ((status = recvfrom(radius_acct->fd, 
				 &radius_pack, sizeof(radius_pack), 0, 
			       (struct sockaddr *) &addr, &fromlen)) <= 0) {
	    log_err(errno, "recvfrom() failed");
	    return -1;
	  }
	  
	  process_radius(radius_acct, &radius_pack, &addr);
	}
      }

#ifdef USING_CURL
      while(CURLM_CALL_MULTI_PERFORM ==
	    curl_multi_perform(curl_multi, &still_running));

#if(_debug_ > 1)
      log_dbg("curl still running %d", still_running);
#endif
      
      while ((msg = curl_multi_info_read(curl_multi, &msgs_left))) {

#if(_debug_ > 1)
	log_dbg("curl messages left %d", msgs_left);
#endif

	if (msg->msg == CURLMSG_DONE) {
	  
	  int found = 0;
	  
	  /* Find out which handle this message is about */ 
	  for (idx=0; (!found && (idx < max_requests)); idx++) 
	    found = (msg->easy_handle == requests[idx].curl);
	  
	  if (found) {
	    --idx;
#if(_debug_)
	    log_dbg("HTTP completed with status %d\n", msg->data.result);
#endif
	    http_aaa_finish(&requests[idx]);
	  } else {
	    log_err(0, "Could not find request in queue");
	  }
	}
      }
#else
      for (idx=0; idx < max_requests; idx++) {
	conn_update(&requests[idx].conn, &fdread, &fdwrite, &fdexcep);
      }
#endif

      break;
    }
  }

  radius_free(radius_auth);
  radius_free(radius_acct);

#ifdef USING_CURL
  curl_multi_cleanup(curl_multi);
  curl_global_cleanup();
#endif

  selfpipe_finish();

  return 0;
}
