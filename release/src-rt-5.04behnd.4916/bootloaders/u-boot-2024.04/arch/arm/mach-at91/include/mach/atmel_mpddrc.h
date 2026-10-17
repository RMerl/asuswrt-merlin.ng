/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
 *
 * Copyright (C) 2015 Atmel Corporation
 *		      Wenyou Yang <wenyou.yang@atmel.com>
 */

#ifndef __ATMEL_MPDDRC_H__
#define __ATMEL_MPDDRC_H__

struct atmel_mpddrc_config {
	u32 mr;
	u32 rtr;
	u32 cr;
	u32 tpr0;
	u32 tpr1;
	u32 tpr2;
	u32 md;
	u32 lpddr23_lpr;
	u32 cal_mr4;
	u32 tim_cal;
};

/*
 * Only define the needed register in mpddr
 * If other register needed, will add them later
 */
struct atmel_mpddr {
	u32 mr;			/* 0x00: Mode Register */
	u32 rtr;		/* 0x04: Refresh Timer Register */
	u32 cr;			/* 0x08: Configuration Register */
	u32 tpr0;		/* 0x0c: Timing Parameter 0 Register */
	u32 tpr1;		/* 0x10: Timing Parameter 1 Register */
	u32 tpr2;		/* 0x14: Timing Parameter 2 Register */
	u32 reserved;		/* 0x18: Reserved */
	u32 lpr;		/* 0x1c: Low-power Register */
	u32 md;			/* 0x20: Memory Device Register */
	u32 reserved1;		/* 0x24: Reserved */
	u32 lpddr23_lpr;	/* 0x28: LPDDR2-LPDDR3 Low-power Register*/
	u32 cal_mr4;		/* 0x2c: Calibration and MR4 Register */
	u32 tim_cal;		/* 0x30: Timing Calibration Register */
	u32 io_calibr;		/* 0x34: IO Calibration */
	u32 ocms;		/* 0x38: OCMS Register */
	u32 ocms_key1;		/* 0x3c: OCMS KEY1 Register */
	u32 ocms_key2;		/* 0x40: OCMS KEY2 Register */
	u32 conf_arbiter;	/* 0x44: Configuration Arbiter Register */
	u32 timeout;		/* 0x48: Timeout Port 0/1/2/3 Register */
	u32 req_port0123;	/* 0x4c: Request Port 0/1/2/3 Register */
	u32 req_port4567;	/* 0x50: Request Port 4/5/6/7 Register */
	u32 bdw_port0123;	/* 0x54: Bandwidth Port 0/1/2/3 Register */
	u32 bdw_port4567;	/* 0x58: Bandwidth Port 4/5/6/7 Register */
	u32 rd_data_path;	/* 0x5c: Read Datapath Register */
	u32 reserved2[33];
	u32 wpmr;		/* 0xe4: Write Protection Mode Register */
	u32 wpsr;		/* 0xe8: Write Protection Status Register */
	u32 reserved3[4];
	u32 version;		/* 0xfc: IP version */
};


int ddr2_init(const unsigned int base,
	      const unsigned int ram_address,
	      const struct atmel_mpddrc_config *mpddr_value);

int lpddr2_init(const unsigned int base,
		const unsigned int ram_address,
		const struct atmel_mpddrc_config *mpddr_value);

int ddr3_init(const unsigned int base,
	      const unsigned int ram_address,
	      const struct atmel_mpddrc_config *mpddr_value);

/* Bit field in mode register */
#define ATMEL_MPDDRC_MR_MODE_NORMAL_CMD		0x0
#define ATMEL_MPDDRC_MR_MODE_NOP_CMD		0x1
#define ATMEL_MPDDRC_MR_MODE_PRCGALL_CMD	0x2
#define ATMEL_MPDDRC_MR_MODE_LMR_CMD		0x3
#define ATMEL_MPDDRC_MR_MODE_RFSH_CMD		0x4
#define ATMEL_MPDDRC_MR_MODE_EXT_LMR_CMD	0x5
#define ATMEL_MPDDRC_MR_MODE_DEEP_CMD		0x6
#define ATMEL_MPDDRC_MR_MODE_LPDDR2_CMD		0x7
#define ATMEL_MPDDRC_MR_MRS(v)			(((v) & 0xFF) << 0x8)

/* Bit field in refresh timer register */
#define ATMEL_MPDDRC_RTR_ADJ_REF		(0x1 << 16)
#define ATMEL_MPDDRC_RTR_MR4VALUE(v)		(((v) & 0x7) << 20)

/* Bit field in configuration register */
#define ATMEL_MPDDRC_CR_NC_MASK			0x3
#define ATMEL_MPDDRC_CR_NC_COL_9		0x0
#define ATMEL_MPDDRC_CR_NC_COL_10		0x1
#define ATMEL_MPDDRC_CR_NC_COL_11		0x2
#define ATMEL_MPDDRC_CR_NC_COL_12		0x3
#define ATMEL_MPDDRC_CR_NR_MASK			(0x3 << 2)
#define ATMEL_MPDDRC_CR_NR_ROW_11		(0x0 << 2)
#define ATMEL_MPDDRC_CR_NR_ROW_12		(0x1 << 2)
#define ATMEL_MPDDRC_CR_NR_ROW_13		(0x2 << 2)
#define ATMEL_MPDDRC_CR_NR_ROW_14		(0x3 << 2)
#define ATMEL_MPDDRC_CR_CAS_MASK		(0x7 << 4)
#define ATMEL_MPDDRC_CR_CAS_DDR_CAS2		(0x2 << 4)
#define ATMEL_MPDDRC_CR_CAS_DDR_CAS3		(0x3 << 4)
#define ATMEL_MPDDRC_CR_CAS_DDR_CAS4		(0x4 << 4)
#define ATMEL_MPDDRC_CR_CAS_DDR_CAS5		(0x5 << 4)
#define ATMEL_MPDDRC_CR_CAS_DDR_CAS6		(0x6 << 4)
#define ATMEL_MPDDRC_CR_DLL_RESET_ENABLED	(0x1 << 7)
#define ATMEL_MPDDRC_CR_DIC_DS			(0x1 << 8)
#define ATMEL_MPDDRC_CR_DIS_DLL			(0x1 << 9)
#define ATMEL_MPDDRC_CR_ZQ_INIT			(0x0 << 10)
#define ATMEL_MPDDRC_CR_ZQ_LONG			(0x1 << 10)
#define ATMEL_MPDDRC_CR_ZQ_SHORT		(0x2 << 10)
#define ATMEL_MPDDRC_CR_ZQ_RESET		(0x3 << 10)
#define ATMEL_MPDDRC_CR_OCD_DEFAULT		(0x7 << 12)
#define ATMEL_MPDDRC_CR_DQMS_SHARED		(0x1 << 16)
#define ATMEL_MPDDRC_CR_ENRDM_ON		(0x1 << 17)
#define ATMEL_MPDDRC_CR_NB_8BANKS		(0x1 << 20)
#define ATMEL_MPDDRC_CR_NDQS_DISABLED		(0x1 << 21)
#define ATMEL_MPDDRC_CR_DECOD_INTERLEAVED	(0x1 << 22)
#define ATMEL_MPDDRC_CR_UNAL_SUPPORTED		(0x1 << 23)

/* Bit field in timing parameter 0 register */
#define ATMEL_MPDDRC_TPR0_TRAS_OFFSET		0
#define ATMEL_MPDDRC_TPR0_TRAS_MASK		0xf
#define ATMEL_MPDDRC_TPR0_TRCD_OFFSET		4
#define ATMEL_MPDDRC_TPR0_TRCD_MASK		0xf
#define ATMEL_MPDDRC_TPR0_TWR_OFFSET		8
#define ATMEL_MPDDRC_TPR0_TWR_MASK		0xf
#define ATMEL_MPDDRC_TPR0_TRC_OFFSET		12
#define ATMEL_MPDDRC_TPR0_TRC_MASK		0xf
#define ATMEL_MPDDRC_TPR0_TRP_OFFSET		16
#define ATMEL_MPDDRC_TPR0_TRP_MASK		0xf
#define ATMEL_MPDDRC_TPR0_TRRD_OFFSET		20
#define ATMEL_MPDDRC_TPR0_TRRD_MASK		0xf
#define ATMEL_MPDDRC_TPR0_TWTR_OFFSET		24
#define ATMEL_MPDDRC_TPR0_TWTR_MASK		0x7
#define ATMEL_MPDDRC_TPR0_RDC_WRRD_OFFSET	27
#define ATMEL_MPDDRC_TPR0_RDC_WRRD_MASK		0x1
#define ATMEL_MPDDRC_TPR0_TMRD_OFFSET		28
#define ATMEL_MPDDRC_TPR0_TMRD_MASK		0xf

/* Bit field in timing parameter 1 register */
#define ATMEL_MPDDRC_TPR1_TRFC_OFFSET		0
#define ATMEL_MPDDRC_TPR1_TRFC_MASK		0x7f
#define ATMEL_MPDDRC_TPR1_TXSNR_OFFSET		8
#define ATMEL_MPDDRC_TPR1_TXSNR_MASK		0xff
#define ATMEL_MPDDRC_TPR1_TXSRD_OFFSET		16
#define ATMEL_MPDDRC_TPR1_TXSRD_MASK		0xff
#define ATMEL_MPDDRC_TPR1_TXP_OFFSET		24
#define ATMEL_MPDDRC_TPR1_TXP_MASK		0xf

/* Bit field in timing parameter 2 register */
#define ATMEL_MPDDRC_TPR2_TXARD_OFFSET		0
#define ATMEL_MPDDRC_TPR2_TXARD_MASK		0xf
#define ATMEL_MPDDRC_TPR2_TXARDS_OFFSET		4
#define ATMEL_MPDDRC_TPR2_TXARDS_MASK		0xf
#define ATMEL_MPDDRC_TPR2_TRPA_OFFSET		8
#define ATMEL_MPDDRC_TPR2_TRPA_MASK		0xf
#define ATMEL_MPDDRC_TPR2_TRTP_OFFSET		12
#define ATMEL_MPDDRC_TPR2_TRTP_MASK		0x7
#define ATMEL_MPDDRC_TPR2_TFAW_OFFSET		16
#define ATMEL_MPDDRC_TPR2_TFAW_MASK		0xf

/* Bit field in Memory Device Register */
#define ATMEL_MPDDRC_MD_SDR_SDRAM	0x0
#define ATMEL_MPDDRC_MD_LP_SDR_SDRAM	0x1
#define ATMEL_MPDDRC_MD_DDR_SDRAM	0x2
#define ATMEL_MPDDRC_MD_LPDDR_SDRAM	0x3
#define ATMEL_MPDDRC_MD_DDR3_SDRAM	0x4
#define ATMEL_MPDDRC_MD_LPDDR3_SDRAM	0x5
#define ATMEL_MPDDRC_MD_DDR2_SDRAM	0x6
#define ATMEL_MPDDRC_MD_LPDDR2_SDRAM	0x7
#define ATMEL_MPDDRC_MD_DBW_MASK	(0x1 << 4)
#define ATMEL_MPDDRC_MD_DBW_32_BITS	(0x0 << 4)
#define ATMEL_MPDDRC_MD_DBW_16_BITS	(0x1 << 4)

/* Bit field in I/O Calibration Register */
#define ATMEL_MPDDRC_IO_CALIBR_RDIV		0x7

#define ATMEL_MPDDRC_IO_CALIBR_LPDDR2_RZQ_34_3	0x1
#define ATMEL_MPDDRC_IO_CALIBR_LPDDR2_RZQ_40	0x2
#define ATMEL_MPDDRC_IO_CALIBR_LPDDR2_RZQ_48	0x3
#define ATMEL_MPDDRC_IO_CALIBR_LPDDR2_RZQ_60	0x4
#define ATMEL_MPDDRC_IO_CALIBR_LPDDR2_RZQ_80	0x6
#define ATMEL_MPDDRC_IO_CALIBR_LPDDR2_RZQ_120	0x7

#define ATMEL_MPDDRC_IO_CALIBR_DDR2_RZQ_35	0x2
#define ATMEL_MPDDRC_IO_CALIBR_DDR2_RZQ_43	0x3
#define ATMEL_MPDDRC_IO_CALIBR_DDR2_RZQ_52	0x4
#define ATMEL_MPDDRC_IO_CALIBR_DDR2_RZQ_70	0x6
#define ATMEL_MPDDRC_IO_CALIBR_DDR2_RZQ_105	0x7

#define ATMEL_MPDDRC_IO_CALIBR_DDR3_RZQ_37	0x2
#define ATMEL_MPDDRC_IO_CALIBR_DDR3_RZQ_44	0x3
#define ATMEL_MPDDRC_IO_CALIBR_DDR3_RZQ_55	0x4
#define ATMEL_MPDDRC_IO_CALIBR_DDR3_RZQ_73	0x6
#define ATMEL_MPDDRC_IO_CALIBR_DDR3_RZQ_110	0x7

#define ATMEL_MPDDRC_IO_CALIBR_DDR3_RZQ_37	0x2
#define ATMEL_MPDDRC_IO_CALIBR_DDR3_RZQ_44	0x3
#define ATMEL_MPDDRC_IO_CALIBR_DDR3_RZQ_55	0x4
#define ATMEL_MPDDRC_IO_CALIBR_DDR3_RZQ_73	0x6
#define ATMEL_MPDDRC_IO_CALIBR_DDR3_RZQ_110	0x7

#define ATMEL_MPDDRC_IO_CALIBR_TZQIO		(0x7f << 8)
#define ATMEL_MPDDRC_IO_CALIBR_TZQIO_(x)	(((x) & 0x7f) << 8)

#define ATMEL_MPDDRC_IO_CALIBR_CALCODEP		(0xf << 16)
#define ATMEL_MPDDRC_IO_CALIBR_CALCODEP_(x)	(((x) & 0xf) << 16)
#define ATMEL_MPDDRC_IO_CALIBR_CALCODEN		(0xf << 20)
#define ATMEL_MPDDRC_IO_CALIBR_CALCODEN_(x)	(((x) & 0xf) << 20)

#define ATMEL_MPDDRC_IO_CALIBR_EN_CALIB		(0x1 << 4)

/* Bit field in Read Data Path Register */
#define ATMEL_MPDDRC_RD_DATA_PATH_SHIFT_SAMPLING	0x3
#define ATMEL_MPDDRC_RD_DATA_PATH_NO_SHIFT		0x0
#define ATMEL_MPDDRC_RD_DATA_PATH_SHIFT_ONE_CYCLE	0x1
#define ATMEL_MPDDRC_RD_DATA_PATH_SHIFT_TWO_CYCLE	0x2
#define ATMEL_MPDDRC_RD_DATA_PATH_SHIFT_THREE_CYCLE	0x3

/* Bit field in LPDDR2 - LPDDR3 Low Power Register */
#define ATMEL_MPDDRC_LPDDR23_LPR_DS(x)			(((x) & 0xf) << 24)

/* Bit field in CAL_MR4 Calibration and MR4 Register */
#define ATMEL_MPDDRC_CAL_MR4_COUNT_CAL(x)		(((x) & 0xffff) << 0)
#define ATMEL_MPDDRC_CAL_MR4_MR4R(x)			(((x) & 0xffff) << 16)

/* Bit field in TIM_CAL Timing Calibration Register */
#define ATMEL_MPDDRC_CALR_ZQCS(x)			(((x) & 0xff) << 0)

#endif
