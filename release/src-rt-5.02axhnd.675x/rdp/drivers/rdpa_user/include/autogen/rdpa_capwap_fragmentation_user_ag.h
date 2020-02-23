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
/*
 * capwap_fragmentation object user header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_CAPWAP_FRAGMENTATION_USR_H_
#define _RDPA_AG_CAPWAP_FRAGMENTATION_USR_H_

#include <sys/ioctl.h>
#include "rdpa_user.h"
#include "rdpa_user_types.h"
#include "rdpa_capwap_fragmentation_user_ioctl_ag.h"

/** \addtogroup capwap_fragmentation
 * @{
 */


/** Get capwap_fragmentation type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a capwap_fragmentation object.
 * \return capwap_fragmentation type handle
 */
static inline bdmf_type_handle rdpa_capwap_fragmentation_drv(void)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_CAPWAP_FRAGMENTATION_DRV;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return 0;
	}

	ret = ioctl(fd, RDPA_CAPWAP_FRAGMENTATION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return 0;
	}

	close(fd);
	return pa.drv;
}

/** Get capwap_fragmentation object.

 * This function returns capwap_fragmentation object instance.
 * \param[out] capwap_fragmentation_obj    Object handle
 * \return    0=OK or error <0
 */
static inline int rdpa_capwap_fragmentation_get(bdmf_object_handle *capwap_fragmentation_obj)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_CAPWAP_FRAGMENTATION_GET;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CAPWAP_FRAGMENTATION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return ret;
	}

	*capwap_fragmentation_obj = pa.mo;
	close(fd);
	return pa.ret;
}

/** Get capwap_fragmentation/enable attribute.
 *
 * Get CAPWAP enable/disable processing.
 * \param[in]   mo_ capwap_fragmentation object handle or mattr transaction handle
 * \param[out]  enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_fragmentation_enable_get(bdmf_object_handle mo_, bdmf_boolean *enable_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)enable_;
	pa.cmd = RDPA_CAPWAP_FRAGMENTATION_ENABLE_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CAPWAP_FRAGMENTATION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set capwap_fragmentation/enable attribute.
 *
 * Set CAPWAP enable/disable processing.
 * \param[in]   mo_ capwap_fragmentation object handle or mattr transaction handle
 * \param[in]   enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_fragmentation_enable_set(bdmf_object_handle mo_, bdmf_boolean enable_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)enable_;
	pa.cmd = RDPA_CAPWAP_FRAGMENTATION_ENABLE_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CAPWAP_FRAGMENTATION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get capwap_fragmentation/cfg attribute.
 *
 * Get CAPWAP fragmentation configuration.
 * \param[in]   mo_ capwap_fragmentation object handle or mattr transaction handle
 * \param[out]  cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_fragmentation_cfg_get(bdmf_object_handle mo_, rdpa_capwap_fragmentation_cfg_t * cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)cfg_;
	pa.cmd = RDPA_CAPWAP_FRAGMENTATION_CFG_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CAPWAP_FRAGMENTATION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set capwap_fragmentation/cfg attribute.
 *
 * Set CAPWAP fragmentation configuration.
 * \param[in]   mo_ capwap_fragmentation object handle or mattr transaction handle
 * \param[in]   cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_fragmentation_cfg_set(bdmf_object_handle mo_, const rdpa_capwap_fragmentation_cfg_t * cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)cfg_;
	pa.cmd = RDPA_CAPWAP_FRAGMENTATION_CFG_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CAPWAP_FRAGMENTATION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get capwap_fragmentation/stats attribute.
 *
 * Get CAPWAP fragmentation statistics.
 * \param[in]   mo_ capwap_fragmentation object handle or mattr transaction handle
 * \param[out]  stats_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_capwap_fragmentation_stats_get(bdmf_object_handle mo_, rdpa_capwap_fragmentation_stats_t * stats_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)stats_;
	pa.cmd = RDPA_CAPWAP_FRAGMENTATION_STATS_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CAPWAP_FRAGMENTATION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}

/** @} end of capwap_fragmentation Doxygen group */




#endif /* _RDPA_AG_CAPWAP_FRAGMENTATION_USR_H_ */
