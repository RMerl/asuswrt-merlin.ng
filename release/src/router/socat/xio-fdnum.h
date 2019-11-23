/* source: xio-fdnum.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_fdnum_h_included
#define __xio_fdnum_h_included 1

extern const struct addrdesc addr_fd;

extern int xioopen_fd(struct opt *opts, int rw, xiosingle_t *xfd, int numfd, int dummy2, int dummy3);

#endif /* !defined(__xio_fdnum_h_included) */
