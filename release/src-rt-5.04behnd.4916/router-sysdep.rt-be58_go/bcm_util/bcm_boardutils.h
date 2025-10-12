/***********************************************************************
 *
 *  Copyright (c) 2020  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2020:DUAL/GPL:standard
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


/*!\enum RebootReasonType
 * \brief  Enumeration of possible reasons for reboot system call
 *
 */
typedef enum 
{
    REBOOT_REASON_NULL                          = 0, /**< no specific reason or unknown reason */
    REBOOT_REASON_SOFTWARE_UPGRADE              = 1, /**< software upgrade caused reboot */
    REBOOT_REASON_MANAGEMENT_REBOOT             = 2, /**< tr69/web/omci/oam asked for reboot */    
    REBOOT_REASON_RESTORE_DEFAULT               = 3, /**< configuration restore default */
    REBOOT_REASON_HARDWARE_ABNORMALITY          = 4, /**< hardware abnormalilty detected, app decided to reboot  */
} RebootReasonType;


/** Return 1 if system bootloader is UBoot, otherwise, return 0. */
int bcmUtl_isBootloaderUboot(void);

/** Generic helper function to get any UBoot env var.
 *
 * @param varName (IN) name of the environment var to read.
 * @param buf    (OUT) buffer to store the value of the env var.
 * @param bufLen (IN/OUT) on entry, this is the length of the buf.  On exit,
 *                        this will be set to the actual number of bytes copied.
 *
 * @return BCMRET_SUCCESS or some other error code if the variable does not
 *                        exist.
 */
BcmRet bcmUtl_getUbootEnvVar(const char *varName, char *buf, UINT32 *bufLen);

/** Return 1 if the Uboot no_commit_image env var is set to 1 or y.
 */
int bcmUtl_isUbootNoCommitImage(void);


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

/** Copy the name of the model name to the given buf.
 *
 * @param buf   (OUT) Buffer to hold the data.
 * @param bufLen (IN) Length of the buffer.  If the buffer is not large
 *                        enough to hold the path, an error will be returned.
 * @return BcmRet enum.
 */
BcmRet bcmUtl_getModelName(char *buf, UINT32 bufLen);

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

/** This is just a plain busybox reboot,
 *  but all CMS and BDK apps should use bcmUtl_loggedBusybox_reboot().
 */
void bcmUtl_busybox_reboot(void);

/** This is the recommended API to call to reboot the system.
 *
 * @param requestor (IN) The requestor.
 * @param reason (IN) An enum to identify the reason, will be mapped to
 *                    a string inside the function.
 *
 *  It will log a line in the kernel printk buffer, which will appear on the console,
 *  write the requestor:reason to /data/reboot_reason which can be read after boot,
 *  create a shutdown file so other apps know that we are shutting down,
 *  and do a graceful busybox reboot.
 *  The requestor may be NULL, and the reason can be REBOOT_REASON_NULL.
 *  But it would be nice to pass meaningfull values if the information is available.
 */
void bcmUtl_loggedBusybox_reboot(const char *requestor, RebootReasonType reason);

#define REBOOT_REASON_MAX_LENGTH 256
/** Get the reason of the board reboot last time.
 *
 * @param buf (IN) buffer to hold the reboot reason.
 * @param bufLen (IN) Length of the buffer.
          The length of reboot reason string will be less than REBOOT_REASON_MAX_LENGTH.
          If the buffer is not large enough, only bufLen chars are returned.
 *
 * @ret BcmRet enum.
 */
BcmRet bcmUtl_getLastRebootReason(char *buf, UINT32 bufLen);


#if defined __cplusplus
};
#endif
#endif  /* __BCM_BOARDUTILS_H__ */
