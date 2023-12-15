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

#include "chilli.h"

#ifdef HAVE_GLOB
#include <glob.h>
#endif

#define QUERY_TIMEOUT 10

struct options_t _options;

typedef struct _cmd_info {
  int type;
  char *command;
  char *desc;
} cmd_info;

static cmd_info commands[] = {
  { CMDSOCK_LIST,          "list",          NULL },
  { CMDSOCK_LIST_IPPOOL,   "listippool",    NULL },
  { CMDSOCK_LIST_RADQUEUE, "listradqueue",  NULL },
  { CMDSOCK_LIST_GARDEN,   "listgarden",    NULL },
  { CMDSOCK_RELOAD,        "reload",        NULL },
  { CMDSOCK_DHCP_LIST,     "dhcp-list",     NULL },
  { CMDSOCK_DHCP_RELEASE,  "dhcp-release",  NULL },
  { CMDSOCK_AUTHORIZE,     "authorize",     NULL },
  { CMDSOCK_LOGIN,         "login",         NULL },
  { CMDSOCK_UPDATE,        "update",        NULL },
  { CMDSOCK_LOGOUT,        "logout",        NULL },
  { CMDSOCK_LOGOUT,        "logoff",        NULL },
  { CMDSOCK_DHCP_DROP,     "drop",          NULL },
  { CMDSOCK_DHCP_DROP,     "block",         NULL },
  { CMDSOCK_PROCS,         "procs",         NULL },
#ifdef ENABLE_MULTIROUTE
  { CMDSOCK_ROUTE,         "route",         NULL },
  { CMDSOCK_ROUTE_GW,      "routegw",       NULL },
#endif
#ifdef ENABLE_CLUSTER
  { CMDSOCK_PEERS,         "peers",         NULL },
  { CMDSOCK_PEER_SET,      "peerset",       NULL },
#endif
#ifdef ENABLE_STATFILE
  { CMDSOCK_STATUSFILE,    "statusfile",    NULL },
#endif
  { CMDSOCK_ADD_GARDEN,  "addgarden", NULL },
  { CMDSOCK_REM_GARDEN,  "remgarden", NULL },
#ifdef ENABLE_INSPECT
  { CMDSOCK_INSPECT,  "inspect", NULL },
#endif
#if defined(ENABLE_LOCATION) && defined(HAVE_AVL)
  { CMDSOCK_LISTLOC,       "listloc",       NULL },
  { CMDSOCK_LISTLOCSUM,    "listlocsum",    NULL },
#endif
  { 0, NULL, NULL }
};

typedef enum _cmd_field_type {
  CMDSOCK_FIELD_NONE = 0,
  CMDSOCK_FIELD_STRING,
  CMDSOCK_FIELD_INTEGER,
  CMDSOCK_FIELD_IPV4,
  CMDSOCK_FIELD_MAC,
} cmd_field_type;

struct cmd_arguments {
  char *name;
  cmd_field_type type;    /* 0=none, 1=string, 2=integer, 3=ip */
  int length;
  void *field;
  char *desc;
  uint16_t *flag;
  uint16_t flagbit;
};

static struct cmdsock_request request;

static struct cmd_arguments args[] = {
  { "ip", 
    CMDSOCK_FIELD_IPV4, 
    sizeof(request.ip),
    &request.ip,
    "IP address of session to perform action on", 0, 0 },
  { "mac",
    CMDSOCK_FIELD_MAC, 
    sizeof(request.mac),
    request.mac,
    "MAC address of session to perform action on", 0, 0 },
  { "sessionid",
    CMDSOCK_FIELD_STRING, 
    sizeof(request.d.sess.sessionid),
    request.d.sess.sessionid,
    "Session-id of session to perform action on", 0, 0 },
  { "username",
    CMDSOCK_FIELD_STRING, 
    sizeof(request.d.sess.username),
    request.d.sess.username,
    "Username to use in RADIUS 'login' or authorization", 0, 0 },
  { "password",
    CMDSOCK_FIELD_STRING, 
    sizeof(request.d.sess.password),
    request.d.sess.password,
    "Password to be used for 'login' command", 0, 0 },
  { "sessiontimeout",
    CMDSOCK_FIELD_INTEGER, 
    sizeof(request.d.sess.params.sessiontimeout),
    &request.d.sess.params.sessiontimeout,
    "Max session time (in seconds)", 0, 0 },
  { "idletimeout",
    CMDSOCK_FIELD_INTEGER, 
    sizeof(request.d.sess.params.idletimeout),
    &request.d.sess.params.idletimeout,
    "Max idle time (in seconds)", 0, 0 },
  { "interiminterval",
    CMDSOCK_FIELD_INTEGER, 
    sizeof(request.d.sess.params.interim_interval),
    &request.d.sess.params.interim_interval,
    "Accounting interim interval",  0, 0 },
  { "maxoctets",
    CMDSOCK_FIELD_INTEGER, 
    sizeof(request.d.sess.params.maxtotaloctets),
    &request.d.sess.params.maxtotaloctets,
    "Max input + output octets (bytes)", 0, 0 },
  { "maxinputoctets",
    CMDSOCK_FIELD_INTEGER, 
    sizeof(request.d.sess.params.maxinputoctets),
    &request.d.sess.params.maxinputoctets,
    "Max input octets (bytes)", 0, 0 },
  { "maxoutputoctets",
    CMDSOCK_FIELD_INTEGER, 
    sizeof(request.d.sess.params.maxoutputoctets),
    &request.d.sess.params.maxoutputoctets,
    "Max output octets (bytes)", 0, 0 },
  { "maxbwup", 
    CMDSOCK_FIELD_INTEGER, 
    sizeof(request.d.sess.params.bandwidthmaxup),
    &request.d.sess.params.bandwidthmaxup,
    "Max bandwidth up", 0, 0 },
  { "maxbwdown",
    CMDSOCK_FIELD_INTEGER, 
    sizeof(request.d.sess.params.bandwidthmaxdown),
    &request.d.sess.params.bandwidthmaxdown,
    "Max bandwidth down", 0, 0 },
  { "splash",
    CMDSOCK_FIELD_STRING, 
    sizeof(request.d.sess.params.url),
    &request.d.sess.params.url,
    "Set splash page",
    &request.d.sess.params.flags, REQUIRE_UAM_SPLASH },
#ifdef ENABLE_REDIRINJECT
  { "inject",
    CMDSOCK_FIELD_STRING, 
    sizeof(request.d.sess.params.url),
    &request.d.sess.params.url,
    "Inject url flag",
    &request.d.sess.params.flags, UAM_INJECT_URL | REQUIRE_UAM_AUTH },
#endif
  { "url",
    CMDSOCK_FIELD_STRING, 
    sizeof(request.d.sess.params.url),
    &request.d.sess.params.url,
    "Set redirect url",
    &request.d.sess.params.flags, REQUIRE_REDIRECT },
#ifdef ENABLE_MULTIROUTE
  { "routeidx",
    CMDSOCK_FIELD_INTEGER, 
    sizeof(request.d.sess.params.routeidx),
    &request.d.sess.params.routeidx,
    "Route interface index",  0, 0 },
#endif
#ifdef ENABLE_LOCATION
  { "location",
    CMDSOCK_FIELD_STRING, 
    sizeof(request.d.sess.location),
    request.d.sess.location,
    "Location of session to perform action on", 0, 0 },
#endif
  { "noacct",
    CMDSOCK_FIELD_NONE, 0, 0,
    "No accounting flag",
    &request.d.sess.params.flags, NO_ACCOUNTING },
  { "data",
    CMDSOCK_FIELD_STRING, 
    sizeof(request.d.data),
    &request.d.data,
    "Text configuration line",
    0, 1 },
  /* more... */
};

static int count = sizeof(args)/sizeof(struct cmd_arguments);

static int parse_mac(uint8_t *mac, char *string) {
  unsigned int temp[PKT_ETH_ALEN];
  char macstr[RADIUS_ATTR_VLEN];
  int macstrlen;
  int i;

  if ((macstrlen = strlen(string)) >= (RADIUS_ATTR_VLEN-1)) {
    fprintf(stderr, "%s: bad MAC address\n", string);
    return -1;
  }
	  
  memcpy(macstr, string, macstrlen);
  macstr[macstrlen] = 0;
	  
  for (i=0; i<macstrlen; i++) 
    if (!isxdigit((int) macstr[i])) 
      macstr[i] = 0x20;
  
  if (sscanf(macstr, "%2x %2x %2x %2x %2x %2x", 
	     &temp[0], &temp[1], &temp[2], 
	     &temp[3], &temp[4], &temp[5]) != 6) {
    fprintf(stderr, "%s: bad MAC address\n", string);
    return -1;
  }
  
  for (i = 0; i < PKT_ETH_ALEN; i++) 
    mac[i] = temp[i];

  return 0;
}

static int usage(char *program) {
  int i;

  fprintf(stderr, "Usage: %s [ -s <socket> ] [ -P <port> ] <command> [<arguments>]\n", program);
  fprintf(stderr, "  socket = full path to UNIX domain socket (e.g. /var/run/chilli.sock)\n");
  fprintf(stderr, "  port = TCP socket port to connect to. Default is 42424\n");

  fprintf(stderr, "  Available Commands:\n    ");
  for (i=0; commands[i].command; i++) {
    fprintf(stderr, "%s%s", 
	    i > 0 ? ", " : "",
	    commands[i].command);
  }

  fprintf(stderr, "\n  Available Arguments:\n");
  for (i=0; i < count; i++) {
    fprintf(stderr, "    %-18s%-7s - type: %-4s [%4d] - %s\n", 
	    args[i].name, 
	    args[i].type == CMDSOCK_FIELD_NONE ? "" :"<value>",
	    args[i].type == CMDSOCK_FIELD_NONE ? "flag" :
	    args[i].type == CMDSOCK_FIELD_MAC ? "mac" :
	    args[i].type == CMDSOCK_FIELD_STRING ? "char" :
	    args[i].type == CMDSOCK_FIELD_INTEGER ? "int" :
	    args[i].type == CMDSOCK_FIELD_IPV4 ? "ip" : "!", 
	    args[i].length, args[i].desc);
  }
  fprintf(stderr, "The ip and/or sessionid is required.\n");
  
  return 1;
}

static void timeout_alarm(int signum) {
  fprintf(stderr, "Timeout\n");
  exit(1);
}

static int process_args(int argc, char *argv[], int argidx) {
  int c = argc - argidx;
  char is_data = 0;
  
  while(c > 0) {
    int i;
    
    for (i=0; i < count; i++) {
      
      if (!strcmp(argv[argidx], args[i].name)) {
	
	if (args[i].flag) {
	  *(args[i].flag) |= args[i].flagbit;
	} else if (args[i].flagbit == 1) {
	  if (is_data == 0) 
	    is_data = 1;
	  else {
	    fprintf(stderr, "data can only be once and by itself\n");
	    return argidx;
	  }
	} else {
	  if (i > 1) {
	    if (is_data == 1) {
	      fprintf(stderr, "data can only be once and by itself\n");
	      return argidx;
	    }
	    is_data = -1;
	  }
	}
	
	if (c == 1 && args[i].length) {
	  fprintf(stderr, "Argument %s requires a value\n", argv[argidx]);
	  return usage(argv[0]);
	}
	
	switch(args[i].type) {
	case CMDSOCK_FIELD_NONE:
	  break;
	case CMDSOCK_FIELD_MAC:
	  parse_mac(((uint8_t *)args[i].field), argv[argidx+1]);
	  break;
	case CMDSOCK_FIELD_STRING:
	  safe_strncpy(((char *)args[i].field), argv[argidx+1], args[i].length);
	  break;
	case CMDSOCK_FIELD_INTEGER:
	  switch(args[i].length) {
	  case 1:
	    *((uint8_t *)args[i].field) |= (uint8_t)atoi(argv[argidx+1]);
	    break;
	  case 2:
	    *((uint16_t *)args[i].field) |= (uint16_t)atoi(argv[argidx+1]);
	    break;
	  case 4:
	    *((uint32_t *)args[i].field) |= (uint32_t)atol(argv[argidx+1]);
	    break;
	  case 8:
	    *((uint64_t *)args[i].field) |= (uint64_t)atol(argv[argidx+1]);
	    break;
	  }
	  break;
	case CMDSOCK_FIELD_IPV4: 
	  {
	    struct in_addr ip;
	    if (!inet_aton(argv[argidx+1], &ip)) {
	      fprintf(stderr, "Invalid IP Address: %s\n", argv[argidx+1]);
	      return usage(argv[0]);
	    }
	    ((struct in_addr *)args[i].field)->s_addr = ip.s_addr;
	    break;
	  }
	}
	break;
      }
    }
    
    if (i == count) {
      if (request.type == CMDSOCK_LOGOUT)
	break;
      fprintf(stderr, "Unknown argument: %s\n", argv[argidx]);
      return usage(argv[0]);
    }
    
    if (args[i].length) {
      c -= 2;
      argidx += 2;
    } else {
      c --;
      argidx ++;
    }
  }

  return argidx;
}

static int chilli_communicate(int s,
			       struct cmdsock_request *buffer,
			       size_t blen) {
  char line[1024];
  int len;

  if (safe_write(s, buffer, blen) != blen) {
    perror("write");
    return -1;
  }
  
  while((len = safe_read(s, line, sizeof(line)-1)) > 0) {
    if (write(1, line, len) != len) {
      perror("write");
      return -1;
    }
  }
  
  if (len < 0) {
    perror("read");
    return -1;
  }

  return 0;
}

int main(int argc, char **argv) {
  /*
   *   chilli_query [ -s <unix-socket> ] <command> [<argument>]
   *   (or maybe this should get the unix-socket from the config file)
   */

#ifdef HAVE_GLOB
  char *cmdsock = DEFSTATEDIR"/chilli*.sock";
  glob_t globbuf;
  int i;
#else
  char *cmdsock = DEFSTATEDIR"/chilli.sock";
#endif
  int s;
  int cmdsockport = 0;
  struct sockaddr_un remote;
  char *cmd;
  int argidx = 1;
  int len;

  struct itimerval itval;

#ifdef ENABLE_CLUSTER
  int peerid = -1;
#endif

  int query_timeout = QUERY_TIMEOUT;

  if (getenv("QUERY_TIMEOUT")) {
    query_timeout = atoi(getenv("QUERY_TIMEOUT"));
  }
  
  set_signal(SIGALRM, timeout_alarm);
  
  memset(&itval, 0, sizeof(itval));
  itval.it_interval.tv_sec = query_timeout;
  itval.it_interval.tv_usec = 0; 
  itval.it_value.tv_sec = query_timeout;
  itval.it_value.tv_usec = 0; 
  
  if (setitimer(ITIMER_REAL, &itval, NULL)) {
    log_err(errno, "setitimer() failed!");
  }
  
  if (argc < 2) return usage(argv[0]);
  
  if (*argv[argidx] == '/') {
    /* for backward support, ignore a unix-socket given as first arg */
    if (argc < 3) return usage(argv[0]);
    cmdsock = argv[argidx++];
  } 
  
  memset(&request,0,sizeof(request));
  
  while (argidx < argc && *argv[argidx] == '-') {
    if (!strcmp(argv[argidx], "-s")) {
      argidx++;
      if (argidx >= argc) return usage(argv[0]);
      cmdsock = argv[argidx++];
#ifdef ENABLE_CLUSTER
    } else if (!strcmp(argv[argidx], "-p")) {
      argidx++;
      if (argidx >= argc) return usage(argv[0]);
      peerid = atoi(argv[argidx++]);
#endif
    } else if (!strcmp(argv[argidx], "-json")) {
      request.options |= CMDSOCK_OPT_JSON;
      argidx++;
    } else if (!strcmp(argv[argidx], "-P")) {
      argidx++;
      if (argidx >= argc) return usage(argv[0]);
      cmdsockport = atoi(argv[argidx++]);
    }
  }
  
  if (argidx >= argc) return usage(argv[0]);
  
  cmd = argv[argidx++];
  for (s = 0; commands[s].command; s++) {
    if (!strcmp(cmd, commands[s].command)) {
      request.type = commands[s].type;
      
      switch(request.type) {

#ifdef ENABLE_INSPECT
      case CMDSOCK_INSPECT:
#endif
#if defined(ENABLE_LOCATION) && defined(HAVE_AVL)
      case CMDSOCK_LISTLOC:
      case CMDSOCK_LISTLOCSUM:
#endif
      case CMDSOCK_LIST:
      case CMDSOCK_LOGIN:
      case CMDSOCK_LOGOUT:
      case CMDSOCK_UPDATE:
      case CMDSOCK_AUTHORIZE:
      case CMDSOCK_ADD_GARDEN:
      case CMDSOCK_REM_GARDEN:
	argidx = process_args(argc, argv, argidx);
	if (request.type != CMDSOCK_LOGOUT || argidx >= argc)
	  break;
	/* else, drop through */
      case CMDSOCK_DHCP_DROP:
      case CMDSOCK_DHCP_RELEASE:
	{
	  if (argc < argidx+1) {
	    fprintf(stderr, "%s requires a MAC address argument\n", cmd);
	    return usage(argv[0]);
	  }

	  if (parse_mac(request.mac, argv[argidx]))
	    return usage(argv[0]);
	  
	  /* do another switch to pick up param configs for authorize */
	}
	break;
#ifdef ENABLE_MULTIROUTE
      case CMDSOCK_ROUTE:
      case CMDSOCK_ROUTE_GW:
	{
	  unsigned int temp[PKT_ETH_ALEN];
	  char macstr[RADIUS_ATTR_VLEN];
	  int macstrlen;
	  int i;
	  
	  if (argc < argidx + 2) {
	    break;
	  }
	  
	  if ((macstrlen = strlen(argv[argidx])) >= (RADIUS_ATTR_VLEN-1)) {
	    fprintf(stderr, "%s: bad MAC address\n", argv[argidx]);
	    break;
	  }

	  memcpy(macstr, argv[argidx], macstrlen);
	  macstr[macstrlen] = 0;

	  for (i=0; i<macstrlen; i++) 
	    if (!isxdigit((int) macstr[i])) 
	      macstr[i] = 0x20;

	  if (sscanf(macstr, "%2x %2x %2x %2x %2x %2x", 
		     &temp[0], &temp[1], &temp[2], 
		     &temp[3], &temp[4], &temp[5]) != 6) {
	    fprintf(stderr, "%s: bad MAC address\n", argv[argidx]);
	    break;
	  }

	  for (i = 0; i < PKT_ETH_ALEN; i++) 
	    request.mac[i] = temp[i];

	  argidx++;
	  request.d.sess.params.routeidx = atoi(argv[argidx]);

	  if (request.type != CMDSOCK_ROUTE_GW)
	    request.type = CMDSOCK_ROUTE_SET;
	  
	  /* do another switch to pick up param configs for authorize */
	}
	break;
#endif
      }
      break;
    }
  }
  
  if (!commands[s].command) {
    fprintf(stderr,"unknown command: %s\n",cmd);
    exit(1);
  }
  
#ifdef ENABLE_CLUSTER
  if (peerid > -1) {

    struct sockaddr_in s;
    int blen = sizeof(struct pkt_chillihdr_t);
    uint8_t b[blen];

    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    printf("blen %d\n", blen);

    if (fd < 0) {
      log_err(errno,"socket() failed");
      exit(1);
    }

    memset(&s, 0, sizeof(struct sockaddr_in));
    s.sin_family = AF_INET;
    s.sin_port = htons(10203);
    s.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    
    (void) safe_sendto(fd, b, blen, 0, 
		       (struct sockaddr *)&s, 
		       sizeof(struct sockaddr_in));

    return 0;
  }
#endif

  if (cmdsockport) {
    struct sockaddr_in remote_port;
    struct in_addr addr;

    if ((s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
      perror("socket");
      exit(1);
    }

    inet_aton("127.0.0.1", &addr);
    memset(&remote_port, 0, sizeof(struct sockaddr_in));
    remote_port.sin_family = AF_INET;
    remote_port.sin_port = htons(cmdsockport);
    remote_port.sin_addr.s_addr = addr.s_addr;

    if (connect(s, (struct sockaddr *)&remote_port, sizeof(remote_port)) == -1) {
      perror("connect");
      exit(1);
    }

    if (chilli_communicate(s, &request, sizeof(request)) != 0) {
      perror("write");
      exit(1);
    }

    shutdown(s, 2);
    close(s);

    return 0;
  }
  
#ifdef HAVE_GLOB
  globbuf.gl_offs = 0;
  glob(cmdsock, GLOB_DOOFFS, NULL, &globbuf);

  if (!globbuf.gl_pathc) {
    fprintf(stderr,"no cmdsock sockets: %s\n",cmdsock);
    exit(1);
  }
  
  for (i=0 ; i < globbuf.gl_pathc; i++) {
    cmdsock = globbuf.gl_pathv[i];
    if (globbuf.gl_pathc>1) {
      char header[256];
      int headerlen;
      safe_snprintf(header, sizeof(header), "\nQuerying socket %s\n", cmdsock);
      headerlen=strlen(header);
      if (write(1, header, strlen(header))!=headerlen) {
        perror("write");
        exit(1);
      }
    }
#endif

  if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  remote.sun_family = AF_UNIX;
  strcpy(remote.sun_path, cmdsock);

#if defined (__FreeBSD__)  || defined (__APPLE__) || defined (__OpenBSD__)
  remote.sun_len = strlen(remote.sun_path) + 1;
#endif

  len = offsetof(struct sockaddr_un, sun_path) + strlen(remote.sun_path);

  if (connect(s, (struct sockaddr *)&remote, len) == -1) {
    perror("connect");
    exit(1);
  }

  if (chilli_communicate(s, &request, sizeof(request)) != 0) {
    perror("write");
    exit(1);
  }

  shutdown(s, 2);
  close(s);

#ifdef HAVE_GLOB
  }
  globfree(&globbuf);
#endif
  
  return 0;
}
