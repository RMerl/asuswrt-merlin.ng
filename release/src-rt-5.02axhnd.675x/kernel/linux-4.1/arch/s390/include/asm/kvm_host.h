/*
 * definition for kernel virtual machines on s390
 *
 * Copyright IBM Corp. 2008, 2009
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (version 2 only)
 * as published by the Free Software Foundation.
 *
 *    Author(s): Carsten Otte <cotte@de.ibm.com>
 */


#ifndef ASM_KVM_HOST_H
#define ASM_KVM_HOST_H

#include <linux/types.h>
#include <linux/hrtimer.h>
#include <linux/interrupt.h>
#include <linux/kvm_types.h>
#include <linux/kvm_host.h>
#include <linux/kvm.h>
#include <asm/debug.h>
#include <asm/cpu.h>
#include <asm/isc.h>

#define KVM_MAX_VCPUS 64
#define KVM_USER_MEM_SLOTS 32

/*
 * These seem to be used for allocating ->chip in the routing table,
 * which we don't use. 4096 is an out-of-thin-air value. If we need
 * to look at ->chip later on, we'll need to revisit this.
 */
#define KVM_NR_IRQCHIPS 1
#define KVM_IRQCHIP_NUM_PINS 4096

#define SIGP_CTRL_C		0x80
#define SIGP_CTRL_SCN_MASK	0x3f

struct sca_entry {
	__u8	reserved0;
	__u8	sigp_ctrl;
	__u16	reserved[3];
	__u64	sda;
	__u64	reserved2[2];
} __attribute__((packed));

union ipte_control {
	unsigned long val;
	struct {
		unsigned long k  : 1;
		unsigned long kh : 31;
		unsigned long kg : 32;
	};
};

struct sca_block {
	union ipte_control ipte_control;
	__u64	reserved[5];
	__u64	mcn;
	__u64	reserved2;
	struct sca_entry cpu[64];
} __attribute__((packed));

#define CPUSTAT_STOPPED    0x80000000
#define CPUSTAT_WAIT       0x10000000
#define CPUSTAT_ECALL_PEND 0x08000000
#define CPUSTAT_STOP_INT   0x04000000
#define CPUSTAT_IO_INT     0x02000000
#define CPUSTAT_EXT_INT    0x01000000
#define CPUSTAT_RUNNING    0x00800000
#define CPUSTAT_RETAINED   0x00400000
#define CPUSTAT_TIMING_SUB 0x00020000
#define CPUSTAT_SIE_SUB    0x00010000
#define CPUSTAT_RRF        0x00008000
#define CPUSTAT_SLSV       0x00004000
#define CPUSTAT_SLSR       0x00002000
#define CPUSTAT_ZARCH      0x00000800
#define CPUSTAT_MCDS       0x00000100
#define CPUSTAT_SM         0x00000080
#define CPUSTAT_IBS        0x00000040
#define CPUSTAT_G          0x00000008
#define CPUSTAT_GED        0x00000004
#define CPUSTAT_J          0x00000002
#define CPUSTAT_P          0x00000001

struct kvm_s390_sie_block {
	atomic_t cpuflags;		/* 0x0000 */
	__u32 : 1;			/* 0x0004 */
	__u32 prefix : 18;
	__u32 : 1;
	__u32 ibc : 12;
	__u8	reserved08[4];		/* 0x0008 */
#define PROG_IN_SIE (1<<0)
	__u32	prog0c;			/* 0x000c */
	__u8	reserved10[16];		/* 0x0010 */
#define PROG_BLOCK_SIE 0x00000001
	atomic_t prog20;		/* 0x0020 */
	__u8	reserved24[4];		/* 0x0024 */
	__u64	cputm;			/* 0x0028 */
	__u64	ckc;			/* 0x0030 */
	__u64	epoch;			/* 0x0038 */
	__u8	reserved40[4];		/* 0x0040 */
#define LCTL_CR0	0x8000
#define LCTL_CR6	0x0200
#define LCTL_CR9	0x0040
#define LCTL_CR10	0x0020
#define LCTL_CR11	0x0010
#define LCTL_CR14	0x0002
	__u16   lctl;			/* 0x0044 */
	__s16	icpua;			/* 0x0046 */
#define ICTL_PINT	0x20000000
#define ICTL_LPSW	0x00400000
#define ICTL_STCTL	0x00040000
#define ICTL_ISKE	0x00004000
#define ICTL_SSKE	0x00002000
#define ICTL_RRBE	0x00001000
#define ICTL_TPROT	0x00000200
	__u32	ictl;			/* 0x0048 */
	__u32	eca;			/* 0x004c */
#define ICPT_INST	0x04
#define ICPT_PROGI	0x08
#define ICPT_INSTPROGI	0x0C
#define ICPT_OPEREXC	0x2C
#define ICPT_PARTEXEC	0x38
#define ICPT_IOINST	0x40
	__u8	icptcode;		/* 0x0050 */
	__u8	icptstatus;		/* 0x0051 */
	__u16	ihcpu;			/* 0x0052 */
	__u8	reserved54[2];		/* 0x0054 */
	__u16	ipa;			/* 0x0056 */
	__u32	ipb;			/* 0x0058 */
	__u32	scaoh;			/* 0x005c */
	__u8	reserved60;		/* 0x0060 */
	__u8	ecb;			/* 0x0061 */
	__u8    ecb2;                   /* 0x0062 */
#define ECB3_AES 0x04
#define ECB3_DEA 0x08
	__u8    ecb3;			/* 0x0063 */
	__u32	scaol;			/* 0x0064 */
	__u8	reserved68[4];		/* 0x0068 */
	__u32	todpr;			/* 0x006c */
	__u8	reserved70[32];		/* 0x0070 */
	psw_t	gpsw;			/* 0x0090 */
	__u64	gg14;			/* 0x00a0 */
	__u64	gg15;			/* 0x00a8 */
	__u8	reservedb0[20];		/* 0x00b0 */
	__u16	extcpuaddr;		/* 0x00c4 */
	__u16	eic;			/* 0x00c6 */
	__u32	reservedc8;		/* 0x00c8 */
	__u16	pgmilc;			/* 0x00cc */
	__u16	iprcc;			/* 0x00ce */
	__u32	dxc;			/* 0x00d0 */
	__u16	mcn;			/* 0x00d4 */
	__u8	perc;			/* 0x00d6 */
	__u8	peratmid;		/* 0x00d7 */
	__u64	peraddr;		/* 0x00d8 */
	__u8	eai;			/* 0x00e0 */
	__u8	peraid;			/* 0x00e1 */
	__u8	oai;			/* 0x00e2 */
	__u8	armid;			/* 0x00e3 */
	__u8	reservede4[4];		/* 0x00e4 */
	__u64	tecmc;			/* 0x00e8 */
	__u8	reservedf0[12];		/* 0x00f0 */
#define CRYCB_FORMAT1 0x00000001
#define CRYCB_FORMAT2 0x00000003
	__u32	crycbd;			/* 0x00fc */
	__u64	gcr[16];		/* 0x0100 */
	__u64	gbea;			/* 0x0180 */
	__u8	reserved188[24];	/* 0x0188 */
	__u32	fac;			/* 0x01a0 */
	__u8	reserved1a4[20];	/* 0x01a4 */
	__u64	cbrlo;			/* 0x01b8 */
	__u8	reserved1c0[8];		/* 0x01c0 */
	__u32	ecd;			/* 0x01c8 */
	__u8	reserved1cc[18];	/* 0x01cc */
	__u64	pp;			/* 0x01de */
	__u8	reserved1e6[2];		/* 0x01e6 */
	__u64	itdba;			/* 0x01e8 */
	__u8	reserved1f0[16];	/* 0x01f0 */
} __attribute__((packed));

struct kvm_s390_itdb {
	__u8	data[256];
} __packed;

struct kvm_s390_vregs {
	__vector128 vrs[32];
	__u8	reserved200[512];	/* for future vector expansion */
} __packed;

struct sie_page {
	struct kvm_s390_sie_block sie_block;
	__u8 reserved200[1024];		/* 0x0200 */
	struct kvm_s390_itdb itdb;	/* 0x0600 */
	__u8 reserved700[1280];		/* 0x0700 */
	struct kvm_s390_vregs vregs;	/* 0x0c00 */
} __packed;

struct kvm_vcpu_stat {
	u32 exit_userspace;
	u32 exit_null;
	u32 exit_external_request;
	u32 exit_external_interrupt;
	u32 exit_stop_request;
	u32 exit_validity;
	u32 exit_instruction;
	u32 halt_successful_poll;
	u32 halt_wakeup;
	u32 instruction_lctl;
	u32 instruction_lctlg;
	u32 instruction_stctl;
	u32 instruction_stctg;
	u32 exit_program_interruption;
	u32 exit_instr_and_program;
	u32 deliver_external_call;
	u32 deliver_emergency_signal;
	u32 deliver_service_signal;
	u32 deliver_virtio_interrupt;
	u32 deliver_stop_signal;
	u32 deliver_prefix_signal;
	u32 deliver_restart_signal;
	u32 deliver_program_int;
	u32 deliver_io_int;
	u32 exit_wait_state;
	u32 instruction_pfmf;
	u32 instruction_stidp;
	u32 instruction_spx;
	u32 instruction_stpx;
	u32 instruction_stap;
	u32 instruction_storage_key;
	u32 instruction_ipte_interlock;
	u32 instruction_stsch;
	u32 instruction_chsc;
	u32 instruction_stsi;
	u32 instruction_stfl;
	u32 instruction_tprot;
	u32 instruction_essa;
	u32 instruction_sigp_sense;
	u32 instruction_sigp_sense_running;
	u32 instruction_sigp_external_call;
	u32 instruction_sigp_emergency;
	u32 instruction_sigp_cond_emergency;
	u32 instruction_sigp_start;
	u32 instruction_sigp_stop;
	u32 instruction_sigp_stop_store_status;
	u32 instruction_sigp_store_status;
	u32 instruction_sigp_store_adtl_status;
	u32 instruction_sigp_arch;
	u32 instruction_sigp_prefix;
	u32 instruction_sigp_restart;
	u32 instruction_sigp_init_cpu_reset;
	u32 instruction_sigp_cpu_reset;
	u32 instruction_sigp_unknown;
	u32 diagnose_10;
	u32 diagnose_44;
	u32 diagnose_9c;
};

#define PGM_OPERATION			0x01
#define PGM_PRIVILEGED_OP		0x02
#define PGM_EXECUTE			0x03
#define PGM_PROTECTION			0x04
#define PGM_ADDRESSING			0x05
#define PGM_SPECIFICATION		0x06
#define PGM_DATA			0x07
#define PGM_FIXED_POINT_OVERFLOW	0x08
#define PGM_FIXED_POINT_DIVIDE		0x09
#define PGM_DECIMAL_OVERFLOW		0x0a
#define PGM_DECIMAL_DIVIDE		0x0b
#define PGM_HFP_EXPONENT_OVERFLOW	0x0c
#define PGM_HFP_EXPONENT_UNDERFLOW	0x0d
#define PGM_HFP_SIGNIFICANCE		0x0e
#define PGM_HFP_DIVIDE			0x0f
#define PGM_SEGMENT_TRANSLATION		0x10
#define PGM_PAGE_TRANSLATION		0x11
#define PGM_TRANSLATION_SPEC		0x12
#define PGM_SPECIAL_OPERATION		0x13
#define PGM_OPERAND			0x15
#define PGM_TRACE_TABEL			0x16
#define PGM_VECTOR_PROCESSING		0x1b
#define PGM_SPACE_SWITCH		0x1c
#define PGM_HFP_SQUARE_ROOT		0x1d
#define PGM_PC_TRANSLATION_SPEC		0x1f
#define PGM_AFX_TRANSLATION		0x20
#define PGM_ASX_TRANSLATION		0x21
#define PGM_LX_TRANSLATION		0x22
#define PGM_EX_TRANSLATION		0x23
#define PGM_PRIMARY_AUTHORITY		0x24
#define PGM_SECONDARY_AUTHORITY		0x25
#define PGM_LFX_TRANSLATION		0x26
#define PGM_LSX_TRANSLATION		0x27
#define PGM_ALET_SPECIFICATION		0x28
#define PGM_ALEN_TRANSLATION		0x29
#define PGM_ALE_SEQUENCE		0x2a
#define PGM_ASTE_VALIDITY		0x2b
#define PGM_ASTE_SEQUENCE		0x2c
#define PGM_EXTENDED_AUTHORITY		0x2d
#define PGM_LSTE_SEQUENCE		0x2e
#define PGM_ASTE_INSTANCE		0x2f
#define PGM_STACK_FULL			0x30
#define PGM_STACK_EMPTY			0x31
#define PGM_STACK_SPECIFICATION		0x32
#define PGM_STACK_TYPE			0x33
#define PGM_STACK_OPERATION		0x34
#define PGM_ASCE_TYPE			0x38
#define PGM_REGION_FIRST_TRANS		0x39
#define PGM_REGION_SECOND_TRANS		0x3a
#define PGM_REGION_THIRD_TRANS		0x3b
#define PGM_MONITOR			0x40
#define PGM_PER				0x80
#define PGM_CRYPTO_OPERATION		0x119

/* irq types in order of priority */
enum irq_types {
	IRQ_PEND_MCHK_EX = 0,
	IRQ_PEND_SVC,
	IRQ_PEND_PROG,
	IRQ_PEND_MCHK_REP,
	IRQ_PEND_EXT_IRQ_KEY,
	IRQ_PEND_EXT_MALFUNC,
	IRQ_PEND_EXT_EMERGENCY,
	IRQ_PEND_EXT_EXTERNAL,
	IRQ_PEND_EXT_CLOCK_COMP,
	IRQ_PEND_EXT_CPU_TIMER,
	IRQ_PEND_EXT_TIMING,
	IRQ_PEND_EXT_SERVICE,
	IRQ_PEND_EXT_HOST,
	IRQ_PEND_PFAULT_INIT,
	IRQ_PEND_PFAULT_DONE,
	IRQ_PEND_VIRTIO,
	IRQ_PEND_IO_ISC_0,
	IRQ_PEND_IO_ISC_1,
	IRQ_PEND_IO_ISC_2,
	IRQ_PEND_IO_ISC_3,
	IRQ_PEND_IO_ISC_4,
	IRQ_PEND_IO_ISC_5,
	IRQ_PEND_IO_ISC_6,
	IRQ_PEND_IO_ISC_7,
	IRQ_PEND_SIGP_STOP,
	IRQ_PEND_RESTART,
	IRQ_PEND_SET_PREFIX,
	IRQ_PEND_COUNT
};

/* We have 2M for virtio device descriptor pages. Smallest amount of
 * memory per page is 24 bytes (1 queue), so (2048*1024) / 24 = 87381
 */
#define KVM_S390_MAX_VIRTIO_IRQS 87381

/*
 * Repressible (non-floating) machine check interrupts
 * subclass bits in MCIC
 */
#define MCHK_EXTD_BIT 58
#define MCHK_DEGR_BIT 56
#define MCHK_WARN_BIT 55
#define MCHK_REP_MASK ((1UL << MCHK_DEGR_BIT) | \
		       (1UL << MCHK_EXTD_BIT) | \
		       (1UL << MCHK_WARN_BIT))

/* Exigent machine check interrupts subclass bits in MCIC */
#define MCHK_SD_BIT 63
#define MCHK_PD_BIT 62
#define MCHK_EX_MASK ((1UL << MCHK_SD_BIT) | (1UL << MCHK_PD_BIT))

#define IRQ_PEND_EXT_MASK ((1UL << IRQ_PEND_EXT_IRQ_KEY)    | \
			   (1UL << IRQ_PEND_EXT_CLOCK_COMP) | \
			   (1UL << IRQ_PEND_EXT_CPU_TIMER)  | \
			   (1UL << IRQ_PEND_EXT_MALFUNC)    | \
			   (1UL << IRQ_PEND_EXT_EMERGENCY)  | \
			   (1UL << IRQ_PEND_EXT_EXTERNAL)   | \
			   (1UL << IRQ_PEND_EXT_TIMING)     | \
			   (1UL << IRQ_PEND_EXT_HOST)       | \
			   (1UL << IRQ_PEND_EXT_SERVICE)    | \
			   (1UL << IRQ_PEND_VIRTIO)         | \
			   (1UL << IRQ_PEND_PFAULT_INIT)    | \
			   (1UL << IRQ_PEND_PFAULT_DONE))

#define IRQ_PEND_IO_MASK ((1UL << IRQ_PEND_IO_ISC_0) | \
			  (1UL << IRQ_PEND_IO_ISC_1) | \
			  (1UL << IRQ_PEND_IO_ISC_2) | \
			  (1UL << IRQ_PEND_IO_ISC_3) | \
			  (1UL << IRQ_PEND_IO_ISC_4) | \
			  (1UL << IRQ_PEND_IO_ISC_5) | \
			  (1UL << IRQ_PEND_IO_ISC_6) | \
			  (1UL << IRQ_PEND_IO_ISC_7))

#define IRQ_PEND_MCHK_MASK ((1UL << IRQ_PEND_MCHK_REP) | \
			    (1UL << IRQ_PEND_MCHK_EX))

struct kvm_s390_interrupt_info {
	struct list_head list;
	u64	type;
	union {
		struct kvm_s390_io_info io;
		struct kvm_s390_ext_info ext;
		struct kvm_s390_pgm_info pgm;
		struct kvm_s390_emerg_info emerg;
		struct kvm_s390_extcall_info extcall;
		struct kvm_s390_prefix_info prefix;
		struct kvm_s390_stop_info stop;
		struct kvm_s390_mchk_info mchk;
	};
};

struct kvm_s390_irq_payload {
	struct kvm_s390_io_info io;
	struct kvm_s390_ext_info ext;
	struct kvm_s390_pgm_info pgm;
	struct kvm_s390_emerg_info emerg;
	struct kvm_s390_extcall_info extcall;
	struct kvm_s390_prefix_info prefix;
	struct kvm_s390_stop_info stop;
	struct kvm_s390_mchk_info mchk;
};

struct kvm_s390_local_interrupt {
	spinlock_t lock;
	struct kvm_s390_float_interrupt *float_int;
	wait_queue_head_t *wq;
	atomic_t *cpuflags;
	DECLARE_BITMAP(sigp_emerg_pending, KVM_MAX_VCPUS);
	struct kvm_s390_irq_payload irq;
	unsigned long pending_irqs;
};

#define FIRQ_LIST_IO_ISC_0 0
#define FIRQ_LIST_IO_ISC_1 1
#define FIRQ_LIST_IO_ISC_2 2
#define FIRQ_LIST_IO_ISC_3 3
#define FIRQ_LIST_IO_ISC_4 4
#define FIRQ_LIST_IO_ISC_5 5
#define FIRQ_LIST_IO_ISC_6 6
#define FIRQ_LIST_IO_ISC_7 7
#define FIRQ_LIST_PFAULT   8
#define FIRQ_LIST_VIRTIO   9
#define FIRQ_LIST_COUNT   10
#define FIRQ_CNTR_IO       0
#define FIRQ_CNTR_SERVICE  1
#define FIRQ_CNTR_VIRTIO   2
#define FIRQ_CNTR_PFAULT   3
#define FIRQ_MAX_COUNT     4

struct kvm_s390_float_interrupt {
	unsigned long pending_irqs;
	spinlock_t lock;
	struct list_head lists[FIRQ_LIST_COUNT];
	int counters[FIRQ_MAX_COUNT];
	struct kvm_s390_mchk_info mchk;
	struct kvm_s390_ext_info srv_signal;
	int next_rr_cpu;
	unsigned long idle_mask[BITS_TO_LONGS(KVM_MAX_VCPUS)];
};

struct kvm_hw_wp_info_arch {
	unsigned long addr;
	unsigned long phys_addr;
	int len;
	char *old_data;
};

struct kvm_hw_bp_info_arch {
	unsigned long addr;
	int len;
};

/*
 * Only the upper 16 bits of kvm_guest_debug->control are arch specific.
 * Further KVM_GUESTDBG flags which an be used from userspace can be found in
 * arch/s390/include/uapi/asm/kvm.h
 */
#define KVM_GUESTDBG_EXIT_PENDING 0x10000000

#define guestdbg_enabled(vcpu) \
		(vcpu->guest_debug & KVM_GUESTDBG_ENABLE)
#define guestdbg_sstep_enabled(vcpu) \
		(vcpu->guest_debug & KVM_GUESTDBG_SINGLESTEP)
#define guestdbg_hw_bp_enabled(vcpu) \
		(vcpu->guest_debug & KVM_GUESTDBG_USE_HW_BP)
#define guestdbg_exit_pending(vcpu) (guestdbg_enabled(vcpu) && \
		(vcpu->guest_debug & KVM_GUESTDBG_EXIT_PENDING))

struct kvm_guestdbg_info_arch {
	unsigned long cr0;
	unsigned long cr9;
	unsigned long cr10;
	unsigned long cr11;
	struct kvm_hw_bp_info_arch *hw_bp_info;
	struct kvm_hw_wp_info_arch *hw_wp_info;
	int nr_hw_bp;
	int nr_hw_wp;
	unsigned long last_bp;
};

struct kvm_vcpu_arch {
	struct kvm_s390_sie_block *sie_block;
	s390_fp_regs      host_fpregs;
	unsigned int      host_acrs[NUM_ACRS];
	s390_fp_regs      guest_fpregs;
	struct kvm_s390_vregs	*host_vregs;
	struct kvm_s390_local_interrupt local_int;
	struct hrtimer    ckc_timer;
	struct kvm_s390_pgm_info pgm;
	union  {
		struct cpuid	cpu_id;
		u64		stidp_data;
	};
	struct gmap *gmap;
	struct kvm_guestdbg_info_arch guestdbg;
	unsigned long pfault_token;
	unsigned long pfault_select;
	unsigned long pfault_compare;
};

struct kvm_vm_stat {
	u32 remote_tlb_flush;
};

struct kvm_arch_memory_slot {
};

struct s390_map_info {
	struct list_head list;
	__u64 guest_addr;
	__u64 addr;
	struct page *page;
};

struct s390_io_adapter {
	unsigned int id;
	int isc;
	bool maskable;
	bool masked;
	bool swap;
	struct rw_semaphore maps_lock;
	struct list_head maps;
	atomic_t nr_maps;
};

#define MAX_S390_IO_ADAPTERS ((MAX_ISC + 1) * 8)
#define MAX_S390_ADAPTER_MAPS 256

/* maximum size of facilities and facility mask is 2k bytes */
#define S390_ARCH_FAC_LIST_SIZE_BYTE (1<<11)
#define S390_ARCH_FAC_LIST_SIZE_U64 \
	(S390_ARCH_FAC_LIST_SIZE_BYTE / sizeof(u64))
#define S390_ARCH_FAC_MASK_SIZE_BYTE S390_ARCH_FAC_LIST_SIZE_BYTE
#define S390_ARCH_FAC_MASK_SIZE_U64 \
	(S390_ARCH_FAC_MASK_SIZE_BYTE / sizeof(u64))

struct kvm_s390_fac {
	/* facility list requested by guest */
	__u64 list[S390_ARCH_FAC_LIST_SIZE_U64];
	/* facility mask supported by kvm & hosting machine */
	__u64 mask[S390_ARCH_FAC_LIST_SIZE_U64];
};

struct kvm_s390_cpu_model {
	struct kvm_s390_fac *fac;
	struct cpuid cpu_id;
	unsigned short ibc;
};

struct kvm_s390_crypto {
	struct kvm_s390_crypto_cb *crycb;
	__u32 crycbd;
	__u8 aes_kw;
	__u8 dea_kw;
};

struct kvm_s390_crypto_cb {
	__u8    reserved00[72];                 /* 0x0000 */
	__u8    dea_wrapping_key_mask[24];      /* 0x0048 */
	__u8    aes_wrapping_key_mask[32];      /* 0x0060 */
	__u8    reserved80[128];                /* 0x0080 */
};

struct kvm_arch{
	struct sca_block *sca;
	debug_info_t *dbf;
	struct kvm_s390_float_interrupt float_int;
	struct kvm_device *flic;
	struct gmap *gmap;
	int css_support;
	int use_irqchip;
	int use_cmma;
	int user_cpu_state_ctrl;
	int user_sigp;
	int user_stsi;
	struct s390_io_adapter *adapters[MAX_S390_IO_ADAPTERS];
	wait_queue_head_t ipte_wq;
	int ipte_lock_count;
	struct mutex ipte_mutex;
	spinlock_t start_stop_lock;
	struct kvm_s390_cpu_model model;
	struct kvm_s390_crypto crypto;
	u64 epoch;
};

#define KVM_HVA_ERR_BAD		(-1UL)
#define KVM_HVA_ERR_RO_BAD	(-2UL)

static inline bool kvm_is_error_hva(unsigned long addr)
{
	return IS_ERR_VALUE(addr);
}

#define ASYNC_PF_PER_VCPU	64
struct kvm_arch_async_pf {
	unsigned long pfault_token;
};

bool kvm_arch_can_inject_async_page_present(struct kvm_vcpu *vcpu);

void kvm_arch_async_page_ready(struct kvm_vcpu *vcpu,
			       struct kvm_async_pf *work);

void kvm_arch_async_page_not_present(struct kvm_vcpu *vcpu,
				     struct kvm_async_pf *work);

void kvm_arch_async_page_present(struct kvm_vcpu *vcpu,
				 struct kvm_async_pf *work);

extern int sie64a(struct kvm_s390_sie_block *, u64 *);
extern char sie_exit;

static inline void kvm_arch_hardware_disable(void) {}
static inline void kvm_arch_check_processor_compat(void *rtn) {}
static inline void kvm_arch_exit(void) {}
static inline void kvm_arch_sync_events(struct kvm *kvm) {}
static inline void kvm_arch_vcpu_uninit(struct kvm_vcpu *vcpu) {}
static inline void kvm_arch_sched_in(struct kvm_vcpu *vcpu, int cpu) {}
static inline void kvm_arch_free_memslot(struct kvm *kvm,
		struct kvm_memory_slot *free, struct kvm_memory_slot *dont) {}
static inline void kvm_arch_memslots_updated(struct kvm *kvm) {}
static inline void kvm_arch_flush_shadow_all(struct kvm *kvm) {}
static inline void kvm_arch_flush_shadow_memslot(struct kvm *kvm,
		struct kvm_memory_slot *slot) {}

#endif
