/***********************************************************************
 *
 *  Copyright (c) 2011  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
:>
 *
 ************************************************************************/

#ifndef __CMS_BOARDIOCTL_H__
#define __CMS_BOARDIOCTL_H__

#include <sys/ioctl.h>
#include "cms.h"
#include "board.h"  /* in bcmdrivers/opensource/include/bcm963xx for BOARD_IOCTL_ACTION */


/*!\file cms_boardioctl.h
 * 
 * ****************** WARNING: THIS FILE IS DEPRECATED ****************
 *
 * new code should use bcm_boarddriverctl.h (or bcm_boardctl.h
 * or bcm_flashutil.h)
 *
 */

CmsRet devCtl_boardIoctl(UINT32 boardIoctl,
                         BOARD_IOCTL_ACTION action,
                         char *string,
                         SINT32 strLen,
                         SINT32 offset,
                         void *data);


#endif /* __CMS_BOARDIOCTL_H__ */
