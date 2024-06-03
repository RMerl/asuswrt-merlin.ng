// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved


*/

/*
 *  Created on: Dec 2015
 *      Author: yuval.raviv@broadcom.com
 */

#ifndef __PHY_DRV_H__
#define __PHY_DRV_H__

#include "bus_drv.h"
#include "dt_access.h"
extern int apd_enabled;
extern int eee_enabled;
extern int phy_speed_max;

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
#define PHY_CAP_SYNCE           (1 << 13)
#define PHY_CAP_LAST            (1 << 14)
#define PHY_CAP_ALL             (PHY_CAP_LAST - 1)

#define PHY_CAP_PURE_SPEED_CAPS ((PHY_CAP_AUTONEG<<1) - 1)
#define PHY_CAP_NON_SPEED_CAPS (PHY_CAP_ALL & (~PHY_CAP_PURE_SPEED_CAPS))

typedef enum
{
    PHY_SPEED_UNKNOWN   =     0,
    PHY_SPEED_10        =    10,
    PHY_SPEED_100       =   100,
    PHY_SPEED_1000      =  1000,
    PHY_SPEED_2500      =  2500,
    PHY_SPEED_5000      =  5000,
    PHY_SPEED_10000     = 10000,
    PHY_SPEED_AUTO      = PHY_SPEED_UNKNOWN,
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

typedef enum
{
    PHY_TYPE_UNKNOWN,
    PHY_TYPE_EGPHY,
    PHY_TYPE_SGMII,
    PHY_TYPE_EXT1,
    PHY_TYPE_EXT2,
    PHY_TYPE_EXT3,
    PHY_TYPE_MPTWO,
    PHY_TYPE_SHORTFIN,
    PHY_TYPE_SHASTA,
    PHY_TYPE_PON,
    PHY_TYPE_WAN_AE,
    PHY_TYPE_DSL_GPHY,
    PHY_TYPE_138CLASS_SERDES,
    PHY_TYPE_158CLASS_SERDES,
    PHY_TYPE_146CLASS_SERDES,
    PHY_TYPE_6756CLASS_SERDES,
    PHY_TYPE_I2C_PHY,
    PHY_TYPE_CROSSBAR,
    PHY_TYPE_MAC2MAC,
    PHY_TYPE_G9991,
    PHY_TYPE_MAX,
} phy_type_t;

typedef void (*link_change_cb_t)(void *ctx);

#define INTER_PHY_TYPE_MIN          0
#define INTER_PHY_TYPE_UNKNOWN      INTER_PHY_TYPE_MIN
#define INTER_PHY_TYPE_AUTO         INTER_PHY_TYPE_UNKNOWN
#define INTER_PHY_TYPE_100BASE_FX   1
#define INTER_PHY_TYPE_1000BASE_X   2
#define INTER_PHY_TYPE_1GBASE_X     3

#define INTER_PHY_TYPE_1GBASE_R     4
#define INTER_PHY_TYPE_2P5GBASE_X   5
#define INTER_PHY_TYPE_2P5GBASE_R   6
#define INTER_PHY_TYPE_2500BASE_X   7

#define INTER_PHY_TYPE_2P5GIDLE     8
#define INTER_PHY_TYPE_5GBASE_R     9
#define INTER_PHY_TYPE_5GBASE_X     10
#define INTER_PHY_TYPE_5000BASE_X   11

#define INTER_PHY_TYPE_5GIDLE       12
#define INTER_PHY_TYPE_10GBASE_R    13
#define INTER_PHY_TYPE_10GBASE_X    14
#define INTER_PHY_TYPE_SGMII        15
#define INTER_PHY_TYPE_USXGMII      16

#define INTER_PHY_TYPE_USXGMII_MP   17
#define INTER_PHY_TYPE_MLTI_SPEED_BASE_X_AN 18

#define INTER_PHY_TYPE_MAX 19

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

#define PHY_CFG_AN_AUTO 0
#define PHY_CFG_AN_OFF  1
#define PHY_CFG_AN_ON   2

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
    int mac_link;
    phy_speed_t speed;
    phy_duplex_t duplex;
    int caps_mask;
    int pause_rx;
    int pause_tx;
    int delay_rx;
    int delay_tx;
    int eee;
    int autogreeen; /* Autogreeen configuration flag now, use eee_mode_get() to get hardware status */
    int swap_pair;
    int xfi_tx_polarity_inverse;
    int xfi_rx_polarity_inverse;
    int flag;
    int loopback_save;
    void *macsec_dev;
    void *macsec_ops;
    /* For cascaded PHY */
    void *sw_port;
    struct phy_dev_s *cascade_next;
    struct phy_dev_s *cascade_prev;
    dt_handle_t dt_handle;
    dt_gpio_desc gpiod_phy_power;
    dt_gpio_desc gpiod_phy_reset;
    dt_gpio_desc gpiod_tx_disable;
    int core_index;  /* For PHY structure it matters in the whole chip */
    int lane_index;  /* For PHY structure it matters in the whole chip */
    usxgmii_m_type_t usxgmii_m_type;
    int usxgmii_m_index;    /* USXGMII_M port index inside A USXGMII_M port group */
    struct phy_dev_s *mphy_master;  /* For some USXGMII_M devices like Serdes needed master */
    int inter_phy_types;        /* Device total capable inter phy types */
    int common_inter_phy_types; /* Common inter phy types with cascaded PHY */
    int configured_inter_phy_types;    /* configured inter types for different speeds */
    int current_inter_phy_type;     /* current inter phy type for fixed speed configuration or link up */
    int configured_current_inter_phy_type;     /* Manually configured */
    int an_enabled;     /* AN status of specific speed for XFI mode */
    int configured_an_enable;     /* Manually configured AN enabled in XFI mode */
    void *descriptor; /* For some PHY familiy workaround */
#define PHY_NAME_LEN 32
    char name[PHY_NAME_LEN];
    int shared_ref_clk_mhz;
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
#define PHY_FLAG_FORCE_2P5G_10GVCO              (1<<16)
#define PHY_FLAG_FORCE_2P5G_XGMII               (1<<17)
#define PHY_FLAG_SHARED_REF_CLK_SET             (1<<18)

#define PhyIsPortConnectedToExternalSwitch(phy) (((phy)->flag & PHY_FLAG_TO_EXTSW)?1:0)
#define PhyIsExtPhyId(phy)                      (((phy)->flag & PHY_FLAG_EXTPHY)?1:0)
#define PhyIsFixedConnection(phy)               (((phy)->flag & PHY_FLAG_FIXED_CONN)?1:0)
#define PhyHasWakeOnLan(phy)                    (((phy)->flag & PHY_FLAG_WAKE_ON_LAN)?1:0)
#define PhyIsForced2p5g10GVco(phy)              (((phy)->flag & PHY_FLAG_FORCE_2P5G_10GVCO)?1:0)
#define PhyIsSharedRefClkSet(phy)               (((phy)->flag & PHY_FLAG_SHARED_REF_CLK_SET)?1:0)
#define PhySetSharedRefClk(phy)                 ((phy)->flag |= PHY_FLAG_SHARED_REF_CLK_SET)

#define PHY_MAC_LINK_VALID  (1<<31)

#define CAPS_TYPE_ADVERTISE      0
#define CAPS_TYPE_SUPPORTED      1
#define CAPS_TYPE_LP_ADVERTISED  2

static uint32_t inter_phy_supported_speed_caps[] = {
    [INTER_PHY_TYPE_UNKNOWN] = PHY_CAP_100_FULL | PHY_CAP_1000_FULL | PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000,
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
    [INTER_PHY_TYPE_10GBASE_X]  = PHY_CAP_10000,
    [INTER_PHY_TYPE_SGMII]      = PHY_CAP_100_FULL | PHY_CAP_1000_FULL,
    [INTER_PHY_TYPE_USXGMII]    = PHY_CAP_100_FULL | PHY_CAP_1000_FULL | PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000,

    [INTER_PHY_TYPE_USXGMII_MP] = PHY_CAP_100_FULL | PHY_CAP_1000_FULL | PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000,
    [INTER_PHY_TYPE_MLTI_SPEED_BASE_X_AN] = PHY_CAP_100_FULL | PHY_CAP_1000_FULL | PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000,
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
#define INTER_PHY_TYPE_10GBASE_X_M      (1<<INTER_PHY_TYPE_10GBASE_X)
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
#define INTER_PHY_TYPE_UNKNOWN_M ((1<<(INTER_PHY_TYPE_MAX ))-1)
#define INTER_PHY_TYPE_FULLTYPES_M ((1<<(INTER_PHY_TYPE_MAX + 1))-1)

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

    if (inter_phy_types == INTER_PHY_TYPE_UNKNOWN)
        *supported_speed_caps = inter_phy_supported_speed_caps[INTER_PHY_TYPE_UNKNOWN];
    else
    {
        for (i=0; i<INTER_PHY_TYPE_MAX; i++)
        {
            if (!(inter_phy_types & (1<<i)))
                continue;

            *supported_speed_caps |= inter_phy_supported_speed_caps[i];
        }
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
    int (*c45_read)(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t *val);
    int (*c45_write)(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t val);
    int (*c45_read_mask)(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t mask, uint16_t shift, uint16_t *val);
    int (*c45_write_mask)(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t mask, uint16_t shift, uint16_t val);
    int (*power_get)(phy_dev_t *phy_dev, int *enable);
    int (*power_set)(phy_dev_t *phy_dev, int enable);
    int (*apd_get)(phy_dev_t *phy_dev, int *enable);
    int (*apd_set)(phy_dev_t *phy_dev, int enable);
    int (*eee_get)(phy_dev_t *phy_dev, int *enable);
    int (*eee_set)(phy_dev_t *phy_dev, int enable);
    int (*eee_mode_get)(phy_dev_t *phy_dev, int *autogreeen);
    int (*eee_mode_set)(phy_dev_t *phy_dev, int autogreeen);
    int (*eee_resolution_get)(phy_dev_t *phy_dev, int *enable);
    int (*read_status)(phy_dev_t *phy_dev);
    int (*speed_set)(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
    int (*config_speed_get)(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex);
    int (*caps_get)(phy_dev_t *phy_dev, int caps_type, uint32_t *caps);
    int (*caps_set)(phy_dev_t *phy_dev, uint32_t caps);
    int (*inter_phy_types_get)(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, uint32_t *types);
    int (*current_inter_phy_type_get)(phy_dev_t *phy_dev);
    int (*configured_inter_phy_types_set)(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, uint32_t types);
    int (*configured_inter_phy_speed_type_set)(phy_dev_t *phy_dev, int adv_phy_caps, phy_duplex_t duplex,
                inter_phy_types_dir_t if_dir, int type, int cfg_an_enable);
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
    int (*loopback_get)(phy_dev_t *phy_dev, int *enable, phy_speed_t *speed);
    int (*loopback_set)(phy_dev_t *phy_dev, int enable, phy_speed_t speed);
    int (*pair_swap_set)(phy_dev_t *phy_dev, int enable);
    int (*cable_diag_run)(phy_dev_t *phy_dev, int *result, int *pair_len);
    int (*cable_diag_set)(phy_dev_t *phy_dev, int enable);
    int (*cable_diag_get)(phy_dev_t *phy_dev, int *enable);
    int (*auto_mdix_set)(phy_dev_t *phy_dev, int enable);
    int (*auto_mdix_get)(phy_dev_t *phy_dev, int *enable);
    int (*wirespeed_set)(phy_dev_t *phy_dev, int enable);
    int (*wirespeed_get)(phy_dev_t *phy_dev, int *enable);
    int (*diag)(phy_dev_t *phy_dev, int level);
    int (*tx_cfg_get)(phy_dev_t *phy_dev, int8_t *pre, int8_t *main, int8_t *post1, int8_t *post2, int8_t *hpf);
    int (*tx_cfg_set)(phy_dev_t *phy_dev, int8_t pre, int8_t main, int8_t post1, int8_t post2, int8_t hpf);
    int (*macsec_oper)(phy_dev_t *phy_dev, void *data);
    int (*dt_priv)(dt_handle_t handle, uint32_t addr, uint32_t phy_mode, void **_priv);
    int (*leds_init)(phy_dev_t *phy_dev, void *leds_info);
    char *(*get_phy_name)(phy_dev_t *phy_dev);
    int (*silent_start_get)(phy_dev_t *phy_dev, int *enable);
    int (*silent_start_set)(phy_dev_t *phy_dev, int enable);
    int (*priv_fun)(phy_dev_t *phy_dev, int op_code, va_list ap);    /* private function for each class */
    int (*an_restart)(phy_dev_t *phy_dev);
    int (*xfi_tx_polarity_inverse_set)(phy_dev_t *phy_dev, int inverse);
    int (*xfi_rx_polarity_inverse_set)(phy_dev_t *phy_dev, int inverse);
    int (*shared_clock_set)(phy_dev_t *phy_dev);
} phy_drv_t;

static inline int phy_dev_shared_clock_set(phy_dev_t *phy_dev)
{
    phy_drv_t *phy_drv = phy_dev->phy_drv;
    
    if (!phy_drv->shared_clock_set)
        return 0;

    return phy_drv->shared_clock_set(phy_dev);
}

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

typedef enum
{
    PROG_TYPE_ENTRY,
    PROG_TYPE_SEQUENCE,
    PROG_TYPE_UDELAY,
} prog_entry_type_t;

typedef struct
{
    char *desc;
    uint16_t reg;
    uint16_t val;
} prog_entry_t;

typedef struct
{
    char *desc;
    uint16_t dev;
    uint16_t reg;
    uint16_t mask;
    uint16_t val;
    prog_entry_type_t type;
    uintptr_t param;
} prog_entry_ext_t;

#define FIRST_PHY_OF_TYPE   0xffff
phy_dev_t *phy_dev_get(phy_type_t phy_type, uint32_t addr);
phy_dev_t *phy_dev_get_force(phy_type_t phy_type, uint32_t addr);
phy_dev_t *phy_dev_add(phy_type_t phy_type, uint32_t addr, void *priv);
int phy_dev_del(phy_dev_t *phy_dev);

int phy_drivers_set(void);
int phy_drivers_init(void);
int phy_driver_set(phy_drv_t *phy_drv);
int phy_driver_init(phy_type_t phy_type);

char *phy_dev_mii_type_to_str(phy_mii_type_t mii_type);
char *phy_dev_speed_to_str(phy_speed_t speed);
char *phy_dev_speed_to_short_str(phy_speed_t speed);
char *phy_dev_duplex_to_str(phy_duplex_t duplex);
char *phy_dev_flowctrl_to_str(int pause_rx, int pause_tx);

void phy_dev_print_status(phy_dev_t *phy_dev);
int phy_dev_prog(phy_dev_t *phy_dev, prog_entry_t *prog_entry);
int phy_dev_prog_ext(phy_dev_t *phy_dev, prog_entry_ext_t *prog_entry);

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

static inline int phy_bus_c45_comp_read(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t *val, int regIn[], int regOut[])
{
    if (!phy_dev->bus_drv)
        return 0;

    return phy_dev->bus_drv->c45_comp_read(phy_dev->addr, dev, reg, val, regIn, regOut);
}

static inline int phy_bus_c45_comp_write(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t val, int regIn[], int regOut[])
{
    if (!phy_dev->bus_drv)
        return 0;

    return phy_dev->bus_drv->c45_comp_write(phy_dev->addr, dev, reg, val, regIn, regOut);
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

static inline int phy_dev_c45_read(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t *val)
{
    if (phy_dev->phy_drv->c45_read)
        return phy_dev->phy_drv->c45_read(phy_dev, dev, reg, val);
    else
        return phy_bus_c45_read(phy_dev, dev, reg, val);
}

static inline int phy_dev_c45_write(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t val)
{
    if (phy_dev->phy_drv->c45_write)
        return phy_dev->phy_drv->c45_write(phy_dev, dev, reg, val);
    else
        return phy_bus_c45_write(phy_dev, dev, reg, val);
}

static inline int phy_dev_c45_read_mask(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg,
    uint16_t mask, uint16_t shift, uint16_t *val)
{
    int ret;
    uint16_t tmp_val;

    if (phy_dev->phy_drv->c45_read_mask)
        return phy_dev->phy_drv->c45_read_mask(phy_dev, dev, reg, mask, shift, val);

    ret = phy_dev_c45_read(phy_dev, dev, reg, &tmp_val);
    *val = (tmp_val & mask) >> shift;

    return ret;
}

static inline int phy_dev_c45_write_mask(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg,
    uint16_t mask, uint16_t shift, uint16_t val)
{
    int ret = 0;
    uint16_t old_val, new_val;

    if (phy_dev->phy_drv->c45_write_mask)
        return phy_dev->phy_drv->c45_write_mask(phy_dev, dev, reg, mask, shift, val);

    ret |= phy_dev_c45_read(phy_dev, dev, reg, &old_val);
    new_val = (old_val & ~mask) | ((val << shift) & mask);
    ret |= phy_dev_c45_write(phy_dev, dev, reg, new_val);

    return ret;
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

static inline int phy_dev_xfi_tx_polarity_set(phy_dev_t *phy_dev, int inverse)
{
    if (!phy_dev->phy_drv->xfi_tx_polarity_inverse_set)
        return 0;

    return phy_dev->phy_drv->xfi_tx_polarity_inverse_set(phy_dev, inverse);
}

static inline int phy_dev_xfi_rx_polarity_set(phy_dev_t *phy_dev, int inverse)
{
    if (!phy_dev->phy_drv->xfi_rx_polarity_inverse_set)
        return 0;

    return phy_dev->phy_drv->xfi_rx_polarity_inverse_set(phy_dev, inverse);
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

    if (!apd_enabled)
        enable = 0;

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

    if (!eee_enabled || !phy_dev->eee)
        enable = 0;

    return phy_dev->phy_drv->eee_set(phy_dev, enable);
}

static inline int phy_dev_eee_mode_get(phy_dev_t *phy_dev, int *autogreeen)
{
    phy_drv_t *phy_drv = phy_dev->phy_drv;

    if (!phy_drv->eee_mode_set)    /* Check set routine */
        return -1;

    if (!phy_dev->eee)
        return -1;

    if (phy_drv->eee_mode_get)
        return phy_drv->eee_mode_get(phy_dev, autogreeen);
    else
        *autogreeen = phy_dev->autogreeen;
    return 0;
}

static inline int phy_dev_eee_mode_set(phy_dev_t *phy_dev, int autogreeen)
{
    int enable;
    int ret = 0;

    if (!phy_dev->phy_drv->eee_mode_set)
    {
        if (autogreeen)
            return -1;
        return 0;
    }

    if (!phy_dev->eee)
        return 0;

    phy_dev->autogreeen = autogreeen;
    phy_dev_eee_get(phy_dev, &enable);
    if (enable)
        ret = phy_dev_eee_set(phy_dev, enable);


    return ret;
}

static inline int phy_dev_eee_resolution_get(phy_dev_t *phy_dev, int *enable)
{
    *enable = 0;

    if (!phy_dev->phy_drv->eee_resolution_get)
        return 0;

    return phy_dev->phy_drv->eee_resolution_get(phy_dev, enable);
}

static inline int phy_dev_enable_read_status(phy_dev_t *phy_dev, int enable)
{
    if (enable)
        phy_dev->flag &= ~PHY_FLAG_SKIP_READ_STATUS;
    else
        phy_dev->flag |= PHY_FLAG_SKIP_READ_STATUS;

    return 0;
}

static inline int phy_dev_read_status(phy_dev_t *phy_dev)
{
    int ret = 0;
#if !defined(DSL_DEVICES)
    phy_speed_t speed = phy_dev->speed;
#endif

    if (!(phy_dev->flag & PHY_FLAG_INITED))
        goto Exit;

    if (phy_dev->flag & PHY_FLAG_SKIP_READ_STATUS)
        goto Exit;

    if (!phy_dev->phy_drv->read_status)
        goto Exit;

    if ((ret = phy_dev->phy_drv->read_status(phy_dev)))
        goto Exit;

    if (phy_dev->flag & PHY_FLAG_CONF_PAUSE_VALID)
    {
        phy_dev->pause_rx &= (phy_dev->flag & PHY_FLAG_CONF_PAUSE_RX) ? 1 : 0;
        phy_dev->pause_tx &= (phy_dev->flag & PHY_FLAG_CONF_PAUSE_TX) ? 1 : 0;
    }

#if !defined(DSL_DEVICES) /* DSL product has revert chain direction due to dynamic module support */
    if (!phy_dev->cascade_prev)
        goto Exit;

    if (phy_dev->speed != speed && phy_dev->cascade_prev->phy_drv->speed_set)
        phy_dev->cascade_prev->phy_drv->speed_set(phy_dev->cascade_prev, phy_dev->speed, phy_dev->duplex);

    if (phy_dev->cascade_prev->phy_drv->read_status)
        phy_dev->cascade_prev->phy_drv->read_status(phy_dev->cascade_prev);
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

    if (phy_speed_max > 0 && speed > phy_speed_max)
        speed = phy_speed_max;

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

static inline int phy_dev_diag(phy_dev_t *phy_dev, int level)
{
    if (!phy_dev->phy_drv->diag)
        return -1;

    return phy_dev->phy_drv->diag(phy_dev, level);
}

static inline int phy_dev_tx_cfg_get(phy_dev_t *phy_dev, int8_t *pre, int8_t *main,
    int8_t *post1, int8_t *post2, int8_t *hpf)
{
    if (!phy_dev->phy_drv->tx_cfg_get)
        return -1;

    return phy_dev->phy_drv->tx_cfg_get(phy_dev, pre, main, post1, post2, hpf);
}

static inline int phy_dev_tx_cfg_set(phy_dev_t *phy_dev, int8_t pre, int8_t main,
    int8_t post1, int8_t post2, int8_t hpf)
{
    if (!phy_dev->phy_drv->tx_cfg_set)
        return -1;

    return phy_dev->phy_drv->tx_cfg_set(phy_dev, pre, main, post1, post2, hpf);
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

static void _phy_dev_caps_speed_max_override(phy_dev_t *phy_dev, uint32_t *caps)
{
    if (phy_dev->caps_mask)
        *caps &= phy_dev->caps_mask;

    if (phy_speed_max > 0)
    {
        if (phy_speed_max < 10000)
            *caps &= ~PHY_CAP_10000;

        if (phy_speed_max < 5000)
            *caps &= ~PHY_CAP_5000;

        if (phy_speed_max < 1000)
            *caps &= ~(PHY_CAP_1000_HALF | PHY_CAP_1000_FULL);

        if (phy_speed_max < 100)
            *caps &= ~(PHY_CAP_100_HALF | PHY_CAP_100_FULL);

        if (phy_speed_max < 10)
            *caps &= ~(PHY_CAP_10_HALF | PHY_CAP_10_FULL);
    }
}

static inline int phy_dev_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    if (!phy_dev->phy_drv->caps_set)
        return 0;

    _phy_dev_caps_speed_max_override(phy_dev, &caps);

    return phy_dev->phy_drv->caps_set(phy_dev, caps);
}

static inline int phy_dev_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps)
{
    int ret;
    *caps = 0;

    if (!phy_dev->phy_drv || !phy_dev->phy_drv->caps_get)
        return 0;

    ret = phy_dev->phy_drv->caps_get(phy_dev, caps_type, caps);

#if !defined(DSL_DEVICES)
    if (caps_type == CAPS_TYPE_SUPPORTED && phy_dev->caps_mask)
        *caps &= phy_dev->caps_mask;
#endif

    return ret;
}

static inline void phy_dev_current_inter_phy_types_set(phy_dev_t *phy_dev, inter_phy_types_dir_t dir, int inter_phy_type)
{
    phy_dev->current_inter_phy_type = inter_phy_type;
}

/* Get PHY configured inter types */
static inline int phy_dev_configured_inter_phy_types_get(phy_dev_t *phy_dev, uint32_t *types)
{
    *types = phy_dev->configured_inter_phy_types;
    return 0;
}

static inline int phy_dev_an_enabled_get(phy_dev_t *phy_dev)
{
    return phy_dev->an_enabled;
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
        /*
            If it is a Copper PHY but Serdes modes undefined, remove Multi Speed (USXGMII, SGMII, MultiSpeed mode)
            but use IEEE modes only since we are doing auto detection in this case.
            If it is SFP module, keep all modes for scanning
        */
        if (!(phy_dev->flag & PHY_FLAG_DYNAMIC) && *types != INTER_PHY_TYPE_UNKNOWN_M)
            *types &= ~INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M;
        phy_dev->inter_phy_types = *types;

        return 0;
    }

    rc = phy_dev->phy_drv->inter_phy_types_get(phy_dev, if_dir, types);
    phy_dev->inter_phy_types = *types;
    return rc;
}

static inline int phy_dev_current_inter_phy_types_get(phy_dev_t *phy_dev)
{
    phy_drv_t *phy_drv = phy_dev->phy_drv;

    if (phy_drv->current_inter_phy_type_get)
        phy_dev->current_inter_phy_type = phy_drv->current_inter_phy_type_get(phy_dev);

    return phy_dev->current_inter_phy_type;
}

static inline int phy_dev_configured_current_inter_phy_type_get(phy_dev_t *phy_dev)
{
    return phy_dev->configured_current_inter_phy_type;
}

static inline void phy_dev_configured_current_inter_phy_type_set(phy_dev_t *phy_dev, int type)
{
    phy_dev->configured_current_inter_phy_type = type;
}

static inline int phy_dev_configured_an_enabled_get(phy_dev_t *phy_dev)
{
    return phy_dev->configured_an_enable;
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

static inline int phy_dev_configured_inter_phy_speed_type_set(phy_dev_t *phy_dev, int adv_phy_caps,
    phy_duplex_t duplex, inter_phy_types_dir_t if_dir, int type,
    int cfg_an)
{
    int rc = 0;
    phy_drv_t *phy_drv = phy_dev->phy_drv;

    if (phy_drv->configured_inter_phy_speed_type_set)
    {
        rc = phy_drv->configured_inter_phy_speed_type_set(phy_dev, adv_phy_caps, duplex,
                if_dir, type, cfg_an);
            phy_dev->configured_current_inter_phy_type = type;
    }
    else
        rc = phy_dev_caps_set(phy_dev, adv_phy_caps);

    return rc;
}

static inline int phy_dev_is_xgmii_mode(phy_dev_t *phy_dev)
{
#if defined(DSL_DEVICES)
    uint32_t types;
    int mode;

    phy_dev_inter_phy_types_get(phy_dev, INTER_PHY_TYPE_DOWN, &types);
    /* If the PHY's mode flags is not defined, we simply guess it is using legacy 2500Base-X,
       thus return XGMII flag based on if the speed is above 2.5G */
    if (types == INTER_PHY_TYPE_UNKNOWN_M)
    {
        if ((phy_dev->speed == PHY_SPEED_2500 && (phy_dev->flag & PHY_FLAG_FORCE_2P5G_XGMII)) ||
           (phy_dev->speed > PHY_SPEED_2500))
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
#else
    return phy_dev->speed > PHY_SPEED_1000;
#endif
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

static inline phy_speed_t phy_caps_to_auto_speed(int caps)
{
    phy_speed_t speed = phy_caps_to_max_speed(caps);
    if (speed != phy_caps_to_min_speed(caps))
        speed = PHY_SPEED_AUTO;
    return speed;
}

/* Since phy_speed_t values changed from enumeration to actual speed,
    the original speed indexed array will cause huge wasting memory occupation.
    changed to non speed index implementation */
static inline uint32_t phy_speed_to_cap(phy_speed_t speed, phy_duplex_t duplex)
{
    int i;
    static uint32_t caps[][2] = {
        {PHY_SPEED_AUTO, PHY_CAP_AUTONEG},
        {PHY_SPEED_10, PHY_CAP_10_FULL},
        {PHY_SPEED_100, PHY_CAP_100_FULL},
        {PHY_SPEED_1000, PHY_CAP_1000_FULL},
        {PHY_SPEED_2500, PHY_CAP_2500},
        {PHY_SPEED_5000, PHY_CAP_5000},
        {PHY_SPEED_10000, PHY_CAP_10000}};
    uint32_t cap = PHY_CAP_AUTONEG;

    for (i=0; i<ARRAY_SIZE(caps); i++)
        if (caps[i][0] == speed)
        {
            cap = caps[i][1];
            break;
        }

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

static inline int cascade_phy_dev_eee_mode_set(phy_dev_t *phy_dev, int autogreeen)
{
    if (is_cascade_phy(phy_dev))
    {
        int rc = 0;
        phy_dev_t *cascade;
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
            rc &= phy_dev_eee_mode_set(cascade, autogreeen);
        return rc;
    }
    return phy_dev_eee_mode_set(phy_dev, autogreeen);
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

static inline int cascade_phy_dev_speed_type_set(phy_dev_t *phy_dev, int phy_adv_caps, phy_duplex_t duplex,
        int type, int cfg_an)
{
    if (is_cascade_phy(phy_dev))
    {
        int rc = 0;
        phy_dev_t *cascade;

        /* Internal PHY status will follow the external PHY link, so configure end PHY first */
        for (cascade = cascade_phy_get_last(phy_dev); cascade; cascade = cascade_phy_get_prev(cascade))
        {
            rc |= phy_dev_configured_inter_phy_speed_type_set(cascade, phy_adv_caps, duplex,
                INTER_PHY_TYPE_UP, type, cfg_an);
        }
        return rc;
    }
    return phy_dev_configured_inter_phy_speed_type_set(phy_dev, phy_adv_caps, duplex,
        INTER_PHY_TYPE_DOWN, type, cfg_an);
}

/*
    Input: Common Inter PHY Types
            Link Speed
    Output: Best Inter PHY Configuration Type
*/
static inline int phy_get_best_inter_phy_configure_type(phy_dev_t *phy_dev, int inter_types, phy_speed_t speed)
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
static inline int phy_dev_has_priv_fun(phy_dev_t *phy_dev)
{
    return phy_dev->phy_drv->priv_fun != 0;
}

/* USXGMII-M related functions */
#define MPHY_VIRTUAL_ADDR           0x100
#define MPHY_VIRTUAL_ADDR_SHIFT     8
static inline int phy_dev_is_mphy(phy_dev_t *phy_dev)
{
    return phy_dev && (phy_dev->usxgmii_m_type > USXGMII_M_NONE && phy_dev->usxgmii_m_type < USXGMII_M_MAX);
}

static inline int mphy_dev_true_addr(phy_dev_t *phy_dev)
{
    return (phy_dev->addr & ((1<<MPHY_VIRTUAL_ADDR_SHIFT)-1));
}

static inline phy_dev_t *mphy_dev_get(phy_dev_t *phy_dev, int instance)
{
    phy_type_t phy_type = phy_dev->phy_drv->phy_type;

    if (!phy_dev_is_mphy(phy_dev))
        return NULL;

    if (phy_type == PHY_TYPE_EXT3)
        return phy_dev_get(phy_type, phy_dev->addr - phy_dev->usxgmii_m_index + instance);
    else
        return phy_dev_get(phy_type, phy_dev->addr + (instance << MPHY_VIRTUAL_ADDR_SHIFT));
}

static inline phy_dev_t *mphy_get_master(phy_dev_t *phy_dev)
{
    int i;
    phy_dev_t *mphy_master;

    if (phy_dev->mphy_master)
        return phy_dev->mphy_master;

    if (!phy_dev_is_mphy(phy_dev))
        return phy_dev;

    for (i=0; i<usxgmii_m_total_ports(phy_dev->usxgmii_m_type); i++)
        if ((mphy_master = mphy_dev_get(phy_dev, i)))
            break;
    phy_dev->mphy_master = mphy_master;

    return mphy_master;
}

static inline int mphy_is_master(phy_dev_t *phy_dev)
{
    phy_dev_t *mphy_master = mphy_get_master(phy_dev);
    return phy_dev_is_mphy(phy_dev) && (mphy_master == phy_dev);
}

static inline phy_speed_t phy_type_to_single_speed(int type)
{
    static phy_speed_t inter_phy_to_single_speed[] = {
        [INTER_PHY_TYPE_AUTO]       = PHY_SPEED_AUTO,
        [INTER_PHY_TYPE_100BASE_FX] = PHY_SPEED_100,
        [INTER_PHY_TYPE_1000BASE_X] = PHY_SPEED_1000,
        [INTER_PHY_TYPE_1GBASE_X]   = PHY_SPEED_1000,

        [INTER_PHY_TYPE_1GBASE_R]   = PHY_SPEED_1000,
        [INTER_PHY_TYPE_2P5GBASE_X] = PHY_SPEED_2500,
        [INTER_PHY_TYPE_2P5GBASE_R] = PHY_SPEED_2500,
        [INTER_PHY_TYPE_2500BASE_X] = PHY_SPEED_2500,

        [INTER_PHY_TYPE_2P5GIDLE]   = PHY_SPEED_2500,
        [INTER_PHY_TYPE_5GBASE_R]   = PHY_SPEED_5000,
        [INTER_PHY_TYPE_5GBASE_X]   = PHY_SPEED_5000,
        [INTER_PHY_TYPE_5000BASE_X] = PHY_SPEED_5000,

        [INTER_PHY_TYPE_5GIDLE]     = PHY_SPEED_5000,
        [INTER_PHY_TYPE_10GBASE_R]  = PHY_SPEED_10000,
        [INTER_PHY_TYPE_10GBASE_X]  = PHY_SPEED_10000,
        [INTER_PHY_TYPE_SGMII]      = PHY_SPEED_AUTO,
        [INTER_PHY_TYPE_USXGMII]    = PHY_SPEED_AUTO,

        [INTER_PHY_TYPE_USXGMII_MP] = PHY_SPEED_AUTO,
        [INTER_PHY_TYPE_MLTI_SPEED_BASE_X_AN] = PHY_SPEED_AUTO,
    };

    if (type >= ARRAY_SIZE(inter_phy_to_single_speed))
        return PHY_SPEED_UNKNOWN;
    return inter_phy_to_single_speed[type];
}

#endif
