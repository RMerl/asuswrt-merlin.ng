/*
 *  Copyright IBM Corp. 2012
 *
 *  Author(s):
 *    Jan Glauber <jang@linux.vnet.ibm.com>
 */

#define KMSG_COMPONENT "zpci"
#define pr_fmt(fmt) KMSG_COMPONENT ": " fmt

#include <linux/kernel.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include <linux/export.h>
#include <linux/pci.h>
#include <asm/debug.h>

#include <asm/pci_dma.h>

static struct dentry *debugfs_root;
debug_info_t *pci_debug_msg_id;
EXPORT_SYMBOL_GPL(pci_debug_msg_id);
debug_info_t *pci_debug_err_id;
EXPORT_SYMBOL_GPL(pci_debug_err_id);

static char *pci_perf_names[] = {
	/* hardware counters */
	"Load operations",
	"Store operations",
	"Store block operations",
	"Refresh operations",
	"DMA read bytes",
	"DMA write bytes",
};

static char *pci_sw_names[] = {
	"Allocated pages",
	"Mapped pages",
	"Unmapped pages",
};

static void pci_sw_counter_show(struct seq_file *m)
{
	struct zpci_dev *zdev = m->private;
	atomic64_t *counter = &zdev->allocated_pages;
	int i;

	for (i = 0; i < ARRAY_SIZE(pci_sw_names); i++, counter++)
		seq_printf(m, "%26s:\t%llu\n", pci_sw_names[i],
			   atomic64_read(counter));
}

static int pci_perf_show(struct seq_file *m, void *v)
{
	struct zpci_dev *zdev = m->private;
	u64 *stat;
	int i;

	if (!zdev)
		return 0;

	mutex_lock(&zdev->lock);
	if (!zdev->fmb) {
		mutex_unlock(&zdev->lock);
		seq_puts(m, "FMB statistics disabled\n");
		return 0;
	}

	/* header */
	seq_printf(m, "FMB @ %p\n", zdev->fmb);
	seq_printf(m, "Update interval: %u ms\n", zdev->fmb_update);
	seq_printf(m, "Samples: %u\n", zdev->fmb->samples);
	seq_printf(m, "Last update TOD: %Lx\n", zdev->fmb->last_update);

	/* hardware counters */
	stat = (u64 *) &zdev->fmb->ld_ops;
	for (i = 0; i < 4; i++)
		seq_printf(m, "%26s:\t%llu\n",
			   pci_perf_names[i], *(stat + i));
	if (zdev->fmb->dma_valid)
		for (i = 4; i < 6; i++)
			seq_printf(m, "%26s:\t%llu\n",
				   pci_perf_names[i], *(stat + i));

	pci_sw_counter_show(m);
	mutex_unlock(&zdev->lock);
	return 0;
}

static ssize_t pci_perf_seq_write(struct file *file, const char __user *ubuf,
				  size_t count, loff_t *off)
{
	struct zpci_dev *zdev = ((struct seq_file *) file->private_data)->private;
	unsigned long val;
	int rc;

	if (!zdev)
		return 0;

	rc = kstrtoul_from_user(ubuf, count, 10, &val);
	if (rc)
		return rc;

	mutex_lock(&zdev->lock);
	switch (val) {
	case 0:
		rc = zpci_fmb_disable_device(zdev);
		break;
	case 1:
		rc = zpci_fmb_enable_device(zdev);
		break;
	}
	mutex_unlock(&zdev->lock);
	return rc ? rc : count;
}

static int pci_perf_seq_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, pci_perf_show,
			   file_inode(filp)->i_private);
}

static const struct file_operations debugfs_pci_perf_fops = {
	.open	 = pci_perf_seq_open,
	.read	 = seq_read,
	.write	 = pci_perf_seq_write,
	.llseek  = seq_lseek,
	.release = single_release,
};

void zpci_debug_init_device(struct zpci_dev *zdev)
{
	zdev->debugfs_dev = debugfs_create_dir(dev_name(&zdev->pdev->dev),
					       debugfs_root);
	if (IS_ERR(zdev->debugfs_dev))
		zdev->debugfs_dev = NULL;

	zdev->debugfs_perf = debugfs_create_file("statistics",
				S_IFREG | S_IRUGO | S_IWUSR,
				zdev->debugfs_dev, zdev,
				&debugfs_pci_perf_fops);
	if (IS_ERR(zdev->debugfs_perf))
		zdev->debugfs_perf = NULL;
}

void zpci_debug_exit_device(struct zpci_dev *zdev)
{
	debugfs_remove(zdev->debugfs_perf);
	debugfs_remove(zdev->debugfs_dev);
}

int __init zpci_debug_init(void)
{
	/* event trace buffer */
	pci_debug_msg_id = debug_register("pci_msg", 8, 1, 8 * sizeof(long));
	if (!pci_debug_msg_id)
		return -EINVAL;
	debug_register_view(pci_debug_msg_id, &debug_sprintf_view);
	debug_set_level(pci_debug_msg_id, 3);

	/* error log */
	pci_debug_err_id = debug_register("pci_error", 2, 1, 16);
	if (!pci_debug_err_id)
		return -EINVAL;
	debug_register_view(pci_debug_err_id, &debug_hex_ascii_view);
	debug_set_level(pci_debug_err_id, 6);

	debugfs_root = debugfs_create_dir("pci", NULL);
	return 0;
}

void zpci_debug_exit(void)
{
	debug_unregister(pci_debug_msg_id);
	debug_unregister(pci_debug_err_id);
	debugfs_remove(debugfs_root);
}
