/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
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

/*****************************************************************************
 *  Description:
 *      This contains special header for different flavors of PMC drivers.
 *****************************************************************************/

#ifndef PMC_DRV_CFG_H
#define PMC_DRV_CFG_H

#define PMC_IMPL_1_X

#define PMC_CPU_BIG_ENDIAN    1
#define PMC_GETRCAL_SUPPORT   1 
#define PMC_CPUTEMP_SUPPORT   1
#define PMC_CLOCK_SET_SUPPORT 1
#define PMC_RAM_BOOT          1
#define PMC_IN_MAIN_LOOP	  kPMCRunStateRunning
#define PMC_SHARED_MEMORY     0x80204000
#define SSBMASTER_BASE        0x80280060

#endif // #ifndef PMC_DRV_CFG_H
