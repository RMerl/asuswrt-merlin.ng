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


#ifndef __CMS_BOARDCMDS_H__
#define __CMS_BOARDCMDS_H__

#include <sys/ioctl.h>
#include "cms.h"

/*!\file cms_boardcmds.h
 * \brief Header file for the Board Control Command API.
 *
 * These functions are the simple board control functions that other apps,
 * including GPL apps, may need.  These functions are mostly just wrappers
 * around devCtl_boardIoctl().  The nice things about this file is that
 * it does not require the program to link against additional bcm kernel
 * driver header files.
 *
 */


/** Get the board's br0 interface mac address.
 * 
 * @param macAddrNum (OUT) The user must pass in an array of UINT8 of at least
 *                         MAC_ADDR_LEN (6) bytes long.
 * 
 * @return CmsRet enum.
 */
CmsRet devCtl_getBaseMacAddress(UINT8 *macAddrNum);


/** Get the available interface mac address.
 * 
 * @param macAddrNum (OUT) The user must pass in an array of UINT8 of at least
 *                         MAC_ADDR_LEN (6) bytes long.
 * 
 * @return CmsRet enum.
 */
CmsRet devCtl_getMacAddress(UINT8 *macAddrNum, UINT32 ulId);


/** Get the given number of consecutive available mac addresses.
 * 
 * @param macAddrNum (OUT) The user must pass in an array of UINT8 of at least
 *                         MAC_ADDR_LEN (6) bytes long.
 * 
 * @return CmsRet enum.
 */
CmsRet devCtl_getMacAddresses(UINT8 *macAddrNum, UINT32 ulId, UINT32 num_addresses);

/** Releases the given number of consecutive mac addresses
 * 
 * @param macAddrNum (OUT) The user must pass in an array of UINT8 of at least
 *                         MAC_ADDR_LEN (6) bytes long.
 * 
 * @return CmsRet enum.
 */
CmsRet devCtl_releaseMacAddresses(UINT8 *macAddrNum, UINT32 num_addresses);

/** Release the interface mac address that is not used anymore
 * 
 * @param macAddrNum (OUT) The user must pass in an array of UINT8 of at least
 *                         MAC_ADDR_LEN (6) bytes long.
 * 
 * @return CmsRet enum.
 */
CmsRet devCtl_releaseMacAddress(UINT8 *macAddrNum);


/** Get the number of ethernet MACS on the system.
 * 
 * @return number of ethernet MACS.
 */
UINT32 devCtl_getNumEnetMacs(void);


/** Get the number of ethernet ports on the system.
 * 
 * @return number of ethernet ports.
 */
UINT32 devCtl_getNumEnetPorts(void);


/** Get SDRAM size on the system.
 * 
 * @return SDRAM size in number of bytes.
 */
UINT32 devCtl_getSdramSize(void);


/** Get the chipId.
 *
 * This info is used in various places, including CLI and writing new
 * flash image.  It may be accessed by GPL apps, so it cannot be put
 * exclusively in the data model.
 *  
 * @param chipId (OUT) The chip id returned by the kernel.
 * @return CmsRet enum.
 */
CmsRet devCtl_getChipId(UINT32 *chipId);


/** Set the boot image state.
 * @param state (IN)   BOOT_SET_NEW_IMAGE, BOOT_SET_OLD_IMAGE,
 *                     BOOT_SET_NEW_IMAGE_ONCE,
 *                     BOOT_SET_PART1_IMAGE, BOOT_SET_PART2_IMAGE,
 *                     BOOT_SET_PART1_IMAGE_ONCE, BOOT_SET_PART2_IMAGE_ONCE
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_setImageState(int state);


/** Get the sequence number of the image
 *
 * @param image (IN)   image number to get the sequence number from (1 or 2)
 *
 * @return int         sequence number
 */
int devCtl_getSequenceNumber(int image);


/** Get the boot image state.
 *
 * @return             BOOT_SET_PART1_IMAGE, BOOT_SET_PART2_IMAGE,
 *                     BOOT_SET_PART1_IMAGE_ONCE, BOOT_SET_PART2_IMAGE_ONCE
 *
 */
int devCtl_getImageState(void);


/** Get image version string.
 *
 * @return number of bytes copied into verStr
 */
int devCtl_getImageVersion(int partition, char *verStr, int verStrSize);
 
 
/** Get the booted image partition.
 *
 * @return             BOOTED_PART1_IMAGE, BOOTED_PART2_IMAGE
 */
int devCtl_getBootedImagePartition(void);


/** Get the booted image id.
 *
 * @return             BOOTED_NEW_IMAGE, BOOTED_OLD_IMAGE
 */
int devCtl_getBootedImageId(void);

#if defined(EPON_SDK_BUILD)
/** Get mac type for a specified uni port
 *
 *  @return success or failure.
 */

UINT32 devCtl_getPortMacType(unsigned short port, unsigned int *mac_type);
    
/** Get the number of FE ports on the system.
 * 
 * @return success or failure.
 */
UINT32 devCtl_getNumFePorts(unsigned int *fe_ports);

/** Get the number of GE ports on the system.
 * 
 * @return success or failure.
 */
UINT32 devCtl_getNumGePorts(unsigned int *ge_ports);

/** Get the number of VoIP ports on the system.
 * 
 * @return success or failure.
 */
UINT32 devCtl_getNumVoipPorts(unsigned int *voip_ports);

/** Get the port_map of internal switch ports used.
 * 
 * @return success or failure.
 */
UINT32 devCtl_getPortMap(unsigned int *port_map);

#endif

#endif /* __CMS_BOARDCMDS_H__ */
