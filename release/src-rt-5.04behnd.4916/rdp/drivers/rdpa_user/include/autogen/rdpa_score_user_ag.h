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
 * score object user header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_SCORE_USR_H_
#define _RDPA_AG_SCORE_USR_H_

#include <sys/ioctl.h>
#include "rdpa_user.h"
#include "rdpa_user_types.h"
#include "rdpa_score_user_ioctl_ag.h"

/** \addtogroup score
 * @{
 */


/** Get score type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a score object.
 * \return score type handle
 */
static inline bdmf_type_handle rdpa_score_drv(void)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_SCORE_DRV;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return 0;
	}

	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return 0;
	}

	close(fd);
	return pa.drv;
}

/** Get score object.

 * This function returns score object instance.
 * \param[out] score_obj    Object handle
 * \return    0=OK or error <0
 */
static inline int rdpa_score_get(bdmf_object_handle *score_obj)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_SCORE_GET;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return ret;
	}

	*score_obj = pa.mo;
	close(fd);
	return pa.ret;
}

/** Get score/nstreams attribute.
 *
 * Get number of enabled streams.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[out]  nstreams_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_score_nstreams_get(bdmf_object_handle mo_, bdmf_number *nstreams_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)nstreams_;
	pa.cmd = RDPA_SCORE_NSTREAMS_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get score/stream attribute entry.
 *
 * Get SCORE stream entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  stream_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_stream_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_score_stream_info_t * stream_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(bdmf_long_t)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)stream_;
	pa.cmd = RDPA_SCORE_STREAM_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set score/stream attribute entry.
 *
 * Set SCORE stream entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   stream_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_stream_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_score_stream_info_t * stream_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(bdmf_long_t)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)stream_;
	pa.cmd = RDPA_SCORE_STREAM_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Add score/stream attribute entry.
 *
 * Add SCORE stream entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   stream_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_stream_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_score_stream_info_t * stream_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)stream_;
	pa.cmd = RDPA_SCORE_STREAM_ADD;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Delete score/stream attribute entry.
 *
 * Delete SCORE stream entry.
 * \param[in]   mo_ score object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_stream_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.cmd = RDPA_SCORE_STREAM_DELETE;
	pa.ai = (bdmf_index)(bdmf_long_t)ai_;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get next score/stream attribute entry.
 *
 * Get next SCORE stream entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_stream_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.cmd = RDPA_SCORE_STREAM_GET_NEXT;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Find score/stream attribute entry.
 *
 * Find SCORE stream entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   stream_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_stream_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_score_stream_info_t * stream_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)stream_;
	pa.cmd = RDPA_SCORE_STREAM_FIND;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get score/nprofiles attribute.
 *
 * Get number of configured profiles.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[out]  nprofiles_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_score_nprofiles_get(bdmf_object_handle mo_, bdmf_number *nprofiles_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)nprofiles_;
	pa.cmd = RDPA_SCORE_NPROFILES_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get score/profile attribute entry.
 *
 * Get SCORE profile entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  profile_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_profile_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_score_profile_cfg_t * profile_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(bdmf_long_t)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)profile_;
	pa.cmd = RDPA_SCORE_PROFILE_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set score/profile attribute entry.
 *
 * Set SCORE profile entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   profile_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_profile_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_score_profile_cfg_t * profile_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(bdmf_long_t)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)profile_;
	pa.cmd = RDPA_SCORE_PROFILE_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Add score/profile attribute entry.
 *
 * Add SCORE profile entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   profile_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_profile_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_score_profile_cfg_t * profile_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)profile_;
	pa.cmd = RDPA_SCORE_PROFILE_ADD;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Delete score/profile attribute entry.
 *
 * Delete SCORE profile entry.
 * \param[in]   mo_ score object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_profile_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.cmd = RDPA_SCORE_PROFILE_DELETE;
	pa.ai = (bdmf_index)(bdmf_long_t)ai_;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get next score/profile attribute entry.
 *
 * Get next SCORE profile entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_profile_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.cmd = RDPA_SCORE_PROFILE_GET_NEXT;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Find score/profile attribute entry.
 *
 * Find SCORE profile entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   profile_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_profile_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_score_profile_cfg_t * profile_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)profile_;
	pa.cmd = RDPA_SCORE_PROFILE_FIND;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get score/nsdrs attribute.
 *
 * Get number of configured SDRs.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[out]  nsdrs_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_score_nsdrs_get(bdmf_object_handle mo_, bdmf_number *nsdrs_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)nsdrs_;
	pa.cmd = RDPA_SCORE_NSDRS_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get score/score_irq_coalescing_num attribute.
 *
 * Get number of reports to gather per interrupt.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[out]  score_irq_coalescing_num_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_score_score_irq_coalescing_num_get(bdmf_object_handle mo_, bdmf_number *score_irq_coalescing_num_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)score_irq_coalescing_num_;
	pa.cmd = RDPA_SCORE_SCORE_IRQ_COALESCING_NUM_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set score/score_irq_coalescing_num attribute.
 *
 * Set number of reports to gather per interrupt.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   score_irq_coalescing_num_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_score_score_irq_coalescing_num_set(bdmf_object_handle mo_, bdmf_number score_irq_coalescing_num_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)score_irq_coalescing_num_;
	pa.cmd = RDPA_SCORE_SCORE_IRQ_COALESCING_NUM_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get score/score_irq_coalescing_timeout attribute.
 *
 * Get timeout for pending reports in ms.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[out]  score_irq_coalescing_timeout_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_score_irq_coalescing_timeout_get(bdmf_object_handle mo_, bdmf_number *score_irq_coalescing_timeout_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)score_irq_coalescing_timeout_;
	pa.cmd = RDPA_SCORE_SCORE_IRQ_COALESCING_TIMEOUT_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set score/score_irq_coalescing_timeout attribute.
 *
 * Set timeout for pending reports in ms.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   score_irq_coalescing_timeout_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_score_irq_coalescing_timeout_set(bdmf_object_handle mo_, bdmf_number score_irq_coalescing_timeout_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)score_irq_coalescing_timeout_;
	pa.cmd = RDPA_SCORE_SCORE_IRQ_COALESCING_TIMEOUT_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get score/score_enable attribute.
 *
 * Get global enable SCORE.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[out]  score_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_score_enable_get(bdmf_object_handle mo_, bdmf_boolean *score_enable_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)score_enable_;
	pa.cmd = RDPA_SCORE_SCORE_ENABLE_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set score/score_enable attribute.
 *
 * Set global enable SCORE.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   score_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_score_enable_set(bdmf_object_handle mo_, bdmf_boolean score_enable_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)score_enable_;
	pa.cmd = RDPA_SCORE_SCORE_ENABLE_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set score/qstat_cfg attribute.
 *
 * Set Qstat configure.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   qstat_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_qstat_cfg_set(bdmf_object_handle mo_, bdmf_number qstat_cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)qstat_cfg_;
	pa.cmd = RDPA_SCORE_QSTAT_CFG_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SCORE_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}

/** @} end of score Doxygen group */




#endif /* _RDPA_AG_SCORE_USR_H_ */
