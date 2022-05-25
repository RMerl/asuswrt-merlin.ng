/*
* <:copyright-BRCM:2015:DUAL/GPL:standard
* 
*    Copyright (c) 2015 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/

/*****************************************************************************
 *
 * Copyright (c) 2013 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Tim Ross <tross@broadcom.com>
 *****************************************************************************/
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/io.h>

#include "fpm.h"
#include "fpm_dev.h"
#include "fpm_priv.h"
#include "proc_cmd.h"

#define PROC_DIR		"driver/fpm"
#define CMD_PROC_FILE		"cmd"
#define HEAD_PROC_FILE		"head"
#define TAIL_PROC_FILE		"tail"
#define PKT_PROC_FILE		"pkt"
#define HP_PROC_FILE		"hp"
#define HPT_PROC_FILE		"hpt"
#define FPM_PROC_FILE		"fpm"
#define HIST_PROC_FILE		"hist"
#define ISR_TIMER_PROC_FILE	"isrtimer"

static u32 token;
static u32 head_size = FPM_NET_BUF_HEAD_PAD;
static u32 tail_size = FPM_NET_BUF_TAIL_PAD;
struct seq_buf {
	u8	*buf;
	u32	remain;
	loff_t	offset;
	u8	line[5 + 32 * 3 + 2 + 32 + 1];
};
static struct seq_buf sbuf;

static int fpm_proc_cmd_help(int argc, char *argv[])
{
	int status = 0;

	if (argc != 2) {
		pr_err("Invalid # of arguments.\n");
		status = -EINVAL;
		goto done;
	}

	if (!strncmp(argv[1], "set", sizeof("set"))) {
		pr_info("set [tok | idx | size | head | tail] {val}\n");
		pr_info("\ttok    set the token to {val}\n");
		pr_info("\tidx    set the token index to {val}\n");
		pr_info("\tsize   set the token size to {val}\n");
		pr_info("\thead   set the head size (offset) to {val}\n");
		pr_info("\ttail   set the tail size (padding) to {val}\n");
	} else if (!strncmp(argv[1], "get", sizeof("get"))) {
		pr_info("get [tok | idx | size | head | tail]\n");
		pr_info("\ttok    get the token val\n");
		pr_info("\tidx    get the token index val\n");
		pr_info("\tsize   get the token size val\n");
		pr_info("\thead   get the head size (offset) val\n");
		pr_info("\ttail   get the tail size (padding) val\n");
	} else if (!strncmp(argv[1], "stats", sizeof("stats"))) {
		pr_info("stats\n");
		pr_info("\tGet info and stats about the FPM block.\n");
#ifdef CONFIG_BCM_FPM_TOKEN_HIST
	} else if (!strncmp(argv[1], "hist", sizeof("hist"))) {
		pr_info("hist {on|off|stop_on_err}\n");
		pr_info("\ton|off       turn token tracking history on/off\n");
		pr_info("\tstop_on_err  stop tracking when error detected\n");
		pr_info("\tcont_on_err  continue tracking when error detected\n");
#endif
	} else {
		pr_err("Unrecognized command: %s\n", argv[1]);
		status = -EINVAL;
	}

done:
	return status;
}

static int fpm_proc_cmd_set_get(int argc, char *argv[])
{
	int status = 0;
	u32 idx, size;

	if (!strncmp(argv[0], "set", sizeof("set"))) {
		if (argc != 3) {
			pr_err("Invalid # of arguments.\n");
			status = -EINVAL;
			goto done;
		}

		if (!strncmp(argv[1], "tok", strlen("tok"))) {
			status = kstrtou32(argv[2], 0, &token);
		} else if (!strncmp(argv[1], "idx", strlen("idx"))) {
			status = kstrtou32(argv[2], 0, &idx);
			token &= ~FPM_TOKEN_INDEX_MASK;
			token |= ((idx << FPM_TOKEN_INDEX_SHIFT)
				  & FPM_TOKEN_INDEX_MASK);
		} else if (!strncmp(argv[1], "size", strlen("size"))) {
			status = kstrtou32(argv[2], 0, &size);
			fpm_set_token_size(&token, size);
		} else if (!strncmp(argv[1], "head", strlen("head"))) {
			status = kstrtou32(argv[2], 0, &head_size);
		} else if (!strncmp(argv[1], "tail", strlen("tail"))) {
			status = kstrtou32(argv[2], 0, &head_size);
		} else {
			pr_err("Unrecognized variable: %s\n", argv[1]);
			status = -EINVAL;
			goto done;
		}
		if (status) {
			pr_err("Bad value: %s\n", argv[2]);
			goto done;
		}
	} else if (!strncmp(argv[0], "get", sizeof("get"))) {
		if (argc != 2) {
			pr_err("Invalid # of arguments.\n");
			status = -EINVAL;
			goto done;
		}

		if (!strncmp(argv[1], "tok", strlen("tok")))
			pr_info("token: 0x%08x\n", token);
		else if (!strncmp(argv[1], "idx", strlen("idx")))
			pr_info("token index: 0x%05x\n",
				FPM_TOKEN_INDEX(token));
		else if (!strncmp(argv[1], "size", strlen("size")))
			pr_info("token size: 0x%03x\n",
				fpm_get_token_size(token));
		else if (!strncmp(argv[1], "head", strlen("head")))
			pr_info("head size: %d bytes\n", head_size);
		else if (!strncmp(argv[1], "tail", strlen("tail")))
			pr_info("tail size: %d bytes\n", tail_size);
		else {
			pr_err("Unrecognized variable: %s\n", argv[1]);
			status = -EINVAL;
			goto done;
		}
	}
	else {
		pr_err("Unrecognized command: %s\n", argv[0]);
		status = -EINVAL;
	}

done:
	return status;
}

static int fpm_proc_cmd_stats(int argc, char *argv[])
{
	int status = 0;
	struct fpm_hw_info info;
	struct fpm_pool_stats stats;
	int i;

	if (argc != 1) {
		pr_err("Invalid # of arguments.\n");
		status = -EINVAL;
		goto done;
	}

	fpm_get_hw_info(&info);
	pr_info("Buffer Size (bytes)..............%d\n",
		info.chunk_size);

	for (i = 0; i < FPM_POOL_NUM; i++) {
		pr_info("\n");
		pr_info("Pool %d:\n", i);
		if (!fpm->pool_pbase[i]) {
			pr_info("Unsupported by chip or disabled in DT\n");
			continue;
		}
		fpm_get_pool_stats(i, &stats);
		pr_info("Base Address (Phys)..............0x%p\n",
			(void *)fpm->pool_pbase[i]);
		pr_info("Base Address (Virt)..............0x%p\n",
			fpm->pool_vbase[i]);
		pr_info("Pool Size (bytes)................%d\n",
			fpm->pool_size[i]);
		pr_info("Total Tokens.....................%d\n",
			fpm->pool_size[i] / info.chunk_size);
		pr_info("Available Tokens.................%d\n",
			stats.tok_avail);
		pr_info("Pool Alloc Weight................%d\n",
			fpm->pool_alloc_weight[i]);
		pr_info("Pool Free Weight.................%d\n",
			fpm->pool_free_weight[i]);
		pr_info("Underflow Count..................%d\n",
			stats.underflow_count);
		pr_info("Overflow Count...................%d\n",
			stats.overflow_count);
		pr_info("Alloc FIFO Empty.................%d\n",
			stats.alloc_fifo_empty);
		pr_info("Alloc FIFO Full..................%d\n",
			stats.alloc_fifo_full);
		pr_info("Free FIFO Empty..................%d\n",
			stats.free_fifo_empty);
		pr_info("Free FIFO Full...................%d\n",
			stats.free_fifo_full);
		pr_info("Pool Full........................%d\n",
			stats.pool_full);
		pr_info("Not Valid Token Frees............%d\n",
			stats.invalid_tok_frees);
		pr_info("Not Valid Token Multicount.......%d\n",
			stats.invalid_tok_multi);
		pr_info("Token Available Low Watermark....%d\n",
			stats.tok_avail_low_wtmk);
	}

done:
	return status;
}

static void *fpm_head_proc_start(struct seq_file *seq, loff_t *pos)
{
	struct seq_buf *sb = NULL;

	if (*pos == 0) {
		sb = &sbuf;
		sb->offset = 0;
		sb->buf = fpm_token_to_buffer(token) + sb->offset;
		sb->remain = head_size;
		seq_printf(seq, "-----------------------------head----------");
		seq_printf(seq, "--------------------------\n");
	}

	return sb;
}

static void *fpm_tail_proc_start(struct seq_file *seq, loff_t *pos)
{
	struct seq_buf *sb = NULL;

	if (*pos == 0) {
		sb = &sbuf;
		sb->offset = head_size + fpm_get_token_size(token);
		sb->buf = fpm_token_to_buffer(token) + sb->offset;
		sb->remain = tail_size;
		seq_printf(seq, "-----------------------------tail----------");
		seq_printf(seq, "--------------------------\n");
	}

	return sb;
}

static void *fpm_pkt_proc_start(struct seq_file *seq, loff_t *pos)
{
	struct seq_buf *sb = NULL;

	if (*pos == 0) {
		sb = &sbuf;
		sb->offset = head_size;
		sb->buf = fpm_token_to_buffer(token) + sb->offset;
		sb->remain = fpm_get_token_size(token);
		seq_printf(seq, "-----------------------------pkt-----------");
		seq_printf(seq, "--------------------------\n");
	}

	return sb;
}

static void *fpm_hp_proc_start(struct seq_file *seq, loff_t *pos)
{
	struct seq_buf *sb = NULL;

	if (*pos == 0) {
		sb = &sbuf;
		sb->offset = 0;
		sb->buf = fpm_token_to_buffer(token) + sb->offset;
		sb->remain = head_size;
		seq_printf(seq, "-----------------------------head----------");
		seq_printf(seq, "--------------------------\n");
	} else if (*pos == 1) {
		sb = &sbuf;
		sb->buf = fpm_token_to_buffer(token) + sb->offset;
		sb->remain = fpm_get_token_size(token);
		seq_printf(seq, "-----------------------------pkt-----------");
		seq_printf(seq, "--------------------------\n");
	}

	return sb;
}

static void *fpm_hpt_proc_start(struct seq_file *seq, loff_t *pos)
{
	struct seq_buf *sb = NULL;

	if (*pos == 0) {
		sb = &sbuf;
		sb->offset = 0;
		sb->buf = fpm_token_to_buffer(token) + sb->offset;
		sb->remain = head_size;
		seq_printf(seq, "-----------------------------head----------");
		seq_printf(seq, "--------------------------\n");
	} else if (*pos == 1) {
		sb = &sbuf;
		sb->buf = fpm_token_to_buffer(token) + sb->offset;
		sb->remain = fpm_get_token_size(token);
		seq_printf(seq, "-----------------------------pkt-----------");
		seq_printf(seq, "--------------------------\n");
	} else if (*pos == 2) {
		sb = &sbuf;
		sb->buf = fpm_token_to_buffer(token) + sb->offset;
		sb->remain = tail_size;
		seq_printf(seq, "-----------------------------tail----------");
		seq_printf(seq, "--------------------------\n");
	}

	return sb;
}

static void fpm_buf_proc_stop(struct seq_file *seq, void *v)
{
}

static void *fpm_buf_proc_next(struct seq_file *seq, void *v,
			       loff_t *pos)
{
	struct seq_buf *sb = v;

	if (!sb->remain)
		sb = NULL;

	return sb;
}

static int fpm_buf_proc_show(struct seq_file *seq, void *v)
{
	struct seq_buf *sb = v;
	int len;

	len = sb->remain < 16 ? sb->remain : 16;
	snprintf(sb->line, 6, "%03llx: ", sb->offset);
	hex_dump_to_buffer(sb->buf, len, 16, 1,
			   &sb->line[5], sizeof(sb->line)-5, true);
	seq_printf(seq, "%s\n", sb->line);
	sb->remain -= len;
	sb->buf += len;
	sb->offset += len;

	return 0;
}

static const struct seq_operations pkt_seq_ops = {
	.start	= fpm_pkt_proc_start,
	.stop	= fpm_buf_proc_stop,
	.next	= fpm_buf_proc_next,
	.show	= fpm_buf_proc_show,
};
static int fpm_pkt_proc_open(struct inode *inode, struct file *file)
{
	int fid;
	fid = seq_open(file, &pkt_seq_ops);
	return fid;
}
static const struct file_operations pkt_fops = {
	.owner		= THIS_MODULE,
	.open		= fpm_pkt_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

static const struct seq_operations head_seq_ops = {
	.start	= fpm_head_proc_start,
	.stop	= fpm_buf_proc_stop,
	.next	= fpm_buf_proc_next,
	.show	= fpm_buf_proc_show,
};
static int fpm_head_proc_open(struct inode *inode, struct file *file)
{
	int fid;
	fid = seq_open(file, &head_seq_ops);
	return fid;
}
static const struct file_operations head_fops = {
	.owner		= THIS_MODULE,
	.open		= fpm_head_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

static const struct seq_operations tail_seq_ops = {
	.start	= fpm_tail_proc_start,
	.stop	= fpm_buf_proc_stop,
	.next	= fpm_buf_proc_next,
	.show	= fpm_buf_proc_show,
};
static int fpm_tail_proc_open(struct inode *inode, struct file *file)
{
	int fid;
	fid = seq_open(file, &tail_seq_ops);
	return fid;
}
static const struct file_operations tail_fops = {
	.owner		= THIS_MODULE,
	.open		= fpm_tail_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

static const struct seq_operations hp_seq_ops = {
	.start	= fpm_hp_proc_start,
	.stop	= fpm_buf_proc_stop,
	.next	= fpm_buf_proc_next,
	.show	= fpm_buf_proc_show,
};
static int fpm_hp_proc_open(struct inode *inode, struct file *file)
{
	int fid;
	fid = seq_open(file, &hp_seq_ops);
	return fid;
}
static const struct file_operations hp_fops = {
	.owner		= THIS_MODULE,
	.open		= fpm_hp_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

static const struct seq_operations hpt_seq_ops = {
	.start	= fpm_hpt_proc_start,
	.stop	= fpm_buf_proc_stop,
	.next	= fpm_buf_proc_next,
	.show	= fpm_buf_proc_show,
};
static int fpm_hpt_proc_open(struct inode *inode, struct file *file)
{
	int fid;
	fid = seq_open(file, &hpt_seq_ops);
	return fid;
}
static const struct file_operations hpt_fops = {
	.owner		= THIS_MODULE,
	.open		= fpm_hpt_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

static int fpm_isr_timer_show(struct seq_file *m, void *v)
{
	seq_printf(m, "FPM ISR timer is set to %lu msec\n", isr_timer_period);
	return 0;
}
static int fpm_isr_timer_open(struct inode *inode, struct file *file)
{
	return single_open(file, fpm_isr_timer_show, PDE_DATA(inode));
}
static ssize_t fpm_isr_timer_write(struct file *f, const char __user *buffer,
				   size_t count, loff_t *off)
{
	char buf[16];
	int len;
	unsigned long input_val;

	if (count >= sizeof(buf)) {
		pr_err("invalid input value\n");
		return count;
	}

	len = min((unsigned long)count, (unsigned long)(sizeof(buf) - 1));

	if (copy_from_user(buf, buffer, len)) {
		pr_err("invalid input value\n");
		return count;
	}

	buf[len] = '\0';
	if (kstrtoul(buf, 0, &input_val)) {
		pr_err("invalid input value\n");
		return count;
	}

	pr_info("Changing FPM ISR timer to %lu msec (old was %lu msec)\n",
		input_val, isr_timer_period);
	isr_timer_period = input_val;
	return count;
}
static const struct file_operations isr_timer_fops = {
	.owner		= THIS_MODULE,
	.open 		= fpm_isr_timer_open,
	.read 		= seq_read,
	.llseek 	= seq_lseek,
	.release 	= single_release,
	.write		= fpm_isr_timer_write,
};

#ifdef CONFIG_BCM_FPM_TOKEN_HIST
static int fpm_proc_cmd_hist(int argc, char *argv[])
{
	struct fpmdev *fdev = fpm;
	int status = 0;

	if (argc != 2) {
		pr_err("Invalid # of arguments.\n");
		status = -EINVAL;
		goto done;
	}

	if (!strncmp(argv[1], "on", strlen("on")))
		fdev->track_tokens = true;
	else if (!strncmp(argv[1], "off", strlen("off")))
		fdev->track_tokens = false;
	else if (!strncmp(argv[1], "stop_on_err", strlen("stop_on_err")))
		fdev->track_on_err = false;
	else if (!strncmp(argv[1], "cont_on_err", strlen("cont_on_err")))
		fdev->track_on_err = true;
	else {
		pr_err("Unrecognized command: %s\n", argv[1]);
		status = -EINVAL;
		goto done;
	}

done:
	return status;
}

static void *fpm_hist_proc_start(struct seq_file *seq, loff_t *pos)
{
	struct fpmdev *fdev = fpm;
	struct fpm_tok_op *op;

	if (*pos == 0)
		seq_printf(seq, "===============Token History==============\n");
	op = &fdev->tok_hist_tail[*pos];
	if (op >= fdev->tok_hist_end)
		op = &fdev->tok_hist_start[op - fdev->tok_hist_end];
	if (op == fdev->tok_hist_head || *pos >= FPM_NUM_HISTORY_ENTRIES)
		op = NULL;

	return op;
}

static void fpm_hist_proc_stop(struct seq_file *seq, void *v)
{
	if (!v)
		seq_printf(seq, "==========================================\n");
}

static void *fpm_hist_proc_next(struct seq_file *seq, void *v,
				loff_t *pos)
{
	struct fpmdev *fdev = fpm;
	struct fpm_tok_op *op = v;

	(*pos)++;
	op++;
	if (op >= fdev->tok_hist_end)
		op = fdev->tok_hist_start;
	if (op == fdev->tok_hist_head || *pos >= FPM_NUM_HISTORY_ENTRIES)
		op = NULL;

	return op;
}

static int fpm_hist_proc_show(struct seq_file *seq, void *v)
{
	struct fpm_tok_op *op = v;
	char linebuf[80];

	seq_printf(seq, "token: 0x%08x\tref count: %d\n", op->token,
		   op->ref_count);
	seq_printf(seq, "op: %pf\tcalled by: %pf\n", op->called,
		   op->caller);
	seq_printf(seq, "timestamp (jiffies): 0x%08x\n", op->ts);
	seq_printf(seq, "op data: 0x%08x\n", op->op_data);
	hex_dump_to_buffer(op->buf, sizeof(op->buf), 16, 1,
			   linebuf, sizeof(linebuf), true);
	seq_printf(seq, "%s\n", linebuf);
	seq_printf(seq, "------------------------------------------\n");

	return 0;
}

static const struct seq_operations hist_seq_ops = {
	.start	= fpm_hist_proc_start,
	.stop	= fpm_hist_proc_stop,
	.next	= fpm_hist_proc_next,
	.show	= fpm_hist_proc_show,
};
static int fpm_hist_proc_open(struct inode *inode, struct file *file)
{
	int fid;
	fid = seq_open(file, &hist_seq_ops);
	return fid;
}
static const struct file_operations hist_fops = {
	.owner		= THIS_MODULE,
	.open		= fpm_hist_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};
#endif

static struct proc_cmd_ops command_entries[] = {
	{ .name = "help", .do_command	= fpm_proc_cmd_help},
	{ .name = "set", .do_command	= fpm_proc_cmd_set_get},
	{ .name = "get", .do_command	= fpm_proc_cmd_set_get},
	{ .name = "stats", .do_command	= fpm_proc_cmd_stats},
#ifdef CONFIG_BCM_FPM_TOKEN_HIST
	{ .name = "hist", .do_command	= fpm_proc_cmd_hist},
#endif
};

struct proc_cmd_table fpm_command_table = {
	.module_name = "FPM",
	.size = ARRAY_SIZE(command_entries),
	.ops = command_entries
};

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *head_proc_file;
static struct proc_dir_entry *tail_proc_file;
static struct proc_dir_entry *pkt_proc_file;
static struct proc_dir_entry *hp_proc_file;
static struct proc_dir_entry *hpt_proc_file;
static struct proc_dir_entry *hist_proc_file;
static struct proc_dir_entry *cmd_proc_file;
static struct proc_dir_entry *isr_timer_proc_file;

void fpm_proc_exit(void)
{
	if (isr_timer_proc_file) {
		remove_proc_entry(ISR_TIMER_PROC_FILE, proc_dir);
		isr_timer_proc_file = NULL;
	}
	if (cmd_proc_file) {
		remove_proc_entry(CMD_PROC_FILE, proc_dir);
		cmd_proc_file = NULL;
	}
	if (head_proc_file) {
		remove_proc_entry(HEAD_PROC_FILE, proc_dir);
		head_proc_file = NULL;
	}
	if (tail_proc_file) {
		remove_proc_entry(TAIL_PROC_FILE, proc_dir);
		tail_proc_file = NULL;
	}
	if (pkt_proc_file) {
		remove_proc_entry(PKT_PROC_FILE, proc_dir);
		pkt_proc_file = NULL;
	}
	if (hp_proc_file) {
		remove_proc_entry(HP_PROC_FILE, proc_dir);
		hp_proc_file = NULL;
	}
	if (hpt_proc_file) {
		remove_proc_entry(HPT_PROC_FILE, proc_dir);
		hpt_proc_file = NULL;
	}
	if (hist_proc_file) {
		remove_proc_entry(HIST_PROC_FILE, proc_dir);
		hist_proc_file = NULL;
	}
	if (proc_dir) {
		remove_proc_entry(PROC_DIR, NULL);
		proc_dir = NULL;
	}
}

int __init fpm_proc_init(void)
{
	int status = 0;

	proc_dir = proc_mkdir(PROC_DIR, NULL);
	if (!proc_dir) {
		pr_err("Failed to create PROC directory %s.\n",
		       PROC_DIR);
		status = -EIO;
		goto done;
	}
	head_proc_file = proc_create(HEAD_PROC_FILE, S_IRUGO,
				     proc_dir, &head_fops);
	if (!head_proc_file) {
		pr_err("Failed to create %s\n", HEAD_PROC_FILE);
		status = -EIO;
		fpm_proc_exit();
		goto done;
	}
	tail_proc_file = proc_create(TAIL_PROC_FILE, S_IRUGO,
				     proc_dir, &tail_fops);
	if (!tail_proc_file) {
		pr_err("Failed to create %s\n", TAIL_PROC_FILE);
		status = -EIO;
		fpm_proc_exit();
		goto done;
	}
	pkt_proc_file = proc_create(PKT_PROC_FILE, S_IRUGO,
				    proc_dir, &pkt_fops);
	if (!pkt_proc_file) {
		pr_err("Failed to create %s\n", PKT_PROC_FILE);
		status = -EIO;
		fpm_proc_exit();
		goto done;
	}
	hp_proc_file = proc_create(HP_PROC_FILE, S_IRUGO,
				   proc_dir, &hp_fops);
	if (!hp_proc_file) {
		pr_err("Failed to create %s\n", HP_PROC_FILE);
		status = -EIO;
		fpm_proc_exit();
		goto done;
	}
	hpt_proc_file = proc_create(HPT_PROC_FILE, S_IRUGO,
				    proc_dir, &hpt_fops);
	if (!hpt_proc_file) {
		pr_err("Failed to create %s\n", HPT_PROC_FILE);
		status = -EIO;
		fpm_proc_exit();
		goto done;
	}
#ifdef CONFIG_BCM_FPM_TOKEN_HIST
	hist_proc_file = proc_create(HIST_PROC_FILE, S_IRUGO,
				    proc_dir, &hist_fops);
	if (!hist_proc_file) {
		pr_err("Failed to create %s\n", HIST_PROC_FILE);
		status = -EIO;
		fpm_proc_exit();
		goto done;
	}
#endif
	cmd_proc_file = proc_create_cmd(CMD_PROC_FILE, proc_dir,
					&fpm_command_table);
	if (!cmd_proc_file) {
		pr_err("Failed to create %s\n", CMD_PROC_FILE);
		status = -EIO;
		fpm_proc_exit();
		goto done;
	}

	isr_timer_proc_file = proc_create_data(ISR_TIMER_PROC_FILE,
					       S_IRUGO | S_IWUGO, proc_dir,
					       &isr_timer_fops, (void *)fpm);
	if (!isr_timer_proc_file) {
		pr_err("Failed to create %s\n", ISR_TIMER_PROC_FILE);
		status = -EIO;
		fpm_proc_exit();
		goto done;
	}

done:
	return status;
}
