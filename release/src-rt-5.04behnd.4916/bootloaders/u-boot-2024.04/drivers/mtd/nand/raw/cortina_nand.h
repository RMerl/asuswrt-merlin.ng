/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2020 Cortina Access Inc..
 */

/* Cortina NAND definition */
#define NAND_BASE_ADDR		0xE0000000
#define BCH_GF_PARAM_M		14
#define BCH_DATA_UNIT		1024
#define FLASH_SHORT_DELAY	100
#define FLASH_LONG_DELAY	1000
#define FLASH_WIDTH		16
#define BBT_PAGE_MASK		0xffffff3f
#define WRITE_SIZE_512		512
#define WRITE_SIZE_2048		2048
#define WRITE_SIZE_4096		4096
#define WRITE_SIZE_8192		8192
#define ECC_STRENGTH_8		8
#define ECC_STRENGTH_16		16
#define ECC_STRENGTH_24		24
#define ECC_STRENGTH_40		40
#define EMPTY_PAGE		0xff
#define ADDR1_MASK0		0x00ffffff
#define ADDR2_MASK0		0xff000000
#define ADDR1_MASK1		0xffff
#define ADDR1_MASK2		0xff
#define OOB_MASK		0xff
#define EXT_ADDR_MASK		0x8000000

/* Status bits */
#define NAND_STATUS_FAIL	0x01
#define NAND_STATUS_FAIL_N1	0x02
#define NAND_STATUS_TRUE_READY	0x20
#define NAND_STATUS_READY	0x40
#define NAND_STATUS_WP		0x80

/* Bit field in FLAS_TYPE */
#define FLASH_PIN			BIT(15)
#define FLASH_TYPE_512			0x4000
#define FLASH_TYPE_2K			0x5000
#define FLASH_TYPE_4K			0x6000
#define FLASH_TYPE_8K			0x7000
#define FLASH_SIZE_CONFIGURABLEOOB	(0x0 << 9)
#define FLASH_SIZE_400OOB		(0x1 << 9)
#define FLASH_SIZE_436OOB		(0x2 << 9)
#define FLASH_SIZE_640OOB		(0x3 << 9)

/* Bit field in FLASH_STATUS */
#define NFLASH_READY	BIT(26)

/* Bit field in FLASH_NF_ACCESS */
#define NFLASH_ENABLE_ALTERNATIVE	(0x0 << 15)
#define AUTO_RESET			BIT(16)
#define DISABLE_AUTO_RESET		(0x0 << 16)
#define NFLASH_REG_WIDTH_RESERVED	(0x3 << 10)
#define NFLASH_REG_WIDTH_32		(0x2 << 10)
#define NFLASH_REG_WIDTH_16		(0x1 << 10)
#define NFLASH_REG_WIDTH_8		(0x0 << 10)

/* Bit field in FLASH_NF_COUNT */
#define REG_CMD_COUNT_EMPTY		0x3
#define REG_CMD_COUNT_3TOGO		0x2
#define REG_CMD_COUNT_2TOGO		0x1
#define REG_CMD_COUNT_1TOGO		0x0
#define REG_ADDR_COUNT_EMPTY		(0x7 << 4)
#define REG_ADDR_COUNT_5		(0x4 << 4)
#define REG_ADDR_COUNT_4		(0x3 << 4)
#define REG_ADDR_COUNT_3		(0x2 << 4)
#define REG_ADDR_COUNT_2		(0x1 << 4)
#define REG_ADDR_COUNT_1		(0x0 << 4)
#define REG_DATA_COUNT_EMPTY		(0x3fff << 8)
#define REG_DATA_COUNT_512_DATA		(0x1FF << 8)
#define REG_DATA_COUNT_2k_DATA		(0x7FF << 8)
#define REG_DATA_COUNT_4k_DATA		(0xFFF << 8)
#define REG_DATA_COUNT_DATA_1		(0x0 << 8)
#define REG_DATA_COUNT_DATA_2		(0x1 << 8)
#define REG_DATA_COUNT_DATA_3		(0x2 << 8)
#define REG_DATA_COUNT_DATA_4		(0x3 << 8)
#define REG_DATA_COUNT_DATA_5		(0x4 << 8)
#define REG_DATA_COUNT_DATA_6		(0x5 << 8)
#define REG_DATA_COUNT_DATA_7		(0x6 << 8)
#define REG_DATA_COUNT_DATA_8		(0x7 << 8)
#define REG_OOB_COUNT_EMPTY		(0x3ff << 22)

/* Bit field in FLASH_FLASH_ACCESS_START */
#define NFLASH_GO		BIT(0)
#define NFLASH_FIFO_REQ		BIT(2)
#define NFLASH_RD		BIT(13)
#define NFLASH_WT		(BIT(12) | BIT(13))

/* Bit field in FLASH_NF_ECC_RESET */
#define RESET_NFLASH_RESET	BIT(2)
#define RESET_NFLASH_FIFO	BIT(1)
#define RESET_NFLASH_ECC	BIT(0)
#define ECC_RESET_ALL \
	RESET_NFLASH_RESET | RESET_NFLASH_FIFO | RESET_NFLASH_ECC

/* Bit field in FLASH_NF_ECC_CONTROL */
#define ENABLE_ECC_GENERATION	BIT(8)
#define DISABLE_ECC_GENERATION	(0 << 8)

/* Flash FIFO control */
#define FIFO_READ		2
#define FIFO_WRITE		3

/* NFLASH INTERRUPT */
#define REGIRQ_CLEAR		BIT(0)
#define F_ADDR_ERR		2

/* BCH ECC field definition */
#define BCH_COMPARE		BIT(0)
#define	BCH_ENABLE		BIT(8)
#define	BCH_DISABLE		(0 << 8)
#define	BCH_DECODE		BIT(1)
#define	BCH_ENCODE		(0 << 1)
#define BCH_DECO_DONE		BIT(30)
#define BCH_GEN_DONE		BIT(31)
#define	BCH_UNCORRECTABLE	0x3
#define	BCH_CORRECTABLE_ERR	0x2
#define	BCH_NO_ERR		0x1
#define	BCH_BUSY		0x0
#define BCH_ERR_MASK		0x3
#define BCH_ERR_NUM_MASK	0x3F
#define BCH_ERR_LOC_MASK	0x3FFF
#define BCH_CORRECT_LOC_MASK	0x7
#define BCH_ERR_CAP_8		(0x0 << 9)
#define BCH_ERR_CAP_16		(0x1 << 9)
#define BCH_ERR_CAP_24		(0x2 << 9)
#define BCH_ERR_CAP_40		(0x3 << 9)

#define BCH_GF_PARAM_M		14

struct nand_ctlr {
	/* Cortina NAND controller register */
	u32 flash_id;
	u32 flash_timeout;
	u32 flash_status;
	u32 flash_type;
	u32 flash_flash_access_start;
	u32 flash_flash_interrupt;
	u32 flash_flash_mask;
	u32 flash_fifo_control;
	u32 flash_fifo_status;
	u32 flash_fifo_address;
	u32 flash_fifo_match_address;
	u32 flash_fifo_data;
	u32 flash_sf_access;
	u32 flash_sf_ext_access;
	u32 flash_sf_address;
	u32 flash_sf_data;
	u32 flash_sf_timing;
	u32 resv[3];
	u32 flash_pf_access;		// offset 0x050
	u32 flash_pf_timing;
	u32 resv1[2];
	u32 flash_nf_access;		// offset 0x060
	u32 flash_nf_count;
	u32 flash_nf_command;
	u32 flash_nf_address_1;
	u32 flash_nf_address_2;
	u32 flash_nf_data;
	u32 flash_nf_timing;
	u32 flash_nf_ecc_status;
	u32 flash_nf_ecc_control;
	u32 flash_nf_ecc_oob;
	u32 flash_nf_ecc_gen0;
	u32 resv3[15];
	u32 flash_nf_ecc_reset;		// offset 0x0c8
	u32 flash_nf_bch_control;
	u32 flash_nf_bch_status;
	u32 flash_nf_bch_error_loc01;
	u32 resv4[19];
	u32 flash_nf_bch_oob0;		// offset 0x124
	u32 resv5[17];
	u32 flash_nf_bch_gen0_0;	// offset 0x16c
};

/* Definition for DMA bitfield */
#define TX_DMA_ENABLE	BIT(0)
#define RX_DMA_ENABLE	BIT(0)
#define DMA_CHECK_OWNER	BIT(1)
#define OWN_DMA			0
#define OWN_CPU			1

#define CA_DMA_DEPTH	3
#define CA_DMA_DESC_NUM	(BIT(0) << CA_DMA_DEPTH)
#define CA_DMA_Q_PTR_MASK	0x1fff

struct dma_q_base_depth_t {
	u32 depth		:  4 ; /* bits 3:0 */
	u32 base		: 28 ; /* bits 31:4 */
};

struct tx_descriptor_t {
	unsigned int buf_adr; /* Buff addr */
	unsigned int buf_adr_hi	:  8 ; /* bits 7:0 */
	unsigned int buf_len	:  16 ;  /* bits 23:8 */
	unsigned int sgm	:  1 ;  /* bits 24 */
	unsigned int rsrvd	:  6 ;  /* bits 30:25 */
	unsigned int own	:  1 ;  /* bits 31:31 */
};

struct rx_descriptor_t {
	unsigned int buf_adr; /* Buff addr */
	unsigned int buf_adr_hi	:  8 ; /* bits 7:0 */
	unsigned int buf_len	: 16 ;  /* bits 23:8 */
	unsigned int rsrvd	:  7 ;  /* bits 30:24 */
	unsigned int own	:  1 ;  /* bits 31:31 */
};

struct dma_global {
	u32 dma_glb_dma_lso_ctrl;
	u32 dma_glb_lso_interrupt;
	u32 dma_glb_lso_intenable;
	u32 dma_glb_dma_lso_vlan_tag_type0;
	u32 dma_glb_dma_lso_vlan_tag_type1;
	u32 dma_glb_dma_lso_axi_user_sel0;
	u32 dma_glb_axi_user_pat0;
	u32 dma_glb_axi_user_pat1;
	u32 dma_glb_axi_user_pat2;
	u32 dma_glb_axi_user_pat3;
	u32 dma_glb_fast_reg_pe0;
	u32 dma_glb_fast_reg_pe1;
	u32 dma_glb_dma_lso_tx_fdes_addr0;
	u32 dma_glb_dma_lso_tx_fdes_addr1;
	u32 dma_glb_dma_lso_tx_cdes_addr0;
	u32 dma_glb_dma_lso_tx_cdes_addr1;
	u32 dma_glb_dma_lso_tx_des_word0;
	u32 dma_glb_dma_lso_tx_des_word1;
	u32 dma_glb_dma_lso_lso_para_word0;
	u32 dma_glb_dma_lso_lso_para_word1;
	u32 dma_glb_dma_lso_debug0;
	u32 dma_glb_dma_lso_debug1;
	u32 dma_glb_dma_lso_debug2;
	u32 dma_glb_dma_lso_spare0;
	u32 dma_glb_dma_lso_spare1;
	u32 dma_glb_dma_ssp_rx_ctrl;
	u32 dma_glb_dma_ssp_tx_ctrl;
	u32 dma_glb_dma_ssp_axi_user_sel0;
	u32 dma_glb_dma_ssp_axi_user_sel1;
	u32 dma_glb_dma_ssp_rx_fdes_addr0;
	u32 dma_glb_dma_ssp_rx_fdes_addr1;
	u32 dma_glb_dma_ssp_rx_cdes_addr0;
	u32 dma_glb_dma_ssp_rx_cdes_addr1;
	u32 dma_glb_dma_ssp_rx_des_word0;
	u32 dma_glb_dma_ssp_rx_des_word1;
	u32 dma_glb_dma_ssp_tx_fdes_addr0;
	u32 dma_glb_dma_ssp_tx_fdes_addr1;
	u32 dma_glb_dma_ssp_tx_cdes_addr0;
	u32 dma_glb_dma_ssp_tx_cdes_addr1;
	u32 dma_glb_dma_ssp_tx_des_word0;
	u32 dma_glb_dma_ssp_tx_des_word1;
	u32 dma_glb_dma_ssp_debug0;
	u32 dma_glb_dma_ssp_debug1;
	u32 dma_glb_dma_ssp_debug2;
	u32 dma_glb_dma_ssp_spare0;
	u32 dma_glb_dma_ssp_spare1;
};

struct dma_ssp {
	u32 dma_q_rxq_control;
	u32 dma_q_rxq_base_depth;
	u32 dma_q_rxq_base;
	u32 dma_q_rxq_wptr;
	u32 dma_q_rxq_rptr;
	u32 dma_q_rxq_pktcnt;
	u32 dma_q_txq_control;
	u32 dma_q_txq_base_depth;
	u32 dma_q_txq_base;
	u32 dma_q_txq_wptr;
	u32 dma_q_txq_rptr;
	u32 dma_q_txq_pktcnt;
	u32 dma_q_rxq_interrupt;
	u32 dma_q_rxq_intenable;
	u32 dma_q_txq_interrupt;
	u32 dma_q_txq_intenable;
	u32 dma_q_rxq_misc_interrupt;
	u32 dma_q_rxq_misc_intenable;
	u32 dma_q_txq_misc_interrupt;
	u32 dma_q_txq_misc_intenable;
	u32 dma_q_rxq_coal_interrupt;
	u32 dma_q_rxq_coal_intenable;
	u32 dma_q_txq_coal_interrupt;
	u32 dma_q_txq_coal_intenable;
	u32 dma_q_rxq_frag_buff_addr0;
	u32 dma_q_rxq_frag_buff_addr1;
	u32 dma_q_rxq_frag_buff_size;
	u32 dma_q_txq_frag_buff_addr0;
	u32 dma_q_txq_frag_buff_addr1;
	u32 dma_q_txq_frag_buff_size;
	u32 dma_q_dma_spare_0;
	u32 dma_q_dma_spare_1;
};
