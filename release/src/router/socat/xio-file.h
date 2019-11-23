/* source: xio-file.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_file_h_included
#define __xio_file_h_included 1

extern const struct optdesc opt_o_rdonly;
extern const struct optdesc opt_o_wronly;
extern const struct optdesc opt_o_rdwr;
extern const struct optdesc opt_o_create;
extern const struct optdesc opt_o_excl;
extern const struct optdesc opt_o_noctty;
extern const struct optdesc opt_o_sync;
extern const struct optdesc opt_o_nofollow;
extern const struct optdesc opt_o_directory;
extern const struct optdesc opt_o_largefile;
extern const struct optdesc opt_o_nshare;
extern const struct optdesc opt_o_rshare;
extern const struct optdesc opt_o_defer;
extern const struct optdesc opt_o_direct;
extern const struct optdesc opt_o_dsync;
extern const struct optdesc opt_o_rsync;
extern const struct optdesc opt_o_delay;
extern const struct optdesc opt_o_priv;
extern const struct optdesc opt_o_trunc;
extern const struct optdesc opt_o_noatime;

extern const struct addrdesc addr_open;

#endif /* !defined(__xio_file_h_included) */
