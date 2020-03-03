/***********************************************************************
 *
 *  Copyright (c) 2011  Broadcom Corporation
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
