/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CVMX_MIO_EMM_DEFS_H__
#define __CVMX_MIO_EMM_DEFS_H__

static inline u64 MIO_EMM_DMA_FIFO_CFG(void)
{
	return 0x160;
}

static inline u64 MIO_EMM_DMA_FIFO_ADR(void)
{
	return 0x170;
}

static inline u64 MIO_EMM_DMA_FIFO_CMD(void)
{
	return 0x178;
}

static inline u64 MIO_EMM_DMA_CFG(void)
{
	return 0x180;
}

static inline u64 MIO_EMM_DMA_ADR(void)
{
	return 0x188;
}

static inline u64 MIO_EMM_DMA_INT(void)
{
	return 0x190;
}

static inline u64 MIO_EMM_CFG(void)
{
	return 0x2000;
}

static inline u64 MIO_EMM_MODEX(u64 a)
{
	return 0x2008 + 8 * a;
}

static inline u64 MIO_EMM_SWITCH(void)
{
	return 0x2048;
}

static inline u64 MIO_EMM_DMA(void)
{
	return 0x2050;
}

static inline u64 MIO_EMM_CMD(void)
{
	return 0x2058;
}

static inline u64 MIO_EMM_RSP_STS(void)
{
	return 0x2060;
}

static inline u64 MIO_EMM_RSP_LO(void)
{
	return 0x2068;
}

static inline u64 MIO_EMM_RSP_HI(void)
{
	return 0x2070;
}

static inline u64 MIO_EMM_INT(void)
{
	return 0x2078;
}

static inline u64 MIO_EMM_WDOG(void)
{
	return 0x2088;
}

static inline u64 MIO_EMM_SAMPLE(void)
{
	return 0x2090;
}

static inline u64 MIO_EMM_STS_MASK(void)
{
	return 0x2098;
}

static inline u64 MIO_EMM_RCA(void)
{
	return 0x20a0;
}

static inline u64 MIO_EMM_BUF_IDX(void)
{
	return 0x20e0;
}

static inline u64 MIO_EMM_BUF_DAT(void)
{
	return 0x20e8;
}

/* Dummy implementation, not documented on MIPS Octeon */
static inline u64 MIO_EMM_DEBUG(void)
{
	return 0x20f8;
}

/**
 * mio_emm_access_wdog
 */
union mio_emm_access_wdog {
	u64 u;
	struct mio_emm_access_wdog_s {
		uint64_t reserved_32_63 : 32;
		uint64_t clk_cnt : 32;
	} s;
};

/**
 * mio_emm_buf_dat
 *
 * MIO_EMM_BUF_DAT = MIO EMMC Data buffer access Register
 *
 */
union mio_emm_buf_dat {
	u64 u;
	struct mio_emm_buf_dat_s {
		uint64_t dat : 64;
	} s;
};

/**
 * mio_emm_buf_idx
 *
 * MIO_EMM_BUF_IDX = MIO EMMC Data buffer address Register
 *
 */
union mio_emm_buf_idx {
	u64 u;
	struct mio_emm_buf_idx_s {
		uint64_t reserved_17_63 : 47;
		uint64_t inc : 1;
		uint64_t reserved_7_15 : 9;
		uint64_t buf_num : 1;
		uint64_t offset : 6;
	} s;
};

/**
 * mio_emm_cfg
 *
 * MIO_EMM_CFG = MIO EMMC Configuration Register
 *
 */
union mio_emm_cfg {
	u64 u;
	struct mio_emm_cfg_s {
		uint64_t reserved_17_63 : 47;
		uint64_t boot_fail : 1;
		uint64_t reserved_4_15 : 12;
		uint64_t bus_ena : 4;
	} s;
};

/**
 * mio_emm_cmd
 *
 * MIO_EMM_CMD = MIO EMMC Command Register
 *
 */
union mio_emm_cmd {
	u64 u;
	struct mio_emm_cmd_s {
		uint64_t reserved_63_63 : 1;
		uint64_t skip_busy : 1;
		uint64_t bus_id : 2;
		uint64_t cmd_val : 1;
		uint64_t reserved_56_58 : 3;
		uint64_t dbuf : 1;
		uint64_t offset : 6;
		uint64_t reserved_43_48 : 6;
		uint64_t ctype_xor : 2;
		uint64_t rtype_xor : 3;
		uint64_t cmd_idx : 6;
		uint64_t arg : 32;
	} s;
};

/**
 * mio_emm_dma
 *
 * MIO_EMM_DMA = MIO EMMC DMA config Register
 *
 */
union mio_emm_dma {
	u64 u;
	struct mio_emm_dma_s {
		uint64_t reserved_63_63 : 1;
		uint64_t skip_busy : 1;
		uint64_t bus_id : 2;
		uint64_t dma_val : 1;
		uint64_t sector : 1;
		uint64_t dat_null : 1;
		uint64_t thres : 6;
		uint64_t rel_wr : 1;
		uint64_t rw : 1;
		uint64_t multi : 1;
		uint64_t block_cnt : 16;
		uint64_t card_addr : 32;
	} s;
};

/**
 * mio_emm_dma_adr
 *
 * This register sets the address for eMMC/SD flash transfers to/from memory. Sixty-four-bit
 * operations must be used to access this register. This register is updated by the DMA
 * hardware and can be reloaded by the values placed in the MIO_EMM_DMA_FIFO_ADR.
 */
union mio_emm_dma_adr {
	u64 u;
	struct mio_emm_dma_adr_s {
		uint64_t reserved_42_63 : 22;
		uint64_t adr : 42;
	} s;
};

/**
 * mio_emm_dma_cfg
 *
 * This register controls the internal DMA engine used with the eMMC/SD flash controller. Sixty-
 * four-bit operations must be used to access this register. This register is updated by the
 * hardware DMA engine and can also be reloaded by writes to the MIO_EMM_DMA_FIFO_CMD register.
 */
union mio_emm_dma_cfg {
	u64 u;
	struct mio_emm_dma_cfg_s {
		uint64_t en : 1;
		uint64_t rw : 1;
		uint64_t clr : 1;
		uint64_t reserved_60_60 : 1;
		uint64_t swap32 : 1;
		uint64_t swap16 : 1;
		uint64_t swap8 : 1;
		uint64_t endian : 1;
		uint64_t size : 20;
		uint64_t reserved_0_35 : 36;
	} s;
};

/**
 * mio_emm_dma_fifo_adr
 *
 * This register specifies the internal address that is loaded into the eMMC internal DMA FIFO.
 * The FIFO is used to queue up operations for the MIO_EMM_DMA_CFG/MIO_EMM_DMA_ADR when the DMA
 * completes successfully.
 */
union mio_emm_dma_fifo_adr {
	u64 u;
	struct mio_emm_dma_fifo_adr_s {
		uint64_t reserved_42_63 : 22;
		uint64_t adr : 39;
		uint64_t reserved_0_2 : 3;
	} s;
};

/**
 * mio_emm_dma_fifo_cfg
 *
 * This register controls DMA FIFO operations.
 *
 */
union mio_emm_dma_fifo_cfg {
	u64 u;
	struct mio_emm_dma_fifo_cfg_s {
		uint64_t reserved_17_63 : 47;
		uint64_t clr : 1;
		uint64_t reserved_13_15 : 3;
		uint64_t int_lvl : 5;
		uint64_t reserved_5_7 : 3;
		uint64_t count : 5;
	} s;
};

/**
 * mio_emm_dma_fifo_cmd
 *
 * This register specifies a command that is loaded into the eMMC internal DMA FIFO.  The FIFO is
 * used to queue up operations for the MIO_EMM_DMA_CFG/MIO_EMM_DMA_ADR when the DMA completes
 * successfully. Writes to this register store both the MIO_EMM_DMA_FIFO_CMD and the
 * MIO_EMM_DMA_FIFO_ADR contents into the FIFO and increment the MIO_EMM_DMA_FIFO_CFG[COUNT]
 * field.
 *
 * Note: This register has a similar format to MIO_EMM_DMA_CFG with the exception
 * that the EN and CLR fields are absent. These are supported in MIO_EMM_DMA_FIFO_CFG.
 */
union mio_emm_dma_fifo_cmd {
	u64 u;
	struct mio_emm_dma_fifo_cmd_s {
		uint64_t reserved_63_63 : 1;
		uint64_t rw : 1;
		uint64_t reserved_61_61 : 1;
		uint64_t intdis : 1;
		uint64_t swap32 : 1;
		uint64_t swap16 : 1;
		uint64_t swap8 : 1;
		uint64_t endian : 1;
		uint64_t size : 20;
		uint64_t reserved_0_35 : 36;
	} s;
};

/**
 * mio_emm_dma_int
 *
 * Sixty-four-bit operations must be used to access this register.
 *
 */
union mio_emm_dma_int {
	u64 u;
	struct mio_emm_dma_int_s {
		uint64_t reserved_2_63 : 62;
		uint64_t fifo : 1;
		uint64_t done : 1;
	} s;
};

/**
 * mio_emm_dma_int_w1s
 */
union mio_emm_dma_int_w1s {
	u64 u;
	struct mio_emm_dma_int_w1s_s {
		uint64_t reserved_2_63 : 62;
		uint64_t fifo : 1;
		uint64_t done : 1;
	} s;
};

/**
 * mio_emm_int
 *
 * MIO_EMM_INT = MIO EMMC Interrupt Register
 *
 */
union mio_emm_int {
	u64 u;
	struct mio_emm_int_s {
		uint64_t reserved_7_63 : 57;
		uint64_t switch_err : 1;
		uint64_t switch_done : 1;
		uint64_t dma_err : 1;
		uint64_t cmd_err : 1;
		uint64_t dma_done : 1;
		uint64_t cmd_done : 1;
		uint64_t buf_done : 1;
	} s;
};

/**
 * mio_emm_int_en
 *
 * MIO_EMM_INT_EN = MIO EMMC Interrupt enable Register
 *
 */
union mio_emm_int_en {
	u64 u;
	struct mio_emm_int_en_s {
		uint64_t reserved_7_63 : 57;
		uint64_t switch_err : 1;
		uint64_t switch_done : 1;
		uint64_t dma_err : 1;
		uint64_t cmd_err : 1;
		uint64_t dma_done : 1;
		uint64_t cmd_done : 1;
		uint64_t buf_done : 1;
	} s;
};

/**
 * mio_emm_int_w1s
 */
union mio_emm_int_w1s {
	u64 u;
	struct mio_emm_int_w1s_s {
		uint64_t reserved_7_63 : 57;
		uint64_t switch_err : 1;
		uint64_t switch_done : 1;
		uint64_t dma_err : 1;
		uint64_t cmd_err : 1;
		uint64_t dma_done : 1;
		uint64_t cmd_done : 1;
		uint64_t buf_done : 1;
	} s;
};

/**
 * mio_emm_mode#
 *
 * MIO_EMM_MODE = MIO EMMC Operating mode Register
 *
 */
union mio_emm_modex {
	u64 u;
	struct mio_emm_modex_s {
		uint64_t reserved_49_63 : 15;
		uint64_t hs_timing : 1;
		uint64_t reserved_43_47 : 5;
		uint64_t bus_width : 3;
		uint64_t reserved_36_39 : 4;
		uint64_t power_class : 4;
		uint64_t clk_hi : 16;
		uint64_t clk_lo : 16;
	} s;
};

/**
 * mio_emm_rca
 */
union mio_emm_rca {
	u64 u;
	struct mio_emm_rca_s {
		uint64_t reserved_16_63 : 48;
		uint64_t card_rca : 16;
	} s;
};

/**
 * mio_emm_rsp_hi
 *
 * MIO_EMM_RSP_HI = MIO EMMC Response data high Register
 *
 */
union mio_emm_rsp_hi {
	u64 u;
	struct mio_emm_rsp_hi_s {
		uint64_t dat : 64;
	} s;
};

/**
 * mio_emm_rsp_lo
 *
 * MIO_EMM_RSP_LO = MIO EMMC Response data low Register
 *
 */
union mio_emm_rsp_lo {
	u64 u;
	struct mio_emm_rsp_lo_s {
		uint64_t dat : 64;
	} s;
};

/**
 * mio_emm_rsp_sts
 *
 * MIO_EMM_RSP_STS = MIO EMMC Response status Register
 *
 */
union mio_emm_rsp_sts {
	u64 u;
	struct mio_emm_rsp_sts_s {
		uint64_t reserved_62_63 : 2;
		uint64_t bus_id : 2;
		uint64_t cmd_val : 1;
		uint64_t switch_val : 1;
		uint64_t dma_val : 1;
		uint64_t dma_pend : 1;
		uint64_t acc_timeout : 1;
		uint64_t reserved_29_54 : 26;
		uint64_t dbuf_err : 1;
		uint64_t reserved_24_27 : 4;
		uint64_t dbuf : 1;
		uint64_t blk_timeout : 1;
		uint64_t blk_crc_err : 1;
		uint64_t rsp_busybit : 1;
		uint64_t stp_timeout : 1;
		uint64_t stp_crc_err : 1;
		uint64_t stp_bad_sts : 1;
		uint64_t stp_val : 1;
		uint64_t rsp_timeout : 1;
		uint64_t rsp_crc_err : 1;
		uint64_t rsp_bad_sts : 1;
		uint64_t rsp_val : 1;
		uint64_t rsp_type : 3;
		uint64_t cmd_type : 2;
		uint64_t cmd_idx : 6;
		uint64_t cmd_done : 1;
	} s;
};

/**
 * mio_emm_sample
 */
union mio_emm_sample {
	u64 u;
	struct mio_emm_sample_s {
		uint64_t reserved_26_63 : 38;
		uint64_t cmd_cnt : 10;
		uint64_t reserved_10_15 : 6;
		uint64_t dat_cnt : 10;
	} s;
};

/**
 * mio_emm_sts_mask
 */
union mio_emm_sts_mask {
	u64 u;
	struct mio_emm_sts_mask_s {
		uint64_t reserved_32_63 : 32;
		uint64_t sts_msk : 32;
	} s;
};

/**
 * mio_emm_switch
 *
 * MIO_EMM_SWITCH = MIO EMMC Operating mode switch Register
 *
 */
union mio_emm_switch {
	u64 u;
	struct mio_emm_switch_s {
		uint64_t reserved_62_63 : 2;
		uint64_t bus_id : 2;
		uint64_t switch_exe : 1;
		uint64_t switch_err0 : 1;
		uint64_t switch_err1 : 1;
		uint64_t switch_err2 : 1;
		uint64_t reserved_49_55 : 7;
		uint64_t hs_timing : 1;
		uint64_t reserved_43_47 : 5;
		uint64_t bus_width : 3;
		uint64_t reserved_36_39 : 4;
		uint64_t power_class : 4;
		uint64_t clk_hi : 16;
		uint64_t clk_lo : 16;
	} s;
};

/**
 * mio_emm_wdog
 *
 * MIO_EMM_WDOG = MIO EMMC Watchdog Register
 *
 */
union mio_emm_wdog {
	u64 u;
	struct mio_emm_wdog_s {
		uint64_t reserved_26_63 : 38;
		uint64_t clk_cnt : 26;
	} s;
};

/*
 * The following structs are only available to enable compilation of the common
 * MMC driver. These registers do not exist on MIPS Octeon.
 */

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
};

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
};

#endif
