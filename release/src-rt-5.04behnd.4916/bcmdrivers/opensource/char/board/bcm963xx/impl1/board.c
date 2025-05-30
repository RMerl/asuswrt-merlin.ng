/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
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


/***************************************************************************
* File Name  : board.c
*
* Description: This file contains Linux character device driver entry
*              for the board related ioctl calls: flash, get free kernel
*              page and dump kernel memory, etc.
*
*
***************************************************************************/

/* Includes. */
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#if defined(CONFIG_COMPAT)
#include "compat_board.h"
#endif
#include <bcmtypes.h>
#include <board.h>


#include "board_wl.h"
#include "board_proc.h"
#include "board_util.h"
#include "board_image.h"
#include "board_ioctl.h"

#if defined(CONFIG_BCM_EXT_TIMER)
#include "bcm_ext_timer.h"
#endif

/* Externs. */

static int board_open( struct inode *inode, struct file *filp );
static ssize_t board_read(struct file *filp,  char __user *buffer, size_t count, loff_t *ppos);
static unsigned int board_poll(struct file *filp, struct poll_table_struct *wait);
static int board_release(struct inode *inode, struct file *filp);

static int board_driver_init(void);

/* brcmboard device driver related variables */
static struct cdev brcmboard_cdev;
static struct device *brcmboard_device = NULL;
static struct class *brcmboard_cl     = NULL;
static dev_t brcmboard_devId;

wait_queue_head_t g_board_wait_queue;

static struct file_operations board_fops =
{
    open:       board_open,
    unlocked_ioctl: board_unlocked_ioctl,
#if defined(CONFIG_COMPAT)
    compat_ioctl: compat_board_ioctl,
#endif    
    poll:       board_poll,
    read:       board_read,
    release:    board_release,
};

uint32 board_init_success = -1;

int BpGetBoardId( char *pszBoardId)
{
    return (envram_get_locked(NVRAM_SZBOARDID, pszBoardId, NVRAM_BOARD_ID_STRING_LEN) <= 0) ? -1 : 0;
}
EXPORT_SYMBOL(BpGetBoardId);

#if defined(MODULE)
int init_module(void)
{
    return( brcm_board_init() );
}

void cleanup_module(void)
{
    if (MOD_IN_USE)
        printk("brcm flash: cleanup_module failed because module is in use\n");
    else
        brcm_board_cleanup();
}
#endif //MODULE

static int board_driver_init(void)
{
    int ret = 0;
  
    alloc_chrdev_region(&brcmboard_devId, 0, 2, "brcmboard");
    
    /* Create class and device ( /sys entries ) */
    brcmboard_cl = class_create(THIS_MODULE, "brcmboard");
    if(brcmboard_cl == NULL)
    {
       printk(KERN_ERR "Error creating device class\n");
       goto err_cdev_cleanup;
    }
    
    brcmboard_device = device_create(brcmboard_cl, NULL, brcmboard_devId, NULL, "brcmboard");
    if(brcmboard_device == NULL)
    {
       printk(KERN_ERR "Error creating device\n");
       goto err_class_cleanup;
    }
    
    /* Set the DMA masks for this device */
    dma_coerce_mask_and_coherent(brcmboard_device, DMA_BIT_MASK(32));
        
    /* Init the character device */
    cdev_init(&brcmboard_cdev, &board_fops);
    brcmboard_cdev.owner = THIS_MODULE;
    ret = cdev_add(&brcmboard_cdev, brcmboard_devId, 1);
    
    if( ret!=0 )
    {
       printk(KERN_ERR "Error %d adding brcmboard driver", ret);
       goto err_device_cleanup;
    }
    else
        return ret;

err_device_cleanup:
    device_destroy(brcmboard_cl, brcmboard_devId);
err_class_cleanup:
    class_destroy(brcmboard_cl);
err_cdev_cleanup:
    cdev_del(&brcmboard_cdev);

    return -1;
}

static int __init brcm_board_init( void )
{
	if( board_driver_init() == 0 )
        printk(KERN_INFO "brcmboard registered\n");
    else
    { 
        printk(KERN_ERR "brcm_board_init: fail to register device.\n");
        return -1;
    }

    printk(KERN_INFO "brcmboard: brcm_board_init entry\n");
    init_waitqueue_head(&g_board_wait_queue);

    board_util_init();

    board_wl_init();
    
    add_proc_files();

    board_init_success = 0;

    return board_init_success;
}

void __exit brcm_board_cleanup( void )
{
    printk("brcm_board_cleanup()\n");

    del_proc_files();

    /* Delete cdev */
    cdev_del(&brcmboard_cdev);

    /* destroy the device and device class */
    device_destroy(brcmboard_cl, brcmboard_devId);
    class_destroy(brcmboard_cl);

    /* Unregister chrdev region */
    unregister_chrdev_region(brcmboard_devId, 1);

    /* Deinit specific modules if initialization was successful */
    if (board_init_success == 0)
    {
        board_wl_deinit();
        board_util_deinit();
	board_init_success = -1;
    }
}

static int board_open( struct inode *inode, struct file *filp )
{
    filp->private_data = board_ioc_alloc();

    if (filp->private_data == NULL)
        return -ENOMEM;

    return( 0 );
}

static int board_release(struct inode *inode, struct file *filp)
{
    BOARD_IOC *board_ioc = filp->private_data;

    wait_event_interruptible(g_board_wait_queue, 1);
    board_ioc_free(board_ioc);

    return( 0 );
}


static unsigned int board_poll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
#if defined (WIRELESS)
    BOARD_IOC *board_ioc = filp->private_data;
#endif

    poll_wait(filp, &g_board_wait_queue, wait);
#if defined (WIRELESS)
    if(board_ioc->eventmask & SES_EVENTS){
        mask |= sesBtn_poll(filp, wait);
    }
#endif

    return mask;
}

static ssize_t board_read(struct file *filp,  char __user *buffer, size_t count, loff_t *ppos)
{
#if defined (WIRELESS)
    BOARD_IOC *board_ioc = filp->private_data;
    if(board_ioc->eventmask & SES_EVENTS){
        return sesBtn_read(filp, buffer, count, ppos);
    }
#endif
    return 0;
}

/***************************************************************************
* MACRO to call driver initialization and cleanup functions.
***************************************************************************/
module_init( brcm_board_init );
module_exit( brcm_board_cleanup );

EXPORT_SYMBOL(dumpaddr);
EXPORT_SYMBOL(kerSysGetChipId);
EXPORT_SYMBOL(kerSysGetChipName);
EXPORT_SYMBOL(kerSysGetMacAddressType);
EXPORT_SYMBOL(kerSysGetMacAddress);
EXPORT_SYMBOL(kerSysReleaseMacAddress);
EXPORT_SYMBOL(kerSysGetGponSerialNumber);
EXPORT_SYMBOL(kerSysGetGponPassword);
EXPORT_SYMBOL(kerSysFsFileGet);
EXPORT_SYMBOL(kerSysFsFileSet);
EXPORT_SYMBOL(kerSysGetSdramSize);
EXPORT_SYMBOL(kerSysGetDslPhyEnable);
EXPORT_SYMBOL(kerSysSetOpticalPowerValues);
EXPORT_SYMBOL(kerSysGetOpticalPowerValues);
#if !defined(CONFIG_BCM_BCA_LEGACY_LED_API)
EXPORT_SYMBOL(kerSysLedCtrl);
#endif
EXPORT_SYMBOL(kerSysSendtoMonitorTask);
EXPORT_SYMBOL(kerSysGetAfeId);


MODULE_LICENSE("GPL");
