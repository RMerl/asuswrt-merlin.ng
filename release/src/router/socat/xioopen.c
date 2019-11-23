/* source: xioopen.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this is the source file of the extended open function */

#include "xiosysincludes.h"

#include "xioopen.h"
#include "xiomodes.h"
#include "nestlex.h"

static xiofile_t *xioallocfd(void);

xiosingle_t hugo;
static xiosingle_t *xioparse_single(const char **addr);
static xiofile_t *xioparse_dual(const char **addr);
static int xioopen_dual(xiofile_t *xfd, int xioflags);

const struct addrname addressnames[] = {
#if 1
#if WITH_STDIO
   { "-",		&addr_stdio },
#endif
#if defined(WITH_UNIX) && defined(WITH_ABSTRACT_UNIXSOCKET)
   { "abstract",		&xioaddr_abstract_client },
   { "abstract-client",		&xioaddr_abstract_client },
   { "abstract-connect",	&xioaddr_abstract_connect },
#if WITH_LISTEN
   { "abstract-listen",		&xioaddr_abstract_listen },
#endif
   { "abstract-recv",		&xioaddr_abstract_recv },
   { "abstract-recvfrom",	&xioaddr_abstract_recvfrom },
   { "abstract-sendto",		&xioaddr_abstract_sendto },
#endif /* defined(WITH_UNIX) && defined(WITH_ABSTRACT_UNIXSOCKET) */
#if WITH_CREAT
   { "creat",	&addr_creat },
   { "create",	&addr_creat },
#endif
#if WITH_GENERICSOCKET
   { "datagram",		&xioaddr_socket_datagram },
   { "dgram",			&xioaddr_socket_datagram },
#endif
#if WITH_PIPE
   { "echo",		&addr_pipe },
#endif
#if WITH_EXEC
   { "exec",		&addr_exec },
#endif
#if WITH_FDNUM
   { "fd",		&addr_fd },
#endif
#if WITH_PIPE
   { "fifo",		&addr_pipe },
#endif
#if WITH_FILE
   { "file",		&addr_open },
#endif
#if WITH_GOPEN
   { "gopen",	&addr_gopen },
#endif
#if WITH_INTERFACE
   { "if",		&xioaddr_interface },
#endif
#if (WITH_IP4 || WITH_IP6) && WITH_TCP
   { "inet",		&addr_tcp_connect },
#endif
#if (WITH_IP4 || WITH_IP6) && WITH_TCP && WITH_LISTEN
   { "inet-l",	&addr_tcp_listen },
   { "inet-listen",	&addr_tcp_listen },
#endif
#if WITH_IP4 && WITH_TCP
   { "inet4",	&addr_tcp4_connect },
#endif
#if WITH_IP4 && WITH_TCP && WITH_LISTEN
   { "inet4-l",	&addr_tcp4_listen },
   { "inet4-listen",	&addr_tcp4_listen },
#endif
#if WITH_IP6 && WITH_TCP
   { "inet6",	&addr_tcp6_connect },
#endif
#if WITH_IP6 && WITH_TCP && WITH_LISTEN
   { "inet6-l",	&addr_tcp6_listen },
   { "inet6-listen",	&addr_tcp6_listen },
#endif
#if WITH_INTERFACE
   { "interface",	&xioaddr_interface },
#endif
#if WITH_RAWIP
#if (WITH_IP4 || WITH_IP6)
   { "ip",		&addr_rawip_sendto },
   { "ip-datagram",	&addr_rawip_datagram },
   { "ip-dgram",	&addr_rawip_datagram },
   { "ip-recv",		&addr_rawip_recv },
   { "ip-recvfrom",	&addr_rawip_recvfrom },
   { "ip-send",	&addr_rawip_sendto },
   { "ip-sendto",	&addr_rawip_sendto },
#endif
#if WITH_IP4
   { "ip4",		&addr_rawip4_sendto },
   { "ip4-datagram",	&addr_rawip4_datagram },
   { "ip4-dgram",	&addr_rawip4_datagram },
   { "ip4-recv",	&addr_rawip4_recv },
   { "ip4-recvfrom",	&addr_rawip4_recvfrom },
   { "ip4-send",	&addr_rawip4_sendto },
   { "ip4-sendto",	&addr_rawip4_sendto },
#endif
#if WITH_IP6
   { "ip6",		&addr_rawip6_sendto },
   { "ip6-datagram",	&addr_rawip6_datagram },
   { "ip6-dgram",	&addr_rawip6_datagram },
   { "ip6-recv",	&addr_rawip6_recv },
   { "ip6-recvfrom",	&addr_rawip6_recvfrom },
   { "ip6-send",	&addr_rawip6_sendto },
   { "ip6-sendto",	&addr_rawip6_sendto },
#endif
#endif /* WITH_RAWIP */
#if WITH_UNIX
   { "local",	&xioaddr_unix_connect },
#endif
#if WITH_FILE
   { "open",		&addr_open },
#endif
#if WITH_OPENSSL
   { "openssl",		&addr_openssl },
   { "openssl-connect",		&addr_openssl },
#if WITH_LISTEN
   { "openssl-listen",		&addr_openssl_listen },
#endif
#endif
#if WITH_PIPE
   { "pipe",		&addr_pipe },
#endif
#if WITH_PROXY
   { "proxy",		&addr_proxy_connect },
   { "proxy-connect",	&addr_proxy_connect },
#endif
#if WITH_PTY
   { "pty",		&addr_pty },
#endif
#if WITH_READLINE
   { "readline",	&addr_readline },
#endif
#if (WITH_IP4 || WITH_IP6) && WITH_SCTP
   { "sctp",		&addr_sctp_connect },
   { "sctp-connect",	&addr_sctp_connect },
#if WITH_LISTEN
   { "sctp-l",		&addr_sctp_listen },
   { "sctp-listen",	&addr_sctp_listen },
#endif
#if WITH_IP4
   { "sctp4",		&addr_sctp4_connect },
   { "sctp4-connect",	&addr_sctp4_connect },
#if WITH_LISTEN
   { "sctp4-l",		&addr_sctp4_listen },
   { "sctp4-listen",	&addr_sctp4_listen },
#endif
#endif /* WITH_IP4 */
#if WITH_IP6
   { "sctp6",		&addr_sctp6_connect },
   { "sctp6-connect",	&addr_sctp6_connect },
#if WITH_LISTEN
   { "sctp6-l",		&addr_sctp6_listen },
   { "sctp6-listen",	&addr_sctp6_listen },
#endif
#endif /* WITH_IP6 */
#endif /* (WITH_IP4 || WITH_IP6) && WITH_SCTP */
#if WITH_GENERICSOCKET
   { "sendto",			&xioaddr_socket_sendto },
#endif
#if WITH_GENERICSOCKET
   { "socket-connect",		&xioaddr_socket_connect },
   { "socket-datagram",		&xioaddr_socket_datagram },
#if WITH_LISTEN
   { "socket-listen",		&xioaddr_socket_listen },
#endif /* WITH_LISTEN */
   { "socket-recv",		&xioaddr_socket_recv },
   { "socket-recvfrom",		&xioaddr_socket_recvfrom },
   { "socket-sendto",		&xioaddr_socket_sendto },
#endif
#if WITH_SOCKS4
   { "socks",	&addr_socks4_connect },
   { "socks4",	&addr_socks4_connect },
#endif
#if WITH_SOCKS4A
   { "socks4a",	&addr_socks4a_connect },
#endif
#if WITH_OPENSSL
   { "ssl",		&addr_openssl },
#if WITH_LISTEN
   { "ssl-l",		&addr_openssl_listen },
#endif
#endif
#if WITH_STDIO
   { "stderr",	&addr_stderr },
   { "stdin",	&addr_stdin },
   { "stdio",	&addr_stdio },
   { "stdout",	&addr_stdout },
#endif
#if WITH_SYSTEM
   { "system",	&addr_system },
#endif
#if (WITH_IP4 || WITH_IP6) && WITH_TCP
   { "tcp",		&addr_tcp_connect },
   { "tcp-connect",	&addr_tcp_connect },
#endif
#if (WITH_IP4 || WITH_IP6) && WITH_TCP && WITH_LISTEN
   { "tcp-l",	&addr_tcp_listen },
   { "tcp-listen",	&addr_tcp_listen },
#endif
#if WITH_IP4 && WITH_TCP
   { "tcp4",		&addr_tcp4_connect },
   { "tcp4-connect",	&addr_tcp4_connect },
#endif
#if WITH_IP4 && WITH_TCP && WITH_LISTEN
   { "tcp4-l",	&addr_tcp4_listen },
   { "tcp4-listen",	&addr_tcp4_listen },
#endif
#if WITH_IP6 && WITH_TCP
   { "tcp6",		&addr_tcp6_connect },
   { "tcp6-connect",	&addr_tcp6_connect },
#endif
#if WITH_IP6 && WITH_TCP && WITH_LISTEN
   { "tcp6-l",	&addr_tcp6_listen },
   { "tcp6-listen",	&addr_tcp6_listen },
#endif
#if WITH_TUN
   { "tun",		&xioaddr_tun },
#endif
#if (WITH_IP4 || WITH_IP6) && WITH_UDP
   { "udp",		&addr_udp_connect },
#endif
#if (WITH_IP4 || WITH_IP6) && WITH_UDP
   { "udp-connect",	&addr_udp_connect },
   { "udp-datagram",	&addr_udp_datagram },
   { "udp-dgram",	&addr_udp_datagram },
#endif
#if (WITH_IP4 || WITH_IP6) && WITH_UDP && WITH_LISTEN
   { "udp-l",	&addr_udp_listen },
   { "udp-listen",	&addr_udp_listen },
#endif
#if (WITH_IP4 || WITH_IP6) && WITH_UDP
   { "udp-recv",	&addr_udp_recv },
   { "udp-recvfrom",	&addr_udp_recvfrom },
   { "udp-send",	&addr_udp_sendto },
   { "udp-sendto",	&addr_udp_sendto },
#endif
#if WITH_IP4 && WITH_UDP
   { "udp4",		&addr_udp4_connect },
   { "udp4-connect",	&addr_udp4_connect },
   { "udp4-datagram",	&addr_udp4_datagram },
   { "udp4-dgram",	&addr_udp4_datagram },
#endif
#if WITH_IP4 && WITH_UDP && WITH_LISTEN
   { "udp4-l",		&addr_udp4_listen },
   { "udp4-listen",	&addr_udp4_listen },
#endif
#if WITH_IP4 && WITH_UDP
   { "udp4-recv",	&addr_udp4_recv },
   { "udp4-recvfrom",	&addr_udp4_recvfrom },
   { "udp4-send",	&addr_udp4_sendto },
   { "udp4-sendto",	&addr_udp4_sendto },
#endif
#if WITH_IP6 && WITH_UDP
   { "udp6",		&addr_udp6_connect },
   { "udp6-connect",	&addr_udp6_connect },
   { "udp6-datagram",	&addr_udp6_datagram },
   { "udp6-dgram",	&addr_udp6_datagram },
#endif
#if WITH_IP6 && WITH_UDP && WITH_LISTEN
   { "udp6-l",		&addr_udp6_listen },
   { "udp6-listen",	&addr_udp6_listen },
#endif
#if WITH_IP6 && WITH_UDP
   { "udp6-recv",	&addr_udp6_recv },
   { "udp6-recvfrom",	&addr_udp6_recvfrom },
   { "udp6-send",	&addr_udp6_sendto },
   { "udp6-sendto",	&addr_udp6_sendto },
#endif
#if WITH_UNIX
   { "unix",		&xioaddr_unix_client },
   { "unix-client",	&xioaddr_unix_client },
   { "unix-connect",	&xioaddr_unix_connect },
#endif
#if WITH_UNIX && WITH_LISTEN
   { "unix-l",		&xioaddr_unix_listen },
   { "unix-listen",	&xioaddr_unix_listen },
#endif
#if WITH_UNIX
   { "unix-recv",	&xioaddr_unix_recv },
   { "unix-recvfrom",	&xioaddr_unix_recvfrom },
   { "unix-send",	&xioaddr_unix_sendto },
   { "unix-sendto",	&xioaddr_unix_sendto },
#endif
#else /* !0 */
#  if WITH_INTEGRATE
#    include "xiointegrate.c"
#  else
#    include "xioaddrtab.c"
#  endif
#endif /* !0 */
   { NULL }	/* end marker */
} ;

int xioopen_single(xiofile_t *xfd, int xioflags);


/* prepares a xiofile_t record for dual address type:
   sets the tag and allocates memory for the substreams.
   returns 0 on success, or <0 if an error occurred.
*/
int xioopen_makedual(xiofile_t *file) {
   file->tag = XIO_TAG_DUAL;
   file->common.flags = XIO_RDWR;
   if ((file->dual.stream[0] = (xiosingle_t *)xioallocfd()) == NULL)
      return -1;
   file->dual.stream[0]->flags = XIO_RDONLY;
   if ((file->dual.stream[1] = (xiosingle_t *)xioallocfd()) == NULL)
      return -1;
   file->dual.stream[1]->flags = XIO_WRONLY;
   return 0;
}

static xiofile_t *xioallocfd(void) {
   xiofile_t *fd;

   if ((fd = Calloc(1, sizeof(xiofile_t))) == NULL) {
      return NULL;
   }
   /* some default values; 0's and NULL's need not be applied (calloc'ed) */
   fd->common.tag       = XIO_TAG_INVALID;
/* fd->common.addr      = NULL; */
   fd->common.flags     = XIO_RDWR;

#if WITH_RETRY
/* fd->stream.retry	= 0; */
/* fd->stream.forever	= false; */
   fd->stream.intervall.tv_sec  = 1;
/* fd->stream.intervall.tv_nsec = 0; */
#endif /* WITH_RETRY */
/* fd->common.ignoreeof = false; */
/* fd->common.eof       = 0; */

   fd->stream.fd        = -1;
   fd->stream.dtype     = XIODATA_STREAM;
#if _WITH_SOCKET
/* fd->stream.salen     = 0; */
#endif /* _WITH_SOCKET */
   fd->stream.howtoend  = END_UNSPEC;
/* fd->stream.name      = NULL; */
   fd->stream.escape	= -1;
/* fd->stream.para.exec.pid = 0; */
   fd->stream.lineterm  = LINETERM_RAW;

   return fd;
}


/* parse the argument that specifies a two-directional data stream
   and open the resulting address
 */
xiofile_t *xioopen(const char *addr,	/* address specification */
		   int xioflags) {
   xiofile_t *xfd;

   if (xioinitialize() < 0) {
      return NULL;
   }

   if ((xfd = xioparse_dual(&addr)) == NULL) {
      return NULL;
   }
   /*!! support n socks */
   if (!sock[0]) {
      sock[0] = xfd;
   } else {
      sock[1] = xfd;
   }
   if (xioopen_dual(xfd, xioflags) < 0) {
      /*!!! free something? */
      return NULL;
   }

   return xfd;
}

static xiofile_t *xioparse_dual(const char **addr) {
   xiofile_t *xfd;
   xiosingle_t *sfd1;

   /* we parse a single address */
   if ((sfd1 = xioparse_single(addr)) == NULL) {
      return NULL;
   }

   /* and now we see if we reached a dual-address separator */
   if (!strncmp(*addr, xioopts.pipesep, strlen(xioopts.pipesep))) {
      /* yes we reached it, so we parse the second single address */
      *addr += strlen(xioopts.pipesep);

      if ((xfd = xioallocfd()) == NULL) {
	 free(sfd1); /*! and maybe have free some if its contents */
	 return NULL;
      }
      xfd->tag = XIO_TAG_DUAL;
      xfd->dual.stream[0] = sfd1;
      if ((xfd->dual.stream[1] = xioparse_single(addr)) == NULL) {
	 return NULL;
      }

      return xfd;
   }

   /* a truly single address */
   xfd = (xiofile_t *)sfd1; sfd1 = NULL;

   return xfd;
}

static int xioopen_dual(xiofile_t *xfd, int xioflags) {

   if (xfd->tag == XIO_TAG_DUAL) {
      /* a really dual address */
      if ((xioflags&XIO_ACCMODE) != XIO_RDWR) {
	 Warn("unidirectional open of dual address");
      }
      if (((xioflags&XIO_ACCMODE)+1) & (XIO_RDONLY+1)) {
	 if (xioopen_single((xiofile_t *)xfd->dual.stream[0], XIO_RDONLY|(xioflags&~XIO_ACCMODE&~XIO_MAYEXEC))
	     < 0) {
	    return -1;
	 }
      }
      if (((xioflags&XIO_ACCMODE)+1) & (XIO_WRONLY+1)) {
	 if (xioopen_single((xiofile_t *)xfd->dual.stream[1], XIO_WRONLY|(xioflags&~XIO_ACCMODE&~XIO_MAYEXEC))
	     < 0) {
	    xioclose((xiofile_t *)xfd->dual.stream[0]);
	    return -1;
	 }
      }
      return 0;
   }

   return xioopen_single(xfd, xioflags);
}


static xiosingle_t *xioparse_single(const char **addr) {
   xiofile_t *xfd;
   xiosingle_t *sfd;
   struct addrname *ae;
   const struct addrdesc *addrdesc = NULL;
   const char *ends[4+1];
   const char *hquotes[] = {
      "'",
      NULL
   } ;
   const char *squotes[] = {
      "\"",
      NULL
   } ;
   const char *nests[] = {
      "'", "'",
      "(", ")",
      "[", "]",
      "{", "}",
      NULL
   } ;
   char token[512], *tokp;
   size_t len;
   int i;

   /* init */
   i = 0;
   /*ends[i++] = xioopts.chainsep;*/	/* default: "|" */
   ends[i++] = xioopts.pipesep;		/* default: "!!" */
   ends[i++] = ","/*xioopts.comma*/;		/* default: "," */
   ends[i++] = ":"/*xioopts.colon*/;		/* default: ":" */
   ends[i++] = NULL;

   if ((xfd = xioallocfd()) == NULL) {
      return NULL;
   }
   sfd = &xfd->stream;
   sfd->argc = 0;

   len = sizeof(token); tokp = token;
   if (nestlex(addr, &tokp, &len, ends, hquotes, squotes, nests,
	       true, true, false) < 0) {
      Error2("keyword too long, in address \"%s%s\"", token, *addr);
   }
   *tokp = '\0';  /*! len? */
   ae = (struct addrname *)
      keyw((struct wordent *)&addressnames, token,
	   sizeof(addressnames)/sizeof(struct addrname)-1);

   if (ae) {
      addrdesc = ae->desc;
      /* keyword */
      if ((sfd->argv[sfd->argc++] = strdup(token)) == NULL) {
	 Error1("strdup(\"%s\"): out of memory", token);
      }
   } else {
      if (false) {
	 ;
#if WITH_FDNUM
      } else if (isdigit(token[0]&0xff) && token[1] == '\0') {
	 Info1("interpreting address \"%s\" as file descriptor", token);
	 addrdesc = &addr_fd;
	 if ((sfd->argv[sfd->argc++] = strdup("FD")) == NULL) {
	    Error("strdup(\"FD\"): out of memory");
	 }
	 if ((sfd->argv[sfd->argc++] = strdup(token)) == NULL) {
	    Error1("strdup(\"%s\"): out of memory", token);
	 }
	 /*! check argc overflow */
#endif /* WITH_FDNUM */
#if WITH_GOPEN
      } else if (strchr(token, '/')) {
	 Info1("interpreting address \"%s\" as file name", token);
	 addrdesc = &addr_gopen;
	 if ((sfd->argv[sfd->argc++] = strdup("GOPEN")) == NULL) {
	    Error("strdup(\"GOPEN\"): out of memory");
	 }
	 if ((sfd->argv[sfd->argc++] = strdup(token)) == NULL) {
	    Error1("strdup(\"%s\"): out of memory", token);
	 }
	 /*! check argc overflow */
#endif /* WITH_GOPEN */
      } else {
	 Error1("unknown device/address \"%s\"", token);
	 /*!!! free something*/ return NULL;
      }
   }

   sfd->tag  = XIO_TAG_RDWR;
   sfd->addr = addrdesc;

   while (!strncmp(*addr, xioopts.paramsep, strlen(xioopts.paramsep))) {
      *addr += strlen(xioopts.paramsep);
      len = sizeof(token);  tokp = token;
      if (nestlex(addr, &tokp, &len, ends, hquotes, squotes, nests,
		  true, true, false) != 0) {
	 Error2("syntax error in address \"%s%s\"", token, *addr);
      }
      *tokp = '\0';
      if ((sfd->argv[sfd->argc++] = strdup(token)) == NULL) {
	 Error1("strdup(\"%s\"): out of memory", token);
      }
   }

   if (parseopts(addr, addrdesc->groups, &sfd->opts) < 0) {
      free(xfd);
      return NULL;
   }

   return sfd;
}

int xioopen_single(xiofile_t *xfd, int xioflags) {
   const struct addrdesc *addrdesc;
   int result;

   if ((xioflags&XIO_ACCMODE) == XIO_RDONLY) {
      xfd->tag = XIO_TAG_RDONLY;
   } else if ((xioflags&XIO_ACCMODE) == XIO_WRONLY) {
      xfd->tag = XIO_TAG_WRONLY;
   } else if ((xioflags&XIO_ACCMODE) == XIO_RDWR) {
      xfd->tag = XIO_TAG_RDWR;
   } else {
      Error1("invalid mode for address \"%s\"", xfd->stream.argv[0]);
   }
   xfd->stream.flags     &= (~XIO_ACCMODE);
   xfd->stream.flags     |= (xioflags & XIO_ACCMODE);
   addrdesc = xfd->stream.addr;
   result = (*addrdesc->func)(xfd->stream.argc, xfd->stream.argv,
			      xfd->stream.opts, xioflags, xfd, 
			      addrdesc->groups, addrdesc->arg1,
			      addrdesc->arg2, addrdesc->arg3);
   return result;
}

