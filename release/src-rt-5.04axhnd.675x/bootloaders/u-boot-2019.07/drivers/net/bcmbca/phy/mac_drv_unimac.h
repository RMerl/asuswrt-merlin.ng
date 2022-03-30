// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

    
*/

/*
 *  Created on: Jun 2017
 *      Author: steven.hsieh@broadcom.com
 */

#ifndef __MAC_DRV_UNIMAC_H__
#define __MAC_DRV_UNIMAC_H__

/* definition for mac_drv priv flags */
#define UNIMAC_DRV_PRIV_FLAG_GMII_DIRECT        (1<<0)
#define UNIMAC_DRV_PRIV_FLAG_EXTSW_CONNECTED    (1<<1)
#define UNIMAC_DRV_PRIV_FLAG_SHRINK_IPG         (1<<2)  // for IMP port with clock speed can't support full line rate due to brcm tag

#endif

