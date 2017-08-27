/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* GIO - GLib Input, Output and Streaming Library
 * 
 * Copyright (C) 2006-2007 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Alexander Larsson <alexl@redhat.com>
 */

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#ifndef HAVE_SYSCTLBYNAME
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif
#endif
#ifdef HAVE_POLL_H
#include <poll.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <gstdio.h>
#include <dirent.h>

#if HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif
#if HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#if HAVE_SYS_VFS_H
#include <sys/vfs.h>
#elif HAVE_SYS_MOUNT_H
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/mount.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#include "gunixmounts.h"
#include "gfile.h"
#include "gfilemonitor.h"
#include "glibintl.h"
#include "gthemedicon.h"


#ifdef HAVE_MNTENT_H
static const char *_resolve_dev_root (void);
#endif

/**
 * SECTION:gunixmounts
 * @include: gio/gunixmounts.h
 * @short_description: UNIX mounts
 *
 * Routines for managing mounted UNIX mount points and paths.
 *
 * Note that <filename>&lt;gio/gunixmounts.h&gt;</filename> belongs to the
 * UNIX-specific GIO interfaces, thus you have to use the
 * <filename>gio-unix-2.0.pc</filename> pkg-config file when using it.
 */

/*
 * GUnixMountType:
 * @G_UNIX_MOUNT_TYPE_UNKNOWN: Unknown UNIX mount type.
 * @G_UNIX_MOUNT_TYPE_FLOPPY: Floppy disk UNIX mount type.
 * @G_UNIX_MOUNT_TYPE_CDROM: CDROM UNIX mount type.
 * @G_UNIX_MOUNT_TYPE_NFS: Network File System (NFS) UNIX mount type.
 * @G_UNIX_MOUNT_TYPE_ZIP: ZIP UNIX mount type.
 * @G_UNIX_MOUNT_TYPE_JAZ: JAZZ UNIX mount type.
 * @G_UNIX_MOUNT_TYPE_MEMSTICK: Memory Stick UNIX mount type.
 * @G_UNIX_MOUNT_TYPE_CF: Compact Flash UNIX mount type.
 * @G_UNIX_MOUNT_TYPE_SM: Smart Media UNIX mount type.
 * @G_UNIX_MOUNT_TYPE_SDMMC: SD/MMC UNIX mount type.
 * @G_UNIX_MOUNT_TYPE_IPOD: iPod UNIX mount type.
 * @G_UNIX_MOUNT_TYPE_CAMERA: Digital camera UNIX mount type.
 * @G_UNIX_MOUNT_TYPE_HD: Hard drive UNIX mount type.
 * 
 * Types of UNIX mounts.
 **/
typedef enum {
  G_UNIX_MOUNT_TYPE_UNKNOWN,
  G_UNIX_MOUNT_TYPE_FLOPPY,
  G_UNIX_MOUNT_TYPE_CDROM,
  G_UNIX_MOUNT_TYPE_NFS,
  G_UNIX_MOUNT_TYPE_ZIP,
  G_UNIX_MOUNT_TYPE_JAZ,
  G_UNIX_MOUNT_TYPE_MEMSTICK,
  G_UNIX_MOUNT_TYPE_CF,
  G_UNIX_MOUNT_TYPE_SM,
  G_UNIX_MOUNT_TYPE_SDMMC,
  G_UNIX_MOUNT_TYPE_IPOD,
  G_UNIX_MOUNT_TYPE_CAMERA,
  G_UNIX_MOUNT_TYPE_HD
} GUnixMountType;

struct _GUnixMountEntry {
  char *mount_path;
  char *device_path;
  char *filesystem_type;
  gboolean is_read_only;
  gboolean is_system_internal;
};

struct _GUnixMountPoint {
  char *mount_path;
  char *device_path;
  char *filesystem_type;
  char *options;
  gboolean is_read_only;
  gboolean is_user_mountable;
  gboolean is_loopback;
};

enum {
  MOUNTS_CHANGED,
  MOUNTPOINTS_CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

struct _GUnixMountMonitor {
  GObject parent;

  GFileMonitor *fstab_monitor;
  GFileMonitor *mtab_monitor;

  GSource *proc_mounts_watch_source;
};

struct _GUnixMountMonitorClass {
  GObjectClass parent_class;
};
  
static GUnixMountMonitor *the_mount_monitor = NULL;

static GList *_g_get_unix_mounts (void);
static GList *_g_get_unix_mount_points (void);

G_DEFINE_TYPE (GUnixMountMonitor, g_unix_mount_monitor, G_TYPE_OBJECT);

#define MOUNT_POLL_INTERVAL 4000

#ifdef HAVE_SYS_MNTTAB_H
#define MNTOPT_RO	"ro"
#endif

#ifdef HAVE_MNTENT_H
#include <mntent.h>
#elif defined (HAVE_SYS_MNTTAB_H)
#include <sys/mnttab.h>
#endif

#ifdef HAVE_SYS_VFSTAB_H
#include <sys/vfstab.h>
#endif

#if defined(HAVE_SYS_MNTCTL_H) && defined(HAVE_SYS_VMOUNT_H) && defined(HAVE_SYS_VFS_H)
#include <sys/mntctl.h>
#include <sys/vfs.h>
#include <sys/vmount.h>
#include <fshelp.h>
#endif

#if (defined(HAVE_GETVFSSTAT) || defined(HAVE_GETFSSTAT)) && defined(HAVE_FSTAB_H) && defined(HAVE_SYS_MOUNT_H)
#include <sys/ucred.h>
#include <sys/mount.h>
#include <fstab.h>
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#endif

#ifndef HAVE_SETMNTENT
#define setmntent(f,m) fopen(f,m)
#endif
#ifndef HAVE_ENDMNTENT
#define endmntent(f) fclose(f)
#endif

static gboolean
is_in (const char *value, const char *set[])
{
  int i;
  for (i = 0; set[i] != NULL; i++)
    {
      if (strcmp (set[i], value) == 0)
	return TRUE;
    }
  return FALSE;
}

/**
 * g_unix_is_mount_path_system_internal:
 * @mount_path: a mount path, e.g. <filename>/media/disk</filename> 
 *    or <filename>/usr</filename>
 *
 * Determines if @mount_path is considered an implementation of the
 * OS. This is primarily used for hiding mountable and mounted volumes
 * that only are used in the OS and has little to no relevance to the
 * casual user.
 *
 * Returns: %TRUE if @mount_path is considered an implementation detail 
 *     of the OS.
 **/
gboolean
g_unix_is_mount_path_system_internal (const char *mount_path)
{
  const char *ignore_mountpoints[] = {
    /* Includes all FHS 2.3 toplevel dirs and other specilized
     * directories that we want to hide from the user.
     */
    "/",              /* we already have "Filesystem root" in Nautilus */ 
    "/bin",
    "/boot",
    "/dev",
    "/etc",
    "/home",
    "/lib",
    "/lib64",
    "/live/cow",
    "/live/image",
    "/media",
    "/mnt",
    "/opt",
    "/root",
    "/sbin",
    "/srv",
    "/tmp",
    "/usr",
    "/usr/local",
    "/var",
    "/var/crash",
    "/var/local",
    "/var/log",
    "/var/log/audit", /* https://bugzilla.redhat.com/show_bug.cgi?id=333041 */
    "/var/mail",
    "/var/run",
    "/var/tmp",       /* https://bugzilla.redhat.com/show_bug.cgi?id=335241 */
    "/proc",
    "/sbin",
    "/net",
    "/sys",
    NULL
  };

  if (is_in (mount_path, ignore_mountpoints))
    return TRUE;
  
  if (g_str_has_prefix (mount_path, "/dev/") ||
      g_str_has_prefix (mount_path, "/proc/") ||
      g_str_has_prefix (mount_path, "/sys/"))
    return TRUE;

  if (g_str_has_suffix (mount_path, "/.gvfs"))
    return TRUE;

  return FALSE;
}

static gboolean
guess_system_internal (const char *mountpoint,
		       const char *fs,
		       const char *device)
{
  const char *ignore_fs[] = {
    "auto",
    "autofs",
    "devfs",
    "devpts",
    "ecryptfs",
    "kernfs",
    "linprocfs",
    "proc",
    "procfs",
    "ptyfs",
    "rootfs",
    "selinuxfs",
    "sysfs",
    "tmpfs",
    "usbfs",
    "nfsd",
    "rpc_pipefs",
    "zfs",
    NULL
  };
  const char *ignore_devices[] = {
    "none",
    "sunrpc",
    "devpts",
    "nfsd",
    "/dev/loop",
    "/dev/vn",
    NULL
  };
  
  if (is_in (fs, ignore_fs))
    return TRUE;
  
  if (is_in (device, ignore_devices))
    return TRUE;

  if (g_unix_is_mount_path_system_internal (mountpoint))
    return TRUE;
  
  return FALSE;
}

#ifdef HAVE_MNTENT_H

static char *
get_mtab_read_file (void)
{
#ifdef _PATH_MOUNTED
# ifdef __linux__
  return "/proc/mounts";
# else
  return _PATH_MOUNTED;
# endif
#else	
  return "/etc/mtab";
#endif
}

static char *
get_mtab_monitor_file (void)
{
#ifdef _PATH_MOUNTED
# ifdef __linux__
  return "/proc/mounts";
# else
  return _PATH_MOUNTED;
# endif
#else	
  return "/etc/mtab";
#endif
}

#ifndef HAVE_GETMNTENT_R
G_LOCK_DEFINE_STATIC(getmntent);
#endif

static GList *
_g_get_unix_mounts (void)
{
#ifdef HAVE_GETMNTENT_R
  struct mntent ent;
  char buf[1024];
#endif
  struct mntent *mntent;
  FILE *file;
  char *read_file;
  GUnixMountEntry *mount_entry;
  GHashTable *mounts_hash;
  GList *return_list;
  
  read_file = get_mtab_read_file ();

  file = setmntent (read_file, "r");
  if (file == NULL)
    return NULL;

  return_list = NULL;
  
  mounts_hash = g_hash_table_new (g_str_hash, g_str_equal);
  
#ifdef HAVE_GETMNTENT_R
  while ((mntent = getmntent_r (file, &ent, buf, sizeof (buf))) != NULL)
#else
  G_LOCK (getmntent);
  while ((mntent = getmntent (file)) != NULL)
#endif
    {
      /* ignore any mnt_fsname that is repeated and begins with a '/'
       *
       * We do this to avoid being fooled by --bind mounts, since
       * these have the same device as the location they bind to.
       * It's not an ideal solution to the problem, but it's likely that
       * the most important mountpoint is first and the --bind ones after
       * that aren't as important. So it should work.
       *
       * The '/' is to handle procfs, tmpfs and other no device mounts.
       */
      if (mntent->mnt_fsname != NULL &&
	  mntent->mnt_fsname[0] == '/' &&
	  g_hash_table_lookup (mounts_hash, mntent->mnt_fsname))
        continue;
      
      mount_entry = g_new0 (GUnixMountEntry, 1);
      mount_entry->mount_path = g_strdup (mntent->mnt_dir);
      if (strcmp (mntent->mnt_fsname, "/dev/root") == 0)
        mount_entry->device_path = g_strdup (_resolve_dev_root ());
      else
        mount_entry->device_path = g_strdup (mntent->mnt_fsname);
      mount_entry->filesystem_type = g_strdup (mntent->mnt_type);
      
#if defined (HAVE_HASMNTOPT)
      if (hasmntopt (mntent, MNTOPT_RO) != NULL)
	mount_entry->is_read_only = TRUE;
#endif
      
      mount_entry->is_system_internal =
	guess_system_internal (mount_entry->mount_path,
			       mount_entry->filesystem_type,
			       mount_entry->device_path);
      
      g_hash_table_insert (mounts_hash,
			   mount_entry->device_path,
			   mount_entry->device_path);
      
      return_list = g_list_prepend (return_list, mount_entry);
    }
  g_hash_table_destroy (mounts_hash);
  
  endmntent (file);

#ifndef HAVE_GETMNTENT_R
  G_UNLOCK (getmntent);
#endif
  
  return g_list_reverse (return_list);
}

#elif defined (HAVE_SYS_MNTTAB_H)

G_LOCK_DEFINE_STATIC(getmntent);

static char *
get_mtab_read_file (void)
{
#ifdef _PATH_MOUNTED
  return _PATH_MOUNTED;
#else	
  return "/etc/mnttab";
#endif
}

static char *
get_mtab_monitor_file (void)
{
  return get_mtab_read_file ();
}

static GList *
_g_get_unix_mounts (void)
{
  struct mnttab mntent;
  FILE *file;
  char *read_file;
  GUnixMountEntry *mount_entry;
  GList *return_list;
  
  read_file = get_mtab_read_file ();
  
  file = setmntent (read_file, "r");
  if (file == NULL)
    return NULL;
  
  return_list = NULL;
  
  G_LOCK (getmntent);
  while (! getmntent (file, &mntent))
    {
      mount_entry = g_new0 (GUnixMountEntry, 1);
      
      mount_entry->mount_path = g_strdup (mntent.mnt_mountp);
      mount_entry->device_path = g_strdup (mntent.mnt_special);
      mount_entry->filesystem_type = g_strdup (mntent.mnt_fstype);
      
#if defined (HAVE_HASMNTOPT)
      if (hasmntopt (&mntent, MNTOPT_RO) != NULL)
	mount_entry->is_read_only = TRUE;
#endif

      mount_entry->is_system_internal =
	guess_system_internal (mount_entry->mount_path,
			       mount_entry->filesystem_type,
			       mount_entry->device_path);
      
      return_list = g_list_prepend (return_list, mount_entry);
    }
  
  endmntent (file);
  
  G_UNLOCK (getmntent);
  
  return g_list_reverse (return_list);
}

#elif defined(HAVE_SYS_MNTCTL_H) && defined(HAVE_SYS_VMOUNT_H) && defined(HAVE_SYS_VFS_H)

static char *
get_mtab_monitor_file (void)
{
  return NULL;
}

static GList *
_g_get_unix_mounts (void)
{
  struct vfs_ent *fs_info;
  struct vmount *vmount_info;
  int vmount_number;
  unsigned int vmount_size;
  int current;
  GList *return_list;
  
  if (mntctl (MCTL_QUERY, sizeof (vmount_size), &vmount_size) != 0)
    {
      g_warning ("Unable to know the number of mounted volumes\n");
      
      return NULL;
    }

  vmount_info = (struct vmount*)g_malloc (vmount_size);

  vmount_number = mntctl (MCTL_QUERY, vmount_size, vmount_info);
  
  if (vmount_info->vmt_revision != VMT_REVISION)
    g_warning ("Bad vmount structure revision number, want %d, got %d\n", VMT_REVISION, vmount_info->vmt_revision);

  if (vmount_number < 0)
    {
      g_warning ("Unable to recover mounted volumes information\n");
      
      g_free (vmount_info);
      return NULL;
    }
  
  return_list = NULL;
  while (vmount_number > 0)
    {
      mount_entry = g_new0 (GUnixMountEntry, 1);
      
      mount_entry->device_path = g_strdup (vmt2dataptr (vmount_info, VMT_OBJECT));
      mount_entry->mount_path = g_strdup (vmt2dataptr (vmount_info, VMT_STUB));
      /* is_removable = (vmount_info->vmt_flags & MNT_REMOVABLE) ? 1 : 0; */
      mount_entry->is_read_only = (vmount_info->vmt_flags & MNT_READONLY) ? 1 : 0;

      fs_info = getvfsbytype (vmount_info->vmt_gfstype);
      
      if (fs_info == NULL)
	mount_entry->filesystem_type = g_strdup ("unknown");
      else
	mount_entry->filesystem_type = g_strdup (fs_info->vfsent_name);

      mount_entry->is_system_internal =
	guess_system_internal (mount_entry->mount_path,
			       mount_entry->filesystem_type,
			       mount_entry->device_path);
      
      return_list = g_list_prepend (return_list, mount_entry);
      
      vmount_info = (struct vmount *)( (char*)vmount_info 
				       + vmount_info->vmt_length);
      vmount_number--;
    }
  
  g_free (vmount_info);
  
  return g_list_reverse (return_list);
}

#elif (defined(HAVE_GETVFSSTAT) || defined(HAVE_GETFSSTAT)) && defined(HAVE_FSTAB_H) && defined(HAVE_SYS_MOUNT_H)

static char *
get_mtab_monitor_file (void)
{
  return NULL;
}

static GList *
_g_get_unix_mounts (void)
{
#if defined(HAVE_GETVFSSTAT)
  struct statvfs *mntent = NULL;
#elif defined(HAVE_GETFSSTAT)
  struct statfs *mntent = NULL;
#else
  #error statfs juggling failed
#endif
  size_t bufsize;
  int num_mounts, i;
  GUnixMountEntry *mount_entry;
  GList *return_list;
  
  /* Pass NOWAIT to avoid blocking trying to update NFS mounts. */
#if defined(HAVE_GETVFSSTAT)
  num_mounts = getvfsstat (NULL, 0, ST_NOWAIT);
#elif defined(HAVE_GETFSSTAT)
  num_mounts = getfsstat (NULL, 0, MNT_NOWAIT);
#endif
  if (num_mounts == -1)
    return NULL;

  bufsize = num_mounts * sizeof (*mntent);
  mntent = g_malloc (bufsize);
#if defined(HAVE_GETVFSSTAT)
  num_mounts = getvfsstat (mntent, bufsize, ST_NOWAIT);
#elif defined(HAVE_GETFSSTAT)
  num_mounts = getfsstat (mntent, bufsize, MNT_NOWAIT);
#endif
  if (num_mounts == -1)
    return NULL;
  
  return_list = NULL;
  
  for (i = 0; i < num_mounts; i++)
    {
      mount_entry = g_new0 (GUnixMountEntry, 1);
      
      mount_entry->mount_path = g_strdup (mntent[i].f_mntonname);
      mount_entry->device_path = g_strdup (mntent[i].f_mntfromname);
      mount_entry->filesystem_type = g_strdup (mntent[i].f_fstypename);
#if defined(HAVE_GETVFSSTAT)
      if (mntent[i].f_flag & ST_RDONLY)
#elif defined(HAVE_GETFSSTAT)
      if (mntent[i].f_flags & MNT_RDONLY)
#endif
        mount_entry->is_read_only = TRUE;

      mount_entry->is_system_internal =
        guess_system_internal (mount_entry->mount_path,
                               mount_entry->filesystem_type,
                               mount_entry->device_path);
      
      return_list = g_list_prepend (return_list, mount_entry);
    }

  g_free (mntent);
  
  return g_list_reverse (return_list);
}
#elif defined(__INTERIX)

static char *
get_mtab_monitor_file (void)
{
  return NULL;
}

static GList *
_g_get_unix_mounts (void)
{
  DIR *dirp;
  GList* return_list = NULL;
  char filename[9 + NAME_MAX];

  dirp = opendir ("/dev/fs");
  if (!dirp)
    {
      g_warning ("unable to read /dev/fs!");
      return NULL;
    }

  while (1)
    {
      struct statvfs statbuf;
      struct dirent entry;
      struct dirent* result;
      
      if (readdir_r (dirp, &entry, &result) || result == NULL)
        break;
      
      strcpy (filename, "/dev/fs/");
      strcat (filename, entry.d_name);
      
      if (statvfs (filename, &statbuf) == 0)
        {
          GUnixMountEntry* mount_entry = g_new0(GUnixMountEntry, 1);
          
          mount_entry->mount_path = g_strdup (statbuf.f_mntonname);
          mount_entry->device_path = g_strdup (statbuf.f_mntfromname);
          mount_entry->filesystem_type = g_strdup (statbuf.f_fstypename);
          
          if (statbuf.f_flag & ST_RDONLY)
            mount_entry->is_read_only = TRUE;
          
          return_list = g_list_prepend(return_list, mount_entry);
        }
    }
  
  return_list = g_list_reverse (return_list);
  
  closedir (dirp);

  return return_list;
}
#else
#error No _g_get_unix_mounts() implementation for system
#endif

/* _g_get_unix_mount_points():
 * read the fstab.
 * don't return swap and ignore mounts.
 */

static char *
get_fstab_file (void)
{
#if defined(HAVE_SYS_MNTCTL_H) && defined(HAVE_SYS_VMOUNT_H) && defined(HAVE_SYS_VFS_H)
  /* AIX */
  return "/etc/filesystems";
#elif defined(_PATH_MNTTAB)
  return _PATH_MNTTAB;
#elif defined(VFSTAB)
  return VFSTAB;
#else
  return "/etc/fstab";
#endif
}

#ifdef HAVE_MNTENT_H
static GList *
_g_get_unix_mount_points (void)
{
#ifdef HAVE_GETMNTENT_R
  struct mntent ent;
  char buf[1024];
#endif
  struct mntent *mntent;
  FILE *file;
  char *read_file;
  GUnixMountPoint *mount_entry;
  GList *return_list;
  
  read_file = get_fstab_file ();
  
  file = setmntent (read_file, "r");
  if (file == NULL)
    return NULL;

  return_list = NULL;
  
#ifdef HAVE_GETMNTENT_R
  while ((mntent = getmntent_r (file, &ent, buf, sizeof (buf))) != NULL)
#else
  G_LOCK (getmntent);
  while ((mntent = getmntent (file)) != NULL)
#endif
    {
      if ((strcmp (mntent->mnt_dir, "ignore") == 0) ||
          (strcmp (mntent->mnt_dir, "swap") == 0) ||
          (strcmp (mntent->mnt_dir, "none") == 0))
	continue;

#ifdef HAVE_HASMNTOPT
      /* We ignore bind fstab entries, as we ignore bind mounts anyway */
      if (hasmntopt (mntent, "bind"))
        continue;
#endif

      mount_entry = g_new0 (GUnixMountPoint, 1);
      mount_entry->mount_path = g_strdup (mntent->mnt_dir);
      if (strcmp (mntent->mnt_fsname, "/dev/root") == 0)
        mount_entry->device_path = g_strdup (_resolve_dev_root ());
      else
        mount_entry->device_path = g_strdup (mntent->mnt_fsname);
      mount_entry->filesystem_type = g_strdup (mntent->mnt_type);
      mount_entry->options = g_strdup (mntent->mnt_opts);
      
#ifdef HAVE_HASMNTOPT
      if (hasmntopt (mntent, MNTOPT_RO) != NULL)
	mount_entry->is_read_only = TRUE;
      
      if (hasmntopt (mntent, "loop") != NULL)
	mount_entry->is_loopback = TRUE;
      
#endif
      
      if ((mntent->mnt_type != NULL && strcmp ("supermount", mntent->mnt_type) == 0)
#ifdef HAVE_HASMNTOPT
	  || (hasmntopt (mntent, "user") != NULL
	      && hasmntopt (mntent, "user") != hasmntopt (mntent, "user_xattr"))
	  || hasmntopt (mntent, "pamconsole") != NULL
	  || hasmntopt (mntent, "users") != NULL
	  || hasmntopt (mntent, "owner") != NULL
#endif
	  )
	mount_entry->is_user_mountable = TRUE;
      
      return_list = g_list_prepend (return_list, mount_entry);
    }
  
  endmntent (file);

#ifndef HAVE_GETMNTENT_R
  G_UNLOCK (getmntent);
#endif
  
  return g_list_reverse (return_list);
}

#elif defined (HAVE_SYS_MNTTAB_H)

static GList *
_g_get_unix_mount_points (void)
{
  struct mnttab mntent;
  FILE *file;
  char *read_file;
  GUnixMountPoint *mount_entry;
  GList *return_list;
  
  read_file = get_fstab_file ();
  
  file = setmntent (read_file, "r");
  if (file == NULL)
    return NULL;

  return_list = NULL;
  
  G_LOCK (getmntent);
  while (! getmntent (file, &mntent))
    {
      if ((strcmp (mntent.mnt_mountp, "ignore") == 0) ||
          (strcmp (mntent.mnt_mountp, "swap") == 0) ||
          (strcmp (mntent.mnt_mountp, "none") == 0))
	continue;
      
      mount_entry = g_new0 (GUnixMountPoint, 1);
      
      mount_entry->mount_path = g_strdup (mntent.mnt_mountp);
      mount_entry->device_path = g_strdup (mntent.mnt_special);
      mount_entry->filesystem_type = g_strdup (mntent.mnt_fstype);
      mount_entry->options = g_strdup (mntent.mnt_mntopts);
      
#ifdef HAVE_HASMNTOPT
      if (hasmntopt (&mntent, MNTOPT_RO) != NULL)
	mount_entry->is_read_only = TRUE;
      
      if (hasmntopt (&mntent, "lofs") != NULL)
	mount_entry->is_loopback = TRUE;
#endif
      
      if ((mntent.mnt_fstype != NULL)
#ifdef HAVE_HASMNTOPT
	  || (hasmntopt (&mntent, "user") != NULL
	      && hasmntopt (&mntent, "user") != hasmntopt (&mntent, "user_xattr"))
	  || hasmntopt (&mntent, "pamconsole") != NULL
	  || hasmntopt (&mntent, "users") != NULL
	  || hasmntopt (&mntent, "owner") != NULL
#endif
	  )
	mount_entry->is_user_mountable = TRUE;
      
      return_list = g_list_prepend (return_list, mount_entry);
    }
  
  endmntent (file);
  G_UNLOCK (getmntent);
  
  return g_list_reverse (return_list);
}
#elif defined(HAVE_SYS_MNTCTL_H) && defined(HAVE_SYS_VMOUNT_H) && defined(HAVE_SYS_VFS_H)

/* functions to parse /etc/filesystems on aix */

/* read character, ignoring comments (begin with '*', end with '\n' */
static int
aix_fs_getc (FILE *fd)
{
  int c;
  
  while ((c = getc (fd)) == '*')
    {
      while (((c = getc (fd)) != '\n') && (c != EOF))
	;
    }
}

/* eat all continuous spaces in a file */
static int
aix_fs_ignorespace (FILE *fd)
{
  int c;
  
  while ((c = aix_fs_getc (fd)) != EOF)
    {
      if (!g_ascii_isspace (c))
	{
	  ungetc (c,fd);
	  return c;
	}
    }
  
  return EOF;
}

/* read one word from file */
static int
aix_fs_getword (FILE *fd, 
                char *word)
{
  int c;
  
  aix_fs_ignorespace (fd);

  while (((c = aix_fs_getc (fd)) != EOF) && !g_ascii_isspace (c))
    {
      if (c == '"')
	{
	  while (((c = aix_fs_getc (fd)) != EOF) && (c != '"'))
	    *word++ = c;
	  else
	    *word++ = c;
	}
    }
  *word = 0;
  
  return c;
}

typedef struct {
  char mnt_mount[PATH_MAX];
  char mnt_special[PATH_MAX];
  char mnt_fstype[16];
  char mnt_options[128];
} AixMountTableEntry;

/* read mount points properties */
static int
aix_fs_get (FILE               *fd, 
            AixMountTableEntry *prop)
{
  static char word[PATH_MAX] = { 0 };
  char value[PATH_MAX];
  
  /* read stanza */
  if (word[0] == 0)
    {
      if (aix_fs_getword (fd, word) == EOF)
	return EOF;
    }

  word[strlen(word) - 1] = 0;
  strcpy (prop->mnt_mount, word);
  
  /* read attributes and value */
  
  while (aix_fs_getword (fd, word) != EOF)
    {
      /* test if is attribute or new stanza */
      if (word[strlen(word) - 1] == ':')
	return 0;
      
      /* read "=" */
      aix_fs_getword (fd, value);
      
      /* read value */
      aix_fs_getword (fd, value);
      
      if (strcmp (word, "dev") == 0)
	strcpy (prop->mnt_special, value);
      else if (strcmp (word, "vfs") == 0)
	strcpy (prop->mnt_fstype, value);
      else if (strcmp (word, "options") == 0)
	strcpy(prop->mnt_options, value);
    }
  
  return 0;
}

static GList *
_g_get_unix_mount_points (void)
{
  struct mntent *mntent;
  FILE *file;
  char *read_file;
  GUnixMountPoint *mount_entry;
  AixMountTableEntry mntent;
  GList *return_list;
  
  read_file = get_fstab_file ();
  
  file = setmntent (read_file, "r");
  if (file == NULL)
    return NULL;
  
  return_list = NULL;
  
  while (!aix_fs_get (file, &mntent))
    {
      if (strcmp ("cdrfs", mntent.mnt_fstype) == 0)
	{
	  mount_entry = g_new0 (GUnixMountPoint, 1);
	  
	  mount_entry->mount_path = g_strdup (mntent.mnt_mount);
	  mount_entry->device_path = g_strdup (mntent.mnt_special);
	  mount_entry->filesystem_type = g_strdup (mntent.mnt_fstype);
	  mount_entry->options = g_strdup (mntent.mnt_options);
	  mount_entry->is_read_only = TRUE;
	  mount_entry->is_user_mountable = TRUE;
	  
	  return_list = g_list_prepend (return_list, mount_entry);
	}
    }
	
  endmntent (file);
  
  return g_list_reverse (return_list);
}

#elif (defined(HAVE_GETVFSSTAT) || defined(HAVE_GETFSSTAT)) && defined(HAVE_FSTAB_H) && defined(HAVE_SYS_MOUNT_H)

static GList *
_g_get_unix_mount_points (void)
{
  struct fstab *fstab = NULL;
  GUnixMountPoint *mount_entry;
  GList *return_list;
#ifdef HAVE_SYS_SYSCTL_H
  int usermnt = 0;
  size_t len = sizeof(usermnt);
  struct stat sb;
#endif
  
  if (!setfsent ())
    return NULL;

  return_list = NULL;
  
#ifdef HAVE_SYS_SYSCTL_H
#if defined(HAVE_SYSCTLBYNAME)
  sysctlbyname ("vfs.usermount", &usermnt, &len, NULL, 0);
#elif defined(CTL_VFS) && defined(VFS_USERMOUNT)
  {
    int mib[2];
    
    mib[0] = CTL_VFS;
    mib[1] = VFS_USERMOUNT;
    sysctl (mib, 2, &usermnt, &len, NULL, 0);
  }
#elif defined(CTL_KERN) && defined(KERN_USERMOUNT)
  {
    int mib[2];
    
    mib[0] = CTL_KERN;
    mib[1] = KERN_USERMOUNT;
    sysctl (mib, 2, &usermnt, &len, NULL, 0);
  }
#endif
#endif
  
  while ((fstab = getfsent ()) != NULL)
    {
      if (strcmp (fstab->fs_vfstype, "swap") == 0)
	continue;
      
      mount_entry = g_new0 (GUnixMountPoint, 1);
      
      mount_entry->mount_path = g_strdup (fstab->fs_file);
      mount_entry->device_path = g_strdup (fstab->fs_spec);
      mount_entry->filesystem_type = g_strdup (fstab->fs_vfstype);
      mount_entry->options = g_strdup (fstab->fs_mntops);
      
      if (strcmp (fstab->fs_type, "ro") == 0)
	mount_entry->is_read_only = TRUE;

#ifdef HAVE_SYS_SYSCTL_H
      if (usermnt != 0)
	{
	  uid_t uid = getuid ();
	  if (stat (fstab->fs_file, &sb) == 0)
	    {
	      if (uid == 0 || sb.st_uid == uid)
		mount_entry->is_user_mountable = TRUE;
	    }
	}
#endif

      return_list = g_list_prepend (return_list, mount_entry);
    }
  
  endfsent ();
  
  return g_list_reverse (return_list);
}
#elif defined(__INTERIX)
static GList *
_g_get_unix_mount_points (void)
{
  return _g_get_unix_mounts ();
}
#else
#error No g_get_mount_table() implementation for system
#endif

static guint64
get_mounts_timestamp (void)
{
  const char *monitor_file;
  struct stat buf;

  monitor_file = get_mtab_monitor_file ();
  if (monitor_file)
    {
      if (stat (monitor_file, &buf) == 0)
	return (guint64)buf.st_mtime;
    }
  return 0;
}

static guint64
get_mount_points_timestamp (void)
{
  const char *monitor_file;
  struct stat buf;

  monitor_file = get_fstab_file ();
  if (monitor_file)
    {
      if (stat (monitor_file, &buf) == 0)
	return (guint64)buf.st_mtime;
    }
  return 0;
}

/**
 * g_unix_mounts_get: (skip)
 * @time_read: (out) (allow-none): guint64 to contain a timestamp, or %NULL
 *
 * Gets a #GList of #GUnixMountEntry containing the unix mounts.
 * If @time_read is set, it will be filled with the mount
 * timestamp, allowing for checking if the mounts have changed
 * with g_unix_mounts_changed_since().
 *
 * Returns: (element-type GUnixMountEntry) (transfer full):
 *     a #GList of the UNIX mounts.
 **/
GList *
g_unix_mounts_get (guint64 *time_read)
{
  if (time_read)
    *time_read = get_mounts_timestamp ();

  return _g_get_unix_mounts ();
}

/**
 * g_unix_mount_at: (skip)
 * @mount_path: path for a possible unix mount.
 * @time_read: (out) (allow-none): guint64 to contain a timestamp.
 * 
 * Gets a #GUnixMountEntry for a given mount path. If @time_read
 * is set, it will be filled with a unix timestamp for checking
 * if the mounts have changed since with g_unix_mounts_changed_since().
 * 
 * Returns: (transfer full): a #GUnixMountEntry.
 **/
GUnixMountEntry *
g_unix_mount_at (const char *mount_path,
		 guint64    *time_read)
{
  GList *mounts, *l;
  GUnixMountEntry *mount_entry, *found;
  
  mounts = g_unix_mounts_get (time_read);

  found = NULL;
  for (l = mounts; l != NULL; l = l->next)
    {
      mount_entry = l->data;

      if (!found && strcmp (mount_path, mount_entry->mount_path) == 0)
	found = mount_entry;
      else
	g_unix_mount_free (mount_entry);
    }
  g_list_free (mounts);

  return found;
}

/**
 * g_unix_mount_points_get: (skip)
 * @time_read: (out) (allow-none): guint64 to contain a timestamp.
 *
 * Gets a #GList of #GUnixMountPoint containing the unix mount points.
 * If @time_read is set, it will be filled with the mount timestamp,
 * allowing for checking if the mounts have changed with
 * g_unix_mount_points_changed_since().
 *
 * Returns: (element-type GUnixMountPoint) (transfer full):
 *     a #GList of the UNIX mountpoints.
 **/
GList *
g_unix_mount_points_get (guint64 *time_read)
{
  if (time_read)
    *time_read = get_mount_points_timestamp ();

  return _g_get_unix_mount_points ();
}

/**
 * g_unix_mounts_changed_since:
 * @time: guint64 to contain a timestamp.
 * 
 * Checks if the unix mounts have changed since a given unix time.
 * 
 * Returns: %TRUE if the mounts have changed since @time. 
 **/
gboolean
g_unix_mounts_changed_since (guint64 time)
{
  return get_mounts_timestamp () != time;
}

/**
 * g_unix_mount_points_changed_since:
 * @time: guint64 to contain a timestamp.
 * 
 * Checks if the unix mount points have changed since a given unix time.
 * 
 * Returns: %TRUE if the mount points have changed since @time. 
 **/
gboolean
g_unix_mount_points_changed_since (guint64 time)
{
  return get_mount_points_timestamp () != time;
}

static void
g_unix_mount_monitor_finalize (GObject *object)
{
  GUnixMountMonitor *monitor;
  
  monitor = G_UNIX_MOUNT_MONITOR (object);

  if (monitor->fstab_monitor)
    {
      g_file_monitor_cancel (monitor->fstab_monitor);
      g_object_unref (monitor->fstab_monitor);
    }
  
  if (monitor->proc_mounts_watch_source != NULL)
    g_source_destroy (monitor->proc_mounts_watch_source);

  if (monitor->mtab_monitor)
    {
      g_file_monitor_cancel (monitor->mtab_monitor);
      g_object_unref (monitor->mtab_monitor);
    }

  the_mount_monitor = NULL;

  G_OBJECT_CLASS (g_unix_mount_monitor_parent_class)->finalize (object);
}


static void
g_unix_mount_monitor_class_init (GUnixMountMonitorClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = g_unix_mount_monitor_finalize;
 
  /**
   * GUnixMountMonitor::mounts-changed:
   * @monitor: the object on which the signal is emitted
   * 
   * Emitted when the unix mounts have changed.
   */ 
  signals[MOUNTS_CHANGED] =
    g_signal_new ("mounts-changed",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_LAST,
		  0,
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  /**
   * GUnixMountMonitor::mountpoints-changed:
   * @monitor: the object on which the signal is emitted
   * 
   * Emitted when the unix mount points have changed.
   */
  signals[MOUNTPOINTS_CHANGED] =
    g_signal_new ("mountpoints-changed",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_LAST,
		  0,
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
}

static void
fstab_file_changed (GFileMonitor      *monitor,
		    GFile             *file,
		    GFile             *other_file,
		    GFileMonitorEvent  event_type,
		    gpointer           user_data)
{
  GUnixMountMonitor *mount_monitor;

  if (event_type != G_FILE_MONITOR_EVENT_CHANGED &&
      event_type != G_FILE_MONITOR_EVENT_CREATED &&
      event_type != G_FILE_MONITOR_EVENT_DELETED)
    return;

  mount_monitor = user_data;
  g_signal_emit (mount_monitor, signals[MOUNTPOINTS_CHANGED], 0);
}

static void
mtab_file_changed (GFileMonitor      *monitor,
		   GFile             *file,
		   GFile             *other_file,
		   GFileMonitorEvent  event_type,
		   gpointer           user_data)
{
  GUnixMountMonitor *mount_monitor;

  if (event_type != G_FILE_MONITOR_EVENT_CHANGED &&
      event_type != G_FILE_MONITOR_EVENT_CREATED &&
      event_type != G_FILE_MONITOR_EVENT_DELETED)
    return;
  
  mount_monitor = user_data;
  g_signal_emit (mount_monitor, signals[MOUNTS_CHANGED], 0);
}

static gboolean
proc_mounts_changed (GIOChannel   *channel,
                     GIOCondition  cond,
                     gpointer      user_data)
{
  GUnixMountMonitor *mount_monitor = G_UNIX_MOUNT_MONITOR (user_data);
  if (cond & G_IO_ERR)
    g_signal_emit (mount_monitor, signals[MOUNTS_CHANGED], 0);
  return TRUE;
}

static void
g_unix_mount_monitor_init (GUnixMountMonitor *monitor)
{
  GFile *file;
    
  if (get_fstab_file () != NULL)
    {
      file = g_file_new_for_path (get_fstab_file ());
      monitor->fstab_monitor = g_file_monitor_file (file, 0, NULL, NULL);
      g_object_unref (file);
      
      g_signal_connect (monitor->fstab_monitor, "changed", (GCallback)fstab_file_changed, monitor);
    }
  
  if (get_mtab_monitor_file () != NULL)
    {
      const gchar *mtab_path;

      mtab_path = get_mtab_monitor_file ();
      /* /proc/mounts monitoring is special - can't just use GFileMonitor.
       * See 'man proc' for more details.
       */
      if (g_strcmp0 (mtab_path, "/proc/mounts") == 0)
        {
          GIOChannel *proc_mounts_channel;
          GError *error = NULL;
          proc_mounts_channel = g_io_channel_new_file ("/proc/mounts", "r", &error);
          if (proc_mounts_channel == NULL)
            {
              g_warning ("Error creating IO channel for /proc/mounts: %s (%s, %d)",
                         error->message, g_quark_to_string (error->domain), error->code);
              g_error_free (error);
            }
          else
            {
              monitor->proc_mounts_watch_source = g_io_create_watch (proc_mounts_channel, G_IO_ERR);
              g_source_set_callback (monitor->proc_mounts_watch_source,
                                     (GSourceFunc) proc_mounts_changed,
                                     monitor,
                                     NULL);
              g_source_attach (monitor->proc_mounts_watch_source,
                               g_main_context_get_thread_default ());
              g_source_unref (monitor->proc_mounts_watch_source);
              g_io_channel_unref (proc_mounts_channel);
            }
        }
      else
        {
          file = g_file_new_for_path (mtab_path);
          monitor->mtab_monitor = g_file_monitor_file (file, 0, NULL, NULL);
          g_object_unref (file);
          g_signal_connect (monitor->mtab_monitor, "changed", (GCallback)mtab_file_changed, monitor);
        }
    }
}

/**
 * g_unix_mount_monitor_set_rate_limit:
 * @mount_monitor: a #GUnixMountMonitor
 * @limit_msec: a integer with the limit in milliseconds to
 *     poll for changes.
 *
 * Sets the rate limit to which the @mount_monitor will report
 * consecutive change events to the mount and mount point entry files.
 *
 * Since: 2.18
 */
void
g_unix_mount_monitor_set_rate_limit (GUnixMountMonitor *mount_monitor,
                                     gint               limit_msec)
{
  g_return_if_fail (G_IS_UNIX_MOUNT_MONITOR (mount_monitor));

  if (mount_monitor->fstab_monitor != NULL)
    g_file_monitor_set_rate_limit (mount_monitor->fstab_monitor, limit_msec);

  if (mount_monitor->mtab_monitor != NULL)
    g_file_monitor_set_rate_limit (mount_monitor->mtab_monitor, limit_msec);
}

/**
 * g_unix_mount_monitor_new:
 * 
 * Gets a new #GUnixMountMonitor. The default rate limit for which the
 * monitor will report consecutive changes for the mount and mount
 * point entry files is the default for a #GFileMonitor. Use
 * g_unix_mount_monitor_set_rate_limit() to change this.
 * 
 * Returns: a #GUnixMountMonitor. 
 */
GUnixMountMonitor *
g_unix_mount_monitor_new (void)
{
  if (the_mount_monitor == NULL)
    {
      the_mount_monitor = g_object_new (G_TYPE_UNIX_MOUNT_MONITOR, NULL);
      return the_mount_monitor;
    }
  
  return g_object_ref (the_mount_monitor);
}

/**
 * g_unix_mount_free:
 * @mount_entry: a #GUnixMountEntry.
 * 
 * Frees a unix mount.
 */
void
g_unix_mount_free (GUnixMountEntry *mount_entry)
{
  g_return_if_fail (mount_entry != NULL);

  g_free (mount_entry->mount_path);
  g_free (mount_entry->device_path);
  g_free (mount_entry->filesystem_type);
  g_free (mount_entry);
}

/**
 * g_unix_mount_point_free:
 * @mount_point: unix mount point to free.
 * 
 * Frees a unix mount point.
 */
void
g_unix_mount_point_free (GUnixMountPoint *mount_point)
{
  g_return_if_fail (mount_point != NULL);

  g_free (mount_point->mount_path);
  g_free (mount_point->device_path);
  g_free (mount_point->filesystem_type);
  g_free (mount_point->options);
  g_free (mount_point);
}

/**
 * g_unix_mount_compare:
 * @mount1: first #GUnixMountEntry to compare.
 * @mount2: second #GUnixMountEntry to compare.
 * 
 * Compares two unix mounts.
 * 
 * Returns: 1, 0 or -1 if @mount1 is greater than, equal to,
 * or less than @mount2, respectively. 
 */
gint
g_unix_mount_compare (GUnixMountEntry *mount1,
		      GUnixMountEntry *mount2)
{
  int res;

  g_return_val_if_fail (mount1 != NULL && mount2 != NULL, 0);
  
  res = g_strcmp0 (mount1->mount_path, mount2->mount_path);
  if (res != 0)
    return res;
	
  res = g_strcmp0 (mount1->device_path, mount2->device_path);
  if (res != 0)
    return res;
	
  res = g_strcmp0 (mount1->filesystem_type, mount2->filesystem_type);
  if (res != 0)
    return res;

  res =  mount1->is_read_only - mount2->is_read_only;
  if (res != 0)
    return res;
  
  return 0;
}

/**
 * g_unix_mount_get_mount_path:
 * @mount_entry: input #GUnixMountEntry to get the mount path for.
 * 
 * Gets the mount path for a unix mount.
 * 
 * Returns: the mount path for @mount_entry.
 */
const gchar *
g_unix_mount_get_mount_path (GUnixMountEntry *mount_entry)
{
  g_return_val_if_fail (mount_entry != NULL, NULL);

  return mount_entry->mount_path;
}

/**
 * g_unix_mount_get_device_path:
 * @mount_entry: a #GUnixMount.
 * 
 * Gets the device path for a unix mount.
 * 
 * Returns: a string containing the device path.
 */
const gchar *
g_unix_mount_get_device_path (GUnixMountEntry *mount_entry)
{
  g_return_val_if_fail (mount_entry != NULL, NULL);

  return mount_entry->device_path;
}

/**
 * g_unix_mount_get_fs_type:
 * @mount_entry: a #GUnixMount.
 * 
 * Gets the filesystem type for the unix mount.
 * 
 * Returns: a string containing the file system type.
 */
const gchar *
g_unix_mount_get_fs_type (GUnixMountEntry *mount_entry)
{
  g_return_val_if_fail (mount_entry != NULL, NULL);

  return mount_entry->filesystem_type;
}

/**
 * g_unix_mount_is_readonly:
 * @mount_entry: a #GUnixMount.
 * 
 * Checks if a unix mount is mounted read only.
 * 
 * Returns: %TRUE if @mount_entry is read only.
 */
gboolean
g_unix_mount_is_readonly (GUnixMountEntry *mount_entry)
{
  g_return_val_if_fail (mount_entry != NULL, FALSE);

  return mount_entry->is_read_only;
}

/**
 * g_unix_mount_is_system_internal:
 * @mount_entry: a #GUnixMount.
 * 
 * Checks if a unix mount is a system path.
 * 
 * Returns: %TRUE if the unix mount is for a system path.
 */
gboolean
g_unix_mount_is_system_internal (GUnixMountEntry *mount_entry)
{
  g_return_val_if_fail (mount_entry != NULL, FALSE);

  return mount_entry->is_system_internal;
}

/**
 * g_unix_mount_point_compare:
 * @mount1: a #GUnixMount.
 * @mount2: a #GUnixMount.
 * 
 * Compares two unix mount points.
 * 
 * Returns: 1, 0 or -1 if @mount1 is greater than, equal to,
 * or less than @mount2, respectively.
 */
gint
g_unix_mount_point_compare (GUnixMountPoint *mount1,
			    GUnixMountPoint *mount2)
{
  int res;

  g_return_val_if_fail (mount1 != NULL && mount2 != NULL, 0);

  res = g_strcmp0 (mount1->mount_path, mount2->mount_path);
  if (res != 0) 
    return res;
	
  res = g_strcmp0 (mount1->device_path, mount2->device_path);
  if (res != 0) 
    return res;
	
  res = g_strcmp0 (mount1->filesystem_type, mount2->filesystem_type);
  if (res != 0) 
    return res;

  res = g_strcmp0 (mount1->options, mount2->options);
  if (res != 0) 
    return res;

  res =  mount1->is_read_only - mount2->is_read_only;
  if (res != 0) 
    return res;

  res = mount1->is_user_mountable - mount2->is_user_mountable;
  if (res != 0) 
    return res;

  res = mount1->is_loopback - mount2->is_loopback;
  if (res != 0)
    return res;
  
  return 0;
}

/**
 * g_unix_mount_point_get_mount_path:
 * @mount_point: a #GUnixMountPoint.
 * 
 * Gets the mount path for a unix mount point.
 * 
 * Returns: a string containing the mount path.
 */
const gchar *
g_unix_mount_point_get_mount_path (GUnixMountPoint *mount_point)
{
  g_return_val_if_fail (mount_point != NULL, NULL);

  return mount_point->mount_path;
}

/**
 * g_unix_mount_point_get_device_path:
 * @mount_point: a #GUnixMountPoint.
 * 
 * Gets the device path for a unix mount point.
 * 
 * Returns: a string containing the device path.
 */
const gchar *
g_unix_mount_point_get_device_path (GUnixMountPoint *mount_point)
{
  g_return_val_if_fail (mount_point != NULL, NULL);

  return mount_point->device_path;
}

/**
 * g_unix_mount_point_get_fs_type:
 * @mount_point: a #GUnixMountPoint.
 * 
 * Gets the file system type for the mount point.
 * 
 * Returns: a string containing the file system type.
 */
const gchar *
g_unix_mount_point_get_fs_type (GUnixMountPoint *mount_point)
{
  g_return_val_if_fail (mount_point != NULL, NULL);

  return mount_point->filesystem_type;
}

/**
 * g_unix_mount_point_get_options:
 * @mount_point: a #GUnixMountPoint.
 * 
 * Gets the options for the mount point.
 * 
 * Returns: a string containing the options.
 *
 * Since: 2.32
 */
const gchar *
g_unix_mount_point_get_options (GUnixMountPoint *mount_point)
{
  g_return_val_if_fail (mount_point != NULL, NULL);

  return mount_point->options;
}

/**
 * g_unix_mount_point_is_readonly:
 * @mount_point: a #GUnixMountPoint.
 * 
 * Checks if a unix mount point is read only.
 * 
 * Returns: %TRUE if a mount point is read only.
 */
gboolean
g_unix_mount_point_is_readonly (GUnixMountPoint *mount_point)
{
  g_return_val_if_fail (mount_point != NULL, FALSE);

  return mount_point->is_read_only;
}

/**
 * g_unix_mount_point_is_user_mountable:
 * @mount_point: a #GUnixMountPoint.
 * 
 * Checks if a unix mount point is mountable by the user.
 * 
 * Returns: %TRUE if the mount point is user mountable.
 */
gboolean
g_unix_mount_point_is_user_mountable (GUnixMountPoint *mount_point)
{
  g_return_val_if_fail (mount_point != NULL, FALSE);

  return mount_point->is_user_mountable;
}

/**
 * g_unix_mount_point_is_loopback:
 * @mount_point: a #GUnixMountPoint.
 * 
 * Checks if a unix mount point is a loopback device.
 * 
 * Returns: %TRUE if the mount point is a loopback. %FALSE otherwise. 
 */
gboolean
g_unix_mount_point_is_loopback (GUnixMountPoint *mount_point)
{
  g_return_val_if_fail (mount_point != NULL, FALSE);

  return mount_point->is_loopback;
}

static GUnixMountType
guess_mount_type (const char *mount_path,
		  const char *device_path,
		  const char *filesystem_type)
{
  GUnixMountType type;
  char *basename;

  type = G_UNIX_MOUNT_TYPE_UNKNOWN;
  
  if ((strcmp (filesystem_type, "udf") == 0) ||
      (strcmp (filesystem_type, "iso9660") == 0) ||
      (strcmp (filesystem_type, "cd9660") == 0))
    type = G_UNIX_MOUNT_TYPE_CDROM;
  else if ((strcmp (filesystem_type, "nfs") == 0) ||
           (strcmp (filesystem_type, "nfs4") == 0))
    type = G_UNIX_MOUNT_TYPE_NFS;
  else if (g_str_has_prefix (device_path, "/vol/dev/diskette/") ||
	   g_str_has_prefix (device_path, "/dev/fd") ||
	   g_str_has_prefix (device_path, "/dev/floppy"))
    type = G_UNIX_MOUNT_TYPE_FLOPPY;
  else if (g_str_has_prefix (device_path, "/dev/cdrom") ||
	   g_str_has_prefix (device_path, "/dev/acd") ||
	   g_str_has_prefix (device_path, "/dev/cd"))
    type = G_UNIX_MOUNT_TYPE_CDROM;
  else if (g_str_has_prefix (device_path, "/vol/"))
    {
      const char *name = mount_path + strlen ("/");
      
      if (g_str_has_prefix (name, "cdrom"))
	type = G_UNIX_MOUNT_TYPE_CDROM;
      else if (g_str_has_prefix (name, "floppy") ||
	       g_str_has_prefix (device_path, "/vol/dev/diskette/")) 
	type = G_UNIX_MOUNT_TYPE_FLOPPY;
      else if (g_str_has_prefix (name, "rmdisk")) 
	type = G_UNIX_MOUNT_TYPE_ZIP;
      else if (g_str_has_prefix (name, "jaz"))
	type = G_UNIX_MOUNT_TYPE_JAZ;
      else if (g_str_has_prefix (name, "memstick"))
	type = G_UNIX_MOUNT_TYPE_MEMSTICK;
    }
  else
    {
      basename = g_path_get_basename (mount_path);
      
      if (g_str_has_prefix (basename, "cdr") ||
	  g_str_has_prefix (basename, "cdwriter") ||
	  g_str_has_prefix (basename, "burn") ||
	  g_str_has_prefix (basename, "dvdr"))
	type = G_UNIX_MOUNT_TYPE_CDROM;
      else if (g_str_has_prefix (basename, "floppy"))
	type = G_UNIX_MOUNT_TYPE_FLOPPY;
      else if (g_str_has_prefix (basename, "zip"))
	type = G_UNIX_MOUNT_TYPE_ZIP;
      else if (g_str_has_prefix (basename, "jaz"))
	type = G_UNIX_MOUNT_TYPE_JAZ;
      else if (g_str_has_prefix (basename, "camera"))
	type = G_UNIX_MOUNT_TYPE_CAMERA;
      else if (g_str_has_prefix (basename, "memstick") ||
	       g_str_has_prefix (basename, "memory_stick") ||
	       g_str_has_prefix (basename, "ram"))
	type = G_UNIX_MOUNT_TYPE_MEMSTICK;
      else if (g_str_has_prefix (basename, "compact_flash"))
	type = G_UNIX_MOUNT_TYPE_CF;
      else if (g_str_has_prefix (basename, "smart_media"))
	type = G_UNIX_MOUNT_TYPE_SM;
      else if (g_str_has_prefix (basename, "sd_mmc"))
	type = G_UNIX_MOUNT_TYPE_SDMMC;
      else if (g_str_has_prefix (basename, "ipod"))
	type = G_UNIX_MOUNT_TYPE_IPOD;
      
      g_free (basename);
    }
  
  if (type == G_UNIX_MOUNT_TYPE_UNKNOWN)
    type = G_UNIX_MOUNT_TYPE_HD;
  
  return type;
}

/*
 * g_unix_mount_guess_type:
 * @mount_entry: a #GUnixMount.
 * 
 * Guesses the type of a unix mount. If the mount type cannot be 
 * determined, returns %G_UNIX_MOUNT_TYPE_UNKNOWN.
 * 
 * Returns: a #GUnixMountType. 
 */
static GUnixMountType
g_unix_mount_guess_type (GUnixMountEntry *mount_entry)
{
  g_return_val_if_fail (mount_entry != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);
  g_return_val_if_fail (mount_entry->mount_path != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);
  g_return_val_if_fail (mount_entry->device_path != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);
  g_return_val_if_fail (mount_entry->filesystem_type != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);

  return guess_mount_type (mount_entry->mount_path,
			   mount_entry->device_path,
			   mount_entry->filesystem_type);
}

/*
 * g_unix_mount_point_guess_type:
 * @mount_point: a #GUnixMountPoint.
 * 
 * Guesses the type of a unix mount point. 
 * If the mount type cannot be determined, 
 * returns %G_UNIX_MOUNT_TYPE_UNKNOWN.
 * 
 * Returns: a #GUnixMountType.
 */
static GUnixMountType
g_unix_mount_point_guess_type (GUnixMountPoint *mount_point)
{
  g_return_val_if_fail (mount_point != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);
  g_return_val_if_fail (mount_point->mount_path != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);
  g_return_val_if_fail (mount_point->device_path != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);
  g_return_val_if_fail (mount_point->filesystem_type != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);

  return guess_mount_type (mount_point->mount_path,
			   mount_point->device_path,
			   mount_point->filesystem_type);
}

static const char *
type_to_icon (GUnixMountType type, gboolean is_mount_point, gboolean use_symbolic)
{
  const char *icon_name;
  
  switch (type)
    {
    case G_UNIX_MOUNT_TYPE_HD:
      if (is_mount_point)
        icon_name = use_symbolic ? "drive-removable-media-symbolic" : "drive-removable-media";
      else
        icon_name = use_symbolic ? "drive-harddisk-symbolic" : "drive-harddisk";
      break;
    case G_UNIX_MOUNT_TYPE_FLOPPY:
    case G_UNIX_MOUNT_TYPE_ZIP:
    case G_UNIX_MOUNT_TYPE_JAZ:
      if (is_mount_point)
        icon_name = use_symbolic ? "drive-removable-media-symbolic" : "drive-removable-media";
      else
        icon_name = use_symbolic ? "media-removable-symbolic" : "media-floppy";
      break;
    case G_UNIX_MOUNT_TYPE_CDROM:
      if (is_mount_point)
        icon_name = use_symbolic ? "drive-optical-symbolic" : "drive-optical";
      else
        icon_name = use_symbolic ? "media-optical-symbolic" : "media-optical";
      break;
    case G_UNIX_MOUNT_TYPE_NFS:
        icon_name = use_symbolic ? "folder-remote-symbolic" : "folder-remote";
      break;
    case G_UNIX_MOUNT_TYPE_MEMSTICK:
      if (is_mount_point)
        icon_name = use_symbolic ? "drive-removable-media-symbolic" : "drive-removable-media";
      else
        icon_name = use_symbolic ? "media-removable-symbolic" : "media-flash";
      break;
    case G_UNIX_MOUNT_TYPE_CAMERA:
      if (is_mount_point)
        icon_name = use_symbolic ? "drive-removable-media-symbolic" : "drive-removable-media";
      else
        icon_name = use_symbolic ? "camera-photo-symbolic" : "camera-photo";
      break;
    case G_UNIX_MOUNT_TYPE_IPOD:
      if (is_mount_point)
        icon_name = use_symbolic ? "drive-removable-media-symbolic" : "drive-removable-media";
      else
        icon_name = use_symbolic ? "multimedia-player-symbolic" : "multimedia-player";
      break;
    case G_UNIX_MOUNT_TYPE_UNKNOWN:
    default:
      if (is_mount_point)
        icon_name = use_symbolic ? "drive-removable-media-symbolic" : "drive-removable-media";
      else
        icon_name = use_symbolic ? "drive-harddisk-symbolic" : "drive-harddisk";
      break;
    }

  return icon_name;
}

/**
 * g_unix_mount_guess_name:
 * @mount_entry: a #GUnixMountEntry
 * 
 * Guesses the name of a Unix mount. 
 * The result is a translated string.
 *
 * Returns: A newly allocated string that must
 *     be freed with g_free()
 */
gchar *
g_unix_mount_guess_name (GUnixMountEntry *mount_entry)
{
  char *name;

  if (strcmp (mount_entry->mount_path, "/") == 0)
    name = g_strdup (_("Filesystem root"));
  else
    name = g_filename_display_basename (mount_entry->mount_path);

  return name;
}

/**
 * g_unix_mount_guess_icon:
 * @mount_entry: a #GUnixMountEntry
 * 
 * Guesses the icon of a Unix mount. 
 *
 * Returns: (transfer full): a #GIcon
 */
GIcon *
g_unix_mount_guess_icon (GUnixMountEntry *mount_entry)
{
  return g_themed_icon_new_with_default_fallbacks (type_to_icon (g_unix_mount_guess_type (mount_entry), FALSE, FALSE));
}

/**
 * g_unix_mount_guess_symbolic_icon:
 * @mount_entry: a #GUnixMountEntry
 *
 * Guesses the symbolic icon of a Unix mount.
 *
 * Returns: (transfer full): a #GIcon
 *
 * Since: 2.34
 */
GIcon *
g_unix_mount_guess_symbolic_icon (GUnixMountEntry *mount_entry)
{
  return g_themed_icon_new_with_default_fallbacks (type_to_icon (g_unix_mount_guess_type (mount_entry), FALSE, TRUE));
}

/**
 * g_unix_mount_point_guess_name:
 * @mount_point: a #GUnixMountPoint
 * 
 * Guesses the name of a Unix mount point. 
 * The result is a translated string.
 *
 * Returns: A newly allocated string that must 
 *     be freed with g_free()
 */
gchar *
g_unix_mount_point_guess_name (GUnixMountPoint *mount_point)
{
  char *name;

  if (strcmp (mount_point->mount_path, "/") == 0)
    name = g_strdup (_("Filesystem root"));
  else
    name = g_filename_display_basename (mount_point->mount_path);

  return name;
}

/**
 * g_unix_mount_point_guess_icon:
 * @mount_point: a #GUnixMountPoint
 * 
 * Guesses the icon of a Unix mount point. 
 *
 * Returns: (transfer full): a #GIcon
 */
GIcon *
g_unix_mount_point_guess_icon (GUnixMountPoint *mount_point)
{
  return g_themed_icon_new_with_default_fallbacks (type_to_icon (g_unix_mount_point_guess_type (mount_point), TRUE, FALSE));
}

/**
 * g_unix_mount_point_guess_symbolic_icon:
 * @mount_point: a #GUnixMountPoint
 *
 * Guesses the symbolic icon of a Unix mount point.
 *
 * Returns: (transfer full): a #GIcon
 *
 * Since: 2.34
 */
GIcon *
g_unix_mount_point_guess_symbolic_icon (GUnixMountPoint *mount_point)
{
  return g_themed_icon_new_with_default_fallbacks (type_to_icon (g_unix_mount_point_guess_type (mount_point), TRUE, TRUE));
}

/**
 * g_unix_mount_guess_can_eject:
 * @mount_entry: a #GUnixMountEntry
 * 
 * Guesses whether a Unix mount can be ejected.
 *
 * Returns: %TRUE if @mount_entry is deemed to be ejectable.
 */
gboolean
g_unix_mount_guess_can_eject (GUnixMountEntry *mount_entry)
{
  GUnixMountType guessed_type;

  guessed_type = g_unix_mount_guess_type (mount_entry);
  if (guessed_type == G_UNIX_MOUNT_TYPE_IPOD ||
      guessed_type == G_UNIX_MOUNT_TYPE_CDROM)
    return TRUE;

  return FALSE;
}

/**
 * g_unix_mount_guess_should_display:
 * @mount_entry: a #GUnixMountEntry
 * 
 * Guesses whether a Unix mount should be displayed in the UI.
 *
 * Returns: %TRUE if @mount_entry is deemed to be displayable.
 */
gboolean
g_unix_mount_guess_should_display (GUnixMountEntry *mount_entry)
{
  const char *mount_path;
  const gchar *user_name;
  gsize user_name_len;

  /* Never display internal mountpoints */
  if (g_unix_mount_is_system_internal (mount_entry))
    return FALSE;
  
  /* Only display things in /media (which are generally user mountable)
     and home dir (fuse stuff) and /run/media/$USER */
  mount_path = mount_entry->mount_path;
  if (mount_path != NULL)
    {
      gboolean is_in_runtime_dir = FALSE;
      /* Hide mounts within a dot path, suppose it was a purpose to hide this mount */
      if (g_strstr_len (mount_path, -1, "/.") != NULL)
        return FALSE;

      /* Check /run/media/$USER/ */
      user_name = g_get_user_name ();
      user_name_len = strlen (user_name);
      if (strncmp (mount_path, "/run/media/", sizeof ("/run/media/") - 1) == 0 &&
          strncmp (mount_path + sizeof ("/run/media/") - 1, user_name, user_name_len) == 0 &&
          mount_path[sizeof ("/run/media/") - 1 + user_name_len] == '/')
        is_in_runtime_dir = TRUE;

      if (is_in_runtime_dir || g_str_has_prefix (mount_path, "/media/"))
        {
          char *path;
          /* Avoid displaying mounts that are not accessible to the user.
           *
           * See http://bugzilla.gnome.org/show_bug.cgi?id=526320 for why we
           * want to avoid g_access() for mount points which can potentially
           * block or fail stat()'ing, such as network mounts.
           */
          path = g_path_get_dirname (mount_path);
          if (g_str_has_prefix (path, "/media/"))
            {
              if (g_access (path, R_OK|X_OK) != 0) 
                {
                  g_free (path);
                  return FALSE;
                }
            }
          g_free (path);

          if (mount_entry->device_path && mount_entry->device_path[0] == '/')
           {
             struct stat st;
             if (g_stat (mount_entry->device_path, &st) == 0 &&
                 S_ISBLK(st.st_mode) &&
                 g_access (mount_path, R_OK|X_OK) != 0)
               return FALSE;
           }
          return TRUE;
        }
      
      if (g_str_has_prefix (mount_path, g_get_home_dir ()) && 
          mount_path[strlen (g_get_home_dir())] == G_DIR_SEPARATOR)
        return TRUE;
    }
  
  return FALSE;
}

/**
 * g_unix_mount_point_guess_can_eject:
 * @mount_point: a #GUnixMountPoint
 * 
 * Guesses whether a Unix mount point can be ejected.
 *
 * Returns: %TRUE if @mount_point is deemed to be ejectable.
 */
gboolean
g_unix_mount_point_guess_can_eject (GUnixMountPoint *mount_point)
{
  GUnixMountType guessed_type;

  guessed_type = g_unix_mount_point_guess_type (mount_point);
  if (guessed_type == G_UNIX_MOUNT_TYPE_IPOD ||
      guessed_type == G_UNIX_MOUNT_TYPE_CDROM)
    return TRUE;

  return FALSE;
}

#ifdef HAVE_MNTENT_H
/* borrowed from gtk/gtkfilesystemunix.c in GTK+ on 02/23/2006 */
static void
_canonicalize_filename (gchar *filename)
{
  gchar *p, *q;
  gboolean last_was_slash = FALSE;
  
  p = filename;
  q = filename;
  
  while (*p)
    {
      if (*p == G_DIR_SEPARATOR)
        {
          if (!last_was_slash)
            *q++ = G_DIR_SEPARATOR;
          
          last_was_slash = TRUE;
        }
      else
        {
          if (last_was_slash && *p == '.')
            {
              if (*(p + 1) == G_DIR_SEPARATOR ||
                  *(p + 1) == '\0')
                {
                  if (*(p + 1) == '\0')
                    break;
                  
                  p += 1;
                }
              else if (*(p + 1) == '.' &&
                       (*(p + 2) == G_DIR_SEPARATOR ||
                        *(p + 2) == '\0'))
                {
                  if (q > filename + 1)
                    {
                      q--;
                      while (q > filename + 1 &&
                             *(q - 1) != G_DIR_SEPARATOR)
                        q--;
                    }
                  
                  if (*(p + 2) == '\0')
                    break;
                  
                  p += 2;
                }
              else
                {
                  *q++ = *p;
                  last_was_slash = FALSE;
                }
            }
          else
            {
              *q++ = *p;
              last_was_slash = FALSE;
            }
        }
      
      p++;
    }
  
  if (q > filename + 1 && *(q - 1) == G_DIR_SEPARATOR)
    q--;
  
  *q = '\0';
}

static char *
_resolve_symlink (const char *file)
{
  GError *error;
  char *dir;
  char *link;
  char *f;
  char *f1;
  
  f = g_strdup (file);
  
  while (g_file_test (f, G_FILE_TEST_IS_SYMLINK)) 
    {
      link = g_file_read_link (f, &error);
      if (link == NULL) 
        {
          g_error_free (error);
          g_free (f);
          f = NULL;
          goto out;
        }
    
      dir = g_path_get_dirname (f);
      f1 = g_strdup_printf ("%s/%s", dir, link);
      g_free (dir);
      g_free (link);
      g_free (f);
      f = f1;
    }
  
 out:
  if (f != NULL)
    _canonicalize_filename (f);
  return f;
}

static const char *
_resolve_dev_root (void)
{
  static gboolean have_real_dev_root = FALSE;
  static char real_dev_root[256];
  struct stat statbuf;
  
  /* see if it's cached already */
  if (have_real_dev_root)
    goto found;
  
  /* otherwise we're going to find it right away.. */
  have_real_dev_root = TRUE;
  
  if (stat ("/dev/root", &statbuf) == 0) 
    {
      if (! S_ISLNK (statbuf.st_mode)) 
        {
          dev_t root_dev = statbuf.st_dev;
          FILE *f;
      
          /* see if device with similar major:minor as /dev/root is mention
           * in /etc/mtab (it usually is) 
           */
          f = fopen ("/etc/mtab", "r");
          if (f != NULL) 
            {
	      struct mntent *entp;
#ifdef HAVE_GETMNTENT_R        
              struct mntent ent;
              char buf[1024];
              while ((entp = getmntent_r (f, &ent, buf, sizeof (buf))) != NULL) 
                {
#else
	      G_LOCK (getmntent);
	      while ((entp = getmntent (f)) != NULL) 
                { 
#endif          
                  if (stat (entp->mnt_fsname, &statbuf) == 0 &&
                      statbuf.st_dev == root_dev) 
                    {
                      strncpy (real_dev_root, entp->mnt_fsname, sizeof (real_dev_root) - 1);
                      real_dev_root[sizeof (real_dev_root) - 1] = '\0';
                      fclose (f);
                      goto found;
                    }
                }

              endmntent (f);

#ifndef HAVE_GETMNTENT_R
	      G_UNLOCK (getmntent);
#endif
            }                                        
      
          /* no, that didn't work.. next we could scan /dev ... but I digress.. */
      
        } 
       else 
        {
          char *resolved;
          resolved = _resolve_symlink ("/dev/root");
          if (resolved != NULL)
            {
              strncpy (real_dev_root, resolved, sizeof (real_dev_root) - 1);
              real_dev_root[sizeof (real_dev_root) - 1] = '\0';
              g_free (resolved);
              goto found;
            }
        }
    }
  
  /* bah sucks.. */
  strcpy (real_dev_root, "/dev/root");
  
found:
  return real_dev_root;
}
#endif
