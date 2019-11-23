/* source: xio-creat.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for opening addresses of create type */

#include "xiosysincludes.h"

#if WITH_CREAT

#include "xioopen.h"
#include "xio-named.h"
#include "xio-creat.h"


static int xioopen_creat(int arg, const char *argv[], struct opt *opts, int rw, xiofile_t *fd, unsigned groups, int dummy1, int dummy2, int dummy3);


/*! within stream model, this is a write-only address - use 2 instead of 3 */
const struct addrdesc addr_creat  = { "create", 3, xioopen_creat, GROUP_FD|GROUP_NAMED|GROUP_FILE, 0, 0, 0 HELP(":<filename>") };


/* retrieve the mode option and perform the creat() call.
   returns the file descriptor or a negative value. */
static int _xioopen_creat(const char *path, int rw, struct opt *opts) {
   mode_t mode = 0666;
   int fd;

   retropt_modet(opts, OPT_PERM,      &mode);

   if ((fd = Creat(path, mode)) < 0) {
      Error3("creat(\"%s\", 0%03o): %s",
	     path, mode, strerror(errno));
      return STAT_RETRYLATER;
   }
   return fd;
}


static int xioopen_creat(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *fd, unsigned groups, int dummy1, int dummy2, int dummy3) {
   const char *filename = argv[1];
   int rw = (xioflags&XIO_ACCMODE);
   bool exists;
   bool opt_unlink_close = false;
   int result;

   /* remove old file, or set user/permissions on old file; parse options */
   if ((result = _xioopen_named_early(argc, argv, fd, groups, &exists, opts)) < 0) {
      return result;
   }

   retropt_bool(opts, OPT_UNLINK_CLOSE, &opt_unlink_close);
   if (opt_unlink_close) {
      if ((fd->stream.unlink_close = strdup(filename)) == NULL) {
	 Error1("strdup(\"%s\"): out of memory", filename);
      }
      fd->stream.opt_unlink_close = true;
   }

   Notice2("creating regular file \"%s\" for %s", filename, ddirection[rw]);
   if ((result = _xioopen_creat(filename, rw, opts)) < 0)
      return result;
   fd->stream.fd = result;

   applyopts_named(filename, opts, PH_PASTOPEN);
   if ((result = applyopts2(fd->stream.fd, opts, PH_PASTOPEN, PH_LATE2)) < 0)
      return result;

   applyopts_cloexec(fd->stream.fd, opts);

   applyopts_fchown(fd->stream.fd, opts);

   if ((result = _xio_openlate(&fd->stream, opts)) < 0)
      return result;

   return 0;
}

#endif /* WITH_CREAT */
