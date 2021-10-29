/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
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


#ifndef _BCM_EXTIRQ_H_
#define _BCM_EXIIRQ_H_

#include <bcm_intr.h>

extern unsigned int extIntrInfo[NUM_EXT_INT];

int map_external_irq (int irq);
void bcm_extirq_init(void);
void init_reset_irq(void);
#if defined(CONFIG_BCM960333)
void mapBcm960333GpioToIntr( unsigned int gpio, unsigned int extIrq );
#endif
#if defined(CONFIG_BCM_6802_MoCA)
void init_moca_ext_irq_info(void);
#endif
#endif
