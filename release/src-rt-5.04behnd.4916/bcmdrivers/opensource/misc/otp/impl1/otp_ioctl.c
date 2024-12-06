/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
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
// BCMFORMAT: notabs reindent:uncrustify:bcm_minimal_i4.cfg
/***************************************************************************
 * File Name  : otp_ioctl.c
 *
 * Description: provides IOCTLS to provide otp read/write to userspace
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
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/of_fdt.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>

#include <bcm_otp.h>
#include <bcm_otp_map.h>
#include "otp_ioctl.h"
#include <linux/bcm_log.h>

/* Defines. */
#define OTP_DEBUG_IF    0

#ifndef OTP_CUST_BTRM_BOOT_ENABLE_FUSE_ROW
#define OTP_CUST_BTRM_BOOT_ENABLE_FUSE_ROW OTP_CUST_BTRM_BOOT_ENABLE_ROW
#endif

#ifndef OTP_CUST_MFG_MRKTID_FUSE_ROW
#define OTP_CUST_MFG_MRKTID_FUSE_ROW OTP_CUST_MFG_MRKTID_ROW
#endif

/* Prototypes. */
static int otp_open(struct inode *inode, struct file *filp);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
static int otp_ioctl(struct inode *inode, struct file *filp, unsigned int command, unsigned long arg);
#endif
static int otp_release(struct inode *inode, struct file *filp);
static int otp_fuse_bootrom_enable(void);
static int otp_fuse_operator_enable(void);
static int otp_fuse_mid(uint32_t id);
static int otp_fuse_oid(uint32_t id);
static int otp_get_bootrom_enable(uint32_t *pRes);
static int otp_get_operator_enable(uint32_t *pRes);
static int otp_get_mid(uint32_t *pRes);
static int otp_get_oid(uint32_t *pRes);

static int otp_fuse_jtag_lock(uint32_t value);
static int otp_fuse_jtag_pwd(uint64_t value);
static int otp_fuse_chip_serial_num(uint32_t value);
static int otp_fuse_jtag_pwd_rdlock(void);
static int otp_fuse_cust_lock(void);
static int otp_get_jtag_lock(uint32_t *pRes);
static int otp_get_jtag_pwd_lock(uint32_t * pRes);
static int otp_get_jtag_permalock(uint32_t * pRes);
static int otp_get_jtag_pwd(uint64_t *pRes);
static int otp_get_chip_serial_num(uint32_t *pRes);
static int otp_get_jtag_pwd_rdlock(uint32_t *pRes);
static int otp_get_cust_lock(uint32_t *pRes);
int bcm_otp_ioctl_init(struct platform_device *pdev);
void __exit brcm_otp_cleanup(void);

/* Globals */
#define OTP_DEV_NAME "otp"
struct class *otp_class = NULL;
struct device *otp_dev = NULL;
struct cdev otp_cdev;
int otp_major = 0;

/****************************************************************
 * OTP specific feature  functions                              *
 ****************************************************************/
static int otp_fuse_jtag_lock(uint32_t value)
{
    return( bcm_otp_fuse_row(OTP_JTAG_MODE_ROW, (value & OTP_JTAG_MODE_MASK) << OTP_JTAG_MODE_REG_SHIFT) );
}

static int otp_fuse_jtag_pwd(uint64_t value)
{
    int ret = -1;
    ret = bcm_otp_fuse_row(OTP_JTAG_PWD_ROW_1, (uint32_t)value & OTP_JTAG_PWD_MASK_1);
    if( !ret )
        ret = bcm_otp_fuse_row(OTP_JTAG_PWD_ROW_2, (uint32_t)((value >> OTP_JTAG_PWD_SHIFT_2) & OTP_JTAG_PWD_MASK_2));
#ifdef OTP_TYPE_V1
    /* Type V1 devices store the jtag password in 3 25-bit fields */
    if( !ret )
        ret = bcm_otp_fuse_row(OTP_JTAG_PWD_ROW_3, (uint32_t)((value >> OTP_JTAG_PWD_SHIFT_3) & OTP_JTAG_PWD_MASK_3));
#endif            
    return ret; 
}

static int otp_fuse_chip_serial_num(uint32_t value)
{
    int ret = -1;
#ifdef OTP_TYPE_V1
    /* Type V1 devices store the jtag serial number in 2 fields */
    ret = bcm_otp_fuse_row(OTP_JTAG_SER_NUM_ROW_1, (value & OTP_JTAG_SER_NUM_MASK_1) << OTP_JTAG_SER_NUM_REG_SHIFT_1);
    if( !ret )
        ret = bcm_otp_fuse_row(OTP_JTAG_SER_NUM_ROW_2, (value >> OTP_JTAG_SER_NUM_SHIFT_2) & OTP_JTAG_SER_NUM_MASK_2);
#else /* OTP_TYPE_V2 OTP_TYPE_V3 */
    /* Type V2 V3 devices store the jtag serial number in 1 32-bit field */
    ret = bcm_otp_fuse_row(OTP_JTAG_SER_NUM_ROW_1, value);
#endif            
    return ret;
}

static int otp_fuse_jtag_pwd_rdlock(void)
{
    return( bcm_otp_fuse_row(OTP_JTAG_PWD_RDLOCK_ROW, 1 << OTP_JTAG_PWD_RDLOCK_REG_SHIFT) );
}

static int otp_fuse_cust_lock(void)
{
    return( bcm_otp_fuse_row(OTP_JTAG_CUST_LOCK_ROW, OTP_JTAG_CUST_LOCK_VAL << OTP_JTAG_CUST_LOCK_REG_SHIFT) );
}

static int otp_get_jtag_lock(uint32_t * pRes)
{
    int ret = 0;
    ret = bcm_otp_get_row(OTP_JTAG_MODE_ROW, pRes );
    *pRes = ( *pRes >> OTP_JTAG_MODE_REG_SHIFT ) & OTP_JTAG_MODE_MASK;
    return ret;
}

static int otp_get_jtag_pwd_lock(uint32_t * pRes)
{
    int ret = 0;
    int i;
    int count = 0;
    ret = otp_get_jtag_lock( pRes );

#ifdef OTP_TYPE_V3
    (void) i;
    (void) count;
    if( *pRes == OTP_JTAG_MODE_LOCK )
        *pRes = 1;
    else 
        *pRes = 0;
#else
    /* Get number of '1's in first half of jtag mode field */
    for( i=0; i<(OTP_JTAG_MODE_NUM_BITS/2); i++)
    {
        if( *pRes & 1 )
            count++;

        *pRes = *pRes >> 1;
    }

    /* Check if first half of jtag mode field have majority '0's */
    if( count < OTP_JTAG_MODE_MAJORITY_BIT_CNT )
    {
        /* Check if last half of jtag mode field have majority '1's */
        count = 0;
        for( ; i<OTP_JTAG_MODE_NUM_BITS; i++)
        {
            if( *pRes & 1 )
                count++;

            *pRes = *pRes >> 1;
        }
        if( count >= OTP_JTAG_MODE_MAJORITY_BIT_CNT )
            *pRes = 1;
        else
            *pRes = 0;
    }
    else
        *pRes = 0;
#endif

    return ret;
}

static int otp_get_jtag_permalock(uint32_t * pRes)
{
    int ret = 0;
    int count = 0;
    int i;

    ret = otp_get_jtag_lock( pRes );
#ifdef OTP_TYPE_V3
    (void) i;
    (void) count;
    if( *pRes == OTP_JTAG_MODE_PERMALOCK )
        *pRes = 1;
    else 
        *pRes = 0;
#else
    /* Get number of '1's in first half of jtag mode field */
    for( i=0; i<(OTP_JTAG_MODE_NUM_BITS/2); i++)
    {
        if( *pRes & 1 )
            count++;

        *pRes = *pRes >> 1;
    }

    /* Check if first half of jtag mode field have majority '1's */
    if( count >= OTP_JTAG_MODE_MAJORITY_BIT_CNT )
    {
        /* Check if last half of jtag mode field have majority '1's */
        count = 0;
        for( ; i<OTP_JTAG_MODE_NUM_BITS; i++)
        {
            if( *pRes & 1 )
                count++;

            *pRes = *pRes >> 1;
        }
        if( count >= OTP_JTAG_MODE_MAJORITY_BIT_CNT )
            *pRes = 1;
        else
            *pRes = 0;
    }
    else
        *pRes = 0;
#endif	

    return ret;
}

static int otp_get_jtag_pwd(uint64_t * pRes)
{
    int ret = -1;
    uint32_t temp_pwd;
    *pRes = 0;

    ret = bcm_otp_get_row(OTP_JTAG_PWD_ROW_1, &temp_pwd );
    if( ret )
        goto exit_pwd;

    *pRes |= (uint64_t)(temp_pwd & OTP_JTAG_PWD_MASK_1);
    ret = bcm_otp_get_row(OTP_JTAG_PWD_ROW_2, &temp_pwd );
    if( ret )
        goto exit_pwd;

    *pRes |= ((uint64_t)(temp_pwd & OTP_JTAG_PWD_MASK_2) << OTP_JTAG_PWD_SHIFT_2) ;

#ifdef OTP_TYPE_V1
    ret = bcm_otp_get_row(OTP_JTAG_PWD_ROW_3, &temp_pwd );
    if( ret )
        goto exit_pwd;

    *pRes |= ((uint64_t)(temp_pwd & OTP_JTAG_PWD_MASK_3) << OTP_JTAG_PWD_SHIFT_3) ;
#endif

exit_pwd:
    return ret;
}

static int otp_get_chip_serial_num(uint32_t * pRes)
{
    int ret = -1;
#ifdef OTP_TYPE_V1
    uint32_t temp_id;
    *pRes = 0;
    ret = bcm_otp_get_row(OTP_JTAG_SER_NUM_ROW_1, &temp_id );
    if( ret )
        goto exit_id;

    *pRes |= (temp_id & OTP_JTAG_PWD_MASK_1) >> OTP_JTAG_SER_NUM_REG_SHIFT_1;
    ret = bcm_otp_get_row(OTP_JTAG_SER_NUM_ROW_2, &temp_id );
    if( ret )
        goto exit_id;

    *pRes |= (temp_id & OTP_JTAG_SER_NUM_MASK_2) << OTP_JTAG_SER_NUM_SHIFT_2 ;
exit_id:
#else /* OTP_TYPE_V2 OTP_TYPE_V3 */
    ret = bcm_otp_get_row(OTP_JTAG_SER_NUM_ROW_1, pRes );
#endif
    return ret;
}

static int otp_get_jtag_pwd_rdlock(uint32_t * pRes)
{
    int ret = 0;
    ret = bcm_otp_get_row(OTP_JTAG_PWD_RDLOCK_ROW, pRes );
    *pRes = ( *pRes & (1 << OTP_JTAG_PWD_RDLOCK_REG_SHIFT) );  
    if( *pRes )
        *pRes = 1;
    else
        *pRes = 0;
    return ret;
}

static int otp_get_cust_lock(uint32_t * pRes)
{
    int ret = 0;
    int count = 0;
    ret = bcm_otp_get_row(OTP_JTAG_CUST_LOCK_ROW, pRes );
    *pRes = ( *pRes >> OTP_JTAG_CUST_LOCK_REG_SHIFT) & OTP_JTAG_CUST_LOCK_VAL;

    /* Check if atleast 3 bits are set */
    while( *pRes )
    {
        if( *pRes & 0x01 )
            count++;

        *pRes = *pRes >> 1;
    }

    if( count >= 3 )
        *pRes = 1;
    else
        *pRes = 0;

    return ret;
}

static int otp_fuse_bootrom_enable(void)
{
    int ret = 0;
    ret = bcm_otp_fuse_row(OTP_CUST_BTRM_BOOT_ENABLE_FUSE_ROW, OTP_CUST_BTRM_BOOT_ENABLE_MASK);
    return ret;
}

static int otp_fuse_operator_enable(void)
{
    int ret = 0;
    /* Only for GEN2 devices */
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    ret = bcm_otp_fuse_row(OTP_CUST_OP_INUSE_FUSE_ROW, OTP_CUST_OP_INUSE_MASK);
#endif   
    return ret;
}

static int otp_fuse_mid(uint32_t id)
{
    int ret = 0;
    ret = bcm_otp_fuse_row(OTP_CUST_MFG_MRKTID_FUSE_ROW, ((id << OTP_CUST_MFG_MRKTID_SHIFT) & OTP_CUST_MFG_MRKTID_MASK));
    return ret;
}

static int otp_fuse_oid(uint32_t id)
{
    int ret = 0;
    /* Only for GEN2 devices */
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    ret = bcm_otp_fuse_row(OTP_CUST_OP_MRKTID_FUSE_ROW, ((id << OTP_CUST_OP_MRKTID_SHIFT) & OTP_CUST_OP_MRKTID_MASK));
#endif   
    return ret;
}

static int otp_get_bootrom_enable(uint32_t *pRes)
{
    int ret = 0;
    ret = bcm_otp_get_row(OTP_CUST_BTRM_BOOT_ENABLE_ROW, pRes);
    *pRes = (*pRes & OTP_CUST_BTRM_BOOT_ENABLE_MASK) >> OTP_CUST_BTRM_BOOT_ENABLE_SHIFT;
    return ret;
}

static int otp_get_operator_enable(uint32_t *pRes)
{
    int ret = 0;
    /* Only for GEN2 devices */
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    ret = bcm_otp_get_row(OTP_CUST_OP_INUSE_ROW, pRes);
    *pRes = (*pRes & OTP_CUST_OP_INUSE_MASK) >> OTP_CUST_OP_INUSE_SHIFT;
#endif
    return ret;
}

static int otp_get_mid(uint32_t *pRes)
{
    int ret = 0;
    ret = bcm_otp_get_row(OTP_CUST_MFG_MRKTID_ROW, pRes);
    *pRes = (*pRes & OTP_CUST_MFG_MRKTID_MASK) >> OTP_CUST_MFG_MRKTID_SHIFT;
    return ret;
}

static int otp_get_oid(uint32_t *pRes)
{
    int ret = 0;
    /* Only for GEN2 devices */
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    ret = bcm_otp_get_row(OTP_CUST_OP_MRKTID_ROW, pRes);
    *pRes = (*pRes & OTP_CUST_OP_MRKTID_MASK) >> OTP_CUST_OP_MRKTID_SHIFT;
#endif   
    return ret;
}

/*************************************************************************
 * OTP driver functions                                                  *
 *************************************************************************/
void __exit brcm_otp_cleanup(void)
{
}


static int otp_open(struct inode *inode, struct file *filp)
{

    return( 0 );
}

static int otp_release(struct inode *inode, struct file *filp)
{

    return( 0 );
}

static int otp_ioctl(struct inode *inode, struct file *flip,
    unsigned int command, unsigned long arg)
{
    int ret = -EINVAL, r=0;
    OTP_IOCTL_PARMS ctrlParms;
    uint32_t val32 = 0;
    uint64_t val64 = 0;

    switch (command) {
    case OTP_IOCTL_GET:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
        {
            switch (ctrlParms.element) {
            case OTP_BTRM_ENABLE_BIT:
                ret = otp_get_bootrom_enable(&val32);
                ctrlParms.value = val32;
                break;
            case OTP_OPERATOR_ENABLE_BIT:
                ret = otp_get_operator_enable(&val32);
                ctrlParms.value = val32;
                break;
            case OTP_MID_BITS:
                ret = otp_get_mid(&val32);
                ctrlParms.value = val32;
                break;
            case OTP_OID_BITS:
                ret = otp_get_oid(&val32);
                ctrlParms.value = val32;
                break;
            case OTP_CHIP_SERIAL_NUM:
                ret = otp_get_chip_serial_num(&val32);
                ctrlParms.value = val32;
                break;
            case OTP_JTAG_PWD_LOCK:
                ret = otp_get_jtag_pwd_lock(&val32);
                ctrlParms.value = val32;
                break;
            case OTP_JTAG_PERMLOCK:
                ret = otp_get_jtag_permalock(&val32);
                ctrlParms.value = val32;
                break;
            case OTP_JTAG_PWD:
                ret = otp_get_jtag_pwd(&val64);
                ctrlParms.value = val64;
                break;
            case OTP_JTAG_PWD_RDLOCK:
                ret = otp_get_jtag_pwd_rdlock(&val32);
                ctrlParms.value = val32;
                break;
            case OTP_CUST_LOCK:
                ret = otp_get_cust_lock(&val32);
                ctrlParms.value = val32;
                break;
            case OTP_ROW:
                ret = bcm_otp_get_row(ctrlParms.id, &val32);
                ctrlParms.value = val32;
                break;
            default:
                ret = -1;
            }
        }

        break;

    case OTP_IOCTL_SET:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
        {
            val32 = (uint32_t)ctrlParms.value;
            val64 = ctrlParms.value;

            switch (ctrlParms.element) {
            case OTP_BTRM_ENABLE_BIT:
                ret = otp_fuse_bootrom_enable();
                break;
            case OTP_OPERATOR_ENABLE_BIT:
                ret = otp_fuse_operator_enable();
                break;
            case OTP_MID_BITS:
                ret = otp_fuse_mid(val32);
                break;
            case OTP_OID_BITS:
                ret = otp_fuse_oid(val32);
                break;
            case OTP_CHIP_SERIAL_NUM:
                ret = otp_fuse_chip_serial_num(val32);
                break;
            case OTP_JTAG_PWD_LOCK:
                ret = otp_fuse_jtag_lock(OTP_JTAG_MODE_LOCK);
                break;
            case OTP_JTAG_PERMLOCK:
                ret = otp_fuse_jtag_lock(OTP_JTAG_MODE_PERMALOCK);
                break;
            case OTP_JTAG_PWD:
                ret = otp_fuse_jtag_pwd(val64);
                break;
            case OTP_JTAG_PWD_RDLOCK:
                ret = otp_fuse_jtag_pwd_rdlock();
                break;
            case OTP_CUST_LOCK:
                ret = otp_fuse_cust_lock();
                break;
            case OTP_ROW:
                ret = bcm_otp_fuse_row(ctrlParms.id, val32);
                break;
            default:
                ret = -1;
            }
        }

        break;
    default:
        break;
    }

    ctrlParms.result = ret;
    r=copy_to_user((void *)arg, (void *) &ctrlParms, sizeof(ctrlParms));

    return (ret);

} /* board_ioctl */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static DEFINE_MUTEX(otpIoctlMutex);

static long unlocked_otp_ioctl(struct file *filep, unsigned int cmd, 
    unsigned long arg)
{
    struct inode *inode;
    long rt;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
    inode = filep->f_dentry->d_inode;
#else
    inode = file_inode(filep);
#endif

    mutex_lock(&otpIoctlMutex);
    rt = otp_ioctl( inode, filep, cmd, arg );
    mutex_unlock(&otpIoctlMutex);

    return rt;
}
#endif

static struct file_operations otp_fops =
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
    .unlocked_ioctl 	= unlocked_otp_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = unlocked_otp_ioctl,
#endif
#else
    .ioctl   = otp_ioctl,
#endif
    .open    = otp_open,
    .release = otp_release,
};

int bcm_otp_ioctl_init(struct platform_device *pdev)
{
    dev_t dev = 0;
    dev_t devno;
    int ret;

    ret = alloc_chrdev_region(&dev, 0, 1, OTP_DEV_NAME);
    if (ret < 0) {
        pr_err("%s:alloc_chrdev_region() failed\n", __func__);
        return -ENODEV;
    }
    otp_major = MAJOR(dev);

    /* create device and class */
    otp_class = class_create(THIS_MODULE, OTP_DEV_NAME);
    if (IS_ERR(otp_class)) {
        ret = PTR_ERR(otp_class);
        pr_err("%s:Fail to create class %s, ret = %d\n", __func__,
               OTP_DEV_NAME, ret);
        goto fail_free_chrdev_region;
    }

    devno = MKDEV(otp_major, 0);
    cdev_init(&otp_cdev, &otp_fops);
    otp_cdev.owner = THIS_MODULE;

    ret = cdev_add(&otp_cdev, devno, 1);
    if (ret) {
        pr_err("%s:Fail to add cdev %s, ret = %d\n", __func__,
               OTP_DEV_NAME, ret);
        goto fail_free_class;
    }

    otp_dev = device_create(otp_class, NULL, devno, NULL, OTP_DEV_NAME);
    if (IS_ERR(otp_dev)) {
        ret = PTR_ERR(otp_dev);
        pr_err("%s:Fail to create device %s, ret = %d\n", __func__,
               OTP_DEV_NAME, ret);
        goto fail_free_cdev;
    }

    printk("brcm_otp_init entry\n");
    return ret;

fail_free_cdev:

    cdev_del(&otp_cdev);

fail_free_class:

    class_destroy(otp_class);

fail_free_chrdev_region:

    unregister_chrdev_region(MKDEV(otp_major, 0), 1);

    printk("brcm_otp_init: failed to register device.\n");
    return ret;
}

