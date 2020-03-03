/*
*  Copyright (c) 2004 The Regents of the University of Michigan.
*  Copyright (c) 2012 Jeff Layton <jlayton@redhat.com>
*  All rights reserved.
*
*  Andy Adamson <andros@citi.umich.edu>
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*  1. Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*  3. Neither the name of the University nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
*  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
*  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
*  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
*  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
*  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
*  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
*  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#include <linux/file.h>
#include <linux/slab.h>
#include <linux/namei.h>
#include <linux/crypto.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <net/net_namespace.h>
#include <linux/sunrpc/rpc_pipe_fs.h>
#include <linux/sunrpc/clnt.h>
#include <linux/nfsd/cld.h>

#include "nfsd.h"
#include "state.h"
#include "vfs.h"
#include "netns.h"

#define NFSDDBG_FACILITY                NFSDDBG_PROC

/* Declarations */
struct nfsd4_client_tracking_ops {
	int (*init)(struct net *);
	void (*exit)(struct net *);
	void (*create)(struct nfs4_client *);
	void (*remove)(struct nfs4_client *);
	int (*check)(struct nfs4_client *);
	void (*grace_done)(struct nfsd_net *);
};

/* Globals */
static char user_recovery_dirname[PATH_MAX] = "/var/lib/nfs/v4recovery";

static int
nfs4_save_creds(const struct cred **original_creds)
{
	struct cred *new;

	new = prepare_creds();
	if (!new)
		return -ENOMEM;

	new->fsuid = GLOBAL_ROOT_UID;
	new->fsgid = GLOBAL_ROOT_GID;
	*original_creds = override_creds(new);
	put_cred(new);
	return 0;
}

static void
nfs4_reset_creds(const struct cred *original)
{
	revert_creds(original);
}

static void
md5_to_hex(char *out, char *md5)
{
	int i;

	for (i=0; i<16; i++) {
		unsigned char c = md5[i];

		*out++ = '0' + ((c&0xf0)>>4) + (c>=0xa0)*('a'-'9'-1);
		*out++ = '0' + (c&0x0f) + ((c&0x0f)>=0x0a)*('a'-'9'-1);
	}
	*out = '\0';
}

static int
nfs4_make_rec_clidname(char *dname, const struct xdr_netobj *clname)
{
	struct xdr_netobj cksum;
	struct hash_desc desc;
	struct scatterlist sg;
	int status;

	dprintk("NFSD: nfs4_make_rec_clidname for %.*s\n",
			clname->len, clname->data);
	desc.flags = CRYPTO_TFM_REQ_MAY_SLEEP;
	desc.tfm = crypto_alloc_hash("md5", 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(desc.tfm)) {
		status = PTR_ERR(desc.tfm);
		goto out_no_tfm;
	}

	cksum.len = crypto_hash_digestsize(desc.tfm);
	cksum.data = kmalloc(cksum.len, GFP_KERNEL);
	if (cksum.data == NULL) {
		status = -ENOMEM;
 		goto out;
	}

	sg_init_one(&sg, clname->data, clname->len);

	status = crypto_hash_digest(&desc, &sg, sg.length, cksum.data);
	if (status)
		goto out;

	md5_to_hex(dname, cksum.data);

	status = 0;
out:
	kfree(cksum.data);
	crypto_free_hash(desc.tfm);
out_no_tfm:
	return status;
}

/*
 * If we had an error generating the recdir name for the legacy tracker
 * then warn the admin. If the error doesn't appear to be transient,
 * then disable recovery tracking.
 */
static void
legacy_recdir_name_error(struct nfs4_client *clp, int error)
{
	printk(KERN_ERR "NFSD: unable to generate recoverydir "
			"name (%d).\n", error);

	/*
	 * if the algorithm just doesn't exist, then disable the recovery
	 * tracker altogether. The crypto libs will generally return this if
	 * FIPS is enabled as well.
	 */
	if (error == -ENOENT) {
		printk(KERN_ERR "NFSD: disabling legacy clientid tracking. "
			"Reboot recovery will not function correctly!\n");
		nfsd4_client_tracking_exit(clp->net);
	}
}

static void
nfsd4_create_clid_dir(struct nfs4_client *clp)
{
	const struct cred *original_cred;
	char dname[HEXDIR_LEN];
	struct dentry *dir, *dentry;
	struct nfs4_client_reclaim *crp;
	int status;
	struct nfsd_net *nn = net_generic(clp->net, nfsd_net_id);

	if (test_and_set_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags))
		return;
	if (!nn->rec_file)
		return;

	status = nfs4_make_rec_clidname(dname, &clp->cl_name);
	if (status)
		return legacy_recdir_name_error(clp, status);

	status = nfs4_save_creds(&original_cred);
	if (status < 0)
		return;

	status = mnt_want_write_file(nn->rec_file);
	if (status)
		goto out_creds;

	dir = nn->rec_file->f_path.dentry;
	/* lock the parent */
	mutex_lock(&d_inode(dir)->i_mutex);

	dentry = lookup_one_len(dname, dir, HEXDIR_LEN-1);
	if (IS_ERR(dentry)) {
		status = PTR_ERR(dentry);
		goto out_unlock;
	}
	if (d_really_is_positive(dentry))
		/*
		 * In the 4.1 case, where we're called from
		 * reclaim_complete(), records from the previous reboot
		 * may still be left, so this is OK.
		 *
		 * In the 4.0 case, we should never get here; but we may
		 * as well be forgiving and just succeed silently.
		 */
		goto out_put;
	status = vfs_mkdir(d_inode(dir), dentry, S_IRWXU);
out_put:
	dput(dentry);
out_unlock:
	mutex_unlock(&d_inode(dir)->i_mutex);
	if (status == 0) {
		if (nn->in_grace) {
			crp = nfs4_client_to_reclaim(dname, nn);
			if (crp)
				crp->cr_clp = clp;
		}
		vfs_fsync(nn->rec_file, 0);
	} else {
		printk(KERN_ERR "NFSD: failed to write recovery record"
				" (err %d); please check that %s exists"
				" and is writeable", status,
				user_recovery_dirname);
	}
	mnt_drop_write_file(nn->rec_file);
out_creds:
	nfs4_reset_creds(original_cred);
}

typedef int (recdir_func)(struct dentry *, struct dentry *, struct nfsd_net *);

struct name_list {
	char name[HEXDIR_LEN];
	struct list_head list;
};

struct nfs4_dir_ctx {
	struct dir_context ctx;
	struct list_head names;
};

static int
nfsd4_build_namelist(struct dir_context *__ctx, const char *name, int namlen,
		loff_t offset, u64 ino, unsigned int d_type)
{
	struct nfs4_dir_ctx *ctx =
		container_of(__ctx, struct nfs4_dir_ctx, ctx);
	struct name_list *entry;

	if (namlen != HEXDIR_LEN - 1)
		return 0;
	entry = kmalloc(sizeof(struct name_list), GFP_KERNEL);
	if (entry == NULL)
		return -ENOMEM;
	memcpy(entry->name, name, HEXDIR_LEN - 1);
	entry->name[HEXDIR_LEN - 1] = '\0';
	list_add(&entry->list, &ctx->names);
	return 0;
}

static int
nfsd4_list_rec_dir(recdir_func *f, struct nfsd_net *nn)
{
	const struct cred *original_cred;
	struct dentry *dir = nn->rec_file->f_path.dentry;
	struct nfs4_dir_ctx ctx = {
		.ctx.actor = nfsd4_build_namelist,
		.names = LIST_HEAD_INIT(ctx.names)
	};
	int status;

	status = nfs4_save_creds(&original_cred);
	if (status < 0)
		return status;

	status = vfs_llseek(nn->rec_file, 0, SEEK_SET);
	if (status < 0) {
		nfs4_reset_creds(original_cred);
		return status;
	}

	status = iterate_dir(nn->rec_file, &ctx.ctx);
	mutex_lock_nested(&d_inode(dir)->i_mutex, I_MUTEX_PARENT);
	while (!list_empty(&ctx.names)) {
		struct name_list *entry;
		entry = list_entry(ctx.names.next, struct name_list, list);
		if (!status) {
			struct dentry *dentry;
			dentry = lookup_one_len(entry->name, dir, HEXDIR_LEN-1);
			if (IS_ERR(dentry)) {
				status = PTR_ERR(dentry);
				break;
			}
			status = f(dir, dentry, nn);
			dput(dentry);
		}
		list_del(&entry->list);
		kfree(entry);
	}
	mutex_unlock(&d_inode(dir)->i_mutex);
	nfs4_reset_creds(original_cred);
	return status;
}

static int
nfsd4_unlink_clid_dir(char *name, int namlen, struct nfsd_net *nn)
{
	struct dentry *dir, *dentry;
	int status;

	dprintk("NFSD: nfsd4_unlink_clid_dir. name %.*s\n", namlen, name);

	dir = nn->rec_file->f_path.dentry;
	mutex_lock_nested(&d_inode(dir)->i_mutex, I_MUTEX_PARENT);
	dentry = lookup_one_len(name, dir, namlen);
	if (IS_ERR(dentry)) {
		status = PTR_ERR(dentry);
		goto out_unlock;
	}
	status = -ENOENT;
	if (d_really_is_negative(dentry))
		goto out;
	status = vfs_rmdir(d_inode(dir), dentry);
out:
	dput(dentry);
out_unlock:
	mutex_unlock(&d_inode(dir)->i_mutex);
	return status;
}

static void
nfsd4_remove_clid_dir(struct nfs4_client *clp)
{
	const struct cred *original_cred;
	struct nfs4_client_reclaim *crp;
	char dname[HEXDIR_LEN];
	int status;
	struct nfsd_net *nn = net_generic(clp->net, nfsd_net_id);

	if (!nn->rec_file || !test_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags))
		return;

	status = nfs4_make_rec_clidname(dname, &clp->cl_name);
	if (status)
		return legacy_recdir_name_error(clp, status);

	status = mnt_want_write_file(nn->rec_file);
	if (status)
		goto out;
	clear_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags);

	status = nfs4_save_creds(&original_cred);
	if (status < 0)
		goto out_drop_write;

	status = nfsd4_unlink_clid_dir(dname, HEXDIR_LEN-1, nn);
	nfs4_reset_creds(original_cred);
	if (status == 0) {
		vfs_fsync(nn->rec_file, 0);
		if (nn->in_grace) {
			/* remove reclaim record */
			crp = nfsd4_find_reclaim_client(dname, nn);
			if (crp)
				nfs4_remove_reclaim_record(crp, nn);
		}
	}
out_drop_write:
	mnt_drop_write_file(nn->rec_file);
out:
	if (status)
		printk("NFSD: Failed to remove expired client state directory"
				" %.*s\n", HEXDIR_LEN, dname);
}

static int
purge_old(struct dentry *parent, struct dentry *child, struct nfsd_net *nn)
{
	int status;

	if (nfs4_has_reclaimed_state(child->d_name.name, nn))
		return 0;

	status = vfs_rmdir(d_inode(parent), child);
	if (status)
		printk("failed to remove client recovery directory %pd\n",
				child);
	/* Keep trying, success or failure: */
	return 0;
}

static void
nfsd4_recdir_purge_old(struct nfsd_net *nn)
{
	int status;

	nn->in_grace = false;
	if (!nn->rec_file)
		return;
	status = mnt_want_write_file(nn->rec_file);
	if (status)
		goto out;
	status = nfsd4_list_rec_dir(purge_old, nn);
	if (status == 0)
		vfs_fsync(nn->rec_file, 0);
	mnt_drop_write_file(nn->rec_file);
out:
	nfs4_release_reclaim(nn);
	if (status)
		printk("nfsd4: failed to purge old clients from recovery"
			" directory %pD\n", nn->rec_file);
}

static int
load_recdir(struct dentry *parent, struct dentry *child, struct nfsd_net *nn)
{
	if (child->d_name.len != HEXDIR_LEN - 1) {
		printk("nfsd4: illegal name %pd in recovery directory\n",
				child);
		/* Keep trying; maybe the others are OK: */
		return 0;
	}
	nfs4_client_to_reclaim(child->d_name.name, nn);
	return 0;
}

static int
nfsd4_recdir_load(struct net *net) {
	int status;
	struct nfsd_net *nn =  net_generic(net, nfsd_net_id);

	if (!nn->rec_file)
		return 0;

	status = nfsd4_list_rec_dir(load_recdir, nn);
	if (status)
		printk("nfsd4: failed loading clients from recovery"
			" directory %pD\n", nn->rec_file);
	return status;
}

/*
 * Hold reference to the recovery directory.
 */

static int
nfsd4_init_recdir(struct net *net)
{
	struct nfsd_net *nn = net_generic(net, nfsd_net_id);
	const struct cred *original_cred;
	int status;

	printk("NFSD: Using %s as the NFSv4 state recovery directory\n",
			user_recovery_dirname);

	BUG_ON(nn->rec_file);

	status = nfs4_save_creds(&original_cred);
	if (status < 0) {
		printk("NFSD: Unable to change credentials to find recovery"
		       " directory: error %d\n",
		       status);
		return status;
	}

	nn->rec_file = filp_open(user_recovery_dirname, O_RDONLY | O_DIRECTORY, 0);
	if (IS_ERR(nn->rec_file)) {
		printk("NFSD: unable to find recovery directory %s\n",
				user_recovery_dirname);
		status = PTR_ERR(nn->rec_file);
		nn->rec_file = NULL;
	}

	nfs4_reset_creds(original_cred);
	if (!status)
		nn->in_grace = true;
	return status;
}

static void
nfsd4_shutdown_recdir(struct net *net)
{
	struct nfsd_net *nn = net_generic(net, nfsd_net_id);

	if (!nn->rec_file)
		return;
	fput(nn->rec_file);
	nn->rec_file = NULL;
}

static int
nfs4_legacy_state_init(struct net *net)
{
	struct nfsd_net *nn = net_generic(net, nfsd_net_id);
	int i;

	nn->reclaim_str_hashtbl = kmalloc(sizeof(struct list_head) *
					  CLIENT_HASH_SIZE, GFP_KERNEL);
	if (!nn->reclaim_str_hashtbl)
		return -ENOMEM;

	for (i = 0; i < CLIENT_HASH_SIZE; i++)
		INIT_LIST_HEAD(&nn->reclaim_str_hashtbl[i]);
	nn->reclaim_str_hashtbl_size = 0;

	return 0;
}

static void
nfs4_legacy_state_shutdown(struct net *net)
{
	struct nfsd_net *nn = net_generic(net, nfsd_net_id);

	kfree(nn->reclaim_str_hashtbl);
}

static int
nfsd4_load_reboot_recovery_data(struct net *net)
{
	int status;

	status = nfsd4_init_recdir(net);
	if (status)
		return status;

	status = nfsd4_recdir_load(net);
	if (status)
		nfsd4_shutdown_recdir(net);

	return status;
}

static int
nfsd4_legacy_tracking_init(struct net *net)
{
	int status;

	/* XXX: The legacy code won't work in a container */
	if (net != &init_net) {
		WARN(1, KERN_ERR "NFSD: attempt to initialize legacy client "
			"tracking in a container!\n");
		return -EINVAL;
	}

	status = nfs4_legacy_state_init(net);
	if (status)
		return status;

	status = nfsd4_load_reboot_recovery_data(net);
	if (status)
		goto err;
	return 0;

err:
	nfs4_legacy_state_shutdown(net);
	return status;
}

static void
nfsd4_legacy_tracking_exit(struct net *net)
{
	struct nfsd_net *nn = net_generic(net, nfsd_net_id);

	nfs4_release_reclaim(nn);
	nfsd4_shutdown_recdir(net);
	nfs4_legacy_state_shutdown(net);
}

/*
 * Change the NFSv4 recovery directory to recdir.
 */
int
nfs4_reset_recoverydir(char *recdir)
{
	int status;
	struct path path;

	status = kern_path(recdir, LOOKUP_FOLLOW, &path);
	if (status)
		return status;
	status = -ENOTDIR;
	if (d_is_dir(path.dentry)) {
		strcpy(user_recovery_dirname, recdir);
		status = 0;
	}
	path_put(&path);
	return status;
}

char *
nfs4_recoverydir(void)
{
	return user_recovery_dirname;
}

static int
nfsd4_check_legacy_client(struct nfs4_client *clp)
{
	int status;
	char dname[HEXDIR_LEN];
	struct nfs4_client_reclaim *crp;
	struct nfsd_net *nn = net_generic(clp->net, nfsd_net_id);

	/* did we already find that this client is stable? */
	if (test_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags))
		return 0;

	status = nfs4_make_rec_clidname(dname, &clp->cl_name);
	if (status) {
		legacy_recdir_name_error(clp, status);
		return status;
	}

	/* look for it in the reclaim hashtable otherwise */
	crp = nfsd4_find_reclaim_client(dname, nn);
	if (crp) {
		set_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags);
		crp->cr_clp = clp;
		return 0;
	}

	return -ENOENT;
}

static struct nfsd4_client_tracking_ops nfsd4_legacy_tracking_ops = {
	.init		= nfsd4_legacy_tracking_init,
	.exit		= nfsd4_legacy_tracking_exit,
	.create		= nfsd4_create_clid_dir,
	.remove		= nfsd4_remove_clid_dir,
	.check		= nfsd4_check_legacy_client,
	.grace_done	= nfsd4_recdir_purge_old,
};

/* Globals */
#define NFSD_PIPE_DIR		"nfsd"
#define NFSD_CLD_PIPE		"cld"

/* per-net-ns structure for holding cld upcall info */
struct cld_net {
	struct rpc_pipe		*cn_pipe;
	spinlock_t		 cn_lock;
	struct list_head	 cn_list;
	unsigned int		 cn_xid;
};

struct cld_upcall {
	struct list_head	 cu_list;
	struct cld_net		*cu_net;
	struct task_struct	*cu_task;
	struct cld_msg		 cu_msg;
};

static int
__cld_pipe_upcall(struct rpc_pipe *pipe, struct cld_msg *cmsg)
{
	int ret;
	struct rpc_pipe_msg msg;

	memset(&msg, 0, sizeof(msg));
	msg.data = cmsg;
	msg.len = sizeof(*cmsg);

	/*
	 * Set task state before we queue the upcall. That prevents
	 * wake_up_process in the downcall from racing with schedule.
	 */
	set_current_state(TASK_UNINTERRUPTIBLE);
	ret = rpc_queue_upcall(pipe, &msg);
	if (ret < 0) {
		set_current_state(TASK_RUNNING);
		goto out;
	}

	schedule();

	if (msg.errno < 0)
		ret = msg.errno;
out:
	return ret;
}

static int
cld_pipe_upcall(struct rpc_pipe *pipe, struct cld_msg *cmsg)
{
	int ret;

	/*
	 * -EAGAIN occurs when pipe is closed and reopened while there are
	 *  upcalls queued.
	 */
	do {
		ret = __cld_pipe_upcall(pipe, cmsg);
	} while (ret == -EAGAIN);

	return ret;
}

static ssize_t
cld_pipe_downcall(struct file *filp, const char __user *src, size_t mlen)
{
	struct cld_upcall *tmp, *cup;
	struct cld_msg __user *cmsg = (struct cld_msg __user *)src;
	uint32_t xid;
	struct nfsd_net *nn = net_generic(file_inode(filp)->i_sb->s_fs_info,
						nfsd_net_id);
	struct cld_net *cn = nn->cld_net;

	if (mlen != sizeof(*cmsg)) {
		dprintk("%s: got %zu bytes, expected %zu\n", __func__, mlen,
			sizeof(*cmsg));
		return -EINVAL;
	}

	/* copy just the xid so we can try to find that */
	if (copy_from_user(&xid, &cmsg->cm_xid, sizeof(xid)) != 0) {
		dprintk("%s: error when copying xid from userspace", __func__);
		return -EFAULT;
	}

	/* walk the list and find corresponding xid */
	cup = NULL;
	spin_lock(&cn->cn_lock);
	list_for_each_entry(tmp, &cn->cn_list, cu_list) {
		if (get_unaligned(&tmp->cu_msg.cm_xid) == xid) {
			cup = tmp;
			list_del_init(&cup->cu_list);
			break;
		}
	}
	spin_unlock(&cn->cn_lock);

	/* couldn't find upcall? */
	if (!cup) {
		dprintk("%s: couldn't find upcall -- xid=%u\n", __func__, xid);
		return -EINVAL;
	}

	if (copy_from_user(&cup->cu_msg, src, mlen) != 0)
		return -EFAULT;

	wake_up_process(cup->cu_task);
	return mlen;
}

static void
cld_pipe_destroy_msg(struct rpc_pipe_msg *msg)
{
	struct cld_msg *cmsg = msg->data;
	struct cld_upcall *cup = container_of(cmsg, struct cld_upcall,
						 cu_msg);

	/* errno >= 0 means we got a downcall */
	if (msg->errno >= 0)
		return;

	wake_up_process(cup->cu_task);
}

static const struct rpc_pipe_ops cld_upcall_ops = {
	.upcall		= rpc_pipe_generic_upcall,
	.downcall	= cld_pipe_downcall,
	.destroy_msg	= cld_pipe_destroy_msg,
};

static struct dentry *
nfsd4_cld_register_sb(struct super_block *sb, struct rpc_pipe *pipe)
{
	struct dentry *dir, *dentry;

	dir = rpc_d_lookup_sb(sb, NFSD_PIPE_DIR);
	if (dir == NULL)
		return ERR_PTR(-ENOENT);
	dentry = rpc_mkpipe_dentry(dir, NFSD_CLD_PIPE, NULL, pipe);
	dput(dir);
	return dentry;
}

static void
nfsd4_cld_unregister_sb(struct rpc_pipe *pipe)
{
	if (pipe->dentry)
		rpc_unlink(pipe->dentry);
}

static struct dentry *
nfsd4_cld_register_net(struct net *net, struct rpc_pipe *pipe)
{
	struct super_block *sb;
	struct dentry *dentry;

	sb = rpc_get_sb_net(net);
	if (!sb)
		return NULL;
	dentry = nfsd4_cld_register_sb(sb, pipe);
	rpc_put_sb_net(net);
	return dentry;
}

static void
nfsd4_cld_unregister_net(struct net *net, struct rpc_pipe *pipe)
{
	struct super_block *sb;

	sb = rpc_get_sb_net(net);
	if (sb) {
		nfsd4_cld_unregister_sb(pipe);
		rpc_put_sb_net(net);
	}
}

/* Initialize rpc_pipefs pipe for communication with client tracking daemon */
static int
nfsd4_init_cld_pipe(struct net *net)
{
	int ret;
	struct dentry *dentry;
	struct nfsd_net *nn = net_generic(net, nfsd_net_id);
	struct cld_net *cn;

	if (nn->cld_net)
		return 0;

	cn = kzalloc(sizeof(*cn), GFP_KERNEL);
	if (!cn) {
		ret = -ENOMEM;
		goto err;
	}

	cn->cn_pipe = rpc_mkpipe_data(&cld_upcall_ops, RPC_PIPE_WAIT_FOR_OPEN);
	if (IS_ERR(cn->cn_pipe)) {
		ret = PTR_ERR(cn->cn_pipe);
		goto err;
	}
	spin_lock_init(&cn->cn_lock);
	INIT_LIST_HEAD(&cn->cn_list);

	dentry = nfsd4_cld_register_net(net, cn->cn_pipe);
	if (IS_ERR(dentry)) {
		ret = PTR_ERR(dentry);
		goto err_destroy_data;
	}

	cn->cn_pipe->dentry = dentry;
	nn->cld_net = cn;
	return 0;

err_destroy_data:
	rpc_destroy_pipe_data(cn->cn_pipe);
err:
	kfree(cn);
	printk(KERN_ERR "NFSD: unable to create nfsdcld upcall pipe (%d)\n",
			ret);
	return ret;
}

static void
nfsd4_remove_cld_pipe(struct net *net)
{
	struct nfsd_net *nn = net_generic(net, nfsd_net_id);
	struct cld_net *cn = nn->cld_net;

	nfsd4_cld_unregister_net(net, cn->cn_pipe);
	rpc_destroy_pipe_data(cn->cn_pipe);
	kfree(nn->cld_net);
	nn->cld_net = NULL;
}

static struct cld_upcall *
alloc_cld_upcall(struct cld_net *cn)
{
	struct cld_upcall *new, *tmp;

	new = kzalloc(sizeof(*new), GFP_KERNEL);
	if (!new)
		return new;

	/* FIXME: hard cap on number in flight? */
restart_search:
	spin_lock(&cn->cn_lock);
	list_for_each_entry(tmp, &cn->cn_list, cu_list) {
		if (tmp->cu_msg.cm_xid == cn->cn_xid) {
			cn->cn_xid++;
			spin_unlock(&cn->cn_lock);
			goto restart_search;
		}
	}
	new->cu_task = current;
	new->cu_msg.cm_vers = CLD_UPCALL_VERSION;
	put_unaligned(cn->cn_xid++, &new->cu_msg.cm_xid);
	new->cu_net = cn;
	list_add(&new->cu_list, &cn->cn_list);
	spin_unlock(&cn->cn_lock);

	dprintk("%s: allocated xid %u\n", __func__, new->cu_msg.cm_xid);

	return new;
}

static void
free_cld_upcall(struct cld_upcall *victim)
{
	struct cld_net *cn = victim->cu_net;

	spin_lock(&cn->cn_lock);
	list_del(&victim->cu_list);
	spin_unlock(&cn->cn_lock);
	kfree(victim);
}

/* Ask daemon to create a new record */
static void
nfsd4_cld_create(struct nfs4_client *clp)
{
	int ret;
	struct cld_upcall *cup;
	struct nfsd_net *nn = net_generic(clp->net, nfsd_net_id);
	struct cld_net *cn = nn->cld_net;

	/* Don't upcall if it's already stored */
	if (test_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags))
		return;

	cup = alloc_cld_upcall(cn);
	if (!cup) {
		ret = -ENOMEM;
		goto out_err;
	}

	cup->cu_msg.cm_cmd = Cld_Create;
	cup->cu_msg.cm_u.cm_name.cn_len = clp->cl_name.len;
	memcpy(cup->cu_msg.cm_u.cm_name.cn_id, clp->cl_name.data,
			clp->cl_name.len);

	ret = cld_pipe_upcall(cn->cn_pipe, &cup->cu_msg);
	if (!ret) {
		ret = cup->cu_msg.cm_status;
		set_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags);
	}

	free_cld_upcall(cup);
out_err:
	if (ret)
		printk(KERN_ERR "NFSD: Unable to create client "
				"record on stable storage: %d\n", ret);
}

/* Ask daemon to create a new record */
static void
nfsd4_cld_remove(struct nfs4_client *clp)
{
	int ret;
	struct cld_upcall *cup;
	struct nfsd_net *nn = net_generic(clp->net, nfsd_net_id);
	struct cld_net *cn = nn->cld_net;

	/* Don't upcall if it's already removed */
	if (!test_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags))
		return;

	cup = alloc_cld_upcall(cn);
	if (!cup) {
		ret = -ENOMEM;
		goto out_err;
	}

	cup->cu_msg.cm_cmd = Cld_Remove;
	cup->cu_msg.cm_u.cm_name.cn_len = clp->cl_name.len;
	memcpy(cup->cu_msg.cm_u.cm_name.cn_id, clp->cl_name.data,
			clp->cl_name.len);

	ret = cld_pipe_upcall(cn->cn_pipe, &cup->cu_msg);
	if (!ret) {
		ret = cup->cu_msg.cm_status;
		clear_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags);
	}

	free_cld_upcall(cup);
out_err:
	if (ret)
		printk(KERN_ERR "NFSD: Unable to remove client "
				"record from stable storage: %d\n", ret);
}

/* Check for presence of a record, and update its timestamp */
static int
nfsd4_cld_check(struct nfs4_client *clp)
{
	int ret;
	struct cld_upcall *cup;
	struct nfsd_net *nn = net_generic(clp->net, nfsd_net_id);
	struct cld_net *cn = nn->cld_net;

	/* Don't upcall if one was already stored during this grace pd */
	if (test_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags))
		return 0;

	cup = alloc_cld_upcall(cn);
	if (!cup) {
		printk(KERN_ERR "NFSD: Unable to check client record on "
				"stable storage: %d\n", -ENOMEM);
		return -ENOMEM;
	}

	cup->cu_msg.cm_cmd = Cld_Check;
	cup->cu_msg.cm_u.cm_name.cn_len = clp->cl_name.len;
	memcpy(cup->cu_msg.cm_u.cm_name.cn_id, clp->cl_name.data,
			clp->cl_name.len);

	ret = cld_pipe_upcall(cn->cn_pipe, &cup->cu_msg);
	if (!ret) {
		ret = cup->cu_msg.cm_status;
		set_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags);
	}

	free_cld_upcall(cup);
	return ret;
}

static void
nfsd4_cld_grace_done(struct nfsd_net *nn)
{
	int ret;
	struct cld_upcall *cup;
	struct cld_net *cn = nn->cld_net;

	cup = alloc_cld_upcall(cn);
	if (!cup) {
		ret = -ENOMEM;
		goto out_err;
	}

	cup->cu_msg.cm_cmd = Cld_GraceDone;
	cup->cu_msg.cm_u.cm_gracetime = (int64_t)nn->boot_time;
	ret = cld_pipe_upcall(cn->cn_pipe, &cup->cu_msg);
	if (!ret)
		ret = cup->cu_msg.cm_status;

	free_cld_upcall(cup);
out_err:
	if (ret)
		printk(KERN_ERR "NFSD: Unable to end grace period: %d\n", ret);
}

static struct nfsd4_client_tracking_ops nfsd4_cld_tracking_ops = {
	.init		= nfsd4_init_cld_pipe,
	.exit		= nfsd4_remove_cld_pipe,
	.create		= nfsd4_cld_create,
	.remove		= nfsd4_cld_remove,
	.check		= nfsd4_cld_check,
	.grace_done	= nfsd4_cld_grace_done,
};

/* upcall via usermodehelper */
static char cltrack_prog[PATH_MAX] = "/sbin/nfsdcltrack";
module_param_string(cltrack_prog, cltrack_prog, sizeof(cltrack_prog),
			S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(cltrack_prog, "Path to the nfsdcltrack upcall program");

static bool cltrack_legacy_disable;
module_param(cltrack_legacy_disable, bool, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(cltrack_legacy_disable,
		"Disable legacy recoverydir conversion. Default: false");

#define LEGACY_TOPDIR_ENV_PREFIX "NFSDCLTRACK_LEGACY_TOPDIR="
#define LEGACY_RECDIR_ENV_PREFIX "NFSDCLTRACK_LEGACY_RECDIR="
#define HAS_SESSION_ENV_PREFIX "NFSDCLTRACK_CLIENT_HAS_SESSION="
#define GRACE_START_ENV_PREFIX "NFSDCLTRACK_GRACE_START="

static char *
nfsd4_cltrack_legacy_topdir(void)
{
	int copied;
	size_t len;
	char *result;

	if (cltrack_legacy_disable)
		return NULL;

	len = strlen(LEGACY_TOPDIR_ENV_PREFIX) +
		strlen(nfs4_recoverydir()) + 1;

	result = kmalloc(len, GFP_KERNEL);
	if (!result)
		return result;

	copied = snprintf(result, len, LEGACY_TOPDIR_ENV_PREFIX "%s",
				nfs4_recoverydir());
	if (copied >= len) {
		/* just return nothing if output was truncated */
		kfree(result);
		return NULL;
	}

	return result;
}

static char *
nfsd4_cltrack_legacy_recdir(const struct xdr_netobj *name)
{
	int copied;
	size_t len;
	char *result;

	if (cltrack_legacy_disable)
		return NULL;

	/* +1 is for '/' between "topdir" and "recdir" */
	len = strlen(LEGACY_RECDIR_ENV_PREFIX) +
		strlen(nfs4_recoverydir()) + 1 + HEXDIR_LEN;

	result = kmalloc(len, GFP_KERNEL);
	if (!result)
		return result;

	copied = snprintf(result, len, LEGACY_RECDIR_ENV_PREFIX "%s/",
				nfs4_recoverydir());
	if (copied > (len - HEXDIR_LEN)) {
		/* just return nothing if output will be truncated */
		kfree(result);
		return NULL;
	}

	copied = nfs4_make_rec_clidname(result + copied, name);
	if (copied) {
		kfree(result);
		return NULL;
	}

	return result;
}

static char *
nfsd4_cltrack_client_has_session(struct nfs4_client *clp)
{
	int copied;
	size_t len;
	char *result;

	/* prefix + Y/N character + terminating NULL */
	len = strlen(HAS_SESSION_ENV_PREFIX) + 1 + 1;

	result = kmalloc(len, GFP_KERNEL);
	if (!result)
		return result;

	copied = snprintf(result, len, HAS_SESSION_ENV_PREFIX "%c",
				clp->cl_minorversion ? 'Y' : 'N');
	if (copied >= len) {
		/* just return nothing if output was truncated */
		kfree(result);
		return NULL;
	}

	return result;
}

static char *
nfsd4_cltrack_grace_start(time_t grace_start)
{
	int copied;
	size_t len;
	char *result;

	/* prefix + max width of int64_t string + terminating NULL */
	len = strlen(GRACE_START_ENV_PREFIX) + 22 + 1;

	result = kmalloc(len, GFP_KERNEL);
	if (!result)
		return result;

	copied = snprintf(result, len, GRACE_START_ENV_PREFIX "%ld",
				grace_start);
	if (copied >= len) {
		/* just return nothing if output was truncated */
		kfree(result);
		return NULL;
	}

	return result;
}

static int
nfsd4_umh_cltrack_upcall(char *cmd, char *arg, char *env0, char *env1)
{
	char *envp[3];
	char *argv[4];
	int ret;

	if (unlikely(!cltrack_prog[0])) {
		dprintk("%s: cltrack_prog is disabled\n", __func__);
		return -EACCES;
	}

	dprintk("%s: cmd: %s\n", __func__, cmd);
	dprintk("%s: arg: %s\n", __func__, arg ? arg : "(null)");
	dprintk("%s: env0: %s\n", __func__, env0 ? env0 : "(null)");
	dprintk("%s: env1: %s\n", __func__, env1 ? env1 : "(null)");

	envp[0] = env0;
	envp[1] = env1;
	envp[2] = NULL;

	argv[0] = (char *)cltrack_prog;
	argv[1] = cmd;
	argv[2] = arg;
	argv[3] = NULL;

	ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
	/*
	 * Disable the upcall mechanism if we're getting an ENOENT or EACCES
	 * error. The admin can re-enable it on the fly by using sysfs
	 * once the problem has been fixed.
	 */
	if (ret == -ENOENT || ret == -EACCES) {
		dprintk("NFSD: %s was not found or isn't executable (%d). "
			"Setting cltrack_prog to blank string!",
			cltrack_prog, ret);
		cltrack_prog[0] = '\0';
	}
	dprintk("%s: %s return value: %d\n", __func__, cltrack_prog, ret);

	return ret;
}

static char *
bin_to_hex_dup(const unsigned char *src, int srclen)
{
	int i;
	char *buf, *hex;

	/* +1 for terminating NULL */
	buf = kmalloc((srclen * 2) + 1, GFP_KERNEL);
	if (!buf)
		return buf;

	hex = buf;
	for (i = 0; i < srclen; i++) {
		sprintf(hex, "%2.2x", *src++);
		hex += 2;
	}
	return buf;
}

static int
nfsd4_umh_cltrack_init(struct net *net)
{
	int ret;
	struct nfsd_net *nn = net_generic(net, nfsd_net_id);
	char *grace_start = nfsd4_cltrack_grace_start(nn->boot_time);

	/* XXX: The usermode helper s not working in container yet. */
	if (net != &init_net) {
		WARN(1, KERN_ERR "NFSD: attempt to initialize umh client "
			"tracking in a container!\n");
		return -EINVAL;
	}

	ret = nfsd4_umh_cltrack_upcall("init", NULL, grace_start, NULL);
	kfree(grace_start);
	return ret;
}

static void
nfsd4_cltrack_upcall_lock(struct nfs4_client *clp)
{
	wait_on_bit_lock(&clp->cl_flags, NFSD4_CLIENT_UPCALL_LOCK,
			 TASK_UNINTERRUPTIBLE);
}

static void
nfsd4_cltrack_upcall_unlock(struct nfs4_client *clp)
{
	smp_mb__before_atomic();
	clear_bit(NFSD4_CLIENT_UPCALL_LOCK, &clp->cl_flags);
	smp_mb__after_atomic();
	wake_up_bit(&clp->cl_flags, NFSD4_CLIENT_UPCALL_LOCK);
}

static void
nfsd4_umh_cltrack_create(struct nfs4_client *clp)
{
	char *hexid, *has_session, *grace_start;
	struct nfsd_net *nn = net_generic(clp->net, nfsd_net_id);

	/*
	 * With v4.0 clients, there's little difference in outcome between a
	 * create and check operation, and we can end up calling into this
	 * function multiple times per client (once for each openowner). So,
	 * for v4.0 clients skip upcalling once the client has been recorded
	 * on stable storage.
	 *
	 * For v4.1+ clients, the outcome of the two operations is different,
	 * so we must ensure that we upcall for the create operation. v4.1+
	 * clients call this on RECLAIM_COMPLETE though, so we should only end
	 * up doing a single create upcall per client.
	 */
	if (clp->cl_minorversion == 0 &&
	    test_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags))
		return;

	hexid = bin_to_hex_dup(clp->cl_name.data, clp->cl_name.len);
	if (!hexid) {
		dprintk("%s: can't allocate memory for upcall!\n", __func__);
		return;
	}

	has_session = nfsd4_cltrack_client_has_session(clp);
	grace_start = nfsd4_cltrack_grace_start(nn->boot_time);

	nfsd4_cltrack_upcall_lock(clp);
	if (!nfsd4_umh_cltrack_upcall("create", hexid, has_session, grace_start))
		set_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags);
	nfsd4_cltrack_upcall_unlock(clp);

	kfree(has_session);
	kfree(grace_start);
	kfree(hexid);
}

static void
nfsd4_umh_cltrack_remove(struct nfs4_client *clp)
{
	char *hexid;

	if (!test_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags))
		return;

	hexid = bin_to_hex_dup(clp->cl_name.data, clp->cl_name.len);
	if (!hexid) {
		dprintk("%s: can't allocate memory for upcall!\n", __func__);
		return;
	}

	nfsd4_cltrack_upcall_lock(clp);
	if (test_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags) &&
	    nfsd4_umh_cltrack_upcall("remove", hexid, NULL, NULL) == 0)
		clear_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags);
	nfsd4_cltrack_upcall_unlock(clp);

	kfree(hexid);
}

static int
nfsd4_umh_cltrack_check(struct nfs4_client *clp)
{
	int ret;
	char *hexid, *has_session, *legacy;

	if (test_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags))
		return 0;

	hexid = bin_to_hex_dup(clp->cl_name.data, clp->cl_name.len);
	if (!hexid) {
		dprintk("%s: can't allocate memory for upcall!\n", __func__);
		return -ENOMEM;
	}

	has_session = nfsd4_cltrack_client_has_session(clp);
	legacy = nfsd4_cltrack_legacy_recdir(&clp->cl_name);

	nfsd4_cltrack_upcall_lock(clp);
	if (test_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags)) {
		ret = 0;
	} else {
		ret = nfsd4_umh_cltrack_upcall("check", hexid, has_session, legacy);
		if (ret == 0)
			set_bit(NFSD4_CLIENT_STABLE, &clp->cl_flags);
	}
	nfsd4_cltrack_upcall_unlock(clp);
	kfree(has_session);
	kfree(legacy);
	kfree(hexid);

	return ret;
}

static void
nfsd4_umh_cltrack_grace_done(struct nfsd_net *nn)
{
	char *legacy;
	char timestr[22]; /* FIXME: better way to determine max size? */

	sprintf(timestr, "%ld", nn->boot_time);
	legacy = nfsd4_cltrack_legacy_topdir();
	nfsd4_umh_cltrack_upcall("gracedone", timestr, legacy, NULL);
	kfree(legacy);
}

static struct nfsd4_client_tracking_ops nfsd4_umh_tracking_ops = {
	.init		= nfsd4_umh_cltrack_init,
	.exit		= NULL,
	.create		= nfsd4_umh_cltrack_create,
	.remove		= nfsd4_umh_cltrack_remove,
	.check		= nfsd4_umh_cltrack_check,
	.grace_done	= nfsd4_umh_cltrack_grace_done,
};

int
nfsd4_client_tracking_init(struct net *net)
{
	int status;
	struct path path;
	struct nfsd_net *nn = net_generic(net, nfsd_net_id);

	/* just run the init if it the method is already decided */
	if (nn->client_tracking_ops)
		goto do_init;

	/*
	 * First, try a UMH upcall. It should succeed or fail quickly, so
	 * there's little harm in trying that first.
	 */
	nn->client_tracking_ops = &nfsd4_umh_tracking_ops;
	status = nn->client_tracking_ops->init(net);
	if (!status)
		return status;

	/*
	 * See if the recoverydir exists and is a directory. If it is,
	 * then use the legacy ops.
	 */
	nn->client_tracking_ops = &nfsd4_legacy_tracking_ops;
	status = kern_path(nfs4_recoverydir(), LOOKUP_FOLLOW, &path);
	if (!status) {
		status = d_is_dir(path.dentry);
		path_put(&path);
		if (status)
			goto do_init;
	}

	/* Finally, try to use nfsdcld */
	nn->client_tracking_ops = &nfsd4_cld_tracking_ops;
	printk(KERN_WARNING "NFSD: the nfsdcld client tracking upcall will be "
			"removed in 3.10. Please transition to using "
			"nfsdcltrack.\n");
do_init:
	status = nn->client_tracking_ops->init(net);
	if (status) {
		printk(KERN_WARNING "NFSD: Unable to initialize client "
				    "recovery tracking! (%d)\n", status);
		nn->client_tracking_ops = NULL;
	}
	return status;
}

void
nfsd4_client_tracking_exit(struct net *net)
{
	struct nfsd_net *nn = net_generic(net, nfsd_net_id);

	if (nn->client_tracking_ops) {
		if (nn->client_tracking_ops->exit)
			nn->client_tracking_ops->exit(net);
		nn->client_tracking_ops = NULL;
	}
}

void
nfsd4_client_record_create(struct nfs4_client *clp)
{
	struct nfsd_net *nn = net_generic(clp->net, nfsd_net_id);

	if (nn->client_tracking_ops)
		nn->client_tracking_ops->create(clp);
}

void
nfsd4_client_record_remove(struct nfs4_client *clp)
{
	struct nfsd_net *nn = net_generic(clp->net, nfsd_net_id);

	if (nn->client_tracking_ops)
		nn->client_tracking_ops->remove(clp);
}

int
nfsd4_client_record_check(struct nfs4_client *clp)
{
	struct nfsd_net *nn = net_generic(clp->net, nfsd_net_id);

	if (nn->client_tracking_ops)
		return nn->client_tracking_ops->check(clp);

	return -EOPNOTSUPP;
}

void
nfsd4_record_grace_done(struct nfsd_net *nn)
{
	if (nn->client_tracking_ops)
		nn->client_tracking_ops->grace_done(nn);
}

static int
rpc_pipefs_event(struct notifier_block *nb, unsigned long event, void *ptr)
{
	struct super_block *sb = ptr;
	struct net *net = sb->s_fs_info;
	struct nfsd_net *nn = net_generic(net, nfsd_net_id);
	struct cld_net *cn = nn->cld_net;
	struct dentry *dentry;
	int ret = 0;

	if (!try_module_get(THIS_MODULE))
		return 0;

	if (!cn) {
		module_put(THIS_MODULE);
		return 0;
	}

	switch (event) {
	case RPC_PIPEFS_MOUNT:
		dentry = nfsd4_cld_register_sb(sb, cn->cn_pipe);
		if (IS_ERR(dentry)) {
			ret = PTR_ERR(dentry);
			break;
		}
		cn->cn_pipe->dentry = dentry;
		break;
	case RPC_PIPEFS_UMOUNT:
		if (cn->cn_pipe->dentry)
			nfsd4_cld_unregister_sb(cn->cn_pipe);
		break;
	default:
		ret = -ENOTSUPP;
		break;
	}
	module_put(THIS_MODULE);
	return ret;
}

static struct notifier_block nfsd4_cld_block = {
	.notifier_call = rpc_pipefs_event,
};

int
register_cld_notifier(void)
{
	return rpc_pipefs_notifier_register(&nfsd4_cld_block);
}

void
unregister_cld_notifier(void)
{
	rpc_pipefs_notifier_unregister(&nfsd4_cld_block);
}
