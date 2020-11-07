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
* File Name  : otp.c
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
 

#include <bcm_map.h>
#include <otp_ioctl.h>
#include <linux/bcm_log.h>
#include <bcm_otp.h>

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
static int __init brcm_otp_init(void);
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
void __exit brcm_otp_cleanup(void);

/* Globals */
uint32_t otp_major = 0;

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
#ifdef OTP_TYPE_B
    /* Type B devices store the jtag password in 3 25-bit fields */
    if( !ret )
        ret = bcm_otp_fuse_row(OTP_JTAG_PWD_ROW_3, (uint32_t)((value >> OTP_JTAG_PWD_SHIFT_3) & OTP_JTAG_PWD_MASK_3));
#endif            
    return ret; 
}

static int otp_fuse_chip_serial_num(uint32_t value)
{
    int ret = -1;
#ifdef OTP_TYPE_B
    /* Type C devices store the jtag serial number in 2 fields */
    ret = bcm_otp_fuse_row(OTP_JTAG_SER_NUM_ROW_1, (value & OTP_JTAG_SER_NUM_MASK_1) << OTP_JTAG_SER_NUM_REG_SHIFT_1);
    if( !ret )
        ret = bcm_otp_fuse_row(OTP_JTAG_SER_NUM_ROW_2, (value >> OTP_JTAG_SER_NUM_SHIFT_2) & OTP_JTAG_SER_NUM_MASK_2);
#else /* OTP_TYPE_C */
    /* Type C devices store the jtag serial number in 1 32-bit field */
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
    int count = 0;
    ret = otp_get_jtag_lock( pRes );

    /* Jtag pwd lock is set majority voting of the last 3 bits of the jtag mode field */
    *pRes = *pRes >> 3;
    while( *pRes )
    {
        if( *pRes & 1 )
            count++;

        *pRes = *pRes >> 1;
    }

    if( count >= 2 )
        *pRes = 1;
    else
        *pRes = 0;

    return ret;
}

static int otp_get_jtag_permalock(uint32_t * pRes)
{
    int ret = 0;
    int count = 0;
    int i;
    int jtag_pwd_lock = 0;
    /* Jtag pwd lock is set majority voting of the 1st and last 3 bits of the jtag mode field */

    /* Check if last 3 bits of jtag mode field have majority '1's */
    ret = otp_get_jtag_pwd_lock( &jtag_pwd_lock );
    if(!jtag_pwd_lock)
    {
        *pRes = 0;
        return ret;
    }

    /* Check if first 3 bits of jtag mode field have majority '1's */
    ret = otp_get_jtag_lock( pRes );
    for( i=0; i<3; i++)
    {
        if( *pRes & 1 )
            count++;

        *pRes = *pRes >> 1;
    }

    if( count >= 2 )
        *pRes = 1;
     else
        *pRes = 0;

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
    
#ifdef OTP_TYPE_B
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
#ifdef OTP_TYPE_B
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
#else /* OTP_TYPE_C */       
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
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96838)   
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
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96838)   
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
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96838)   
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
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96838)   
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
    int ret = -EINVAL;
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
    copy_to_user((void *)arg, (void *) &ctrlParms, sizeof(ctrlParms));

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



static int __init brcm_otp_init(void)
{
    int ret;

    ret = register_chrdev(OTP_DRV_MAJOR, "otp", &otp_fops );
    if (ret < 0)
        printk( "brcm_otp_init(major %d): failed to register device.\n",OTP_DRV_MAJOR);
    else
    {
        printk("brcm_otp_init entry\n");
        otp_major = OTP_DRV_MAJOR;
    }

   return ret;
}

/***************************************************************************
* MACRO to call driver initialization and cleanup functions.
***************************************************************************/
module_init(brcm_otp_init);
module_exit(brcm_otp_cleanup);

MODULE_LICENSE("proprietary");

