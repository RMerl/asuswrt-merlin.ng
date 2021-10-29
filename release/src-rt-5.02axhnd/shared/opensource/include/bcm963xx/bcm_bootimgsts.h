/*
<:copyright-BRCM:2017:DUAL/GPL:standard 

   Copyright (c) 2017 Broadcom 
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

/***************************************************************************/
/*                                                                         */
/*   MODULE:  bcm_bootimgsts.h                                             */
/*   PURPOSE: Define bits which survive a board reset but not power cycle. */
/*                                                                         */
/***************************************************************************/
#ifndef _BCM_BOOTIMGSTS_H
#define _BCM_BOOTIMGSTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <bcm_map_part.h>

#if defined(CONFIG_BCM963268) || defined(_BCM963268_) || defined(CONFIG_BCM963381) || defined(_BCM963381_) || defined(CONFIG_BCM963138) || defined(_BCM963138_) || defined(CONFIG_BCM963148) || defined(_BCM963148_) || defined(CONFIG_BCM96838) || defined(_BCM96838_) || defined(CONFIG_BCM96848) || defined(_BCM96848_)
#define SET_NON_VOLATILE_REG HS_SPI->hs_spiGlobalCtrl
#define SET_NON_VOLATILE_MASK 0x400000
#define BOOT_INACTIVE_IMAGE_ONCE_REG HS_SPI_PROFILES[7].polling_and_mask
#elif !defined(CONFIG_BCM947189) && !defined(_BCM947189_)
#define SET_NON_VOLATILE_REG HS_SPI->hs_spiGlobalCtrl
#define SET_NON_VOLATILE_MASK 0x400000
#define MISC_SW_DEBUG MISC->miscSWdebugNW
#define BOOT_INACTIVE_IMAGE_ONCE_REG MISC_SW_DEBUG[0]
#endif
#define BOOT_INACTIVE_IMAGE_ONCE_MASK 0x80000000     
#ifdef __cplusplus
}
#endif

#endif /* _BCM_BOOTIMGSTS_H */

