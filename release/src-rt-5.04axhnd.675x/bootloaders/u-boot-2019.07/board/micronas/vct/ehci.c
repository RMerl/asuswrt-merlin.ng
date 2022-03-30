// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * Original Author Guenter Gebhardt
 * Copyright (C) 2006 Micronas GmbH
 */

#include <common.h>

#include "vct.h"

int vct_ehci_hcd_init(u32 *hccr, u32 *hcor)
{
	int retval;
	u32 val;
	u32 addr;

	dcgu_set_reset_switch(DCGU_HW_MODULE_USB_24, DCGU_SWITCH_ON);
	dcgu_set_reset_switch(DCGU_HW_MODULE_USB_60, DCGU_SWITCH_ON);
	dcgu_set_clk_switch(DCGU_HW_MODULE_USB_24, DCGU_SWITCH_ON);
	dcgu_set_clk_switch(DCGU_HW_MODULE_USB_PLL, DCGU_SWITCH_ON);
	dcgu_set_reset_switch(DCGU_HW_MODULE_USB_24, DCGU_SWITCH_OFF);

	/* Wait until (DCGU_USBPHY_STAT == 7) */
	addr = DCGU_USBPHY_STAT(DCGU_BASE);
	val = reg_read(addr);
	while (val != 7)
		val = reg_read(addr);

	dcgu_set_clk_switch(DCGU_HW_MODULE_USB_60, DCGU_SWITCH_ON);
	dcgu_set_reset_switch(DCGU_HW_MODULE_USB_60, DCGU_SWITCH_OFF);

	retval = scc_reset(SCC_USB_RW, 0);
	if (retval) {
		printf("scc_reset(SCC_USB_RW, 0) returned: 0x%x\n", retval);
		return retval;
	} else {
		retval = scc_reset(SCC_CPU1_SPDMA_RW, 0);
		if (retval) {
			printf("scc_reset(SCC_CPU1_SPDMA_RW, 0) returned: 0x%x\n",
			       retval);
			return retval;
		}
	}

	if (!retval) {
		/*
		 * For the AGU bypass, where the  SCC client provides full
		 * physical address
		 */
		scc_set_usb_address_generation_mode(1);
		scc_setup_dma(SCC_USB_RW, BCU_USB_BUFFER_1, DMA_LINEAR,
			      USE_NO_FH, DMA_READ, 0);
		scc_setup_dma(SCC_CPU1_SPDMA_RW, BCU_USB_BUFFER_1, DMA_LINEAR,
			      USE_NO_FH, DMA_WRITE, 0);
		scc_setup_dma(SCC_USB_RW, BCU_USB_BUFFER_0, DMA_LINEAR,
			      USE_NO_FH, DMA_WRITE, 0);
		scc_setup_dma(SCC_CPU1_SPDMA_RW, BCU_USB_BUFFER_0, DMA_LINEAR,
			      USE_NO_FH, DMA_READ, 0);

		/* Enable memory interface */
		scc_enable(SCC_USB_RW, 1);

		/* Start (start_cmd=0) DMAs */
		scc_dma_cmd(SCC_USB_RW, DMA_START, 0, DMA_READ);
		scc_dma_cmd(SCC_USB_RW, DMA_START, 0, DMA_WRITE);
	} else {
		printf("Cannot configure USB memory channel.\n");
		printf("USB can not access RAM. SCC configuration failed.\n");
		return retval;
	}

	/* Wait a short while */
	udelay(300000);

	reg_write(USBH_BURSTSIZE(USBH_BASE), 0x00001c1c);

	/* Set EHCI structures and DATA in RAM */
	reg_write(USBH_USBHMISC(USBH_BASE), 0x00840003);
	/* Set USBMODE to bigendian and set host mode */
	reg_write(USBH_USBMODE(USBH_BASE), 0x00000007);

	/*
	 * USBH_BURSTSIZE MUST EQUAL 0x00001c1c in order for
	 * 512 byte USB transfers on the bulk pipe to work properly.
	 * Set USBH_BURSTSIZE to 0x00001c1c
	 */
	reg_write(USBH_BURSTSIZE(USBH_BASE), 0x00001c1c);

	/* Insert access register addresses */
	*hccr = REG_GLOBAL_START_ADDR + USBH_CAPLENGTH(USBH_BASE);
	*hcor = REG_GLOBAL_START_ADDR + USBH_USBCMD(USBH_BASE);

	return 0;
}
