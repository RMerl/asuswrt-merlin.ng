/* source: xioshutdown.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this is the source of the extended shutdown function */


#include "xiosysincludes.h"
#include "xioopen.h"

static pid_t socat_kill_pid;	/* here we pass the pid to be killed in sighandler */

static void signal_kill_pid(int dummy) {
   int _errno;
   _errno = errno;
   diag_in_handler = 1;
   Notice("SIGALRM while waiting for wo child process to die, killing it now");
   Kill(socat_kill_pid, SIGTERM);
   diag_in_handler = 0;
   errno = _errno;
}

int xioshutdown(xiofile_t *sock, int how) {
   int result = 0;

   if (sock->tag == XIO_TAG_INVALID) {
      Error("xioshutdown(): invalid file descriptor");
      errno = EINVAL;
      return -1;
   }

   if (sock->tag == XIO_TAG_DUAL) {
      if ((how+1)&1) {
	 result = xioshutdown((xiofile_t *)sock->dual.stream[0], 0);
      }
      if ((how+1)&2) {
	 result |= xioshutdown((xiofile_t *)sock->dual.stream[1], 1);
      }
      return result;
   }

   switch (sock->stream.howtoshut) {
      char writenull;
   case XIOSHUT_NONE:
      return 0;
   case XIOSHUT_CLOSE:
      if (Close(sock->stream.fd) < 0) {
	 Info2("close(%d): %s",
	       sock->stream.fd, strerror(errno));
      }
      return 0;
   case XIOSHUT_DOWN:
      if ((result = Shutdown(sock->stream.fd, how)) < 0) {
	 Info3("shutdown(%d, %d): %s",
	       sock->stream.fd, how, strerror(errno));
      }
      return 0;
#if _WITH_SOCKET
   case XIOSHUT_NULL:
      /* send an empty packet; only useful on datagram sockets? */
      xiowrite(sock, &writenull, 0);
      return 0;
#endif /* _WITH_SOCKET */
   default: ;
   }

   if (false) {
      ;
#if WITH_OPENSSL
   } else if ((sock->stream.dtype & XIODATA_MASK) == XIODATA_OPENSSL) {
      sycSSL_shutdown (sock->stream.para.openssl.ssl);
      /*! what about half/full close? */
#endif /* WITH_OPENSSL */

   } else if ((sock->stream.dtype & XIODATA_MASK) == XIODATA_PIPE) {
      if ((how+1)&1) {
	 if (Close(sock->stream.fd) < 0) {
	    Info2("close(%d): %s",
		  sock->stream.fd, strerror(errno));
	 }
      }
      if ((how+1)&2) {
	 if (Close(sock->stream.para.bipipe.fdout) < 0) {
	    Info2("close(%d): %s",
		  sock->stream.para.bipipe.fdout, strerror(errno));
	 } 
      }
      
   } else if ((sock->stream.dtype & XIODATA_MASK) == XIODATA_2PIPE) {
      if ((how+1)&1) {
	 if (Close(sock->stream.fd) < 0) {
	    Info2("close(%d): %s",
		  sock->stream.fd, strerror(errno));
	 }
      }
      if ((how+1)&2) {
	 if (Close(sock->stream.para.exec.fdout) < 0) {
	    Info2("close(%d): %s",
		  sock->stream.para.exec.fdout, strerror(errno));
	 } 
      }
#if _WITH_SOCKET
   } else if (sock->stream.howtoend == END_SHUTDOWN) {
      if ((result = Shutdown(sock->stream.fd, how)) < 0) {
	 Info3("shutdown(%d, %d): %s",
	       sock->stream.fd, how, strerror(errno));
      }
   } else if (sock->stream.howtoend == END_SHUTDOWN_KILL) {
      if ((result = Shutdown(sock->stream.fd, how)) < 0) {
	 Info3("shutdown(%d, %d): %s",
	       sock->stream.fd, how, strerror(errno));
      }
      if ((sock->stream.flags&XIO_ACCMODE) == XIO_WRONLY) {
	 /* the child process might want to flush some data before terminating
	    */
	 int status = 0;

	 /* we wait for the child process to die, but to prevent timeout
	    we raise an alarm after some time.
	    NOTE: the alarm does not terminate waitpid() on Linux/glibc (BUG?),
	    therefore we have to do the kill in the signal handler */
	 {
	    struct sigaction act;
	    sigfillset(&act.sa_mask);
	    act.sa_flags = 0;
	    act.sa_handler = signal_kill_pid;
	    Sigaction(SIGALRM, &act, NULL);
	 }
	 socat_kill_pid = sock->stream.para.exec.pid;
#if HAVE_SETITIMER
	 /*! with next feature release, we get usec resolution and an option */
#else
	 Alarm(1 /*! sock->stream.para.exec.waitdie */);
#endif /* !HAVE_SETITIMER */
	 if (Waitpid(sock->stream.para.exec.pid, &status, 0) < 0) {
	    Warn3("waitpid("F_pid", %p, 0): %s",
		  sock->stream.para.exec.pid, &status, strerror(errno));
	 }
	 Alarm(0);
      }
   } else if ((sock->stream.dtype & XIODATA_MASK) ==
	      (XIODATA_RECVFROM & XIODATA_MASK)) {
      if (how >= 1) {
	 if (Close(sock->stream.fd) < 0) {
	    Info2("close(%d): %s",
		  sock->stream.fd, strerror(errno));
	 }
	 sock->stream.eof = 2;
	 sock->stream.fd = -1;
      }
#endif /* _WITH_SOCKET */
#if 0
   } else {
      Error1("xioshutdown(): bad data type specification %d", sock->stream.dtype);
      return -1;
#endif

   }
#if 0
   else if (sock->stream.howtoend == END_CLOSE &&
	      sock->stream.dtype == DATA_STREAM) {
      return result;
   }
#if WITH_TERMIOS
   if (sock->stream.ttyvalid) {
      if (Tcsetattr(sock->stream.fd, 0, &sock->stream.savetty) < 0) {
	 Warn2("cannot restore terminal settings on fd %d: %s",
	       sock->stream.fd, strerror(errno));
      }
   }
#endif /* WITH_TERMIOS */
#endif

   return result;
}
