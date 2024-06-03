/*
   <:copyright-BRCM:2022:DUAL/GPL:standard
   
      Copyright (c) 2022 Broadcom 
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
* File Name  : spdsvc_gpl.c
*
* Description: Speed Service GPL Driver
*
*******************************************************************************
*/

#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/bcm_log.h>
#include "spdsvc_defs.h"
#include "spdsvc_gpl.h"

//#define CC_SPDSVC_GPL_DEBUG_ENABLE

#define isLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_SPDSVC, BCM_LOG_LEVEL_DEBUG)
#define __logDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_SPDSVC, fmt, ##arg)
#define __logInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_SPDSVC, fmt, ##arg)
#define __logNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_SPDSVC, fmt, ##arg)
#define __logError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_SPDSVC, fmt, ##arg)

#define __print(fmt, arg...) bcm_print(fmt, ##arg)

#if defined(CC_SPDSVC_GPL_DEBUG_ENABLE)
#define __debug(fmt, arg...)                    \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
            __print(fmt, ##arg); )

#else
#define __debug(fmt, arg...)
#endif

typedef struct {
    spdsvc_ioctl_command_t spdsvc_ioctl_command;
} spdsvc_gpl_t;

static spdsvc_gpl_t spdsvc_gpl_g;

/*******************************************************************************
 *
 * User Space event notification
 *
 *******************************************************************************/
static struct eventfd_ctx *spdsvc_event_ctx_g[SPDSVC_TR471_EVT_MAX] = {[0 ... SPDSVC_TR471_EVT_MAX-1] = NULL};

// Must be called in the context of the user space process (via ioctl, etc)
int spdsvc_event_register(eSpdSvc_tr471_evt_type etype, int event_fd)
{
    if (etype < 0 || etype >= SPDSVC_TR471_EVT_MAX)
    {
        __logError("Invalid etype %d\n",etype);

        return -1;
    }
    if (spdsvc_event_ctx_g[etype])
    {
        __logError("Event already registered %d\n",etype);

        return -1;
    }

    spdsvc_event_ctx_g[etype] = eventfd_ctx_fdget(event_fd);

    if(IS_ERR(spdsvc_event_ctx_g[etype]))
    {
        __logError("Could not eventfd_ctx_fdget");

        return PTR_ERR(spdsvc_event_ctx_g[etype]);
    }

//    bcm_print("\n\tArcher event registered (event_fd %d)\n\n", event_fd);

    return 0;
}

static inline int spdsvc_event_send(eSpdSvc_tr471_evt_type etype, uint64_t increment)
{
    if(unlikely(!spdsvc_event_ctx_g[etype]))
    {
        __logError("Event not registered %d\n",etype);

        return -1;
    }

    eventfd_signal(spdsvc_event_ctx_g[etype], increment);

    return 0;
}

int spdsvc_event_unregister(eSpdSvc_tr471_evt_type etype)
{
    if(!spdsvc_event_ctx_g[etype])
    {
        __logError("Event not registered %d\n",etype);

        return -1;
    }

    eventfd_ctx_put(spdsvc_event_ctx_g[etype]);

    spdsvc_event_ctx_g[etype] = NULL;

    return 0;
}
int spdsvc_event_unregister_all(void)
{
    int evt;
    for (evt = 0; evt < SPDSVC_TR471_EVT_MAX; evt++)
    {
        if(spdsvc_event_ctx_g[evt])
        {
            spdsvc_event_unregister(evt);
        }
    }
    return 0;
}
/*******************************************************************************
 *
 * TR-471 Receive Queue
 *
 *******************************************************************************/
#if(SPDSVC_TR471_RX_QUEUE_PAGE_SIZE != PAGE_SIZE)
#error "SPDSVC_TR471_RX_QUEUE_PAGE_SIZE != PAGE_SIZE"
#endif

static spdsvc_tr471_rx_queue_t *spdsvc_tr471_rx_queue_p = NULL;

void spdsvc_tr471_rx_queue_init(void)
{
    bcm_async_queue_t *queue_p = &spdsvc_tr471_rx_queue_p->async_queue;

    memset(spdsvc_tr471_rx_queue_p, 0, spdsvc_tr471_rx_queue_mem_size());

    queue_p->alloc_p = (uint8_t *)spdsvc_tr471_rx_queue_p->entry;
    queue_p->entry_size = sizeof(spdsvc_tr471_rx_queue_entry_t);
    queue_p->depth = SPDSVC_TR471_RX_QUEUE_SIZE;
}

static int spdsvc_tr471_rx_queue_construct(void)
{
    int rx_queue_mem_size = spdsvc_tr471_rx_queue_mem_size();

    spdsvc_tr471_rx_queue_p = kmalloc(rx_queue_mem_size, GFP_KERNEL);

    if(!spdsvc_tr471_rx_queue_p)
    {
        return -1;
    }

    spdsvc_tr471_rx_queue_init();

//    bcm_print("\n\tSPDSVC Rx Queue Initialized (%uB/%uB, page %uB)\n\n",
//              sizeof(spdsvc_tr471_rx_queue_t), rx_queue_mem_size, PAGE_SIZE);

    return 0;
}

int spdsvc_tr471_rx_queue_write(spdsvc_tr471_rx_queue_entry_t *entry_p)
{
    bcm_async_queue_t *queue_p = &spdsvc_tr471_rx_queue_p->async_queue;

    if(bcm_async_queue_not_full(queue_p))
    {
        spdsvc_tr471_rx_queue_entry_t *queue_entry_p = (spdsvc_tr471_rx_queue_entry_t *)
            bcm_async_queue_entry_write(queue_p);

        *queue_entry_p = *entry_p;
//        WRITE_ONCE(queue_entry_p, );

        bcm_async_queue_entry_enqueue(queue_p);

        queue_p->stats.writes++;

        spdsvc_event_send(SPDSVC_TR471_EVT_RX_QUEUE, 1);

        return 1;
    }
    else
    {
        queue_p->stats.discards++;

        spdsvc_event_send(SPDSVC_TR471_EVT_RX_QUEUE, 1);

        return 0;
    }
}
int spdsvc_tr471_burst_cmpl_event(void)
{
    spdsvc_event_send(SPDSVC_TR471_EVT_BURST_CMPL, 1);
    return 0;
}
/*******************************************************************************
 *
 * Speed Service GPL Binding
 *
 *******************************************************************************/
int spdsvc_gpl_bind(spdsvc_ioctl_command_t spdsvc_ioctl_command)
{
    if(spdsvc_ioctl_command == NULL)
    {
        __logError("spdsvc_ioctl_command == NULL!");

        return -1;
    }

    spdsvc_gpl_g.spdsvc_ioctl_command = spdsvc_ioctl_command;

    return 0;
}

/*******************************************************************************
 *
 * Speed Service Device management
 *
 *******************************************************************************/
#define CLASS_NAME "spdsvc_rx_queue"

static struct class *class;
static struct device *device;

static int spdsvc_release(struct inode *inodep, struct file *filep)
{    
//    __debug("spdsvc: Device successfully closed\n");

    return 0;
}

static int spdsvc_open(struct inode *inodep, struct file *filep)
{
//    __debug("spdsvc: Device opened\n");

    return 0;
}

static int spdsvc_mmap(struct file *filp, struct vm_area_struct *vma)
{
    int ret = 0;
    struct page *page = NULL;
    unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);
    int rx_queue_mem_size = spdsvc_tr471_rx_queue_mem_size();
    bcm_async_queue_t *queue_p = &spdsvc_tr471_rx_queue_p->async_queue;

    if(size > rx_queue_mem_size)
    {
        __logError("size %lu > %u", size, rx_queue_mem_size);

        ret = -EINVAL;

        goto out;  
    } 
   
    page = virt_to_page((unsigned long)spdsvc_tr471_rx_queue_p + (vma->vm_pgoff << PAGE_SHIFT)); 

    ret = remap_pfn_range(vma, vma->vm_start, page_to_pfn(page), size, vma->vm_page_prot);
    if(ret != 0)
    {
        __logError("Could not remap_pfn_range");

        goto out;
    }

    // application called this function to map this queue memory, reset read and write indices
    queue_p->read = 0;
    queue_p->write = 0;

out:
    return ret;
}

static ssize_t spdsvc_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    __logError("Not supported");

    return -EFAULT;
}

static ssize_t spdsvc_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    __logError("Not supported");

    return -EFAULT;
}

static long spdsvc_ioctl(struct file *filep, unsigned int command, unsigned long arg)
{
    spdsvc_ioctl_arg_t *ioctlArg_p = (spdsvc_ioctl_arg_t *)arg;
    spdsvc_ioctl_t ioctlCmd = (spdsvc_ioctl_t)command;

    if(ioctlCmd >= SPDSVC_IOCTL_MAX)
    {
        __logError("Invalid command: %u (max %u)", ioctlCmd, SPDSVC_IOCTL_MAX);

        return -EFAULT;
    }

    __debug("IOCTL: %u", ioctlCmd);

    return spdsvc_gpl_g.spdsvc_ioctl_command(ioctlCmd, ioctlArg_p);
}

static const struct file_operations spdsvc_fops_g =
{
    .owner = THIS_MODULE,
    .open = spdsvc_open,
    .read = spdsvc_read,
    .write = spdsvc_write,
    .release = spdsvc_release,
    .mmap = spdsvc_mmap,
    .unlocked_ioctl = spdsvc_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = spdsvc_ioctl,
#endif
};

/*******************************************************************************
 *
 * Speed Service GPL Initialization
 *
 *******************************************************************************/
static int __init spdsvc_gpl_construct(void)
{
    int ret;

    memset(&spdsvc_gpl_g, 0, sizeof(spdsvc_gpl_t));

    ret = register_chrdev(SPDSVC_DRV_MAJOR, SPDSVC_DRV_NAME, &spdsvc_fops_g);
    if(ret)
    {
        __logError("Cannot register_chrdev <%d>", SPDSVC_DRV_MAJOR);

        return ret;
    }

    class = class_create(THIS_MODULE, CLASS_NAME);
    if(IS_ERR(class))
    { 
        __logError("Could not class_create");

        return PTR_ERR(class);
    }

    device = device_create(class, NULL, MKDEV(SPDSVC_DRV_MAJOR, 0), NULL, SPDSVC_DRV_NAME);
    if(IS_ERR(device))
    {
        __logError("Could not device_create");

        return PTR_ERR(device);
    }

    ret = spdsvc_tr471_rx_queue_construct();
    if(ret)
    {
        __logError("Cannot spdsvc_tr471_rx_queue_construct");

        return ret;
    }

    __print(SPDSVC_MODNAME " Char Driver " SPDSVC_VER_STR " Registered <%d>\n", SPDSVC_DRV_MAJOR);

    /* debugging only */
    bcmLog_setLogLevel(BCM_LOG_ID_SPDSVC, BCM_LOG_LEVEL_ERROR);

    return 0;
}

static void __exit spdsvc_gpl_destruct(void)
{
    device_destroy(class, MKDEV(SPDSVC_DRV_MAJOR, 0));  
    class_unregister(class);
    class_destroy(class); 
    unregister_chrdev(SPDSVC_DRV_MAJOR, SPDSVC_DRV_NAME);

    __logNotice(SPDSVC_MODNAME " Char Driver " SPDSVC_VER_STR " Unregistered <%d>", SPDSVC_DRV_MAJOR);
}

module_init(spdsvc_gpl_construct);
module_exit(spdsvc_gpl_destruct);

EXPORT_SYMBOL(spdsvc_event_register);
EXPORT_SYMBOL(spdsvc_event_unregister);
EXPORT_SYMBOL(spdsvc_event_unregister_all);
EXPORT_SYMBOL(spdsvc_tr471_rx_queue_write);
EXPORT_SYMBOL(spdsvc_tr471_rx_queue_init);
EXPORT_SYMBOL(spdsvc_tr471_burst_cmpl_event);
EXPORT_SYMBOL(spdsvc_gpl_bind);

MODULE_DESCRIPTION(SPDSVC_MODNAME " GPL");
MODULE_VERSION(SPDSVC_VERSION);
MODULE_LICENSE("GPL");
