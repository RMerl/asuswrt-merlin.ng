/* source: xio-process.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_process_h_included
#define __xio_process_h_included 1

extern const struct optdesc opt_setgid_early;
extern const struct optdesc opt_setgid;
extern const struct optdesc opt_setuid_early;
extern const struct optdesc opt_setuid;
extern const struct optdesc opt_substuser_early;
extern const struct optdesc opt_substuser;
#if defined(HAVE_SETGRENT) && defined(HAVE_GETGRENT) && defined(HAVE_ENDGRENT)
extern const struct optdesc opt_substuser_delayed;
#endif
extern const struct optdesc opt_chroot_early;
extern const struct optdesc opt_chroot;
extern const struct optdesc opt_setsid;
extern const struct optdesc opt_setpgid;

/* for option substuser-delayed, save info for later application */
extern bool delayeduser;
extern uid_t delayeduser_uid;	/* numeric user id to switch to */
extern gid_t delayeduser_gid;	/* numeric group id to switch to */
extern gid_t delayeduser_gids[NGROUPS];	/* num.supplementary group ids */
extern int   delayeduser_ngids;	/* number of suppl. gids */
extern char *delayeduser_name;	/* name of user to switch to */
extern char *delayeduser_dir;	/* home directory of user to switch to */
extern char *delayeduser_shell;	/* login shell of user to switch to */

extern int _xioopen_setdelayeduser(void);

#endif /* !defined(__xio_process_h_included) */
