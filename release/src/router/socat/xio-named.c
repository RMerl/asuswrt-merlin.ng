/* source: xio-named.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for filesystem entry functions */

#include "xiosysincludes.h"

#if _WITH_NAMED

#include "xioopen.h"
#include "xio-named.h"


#if WITH_NAMED
const struct optdesc opt_group_early = { "group-early", NULL, OPT_GROUP_EARLY, GROUP_NAMED, PH_PREOPEN, TYPE_GIDT, OFUNC_SPEC };
const struct optdesc opt_perm_early  = { "perm-early",  NULL, OPT_PERM_EARLY,  GROUP_NAMED, PH_PREOPEN, TYPE_MODET,OFUNC_SPEC };
const struct optdesc opt_user_early  = { "user-early",  NULL, OPT_USER_EARLY,  GROUP_NAMED, PH_PREOPEN, TYPE_UIDT, OFUNC_SPEC };
/*0 const struct optdesc opt_force       = { "force",       NULL, OPT_FORCE,       GROUP_NAMED, PH_???,    TYPE_BOOL, OFUNC_SPEC };*/
const struct optdesc opt_unlink      = { "unlink",      NULL, OPT_UNLINK,      GROUP_NAMED, PH_PREOPEN,  TYPE_BOOL, OFUNC_SPEC };
const struct optdesc opt_unlink_early= { "unlink-early",NULL, OPT_UNLINK_EARLY,GROUP_NAMED, PH_EARLY,    TYPE_BOOL, OFUNC_SPEC };
const struct optdesc opt_unlink_late = { "unlink-late", NULL, OPT_UNLINK_LATE, GROUP_NAMED, PH_PASTOPEN, TYPE_BOOL, OFUNC_SPEC };
const struct optdesc opt_unlink_close  = { "unlink-close", NULL, OPT_UNLINK_CLOSE, GROUP_NAMED, PH_LATE,  TYPE_BOOL, OFUNC_SPEC };
const struct optdesc opt_umask       = { "umask",       NULL, OPT_UMASK,       GROUP_NAMED, PH_EARLY,  TYPE_MODET, OFUNC_SPEC };
#endif /* WITH_NAMED */

/* applies to filesystem entry all options belonging to phase */
int applyopts_named(const char *filename, struct opt *opts, unsigned int phase) {
   struct opt *opt;

   if (!opts)  return 0;

   opt = opts; while (opt->desc != ODESC_END) {
      if (opt->desc == ODESC_DONE ||
	  opt->desc->phase != phase && phase != PH_ALL ||
	  !(opt->desc->group & GROUP_NAMED)) {
	 ++opt; continue; }
      switch (opt->desc->optcode) {
      case OPT_GROUP_EARLY:
      case OPT_GROUP:
	 if (Chown(filename, -1, opt->value.u_gidt) < 0) {
	    Error3("chown(\"%s\", -1, "F_gid"): %s", filename,
		   opt->value.u_gidt, strerror(errno));
	 }
	 break;
      case OPT_USER_EARLY:
      case OPT_USER:
	 if (Chown(filename, opt->value.u_uidt, -1) < 0) {
	    Error3("chown(\"%s\", "F_uid", -1): %s", filename,
		   opt->value.u_uidt, strerror(errno));
	 }
	 break;
      case OPT_PERM_EARLY:
      case OPT_PERM:
	 if (Chmod(filename, opt->value.u_modet) < 0) {
	    Error3("chmod(\"%s\", "F_mode"): %s",
		   filename, opt->value.u_modet, strerror(errno));
	 }
	 break;
      case OPT_UNLINK_EARLY:
      case OPT_UNLINK:
      case OPT_UNLINK_LATE:
	 if (Unlink(filename) < 0) {
	    if (errno == ENOENT) {
	       Warn2("unlink(\"%s\"): %s", filename, strerror(errno));
	    } else {
	       Error2("unlink(\"%s\"): %s", filename, strerror(errno));
	    }
	 }
	 break;
      case OPT_UMASK:
	 if (Umask(opt->value.u_modet) < 0) {
	    /* linux docu says it always succeeds, but who believes it? */
	    Error2("umask("F_mode"): %s", opt->value.u_modet, strerror(errno));
	 }
	 break;
      default: Error1("applyopts_named(): option \"%s\" not implemented",
		      opt->desc->defname);
	 break;
      }
      opt->desc = ODESC_DONE;
      ++opt;
   }
   return 0;
}


/* perform actions that are common to all NAMED group addresses: checking if 
   the entry exists, parsing options, ev.removing old filesystem entry or
   setting early owners and permissions.
   It applies options of PH_EARLY and PH_PREOPEN.
   If the path exists, its st_mode field is returned.
   After this sub you may proceed with open() or whatever...
   */
int _xioopen_named_early(int argc, const char *argv[], xiofile_t *xfd,
			 int groups,
		      bool *exists, struct opt *opts) {
   const char *path = argv[1];
#if HAVE_STAT64
   struct stat64 statbuf;
#else
   struct stat statbuf;
#endif /* !HAVE_STAT64 */
   bool opt_unlink_early = false;

   if (argc != 2) {
      Error2("%s: wrong number of parameters (%d instead of 1)", argv[0]?argv[0]:"<named>", argc);
   }
   statbuf.st_mode = 0;
   /* find the appropriate groupbits */
   if (
#if HAVE_STAT64
       Stat64(path, &statbuf) < 0
#else
       Stat(path, &statbuf) < 0
#endif /* !HAVE_STAT64 */
      ) {
      if (errno != ENOENT) {
	 Error2("stat(\"%s\"): %s", path, strerror(errno));
	    return STAT_RETRYLATER;
      }
      *exists = false;
   } else {
      *exists = true;
   }

   if (applyopts_single(&xfd->stream, opts, PH_INIT) < 0)  return -1;
   applyopts(-1, opts, PH_INIT);

   retropt_bool(opts, OPT_UNLINK_EARLY, &opt_unlink_early);
   if (*exists && opt_unlink_early) {
      Info1("\"%s\" already exists; removing it", path);
      if (Unlink(path) < 0) {
	 Error2("unlink(\"%s\"): %s", path, strerror(errno));
      } else {
	 *exists = false;
      }
   }

   applyopts_named(path, opts, PH_EARLY);
   applyopts(-1, opts, PH_EARLY);
   if (*exists) {
      applyopts_named(path, opts, PH_PREOPEN);
   } else {
      dropopts(opts, PH_PREOPEN);
   }

   return statbuf.st_mode;
}


/* retrieve the OPEN group options and perform the open() call.
   returns the file descriptor or a negative value.
   Applies options of phases PREOPEN, OPEN, PASTOPEN, and FD
*/
int _xioopen_open(const char *path, int rw, struct opt *opts) {
   mode_t mode = 0666;
   flags_t flags = rw;
   bool flag;
   int fd;

   applyopts_named(path, opts, PH_PREOPEN);

   /* this only applies pure OPEN flags, not mixed OPEN/FCNTL options */
   applyopts_flags(opts, GROUP_OPEN, &flags);

   /* we have to handle mixed OPEN/FCNTL flags specially */
   if (retropt_bool(opts, OPT_O_APPEND, &flag) >= 0   && flag)
      flags |= O_APPEND;
   if (retropt_bool(opts, OPT_O_NONBLOCK, &flag) >= 0 && flag)
      flags |= O_NONBLOCK;
#ifdef O_ASYNC
   if (retropt_bool(opts, OPT_O_ASYNC, &flag) >= 0    && flag)
      flags |= O_ASYNC;
#endif
   if (retropt_bool(opts, OPT_O_TRUNC, &flag) >= 0  && flag)
      flags |= O_TRUNC;
#ifdef O_BINARY
   if (retropt_bool(opts, OPT_O_BINARY,    &flag) >= 0   && flag)
      flags |= O_BINARY;
#endif
#ifdef O_TEXT
   if (retropt_bool(opts, OPT_O_TEXT,      &flag) >= 0   && flag)
      flags |= O_TEXT;
#endif
#ifdef O_NOINHERIT
   if (retropt_bool(opts, OPT_O_NOINHERIT, &flag) >= 0   && flag)
      flags |= O_NOINHERIT;
#endif
#ifdef O_NOATIME
   if (retropt_bool(opts, OPT_O_NOATIME,   &flag) >= 0   && flag)
      flags |= O_NOATIME;
#endif

   retropt_modet(opts, OPT_PERM,      &mode);

   if ((fd = Open(path, flags, mode)) < 0) {
      Error4("open(\"%s\", 0%lo, 0%03o): %s",
	     path, flags, mode, strerror(errno));
      return STAT_RETRYLATER;
   }
   /*0 Info4("open(\"%s\", 0%o, 0%03o) -> %d", path, flags, mode, fd);*/
   applyopts_named(path, opts, PH_PASTOPEN);
#if 0
   applyopts_named(path, opts, PH_FD);
   applyopts(fd, opts, PH_FD);
   applyopts_cloexec(fd, opts);
#endif
   return fd;
}

#endif /* _WITH_NAMED */
