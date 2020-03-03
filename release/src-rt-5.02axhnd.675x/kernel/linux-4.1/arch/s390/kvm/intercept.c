/*
 * in-kernel handling for sie intercepts
 *
 * Copyright IBM Corp. 2008, 2014
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (version 2 only)
 * as published by the Free Software Foundation.
 *
 *    Author(s): Carsten Otte <cotte@de.ibm.com>
 *               Christian Borntraeger <borntraeger@de.ibm.com>
 */

#include <linux/kvm_host.h>
#include <linux/errno.h>
#include <linux/pagemap.h>

#include <asm/kvm_host.h>
#include <asm/asm-offsets.h>
#include <asm/irq.h>

#include "kvm-s390.h"
#include "gaccess.h"
#include "trace.h"
#include "trace-s390.h"


static const intercept_handler_t instruction_handlers[256] = {
	[0x01] = kvm_s390_handle_01,
	[0x82] = kvm_s390_handle_lpsw,
	[0x83] = kvm_s390_handle_diag,
	[0xae] = kvm_s390_handle_sigp,
	[0xb2] = kvm_s390_handle_b2,
	[0xb6] = kvm_s390_handle_stctl,
	[0xb7] = kvm_s390_handle_lctl,
	[0xb9] = kvm_s390_handle_b9,
	[0xe5] = kvm_s390_handle_e5,
	[0xeb] = kvm_s390_handle_eb,
};

void kvm_s390_rewind_psw(struct kvm_vcpu *vcpu, int ilc)
{
	struct kvm_s390_sie_block *sie_block = vcpu->arch.sie_block;

	/* Use the length of the EXECUTE instruction if necessary */
	if (sie_block->icptstatus & 1) {
		ilc = (sie_block->icptstatus >> 4) & 0x6;
		if (!ilc)
			ilc = 4;
	}
	sie_block->gpsw.addr = __rewind_psw(sie_block->gpsw, ilc);
}

static int handle_noop(struct kvm_vcpu *vcpu)
{
	switch (vcpu->arch.sie_block->icptcode) {
	case 0x0:
		vcpu->stat.exit_null++;
		break;
	case 0x10:
		vcpu->stat.exit_external_request++;
		break;
	default:
		break; /* nothing */
	}
	return 0;
}

static int handle_stop(struct kvm_vcpu *vcpu)
{
	struct kvm_s390_local_interrupt *li = &vcpu->arch.local_int;
	int rc = 0;
	uint8_t flags, stop_pending;

	vcpu->stat.exit_stop_request++;

	/* delay the stop if any non-stop irq is pending */
	if (kvm_s390_vcpu_has_irq(vcpu, 1))
		return 0;

	/* avoid races with the injection/SIGP STOP code */
	spin_lock(&li->lock);
	flags = li->irq.stop.flags;
	stop_pending = kvm_s390_is_stop_irq_pending(vcpu);
	spin_unlock(&li->lock);

	trace_kvm_s390_stop_request(stop_pending, flags);
	if (!stop_pending)
		return 0;

	if (flags & KVM_S390_STOP_FLAG_STORE_STATUS) {
		rc = kvm_s390_vcpu_store_status(vcpu,
						KVM_S390_STORE_STATUS_NOADDR);
		if (rc)
			return rc;
	}

	if (!kvm_s390_user_cpu_state_ctrl(vcpu->kvm))
		kvm_s390_vcpu_stop(vcpu);
	return -EOPNOTSUPP;
}

static int handle_validity(struct kvm_vcpu *vcpu)
{
	int viwhy = vcpu->arch.sie_block->ipb >> 16;

	vcpu->stat.exit_validity++;
	trace_kvm_s390_intercept_validity(vcpu, viwhy);
	WARN_ONCE(true, "kvm: unhandled validity intercept 0x%x\n", viwhy);
	return -EOPNOTSUPP;
}

static int handle_instruction(struct kvm_vcpu *vcpu)
{
	intercept_handler_t handler;

	vcpu->stat.exit_instruction++;
	trace_kvm_s390_intercept_instruction(vcpu,
					     vcpu->arch.sie_block->ipa,
					     vcpu->arch.sie_block->ipb);
	handler = instruction_handlers[vcpu->arch.sie_block->ipa >> 8];
	if (handler)
		return handler(vcpu);
	return -EOPNOTSUPP;
}

static void __extract_prog_irq(struct kvm_vcpu *vcpu,
			       struct kvm_s390_pgm_info *pgm_info)
{
	memset(pgm_info, 0, sizeof(struct kvm_s390_pgm_info));
	pgm_info->code = vcpu->arch.sie_block->iprcc;

	switch (vcpu->arch.sie_block->iprcc & ~PGM_PER) {
	case PGM_AFX_TRANSLATION:
	case PGM_ASX_TRANSLATION:
	case PGM_EX_TRANSLATION:
	case PGM_LFX_TRANSLATION:
	case PGM_LSTE_SEQUENCE:
	case PGM_LSX_TRANSLATION:
	case PGM_LX_TRANSLATION:
	case PGM_PRIMARY_AUTHORITY:
	case PGM_SECONDARY_AUTHORITY:
	case PGM_SPACE_SWITCH:
		pgm_info->trans_exc_code = vcpu->arch.sie_block->tecmc;
		break;
	case PGM_ALEN_TRANSLATION:
	case PGM_ALE_SEQUENCE:
	case PGM_ASTE_INSTANCE:
	case PGM_ASTE_SEQUENCE:
	case PGM_ASTE_VALIDITY:
	case PGM_EXTENDED_AUTHORITY:
		pgm_info->exc_access_id = vcpu->arch.sie_block->eai;
		break;
	case PGM_ASCE_TYPE:
	case PGM_PAGE_TRANSLATION:
	case PGM_REGION_FIRST_TRANS:
	case PGM_REGION_SECOND_TRANS:
	case PGM_REGION_THIRD_TRANS:
	case PGM_SEGMENT_TRANSLATION:
		pgm_info->trans_exc_code = vcpu->arch.sie_block->tecmc;
		pgm_info->exc_access_id  = vcpu->arch.sie_block->eai;
		pgm_info->op_access_id  = vcpu->arch.sie_block->oai;
		break;
	case PGM_MONITOR:
		pgm_info->mon_class_nr = vcpu->arch.sie_block->mcn;
		pgm_info->mon_code = vcpu->arch.sie_block->tecmc;
		break;
	case PGM_VECTOR_PROCESSING:
	case PGM_DATA:
		pgm_info->data_exc_code = vcpu->arch.sie_block->dxc;
		break;
	case PGM_PROTECTION:
		pgm_info->trans_exc_code = vcpu->arch.sie_block->tecmc;
		pgm_info->exc_access_id  = vcpu->arch.sie_block->eai;
		break;
	default:
		break;
	}

	if (vcpu->arch.sie_block->iprcc & PGM_PER) {
		pgm_info->per_code = vcpu->arch.sie_block->perc;
		pgm_info->per_atmid = vcpu->arch.sie_block->peratmid;
		pgm_info->per_address = vcpu->arch.sie_block->peraddr;
		pgm_info->per_access_id = vcpu->arch.sie_block->peraid;
	}
}

/*
 * restore ITDB to program-interruption TDB in guest lowcore
 * and set TX abort indication if required
*/
static int handle_itdb(struct kvm_vcpu *vcpu)
{
	struct kvm_s390_itdb *itdb;
	int rc;

	if (!IS_TE_ENABLED(vcpu) || !IS_ITDB_VALID(vcpu))
		return 0;
	if (current->thread.per_flags & PER_FLAG_NO_TE)
		return 0;
	itdb = (struct kvm_s390_itdb *)vcpu->arch.sie_block->itdba;
	rc = write_guest_lc(vcpu, __LC_PGM_TDB, itdb, sizeof(*itdb));
	if (rc)
		return rc;
	memset(itdb, 0, sizeof(*itdb));

	return 0;
}

#define per_event(vcpu) (vcpu->arch.sie_block->iprcc & PGM_PER)

static int handle_prog(struct kvm_vcpu *vcpu)
{
	struct kvm_s390_pgm_info pgm_info;
	psw_t psw;
	int rc;

	vcpu->stat.exit_program_interruption++;

	if (guestdbg_enabled(vcpu) && per_event(vcpu)) {
		kvm_s390_handle_per_event(vcpu);
		/* the interrupt might have been filtered out completely */
		if (vcpu->arch.sie_block->iprcc == 0)
			return 0;
	}

	trace_kvm_s390_intercept_prog(vcpu, vcpu->arch.sie_block->iprcc);
	if (vcpu->arch.sie_block->iprcc == PGM_SPECIFICATION) {
		rc = read_guest_lc(vcpu, __LC_PGM_NEW_PSW, &psw, sizeof(psw_t));
		if (rc)
			return rc;
		/* Avoid endless loops of specification exceptions */
		if (!is_valid_psw(&psw))
			return -EOPNOTSUPP;
	}
	rc = handle_itdb(vcpu);
	if (rc)
		return rc;

	__extract_prog_irq(vcpu, &pgm_info);
	return kvm_s390_inject_prog_irq(vcpu, &pgm_info);
}

static int handle_instruction_and_prog(struct kvm_vcpu *vcpu)
{
	int rc, rc2;

	vcpu->stat.exit_instr_and_program++;
	rc = handle_instruction(vcpu);
	rc2 = handle_prog(vcpu);

	if (rc == -EOPNOTSUPP)
		vcpu->arch.sie_block->icptcode = 0x04;
	if (rc)
		return rc;
	return rc2;
}

/**
 * handle_external_interrupt - used for external interruption interceptions
 *
 * This interception only occurs if the CPUSTAT_EXT_INT bit was set, or if
 * the new PSW does not have external interrupts disabled. In the first case,
 * we've got to deliver the interrupt manually, and in the second case, we
 * drop to userspace to handle the situation there.
 */
static int handle_external_interrupt(struct kvm_vcpu *vcpu)
{
	u16 eic = vcpu->arch.sie_block->eic;
	struct kvm_s390_irq irq;
	psw_t newpsw;
	int rc;

	vcpu->stat.exit_external_interrupt++;

	rc = read_guest_lc(vcpu, __LC_EXT_NEW_PSW, &newpsw, sizeof(psw_t));
	if (rc)
		return rc;
	/* We can not handle clock comparator or timer interrupt with bad PSW */
	if ((eic == EXT_IRQ_CLK_COMP || eic == EXT_IRQ_CPU_TIMER) &&
	    (newpsw.mask & PSW_MASK_EXT))
		return -EOPNOTSUPP;

	switch (eic) {
	case EXT_IRQ_CLK_COMP:
		irq.type = KVM_S390_INT_CLOCK_COMP;
		break;
	case EXT_IRQ_CPU_TIMER:
		irq.type = KVM_S390_INT_CPU_TIMER;
		break;
	case EXT_IRQ_EXTERNAL_CALL:
		irq.type = KVM_S390_INT_EXTERNAL_CALL;
		irq.u.extcall.code = vcpu->arch.sie_block->extcpuaddr;
		rc = kvm_s390_inject_vcpu(vcpu, &irq);
		/* ignore if another external call is already pending */
		if (rc == -EBUSY)
			return 0;
		return rc;
	default:
		return -EOPNOTSUPP;
	}

	return kvm_s390_inject_vcpu(vcpu, &irq);
}

/**
 * Handle MOVE PAGE partial execution interception.
 *
 * This interception can only happen for guests with DAT disabled and
 * addresses that are currently not mapped in the host. Thus we try to
 * set up the mappings for the corresponding user pages here (or throw
 * addressing exceptions in case of illegal guest addresses).
 */
static int handle_mvpg_pei(struct kvm_vcpu *vcpu)
{
	unsigned long srcaddr, dstaddr;
	int reg1, reg2, rc;

	kvm_s390_get_regs_rre(vcpu, &reg1, &reg2);

	/* Make sure that the source is paged-in */
	rc = guest_translate_address(vcpu, vcpu->run->s.regs.gprs[reg2],
				     reg2, &srcaddr, 0);
	if (rc)
		return kvm_s390_inject_prog_cond(vcpu, rc);
	rc = kvm_arch_fault_in_page(vcpu, srcaddr, 0);
	if (rc != 0)
		return rc;

	/* Make sure that the destination is paged-in */
	rc = guest_translate_address(vcpu, vcpu->run->s.regs.gprs[reg1],
				     reg1, &dstaddr, 1);
	if (rc)
		return kvm_s390_inject_prog_cond(vcpu, rc);
	rc = kvm_arch_fault_in_page(vcpu, dstaddr, 1);
	if (rc != 0)
		return rc;

	kvm_s390_rewind_psw(vcpu, 4);

	return 0;
}

static int handle_partial_execution(struct kvm_vcpu *vcpu)
{
	if (vcpu->arch.sie_block->ipa == 0xb254)	/* MVPG */
		return handle_mvpg_pei(vcpu);
	if (vcpu->arch.sie_block->ipa >> 8 == 0xae)	/* SIGP */
		return kvm_s390_handle_sigp_pei(vcpu);

	return -EOPNOTSUPP;
}

static const intercept_handler_t intercept_funcs[] = {
	[0x00 >> 2] = handle_noop,
	[0x04 >> 2] = handle_instruction,
	[0x08 >> 2] = handle_prog,
	[0x0C >> 2] = handle_instruction_and_prog,
	[0x10 >> 2] = handle_noop,
	[0x14 >> 2] = handle_external_interrupt,
	[0x18 >> 2] = handle_noop,
	[0x1C >> 2] = kvm_s390_handle_wait,
	[0x20 >> 2] = handle_validity,
	[0x28 >> 2] = handle_stop,
	[0x38 >> 2] = handle_partial_execution,
};

int kvm_handle_sie_intercept(struct kvm_vcpu *vcpu)
{
	intercept_handler_t func;
	u8 code = vcpu->arch.sie_block->icptcode;

	if (code & 3 || (code >> 2) >= ARRAY_SIZE(intercept_funcs))
		return -EOPNOTSUPP;
	func = intercept_funcs[code >> 2];
	if (func)
		return func(vcpu);
	return -EOPNOTSUPP;
}
