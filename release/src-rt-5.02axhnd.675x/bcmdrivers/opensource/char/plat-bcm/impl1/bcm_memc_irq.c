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

#include <linux/interrupt.h>
#include <linux/cpu.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <board.h>

irqreturn_t memc_isr(int irq, void *dev_id)
{
#if defined (CONFIG_OPTEE)
	printk("ALERT!! Kernel code corruption detected\n");

	/* Acknowledge interrupt */
	MEMC->SEC_INTR2_CPU_CLEAR = 0xF;
#endif
	return IRQ_HANDLED;
}

static int __init bcm_memc_install_isr(void)
{
#if defined (CONFIG_OPTEE)
	if(request_irq(INTERRUPT_ID_RANGE_CHECK, memc_isr, 0, "memc", NULL)){
		printk("ERROR: failed to configure interrupt \n");
	}
	else {
		/* Enable interrupt by clearing the mask */
		MEMC->SEC_INTR2_CPU_MASK_CLEAR = 0xF;
	}
#endif
	return 0;
}
early_initcall(bcm_memc_install_isr);
