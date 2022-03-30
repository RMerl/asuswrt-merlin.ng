// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Broadcom
 */
/*
    
*/

#ifndef RDP_PLATFORM_H_INCLUDED
#define RDP_PLATFORM_H_INCLUDED


#define MAX_NUM_OF_QUEUES_IN_SCHED 32

/* SBPM */
#define SBPM_BASE_ADDRESS	0
#define SBPM_INIT_OFFSET	(SBPM_MAX_BUFFER_NUMBER)
#define SBPM_MAX_NUM_OF_BNS	(SBPM_MAX_BUFFER_NUMBER + 1)


typedef enum rnr_quad_id_e {
	RNR_QUAD0_ID = 0,
	RNR_QUAD_ID_LAST = RNR_QUAD0_ID,
	NUM_OF_RNR_QUADS = RNR_QUAD_ID_LAST + 1,
} rnr_quad_id_e;

#if defined(CONFIG_BCM6858)

#define RNR_FREQ_IN_MHZ 1000
#define UBUS_SLV_FREQ_IN_MHZ 500
#define NUM_OF_CORES_IN_QUAD 4
#define DRV_PARSER_MASKED_DA_FILTER_NUM 2
#define DRV_PARSER_DA_FILTER_NUM 9
#define NUM_OF_RNR_WITH_PROFILING 16

/* DISP CONGESTION */
#define DIS_REOR_CONGESTION_THRESHOLD	450
#define DIS_REOR_LINKED_LIST_BUFFER_NUM  1024
#define NUM_OF_PROCESSING_TASKS		  96

/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 127

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 8
#define BBH_TX_DS_PD_FIFO_SIZE_1 8

/* HASH */
#define HASH_NUM_OF_ENGINES		   2
#define HASH_NUM_OF_ENGINES_LOG2	  1
#define HASH_NUM_OF_EFFECTIVE_ENGINES 2
#define HASH_NUM_OF_ENTRIES_IN_RAM	2048

#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_ID_PON)
#define IS_SDMA(dma_id) (dma_id > DMA1_ID)
#define IS_DMA(dma_id) (dma_id <= DMA1_ID)
#define IS_PROCESSING_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == processing_runner_image)
#define IS_DS_TM_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == ds_tm_runner_image)

/* NATC */
#define NATC_CACHE_ENTRIES_NUM   512

/* TCAM table size (entries) */
#define RDP_TCAM_TABLE_SIZE	 512
/* Number of TCAM engines */
#define RDP_TCAM_NUM_ENGINES	2

/* Max number of queues supported by QM */
#define QM_NUM_QUEUES		   288

typedef enum dma_id_e
{
	DMA0_ID = 0,
	DMA1_ID,
	SDMA0_ID,
	SDMA1_ID,
	DMA_NUM,
	DMA_ID_FIRST = DMA0_ID,
} dma_id_e;

typedef enum bbh_id_e
{
	BBH_ID_0 = 0,  /* XLMAC1_0_RGMII */
	BBH_ID_1,	  /* XLMAC0_1_2p5G  */
	BBH_ID_2,	  /* XLMAC0_2_1G	*/
	BBH_ID_3,	  /* XLMAC0_3_1G	*/
	BBH_ID_4,	  /* XLMAC0_0_10G   */
	BBH_ID_5,	  /* XLMAC1_1_RGMII */
	BBH_ID_6,	  /* XLMAC1_2_RGMII */
	BBH_ID_7,	  /* XLMAC1_3_RGMII */
	BBH_ID_PON,
	BBH_ID_NUM,
	BBH_ID_FIRST = BBH_ID_0,
	BBH_ID_LAST = BBH_ID_PON,
	BBH_ID_LAST_XLMAC = BBH_ID_7,
	BBH_ID_NULL = BBH_ID_NUM
} bbh_id_e;


/* SBPM */

#define SBPM_MAX_BUFFER_NUMBER			  0xFFF


/**********************************************************************************************************/
#elif  defined(CONFIG_BCM6846)

#define RNR_FREQ_IN_MHZ 1400
#define UBUS_SLV_FREQ_IN_MHZ 466
#define NUM_OF_CORES_IN_QUAD 3
#define DRV_PARSER_MASKED_DA_FILTER_NUM 2
#define DRV_PARSER_DA_FILTER_NUM 9
#define NUM_OF_RNR_WITH_PROFILING 3

#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_TX_WAN_ID)
#define IS_WAN_RX_PORT(bbh_id) (bbh_id == BBH_ID_PON)
#define IS_SDMA(dma_id)  (dma_id > DMA0_ID)
#define IS_DMA(dma_id)   (dma_id <= DMA0_ID)
#define IS_PROCESSING_RUNNER_IMAGE(i)  ((rdp_core_to_image_map[i] == processing0_runner_image) || (rdp_core_to_image_map[i] == processing1_runner_image))
#define IS_DS_TM_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == ds_tm_runner_image)

/* NATC */
#define NATC_CACHE_ENTRIES_NUM   512

/* DISP CONGESTION */
#define DIS_REOR_CONGESTION_THRESHOLD	200
#define DIS_REOR_LINKED_LIST_BUFFER_NUM  512
#define NUM_OF_PROCESSING_TASKS		  12

/* SBPM */

#define SBPM_MAX_BUFFER_NUMBER			  0x5FF


/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 63
#define BBH_FREQUENCY (466770000) /* 466.77 MHz */

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 7
#define BBH_TX_DS_PD_FIFO_SIZE_1 7

/* HASH */
#define HASH_NUM_OF_ENGINES		   2
#define HASH_NUM_OF_ENGINES_LOG2	  1
#define HASH_NUM_OF_EFFECTIVE_ENGINES 2

/* TCAM table size (entries) */
#define RDP_TCAM_TABLE_SIZE	 256
/* Number of TCAM engines */
#define RDP_TCAM_NUM_ENGINES	1

/* Max number of queues supported by QM */
#define QM_NUM_QUEUES		   96
/* Max number of EPON queues supported by QM */
#define QM_NUM_EPON_QUEUES	  8
/* Max number of reported queues supported by QM */
#define QM_NUM_REPORTED_QUEUES  32

typedef enum bbh_id_e
{
	BBH_ID_0 = 0,
	BBH_ID_1,
	BBH_ID_2,
	BBH_ID_3,
	BBH_ID_4,
	BBH_ID_PON,
	BBH_ID_NUM,
	BBH_ID_FIRST = BBH_ID_0,
	BBH_ID_LAST = BBH_ID_PON,
	BBH_ID_NULL = BBH_ID_NUM
} bbh_id_e;

typedef enum dma_id_e
{
	DMA0_ID = 0,
	SDMA0_ID,
	DMA_NUM,
	DMA_ID_FIRST = DMA0_ID,
} dma_id_e;

/**********************************************************************************************************/

#elif  defined(CONFIG_BCM6878)
#define RNR_FREQ_IN_MHZ 1000
#define UBUS_SLV_FREQ_IN_MHZ 466
#define NUM_OF_CORES_IN_QUAD 3
#define DRV_PARSER_MASKED_DA_FILTER_NUM 2
#define DRV_PARSER_DA_FILTER_NUM 9
#define NUM_OF_RNR_WITH_PROFILING 3

#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_TX_WAN_ID)
#define IS_WAN_RX_PORT(bbh_id) (bbh_id == BBH_ID_PON)
#define IS_SDMA(dma_id)  (dma_id > DMA0_ID)
#define IS_DMA(dma_id)   (dma_id <= DMA0_ID)
#define IS_PROCESSING_RUNNER_IMAGE(i)  ((rdp_core_to_image_map[i] == processing0_runner_image) || (rdp_core_to_image_map[i] == processing1_runner_image))
#define IS_DS_TM_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == ds_tm_runner_image)


/* NATC */
#define NATC_CACHE_ENTRIES_NUM   512

/* DISP CONGESTION */
#define DIS_REOR_CONGESTION_THRESHOLD	100
#define DIS_REOR_LINKED_LIST_BUFFER_NUM  256
#define NUM_OF_PROCESSING_TASKS		  12

/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 63
#define BBH_FREQUENCY (466770000) /* 466.77 MHz */

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 7
#define BBH_TX_DS_PD_FIFO_SIZE_1 7

/* HASH */
#define HASH_NUM_OF_ENGINES		   2
#define HASH_NUM_OF_ENGINES_LOG2	  1
#define HASH_NUM_OF_EFFECTIVE_ENGINES 2

/* TCAM table size (entries) */
#define RDP_TCAM_TABLE_SIZE	 256
/* Number of TCAM engines */
#define RDP_TCAM_NUM_ENGINES	1

/* Max number of queues supported by QM */
#define QM_NUM_QUEUES		   128
/* Max number of EPON queues supported by QM */
#define QM_NUM_EPON_QUEUES	  8
/* Max number of reported queues supported by QM */
#define QM_NUM_REPORTED_QUEUES  32

typedef enum bbh_id_e
{
	BBH_ID_0 = 0,
	BBH_ID_1,
	BBH_ID_2,
	BBH_ID_3,
	BBH_ID_4,
	BBH_ID_PON,
	BBH_ID_NUM,
	BBH_ID_FIRST = BBH_ID_0,
	BBH_ID_LAST = BBH_ID_PON,
	BBH_ID_NULL = BBH_ID_NUM
} bbh_id_e;

typedef enum dma_id_e
{
	DMA0_ID = 0,
	DMA_NUM,
	DMA_ID_FIRST = DMA0_ID,
} dma_id_e;

typedef enum bbh_tx_id_e
{
	BBH_TX_ID_LAN = 0,
	BBH_TX_ID_PON,
	BBH_TX_ID_NUM,
	BBH_TX_ID_FIRST = BBH_TX_ID_LAN,
	BBH_TX_ID_LAST = BBH_TX_ID_PON,
	BBH_TX_ID_NULL = BBH_TX_ID_NUM
} bbh_tx_id_e;

/* SBPM */
#define SBPM_MAX_BUFFER_NUMBER			  0x3FF



#elif defined(CONFIG_BCM6856) 

#define RNR_FREQ_IN_MHZ 1400
#define UBUS_SLV_FREQ_IN_MHZ 500
#define NUM_OF_CORES_IN_QUAD 4
#define DRV_PARSER_MASKED_DA_FILTER_NUM 2
#define DRV_PARSER_DA_FILTER_NUM 9
#define NUM_OF_RNR_WITH_PROFILING 8

#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_TX_WAN_ID)
#define IS_SDMA(dma_id)  (dma_id > DMA0_ID)
#define IS_DMA(dma_id)   (dma_id <= DMA0_ID)
#define IS_PROCESSING_RUNNER_IMAGE(i)  ((rdp_core_to_image_map[i] == processing0_runner_image) || (rdp_core_to_image_map[i] == processing1_runner_image) || (rdp_core_to_image_map[i] == processing2_runner_image) || (rdp_core_to_image_map[i] == processing3_runner_image))
#define IS_DS_TM_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == ds_tm_runner_image)

/* NATC */
#define NATC_CACHE_ENTRIES_NUM   512

/* DISP CONGESTION */
#define DIS_REOR_CONGESTION_THRESHOLD	200
#define DIS_REOR_LINKED_LIST_BUFFER_NUM  1024
#define NUM_OF_PROCESSING_TASKS		  32

/* SBPM */
#define SBPM_UG0_BN_THRESHOLD			   0x3FF

/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 63
#define BBH_FREQUENCY (500*1000000) /* 500 MHz */

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 7
#define BBH_TX_DS_PD_FIFO_SIZE_1 7

/* HASH */
#define HASH_NUM_OF_ENGINES		   4
#define HASH_NUM_OF_ENGINES_LOG2	  2
#define HASH_NUM_OF_EFFECTIVE_ENGINES 4
#define HASH_NUM_OF_ENTRIES_IN_RAM	6144

/* TCAM table size (entries) */
#define RDP_TCAM_TABLE_SIZE	 256
/* Number of TCAM engines */
#define RDP_TCAM_NUM_ENGINES	2

/* Max number of queues supported by QM */
#define QM_NUM_QUEUES		   160
/* Max number of reported queues supported by QM */
#define QM_NUM_REPORTED_QUEUES  128

typedef enum dma_id_e
{
	DMA0_ID = 0,
	DMA1_ID,
	SDMA0_ID,
	SDMA1_ID,
	DMA_NUM,
	DMA_ID_FIRST = DMA0_ID,
} dma_id_e;

typedef enum bbh_id_e
{
	BBH_ID_0 = 0,
	BBH_ID_1,
	BBH_ID_2,
	BBH_ID_3,
	BBH_ID_4,
	BBH_ID_5,
	BBH_ID_PON,
	BBH_ID_NUM,
	BBH_ID_FIRST = BBH_ID_0,
	BBH_ID_LAST = BBH_ID_PON,
	BBH_ID_NULL = BBH_ID_NUM
} bbh_id_e;

typedef enum bbh_tx_id_e
{
	BBH_TX_ID_LAN = 0,
	BBH_TX_ID_PON,
	BBH_TX_ID_NUM,
	BBH_TX_ID_FIRST = BBH_TX_ID_LAN,
	BBH_TX_ID_LAST = BBH_TX_ID_PON,
	BBH_TX_ID_NULL = BBH_TX_ID_NUM
} bbh_tx_id_e;

#elif  defined(CONFIG_BCM63146)

#define RNR_FREQ_IN_MHZ 1250
/* TBD63146. Need for CONFIG_BCM63146 */
#define UBUS_SLV_FREQ_IN_MHZ 560
/* TBD below */
#define NUM_OF_CORES_IN_QUAD 3
#define DRV_PARSER_MASKED_DA_FILTER_NUM 2
#define DRV_PARSER_DA_FILTER_NUM 9
#define NUM_OF_RNR_WITH_PROFILING 3

/* IS_WAN_TX_PORT and IS_WAN_RX_PORT identify TX WAN ports for DSL only (NOT Ethernet) */
#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_TX_ID_DSL)
#define IS_WAN_RX_PORT(bbh_id) (bbh_id == BBH_ID_DSL)
/* IS_DS_WAN_PORT and IS_US_WAN_PORT identify ALL (DS/US respectively) WAN ports */
/* TBD63146. Identify Ethernet WAN */
#define IS_DS_WAN_PORT(bbh_id)  (bbh_id == BBH_ID_DSL)
#define IS_US_WAN_PORT(bbh_id) (bbh_id == BBH_TX_ID_DSL)
#define IS_PROCESSING_RUNNER_IMAGE(i) \
	((rdp_core_to_image_map[i] == processing0_runner_image) || \
	(rdp_core_to_image_map[i] == processing1_runner_image) || \
	(rdp_core_to_image_map[i] == processing2_runner_image))
#define IS_DS_TM_RUNNER_IMAGE(i) (rdp_core_to_image_map[i] == ds_tm_runner_image)
#define IS_US_TM_RUNNER_IMAGE(i) (rdp_core_to_image_map[i] == us_tm_runner_image)

/* NATC */
#define NATC_CACHE_ENTRIES_NUM 1024

/* DISP CONGESTION */
#define DIS_REOR_CONGESTION_THRESHOLD	200
#define DIS_REOR_LINKED_LIST_BUFFER_NUM	512
#define NUM_OF_PROCESSING_TASKS	 	32

/* SBPM */
/* the real physical limit is 0x5FF */
#define SBPM_MAX_BUFFER_NUMBER	0x17F
#define SBPM_UG0_BN_THRESHOLD	0x15F
#define SBPM_UG1_BN_THRESHOLD	0x23

/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 127
/* TBD63146. Need for BCM63146 */
#define BBH_FREQUENCY (560*1000000) /* 560 MHz */

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 7
#define BBH_TX_DS_PD_FIFO_SIZE_1 7

/* HASH */
#define HASH_NUM_OF_ENGINES		2
#define HASH_NUM_OF_ENGINES_LOG2	1
#define HASH_NUM_OF_EFFECTIVE_ENGINES	2

/* TCAM table size (entries) */
#define RDP_TCAM_TABLE_SIZE	256
/* Number of TCAM engines */
#define RDP_TCAM_NUM_ENGINES	2

/* Max number of queues supported by QM */
#define QM_NUM_QUEUES		128

typedef enum dma_id_e {
	DMA0_ID = 0,
	DMA1_ID,
	DMA_NUM,
	DMA_ID_FIRST = DMA0_ID,
} dma_id_e;

typedef enum bbh_id_e {
	/* RX */
	/* RX BBH_ID needs to be the same as DISP_REOR_VIQ defined in
	 * project_data_structures.xml or else bbh_rx configuration will break */
	BBH_ID_0 = 0,
	BBH_ID_1,
	BBH_ID_2,
	BBH_ID_3,
	BBH_ID_4,
	BBH_ID_5,
	BBH_ID_6_2P5G,
	BBH_ID_7_5G,
	BBH_ID_DSL,
	BBH_ID_NUM,
	BBH_ID_FIRST = BBH_ID_0,
	BBH_ID_NUM_LAN = BBH_ID_7_5G,
	BBH_ID_LAST = BBH_ID_DSL,
	BBH_ID_NULL = BBH_ID_NUM,
} bbh_id_e;

typedef enum bbh_tx_id_e {
	BBH_TX_ID_LAN = 0,
	BBH_ID_QM_COPY,
	BBH_TX_ID_DSL,
	BBH_TX_ID_NUM,
	BBH_TX_ID_FIRST = BBH_TX_ID_LAN,
	BBH_TX_ID_LAST = BBH_TX_ID_DSL,
	BBH_TX_ID_NULL = BBH_ID_NULL,
} bbh_tx_id_e;

typedef enum dma_target_e {
	DMA_TARGET_DMA = 0,
	DMA_TARGET_SDMA,
	DMA_TARGET_NUM,
	DMA_TARGET_FIRST = DMA_TARGET_DMA,
} dma_target_e;

/* some change of definition for easy to compilation */
#define BBH_ID_PON BBH_ID_DSL
#define BB_ID_RX_PON BB_ID_RX_DSL
#define BB_ID_TX_PON_ETH_PD BB_ID_TX_DSL
#define BB_ID_TX_PON_ETH_STAT BB_ID_TX_DSL_STAT

#define SDMA_U_THRESH_IN_BBH_LAN_VALUE (0x2)
#define SDMA_U_THRESH_OUT_BBH_LAN_VALUE	(0x1)

#define SDMA_U_THRESH_IN_BBH_5G_VALUE (0x3)
#define SDMA_U_THRESH_OUT_BBH_5G_VALUE (0x2)

#define SDMA_U_THRESH_IN_BBH_2P5G_VALUE (0x3)
#define SDMA_U_THRESH_OUT_BBH_2P5G_VALUE (0x2)

#define SDMA_U_THRESH_IN_BBH_DSL_VALUE (0x6)
#define SDMA_U_THRESH_OUT_BBH_DSL_VALUE (0x4)

#define SDMA_STRICT_PRI_RX_BBH_LAN_VALUE (0x4)
#define SDMA_STRICT_PRI_RX_BBH_5G_VALUE (0x04)
#define SDMA_STRICT_PRI_RX_BBH_2P5G_VALUE (0x04)
#define SDMA_STRICT_PRI_RX_BBH_DSL_VALUE (0x4)

#define SDMA_STRICT_PRI_TX_BBH_LAN_VALUE (0x04)
#define DMA_STRICT_PRI_TX_BBH_LAN_VALUE (0x08)
#define SDMA_STRICT_PRI_TX_BBH_COPY_VALUE (0x04)
#define DMA_STRICT_PRI_TX_BBH_COPY_VALUE (0x08)
#define SDMA_STRICT_PRI_TX_BBH_DSL_VALUE (0x04)
#define DMA_STRICT_PRI_TX_BBH_DSL_VALUE (0x08)

#define SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE (0x00)
#define SDMA_RR_WEIGHT_RX_BBH_5G_VALUE (0x00)
#define SDMA_RR_WEIGHT_RX_BBH_2P5G_VALUE (0x00)
#define SDMA_RR_WEIGHT_RX_BBH_DSL_VALUE (0x00)

#define SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE (0x00)
#define DMA_RR_WEIGHT_TX_BBH_LAN_VALUE (0x00)
#define SDMA_RR_WEIGHT_TX_BBH_COPY_VALUE (0x00)
#define DMA_RR_WEIGHT_TX_BBH_COPY_VALUE (0x00)
#define SDMA_RR_WEIGHT_TX_BBH_DSL_VALUE (0x00)
#define DMA_RR_WEIGHT_TX_BBH_DSL_VALUE (0x00)
#endif

#endif
