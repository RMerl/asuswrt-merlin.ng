// <:copyright-BRCM:2013:DUAL/GPL:standard
// 
//    Copyright (c) 2013 Broadcom 
//    All Rights Reserved
// 
// Unless you and Broadcom execute a separate written software license
// agreement governing use of this software, this software is licensed
// to you under the terms of the GNU General Public License version 2
// (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
// with the following added to such license:
// 
//    As a special exception, the copyright holders of this software give
//    you permission to link this software with independent modules, and
//    to copy and distribute the resulting executable under terms of your
//    choice, provided that you also meet, for each linked independent
//    module, the terms and conditions of the license of that module.
//    An independent module is a module which is not derived from this
//    software.  The special exception does not apply to any modifications
//    of the software.
// 
// Not withstanding the above, under no circumstances may you combine
// this software in any way with any other Broadcom software provided
// under a license other than the GPL, without Broadcom's express prior
// written consent.
// 
// :>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <bdmf_system.h>
#include "bdmf_user_interface.h" /* BDMF MACROS, bdmf_ioctl_t */
#include "rdpa_user_int.h"
#include "rdpa_user.h"
#include "rdpa_api.h"

static void __ref rdpa_user_drv_exit(void);

static int device_created = 0;
static int chrdev_region_alloc = 0;
static int chrdev_added = 0; 
static struct class *rdpa_user_class = NULL;

/* for making node at the dev fs */
static int rdpa_user_drv_major;
static struct cdev my_cdev;
static dev_t rdpa_user_drv_dev_id;

static long ioctl(struct file *filp, unsigned int op, unsigned long args)
{
    ioctl_pa_t pa = {0};
    int ret = 0;

    if (copy_from_user(&pa, (long*)args, sizeof(ioctl_pa_t)))
    {
        BDMF_TRACE_ERR("failed to copy from user\n");
        return -1;
    }

    BDMF_TRACE_DBG("ioctl: %u", op);

    switch (op){
        case BDMF_NEW_AND_SET:
            ret = bdmf_user_new_and_set(&pa.bdmf_pa);
            break;
        
        case BDMF_DESTROY:
            ret = bdmf_user_destroy(&pa.bdmf_pa);
            break;
        
        case BDMF_MATTR_ALLOC:
            ret = bdmf_user_mattr_alloc(&pa.bdmf_pa);
            break;
        
        case BDMF_MATTR_FREE:
            ret = bdmf_user_mattr_free(&pa.bdmf_pa);
            break;
        
        case BDMF_GET:
            ret = bdmf_user_get(&pa.bdmf_pa);
            break;
        
        case BDMF_PUT:
            ret = bdmf_user_put(&pa.bdmf_pa);
            break;
        
        case BDMF_GET_NEXT:
            ret = bdmf_user_get_next(&pa.bdmf_pa);
            break;
        
        case BDMF_LINK:
            ret = bdmf_user_link(&pa.bdmf_pa);
            break;

        case BDMF_UNLINK:
            ret = bdmf_user_unlink(&pa.bdmf_pa);
            break;
        
        case BDMF_GET_NEXT_US_LINK:
            ret = bdmf_user_get_next_us_link(&pa.bdmf_pa);
            break;
        
        case BDMF_GET_NEXT_DS_LINK:
            ret = bdmf_user_get_next_ds_link(&pa.bdmf_pa);
            break;
        
        case BDMF_US_LINK_TO_OBJECT:
            ret = bdmf_user_us_link_to_object(&pa.bdmf_pa);
            break;
        
        case BDMF_DS_LINK_TO_OBJECT:
            ret = bdmf_user_ds_link_to_object(&pa.bdmf_pa);
            break;

        case BDMF_GET_OWNER:
            ret = bdmf_user_get_owner(&pa.bdmf_pa);
            break;

#ifdef PORT_OBJECT
        case RDPA_PORT_IOCTL:
            ret = rdpa_port_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif /* PORT_OBJECT */

#ifdef EGRESS_TM_OBJECT
        case RDPA_EGRESS_TM_IOCTL:
            ret = rdpa_egress_tm_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif /* EGRESS_TM_OBJECT */

#ifdef SYSTEM_OBJECT
        case RDPA_SYSTEM_IOCTL:
            ret = rdpa_system_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif /* SYSTEM_OBJECT */

#ifdef VLAN_OBJECT
        case RDPA_VLAN_IOCTL:
            ret = rdpa_vlan_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif /* VLAN_OBJECT */

#ifdef VLAN_ACTION_OBJECT
        case RDPA_VLAN_ACTION_IOCTL:
            ret = rdpa_vlan_action_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif /* VLAN_ACTION_OBJECT */

#ifdef TCONT_OBJECT
        case RDPA_TCONT_IOCTL:
            ret = rdpa_tcont_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break; 
#endif /* TCONT_OBJECT */

#ifdef IPTV_OBJECT
        case RDPA_IPTV_IOCTL:
            ret = rdpa_iptv_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break; 
#endif /* IPTV_OBJECT */

#ifdef TC_TO_QUEUE_OBJECT
        case RDPA_TC_TO_QUEUE_IOCTL:
            ret = rdpa_tc_to_queue_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif /* TC_TO_QUEUE_OBJECT */

#ifdef LLID_OBJECT
        case RDPA_LLID_IOCTL:
            ret = rdpa_llid_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif /* LLID_OBJECT */

#ifdef INGRESS_CLASS_OBJECT
        case RDPA_INGRESS_CLASS_IOCTL:
            ret = rdpa_ingress_class_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif /* INGRESS_CLASS_OBJECT */

#ifdef POLICER_OBJECT
        case RDPA_POLICER_IOCTL:
            ret = rdpa_policer_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif /* POLICER_OBJECT */

#ifdef PBIT_TO_QUEUE_OBJECT
        case RDPA_PBIT_TO_QUEUE_IOCTL:
            ret = rdpa_pbit_to_queue_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif /* PBIT_TO_QUEUE_OBJECT */

#ifdef PBIT_TO_GEM_OBJECT
        case RDPA_PBIT_TO_GEM_IOCTL:
            ret = rdpa_pbit_to_gem_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif /* PBIT_TO_GEM_OBJECT */

#ifdef DSCP_TO_PBIT_OBJECT
        case RDPA_DSCP_TO_PBIT_IOCTL:
            ret = rdpa_dscp_to_pbit_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif /* DSCP_TO_PBIT_OBJECT */

#ifdef UDPSPDTEST_OBJECT
        case RDPA_UDPSPDTEST_IOCTL:
            ret = rdpa_udpspdtest_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif

#ifdef CAPWAP_OBJECT
        case RDPA_CAPWAP_IOCTL:
            ret = rdpa_capwap_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;

        case RDPA_CAPWAP_REASSEMBLY_IOCTL:
            ret = rdpa_capwap_reassembly_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;

        case RDPA_CAPWAP_FRAGMENTATION_IOCTL:
            ret = rdpa_capwap_fragmentation_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif

#ifdef MLLID_OBJECT
        case RDPA_MLLID_IOCTL:
            ret = rdpa_mllid_ag_ioctl(pa.rdpa_pa.cmd, &pa.rdpa_pa);
            break;
#endif /* MLLID_OBJECT */
        default:
            BDMF_TRACE_ERR("no such ioctl cmd: %u\n", op);
            ret = EINVAL;
        }

    if (copy_to_user((long*)args, &pa, sizeof(ioctl_pa_t)))
    {
        BDMF_TRACE_ERR("failed to copy to user\n");
        return -1;
    }

    return ret;
}

struct file_operations rdpa_user_drv_cmd_fops = {

    owner : THIS_MODULE,    
    unlocked_ioctl : ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl  = ioctl,
#endif

};

static int __init rdpa_user_drv_init(void)
{
    int rc = 0;

	BDMF_TRACE_DBG("rdpa_user_drv_init");

    if((rc = alloc_chrdev_region(&rdpa_user_drv_dev_id, 0, 1, "rdpa_user")))
    {
        BDMF_TRACE_ERR("Unable to register char driver \n");
        goto error;    
    }
    chrdev_region_alloc = 1;

    rdpa_user_drv_major = MAJOR(rdpa_user_drv_dev_id);

    rdpa_user_class = class_create(THIS_MODULE, "rdpa_user");
    if (!rdpa_user_class) 
    {
		BDMF_TRACE_ERR("Unable to class_create() for the device rdpa_user");
		goto error;
	}

    if (!device_create(rdpa_user_class, NULL, rdpa_user_drv_dev_id, NULL, "rdpa_user"))
    {
        BDMF_TRACE_ERR("Unable to device_create() for the device rdpa_user");
        goto error;
    }

    device_created = 1;

    cdev_init(&my_cdev, &rdpa_user_drv_cmd_fops);

    /* return 0 on success */
    if((rc = cdev_add(&my_cdev, rdpa_user_drv_dev_id, 1)))
    {
        BDMF_TRACE_ERR("unable to add char device\n");
        goto error;    
    }           
    chrdev_added = 1;

    return 0;    

error:
    rdpa_user_drv_exit();
    return rc;    
}

/* WARNING: __exit_refok suppresses warnings from CONFIG_DEBUG_SECTION_MISMATCH
 *          This should only be called from __init or __exit functions.
 */
static void __ref rdpa_user_drv_exit(void)
{
    BDMF_TRACE_DBG("rdpa_user_drv_exit");

    if(chrdev_region_alloc)
    {
        unregister_chrdev_region(rdpa_user_drv_dev_id, 1);
        unregister_chrdev(rdpa_user_drv_major, "rdpa_user");
    }
    if(rdpa_user_class)
    {
        if (device_created)
        {
            device_destroy(rdpa_user_class, rdpa_user_drv_dev_id);
        }
        class_destroy(rdpa_user_class);
    }
    if(chrdev_added)
    {
        cdev_del(&my_cdev);
    }
}

module_init(rdpa_user_drv_init);
module_exit(rdpa_user_drv_exit);

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Erez Fishhimer");
MODULE_DESCRIPTION("rdpa_user_ag");
MODULE_VERSION("0.1");
