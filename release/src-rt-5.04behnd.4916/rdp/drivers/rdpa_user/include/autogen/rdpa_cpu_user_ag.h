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
 * cpu object user header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_CPU_USR_H_
#define _RDPA_AG_CPU_USR_H_

#include <sys/ioctl.h>
#include "rdpa_user.h"
#include "rdpa_user_types.h"
#include "rdpa_cpu_user_ioctl_ag.h"

/** \addtogroup cpu
 * @{
 */


/** Get cpu type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a cpu object.
 * \return cpu type handle
 */
static inline bdmf_type_handle rdpa_cpu_drv(void)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_CPU_DRV;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return 0;
	}

	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return 0;
	}

	close(fd);
	return pa.drv;
}

/** Get cpu object by key.

 * This function returns cpu object instance by key.
 * \param[in] index_    Object key
 * \param[out] cpu_obj    Object handle
 * \return    0=OK or error <0
 */
static inline int rdpa_cpu_get(rdpa_cpu_port index_, bdmf_object_handle *cpu_obj)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_CPU_GET;
	pa.parm = (uint64_t)(long)index_;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return ret;
	}

	*cpu_obj = pa.mo;
	close(fd);
	return pa.ret;
}

/** Get cpu/index attribute.
 *
 * Get CPU interface.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_index_get(bdmf_object_handle mo_, rdpa_cpu_port *index_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)index_;
	pa.cmd = RDPA_CPU_INDEX_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set cpu/index attribute.
 *
 * Set CPU interface.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_index_set(bdmf_object_handle mo_, rdpa_cpu_port index_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)index_;
	pa.cmd = RDPA_CPU_INDEX_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get cpu/num_queues attribute.
 *
 * Get Number of receive queues.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[out]  num_queues_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_num_queues_get(bdmf_object_handle mo_, bdmf_number *num_queues_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)num_queues_;
	pa.cmd = RDPA_CPU_NUM_QUEUES_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set cpu/num_queues attribute.
 *
 * Set Number of receive queues.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   num_queues_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_num_queues_set(bdmf_object_handle mo_, bdmf_number num_queues_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)num_queues_;
	pa.cmd = RDPA_CPU_NUM_QUEUES_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set cpu/rxq_flush attribute entry.
 *
 * Set Flush receive queue.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   rxq_flush_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_rxq_flush_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_boolean rxq_flush_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.parm = (uint64_t)(long)rxq_flush_;
	pa.cmd = RDPA_CPU_RXQ_FLUSH_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get cpu/rxq_stat attribute entry.
 *
 * Get Rx statistics.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  rxq_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_rxq_stat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_cpu_rx_stat_t * rxq_stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)rxq_stat_;
	pa.cmd = RDPA_CPU_RXQ_STAT_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set cpu/rxq_stat attribute entry.
 *
 * Set Rx statistics.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   rxq_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_rxq_stat_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_cpu_rx_stat_t * rxq_stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)rxq_stat_;
	pa.cmd = RDPA_CPU_RXQ_STAT_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get cpu/meter_cfg attribute entry.
 *
 * Get CPU meter configuration.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  meter_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_meter_cfg_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_cpu_meter_cfg_t * meter_cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)meter_cfg_;
	pa.cmd = RDPA_CPU_METER_CFG_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set cpu/meter_cfg attribute entry.
 *
 * Set CPU meter configuration.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   meter_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_meter_cfg_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_cpu_meter_cfg_t * meter_cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)meter_cfg_;
	pa.cmd = RDPA_CPU_METER_CFG_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get cpu/meter_stat attribute entry.
 *
 * Get CPU meter drop counter.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  meter_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_meter_stat_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number *meter_stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)meter_stat_;
	pa.cmd = RDPA_CPU_METER_STAT_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set cpu/meter_stat attribute entry.
 *
 * Set CPU meter drop counter.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   meter_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_meter_stat_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number meter_stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.parm = (uint64_t)(long)meter_stat_;
	pa.cmd = RDPA_CPU_METER_STAT_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get cpu/reason_cfg attribute entry.
 *
 * Get Trap reason configuration.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  reason_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_reason_cfg_get(bdmf_object_handle mo_, rdpa_cpu_reason_index_t * ai_, rdpa_cpu_reason_cfg_t * reason_cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)reason_cfg_;
	pa.cmd = RDPA_CPU_REASON_CFG_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set cpu/reason_cfg attribute entry.
 *
 * Set Trap reason configuration.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   reason_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_reason_cfg_set(bdmf_object_handle mo_, rdpa_cpu_reason_index_t * ai_, const rdpa_cpu_reason_cfg_t * reason_cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)reason_cfg_;
	pa.cmd = RDPA_CPU_REASON_CFG_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get next cpu/reason_cfg attribute entry.
 *
 * Get next Trap reason configuration.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_reason_cfg_get_next(bdmf_object_handle mo_, rdpa_cpu_reason_index_t * ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.cmd = RDPA_CPU_REASON_CFG_GET_NEXT;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get cpu/reason_stat attribute entry.
 *
 * Get Per trap reason statistics.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  reason_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_reason_stat_get(bdmf_object_handle mo_, rdpa_cpu_reason_index_t * ai_, bdmf_number *reason_stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)reason_stat_;
	pa.cmd = RDPA_CPU_REASON_STAT_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set cpu/reason_stat attribute entry.
 *
 * Set Per trap reason statistics.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   reason_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_reason_stat_set(bdmf_object_handle mo_, rdpa_cpu_reason_index_t * ai_, bdmf_number reason_stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.parm = (uint64_t)(long)reason_stat_;
	pa.cmd = RDPA_CPU_REASON_STAT_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get next cpu/reason_stat attribute entry.
 *
 * Get next Per trap reason statistics.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_reason_stat_get_next(bdmf_object_handle mo_, rdpa_cpu_reason_index_t * ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.cmd = RDPA_CPU_REASON_STAT_GET_NEXT;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set cpu/int_connect attribute.
 *
 * Set Connect interrupts.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   int_connect_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_int_connect_set(bdmf_object_handle mo_, bdmf_boolean int_connect_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)int_connect_;
	pa.cmd = RDPA_CPU_INT_CONNECT_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get cpu/tc_to_rxq attribute entry.
 *
 * Get TC to CPU RX queue mapping.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  tc_to_rxq_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_tc_to_rxq_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number *tc_to_rxq_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)tc_to_rxq_;
	pa.cmd = RDPA_CPU_TC_TO_RXQ_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set cpu/tc_to_rxq attribute entry.
 *
 * Set TC to CPU RX queue mapping.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   tc_to_rxq_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_tc_to_rxq_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number tc_to_rxq_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.parm = (uint64_t)(long)tc_to_rxq_;
	pa.cmd = RDPA_CPU_TC_TO_RXQ_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_CPU_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}

/** @} end of cpu Doxygen group */




#endif /* _RDPA_AG_CPU_USR_H_ */
