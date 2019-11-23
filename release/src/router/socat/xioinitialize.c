/* source: xioinitialize.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for the initialize function */

#include "xiosysincludes.h"

#include "xioopen.h"
#include "xiolockfile.h"

#include "xio-openssl.h"	/* xio_reset_fips_mode() */

static int xioinitialized;
xiofile_t *sock[XIO_MAXSOCK];
int (*xiohook_newchild)(void);	/* xio calls this function from a new child
				   process */
int num_child = 0;

/* returns 0 on success or != if an error occurred */
int xioinitialize(void) {
   if (xioinitialized)  return 0;

   /* configure and .h's cannot guarantee this */
   assert(sizeof(uint8_t)==1);
   assert(sizeof(uint16_t)==2);
   assert(sizeof(uint32_t)==4);

   /* assertions regarding O_ flags - important for XIO_READABLE() etc. */
   assert(O_RDONLY==0);
   assert(O_WRONLY==1);
   assert(O_RDWR==2);

   assert(SHUT_RD==0);
   assert(SHUT_WR==1);
   assert(SHUT_RDWR==2);

   /* some assertions about termios */
#if WITH_TERMIOS
#if defined(CRDLY) && CRDLY_SHIFT >= 0
   assert(3 << opt_crdly.arg3  == CRDLY);
#endif
#if defined(TABDLY) && TABDLY_SHIFT >= 0
   assert(3 << opt_tabdly.arg3 == TABDLY);
#endif
#if CSIZE_SHIFT >= 0
   assert(3 << opt_csize.arg3  == CSIZE);
#endif
   {
      union {
	 struct termios termarg;
	 tcflag_t flags[4];
#if HAVE_TERMIOS_ISPEED
	 speed_t speeds[sizeof(struct termios)/sizeof(speed_t)];
#endif
      } tdata;
      tdata.termarg.c_iflag = 0x12345678;
      tdata.termarg.c_oflag = 0x23456789;
      tdata.termarg.c_cflag = 0x3456789a;
      tdata.termarg.c_lflag = 0x456789ab;
      assert(tdata.termarg.c_iflag == tdata.flags[0]);
      assert(tdata.termarg.c_oflag == tdata.flags[1]);
      assert(tdata.termarg.c_cflag == tdata.flags[2]);
      assert(tdata.termarg.c_lflag == tdata.flags[3]);
#if HAVE_TERMIOS_ISPEED && (ISPEED_OFFSET != -1) && (OSPEED_OFFSET != -1)
#if defined(ISPEED_OFFSET) && (ISPEED_OFFSET != -1)
#if defined(OSPEED_OFFSET) && (OSPEED_OFFSET != -1)
      tdata.termarg.c_ispeed = 0x56789abc;
      tdata.termarg.c_ospeed = 0x6789abcd;
      assert(tdata.termarg.c_ispeed == tdata.speeds[ISPEED_OFFSET]);
      assert(tdata.termarg.c_ospeed == tdata.speeds[OSPEED_OFFSET]);
#endif
#endif
#endif
   }
#endif

   /* these dependencies required in applyopts() for OFUNC_FCNTL */
   assert(F_GETFD == F_SETFD-1);
   assert(F_GETFL == F_SETFL-1);

   {
      const char *default_ip;
      default_ip = getenv("SOCAT_DEFAULT_LISTEN_IP");
      if (default_ip != NULL) {
	 switch (default_ip[0]) {
	 case '4':
	 case '6':
	    xioopts.default_ip = default_ip[0]; break;
	 }
      }
   }
   {
      const char *preferred_ip;
      preferred_ip = getenv("SOCAT_PREFERRED_RESOLVE_IP");
      if (preferred_ip != NULL) {
	 switch (preferred_ip[0]) {
	 case '4':
	 case '6':
	    xioopts.preferred_ip = preferred_ip[0]; break;
	 default:
	    xioopts.preferred_ip = '0'; break;
	 }
      }
   }

   if (Atexit(xioexit) < 0) {
      Error("atexit(xioexit) failed");
      return -1;
   }

   xioinitialized = 1;
   return 0;
}

/* call this function when option -lp (reset program name) has been applied */
int xioinitialize2(void) {
   pid_t pid = Getpid();
   xiosetenvulong("PID", pid, 1);
   xiosetenvulong("PPID", pid, 1);
   return 0;
}


/* well, this function is not for initialization, but I could not find a better
   place for it
   it is called in the child process after fork
   it drops the locks of the xiofile's so only the parent owns them
 */
void xiodroplocks(void) {
   int i;

   for (i = 0; i < XIO_MAXSOCK; ++i) {
      if (sock[i] != NULL && sock[i]->tag != XIO_TAG_INVALID) {
	 xiofiledroplock(sock[i]);
      }
   }
}


/* consider an invokation like this:
   socat -u exec:'some program that accepts data' tcp-l:...,fork
   we do not want the program to be killed by the first tcp-l sub process, it's
   better if it survives all sub processes. Thus, it must not be killed when
   the sub process delivers EOF. Also, a socket that is reused in sub processes
   should not be shut down (affects the connection), but closed (affects only
   sub processes copy of file descriptor) */
static int xio_nokill(xiofile_t *sock) {
   int result = 0;
   switch (sock->tag) {
   case XIO_TAG_INVALID:
   default:
      return -1;
   case XIO_TAG_DUAL:
      if ((result = xio_nokill((xiofile_t *)sock->dual.stream[0])) != 0)
	 return result;
      result = xio_nokill((xiofile_t *)sock->dual.stream[1]);
      break;
   case XIO_TAG_RDONLY:
   case XIO_TAG_WRONLY:
   case XIO_TAG_RDWR:
      /* here is the core of this function */
      switch (sock->stream.howtoend) {
      case END_SHUTDOWN_KILL: sock->stream.howtoend = END_CLOSE; break;
      case END_CLOSE_KILL:    sock->stream.howtoend = END_CLOSE; break;
      case END_SHUTDOWN:      sock->stream.howtoend = END_CLOSE; break;
      default: break;
      }
      break;
   }
   return result;
}

/* call this function immediately after fork() in child process */
/* it performs some neccessary actions
   returns 0 on success or != 0 if an error occurred */
int xio_forked_inchild(void) {
   int result = 0;
   int i;

   for (i=0; i<NUMUNKNOWN; ++i) {
      diedunknown[i] = 0;
   }
   num_child = 0;
   xiodroplocks();
#if WITH_FIPS
   if (xio_reset_fips_mode() != 0) {
      result = 1;
   }
#endif /* WITH_FIPS */
   /* some locks belong to parent process, so "drop" them now */
   if (xiohook_newchild) {
      if ((*xiohook_newchild)() != 0) {
	 Exit(1);
      }
   }

   /* change XIO_SHUTDOWN_KILL to XIO_SHUTDOWN */
   if (sock1 != NULL) {
      int result2;
      result2 = xio_nokill(sock1);
      if (result2 < 0)  Exit(1);
      result |= result2;
   }

   return result;
}

/* subchild != 0 means that the current process is already a child process of
   the master process and thus the new sub child process should not set the
   SOCAT_PID variable */
pid_t xio_fork(bool subchild, int level) {
   pid_t pid;
   const char *forkwaitstring;
   int forkwaitsecs = 0;

   if ((pid = Fork()) < 0) {
      Msg1(level, "fork(): %s", strerror(errno));
      return pid;
   }

   if (pid == 0) {	/* child process */
      pid_t cpid = Getpid();

      Info1("just born: child process "F_pid, cpid);
      if (!subchild) {
	 /* set SOCAT_PID to new value */
	 xiosetenvulong("PID", pid, 1);
      }
      /* gdb recommends to have env controlled sleep after fork */
      if (forkwaitstring = getenv("SOCAT_FORK_WAIT")) {
	 forkwaitsecs = atoi(forkwaitstring);
	 Sleep(forkwaitsecs);
      }
      if (xio_forked_inchild() != 0) {
	 Exit(1);
      }
      return 0;
   }

   num_child++;
   /* parent process */
   Notice1("forked off child process "F_pid, pid);
   /* gdb recommends to have env controlled sleep after fork */
   if (forkwaitstring = getenv("SOCAT_FORK_WAIT")) {
      forkwaitsecs = atoi(forkwaitstring);
      Sleep(forkwaitsecs);
   }
   return pid;
}
