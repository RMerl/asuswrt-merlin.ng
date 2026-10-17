/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2021 Broadcom.
 */

#ifndef _BNXT_H_
#define _BNXT_H_

#include <pci.h>
#include <linux/if_ether.h>

#include "bnxt_hsi.h"

union dma_addr64_t {
	dma_addr_t addr;
	u64 as_u64;
};

#define DRIVER_VERSION_MAJOR                    1
#define DRIVER_VERSION_MINOR                    0
#define DRIVER_VERSION_UPDATE                   0

/* Broadcom ethernet driver defines. */
#define FLAG_SET(f, b)                          ((f) |= (b))
#define FLAG_TEST(f, b)                         ((f) & (b))
#define FLAG_RESET(f, b)                        ((f) &= ~(b))
#define BNXT_FLAG_HWRM_SHORT_CMD_SUPP           BIT(0)
#define BNXT_FLAG_HWRM_SHORT_CMD_REQ            BIT(1)
#define BNXT_FLAG_RESOURCE_QCAPS_SUPPORT        BIT(2)
#define BNXT_FLAG_MULTI_HOST                    BIT(3)
#define BNXT_FLAG_NPAR_MODE                     BIT(4)
/*******************************************************************************
 * Status codes.
 ******************************************************************************/
#define STATUS_SUCCESS                          0
#define STATUS_FAILURE                          1
#define STATUS_LINK_ACTIVE                      4
#define STATUS_LINK_DOWN                        5
#define STATUS_TIMEOUT                          0xffff
/*******************************************************************************
 * Receive filter masks.
 ******************************************************************************/
#define RX_MASK_ACCEPT_NONE                     0x0000
#define RX_MASK_ACCEPT_MULTICAST                0x0002
#define RX_MASK_ACCEPT_ALL_MULTICAST            0x0004
#define RX_MASK_ACCEPT_BROADCAST                0x0008
#define RX_MASK_PROMISCUOUS_MODE                0x10000
/*******************************************************************************
 * media speed.
 ******************************************************************************/
#define MEDIUM_SPEED_AUTONEG                    0x0000L
#define MEDIUM_SPEED_1000MBPS                   0x0300L
#define MEDIUM_SPEED_2500MBPS                   0x0400L
#define MEDIUM_SPEED_10GBPS                     0x0600L
#define MEDIUM_SPEED_25GBPS                     0x0800L
#define MEDIUM_SPEED_40GBPS                     0x0900L
#define MEDIUM_SPEED_50GBPS                     0x0a00L
#define MEDIUM_SPEED_100GBPS                    0x0b00L
#define MEDIUM_SPEED_200GBPS                    0x0c00L
#define MEDIUM_SPEED_MASK                       0xff00L
#define GET_MEDIUM_SPEED(m)                     ((m) & MEDIUM_SPEED_MASK)
#define SET_MEDIUM_SPEED(bp, s) (((bp)->medium & ~MEDIUM_SPEED_MASK) | (s))
#define MEDIUM_UNKNOWN_DUPLEX                   0x00000L
#define MEDIUM_FULL_DUPLEX                      0x00000L
#define MEDIUM_HALF_DUPLEX                      0x10000L
#define GET_MEDIUM_DUPLEX(m)                    ((m) & MEDIUM_HALF_DUPLEX)
#define SET_MEDIUM_DUPLEX(bp, d) (((bp)->medium & ~MEDIUM_HALF_DUPLEX) | (d))
#define MEDIUM_SELECTIVE_AUTONEG                0x01000000L
#define GET_MEDIUM_AUTONEG_MODE(m)              ((m) & 0xff000000L)
#define GRC_COM_CHAN_BASE                       0
#define GRC_COM_CHAN_TRIG                       0x100
#define HWRM_CMD_DEFAULT_TIMEOUT                500 /* in Miliseconds  */
#define HWRM_CMD_POLL_WAIT_TIME                 100 /* In MicroeSconds */
#define HWRM_CMD_DEFAULT_MULTIPLAYER(a)         ((a) * 10)
#define HWRM_CMD_FLASH_MULTIPLAYER(a)           ((a) * 100)
#define HWRM_CMD_FLASH_ERASE_MULTIPLAYER(a)     ((a) * 1000)
#define MAX_ETHERNET_PACKET_BUFFER_SIZE         1536
#define DEFAULT_NUMBER_OF_CMPL_RINGS            0x01
#define DEFAULT_NUMBER_OF_TX_RINGS              0x01
#define DEFAULT_NUMBER_OF_RX_RINGS              0x01
#define DEFAULT_NUMBER_OF_RING_GRPS             0x01
#define DEFAULT_NUMBER_OF_STAT_CTXS             0x01
#define NUM_RX_BUFFERS                          512
#define MAX_RX_DESC_CNT                         1024
#define MAX_TX_DESC_CNT                         512
#define MAX_CQ_DESC_CNT                         2048
#define TX_RING_DMA_BUFFER_SIZE (MAX_TX_DESC_CNT * sizeof(struct tx_bd_short))
#define RX_RING_DMA_BUFFER_SIZE \
	(MAX_RX_DESC_CNT * sizeof(struct rx_prod_pkt_bd))
#define CQ_RING_DMA_BUFFER_SIZE (MAX_CQ_DESC_CNT * sizeof(struct cmpl_base))
#define BNXT_DMA_ALIGNMENT                      256 //64
#define REQ_BUFFER_SIZE                         1024
#define RESP_BUFFER_SIZE                        1024
#define DMA_BUFFER_SIZE                         1024
#define LM_PAGE_BITS                            8
#define BNXT_RX_STD_DMA_SZ                      1536
#define NEXT_IDX(N, S)                          (((N) + 1) & ((S) - 1))
#define BD_NOW(bd, entry, len) (&((u8 *)(bd))[(entry) * (len)])
#define BNXT_CQ_INTR_MODE()                     RING_ALLOC_REQ_INT_MODE_POLL
#define BNXT_INTR_MODE()                        RING_ALLOC_REQ_INT_MODE_POLL
/* Set default link timeout period to 500 millseconds */
#define LINK_DEFAULT_TIMEOUT                    500
#define RX_MASK \
	(RX_MASK_ACCEPT_BROADCAST | \
	RX_MASK_ACCEPT_ALL_MULTICAST | \
	RX_MASK_ACCEPT_MULTICAST)
#define TX_RING_QID                             ((u16)bp->port_idx * 10)
#define RX_RING_QID                             0
#define LM_PAGE_SIZE                            LM_PAGE_BITS
#define virt_to_bus(a)                          ((dma_addr_t)(a))
#define REQ_BUF_SIZE_ALIGNED  ALIGN(REQ_BUFFER_SIZE,  BNXT_DMA_ALIGNMENT)
#define RESP_BUF_SIZE_ALIGNED ALIGN(RESP_BUFFER_SIZE, BNXT_DMA_ALIGNMENT)
#define DMA_BUF_SIZE_ALIGNED  ALIGN(DMA_BUFFER_SIZE,  BNXT_DMA_ALIGNMENT)
#define RX_STD_DMA_ALIGNED    ALIGN(BNXT_RX_STD_DMA_SZ, BNXT_DMA_ALIGNMENT)
#define PCI_COMMAND_INTX_DISABLE         0x0400 /* Interrupt disable */
#define TX_AVAIL(r)                      ((r) - 1)
#define NO_MORE_CQ_BD_TO_SERVICE         1
#define SERVICE_NEXT_CQ_BD               0
#define PHY_STATUS         0x0001
#define PHY_SPEED          0x0002
#define DETECT_MEDIA       0x0004
#define SUPPORT_SPEEDS     0x0008
#define str_1        "1"
#define str_2        "2"
#define str_2_5      "2.5"
#define str_10       "10"
#define str_20       "20"
#define str_25       "25"
#define str_40       "40"
#define str_50       "50"
#define str_100      "100"
#define str_gbps     "Gbps"
#define str_mbps     "Mbps"
#define str_unknown  "Unknown"
/* Broadcom ethernet driver nvm defines. */
/* nvm cfg 1 - MAC settings */
#define FUNC_MAC_ADDR_NUM                                       1
/* nvm cfg 203 - u32 link_settings */
#define LINK_SPEED_DRV_NUM                                      203
#define LINK_SPEED_DRV_MASK                                     0x0000000F
#define LINK_SPEED_DRV_SHIFT                                    0
#define LINK_SPEED_DRV_AUTONEG                                  0x0
#define LINK_SPEED_DRV_1G                                       0x1
#define LINK_SPEED_DRV_10G                                      0x2
#define LINK_SPEED_DRV_25G                                      0x3
#define LINK_SPEED_DRV_40G                                      0x4
#define LINK_SPEED_DRV_50G                                      0x5
#define LINK_SPEED_DRV_100G                                     0x6
#define LINK_SPEED_DRV_200G                                     0x7
#define LINK_SPEED_DRV_2_5G                                     0xE
#define LINK_SPEED_DRV_100M                                     0xF
/* nvm cfg 201 - u32 speed_cap_mask */
#define SPEED_CAPABILITY_DRV_1G                                 0x1
#define SPEED_CAPABILITY_DRV_10G                                0x2
#define SPEED_CAPABILITY_DRV_25G                                0x4
#define SPEED_CAPABILITY_DRV_40G                                0x8
#define SPEED_CAPABILITY_DRV_50G                                0x10
#define SPEED_CAPABILITY_DRV_100G                               0x20
#define SPEED_CAPABILITY_DRV_100M                               0x8000
/* nvm cfg 202 */
/* nvm cfg 205 */
#define LINK_SPEED_FW_NUM                                       205
/* nvm cfg 210 */
/* nvm cfg 211 */
/* nvm cfg 213 */
#define SPEED_DRV_MASK    LINK_SPEED_DRV_MASK
/******************************************************************************
 * Doorbell info.
 *****************************************************************************/
#define RX_DOORBELL_KEY_RX    (0x1UL << 28)
#define TX_DOORBELL_KEY_TX    (0x0UL << 28)

#define CMPL_DOORBELL_IDX_VALID     0x4000000UL
#define CMPL_DOORBELL_KEY_CMPL  (0x2UL << 28)

/******************************************************************************
 * Transmit info.
 *****************************************************************************/
struct tx_bd_short {
	u16 flags_type;
#define TX_BD_SHORT_TYPE_TX_BD_SHORT        0x0UL
#define TX_BD_SHORT_FLAGS_PACKET_END        0x40UL
#define TX_BD_SHORT_FLAGS_NO_CMPL           0x80UL
#define TX_BD_SHORT_FLAGS_BD_CNT_SFT        8
#define TX_BD_SHORT_FLAGS_LHINT_LT512       (0x0UL << 13)
#define TX_BD_SHORT_FLAGS_LHINT_LT1K        (0x1UL << 13)
#define TX_BD_SHORT_FLAGS_LHINT_LT2K        (0x2UL << 13)
#define TX_BD_SHORT_FLAGS_LHINT_GTE2K       (0x3UL << 13)
#define TX_BD_SHORT_FLAGS_COAL_NOW          0x8000UL
	u16 len;
	u32 opaque;
	union dma_addr64_t dma;
};

struct lm_tx_info_t {
	void             *bd_virt;
	u16              prod_id;  /* Tx producer index. */
	u16              cons_id;
	u16              ring_cnt;
	u32              cnt;      /* Tx statistics. */
	u32              cnt_req;
};

struct cmpl_base {
	u16 type;
#define CMPL_BASE_TYPE_MASK              0x3fUL
#define CMPL_BASE_TYPE_TX_L2             0x0UL
#define CMPL_BASE_TYPE_RX_L2             0x11UL
#define CMPL_BASE_TYPE_STAT_EJECT        0x1aUL
#define CMPL_BASE_TYPE_HWRM_ASYNC_EVENT  0x2eUL
	u16 info1;
	u32 info2;
	u32 info3_v;
#define CMPL_BASE_V          0x1UL
	u32 info4;
};

struct lm_cmp_info_t {
	void      *bd_virt;
	u16       cons_idx;
	u16       ring_cnt;
	u8        completion_bit;
	u8        res[3];
};

struct rx_pkt_cmpl {
	u16 flags_type;
	u16 len;
	u32 opaque;
	u8  agg_bufs_v1;
	u8  rss_hash_type;
	u8  payload_offset;
	u8  unused1;
	u32 rss_hash;
};

struct rx_pkt_cmpl_hi {
	u32 flags2;
	u32 metadata;
	u16 errors_v2;
#define RX_PKT_CMPL_V2                        0x1UL
#define RX_PKT_CMPL_ERRORS_BUFFER_ERROR_SFT   1
	u16 cfa_code;
	u32 reorder;
};

struct rx_prod_pkt_bd {
	u16 flags_type;
#define RX_PROD_PKT_BD_TYPE_RX_PROD_PKT   0x4UL
	u16  len;
	u32 opaque;
	union dma_addr64_t dma;
};

struct lm_rx_info_t {
	void                   *bd_virt;
	void                   *iob[NUM_RX_BUFFERS];
	void                   *iob_rx;
	u16                    iob_len;
	u16                    iob_recv;
	u16                    iob_cnt;
	u16                    buf_cnt; /* Total Rx buffer descriptors. */
	u16                    ring_cnt;
	u16                    cons_idx; /* Last processed consumer index. */
	u32                    rx_cnt;
	u32                    rx_buf_cnt;
	u32                    err;
	u32                    crc;
	u32                    dropped;
};

#define VALID_DRIVER_REG          0x0001
#define VALID_STAT_CTX            0x0002
#define VALID_RING_CQ             0x0004
#define VALID_RING_TX             0x0008
#define VALID_RING_RX             0x0010
#define VALID_RING_GRP            0x0020
#define VALID_VNIC_ID             0x0040
#define VALID_RX_IOB              0x0080
#define VALID_L2_FILTER           0x0100

enum RX_FLAGS {
	PKT_DONE = 0,
	PKT_RECEIVED = 1,
	PKT_DROPPED = 2,
};

struct bnxt {
	struct udevice             *pdev;
	const char                 *name;
	unsigned int               cardnum;
	void                       *hwrm_addr_req;
	void                       *hwrm_addr_resp;
	void                       *hwrm_addr_data;
	dma_addr_t                 data_addr_mapping;
	dma_addr_t                 req_addr_mapping;
	dma_addr_t                 resp_addr_mapping;
	struct lm_tx_info_t        tx;    /* Tx info. */
	struct lm_rx_info_t        rx;    /* Rx info. */
	struct lm_cmp_info_t       cq;    /* completion info. */
	u16                        last_resp_code;
	u16                        seq_id;
	u32                        flag_hwrm;
	u32                        flags;
	u16                        vendor_id;
	u16                        device_id;
	u16                        subsystem_vendor;
	u16                        subsystem_device;
	u16                        cmd_reg;
	u8                         irq;
	void __iomem              *bar0;
	void __iomem              *bar1;
	void __iomem              *bar2;
	u16                       chip_num;
	/* chip num:16-31, rev:12-15, metal:4-11, bond_id:0-3 */
	u32                       chip_id;
	u32                       hwrm_cmd_timeout;
	u16                       hwrm_spec_code;
	u16                       hwrm_max_req_len;
	u16                       hwrm_max_ext_req_len;
	u8                        fw_maj;
	u8                        fw_min;
	u8                        fw_bld;
	u8                        fw_rsvd;
	u8                        mac_addr[ETH_ALEN]; /* HW MAC address */
	u8                        mac_set[ETH_ALEN];  /* NVM Configured MAC */
	u16                       fid;
	u8                        port_idx;
	u8                        ordinal_value;
	u16                       mtu;
	u16                       ring_grp_id;
	u16                       cq_ring_id;
	u16                       tx_ring_id;
	u16                       rx_ring_id;
	u16                       current_link_speed;
	u16                       link_status;
	u16                       wait_link_timeout;
	u64                       l2_filter_id;
	u16                       vnic_id;
	u16                       stat_ctx_id;
	u32                       medium;
	u16                       support_speeds;
	u32                       link_set;
	u8                        media_detect;
	u8                        media_change;
	u16                       max_vfs;
	u16                       vf_res_strategy;
	u16                       min_vnics;
	u16                       max_vnics;
	u16                       max_msix;
	u16                       min_hw_ring_grps;
	u16                       max_hw_ring_grps;
	u16                       min_tx_rings;
	u16                       max_tx_rings;
	u16                       min_rx_rings;
	u16                       max_rx_rings;
	u16                       min_cp_rings;
	u16                       max_cp_rings;
	u16                       min_rsscos_ctxs;
	u16                       max_rsscos_ctxs;
	u16                       min_l2_ctxs;
	u16                       max_l2_ctxs;
	u16                       min_stat_ctxs;
	u16                       max_stat_ctxs;
	u16                       num_cmpl_rings;
	u16                       num_tx_rings;
	u16                       num_rx_rings;
	u16                       num_stat_ctxs;
	u16                       num_hw_ring_grps;
	bool                      card_en;
};

#define SHORT_CMD_SUPPORTED   VER_GET_RESP_DEV_CAPS_CFG_SHORT_CMD_SUPPORTED
#define SHORT_CMD_REQUIRED    VER_GET_RESP_DEV_CAPS_CFG_SHORT_CMD_REQUIRED
#define CQ_DOORBELL_KEY_IDX(a) \
	(CMPL_DOORBELL_KEY_CMPL | \
	CMPL_DOORBELL_IDX_VALID | \
	(u32)(a))
#define TX_BD_FLAGS \
	(TX_BD_SHORT_TYPE_TX_BD_SHORT | \
	TX_BD_SHORT_FLAGS_NO_CMPL | \
	TX_BD_SHORT_FLAGS_COAL_NOW | \
	TX_BD_SHORT_FLAGS_PACKET_END | \
	(1 << TX_BD_SHORT_FLAGS_BD_CNT_SFT))
#define MEM_HWRM_RESP memalign(BNXT_DMA_ALIGNMENT, RESP_BUF_SIZE_ALIGNED)
#define PORT_PHY_FLAGS (BNXT_FLAG_NPAR_MODE | BNXT_FLAG_MULTI_HOST)
#define RING_FREE(bp, rid, flag) bnxt_hwrm_ring_free(bp, rid, flag)
#define QCFG_PHY_ALL (SUPPORT_SPEEDS | DETECT_MEDIA | PHY_SPEED | PHY_STATUS)

#endif /* _BNXT_H_ */
