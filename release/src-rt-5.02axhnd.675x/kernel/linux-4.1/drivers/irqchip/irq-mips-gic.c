/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2008 Ralf Baechle (ralf@linux-mips.org)
 * Copyright (C) 2012 MIPS Technologies, Inc.  All rights reserved.
 */
#include <linux/bitmap.h>
#include <linux/clocksource.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqchip/mips-gic.h>
#include <linux/of_address.h>
#include <linux/sched.h>
#include <linux/smp.h>

#include <asm/mips-cm.h>
#include <asm/setup.h>
#include <asm/traps.h>

#include <dt-bindings/interrupt-controller/mips-gic.h>

#include "irqchip.h"

unsigned int gic_present;

struct gic_pcpu_mask {
	DECLARE_BITMAP(pcpu_mask, GIC_MAX_INTRS);
};

static void __iomem *gic_base;
static struct gic_pcpu_mask pcpu_masks[NR_CPUS];
static DEFINE_SPINLOCK(gic_lock);
static struct irq_domain *gic_irq_domain;
static int gic_shared_intrs;
static int gic_vpes;
static unsigned int gic_cpu_pin;
static unsigned int timer_cpu_pin;
static struct irq_chip gic_level_irq_controller, gic_edge_irq_controller;

static void __gic_irq_dispatch(void);

static inline unsigned int gic_read(unsigned int reg)
{
	return __raw_readl(gic_base + reg);
}

static inline void gic_write(unsigned int reg, unsigned int val)
{
	__raw_writel(val, gic_base + reg);
}

static inline void gic_update_bits(unsigned int reg, unsigned int mask,
				   unsigned int val)
{
	unsigned int regval;

	regval = gic_read(reg);
	regval &= ~mask;
	regval |= val;
	gic_write(reg, regval);
}

static inline void gic_reset_mask(unsigned int intr)
{
	gic_write(GIC_REG(SHARED, GIC_SH_RMASK) + GIC_INTR_OFS(intr),
		  1 << GIC_INTR_BIT(intr));
}

static inline void gic_set_mask(unsigned int intr)
{
	gic_write(GIC_REG(SHARED, GIC_SH_SMASK) + GIC_INTR_OFS(intr),
		  1 << GIC_INTR_BIT(intr));
}

static inline void gic_set_polarity(unsigned int intr, unsigned int pol)
{
	gic_update_bits(GIC_REG(SHARED, GIC_SH_SET_POLARITY) +
			GIC_INTR_OFS(intr), 1 << GIC_INTR_BIT(intr),
			pol << GIC_INTR_BIT(intr));
}

static inline void gic_set_trigger(unsigned int intr, unsigned int trig)
{
	gic_update_bits(GIC_REG(SHARED, GIC_SH_SET_TRIGGER) +
			GIC_INTR_OFS(intr), 1 << GIC_INTR_BIT(intr),
			trig << GIC_INTR_BIT(intr));
}

static inline void gic_set_dual_edge(unsigned int intr, unsigned int dual)
{
	gic_update_bits(GIC_REG(SHARED, GIC_SH_SET_DUAL) + GIC_INTR_OFS(intr),
			1 << GIC_INTR_BIT(intr),
			dual << GIC_INTR_BIT(intr));
}

static inline void gic_map_to_pin(unsigned int intr, unsigned int pin)
{
	gic_write(GIC_REG(SHARED, GIC_SH_INTR_MAP_TO_PIN_BASE) +
		  GIC_SH_MAP_TO_PIN(intr), GIC_MAP_TO_PIN_MSK | pin);
}

static inline void gic_map_to_vpe(unsigned int intr, unsigned int vpe)
{
	gic_write(GIC_REG(SHARED, GIC_SH_INTR_MAP_TO_VPE_BASE) +
		  GIC_SH_MAP_TO_VPE_REG_OFF(intr, vpe),
		  GIC_SH_MAP_TO_VPE_REG_BIT(vpe));
}

#ifdef CONFIG_CLKSRC_MIPS_GIC
cycle_t gic_read_count(void)
{
	unsigned int hi, hi2, lo;

	do {
		hi = gic_read(GIC_REG(SHARED, GIC_SH_COUNTER_63_32));
		lo = gic_read(GIC_REG(SHARED, GIC_SH_COUNTER_31_00));
		hi2 = gic_read(GIC_REG(SHARED, GIC_SH_COUNTER_63_32));
	} while (hi2 != hi);

	return (((cycle_t) hi) << 32) + lo;
}

unsigned int gic_get_count_width(void)
{
	unsigned int bits, config;

	config = gic_read(GIC_REG(SHARED, GIC_SH_CONFIG));
	bits = 32 + 4 * ((config & GIC_SH_CONFIG_COUNTBITS_MSK) >>
			 GIC_SH_CONFIG_COUNTBITS_SHF);

	return bits;
}

void gic_write_compare(cycle_t cnt)
{
	gic_write(GIC_REG(VPE_LOCAL, GIC_VPE_COMPARE_HI),
				(int)(cnt >> 32));
	gic_write(GIC_REG(VPE_LOCAL, GIC_VPE_COMPARE_LO),
				(int)(cnt & 0xffffffff));
}

void gic_write_cpu_compare(cycle_t cnt, int cpu)
{
	unsigned long flags;

	local_irq_save(flags);

	gic_write(GIC_REG(VPE_LOCAL, GIC_VPE_OTHER_ADDR), cpu);
	gic_write(GIC_REG(VPE_OTHER, GIC_VPE_COMPARE_HI),
				(int)(cnt >> 32));
	gic_write(GIC_REG(VPE_OTHER, GIC_VPE_COMPARE_LO),
				(int)(cnt & 0xffffffff));

	local_irq_restore(flags);
}

cycle_t gic_read_compare(void)
{
	unsigned int hi, lo;

	hi = gic_read(GIC_REG(VPE_LOCAL, GIC_VPE_COMPARE_HI));
	lo = gic_read(GIC_REG(VPE_LOCAL, GIC_VPE_COMPARE_LO));

	return (((cycle_t) hi) << 32) + lo;
}

void gic_start_count(void)
{
	u32 gicconfig;

	/* Start the counter */
	gicconfig = gic_read(GIC_REG(SHARED, GIC_SH_CONFIG));
	gicconfig &= ~(1 << GIC_SH_CONFIG_COUNTSTOP_SHF);
	gic_write(GIC_REG(SHARED, GIC_SH_CONFIG), gicconfig);
}

void gic_stop_count(void)
{
	u32 gicconfig;

	/* Stop the counter */
	gicconfig = gic_read(GIC_REG(SHARED, GIC_SH_CONFIG));
	gicconfig |= 1 << GIC_SH_CONFIG_COUNTSTOP_SHF;
	gic_write(GIC_REG(SHARED, GIC_SH_CONFIG), gicconfig);
}

#endif

static bool gic_local_irq_is_routable(int intr)
{
	u32 vpe_ctl;

	/* All local interrupts are routable in EIC mode. */
	if (cpu_has_veic)
		return true;

	vpe_ctl = gic_read(GIC_REG(VPE_LOCAL, GIC_VPE_CTL));
	switch (intr) {
	case GIC_LOCAL_INT_TIMER:
		return vpe_ctl & GIC_VPE_CTL_TIMER_RTBL_MSK;
	case GIC_LOCAL_INT_PERFCTR:
		return vpe_ctl & GIC_VPE_CTL_PERFCNT_RTBL_MSK;
	case GIC_LOCAL_INT_FDC:
		return vpe_ctl & GIC_VPE_CTL_FDC_RTBL_MSK;
	case GIC_LOCAL_INT_SWINT0:
	case GIC_LOCAL_INT_SWINT1:
		return vpe_ctl & GIC_VPE_CTL_SWINT_RTBL_MSK;
	default:
		return true;
	}
}

static void gic_bind_eic_interrupt(int irq, int set)
{
	/* Convert irq vector # to hw int # */
	irq -= GIC_PIN_TO_VEC_OFFSET;

	/* Set irq to use shadow set */
	gic_write(GIC_REG(VPE_LOCAL, GIC_VPE_EIC_SHADOW_SET_BASE) +
		  GIC_VPE_EIC_SS(irq), set);
}

void gic_send_ipi(unsigned int intr)
{
	gic_write(GIC_REG(SHARED, GIC_SH_WEDGE), GIC_SH_WEDGE_SET(intr));
}

int gic_get_c0_compare_int(void)
{
	if (!gic_local_irq_is_routable(GIC_LOCAL_INT_TIMER))
		return MIPS_CPU_IRQ_BASE + cp0_compare_irq;
	return irq_create_mapping(gic_irq_domain,
				  GIC_LOCAL_TO_HWIRQ(GIC_LOCAL_INT_TIMER));
}

int gic_get_c0_perfcount_int(void)
{
	if (!gic_local_irq_is_routable(GIC_LOCAL_INT_PERFCTR)) {
		/* Is the performance counter shared with the timer? */
		if (cp0_perfcount_irq < 0)
			return -1;
		return MIPS_CPU_IRQ_BASE + cp0_perfcount_irq;
	}
	return irq_create_mapping(gic_irq_domain,
				  GIC_LOCAL_TO_HWIRQ(GIC_LOCAL_INT_PERFCTR));
}

int gic_get_c0_fdc_int(void)
{
	if (!gic_local_irq_is_routable(GIC_LOCAL_INT_FDC)) {
		/* Is the FDC IRQ even present? */
		if (cp0_fdc_irq < 0)
			return -1;
		return MIPS_CPU_IRQ_BASE + cp0_fdc_irq;
	}

	/*
	 * Some cores claim the FDC is routable but it doesn't actually seem to
	 * be connected.
	 */
	switch (current_cpu_type()) {
	case CPU_INTERAPTIV:
	case CPU_PROAPTIV:
		return -1;
	}

	return irq_create_mapping(gic_irq_domain,
				  GIC_LOCAL_TO_HWIRQ(GIC_LOCAL_INT_FDC));
}

static void gic_handle_shared_int(bool chained)
{
	unsigned int i, intr, virq;
	unsigned long *pcpu_mask;
	unsigned long pending_reg, intrmask_reg;
	DECLARE_BITMAP(pending, GIC_MAX_INTRS);
	DECLARE_BITMAP(intrmask, GIC_MAX_INTRS);

	/* Get per-cpu bitmaps */
	pcpu_mask = pcpu_masks[smp_processor_id()].pcpu_mask;

	pending_reg = GIC_REG(SHARED, GIC_SH_PEND);
	intrmask_reg = GIC_REG(SHARED, GIC_SH_MASK);

	for (i = 0; i < BITS_TO_LONGS(gic_shared_intrs); i++) {
		pending[i] = gic_read(pending_reg);
		intrmask[i] = gic_read(intrmask_reg);
		pending_reg += 0x4;
		intrmask_reg += 0x4;
	}

	bitmap_and(pending, pending, intrmask, gic_shared_intrs);
	bitmap_and(pending, pending, pcpu_mask, gic_shared_intrs);

	intr = find_first_bit(pending, gic_shared_intrs);
	while (intr != gic_shared_intrs) {
		virq = irq_linear_revmap(gic_irq_domain,
					 GIC_SHARED_TO_HWIRQ(intr));
		if (chained)
			generic_handle_irq(virq);
		else
			do_IRQ(virq);

		/* go to next pending bit */
		bitmap_clear(pending, intr, 1);
		intr = find_first_bit(pending, gic_shared_intrs);
	}
}

static void gic_mask_irq(struct irq_data *d)
{
	gic_reset_mask(GIC_HWIRQ_TO_SHARED(d->hwirq));
}

static void gic_unmask_irq(struct irq_data *d)
{
	gic_set_mask(GIC_HWIRQ_TO_SHARED(d->hwirq));
}

static void gic_ack_irq(struct irq_data *d)
{
	unsigned int irq = GIC_HWIRQ_TO_SHARED(d->hwirq);

	gic_write(GIC_REG(SHARED, GIC_SH_WEDGE), GIC_SH_WEDGE_CLR(irq));
}

static int gic_set_type(struct irq_data *d, unsigned int type)
{
	unsigned int irq = GIC_HWIRQ_TO_SHARED(d->hwirq);
	unsigned long flags;
	bool is_edge;

	spin_lock_irqsave(&gic_lock, flags);
	switch (type & IRQ_TYPE_SENSE_MASK) {
	case IRQ_TYPE_EDGE_FALLING:
		gic_set_polarity(irq, GIC_POL_NEG);
		gic_set_trigger(irq, GIC_TRIG_EDGE);
		gic_set_dual_edge(irq, GIC_TRIG_DUAL_DISABLE);
		is_edge = true;
		break;
	case IRQ_TYPE_EDGE_RISING:
		gic_set_polarity(irq, GIC_POL_POS);
		gic_set_trigger(irq, GIC_TRIG_EDGE);
		gic_set_dual_edge(irq, GIC_TRIG_DUAL_DISABLE);
		is_edge = true;
		break;
	case IRQ_TYPE_EDGE_BOTH:
		/* polarity is irrelevant in this case */
		gic_set_trigger(irq, GIC_TRIG_EDGE);
		gic_set_dual_edge(irq, GIC_TRIG_DUAL_ENABLE);
		is_edge = true;
		break;
	case IRQ_TYPE_LEVEL_LOW:
		gic_set_polarity(irq, GIC_POL_NEG);
		gic_set_trigger(irq, GIC_TRIG_LEVEL);
		gic_set_dual_edge(irq, GIC_TRIG_DUAL_DISABLE);
		is_edge = false;
		break;
	case IRQ_TYPE_LEVEL_HIGH:
	default:
		gic_set_polarity(irq, GIC_POL_POS);
		gic_set_trigger(irq, GIC_TRIG_LEVEL);
		gic_set_dual_edge(irq, GIC_TRIG_DUAL_DISABLE);
		is_edge = false;
		break;
	}

	if (is_edge) {
		__irq_set_chip_handler_name_locked(d->irq,
						   &gic_edge_irq_controller,
						   handle_edge_irq, NULL);
	} else {
		__irq_set_chip_handler_name_locked(d->irq,
						   &gic_level_irq_controller,
						   handle_level_irq, NULL);
	}
	spin_unlock_irqrestore(&gic_lock, flags);

	return 0;
}

#ifdef CONFIG_SMP
static int gic_set_affinity(struct irq_data *d, const struct cpumask *cpumask,
			    bool force)
{
	unsigned int irq = GIC_HWIRQ_TO_SHARED(d->hwirq);
	cpumask_t	tmp = CPU_MASK_NONE;
	unsigned long	flags;
	int		i;

	cpumask_and(&tmp, cpumask, cpu_online_mask);
	if (cpumask_empty(&tmp))
		return -EINVAL;

	/* Assumption : cpumask refers to a single CPU */
	spin_lock_irqsave(&gic_lock, flags);

	/* Re-route this IRQ */
	gic_map_to_vpe(irq, cpumask_first(&tmp));

	/* Update the pcpu_masks */
	for (i = 0; i < NR_CPUS; i++)
		clear_bit(irq, pcpu_masks[i].pcpu_mask);
	set_bit(irq, pcpu_masks[cpumask_first(&tmp)].pcpu_mask);

	cpumask_copy(d->affinity, cpumask);
	spin_unlock_irqrestore(&gic_lock, flags);

	return IRQ_SET_MASK_OK_NOCOPY;
}
#endif

static struct irq_chip gic_level_irq_controller = {
	.name			=	"MIPS GIC",
	.irq_mask		=	gic_mask_irq,
	.irq_unmask		=	gic_unmask_irq,
	.irq_set_type		=	gic_set_type,
#ifdef CONFIG_SMP
	.irq_set_affinity	=	gic_set_affinity,
#endif
};

static struct irq_chip gic_edge_irq_controller = {
	.name			=	"MIPS GIC",
	.irq_ack		=	gic_ack_irq,
	.irq_mask		=	gic_mask_irq,
	.irq_unmask		=	gic_unmask_irq,
	.irq_set_type		=	gic_set_type,
#ifdef CONFIG_SMP
	.irq_set_affinity	=	gic_set_affinity,
#endif
};

static void gic_handle_local_int(bool chained)
{
	unsigned long pending, masked;
	unsigned int intr, virq;

	pending = gic_read(GIC_REG(VPE_LOCAL, GIC_VPE_PEND));
	masked = gic_read(GIC_REG(VPE_LOCAL, GIC_VPE_MASK));

	bitmap_and(&pending, &pending, &masked, GIC_NUM_LOCAL_INTRS);

	intr = find_first_bit(&pending, GIC_NUM_LOCAL_INTRS);
	while (intr != GIC_NUM_LOCAL_INTRS) {
		virq = irq_linear_revmap(gic_irq_domain,
					 GIC_LOCAL_TO_HWIRQ(intr));
		if (chained)
			generic_handle_irq(virq);
		else
			do_IRQ(virq);

		/* go to next pending bit */
		bitmap_clear(&pending, intr, 1);
		intr = find_first_bit(&pending, GIC_NUM_LOCAL_INTRS);
	}
}

static void gic_mask_local_irq(struct irq_data *d)
{
	int intr = GIC_HWIRQ_TO_LOCAL(d->hwirq);

	gic_write(GIC_REG(VPE_LOCAL, GIC_VPE_RMASK), 1 << intr);
}

static void gic_unmask_local_irq(struct irq_data *d)
{
	int intr = GIC_HWIRQ_TO_LOCAL(d->hwirq);

	gic_write(GIC_REG(VPE_LOCAL, GIC_VPE_SMASK), 1 << intr);
}

static struct irq_chip gic_local_irq_controller = {
	.name			=	"MIPS GIC Local",
	.irq_mask		=	gic_mask_local_irq,
	.irq_unmask		=	gic_unmask_local_irq,
};

static void gic_mask_local_irq_all_vpes(struct irq_data *d)
{
	int intr = GIC_HWIRQ_TO_LOCAL(d->hwirq);
	int i;
	unsigned long flags;

	spin_lock_irqsave(&gic_lock, flags);
	for (i = 0; i < gic_vpes; i++) {
		gic_write(GIC_REG(VPE_LOCAL, GIC_VPE_OTHER_ADDR), i);
		gic_write(GIC_REG(VPE_OTHER, GIC_VPE_RMASK), 1 << intr);
	}
	spin_unlock_irqrestore(&gic_lock, flags);
}

static void gic_unmask_local_irq_all_vpes(struct irq_data *d)
{
	int intr = GIC_HWIRQ_TO_LOCAL(d->hwirq);
	int i;
	unsigned long flags;

	spin_lock_irqsave(&gic_lock, flags);
	for (i = 0; i < gic_vpes; i++) {
		gic_write(GIC_REG(VPE_LOCAL, GIC_VPE_OTHER_ADDR), i);
		gic_write(GIC_REG(VPE_OTHER, GIC_VPE_SMASK), 1 << intr);
	}
	spin_unlock_irqrestore(&gic_lock, flags);
}

static struct irq_chip gic_all_vpes_local_irq_controller = {
	.name			=	"MIPS GIC Local",
	.irq_mask		=	gic_mask_local_irq_all_vpes,
	.irq_unmask		=	gic_unmask_local_irq_all_vpes,
};

static void __gic_irq_dispatch(void)
{
	gic_handle_local_int(false);
	gic_handle_shared_int(false);
}

static void gic_irq_dispatch(unsigned int irq, struct irq_desc *desc)
{
	gic_handle_local_int(true);
	gic_handle_shared_int(true);
}

#ifdef CONFIG_MIPS_GIC_IPI
static int gic_resched_int_base;
static int gic_call_int_base;

unsigned int plat_ipi_resched_int_xlate(unsigned int cpu)
{
	return gic_resched_int_base + cpu;
}

unsigned int plat_ipi_call_int_xlate(unsigned int cpu)
{
	return gic_call_int_base + cpu;
}

static irqreturn_t ipi_resched_interrupt(int irq, void *dev_id)
{
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
	.name		= "IPI resched"
};

static struct irqaction irq_call = {
	.handler	= ipi_call_interrupt,
	.flags		= IRQF_PERCPU,
	.name		= "IPI call"
};

static __init void gic_ipi_init_one(unsigned int intr, int cpu,
				    struct irqaction *action)
{
	int virq = irq_create_mapping(gic_irq_domain,
				      GIC_SHARED_TO_HWIRQ(intr));
	int i;

	gic_map_to_vpe(intr, cpu);
	for (i = 0; i < NR_CPUS; i++)
		clear_bit(intr, pcpu_masks[i].pcpu_mask);
	set_bit(intr, pcpu_masks[cpu].pcpu_mask);

	irq_set_irq_type(virq, IRQ_TYPE_EDGE_RISING);

	irq_set_handler(virq, handle_percpu_irq);
	setup_irq(virq, action);
}

static __init void gic_ipi_init(void)
{
	int i;

	/* Use last 2 * NR_CPUS interrupts as IPIs */
	gic_resched_int_base = gic_shared_intrs - nr_cpu_ids;
	gic_call_int_base = gic_resched_int_base - nr_cpu_ids;

	for (i = 0; i < nr_cpu_ids; i++) {
		gic_ipi_init_one(gic_call_int_base + i, i, &irq_call);
		gic_ipi_init_one(gic_resched_int_base + i, i, &irq_resched);
	}
}
#else
static inline void gic_ipi_init(void)
{
}
#endif

static void __init gic_basic_init(void)
{
	unsigned int i;

	board_bind_eic_interrupt = &gic_bind_eic_interrupt;

	/* Setup defaults */
	for (i = 0; i < gic_shared_intrs; i++) {
		gic_set_polarity(i, GIC_POL_POS);
		gic_set_trigger(i, GIC_TRIG_LEVEL);
		gic_reset_mask(i);
	}

	for (i = 0; i < gic_vpes; i++) {
		unsigned int j;

		gic_write(GIC_REG(VPE_LOCAL, GIC_VPE_OTHER_ADDR), i);
		for (j = 0; j < GIC_NUM_LOCAL_INTRS; j++) {
			if (!gic_local_irq_is_routable(j))
				continue;
			gic_write(GIC_REG(VPE_OTHER, GIC_VPE_RMASK), 1 << j);
		}
	}
}

static int gic_local_irq_domain_map(struct irq_domain *d, unsigned int virq,
				    irq_hw_number_t hw)
{
	int intr = GIC_HWIRQ_TO_LOCAL(hw);
	int ret = 0;
	int i;
	unsigned long flags;

	if (!gic_local_irq_is_routable(intr))
		return -EPERM;

	/*
	 * HACK: These are all really percpu interrupts, but the rest
	 * of the MIPS kernel code does not use the percpu IRQ API for
	 * the CP0 timer and performance counter interrupts.
	 */
	switch (intr) {
	case GIC_LOCAL_INT_TIMER:
	case GIC_LOCAL_INT_PERFCTR:
	case GIC_LOCAL_INT_FDC:
		irq_set_chip_and_handler(virq,
					 &gic_all_vpes_local_irq_controller,
					 handle_percpu_irq);
		break;
	default:
		irq_set_chip_and_handler(virq,
					 &gic_local_irq_controller,
					 handle_percpu_devid_irq);
		irq_set_percpu_devid(virq);
		break;
	}

	spin_lock_irqsave(&gic_lock, flags);
	for (i = 0; i < gic_vpes; i++) {
		u32 val = GIC_MAP_TO_PIN_MSK | gic_cpu_pin;

		gic_write(GIC_REG(VPE_LOCAL, GIC_VPE_OTHER_ADDR), i);

		switch (intr) {
		case GIC_LOCAL_INT_WD:
			gic_write(GIC_REG(VPE_OTHER, GIC_VPE_WD_MAP), val);
			break;
		case GIC_LOCAL_INT_COMPARE:
			gic_write(GIC_REG(VPE_OTHER, GIC_VPE_COMPARE_MAP), val);
			break;
		case GIC_LOCAL_INT_TIMER:
			/* CONFIG_MIPS_CMP workaround (see __gic_init) */
			val = GIC_MAP_TO_PIN_MSK | timer_cpu_pin;
			gic_write(GIC_REG(VPE_OTHER, GIC_VPE_TIMER_MAP), val);
			break;
		case GIC_LOCAL_INT_PERFCTR:
			gic_write(GIC_REG(VPE_OTHER, GIC_VPE_PERFCTR_MAP), val);
			break;
		case GIC_LOCAL_INT_SWINT0:
			gic_write(GIC_REG(VPE_OTHER, GIC_VPE_SWINT0_MAP), val);
			break;
		case GIC_LOCAL_INT_SWINT1:
			gic_write(GIC_REG(VPE_OTHER, GIC_VPE_SWINT1_MAP), val);
			break;
		case GIC_LOCAL_INT_FDC:
			gic_write(GIC_REG(VPE_OTHER, GIC_VPE_FDC_MAP), val);
			break;
		default:
			pr_err("Invalid local IRQ %d\n", intr);
			ret = -EINVAL;
			break;
		}
	}
	spin_unlock_irqrestore(&gic_lock, flags);

	return ret;
}

static int gic_shared_irq_domain_map(struct irq_domain *d, unsigned int virq,
				     irq_hw_number_t hw)
{
	int intr = GIC_HWIRQ_TO_SHARED(hw);
	unsigned long flags;

	irq_set_chip_and_handler(virq, &gic_level_irq_controller,
				 handle_level_irq);

	spin_lock_irqsave(&gic_lock, flags);
	gic_map_to_pin(intr, gic_cpu_pin);
	/* Map to VPE 0 by default */
	gic_map_to_vpe(intr, 0);
	set_bit(intr, pcpu_masks[0].pcpu_mask);
	spin_unlock_irqrestore(&gic_lock, flags);

	return 0;
}

static int gic_irq_domain_map(struct irq_domain *d, unsigned int virq,
			      irq_hw_number_t hw)
{
	if (GIC_HWIRQ_TO_LOCAL(hw) < GIC_NUM_LOCAL_INTRS)
		return gic_local_irq_domain_map(d, virq, hw);
	return gic_shared_irq_domain_map(d, virq, hw);
}

static int gic_irq_domain_xlate(struct irq_domain *d, struct device_node *ctrlr,
				const u32 *intspec, unsigned int intsize,
				irq_hw_number_t *out_hwirq,
				unsigned int *out_type)
{
	if (intsize != 3)
		return -EINVAL;

	if (intspec[0] == GIC_SHARED)
		*out_hwirq = GIC_SHARED_TO_HWIRQ(intspec[1]);
	else if (intspec[0] == GIC_LOCAL)
		*out_hwirq = GIC_LOCAL_TO_HWIRQ(intspec[1]);
	else
		return -EINVAL;
	*out_type = intspec[2] & IRQ_TYPE_SENSE_MASK;

	return 0;
}

static struct irq_domain_ops gic_irq_domain_ops = {
	.map = gic_irq_domain_map,
	.xlate = gic_irq_domain_xlate,
};

static void __init __gic_init(unsigned long gic_base_addr,
			      unsigned long gic_addrspace_size,
			      unsigned int cpu_vec, unsigned int irqbase,
			      struct device_node *node)
{
	unsigned int gicconfig;

	gic_base = ioremap_nocache(gic_base_addr, gic_addrspace_size);

	gicconfig = gic_read(GIC_REG(SHARED, GIC_SH_CONFIG));
	gic_shared_intrs = (gicconfig & GIC_SH_CONFIG_NUMINTRS_MSK) >>
		   GIC_SH_CONFIG_NUMINTRS_SHF;
	gic_shared_intrs = ((gic_shared_intrs + 1) * 8);

	gic_vpes = (gicconfig & GIC_SH_CONFIG_NUMVPES_MSK) >>
		  GIC_SH_CONFIG_NUMVPES_SHF;
	gic_vpes = gic_vpes + 1;

	if (cpu_has_veic) {
		/* Always use vector 1 in EIC mode */
		gic_cpu_pin = 0;
		timer_cpu_pin = gic_cpu_pin;
		set_vi_handler(gic_cpu_pin + GIC_PIN_TO_VEC_OFFSET,
			       __gic_irq_dispatch);
	} else {
		gic_cpu_pin = cpu_vec - GIC_CPU_PIN_OFFSET;
		irq_set_chained_handler(MIPS_CPU_IRQ_BASE + cpu_vec,
					gic_irq_dispatch);
		/*
		 * With the CMP implementation of SMP (deprecated), other CPUs
		 * are started by the bootloader and put into a timer based
		 * waiting poll loop. We must not re-route those CPU's local
		 * timer interrupts as the wait instruction will never finish,
		 * so just handle whatever CPU interrupt it is routed to by
		 * default.
		 *
		 * This workaround should be removed when CMP support is
		 * dropped.
		 */
		if (IS_ENABLED(CONFIG_MIPS_CMP) &&
		    gic_local_irq_is_routable(GIC_LOCAL_INT_TIMER)) {
			timer_cpu_pin = gic_read(GIC_REG(VPE_LOCAL,
							 GIC_VPE_TIMER_MAP)) &
					GIC_MAP_MSK;
			irq_set_chained_handler(MIPS_CPU_IRQ_BASE +
						GIC_CPU_PIN_OFFSET +
						timer_cpu_pin,
						gic_irq_dispatch);
		} else {
			timer_cpu_pin = gic_cpu_pin;
		}
	}

	gic_irq_domain = irq_domain_add_simple(node, GIC_NUM_LOCAL_INTRS +
					       gic_shared_intrs, irqbase,
					       &gic_irq_domain_ops, NULL);
	if (!gic_irq_domain)
		panic("Failed to add GIC IRQ domain");

	gic_basic_init();

	gic_ipi_init();
}

void __init gic_init(unsigned long gic_base_addr,
		     unsigned long gic_addrspace_size,
		     unsigned int cpu_vec, unsigned int irqbase)
{
	__gic_init(gic_base_addr, gic_addrspace_size, cpu_vec, irqbase, NULL);
}

static int __init gic_of_init(struct device_node *node,
			      struct device_node *parent)
{
	struct resource res;
	unsigned int cpu_vec, i = 0, reserved = 0;
	phys_addr_t gic_base;
	size_t gic_len;

	/* Find the first available CPU vector. */
	while (!of_property_read_u32_index(node, "mti,reserved-cpu-vectors",
					   i++, &cpu_vec))
		reserved |= BIT(cpu_vec);
	for (cpu_vec = 2; cpu_vec < 8; cpu_vec++) {
		if (!(reserved & BIT(cpu_vec)))
			break;
	}
	if (cpu_vec == 8) {
		pr_err("No CPU vectors available for GIC\n");
		return -ENODEV;
	}

	if (of_address_to_resource(node, 0, &res)) {
		/*
		 * Probe the CM for the GIC base address if not specified
		 * in the device-tree.
		 */
		if (mips_cm_present()) {
			gic_base = read_gcr_gic_base() &
				~CM_GCR_GIC_BASE_GICEN_MSK;
			gic_len = 0x20000;
		} else {
			pr_err("Failed to get GIC memory range\n");
			return -ENODEV;
		}
	} else {
		gic_base = res.start;
		gic_len = resource_size(&res);
	}

	if (mips_cm_present()) {
		write_gcr_gic_base(gic_base | CM_GCR_GIC_BASE_GICEN_MSK);
		/* Ensure GIC region is enabled before trying to access it */
		__sync();
	}
	gic_present = true;

	__gic_init(gic_base, gic_len, cpu_vec, 0, node);

	return 0;
}
IRQCHIP_DECLARE(mips_gic, "mti,gic", gic_of_init);
