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

#ifndef _CFE_
#include "bdmf_shell.h"
#else
#define BUG_ON(cond) do {} while(0)
#endif
#ifndef XRDP_EMULATION
#include "rdd_crc.h"
#else
#include "access_macros.h"
#endif
#include "rdp_platform.h"

/* default values for PARSER */
#define PARSER_TCP_CTL_FLAGS                0x07
#define PARSER_EXCP_IP_HDR_LEN_ERR      (1 << 0)
#define PARSER_EXCP_CHKSUM_ERR          (1 << 1)
#define PARSER_EXCP_ETH_MULCST          (1 << 2)
#define PARSER_EXCP_IP_MULCST           (1 << 3)
#define PARSER_EXCP_DISABLE_IP_LEN_ERR  (1 << 4)
#define PARSER_EXCP_DISABLE_IP_VER_TEST (1 << 5)
#define PARSER_EXCP_PPP_CODE_1_PROTOCOL (1 << 7) /* defines the protocol of ppp_code_1 (1 - IPv6, 0 - IPv4) in register PPP_IP_Protocol_Code*/
#define PARSER_EXCP_IP_L2_MULCST        (1 << 10)
#define PARSER_EXCP_L4_INV_5_TUPPLE     (1 << 11)
#define PARSER_EXCP_UDP_1588            (1 << 12)
#define PARSER_EXCP_DHCP                (1 << 13)
#define PARSER_EXCP_MAC_SPOOF           (1 << 3) /* MAC_SPOF bit is located in bit [3] on the eng_conf register (ENG)*/
#define PARSER_EXCP_STATUS_BITS     (PARSER_EXCP_IP_HDR_LEN_ERR | PARSER_EXCP_CHKSUM_ERR | PARSER_EXCP_ETH_MULCST | PARSER_EXCP_IP_MULCST \
                                    | PARSER_EXCP_PPP_CODE_1_PROTOCOL | PARSER_EXCP_IP_L2_MULCST | PARSER_EXCP_L4_INV_5_TUPPLE \
                                    | PARSER_EXCP_UDP_1588 | PARSER_EXCP_DHCP)
#define PARSER_AH_DETECTION                 0x18000 | PARSER_EXCP_MAC_SPOOF /* eng_conf register default configuration*/
#define PARSER_PPP_PROTOCOL_CODE_0_IPV4     0x21
#define PARSER_PPP_PROTOCOL_CODE_1_IPV6     0x57
#define PARSER_IP_PROTOCOL_IPIP             4
#define PARSER_PROFILE_US           0x02 /* Profile 0 for DS, profile 1 for US */

typedef enum
{
    DMA_BUFSIZE_128  = 0,
    DMA_BUFSIZE_256  = 1,
    DMA_BUFSIZE_512  = 2,
    DMA_BUFSIZE_1024 = 3,
    DMA_BUFSIZE_2048 = 4,
} drv_rnr_dma_bufsize_t;

typedef enum  ddr_buf_size_e
{
    BUF_256 = 0,
    BUF_512,
    BUF_1K,
    BUF_2K,
    BUF_4K
} ddr_buf_size_e;

void lookup_bbh_tx_bufsz_by_fpm_bufsz(uint32_t *fpm_bufsize, uint8_t *bbh_tx_bufsize);
void lookup_dma_bufsz_by_fpm_bufsz(uint32_t *fpm_bufsz, uint8_t* dma_bufsz);
void rdpa_system_group_allocation_defaults_set(uint8_t us, uint8_t ds, uint8_t wlan);

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
    hash_max_1_5k_entries_per_engine,
} tbl_size_e;

typedef struct
{
    uint16_t total_fpm_tokens; /**< define the max number of fpm tokens assigned to UG */
    uint16_t high_prio_rsv_percent; /**< fpm tokens reserved to high priority traffic */
    uint16_t excl_prio_rsv_percent; /**< fpm tokens reserved to exclusive priority traffic */
} fpm_token_allocation_t;

typedef struct
{
    uint32_t mtu_size;
    uint32_t headroom_size;
    bcm_tag_t bcmsw_tag;
    bdmf_boolean is_gateway;
    bdmf_boolean vlan_stats_enable;
    ddr_addr rdp_phy_ddr_rnr_tables_base; /* runner tables DDR base address */
    void *rdp_ddr_rnr_tables_base_virt;   /* runner tables DDR0 virtual base address */
    bdmf_phys_addr_t rdp_ddr_rnr_tables_base_phys; /* runner tables DDR0 physical base address */
    uint32_t rnr_tables_buf_size;
    void *rdp_ddr_pkt_base_virt;          /* fpm pool DDR0 virtual base address for unicast packets */
    bdmf_phys_addr_t rdp_ddr_pkt_base_phys;  /* fpm pool DDR0 physical base address for unicast packets */
    uint32_t enabled_port_map;
    uint32_t runner_freq; /* rdp block clock */
    uint32_t fpm_buf_size;
    int xfi_port;
    uint32_t bbh_id_gbe_wan; /* bbh_id for gbe port */
    uint16_t number_of_ds_queues; /**< define the number of queue for DS queues */
    uint16_t number_of_us_queues; /**< define the number of queue for US queues */
    uint16_t number_of_service_queues; /**< define the number of queue for SERVICE queues */
    fpm_token_allocation_t us_fpm_tokens_allocation; /**< define the max number of fpm tokens assigned to US */
    fpm_token_allocation_t ds_fpm_tokens_allocation; /**< define the max number of fpm tokens assigned to DS */
    fpm_token_allocation_t wlan_fpm_tokens_allocation; /**< define the max number of fpm tokens assigned to WLAN */
    bdmf_boolean fpm_token_allocation_user_mode; /**< use user mode UG*/
#ifdef G9991
    rdpa_emac system_port;
    uint32_t g9991_port_vec;
    uint32_t g9991_bbh_vec;
#endif
    bdmf_boolean dpu_split_scheduling_mode;
    tbl_size_e iptv_table_size;
    tbl_size_e arl_table_size;
    uint32_t fw_clang_dis;
} dpi_params_t;

typedef struct bbh_to_dma_x
{
    bbh_id_e bbh_id;
    dma_id_e dma_id;
} bbh_to_dma_x_t;

/* QM queue index */
typedef uint16_t  rdp_qm_queue_idx_t;

/* FPM pool id */
typedef enum
{
    FPM_POOL_ID_0 = 0,
    FPM_POOL_ID_1 = 1,
    FPM_POOL_ID_2 = 2,
    FPM_POOL_ID_3 = 3,
} fpm_pool_id_e;

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

#ifndef _CFE_
extern struct bdmfmon_enum_val bbh_id_enum_table[];
extern struct bdmfmon_enum_val bbh_id_tx_enum_table[];
extern struct bdmfmon_enum_val dma_id_enum_table[];
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
#endif

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

#if defined(_CFE_)
#define RDPA_MAX_VLANS 128
#else
#if RDPA_MAX_VLANS == 256
#define HASH_TABLE_BRIDGE_AND_VLAN_LKP_SIZE_PER_ENGINE  hash_max_64_entries_per_engine
#elif (RDPA_MAX_VLANS == 128 || RDPA_MAX_VLANS == 32)
#define HASH_TABLE_BRIDGE_AND_VLAN_LKP_SIZE_PER_ENGINE  hash_max_32_entries_per_engine
#else
#error wrong configuration for num of vlans
#endif 
#endif /*CFE*/

/* NAT Cache default mask, include vport */
#define NATC_KEY0_DEF_MASK  0x80ffffff


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
#endif
