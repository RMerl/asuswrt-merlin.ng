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
 * vlan_action object user header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_VLAN_ACTION_USR_H_
#define _RDPA_AG_VLAN_ACTION_USR_H_

#include <sys/ioctl.h>
#include "rdpa_user.h"
#include "rdpa_user_types.h"
#include "rdpa_vlan_action_user_ioctl_ag.h"

/** \addtogroup vlan_action
 * @{
 */


/** Get vlan_action type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a vlan_action object.
 * \return vlan_action type handle
 */
static inline bdmf_type_handle rdpa_vlan_action_drv(void)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_VLAN_ACTION_DRV;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return 0;
	}

	ret = ioctl(fd, RDPA_VLAN_ACTION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return 0;
	}

	close(fd);
	return pa.drv;
}


/** vlan_action object key. */
typedef struct {
    rdpa_traffic_dir dir; /**< vlan_action: Traffic Direction */
    bdmf_number index; /**< vlan_action: VLAN Action Index */
} rdpa_vlan_action_key_t;

/** Get vlan_action object by key.

 * This function returns vlan_action object instance by key.
 * \param[in] key_    Object key
 * \param[out] vlan_action_obj    Object handle
 * \return    0=OK or error <0
 */
static inline int rdpa_vlan_action_get(const rdpa_vlan_action_key_t * key_, bdmf_object_handle *vlan_action_obj)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_VLAN_ACTION_GET;
	pa.ptr = (bdmf_ptr)(unsigned long)key_;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_VLAN_ACTION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return ret;
	}

	*vlan_action_obj = pa.mo;
	close(fd);
	return pa.ret;
}

/** Get vlan_action/dir attribute.
 *
 * Get Traffic Direction.
 * \param[in]   mo_ vlan_action object handle or mattr transaction handle
 * \param[out]  dir_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_vlan_action_dir_get(bdmf_object_handle mo_, rdpa_traffic_dir *dir_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)dir_;
	pa.cmd = RDPA_VLAN_ACTION_DIR_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_VLAN_ACTION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set vlan_action/dir attribute.
 *
 * Set Traffic Direction.
 * \param[in]   mo_ vlan_action object handle or mattr transaction handle
 * \param[in]   dir_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_vlan_action_dir_set(bdmf_object_handle mo_, rdpa_traffic_dir dir_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)dir_;
	pa.cmd = RDPA_VLAN_ACTION_DIR_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_VLAN_ACTION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get vlan_action/index attribute.
 *
 * Get VLAN Action Index.
 * \param[in]   mo_ vlan_action object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_vlan_action_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)index_;
	pa.cmd = RDPA_VLAN_ACTION_INDEX_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_VLAN_ACTION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set vlan_action/index attribute.
 *
 * Set VLAN Action Index.
 * \param[in]   mo_ vlan_action object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_vlan_action_index_set(bdmf_object_handle mo_, bdmf_number index_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)index_;
	pa.cmd = RDPA_VLAN_ACTION_INDEX_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_VLAN_ACTION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get vlan_action/action attribute.
 *
 * Get Action and parameters.
 * \param[in]   mo_ vlan_action object handle or mattr transaction handle
 * \param[out]  action_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_action_action_get(bdmf_object_handle mo_, rdpa_vlan_action_cfg_t * action_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)action_;
	pa.cmd = RDPA_VLAN_ACTION_ACTION_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_VLAN_ACTION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set vlan_action/action attribute.
 *
 * Set Action and parameters.
 * \param[in]   mo_ vlan_action object handle or mattr transaction handle
 * \param[in]   action_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_action_action_set(bdmf_object_handle mo_, const rdpa_vlan_action_cfg_t * action_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)action_;
	pa.cmd = RDPA_VLAN_ACTION_ACTION_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_VLAN_ACTION_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}

/** @} end of vlan_action Doxygen group */




#endif /* _RDPA_AG_VLAN_ACTION_USR_H_ */
