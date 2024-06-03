/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
/****************************************************************************
 * vFlash block IO proc driver
 *
 * Author: Tim Ross <tim.ross@broadcom.com>
 * Author: Venky Selvaraj <venky.selvaraj@broadcom.com>
 *****************************************************************************/
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>
#include "vfbio_priv.h"
#include "vfbio_proc.h"

#define VFBIO_PROC_DIR "driver/vfbio"
#define VFBIO_DH_PROC_FILE	"device_health"
#define EST_MAX 12
#define PRE_EOL_MAX 5

char *est[EST_MAX] = {
	"Not Defined",
	"0% - 10% device lifetime used",
	"10% - 20% device lifetime used",
	"20% - 30% device lifetime used",
	"30% - 40% device lifetime used",
	"40% - 50% device lifetime used",
	"50% - 60% device lifetime used",
	"60% - 70% device lifetime used",
	"70% - 80% device lifetime used",
	"80% - 90% device lifetime used",
	"90% - 100% device lifetime used",
	"Exceeded its maximum estimated device lifetime"
};

char *pre_eol[PRE_EOL_MAX] = {
	"Not Defined",
	"Normal",
	"Warning",
	"Urgent",
	"Read Only"
};

struct dh_res {
	bool end;
	u8 est_a;
	u8 est_b;
	u8 pre_eol;
};

static struct proc_dir_entry *vfbio_proc_dir;
static struct proc_dir_entry *vfbio_dh_proc_file;
static bool vbio_dh_end;
static bool vbio_dh_begin;
static u32 device_cnt;

static inline int vfbio_get_device_health(int tunnel,
			struct dh_res *res, bool begin)
{
	int status = 0;
	rpc_msg msg;

	BUG_ON(!res);

	rpc_msg_init(&msg, RPC_SERVICE_VFBIO, VFBIO_FUNC_DEVICE_HEALTH, 0,
		     0, 0, 0);
	vfbio_msg_set_begin(&msg, begin);
	status = rpc_send_request(tunnel, &msg);
	if (unlikely(status)) {
		pr_err(RED("rpc_send_request failure (%d)\n"),
			status);
		rpc_dump_msg(&msg);
		return status;
	}
	status = vfbio_msg_get_retcode(&msg);
	if (unlikely(status)) {
		pr_err(RED("vfbio msg retcode %d\n"), (s8)status);
		rpc_dump_msg(&msg);
		status = -EIO;
	}
	res->end = vfbio_msg_get_dh_end(&msg);
	res->est_a = vfbio_msg_get_dh_est_a(&msg);
	res->est_b = vfbio_msg_get_dh_est_b(&msg);
	res->pre_eol = vfbio_msg_get_dh_pre_eol(&msg);

	return status;
}

static void *vfbio_dh_proc_start(struct seq_file *seq, loff_t *pos)
{
	struct vfbio_device *vfdev = NULL;

	pr_debug("-->\n");

	if (*pos == 0) {
		device_cnt = 0;
		vbio_dh_begin = true;
		vbio_dh_end = false;
		seq_puts(seq, "---------- Flash devices health report ----------\n");
	} else
		vbio_dh_begin = false;

	if (vbio_dh_end)
		return NULL;

	vfdev = list_first_entry(&vfdevs_list, struct vfbio_device, list);

	pr_debug("<--\n");
	return vfdev;
}

static void vfbio_dh_proc_stop(struct seq_file *seq, void *v)
{
	pr_debug("-->\n");
	pr_debug("<--\n");
}

static void *vfbio_dh_proc_next(struct seq_file *seq, void *v,
				      loff_t *pos)
{
	struct vfbio_device *vfdev = v;

	pr_debug("-->\n");

	(*pos)++;
	if (vbio_dh_end)
		return NULL;

	pr_debug("<--\n");
	return vfdev;
}

static int vfbio_dh_proc_show(struct seq_file *seq, void *v)
{
	struct vfbio_device *vfbio = v;
	struct dh_res res;
	int status = 0;

	pr_debug("-->\n");

	if (!v)
		return -EINVAL;

	memset(&res, 0, sizeof(struct dh_res));
	device_cnt++;
	status = vfbio_get_device_health(vfbio->tunnel, &res, vbio_dh_begin);
	if (unlikely(status))
		return -EIO;
	seq_printf(seq, "Device Number: %u\n", device_cnt);
	seq_printf(seq, "Device Lifetime estimate(EST-A) : Value as per JESD84-B51 standard - %d (%s)\n",
		res.est_a, (res.est_a < EST_MAX) ?
		est[res.est_a] : "Reserved");
	seq_printf(seq, "Device Lifetime estimate(EST-B) : Value as per JESD84-B51 standard - %d (%s)\n",
		res.est_b, (res.est_b < EST_MAX) ?
		est[res.est_b] : "Reserved");
	seq_printf(seq, "Pre EOL Lifetime Estimation(Pre EOL) : Value as per JESD84-B51 standard - %d (%s)\n",
		res.pre_eol, (res.pre_eol < PRE_EOL_MAX) ?
		pre_eol[res.pre_eol] : "Reserved");
	seq_puts(seq, "\n");
	vbio_dh_begin = false;
	vbio_dh_end = res.end;
	pr_debug("<--\n");
	return 0;
}

static const struct seq_operations vfbio_dh_seq_ops = {
	.start = vfbio_dh_proc_start,
	.stop = vfbio_dh_proc_stop,
	.next = vfbio_dh_proc_next,
	.show = vfbio_dh_proc_show,
};

static int vfbio_dh_proc_open(struct inode *inode, struct file *file)
{
	int status;

	pr_debug("-->\n");

	status = seq_open(file, &vfbio_dh_seq_ops);
	if (!status) {
		struct seq_file *sf = file->private_data;

		sf->private = PDE_DATA(inode);
	}

	pr_debug("<--\n");
	return status;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5,15,0))
static const struct proc_ops vfbio_dh_fops = {
	.proc_open = vfbio_dh_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = seq_release,
};
#else
static const struct file_operations vfbio_dh_fops = {
	.owner = THIS_MODULE,
	.open = vfbio_dh_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};
#endif

int __init vfbio_proc_init(void)
{
	int status = 0;

	pr_debug("-->\n");
	vfbio_proc_dir = proc_mkdir(VFBIO_PROC_DIR, NULL);
	if (!vfbio_proc_dir) {
		pr_err("Failed to create PROC directory %s.\n",
		       VFBIO_PROC_DIR);
		status = -EIO;
		goto done;
	}
	vfbio_dh_proc_file = proc_create(VFBIO_DH_PROC_FILE, 0444,
					 vfbio_proc_dir, &vfbio_dh_fops);
	if (!vfbio_dh_proc_file) {
		pr_err("Failed to create %s\n", VFBIO_DH_PROC_FILE);
		status = -EIO;
		vfbio_proc_exit();
		goto done;
	}

done:
	pr_debug("<--\n");
	return status;
}

void vfbio_proc_exit(void)
{
	pr_debug("-->\n");
	if (vfbio_dh_proc_file) {
		remove_proc_entry(VFBIO_DH_PROC_FILE, vfbio_proc_dir);
		vfbio_dh_proc_file = NULL;
	}
	if (vfbio_proc_dir) {
		remove_proc_entry(VFBIO_PROC_DIR, NULL);
		vfbio_proc_dir = NULL;
	}
	pr_debug("<--\n");
}
