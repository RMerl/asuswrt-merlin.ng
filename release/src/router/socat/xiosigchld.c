/* source: xiosigchld.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this is the source of the extended child signal handler */


#include "xiosysincludes.h"
#include "xioopen.h"


/*!! with socat, at most 4 exec children exist */
pid_t diedunknown[NUMUNKNOWN];	/* children that died before they were registered */
size_t nextunknown;


/* register for a xio filedescriptor a callback (handler).
   when a SIGCHLD occurs, the signal handler will ??? */
int xiosetsigchild(xiofile_t *xfd, int (*callback)(struct single *)) {
   if (xfd->tag != XIO_TAG_DUAL) {
      xfd->stream.sigchild = callback;
   } else {
      xfd->dual.stream[0]->sigchild = callback;
      xfd->dual.stream[1]->sigchild = callback;
   }
   return 0;
}

/* exec'd child has died, perform appropriate changes to descriptor */
/* is async-signal-safe */
static int sigchld_stream(struct single *file) {
   /*!! call back to application */
   file->para.exec.pid = 0;
   if (file->sigchild) {
      return (*file->sigchild)(file);
   }
   return 0;
}

/* return 0 if socket is not responsible for deadchild */
static int xio_checkchild(xiofile_t *socket, int socknum, pid_t deadchild) {
   int retval;
   if (socket != NULL) {
      if (socket->tag != XIO_TAG_DUAL) {
	 if ((socket->stream.howtoend == END_KILL ||
	      socket->stream.howtoend == END_CLOSE_KILL ||
	      socket->stream.howtoend == END_SHUTDOWN_KILL) &&
	     socket->stream.para.exec.pid == deadchild) {
	    Info2("exec'd process %d on socket %d terminated",
		  socket->stream.para.exec.pid, socknum);
	    sigchld_stream(&socket->stream);	/* is async-signal-safe */
	    return 1;
	 }
      } else {
	 if (retval = xio_checkchild((xiofile_t *)socket->dual.stream[0], socknum, deadchild))
	    return retval;
	 else
	    return xio_checkchild((xiofile_t *)socket->dual.stream[1], socknum, deadchild);
      }
   }
   return 0;
}

/* this is the "physical" signal handler for SIGCHLD */
/* the current socat/xio implementation knows two kinds of children:
   exec/system addresses perform a fork: these children are registered and
   there death influences the parents flow;
   listen-socket with fork children: these children are "anonymous" and their
   death does not affect the parent process (now; maybe we have a child
   process counter later) */
void childdied(int signum) {
   pid_t pid;
   int _errno;
   int status = 0;
   bool wassig = false;
   int i;

   _errno = errno;	/* save current value; e.g., select() on Cygwin seems
			   to set it to EINTR _before_ handling the signal, and
			   then passes the value left by the signal handler to
			   the caller of select(), accept() etc. */
   diag_in_handler = 1;
   Notice1("childdied(): handling signal %d", signum);
   Info1("childdied(signum=%d)", signum);
   do {
      pid = Waitpid(-1, &status, WNOHANG);
      if (pid == 0) {
	 Msg(wassig?E_INFO:E_WARN,
	     "waitpid(-1, {}, WNOHANG): no child has exited");
	 Info("childdied() finished");
	 diag_in_handler = 0;
	 errno = _errno;
	 return;
      } else if (pid < 0 && errno == ECHILD) {
	 Msg(wassig?E_INFO:E_WARN,
	      "waitpid(-1, {}, WNOHANG): "F_strerror);
	 Info("childdied() finished");
	 diag_in_handler = 0;
	 errno = _errno;
	 return;
      }
      wassig = true;
      if (pid < 0) {
	 Warn1("waitpid(-1, {%d}, WNOHANG): "F_strerror, status);
	 Info("childdied() finished");
	 diag_in_handler = 0;
	 errno = _errno;
	 return;
      }
   /*! indent */
   if (num_child) num_child--;
   /* check if it was a registered child process */
   i = 0;
   while (i < XIO_MAXSOCK) {
      if (xio_checkchild(sock[i], i, pid))  break;
      ++i;
   }
   if (i == XIO_MAXSOCK) {
	 Info2("childdied(%d): cannot identify child %d", signum, pid);
	 if (nextunknown == NUMUNKNOWN) {
	    nextunknown = 0;
	 }
	 diedunknown[nextunknown++] = pid;
	 Debug1("saving pid in diedunknown"F_Zu,
		nextunknown/*sic, for compatibility*/);
      }

   if (WIFEXITED(status)) {
      if (WEXITSTATUS(status) == 0) {
	 Info2("waitpid(): child %d exited with status %d",
	       pid, WEXITSTATUS(status));
      } else {
	 if (i == XIO_MAXSOCK) {
	    Info2("waitpid(): child %d exited with status %d",
		   pid, WEXITSTATUS(status));
	 } else {
	    Error2("waitpid(): child %d exited with status %d",
		   pid, WEXITSTATUS(status));
	 }
      }
   } else if (WIFSIGNALED(status)) {
      if (i == XIO_MAXSOCK) {
	 Info2("waitpid(): child %d exited on signal %d",
	       pid, WTERMSIG(status));
      } else {
	 Error2("waitpid(): child %d exited on signal %d",
	       pid, WTERMSIG(status));
      }
   } else if (WIFSTOPPED(status)) {
      Info2("waitpid(): child %d stopped on signal %d",
	    pid, WSTOPSIG(status));
   } else {
      Warn1("waitpid(): cannot determine status of child %d", pid);
   }

#if !HAVE_SIGACTION
   /* we might need to re-register our handler */
   if (Signal(SIGCHLD, childdied) == SIG_ERR) {
      Warn("signal(SIGCHLD, childdied): "F_strerror);
   }
#endif /* !HAVE_SIGACTION */
  } while (1);
   Info("childdied() finished");
   diag_in_handler = 0;
   errno = _errno;
}


int xiosetchilddied(void) {
#if HAVE_SIGACTION
   struct sigaction act;
   memset(&act, 0, sizeof(struct sigaction));
   act.sa_flags   = SA_NOCLDSTOP/*|SA_RESTART*/
#ifdef SA_NOMASK
      |SA_NOMASK
#endif
      ;
   act.sa_handler = childdied;
   sigfillset(&act.sa_mask);
   if (Sigaction(SIGCHLD, &act, NULL) < 0) {
      /*! man does not say that errno is defined */
      Warn2("sigaction(SIGCHLD, %p, NULL): %s", childdied, strerror(errno));
   }
#else /* HAVE_SIGACTION */
   if (Signal(SIGCHLD, childdied) == SIG_ERR) {
      Warn2("signal(SIGCHLD, %p): %s", childdied, strerror(errno));
   }
#endif /* !HAVE_SIGACTION */
   return 0;
}
