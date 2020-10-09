/** quotaio.c
 *
 * Generic IO operations on quotafiles
 * Jan Kara <jack@suse.cz> - sponsored by SuSE CR
 * Aditya Kali <adityakali@google.com> - Ported to e2fsprogs
 */

#include "config.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <assert.h>

#include "common.h"
#include "quotaio.h"

static const char * const extensions[MAXQUOTAS] = {
	[USRQUOTA] = "user",
	[GRPQUOTA] = "group",
	[PRJQUOTA] = "project",
};
static const char * const basenames[] = {
	"",		/* undefined */
	"quota",	/* QFMT_VFS_OLD */
	"aquota",	/* QFMT_VFS_V0 */
	"",		/* QFMT_OCFS2 */
	"aquota"	/* QFMT_VFS_V1 */
};

/* Header in all newer quotafiles */
struct disk_dqheader {
	__le32 dqh_magic;
	__le32 dqh_version;
} __attribute__ ((packed));

/**
 * Convert type of quota to written representation
 */
const char *quota_type2name(enum quota_type qtype)
{
	if (qtype >= MAXQUOTAS)
		return "unknown";
	return extensions[qtype];
}

ext2_ino_t quota_type2inum(enum quota_type qtype,
                           struct ext2_super_block *sb)
{
	switch (qtype) {
	case USRQUOTA:
		return EXT4_USR_QUOTA_INO;
	case GRPQUOTA:
		return EXT4_GRP_QUOTA_INO;
	case PRJQUOTA:
		return sb->s_prj_quota_inum;
	default:
		return 0;
	}
	return 0;
}

/**
 * Creates a quota file name for given type and format.
 */
const char *quota_get_qf_name(enum quota_type type, int fmt, char *buf)
{
	if (!buf)
		return NULL;
	snprintf(buf, QUOTA_NAME_LEN, "%s.%s",
		 basenames[fmt], extensions[type]);

	return buf;
}

/*
 * Set grace time if needed
 */
void update_grace_times(struct dquot *q)
{
	time_t now;

	time(&now);
	if (q->dq_dqb.dqb_bsoftlimit && toqb(q->dq_dqb.dqb_curspace) >
			q->dq_dqb.dqb_bsoftlimit) {
		if (!q->dq_dqb.dqb_btime)
			q->dq_dqb.dqb_btime =
				now + q->dq_h->qh_info.dqi_bgrace;
	} else {
		q->dq_dqb.dqb_btime = 0;
	}

	if (q->dq_dqb.dqb_isoftlimit && q->dq_dqb.dqb_curinodes >
			q->dq_dqb.dqb_isoftlimit) {
		if (!q->dq_dqb.dqb_itime)
				q->dq_dqb.dqb_itime =
					now + q->dq_h->qh_info.dqi_igrace;
	} else {
		q->dq_dqb.dqb_itime = 0;
	}
}

errcode_t quota_inode_truncate(ext2_filsys fs, ext2_ino_t ino)
{
	struct ext2_inode inode;
	errcode_t err;
	enum quota_type qtype;

	if ((err = ext2fs_read_inode(fs, ino, &inode)))
		return err;

	for (qtype = 0; qtype < MAXQUOTAS; qtype++)
		if (ino == quota_type2inum(qtype, fs->super))
			break;

	if (qtype != MAXQUOTAS) {
		inode.i_dtime = fs->now ? fs->now : time(0);
		if (!ext2fs_inode_has_valid_blocks2(fs, &inode))
			return 0;
		err = ext2fs_punch(fs, ino, &inode, NULL, 0, ~0ULL);
		if (err)
			return err;
		fs->flags &= ~EXT2_FLAG_SUPER_ONLY;
		memset(&inode, 0, sizeof(struct ext2_inode));
	} else {
		inode.i_flags &= ~EXT2_IMMUTABLE_FL;
	}
	err = ext2fs_write_inode(fs, ino, &inode);
	return err;
}

/* Functions to read/write quota file. */
static unsigned int quota_write_nomount(struct quota_file *qf,
					ext2_loff_t offset,
					void *buf, unsigned int size)
{
	ext2_file_t	e2_file = qf->e2_file;
	unsigned int	bytes_written = 0;
	errcode_t	err;

	err = ext2fs_file_llseek(e2_file, offset, EXT2_SEEK_SET, NULL);
	if (err) {
		log_err("ext2fs_file_llseek failed: %ld", err);
		return 0;
	}

	err = ext2fs_file_write(e2_file, buf, size, &bytes_written);
	if (err) {
		log_err("ext2fs_file_write failed: %ld", err);
		return 0;
	}

	/* Correct inode.i_size is set in end_io. */
	return bytes_written;
}

static unsigned int quota_read_nomount(struct quota_file *qf,
				       ext2_loff_t offset,
				       void *buf, unsigned int size)
{
	ext2_file_t	e2_file = qf->e2_file;
	unsigned int	bytes_read = 0;
	errcode_t	err;

	err = ext2fs_file_llseek(e2_file, offset, EXT2_SEEK_SET, NULL);
	if (err) {
		log_err("ext2fs_file_llseek failed: %ld", err);
		return 0;
	}

	err = ext2fs_file_read(e2_file, buf, size, &bytes_read);
	if (err) {
		log_err("ext2fs_file_read failed: %ld", err);
		return 0;
	}

	return bytes_read;
}

/*
 * Detect quota format and initialize quota IO
 */
errcode_t quota_file_open(quota_ctx_t qctx, struct quota_handle *h,
			  ext2_ino_t qf_ino, enum quota_type qtype,
			  int fmt, int flags)
{
	ext2_filsys fs = qctx->fs;
	ext2_file_t e2_file;
	errcode_t err;
	int allocated_handle = 0;

	if (qtype >= MAXQUOTAS)
		return EINVAL;

	if (fmt == -1)
		fmt = QFMT_VFS_V1;

	err = ext2fs_read_bitmaps(fs);
	if (err)
		return err;

	if (qf_ino == 0)
		qf_ino = *quota_sb_inump(fs->super, qtype);

	log_debug("Opening quota ino=%u, type=%d", qf_ino, qtype);
	err = ext2fs_file_open(fs, qf_ino, flags, &e2_file);
	if (err) {
		log_err("ext2fs_file_open failed: %s", error_message(err));
		return err;
	}

	if (!h) {
		if (qctx->quota_file[qtype]) {
			h = qctx->quota_file[qtype];
			if (((flags & EXT2_FILE_WRITE) == 0) ||
			    (h->qh_file_flags & EXT2_FILE_WRITE)) {
				ext2fs_file_close(e2_file);
				return 0;
			}
			(void) quota_file_close(qctx, h);
		}
		err = ext2fs_get_mem(sizeof(struct quota_handle), &h);
		if (err) {
			log_err("Unable to allocate quota handle");
			ext2fs_file_close(e2_file);
			return err;
		}
		allocated_handle = 1;
	}

	h->qh_qf.e2_file = e2_file;
	h->qh_qf.fs = fs;
	h->qh_qf.ino = qf_ino;
	h->e2fs_write = quota_write_nomount;
	h->e2fs_read = quota_read_nomount;
	h->qh_file_flags = flags;
	h->qh_io_flags = 0;
	h->qh_type = qtype;
	h->qh_fmt = fmt;
	memset(&h->qh_info, 0, sizeof(h->qh_info));
	h->qh_ops = &quotafile_ops_2;

	if (h->qh_ops->check_file &&
	    (h->qh_ops->check_file(h, qtype, fmt) == 0)) {
		log_err("qh_ops->check_file failed");
		err = EIO;
		goto errout;
	}

	if (h->qh_ops->init_io && (h->qh_ops->init_io(h) < 0)) {
		log_err("qh_ops->init_io failed");
		err = EIO;
		goto errout;
	}
	if (allocated_handle)
		qctx->quota_file[qtype] = h;

	return 0;
errout:
	ext2fs_file_close(e2_file);
	if (allocated_handle)
		ext2fs_free_mem(&h);
	return err;
}

static errcode_t quota_inode_init_new(ext2_filsys fs, ext2_ino_t ino)
{
	struct ext2_inode inode;
	errcode_t err = 0;

	err = ext2fs_read_inode(fs, ino, &inode);
	if (err) {
		log_err("ex2fs_read_inode failed");
		return err;
	}

	if (EXT2_I_SIZE(&inode)) {
		err = quota_inode_truncate(fs, ino);
		if (err)
			return err;
	}

	memset(&inode, 0, sizeof(struct ext2_inode));
	ext2fs_iblk_set(fs, &inode, 0);
	inode.i_atime = inode.i_mtime =
		inode.i_ctime = fs->now ? fs->now : time(0);
	inode.i_links_count = 1;
	inode.i_mode = LINUX_S_IFREG | 0600;
	inode.i_flags |= EXT2_IMMUTABLE_FL;
	if (ext2fs_has_feature_extents(fs->super))
		inode.i_flags |= EXT4_EXTENTS_FL;

	err = ext2fs_write_new_inode(fs, ino, &inode);
	if (err) {
		log_err("ext2fs_write_new_inode failed: %ld", err);
		return err;
	}
	return err;
}

/*
 * Create new quotafile of specified format on given filesystem
 */
errcode_t quota_file_create(struct quota_handle *h, ext2_filsys fs,
			    enum quota_type qtype, int fmt)
{
	ext2_file_t e2_file;
	errcode_t err;
	ext2_ino_t qf_inum = 0;

	if (fmt == -1)
		fmt = QFMT_VFS_V1;

	h->qh_qf.fs = fs;
	qf_inum = quota_type2inum(qtype, fs->super);
	if (qf_inum == 0 && qtype == PRJQUOTA) {
		err = ext2fs_new_inode(fs, EXT2_ROOT_INO, LINUX_S_IFREG | 0600,
				       0, &qf_inum);
		if (err)
			return err;
		ext2fs_inode_alloc_stats2(fs, qf_inum, +1, 0);
		ext2fs_mark_ib_dirty(fs);
	} else if (qf_inum == 0) {
		return EXT2_ET_BAD_INODE_NUM;
	}

	err = ext2fs_read_bitmaps(fs);
	if (err)
		goto out_err;

	err = quota_inode_init_new(fs, qf_inum);
	if (err) {
		log_err("init_new_quota_inode failed");
		goto out_err;
	}
	h->qh_qf.ino = qf_inum;
	h->qh_file_flags = EXT2_FILE_WRITE | EXT2_FILE_CREATE;
	h->e2fs_write = quota_write_nomount;
	h->e2fs_read = quota_read_nomount;

	log_debug("Creating quota ino=%u, type=%d", qf_inum, qtype);
	err = ext2fs_file_open(fs, qf_inum, h->qh_file_flags, &e2_file);
	if (err) {
		log_err("ext2fs_file_open failed: %ld", err);
		goto out_err;
	}
	h->qh_qf.e2_file = e2_file;

	h->qh_io_flags = 0;
	h->qh_type = qtype;
	h->qh_fmt = fmt;
	memset(&h->qh_info, 0, sizeof(h->qh_info));
	h->qh_ops = &quotafile_ops_2;

	if (h->qh_ops->new_io && (h->qh_ops->new_io(h) < 0)) {
		log_err("qh_ops->new_io failed");
		err = EIO;
		goto out_err1;
	}

	return 0;

out_err1:
	ext2fs_file_close(e2_file);
out_err:

	if (qf_inum)
		quota_inode_truncate(fs, qf_inum);

	return err;
}

/*
 * Close quotafile and release handle
 */
errcode_t quota_file_close(quota_ctx_t qctx, struct quota_handle *h)
{
	if (h->qh_io_flags & IOFL_INFODIRTY) {
		if (h->qh_ops->write_info && h->qh_ops->write_info(h) < 0)
			return EIO;
		h->qh_io_flags &= ~IOFL_INFODIRTY;
	}

	if (h->qh_ops->end_io && h->qh_ops->end_io(h) < 0)
		return EIO;
	if (h->qh_qf.e2_file)
		ext2fs_file_close(h->qh_qf.e2_file);
	if (qctx->quota_file[h->qh_type] == h)
		ext2fs_free_mem(&qctx->quota_file[h->qh_type]);
	return 0;
}

/*
 * Create empty quota structure
 */
struct dquot *get_empty_dquot(void)
{
	struct dquot *dquot;

	if (ext2fs_get_memzero(sizeof(struct dquot), &dquot)) {
		log_err("Failed to allocate dquot");
		return NULL;
	}

	dquot->dq_id = -1;
	return dquot;
}
