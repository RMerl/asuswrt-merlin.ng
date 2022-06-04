#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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
#ifndef __ARCH_HARDWARE_H
#define __ARCH_HARDWARE_H

#include <asm/sizes.h>

/* macro to get at IO space when running virtually */
#if defined(CONFIG_MMU) && !defined(CONFIG_BCM96846)
/* in 63138, we have two memory space area defined for registers.  One starts
 * from 0x8000 0000 to roughly 0x80800000. And the other one is for PERIPH.
 * It starts from 0xfffc 0000 to 0xffff 0100. In addition, SPI boot is from
 * 0xffd0 0000 to 0xfff0 0000. Therefore, we can define the following macro
 * of address translation that combines the three different areas into one
 * contiguous virtual address area. They will be mapped to 0xfc00 0000,
 * where
 *    0xfc00 0000 - 0xfc80 0000 -> 0x8000 0000 - 0x8080 0000
 *    0xfcd0 0000 - 0xfcf0 0000 -> 0xffd0 0000 - 0xfff0 0000
 *    0xfcfc 0000 - 0xfcff 0100 -> 0xfffc 0000 - 0xffff 0100 */

#define IO_ADDRESS(x)		(((x) & 0x00ffffff) + 0xfc000000)

#else
#define IO_ADDRESS(x)		(x)
#endif

#define __io_address(n)		IOMEM(IO_ADDRESS(n))

#ifdef CONFIG_PLAT_BCM63XX_ACP
#define ACP_ADDRESS(x)		((x) | 0xe0000000)
#endif

#endif /* __ARCH_HARDWARE_H */
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
