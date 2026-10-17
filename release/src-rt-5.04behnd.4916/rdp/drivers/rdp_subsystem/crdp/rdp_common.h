/*
    <:copyright-BRCM:2015:DUAL/GPL:standard

       Copyright (c) 2015 Broadcom
       All Rights Reserved

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

    :>
*/

#ifndef RDP_COMMON_H_INCLUDED
#define RDP_COMMON_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include "bdmf_shell.h"

#include "rdd_crc.h"
#include "rdp_platform.h"
#include "bcm_rdp_arch.h"

#if defined(DEBUG_DAEMON) && !defined(RDP_SIM)
#include "rdd_debug_daemon.h"
#endif

#if defined(__KERNEL__)
#include <bcm_pkt_lengths.h>
#endif

/* Compatibility with non TM C code */
#ifndef TM_C_CODE
#define TM_MAX_GROUP    1
#endif

/* default values for PARSER */

#define PARSER_EXCP_IP_HDR_LEN_ERR      (1 << 0)
#define PARSER_EXCP_CHKSUM_ERR          (1 << 1)
#define PARSER_EXCP_ETH_MULCST          (1 << 2)
#define PARSER_EXCP_IP_MULCST           (1 << 3)
#define PARSER_EXCP_IP_FRAGMENT         (1 << 4) /* including first, middle or last fragment packet */
#define PARSER_EXCP_IP_VERSION_ERR      (1 << 5)
#define PARSER_EXCP_IP_MULCST_CTRL      (1 << 6) /* Set when Mcast Layer 3 Control match, identified by IP match */
#define PARSER_EXCP_ETH_BCAST           (1 << 7)
#define PARSER_EXCP_HDR_PARSE_ERR       (1 << 8) /* not enough bytes in the header to complete parsing of the packet */
#define PARSER_EXCP_IP_PKT_LEN_ERR      (1 << 9)
#define PARSER_EXCP_IP_L2_MULCST        (1 << 10) /* Set when Mcast Layer 2 match, identified by DA filter match */
#define PARSER_EXCP_L4_INV_5_TUPPLE     (1 << 11) /* Set when !(L4 proto=TCP|UDP|GRE|ESP|4in6|6in4) */
                                                  /* ASM FW: bit must be enabled to change TOS from 0 to 0xFF in exception path */
#define PARSER_EXCP_UDP_1588            (1 << 12)
#define PARSER_EXCP_DHCP                (1 << 13)
#define PARSER_EXCP_DOS_ATTACK          (1 << 14)
#define PARSER_EXCP_DNS                 (1 << 15)
#define PARSER_EXCP_GRE_VER_ERR         (1 << 16) /* GRE withe unrecgonized version (not 0 or 1) */
#define PARSER_EXCP_STATUS_BITS     (PARSER_EXCP_IP_HDR_LEN_ERR | PARSER_EXCP_CHKSUM_ERR | PARSER_EXCP_ETH_MULCST | PARSER_EXCP_IP_MULCST \
                                    | PARSER_EXCP_IP_FRAGMENT | PARSER_EXCP_IP_VERSION_ERR | PARSER_EXCP_IP_MULCST_CTRL | PARSER_EXCP_ETH_BCAST \
                                    | PARSER_EXCP_HDR_PARSE_ERR | PARSER_EXCP_IP_PKT_LEN_ERR |  PARSER_EXCP_IP_L2_MULCST | PARSER_EXCP_L4_INV_5_TUPPLE \
                                    | PARSER_EXCP_UDP_1588 | PARSER_EXCP_DHCP | PARSER_EXCP_DOS_ATTACK | PARSER_EXCP_DNS | PARSER_EXCP_GRE_VER_ERR)



#define PARSER_ENG_CFG_IP_FILTERS_IPV6_LSB          (1 << 0)
#define PARSER_ENG_CFG_LLC_SNAP                     (1 << 2)
#define PARSER_ENG_CFG_PPP_CODE_1_IPV6              (1 << 3)
#define PARSER_ENG_CFG_DIS_HDR_LEN_ERR              (1 << 4)
#define PARSER_ENG_CFG_IP_VER_ALWAYS_4              (1 << 5)
#define PARSER_ENG_CFG_ENABLE_IP_CHECKSUM_MISMATCH  (1 << 6)
#define PARSER_ENG_CFG_ENABLE_ICMP_IPV6             (1 << 7)
#define PARSER_ENG_CFG_IPV6_OR_HDR_ERR_COND         (1 << 8)
#define PARSER_ENG_CFG_ENABLE_IPV6_HOP_BY_HOP       (1 << 9)
#define PARSER_ENG_CFG_SELECT_MAC_MODE              (1 << 10)
#define PARSER_ENG_CFG_IPV6_MCAST_MODE_1            (1 << 11)
#define PARSER_ENG_CFG_DIS_IPV4_PAD_LEN_ERR         (1 << 13)
#define PARSER_ENG_CFG_DIS_IPV6_PAD_LEN_ERR         (1 << 14)
#define PARSER_ENG_CFG_AH_IPV6                      (1 << 15)
#define PARSER_ENG_CFG_AH_IPV4                      (1 << 16)
#define PARSER_ENG_CFG_IPV4_CHEKSUM_NO_0xFFFF       (1 << 18)
#define PARSER_ENG_CFG_DIS_IPV4_OVER_IPV6           (1 << 19)
#define PARSER_ENG_CFG_DIS_IPV6_OVER_IPV4           (1 << 20)
#define PARSER_ENG_CFG_YL_MODE                      (1 << 21)
#define PARSER_ENG_CFG_ENABLE_MC_BC_BITS_IC_KEY     (1 << 22)
#define PARSER_ENG_CFG_MASK_L2_L3_VALID_KEY         (1 << 23)

#define PARSER_ENG_CFG_IPV4_ZERO_DETECT             (1 << 24)
#define PARSER_ENG_CFG_IPV6_ZERO_DETECT             (1 << 25)

/* eng_conf register default configuration*/
#define PARSER_ENG_CFG_DEFAULT          (PARSER_ENG_CFG_AH_IPV4 | PARSER_ENG_CFG_AH_IPV6 | PARSER_ENG_CFG_LLC_SNAP | PARSER_ENG_CFG_ENABLE_IP_CHECKSUM_MISMATCH | PARSER_ENG_CFG_PPP_CODE_1_IPV6 | PARSER_ENG_CFG_ENABLE_MC_BC_BITS_IC_KEY)

#define PARSER_DOS_REASON_MASK              0x3FFF
#define PARSER_DOS_REASON_MAC_SPOOF         (1 << 0)
#define PARSER_DOS_REASON_IP_LAND           (1 << 1)
#define PARSER_DOS_REASON_TCP_BLAT          (1 << 2)
#define PARSER_DOS_REASON_UDP_BLAT          (1 << 3)
#define PARSER_DOS_REASON_TCP_NULL_SCAN     (1 << 4)
#define PARSER_DOS_REASON_TCP_XMAS_SCAN     (1 << 5)
#define PARSER_DOS_REASON_TCP_SYNFIN_SCAN   (1 << 6)
#define PARSER_DOS_REASON_TCP_SYN_ERROR     (1 << 7)
#define PARSER_DOS_REASON_TCP_SHORT_HDR     (1 << 8)
#define PARSER_DOS_REASON_TCP_FRAG_ERROR    (1 << 9)
#define PARSER_DOS_REASON_ICMPV4_FRAGMENT   (1 << 10)
#define PARSER_DOS_REASON_ICMPV6_FRAGMENT   (1 << 11)
#define PARSER_DOS_REASON_ICMPV4_LONG_PING  (1 << 12)
#define PARSER_DOS_REASON_ICMPV6_LONG_PING  (1 << 13)
/* disable blat as default as used by QA, enable from system configuration */
#define PARSER_DOS_REASON_MASK_BITS         (PARSER_DOS_REASON_MAC_SPOOF | PARSER_DOS_REASON_IP_LAND | PARSER_DOS_REASON_TCP_NULL_SCAN | PARSER_DOS_REASON_TCP_XMAS_SCAN | PARSER_DOS_REASON_TCP_SYNFIN_SCAN | \
                                            PARSER_DOS_REASON_TCP_SYN_ERROR | PARSER_DOS_REASON_TCP_SHORT_HDR | PARSER_DOS_REASON_TCP_FRAG_ERROR | PARSER_DOS_REASON_ICMPV4_FRAGMENT | \
                                            PARSER_DOS_REASON_ICMPV6_FRAGMENT | PARSER_DOS_REASON_ICMPV4_LONG_PING | PARSER_DOS_REASON_ICMPV6_LONG_PING)

#define PARSER_MAX_ICMPV4_SIZE              576   /* rfc1812 section 4.3.2.3 */
#define PARSER_MAX_ICMPV6_SIZE              1500  /* MTU */
#define PARSER_PPP_PROTOCOL_CODE_0_IPV4     0x21
#define PARSER_PPP_PROTOCOL_CODE_1_IPV6     0x57
#define PARSER_IP_PROTOCOL_IPIP             4
#define PARSER_PROFILE_US                   (1 << 1) /* Profile 0 for DS, profile 1 for US */
#define PARSER_PROFILE_US_PROP_TAG          (1 << 2) /* Profile 2 - proprietary tag */
#define PARSER_TCP_CTL_FLAGS                0x07
#define PARSER_MAC_MODE                     0x400

#ifdef DQM_MINI_FPM_MODE
#if defined(RDP_ARCH_SIM)
#define MINI_FPM_DQM_DDR_SEGMENT_SIZE   (1*512*1024) /* 1/2MB */
#else
#define MINI_FPM_DQM_DDR_SEGMENT_SIZE   (8*1024*1024) /* 8MB */
#endif
#endif

#if defined(RDP_ARCH_SIM) || defined(RDP_ARCH_QEMU_SIM)
#define RDP_NO_RUNNER_HW
#elif defined(RDP_ARCH_BOARD) || defined (RDP_ARCH_BOOT)
#else
#error "no arch defined"
#endif /* RDP_ARCH */

typedef enum
{
    DMA_BUFSIZE_128  = 0,
    DMA_BUFSIZE_256  = 1,
    DMA_BUFSIZE_320  = 1,
    DMA_BUFSIZE_512  = 2,
    DMA_BUFSIZE_640  = 2,
    DMA_BUFSIZE_1024 = 3,
    DMA_BUFSIZE_1280 = 3,
    DMA_BUFSIZE_2048 = 4,
    DMA_BUFSIZE_2560 = 4,
} drv_rnr_dma_bufsize_t;

typedef enum  ddr_buf_size_e
{
    BUF_256 = 0,
    BUF_512,
    BUF_1024,
    BUF_2048,
    BUF_320,
    BUF_640,
    BUF_1280,
    BUF_2560
} ddr_buf_size_e;

#define BBH_LAN_TX_FIFO_SIZE            8

void lookup_bbh_tx_bufsz_by_fpm_bufsz(uint32_t *fpm_bufsize, uint8_t *bbh_tx_bufsize);
void lookup_dma_bufsz_by_fpm_bufsz(uint32_t *fpm_bufsz, uint8_t* dma_bufsz);
void rdpa_system_group_allocation_defaults_set(uint8_t us, uint8_t ds, uint8_t wlan);
void rdpa_system_bufmng_allocation_defaults_set(void);

typedef struct
{
    uint32_t low;
    uint8_t high;
} ddr_addr;

typedef enum
{
    bcm_tag_opcode0,
    bcm_tag_opcode1
} bcm_tag_t;

typedef enum
{
    hash_max_16_entries_per_engine,
    hash_max_32_entries_per_engine,
    hash_max_64_entries_per_engine,
    hash_max_128_entries_per_engine,
    hash_max_256_entries_per_engine,
    hash_max_512_entries_per_engine,
    hash_max_1k_entries_per_engine,
    hash_max_1_125k_entries_per_engine = 8,
    hash_max_1_25k_entries_per_engine = 9,
} tbl_size_e;

typedef struct
{
    uint32_t us_total_fpm_tokens;      /**< define the max number of fpm tokens assigned to UG */
    uint16_t us_high_prio_rsv_percent; /**< fpm tokens reserved to high priority traffic */
    uint16_t us_excl_prio_rsv_percent; /**< fpm tokens reserved to exclusive priority traffic */
    uint32_t ds_total_fpm_tokens;      /**< define the max number of fpm tokens assigned to UG */
    uint16_t ds_high_prio_rsv_percent; /**< fpm tokens reserved to high priority traffic */
    uint16_t ds_excl_prio_rsv_percent; /**< fpm tokens reserved to exclusive priority traffic */
    uint32_t cpu_wlan_total_fpm_tokens;      /**< define the max number of fpm tokens assigned to UG */
    uint16_t cpu_wlan_high_prio_rsv_percent; /**< fpm tokens reserved to high priority traffic */
    uint16_t cpu_wlan_excl_prio_rsv_percent; /**< fpm tokens reserved to exclusive priority traffic */
    uint32_t mbr_rsv_fpm_tokens;       /**< define the max number of fpm tokens assigned to MBR */
} fpm_token_allocation_t;

typedef struct
{
    uint32_t wlan_rsv_fpm_tokens;   /**< number of tokens reserved to wlan group */
    uint32_t wlan_max_fpm_tokens;   /**< max number of tokens for wlan group */
    uint32_t be1_rsv_fpm_tokens;    /**< number of tokens reserved to be1 group */
    uint32_t be1_max_fpm_tokens;    /**< max number of tokens for be1 group */
    uint32_t be2_rsv_fpm_tokens;    /**< number of tokens reserved to be2 group */
    uint32_t be2_max_fpm_tokens;    /**< max number of tokens for be2 group */
    uint32_t phy_rsv_fpm_tokens;    /**< number of tokens reserved to phy group */
    uint32_t phy_max_fpm_tokens;    /**< max number of tokens for phy group */
} bufmng_group_allocation_t;

typedef struct
{
    uint32_t wlan_norm_rsv_fpm_tokens[4];  /**< number of tokens reserved to wlan normal priority */
    uint32_t wlan_high_rsv_fpm_tokens[4];  /**< number of tokens reserved to wlan high priority */
    uint32_t cpu_norm_rsv_fpm_tokens;      /**< number of tokens reserved to cpu normal priority */
    uint32_t cpu_high_rsv_fpm_tokens;      /**< number of tokens reserved to cpu high priority */
} bufmng_port_reservation_t;

typedef struct
{
    uint32_t max_pkt_size;
    uint32_t headroom_size;
    bcm_tag_t bcmsw_tag;
    bdmf_boolean is_gateway;
    ddr_addr rdp_phy_ddr_rnr_tables_base; /* runner tables DDR base address */
    void *rdp_ddr_rnr_tables_base_virt;   /* runner tables DDR0 virtual base address */
    bdmf_phys_addr_t rdp_ddr_rnr_tables_base_phys; /* runner tables DDR0 physical base address */
    uint32_t rnr_tables_mem_size;
    uint32_t rnr_tables_mem_used;
#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
    ddr_addr rdp_phy_ddr_ddos_tables_base; /* runner tables DDR base address */
    void *rdp_ddr_ddos_tables_base_virt;   /* runner tables DDR0 virtual base address */
    bdmf_phys_addr_t rdp_ddr_ddos_tables_base_phys; /* runner tables DDR0 physical base address */
    uint32_t ddos_tables_mem_size;
#endif
    void *rdp_ddr_pkt_base_virt;          /* fpm pool DDR0 virtual base address for unicast packets */
    bdmf_phys_addr_t rdp_ddr_pkt_base_phys;  /* fpm pool DDR0 physical base address for unicast packets */
    uint32_t fpm_buf_size;
    uint32_t fpm_pool_size_tokens_set;  /* defines mutiplicators set for pools */
    int xfi_port;
    uint32_t bbh_id_gbe_wan; /* bbh_id for gbe port */
    uint16_t number_of_ds_queues; /**< define the number of queue for DS queues */
    uint16_t number_of_us_queues; /**< define the number of queue for US queues */
    uint16_t number_of_service_queues; /**< define the number of queue for SERVICE queues */
    fpm_token_allocation_t fpm_tokens_allocation; /**< define allocations for US/DS/CPU/WLAN */
    bufmng_group_allocation_t bufmng_group_allocation; /**< define allocations of buffer mananager groups */
    bufmng_port_reservation_t bufmng_port_reservation;  /**< define FPM token reservations for ports */
    bdmf_boolean fpm_token_allocation_user_mode; /**< use user mode UG*/
    uint8_t num_of_wlan_ports;
    uint8_t num_of_dhd_radios;  /**< active number of DHD radios */
    uint16_t dhd_rsv_tokens[4];
    uint8_t xepon_on;
    uint32_t g9991_port_vec;
    uint32_t g9991_bbh_vec;
    tbl_size_e iptv_table_size;
    tbl_size_e arl_table_size;
    uint32_t dhd_offload_bitmask;
    uint32_t fpm_pool_memory_size;
    rdpa_usxgmiim_cfg_t bbh_tx_port_cfg;
    ports_profile_t port_profiles[BBH_ID_NUM];
    rdpa_tm_core_emac_map tm_core_emac_map;
    uint32_t active_pon_ports_count;  /**< define the number active pysical WAN ports (not emac) */
    /* XPE */
    uint16_t xpe_prog_addr; /* Address of the XPE func in the first processing image.
                             *  Assumption is that address will be same for all images */
    uint8_t xpe_prog_size;
} dpi_params_t;

typedef struct bbh_to_dma_x
{
    bbh_id_e bbh_id;
    dma_id_e dma_id;
} bbh_to_dma_x_t;

/* QM queue index */
typedef uint16_t  rdp_qm_queue_idx_t;

/* FPM user group */
typedef enum
{
    FPM_DS_UG = 0,
    FPM_US_UG = 1,
    FPM_WLAN_UG = 2,
    FPM_ALL_PASS_UG = 3,
} fpm_ug_id_e;

/* Peripheral */
typedef enum
{
    QM_PERIPH_ID_ETH0 = 0,
    QM_PERIPH_ID_ETH1 = 1,
    QM_PERIPH_ID_ETH2 = 2,
    QM_PERIPH_ID_ETH3 = 3,
    QM_PERIPH_ID_GPON = 4,
    QM_PERIPH_ID_EPON = 5,

    QM_PERIPH_ID__NUM_OF = 6
} qm_periph_id_e;

extern struct bdmfmon_enum_val bbh_id_enum_table[];
extern struct bdmfmon_enum_val bbh_id_tx_enum_table[];
extern struct bdmfmon_enum_val dma_id_enum_table[];
extern struct bdmfmon_enum_val tm_identifier_enum_table[];
extern struct bdmfmon_enum_val rnr_id_enum_table[];
extern struct bdmfmon_enum_val quad_idx_enum_table[];
extern struct bdmfmon_enum_val tbl_idx_enum_table[];
extern struct bdmfmon_enum_val eng_idx_enum_table[];
extern struct bdmfmon_enum_val ubus_mstr_id_enum_table[];
extern struct bdmfmon_enum_val channel_id_enum_table[];
extern struct bdmfmon_enum_val umac_misc_id_enum_table[];
#define umac_id_enum_table umac_misc_id_enum_table
#define umac_mib_id_enum_table umac_misc_id_enum_table
extern struct bdmfmon_enum_val bacif_id_enum_table[];

typedef enum  ddr_byte_res_e
{
    RES_1B = 0,
    RES_2B
} ddr_byte_res_e;

typedef enum natc_tbl_id_e
{
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

typedef enum natc_eng_id_e
{
    NATC_ENG0_ID = 0,
    NATC_ENG1_ID,
    NATC_ENG2_ID,
    NATC_ENG3_ID,
    NATC_ENG_ID_LAST = NATC_ENG3_ID,
} natc_eng_id_e;

typedef enum ubus_mstr_id_e
{
    UBUS_MSTR0_ID = 0,
    UBUS_MSTR1_ID,
    UBUS_MSTR_ID_LAST = UBUS_MSTR1_ID,
} ubus_mstr_id_e;

typedef enum xlif_channel_id_e
{
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
#define BB_ID_CPU3 (BB_ID_LAST + 4)
#define BB_ID_CPU4 (BB_ID_LAST + 5)  /* WLAN0 */
#define BB_ID_CPU5 (BB_ID_LAST + 6)  /* WLAN1 */
#define BB_ID_CPU6 (BB_ID_LAST + 7)  /* WLAN2 */

/* HASH key masks */
/* IPTV */
#define HASH_TABLE_IPTV_KEY_MASK_LO  0xFFFFFFFF /* 48bit key */
#define HASH_TABLE_IPTV_KEY_MASK_HI  0x0000FFFF /* Only 12 bits of internal context are used */

/* ARL */
#define HASH_TABLE_ARL_KEY_MASK_LO  0xFFFFFFFF /* 60bit key */
#define HASH_TABLE_ARL_KEY_MASK_HI  0xFFFFFFF

/* Bridge and VLAN */
#define HASH_TABLE_BRIDGE_AND_VLAN_LKP_MASK_LO  0x000FFFFF /* 20bit key */
#define HASH_TABLE_BRIDGE_AND_VLAN_LKP_MASK_HI  0x0

#if RDPA_MAX_VLANS == 256
#define HASH_TABLE_BRIDGE_AND_VLAN_LKP_SIZE_PER_ENGINE  hash_max_64_entries_per_engine
#elif (RDPA_MAX_VLANS == 128 || RDPA_MAX_VLANS == 32)
#define HASH_TABLE_BRIDGE_AND_VLAN_LKP_SIZE_PER_ENGINE  hash_max_32_entries_per_engine
#else
#error wrong configuration for num of vlans
#endif

/* NAT Cache default mask, include vport */
#if defined(CONFIG_RNR_UNKNOWN_UCAST_FLOODING) && !defined(PKTRUNNER_IMPL2)
#define NATC_KEY0_DEF_MASK  0xffffffff
#else
#define NATC_KEY0_DEF_MASK  0x80ffffff
#endif
#if !defined(__KERNEL__)
typedef uint32_t bool;
#endif
/* DDR token info datastructure and APIs */
typedef DDR_TOKEN_INFO_ENTRY_STRUCT ddr_token_info_t;


#ifdef __cplusplus
}
#endif


#ifdef USE_BDMF_SHELL
#define HAL_CLI_PRINT_LIST(session, blk_name, cfg_arr) \
    do { \
        int i, rc; \
        bdmfmon_cmd_parm_t cmd_parm[2] = {}; \
        for (i = 0, rc = 0; !rc && i < ARRAY_LENGTH(cfg_arr); i++) \
        { \
            cmd_parm[0].value.unumber = cfg_arr[i]; \
            rc = bcm_##blk_name##_cli_get(session, cmd_parm, 1); \
            if (rc) \
                bdmf_session_print(session, "Callback of %s with parameter %d failed with return code %d\n", "##blk_name##", cfg_arr[i], rc); \
        } \
    }while (0);

#define HAL_CLI_PRINT_LIST_AND_TITLES(session, blk_name, titles, cfg_arr, num_of) \
    do { \
        int i, rc; \
        bdmfmon_cmd_parm_t cmd_parm[2] = {}; \
        for (i = 0, rc = 0; !rc && i < num_of; i++) \
        { \
            cmd_parm[0].value.unumber = cfg_arr[i]; \
            bdmf_session_print(session, "==>%s\n", titles[i]); \
            rc = bcm_##blk_name##_cli_get(session, cmd_parm, 1); \
            if (rc) \
                bdmf_session_print(session, "Callback of %s with parameter %d failed with return code %d\n", "##blk_name##", cfg_arr[i], rc); \
        } \
    }while (0);

#define HAL_CLI_IDX_PRINT_LIST(session, blk_name, cfg_arr, idx) \
    do { \
        int i, rc; \
        bdmfmon_cmd_parm_t cmd_parm[3] = {}; \
        for (i = 0, rc = 0; !rc && i < ARRAY_LENGTH(cfg_arr); i++) \
        { \
            cmd_parm[0].value.unumber = cfg_arr[i]; \
            cmd_parm[1].value.unumber = idx; \
            rc = bcm_##blk_name##_cli_get(session, cmd_parm, 2); \
            if (rc) \
                    bdmf_session_print(session, "Callback of %s with parameters %d, %d failed with return code %d\n", "##blk_name##", cfg_arr[i], idx, rc); \
        } \
    }while (0);

#define HAL_CLI_PRINT_NUM_OF_LIST(session, blk_name, cfg_arr, n_list) \
    do { \
        int i, j, rc; \
        bdmfmon_cmd_parm_t cmd_parm[3] = {}; \
        for (i = 0; i < ARRAY_LENGTH(cfg_arr); i++) \
        { \
            cmd_parm[0].value.unumber = cfg_arr[i]; \
            for (j = 0,  rc = 0; !rc && j < n_list; j++) \
            { \
                cmd_parm[1].value.unumber = j; \
                bdmf_session_print(session, "\nCfg index %d:\n", j); \
                rc = bcm_##blk_name##_cli_get(session, cmd_parm, 2); \
                if (rc) \
                    bdmf_session_print(session, "Callback of %s with parameters %d, %d failed with return code %d\n", "##blk_name##", cfg_arr[i], j, rc); \
            } \
        } \
    }while (0);
#endif

#define IS_POWER_OF_2_INTEGER(val) (!(((val)-1) & (val)))
#define CONST_INT_1K        (0x400)
#define CONST_INT_2K        (0x800)
#define CONST_INT_4K       (0x1000)
#define CONST_INT_8K       (0x2000)
#define CONST_INT_16K      (0x4000)
#define CONST_INT_32K      (0x8000)
#define CONST_INT_64K     (0x10000)
#define CONST_INT_128K    (0x20000)
#define CONST_INT_256K    (0x40000)
#define CONST_INT_512K    (0x80000)
#define CONST_INT_1M     (0x100000)
#define CONST_INT_2M     (0x200000)
#define CONST_INT_4M     (0x400000)
#define CONST_INT_8M     (0x800000)
#define CONST_INT_16M   (0x1000000)
#define CONST_INT_32M   (0x2000000)
#define CONST_INT_64M   (0x4000000)
#define CONST_INT_128M  (0x8000000)
#define CONST_INT_256M (0x10000000)
#define CONST_INT_512M (0x20000000)
#define CONST_INT_1G   (0x40000000)
#define CONST_INT_2G   (0x80000000)

#define BB_RX_ID_NOT_VALID 0xff
#define PORT_MAPPING_ENTRY_IS_VALID(index) (emac_port_mapping[index].bb_rx_id != BB_RX_ID_NOT_VALID)

typedef struct {
    uint8_t bb_rx_id;
    uint8_t bbh_tx_id;
    uint8_t bb_tx_id;
    int bbh_queue;
    uint8_t tx_runner_core;
#if defined(MULTIPLE_BBH_TX_LAN)
    uint8_t core_index;
#endif    
    uint8_t viq;
    uint8_t tm_ds_index; /* used for multiple DS TM cores / incase of US value is 0 */
    uint8_t tm_index;  /* used for dynamic TM core according to ports rate */
#if defined(TM_C_CODE)
    uint8_t tm_identity;
#endif
    char name[20];
} port_mapping_t;

extern uint32_t port_vport_mapping[rdpa_port_type__num_of][PROJ_DEFS_NUMBER_OF_VPORTS];
extern port_mapping_t emac_port_mapping[];
#ifdef CONFIG_BCM_TCONT
extern port_mapping_t pon_port_mapping[];
#endif
#ifdef G9991_COMMON
extern rdpa_emac vport_to_emac[];
#endif
#if defined(SF2_SUPPORT)
extern port_mapping_t sf2_emac_port_mapping[];
#endif
#if defined(CONFIG_RNR_DSL_INTF_SUPPORT)
extern port_mapping_t dsl_port_mapping[];
#endif

/* port_mapping API */

/* DS channel (index which is used to access egress TM DS elements) */
static inline int16_t port_mapping_channel_get(rdpa_port_type type, uint32_t index)
{
    int16_t channel = 0;

    switch (type)
    {
    case rdpa_port_epon_ae:
        break;

    case rdpa_port_emac:
#ifdef G9991_COMMON
    case rdpa_port_sys_port:
        channel = port_vport_mapping[type][index];
#else
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }
#if defined(BCM63158)
        channel = emac_port_mapping[index].bbh_queue;
#else
        channel = index; /* (bbh_queue is not a unique number) */
#endif
#if defined(TM_CORE_LOAD_BALANCING)
        if (emac_port_mapping[index].tm_identity == TM_PON_DSL_AE)
            channel += RDD_US_CHANNEL_OFFSET_ETHWAN;
#endif
#endif
        break;

    case rdpa_port_sid:
    case rdpa_port_ctrl_sid:
        channel = index;
        break;

#if defined(SF2_SUPPORT)
    case rdpa_port_sf2_emac:
        if (!((1 << index) & OUTBOUND_SF2_PORTS_MASK))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }
        channel = sf2_emac_port_mapping[index].bbh_queue;
        break;
#endif

#ifdef CONFIG_BCM_TCONT
    case rdpa_port_gpon:
    case rdpa_port_xgpon:
#if defined(TM_CORE_LOAD_BALANCING)
        channel = RDD_US_CHANNEL_OFFSET_TCONT;
#endif
        break;
#endif

#if defined(CONFIG_RNR_DSL_INTF_SUPPORT)
    case rdpa_port_dsl:
#if defined(TM_CORE_LOAD_BALANCING)
        channel = RDD_US_CHANNEL_OFFSET_DSL;
#endif
        break;
#endif

    default:
        BDMF_TRACE_ERR("[%s] Invalid type %d\n", __FUNCTION__, type);
    }
    return channel;
}

static inline int16_t channel_to_port_mapping_index_get(rdpa_port_type type, uint32_t channel)
{
    int16_t index = 0;

    switch (type)
    {
#if !defined(G9991_COMMON)
    case rdpa_port_emac:
    case rdpa_vport_sq:
        index = channel; /* (bbh_queue is not a unique number) */
#if !defined(BCM_PON_XRDP)
        {
            uint16_t i;

            for (i = 0 ; i <= BBH_ID_NUM_LAN; i++)
            {
                if (channel == emac_port_mapping[i].bbh_queue && PORT_MAPPING_ENTRY_IS_VALID(i))
                    index = i;
            }
        }
#endif
        break;
#endif

#if defined(SF2_SUPPORT)
    case rdpa_port_sf2_emac:
        {
            uint16_t i;

            for( i = 0 ; i < rdpa_emac__num_of; i++)
                if ((((1 << i) & OUTBOUND_SF2_PORTS_MASK)) && (channel == sf2_emac_port_mapping[i].bbh_queue))
                    index = i;
        }
        return index;
#endif

#ifdef G9991_COMMON
    case rdpa_port_emac:
    case rdpa_port_sid:
    case rdpa_port_ctrl_sid:
    case rdpa_port_sys_port:
        index = channel;
        break;
#endif

#ifdef CONFIG_BCM_TCONT
    case rdpa_port_gpon:
    case rdpa_port_xgpon:
    case rdpa_port_epon:
    case rdpa_port_xepon:
    case rdpa_port_epon_ae:
        index = 0;
        break;
#endif
#if defined(CONFIG_RNR_DSL_INTF_SUPPORT)
    case rdpa_port_dsl:
        return 0;
#endif

    default:
        BDMF_TRACE_ERR("[%s] Invalid type %d\n", __FUNCTION__, type);
    }
    return index;
}

#ifdef TM_C_CODE
static inline tm_identifier_e port_mapping_tm_identity_get(rdpa_port_type type, uint32_t index)
{
    switch (type)
    {
    case rdpa_port_emac:
    case rdpa_port_sys_port:
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            return TM_ERROR;
        }

/* TODO AE SUPPORT AE for 68375 */
#if (defined(BCM6888) || defined(BCM6837))
        /* special handle for AE to US core in 6888 and 6837 and 68375 */
        if (emac_port_mapping[index].bb_rx_id == BB_ID_RX_BBH_AE)
            return TM_PON_DSL_AE;
#endif

        return emac_port_mapping[index].tm_identity;

    case rdpa_port_sid:
    case rdpa_port_ctrl_sid:
        return TM_ETH_START; /* TODO - is it ok, not really supported this way?*/

#ifdef CONFIG_BCM_TCONT
    case rdpa_port_gpon:
    case rdpa_port_xgpon:
    case rdpa_port_epon:
    case rdpa_port_xepon:
    case rdpa_port_epon_ae:
        return TM_PON_DSL_AE; 
#endif

#if defined(CONFIG_RNR_DSL_INTF_SUPPORT)
    case rdpa_port_dsl:
        return TM_PON_DSL_AE; 
#endif

    default:
        BDMF_TRACE_ERR("[%s] Invalid type %d\n", __FUNCTION__, type);
        return TM_ERROR;
    }
}

static inline int8_t port_mapping_tm_identity_set(rdpa_port_type type, uint32_t index, tm_identifier_e value)
{
    switch (type)
    {
    case rdpa_port_emac:
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
            BDMF_TRACE_RET(BDMF_ERR_NOENT, "[%s] Invalid index %d\n", __FUNCTION__, index);

        bdmf_trace("repalace  emac_port_mapping[index].tm_identity index=%d identity=%d\n", index, value);
        emac_port_mapping[index].tm_identity = value;
        return  0;

    default:
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "[%s] Not supported type %d\n", __FUNCTION__, type);
    }
}
#endif

static inline uint8_t port_mapping_tm_index_get(rdpa_port_type type, uint32_t index)
{
    uint8_t tm_index = 0;
    switch (type)
    {
    case rdpa_port_emac:
    case rdpa_port_sys_port:
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }
        tm_index = emac_port_mapping[index].tm_index;
        break;

    case rdpa_port_sid:
    case rdpa_port_ctrl_sid:
        tm_index = rdpa_dir_ds;
        break;

#if defined(SF2_SUPPORT)
    case rdpa_port_sf2_emac:
        if (!((1 << index) & OUTBOUND_SF2_PORTS_MASK))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }

        tm_index = sf2_emac_port_mapping[index].tm_index;
        break;
#endif

#ifdef CONFIG_BCM_TCONT
    case rdpa_port_gpon:
    case rdpa_port_xgpon:
    case rdpa_port_epon:
    case rdpa_port_xepon:
    case rdpa_port_epon_ae:
        tm_index = pon_port_mapping[0].tm_index;
        break;
#endif

#if defined(CONFIG_RNR_DSL_INTF_SUPPORT)
    case rdpa_port_dsl:
        return dsl_port_mapping[0].tm_index;
#endif

    default:
        BDMF_TRACE_ERR("[%s] Invalid type %d\n", __FUNCTION__, type);
    }
    return tm_index;
}

static inline int8_t port_mapping_tm_index_set(rdpa_port_type type, uint32_t index, uint8_t tm_index)
{
    switch (type)
    {
    case rdpa_port_emac:
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
            BDMF_TRACE_RET(BDMF_ERR_NOENT, "[%s] Invalid index %d\n", __FUNCTION__, index);
        emac_port_mapping[index].tm_index = tm_index;
        break;

#if defined(SF2_SUPPORT)
    case rdpa_port_sf2_emac:
        if (!((1 << index) & OUTBOUND_SF2_PORTS_MASK))
            BDMF_TRACE_RET(BDMF_ERR_NOENT, "[%s] Invalid index %d\n", __FUNCTION__, index);
        sf2_emac_port_mapping[index].tm_index = tm_index;
        break;
#endif

    default:
        BDMF_TRACE_RET(BDMF_ERR_NOENT, "[%s] Invalid type %d\n", __FUNCTION__, type);
    }
    return 0;
}

static inline uint8_t port_mapping_tx_runner_core_get(rdpa_port_type type, uint32_t index)
{
    /* CRDP-TODO*/
    return 1;
}

static inline int8_t port_mapping_tx_runner_core_set(rdpa_port_type type, uint32_t index, uint8_t tx_runner_core)
{
    switch (type)
    {
    case rdpa_port_emac:
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
            BDMF_TRACE_RET(BDMF_ERR_NOENT, "[%s] Invalid index %d\n", __FUNCTION__, index);
        emac_port_mapping[index].tx_runner_core = tx_runner_core;
        break;
#if defined(SF2_SUPPORT)
    case rdpa_port_sf2_emac:
        if (!((1 << index) & OUTBOUND_SF2_PORTS_MASK))
            BDMF_TRACE_RET(BDMF_ERR_NOENT, "[%s] Invalid index %d\n", __FUNCTION__, index);
        sf2_emac_port_mapping[index].tx_runner_core = tx_runner_core;
        break;
#endif
#ifdef CONFIG_BCM_TCONT
    case rdpa_port_gpon:
    case rdpa_port_xgpon:
    case rdpa_port_epon:
    case rdpa_port_xepon:
    case rdpa_port_epon_ae:
        pon_port_mapping[index].tx_runner_core = tx_runner_core;
        break;
#endif
#if defined(CONFIG_RNR_DSL_INTF_SUPPORT)
    case rdpa_port_dsl:
        if (index > 0)
            BDMF_TRACE_RET(BDMF_ERR_NOENT, "[%s] Invalid index %d\n", __FUNCTION__, index);
        dsl_port_mapping[index].tx_runner_core = tx_runner_core;
        break;
#endif

    default:
        BDMF_TRACE_RET(BDMF_ERR_NOENT, "[%s] Invalid type %d\n", __FUNCTION__, type);
    }
    return 0;
}

static inline uint8_t port_mapping_bbh_queue_get(rdpa_port_type type, uint32_t index)
{
    uint8_t bbh_queue = 0;
    switch (type)
    {
#if !defined G9991_COMMON
    case rdpa_port_emac:
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }

        bbh_queue = emac_port_mapping[index].bbh_queue;
        break;
#else
    case rdpa_port_emac:
    case rdpa_port_sys_port:
    case rdpa_port_sid:
    case rdpa_port_ctrl_sid:
        bbh_queue = index;
        break;
#endif

#if defined(SF2_SUPPORT)
    case rdpa_port_sf2_emac:
        if (!((1 << index) & OUTBOUND_SF2_PORTS_MASK))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }
        bbh_queue = sf2_emac_port_mapping[index].bbh_queue;
        break;
#endif

#ifdef CONFIG_BCM_TCONT
    case rdpa_port_gpon:
    case rdpa_port_xgpon:
    case rdpa_port_epon:
    case rdpa_port_xepon:
    case rdpa_port_epon_ae:
        bbh_queue = pon_port_mapping[0].bbh_queue;
        break;
#endif

#if defined(CONFIG_RNR_DSL_INTF_SUPPORT)
    case rdpa_port_dsl:
        return dsl_port_mapping[0].bbh_queue;
#endif

    default:
        BDMF_TRACE_ERR("[%s] Invalid type %d\n", __FUNCTION__, type);
    }
    return bbh_queue;
}

static inline int port_mapping_bbh_queue_set(rdpa_port_type type, uint32_t index, uint8_t bbh_queue)
{
    switch (type)
    {
    case rdpa_port_emac:
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
            BDMF_TRACE_RET(BDMF_ERR_NOENT, "[%s] Invalid index %d\n", __FUNCTION__, index);
        emac_port_mapping[index].bbh_queue = bbh_queue;
        break;

#if defined(SF2_SUPPORT)
    case rdpa_port_sf2_emac:
        if (!((1 << index) & OUTBOUND_SF2_PORTS_MASK))
            BDMF_TRACE_RET(BDMF_ERR_NOENT, "[%s] Invalid index %d\n", __FUNCTION__, index);
        sf2_emac_port_mapping[index].bbh_queue = bbh_queue;
        break;
#endif

    default:
        BDMF_TRACE_RET(BDMF_ERR_NOENT, "[%s] Invalid type %d\n", __FUNCTION__, type);
    }
    return 0;
}

static inline uint8_t port_mapping_get_bbh_queue_by_channel_id(rdpa_port_type type, int16_t channel_id)
{
    uint8_t index = channel_to_port_mapping_index_get(type, channel_id);
    return port_mapping_bbh_queue_get(type, index);
}

static inline uint8_t port_mapping_bb_rx_id_get(rdpa_port_type type, uint32_t index)
{
    uint8_t bb_rx_id = 0;

    switch (type)
    {
    case rdpa_port_emac:
    case rdpa_port_sys_port:
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }
        bb_rx_id = emac_port_mapping[index].bb_rx_id;
        break;

    case rdpa_port_sid:
    case rdpa_port_ctrl_sid:
        bb_rx_id = index;
        break;

#if defined(SF2_SUPPORT)
    case rdpa_port_sf2_emac:
        if (!((1 << index) & OUTBOUND_SF2_PORTS_MASK))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }
        bb_rx_id = sf2_emac_port_mapping[index].bb_rx_id;
        break;
#endif

#ifdef CONFIG_BCM_TCONT
    case rdpa_port_gpon:
    case rdpa_port_xgpon:
    case rdpa_port_epon:
    case rdpa_port_xepon:
    case rdpa_port_epon_ae:
        bb_rx_id = pon_port_mapping[0].bb_rx_id;
        break;
#endif

#if defined(CONFIG_RNR_DSL_INTF_SUPPORT)
    case rdpa_port_dsl:
        return dsl_port_mapping[0].bb_rx_id;
#endif

    case rdpa_port_cpu:
    case rdpa_port_wlan:
    case rdpa_port_gdx:
    case rdpa_port_bond:
        /* No bb_rx_id in those cases - return 0 */
        break;
    default:
        BDMF_TRACE_ERR("[%s] Invalid type %d\n", __FUNCTION__, type);
    }
    return bb_rx_id;
}

static inline uint8_t port_mapping_bbh_tx_id_get(rdpa_port_type type, uint32_t index)
{
    uint8_t bbh_tx_id = 0;
    switch (type)
    {
    case rdpa_port_emac:
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }
        bbh_tx_id = emac_port_mapping[index].bbh_tx_id;
        break;

#if defined(SF2_SUPPORT)
    case rdpa_port_sf2_emac:
        if (!((1 << index) & OUTBOUND_SF2_PORTS_MASK))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }
        bbh_tx_id = sf2_emac_port_mapping[index].bbh_tx_id;
        break;
#endif

#ifdef CONFIG_BCM_TCONT
    case rdpa_port_gpon:
    case rdpa_port_xgpon:
    case rdpa_port_epon:
    case rdpa_port_xepon:
    case rdpa_port_epon_ae:
        bbh_tx_id = pon_port_mapping[0].bbh_tx_id;
        break;
#endif

#if defined(CONFIG_RNR_DSL_INTF_SUPPORT)
    case rdpa_port_dsl:
        return dsl_port_mapping[0].bbh_tx_id;
#endif

    default:
        BDMF_TRACE_ERR("[%s] Invalid type %d\n", __FUNCTION__, type);
    }
    return bbh_tx_id;
}

static inline uint8_t port_mapping_bb_tx_id_get(rdpa_port_type type, uint32_t index)
{
    uint8_t bb_tx_id = 0;
    switch (type)
    {
    case rdpa_port_emac:
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }

        bb_tx_id = emac_port_mapping[index].bb_tx_id;
        break;

#if defined(SF2_SUPPORT)
    case rdpa_port_sf2_emac:
        if (!((1 << index) & OUTBOUND_SF2_PORTS_MASK))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }
        bb_tx_id = sf2_emac_port_mapping[index].bb_tx_id;
        break;
#endif

#ifdef CONFIG_BCM_TCONT
    case rdpa_port_gpon:
    case rdpa_port_xgpon:
    case rdpa_port_epon:
    case rdpa_port_xepon:
    case rdpa_port_epon_ae:
        bb_tx_id = pon_port_mapping[0].bb_tx_id;
        break;
#endif

#if defined(CONFIG_RNR_DSL_INTF_SUPPORT)
    case rdpa_port_dsl:
        return dsl_port_mapping[0].bb_tx_id;
#endif

#ifdef G9991_COMMON
    case rdpa_port_sys_port:
    case rdpa_port_sid:
    case rdpa_port_ctrl_sid:
        bb_tx_id = emac_port_mapping[vport_to_emac[index]].bb_tx_id;
        break;
#endif

    default:
        BDMF_TRACE_ERR("[%s] Invalid type %d\n", __FUNCTION__, type);
    }
    return bb_tx_id;
}

static inline uint8_t port_mapping_viq_get(rdpa_port_type type, uint32_t index)
{
    uint8_t viq = 0;
    switch (type)
    {
    case rdpa_port_emac:
#ifdef G9991_FC
    case rdpa_port_sys_port:
#endif
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }
        viq = emac_port_mapping[index].viq;
        break;

#if defined(SF2_SUPPORT)
    case rdpa_port_sf2_emac:
        if (!((1 << index) & OUTBOUND_SF2_PORTS_MASK))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }
        viq = sf2_emac_port_mapping[index].viq;
        break;
#endif

#ifdef CONFIG_BCM_TCONT
    case rdpa_port_gpon:
    case rdpa_port_xgpon:
    case rdpa_port_epon:
    case rdpa_port_xepon:
    case rdpa_port_epon_ae:
        viq = pon_port_mapping[0].viq;
        break;
#endif

#if defined(CONFIG_RNR_DSL_INTF_SUPPORT)
    case rdpa_port_dsl:
        return dsl_port_mapping[0].viq;
#endif

    default:
        BDMF_TRACE_ERR("[%s] Invalid type %d\n", __FUNCTION__, type);
    }
    return viq;
}

static inline uint8_t port_mapping_viq_set(rdpa_port_type type, uint32_t index, uint8_t viq)
{
    switch (type)
    {
    case rdpa_port_emac:
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
            BDMF_TRACE_RET(BDMF_ERR_NOENT, "[%s] Invalid index %d\n", __FUNCTION__, index);
        emac_port_mapping[index].viq = viq;
        break;
#if defined(SF2_SUPPORT)
    case rdpa_port_sf2_emac:
        if (!((1 << index) & OUTBOUND_SF2_PORTS_MASK))
            BDMF_TRACE_RET(BDMF_ERR_NOENT, "[%s] Invalid index %d\n", __FUNCTION__, index);
        sf2_emac_port_mapping[index].viq = viq;
        break;
#endif
#ifdef CONFIG_BCM_TCONT
    case rdpa_port_gpon:
    case rdpa_port_xgpon:
    case rdpa_port_epon:
    case rdpa_port_xepon:
    case rdpa_port_epon_ae:
        pon_port_mapping[0].viq = viq;
        break;
#endif
#if defined(CONFIG_RNR_DSL_INTF_SUPPORT)
    case rdpa_port_dsl:
        dsl_port_mapping[0].viq = viq;
        break;
#endif
    default:
        BDMF_TRACE_RET(BDMF_ERR_NOENT, "[%s] Invalid type %d\n", __FUNCTION__, type);
    }
    return 0;
}

static inline uint8_t port_mapping_get_tx_runner_core_by_channel_id(rdpa_port_type type, int16_t channel_id)
{
    uint32_t index = channel_to_port_mapping_index_get(type, channel_id);
    return port_mapping_tx_runner_core_get(type, index);
}

static inline uint8_t port_mapping_tm_ds_index_get(rdpa_port_type type, uint32_t index)
{
#if defined (MULTIPLE_BBH_TX_LAN) || defined(TM_CORE_LOAD_BALANCING)
    uint8_t tm_index = 0;
    switch (type)
    {
#if !defined(G9991_COMMON)
    case rdpa_port_emac:
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }
        tm_index = emac_port_mapping[index].tm_ds_index;
        break;
#else
    case rdpa_port_emac:
    case rdpa_port_sid:
    case rdpa_port_ctrl_sid:
    case rdpa_port_sys_port:
        tm_index = emac_port_mapping[vport_to_emac[index]].tm_ds_index;
        break;
#endif


#if defined(SF2_SUPPORT)
    case rdpa_port_sf2_emac:
        if (!((1 << index) & OUTBOUND_SF2_PORTS_MASK))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }
        tm_index = sf2_emac_port_mapping[index].tm_ds_index;
        break;
#endif

#ifdef CONFIG_BCM_TCONT
    case rdpa_port_gpon:
    case rdpa_port_xgpon:
    case rdpa_port_epon:
    case rdpa_port_xepon:
    case rdpa_port_epon_ae:
        tm_index = pon_port_mapping[0].tm_ds_index;
        break;
#endif
#if defined(CONFIG_RNR_DSL_INTF_SUPPORT)
    case rdpa_port_dsl:
        return dsl_port_mapping[0].tm_ds_index;
#endif
    default:
        BDMF_TRACE_ERR("[%s] Invalid type %d\n", __FUNCTION__, type);
    }
    return tm_index;
#else
    return 0;
#endif
}

static inline uint8_t port_mapping_get_tm_index_by_channel_id(rdpa_port_type type, int16_t channel_id)
{
#if defined(MULTIPLE_BBH_TX_LAN)
    uint8_t index = channel_to_port_mapping_index_get(type, channel_id);
    return port_mapping_tm_ds_index_get(type, index);
#else
    return 0;
#endif
}

static inline uint8_t port_mapping_core_index_get(rdpa_port_type type, uint32_t channel_id)
{
    uint8_t core_index = 0;
#if defined(MULTIPLE_BBH_TX_LAN)

    uint8_t index = channel_to_port_mapping_index_get(type, channel_id);

    switch (type)
    {
#if !defined(G9991_COMMON)
    case rdpa_port_emac:
        if (index > BBH_ID_NUM_LAN || !PORT_MAPPING_ENTRY_IS_VALID(index))
        {
            BDMF_TRACE_ERR("[%s] Invalid index %d\n", __FUNCTION__, index);
            break;
        }
        core_index = emac_port_mapping[index].core_index;
        break;
#else
    case rdpa_port_emac:
    case rdpa_port_sid:
    case rdpa_port_ctrl_sid:
    case rdpa_port_sys_port:
        core_index = emac_port_mapping[vport_to_emac[index]].core_index;
        break;
#endif

#ifdef CONFIG_BCM_TCONT
    case rdpa_port_gpon:
    case rdpa_port_xgpon:
    case rdpa_port_epon:
    case rdpa_port_xepon:
    case rdpa_port_epon_ae:
        core_index = pon_port_mapping[0].core_index;
        break;
#endif
    default:
        BDMF_TRACE_ERR("[%s] Invalid type %d\n", __FUNCTION__, type);
    }
#endif    
    return core_index;
}

#endif /*RDP_COMMON_H_INCLUDED*/
