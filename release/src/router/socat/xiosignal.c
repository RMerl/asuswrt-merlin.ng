/* source: xiosignal.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains code for handling signals (except SIGCHLD) */

#include "config.h"
#include "xioconfig.h"  /* what features are enabled */

#include "sysincludes.h"

#include "mytypes.h"
#include "compat.h"
#include "error.h"

#include "sycls.h"


#define SOCAT_MAXPIDS 4

struct socat_sig_desc {
   int sig_use;
   pid_t  sig_pids[SOCAT_MAXPIDS];
} ;

#if 0
size_t socat_sigint_use;	/* how many pids are set in following array */
static pid_t socat_sigint_pids[SOCAT_MAXPIDS];
size_t socat_sigquit_use;	/* how many pids are set in following array */
static pid_t socat_sigquit_pids[SOCAT_MAXPIDS];
#else
static struct socat_sig_desc socat_sighup;
static struct socat_sig_desc socat_sigint;
static struct socat_sig_desc socat_sigquit;
#endif


/* is async-signal-safe */
static struct socat_sig_desc *socat_get_sig_desc(int signum) {
   struct socat_sig_desc *sigdesc;
   switch (signum) {
   case SIGHUP:  sigdesc = &socat_sighup;  break;
   case SIGINT:  sigdesc = &socat_sigint;  break;
   case SIGQUIT: sigdesc = &socat_sigquit; break;
   default: sigdesc = NULL; break;
   }
   return sigdesc;
}

/* a signal handler that possibly passes the signal to sub processes */
void socatsignalpass(int sig) {
   int i;
   struct socat_sig_desc *sigdesc;
   int _errno;

   _errno = errno;
   diag_in_handler = 1;
   Notice1("socatsignalpass(%d)", sig);
   if ((sigdesc = socat_get_sig_desc(sig)) == NULL) {	/* is async-signal-safe */
      diag_in_handler = 0;
      errno = _errno;
      return;
   }

   for (i=0; i<sigdesc->sig_use; ++i) {
      if (sigdesc->sig_pids[i]) {
	 if (Kill(sigdesc->sig_pids[i], sig) < 0) {
	    Warn2("kill("F_pid", %d): %m",
		  sigdesc->sig_pids[i], sig);
	 }
      }
   }
#if !HAVE_SIGACTION
   Signal(sig, socatsignalpass);
#endif /* !HAVE_SIGACTION */
   Debug("socatsignalpass() ->");
   diag_in_handler = 0;
   errno = _errno;
}


/* register the sub process pid for passing of signals of type signum. 
   Only for SIGHUP, SIGINT, and SIGQUIT!
   returns 0 on success or <0 if an error occurred */
int xio_opt_signal(pid_t pid, int signum) {
   struct socat_sig_desc *sigdesc;

   if ((sigdesc = socat_get_sig_desc(signum)) == NULL) {
      Error("sub process registered for unsupported signal");
      return -1;
   }

   if (sigdesc->sig_use >= SOCAT_MAXPIDS) {
      Error1("too many sub processes registered for signal %d", signum);
      return -1;
   }
   if (sigdesc->sig_use == 0) {
      /* the special signal handler has not been registered yet - do it now */
#if HAVE_SIGACTION
      struct sigaction act;
      memset(&act, 0, sizeof(struct sigaction));
      act.sa_flags   = 0/*|SA_RESTART*/;
      act.sa_handler = socatsignalpass;
      sigfillset(&act.sa_mask);
      if (Sigaction(signum, &act, NULL) < 0) {
	 /*! man does not say that errno is defined */
	 Warn3("sigaction(%d, %p, NULL): %s", signum, &act, strerror(errno));
      }
#else
      Signal(signum, socatsignalpass);
#endif /* !HAVE_SIGACTION */
   }
   sigdesc->sig_pids[sigdesc->sig_use++] = pid;
   return 0;
}

