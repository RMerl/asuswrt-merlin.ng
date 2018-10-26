#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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
