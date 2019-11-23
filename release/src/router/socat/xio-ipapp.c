/* source: xio-ipapp.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for TCP and UDP related options */

#include "xiosysincludes.h"

#if WITH_TCP || WITH_UDP

#include "xioopen.h"
#include "xio-socket.h"
#include "xio-ip.h"
#include "xio-listen.h"
#include "xio-ip6.h"
#include "xio-ipapp.h"

const struct optdesc opt_sourceport = { "sourceport", "sp",       OPT_SOURCEPORT,  GROUP_IPAPP,     PH_LATE,TYPE_2BYTE,	OFUNC_SPEC };
/*const struct optdesc opt_port = { "port",  NULL,    OPT_PORT,        GROUP_IPAPP, PH_BIND,    TYPE_USHORT,	OFUNC_SPEC };*/
const struct optdesc opt_lowport = { "lowport", NULL, OPT_LOWPORT, GROUP_IPAPP, PH_LATE, TYPE_BOOL, OFUNC_SPEC };

#if WITH_IP4
/* we expect the form "host:port" */
int xioopen_ipapp_connect(int argc, const char *argv[], struct opt *opts,
			   int xioflags, xiofile_t *xxfd,
			   unsigned groups, int socktype, int ipproto,
			   int pf) {
   struct single *xfd = &xxfd->stream;
   struct opt *opts0 = NULL;
   const char *hostname = argv[1], *portname = argv[2];
   bool dofork = false;
   union sockaddr_union us_sa,  *us = &us_sa;
   union sockaddr_union them_sa, *them = &them_sa;
   socklen_t uslen = sizeof(us_sa);
   socklen_t themlen = sizeof(them_sa);
   bool needbind = false;
   bool lowport = false;
   int level;
   int result;

   if (argc != 3) {
      Error2("%s: wrong number of parameters (%d instead of 2)", argv[0], argc-1);
   }

   xfd->howtoend = END_SHUTDOWN;

   if (applyopts_single(xfd, opts, PH_INIT) < 0)  return -1;
   applyopts(-1, opts, PH_INIT);

   retropt_bool(opts, OPT_FORK, &dofork);

   if (_xioopen_ipapp_prepare(opts, &opts0, hostname, portname, &pf, ipproto,
			      xfd->para.socket.ip.res_opts[1],
			      xfd->para.socket.ip.res_opts[0],
			      them, &themlen, us, &uslen, &needbind, &lowport,
			      socktype) != STAT_OK) {
      return STAT_NORETRY;
   }

   if (dofork) {
      xiosetchilddied();	/* set SIGCHLD handler */
   }

   if (xioopts.logopt == 'm') {
      Info("starting connect loop, switching to syslog");
      diag_set('y', xioopts.syslogfac);  xioopts.logopt = 'y';
   } else {
      Info("starting connect loop");
   }

   do {	/* loop over retries and forks */

#if WITH_RETRY
      if (xfd->forever || xfd->retry) {
	 level = E_INFO;
      } else
#endif /* WITH_RETRY */
	 level = E_ERROR;

      result =
	 _xioopen_connect(xfd,
			  needbind?(struct sockaddr *)us:NULL, uslen,
			  (struct sockaddr *)them, themlen,
			  opts, pf, socktype, ipproto, lowport, level);
      switch (result) {
      case STAT_OK: break;
#if WITH_RETRY
      case STAT_RETRYLATER:
      case STAT_RETRYNOW:
	 if (xfd->forever || xfd->retry) {
	    --xfd->retry;
	    if (result == STAT_RETRYLATER) {
	       Nanosleep(&xfd->intervall, NULL);
	    }
	    dropopts(opts, PH_ALL); free(opts); opts = copyopts(opts0, GROUP_ALL);
	    continue;
	 }
	 return STAT_NORETRY;
#endif /* WITH_RETRY */
      default:
	  free(opts0);free(opts);
	 return result;
      }

#if WITH_RETRY
      if (dofork) {
	 pid_t pid;
	 int level = E_ERROR;
	 if (xfd->forever || xfd->retry) {
	    level = E_WARN;	/* most users won't expect a problem here,
				   so Notice is too weak */
	 }
	 while ((pid = xio_fork(false, level)) < 0) {
	    if (xfd->forever || --xfd->retry) {
	       Nanosleep(&xfd->intervall, NULL); continue;
	    }
	  free(opts0);
	    return STAT_RETRYLATER;
	 }

	 if (pid == 0) {	/* child process */
	    xfd->forever = false;  xfd->retry = 0;
	    break;
	 }

	 /* parent process */
	 Close(xfd->fd);
	 /* with and without retry */
	 Nanosleep(&xfd->intervall, NULL);
	 dropopts(opts, PH_ALL); free(opts); opts = copyopts(opts0, GROUP_ALL);
	 continue;	/* with next socket() bind() connect() */
      } else
#endif /* WITH_RETRY */
      {
	 break;
      }
   } while (true);
   /* only "active" process breaks (master without fork, or child) */

   if ((result = _xio_openlate(xfd, opts)) < 0) {
	   free(opts0);free(opts);
      return result;
   }
   free(opts0);free(opts);
   return 0;
}


/* returns STAT_OK on success or some other value on failure
   applies and consumes the following options:
   PH_EARLY
   OPT_PROTOCOL_FAMILY, OPT_BIND, OPT_SOURCEPORT, OPT_LOWPORT
 */
int
   _xioopen_ipapp_prepare(struct opt *opts, struct opt **opts0,
			   const char *hostname,
			   const char *portname,
			   int *pf,
			   int protocol,
			   unsigned long res_opts0, unsigned long res_opts1,
			   union sockaddr_union *them, socklen_t *themlen,
			   union sockaddr_union *us, socklen_t *uslen,
			   bool *needbind, bool *lowport,
			   int socktype) {
   uint16_t port;
   char infobuff[256];
   int result;

   retropt_socket_pf(opts, pf);

   if ((result =
	xiogetaddrinfo(hostname, portname,
		       *pf, socktype, protocol,
		       (union sockaddr_union *)them, themlen,
		       res_opts0, res_opts1
		       ))
       != STAT_OK) {
      return STAT_NORETRY;	/*! STAT_RETRYLATER? */
   }
   if (*pf == PF_UNSPEC) {
      *pf = them->soa.sa_family;
   }

   applyopts(-1, opts, PH_EARLY);

   /* 3 means: IP address AND port accepted */
   if (retropt_bind(opts, *pf, socktype, protocol, (struct sockaddr *)us, uslen, 3,
		    res_opts0, res_opts1)
       != STAT_NOACTION) {
      *needbind = true;
   } else {
      switch (*pf) {
#if WITH_IP4
      case PF_INET:  socket_in_init(&us->ip4);  *uslen = sizeof(us->ip4); break;
#endif /* WITH_IP4 */
#if WITH_IP6
      case PF_INET6: socket_in6_init(&us->ip6); *uslen = sizeof(us->ip6); break;
#endif /* WITH_IP6 */
      }
   }

   if (retropt_2bytes(opts, OPT_SOURCEPORT, &port) >= 0) {
      switch (*pf) {
#if WITH_IP4
      case PF_INET:  us->ip4.sin_port = htons(port); break;
#endif /* WITH_IP4 */
#if WITH_IP6
      case PF_INET6: us->ip6.sin6_port = htons(port); break;
#endif /* WITH_IP6 */
      default: Error("unsupported protocol family");
      }
      *needbind = true;
   }

   retropt_bool(opts, OPT_LOWPORT, lowport);

   *opts0 = copyopts(opts, GROUP_ALL);

   Notice1("opening connection to %s",
	   sockaddr_info((struct sockaddr *)them, *themlen, infobuff, sizeof(infobuff)));
   return STAT_OK;
}
#endif /* WITH_IP4 */


#if WITH_TCP && WITH_LISTEN
/*
   applies and consumes the following options:
   OPT_PROTOCOL_FAMILY, OPT_BIND
 */
int _xioopen_ipapp_listen_prepare(struct opt *opts, struct opt **opts0,
				   const char *portname, int *pf, int ipproto,
				  unsigned long res_opts0,
				  unsigned long res_opts1,
				   union sockaddr_union *us, socklen_t *uslen,
				   int socktype) {
   char *bindname = NULL;
   int result;

   retropt_socket_pf(opts, pf);

   retropt_string(opts, OPT_BIND, &bindname);
   if ((result =
	xiogetaddrinfo(bindname, portname, *pf, socktype, ipproto,
		       (union sockaddr_union *)us, uslen,
		       res_opts0, res_opts1))
       != STAT_OK) {
      /*! STAT_RETRY? */
      return result;
   }

   *opts0 = copyopts(opts, GROUP_ALL);
   return STAT_OK;
}


/* we expect the form: port */
/* currently only used for TCP4 */
int xioopen_ipapp_listen(int argc, const char *argv[], struct opt *opts,
			  int xioflags, xiofile_t *fd,
			 unsigned groups, int socktype,
			 int ipproto, int pf) {
   struct opt *opts0 = NULL;
   union sockaddr_union us_sa, *us = &us_sa;
   socklen_t uslen = sizeof(us_sa);
   int result;

   if (argc != 2) {
      Error2("%s: wrong number of parameters (%d instead of 1)", argv[0], argc-1);
   }

   if (pf == PF_UNSPEC) {
#if WITH_IP4 && WITH_IP6
      pf = xioopts.default_ip=='6'?PF_INET6:PF_INET;
#elif WITH_IP6
      pf = PF_INET6;
#else
      pf = PF_INET;
#endif
   }

   fd->stream.howtoend = END_SHUTDOWN;

   if (applyopts_single(&fd->stream, opts, PH_INIT) < 0)  return -1;
   applyopts(-1, opts, PH_INIT);
   applyopts(-1, opts, PH_EARLY);

   if (_xioopen_ipapp_listen_prepare(opts, &opts0, argv[1], &pf, ipproto,
				     fd->stream.para.socket.ip.res_opts[1],
				     fd->stream.para.socket.ip.res_opts[0],
				     us, &uslen, socktype)
       != STAT_OK) {
      return STAT_NORETRY;
   }

   if ((result =
	xioopen_listen(&fd->stream, xioflags,
		       (struct sockaddr *)us, uslen,
		     opts, opts0, pf, socktype, ipproto))
       != 0)
      return result;
   return 0;
}
#endif /* WITH_IP4 && WITH_TCP && WITH_LISTEN */

#endif /* WITH_TCP || WITH_UDP */
