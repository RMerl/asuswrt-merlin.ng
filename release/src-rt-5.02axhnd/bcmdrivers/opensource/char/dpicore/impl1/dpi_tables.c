/*
 * <:copyright-BRCM:2017:DUAL/GPL:standard 
 * 
 *    Copyright (c) 2017 Broadcom 
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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/hashtable.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/etherdevice.h>
#include <net/netfilter/nf_conntrack.h>

#include <linux/dpi.h>

#include "dpi_local.h"

#define APP_TABLE_BITS		6
#define DEV_TABLE_BITS		4
#define APPINST_TABLE_BITS	6
#define URL_TABLE_BITS		6
#define URL_MAX_HASH_CHARS	20

#define APP_TABLE_MAX_SIZE	10000
#define DEV_TABLE_MAX_SIZE	1024
#define APPINST_TABLE_MAX_SIZE	1000000
#define URL_TABLE_MAX_SIZE	10000

/* ----- variables ----- */
#define DEFINE_DPI_TABLE(name, bits) \
	struct name { \
		DECLARE_HASHTABLE(list, bits); \
		struct kmem_cache *cache; \
		unsigned long size; \
		unsigned long max_size; \
		spinlock_t lock; \
	} name
DEFINE_DPI_TABLE(app_table, APP_TABLE_BITS);
DEFINE_DPI_TABLE(dev_table, DEV_TABLE_BITS);
DEFINE_DPI_TABLE(appinst_table, APPINST_TABLE_BITS);
DEFINE_DPI_TABLE(url_table, URL_TABLE_BITS);

/* ----- public functions ----- */
struct dpi_dev *dpi_dev_find_or_alloc(uint8_t *mac)
{
	struct hlist_head *h;
	struct dpi_dev *entry;
	uint32_t val = *((uint32_t *) &mac[0]) ^ *((uint16_t *) &mac[4]);

	if (is_zero_ether_addr(mac))
		return NULL;

	spin_lock_bh(&dev_table.lock);

	/* search for existing entry */
	h = &dev_table.list[hash_min(val, DEV_TABLE_BITS)];
	hlist_for_each_entry(entry, h, node) {
		if (ether_addr_equal(entry->mac, mac))
			goto out;
	}

	/* allocate new entry if not found */
	if (dev_table.size >= dev_table.max_size) {
		pr_err("dev table full (%lu entries)\n", dev_table.size);
		entry = NULL;
		goto out;
	}
	entry = kmem_cache_zalloc(dev_table.cache, GFP_ATOMIC);
	if (!entry) {
		pr_err("unable to allocate entry\n");
		goto out;
	}
	memcpy(entry->mac, mac, sizeof(entry->mac));

	/* add entry to hlist */
	pr_debug("new dev<%pM>\n", entry->mac);
	INIT_HLIST_NODE(&entry->node);
	hlist_add_head(&entry->node, h);
	dev_table.size++;
	dpi_stats.dev_count++;

out:
	spin_unlock_bh(&dev_table.lock);
	return entry;
}

struct dpi_app *dpi_app_find_or_alloc(uint32_t app_id)
{
	struct hlist_head *h;
	struct dpi_app *entry;

	spin_lock_bh(&app_table.lock);

	/* search for existing entry */
	h = &app_table.list[hash_min(app_id, APP_TABLE_BITS)];
	hlist_for_each_entry(entry, h, node) {
		if (entry->app_id == app_id)
			goto out;
	}

	/* allocate new entry if not found */
	if (app_table.size >= app_table.max_size) {
		pr_info("app table full (%lu entries)\n", app_table.size);
		entry = NULL;
		goto out;
	}
	entry = kmem_cache_zalloc(app_table.cache, GFP_ATOMIC);
	if (!entry) {
		pr_err("unable to allocate entry\n");
		goto out;
	}
	entry->app_id = app_id;

	/* add entry to hlist */
	pr_debug("new app<%08x>\n", entry->app_id);
	INIT_HLIST_NODE(&entry->node);
	hlist_add_head(&entry->node, h);
	app_table.size++;
	dpi_stats.app_count++;

out:
	spin_unlock_bh(&app_table.lock);
	return entry;
}

struct dpi_appinst *dpi_appinst_find_or_alloc(struct dpi_app *app,
					      struct dpi_dev *dev)
{
	struct hlist_head *h;
	struct dpi_appinst *entry;
	uint32_t val = app->app_id ^
			*((uint32_t *) &dev->mac[0]) ^
			*((uint16_t *) &dev->mac[4]);

	spin_lock_bh(&appinst_table.lock);

	/* search for existing entry */
	h = &appinst_table.list[hash_min(val, APPINST_TABLE_BITS)];
	hlist_for_each_entry(entry, h, node) {
		if (entry->app == app && entry->dev == dev)
			goto out;
	}

	/* allocate new entry if not found */
	if (appinst_table.size >= appinst_table.max_size) {
		pr_info("appinst table full (%lu entries)\n",
			appinst_table.size);
		entry = NULL;
		goto out;
	}
	entry = kmem_cache_zalloc(appinst_table.cache, GFP_ATOMIC);
	if (!entry) {
		pr_err("unable to allocate entry\n");
		goto out;
	}
	entry->app = app;
	entry->dev = dev;

	/* add entry to hlist */
	pr_debug("new appinst [app<%08x> dev<%pM>]\n", app->app_id, dev->mac);
	INIT_HLIST_NODE(&entry->node);
	hlist_add_head(&entry->node, h);
	appinst_table.size++;
	dpi_stats.appinst_count++;

out:
	spin_unlock_bh(&appinst_table.lock);
	return entry;
}

static uint32_t __hash_string(char *string, int len, int bits)
{
	uint32_t hash = 0x61C88647;
	int i;

	for (i = 0; i < URL_MAX_HASH_CHARS && *string; i++, string++)
		hash = hash * (*string);

	return hash >> (32 - bits);
}

struct dpi_url *dpi_url_find_or_alloc(char *hostname, int len)
{
	struct hlist_head *h;
	struct dpi_url *entry;
	uint32_t hash = __hash_string(hostname, len, URL_TABLE_BITS);

	spin_lock_bh(&url_table.lock);

	/* search for existing entry */
	h = &url_table.list[hash];
	hlist_for_each_entry(entry, h, node) {
		if (memcmp(entry->hostname, hostname, entry->len) == 0)
			goto out;
	}

	/* allocate new entry if not found */
	if (url_table.size >= url_table.max_size) {
		pr_info("url table full (%lu entries)\n", url_table.size);
		entry = NULL;
		goto out;
	}
	entry = kmem_cache_zalloc(url_table.cache, GFP_ATOMIC);
	if (!entry) {
		pr_err("unable to allocate entry\n");
		goto out;
	}

	entry->len = min(len, DPI_URLINFO_MAX_HOST_LEN - 1);
	if (entry->len != len)
		pr_debug("truncating long URL<%s>\n", hostname);
	/* copy string and explicitly add null terminator */
	memcpy(entry->hostname, hostname, entry->len);
	entry->hostname[entry->len] = '\0';

	/* add entry to hlist */
	pr_debug("new url<%s>\n", entry->hostname);
	INIT_HLIST_NODE(&entry->node);
	hlist_add_head(&entry->node, h);
	url_table.size++;
	dpi_stats.url_count++;

out:
	spin_unlock_bh(&url_table.lock);
	return entry;
}

void dpi_reset_stats(void)
{
	struct dpi_appinst *appinst;
	struct dpi_dev *dev;
	int i;

	spin_lock_bh(&appinst_table.lock);
	hash_for_each(appinst_table.list, i, appinst, node) {
		memset(&appinst->us, 0, sizeof(appinst->us));
		memset(&appinst->ds, 0, sizeof(appinst->ds));
	}
	spin_unlock_bh(&appinst_table.lock);

	spin_lock_bh(&dev_table.lock);
	hash_for_each(dev_table.list, i, dev, node) {
		memset(&dev->us, 0, sizeof(dev->us));
		memset(&dev->ds, 0, sizeof(dev->ds));
	}
	spin_unlock_bh(&dev_table.lock);
}

/* ----- local functions ----- */
#define DEFINE_DPI_HLIST_SEQ(name, table, entry_type, fmt, show_fun) \
	static void *dpi_##name##_seq_start(struct seq_file *s, loff_t *pos) \
	{ \
		loff_t *spos = s->private; \
		*spos = *pos - 1; \
		if (*pos == 0) \
			return SEQ_START_TOKEN; \
		if (*spos >= HASH_SIZE(table.list)) \
			return NULL; \
		return spos; \
	} \
	static void *dpi_##name##_seq_next(struct seq_file *s, void *v, \
					    loff_t *pos) \
	{ \
		loff_t *spos = s->private; \
		(*pos)++; \
		(*spos)++; \
		if (*spos >= HASH_SIZE(table.list)) \
			return NULL; \
		return spos; \
	} \
	static void dpi_##name##_seq_stop(struct seq_file *s, void *v) \
	{ \
	} \
	static int dpi_##name##_seq_show(struct seq_file *s, void *v) \
	{ \
		loff_t *spos = s->private; \
		struct hlist_head *h = &table.list[*spos]; \
		entry_type *entry; \
		\
		if (v == SEQ_START_TOKEN) \
			return seq_printf(s, fmt "\n"); \
		\
		/* print each entry in the bucket */ \
		hlist_for_each_entry(entry, h, node) \
			if (!show_fun(s, entry)) \
				seq_printf(s, "\n"); \
		return 0; \
	} \
	static const struct seq_operations dpi_##name##_seq_ops = { \
		.start = dpi_##name##_seq_start, \
		.next  = dpi_##name##_seq_next, \
		.stop  = dpi_##name##_seq_stop, \
		.show  = dpi_##name##_seq_show \
	}; \
	static int dpi_##name##_open(struct inode *inode, struct file *file) \
	{ \
		return seq_open_private(file, &dpi_##name##_seq_ops, \
					sizeof(loff_t)); \
	} \
	static const struct file_operations dpi_##name##_fops = { \
		.open    = dpi_##name##_open, \
		.read    = seq_read, \
		.llseek  = seq_lseek, \
		.release = seq_release_private, \
	}

static int dpi_app_entry_show(struct seq_file *s, struct dpi_app *entry)
{
	/* don't print behaviour in app_id */
	seq_printf(s, "%u", entry->app_id);
	return 0;
}
DEFINE_DPI_HLIST_SEQ(app, app_table, struct dpi_app, "app_id",
		     dpi_app_entry_show);

static int dpi_dev_entry_show(struct seq_file *s, struct dpi_dev *entry)
{
	seq_printf(s, "%pM %u %u %u %u %u",
		   entry->mac,
		   entry->vendor_id,
		   entry->os_id,
		   entry->class_id,
		   entry->type_id,
		   entry->dev_id);
	return 0;
}
static int dpi_dev_entry_stats_show(struct seq_file *s, struct dpi_dev *entry)
{
	dpi_dev_entry_show(s, entry);

	seq_printf(s, " %llu %llu %llu %llu",
		   entry->us.pkts,
		   entry->us.bytes,
		   entry->ds.pkts,
		   entry->ds.bytes);
	return 0;
}
DEFINE_DPI_HLIST_SEQ(dev, dev_table, struct dpi_dev,
		     "mac vendor os class type dev up_pkt up_byte down_pkt down_byte",
		     dpi_dev_entry_stats_show);

static int dpi_appinst_entry_show(struct seq_file *s,
				  struct dpi_appinst *entry)
{
	BUG_ON(!entry->app);
	BUG_ON(!entry->dev);

	dpi_app_entry_show(s, entry->app);
	seq_puts(s, " ");
	dpi_dev_entry_show(s, entry->dev);

	seq_printf(s, " %llu %llu %llu %llu",
		   entry->us.pkts,
		   entry->us.bytes,
		   entry->ds.pkts,
		   entry->ds.bytes);
	return 0;
}
DEFINE_DPI_HLIST_SEQ(appinst, appinst_table, struct dpi_appinst,
		     "app_id mac vendor os class type dev up_pkt up_byte down_pkt down_byte",
		     dpi_appinst_entry_show);


static int dpi_url_entry_show(struct seq_file *s, struct dpi_url *entry)
{
	seq_printf(s, "%s", entry->hostname);
	return 0;
}
DEFINE_DPI_HLIST_SEQ(url, url_table, struct dpi_url, "host_name",
		     dpi_url_entry_show);


#define init_table(name, struct_name, max_entries) \
	({ \
		int __ret = 0; \
		spin_lock_init(&name.lock); \
		hash_init(name.list); \
		name.max_size = max_entries; \
		name.cache = KMEM_CACHE(struct_name, SLAB_HWCACHE_ALIGN); \
		if (!name.cache) { \
			pr_err("couldn't allocate "#name"\n"); \
			__ret = -1; \
		} \
		__ret; \
	})

#define init_proc_entry(name, permissions, parent, fops) \
	({ \
		struct proc_dir_entry *__pde; \
		int __ret = 0; \
		__pde = proc_create(name, permissions, parent, &fops); \
		if (!__pde) { \
			pr_err("couldn't create proc entry '"#name"'\n"); \
			__ret = -1; \
		} \
		__ret; \
	})

int __init dpi_init_tables(void)
{
	if (init_table(app_table, dpi_app, APP_TABLE_MAX_SIZE))
		goto err;
	if (init_table(dev_table, dpi_dev, DEV_TABLE_MAX_SIZE))
		goto err_free_app_table;
	if (init_table(appinst_table, dpi_appinst, APPINST_TABLE_MAX_SIZE))
		goto err_free_dev_table;
	if (init_table(url_table, dpi_url, URL_TABLE_MAX_SIZE))
		goto err_free_appinst_table;

	/* create proc entries */
	if (init_proc_entry("appinsts", 0440, dpi_dir, dpi_appinst_fops))
		goto err_free_url_table;
	if (init_proc_entry("apps", 0440, dpi_dir, dpi_app_fops))
		goto err_free_appinsts;
	if (init_proc_entry("devices", 0440, dpi_dir, dpi_dev_fops))
		goto err_free_apps;
	if (init_proc_entry("urls", 0440, dpi_dir, dpi_url_fops))
		goto err_free_devices;
	return 0;

err_free_devices:
	remove_proc_entry("devices", dpi_dir);
err_free_apps:
	remove_proc_entry("apps", dpi_dir);
err_free_appinsts:
	remove_proc_entry("appinsts", dpi_dir);
err_free_url_table:
	kmem_cache_destroy(url_table.cache);
err_free_appinst_table:
	kmem_cache_destroy(appinst_table.cache);
err_free_dev_table:
	kmem_cache_destroy(dev_table.cache);
err_free_app_table:
	kmem_cache_destroy(app_table.cache);
err:
	return -ENOMEM;
}

#define cleanup_dpi_table(name, struct_name) \
	do { \
		struct hlist_node *__tmp; \
		struct struct_name *__item; \
		int __i; \
		\
		spin_lock_bh(&name.lock); \
		hash_for_each_safe(name.list, __i, __tmp, __item, node) \
			kmem_cache_free(name.cache, __item); \
		kmem_cache_destroy(name.cache); \
		spin_unlock_bh(&name.lock); \
	} while (0)
void __exit dpi_deinit_tables(void)
{
	remove_proc_entry("devices", dpi_dir);
	remove_proc_entry("apps", dpi_dir);
	remove_proc_entry("appinsts", dpi_dir);
	proc_remove(dpi_dir);

	cleanup_dpi_table(app_table, dpi_app);
	cleanup_dpi_table(dev_table, dpi_dev);
	cleanup_dpi_table(appinst_table, dpi_appinst);
	cleanup_dpi_table(url_table, dpi_url);
}
