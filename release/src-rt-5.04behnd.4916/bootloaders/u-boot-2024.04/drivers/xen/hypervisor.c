// SPDX-License-Identifier: MIT License
/*
 * hypervisor.c
 *
 * Communication to/from hypervisor.
 *
 * Copyright (c) 2002-2003, K A Fraser
 * Copyright (c) 2005, Grzegorz Milos, gm281@cam.ac.uk,Intel Research Cambridge
 * Copyright (c) 2020, EPAM Systems Inc.
 */
#include <common.h>
#include <cpu_func.h>
#include <log.h>
#include <memalign.h>

#include <asm/io.h>
#include <asm/armv8/mmu.h>
#include <asm/xen/system.h>

#include <linux/bug.h>

#include <xen/hvm.h>
#include <xen/events.h>
#include <xen/gnttab.h>
#include <xen/xenbus.h>
#include <xen/interface/memory.h>

#define active_evtchns(cpu, sh, idx)	\
	((sh)->evtchn_pending[idx] &	\
	 ~(sh)->evtchn_mask[idx])

int in_callback;

/*
 * Shared page for communicating with the hypervisor.
 * Events flags go here, for example.
 */
struct shared_info *HYPERVISOR_shared_info;

static const char *param_name(int op)
{
#define PARAM(x)[HVM_PARAM_##x] = #x
	static const char *const names[] = {
		PARAM(CALLBACK_IRQ),
		PARAM(STORE_PFN),
		PARAM(STORE_EVTCHN),
		PARAM(PAE_ENABLED),
		PARAM(IOREQ_PFN),
		PARAM(VPT_ALIGN),
		PARAM(CONSOLE_PFN),
		PARAM(CONSOLE_EVTCHN),
	};
#undef PARAM

	if (op >= ARRAY_SIZE(names))
		return "unknown";

	if (!names[op])
		return "reserved";

	return names[op];
}

/**
 * hvm_get_parameter_maintain_dcache - function to obtain a HVM
 * parameter value.
 * @idx: HVM parameter index
 * @value: Value to fill in
 *
 * According to Xen on ARM ABI (xen/include/public/arch-arm.h):
 * all memory which is shared with other entities in the system
 * (including the hypervisor and other guests) must reside in memory
 * which is mapped as Normal Inner Write-Back Outer Write-Back
 * Inner-Shareable.
 *
 * Thus, page attributes must be equally set for all the entities
 * working with that page.
 *
 * Before MMU setup the data cache is turned off, so it means that
 * manual data cache maintenance is required, because of the
 * difference of page attributes.
 */
int hvm_get_parameter_maintain_dcache(int idx, uint64_t *value)
{
	struct xen_hvm_param xhv;
	int ret;

	invalidate_dcache_range((unsigned long)&xhv,
				(unsigned long)&xhv + sizeof(xhv));
	xhv.domid = DOMID_SELF;
	xhv.index = idx;
	invalidate_dcache_range((unsigned long)&xhv,
				(unsigned long)&xhv + sizeof(xhv));

	ret = HYPERVISOR_hvm_op(HVMOP_get_param, &xhv);
	if (ret < 0) {
		pr_err("Cannot get hvm parameter %s (%d): %d!\n",
			   param_name(idx), idx, ret);
		BUG();
	}
	invalidate_dcache_range((unsigned long)&xhv,
				(unsigned long)&xhv + sizeof(xhv));

	*value = xhv.value;

	return ret;
}

int hvm_get_parameter(int idx, uint64_t *value)
{
	struct xen_hvm_param xhv;
	int ret;

	xhv.domid = DOMID_SELF;
	xhv.index = idx;
	ret = HYPERVISOR_hvm_op(HVMOP_get_param, &xhv);
	if (ret < 0) {
		pr_err("Cannot get hvm parameter %s (%d): %d!\n",
			   param_name(idx), idx, ret);
		BUG();
	}

	*value = xhv.value;

	return ret;
}

struct shared_info *map_shared_info(void *p)
{
	struct xen_add_to_physmap xatp;

	HYPERVISOR_shared_info = (struct shared_info *)memalign(PAGE_SIZE,
								PAGE_SIZE);
	if (!HYPERVISOR_shared_info)
		BUG();

	xatp.domid = DOMID_SELF;
	xatp.idx = 0;
	xatp.space = XENMAPSPACE_shared_info;
	xatp.gpfn = virt_to_pfn(HYPERVISOR_shared_info);
	if (HYPERVISOR_memory_op(XENMEM_add_to_physmap, &xatp) != 0)
		BUG();

	return HYPERVISOR_shared_info;
}

void unmap_shared_info(void)
{
	xen_pfn_t shared_info_pfn = virt_to_pfn(HYPERVISOR_shared_info);
	struct xen_remove_from_physmap xrfp = {0};
	struct xen_memory_reservation reservation = {0};
	xen_ulong_t nr_exts = 1;

	xrfp.domid = DOMID_SELF;
	xrfp.gpfn = shared_info_pfn;
	if (HYPERVISOR_memory_op(XENMEM_remove_from_physmap, &xrfp) != 0)
		panic("Failed to unmap HYPERVISOR_shared_info\n");

	/*
	 * After removing from physmap there will be a hole in address space on
	 * HYPERVISOR_shared_info address, so to free memory allocated with
	 * memalign and prevent exceptions during access to this page we need to
	 * fill this 4KB hole with XENMEM_populate_physmap before jumping to Linux.
	 */
	reservation.domid = DOMID_SELF;
	reservation.extent_order = 0;
	reservation.address_bits = 0;
	set_xen_guest_handle(reservation.extent_start, &shared_info_pfn);
	reservation.nr_extents = nr_exts;
	if (HYPERVISOR_memory_op(XENMEM_populate_physmap, &reservation) != nr_exts)
		panic("Failed to populate memory on HYPERVISOR_shared_info addr\n");

	/* Now we can return this to memory allocator */
	free(HYPERVISOR_shared_info);
}

void do_hypervisor_callback(struct pt_regs *regs)
{
	unsigned long l1, l2, l1i, l2i;
	unsigned int port;
	int cpu = 0;
	struct shared_info *s = HYPERVISOR_shared_info;
	struct vcpu_info *vcpu_info = &s->vcpu_info[cpu];

	in_callback = 1;

	vcpu_info->evtchn_upcall_pending = 0;
	l1 = xchg(&vcpu_info->evtchn_pending_sel, 0);

	while (l1 != 0) {
		l1i = __ffs(l1);
		l1 &= ~(1UL << l1i);

		while ((l2 = active_evtchns(cpu, s, l1i)) != 0) {
			l2i = __ffs(l2);
			l2 &= ~(1UL << l2i);

			port = (l1i * (sizeof(unsigned long) * 8)) + l2i;
			do_event(port, regs);
		}
	}

	in_callback = 0;
}

void force_evtchn_callback(void)
{
#ifdef XEN_HAVE_PV_UPCALL_MASK
	int save;
#endif
	struct vcpu_info *vcpu;

	vcpu = &HYPERVISOR_shared_info->vcpu_info[smp_processor_id()];
#ifdef XEN_HAVE_PV_UPCALL_MASK
	save = vcpu->evtchn_upcall_mask;
#endif

	while (vcpu->evtchn_upcall_pending) {
#ifdef XEN_HAVE_PV_UPCALL_MASK
		vcpu->evtchn_upcall_mask = 1;
#endif
		do_hypervisor_callback(NULL);
#ifdef XEN_HAVE_PV_UPCALL_MASK
		vcpu->evtchn_upcall_mask = save;
#endif
	};
}

void mask_evtchn(uint32_t port)
{
	struct shared_info *s = HYPERVISOR_shared_info;

	synch_set_bit(port, &s->evtchn_mask[0]);
}

void unmask_evtchn(uint32_t port)
{
	struct shared_info *s = HYPERVISOR_shared_info;
	struct vcpu_info *vcpu_info = &s->vcpu_info[smp_processor_id()];

	synch_clear_bit(port, &s->evtchn_mask[0]);

	/*
	 * Just like a real IO-APIC we 'lose the interrupt edge' if the
	 * channel is masked.
	 */
	if (synch_test_bit(port, &s->evtchn_pending[0]) &&
	    !synch_test_and_set_bit(port / (sizeof(unsigned long) * 8),
				    &vcpu_info->evtchn_pending_sel)) {
		vcpu_info->evtchn_upcall_pending = 1;
#ifdef XEN_HAVE_PV_UPCALL_MASK
		if (!vcpu_info->evtchn_upcall_mask)
#endif
			force_evtchn_callback();
	}
}

void clear_evtchn(uint32_t port)
{
	struct shared_info *s = HYPERVISOR_shared_info;

	synch_clear_bit(port, &s->evtchn_pending[0]);
}

int xen_init(void)
{
	int el = current_el();

	debug("%s\n", __func__);

	if (el != 1) {
		puts("XEN:\tnot running from EL1\n");
		return 0;
	}

	map_shared_info(NULL);
	init_events();
	init_xenbus();
	init_gnttab();

	return 0;
}

void xen_fini(void)
{
	debug("%s\n", __func__);

	fini_gnttab();
	fini_xenbus();
	fini_events();
	unmap_shared_info();
}
