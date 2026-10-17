/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2015 Regents of the University of California
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 *
 * Taken from Linux arch/riscv/include/asm/sbi.h
 */

#ifndef _ASM_RISCV_SBI_H
#define _ASM_RISCV_SBI_H

#include <linux/types.h>

enum sbi_ext_id {
	SBI_EXT_0_1_SET_TIMER = 0x0,
	SBI_EXT_0_1_CONSOLE_PUTCHAR = 0x1,
	SBI_EXT_0_1_CONSOLE_GETCHAR = 0x2,
	SBI_EXT_0_1_CLEAR_IPI = 0x3,
	SBI_EXT_0_1_SEND_IPI = 0x4,
	SBI_EXT_0_1_REMOTE_FENCE_I = 0x5,
	SBI_EXT_0_1_REMOTE_SFENCE_VMA = 0x6,
	SBI_EXT_0_1_REMOTE_SFENCE_VMA_ASID = 0x7,
	SBI_EXT_0_1_SHUTDOWN = 0x8,
	SBI_EXT_BASE = 0x10,
	SBI_EXT_TIME = 0x54494D45,
	SBI_EXT_IPI = 0x735049,
	SBI_EXT_RFENCE = 0x52464E43,
	SBI_EXT_HSM = 0x48534D,
	SBI_EXT_SRST = 0x53525354,
	SBI_EXT_PMU = 0x504D55,
	SBI_EXT_DBCN = 0x4442434E,
	SBI_EXT_SUSP = 0x53555350,
	SBI_EXT_CPPC = 0x43505043,
	SBI_EXT_NACL = 0x4E41434C,
	SBI_EXT_STA = 0x535441,
	SBI_EXT_DBTR = 0x44425452,
};

enum sbi_ext_base_fid {
	SBI_EXT_BASE_GET_SPEC_VERSION = 0,
	SBI_EXT_BASE_GET_IMP_ID,
	SBI_EXT_BASE_GET_IMP_VERSION,
	SBI_EXT_BASE_PROBE_EXT,
	SBI_EXT_BASE_GET_MVENDORID,
	SBI_EXT_BASE_GET_MARCHID,
	SBI_EXT_BASE_GET_MIMPID,
};

enum sbi_ext_time_fid {
	SBI_EXT_TIME_SET_TIMER = 0,
};

enum sbi_ext_ipi_fid {
	SBI_EXT_IPI_SEND_IPI = 0,
};

enum sbi_ext_rfence_fid {
	SBI_EXT_RFENCE_REMOTE_FENCE_I = 0,
	SBI_EXT_RFENCE_REMOTE_SFENCE_VMA,
	SBI_EXT_RFENCE_REMOTE_SFENCE_VMA_ASID,
	SBI_EXT_RFENCE_REMOTE_HFENCE_GVMA_VMID,
	SBI_EXT_RFENCE_REMOTE_HFENCE_GVMA,
	SBI_EXT_RFENCE_REMOTE_HFENCE_VVMA_ASID,
	SBI_EXT_RFENCE_REMOTE_HFENCE_VVMA,
};

enum sbi_ext_hsm_fid {
	SBI_EXT_HSM_HART_START = 0,
	SBI_EXT_HSM_HART_STOP,
	SBI_EXT_HSM_HART_STATUS,
	SBI_EXT_HSM_HART_SUSPEND,
};

enum sbi_hsm_hart_status {
	SBI_HSM_HART_STATUS_STARTED = 0,
	SBI_HSM_HART_STATUS_STOPPED,
	SBI_HSM_HART_STATUS_START_PENDING,
	SBI_HSM_HART_STATUS_STOP_PENDING,
	SBI_HSM_HART_STATUS_SUSPEND_PENDING,
	SBI_HSM_HART_STATUS_RESUME_PENDING,
};

enum sbi_ext_srst_fid {
	SBI_EXT_SRST_RESET = 0,
};

enum sbi_srst_reset_type {
	SBI_SRST_RESET_TYPE_SHUTDOWN = 0,
	SBI_SRST_RESET_TYPE_COLD_REBOOT,
	SBI_SRST_RESET_TYPE_WARM_REBOOT,
};

enum sbi_srst_reset_reason {
	SBI_SRST_RESET_REASON_NONE = 0,
	SBI_SRST_RESET_REASON_SYS_FAILURE,
};

enum sbi_ext_dbcn_fid {
	SBI_EXT_DBCN_CONSOLE_WRITE = 0,
	SBI_EXT_DBCN_CONSOLE_READ,
	SBI_EXT_DBCN_CONSOLE_WRITE_BYTE,
};

#ifdef CONFIG_SBI_V01
#define SBI_EXT_SET_TIMER		SBI_EXT_0_1_SET_TIMER
#define SBI_FID_SET_TIMER		0
#define SBI_EXT_SEND_IPI		SBI_EXT_0_1_SEND_IPI
#define SBI_FID_SEND_IPI		0
#define SBI_EXT_REMOTE_FENCE_I		SBI_EXT_0_1_REMOTE_FENCE_I
#define SBI_FID_REMOTE_FENCE_I		0
#define SBI_EXT_REMOTE_SFENCE_VMA	SBI_EXT_0_1_REMOTE_SFENCE_VMA
#define SBI_FID_REMOTE_SFENCE_VMA	0
#define SBI_EXT_REMOTE_SFENCE_VMA_ASID	SBI_EXT_0_1_REMOTE_SFENCE_VMA_ASID
#define SBI_FID_REMOTE_SFENCE_VMA_ASID	0
#else
#define SBI_EXT_SET_TIMER		SBI_EXT_TIME
#define SBI_FID_SET_TIMER		SBI_EXT_TIME_SET_TIMER
#define SBI_EXT_SEND_IPI		SBI_EXT_IPI
#define SBI_FID_SEND_IPI		SBI_EXT_IPI_SEND_IPI
#define SBI_EXT_REMOTE_FENCE_I		SBI_EXT_RFENCE
#define SBI_FID_REMOTE_FENCE_I		SBI_EXT_RFENCE_REMOTE_FENCE_I
#define SBI_EXT_REMOTE_SFENCE_VMA	SBI_EXT_RFENCE
#define SBI_FID_REMOTE_SFENCE_VMA	SBI_EXT_RFENCE_REMOTE_SFENCE_VMA
#define SBI_EXT_REMOTE_SFENCE_VMA_ASID	SBI_EXT_RFENCE
#define SBI_FID_REMOTE_SFENCE_VMA_ASID	SBI_EXT_RFENCE_REMOTE_SFENCE_VMA_ASID
#endif

#define SBI_SPEC_VERSION_MAJOR_SHIFT	24
#define SBI_SPEC_VERSION_MAJOR_MASK	0x7f
#define SBI_SPEC_VERSION_MINOR_MASK	0xffffff

/* SBI return error codes */
#define SBI_SUCCESS			0
#define SBI_ERR_FAILURE			-1
#define SBI_ERR_NOT_SUPPORTED		-2
#define SBI_ERR_INVALID_PARAM		-3
#define SBI_ERR_DENIED			-4
#define SBI_ERR_INVALID_ADDRESS		-5

struct sbiret {
	long error;
	long value;
};

struct sbiret sbi_ecall(int ext, int fid, unsigned long arg0,
			unsigned long arg1, unsigned long arg2,
			unsigned long arg3, unsigned long arg4,
			unsigned long arg5);

#ifdef CONFIG_SBI_V01
void sbi_console_putchar(int ch);
int sbi_console_getchar(void);
void sbi_clear_ipi(void);
void sbi_shutdown(void);
void sbi_send_ipi(const unsigned long *hart_mask);
void sbi_remote_fence_i(const unsigned long *hart_mask);
void sbi_remote_sfence_vma(const unsigned long *hart_mask,
			   unsigned long start,
			   unsigned long size);
void sbi_remote_sfence_vma_asid(const unsigned long *hart_mask,
				unsigned long start,
				unsigned long size,
				unsigned long asid);
#endif
void sbi_set_timer(uint64_t stime_value);
long sbi_get_spec_version(void);
int sbi_get_impl_id(void);
int sbi_get_impl_version(long *version);
int sbi_probe_extension(int ext);
int sbi_get_mvendorid(long *mvendorid);
int sbi_get_marchid(long *marchid);
int sbi_get_mimpid(long *mimpid);
void sbi_srst_reset(unsigned long type, unsigned long reason);
int sbi_dbcn_write_byte(unsigned char ch);

#endif
