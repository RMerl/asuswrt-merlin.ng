/* source: xio-pipe.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for opening addresses of pipe type */

#include "xiosysincludes.h"
#include "xioopen.h"

#include "xio-named.h"


#if WITH_PIPE

static int xioopen_fifo(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *fd, unsigned groups, int dummy1, int dummy2, int dummy3);
static int xioopen_fifo_unnamed(xiofile_t *sock, struct opt *opts);


const struct addrdesc addr_pipe   = { "pipe",   3, xioopen_fifo,  GROUP_FD|GROUP_NAMED|GROUP_OPEN|GROUP_FIFO, 0, 0, 0 HELP(":<filename>") };


/* process an unnamed bidirectional "pipe" or "fifo" or "echo" argument with
   options */
static int xioopen_fifo_unnamed(xiofile_t *sock, struct opt *opts) {
   struct opt *opts2;
   int filedes[2];
   int numleft;
   int result;

   if (applyopts_single(&sock->stream, opts, PH_INIT) < 0)  return -1;
   applyopts(-1, opts, PH_INIT);

   if (Pipe(filedes) != 0) {
      Error2("pipe(%p): %s", filedes, strerror(errno));
      return -1;
   }
   /*0 Info2("pipe({%d,%d})", filedes[0], filedes[1]);*/

   sock->common.tag               = XIO_TAG_RDWR;
   sock->stream.dtype             = XIODATA_PIPE;
   sock->stream.fd                = filedes[0];
   sock->stream.para.bipipe.fdout = filedes[1];
   applyopts_cloexec(sock->stream.fd,                opts);
   applyopts_cloexec(sock->stream.para.bipipe.fdout, opts);

   /* one-time and input-direction options, no second application */
   retropt_bool(opts, OPT_IGNOREEOF, &sock->stream.ignoreeof);

   /* here we copy opts! */
   if ((opts2 = copyopts(opts, GROUP_FIFO)) == NULL) {
      return STAT_NORETRY;
   }

   /* apply options to first FD */
   if ((result = applyopts(sock->stream.fd, opts, PH_ALL)) < 0) {
      return result;
   }
   if ((result = applyopts_single(&sock->stream, opts, PH_ALL)) < 0) {
      return result;
   }

   /* apply options to second FD */
   if ((result = applyopts(sock->stream.para.bipipe.fdout, opts2, PH_ALL)) < 0)
   {
      return result;
   }

   if ((numleft = leftopts(opts)) > 0) {
      Error1("%d option(s) could not be used", numleft);
      showleft(opts);
   }
   Notice("writing to and reading from unnamed pipe");
   return 0;
}


/* open a named or unnamed pipe/fifo */
static int xioopen_fifo(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *fd, unsigned groups, int dummy1, int dummy2, int dummy3) {
   const char *pipename = argv[1];
   int rw = (xioflags & XIO_ACCMODE);
#if HAVE_STAT64
   struct stat64 pipstat;
#else
   struct stat pipstat;
#endif /* !HAVE_STAT64 */
   bool opt_unlink_early = false;
   bool opt_unlink_close = true;
   mode_t mode = 0666;
   int result;

   if (argc == 1) {
      return xioopen_fifo_unnamed(fd, fd->stream.opts);
   }

   if (argc != 2) {
      Error2("%s: wrong number of parameters (%d instead of 1)", argv[0], argc-1);
   }

   if (applyopts_single(&fd->stream, opts, PH_INIT) < 0)  return -1;
   applyopts(-1, opts, PH_INIT);

   retropt_bool(opts, OPT_UNLINK_EARLY, &opt_unlink_early);
   applyopts_named(pipename, opts, PH_EARLY);	/* umask! */
   applyopts(-1, opts, PH_EARLY);

   if (opt_unlink_early) {
      if (Unlink(pipename) < 0) {
	 if (errno == ENOENT) {
	    Warn2("unlink(%s): %s", pipename, strerror(errno));
	 } else {
	    Error2("unlink(%s): %s", pipename, strerror(errno));
	    return STAT_RETRYLATER;
	 }
      }
   }

   retropt_bool(opts, OPT_UNLINK_CLOSE, &opt_unlink_close);
   retropt_modet(opts, OPT_PERM, &mode);
   if (applyopts_named(pipename, opts, PH_EARLY) < 0) {
      return STAT_RETRYLATER;
   }
   if (applyopts_named(pipename, opts, PH_PREOPEN) < 0) {
      return STAT_RETRYLATER;
   }
   if (
#if HAVE_STAT64
       Stat64(pipename, &pipstat) < 0
#else
       Stat(pipename, &pipstat) < 0
#endif /* !HAVE_STAT64 */
      ) {
      if (errno != ENOENT) {
	 Error3("stat(\"%s\", %p): %s", pipename, &pipstat, strerror(errno));
      } else {
	 Debug1("xioopen_fifo(\"%s\"): does not exist, creating fifo", pipename);
#if 0
	 result = Mknod(pipename, S_IFIFO|mode, 0);
	 if (result < 0) {
	    Error3("mknod(%s, %d, 0): %s", pipename, mode, strerror(errno));
	    return STAT_RETRYLATER;
	 }
#else
	 result = Mkfifo(pipename, mode);
	 if (result < 0) {
	    Error3("mkfifo(%s, %d): %s", pipename, mode, strerror(errno));
	    return STAT_RETRYLATER;
	 }
#endif
	 Notice2("created named pipe \"%s\" for %s", pipename, ddirection[rw]);
	 applyopts_named(pipename, opts, PH_ALL);

      }
      if (opt_unlink_close) {
	 if ((fd->stream.unlink_close = strdup(pipename)) == NULL) {
	    Error1("strdup(\"%s\"): out of memory", pipename);
	 }
	 fd->stream.opt_unlink_close = true;
      }
   } else {
      /* exists */
      Debug1("xioopen_fifo(\"%s\"): already exist, opening it", pipename);
      Notice3("opening %s \"%s\" for %s",
	      filetypenames[(pipstat.st_mode&S_IFMT)>>12],
	      pipename, ddirection[rw]);
      /*applyopts_early(pipename, opts);*/
      applyopts_named(pipename, opts, PH_EARLY);
   }

   if ((result = _xioopen_open(pipename, rw, opts)) < 0) {
      return result;
   }
   fd->stream.fd = result;

   applyopts_named(pipename, opts, PH_FD);
   applyopts(fd->stream.fd, opts, PH_FD);
   applyopts_cloexec(fd->stream.fd, opts);
   return _xio_openlate(&fd->stream, opts);
}

#endif /* WITH_PIPE */
