#ifndef _NETSNMP_FSYS_MOUNTS_H
#define _NETSNMP_FSYS_MOUNTS_H
/*
 *  Some mounts can map to one of two hrFS types
 *    (depending on other characteristics of the system)
 *  Note which should be used *before* defining any
 *    type tokens which may be missing.
 */

/*
 *  Ensure all recognised filesystem mount type tokens are
 *    available (even on systems where they're not used)
 */
#ifndef MOUNT_FFS
#define MOUNT_FFS    "ffs"
#endif
#ifndef MOUNT_NFS
#define MOUNT_NFS    "nfs"
#endif
#ifndef MOUNT_MFS
#define MOUNT_MFS    "mfs"
#endif
#ifndef MOUNT_MSDOS
#define MOUNT_MSDOS  "msdos"
#endif
#ifndef MOUNT_MSDOSFS
#define MOUNT_MSDOSFS "msdosfs"
#endif
#ifndef MOUNT_AFS
#define MOUNT_AFS    "afs"
#endif
#ifndef MOUNT_CD9660
#define MOUNT_CD9660 "cd9660"
#endif
#ifndef MOUNT_EXT2FS
#define MOUNT_EXT2FS "ext2fs"
#endif
#ifndef MOUNT_NTFS
#define MOUNT_NTFS   "ntfs"
#endif
#ifndef MOUNT_UFS
#define MOUNT_UFS    "ufs"
#endif
#ifndef MOUNT_ZFS
#define MOUNT_ZFS    "zfs"
#endif
#ifndef MOUNT_NVMFS
#define MOUNT_NVMFS  "nvmfs"
#endif
#ifndef MOUNT_ACFS
#define MOUNT_ACFS   "acfs"
#endif

#endif /* _NETSNMP_FSYS_MOUNTS_H */
