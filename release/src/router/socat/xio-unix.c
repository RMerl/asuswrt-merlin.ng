/* source: xio-unix.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for opening addresses of UNIX socket type */

#include "xiosysincludes.h"
#include "xioopen.h"

#include "xio-socket.h"
#include "xio-listen.h"
#include "xio-unix.h"
#include "xio-named.h"


#if WITH_UNIX

/* to avoid unneccessary "live" if () conditionals when no abstract support is
   compiled in (or at least to give optimizing compilers a good chance) we need
   a constant that can be used in C expressions */ 
#if WITH_ABSTRACT_UNIXSOCKET
#  define ABSTRACT 1
#else
#  define ABSTRACT 0
#endif

static int xioopen_unix_connect(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *xxfd, unsigned groups, int abstract, int dummy2, int dummy3);
static int xioopen_unix_listen(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *xxfd, unsigned groups, int abstract, int dummy2, int dummy3);
static int xioopen_unix_sendto(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *xxfd, unsigned groups, int abstract, int dummy2, int dummy3);
static int xioopen_unix_recvfrom(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *xxfd, unsigned groups, int abstract, int dummy2, int dummy3);
static
int xioopen_unix_recv(int argc, const char *argv[], struct opt *opts,
		      int xioflags, xiofile_t *xxfd, unsigned groups,
		      int abstract, int dummy2, int dummy3);
static
int xioopen_unix_client(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *xxfd, unsigned groups, int abstract, int dummy2, int dummy3);

/* the first free parameter is 0 for "normal" unix domain sockets, or 1 for
   abstract unix sockets (Linux); the second and third free parameter are
   unsused */
const struct addrdesc xioaddr_unix_connect = { "unix-connect",  3, xioopen_unix_connect,  GROUP_FD|GROUP_NAMED|GROUP_SOCKET|GROUP_SOCK_UNIX|GROUP_RETRY,                          0, 0, 0 HELP(":<filename>") };
#if WITH_LISTEN
const struct addrdesc xioaddr_unix_listen  = { "unix-listen",   3, xioopen_unix_listen,   GROUP_FD|GROUP_NAMED|GROUP_SOCKET|GROUP_SOCK_UNIX|GROUP_LISTEN|GROUP_CHILD|GROUP_RETRY, 0, 0, 0 HELP(":<filename>") };
#endif /* WITH_LISTEN */
const struct addrdesc xioaddr_unix_sendto  = { "unix-sendto",   3, xioopen_unix_sendto,   GROUP_FD|GROUP_NAMED|GROUP_SOCKET|GROUP_SOCK_UNIX|GROUP_RETRY,                          0, 0, 0 HELP(":<filename>") };
const struct addrdesc xioaddr_unix_recvfrom= { "unix-recvfrom", 3, xioopen_unix_recvfrom, GROUP_FD|GROUP_NAMED|GROUP_SOCKET|GROUP_SOCK_UNIX|GROUP_RETRY|GROUP_CHILD,              0, 0, 0 HELP(":<filename>") };
const struct addrdesc xioaddr_unix_recv    = { "unix-recv",     1, xioopen_unix_recv,     GROUP_FD|GROUP_NAMED|GROUP_SOCKET|GROUP_SOCK_UNIX|GROUP_RETRY,                          0, 0, 0 HELP(":<filename>") };
const struct addrdesc xioaddr_unix_client  = { "unix-client",   3, xioopen_unix_client,   GROUP_FD|GROUP_NAMED|GROUP_SOCKET|GROUP_SOCK_UNIX|GROUP_RETRY,                          0, 0, 0 HELP(":<filename>") };
#if WITH_ABSTRACT_UNIXSOCKET
const struct addrdesc xioaddr_abstract_connect = { "abstract-connect",  3, xioopen_unix_connect,  GROUP_FD|GROUP_SOCKET|GROUP_SOCK_UNIX|GROUP_RETRY,                          1, 0, 0 HELP(":<filename>") };
#if WITH_LISTEN
const struct addrdesc xioaddr_abstract_listen  = { "abstract-listen",   3, xioopen_unix_listen,   GROUP_FD|GROUP_SOCKET|GROUP_SOCK_UNIX|GROUP_LISTEN|GROUP_CHILD|GROUP_RETRY, 1, 0, 0 HELP(":<filename>") };
#endif /* WITH_LISTEN */
const struct addrdesc xioaddr_abstract_sendto  = { "abstract-sendto",   3, xioopen_unix_sendto,   GROUP_FD|GROUP_SOCKET|GROUP_SOCK_UNIX|GROUP_RETRY,                          1, 0, 0 HELP(":<filename>") };
const struct addrdesc xioaddr_abstract_recvfrom= { "abstract-recvfrom", 3, xioopen_unix_recvfrom, GROUP_FD|GROUP_SOCKET|GROUP_SOCK_UNIX|GROUP_RETRY|GROUP_CHILD,              1, 0, 0 HELP(":<filename>") };
const struct addrdesc xioaddr_abstract_recv    = { "abstract-recv",     1, xioopen_unix_recv,     GROUP_FD|GROUP_SOCKET|GROUP_SOCK_UNIX|GROUP_RETRY,                          1, 0, 0 HELP(":<filename>") };
const struct addrdesc xioaddr_abstract_client  = { "abstract-client",   3, xioopen_unix_client,   GROUP_FD|GROUP_SOCKET|GROUP_SOCK_UNIX|GROUP_RETRY,                          1, 0, 0 HELP(":<filename>") };
#endif /* WITH_ABSTRACT_UNIXSOCKET */

const struct optdesc xioopt_unix_tightsocklen = { "unix-tightsocklen",    "tightsocklen",  OPT_UNIX_TIGHTSOCKLEN,  GROUP_SOCK_UNIX, PH_PREBIND, TYPE_BOOL, OFUNC_OFFSET, XIO_OFFSETOF(para.socket.un.tight), XIO_SIZEOF(para.socket.un.tight) };


/* fills the socket address struct and returns its effective length.
   abstract is usually 0;  != 0 generates an abstract socket address on Linux.
   tight!=0 calculates the resulting length from the path length, not from the
   structures length; this is more common (see option unix-tightsocklen)
   the struct need not be initialized when calling this function.
*/
socklen_t
xiosetunix(int pf,
	   struct sockaddr_un *saun,
	   const char *path,
	   bool abstract,
	   bool tight) {
   size_t pathlen;
   socklen_t len;

   socket_un_init(saun);
#ifdef WITH_ABSTRACT_UNIXSOCKET
   if (abstract) {
      if ((pathlen = strlen(path)) >= sizeof(saun->sun_path)) {
	 Warn2("socket address "F_Zu" characters long, truncating to "F_Zu"",
	       pathlen+1, sizeof(saun->sun_path));
      }
      saun->sun_path[0] = '\0';	/* so it's abstract */
      strncpy(saun->sun_path+1, path, sizeof(saun->sun_path)-1);	/* ok */
      if (tight) {
	 len = sizeof(struct sockaddr_un)-sizeof(saun->sun_path)+
	    MIN(pathlen+1, sizeof(saun->sun_path));
#if HAVE_STRUCT_SOCKADDR_SALEN
	 saun->sun_len = len;
#endif
      } else {
	 len = sizeof(struct sockaddr_un);
      }
      return len;
   }
#endif /* WITH_ABSTRACT_UNIXSOCKET */

   if ((pathlen = strlen(path)) > sizeof(saun->sun_path)) {
      Warn2("unix socket address "F_Zu" characters long, truncating to "F_Zu"",
	    pathlen, sizeof(saun->sun_path));
   }
   strncpy(saun->sun_path, path, sizeof(saun->sun_path));	/* ok */
   if (tight) {
      len = sizeof(struct sockaddr_un)-sizeof(saun->sun_path)+
	 MIN(pathlen, sizeof(saun->sun_path));
#if HAVE_STRUCT_SOCKADDR_SALEN
      saun->sun_len = len;
#endif
   } else {
      len = sizeof(struct sockaddr_un);
   }
   return len;
}

#if WITH_LISTEN
static int xioopen_unix_listen(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *xxfd, unsigned groups, int abstract, int dummy2, int dummy3) {
   /* we expect the form: filename */
   const char *name;
   xiosingle_t *xfd = &xxfd->stream;
   int pf = PF_UNIX;
   int socktype = SOCK_STREAM;
   int protocol = 0;
   struct sockaddr_un us;
   socklen_t uslen;
   struct opt *opts0 = NULL;
   pid_t pid = Getpid();
   bool opt_unlink_early = false;
   bool opt_unlink_close = true;
   int result;

   if (argc != 2) {
      Error2("%s: wrong number of parameters (%d instead of 1)",
	     argv[0], argc-1);
      return STAT_NORETRY;
   }
   name = argv[1];

   xfd->para.socket.un.tight = true;
   retropt_socket_pf(opts, &pf);
   xfd->howtoend = END_SHUTDOWN;

   if (!(ABSTRACT && abstract)) {
      /* only for non abstract because abstract do not work in file system */
      retropt_bool(opts, OPT_UNLINK_EARLY, &opt_unlink_early);
      retropt_bool(opts, OPT_UNLINK_CLOSE, &opt_unlink_close);
   }

   if (applyopts_single(xfd, opts, PH_INIT) < 0) return STAT_NORETRY;
   applyopts(-1, opts, PH_INIT);
   applyopts_named(name, opts, PH_EARLY);	/* umask! */
   applyopts_offset(xfd, opts);
   applyopts(-1, opts, PH_EARLY);

   uslen = xiosetunix(pf, &us, name, abstract, xfd->para.socket.un.tight);

   if (!(ABSTRACT && abstract)) {
      if (opt_unlink_early) {
	 if (Unlink(name) < 0) {
	    if (errno == ENOENT) {
	       Warn2("unlink(\"%s\"): %s", name, strerror(errno));
	    } else {
	       Error2("unlink(\"%s\"): %s", name, strerror(errno));
	    }
	 }
      } else {
	 struct stat buf;
	 if (Lstat(name, &buf) == 0) {
	    Error1("\"%s\" exists", name);
	    return STAT_RETRYLATER;
	 }
      }
      if (opt_unlink_close) {
	 if ((xfd->unlink_close = strdup(name)) == NULL) {
	    Error1("strdup(\"%s\"): out of memory", name);
	 }
	 xfd->opt_unlink_close = true;
      }

      /* trying to set user-early, perm-early etc. here is useless because
	 file system entry is available only past bind() call. */
   }

   opts0 = copyopts(opts, GROUP_ALL);

   /* this may fork() */
   if ((result =
	xioopen_listen(xfd, xioflags,
		       (struct sockaddr *)&us, uslen,
		       opts, opts0, pf, socktype, protocol))
       != 0)
      return result;

   if (!(ABSTRACT && abstract)) {
      if (opt_unlink_close) {
	 if (pid != Getpid()) {
	    /* in a child process - do not unlink-close here! */
	    xfd->opt_unlink_close = false;
	 }
      }
   }

   return 0;
}
#endif /* WITH_LISTEN */


static int xioopen_unix_connect(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *xxfd, unsigned groups, int abstract, int dummy2, int dummy3) {
   /* we expect the form: filename */
   const char *name;
   struct single *xfd = &xxfd->stream;
   int pf = PF_UNIX;
   int socktype = SOCK_STREAM;
   int protocol = 0;
   struct sockaddr_un them, us;
   socklen_t themlen, uslen = sizeof(us);
   bool needbind = false;
   bool opt_unlink_close = false;
   int result;

   if (argc != 2) {
      Error2("%s: wrong number of parameters (%d instead of 1)",
	     argv[0], argc-1);
      return STAT_NORETRY;
   }
   name = argv[1];

   xfd->para.socket.un.tight = true;
   retropt_socket_pf(opts, &pf);
   xfd->howtoend = END_SHUTDOWN;
   if (applyopts_single(xfd, opts, PH_INIT) < 0)  return STAT_NORETRY;
   applyopts(-1, opts, PH_INIT);
   applyopts_offset(xfd, opts);
   applyopts(-1, opts, PH_EARLY);

   themlen = xiosetunix(pf, &them, name, abstract, xfd->para.socket.un.tight);
   if (!(ABSTRACT && abstract)) {
      /* only for non abstract because abstract do not work in file system */
      retropt_bool(opts, OPT_UNLINK_CLOSE, &opt_unlink_close);
   }
   if (retropt_bind(opts, pf, socktype, protocol, (struct sockaddr *)&us, &uslen,
		    (abstract<<1)|xfd->para.socket.un.tight, 0, 0) == STAT_OK) {
      needbind = true;
   }

   if (opt_unlink_close) {
      if ((xfd->unlink_close = strdup(name)) == NULL) {
	 Error1("strdup(\"%s\"): out of memory", name);
      }
      xfd->opt_unlink_close = true;
   }

   if ((result =
	xioopen_connect(xfd,
			needbind?(struct sockaddr *)&us:NULL, uslen,
			(struct sockaddr *)&them, themlen,
			opts, pf, socktype, protocol, false)) != 0) {
      return result;
   }
   if ((result = _xio_openlate(xfd, opts)) < 0) {
      return result;
   }
   return STAT_OK;
}


static int xioopen_unix_sendto(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *xxfd, unsigned groups, int abstract, int dummy, int dummy3) {
   /* we expect the form: filename */
   const char *name;
   xiosingle_t *xfd = &xxfd->stream;
   int pf = PF_UNIX;
   int socktype = SOCK_DGRAM;
   int protocol = 0;
   union sockaddr_union us;
   socklen_t uslen = sizeof(us);
   bool needbind = false;
   bool opt_unlink_close = false;

   if (argc != 2) {
      Error2("%s: wrong number of parameters (%d instead of 1)",
	     argv[0], argc-1);
      return STAT_NORETRY;
   }
   name = argv[1];

   xfd->para.socket.un.tight = true;
   retropt_socket_pf(opts, &pf);
   xfd->howtoend = END_SHUTDOWN;
   applyopts_offset(xfd, opts);

   xfd->salen = xiosetunix(pf, &xfd->peersa.un, name, abstract, xfd->para.socket.un.tight);

   if (!(ABSTRACT && abstract)) {
      /* only for non abstract because abstract do not work in file system */
      retropt_bool(opts, OPT_UNLINK_CLOSE, &opt_unlink_close);
   }

   xfd->dtype = XIODATA_RECVFROM;

   if (retropt_bind(opts, pf, socktype, protocol, &us.soa, &uslen,
		   (abstract<<1)| xfd->para.socket.un.tight, 0, 0) == STAT_OK) {
      needbind = true;
   }

   if (opt_unlink_close) {
      if ((xfd->unlink_close = strdup(name)) == NULL) {
	 Error1("strdup(\"%s\"): out of memory", name);
      }
      xfd->opt_unlink_close = true;
   }

   if (applyopts_single(xfd, opts, PH_INIT) < 0)  return -1;
   applyopts(-1, opts, PH_INIT);

   return
      _xioopen_dgram_sendto(needbind?&us:NULL, uslen,
			    opts, xioflags, xfd, groups,
			    pf, socktype, protocol);
}


static
int xioopen_unix_recvfrom(int argc, const char *argv[], struct opt *opts,
		     int xioflags, xiofile_t *xxfd, unsigned groups,
		     int abstract, int dummy2, int dummy3) {
   /* we expect the form: filename */
   const char *name;
   xiosingle_t *xfd = &xxfd->stream;
   int pf = PF_UNIX;
   int socktype = SOCK_DGRAM;
   int protocol = 0;
   struct sockaddr_un us;
   socklen_t uslen;
   bool needbind = true;
   bool opt_unlink_early = false;
   bool opt_unlink_close = true;

   if (argc != 2) {
      Error2("%s: wrong number of parameters (%d instead of 1)",
	     argv[0], argc-1);
      return STAT_NORETRY;
   }
   name = argv[1];

   xfd->para.socket.un.tight = true;
   retropt_socket_pf(opts, &pf);
   xfd->howtoend = END_NONE;
   if (applyopts_single(xfd, opts, PH_INIT) < 0)  return STAT_NORETRY;
   applyopts(-1, opts, PH_INIT);
   applyopts_named(name, opts, PH_EARLY);       /* umask! */
   applyopts_offset(xfd, opts);

   if (!(ABSTRACT && abstract)) {
      /* only for non abstract because abstract do not work in file system */
      retropt_bool(opts, OPT_UNLINK_EARLY, &opt_unlink_early);
      retropt_bool(opts, OPT_UNLINK_CLOSE, &opt_unlink_close);
   }
   applyopts(-1, opts, PH_EARLY);

   uslen = xiosetunix(pf, &us, name, abstract, xfd->para.socket.un.tight);

#if 0
   if (retropt_bind(opts, pf, socktype, protocol, (struct sockaddr *)&us, &uslen,
		    (abstract<<1)|xfd->para.socket.un.tight, 0, 0) == STAT_OK) {
   }
#endif

   if (!(ABSTRACT && abstract)) {
      if (opt_unlink_early) {
	 if (Unlink(name) < 0) {
	    if (errno == ENOENT) {
	       Warn2("unlink(\"%s\"): %s", name, strerror(errno));
	    } else {
	       Error2("unlink(\"%s\"): %s", name, strerror(errno));
	    }
	 }
      } else {
	 struct stat buf;
	 if (Lstat(name, &buf) == 0) {
	    Error1("\"%s\" exists", name);
	    return STAT_RETRYLATER;
	 }
      }
      if (opt_unlink_close) {
	 if ((xfd->unlink_close = strdup(name)) == NULL) {
	    Error1("strdup(\"%s\"): out of memory", name);
	 }
	 xfd->opt_unlink_close = true;
      }

      /* trying to set user-early, perm-early etc. here is useless because
	 file system entry is available only past bind() call. */
   }
   applyopts_named(name, opts, PH_EARLY);	/* umask! */

   xfd->para.socket.la.soa.sa_family = pf;

   xfd->dtype = XIODATA_RECVFROM_ONE;

   /* this may fork */
   return
      _xioopen_dgram_recvfrom(xfd, xioflags,
			      needbind?(struct sockaddr *)&us:NULL, uslen,
			      opts, pf, socktype, protocol, E_ERROR);
}


static
int xioopen_unix_recv(int argc, const char *argv[], struct opt *opts,
		      int xioflags, xiofile_t *xxfd, unsigned groups,
		      int abstract, int dummy2, int dummy3) {
   /* we expect the form: filename */
   const char *name;
   xiosingle_t *xfd = &xxfd->stream;
   int pf = PF_UNIX;
   int socktype = SOCK_DGRAM;
   int protocol = 0;
   union sockaddr_union us;
   socklen_t uslen;
   bool opt_unlink_early = false;
   bool opt_unlink_close = true;
   int result;

   if (argc != 2) {
      Error2("%s: wrong number of parameters (%d instead of 1)",
	     argv[0], argc-1);
      return STAT_NORETRY;
   }
   name = argv[1];

   xfd->para.socket.un.tight = true;
   retropt_socket_pf(opts, &pf);
   xfd->howtoend = END_SHUTDOWN;
   if (applyopts_single(xfd, opts, PH_INIT) < 0)  return STAT_NORETRY;
   applyopts(-1, opts, PH_INIT);
   applyopts_named(name, opts, PH_EARLY);       /* umask! */
   applyopts_offset(xfd, opts);

   if (!(ABSTRACT && abstract)) {
      /* only for non abstract because abstract do not work in file system */
      retropt_bool(opts, OPT_UNLINK_EARLY, &opt_unlink_early);
      retropt_bool(opts, OPT_UNLINK_CLOSE, &opt_unlink_close);
   }
   applyopts(-1, opts, PH_EARLY);

   uslen = xiosetunix(pf, &us.un, name, abstract, xfd->para.socket.un.tight);

#if 0
   if (retropt_bind(opts, pf, socktype, protocol, &us.soa, &uslen,
		    (abstract<<1)|xfd->para.socket.un.tight, 0, 0)
       == STAT_OK) {
   }
#endif

   if (!(ABSTRACT && abstract)) {
      if (opt_unlink_early) {
	 if (Unlink(name) < 0) {
	    if (errno == ENOENT) {
	       Warn2("unlink(\"%s\"): %s", name, strerror(errno));
	    } else {
	       Error2("unlink(\"%s\"): %s", name, strerror(errno));
	    }
	 }
      } else {
	 struct stat buf;
	 if (Lstat(name, &buf) == 0) {
	    Error1("\"%s\" exists", name);
	    return STAT_RETRYLATER;
	 }
      }
      if (opt_unlink_close) {
	 if ((xfd->unlink_close = strdup(name)) == NULL) {
	    Error1("strdup(\"%s\"): out of memory", name);
	 }
	 xfd->opt_unlink_close = true;
      }
   }
   applyopts_named(name, opts, PH_EARLY);	/* umask! */

   xfd->para.socket.la.soa.sa_family = pf;

   xfd->dtype = XIODATA_RECV;
   result = _xioopen_dgram_recv(xfd, xioflags, &us.soa, uslen,
				opts, pf, socktype, protocol, E_ERROR);
   return result;
}


static int xioopen_unix_client(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *xxfd, unsigned groups, int abstract, int dummy2, int dummy3) {
   /* we expect the form: filename */
   if (argc != 2) {
      Error2("%s: wrong number of parameters (%d instead of 1)", argv[0], argc-1);
   }

   return 
      _xioopen_unix_client(&xxfd->stream, xioflags, groups, abstract, opts,
			   argv[1]);
}

/* establishes communication with an existing UNIX type socket. supports stream
   and datagram socket types: first tries to connect(), but when this fails it
   falls back to sendto().
   applies and consumes the following option:
   PH_INIT, PH_PASTSOCKET, PH_FD, PH_PREBIND, PH_BIND, PH_PASTBIND,
   PH_CONNECTED, PH_LATE, ?PH_CONNECT
   OFUNC_OFFSET, 
   OPT_PROTOCOL_FAMILY, OPT_UNIX_TIGHTSOCKLEN, OPT_UNLINK_CLOSE, OPT_BIND,
   OPT_SO_TYPE, OPT_SO_PROTOTYPE, OPT_CLOEXEC, OPT_USER, OPT_GROUP, ?OPT_FORK, 
*/
int 
_xioopen_unix_client(xiosingle_t *xfd, int xioflags, unsigned groups,
		     int abstract, struct opt *opts, const char *name) {
   int pf = PF_UNIX;
   int socktype = 0;	/* to be determined by server socket type */
   int protocol = 0;
   union sockaddr_union them, us;
   socklen_t themlen, uslen = sizeof(us);
   bool needbind = false;
   bool opt_unlink_close = false;
   struct opt *opts0;
   int result;

   xfd->para.socket.un.tight = true;
   retropt_socket_pf(opts, &pf);
   xfd->howtoend = END_SHUTDOWN;
   if (applyopts_single(xfd, opts, PH_INIT) < 0)  return STAT_NORETRY;
   applyopts(-1, opts, PH_INIT);
   applyopts_offset(xfd, opts);
   applyopts(-1, opts, PH_EARLY);

   themlen = xiosetunix(pf, &them.un, name, abstract, xfd->para.socket.un.tight);
   if (!(ABSTRACT && abstract)) {
      /* only for non abstract because abstract do not work in file system */
      retropt_bool(opts, OPT_UNLINK_CLOSE, &opt_unlink_close);
   }
   if (retropt_bind(opts, pf, socktype, protocol, &us.soa, &uslen,
		    (abstract<<1)|xfd->para.socket.un.tight, 0, 0)
       != STAT_NOACTION) {
      needbind = true;
   }

   if (opt_unlink_close) {
      if ((xfd->unlink_close = strdup(name)) == NULL) {
	 Error1("strdup(\"%s\"): out of memory", name);
      }
      xfd->opt_unlink_close = true;
   }

   /* save options, because we might have to start again */
   opts0 = copyopts(opts, GROUP_ALL);

   /* xfd->dtype = DATA_STREAM; // is default */
   if ((result =
	xioopen_connect(xfd,
			needbind?(struct sockaddr *)&us:NULL, uslen,
			(struct sockaddr *)&them, themlen,
			opts, pf, socktype?socktype:SOCK_STREAM, protocol,
			false)) != 0) {
      if (errno == EPROTOTYPE) {
	 if (needbind) {
	    Unlink(us.un.sun_path);
	 }

	 dropopts2(opts, PH_INIT, PH_SPEC); opts = opts0;

	 xfd->peersa = them;
	 xfd->salen = sizeof(struct sockaddr_un);
	 if ((result =
	      _xioopen_dgram_sendto(needbind?&us:NULL, uslen,
				    opts, xioflags, xfd, groups,
				    pf, socktype?socktype:SOCK_DGRAM, protocol))
	     != 0) {
	    return result;
	 }
	 xfd->dtype = XIODATA_RECVFROM;
      }
   }
   if ((result = _xio_openlate(xfd, opts)) < 0) {
      return result;
   }
   return 0;
}


/* returns information that can be used for constructing an environment
   variable describing the socket address.
   if idx is 0, this function writes "ADDR" into namebuff and the path into
   valuebuff, and returns 0 (which means that no more info is there).
   if idx is != 0, it returns -1
   namelen and valuelen contain the max. allowed length of output chars in the
   respective buffer.
   on error this function returns -1.
*/
int
xiosetsockaddrenv_unix(int idx, char *namebuff, size_t namelen,
		       char *valuebuff, size_t valuelen,
		       struct sockaddr_un *sa, socklen_t salen, int ipproto) {
   if (idx != 0) {
      return -1;
   }
   strcpy(namebuff, "ADDR");
   sockaddr_unix_info(sa, salen, valuebuff, valuelen);
   return 0;
}

#endif /* WITH_UNIX */
