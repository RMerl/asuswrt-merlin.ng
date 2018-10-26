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
 * iptv object user header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_IPTV_USR_H_
#define _RDPA_AG_IPTV_USR_H_

#include <sys/ioctl.h>
#include "rdpa_user.h"
#include "rdpa_user_types.h"
#include "rdpa_iptv_user_ioctl_ag.h"

/** \addtogroup iptv
 * @{
 */


/** Get iptv type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create an iptv object.
 * \return iptv type handle
 */
static inline bdmf_type_handle rdpa_iptv_drv(void)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_IPTV_DRV;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return 0;
	}

	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return 0;
	}

	close(fd);
	return pa.drv;
}

/** Get iptv object.

 * This function returns iptv object instance.
 * \param[out] iptv_obj    Object handle
 * \return    0=OK or error <0
 */
static inline int rdpa_iptv_get(bdmf_object_handle *iptv_obj)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_IPTV_GET;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return ret;
	}

	*iptv_obj = pa.mo;
	close(fd);
	return pa.ret;
}

/** Get iptv/lookup_method attribute.
 *
 * Get IPTV Lookup Method.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[out]  lookup_method_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_iptv_lookup_method_get(bdmf_object_handle mo_, rdpa_iptv_lookup_method *lookup_method_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)lookup_method_;
	pa.cmd = RDPA_IPTV_LOOKUP_METHOD_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set iptv/lookup_method attribute.
 *
 * Set IPTV Lookup Method.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   lookup_method_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_iptv_lookup_method_set(bdmf_object_handle mo_, rdpa_iptv_lookup_method lookup_method_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)lookup_method_;
	pa.cmd = RDPA_IPTV_LOOKUP_METHOD_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get iptv/mcast_prefix_filter attribute.
 *
 * Get  Multicast Prefix Filtering Method.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[out]  mcast_prefix_filter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_mcast_prefix_filter_get(bdmf_object_handle mo_, rdpa_mcast_filter_method *mcast_prefix_filter_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)mcast_prefix_filter_;
	pa.cmd = RDPA_IPTV_MCAST_PREFIX_FILTER_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set iptv/mcast_prefix_filter attribute.
 *
 * Set  Multicast Prefix Filtering Method.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   mcast_prefix_filter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_mcast_prefix_filter_set(bdmf_object_handle mo_, rdpa_mcast_filter_method mcast_prefix_filter_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)mcast_prefix_filter_;
	pa.cmd = RDPA_IPTV_MCAST_PREFIX_FILTER_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get iptv/lookup_miss_action attribute.
 *
 * Get Multicast iptv lookup miss action.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[out]  lookup_miss_action_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_lookup_miss_action_get(bdmf_object_handle mo_, rdpa_forward_action *lookup_miss_action_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)lookup_miss_action_;
	pa.cmd = RDPA_IPTV_LOOKUP_MISS_ACTION_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set iptv/lookup_miss_action attribute.
 *
 * Set Multicast iptv lookup miss action.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   lookup_miss_action_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_lookup_miss_action_set(bdmf_object_handle mo_, rdpa_forward_action lookup_miss_action_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)lookup_miss_action_;
	pa.cmd = RDPA_IPTV_LOOKUP_MISS_ACTION_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get iptv/iptv_stat attribute.
 *
 * Get IPTV global statistics.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[out]  iptv_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_iptv_stat_get(bdmf_object_handle mo_, rdpa_iptv_stat_t * iptv_stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)iptv_stat_;
	pa.cmd = RDPA_IPTV_IPTV_STAT_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set iptv/iptv_stat attribute.
 *
 * Set IPTV global statistics.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   iptv_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_iptv_stat_set(bdmf_object_handle mo_, const rdpa_iptv_stat_t * iptv_stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)iptv_stat_;
	pa.cmd = RDPA_IPTV_IPTV_STAT_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set iptv/channel_request attribute.
 *
 * Set Request to view the channel (reflecting IGMP JOIN/LEAVE membership reports).
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   channel_request_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_iptv_channel_request_set(bdmf_object_handle mo_, const rdpa_iptv_channel_request_t * channel_request_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)channel_request_;
	pa.cmd = RDPA_IPTV_CHANNEL_REQUEST_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Add iptv/channel_request attribute.
 *
 * Add Request to view the channel (reflecting IGMP JOIN/LEAVE membership reports).
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   channel_request_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_iptv_channel_request_add(bdmf_object_handle mo_, rdpa_channel_req_key_t * ai_, const rdpa_iptv_channel_request_t * channel_request_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)channel_request_;
	pa.cmd = RDPA_IPTV_CHANNEL_REQUEST_ADD;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Delete iptv/channel_request attribute entry.
 *
 * Delete Request to view the channel (reflecting IGMP JOIN/LEAVE membership reports).
 * \param[in]   mo_ iptv object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_iptv_channel_request_delete(bdmf_object_handle mo_, const rdpa_channel_req_key_t * ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.cmd = RDPA_IPTV_CHANNEL_REQUEST_DELETE;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get iptv/channel attribute entry.
 *
 * Get IPTV channels table.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  channel_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_channel_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_iptv_channel_t * channel_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)channel_;
	pa.cmd = RDPA_IPTV_CHANNEL_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get next iptv/channel attribute entry.
 *
 * Get next IPTV channels table.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_channel_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.cmd = RDPA_IPTV_CHANNEL_GET_NEXT;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Find iptv/channel attribute entry.
 *
 * Find IPTV channels table.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   channel_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_channel_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_iptv_channel_t * channel_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)channel_;
	pa.cmd = RDPA_IPTV_CHANNEL_FIND;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get iptv/channel_pm_stats attribute entry.
 *
 * Get IPTV channels Performance Monitoring statistics.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  channel_pm_stats_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_iptv_channel_pm_stats_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * channel_pm_stats_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)channel_pm_stats_;
	pa.cmd = RDPA_IPTV_CHANNEL_PM_STATS_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get next iptv/channel_pm_stats attribute entry.
 *
 * Get next IPTV channels Performance Monitoring statistics.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_iptv_channel_pm_stats_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.cmd = RDPA_IPTV_CHANNEL_PM_STATS_GET_NEXT;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set iptv/flush attribute.
 *
 * Set Flush IPTV table (remove all configured channels).
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   flush_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_flush_set(bdmf_object_handle mo_, bdmf_boolean flush_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)flush_;
	pa.cmd = RDPA_IPTV_FLUSH_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IPTV_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}

/** @} end of iptv Doxygen group */




#endif /* _RDPA_AG_IPTV_USR_H_ */
