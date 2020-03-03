/*
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
 *
 * Copyright SUSE Linux Products GmbH 2009
 *
 * Authors: Alexander Graf <agraf@suse.de>
 */

#ifndef __ASM_KVM_BOOK3S_ASM_H__
#define __ASM_KVM_BOOK3S_ASM_H__

/* XICS ICP register offsets */
#define XICS_XIRR		4
#define XICS_MFRR		0xc
#define XICS_IPI		2	/* interrupt source # for IPIs */

#ifdef __ASSEMBLY__

#ifdef CONFIG_KVM_BOOK3S_HANDLER

#include <asm/kvm_asm.h>

.macro DO_KVM intno
	.if (\intno == BOOK3S_INTERRUPT_SYSTEM_RESET) || \
	    (\intno == BOOK3S_INTERRUPT_MACHINE_CHECK) || \
	    (\intno == BOOK3S_INTERRUPT_DATA_STORAGE) || \
	    (\intno == BOOK3S_INTERRUPT_INST_STORAGE) || \
	    (\intno == BOOK3S_INTERRUPT_DATA_SEGMENT) || \
	    (\intno == BOOK3S_INTERRUPT_INST_SEGMENT) || \
	    (\intno == BOOK3S_INTERRUPT_EXTERNAL) || \
	    (\intno == BOOK3S_INTERRUPT_EXTERNAL_HV) || \
	    (\intno == BOOK3S_INTERRUPT_ALIGNMENT) || \
	    (\intno == BOOK3S_INTERRUPT_PROGRAM) || \
	    (\intno == BOOK3S_INTERRUPT_FP_UNAVAIL) || \
	    (\intno == BOOK3S_INTERRUPT_DECREMENTER) || \
	    (\intno == BOOK3S_INTERRUPT_SYSCALL) || \
	    (\intno == BOOK3S_INTERRUPT_TRACE) || \
	    (\intno == BOOK3S_INTERRUPT_PERFMON) || \
	    (\intno == BOOK3S_INTERRUPT_ALTIVEC) || \
	    (\intno == BOOK3S_INTERRUPT_VSX)

	b	kvmppc_trampoline_\intno
kvmppc_resume_\intno:

	.endif
.endm

#else

.macro DO_KVM intno
.endm

#endif /* CONFIG_KVM_BOOK3S_HANDLER */

#else  /*__ASSEMBLY__ */

/*
 * This struct goes in the PACA on 64-bit processors.  It is used
 * to store host state that needs to be saved when we enter a guest
 * and restored when we exit, but isn't specific to any particular
 * guest or vcpu.  It also has some scratch fields used by the guest
 * exit code.
 */
struct kvmppc_host_state {
	ulong host_r1;
	ulong host_r2;
	ulong host_msr;
	ulong vmhandler;
	ulong scratch0;
	ulong scratch1;
	ulong scratch2;
	u8 in_guest;
	u8 restore_hid5;
	u8 napping;

#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
	u8 hwthread_req;
	u8 hwthread_state;
	u8 host_ipi;
	u8 ptid;
	struct kvm_vcpu *kvm_vcpu;
	struct kvmppc_vcore *kvm_vcore;
	unsigned long xics_phys;
	u32 saved_xirr;
	u64 dabr;
	u64 host_mmcr[7];	/* MMCR 0,1,A, SIAR, SDAR, MMCR2, SIER */
	u32 host_pmc[8];
	u64 host_purr;
	u64 host_spurr;
	u64 host_dscr;
	u64 dec_expires;
#endif
#ifdef CONFIG_PPC_BOOK3S_64
	u64 cfar;
	u64 ppr;
	u64 host_fscr;
#endif
};

struct kvmppc_book3s_shadow_vcpu {
	bool in_use;
	ulong gpr[14];
	u32 cr;
	u32 xer;
	ulong ctr;
	ulong lr;
	ulong pc;

	ulong shadow_srr1;
	ulong fault_dar;
	u32 fault_dsisr;
	u32 last_inst;

#ifdef CONFIG_PPC_BOOK3S_32
	u32     sr[16];			/* Guest SRs */

	struct kvmppc_host_state hstate;
#endif

#ifdef CONFIG_PPC_BOOK3S_64
	u8 slb_max;			/* highest used guest slb entry */
	struct  {
		u64     esid;
		u64     vsid;
	} slb[64];			/* guest SLB */
	u64 shadow_fscr;
#endif
};

#endif /*__ASSEMBLY__ */

/* Values for kvm_state */
#define KVM_HWTHREAD_IN_KERNEL	0
#define KVM_HWTHREAD_IN_NAP	1
#define KVM_HWTHREAD_IN_KVM	2

#endif /* __ASM_KVM_BOOK3S_ASM_H__ */
