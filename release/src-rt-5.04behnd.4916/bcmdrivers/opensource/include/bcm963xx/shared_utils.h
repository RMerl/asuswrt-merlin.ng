/*
 * <:copyright-BRCM:2021:DUAL/GPL:standard
 * 
 *    Copyright (c) 2021 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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
