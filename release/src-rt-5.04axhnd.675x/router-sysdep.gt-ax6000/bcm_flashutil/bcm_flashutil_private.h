/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
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
 * :>
 ************************************************************************/


#ifndef _BCM_FLASHUTIL_PRIVATE_H__
#define _BCM_FLASHUTIL_PRIVATE_H_

/*!\file bcm_flashutil_private.h
 * \brief Internal header file for the bcm_flashutil library.  This header
 *        is not exposed to external callers.
 */

#include "board.h"
#include "bcm_hwdefs.h"
#include "bcm_retcodes.h"
#include "number_defs.h"

#define IDENT_TAG               "@(#) $imageversion: "


int verifyImageDDRType(uint32_t wfiFlags, PNVRAM_DATA pNVRAM);
BcmRet devCtl_flashConfigAccess(UINT32 boardIoctl, BOARD_IOCTL_ACTION action,
    char *string, SINT32 strLen, SINT32 offset, void *data);


#endif /* _BCM_FLASHUTIL_PRIVATE_H_ */
