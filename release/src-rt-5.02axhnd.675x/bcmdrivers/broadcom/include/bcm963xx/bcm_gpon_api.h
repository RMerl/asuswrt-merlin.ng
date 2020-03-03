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

#ifndef BCM_GPON_API_H
#define BCM_GPON_API_H

#include <bcmtypes.h>

/* Legacy GPON driver API: */

#define MAXALLOCID    32
#define REALALLOCIDS  32
#define MAXPORTID     33

#define IOC_PLOAM_BUFFER_SIZE  1
#define IOC_PLOAM_WRPTR        2
#define IOC_PLOAM_SET_GEMPORT  3

typedef struct {
    UINT16 enable;
    UINT16 portIndex;
    UINT16 portId;
    UINT16 allocId;
} iocGemPort_t;

#endif /* BCM_GPON_API_H */
