/* source: xio-process.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file handles process related addresses options */

#include "xiosysincludes.h"
#include "xioopen.h"

#include "xio-process.h"

/****** process related options ******/
const struct optdesc opt_setgid_early= { "setgid-early",NULL,  OPT_SETGID_EARLY,GROUP_PROCESS, PH_EARLY,   TYPE_GIDT, OFUNC_SPEC };
const struct optdesc opt_setgid      = { "setgid",      NULL,  OPT_SETGID,      GROUP_PROCESS, PH_LATE2,    TYPE_GIDT, OFUNC_SPEC };
const struct optdesc opt_setuid_early= { "setuid-early",NULL,  OPT_SETUID_EARLY,GROUP_PROCESS, PH_EARLY,   TYPE_UIDT, OFUNC_SPEC };
const struct optdesc opt_setuid      = { "setuid",      NULL,  OPT_SETUID,      GROUP_PROCESS, PH_LATE2,    TYPE_UIDT, OFUNC_SPEC };
const struct optdesc opt_substuser_early   = { "substuser-early", "su-e",  OPT_SUBSTUSER_EARLY,   GROUP_PROCESS, PH_EARLY,  TYPE_UIDT, OFUNC_SPEC };
const struct optdesc opt_substuser   = { "substuser", "su",  OPT_SUBSTUSER,   GROUP_PROCESS, PH_LATE2,  TYPE_UIDT, OFUNC_SPEC };
#if defined(HAVE_SETGRENT) && defined(HAVE_GETGRENT) && defined(HAVE_ENDGRENT)
const struct optdesc opt_substuser_delayed = { "substuser-delayed", "su-d", OPT_SUBSTUSER_DELAYED,   GROUP_PROCESS, PH_INIT,  TYPE_UIDT, OFUNC_SPEC };
#endif
const struct optdesc opt_chroot_early = { "chroot-early", NULL, OPT_CHROOT_EARLY, GROUP_PROCESS, PH_EARLY, TYPE_STRING, OFUNC_SPEC };
const struct optdesc opt_chroot       = { "chroot",       NULL, OPT_CHROOT,       GROUP_PROCESS, PH_LATE, TYPE_STRING, OFUNC_SPEC };
const struct optdesc opt_setsid  = { "setsid",    "sid", OPT_SETSID,     GROUP_PROCESS,   PH_LATE, TYPE_BOOL,     OFUNC_SPEC };
const struct optdesc opt_setpgid = { "setpgid",   "pgid",OPT_SETPGID,    GROUP_FORK,   PH_LATE, TYPE_INT,      OFUNC_SPEC };


/* for option substuser-delayed, save info for later application */
bool delayeduser = false;
uid_t delayeduser_uid;	/* numeric user id to switch to */
gid_t delayeduser_gid;	/* numeric group id to switch to */
gid_t delayeduser_gids[NGROUPS];	/* num.supplementary group ids */
int   delayeduser_ngids;	/* number of suppl. gids */
char *delayeduser_name;	/* name of user to switch to */
char *delayeduser_dir;	/* home directory of user to switch to */
char *delayeduser_shell;	/* login shell of user to switch to */


int _xioopen_setdelayeduser(void) {
   if (delayeduser) {
#if HAVE_SETGROUPS
      if ((Setgroups(delayeduser_ngids, delayeduser_gids)) != 0) {
	 Error3("setgroups(%d, %p): %s",
		delayeduser_ngids, delayeduser_gids, strerror(errno));
      }
#endif /* HAVE_SETGROUPS */
      if (Setgid(delayeduser_gid) < 0) {
	 Error2("setgid("F_gid"): %s", delayeduser_gid,
		strerror(errno));
      }
      if (Setuid(delayeduser_uid) < 0) {
	 Error2("setuid("F_uid"): %s", delayeduser_uid,
		strerror(errno));
      }
#if 1
      if (setenv("USER", delayeduser_name, 1) < 0)
	 Error1("setenv(\"USER\", \"%s\", 1): insufficient space",
		delayeduser_name);
      if (setenv("LOGNAME", delayeduser_name, 1) < 0)
	 Error1("setenv(\"LOGNAME\", \"%s\", 1): insufficient space",
		delayeduser_name);
      if (setenv("HOME", delayeduser_dir, 1) < 0)
	 Error1("setenv(\"HOME\", \"%s\", 1): insufficient space",
		delayeduser_dir);
      if (setenv("SHELL", delayeduser_shell, 1) < 0)
	 Error1("setenv(\"SHELL\", \"%s\", 1): insufficient space",
		delayeduser_shell);
#endif
      delayeduser = false;
   }
   return 0;
}

