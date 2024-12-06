/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom
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
/****************************************************************************
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/

#ifndef PMC_RCCORE_H
#define PMC_RCCORE_H

enum{
  /* One zone per each runner pair */
    PMC_RCCORE_0_1,  	/* 0, 1 */
    PMC_RCCORE_2_3,  	/* 2, 3 */ 
    PMC_RCCORE_4_5,  	/* 4, 5 */
    PMC_RCCORE_6_7,  	/* 6, 7 */
#if defined(CONFIG_BCM968880)	
    PMC_RCCORE_8_9,    	/* 8, 9 */
    PMC_RCCORE_10_11,  	/* 10, 11 */
    PMC_RCCORE_12_13,  	/* 12, 13 */
#endif	
    PMC_RCCORE_MAX
};

int pmc_rccore_power_up(unsigned int rccore_num);
int pmc_rccore_power_down(unsigned int rccore_num);

#endif //#ifndef PMC_RCCORE_H