/*
  <:copyright-BRCM:2023:DUAL/GPL:standard
   
  Copyright (c) 2023 Broadcom 
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

/*
*******************************************************************************
*
* File Name  : crossbow_gpl.c
*
* Description: Crossbow GPL Driver
*
*******************************************************************************
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bcm_log.h>
#include <linux/nbuff.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include "bcm_rsvmem.h"
#include "bcm_timer.h"

#include "crossbow_gpl.h"

#define isLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_ARCHER, BCM_LOG_LEVEL_DEBUG)
#define __logDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_ARCHER, fmt, ##arg)
#define __logInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_ARCHER, fmt, ##arg)
#define __logNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_ARCHER, fmt, ##arg)
#define __logError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_ARCHER, fmt, ##arg)

crossbow_enet_hooks_t crossbow_gpl_hooks_g;
EXPORT_SYMBOL(crossbow_gpl_hooks_g);

#if defined(CC_CROSSBOW_TIMER)
crossbow_timer_hooks_t crossbow_gpl_timer_hooks_g;
EXPORT_SYMBOL(crossbow_gpl_timer_hooks_g);
#endif

#define PROC_DIR          "driver/crossbow"
#define CPU_PROC_FILE     "cpu_usage"

static struct proc_dir_entry *proc_dir;

static ssize_t cpu_show(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    unsigned long sec;

    if (kstrtoul_from_user(buf, count, 10, &sec))
        return -1;

    if (!crossbow_gpl_hooks_g.cycle_get)
        return -1;

    crossbow_gpl_hooks_g.cycle_get(sec);

    return count;
}

static int cpu_open(struct seq_file *s, void *v)
{
    return 0;
}

static int cpu_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, cpu_open, NULL);
}

static const struct proc_ops cpu_proc_fops = {
    .proc_open    = cpu_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = cpu_show,
};

static int crossbow_proc_cpu_profile_init(void)
{
    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir) {
        pr_err("couldn't create %s proc directory\n", PROC_DIR);
	goto err;
    }

    if (!proc_create(CPU_PROC_FILE, 0666, proc_dir, &cpu_proc_fops))
    {
        pr_err("failed to create %s entry\n", CPU_PROC_FILE);
        proc_remove(proc_dir);
        goto err;
    }

    return 0;

 err:
    return -ENOMEM;
}

#if IS_ENABLED(CONFIG_BCM_FPI)
typedef struct {
    struct dentry *frag_enable;
    struct dentry *reasm_enable;
    crossbow_fpi_frag_reasm_hooks_t enable_hooks;
} crossbow_fpi_frag_reasm_t;

crossbow_fpi_frag_reasm_t crossbow_gpl_fpi_frag_reasm_g;
#endif

int archer_gpl_cpu_affinity_set(struct task_struct *task_p);

int crossbow_gpl_enet_bind(crossbow_enet_hooks_t *hooks_p)
{
    bcmFun_t *archer_host_bind = bcmFun_get(BCM_FUN_ID_ARCHER_HOST_BIND);
    int ret;

    if(!archer_host_bind)
    {
        __logError("Archer binding is not available");

        return -1;
    }

    crossbow_gpl_hooks_g.rxq_isr = hooks_p->rxq_isr;
    crossbow_gpl_hooks_g.rxq_isr_cpu_id = hooks_p->rxq_isr_cpu_id;
    crossbow_gpl_hooks_g.rcq_isr = hooks_p->rcq_isr;
    crossbow_gpl_hooks_g.rcq_isr_cpu_id = hooks_p->rcq_isr_cpu_id;
    crossbow_gpl_hooks_g.stats_dump = hooks_p->stats_dump;

    ret = archer_host_bind(&crossbow_gpl_hooks_g);
    if(ret)
    {
        __logError("Could not archer_host_bind");

        return ret;
    }

    if(crossbow_gpl_hooks_g.archer_task_p)
    {
        archer_gpl_cpu_affinity_set(crossbow_gpl_hooks_g.archer_task_p);
    }

    crossbow_proc_cpu_profile_init();

    return 0;
}
EXPORT_SYMBOL(crossbow_gpl_enet_bind);

#if defined(CC_CROSSBOW_TIMER)

int crossbow_gpl_timer_bind(crossbow_timer_hooks_t *hooks_p)
{
    crossbow_gpl_timer_hooks_g = *hooks_p;

    return bcm_timer_hw_bind(hooks_p);
}
EXPORT_SYMBOL(crossbow_gpl_timer_bind);

#endif /* CC_CROSSBOW_TIMER */

int crossbow_gpl_flow_memory_get(crossbow_gpl_flow_memory_t *flow_mem_p)
{
    int ret;

    ret = BcmMemReserveGetByName(RNRMEM_BASE_ADDR_STR, &flow_mem_p->virt_p,
                                 &flow_mem_p->phys_addr, &flow_mem_p->size);
    if(ret)
    {
        __logError("Could not BcmMemReserveGetByName (%s)", RNRMEM_BASE_ADDR_STR);

        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(crossbow_gpl_flow_memory_get);

#if IS_ENABLED(CONFIG_BCM_FPI)
static ssize_t crossbow_debugfs_fpi_frag_enable_get(struct file *file,
        char __user *user_buf, size_t count, loff_t *ppos)
{
    ssize_t ret, out_offset = 0, out_count = 64;
    char *buf = kmalloc(out_count, GFP_KERNEL);

    if(buf == NULL)
        return -ENOMEM;

    if(crossbow_gpl_fpi_frag_reasm_g.enable_hooks.enable_get)
    {
        int val = crossbow_gpl_fpi_frag_reasm_g.enable_hooks.enable_get(1);
        out_offset += snprintf(buf + out_offset, out_count - out_offset,
            "ip_frag_enable is set to %d\n", val);
    }
    else
    {
        out_offset += snprintf(buf + out_offset, out_count - out_offset,
            "fpi_frag_reasm_hooks not set\n");
    }
    ret = simple_read_from_buffer(user_buf, count, ppos, buf, out_offset);
    kfree(buf);
    return ret;
}

static ssize_t crossbow_debugfs_fpi_frag_enable_set(struct file *file,
        const char __user *user_buf, size_t count, loff_t *ppos)
{
    char buf[16];
    size_t buf_size;
    int val;

    buf_size = min(count, (sizeof(buf) - 1));
    if(copy_from_user(buf, user_buf, buf_size))
        return -EFAULT;

    buf[buf_size] = '\0';
    val = simple_strtol(buf, NULL, 10);

    if(crossbow_gpl_fpi_frag_reasm_g.enable_hooks.enable_set)
        crossbow_gpl_fpi_frag_reasm_g.enable_hooks.enable_set(1, val);
    else
        pr_err("fpi_frag_reasm_hooks not set\n");
    return count;
}

static ssize_t crossbow_debugfs_fpi_reasm_enable_get(struct file *file,
        char __user *user_buf, size_t count, loff_t *ppos)
{
    ssize_t ret, out_offset = 0, out_count = 64;
    char *buf = kmalloc(out_count, GFP_KERNEL);

    if(buf == NULL)
        return -ENOMEM;

    if(crossbow_gpl_fpi_frag_reasm_g.enable_hooks.enable_get)
    {
        int val = crossbow_gpl_fpi_frag_reasm_g.enable_hooks.enable_get(0);
        out_offset += snprintf(buf + out_offset, out_count - out_offset,
            "ip_reasm_enable is set to %d\n", val);
    }
    else
    {
        out_offset += snprintf(buf + out_offset, out_count - out_offset,
            "fpi_frag_reasm_hooks not set\n");
    }
    ret = simple_read_from_buffer(user_buf, count, ppos, buf, out_offset);
    kfree(buf);
    return ret;
}

static ssize_t crossbow_debugfs_fpi_reasm_enable_set(struct file *file,
        const char __user *user_buf, size_t count, loff_t *ppos)
{
    char buf[16];
    size_t buf_size;
    int val;

    buf_size = min(count, (sizeof(buf) - 1));
    if(copy_from_user(buf, user_buf, buf_size))
        return -EFAULT;

    buf[buf_size] = '\0';
    val = simple_strtol(buf, NULL, 10);

    if(crossbow_gpl_fpi_frag_reasm_g.enable_hooks.enable_set)
        crossbow_gpl_fpi_frag_reasm_g.enable_hooks.enable_set(0, val);
    else
        pr_err("fpi_frag_reasm_hooks not set\n");
    return count;
}

static const struct file_operations crossbow_debugfs_fpi_frag_enable = {
    .owner = THIS_MODULE,
    .open = simple_open,
    .write = crossbow_debugfs_fpi_frag_enable_set,
    .read = crossbow_debugfs_fpi_frag_enable_get,
};

static const struct file_operations crossbow_debugfs_fpi_reasm_enable = {
    .owner = THIS_MODULE,
    .open = simple_open,
    .write = crossbow_debugfs_fpi_reasm_enable_set,
    .read = crossbow_debugfs_fpi_reasm_enable_get,
};
#endif

void crossbow_gpl_fpi_frag_reasm_enable_bind(crossbow_fpi_frag_reasm_hooks_t *hooks_p)
{
#if IS_ENABLED(CONFIG_BCM_FPI)
    crossbow_gpl_fpi_frag_reasm_g.enable_hooks.enable_get = hooks_p->enable_get;
    crossbow_gpl_fpi_frag_reasm_g.enable_hooks.enable_set = hooks_p->enable_set;
#endif
}

EXPORT_SYMBOL(crossbow_gpl_fpi_frag_reasm_enable_bind);


int crossbow_gpl_construct(struct dentry *debugfs_dir)
{
    memset(&crossbow_gpl_hooks_g, 0, sizeof(crossbow_enet_hooks_t));

#if IS_ENABLED(CONFIG_BCM_FPI)
    memset(&crossbow_gpl_fpi_frag_reasm_g, 0, sizeof(crossbow_fpi_frag_reasm_t));
    crossbow_gpl_fpi_frag_reasm_g.frag_enable = debugfs_create_file("ip_frag_enable",
            0600, debugfs_dir, &crossbow_gpl_fpi_frag_reasm_g, &crossbow_debugfs_fpi_frag_enable);
    crossbow_gpl_fpi_frag_reasm_g.reasm_enable = debugfs_create_file("ip_reasm_enable",
            0600, debugfs_dir, &crossbow_gpl_fpi_frag_reasm_g, &crossbow_debugfs_fpi_reasm_enable);
#endif

    return 0;
}

void crossbow_gpl_destruct(void)
{
#if IS_ENABLED(CONFIG_BCM_FPI)
    debugfs_remove(crossbow_gpl_fpi_frag_reasm_g.frag_enable);
    debugfs_remove(crossbow_gpl_fpi_frag_reasm_g.reasm_enable);
#endif
}
