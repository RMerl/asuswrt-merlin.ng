/*
<:copyright-broadcom

 Copyright (c) 2007 Broadcom Corporation
 All Rights Reserved
 No portions of this material may be reproduced in any form without the
 written permission of:
          Broadcom Corporation
          5300 California Avenue
          Irvine, California 92617
 All information contained in this document is Broadcom Corporation
 company private, proprietary, and trade secret.

:>
*/
#ifndef BCM_GPON_API_COMMON_H
#define BCM_GPON_API_COMMON_H

#include <bcmtypes.h>

/*Parameter Tags indicating whether the parameter is an input, output, or input/output argument*/
#ifndef IN
#define IN
#endif /*IN*/

#ifndef OUT
#define OUT
#endif /*OUT*/

#ifndef INOUT
#define INOUT
#endif /*INOUT*/

/*Type of ioctl argument pointer for command BCM_PLOAM_IOC_GET_DRIVER_VERSION
 *and BCM_OMCI_IOC_GET_DRIVER_VERSION.*/
typedef struct {
  OUT UINT8 driverMajor;
  OUT UINT8 driverMinor;
  OUT UINT8 driverFix;
  OUT UINT8 apiMajor;
  OUT UINT8 apiMinor;
} BCM_Gpon_DriverVersionInfo;

#endif /*BCM_GPON_API_COMMON_H*/
