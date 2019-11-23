/* source: xio-sctp.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_sctp_h_included
#define __xio_sctp_h_included 1

extern const struct addrdesc addr_sctp_connect;
extern const struct addrdesc addr_sctp_listen;
extern const struct addrdesc addr_sctp4_connect;
extern const struct addrdesc addr_sctp4_listen;
extern const struct addrdesc addr_sctp6_connect;
extern const struct addrdesc addr_sctp6_listen;

extern const struct optdesc opt_sctp_nodelay;
extern const struct optdesc opt_sctp_maxseg;
extern const struct optdesc opt_sctp_maxseg_late;

#endif /* !defined(__xio_sctp_h_included) */
