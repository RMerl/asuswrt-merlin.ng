/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2015 Regents of the University of California
 *
 * Taken from Linux arch/riscv/include/asm/csr.h
 */

#ifndef _ASM_RISCV_CSR_H
#define _ASM_RISCV_CSR_H

#include <asm/asm.h>
#include <linux/const.h>

/* Status register flags */
#define SR_SIE		_AC(0x00000002, UL) /* Supervisor Interrupt Enable */
#define SR_SPIE		_AC(0x00000020, UL) /* Previous Supervisor IE */
#define SR_SPP		_AC(0x00000100, UL) /* Previously Supervisor */
#ifdef CONFIG_RISCV_PRIV_1_9
#define SR_PUM		_AC(0x00040000, UL) /* Protect User Memory Access */
#else
#define SR_SUM		_AC(0x00040000, UL) /* Supervisor User Memory Access */
#endif

#define SR_FS		_AC(0x00006000, UL) /* Floating-point Status */
#define SR_FS_OFF	_AC(0x00000000, UL)
#define SR_FS_INITIAL	_AC(0x00002000, UL)
#define SR_FS_CLEAN	_AC(0x00004000, UL)
#define SR_FS_DIRTY	_AC(0x00006000, UL)

#define SR_XS		_AC(0x00018000, UL) /* Extension Status */
#define SR_XS_OFF	_AC(0x00000000, UL)
#define SR_XS_INITIAL	_AC(0x00008000, UL)
#define SR_XS_CLEAN	_AC(0x00010000, UL)
#define SR_XS_DIRTY	_AC(0x00018000, UL)

#ifdef CONFIG_RISCV_PRIV_1_9
#define SR_VM		_AC(0x1F000000, UL) /* Virtualization Management */
#define SR_VM_MODE_BARE	_AC(0x00000000, UL) /* No translation or protection */
#define SR_VM_MODE_BB	_AC(0x01000000, UL) /* Single base-and-bound */
/* Separate instruction and data base-and-bound */
#define SR_VM_MODE_BBID	_AC(0x02000000, UL)
#ifndef CONFIG_64BIT
#define SR_VM_MODE_32	_AC(0x08000000, UL)
#define SR_VM_MODE	SR_VM_MODE_32
#else
#define SR_VM_MODE_39	_AC(0x09000000, UL)
#define SR_VM_MODE_48	_AC(0x0A000000, UL)
#define SR_VM_MODE	SR_VM_MODE_39
#endif
#endif

#ifndef CONFIG_64BIT
#define SR_SD		_AC(0x80000000, UL) /* FS/XS dirty */
#else
#define SR_SD		_AC(0x8000000000000000, UL) /* FS/XS dirty */
#endif

/* SATP flags */
#ifndef CONFIG_RISCV_PRIV_1_9
#ifndef CONFIG_64BIT
#define SATP_PPN	_AC(0x003FFFFF, UL)
#define SATP_MODE_32	_AC(0x80000000, UL)
#define SATP_MODE	SATP_MODE_32
#else
#define SATP_PPN	_AC(0x00000FFFFFFFFFFF, UL)
#define SATP_MODE_39	_AC(0x8000000000000000, UL)
#define SATP_MODE	SATP_MODE_39
#endif
#endif

/* SCAUSE */
#define SCAUSE_IRQ_FLAG		(_AC(1, UL) << (__riscv_xlen - 1))

#define IRQ_U_SOFT		0
#define IRQ_S_SOFT		1
#define IRQ_M_SOFT		3
#define IRQ_U_TIMER		4
#define IRQ_S_TIMER		5
#define IRQ_M_TIMER		7
#define IRQ_U_EXT		8
#define IRQ_S_EXT		9
#define IRQ_M_EXT		11

#define EXC_INST_MISALIGNED	0
#define EXC_INST_ACCESS		1
#define EXC_BREAKPOINT		3
#define EXC_LOAD_ACCESS		5
#define EXC_STORE_ACCESS	7
#define EXC_SYSCALL		8
#define EXC_INST_PAGE_FAULT	12
#define EXC_LOAD_PAGE_FAULT	13
#define EXC_STORE_PAGE_FAULT	15

/* SIE (Interrupt Enable) and SIP (Interrupt Pending) flags */
#define MIE_MSIE		(_AC(0x1, UL) << IRQ_M_SOFT)
#define SIE_SSIE		(_AC(0x1, UL) << IRQ_S_SOFT)
#define SIE_STIE		(_AC(0x1, UL) << IRQ_S_TIMER)
#define SIE_SEIE		(_AC(0x1, UL) << IRQ_S_EXT)

#define CSR_FCSR		0x003
#define CSR_CYCLE		0xc00
#define CSR_TIME		0xc01
#define CSR_INSTRET		0xc02
#define CSR_SSTATUS		0x100
#define CSR_SIE			0x104
#define CSR_STVEC		0x105
#define CSR_SCOUNTEREN		0x106
#define CSR_SSCRATCH		0x140
#define CSR_SEPC		0x141
#define CSR_SCAUSE		0x142
#define CSR_STVAL		0x143
#define CSR_SIP			0x144
#ifdef CONFIG_RISCV_PRIV_1_9
#define CSR_SPTBR		0x180
#else
#define CSR_SATP		0x180
#endif
#define CSR_MSTATUS		0x300
#define CSR_MISA		0x301
#define CSR_MIE			0x304
#define CSR_MTVEC		0x305
#ifdef CONFIG_RISCV_PRIV_1_9
#define CSR_MUCOUNTEREN         0x320
#define CSR_MSCOUNTEREN         0x321
#define CSR_MHCOUNTEREN         0x322
#else
#define CSR_MCOUNTEREN		0x306
#endif
#define CSR_MSCRATCH		0x340
#define CSR_MEPC		0x341
#define CSR_MCAUSE		0x342
#define CSR_MTVAL		0x343
#define CSR_MIP			0x344
#ifdef CONFIG_RISCV_PRIV_1_9
#define CSR_MBASE		0x380
#define CSR_MBOUND		0x381
#define CSR_MIBASE		0x382
#define CSR_MIBOUND		0x383
#define CSR_MDBASE		0x384
#define CSR_MDBOUND		0x385
#endif
#define CSR_CYCLEH		0xc80
#define CSR_TIMEH		0xc81
#define CSR_INSTRETH		0xc82
#define CSR_MARCHID		0xf12
#define CSR_MHARTID		0xf14

#ifndef __ASSEMBLY__

#define csr_swap(csr, val)					\
({								\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrrw %0, " __ASM_STR(csr) ", %1"\
			      : "=r" (__v) : "rK" (__v)		\
			      : "memory");			\
	__v;							\
})

#define csr_read(csr)						\
({								\
	register unsigned long __v;				\
	__asm__ __volatile__ ("csrr %0, " __ASM_STR(csr)	\
			      : "=r" (__v) :			\
			      : "memory");			\
	__v;							\
})

#define csr_write(csr, val)					\
({								\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrw " __ASM_STR(csr) ", %0"	\
			      : : "rK" (__v)			\
			      : "memory");			\
})

#define csr_read_set(csr, val)					\
({								\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrrs %0, " __ASM_STR(csr) ", %1"\
			      : "=r" (__v) : "rK" (__v)		\
			      : "memory");			\
	__v;							\
})

#define csr_set(csr, val)					\
({								\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrs " __ASM_STR(csr) ", %0"	\
			      : : "rK" (__v)			\
			      : "memory");			\
})

#define csr_read_clear(csr, val)				\
({								\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrrc %0, " __ASM_STR(csr) ", %1"\
			      : "=r" (__v) : "rK" (__v)		\
			      : "memory");			\
	__v;							\
})

#define csr_clear(csr, val)					\
({								\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrc " __ASM_STR(csr) ", %0"	\
			      : : "rK" (__v)			\
			      : "memory");			\
})

#endif /* __ASSEMBLY__ */

#endif /* _ASM_RISCV_CSR_H */
