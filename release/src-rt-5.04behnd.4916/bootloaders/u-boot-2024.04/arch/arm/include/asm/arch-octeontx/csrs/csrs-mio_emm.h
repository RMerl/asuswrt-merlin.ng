/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */
#ifndef __CSRS_MIO_EMM_H__
#define __CSRS_MIO_EMM_H__

/**
 * @file
 *
 * Configuration and status register (CSR) address and type definitions for
 * MIO_EMM.
 *
 * This file is auto generated.  Do not edit.
 *
 */

/**
 * Enumeration mio_emm_bar_e
 *
 * eMMC Base Address Register Enumeration Enumerates the base address
 * registers.
 */
#define MIO_EMM_BAR_E_MIO_EMM_PF_BAR0_CN8 (0x87e009000000ll)
#define MIO_EMM_BAR_E_MIO_EMM_PF_BAR0_CN8_SIZE 0x800000ull
#define MIO_EMM_BAR_E_MIO_EMM_PF_BAR0_CN9 (0x87e009000000ll)
#define MIO_EMM_BAR_E_MIO_EMM_PF_BAR0_CN9_SIZE 0x10000ull
#define MIO_EMM_BAR_E_MIO_EMM_PF_BAR4 (0x87e009f00000ll)
#define MIO_EMM_BAR_E_MIO_EMM_PF_BAR4_SIZE 0x100000ull

/**
 * Enumeration mio_emm_int_vec_e
 *
 * eMMC MSI-X Vector Enumeration Enumerates the MSI-X interrupt vectors.
 */
#define MIO_EMM_INT_VEC_E_DMA_INT_DONE (8)
#define MIO_EMM_INT_VEC_E_DMA_INT_FIFO (7)
#define MIO_EMM_INT_VEC_E_EMM_BUF_DONE (0)
#define MIO_EMM_INT_VEC_E_EMM_CMD_DONE (1)
#define MIO_EMM_INT_VEC_E_EMM_CMD_ERR (3)
#define MIO_EMM_INT_VEC_E_EMM_DMA_DONE (2)
#define MIO_EMM_INT_VEC_E_EMM_DMA_ERR (4)
#define MIO_EMM_INT_VEC_E_EMM_SWITCH_DONE (5)
#define MIO_EMM_INT_VEC_E_EMM_SWITCH_ERR (6)
#define MIO_EMM_INT_VEC_E_NCB_FLT (9)
#define MIO_EMM_INT_VEC_E_NCB_RAS (0xa)

/**
 * Register (RSL) mio_emm_access_wdog
 *
 * eMMC Access Watchdog Register
 */
union mio_emm_access_wdog {
	u64 u;
	struct mio_emm_access_wdog_s {
		u64 clk_cnt                          : 32;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct mio_emm_access_wdog_s cn; */
};

static inline u64 MIO_EMM_ACCESS_WDOG(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_ACCESS_WDOG(void)
{
	return 0x20f0;
}

/**
 * Register (RSL) mio_emm_buf_dat
 *
 * eMMC Data Buffer Access Register
 */
union mio_emm_buf_dat {
	u64 u;
	struct mio_emm_buf_dat_s {
		u64 dat                              : 64;
	} s;
	/* struct mio_emm_buf_dat_s cn; */
};

static inline u64 MIO_EMM_BUF_DAT(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_BUF_DAT(void)
{
	return 0x20e8;
}

/**
 * Register (RSL) mio_emm_buf_idx
 *
 * eMMC Data Buffer Address Register
 */
union mio_emm_buf_idx {
	u64 u;
	struct mio_emm_buf_idx_s {
		u64 offset                           : 6;
		u64 buf_num                          : 1;
		u64 reserved_7_15                    : 9;
		u64 inc                              : 1;
		u64 reserved_17_63                   : 47;
	} s;
	/* struct mio_emm_buf_idx_s cn; */
};

static inline u64 MIO_EMM_BUF_IDX(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_BUF_IDX(void)
{
	return 0x20e0;
}

/**
 * Register (RSL) mio_emm_calb
 *
 * eMMC Calbration Register This register initiates delay line
 * characterization.
 */
union mio_emm_calb {
	u64 u;
	struct mio_emm_calb_s {
		u64 start                            : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct mio_emm_calb_s cn; */
};

static inline u64 MIO_EMM_CALB(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_CALB(void)
{
	return 0x20c0;
}

/**
 * Register (RSL) mio_emm_cfg
 *
 * eMMC Configuration Register
 */
union mio_emm_cfg {
	u64 u;
	struct mio_emm_cfg_s {
		u64 bus_ena                          : 4;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct mio_emm_cfg_s cn; */
};

static inline u64 MIO_EMM_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_CFG(void)
{
	return 0x2000;
}

/**
 * Register (RSL) mio_emm_cmd
 *
 * eMMC Command Register
 */
union mio_emm_cmd {
	u64 u;
	struct mio_emm_cmd_s {
		u64 arg                              : 32;
		u64 cmd_idx                          : 6;
		u64 rtype_xor                        : 3;
		u64 ctype_xor                        : 2;
		u64 reserved_43_48                   : 6;
		u64 offset                           : 6;
		u64 dbuf                             : 1;
		u64 reserved_56_58                   : 3;
		u64 cmd_val                          : 1;
		u64 bus_id                           : 2;
		u64 skip_busy                        : 1;
		u64 reserved_63                      : 1;
	} s;
	/* struct mio_emm_cmd_s cn; */
};

static inline u64 MIO_EMM_CMD(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_CMD(void)
{
	return 0x2058;
}

/**
 * Register (RSL) mio_emm_comp
 *
 * eMMC Compensation Register
 */
union mio_emm_comp {
	u64 u;
	struct mio_emm_comp_s {
		u64 nctl                             : 3;
		u64 reserved_3_7                     : 5;
		u64 pctl                             : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct mio_emm_comp_s cn; */
};

static inline u64 MIO_EMM_COMP(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_COMP(void)
{
	return 0x2040;
}

/**
 * Register (RSL) mio_emm_debug
 *
 * eMMC Debug Register
 */
union mio_emm_debug {
	u64 u;
	struct mio_emm_debug_s {
		u64 clk_on                           : 1;
		u64 reserved_1_7                     : 7;
		u64 cmd_sm                           : 4;
		u64 data_sm                          : 4;
		u64 dma_sm                           : 4;
		u64 emmc_clk_disable                 : 1;
		u64 rdsync_rst                       : 1;
		u64 reserved_22_63                   : 42;
	} s;
	struct mio_emm_debug_cn96xxp1 {
		u64 clk_on                           : 1;
		u64 reserved_1_7                     : 7;
		u64 cmd_sm                           : 4;
		u64 data_sm                          : 4;
		u64 dma_sm                           : 4;
		u64 reserved_20_63                   : 44;
	} cn96xxp1;
	/* struct mio_emm_debug_s cn96xxp3; */
	/* struct mio_emm_debug_cn96xxp1 cnf95xx; */
};

static inline u64 MIO_EMM_DEBUG(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_DEBUG(void)
{
	return 0x20f8;
}

/**
 * Register (RSL) mio_emm_dma
 *
 * eMMC External DMA Configuration Register
 */
union mio_emm_dma {
	u64 u;
	struct mio_emm_dma_s {
		u64 card_addr                        : 32;
		u64 block_cnt                        : 16;
		u64 multi                            : 1;
		u64 rw                               : 1;
		u64 rel_wr                           : 1;
		u64 thres                            : 6;
		u64 dat_null                         : 1;
		u64 sector                           : 1;
		u64 dma_val                          : 1;
		u64 bus_id                           : 2;
		u64 skip_busy                        : 1;
		u64 extra_args                       : 1;
	} s;
	struct mio_emm_dma_cn8 {
		u64 card_addr                        : 32;
		u64 block_cnt                        : 16;
		u64 multi                            : 1;
		u64 rw                               : 1;
		u64 rel_wr                           : 1;
		u64 thres                            : 6;
		u64 dat_null                         : 1;
		u64 sector                           : 1;
		u64 dma_val                          : 1;
		u64 bus_id                           : 2;
		u64 skip_busy                        : 1;
		u64 reserved_63                      : 1;
	} cn8;
	struct mio_emm_dma_cn9 {
		u64 card_addr                        : 32;
		u64 block_cnt                        : 16;
		u64 multi                            : 1;
		u64 rw                               : 1;
		u64 reserved_50                      : 1;
		u64 thres                            : 6;
		u64 dat_null                         : 1;
		u64 sector                           : 1;
		u64 dma_val                          : 1;
		u64 bus_id                           : 2;
		u64 skip_busy                        : 1;
		u64 extra_args                       : 1;
	} cn9;
};

static inline u64 MIO_EMM_DMA(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_DMA(void)
{
	return 0x2050;
}

/**
 * Register (RSL) mio_emm_dma_adr
 *
 * eMMC DMA Address Register This register sets the address for eMMC/SD
 * flash transfers to/from memory. Sixty-four-bit operations must be used
 * to access this register. This register is updated by the DMA hardware
 * and can be reloaded by the values placed in the MIO_EMM_DMA_FIFO_ADR.
 */
union mio_emm_dma_adr {
	u64 u;
	struct mio_emm_dma_adr_s {
		u64 adr                              : 53;
		u64 reserved_53_63                   : 11;
	} s;
	struct mio_emm_dma_adr_cn8 {
		u64 adr                              : 49;
		u64 reserved_49_63                   : 15;
	} cn8;
	/* struct mio_emm_dma_adr_s cn9; */
};

static inline u64 MIO_EMM_DMA_ADR(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_DMA_ADR(void)
{
	return 0x188;
}

/**
 * Register (RSL) mio_emm_dma_arg
 *
 * eMMC External DMA Extra Arguments Register
 */
union mio_emm_dma_arg {
	u64 u;
	struct mio_emm_dma_arg_s {
		u64 cmd23_args                       : 8;
		u64 force_pgm                        : 1;
		u64 context_id                       : 4;
		u64 tag_req                          : 1;
		u64 pack_cmd                         : 1;
		u64 rel_wr                           : 1;
		u64 alt_cmd                          : 6;
		u64 skip_blk_cmd                     : 1;
		u64 reserved_23_31                   : 9;
		u64 alt_cmd_arg                      : 32;
	} s;
	/* struct mio_emm_dma_arg_s cn; */
};

static inline u64 MIO_EMM_DMA_ARG(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_DMA_ARG(void)
{
	return 0x2090;
}

/**
 * Register (RSL) mio_emm_dma_cfg
 *
 * eMMC DMA Configuration Register This register controls the internal
 * DMA engine used with the eMMC/SD flash controller. Sixty- four-bit
 * operations must be used to access this register. This register is
 * updated by the hardware DMA engine and can also be reloaded by writes
 * to the MIO_EMM_DMA_FIFO_CMD register.
 */
union mio_emm_dma_cfg {
	u64 u;
	struct mio_emm_dma_cfg_s {
		u64 reserved_0_35                    : 36;
		u64 size                             : 20;
		u64 endian                           : 1;
		u64 swap8                            : 1;
		u64 swap16                           : 1;
		u64 swap32                           : 1;
		u64 reserved_60                      : 1;
		u64 clr                              : 1;
		u64 rw                               : 1;
		u64 en                               : 1;
	} s;
	/* struct mio_emm_dma_cfg_s cn; */
};

static inline u64 MIO_EMM_DMA_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_DMA_CFG(void)
{
	return 0x180;
}

/**
 * Register (RSL) mio_emm_dma_fifo_adr
 *
 * eMMC Internal DMA FIFO Address Register This register specifies the
 * internal address that is loaded into the eMMC internal DMA FIFO. The
 * FIFO is used to queue up operations for the
 * MIO_EMM_DMA_CFG/MIO_EMM_DMA_ADR when the DMA completes successfully.
 */
union mio_emm_dma_fifo_adr {
	u64 u;
	struct mio_emm_dma_fifo_adr_s {
		u64 reserved_0_2                     : 3;
		u64 adr                              : 50;
		u64 reserved_53_63                   : 11;
	} s;
	struct mio_emm_dma_fifo_adr_cn8 {
		u64 reserved_0_2                     : 3;
		u64 adr                              : 46;
		u64 reserved_49_63                   : 15;
	} cn8;
	/* struct mio_emm_dma_fifo_adr_s cn9; */
};

static inline u64 MIO_EMM_DMA_FIFO_ADR(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_DMA_FIFO_ADR(void)
{
	return 0x170;
}

/**
 * Register (RSL) mio_emm_dma_fifo_cfg
 *
 * eMMC Internal DMA FIFO Configuration Register This register controls
 * DMA FIFO operations.
 */
union mio_emm_dma_fifo_cfg {
	u64 u;
	struct mio_emm_dma_fifo_cfg_s {
		u64 count                            : 5;
		u64 reserved_5_7                     : 3;
		u64 int_lvl                          : 5;
		u64 reserved_13_15                   : 3;
		u64 clr                              : 1;
		u64 reserved_17_63                   : 47;
	} s;
	/* struct mio_emm_dma_fifo_cfg_s cn; */
};

static inline u64 MIO_EMM_DMA_FIFO_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_DMA_FIFO_CFG(void)
{
	return 0x160;
}

/**
 * Register (RSL) mio_emm_dma_fifo_cmd
 *
 * eMMC Internal DMA FIFO Command Register This register specifies a
 * command that is loaded into the eMMC internal DMA FIFO.  The FIFO is
 * used to queue up operations for the MIO_EMM_DMA_CFG/MIO_EMM_DMA_ADR
 * when the DMA completes successfully. Writes to this register store
 * both the MIO_EMM_DMA_FIFO_CMD and the MIO_EMM_DMA_FIFO_ADR contents
 * into the FIFO and increment the MIO_EMM_DMA_FIFO_CFG[COUNT] field.
 * Note: This register has a similar format to MIO_EMM_DMA_CFG with the
 * exception that the EN and CLR fields are absent. These are supported
 * in MIO_EMM_DMA_FIFO_CFG.
 */
union mio_emm_dma_fifo_cmd {
	u64 u;
	struct mio_emm_dma_fifo_cmd_s {
		u64 reserved_0_35                    : 36;
		u64 size                             : 20;
		u64 endian                           : 1;
		u64 swap8                            : 1;
		u64 swap16                           : 1;
		u64 swap32                           : 1;
		u64 intdis                           : 1;
		u64 reserved_61                      : 1;
		u64 rw                               : 1;
		u64 reserved_63                      : 1;
	} s;
	/* struct mio_emm_dma_fifo_cmd_s cn; */
};

static inline u64 MIO_EMM_DMA_FIFO_CMD(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_DMA_FIFO_CMD(void)
{
	return 0x178;
}

/**
 * Register (RSL) mio_emm_dma_int
 *
 * eMMC DMA Interrupt Register Sixty-four-bit operations must be used to
 * access this register.
 */
union mio_emm_dma_int {
	u64 u;
	struct mio_emm_dma_int_s {
		u64 done                             : 1;
		u64 fifo                             : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct mio_emm_dma_int_s cn; */
};

static inline u64 MIO_EMM_DMA_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_DMA_INT(void)
{
	return 0x190;
}

/**
 * Register (RSL) mio_emm_dma_int_ena_w1c
 *
 * eMMC DMA Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union mio_emm_dma_int_ena_w1c {
	u64 u;
	struct mio_emm_dma_int_ena_w1c_s {
		u64 done                             : 1;
		u64 fifo                             : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct mio_emm_dma_int_ena_w1c_s cn; */
};

static inline u64 MIO_EMM_DMA_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_DMA_INT_ENA_W1C(void)
{
	return 0x1a8;
}

/**
 * Register (RSL) mio_emm_dma_int_ena_w1s
 *
 * eMMC DMA Interrupt Enable Set Register This register sets interrupt
 * enable bits.
 */
union mio_emm_dma_int_ena_w1s {
	u64 u;
	struct mio_emm_dma_int_ena_w1s_s {
		u64 done                             : 1;
		u64 fifo                             : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct mio_emm_dma_int_ena_w1s_s cn; */
};

static inline u64 MIO_EMM_DMA_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_DMA_INT_ENA_W1S(void)
{
	return 0x1a0;
}

/**
 * Register (RSL) mio_emm_dma_int_w1s
 *
 * eMMC DMA Interrupt Set Register This register sets interrupt bits.
 */
union mio_emm_dma_int_w1s {
	u64 u;
	struct mio_emm_dma_int_w1s_s {
		u64 done                             : 1;
		u64 fifo                             : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct mio_emm_dma_int_w1s_s cn; */
};

static inline u64 MIO_EMM_DMA_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_DMA_INT_W1S(void)
{
	return 0x198;
}

/**
 * Register (RSL) mio_emm_int
 *
 * eMMC Interrupt Register
 */
union mio_emm_int {
	u64 u;
	struct mio_emm_int_s {
		u64 buf_done                         : 1;
		u64 cmd_done                         : 1;
		u64 dma_done                         : 1;
		u64 cmd_err                          : 1;
		u64 dma_err                          : 1;
		u64 switch_done                      : 1;
		u64 switch_err                       : 1;
		u64 ncb_flt                          : 1;
		u64 ncb_ras                          : 1;
		u64 reserved_9_63                    : 55;
	} s;
	struct mio_emm_int_cn8 {
		u64 buf_done                         : 1;
		u64 cmd_done                         : 1;
		u64 dma_done                         : 1;
		u64 cmd_err                          : 1;
		u64 dma_err                          : 1;
		u64 switch_done                      : 1;
		u64 switch_err                       : 1;
		u64 reserved_7_63                    : 57;
	} cn8;
	/* struct mio_emm_int_s cn9; */
};

static inline u64 MIO_EMM_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_INT(void)
{
	return 0x2078;
}

/**
 * Register (RSL) mio_emm_int_ena_w1c
 *
 * eMMC Interrupt Enable Clear Register This register clears interrupt
 * enable bits.
 */
union mio_emm_int_ena_w1c {
	u64 u;
	struct mio_emm_int_ena_w1c_s {
		u64 buf_done                         : 1;
		u64 cmd_done                         : 1;
		u64 dma_done                         : 1;
		u64 cmd_err                          : 1;
		u64 dma_err                          : 1;
		u64 switch_done                      : 1;
		u64 switch_err                       : 1;
		u64 ncb_flt                          : 1;
		u64 ncb_ras                          : 1;
		u64 reserved_9_63                    : 55;
	} s;
	struct mio_emm_int_ena_w1c_cn8 {
		u64 buf_done                         : 1;
		u64 cmd_done                         : 1;
		u64 dma_done                         : 1;
		u64 cmd_err                          : 1;
		u64 dma_err                          : 1;
		u64 switch_done                      : 1;
		u64 switch_err                       : 1;
		u64 reserved_7_63                    : 57;
	} cn8;
	/* struct mio_emm_int_ena_w1c_s cn9; */
};

static inline u64 MIO_EMM_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_INT_ENA_W1C(void)
{
	return 0x20b8;
}

/**
 * Register (RSL) mio_emm_int_ena_w1s
 *
 * eMMC Interrupt Enable Set Register This register sets interrupt enable
 * bits.
 */
union mio_emm_int_ena_w1s {
	u64 u;
	struct mio_emm_int_ena_w1s_s {
		u64 buf_done                         : 1;
		u64 cmd_done                         : 1;
		u64 dma_done                         : 1;
		u64 cmd_err                          : 1;
		u64 dma_err                          : 1;
		u64 switch_done                      : 1;
		u64 switch_err                       : 1;
		u64 ncb_flt                          : 1;
		u64 ncb_ras                          : 1;
		u64 reserved_9_63                    : 55;
	} s;
	struct mio_emm_int_ena_w1s_cn8 {
		u64 buf_done                         : 1;
		u64 cmd_done                         : 1;
		u64 dma_done                         : 1;
		u64 cmd_err                          : 1;
		u64 dma_err                          : 1;
		u64 switch_done                      : 1;
		u64 switch_err                       : 1;
		u64 reserved_7_63                    : 57;
	} cn8;
	/* struct mio_emm_int_ena_w1s_s cn9; */
};

static inline u64 MIO_EMM_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_INT_ENA_W1S(void)
{
	return 0x20b0;
}

/**
 * Register (RSL) mio_emm_int_w1s
 *
 * eMMC Interrupt Set Register This register sets interrupt bits.
 */
union mio_emm_int_w1s {
	u64 u;
	struct mio_emm_int_w1s_s {
		u64 buf_done                         : 1;
		u64 cmd_done                         : 1;
		u64 dma_done                         : 1;
		u64 cmd_err                          : 1;
		u64 dma_err                          : 1;
		u64 switch_done                      : 1;
		u64 switch_err                       : 1;
		u64 ncb_flt                          : 1;
		u64 ncb_ras                          : 1;
		u64 reserved_9_63                    : 55;
	} s;
	struct mio_emm_int_w1s_cn8 {
		u64 buf_done                         : 1;
		u64 cmd_done                         : 1;
		u64 dma_done                         : 1;
		u64 cmd_err                          : 1;
		u64 dma_err                          : 1;
		u64 switch_done                      : 1;
		u64 switch_err                       : 1;
		u64 reserved_7_63                    : 57;
	} cn8;
	/* struct mio_emm_int_w1s_s cn9; */
};

static inline u64 MIO_EMM_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_INT_W1S(void)
{
	return 0x2080;
}

/**
 * Register (RSL) mio_emm_io_ctl
 *
 * eMMC I/O Control Register
 */
union mio_emm_io_ctl {
	u64 u;
	struct mio_emm_io_ctl_s {
		u64 slew                             : 1;
		u64 reserved_1                       : 1;
		u64 drive                            : 2;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct mio_emm_io_ctl_s cn; */
};

static inline u64 MIO_EMM_IO_CTL(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_IO_CTL(void)
{
	return 0x2040;
}

/**
 * Register (RSL) mio_emm_mode#
 *
 * eMMC Operating Mode Register
 */
union mio_emm_modex {
	u64 u;
	struct mio_emm_modex_s {
		u64 clk_lo                           : 16;
		u64 clk_hi                           : 16;
		u64 power_class                      : 4;
		u64 clk_swap                         : 1;
		u64 reserved_37_39                   : 3;
		u64 bus_width                        : 3;
		u64 reserved_43_47                   : 5;
		u64 hs_timing                        : 1;
		u64 hs200_timing                     : 1;
		u64 hs400_timing                     : 1;
		u64 reserved_51_63                   : 13;
	} s;
	struct mio_emm_modex_cn8 {
		u64 clk_lo                           : 16;
		u64 clk_hi                           : 16;
		u64 power_class                      : 4;
		u64 reserved_36_39                   : 4;
		u64 bus_width                        : 3;
		u64 reserved_43_47                   : 5;
		u64 hs_timing                        : 1;
		u64 reserved_49_63                   : 15;
	} cn8;
	struct mio_emm_modex_cn96xxp1 {
		u64 clk_lo                           : 16;
		u64 clk_hi                           : 16;
		u64 power_class                      : 4;
		u64 reserved_36_39                   : 4;
		u64 bus_width                        : 3;
		u64 reserved_43_47                   : 5;
		u64 hs_timing                        : 1;
		u64 hs200_timing                     : 1;
		u64 hs400_timing                     : 1;
		u64 reserved_51_63                   : 13;
	} cn96xxp1;
	/* struct mio_emm_modex_s cn96xxp3; */
	/* struct mio_emm_modex_s cnf95xx; */
};

static inline u64 MIO_EMM_MODEX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_MODEX(u64 a)
{
	return 0x2008 + 8 * a;
}

/**
 * Register (RSL) mio_emm_msix_pba#
 *
 * eMMC MSI-X Pending Bit Array Registers This register is the MSI-X PBA
 * table; the bit number is indexed by the MIO_EMM_INT_VEC_E enumeration.
 */
union mio_emm_msix_pbax {
	u64 u;
	struct mio_emm_msix_pbax_s {
		u64 pend                             : 64;
	} s;
	/* struct mio_emm_msix_pbax_s cn; */
};

static inline u64 MIO_EMM_MSIX_PBAX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_MSIX_PBAX(u64 a)
{
	return 0xf0000 + 8 * a;
}

/**
 * Register (RSL) mio_emm_msix_vec#_addr
 *
 * eMMC MSI-X Vector-Table Address Register This register is the MSI-X
 * vector table, indexed by the MIO_EMM_INT_VEC_E enumeration.
 */
union mio_emm_msix_vecx_addr {
	u64 u;
	struct mio_emm_msix_vecx_addr_s {
		u64 secvec                           : 1;
		u64 reserved_1                       : 1;
		u64 addr                             : 51;
		u64 reserved_53_63                   : 11;
	} s;
	struct mio_emm_msix_vecx_addr_cn8 {
		u64 secvec                           : 1;
		u64 reserved_1                       : 1;
		u64 addr                             : 47;
		u64 reserved_49_63                   : 15;
	} cn8;
	/* struct mio_emm_msix_vecx_addr_s cn9; */
};

static inline u64 MIO_EMM_MSIX_VECX_ADDR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_MSIX_VECX_ADDR(u64 a)
{
	return 0 + 0x10 * a;
}

/**
 * Register (RSL) mio_emm_msix_vec#_ctl
 *
 * eMMC MSI-X Vector-Table Control and Data Register This register is the
 * MSI-X vector table, indexed by the MIO_EMM_INT_VEC_E enumeration.
 */
union mio_emm_msix_vecx_ctl {
	u64 u;
	struct mio_emm_msix_vecx_ctl_s {
		u64 data                             : 32;
		u64 mask                             : 1;
		u64 reserved_33_63                   : 31;
	} s;
	struct mio_emm_msix_vecx_ctl_cn8 {
		u64 data                             : 20;
		u64 reserved_20_31                   : 12;
		u64 mask                             : 1;
		u64 reserved_33_63                   : 31;
	} cn8;
	/* struct mio_emm_msix_vecx_ctl_s cn9; */
};

static inline u64 MIO_EMM_MSIX_VECX_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_MSIX_VECX_CTL(u64 a)
{
	return 8 + 0x10 * a;
}

/**
 * Register (RSL) mio_emm_rca
 *
 * eMMC Relative Card Address Register
 */
union mio_emm_rca {
	u64 u;
	struct mio_emm_rca_s {
		u64 card_rca                         : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct mio_emm_rca_s cn; */
};

static inline u64 MIO_EMM_RCA(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_RCA(void)
{
	return 0x20a0;
}

/**
 * Register (RSL) mio_emm_rsp_hi
 *
 * eMMC Response Data High Register
 */
union mio_emm_rsp_hi {
	u64 u;
	struct mio_emm_rsp_hi_s {
		u64 dat                              : 64;
	} s;
	/* struct mio_emm_rsp_hi_s cn; */
};

static inline u64 MIO_EMM_RSP_HI(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_RSP_HI(void)
{
	return 0x2070;
}

/**
 * Register (RSL) mio_emm_rsp_lo
 *
 * eMMC Response Data Low Register
 */
union mio_emm_rsp_lo {
	u64 u;
	struct mio_emm_rsp_lo_s {
		u64 dat                              : 64;
	} s;
	/* struct mio_emm_rsp_lo_s cn; */
};

static inline u64 MIO_EMM_RSP_LO(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_RSP_LO(void)
{
	return 0x2068;
}

/**
 * Register (RSL) mio_emm_rsp_sts
 *
 * eMMC Response Status Register
 */
union mio_emm_rsp_sts {
	u64 u;
	struct mio_emm_rsp_sts_s {
		u64 cmd_done                         : 1;
		u64 cmd_idx                          : 6;
		u64 cmd_type                         : 2;
		u64 rsp_type                         : 3;
		u64 rsp_val                          : 1;
		u64 rsp_bad_sts                      : 1;
		u64 rsp_crc_err                      : 1;
		u64 rsp_timeout                      : 1;
		u64 stp_val                          : 1;
		u64 stp_bad_sts                      : 1;
		u64 stp_crc_err                      : 1;
		u64 stp_timeout                      : 1;
		u64 rsp_busybit                      : 1;
		u64 blk_crc_err                      : 1;
		u64 blk_timeout                      : 1;
		u64 dbuf                             : 1;
		u64 reserved_24_27                   : 4;
		u64 dbuf_err                         : 1;
		u64 reserved_29_54                   : 26;
		u64 acc_timeout                      : 1;
		u64 dma_pend                         : 1;
		u64 dma_val                          : 1;
		u64 switch_val                       : 1;
		u64 cmd_val                          : 1;
		u64 bus_id                           : 2;
		u64 reserved_62_63                   : 2;
	} s;
	/* struct mio_emm_rsp_sts_s cn; */
};

static inline u64 MIO_EMM_RSP_STS(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_RSP_STS(void)
{
	return 0x2060;
}

/**
 * Register (RSL) mio_emm_sample
 *
 * eMMC Sampling Register
 */
union mio_emm_sample {
	u64 u;
	struct mio_emm_sample_s {
		u64 dat_cnt                          : 10;
		u64 reserved_10_15                   : 6;
		u64 cmd_cnt                          : 10;
		u64 reserved_26_63                   : 38;
	} s;
	/* struct mio_emm_sample_s cn; */
};

static inline u64 MIO_EMM_SAMPLE(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_SAMPLE(void)
{
	return 0x2090;
}

/**
 * Register (RSL) mio_emm_sts_mask
 *
 * eMMC Status Mask Register
 */
union mio_emm_sts_mask {
	u64 u;
	struct mio_emm_sts_mask_s {
		u64 sts_msk                          : 32;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct mio_emm_sts_mask_s cn; */
};

static inline u64 MIO_EMM_STS_MASK(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_STS_MASK(void)
{
	return 0x2098;
}

/**
 * Register (RSL) mio_emm_switch
 *
 * eMMC Operating Mode Switch Register This register allows software to
 * change eMMC related parameters associated with a specific BUS_ID.  The
 * MIO_EMM_MODE() registers contain the current setting for each BUS.
 * This register is also used to switch the [CLK_HI] and [CLK_LO]
 * settings associated with the common EMMC_CLK.  These settings can only
 * be changed when [BUS_ID] = 0.
 */
union mio_emm_switch {
	u64 u;
	struct mio_emm_switch_s {
		u64 clk_lo                           : 16;
		u64 clk_hi                           : 16;
		u64 power_class                      : 4;
		u64 clk_swap                         : 1;
		u64 reserved_37_39                   : 3;
		u64 bus_width                        : 3;
		u64 reserved_43_47                   : 5;
		u64 hs_timing                        : 1;
		u64 hs200_timing                     : 1;
		u64 hs400_timing                     : 1;
		u64 reserved_51_55                   : 5;
		u64 switch_err2                      : 1;
		u64 switch_err1                      : 1;
		u64 switch_err0                      : 1;
		u64 switch_exe                       : 1;
		u64 bus_id                           : 2;
		u64 reserved_62_63                   : 2;
	} s;
	struct mio_emm_switch_cn8 {
		u64 clk_lo                           : 16;
		u64 clk_hi                           : 16;
		u64 power_class                      : 4;
		u64 reserved_36_39                   : 4;
		u64 bus_width                        : 3;
		u64 reserved_43_47                   : 5;
		u64 hs_timing                        : 1;
		u64 reserved_49_55                   : 7;
		u64 switch_err2                      : 1;
		u64 switch_err1                      : 1;
		u64 switch_err0                      : 1;
		u64 switch_exe                       : 1;
		u64 bus_id                           : 2;
		u64 reserved_62_63                   : 2;
	} cn8;
	struct mio_emm_switch_cn96xxp1 {
		u64 clk_lo                           : 16;
		u64 clk_hi                           : 16;
		u64 power_class                      : 4;
		u64 reserved_36_39                   : 4;
		u64 bus_width                        : 3;
		u64 reserved_43_47                   : 5;
		u64 hs_timing                        : 1;
		u64 hs200_timing                     : 1;
		u64 hs400_timing                     : 1;
		u64 reserved_51_55                   : 5;
		u64 switch_err2                      : 1;
		u64 switch_err1                      : 1;
		u64 switch_err0                      : 1;
		u64 switch_exe                       : 1;
		u64 bus_id                           : 2;
		u64 reserved_62_63                   : 2;
	} cn96xxp1;
	/* struct mio_emm_switch_s cn96xxp3; */
	/* struct mio_emm_switch_s cnf95xx; */
};

static inline u64 MIO_EMM_SWITCH(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_SWITCH(void)
{
	return 0x2048;
}

/**
 * Register (RSL) mio_emm_tap
 *
 * eMMC TAP Delay Register This register indicates the delay line
 * characteristics.
 */
union mio_emm_tap {
	u64 u;
	struct mio_emm_tap_s {
		u64 delay                            : 8;
		u64 reserved_8_63                    : 56;
	} s;
	/* struct mio_emm_tap_s cn; */
};

static inline u64 MIO_EMM_TAP(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_TAP(void)
{
	return 0x20c8;
}

/**
 * Register (RSL) mio_emm_timing
 *
 * eMMC Timing Register This register determines the number of tap delays
 * the EMM_DAT, EMM_DS, and EMM_CMD lines are transmitted or received in
 * relation to EMM_CLK. These values should only be changed when the eMMC
 * bus is idle.
 */
union mio_emm_timing {
	u64 u;
	struct mio_emm_timing_s {
		u64 data_out_tap                     : 6;
		u64 reserved_6_15                    : 10;
		u64 data_in_tap                      : 6;
		u64 reserved_22_31                   : 10;
		u64 cmd_out_tap                      : 6;
		u64 reserved_38_47                   : 10;
		u64 cmd_in_tap                       : 6;
		u64 reserved_54_63                   : 10;
	} s;
	/* struct mio_emm_timing_s cn; */
};

static inline u64 MIO_EMM_TIMING(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_TIMING(void)
{
	return 0x20d0;
}

/**
 * Register (RSL) mio_emm_wdog
 *
 * eMMC Watchdog Register
 */
union mio_emm_wdog {
	u64 u;
	struct mio_emm_wdog_s {
		u64 clk_cnt                          : 26;
		u64 reserved_26_63                   : 38;
	} s;
	/* struct mio_emm_wdog_s cn; */
};

static inline u64 MIO_EMM_WDOG(void)
	__attribute__ ((pure, always_inline));
static inline u64 MIO_EMM_WDOG(void)
{
	return 0x2088;
}

#endif /* __CSRS_MIO_EMM_H__ */
