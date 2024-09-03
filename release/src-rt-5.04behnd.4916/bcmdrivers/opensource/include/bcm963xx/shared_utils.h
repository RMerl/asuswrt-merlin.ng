/*
 * <:copyright-BRCM:2021:DUAL/GPL:standard
 * 
 *    Copyright (c) 2021 Broadcom 
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

#ifndef __SHARED_UTILS_H__
#define __SHARED_UTILS_H__

unsigned int UtilGetChipIdRaw(void);
unsigned int UtilGetChipRev(void);
unsigned int UtilGetChipId(void);
unsigned int util_get_chip_id(void); /*wrapper for RDPA compilation*/
unsigned int UtilGetChipIsLP(void);
char *UtilGetChipName(char *buf, int len);
int UtilGetChipIsPinCompatible(void); 
int UtilGetLanLedGpio(int n, unsigned short *gpio_num);
int UtilGetLan1LedGpio(unsigned short *gpio_num);
int UtilGetLan2LedGpio(unsigned short *gpio_num);
int UtilGetLan3LedGpio(unsigned short *gpio_num);
int UtilGetLan4LedGpio(unsigned short *gpio_num);
#include "bcm_pinmux.h"
#define set_pinmux bcm_set_pinmux

#endif
