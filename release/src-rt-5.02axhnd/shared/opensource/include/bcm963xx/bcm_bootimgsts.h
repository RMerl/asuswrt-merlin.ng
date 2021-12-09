/*
<:copyright-BRCM:2017:DUAL/GPL:standard 

   Copyright (c) 2017 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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

