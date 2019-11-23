/* source: xio-ext2.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for handling Linux ext2fs options
   they can also be set with chattr(1) and viewed with lsattr(1) */

#include "xiosysincludes.h"
#include "xioopen.h"

#include "xio-ext2.h"


#if WITH_EXT2

/****** FD options ******/

#ifdef EXT2_SECRM_FL
/* secure deletion, chattr 's' */
const struct optdesc opt_ext2_secrm        = { "ext2-secrm",        "secrm",        OPT_EXT2_SECRM,        GROUP_REG, PH_FD, TYPE_BOOL, OFUNC_IOCTL_MASK_LONG, EXT2_IOC_GETFLAGS, EXT2_IOC_SETFLAGS, EXT2_SECRM_FL };
#endif /* EXT2_SECRM_FL */

#ifdef EXT2_UNRM_FL
/* undelete, chattr 'u' */
const struct optdesc opt_ext2_unrm         = { "ext2-unrm",         "unrm",         OPT_EXT2_UNRM,         GROUP_REG, PH_FD, TYPE_BOOL, OFUNC_IOCTL_MASK_LONG, EXT2_IOC_GETFLAGS, EXT2_IOC_SETFLAGS, EXT2_UNRM_FL };
#endif /* EXT2_UNRM_FL */

#ifdef EXT2_COMPR_FL
/* compress file, chattr 'c' */
const struct optdesc opt_ext2_compr        = { "ext2-compr",        "compr",        OPT_EXT2_COMPR,        GROUP_REG, PH_FD, TYPE_BOOL, OFUNC_IOCTL_MASK_LONG, EXT2_IOC_GETFLAGS, EXT2_IOC_SETFLAGS, EXT2_COMPR_FL };
#endif /* EXT2_COMPR_FL */

#ifdef EXT2_SYNC_FL
/* synchronous update, chattr 'S' */
const struct optdesc opt_ext2_sync         = { "ext2-sync",         "sync",         OPT_EXT2_SYNC,         GROUP_REG, PH_FD, TYPE_BOOL, OFUNC_IOCTL_MASK_LONG, EXT2_IOC_GETFLAGS, EXT2_IOC_SETFLAGS, EXT2_SYNC_FL };
#endif /* EXT2_SYNC_FL */

#ifdef EXT2_IMMUTABLE_FL
/* immutable file, chattr 'i' */
const struct optdesc opt_ext2_immutable    = { "ext2-immutable",    "immutable",    OPT_EXT2_IMMUTABLE,    GROUP_REG, PH_FD, TYPE_BOOL, OFUNC_IOCTL_MASK_LONG, EXT2_IOC_GETFLAGS, EXT2_IOC_SETFLAGS, EXT2_IMMUTABLE_FL };
#endif /* EXT2_IMMUTABLE_FL */

#ifdef EXT2_APPEND_FL
/* writes to file may only append, chattr 'a' */
const struct optdesc opt_ext2_append       = { "ext2-append",       "append",       OPT_EXT2_APPEND,       GROUP_REG, PH_FD, TYPE_BOOL, OFUNC_IOCTL_MASK_LONG, EXT2_IOC_GETFLAGS, EXT2_IOC_SETFLAGS, EXT2_APPEND_FL };
#endif /* EXT2_APPEND_FL */

#ifdef EXT2_NODUMP_FL
/* do not dump file, chattr 'd' */
const struct optdesc opt_ext2_nodump       = { "ext2-nodump",       "nodump",       OPT_EXT2_NODUMP,       GROUP_REG, PH_FD, TYPE_BOOL, OFUNC_IOCTL_MASK_LONG, EXT2_IOC_GETFLAGS, EXT2_IOC_SETFLAGS, EXT2_NODUMP_FL };
#endif /* EXT2_NODUMP_FL */

#ifdef EXT2_NOATIME_FL
/* do not update atime, chattr 'A' */
const struct optdesc opt_ext2_noatime      = { "ext2-noatime",      "noatime",      OPT_EXT2_NOATIME,      GROUP_REG, PH_FD, TYPE_BOOL, OFUNC_IOCTL_MASK_LONG, EXT2_IOC_GETFLAGS, EXT2_IOC_SETFLAGS, EXT2_NOATIME_FL };
#endif /* EXT2_NOATIME_FL */

/* EXT2_DIRTY_FL ??? */
/* EXT2_COMPRBLK_FL one ore more compress clusters */
/* EXT2_NOCOMPR_FL access raw compressed data */
/* EXT2_ECOMPR_FL compression error */
/* EXT2_BTREE_FL btree format dir */
/* EXT2_INDEX_FL hash indexed directory */
/* EXT2_IMAGIC ??? */

#ifdef EXT2_JOURNAL_DATA_FL
/* file data should be journaled, chattr 'j' */
const struct optdesc opt_ext2_journal_data = { "ext2-journal-data", "journal-data", OPT_EXT2_JOURNAL_DATA, GROUP_REG, PH_FD, TYPE_BOOL, OFUNC_IOCTL_MASK_LONG, EXT2_IOC_GETFLAGS, EXT2_IOC_SETFLAGS, EXT2_JOURNAL_DATA_FL };
#endif /* EXT2_JOURNAL_DATA_FL */

#ifdef EXT2_NOTAIL_FL
/* file tail should not be merged, chattr 't' */
const struct optdesc opt_ext2_notail       = { "ext2-notail",       "notail",       OPT_EXT2_NOTAIL,       GROUP_REG, PH_FD, TYPE_BOOL, OFUNC_IOCTL_MASK_LONG, EXT2_IOC_GETFLAGS, EXT2_IOC_SETFLAGS, EXT2_NOTAIL_FL };
#endif /* EXT2_NOTAIL_FL */

#ifdef EXT2_DIRSYNC_FL
/* synchronous directory modifications, chattr 'D' */
const struct optdesc opt_ext2_dirsync      = { "ext2-dirsync",      "dirsync",      OPT_EXT2_DIRSYNC,      GROUP_REG, PH_FD, TYPE_BOOL, OFUNC_IOCTL_MASK_LONG, EXT2_IOC_GETFLAGS, EXT2_IOC_SETFLAGS, EXT2_DIRSYNC_FL };
#endif /* EXT2_DIRSYNC_FL */

#ifdef EXT2_TOPDIR_FL
/* top of directory hierarchies, chattr 'T' */
const struct optdesc opt_ext2_topdir       = { "ext2-topdir",       "topdir",       OPT_EXT2_TOPDIR,       GROUP_REG, PH_FD, TYPE_BOOL, OFUNC_IOCTL_MASK_LONG, EXT2_IOC_GETFLAGS, EXT2_IOC_SETFLAGS, EXT2_TOPDIR_FL };
#endif /* EXT2_TOPDIR_FL */

/* EXTENTS inode uses extents */


#endif /* WITH_EXT2 */


