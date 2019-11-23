/* source: xio-listen.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_listen_h_included
#define __xio_listen_h_included 1

extern const struct optdesc opt_backlog;
extern const struct optdesc opt_fork;
extern const struct optdesc opt_max_children;
extern const struct optdesc opt_range;

int
   xioopen_listen(struct single *xfd, int xioflags,
		  struct sockaddr *us, socklen_t uslen,
		  struct opt *opts, struct opt *opts0,
		  int pf, int socktype, int proto);
int _xioopen_listen(struct single *fd, int xioflags,
		    struct sockaddr *us, socklen_t uslen,
		 struct opt *opts, int pf, int socktype, int proto, int level);

#endif /* !defined(__xio_listen_h_included) */
