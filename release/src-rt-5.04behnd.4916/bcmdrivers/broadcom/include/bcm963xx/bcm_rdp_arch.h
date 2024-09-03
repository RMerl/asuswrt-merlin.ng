/* 
 <:copyright-BRCM:2020:DUAL/GPL:standard
 
    Copyright (c) 2020 Broadcom 
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

#ifndef BCM_RDP_ARCH_H
#define BCM_RDP_ARCH_H

#ifdef RDP_SIM
#define RDP_ARCH_SIM
#elif defined CONFIG_BCM_QEMU_SIM
#define RDP_ARCH_QEMU_SIM
#elif defined(_CFE_) || defined(__UBOOT__)
#define RDP_ARCH_BOOT
#else
#define RDP_ARCH_BOARD
#endif /* RDP_SIM */
#endif /* BCM_RDP_ARCH_H */
