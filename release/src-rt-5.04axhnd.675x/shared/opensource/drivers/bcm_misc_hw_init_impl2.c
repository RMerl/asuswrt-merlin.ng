/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
 *    All Rights Reserved
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
 */

// FORMAT: notabs uncrustify:bcm_minimal_i4.cfg

#include "boardparms.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <bcm_map_part.h>
#include <linux/string.h>

#include "bcm_misc_hw_init.h"

int bcm_misc_hw_init(void)
{
    {
        unsigned short PhyBaseAddr;
        if( BpGetEphyBaseAddress(&PhyBaseAddr) == BP_SUCCESS ) {
            MISC_REG->EphyPhyAd &= ~EPHY_PHYAD_MASK;
            MISC_REG->EphyPhyAd |= (EPHY_PHYAD_MASK & PhyBaseAddr);
        }
    }

    return 0;
}

arch_initcall(bcm_misc_hw_init);
