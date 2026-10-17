// SPDX-License-Identifier: GPL-2.0+
#include "internal.h"

#define makedev(major, minor) ((dev_t)((((major) & 0xfff) << 8) | ((minor) & 0xff)))
static dev_t erofs_new_decode_dev(u32 dev)
{
	const unsigned int major = (dev & 0xfff00) >> 8;
	const unsigned int minor = (dev & 0xff) | ((dev >> 12) & 0xfff00);

	return makedev(major, minor);
}

int erofs_read_inode_from_disk(struct erofs_inode *vi)
{
	int ret, ifmt;
	char buf[sizeof(struct erofs_inode_extended)];
	struct erofs_inode_compact *dic;
	struct erofs_inode_extended *die;
	const erofs_off_t inode_loc = iloc(vi->nid);

	ret = erofs_dev_read(0, buf, inode_loc, sizeof(*dic));
	if (ret < 0)
		return -EIO;

	dic = (struct erofs_inode_compact *)buf;
	ifmt = le16_to_cpu(dic->i_format);

	vi->datalayout = erofs_inode_datalayout(ifmt);
	if (vi->datalayout >= EROFS_INODE_DATALAYOUT_MAX) {
		erofs_err("unsupported datalayout %u of nid %llu",
			  vi->datalayout, vi->nid | 0ULL);
		return -EOPNOTSUPP;
	}
	switch (erofs_inode_version(ifmt)) {
	case EROFS_INODE_LAYOUT_EXTENDED:
		vi->inode_isize = sizeof(struct erofs_inode_extended);

		ret = erofs_dev_read(0, buf + sizeof(*dic),
				     inode_loc + sizeof(*dic),
				     sizeof(*die) - sizeof(*dic));
		if (ret < 0)
			return -EIO;

		die = (struct erofs_inode_extended *)buf;
		vi->xattr_isize = erofs_xattr_ibody_size(die->i_xattr_icount);
		vi->i_mode = le16_to_cpu(die->i_mode);

		switch (vi->i_mode & S_IFMT) {
		case S_IFREG:
		case S_IFDIR:
		case S_IFLNK:
			vi->u.i_blkaddr = le32_to_cpu(die->i_u.raw_blkaddr);
			break;
		case S_IFCHR:
		case S_IFBLK:
			vi->u.i_rdev =
				erofs_new_decode_dev(le32_to_cpu(die->i_u.rdev));
			break;
		case S_IFIFO:
		case S_IFSOCK:
			vi->u.i_rdev = 0;
			break;
		default:
			goto bogusimode;
		}

		vi->i_uid = le32_to_cpu(die->i_uid);
		vi->i_gid = le32_to_cpu(die->i_gid);
		vi->i_nlink = le32_to_cpu(die->i_nlink);

		vi->i_mtime = le64_to_cpu(die->i_mtime);
		vi->i_mtime_nsec = le64_to_cpu(die->i_mtime_nsec);
		vi->i_size = le64_to_cpu(die->i_size);
		if (vi->datalayout == EROFS_INODE_CHUNK_BASED)
			/* fill chunked inode summary info */
			vi->u.chunkformat = le16_to_cpu(die->i_u.c.format);
		break;
	case EROFS_INODE_LAYOUT_COMPACT:
		vi->inode_isize = sizeof(struct erofs_inode_compact);
		vi->xattr_isize = erofs_xattr_ibody_size(dic->i_xattr_icount);
		vi->i_mode = le16_to_cpu(dic->i_mode);

		switch (vi->i_mode & S_IFMT) {
		case S_IFREG:
		case S_IFDIR:
		case S_IFLNK:
			vi->u.i_blkaddr = le32_to_cpu(dic->i_u.raw_blkaddr);
			break;
		case S_IFCHR:
		case S_IFBLK:
			vi->u.i_rdev =
				erofs_new_decode_dev(le32_to_cpu(dic->i_u.rdev));
			break;
		case S_IFIFO:
		case S_IFSOCK:
			vi->u.i_rdev = 0;
			break;
		default:
			goto bogusimode;
		}

		vi->i_uid = le16_to_cpu(dic->i_uid);
		vi->i_gid = le16_to_cpu(dic->i_gid);
		vi->i_nlink = le16_to_cpu(dic->i_nlink);

		vi->i_mtime = sbi.build_time;
		vi->i_mtime_nsec = sbi.build_time_nsec;

		vi->i_size = le32_to_cpu(dic->i_size);
		if (vi->datalayout == EROFS_INODE_CHUNK_BASED)
			vi->u.chunkformat = le16_to_cpu(dic->i_u.c.format);
		break;
	default:
		erofs_err("unsupported on-disk inode version %u of nid %llu",
			  erofs_inode_version(ifmt), vi->nid | 0ULL);
		return -EOPNOTSUPP;
	}

	vi->flags = 0;
	if (vi->datalayout == EROFS_INODE_CHUNK_BASED) {
		if (vi->u.chunkformat & ~EROFS_CHUNK_FORMAT_ALL) {
			erofs_err("unsupported chunk format %x of nid %llu",
				  vi->u.chunkformat, vi->nid | 0ULL);
			return -EOPNOTSUPP;
		}
		vi->u.chunkbits = sbi.blkszbits +
			(vi->u.chunkformat & EROFS_CHUNK_FORMAT_BLKBITS_MASK);
	} else if (erofs_inode_is_data_compressed(vi->datalayout)) {
		if (erofs_blksiz() != EROFS_MAX_BLOCK_SIZE)
			return -EOPNOTSUPP;
		return z_erofs_fill_inode(vi);
	}
	return 0;
bogusimode:
	erofs_err("bogus i_mode (%o) @ nid %llu", vi->i_mode, vi->nid | 0ULL);
	return -EFSCORRUPTED;
}

struct erofs_dirent *find_target_dirent(erofs_nid_t pnid,
					void *dentry_blk,
					const char *name, unsigned int len,
					unsigned int nameoff,
					unsigned int maxsize)
{
	struct erofs_dirent *de = dentry_blk;
	const struct erofs_dirent *end = dentry_blk + nameoff;

	while (de < end) {
		const char *de_name;
		unsigned int de_namelen;

		nameoff = le16_to_cpu(de->nameoff);
		de_name = (char *)dentry_blk + nameoff;

		/* the last dirent in the block? */
		if (de + 1 >= end)
			de_namelen = strnlen(de_name, maxsize - nameoff);
		else
			de_namelen = le16_to_cpu(de[1].nameoff) - nameoff;

		/* a corrupted entry is found */
		if (nameoff + de_namelen > maxsize ||
		    de_namelen > EROFS_NAME_LEN) {
			erofs_err("bogus dirent @ nid %llu", pnid | 0ULL);
			DBG_BUGON(1);
			return ERR_PTR(-EFSCORRUPTED);
		}

		if (len == de_namelen && !memcmp(de_name, name, de_namelen))
			return de;
		++de;
	}
	return NULL;
}

struct nameidata {
	erofs_nid_t	nid;
	unsigned int	ftype;
};

int erofs_namei(struct nameidata *nd, const char *name, unsigned int len)
{
	erofs_nid_t nid = nd->nid;
	int ret;
	char buf[EROFS_MAX_BLOCK_SIZE];
	struct erofs_inode vi = { .nid = nid };
	erofs_off_t offset;

	ret = erofs_read_inode_from_disk(&vi);
	if (ret)
		return ret;

	offset = 0;
	while (offset < vi.i_size) {
		erofs_off_t maxsize = min_t(erofs_off_t,
					    vi.i_size - offset, erofs_blksiz());
		struct erofs_dirent *de = (void *)buf;
		unsigned int nameoff;

		ret = erofs_pread(&vi, buf, maxsize, offset);
		if (ret)
			return ret;

		nameoff = le16_to_cpu(de->nameoff);
		if (nameoff < sizeof(struct erofs_dirent) ||
		    nameoff >= erofs_blksiz()) {
			erofs_err("invalid de[0].nameoff %u @ nid %llu",
				  nameoff, nid | 0ULL);
			return -EFSCORRUPTED;
		}

		de = find_target_dirent(nid, buf, name, len,
					nameoff, maxsize);
		if (IS_ERR(de))
			return PTR_ERR(de);

		if (de) {
			nd->nid = le64_to_cpu(de->nid);
			return 0;
		}
		offset += maxsize;
	}
	return -ENOENT;
}

static int link_path_walk(const char *name, struct nameidata *nd)
{
	nd->nid = sbi.root_nid;

	while (*name == '/')
		name++;

	/* At this point we know we have a real path component. */
	while (*name != '\0') {
		const char *p = name;
		int ret;

		do {
			++p;
		} while (*p != '\0' && *p != '/');

		DBG_BUGON(p <= name);
		ret = erofs_namei(nd, name, p - name);
		if (ret)
			return ret;

		name = p;
		/* Skip until no more slashes. */
		for (name = p; *name == '/'; ++name)
			;
	}
	return 0;
}

int erofs_ilookup(const char *path, struct erofs_inode *vi)
{
	int ret;
	struct nameidata nd;

	ret = link_path_walk(path, &nd);
	if (ret)
		return ret;

	vi->nid = nd.nid;
	return erofs_read_inode_from_disk(vi);
}
