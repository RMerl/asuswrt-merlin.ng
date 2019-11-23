/* source: xio-fd.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_fd_h_included
#define __xio_fd_h_included 1

extern const struct optdesc opt_ioctl_void;
extern const struct optdesc opt_ioctl_int;
extern const struct optdesc opt_ioctl_intp;
extern const struct optdesc opt_ioctl_bin;
extern const struct optdesc opt_ioctl_string;
extern const struct optdesc opt_append;
extern const struct optdesc opt_nonblock;
extern const struct optdesc opt_o_ndelay;
extern const struct optdesc opt_async;
extern const struct optdesc opt_o_binary;
extern const struct optdesc opt_o_text;
extern const struct optdesc opt_o_noinherit;
extern const struct optdesc opt_cloexec;
extern const struct optdesc opt_ftruncate32;
extern const struct optdesc opt_ftruncate64;
extern const struct optdesc opt_group;
extern const struct optdesc opt_group_late;
extern const struct optdesc opt_perm;
extern const struct optdesc opt_perm_late;
extern const struct optdesc opt_user;
extern const struct optdesc opt_user_late;
extern const struct optdesc opt_lseek32_cur;
extern const struct optdesc opt_lseek32_end;
extern const struct optdesc opt_lseek32_set;
extern const struct optdesc opt_lseek64_cur;
extern const struct optdesc opt_lseek64_end;
extern const struct optdesc opt_lseek64_set;
extern const struct optdesc opt_flock_sh;
extern const struct optdesc opt_flock_sh_nb;
extern const struct optdesc opt_flock_ex;
extern const struct optdesc opt_flock_ex_nb;
extern const struct optdesc opt_f_setlk_rd;
extern const struct optdesc opt_f_setlkw_rd;
extern const struct optdesc opt_f_setlk_wr;
extern const struct optdesc opt_f_setlkw_wr;
extern const struct optdesc opt_cool_write;
extern const struct optdesc opt_end_close;
extern const struct optdesc opt_shut_none;
extern const struct optdesc opt_shut_down;
extern const struct optdesc opt_shut_close;
extern const struct optdesc opt_shut_null;
extern const struct optdesc opt_streams_i_push;

#endif /* !defined(__xio_fd_h_included) */
