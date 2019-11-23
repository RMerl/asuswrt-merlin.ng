/* source: xio-tcpwrap.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_tcpwrap_h_included
#define __xio_tcpwrap_h_included 1

#if (WITH_TCP || WITH_UDP) && WITH_LIBWRAP

extern const struct optdesc opt_tcpwrappers;
extern const struct optdesc opt_tcpwrap_etc;
extern const struct optdesc opt_tcpwrap_hosts_allow_table;
extern const struct optdesc opt_tcpwrap_hosts_deny_table;

extern int xio_retropt_tcpwrap(xiosingle_t *xfd, struct opt *opts);
extern
int xio_tcpwrap_check(xiosingle_t *xfd, union sockaddr_union *us,
		      union sockaddr_union *them);

#endif /* (WITH_TCP || WITH_UDP) && WITH_LIBWRAP */

#endif /* !defined(__xio_tcpwrap_h_included) */
