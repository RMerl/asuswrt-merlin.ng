/*
 * Copyright (C) 2012 - Virtual Open Systems and Columbia University
 * Author: Christoffer Dall <c.dall@virtualopensystems.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/cpu.h>
#include <linux/cpu_pm.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/kvm_host.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/mman.h>
#include <linux/sched.h>
#include <linux/kvm.h>
#include <trace/events/kvm.h>

#define CREATE_TRACE_POINTS
#include "trace.h"

#include <asm/uaccess.h>
#include <asm/ptrace.h>
#include <asm/mman.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <asm/virt.h>
#include <asm/kvm_arm.h>
#include <asm/kvm_asm.h>
#include <asm/kvm_mmu.h>
#include <asm/kvm_emulate.h>
#include <asm/kvm_coproc.h>
#include <asm/kvm_psci.h>

#ifdef REQUIRES_VIRT
__asm__(".arch_extension	virt");
#endif

static DEFINE_PER_CPU(unsigned long, kvm_arm_hyp_stack_page);
static kvm_cpu_context_t __percpu *kvm_host_cpu_state;
static unsigned long hyp_default_vectors;

/* Per-CPU variable containing the currently running vcpu. */
static DEFINE_PER_CPU(struct kvm_vcpu *, kvm_arm_running_vcpu);

/* The VMID used in the VTTBR */
static atomic64_t kvm_vmid_gen = ATOMIC64_INIT(1);
static u8 kvm_next_vmid;
static DEFINE_SPINLOCK(kvm_vmid_lock);

static void kvm_arm_set_running_vcpu(struct kvm_vcpu *vcpu)
{
	BUG_ON(preemptible());
	__this_cpu_write(kvm_arm_running_vcpu, vcpu);
}

/**
 * kvm_arm_get_running_vcpu - get the vcpu running on the current CPU.
 * Must be called from non-preemptible context
 */
struct kvm_vcpu *kvm_arm_get_running_vcpu(void)
{
	BUG_ON(preemptible());
	return __this_cpu_read(kvm_arm_running_vcpu);
}

/**
 * kvm_arm_get_running_vcpus - get the per-CPU array of currently running vcpus.
 */
struct kvm_vcpu * __percpu *kvm_get_running_vcpus(void)
{
	return &kvm_arm_running_vcpu;
}

int kvm_arch_hardware_enable(void)
{
	return 0;
}

int kvm_arch_vcpu_should_kick(struct kvm_vcpu *vcpu)
{
	return kvm_vcpu_exiting_guest_mode(vcpu) == IN_GUEST_MODE;
}

int kvm_arch_hardware_setup(void)
{
	return 0;
}

void kvm_arch_check_processor_compat(void *rtn)
{
	*(int *)rtn = 0;
}


/**
 * kvm_arch_init_vm - initializes a VM data structure
 * @kvm:	pointer to the KVM struct
 */
int kvm_arch_init_vm(struct kvm *kvm, unsigned long type)
{
	int ret = 0;

	if (type)
		return -EINVAL;

	ret = kvm_alloc_stage2_pgd(kvm);
	if (ret)
		goto out_fail_alloc;

	ret = create_hyp_mappings(kvm, kvm + 1);
	if (ret)
		goto out_free_stage2_pgd;

	kvm_timer_init(kvm);

	/* Mark the initial VMID generation invalid */
	kvm->arch.vmid_gen = 0;

	/* The maximum number of VCPUs is limited by the host's GIC model */
	kvm->arch.max_vcpus = kvm_vgic_get_max_vcpus();

	return ret;
out_free_stage2_pgd:
	kvm_free_stage2_pgd(kvm);
out_fail_alloc:
	return ret;
}

int kvm_arch_vcpu_fault(struct kvm_vcpu *vcpu, struct vm_fault *vmf)
{
	return VM_FAULT_SIGBUS;
}


/**
 * kvm_arch_destroy_vm - destroy the VM data structure
 * @kvm:	pointer to the KVM struct
 */
void kvm_arch_destroy_vm(struct kvm *kvm)
{
	int i;

	for (i = 0; i < KVM_MAX_VCPUS; ++i) {
		if (kvm->vcpus[i]) {
			kvm_arch_vcpu_free(kvm->vcpus[i]);
			kvm->vcpus[i] = NULL;
		}
	}

	kvm_vgic_destroy(kvm);
}

int kvm_vm_ioctl_check_extension(struct kvm *kvm, long ext)
{
	int r;
	switch (ext) {
	case KVM_CAP_IRQCHIP:
	case KVM_CAP_IRQFD:
	case KVM_CAP_IOEVENTFD:
	case KVM_CAP_DEVICE_CTRL:
	case KVM_CAP_USER_MEMORY:
	case KVM_CAP_SYNC_MMU:
	case KVM_CAP_DESTROY_MEMORY_REGION_WORKS:
	case KVM_CAP_ONE_REG:
	case KVM_CAP_ARM_PSCI:
	case KVM_CAP_ARM_PSCI_0_2:
	case KVM_CAP_READONLY_MEM:
	case KVM_CAP_MP_STATE:
		r = 1;
		break;
	case KVM_CAP_COALESCED_MMIO:
		r = KVM_COALESCED_MMIO_PAGE_OFFSET;
		break;
	case KVM_CAP_ARM_SET_DEVICE_ADDR:
		r = 1;
		break;
	case KVM_CAP_NR_VCPUS:
		r = num_online_cpus();
		break;
	case KVM_CAP_MAX_VCPUS:
		r = KVM_MAX_VCPUS;
		break;
	default:
		r = kvm_arch_dev_ioctl_check_extension(ext);
		break;
	}
	return r;
}

long kvm_arch_dev_ioctl(struct file *filp,
			unsigned int ioctl, unsigned long arg)
{
	return -EINVAL;
}


struct kvm_vcpu *kvm_arch_vcpu_create(struct kvm *kvm, unsigned int id)
{
	int err;
	struct kvm_vcpu *vcpu;

	if (irqchip_in_kernel(kvm) && vgic_initialized(kvm)) {
		err = -EBUSY;
		goto out;
	}

	if (id >= kvm->arch.max_vcpus) {
		err = -EINVAL;
		goto out;
	}

	vcpu = kmem_cache_zalloc(kvm_vcpu_cache, GFP_KERNEL);
	if (!vcpu) {
		err = -ENOMEM;
		goto out;
	}

	err = kvm_vcpu_init(vcpu, kvm, id);
	if (err)
		goto free_vcpu;

	err = create_hyp_mappings(vcpu, vcpu + 1);
	if (err)
		goto vcpu_uninit;

	return vcpu;
vcpu_uninit:
	kvm_vcpu_uninit(vcpu);
free_vcpu:
	kmem_cache_free(kvm_vcpu_cache, vcpu);
out:
	return ERR_PTR(err);
}

void kvm_arch_vcpu_postcreate(struct kvm_vcpu *vcpu)
{
}

void kvm_arch_vcpu_free(struct kvm_vcpu *vcpu)
{
	kvm_mmu_free_memory_caches(vcpu);
	kvm_timer_vcpu_terminate(vcpu);
	kvm_vgic_vcpu_destroy(vcpu);
	kvm_vcpu_uninit(vcpu);
	kmem_cache_free(kvm_vcpu_cache, vcpu);
}

void kvm_arch_vcpu_destroy(struct kvm_vcpu *vcpu)
{
	kvm_arch_vcpu_free(vcpu);
}

int kvm_cpu_has_pending_timer(struct kvm_vcpu *vcpu)
{
	return kvm_timer_should_fire(vcpu);
}

int kvm_arch_vcpu_init(struct kvm_vcpu *vcpu)
{
	/* Force users to call KVM_ARM_VCPU_INIT */
	vcpu->arch.target = -1;
	bitmap_zero(vcpu->arch.features, KVM_VCPU_MAX_FEATURES);

	/* Set up the timer */
	kvm_timer_vcpu_init(vcpu);

	return 0;
}

void kvm_arch_vcpu_load(struct kvm_vcpu *vcpu, int cpu)
{
	vcpu->cpu = cpu;
	vcpu->arch.host_cpu_context = this_cpu_ptr(kvm_host_cpu_state);

	kvm_arm_set_running_vcpu(vcpu);
}

void kvm_arch_vcpu_put(struct kvm_vcpu *vcpu)
{
	/*
	 * The arch-generic KVM code expects the cpu field of a vcpu to be -1
	 * if the vcpu is no longer assigned to a cpu.  This is used for the
	 * optimized make_all_cpus_request path.
	 */
	vcpu->cpu = -1;

	kvm_arm_set_running_vcpu(NULL);
}

int kvm_arch_vcpu_ioctl_set_guest_debug(struct kvm_vcpu *vcpu,
					struct kvm_guest_debug *dbg)
{
	return -EINVAL;
}


int kvm_arch_vcpu_ioctl_get_mpstate(struct kvm_vcpu *vcpu,
				    struct kvm_mp_state *mp_state)
{
	if (vcpu->arch.pause)
		mp_state->mp_state = KVM_MP_STATE_STOPPED;
	else
		mp_state->mp_state = KVM_MP_STATE_RUNNABLE;

	return 0;
}

int kvm_arch_vcpu_ioctl_set_mpstate(struct kvm_vcpu *vcpu,
				    struct kvm_mp_state *mp_state)
{
	switch (mp_state->mp_state) {
	case KVM_MP_STATE_RUNNABLE:
		vcpu->arch.pause = false;
		break;
	case KVM_MP_STATE_STOPPED:
		vcpu->arch.pause = true;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/**
 * kvm_arch_vcpu_runnable - determine if the vcpu can be scheduled
 * @v:		The VCPU pointer
 *
 * If the guest CPU is not waiting for interrupts or an interrupt line is
 * asserted, the CPU is by definition runnable.
 */
int kvm_arch_vcpu_runnable(struct kvm_vcpu *v)
{
	return !!v->arch.irq_lines || kvm_vgic_vcpu_pending_irq(v);
}

/* Just ensure a guest exit from a particular CPU */
static void exit_vm_noop(void *info)
{
}

void force_vm_exit(const cpumask_t *mask)
{
	smp_call_function_many(mask, exit_vm_noop, NULL, true);
}

/**
 * need_new_vmid_gen - check that the VMID is still valid
 * @kvm: The VM's VMID to checkt
 *
 * return true if there is a new generation of VMIDs being used
 *
 * The hardware supports only 256 values with the value zero reserved for the
 * host, so we check if an assigned value belongs to a previous generation,
 * which which requires us to assign a new value. If we're the first to use a
 * VMID for the new generation, we must flush necessary caches and TLBs on all
 * CPUs.
 */
static bool need_new_vmid_gen(struct kvm *kvm)
{
	return unlikely(kvm->arch.vmid_gen != atomic64_read(&kvm_vmid_gen));
}

/**
 * update_vttbr - Update the VTTBR with a valid VMID before the guest runs
 * @kvm	The guest that we are about to run
 *
 * Called from kvm_arch_vcpu_ioctl_run before entering the guest to ensure the
 * VM has a valid VMID, otherwise assigns a new one and flushes corresponding
 * caches and TLBs.
 */
static void update_vttbr(struct kvm *kvm)
{
	phys_addr_t pgd_phys;
	u64 vmid;

	if (!need_new_vmid_gen(kvm))
		return;

	spin_lock(&kvm_vmid_lock);

	/*
	 * We need to re-check the vmid_gen here to ensure that if another vcpu
	 * already allocated a valid vmid for this vm, then this vcpu should
	 * use the same vmid.
	 */
	if (!need_new_vmid_gen(kvm)) {
		spin_unlock(&kvm_vmid_lock);
		return;
	}

	/* First user of a new VMID generation? */
	if (unlikely(kvm_next_vmid == 0)) {
		atomic64_inc(&kvm_vmid_gen);
		kvm_next_vmid = 1;

		/*
		 * On SMP we know no other CPUs can use this CPU's or each
		 * other's VMID after force_vm_exit returns since the
		 * kvm_vmid_lock blocks them from reentry to the guest.
		 */
		force_vm_exit(cpu_all_mask);
		/*
		 * Now broadcast TLB + ICACHE invalidation over the inner
		 * shareable domain to make sure all data structures are
		 * clean.
		 */
		kvm_call_hyp(__kvm_flush_vm_context);
	}

	kvm->arch.vmid_gen = atomic64_read(&kvm_vmid_gen);
	kvm->arch.vmid = kvm_next_vmid;
	kvm_next_vmid++;

	/* update vttbr to be used with the new vmid */
	pgd_phys = virt_to_phys(kvm_get_hwpgd(kvm));
	BUG_ON(pgd_phys & ~VTTBR_BADDR_MASK);
	vmid = ((u64)(kvm->arch.vmid) << VTTBR_VMID_SHIFT) & VTTBR_VMID_MASK;
	kvm->arch.vttbr = pgd_phys | vmid;

	spin_unlock(&kvm_vmid_lock);
}

static int kvm_vcpu_first_run_init(struct kvm_vcpu *vcpu)
{
	struct kvm *kvm = vcpu->kvm;
	int ret;

	if (likely(vcpu->arch.has_run_once))
		return 0;

	vcpu->arch.has_run_once = true;

	/*
	 * Map the VGIC hardware resources before running a vcpu the first
	 * time on this VM.
	 */
	if (unlikely(irqchip_in_kernel(kvm) && !vgic_ready(kvm))) {
		ret = kvm_vgic_map_resources(kvm);
		if (ret)
			return ret;
	}

	/*
	 * Enable the arch timers only if we have an in-kernel VGIC
	 * and it has been properly initialized, since we cannot handle
	 * interrupts from the virtual timer with a userspace gic.
	 */
	if (irqchip_in_kernel(kvm) && vgic_initialized(kvm))
		kvm_timer_enable(kvm);

	return 0;
}

bool kvm_arch_intc_initialized(struct kvm *kvm)
{
	return vgic_initialized(kvm);
}

static void vcpu_pause(struct kvm_vcpu *vcpu)
{
	wait_queue_head_t *wq = kvm_arch_vcpu_wq(vcpu);

	wait_event_interruptible(*wq, !vcpu->arch.pause);
}

static int kvm_vcpu_initialized(struct kvm_vcpu *vcpu)
{
	return vcpu->arch.target >= 0;
}

/**
 * kvm_arch_vcpu_ioctl_run - the main VCPU run function to execute guest code
 * @vcpu:	The VCPU pointer
 * @run:	The kvm_run structure pointer used for userspace state exchange
 *
 * This function is called through the VCPU_RUN ioctl called from user space. It
 * will execute VM code in a loop until the time slice for the process is used
 * or some emulation is needed from user space in which case the function will
 * return with return value 0 and with the kvm_run structure filled in with the
 * required data for the requested emulation.
 */
int kvm_arch_vcpu_ioctl_run(struct kvm_vcpu *vcpu, struct kvm_run *run)
{
	int ret;
	sigset_t sigsaved;

	if (unlikely(!kvm_vcpu_initialized(vcpu)))
		return -ENOEXEC;

	ret = kvm_vcpu_first_run_init(vcpu);
	if (ret)
		return ret;

	if (run->exit_reason == KVM_EXIT_MMIO) {
		ret = kvm_handle_mmio_return(vcpu, vcpu->run);
		if (ret)
			return ret;
	}

	if (vcpu->sigset_active)
		sigprocmask(SIG_SETMASK, &vcpu->sigset, &sigsaved);

	ret = 1;
	run->exit_reason = KVM_EXIT_UNKNOWN;
	while (ret > 0) {
		/*
		 * Check conditions before entering the guest
		 */
		cond_resched();

		update_vttbr(vcpu->kvm);

		if (vcpu->arch.pause)
			vcpu_pause(vcpu);

		kvm_vgic_flush_hwstate(vcpu);
		kvm_timer_flush_hwstate(vcpu);

		local_irq_disable();

		/*
		 * Re-check atomic conditions
		 */
		if (signal_pending(current)) {
			ret = -EINTR;
			run->exit_reason = KVM_EXIT_INTR;
		}

		if (ret <= 0 || need_new_vmid_gen(vcpu->kvm)) {
			local_irq_enable();
			kvm_timer_sync_hwstate(vcpu);
			kvm_vgic_sync_hwstate(vcpu);
			continue;
		}

		/**************************************************************
		 * Enter the guest
		 */
		trace_kvm_entry(*vcpu_pc(vcpu));
		kvm_guest_enter();
		vcpu->mode = IN_GUEST_MODE;

		ret = kvm_call_hyp(__kvm_vcpu_run, vcpu);

		vcpu->mode = OUTSIDE_GUEST_MODE;
		kvm_guest_exit();
		trace_kvm_exit(kvm_vcpu_trap_get_class(vcpu), *vcpu_pc(vcpu));
		/*
		 * We may have taken a host interrupt in HYP mode (ie
		 * while executing the guest). This interrupt is still
		 * pending, as we haven't serviced it yet!
		 *
		 * We're now back in SVC mode, with interrupts
		 * disabled.  Enabling the interrupts now will have
		 * the effect of taking the interrupt again, in SVC
		 * mode this time.
		 */
		local_irq_enable();

		/*
		 * Back from guest
		 *************************************************************/

		kvm_timer_sync_hwstate(vcpu);
		kvm_vgic_sync_hwstate(vcpu);

		ret = handle_exit(vcpu, run, ret);
	}

	if (vcpu->sigset_active)
		sigprocmask(SIG_SETMASK, &sigsaved, NULL);
	return ret;
}

static int vcpu_interrupt_line(struct kvm_vcpu *vcpu, int number, bool level)
{
	int bit_index;
	bool set;
	unsigned long *ptr;

	if (number == KVM_ARM_IRQ_CPU_IRQ)
		bit_index = __ffs(HCR_VI);
	else /* KVM_ARM_IRQ_CPU_FIQ */
		bit_index = __ffs(HCR_VF);

	ptr = (unsigned long *)&vcpu->arch.irq_lines;
	if (level)
		set = test_and_set_bit(bit_index, ptr);
	else
		set = test_and_clear_bit(bit_index, ptr);

	/*
	 * If we didn't change anything, no need to wake up or kick other CPUs
	 */
	if (set == level)
		return 0;

	/*
	 * The vcpu irq_lines field was updated, wake up sleeping VCPUs and
	 * trigger a world-switch round on the running physical CPU to set the
	 * virtual IRQ/FIQ fields in the HCR appropriately.
	 */
	kvm_vcpu_kick(vcpu);

	return 0;
}

int kvm_vm_ioctl_irq_line(struct kvm *kvm, struct kvm_irq_level *irq_level,
			  bool line_status)
{
	u32 irq = irq_level->irq;
	unsigned int irq_type, vcpu_idx, irq_num;
	int nrcpus = atomic_read(&kvm->online_vcpus);
	struct kvm_vcpu *vcpu = NULL;
	bool level = irq_level->level;

	irq_type = (irq >> KVM_ARM_IRQ_TYPE_SHIFT) & KVM_ARM_IRQ_TYPE_MASK;
	vcpu_idx = (irq >> KVM_ARM_IRQ_VCPU_SHIFT) & KVM_ARM_IRQ_VCPU_MASK;
	irq_num = (irq >> KVM_ARM_IRQ_NUM_SHIFT) & KVM_ARM_IRQ_NUM_MASK;

	trace_kvm_irq_line(irq_type, vcpu_idx, irq_num, irq_level->level);

	switch (irq_type) {
	case KVM_ARM_IRQ_TYPE_CPU:
		if (irqchip_in_kernel(kvm))
			return -ENXIO;

		if (vcpu_idx >= nrcpus)
			return -EINVAL;

		vcpu = kvm_get_vcpu(kvm, vcpu_idx);
		if (!vcpu)
			return -EINVAL;

		if (irq_num > KVM_ARM_IRQ_CPU_FIQ)
			return -EINVAL;

		return vcpu_interrupt_line(vcpu, irq_num, level);
	case KVM_ARM_IRQ_TYPE_PPI:
		if (!irqchip_in_kernel(kvm))
			return -ENXIO;

		if (vcpu_idx >= nrcpus)
			return -EINVAL;

		vcpu = kvm_get_vcpu(kvm, vcpu_idx);
		if (!vcpu)
			return -EINVAL;

		if (irq_num < VGIC_NR_SGIS || irq_num >= VGIC_NR_PRIVATE_IRQS)
			return -EINVAL;

		return kvm_vgic_inject_irq(kvm, vcpu->vcpu_id, irq_num, level);
	case KVM_ARM_IRQ_TYPE_SPI:
		if (!irqchip_in_kernel(kvm))
			return -ENXIO;

		if (irq_num < VGIC_NR_PRIVATE_IRQS)
			return -EINVAL;

		return kvm_vgic_inject_irq(kvm, 0, irq_num, level);
	}

	return -EINVAL;
}

static int kvm_vcpu_set_target(struct kvm_vcpu *vcpu,
			       const struct kvm_vcpu_init *init)
{
	unsigned int i;
	int phys_target = kvm_target_cpu();

	if (init->target != phys_target)
		return -EINVAL;

	/*
	 * Secondary and subsequent calls to KVM_ARM_VCPU_INIT must
	 * use the same target.
	 */
	if (vcpu->arch.target != -1 && vcpu->arch.target != init->target)
		return -EINVAL;

	/* -ENOENT for unknown features, -EINVAL for invalid combinations. */
	for (i = 0; i < sizeof(init->features) * 8; i++) {
		bool set = (init->features[i / 32] & (1 << (i % 32)));

		if (set && i >= KVM_VCPU_MAX_FEATURES)
			return -ENOENT;

		/*
		 * Secondary and subsequent calls to KVM_ARM_VCPU_INIT must
		 * use the same feature set.
		 */
		if (vcpu->arch.target != -1 && i < KVM_VCPU_MAX_FEATURES &&
		    test_bit(i, vcpu->arch.features) != set)
			return -EINVAL;

		if (set)
			set_bit(i, vcpu->arch.features);
	}

	vcpu->arch.target = phys_target;

	/* Now we know what it is, we can reset it. */
	return kvm_reset_vcpu(vcpu);
}


static int kvm_arch_vcpu_ioctl_vcpu_init(struct kvm_vcpu *vcpu,
					 struct kvm_vcpu_init *init)
{
	int ret;

	ret = kvm_vcpu_set_target(vcpu, init);
	if (ret)
		return ret;

	/*
	 * Ensure a rebooted VM will fault in RAM pages and detect if the
	 * guest MMU is turned off and flush the caches as needed.
	 */
	if (vcpu->arch.has_run_once)
		stage2_unmap_vm(vcpu->kvm);

	vcpu_reset_hcr(vcpu);

	/*
	 * Handle the "start in power-off" case by marking the VCPU as paused.
	 */
	if (test_bit(KVM_ARM_VCPU_POWER_OFF, vcpu->arch.features))
		vcpu->arch.pause = true;
	else
		vcpu->arch.pause = false;

	return 0;
}

long kvm_arch_vcpu_ioctl(struct file *filp,
			 unsigned int ioctl, unsigned long arg)
{
	struct kvm_vcpu *vcpu = filp->private_data;
	void __user *argp = (void __user *)arg;

	switch (ioctl) {
	case KVM_ARM_VCPU_INIT: {
		struct kvm_vcpu_init init;

		if (copy_from_user(&init, argp, sizeof(init)))
			return -EFAULT;

		return kvm_arch_vcpu_ioctl_vcpu_init(vcpu, &init);
	}
	case KVM_SET_ONE_REG:
	case KVM_GET_ONE_REG: {
		struct kvm_one_reg reg;

		if (unlikely(!kvm_vcpu_initialized(vcpu)))
			return -ENOEXEC;

		if (copy_from_user(&reg, argp, sizeof(reg)))
			return -EFAULT;
		if (ioctl == KVM_SET_ONE_REG)
			return kvm_arm_set_reg(vcpu, &reg);
		else
			return kvm_arm_get_reg(vcpu, &reg);
	}
	case KVM_GET_REG_LIST: {
		struct kvm_reg_list __user *user_list = argp;
		struct kvm_reg_list reg_list;
		unsigned n;

		if (unlikely(!kvm_vcpu_initialized(vcpu)))
			return -ENOEXEC;

		if (copy_from_user(&reg_list, user_list, sizeof(reg_list)))
			return -EFAULT;
		n = reg_list.n;
		reg_list.n = kvm_arm_num_regs(vcpu);
		if (copy_to_user(user_list, &reg_list, sizeof(reg_list)))
			return -EFAULT;
		if (n < reg_list.n)
			return -E2BIG;
		return kvm_arm_copy_reg_indices(vcpu, user_list->reg);
	}
	default:
		return -EINVAL;
	}
}

/**
 * kvm_vm_ioctl_get_dirty_log - get and clear the log of dirty pages in a slot
 * @kvm: kvm instance
 * @log: slot id and address to which we copy the log
 *
 * Steps 1-4 below provide general overview of dirty page logging. See
 * kvm_get_dirty_log_protect() function description for additional details.
 *
 * We call kvm_get_dirty_log_protect() to handle steps 1-3, upon return we
 * always flush the TLB (step 4) even if previous step failed  and the dirty
 * bitmap may be corrupt. Regardless of previous outcome the KVM logging API
 * does not preclude user space subsequent dirty log read. Flushing TLB ensures
 * writes will be marked dirty for next log read.
 *
 *   1. Take a snapshot of the bit and clear it if needed.
 *   2. Write protect the corresponding page.
 *   3. Copy the snapshot to the userspace.
 *   4. Flush TLB's if needed.
 */
int kvm_vm_ioctl_get_dirty_log(struct kvm *kvm, struct kvm_dirty_log *log)
{
	bool is_dirty = false;
	int r;

	mutex_lock(&kvm->slots_lock);

	r = kvm_get_dirty_log_protect(kvm, log, &is_dirty);

	if (is_dirty)
		kvm_flush_remote_tlbs(kvm);

	mutex_unlock(&kvm->slots_lock);
	return r;
}

static int kvm_vm_ioctl_set_device_addr(struct kvm *kvm,
					struct kvm_arm_device_addr *dev_addr)
{
	unsigned long dev_id, type;

	dev_id = (dev_addr->id & KVM_ARM_DEVICE_ID_MASK) >>
		KVM_ARM_DEVICE_ID_SHIFT;
	type = (dev_addr->id & KVM_ARM_DEVICE_TYPE_MASK) >>
		KVM_ARM_DEVICE_TYPE_SHIFT;

	switch (dev_id) {
	case KVM_ARM_DEVICE_VGIC_V2:
		return kvm_vgic_addr(kvm, type, &dev_addr->addr, true);
	default:
		return -ENODEV;
	}
}

long kvm_arch_vm_ioctl(struct file *filp,
		       unsigned int ioctl, unsigned long arg)
{
	struct kvm *kvm = filp->private_data;
	void __user *argp = (void __user *)arg;

	switch (ioctl) {
	case KVM_CREATE_IRQCHIP: {
		return kvm_vgic_create(kvm, KVM_DEV_TYPE_ARM_VGIC_V2);
	}
	case KVM_ARM_SET_DEVICE_ADDR: {
		struct kvm_arm_device_addr dev_addr;

		if (copy_from_user(&dev_addr, argp, sizeof(dev_addr)))
			return -EFAULT;
		return kvm_vm_ioctl_set_device_addr(kvm, &dev_addr);
	}
	case KVM_ARM_PREFERRED_TARGET: {
		int err;
		struct kvm_vcpu_init init;

		err = kvm_vcpu_preferred_target(&init);
		if (err)
			return err;

		if (copy_to_user(argp, &init, sizeof(init)))
			return -EFAULT;

		return 0;
	}
	default:
		return -EINVAL;
	}
}

static void cpu_init_hyp_mode(void *dummy)
{
	phys_addr_t boot_pgd_ptr;
	phys_addr_t pgd_ptr;
	unsigned long hyp_stack_ptr;
	unsigned long stack_page;
	unsigned long vector_ptr;

	/* Switch from the HYP stub to our own HYP init vector */
	__hyp_set_vectors(kvm_get_idmap_vector());

	boot_pgd_ptr = kvm_mmu_get_boot_httbr();
	pgd_ptr = kvm_mmu_get_httbr();
	stack_page = __this_cpu_read(kvm_arm_hyp_stack_page);
	hyp_stack_ptr = stack_page + PAGE_SIZE;
	vector_ptr = (unsigned long)__kvm_hyp_vector;

	__cpu_init_hyp_mode(boot_pgd_ptr, pgd_ptr, hyp_stack_ptr, vector_ptr);
}

static int hyp_init_cpu_notify(struct notifier_block *self,
			       unsigned long action, void *cpu)
{
	switch (action) {
	case CPU_STARTING:
	case CPU_STARTING_FROZEN:
		if (__hyp_get_vectors() == hyp_default_vectors)
			cpu_init_hyp_mode(NULL);
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block hyp_init_cpu_nb = {
	.notifier_call = hyp_init_cpu_notify,
};

#ifdef CONFIG_CPU_PM
static int hyp_init_cpu_pm_notifier(struct notifier_block *self,
				    unsigned long cmd,
				    void *v)
{
	if (cmd == CPU_PM_EXIT &&
	    __hyp_get_vectors() == hyp_default_vectors) {
		cpu_init_hyp_mode(NULL);
		return NOTIFY_OK;
	}

	return NOTIFY_DONE;
}

static struct notifier_block hyp_init_cpu_pm_nb = {
	.notifier_call = hyp_init_cpu_pm_notifier,
};

static void __init hyp_cpu_pm_init(void)
{
	cpu_pm_register_notifier(&hyp_init_cpu_pm_nb);
}
#else
static inline void hyp_cpu_pm_init(void)
{
}
#endif

/**
 * Inits Hyp-mode on all online CPUs
 */
static int init_hyp_mode(void)
{
	int cpu;
	int err = 0;

	/*
	 * Allocate Hyp PGD and setup Hyp identity mapping
	 */
	err = kvm_mmu_init();
	if (err)
		goto out_err;

	/*
	 * It is probably enough to obtain the default on one
	 * CPU. It's unlikely to be different on the others.
	 */
	hyp_default_vectors = __hyp_get_vectors();

	/*
	 * Allocate stack pages for Hypervisor-mode
	 */
	for_each_possible_cpu(cpu) {
		unsigned long stack_page;

		stack_page = __get_free_page(GFP_KERNEL);
		if (!stack_page) {
			err = -ENOMEM;
			goto out_free_stack_pages;
		}

		per_cpu(kvm_arm_hyp_stack_page, cpu) = stack_page;
	}

	/*
	 * Map the Hyp-code called directly from the host
	 */
	err = create_hyp_mappings(__kvm_hyp_code_start, __kvm_hyp_code_end);
	if (err) {
		kvm_err("Cannot map world-switch code\n");
		goto out_free_mappings;
	}

	/*
	 * Map the Hyp stack pages
	 */
	for_each_possible_cpu(cpu) {
		char *stack_page = (char *)per_cpu(kvm_arm_hyp_stack_page, cpu);
		err = create_hyp_mappings(stack_page, stack_page + PAGE_SIZE);

		if (err) {
			kvm_err("Cannot map hyp stack\n");
			goto out_free_mappings;
		}
	}

	/*
	 * Map the host CPU structures
	 */
	kvm_host_cpu_state = alloc_percpu(kvm_cpu_context_t);
	if (!kvm_host_cpu_state) {
		err = -ENOMEM;
		kvm_err("Cannot allocate host CPU state\n");
		goto out_free_mappings;
	}

	for_each_possible_cpu(cpu) {
		kvm_cpu_context_t *cpu_ctxt;

		cpu_ctxt = per_cpu_ptr(kvm_host_cpu_state, cpu);
		err = create_hyp_mappings(cpu_ctxt, cpu_ctxt + 1);

		if (err) {
			kvm_err("Cannot map host CPU state: %d\n", err);
			goto out_free_context;
		}
	}

	/*
	 * Execute the init code on each CPU.
	 */
	on_each_cpu(cpu_init_hyp_mode, NULL, 1);

	/*
	 * Init HYP view of VGIC
	 */
	err = kvm_vgic_hyp_init();
	if (err)
		goto out_free_context;

	/*
	 * Init HYP architected timer support
	 */
	err = kvm_timer_hyp_init();
	if (err)
		goto out_free_mappings;

#ifndef CONFIG_HOTPLUG_CPU
	free_boot_hyp_pgd();
#endif

	kvm_perf_init();

	kvm_info("Hyp mode initialized successfully\n");

	return 0;
out_free_context:
	free_percpu(kvm_host_cpu_state);
out_free_mappings:
	free_hyp_pgds();
out_free_stack_pages:
	for_each_possible_cpu(cpu)
		free_page(per_cpu(kvm_arm_hyp_stack_page, cpu));
out_err:
	kvm_err("error initializing Hyp mode: %d\n", err);
	return err;
}

static void check_kvm_target_cpu(void *ret)
{
	*(int *)ret = kvm_target_cpu();
}

struct kvm_vcpu *kvm_mpidr_to_vcpu(struct kvm *kvm, unsigned long mpidr)
{
	struct kvm_vcpu *vcpu;
	int i;

	mpidr &= MPIDR_HWID_BITMASK;
	kvm_for_each_vcpu(i, vcpu, kvm) {
		if (mpidr == kvm_vcpu_get_mpidr_aff(vcpu))
			return vcpu;
	}
	return NULL;
}

/**
 * Initialize Hyp-mode and memory mappings on all CPUs.
 */
int kvm_arch_init(void *opaque)
{
	int err;
	int ret, cpu;

	if (!is_hyp_mode_available()) {
		kvm_err("HYP mode not available\n");
		return -ENODEV;
	}

	for_each_online_cpu(cpu) {
		smp_call_function_single(cpu, check_kvm_target_cpu, &ret, 1);
		if (ret < 0) {
			kvm_err("Error, CPU %d not supported!\n", cpu);
			return -ENODEV;
		}
	}

	cpu_notifier_register_begin();

	err = init_hyp_mode();
	if (err)
		goto out_err;

	err = __register_cpu_notifier(&hyp_init_cpu_nb);
	if (err) {
		kvm_err("Cannot register HYP init CPU notifier (%d)\n", err);
		goto out_err;
	}

	cpu_notifier_register_done();

	hyp_cpu_pm_init();

	kvm_coproc_table_init();
	return 0;
out_err:
	cpu_notifier_register_done();
	return err;
}

/* NOP: Compiling as a module not supported */
void kvm_arch_exit(void)
{
	kvm_perf_teardown();
}

static int arm_init(void)
{
	int rc = kvm_init(NULL, sizeof(struct kvm_vcpu), 0, THIS_MODULE);
	return rc;
}

module_init(arm_init);
