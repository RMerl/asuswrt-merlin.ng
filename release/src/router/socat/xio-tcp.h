/* source: xio-tcp.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_tcp_h_included
#define __xio_tcp_h_included 1

extern const struct addrdesc addr_tcp_connect;
extern const struct addrdesc addr_tcp_listen;
extern const struct addrdesc addr_tcp4_connect;
extern const struct addrdesc addr_tcp4_listen;
extern const struct addrdesc addr_tcp6_connect;
extern const struct addrdesc addr_tcp6_listen;

extern const struct optdesc opt_tcp_nodelay;
extern const struct optdesc opt_tcp_maxseg;
extern const struct optdesc opt_tcp_maxseg_late;
extern const struct optdesc opt_tcp_cork;
extern const struct optdesc opt_tcp_stdurg;
extern const struct optdesc opt_tcp_rfc1323;
extern const struct optdesc opt_tcp_keepidle;
extern const struct optdesc opt_tcp_keepintvl;
extern const struct optdesc opt_tcp_keepcnt;
extern const struct optdesc opt_tcp_syncnt;
extern const struct optdesc opt_tcp_linger2;
extern const struct optdesc opt_tcp_defer_accept;
extern const struct optdesc opt_tcp_window_clamp;
extern const struct optdesc opt_tcp_info;
extern const struct optdesc opt_tcp_quickack;
extern const struct optdesc opt_tcp_noopt;
extern const struct optdesc opt_tcp_nopush;
extern const struct optdesc opt_tcp_md5sig;
extern const struct optdesc opt_tcp_sack_disable;
extern const struct optdesc opt_tcp_signature_enable;
extern const struct optdesc opt_tcp_abort_threshold;
extern const struct optdesc opt_tcp_conn_abort_threshold;
extern const struct optdesc opt_tcp_keepinit;
extern const struct optdesc opt_tcp_paws;
extern const struct optdesc opt_tcp_sackena;
extern const struct optdesc opt_tcp_tsoptena;

#endif /* !defined(__xio_tcp_h_included) */
