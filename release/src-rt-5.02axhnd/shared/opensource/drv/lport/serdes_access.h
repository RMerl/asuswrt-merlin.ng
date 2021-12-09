/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2015:DUAL/GPL:standard

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
/*

 * serdes_access.h
 *
 *  Created on: August 2015
 *      Author: yonatani
 */

#ifndef SHARED_OPENSOURCE_DRV_LPORT_SERDES_ACCESS_H_
#define SHARED_OPENSOURCE_DRV_LPORT_SERDES_ACCESS_H_

#ifdef _CFE_
#include "lib_types.h"
#else
#include <linux/types.h>
#endif
#include "lport_defs.h"


typedef enum
{
    MERLIN_ID_0,
    MERLIN_ID_1
} E_MERLIN_ID;

typedef enum
{
    MERLIN_LANE_0,
    MERLIN_LANE_1,
    MERLIN_LANE_2,
    MERLIN_LANE_3
} E_MERLIN_LANE;

typedef enum
{
    MERLIN_VCO_UNKNOWN,
    MERLIN_VCO_9375_MHZ,
    MERLIN_VCO_103125_MHZ
} E_MERLIN_VCO;

typedef enum{
    
    SERDES_PRBS_PATTERN_TYPE_PRBS7,
    SERDES_PRBS_PATTERN_TYPE_PRBS9,
    SERDES_PRBS_PATTERN_TYPE_PRBS11,
    SERDES_PRBS_PATTERN_TYPE_PRBS15,
    SERDES_PRBS_PATTERN_TYPE_PRBS23,
    SERDE_SPRBS_PATTERN_TYPE_PRBS31,
    SERDE_SPRBS_PATTERN_TYPE_PRBS58,
    SERDES_PRBS_PATTERN_TYPE_8180,
    SERDES_PRBS_PATTERN_TYPE_LAST,
} E_SERDES_PRBS_PATTERN_TYPE;

/* TODO move to separate include*/
typedef enum{
    MERLIN_CMD_STATS_GET,
    MERLIN_CMD_STATUS_GET,
    MERLIN_CMD_FORCE_LINK_DOWN,
    MERLIN_CMD_FORCE_LINK_UP,
    MERLIN_CMD_PRBS_STATS_GET,
    MERLIN_CMD_PRBS_GENRATE,
    MERLIN_CMD_PRBS_MONITOR,
    MERLIN_CMD_ID,
    MERLIN_CMD_REG_WRITE,
    MERLIN_CMD_REG_READ,
    MERLIN_CMD_LOOPBACK_SET,
    MERLIN_CMD_TXCFG_SET,
    MERLIN_CMD_TXCFG_GET
} merlin_command_t;

typedef enum{
    MERLIN_LOOPBACK_NONE = 0,
    MERLIN_PCS_LOCAL_LOOPBACK,
    MERLIN_PCS_REMOTE_LOOPBACK,
    MERLIN_PMD_LOCAL_LOOPBACK,
    MERLIN_PMD_REMOTE_LOOPBACK 
} merlin_loopback_mode;

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
} merlin_status_t;


typedef struct{
    uint16_t kcode66ErrCount;
    uint16_t sync66ErrCount;
    uint16_t cl49ieee_errored_blocks;
    uint16_t BER_count_per_ln;
    uint16_t cl49_valid_sh_cnt;
    uint16_t cl49_invalid_sh_cnt;
} merlin_stats_t;

typedef struct{
    uint32_t prbs_error;
    uint16_t prbs_lock_lost;
    uint16_t prbs_lock_status;
} merlin_prbs_stats_t;


typedef struct{
    uint16_t rev_letter;
    uint16_t rev_number;
    uint16_t bonding;
    uint16_t tech_proc;
    uint16_t model_number;
} merlin_serdes_id_t;

typedef struct {
    uint16_t device_type;
    uint16_t reg_addr;
    uint16_t reg_value;
} merlin_reg_access_t;

typedef struct {
    merlin_loopback_mode mode;
    uint8_t enable;
} merlin_loopback_set_t;

typedef struct {
    uint16_t pre;
    uint16_t main;
    uint16_t post1;
    uint16_t post2;
    uint16_t hpf;
} merlin_tx_cfg_t;

typedef union{
    merlin_status_t status;
    merlin_stats_t  stats;
    merlin_prbs_stats_t prbs_stats;
    merlin_serdes_id_t serdes_id;
    uint16_t val;
    uint16_t prbs_type;
    merlin_reg_access_t reg_data;
    merlin_loopback_set_t loopback;
    merlin_tx_cfg_t tx_cfg;
} merlin_control_t;


typedef int (*merlin_core_init_cb)(E_MERLIN_ID core_id, E_MERLIN_VCO vco);
typedef int (*merlin_lane_init_cb)(E_MERLIN_LANE lane_id, LPORT_PORT_MUX_SELECT mux_sel);
typedef int (*merlin_speed_set_cb)(E_MERLIN_LANE lane_id, LPORT_PORT_RATE port_rate);
typedef int (*merlin_get_status_cb)(E_MERLIN_LANE lane_index, lport_port_status_s *port_status);
typedef int (*merlin_ioctl_cb)(E_MERLIN_LANE lane_id, merlin_command_t cmd, merlin_control_t *data);

typedef struct
{
    merlin_core_init_cb merlin_core_init;
    merlin_lane_init_cb merlin_lane_init;
    merlin_speed_set_cb merlin_speed_set;
    merlin_get_status_cb merlin_get_status;
    merlin_ioctl_cb merlin_ioctl;
} merlin_sdk_cb_s;

int write_serdes_reg(E_MERLIN_ID merlin_id, uint32_t addr,uint16_t mask,uint16_t value);
int read_serdes_reg(E_MERLIN_ID merlin_id, uint32_t addr, uint16_t mask,uint16_t *value);
int lport_serdes_init(lport_init_s *init_params);
int lport_serdes_get_status(uint32_t port, lport_port_status_s *port_status);
int lport_serdes_speed_set(uint32_t port, LPORT_PORT_RATE rate);
int lport_serdes_set_tx_cfg(uint32_t port, uint16_t pre, uint16_t main, uint16_t post1, uint16_t post2, uint16_t hpf);
int lport_serdes_get_tx_cfg(uint32_t port, uint16_t *pre, uint16_t *main, uint16_t *post1, uint16_t *post2, uint16_t *hpf);

/* Control SFP TX disable GPIO state */
int port_write_tx_dis_state(uint32_t port, uint32_t state);


/* 
    Generation MPRBB pattern on SERDES Port
    MPRB Pattern are:
        PRBS7|PRBS9|PRBS11|PRBS15|PRBS23|PRBS31|PRBS58|8180
*/
int lport_serdes_prbs_generation(uint32_t port,E_SERDES_PRBS_PATTERN_TYPE pattern_type);
/* 
    Monitor MPRBB pattern on SERDES Port
    MPRB Pattern are:
        PRBS7|PRBS9|PRBS11|PRBS15|PRBS23|PRBS31|PRBS58|
*/
int lport_serdes_prbs_monitor(uint32_t port, E_SERDES_PRBS_PATTERN_TYPE pattern_type);

/* Get Counter Ststistics after ebable MPRB */
int lport_serdes_prbs_stats_get(uint32_t port);

#endif /* SHARED_OPENSOURCE_DRV_LPORT_SERDES_ACCESS_H_ */
