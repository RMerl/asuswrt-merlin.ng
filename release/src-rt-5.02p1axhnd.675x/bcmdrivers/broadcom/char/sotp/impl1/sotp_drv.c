/*
<:copyright-BRCM:2018:proprietary:standard

   Copyright (c) 2018 Broadcom 
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
* File Name  : sotp_drv.c
*
* Description: provides IOCTLS to provide sotp read/write to userspace
*
*
***************************************************************************/

/* Includes. */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/crc32.h>
#include <bcmtypes.h>
#include <bcm_map.h>
#include <bcm_sotp.h>
#include <linux/bcm_log.h>
#include "sotp_base_defs.h"

/***************************************************************************
* Types and Defines
***************************************************************************/
typedef struct sotp_ctx
{
    uint32_t sotp_major;
    uintptr_t sotp_base;
} SOTP_CTX;


#define SOTP_DBG_API        0
#define SOTP_DBG_ENABLE     0
#if SOTP_DBG_ENABLE
#   define SOTP_DBG_LEVEL      KERN_DEBUG
#   define SOTP_DBG(fmt, args...) printk( SOTP_DBG_LEVEL "SOTP_drv: " fmt, ## args)
#else
#   define SOTP_DBG(fmt, args...) /* not DBGging: nothing */
#endif
#define SOTP_ERR_PRINT(fmt, args...) printk( KERN_ERR "SOTP_drv: " fmt, ## args)

/***************************************************************************
* Prototypes
***************************************************************************/
static int sotp_open(struct inode *inode, struct file *filp);
static int sotp_release(struct inode *inode, struct file *filp);
static int __init brcm_sotp_init(void);
static void __exit brcm_sotp_cleanup(void);
static int sotp_ioctl(struct inode *inode, struct file *flip, unsigned int command, unsigned long arg);
static long unlocked_sotp_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

/***************************************************************************
* Local Variables
***************************************************************************/
static SOTP_CTX sotp;
static DEFINE_MUTEX(sotpIoctlMutex);

/***************************************************************************
* Driver API
***************************************************************************/
static struct file_operations sotp_fops =
{
    .unlocked_ioctl = unlocked_sotp_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl   = unlocked_sotp_ioctl,
#endif
    .open           = sotp_open,
    .release        = sotp_release,
};

static int sotp_ioctl(struct inode *inode, struct file *flip, unsigned int command, unsigned long arg)
{
    int ret = -EINVAL;
    SOTP_IOCTL_PARMS ctrlParms;
    char * bufp = NULL;

    SOTP_DBG("%s: entry\n", __FUNCTION__); 


    if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) != 0) 
    {
        SOTP_ERR_PRINT("%s: Failed to copy user ctrlParms!\n", __FUNCTION__);
        return ret;
    }

#if defined(CONFIG_COMPAT)
    if (is_compat_task()) 
    {
        BCM_IOC_PTR_ZERO_EXT(ctrlParms.inout_data);
    }
#endif

    SOTP_DBG("%s: SOTP_IOCTL_%s Elem:%d bufp:%p\n", __FUNCTION__, (command==SOTP_IOCTL_GET?"GET":"SET"), ctrlParms.element, ctrlParms.inout_data);

    if (!ctrlParms.inout_data) 
    {
        SOTP_ERR_PRINT("inout_data is null\n");
        return ret;
    }

    bufp = kmalloc(ctrlParms.data_len, GFP_KERNEL);

    if (!bufp) 
    {
        SOTP_ERR_PRINT("Kmalloc failed\n");
        return -ENOMEM;
    }


    switch (command) {
    case SOTP_IOCTL_GET:
        switch (ctrlParms.element) {
        case SOTP_MAP:
            ret = sotp_dump_map( &ctrlParms.result );
            break;
        case SOTP_ROW:
            ret = sotp_get_row_data( ctrlParms.row_addr, bufp, ctrlParms.data_len, &ctrlParms.result, ctrlParms.raw_access );
            break;
        case SOTP_REGION_READLOCK:
            ret = sotp_get_region_readlock_status( ctrlParms.region_num, &ctrlParms.result );
            break;
        case SOTP_REGION_FUSELOCK:
            ret = sotp_get_region_fuselock_status( ctrlParms.region_num, &ctrlParms.result );
            break;
        case SOTP_KEYSLOT:
            ret = sotp_get_keyslot_data( ctrlParms.keyslot_section_num, bufp, ctrlParms.data_len, &ctrlParms.result );
            break;
        case SOTP_KEYSLOT_READLOCK:
            ret = sotp_get_keyslot_readlock_status( ctrlParms.keyslot_section_num, &ctrlParms.result );
            break;
        default:
            ret = -1;
        }

        /* Copy SOTP data */
        copy_to_user(ctrlParms.inout_data, bufp, ctrlParms.data_len);

        break;

    case SOTP_IOCTL_SET:
        /* Copy user data */
        if( ctrlParms.data_len )
        {
            if (copy_from_user(bufp, ctrlParms.inout_data, ctrlParms.data_len) != 0) 
            {
                SOTP_ERR_PRINT("%s: Failed to copy user data buf!\n", __FUNCTION__);
                kfree(bufp);
                return ret;
            }
        }  

        switch (ctrlParms.element) {
        case SOTP_ROW:
            ret = sotp_set_row_data( ctrlParms.row_addr, ctrlParms.inout_data, ctrlParms.data_len, &ctrlParms.result, ctrlParms.raw_access);
            break;
        case SOTP_REGION_READLOCK:
            ret = sotp_set_region_readlock( ctrlParms.region_num, &ctrlParms.result );
            break;
        case SOTP_REGION_FUSELOCK:
            ret = sotp_set_region_fuselock( ctrlParms.region_num, &ctrlParms.result );
            break;
        case SOTP_KEYSLOT:
            ret = sotp_set_keyslot_data( ctrlParms.keyslot_section_num, ctrlParms.inout_data, ctrlParms.data_len, &ctrlParms.result );
            break;
        case SOTP_KEYSLOT_READLOCK:
            ret = sotp_set_keyslot_readlock( ctrlParms.keyslot_section_num, &ctrlParms.result );
            break;
        default:
            ret = -1;
        }
        break;
    default:
        break;
    }

    copy_to_user((void *)arg, (void *) &ctrlParms, sizeof(ctrlParms));

    return (ret);
} 

static long unlocked_sotp_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct inode *inode;
    long rt;

    inode = file_inode(filep);
    mutex_lock(&sotpIoctlMutex);
    rt = sotp_ioctl( inode, filep, cmd, arg );
    mutex_unlock(&sotpIoctlMutex);
    
    return rt;
}

static int is_sotp_accessable(void)
{
    if( SOTP->sotp_otp_prog_ctrl == 0xDEADBEEF && SOTP->sotp_otp_ctrl_0 == 0xDEADBEEF )
    {
        return -EPERM;
    }

    return 0;
}

static int sotp_open(struct inode *inode, struct file *filp)
{
    if( !sotp.sotp_base )
        return -EINVAL;
    else
        return( 0 );
}

static int sotp_release(struct inode *inode, struct file *filp)
{
    if( !sotp.sotp_base )
        return -EINVAL;
    else
        return( 0 );
}

static int __init brcm_sotp_init(void)
{
    int ret;

    memset(&sotp, 0, sizeof(SOTP_CTX));

    ret = register_chrdev(SOTP_DRV_MAJOR, "sotp", &sotp_fops );
    if (ret < 0)
    {
        SOTP_ERR_PRINT( "%s(major %d): failed to register device.\n",__FUNCTION__, SOTP_DRV_MAJOR);
        goto quit;
    }

    /* Get SOTP base address */
    sotp.sotp_base = (uintptr_t)SOTP_BASE;

    /* Initialize SOTP core */
    ret = sotp_init((void*)sotp.sotp_base);

    if (ret < 0)
    {
        SOTP_ERR_PRINT( "%s(major %d): SOTP initialization failed. Not loading driver.\n",__FUNCTION__, SOTP_DRV_MAJOR);
        goto quit;
    }

    ret = is_sotp_accessable();
    if (ret < 0)
    {
        SOTP_ERR_PRINT( "%s(major %d): SOTP access not permitted. Not loading driver.\n",__FUNCTION__, SOTP_DRV_MAJOR);
        goto quit;
    }

    SOTP_ERR_PRINT( "%s(major %d): SOTP loaded succesfully!.\n",__FUNCTION__, SOTP_DRV_MAJOR);
    sotp.sotp_major = SOTP_DRV_MAJOR;
quit:
   return ret;
}

static void __exit brcm_sotp_cleanup(void)
{
    SOTP_DBG("%s:\n", __FUNCTION__);     
}


/***************************************************************************
* MACRO to call driver initialization and cleanup functions.
***************************************************************************/
module_init(brcm_sotp_init);
module_exit(brcm_sotp_cleanup);

MODULE_LICENSE("proprietary");
MODULE_DESCRIPTION("Broadcom SOTP driver");

