/*
 * mkquota.c --- create quota files for a filesystem
 *
 * Aditya Kali <adityakali@google.com>
 */
#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"
#include "e2p/e2p.h"

#include "quotaio.h"
#include "quotaio_v2.h"
#include "quotaio_tree.h"
#include "common.h"
#include "dict.h"

/* Needed for architectures where sizeof(int) != sizeof(void *) */
#define UINT_TO_VOIDPTR(val)  ((void *)(intptr_t)(val))
#define VOIDPTR_TO_UINT(ptr)  ((unsigned int)(intptr_t)(ptr))

#if DEBUG_QUOTA
static void print_inode(struct ext2_inode *inode)
{
	if (!inode)
		return;

	fprintf(stderr, "  i_mode = %d\n", inode->i_mode);
	fprintf(stderr, "  i_uid = %d\n", inode->i_uid);
	fprintf(stderr, "  i_size = %d\n", inode->i_size);
	fprintf(stderr, "  i_atime = %d\n", inode->i_atime);
	fprintf(stderr, "  i_ctime = %d\n", inode->i_ctime);
	fprintf(stderr, "  i_mtime = %d\n", inode->i_mtime);
	fprintf(stderr, "  i_dtime = %d\n", inode->i_dtime);
	fprintf(stderr, "  i_gid = %d\n", inode->i_gid);
	fprintf(stderr, "  i_links_count = %d\n", inode->i_links_count);
	fprintf(stderr, "  i_blocks = %d\n", inode->i_blocks);
	fprintf(stderr, "  i_flags = %d\n", inode->i_flags);

	return;
}

static void print_dquot(const char *desc, struct dquot *dq)
{
	if (desc)
		fprintf(stderr, "%s: ", desc);
	fprintf(stderr, "%u %lld:%lld:%lld %lld:%lld:%lld\n",
		dq->dq_id, (long long) dq->dq_dqb.dqb_curspace,
		(long long) dq->dq_dqb.dqb_bsoftlimit,
		(long long) dq->dq_dqb.dqb_bhardlimit,
		(long long) dq->dq_dqb.dqb_curinodes,
		(long long) dq->dq_dqb.dqb_isoftlimit,
		(long long) dq->dq_dqb.dqb_ihardlimit);
}
#else
static void print_dquot(const char *desc EXT2FS_ATTR((unused)),
			struct dquot *dq EXT2FS_ATTR((unused)))
{
}
#endif

/*
 * Returns 0 if not able to find the quota file, otherwise returns its
 * inode number.
 */
int quota_file_exists(ext2_filsys fs, enum quota_type qtype)
{
	char qf_name[256];
	errcode_t ret;
	ext2_ino_t ino;

	if (qtype >= MAXQUOTAS)
		return -EINVAL;

	quota_get_qf_name(qtype, QFMT_VFS_V1, qf_name);

	ret = ext2fs_lookup(fs, EXT2_ROOT_INO, qf_name, strlen(qf_name), 0,
			    &ino);
	if (ret)
		return 0;

	return ino;
}

/*
 * Set the value for reserved quota inode number field in superblock.
 */
void quota_set_sb_inum(ext2_filsys fs, ext2_ino_t ino, enum quota_type qtype)
{
	ext2_ino_t *inump;

	inump = quota_sb_inump(fs->super, qtype);

	log_debug("setting quota ino in superblock: ino=%u, type=%d", ino,
		 qtype);
	*inump = ino;
	ext2fs_mark_super_dirty(fs);
}

errcode_t quota_remove_inode(ext2_filsys fs, enum quota_type qtype)
{
	ext2_ino_t qf_ino;
	errcode_t	retval;

	retval = ext2fs_read_bitmaps(fs);
	if (retval) {
		log_debug("Couldn't read bitmaps: %s", error_message(retval));
		return retval;
	}

	qf_ino = *quota_sb_inump(fs->super, qtype);
	if (qf_ino == 0)
		return 0;
	retval = quota_inode_truncate(fs, qf_ino);
	if (retval)
		return retval;
	if (qf_ino >= EXT2_FIRST_INODE(fs->super)) {
		struct ext2_inode inode;

		retval = ext2fs_read_inode(fs, qf_ino, &inode);
		if (!retval) {
			memset(&inode, 0, sizeof(struct ext2_inode));
			ext2fs_write_inode(fs, qf_ino, &inode);
		}
		ext2fs_inode_alloc_stats2(fs, qf_ino, -1, 0);
		ext2fs_mark_ib_dirty(fs);

	}
	quota_set_sb_inum(fs, 0, qtype);

	ext2fs_mark_super_dirty(fs);
	fs->flags &= ~EXT2_FLAG_SUPER_ONLY;
	retval = ext2fs_write_bitmaps(fs);
	if (retval) {
		log_debug("Couldn't write bitmaps: %s", error_message(retval));
		return retval;
	}
	return 0;
}

static void write_dquots(dict_t *dict, struct quota_handle *qh)
{
	dnode_t		*n;
	struct dquot	*dq;

	for (n = dict_first(dict); n; n = dict_next(dict, n)) {
		dq = dnode_get(n);
		if (dq) {
			print_dquot("write", dq);
			dq->dq_h = qh;
			update_grace_times(dq);
			qh->qh_ops->commit_dquot(dq);
		}
	}
}

errcode_t quota_write_inode(quota_ctx_t qctx, unsigned int qtype_bits)
{
	int		retval = 0;
	enum quota_type	qtype;
	dict_t		*dict;
	ext2_filsys	fs;
	struct quota_handle *h = NULL;
	int		fmt = QFMT_VFS_V1;

	if (!qctx)
		return 0;

	fs = qctx->fs;
	retval = ext2fs_get_mem(sizeof(struct quota_handle), &h);
	if (retval) {
		log_debug("Unable to allocate quota handle: %s",
			error_message(retval));
		goto out;
	}

	retval = ext2fs_read_bitmaps(fs);
	if (retval) {
		log_debug("Couldn't read bitmaps: %s", error_message(retval));
		goto out;
	}

	for (qtype = 0; qtype < MAXQUOTAS; qtype++) {
		if (((1 << qtype) & qtype_bits) == 0)
			continue;

		dict = qctx->quota_dict[qtype];
		if (!dict)
			continue;

		retval = quota_file_create(h, fs, qtype, fmt);
		if (retval) {
			log_debug("Cannot initialize io on quotafile: %s",
				  error_message(retval));
			goto out;
		}

		write_dquots(dict, h);
		retval = quota_file_close(qctx, h);
		if (retval) {
			log_debug("Cannot finish IO on new quotafile: %s",
				  strerror(errno));
			if (h->qh_qf.e2_file)
				ext2fs_file_close(h->qh_qf.e2_file);
			(void) quota_inode_truncate(fs, h->qh_qf.ino);
			goto out;
		}

		/* Set quota inode numbers in superblock. */
		quota_set_sb_inum(fs, h->qh_qf.ino, qtype);
		ext2fs_mark_super_dirty(fs);
		ext2fs_mark_bb_dirty(fs);
		fs->flags &= ~EXT2_FLAG_SUPER_ONLY;
	}

	retval = ext2fs_write_bitmaps(fs);
	if (retval) {
		log_debug("Couldn't write bitmaps: %s", error_message(retval));
		goto out;
	}
out:
	if (h)
		ext2fs_free_mem(&h);
	return retval;
}

/******************************************************************/
/* Helper functions for computing quota in memory.                */
/******************************************************************/

static int dict_uint_cmp(const void *a, const void *b)
{
	unsigned int	c, d;

	c = VOIDPTR_TO_UINT(a);
	d = VOIDPTR_TO_UINT(b);

	if (c == d)
		return 0;
	else if (c > d)
		return 1;
	else
		return -1;
}

static inline int project_quota_valid(quota_ctx_t qctx)
{
	return (EXT2_INODE_SIZE(qctx->fs->super) > EXT2_GOOD_OLD_INODE_SIZE);
}

static inline qid_t get_qid(struct ext2_inode_large *inode, enum quota_type qtype)
{
	unsigned int inode_size;

	switch (qtype) {
	case USRQUOTA:
		return inode_uid(*inode);
	case GRPQUOTA:
		return inode_gid(*inode);
	case PRJQUOTA:
		inode_size = EXT2_GOOD_OLD_INODE_SIZE +
			inode->i_extra_isize;
		if (inode_includes(inode_size, i_projid))
			return inode_projid(*inode);
		return 0;
	default:
		return 0;
	}

	return 0;
}

static void quota_dnode_free(dnode_t *node,
			     void *context EXT2FS_ATTR((unused)))
{
	void *ptr = node ? dnode_get(node) : 0;

	ext2fs_free_mem(&ptr);
	free(node);
}

/*
 * Set up the quota tracking data structures.
 */
errcode_t quota_init_context(quota_ctx_t *qctx, ext2_filsys fs,
			     unsigned int qtype_bits)
{
	errcode_t err;
	dict_t	*dict;
	quota_ctx_t ctx;
	enum quota_type	qtype;

	err = ext2fs_get_mem(sizeof(struct quota_ctx), &ctx);
	if (err) {
		log_debug("Failed to allocate quota context");
		return err;
	}

	memset(ctx, 0, sizeof(struct quota_ctx));
	for (qtype = 0; qtype < MAXQUOTAS; qtype++) {
		ctx->quota_file[qtype] = NULL;
		if (qtype_bits) {
			if (((1 << qtype) & qtype_bits) == 0)
				continue;
		} else {
			if (*quota_sb_inump(fs->super, qtype) == 0)
				continue;
		}
		err = ext2fs_get_mem(sizeof(dict_t), &dict);
		if (err) {
			log_debug("Failed to allocate dictionary");
			quota_release_context(&ctx);
			return err;
		}
		ctx->quota_dict[qtype] = dict;
		dict_init(dict, DICTCOUNT_T_MAX, dict_uint_cmp);
		dict_set_allocator(dict, NULL, quota_dnode_free, NULL);
	}

	ctx->fs = fs;
	*qctx = ctx;
	return 0;
}

void quota_release_context(quota_ctx_t *qctx)
{
	errcode_t err;
	dict_t	*dict;
	enum quota_type	qtype;
	quota_ctx_t ctx;

	if (!qctx)
		return;

	ctx = *qctx;
	for (qtype = 0; qtype < MAXQUOTAS; qtype++) {
		dict = ctx->quota_dict[qtype];
		ctx->quota_dict[qtype] = 0;
		if (dict) {
			dict_free_nodes(dict);
			free(dict);
		}
		if (ctx->quota_file[qtype]) {
			err = quota_file_close(ctx, ctx->quota_file[qtype]);
			if (err) {
				log_err("Cannot close quotafile: %s",
					strerror(errno));
				ext2fs_free_mem(&ctx->quota_file[qtype]);
			}
		}
	}
	*qctx = NULL;
	free(ctx);
}

static struct dquot *get_dq(dict_t *dict, __u32 key)
{
	struct dquot	*dq;
	dnode_t		*n;

	n = dict_lookup(dict, UINT_TO_VOIDPTR(key));
	if (n)
		dq = dnode_get(n);
	else {
		if (ext2fs_get_mem(sizeof(struct dquot), &dq)) {
			log_err("Unable to allocate dquot");
			return NULL;
		}
		memset(dq, 0, sizeof(struct dquot));
		dict_alloc_insert(dict, UINT_TO_VOIDPTR(key), dq);
		dq->dq_id = key;
	}
	return dq;
}


/*
 * Called to update the blocks used by a particular inode
 */
void quota_data_add(quota_ctx_t qctx, struct ext2_inode_large *inode,
		    ext2_ino_t ino EXT2FS_ATTR((unused)),
		    qsize_t space)
{
	struct dquot	*dq;
	dict_t		*dict;
	enum quota_type	qtype;

	if (!qctx)
		return;

	log_debug("ADD_DATA: Inode: %u, UID/GID: %u/%u, space: %ld", ino,
			inode_uid(*inode),
			inode_gid(*inode), space);
	for (qtype = 0; qtype < MAXQUOTAS; qtype++) {
		if (qtype == PRJQUOTA && !project_quota_valid(qctx))
			continue;
		dict = qctx->quota_dict[qtype];
		if (dict) {
			dq = get_dq(dict, get_qid(inode, qtype));
			if (dq)
				dq->dq_dqb.dqb_curspace += space;
		}
	}
}

/*
 * Called to remove some blocks used by a particular inode
 */
void quota_data_sub(quota_ctx_t qctx, struct ext2_inode_large *inode,
		    ext2_ino_t ino EXT2FS_ATTR((unused)),
		    qsize_t space)
{
	struct dquot	*dq;
	dict_t		*dict;
	enum quota_type	qtype;

	if (!qctx)
		return;

	log_debug("SUB_DATA: Inode: %u, UID/GID: %u/%u, space: %ld", ino,
			inode_uid(*inode),
			inode_gid(*inode), space);
	for (qtype = 0; qtype < MAXQUOTAS; qtype++) {
		if (qtype == PRJQUOTA && !project_quota_valid(qctx))
			continue;
		dict = qctx->quota_dict[qtype];
		if (dict) {
			dq = get_dq(dict, get_qid(inode, qtype));
			dq->dq_dqb.dqb_curspace -= space;
		}
	}
}

/*
 * Called to count the files used by an inode's user/group
 */
void quota_data_inodes(quota_ctx_t qctx, struct ext2_inode_large *inode,
		       ext2_ino_t ino EXT2FS_ATTR((unused)), int adjust)
{
	struct dquot	*dq;
	dict_t		*dict;
	enum quota_type	qtype;

	if (!qctx)
		return;

	log_debug("ADJ_INODE: Inode: %u, UID/GID: %u/%u, adjust: %d", ino,
			inode_uid(*inode),
			inode_gid(*inode), adjust);
	for (qtype = 0; qtype < MAXQUOTAS; qtype++) {
		if (qtype == PRJQUOTA && !project_quota_valid(qctx))
			continue;
		dict = qctx->quota_dict[qtype];
		if (dict) {
			dq = get_dq(dict, get_qid(inode, qtype));
			dq->dq_dqb.dqb_curinodes += adjust;
		}
	}
}

errcode_t quota_compute_usage(quota_ctx_t qctx)
{
	ext2_filsys fs;
	ext2_ino_t ino;
	errcode_t ret;
	struct ext2_inode_large *inode;
	int inode_size;
	qsize_t space;
	ext2_inode_scan scan;

	if (!qctx)
		return 0;

	fs = qctx->fs;
	ret = ext2fs_open_inode_scan(fs, 0, &scan);
	if (ret) {
		log_err("while opening inode scan. ret=%ld", ret);
		return ret;
	}
	inode_size = fs->super->s_inode_size;
	inode = malloc(inode_size);
	if (!inode) {
		ext2fs_close_inode_scan(scan);
		return ENOMEM;
	}
	while (1) {
		ret = ext2fs_get_next_inode_full(scan, &ino,
						 EXT2_INODE(inode), inode_size);
		if (ret) {
			log_err("while getting next inode. ret=%ld", ret);
			ext2fs_close_inode_scan(scan);
			free(inode);
			return ret;
		}
		if (ino == 0)
			break;
		if (inode->i_links_count &&
		    (ino == EXT2_ROOT_INO ||
		     ino >= EXT2_FIRST_INODE(fs->super))) {
			space = ext2fs_get_stat_i_blocks(fs,
						EXT2_INODE(inode)) << 9;
			quota_data_add(qctx, inode, ino, space);
			quota_data_inodes(qctx, inode, ino, +1);
		}
	}

	ext2fs_close_inode_scan(scan);
	free(inode);
	return 0;
}

struct scan_dquots_data {
	dict_t		*quota_dict;
	int             update_limits; /* update limits from disk */
	int		update_usage;
	int		check_consistency;
	int		usage_is_inconsistent;
};

static int scan_dquots_callback(struct dquot *dquot, void *cb_data)
{
	struct scan_dquots_data *scan_data = cb_data;
	dict_t *quota_dict = scan_data->quota_dict;
	struct dquot *dq;

	dq = get_dq(quota_dict, dquot->dq_id);
	dq->dq_id = dquot->dq_id;
	dq->dq_flags |= DQF_SEEN;

	print_dquot("mem", dq);
	print_dquot("dsk", dquot);

	/* Check if there is inconsistency */
	if (scan_data->check_consistency &&
	    (dq->dq_dqb.dqb_curspace != dquot->dq_dqb.dqb_curspace ||
	     dq->dq_dqb.dqb_curinodes != dquot->dq_dqb.dqb_curinodes)) {
		scan_data->usage_is_inconsistent = 1;
		fprintf(stderr, "[QUOTA WARNING] Usage inconsistent for ID %u:"
			"actual (%lld, %lld) != expected (%lld, %lld)\n",
			dq->dq_id, (long long) dq->dq_dqb.dqb_curspace,
			(long long) dq->dq_dqb.dqb_curinodes,
			(long long) dquot->dq_dqb.dqb_curspace,
			(long long) dquot->dq_dqb.dqb_curinodes);
	}

	if (scan_data->update_limits) {
		dq->dq_dqb.dqb_ihardlimit = dquot->dq_dqb.dqb_ihardlimit;
		dq->dq_dqb.dqb_isoftlimit = dquot->dq_dqb.dqb_isoftlimit;
		dq->dq_dqb.dqb_bhardlimit = dquot->dq_dqb.dqb_bhardlimit;
		dq->dq_dqb.dqb_bsoftlimit = dquot->dq_dqb.dqb_bsoftlimit;
	}

	if (scan_data->update_usage) {
		dq->dq_dqb.dqb_curspace = dquot->dq_dqb.dqb_curspace;
		dq->dq_dqb.dqb_curinodes = dquot->dq_dqb.dqb_curinodes;
	}

	return 0;
}

/*
 * Read all dquots from quota file into memory
 */
static errcode_t quota_read_all_dquots(struct quota_handle *qh,
                                       quota_ctx_t qctx,
				       int update_limits EXT2FS_ATTR((unused)))
{
	struct scan_dquots_data scan_data;

	scan_data.quota_dict = qctx->quota_dict[qh->qh_type];
	scan_data.check_consistency = 0;
	scan_data.update_limits = 0;
	scan_data.update_usage = 1;

	return qh->qh_ops->scan_dquots(qh, scan_dquots_callback, &scan_data);
}

/*
 * Write all memory dquots into quota file
 */
#if 0 /* currently unused, but may be useful in the future? */
static errcode_t quota_write_all_dquots(struct quota_handle *qh,
                                        quota_ctx_t qctx)
{
	errcode_t err;

	err = ext2fs_read_bitmaps(qctx->fs);
	if (err)
		return err;
	write_dquots(qctx->quota_dict[qh->qh_type], qh);
	ext2fs_mark_bb_dirty(qctx->fs);
	qctx->fs->flags &= ~EXT2_FLAG_SUPER_ONLY;
	ext2fs_write_bitmaps(qctx->fs);
	return 0;
}
#endif

/*
 * Updates the in-memory quota limits from the given quota inode.
 */
errcode_t quota_update_limits(quota_ctx_t qctx, ext2_ino_t qf_ino,
			      enum quota_type qtype)
{
	struct quota_handle *qh;
	errcode_t err;

	if (!qctx)
		return 0;

	err = ext2fs_get_mem(sizeof(struct quota_handle), &qh);
	if (err) {
		log_debug("Unable to allocate quota handle");
		return err;
	}

	err = quota_file_open(qctx, qh, qf_ino, qtype, -1, 0);
	if (err) {
		log_debug("Open quota file failed");
		goto out;
	}

	quota_read_all_dquots(qh, qctx, 1);

	err = quota_file_close(qctx, qh);
	if (err) {
		log_debug("Cannot finish IO on new quotafile: %s",
			strerror(errno));
		if (qh->qh_qf.e2_file)
			ext2fs_file_close(qh->qh_qf.e2_file);
	}
out:
	ext2fs_free_mem(&qh);
	return err;
}

/*
 * Compares the measured quota in qctx->quota_dict with that in the quota inode
 * on disk and updates the limits in qctx->quota_dict. 'usage_inconsistent' is
 * set to 1 if the supplied and on-disk quota usage values are not identical.
 */
errcode_t quota_compare_and_update(quota_ctx_t qctx, enum quota_type qtype,
				   int *usage_inconsistent)
{
	struct quota_handle qh;
	struct scan_dquots_data scan_data;
	struct dquot *dq;
	dnode_t *n;
	dict_t *dict = qctx->quota_dict[qtype];
	errcode_t err = 0;

	if (!dict)
		goto out;

	err = quota_file_open(qctx, &qh, 0, qtype, -1, 0);
	if (err) {
		log_debug("Open quota file failed");
		goto out;
	}

	scan_data.quota_dict = qctx->quota_dict[qtype];
	scan_data.update_limits = 1;
	scan_data.update_usage = 0;
	scan_data.check_consistency = 1;
	scan_data.usage_is_inconsistent = 0;
	err = qh.qh_ops->scan_dquots(&qh, scan_dquots_callback, &scan_data);
	if (err) {
		log_debug("Error scanning dquots");
		*usage_inconsistent = 1;
		goto out_close_qh;
	}

	for (n = dict_first(dict); n; n = dict_next(dict, n)) {
		dq = dnode_get(n);
		if (!dq)
			continue;
		if ((dq->dq_flags & DQF_SEEN) == 0) {
			fprintf(stderr, "[QUOTA WARNING] "
				"Missing quota entry ID %d\n", dq->dq_id);
			scan_data.usage_is_inconsistent = 1;
		}
	}
	*usage_inconsistent = scan_data.usage_is_inconsistent;

out_close_qh:
	err = quota_file_close(qctx, &qh);
	if (err) {
		log_debug("Cannot close quotafile: %s", error_message(errno));
		if (qh.qh_qf.e2_file)
			ext2fs_file_close(qh.qh_qf.e2_file);
	}
out:
	return err;
}

int parse_quota_opts(const char *opts, int (*func)(char *))
{
	char	*buf, *token, *next, *p;
	int	len;
	int	ret = 0;

	len = strlen(opts);
	buf = malloc(len + 1);
	if (!buf) {
		fprintf(stderr,
			"Couldn't allocate memory to parse quota options!\n");
		return -ENOMEM;
	}
	strcpy(buf, opts);
	for (token = buf; token && *token; token = next) {
		p = strchr(token, ',');
		next = 0;
		if (p) {
			*p = 0;
			next = p + 1;
		}
		ret = func(token);
		if (ret)
			break;
	}
	free(buf);
	return ret;
}
