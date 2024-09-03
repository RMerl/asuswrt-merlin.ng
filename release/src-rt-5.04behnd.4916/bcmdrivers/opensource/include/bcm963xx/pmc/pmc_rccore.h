/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
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