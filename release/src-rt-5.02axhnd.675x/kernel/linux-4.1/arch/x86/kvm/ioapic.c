/*
 *  Copyright (C) 2001  MandrakeSoft S.A.
 *  Copyright 2010 Red Hat, Inc. and/or its affiliates.
 *
 *    MandrakeSoft S.A.
 *    43, rue d'Aboukir
 *    75002 Paris - France
 *    http://www.linux-mandrake.com/
 *    http://www.mandrakesoft.com/
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *  Yunhong Jiang <yunhong.jiang@intel.com>
 *  Yaozu (Eddie) Dong <eddie.dong@intel.com>
 *  Based on Xen 3.1 code.
 */

#include <linux/kvm_host.h>
#include <linux/kvm.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/smp.h>
#include <linux/hrtimer.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <asm/processor.h>
#include <asm/page.h>
#include <asm/current.h>
#include <trace/events/kvm.h>

#include "ioapic.h"
#include "lapic.h"
#include "irq.h"

#if 0
#define ioapic_debug(fmt,arg...) printk(KERN_WARNING fmt,##arg)
#else
#define ioapic_debug(fmt, arg...)
#endif
static int ioapic_service(struct kvm_ioapic *vioapic, int irq,
		bool line_status);

static unsigned long ioapic_read_indirect(struct kvm_ioapic *ioapic,
					  unsigned long addr,
					  unsigned long length)
{
	unsigned long result = 0;

	switch (ioapic->ioregsel) {
	case IOAPIC_REG_VERSION:
		result = ((((IOAPIC_NUM_PINS - 1) & 0xff) << 16)
			  | (IOAPIC_VERSION_ID & 0xff));
		break;

	case IOAPIC_REG_APIC_ID:
	case IOAPIC_REG_ARB_ID:
		result = ((ioapic->id & 0xf) << 24);
		break;

	default:
		{
			u32 redir_index = (ioapic->ioregsel - 0x10) >> 1;
			u64 redir_content;

			if (redir_index < IOAPIC_NUM_PINS)
				redir_content =
					ioapic->redirtbl[redir_index].bits;
			else
				redir_content = ~0ULL;

			result = (ioapic->ioregsel & 0x1) ?
			    (redir_content >> 32) & 0xffffffff :
			    redir_content & 0xffffffff;
			break;
		}
	}

	return result;
}

static void rtc_irq_eoi_tracking_reset(struct kvm_ioapic *ioapic)
{
	ioapic->rtc_status.pending_eoi = 0;
	bitmap_zero(ioapic->rtc_status.dest_map, KVM_MAX_VCPUS);
}

static void kvm_rtc_eoi_tracking_restore_all(struct kvm_ioapic *ioapic);

static void rtc_status_pending_eoi_check_valid(struct kvm_ioapic *ioapic)
{
	if (WARN_ON(ioapic->rtc_status.pending_eoi < 0))
		kvm_rtc_eoi_tracking_restore_all(ioapic);
}

static void __rtc_irq_eoi_tracking_restore_one(struct kvm_vcpu *vcpu)
{
	bool new_val, old_val;
	struct kvm_ioapic *ioapic = vcpu->kvm->arch.vioapic;
	union kvm_ioapic_redirect_entry *e;

	e = &ioapic->redirtbl[RTC_GSI];
	if (!kvm_apic_match_dest(vcpu, NULL, 0,	e->fields.dest_id,
				e->fields.dest_mode))
		return;

	new_val = kvm_apic_pending_eoi(vcpu, e->fields.vector);
	old_val = test_bit(vcpu->vcpu_id, ioapic->rtc_status.dest_map);

	if (new_val == old_val)
		return;

	if (new_val) {
		__set_bit(vcpu->vcpu_id, ioapic->rtc_status.dest_map);
		ioapic->rtc_status.pending_eoi++;
	} else {
		__clear_bit(vcpu->vcpu_id, ioapic->rtc_status.dest_map);
		ioapic->rtc_status.pending_eoi--;
		rtc_status_pending_eoi_check_valid(ioapic);
	}
}

void kvm_rtc_eoi_tracking_restore_one(struct kvm_vcpu *vcpu)
{
	struct kvm_ioapic *ioapic = vcpu->kvm->arch.vioapic;

	spin_lock(&ioapic->lock);
	__rtc_irq_eoi_tracking_restore_one(vcpu);
	spin_unlock(&ioapic->lock);
}

static void kvm_rtc_eoi_tracking_restore_all(struct kvm_ioapic *ioapic)
{
	struct kvm_vcpu *vcpu;
	int i;

	if (RTC_GSI >= IOAPIC_NUM_PINS)
		return;

	rtc_irq_eoi_tracking_reset(ioapic);
	kvm_for_each_vcpu(i, vcpu, ioapic->kvm)
	    __rtc_irq_eoi_tracking_restore_one(vcpu);
}

static void rtc_irq_eoi(struct kvm_ioapic *ioapic, struct kvm_vcpu *vcpu)
{
	if (test_and_clear_bit(vcpu->vcpu_id, ioapic->rtc_status.dest_map)) {
		--ioapic->rtc_status.pending_eoi;
		rtc_status_pending_eoi_check_valid(ioapic);
	}
}

static bool rtc_irq_check_coalesced(struct kvm_ioapic *ioapic)
{
	if (ioapic->rtc_status.pending_eoi > 0)
		return true; /* coalesced */

	return false;
}

static int ioapic_set_irq(struct kvm_ioapic *ioapic, unsigned int irq,
		int irq_level, bool line_status)
{
	union kvm_ioapic_redirect_entry entry;
	u32 mask = 1 << irq;
	u32 old_irr;
	int edge, ret;

	entry = ioapic->redirtbl[irq];
	edge = (entry.fields.trig_mode == IOAPIC_EDGE_TRIG);

	if (!irq_level) {
		ioapic->irr &= ~mask;
		ret = 1;
		goto out;
	}

	/*
	 * Return 0 for coalesced interrupts; for edge-triggered interrupts,
	 * this only happens if a previous edge has not been delivered due
	 * do masking.  For level interrupts, the remote_irr field tells
	 * us if the interrupt is waiting for an EOI.
	 *
	 * RTC is special: it is edge-triggered, but userspace likes to know
	 * if it has been already ack-ed via EOI because coalesced RTC
	 * interrupts lead to time drift in Windows guests.  So we track
	 * EOI manually for the RTC interrupt.
	 */
	if (irq == RTC_GSI && line_status &&
		rtc_irq_check_coalesced(ioapic)) {
		ret = 0;
		goto out;
	}

	old_irr = ioapic->irr;
	ioapic->irr |= mask;
	if (edge)
		ioapic->irr_delivered &= ~mask;
	if ((edge && old_irr == ioapic->irr) ||
	    (!edge && entry.fields.remote_irr)) {
		ret = 0;
		goto out;
	}

	ret = ioapic_service(ioapic, irq, line_status);

out:
	trace_kvm_ioapic_set_irq(entry.bits, irq, ret == 0);
	return ret;
}

static void kvm_ioapic_inject_all(struct kvm_ioapic *ioapic, unsigned long irr)
{
	u32 idx;

	rtc_irq_eoi_tracking_reset(ioapic);
	for_each_set_bit(idx, &irr, IOAPIC_NUM_PINS)
		ioapic_set_irq(ioapic, idx, 1, true);

	kvm_rtc_eoi_tracking_restore_all(ioapic);
}


static void update_handled_vectors(struct kvm_ioapic *ioapic)
{
	DECLARE_BITMAP(handled_vectors, 256);
	int i;

	memset(handled_vectors, 0, sizeof(handled_vectors));
	for (i = 0; i < IOAPIC_NUM_PINS; ++i)
		__set_bit(ioapic->redirtbl[i].fields.vector, handled_vectors);
	memcpy(ioapic->handled_vectors, handled_vectors,
	       sizeof(handled_vectors));
	smp_wmb();
}

void kvm_ioapic_scan_entry(struct kvm_vcpu *vcpu, u64 *eoi_exit_bitmap,
			u32 *tmr)
{
	struct kvm_ioapic *ioapic = vcpu->kvm->arch.vioapic;
	union kvm_ioapic_redirect_entry *e;
	int index;

	spin_lock(&ioapic->lock);
	for (index = 0; index < IOAPIC_NUM_PINS; index++) {
		e = &ioapic->redirtbl[index];
		if (e->fields.trig_mode == IOAPIC_LEVEL_TRIG ||
		    kvm_irq_has_notifier(ioapic->kvm, KVM_IRQCHIP_IOAPIC, index) ||
		    index == RTC_GSI) {
			if (kvm_apic_match_dest(vcpu, NULL, 0,
				e->fields.dest_id, e->fields.dest_mode)) {
				__set_bit(e->fields.vector,
					(unsigned long *)eoi_exit_bitmap);
				if (e->fields.trig_mode == IOAPIC_LEVEL_TRIG)
					__set_bit(e->fields.vector,
						(unsigned long *)tmr);
			}
		}
	}
	spin_unlock(&ioapic->lock);
}

void kvm_vcpu_request_scan_ioapic(struct kvm *kvm)
{
	struct kvm_ioapic *ioapic = kvm->arch.vioapic;

	if (!ioapic)
		return;
	kvm_make_scan_ioapic_request(kvm);
}

static void ioapic_write_indirect(struct kvm_ioapic *ioapic, u32 val)
{
	unsigned index;
	bool mask_before, mask_after;
	union kvm_ioapic_redirect_entry *e;

	switch (ioapic->ioregsel) {
	case IOAPIC_REG_VERSION:
		/* Writes are ignored. */
		break;

	case IOAPIC_REG_APIC_ID:
		ioapic->id = (val >> 24) & 0xf;
		break;

	case IOAPIC_REG_ARB_ID:
		break;

	default:
		index = (ioapic->ioregsel - 0x10) >> 1;

		ioapic_debug("change redir index %x val %x\n", index, val);
		if (index >= IOAPIC_NUM_PINS)
			return;
		e = &ioapic->redirtbl[index];
		mask_before = e->fields.mask;
		if (ioapic->ioregsel & 1) {
			e->bits &= 0xffffffff;
			e->bits |= (u64) val << 32;
		} else {
			e->bits &= ~0xffffffffULL;
			e->bits |= (u32) val;
			e->fields.remote_irr = 0;
		}
		update_handled_vectors(ioapic);
		mask_after = e->fields.mask;
		if (mask_before != mask_after)
			kvm_fire_mask_notifiers(ioapic->kvm, KVM_IRQCHIP_IOAPIC, index, mask_after);
		if (e->fields.trig_mode == IOAPIC_LEVEL_TRIG
		    && ioapic->irr & (1 << index))
			ioapic_service(ioapic, index, false);
		kvm_vcpu_request_scan_ioapic(ioapic->kvm);
		break;
	}
}

static int ioapic_service(struct kvm_ioapic *ioapic, int irq, bool line_status)
{
	union kvm_ioapic_redirect_entry *entry = &ioapic->redirtbl[irq];
	struct kvm_lapic_irq irqe;
	int ret;

	if (entry->fields.mask)
		return -1;

	ioapic_debug("dest=%x dest_mode=%x delivery_mode=%x "
		     "vector=%x trig_mode=%x\n",
		     entry->fields.dest_id, entry->fields.dest_mode,
		     entry->fields.delivery_mode, entry->fields.vector,
		     entry->fields.trig_mode);

	irqe.dest_id = entry->fields.dest_id;
	irqe.vector = entry->fields.vector;
	irqe.dest_mode = entry->fields.dest_mode;
	irqe.trig_mode = entry->fields.trig_mode;
	irqe.delivery_mode = entry->fields.delivery_mode << 8;
	irqe.level = 1;
	irqe.shorthand = 0;

	if (irqe.trig_mode == IOAPIC_EDGE_TRIG)
		ioapic->irr_delivered |= 1 << irq;

	if (irq == RTC_GSI && line_status) {
		/*
		 * pending_eoi cannot ever become negative (see
		 * rtc_status_pending_eoi_check_valid) and the caller
		 * ensures that it is only called if it is >= zero, namely
		 * if rtc_irq_check_coalesced returns false).
		 */
		BUG_ON(ioapic->rtc_status.pending_eoi != 0);
		ret = kvm_irq_delivery_to_apic(ioapic->kvm, NULL, &irqe,
				ioapic->rtc_status.dest_map);
		ioapic->rtc_status.pending_eoi = (ret < 0 ? 0 : ret);
	} else
		ret = kvm_irq_delivery_to_apic(ioapic->kvm, NULL, &irqe, NULL);

	if (ret && irqe.trig_mode == IOAPIC_LEVEL_TRIG)
		entry->fields.remote_irr = 1;

	return ret;
}

int kvm_ioapic_set_irq(struct kvm_ioapic *ioapic, int irq, int irq_source_id,
		       int level, bool line_status)
{
	int ret, irq_level;

	BUG_ON(irq < 0 || irq >= IOAPIC_NUM_PINS);

	spin_lock(&ioapic->lock);
	irq_level = __kvm_irq_line_state(&ioapic->irq_states[irq],
					 irq_source_id, level);
	ret = ioapic_set_irq(ioapic, irq, irq_level, line_status);

	spin_unlock(&ioapic->lock);

	return ret;
}

void kvm_ioapic_clear_all(struct kvm_ioapic *ioapic, int irq_source_id)
{
	int i;

	spin_lock(&ioapic->lock);
	for (i = 0; i < KVM_IOAPIC_NUM_PINS; i++)
		__clear_bit(irq_source_id, &ioapic->irq_states[i]);
	spin_unlock(&ioapic->lock);
}

static void kvm_ioapic_eoi_inject_work(struct work_struct *work)
{
	int i;
	struct kvm_ioapic *ioapic = container_of(work, struct kvm_ioapic,
						 eoi_inject.work);
	spin_lock(&ioapic->lock);
	for (i = 0; i < IOAPIC_NUM_PINS; i++) {
		union kvm_ioapic_redirect_entry *ent = &ioapic->redirtbl[i];

		if (ent->fields.trig_mode != IOAPIC_LEVEL_TRIG)
			continue;

		if (ioapic->irr & (1 << i) && !ent->fields.remote_irr)
			ioapic_service(ioapic, i, false);
	}
	spin_unlock(&ioapic->lock);
}

#define IOAPIC_SUCCESSIVE_IRQ_MAX_COUNT 10000

static void __kvm_ioapic_update_eoi(struct kvm_vcpu *vcpu,
			struct kvm_ioapic *ioapic, int vector, int trigger_mode)
{
	int i;
	struct kvm_lapic *apic = vcpu->arch.apic;

	for (i = 0; i < IOAPIC_NUM_PINS; i++) {
		union kvm_ioapic_redirect_entry *ent = &ioapic->redirtbl[i];

		if (ent->fields.vector != vector)
			continue;

		if (i == RTC_GSI)
			rtc_irq_eoi(ioapic, vcpu);
		/*
		 * We are dropping lock while calling ack notifiers because ack
		 * notifier callbacks for assigned devices call into IOAPIC
		 * recursively. Since remote_irr is cleared only after call
		 * to notifiers if the same vector will be delivered while lock
		 * is dropped it will be put into irr and will be delivered
		 * after ack notifier returns.
		 */
		spin_unlock(&ioapic->lock);
		kvm_notify_acked_irq(ioapic->kvm, KVM_IRQCHIP_IOAPIC, i);
		spin_lock(&ioapic->lock);

		if (trigger_mode != IOAPIC_LEVEL_TRIG ||
		    kvm_apic_get_reg(apic, APIC_SPIV) & APIC_SPIV_DIRECTED_EOI)
			continue;

		ASSERT(ent->fields.trig_mode == IOAPIC_LEVEL_TRIG);
		ent->fields.remote_irr = 0;
		if (!ent->fields.mask && (ioapic->irr & (1 << i))) {
			++ioapic->irq_eoi[i];
			if (ioapic->irq_eoi[i] == IOAPIC_SUCCESSIVE_IRQ_MAX_COUNT) {
				/*
				 * Real hardware does not deliver the interrupt
				 * immediately during eoi broadcast, and this
				 * lets a buggy guest make slow progress
				 * even if it does not correctly handle a
				 * level-triggered interrupt.  Emulate this
				 * behavior if we detect an interrupt storm.
				 */
				schedule_delayed_work(&ioapic->eoi_inject, HZ / 100);
				ioapic->irq_eoi[i] = 0;
				trace_kvm_ioapic_delayed_eoi_inj(ent->bits);
			} else {
				ioapic_service(ioapic, i, false);
			}
		} else {
			ioapic->irq_eoi[i] = 0;
		}
	}
}

void kvm_ioapic_update_eoi(struct kvm_vcpu *vcpu, int vector, int trigger_mode)
{
	struct kvm_ioapic *ioapic = vcpu->kvm->arch.vioapic;

	spin_lock(&ioapic->lock);
	__kvm_ioapic_update_eoi(vcpu, ioapic, vector, trigger_mode);
	spin_unlock(&ioapic->lock);
}

static inline struct kvm_ioapic *to_ioapic(struct kvm_io_device *dev)
{
	return container_of(dev, struct kvm_ioapic, dev);
}

static inline int ioapic_in_range(struct kvm_ioapic *ioapic, gpa_t addr)
{
	return ((addr >= ioapic->base_address &&
		 (addr < ioapic->base_address + IOAPIC_MEM_LENGTH)));
}

static int ioapic_mmio_read(struct kvm_vcpu *vcpu, struct kvm_io_device *this,
				gpa_t addr, int len, void *val)
{
	struct kvm_ioapic *ioapic = to_ioapic(this);
	u32 result;
	if (!ioapic_in_range(ioapic, addr))
		return -EOPNOTSUPP;

	ioapic_debug("addr %lx\n", (unsigned long)addr);
	ASSERT(!(addr & 0xf));	/* check alignment */

	addr &= 0xff;
	spin_lock(&ioapic->lock);
	switch (addr) {
	case IOAPIC_REG_SELECT:
		result = ioapic->ioregsel;
		break;

	case IOAPIC_REG_WINDOW:
		result = ioapic_read_indirect(ioapic, addr, len);
		break;

	default:
		result = 0;
		break;
	}
	spin_unlock(&ioapic->lock);

	switch (len) {
	case 8:
		*(u64 *) val = result;
		break;
	case 1:
	case 2:
	case 4:
		memcpy(val, (char *)&result, len);
		break;
	default:
		printk(KERN_WARNING "ioapic: wrong length %d\n", len);
	}
	return 0;
}

static int ioapic_mmio_write(struct kvm_vcpu *vcpu, struct kvm_io_device *this,
				 gpa_t addr, int len, const void *val)
{
	struct kvm_ioapic *ioapic = to_ioapic(this);
	u32 data;
	if (!ioapic_in_range(ioapic, addr))
		return -EOPNOTSUPP;

	ioapic_debug("ioapic_mmio_write addr=%p len=%d val=%p\n",
		     (void*)addr, len, val);
	ASSERT(!(addr & 0xf));	/* check alignment */

	switch (len) {
	case 8:
	case 4:
		data = *(u32 *) val;
		break;
	case 2:
		data = *(u16 *) val;
		break;
	case 1:
		data = *(u8  *) val;
		break;
	default:
		printk(KERN_WARNING "ioapic: Unsupported size %d\n", len);
		return 0;
	}

	addr &= 0xff;
	spin_lock(&ioapic->lock);
	switch (addr) {
	case IOAPIC_REG_SELECT:
		ioapic->ioregsel = data & 0xFF; /* 8-bit register */
		break;

	case IOAPIC_REG_WINDOW:
		ioapic_write_indirect(ioapic, data);
		break;

	default:
		break;
	}
	spin_unlock(&ioapic->lock);
	return 0;
}

static void kvm_ioapic_reset(struct kvm_ioapic *ioapic)
{
	int i;

	cancel_delayed_work_sync(&ioapic->eoi_inject);
	for (i = 0; i < IOAPIC_NUM_PINS; i++)
		ioapic->redirtbl[i].fields.mask = 1;
	ioapic->base_address = IOAPIC_DEFAULT_BASE_ADDRESS;
	ioapic->ioregsel = 0;
	ioapic->irr = 0;
	ioapic->irr_delivered = 0;
	ioapic->id = 0;
	memset(ioapic->irq_eoi, 0x00, sizeof(ioapic->irq_eoi));
	rtc_irq_eoi_tracking_reset(ioapic);
	update_handled_vectors(ioapic);
}

static const struct kvm_io_device_ops ioapic_mmio_ops = {
	.read     = ioapic_mmio_read,
	.write    = ioapic_mmio_write,
};

int kvm_ioapic_init(struct kvm *kvm)
{
	struct kvm_ioapic *ioapic;
	int ret;

	ioapic = kzalloc(sizeof(struct kvm_ioapic), GFP_KERNEL);
	if (!ioapic)
		return -ENOMEM;
	spin_lock_init(&ioapic->lock);
	INIT_DELAYED_WORK(&ioapic->eoi_inject, kvm_ioapic_eoi_inject_work);
	kvm->arch.vioapic = ioapic;
	kvm_ioapic_reset(ioapic);
	kvm_iodevice_init(&ioapic->dev, &ioapic_mmio_ops);
	ioapic->kvm = kvm;
	mutex_lock(&kvm->slots_lock);
	ret = kvm_io_bus_register_dev(kvm, KVM_MMIO_BUS, ioapic->base_address,
				      IOAPIC_MEM_LENGTH, &ioapic->dev);
	mutex_unlock(&kvm->slots_lock);
	if (ret < 0) {
		kvm->arch.vioapic = NULL;
		kfree(ioapic);
	}

	return ret;
}

void kvm_ioapic_destroy(struct kvm *kvm)
{
	struct kvm_ioapic *ioapic = kvm->arch.vioapic;

	cancel_delayed_work_sync(&ioapic->eoi_inject);
	if (ioapic) {
		kvm_io_bus_unregister_dev(kvm, KVM_MMIO_BUS, &ioapic->dev);
		kvm->arch.vioapic = NULL;
		kfree(ioapic);
	}
}

int kvm_get_ioapic(struct kvm *kvm, struct kvm_ioapic_state *state)
{
	struct kvm_ioapic *ioapic = ioapic_irqchip(kvm);
	if (!ioapic)
		return -EINVAL;

	spin_lock(&ioapic->lock);
	memcpy(state, ioapic, sizeof(struct kvm_ioapic_state));
	state->irr &= ~ioapic->irr_delivered;
	spin_unlock(&ioapic->lock);
	return 0;
}

int kvm_set_ioapic(struct kvm *kvm, struct kvm_ioapic_state *state)
{
	struct kvm_ioapic *ioapic = ioapic_irqchip(kvm);
	if (!ioapic)
		return -EINVAL;

	spin_lock(&ioapic->lock);
	memcpy(ioapic, state, sizeof(struct kvm_ioapic_state));
	ioapic->irr = 0;
	ioapic->irr_delivered = 0;
	update_handled_vectors(ioapic);
	kvm_vcpu_request_scan_ioapic(kvm);
	kvm_ioapic_inject_all(ioapic, state->irr);
	spin_unlock(&ioapic->lock);
	return 0;
}
