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
 * ip_class object user header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_IP_CLASS_USR_H_
#define _RDPA_AG_IP_CLASS_USR_H_

#include <sys/ioctl.h>
#include "rdpa_user.h"
#include "rdpa_user_types.h"
#include "rdpa_ip_class_user_ioctl_ag.h"

/** \addtogroup ip_class
 * @{
 */


/** Get ip_class type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create an ip_class object.
 * \return ip_class type handle
 */
static inline bdmf_type_handle rdpa_ip_class_drv(void)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_IP_CLASS_DRV;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return 0;
	}

	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return 0;
	}

	close(fd);
	return pa.drv;
}

/** Get ip_class object.

 * This function returns ip_class object instance.
 * \param[out] ip_class_obj    Object handle
 * \return    0=OK or error <0
 */
static inline int rdpa_ip_class_get(bdmf_object_handle *ip_class_obj)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_IP_CLASS_GET;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return ret;
	}

	*ip_class_obj = pa.mo;
	close(fd);
	return pa.ret;
}

/** Get ip_class/nflows attribute.
 *
 * Get number of configured IP flows (5-tuple, 6-tuple, or 3-tuple).
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[out]  nflows_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_nflows_get(bdmf_object_handle mo_, bdmf_number *nflows_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)nflows_;
	pa.cmd = RDPA_IP_CLASS_NFLOWS_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get ip_class/flow_stat attribute entry.
 *
 * Get 5-tuple based IP flow entry statistics.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_flow_stat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * flow_stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)flow_stat_;
	pa.cmd = RDPA_IP_CLASS_FLOW_STAT_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get next ip_class/flow_stat attribute entry.
 *
 * Get next 5-tuple based IP flow entry statistics.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_flow_stat_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai_ptr = (bdmf_ptr)(unsigned long)ai_;
	pa.cmd = RDPA_IP_CLASS_FLOW_STAT_GET_NEXT;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set ip_class/flush attribute.
 *
 * Set Flush flows.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   flush_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_flush_set(bdmf_object_handle mo_, bdmf_boolean flush_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)flush_;
	pa.cmd = RDPA_IP_CLASS_FLUSH_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get ip_class/l4_filter attribute entry.
 *
 * Get L4 filter configuration.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  l4_filter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_l4_filter_get(bdmf_object_handle mo_, rdpa_l4_filter_index ai_, rdpa_l4_filter_cfg_t * l4_filter_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)l4_filter_;
	pa.cmd = RDPA_IP_CLASS_L4_FILTER_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set ip_class/l4_filter attribute entry.
 *
 * Set L4 filter configuration.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   l4_filter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_l4_filter_set(bdmf_object_handle mo_, rdpa_l4_filter_index ai_, const rdpa_l4_filter_cfg_t * l4_filter_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)l4_filter_;
	pa.cmd = RDPA_IP_CLASS_L4_FILTER_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get ip_class/l4_filter_stat attribute entry.
 *
 * Get L4 filter statistics.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  l4_filter_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_l4_filter_stat_get(bdmf_object_handle mo_, rdpa_l4_filter_index ai_, uint32_t *l4_filter_stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)l4_filter_stat_;
	pa.cmd = RDPA_IP_CLASS_L4_FILTER_STAT_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get ip_class/key_type attribute.
 *
 * Get IP class key type.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[out]  key_type_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_key_type_get(bdmf_object_handle mo_, rdpa_key_type *key_type_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)key_type_;
	pa.cmd = RDPA_IP_CLASS_KEY_TYPE_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set ip_class/key_type attribute.
 *
 * Set IP class key type.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   key_type_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_key_type_set(bdmf_object_handle mo_, rdpa_key_type key_type_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)key_type_;
	pa.cmd = RDPA_IP_CLASS_KEY_TYPE_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get ip_class/pathstat attribute entry.
 *
 * Get Ip class path entry statistics.

 * \deprecated This function has been deprecated.
 *
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  pathstat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_pathstat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * pathstat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)pathstat_;
	pa.cmd = RDPA_IP_CLASS_PATHSTAT_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get ip_class/tcp_ack_prio attribute.
 *
 * Get TCP pure ACK prioritization (common for L3 and L2).
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[out]  tcp_ack_prio_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_tcp_ack_prio_get(bdmf_object_handle mo_, bdmf_boolean *tcp_ack_prio_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)tcp_ack_prio_;
	pa.cmd = RDPA_IP_CLASS_TCP_ACK_PRIO_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set ip_class/tcp_ack_prio attribute.
 *
 * Set TCP pure ACK prioritization (common for L3 and L2).
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   tcp_ack_prio_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_tcp_ack_prio_set(bdmf_object_handle mo_, bdmf_boolean tcp_ack_prio_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)tcp_ack_prio_;
	pa.cmd = RDPA_IP_CLASS_TCP_ACK_PRIO_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get ip_class/tos_mflows attribute.
 *
 * Get TOS multi-flows (common for L3 and L2).
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[out]  tos_mflows_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_tos_mflows_get(bdmf_object_handle mo_, bdmf_boolean *tos_mflows_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)tos_mflows_;
	pa.cmd = RDPA_IP_CLASS_TOS_MFLOWS_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set ip_class/tos_mflows attribute.
 *
 * Set TOS multi-flows (common for L3 and L2).
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   tos_mflows_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_tos_mflows_set(bdmf_object_handle mo_, bdmf_boolean tos_mflows_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)tos_mflows_;
	pa.cmd = RDPA_IP_CLASS_TOS_MFLOWS_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get ip_class/pbit_key_mode attribute.
 *
 * Get create ucast keys with VLAN's pbit - enable only via scratchpad key PbitKeyMode.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[out]  pbit_key_mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_pbit_key_mode_get(bdmf_object_handle mo_, bdmf_boolean *pbit_key_mode_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)pbit_key_mode_;
	pa.cmd = RDPA_IP_CLASS_PBIT_KEY_MODE_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set ip_class/pbit_key_mode attribute.
 *
 * Set create ucast keys with VLAN's pbit - enable only via scratchpad key PbitKeyMode.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   pbit_key_mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_pbit_key_mode_set(bdmf_object_handle mo_, bdmf_boolean pbit_key_mode_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)pbit_key_mode_;
	pa.cmd = RDPA_IP_CLASS_PBIT_KEY_MODE_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_IP_CLASS_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}

/** @} end of ip_class Doxygen group */




#endif /* _RDPA_AG_IP_CLASS_USR_H_ */
