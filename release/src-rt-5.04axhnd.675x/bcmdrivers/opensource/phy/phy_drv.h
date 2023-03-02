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
 *  Created on: Dec 2015
 *      Author: yuval.raviv@broadcom.com
 */

#ifndef __PHY_DRV_H__
#define __PHY_DRV_H__

#include "bus_drv.h"
#include "dt_access.h"

#define PHY_CAP_10_HALF         (1 << 0)
#define PHY_CAP_10_FULL         (1 << 1)
#define PHY_CAP_100_HALF        (1 << 2)
#define PHY_CAP_100_FULL        (1 << 3)
#define PHY_CAP_1000_HALF       (1 << 4)
#define PHY_CAP_1000_FULL       (1 << 5)
#define PHY_CAP_2500            (1 << 6)
#define PHY_CAP_5000            (1 << 7)
#define PHY_CAP_10000           (1 << 8)
#define PHY_CAP_AUTONEG         (1 << 9)
#define PHY_CAP_PAUSE           (1 << 10)
#define PHY_CAP_PAUSE_ASYM      (1 << 11)
#define PHY_CAP_REPEATER        (1 << 12)
#define PHY_CAP_LAST            (1 << 13)
#define PHY_CAP_ALL             (PHY_CAP_LAST - 1)

typedef enum
{
    PHY_SPEED_UNKNOWN,
    PHY_SPEED_AUTO = PHY_SPEED_UNKNOWN,
    PHY_SPEED_10        =    10,
    PHY_SPEED_100       =   100,
    PHY_SPEED_1000      =  1000,
    PHY_SPEED_2500      =  2500,
    PHY_SPEED_5000      =  5000,
    PHY_SPEED_10000     = 10000,
} phy_speed_t;

typedef enum
{
    PHY_DUPLEX_UNKNOWN,
    PHY_DUPLEX_HALF,
    PHY_DUPLEX_FULL,
} phy_duplex_t;

typedef enum
{
    PHY_MII_TYPE_UNKNOWN,
    PHY_MII_TYPE_MII,
    PHY_MII_TYPE_TMII,
    PHY_MII_TYPE_GMII,
    PHY_MII_TYPE_RGMII,
    PHY_MII_TYPE_SGMII,
    PHY_MII_TYPE_HSGMII,
    PHY_MII_TYPE_XFI,
    PHY_MII_TYPE_SERDES,
    PHY_MII_TYPE_LAST,
} phy_mii_type_t;

static inline const char *phy_mii_type(phy_mii_type_t interface)
{
	switch (interface) {
	case PHY_MII_TYPE_UNKNOWN:
		return "";
	case PHY_MII_TYPE_MII:
		return "mii";
	case PHY_MII_TYPE_TMII:
		return "tmii";
	case PHY_MII_TYPE_GMII:
		return "gmii";
	case PHY_MII_TYPE_RGMII:
		return "rgmii";
	case PHY_MII_TYPE_SGMII:
		return "sgmii";
	case PHY_MII_TYPE_HSGMII:
		return "hsgmii";
	case PHY_MII_TYPE_XFI:
		return "xfi";
	case PHY_MII_TYPE_SERDES:
		return "serdes";
	default:
		return "unknown";
	}
}

typedef enum
{
    PHY_TYPE_UNKNOWN,
    PHY_TYPE_6858_EGPHY,
    PHY_TYPE_6846_EGPHY,
    PHY_TYPE_6856_SGMII,
    PHY_TYPE_EXT1,
    PHY_TYPE_EXT2,
    PHY_TYPE_EXT3,
    PHY_TYPE_LPORT_SERDES,
    PHY_TYPE_53125,
    PHY_TYPE_PON,
    PHY_TYPE_DSL_GPHY,
    PHY_TYPE_138CLASS_SERDES,
    PHY_TYPE_158CLASS_SERDES,
    PHY_TYPE_146CLASS_SERDES,
    PHY_TYPE_6756CLASS_SERDES,
    PHY_TYPE_I2C_PHY,
    PHY_TYPE_CROSSBAR,
    PHY_TYPE_MAC2MAC,
    PHY_TYPE_G9991,
    PHY_TYPE_GPY211,
    PHY_TYPE_MAX,
} phy_type_t;

typedef void (*link_change_cb_t)(void *ctx);

typedef enum {
    INTER_PHY_TYPE_MIN,
    INTER_PHY_TYPE_100BASE_FX = INTER_PHY_TYPE_MIN,
    INTER_PHY_TYPE_1000BASE_X,
    INTER_PHY_TYPE_1GBASE_X,
    INTER_PHY_TYPE_1GBASE_R,

    INTER_PHY_TYPE_2P5GBASE_X,
    INTER_PHY_TYPE_2P5GBASE_R,
    INTER_PHY_TYPE_2500BASE_X,
    INTER_PHY_TYPE_2P5GIDLE,

    INTER_PHY_TYPE_5GBASE_R,
    INTER_PHY_TYPE_5GBASE_X,
    INTER_PHY_TYPE_5000BASE_X,
    INTER_PHY_TYPE_5GIDLE,

    INTER_PHY_TYPE_10GBASE_R,
    INTER_PHY_TYPE_SGMII,
    INTER_PHY_TYPE_USXGMII,
    INTER_PHY_TYPE_USXGMII_MP,
    INTER_PHY_TYPE_MLTI_SPEED_BASE_X_AN,

    INTER_PHY_TYPE_UNKNOWN,
} inter_phy_type_t;

//USXGMII-M support
typedef enum {
    USXGMII_M_NONE,
    USXGMII_S = USXGMII_M_NONE,
    USXGMII_M_10G_S,
    USXGMII_M_10G_D,
    USXGMII_M_10G_Q,

    USXGMII_M_5G_S,
    USXGMII_M_5G_D,
    USXGMII_M_2G5_S,
    USXGMII_M_MAX
} usxgmii_m_type_t;

extern char * usxgmii_m_type_strs[];

static inline int usxgmii_m_total_ports(usxgmii_m_type_t usxgmii_m_type)
{
    static int port_num[] =
    {
        [USXGMII_S]         = 1,
        [USXGMII_M_10G_S]   = 1,
        [USXGMII_M_10G_D]   = 2,
        [USXGMII_M_10G_Q]   = 4,

        [USXGMII_M_5G_S]    = 1,
        [USXGMII_M_5G_D]    = 2,
        [USXGMII_M_2G5_S]   = 1,
    };

    return port_num[usxgmii_m_type];
}

#define IS_USXGMII_MULTI_PORTS(phy_dev) (usxgmii_m_total_ports(phy_dev->usxgmii_m_type) > 1)

/* Phy device */
typedef struct phy_dev_s
{
    struct phy_drv_s *phy_drv;
    bus_drv_t *bus_drv;
    phy_mii_type_t mii_type;
    link_change_cb_t link_change_cb;
    void *link_change_ctx;
    uint32_t addr;          // contains phy address only
    void *priv;
    int link;
    phy_speed_t speed;
    phy_duplex_t duplex;
    int caps_mask;
    int pause_rx;
    int pause_tx;
    int delay_rx;
    int delay_tx;
    int swap_pair;
    int flag;
    int loopback_save;
    void *macsec_dev;
    /* For cascaded PHY */
    void *sw_port;
    struct phy_dev_s *cascade_next;
    struct phy_dev_s *cascade_prev;
    dt_handle_t dt_handle;
    dt_gpio_desc gpiod_phy_reset;
    dt_gpio_desc gpiod_tx_disable;
    int core_index;  /* For PHY structure it matters in the whole chip */
    int lane_index;  /* For PHY structure it matters in the whole chip */
    usxgmii_m_type_t usxgmii_m_type;
    int usxgmii_m_index;
    struct phy_dev_s *mphy_base;
    int inter_phy_types;        /* Device total capable inter phy types */
    int common_inter_phy_types; /* Common inter phy types with cascaded PHY */
    int configured_inter_phy_types;    /* configured inter types for different speeds */
    inter_phy_type_t current_inter_phy_type;     /* current inter phy type for fixed speed configuration or link up */
    int an_enabled;     /* AN for specific speed */
    void *descriptor; /* For some PHY familiy workaround */
#define PHY_NAME_LEN 32
    char name[PHY_NAME_LEN];
} phy_dev_t;

#define PHY_FLAG_NOT_PRESENTED                  (1<<0)      /* for SFP module indicating not inserted */
#define PHY_FLAG_POWER_SET_ENABLED              (1<<1)
#define PHY_FLAG_DYNAMIC                        (1<<2)
#define PHY_FLAG_CABLE_DIAG_ENABLED             (1<<3)
#define PHY_FLAG_TO_EXTSW                       (1<<4)
#define PHY_FLAG_CABLE_DIAG_INITED              (1<<5)
#define PHY_FLAG_CONF_PAUSE_RX                  (1<<6)
#define PHY_FLAG_CONF_PAUSE_TX                  (1<<7)
#define PHY_FLAG_CONF_PAUSE_VALID               (1<<8)
#define PHY_FLAG_COPPER_SFP_TYPE                (1<<9)
#define PHY_FLAG_COPPER_CONFIGURABLE_SFP_TYPE   (1<<10)
#define PHY_FLAG_INITED                         (1<<11)
#define PHY_FLAG_EXTPHY                         (1<<12)
#define PHY_FLAG_SKIP_READ_STATUS               (1<<13)
#define PHY_FLAG_FIXED_CONN                     (1<<14)
#define PHY_FLAG_WAKE_ON_LAN                    (1<<15)

#define PhyIsPortConnectedToExternalSwitch(phy) (((phy)->flag & PHY_FLAG_TO_EXTSW)?1:0)
#define PhyIsExtPhyId(phy)                      (((phy)->flag & PHY_FLAG_EXTPHY)?1:0)
#define PhyIsFixedConnection(phy)   (((phy)->flag & PHY_FLAG_FIXED_CONN) != 0)
#define PhyHasWakeOnLan(phy)        ((phy)->flag & PHY_FLAG_WAKE_ON_LAN)

typedef struct phy_wol_info {
	uint8_t mac_addr[6];	// pkt1
} phy_wol_info;

#define CAPS_TYPE_ADVERTISE      0
#define CAPS_TYPE_SUPPORTED      1
#define CAPS_TYPE_LP_ADVERTISED  2

static uint32_t inter_phy_supported_speed_caps[] = {
    [INTER_PHY_TYPE_100BASE_FX] = PHY_CAP_100_FULL,
    [INTER_PHY_TYPE_1000BASE_X] = PHY_CAP_1000_FULL,
    [INTER_PHY_TYPE_1GBASE_X]   = PHY_CAP_1000_FULL,
    [INTER_PHY_TYPE_1GBASE_R]   = PHY_CAP_1000_FULL,

    [INTER_PHY_TYPE_2P5GBASE_X] = PHY_CAP_2500,
    [INTER_PHY_TYPE_2P5GBASE_R] = PHY_CAP_2500,
    [INTER_PHY_TYPE_2500BASE_X] = PHY_CAP_2500,
    [INTER_PHY_TYPE_2P5GIDLE]   = PHY_CAP_2500,

    [INTER_PHY_TYPE_5GBASE_R]   = PHY_CAP_5000,
    [INTER_PHY_TYPE_5GBASE_X]   = PHY_CAP_5000,
    [INTER_PHY_TYPE_5000BASE_X] = PHY_CAP_5000,
    [INTER_PHY_TYPE_5GIDLE]     = PHY_CAP_5000,

    [INTER_PHY_TYPE_10GBASE_R]  = PHY_CAP_10000,
    [INTER_PHY_TYPE_SGMII]      = PHY_CAP_100_FULL | PHY_CAP_1000_FULL,
    [INTER_PHY_TYPE_USXGMII]    = PHY_CAP_100_FULL | PHY_CAP_1000_FULL | PHY_CAP_2500 | PHY_CAP_5000, PHY_CAP_10000,
    [INTER_PHY_TYPE_USXGMII_MP] = PHY_CAP_100_FULL | PHY_CAP_1000_FULL | PHY_CAP_2500 | PHY_CAP_5000, PHY_CAP_10000,

    [INTER_PHY_TYPE_MLTI_SPEED_BASE_X_AN] = PHY_CAP_100_FULL | PHY_CAP_1000_FULL | PHY_CAP_2500 | PHY_CAP_5000, PHY_CAP_10000,
};

#define INTER_PHY_TYPE_100BASE_FX_M     (1<<INTER_PHY_TYPE_100BASE_FX)
#define INTER_PHY_TYPE_1000BASE_X_M     (1<<INTER_PHY_TYPE_1000BASE_X)
#define INTER_PHY_TYPE_1GBASE_X_M       (1<<INTER_PHY_TYPE_1GBASE_X)
#define INTER_PHY_TYPE_1GBASE_R_M       (1<<INTER_PHY_TYPE_1GBASE_R)

#define INTER_PHY_TYPE_2P5GBASE_X_M     (1<<INTER_PHY_TYPE_2P5GBASE_X)
#define INTER_PHY_TYPE_2P5GBASE_R_M     (1<<INTER_PHY_TYPE_2P5GBASE_R)
#define INTER_PHY_TYPE_2500BASE_X_M     (1<<INTER_PHY_TYPE_2500BASE_X)
#define INTER_PHY_TYPE_2P5GIDLE_M       (1<<INTER_PHY_TYPE_2P5GIDLE)

#define INTER_PHY_TYPE_5GBASE_X_M       (1<<INTER_PHY_TYPE_5GBASE_X)
#define INTER_PHY_TYPE_5GBASE_R_M       (1<<INTER_PHY_TYPE_5GBASE_R)
#define INTER_PHY_TYPE_5000BASE_X_M       (1<<INTER_PHY_TYPE_5000BASE_X)
#define INTER_PHY_TYPE_5GIDLE_M         (1<<INTER_PHY_TYPE_5GIDLE)

#define INTER_PHY_TYPE_10GBASE_R_M      (1<<INTER_PHY_TYPE_10GBASE_R)
#define INTER_PHY_TYPE_SGMII_M          (1<<INTER_PHY_TYPE_SGMII)
#define INTER_PHY_TYPE_MLTI_SPEED_BASE_X_AN_M  (1<<INTER_PHY_TYPE_MLTI_SPEED_BASE_X_AN)
#define INTER_PHY_TYPE_USXGMII_M        (1<<INTER_PHY_TYPE_USXGMII)

#define INTER_PHY_TYPE_USXGMII_MP_M     (1<<INTER_PHY_TYPE_USXGMII_MP)

#define INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M \
    (INTER_PHY_TYPE_SGMII_M | INTER_PHY_TYPE_USXGMII_M | \
     INTER_PHY_TYPE_USXGMII_MP_M | INTER_PHY_TYPE_MLTI_SPEED_BASE_X_AN_M )

#define INTER_PHY_TYPE_IS_MULTI_SPEED_AN(inter_type) (((1<<inter_type) & INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M)>0)

#define INTER_PHY_TYPE_AN_AND_FORCED_SPEED(inter_type) \
    (((1<<inter_type)&(INTER_PHY_TYPE_1000BASE_X_M | INTER_PHY_TYPE_1GBASE_R_M | INTER_PHY_TYPE_2P5GBASE_X_M | \
        INTER_PHY_TYPE_2P5GBASE_R_M | INTER_PHY_TYPE_5GBASE_R_M | INTER_PHY_TYPE_5000BASE_X_M | INTER_PHY_TYPE_10GBASE_R_M)) > 0)

#define INTER_PHY_TYPE_AN_ONLY(inter_type) \
    (((1<<inter_type) & (INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M ))>0)

#define INTER_PHY_TYPE_FORCED_SPEED_ONLY(inter_type) \
    (!INTER_PHY_TYPE_AN_ONLY(inter_type) && !INTER_PHY_TYPE_AN_AND_FORCED_SPEED(inter_type))

#define INTER_PHY_TYPE_AN_SUPPORT(inter_type) \
    (INTER_PHY_TYPE_AN_ONLY(inter_type) || INTER_PHY_TYPE_AN_AND_FORCED_SPEED(inter_type))

#define INTER_PHY_TYPE_FORCED_SPEED_SUPPORT(inter_type) \
    (INTER_PHY_TYPE_FORCED_SPEED_ONLY(inter_type) || INTER_PHY_TYPE_AN_AND_FORCED_SPEED(inter_type))

/*
    Set all types bit plus UNKNOWN bit to cover all types for backward
    compatible and dishtiguish from real all types case
*/
#define INTER_PHY_TYPE_UNKNOWN_M ((1<<(INTER_PHY_TYPE_UNKNOWN + 1))-1)
#define INTER_PHY_TYPE_FULLTYPES_M ((1<<(INTER_PHY_TYPE_UNKNOWN))-1)

#define INTER_PHY_TYPES_SPEED_MASK_100M  INTER_PHY_TYPE_100BASE_FX_M

#define INTER_PHY_TYPES_SPEED_MASK_1G  ( \
    INTER_PHY_TYPE_1000BASE_X_M | INTER_PHY_TYPE_1GBASE_X_M | INTER_PHY_TYPE_1GBASE_R_M )

#define INTER_PHY_TYPES_SPEED_MASK_2P5G ( \
    INTER_PHY_TYPE_2P5GBASE_X_M | INTER_PHY_TYPE_2P5GBASE_R_M | \
    INTER_PHY_TYPE_2500BASE_X_M | INTER_PHY_TYPE_2P5GIDLE_M )

#define INTER_PHY_TYPES_SPEED_MASK_5G ( \
    INTER_PHY_TYPE_5GBASE_X_M | INTER_PHY_TYPE_5GBASE_R_M | \
    INTER_PHY_TYPE_5000BASE_X_M | INTER_PHY_TYPE_5GIDLE_M )

#define INTER_PHY_TYPES_SPEED_MASK_10G INTER_PHY_TYPE_10GBASE_R_M

static inline int phy_speed_to_inter_phy_speed_mask(phy_speed_t speed)
{
    switch(speed)
    {
        default:
        case PHY_SPEED_10:
        case PHY_SPEED_AUTO: // and case PHY_SPEED_UNKNOWN:
            /* We just do speed conversion, will leave other control to caller */
            return INTER_PHY_TYPE_FULLTYPES_M;
        case PHY_SPEED_100:
            return INTER_PHY_TYPES_SPEED_MASK_100M;
        case PHY_SPEED_1000:
            return INTER_PHY_TYPES_SPEED_MASK_1G;
        case PHY_SPEED_2500:
            return INTER_PHY_TYPES_SPEED_MASK_2P5G;
        case PHY_SPEED_5000:
            return INTER_PHY_TYPES_SPEED_MASK_5G;
        case PHY_SPEED_10000:
            return INTER_PHY_TYPES_SPEED_MASK_10G;
    }
    return INTER_PHY_TYPE_UNKNOWN_M;
}
/*
    INTER_PHY_TYPES Naming rule:
    o Top Letters before digits: S: SGMII; U: USXGMII-S; M: USXGMII-M; A: MultiSpeedAN
    o Letters after first one is [SPEED] : 0, 1, 2, 5, 10 means 100M, 1G, 2.5G, 5G and 10G
    o Letters after [SPEED]: F: 100Base-FX; X: Base-X; I: Idle Stuffing; R: Base-R(Replicated) K: N000Base-X,
                           :
    ex: INTER_PHY_TYPES_US1XK2KXIR5XIR10R_M means:
        USXGMII, SGMII, 1GBase-X, 1000Base-X,
            2.5GBase-X, 2500Base-X, 2.5G Idle Stuffing, 2.5GBase-R,
            5GBase-X, 5G Idle Stuffing, 5GBase-R, 10GBase-R capable
*/
#define INTER_PHY_TYPES_S0F1K_M \
    ( INTER_PHY_TYPE_SGMII_M | INTER_PHY_TYPE_100BASE_FX_M | INTER_PHY_TYPE_1000BASE_X_M )

#define INTER_PHY_TYPES_S0F1K2K_M ( INTER_PHY_TYPES_S0F1K_M | INTER_PHY_TYPE_2500BASE_X_M )

#define INTER_PHY_TYPES_S1K_M \
    ( INTER_PHY_TYPE_SGMII_M | INTER_PHY_TYPE_1000BASE_X_M )

#define INTER_PHY_TYPES_S1K2K_M ( INTER_PHY_TYPES_S1K_M | INTER_PHY_TYPE_2500BASE_X_M )

#define INTER_PHY_TYPES_S0F1K2K5R10R_M ( \
    INTER_PHY_TYPES_S0F1K2K_M | INTER_PHY_TYPE_5GBASE_R_M | INTER_PHY_TYPE_10GBASE_R_M )

#define INTER_PHY_TYPES_S1K2KI5I_M ( \
    INTER_PHY_TYPES_S1K_M | INTER_PHY_TYPE_2500BASE_X_M | \
    INTER_PHY_TYPE_2P5GIDLE_M | INTER_PHY_TYPE_5GIDLE_M )

#define INTER_PHY_TYPES_S1K2KI5KI_M ( \
    INTER_PHY_TYPES_S1K2KI5I_M | INTER_PHY_TYPE_5000BASE_X_M)

#define INTER_PHY_TYPES_US1K2KI5KI_M ( \
    INTER_PHY_TYPE_USXGMII_M | INTER_PHY_TYPES_S1K2KI5KI_M )

#define INTER_PHY_TYPES_US1K2KI5KI10R_M ( \
    INTER_PHY_TYPES_US1K2KI5KI_M | INTER_PHY_TYPE_10GBASE_R_M )

#define INTER_PHY_TYPES_US1K2KIR_M ( \
    INTER_PHY_TYPE_USXGMII_M | INTER_PHY_TYPES_S1K2K_M | \
    INTER_PHY_TYPE_2P5GIDLE_M | INTER_PHY_TYPE_2P5GBASE_R_M )

#define INTER_PHY_TYPES_US1K2KIR5KIR_M ( \
    INTER_PHY_TYPES_US1K2KIR_M | \
    INTER_PHY_TYPE_5000BASE_X_M | INTER_PHY_TYPE_5GIDLE_M | INTER_PHY_TYPE_5GBASE_R_M )

#define INTER_PHY_TYPES_US1K2KIR5KIR10R_M ( \
    INTER_PHY_TYPES_US1K2KIR5KIR_M | INTER_PHY_TYPE_10GBASE_R_M )

#define INTER_PHY_TYPES_UMS1K2KIR5KIR10R_M ( \
    INTER_PHY_TYPES_US1K2KIR5KIR10R_M | INTER_PHY_TYPE_USXGMII_MP_M )

#define INTER_PHY_TYPES_US1KR2KXR_M ( \
    INTER_PHY_TYPE_USXGMII_M | INTER_PHY_TYPES_S1K2K_M | INTER_PHY_TYPE_1GBASE_R_M | \
    INTER_PHY_TYPE_2P5GBASE_X_M | INTER_PHY_TYPE_2P5GBASE_R_M )

#define INTER_PHY_TYPES_AUS1KR2KXR_M ( \
    INTER_PHY_TYPES_US1KR2KXR_M | INTER_PHY_TYPE_MLTI_SPEED_BASE_X_AN_M )

#define INTER_PHY_TYPES_US1KR2KXR5KXR10R_M ( \
    INTER_PHY_TYPES_US1KR2KXR_M | INTER_PHY_TYPE_5000BASE_X_M | INTER_PHY_TYPE_5GBASE_X_M | \
    INTER_PHY_TYPE_5GBASE_R | INTER_PHY_TYPE_10GBASE_R )

#define INTER_PHY_TYPES_AUS1KR2KXR5KXR_M ( \
    INTER_PHY_TYPES_AUS1KR2KXR_M | INTER_PHY_TYPE_5000BASE_X_M | \
    INTER_PHY_TYPE_5GBASE_X_M | INTER_PHY_TYPE_5GBASE_R_M)

#define INTER_PHY_TYPES_AMUS1KR2KXR5KXR_M ( \
    INTER_PHY_TYPES_AUS1KR2KXR5KXR_M | INTER_PHY_TYPE_USXGMII_MP_M )

#define INTER_PHY_TYPES_AUS1KR2KXR5KXR10R_M ( \
    INTER_PHY_TYPES_AUS1KR2KXR5KXR_M |INTER_PHY_TYPE_10GBASE_R_M)

#define INTER_PHY_TYPES_AMUS1KR2KXR5KXR10R_M ( \
    INTER_PHY_TYPES_AUS1KR2KXR5KXR10R_M | INTER_PHY_TYPE_USXGMII_MP_M )

#define INTER_PHY_TYPES_S1K2KR5R_M (\
    INTER_PHY_TYPES_S1K2K_M|INTER_PHY_TYPE_5GBASE_R_M)

typedef enum {
    INTER_PHY_TYPE_UP,      /* interface type on upword toward MAC */
    INTER_PHY_TYPE_DOWN,    /* interface types on downward away from MAC */
} inter_phy_types_dir_t;

static inline int get_inter_phy_supported_speed_caps(int inter_phy_types, uint32_t *supported_speed_caps)
{
    int i;

    *supported_speed_caps = 0;

    for (i=0; i<INTER_PHY_TYPE_UNKNOWN; i++)
    {
        if (!(inter_phy_types & (1<<i)))
            continue;

        *supported_speed_caps |= inter_phy_supported_speed_caps[i];
    }
    return 0;
}

/* Phy driver */
typedef struct phy_drv_s
{
    phy_type_t phy_type;
    char *name;
    int initialized;
    int (*read)(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val);
    int (*write)(phy_dev_t *phy_dev, uint16_t reg, uint16_t val);
    int (*power_get)(phy_dev_t *phy_dev, int *enable);
    int (*power_set)(phy_dev_t *phy_dev, int enable);
    int (*set_wol_sleep)(phy_dev_t *phy_dev, phy_wol_info *pwi);
    int (*apd_get)(phy_dev_t *phy_dev, int *enable);
    int (*apd_set)(phy_dev_t *phy_dev, int enable);
    int (*eee_get)(phy_dev_t *phy_dev, int *enable);
    int (*eee_set)(phy_dev_t *phy_dev, int enable);
    int (*eee_resolution_get)(phy_dev_t *phy_dev, int *enable);
    int (*read_status)(phy_dev_t *phy_dev);
    int (*speed_set)(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
    int (*config_speed_get)(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex);
    int (*caps_get)(phy_dev_t *phy_dev, int caps_type, uint32_t *caps);
    int (*caps_set)(phy_dev_t *phy_dev, uint32_t caps);
    int (*inter_phy_types_get)(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, uint32_t *types);
    inter_phy_type_t (*current_inter_phy_type_get)(phy_dev_t *phy_dev);
    int (*configured_inter_phy_types_set)(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, uint32_t types);
    //int (*configured_inter_phy_types_get)(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, uint32_t *types);
    int (*common_inter_phy_types_get)(phy_dev_t *phy_dev, uint32_t *types);
    int (*phyid_get)(phy_dev_t *phy_dev, uint32_t *phyid);
    int (*init)(phy_dev_t *phy_dev);
    int (*link_change_register)(phy_dev_t *phy_dev);
    int (*link_change_unregister)(phy_dev_t *phy_dev);
    int (*dev_add)(phy_dev_t *phy_dev);
    int (*dev_del)(phy_dev_t *phy_dev);
    int (*drv_init)(struct phy_drv_s *phy_drv);
    int (*isolate_phy)(phy_dev_t *phy_dev, int isolate);
    int (*super_isolate_phy)(phy_dev_t *phy_dev, int isolate);
    int (*loopback_get) (phy_dev_t *phy_dev, int *enable, phy_speed_t *speed);
    int (*loopback_set) (phy_dev_t *phy_dev, int enable, phy_speed_t speed);
    int (*pair_swap_set)(phy_dev_t *phy_dev, int enable);
    int (*cable_diag_run) (phy_dev_t *phy_dev, int *result, int *pair_len);
    int (*cable_diag_set) (phy_dev_t *phy_dev, int enable);
    int (*cable_diag_get) (phy_dev_t *phy_dev, int *enable);
    int (*auto_mdix_set) (phy_dev_t *phy_dev, int enable);
    int (*auto_mdix_get) (phy_dev_t *phy_dev, int *enable);
    int (*wirespeed_set) (phy_dev_t *phy_dev, int enable);
    int (*wirespeed_get) (phy_dev_t *phy_dev, int *enable);
    int (*macsec_oper) (phy_dev_t *phy_dev, void *data);
    int (*dt_priv)(dt_handle_t handle, uint32_t addr, uint32_t phy_mode, void **_priv);
    int (*leds_init) (phy_dev_t *phy_dev, void *leds_info);
    char *(*get_phy_name)(phy_dev_t *phy_dev);
    int (*silent_start_get) (phy_dev_t *phy_dev, int *enable);
    int (*silent_start_set) (phy_dev_t *phy_dev, int enable);
    int (*priv_fun) (phy_dev_t *phy_dev, int op_code, va_list ap);    /* private function for each class */
    int (*an_restart)(phy_dev_t *phy_dev);
} phy_drv_t;

static inline int phy_dev_an_restart(phy_dev_t *phy_dev)
{
    phy_drv_t *phy_drv = phy_dev->phy_drv;
    if (!phy_drv->an_restart)
        return 0;
    return phy_drv->an_restart(phy_dev);
}

static inline char *phy_dev_get_phy_name(phy_dev_t *phy_dev)
{
    phy_drv_t *phy_drv = phy_dev->phy_drv;

    if (strlen(phy_dev->name))
        return phy_dev->name;

    if (phy_drv->get_phy_name)
        strncpy(phy_dev->name, (*phy_drv->get_phy_name)(phy_dev), PHY_NAME_LEN-1);
    else
        /* If no PHY device name, set it to PHY driver default name */
        strncpy(phy_dev->name, phy_drv->name, PHY_NAME_LEN-1);

    return phy_dev->name;
}

typedef struct
{
    char *desc;
    uint16_t reg;
    uint16_t val;
} prog_entry_t;

phy_dev_t *phy_dev_get(phy_type_t phy_type, uint32_t addr);
phy_dev_t *phy_dev_add(phy_type_t phy_type, uint32_t addr, void *priv);
int phy_dev_del(phy_dev_t *phy_dev);

int phy_drivers_set(void);
int phy_drivers_init(void);
int phy_driver_set(phy_drv_t *phy_drv);
int phy_driver_init(phy_type_t phy_type);

char *phy_dev_mii_type_to_str(phy_mii_type_t mii_type);
char *phy_dev_speed_to_str(phy_speed_t speed);
char *phy_dev_duplex_to_str(phy_duplex_t duplex);
char *phy_dev_flowctrl_to_str(int pause_rx, int pause_tx);

void phy_dev_print_status(phy_dev_t *phy_dev);
void phy_dev_prog(phy_dev_t *phy_dev, prog_entry_t *prog_entry);

void phy_dev_link_change_register(phy_dev_t *phy_dev, link_change_cb_t cb, void *ctx);
void phy_dev_link_change_unregister(phy_dev_t *phy_dev);
void phy_dev_link_change_notify(phy_dev_t *phy_dev);
void phy_dev_force_link_reset(phy_dev_t *phy_dev);

typedef void (*phy_dev_work_func_t)(phy_dev_t *phy_dev);
int phy_dev_queue_work(phy_dev_t *phy_dev, phy_dev_work_func_t func);

phy_dev_t *phy_drv_find_device(dt_handle_t handle);
void phy_dev_attach(phy_dev_t *phy_dev, uint32_t phy_mode, int delay_rx, int delay_tx, int instance);

#define is_cascade_phy(phy) (phy->cascade_prev || phy->cascade_next)
#define cascade_phy_get_next(phy)   ((phy->cascade_next && !(phy->cascade_next->flag & PHY_FLAG_NOT_PRESENTED))? phy->cascade_next : NULL)
#define cascade_phy_get_prev(phy)   (phy->cascade_prev? phy->cascade_prev : NULL)

static inline phy_dev_t *cascade_phy_get_first(phy_dev_t *phy_dev)
{
    phy_dev_t *phy;

    for(phy=phy_dev; phy->cascade_prev; phy=phy->cascade_prev);
    return phy;
}

static inline phy_dev_t *cascade_phy_get_last(phy_dev_t *phy_dev)
{
    phy_dev_t *phy;

    if (!phy_dev) return NULL;
    for(phy=phy_dev;
        phy->cascade_next && !(phy->cascade_next->flag & PHY_FLAG_NOT_PRESENTED);
        phy=phy->cascade_next);
    return phy;
}

static inline int phy_bus_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val)
{
    if (!phy_dev->bus_drv)
        return 0;

    return phy_dev->bus_drv->c22_read(phy_dev->addr, reg, val);
}

static inline int phy_bus_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val)
{
    if (!phy_dev->bus_drv)
        return 0;

    return phy_dev->bus_drv->c22_write(phy_dev->addr, reg, val);
}

static inline int phy_bus_c45_read(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t *val)
{
    if (!phy_dev->bus_drv)
        return 0;

    return phy_dev->bus_drv->c45_read(phy_dev->addr, dev, reg, val);
}

static inline int phy_bus_c45_write(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t val)
{
    if (!phy_dev->bus_drv)
        return 0;

    return phy_dev->bus_drv->c45_write(phy_dev->addr, dev, reg, val);
}

static inline int phy_dev_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val)
{
    if (phy_dev->phy_drv->read)
        return phy_dev->phy_drv->read(phy_dev, reg, val);
    else
        return phy_bus_read(phy_dev, reg, val);
}

static inline int phy_dev_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val)
{
    if (phy_dev->phy_drv->write)
        return phy_dev->phy_drv->write(phy_dev, reg, val);
    else
        return phy_bus_write(phy_dev, reg, val);
}

static inline int phy_dev_isolate_phy(phy_dev_t *phy_dev, int isolate)
{
    if (!phy_dev->phy_drv->isolate_phy)
        return 0;

    return phy_dev->phy_drv->isolate_phy(phy_dev, isolate);
}

static inline int phy_dev_super_isolate_phy(phy_dev_t *phy_dev, int isolate)
{
    if (!phy_dev->phy_drv->super_isolate_phy)
        return 0;

    return phy_dev->phy_drv->super_isolate_phy(phy_dev, isolate);
}

static inline int phy_dev_pair_swap_set(phy_dev_t *phy_dev, int enable)
{
    if (!phy_dev->phy_drv->pair_swap_set)
        return 0;

    return phy_dev->phy_drv->pair_swap_set(phy_dev, enable);
}

static inline int phy_dev_set_wol_sleep(phy_dev_t *phy_dev, phy_wol_info *pwi)
{
    phy_dev = cascade_phy_get_last(phy_dev);
    if (!PhyHasWakeOnLan(phy_dev) || !phy_dev->phy_drv->set_wol_sleep) {
        printk("phy_dev %p phy_drv %s addr 0x%x does not support wol-sleep\n",
               phy_dev, phy_dev->phy_drv->name, phy_dev->addr);
        return -1;
    }

    return phy_dev->phy_drv->set_wol_sleep(phy_dev, pwi);
}

static inline int phy_dev_power_get(phy_dev_t *phy_dev, int *enable)
{
    *enable = 0;

    if (!phy_dev->phy_drv->power_get)
        return 0;

    return phy_dev->phy_drv->power_get(phy_dev, enable);
}

static inline int phy_dev_power_set(phy_dev_t *phy_dev, int enable)
{
    if ((PhyIsPortConnectedToExternalSwitch(phy_dev) || PhyIsFixedConnection(phy_dev)) && !enable)
        return 0;

    if (is_cascade_phy(phy_dev))
    {
        phy_dev_t *cascade;
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
        {
            //printk("phy %s:%d power_set=%d\n", (phy_dev->phy_drv) ? cascade->phy_drv->name : NULL, cascade->addr, enable);
            if (enable)
                cascade->flag |= PHY_FLAG_POWER_SET_ENABLED;
            else
                cascade->flag &= ~PHY_FLAG_POWER_SET_ENABLED;

            if (cascade->phy_drv->power_set)
                cascade->phy_drv->power_set(cascade, enable);
        }
        return 0;
    }

    if (enable)
        phy_dev->flag |= PHY_FLAG_POWER_SET_ENABLED;
    else
        phy_dev->flag &= ~PHY_FLAG_POWER_SET_ENABLED;

    if (!phy_dev->phy_drv->power_set)
        return 0;

    return phy_dev->phy_drv->power_set(phy_dev, enable);
}

static inline int phy_dev_silent_start_get(phy_dev_t *phy_dev, int *enable)
{
    *enable = 0;

    if (!phy_dev->phy_drv->silent_start_get)
        return -1;

    return phy_dev->phy_drv->silent_start_get(phy_dev, enable);
}

static inline int phy_dev_silent_start_set(phy_dev_t *phy_dev, int enable)
{
    if (!phy_dev->phy_drv->silent_start_get)
        return -1;

    return phy_dev->phy_drv->silent_start_set(phy_dev, enable);
}

static inline int phy_dev_apd_get(phy_dev_t *phy_dev, int *enable)
{
    *enable = 0;

    if (!phy_dev->phy_drv->apd_get)
        return 0;

    return phy_dev->phy_drv->apd_get(phy_dev, enable);
}

static inline int cascade_phy_dev_apd_get(phy_dev_t *phy_dev, int *enable)
{
    int _enable;
    int rc = 0;
    phy_dev_t *cascade;

    if (is_cascade_phy(phy_dev))
    {
        for (*enable = 0, cascade = cascade_phy_get_first(phy_dev);
            cascade; cascade = cascade_phy_get_next(cascade))
        {
            if (cascade->flag & PHY_FLAG_NOT_PRESENTED)
                break;
            rc |= phy_dev_apd_get(cascade, &_enable);
            *enable |= _enable;
        }
        return rc;
    }
    return phy_dev_apd_get(phy_dev, enable);
}

static inline int phy_dev_apd_set(phy_dev_t *phy_dev, int enable)
{
    if ((PhyIsPortConnectedToExternalSwitch(phy_dev) || PhyIsFixedConnection(phy_dev)))
        return 0;

    if (!phy_dev->phy_drv->apd_set)
        return 0;

    return phy_dev->phy_drv->apd_set(phy_dev, enable);
}

static inline int cascade_phy_dev_apd_set(phy_dev_t *phy_dev, int enable)
{
    int rc = 0;
    phy_dev_t *cascade;

    if (is_cascade_phy(phy_dev))
    {
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
        {
            if (cascade->flag & PHY_FLAG_NOT_PRESENTED)
                break;
            rc |= phy_dev_apd_set(cascade, enable);
        }
        return rc;
    }
    return phy_dev_apd_set(phy_dev, enable);
}

static inline int phy_dev_cable_diag_run(phy_dev_t *phy_dev, int *result, int *pair_len)
{
    if (!phy_dev->phy_drv->cable_diag_run)
        return -1;
    return phy_dev->phy_drv->cable_diag_run(phy_dev, result, pair_len);
}

static inline int phy_dev_cable_diag_is_supported(phy_dev_t *phy_dev)
{
    return phy_dev->phy_drv->cable_diag_run != NULL;
}

static inline int phy_dev_cable_diag_is_enabled(phy_dev_t *phy_dev)
{
    return (phy_dev->flag & PHY_FLAG_CABLE_DIAG_ENABLED) > 0;
}

static inline int phy_dev_cable_diag_set(phy_dev_t *phy_dev, int enable)
{
    if (!phy_dev->phy_drv->cable_diag_run)
        return -1;

    phy_dev->flag &= ~PHY_FLAG_CABLE_DIAG_ENABLED;
    phy_dev->flag |= enable? PHY_FLAG_CABLE_DIAG_ENABLED: 0;

    if (!phy_dev->phy_drv->cable_diag_set)
        return 0;

    return phy_dev->phy_drv->cable_diag_set(phy_dev, enable);
}

static inline int phy_dev_cable_diag_get(phy_dev_t *phy_dev, int *enable)
{
    if (!phy_dev->phy_drv->cable_diag_run)
        return -1;

    *enable = (phy_dev->flag & PHY_FLAG_CABLE_DIAG_ENABLED) > 0;
    return 0;
}

static inline int phy_dev_eee_get(phy_dev_t *phy_dev, int *enable)
{
    *enable = 0;

    if (!phy_dev->phy_drv->eee_get)
        return 0;

    return phy_dev->phy_drv->eee_get(phy_dev, enable);
}

static inline int phy_dev_eee_set(phy_dev_t *phy_dev, int enable)
{
    if (!phy_dev->phy_drv->eee_set)
        return 0;

    return phy_dev->phy_drv->eee_set(phy_dev, enable);
}

static inline int phy_dev_eee_resolution_get(phy_dev_t *phy_dev, int *enable)
{
    *enable = 0;

    if (!phy_dev->phy_drv->eee_resolution_get)
        return 0;

    return phy_dev->phy_drv->eee_resolution_get(phy_dev, enable);
}

static inline int phy_dev_read_status(phy_dev_t *phy_dev)
{
    int ret = 0;
#if !defined(DSL_DEVICES)
    phy_speed_t speed = phy_dev->speed;
#endif

    if (!(phy_dev->flag & PHY_FLAG_INITED))
        goto Exit;

    if (!phy_dev->phy_drv->read_status)
        goto Exit;

    if ((ret = phy_dev->phy_drv->read_status(phy_dev)))
        goto Exit;

#if !defined(DSL_DEVICES) /* DSL product has revert chain direction due to dynamic module support */
    if (phy_dev->speed == speed)
        goto Exit;

    if (!phy_dev->cascade_prev)
        goto Exit;

    if (!phy_dev->cascade_prev->phy_drv->speed_set)
        goto Exit;

    if ((ret = phy_dev->cascade_prev->phy_drv->speed_set(phy_dev->cascade_prev, phy_dev->speed, phy_dev->duplex)))
        goto Exit;
#endif

#if defined(DSL_DEVICES)
    // administratively force link down if ethernet phy is not in power enable state
    if (phy_dev->phy_drv->phy_type != PHY_TYPE_PON &&
        !(phy_dev->flag & PHY_FLAG_POWER_SET_ENABLED) &&
        !(phy_dev->flag & PHY_FLAG_TO_EXTSW)) {
        phy_dev->link = 0;
    }
#endif
Exit:
    return ret;
}

static inline int phy_dev_config_speed_get(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex)
{
    if (!phy_dev->phy_drv->config_speed_get)
        return 0;

    return phy_dev->phy_drv->config_speed_get(phy_dev, speed, duplex);
}

static inline int phy_dev_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    if (!phy_dev->phy_drv->speed_set)
        return 0;

    return phy_dev->phy_drv->speed_set(phy_dev, speed, duplex);
}

static inline int phy_dev_auto_mdix_set(phy_dev_t *phy_dev, int enable)
{
    if (!phy_dev->phy_drv->auto_mdix_set)
        return -1;

    return phy_dev->phy_drv->auto_mdix_set(phy_dev, enable);
}

static inline int phy_dev_auto_mdix_get(phy_dev_t *phy_dev, int *enable)
{
    if (!phy_dev->phy_drv->auto_mdix_get)
        return -1;

    return phy_dev->phy_drv->auto_mdix_get(phy_dev, enable);
}

static inline int phy_dev_wirespeed_set(phy_dev_t *phy_dev, int enable)
{
    if (!phy_dev->phy_drv->wirespeed_set)
        return -1;

    return phy_dev->phy_drv->wirespeed_set(phy_dev, enable);
}

static inline int phy_dev_wirespeed_get(phy_dev_t *phy_dev, int *enable)
{
    if (!phy_dev->phy_drv->wirespeed_get)
        return -1;

    return phy_dev->phy_drv->wirespeed_get(phy_dev, enable);
}


static inline void phy_dev_status_propagate(phy_dev_t *end_phy)
{
    phy_dev_t *phy_dev;

    for (phy_dev = end_phy->cascade_prev; phy_dev; phy_dev = phy_dev->cascade_prev)
        phy_dev_speed_set(phy_dev, end_phy->speed, end_phy->duplex);
}

/* For propagating status to match first PHY,
    for status only without changing configuration during the link up
    due to the status call-back is done in internal PHY */
static inline void phy_dev_status_reverse_propagate(phy_dev_t *first_phy)
{
    phy_dev_t *phy_dev;

    for (phy_dev = first_phy->cascade_next;
        phy_dev && !(phy_dev->flag & PHY_FLAG_NOT_PRESENTED);
        phy_dev = phy_dev->cascade_next)
    {
        phy_dev->link = first_phy->link;
        phy_dev->speed = first_phy->speed;
        phy_dev->duplex = first_phy->duplex;
        phy_dev->current_inter_phy_type = first_phy->current_inter_phy_type;
    }
}

static inline int phy_dev_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    if (!phy_dev->phy_drv->caps_set)
        return 0;

    if (phy_dev->caps_mask)
        caps &= phy_dev->caps_mask;

    return phy_dev->phy_drv->caps_set(phy_dev, caps);
}

static inline int phy_dev_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps)
{
    int ret;
    *caps = 0;

    if (!phy_dev->phy_drv->caps_get)
        return 0;

    ret = phy_dev->phy_drv->caps_get(phy_dev, caps_type, caps);

#if !defined(DSL_DEVICES)
    if (caps_type == CAPS_TYPE_SUPPORTED && phy_dev->caps_mask)
        *caps &= phy_dev->caps_mask;
#endif

    return ret;
}

static inline void phy_dev_current_inter_phy_types_set(phy_dev_t *phy_dev, inter_phy_types_dir_t dir, inter_phy_type_t inter_phy_type)
{
    phy_dev->current_inter_phy_type = inter_phy_type;
}

/* Get PHY configured inter types */
static inline int phy_dev_configured_inter_phy_types_get(phy_dev_t *phy_dev, uint32_t *types)
{
    *types = phy_dev->configured_inter_phy_types;
    return 0;
}

/* Get PHY inter types all capabilities */
static inline int phy_dev_inter_phy_types_get(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, uint32_t *types)
{
    int rc;
    *types = INTER_PHY_TYPE_UNKNOWN_M;

    if (phy_dev->inter_phy_types)
    {
        *types = phy_dev->inter_phy_types;
        return 0;
    }

    if (!phy_dev->phy_drv->inter_phy_types_get)
    {
        *types = phy_dev->inter_phy_types = INTER_PHY_TYPE_UNKNOWN_M;
        return 0;
    }

    rc = phy_dev->phy_drv->inter_phy_types_get(phy_dev, if_dir, types);
    phy_dev->inter_phy_types = *types;
    return rc;
}

static inline inter_phy_type_t phy_dev_current_inter_phy_types_get(phy_dev_t *phy_dev)
{
    phy_drv_t *phy_drv = phy_dev->phy_drv;

    if (phy_drv->current_inter_phy_type_get)
        phy_dev->current_inter_phy_type = phy_drv->current_inter_phy_type_get(phy_dev);

    return phy_dev->current_inter_phy_type;
}

/*
    For device without run time scanning multi types capability like most hardware based Serdes interface,
    the driver needs to call phy_get_best_inter_phy_configure_type(int inter_types, phy_speed_t speed)
    to get the unique inter phy type for each speed and configure properly.
    Function phy_get_best_inter_phy_configure_type(int inter_types, phy_speed_t speed) is the protocol to
    guarantee cascade PHYs to communicate through matching inter phy.
*/
static inline int phy_dev_configured_inter_phy_types_set(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, uint32_t types)
{
    int rc = 0;
    phy_drv_t *phy_drv = phy_dev->phy_drv;

    if (phy_drv->configured_inter_phy_types_set)
        rc = phy_drv->configured_inter_phy_types_set(phy_dev, if_dir, types);

    phy_dev->configured_inter_phy_types = types;
    return rc;
}

static inline int phy_dev_is_xgmii_mode(phy_dev_t *phy_dev)
{
    uint32_t types;
    inter_phy_type_t mode;

    phy_dev_inter_phy_types_get(phy_dev, INTER_PHY_TYPE_DOWN, &types);
    /* If the PHY's mode flags is not defined, we simply guess it is using legacy 2500Base-X,
       thus return XGMII flag based on if the speed is above 2.5G */
    if (types == INTER_PHY_TYPE_UNKNOWN_M)
    {
        if (phy_dev->speed > PHY_SPEED_2500)
            return 1;
        return 0;
    }

    mode = (1<<phy_dev_current_inter_phy_types_get(phy_dev));
    if (mode &
            (INTER_PHY_TYPE_100BASE_FX_M | INTER_PHY_TYPE_1000BASE_X_M | INTER_PHY_TYPE_SGMII_M |
             INTER_PHY_TYPE_2500BASE_X_M | INTER_PHY_TYPE_5000BASE_X_M))
        return 0;

    if ((mode & (INTER_PHY_TYPE_USXGMII_M | INTER_PHY_TYPE_USXGMII_MP_M)) &&
        phy_dev->speed <= PHY_SPEED_1000)
        return 0;

    return 1;
}

static inline phy_speed_t phy_caps_to_max_speed(uint32_t caps)
{
    int i;
    static int speed[] = {PHY_CAP_10000, PHY_SPEED_10000, PHY_CAP_5000, PHY_SPEED_5000, PHY_CAP_2500, PHY_SPEED_2500,
        PHY_CAP_1000_FULL, PHY_SPEED_1000, PHY_CAP_1000_HALF, PHY_SPEED_1000,
        PHY_CAP_100_FULL, PHY_SPEED_100, PHY_CAP_100_HALF, PHY_SPEED_100,
        PHY_CAP_10_FULL, PHY_SPEED_10, PHY_CAP_10_HALF, PHY_SPEED_10};

    for (i=0; i<ARRAY_SIZE(speed); i+=2) {
        if (caps & speed[i])
            return speed[i+1];
    }
    return PHY_SPEED_UNKNOWN;
}

static inline phy_speed_t phy_caps_to_min_speed(uint32_t caps)
{
    int i;
    static int speed[] = {PHY_CAP_10000, PHY_SPEED_10000, PHY_CAP_5000, PHY_SPEED_5000, PHY_CAP_2500, PHY_SPEED_2500,
        PHY_CAP_1000_FULL, PHY_SPEED_1000, PHY_CAP_1000_HALF, PHY_SPEED_1000,
        PHY_CAP_100_FULL, PHY_SPEED_100, PHY_CAP_100_HALF, PHY_SPEED_100,
        PHY_CAP_10_FULL, PHY_SPEED_10, PHY_CAP_10_HALF, PHY_SPEED_10};

    for (i = ARRAY_SIZE(speed)-1; i>0; i-=2) {
        if (caps & speed[i-1])
            return speed[i];
    }
    return PHY_SPEED_UNKNOWN;
}

/* Since phy_speed_t values changed from enumeration to actual speed,
    the original speed indexed array will cause huge wasting memory occupation.
    changed to non speed index implementation */
static inline uint32_t phy_speed_to_cap(phy_speed_t speed, phy_duplex_t duplex)
{
    int i;
    static uint32_t caps[] = {
        PHY_SPEED_AUTO, PHY_CAP_AUTONEG,
        PHY_SPEED_10, PHY_CAP_10_FULL,
        PHY_SPEED_100, PHY_CAP_100_FULL,
        PHY_SPEED_1000, PHY_CAP_1000_FULL,
        PHY_SPEED_2500, PHY_CAP_2500,
        PHY_SPEED_5000, PHY_CAP_5000,
        PHY_SPEED_10000, PHY_CAP_10000};
    uint32_t cap;

    for (i=0; i<ARRAY_SIZE(caps); i+=2)
        if (caps[i] == speed)
            break;

    cap = caps[i+1];
    if (speed <= PHY_SPEED_1000 && speed != PHY_SPEED_AUTO && duplex != PHY_DUPLEX_FULL)
        cap >>= 1;
    return cap;
}

static inline int phy_dev_phyid_get(phy_dev_t *phy_dev, uint32_t *phyid)
{
    *phyid = 0;

    if (!phy_dev->phy_drv->phyid_get)
        return 0;

    return phy_dev->phy_drv->phyid_get(phy_dev, phyid);
}

static inline int phy_dev_is_broadcom_phy(phy_dev_t *phy_dev)
{
    uint32_t phy_id;

    phy_dev_phyid_get(phy_dev, &phy_id);
    return ((phy_id & 0xffff0000) == 0xae020000 || (phy_id & 0xffff0000) == 0x35900000);
}

static inline int phy_dev_init(phy_dev_t *first_phy)
{
    int rc = 0;
    phy_dev_t *phy_dev;

    for (phy_dev = cascade_phy_get_first(first_phy); phy_dev; phy_dev = phy_dev->cascade_next)
    {
        phy_dev->link = 0;
        phy_dev->speed = PHY_SPEED_UNKNOWN;
        phy_dev->duplex = PHY_DUPLEX_UNKNOWN;

        if (phy_dev->phy_drv->init != NULL)
            rc |= phy_dev->phy_drv->init(phy_dev);
        phy_dev->flag |= PHY_FLAG_INITED;

        if (phy_dev->phy_drv->phy_type != PHY_TYPE_CROSSBAR)
        {
#if !defined(DSL_DEVICES)
            rc |= phy_dev_apd_set(phy_dev, 1);
#endif
            rc |= phy_dev_eee_set(phy_dev, 1);
        }
    }

    return rc;
}

static inline int phy_dev_leds_init(phy_dev_t *first_phy, void *leds_info)
{
    int rc = 0;
    phy_dev_t *phy_dev;

    for (phy_dev = cascade_phy_get_first(first_phy); phy_dev; phy_dev = phy_dev->cascade_next)
    {
        if (phy_dev->phy_drv->leds_init != NULL)
            rc |= phy_dev->phy_drv->leds_init(phy_dev, leds_info);
    }

    return rc;
}

static inline int cascade_phy_dev_isolate_phy(phy_dev_t *phy_dev, int isolate)
{
    if (is_cascade_phy(phy_dev))
    {
        int rc = 0;
        phy_dev_t *cascade;
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
            rc |= phy_dev_isolate_phy(cascade, isolate);
        return rc;
    }
    return phy_dev_isolate_phy(phy_dev, isolate);
}

static inline int cascade_phy_dev_eee_set(phy_dev_t *phy_dev, int enable)
{
    if (is_cascade_phy(phy_dev))
    {
        int rc = 0;
        phy_dev_t *cascade;
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
            rc |= phy_dev_eee_set(cascade, enable);
        return rc;
    }
    return phy_dev_eee_set(phy_dev, enable);
}

static inline int cascade_phy_dev_eee_get(phy_dev_t *phy_dev, int *enable)
{
    int val;
    if (is_cascade_phy(phy_dev))
    {
        int rc = 0;
        phy_dev_t *cascade;
        *enable = 0;
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
        {
            rc |= phy_dev_eee_get(cascade, &val);
            *enable |= val;
        }
        return rc;
    }
    return phy_dev_eee_get(phy_dev, enable);
}

static inline int cascade_phy_dev_eee_resolution_get(phy_dev_t *phy_dev, int *enable)
{
    int val;
    if (is_cascade_phy(phy_dev))
    {
        int rc = 0;
        phy_dev_t *cascade;
        *enable = 0;
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
        {
            rc |= phy_dev_eee_resolution_get(cascade, &val);
            *enable |= val;
        }
        return rc;
    }
    return phy_dev_eee_resolution_get(phy_dev, enable);
}

static inline int cascade_phy_dev_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    if (is_cascade_phy(phy_dev))
    {
        int rc = 0;
        phy_dev_t *cascade;

        /* Internal PHY status will follow the external PHY link, so configure end PHY first */
        for (cascade = cascade_phy_get_last(phy_dev); cascade; cascade = cascade_phy_get_prev(cascade))
            rc |= phy_dev_speed_set(cascade, speed, duplex);
        return rc;
    }
    return phy_dev_speed_set(phy_dev, speed, duplex);
}

/*
    Input: Common Inter PHY Types
            Link Speed
    Output: Best Inter PHY Configuration Type
*/
static inline inter_phy_type_t phy_get_best_inter_phy_configure_type(phy_dev_t *phy_dev, int inter_types, phy_speed_t speed)
{
    /*
        We will treat native IEEE standard in highest priority in each speed;
        Next we will newer IEEE derived protocol in higher priority;
     */

    /* USXGMII/Muti Speed AN is in highest priority for inter PHY connection */
    if (inter_types & INTER_PHY_TYPE_USXGMII_MP_M)
        return INTER_PHY_TYPE_USXGMII_MP;

    if (inter_types & INTER_PHY_TYPE_USXGMII_M)
        return INTER_PHY_TYPE_USXGMII;

    switch(speed)
    {
        /* Check 10G speed, 10G has nothing to check but only 10GBase-R */
        case PHY_SPEED_AUTO:
            if (inter_types & INTER_PHY_TYPE_SGMII_M)
                return INTER_PHY_TYPE_SGMII;
            break;
        case PHY_SPEED_10000:
            if (inter_types & INTER_PHY_TYPE_10GBASE_R_M)
                return INTER_PHY_TYPE_10GBASE_R;
            break;
        case PHY_SPEED_5000:
            /* Check 5G speed */
            if (inter_types & INTER_PHY_TYPE_5GBASE_R_M)
                return INTER_PHY_TYPE_5GBASE_R;
            else if (inter_types & INTER_PHY_TYPE_5GBASE_X_M)
                return INTER_PHY_TYPE_5GBASE_X;
            else if (inter_types & INTER_PHY_TYPE_5000BASE_X_M)
                return INTER_PHY_TYPE_5000BASE_X;
            else if (inter_types & INTER_PHY_TYPE_5GIDLE_M)
                return INTER_PHY_TYPE_5GIDLE;
            break;
        case PHY_SPEED_2500:
            /* Check 2.5G speed */
            if (inter_types & INTER_PHY_TYPE_2P5GBASE_X_M)
                return INTER_PHY_TYPE_2P5GBASE_X;
            else if (inter_types & INTER_PHY_TYPE_2P5GBASE_R_M)
                return INTER_PHY_TYPE_2P5GBASE_R;
            else if (inter_types & INTER_PHY_TYPE_2500BASE_X_M)
                return INTER_PHY_TYPE_2500BASE_X;
            else if (inter_types & INTER_PHY_TYPE_2P5GIDLE_M)
                return INTER_PHY_TYPE_2P5GIDLE;
            break;
        case PHY_SPEED_1000:
            /* Check 1G speed */
            if (inter_types & INTER_PHY_TYPE_SGMII_M)
                return INTER_PHY_TYPE_SGMII;
            if (inter_types & INTER_PHY_TYPE_1000BASE_X_M)
                return INTER_PHY_TYPE_1000BASE_X;
            else if (inter_types & INTER_PHY_TYPE_1GBASE_R_M)
                return INTER_PHY_TYPE_1GBASE_R;
            else if (inter_types & INTER_PHY_TYPE_1GBASE_X_M)
                return INTER_PHY_TYPE_1GBASE_X;
            break;
        case PHY_SPEED_100:
            /* Check 100M speed */
            if (inter_types & INTER_PHY_TYPE_SGMII_M)
                return INTER_PHY_TYPE_SGMII;
            if (inter_types & INTER_PHY_TYPE_100BASE_FX_M)
                return INTER_PHY_TYPE_100BASE_FX;
            break;
        default:
            break;
    }
    return INTER_PHY_TYPE_UNKNOWN;
}

static inline int cascade_phy_get_common_inter_types(phy_dev_t *phy_dev)
{
    return phy_dev->common_inter_phy_types;
}

/*
    For device without run time scanning multi types capability like most hardware based Serdes interface,
    the driver needs to call phy_get_best_inter_phy_configure_type(int inter_types, phy_speed_t speed)
    to get the unique inter phy type for each speed and configure properly.
    Function phy_get_best_inter_phy_configure_type(int inter_types, phy_speed_t speed) is the protocol to
    guarantee cascade PHYs to communicate through matching inter phy.
*/
static inline int cascade_phy_set_common_inter_types(phy_dev_t *phy_dev)
{
    phy_dev_t *cascade;
    uint32_t inter_phy_types, inter_phy_types_last;

    inter_phy_types = inter_phy_types_last = 0;
    if (is_cascade_phy(phy_dev))
    {
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
        {
            if (!inter_phy_types_last)
            {
                phy_dev_inter_phy_types_get(cascade, INTER_PHY_TYPE_DOWN, &inter_phy_types);
                inter_phy_types_last = inter_phy_types;
            }
            else
            {
                phy_dev_inter_phy_types_get(cascade, INTER_PHY_TYPE_UP, &inter_phy_types);
                inter_phy_types_last &= inter_phy_types;
            }

        }

        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
            cascade->common_inter_phy_types = inter_phy_types_last;
    }
    else
    {
        phy_dev_inter_phy_types_get(phy_dev, INTER_PHY_TYPE_UP, &inter_phy_types);
        phy_dev->common_inter_phy_types = inter_phy_types;
    }
    return 0;
}

// find minimum caps
static inline int cascade_phy_dev_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps)
{
    int rc = 0;
    phy_dev_t *cascade;
    uint32_t cascade_caps;
#if defined(DSL_DEVICES)
    uint32_t inter_phy_types = 0, inter_phy_types_last = 0;
    uint32_t inter_phy_supported_speed_caps;
#endif

    *caps = 0;
    if (is_cascade_phy(phy_dev))
    {
        for (cascade = cascade_phy_get_first(phy_dev), *caps = 0; cascade; cascade = cascade_phy_get_next(cascade))
        {
            if (cascade->phy_drv->caps_get)
            {
                rc |= phy_dev_caps_get(cascade, caps_type, &cascade_caps);
                if (cascade_caps)
                {
                    *caps = (*caps) ? ((*caps & cascade_caps) | (cascade_caps & (PHY_CAP_PAUSE|PHY_CAP_PAUSE_ASYM))) : cascade_caps;

#if defined(DSL_DEVICES)
                    if (!inter_phy_types_last)
                    {
                        rc |= phy_dev_inter_phy_types_get(cascade, INTER_PHY_TYPE_DOWN, &inter_phy_types);
                        inter_phy_types_last = inter_phy_types;
                    }
                    else
                    {
                        rc |= phy_dev_inter_phy_types_get(cascade, INTER_PHY_TYPE_UP, &inter_phy_types);
                        inter_phy_types &= inter_phy_types_last;
                    }

                    rc |= get_inter_phy_supported_speed_caps(inter_phy_types, &inter_phy_supported_speed_caps);
                    *caps = (*caps & inter_phy_supported_speed_caps) | (*caps & PHY_CAP_AUTONEG);
#endif
                }
            }
        }
        return rc;
    }
    return phy_dev_caps_get(phy_dev, caps_type, caps);
}

static inline int cascade_phy_dev_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    if (is_cascade_phy(phy_dev))
    {
        int rc = 0;
        phy_dev_t *cascade;
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
            rc |= phy_dev_caps_set(cascade, caps);
        return rc;
    }
    return phy_dev_caps_set(phy_dev, caps);
}

/* Return if the phy_speed_t is covered by the PHY Speed CAP */
static inline int phy_dev_cap_speed_match(uint32_t caps, phy_speed_t speed)
{
    switch (speed)
    {
        case PHY_SPEED_10000:
            return ((caps & PHY_CAP_10000)>0);
        case PHY_SPEED_5000:
            return ((caps & PHY_CAP_5000)>0);
        case PHY_SPEED_2500:
            return ((caps & PHY_CAP_2500)>0);
        case PHY_SPEED_1000:
            return ((caps & PHY_CAP_1000_FULL)>0);
        case PHY_SPEED_100:
            return ((caps & PHY_CAP_100_FULL)>0);
        case PHY_SPEED_10:
            return ((caps & PHY_CAP_10_FULL)>0);
        default:
            break;
    }
    return 0;
}

static inline phy_speed_t cascade_phy_max_speed_get(phy_dev_t *phy_dev)
{
    uint32_t caps;

    if (cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_SUPPORTED, &caps))
        return 0;

    return phy_caps_to_max_speed(caps);
}

static inline phy_speed_t phy_max_speed_get(phy_dev_t *phy_dev)
{
    uint32_t caps;

    if (phy_dev_caps_get(phy_dev, CAPS_TYPE_SUPPORTED, &caps))
        return 0;

    return phy_caps_to_max_speed(caps);
}

static inline int cascade_phy_dev_power_set(phy_dev_t *phy_dev, int enable)
{
    // current phy_dev_power_set() already handle cascade
    return phy_dev_power_set(phy_dev, enable);
}

/* Get last non dynamic PHY */
static inline phy_dev_t *cascade_phy_get_last_active(phy_dev_t *phy_dev)
{
    phy_dev_t *phy = cascade_phy_get_last(phy_dev);
    if (phy && (phy->flag & PHY_FLAG_DYNAMIC))
        phy = phy->cascade_prev;
    return phy;
}

static inline void _phy_register_polling_timer(phy_dev_t *phy, link_change_cb_t cb, int _register)
{
    phy_dev_t *end_phy = cascade_phy_get_last_active(phy);
    if (_register)
        phy_dev_link_change_register(end_phy, cb, end_phy);
    else
        phy_dev_link_change_unregister(end_phy);
}
#define phy_register_polling_timer(phy, cb) _phy_register_polling_timer(phy, cb, 1)
#define phy_unregister_polling_timer(phy) _phy_register_polling_timer(phy, 0, 0)

static inline int phy_drv_dev_add(phy_dev_t *phy_dev)
{
    if (phy_dev->phy_drv->initialized)
        return 0;

    if (!phy_dev->phy_drv->dev_add)
        return 0;

    return phy_dev->phy_drv->dev_add(phy_dev);
}

static inline int phy_drv_dev_del(phy_dev_t *phy_dev)
{
    if (phy_dev->phy_drv->initialized)
        return 0;

    if (!phy_dev->phy_drv->dev_del)
        return 0;

    return phy_dev->phy_drv->dev_del(phy_dev);
}

static inline int phy_drv_init(phy_drv_t *phy_drv)
{
    if (phy_drv->initialized)
        return 0;

    if (!phy_drv->drv_init)
        return 0;

    return phy_drv->drv_init(phy_drv);
}

static inline int phy_loopback_set(phy_dev_t *phy_dev, int enable, phy_speed_t speed)
{
    phy_drv_t *phy_drv = phy_dev->phy_drv;

    if(!phy_drv->loopback_set)
        return 0;
    return phy_drv->loopback_set(phy_dev, enable, speed);
}

static inline int phy_loopback_get(phy_dev_t *phy_dev, int *enable, phy_speed_t *speed)
{
    phy_drv_t *phy_drv = phy_dev->phy_drv;

    if(!phy_drv->loopback_get)
        return 0;
    return phy_drv->loopback_get(phy_dev, enable, speed);
}

#define phy_speed_2_mbps(speed)         speed
#define phy_mbps_2_speed(speed_mbps)    ((phy_speed_t) speed_mbps)

static inline int phy_dev_macsec_oper(phy_dev_t *phy_dev, void *data)
{
    if (!phy_dev->phy_drv->macsec_oper || !phy_dev->macsec_dev)
        return 0;

    return phy_dev->phy_drv->macsec_oper(phy_dev, data);
}

int phy_priv_fun(phy_dev_t *phy_dev, int op, ...);

/* USXGMII-M related functions */
#define MPHY_VIRTUAL_ADDR           0x100
#define MPHY_VIRTUAL_ADDR_SHIFT     8
static inline int phy_dev_is_mphy(phy_dev_t *phy_dev)
{
    return phy_dev && (phy_dev->usxgmii_m_type || phy_dev->mphy_base);
}

static inline phy_dev_t *mphy_dev_base(phy_dev_t *phy_dev)
{
    if (!phy_dev) return NULL;
    if (phy_dev->usxgmii_m_type) return phy_dev;
    if (phy_dev->mphy_base) return phy_dev->mphy_base;
    return NULL;
}

static inline int is_mphy_dev_base(phy_dev_t *phy_dev)
{
    return (phy_dev->mphy_base == phy_dev);
}

static inline int mphy_dev_instance(phy_dev_t *phy_dev)
{
    int instance;
    if (phy_dev && phy_dev->mphy_base)
    {
        instance = phy_dev->addr - phy_dev->mphy_base->addr;
        return (instance >=  MPHY_VIRTUAL_ADDR) ? instance >> MPHY_VIRTUAL_ADDR_SHIFT : instance;
    }
    return 0;
}

static inline int mphy_dev_true_addr(phy_dev_t *phy_dev)
{
    return (phy_dev->addr & ((1<<MPHY_VIRTUAL_ADDR_SHIFT)-1));
}

static inline phy_dev_t *mphy_dev_get(phy_dev_t *mphy_base, int instance)
{
    phy_dev_t *phy = NULL;
    if (mphy_base && mphy_base->usxgmii_m_type)
    {
        phy = phy_dev_get(mphy_base->phy_drv->phy_type, mphy_base->addr + instance);
        if (!phy)
            phy = phy_dev_get(mphy_base->phy_drv->phy_type, mphy_base->addr + (instance << MPHY_VIRTUAL_ADDR_SHIFT));
    }
    return phy;
}
#endif
