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
#include "bcm_score.h"
#include "bcm_bnl_ioctl.h"
#include "bnl.h"

#define BNL_MODNAME "Broadcom Network Latency Monitor"
#define BNL_VERSION	"v1.0"

struct bnl_cdev {
	struct class *class;
	struct device *device;
	struct cdev cdev;
	int major;
};

typedef struct  {
    uint16_t profile_id;
    uint16_t rsvd[15];
} bnl_profile_state_t;

struct bnl_cdev bnl_cdev_g;

/*TODO better to move this under score ioctl */

extern int bcm_score_bnl_request(score_ioctl_t cmd, score_ioctl_data_t *score_data);
extern int bcm_bnl_get_stats_by_streamid(bnlctl_stream_stats_t *stats);
extern int bcm_bnl_get_stream_dump_by_streamid(bnlctl_stream_dump_t *stream_dump);
extern int bcm_bnl_stream_tbl_dump_get_next(bnlctl_stream_dump_t *stream_dump);
extern int bcm_bnl_get_profile_stats(bnlctl_profile_stats_t *stats);
extern int bcm_bnl_profile_add(bnlctl_profile_cfg_t *cfg);
extern int bcm_bnl_profile_delete(bnlctl_profile_cfg_t *cfg);
extern int bcm_bnl_profile_config_set(bnlctl_profile_cfg_t *cfg);
extern int bcm_bnl_profile_config_get(bnlctl_profile_cfg_t *cfg);
extern bool is_valid_bnl_profile(uint8_t profile_id);

int __bnl_process_profile_delete(bnlctl_profile_cfg_t *cfg)
{
    score_ioctl_data_t score_data = {};
    score_dcp_del_data_t *dcp_del;
    dcp_del = &score_data.dcp_del;

    dcp_del->dcp_id = cfg->profile_id;
    //dcp_del->dcp_type = DCP_TYPE_BNL;
    return bcm_score_bnl_request(SCORE_IOCTL_SCORE_DCP_DELETE, &score_data);
}
int bnl_ioctl_process_profile_delete(bnlctl_profile_cfg_t *cfg)
{
	int ret;

	if(is_valid_bnl_profile(cfg->profile_id) == false)
		return -1;

	ret = __bnl_process_profile_delete(cfg);
	if(ret == 0)
		ret = bcm_bnl_profile_delete(cfg);
	return ret;
}

int bnl_ioctl_process_profile_add(bnlctl_profile_cfg_t *cfg)
{
    int ret;
    score_ioctl_data_t score_data = {};
    score_dcp_cfg_data_t *dcp_cfg;
    dcp_cfg = &score_data.dcp_cfg;

    dcp_cfg->info.dcp_type = DCP_TYPE_BNL;
    dcp_cfg->info.tuple_type = SCORE_FIVE_TUPLE;

    ret = bcm_score_bnl_request(SCORE_IOCTL_SCORE_DCP_ADD, &score_data);
    if (ret == 0) {
        cfg->profile_id = dcp_cfg->info.dcp_id;
        ret = bcm_bnl_profile_add(cfg);
        if( ret != 0) {
            __bnl_process_profile_delete(cfg);
        }
    }
    return ret;
}

int bnl_ioctl_process_profile_status(bnlctl_profile_stats_t *stats)
{
    return bcm_bnl_get_profile_stats(stats);
}
int bnl_ioctl_process_profile_config_set(bnlctl_profile_cfg_t *cfg)
{
    return bcm_bnl_profile_config_set(cfg);
}
int bnl_ioctl_process_profile_config_get(bnlctl_profile_cfg_t *cfg)
{
    return bcm_bnl_profile_config_get(cfg);
}


int bnl_ioctl_process_stream_start(bnlctl_stream_start_cfg_t *cfg)
{
    int ret;
    score_ioctl_data_t score_data = {};
    score_mon_start_data_t *start_cfg;
    start_cfg = &score_data.mon_start;

    start_cfg->dcp_id = cfg->profile_id;
    //start_cfg->ct_zone_id = cfg->ct_zone_id;

    /* copy bnl cfg */
    memcpy(&start_cfg->stream_data.bnl_cfg, &cfg->bnl_cfg, sizeof(start_cfg->stream_data.bnl_cfg));
    /* copy 5 tuple */
    memcpy(&start_cfg->stream_tuple, &cfg->tuple, sizeof(start_cfg->stream_tuple));

    ret = bcm_score_bnl_request(SCORE_IOCTL_SCORE_MONITOR_START, &score_data);

    if (ret == 0) {
        cfg->stream_id = start_cfg->score_stream_id;
        /* copy back bnl cfg */
        memcpy(&cfg->bnl_cfg, &start_cfg->stream_data.bnl_cfg, sizeof(cfg->bnl_cfg));
    }
    else
        cfg->stream_id = SCORE_STREAM_ID_INVALID;

    return ret;
}

int bnl_ioctl_process_stream_stop(bnlctl_stream_stop_cfg_t *cfg)
{
    score_ioctl_data_t score_data = {};
    score_mon_stop_data_t *stop_cfg;
    stop_cfg = &score_data.mon_stop;

    stop_cfg->dcp_id = cfg->profile_id;
    stop_cfg->score_stream_id = cfg->stream_id;

    return  bcm_score_bnl_request(SCORE_IOCTL_SCORE_MONITOR_STOP, &score_data);
}

int bnl_ioctl_process_stream_stats_get(bnlctl_stream_stats_t *stats)
{
    return bcm_bnl_get_stats_by_streamid(stats);
}

int bnl_ioctl_process_stream_dump(bnlctl_stream_dump_t *stream_dump)
{
    return bcm_bnl_get_stream_dump_by_streamid(stream_dump);
}

int bnl_ioctl_process_stream_table_dump(bnlctl_stream_dump_t *stream_dump)
{
    return bcm_bnl_stream_tbl_dump_get_next(stream_dump);
}

/*
 *------------------------------------------------------------------------------
 * Function Name: bnl_ioctl
 * Description	: Main entry point to handle user applications IOCTL requests
 *		  Flow Provisioning Interface Utility.
 * Returns	: 0 - success or error
 *------------------------------------------------------------------------------
 */
static long bnl_ioctl(struct file *filep, unsigned int command,
		      unsigned long arg)
{
	bnlctl_data_t ctl_args;
	bnl_profile_state_t *bnl_prof_state;
	int ret = 0;

	BCM_LOG_DEBUG(BCM_LOG_ID_BNL, "cmd<%d>",  command);

	switch (command) {
		case BNL_IOCTL_CTRL_INIT:


			bnl_prof_state = kzalloc(sizeof(*bnl_prof_state), GFP_KERNEL);
			if (!bnl_prof_state) {

				BCM_LOG_ERROR(BCM_LOG_ID_BNL, "BNL Char Device: Statefull open");
				return -ENOMEM;
			}

			bnl_prof_state->profile_id = BNL_PROFILE_ID_INVALID;

			filep->private_data = (void *)bnl_prof_state;

			BCM_LOG_DEBUG(BCM_LOG_ID_BNL, "BNL Char Device: State init success");

			break;

		case BNL_IOCTL_PROFILE_ADD:
			if (copy_from_user(&ctl_args.profile_cfg, (uint8_t *)arg, sizeof(ctl_args.profile_cfg)) != 0) {
				BCM_LOG_ERROR(BCM_LOG_ID_BNL, "copy_from_user error!");
				return -EINVAL;
			}

			ret = bnl_ioctl_process_profile_add(&ctl_args.profile_cfg);

			if (ret == 0) {
				bnl_profile_state_t *bnl_prof_state = filep->private_data;
				if(bnl_prof_state)
					bnl_prof_state->profile_id = ctl_args.profile_cfg.profile_id;
				if (copy_to_user((uint8_t *)arg, &ctl_args.profile_cfg, sizeof(ctl_args.profile_cfg)))
					ret = -EFAULT;
			}
			break;
		case BNL_IOCTL_PROFILE_DELETE:
			if (copy_from_user(&ctl_args.profile_cfg, (uint8_t *)arg, sizeof(ctl_args.profile_cfg)) != 0) {
				BCM_LOG_ERROR(BCM_LOG_ID_BNL, "copy_from_user error!");
				return -EINVAL;
			}

			ret = bnl_ioctl_process_profile_delete(&ctl_args.profile_cfg);

			if (ret == 0) {
				bnl_profile_state_t *bnl_prof_state = filep->private_data;
				if(bnl_prof_state)
					bnl_prof_state->profile_id = BNL_PROFILE_ID_INVALID;

				if (copy_to_user((uint8_t *)arg, &ctl_args.profile_cfg, sizeof(ctl_args.profile_cfg)))
					ret = -EFAULT;
			}
			break;

		case BNL_IOCTL_PROFILE_CONFIG_SET:

			if (copy_from_user(&ctl_args.profile_cfg, (uint8_t *)arg, sizeof(ctl_args.profile_cfg)) != 0) {
				BCM_LOG_ERROR(BCM_LOG_ID_BNL, "copy_from_user error!");
				return -EINVAL;
			}

			ret = bnl_ioctl_process_profile_config_set(&ctl_args.profile_cfg);

			if (ret == 0)
				if (copy_to_user((uint8_t *)arg, &ctl_args.profile_cfg, sizeof(ctl_args.profile_cfg)))
					ret = -EFAULT;
			break;

		case BNL_IOCTL_PROFILE_CONFIG_GET:

			if (copy_from_user(&ctl_args.profile_cfg, (uint8_t *)arg, sizeof(ctl_args.profile_cfg)) != 0) {
				BCM_LOG_ERROR(BCM_LOG_ID_BNL, "copy_from_user error!");
				return -EINVAL;
			}

			ret = bnl_ioctl_process_profile_config_get(&ctl_args.profile_cfg);

			if (ret == 0)
				if (copy_to_user((uint8_t *)arg, &ctl_args.profile_cfg, sizeof(ctl_args.profile_cfg)))
					ret = -EFAULT;
			break;

		case BNL_IOCTL_PROFILE_STATUS:

			if (copy_from_user(&ctl_args.profile_stats, (uint8_t *)arg, sizeof(ctl_args.profile_stats)) != 0) {
				BCM_LOG_ERROR(BCM_LOG_ID_BNL, "copy_from_user error!");
				return -EINVAL;
			}

			ret = bnl_ioctl_process_profile_status(&ctl_args.profile_stats);

			if (ret == 0)
				if (copy_to_user((uint8_t *)arg, &ctl_args.profile_stats, sizeof(ctl_args.profile_stats)))
					ret = -EFAULT;
			break;
		case BNL_IOCTL_STREAM_MON_START:
			if (copy_from_user(&ctl_args.start_cfg, (uint8_t *)arg, sizeof(ctl_args.start_cfg)) != 0) {
				BCM_LOG_ERROR(BCM_LOG_ID_BNL, "copy_from_user error!");
				return -EINVAL;
			}

			ret = bnl_ioctl_process_stream_start(&ctl_args.start_cfg);

			if (ret == 0)
				if (copy_to_user((uint8_t *)arg, &ctl_args.start_cfg, sizeof(ctl_args.start_cfg)))
					ret = -EFAULT;
			break;

		case BNL_IOCTL_STREAM_MON_STOP:
			if (copy_from_user(&ctl_args.stop_cfg, (uint8_t *)arg, sizeof(ctl_args.stop_cfg)) != 0) {
				BCM_LOG_ERROR(BCM_LOG_ID_BNL, "copy_from_user error!");
				return -EINVAL;
			}

			ret = bnl_ioctl_process_stream_stop(&ctl_args.stop_cfg);

			if (ret == 0)
				if (copy_to_user((uint8_t *)arg, &ctl_args.stop_cfg, sizeof(ctl_args.stop_cfg)))
					ret = -EFAULT;
			break;

		case BNL_IOCTL_STREAM_STATS_GET:

			if (copy_from_user(&ctl_args.stream_stats, (uint8_t *)arg, sizeof(ctl_args.stream_stats)) != 0) {
				BCM_LOG_ERROR(BCM_LOG_ID_BNL, "copy_from_user error!");
				return -EINVAL;
			}

			ret = bnl_ioctl_process_stream_stats_get(&ctl_args.stream_stats);

			if (ret == 0)
				if (copy_to_user((uint8_t *)arg, &ctl_args.stream_stats, sizeof(ctl_args.stream_stats)))
					ret = -EFAULT;
			break;

		case BNL_IOCTL_STREAM_DUMP:

			if (copy_from_user(&ctl_args.stream_dump, (uint8_t *)arg, sizeof(ctl_args.stream_dump)) != 0) {
				BCM_LOG_ERROR(BCM_LOG_ID_BNL, "copy_from_user error!");
				return -EINVAL;
			}

			ret = bnl_ioctl_process_stream_dump(&ctl_args.stream_dump);

			if (ret == 0)
				if (copy_to_user((uint8_t *)arg, &ctl_args.stream_dump, sizeof(ctl_args.stream_dump)))
					ret = -EFAULT;
			break;

		case BNL_IOCTL_STREAM_TABLE_DUMP:

			if (copy_from_user(&ctl_args.stream_dump, (uint8_t *)arg, sizeof(ctl_args.stream_dump)) != 0) {
				BCM_LOG_ERROR(BCM_LOG_ID_BNL, "copy_from_user error!");
				return -EINVAL;
			}

			ret = bnl_ioctl_process_stream_table_dump(&ctl_args.stream_dump);

			if (ret == 0)
				if (copy_to_user((uint8_t *)arg, &ctl_args.stream_dump, sizeof(ctl_args.stream_dump)))
					ret = -EFAULT;
			break;


		default:
			BCM_LOG_ERROR(BCM_LOG_ID_BNL, "Invalid cmd[%u]", command);
			ret = -EINVAL;
	}

	return ret;
} /* bnl_ioctl */

/*
 *------------------------------------------------------------------------------
 * Function Name: _open
 * Description  : Called when a user application opens this device.
 * Returns      : 0 - success
 *------------------------------------------------------------------------------
 */
static int bnl_open(struct inode *inode, struct file *file)
{
    BCM_LOG_DEBUG(BCM_LOG_ID_BNL, "BNL Char Device");

    return 0;
}

static int bnl_release(struct inode *inode, struct file *file)
{
    int ret;
    bnl_profile_state_t *bnl_prof_state = file->private_data;
	
    if (bnl_prof_state) {
        if ((bnl_prof_state->profile_id != BNL_PROFILE_ID_INVALID)) {
            bnlctl_profile_cfg_t cfg = {};
            BCM_LOG_DEBUG(BCM_LOG_ID_BNL, "BNL Char Device releasing state");
            cfg.profile_id = bnl_prof_state->profile_id;
            ret = bnl_ioctl_process_profile_delete(&cfg);
        }

        kfree(bnl_prof_state);
    }
	
	BCM_LOG_DEBUG(BCM_LOG_ID_BNL, "BNL Char Device");
	return 0;
}

/* Global file ops */
static struct file_operations bnl_fops =
{
	.unlocked_ioctl = bnl_ioctl,
#if defined(CONFIG_COMPAT)
	.compat_ioctl = bnl_ioctl,
#endif
	.open = bnl_open,
	.release = bnl_release
};

void bnl_cdev_uninit(void)
{
	if (!IS_ERR(bnl_cdev_g.device)) {
		device_destroy(bnl_cdev_g.class, MKDEV(bnl_cdev_g.major, 0));
		cdev_del(&bnl_cdev_g.cdev);
	}

	if (!IS_ERR(bnl_cdev_g.class))
		class_destroy(bnl_cdev_g.class);

	if (bnl_cdev_g.major)
		unregister_chrdev_region(MKDEV(bnl_cdev_g.major, 0),
					 BNL_NUM_DEVICES);
}

/*
 *------------------------------------------------------------------------------
 * Function Name: bnl_cdev_init
 * Description  : Initial function that is called at module startup that
 *                registers char device.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
int bnl_cdev_init(void)
{
	dev_t dev = 0;
	dev_t devno;
	int rc;

	bcmLog_setLogLevel(BCM_LOG_ID_BNL, BCM_LOG_LEVEL_NOTICE);

	rc = alloc_chrdev_region(&dev, 0, BNL_NUM_DEVICES, BNL_DEV_NAME);
	if (rc < 0) {
		__logError("alloc_chrdev_region() failed\n");
		return -ENODEV;
	}
	bnl_cdev_g.major = MAJOR(dev);

	/* create device and class */
	bnl_cdev_g.class = class_create(THIS_MODULE, BNL_DEV_NAME);
	if (IS_ERR(bnl_cdev_g.class)) {
		rc = PTR_ERR(bnl_cdev_g.class);
		__logError("Fail to create class %s, rc = %d\n", BNL_DEV_NAME, rc);
		goto fail;
	}

	devno = MKDEV(bnl_cdev_g.major, 0);
	cdev_init(&bnl_cdev_g.cdev, &bnl_fops);
	bnl_cdev_g.cdev.owner = THIS_MODULE;

	rc = cdev_add(&bnl_cdev_g.cdev, devno, 1);
	if (rc) {
		__logError("Fail to add cdev %s, rc = %d\n",  BNL_DEV_NAME, rc);
		goto fail;
	}

	bnl_cdev_g.device = device_create(bnl_cdev_g.class, NULL, devno, NULL,
				     BNL_DEV_NAME);
	if (IS_ERR(bnl_cdev_g.device)) {
		rc = PTR_ERR(bnl_cdev_g.device);
		__logError("Fail to create device %s, rc = %d\n", BNL_DEV_NAME, rc);
		goto fail;
	}

	__logInfo(BNL_MODNAME " Char Driver " BNL_VERSION " Registered<%d>"
		CLRnl, bnl_cdev_g.major);

	return bnl_cdev_g.major;

fail:
	bnl_cdev_uninit();
	return rc;
}
