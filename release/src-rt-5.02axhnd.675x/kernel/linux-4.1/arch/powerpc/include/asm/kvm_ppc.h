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
 * Copyright IBM Corp. 2008
 *
 * Authors: Hollis Blanchard <hollisb@us.ibm.com>
 */

#ifndef __POWERPC_KVM_PPC_H__
#define __POWERPC_KVM_PPC_H__

/* This file exists just so we can dereference kvm_vcpu, avoiding nested header
 * dependencies. */

#include <linux/mutex.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/kvm_types.h>
#include <linux/kvm_host.h>
#include <linux/bug.h>
#ifdef CONFIG_PPC_BOOK3S
#include <asm/kvm_book3s.h>
#else
#include <asm/kvm_booke.h>
#endif
#ifdef CONFIG_KVM_BOOK3S_64_HANDLER
#include <asm/paca.h>
#endif

/*
 * KVMPPC_INST_SW_BREAKPOINT is debug Instruction
 * for supporting software breakpoint.
 */
#define KVMPPC_INST_SW_BREAKPOINT	0x00dddd00

enum emulation_result {
	EMULATE_DONE,         /* no further processing */
	EMULATE_DO_MMIO,      /* kvm_run filled with MMIO request */
	EMULATE_FAIL,         /* can't emulate this instruction */
	EMULATE_AGAIN,        /* something went wrong. go again */
	EMULATE_EXIT_USER,    /* emulation requires exit to user-space */
};

enum instruction_type {
	INST_GENERIC,
	INST_SC,		/* system call */
};

enum xlate_instdata {
	XLATE_INST,		/* translate instruction address */
	XLATE_DATA		/* translate data address */
};

enum xlate_readwrite {
	XLATE_READ,		/* check for read permissions */
	XLATE_WRITE		/* check for write permissions */
};

extern int kvmppc_vcpu_run(struct kvm_run *kvm_run, struct kvm_vcpu *vcpu);
extern int __kvmppc_vcpu_run(struct kvm_run *kvm_run, struct kvm_vcpu *vcpu);
extern void kvmppc_handler_highmem(void);

extern void kvmppc_dump_vcpu(struct kvm_vcpu *vcpu);
extern int kvmppc_handle_load(struct kvm_run *run, struct kvm_vcpu *vcpu,
                              unsigned int rt, unsigned int bytes,
			      int is_default_endian);
extern int kvmppc_handle_loads(struct kvm_run *run, struct kvm_vcpu *vcpu,
                               unsigned int rt, unsigned int bytes,
			       int is_default_endian);
extern int kvmppc_handle_store(struct kvm_run *run, struct kvm_vcpu *vcpu,
			       u64 val, unsigned int bytes,
			       int is_default_endian);

extern int kvmppc_load_last_inst(struct kvm_vcpu *vcpu,
				 enum instruction_type type, u32 *inst);

extern int kvmppc_ld(struct kvm_vcpu *vcpu, ulong *eaddr, int size, void *ptr,
		     bool data);
extern int kvmppc_st(struct kvm_vcpu *vcpu, ulong *eaddr, int size, void *ptr,
		     bool data);
extern int kvmppc_emulate_instruction(struct kvm_run *run,
                                      struct kvm_vcpu *vcpu);
extern int kvmppc_emulate_loadstore(struct kvm_vcpu *vcpu);
extern int kvmppc_emulate_mmio(struct kvm_run *run, struct kvm_vcpu *vcpu);
extern void kvmppc_emulate_dec(struct kvm_vcpu *vcpu);
extern u32 kvmppc_get_dec(struct kvm_vcpu *vcpu, u64 tb);
extern void kvmppc_decrementer_func(struct kvm_vcpu *vcpu);
extern int kvmppc_sanity_check(struct kvm_vcpu *vcpu);
extern int kvmppc_subarch_vcpu_init(struct kvm_vcpu *vcpu);
extern void kvmppc_subarch_vcpu_uninit(struct kvm_vcpu *vcpu);

/* Core-specific hooks */

extern void kvmppc_mmu_map(struct kvm_vcpu *vcpu, u64 gvaddr, gpa_t gpaddr,
                           unsigned int gtlb_idx);
extern void kvmppc_mmu_priv_switch(struct kvm_vcpu *vcpu, int usermode);
extern void kvmppc_mmu_switch_pid(struct kvm_vcpu *vcpu, u32 pid);
extern void kvmppc_mmu_destroy(struct kvm_vcpu *vcpu);
extern int kvmppc_mmu_init(struct kvm_vcpu *vcpu);
extern int kvmppc_mmu_dtlb_index(struct kvm_vcpu *vcpu, gva_t eaddr);
extern int kvmppc_mmu_itlb_index(struct kvm_vcpu *vcpu, gva_t eaddr);
extern gpa_t kvmppc_mmu_xlate(struct kvm_vcpu *vcpu, unsigned int gtlb_index,
                              gva_t eaddr);
extern void kvmppc_mmu_dtlb_miss(struct kvm_vcpu *vcpu);
extern void kvmppc_mmu_itlb_miss(struct kvm_vcpu *vcpu);
extern int kvmppc_xlate(struct kvm_vcpu *vcpu, ulong eaddr,
			enum xlate_instdata xlid, enum xlate_readwrite xlrw,
			struct kvmppc_pte *pte);

extern struct kvm_vcpu *kvmppc_core_vcpu_create(struct kvm *kvm,
                                                unsigned int id);
extern void kvmppc_core_vcpu_free(struct kvm_vcpu *vcpu);
extern int kvmppc_core_vcpu_setup(struct kvm_vcpu *vcpu);
extern int kvmppc_core_check_processor_compat(void);
extern int kvmppc_core_vcpu_translate(struct kvm_vcpu *vcpu,
                                      struct kvm_translation *tr);

extern void kvmppc_core_vcpu_load(struct kvm_vcpu *vcpu, int cpu);
extern void kvmppc_core_vcpu_put(struct kvm_vcpu *vcpu);

extern int kvmppc_core_prepare_to_enter(struct kvm_vcpu *vcpu);
extern int kvmppc_core_pending_dec(struct kvm_vcpu *vcpu);
extern void kvmppc_core_queue_program(struct kvm_vcpu *vcpu, ulong flags);
extern void kvmppc_core_queue_dec(struct kvm_vcpu *vcpu);
extern void kvmppc_core_dequeue_dec(struct kvm_vcpu *vcpu);
extern void kvmppc_core_queue_external(struct kvm_vcpu *vcpu,
                                       struct kvm_interrupt *irq);
extern void kvmppc_core_dequeue_external(struct kvm_vcpu *vcpu);
extern void kvmppc_core_queue_dtlb_miss(struct kvm_vcpu *vcpu, ulong dear_flags,
					ulong esr_flags);
extern void kvmppc_core_queue_data_storage(struct kvm_vcpu *vcpu,
					   ulong dear_flags,
					   ulong esr_flags);
extern void kvmppc_core_queue_itlb_miss(struct kvm_vcpu *vcpu);
extern void kvmppc_core_queue_inst_storage(struct kvm_vcpu *vcpu,
					   ulong esr_flags);
extern void kvmppc_core_flush_tlb(struct kvm_vcpu *vcpu);
extern int kvmppc_core_check_requests(struct kvm_vcpu *vcpu);

extern int kvmppc_booke_init(void);
extern void kvmppc_booke_exit(void);

extern void kvmppc_core_destroy_mmu(struct kvm_vcpu *vcpu);
extern int kvmppc_kvm_pv(struct kvm_vcpu *vcpu);
extern void kvmppc_map_magic(struct kvm_vcpu *vcpu);

extern long kvmppc_alloc_hpt(struct kvm *kvm, u32 *htab_orderp);
extern long kvmppc_alloc_reset_hpt(struct kvm *kvm, u32 *htab_orderp);
extern void kvmppc_free_hpt(struct kvm *kvm);
extern long kvmppc_prepare_vrma(struct kvm *kvm,
				struct kvm_userspace_memory_region *mem);
extern void kvmppc_map_vrma(struct kvm_vcpu *vcpu,
			struct kvm_memory_slot *memslot, unsigned long porder);
extern int kvmppc_pseries_do_hcall(struct kvm_vcpu *vcpu);

extern long kvm_vm_ioctl_create_spapr_tce(struct kvm *kvm,
				struct kvm_create_spapr_tce *args);
extern long kvmppc_h_put_tce(struct kvm_vcpu *vcpu, unsigned long liobn,
			     unsigned long ioba, unsigned long tce);
extern long kvmppc_h_get_tce(struct kvm_vcpu *vcpu, unsigned long liobn,
			     unsigned long ioba);
extern struct page *kvm_alloc_hpt(unsigned long nr_pages);
extern void kvm_release_hpt(struct page *page, unsigned long nr_pages);
extern int kvmppc_core_init_vm(struct kvm *kvm);
extern void kvmppc_core_destroy_vm(struct kvm *kvm);
extern void kvmppc_core_free_memslot(struct kvm *kvm,
				     struct kvm_memory_slot *free,
				     struct kvm_memory_slot *dont);
extern int kvmppc_core_create_memslot(struct kvm *kvm,
				      struct kvm_memory_slot *slot,
				      unsigned long npages);
extern int kvmppc_core_prepare_memory_region(struct kvm *kvm,
				struct kvm_memory_slot *memslot,
				struct kvm_userspace_memory_region *mem);
extern void kvmppc_core_commit_memory_region(struct kvm *kvm,
				struct kvm_userspace_memory_region *mem,
				const struct kvm_memory_slot *old);
extern int kvm_vm_ioctl_get_smmu_info(struct kvm *kvm,
				      struct kvm_ppc_smmu_info *info);
extern void kvmppc_core_flush_memslot(struct kvm *kvm,
				      struct kvm_memory_slot *memslot);

extern int kvmppc_bookehv_init(void);
extern void kvmppc_bookehv_exit(void);

extern int kvmppc_prepare_to_enter(struct kvm_vcpu *vcpu);

extern int kvm_vm_ioctl_get_htab_fd(struct kvm *kvm, struct kvm_get_htab_fd *);

int kvm_vcpu_ioctl_interrupt(struct kvm_vcpu *vcpu, struct kvm_interrupt *irq);

extern int kvm_vm_ioctl_rtas_define_token(struct kvm *kvm, void __user *argp);
extern int kvmppc_rtas_hcall(struct kvm_vcpu *vcpu);
extern void kvmppc_rtas_tokens_free(struct kvm *kvm);
extern int kvmppc_xics_set_xive(struct kvm *kvm, u32 irq, u32 server,
				u32 priority);
extern int kvmppc_xics_get_xive(struct kvm *kvm, u32 irq, u32 *server,
				u32 *priority);
extern int kvmppc_xics_int_on(struct kvm *kvm, u32 irq);
extern int kvmppc_xics_int_off(struct kvm *kvm, u32 irq);

void kvmppc_core_dequeue_debug(struct kvm_vcpu *vcpu);
void kvmppc_core_queue_debug(struct kvm_vcpu *vcpu);

union kvmppc_one_reg {
	u32	wval;
	u64	dval;
	vector128 vval;
	u64	vsxval[2];
	struct {
		u64	addr;
		u64	length;
	}	vpaval;
};

struct kvmppc_ops {
	struct module *owner;
	int (*get_sregs)(struct kvm_vcpu *vcpu, struct kvm_sregs *sregs);
	int (*set_sregs)(struct kvm_vcpu *vcpu, struct kvm_sregs *sregs);
	int (*get_one_reg)(struct kvm_vcpu *vcpu, u64 id,
			   union kvmppc_one_reg *val);
	int (*set_one_reg)(struct kvm_vcpu *vcpu, u64 id,
			   union kvmppc_one_reg *val);
	void (*vcpu_load)(struct kvm_vcpu *vcpu, int cpu);
	void (*vcpu_put)(struct kvm_vcpu *vcpu);
	void (*set_msr)(struct kvm_vcpu *vcpu, u64 msr);
	int (*vcpu_run)(struct kvm_run *run, struct kvm_vcpu *vcpu);
	struct kvm_vcpu *(*vcpu_create)(struct kvm *kvm, unsigned int id);
	void (*vcpu_free)(struct kvm_vcpu *vcpu);
	int (*check_requests)(struct kvm_vcpu *vcpu);
	int (*get_dirty_log)(struct kvm *kvm, struct kvm_dirty_log *log);
	void (*flush_memslot)(struct kvm *kvm, struct kvm_memory_slot *memslot);
	int (*prepare_memory_region)(struct kvm *kvm,
				     struct kvm_memory_slot *memslot,
				     struct kvm_userspace_memory_region *mem);
	void (*commit_memory_region)(struct kvm *kvm,
				     struct kvm_userspace_memory_region *mem,
				     const struct kvm_memory_slot *old);
	int (*unmap_hva)(struct kvm *kvm, unsigned long hva);
	int (*unmap_hva_range)(struct kvm *kvm, unsigned long start,
			   unsigned long end);
	int (*age_hva)(struct kvm *kvm, unsigned long start, unsigned long end);
	int (*test_age_hva)(struct kvm *kvm, unsigned long hva);
	void (*set_spte_hva)(struct kvm *kvm, unsigned long hva, pte_t pte);
	void (*mmu_destroy)(struct kvm_vcpu *vcpu);
	void (*free_memslot)(struct kvm_memory_slot *free,
			     struct kvm_memory_slot *dont);
	int (*create_memslot)(struct kvm_memory_slot *slot,
			      unsigned long npages);
	int (*init_vm)(struct kvm *kvm);
	void (*destroy_vm)(struct kvm *kvm);
	int (*get_smmu_info)(struct kvm *kvm, struct kvm_ppc_smmu_info *info);
	int (*emulate_op)(struct kvm_run *run, struct kvm_vcpu *vcpu,
			  unsigned int inst, int *advance);
	int (*emulate_mtspr)(struct kvm_vcpu *vcpu, int sprn, ulong spr_val);
	int (*emulate_mfspr)(struct kvm_vcpu *vcpu, int sprn, ulong *spr_val);
	void (*fast_vcpu_kick)(struct kvm_vcpu *vcpu);
	long (*arch_vm_ioctl)(struct file *filp, unsigned int ioctl,
			      unsigned long arg);
	int (*hcall_implemented)(unsigned long hcall);
};

extern struct kvmppc_ops *kvmppc_hv_ops;
extern struct kvmppc_ops *kvmppc_pr_ops;

static inline int kvmppc_get_last_inst(struct kvm_vcpu *vcpu,
					enum instruction_type type, u32 *inst)
{
	int ret = EMULATE_DONE;
	u32 fetched_inst;

	/* Load the instruction manually if it failed to do so in the
	 * exit path */
	if (vcpu->arch.last_inst == KVM_INST_FETCH_FAILED)
		ret = kvmppc_load_last_inst(vcpu, type, &vcpu->arch.last_inst);

	/*  Write fetch_failed unswapped if the fetch failed */
	if (ret == EMULATE_DONE)
		fetched_inst = kvmppc_need_byteswap(vcpu) ?
				swab32(vcpu->arch.last_inst) :
				vcpu->arch.last_inst;
	else
		fetched_inst = vcpu->arch.last_inst;

	*inst = fetched_inst;
	return ret;
}

static inline bool is_kvmppc_hv_enabled(struct kvm *kvm)
{
	return kvm->arch.kvm_ops == kvmppc_hv_ops;
}

extern int kvmppc_hwrng_present(void);

/*
 * Cuts out inst bits with ordering according to spec.
 * That means the leftmost bit is zero. All given bits are included.
 */
static inline u32 kvmppc_get_field(u64 inst, int msb, int lsb)
{
	u32 r;
	u32 mask;

	BUG_ON(msb > lsb);

	mask = (1 << (lsb - msb + 1)) - 1;
	r = (inst >> (63 - lsb)) & mask;

	return r;
}

/*
 * Replaces inst bits with ordering according to spec.
 */
static inline u32 kvmppc_set_field(u64 inst, int msb, int lsb, int value)
{
	u32 r;
	u32 mask;

	BUG_ON(msb > lsb);

	mask = ((1 << (lsb - msb + 1)) - 1) << (63 - lsb);
	r = (inst & ~mask) | ((value << (63 - lsb)) & mask);

	return r;
}

#define one_reg_size(id)	\
	(1ul << (((id) & KVM_REG_SIZE_MASK) >> KVM_REG_SIZE_SHIFT))

#define get_reg_val(id, reg)	({		\
	union kvmppc_one_reg __u;		\
	switch (one_reg_size(id)) {		\
	case 4: __u.wval = (reg); break;	\
	case 8: __u.dval = (reg); break;	\
	default: BUG();				\
	}					\
	__u;					\
})


#define set_reg_val(id, val)	({		\
	u64 __v;				\
	switch (one_reg_size(id)) {		\
	case 4: __v = (val).wval; break;	\
	case 8: __v = (val).dval; break;	\
	default: BUG();				\
	}					\
	__v;					\
})

int kvmppc_core_get_sregs(struct kvm_vcpu *vcpu, struct kvm_sregs *sregs);
int kvmppc_core_set_sregs(struct kvm_vcpu *vcpu, struct kvm_sregs *sregs);

int kvmppc_get_sregs_ivor(struct kvm_vcpu *vcpu, struct kvm_sregs *sregs);
int kvmppc_set_sregs_ivor(struct kvm_vcpu *vcpu, struct kvm_sregs *sregs);

int kvm_vcpu_ioctl_get_one_reg(struct kvm_vcpu *vcpu, struct kvm_one_reg *reg);
int kvm_vcpu_ioctl_set_one_reg(struct kvm_vcpu *vcpu, struct kvm_one_reg *reg);
int kvmppc_get_one_reg(struct kvm_vcpu *vcpu, u64 id, union kvmppc_one_reg *);
int kvmppc_set_one_reg(struct kvm_vcpu *vcpu, u64 id, union kvmppc_one_reg *);

void kvmppc_set_pid(struct kvm_vcpu *vcpu, u32 pid);

struct openpic;

#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
extern void kvm_cma_reserve(void) __init;
static inline void kvmppc_set_xics_phys(int cpu, unsigned long addr)
{
	paca[cpu].kvm_hstate.xics_phys = addr;
}

static inline u32 kvmppc_get_xics_latch(void)
{
	u32 xirr;

	xirr = get_paca()->kvm_hstate.saved_xirr;
	get_paca()->kvm_hstate.saved_xirr = 0;
	return xirr;
}

static inline void kvmppc_set_host_ipi(int cpu, u8 host_ipi)
{
	paca[cpu].kvm_hstate.host_ipi = host_ipi;
}

static inline void kvmppc_fast_vcpu_kick(struct kvm_vcpu *vcpu)
{
	vcpu->kvm->arch.kvm_ops->fast_vcpu_kick(vcpu);
}

extern void kvm_hv_vm_activated(void);
extern void kvm_hv_vm_deactivated(void);
extern bool kvm_hv_mode_active(void);

#else
static inline void __init kvm_cma_reserve(void)
{}

static inline void kvmppc_set_xics_phys(int cpu, unsigned long addr)
{}

static inline u32 kvmppc_get_xics_latch(void)
{
	return 0;
}

static inline void kvmppc_set_host_ipi(int cpu, u8 host_ipi)
{}

static inline void kvmppc_fast_vcpu_kick(struct kvm_vcpu *vcpu)
{
	kvm_vcpu_kick(vcpu);
}

static inline bool kvm_hv_mode_active(void)		{ return false; }

#endif

#ifdef CONFIG_KVM_XICS
static inline int kvmppc_xics_enabled(struct kvm_vcpu *vcpu)
{
	return vcpu->arch.irq_type == KVMPPC_IRQ_XICS;
}
extern void kvmppc_xics_free_icp(struct kvm_vcpu *vcpu);
extern int kvmppc_xics_create_icp(struct kvm_vcpu *vcpu, unsigned long server);
extern int kvm_vm_ioctl_xics_irq(struct kvm *kvm, struct kvm_irq_level *args);
extern int kvmppc_xics_hcall(struct kvm_vcpu *vcpu, u32 cmd);
extern u64 kvmppc_xics_get_icp(struct kvm_vcpu *vcpu);
extern int kvmppc_xics_set_icp(struct kvm_vcpu *vcpu, u64 icpval);
extern int kvmppc_xics_connect_vcpu(struct kvm_device *dev,
			struct kvm_vcpu *vcpu, u32 cpu);
#else
static inline int kvmppc_xics_enabled(struct kvm_vcpu *vcpu)
	{ return 0; }
static inline void kvmppc_xics_free_icp(struct kvm_vcpu *vcpu) { }
static inline int kvmppc_xics_create_icp(struct kvm_vcpu *vcpu,
					 unsigned long server)
	{ return -EINVAL; }
static inline int kvm_vm_ioctl_xics_irq(struct kvm *kvm,
					struct kvm_irq_level *args)
	{ return -ENOTTY; }
static inline int kvmppc_xics_hcall(struct kvm_vcpu *vcpu, u32 cmd)
	{ return 0; }
#endif

static inline unsigned long kvmppc_get_epr(struct kvm_vcpu *vcpu)
{
#ifdef CONFIG_KVM_BOOKE_HV
	return mfspr(SPRN_GEPR);
#elif defined(CONFIG_BOOKE)
	return vcpu->arch.epr;
#else
	return 0;
#endif
}

static inline void kvmppc_set_epr(struct kvm_vcpu *vcpu, u32 epr)
{
#ifdef CONFIG_KVM_BOOKE_HV
	mtspr(SPRN_GEPR, epr);
#elif defined(CONFIG_BOOKE)
	vcpu->arch.epr = epr;
#endif
}

#ifdef CONFIG_KVM_MPIC

void kvmppc_mpic_set_epr(struct kvm_vcpu *vcpu);
int kvmppc_mpic_connect_vcpu(struct kvm_device *dev, struct kvm_vcpu *vcpu,
			     u32 cpu);
void kvmppc_mpic_disconnect_vcpu(struct openpic *opp, struct kvm_vcpu *vcpu);

#else

static inline void kvmppc_mpic_set_epr(struct kvm_vcpu *vcpu)
{
}

static inline int kvmppc_mpic_connect_vcpu(struct kvm_device *dev,
		struct kvm_vcpu *vcpu, u32 cpu)
{
	return -EINVAL;
}

static inline void kvmppc_mpic_disconnect_vcpu(struct openpic *opp,
		struct kvm_vcpu *vcpu)
{
}

#endif /* CONFIG_KVM_MPIC */

int kvm_vcpu_ioctl_config_tlb(struct kvm_vcpu *vcpu,
			      struct kvm_config_tlb *cfg);
int kvm_vcpu_ioctl_dirty_tlb(struct kvm_vcpu *vcpu,
			     struct kvm_dirty_tlb *cfg);

long kvmppc_alloc_lpid(void);
void kvmppc_claim_lpid(long lpid);
void kvmppc_free_lpid(long lpid);
void kvmppc_init_lpid(unsigned long nr_lpids);

static inline void kvmppc_mmu_flush_icache(pfn_t pfn)
{
	struct page *page;
	/*
	 * We can only access pages that the kernel maps
	 * as memory. Bail out for unmapped ones.
	 */
	if (!pfn_valid(pfn))
		return;

	/* Clear i-cache for new pages */
	page = pfn_to_page(pfn);
	if (!test_bit(PG_arch_1, &page->flags)) {
		flush_dcache_icache_page(page);
		set_bit(PG_arch_1, &page->flags);
	}
}

/*
 * Shared struct helpers. The shared struct can be little or big endian,
 * depending on the guest endianness. So expose helpers to all of them.
 */
static inline bool kvmppc_shared_big_endian(struct kvm_vcpu *vcpu)
{
#if defined(CONFIG_PPC_BOOK3S_64) && defined(CONFIG_KVM_BOOK3S_PR_POSSIBLE)
	/* Only Book3S_64 PR supports bi-endian for now */
	return vcpu->arch.shared_big_endian;
#elif defined(CONFIG_PPC_BOOK3S_64) && defined(__LITTLE_ENDIAN__)
	/* Book3s_64 HV on little endian is always little endian */
	return false;
#else
	return true;
#endif
}

#define SPRNG_WRAPPER_GET(reg, bookehv_spr)				\
static inline ulong kvmppc_get_##reg(struct kvm_vcpu *vcpu)		\
{									\
	return mfspr(bookehv_spr);					\
}									\

#define SPRNG_WRAPPER_SET(reg, bookehv_spr)				\
static inline void kvmppc_set_##reg(struct kvm_vcpu *vcpu, ulong val)	\
{									\
	mtspr(bookehv_spr, val);						\
}									\

#define SHARED_WRAPPER_GET(reg, size)					\
static inline u##size kvmppc_get_##reg(struct kvm_vcpu *vcpu)		\
{									\
	if (kvmppc_shared_big_endian(vcpu))				\
	       return be##size##_to_cpu(vcpu->arch.shared->reg);	\
	else								\
	       return le##size##_to_cpu(vcpu->arch.shared->reg);	\
}									\

#define SHARED_WRAPPER_SET(reg, size)					\
static inline void kvmppc_set_##reg(struct kvm_vcpu *vcpu, u##size val)	\
{									\
	if (kvmppc_shared_big_endian(vcpu))				\
	       vcpu->arch.shared->reg = cpu_to_be##size(val);		\
	else								\
	       vcpu->arch.shared->reg = cpu_to_le##size(val);		\
}									\

#define SHARED_WRAPPER(reg, size)					\
	SHARED_WRAPPER_GET(reg, size)					\
	SHARED_WRAPPER_SET(reg, size)					\

#define SPRNG_WRAPPER(reg, bookehv_spr)					\
	SPRNG_WRAPPER_GET(reg, bookehv_spr)				\
	SPRNG_WRAPPER_SET(reg, bookehv_spr)				\

#ifdef CONFIG_KVM_BOOKE_HV

#define SHARED_SPRNG_WRAPPER(reg, size, bookehv_spr)			\
	SPRNG_WRAPPER(reg, bookehv_spr)					\

#else

#define SHARED_SPRNG_WRAPPER(reg, size, bookehv_spr)			\
	SHARED_WRAPPER(reg, size)					\

#endif

SHARED_WRAPPER(critical, 64)
SHARED_SPRNG_WRAPPER(sprg0, 64, SPRN_GSPRG0)
SHARED_SPRNG_WRAPPER(sprg1, 64, SPRN_GSPRG1)
SHARED_SPRNG_WRAPPER(sprg2, 64, SPRN_GSPRG2)
SHARED_SPRNG_WRAPPER(sprg3, 64, SPRN_GSPRG3)
SHARED_SPRNG_WRAPPER(srr0, 64, SPRN_GSRR0)
SHARED_SPRNG_WRAPPER(srr1, 64, SPRN_GSRR1)
SHARED_SPRNG_WRAPPER(dar, 64, SPRN_GDEAR)
SHARED_SPRNG_WRAPPER(esr, 64, SPRN_GESR)
SHARED_WRAPPER_GET(msr, 64)
static inline void kvmppc_set_msr_fast(struct kvm_vcpu *vcpu, u64 val)
{
	if (kvmppc_shared_big_endian(vcpu))
	       vcpu->arch.shared->msr = cpu_to_be64(val);
	else
	       vcpu->arch.shared->msr = cpu_to_le64(val);
}
SHARED_WRAPPER(dsisr, 32)
SHARED_WRAPPER(int_pending, 32)
SHARED_WRAPPER(sprg4, 64)
SHARED_WRAPPER(sprg5, 64)
SHARED_WRAPPER(sprg6, 64)
SHARED_WRAPPER(sprg7, 64)

static inline u32 kvmppc_get_sr(struct kvm_vcpu *vcpu, int nr)
{
	if (kvmppc_shared_big_endian(vcpu))
	       return be32_to_cpu(vcpu->arch.shared->sr[nr]);
	else
	       return le32_to_cpu(vcpu->arch.shared->sr[nr]);
}

static inline void kvmppc_set_sr(struct kvm_vcpu *vcpu, int nr, u32 val)
{
	if (kvmppc_shared_big_endian(vcpu))
	       vcpu->arch.shared->sr[nr] = cpu_to_be32(val);
	else
	       vcpu->arch.shared->sr[nr] = cpu_to_le32(val);
}

/*
 * Please call after prepare_to_enter. This function puts the lazy ee and irq
 * disabled tracking state back to normal mode, without actually enabling
 * interrupts.
 */
static inline void kvmppc_fix_ee_before_entry(void)
{
	trace_hardirqs_on();

#ifdef CONFIG_PPC64
	/*
	 * To avoid races, the caller must have gone directly from having
	 * interrupts fully-enabled to hard-disabled.
	 */
	WARN_ON(local_paca->irq_happened != PACA_IRQ_HARD_DIS);

	/* Only need to enable IRQs by hard enabling them after this */
	local_paca->irq_happened = 0;
	local_paca->soft_enabled = 1;
#endif
}

static inline ulong kvmppc_get_ea_indexed(struct kvm_vcpu *vcpu, int ra, int rb)
{
	ulong ea;
	ulong msr_64bit = 0;

	ea = kvmppc_get_gpr(vcpu, rb);
	if (ra)
		ea += kvmppc_get_gpr(vcpu, ra);

#if defined(CONFIG_PPC_BOOK3E_64)
	msr_64bit = MSR_CM;
#elif defined(CONFIG_PPC_BOOK3S_64)
	msr_64bit = MSR_SF;
#endif

	if (!(kvmppc_get_msr(vcpu) & msr_64bit))
		ea = (uint32_t)ea;

	return ea;
}

extern void xics_wake_cpu(int cpu);

#endif /* __POWERPC_KVM_PPC_H__ */
