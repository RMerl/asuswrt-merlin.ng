/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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
