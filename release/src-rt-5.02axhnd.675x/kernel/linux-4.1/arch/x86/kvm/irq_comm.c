/*
 * irq_comm.c: Common API for in kernel interrupt controller
 * Copyright (c) 2007, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 * Authors:
 *   Yaozu (Eddie) Dong <Eddie.dong@intel.com>
 *
 * Copyright 2010 Red Hat, Inc. and/or its affiliates.
 */

#include <linux/kvm_host.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <trace/events/kvm.h>

#include <asm/msidef.h>

#include "irq.h"

#include "ioapic.h"

static int kvm_set_pic_irq(struct kvm_kernel_irq_routing_entry *e,
			   struct kvm *kvm, int irq_source_id, int level,
			   bool line_status)
{
	struct kvm_pic *pic = pic_irqchip(kvm);
	return kvm_pic_set_irq(pic, e->irqchip.pin, irq_source_id, level);
}

static int kvm_set_ioapic_irq(struct kvm_kernel_irq_routing_entry *e,
			      struct kvm *kvm, int irq_source_id, int level,
			      bool line_status)
{
	struct kvm_ioapic *ioapic = kvm->arch.vioapic;
	return kvm_ioapic_set_irq(ioapic, e->irqchip.pin, irq_source_id, level,
				line_status);
}

inline static bool kvm_is_dm_lowest_prio(struct kvm_lapic_irq *irq)
{
	return irq->delivery_mode == APIC_DM_LOWEST;
}

int kvm_irq_delivery_to_apic(struct kvm *kvm, struct kvm_lapic *src,
		struct kvm_lapic_irq *irq, unsigned long *dest_map)
{
	int i, r = -1;
	struct kvm_vcpu *vcpu, *lowest = NULL;

	if (irq->dest_mode == 0 && irq->dest_id == 0xff &&
			kvm_is_dm_lowest_prio(irq)) {
		printk(KERN_INFO "kvm: apic: phys broadcast and lowest prio\n");
		irq->delivery_mode = APIC_DM_FIXED;
	}

	if (kvm_irq_delivery_to_apic_fast(kvm, src, irq, &r, dest_map))
		return r;

	kvm_for_each_vcpu(i, vcpu, kvm) {
		if (!kvm_apic_present(vcpu))
			continue;

		if (!kvm_apic_match_dest(vcpu, src, irq->shorthand,
					irq->dest_id, irq->dest_mode))
			continue;

		if (!kvm_is_dm_lowest_prio(irq)) {
			if (r < 0)
				r = 0;
			r += kvm_apic_set_irq(vcpu, irq, dest_map);
		} else if (kvm_lapic_enabled(vcpu)) {
			if (!lowest)
				lowest = vcpu;
			else if (kvm_apic_compare_prio(vcpu, lowest) < 0)
				lowest = vcpu;
		}
	}

	if (lowest)
		r = kvm_apic_set_irq(lowest, irq, dest_map);

	return r;
}

static inline void kvm_set_msi_irq(struct kvm_kernel_irq_routing_entry *e,
				   struct kvm_lapic_irq *irq)
{
	trace_kvm_msi_set_irq(e->msi.address_lo, e->msi.data);

	irq->dest_id = (e->msi.address_lo &
			MSI_ADDR_DEST_ID_MASK) >> MSI_ADDR_DEST_ID_SHIFT;
	irq->vector = (e->msi.data &
			MSI_DATA_VECTOR_MASK) >> MSI_DATA_VECTOR_SHIFT;
	irq->dest_mode = (1 << MSI_ADDR_DEST_MODE_SHIFT) & e->msi.address_lo;
	irq->trig_mode = (1 << MSI_DATA_TRIGGER_SHIFT) & e->msi.data;
	irq->delivery_mode = e->msi.data & 0x700;
	irq->level = 1;
	irq->shorthand = 0;
	/* TODO Deal with RH bit of MSI message address */
}

int kvm_set_msi(struct kvm_kernel_irq_routing_entry *e,
		struct kvm *kvm, int irq_source_id, int level, bool line_status)
{
	struct kvm_lapic_irq irq;

	if (!level)
		return -1;

	kvm_set_msi_irq(e, &irq);

	return kvm_irq_delivery_to_apic(kvm, NULL, &irq, NULL);
}


static int kvm_set_msi_inatomic(struct kvm_kernel_irq_routing_entry *e,
			 struct kvm *kvm)
{
	struct kvm_lapic_irq irq;
	int r;

	kvm_set_msi_irq(e, &irq);

	if (kvm_irq_delivery_to_apic_fast(kvm, NULL, &irq, &r, NULL))
		return r;
	else
		return -EWOULDBLOCK;
}

/*
 * Deliver an IRQ in an atomic context if we can, or return a failure,
 * user can retry in a process context.
 * Return value:
 *  -EWOULDBLOCK - Can't deliver in atomic context: retry in a process context.
 *  Other values - No need to retry.
 */
int kvm_set_irq_inatomic(struct kvm *kvm, int irq_source_id, u32 irq, int level)
{
	struct kvm_kernel_irq_routing_entry entries[KVM_NR_IRQCHIPS];
	struct kvm_kernel_irq_routing_entry *e;
	int ret = -EINVAL;
	int idx;

	trace_kvm_set_irq(irq, level, irq_source_id);

	/*
	 * Injection into either PIC or IOAPIC might need to scan all CPUs,
	 * which would need to be retried from thread context;  when same GSI
	 * is connected to both PIC and IOAPIC, we'd have to report a
	 * partial failure here.
	 * Since there's no easy way to do this, we only support injecting MSI
	 * which is limited to 1:1 GSI mapping.
	 */
	idx = srcu_read_lock(&kvm->irq_srcu);
	if (kvm_irq_map_gsi(kvm, entries, irq) > 0) {
		e = &entries[0];
		if (likely(e->type == KVM_IRQ_ROUTING_MSI))
			ret = kvm_set_msi_inatomic(e, kvm);
		else
			ret = -EWOULDBLOCK;
	}
	srcu_read_unlock(&kvm->irq_srcu, idx);
	return ret;
}

int kvm_request_irq_source_id(struct kvm *kvm)
{
	unsigned long *bitmap = &kvm->arch.irq_sources_bitmap;
	int irq_source_id;

	mutex_lock(&kvm->irq_lock);
	irq_source_id = find_first_zero_bit(bitmap, BITS_PER_LONG);

	if (irq_source_id >= BITS_PER_LONG) {
		printk(KERN_WARNING "kvm: exhaust allocatable IRQ sources!\n");
		irq_source_id = -EFAULT;
		goto unlock;
	}

	ASSERT(irq_source_id != KVM_USERSPACE_IRQ_SOURCE_ID);
	ASSERT(irq_source_id != KVM_IRQFD_RESAMPLE_IRQ_SOURCE_ID);
	set_bit(irq_source_id, bitmap);
unlock:
	mutex_unlock(&kvm->irq_lock);

	return irq_source_id;
}

void kvm_free_irq_source_id(struct kvm *kvm, int irq_source_id)
{
	ASSERT(irq_source_id != KVM_USERSPACE_IRQ_SOURCE_ID);
	ASSERT(irq_source_id != KVM_IRQFD_RESAMPLE_IRQ_SOURCE_ID);

	mutex_lock(&kvm->irq_lock);
	if (irq_source_id < 0 ||
	    irq_source_id >= BITS_PER_LONG) {
		printk(KERN_ERR "kvm: IRQ source ID out of range!\n");
		goto unlock;
	}
	clear_bit(irq_source_id, &kvm->arch.irq_sources_bitmap);
	if (!irqchip_in_kernel(kvm))
		goto unlock;

	kvm_ioapic_clear_all(kvm->arch.vioapic, irq_source_id);
	kvm_pic_clear_all(pic_irqchip(kvm), irq_source_id);
unlock:
	mutex_unlock(&kvm->irq_lock);
}

void kvm_register_irq_mask_notifier(struct kvm *kvm, int irq,
				    struct kvm_irq_mask_notifier *kimn)
{
	mutex_lock(&kvm->irq_lock);
	kimn->irq = irq;
	hlist_add_head_rcu(&kimn->link, &kvm->arch.mask_notifier_list);
	mutex_unlock(&kvm->irq_lock);
}

void kvm_unregister_irq_mask_notifier(struct kvm *kvm, int irq,
				      struct kvm_irq_mask_notifier *kimn)
{
	mutex_lock(&kvm->irq_lock);
	hlist_del_rcu(&kimn->link);
	mutex_unlock(&kvm->irq_lock);
	synchronize_srcu(&kvm->irq_srcu);
}

void kvm_fire_mask_notifiers(struct kvm *kvm, unsigned irqchip, unsigned pin,
			     bool mask)
{
	struct kvm_irq_mask_notifier *kimn;
	int idx, gsi;

	idx = srcu_read_lock(&kvm->irq_srcu);
	gsi = kvm_irq_map_chip_pin(kvm, irqchip, pin);
	if (gsi != -1)
		hlist_for_each_entry_rcu(kimn, &kvm->arch.mask_notifier_list, link)
			if (kimn->irq == gsi)
				kimn->func(kimn, mask);
	srcu_read_unlock(&kvm->irq_srcu, idx);
}

int kvm_set_routing_entry(struct kvm_kernel_irq_routing_entry *e,
			  const struct kvm_irq_routing_entry *ue)
{
	int r = -EINVAL;
	int delta;
	unsigned max_pin;

	switch (ue->type) {
	case KVM_IRQ_ROUTING_IRQCHIP:
		delta = 0;
		switch (ue->u.irqchip.irqchip) {
		case KVM_IRQCHIP_PIC_MASTER:
			e->set = kvm_set_pic_irq;
			max_pin = PIC_NUM_PINS;
			break;
		case KVM_IRQCHIP_PIC_SLAVE:
			e->set = kvm_set_pic_irq;
			max_pin = PIC_NUM_PINS;
			delta = 8;
			break;
		case KVM_IRQCHIP_IOAPIC:
			max_pin = KVM_IOAPIC_NUM_PINS;
			e->set = kvm_set_ioapic_irq;
			break;
		default:
			goto out;
		}
		e->irqchip.irqchip = ue->u.irqchip.irqchip;
		e->irqchip.pin = ue->u.irqchip.pin + delta;
		if (e->irqchip.pin >= max_pin)
			goto out;
		break;
	case KVM_IRQ_ROUTING_MSI:
		e->set = kvm_set_msi;
		e->msi.address_lo = ue->u.msi.address_lo;
		e->msi.address_hi = ue->u.msi.address_hi;
		e->msi.data = ue->u.msi.data;
		break;
	default:
		goto out;
	}

	r = 0;
out:
	return r;
}

#define IOAPIC_ROUTING_ENTRY(irq) \
	{ .gsi = irq, .type = KVM_IRQ_ROUTING_IRQCHIP,	\
	  .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (irq) } }
#define ROUTING_ENTRY1(irq) IOAPIC_ROUTING_ENTRY(irq)

#define PIC_ROUTING_ENTRY(irq) \
	{ .gsi = irq, .type = KVM_IRQ_ROUTING_IRQCHIP,	\
	  .u.irqchip = { .irqchip = SELECT_PIC(irq), .pin = (irq) % 8 } }
#define ROUTING_ENTRY2(irq) \
	IOAPIC_ROUTING_ENTRY(irq), PIC_ROUTING_ENTRY(irq)

static const struct kvm_irq_routing_entry default_routing[] = {
	ROUTING_ENTRY2(0), ROUTING_ENTRY2(1),
	ROUTING_ENTRY2(2), ROUTING_ENTRY2(3),
	ROUTING_ENTRY2(4), ROUTING_ENTRY2(5),
	ROUTING_ENTRY2(6), ROUTING_ENTRY2(7),
	ROUTING_ENTRY2(8), ROUTING_ENTRY2(9),
	ROUTING_ENTRY2(10), ROUTING_ENTRY2(11),
	ROUTING_ENTRY2(12), ROUTING_ENTRY2(13),
	ROUTING_ENTRY2(14), ROUTING_ENTRY2(15),
	ROUTING_ENTRY1(16), ROUTING_ENTRY1(17),
	ROUTING_ENTRY1(18), ROUTING_ENTRY1(19),
	ROUTING_ENTRY1(20), ROUTING_ENTRY1(21),
	ROUTING_ENTRY1(22), ROUTING_ENTRY1(23),
};

int kvm_setup_default_irq_routing(struct kvm *kvm)
{
	return kvm_set_irq_routing(kvm, default_routing,
				   ARRAY_SIZE(default_routing), 0);
}
