/***********************************************************************
 *
 * Copyright (c) 2019 Broadcom
 * All Rights Reserved
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
 *
 ************************************************************************/
 
#ifndef __BCM_OPTRX_H__
#define __BCM_OPTRX_H__
 
#include <fcntl.h>
#include "number_defs.h"
#include "cms_retcodes.h"
#include "cms_log.h"
#include "laser.h"
 
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
 * @return CmsRet enum.
 */
static inline CmsRet bcmOpTrx_getOpticalTrxInfo(OpticalTrxData *trxDataP)
{
    int laserFd;
    UINT32 word;
    int ret;
 
    memset(trxDataP, 0x0, sizeof(OpticalTrxData));
 
    laserFd = open("/dev/laser_dev", O_RDWR);
    if (laserFd < 0)
    {
       cmsLog_error("Laser driver open error");
       return CMSRET_INTERNAL_ERROR;
    }
 
    ret = ioctl(laserFd, LASER_IOCTL_GET_TEMPTURE, &word);
    if (ret < 0)
    {
        cmsLog_error("ioctl error on LASER_IOCTL_GET_TEMPTURE");
    }
    else
    {
        trxDataP->temperature = word & 0xFFFF;
    }
 
    ret = ioctl(laserFd, LASER_IOCTL_GET_VOLTAGE, &word);
    if (ret < 0)
    {
        cmsLog_error("ioctl error on LASER_IOCTL_GET_VOTAGE");
    }
    else
    {
        trxDataP->voltage = word & 0xFFFF;
    }
 
    ret = ioctl(laserFd, LASER_IOCTL_GET_BIAS_CURRENT, &word);
    if (ret < 0)
    {
        cmsLog_error("ioctl error on LASER_IOCTL_GET_BIAS_CURRENT");
    }
    else
    {
        trxDataP->biasCurrent = word & 0xFFFF;
    }
 
    ret = ioctl(laserFd, LASER_IOCTL_GET_TX_PWR, &word);
    if (ret < 0)
    {
        cmsLog_error("ioctl error on LASER_IOCTL_GET_TX_PWR");
    }
    else
    {
        trxDataP->txPower = word & 0xFFFF;
    }
 
    ret = ioctl(laserFd, LASER_IOCTL_GET_RX_PWR, &word);
    if (ret < 0)
    {
        cmsLog_error("ioctl error on LASER_IOCTL_GET_RX_PWR");
    }
    else
    {
        trxDataP->rxPower = word & 0xFFFF;
    }
 
    close(laserFd);
 
    return CMSRET_SUCCESS;
}
 
#endif /* __BCM_OPTRX_H__ */