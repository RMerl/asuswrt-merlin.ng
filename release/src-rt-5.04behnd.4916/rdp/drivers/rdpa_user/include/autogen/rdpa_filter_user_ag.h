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
 * filter object user header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_FILTER_USR_H_
#define _RDPA_AG_FILTER_USR_H_

#include <sys/ioctl.h>
#include "rdpa_user.h"
#include "rdpa_user_types.h"
#include "rdpa_filter_user_ioctl_ag.h"

/** \addtogroup filter
 * @{
 */


/** Get filter type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a filter object.
 * \return filter type handle
 */
static inline bdmf_type_handle rdpa_filter_drv(void)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_FILTER_DRV;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return 0;
	}

	ret = ioctl(fd, RDPA_FILTER_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return 0;
	}

	close(fd);
	return pa.drv;
}

/** Get filter object.

 * This function returns filter object instance.
 * \param[out] filter_obj    Object handle
 * \return    0=OK or error <0
 */
static inline int rdpa_filter_get(bdmf_object_handle *filter_obj)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_FILTER_GET;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_FILTER_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return ret;
	}

	*filter_obj = pa.mo;
	close(fd);
	return pa.ret;
}

/** Get filter/global_cfg attribute.
 *
 * Get Global configuration.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[out]  global_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_global_cfg_get(bdmf_object_handle mo_, rdpa_filter_global_cfg_t * global_cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)global_cfg_;
	pa.cmd = RDPA_FILTER_GLOBAL_CFG_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_FILTER_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set filter/global_cfg attribute.
 *
 * Set Global configuration.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in]   global_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_global_cfg_set(bdmf_object_handle mo_, const rdpa_filter_global_cfg_t * global_cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)global_cfg_;
	pa.cmd = RDPA_FILTER_GLOBAL_CFG_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_FILTER_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get filter/etype_udef attribute entry.
 *
 * Get Ether-Type, User-Defined filter.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  etype_udef_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_etype_udef_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number *etype_udef_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)etype_udef_;
	pa.cmd = RDPA_FILTER_ETYPE_UDEF_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_FILTER_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set filter/etype_udef attribute entry.
 *
 * Set Ether-Type, User-Defined filter.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   etype_udef_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_etype_udef_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number etype_udef_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.parm = (uint64_t)(long)etype_udef_;
	pa.cmd = RDPA_FILTER_ETYPE_UDEF_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_FILTER_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get next filter/etype_udef attribute entry.
 *
 * Get next Ether-Type, User-Defined filter.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_etype_udef_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.cmd = RDPA_FILTER_ETYPE_UDEF_GET_NEXT;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_FILTER_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get filter/stats attribute entry.
 *
 * Get Drop statistics.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  stats_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_stats_get(bdmf_object_handle mo_, rdpa_filter_stats_key_t * ai_, bdmf_number *stats_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)stats_;
	pa.cmd = RDPA_FILTER_STATS_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_FILTER_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set filter/stats attribute entry.
 *
 * Set Drop statistics.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   stats_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_stats_set(bdmf_object_handle mo_, rdpa_filter_stats_key_t * ai_, bdmf_number stats_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.parm = (uint64_t)(long)stats_;
	pa.cmd = RDPA_FILTER_STATS_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_FILTER_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get next filter/stats attribute entry.
 *
 * Get next Drop statistics.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_stats_get_next(bdmf_object_handle mo_, rdpa_filter_stats_key_t * ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.cmd = RDPA_FILTER_STATS_GET_NEXT;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_FILTER_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}

/** @} end of filter Doxygen group */




#endif /* _RDPA_AG_FILTER_USR_H_ */
