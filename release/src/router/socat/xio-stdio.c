/* source: xio-stdio.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for opening addresses stdio type */

#include "xiosysincludes.h"
#include "xioopen.h"

#include "xio-fdnum.h"
#include "xio-stdio.h"


#if WITH_STDIO

static int xioopen_stdio(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *fd, unsigned groups, int dummy1, int dummy2, int dummy3);
static int xioopen_stdfd(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *xfd, unsigned groups, int fd, int dummy2, int dummy3);


/* we specify all option groups that we can imagine for a FD, becasue the
   changed parsing mechanism does not allow us to check the type of FD before
   applying the options */
const struct addrdesc addr_stdio  = { "stdio",  3, xioopen_stdio, GROUP_FD|GROUP_FIFO|GROUP_CHR|GROUP_BLK|GROUP_FILE|GROUP_SOCKET|GROUP_TERMIOS|GROUP_SOCK_UNIX|GROUP_SOCK_IP|GROUP_IPAPP, 0, 0, 0 HELP(NULL) };
const struct addrdesc addr_stdin  = { "stdin",  1, xioopen_stdfd, GROUP_FD|GROUP_FIFO|GROUP_CHR|GROUP_BLK|GROUP_FILE|GROUP_SOCKET|GROUP_TERMIOS|GROUP_SOCK_UNIX|GROUP_SOCK_IP|GROUP_IPAPP, 0, 0, 0 HELP(NULL) };
const struct addrdesc addr_stdout = { "stdout", 2, xioopen_stdfd, GROUP_FD|GROUP_FIFO|GROUP_CHR|GROUP_BLK|GROUP_FILE|GROUP_SOCKET|GROUP_TERMIOS|GROUP_SOCK_UNIX|GROUP_SOCK_IP|GROUP_IPAPP, 1, 0, 0 HELP(NULL) };
const struct addrdesc addr_stderr = { "stderr", 2, xioopen_stdfd, GROUP_FD|GROUP_FIFO|GROUP_CHR|GROUP_BLK|GROUP_FILE|GROUP_SOCKET|GROUP_TERMIOS|GROUP_SOCK_UNIX|GROUP_SOCK_IP|GROUP_IPAPP, 2, 0, 0 HELP(NULL) };


/* process a bidirectional "stdio" or "-" argument with options.
   generate a dual address. */
int xioopen_stdio_bi(xiofile_t *sock) {
   struct opt *optspr;
   unsigned int groups1 = addr_stdio.groups;
   int result;

   if (xioopen_makedual(sock) < 0) {
      return -1;
   }

   sock->dual.stream[0]->tag = XIO_TAG_RDONLY;
   sock->dual.stream[0]->fd = 0 /*stdin*/;
   sock->dual.stream[1]->tag = XIO_TAG_WRONLY;
   sock->dual.stream[1]->fd = 1 /*stdout*/;
   sock->dual.stream[0]->howtoend =
      sock->dual.stream[1]->howtoend = END_NONE;

#if WITH_TERMIOS
   if (Isatty(sock->dual.stream[0]->fd)) {
      if (Tcgetattr(sock->dual.stream[0]->fd,
		    &sock->dual.stream[0]->savetty)
	  < 0) {
	 Warn2("cannot query current terminal settings on fd %d: %s",
	       sock->dual.stream[0]->fd, strerror(errno));
      } else {
	 sock->dual.stream[0]->ttyvalid = true;
      }
   }
   if (Isatty(sock->dual.stream[1]->fd)) {
      if (Tcgetattr(sock->dual.stream[1]->fd,
		    &sock->dual.stream[1]->savetty)
	  < 0) {
	 Warn2("cannot query current terminal settings on fd %d: %s",
	       sock->dual.stream[1]->fd, strerror(errno));
      } else {
	 sock->dual.stream[1]->ttyvalid = true;
      }
   }
#endif /* WITH_TERMIOS */

   /* options here are one-time and one-direction, no second use */
   retropt_bool(sock->stream.opts, OPT_IGNOREEOF, &sock->dual.stream[0]->ignoreeof);

   /* extract opts that should be applied only once */
   if ((optspr = copyopts(sock->stream.opts, GROUP_PROCESS)) == NULL) {
      return -1;
   }
   /* here we copy opts, because most have to be applied twice! */
   if ((sock->dual.stream[1]->opts = copyopts(sock->stream.opts, GROUP_FD|GROUP_APPL|(groups1&~GROUP_PROCESS))) == NULL) {
      return -1;
   }
   sock->dual.stream[0]->opts = sock->stream.opts;
   sock->stream.opts = NULL;

   if (applyopts_single(sock->dual.stream[0],
			sock->dual.stream[0]->opts, PH_INIT)
       < 0)
      return -1;
   if (applyopts_single(sock->dual.stream[1],
			sock->dual.stream[1]->opts, PH_INIT)
       < 0)
      return -1;
   applyopts(-1, sock->dual.stream[0]->opts, PH_INIT);
   applyopts(-1, sock->dual.stream[1]->opts, PH_INIT);
   if ((result = applyopts(-1, optspr, PH_EARLY)) < 0)
      return result;
   if ((result = applyopts(-1, optspr, PH_PREOPEN)) < 0)
      return result;

   /* apply options to first FD */
   if ((result =
	applyopts(sock->dual.stream[0]->fd,
		  sock->dual.stream[0]->opts, PH_ALL))
       < 0) {
      return result;
   }
   if ((result = _xio_openlate(sock->dual.stream[0],
			       sock->dual.stream[0]->opts)) < 0) {
      return result;
   }
#if 0
   /* ignore this opt */
   retropt_bool(sock->dual.stream[0]->opts, OPT_COOL_WRITE);
#endif

   /* apply options to second FD */
   if ((result = applyopts(sock->dual.stream[1]->fd,
			   sock->dual.stream[1]->opts, PH_ALL)) < 0) {
      return result;
   }
   if ((result = _xio_openlate(sock->dual.stream[1],
			       sock->dual.stream[1]->opts)) < 0) {
      return result;
   }

#if 0
   if ((result = _xio_openlate(sock->dual.stream[1], optspr)) < 0) {
      return result;
   }
#endif

   Notice("reading from and writing to stdio");
   return 0;
}


/* wrap around unidirectional xioopensingle and xioopen_fd to automatically determine stdin or stdout fd depending on rw.
   Do not set FD_CLOEXEC flag. */
static int xioopen_stdio(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *fd, unsigned groups, int dummy1, int dummy2, int dummy3) {
   int rw = (xioflags&XIO_ACCMODE);

   if (argc != 1) {
      Error2("%s: wrong number of parameters (%d instead of 0)", argv[0], argc-1);
   }

   if (rw == XIO_RDWR) {
      return xioopen_stdio_bi(fd);
   }

   Notice2("using %s for %s",
	   &("stdin\0\0\0stdout"[rw<<3]),
	   ddirection[rw]);
   return xioopen_fd(opts, rw, &fd->stream, rw, dummy2, dummy3);
}

/* wrap around unidirectional xioopensingle and xioopen_fd to automatically determine stdin or stdout fd depending on rw.
   Do not set FD_CLOEXEC flag. */
static int xioopen_stdfd(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *xfd, unsigned groups, int fd, int dummy2, int dummy3) {
   int rw = (xioflags&XIO_ACCMODE);

   if (argc != 1) {
      Error2("%s: wrong number of parameters (%d instead of 0)", argv[0], argc-1);
   }
   Notice2("using %s for %s",
	   &("stdin\0\0\0stdout\0\0stderr"[fd<<3]),
	   ddirection[rw]);
   return xioopen_fd(opts, rw, &xfd->stream, fd, dummy2, dummy3);
}
#endif /* WITH_STDIO */
