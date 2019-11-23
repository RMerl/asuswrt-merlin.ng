/* source: xio-ip4.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_ip4_h_included
#define __xio_ip4_h_included 1

extern const struct optdesc opt_ip4_add_membership;

int xioparsenetwork_ip4(const char *rangename, struct xiorange *range);
extern
int xiocheckrange_ip4(struct sockaddr_in *pa, struct xiorange *range);
extern int
xiosetsockaddrenv_ip4(int idx, char *namebuff, size_t namelen,
		      char *valuebuff, size_t valuelen,
		      struct sockaddr_in *sa, int ipproto);

#endif /* !defined(__xio_ip4_h_included) */
