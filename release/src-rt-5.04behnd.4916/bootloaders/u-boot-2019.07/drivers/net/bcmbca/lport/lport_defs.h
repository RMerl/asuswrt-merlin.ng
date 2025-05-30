// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

   
   */
/*
 * lport_defs.h
 *
 *  Created on: April 2015
 *      Author: yonatani
 */

#ifndef SHARED_OPENSOURCE_DRV_LPORT_LPORT_DEFS_H_
#define SHARED_OPENSOURCE_DRV_LPORT_LPORT_DEFS_H_

#include "lport_types.h"

//defines
#define LPORT_TX_THRESHOLD 4
#define LPORT_NUM_OF_PORTS_PER_XLMAC 4
#define LPORT_NUM_OF_XLMACS 2
/* ONLY 7 ports are applicable in revision A0 */
#define LPORT_NUM_OF_PORTS ((LPORT_NUM_OF_XLMACS * LPORT_NUM_OF_PORTS_PER_XLMAC))
#define LPORT_NUM_OF_RGMII 3
#define LPORT_LAST_EGPHY_PORT 3
#define LPORT_FIRST_RGMII_PORT 4
#define LPORT_IS_SERDES_PORT(mux) (mux >= PORT_SGMII && mux <= PORT_XFI)
#define LPORT_IS_XFI_PORT(mux) (mux == PORT_XFI)

/* TSC_CLK = 644MHz = 1.553ns, TS_CLK = 250MHz = 4ns
 * ts_tsts_adjust = 2.5 TSC_CLK period + One TS_CLK period = 2.5 * 1.553 + 4 = 7.88
 * ts_osts_adjust = 6 TSC CLK period + One TS_CLK period = 6 * 1.553 + 4 = 13.2
 */
#define TS_TSTS_ADJ 8
#define TS_OSTS_ADJ 13

//typedefs
typedef enum
{
    LPORT_ERR_OK = 0,
    LPORT_ERR_PARAM = -1,
    LPORT_ERR_STATE = -2,
    LPORT_ERR_IO = -3,
    LPORT_ERR_RANGE = -3,
    LPORT_ERR_FIFO_EMPTY = -4,
    LPORT_ERR_INVALID = -5,
}LPORT_RC;

typedef enum
{
    PORT_UNAVAIL = 0,
    PORT_SGMII,
    PORT_HSGMII,
    PORT_XFI,
    PORT_GPHY,
    PORT_RGMII
}LPORT_PORT_MUX_SELECT;

typedef enum
{
    LPORT_RATE_UNKNOWN = 0,
    LPORT_RATE_10MB,
    LPORT_RATE_100MB,
    LPORT_RATE_1000MB,
    LPORT_RATE_2500MB,
    LPORT_RATE_10G,
}LPORT_PORT_RATE;

typedef enum
{
    LPORT_HALF_DUPLEX,
    LPORT_FULL_DUPLEX
}LPORT_PORT_DUPLEX;

typedef enum
{
    XLMAC_CRC_APPEND,
    XLMAC_CRC_KEEP,
    XLMAC_CRC_REPLACE,
    XLMAC_CRC_PERPKT
}XLMAC_CRC_MODE;

typedef struct
{
    uint8_t valid;
    uint8_t eee_enable;
    uint8_t rvmii_ref;/*0=50mhz,1=25mhz*/
    uint8_t portmode;/*2=Mii,3=RGMII,4=RvMII*/
    uint8_t delay_rx;/*rgmii rx delay enabled*/
    uint8_t delay_tx;/*rgmii tx delay enabled*/
    uint8_t ib_status_overide;/*if set user can set the status ,used when working in MAC2MAC mode*/
    uint8_t is_1p8v; /*rgmii connected to 1.8v*/
    uint8_t phy_attached; /*Phy is connected to RGMII port*/
    uint16_t phyid;/*PhyID if exists*/
}lport_rgmii_cfg_s;

typedef struct
{
    LPORT_PORT_MUX_SELECT prt_mux_sel[LPORT_NUM_OF_PORTS];
    int has_rgmii_cfg;
    lport_rgmii_cfg_s rgmii_cfg[LPORT_NUM_OF_RGMII];
}lport_init_s;

typedef struct
{
    uint8_t local_loopback;
    uint8_t pad_en;
    uint8_t pad_threashold;
    uint8_t average_igp;
    uint8_t tx_threshold;
    uint8_t tx_preamble_len;
    uint8_t throt_num;
    uint8_t throt_denom;
    LPORT_PORT_RATE speed;
}lport_port_cfg_s;

typedef struct
{
    uint64_t tx_ctrl_sa;
    uint64_t rx_ctrl_sa;
    uint8_t  rx_pass_ctrl;
    uint8_t  rx_pass_pause;
    uint16_t pause_xoff_timer;
    uint8_t  rx_pause_en;
    uint8_t  tx_pause_en;
    uint8_t  pause_refresh_en;
    uint16_t pause_refresh_timer;
}lport_flow_ctrl_cfg_s;

typedef struct
{
    uint8_t autoneg_en;
    uint8_t port_up;
    LPORT_PORT_RATE rate;
    LPORT_PORT_DUPLEX duplex;
    uint8_t rx_pause_en;
    uint8_t tx_pause_en;
}lport_port_status_s;

typedef struct
{
    uint8_t autoneg_en;
    uint8_t port_up;
    uint32_t rate_adv_map;
    LPORT_PORT_DUPLEX duplex;
    uint8_t rx_pause_en;
    uint8_t tx_pause_en;
}lport_port_phycfg_s;

typedef struct
{
    uint8_t rgmii_da_mac[6];
    uint16_t payload_len;
    uint8_t ate_en;
}lport_rgmii_ate_s;

#endif /* SHARED_OPENSOURCE_DRV_LPORT_LPORT_DEFS_H_ */
