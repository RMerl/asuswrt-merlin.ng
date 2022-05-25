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
