/*
* <:copyright-BRCM:2020:DUAL/GPL:standard
* 
*    Copyright (c) 2020 Broadcom 
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
