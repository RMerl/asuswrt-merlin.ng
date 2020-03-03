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
 * Copyright IBM Corp. 2007
 *
 * Authors: Hollis Blanchard <hollisb@us.ibm.com>
 */

#ifndef __POWERPC_KVM_HOST_H__
#define __POWERPC_KVM_HOST_H__

#include <linux/mutex.h>
#include <linux/hrtimer.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/kvm_types.h>
#include <linux/threads.h>
#include <linux/spinlock.h>
#include <linux/kvm_para.h>
#include <linux/list.h>
#include <linux/atomic.h>
#include <asm/kvm_asm.h>
#include <asm/processor.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <asm/hvcall.h>

#define KVM_MAX_VCPUS		NR_CPUS
#define KVM_MAX_VCORES		NR_CPUS
#define KVM_USER_MEM_SLOTS 32
#define KVM_MEM_SLOTS_NUM KVM_USER_MEM_SLOTS

#ifdef CONFIG_KVM_MMIO
#define KVM_COALESCED_MMIO_PAGE_OFFSET 1
#endif

/* These values are internal and can be increased later */
#define KVM_NR_IRQCHIPS          1
#define KVM_IRQCHIP_NUM_PINS     256

#include <linux/mmu_notifier.h>

#define KVM_ARCH_WANT_MMU_NOTIFIER

extern int kvm_unmap_hva(struct kvm *kvm, unsigned long hva);
extern int kvm_unmap_hva_range(struct kvm *kvm,
			       unsigned long start, unsigned long end);
extern int kvm_age_hva(struct kvm *kvm, unsigned long start, unsigned long end);
extern int kvm_test_age_hva(struct kvm *kvm, unsigned long hva);
extern void kvm_set_spte_hva(struct kvm *kvm, unsigned long hva, pte_t pte);

static inline void kvm_arch_mmu_notifier_invalidate_page(struct kvm *kvm,
							 unsigned long address)
{
}

#define HPTEG_CACHE_NUM			(1 << 15)
#define HPTEG_HASH_BITS_PTE		13
#define HPTEG_HASH_BITS_PTE_LONG	12
#define HPTEG_HASH_BITS_VPTE		13
#define HPTEG_HASH_BITS_VPTE_LONG	5
#define HPTEG_HASH_BITS_VPTE_64K	11
#define HPTEG_HASH_NUM_PTE		(1 << HPTEG_HASH_BITS_PTE)
#define HPTEG_HASH_NUM_PTE_LONG		(1 << HPTEG_HASH_BITS_PTE_LONG)
#define HPTEG_HASH_NUM_VPTE		(1 << HPTEG_HASH_BITS_VPTE)
#define HPTEG_HASH_NUM_VPTE_LONG	(1 << HPTEG_HASH_BITS_VPTE_LONG)
#define HPTEG_HASH_NUM_VPTE_64K		(1 << HPTEG_HASH_BITS_VPTE_64K)

/* Physical Address Mask - allowed range of real mode RAM access */
#define KVM_PAM			0x0fffffffffffffffULL

struct lppaca;
struct slb_shadow;
struct dtl_entry;

struct kvmppc_vcpu_book3s;
struct kvmppc_book3s_shadow_vcpu;

struct kvm_vm_stat {
	u32 remote_tlb_flush;
};

struct kvm_vcpu_stat {
	u32 sum_exits;
	u32 mmio_exits;
	u32 signal_exits;
	u32 light_exits;
	/* Account for special types of light exits: */
	u32 itlb_real_miss_exits;
	u32 itlb_virt_miss_exits;
	u32 dtlb_real_miss_exits;
	u32 dtlb_virt_miss_exits;
	u32 syscall_exits;
	u32 isi_exits;
	u32 dsi_exits;
	u32 emulated_inst_exits;
	u32 dec_exits;
	u32 ext_intr_exits;
	u32 halt_successful_poll;
	u32 halt_wakeup;
	u32 dbell_exits;
	u32 gdbell_exits;
	u32 ld;
	u32 st;
#ifdef CONFIG_PPC_BOOK3S
	u32 pf_storage;
	u32 pf_instruc;
	u32 sp_storage;
	u32 sp_instruc;
	u32 queue_intr;
	u32 ld_slow;
	u32 st_slow;
#endif
};

enum kvm_exit_types {
	MMIO_EXITS,
	SIGNAL_EXITS,
	ITLB_REAL_MISS_EXITS,
	ITLB_VIRT_MISS_EXITS,
	DTLB_REAL_MISS_EXITS,
	DTLB_VIRT_MISS_EXITS,
	SYSCALL_EXITS,
	ISI_EXITS,
	DSI_EXITS,
	EMULATED_INST_EXITS,
	EMULATED_MTMSRWE_EXITS,
	EMULATED_WRTEE_EXITS,
	EMULATED_MTSPR_EXITS,
	EMULATED_MFSPR_EXITS,
	EMULATED_MTMSR_EXITS,
	EMULATED_MFMSR_EXITS,
	EMULATED_TLBSX_EXITS,
	EMULATED_TLBWE_EXITS,
	EMULATED_RFI_EXITS,
	EMULATED_RFCI_EXITS,
	EMULATED_RFDI_EXITS,
	DEC_EXITS,
	EXT_INTR_EXITS,
	HALT_WAKEUP,
	USR_PR_INST,
	FP_UNAVAIL,
	DEBUG_EXITS,
	TIMEINGUEST,
	DBELL_EXITS,
	GDBELL_EXITS,
	__NUMBER_OF_KVM_EXIT_TYPES
};

/* allow access to big endian 32bit upper/lower parts and 64bit var */
struct kvmppc_exit_timing {
	union {
		u64 tv64;
		struct {
			u32 tbu, tbl;
		} tv32;
	};
};

struct kvmppc_pginfo {
	unsigned long pfn;
	atomic_t refcnt;
};

struct kvmppc_spapr_tce_table {
	struct list_head list;
	struct kvm *kvm;
	u64 liobn;
	u32 window_size;
	struct page *pages[0];
};

/* XICS components, defined in book3s_xics.c */
struct kvmppc_xics;
struct kvmppc_icp;

/*
 * The reverse mapping array has one entry for each HPTE,
 * which stores the guest's view of the second word of the HPTE
 * (including the guest physical address of the mapping),
 * plus forward and backward pointers in a doubly-linked ring
 * of HPTEs that map the same host page.  The pointers in this
 * ring are 32-bit HPTE indexes, to save space.
 */
struct revmap_entry {
	unsigned long guest_rpte;
	unsigned int forw, back;
};

/*
 * We use the top bit of each memslot->arch.rmap entry as a lock bit,
 * and bit 32 as a present flag.  The bottom 32 bits are the
 * index in the guest HPT of a HPTE that points to the page.
 */
#define KVMPPC_RMAP_LOCK_BIT	63
#define KVMPPC_RMAP_RC_SHIFT	32
#define KVMPPC_RMAP_REFERENCED	(HPTE_R_R << KVMPPC_RMAP_RC_SHIFT)
#define KVMPPC_RMAP_CHANGED	(HPTE_R_C << KVMPPC_RMAP_RC_SHIFT)
#define KVMPPC_RMAP_PRESENT	0x100000000ul
#define KVMPPC_RMAP_INDEX	0xfffffffful

struct kvm_arch_memory_slot {
#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
	unsigned long *rmap;
#endif /* CONFIG_KVM_BOOK3S_HV_POSSIBLE */
};

struct kvm_arch {
	unsigned int lpid;
#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
	unsigned long hpt_virt;
	struct revmap_entry *revmap;
	unsigned int host_lpid;
	unsigned long host_lpcr;
	unsigned long sdr1;
	unsigned long host_sdr1;
	int tlbie_lock;
	unsigned long lpcr;
	unsigned long vrma_slb_v;
	int hpte_setup_done;
	u32 hpt_order;
	atomic_t vcpus_running;
	u32 online_vcores;
	unsigned long hpt_npte;
	unsigned long hpt_mask;
	atomic_t hpte_mod_interest;
	cpumask_t need_tlb_flush;
	int hpt_cma_alloc;
	struct dentry *debugfs_dir;
	struct dentry *htab_dentry;
#endif /* CONFIG_KVM_BOOK3S_HV_POSSIBLE */
#ifdef CONFIG_KVM_BOOK3S_PR_POSSIBLE
	struct mutex hpt_mutex;
#endif
#ifdef CONFIG_PPC_BOOK3S_64
	struct list_head spapr_tce_tables;
	struct list_head rtas_tokens;
	DECLARE_BITMAP(enabled_hcalls, MAX_HCALL_OPCODE/4 + 1);
#endif
#ifdef CONFIG_KVM_MPIC
	struct openpic *mpic;
#endif
#ifdef CONFIG_KVM_XICS
	struct kvmppc_xics *xics;
#endif
	struct kvmppc_ops *kvm_ops;
#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
	/* This array can grow quite large, keep it at the end */
	struct kvmppc_vcore *vcores[KVM_MAX_VCORES];
#endif
};

/*
 * Struct for a virtual core.
 * Note: entry_exit_map combines a bitmap of threads that have entered
 * in the bottom 8 bits and a bitmap of threads that have exited in the
 * next 8 bits.  This is so that we can atomically set the entry bit
 * iff the exit map is 0 without taking a lock.
 */
struct kvmppc_vcore {
	int n_runnable;
	int num_threads;
	int entry_exit_map;
	int napping_threads;
	int first_vcpuid;
	u16 pcpu;
	u16 last_cpu;
	u8 vcore_state;
	u8 in_guest;
	struct list_head runnable_threads;
	spinlock_t lock;
	wait_queue_head_t wq;
	spinlock_t stoltb_lock;	/* protects stolen_tb and preempt_tb */
	u64 stolen_tb;
	u64 preempt_tb;
	struct kvm_vcpu *runner;
	struct kvm *kvm;
	u64 tb_offset;		/* guest timebase - host timebase */
	ulong lpcr;
	u32 arch_compat;
	ulong pcr;
	ulong dpdes;		/* doorbell state (POWER8) */
	void *mpp_buffer; /* Micro Partition Prefetch buffer */
	bool mpp_buffer_is_valid;
	ulong conferring_threads;
};

#define VCORE_ENTRY_MAP(vc)	((vc)->entry_exit_map & 0xff)
#define VCORE_EXIT_MAP(vc)	((vc)->entry_exit_map >> 8)
#define VCORE_IS_EXITING(vc)	(VCORE_EXIT_MAP(vc) != 0)

/* Values for vcore_state */
#define VCORE_INACTIVE	0
#define VCORE_SLEEPING	1
#define VCORE_PREEMPT	2
#define VCORE_RUNNING	3
#define VCORE_EXITING	4

/*
 * Struct used to manage memory for a virtual processor area
 * registered by a PAPR guest.  There are three types of area
 * that a guest can register.
 */
struct kvmppc_vpa {
	unsigned long gpa;	/* Current guest phys addr */
	void *pinned_addr;	/* Address in kernel linear mapping */
	void *pinned_end;	/* End of region */
	unsigned long next_gpa;	/* Guest phys addr for update */
	unsigned long len;	/* Number of bytes required */
	u8 update_pending;	/* 1 => update pinned_addr from next_gpa */
	bool dirty;		/* true => area has been modified by kernel */
};

struct kvmppc_pte {
	ulong eaddr;
	u64 vpage;
	ulong raddr;
	bool may_read		: 1;
	bool may_write		: 1;
	bool may_execute	: 1;
	u8 page_size;		/* MMU_PAGE_xxx */
};

struct kvmppc_mmu {
	/* book3s_64 only */
	void (*slbmte)(struct kvm_vcpu *vcpu, u64 rb, u64 rs);
	u64  (*slbmfee)(struct kvm_vcpu *vcpu, u64 slb_nr);
	u64  (*slbmfev)(struct kvm_vcpu *vcpu, u64 slb_nr);
	void (*slbie)(struct kvm_vcpu *vcpu, u64 slb_nr);
	void (*slbia)(struct kvm_vcpu *vcpu);
	/* book3s */
	void (*mtsrin)(struct kvm_vcpu *vcpu, u32 srnum, ulong value);
	u32  (*mfsrin)(struct kvm_vcpu *vcpu, u32 srnum);
	int  (*xlate)(struct kvm_vcpu *vcpu, gva_t eaddr,
		      struct kvmppc_pte *pte, bool data, bool iswrite);
	void (*reset_msr)(struct kvm_vcpu *vcpu);
	void (*tlbie)(struct kvm_vcpu *vcpu, ulong addr, bool large);
	int  (*esid_to_vsid)(struct kvm_vcpu *vcpu, ulong esid, u64 *vsid);
	u64  (*ea_to_vp)(struct kvm_vcpu *vcpu, gva_t eaddr, bool data);
	bool (*is_dcbz32)(struct kvm_vcpu *vcpu);
};

struct kvmppc_slb {
	u64 esid;
	u64 vsid;
	u64 orige;
	u64 origv;
	bool valid	: 1;
	bool Ks		: 1;
	bool Kp		: 1;
	bool nx		: 1;
	bool large	: 1;	/* PTEs are 16MB */
	bool tb		: 1;	/* 1TB segment */
	bool class	: 1;
	u8 base_page_size;	/* MMU_PAGE_xxx */
};

/* Struct used to accumulate timing information in HV real mode code */
struct kvmhv_tb_accumulator {
	u64	seqcount;	/* used to synchronize access, also count * 2 */
	u64	tb_total;	/* total time in timebase ticks */
	u64	tb_min;		/* min time */
	u64	tb_max;		/* max time */
};

# ifdef CONFIG_PPC_FSL_BOOK3E
#define KVMPPC_BOOKE_IAC_NUM	2
#define KVMPPC_BOOKE_DAC_NUM	2
# else
#define KVMPPC_BOOKE_IAC_NUM	4
#define KVMPPC_BOOKE_DAC_NUM	2
# endif
#define KVMPPC_BOOKE_MAX_IAC	4
#define KVMPPC_BOOKE_MAX_DAC	2

/* KVMPPC_EPR_USER takes precedence over KVMPPC_EPR_KERNEL */
#define KVMPPC_EPR_NONE		0 /* EPR not supported */
#define KVMPPC_EPR_USER		1 /* exit to userspace to fill EPR */
#define KVMPPC_EPR_KERNEL	2 /* in-kernel irqchip */

#define KVMPPC_IRQ_DEFAULT	0
#define KVMPPC_IRQ_MPIC		1
#define KVMPPC_IRQ_XICS		2

struct openpic;

struct kvm_vcpu_arch {
	ulong host_stack;
	u32 host_pid;
#ifdef CONFIG_PPC_BOOK3S
	struct kvmppc_slb slb[64];
	int slb_max;		/* 1 + index of last valid entry in slb[] */
	int slb_nr;		/* total number of entries in SLB */
	struct kvmppc_mmu mmu;
	struct kvmppc_vcpu_book3s *book3s;
#endif
#ifdef CONFIG_PPC_BOOK3S_32
	struct kvmppc_book3s_shadow_vcpu *shadow_vcpu;
#endif

	ulong gpr[32];

	struct thread_fp_state fp;

#ifdef CONFIG_SPE
	ulong evr[32];
	ulong spefscr;
	ulong host_spefscr;
	u64 acc;
#endif
#ifdef CONFIG_ALTIVEC
	struct thread_vr_state vr;
#endif

#ifdef CONFIG_KVM_BOOKE_HV
	u32 host_mas4;
	u32 host_mas6;
	u32 shadow_epcr;
	u32 shadow_msrp;
	u32 eplc;
	u32 epsc;
	u32 oldpir;
#endif

#if defined(CONFIG_BOOKE)
#if defined(CONFIG_KVM_BOOKE_HV) || defined(CONFIG_64BIT)
	u32 epcr;
#endif
#endif

#ifdef CONFIG_PPC_BOOK3S
	/* For Gekko paired singles */
	u32 qpr[32];
#endif

	ulong pc;
	ulong ctr;
	ulong lr;
#ifdef CONFIG_PPC_BOOK3S
	ulong tar;
#endif

	ulong xer;
	u32 cr;

#ifdef CONFIG_PPC_BOOK3S
	ulong hflags;
	ulong guest_owned_ext;
	ulong purr;
	ulong spurr;
	ulong ic;
	ulong vtb;
	ulong dscr;
	ulong amr;
	ulong uamor;
	ulong iamr;
	u32 ctrl;
	u32 dabrx;
	ulong dabr;
	ulong dawr;
	ulong dawrx;
	ulong ciabr;
	ulong cfar;
	ulong ppr;
	ulong pspb;
	ulong fscr;
	ulong shadow_fscr;
	ulong ebbhr;
	ulong ebbrr;
	ulong bescr;
	ulong csigr;
	ulong tacr;
	ulong tcscr;
	ulong acop;
	ulong wort;
	ulong shadow_srr1;
#endif
	u32 vrsave; /* also USPRG0 */
	u32 mmucr;
	/* shadow_msr is unused for BookE HV */
	ulong shadow_msr;
	ulong csrr0;
	ulong csrr1;
	ulong dsrr0;
	ulong dsrr1;
	ulong mcsrr0;
	ulong mcsrr1;
	ulong mcsr;
	u32 dec;
#ifdef CONFIG_BOOKE
	u32 decar;
#endif
	/* Time base value when we entered the guest */
	u64 entry_tb;
	u64 entry_vtb;
	u64 entry_ic;
	u32 tcr;
	ulong tsr; /* we need to perform set/clr_bits() which requires ulong */
	u32 ivor[64];
	ulong ivpr;
	u32 pvr;

	u32 shadow_pid;
	u32 shadow_pid1;
	u32 pid;
	u32 swap_pid;

	u32 ccr0;
	u32 ccr1;
	u32 dbsr;

	u64 mmcr[5];
	u32 pmc[8];
	u32 spmc[2];
	u64 siar;
	u64 sdar;
	u64 sier;
#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	u64 tfhar;
	u64 texasr;
	u64 tfiar;

	u32 cr_tm;
	u64 xer_tm;
	u64 lr_tm;
	u64 ctr_tm;
	u64 amr_tm;
	u64 ppr_tm;
	u64 dscr_tm;
	u64 tar_tm;

	ulong gpr_tm[32];

	struct thread_fp_state fp_tm;

	struct thread_vr_state vr_tm;
	u32 vrsave_tm; /* also USPRG0 */

#endif

#ifdef CONFIG_KVM_EXIT_TIMING
	struct mutex exit_timing_lock;
	struct kvmppc_exit_timing timing_exit;
	struct kvmppc_exit_timing timing_last_enter;
	u32 last_exit_type;
	u32 timing_count_type[__NUMBER_OF_KVM_EXIT_TYPES];
	u64 timing_sum_duration[__NUMBER_OF_KVM_EXIT_TYPES];
	u64 timing_sum_quad_duration[__NUMBER_OF_KVM_EXIT_TYPES];
	u64 timing_min_duration[__NUMBER_OF_KVM_EXIT_TYPES];
	u64 timing_max_duration[__NUMBER_OF_KVM_EXIT_TYPES];
	u64 timing_last_exit;
	struct dentry *debugfs_exit_timing;
#endif

#ifdef CONFIG_PPC_BOOK3S
	ulong fault_dar;
	u32 fault_dsisr;
	unsigned long intr_msr;
#endif

#ifdef CONFIG_BOOKE
	ulong fault_dear;
	ulong fault_esr;
	ulong queued_dear;
	ulong queued_esr;
	spinlock_t wdt_lock;
	struct timer_list wdt_timer;
	u32 tlbcfg[4];
	u32 tlbps[4];
	u32 mmucfg;
	u32 eptcfg;
	u32 epr;
	u64 sprg9;
	u32 pwrmgtcr0;
	u32 crit_save;
	/* guest debug registers*/
	struct debug_reg dbg_reg;
#endif
	gpa_t paddr_accessed;
	gva_t vaddr_accessed;
	pgd_t *pgdir;

	u8 io_gpr; /* GPR used as IO source/target */
	u8 mmio_host_swabbed;
	u8 mmio_sign_extend;
	u8 osi_needed;
	u8 osi_enabled;
	u8 papr_enabled;
	u8 watchdog_enabled;
	u8 sane;
	u8 cpu_type;
	u8 hcall_needed;
	u8 epr_flags; /* KVMPPC_EPR_xxx */
	u8 epr_needed;

	u32 cpr0_cfgaddr; /* holds the last set cpr0_cfgaddr */

	struct hrtimer dec_timer;
	u64 dec_jiffies;
	u64 dec_expires;
	unsigned long pending_exceptions;
	u8 ceded;
	u8 prodded;
	u32 last_inst;

	wait_queue_head_t *wqp;
	struct kvmppc_vcore *vcore;
	int ret;
	int trap;
	int state;
	int ptid;
	bool timer_running;
	wait_queue_head_t cpu_run;

	struct kvm_vcpu_arch_shared *shared;
#if defined(CONFIG_PPC_BOOK3S_64) && defined(CONFIG_KVM_BOOK3S_PR_POSSIBLE)
	bool shared_big_endian;
#endif
	unsigned long magic_page_pa; /* phys addr to map the magic page to */
	unsigned long magic_page_ea; /* effect. addr to map the magic page to */
	bool disable_kernel_nx;

	int irq_type;		/* one of KVM_IRQ_* */
	int irq_cpu_id;
	struct openpic *mpic;	/* KVM_IRQ_MPIC */
#ifdef CONFIG_KVM_XICS
	struct kvmppc_icp *icp; /* XICS presentation controller */
#endif

#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
	struct kvm_vcpu_arch_shared shregs;

	unsigned long pgfault_addr;
	long pgfault_index;
	unsigned long pgfault_hpte[2];

	struct list_head run_list;
	struct task_struct *run_task;
	struct kvm_run *kvm_run;

	spinlock_t vpa_update_lock;
	struct kvmppc_vpa vpa;
	struct kvmppc_vpa dtl;
	struct dtl_entry *dtl_ptr;
	unsigned long dtl_index;
	u64 stolen_logged;
	struct kvmppc_vpa slb_shadow;

	spinlock_t tbacct_lock;
	u64 busy_stolen;
	u64 busy_preempt;

	u32 emul_inst;
#endif

#ifdef CONFIG_KVM_BOOK3S_HV_EXIT_TIMING
	struct kvmhv_tb_accumulator *cur_activity;	/* What we're timing */
	u64	cur_tb_start;			/* when it started */
	struct kvmhv_tb_accumulator rm_entry;	/* real-mode entry code */
	struct kvmhv_tb_accumulator rm_intr;	/* real-mode intr handling */
	struct kvmhv_tb_accumulator rm_exit;	/* real-mode exit code */
	struct kvmhv_tb_accumulator guest_time;	/* guest execution */
	struct kvmhv_tb_accumulator cede_time;	/* time napping inside guest */

	struct dentry *debugfs_dir;
	struct dentry *debugfs_timings;
#endif /* CONFIG_KVM_BOOK3S_HV_EXIT_TIMING */
};

#define VCPU_FPR(vcpu, i)	(vcpu)->arch.fp.fpr[i][TS_FPROFFSET]

/* Values for vcpu->arch.state */
#define KVMPPC_VCPU_NOTREADY		0
#define KVMPPC_VCPU_RUNNABLE		1
#define KVMPPC_VCPU_BUSY_IN_HOST	2

/* Values for vcpu->arch.io_gpr */
#define KVM_MMIO_REG_MASK	0x001f
#define KVM_MMIO_REG_EXT_MASK	0xffe0
#define KVM_MMIO_REG_GPR	0x0000
#define KVM_MMIO_REG_FPR	0x0020
#define KVM_MMIO_REG_QPR	0x0040
#define KVM_MMIO_REG_FQPR	0x0060

#define __KVM_HAVE_ARCH_WQP
#define __KVM_HAVE_CREATE_DEVICE

static inline void kvm_arch_hardware_disable(void) {}
static inline void kvm_arch_hardware_unsetup(void) {}
static inline void kvm_arch_sync_events(struct kvm *kvm) {}
static inline void kvm_arch_memslots_updated(struct kvm *kvm) {}
static inline void kvm_arch_flush_shadow_all(struct kvm *kvm) {}
static inline void kvm_arch_sched_in(struct kvm_vcpu *vcpu, int cpu) {}
static inline void kvm_arch_exit(void) {}

#endif /* __POWERPC_KVM_HOST_H__ */
