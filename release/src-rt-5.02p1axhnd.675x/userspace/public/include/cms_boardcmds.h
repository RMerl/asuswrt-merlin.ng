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


#ifndef __CMS_BOARDCMDS_H__
#define __CMS_BOARDCMDS_H__

#include <sys/ioctl.h>
#include "cms.h"

/*!\file cms_boardcmds.h
 *
 * ****************** WARNING: THIS FILE IS DEPRECATED ****************
 *
 * new code should use bcm_boardctl.h (or bcm_flashutil.h or bcm_boarddriverctl.h)
 *
 */

CmsRet devCtl_getBaseMacAddress(UINT8 *macAddrNum);
CmsRet devCtl_getMacAddress(UINT8 *macAddrNum, UINT32 ulId);
CmsRet devCtl_getMacAddresses(UINT8 *macAddrNum, UINT32 ulId, UINT32 num_addresses);
CmsRet devCtl_releaseMacAddresses(UINT8 *macAddrNum, UINT32 num_addresses);
CmsRet devCtl_releaseMacAddress(UINT8 *macAddrNum);
UINT32 devCtl_getNumEnetMacs(void);
UINT32 devCtl_getNumEnetPorts(void);
UINT32 devCtl_getSdramSize(void);
CmsRet devCtl_getChipId(UINT32 *chipId);
int devCtl_setImageState(int state);
int devCtl_getSequenceNumber(int image);
int devCtl_getImageState(void);
int devCtl_getImageVersion(int partition, char *verStr, int verStrSize);
int devCtl_getBootedImagePartition(void);
int devCtl_getBootedImageId(void);

/*
 * ****************** WARNING: THIS FILE IS DEPRECATED ****************
 *
 * new code should use bcm_boardctl.h instead
 *
 */
#if defined(EPON_SDK_BUILD)
UINT32 devCtl_getPortMacType(unsigned short port, unsigned int *mac_type);
UINT32 devCtl_getNumFePorts(unsigned int *fe_ports);
UINT32 devCtl_getNumGePorts(unsigned int *ge_ports);
UINT32 devCtl_getNumVoipPorts(unsigned int *voip_ports);
UINT32 devCtl_getPortMap(unsigned int *port_map);
#endif

#endif /* __CMS_BOARDCMDS_H__ */
