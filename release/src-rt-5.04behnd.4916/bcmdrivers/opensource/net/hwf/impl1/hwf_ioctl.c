/*
   <:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom
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
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/bcm_log.h>
#include "hwf_ioctl.h"
#include "hwf.h"

typedef struct {
	struct class *class;
	struct device *device;
	struct cdev cdev;
	int major;
} hwf_cdev_t;

#undef HWF_DECL
#define HWF_DECL(_x)	#_x,

const char *hwfctl_ioctl_name[] = {
	HWF_DECL(hwfctl_ioctl_sys)
	HWF_DECL(hwfctl_ioctl_max)
};

const char *hwfctl_subsys_name[] = {
	HWF_DECL(hwfctl_subsys_system)
	HWF_DECL(hwfctl_subsys_vserver)
	HWF_DECL(hwfctl_subsys_hosts)
	HWF_DECL(hwfctl_subsys_ratelimiters)
	HWF_DECL(hwfctl_subsys_stats)
	HWF_DECL(hwfctl_subsys_flow)
	HWF_DECL(hwfctl_subsys_max)
};

const char *hwfctl_op_name[] = {
	HWF_DECL(hwfctl_op_status)
	HWF_DECL(hwfctl_op_set)
	HWF_DECL(hwfctl_op_get)
	HWF_DECL(hwfctl_op_getnext)
	HWF_DECL(hwfctl_op_add)
	HWF_DECL(hwfctl_op_del)
	HWF_DECL(hwfctl_op_del_by_name)
	HWF_DECL(hwfctl_op_update)
	HWF_DECL(hwfctl_op_update_by_name)
	HWF_DECL(hwfctl_op_show)
	HWF_DECL(hwfctl_op_max)
};

static int hwf_ioctl_process_system(hwfctl_data_t *hwfctl)
{
	int ret = HWF_SUCCESS;

	if (hwfctl->op == hwfctl_op_set) {

		if (hwfctl->info.valid.hwf_enable) {
			if (hwfctl->info.config.hwf_enable)
				bcm_hwf_enable(true);
			else
				bcm_hwf_enable(false);
		}

		if (hwfctl->info.valid.expect_lookup_enable) {
			if (hwfctl->info.config.expect_lookup_enable)
				bcm_hwf_expect_lookup_enable(true);
			else
				bcm_hwf_expect_lookup_enable(false);
		}

		if (hwfctl->info.valid.lan_ct_limit) {
			if (hwfctl->info.config.lan_ct_limit)
				bcm_hwf_lan_ct_limit_enable(true);
			else
				bcm_hwf_lan_ct_limit_enable(false);
		}

		if (hwfctl->info.valid.wan_miss_ratelimit) {
			if (hwfctl->info.config.wan_miss_ratelimit)
				bcm_hwf_wan_miss_ratelimit_enable(true);
			else
				bcm_hwf_wan_miss_ratelimit_enable(false);
		}

	} else if (hwfctl->op == hwfctl_op_status) {
			ret = bcm_hwf_status(hwfctl);
	} else {
		BCM_LOG_ERROR(BCM_LOG_ID_HWF, "Invalid op[%u]", hwfctl->op);
		ret = HWF_ERROR;
	}
	
	return ret;
}

static int hwf_ioctl_process_hosts(hwfctl_data_t *hwfctl)
{
	int ret = 0;

	hwfctl_hosts_t *host = &hwfctl->hosts;

	if (hwfctl->op == hwfctl_op_set) {
		if (hwfctl->info.hosts_default_ct_max || hwfctl->info.hosts_default_ct_rate
				|| hwfctl->info.hosts_default_ct_burst || hwfctl->info.valid.allow_dynamic_hosts
				|| hwfctl->info.valid.nohost_pkt_rate || hwfctl->info.valid.after_max_ct_rate)
			ret = bcm_hwf_limit_hosts_cfg_set(hwfctl);
		else
			BCM_LOG_ERROR(BCM_LOG_ID_HWF, "Invalid params[%u]", hwfctl->op);
	}
	else if (hwfctl->op == hwfctl_op_get) 
		ret = bcm_hwf_limit_hosts_cfg_get(hwfctl);
	else if (hwfctl->op == hwfctl_op_add)
		ret = bcm_hwf_limit_host_obj_add(host);
	else if (hwfctl->op == hwfctl_op_update)
		ret = bcm_hwf_limit_host_obj_update(host);
	else if (hwfctl->op == hwfctl_op_del)
		ret = bcm_hwf_limit_host_obj_delete(host);
	else {
		BCM_LOG_ERROR(BCM_LOG_ID_HWF, "Invalid op[%u]", hwfctl->op);
		ret = -1;
	}
	return ret;
}

static int hwf_ioctl_process_ratelimiter(hwfctl_data_t *hwfctl)
{
	int ret = 0;
	hwfctl_ratelimiter_t *ratelimiter = &hwfctl->ratelimiter;
	int namelen;

	namelen = strnlen(ratelimiter->name, HWF_MAX_KEY_NAME_LEN);

	if ((namelen <= 0) ||  (namelen >= HWF_MAX_KEY_NAME_LEN)){
		BCM_LOG_ERROR(BCM_LOG_ID_HWF, "Invalid params - name");
		ret = -EINVAL;
		goto done;
	}

	if ((hwfctl->op == hwfctl_op_add) && (!ratelimiter->type)) {
		BCM_LOG_ERROR(BCM_LOG_ID_HWF, "Invalid params - type");
		ret = -EINVAL;
		goto done;
	}

	if (((hwfctl->op == hwfctl_op_add) || (hwfctl->op == hwfctl_op_update))
			&& (!ratelimiter->rate || !ratelimiter->burst)) {
		BCM_LOG_ERROR(BCM_LOG_ID_HWF, "Invalid params");
		ret = -EINVAL;
		goto done;
	}

	if (hwfctl->op == hwfctl_op_add)
		ret = bcm_hwf_ratelimiter_add(ratelimiter);
	else if (hwfctl->op == hwfctl_op_update)
		ret = bcm_hwf_ratelimiter_update(ratelimiter);
	else if (hwfctl->op == hwfctl_op_del_by_name)
		ret = bcm_hwf_ratelimiter_delete(ratelimiter->name);
	else {
		BCM_LOG_ERROR(BCM_LOG_ID_HWF, "Invalid op[%u]", hwfctl->op);
		ret = -1;
	}
done:
	return ret;
}

static int hwf_ioctl_process_vserver(hwfctl_data_t *hwfctl)
{
	int ret = 0;
	hwfctl_vserver_t *vserver = &hwfctl->vserver;
	int namelen;

	namelen = strnlen(vserver->name, HWF_MAX_KEY_NAME_LEN);
	if (((namelen <= 0) ||  (namelen >= HWF_MAX_KEY_NAME_LEN))) {
		BCM_LOG_ERROR(BCM_LOG_ID_HWF, "Invalid params");
		ret = -EINVAL;
		goto done;
	}

	namelen = strnlen(vserver->ratelimiter_name, HWF_MAX_KEY_NAME_LEN);
	if (((namelen >= HWF_MAX_KEY_NAME_LEN))) {
		BCM_LOG_ERROR(BCM_LOG_ID_HWF, "Invalid params");
		ret = -EINVAL;
		goto done;
	}

	if (hwfctl->op == hwfctl_op_add)
		ret = bcm_hwf_vserver_add(vserver);
	else if (hwfctl->op == hwfctl_op_update)
		ret = bcm_hwf_vserver_update(vserver);
	else if (hwfctl->op == hwfctl_op_del_by_name)
		ret = bcm_hwf_vserver_delete(vserver->name);
	else {
		BCM_LOG_ERROR(BCM_LOG_ID_HWF, "Invalid op[%u]", hwfctl->op);
		ret = -1;
	}
done:
	return ret;
}

static int hwf_ioctl_process_stats(hwfctl_data_t *hwfctl)
{
	int ret = 0;

	if (hwfctl->op == hwfctl_op_show)
		BCM_LOG_ERROR(BCM_LOG_ID_HWF, "show not supported yet");
	else {
		BCM_LOG_ERROR(BCM_LOG_ID_HWF, "Invalid op[%u]", hwfctl->op);
		ret = -1;
	}
	return ret;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: hwf_ioctl
 * Description	: Main entry point to handle user applications IOCTL requests
 *		  Flow Provisioning Interface Utility.
 * Returns	: 0 - success or error
 *------------------------------------------------------------------------------
 */
static long hwf_ioctl(struct file *filep, unsigned int command,
		      unsigned long arg)
{
	hwfctl_ioctl_t cmd;
	hwfctl_data_t pa;
	hwfctl_data_t *hwfctl_p = &pa;
	int ret = 0;

	if (command > hwfctl_ioctl_max)
		cmd = hwfctl_ioctl_max;
	else
		cmd = (hwfctl_ioctl_t)command;

	if (copy_from_user(hwfctl_p, (uint8_t *)arg, sizeof(pa)) != 0) {
		BCM_LOG_ERROR(BCM_LOG_ID_HWF, "copy_from_user error!");
		return -EINVAL;
	}
	if ((command > hwfctl_ioctl_max) || (hwfctl_p->subsys > hwfctl_subsys_max) ||
	    (hwfctl_p->op > hwfctl_op_max)) {
		BCM_LOG_ERROR(BCM_LOG_ID_HWF, "parameter error! cmd=%d subsys=%d op=%d",
			      command, hwfctl_p->subsys, hwfctl_p->op);
		return -EINVAL;
	}
	BCM_LOG_DEBUG(BCM_LOG_ID_HWF, "cmd<%d>%s subsys<%d>%s op<%d>%s",
		      command, hwfctl_ioctl_name[command-hwfctl_ioctl_sys],
		      hwfctl_p->subsys, hwfctl_subsys_name[hwfctl_p->subsys],
		      hwfctl_p->op, hwfctl_op_name[hwfctl_p->op]);

	if (cmd == hwfctl_ioctl_sys) {
		switch (hwfctl_p->subsys) {
		case hwfctl_subsys_system:
			ret = hwf_ioctl_process_system(hwfctl_p);
			break;
		case hwfctl_subsys_vserver:
			ret = hwf_ioctl_process_vserver(hwfctl_p);
			break;
		case hwfctl_subsys_hosts:
			ret = hwf_ioctl_process_hosts(hwfctl_p);
			break;
		case hwfctl_subsys_ratelimiters:
			ret = hwf_ioctl_process_ratelimiter(hwfctl_p);
			break;
		case hwfctl_subsys_stats:
			ret = hwf_ioctl_process_stats(hwfctl_p);
			break;
		case hwfctl_subsys_flow:
			break;

		default:
			BCM_LOG_ERROR(BCM_LOG_ID_HWF, "Invalid subsys[%u]",
				      hwfctl_p->subsys);
		}
	} else {
		BCM_LOG_ERROR(BCM_LOG_ID_HWF, "Invalid cmd[%u]", command);
		ret = -EINVAL;
	}

	/* we will always copy to user at the end, doesn't matter
	 * which op / action is taken */
	if (copy_to_user((uint8_t *)arg, hwfctl_p, sizeof(pa)))
		ret = -EFAULT;

	return ret;
} /* hwf_ioctl */

struct hwf_cdev {
	struct class *class;
	struct device *device;
	struct cdev cdev;
	int major;
};

struct hwf_cdev hwf_cdev_g;

/*
 *------------------------------------------------------------------------------
 * Function Name: hwf_open
 * Description  : Called when a user application opens this device.
 * Returns      : 0 - success
 *------------------------------------------------------------------------------
 */
static int hwf_open(struct inode *inode, struct file *filp)
{
	BCM_LOG_DEBUG(BCM_LOG_ID_HWF, "HWF Char Device");
	return 0;
} /* hwf_open */

/* Global file ops */
static struct file_operations hwf_fops =
{
	.unlocked_ioctl = hwf_ioctl,
#if defined(CONFIG_COMPAT)
	.compat_ioctl = hwf_ioctl,
#endif
	.open = hwf_open,
};

void hwf_cdev_deinit(void)
{
	if (!IS_ERR(hwf_cdev_g.device)) {
		device_destroy(hwf_cdev_g.class, MKDEV(hwf_cdev_g.major, 0));
		cdev_del(&hwf_cdev_g.cdev);
	}

	if (!IS_ERR(hwf_cdev_g.class))
		class_destroy(hwf_cdev_g.class);

	if (hwf_cdev_g.major)
		unregister_chrdev_region(MKDEV(hwf_cdev_g.major, 0),
					 HWF_NUM_DEVICES);
}

/*
 *------------------------------------------------------------------------------
 * Function Name: hwf_cdev_init
 * Description  : Initial function that is called at module startup that
 *                registers char device.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
int hwf_cdev_init(void)
{
	dev_t dev = 0;
	dev_t devno;
	int rc;

	bcmLog_setLogLevel(BCM_LOG_ID_HWF, BCM_LOG_LEVEL_NOTICE);

	rc = alloc_chrdev_region(&dev, 0, HWF_NUM_DEVICES, HWF_DEV_NAME);
	if (rc < 0) {
		pr_err("%s:alloc_chrdev_region() failed\n", __func__);
		return -ENODEV;
	}
	hwf_cdev_g.major = MAJOR(dev);

	/* create device and class */
	hwf_cdev_g.class = class_create(THIS_MODULE, HWF_DEV_NAME);
	if (IS_ERR(hwf_cdev_g.class)) {
		rc = PTR_ERR(hwf_cdev_g.class);
		pr_err("%s:Fail to create class %s, rc = %d\n", __func__,
		       HWF_DEV_NAME, rc);
		goto fail;
	}

	devno = MKDEV(hwf_cdev_g.major, 0);
	cdev_init(&hwf_cdev_g.cdev, &hwf_fops);
	hwf_cdev_g.cdev.owner = THIS_MODULE;

	rc = cdev_add(&hwf_cdev_g.cdev, devno, 1);
	if (rc) {
		pr_err("%s:Fail to add cdev %s, rc = %d\n", __func__,
		       HWF_DEV_NAME, rc);
		goto fail;
	}

	hwf_cdev_g.device = device_create(hwf_cdev_g.class, NULL, devno, NULL,
				      HWF_DEV_NAME);
	if (IS_ERR(hwf_cdev_g.device)) {
		rc = PTR_ERR(hwf_cdev_g.device);
		pr_err("%s:Fail to create device %s, rc = %d\n", __func__,
		       HWF_DEV_NAME, rc);
		goto fail;
	}

	pr_info(HWF_MODNAME " Char Driver " HWF_VERSION " Registered<%d>"
		CLRnl, hwf_cdev_g.major);

	return hwf_cdev_g.major;

fail:
	hwf_cdev_deinit();
	return rc;
}
