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
 * lport_defs.h
 *
 *  Created on: April 2015
 *      Author: yonatani
 */

#ifndef SHARED_OPENSOURCE_DRV_LPORT_LPORT_DEFS_H_
#define SHARED_OPENSOURCE_DRV_LPORT_LPORT_DEFS_H_


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
#define LPORT_IS_XFI_PORT(mux) (mux >= PORT_SFI && mux <= PORT_XFI)

/* TSC_CLK = 644MHz = 1.553ns, TS_CLK = 250MHz = 4ns
 * ts_tsts_adjust = 2.5 TSC_CLK period + One TS_CLK period = 2.5 * 1.553 + 4 = 7.88
 * ts_osts_adjust = 6 TSC CLK period + One TS_CLK period = 6 * 1.553 + 4 = 13.2
 */
#define TS_TSTS_ADJ 8
#define TS_OSTS_ADJ 13

#ifdef _CFE_
#define CFE_NUM_OF_PORTS LPORT_NUM_OF_PORTS
#include "lib_types.h"
#else
#include <linux/types.h>
#endif

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
    PORT_SGMII_AN_IEEE_CL37,
    PORT_SGMII_AN_USER_CL37,
    PORT_SGMII_AN_SLAVE,
    PORT_SGMII_AN_MASTER,
    PORT_HSGMII,
    PORT_SFI,
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
    uint8_t phy_attached; /*Phy is connected to RGMII port*/
    uint16_t phyid;/*PhyID if exists*/
}lport_rgmii_cfg_s;

typedef struct
{
    LPORT_PORT_MUX_SELECT prt_mux_sel[LPORT_NUM_OF_PORTS];
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
