/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Platform data definitions for Atmel USBA gadget driver
 * pieces copied from linux:include/linux/platform_data/atmel.h
 */
#ifndef __LINUX_USB_AT91_UDC_H__
#define __LINUX_USB_AT91_UDC_H__

struct at91_udc_data {
	int	vbus_pin;		/* high == host powering us */
	u8	vbus_active_low;	/* vbus polarity */
	u8	vbus_polled;		/* Use polling, not interrupt */
	int	pullup_pin;		/* active == D+ pulled up */
	u8	pullup_active_low;	/* true == pullup_pin is active low */
	unsigned long	baseaddr;
};

int at91_udc_probe(struct at91_udc_data *pdata);
#endif /* __LINUX_USB_AT91_UDC_H__ */
