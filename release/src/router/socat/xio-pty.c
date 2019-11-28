/* source: xio-pty.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for creating pty addresses */

#include "xiosysincludes.h"
#include "xioopen.h"

#include "xio-named.h"
#include "xio-termios.h"


#if WITH_PTY

/* here define the preferred polling intervall, in seconds */
#define PTY_INTERVALL 1,0	/* for struct timespec */

#define MAXPTYNAMELEN 64

static int xioopen_pty(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *fd, unsigned groups, int dummy1, int dummy2, int dummy3);

const struct addrdesc addr_pty = { "pty",   3, xioopen_pty, GROUP_NAMED|GROUP_FD|GROUP_TERMIOS|GROUP_PTY, 0, 0, 0 HELP("") };

const struct optdesc opt_symbolic_link = { "symbolic-link", "link", OPT_SYMBOLIC_LINK, GROUP_PTY, PH_LATE, TYPE_FILENAME, OFUNC_SPEC, 0, 0 };
#if HAVE_POLL
const struct optdesc opt_pty_wait_slave = { "pty-wait-slave", "wait-slave", OPT_PTY_WAIT_SLAVE, GROUP_PTY, PH_EARLY, TYPE_BOOL,   OFUNC_SPEC, 0, 0 };
const struct optdesc opt_pty_intervall  = { "pty-interval",  NULL,         OPT_PTY_INTERVALL,  GROUP_PTY, PH_EARLY, TYPE_TIMESPEC, OFUNC_SPEC, 0, 0 };
#endif /* HAVE_POLL */

static int xioopen_pty(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *xfd, unsigned groups, int dummy1, int dummy2, int dummy3) {
   /* we expect the form: filename */
   int ptyfd = -1, ttyfd = -1;
#if defined(HAVE_DEV_PTMX) || defined(HAVE_DEV_PTC)
   bool useptmx = false;	/* use /dev/ptmx or equivalent */
#endif
#if HAVE_OPENPTY
   bool useopenpty = false;	/* try only openpty */
#endif	/* HAVE_OPENPTY */
   char ptyname[MAXPTYNAMELEN];
   char *tn = NULL;
   char *linkname = NULL;
   bool opt_unlink_close = true;	/* remove symlink afterwards */
   bool wait_slave = false;	/* true would be better for many platforms, but
				   some OSes cannot handle this, and for common
				   default behaviour as well as backward 
				   compatibility we choose "no" as default */
   struct timespec pollintv = { PTY_INTERVALL };

   if (argc != 1) {
      Error2("%s: wrong number of parameters (%d instead of 0)", argv[0], argc-1);
   }

   xfd->stream.howtoend = END_CLOSE;

   if (applyopts_single(&xfd->stream, opts, PH_INIT) < 0)  return -1;
   applyopts(-1, opts, PH_INIT);

   retropt_bool(opts, OPT_UNLINK_CLOSE, &opt_unlink_close);

   /* trying to set user-early, perm-early etc. here might be useless because
      file system entry is eventually available only past pty creation */
   /* name not yet known; umask should not be handled with this function! */
   /* umask does not affect resulting mode, on Linux 2.4 */
   applyopts_named("", opts, PH_EARLY);	/* umask! */

#if defined(HAVE_DEV_PTMX) || defined(HAVE_DEV_PTC)
   retropt_bool(opts, OPT_PTMX, &useptmx);
#endif
#if HAVE_OPENPTY
   retropt_bool(opts, OPT_OPENPTY, &useopenpty);
#endif

#if (defined(HAVE_DEV_PTMX) || defined(HAVE_DEV_PTC))
#  if  HAVE_OPENPTY
   useopenpty = !useptmx;
#  else /* !HAVE_OPENPTY */
   useptmx = true;
#  endif /* !HAVE_OPENPTY */
#else
# if HAVE_OPENPTY
   useopenpty = true;
# endif /* HAVE_OPENPTY */
#endif /* ! (defined(HAVE_DEV_PTMX) || defined(HAVE_DEV_PTC)) */

#if HAVE_POLL
   retropt_bool(opts, OPT_PTY_WAIT_SLAVE, &wait_slave);
   retropt_timespec(opts, OPT_PTY_INTERVALL, &pollintv);
#endif /* HAVE_POLL */

   if (applyopts_single(&xfd->stream, opts, PH_INIT) < 0)  return -1;
   applyopts2(-1, opts, PH_INIT, PH_EARLY);

   applyopts(-1, opts, PH_PREBIGEN);

#if defined(HAVE_DEV_PTMX)
#  define PTMX "/dev/ptmx"	/* Linux */
#elif HAVE_DEV_PTC
#  define PTMX "/dev/ptc"	/* AIX 4.3.3 */
#endif
#if HAVE_DEV_PTMX || HAVE_DEV_PTC
   if (useptmx) {
      if ((ptyfd = Open(PTMX, O_RDWR|O_NOCTTY, 0620)) < 0) {
	 Warn1("open(\""PTMX"\", O_RDWR|O_NOCTTY, 0620): %s",
	       strerror(errno));
	 /*!*/
      } else {
	 ;/*0 Info1("open(\""PTMX"\", O_RDWR|O_NOCTTY, 0620) -> %d", ptyfd);*/
      }
      if (ptyfd >= 0 && ttyfd < 0) {
	 /* we used PTMX before forking */
	 /*0 extern char *ptsname(int);*/
#if HAVE_GRANTPT	/* AIX, not Linux */
	 if (Grantpt(ptyfd)/*!*/ < 0) {
	    Warn2("grantpt(%d): %s", ptyfd, strerror(errno));
	 }
#endif /* HAVE_GRANTPT */
#if HAVE_UNLOCKPT
	 if (Unlockpt(ptyfd)/*!*/ < 0) {
	    Warn2("unlockpt(%d): %s", ptyfd, strerror(errno));
	 }
#endif /* HAVE_UNLOCKPT */
#if HAVE_PROTOTYPE_LIB_ptsname	/* AIX, not Linux */
	 if ((tn = Ptsname(ptyfd)) == NULL) {
	    Warn2("ptsname(%d): %s", ptyfd, strerror(errno));
	 } else {
	    Notice1("PTY is %s", tn);
	 }
#endif /* HAVE_PROTOTYPE_LIB_ptsname */
	 if (tn == NULL) {
	    if ((tn = Ttyname(ptyfd)) == NULL) {
	       Warn2("ttyname(%d): %s", ptyfd, strerror(errno));
	    }
	 }
	 ptyname[0] = '\0'; strncat(ptyname, tn, MAXPTYNAMELEN-1);
      }
   }
#endif /* HAVE_DEV_PTMX || HAVE_DEV_PTC */
#if HAVE_OPENPTY
   if (ptyfd < 0) {
      int result;
      if ((result = Openpty(&ptyfd, &ttyfd, ptyname, NULL, NULL)) < 0) {
	 Error4("openpty(%p, %p, %p, NULL, NULL): %s",
		&ptyfd, &ttyfd, ptyname, strerror(errno));
	 return -1;
      }
      Notice1("PTY is %s", ptyname);
   }
#endif /* HAVE_OPENPTY */

   if (!retropt_string(opts, OPT_SYMBOLIC_LINK, &linkname)) {
      if (Unlink(linkname) < 0 && errno != ENOENT) {
	 Error2("unlink(\"%s\"): %s", linkname, strerror(errno));
      }
      if (Symlink(ptyname, linkname) < 0) {
	 Error3("symlink(\"%s\", \"%s\"): %s",
		ptyname, linkname, strerror(errno));
      }
      if (opt_unlink_close) {
	 if ((xfd->stream.unlink_close = strdup(linkname)) == NULL) {
	    Error1("strdup(\"%s\"): out of memory", linkname);
	 }
	 xfd->stream.opt_unlink_close = true;
      }
   }

   applyopts_named(ptyname, opts, PH_PASTOPEN);
   applyopts_named(ptyname, opts, PH_FD);

   applyopts_cloexec(ptyfd, opts);/*!*/
   xfd->stream.dtype    = XIODATA_PTY;

   applyopts(ptyfd, opts, PH_FD);

   {
      /* special handling of user-late etc.; with standard behaviour (up to
	 1.7.1.1) they affected /dev/ptmx instead of /dev/pts/N */
      uid_t uid = -1, gid = -1;
      mode_t perm;

      bool dont;
      dont = retropt_uid(opts, OPT_USER_LATE, &uid);
      dont &= retropt_gid(opts, OPT_GROUP_LATE, &gid);

      if (!dont) {
	 if (Chown(ptyname, uid, gid) < 0) {
	    Error4("chown(\"%s\", %d, %d): %s",
		   ptyname, uid, gid, strerror(errno));
	 }
      }

      if (retropt_mode(opts, OPT_PERM_LATE, &perm) == 0) {
	 if (Chmod(ptyname, perm) < 0) {
	    Error3("chmod(\"%s\", %03o): %s",
		   ptyname, perm, strerror(errno));
	 }
      }

   }

   xfd->stream.fd = ptyfd;
   applyopts(ptyfd, opts, PH_LATE);
   if (applyopts_single(&xfd->stream, opts, PH_LATE) < 0)  return -1;

#if HAVE_POLL
   /* if you can and wish: */
   if (wait_slave) {
      /* try to wait until someone opens the slave side of the pty */
      /* we want to get a HUP (hangup) condition on the pty */
#if HAVE_DEV_PTMX || HAVE_DEV_PTC
      if (useptmx) {
	 ttyfd = Open(tn, O_RDWR|O_NOCTTY, 0620);
	 Close(ttyfd);
      }
#endif
#if HAVE_OPENPTY
      if (useopenpty) {
	 Close(ttyfd);
      }
#endif /* HAVE_OPENPTY */

      /* now we poll until the HUP vanishes - this indicates a slave conn. */
      while (true) {
	 struct pollfd ufd;
	 ufd.fd = ptyfd;
	 ufd.events = (POLLHUP);
	 if (Poll(&ufd, 1, 0) < 0) {
	    Error3("poll({%d, 0x%04hu,}, 1, 0): %s",
		   ufd.fd, ufd.events, strerror(errno));
	    /*! close something */
	    return -1;
	 }
	 if (!(ufd.revents & POLLHUP)) {
	    break;
	 }
	 Nanosleep(&pollintv, NULL);
	 continue;
      }
   }
#endif /* HAVE_POLL */

   return STAT_OK;
}
#endif /* WITH_PTY */
