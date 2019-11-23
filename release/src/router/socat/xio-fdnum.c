/* source: xio-fdnum.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for opening addresses of fdnum type */

#include "xiosysincludes.h"
#include "xioopen.h"

#include "xio-fdnum.h"


#if WITH_FDNUM

static int xioopen_fdnum(int argc, const char *argv[], struct opt *opts, int rw, xiofile_t *xfd, unsigned groups, int dummy1, int dummy2, int dummy3);


const struct addrdesc addr_fd     = { "fd",     3, xioopen_fdnum, GROUP_FD|GROUP_FIFO|GROUP_CHR|GROUP_BLK|GROUP_FILE|GROUP_SOCKET|GROUP_TERMIOS|GROUP_SOCK_UNIX|GROUP_SOCK_IP|GROUP_IPAPP, 0, 0, 0 HELP(":<num>") };


/* use some file descriptor and apply the options. Set the FD_CLOEXEC flag. */
static int xioopen_fdnum(int argc, const char *argv[], struct opt *opts,
			 int xioflags, xiofile_t *xfd, unsigned groups,
			 int dummy1, int dummy2, int dummy3) {
   char *a1;
   int rw = (xioflags&XIO_ACCMODE);
   int numfd;
   int result;

   if (argc != 2) {
      Error3("%s:%s: wrong number of parameters (%d instead of 1)", argv[0], argv[1], argc-1);
   }

   numfd = strtoul(argv[1], &a1, 0);
   if (*a1 != '\0') {
      Error1("error in FD number \"%s\"", argv[1]);
   }
   /* we dont want to see these fds in child processes */
   if (Fcntl_l(numfd, F_SETFD, FD_CLOEXEC) < 0) {
      Warn2("fcntl(%d, F_SETFD, FD_CLOEXEC): %s", numfd, strerror(errno));
   }
   Notice2("using file descriptor %d for %s", numfd, ddirection[rw]);
   if ((result = xioopen_fd(opts, rw, &xfd->stream, numfd, dummy2, dummy3)) < 0) {
      return result;
   }
   return 0;
}

#endif /* WITH_FDNUM */

#if WITH_FD

/* retrieve and apply options to a standard file descriptor.
   Do not set FD_CLOEXEC flag. */
int xioopen_fd(struct opt *opts, int rw, xiosingle_t *xfd, int numfd, int dummy2, int dummy3) {

   xfd->fd = numfd;
   xfd->howtoend = END_NONE;

#if WITH_TERMIOS
   if (Isatty(xfd->fd)) {
      if (Tcgetattr(xfd->fd, &xfd->savetty) < 0) {
	 Warn2("cannot query current terminal settings on fd %d: %s",
	       xfd->fd, strerror(errno));
      } else {
	 xfd->ttyvalid = true;
      }
   }
#endif /* WITH_TERMIOS */
   if (applyopts_single(xfd, opts, PH_INIT) < 0)  return -1;
   applyopts(-1, opts, PH_INIT);

   applyopts2(xfd->fd, opts, PH_INIT, PH_FD);

   return _xio_openlate(xfd, opts);
}

#endif /* WITH_FD */
