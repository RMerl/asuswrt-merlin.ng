/* 
 * Copyright (C) 1995-1997, 1999 Jeffrey A. Uphoff
 * Modified by Olaf Kirch, Dec. 1996.
 *
 * NSM for Linux.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sm_inter.h"
#include "system.h"
#include "xlog.h"

/*
 * Status definitions.
 */
#define STAT_FAIL	stat_fail
#define STAT_SUCC	stat_succ

/*
 * Function prototypes.
 */
extern _Bool	statd_matchhostname(const char *hostname1, const char *hostname2);
extern _Bool	statd_present_address(const struct sockaddr *sap, char *buf,
					const size_t buflen);
__attribute__((__malloc__))
extern char *	statd_canonical_name(const char *hostname);

extern void	my_svc_run(int);
extern void	notify_hosts(void);
extern void	shuffle_dirs(void);
extern int	statd_get_socket(void);
extern int	process_notify_list(void);
extern int	process_reply(FD_SET_TYPE *);
extern char *	xstrdup(const char *);
extern void *	xmalloc(size_t);
extern void	load_state(void);

/*
 * Host status structure and macros.
 */
stat_chge		SM_stat_chge;
#define MY_NAME		SM_stat_chge.mon_name
#define MY_STATE	SM_stat_chge.state

/*
 * Some timeout values.  (Timeout values are in whole seconds.)
 */
#define CALLBACK_TIMEOUT	 3 /* For client call-backs. */
#define NOTIFY_TIMEOUT		 5 /* For status-change notifications. */
#define SELECT_TIMEOUT		10 /* Max select() timeout when work to do. */
#define MAX_TRIES		 5 /* Max number of tries for any host. */

/*
 * Modes of operation - Lon
 */
extern int run_mode;
#define MODE_NODAEMON 1		/* No-daemon/foreground mode. */
#define MODE_LOG_STDERR 2	/* in foreground mode, log to stderr */
#define MODE_NOTIFY_ONLY 4	/* Send SM_NOTIFY to everyone monitored on
				   a single interface/alias */
/* LH - notify_only mode would be for notifying hosts on an IP alias
 * that just came back up, for ex, when failing over a HA service to
 * another host.... */
#define STATIC_HOSTNAME 8	/* Always use the hostname set by -n */
#define	MODE_NO_NOTIFY	16	/* Don't notify peers of a reboot */
