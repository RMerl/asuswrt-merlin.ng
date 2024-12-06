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
 * xtmflow object user header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_XTMFLOW_USR_H_
#define _RDPA_AG_XTMFLOW_USR_H_

#include <sys/ioctl.h>
#include "rdpa_user.h"
#include "rdpa_user_types.h"
#include "rdpa_xtmflow_user_ioctl_ag.h"

/** \addtogroup xtmflow
 * @{
 */


/** Get xtmflow type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a xtmflow object.
 * \return xtmflow type handle
 */
static inline bdmf_type_handle rdpa_xtmflow_drv(void)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_XTMFLOW_DRV;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return 0;
	}

	ret = ioctl(fd, RDPA_XTMFLOW_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return 0;
	}

	close(fd);
	return pa.drv;
}

/** Get xtmflow object by key.

 * This function returns xtmflow object instance by key.
 * \param[in] index_    Object key
 * \param[out] xtmflow_obj    Object handle
 * \return    0=OK or error <0
 */
static inline int rdpa_xtmflow_get(bdmf_number index_, bdmf_object_handle *xtmflow_obj)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_XTMFLOW_GET;
	pa.parm = (uint64_t)(long)index_;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMFLOW_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return ret;
	}

	*xtmflow_obj = pa.mo;
	close(fd);
	return pa.ret;
}

/** Get xtmflow/index attribute.
 *
 * Get xtmflow index.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)index_;
	pa.cmd = RDPA_XTMFLOW_INDEX_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMFLOW_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set xtmflow/index attribute.
 *
 * Set xtmflow index.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_index_set(bdmf_object_handle mo_, bdmf_number index_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)index_;
	pa.cmd = RDPA_XTMFLOW_INDEX_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMFLOW_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get xtmflow/hdr_type attribute.
 *
 * Get xtmflow hdr_type.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[out]  hdr_type_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_hdr_type_get(bdmf_object_handle mo_, bdmf_number *hdr_type_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)hdr_type_;
	pa.cmd = RDPA_XTMFLOW_HDR_TYPE_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMFLOW_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set xtmflow/hdr_type attribute.
 *
 * Set xtmflow hdr_type.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[in]   hdr_type_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_hdr_type_set(bdmf_object_handle mo_, bdmf_number hdr_type_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)hdr_type_;
	pa.cmd = RDPA_XTMFLOW_HDR_TYPE_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMFLOW_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get xtmflow/fstat attribute.
 *
 * Get xtmflow fstat.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[out]  fstat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_fstat_get(bdmf_object_handle mo_, bdmf_number *fstat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)fstat_;
	pa.cmd = RDPA_XTMFLOW_FSTAT_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMFLOW_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set xtmflow/fstat attribute.
 *
 * Set xtmflow fstat.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[in]   fstat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_fstat_set(bdmf_object_handle mo_, bdmf_number fstat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)fstat_;
	pa.cmd = RDPA_XTMFLOW_FSTAT_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMFLOW_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get xtmflow/us_cfg attribute.
 *
 * Get US configuration.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[out]  us_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmflow_us_cfg_get(bdmf_object_handle mo_, rdpa_xtmflow_us_cfg_t * us_cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)us_cfg_;
	pa.cmd = RDPA_XTMFLOW_US_CFG_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMFLOW_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set xtmflow/us_cfg attribute.
 *
 * Set US configuration.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[in]   us_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmflow_us_cfg_set(bdmf_object_handle mo_, const rdpa_xtmflow_us_cfg_t * us_cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)us_cfg_;
	pa.cmd = RDPA_XTMFLOW_US_CFG_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMFLOW_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get xtmflow/stat attribute.
 *
 * Get xtmflow statistics.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[out]  stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmflow_stat_get(bdmf_object_handle mo_, rdpa_xtmflow_stat_t * stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)stat_;
	pa.cmd = RDPA_XTMFLOW_STAT_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMFLOW_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set xtmflow/stat attribute.
 *
 * Set xtmflow statistics.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[in]   stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmflow_stat_set(bdmf_object_handle mo_, const rdpa_xtmflow_stat_t * stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)stat_;
	pa.cmd = RDPA_XTMFLOW_STAT_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMFLOW_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get xtmflow/ptmBonding attribute.
 *
 * Get xtmflow ptmBonding.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[out]  ptmBonding_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_ptmBonding_get(bdmf_object_handle mo_, bdmf_number *ptmBonding_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)ptmBonding_;
	pa.cmd = RDPA_XTMFLOW_PTMBONDING_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMFLOW_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set xtmflow/ptmBonding attribute.
 *
 * Set xtmflow ptmBonding.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[in]   ptmBonding_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_ptmBonding_set(bdmf_object_handle mo_, bdmf_number ptmBonding_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)ptmBonding_;
	pa.cmd = RDPA_XTMFLOW_PTMBONDING_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTMFLOW_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}

/** @} end of xtmflow Doxygen group */




#endif /* _RDPA_AG_XTMFLOW_USR_H_ */
