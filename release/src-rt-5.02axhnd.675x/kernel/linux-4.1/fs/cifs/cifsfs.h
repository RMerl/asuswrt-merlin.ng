/*
 *   fs/cifs/cifsfs.h
 *
 *   Copyright (c) International Business Machines  Corp., 2002, 2007
 *   Author(s): Steve French (sfrench@us.ibm.com)
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation; either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _CIFSFS_H
#define _CIFSFS_H

#include <linux/hash.h>

#define ROOT_I 2

/*
 * ino_t is 32-bits on 32-bit arch. We have to squash the 64-bit value down
 * so that it will fit. We use hash_64 to convert the value to 31 bits, and
 * then add 1, to ensure that we don't end up with a 0 as the value.
 */
static inline ino_t
cifs_uniqueid_to_ino_t(u64 fileid)
{
	if ((sizeof(ino_t)) < (sizeof(u64)))
		return (ino_t)hash_64(fileid, (sizeof(ino_t) * 8) - 1) + 1;

	return (ino_t)fileid;

}

extern struct file_system_type cifs_fs_type;
extern const struct address_space_operations cifs_addr_ops;
extern const struct address_space_operations cifs_addr_ops_smallbuf;

/* Functions related to super block operations */
extern void cifs_sb_active(struct super_block *sb);
extern void cifs_sb_deactive(struct super_block *sb);

/* Functions related to inodes */
extern const struct inode_operations cifs_dir_inode_ops;
extern struct inode *cifs_root_iget(struct super_block *);
extern int cifs_create(struct inode *, struct dentry *, umode_t,
		       bool excl);
extern int cifs_atomic_open(struct inode *, struct dentry *,
			    struct file *, unsigned, umode_t,
			    int *);
extern struct dentry *cifs_lookup(struct inode *, struct dentry *,
				  unsigned int);
extern int cifs_unlink(struct inode *dir, struct dentry *dentry);
extern int cifs_hardlink(struct dentry *, struct inode *, struct dentry *);
extern int cifs_mknod(struct inode *, struct dentry *, umode_t, dev_t);
extern int cifs_mkdir(struct inode *, struct dentry *, umode_t);
extern int cifs_rmdir(struct inode *, struct dentry *);
extern int cifs_rename2(struct inode *, struct dentry *, struct inode *,
			struct dentry *, unsigned int);
extern int cifs_revalidate_file_attr(struct file *filp);
extern int cifs_revalidate_dentry_attr(struct dentry *);
extern int cifs_revalidate_file(struct file *filp);
extern int cifs_revalidate_dentry(struct dentry *);
extern int cifs_invalidate_mapping(struct inode *inode);
extern int cifs_revalidate_mapping(struct inode *inode);
extern int cifs_zap_mapping(struct inode *inode);
extern int cifs_getattr(struct vfsmount *, struct dentry *, struct kstat *);
extern int cifs_setattr(struct dentry *, struct iattr *);

extern const struct inode_operations cifs_file_inode_ops;
extern const struct inode_operations cifs_symlink_inode_ops;
extern const struct inode_operations cifs_dfs_referral_inode_operations;


/* Functions related to files and directories */
extern const struct file_operations cifs_file_ops;
extern const struct file_operations cifs_file_direct_ops; /* if directio mnt */
extern const struct file_operations cifs_file_strict_ops; /* if strictio mnt */
extern const struct file_operations cifs_file_nobrl_ops; /* no brlocks */
extern const struct file_operations cifs_file_direct_nobrl_ops;
extern const struct file_operations cifs_file_strict_nobrl_ops;
extern int cifs_open(struct inode *inode, struct file *file);
extern int cifs_close(struct inode *inode, struct file *file);
extern int cifs_closedir(struct inode *inode, struct file *file);
extern ssize_t cifs_user_readv(struct kiocb *iocb, struct iov_iter *to);
extern ssize_t cifs_strict_readv(struct kiocb *iocb, struct iov_iter *to);
extern ssize_t cifs_user_writev(struct kiocb *iocb, struct iov_iter *from);
extern ssize_t cifs_strict_writev(struct kiocb *iocb, struct iov_iter *from);
extern int cifs_lock(struct file *, int, struct file_lock *);
extern int cifs_fsync(struct file *, loff_t, loff_t, int);
extern int cifs_strict_fsync(struct file *, loff_t, loff_t, int);
extern int cifs_flush(struct file *, fl_owner_t id);
extern int cifs_file_mmap(struct file * , struct vm_area_struct *);
extern int cifs_file_strict_mmap(struct file * , struct vm_area_struct *);
extern const struct file_operations cifs_dir_ops;
extern int cifs_dir_open(struct inode *inode, struct file *file);
extern int cifs_readdir(struct file *file, struct dir_context *ctx);

/* Functions related to dir entries */
extern const struct dentry_operations cifs_dentry_ops;
extern const struct dentry_operations cifs_ci_dentry_ops;

#ifdef CONFIG_CIFS_DFS_UPCALL
extern struct vfsmount *cifs_dfs_d_automount(struct path *path);
#else
#define cifs_dfs_d_automount NULL
#endif

/* Functions related to symlinks */
extern void *cifs_follow_link(struct dentry *direntry, struct nameidata *nd);
extern int cifs_readlink(struct dentry *direntry, char __user *buffer,
			 int buflen);
extern int cifs_symlink(struct inode *inode, struct dentry *direntry,
			const char *symname);
extern int	cifs_removexattr(struct dentry *, const char *);
extern int	cifs_setxattr(struct dentry *, const char *, const void *,
			size_t, int);
extern ssize_t	cifs_getxattr(struct dentry *, const char *, void *, size_t);
extern ssize_t	cifs_listxattr(struct dentry *, char *, size_t);
extern long cifs_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

#ifdef CONFIG_CIFS_NFSD_EXPORT
extern const struct export_operations cifs_export_ops;
#endif /* CONFIG_CIFS_NFSD_EXPORT */

#define CIFS_VERSION   "2.06"
#endif				/* _CIFSFS_H */
