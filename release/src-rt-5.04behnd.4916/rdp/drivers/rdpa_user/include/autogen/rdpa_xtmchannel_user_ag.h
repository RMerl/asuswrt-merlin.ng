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
 * xtmchannel object user header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_XTMCHANNEL_USR_H_
#define _RDPA_AG_XTMCHANNEL_USR_H_

#include <sys/ioctl.h>
#include "rdpa_user.h"
#include "rdpa_user_types.h"
#include "rdpa_xtmchannel_user_ioctl_ag.h"

/** \addtogroup xtmchannel
 * @{
 */


/** Get xtmchannel type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a xtmchannel object.
 * \return xtmchannel type handle
 */
static inline bdmf_type_handle rdpa_xtmchannel_drv(void)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_XTMCHANNEL_DRV;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return 0;
	}

	ret = ioctl(fd, RDPA_XTMCHANNEL_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return 0;
	}

	close(fd);
	return pa.drv;
}

/** Get xtmchannel object by key.

 * This function returns xtmchannel object instance by key.
 * \param[in] index_    Object key
 * \param[out] xtmchannel_obj    Object handle
 * \return    0=OK or error <0
 */
static inline int rdpa_xtmchannel_get(bdmf_number index_, bdmf_object_handle *xtmchannel_obj)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_XTMCHANNEL_GET;
	pa.parm = (uint64_t)(long)index_;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMCHANNEL_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return ret;
	}

	*xtmchannel_obj = pa.mo;
	close(fd);
	return pa.ret;
}

/** Get xtmchannel/index attribute.
 *
 * Get XTMCHANNEL index.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmchannel_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)index_;
	pa.cmd = RDPA_XTMCHANNEL_INDEX_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMCHANNEL_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set xtmchannel/index attribute.
 *
 * Set XTMCHANNEL index.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmchannel_index_set(bdmf_object_handle mo_, bdmf_number index_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)index_;
	pa.cmd = RDPA_XTMCHANNEL_INDEX_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMCHANNEL_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get xtmchannel/egress_tm attribute.
 *
 * Get US scheduler object.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[out]  egress_tm_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmchannel_egress_tm_get(bdmf_object_handle mo_, bdmf_object_handle *egress_tm_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)egress_tm_;
	pa.cmd = RDPA_XTMCHANNEL_EGRESS_TM_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMCHANNEL_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set xtmchannel/egress_tm attribute.
 *
 * Set US scheduler object.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[in]   egress_tm_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmchannel_egress_tm_set(bdmf_object_handle mo_, bdmf_object_handle egress_tm_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.object = egress_tm_;
	pa.cmd = RDPA_XTMCHANNEL_EGRESS_TM_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMCHANNEL_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get xtmchannel/enable attribute.
 *
 * Get Enable XTMCHANNEL.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[out]  enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmchannel_enable_get(bdmf_object_handle mo_, bdmf_boolean *enable_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)enable_;
	pa.cmd = RDPA_XTMCHANNEL_ENABLE_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMCHANNEL_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set xtmchannel/enable attribute.
 *
 * Set Enable XTMCHANNEL.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[in]   enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmchannel_enable_set(bdmf_object_handle mo_, bdmf_boolean enable_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)enable_;
	pa.cmd = RDPA_XTMCHANNEL_ENABLE_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMCHANNEL_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get xtmchannel/orl_prty attribute.
 *
 * Get Priority for overall rate limiter.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[out]  orl_prty_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmchannel_orl_prty_get(bdmf_object_handle mo_, rdpa_tm_orl_prty *orl_prty_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)orl_prty_;
	pa.cmd = RDPA_XTMCHANNEL_ORL_PRTY_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMCHANNEL_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set xtmchannel/orl_prty attribute.
 *
 * Set Priority for overall rate limiter.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[in]   orl_prty_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmchannel_orl_prty_set(bdmf_object_handle mo_, rdpa_tm_orl_prty orl_prty_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)orl_prty_;
	pa.cmd = RDPA_XTMCHANNEL_ORL_PRTY_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMCHANNEL_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}

/** @} end of xtmchannel Doxygen group */




#endif /* _RDPA_AG_XTMCHANNEL_USR_H_ */
