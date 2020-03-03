/*
 * Copyright (C) 2012,2013 - ARM Ltd
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * Derived from arch/arm/kvm/reset.c
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/errno.h>
#include <linux/kvm_host.h>
#include <linux/kvm.h>

#include <kvm/arm_arch_timer.h>

#include <asm/cputype.h>
#include <asm/ptrace.h>
#include <asm/kvm_arm.h>
#include <asm/kvm_coproc.h>

/*
 * ARMv8 Reset Values
 */
static const struct kvm_regs default_regs_reset = {
	.regs.pstate = (PSR_MODE_EL1h | PSR_A_BIT | PSR_I_BIT |
			PSR_F_BIT | PSR_D_BIT),
};

static const struct kvm_regs default_regs_reset32 = {
	.regs.pstate = (COMPAT_PSR_MODE_SVC | COMPAT_PSR_A_BIT |
			COMPAT_PSR_I_BIT | COMPAT_PSR_F_BIT),
};

static const struct kvm_irq_level default_vtimer_irq = {
	.irq	= 27,
	.level	= 1,
};

static bool cpu_has_32bit_el1(void)
{
	u64 pfr0;

	pfr0 = read_cpuid(ID_AA64PFR0_EL1);
	return !!(pfr0 & 0x20);
}

int kvm_arch_dev_ioctl_check_extension(long ext)
{
	int r;

	switch (ext) {
	case KVM_CAP_ARM_EL1_32BIT:
		r = cpu_has_32bit_el1();
		break;
	default:
		r = 0;
	}

	return r;
}

/**
 * kvm_reset_vcpu - sets core registers and sys_regs to reset value
 * @vcpu: The VCPU pointer
 *
 * This function finds the right table above and sets the registers on
 * the virtual CPU struct to their architectually defined reset
 * values.
 */
int kvm_reset_vcpu(struct kvm_vcpu *vcpu)
{
	const struct kvm_irq_level *cpu_vtimer_irq;
	const struct kvm_regs *cpu_reset;

	switch (vcpu->arch.target) {
	default:
		if (test_bit(KVM_ARM_VCPU_EL1_32BIT, vcpu->arch.features)) {
			if (!cpu_has_32bit_el1())
				return -EINVAL;
			cpu_reset = &default_regs_reset32;
		} else {
			cpu_reset = &default_regs_reset;
		}

		cpu_vtimer_irq = &default_vtimer_irq;
		break;
	}

	/* Reset core registers */
	memcpy(vcpu_gp_regs(vcpu), cpu_reset, sizeof(*cpu_reset));

	/* Reset system registers */
	kvm_reset_sys_regs(vcpu);

	/* Reset timer */
	kvm_timer_vcpu_reset(vcpu, cpu_vtimer_irq);

	return 0;
}
