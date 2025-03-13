/***********************************************************************
 *
 *  Copyright (c) 2020  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2020:DUAL/GPL:standard
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

#ifndef __BCM_BOARDUTILS_H__
#define __BCM_BOARDUTILS_H__

#include "number_defs.h"
#include "bcm_retcodes.h"

#if defined __cplusplus
extern "C" {
#endif

/*!\file bcm_boardutils.h
 * \brief Helper functions for various uboot and board parameter access
 *        functions that do not need an ioctl.
 */


/** Return 1 if system bootloader is UBoot, otherwise, return 0. */
int bcmUtl_isBootloaderUboot(void);


/** Copy the name of the manufacturer to the given buf.
 *
 * This function should only be used in the DESKTOP_LINUX mode, but if
 * it is called on a real gateway build, it will return /.
 *
 * @param buf   (OUT) Buffer to hold the data.
 * @param bufLen (IN) Length of the buffer.  If the buffer is not large
 *                        enough to hold the path, an error will be returned.
 * @return BcmRet enum.
 */
BcmRet bcmUtl_getManufacturer(char *buf, UINT32 bufLen);


/** Copy the hardware version to the given buf.
 *  The behavior is the same as getManufacturer.
 */
BcmRet bcmUtl_getHardwareVersion(char *buf, UINT32 bufLen);

/** Copy the Software version to the given buf.
 *  The behavior is the same as getManufacturer.
 */
BcmRet bcmUtl_getSoftwareVersion(char *buf, UINT32 bufLen);

/** Get the base mac address.
 *
 * @param macAddrBuf (IN) Buf must be at least 17 bytes.  Note this API copies
 *         the exact 17 bytes of mac address into the buffer.  It does not
 *         write a terminating null into the buffer.
 *
 * @return BcmRet enum.
*/
BcmRet bcmUtl_getBaseMacAddress(char *macAddrBuf);

/** Get the serial number (just the mac address with colons stripped out).
 *
 * @param serialNumBuf (IN) buffer to hold the serial number.
 * @param bufLen (IN) Length of the buffer.  If the buffer is not large
 *                        enough to hold the path, an error will be returned.
 *
 * @ret BcmRet enum.
 */
BcmRet bcmUtl_getSerialNumber(char *buf, UINT32 bufLen);

/** Get the UBOOT loader version number.
 *
 * @param versionBuf (IN) buffer to hold the version number.
 * @param bufLen (IN) Length of the buffer.  If the buffer is not large
 *                        enough to hold the path, an error will be returned.
 *
 * @ret BcmRet enum.
 */
BcmRet bcmUtl_getBootloaderVersion(char* versionBuf, UINT32 bufLen);

/** Start a watchdog timer for reboot.
 *
 *  See userspace/public/apps/wdtctl/README.txt for details on how this
 *  function is used to implement the Firmware Upgrade Watchdog Timer feature.
 */
void bcmUtl_startRebootWatchdog();

#if defined __cplusplus
};
#endif
#endif  /* __BCM_BOARDUTILS_H__ */
