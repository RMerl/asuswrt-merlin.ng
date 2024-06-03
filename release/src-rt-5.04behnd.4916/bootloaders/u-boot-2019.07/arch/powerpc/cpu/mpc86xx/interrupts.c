// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002 (440 port)
 * Scott McNutt, Artesyn Communication Producs, smcnutt@artsyncp.com
 *
 * (C) Copyright 2003 Motorola Inc. (MPC85xx port)
 * Xianghua Xiao (X.Xiao@motorola.com)
 *
 * (C) Copyright 2004, 2007 Freescale Semiconductor. (MPC86xx Port)
 * Jeff Brown
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
 */

#include <common.h>
#include <mpc86xx.h>
#include <command.h>
#include <asm/processor.h>
#ifdef CONFIG_POST
#include <post.h>
#endif

void interrupt_init_cpu(unsigned *decrementer_count)
{
	volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	volatile ccsr_pic_t *pic = &immr->im_pic;

#ifdef CONFIG_POST
	/*
	 * The POST word is stored in the PIC's TFRR register which gets
	 * cleared when the PIC is reset.  Save it off so we can restore it
	 * later.
	 */
	ulong post_word = post_word_load();
#endif

	pic->gcr = MPC86xx_PICGCR_RST;
	while (pic->gcr & MPC86xx_PICGCR_RST)
		;
	pic->gcr = MPC86xx_PICGCR_MODE;

	*decrementer_count = get_tbclk() / CONFIG_SYS_HZ;
	debug("interrupt init: tbclk() = %ld MHz, decrementer_count = %d\n",
	      (get_tbclk() / 1000000),
	      *decrementer_count);

#ifdef CONFIG_INTERRUPTS

	pic->iivpr1 = 0x810001;	/* 50220 enable mcm interrupts */
	debug("iivpr1@%p = %x\n", &pic->iivpr1, pic->iivpr1);

	pic->iivpr2 = 0x810002;	/* 50240 enable ddr interrupts */
	debug("iivpr2@%p = %x\n", &pic->iivpr2, pic->iivpr2);

	pic->iivpr3 = 0x810003;	/* 50260 enable lbc interrupts */
	debug("iivpr3@%p = %x\n", &pic->iivpr3, pic->iivpr3);

#if defined(CONFIG_PCI1) || defined(CONFIG_PCIE1)
	pic->iivpr8 = 0x810008;	/* enable pcie1 interrupts */
	debug("iivpr8@%p = %x\n", &pic->iivpr8, pic->iivpr8);
#endif
#if defined(CONFIG_PCI2) || defined(CONFIG_PCIE2)
	pic->iivpr9 = 0x810009;	/* enable pcie2 interrupts */
	debug("iivpr9@%p = %x\n", &pic->iivpr9, pic->iivpr9);
#endif

	pic->ctpr = 0;	/* 40080 clear current task priority register */
#endif

#ifdef CONFIG_POST
	post_word_store(post_word);
#endif
}

/*
 * timer_interrupt - gets called when the decrementer overflows,
 * with interrupts disabled.
 * Trivial implementation - no need to be really accurate.
 */
void timer_interrupt_cpu(struct pt_regs *regs)
{
	/* nothing to do here */
}

/*
 * Install and free a interrupt handler. Not implemented yet.
 */
void irq_install_handler(int vec, interrupt_handler_t *handler, void *arg)
{
}

void irq_free_handler(int vec)
{
}

/*
 * irqinfo - print information about PCI devices,not implemented.
 */
int do_irqinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return 0;
}

/*
 * Handle external interrupts
 */
void external_interrupt(struct pt_regs *regs)
{
	puts("external_interrupt (oops!)\n");
}
