// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

*/

/*
 *  Created on: Dec 2016
 *      Author: steven.hsieh@broadcom.com
 */

/*
 * Phy drivers for 63138, 63148, 4908
 */

#include "phy_drv_dsl_phy.h"
#include "8486x_map_part.h"

typedef struct phy_cl45_s {
    phy_dsl_priv_m;
    void *descriptor;
    int forced_100m_fd;
    int (*read_status)(phy_dev_t *phy_dev);
    int support_speed_caps; /* The hardware does not tell us correct thing, so need to guess and save here */
} phy_cl45_t;

#define PHY_SPEED_HIGHEST_SUPPORT PHY_SPEED_10000
#define SET_1GFD(p, v) do { \
    uint16 v16; \
    phy_bus_c45_read32(p, CL45_REG_1G_CTL, &v16); \
    v16 &= ~CL45_REG_1G_CTL_1G_ADV_MASK; \
    if (v>0) v16 |= CL45_REG_1G_CTL_1G_FD_ADV; \
    phy_bus_c45_write32(p, CL45_REG_1G_CTL, v16);} while(0)

#define SET_100MFD(p, v) do { \
    uint16 v16; \
    phy_bus_c45_read32(p, CL45_REG_COP_AN, &v16); \
    v16 &= ~CL45_REG_COP_AN_100M_ADV_MASK; \
    if (v>0) v16 |= CL45_REG_COP_AN_100M_FHD_ADV; /* Added 100MHD to detect forced 100M link partner */ \
    phy_bus_c45_write32(p, CL45_REG_COP_AN, v16);} while(0)

#define SET_2P5G(p, v) do { \
    uint16 v16; \
    if (IsBrcm2P5GPhy(p)) \
    { \
        phy_bus_c45_read32(p, CL45_REG_MGB_AN_CTL, &v16); \
        v16 = v>0? v16|CL45_REG_MGB_AN_2P5G_ADV|CL45_REG_MGB_ENABLE: \
                v16&~(CL45_REG_MGB_AN_2P5G_ADV|CL45_REG_MGB_ENABLE); \
        phy_bus_c45_write32(p, CL45_REG_MGB_AN_CTL, v16); \
    } \
    else \
    { \
        phy_bus_c45_read32(p, CL45REG_10GBASET_AN_DEF_CTL, &v16); \
        v16 = v>0? v16|CL45_10GAN_2P5G_ABL: v16&~CL45_10GAN_2P5G_ABL; \
        phy_bus_c45_write32(p, CL45REG_10GBASET_AN_DEF_CTL, v16); \
    } } while(0)

#define SET_5G(p, v) do { \
    uint16 v16; \
    phy_bus_c45_read32(p, CL45REG_10GBASET_AN_DEF_CTL, &v16); \
    v16 = v>0? v16|CL45_10GAN_5G_ABL : v16&~CL45_10GAN_5G_ABL; \
    phy_bus_c45_write32(p, CL45REG_10GBASET_AN_DEF_CTL, v16); \
    } while(0)

#define SET_10G(p, v) do { \
    uint16 v16; \
    phy_bus_c45_read32(p, CL45REG_10GBASET_AN_DEF_CTL, &v16); \
    v16 = v>0? v16|CL45_10GAN_10G_ABL : v16&~CL45_10GAN_10G_ABL; \
    phy_bus_c45_write32(p, CL45REG_10GBASET_AN_DEF_CTL, v16); \
    } while(0)

typedef struct phyDesc {
    int devId;
    char *devName;
    int flag;
#define CL45PHY_BRCM2P5G_CAP (1<<0)
#define CL45PHY_BRCM10G_CAP  (1<<1)
} phyDesc_t;

static phyDesc_t phyDesc[] = {
    {0xae025048, "84860", CL45PHY_BRCM2P5G_CAP},
    {0xae025040, "84861", CL45PHY_BRCM10G_CAP},
};

static inline int IsBrcm2P5G10GPhy(phy_dev_t *phy_dev)
{
    phy_cl45_t *phy_cl45 = phy_dev->priv;
    phyDesc_t *phyDesc;

    if (phy_cl45 == 0)
        return 0;

    phyDesc = phy_cl45->descriptor;
    if (phyDesc == 0)
        return 0;

    return phyDesc->flag & (CL45PHY_BRCM2P5G_CAP|CL45PHY_BRCM10G_CAP);
}

static inline int IsBrcm2P5GPhy(phy_dev_t *phy_dev)
{
    return (IsBrcm2P5G10GPhy(phy_dev) & CL45PHY_BRCM2P5G_CAP);
}

static inline int IsBrcm10GPhy(phy_dev_t *phy_dev)
{
    return (IsBrcm2P5G10GPhy(phy_dev) & CL45PHY_BRCM10G_CAP);
}

/* Adjust single port USXGMII_M or USXGMII_S so that single board ID applies */
static void dsl_usxgmii_single_port_adjustment(phy_dev_t *phy_dev)
{
    if (IS_USXGMII_MULTI_PORTS(phy_dev))
        return;

    if (!(phy_dev->inter_phy_types & INTER_PHY_TYPE_USXGMII_MP_M))
        phy_dev->usxgmii_m_type = USXGMII_S;
}

static int dsl_cl45phy_read_status(phy_dev_t *phy_dev);
static int dsl_cl45phy_set_speed(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
static int dsl_cl45phy_get_config_speed(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex);
static int dsl_cl45phy_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps);
static int dsl_cl45phy_caps_set(phy_dev_t *phy_dev, uint32_t caps);
static int dsl_cl45phy_loopback_set(phy_dev_t *phy_dev, int enable, phy_speed_t speed);
static int dsl_cl45phy_loopback_get(phy_dev_t *phy_dev, int *enable, phy_speed_t *speed);
#define MAX_CL45_PHYS 8
int dsl_runner_ext3_phy_init(phy_dev_t *phy_dev)
{
    static phy_cl45_t phys_cl45[MAX_CL45_PHYS];
    static int init;
    phy_drv_t *phy_drv = phy_dev->phy_drv;
    phy_cl45_t *phy_cl45;
    phy_drv_t _phy_drv = *phy_drv;
    uint32_t phy_id;
    uint32_t caps, support_caps;
    int i;

    phy_dev->inter_phy_types = 0;
    if (phy_dev->priv)
        return 0;

    phy_dev->autogreeen = 1;    /* By default, we use AutoGreeen mode for EEE */

    if (init > (MAX_CL45_PHYS - 1))
    {
        BUG_CHECK(" Error: More CL45 PHY %d requeted than supported %d\n", init+1, MAX_CL45_PHYS);
    }

    if (init == 0)
    {
        memset(phy_drv, 0, sizeof(*phy_drv));

        phy_drv->read_status = dsl_cl45phy_read_status;
        phy_drv->phy_type = _phy_drv.phy_type;
        phy_drv->name = _phy_drv.name;
        phy_drv->initialized = _phy_drv.initialized;
        phy_drv->speed_set = dsl_cl45phy_set_speed;
        phy_drv->caps_get = dsl_cl45phy_caps_get;
        phy_drv->caps_set = dsl_cl45phy_caps_set;
        phy_drv->config_speed_get = dsl_cl45phy_get_config_speed;
        phy_drv->phyid_get = _phy_drv.phyid_get;
        phy_drv->init = dsl_runner_ext3_phy_init;
        phy_drv->loopback_set = dsl_cl45phy_loopback_set;
        phy_drv->loopback_get = dsl_cl45phy_loopback_get;
        phy_drv->pair_swap_set = _phy_drv.pair_swap_set;
        phy_drv->isolate_phy = _phy_drv.isolate_phy;
        phy_drv->super_isolate_phy = _phy_drv.super_isolate_phy;
        phy_drv->apd_get = _phy_drv.apd_get;
        phy_drv->apd_set = _phy_drv.apd_set;
        phy_drv->eee_get = _phy_drv.eee_get;
        phy_drv->eee_set = _phy_drv.eee_set;
        phy_drv->eee_resolution_get = _phy_drv.eee_resolution_get;
        phy_drv->eee_mode_set = _phy_drv.eee_mode_set;
        phy_drv->eee_mode_get = _phy_drv.eee_mode_get;
        phy_drv->cable_diag_run = _phy_drv.cable_diag_run;
        phy_drv->inter_phy_types_get = _phy_drv.inter_phy_types_get;
        phy_drv->configured_inter_phy_types_set = _phy_drv.configured_inter_phy_types_set;
        phy_drv->priv_fun = dsl_phy_exp_op;
        phy_drv->power_set = _phy_drv.power_set;
        phy_drv->power_get = _phy_drv.power_get;
        phy_drv->get_phy_name = _phy_drv.get_phy_name;
        phy_drv->current_inter_phy_type_get = _phy_drv.current_inter_phy_type_get;
#ifdef MACSEC_SUPPORT
        phy_drv->macsec_oper = _phy_drv.macsec_oper;
#endif
		phy_drv->an_restart = _phy_drv.an_restart;
        phy_drv->xfi_tx_polarity_inverse_set = _phy_drv.xfi_tx_polarity_inverse_set;
        phy_drv->xfi_rx_polarity_inverse_set = _phy_drv.xfi_rx_polarity_inverse_set;
        phy_drv->leds_init = dsl_phy_leds_init;
        phy_drv->drv_init = _phy_drv.drv_init;
    }

    phy_cl45 = phy_dev->priv = &phys_cl45[init++];
    phy_cl45->config_speed = -1;
    if (init == 1)
        phy_cl45->read_status = _phy_drv.read_status;
    else
        phy_cl45->read_status = phys_cl45[0].read_status;

    for (i=0; i<ARRAY_SIZE(phyDesc); i++)
    {
        phy_dev_phyid_get(phy_dev, &phy_id);
        if (phyDesc[i].devId == phy_id)
        {
            phy_cl45->descriptor = &phyDesc[i];
            break;
        }
    }

    dsl_usxgmii_single_port_adjustment(phy_dev);

    /* Reset PHY */
    phy_bus_c45_write32(phy_dev, 0x10000, 0x8000);
    {
        u16 v16;
        u16 timeout = 4000;
        while (--timeout) {
            phy_bus_c45_read32(phy_dev, 0x10000, &v16);
            if ((v16 & 0x8000) == 0)
                break;
            udelay(500);
        }
#ifdef __UBOOT__
        if (!timeout)
            printk("Could not reset PHY. Your PHY rev is unsupported in u-boot.\n");
#else
        BUG_ON(!timeout);
#endif
    }

    if (phy_dev->cascade_prev)
        cascade_phy_set_common_inter_types(phy_dev->cascade_prev);
    phy_dev_configured_inter_phy_types_set(phy_dev, INTER_PHY_TYPE_UP, phy_dev->common_inter_phy_types);
    phy_dev_pair_swap_set(phy_dev, phy_dev->swap_pair);
    phy_dev_speed_set(phy_dev, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);
    phy_dev_super_isolate_phy(phy_dev, 0);
    phy_dev->an_enabled = 1; /* We always use AN even for specific speed */

    if (PhyHasWakeOnLan(phy_dev)) {
        printk("%s:%d: setting phy_dev %p addr 0x%x CORE_CFG_LED4_OVERRIDE to support Wake-on-Lan\n",
               __func__, __LINE__, phy_dev, phy_dev->addr);
        phy_bus_c45_write32(phy_dev, 0x1e407d, 0x5);
    }

    if (phy_dev->xfi_tx_polarity_inverse)
        printk("Invert XFI Tx Polarity of PHY %s at %d\n", phy_dev_get_phy_name(phy_dev), phy_dev->addr);
    phy_dev_xfi_tx_polarity_set(phy_dev, phy_dev->xfi_tx_polarity_inverse);

    if (phy_dev->xfi_rx_polarity_inverse)
        printk("Invert XFI Rx Polarity of PHY %s at %d\n", phy_dev_get_phy_name(phy_dev), phy_dev->addr);
    phy_dev_xfi_rx_polarity_set(phy_dev, phy_dev->xfi_rx_polarity_inverse);

    /* Correct default Speed Caps and Repeter bit by read caps and write it back */
    phy_dev_caps_get(phy_dev, CAPS_TYPE_SUPPORTED, &support_caps);
    caps = phy_xfi_speed_cap_adjust(phy_dev, support_caps);
    caps |= support_caps & PHY_CAP_NON_SPEED_CAPS;
    phy_dev_caps_set(phy_dev, caps);

    return 0;
}

static int dsl_cl45phy_set_speed(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    u16 v16;
    phy_cl45_t *phy_cl45 = phy_dev->priv;
    u32 caps;

    if (speed == phy_cl45->config_speed) return 0;

    cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_SUPPORTED, &caps);

    switch(speed)
    {
        case  PHY_SPEED_AUTO:
            SET_10G(phy_dev, ((caps & PHY_CAP_10000)>0));
            SET_5G(phy_dev, ((caps & PHY_CAP_5000)>0));
            SET_2P5G(phy_dev, ((caps & PHY_CAP_2500)>0));
            SET_1GFD(phy_dev, 1);
            SET_100MFD(phy_dev, 1);
            break;

        case PHY_SPEED_10000:
            if (!(caps & PHY_CAP_10000))
                goto not_supported;
            SET_10G(phy_dev, 1);
            SET_5G(phy_dev, 0);
            SET_2P5G(phy_dev, 0);
            SET_1GFD(phy_dev, 0);
            SET_100MFD(phy_dev, 0);
            break;

        case PHY_SPEED_5000:
            if (!(caps & PHY_CAP_5000))
                goto not_supported;
            SET_10G(phy_dev, 0);
            SET_5G(phy_dev, 1);
            SET_2P5G(phy_dev, 0);
            SET_1GFD(phy_dev, 0);
            SET_100MFD(phy_dev, 0);
            break;

        case PHY_SPEED_2500:
            if (!(caps & PHY_CAP_2500))
                goto not_supported;
            SET_10G(phy_dev, 0);
            SET_5G(phy_dev, 0);
            SET_2P5G(phy_dev, 1);
            SET_1GFD(phy_dev, 0);
            SET_100MFD(phy_dev, 0);
            break;

        case PHY_SPEED_1000:
            SET_10G(phy_dev, 0);
            SET_5G(phy_dev, 0);
            SET_2P5G(phy_dev, 0);
            SET_1GFD(phy_dev, 1);
            SET_100MFD(phy_dev, 0);
            break;

        case PHY_SPEED_100:
            SET_10G(phy_dev, 0);
            SET_5G(phy_dev, 0);
            SET_2P5G(phy_dev, 0);
            SET_1GFD(phy_dev, 0);
            SET_100MFD(phy_dev, 1);
            break;
        default:
            goto not_supported;
    }

    phy_dev->an_enabled = 1;

    /* Restart 10G level AN for IEEE2.5G */
    phy_bus_c45_read32(phy_dev, CL45REG_10GBASET_AN_CTL, &v16);
    v16 |= CL45_REG_10G_AN_ENABLE | CL45_REG_10G_AN_RESTART;
    phy_bus_c45_write32(phy_dev, CL45REG_10GBASET_AN_CTL, v16);

    /* Restart Auto Negotiation */
    phy_bus_c45_read32(phy_dev, CL45_REG_1G100M_CTL, &v16);
    v16 |= CL45_REG_1G100M_AN_ENABLED | CL45_REG_1G100M_AN_RESTART;
    phy_bus_c45_write32(phy_dev, CL45_REG_1G100M_CTL, v16);

    /* Restart XIFI Auto Negotiation */
    /* Below not correct, need to add TBD */
    #if 0
    phy_bus_c45_read32(phy_dev, 0, &v16);
    v16 |= CL45_REG_1G100M_AN_ENABLED | CL45_REG_1G100M_AN_RESTART;
    phy_bus_c45_read32(phy_dev, 0x10000, &v16);
    phy_bus_c45_write32(phy_dev, 0, v16);
    #endif

    phy_cl45->config_speed = speed;

    if (IS_USER_CONFIG())
        phy_cl45->link_changed = 1;

    return 0;
not_supported:
    printk("Not Supported Speed %x attempted to set on this interface\n", speed);
    return -1;
}

static int dsl_cl45phy_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps)
{
    u16 v16;
    phy_cl45_t *phy_cl45 = phy_dev->priv;

    *caps = 0;
    if (caps_type == CAPS_TYPE_SUPPORTED)
    {
        if (phy_cl45->support_speed_caps)
        {
            *caps = phy_cl45->support_speed_caps;
            return 0;
        }

        /* Use hardware power on advertisement caps as its support caps, since hw caps is not always correct in every chips */
        dsl_cl45phy_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, caps);

        *caps = phy_xfi_speed_cap_adjust(phy_dev, *caps);

        phy_cl45->support_speed_caps = *caps;
        return 0;
    }
    else if (caps_type == CAPS_TYPE_ADVERTISE)
    {
        /* 1000Base-T/100Base-TX MII control */
        phy_bus_c45_read32(phy_dev, CL45_REG_1G100M_CTL, &v16);
        if (v16 & CL45_REG_1G100M_AN_ENABLED)  *caps |= PHY_CAP_AUTONEG;

        /* Copper auto-negotiation advertisement */
        phy_bus_c45_read32(phy_dev, CL45_REG_COP_AN, &v16);
        if (v16 & CL45_REG_COP_PAUSE)             *caps |= PHY_CAP_PAUSE;
        if (v16 & CL45_REG_COP_PAUSE_ASYM)        *caps |= PHY_CAP_PAUSE_ASYM;
        if (v16 & CL45_REG_COP_AN_100M_HD_ADV)    *caps |= PHY_CAP_100_HALF;
        if (v16 & CL45_REG_COP_AN_100M_FD_ADV)    *caps |= PHY_CAP_100_FULL;
        if (v16 & CL45_REG_COP_AN_10M_FD_ADV)     *caps |= PHY_CAP_10_FULL;
        if (v16 & CL45_REG_COP_AN_10M_HD_ADV)     *caps |= PHY_CAP_10_HALF;

        /* 1000Base-T control */
        phy_bus_c45_read32(phy_dev, CL45_REG_1G_CTL, &v16);
        if (v16 & CL45_REG_1G_CTL_1G_HD_ADV) *caps |= PHY_CAP_1000_HALF;
        if (v16 & CL45_REG_1G_CTL_1G_FD_ADV) *caps |= PHY_CAP_1000_FULL;
        if (v16 & CL45_REG_CAP_REPEATER)     *caps |= PHY_CAP_REPEATER;

        /* 10GBase-T AN control */
        if (IsBrcm2P5GPhy(phy_dev))
        {
            phy_bus_c45_read32(phy_dev, CL45_REG_MGB_AN_CTL, &v16);
            if(v16 & CL45_REG_MGB_AN_2P5G_ADV) *caps |= PHY_CAP_2500;
        }
        else
        {
            phy_bus_c45_read32(phy_dev, CL45REG_10GBASET_AN_DEF_CTL, &v16);
            if (v16 & CL45_10GAN_2P5G_ABL)     *caps |= PHY_CAP_2500;
            if (v16 & CL45_10GAN_5G_ABL)       *caps |= PHY_CAP_5000;
            if (v16 & CL45_10GAN_10G_ABL)      *caps |= PHY_CAP_10000;
        }
    }

    return 0;
}

#define PHY_CAP_AUTO_SPEED_MASK ((PHY_CAP_AUTONEG<<1)-1)
static int dsl_cl45phy_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    int rc = 0;
    u16 v16;
    phy_speed_t speed;

    if (caps & PHY_CAP_AUTO_SPEED_MASK) {
        if (caps & PHY_CAP_AUTONEG)
            speed = PHY_SPEED_AUTO;
        else
            speed = phy_caps_to_max_speed(caps);
        rc = dsl_cl45phy_set_speed(phy_dev, speed, PHY_DUPLEX_FULL);
    }

    rc += phy_bus_c45_read32(phy_dev, CL45_REG_COP_AN, &v16);
    v16 &= ~(CL45_REG_COP_PAUSE|CL45_REG_COP_PAUSE_ASYM);
    v16 |= (caps&PHY_CAP_PAUSE?CL45_REG_COP_PAUSE:0)|(caps&PHY_CAP_PAUSE_ASYM?CL45_REG_COP_PAUSE_ASYM:0);
    rc += phy_bus_c45_write32(phy_dev, CL45_REG_COP_AN, v16);

    return rc;
}

static int dsl_cl45phy_get_config_speed(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex)
{
    phy_cl45_t *phy_cl45 = phy_dev->priv;

    *speed = phy_cl45->config_speed;
    *duplex = PHY_DUPLEX_FULL;
    return 0;
}

static int dsl_cl45phy_loopback_set(phy_dev_t *phy_dev, int enable, phy_speed_t speed)
{
    int rc = 0;
    u16 v16;
    int cur_loopback;
    uint32_t caps;
    phy_speed_t cur_speed;

    dsl_cl45phy_loopback_get(phy_dev, &cur_loopback, &cur_speed);

    if(enable) {
        if (cur_loopback) {
            printk("Error: Disable loop back first before enabling it.\n");
            return -1;
        }

        if (phy_dev->link) {
            printk("Error: Disconnect the port before renabling loopback.\n");
            return -1;
        }

        if (speed == PHY_SPEED_UNKNOWN)
            speed = cascade_phy_max_speed_get(phy_dev);

        cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps);
        if (!phy_dev_cap_speed_match(caps, speed)) {
            printk("Error: Speed %d Mbps is not supported by this PHY.\n", phy_speed_2_mbps(speed));
            return -1;
        }

        switch(speed) {
            case PHY_SPEED_100:
            case PHY_SPEED_1000:
                rc += phy_bus_c45_read32(phy_dev, CL45_REG_1G100M_CTL, &v16);
                phy_dev->loopback_save = v16;
                v16 &= ~CL45_REG_SPEED_MASK;
                v16 |= (speed==PHY_SPEED_1000? CL45_REG_1000M_SPEED: CL45_REG_100M_SPEED)|CL45_REG_INTNL_LOOPBACK;
                rc += phy_bus_c45_write32(phy_dev, CL45_REG_1G100M_CTL, v16);
                break;
            case PHY_SPEED_2500:
            case PHY_SPEED_10000:
                rc += phy_bus_c45_read32(phy_dev, CL45_REG_PCS_CONTROL_1, &v16);
                phy_dev->loopback_save = v16;
                v16 &= ~CL45_SPEED_SEL_MASK;
                v16 |= CL45_PCS_LOOPBACK|(speed==PHY_SPEED_2500? CL45_SPEED_SEL_2P5G: CL45_SPEED_SEL_10G);
                rc += phy_bus_c45_write32(phy_dev, CL45_REG_PCS_CONTROL_1, v16);
                break;
            default:
                break;
        }
        rc += phy_bus_c45_read32(phy_dev, CL45_REG_1G_TEST_REG, &v16);
        v16 |= CL45_FORCE_LINK_UP;
        rc += phy_bus_c45_write32(phy_dev, CL45_REG_1G_TEST_REG, v16);
    }
    else {
        /* Only do disable if enable is done by the same API, not direct register operation */
        if (!phy_dev->loopback_save) {
            printk("Error: Loopback is not enabled\n");
            return -1;
        }

        rc += phy_bus_c45_write32(phy_dev, cur_speed>PHY_SPEED_1000? CL45_REG_PCS_CONTROL_1:CL45_REG_1G100M_CTL,
                phy_dev->loopback_save);
        rc += phy_bus_c45_read32(phy_dev, CL45_REG_1G_TEST_REG, &v16);
        v16 &= ~CL45_FORCE_LINK_UP;
        rc += phy_bus_c45_write32(phy_dev, CL45_REG_1G_TEST_REG, v16);
        phy_dev->loopback_save = 0;
    }
    return rc;
}

static int dsl_cl45phy_loopback_get(phy_dev_t *phy_dev, int *enable, phy_speed_t *speed)
{
    u16 v16;
    int rc = 0;

    *enable = 0;
    if (phy_dev->loopback_save == 0)
        return 0;

    *enable = 1;

    rc += phy_bus_c45_read32(phy_dev, CL45_REG_1G100M_CTL, &v16);
    if (v16 & CL45_REG_INTNL_LOOPBACK) {
        *speed = (v16&CL45_REG_1000M_SPEED)? PHY_SPEED_1000:PHY_SPEED_100;
    }
    else {
        rc += phy_bus_c45_read32(phy_dev, CL45_REG_PCS_CONTROL_1, &v16);
        *speed = (v16 & CL45_SPEED_SEL_MASK) == CL45_SPEED_SEL_10G? PHY_SPEED_10000: PHY_SPEED_2500;
    }

    return rc;
}

static int phy_bus_c45_xfi_read32(phy_dev_t *phy_dev, int reg, uint16_t *v16)
{
    phy_bus_c45_write32(phy_dev, 0x1e4110, 0x2004);
    phy_bus_c45_write32(phy_dev, 0x1e4111, 0x2004);
    phy_bus_c45_write32(phy_dev, 0x1e4113, 0x2004);

    phy_bus_c45_read32(phy_dev, reg, v16);

    phy_bus_c45_write32(phy_dev, 0x1e4110, 0x0001);
    phy_bus_c45_write32(phy_dev, 0x1e4111, 0x0001);
    phy_bus_c45_write32(phy_dev, 0x1e4113, 0x1002);

    return 0;
}
static int dsl_cl45phy_read_status(phy_dev_t *phy_dev)
{
    u16 v16;
    phy_cl45_t *phy_cl45 = phy_dev->priv;
    phy_speed_t org_speed = phy_dev->speed;
    int org_link = phy_dev->link;
    int i = 0;

    if (phy_cl45->link_changed)
    {
        phy_cl45->link_changed = 0;
        goto ret;
    }

    phy_cl45->read_status(phy_dev);

    phy_bus_c45_read32(phy_dev, CL45_REG_UDEF_STATUS, &v16);
    phy_dev->mac_link = ((v16 & CL45_UDEF_STATUS_MAC_LINK)> 0) | PHY_MAC_LINK_VALID;

    if (org_link && phy_dev->link && org_speed != phy_dev->speed)
        phy_dev->link = 0;

    /* Check 100M speed specially */
    if (phy_dev->link && phy_dev->speed == PHY_SPEED_100 &&
        phy_dev->duplex == PHY_DUPLEX_HALF)
    {
        phy_bus_c45_read32(phy_dev, CL45_REG_1G_CTL, &v16);
        if ((v16 & CL45_AN_LP_AN_CAP))  /* We don't support HD, suppress link up */
            phy_dev->link = 0;
        else    /* It is 100M Non AN LP, set to forced 100M FD to link with it */
        {
            /* Set wrong speed to force current link down for XFI */
            phy_bus_c45_xfi_read32(phy_dev, 0x10000, &v16);
            if (v16)
            {
                phy_bus_c45_write32(phy_dev, CL45_REG_1G100M_CTL, CL45_REG_1000M_SPEED);
                for (i=0; i<30 && phy_dev->link; i++, msleep(100))
                    phy_cl45->read_status(phy_dev);
            }
            phy_bus_c45_write32(phy_dev, CL45_REG_1G100M_CTL, FORCED_100MFD_CTRL);
            msleep(1000);
            for (i=0; i<60 && (!phy_dev->link || phy_dev->duplex == PHY_DUPLEX_HALF); i++, msleep(100))
                phy_cl45->read_status(phy_dev);
            phy_cl45->forced_100m_fd = 1;
        }
    }

ret:
    if (phy_dev->link == 0)
    {
        if (phy_cl45->forced_100m_fd)
        {
            phy_bus_c45_write32(phy_dev, CL45_REG_1G100M_CTL, REGULAR_CTRL_REG0_VAL);
            phy_cl45->forced_100m_fd = 0;
        }
        phy_dev->speed = 0; /* For uboot trick borrowing NON DSL code */
    }

    if (org_link ^ phy_dev->link)
        phy_dev_configured_inter_phy_types_set(phy_dev, INTER_PHY_TYPE_UP, phy_dev->configured_inter_phy_types);

    return 0;
}


