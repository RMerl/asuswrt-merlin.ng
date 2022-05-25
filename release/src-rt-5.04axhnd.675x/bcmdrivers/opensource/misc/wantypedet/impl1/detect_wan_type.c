/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/of_device.h>
#include <linux/miscdevice.h>
#include "opticaldet.h"
#include "pmd.h"
#include "bcm_ioremap_shared.h"
 
extern int try_wan_type_sensing(pmd_calibration_parameters *calibration_parameters_from_json);
 
void *g_gpon_rcvr_base = NULL;
void *g_epon_top_base = NULL;
void *g_epon_lif_base = NULL;
void *g_epon_xpcsr_base = NULL;
void *g_ngpon_rxgen_base = NULL;
 
static int _file_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int _file_release(struct inode *inode, struct file *file)
{
    return 0;
}

/* The frame size is larger than 1024 bytes [-Werror=frame-larger-than=] */
static pmd_wan_type_auto_detect_settings pmd_wan_auto_detect = {0};
static long _detect_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    unsigned long detectVal = 0;

    switch (cmd)
    {
        case OPTICALDET_IOCTL_DETECT:
            {
                if (copy_from_user(&pmd_wan_auto_detect, (void *)arg, sizeof(pmd_wan_type_auto_detect_settings)))
                {
                    printk(KERN_ERR "\nError in Optical WAN type auto-detection module - OPTICALDET_IOCTL_DETECT.\n");
                }

                if (pmd_wan_auto_detect.is_calibration_file_valid) {
                    detectVal = try_wan_type_sensing(&(pmd_wan_auto_detect.calibration_parameters_from_json));
                } else {
                    detectVal = try_wan_type_sensing(NULL);
                }
            }
            break;
        default:
            printk("%s: ERROR: No such IOCTL", __FILE__);
            return -1;
    }

    if (copy_to_user((void*)arg, (void*)&detectVal, sizeof(unsigned long)))
	{
	    printk(KERN_ERR "%s: failed copy data to user!\n", __FUNCTION__);
		return -1;
	}

    return 0;
}

static const struct file_operations detect_file_ops =
{
    .owner = THIS_MODULE,
    .open = _file_open,
    .release = _file_release,
    .unlocked_ioctl = _detect_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = _detect_ioctl,
#endif
};

static const struct of_device_id bcmbca_wantypedetect_of_match[] = {
    { .compatible = "brcm,wantypedetect" },
    { },
};
 
MODULE_DEVICE_TABLE(of, bcmbca_wantypedetect_of_match);
 
static int bcmbca_wantypedetect_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const struct of_device_id *match;
    struct miscdevice *mdev;
    struct resource *r_mem;
    int ret;
 
    match = of_match_device(bcmbca_wantypedetect_of_match, dev);
    if (!match)
    {
        dev_err(dev, "Optical WAN type auto-detection dev: Failed to match\n");
        return -ENODEV;
    }
 
    r_mem = platform_get_resource_byname(pdev, IORESOURCE_MEM, "gpon_general_config");
    if (!r_mem)
    {
        dev_err(dev, "Optical WAN type auto-detection dev unable to get GPON General config register resource.\n");
        return -ENODEV;
    }
   
    g_gpon_rcvr_base = devm_ioremap_shared_resource(dev, r_mem);
    if (IS_ERR(g_gpon_rcvr_base))
    {
        dev_err(dev, "Optical WAN type auto-detection dev unable to remap GPON General config register resource.\n");
        return PTR_ERR(g_gpon_rcvr_base);
    }
    
    r_mem = platform_get_resource_byname(pdev, IORESOURCE_MEM, "epon_top");
    if (!r_mem)
    {
        dev_err(dev, "Optical WAN type auto-detection dev unable to get EPON Top register resource.\n");
        return -ENODEV;
    }
   
    g_epon_top_base = devm_ioremap_shared_resource(dev, r_mem);
    if (IS_ERR(g_epon_top_base))
    {
        dev_err(dev, "Optical WAN type auto-detection dev unable to remap EPON Top register resource.\n");
        return PTR_ERR(g_epon_top_base);
    }
    
    r_mem = platform_get_resource_byname(pdev, IORESOURCE_MEM, "epon_lif");
    if (!r_mem)
    {
        dev_err(dev, "Optical WAN type auto-detection dev unable to get EPON Lif register resource.\n");
        return -ENODEV;
    }
   
    g_epon_lif_base = devm_ioremap_shared_resource(dev, r_mem);
    if (IS_ERR(g_epon_lif_base))
    {
        dev_err(dev, "Optical WAN type auto-detection dev unable to remap EPON Lif register resource.\n");
        return PTR_ERR(g_epon_lif_base);
    }
    
    r_mem = platform_get_resource_byname(pdev, IORESOURCE_MEM, "epon_xpcsr");
    if (r_mem)
    {
        g_epon_xpcsr_base = devm_ioremap_shared_resource(dev, r_mem);
        if (IS_ERR(g_epon_xpcsr_base))
        {
            dev_err(dev, "Optical WAN type auto-detection dev unable to remap EPON Xpcsr register resource.\n");
            return PTR_ERR(g_epon_xpcsr_base);
        }
    }
    
    r_mem = platform_get_resource_byname(pdev, IORESOURCE_MEM, "ngpon_rxgen");
    if (r_mem)
    {
        g_ngpon_rxgen_base = devm_ioremap_shared_resource(dev, r_mem);
        if (IS_ERR(g_ngpon_rxgen_base))
        {
            dev_err(dev, "Optical WAN type auto-detection dev unable to remap NGPON Rx Gen register resource.\n");
            return PTR_ERR(g_ngpon_rxgen_base);
        }
    }
    
    mdev = devm_kzalloc(dev, sizeof(*mdev), GFP_KERNEL);
    if (!mdev)
    {
        dev_err(dev, "Failed to allocate memory for Optical WAN type auto-detection dev\n");
        return -ENOMEM;
    }
 
    platform_set_drvdata(pdev, mdev);
 
    mdev->minor  = MISC_DYNAMIC_MINOR;
    mdev->name   = "wantypedetect";
    mdev->fops   = &detect_file_ops;
    mdev->parent = NULL;
 
    ret = misc_register(mdev);
    if (ret) {
        dev_err(dev, "Failed to register Optical WAN type auto-detection dev\n");
        return ret;
    }

    dev_err(dev, "Optical WAN type auto-detection module loaded succesfully!\n");
 
    return ret;
}
 
static int bcmbca_wantypedetect_remove(struct platform_device *pdev)
{
    struct miscdevice *mdev = platform_get_drvdata(pdev);
 
    misc_deregister(mdev);
    dev_info(&pdev->dev, "Optical WAN type auto-detection module unloaded\n");
 
    return 0;
}
 
static struct platform_driver bcmbca_wantypedetect_drv = {
	.probe = bcmbca_wantypedetect_probe,
	.remove = bcmbca_wantypedetect_remove,
	.driver = {
		.name = "bcmbca-wantypedetect",
		.of_match_table = bcmbca_wantypedetect_of_match,
	},
};
 
module_platform_driver(bcmbca_wantypedetect_drv);
 
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Optical WAN type auto-detection driver");
MODULE_LICENSE("GPL v2");
