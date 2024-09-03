// <:copyright-BRCM:2013:DUAL/GPL:standard
// 
//    Copyright (c) 2013 Broadcom 
//    All Rights Reserved
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2, as published by
// the Free Software Foundation (the "GPL").
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// 
// A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
// writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
// 
// :>
/*
 * xtm object user header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_XTM_USR_H_
#define _RDPA_AG_XTM_USR_H_

#include <sys/ioctl.h>
#include "rdpa_user.h"
#include "rdpa_user_types.h"
#include "rdpa_xtm_user_ioctl_ag.h"

/** \addtogroup xtm
 * @{
 */


/** Get xtm type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a xtm object.
 * \return xtm type handle
 */
static inline bdmf_type_handle rdpa_xtm_drv(void)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_XTM_DRV;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return 0;
	}

	ret = ioctl(fd, RDPA_XTM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return 0;
	}

	close(fd);
	return pa.drv;
}

/** Get xtm object.

 * This function returns xtm object instance.
 * \param[out] xtm_obj    Object handle
 * \return    0=OK or error <0
 */
static inline int rdpa_xtm_get(bdmf_object_handle *xtm_obj)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret = 0;

	pa.cmd = RDPA_XTM_GET;
	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return ret;
	}

	*xtm_obj = pa.mo;
	close(fd);
	return pa.ret;
}

/** Get xtm/index attribute.
 *
 * Get XTM Index.
 * \param[in]   mo_ xtm object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtm_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)index_;
	pa.cmd = RDPA_XTM_INDEX_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_XTM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}

/** @} end of xtm Doxygen group */




#endif /* _RDPA_AG_XTM_USR_H_ */
