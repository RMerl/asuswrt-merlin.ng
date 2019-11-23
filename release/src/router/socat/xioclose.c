/* source: xioclose.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this is the source of the extended close function */


#include "xiosysincludes.h"
#include "xioopen.h"
#include "xiolockfile.h"

#include "xio-termios.h"


/* close the xio fd; must be valid and "simple" (not dual) */
int xioclose1(struct single *pipe) {

   if (pipe->tag == XIO_TAG_INVALID) {
      Notice("xioclose1(): invalid file descriptor");
      errno = EINVAL;
      return -1;
   }

#if WITH_READLINE
   if ((pipe->dtype & XIODATA_MASK) == XIODATA_READLINE) {
      Write_history(pipe->para.readline.history_file);
      /*xiotermios_setflag(pipe->fd, 3, ECHO|ICANON);*/	/* error when pty closed */
   }
#endif /* WITH_READLINE */
#if WITH_OPENSSL
   if ((pipe->dtype & XIODATA_MASK) == XIODATA_OPENSSL) {
      if (pipe->para.openssl.ssl) {
	 /* e.g. on TCP connection refused, we do not yet have this set */
	 sycSSL_shutdown(pipe->para.openssl.ssl);
	 sycSSL_free(pipe->para.openssl.ssl);
	 pipe->para.openssl.ssl = NULL;
      }
      Close(pipe->fd);  pipe->fd = -1;
      if (pipe->para.openssl.ctx) {
	 sycSSL_CTX_free(pipe->para.openssl.ctx);
	 pipe->para.openssl.ctx = NULL;
      }
   } else
#endif /* WITH_OPENSSL */
#if WITH_TERMIOS
   if (pipe->ttyvalid) {
      if (Tcsetattr(pipe->fd, 0, &pipe->savetty) < 0) {
	 Warn2("cannot restore terminal settings on fd %d: %s",
	       pipe->fd, strerror(errno));
      }
   }
#endif /* WITH_TERMIOS */
   if (pipe->fd >= 0) {
      switch (pipe->howtoend) {
      case END_KILL: case END_SHUTDOWN_KILL: case END_CLOSE_KILL:
	 if (pipe->para.exec.pid > 0) {
	    pid_t pid;

	    /* first unregister child pid, so our sigchld handler will not throw an error */
	    pid = pipe->para.exec.pid;
	    pipe->para.exec.pid = 0;
	    if (Kill(pid, SIGTERM) < 0) {
	       Msg2(errno==ESRCH?E_INFO:E_WARN, "kill(%d, SIGTERM): %s",
		    pid, strerror(errno));
	    }
	 }
      default:
	 break;
      }
      switch (pipe->howtoend) {
      case END_CLOSE: case END_CLOSE_KILL:
	 if (Close(pipe->fd) < 0) {
	 Info2("close(%d): %s", pipe->fd, strerror(errno)); } break;
#if _WITH_SOCKET
      case END_SHUTDOWN: case END_SHUTDOWN_KILL:
	 if (Shutdown(pipe->fd, 2) < 0) {
	     Info3("shutdown(%d, %d): %s", pipe->fd, 2, strerror(errno)); }
         break;
#endif /* _WITH_SOCKET */
      case END_NONE: default: break;
      }
   }

   /* unlock */
   if (pipe->havelock) {
      xiounlock(pipe->lock.lockfile);
      pipe->havelock = false;
   }      
   if (pipe->opt_unlink_close && pipe->unlink_close) {
      if (Unlink(pipe->unlink_close) < 0) {
	 Info2("unlink(\"%s\"): %s", pipe->unlink_close, strerror(errno));
      }
      free(pipe->unlink_close);
   }

   pipe->tag = XIO_TAG_INVALID;
   return 0;	/*! */
}


/* close the xio fd */
int xioclose(xiofile_t *file) {
   int result;

   if (file->tag == XIO_TAG_INVALID) {
      Error("xioclose(): invalid file descriptor");
      errno = EINVAL;
      return -1;
   }

   if (file->tag == XIO_TAG_DUAL) {
      result  = xioclose1(file->dual.stream[0]);
      result |= xioclose1(file->dual.stream[1]);
      file->tag = XIO_TAG_INVALID;
   } else {
      result = xioclose1(&file->stream);
   }
   return result;
}

