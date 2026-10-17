// SPDX-License-Identifier: GPL-2.0+
/*
 * Mentor USB OTG Core functionality common for both Host and Device
 * functionality.
 *
 * Copyright (c) 2008 Texas Instruments
 *
 * Author: Thomas Abraham t-abraham@ti.com, Texas Instruments
 */

#include <common.h>
#include <linux/bitops.h>

#include "musb_core.h"
struct musb_regs *musbr;

/*
 * program the mentor core to start (enable interrupts, dma, etc.)
 */
void musb_start(void)
{
#if defined(CONFIG_USB_MUSB_HCD)
	u8 devctl;
	u8 busctl;
#endif

	/* disable all interrupts */
	writew(0, &musbr->intrtxe);
	writew(0, &musbr->intrrxe);
	writeb(0, &musbr->intrusbe);
	writeb(0, &musbr->testmode);

	/* put into basic highspeed mode and start session */
	writeb(MUSB_POWER_HSENAB, &musbr->power);
#if defined(CONFIG_USB_MUSB_HCD)
	/* Program PHY to use EXT VBUS if required */
	if (musb_cfg.extvbus == 1) {
		busctl = musb_read_ulpi_buscontrol(musbr);
		musb_write_ulpi_buscontrol(musbr, busctl | ULPI_USE_EXTVBUS);
	}

	devctl = readb(&musbr->devctl);
	writeb(devctl | MUSB_DEVCTL_SESSION, &musbr->devctl);
#endif
}

#ifdef MUSB_NO_DYNAMIC_FIFO
# define config_fifo(dir, idx, addr)
#else
# define config_fifo(dir, idx, addr) \
	do { \
		writeb(idx, &musbr->dir##fifosz); \
		writew(addr, &musbr->dir##fifoadd); \
	} while (0)
#endif

/*
 * This function configures the endpoint configuration. The musb hcd or musb
 * device implementation can use this function to configure the endpoints
 * and set the FIFO sizes. Note: The summation of FIFO sizes of all endpoints
 * should not be more than the available FIFO size.
 *
 * epinfo	- Pointer to EP configuration table
 * cnt		- Number of entries in the EP conf table.
 */
void musb_configure_ep(const struct musb_epinfo *epinfo, u8 cnt)
{
	u16 csr;
	u16 fifoaddr = 64 >> 3; /* First 64 bytes of FIFO reserved for EP0 */
	u32 fifosize;
	u8  idx;

	while (cnt--) {
		/* prepare fifosize to write to register */
		fifosize = epinfo->epsize >> 3;
		idx = fifosize ? ((ffs(fifosize) - 1) & 0xF) : 0;

		writeb(epinfo->epnum, &musbr->index);
		if (epinfo->epdir) {
			/* Configure fifo size and fifo base address */
			config_fifo(tx, idx, fifoaddr);

			csr = readw(&musbr->txcsr);
			/* clear the data toggle bit */
			writew(csr | MUSB_TXCSR_CLRDATATOG, &musbr->txcsr);
			/* Flush fifo if required */
			if (csr & MUSB_TXCSR_TXPKTRDY)
				writew(csr | MUSB_TXCSR_FLUSHFIFO,
					&musbr->txcsr);
		} else {
			/* Configure fifo size and fifo base address */
			config_fifo(rx, idx, fifoaddr);

			csr = readw(&musbr->rxcsr);
			/* clear the data toggle bit */
			writew(csr | MUSB_RXCSR_CLRDATATOG, &musbr->rxcsr);
			/* Flush fifo if required */
			if (csr & MUSB_RXCSR_RXPKTRDY)
				writew(csr | MUSB_RXCSR_FLUSHFIFO,
					&musbr->rxcsr);
		}
		fifoaddr += 1 << idx;
		epinfo++;
	}
}

/*
 * This function writes data to endpoint fifo
 *
 * ep		- endpoint number
 * length	- number of bytes to write to FIFO
 * fifo_data	- Pointer to data buffer that contains the data to write
 */
__attribute__((weak))
void write_fifo(u8 ep, u32 length, void *fifo_data)
{
	u8  *data = (u8 *)fifo_data;

	/* select the endpoint index */
	writeb(ep, &musbr->index);

	/* write the data to the fifo */
	while (length--)
		writeb(*data++, &musbr->fifox[ep]);
}

/*
 * AM35x supports only 32bit read operations so
 * use seperate read_fifo() function for it.
 */
#ifndef CONFIG_USB_AM35X
/*
 * This function reads data from endpoint fifo
 *
 * ep           - endpoint number
 * length       - number of bytes to read from FIFO
 * fifo_data    - pointer to data buffer into which data is read
 */
__attribute__((weak))
void read_fifo(u8 ep, u32 length, void *fifo_data)
{
	u8  *data = (u8 *)fifo_data;

	/* select the endpoint index */
	writeb(ep, &musbr->index);

	/* read the data to the fifo */
	while (length--)
		*data++ = readb(&musbr->fifox[ep]);
}
#endif /* CONFIG_USB_AM35X */
