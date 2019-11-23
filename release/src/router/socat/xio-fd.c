/* source: xio-fd.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains common file descriptor related option definitions */

#include "xiosysincludes.h"
#include "xioopen.h"

#include "xio-fd.h"

/****** for ALL addresses - with open() or fcntl(F_SETFL) ******/
const struct optdesc opt_append    = { "append",    NULL, OPT_O_APPEND,    GROUP_OPEN|GROUP_FD, PH_LATE, TYPE_BOOL, OFUNC_FCNTL, F_SETFL, O_APPEND };
const struct optdesc opt_nonblock  = { "o-nonblock", "nonblock", OPT_O_NONBLOCK,  GROUP_OPEN|GROUP_FD, PH_FD, TYPE_BOOL, OFUNC_FCNTL, F_SETFL, O_NONBLOCK };
#if defined(O_NDELAY) && (!defined(O_NONBLOCK) || O_NDELAY != O_NONBLOCK)
const struct optdesc opt_o_ndelay  = { "o-ndelay",  NULL, OPT_O_NDELAY,  GROUP_OPEN|GROUP_FD, PH_LATE, TYPE_BOOL, OFUNC_FCNTL, F_SETFL, O_NDELAY };
#endif
#ifdef O_ASYNC
const struct optdesc opt_async     = { "async",     NULL, OPT_O_ASYNC,     GROUP_OPEN|GROUP_FD, PH_LATE, TYPE_BOOL, OFUNC_FCNTL, F_SETFL, O_ASYNC };
#endif
#ifdef O_BINARY
const struct optdesc opt_o_binary    = { "o-binary",    "binary",    OPT_O_BINARY,    GROUP_OPEN|GROUP_FD, PH_OPEN, TYPE_BOOL, OFUNC_FCNTL, F_SETFL, O_BINARY };
#endif
#ifdef O_TEXT
const struct optdesc opt_o_text      = { "o-text",      "text",      OPT_O_TEXT,      GROUP_OPEN|GROUP_FD, PH_OPEN, TYPE_BOOL, OFUNC_FCNTL, F_SETFL, O_TEXT };
#endif
#ifdef O_NOINHERIT
const struct optdesc opt_o_noinherit = { "o-noinherit", "noinherit", OPT_O_NOINHERIT, GROUP_OPEN|GROUP_FD, PH_OPEN, TYPE_BOOL, OFUNC_FCNTL, F_SETFL, O_NOINHERIT };
#endif
#ifdef O_NOATIME
const struct optdesc opt_o_noatime   = { "o-noatime",   "noatime",   OPT_O_NOATIME,   GROUP_OPEN|GROUP_FD, PH_FD, TYPE_BOOL, OFUNC_FCNTL, F_SETFL, O_NOATIME };
#endif
/****** for ALL addresses - with fcntl(F_SETFD) ******/
const struct optdesc opt_cloexec   = { "cloexec",   NULL, OPT_CLOEXEC,   GROUP_FD, PH_LATE, TYPE_BOOL, OFUNC_FCNTL, F_SETFD, FD_CLOEXEC };
/****** ftruncate() ******/
/* this record is good for ftruncate() or ftruncate64() if available */
#if HAVE_FTRUNCATE64
const struct optdesc opt_ftruncate32  = { "ftruncate32",  NULL,       OPT_FTRUNCATE32,  GROUP_REG, PH_LATE, TYPE_OFF32, OFUNC_SPEC };
const struct optdesc opt_ftruncate64  = { "ftruncate64",  "truncate", OPT_FTRUNCATE64,  GROUP_REG, PH_LATE, TYPE_OFF64, OFUNC_SPEC };
#else
const struct optdesc opt_ftruncate32  = { "ftruncate32",  "truncate", OPT_FTRUNCATE32,  GROUP_REG, PH_LATE, TYPE_OFF32, OFUNC_SPEC };
#endif /* !HAVE_FTRUNCATE64 */
/****** for ALL addresses - permissions, ownership, and positioning ******/
const struct optdesc opt_group     = { "group",     "gid",   OPT_GROUP,     GROUP_FD|GROUP_NAMED,PH_FD,TYPE_GIDT,OFUNC_SPEC };
const struct optdesc opt_group_late= { "group-late","gid-l", OPT_GROUP_LATE,GROUP_FD, PH_LATE,  TYPE_GIDT, OFUNC_SPEC };
const struct optdesc opt_perm      = { "perm",      "mode",  OPT_PERM,      GROUP_FD|GROUP_NAMED, PH_FD,    TYPE_MODET,OFUNC_SPEC };
const struct optdesc opt_perm_late = { "perm-late", NULL,    OPT_PERM_LATE, GROUP_FD, PH_LATE,  TYPE_MODET,OFUNC_SPEC };
const struct optdesc opt_user      = { "user",      "uid",   OPT_USER,      GROUP_FD|GROUP_NAMED, PH_FD,    TYPE_UIDT, OFUNC_SPEC };
const struct optdesc opt_user_late = { "user-late", "uid-l", OPT_USER_LATE, GROUP_FD, PH_LATE,  TYPE_UIDT, OFUNC_SPEC };
/* for something like random access files */
#if HAVE_LSEEK64
const struct optdesc opt_lseek32_cur  = { "lseek32-cur",  NULL,       OPT_SEEK32_CUR,  GROUP_REG|GROUP_BLK, PH_LATE,  TYPE_OFF32, OFUNC_SEEK32, SEEK_CUR };
const struct optdesc opt_lseek32_end  = { "lseek32-end",  NULL,       OPT_SEEK32_END,  GROUP_REG|GROUP_BLK, PH_LATE,  TYPE_OFF32, OFUNC_SEEK32, SEEK_END };
const struct optdesc opt_lseek32_set  = { "lseek32-set",  NULL,       OPT_SEEK32_SET,  GROUP_REG|GROUP_BLK, PH_LATE,  TYPE_OFF32, OFUNC_SEEK32, SEEK_SET };
const struct optdesc opt_lseek64_cur  = { "lseek64-cur",  "seek-cur", OPT_SEEK64_CUR,  GROUP_REG|GROUP_BLK, PH_LATE,  TYPE_OFF64, OFUNC_SEEK64, SEEK_CUR };
const struct optdesc opt_lseek64_end  = { "lseek64-end",  "seek-end", OPT_SEEK64_END,  GROUP_REG|GROUP_BLK, PH_LATE,  TYPE_OFF64, OFUNC_SEEK64, SEEK_END };
const struct optdesc opt_lseek64_set  = { "lseek64-set",  "seek",     OPT_SEEK64_SET,  GROUP_REG|GROUP_BLK, PH_LATE,  TYPE_OFF64, OFUNC_SEEK64, SEEK_SET };
#else
const struct optdesc opt_lseek32_cur  = { "lseek32-cur",  "seek-cur", OPT_SEEK32_CUR,  GROUP_REG|GROUP_BLK, PH_LATE,  TYPE_OFF32, OFUNC_SEEK32, SEEK_CUR };
const struct optdesc opt_lseek32_end  = { "lseek32-end",  "seek-end", OPT_SEEK32_END,  GROUP_REG|GROUP_BLK, PH_LATE,  TYPE_OFF32, OFUNC_SEEK32, SEEK_END };
const struct optdesc opt_lseek32_set  = { "lseek32-set",  "seek",     OPT_SEEK32_SET,  GROUP_REG|GROUP_BLK, PH_LATE,  TYPE_OFF32, OFUNC_SEEK32, SEEK_SET };
#endif /* !HAVE_LSEEK64 */
/* for all addresses (?) */
const struct optdesc opt_f_setlk_rd   = { "f-setlk-rd",   "setlk-rd", OPT_F_SETLK_RD,   GROUP_FD, PH_FD,TYPE_BOOL, OFUNC_SPEC, F_SETLK, F_RDLCK };
const struct optdesc opt_f_setlkw_rd  = { "f-setlkw-rd",  "setlkw-rd",OPT_F_SETLKW_RD,  GROUP_FD, PH_FD,TYPE_BOOL, OFUNC_SPEC, F_SETLKW, F_RDLCK };
const struct optdesc opt_f_setlk_wr   = { "f-setlk-wr",   "setlk",    OPT_F_SETLK_WR,   GROUP_FD, PH_FD,TYPE_BOOL, OFUNC_SPEC, F_SETLK, F_WRLCK };
const struct optdesc opt_f_setlkw_wr  = { "f-setlkw-wr",  "setlkw",   OPT_F_SETLKW_WR,  GROUP_FD, PH_FD,TYPE_BOOL, OFUNC_SPEC, F_SETLKW, F_WRLCK };
#if HAVE_FLOCK
const struct optdesc opt_flock_sh     = { "flock-sh",    NULL,    OPT_FLOCK_SH,    GROUP_FD, PH_FD,TYPE_BOOL, OFUNC_FLOCK, LOCK_SH };
const struct optdesc opt_flock_sh_nb  = { "flock-sh-nb", NULL,    OPT_FLOCK_SH_NB, GROUP_FD, PH_FD,TYPE_BOOL, OFUNC_FLOCK, LOCK_SH|LOCK_NB };
const struct optdesc opt_flock_ex     = { "flock-ex",    "flock", OPT_FLOCK_EX,    GROUP_FD, PH_FD,TYPE_BOOL, OFUNC_FLOCK, LOCK_EX };
const struct optdesc opt_flock_ex_nb  = { "flock-ex-nb", "flock-nb", OPT_FLOCK_EX_NB, GROUP_FD, PH_FD,TYPE_BOOL, OFUNC_FLOCK, LOCK_EX|LOCK_NB };
#endif /* HAVE_FLOCK */
const struct optdesc opt_cool_write = { "cool-write", "coolwrite", OPT_COOL_WRITE, GROUP_FD, PH_INIT, TYPE_BOOL, OFUNC_OFFSET, XIO_OFFSETOF(cool_write) };

/* control closing of connections */
const struct optdesc opt_end_close = { "end-close", "close", OPT_END_CLOSE,  GROUP_FD, PH_INIT, TYPE_CONST,  OFUNC_OFFSET, XIO_OFFSETOF(howtoend),  END_CLOSE };
const struct optdesc opt_shut_none = { "shut-none",  NULL,   OPT_SHUT_NONE,  GROUP_FD, PH_INIT, TYPE_CONST, OFUNC_OFFSET, XIO_OFFSETOF(howtoshut), XIOSHUT_NONE };
const struct optdesc opt_shut_down = { "shut-down",  NULL,   OPT_SHUT_DOWN,  GROUP_FD, PH_INIT, TYPE_CONST, OFUNC_OFFSET, XIO_OFFSETOF(howtoshut), XIOSHUT_DOWN };
const struct optdesc opt_shut_close= { "shut-close", NULL,   OPT_SHUT_CLOSE, GROUP_FD, PH_INIT, TYPE_CONST, OFUNC_OFFSET, XIO_OFFSETOF(howtoshut), XIOSHUT_CLOSE };
const struct optdesc opt_shut_null = { "shut-null",  NULL,   OPT_SHUT_NULL,  GROUP_FD, PH_INIT, TYPE_CONST, OFUNC_OFFSET, XIO_OFFSETOF(howtoshut), XIOSHUT_NULL };

/****** generic ioctl() options ******/
const struct optdesc opt_ioctl_void   = { "ioctl-void",  "ioctl",    OPT_IOCTL_VOID,  GROUP_FD, PH_FD, TYPE_INT,       OFUNC_IOCTL_GENERIC, 0, 0, 0 };
const struct optdesc opt_ioctl_int    = { "ioctl-int",   NULL,       OPT_IOCTL_INT,   GROUP_FD, PH_FD, TYPE_INT_INT,   OFUNC_IOCTL_GENERIC, 0, 0, 0 };
const struct optdesc opt_ioctl_intp   = { "ioctl-intp",  NULL,       OPT_IOCTL_INTP,  GROUP_FD, PH_FD, TYPE_INT_INTP,  OFUNC_IOCTL_GENERIC, 0, 0, 0 };
const struct optdesc opt_ioctl_bin    = { "ioctl-bin",   NULL,       OPT_IOCTL_BIN,   GROUP_FD, PH_FD, TYPE_INT_BIN,   OFUNC_IOCTL_GENERIC, 0, 0, 0 };
const struct optdesc opt_ioctl_string = { "ioctl-string",NULL,       OPT_IOCTL_STRING,GROUP_FD, PH_FD, TYPE_INT_STRING,OFUNC_IOCTL_GENERIC, 0, 0, 0 };

/* POSIX STREAMS */
#define ENABLE_OPTIONS
#include "xio-streams.c"
#undef ENABLE_OPTIONS
