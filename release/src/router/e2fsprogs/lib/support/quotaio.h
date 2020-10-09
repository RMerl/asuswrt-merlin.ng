/** quotaio.h
 *
 * Interface to the quota library.
 *
 * The quota library provides interface for creating and updating the quota
 * files and the ext4 superblock fields. It supports the new VFS_V1 quota
 * format. The quota library also provides support for keeping track of quotas
 * in memory.
 * The typical way to use the quota library is as follows:
 * {
 *	quota_ctx_t qctx;
 *
 *	quota_init_context(&qctx, fs, QUOTA_ALL_BIT);
 *	{
 *		quota_compute_usage(qctx);
 *		AND/OR
 *		quota_data_add/quota_data_sub/quota_data_inodes();
 *	}
 *	quota_write_inode(qctx, USRQUOTA);
 *	quota_write_inode(qctx, GRPQUOTA);
 *	quota_release_context(&qctx);
 * }
 *
 * This initial version does not support reading the quota files. This support
 * will be added in near future.
 *
 * Aditya Kali <adityakali@google.com>
 * Header of IO operations for quota utilities
 *
 * Jan Kara <jack@suse.cz>
 */

#ifndef GUARD_QUOTAIO_H
#define GUARD_QUOTAIO_H

#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"
#include "dqblk_v2.h"

typedef int64_t qsize_t;	/* Type in which we store size limitations */

enum quota_type {
	USRQUOTA = 0,
	GRPQUOTA = 1,
	PRJQUOTA = 2,
	MAXQUOTAS = 3,
};

#if MAXQUOTAS > 32
#error "cannot have more than 32 quota types to fit in qtype_bits"
#endif

#define QUOTA_USR_BIT (1 << USRQUOTA)
#define QUOTA_GRP_BIT (1 << GRPQUOTA)
#define QUOTA_PRJ_BIT (1 << PRJQUOTA)
#define QUOTA_ALL_BIT (QUOTA_USR_BIT | QUOTA_GRP_BIT | QUOTA_PRJ_BIT)

typedef struct quota_ctx *quota_ctx_t;
struct dict_t;

struct quota_ctx {
	ext2_filsys	fs;
	struct dict_t	*quota_dict[MAXQUOTAS];
	struct quota_handle *quota_file[MAXQUOTAS];
};

/*
 * Definitions of magics and versions of current quota files
 */
#define INITQMAGICS {\
	0xd9c01f11,	/* USRQUOTA */\
	0xd9c01927,	/* GRPQUOTA */\
	0xd9c03f14	/* PRJQUOTA */\
}

/* Size of blocks in which are counted size limits in generic utility parts */
#define QUOTABLOCK_BITS 10
#define QUOTABLOCK_SIZE (1 << QUOTABLOCK_BITS)
#define toqb(x) (((x) + QUOTABLOCK_SIZE - 1) >> QUOTABLOCK_BITS)

/* Quota format type IDs */
#define	QFMT_VFS_OLD 1
#define	QFMT_VFS_V0 2
#define	QFMT_VFS_V1 4

/*
 * The following constants define the default amount of time given a user
 * before the soft limits are treated as hard limits (usually resulting
 * in an allocation failure). The timer is started when the user crosses
 * their soft limit, it is reset when they go below their soft limit.
 */
#define MAX_IQ_TIME  604800	/* (7*24*60*60) 1 week */
#define MAX_DQ_TIME  604800	/* (7*24*60*60) 1 week */

#define IOFL_INFODIRTY	0x01	/* Did info change? */

struct quotafile_ops;

/* Generic information about quotafile */
struct util_dqinfo {
	time_t dqi_bgrace;	/* Block grace time for given quotafile */
	time_t dqi_igrace;	/* Inode grace time for given quotafile */
	union {
		struct v2_mem_dqinfo v2_mdqi;
	} u;			/* Format specific info about quotafile */
};

struct quota_file {
	ext2_filsys fs;
	ext2_ino_t ino;
	ext2_file_t e2_file;
};

/* Structure for one opened quota file */
struct quota_handle {
	enum quota_type qh_type;	/* Type of quotafile */
	int qh_fmt;		/* Quotafile format */
	int qh_file_flags;
	int qh_io_flags;	/* IO flags for file */
	struct quota_file qh_qf;
	unsigned int (*e2fs_read)(struct quota_file *qf, ext2_loff_t offset,
			 void *buf, unsigned int size);
	unsigned int (*e2fs_write)(struct quota_file *qf, ext2_loff_t offset,
			  void *buf, unsigned int size);
	struct quotafile_ops *qh_ops;	/* Operations on quotafile */
	struct util_dqinfo qh_info;	/* Generic quotafile info */
};

/* Utility quota block */
struct util_dqblk {
	qsize_t dqb_ihardlimit;
	qsize_t dqb_isoftlimit;
	qsize_t dqb_curinodes;
	qsize_t dqb_bhardlimit;
	qsize_t dqb_bsoftlimit;
	qsize_t dqb_curspace;
	time_t dqb_btime;
	time_t dqb_itime;
	union {
		struct v2_mem_dqblk v2_mdqb;
	} u;			/* Format specific dquot information */
};

/* Structure for one loaded quota */
struct dquot {
	struct dquot *dq_next;	/* Pointer to next dquot in the list */
	qid_t dq_id;		/* ID dquot belongs to */
	int dq_flags;		/* Some flags for utils */
	struct quota_handle *dq_h;	/* Handle of quotafile for this dquot */
	struct util_dqblk dq_dqb;	/* Parsed data of dquot */
};

#define DQF_SEEN	0x0001

/* Structure of quotafile operations */
struct quotafile_ops {
	/* Check whether quotafile is in our format */
	int (*check_file) (struct quota_handle *h, int type, int fmt);
	/* Open quotafile */
	int (*init_io) (struct quota_handle *h);
	/* Create new quotafile */
	int (*new_io) (struct quota_handle *h);
	/* Write all changes and close quotafile */
	int (*end_io) (struct quota_handle *h);
	/* Write info about quotafile */
	int (*write_info) (struct quota_handle *h);
	/* Read dquot into memory */
	struct dquot *(*read_dquot) (struct quota_handle *h, qid_t id);
	/* Write given dquot to disk */
	int (*commit_dquot) (struct dquot *dquot);
	/* Scan quotafile and call callback on every structure */
	int (*scan_dquots) (struct quota_handle *h,
			    int (*process_dquot) (struct dquot *dquot,
						  void *data),
			    void *data);
	/* Function to print format specific file information */
	int (*report) (struct quota_handle *h, int verbose);
};

/* This might go into a special header file but that sounds a bit silly... */
extern struct quotafile_ops quotafile_ops_meta;

/* Open existing quotafile of given type (and verify its format) on given
 * filesystem. */
errcode_t quota_file_open(quota_ctx_t qctx, struct quota_handle *h,
			  ext2_ino_t qf_ino, enum quota_type type,
			  int fmt, int flags);


/* Create new quotafile of specified format on given filesystem */
errcode_t quota_file_create(struct quota_handle *h, ext2_filsys fs,
			    enum quota_type qtype, int fmt);

/* Close quotafile */
errcode_t quota_file_close(quota_ctx_t qctx, struct quota_handle *h);

/* Get empty quota structure */
struct dquot *get_empty_dquot(void);

errcode_t quota_inode_truncate(ext2_filsys fs, ext2_ino_t ino);

const char *quota_type2name(enum quota_type qtype);
ext2_ino_t quota_type2inum(enum quota_type qtype, struct ext2_super_block *);

void update_grace_times(struct dquot *q);

/* size for the buffer returned by quota_get_qf_name(); must be greater
   than maxlen of extensions[] and fmtnames[] (plus 2) found in quotaio.c */
#define QUOTA_NAME_LEN 16

const char *quota_get_qf_name(enum quota_type, int fmt, char *buf);

/* In mkquota.c */
errcode_t quota_init_context(quota_ctx_t *qctx, ext2_filsys fs,
			     unsigned int qtype_bits);
void quota_data_inodes(quota_ctx_t qctx, struct ext2_inode_large *inode,
		       ext2_ino_t ino, int adjust);
void quota_data_add(quota_ctx_t qctx, struct ext2_inode_large *inode,
		    ext2_ino_t ino, qsize_t space);
void quota_data_sub(quota_ctx_t qctx, struct ext2_inode_large *inode,
		    ext2_ino_t ino, qsize_t space);
errcode_t quota_write_inode(quota_ctx_t qctx, enum quota_type qtype);
errcode_t quota_update_limits(quota_ctx_t qctx, ext2_ino_t qf_ino,
			      enum quota_type type);
errcode_t quota_compute_usage(quota_ctx_t qctx);
void quota_release_context(quota_ctx_t *qctx);
errcode_t quota_remove_inode(ext2_filsys fs, enum quota_type qtype);
int quota_file_exists(ext2_filsys fs, enum quota_type qtype);
void quota_set_sb_inum(ext2_filsys fs, ext2_ino_t ino, enum quota_type qtype);
errcode_t quota_compare_and_update(quota_ctx_t qctx, enum quota_type qtype,
				   int *usage_inconsistent);
int parse_quota_opts(const char *opts, int (*func)(char *));

/* parse_qtype.c */
int parse_quota_types(const char *in_str, unsigned int *qtype_bits,
		      char **err_token);

/*
 * Return pointer to reserved inode field in superblock for given quota type.
 *
 * This allows the caller to get or set the quota inode by type without the
 * need for the quota array to be contiguous in the superblock.
 */
static inline ext2_ino_t *quota_sb_inump(struct ext2_super_block *sb,
					 enum quota_type qtype)
{
	switch (qtype) {
	case USRQUOTA:
		return &sb->s_usr_quota_inum;
	case GRPQUOTA:
		return &sb->s_grp_quota_inum;
	case PRJQUOTA:
		return &sb->s_prj_quota_inum;
	default:
		return NULL;
	}

	return NULL;
}

#endif /* GUARD_QUOTAIO_H */
