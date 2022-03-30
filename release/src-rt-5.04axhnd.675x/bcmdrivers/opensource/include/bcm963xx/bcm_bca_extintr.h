/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
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

#ifndef __BCM_BCA_EXTINTR_H
#define __BCM_BCA_EXTINTR_H

#if defined(CONFIG_BCM_BCA_EXTINTR)
#include <linux/interrupt.h>
#include <bcm_bca_extintr_dt_bindings.h>

#define IsExtIntrActLow(irq)            ((irq&BCA_EXTINTR_TYPE_LEVEL_MASK) == BCA_EXTINTR_TYPE_LOW_LEVEL)
#define IsExtIntrActHigh(irq)           ((irq&BCA_EXTINTR_TYPE_LEVEL_MASK) == BCA_EXTINTR_TYPE_HIGH_LEVEL)
#define IsExtIntrSenseLevel(irq)        ((irq&BCA_EXTINTR_TYPE_SENSE_MASK) == BCA_EXTINTR_TYPE_SENSE_LEVEL)
#define IsExtIntrSenseEdge(irq)         ((irq&BCA_EXTINTR_TYPE_SENSE_MASK) == BCA_EXTINTR_TYPE_SENSE_EDGE)
#define IsExtIntrBothEdge(irq)          ((irq&BCA_EXTINTR_TYPE_BOTH_EDGE_MASK) == BCA_EXTINTR_TYPE_BOTH_EDGE)

extern int bcm_bca_extintr_request(void *dev, struct device_node *np, const char *consumer_name, irq_handler_t pfunc, void *param,
    const char *interrupt_name, irq_handler_t thread_fn);
extern int bcm_bca_extintr_free(void *_dev, int irq, void *param);
extern void bcm_bca_extintr_clear(unsigned int irq);
extern void bcm_bca_extintr_mask(unsigned int irq);
extern void bcm_bca_extintr_unmask(unsigned int irq);
extern void* bcm_bca_extintr_get_gpiod(unsigned int irq);
#endif

#endif
