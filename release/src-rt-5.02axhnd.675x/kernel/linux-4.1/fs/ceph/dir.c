#include <linux/ceph/ceph_debug.h>

#include <linux/spinlock.h>
#include <linux/fs_struct.h>
#include <linux/namei.h>
#include <linux/slab.h>
#include <linux/sched.h>

#include "super.h"
#include "mds_client.h"

/*
 * Directory operations: readdir, lookup, create, link, unlink,
 * rename, etc.
 */

/*
 * Ceph MDS operations are specified in terms of a base ino and
 * relative path.  Thus, the client can specify an operation on a
 * specific inode (e.g., a getattr due to fstat(2)), or as a path
 * relative to, say, the root directory.
 *
 * Normally, we limit ourselves to strict inode ops (no path component)
 * or dentry operations (a single path component relative to an ino).  The
 * exception to this is open_root_dentry(), which will open the mount
 * point by name.
 */

const struct dentry_operations ceph_dentry_ops;

/*
 * Initialize ceph dentry state.
 */
int ceph_init_dentry(struct dentry *dentry)
{
	struct ceph_dentry_info *di;

	if (dentry->d_fsdata)
		return 0;

	di = kmem_cache_alloc(ceph_dentry_cachep, GFP_NOFS | __GFP_ZERO);
	if (!di)
		return -ENOMEM;          /* oh well */

	spin_lock(&dentry->d_lock);
	if (dentry->d_fsdata) {
		/* lost a race */
		kmem_cache_free(ceph_dentry_cachep, di);
		goto out_unlock;
	}

	if (ceph_snap(d_inode(dentry->d_parent)) == CEPH_NOSNAP)
		d_set_d_op(dentry, &ceph_dentry_ops);
	else if (ceph_snap(d_inode(dentry->d_parent)) == CEPH_SNAPDIR)
		d_set_d_op(dentry, &ceph_snapdir_dentry_ops);
	else
		d_set_d_op(dentry, &ceph_snap_dentry_ops);

	di->dentry = dentry;
	di->lease_session = NULL;
	dentry->d_time = jiffies;
	/* avoid reordering d_fsdata setup so that the check above is safe */
	smp_mb();
	dentry->d_fsdata = di;
	ceph_dentry_lru_add(dentry);
out_unlock:
	spin_unlock(&dentry->d_lock);
	return 0;
}

struct inode *ceph_get_dentry_parent_inode(struct dentry *dentry)
{
	struct inode *inode = NULL;

	if (!dentry)
		return NULL;

	spin_lock(&dentry->d_lock);
	if (!IS_ROOT(dentry)) {
		inode = d_inode(dentry->d_parent);
		ihold(inode);
	}
	spin_unlock(&dentry->d_lock);
	return inode;
}


/*
 * for readdir, we encode the directory frag and offset within that
 * frag into f_pos.
 */
static unsigned fpos_frag(loff_t p)
{
	return p >> 32;
}
static unsigned fpos_off(loff_t p)
{
	return p & 0xffffffff;
}

static int fpos_cmp(loff_t l, loff_t r)
{
	int v = ceph_frag_compare(fpos_frag(l), fpos_frag(r));
	if (v)
		return v;
	return (int)(fpos_off(l) - fpos_off(r));
}

/*
 * When possible, we try to satisfy a readdir by peeking at the
 * dcache.  We make this work by carefully ordering dentries on
 * d_child when we initially get results back from the MDS, and
 * falling back to a "normal" sync readdir if any dentries in the dir
 * are dropped.
 *
 * Complete dir indicates that we have all dentries in the dir.  It is
 * defined IFF we hold CEPH_CAP_FILE_SHARED (which will be revoked by
 * the MDS if/when the directory is modified).
 */
static int __dcache_readdir(struct file *file,  struct dir_context *ctx,
			    u32 shared_gen)
{
	struct ceph_file_info *fi = file->private_data;
	struct dentry *parent = file->f_path.dentry;
	struct inode *dir = d_inode(parent);
	struct list_head *p;
	struct dentry *dentry, *last;
	struct ceph_dentry_info *di;
	int err = 0;

	/* claim ref on last dentry we returned */
	last = fi->dentry;
	fi->dentry = NULL;

	dout("__dcache_readdir %p v%u at %llu (last %p)\n",
	     dir, shared_gen, ctx->pos, last);

	spin_lock(&parent->d_lock);

	/* start at beginning? */
	if (ctx->pos == 2 || last == NULL ||
	    fpos_cmp(ctx->pos, ceph_dentry(last)->offset) < 0) {
		if (list_empty(&parent->d_subdirs))
			goto out_unlock;
		p = parent->d_subdirs.prev;
		dout(" initial p %p/%p\n", p->prev, p->next);
	} else {
		p = last->d_child.prev;
	}

more:
	dentry = list_entry(p, struct dentry, d_child);
	di = ceph_dentry(dentry);
	while (1) {
		dout(" p %p/%p %s d_subdirs %p/%p\n", p->prev, p->next,
		     d_unhashed(dentry) ? "!hashed" : "hashed",
		     parent->d_subdirs.prev, parent->d_subdirs.next);
		if (p == &parent->d_subdirs) {
			fi->flags |= CEPH_F_ATEND;
			goto out_unlock;
		}
		spin_lock_nested(&dentry->d_lock, DENTRY_D_LOCK_NESTED);
		if (di->lease_shared_gen == shared_gen &&
		    !d_unhashed(dentry) && d_really_is_positive(dentry) &&
		    ceph_snap(d_inode(dentry)) != CEPH_SNAPDIR &&
		    ceph_ino(d_inode(dentry)) != CEPH_INO_CEPH &&
		    fpos_cmp(ctx->pos, di->offset) <= 0)
			break;
		dout(" skipping %p %pd at %llu (%llu)%s%s\n", dentry,
		     dentry, di->offset,
		     ctx->pos, d_unhashed(dentry) ? " unhashed" : "",
		     !d_inode(dentry) ? " null" : "");
		spin_unlock(&dentry->d_lock);
		p = p->prev;
		dentry = list_entry(p, struct dentry, d_child);
		di = ceph_dentry(dentry);
	}

	dget_dlock(dentry);
	spin_unlock(&dentry->d_lock);
	spin_unlock(&parent->d_lock);

	/* make sure a dentry wasn't dropped while we didn't have parent lock */
	if (!ceph_dir_is_complete_ordered(dir)) {
		dout(" lost dir complete on %p; falling back to mds\n", dir);
		dput(dentry);
		err = -EAGAIN;
		goto out;
	}

	dout(" %llu (%llu) dentry %p %pd %p\n", di->offset, ctx->pos,
	     dentry, dentry, d_inode(dentry));
	if (!dir_emit(ctx, dentry->d_name.name,
		      dentry->d_name.len,
		      ceph_translate_ino(dentry->d_sb, d_inode(dentry)->i_ino),
		      d_inode(dentry)->i_mode >> 12)) {
		if (last) {
			/* remember our position */
			fi->dentry = last;
			fi->next_offset = fpos_off(di->offset);
		}
		dput(dentry);
		return 0;
	}

	ctx->pos = di->offset + 1;

	if (last)
		dput(last);
	last = dentry;

	spin_lock(&parent->d_lock);
	p = p->prev;	/* advance to next dentry */
	goto more;

out_unlock:
	spin_unlock(&parent->d_lock);
out:
	if (last)
		dput(last);
	return err;
}

/*
 * make note of the last dentry we read, so we can
 * continue at the same lexicographical point,
 * regardless of what dir changes take place on the
 * server.
 */
static int note_last_dentry(struct ceph_file_info *fi, const char *name,
			    int len)
{
	kfree(fi->last_name);
	fi->last_name = kmalloc(len+1, GFP_NOFS);
	if (!fi->last_name)
		return -ENOMEM;
	memcpy(fi->last_name, name, len);
	fi->last_name[len] = 0;
	dout("note_last_dentry '%s'\n", fi->last_name);
	return 0;
}

static int ceph_readdir(struct file *file, struct dir_context *ctx)
{
	struct ceph_file_info *fi = file->private_data;
	struct inode *inode = file_inode(file);
	struct ceph_inode_info *ci = ceph_inode(inode);
	struct ceph_fs_client *fsc = ceph_inode_to_client(inode);
	struct ceph_mds_client *mdsc = fsc->mdsc;
	unsigned frag = fpos_frag(ctx->pos);
	int off = fpos_off(ctx->pos);
	int err;
	u32 ftype;
	struct ceph_mds_reply_info_parsed *rinfo;

	dout("readdir %p file %p frag %u off %u\n", inode, file, frag, off);
	if (fi->flags & CEPH_F_ATEND)
		return 0;

	/* always start with . and .. */
	if (ctx->pos == 0) {
		dout("readdir off 0 -> '.'\n");
		if (!dir_emit(ctx, ".", 1, 
			    ceph_translate_ino(inode->i_sb, inode->i_ino),
			    inode->i_mode >> 12))
			return 0;
		ctx->pos = 1;
		off = 1;
	}
	if (ctx->pos == 1) {
		ino_t ino = parent_ino(file->f_path.dentry);
		dout("readdir off 1 -> '..'\n");
		if (!dir_emit(ctx, "..", 2,
			    ceph_translate_ino(inode->i_sb, ino),
			    inode->i_mode >> 12))
			return 0;
		ctx->pos = 2;
		off = 2;
	}

	/* can we use the dcache? */
	spin_lock(&ci->i_ceph_lock);
	if ((ctx->pos == 2 || fi->dentry) &&
	    ceph_test_mount_opt(fsc, DCACHE) &&
	    !ceph_test_mount_opt(fsc, NOASYNCREADDIR) &&
	    ceph_snap(inode) != CEPH_SNAPDIR &&
	    __ceph_dir_is_complete_ordered(ci) &&
	    __ceph_caps_issued_mask(ci, CEPH_CAP_FILE_SHARED, 1)) {
		u32 shared_gen = ci->i_shared_gen;
		spin_unlock(&ci->i_ceph_lock);
		err = __dcache_readdir(file, ctx, shared_gen);
		if (err != -EAGAIN)
			return err;
		frag = fpos_frag(ctx->pos);
		off = fpos_off(ctx->pos);
	} else {
		spin_unlock(&ci->i_ceph_lock);
	}
	if (fi->dentry) {
		err = note_last_dentry(fi, fi->dentry->d_name.name,
				       fi->dentry->d_name.len);
		if (err)
			return err;
		dput(fi->dentry);
		fi->dentry = NULL;
	}

	/* proceed with a normal readdir */

	if (ctx->pos == 2) {
		/* note dir version at start of readdir so we can tell
		 * if any dentries get dropped */
		fi->dir_release_count = atomic_read(&ci->i_release_count);
		fi->dir_ordered_count = ci->i_ordered_count;
	}

more:
	/* do we have the correct frag content buffered? */
	if (fi->frag != frag || fi->last_readdir == NULL) {
		struct ceph_mds_request *req;
		int op = ceph_snap(inode) == CEPH_SNAPDIR ?
			CEPH_MDS_OP_LSSNAP : CEPH_MDS_OP_READDIR;

		/* discard old result, if any */
		if (fi->last_readdir) {
			ceph_mdsc_put_request(fi->last_readdir);
			fi->last_readdir = NULL;
		}

		dout("readdir fetching %llx.%llx frag %x offset '%s'\n",
		     ceph_vinop(inode), frag, fi->last_name);
		req = ceph_mdsc_create_request(mdsc, op, USE_AUTH_MDS);
		if (IS_ERR(req))
			return PTR_ERR(req);
		err = ceph_alloc_readdir_reply_buffer(req, inode);
		if (err) {
			ceph_mdsc_put_request(req);
			return err;
		}
		/* hints to request -> mds selection code */
		req->r_direct_mode = USE_AUTH_MDS;
		req->r_direct_hash = ceph_frag_value(frag);
		req->r_direct_is_hash = true;
		if (fi->last_name) {
			req->r_path2 = kstrdup(fi->last_name, GFP_NOFS);
			if (!req->r_path2) {
				ceph_mdsc_put_request(req);
				return -ENOMEM;
			}
		}
		req->r_readdir_offset = fi->next_offset;
		req->r_args.readdir.frag = cpu_to_le32(frag);

		req->r_inode = inode;
		ihold(inode);
		req->r_dentry = dget(file->f_path.dentry);
		err = ceph_mdsc_do_request(mdsc, NULL, req);
		if (err < 0) {
			ceph_mdsc_put_request(req);
			return err;
		}
		dout("readdir got and parsed readdir result=%d"
		     " on frag %x, end=%d, complete=%d\n", err, frag,
		     (int)req->r_reply_info.dir_end,
		     (int)req->r_reply_info.dir_complete);

		if (!req->r_did_prepopulate) {
			dout("readdir !did_prepopulate");
			/* preclude from marking dir complete */
			fi->dir_release_count--;
		}

		/* note next offset and last dentry name */
		rinfo = &req->r_reply_info;
		if (le32_to_cpu(rinfo->dir_dir->frag) != frag) {
			frag = le32_to_cpu(rinfo->dir_dir->frag);
			if (ceph_frag_is_leftmost(frag))
				fi->next_offset = 2;
			else
				fi->next_offset = 0;
			off = fi->next_offset;
		}
		fi->frag = frag;
		fi->offset = fi->next_offset;
		fi->last_readdir = req;

		if (req->r_reply_info.dir_end) {
			kfree(fi->last_name);
			fi->last_name = NULL;
			if (ceph_frag_is_rightmost(frag))
				fi->next_offset = 2;
			else
				fi->next_offset = 0;
		} else {
			err = note_last_dentry(fi,
				       rinfo->dir_dname[rinfo->dir_nr-1],
				       rinfo->dir_dname_len[rinfo->dir_nr-1]);
			if (err)
				return err;
			fi->next_offset += rinfo->dir_nr;
		}
	}

	rinfo = &fi->last_readdir->r_reply_info;
	dout("readdir frag %x num %d off %d chunkoff %d\n", frag,
	     rinfo->dir_nr, off, fi->offset);

	ctx->pos = ceph_make_fpos(frag, off);
	while (off >= fi->offset && off - fi->offset < rinfo->dir_nr) {
		struct ceph_mds_reply_inode *in =
			rinfo->dir_in[off - fi->offset].in;
		struct ceph_vino vino;
		ino_t ino;

		dout("readdir off %d (%d/%d) -> %lld '%.*s' %p\n",
		     off, off - fi->offset, rinfo->dir_nr, ctx->pos,
		     rinfo->dir_dname_len[off - fi->offset],
		     rinfo->dir_dname[off - fi->offset], in);
		BUG_ON(!in);
		ftype = le32_to_cpu(in->mode) >> 12;
		vino.ino = le64_to_cpu(in->ino);
		vino.snap = le64_to_cpu(in->snapid);
		ino = ceph_vino_to_ino(vino);
		if (!dir_emit(ctx,
			    rinfo->dir_dname[off - fi->offset],
			    rinfo->dir_dname_len[off - fi->offset],
			    ceph_translate_ino(inode->i_sb, ino), ftype)) {
			dout("filldir stopping us...\n");
			return 0;
		}
		off++;
		ctx->pos++;
	}

	if (fi->last_name) {
		ceph_mdsc_put_request(fi->last_readdir);
		fi->last_readdir = NULL;
		goto more;
	}

	/* more frags? */
	if (!ceph_frag_is_rightmost(frag)) {
		frag = ceph_frag_next(frag);
		off = 0;
		ctx->pos = ceph_make_fpos(frag, off);
		dout("readdir next frag is %x\n", frag);
		goto more;
	}
	fi->flags |= CEPH_F_ATEND;

	/*
	 * if dir_release_count still matches the dir, no dentries
	 * were released during the whole readdir, and we should have
	 * the complete dir contents in our cache.
	 */
	spin_lock(&ci->i_ceph_lock);
	if (atomic_read(&ci->i_release_count) == fi->dir_release_count) {
		if (ci->i_ordered_count == fi->dir_ordered_count)
			dout(" marking %p complete and ordered\n", inode);
		else
			dout(" marking %p complete\n", inode);
		__ceph_dir_set_complete(ci, fi->dir_release_count,
					fi->dir_ordered_count);
	}
	spin_unlock(&ci->i_ceph_lock);

	dout("readdir %p file %p done.\n", inode, file);
	return 0;
}

static void reset_readdir(struct ceph_file_info *fi, unsigned frag)
{
	if (fi->last_readdir) {
		ceph_mdsc_put_request(fi->last_readdir);
		fi->last_readdir = NULL;
	}
	kfree(fi->last_name);
	fi->last_name = NULL;
	if (ceph_frag_is_leftmost(frag))
		fi->next_offset = 2;  /* compensate for . and .. */
	else
		fi->next_offset = 0;
	if (fi->dentry) {
		dput(fi->dentry);
		fi->dentry = NULL;
	}
	fi->flags &= ~CEPH_F_ATEND;
}

static loff_t ceph_dir_llseek(struct file *file, loff_t offset, int whence)
{
	struct ceph_file_info *fi = file->private_data;
	struct inode *inode = file->f_mapping->host;
	loff_t old_offset = ceph_make_fpos(fi->frag, fi->next_offset);
	loff_t retval;

	mutex_lock(&inode->i_mutex);
	retval = -EINVAL;
	switch (whence) {
	case SEEK_END:
		offset += inode->i_size + 2;   /* FIXME */
		break;
	case SEEK_CUR:
		offset += file->f_pos;
	case SEEK_SET:
		break;
	default:
		goto out;
	}

	if (offset >= 0) {
		if (offset != file->f_pos) {
			file->f_pos = offset;
			file->f_version = 0;
			fi->flags &= ~CEPH_F_ATEND;
		}
		retval = offset;

		/*
		 * discard buffered readdir content on seekdir(0), or
		 * seek to new frag, or seek prior to current chunk.
		 */
		if (offset == 0 ||
		    fpos_frag(offset) != fi->frag ||
		    fpos_off(offset) < fi->offset) {
			dout("dir_llseek dropping %p content\n", file);
			reset_readdir(fi, fpos_frag(offset));
		}

		/* bump dir_release_count if we did a forward seek */
		if (fpos_cmp(offset, old_offset) > 0)
			fi->dir_release_count--;
	}
out:
	mutex_unlock(&inode->i_mutex);
	return retval;
}

/*
 * Handle lookups for the hidden .snap directory.
 */
int ceph_handle_snapdir(struct ceph_mds_request *req,
			struct dentry *dentry, int err)
{
	struct ceph_fs_client *fsc = ceph_sb_to_client(dentry->d_sb);
	struct inode *parent = d_inode(dentry->d_parent); /* we hold i_mutex */

	/* .snap dir? */
	if (err == -ENOENT &&
	    ceph_snap(parent) == CEPH_NOSNAP &&
	    strcmp(dentry->d_name.name,
		   fsc->mount_options->snapdir_name) == 0) {
		struct inode *inode = ceph_get_snapdir(parent);
		dout("ENOENT on snapdir %p '%pd', linking to snapdir %p\n",
		     dentry, dentry, inode);
		BUG_ON(!d_unhashed(dentry));
		d_add(dentry, inode);
		err = 0;
	}
	return err;
}

/*
 * Figure out final result of a lookup/open request.
 *
 * Mainly, make sure we return the final req->r_dentry (if it already
 * existed) in place of the original VFS-provided dentry when they
 * differ.
 *
 * Gracefully handle the case where the MDS replies with -ENOENT and
 * no trace (which it may do, at its discretion, e.g., if it doesn't
 * care to issue a lease on the negative dentry).
 */
struct dentry *ceph_finish_lookup(struct ceph_mds_request *req,
				  struct dentry *dentry, int err)
{
	if (err == -ENOENT) {
		/* no trace? */
		err = 0;
		if (!req->r_reply_info.head->is_dentry) {
			dout("ENOENT and no trace, dentry %p inode %p\n",
			     dentry, d_inode(dentry));
			if (d_really_is_positive(dentry)) {
				d_drop(dentry);
				err = -ENOENT;
			} else {
				d_add(dentry, NULL);
			}
		}
	}
	if (err)
		dentry = ERR_PTR(err);
	else if (dentry != req->r_dentry)
		dentry = dget(req->r_dentry);   /* we got spliced */
	else
		dentry = NULL;
	return dentry;
}

static int is_root_ceph_dentry(struct inode *inode, struct dentry *dentry)
{
	return ceph_ino(inode) == CEPH_INO_ROOT &&
		strncmp(dentry->d_name.name, ".ceph", 5) == 0;
}

/*
 * Look up a single dir entry.  If there is a lookup intent, inform
 * the MDS so that it gets our 'caps wanted' value in a single op.
 */
static struct dentry *ceph_lookup(struct inode *dir, struct dentry *dentry,
				  unsigned int flags)
{
	struct ceph_fs_client *fsc = ceph_sb_to_client(dir->i_sb);
	struct ceph_mds_client *mdsc = fsc->mdsc;
	struct ceph_mds_request *req;
	int op;
	int err;

	dout("lookup %p dentry %p '%pd'\n",
	     dir, dentry, dentry);

	if (dentry->d_name.len > NAME_MAX)
		return ERR_PTR(-ENAMETOOLONG);

	err = ceph_init_dentry(dentry);
	if (err < 0)
		return ERR_PTR(err);

	/* can we conclude ENOENT locally? */
	if (d_really_is_negative(dentry)) {
		struct ceph_inode_info *ci = ceph_inode(dir);
		struct ceph_dentry_info *di = ceph_dentry(dentry);

		spin_lock(&ci->i_ceph_lock);
		dout(" dir %p flags are %d\n", dir, ci->i_ceph_flags);
		if (strncmp(dentry->d_name.name,
			    fsc->mount_options->snapdir_name,
			    dentry->d_name.len) &&
		    !is_root_ceph_dentry(dir, dentry) &&
		    ceph_test_mount_opt(fsc, DCACHE) &&
		    __ceph_dir_is_complete(ci) &&
		    (__ceph_caps_issued_mask(ci, CEPH_CAP_FILE_SHARED, 1))) {
			spin_unlock(&ci->i_ceph_lock);
			dout(" dir %p complete, -ENOENT\n", dir);
			d_add(dentry, NULL);
			di->lease_shared_gen = ci->i_shared_gen;
			return NULL;
		}
		spin_unlock(&ci->i_ceph_lock);
	}

	op = ceph_snap(dir) == CEPH_SNAPDIR ?
		CEPH_MDS_OP_LOOKUPSNAP : CEPH_MDS_OP_LOOKUP;
	req = ceph_mdsc_create_request(mdsc, op, USE_ANY_MDS);
	if (IS_ERR(req))
		return ERR_CAST(req);
	req->r_dentry = dget(dentry);
	req->r_num_caps = 2;
	/* we only need inode linkage */
	req->r_args.getattr.mask = cpu_to_le32(CEPH_STAT_CAP_INODE);
	req->r_locked_dir = dir;
	err = ceph_mdsc_do_request(mdsc, NULL, req);
	err = ceph_handle_snapdir(req, dentry, err);
	dentry = ceph_finish_lookup(req, dentry, err);
	ceph_mdsc_put_request(req);  /* will dput(dentry) */
	dout("lookup result=%p\n", dentry);
	return dentry;
}

/*
 * If we do a create but get no trace back from the MDS, follow up with
 * a lookup (the VFS expects us to link up the provided dentry).
 */
int ceph_handle_notrace_create(struct inode *dir, struct dentry *dentry)
{
	struct dentry *result = ceph_lookup(dir, dentry, 0);

	if (result && !IS_ERR(result)) {
		/*
		 * We created the item, then did a lookup, and found
		 * it was already linked to another inode we already
		 * had in our cache (and thus got spliced). To not
		 * confuse VFS (especially when inode is a directory),
		 * we don't link our dentry to that inode, return an
		 * error instead.
		 *
		 * This event should be rare and it happens only when
		 * we talk to old MDS. Recent MDS does not send traceless
		 * reply for request that creates new inode.
		 */
		d_drop(result);
		return -ESTALE;
	}
	return PTR_ERR(result);
}

static int ceph_mknod(struct inode *dir, struct dentry *dentry,
		      umode_t mode, dev_t rdev)
{
	struct ceph_fs_client *fsc = ceph_sb_to_client(dir->i_sb);
	struct ceph_mds_client *mdsc = fsc->mdsc;
	struct ceph_mds_request *req;
	struct ceph_acls_info acls = {};
	int err;

	if (ceph_snap(dir) != CEPH_NOSNAP)
		return -EROFS;

	err = ceph_pre_init_acls(dir, &mode, &acls);
	if (err < 0)
		return err;

	dout("mknod in dir %p dentry %p mode 0%ho rdev %d\n",
	     dir, dentry, mode, rdev);
	req = ceph_mdsc_create_request(mdsc, CEPH_MDS_OP_MKNOD, USE_AUTH_MDS);
	if (IS_ERR(req)) {
		err = PTR_ERR(req);
		goto out;
	}
	req->r_dentry = dget(dentry);
	req->r_num_caps = 2;
	req->r_locked_dir = dir;
	req->r_args.mknod.mode = cpu_to_le32(mode);
	req->r_args.mknod.rdev = cpu_to_le32(rdev);
	req->r_dentry_drop = CEPH_CAP_FILE_SHARED;
	req->r_dentry_unless = CEPH_CAP_FILE_EXCL;
	if (acls.pagelist) {
		req->r_pagelist = acls.pagelist;
		acls.pagelist = NULL;
	}
	err = ceph_mdsc_do_request(mdsc, dir, req);
	if (!err && !req->r_reply_info.head->is_dentry)
		err = ceph_handle_notrace_create(dir, dentry);
	ceph_mdsc_put_request(req);
out:
	if (!err)
		ceph_init_inode_acls(d_inode(dentry), &acls);
	else
		d_drop(dentry);
	ceph_release_acls_info(&acls);
	return err;
}

static int ceph_create(struct inode *dir, struct dentry *dentry, umode_t mode,
		       bool excl)
{
	return ceph_mknod(dir, dentry, mode, 0);
}

static int ceph_symlink(struct inode *dir, struct dentry *dentry,
			    const char *dest)
{
	struct ceph_fs_client *fsc = ceph_sb_to_client(dir->i_sb);
	struct ceph_mds_client *mdsc = fsc->mdsc;
	struct ceph_mds_request *req;
	int err;

	if (ceph_snap(dir) != CEPH_NOSNAP)
		return -EROFS;

	dout("symlink in dir %p dentry %p to '%s'\n", dir, dentry, dest);
	req = ceph_mdsc_create_request(mdsc, CEPH_MDS_OP_SYMLINK, USE_AUTH_MDS);
	if (IS_ERR(req)) {
		err = PTR_ERR(req);
		goto out;
	}
	req->r_path2 = kstrdup(dest, GFP_NOFS);
	if (!req->r_path2) {
		err = -ENOMEM;
		ceph_mdsc_put_request(req);
		goto out;
	}
	req->r_locked_dir = dir;
	req->r_dentry = dget(dentry);
	req->r_num_caps = 2;
	req->r_dentry_drop = CEPH_CAP_FILE_SHARED;
	req->r_dentry_unless = CEPH_CAP_FILE_EXCL;
	err = ceph_mdsc_do_request(mdsc, dir, req);
	if (!err && !req->r_reply_info.head->is_dentry)
		err = ceph_handle_notrace_create(dir, dentry);
	ceph_mdsc_put_request(req);
out:
	if (err)
		d_drop(dentry);
	return err;
}

static int ceph_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	struct ceph_fs_client *fsc = ceph_sb_to_client(dir->i_sb);
	struct ceph_mds_client *mdsc = fsc->mdsc;
	struct ceph_mds_request *req;
	struct ceph_acls_info acls = {};
	int err = -EROFS;
	int op;

	if (ceph_snap(dir) == CEPH_SNAPDIR) {
		/* mkdir .snap/foo is a MKSNAP */
		op = CEPH_MDS_OP_MKSNAP;
		dout("mksnap dir %p snap '%pd' dn %p\n", dir,
		     dentry, dentry);
	} else if (ceph_snap(dir) == CEPH_NOSNAP) {
		dout("mkdir dir %p dn %p mode 0%ho\n", dir, dentry, mode);
		op = CEPH_MDS_OP_MKDIR;
	} else {
		goto out;
	}

	mode |= S_IFDIR;
	err = ceph_pre_init_acls(dir, &mode, &acls);
	if (err < 0)
		goto out;

	req = ceph_mdsc_create_request(mdsc, op, USE_AUTH_MDS);
	if (IS_ERR(req)) {
		err = PTR_ERR(req);
		goto out;
	}

	req->r_dentry = dget(dentry);
	req->r_num_caps = 2;
	req->r_locked_dir = dir;
	req->r_args.mkdir.mode = cpu_to_le32(mode);
	req->r_dentry_drop = CEPH_CAP_FILE_SHARED;
	req->r_dentry_unless = CEPH_CAP_FILE_EXCL;
	if (acls.pagelist) {
		req->r_pagelist = acls.pagelist;
		acls.pagelist = NULL;
	}
	err = ceph_mdsc_do_request(mdsc, dir, req);
	if (!err &&
	    !req->r_reply_info.head->is_target &&
	    !req->r_reply_info.head->is_dentry)
		err = ceph_handle_notrace_create(dir, dentry);
	ceph_mdsc_put_request(req);
out:
	if (!err)
		ceph_init_inode_acls(d_inode(dentry), &acls);
	else
		d_drop(dentry);
	ceph_release_acls_info(&acls);
	return err;
}

static int ceph_link(struct dentry *old_dentry, struct inode *dir,
		     struct dentry *dentry)
{
	struct ceph_fs_client *fsc = ceph_sb_to_client(dir->i_sb);
	struct ceph_mds_client *mdsc = fsc->mdsc;
	struct ceph_mds_request *req;
	int err;

	if (ceph_snap(dir) != CEPH_NOSNAP)
		return -EROFS;

	dout("link in dir %p old_dentry %p dentry %p\n", dir,
	     old_dentry, dentry);
	req = ceph_mdsc_create_request(mdsc, CEPH_MDS_OP_LINK, USE_AUTH_MDS);
	if (IS_ERR(req)) {
		d_drop(dentry);
		return PTR_ERR(req);
	}
	req->r_dentry = dget(dentry);
	req->r_num_caps = 2;
	req->r_old_dentry = dget(old_dentry);
	req->r_locked_dir = dir;
	req->r_dentry_drop = CEPH_CAP_FILE_SHARED;
	req->r_dentry_unless = CEPH_CAP_FILE_EXCL;
	/* release LINK_SHARED on source inode (mds will lock it) */
	req->r_old_inode_drop = CEPH_CAP_LINK_SHARED;
	err = ceph_mdsc_do_request(mdsc, dir, req);
	if (err) {
		d_drop(dentry);
	} else if (!req->r_reply_info.head->is_dentry) {
		ihold(d_inode(old_dentry));
		d_instantiate(dentry, d_inode(old_dentry));
	}
	ceph_mdsc_put_request(req);
	return err;
}

/*
 * For a soon-to-be unlinked file, drop the AUTH_RDCACHE caps.  If it
 * looks like the link count will hit 0, drop any other caps (other
 * than PIN) we don't specifically want (due to the file still being
 * open).
 */
static int drop_caps_for_unlink(struct inode *inode)
{
	struct ceph_inode_info *ci = ceph_inode(inode);
	int drop = CEPH_CAP_LINK_SHARED | CEPH_CAP_LINK_EXCL;

	spin_lock(&ci->i_ceph_lock);
	if (inode->i_nlink == 1) {
		drop |= ~(__ceph_caps_wanted(ci) | CEPH_CAP_PIN);
		ci->i_ceph_flags |= CEPH_I_NODELAY;
	}
	spin_unlock(&ci->i_ceph_lock);
	return drop;
}

/*
 * rmdir and unlink are differ only by the metadata op code
 */
static int ceph_unlink(struct inode *dir, struct dentry *dentry)
{
	struct ceph_fs_client *fsc = ceph_sb_to_client(dir->i_sb);
	struct ceph_mds_client *mdsc = fsc->mdsc;
	struct inode *inode = d_inode(dentry);
	struct ceph_mds_request *req;
	int err = -EROFS;
	int op;

	if (ceph_snap(dir) == CEPH_SNAPDIR) {
		/* rmdir .snap/foo is RMSNAP */
		dout("rmsnap dir %p '%pd' dn %p\n", dir, dentry, dentry);
		op = CEPH_MDS_OP_RMSNAP;
	} else if (ceph_snap(dir) == CEPH_NOSNAP) {
		dout("unlink/rmdir dir %p dn %p inode %p\n",
		     dir, dentry, inode);
		op = d_is_dir(dentry) ?
			CEPH_MDS_OP_RMDIR : CEPH_MDS_OP_UNLINK;
	} else
		goto out;
	req = ceph_mdsc_create_request(mdsc, op, USE_AUTH_MDS);
	if (IS_ERR(req)) {
		err = PTR_ERR(req);
		goto out;
	}
	req->r_dentry = dget(dentry);
	req->r_num_caps = 2;
	req->r_locked_dir = dir;
	req->r_dentry_drop = CEPH_CAP_FILE_SHARED;
	req->r_dentry_unless = CEPH_CAP_FILE_EXCL;
	req->r_inode_drop = drop_caps_for_unlink(inode);
	err = ceph_mdsc_do_request(mdsc, dir, req);
	if (!err && !req->r_reply_info.head->is_dentry)
		d_delete(dentry);
	ceph_mdsc_put_request(req);
out:
	return err;
}

static int ceph_rename(struct inode *old_dir, struct dentry *old_dentry,
		       struct inode *new_dir, struct dentry *new_dentry)
{
	struct ceph_fs_client *fsc = ceph_sb_to_client(old_dir->i_sb);
	struct ceph_mds_client *mdsc = fsc->mdsc;
	struct ceph_mds_request *req;
	int op = CEPH_MDS_OP_RENAME;
	int err;

	if (ceph_snap(old_dir) != ceph_snap(new_dir))
		return -EXDEV;
	if (ceph_snap(old_dir) != CEPH_NOSNAP) {
		if (old_dir == new_dir && ceph_snap(old_dir) == CEPH_SNAPDIR)
			op = CEPH_MDS_OP_RENAMESNAP;
		else
			return -EROFS;
	}
	dout("rename dir %p dentry %p to dir %p dentry %p\n",
	     old_dir, old_dentry, new_dir, new_dentry);
	req = ceph_mdsc_create_request(mdsc, op, USE_AUTH_MDS);
	if (IS_ERR(req))
		return PTR_ERR(req);
	ihold(old_dir);
	req->r_dentry = dget(new_dentry);
	req->r_num_caps = 2;
	req->r_old_dentry = dget(old_dentry);
	req->r_old_dentry_dir = old_dir;
	req->r_locked_dir = new_dir;
	req->r_old_dentry_drop = CEPH_CAP_FILE_SHARED;
	req->r_old_dentry_unless = CEPH_CAP_FILE_EXCL;
	req->r_dentry_drop = CEPH_CAP_FILE_SHARED;
	req->r_dentry_unless = CEPH_CAP_FILE_EXCL;
	/* release LINK_RDCACHE on source inode (mds will lock it) */
	req->r_old_inode_drop = CEPH_CAP_LINK_SHARED;
	if (d_really_is_positive(new_dentry))
		req->r_inode_drop = drop_caps_for_unlink(d_inode(new_dentry));
	err = ceph_mdsc_do_request(mdsc, old_dir, req);
	if (!err && !req->r_reply_info.head->is_dentry) {
		/*
		 * Normally d_move() is done by fill_trace (called by
		 * do_request, above).  If there is no trace, we need
		 * to do it here.
		 */

		d_move(old_dentry, new_dentry);

		/* ensure target dentry is invalidated, despite
		   rehashing bug in vfs_rename_dir */
		ceph_invalidate_dentry_lease(new_dentry);

		/* d_move screws up sibling dentries' offsets */
		ceph_dir_clear_complete(old_dir);
		ceph_dir_clear_complete(new_dir);

	}
	ceph_mdsc_put_request(req);
	return err;
}

/*
 * Ensure a dentry lease will no longer revalidate.
 */
void ceph_invalidate_dentry_lease(struct dentry *dentry)
{
	spin_lock(&dentry->d_lock);
	dentry->d_time = jiffies;
	ceph_dentry(dentry)->lease_shared_gen = 0;
	spin_unlock(&dentry->d_lock);
}

/*
 * Check if dentry lease is valid.  If not, delete the lease.  Try to
 * renew if the least is more than half up.
 */
static int dentry_lease_is_valid(struct dentry *dentry)
{
	struct ceph_dentry_info *di;
	struct ceph_mds_session *s;
	int valid = 0;
	u32 gen;
	unsigned long ttl;
	struct ceph_mds_session *session = NULL;
	struct inode *dir = NULL;
	u32 seq = 0;

	spin_lock(&dentry->d_lock);
	di = ceph_dentry(dentry);
	if (di->lease_session) {
		s = di->lease_session;
		spin_lock(&s->s_gen_ttl_lock);
		gen = s->s_cap_gen;
		ttl = s->s_cap_ttl;
		spin_unlock(&s->s_gen_ttl_lock);

		if (di->lease_gen == gen &&
		    time_before(jiffies, dentry->d_time) &&
		    time_before(jiffies, ttl)) {
			valid = 1;
			if (di->lease_renew_after &&
			    time_after(jiffies, di->lease_renew_after)) {
				/* we should renew */
				dir = d_inode(dentry->d_parent);
				session = ceph_get_mds_session(s);
				seq = di->lease_seq;
				di->lease_renew_after = 0;
				di->lease_renew_from = jiffies;
			}
		}
	}
	spin_unlock(&dentry->d_lock);

	if (session) {
		ceph_mdsc_lease_send_msg(session, dir, dentry,
					 CEPH_MDS_LEASE_RENEW, seq);
		ceph_put_mds_session(session);
	}
	dout("dentry_lease_is_valid - dentry %p = %d\n", dentry, valid);
	return valid;
}

/*
 * Check if directory-wide content lease/cap is valid.
 */
static int dir_lease_is_valid(struct inode *dir, struct dentry *dentry)
{
	struct ceph_inode_info *ci = ceph_inode(dir);
	struct ceph_dentry_info *di = ceph_dentry(dentry);
	int valid = 0;

	spin_lock(&ci->i_ceph_lock);
	if (ci->i_shared_gen == di->lease_shared_gen)
		valid = __ceph_caps_issued_mask(ci, CEPH_CAP_FILE_SHARED, 1);
	spin_unlock(&ci->i_ceph_lock);
	dout("dir_lease_is_valid dir %p v%u dentry %p v%u = %d\n",
	     dir, (unsigned)ci->i_shared_gen, dentry,
	     (unsigned)di->lease_shared_gen, valid);
	return valid;
}

/*
 * Check if cached dentry can be trusted.
 */
static int ceph_d_revalidate(struct dentry *dentry, unsigned int flags)
{
	int valid = 0;
	struct inode *dir;

	if (flags & LOOKUP_RCU)
		return -ECHILD;

	dout("d_revalidate %p '%pd' inode %p offset %lld\n", dentry,
	     dentry, d_inode(dentry), ceph_dentry(dentry)->offset);

	dir = ceph_get_dentry_parent_inode(dentry);

	/* always trust cached snapped dentries, snapdir dentry */
	if (ceph_snap(dir) != CEPH_NOSNAP) {
		dout("d_revalidate %p '%pd' inode %p is SNAPPED\n", dentry,
		     dentry, d_inode(dentry));
		valid = 1;
	} else if (d_really_is_positive(dentry) &&
		   ceph_snap(d_inode(dentry)) == CEPH_SNAPDIR) {
		valid = 1;
	} else if (dentry_lease_is_valid(dentry) ||
		   dir_lease_is_valid(dir, dentry)) {
		if (d_really_is_positive(dentry))
			valid = ceph_is_any_caps(d_inode(dentry));
		else
			valid = 1;
	}

	dout("d_revalidate %p %s\n", dentry, valid ? "valid" : "invalid");
	if (valid) {
		ceph_dentry_lru_touch(dentry);
	} else {
		ceph_dir_clear_complete(dir);
	}
	iput(dir);
	return valid;
}

/*
 * Release our ceph_dentry_info.
 */
static void ceph_d_release(struct dentry *dentry)
{
	struct ceph_dentry_info *di = ceph_dentry(dentry);

	dout("d_release %p\n", dentry);
	ceph_dentry_lru_del(dentry);
	if (di->lease_session)
		ceph_put_mds_session(di->lease_session);
	kmem_cache_free(ceph_dentry_cachep, di);
	dentry->d_fsdata = NULL;
}

static int ceph_snapdir_d_revalidate(struct dentry *dentry,
					  unsigned int flags)
{
	/*
	 * Eventually, we'll want to revalidate snapped metadata
	 * too... probably...
	 */
	return 1;
}

/*
 * When the VFS prunes a dentry from the cache, we need to clear the
 * complete flag on the parent directory.
 *
 * Called under dentry->d_lock.
 */
static void ceph_d_prune(struct dentry *dentry)
{
	dout("ceph_d_prune %p\n", dentry);

	/* do we have a valid parent? */
	if (IS_ROOT(dentry))
		return;

	/* if we are not hashed, we don't affect dir's completeness */
	if (d_unhashed(dentry))
		return;

	/*
	 * we hold d_lock, so d_parent is stable, and d_fsdata is never
	 * cleared until d_release
	 */
	ceph_dir_clear_complete(d_inode(dentry->d_parent));
}

/*
 * read() on a dir.  This weird interface hack only works if mounted
 * with '-o dirstat'.
 */
static ssize_t ceph_read_dir(struct file *file, char __user *buf, size_t size,
			     loff_t *ppos)
{
	struct ceph_file_info *cf = file->private_data;
	struct inode *inode = file_inode(file);
	struct ceph_inode_info *ci = ceph_inode(inode);
	int left;
	const int bufsize = 1024;

	if (!ceph_test_mount_opt(ceph_sb_to_client(inode->i_sb), DIRSTAT))
		return -EISDIR;

	if (!cf->dir_info) {
		cf->dir_info = kmalloc(bufsize, GFP_NOFS);
		if (!cf->dir_info)
			return -ENOMEM;
		cf->dir_info_len =
			snprintf(cf->dir_info, bufsize,
				"entries:   %20lld\n"
				" files:    %20lld\n"
				" subdirs:  %20lld\n"
				"rentries:  %20lld\n"
				" rfiles:   %20lld\n"
				" rsubdirs: %20lld\n"
				"rbytes:    %20lld\n"
				"rctime:    %10ld.%09ld\n",
				ci->i_files + ci->i_subdirs,
				ci->i_files,
				ci->i_subdirs,
				ci->i_rfiles + ci->i_rsubdirs,
				ci->i_rfiles,
				ci->i_rsubdirs,
				ci->i_rbytes,
				(long)ci->i_rctime.tv_sec,
				(long)ci->i_rctime.tv_nsec);
	}

	if (*ppos >= cf->dir_info_len)
		return 0;
	size = min_t(unsigned, size, cf->dir_info_len-*ppos);
	left = copy_to_user(buf, cf->dir_info + *ppos, size);
	if (left == size)
		return -EFAULT;
	*ppos += (size - left);
	return size - left;
}

/*
 * an fsync() on a dir will wait for any uncommitted directory
 * operations to commit.
 */
static int ceph_dir_fsync(struct file *file, loff_t start, loff_t end,
			  int datasync)
{
	struct inode *inode = file_inode(file);
	struct ceph_inode_info *ci = ceph_inode(inode);
	struct list_head *head = &ci->i_unsafe_dirops;
	struct ceph_mds_request *req;
	u64 last_tid;
	int ret = 0;

	dout("dir_fsync %p\n", inode);
	ret = filemap_write_and_wait_range(inode->i_mapping, start, end);
	if (ret)
		return ret;
	mutex_lock(&inode->i_mutex);

	spin_lock(&ci->i_unsafe_lock);
	if (list_empty(head))
		goto out;

	req = list_entry(head->prev,
			 struct ceph_mds_request, r_unsafe_dir_item);
	last_tid = req->r_tid;

	do {
		ceph_mdsc_get_request(req);
		spin_unlock(&ci->i_unsafe_lock);

		dout("dir_fsync %p wait on tid %llu (until %llu)\n",
		     inode, req->r_tid, last_tid);
		if (req->r_timeout) {
			unsigned long time_left = wait_for_completion_timeout(
							&req->r_safe_completion,
							req->r_timeout);
			if (time_left > 0)
				ret = 0;
			else
				ret = -EIO;  /* timed out */
		} else {
			wait_for_completion(&req->r_safe_completion);
		}
		ceph_mdsc_put_request(req);

		spin_lock(&ci->i_unsafe_lock);
		if (ret || list_empty(head))
			break;
		req = list_entry(head->next,
				 struct ceph_mds_request, r_unsafe_dir_item);
	} while (req->r_tid < last_tid);
out:
	spin_unlock(&ci->i_unsafe_lock);
	mutex_unlock(&inode->i_mutex);

	return ret;
}

/*
 * We maintain a private dentry LRU.
 *
 * FIXME: this needs to be changed to a per-mds lru to be useful.
 */
void ceph_dentry_lru_add(struct dentry *dn)
{
	struct ceph_dentry_info *di = ceph_dentry(dn);
	struct ceph_mds_client *mdsc;

	dout("dentry_lru_add %p %p '%pd'\n", di, dn, dn);
	mdsc = ceph_sb_to_client(dn->d_sb)->mdsc;
	spin_lock(&mdsc->dentry_lru_lock);
	list_add_tail(&di->lru, &mdsc->dentry_lru);
	mdsc->num_dentry++;
	spin_unlock(&mdsc->dentry_lru_lock);
}

void ceph_dentry_lru_touch(struct dentry *dn)
{
	struct ceph_dentry_info *di = ceph_dentry(dn);
	struct ceph_mds_client *mdsc;

	dout("dentry_lru_touch %p %p '%pd' (offset %lld)\n", di, dn, dn,
	     di->offset);
	mdsc = ceph_sb_to_client(dn->d_sb)->mdsc;
	spin_lock(&mdsc->dentry_lru_lock);
	list_move_tail(&di->lru, &mdsc->dentry_lru);
	spin_unlock(&mdsc->dentry_lru_lock);
}

void ceph_dentry_lru_del(struct dentry *dn)
{
	struct ceph_dentry_info *di = ceph_dentry(dn);
	struct ceph_mds_client *mdsc;

	dout("dentry_lru_del %p %p '%pd'\n", di, dn, dn);
	mdsc = ceph_sb_to_client(dn->d_sb)->mdsc;
	spin_lock(&mdsc->dentry_lru_lock);
	list_del_init(&di->lru);
	mdsc->num_dentry--;
	spin_unlock(&mdsc->dentry_lru_lock);
}

/*
 * Return name hash for a given dentry.  This is dependent on
 * the parent directory's hash function.
 */
unsigned ceph_dentry_hash(struct inode *dir, struct dentry *dn)
{
	struct ceph_inode_info *dci = ceph_inode(dir);

	switch (dci->i_dir_layout.dl_dir_hash) {
	case 0:	/* for backward compat */
	case CEPH_STR_HASH_LINUX:
		return dn->d_name.hash;

	default:
		return ceph_str_hash(dci->i_dir_layout.dl_dir_hash,
				     dn->d_name.name, dn->d_name.len);
	}
}

const struct file_operations ceph_dir_fops = {
	.read = ceph_read_dir,
	.iterate = ceph_readdir,
	.llseek = ceph_dir_llseek,
	.open = ceph_open,
	.release = ceph_release,
	.unlocked_ioctl = ceph_ioctl,
	.fsync = ceph_dir_fsync,
};

const struct file_operations ceph_snapdir_fops = {
	.iterate = ceph_readdir,
	.llseek = ceph_dir_llseek,
	.open = ceph_open,
	.release = ceph_release,
};

const struct inode_operations ceph_dir_iops = {
	.lookup = ceph_lookup,
	.permission = ceph_permission,
	.getattr = ceph_getattr,
	.setattr = ceph_setattr,
	.setxattr = ceph_setxattr,
	.getxattr = ceph_getxattr,
	.listxattr = ceph_listxattr,
	.removexattr = ceph_removexattr,
	.get_acl = ceph_get_acl,
	.set_acl = ceph_set_acl,
	.mknod = ceph_mknod,
	.symlink = ceph_symlink,
	.mkdir = ceph_mkdir,
	.link = ceph_link,
	.unlink = ceph_unlink,
	.rmdir = ceph_unlink,
	.rename = ceph_rename,
	.create = ceph_create,
	.atomic_open = ceph_atomic_open,
};

const struct inode_operations ceph_snapdir_iops = {
	.lookup = ceph_lookup,
	.permission = ceph_permission,
	.getattr = ceph_getattr,
	.mkdir = ceph_mkdir,
	.rmdir = ceph_unlink,
	.rename = ceph_rename,
};

const struct dentry_operations ceph_dentry_ops = {
	.d_revalidate = ceph_d_revalidate,
	.d_release = ceph_d_release,
	.d_prune = ceph_d_prune,
};

const struct dentry_operations ceph_snapdir_dentry_ops = {
	.d_revalidate = ceph_snapdir_d_revalidate,
	.d_release = ceph_d_release,
};

const struct dentry_operations ceph_snap_dentry_ops = {
	.d_release = ceph_d_release,
	.d_prune = ceph_d_prune,
};
