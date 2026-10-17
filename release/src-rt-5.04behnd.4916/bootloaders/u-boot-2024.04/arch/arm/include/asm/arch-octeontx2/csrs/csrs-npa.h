/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */
#ifndef __CSRS_NPA_H__
#define __CSRS_NPA_H__

/**
 * @file
 *
 * Configuration and status register (CSR) address and type definitions for
 * NPA.
 *
 * This file is auto generated.  Do not edit.
 *
 */

/**
 * Enumeration npa_af_int_vec_e
 *
 * NPA Admin Function Interrupt Vector Enumeration Enumerates the NPA AF
 * MSI-X interrupt vectors.
 */
#define NPA_AF_INT_VEC_E_AF_ERR (3)
#define NPA_AF_INT_VEC_E_AQ_DONE (2)
#define NPA_AF_INT_VEC_E_GEN (1)
#define NPA_AF_INT_VEC_E_POISON (4)
#define NPA_AF_INT_VEC_E_RVU (0)

/**
 * Enumeration npa_aq_comp_e
 *
 * NPA Admin Queue Completion Enumeration Enumerates the values of
 * NPA_AQ_RES_S[COMPCODE].
 */
#define NPA_AQ_COMP_E_CTX_FAULT (4)
#define NPA_AQ_COMP_E_CTX_POISON (3)
#define NPA_AQ_COMP_E_GOOD (1)
#define NPA_AQ_COMP_E_LOCKERR (5)
#define NPA_AQ_COMP_E_NOTDONE (0)
#define NPA_AQ_COMP_E_SWERR (2)

/**
 * Enumeration npa_aq_ctype_e
 *
 * NPA Admin Queue Context Type Enumeration Enumerates
 * NPA_AQ_INST_S[CTYPE] values.
 */
#define NPA_AQ_CTYPE_E_AURA (0)
#define NPA_AQ_CTYPE_E_POOL (1)

/**
 * Enumeration npa_aq_instop_e
 *
 * NPA Admin Queue Opcode Enumeration Enumerates NPA_AQ_INST_S[OP]
 * values.
 */
#define NPA_AQ_INSTOP_E_INIT (1)
#define NPA_AQ_INSTOP_E_LOCK (4)
#define NPA_AQ_INSTOP_E_NOP (0)
#define NPA_AQ_INSTOP_E_READ (3)
#define NPA_AQ_INSTOP_E_UNLOCK (5)
#define NPA_AQ_INSTOP_E_WRITE (2)

/**
 * Enumeration npa_aura_err_int_e
 *
 * NPA Aura Error Interrupt Enumeration Enumerates the bit index of
 * NPA_AURA_S[ERR_INT], and NPA_AURA_S[ERR_INT_ENA].
 */
#define NPA_AURA_ERR_INT_E_AURA_ADD_OVER (1)
#define NPA_AURA_ERR_INT_E_AURA_ADD_UNDER (2)
#define NPA_AURA_ERR_INT_E_AURA_FREE_UNDER (0)
#define NPA_AURA_ERR_INT_E_POOL_DIS (3)
#define NPA_AURA_ERR_INT_E_RX(a) (0 + (a))

/**
 * Enumeration npa_bpintf_e
 *
 * NPA Backpressure Interface Enumeration Enumerates index of
 * NPA_AURA_S[BP_ENA].
 */
#define NPA_BPINTF_E_NIXX_RX(a) (0 + (a))

/**
 * Enumeration npa_inpq_e
 *
 * NPA Input Queue Enumeration Enumerates ALLOC/FREE input queues from
 * coprocessors.
 */
#define NPA_INPQ_E_AURA_OP (0xe)
#define NPA_INPQ_E_BPHY (7)
#define NPA_INPQ_E_DPI (6)
#define NPA_INPQ_E_INTERNAL_RSV (0xf)
#define NPA_INPQ_E_NIXX_RX(a) (0 + 2 * (a))
#define NPA_INPQ_E_NIXX_TX(a) (1 + 2 * (a))
#define NPA_INPQ_E_RX(a) (0 + (a))
#define NPA_INPQ_E_SSO (4)
#define NPA_INPQ_E_TIM (5)

/**
 * Enumeration npa_lf_int_vec_e
 *
 * NPA Local Function Interrupt Vector Enumeration Enumerates the NPA
 * MSI-X interrupt vectors per LF.
 */
#define NPA_LF_INT_VEC_E_ERR_INT (0x40)
#define NPA_LF_INT_VEC_E_POISON (0x41)
#define NPA_LF_INT_VEC_E_QINTX(a) (0 + (a))

/**
 * Enumeration npa_ndc0_port_e
 *
 * NPA NDC0 Port Enumeration Enumerates NPA NDC0 (NDC_IDX_E::NPA_U(0))
 * ports and the PORT index of NDC_AF_PORT()_RT()_RW()_REQ_PC and
 * NDC_AF_PORT()_RT()_RW()_LAT_PC.
 */
#define NPA_NDC0_PORT_E_AURA0 (0)
#define NPA_NDC0_PORT_E_AURA1 (1)
#define NPA_NDC0_PORT_E_POOL0 (2)
#define NPA_NDC0_PORT_E_POOL1 (3)
#define NPA_NDC0_PORT_E_STACK0 (4)
#define NPA_NDC0_PORT_E_STACK1 (5)

/**
 * Enumeration npa_pool_err_int_e
 *
 * NPA Pool Error Interrupt Enumeration Enumerates the bit index of
 * NPA_POOL_S[ERR_INT] and NPA_POOL_S[ERR_INT_ENA].
 */
#define NPA_POOL_ERR_INT_E_OVFLS (0)
#define NPA_POOL_ERR_INT_E_PERR (2)
#define NPA_POOL_ERR_INT_E_RX(a) (0 + (a))
#define NPA_POOL_ERR_INT_E_RANGE (1)

/**
 * Structure npa_aq_inst_s
 *
 * NPA Admin Queue Instruction Structure This structure specifies the AQ
 * instruction. Instructions and associated software structures are
 * stored in memory as little-endian unless NPA_AF_GEN_CFG[AF_BE] is set.
 * Hardware reads of NPA_AQ_INST_S do not allocate into LLC.  Hardware
 * reads and writes of the context structure selected by [CTYPE], [LF]
 * and [CINDEX] use the NDC and LLC caching style configured for that
 * context, i.e.: * NPA_AURA_HW_S reads and writes use
 * NPA_AF_LF()_AURAS_CFG[CACHING] and NPA_AF_LF()_AURAS_CFG[WAY_MASK]. *
 * NPA_POOL_HW_S reads and writes use NPA_AURA_HW_S[POOL_CACHING] and
 * NPA_AURA_HW_S[POOL_WAY_MASK].
 */
union npa_aq_inst_s {
	u64 u[2];
	struct npa_aq_inst_s_s {
		u64 op                               : 4;
		u64 ctype                            : 4;
		u64 lf                               : 9;
		u64 reserved_17_23                   : 7;
		u64 cindex                           : 20;
		u64 reserved_44_62                   : 19;
		u64 doneint                          : 1;
		u64 res_addr                         : 64;
	} s;
	/* struct npa_aq_inst_s_s cn; */
};

/**
 * Structure npa_aq_res_s
 *
 * NPA Admin Queue Result Structure NPA writes this structure after it
 * completes the NPA_AQ_INST_S instruction. The result structure is
 * exactly 16 bytes, and each instruction completion produces exactly one
 * result structure.  Results and associated software structures are
 * stored in memory as little-endian unless NPA_AF_GEN_CFG[AF_BE] is set.
 * When [OP] = NPA_AQ_INSTOP_E::INIT, WRITE or READ, this structure is
 * immediately followed by context read or write data. See
 * NPA_AQ_INSTOP_E.  Hardware writes of NPA_AQ_RES_S and context data
 * always allocate into LLC. Hardware reads of context data do not
 * allocate into LLC.
 */
union npa_aq_res_s {
	u64 u[2];
	struct npa_aq_res_s_s {
		u64 op                               : 4;
		u64 ctype                            : 4;
		u64 compcode                         : 8;
		u64 doneint                          : 1;
		u64 reserved_17_63                   : 47;
		u64 reserved_64_127                  : 64;
	} s;
	/* struct npa_aq_res_s_s cn; */
};

/**
 * Structure npa_aura_op_wdata_s
 *
 * NPA Aura Operation Write Data Structure This structure specifies the
 * write data format of a 64-bit atomic load-and-add to
 * NPA_LF_AURA_OP_ALLOC() and NPA_LF_POOL_OP_PC, and a 128-bit atomic
 * CASP operation to NPA_LF_AURA_OP_ALLOC().
 */
union npa_aura_op_wdata_s {
	u64 u;
	struct npa_aura_op_wdata_s_s {
		u64 aura                             : 20;
		u64 reserved_20_62                   : 43;
		u64 drop                             : 1;
	} s;
	/* struct npa_aura_op_wdata_s_s cn; */
};

/**
 * Structure npa_aura_s
 *
 * NPA Aura Context Structure This structure specifies the format used by
 * software with the NPA admin queue to read and write an aura's
 * NPA_AURA_HW_S structure maintained by hardware in LLC/DRAM.
 */
union npa_aura_s {
	u64 u[8];
	struct npa_aura_s_s {
		u64 pool_addr                        : 64;
		u64 ena                              : 1;
		u64 reserved_65_66                   : 2;
		u64 pool_caching                     : 1;
		u64 pool_way_mask                    : 16;
		u64 avg_con                          : 9;
		u64 reserved_93                      : 1;
		u64 pool_drop_ena                    : 1;
		u64 aura_drop_ena                    : 1;
		u64 bp_ena                           : 2;
		u64 reserved_98_103                  : 6;
		u64 aura_drop                        : 8;
		u64 shift                            : 6;
		u64 reserved_118_119                 : 2;
		u64 avg_level                        : 8;
		u64 count                            : 36;
		u64 reserved_164_167                 : 4;
		u64 nix0_bpid                        : 9;
		u64 reserved_177_179                 : 3;
		u64 nix1_bpid                        : 9;
		u64 reserved_189_191                 : 3;
		u64 limit                            : 36;
		u64 reserved_228_231                 : 4;
		u64 bp                               : 8;
		u64 reserved_240_243                 : 4;
		u64 fc_ena                           : 1;
		u64 fc_up_crossing                   : 1;
		u64 fc_stype                         : 2;
		u64 fc_hyst_bits                     : 4;
		u64 reserved_252_255                 : 4;
		u64 fc_addr                          : 64;
		u64 pool_drop                        : 8;
		u64 update_time                      : 16;
		u64 err_int                          : 8;
		u64 err_int_ena                      : 8;
		u64 thresh_int                       : 1;
		u64 thresh_int_ena                   : 1;
		u64 thresh_up                        : 1;
		u64 reserved_363                     : 1;
		u64 thresh_qint_idx                  : 7;
		u64 reserved_371                     : 1;
		u64 err_qint_idx                     : 7;
		u64 reserved_379_383                 : 5;
		u64 thresh                           : 36;
		u64 reserved_420_447                 : 28;
		u64 reserved_448_511                 : 64;
	} s;
	/* struct npa_aura_s_s cn; */
};

/**
 * Structure npa_pool_s
 *
 * NPA Pool Context Structure This structure specifies the format used by
 * software with the NPA admin queue to read and write a pool's
 * NPA_POOL_HW_S structure maintained by hardware in LLC/DRAM.
 */
union npa_pool_s {
	u64 u[16];
	struct npa_pool_s_s {
		u64 stack_base                       : 64;
		u64 ena                              : 1;
		u64 nat_align                        : 1;
		u64 reserved_66_67                   : 2;
		u64 stack_caching                    : 1;
		u64 reserved_69_71                   : 3;
		u64 stack_way_mask                   : 16;
		u64 buf_offset                       : 12;
		u64 reserved_100_103                 : 4;
		u64 buf_size                         : 11;
		u64 reserved_115_127                 : 13;
		u64 stack_max_pages                  : 32;
		u64 stack_pages                      : 32;
		u64 op_pc                            : 48;
		u64 reserved_240_255                 : 16;
		u64 stack_offset                     : 4;
		u64 reserved_260_263                 : 4;
		u64 shift                            : 6;
		u64 reserved_270_271                 : 2;
		u64 avg_level                        : 8;
		u64 avg_con                          : 9;
		u64 fc_ena                           : 1;
		u64 fc_stype                         : 2;
		u64 fc_hyst_bits                     : 4;
		u64 fc_up_crossing                   : 1;
		u64 reserved_297_299                 : 3;
		u64 update_time                      : 16;
		u64 reserved_316_319                 : 4;
		u64 fc_addr                          : 64;
		u64 ptr_start                        : 64;
		u64 ptr_end                          : 64;
		u64 reserved_512_535                 : 24;
		u64 err_int                          : 8;
		u64 err_int_ena                      : 8;
		u64 thresh_int                       : 1;
		u64 thresh_int_ena                   : 1;
		u64 thresh_up                        : 1;
		u64 reserved_555                     : 1;
		u64 thresh_qint_idx                  : 7;
		u64 reserved_563                     : 1;
		u64 err_qint_idx                     : 7;
		u64 reserved_571_575                 : 5;
		u64 thresh                           : 36;
		u64 reserved_612_639                 : 28;
		u64 reserved_640_703                 : 64;
		u64 reserved_704_767                 : 64;
		u64 reserved_768_831                 : 64;
		u64 reserved_832_895                 : 64;
		u64 reserved_896_959                 : 64;
		u64 reserved_960_1023                : 64;
	} s;
	/* struct npa_pool_s_s cn; */
};

/**
 * Structure npa_qint_hw_s
 *
 * NPA Queue Interrupt Context Hardware Structure This structure contains
 * context state maintained by hardware for each queue interrupt (QINT)
 * in NDC/LLC/DRAM. Software accesses this structure with the
 * NPA_LF_QINT()_* registers. Hardware maintains a table of
 * NPA_AF_CONST[QINTS] contiguous NPA_QINT_HW_S structures per LF
 * starting at IOVA NPA_AF_LF()_QINTS_BASE. Always stored in byte
 * invariant little-endian format (LE8).
 */
union npa_qint_hw_s {
	u32 u;
	struct npa_qint_hw_s_s {
		u32 count                            : 22;
		u32 reserved_22_30                   : 9;
		u32 ena                              : 1;
	} s;
	/* struct npa_qint_hw_s_s cn; */
};

/**
 * Register (RVU_PF_BAR0) npa_af_active_cycles_pc
 *
 * NPA AF Active Cycles Register
 */
union npa_af_active_cycles_pc {
	u64 u;
	struct npa_af_active_cycles_pc_s {
		u64 act_cyc                          : 64;
	} s;
	/* struct npa_af_active_cycles_pc_s cn; */
};

static inline u64 NPA_AF_ACTIVE_CYCLES_PC(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_ACTIVE_CYCLES_PC(void)
{
	return 0xf0;
}

/**
 * Register (RVU_PF_BAR0) npa_af_aq_base
 *
 * NPA AF Admin Queue Base Address Register
 */
union npa_af_aq_base {
	u64 u;
	struct npa_af_aq_base_s {
		u64 reserved_0_6                     : 7;
		u64 base_addr                        : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct npa_af_aq_base_s cn; */
};

static inline u64 NPA_AF_AQ_BASE(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_AQ_BASE(void)
{
	return 0x610;
}

/**
 * Register (RVU_PF_BAR0) npa_af_aq_cfg
 *
 * NPA AF Admin Queue Configuration Register
 */
union npa_af_aq_cfg {
	u64 u;
	struct npa_af_aq_cfg_s {
		u64 qsize                            : 4;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct npa_af_aq_cfg_s cn; */
};

static inline u64 NPA_AF_AQ_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_AQ_CFG(void)
{
	return 0x600;
}

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done
 *
 * NPA AF AQ Done Count Register
 */
union npa_af_aq_done {
	u64 u;
	struct npa_af_aq_done_s {
		u64 done                             : 20;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct npa_af_aq_done_s cn; */
};

static inline u64 NPA_AF_AQ_DONE(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_AQ_DONE(void)
{
	return 0x650;
}

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done_ack
 *
 * NPA AF AQ Done Count Ack Register This register is written by software
 * to acknowledge interrupts.
 */
union npa_af_aq_done_ack {
	u64 u;
	struct npa_af_aq_done_ack_s {
		u64 done_ack                         : 20;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct npa_af_aq_done_ack_s cn; */
};

static inline u64 NPA_AF_AQ_DONE_ACK(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_AQ_DONE_ACK(void)
{
	return 0x660;
}

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done_ena_w1c
 *
 * NPA AF AQ Done Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union npa_af_aq_done_ena_w1c {
	u64 u;
	struct npa_af_aq_done_ena_w1c_s {
		u64 done                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npa_af_aq_done_ena_w1c_s cn; */
};

static inline u64 NPA_AF_AQ_DONE_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_AQ_DONE_ENA_W1C(void)
{
	return 0x698;
}

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done_ena_w1s
 *
 * NPA AF AQ Done Interrupt Enable Set Register This register sets
 * interrupt enable bits.
 */
union npa_af_aq_done_ena_w1s {
	u64 u;
	struct npa_af_aq_done_ena_w1s_s {
		u64 done                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npa_af_aq_done_ena_w1s_s cn; */
};

static inline u64 NPA_AF_AQ_DONE_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_AQ_DONE_ENA_W1S(void)
{
	return 0x690;
}

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done_int
 *
 * NPA AF AQ Done Interrupt Register
 */
union npa_af_aq_done_int {
	u64 u;
	struct npa_af_aq_done_int_s {
		u64 done                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npa_af_aq_done_int_s cn; */
};

static inline u64 NPA_AF_AQ_DONE_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_AQ_DONE_INT(void)
{
	return 0x680;
}

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done_int_w1s
 *
 * INTERNAL: NPA AF AQ Done Interrupt Set Register
 */
union npa_af_aq_done_int_w1s {
	u64 u;
	struct npa_af_aq_done_int_w1s_s {
		u64 done                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npa_af_aq_done_int_w1s_s cn; */
};

static inline u64 NPA_AF_AQ_DONE_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_AQ_DONE_INT_W1S(void)
{
	return 0x688;
}

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done_timer
 *
 * NPA AF Admin Queue Done Interrupt Timer Register Used to debug the
 * queue interrupt coalescing timer.
 */
union npa_af_aq_done_timer {
	u64 u;
	struct npa_af_aq_done_timer_s {
		u64 count                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct npa_af_aq_done_timer_s cn; */
};

static inline u64 NPA_AF_AQ_DONE_TIMER(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_AQ_DONE_TIMER(void)
{
	return 0x670;
}

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done_wait
 *
 * NPA AF AQ Done Interrupt Coalescing Wait Register Specifies the queue
 * interrupt coalescing settings.
 */
union npa_af_aq_done_wait {
	u64 u;
	struct npa_af_aq_done_wait_s {
		u64 num_wait                         : 20;
		u64 reserved_20_31                   : 12;
		u64 time_wait                        : 16;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct npa_af_aq_done_wait_s cn; */
};

static inline u64 NPA_AF_AQ_DONE_WAIT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_AQ_DONE_WAIT(void)
{
	return 0x640;
}

/**
 * Register (RVU_PF_BAR0) npa_af_aq_door
 *
 * NPA AF Admin Queue Doorbell Register Software writes to this register
 * to enqueue one or more entries to AQ.
 */
union npa_af_aq_door {
	u64 u;
	struct npa_af_aq_door_s {
		u64 count                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct npa_af_aq_door_s cn; */
};

static inline u64 NPA_AF_AQ_DOOR(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_AQ_DOOR(void)
{
	return 0x630;
}

/**
 * Register (RVU_PF_BAR0) npa_af_aq_status
 *
 * NPA AF Admin Queue Status Register
 */
union npa_af_aq_status {
	u64 u;
	struct npa_af_aq_status_s {
		u64 reserved_0_3                     : 4;
		u64 head_ptr                         : 20;
		u64 reserved_24_35                   : 12;
		u64 tail_ptr                         : 20;
		u64 reserved_56_61                   : 6;
		u64 aq_busy                          : 1;
		u64 aq_err                           : 1;
	} s;
	struct npa_af_aq_status_cn {
		u64 reserved_0_3                     : 4;
		u64 head_ptr                         : 20;
		u64 reserved_24_31                   : 8;
		u64 reserved_32_35                   : 4;
		u64 tail_ptr                         : 20;
		u64 reserved_56_61                   : 6;
		u64 aq_busy                          : 1;
		u64 aq_err                           : 1;
	} cn;
};

static inline u64 NPA_AF_AQ_STATUS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_AQ_STATUS(void)
{
	return 0x620;
}

/**
 * Register (RVU_PF_BAR0) npa_af_avg_delay
 *
 * NPA AF Queue Average Delay Register
 */
union npa_af_avg_delay {
	u64 u;
	struct npa_af_avg_delay_s {
		u64 avg_dly                          : 19;
		u64 reserved_19_23                   : 5;
		u64 avg_timer                        : 16;
		u64 reserved_40_62                   : 23;
		u64 avg_timer_dis                    : 1;
	} s;
	/* struct npa_af_avg_delay_s cn; */
};

static inline u64 NPA_AF_AVG_DELAY(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_AVG_DELAY(void)
{
	return 0x100;
}

/**
 * Register (RVU_PF_BAR0) npa_af_bar2_alias#
 *
 * INTERNAL: NPA Admin Function  BAR2 Alias Registers  These registers
 * alias to the NPA BAR2 registers for the PF and function selected by
 * NPA_AF_BAR2_SEL[PF_FUNC].  Internal: Not implemented. Placeholder for
 * bug33464.
 */
union npa_af_bar2_aliasx {
	u64 u;
	struct npa_af_bar2_aliasx_s {
		u64 data                             : 64;
	} s;
	/* struct npa_af_bar2_aliasx_s cn; */
};

static inline u64 NPA_AF_BAR2_ALIASX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_BAR2_ALIASX(u64 a)
{
	return 0x9100000 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) npa_af_bar2_sel
 *
 * INTERNAL: NPA Admin Function BAR2 Select Register  This register
 * configures BAR2 accesses from the NPA_AF_BAR2_ALIAS() registers in
 * BAR0. Internal: Not implemented. Placeholder for bug33464.
 */
union npa_af_bar2_sel {
	u64 u;
	struct npa_af_bar2_sel_s {
		u64 alias_pf_func                    : 16;
		u64 alias_ena                        : 1;
		u64 reserved_17_63                   : 47;
	} s;
	/* struct npa_af_bar2_sel_s cn; */
};

static inline u64 NPA_AF_BAR2_SEL(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_BAR2_SEL(void)
{
	return 0x9000000;
}

/**
 * Register (RVU_PF_BAR0) npa_af_blk_rst
 *
 * NPA AF Block Reset Register
 */
union npa_af_blk_rst {
	u64 u;
	struct npa_af_blk_rst_s {
		u64 rst                              : 1;
		u64 reserved_1_62                    : 62;
		u64 busy                             : 1;
	} s;
	/* struct npa_af_blk_rst_s cn; */
};

static inline u64 NPA_AF_BLK_RST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_BLK_RST(void)
{
	return 0;
}

/**
 * Register (RVU_PF_BAR0) npa_af_bp_test
 *
 * INTERNAL: NPA AF Backpressure Test Register
 */
union npa_af_bp_test {
	u64 u;
	struct npa_af_bp_test_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 32;
		u64 enable                           : 16;
	} s;
	/* struct npa_af_bp_test_s cn; */
};

static inline u64 NPA_AF_BP_TEST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_BP_TEST(void)
{
	return 0x200;
}

/**
 * Register (RVU_PF_BAR0) npa_af_const
 *
 * NPA AF Constants Register This register contains constants for
 * software discovery.
 */
union npa_af_const {
	u64 u;
	struct npa_af_const_s {
		u64 stack_page_bytes                 : 8;
		u64 stack_page_ptrs                  : 8;
		u64 lfs                              : 12;
		u64 qints                            : 12;
		u64 num_ndc                          : 3;
		u64 reserved_43_63                   : 21;
	} s;
	/* struct npa_af_const_s cn; */
};

static inline u64 NPA_AF_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_CONST(void)
{
	return 0x10;
}

/**
 * Register (RVU_PF_BAR0) npa_af_const1
 *
 * NPA AF Constants Register 1 This register contains constants for
 * software discovery.
 */
union npa_af_const1 {
	u64 u;
	struct npa_af_const1_s {
		u64 aura_log2bytes                   : 4;
		u64 pool_log2bytes                   : 4;
		u64 qint_log2bytes                   : 4;
		u64 reserved_12_63                   : 52;
	} s;
	/* struct npa_af_const1_s cn; */
};

static inline u64 NPA_AF_CONST1(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_CONST1(void)
{
	return 0x18;
}

/**
 * Register (RVU_PF_BAR0) npa_af_dtx_filter_ctl
 *
 * NPA AF DTX LF Filter Control Register
 */
union npa_af_dtx_filter_ctl {
	u64 u;
	struct npa_af_dtx_filter_ctl_s {
		u64 ena                              : 1;
		u64 reserved_1_3                     : 3;
		u64 lf                               : 7;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct npa_af_dtx_filter_ctl_s cn; */
};

static inline u64 NPA_AF_DTX_FILTER_CTL(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_DTX_FILTER_CTL(void)
{
	return 0x10040;
}

/**
 * Register (RVU_PF_BAR0) npa_af_eco
 *
 * INTERNAL: NPA AF ECO Register
 */
union npa_af_eco {
	u64 u;
	struct npa_af_eco_s {
		u64 eco_rw                           : 32;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct npa_af_eco_s cn; */
};

static inline u64 NPA_AF_ECO(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_ECO(void)
{
	return 0x300;
}

/**
 * Register (RVU_PF_BAR0) npa_af_err_int
 *
 * NPA Admin Function Error Interrupt Register
 */
union npa_af_err_int {
	u64 u;
	struct npa_af_err_int_s {
		u64 reserved_0_11                    : 12;
		u64 aq_door_err                      : 1;
		u64 aq_res_fault                     : 1;
		u64 aq_inst_fault                    : 1;
		u64 reserved_15_63                   : 49;
	} s;
	/* struct npa_af_err_int_s cn; */
};

static inline u64 NPA_AF_ERR_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_ERR_INT(void)
{
	return 0x180;
}

/**
 * Register (RVU_PF_BAR0) npa_af_err_int_ena_w1c
 *
 * NPA Admin Function Error Interrupt Enable Clear Register This register
 * clears interrupt enable bits.
 */
union npa_af_err_int_ena_w1c {
	u64 u;
	struct npa_af_err_int_ena_w1c_s {
		u64 reserved_0_11                    : 12;
		u64 aq_door_err                      : 1;
		u64 aq_res_fault                     : 1;
		u64 aq_inst_fault                    : 1;
		u64 reserved_15_63                   : 49;
	} s;
	/* struct npa_af_err_int_ena_w1c_s cn; */
};

static inline u64 NPA_AF_ERR_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_ERR_INT_ENA_W1C(void)
{
	return 0x198;
}

/**
 * Register (RVU_PF_BAR0) npa_af_err_int_ena_w1s
 *
 * NPA Admin Function Error Interrupt Enable Set Register This register
 * sets interrupt enable bits.
 */
union npa_af_err_int_ena_w1s {
	u64 u;
	struct npa_af_err_int_ena_w1s_s {
		u64 reserved_0_11                    : 12;
		u64 aq_door_err                      : 1;
		u64 aq_res_fault                     : 1;
		u64 aq_inst_fault                    : 1;
		u64 reserved_15_63                   : 49;
	} s;
	/* struct npa_af_err_int_ena_w1s_s cn; */
};

static inline u64 NPA_AF_ERR_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_ERR_INT_ENA_W1S(void)
{
	return 0x190;
}

/**
 * Register (RVU_PF_BAR0) npa_af_err_int_w1s
 *
 * NPA Admin Function Error Interrupt Set Register This register sets
 * interrupt bits.
 */
union npa_af_err_int_w1s {
	u64 u;
	struct npa_af_err_int_w1s_s {
		u64 reserved_0_11                    : 12;
		u64 aq_door_err                      : 1;
		u64 aq_res_fault                     : 1;
		u64 aq_inst_fault                    : 1;
		u64 reserved_15_63                   : 49;
	} s;
	/* struct npa_af_err_int_w1s_s cn; */
};

static inline u64 NPA_AF_ERR_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_ERR_INT_W1S(void)
{
	return 0x188;
}

/**
 * Register (RVU_PF_BAR0) npa_af_gen_cfg
 *
 * NPA AF General Configuration Register This register provides NPA
 * control and status information.
 */
union npa_af_gen_cfg {
	u64 u;
	struct npa_af_gen_cfg_s {
		u64 reserved_0                       : 1;
		u64 af_be                            : 1;
		u64 reserved_2                       : 1;
		u64 force_cond_clk_en                : 1;
		u64 force_intf_clk_en                : 1;
		u64 reserved_5_9                     : 5;
		u64 ocla_bp                          : 1;
		u64 reserved_11                      : 1;
		u64 ratem1                           : 4;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct npa_af_gen_cfg_s cn; */
};

static inline u64 NPA_AF_GEN_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_GEN_CFG(void)
{
	return 0x30;
}

/**
 * Register (RVU_PF_BAR0) npa_af_gen_int
 *
 * NPA AF General Interrupt Register This register contains general error
 * interrupt summary bits.
 */
union npa_af_gen_int {
	u64 u;
	struct npa_af_gen_int_s {
		u64 free_dis                         : 16;
		u64 alloc_dis                        : 16;
		u64 unmapped_pf_func                 : 1;
		u64 reserved_33_63                   : 31;
	} s;
	/* struct npa_af_gen_int_s cn; */
};

static inline u64 NPA_AF_GEN_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_GEN_INT(void)
{
	return 0x140;
}

/**
 * Register (RVU_PF_BAR0) npa_af_gen_int_ena_w1c
 *
 * NPA AF General Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union npa_af_gen_int_ena_w1c {
	u64 u;
	struct npa_af_gen_int_ena_w1c_s {
		u64 free_dis                         : 16;
		u64 alloc_dis                        : 16;
		u64 unmapped_pf_func                 : 1;
		u64 reserved_33_63                   : 31;
	} s;
	/* struct npa_af_gen_int_ena_w1c_s cn; */
};

static inline u64 NPA_AF_GEN_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_GEN_INT_ENA_W1C(void)
{
	return 0x158;
}

/**
 * Register (RVU_PF_BAR0) npa_af_gen_int_ena_w1s
 *
 * NPA AF General Interrupt Enable Set Register This register sets
 * interrupt enable bits.
 */
union npa_af_gen_int_ena_w1s {
	u64 u;
	struct npa_af_gen_int_ena_w1s_s {
		u64 free_dis                         : 16;
		u64 alloc_dis                        : 16;
		u64 unmapped_pf_func                 : 1;
		u64 reserved_33_63                   : 31;
	} s;
	/* struct npa_af_gen_int_ena_w1s_s cn; */
};

static inline u64 NPA_AF_GEN_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_GEN_INT_ENA_W1S(void)
{
	return 0x150;
}

/**
 * Register (RVU_PF_BAR0) npa_af_gen_int_w1s
 *
 * NPA AF General Interrupt Set Register This register sets interrupt
 * bits.
 */
union npa_af_gen_int_w1s {
	u64 u;
	struct npa_af_gen_int_w1s_s {
		u64 free_dis                         : 16;
		u64 alloc_dis                        : 16;
		u64 unmapped_pf_func                 : 1;
		u64 reserved_33_63                   : 31;
	} s;
	/* struct npa_af_gen_int_w1s_s cn; */
};

static inline u64 NPA_AF_GEN_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_GEN_INT_W1S(void)
{
	return 0x148;
}

/**
 * Register (RVU_PF_BAR0) npa_af_inp_ctl
 *
 * NPA AF Input Control Register
 */
union npa_af_inp_ctl {
	u64 u;
	struct npa_af_inp_ctl_s {
		u64 free_dis                         : 16;
		u64 alloc_dis                        : 16;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct npa_af_inp_ctl_s cn; */
};

static inline u64 NPA_AF_INP_CTL(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_INP_CTL(void)
{
	return 0xd0;
}

/**
 * Register (RVU_PF_BAR0) npa_af_lf#_auras_cfg
 *
 * NPA AF Local Function Auras Configuration Registers
 */
union npa_af_lfx_auras_cfg {
	u64 u;
	struct npa_af_lfx_auras_cfg_s {
		u64 way_mask                         : 16;
		u64 loc_aura_size                    : 4;
		u64 loc_aura_offset                  : 14;
		u64 caching                          : 1;
		u64 be                               : 1;
		u64 rmt_aura_size                    : 4;
		u64 rmt_aura_offset                  : 14;
		u64 rmt_lf                           : 7;
		u64 reserved_61_63                   : 3;
	} s;
	struct npa_af_lfx_auras_cfg_cn96xxp1 {
		u64 way_mask                         : 16;
		u64 loc_aura_size                    : 4;
		u64 loc_aura_offset                  : 14;
		u64 caching                          : 1;
		u64 reserved_35                      : 1;
		u64 rmt_aura_size                    : 4;
		u64 rmt_aura_offset                  : 14;
		u64 rmt_lf                           : 7;
		u64 reserved_61_63                   : 3;
	} cn96xxp1;
	/* struct npa_af_lfx_auras_cfg_s cn96xxp3; */
	/* struct npa_af_lfx_auras_cfg_s cnf95xx; */
};

static inline u64 NPA_AF_LFX_AURAS_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_LFX_AURAS_CFG(u64 a)
{
	return 0x4000 + 0x40000 * a;
}

/**
 * Register (RVU_PF_BAR0) npa_af_lf#_loc_auras_base
 *
 * NPA AF Local Function Auras Base Registers
 */
union npa_af_lfx_loc_auras_base {
	u64 u;
	struct npa_af_lfx_loc_auras_base_s {
		u64 reserved_0_6                     : 7;
		u64 addr                             : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct npa_af_lfx_loc_auras_base_s cn; */
};

static inline u64 NPA_AF_LFX_LOC_AURAS_BASE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_LFX_LOC_AURAS_BASE(u64 a)
{
	return 0x4010 + 0x40000 * a;
}

/**
 * Register (RVU_PF_BAR0) npa_af_lf#_qints_base
 *
 * NPA AF Local Function Queue Interrupts Base Registers
 */
union npa_af_lfx_qints_base {
	u64 u;
	struct npa_af_lfx_qints_base_s {
		u64 reserved_0_6                     : 7;
		u64 addr                             : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct npa_af_lfx_qints_base_s cn; */
};

static inline u64 NPA_AF_LFX_QINTS_BASE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_LFX_QINTS_BASE(u64 a)
{
	return 0x4110 + 0x40000 * a;
}

/**
 * Register (RVU_PF_BAR0) npa_af_lf#_qints_cfg
 *
 * NPA AF Local Function Queue Interrupts Configuration Registers This
 * register controls access to the LF's queue interrupt context table in
 * LLC/DRAM. The table consists of NPA_AF_CONST[QINTS] contiguous
 * NPA_QINT_HW_S structures. The size of each structure is 1 \<\<
 * NPA_AF_CONST1[QINT_LOG2BYTES] bytes.
 */
union npa_af_lfx_qints_cfg {
	u64 u;
	struct npa_af_lfx_qints_cfg_s {
		u64 reserved_0_19                    : 20;
		u64 way_mask                         : 16;
		u64 caching                          : 2;
		u64 reserved_38_63                   : 26;
	} s;
	/* struct npa_af_lfx_qints_cfg_s cn; */
};

static inline u64 NPA_AF_LFX_QINTS_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_LFX_QINTS_CFG(u64 a)
{
	return 0x4100 + 0x40000 * a;
}

/**
 * Register (RVU_PF_BAR0) npa_af_lf_rst
 *
 * NPA Admin Function LF Reset Register
 */
union npa_af_lf_rst {
	u64 u;
	struct npa_af_lf_rst_s {
		u64 lf                               : 8;
		u64 reserved_8_11                    : 4;
		u64 exec                             : 1;
		u64 reserved_13_63                   : 51;
	} s;
	/* struct npa_af_lf_rst_s cn; */
};

static inline u64 NPA_AF_LF_RST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_LF_RST(void)
{
	return 0x20;
}

/**
 * Register (RVU_PF_BAR0) npa_af_ndc_cfg
 *
 * NDC AF General Configuration Register This register provides NDC
 * control.
 */
union npa_af_ndc_cfg {
	u64 u;
	struct npa_af_ndc_cfg_s {
		u64 ndc_bypass                       : 1;
		u64 ndc_ign_pois                     : 1;
		u64 byp_aura                         : 1;
		u64 byp_pool                         : 1;
		u64 byp_stack                        : 1;
		u64 byp_qint                         : 1;
		u64 reserved_6_63                    : 58;
	} s;
	/* struct npa_af_ndc_cfg_s cn; */
};

static inline u64 NPA_AF_NDC_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_NDC_CFG(void)
{
	return 0x40;
}

/**
 * Register (RVU_PF_BAR0) npa_af_ndc_sync
 *
 * NPA AF NDC Sync Register Used to synchronize the NPA NDC.
 */
union npa_af_ndc_sync {
	u64 u;
	struct npa_af_ndc_sync_s {
		u64 lf                               : 8;
		u64 reserved_8_11                    : 4;
		u64 exec                             : 1;
		u64 reserved_13_63                   : 51;
	} s;
	/* struct npa_af_ndc_sync_s cn; */
};

static inline u64 NPA_AF_NDC_SYNC(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_NDC_SYNC(void)
{
	return 0x50;
}

/**
 * Register (RVU_PF_BAR0) npa_af_ras
 *
 * NPA AF RAS Interrupt Register This register is intended for delivery
 * of RAS events to the SCP, so should be ignored by OS drivers.
 */
union npa_af_ras {
	u64 u;
	struct npa_af_ras_s {
		u64 reserved_0_31                    : 32;
		u64 aq_ctx_poison                    : 1;
		u64 aq_res_poison                    : 1;
		u64 aq_inst_poison                   : 1;
		u64 reserved_35_63                   : 29;
	} s;
	/* struct npa_af_ras_s cn; */
};

static inline u64 NPA_AF_RAS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_RAS(void)
{
	return 0x1a0;
}

/**
 * Register (RVU_PF_BAR0) npa_af_ras_ena_w1c
 *
 * NPA AF RAS Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union npa_af_ras_ena_w1c {
	u64 u;
	struct npa_af_ras_ena_w1c_s {
		u64 reserved_0_31                    : 32;
		u64 aq_ctx_poison                    : 1;
		u64 aq_res_poison                    : 1;
		u64 aq_inst_poison                   : 1;
		u64 reserved_35_63                   : 29;
	} s;
	/* struct npa_af_ras_ena_w1c_s cn; */
};

static inline u64 NPA_AF_RAS_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_RAS_ENA_W1C(void)
{
	return 0x1b8;
}

/**
 * Register (RVU_PF_BAR0) npa_af_ras_ena_w1s
 *
 * NPA AF RAS Interrupt Enable Set Register This register sets interrupt
 * enable bits.
 */
union npa_af_ras_ena_w1s {
	u64 u;
	struct npa_af_ras_ena_w1s_s {
		u64 reserved_0_31                    : 32;
		u64 aq_ctx_poison                    : 1;
		u64 aq_res_poison                    : 1;
		u64 aq_inst_poison                   : 1;
		u64 reserved_35_63                   : 29;
	} s;
	/* struct npa_af_ras_ena_w1s_s cn; */
};

static inline u64 NPA_AF_RAS_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_RAS_ENA_W1S(void)
{
	return 0x1b0;
}

/**
 * Register (RVU_PF_BAR0) npa_af_ras_w1s
 *
 * NPA AF RAS Interrupt Set Register This register sets interrupt bits.
 */
union npa_af_ras_w1s {
	u64 u;
	struct npa_af_ras_w1s_s {
		u64 reserved_0_31                    : 32;
		u64 aq_ctx_poison                    : 1;
		u64 aq_res_poison                    : 1;
		u64 aq_inst_poison                   : 1;
		u64 reserved_35_63                   : 29;
	} s;
	/* struct npa_af_ras_w1s_s cn; */
};

static inline u64 NPA_AF_RAS_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_RAS_W1S(void)
{
	return 0x1a8;
}

/**
 * Register (RVU_PF_BAR0) npa_af_rvu_int
 *
 * NPA AF RVU Interrupt Register This register contains RVU error
 * interrupt summary bits.
 */
union npa_af_rvu_int {
	u64 u;
	struct npa_af_rvu_int_s {
		u64 unmapped_slot                    : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npa_af_rvu_int_s cn; */
};

static inline u64 NPA_AF_RVU_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_RVU_INT(void)
{
	return 0x160;
}

/**
 * Register (RVU_PF_BAR0) npa_af_rvu_int_ena_w1c
 *
 * NPA AF RVU Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union npa_af_rvu_int_ena_w1c {
	u64 u;
	struct npa_af_rvu_int_ena_w1c_s {
		u64 unmapped_slot                    : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npa_af_rvu_int_ena_w1c_s cn; */
};

static inline u64 NPA_AF_RVU_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_RVU_INT_ENA_W1C(void)
{
	return 0x178;
}

/**
 * Register (RVU_PF_BAR0) npa_af_rvu_int_ena_w1s
 *
 * NPA AF RVU Interrupt Enable Set Register This register sets interrupt
 * enable bits.
 */
union npa_af_rvu_int_ena_w1s {
	u64 u;
	struct npa_af_rvu_int_ena_w1s_s {
		u64 unmapped_slot                    : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npa_af_rvu_int_ena_w1s_s cn; */
};

static inline u64 NPA_AF_RVU_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_RVU_INT_ENA_W1S(void)
{
	return 0x170;
}

/**
 * Register (RVU_PF_BAR0) npa_af_rvu_int_w1s
 *
 * NPA AF RVU Interrupt Set Register This register sets interrupt bits.
 */
union npa_af_rvu_int_w1s {
	u64 u;
	struct npa_af_rvu_int_w1s_s {
		u64 unmapped_slot                    : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npa_af_rvu_int_w1s_s cn; */
};

static inline u64 NPA_AF_RVU_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_RVU_INT_W1S(void)
{
	return 0x168;
}

/**
 * Register (RVU_PF_BAR0) npa_af_rvu_lf_cfg_debug
 *
 * NPA Privileged LF Configuration Debug Register This debug register
 * allows software to lookup the reverse mapping from VF/PF slot to LF.
 * The forward mapping is programmed with NPA_PRIV_LF()_CFG.
 */
union npa_af_rvu_lf_cfg_debug {
	u64 u;
	struct npa_af_rvu_lf_cfg_debug_s {
		u64 lf                               : 12;
		u64 lf_valid                         : 1;
		u64 exec                             : 1;
		u64 reserved_14_15                   : 2;
		u64 slot                             : 8;
		u64 pf_func                          : 16;
		u64 reserved_40_63                   : 24;
	} s;
	/* struct npa_af_rvu_lf_cfg_debug_s cn; */
};

static inline u64 NPA_AF_RVU_LF_CFG_DEBUG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_AF_RVU_LF_CFG_DEBUG(void)
{
	return 0x10030;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_aura_op_alloc#
 *
 * NPA Aura Allocate Operation Registers These registers are used to
 * allocate one or two pointers from a given aura's pool. A 64-bit atomic
 * load-and-add to NPA_LF_AURA_OP_ALLOC(0) allocates a single pointer. A
 * 128-bit atomic CASP operation to NPA_LF_AURA_OP_ALLOC(0..1) allocates
 * two pointers. The atomic write data format is NPA_AURA_OP_WDATA_S. For
 * CASP, the first SWAP word in the write data contains
 * NPA_AURA_OP_WDATA_S and the remaining write data words are ignored.
 * All other accesses to this register (e.g. reads and writes) are
 * RAZ/WI.  RSL accesses to this register are RAZ/WI.
 */
union npa_lf_aura_op_allocx {
	u64 u;
	struct npa_lf_aura_op_allocx_s {
		u64 addr                             : 64;
	} s;
	/* struct npa_lf_aura_op_allocx_s cn; */
};

static inline u64 NPA_LF_AURA_OP_ALLOCX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_AURA_OP_ALLOCX(u64 a)
{
	return 0x10 + 8 * a;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_aura_op_cnt
 *
 * NPA LF Aura Count Register A 64-bit atomic load-and-add to this
 * register returns a given aura's count. A write sets or adds the aura's
 * count. A read is RAZ.  RSL accesses to this register are RAZ/WI.
 */
union npa_lf_aura_op_cnt {
	u64 u;
	struct npa_lf_aura_op_cnt_s {
		u64 count                            : 36;
		u64 reserved_36_41                   : 6;
		u64 op_err                           : 1;
		u64 cnt_add                          : 1;
		u64 aura                             : 20;
	} s;
	/* struct npa_lf_aura_op_cnt_s cn; */
};

static inline u64 NPA_LF_AURA_OP_CNT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_AURA_OP_CNT(void)
{
	return 0x30;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_aura_op_free0
 *
 * NPA LF Aura Free Operation Register 0 A 128-bit write (STP) to
 * NPA_LF_AURA_OP_FREE0 and NPA_LF_AURA_OP_FREE1 frees a pointer into a
 * given aura's pool. All other accesses to these registers (e.g. reads
 * and 64-bit writes) are RAZ/WI.  RSL accesses to this register are
 * RAZ/WI.
 */
union npa_lf_aura_op_free0 {
	u64 u;
	struct npa_lf_aura_op_free0_s {
		u64 addr                             : 64;
	} s;
	/* struct npa_lf_aura_op_free0_s cn; */
};

static inline u64 NPA_LF_AURA_OP_FREE0(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_AURA_OP_FREE0(void)
{
	return 0x20;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_aura_op_free1
 *
 * NPA LF Aura Free Operation Register 1 See NPA_LF_AURA_OP_FREE0.  RSL
 * accesses to this register are RAZ/WI.
 */
union npa_lf_aura_op_free1 {
	u64 u;
	struct npa_lf_aura_op_free1_s {
		u64 aura                             : 20;
		u64 reserved_20_62                   : 43;
		u64 fabs                             : 1;
	} s;
	/* struct npa_lf_aura_op_free1_s cn; */
};

static inline u64 NPA_LF_AURA_OP_FREE1(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_AURA_OP_FREE1(void)
{
	return 0x28;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_aura_op_int
 *
 * NPA LF Aura Interrupt Operation Register A 64-bit atomic load-and-add
 * to this register reads
 * NPA_AURA_HW_S[ERR_INT,ERR_INT_ENA,THRESH_INT,THRESH_INT_ENA]. A write
 * optionally sets or clears these fields. A read is RAZ.  RSL accesses
 * to this register are RAZ/WI.
 */
union npa_lf_aura_op_int {
	u64 u;
	struct npa_lf_aura_op_int_s {
		u64 err_int                          : 8;
		u64 err_int_ena                      : 8;
		u64 thresh_int                       : 1;
		u64 thresh_int_ena                   : 1;
		u64 reserved_18_41                   : 24;
		u64 op_err                           : 1;
		u64 setop                            : 1;
		u64 aura                             : 20;
	} s;
	/* struct npa_lf_aura_op_int_s cn; */
};

static inline u64 NPA_LF_AURA_OP_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_AURA_OP_INT(void)
{
	return 0x60;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_aura_op_limit
 *
 * NPA LF Aura Allocation Limit Register A 64-bit atomic load-and-add to
 * this register returns a given aura's limit. A write sets the aura's
 * limit. A read is RAZ.  RSL accesses to this register are RAZ/WI.
 */
union npa_lf_aura_op_limit {
	u64 u;
	struct npa_lf_aura_op_limit_s {
		u64 limit                            : 36;
		u64 reserved_36_41                   : 6;
		u64 op_err                           : 1;
		u64 reserved_43                      : 1;
		u64 aura                             : 20;
	} s;
	/* struct npa_lf_aura_op_limit_s cn; */
};

static inline u64 NPA_LF_AURA_OP_LIMIT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_AURA_OP_LIMIT(void)
{
	return 0x50;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_aura_op_thresh
 *
 * NPA LF Aura Threshold Operation Register A 64-bit atomic load-and-add
 * to this register reads NPA_AURA_HW_S[THRESH_UP,THRESH]. A write to the
 * register writes NPA_AURA_HW_S[THRESH_UP,THRESH] and recomputes
 * NPA_AURA_HW_S[THRESH_INT]. A read is RAZ.  RSL accesses to this
 * register are RAZ/WI.
 */
union npa_lf_aura_op_thresh {
	u64 u;
	struct npa_lf_aura_op_thresh_s {
		u64 thresh                           : 36;
		u64 reserved_36_41                   : 6;
		u64 op_err                           : 1;
		u64 thresh_up                        : 1;
		u64 aura                             : 20;
	} s;
	/* struct npa_lf_aura_op_thresh_s cn; */
};

static inline u64 NPA_LF_AURA_OP_THRESH(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_AURA_OP_THRESH(void)
{
	return 0x70;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_err_int
 *
 * NPA LF Error Interrupt Register
 */
union npa_lf_err_int {
	u64 u;
	struct npa_lf_err_int_s {
		u64 aura_dis                         : 1;
		u64 aura_oor                         : 1;
		u64 reserved_2                       : 1;
		u64 rmt_req_oor                      : 1;
		u64 reserved_4_11                    : 8;
		u64 aura_fault                       : 1;
		u64 pool_fault                       : 1;
		u64 stack_fault                      : 1;
		u64 qint_fault                       : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct npa_lf_err_int_s cn; */
};

static inline u64 NPA_LF_ERR_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_ERR_INT(void)
{
	return 0x200;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_err_int_ena_w1c
 *
 * NPA LF Error Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union npa_lf_err_int_ena_w1c {
	u64 u;
	struct npa_lf_err_int_ena_w1c_s {
		u64 aura_dis                         : 1;
		u64 aura_oor                         : 1;
		u64 reserved_2                       : 1;
		u64 rmt_req_oor                      : 1;
		u64 reserved_4_11                    : 8;
		u64 aura_fault                       : 1;
		u64 pool_fault                       : 1;
		u64 stack_fault                      : 1;
		u64 qint_fault                       : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct npa_lf_err_int_ena_w1c_s cn; */
};

static inline u64 NPA_LF_ERR_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_ERR_INT_ENA_W1C(void)
{
	return 0x210;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_err_int_ena_w1s
 *
 * NPA LF Error Interrupt Enable Set Register This register sets
 * interrupt enable bits.
 */
union npa_lf_err_int_ena_w1s {
	u64 u;
	struct npa_lf_err_int_ena_w1s_s {
		u64 aura_dis                         : 1;
		u64 aura_oor                         : 1;
		u64 reserved_2                       : 1;
		u64 rmt_req_oor                      : 1;
		u64 reserved_4_11                    : 8;
		u64 aura_fault                       : 1;
		u64 pool_fault                       : 1;
		u64 stack_fault                      : 1;
		u64 qint_fault                       : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct npa_lf_err_int_ena_w1s_s cn; */
};

static inline u64 NPA_LF_ERR_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_ERR_INT_ENA_W1S(void)
{
	return 0x218;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_err_int_w1s
 *
 * NPA LF Error Interrupt Set Register This register sets interrupt bits.
 */
union npa_lf_err_int_w1s {
	u64 u;
	struct npa_lf_err_int_w1s_s {
		u64 aura_dis                         : 1;
		u64 aura_oor                         : 1;
		u64 reserved_2                       : 1;
		u64 rmt_req_oor                      : 1;
		u64 reserved_4_11                    : 8;
		u64 aura_fault                       : 1;
		u64 pool_fault                       : 1;
		u64 stack_fault                      : 1;
		u64 qint_fault                       : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct npa_lf_err_int_w1s_s cn; */
};

static inline u64 NPA_LF_ERR_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_ERR_INT_W1S(void)
{
	return 0x208;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_available
 *
 * NPA LF Pool Available Count Operation Register A 64-bit atomic load-
 * and-add to this register returns a given pool's free pointer count.
 * Reads and writes are RAZ/WI.  RSL accesses to this register are
 * RAZ/WI.
 */
union npa_lf_pool_op_available {
	u64 u;
	struct npa_lf_pool_op_available_s {
		u64 count                            : 36;
		u64 reserved_36_41                   : 6;
		u64 op_err                           : 1;
		u64 reserved_43                      : 1;
		u64 aura                             : 20;
	} s;
	/* struct npa_lf_pool_op_available_s cn; */
};

static inline u64 NPA_LF_POOL_OP_AVAILABLE(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_POOL_OP_AVAILABLE(void)
{
	return 0x110;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_int
 *
 * NPA LF Pool Interrupt Operation Register A 64-bit atomic load-and-add
 * to this register reads
 * NPA_POOL_S[ERR_INT,ERR_INT_ENA,THRESH_INT,THRESH_INT_ENA]. A write
 * optionally sets or clears these fields. A read is RAZ.  RSL accesses
 * to this register are RAZ/WI.
 */
union npa_lf_pool_op_int {
	u64 u;
	struct npa_lf_pool_op_int_s {
		u64 err_int                          : 8;
		u64 err_int_ena                      : 8;
		u64 thresh_int                       : 1;
		u64 thresh_int_ena                   : 1;
		u64 reserved_18_41                   : 24;
		u64 op_err                           : 1;
		u64 setop                            : 1;
		u64 aura                             : 20;
	} s;
	/* struct npa_lf_pool_op_int_s cn; */
};

static inline u64 NPA_LF_POOL_OP_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_POOL_OP_INT(void)
{
	return 0x160;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_pc
 *
 * NPA LF Pool Performance Count Register A 64-bit atomic load-and-add to
 * this register reads NPA_POOL_S[OP_PC] from a given aura's pool. The
 * aura is selected by the atomic write data, whose format is
 * NPA_AURA_OP_WDATA_S. Reads and writes are RAZ/WI.  RSL accesses to
 * this register are RAZ/WI.
 */
union npa_lf_pool_op_pc {
	u64 u;
	struct npa_lf_pool_op_pc_s {
		u64 op_pc                            : 48;
		u64 op_err                           : 1;
		u64 reserved_49_63                   : 15;
	} s;
	/* struct npa_lf_pool_op_pc_s cn; */
};

static inline u64 NPA_LF_POOL_OP_PC(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_POOL_OP_PC(void)
{
	return 0x100;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_ptr_end0
 *
 * NPA LF Pool Pointer End Operation Register 0 A 128-bit write (STP) to
 * the NPA_LF_POOL_OP_PTR_END0 and NPA_LF_POOL_OP_PTR_END1 registers
 * writes to a given pool's pointer end value. All other accesses to
 * these registers (e.g. reads and 64-bit writes) are RAZ/WI.  RSL
 * accesses to this register are RAZ/WI.
 */
union npa_lf_pool_op_ptr_end0 {
	u64 u;
	struct npa_lf_pool_op_ptr_end0_s {
		u64 ptr_end                          : 64;
	} s;
	/* struct npa_lf_pool_op_ptr_end0_s cn; */
};

static inline u64 NPA_LF_POOL_OP_PTR_END0(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_POOL_OP_PTR_END0(void)
{
	return 0x130;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_ptr_end1
 *
 * NPA LF Pool Pointer End Operation Register 1 See
 * NPA_LF_POOL_OP_PTR_END0.  RSL accesses to this register are RAZ/WI.
 */
union npa_lf_pool_op_ptr_end1 {
	u64 u;
	struct npa_lf_pool_op_ptr_end1_s {
		u64 aura                             : 20;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct npa_lf_pool_op_ptr_end1_s cn; */
};

static inline u64 NPA_LF_POOL_OP_PTR_END1(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_POOL_OP_PTR_END1(void)
{
	return 0x138;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_ptr_start0
 *
 * NPA LF Pool Pointer Start Operation Register 0 A 128-bit write (STP)
 * to the NPA_LF_POOL_OP_PTR_START0 and NPA_LF_POOL_OP_PTR_START1
 * registers writes to a given pool's pointer start value. All other
 * accesses to these registers (e.g. reads and 64-bit writes) are RAZ/WI.
 * RSL accesses to this register are RAZ/WI.
 */
union npa_lf_pool_op_ptr_start0 {
	u64 u;
	struct npa_lf_pool_op_ptr_start0_s {
		u64 ptr_start                        : 64;
	} s;
	/* struct npa_lf_pool_op_ptr_start0_s cn; */
};

static inline u64 NPA_LF_POOL_OP_PTR_START0(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_POOL_OP_PTR_START0(void)
{
	return 0x120;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_ptr_start1
 *
 * NPA LF Pool Pointer Start Operation Register 1 See
 * NPA_LF_POOL_OP_PTR_START0.  RSL accesses to this register are RAZ/WI.
 */
union npa_lf_pool_op_ptr_start1 {
	u64 u;
	struct npa_lf_pool_op_ptr_start1_s {
		u64 aura                             : 20;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct npa_lf_pool_op_ptr_start1_s cn; */
};

static inline u64 NPA_LF_POOL_OP_PTR_START1(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_POOL_OP_PTR_START1(void)
{
	return 0x128;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_thresh
 *
 * NPA LF Pool Threshold Operation Register A 64-bit atomic load-and-add
 * to this register reads NPA_POOL_S[THRESH_UP,THRESH]. A write to the
 * register writes NPA_POOL_S[THRESH_UP,THRESH]. A read is RAZ.  RSL
 * accesses to this register are RAZ/WI.
 */
union npa_lf_pool_op_thresh {
	u64 u;
	struct npa_lf_pool_op_thresh_s {
		u64 thresh                           : 36;
		u64 reserved_36_41                   : 6;
		u64 op_err                           : 1;
		u64 thresh_up                        : 1;
		u64 aura                             : 20;
	} s;
	/* struct npa_lf_pool_op_thresh_s cn; */
};

static inline u64 NPA_LF_POOL_OP_THRESH(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_POOL_OP_THRESH(void)
{
	return 0x170;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_qint#_cnt
 *
 * NPA LF Queue Interrupt Count Registers
 */
union npa_lf_qintx_cnt {
	u64 u;
	struct npa_lf_qintx_cnt_s {
		u64 count                            : 22;
		u64 reserved_22_63                   : 42;
	} s;
	/* struct npa_lf_qintx_cnt_s cn; */
};

static inline u64 NPA_LF_QINTX_CNT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_QINTX_CNT(u64 a)
{
	return 0x300 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_qint#_ena_w1c
 *
 * NPA LF Queue Interrupt Enable Clear Registers This register clears
 * interrupt enable bits.
 */
union npa_lf_qintx_ena_w1c {
	u64 u;
	struct npa_lf_qintx_ena_w1c_s {
		u64 intr                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npa_lf_qintx_ena_w1c_s cn; */
};

static inline u64 NPA_LF_QINTX_ENA_W1C(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_QINTX_ENA_W1C(u64 a)
{
	return 0x330 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_qint#_ena_w1s
 *
 * NPA LF Queue Interrupt Enable Set Registers This register sets
 * interrupt enable bits.
 */
union npa_lf_qintx_ena_w1s {
	u64 u;
	struct npa_lf_qintx_ena_w1s_s {
		u64 intr                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npa_lf_qintx_ena_w1s_s cn; */
};

static inline u64 NPA_LF_QINTX_ENA_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_QINTX_ENA_W1S(u64 a)
{
	return 0x320 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_qint#_int
 *
 * NPA LF Queue Interrupt Registers
 */
union npa_lf_qintx_int {
	u64 u;
	struct npa_lf_qintx_int_s {
		u64 intr                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npa_lf_qintx_int_s cn; */
};

static inline u64 NPA_LF_QINTX_INT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_QINTX_INT(u64 a)
{
	return 0x310 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_qint#_int_w1s
 *
 * INTERNAL: NPA LF Queue Interrupt Set Registers
 */
union npa_lf_qintx_int_w1s {
	u64 u;
	struct npa_lf_qintx_int_w1s_s {
		u64 intr                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npa_lf_qintx_int_w1s_s cn; */
};

static inline u64 NPA_LF_QINTX_INT_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_QINTX_INT_W1S(u64 a)
{
	return 0x318 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_ras
 *
 * NPA LF RAS Interrupt Register
 */
union npa_lf_ras {
	u64 u;
	struct npa_lf_ras_s {
		u64 aura_poison                      : 1;
		u64 pool_poison                      : 1;
		u64 stack_poison                     : 1;
		u64 qint_poison                      : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct npa_lf_ras_s cn; */
};

static inline u64 NPA_LF_RAS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_RAS(void)
{
	return 0x220;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_ras_ena_w1c
 *
 * NPA LF RAS Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union npa_lf_ras_ena_w1c {
	u64 u;
	struct npa_lf_ras_ena_w1c_s {
		u64 aura_poison                      : 1;
		u64 pool_poison                      : 1;
		u64 stack_poison                     : 1;
		u64 qint_poison                      : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct npa_lf_ras_ena_w1c_s cn; */
};

static inline u64 NPA_LF_RAS_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_RAS_ENA_W1C(void)
{
	return 0x230;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_ras_ena_w1s
 *
 * NPA LF RAS Interrupt Enable Set Register This register sets interrupt
 * enable bits.
 */
union npa_lf_ras_ena_w1s {
	u64 u;
	struct npa_lf_ras_ena_w1s_s {
		u64 aura_poison                      : 1;
		u64 pool_poison                      : 1;
		u64 stack_poison                     : 1;
		u64 qint_poison                      : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct npa_lf_ras_ena_w1s_s cn; */
};

static inline u64 NPA_LF_RAS_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_RAS_ENA_W1S(void)
{
	return 0x238;
}

/**
 * Register (RVU_PFVF_BAR2) npa_lf_ras_w1s
 *
 * NPA LF RAS Interrupt Set Register This register sets interrupt bits.
 */
union npa_lf_ras_w1s {
	u64 u;
	struct npa_lf_ras_w1s_s {
		u64 aura_poison                      : 1;
		u64 pool_poison                      : 1;
		u64 stack_poison                     : 1;
		u64 qint_poison                      : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct npa_lf_ras_w1s_s cn; */
};

static inline u64 NPA_LF_RAS_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_LF_RAS_W1S(void)
{
	return 0x228;
}

/**
 * Register (RVU_PF_BAR0) npa_priv_af_int_cfg
 *
 * NPA Privileged AF Interrupt Configuration Register
 */
union npa_priv_af_int_cfg {
	u64 u;
	struct npa_priv_af_int_cfg_s {
		u64 msix_offset                      : 11;
		u64 reserved_11                      : 1;
		u64 msix_size                        : 8;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct npa_priv_af_int_cfg_s cn; */
};

static inline u64 NPA_PRIV_AF_INT_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_PRIV_AF_INT_CFG(void)
{
	return 0x10000;
}

/**
 * Register (RVU_PF_BAR0) npa_priv_lf#_cfg
 *
 * NPA Privileged Local Function Configuration Registers These registers
 * allow each NPA local function (LF) to be provisioned to a VF/PF slot
 * for RVU. See also NPA_AF_RVU_LF_CFG_DEBUG.  Software should read this
 * register after write to ensure that the LF is mapped to [PF_FUNC]
 * before issuing transactions to the mapped PF and function.  [SLOT]
 * must be zero.  Internal: Hardware ignores [SLOT] and always assumes
 * 0x0.
 */
union npa_priv_lfx_cfg {
	u64 u;
	struct npa_priv_lfx_cfg_s {
		u64 slot                             : 8;
		u64 pf_func                          : 16;
		u64 reserved_24_62                   : 39;
		u64 ena                              : 1;
	} s;
	/* struct npa_priv_lfx_cfg_s cn; */
};

static inline u64 NPA_PRIV_LFX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_PRIV_LFX_CFG(u64 a)
{
	return 0x10010 + 0x100 * a;
}

/**
 * Register (RVU_PF_BAR0) npa_priv_lf#_int_cfg
 *
 * NPA Privileged LF Interrupt Configuration Registers
 */
union npa_priv_lfx_int_cfg {
	u64 u;
	struct npa_priv_lfx_int_cfg_s {
		u64 msix_offset                      : 11;
		u64 reserved_11                      : 1;
		u64 msix_size                        : 8;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct npa_priv_lfx_int_cfg_s cn; */
};

static inline u64 NPA_PRIV_LFX_INT_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPA_PRIV_LFX_INT_CFG(u64 a)
{
	return 0x10020 + 0x100 * a;
}

#endif /* __CSRS_NPA_H__ */
