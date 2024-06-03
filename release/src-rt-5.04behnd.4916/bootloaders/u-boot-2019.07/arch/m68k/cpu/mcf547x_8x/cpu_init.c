// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <common.h>
#include <MCD_dma.h>
#include <asm/immap.h>
#include <asm/io.h>

#if defined(CONFIG_CMD_NET)
#include <config.h>
#include <net.h>
#include <asm/fsl_mcdmafec.h>
#endif

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f(void)
{
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	fbcs_t *fbcs = (fbcs_t *) MMAP_FBCS;
	xlbarb_t *xlbarb = (xlbarb_t *) MMAP_XARB;

	out_be32(&xlbarb->adrto, 0x2000);
	out_be32(&xlbarb->datto, 0x2500);
	out_be32(&xlbarb->busto, 0x3000);

	out_be32(&xlbarb->cfg, XARB_CFG_AT | XARB_CFG_DT);

	/* Master Priority Enable */
	out_be32(&xlbarb->prien, 0xff);
	out_be32(&xlbarb->pri, 0);

#if (defined(CONFIG_SYS_CS0_BASE) && defined(CONFIG_SYS_CS0_MASK) && defined(CONFIG_SYS_CS0_CTRL))
	out_be32(&fbcs->csar0, CONFIG_SYS_CS0_BASE);
	out_be32(&fbcs->cscr0, CONFIG_SYS_CS0_CTRL);
	out_be32(&fbcs->csmr0, CONFIG_SYS_CS0_MASK);
#endif

#if (defined(CONFIG_SYS_CS1_BASE) && defined(CONFIG_SYS_CS1_MASK) && defined(CONFIG_SYS_CS1_CTRL))
	out_be32(&fbcs->csar1, CONFIG_SYS_CS1_BASE);
	out_be32(&fbcs->cscr1, CONFIG_SYS_CS1_CTRL);
	out_be32(&fbcs->csmr1, CONFIG_SYS_CS1_MASK);
#endif

#if (defined(CONFIG_SYS_CS2_BASE) && defined(CONFIG_SYS_CS2_MASK) && defined(CONFIG_SYS_CS2_CTRL))
	out_be32(&fbcs->csar2, CONFIG_SYS_CS2_BASE);
	out_be32(&fbcs->cscr2, CONFIG_SYS_CS2_CTRL);
	out_be32(&fbcs->csmr2, CONFIG_SYS_CS2_MASK);
#endif

#if (defined(CONFIG_SYS_CS3_BASE) && defined(CONFIG_SYS_CS3_MASK) && defined(CONFIG_SYS_CS3_CTRL))
	out_be32(&fbcs->csar3, CONFIG_SYS_CS3_BASE);
	out_be32(&fbcs->cscr3, CONFIG_SYS_CS3_CTRL);
	out_be32(&fbcs->csmr3, CONFIG_SYS_CS3_MASK);
#endif

#if (defined(CONFIG_SYS_CS4_BASE) && defined(CONFIG_SYS_CS4_MASK) && defined(CONFIG_SYS_CS4_CTRL))
	out_be32(&fbcs->csar4, CONFIG_SYS_CS4_BASE);
	out_be32(&fbcs->cscr4, CONFIG_SYS_CS4_CTRL);
	out_be32(&fbcs->csmr4, CONFIG_SYS_CS4_MASK);
#endif

#if (defined(CONFIG_SYS_CS5_BASE) && defined(CONFIG_SYS_CS5_MASK) && defined(CONFIG_SYS_CS5_CTRL))
	out_be32(&fbcs->csar5, CONFIG_SYS_CS5_BASE);
	out_be32(&fbcs->cscr5, CONFIG_SYS_CS5_CTRL);
	out_be32(&fbcs->csmr5, CONFIG_SYS_CS5_MASK);
#endif

#ifdef CONFIG_SYS_I2C_FSL
	out_be16(&gpio->par_feci2cirq,
		GPIO_PAR_FECI2CIRQ_SCL | GPIO_PAR_FECI2CIRQ_SDA);
#endif

	icache_enable();
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r(void)
{
#if defined(CONFIG_CMD_NET) && defined(CONFIG_FSLDMAFEC)
	MCD_initDma((dmaRegs *) (MMAP_MCDMA), (void *)(MMAP_SRAM + 512),
		    MCD_RELOC_TASKS);
#endif
	return (0);
}

void uart_port_conf(int port)
{
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	u8 *pscsicr = (u8 *) (CONFIG_SYS_UART_BASE + 0x40);

	/* Setup Ports: */
	switch (port) {
	case 0:
		out_8(&gpio->par_psc0, GPIO_PAR_PSC0_TXD0 | GPIO_PAR_PSC0_RXD0);
		break;
	case 1:
		out_8(&gpio->par_psc1, GPIO_PAR_PSC1_TXD1 | GPIO_PAR_PSC1_RXD1);
		break;
	case 2:
		out_8(&gpio->par_psc2, GPIO_PAR_PSC2_TXD2 | GPIO_PAR_PSC2_RXD2);
		break;
	case 3:
		out_8(&gpio->par_psc3, GPIO_PAR_PSC3_TXD3 | GPIO_PAR_PSC3_RXD3);
		break;
	}

	clrbits_8(pscsicr, 0x07);
}

#if defined(CONFIG_CMD_NET)
int fecpin_setclear(struct eth_device *dev, int setclear)
{
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	struct fec_info_dma *info = (struct fec_info_dma *)dev->priv;

	if (setclear) {
		if (info->iobase == CONFIG_SYS_FEC0_IOBASE)
			setbits_be16(&gpio->par_feci2cirq, 0xf000);
		else
			setbits_be16(&gpio->par_feci2cirq, 0x0fc0);
	} else {
		if (info->iobase == CONFIG_SYS_FEC0_IOBASE)
			clrbits_be16(&gpio->par_feci2cirq, 0xf000);
		else
			clrbits_be16(&gpio->par_feci2cirq, 0x0fc0);
	}
	return 0;
}
#endif
