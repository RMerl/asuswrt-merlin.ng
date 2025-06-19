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


#ifndef __BCM_BOARDDRIVERCTL_H__
#define __BCM_BOARDDRIVERCTL_H__

#include <sys/ioctl.h>
#include "board.h"  // for BOARD_IOCTL_ACTION, requires special include path
#include "bcm_retcodes.h"
#include "number_defs.h"
#include "bcm_boardctl.h"


/*!\file bcm_boarddriverctl.h
 * \brief Header file for various board ioctls.
 *
 */

/** This was the original board driver ioctl function.  Despite its name,
 *  it might actually perform the requested action on a file instead of
 *  doing a real ioctl to the board driver.  Because it may be file based,
 *  it is now implemented in libbcm_flashutil.so.  For simple board ioctls
 *  which do not involve the config file or scratchpad, look in bcm_boardctl.h.
 *
 * @param boardIoctl (IN) The ioctl to perform.
 * @param action     (IN) The sub-action associated with the ioctl.
 * @param string     (IN) Input data for the ioctl.
 * @param strLen     (IN) Length of input data.
 * @param offset     (IN) Offset for the ioctl/sub-action.
 * @param data   (IN/OUT) Depends on the ioctl/sub-action.  Could be used
 *                        to pass in additional data or be used to return data
 *                        from the ioctl.
 * @return BcmRet enum.
 */
BcmRet devCtl_boardIoctl(UINT32 boardIoctl,
                          BOARD_IOCTL_ACTION action,
                          char *string,
                          SINT32 strLen,
                          SINT32 offset,
                          void *data);


/** Do a real ioctl to the board driver.  Most callers should probably use
 *  devCtl_boardIoctl or one of the wrapper functions in bcm_boardctl.h.
 */
BcmRet devCtl_boardDriverIoctl(UINT32 boardIoctl,
                         BOARD_IOCTL_ACTION action,
                         char *string,
                         SINT32 strLen,
                         SINT32 offset,
                         void *data);


#endif /* __BCM_BOARDDRIVERCTL_H__ */
