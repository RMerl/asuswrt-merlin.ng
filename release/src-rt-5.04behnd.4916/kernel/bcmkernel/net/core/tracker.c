/*
* <:copyright-BRCM:2020:DUAL/GPL:standard
* 
*    Copyright (c) 2020 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
:>
*/
#include <linux/hashtable.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/tracker.h>

#define STATES_SIZE 64
#define STATES_MASK (STATES_SIZE - 1)
#define STR_SIZE 62

struct entry {
    struct hlist_node hn;
    unsigned long ptr;
    unsigned cur_idx;
    char states[STATES_SIZE][STR_SIZE];
};

static DEFINE_HASHTABLE(tracker_hash, 15); // 32K
static DEFINE_SPINLOCK(tracker_lock);

void track_printf(void *ptr_any, const char *fmt, ...) {
    struct entry *en;
    char *buf;
    const unsigned long ptr = (unsigned long) ptr_any;
    va_list args;

    va_start(args, fmt);

    spin_lock_bh(&tracker_lock);
    hash_for_each_possible(tracker_hash, en, hn, ptr) {
        if (en->ptr == ptr)
            goto out_good;
    }
    en = kzalloc(sizeof(*en), GFP_ATOMIC);
    if (!en)
        goto out_bad;

    en->ptr = ptr;
    hash_add(tracker_hash, &en->hn, ptr);

out_good:
    buf = en->states[en->cur_idx & STATES_MASK];
    vsnprintf(buf, STR_SIZE, fmt, args);
    buf[STR_SIZE - 1] = 0;
    en->cur_idx++;

out_bad:
    spin_unlock_bh(&tracker_lock);
    va_end(args);
}

static int tracker_info_show(struct seq_file *m, void *v) {
    struct entry *en;
    int i, j, k;

    spin_lock_bh(&tracker_lock);
    hash_for_each(tracker_hash, i, en, hn) {
        seq_printf(m, "%lx %d ", en->ptr, en->cur_idx);
        for (j = 0, k = en->cur_idx - 1; j < STATES_SIZE; j++, k--) {
            const char *st = en->states[k & STATES_MASK];
            if (st)
                seq_printf(m, " %s", st);
        }
        seq_printf(m, "\n");
    }
    spin_unlock_bh(&tracker_lock);
    return 0;
}

static int tracker_info_open(struct inode *inode, struct file *file)
{
    return single_open(file, tracker_info_show, NULL);
}

static const struct file_operations tracker_info_ops = {
    .open    = tracker_info_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

void track_init(void) {
    proc_create("tracker", 0, NULL, &tracker_info_ops);
}

void tracker_print(void *ptr_any) {
    const unsigned long ptr = (unsigned long) ptr_any;
    struct entry *en;
    int j, k;

    spin_lock_bh(&tracker_lock);
    hash_for_each_possible(tracker_hash, en, hn, ptr) {
        if (en->ptr == ptr) {
            printk("%lx %d ", en->ptr, en->cur_idx);
            for (j = 0, k = en->cur_idx - 1; j < STATES_SIZE; j++, k--) {
                const char *st = en->states[k & STATES_MASK];
                if (st && *st)
                    printk(" %s", st);
            }
            printk("\n");
        }
    }
    spin_unlock_bh(&tracker_lock);
}

void tracker_find(void *ptr_any, void (*cb)(void *ctx, const char *st), void *ctx) {
    const unsigned long ptr = (unsigned long) ptr_any;
    struct entry *en;
    int j, k;

    spin_lock_bh(&tracker_lock);
    hash_for_each_possible(tracker_hash, en, hn, ptr) {
        if (en->ptr == ptr) {
            for (j = 0, k = en->cur_idx - 1; j < STATES_SIZE; j++, k--) {
                const char *st = en->states[k & STATES_MASK];
                if (st && *st)
                    cb(ctx, st);
            }
        }
    }
    spin_unlock_bh(&tracker_lock);
}

EXPORT_SYMBOL(tracker_find);
EXPORT_SYMBOL(tracker_print);
EXPORT_SYMBOL(track_init);
EXPORT_SYMBOL(track_printf);
