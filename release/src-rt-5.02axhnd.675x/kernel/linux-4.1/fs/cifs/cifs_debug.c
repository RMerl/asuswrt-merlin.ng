/*
 *   fs/cifs_debug.c
 *
 *   Copyright (C) International Business Machines  Corp., 2000,2005
 *
 *   Modified by Steve French (sfrench@us.ibm.com)
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include "cifspdu.h"
#include "cifsglob.h"
#include "cifsproto.h"
#include "cifs_debug.h"
#include "cifsfs.h"

void
cifs_dump_mem(char *label, void *data, int length)
{
	pr_debug("%s: dump of %d bytes of data at 0x%p\n", label, length, data);
	print_hex_dump(KERN_DEBUG, "", DUMP_PREFIX_OFFSET, 16, 4,
		       data, length, true);
}

#ifdef CONFIG_CIFS_DEBUG
void cifs_vfs_err(const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);

	vaf.fmt = fmt;
	vaf.va = &args;

	pr_err_ratelimited("CIFS VFS: %pV", &vaf);

	va_end(args);
}
#endif

void cifs_dump_detail(void *buf)
{
#ifdef CONFIG_CIFS_DEBUG2
	struct smb_hdr *smb = (struct smb_hdr *)buf;

	cifs_dbg(VFS, "Cmd: %d Err: 0x%x Flags: 0x%x Flgs2: 0x%x Mid: %d Pid: %d\n",
		 smb->Command, smb->Status.CifsError,
		 smb->Flags, smb->Flags2, smb->Mid, smb->Pid);
	cifs_dbg(VFS, "smb buf %p len %u\n", smb, smbCalcSize(smb));
#endif /* CONFIG_CIFS_DEBUG2 */
}

void cifs_dump_mids(struct TCP_Server_Info *server)
{
#ifdef CONFIG_CIFS_DEBUG2
	struct list_head *tmp;
	struct mid_q_entry *mid_entry;

	if (server == NULL)
		return;

	cifs_dbg(VFS, "Dump pending requests:\n");
	spin_lock(&GlobalMid_Lock);
	list_for_each(tmp, &server->pending_mid_q) {
		mid_entry = list_entry(tmp, struct mid_q_entry, qhead);
		cifs_dbg(VFS, "State: %d Cmd: %d Pid: %d Cbdata: %p Mid %llu\n",
			 mid_entry->mid_state,
			 le16_to_cpu(mid_entry->command),
			 mid_entry->pid,
			 mid_entry->callback_data,
			 mid_entry->mid);
#ifdef CONFIG_CIFS_STATS2
		cifs_dbg(VFS, "IsLarge: %d buf: %p time rcv: %ld now: %ld\n",
			 mid_entry->large_buf,
			 mid_entry->resp_buf,
			 mid_entry->when_received,
			 jiffies);
#endif /* STATS2 */
		cifs_dbg(VFS, "IsMult: %d IsEnd: %d\n",
			 mid_entry->multiRsp, mid_entry->multiEnd);
		if (mid_entry->resp_buf) {
			cifs_dump_detail(mid_entry->resp_buf);
			cifs_dump_mem("existing buf: ",
				mid_entry->resp_buf, 62);
		}
	}
	spin_unlock(&GlobalMid_Lock);
#endif /* CONFIG_CIFS_DEBUG2 */
}

#ifdef CONFIG_PROC_FS
static int cifs_debug_data_proc_show(struct seq_file *m, void *v)
{
	struct list_head *tmp1, *tmp2, *tmp3;
	struct mid_q_entry *mid_entry;
	struct TCP_Server_Info *server;
	struct cifs_ses *ses;
	struct cifs_tcon *tcon;
	int i, j;
	__u32 dev_type;

	seq_puts(m,
		    "Display Internal CIFS Data Structures for Debugging\n"
		    "---------------------------------------------------\n");
	seq_printf(m, "CIFS Version %s\n", CIFS_VERSION);
	seq_printf(m, "Features:");
#ifdef CONFIG_CIFS_DFS_UPCALL
	seq_printf(m, " dfs");
#endif
#ifdef CONFIG_CIFS_FSCACHE
	seq_printf(m, " fscache");
#endif
#ifdef CONFIG_CIFS_WEAK_PW_HASH
	seq_printf(m, " lanman");
#endif
#ifdef CONFIG_CIFS_POSIX
	seq_printf(m, " posix");
#endif
#ifdef CONFIG_CIFS_UPCALL
	seq_printf(m, " spnego");
#endif
#ifdef CONFIG_CIFS_XATTR
	seq_printf(m, " xattr");
#endif
#ifdef CONFIG_CIFS_ACL
	seq_printf(m, " acl");
#endif
	seq_putc(m, '\n');
	seq_printf(m, "Active VFS Requests: %d\n", GlobalTotalActiveXid);
	seq_printf(m, "Servers:");

	i = 0;
	spin_lock(&cifs_tcp_ses_lock);
	list_for_each(tmp1, &cifs_tcp_ses_list) {
		server = list_entry(tmp1, struct TCP_Server_Info,
				    tcp_ses_list);
		i++;
		list_for_each(tmp2, &server->smb_ses_list) {
			ses = list_entry(tmp2, struct cifs_ses,
					 smb_ses_list);
			if ((ses->serverDomain == NULL) ||
				(ses->serverOS == NULL) ||
				(ses->serverNOS == NULL)) {
				seq_printf(m, "\n%d) entry for %s not fully "
					   "displayed\n\t", i, ses->serverName);
			} else {
				seq_printf(m,
				    "\n%d) Name: %s  Domain: %s Uses: %d OS:"
				    " %s\n\tNOS: %s\tCapability: 0x%x\n\tSMB"
				    " session status: %d\t",
				i, ses->serverName, ses->serverDomain,
				ses->ses_count, ses->serverOS, ses->serverNOS,
				ses->capabilities, ses->status);
			}
			seq_printf(m, "TCP status: %d\n\tLocal Users To "
				   "Server: %d SecMode: 0x%x Req On Wire: %d",
				   server->tcpStatus, server->srv_count,
				   server->sec_mode, in_flight(server));

#ifdef CONFIG_CIFS_STATS2
			seq_printf(m, " In Send: %d In MaxReq Wait: %d",
				atomic_read(&server->in_send),
				atomic_read(&server->num_waiters));
#endif

			seq_puts(m, "\n\tShares:");
			j = 0;
			list_for_each(tmp3, &ses->tcon_list) {
				tcon = list_entry(tmp3, struct cifs_tcon,
						  tcon_list);
				++j;
				dev_type = le32_to_cpu(tcon->fsDevInfo.DeviceType);
				seq_printf(m, "\n\t%d) %s Mounts: %d ", j,
					   tcon->treeName, tcon->tc_count);
				if (tcon->nativeFileSystem) {
					seq_printf(m, "Type: %s ",
						   tcon->nativeFileSystem);
				}
				seq_printf(m, "DevInfo: 0x%x Attributes: 0x%x"
					"\n\tPathComponentMax: %d Status: %d",
					le32_to_cpu(tcon->fsDevInfo.DeviceCharacteristics),
					le32_to_cpu(tcon->fsAttrInfo.Attributes),
					le32_to_cpu(tcon->fsAttrInfo.MaxPathNameComponentLength),
					tcon->tidStatus);
				if (dev_type == FILE_DEVICE_DISK)
					seq_puts(m, " type: DISK ");
				else if (dev_type == FILE_DEVICE_CD_ROM)
					seq_puts(m, " type: CDROM ");
				else
					seq_printf(m, " type: %d ", dev_type);
				if (server->ops->dump_share_caps)
					server->ops->dump_share_caps(m, tcon);

				if (tcon->need_reconnect)
					seq_puts(m, "\tDISCONNECTED ");
				seq_putc(m, '\n');
			}

			seq_puts(m, "\n\tMIDs:\n");

			spin_lock(&GlobalMid_Lock);
			list_for_each(tmp3, &server->pending_mid_q) {
				mid_entry = list_entry(tmp3, struct mid_q_entry,
					qhead);
				seq_printf(m, "\tState: %d com: %d pid:"
					      " %d cbdata: %p mid %llu\n",
					      mid_entry->mid_state,
					      le16_to_cpu(mid_entry->command),
					      mid_entry->pid,
					      mid_entry->callback_data,
					      mid_entry->mid);
			}
			spin_unlock(&GlobalMid_Lock);
		}
	}
	spin_unlock(&cifs_tcp_ses_lock);
	seq_putc(m, '\n');

	/* BB add code to dump additional info such as TCP session info now */
	return 0;
}

static int cifs_debug_data_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cifs_debug_data_proc_show, NULL);
}

static const struct file_operations cifs_debug_data_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= cifs_debug_data_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

#ifdef CONFIG_CIFS_STATS
static ssize_t cifs_stats_proc_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	char c;
	bool bv;
	int rc;
	struct list_head *tmp1, *tmp2, *tmp3;
	struct TCP_Server_Info *server;
	struct cifs_ses *ses;
	struct cifs_tcon *tcon;

	rc = get_user(c, buffer);
	if (rc)
		return rc;

	if (strtobool(&c, &bv) == 0) {
#ifdef CONFIG_CIFS_STATS2
		atomic_set(&totBufAllocCount, 0);
		atomic_set(&totSmBufAllocCount, 0);
#endif /* CONFIG_CIFS_STATS2 */
		spin_lock(&cifs_tcp_ses_lock);
		list_for_each(tmp1, &cifs_tcp_ses_list) {
			server = list_entry(tmp1, struct TCP_Server_Info,
					    tcp_ses_list);
			list_for_each(tmp2, &server->smb_ses_list) {
				ses = list_entry(tmp2, struct cifs_ses,
						 smb_ses_list);
				list_for_each(tmp3, &ses->tcon_list) {
					tcon = list_entry(tmp3,
							  struct cifs_tcon,
							  tcon_list);
					atomic_set(&tcon->num_smbs_sent, 0);
					if (server->ops->clear_stats)
						server->ops->clear_stats(tcon);
				}
			}
		}
		spin_unlock(&cifs_tcp_ses_lock);
	}

	return count;
}

static int cifs_stats_proc_show(struct seq_file *m, void *v)
{
	int i;
	struct list_head *tmp1, *tmp2, *tmp3;
	struct TCP_Server_Info *server;
	struct cifs_ses *ses;
	struct cifs_tcon *tcon;

	seq_printf(m,
			"Resources in use\nCIFS Session: %d\n",
			sesInfoAllocCount.counter);
	seq_printf(m, "Share (unique mount targets): %d\n",
			tconInfoAllocCount.counter);
	seq_printf(m, "SMB Request/Response Buffer: %d Pool size: %d\n",
			bufAllocCount.counter,
			cifs_min_rcv + tcpSesAllocCount.counter);
	seq_printf(m, "SMB Small Req/Resp Buffer: %d Pool size: %d\n",
			smBufAllocCount.counter, cifs_min_small);
#ifdef CONFIG_CIFS_STATS2
	seq_printf(m, "Total Large %d Small %d Allocations\n",
				atomic_read(&totBufAllocCount),
				atomic_read(&totSmBufAllocCount));
#endif /* CONFIG_CIFS_STATS2 */

	seq_printf(m, "Operations (MIDs): %d\n", atomic_read(&midCount));
	seq_printf(m,
		"\n%d session %d share reconnects\n",
		tcpSesReconnectCount.counter, tconInfoReconnectCount.counter);

	seq_printf(m,
		"Total vfs operations: %d maximum at one time: %d\n",
		GlobalCurrentXid, GlobalMaxActiveXid);

	i = 0;
	spin_lock(&cifs_tcp_ses_lock);
	list_for_each(tmp1, &cifs_tcp_ses_list) {
		server = list_entry(tmp1, struct TCP_Server_Info,
				    tcp_ses_list);
		list_for_each(tmp2, &server->smb_ses_list) {
			ses = list_entry(tmp2, struct cifs_ses,
					 smb_ses_list);
			list_for_each(tmp3, &ses->tcon_list) {
				tcon = list_entry(tmp3,
						  struct cifs_tcon,
						  tcon_list);
				i++;
				seq_printf(m, "\n%d) %s", i, tcon->treeName);
				if (tcon->need_reconnect)
					seq_puts(m, "\tDISCONNECTED ");
				seq_printf(m, "\nSMBs: %d",
					   atomic_read(&tcon->num_smbs_sent));
				if (server->ops->print_stats)
					server->ops->print_stats(m, tcon);
			}
		}
	}
	spin_unlock(&cifs_tcp_ses_lock);

	seq_putc(m, '\n');
	return 0;
}

static int cifs_stats_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cifs_stats_proc_show, NULL);
}

static const struct file_operations cifs_stats_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= cifs_stats_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= cifs_stats_proc_write,
};
#endif /* STATS */

static struct proc_dir_entry *proc_fs_cifs;
static const struct file_operations cifsFYI_proc_fops;
static const struct file_operations cifs_lookup_cache_proc_fops;
static const struct file_operations traceSMB_proc_fops;
static const struct file_operations cifs_security_flags_proc_fops;
static const struct file_operations cifs_linux_ext_proc_fops;

void
cifs_proc_init(void)
{
	proc_fs_cifs = proc_mkdir("fs/cifs", NULL);
	if (proc_fs_cifs == NULL)
		return;

	proc_create("DebugData", 0, proc_fs_cifs, &cifs_debug_data_proc_fops);

#ifdef CONFIG_CIFS_STATS
	proc_create("Stats", 0, proc_fs_cifs, &cifs_stats_proc_fops);
#endif /* STATS */
	proc_create("cifsFYI", 0, proc_fs_cifs, &cifsFYI_proc_fops);
	proc_create("traceSMB", 0, proc_fs_cifs, &traceSMB_proc_fops);
	proc_create("LinuxExtensionsEnabled", 0, proc_fs_cifs,
		    &cifs_linux_ext_proc_fops);
	proc_create("SecurityFlags", 0, proc_fs_cifs,
		    &cifs_security_flags_proc_fops);
	proc_create("LookupCacheEnabled", 0, proc_fs_cifs,
		    &cifs_lookup_cache_proc_fops);
}

void
cifs_proc_clean(void)
{
	if (proc_fs_cifs == NULL)
		return;

	remove_proc_entry("DebugData", proc_fs_cifs);
	remove_proc_entry("cifsFYI", proc_fs_cifs);
	remove_proc_entry("traceSMB", proc_fs_cifs);
#ifdef CONFIG_CIFS_STATS
	remove_proc_entry("Stats", proc_fs_cifs);
#endif
	remove_proc_entry("SecurityFlags", proc_fs_cifs);
	remove_proc_entry("LinuxExtensionsEnabled", proc_fs_cifs);
	remove_proc_entry("LookupCacheEnabled", proc_fs_cifs);
	remove_proc_entry("fs/cifs", NULL);
}

static int cifsFYI_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", cifsFYI);
	return 0;
}

static int cifsFYI_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cifsFYI_proc_show, NULL);
}

static ssize_t cifsFYI_proc_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *ppos)
{
	char c;
	bool bv;
	int rc;

	rc = get_user(c, buffer);
	if (rc)
		return rc;
	if (strtobool(&c, &bv) == 0)
		cifsFYI = bv;
	else if ((c > '1') && (c <= '9'))
		cifsFYI = (int) (c - '0'); /* see cifs_debug.h for meanings */

	return count;
}

static const struct file_operations cifsFYI_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= cifsFYI_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= cifsFYI_proc_write,
};

static int cifs_linux_ext_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", linuxExtEnabled);
	return 0;
}

static int cifs_linux_ext_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cifs_linux_ext_proc_show, NULL);
}

static ssize_t cifs_linux_ext_proc_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	char c;
	bool bv;
	int rc;

	rc = get_user(c, buffer);
	if (rc)
		return rc;

	rc = strtobool(&c, &bv);
	if (rc)
		return rc;

	linuxExtEnabled = bv;

	return count;
}

static const struct file_operations cifs_linux_ext_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= cifs_linux_ext_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= cifs_linux_ext_proc_write,
};

static int cifs_lookup_cache_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", lookupCacheEnabled);
	return 0;
}

static int cifs_lookup_cache_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cifs_lookup_cache_proc_show, NULL);
}

static ssize_t cifs_lookup_cache_proc_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	char c;
	bool bv;
	int rc;

	rc = get_user(c, buffer);
	if (rc)
		return rc;

	rc = strtobool(&c, &bv);
	if (rc)
		return rc;

	lookupCacheEnabled = bv;

	return count;
}

static const struct file_operations cifs_lookup_cache_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= cifs_lookup_cache_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= cifs_lookup_cache_proc_write,
};

static int traceSMB_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", traceSMB);
	return 0;
}

static int traceSMB_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, traceSMB_proc_show, NULL);
}

static ssize_t traceSMB_proc_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *ppos)
{
	char c;
	bool bv;
	int rc;

	rc = get_user(c, buffer);
	if (rc)
		return rc;

	rc = strtobool(&c, &bv);
	if (rc)
		return rc;

	traceSMB = bv;

	return count;
}

static const struct file_operations traceSMB_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= traceSMB_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= traceSMB_proc_write,
};

static int cifs_security_flags_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "0x%x\n", global_secflags);
	return 0;
}

static int cifs_security_flags_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cifs_security_flags_proc_show, NULL);
}

/*
 * Ensure that if someone sets a MUST flag, that we disable all other MAY
 * flags except for the ones corresponding to the given MUST flag. If there are
 * multiple MUST flags, then try to prefer more secure ones.
 */
static void
cifs_security_flags_handle_must_flags(unsigned int *flags)
{
	unsigned int signflags = *flags & CIFSSEC_MUST_SIGN;

	if ((*flags & CIFSSEC_MUST_KRB5) == CIFSSEC_MUST_KRB5)
		*flags = CIFSSEC_MUST_KRB5;
	else if ((*flags & CIFSSEC_MUST_NTLMSSP) == CIFSSEC_MUST_NTLMSSP)
		*flags = CIFSSEC_MUST_NTLMSSP;
	else if ((*flags & CIFSSEC_MUST_NTLMV2) == CIFSSEC_MUST_NTLMV2)
		*flags = CIFSSEC_MUST_NTLMV2;
	else if ((*flags & CIFSSEC_MUST_NTLM) == CIFSSEC_MUST_NTLM)
		*flags = CIFSSEC_MUST_NTLM;
	else if (CIFSSEC_MUST_LANMAN &&
		 (*flags & CIFSSEC_MUST_LANMAN) == CIFSSEC_MUST_LANMAN)
		*flags = CIFSSEC_MUST_LANMAN;
	else if (CIFSSEC_MUST_PLNTXT &&
		 (*flags & CIFSSEC_MUST_PLNTXT) == CIFSSEC_MUST_PLNTXT)
		*flags = CIFSSEC_MUST_PLNTXT;

	*flags |= signflags;
}

static ssize_t cifs_security_flags_proc_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	int rc;
	unsigned int flags;
	char flags_string[12];
	char c;
	bool bv;

	if ((count < 1) || (count > 11))
		return -EINVAL;

	memset(flags_string, 0, 12);

	if (copy_from_user(flags_string, buffer, count))
		return -EFAULT;

	if (count < 3) {
		/* single char or single char followed by null */
		c = flags_string[0];
		if (strtobool(&c, &bv) == 0) {
			global_secflags = bv ? CIFSSEC_MAX : CIFSSEC_DEF;
			return count;
		} else if (!isdigit(c)) {
			cifs_dbg(VFS, "Invalid SecurityFlags: %s\n",
					flags_string);
			return -EINVAL;
		}
	}

	/* else we have a number */
	rc = kstrtouint(flags_string, 0, &flags);
	if (rc) {
		cifs_dbg(VFS, "Invalid SecurityFlags: %s\n",
				flags_string);
		return rc;
	}

	cifs_dbg(FYI, "sec flags 0x%x\n", flags);

	if (flags == 0)  {
		cifs_dbg(VFS, "Invalid SecurityFlags: %s\n", flags_string);
		return -EINVAL;
	}

	if (flags & ~CIFSSEC_MASK) {
		cifs_dbg(VFS, "Unsupported security flags: 0x%x\n",
			 flags & ~CIFSSEC_MASK);
		return -EINVAL;
	}

	cifs_security_flags_handle_must_flags(&flags);

	/* flags look ok - update the global security flags for cifs module */
	global_secflags = flags;
	if (global_secflags & CIFSSEC_MUST_SIGN) {
		/* requiring signing implies signing is allowed */
		global_secflags |= CIFSSEC_MAY_SIGN;
		cifs_dbg(FYI, "packet signing now required\n");
	} else if ((global_secflags & CIFSSEC_MAY_SIGN) == 0) {
		cifs_dbg(FYI, "packet signing disabled\n");
	}
	/* BB should we turn on MAY flags for other MUST options? */
	return count;
}

static const struct file_operations cifs_security_flags_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= cifs_security_flags_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= cifs_security_flags_proc_write,
};
#else
inline void cifs_proc_init(void)
{
}

inline void cifs_proc_clean(void)
{
}
#endif /* PROC_FS */
