/* source: xio-pty.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_pty_h_included
#define __xio_pty_h_included 1

extern const struct addrdesc addr_pty;

extern const struct optdesc opt_symbolic_link;
#if HAVE_POLL
extern const struct optdesc opt_pty_wait_slave;
extern const struct optdesc opt_pty_intervall;
#endif /* HAVE_POLL */

#endif /* !defined(__xio_pty_h_included) */
