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
