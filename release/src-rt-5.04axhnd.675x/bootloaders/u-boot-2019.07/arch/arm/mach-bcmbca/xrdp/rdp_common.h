// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Broadcom
 */
/*
    
*/

#ifndef RDP_COMMON_H_INCLUDED
#define RDP_COMMON_H_INCLUDED

#include "rdp_platform.h"
#include "rdd.h"
#include "access_macros.h"

/* default values for PARSER */
#define PARSER_TCP_CTL_FLAGS		0x07
#define PARSER_EXCP_IP_HDR_LEN_ERR	(1 << 0)
#define PARSER_EXCP_CHKSUM_ERR		(1 << 1)
#define PARSER_EXCP_ETH_MULCST		(1 << 2)
#define PARSER_EXCP_IP_MULCST		(1 << 3)
#define PARSER_EXCP_DISABLE_IP_LEN_ERR	(1 << 4)
#define PARSER_EXCP_DISABLE_IP_VER_TEST	(1 << 5)
/* defines the protocol of ppp_code_1 (1 - IPv6, 0 - IPv4) in
 * register PPP_IP_Protocol_Code */
#define PARSER_EXCP_PPP_CODE_1_PROTOCOL	(1 << 7) 
#define PARSER_EXCP_IP_L2_MULCST	(1 << 10)
#define PARSER_EXCP_L4_INV_5_TUPPLE	(1 << 11)
#define PARSER_EXCP_UDP_1588		(1 << 12)
#define PARSER_EXCP_DHCP		(1 << 13)
/* MAC_SPOF bit is located in bit [3] on the eng_conf register (ENG) */
#define PARSER_EXCP_MAC_SPOOF		(1 << 3)
#define PARSER_EXCP_STATUS_BITS	\
	(PARSER_EXCP_IP_HDR_LEN_ERR | PARSER_EXCP_CHKSUM_ERR | \
	 PARSER_EXCP_ETH_MULCST | PARSER_EXCP_IP_MULCST | \
	 PARSER_EXCP_PPP_CODE_1_PROTOCOL | PARSER_EXCP_IP_L2_MULCST | \
	 PARSER_EXCP_L4_INV_5_TUPPLE | PARSER_EXCP_UDP_1588 | PARSER_EXCP_DHCP)
/* eng_conf register default configuration*/
#define PARSER_AH_DETECTION		0x18000 | PARSER_EXCP_MAC_SPOOF
#define PARSER_PPP_PROTOCOL_CODE_0_IPV4	0x21
#define PARSER_PPP_PROTOCOL_CODE_1_IPV6	0x57
#define PARSER_IP_PROTOCOL_IPIP		4
/* Profile 0 for DS, profile 1 for US */
#define PARSER_PROFILE_US		0x02

typedef enum {
	DMA_BUFSIZE_128  = 0,
	DMA_BUFSIZE_256  = 1,
	DMA_BUFSIZE_512  = 2,
	DMA_BUFSIZE_1024 = 3,
	DMA_BUFSIZE_2048 = 4,
} drv_rnr_dma_bufsize_t;

typedef enum  ddr_buf_size_e {
	BUF_256 = 0,
	BUF_512,
	BUF_1K,
	BUF_2K,
	BUF_4K
} ddr_buf_size_e;

void lookup_bbh_tx_bufsz_by_fpm_bufsz(uint32_t *fpm_bufsize,
				      uint8_t *bbh_tx_bufsize);
void lookup_dma_bufsz_by_fpm_bufsz(uint32_t *fpm_bufsz, uint8_t *dma_bufsz);

typedef struct {
	uint32_t low;
	uint8_t high;
} ddr_addr;

typedef enum {
	bcm_tag_opcode0,
	bcm_tag_opcode1
} bcm_tag_t;

typedef enum {
	hash_max_16_entries_per_engine,
	hash_max_32_entries_per_engine,
	hash_max_64_entries_per_engine,
	hash_max_128_entries_per_engine,
	hash_max_256_entries_per_engine,
	hash_max_512_entries_per_engine,
	hash_max_1k_entries_per_engine,
	hash_max_1_5k_entries_per_engine,
} tbl_size_e;

typedef struct {
	uint32_t mtu_size;
	uint32_t headroom_size;
	bcm_tag_t bcmsw_tag;
	/* runner tables DDR base address */
	ddr_addr rdp_phy_ddr_rnr_tables_base;
	/* runner tables DDR0 virtual base address */
	void *rdp_ddr_rnr_tables_base_virt;
	/* fpm pool DDR0 virtual base address for unicast packets */
	void *rdp_ddr_pkt_base_virt;
	uint32_t enabled_port_map;
	uint32_t runner_freq; /* rdp block clock */
	uint32_t fpm_buf_size;
	int xfi_port;
	uint32_t bbh_id_gbe_wan; /* bbh_id for gbe port */
	/* define the number of queue for DS queues */
	uint16_t number_of_ds_queues;
	/* define the number of queue for US queues */
	uint16_t number_of_us_queues;
	/* define the number of queue for SERVICE queues */
	uint16_t number_of_service_queues;
#ifdef G9991
	rdpa_emac system_port;
	uint32_t g9991_port_vec;
	uint32_t g9991_bbh_vec;
#endif
	bdmf_boolean dpu_split_scheduling_mode;
	tbl_size_e iptv_table_size;
	tbl_size_e arl_table_size;
} dpi_params_t;

typedef struct bbh_to_dma_x {
	bbh_id_e bbh_id;
	dma_id_e dma_id;
} bbh_to_dma_x_t;

/* QM queue index */
typedef uint16_t  rdp_qm_queue_idx_t;

/* FPM pool id */
typedef enum {
	FPM_POOL_ID_0 = 0,
	FPM_POOL_ID_1 = 1,
	FPM_POOL_ID_2 = 2,
	FPM_POOL_ID_3 = 3,
} fpm_pool_id_e;

/* FPM user group */
typedef enum {
	FPM_DS_UG = 0,
	FPM_US_UG = 1,
	FPM_WLAN_UG = 2,
	FPM_ALL_PASS_UG = 3,	
} fpm_ug_id_e;

/* Peripheral */
typedef enum {
	QM_PERIPH_ID_ETH0 = 0,
	QM_PERIPH_ID_ETH1 = 1,
	QM_PERIPH_ID_ETH2 = 2,
	QM_PERIPH_ID_ETH3 = 3,
	QM_PERIPH_ID_GPON = 4,
	QM_PERIPH_ID_EPON = 5,

	QM_PERIPH_ID__NUM_OF = 6
} qm_periph_id_e;

typedef enum ddr_byte_res_e {
	RES_1B = 0,
	RES_2B
} ddr_byte_res_e;

typedef enum natc_tbl_id_e {
	NATC_TBL0_ID = 0,
	NATC_TBL1_ID,
	NATC_TBL2_ID,
	NATC_TBL3_ID,
	NATC_TBL4_ID,
	NATC_TBL5_ID,
	NATC_TBL6_ID,
	NATC_TBL7_ID,
	NATC_TBL_ID_LAST = NATC_TBL7_ID,
} natc_tbl_id_e;

typedef enum natc_eng_id_e {
	NATC_ENG0_ID = 0,
	NATC_ENG1_ID,
	NATC_ENG2_ID,
	NATC_ENG3_ID,
	NATC_ENG_ID_LAST = NATC_ENG3_ID,
} natc_eng_id_e;

typedef enum ubus_mstr_id_e {
	UBUS_MSTR0_ID = 0,
	UBUS_MSTR1_ID,
	UBUS_MSTR_ID_LAST = UBUS_MSTR1_ID,
} ubus_mstr_id_e;

typedef enum xlif_channel_id_e {
	CHANNEL0_ID = 0,
	CHANNEL1_ID,
	CHANNEL2_ID,
	CHANNEL3_ID,
	CHANNEL4_ID,
	CHANNEL5_ID,
	CHANNEL6_ID,
	CHANNEL7_ID,
	CHANNEL_ID_LAST = CHANNEL7_ID,
} xlif_channel_id_e;

#define BB_ID_CPU0 (BB_ID_LAST + 1)
#define BB_ID_CPU1 (BB_ID_LAST + 2)
#define BB_ID_CPU2 (BB_ID_LAST + 3)

/* HASH key masks */
/* IPTV */
#define HASH_TABLE_IPTV_KEY_MASK_LO 0xFFFFFFFF /* 32bit key */
/* Only 12 bits of internal context are used */
#define HASH_TABLE_IPTV_KEY_MASK_HI 0x0000FFFF

/* ARL */
#define HASH_TABLE_ARL_KEY_MASK_LO 0xFFFFFFFF /* 60bit key */
#define HASH_TABLE_ARL_KEY_MASK_HI 0xFFFFFFF

/* Bridge and VLAN */
#define HASH_TABLE_BRIDGE_AND_VLAN_LKP_MASK_LO 0x000FFFFF /* 20bit key */
#define HASH_TABLE_BRIDGE_AND_VLAN_LKP_MASK_HI 0x0

#endif /* RDP_COMMON_H_INCLUDED */

