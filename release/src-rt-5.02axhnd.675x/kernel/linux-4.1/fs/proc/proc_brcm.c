#if defined(CONFIG_BCM_KF_PROC_BCM)
/*
 *
    <:copyright-BRCM:2011:DUAL/GPL:standard
    
       Copyright (c) 2011 Broadcom 
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

/************************************************************
    proc_brcm.c

    procfs entries like proc/shirkmem, proc/brcm/pagewalk and proc/brcm/cstat

     9/27/2006  Xi Wang      Created  
   11/12/2008  Xi Wang      Updated for 2.6.21


 ************************************************************/

#include <generated/autoconf.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/tty.h>
#include <linux/string.h>
#include <linux/mman.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/pagemap.h>
#include <linux/swap.h>
#include <linux/slab.h>
#include <linux/smp.h>
#include <linux/signal.h>
#include <linux/module.h>
#include <linux/init.h>
//#include <linux/smp_lock.h>
#include <linux/seq_file.h>
#include <linux/times.h>
#include <linux/profile.h>
#include <linux/blkdev.h>
#include <linux/hugetlb.h>
#include <linux/jiffies.h>
#include <linux/sysrq.h>
#include <linux/vmalloc.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include <asm/tlb.h>
#include <asm/div64.h>

#ifdef CONFIG_BCM_CFE_XARGS
extern int bcm_blxparms_init(struct proc_dir_entry *pentry);
#endif
//extern int proc_calc_metrics(char *page, char **start, off_t off, int count, int *eof, int len);
int proc_calc_metrics(char *page, char **start, off_t off, int count, int *eof, int len)
{
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}


#ifdef CONFIG_BCM_CSTAT


#define PERF_C_INTERVAL (HZ*1)
#define DIV 1000
#define N_INST
#define COUNTER_RESET_V 0xffffffffu

#define BRCM_PERFREG_BASE 0xff420000
typedef struct {
    unsigned global_ctrl;
    unsigned ctrl[2];
    unsigned donottouch[1];
    unsigned counters[4];
} PerformanceControl;
#define BRCM_PERF ((volatile PerformanceControl *) BRCM_PERFREG_BASE)

struct timer_list cachestat_timer;
int cachestat_interval = 0;

void static brcm_perf_timer_func(unsigned long data);
static int perf_counters_proc(char *page, char **start, off_t off, int count, int *eof, void *data);


static void cachestat_timer_func(unsigned long data)
{
    static int tp = 0;
    static int item = 0;
    register unsigned long temp;
    int i,ratio;
    unsigned tdiv = cachestat_interval*DIV;
    unsigned counters[4];
    
    for (i=0;i<4;i++) {
        counters[i] = COUNTER_RESET_V - BRCM_PERF->counters[i];
        BRCM_PERF->counters[i]=COUNTER_RESET_V;
    }
    
    if (item == 0) {   
        printk("TP %d instruction miss %uk/sec\n", tp, counters[0]/tdiv);
        printk("TP %d instruction hit %uk/sec\n", tp, counters[1]/tdiv);
        ratio = (counters[0]+counters[1])? counters[0]*1000/(counters[0]+counters[1]) : 0;
        printk("TP %d miss ratio %u\n", tp, ratio);
    }

    if (item == 1) {   
        printk("TP %d data miss %uk/sec\n", tp, counters[0]/tdiv);
        printk("TP %d data hit %uk/sec\n", tp, counters[1]/tdiv);
        ratio = (counters[0]+counters[1])? counters[0]*1000/(counters[0]+counters[1]) : 0;
        printk("TP %d miss ratio %u\n", tp, ratio);
    }

#if defined(N_INST)
    printk("TP %d number of instructions %uk/sec\n", tp, counters[2]/tdiv);
    printk("TP %d number of cycles %uk/sec\n", tp, counters[3]/tdiv);
#endif

    if (tp >= 1) {
        printk("\n");
        tp = 0;
        if (item >= 1) {
            item = 0;
        }
        else {            
            item++;
        }
    }
    else {
        tp++;
    }
    
    if (tp ==0) {
        asm("mfc0 %0,$22,2" : "=d" (temp));
        temp &= 0x3fffffff;
        temp |= 0x00000000;
        asm("mtc0 %0,$22,2" :: "d" (temp));
    }
    else {
        asm("mfc0 %0,$22,2" : "=d" (temp));
        temp &= 0x3fffffff;
        temp |= 0x40000000;
        asm("mtc0 %0,$22,2" :: "d" (temp));
    }    

    if (item == 0) {
        BRCM_PERF->global_ctrl = 0x0;
        BRCM_PERF->global_ctrl = 0x80000018;
        if (tp == 0) {
            BRCM_PERF->ctrl[0] = 0x80188014;
        }
        else {
            BRCM_PERF->ctrl[0] = 0xa018a014;
        }
    }
    
    if (item == 1) {
        BRCM_PERF->global_ctrl = 0x0;
        BRCM_PERF->global_ctrl = 0x80000011;
        if (tp == 0) {
            BRCM_PERF->ctrl[0] = 0x80288024;
        }
        else {
            BRCM_PERF->ctrl[0] = 0xa028a024;
        }
    }

#if defined(N_INST)
    if (tp ==0) {
        BRCM_PERF->ctrl[1] = 0x80488044;
    }
    else {
        BRCM_PERF->ctrl[1] = 0xa048a044;
    }
#endif

    cachestat_timer.expires = jiffies+cachestat_interval*HZ;
    add_timer(&cachestat_timer);
}


static void cachestat_start()
{
    int i;

    printk("Starting cache performance counters..\n\n");
    
    init_timer(&cachestat_timer);
    cachestat_timer.expires = jiffies+HZ;
    cachestat_timer.data = 0;
    cachestat_timer.function = cachestat_timer_func;

    for (i=0;i<4;i++) {
        BRCM_PERF->counters[i]=COUNTER_RESET_V;
    }

    BRCM_PERF->global_ctrl = 0x80000018;
    BRCM_PERF->global_ctrl = 0x80000011;

    add_timer(&cachestat_timer);
}

static void cachestat_stop()
{
    del_timer_sync(&cachestat_timer);
    printk("Cache performance counting stopped..\n");
}

static int cachestat_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len=0;

    len += sprintf(page, "%d\n", cachestat_interval);

	return proc_calc_metrics(page, start, off, count, eof, len);

}


static int cachestat_write_proc(struct file *file, const char *buf, unsigned long count, void *data)
{
    char ibuf[20];
    int arg;

    if (count<1 || count>sizeof(ibuf)) {
        return -EFAULT;
    }
    if (copy_from_user(ibuf, buf, count)) {
        return -EFAULT;
    }
    ibuf[count] = 0;

    if (sscanf(ibuf, "%d\n", &arg) == 1) {
        if (arg>=0) {
            if (arg && !cachestat_interval) {
                cachestat_interval = arg;
                cachestat_start();
            }
            else if (!arg && cachestat_interval) {
                cachestat_interval = arg;
                cachestat_stop();
            }
            else {
                cachestat_interval = arg;
            }
        }
        return count;
    }
    return -EFAULT;
}

#endif

#if defined(CONFIG_BRCM_OLT_FPGA_RESTORE)
/* These functions save and restore the state of the olt fpga */
static int olt_fpga_restore_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	char *op;
	struct pci_dev *dev;
	op = page + off;
	*op = '\0';
	*eof = 1;
	dev = pci_get_device(0x1172, 0x0004, NULL);
	if (dev != NULL) {
		printk("found fpga\n");
		printk("save fpga config\n");
		(void)pci_save_state(dev);
	} else {
		printk("no fpga\n");
        	return -EFAULT;
	}
	return 0;
}

static int olt_fpga_restore_write_proc(struct file *file, const char *buf, unsigned long count, void *data)
{
	struct pci_dev *dev;
	dev = pci_get_device(0x1172, 0x0004, NULL);
	if (dev != NULL) {
		printk("found fpga\n");
	} else {
		printk("no fpga\n");
        	return -EFAULT;
	}
		printk("restore fpga config\n");
		(void)pci_restore_state(dev);
	return count;
}

#endif

#if 0
static int cp0regs_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    register unsigned long temp;
    unsigned long *mips_core_base = NULL;
	int len=0;

    len += sprintf(page+len, "Running on processor %d\n", smp_processor_id());

    len += sprintf(page+len, "Status = %x\n", __read_32bit_c0_register($12, 0));
    len += sprintf(page+len, "Cause = %x\n", __read_32bit_c0_register($13, 0));

    len += sprintf(page+len, "BRCM Config_0 = %x\n", __read_32bit_c0_register($22, 0));
    len += sprintf(page+len, "CMT Interrupt = %x\n", __read_32bit_c0_register($22, 1));
    len += sprintf(page+len, "CMT Control = %x\n", __read_32bit_c0_register($22, 2));
    len += sprintf(page+len, "CMT Local = %x\n", __read_32bit_c0_register($22, 3));
    len += sprintf(page+len, "BRCM Config_1 = %x\n", __read_32bit_c0_register($22, 5));

    temp = __read_32bit_c0_register($22, 6);
    mips_core_base =(unsigned long *) (temp & 0xfffc0000);
    len += sprintf(page+len, "Core Base = %x\n", temp);
    len += sprintf(page+len, "RAC Config (%x) = %x\n", mips_core_base, *mips_core_base);
    len += sprintf(page+len, "RAC Range (%x) = %x\n", mips_core_base+1, *(mips_core_base+1));
    len += sprintf(page+len, "RAC Config1 (%x) = %x\n", mips_core_base+2, *(mips_core_base+2));
    len += sprintf(page+len, "LMB (%x) = %x\n", mips_core_base+7, *(mips_core_base+7));
    
    len += sprintf(page+len, "\n");
    
    *(page+len) = 0;
    len++;
	return proc_calc_metrics(page, start, off, count, eof, len);
}
#endif


#ifdef CONFIG_BRCM_VMTOOLS


#define ALLPAGES (1024*1024)

extern int pagewalk(char *print);
extern int proc_calc_metrics(char *page, char **start, off_t off, int count, int *eof, int len);
extern int meminfo_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data);


static int shrinkmem_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len;
    
    len = sprintf(page, "\nTry to free as many pages as possible (most of pages left will be data pages), then show memory info.\n\n");

    shrink_all_memory(ALLPAGES);

    len += meminfo_read_proc(page+len, start, off, count, eof, data);

    return proc_calc_metrics(page, start, off, count, eof, len);
}


static int pagewalk_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len;

    printk("\nList all occupied memory pages in the system and show what they are used for. ");
    printk("(output generated by printk - i.e. not for file operations)\n\n");

    len = pagewalk(page);

    return proc_calc_metrics(page, start, off, count, eof, len);
}


static int shrinkpagewalk_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

    printk("\nTry to free as many pages as possible (most of pages left will be data pages), then:\n");
    printk("List all occupied memory pages in the system and show what they are used for. ");
    printk("(output generated by printk - i.e. not for file operations)\n\n");

    shrink_all_memory(ALLPAGES);

    len = pagewalk(page);

    return proc_calc_metrics(page, start, off, count, eof, len);
}

#endif


/****************************************************************************
 * This section is for reporting kernel configs
 ****************************************************************************/


int bcm_kernel_config_smp=0;
int bcm_kernel_config_preempt=0;
int bcm_kernel_config_debug_spinlock=0;
int bcm_kernel_config_debug_mutexes=0;

EXPORT_SYMBOL(bcm_kernel_config_smp);
EXPORT_SYMBOL(bcm_kernel_config_preempt);
EXPORT_SYMBOL(bcm_kernel_config_debug_spinlock);
EXPORT_SYMBOL(bcm_kernel_config_debug_mutexes);

static int bcm_kernel_config_show(struct seq_file *m, void *v)
{
	seq_printf(m, "CONFIG_SMP=%d\n", bcm_kernel_config_smp);
	seq_printf(m, "CONFIG_PREEMPT=%d\n", bcm_kernel_config_preempt);
	seq_printf(m, "CONFIG_DEBUG_SPINLOCK=%d\n", bcm_kernel_config_debug_spinlock);
	seq_printf(m, "CONFIG_DEBUG_MUTEXES=%d\n", bcm_kernel_config_debug_mutexes);
	return 0;
}

static int bcm_kernel_config_open(struct inode *inode, struct file *file)
{
	return single_open(file, bcm_kernel_config_show, NULL);
}

static const struct file_operations proc_kernel_config_operations = {
	.open		= bcm_kernel_config_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

static int __init bcm_kernel_config_init(struct proc_dir_entry *pentry)
{

#ifdef CONFIG_SMP
	bcm_kernel_config_smp=1;
#endif
#ifdef CONFIG_PREEMPT
	bcm_kernel_config_preempt=1;
#endif
#ifdef CONFIG_DEBUG_SPINLOCK
	bcm_kernel_config_debug_spinlock=1;
#endif
#ifdef CONFIG_DEBUG_MUTEXES
	bcm_kernel_config_debug_mutexes=1;
#endif

	printk(KERN_INFO "--Kernel Config--\n");
	printk(KERN_INFO "  SMP=%d\n", bcm_kernel_config_smp);
	printk(KERN_INFO "  PREEMPT=%d\n", bcm_kernel_config_preempt);
	printk(KERN_INFO "  DEBUG_SPINLOCK=%d\n", bcm_kernel_config_debug_spinlock);
	printk(KERN_INFO "  DEBUG_MUTEXES=%d\n", bcm_kernel_config_debug_mutexes);

	proc_create("kernel_config", 0, pentry, &proc_kernel_config_operations);
	return 0;
}


/****************************************************************************
 * Entry point from proc_root_init
 ****************************************************************************/
void __init proc_brcm_init(struct proc_dir_entry *pentry)
{

    struct proc_dir_entry *entry __attribute ((unused));

#ifdef CONFIG_BRCM_VMTOOLS
    create_proc_read_entry("shrinkmem", 0, pentry, shrinkmem_read_proc, NULL);
    create_proc_read_entry("pagewalk", 0, pentry, pagewalk_read_proc, NULL);
    create_proc_read_entry("shrinkpagewalk", 0, pentry, shrinkpagewalk_read_proc, NULL);
#endif

#ifdef CONFIG_BCM_CSTAT
    entry = create_proc_entry("cstat", 0, pentry);
    entry->read_proc = cachestat_read_proc;
    entry->write_proc = cachestat_write_proc;
#endif

#if defined(CONFIG_BRCM_OLT_FPGA_RESTORE)
    entry = create_proc_entry("olt_fpga_restore", 0, pentry);
    entry->read_proc = olt_fpga_restore_read_proc;
    entry->write_proc = olt_fpga_restore_write_proc;
#endif

#if 0
    create_proc_read_entry("cp0regs", 0, pentry, cp0regs_read_proc, NULL);
#endif

    bcm_kernel_config_init(pentry);
#ifdef CONFIG_BCM_CFE_XARGS
    bcm_blxparms_init(pentry);
#endif
}

#endif
