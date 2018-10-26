 /*
<:copyright-BRCM:2015:proprietary:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>

*/
/***********************************************************************/
/*                                                                     */
/*   MODULE:  merlin_serdes.h                                          */
/*   DATE:    30/11/2015                                               */
/*   PURPOSE: definitions for APIs of Merlin Serdes                    */
/*                                                                     */
/***********************************************************************/
#ifndef __SERDES_MERLIN_H
#define __SERDES_MERLIN_H

/*#define MERLIN_DEBUG*/
/*#define MELIN_PC_DEBUG*/

#include "serdes_access.h"

#ifndef FALSE
#define FALSE 0x0
#endif

#ifndef TRUE
#define TRUE  0x1
#endif

#define MERLIN_LANES_PER_CORE   2
#define MERLIN_MAX_CORE_NUMBER  2
#define MERLIN_MAX_LANE_NUMBER  MERLIN_LANES_PER_CORE*MERLIN_MAX_CORE_NUMBER

/*RMIC/PMI address*/
#define MMD_TYPE_OFFSET      27
#define LANE_ADDRESS_OFFSET  16

typedef enum{
    MERLIN_INTERFACE_XFI    = 0,
    MERLIN_INTERFACE_HSGMII = 1,
    MERLIN_INTERFACE_SGMII  = 2,
    MERLIN_INTERFACE_UNUSED = 3
}merlin_lane_type_t;

typedef struct{
    merlin_lane_type_t  lane_type[MERLIN_MAX_LANE_NUMBER];
 }merlin_init_config_t;

typedef enum phymod_an_mode_type_e {
    MERLIN_AN_MODE_CL37,
    MERLIN_AN_MODE_CL37_USER,
    MERLIN_AN_MODE_CL73,
    MERLIN_AN_MODE_CL73_USER,
    MERLIN_AN_MODE_SGMII_MASTER,
    MERLIN_AN_MODE_SGMII_SLAVE
} merlin_an_mode_t;

typedef struct{
    uint32_t pll_ready[MERLIN_MAX_CORE_NUMBER];
    uint32_t pll_val[MERLIN_MAX_CORE_NUMBER];
    merlin_lane_type_t lane_type[MERLIN_MAX_LANE_NUMBER];
    merlin_an_mode_t an_mode[MERLIN_MAX_LANE_NUMBER];
 }merlin_config_status_t;

typedef struct merlin_access_s{
    uint32_t index;
    uint32_t type;       /* Address Type */
    uint32_t addr;       /* Address used depends on type*/
} merlin_access_t;

typedef enum{
    MERLIN_CL22    = 0,
    MERLIN_PMD_PMA = 1,
    MERLIN_PCS     = 3,
    MERLIN_AN      = 7
}merlin_dev_type;

typedef enum{
    MERLIN_LOOPBACK_NONE       = 0,
    MERLIN_PCS_LOCAL_LOOPBACK  = 1,
    MERLIN_PCS_REMOTE_LOOPBACK = 2,
    MERLIN_PMD_LOCAL_LOOPBACK  = 3,
    MERLIN_PMD_REMOTE_LOOPBACK = 4
}merlin_loopback_mode;

typedef struct {
    uint32_t an_cap;
    merlin_an_mode_t mode;
}merlin_autoneg_ability_t;

typedef enum{
    MERLIN_CAP_10M       = 1,
    MERLIN_CAP_100M      = 1<<1,
    MERLIN_CAP_1000M     = 1<<2,
    MERLIN_CAP_2P5G      = 1<<3,
    MERLIN_CAP_10G       = 1<<4,
    MERLIN_CAP_FULL      = 1<<5,
    MERLIN_CAP_HALF      = 1<<6,
    MERLIN_CAP_PAUSE     = 1<<7,
    MERLIN_CAP_FEC       = 1<<8
}merlin_capability_t;

typedef enum{
    MERLIN_PRBS_MODE_7  = 0,
    MERLIN_PRBS_MODE_9,
    MERLIN_PRBS_MODE_11,
    MERLIN_PRBS_MODE_15,
    MERLIN_PRBS_MODE_23,
    MERLIN_PRBS_MODE_31,
    MERLIN_PRBS_MODE_58,
    MERLIN_PRBS_MODE_INVALID
}merlin_prbs_mode_t;

typedef enum{
    MERLIN_CMD_PMD_RESET = 0,
    MERLIN_CMD_PMD_ABILITY_GET,
    MERLIN_CMD_PMD_TRANSMIT_CONTROL,
    MERLIN_CMD_PMD_RECEIVE_CONTROL,
    MERLIN_CMD_PCS_ABILITY_GET,
    MERLIN_CMD_STATS_GET,
    MERLIN_CMD_STATUS_GET,
    MERLIN_CMD_LP_STATUS_GET,
    MERLIN_CMD_FORCE_LINK_DOWN,
    MERLIN_CMD_FORCE_LINK_UP,
    MERLIN_CMD_DATAPATH_RESET, 
    MERLIN_CMD_PRBS_STATS_GET,
    MERLIN_CMD_PRBS_GENERATOR,
    MERLIN_CMD_PRBS_MONITOR,
    MERLIN_CMD_UC_RAM_WRITE,
    MERLIN_CMD_UC_RAM_READ,
    MERLIN_CMD_MAP_LANE_INDEX,
    MERLIN_CMD_ID,
    MERLIN_CMD_REG_WRITE,
    MERLIN_CMD_REG_READ,
}merlin_command_t;

typedef struct{
    uint16_t kcode66ErrCount;
    uint16_t sync66ErrCount;
    uint16_t cl49ieee_errored_blocks;
    uint16_t BER_count_per_ln;
    uint16_t cl49_valid_sh_cnt;
    uint16_t cl49_invalid_sh_cnt;
    uint32_t fec_corrected;
    uint32_t fec_uncorrected;
}merlin_stats_t;

typedef struct{
    uint32_t prbs_tx_pkt;
    uint32_t prbs_rx_pkt;
    uint32_t prbs_error;
    uint16_t crcerrcnt;
}merlin_prbs_stats_t;

typedef struct{
    uint16_t rev_letter;
    uint16_t rev_number;
    uint16_t bonding;
    uint16_t tech_proc;
    uint16_t model_number;
}merlin_serdes_id_t;

typedef struct{
    uint16_t tx_LOCAL_FAULT;
    uint16_t tx_REMOTE_FAULT;
    uint16_t PMD_LOCK;
    uint16_t signal_ok;
    uint16_t rx_LOCAL_FAULT;
    uint16_t rx_REMOTE_FAULT;
    uint16_t rx_LINK_STATUS;
    uint16_t rx_SYNC_STATUS;
    uint16_t pll_lock;
    uint16_t cl36_syncacq_state_coded_per_ln;
    uint16_t cl36_syncacq_his_state_per_ln;
}merlin_status_t;

typedef struct{
    uint16_t kcode66ErrCount;
    uint16_t sync66ErrCount;
    uint16_t cl49ieee_errored_blocks;
    uint16_t BER_count_per_ln;
}merlin_rx_stats_t;

typedef struct{
    uint32_t  port;
    uint16_t  lane_index;
}merlin_lport_map_t;

typedef struct{
    uint16_t start;
    uint16_t len;
    uint16_t val;
}merlin_uc_ram_data_t;

typedef struct {
    uint16_t device_type;
    uint16_t reg_addr;
    uint16_t reg_value;
}merlin_reg_access_t;

typedef union{
    merlin_status_t status;
    merlin_stats_t  stats;
    merlin_prbs_stats_t prbs_stats;
    merlin_serdes_id_t serdes_id;
    merlin_lport_map_t map;
    merlin_uc_ram_data_t ram_data;
    uint16_t val;
    merlin_reg_access_t reg_data;
    uint16_t prbs_type;
}merlin_control_t;

typedef enum {    
    SFI_COPPER_OSx1,
    SFI_OPTICS_OSx1,
    SFI_OPTICS_DFE_OSx1,
    XFI_OSx1,
    XFI_DFE_OSx1,
    nPPI_OPTICS_OSx1,
    nPPI_OPTICS_DFE_OSx1,   
    CUSTOM_ELEC_SPEC
}merlin_elec_spec_enum_t;

#define MERLIN_READ_REG(dev_type, lane_index, reg_addr, mask, shift, value)       \
    do {                                                                         \
        uint32_t addr=0;                                             \
        addr = ((dev_type)<<MMD_TYPE_OFFSET)|(((uint32_t)(lane_index%MERLIN_LANES_PER_CORE))<<LANE_ADDRESS_OFFSET)|(reg_addr); \
        read_serdes_reg((lane_index)/MERLIN_LANES_PER_CORE, addr, (mask), (value));\
        *value >>= shift;\
      } while (0);

#define MERLIN_WRITE_REG(dev_type, lane_index, reg_addr, mask, shift, value)        \
    do {    \
        uint32_t addr=0;  \
        addr = ((dev_type)<<MMD_TYPE_OFFSET)|(((uint32_t)(lane_index%MERLIN_LANES_PER_CORE))<<LANE_ADDRESS_OFFSET)|(reg_addr);    \
        write_serdes_reg((lane_index)/MERLIN_LANES_PER_CORE, addr, (uint16_t)(mask), ((value) << (shift)));    \
     } while (0);

int merlin_init(E_MERLIN_ID merlin_id, E_MERLIN_VCO pll);

int merlin_link_config_set(E_MERLIN_LANE lane_index, lport_serdes_cfg_s *serdes_cfg);

int merlin_link_config_get(E_MERLIN_LANE lane_index, lport_serdes_cfg_s *serdes_cfg);

int merlin_datapath_reset(E_MERLIN_ID merlin_id);

int merlin_read_register(uint16_t dev_type, uint16_t lane_index,    uint16_t reg_addr, 
                                                    uint16_t mask, uint8_t shift, uint16_t* value);


int merlin_write_register(uint16_t dev_type, uint16_t lane_index,   uint16_t reg_addr, 
                                                    uint16_t mask, uint8_t shift, uint16_t value);

int merlin_link_force_speed(uint16_t lane_index, LPORT_PORT_RATE speed);

int merlin_link_enable(uint16_t lane_index);

int merlin_link_disable(uint16_t lane_index);

int merlin_link_status_get(uint16_t lane_index, uint8_t *link_up);

int merlin_link_rate_and_duplex_get(uint16_t lane_index, uint16_t *speed, uint16_t *duplex);

int merlin_advertise_cap_set(uint16_t lane_index, merlin_autoneg_ability_t capability);

int merlin_advertise_cap_get(uint16_t lane_index, merlin_autoneg_ability_t *capability);

int merlin_auto_enable(uint16_t lane_index);

int merlin_auto_disable(uint16_t lane_index);

int merlin_loopback_set(uint16_t lane_index, merlin_loopback_mode mode, uint8_t enable);

int merlin_control(uint16_t lane_index, merlin_command_t cmd, merlin_control_t * data);

int merlin_prbs_enable_set(uint16_t lane_index, merlin_prbs_mode_t mode, uint8_t enable);

int merlin_prbs_check(uint16_t lane_index, uint8_t *locked, merlin_prbs_stats_t *stats);

#endif


