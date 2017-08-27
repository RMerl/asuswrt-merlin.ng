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
#define PARSER_EXCP_CHKSUM_ERR      (1 << 1)
#define PARSER_EXCP_IP_VER_ERR      (1 << 2)
#define PARSER_EXCP_IP_FRAG         (1 << 3)
#define PARSER_EXCP_IP_LEN_ERR      (1 << 10)
#define PARSER_EXCP_L4_INV_5_TUPPLE (1 << 11)
#define PARSER_EXCP_ERR             (1 << 12)
#define PARSER_EXCP_DHCP            (1 << 13)
#define PARSER_EXCP_STATUS_BITS     (PARSER_EXCP_CHKSUM_ERR | PARSER_EXCP_IP_VER_ERR | PARSER_EXCP_IP_FRAG \
    | PARSER_EXCP_IP_LEN_ERR | PARSER_EXCP_L4_INV_5_TUPPLE | PARSER_EXCP_ERR | PARSER_EXCP_DHCP)
#define PARSER_AH_DETECTION                 0x18000
#define PARSER_PPP_PROTOCOL_CODE_0_IPV4     0x21
#define PARSER_PPP_PROTOCOL_CODE_1_IPV6     0x57
#define PARSER_PROFILE_US           0x02 /* Profile 0 for DS, profile 1 for US */

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

typedef struct
{
    uint32_t mtu_size;
    uint32_t headroom_size;
    bcm_tag_t bcmsw_tag;
    ddr_addr rdp_phy_ddr_rnr_tables_base; /* runner tables DDR base address */
    void *rdp_ddr_rnr_tables_base_virt;   /* runner tables DDR0 virtual base address */
    void *rdp_ddr_pkt_base_virt;          /* fpm pool DDR0 virtual base address for unicast packets */
    uint32_t enabled_port_map;
    uint32_t runner_freq; /* rdp block clock */
    uint32_t fpm_buf_size;
    int xfi_port;
    uint32_t bbh_id_gbe_wan; /* bbh_id for gbe port */
#ifdef G9991
    rdpa_emac system_port;
    uint32_t g9991_port_vec;
#endif
} dpi_params_t;

typedef enum dma_id_e
{
    DMA0_ID = 0,
    DMA1_ID,
    SDMA0_ID,
    SDMA1_ID,
    DMA_NUM,
    DMA_ID_FIRST = DMA0_ID,
} dma_id_e;

#define IS_SDMA(dma_id)  (dma_id > DMA1_ID)
#define IS_DMA(dma_id)   (dma_id <= DMA1_ID)

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
    FPM_CPU_UG = 2,
    FPM_WLAN_UG = 3,
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
extern struct bdmfmon_enum_val dma_id_enum_table[];
extern struct bdmfmon_enum_val rnr_id_enum_table[];
extern struct bdmfmon_enum_val quad_idx_enum_table[];
extern struct bdmfmon_enum_val tbl_idx_enum_table[];
extern struct bdmfmon_enum_val eng_idx_enum_table[];
extern struct bdmfmon_enum_val ubus_mstr_id_enum_table[];
extern struct bdmfmon_enum_val channel_id_enum_table[];
#ifdef BCM68360
extern struct bdmfmon_enum_val umac_misc_id_enum_table[];
#define umac_id_enum_table umac_misc_id_enum_table
#define umac_mib_id_enum_table umac_misc_id_enum_table
extern struct bdmfmon_enum_val bacif_id_enum_table[];
#endif
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
