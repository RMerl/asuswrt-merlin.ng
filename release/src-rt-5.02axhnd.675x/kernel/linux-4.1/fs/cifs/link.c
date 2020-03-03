/*
 *   fs/cifs/link.c
 *
 *   Copyright (C) International Business Machines  Corp., 2002,2008
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
#include <linux/fs.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <linux/namei.h>
#include "cifsfs.h"
#include "cifspdu.h"
#include "cifsglob.h"
#include "cifsproto.h"
#include "cifs_debug.h"
#include "cifs_fs_sb.h"
#include "cifs_unicode.h"
#ifdef CONFIG_CIFS_SMB2
#include "smb2proto.h"
#endif

/*
 * M-F Symlink Functions - Begin
 */

#define CIFS_MF_SYMLINK_LEN_OFFSET (4+1)
#define CIFS_MF_SYMLINK_MD5_OFFSET (CIFS_MF_SYMLINK_LEN_OFFSET+(4+1))
#define CIFS_MF_SYMLINK_LINK_OFFSET (CIFS_MF_SYMLINK_MD5_OFFSET+(32+1))
#define CIFS_MF_SYMLINK_LINK_MAXLEN (1024)
#define CIFS_MF_SYMLINK_FILE_SIZE \
	(CIFS_MF_SYMLINK_LINK_OFFSET + CIFS_MF_SYMLINK_LINK_MAXLEN)

#define CIFS_MF_SYMLINK_LEN_FORMAT "XSym\n%04u\n"
#define CIFS_MF_SYMLINK_MD5_FORMAT \
	"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n"
#define CIFS_MF_SYMLINK_MD5_ARGS(md5_hash) \
	md5_hash[0],  md5_hash[1],  md5_hash[2],  md5_hash[3], \
	md5_hash[4],  md5_hash[5],  md5_hash[6],  md5_hash[7], \
	md5_hash[8],  md5_hash[9],  md5_hash[10], md5_hash[11],\
	md5_hash[12], md5_hash[13], md5_hash[14], md5_hash[15]

static int
symlink_hash(unsigned int link_len, const char *link_str, u8 *md5_hash)
{
	int rc;
	unsigned int size;
	struct crypto_shash *md5;
	struct sdesc *sdescmd5;

	md5 = crypto_alloc_shash("md5", 0, 0);
	if (IS_ERR(md5)) {
		rc = PTR_ERR(md5);
		cifs_dbg(VFS, "%s: Crypto md5 allocation error %d\n",
			 __func__, rc);
		return rc;
	}
	size = sizeof(struct shash_desc) + crypto_shash_descsize(md5);
	sdescmd5 = kmalloc(size, GFP_KERNEL);
	if (!sdescmd5) {
		rc = -ENOMEM;
		goto symlink_hash_err;
	}
	sdescmd5->shash.tfm = md5;
	sdescmd5->shash.flags = 0x0;

	rc = crypto_shash_init(&sdescmd5->shash);
	if (rc) {
		cifs_dbg(VFS, "%s: Could not init md5 shash\n", __func__);
		goto symlink_hash_err;
	}
	rc = crypto_shash_update(&sdescmd5->shash, link_str, link_len);
	if (rc) {
		cifs_dbg(VFS, "%s: Could not update with link_str\n", __func__);
		goto symlink_hash_err;
	}
	rc = crypto_shash_final(&sdescmd5->shash, md5_hash);
	if (rc)
		cifs_dbg(VFS, "%s: Could not generate md5 hash\n", __func__);

symlink_hash_err:
	crypto_free_shash(md5);
	kfree(sdescmd5);

	return rc;
}

static int
parse_mf_symlink(const u8 *buf, unsigned int buf_len, unsigned int *_link_len,
		 char **_link_str)
{
	int rc;
	unsigned int link_len;
	const char *md5_str1;
	const char *link_str;
	u8 md5_hash[16];
	char md5_str2[34];

	if (buf_len != CIFS_MF_SYMLINK_FILE_SIZE)
		return -EINVAL;

	md5_str1 = (const char *)&buf[CIFS_MF_SYMLINK_MD5_OFFSET];
	link_str = (const char *)&buf[CIFS_MF_SYMLINK_LINK_OFFSET];

	rc = sscanf(buf, CIFS_MF_SYMLINK_LEN_FORMAT, &link_len);
	if (rc != 1)
		return -EINVAL;

	rc = symlink_hash(link_len, link_str, md5_hash);
	if (rc) {
		cifs_dbg(FYI, "%s: MD5 hash failure: %d\n", __func__, rc);
		return rc;
	}

	snprintf(md5_str2, sizeof(md5_str2),
		 CIFS_MF_SYMLINK_MD5_FORMAT,
		 CIFS_MF_SYMLINK_MD5_ARGS(md5_hash));

	if (strncmp(md5_str1, md5_str2, 17) != 0)
		return -EINVAL;

	if (_link_str) {
		*_link_str = kstrndup(link_str, link_len, GFP_KERNEL);
		if (!*_link_str)
			return -ENOMEM;
	}

	*_link_len = link_len;
	return 0;
}

static int
format_mf_symlink(u8 *buf, unsigned int buf_len, const char *link_str)
{
	int rc;
	unsigned int link_len;
	unsigned int ofs;
	u8 md5_hash[16];

	if (buf_len != CIFS_MF_SYMLINK_FILE_SIZE)
		return -EINVAL;

	link_len = strlen(link_str);

	if (link_len > CIFS_MF_SYMLINK_LINK_MAXLEN)
		return -ENAMETOOLONG;

	rc = symlink_hash(link_len, link_str, md5_hash);
	if (rc) {
		cifs_dbg(FYI, "%s: MD5 hash failure: %d\n", __func__, rc);
		return rc;
	}

	snprintf(buf, buf_len,
		 CIFS_MF_SYMLINK_LEN_FORMAT CIFS_MF_SYMLINK_MD5_FORMAT,
		 link_len,
		 CIFS_MF_SYMLINK_MD5_ARGS(md5_hash));

	ofs = CIFS_MF_SYMLINK_LINK_OFFSET;
	memcpy(buf + ofs, link_str, link_len);

	ofs += link_len;
	if (ofs < CIFS_MF_SYMLINK_FILE_SIZE) {
		buf[ofs] = '\n';
		ofs++;
	}

	while (ofs < CIFS_MF_SYMLINK_FILE_SIZE) {
		buf[ofs] = ' ';
		ofs++;
	}

	return 0;
}

bool
couldbe_mf_symlink(const struct cifs_fattr *fattr)
{
	if (!S_ISREG(fattr->cf_mode))
		/* it's not a symlink */
		return false;

	if (fattr->cf_eof != CIFS_MF_SYMLINK_FILE_SIZE)
		/* it's not a symlink */
		return false;

	return true;
}

static int
create_mf_symlink(const unsigned int xid, struct cifs_tcon *tcon,
		  struct cifs_sb_info *cifs_sb, const char *fromName,
		  const char *toName)
{
	int rc;
	u8 *buf;
	unsigned int bytes_written = 0;

	buf = kmalloc(CIFS_MF_SYMLINK_FILE_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	rc = format_mf_symlink(buf, CIFS_MF_SYMLINK_FILE_SIZE, toName);
	if (rc)
		goto out;

	if (tcon->ses->server->ops->create_mf_symlink)
		rc = tcon->ses->server->ops->create_mf_symlink(xid, tcon,
					cifs_sb, fromName, buf, &bytes_written);
	else
		rc = -EOPNOTSUPP;

	if (rc)
		goto out;

	if (bytes_written != CIFS_MF_SYMLINK_FILE_SIZE)
		rc = -EIO;
out:
	kfree(buf);
	return rc;
}

static int
query_mf_symlink(const unsigned int xid, struct cifs_tcon *tcon,
		 struct cifs_sb_info *cifs_sb, const unsigned char *path,
		 char **symlinkinfo)
{
	int rc;
	u8 *buf = NULL;
	unsigned int link_len = 0;
	unsigned int bytes_read = 0;

	buf = kmalloc(CIFS_MF_SYMLINK_FILE_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (tcon->ses->server->ops->query_mf_symlink)
		rc = tcon->ses->server->ops->query_mf_symlink(xid, tcon,
					      cifs_sb, path, buf, &bytes_read);
	else
		rc = -ENOSYS;

	if (rc)
		goto out;

	if (bytes_read == 0) { /* not a symlink */
		rc = -EINVAL;
		goto out;
	}

	rc = parse_mf_symlink(buf, bytes_read, &link_len, symlinkinfo);
out:
	kfree(buf);
	return rc;
}

int
check_mf_symlink(unsigned int xid, struct cifs_tcon *tcon,
		 struct cifs_sb_info *cifs_sb, struct cifs_fattr *fattr,
		 const unsigned char *path)
{
	int rc;
	u8 *buf = NULL;
	unsigned int link_len = 0;
	unsigned int bytes_read = 0;

	if (!couldbe_mf_symlink(fattr))
		/* it's not a symlink */
		return 0;

	buf = kmalloc(CIFS_MF_SYMLINK_FILE_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (tcon->ses->server->ops->query_mf_symlink)
		rc = tcon->ses->server->ops->query_mf_symlink(xid, tcon,
					      cifs_sb, path, buf, &bytes_read);
	else
		rc = -ENOSYS;

	if (rc)
		goto out;

	if (bytes_read == 0) /* not a symlink */
		goto out;

	rc = parse_mf_symlink(buf, bytes_read, &link_len, NULL);
	if (rc == -EINVAL) {
		/* it's not a symlink */
		rc = 0;
		goto out;
	}

	if (rc != 0)
		goto out;

	/* it is a symlink */
	fattr->cf_eof = link_len;
	fattr->cf_mode &= ~S_IFMT;
	fattr->cf_mode |= S_IFLNK | S_IRWXU | S_IRWXG | S_IRWXO;
	fattr->cf_dtype = DT_LNK;
out:
	kfree(buf);
	return rc;
}

/*
 * SMB 1.0 Protocol specific functions
 */

int
cifs_query_mf_symlink(unsigned int xid, struct cifs_tcon *tcon,
		      struct cifs_sb_info *cifs_sb, const unsigned char *path,
		      char *pbuf, unsigned int *pbytes_read)
{
	int rc;
	int oplock = 0;
	struct cifs_fid fid;
	struct cifs_open_parms oparms;
	struct cifs_io_parms io_parms;
	int buf_type = CIFS_NO_BUFFER;
	FILE_ALL_INFO file_info;

	oparms.tcon = tcon;
	oparms.cifs_sb = cifs_sb;
	oparms.desired_access = GENERIC_READ;
	oparms.create_options = CREATE_NOT_DIR;
	oparms.disposition = FILE_OPEN;
	oparms.path = path;
	oparms.fid = &fid;
	oparms.reconnect = false;

	rc = CIFS_open(xid, &oparms, &oplock, &file_info);
	if (rc)
		return rc;

	if (file_info.EndOfFile != cpu_to_le64(CIFS_MF_SYMLINK_FILE_SIZE)) {
		rc = -ENOENT;
		/* it's not a symlink */
		goto out;
	}

	io_parms.netfid = fid.netfid;
	io_parms.pid = current->tgid;
	io_parms.tcon = tcon;
	io_parms.offset = 0;
	io_parms.length = CIFS_MF_SYMLINK_FILE_SIZE;

	rc = CIFSSMBRead(xid, &io_parms, pbytes_read, &pbuf, &buf_type);
out:
	CIFSSMBClose(xid, tcon, fid.netfid);
	return rc;
}

int
cifs_create_mf_symlink(unsigned int xid, struct cifs_tcon *tcon,
		       struct cifs_sb_info *cifs_sb, const unsigned char *path,
		       char *pbuf, unsigned int *pbytes_written)
{
	int rc;
	int oplock = 0;
	struct cifs_fid fid;
	struct cifs_open_parms oparms;
	struct cifs_io_parms io_parms;
	int create_options = CREATE_NOT_DIR;

	if (backup_cred(cifs_sb))
		create_options |= CREATE_OPEN_BACKUP_INTENT;

	oparms.tcon = tcon;
	oparms.cifs_sb = cifs_sb;
	oparms.desired_access = GENERIC_WRITE;
	oparms.create_options = create_options;
	oparms.disposition = FILE_CREATE;
	oparms.path = path;
	oparms.fid = &fid;
	oparms.reconnect = false;

	rc = CIFS_open(xid, &oparms, &oplock, NULL);
	if (rc)
		return rc;

	io_parms.netfid = fid.netfid;
	io_parms.pid = current->tgid;
	io_parms.tcon = tcon;
	io_parms.offset = 0;
	io_parms.length = CIFS_MF_SYMLINK_FILE_SIZE;

	rc = CIFSSMBWrite(xid, &io_parms, pbytes_written, pbuf, NULL, 0);
	CIFSSMBClose(xid, tcon, fid.netfid);
	return rc;
}

/*
 * SMB 2.1/SMB3 Protocol specific functions
 */
#ifdef CONFIG_CIFS_SMB2
int
smb3_query_mf_symlink(unsigned int xid, struct cifs_tcon *tcon,
		      struct cifs_sb_info *cifs_sb, const unsigned char *path,
		      char *pbuf, unsigned int *pbytes_read)
{
	int rc;
	struct cifs_fid fid;
	struct cifs_open_parms oparms;
	struct cifs_io_parms io_parms;
	int buf_type = CIFS_NO_BUFFER;
	__le16 *utf16_path;
	__u8 oplock = SMB2_OPLOCK_LEVEL_II;
	struct smb2_file_all_info *pfile_info = NULL;

	oparms.tcon = tcon;
	oparms.cifs_sb = cifs_sb;
	oparms.desired_access = GENERIC_READ;
	oparms.create_options = CREATE_NOT_DIR;
	if (backup_cred(cifs_sb))
		oparms.create_options |= CREATE_OPEN_BACKUP_INTENT;
	oparms.disposition = FILE_OPEN;
	oparms.fid = &fid;
	oparms.reconnect = false;

	utf16_path = cifs_convert_path_to_utf16(path, cifs_sb);
	if (utf16_path == NULL)
		return -ENOMEM;

	pfile_info = kzalloc(sizeof(struct smb2_file_all_info) + PATH_MAX * 2,
			     GFP_KERNEL);

	if (pfile_info == NULL) {
		kfree(utf16_path);
		return  -ENOMEM;
	}

	rc = SMB2_open(xid, &oparms, utf16_path, &oplock, pfile_info, NULL);
	if (rc)
		goto qmf_out_open_fail;

	if (pfile_info->EndOfFile != cpu_to_le64(CIFS_MF_SYMLINK_FILE_SIZE)) {
		/* it's not a symlink */
		rc = -ENOENT; /* Is there a better rc to return? */
		goto qmf_out;
	}

	io_parms.netfid = fid.netfid;
	io_parms.pid = current->tgid;
	io_parms.tcon = tcon;
	io_parms.offset = 0;
	io_parms.length = CIFS_MF_SYMLINK_FILE_SIZE;
	io_parms.persistent_fid = fid.persistent_fid;
	io_parms.volatile_fid = fid.volatile_fid;
	rc = SMB2_read(xid, &io_parms, pbytes_read, &pbuf, &buf_type);
qmf_out:
	SMB2_close(xid, tcon, fid.persistent_fid, fid.volatile_fid);
qmf_out_open_fail:
	kfree(utf16_path);
	kfree(pfile_info);
	return rc;
}

int
smb3_create_mf_symlink(unsigned int xid, struct cifs_tcon *tcon,
		       struct cifs_sb_info *cifs_sb, const unsigned char *path,
		       char *pbuf, unsigned int *pbytes_written)
{
	int rc;
	struct cifs_fid fid;
	struct cifs_open_parms oparms;
	struct cifs_io_parms io_parms;
	int create_options = CREATE_NOT_DIR;
	__le16 *utf16_path;
	__u8 oplock = SMB2_OPLOCK_LEVEL_EXCLUSIVE;
	struct kvec iov[2];

	if (backup_cred(cifs_sb))
		create_options |= CREATE_OPEN_BACKUP_INTENT;

	cifs_dbg(FYI, "%s: path: %s\n", __func__, path);

	utf16_path = cifs_convert_path_to_utf16(path, cifs_sb);
	if (!utf16_path)
		return -ENOMEM;

	oparms.tcon = tcon;
	oparms.cifs_sb = cifs_sb;
	oparms.desired_access = GENERIC_WRITE;
	oparms.create_options = create_options;
	oparms.disposition = FILE_CREATE;
	oparms.fid = &fid;
	oparms.reconnect = false;

	rc = SMB2_open(xid, &oparms, utf16_path, &oplock, NULL, NULL);
	if (rc) {
		kfree(utf16_path);
		return rc;
	}

	io_parms.netfid = fid.netfid;
	io_parms.pid = current->tgid;
	io_parms.tcon = tcon;
	io_parms.offset = 0;
	io_parms.length = CIFS_MF_SYMLINK_FILE_SIZE;
	io_parms.persistent_fid = fid.persistent_fid;
	io_parms.volatile_fid = fid.volatile_fid;

	/* iov[0] is reserved for smb header */
	iov[1].iov_base = pbuf;
	iov[1].iov_len = CIFS_MF_SYMLINK_FILE_SIZE;

	rc = SMB2_write(xid, &io_parms, pbytes_written, iov, 1);

	/* Make sure we wrote all of the symlink data */
	if ((rc == 0) && (*pbytes_written != CIFS_MF_SYMLINK_FILE_SIZE))
		rc = -EIO;

	SMB2_close(xid, tcon, fid.persistent_fid, fid.volatile_fid);

	kfree(utf16_path);
	return rc;
}
#endif /* CONFIG_CIFS_SMB2 */

/*
 * M-F Symlink Functions - End
 */

int
cifs_hardlink(struct dentry *old_file, struct inode *inode,
	      struct dentry *direntry)
{
	int rc = -EACCES;
	unsigned int xid;
	char *from_name = NULL;
	char *to_name = NULL;
	struct cifs_sb_info *cifs_sb = CIFS_SB(inode->i_sb);
	struct tcon_link *tlink;
	struct cifs_tcon *tcon;
	struct TCP_Server_Info *server;
	struct cifsInodeInfo *cifsInode;

	tlink = cifs_sb_tlink(cifs_sb);
	if (IS_ERR(tlink))
		return PTR_ERR(tlink);
	tcon = tlink_tcon(tlink);

	xid = get_xid();

	from_name = build_path_from_dentry(old_file);
	to_name = build_path_from_dentry(direntry);
	if ((from_name == NULL) || (to_name == NULL)) {
		rc = -ENOMEM;
		goto cifs_hl_exit;
	}

	if (tcon->unix_ext)
		rc = CIFSUnixCreateHardLink(xid, tcon, from_name, to_name,
					    cifs_sb->local_nls,
					    cifs_remap(cifs_sb));
	else {
		server = tcon->ses->server;
		if (!server->ops->create_hardlink) {
			rc = -ENOSYS;
			goto cifs_hl_exit;
		}
		rc = server->ops->create_hardlink(xid, tcon, from_name, to_name,
						  cifs_sb);
		if ((rc == -EIO) || (rc == -EINVAL))
			rc = -EOPNOTSUPP;
	}

	d_drop(direntry);	/* force new lookup from server of target */

	/*
	 * if source file is cached (oplocked) revalidate will not go to server
	 * until the file is closed or oplock broken so update nlinks locally
	 */
	if (d_really_is_positive(old_file)) {
		cifsInode = CIFS_I(d_inode(old_file));
		if (rc == 0) {
			spin_lock(&d_inode(old_file)->i_lock);
			inc_nlink(d_inode(old_file));
			spin_unlock(&d_inode(old_file)->i_lock);

			/*
			 * parent dir timestamps will update from srv within a
			 * second, would it really be worth it to set the parent
			 * dir cifs inode time to zero to force revalidate
			 * (faster) for it too?
			 */
		}
		/*
		 * if not oplocked will force revalidate to get info on source
		 * file from srv.  Note Samba server prior to 4.2 has bug -
		 * not updating src file ctime on hardlinks but Windows servers
		 * handle it properly
		 */
		cifsInode->time = 0;

		/*
		 * Will update parent dir timestamps from srv within a second.
		 * Would it really be worth it to set the parent dir (cifs
		 * inode) time field to zero to force revalidate on parent
		 * directory faster ie
		 *
		 * CIFS_I(inode)->time = 0;
		 */
	}

cifs_hl_exit:
	kfree(from_name);
	kfree(to_name);
	free_xid(xid);
	cifs_put_tlink(tlink);
	return rc;
}

void *
cifs_follow_link(struct dentry *direntry, struct nameidata *nd)
{
	struct inode *inode = d_inode(direntry);
	int rc = -ENOMEM;
	unsigned int xid;
	char *full_path = NULL;
	char *target_path = NULL;
	struct cifs_sb_info *cifs_sb = CIFS_SB(inode->i_sb);
	struct tcon_link *tlink = NULL;
	struct cifs_tcon *tcon;
	struct TCP_Server_Info *server;

	xid = get_xid();

	tlink = cifs_sb_tlink(cifs_sb);
	if (IS_ERR(tlink)) {
		rc = PTR_ERR(tlink);
		tlink = NULL;
		goto out;
	}
	tcon = tlink_tcon(tlink);
	server = tcon->ses->server;

	full_path = build_path_from_dentry(direntry);
	if (!full_path)
		goto out;

	cifs_dbg(FYI, "Full path: %s inode = 0x%p\n", full_path, inode);

	rc = -EACCES;
	/*
	 * First try Minshall+French Symlinks, if configured
	 * and fallback to UNIX Extensions Symlinks.
	 */
	if (cifs_sb->mnt_cifs_flags & CIFS_MOUNT_MF_SYMLINKS)
		rc = query_mf_symlink(xid, tcon, cifs_sb, full_path,
				      &target_path);

	if (rc != 0 && server->ops->query_symlink)
		rc = server->ops->query_symlink(xid, tcon, full_path,
						&target_path, cifs_sb);

	kfree(full_path);
out:
	if (rc != 0) {
		kfree(target_path);
		target_path = ERR_PTR(rc);
	}

	free_xid(xid);
	if (tlink)
		cifs_put_tlink(tlink);
	nd_set_link(nd, target_path);
	return NULL;
}

int
cifs_symlink(struct inode *inode, struct dentry *direntry, const char *symname)
{
	int rc = -EOPNOTSUPP;
	unsigned int xid;
	struct cifs_sb_info *cifs_sb = CIFS_SB(inode->i_sb);
	struct tcon_link *tlink;
	struct cifs_tcon *pTcon;
	char *full_path = NULL;
	struct inode *newinode = NULL;

	xid = get_xid();

	tlink = cifs_sb_tlink(cifs_sb);
	if (IS_ERR(tlink)) {
		rc = PTR_ERR(tlink);
		goto symlink_exit;
	}
	pTcon = tlink_tcon(tlink);

	full_path = build_path_from_dentry(direntry);
	if (full_path == NULL) {
		rc = -ENOMEM;
		goto symlink_exit;
	}

	cifs_dbg(FYI, "Full path: %s\n", full_path);
	cifs_dbg(FYI, "symname is %s\n", symname);

	/* BB what if DFS and this volume is on different share? BB */
	if (cifs_sb->mnt_cifs_flags & CIFS_MOUNT_MF_SYMLINKS)
		rc = create_mf_symlink(xid, pTcon, cifs_sb, full_path, symname);
	else if (pTcon->unix_ext)
		rc = CIFSUnixCreateSymLink(xid, pTcon, full_path, symname,
					   cifs_sb->local_nls,
					   cifs_remap(cifs_sb));
	/* else
	   rc = CIFSCreateReparseSymLink(xid, pTcon, fromName, toName,
					cifs_sb_target->local_nls); */

	if (rc == 0) {
		if (pTcon->unix_ext)
			rc = cifs_get_inode_info_unix(&newinode, full_path,
						      inode->i_sb, xid);
		else
			rc = cifs_get_inode_info(&newinode, full_path, NULL,
						 inode->i_sb, xid, NULL);

		if (rc != 0) {
			cifs_dbg(FYI, "Create symlink ok, getinodeinfo fail rc = %d\n",
				 rc);
		} else {
			d_instantiate(direntry, newinode);
		}
	}
symlink_exit:
	kfree(full_path);
	cifs_put_tlink(tlink);
	free_xid(xid);
	return rc;
}
