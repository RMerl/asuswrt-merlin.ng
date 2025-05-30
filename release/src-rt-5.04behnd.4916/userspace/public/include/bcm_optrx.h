/***********************************************************************
 *
 * Copyright (c) 2019 Broadcom
 * All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
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
 * :>
 *
 ************************************************************************/
 
#ifndef __BCM_OPTRX_H__
#define __BCM_OPTRX_H__
 
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include "number_defs.h"
#include "bcm_retcodes.h"
#include "bcm_ulog.h"
#include "laser.h"
#include "opticaldet_ioctl.h"
 
typedef struct OpticalTrxData
{
    UINT32 temperature;
    UINT32 voltage;
    UINT32 biasCurrent;
    UINT32 txPower;
    UINT32 rxPower;
} OpticalTrxData;
 
 
/** Read optical transceiver raw data. It is caller's responsiblity to convert
 *  and represent the data according to the data model specification.
 *
 * @param trxDataP (IN) optical transceiver raw data.
 *
 * @return BcmRet enum.
 */
static inline BcmRet bcmOpTrx_getOpticalTrxInfo(OpticalTrxData *trxDataP)
{
#ifndef DESKTOP_LINUX
    int laserFd;
    UINT32 word;
    int ret;
 
    memset(trxDataP, 0x0, sizeof(OpticalTrxData));
 
    laserFd = open("/dev/laser_dev", O_RDWR);
    if (laserFd < 0)
    {
       bcmuLog_error("Laser driver open error");
       return BCMRET_INTERNAL_ERROR;
    }
 
    ret = ioctl(laserFd, LASER_IOCTL_GET_TEMPTURE, &word);
    if (ret < 0)
    {
        if ( errno == EIO || errno == ENODEV )
        {
            /* Laser driver not initialized/Optical module not present. Return immediately */
            bcmuLog_notice("%s - Laser driver not initialized/Optical module not present", strerror(errno));
            close(laserFd);
            return BCMRET_INTERNAL_ERROR;
        }

        bcmuLog_error("ioctl error on LASER_IOCTL_GET_TEMPTURE");
    }
    else
    {
        trxDataP->temperature = word & 0xFFFF;
    }
 
    ret = ioctl(laserFd, LASER_IOCTL_GET_VOLTAGE, &word);
    if (ret < 0)
    {
        if (errno == EOPNOTSUPP) /* VOLTAGE is optional in XFP */
        {
            trxDataP->voltage = 0;
        }
        else
        {
            bcmuLog_error("ioctl error on LASER_IOCTL_GET_VOLTAGE");
            trxDataP->voltage = 0xFFFF;
        }
    }
    else
    {
        // PMD driver uses 4-bytes for voltage: first two bytes for PMD
        // Voltage 1.0V, next two bytes for PMD Voltage 3.3V. Only use
        // PMD Voltage 3.3V
        trxDataP->voltage = word & 0xFFFF;
    }
 
    ret = ioctl(laserFd, LASER_IOCTL_GET_BIAS_CURRENT, &word);
    if (ret < 0)
    {
        bcmuLog_error("ioctl error on LASER_IOCTL_GET_BIAS_CURRENT");
    }
    else
    {
        trxDataP->biasCurrent = word & 0xFFFF;
    }
 
    ret = ioctl(laserFd, LASER_IOCTL_GET_TX_PWR, &word);
    if (ret < 0)
    {
        bcmuLog_error("ioctl error on LASER_IOCTL_GET_TX_PWR");
    }
    else
    {
        trxDataP->txPower = word & 0xFFFF;
    }
 
    ret = ioctl(laserFd, LASER_IOCTL_GET_RX_PWR, &word);
    if (ret < 0)
    {
        bcmuLog_error("ioctl error on LASER_IOCTL_GET_RX_PWR");
    }
    else
    {
        trxDataP->rxPower = word & 0xFFFF;
    }
 
    close(laserFd);
#else
    memset(trxDataP, 0x0, sizeof(OpticalTrxData));
#endif /* DESKTOP_LINUX */
 
    return BCMRET_SUCCESS;
}

/** Read optical transceiver static info(e.g. type, vendor name, vendor pn, vendor sn, wave length, etc).
 *
 * @param trxStaticInfoP (OUT) optical transceiver static info.
 *
 * @return BcmRet enum.
 */
static inline BcmRet bcmOpTrx_getOpticalTrxStaticInfo(TRX_INFOMATION *trxStaticInfoP)
{
#ifndef DESKTOP_LINUX
    int opticaldetFd;
    int ret;
 
    memset(trxStaticInfoP, 0x0, sizeof(TRX_INFOMATION));
 
    opticaldetFd = open(OPTDETECT_IOC_DEV, O_RDWR);
    if (opticaldetFd < 0)
    {
       bcmuLog_error("open %s failed! err[%s]\n", OPTDETECT_IOC_DEV, strerror(errno));
       return BCMRET_INTERNAL_ERROR;
    }
 
    ret = ioctl(opticaldetFd, OPTDETECT_IOCTL_GET_TRX_INFO, trxStaticInfoP);
    close(opticaldetFd);
    if (ret != 0)
    {
        bcmuLog_error("ioctl failed, ret=%d, err[%s]\n", ret, strerror(errno));
        return BCMRET_INTERNAL_ERROR;
    }

#else
    memset(trxStaticInfoP, 0x0, sizeof(TRX_INFOMATION));
#endif /* DESKTOP_LINUX */
 
    return BCMRET_SUCCESS;
}

#endif /* __BCM_OPTRX_H__ */
