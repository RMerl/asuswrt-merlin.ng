/*
*  Copyright 2011, Broadcom Corporation
*
* <:copyright-BRCM:2011:proprietary:standard
* 
*    Copyright (c) 2011 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/

//**************************************************************************
// File Name  : EponDriver.c
//
// Description: Broadcom EPON  Interface Driver
//               
//**************************************************************************
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>     /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    /* O_ACCMODE */
#include <linux/aio.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/delay.h> 
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include "bcm_epon_common.h"
#include "mac_drv.h"
#include <rdpa_api.h>
#ifdef EPON_NORMAL_MODE
#include "EponCtrl.h"
#if defined(CONFIG_COMPAT)
#include "EponCtrlCompat.h"
#include "EponUser.h"
#include "rdpa_epon.h"
#include "rdpa_ag_epon.h"
#endif
#endif

// Hardware specific libraries.
#define BCM_EPON_MODULE_VERSION "0.2"

int epon_usr_init = 0;
int epon_chrdev_major = 313;
module_param(epon_chrdev_major, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(epon_usr_init, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);


static DEFINE_MUTEX(EponIoctlMutex);
static struct cdev epon_chrdev_cdev;
extern void EponAeDriverInit(BOOL);
#ifndef EPON_NORMAL_MODE
#define EponDriverInit()
#define EponDriverExit()
#endif

long eponCoreDrvIoctl(struct file *filp, unsigned int cmd, unsigned long arg)
    {
    long ret = EponSTATUSSUCCESS;
    mutex_lock(&EponIoctlMutex);
#ifdef EPON_NORMAL_MODE    
    ret = EponIoctlAcess(cmd,arg);
#endif
    mutex_unlock(&EponIoctlMutex);
    return ret;
    }

#if defined(CONFIG_COMPAT)   
long eponCoreDrvIoctlCompat(struct file *filp, unsigned int cmd, unsigned long arg)
    {
    long ret = EponSTATUSSUCCESS;
    mutex_lock(&EponIoctlMutex);
#ifdef EPON_NORMAL_MODE    
    ret = EponIoctlAcessCompat(cmd,arg);
#endif
    mutex_unlock(&EponIoctlMutex);
    return ret;
    }
#endif    

/*
 * Open and close
 */
int eponCoreDrvOpen(struct inode *inode, struct file *filp)
    {
    //printk("%s:%d\n", __FUNCTION__, __LINE__);
    return 0;
    }


int eponCoreDrvRelease(struct inode *inode, struct file *filp)
    {
    //printk("%s:%d\n", __FUNCTION__, __LINE__);
    return 0;
    }

/*
 * The fops
 */
struct file_operations epon_core_fops = {
    .owner = THIS_MODULE,
    .open = eponCoreDrvOpen,
    .release = eponCoreDrvRelease,
    .unlocked_ioctl = eponCoreDrvIoctl,
    #if defined(CONFIG_COMPAT)
    .compat_ioctl = eponCoreDrvIoctlCompat
    #endif
};

int EponChrdevInit(void)
    {
    dev_t dev;
    int rc;
    
    if (!epon_chrdev_major)
        {
        BCM_LOG_ERROR(BCM_LOG_ID_EPON, "EPON Major number not defined.");
        return -1;
        }
    
    dev = MKDEV(epon_chrdev_major, 0);
    rc = register_chrdev_region(dev, 1, BCM_EPON_DRIVER_NAME);
    if (rc < 0)
        {
        BCM_LOG_ERROR(BCM_LOG_ID_EPON, "EPON register chrdev feailed..");
        return -1;
        }

    epon_chrdev_major = MAJOR(dev);
    cdev_init(&epon_chrdev_cdev, &epon_core_fops);
    
    rc = cdev_add(&epon_chrdev_cdev, dev, 1);
    if(rc)
        {
        BCM_LOG_ERROR(BCM_LOG_ID_EPON, "EPON dev create failed.");
        return -1;
        }
    
    return 0;
    }


static int __init moduleInit(void)
    {
    int rc;

    rc = EponChrdevInit();
    if(rc)
        {
        BCM_LOG_ERROR(BCM_LOG_ID_EPON, "EPON dev init failed.");
        return -1;
        }

    if (epon_usr_init)
        {
        EponDriverInit();
        BCM_LOG_INFO(BCM_LOG_ID_EPON, "EPON module initialization done");
        }
    else
        {
        EponAeDriverInit(1);
        }
       
    return 0;
    }

void EponChrdevExit(void)
    {
    cdev_del(&epon_chrdev_cdev);
    if ((epon_chrdev_major))
        unregister_chrdev_region(MKDEV(epon_chrdev_major, 0), 1);
    }


/*
 *     Module 'remove' entry point.
 *     o delete /proc/net/router directory and static entries.
 */
static void __exit moduleCleanup(void)
    {
    EponChrdevExit();
    if (epon_usr_init)
        {
        EponDriverExit();
        }

    BCM_LOG_INFO(BCM_LOG_ID_EPON, "EPON module cleanup done");
    }

module_init(moduleInit);
module_exit(moduleCleanup);

MODULE_LICENSE("Proprietary");
MODULE_VERSION(BCM_EPON_MODULE_VERSION);
MODULE_DESCRIPTION("Broadcom EPON Driver");

