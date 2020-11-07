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
 * dscp_to_pbit object user header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_DSCP_TO_PBIT_USR_H_
#define _RDPA_AG_DSCP_TO_PBIT_USR_H_

#include <sys/ioctl.h>
#include "rdpa_user.h"
#include "rdpa_user_types.h"
#include "rdpa_dscp_to_pbit_user_ioctl_ag.h"

/** \addtogroup dscp_to_pbit
 * @{
 */


/** Get dscp_to_pbit type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a dscp_to_pbit object.
 * \return dscp_to_pbit type handle
 */
static inline bdmf_type_handle rdpa_dscp_to_pbit_drv(void)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_DSCP_TO_PBIT_DRV;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return 0;
	}

	ret = ioctl(fd, RDPA_DSCP_TO_PBIT_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return 0;
	}

	close(fd);
	return pa.drv;
}

/** Get dscp_to_pbit object by key.

 * This function returns dscp_to_pbit object instance by key.
 * \param[in] table_    Object key
 * \param[out] dscp_to_pbit_obj    Object handle
 * \return    0=OK or error <0
 */
static inline int rdpa_dscp_to_pbit_get(bdmf_number table_, bdmf_object_handle *dscp_to_pbit_obj)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_DSCP_TO_PBIT_GET;
	pa.parm = (uint64_t)(long)table_;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_DSCP_TO_PBIT_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return ret;
	}

	*dscp_to_pbit_obj = pa.mo;
	close(fd);
	return pa.ret;
}

/** Get dscp_to_pbit/table attribute.
 *
 * Get Table index.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[out]  table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dscp_to_pbit_table_get(bdmf_object_handle mo_, bdmf_number *table_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)table_;
	pa.cmd = RDPA_DSCP_TO_PBIT_TABLE_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_DSCP_TO_PBIT_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set dscp_to_pbit/table attribute.
 *
 * Set Table index.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[in]   table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dscp_to_pbit_table_set(bdmf_object_handle mo_, bdmf_number table_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)table_;
	pa.cmd = RDPA_DSCP_TO_PBIT_TABLE_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_DSCP_TO_PBIT_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get dscp_to_pbit/qos_mapping attribute.
 *
 * Get Yes : qos mapping table, no : vlan action per port table.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[out]  qos_mapping_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dscp_to_pbit_qos_mapping_get(bdmf_object_handle mo_, bdmf_boolean *qos_mapping_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)qos_mapping_;
	pa.cmd = RDPA_DSCP_TO_PBIT_QOS_MAPPING_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_DSCP_TO_PBIT_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set dscp_to_pbit/qos_mapping attribute.
 *
 * Set Yes : qos mapping table, no : vlan action per port table.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[in]   qos_mapping_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dscp_to_pbit_qos_mapping_set(bdmf_object_handle mo_, bdmf_boolean qos_mapping_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)qos_mapping_;
	pa.cmd = RDPA_DSCP_TO_PBIT_QOS_MAPPING_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_DSCP_TO_PBIT_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get dscp_to_pbit/dscp_map attribute entry.
 *
 * Get DSCP PBIT array.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  dscp_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dscp_to_pbit_dscp_map_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_pbit *dscp_map_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)dscp_map_;
	pa.cmd = RDPA_DSCP_TO_PBIT_DSCP_MAP_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_DSCP_TO_PBIT_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set dscp_to_pbit/dscp_map attribute entry.
 *
 * Set DSCP PBIT array.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   dscp_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dscp_to_pbit_dscp_map_set(bdmf_object_handle mo_, bdmf_index ai_, rdpa_pbit dscp_map_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.parm = (uint64_t)(long)dscp_map_;
	pa.cmd = RDPA_DSCP_TO_PBIT_DSCP_MAP_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_DSCP_TO_PBIT_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get dscp_to_pbit/dscp_pbit_dei_map attribute entry.
 *
 * Get DSCP PBIT/DEI array.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  dscp_pbit_dei_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dscp_to_pbit_dscp_pbit_dei_map_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_pbit_dei_t * dscp_pbit_dei_map_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)dscp_pbit_dei_map_;
	pa.cmd = RDPA_DSCP_TO_PBIT_DSCP_PBIT_DEI_MAP_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_DSCP_TO_PBIT_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set dscp_to_pbit/dscp_pbit_dei_map attribute entry.
 *
 * Set DSCP PBIT/DEI array.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   dscp_pbit_dei_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dscp_to_pbit_dscp_pbit_dei_map_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_pbit_dei_t * dscp_pbit_dei_map_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)dscp_pbit_dei_map_;
	pa.cmd = RDPA_DSCP_TO_PBIT_DSCP_PBIT_DEI_MAP_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_DSCP_TO_PBIT_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}

/** @} end of dscp_to_pbit Doxygen group */




#endif /* _RDPA_AG_DSCP_TO_PBIT_USR_H_ */
