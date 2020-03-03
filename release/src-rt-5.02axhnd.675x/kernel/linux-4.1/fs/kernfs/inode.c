/*
 * fs/kernfs/inode.c - kernfs inode implementation
 *
 * Copyright (c) 2001-3 Patrick Mochel
 * Copyright (c) 2007 SUSE Linux Products GmbH
 * Copyright (c) 2007, 2013 Tejun Heo <tj@kernel.org>
 *
 * This file is released under the GPLv2.
 */

#include <linux/pagemap.h>
#include <linux/backing-dev.h>
#include <linux/capability.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/xattr.h>
#include <linux/security.h>

#include "kernfs-internal.h"

static const struct address_space_operations kernfs_aops = {
	.readpage	= simple_readpage,
	.write_begin	= simple_write_begin,
	.write_end	= simple_write_end,
};

static const struct inode_operations kernfs_iops = {
	.permission	= kernfs_iop_permission,
	.setattr	= kernfs_iop_setattr,
	.getattr	= kernfs_iop_getattr,
	.setxattr	= kernfs_iop_setxattr,
	.removexattr	= kernfs_iop_removexattr,
	.getxattr	= kernfs_iop_getxattr,
	.listxattr	= kernfs_iop_listxattr,
};

static struct kernfs_iattrs *kernfs_iattrs(struct kernfs_node *kn)
{
	static DEFINE_MUTEX(iattr_mutex);
	struct kernfs_iattrs *ret;
	struct iattr *iattrs;

	mutex_lock(&iattr_mutex);

	if (kn->iattr)
		goto out_unlock;

	kn->iattr = kzalloc(sizeof(struct kernfs_iattrs), GFP_KERNEL);
	if (!kn->iattr)
		goto out_unlock;
	iattrs = &kn->iattr->ia_iattr;

	/* assign default attributes */
	iattrs->ia_mode = kn->mode;
	iattrs->ia_uid = GLOBAL_ROOT_UID;
	iattrs->ia_gid = GLOBAL_ROOT_GID;
	iattrs->ia_atime = iattrs->ia_mtime = iattrs->ia_ctime = CURRENT_TIME;

	simple_xattrs_init(&kn->iattr->xattrs);
out_unlock:
	ret = kn->iattr;
	mutex_unlock(&iattr_mutex);
	return ret;
}

static int __kernfs_setattr(struct kernfs_node *kn, const struct iattr *iattr)
{
	struct kernfs_iattrs *attrs;
	struct iattr *iattrs;
	unsigned int ia_valid = iattr->ia_valid;

	attrs = kernfs_iattrs(kn);
	if (!attrs)
		return -ENOMEM;

	iattrs = &attrs->ia_iattr;

	if (ia_valid & ATTR_UID)
		iattrs->ia_uid = iattr->ia_uid;
	if (ia_valid & ATTR_GID)
		iattrs->ia_gid = iattr->ia_gid;
	if (ia_valid & ATTR_ATIME)
		iattrs->ia_atime = iattr->ia_atime;
	if (ia_valid & ATTR_MTIME)
		iattrs->ia_mtime = iattr->ia_mtime;
	if (ia_valid & ATTR_CTIME)
		iattrs->ia_ctime = iattr->ia_ctime;
	if (ia_valid & ATTR_MODE) {
		umode_t mode = iattr->ia_mode;
		iattrs->ia_mode = kn->mode = mode;
	}
	return 0;
}

/**
 * kernfs_setattr - set iattr on a node
 * @kn: target node
 * @iattr: iattr to set
 *
 * Returns 0 on success, -errno on failure.
 */
int kernfs_setattr(struct kernfs_node *kn, const struct iattr *iattr)
{
	int ret;

	mutex_lock(&kernfs_mutex);
	ret = __kernfs_setattr(kn, iattr);
	mutex_unlock(&kernfs_mutex);
	return ret;
}

int kernfs_iop_setattr(struct dentry *dentry, struct iattr *iattr)
{
	struct inode *inode = d_inode(dentry);
	struct kernfs_node *kn = dentry->d_fsdata;
	int error;

	if (!kn)
		return -EINVAL;

	mutex_lock(&kernfs_mutex);
	error = setattr_prepare(dentry, iattr);
	if (error)
		goto out;

	error = __kernfs_setattr(kn, iattr);
	if (error)
		goto out;

	/* this ignores size changes */
	setattr_copy(inode, iattr);

out:
	mutex_unlock(&kernfs_mutex);
	return error;
}

static int kernfs_node_setsecdata(struct kernfs_node *kn, void **secdata,
				  u32 *secdata_len)
{
	struct kernfs_iattrs *attrs;
	void *old_secdata;
	size_t old_secdata_len;

	attrs = kernfs_iattrs(kn);
	if (!attrs)
		return -ENOMEM;

	old_secdata = attrs->ia_secdata;
	old_secdata_len = attrs->ia_secdata_len;

	attrs->ia_secdata = *secdata;
	attrs->ia_secdata_len = *secdata_len;

	*secdata = old_secdata;
	*secdata_len = old_secdata_len;
	return 0;
}

int kernfs_iop_setxattr(struct dentry *dentry, const char *name,
			const void *value, size_t size, int flags)
{
	struct kernfs_node *kn = dentry->d_fsdata;
	struct kernfs_iattrs *attrs;
	void *secdata;
	int error;
	u32 secdata_len = 0;

	attrs = kernfs_iattrs(kn);
	if (!attrs)
		return -ENOMEM;

	if (!strncmp(name, XATTR_SECURITY_PREFIX, XATTR_SECURITY_PREFIX_LEN)) {
		const char *suffix = name + XATTR_SECURITY_PREFIX_LEN;
		error = security_inode_setsecurity(d_inode(dentry), suffix,
						value, size, flags);
		if (error)
			return error;
		error = security_inode_getsecctx(d_inode(dentry),
						&secdata, &secdata_len);
		if (error)
			return error;

		mutex_lock(&kernfs_mutex);
		error = kernfs_node_setsecdata(kn, &secdata, &secdata_len);
		mutex_unlock(&kernfs_mutex);

		if (secdata)
			security_release_secctx(secdata, secdata_len);
		return error;
	} else if (!strncmp(name, XATTR_TRUSTED_PREFIX, XATTR_TRUSTED_PREFIX_LEN)) {
		return simple_xattr_set(&attrs->xattrs, name, value, size,
					flags);
	}

	return -EINVAL;
}

int kernfs_iop_removexattr(struct dentry *dentry, const char *name)
{
	struct kernfs_node *kn = dentry->d_fsdata;
	struct kernfs_iattrs *attrs;

	attrs = kernfs_iattrs(kn);
	if (!attrs)
		return -ENOMEM;

	return simple_xattr_remove(&attrs->xattrs, name);
}

ssize_t kernfs_iop_getxattr(struct dentry *dentry, const char *name, void *buf,
			    size_t size)
{
	struct kernfs_node *kn = dentry->d_fsdata;
	struct kernfs_iattrs *attrs;

	attrs = kernfs_iattrs(kn);
	if (!attrs)
		return -ENOMEM;

	return simple_xattr_get(&attrs->xattrs, name, buf, size);
}

ssize_t kernfs_iop_listxattr(struct dentry *dentry, char *buf, size_t size)
{
	struct kernfs_node *kn = dentry->d_fsdata;
	struct kernfs_iattrs *attrs;

	attrs = kernfs_iattrs(kn);
	if (!attrs)
		return -ENOMEM;

	return simple_xattr_list(&attrs->xattrs, buf, size);
}

static inline void set_default_inode_attr(struct inode *inode, umode_t mode)
{
	inode->i_mode = mode;
	inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
}

static inline void set_inode_attr(struct inode *inode, struct iattr *iattr)
{
	inode->i_uid = iattr->ia_uid;
	inode->i_gid = iattr->ia_gid;
	inode->i_atime = iattr->ia_atime;
	inode->i_mtime = iattr->ia_mtime;
	inode->i_ctime = iattr->ia_ctime;
}

static void kernfs_refresh_inode(struct kernfs_node *kn, struct inode *inode)
{
	struct kernfs_iattrs *attrs = kn->iattr;

	inode->i_mode = kn->mode;
	if (attrs) {
		/*
		 * kernfs_node has non-default attributes get them from
		 * persistent copy in kernfs_node.
		 */
		set_inode_attr(inode, &attrs->ia_iattr);
		security_inode_notifysecctx(inode, attrs->ia_secdata,
					    attrs->ia_secdata_len);
	}

	if (kernfs_type(kn) == KERNFS_DIR)
		set_nlink(inode, kn->dir.subdirs + 2);
}

int kernfs_iop_getattr(struct vfsmount *mnt, struct dentry *dentry,
		   struct kstat *stat)
{
	struct kernfs_node *kn = dentry->d_fsdata;
	struct inode *inode = d_inode(dentry);

	mutex_lock(&kernfs_mutex);
	kernfs_refresh_inode(kn, inode);
	mutex_unlock(&kernfs_mutex);

	generic_fillattr(inode, stat);
	return 0;
}

static void kernfs_init_inode(struct kernfs_node *kn, struct inode *inode)
{
	kernfs_get(kn);
	inode->i_private = kn;
	inode->i_mapping->a_ops = &kernfs_aops;
	inode->i_op = &kernfs_iops;

	set_default_inode_attr(inode, kn->mode);
	kernfs_refresh_inode(kn, inode);

	/* initialize inode according to type */
	switch (kernfs_type(kn)) {
	case KERNFS_DIR:
		inode->i_op = &kernfs_dir_iops;
		inode->i_fop = &kernfs_dir_fops;
		if (kn->flags & KERNFS_EMPTY_DIR)
			make_empty_dir_inode(inode);
		break;
	case KERNFS_FILE:
		inode->i_size = kn->attr.size;
		inode->i_fop = &kernfs_file_fops;
		break;
	case KERNFS_LINK:
		inode->i_op = &kernfs_symlink_iops;
		break;
	default:
		BUG();
	}

	unlock_new_inode(inode);
}

/**
 *	kernfs_get_inode - get inode for kernfs_node
 *	@sb: super block
 *	@kn: kernfs_node to allocate inode for
 *
 *	Get inode for @kn.  If such inode doesn't exist, a new inode is
 *	allocated and basics are initialized.  New inode is returned
 *	locked.
 *
 *	LOCKING:
 *	Kernel thread context (may sleep).
 *
 *	RETURNS:
 *	Pointer to allocated inode on success, NULL on failure.
 */
struct inode *kernfs_get_inode(struct super_block *sb, struct kernfs_node *kn)
{
	struct inode *inode;

	inode = iget_locked(sb, kn->ino);
	if (inode && (inode->i_state & I_NEW))
		kernfs_init_inode(kn, inode);

	return inode;
}

/*
 * The kernfs_node serves as both an inode and a directory entry for
 * kernfs.  To prevent the kernfs inode numbers from being freed
 * prematurely we take a reference to kernfs_node from the kernfs inode.  A
 * super_operations.evict_inode() implementation is needed to drop that
 * reference upon inode destruction.
 */
void kernfs_evict_inode(struct inode *inode)
{
	struct kernfs_node *kn = inode->i_private;

	truncate_inode_pages_final(&inode->i_data);
	clear_inode(inode);
	kernfs_put(kn);
}

int kernfs_iop_permission(struct inode *inode, int mask)
{
	struct kernfs_node *kn;

	if (mask & MAY_NOT_BLOCK)
		return -ECHILD;

	kn = inode->i_private;

	mutex_lock(&kernfs_mutex);
	kernfs_refresh_inode(kn, inode);
	mutex_unlock(&kernfs_mutex);

	return generic_permission(inode, mask);
}
