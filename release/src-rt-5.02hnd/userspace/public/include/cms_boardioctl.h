/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
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
 * \brief Header file for the Board Control API, board ioctl portion.
 *
 * This file is separate from all the other commands because if a program
 * wants to call devCtl_boardIoctl, it needs to include a bunch of header
 * files from the bcm kernel driver directories.  Most apps will not need
 * to call the boardIoctl directly.
 *
 */

/* Device filename of ioctl device.  Moved to bcmdrivers/opensource/include/bcm963xx/board.h
 * #define BOARD_DEVICE_NAME  "/dev/brcmboard"
 */


/** Do board ioctl.
 *
 * @param boardIoctl (IN) The ioctl to perform.
 * @param action     (IN) The sub-action associated with the ioctl.
 * @param string     (IN) Input data for the ioctl.
 * @param strLen     (IN) Length of input data.
 * @param offset     (IN) Offset for the ioctl/sub-action.
 * @param data   (IN/OUT) Depends on the ioctl/sub-action.  Could be used
 *                        to pass in additional data or be used to return data
 *                        from the ioctl.
 * @return CmsRet enum.
 */
CmsRet devCtl_boardIoctl(UINT32 boardIoctl,
                         BOARD_IOCTL_ACTION action,
                         char *string,
                         SINT32 strLen,
                         SINT32 offset,
                         void *data);


#endif /* __CMS_BOARDIOCTL_H__ */
