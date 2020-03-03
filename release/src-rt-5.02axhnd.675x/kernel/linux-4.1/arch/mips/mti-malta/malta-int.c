/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000, 2001, 2004 MIPS Technologies, Inc.
 * Copyright (C) 2001 Ralf Baechle
 * Copyright (C) 2013 Imagination Technologies Ltd.
 *
 * Routines for generic manipulation of the interrupts found on the MIPS
 * Malta board. The interrupt controller is located in the South Bridge
 * a PIIX4 device with two internal 82C95 interrupt controllers.
 */
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irqchip/mips-gic.h>
#include <linux/kernel_stat.h>
#include <linux/kernel.h>
#include <linux/random.h>

#include <asm/traps.h>
#include <asm/i8259.h>
#include <asm/irq_cpu.h>
#include <asm/irq_regs.h>
#include <asm/mips-cm.h>
#include <asm/mips-boards/malta.h>
#include <asm/mips-boards/maltaint.h>
#include <asm/gt64120.h>
#include <asm/mips-boards/generic.h>
#include <asm/mips-boards/msc01_pci.h>
#include <asm/msc01_ic.h>
#include <asm/setup.h>
#include <asm/rtlx.h>

static void __iomem *_msc01_biu_base;

static DEFINE_RAW_SPINLOCK(mips_irq_lock);

static inline int mips_pcibios_iack(void)
{
	int irq;

	/*
	 * Determine highest priority pending interrupt by performing
	 * a PCI Interrupt Acknowledge cycle.
	 */
	switch (mips_revision_sconid) {
	case MIPS_REVISION_SCON_SOCIT:
	case MIPS_REVISION_SCON_ROCIT:
	case MIPS_REVISION_SCON_SOCITSC:
	case MIPS_REVISION_SCON_SOCITSCP:
		MSC_READ(MSC01_PCI_IACK, irq);
		irq &= 0xff;
		break;
	case MIPS_REVISION_SCON_GT64120:
		irq = GT_READ(GT_PCI0_IACK_OFS);
		irq &= 0xff;
		break;
	case MIPS_REVISION_SCON_BONITO:
		/* The following will generate a PCI IACK cycle on the
		 * Bonito controller. It's a little bit kludgy, but it
		 * was the easiest way to implement it in hardware at
		 * the given time.
		 */
		BONITO_PCIMAP_CFG = 0x20000;

		/* Flush Bonito register block */
		(void) BONITO_PCIMAP_CFG;
		iob();	  /* sync */

		irq = __raw_readl((u32 *)_pcictrl_bonito_pcicfg);
		iob();	  /* sync */
		irq &= 0xff;
		BONITO_PCIMAP_CFG = 0;
		break;
	default:
		pr_emerg("Unknown system controller.\n");
		return -1;
	}
	return irq;
}

static inline int get_int(void)
{
	unsigned long flags;
	int irq;
	raw_spin_lock_irqsave(&mips_irq_lock, flags);

	irq = mips_pcibios_iack();

	/*
	 * The only way we can decide if an interrupt is spurious
	 * is by checking the 8259 registers.  This needs a spinlock
	 * on an SMP system,  so leave it up to the generic code...
	 */

	raw_spin_unlock_irqrestore(&mips_irq_lock, flags);

	return irq;
}

static void malta_hw0_irqdispatch(void)
{
	int irq;

	irq = get_int();
	if (irq < 0) {
		/* interrupt has already been cleared */
		return;
	}

	do_IRQ(MALTA_INT_BASE + irq);

#ifdef CONFIG_MIPS_VPE_APSP_API_MT
	if (aprp_hook)
		aprp_hook();
#endif
}

static irqreturn_t i8259_handler(int irq, void *dev_id)
{
	malta_hw0_irqdispatch();
	return IRQ_HANDLED;
}

static void corehi_irqdispatch(void)
{
	unsigned int intedge, intsteer, pcicmd, pcibadaddr;
	unsigned int pcimstat, intisr, inten, intpol;
	unsigned int intrcause, datalo, datahi;
	struct pt_regs *regs = get_irq_regs();

	pr_emerg("CoreHI interrupt, shouldn't happen, we die here!\n");
	pr_emerg("epc	 : %08lx\nStatus: %08lx\n"
		 "Cause : %08lx\nbadVaddr : %08lx\n",
		 regs->cp0_epc, regs->cp0_status,
		 regs->cp0_cause, regs->cp0_badvaddr);

	/* Read all the registers and then print them as there is a
	   problem with interspersed printk's upsetting the Bonito controller.
	   Do it for the others too.
	*/

	switch (mips_revision_sconid) {
	case MIPS_REVISION_SCON_SOCIT:
	case MIPS_REVISION_SCON_ROCIT:
	case MIPS_REVISION_SCON_SOCITSC:
	case MIPS_REVISION_SCON_SOCITSCP:
		ll_msc_irq();
		break;
	case MIPS_REVISION_SCON_GT64120:
		intrcause = GT_READ(GT_INTRCAUSE_OFS);
		datalo = GT_READ(GT_CPUERR_ADDRLO_OFS);
		datahi = GT_READ(GT_CPUERR_ADDRHI_OFS);
		pr_emerg("GT_INTRCAUSE = %08x\n", intrcause);
		pr_emerg("GT_CPUERR_ADDR = %02x%08x\n",
				datahi, datalo);
		break;
	case MIPS_REVISION_SCON_BONITO:
		pcibadaddr = BONITO_PCIBADADDR;
		pcimstat = BONITO_PCIMSTAT;
		intisr = BONITO_INTISR;
		inten = BONITO_INTEN;
		intpol = BONITO_INTPOL;
		intedge = BONITO_INTEDGE;
		intsteer = BONITO_INTSTEER;
		pcicmd = BONITO_PCICMD;
		pr_emerg("BONITO_INTISR = %08x\n", intisr);
		pr_emerg("BONITO_INTEN = %08x\n", inten);
		pr_emerg("BONITO_INTPOL = %08x\n", intpol);
		pr_emerg("BONITO_INTEDGE = %08x\n", intedge);
		pr_emerg("BONITO_INTSTEER = %08x\n", intsteer);
		pr_emerg("BONITO_PCICMD = %08x\n", pcicmd);
		pr_emerg("BONITO_PCIBADADDR = %08x\n", pcibadaddr);
		pr_emerg("BONITO_PCIMSTAT = %08x\n", pcimstat);
		break;
	}

	die("CoreHi interrupt", regs);
}

static irqreturn_t corehi_handler(int irq, void *dev_id)
{
	corehi_irqdispatch();
	return IRQ_HANDLED;
}

#ifdef CONFIG_MIPS_MT_SMP

#define MIPS_CPU_IPI_RESCHED_IRQ 0	/* SW int 0 for resched */
#define C_RESCHED C_SW0
#define MIPS_CPU_IPI_CALL_IRQ 1		/* SW int 1 for resched */
#define C_CALL C_SW1
static int cpu_ipi_resched_irq, cpu_ipi_call_irq;

static void ipi_resched_dispatch(void)
{
	do_IRQ(MIPS_CPU_IRQ_BASE + MIPS_CPU_IPI_RESCHED_IRQ);
}

static void ipi_call_dispatch(void)
{
	do_IRQ(MIPS_CPU_IRQ_BASE + MIPS_CPU_IPI_CALL_IRQ);
}

static irqreturn_t ipi_resched_interrupt(int irq, void *dev_id)
{
#ifdef CONFIG_MIPS_VPE_APSP_API_CMP
	if (aprp_hook)
		aprp_hook();
#endif

	scheduler_ipi();

	return IRQ_HANDLED;
}

static irqreturn_t ipi_call_interrupt(int irq, void *dev_id)
{
	smp_call_function_interrupt();

	return IRQ_HANDLED;
}

static struct irqaction irq_resched = {
	.handler	= ipi_resched_interrupt,
	.flags		= IRQF_PERCPU,
	.name		= "IPI_resched"
};

static struct irqaction irq_call = {
	.handler	= ipi_call_interrupt,
	.flags		= IRQF_PERCPU,
	.name		= "IPI_call"
};
#endif /* CONFIG_MIPS_MT_SMP */

static struct irqaction i8259irq = {
	.handler = i8259_handler,
	.name = "XT-PIC cascade",
	.flags = IRQF_NO_THREAD,
};

static struct irqaction corehi_irqaction = {
	.handler = corehi_handler,
	.name = "CoreHi",
	.flags = IRQF_NO_THREAD,
};

static msc_irqmap_t msc_irqmap[] __initdata = {
	{MSC01C_INT_TMR,		MSC01_IRQ_EDGE, 0},
	{MSC01C_INT_PCI,		MSC01_IRQ_LEVEL, 0},
};
static int msc_nr_irqs __initdata = ARRAY_SIZE(msc_irqmap);

static msc_irqmap_t msc_eicirqmap[] __initdata = {
	{MSC01E_INT_SW0,		MSC01_IRQ_LEVEL, 0},
	{MSC01E_INT_SW1,		MSC01_IRQ_LEVEL, 0},
	{MSC01E_INT_I8259A,		MSC01_IRQ_LEVEL, 0},
	{MSC01E_INT_SMI,		MSC01_IRQ_LEVEL, 0},
	{MSC01E_INT_COREHI,		MSC01_IRQ_LEVEL, 0},
	{MSC01E_INT_CORELO,		MSC01_IRQ_LEVEL, 0},
	{MSC01E_INT_TMR,		MSC01_IRQ_EDGE, 0},
	{MSC01E_INT_PCI,		MSC01_IRQ_LEVEL, 0},
	{MSC01E_INT_PERFCTR,		MSC01_IRQ_LEVEL, 0},
	{MSC01E_INT_CPUCTR,		MSC01_IRQ_LEVEL, 0}
};

static int msc_nr_eicirqs __initdata = ARRAY_SIZE(msc_eicirqmap);

void __init arch_init_ipiirq(int irq, struct irqaction *action)
{
	setup_irq(irq, action);
	irq_set_handler(irq, handle_percpu_irq);
}

void __init arch_init_irq(void)
{
	int corehi_irq, i8259_irq;

	init_i8259_irqs();

	if (!cpu_has_veic)
		mips_cpu_irq_init();

	if (mips_cm_present()) {
		write_gcr_gic_base(GIC_BASE_ADDR | CM_GCR_GIC_BASE_GICEN_MSK);
		gic_present = 1;
	} else {
		if (mips_revision_sconid == MIPS_REVISION_SCON_ROCIT) {
			_msc01_biu_base = ioremap_nocache(MSC01_BIU_REG_BASE,
						MSC01_BIU_ADDRSPACE_SZ);
			gic_present =
			  (__raw_readl(_msc01_biu_base + MSC01_SC_CFG_OFS) &
			   MSC01_SC_CFG_GICPRES_MSK) >>
			  MSC01_SC_CFG_GICPRES_SHF;
		}
	}
	if (gic_present)
		pr_debug("GIC present\n");

	switch (mips_revision_sconid) {
	case MIPS_REVISION_SCON_SOCIT:
	case MIPS_REVISION_SCON_ROCIT:
		if (cpu_has_veic)
			init_msc_irqs(MIPS_MSC01_IC_REG_BASE,
					MSC01E_INT_BASE, msc_eicirqmap,
					msc_nr_eicirqs);
		else
			init_msc_irqs(MIPS_MSC01_IC_REG_BASE,
					MSC01C_INT_BASE, msc_irqmap,
					msc_nr_irqs);
		break;

	case MIPS_REVISION_SCON_SOCITSC:
	case MIPS_REVISION_SCON_SOCITSCP:
		if (cpu_has_veic)
			init_msc_irqs(MIPS_SOCITSC_IC_REG_BASE,
					MSC01E_INT_BASE, msc_eicirqmap,
					msc_nr_eicirqs);
		else
			init_msc_irqs(MIPS_SOCITSC_IC_REG_BASE,
					MSC01C_INT_BASE, msc_irqmap,
					msc_nr_irqs);
	}

	if (gic_present) {
		int i;

		gic_init(GIC_BASE_ADDR, GIC_ADDRSPACE_SZ, MIPSCPU_INT_GIC,
			 MIPS_GIC_IRQ_BASE);
		if (!mips_cm_present()) {
			/* Enable the GIC */
			i = __raw_readl(_msc01_biu_base + MSC01_SC_CFG_OFS);
			__raw_writel(i | (0x1 << MSC01_SC_CFG_GICENA_SHF),
				 _msc01_biu_base + MSC01_SC_CFG_OFS);
			pr_debug("GIC Enabled\n");
		}
		i8259_irq = MIPS_GIC_IRQ_BASE + GIC_INT_I8259A;
		corehi_irq = MIPS_CPU_IRQ_BASE + MIPSCPU_INT_COREHI;
	} else {
#if defined(CONFIG_MIPS_MT_SMP)
		/* set up ipi interrupts */
		if (cpu_has_veic) {
			set_vi_handler (MSC01E_INT_SW0, ipi_resched_dispatch);
			set_vi_handler (MSC01E_INT_SW1, ipi_call_dispatch);
			cpu_ipi_resched_irq = MSC01E_INT_SW0;
			cpu_ipi_call_irq = MSC01E_INT_SW1;
		} else {
			cpu_ipi_resched_irq = MIPS_CPU_IRQ_BASE +
				MIPS_CPU_IPI_RESCHED_IRQ;
			cpu_ipi_call_irq = MIPS_CPU_IRQ_BASE +
				MIPS_CPU_IPI_CALL_IRQ;
		}
		arch_init_ipiirq(cpu_ipi_resched_irq, &irq_resched);
		arch_init_ipiirq(cpu_ipi_call_irq, &irq_call);
#endif
		if (cpu_has_veic) {
			set_vi_handler(MSC01E_INT_I8259A,
				       malta_hw0_irqdispatch);
			set_vi_handler(MSC01E_INT_COREHI,
				       corehi_irqdispatch);
			i8259_irq = MSC01E_INT_BASE + MSC01E_INT_I8259A;
			corehi_irq = MSC01E_INT_BASE + MSC01E_INT_COREHI;
		} else {
			i8259_irq = MIPS_CPU_IRQ_BASE + MIPSCPU_INT_I8259A;
			corehi_irq = MIPS_CPU_IRQ_BASE + MIPSCPU_INT_COREHI;
		}
	}

	setup_irq(i8259_irq, &i8259irq);
	setup_irq(corehi_irq, &corehi_irqaction);
}

void malta_be_init(void)
{
	/* Could change CM error mask register. */
}


static char *tr[8] = {
	"mem",	"gcr",	"gic",	"mmio",
	"0x04", "0x05", "0x06", "0x07"
};

static char *mcmd[32] = {
	[0x00] = "0x00",
	[0x01] = "Legacy Write",
	[0x02] = "Legacy Read",
	[0x03] = "0x03",
	[0x04] = "0x04",
	[0x05] = "0x05",
	[0x06] = "0x06",
	[0x07] = "0x07",
	[0x08] = "Coherent Read Own",
	[0x09] = "Coherent Read Share",
	[0x0a] = "Coherent Read Discard",
	[0x0b] = "Coherent Ready Share Always",
	[0x0c] = "Coherent Upgrade",
	[0x0d] = "Coherent Writeback",
	[0x0e] = "0x0e",
	[0x0f] = "0x0f",
	[0x10] = "Coherent Copyback",
	[0x11] = "Coherent Copyback Invalidate",
	[0x12] = "Coherent Invalidate",
	[0x13] = "Coherent Write Invalidate",
	[0x14] = "Coherent Completion Sync",
	[0x15] = "0x15",
	[0x16] = "0x16",
	[0x17] = "0x17",
	[0x18] = "0x18",
	[0x19] = "0x19",
	[0x1a] = "0x1a",
	[0x1b] = "0x1b",
	[0x1c] = "0x1c",
	[0x1d] = "0x1d",
	[0x1e] = "0x1e",
	[0x1f] = "0x1f"
};

static char *core[8] = {
	"Invalid/OK",	"Invalid/Data",
	"Shared/OK",	"Shared/Data",
	"Modified/OK",	"Modified/Data",
	"Exclusive/OK", "Exclusive/Data"
};

static char *causes[32] = {
	"None", "GC_WR_ERR", "GC_RD_ERR", "COH_WR_ERR",
	"COH_RD_ERR", "MMIO_WR_ERR", "MMIO_RD_ERR", "0x07",
	"0x08", "0x09", "0x0a", "0x0b",
	"0x0c", "0x0d", "0x0e", "0x0f",
	"0x10", "0x11", "0x12", "0x13",
	"0x14", "0x15", "0x16", "INTVN_WR_ERR",
	"INTVN_RD_ERR", "0x19", "0x1a", "0x1b",
	"0x1c", "0x1d", "0x1e", "0x1f"
};

int malta_be_handler(struct pt_regs *regs, int is_fixup)
{
	/* This duplicates the handling in do_be which seems wrong */
	int retval = is_fixup ? MIPS_BE_FIXUP : MIPS_BE_FATAL;

	if (mips_cm_present()) {
		unsigned long cm_error = read_gcr_error_cause();
		unsigned long cm_addr = read_gcr_error_addr();
		unsigned long cm_other = read_gcr_error_mult();
		unsigned long cause, ocause;
		char buf[256];

		cause = cm_error & CM_GCR_ERROR_CAUSE_ERRTYPE_MSK;
		if (cause != 0) {
			cause >>= CM_GCR_ERROR_CAUSE_ERRTYPE_SHF;
			if (cause < 16) {
				unsigned long cca_bits = (cm_error >> 15) & 7;
				unsigned long tr_bits = (cm_error >> 12) & 7;
				unsigned long cmd_bits = (cm_error >> 7) & 0x1f;
				unsigned long stag_bits = (cm_error >> 3) & 15;
				unsigned long sport_bits = (cm_error >> 0) & 7;

				snprintf(buf, sizeof(buf),
					 "CCA=%lu TR=%s MCmd=%s STag=%lu "
					 "SPort=%lu\n",
					 cca_bits, tr[tr_bits], mcmd[cmd_bits],
					 stag_bits, sport_bits);
			} else {
				/* glob state & sresp together */
				unsigned long c3_bits = (cm_error >> 18) & 7;
				unsigned long c2_bits = (cm_error >> 15) & 7;
				unsigned long c1_bits = (cm_error >> 12) & 7;
				unsigned long c0_bits = (cm_error >> 9) & 7;
				unsigned long sc_bit = (cm_error >> 8) & 1;
				unsigned long cmd_bits = (cm_error >> 3) & 0x1f;
				unsigned long sport_bits = (cm_error >> 0) & 7;
				snprintf(buf, sizeof(buf),
					 "C3=%s C2=%s C1=%s C0=%s SC=%s "
					 "MCmd=%s SPort=%lu\n",
					 core[c3_bits], core[c2_bits],
					 core[c1_bits], core[c0_bits],
					 sc_bit ? "True" : "False",
					 mcmd[cmd_bits], sport_bits);
			}

			ocause = (cm_other & CM_GCR_ERROR_MULT_ERR2ND_MSK) >>
				 CM_GCR_ERROR_MULT_ERR2ND_SHF;

			pr_err("CM_ERROR=%08lx %s <%s>\n", cm_error,
			       causes[cause], buf);
			pr_err("CM_ADDR =%08lx\n", cm_addr);
			pr_err("CM_OTHER=%08lx %s\n", cm_other, causes[ocause]);

			/* reprime cause register */
			write_gcr_error_cause(0);
		}
	}

	return retval;
}
