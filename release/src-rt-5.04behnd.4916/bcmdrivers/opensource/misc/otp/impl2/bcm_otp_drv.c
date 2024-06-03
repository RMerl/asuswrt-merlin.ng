/*
 * <:copyright-BRCM:2022:DUAL/GPL:standard
 * 
 *    Copyright (c) 2022 Broadcom 
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

#include <linux/string.h>
#include <bcm_strap_drv.h>
#include <linux/module.h>
#include <linux/of_fdt.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include "bcm_otp.h"
#include "shared_utils.h"
#include "otp_ioctl.h"
#include "itc_rpc.h"

#define OTP_RPC_TUNNEL_NAME "rg-smc"
#define OTP_DEV_NAME "otp"

int otp_rpc_tunnel_id;
struct device *otp_dev = NULL;
struct class *otp_class = NULL;
struct cdev otp_cdev;
int otp_major = 0;

otp_map_cmn_err_t bcm_otp_commit(void);

/************************************************************
 *  int bcm_otp_fuse_row 
 *  Input parameters: 
 *     row   - Row address
 *     value - 32-bit OTP value
 *  Return value:
 *      returns 0 if successful
 ***********************************************************/
int bcm_otp_fuse_row(int row, unsigned int val)
{
    return 0;
}
EXPORT_SYMBOL(bcm_otp_fuse_row);

/************************************************************
 *  int bcm_otp_fuse_row_ecc 
 *  Input parameters: 
 *     row   - Row address
 *     ecc   - row 7 bit ecc  
 *     value - 32-bit OTP value
 *  Return value:
 *      returns 0 if successful
 ***********************************************************/
int bcm_otp_fuse_row_ecc(int row, unsigned int val, unsigned int ecc)
{
    return 0;
}
EXPORT_SYMBOL(bcm_otp_fuse_row_ecc);

int bcm_otp_get_row(int row, unsigned int* val)
{
    return 0;
}
EXPORT_SYMBOL(bcm_otp_get_row);

/***********************************************************
 *  int bcm_otp_get_row_ecc
 *  Input parameters: 
 *     row    - Row address
 *     *val - Pointer to 32-bit OTP value
 *     *val_hi - Pointer to 32-bit OTP hi word data (typically ecc)
 *  Return value:
 *      returns 0 if successful, value in *value
 ***********************************************************/
int bcm_otp_get_row_ecc(int row, unsigned int* val, unsigned int* val_hi)
{

    return 0;
}
EXPORT_SYMBOL(bcm_otp_get_row_ecc);

/************************************************************
 *  OTP utility functions                                   *
 ************************************************************/
int bcm_is_btrm_boot(void)
{
    int rval = 0;

    return rval;
}
EXPORT_SYMBOL(bcm_is_btrm_boot);

int bcm_otp_is_boot_secure(void)
{
    int rval = 0;
    return rval;
}
EXPORT_SYMBOL(bcm_otp_is_boot_secure);

int bcm_otp_is_boot_mfg_secure(void)
{
    return 0;
}
EXPORT_SYMBOL(bcm_otp_is_boot_mfg_secure);

#if defined(OTP_SGMII_DISABLE_ROW)
int bcm_otp_is_sgmii_disabled(unsigned int* val)
{
    return 0;
}
EXPORT_SYMBOL(bcm_otp_is_sgmii_disabled);
#endif

#if defined(OTP_CPU_CLOCK_FREQ_ROW)
int bcm_otp_get_cpu_clk(unsigned int* val)
{
    return 0;
}
EXPORT_SYMBOL(bcm_otp_get_cpu_clk);
#endif

int bcm_otp_is_usb3_disabled(unsigned int* val)
{
    int rval = 0; 
    return rval;
}
EXPORT_SYMBOL(bcm_otp_is_usb3_disabled);


int bcm_otp_is_sata_disabled(unsigned int* val)
{
    return 0;
}
EXPORT_SYMBOL(bcm_otp_is_sata_disabled);

int bcm_otp_get_nr_cpus(unsigned int* val)
{
    int rval = 0;

    return rval;
}
EXPORT_SYMBOL(bcm_otp_get_nr_cpus);

#if defined(OTP_PMC_BOOT_ROW)
int bcm_otp_get_pmc_boot_sts(unsigned int* val)
{
    return 0;
}
EXPORT_SYMBOL(bcm_otp_get_pmc_boot_sts);
#endif

#if defined(OTP_PCM_DISABLE_ROW)
int bcm_otp_is_pcm_disabled(unsigned int* val)
{
    return 0;
}
EXPORT_SYMBOL(bcm_otp_is_pcm_disabled);
#endif

int bcm_otp_is_pcie_port_disabled(unsigned int pcie_port_num, unsigned int* val)
{

    return 0;
}
EXPORT_SYMBOL(bcm_otp_is_pcie_port_disabled);

int bcm_otp_get_ldo_trim(unsigned int* val)
{
    return 0;
}

int bcm_otp_is_rescal_enabled(unsigned int* val)
{
    int rval = 0;
    return rval;
}


int bcm_otp_get_dgasp_trim(unsigned int* val)
{
    return 0;
}
EXPORT_SYMBOL(bcm_otp_get_dgasp_trim);

int bcm_otp_auth_prog_mode(void)
{
    int ret = -1;

    ret = 0;
    return ret;
}
EXPORT_SYMBOL(bcm_otp_auth_prog_mode);

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
    int ret = 0;
    unsigned long must_check;
    OTP_IOCTL_PARMS ctrlParms;
    u32 val_size;

    switch (command) 
	{
    case OTP_IOCTL_GET:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
        {
            uint32_t *val32 = 0;
            uint64_t *val64 = 0;
            switch (ctrlParms.element) 
		    {
            case OTP_MRKID:
                ret = bcm_otp_read(OTP_MAP_CUST_MFG_MRKTID, &val32, &val_size);
                if(!ret)
                {
                    ctrlParms.value = *val32;
                }
                break;
            case OTP_CSEC_CHIPID:
                ret = bcm_otp_read(OTP_MAP_CSEC_CHIPID, &val32, &val_size);
                if(!ret)
                {
                    ctrlParms.value = *val32;
                }
                break;
            case OTP_LEDS_SETTINGS:
                ret = bcm_otp_read(OTP_MAP_LEDS, (u32**)&val64, &val_size);
                if(!ret)
                {
                    ctrlParms.value = *val64;
                }
                break;
            default:
                ret = -1;
            }
        }
        break;

    case OTP_IOCTL_SET:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
        {
            uint32_t val32 = (uint32_t)ctrlParms.value;
            uint64_t val64 = ctrlParms.value;

            switch (ctrlParms.element) 
            {
            case OTP_MRKID:
                ret = bcm_otp_write(OTP_MAP_CUST_MFG_MRKTID, &val32, otp_feat_info_get_size(OTP_MAP_CUST_MFG_MRKTID));
                break;
            case OTP_JTAG_PWD:
                ret = bcm_otp_write(OTP_MAP_JTAG_PWD, (u32*)&val64, sizeof(val64));
                break;
            case OTP_CSEC_CHIPID:
                ret = bcm_otp_write(OTP_MAP_CSEC_CHIPID, &val32, sizeof(val32));
                break;
            case OTP_LEDS_SETTINGS:
                ret = bcm_otp_write(OTP_MAP_LEDS, (u32*)&val64, otp_feat_info_get_size(OTP_MAP_LEDS));
	                break;
            case OTP_CU_LOCK:
            {
                val32 = 1;
                ret = bcm_otp_write(OTP_MAP_DBG_MODE, (const u32*)&val32, sizeof(u32));
                if (ret) 
                {
                    break;
                }
                ret = bcm_otp_write(OTP_MAP_JU_MODE, (const u32*)&val32, sizeof(u32));
                break;
            }
            case OTP_COMMIT:
                ret = bcm_otp_commit();
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
    must_check = copy_to_user((void *)arg, (void *) &ctrlParms, sizeof(ctrlParms));

    return (ret);

} /* board_ioctl */

static DEFINE_MUTEX(otpIoctlMutex);

static long unlocked_otp_ioctl(struct file *filep, unsigned int cmd, 
    unsigned long arg)
{
    struct inode *inode;
    long rt;

    inode = file_inode(filep);

    mutex_lock(&otpIoctlMutex);
    rt = otp_ioctl( inode, filep, cmd, arg );
    mutex_unlock(&otpIoctlMutex);

    return rt;
}

static struct file_operations otp_fops =
{
    .unlocked_ioctl 	= unlocked_otp_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = unlocked_otp_ioctl,
#endif
    .open    = otp_open,
    .release = otp_release,
};

int bcm_otp_ioctl_init(struct platform_device *pdev)
{
    dev_t dev = 0;
    dev_t devno;
    int ret;

    otp_rpc_tunnel_id = rpc_get_fifo_tunnel_id(OTP_RPC_TUNNEL_NAME);
    if (RPC_INVALID_TUNNEL == otp_rpc_tunnel_id) 
    {
        printk("bcm_otp_ioctl_init(major %d): No tunnel for RPC request\n", OTP_DRV_MAJOR);
        return (-ENXIO);
    }

    ret = bcm_otp_init();
    if(ret)
    {
        printk("bcm_otp_ioctl_init(major %d): bcm_otp_init failed .\n",OTP_DRV_MAJOR);
        return ret;
    }

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

static struct of_device_id const bcm_otp_drv_of_match[] = {
    { .compatible = "brcm,otp",},
    {}
};

MODULE_DEVICE_TABLE(of, bcm_otp_drv_of_match);

static struct platform_driver bcm_otp_driver = {
    .driver = {
        .name = "bcm-otp-drv",
        .of_match_table = bcm_otp_drv_of_match,
    },
    .probe = bcm_otp_ioctl_init,
};

static int __init bcm_otp_drv_reg(void)
{
    printk("OTP driver initcall\n");

    return platform_driver_register(&bcm_otp_driver);
}

device_initcall(bcm_otp_drv_reg);

MODULE_DESCRIPTION("Broadcom BCA OTP Driver");
MODULE_LICENSE("GPL v2");
