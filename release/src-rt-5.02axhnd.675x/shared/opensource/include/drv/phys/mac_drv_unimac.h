/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
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

