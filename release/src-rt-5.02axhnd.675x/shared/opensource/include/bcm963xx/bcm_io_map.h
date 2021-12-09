/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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

/* This file define the register io mapping macro for ARMv7 and ARMv8 based chip for
   non-device tree based driver. 
 */

#ifndef __BCM_IO_MAP_H
#define __BCM_IO_MAP_H

/* BCM_IO_ADDR macro for ARMv7 */
#ifdef _CFE_
#define BCM_IO_ADDR(x)           (x)
#elif defined(__KERNEL__)
#if defined(CONFIG_ARM) && !defined(CONFIG_BCM947189) && !defined(CONFIG_BCM96846) && !defined(CONFIG_BCM947622) && !defined(CONFIG_BCM963178) && !defined(CONFIG_BCM96878)
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
