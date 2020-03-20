/*
<:copyright-BRCM:2007:GPL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

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
/******************************************************************************
 *
 *  Broadcom IPsec SPU Driver Common API
 *  Description: Header file for IPSec SPU Device Driver
 *  File: spuext.h
 *  Author: Pavan Kumar
 *  Date: 11/16/2007
 *
 *****************************************************************************/
#ifndef _SPUEXT_H_
#define _SPUEXT_H_

#define UBSEC_EXPLICIT_IV                131072
#define UBSEC_USING_EXPLICIT_IV(f)       ( (f) & (UBSEC_EXPLICIT_IV) )
#define UBSEC_EXTCHIPINFO_EXPLICIT_IV    0x00001000
#define OPERATION_IPSEC_3DES_EXPLICIT_IV 0x4100
#define OPERATION_IPSEC_AES_EXPLICIT_IV  0x4200
#define BCM_OEM_4_FEATURES()             pExtChipInfo->Features\
                                         &= ~(UBSEC_EXTCHIPINFO_EXPLICIT_IV);
#define BCM_OEM_4_PDEVICE_FEATURES()     pDevice->Features\
                                         &= ~(UBSEC_EXTCHIPINFO_EXPLICIT_IV);
#define BCM_OEM_4_CHECK() if(UBSEC_USING_EXPLICIT_IV(at->flags)){ \
        if (!(features & UBSEC_EXTCHIPINFO_EXPLICIT_IV))        \
                return UBSEC_STATUS_NO_DEVICE;                  \
  }

#endif /* _SPUEXT_H_ */
