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
 *  Created on: Dec 2016
 *      Author: steven.hsieh@broadcom.com
 */

/*
 * Phy drivers for 63138, 63148, 4908
 */
#include "phy_drv_xgae.h"

static void config_serdes(phy_dev_t *phy_dev, u32 seq[], int seqSize);
static int ethsw_phy_enable_an(phy_dev_t *phy_dev);
static void sgmiiResCal(phy_dev_t *phy_dev);

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
    if (v>0) v16 |= CL45_REG_COP_AN_100M_FD_ADV; \
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

#define SET_10G(p, v) do { \
    uint16 v16; \
    phy_bus_c45_read32(p, CL45REG_10GBASET_AN_DEF_CTL, &v16); \
    v16 &= ~CL45_10GAN_5G_ABL; \
    v16 = v>0? v16|CL45_10GAN_10G_ABL : v16&~CL45_10GAN_10G_ABL; \
    phy_bus_c45_write32(p, CL45REG_10GBASET_AN_DEF_CTL, v16); \
    } while(0)
static void ethsw_2p5g_serdes_link_stats(phy_dev_t *phy_dev);
DEFINE_MUTEX(serdes_mutex);

#if defined(PHY_SERDES_2P5G_CAPABLE)
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

static int sf2_cl45phy_read_status(phy_dev_t *phy_dev);
static int ethsw_cl45phy_set_speed(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
static int ethsw_cl45phy_get_config_speed(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex);
static int ethsw_cl45phy_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps);
static int ethsw_cl45phy_caps_set(phy_dev_t *phy_dev, uint32_t caps);
static int sf2_cl45phy_loopback_set(phy_dev_t *phy_dev, int enable, phy_speed_t speed);
static int sf2_cl45phy_loopback_get(phy_dev_t *phy_dev, int *enable, phy_speed_t *speed);
#define MAX_CL45_PHYS 4
int dsl_runner_ext3_phy_init(phy_dev_t *phy_dev)
{
    static phy_cl45_t phys_cl45[MAX_CL45_PHYS];
    static int init;
    phy_drv_t *phy_drv = phy_dev->phy_drv;
    phy_cl45_t *phy_cl45;
    phy_drv_t _phy_drv = *phy_drv;
    uint32_t phy_id;
    int i;

    if (init > (MAX_CL45_PHYS - 1))
    {
        printk(" Error: More CL45 PHY %d requeted than supported %d\n", init+1, MAX_CL45_PHYS);
        BUG();
    }

    if (init++ == 0)
    {
        memset(phy_drv, 0, sizeof(*phy_drv));

        phy_drv->read_status = sf2_cl45phy_read_status;
        phy_drv->phy_type = _phy_drv.phy_type;
        phy_drv->name = _phy_drv.name;
        phy_drv->initialized = _phy_drv.initialized;
        phy_drv->bus_drv = _phy_drv.bus_drv;
        phy_drv->speed_set = ethsw_cl45phy_set_speed;
        phy_drv->caps_get = ethsw_cl45phy_caps_get;
        phy_drv->caps_set = ethsw_cl45phy_caps_set;
        phy_drv->config_speed_get = ethsw_cl45phy_get_config_speed;
        phy_drv->phyid_get = _phy_drv.phyid_get;
        phy_drv->init = dsl_runner_ext3_phy_init;
        phy_drv->loopback_set = sf2_cl45phy_loopback_set;
        phy_drv->loopback_get = sf2_cl45phy_loopback_get;
        phy_drv->pair_swap_set = _phy_drv.pair_swap_set;
        phy_drv->isolate_phy = _phy_drv.isolate_phy;
        phy_drv->super_isolate_phy = _phy_drv.super_isolate_phy;
        phy_drv->eee_get = _phy_drv.eee_get;
        phy_drv->eee_set = _phy_drv.eee_set;
        phy_drv->eee_resolution_get = _phy_drv.eee_resolution_get;
    }

    phy_cl45 = phy_dev->priv = &phys_cl45[init++];
    phy_cl45->config_speed = -1;
    for (i=0; i<ARRAY_SIZE(phyDesc); i++)
    {
        phy_dev_phyid_get(phy_dev, &phy_id);
        if (phyDesc[i].devId == phy_id)
        {
            phy_cl45->descriptor = &phyDesc[i];
            break;
        }
    }

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
//#ifdef __UBOOT__
#if 1
        if (!timeout)
            printk("Could not reset PHY. Your PHY rev is unsupported in u-boot.\n");
#else
        BUG_ON(!timeout);
#endif
    }

    phy_dev_pair_swap_set(phy_dev, phy_dev->swap_pair);
    phy_dev_speed_set(phy_dev, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);
    phy_dev_super_isolate_phy(phy_dev, 0);

    return 0;
}

static u32 serdesRef50mVco6p25 [] =
{
    0x8000, 0x0c2f,
    0x8308, 0xc000,
    0x8050, 0x5740,
    0x8051, 0x03c001d0,
    0x8052, 0x19f0,
    0x8053, 0xaab0,
    0x8054, 0x8821,
    0x8055, 0x0044,
    0x8056, 0x8000,
    0x8057, 0x0872,
    0x8058, 0x0000,

    0x8106, 0x00400020,
    0x8054, 0x8021,
    0x8054, 0x8821,
};

static u32 serdesSet2p5GFiber [] =
{
    0x0010, 0x0C2F,       /* disable pll start sequencer */
    0x8066, 0x0009,       /* Set AFE for 2.5G */
    0x8065, 0x1620,
    0x8300, 0x0149,       /* enable fiber mode, also depend board parameters */
    0x8308, 0xC010,       /* Force 2.5G Fiber, enable 50MHz refclk */
    0x834a, 0x0001,       /* Set os2 mode */
    0x0000, 0x0140,       /* disable AN, set 1G mode */
    0x0010, 0x2C2F,       /* enable pll start sequencer */
};

static u32 serdesSet1GFiber [] =
{
    0x0010, 0x0c2f,     /* disable pll start sequencer */
    0x8300, 0x0149,     /* Force Auto Detect, Invert Signal Polarity, also depend on board parameters */
    0x8066, 0x8009,     /* Set AFE to Default Values */
    0x8065, 0x3220,
    0x8473, 0x1251,
    0x834a, 0x0003,
    0x0000, 0x0140,
    0x0010, 0x2c2f,     /* enable pll start sequencer */
};

static u32 serdesSet100MFiber [] =
{
    0x0010, 0x0c2f,     /* disable pll start sequencer */
    0x8300, 0x0009,     /* enable fiber mode, also depend on board parameters */
    0x8400, 0x014b,     /* enable fxmode */
    0x8066, 0x8009,     /* Set AFE to Default Values */
    0x8065, 0x3220,
    0x8402, 0x0880,     /* set disable_idle_correlator */
    0x8473, 0x1251,     /* Set step_one[1:0] */
    0x834a, 0x0003,     /* set os5 mode */
    0x0010, 0x2c2f,     /* enable pll start sequencer */
};

static u32 serdesSet100MForcedSGMII [] =
{
    0x0010, 0x0c2f,     /* disable pll start sequencer */
    0x8300, 0x0100,     /* also depend on board parameters */
    0x8066, 0x8009,     /* Set AFE to Default Values */
    0x8065, 0x3220,
    0x8301, 0x0007,
    0x8473, 0x1251,
    0x834a, 0x0003,
    0x0000, 0x2100,
    0x0010, 0x2c2f,     /* enable pll start sequencer */
};

#else
static u32 serdesSet1GFiber [] =
{
    0x0010, 0x0c2f,
    0x8182, 0x4000,     /* This and next lines are for yield rate improvement */
    0x8186, 0x003c,
    0x8300, 0x015d,     /* Force Auto Detect, Invert Signal Polarity, also depend on board parameters */
    0x8301, 0x7,
    0x0,    0x0140,
    0x0010, 0x2c2f
};

static u32 serdesSet100MFiber [] =
{
    0x0010, 0x0c2f,
    0x8182, 0x4000,     /* This and next lines are for yield rate improvement */
    0x8186, 0x003c,
    0x8300, 0xd,        /*  also depend on board parameters */
    0x8400, 0x14b,
    0x8402, 0x880,
    0x0010, 0x2c2f,
};

static u32 serdesSet100MForcedSGMII [] =
{
    0x0010, 0x0c2f,
    0x8182, 0x4000,     /* This and next lines are for yield rate improvement */
    0x8186, 0x003c,
    0x8300, 0x0100,     /*  also depend on board parameters */
    0x0,    0x2100,
    0x0010, 0x2c2f
};

#endif
static void sf2_config_los(phy_dev_t *phy_dev)
{
    u16 v16;

    phy_serdes_t *phy_serdes = phy_dev->priv;

    ethsw_phy_exp_read(phy_dev, SERDES_1000X_CONTROL, &v16);
    if (phy_serdes->signal_detect_gpio != -1)
    {
        v16 |= SERDES_SIGNAL_DETECTE_EN;
        if (phy_serdes->signal_detect_invert)
            v16 |= SERDES_INVERT_SIGNAL_DET;
        else
            v16 &= ~SERDES_INVERT_SIGNAL_DET;
    }
    else
        v16 &= ~SERDES_SIGNAL_DETECTE_EN;
    ethsw_phy_exp_write(phy_dev, SERDES_1000X_CONTROL, v16);
}

void ethsw_sgmii_serdes_reset(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    ethsw_phy_reset(phy_dev);
    phy_serdes->current_speed = -1;
    sgmiiResCal(phy_dev);
}

#if defined(PHY_SERDES_2P5G_CAPABLE)
static void ethsw_config_serdes_2p5g(phy_dev_t *phy_dev)
{
    ethsw_sgmii_serdes_reset(phy_dev);
    config_serdes(phy_dev, serdesRef50mVco6p25, sizeof(serdesRef50mVco6p25));
    msleep(1);
    config_serdes(phy_dev, serdesSet2p5GFiber, sizeof(serdesSet2p5GFiber));
    sf2_config_los(phy_dev);
}

static void inline ethsw_config_serdes_forced_2p5g(phy_dev_t *phy_dev)
{
    ethsw_config_serdes_2p5g(phy_dev);
    msleep(20);
}

static int ethsw_cl45phy_set_speed(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    u16 v16;
    phy_cl45_t *phy_cl45 = phy_dev->priv;
    u32 caps;

    if (speed == phy_cl45->config_speed) return 0;

    cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps);

    SET_10G(phy_dev, 0);
    switch(speed)
    {
        case  PHY_SPEED_AUTO:
            if (caps & PHY_CAP_10000)
                SET_10G(phy_dev, 1);
            SET_2P5G(phy_dev, 1);
            SET_1GFD(phy_dev, 1);
            SET_100MFD(phy_dev, 1);
            break;

        case PHY_SPEED_10000:
            if (!(caps & PHY_CAP_10000))
                return -1;
            SET_10G(phy_dev, 1);
            SET_2P5G(phy_dev, 0);
            SET_1GFD(phy_dev, 0);
            SET_100MFD(phy_dev, 0);
            break;

        case PHY_SPEED_2500:
            SET_2P5G(phy_dev, 1);
            SET_1GFD(phy_dev, 0);
            SET_100MFD(phy_dev, 0);
            break;

        case PHY_SPEED_1000:
            SET_2P5G(phy_dev, 0);
            SET_1GFD(phy_dev, 1);
            SET_100MFD(phy_dev, 0);
            break;

        case PHY_SPEED_100:
            SET_2P5G(phy_dev, 0);
            SET_1GFD(phy_dev, 0);
            SET_100MFD(phy_dev, 1);
            break;
        default:
            printk("Not Supported Speed %x attempted to set on this interface\n", speed);
            return -1;
    }

    if (!IsBrcm2P5GPhy(phy_dev))
    {
        /* Restart 10G level AN for IEEE2.5G */
        phy_bus_c45_read32(phy_dev, CL45REG_10GBASET_AN_CTL, &v16);
        v16 |= CL45_REG_10G_AN_ENABLE | CL45_REG_10G_AN_RESTART;
        phy_bus_c45_write32(phy_dev, CL45REG_10GBASET_AN_CTL, v16);
    }

    /* Restart Auto Negotiation */
    phy_bus_c45_read32(phy_dev, CL45_REG_1G100M_CTL, &v16);
    v16 |= CL45_REG_1G100M_AN_ENABLED | CL45_REG_1G100M_AN_RESTART;
    phy_bus_c45_write32(phy_dev, CL45_REG_1G100M_CTL, v16);

    phy_cl45->config_speed = speed;

    return 0;
}

static int ethsw_cl45phy_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps)
{
    u16 v16;

    if (caps_type == CAPS_TYPE_SUPPORTED)
    {
        phy_bus_c45_read32(phy_dev, CL45_REG_PMA_PMD_EXT_ABL, &v16);

        *caps = PHY_CAP_AUTONEG;
        if (v16 & CL45_REG_CAP_2P5G_5G)     *caps |= PHY_CAP_2500;
        if (v16 & CL45_REG_CAP_100MB_T)     *caps |= PHY_CAP_100_FULL|PHY_CAP_100_HALF;
        if (v16 & CL45_REG_CAP_1GB_T)       *caps |= PHY_CAP_1000_FULL|PHY_CAP_1000_HALF;
        if (v16 & CL45_REG_CAP_10GB_T)      *caps |= PHY_CAP_10000;
        if (IsBrcm2P5GPhy(phy_dev))         *caps |= PHY_CAP_2500;
        if (IsBrcm10GPhy(phy_dev))          *caps |= PHY_CAP_10000;
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
    else
        *caps = 0;

    return 0;
}

#define PHY_CAP_AUTO_SPEED_MASK ((PHY_CAP_AUTONEG<<1)-1)
static int ethsw_cl45phy_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    int rc = 0;
    u16 v16;
    phy_speed_t speed;

    if (caps & PHY_CAP_AUTO_SPEED_MASK) {
        if (caps & PHY_CAP_AUTONEG)
            speed = PHY_SPEED_AUTO;
        else
            speed = phy_caps_to_max_speed(caps);
        rc = ethsw_cl45phy_set_speed(phy_dev, speed, PHY_DUPLEX_FULL);
    }

    rc += phy_bus_c45_read32(phy_dev, CL45_REG_COP_AN, &v16);
    v16 &= ~(CL45_REG_COP_PAUSE|CL45_REG_COP_PAUSE_ASYM);
    v16 |= (caps&PHY_CAP_PAUSE?CL45_REG_COP_PAUSE:0)|(caps&PHY_CAP_PAUSE_ASYM?CL45_REG_COP_PAUSE_ASYM:0);
    rc += phy_bus_c45_write32(phy_dev, CL45_REG_COP_AN, v16);

    return rc;
}

static int ethsw_cl45phy_get_config_speed(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex)
{
    phy_cl45_t *phy_cl45 = phy_dev->priv;

    *speed = phy_cl45->config_speed;
    *duplex = PHY_DUPLEX_FULL;
    return 0;
}

static int sf2_cl45phy_loopback_set(phy_dev_t *phy_dev, int enable, phy_speed_t speed)
{
    int rc = 0;
    u16 v16;
    int cur_loopback, caps;
    phy_speed_t cur_speed;

    sf2_cl45phy_loopback_get(phy_dev, &cur_loopback, &cur_speed);

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

static int sf2_cl45phy_loopback_get(phy_dev_t *phy_dev, int *enable, phy_speed_t *speed)
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

static int sf2_cl45phy_read_status(phy_dev_t *phy_dev)
{
    u16 v16;

    phy_bus_c45_read32(phy_dev, CL45_REG_UDEF_STATUS, &v16);
    phy_dev->link = (v16 & CL45_UDEF_STATUS_COPPER_LINK)> 0;

    switch (v16 & CL45_UDEF_STATUS_COPPER_SPD_M)
    {
        case CL45_UDEF_STATUS_COPPER_SPD_10G:
            phy_dev->speed = PHY_SPEED_10000;
            break;
        case CL45_UDEF_STATUS_COPPER_SPD_2P5G:
            phy_dev->speed = PHY_SPEED_2500;
            break;
        case CL45_UDEF_STATUS_COPPER_SPD_1G:
            phy_dev->speed = PHY_SPEED_1000;
            break;
        case CL45_UDEF_STATUS_COPPER_SPD_100M:
            phy_dev->speed = PHY_SPEED_100;
            break;
    }

    phy_dev->duplex = PHY_DUPLEX_FULL;

    phy_bus_c45_read32(phy_dev, CL45_REG_1G100M_AUX_STATUS , &v16);
    phy_dev->pause_rx = (v16 & CL45_AN_STATUS_RX_PAUSE)>0;
    phy_dev->pause_tx = (v16 & CL45_AN_STATUS_TX_PAUSE)>0;

    return 0;
}
#endif

enum {SFP_MODULE_OUT, SFP_MODULE_IN, SFP_LINK_UP};

static int ethsw_powerop_serdes(phy_dev_t *phy_dev, int powerLevel);
int sf2_serdes_phy_read_status(phy_dev_t *phy_dev);
int sf2_serdes_init(phy_dev_t *phy_dev);
static int ethsw_serdes_power_mode_set(phy_dev_t *phy_dev, int mode);
static int ethsw_conf_copper_sfp(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);

phy_drv_t phy_drv_i2c_phy =
{
    .phy_type = PHY_TYPE_I2C_PHY,
    .speed_set = ethsw_conf_copper_sfp,
    .config_speed_get = ethsw_serdes_speed_get,
    .caps_get = ethsw_serdes_caps_get,
    .name = "I2C",
};

static int sfp_i2c_module_detect(phy_dev_t *phy_dev);
/****************************************************************************
    Broadcom Extended PHY registers
****************************************************************************/
#define BRCM_MIIEXT_BANK            0x1f
    #define BRCM_MIIEXT_BANK_MASK       0xfff0
    #define BRCM_MIIEXT_ADDR_RANGE      0xffe0
    #define BRCM_MIIEXT_DEF_BANK        0x8000
#define BRCM_MIIEXT_OFFSET          0x10
    #define BRCM_MIIEXT_OFF_MASK    0xf

#define MIIEX_DIGITAL_STATUS_1000X          0x8304
    #define MIIEX_SPEED_SHIFT       3
    #define MIIEX_SPEED             (3<<MIIEX_SPEED_SHIFT)
        #define MIIEX_SPD10         (0<<MIIEX_SPEED_SHIFT)
        #define MIIEX_SPD100        (1<<MIIEX_SPEED_SHIFT)
        #define MIIEX_SPD1000       (2<<MIIEX_SPEED_SHIFT)
        #define MIIEX_SPD2500       (3<<MIIEX_SPEED_SHIFT)
    #define MIIEX_DUPLEX            (1<<2)
    #define MIIEX_LINK              (1<<1)
    #define MIIEX_SGMII_MODE        (1<<0)

#define PLL_AFE_CTRL1               0x8051
    #define PLL_AFE_PLL_PON_SHIFT   6
    #define PLL_AFE_PLL_PON_MASK    (0xf << PLL_AFE_PLL_PON_SHIFT)

#define TX_AFE_CTRL2                0x8067
    #define TX_AFE_TX_PON_SHIFT     3
    #define TX_AFE_TX_PON_MASK      (0xf << TX_AFE_TX_PON_SHIFT)

#define RX_AFE_CTRL0                0x80b2
    #define RX_AFE_RX_PON_SHIFT     8
    #define RX_AFE_RX_PON_MASK      (0xf << RX_AFE_RX_PON_SHIFT)

#define RX_AFE_CTRL2                0x80b5
    #define INPUTERM_LOWZGND_EN     (1<<5)
    #define INPUTERM_LOWZVDD_EN     (1<<6)

#define BLK2_TESTMODELANE           0x8106
    #define RESCAL_SEL              (1<<6)

static void sgmiiResCal(phy_dev_t *phy_dev)
{
    static int init = 0;
#if defined(RCAL_1UM_VERT)
    static int val;
    uint16_t v16;
#endif

    /*
       Resistor calibration for SFP RX_DATA AC coupling board design
       For SFP RX_DATA DC coupling board design, set value to (RX_AFE_CTRL2_DIV4 | INPUTERM_LOWZVDD_EN))
    */
    ethsw_phy_exp_write(phy_dev, RX_AFE_CTRL2, (RX_AFE_CTRL2_DIV4 | INPUTERM_LOWZGND_EN));

#if defined(RCAL_1UM_VERT)
    if (init == 0) {
        if(GetRCalSetting(RCAL_1UM_VERT, &val) != kPMC_NO_ERROR)
        {
            printk("AVS is not turned on, leave SGMII termination resistor values as current default\n");
            ethsw_phy_exp_read(phy_dev, PLL_AFE_CTRL1, &v16);
            printk("    PLL_PON: 0x%04x; ", (v16 & PLL_AFE_PLL_PON_MASK) >> PLL_AFE_PLL_PON_SHIFT);
            ethsw_phy_exp_read(phy_dev, TX_AFE_CTRL2, &v16);
            pr_cont("TX_PON: 0x%04x; ", (v16 & TX_AFE_TX_PON_MASK) >> TX_AFE_TX_PON_SHIFT);
            ethsw_phy_exp_read(phy_dev, RX_AFE_CTRL0, &v16);
            pr_cont("RX_PON: 0x%04x\n", (v16 & RX_AFE_RX_PON_MASK) >> RX_AFE_RX_PON_SHIFT);
            return;
        }
    }

    val &= 0xf;
    if (!init)
        printk("Setting SGMII Calibration value to 0x%x\n", val);

    ethsw_phy_exp_read(phy_dev, PLL_AFE_CTRL1, &v16);
    v16 = (v16 & (~PLL_AFE_PLL_PON_MASK)) | (val << PLL_AFE_PLL_PON_SHIFT);
    ethsw_phy_exp_write(phy_dev, PLL_AFE_CTRL1, v16);
    if (!init)
        printk("    PLL_PON(%04x): 0x%04x; ", PLL_AFE_CTRL1, v16);

    ethsw_phy_exp_read(phy_dev, TX_AFE_CTRL2, &v16);
    v16 = (v16 & (~TX_AFE_TX_PON_MASK)) | (val << TX_AFE_TX_PON_SHIFT);
    ethsw_phy_exp_write(phy_dev, TX_AFE_CTRL2, v16);
    if (!init)
        pr_cont("TX_PON(%04x): 0x%04x; ", TX_AFE_CTRL2, v16);

    ethsw_phy_exp_read(phy_dev, RX_AFE_CTRL0, &v16);
    v16 = (v16 & (~RX_AFE_RX_PON_MASK)) | (val << RX_AFE_RX_PON_SHIFT);
    ethsw_phy_exp_write(phy_dev, RX_AFE_CTRL0, v16);
    if (!init)
        pr_cont("RX_PON(%04x): 0x%04x\n", RX_AFE_CTRL0, v16);

#if defined(CONFIG_BCM947622)
    // SGMIIPLUS2_X1 need this for the internal reg settings to take effect
    ethsw_phy_exp_read(phy_dev, BLK2_TESTMODELANE, &v16);
    v16 |= RESCAL_SEL;
    ethsw_phy_exp_write(phy_dev, BLK2_TESTMODELANE, v16);
#endif

#endif
    init = 1;
}

static int sf2_serdes_cfg_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
static int sf2_2p5g_serdes_init(phy_dev_t *phy_dev)
{
    uint16_t v16;
    phy_serdes_t *phy_serdes = phy_dev->priv;

    ethsw_powerup_serdes(phy_dev);

    /* read back for testing */
    phy_bus_read(phy_dev, MII_CONTROL, &v16);

    /* Reset PHY and Calibrate SGMII termination resistors */
    ethsw_sgmii_serdes_reset(phy_dev);

    /* If not a combo PHY, call the function to init state machine without leaving
       SFP on without control during initialization */
    if (phy_dev->cascade_next == NULL)
    {
        phy_dev->phy_drv->read_status(phy_dev);
    }

	sf2_serdes_cfg_speed_set(phy_dev, phy_serdes->config_speed, PHY_DUPLEX_FULL);
    return 0;
}

static int ethsw_serdes_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
static int ethsw_sfp_light_detected(phy_dev_t *phy_dev);
#define MAX_SERDES_NUMBER 2
static phy_serdes_t serdes[MAX_SERDES_NUMBER] = {
    {
        .phy_type = PHY_TYPE_SF2_SERDES,
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_1000_FULL|PHY_CAP_100_FULL
#if defined(PHY_SERDES_2P5G_CAPABLE)
                |PHY_CAP_2500
#endif
            ,
        .link_stats = ethsw_2p5g_serdes_link_stats,
        .config_speed = PHY_SPEED_AUTO,
        .serdes_init = sf2_2p5g_serdes_init,
        .power_mode = SERDES_NO_POWER_SAVING,
        .power_admin_on = 1,
        .speed_set = ethsw_serdes_speed_set,
        .power_set = ethsw_powerop_serdes,
        .enable_an = ethsw_phy_enable_an,
        .light_detected = ethsw_sfp_light_detected,
    },
#if defined(PHY_SERDES_10G_CAPABLE)
    {
        .phy_type = PHY_TYPE_XGAE,
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_10000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .link_stats = ethsw_xgae_serdes_link_stats,
        .config_speed = PHY_SPEED_AUTO,
        .serdes_init = sf2_xgae_serdes_init,
        .power_mode = SERDES_NO_POWER_SAVING,
        .power_admin_on = 1,
        .speed_set = ethsw_xgae_speed_set,
        .enable_an = ethsw_xgae_enable_an,
        .light_detected = ethsw_xgae_light_detected,
        .lbe_op = sf2_xgae_lbe_op,
    },
#endif
};

#if defined(CONFIG_I2C) && defined(CONFIG_BCM_OPTICALDET)
static phy_i2c_priv_t phy_i2c_priv[MAX_SERDES_NUMBER];
static int phy_i2c_num;
#endif

static phy_drv_t phy_drv_sf2_serdes_extsw;
int sf2_serdes_init(phy_dev_t *phy_dev)
{
    static int i;
    phy_serdes_t *phy_serdes;
    int ret = 0;
#if defined(CONFIG_I2C) && defined(CONFIG_BCM_OPTICALDET)
    phy_dev_t *i2c_phy;
    int bus_num = -1;
#endif
    phy_drv_t *phy_drv = phy_dev->phy_drv;
    int bug = 0;

    for (i = 0; i < MAX_SERDES_NUMBER; i++)
        if (!serdes[i].used && serdes[i].phy_type == phy_drv->phy_type)
            break;

    if (i == MAX_SERDES_NUMBER) return -1;
    phy_serdes = phy_dev->priv = &serdes[i];
    phy_serdes->used = 1;
    phy_serdes->phy_dev = phy_dev;

    if (phy_serdes->speed_caps & PHY_CAP_10000) {
        phy_serdes->highest_speed_cap = PHY_CAP_10000;
        phy_serdes->highest_speed = PHY_SPEED_10000;
    } else if (phy_serdes->speed_caps & PHY_CAP_2500) {
        phy_serdes->highest_speed_cap = PHY_CAP_2500;
        phy_serdes->highest_speed = PHY_SPEED_2500;
    } else if (phy_serdes->speed_caps & PHY_CAP_1000_FULL) {
        phy_serdes->highest_speed_cap = PHY_CAP_1000_FULL;
        phy_serdes->highest_speed = PHY_SPEED_1000;
    } else if (phy_serdes->speed_caps & PHY_CAP_100_FULL) {
        phy_serdes->highest_speed_cap = PHY_CAP_100_FULL;
        phy_serdes->highest_speed = PHY_SPEED_100;
    }

    if (IsPortConnectedToExternalSwitch(phy_dev->meta_id)) {
        phy_serdes->power_mode = SERDES_NO_POWER_SAVING,
        phy_serdes->config_speed = phy_serdes->highest_speed;
        phy_serdes->signal_detect_gpio = -1;
        phy_drv_sf2_serdes_extsw.bus_drv = phy_drv->bus_drv;
        memcpy(phy_drv, &phy_drv_sf2_serdes_extsw, sizeof(*phy_drv));
        phy_dev->flag |= PHY_FLAG_TO_EXTSW;
        goto end;
    }

    sf2_serdes_sfp_lbe_op(phy_dev, LASER_OFF); /* Notify no SFP to turn off laser in the beginning, just in case hardware set on */

#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM947622_) || defined(CONFIG_BCM947622)
    {
        int val;
        bcm_otp_is_sgmii_disabled(&val);
        if(val)
        {
            printk("****** Error: Invalid Serdes PHY defiend in board parameter - this chip does not support Serdes.\n");
            return -1;
        }
    }
#endif

#if defined(CONFIG_I2C) && defined(CONFIG_BCM_OPTICALDET)
    if ((phy_dev->cascade_next == NULL))
    {
        phy_driver_set(&phy_drv_i2c_phy);
        switch (phy_drv->phy_type)
        {
            case PHY_TYPE_SF2_SERDES:
                if( opticaldet_get_sgmii_i2c_bus_num(&bus_num) == OPTICALDET_NOBUS) {
                    printk("Error: ***** No I2C bus number defined for %s Serdes addr %d.\n",
                            phy_get_speed_string(phy_serdes->highest_speed), phy_dev->addr);
                    printk("              Wrong board parameters or wrong board design.\n");
                    BUG();
                }
                break;
            case PHY_TYPE_XGAE:
                if( opticaldet_get_xpon_i2c_bus_num(&bus_num) == OPTICALDET_NOBUS) {
                    printk("Error: ***** No I2C bus number defined for %s Serdes addr %d.\n",
                            phy_get_speed_string(phy_serdes->highest_speed), phy_dev->addr);
                    printk("              Wrong board parameters or wrong board design.\n");
                    BUG();
                }
                break;
            default:
                printk(" Unknown Serdes Type: %d\n", phy_drv->phy_type);
                BUG();
        }
        i2c_phy = phy_dev_add(PHY_TYPE_I2C_PHY, bus_num, 0);

        phy_dev->cascade_next = i2c_phy;
        i2c_phy->cascade_prev = phy_dev;
        i2c_phy->flag = PHY_FLAG_NOT_PRESENTED|PHY_FLAG_DYNAMIC;
        i2c_phy->phy_drv->bus_drv = bus_drv_get(BUS_TYPE_DSL_I2C);

        if (phy_i2c_num >= ARRAY_SIZE(phy_i2c_priv)) {
            printk(" ********* Error: Too many I2C PHYs defined\n");
            BUG();
        }
        i2c_phy->priv = &phy_i2c_priv[phy_i2c_num++];
    }
#endif
/* Get SFP MOD_ABS GPIO definition */
    if (!(phy_dev->cascade_next->flag & PHY_FLAG_NOT_PRESENTED)) {   /* Copper PHY should not check LOS */
        phy_serdes->signal_detect_gpio = -1;
        phy_serdes->sfp_module_detect_gpio = -1;
    }
    else {
        if (BpGetSfpDetectGpio(&phy_serdes->sfp_module_detect_gpio) == BP_SUCCESS
#if defined(CONFIG_BP_PHYS_INTF)
                || BpGetSfpModDetectGpio(phy_serdes->phy_type == PHY_TYPE_SF2_SERDES? BP_INTF_TYPE_SGMII: BP_INTF_TYPE_xPON,
                    0, &phy_serdes->sfp_module_detect_gpio) == BP_SUCCESS
#endif
           ) {
            phy_serdes->sfp_module_detect_gpio &= BP_GPIO_NUM_MASK;
        }

        /* Get LOS GPIO definition */
        if (BpGetSgmiiGpios(&phy_serdes->signal_detect_gpio) == BP_SUCCESS
#if defined(CONFIG_BP_PHYS_INTF)
                || BpGetSfpSigDetect(phy_serdes->phy_type == PHY_TYPE_SF2_SERDES? BP_INTF_TYPE_SGMII: BP_INTF_TYPE_xPON,
                    0, &phy_serdes->signal_detect_gpio) == BP_SUCCESS
#endif
           ) {
            phy_serdes->signal_detect_invert = (phy_serdes->signal_detect_gpio & BP_ACTIVE_LOW) > 0;
            phy_serdes->signal_detect_gpio &= BP_GPIO_NUM_MASK;
        }

        /* Validate MOD_ABS GPIO definition */
        if (phy_serdes->sfp_module_detect_gpio != -1) {
            printk("GPIO %d set as SFP MOD_ABS for %s Serdes addr %d module insertion detection\n",
                    phy_serdes->sfp_module_detect_gpio, phy_get_speed_string(phy_serdes->highest_speed),
                    phy_dev->addr);
            kerSysSetGpioDirInput(phy_serdes->sfp_module_detect_gpio);
        }
        else {
            if (!phy_dev->cascade_next || (phy_dev->cascade_next->flag & PHY_FLAG_NOT_PRESENTED)) {/* SFP(No Copper PHY) case */
/* old 63138 and 63148 have invalid board parameters defined; skp this checking */
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
                printk("Warning: ***** No GPIO Pin defined for %s Serdes addr %d SFP module insertion.\n",
                        phy_get_speed_string(phy_serdes->highest_speed), phy_dev->addr);
#else
                printk("Error: ***** No GPIO Pin defined for %s Serdes addr %d SFP module insertion.\n",
                        phy_get_speed_string(phy_serdes->highest_speed), phy_dev->addr);
                printk("              Wrong board parameters or wrong board design.\n");
                bug = 1;
#endif
            }
        }

        /* Validate LOS GPIO Definition */
        if (phy_serdes->signal_detect_gpio != -1) {
            printk("GPIO %d set as %s Serdes addr %d Loss Of Signal Detection\n",
                    phy_serdes->signal_detect_gpio, phy_get_speed_string(phy_serdes->highest_speed), phy_dev->addr);

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
            if (phy_serdes->signal_detect_gpio == 36)
                MISC->miscSGMIIFiberDetect = MISC_SGMII_FIBER_GPIO36;
            else
                MISC->miscSGMIIFiberDetect = 0;
#endif
        }
        else {
            if ((phy_drv->phy_type == PHY_TYPE_SF2_SERDES &&
                        (!phy_dev->cascade_next || (phy_dev->cascade_next->flag & PHY_FLAG_NOT_PRESENTED)))) { /* SFP(No Copper PHY) case */
                printk("Error: ****** No GPIO for Loss Of Signal Detection defined for %s Serdes addr %d.\n",
                        phy_get_speed_string(phy_serdes->highest_speed), phy_dev->addr);
                printk("              Wrong board parameters or wrong board design.\n");
                bug = 1;
            }
        }

        if (bug)
            BUG();
    }

end:
    phy_serdes->serdes_init(phy_dev);
    return ret;
}

#include "crossbar_dev.h"
phy_dev_t *crossbar_get_phy_by_type(int phy_type);
int crossbar_phys_in_one_group(phy_dev_t *phy1, phy_dev_t *phy2);
/*
    static void serdes_work_around(int phy_id)
    Serdes work around during 100FX for dependency on Serdes status.
    When link is down, if crossbar is connected to Copper external port and
    Serdes is configured as 100FX, then 100FX Serdes will NEVER link up.
    The work around is to place crossbar back to Fibre external port when
    Copper port link down.
    phy_id:
        -1: Initialize work around.
        Others: current link down PHY ID.
*/
void serdes_work_around(phy_dev_t *phy_dev)
{
#if defined(SWITCH_REG_SINGLE_SERDES_CNTRL)
    static phy_dev_t *crossbar_serdes_phy;
    phy_serdes_t *phy_serdes = crossbar_serdes_phy ? crossbar_serdes_phy->priv : NULL;

    if(phy_dev == NULL)    /* Init work around */
    {
        crossbar_serdes_phy = crossbar_get_phy_by_type(PHY_TYPE_SF2_SERDES);
        if (!crossbar_serdes_phy)
            return;
    }

    if (!crossbar_serdes_phy || !crossbar_phys_in_one_group(phy_dev, crossbar_serdes_phy) ||
        (phy_serdes->config_speed != PHY_SPEED_AUTO &&
        phy_serdes->config_speed != PHY_SPEED_100) ||
        phy_dev->phy_drv->phy_type == PHY_TYPE_SF2_SERDES)
    {
        /* If no Serdes in layout, link down PHY in different Switch Group from Serdes,
          no 100FX configured or Serdes is Link down, no action is taken */
        return;
    }

    crossbar_plat_select(-1,-1,-1, crossbar_serdes_phy);
#endif /* SERDES */
}
EXPORT_SYMBOL(serdes_work_around);

/* mask[] contains value to be read-modified-write */
static void config_serdes(phy_dev_t *phy_dev, u32 seq[], int seqSize)
{
    int i;
    u16 val, v16, mask;

    seqSize /= sizeof(seq[0]);
    for(i=0; i<seqSize; i+=2)
    {
        mask = seq[i+1] >> 16;
        val = seq[i+1] & 0xffff;

        if (mask == 0)
            ethsw_phy_exp_write(phy_dev, seq[i], val);
        else
        {
            ethsw_phy_exp_read(phy_dev, seq[i], &v16);
            v16 &= mask;
            v16 |= val & ~mask;
            ethsw_phy_exp_write(phy_dev, seq[i], v16);
        }
    }
}

static void ethsw_sfp_restore_from_power_saving(phy_dev_t *phy_dev);
static int ethsw_powerop_serdes(phy_dev_t *phy_dev, int powerLevel)
{
    u32 val32;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    u16 v16;

    if (powerLevel == phy_serdes->cur_power_level)
        return 0;

    val32 = *(u32 *)SWITCH_REG_SINGLE_SERDES_CNTRL;
    switch(powerLevel)
    {
        case SERDES_POWER_ON:
            val32 |= SWITCH_REG_SERDES_RESETPLL|SWITCH_REG_SERDES_RESETMDIO|SWITCH_REG_SERDES_RESET;
            val32 &= ~(SWITCH_REG_SERDES_IDDQ|SWITCH_REG_SERDES_PWRDWN);
            *(u32 *)SWITCH_REG_SINGLE_SERDES_CNTRL = val32;
            msleep(1);
            val32 &= ~(SWITCH_REG_SERDES_RESETPLL|SWITCH_REG_SERDES_RESETMDIO|SWITCH_REG_SERDES_RESET);
            *(u32 *)SWITCH_REG_SINGLE_SERDES_CNTRL = val32;
            msleep(1);

            /* Do dummy MDIO read to work around ASIC problem */
            phy_bus_read(phy_dev, 0, &v16);
            ethsw_sfp_restore_from_power_saving(phy_dev);
            break;
        case SERDES_POWER_STANDBY:
            if (val32 & SWITCH_REG_SERDES_IDDQ) {
                val32 |= SWITCH_REG_SERDES_PWRDWN;
                val32 &= ~SWITCH_REG_SERDES_IDDQ;
                *(u32 *)SWITCH_REG_SINGLE_SERDES_CNTRL = val32;
                msleep(1);
            }
            // note lack of break here
        case SERDES_POWER_DOWN:
            val32 |= SWITCH_REG_SERDES_PWRDWN|SWITCH_REG_SERDES_RESETPLL|
                SWITCH_REG_SERDES_RESETMDIO|SWITCH_REG_SERDES_RESET;
            *(u32 *)SWITCH_REG_SINGLE_SERDES_CNTRL = val32;
            break;
        case SERDES_POWER_DOWN_FORCE:
            val32 |= SWITCH_REG_SERDES_PWRDWN|SWITCH_REG_SERDES_RESETPLL|
                SWITCH_REG_SERDES_RESETMDIO|SWITCH_REG_SERDES_RESET|
                SWITCH_REG_SERDES_IDDQ;
            *(u32 *)SWITCH_REG_SINGLE_SERDES_CNTRL = val32;
            break;
        default:
            printk("Wrong power level request to Serdes module\n");
            return 0;
    }

	if(powerLevel != SERDES_POWER_ON)
    	phy_serdes->current_speed = -1;
    phy_serdes->cur_power_level = powerLevel;
    return 0;
}

static int ethsw_phy_enable_an(phy_dev_t *phy_dev)
{
    u16 val16 = 0;

    phy_bus_read(phy_dev, MII_CONTROL, &val16);

    if (val16 & MII_CONTROL_AN_ENABLE)
        return 0;

    val16 |= MII_CONTROL_AN_ENABLE|MII_CONTROL_RESTART_AUTONEG;
    phy_bus_write(phy_dev, MII_CONTROL, val16);
    msleep(50);
    return 1;
}

static void ethsw_config_serdes_1kx(phy_dev_t *phy_dev)
{
    ethsw_sgmii_serdes_reset(phy_dev);
#if defined(PHY_SERDES_2P5G_CAPABLE)
    config_serdes(phy_dev, serdesRef50mVco6p25, sizeof(serdesRef50mVco6p25));
    msleep(1);
#endif
    config_serdes(phy_dev, serdesSet1GFiber, sizeof(serdesSet1GFiber));
    sf2_config_los(phy_dev);
}

static void inline ethsw_config_serdes_forced_1kx(phy_dev_t *phy_dev)
{
    ethsw_config_serdes_1kx(phy_dev);
    msleep(50);
}

static void ethsw_config_serdes_100fx(phy_dev_t *phy_dev)
{
    ethsw_sgmii_serdes_reset(phy_dev);
#if defined(PHY_SERDES_2P5G_CAPABLE)
    config_serdes(phy_dev, serdesRef50mVco6p25, sizeof(serdesRef50mVco6p25));
    msleep(1);
#endif
    config_serdes(phy_dev, serdesSet100MFiber, sizeof(serdesSet100MFiber));
    sf2_config_los(phy_dev);
}

static void inline ethsw_config_serdes_forced_100fx(phy_dev_t *phy_dev)
{
    ethsw_config_serdes_100fx(phy_dev);
    msleep(20);
}

static void ethsw_config_serdes_auto_sgmii(phy_dev_t *phy_dev)
{
    config_serdes(phy_dev, serdesSet100MForcedSGMII, sizeof(serdesSet100MForcedSGMII));
    ethsw_phy_enable_an(phy_dev); /* do sleep inside */
}

static void ethsw_config_serdes_100M_forced_sgmii(phy_dev_t *phy_dev)
{
    config_serdes(phy_dev, serdesSet100MForcedSGMII, sizeof(serdesSet100MForcedSGMII));
    msleep(20);
}

#define SERDES_AUTO_DETECT_INT_MS 150
#define SERDES_FIBRL_SETTLE_DELAY_MS 200

/*
    The testing result shows lower speed will be easier to link up
   during the fibre insertion, thus we are doing retry of the highest
   speed when linked in non highest speed.
*/
#define PHY_CAP_SPEED_MASK ((PHY_CAP_10000 << 1) - 1)
#define IS_PHY_HIGHEST_SPEED_CAP(speed_caps, curSpeed) (!((speed_caps & (PHY_CAP_SPEED_MASK)) & (~((curSpeed<<1)-1))))
static void ethsw_serdes_speed_detect(phy_dev_t *phy_dev)
{
    static int retry = 0;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    u16 rnd;
    phy_speed_t speed;
    u32 speed_cap;
    
    if (phy_serdes->config_speed != PHY_SPEED_AUTO)
        return;

    /* Introduce random phase shift to get bigger chanse to link up on back to back connection */
    if (retry == 0)
    {
        get_random_bytes(&rnd, sizeof(rnd));
        msleep(1000 * (rnd%100) / 100);
    }

    if (retry == 0)
    {
        phy_serdes->link_stats(phy_dev);
        if (phy_dev->link) goto end;
    }

    for (speed = phy_serdes->highest_speed; speed != PHY_SPEED_AUTO; speed--)
    {
        speed_cap = phy_speed_to_caps(speed, PHY_DUPLEX_FULL);
        if (!(phy_serdes->speed_caps & speed_cap))
            continue;

        phy_serdes->speed_set(phy_dev, speed, PHY_DUPLEX_FULL);
        msleep(SERDES_AUTO_DETECT_INT_MS);
        phy_serdes->link_stats(phy_dev);
        if (phy_dev->link)
            goto LinkUp;

        if((phy_serdes->speed_caps & PHY_CAP_AUTONEG) && phy_serdes->enable_an)
        {
            if(phy_serdes->enable_an(phy_dev))
            {
                phy_serdes->link_stats(phy_dev);
                if (phy_dev->link)
                    goto LinkUp;
            }
        }
    }

    goto NoLinkUp;

LinkUp:
    if (phy_dev->speed == phy_serdes->highest_speed)
        goto end;

    if (retry) goto end; /* If we retried already, return; */

    /* Otherwise, take a sleep to let fibre settle down, then retry higher speed */
    retry++;
    msleep(SERDES_FIBRL_SETTLE_DELAY_MS);
    ethsw_serdes_speed_detect(phy_dev);
    goto end;

NoLinkUp:
    /*
        No link up here.
        Set speed to highest when in NO_POWER_SAVING_MODE until next detection
    */
    if( phy_serdes->power_mode == SERDES_NO_POWER_SAVING)
        phy_serdes->speed_set(phy_dev, phy_serdes->highest_speed, PHY_DUPLEX_FULL);
end:
    retry = 0;
}

int ethsw_serdes_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps)
{
    phy_sf2_priv_t *phy_sf2_priv = phy_dev->priv;

    if (caps_type == CAPS_TYPE_SUPPORTED)
        *caps = phy_sf2_priv->speed_caps;
    else if (caps_type == CAPS_TYPE_ADVERTISE)
    {
        if (phy_sf2_priv->config_speed == PHY_SPEED_AUTO)
            *caps = phy_sf2_priv->speed_caps|PHY_CAP_AUTONEG;
        else
            *caps = phy_speed_to_caps(phy_sf2_priv->config_speed, PHY_DUPLEX_FULL);
    }
    else
        *caps = 0;
    return 0;
}

static int ethsw_serdes_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    phy_dev_t *cascade_next = phy_dev->cascade_next;

    if (phy_serdes->current_speed == speed)
        return 0;

    phy_serdes->sgmii_mode = NONSGMII_MODE;
    if (phy_serdes->sfp_module_type == SFP_COPPER && speed <= PHY_SPEED_1000) {
        cascade_next->phy_drv->speed_set(cascade_next, speed, duplex);
        phy_serdes->sgmii_mode = SGMII_AUTO;
        ethsw_config_serdes_auto_sgmii(phy_dev);
    } else {
        switch ((int)speed)
        {
            case PHY_SPEED_AUTO:
                break;
#if defined(PHY_SERDES_2P5G_CAPABLE)
            case PHY_SPEED_2500:
                ethsw_config_serdes_2p5g(phy_dev);
                break;
#endif
            case PHY_SPEED_1000:
                ethsw_config_serdes_1kx(phy_dev);
                break;
            case PHY_SPEED_100:
                if (phy_dev->cascade_next && phy_dev->cascade_next->phy_drv->phy_type == PHY_TYPE_EXT3) /* 84881 */
                    ethsw_config_serdes_100M_forced_sgmii(phy_dev);
                else
                    ethsw_config_serdes_100fx(phy_dev);
                break;
            default:
                printk("Not supported speed: 0x%x\n", speed);
                return -1;
        }
    }

    phy_serdes->current_speed = speed;
    return 0;
}

int ethsw_serdes_cfg_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int rc = 0;

    mutex_lock(&serdes_mutex);

    if (speed != PHY_SPEED_AUTO &&
#if defined(PHY_SERDES_10G_CAPABLE)
            speed != PHY_SPEED_10000 &&
#endif
#if defined(PHY_SERDES_2P5G_CAPABLE)
            speed != PHY_SPEED_2500 &&
#endif
            speed != PHY_SPEED_1000 && speed != PHY_SPEED_100) {
        printk("Not supported speed: 0x%x\n", speed);
        rc = -1;
        goto end;
    }

    phy_serdes->config_speed = speed;
    phy_serdes->speed_set(phy_dev, speed, duplex);

end:
    mutex_unlock(&serdes_mutex);
    return rc;
}

static void ethsw_sfp_restore_from_power_saving(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if(phy_serdes->power_mode == SERDES_BASIC_POWER_SAVING &&
       (!phy_dev->cascade_next || phy_dev->cascade_next->flag & PHY_FLAG_NOT_PRESENTED))   /* MODULE is not plugged in */
        return;

    ethsw_sgmii_serdes_reset(phy_dev);

#if defined(PHY_SERDES_2P5G_CAPABLE)
    config_serdes(phy_dev, serdesRef50mVco6p25, sizeof(serdesRef50mVco6p25));
    msleep(1);
#endif

    if(phy_serdes->sfp_module_type == SFP_COPPER)
    {
        /* Configure Serdes into SGMII mode */
        phy_serdes->speed_set(phy_dev, phy_serdes->config_speed, PHY_DUPLEX_FULL);
    }
    else
    {
        phy_serdes->speed_set(phy_dev, phy_serdes->config_speed, PHY_DUPLEX_FULL);
    }
}

static void ethsw_2p5g_serdes_link_stats(phy_dev_t *phy_dev)
{
    uint16_t v16;

    phy_bus_read(phy_dev, MII_STATUS, &v16);
    phy_dev->link = (v16 & MII_STATUS_LINK) != 0;


    if(phy_dev->link)
    {
        ethsw_phy_exp_read(phy_dev, MIIEX_DIGITAL_STATUS_1000X, &v16);
        switch (v16 & MIIEX_SPEED)
        {
            case MIIEX_SPD2500:
                phy_dev->speed = PHY_SPEED_2500;
                break;
            case MIIEX_SPD1000:
                phy_dev->speed = PHY_SPEED_1000;
                break;
            case MIIEX_SPD100:
                phy_dev->speed = PHY_SPEED_100;
                break;
            case MIIEX_SPD10:
                phy_dev->speed = PHY_SPEED_10;
                break;
        }

        phy_dev->duplex = (v16 & MIIEX_DUPLEX)? PHY_DUPLEX_FULL: PHY_DUPLEX_HALF;
    }
}

static int ethsw_serdes_power_mode_set(phy_dev_t *phy_dev, int mode)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    mutex_lock(&serdes_mutex);

    if (phy_serdes->power_mode == mode)
        goto end;

    phy_serdes->power_mode = mode;

    if(mode == SERDES_NO_POWER_SAVING)
        ethsw_powerup_serdes(phy_dev);
    else if (mode == SERDES_FORCE_OFF)
        ethsw_powerdown_forceoff_serdes(phy_dev);
    else
        ethsw_powerdown_serdes(phy_dev);

end:
    mutex_unlock(&serdes_mutex);
    return 0;
}

void phy_drv_sfp_group_list(void)
{
    phy_serdes_t *phy_serdes;
    char *serdes_type;
    char *serdes_status;
    int i, sfp_serdeses = 0;
    phy_dev_t *phy_dev;

    for (i = 0; i < MAX_SERDES_NUMBER; i++) {
        if (!serdes[i].used) continue;

        phy_serdes = &serdes[i];
        phy_dev = phy_serdes->phy_dev;
        if (!(phy_dev->cascade_next->flag & PHY_FLAG_DYNAMIC)) continue;

        sfp_serdeses++;
        switch(phy_serdes->highest_speed_cap) {
            case PHY_CAP_10000:
                serdes_type = "10G AE Serdes";
                break;
            case PHY_CAP_2500:
                serdes_type = "2.5G Serdes";
                break;
            case PHY_CAP_1000_FULL:
                serdes_type = "1G Serdes";
                break;
            default:
                serdes_type = "unknown";
        }

        switch(phy_serdes->sfp_status) {
            case SFP_MODULE_OUT:
                serdes_status = "unplug";
                break;
            case SFP_MODULE_IN:
                serdes_status = "plug in";
                break;
            case SFP_LINK_UP:
                serdes_status = "link up";
                break;
            default:
                serdes_status = "unkown";
                break;
        }

        if (sfp_serdeses == 1)
            printk("%-16s %-16s %-16s\n", "Serdes ID", "Serdes Type", "SFP Status");
        if (phy_serdes->phy_type == PHY_TYPE_XGAE)
            printk("%-16s %-16s %-16s\n", "N/A", serdes_type, serdes_status);
        else
            printk("%-16d %-16s %-16s\n", phy_serdes->phy_dev->addr, serdes_type, serdes_status);
    }

    if (sfp_serdeses == 0)
        printk(" No SFP design in this board\n");
}

static int phy_i2c_save_registers[] = {0, 4, 9, 0x1b};
/*
    When we are in AutoDetection mode, configure SFP as Fiber mode/AN all speed,
    so that any speed can be matched without changing SFP configuration taking very long time.
    When we are configured fixed speed mode, configure SFP module in Fiber/AN specific mode.
*/
static int ethsw_conf_copper_sfp(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    uint16_t v16;
    phy_i2c_priv_t *phy_i2c_priv = phy_dev->priv;
    phy_serdes_t *phy_serdes = phy_dev->cascade_prev->priv;
    int i;

    if (phy_i2c_priv->saved == 0) { /* Save default I2C PHY values for X-BaseX mode */
        for (i=0; i<ARRAY_SIZE(phy_i2c_save_registers); i++) {
            phy_bus_read(phy_dev, phy_i2c_save_registers[i], &v16);
            phy_i2c_priv->saved_registers[i] = v16;
        }
        phy_i2c_priv->saved = 1;
    }

    phy_i2c_priv->current_speed = speed;
    phy_i2c_priv->config_speed = phy_serdes->config_speed;
    if (speed > PHY_SPEED_1000) {
        if (phy_i2c_priv->sgmii_mode == NONSGMII_MODE)
            return 0;
        phy_i2c_priv->sgmii_mode = NONSGMII_MODE;
        /* Restore default I2C PHY values for X-BaseX Fiber and AN mode */
        for (i=0; i<ARRAY_SIZE(phy_i2c_save_registers); i++)
            phy_bus_write(phy_dev, phy_i2c_save_registers[i], phy_i2c_priv->saved_registers[i]);
    }
    else if (phy_serdes->config_speed == PHY_SPEED_AUTO) {
        /* Configure SFP PHY into SGMII mode */
        if (phy_i2c_priv->sgmii_mode == SGMII_AUTO)
            return 0;

        phy_bus_write(phy_dev, 0x1b, 0x9084);    /* Enable SGMII mode */
        phy_bus_write(phy_dev, MII_CONTROL, 0x8000);
        msleep(100);
        phy_i2c_priv->sgmii_mode = SGMII_AUTO;
        phy_bus_write(phy_dev, 0x9, 0x0f00);     /* Advertise 1kBase-T Full/Half-Duplex */
        phy_bus_write(phy_dev, 0x4, 0x0de1);     /* Adverstize 100/10Base-T Full/Half-Duplex */
        phy_bus_write(phy_dev, MII_CONTROL, 0x1140);
    } else {  /* Fixed speed configuration */
        phy_bus_write(phy_dev, 0x1b, 0x9084);    /* Enable SGMII mode */
        phy_i2c_priv->sgmii_mode = SGMII_FORCE;
        v16 = (speed==PHY_SPEED_1000)? MII_CONTROL_SPEED_1000: MII_CONTROL_SPEED_100;
        v16 |= MII_CONTROL_DUPLEX_MODE;
        phy_bus_write(phy_dev, MII_CONTROL, v16);
    }

    /* Do a final PHY reset to make configuration valid */
    phy_bus_read(phy_dev, MII_CONTROL, &v16);
    phy_bus_write(phy_dev, MII_CONTROL, v16|MII_CONTROL_RESET);
    msleep(100);
    return 0;
}

static uint32_t ethsw_get_sfp_module_caps(phy_dev_t *phy_dev)
{
    uint32_t speed_caps = PHY_CAP_AUTONEG;
#if defined(CONFIG_I2C) && defined(CONFIG_BCM_OPTICALDET)
    uint8_t val;

    bcmsfp_read_byte(phy_dev->addr, SFP_CLIENT_EEPROM, 3, &val);
    if (val & 0xf0)
        speed_caps |= PHY_CAP_10000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL;

    
    bcmsfp_read_byte(phy_dev->addr, SFP_CLIENT_EEPROM, 6, &val);
    if (val & 0xf)
        speed_caps |= PHY_CAP_1000_FULL|PHY_CAP_100_FULL;
#endif

    return speed_caps;
}

#define I2C_RESCAN_WAIT_MS 1500
static int sfp_i2c_module_detect(phy_dev_t *phy_dev)
{
    uint16_t v16;
    phy_dev_t *phy_i2c = phy_dev->cascade_next;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    phy_i2c_priv_t *phy_i2c_priv = phy_i2c->priv;

    if (phy_i2c == NULL) return 0;

#if defined(CONFIG_I2C) && defined(CONFIG_BCM_OPTICALDET)
    /* check if SFP i2c driver is fully initialized first */
    if( bcm_i2c_sfp_get_status(phy_i2c->addr) != SFP_STATUS_INSERTED ) {
        /* sfp module is detected but bcmsfp driver is not finished initializing yet.
         continue to wait */
         return 2;
    }
    bcm_i2c_sfp_rescan(phy_dev->addr);
    msleep(I2C_RESCAN_WAIT_MS);
#endif

    /* If I2C read operation succeeds, I2C module is connected
       and which means it is a copper SFP module */
    if (phy_bus_read(phy_i2c, MII_CONTROL, &v16) == 0 )
    {
        phy_serdes->sfp_module_type = SFP_COPPER;
        phy_i2c->flag &= ~PHY_FLAG_NOT_PRESENTED;
        ethsw_powerdown_serdes(phy_dev);  /* Power down and up Serdes to reset Serdes Status */
        ethsw_powerup_serdes(phy_dev);
        phy_serdes->speed_set(phy_dev, phy_serdes->config_speed, PHY_DUPLEX_FULL);
        phy_i2c->phy_drv->speed_set(phy_i2c, phy_serdes->config_speed, PHY_DUPLEX_FULL);
        phy_i2c_priv->speed_caps = ethsw_get_sfp_module_caps(phy_i2c);
        return 1;
    }

    phy_i2c->flag |= PHY_FLAG_NOT_PRESENTED;
    return 0;
}

static int ethsw_sfp_module_detected(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (phy_serdes->sfp_module_detect_gpio != -1)
    {
        return kerSysGetGpioValue(phy_serdes->sfp_module_detect_gpio) == 0;
    }
    else
    {
        return ((*(u32*)SWITCH_SINGLE_SERDES_STAT) & SWITCH_REG_SSER_RXSIG_DET) > 0;
    }
}

static int ethsw_sfp_light_detected(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (phy_serdes->signal_detect_gpio == -1)   /* If LOS is not configured, always return true */
    {
        return 1;
    }
    else
    {
        return ((*(u32*)SWITCH_SINGLE_SERDES_STAT) & SWITCH_REG_SSER_RXSIG_DET) > 0;
    }
}

/*
   Module detection is not going through SGMII,
   so it can be done even under SGMII power down.
 */
static int ethsw_sfp_module_detect(phy_dev_t *phy_dev)
{
    int rc;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    serdes_sfp_type_t sfp_module_type;
    int sfp_module_detected;
    TRX_TYPE trx_type = TRX_TYPE_ETHERNET;
    uint32_t speed_caps;

    // don't do module detect when connected to external switch
    if (IsPortConnectedToExternalSwitch(phy_dev->meta_id))
        return 1;

    if (phy_dev->cascade_next && phy_serdes->i2cInitDetectDelay++ < I2CInitDetectDelay)
        return 0;

    sfp_module_detected = ethsw_sfp_module_detected(phy_dev);
#if defined(CONFIG_I2C) && defined(CONFIG_BCM_OPTICALDET)
    if (sfp_module_detected && phy_serdes->sfp_module_type == SFP_NO_MODULE)
        msleep(100);    /* Let I2C driver prepare data */

    rc = trx_get_type(phy_dev->cascade_next->addr, &trx_type);

    if (sfp_module_detected && rc == OPTICALDET_NOSFP)  /* I2C driver not sync with us yet */
        sfp_module_detected = 0;
#endif

    if ( !sfp_module_detected || trx_type == TRX_TYPE_XPON)
    {
        if(phy_serdes->sfp_module_type > SFP_GPON_MODULE)
        {
            if (phy_dev->cascade_next)
                phy_dev->cascade_next->flag |= PHY_FLAG_NOT_PRESENTED;
        }

        if(!sfp_module_detected)
            sfp_module_type = SFP_NO_MODULE;
        else
            sfp_module_type = SFP_GPON_MODULE;

        if (phy_serdes->sfp_module_type == sfp_module_type)
            return 0;

        if(!sfp_module_detected)
        {
            printk("SFP Module is Unplugged\n");
            sf2_serdes_sfp_lbe_op(phy_dev, LASER_OFF);
        }
        else
            printk("GPON SFP Module is Plugged in\n");

        phy_serdes->sfp_module_type = sfp_module_type;
        return 0;
    }

    sf2_serdes_sfp_lbe_op(phy_dev, LASER_ON);
    if (phy_serdes->sfp_module_type <= SFP_GPON_MODULE)
    {
        /* This is a dynamic PHY */
        rc = sfp_i2c_module_detect(phy_dev);
        if( rc == 1 )
        {
            phy_dev_caps_get(phy_dev->cascade_next, CAPS_TYPE_SUPPORTED, &speed_caps);
            printk("%dG Sgmii Copper SFP Module is Plugged in\n", (speed_caps & PHY_CAP_10000)? 10: 1);
        }
        else if( rc == 0 )
        {
            phy_serdes->sfp_module_type = SFP_FIBER;
            if (phy_dev->cascade_next) phy_serdes->i2cDetectDelay = 0;
            printk("XBase-X SFP Module is Plugged in\n");
        }
        /* otherwise continue to wait for sfp i2c driver to finish the initialization */
    }
#ifdef ADDITIONAL_I2C_SFP_DETECT
    /* this is probably no longer needed as sfp_i2c_module_detect continue to wait until sfp driver
       init is done. */
    else if(!(phy_dev->cascade_next->flag & PHY_FLAG_NOT_PRESENTED))   /* MODULE is plug in */
    {
        /* Work around for some I2C long initialization, do more I2C after Fiber type detected */
        if (phy_serdes->sfp_module_type == SFP_FIBER &&
                phy_serdes->i2cDetectDelay < I2CDetectDelay)
        {
            if (sfp_i2c_module_detect(phy_dev) == 1)
            {
                phy_serdes->sfp_module_type = SFP_COPPER;
                phy_serdes->i2cDetectDelay = I2CDetectDelay;
                printk("Copper SFP Module Detected\n");
            }
            else
            {
                phy_serdes->i2cDetectDelay++;
            }
        }
    }
#endif
    return 1;
}

int sf2_serdes_phy_read_status(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int speed_caps;
    phy_speed_t highest_speed;
    
    mutex_lock(&serdes_mutex);

    if (!phy_serdes->power_admin_on)
    {
        phy_dev->link = 0;
        goto sfp_end;
    }

    if (phy_serdes->power_mode > SERDES_NO_POWER_SAVING && phy_serdes->sfp_status < SFP_LINK_UP)
        ethsw_powerstandby_serdes(phy_dev);

    switch (phy_serdes->sfp_status)
    {
        case SFP_MODULE_OUT:
sfp_module_out:
            if(phy_serdes->sfp_status == SFP_MODULE_OUT && ethsw_sfp_module_detect(phy_dev))
                goto sfp_module_in;

            phy_serdes->sfp_status = SFP_MODULE_OUT;
            goto sfp_end;

        case SFP_MODULE_IN:
sfp_module_in:
            if(phy_serdes->sfp_status >= SFP_MODULE_IN && !ethsw_sfp_module_detect(phy_dev))
            {
                phy_serdes->sfp_status = SFP_MODULE_IN;
                goto sfp_module_out;
            }

            if(phy_serdes->sfp_status <= SFP_MODULE_IN)
            {
                if(phy_serdes->power_mode == SERDES_BASIC_POWER_SAVING)
                {
                    ethsw_powerup_serdes(phy_dev);
                }
                else if (phy_serdes->power_mode == SERDES_NO_POWER_SAVING)
                {
                    if (!phy_serdes->light_detected(phy_dev))
                        goto sfp_end;

                    if (phy_serdes->config_speed == PHY_SPEED_AUTO)
                        ethsw_serdes_speed_detect(phy_dev);
                }

                phy_serdes->link_stats(phy_dev);
                if (phy_dev->link)
                {
                    phy_serdes->sfp_status = SFP_MODULE_IN;
                    goto sfp_link_up;
                }
            }
            phy_serdes->sfp_status = SFP_MODULE_IN;
            goto sfp_end;

        case SFP_LINK_UP:
sfp_link_up:
            if(phy_serdes->sfp_status == SFP_LINK_UP)
            {

                if(!ethsw_sfp_module_detect(phy_dev))
                {
                    phy_dev->link = 0;
                    goto sfp_module_out;
                }
                phy_serdes->link_stats(phy_dev);
                if(!phy_dev->link)
                {
                    /* If link goes down, we must put the link to the highest.
                        because link detection assume the phy is in highest speed when
                        link is down and will check link before doing different speed config */
                    cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_SUPPORTED, &speed_caps);
                    highest_speed = phy_caps_to_max_speed(speed_caps);
                    if(phy_serdes->config_speed == PHY_SPEED_AUTO)
                        phy_serdes->speed_set(phy_dev, highest_speed, PHY_DUPLEX_FULL);
                    goto sfp_module_in;
                }
            }
            phy_serdes->sfp_status = SFP_LINK_UP;
            goto sfp_end;
    }

sfp_end:
    if( phy_serdes->power_admin_on && phy_serdes->power_mode > SERDES_NO_POWER_SAVING &&
        phy_serdes->sfp_status != SFP_LINK_UP)
        ethsw_powerdown_serdes(phy_dev);

    mutex_unlock(&serdes_mutex);
    return 0;
}

int ethsw_serdes_power_mode_get(phy_dev_t *phy_dev, int *mode)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    mutex_lock(&serdes_mutex);
    *mode = phy_serdes->power_mode;
    mutex_unlock(&serdes_mutex);
    return 0;
}

int ethsw_serdes_speed_get(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    mutex_lock(&serdes_mutex);
    *speed = phy_serdes->config_speed;
    *duplex = PHY_DUPLEX_FULL;
    mutex_unlock(&serdes_mutex);
    return 0;
}

static int sf2_serdes_power_get(phy_dev_t *phy_dev, int *enable)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    *enable = phy_serdes->power_admin_on;
    return 0;
}

static int sf2_serdes_power_set(phy_dev_t *phy_dev, int enable)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    phy_serdes->power_admin_on = enable > 0;

    if(enable) {
        if(phy_serdes->power_mode == SERDES_NO_POWER_SAVING)
            ethsw_powerup_serdes(phy_dev);
    }
    else
        ethsw_powerdown_serdes(phy_dev);

    return 0;
}

static int sf2_serdes_apd_get(phy_dev_t *phy_dev, int *enable)
{
    phy_dev_t *cur;
    int tmp;

    *enable = 0;
    ethsw_serdes_power_mode_get(phy_dev, &tmp);
    if (tmp != SERDES_BASIC_POWER_SAVING)
        return 0;

    tmp = 1;
    for (cur = phy_dev->cascade_next; cur && tmp; cur = cur->cascade_next)
        phy_dev_apd_get(cur, &tmp);

    *enable = tmp;
    return 0;
}

static int sf2_serdes_apd_set(phy_dev_t *phy_dev, int enable)
{
    static int init = 0;

    /* We want to force default APD value to stay in driver default value
        but not global APD default value for stability */
    if (!init) {
        init = 1;
        return 0;
    }

    ethsw_serdes_power_mode_set(phy_dev,
        enable ? SERDES_BASIC_POWER_SAVING : SERDES_NO_POWER_SAVING);

    return 0;
}

static int sf2_serdes_eee_set(phy_dev_t *phy_dev, int enable)
{
    return 0;
}

static int sf2_serdes_eee_get(phy_dev_t *phy_dev, int *enable)
{
    phy_dev_t *cur;
    int tmp = 1;

    for (cur = phy_dev->cascade_next; cur && tmp; cur = cur->cascade_next)
        phy_dev_eee_get(cur, &tmp);

    *enable = tmp;
    return 0;
}

static int sf2_serdes_eee_resolution_get(phy_dev_t *phy_dev, int *enable)
{
    phy_dev_t *cur = phy_dev->cascade_next;
    int tmp;

    *enable = 0;
    if (!cur)
        return 0;

    do {
        phy_dev_eee_resolution_get(cur, &tmp);
        cur = cur->cascade_next;
    } while (cur && tmp);

    *enable = tmp;
    return 0;
}

static int sf2_serdes_cfg_speed_set(phy_dev_t *phy_dev, phy_speed_t speed,
                phy_duplex_t duplex)
{
    int enabled;

    phy_dev_power_get(phy_dev, &enabled);
    if (!enabled)
        return -1;

    return ethsw_serdes_cfg_speed_set(phy_dev, speed, PHY_DUPLEX_FULL);
}

static  int ethsw_serdes_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    phy_speed_t speed;

    if (caps & PHY_CAP_AUTONEG)
        speed = PHY_SPEED_AUTO;
    else
        speed = phy_caps_to_max_speed(caps);
    return sf2_serdes_cfg_speed_set(phy_dev, speed, PHY_DUPLEX_FULL);
}

phy_drv_t phy_drv_sf2_serdes =
{
    .init = sf2_serdes_init,
    .phy_type = PHY_TYPE_SF2_SERDES,
    .read_status = sf2_serdes_phy_read_status,
    .power_get = sf2_serdes_power_get,
    .power_set = sf2_serdes_power_set,
    .apd_get = sf2_serdes_apd_get,
    .apd_set = sf2_serdes_apd_set,
    .eee_set = sf2_serdes_eee_set,
    .eee_get = sf2_serdes_eee_get,
    .eee_resolution_get = sf2_serdes_eee_resolution_get,
    .speed_set = sf2_serdes_cfg_speed_set,
    .caps_get = ethsw_serdes_caps_get,
    .caps_set = ethsw_serdes_caps_set,
    .isolate_phy = mii_isolate_phy,
    .config_speed_get = ethsw_serdes_speed_get,

    .name = "SERDES",
};

static phy_drv_t phy_drv_sf2_serdes_extsw =
{
    .init = sf2_serdes_init,
    .phy_type = PHY_TYPE_SF2_SERDES,
    .read_status = sf2_serdes_phy_read_status,
    .power_get = sf2_serdes_power_get,
    .caps_get = ethsw_serdes_caps_get,
    .config_speed_get = ethsw_serdes_speed_get,

    .name = "2_EXTSW",
};

