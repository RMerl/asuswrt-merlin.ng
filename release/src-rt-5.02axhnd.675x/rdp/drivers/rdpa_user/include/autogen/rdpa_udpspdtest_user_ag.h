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
 * udpspdtest object user header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_UDPSPDTEST_USR_H_
#define _RDPA_AG_UDPSPDTEST_USR_H_

#include <sys/ioctl.h>
#include "rdpa_user.h"
#include "rdpa_user_types.h"
#include "rdpa_udpspdtest_user_ioctl_ag.h"

/** \addtogroup udpspdtest
 * @{
 */


/** Get udpspdtest type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create an udpspdtest object.
 * \return udpspdtest type handle
 */
static inline bdmf_type_handle rdpa_udpspdtest_drv(void)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_UDPSPDTEST_DRV;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return 0;
	}

	ret = ioctl(fd, RDPA_UDPSPDTEST_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return 0;
	}

	close(fd);
	return pa.drv;
}

/** Get udpspdtest object.

 * This function returns udpspdtest object instance.
 * \param[out] udpspdtest_obj    Object handle
 * \return    0=OK or error <0
 */
static inline int rdpa_udpspdtest_get(bdmf_object_handle *udpspdtest_obj)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_UDPSPDTEST_GET;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_UDPSPDTEST_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return ret;
	}

	*udpspdtest_obj = pa.mo;
	close(fd);
	return pa.ret;
}

/** Get udpspdtest/cfg attribute.
 *
 * Get UDP Speed Test configuration.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[out]  cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_cfg_get(bdmf_object_handle mo_, rdpa_udpspdtest_cfg_t * cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)cfg_;
	pa.cmd = RDPA_UDPSPDTEST_CFG_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_UDPSPDTEST_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set udpspdtest/cfg attribute.
 *
 * Set UDP Speed Test configuration.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_cfg_set(bdmf_object_handle mo_, const rdpa_udpspdtest_cfg_t * cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)cfg_;
	pa.cmd = RDPA_UDPSPDTEST_CFG_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_UDPSPDTEST_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get udpspdtest/stream_stat attribute entry.
 *
 * Get UDP Speed Test stream statistics.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  stream_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_udpspdtest_stream_stat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_udpspdtest_stat_t * stream_stat_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)stream_stat_;
	pa.cmd = RDPA_UDPSPDTEST_STREAM_STAT_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_UDPSPDTEST_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get udpspdtest/rx_params attribute entry.
 *
 * Get Stream RX parameters.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  rx_params_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_rx_params_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_udpspdtest_rx_params_t * rx_params_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)rx_params_;
	pa.cmd = RDPA_UDPSPDTEST_RX_PARAMS_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_UDPSPDTEST_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set udpspdtest/rx_params attribute entry.
 *
 * Set Stream RX parameters.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   rx_params_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_rx_params_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_udpspdtest_rx_params_t * rx_params_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)rx_params_;
	pa.cmd = RDPA_UDPSPDTEST_RX_PARAMS_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_UDPSPDTEST_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set udpspdtest/rx_start attribute.
 *
 * Set Start packets receive on a stream.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   rx_start_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_rx_start_set(bdmf_object_handle mo_, bdmf_number rx_start_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)rx_start_;
	pa.cmd = RDPA_UDPSPDTEST_RX_START_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_UDPSPDTEST_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set udpspdtest/rx_stop attribute.
 *
 * Set Stop packets receive on a steam.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   rx_stop_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_rx_stop_set(bdmf_object_handle mo_, bdmf_number rx_stop_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)rx_stop_;
	pa.cmd = RDPA_UDPSPDTEST_RX_STOP_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_UDPSPDTEST_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Get udpspdtest/tx_params attribute entry.
 *
 * Get Stream TX parametes.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  tx_params_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_tx_params_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_udpspdtest_tx_params_t * tx_params_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)tx_params_;
	pa.cmd = RDPA_UDPSPDTEST_TX_PARAMS_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_UDPSPDTEST_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set udpspdtest/tx_params attribute entry.
 *
 * Set Stream TX parametes.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   tx_params_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_tx_params_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_udpspdtest_tx_params_t * tx_params_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ai = (bdmf_index)(long)ai_;
	pa.ptr = (bdmf_ptr)(unsigned long)tx_params_;
	pa.cmd = RDPA_UDPSPDTEST_TX_PARAMS_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_UDPSPDTEST_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set udpspdtest/tx_start attribute.
 *
 * Set Start packets transmission on a stream.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   tx_start_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_tx_start_set(bdmf_object_handle mo_, bdmf_number tx_start_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)tx_start_;
	pa.cmd = RDPA_UDPSPDTEST_TX_START_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_UDPSPDTEST_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}


/** Set udpspdtest/tx_stop attribute.
 *
 * Set Stop packets transmission on a steam.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   tx_stop_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_tx_stop_set(bdmf_object_handle mo_, bdmf_number tx_stop_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.parm = (uint64_t)(long)tx_stop_;
	pa.cmd = RDPA_UDPSPDTEST_TX_STOP_SET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_UDPSPDTEST_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}

/** @} end of udpspdtest Doxygen group */




#endif /* _RDPA_AG_UDPSPDTEST_USR_H_ */
