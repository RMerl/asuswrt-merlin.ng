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
 * egress_tm object user header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_EGRESS_TM_USR_H_
#define _RDPA_AG_EGRESS_TM_USR_H_

#include <sys/ioctl.h>
#include "rdpa_user.h"
#include "rdpa_user_types.h"
#include "rdpa_egress_tm_user_ioctl_ag.h"

/** \addtogroup egress_tm
 * @{
 */


/** Get egress_tm type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create an egress_tm object.
 * \return egress_tm type handle
 */
static inline bdmf_type_handle rdpa_egress_tm_drv(void)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_EGRESS_TM_DRV;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return 0;
	}

	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return 0;
	}

	close(fd);
	return pa.drv;
}


/** egress_tm object key. */
typedef struct {
    rdpa_traffic_dir dir; /**< egress_tm: Traffic Direction */
    bdmf_number index; /**< egress_tm: Egress-TM Index */
} rdpa_egress_tm_key_t;

/** Get egress_tm object by key.

 * This function returns egress_tm object instance by key.
 * \param[in] key_    Object key
 * \param[out] egress_tm_obj    Object handle
 * \return    0=OK or error <0
 */
static inline int rdpa_egress_tm_get(const rdpa_egress_tm_key_t * key_, bdmf_object_handle *egress_tm_obj)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_EGRESS_TM_GET;
	pa.ptr = (bdmf_ptr)(unsigned long)key_;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return ret;
	}

	*egress_tm_obj = pa.mo;
	close(fd);
	return pa.ret;
}

/** Get egress_tm/dir attribute.
 *
 * Get Traffic Direction.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  dir_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_dir_get(bdmf_object_handle mo_, rdpa_traffic_dir *dir_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)dir_;
	pa.cmd = RDPA_EGRESS_TM_DIR_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set egress_tm/dir attribute.
 *
 * Set Traffic Direction.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   dir_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_dir_set(bdmf_object_handle mo_, rdpa_traffic_dir dir_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)dir_;
	pa.cmd = RDPA_EGRESS_TM_DIR_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/index attribute.
 *
 * Get Egress-TM Index.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)index_;
	pa.cmd = RDPA_EGRESS_TM_INDEX_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set egress_tm/index attribute.
 *
 * Set Egress-TM Index.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_index_set(bdmf_object_handle mo_, bdmf_number index_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)index_;
	pa.cmd = RDPA_EGRESS_TM_INDEX_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/level attribute.
 *
 * Get Egress-TM Next Level.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  level_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_level_get(bdmf_object_handle mo_, rdpa_tm_level_type *level_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)level_;
	pa.cmd = RDPA_EGRESS_TM_LEVEL_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set egress_tm/level attribute.
 *
 * Set Egress-TM Next Level.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   level_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_level_set(bdmf_object_handle mo_, rdpa_tm_level_type level_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)level_;
	pa.cmd = RDPA_EGRESS_TM_LEVEL_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/mode attribute.
 *
 * Get Scheduler Operating Mode.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_mode_get(bdmf_object_handle mo_, rdpa_tm_sched_mode *mode_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)mode_;
	pa.cmd = RDPA_EGRESS_TM_MODE_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set egress_tm/mode attribute.
 *
 * Set Scheduler Operating Mode.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_mode_set(bdmf_object_handle mo_, rdpa_tm_sched_mode mode_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)mode_;
	pa.cmd = RDPA_EGRESS_TM_MODE_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/overall_rl attribute.
 *
 * Get Overall Rate Limiter.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  overall_rl_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_overall_rl_get(bdmf_object_handle mo_, bdmf_boolean *overall_rl_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)overall_rl_;
	pa.cmd = RDPA_EGRESS_TM_OVERALL_RL_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set egress_tm/overall_rl attribute.
 *
 * Set Overall Rate Limiter.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   overall_rl_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_overall_rl_set(bdmf_object_handle mo_, bdmf_boolean overall_rl_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)overall_rl_;
	pa.cmd = RDPA_EGRESS_TM_OVERALL_RL_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/service_queue attribute.
 *
 * Get Service Queue Parameters Configuration.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  service_queue_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_service_queue_get(bdmf_object_handle mo_, rdpa_tm_service_queue_t * service_queue_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)service_queue_;
	pa.cmd = RDPA_EGRESS_TM_SERVICE_QUEUE_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set egress_tm/service_queue attribute.
 *
 * Set Service Queue Parameters Configuration.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   service_queue_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_service_queue_set(bdmf_object_handle mo_, const rdpa_tm_service_queue_t * service_queue_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)service_queue_;
	pa.cmd = RDPA_EGRESS_TM_SERVICE_QUEUE_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/rl attribute.
 *
 * Get Rate Configuration.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  rl_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_rl_get(bdmf_object_handle mo_, rdpa_tm_rl_cfg_t * rl_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)rl_;
	pa.cmd = RDPA_EGRESS_TM_RL_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set egress_tm/rl attribute.
 *
 * Set Rate Configuration.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   rl_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_rl_set(bdmf_object_handle mo_, const rdpa_tm_rl_cfg_t * rl_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)rl_;
	pa.cmd = RDPA_EGRESS_TM_RL_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/rl_rate_mode attribute.
 *
 * Get Subsidiary Rate Limiter Rate Mode.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  rl_rate_mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_rl_rate_mode_get(bdmf_object_handle mo_, rdpa_tm_rl_rate_mode *rl_rate_mode_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)rl_rate_mode_;
	pa.cmd = RDPA_EGRESS_TM_RL_RATE_MODE_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set egress_tm/rl_rate_mode attribute.
 *
 * Set Subsidiary Rate Limiter Rate Mode.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   rl_rate_mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_rl_rate_mode_set(bdmf_object_handle mo_, rdpa_tm_rl_rate_mode rl_rate_mode_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)rl_rate_mode_;
	pa.cmd = RDPA_EGRESS_TM_RL_RATE_MODE_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/num_queues attribute.
 *
 * Get Number of Queues.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  num_queues_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_num_queues_get(bdmf_object_handle mo_, uint8_t *num_queues_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)num_queues_;
	pa.cmd = RDPA_EGRESS_TM_NUM_QUEUES_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set egress_tm/num_queues attribute.
 *
 * Set Number of Queues.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   num_queues_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_num_queues_set(bdmf_object_handle mo_, uint8_t num_queues_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)num_queues_;
	pa.cmd = RDPA_EGRESS_TM_NUM_QUEUES_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/num_sp_elements attribute.
 *
 * Get Number of SP Scheduling Elements for SP_WRR Scheduling Mode.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  num_sp_elements_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_num_sp_elements_get(bdmf_object_handle mo_, rdpa_tm_num_sp_elem *num_sp_elements_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)num_sp_elements_;
	pa.cmd = RDPA_EGRESS_TM_NUM_SP_ELEMENTS_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set egress_tm/num_sp_elements attribute.
 *
 * Set Number of SP Scheduling Elements for SP_WRR Scheduling Mode.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   num_sp_elements_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_num_sp_elements_set(bdmf_object_handle mo_, rdpa_tm_num_sp_elem num_sp_elements_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)num_sp_elements_;
	pa.cmd = RDPA_EGRESS_TM_NUM_SP_ELEMENTS_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/queue_cfg attribute entry.
 *
 * Get Queue Parameters Configuration.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  queue_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_cfg_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_tm_queue_cfg_t * queue_cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)queue_cfg_;
	pa.cmd = RDPA_EGRESS_TM_QUEUE_CFG_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set egress_tm/queue_cfg attribute entry.
 *
 * Set Queue Parameters Configuration.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   queue_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_cfg_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_tm_queue_cfg_t * queue_cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)queue_cfg_;
	pa.cmd = RDPA_EGRESS_TM_QUEUE_CFG_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Delete egress_tm/queue_cfg attribute entry.
 *
 * Delete Queue Parameters Configuration.
 * \param[in]   mo_ egress_tm object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_cfg_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.cmd = RDPA_EGRESS_TM_QUEUE_CFG_DELETE;
	pa.ai = (bdmf_index)(long)ai_;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/queue_statistics attribute entry.
 *
 * Get Dropped Service Queue Statistics.

 * \deprecated This function has been deprecated.
 *
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  queue_statistics_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_statistics_get(bdmf_object_handle mo_, rdpa_tm_queue_index_t * ai_, rdpa_stat_1way_t * queue_statistics_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)queue_statistics_;
	pa.cmd = RDPA_EGRESS_TM_QUEUE_STATISTICS_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get next egress_tm/queue_statistics attribute entry.
 *
 * Get next Dropped Service Queue Statistics.

 * \deprecated This function has been deprecated.
 *
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_statistics_get_next(bdmf_object_handle mo_, rdpa_tm_queue_index_t * ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.cmd = RDPA_EGRESS_TM_QUEUE_STATISTICS_GET_NEXT;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/queue_stat attribute entry.
 *
 * Get Retrieve Egress Queue Statistics.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  queue_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_stat_get(bdmf_object_handle mo_, rdpa_tm_queue_index_t * ai_, rdpa_stat_1way_t * queue_stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)queue_stat_;
	pa.cmd = RDPA_EGRESS_TM_QUEUE_STAT_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set egress_tm/queue_stat attribute entry.
 *
 * Set Retrieve Egress Queue Statistics.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   queue_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_stat_set(bdmf_object_handle mo_, rdpa_tm_queue_index_t * ai_, const rdpa_stat_1way_t * queue_stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)queue_stat_;
	pa.cmd = RDPA_EGRESS_TM_QUEUE_STAT_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get next egress_tm/queue_stat attribute entry.
 *
 * Get next Retrieve Egress Queue Statistics.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_stat_get_next(bdmf_object_handle mo_, rdpa_tm_queue_index_t * ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.cmd = RDPA_EGRESS_TM_QUEUE_STAT_GET_NEXT;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/queue_occupancy attribute entry.
 *
 * Get Retrieve Egress Queue Occupancy.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  queue_occupancy_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_occupancy_get(bdmf_object_handle mo_, rdpa_tm_queue_index_t * ai_, rdpa_stat_t * queue_occupancy_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)queue_occupancy_;
	pa.cmd = RDPA_EGRESS_TM_QUEUE_OCCUPANCY_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get next egress_tm/queue_occupancy attribute entry.
 *
 * Get next Retrieve Egress Queue Occupancy.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_occupancy_get_next(bdmf_object_handle mo_, rdpa_tm_queue_index_t * ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.cmd = RDPA_EGRESS_TM_QUEUE_OCCUPANCY_GET_NEXT;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/subsidiary attribute entry.
 *
 * Get Next Level Egress-TM.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  subsidiary_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_subsidiary_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_object_handle *subsidiary_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)subsidiary_;
	pa.cmd = RDPA_EGRESS_TM_SUBSIDIARY_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set egress_tm/subsidiary attribute entry.
 *
 * Set Next Level Egress-TM.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   subsidiary_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_subsidiary_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_object_handle subsidiary_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.object = subsidiary_;
	pa.cmd = RDPA_EGRESS_TM_SUBSIDIARY_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Find egress_tm/subsidiary attribute entry.
 *
 * Find Next Level Egress-TM.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   subsidiary_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_subsidiary_find(bdmf_object_handle mo_, bdmf_index * ai_, bdmf_object_handle *subsidiary_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)subsidiary_;
	pa.cmd = RDPA_EGRESS_TM_SUBSIDIARY_FIND;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/weight attribute.
 *
 * Get Weight for WRR scheduling (0 for unset).
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  weight_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_weight_get(bdmf_object_handle mo_, bdmf_number *weight_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)weight_;
	pa.cmd = RDPA_EGRESS_TM_WEIGHT_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set egress_tm/weight attribute.
 *
 * Set Weight for WRR scheduling (0 for unset).
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   weight_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_weight_set(bdmf_object_handle mo_, bdmf_number weight_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)weight_;
	pa.cmd = RDPA_EGRESS_TM_WEIGHT_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get egress_tm/queue_location attribute entry.
 *
 * Get Get queue location by qid.

 * \deprecated This function has been deprecated.
 *
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  queue_location_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_location_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_tm_queue_location_t * queue_location_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)queue_location_;
	pa.cmd = RDPA_EGRESS_TM_QUEUE_LOCATION_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}

/** @} end of egress_tm Doxygen group */




#endif /* _RDPA_AG_EGRESS_TM_USR_H_ */
