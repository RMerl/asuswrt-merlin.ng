// SPDX-License-Identifier: GPL-2.0
/*
 * Broadcom STB common PM routines
 *
 * Copyright © 2014-2018 Broadcom
 */

#define pr_fmt(fmt) "brcmstb-pm: " fmt

#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/sort.h>
#include <linux/slab.h>
#include <linux/sizes.h>
#include <linux/module.h>
#include <linux/brcmstb/brcmstb.h>
#include <linux/brcmstb/memory_api.h>
#include <linux/soc/brcmstb/brcmstb.h>
#include <linux/seq_file.h>

#ifdef CONFIG_BRCMSTB_PM_DEBUG
#include <linux/debugfs.h>
#endif

#include "pm-common.h"

#define BRCMSTB_PM_DEBUG_NAME	"brcmstb-pm"

/* Capped for performance reasons */
#define MAX_HASH_SIZE			SZ_256M
/* Max per bank, to keep some fairness */
#define MAX_HASH_SIZE_BANK		SZ_64M

#define memreg_dbg(fmt, ...) \
	if (memreg_debug) pr_info(fmt, ##__VA_ARGS__)

static int memreg_debug;
module_param(memreg_debug, int, S_IRUGO);
MODULE_PARM_DESC(memreg_debug, "Enable debugging for BRCM memory regions");

static int brcm_pm_debug_init(void);

/* Must be visible to pm-arm.c */
struct dma_region exclusions[MAX_EXCLUDE];
EXPORT_SYMBOL_GPL(exclusions);
struct dma_region regions[MAX_REGION];
EXPORT_SYMBOL_GPL(regions);
int num_regions;
EXPORT_SYMBOL_GPL(num_regions);
int num_exclusions;
EXPORT_SYMBOL_GPL(num_exclusions);
static struct brcmstb_memory bm;

struct procfs_data {
	struct dma_region *region;
	unsigned len;
};

static int __clear_region(struct dma_region arr[], int max)
{
	int i, j;
	bool found_non_persistent = false;

	for (i = 0, j = 0; i < max; i++) {
		if (!arr[i].persistent) {
			/*
			 * Found a non-persistent entry. Remember this, so we
			 * can fill the freed up entry with a persistent entry
			 * should there be one.
			 */
			found_non_persistent = true;
		} else if (found_non_persistent) {
			/*
			 * We found a persistent entry, but at least one non-
			 * persistent entry preceeded it. Copy our entry to the
			 * first available entry in the array.
			 */
			arr[j++] = arr[i];
		} else {
			/*
			 * So far we've only found persistent entries. We need
			 * to keep all of them. Just increment the "empty"
			 * counter.
			 */
			j++;
			continue;
		}
		memset(&arr[i], 0, sizeof(arr[i]));
	}

	return j;
}

static int dma_region_compare(const void *a, const void *b)
{
	struct dma_region *reg_a = (struct dma_region *)a;
	struct dma_region *reg_b = (struct dma_region *)b;

	if (reg_a->addr < reg_b->addr)
		return -1;
	if (reg_a->addr > reg_b->addr)
		return 1;
	return 0;
}

static int __sorted_insert(struct dma_region arr[], int *cur, int max,
			   phys_addr_t addr, size_t len, bool persistent)
{
	unsigned int i;
	int end = *cur;

	for (i = 0; i < end; i++) {
		if (arr[i].addr == addr && arr[i].len == len &&
		    arr[i].persistent == persistent)
			return 0;
	}

	if (end >= max)
		return -ENOSPC;

	arr[end].addr = addr;
	arr[end].len = len;
	arr[end].persistent = persistent;
	end++;
	*cur = end;

	/* Not pretty to insert first and sort second, but it works for now. */
	sort(arr, end, sizeof(arr[0]), &dma_region_compare, NULL);

	return 0;
}

int __pm_mem_exclude(phys_addr_t addr, size_t len, bool persistent)
{
	return __sorted_insert(exclusions, &num_exclusions, MAX_EXCLUDE, addr,
			      len, persistent);
}

static inline int region_collision(struct dma_region *reg1,
				   struct dma_region *reg2)
{
	return (reg1->addr + reg1->len > reg2->addr) &&
	       (reg2->addr + reg2->len > reg1->addr);
}

/**
 * Check if @regions[0] collides with regions in @exceptions, and modify
 * regions[0..(max-1)] to ensure that they they exclude any area in @exceptions
 *
 * Note that the regions in @exceptions must be sorted into ascending order
 * prior to calling this function
 *
 * Returns the number of @regions used
 *
 * e.g., if @regions[0] and @exceptions do not overlap, return 1 and do nothing
 *       if @exceptions contains two ranges and both are entirely contained
 *          within @regions[0], split @regions[0] into @regions[0],
 *          @regions[1], and @regions[2], and return 3
 */
static int region_handle_collisions(struct dma_region *regions, int max,
				struct dma_region *exceptions, int num_except)
{
	int i;
	struct dma_region *reg = &regions[0];
	int reg_count = 1;

	/*
	 * Since the list of regions is ordered in ascending order we need only
	 * to compare the last entry in regions against each exception region
	 */
	for (i = 0; i < num_except; i++) {
		struct dma_region *except = &exceptions[i];
		dma_addr_t start = reg->addr;
		dma_addr_t end = reg->addr + reg->len;

		if (!region_collision(reg, except))
			/* No collision */
			continue;

		if (start < except->addr && end > except->addr + except->len) {
			reg->len = except->addr - start;
			if (reg_count < max) {
				/* Split in 2 */
				reg++;
				reg_count++;
				reg->addr = except->addr + except->len;
				reg->len = end - reg->addr;
			} else {
				pr_warn("Not enough space to split region\n");
				break;
			}
		} else if (start < except->addr) {
			/* Overlap at right edge; truncate end of 'reg' */
			reg->len = except->addr - start;
		} else if (end > except->addr + except->len) {
			/* Overlap at left edge; truncate beginning of 'reg' */
			reg->addr = except->addr + except->len;
			reg->len = end - reg->addr;
		} else {
			/*
			 * 'reg' is entirely contained within 'except'?  This
			 * should not happen, but trim to zero-length just in
			 * case
			 */
			reg->len = 0;
			reg_count--;
			break;
		}
	}

	return reg_count;
}

static int setup_default_regions(void)
{
	struct brcmstb_range *range;
	int idx = 0, memc;
	u64 total = 0;

	memreg_dbg("%s begin\n", __func__);
	/*
	 * Hash up to MAX_HASH_SIZE_BANK from each memory bank, with a
	 * total limit of MAX_HASH_SIZE. Account for collisions with the
	 * 'exclude' regions.
	 */
	for_each_range_of_memc(bm, memc, range) {
		phys_addr_t block_start = range->addr;
		u64 size_limit = range->size;

		struct dma_region *reg = &regions[idx];
		int i, count;
		u64 bank_total = 0;

		memreg_dbg("  block_start=%llx, len=%llu\n", block_start,
			size_limit);
		reg->addr = block_start;
		reg->len = size_limit;

		/*
		 * Check for collisions with the excluded regions.  'reg' may be
		 * split into 0 to (num_exclude + 1) segments, so account
		 * accordingly
		 */
		count = 1;

		/*
		 * Add region length(s) to total. Cap at MAX_HASH_SIZE_BANK
		 * per bank and MAX_HASH_SIZE total.
		 */
		for (i = 0; i < count; i++) {
			/* Don't create 0-sized regions */
			if (total >= MAX_HASH_SIZE) {
				memreg_dbg("    %d) MAX_HASH_SIZE reached (%lld)\n",
					i, total);
				break;
			}
			if (bank_total >= MAX_HASH_SIZE_BANK) {
				memreg_dbg("    %d) MAX_HASH_SIZE_BANK reached (%lld)\n",
					i, bank_total);
				break;
			}
			if (total + reg[i].len > MAX_HASH_SIZE) {
				memreg_dbg("    %d) MAX_HASH_SIZE exceeded; total=%llu, len=%llu\n",
					i, total, MAX_HASH_SIZE - total);
				reg[i].len = MAX_HASH_SIZE - total;
			}
			if (bank_total + reg[i].len > MAX_HASH_SIZE_BANK) {
				memreg_dbg("    %d) MAX_HASH_SIZE_BANK exceeded; bank_total=%llu, len=%llu\n",
					i, bank_total, MAX_HASH_SIZE_BANK - bank_total);
				reg[i].len = MAX_HASH_SIZE_BANK - bank_total;
			}
			total += reg[i].len;
			bank_total += reg[i].len;
		}

		idx += i;

		/* Apply total cap */
		if (total >= MAX_HASH_SIZE) {
			memreg_dbg("    %d) MAX_HASH_SIZE reached (%lld); exiting\n",
				i, total);
			break;
		}
	}

	/* Extra verbose debug */
	if (memreg_debug > 1) {
		int i;

		for (i = 0; i < MAX_REGION; i++) {
			struct dma_region *entry = &regions[i];
			unsigned long addr = entry->addr;
			unsigned long len = entry->len;
			unsigned long end = (addr > 0 || len > 0)
				? addr + len - 1 : 0;

			pr_info("[%p] %3d\t0x%08lx\t%12lu\t0x%08lx%s\n",
				entry, i, addr, len, end,
				entry->persistent ? "\t*" : "");
		}
	}
	memreg_dbg("%s complete with %d region(s)\n", __func__, idx);

	return idx;
}

/* Initialize the DMA region list and return the number of regions */
int configure_main_hash(struct dma_region *combined, int max,
			struct dma_region *exclude, int num_exclude)
{
	int count, idx = 0;

	memreg_dbg("%s begin\n", __func__);

	memset(combined, 0, max);
	memcpy(combined, &regions, sizeof(regions));
	/*
	 * Now sort the excluded regions in ascending order. This makes things
	 * easier when we come to adding the regions since we avoid having to
	 * add entries in the middle of the region list
	 */
	sort(exclude, num_exclude, sizeof(exclude[0]), &dma_region_compare,
			NULL);

	count = region_handle_collisions(&combined[0], max - idx,
			exclude, num_exclude);

	memreg_dbg("%s: idx=%d, count=%d\n", __func__, idx, count);

	/* Extra verbose debug */
	if (memreg_debug > 1) {
		int i;

		for (i = 0; i < MAX_REGION; i++) {
			struct dma_region *entry = &combined[i];
			unsigned long addr = entry->addr;
			unsigned long len = entry->len;
			unsigned long end = (addr > 0 || len > 0) ? addr + len - 1 : 0;

			pr_info("%s: [%p] %3d\t0x%08lx\t%12lu\t0x%08lx%s\n",
				__func__, entry, i, addr, len,
			end, entry->persistent ? "\t*" : "");
		}
	}
	memreg_dbg("%s complete with %d region(s)\n", __func__, count);

	return count;
}
EXPORT_SYMBOL(configure_main_hash);

int brcmstb_pm_mem_exclude(phys_addr_t addr, size_t len)
{
	return __pm_mem_exclude(addr, len, false);
}
EXPORT_SYMBOL(brcmstb_pm_mem_exclude);

int brcmstb_pm_mem_region(phys_addr_t addr, size_t len)
{
	return __sorted_insert(regions, &num_regions, MAX_EXCLUDE, addr, len,
			      false);
}
EXPORT_SYMBOL(brcmstb_pm_mem_region);

static int brcm_pm_proc_show(struct seq_file *s, void *data)
{
	int i;
	struct procfs_data *procfs_data = s->private;

	if (!procfs_data) {
		seq_puts(s, "--- No region pointer ---\n");
		return 0;
	}
	if (procfs_data->len == 0) {
		seq_puts(s, "--- Nothing to display ---\n");
		return 0;
	}
	if (!procfs_data->region) {
		seq_printf(s, "--- Pointer is NULL, but length is %u ---\n",
			procfs_data->len);
		return 0;
	}

	for (i = 0; i < procfs_data->len; i++) {
		struct dma_region *entry = &procfs_data->region[i];
		unsigned long addr = entry->addr;
		unsigned long len = entry->len;
		unsigned long end = (addr > 0 || len > 0) ? addr + len - 1 : 0;

		seq_printf(s, "%3d\t0x%08lx\t%12lu\t0x%08lx%s\n", i, addr, len,
			end, entry->persistent ? "\t*" : "");
	}
	return 0;
}

static ssize_t brcm_pm_seq_write(struct file *file, const char __user *buf,
	size_t size, loff_t *ppos)
{
	unsigned long start_addr, len;
	int ret;
	char str[128];
	char *len_ptr;
	struct seq_file *s = file->private_data;
	struct procfs_data *procfs_data = s->private;
	bool is_exclusion;

	if (!procfs_data)
		return -ENOMEM;

	if (size >= sizeof(str))
		return -E2BIG;

	is_exclusion = (procfs_data->region == exclusions);

	memset(str, 0, sizeof(str));
	ret = copy_from_user(str, buf, size);
	if (ret)
		return ret;

	/* Strip trailing newline */
	len_ptr = str + strlen(str) - 1;
	while (*len_ptr == '\r' || *len_ptr == '\n')
		*len_ptr-- = '\0';

	/* Special command "clear" empties the exclusions or regions list. */
	if (strcmp(str, "clear") == 0) {
		struct dma_region *region = procfs_data->region;
		int max, ret;

		max = is_exclusion ? num_exclusions : num_regions;
		ret = __clear_region(region, max);

		if (is_exclusion)
			num_exclusions = ret;
		else
			num_regions = ret;

		return size;
	}

	/*
	 * We expect userland input to be in the format
	 *     <start-address> <length>
	 * where start-address and length are separated by one or more spaces.
	 * Both must be valid numbers. We do accept decimal, hexadecimal and
	 * octal numbers.
	 */
	len_ptr = strchr(str, ' ');
	if (!len_ptr)
		return -EINVAL;
	*len_ptr = '\0';
	do {
		len_ptr++;
	} while (*len_ptr == ' ');

	if (kstrtoul(str, 0, &start_addr) != 0)
		return -EINVAL;
	if (kstrtoul(len_ptr, 0, &len) != 0)
		return -EINVAL;

	if (is_exclusion)
		ret = brcmstb_pm_mem_exclude(start_addr, len);
	else
		ret = brcmstb_pm_mem_region(start_addr, len);

	return ret < 0 ? ret : size;
}

static int brcm_pm_proc_open(struct inode *inode, struct file *file)
{
	void *data;

	/*
	 * Using debugfs, inode->i_private contains our private data. For
	 * procfs, our private data resides in PDE_DATA(inode) instead.
	 */
	if (inode->i_private)
		data = inode->i_private;
	else
		data = PDE_DATA(inode);

	return single_open(file, brcm_pm_proc_show, data);
}

static int brcm_pm_driver_init(void)
{
	int ret;

	ret = brcmstb_memory_get(&bm);
	if (ret)
		return ret;

	return setup_default_regions();
}

static const struct file_operations brcm_pm_debug_fops = {
	.open		= brcm_pm_proc_open,
	.read		= seq_read,
	.write		= brcm_pm_seq_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct proc_ops brcm_pm_proc_ops = {
	.proc_open	= brcm_pm_proc_open,
	.proc_read	= seq_read,
	.proc_write	= brcm_pm_seq_write,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
};

static int __init proc_pm_init(void)
{
	struct procfs_data *exclusion_data, *region_data;
	struct proc_dir_entry *brcmstb_root;
	const int perm = S_IRUSR | S_IWUSR;
	int ret = 0;

	brcmstb_root = proc_mkdir("brcmstb", NULL);
	if (!brcmstb_root)
		return 0;

	/*
	 * This driver has no "exit" function, so we don't worry about freeing
	 * these memory areas if setup succeeds.
	 */
	exclusion_data = kmalloc(sizeof(*exclusion_data), GFP_KERNEL);
	if (!exclusion_data)
		return -ENOMEM;
	region_data = kmalloc(sizeof(*region_data), GFP_KERNEL);
	if (!region_data) {
		kfree(exclusion_data);
		return -ENOMEM;
	}

	exclusion_data->region = exclusions;
	exclusion_data->len = ARRAY_SIZE(exclusions);
	region_data->region = regions;
	region_data->len = ARRAY_SIZE(regions);

	memreg_dbg("%s: exclusion_data=%p, exclusions=%p\n", __func__,
		exclusion_data, exclusions);
	memreg_dbg("%s: region_data=%p, regions=%p\n", __func__, region_data,
		regions);

	/* Extra verbose debug */
	if (memreg_debug > 1) {
		int i;

		for (i = 0; i < MAX_REGION; i++) {
			pr_info("%s: regions[%d]=%p/%p\n", __func__, i,
				regions + i, &regions[i]);
		}
	}

	if (!proc_create_data("regions", perm, brcmstb_root, &brcm_pm_proc_ops,
			      region_data))
		goto err_out;
	if (!proc_create_data("exclusions", perm, brcmstb_root,
			      &brcm_pm_proc_ops, exclusion_data))
		goto err_out;

	if (brcm_pm_driver_init() < 0)
		goto err_out;

#if !IS_BUILTIN(CONFIG_BRCMSTB_PM)
	ret = brcm_pm_debug_init();
#endif
	if (ret == 0)
		return 0;

err_out:
	proc_remove(brcmstb_root); /* cleans up recursively */
	kfree(exclusion_data);
	kfree(region_data);

	return -ENOENT;
}
module_init(proc_pm_init);

#ifdef CONFIG_BRCMSTB_PM_DEBUG

static int brcm_pm_debug_init(void)
{
	struct dentry *dir;
	struct procfs_data *exclusion_data, *region_data;

	dir = debugfs_create_dir(BRCMSTB_PM_DEBUG_NAME, NULL);
	if (IS_ERR_OR_NULL(dir))
		return IS_ERR(dir) ? PTR_ERR(dir) : -ENOENT;

	/*
	 * This driver has no "exit" function, so we don't worry about freeing
	 * these memory areas if setup succeeds.
	 */
	exclusion_data = kmalloc(sizeof(*exclusion_data), GFP_KERNEL);
	if (!exclusion_data)
		return -ENOMEM;
	region_data = kmalloc(sizeof(*region_data), GFP_KERNEL);
	if (!region_data) {
		kfree(exclusion_data);
		return -ENOMEM;
	}

	memreg_dbg("%s: exclusion_data=%p, exclusions=%p\n", __func__,
		exclusion_data, exclusions);
	memreg_dbg("%s: region_data=%p, regions=%p\n", __func__, region_data,
		regions);

	exclusion_data->region = exclusions;
	exclusion_data->len = ARRAY_SIZE(exclusions);
	region_data->region = regions;
	region_data->len = ARRAY_SIZE(regions);

	debugfs_create_file("exclusions", S_IFREG | S_IRUGO | S_IWUSR, dir,
		exclusion_data, &brcm_pm_debug_fops);
	debugfs_create_file("regions", S_IFREG | S_IRUGO | S_IWUSR, dir,
		region_data, &brcm_pm_debug_fops);

	return 0;
}

#if IS_BUILTIN(CONFIG_BRCMSTB_PM)
fs_initcall(brcm_pm_debug_init);
#endif
#else
static int __maybe_unused brcm_pm_debug_init(void)
{
	return 0;
}
#endif /* CONFIG_BRCMSTB_PM_DEBUG */

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom STB PM common routines");
MODULE_LICENSE("GPL v2");
