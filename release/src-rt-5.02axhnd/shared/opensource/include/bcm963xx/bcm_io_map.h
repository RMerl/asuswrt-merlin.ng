/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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

/* This file define the register io mapping macro for ARMv7 and ARMv8 based chip for
   non-device tree based driver. 
 */

#ifndef __BCM_IO_MAP_H
#define __BCM_IO_MAP_H

/* BCM_IO_ADDR macro for ARMv7 */
#ifdef _CFE_
#define BCM_IO_ADDR(x)           (x)
#elif defined(__KERNEL__)
#if defined(CONFIG_ARM) && !defined(CONFIG_BCM947189) && !defined(CONFIG_BCM96846)
#include <mach/hardware.h>
#define BCM_IO_ADDR(x)           IO_ADDRESS(x)
#else
#define BCM_IO_ADDR(x)           (x)
#endif
#else /* Userspace - #define BCM_IO_ADDR to allow compiling when bcm_map.h is not included first */
#define BCM_IO_ADDR(x)           (x)
#endif


/* BCM_IO_MAP/NOMAP macro for ARMv8/ARMv7 */
#define BCM_IO_NOMAP(index, phy_base, offset)          ((phy_base)+(offset))

#ifdef _CFE_
#define BCM_IO_MAP(index, phy_base, offset)            ((phy_base)+(offset))
#elif defined(__KERNEL__) 
#define BCM_IO_MAP(index, phy_base, offset)            ((void *)(bcm_io_block_address[index]+(offset)))
#else /* Userspace */
#define BCM_IO_MAP(index, phy_base, offset)            ((phy_base)+(offset))
#endif

#ifndef __ASSEMBLER__

typedef struct {
    unsigned int index;
    unsigned int size;
    unsigned long address;
}BCM_IO_BLOCKS;

#endif

#endif
