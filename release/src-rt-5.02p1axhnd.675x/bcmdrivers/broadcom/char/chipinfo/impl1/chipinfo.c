/*
<:copyright-BRCM:2011:proprietary:standard

   Copyright (c) 2011 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/
/***************************************************************************
* File Name  : chipinfo.c
*
* Description: provides IOCTLS to provide chip information to userspace
*
*
***************************************************************************/

/* Includes. */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
// #include <linux/interrupt.h>
// #include <linux/capability.h>
// #include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>
// #include <linux/pagemap.h>
// #include <asm/uaccess.h>
// #include <linux/wait.h>
// #include <linux/poll.h>
// #include <linux/sched.h>
// #include <linux/list.h>
// #include <linux/if.h>
// #include <linux/ctype.h>
// #include <linux/proc_fs.h>
#include <linux/uaccess.h>

#include <bcm_map.h>
#include <chipinfo_ioctl.h>
#include <linux/bcm_log.h>

/* Typedefs. */

/* Prototypes. */
static int chipinfo_open(struct inode *inode, struct file *filp);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
static int chipinfo_ioctl(struct inode *inode, struct file *flip, unsigned int command, unsigned long arg);
#endif
static int chipinfo_release(struct inode *inode, struct file *filp);
static int __init brcm_chipinfo_init(void);
void __exit brcm_chipinfo_cleanup(void);


/*
 * flashImageMutex must be acquired for all write operations to
 * nvram, CFE, or fs+kernel image.  (cfe and nvram may share a sector).
 */

uint32_t chipinfo_major = 0;

void __exit brcm_chipinfo_cleanup(void)
{
}


static int chipinfo_open(struct inode *inode, struct file *filp)
{

    return( 0 );
}

static int chipinfo_release(struct inode *inode, struct file *filp)
{

    return( 0 );
}

struct file_operations monitor_fops;

//********************************************************************************************
// misc. ioctl calls come to here. (flash, led, reset, kernel memory access, etc.)
//********************************************************************************************
static int chipinfo_ioctl(struct inode *inode, struct file *flip,
                       unsigned int command, unsigned long arg)
{
    int ret = -EINVAL;
    CHIPINFO_IOCTL_PARMS ctrlParms;

    switch (command) {
    case CHIPINFO_IOCTL_GET_CHIP_CAPABILITY:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
        {

            switch (ctrlParms.action) {
            case CAN_DECT:
		#if defined(OTP_GET_USER_BIT) && defined(OTP_DECT_DISABLE)
			ctrlParms.result = OTP_GET_USER_BIT(OTP_DECT_DISABLE) ^ 1;
		#else
			ctrlParms.result = -EINVAL;
		#endif
		ret = 0;
		break;
            case CAN_STBC:
		#if defined(OTP_GET_USER_BIT) && defined(OTP_SUPPORT_STBC)
			ctrlParms.result = OTP_GET_USER_BIT(OTP_SUPPORT_STBC);
		#else
			ctrlParms.result = -EINVAL;
		#endif
		ret = 0;
		break;
	    default:
		ret = -1;
	    }
        }
        if (ret == 0) { 
             copy_to_user((void *)arg, (void *) &ctrlParms, sizeof(ctrlParms));
        }

        break;
    default:
        break;
    }

    return (ret);

} /* board_ioctl */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static DEFINE_MUTEX(chipIoctlMutex);

static long unlocked_chipinfo_ioctl(struct file *filep, unsigned int cmd, 
                              unsigned long arg)
{
    struct inode *inode;
    long rt;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
    inode = filep->f_dentry->d_inode;
#else
    inode = file_inode(filep);
#endif


    mutex_lock(&chipIoctlMutex);
    rt = chipinfo_ioctl( inode, filep, cmd, arg );
    mutex_unlock(&chipIoctlMutex);
    
    return rt;
}
#endif

static struct file_operations chipinfo_fops =
{

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
    .unlocked_ioctl = unlocked_chipinfo_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = unlocked_chipinfo_ioctl,
#endif
#else
    .ioctl   = chipinfo_ioctl,
#endif
    .open    = chipinfo_open,
    .release = chipinfo_release,
};

static int __init brcm_chipinfo_init(void)
{
    int ret;

    ret = register_chrdev(CHIPINFO_DRV_MAJOR, "chipinfo", &chipinfo_fops );
    if (ret < 0)
        printk( "brcm_chipinfo_init(major %d): fail to register device.\n",CHIPINFO_DRV_MAJOR);
    else
    {
        printk("brcmchipinfo: brcm_chipinfo_init entry\n");
        chipinfo_major = CHIPINFO_DRV_MAJOR;
    }

   return ret;
}

/***************************************************************************
* MACRO to call driver initialization and cleanup functions.
***************************************************************************/
module_init(brcm_chipinfo_init);
module_exit(brcm_chipinfo_cleanup);

MODULE_LICENSE("proprietary");

