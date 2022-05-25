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

#include "phy_drv_dsl_phy.h"
#include <linux/pm.h>
#include <linux/reboot.h>
#include "clk_rst.h"

typedef struct phy_cl45_s {
    phy_dsl_priv_m;
    void *descriptor;
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
    {0x67c9de10, "GPY211", CL45PHY_BRCM2P5G_CAP},
    {0x67c9de00, "GPY211", CL45PHY_BRCM2P5G_CAP},
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

static int dsl_cl45phy_read_status(phy_dev_t *phy_dev);
static int dsl_cl45phy_set_speed(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
static int dsl_cl45phy_get_config_speed(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex);
static int dsl_cl45phy_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps);
static int dsl_cl45phy_caps_set(phy_dev_t *phy_dev, uint32_t caps);
static int dsl_cl45phy_loopback_set(phy_dev_t *phy_dev, int enable, phy_speed_t speed);
static int dsl_cl45phy_loopback_get(phy_dev_t *phy_dev, int *enable, phy_speed_t *speed);
static int dsl_cl45phy_set_wol_sleep(phy_dev_t *phy_dev, phy_wol_info *pwi);

#define MAX_CL45_PHYS 8
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
        phy_drv->set_wol_sleep = dsl_cl45phy_set_wol_sleep;
        phy_drv->an_restart = _phy_drv.an_restart;
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
// #ifdef __UBOOT__
#if 1
        if (!timeout){
            printk("Could not reset PHY. Your PHY rev is unsupported in u-boot.\n");
			return 0;
		}
#else
        // if (!timeout)
         //   BUG_CHECK("PHY at address %d failed to reset.", phy_dev->addr);
		return 0;
#endif
    }

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

        printk("%s:%d: disabling xtal_clk\n", __func__, __LINE__);
        bcm_disable_xtal_clk();
    }

    return 0;
}

static void PHY_BUS_READ(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg)
{
    u16 val = 0;
    phy_bus_c45_read(phy_dev, dev, reg, &val);
    printk("[%s(%d)] MMD %d.%d val=0x%x\n", __FUNCTION__, __LINE__, dev, reg, val);
}

int dsl_runner_gpy211_phy_init(phy_dev_t *phy_dev)
{
    static phy_cl45_t phys_cl45[MAX_CL45_PHYS];
    static int init;
    phy_drv_t *phy_drv = phy_dev->phy_drv;
    phy_cl45_t *phy_cl45;
    phy_drv_t _phy_drv = *phy_drv;
    uint32_t phy_id;
    int i;
    u16 val;

    if (init > (MAX_CL45_PHYS - 1))
    {
        printk(" Error: More CL45 PHY %d requeted than supported %d\n", init+1, MAX_CL45_PHYS);
        BUG();
    }

    if (init == 0)
    {
        memset(phy_drv, 0, sizeof(*phy_drv));

        phy_drv->read_status = dsl_cl45phy_read_status;
        phy_drv->phy_type = _phy_drv.phy_type;
        phy_drv->name = _phy_drv.name;
        phy_drv->initialized = _phy_drv.initialized;
        phy_drv->speed_set = dsl_cl45phy_set_speed;
        phy_drv->caps_get = _phy_drv.caps_get;
//        phy_drv->caps_set = _phy_drv.caps_set;
        phy_drv->caps_set = dsl_cl45phy_caps_set;
        phy_drv->config_speed_get = dsl_cl45phy_get_config_speed;
        phy_drv->phyid_get = _phy_drv.phyid_get;
        phy_drv->init = dsl_runner_gpy211_phy_init;
//        phy_drv->loopback_set = dsl_cl45phy_loopback_set;
//        phy_drv->loopback_get = dsl_cl45phy_loopback_get;
//        phy_drv->pair_swap_set = _phy_drv.pair_swap_set;
//        phy_drv->isolate_phy = _phy_drv.isolate_phy;
//        phy_drv->super_isolate_phy = _phy_drv.super_isolate_phy;
//        phy_drv->apd_get = _phy_drv.apd_get;
//        phy_drv->apd_set = _phy_drv.apd_set;
        phy_drv->eee_get = _phy_drv.eee_get;
        phy_drv->eee_set = _phy_drv.eee_set;
        phy_drv->eee_resolution_get = _phy_drv.eee_resolution_get;
//        phy_drv->cable_diag_run = _phy_drv.cable_diag_run;
        phy_drv->inter_phy_types_get = _phy_drv.inter_phy_types_get;
        phy_drv->configured_inter_phy_types_set = _phy_drv.configured_inter_phy_types_set;
        phy_drv->priv_fun = dsl_phy_exp_op;
        phy_drv->power_set = _phy_drv.power_set;
        phy_drv->power_get = _phy_drv.power_get;
        phy_drv->get_phy_name = _phy_drv.get_phy_name;
        phy_drv->current_inter_phy_type_get = _phy_drv.current_inter_phy_type_get;
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

    printk("[%s(%d)] phy addr=0x%x, phyid=0x%x\n", __FUNCTION__, __LINE__, phy_dev->addr, phy_id);

	/* not to reset */
#if 0
    /* reg 0.0 : STD_CTRL */
    phy_bus_c45_write(phy_dev, 0x0, 0x0, 0xb040);
    {
        u16 timeout = 4000;
        while (--timeout) {
            phy_bus_c45_read(phy_dev, 0x0, 0x0, &val);
            if ((val & 0x8000) == 0)
                break;
            udelay(500);
        }

	if (!timeout){
		printk("Could not reset PHY. Your PHY rev is unsupported in u-boot.\n");
		return 0;
	}
    }
#endif

    cascade_phy_set_common_inter_types(phy_dev->cascade_prev);
    phy_dev_configured_inter_phy_types_set(phy_dev, INTER_PHY_TYPE_UP, phy_dev->common_inter_phy_types);
//    phy_dev_pair_swap_set(phy_dev, phy_dev->swap_pair);
    phy_dev_speed_set(phy_dev, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);
//    phy_dev_super_isolate_phy(phy_dev, 0);
    phy_dev->an_enabled = 1; /* We always use AN even for specific speed */

    return 0;
}

static void GPY211_SET_SPEED(phy_dev_t *phy_dev, int speed, uint32_t caps)
{
	/*
		check flow as below
		0.4  : 100M
		0.9  : 1G
		7.32 : 2.5G
		1.0  : PMAPMD
		30.8 : VSPEC1_SGMII_CTRL
		0.0  : PHY STD_CTRL and auto-nego (7.0 bit 9 should mirror to 0.0 bit 9)
	*/

	/*
		disable SGMII-ANEG mode due to BRCM can't support this, it will make SGMII / serdes can't work
		phy_bus_c45_write(phy_dev, 0x1e, 0x08, 0x24d9); // disable SGMII-ANEG mode / 2500 BaseX
		phy_bus_c45_write(phy_dev, 0x1e, 0x08, 0x24da); // disable SGMII-ANEG mode / 2.5G SGMII
	*/

	if (speed == PHY_SPEED_AUTO) {
		phy_bus_c45_write(phy_dev, 0x00, 0x04, 0x0de1); // 100M
		phy_bus_c45_write(phy_dev, 0x00, 0x09, 0x0200); // 1G
		phy_bus_c45_write(phy_dev, 0x07, 0x20, 0x40a2); // 2.5G
		phy_bus_c45_write(phy_dev, 0x01, 0x00, 0x2058); // PMAPMD
		phy_bus_c45_write(phy_dev, 0x00, 0x00, 0x3240); // PHY STD_CTRL and auto-nego
		printk("[%s(%d)] auto-nego mode\n", __FUNCTION__, __LINE__);
	}
	else if (speed == PHY_SPEED_2500) {
		if (!(caps & PHY_CAP_2500)) {
			printk("[%s(%d)] not support, caps=0x%x\n", __FUNCTION__, __LINE__, caps);
			return;
		}
		phy_bus_c45_write(phy_dev, 0x00, 0x04, 0x0c01); // 100M
		phy_bus_c45_write(phy_dev, 0x00, 0x09, 0x0000); // 1G
		phy_bus_c45_write(phy_dev, 0x07, 0x20, 0x40a2); // 2.5G
		phy_bus_c45_write(phy_dev, 0x01, 0x00, 0x2058); // PMAPMD
		phy_bus_c45_write(phy_dev, 0x00, 0x00, 0x3240); // PHY STD_CTRL and auto-nego
		printk("[%s(%d)] force 2500M\n", __FUNCTION__, __LINE__);
	}
	else if (speed == PHY_SPEED_1000) {
		phy_bus_c45_write(phy_dev, 0x00, 0x04, 0x0de1); // 100M
		phy_bus_c45_write(phy_dev, 0x00, 0x09, 0x0200); // 1G
		phy_bus_c45_write(phy_dev, 0x07, 0x20, 0x4002); // 2.5G
		phy_bus_c45_write(phy_dev, 0x01, 0x00, 0x0058); // PMAPMD
		phy_bus_c45_write(phy_dev, 0x00, 0x00, 0x1340); // PHY STD_CTRL and auto-nego
		printk("[%s(%d)] force 1000M\n", __FUNCTION__, __LINE__);
	}
	else if (speed == PHY_SPEED_100) {
		phy_bus_c45_write(phy_dev, 0x00, 0x04, 0x0de1); // 100M
		phy_bus_c45_write(phy_dev, 0x00, 0x09, 0x0000); // 1G
		phy_bus_c45_write(phy_dev, 0x07, 0x20, 0x4002); // 2.5G
		phy_bus_c45_write(phy_dev, 0x01, 0x00, 0x2018); // PMAPMD
		phy_bus_c45_write(phy_dev, 0x00, 0x00, 0x3300); // PHY STD_CTRL and auto-nego
		printk("[%s(%d)] force 100M\n", __FUNCTION__, __LINE__);
	}
}

static int dsl_cl45phy_set_speed(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    u16 v16;
    phy_cl45_t *phy_cl45 = phy_dev->priv;
    u32 caps;
    uint16_t val;
 
    printk("[%s(%d)] name=%s, speed=%d, duplex=%d, config_speed=%d\n", __FUNCTION__, __LINE__, phy_dev->phy_drv->name, speed, duplex, phy_cl45->config_speed);
    if (speed == phy_cl45->config_speed) return 0;

    cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_SUPPORTED, &caps);

    if (!strcmp(phy_dev->phy_drv->name, "GPY211")) {
	GPY211_SET_SPEED(phy_dev, speed, caps);
	phy_dev->an_enabled = 1;
    }
    else { // NON-GPY211
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
    } // NON-GPY211

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

    if (caps_type == CAPS_TYPE_SUPPORTED)
    {
        phy_bus_c45_read32(phy_dev, CL45_REG_PMA_PMD_EXT_ABL, &v16);

        *caps = PHY_CAP_AUTONEG;
        if (v16 & CL45_REG_CAP_5G)          *caps |= PHY_CAP_5000;
        if (v16 & CL45_REG_CAP_2P5G)        *caps |= PHY_CAP_2500;
        if (v16 & CL45_REG_CAP_100M)        *caps |= PHY_CAP_100_FULL|PHY_CAP_100_HALF;
        if (v16 & CL45_REG_CAP_1G)          *caps |= PHY_CAP_1000_FULL|PHY_CAP_1000_HALF;
        if (v16 & CL45_REG_CAP_10G)         *caps |= PHY_CAP_10000;
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
    int cur_loopback, caps;
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

void phy_shortfin_short_amble_workaround(phy_dev_t *phy_dev);
static int is_first = 1;
static int dsl_cl45phy_read_status(phy_dev_t *phy_dev)
{
    u16 v16;
    int mode, duplex;
    phy_cl45_t *phy_cl45 = phy_dev->priv;
    phy_speed_t org_speed = phy_dev->speed;
    int org_link = phy_dev->link;

    if (phy_cl45->link_changed)
    {
        phy_dev->link = 0;
        phy_cl45->link_changed = 0;
        return 0;
    }

    if (!strcmp(phy_dev->phy_drv->name, "GPY211")) {
    phy_bus_c45_read(phy_dev, 0x0, 0x1, &v16);
    phy_dev->link = ((v16 >> 2) & 0x1);

    //printk("[%s(%d)] name=%s, link=%d, link_changed=%d, is_first=%d\n", __FUNCTION__, __LINE__, phy_dev->phy_drv->name, phy_dev->link, phy_cl45->link_changed, is_first);

    /* bugfix : at first time bootup, phy link can't work, need to reset gpy211 again */
    if (!phy_dev->link && is_first) {
	/*
		disable SGMII-ANEG mode due to BRCM can't support this, it will make SGMII / serdes can't work
		phy_bus_c45_write(phy_dev, 0x1e, 0x08, 0x24d9); // disable SGMII-ANEG mode / 2500 BaseX
		phy_bus_c45_write(phy_dev, 0x1e, 0x08, 0x24da); // disable SGMII-ANEG mode / 2.5G SGMII
	*/
	phy_bus_c45_write(phy_dev, 0x1e, 0x08, 0x24d9); // disable SGMII-ANEG mode / 2500 BaseX
	phy_bus_c45_write(phy_dev, 0x00, 0x00, 0x3240); // PHY STD_CTRL and auto-nego
        is_first = 0;
    }

    phy_bus_c45_read(phy_dev, 0x0, 0x18, &v16);
    mode = ((v16 & 0x7u) >> 0);
    if (mode & 0x1) {
        phy_dev->speed  = PHY_SPEED_100;
    }
    else if (mode & 0x2) {
        phy_dev->speed  = PHY_SPEED_1000;
    }
   else if (mode & 0x4) {
       phy_dev->speed  = PHY_SPEED_2500;
   }

    phy_dev->duplex = PHY_DUPLEX_FULL;

    phy_dev->pause_rx = 0;
    phy_dev->pause_tx = 0;

    }
    else {
    phy_bus_c45_read32(phy_dev, CL45_REG_UDEF_STATUS, &v16);
    phy_dev->link = (v16 & CL45_UDEF_STATUS_COPPER_LINK)> 0;

    switch (v16 & CL45_UDEF_STATUS_COPPER_SPD_M)
    {
        case CL45_UDEF_STATUS_COPPER_SPD_10G:
            phy_dev->speed = PHY_SPEED_10000;
            break;
        case CL45_UDEF_STATUS_COPPER_SPD_5G:
            phy_dev->speed = PHY_SPEED_5000;
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
    }	/* NON-GPY211 */

    if (org_link && phy_dev->link && org_speed != phy_dev->speed)
        phy_dev->link = 0;

    if (phy_dev->link)
        phy_shortfin_short_amble_workaround(phy_dev);

    return 0;
}

static phy_dev_t *phy_dev_for_pm_power_off;
static void (*pm_power_off_saved) (void);

static void dsl_cl45phy_pm_power_off(void)
{
    if (!phy_dev_for_pm_power_off)
        return;

    printk("%s:%d sleeping for wake-on-lan\n", __func__, __LINE__);
    // set phy speed to 100Mbps to save more power
    dsl_cl45phy_set_speed(phy_dev_for_pm_power_off, PHY_SPEED_100, PHY_DUPLEX_HALF);
    phy_bus_c45_write32(phy_dev_for_pm_power_off, 0x1d800, 0x0c01);  // enable wake-on-lan 
    phy_bus_c45_write32(phy_dev_for_pm_power_off, 0x1a8ec, 0x401f);  // invert led4 to sleep

    // in case power off failed
    pm_power_off = pm_power_off_saved;
    dsl_cl45phy_set_speed(phy_dev_for_pm_power_off, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);
    phy_dev_for_pm_power_off = NULL;
    printk("%s:%d failed to power off. Reboot ...\n", __func__, __LINE__);
    orderly_reboot();
}

static int dsl_cl45phy_set_wol_sleep(phy_dev_t *phy_dev, phy_wol_info *pwi)
{
    phy_bus_c45_write32(phy_dev, 0x1d813, 0);  // clear wol intr mask
    phy_bus_c45_write32(phy_dev, 0x1d804,      // set wol pkt1 bits 15..0
        (pwi->mac_addr[4] << 8) | pwi->mac_addr[5]);
    phy_bus_c45_write32(phy_dev, 0x1d805,      // set wol pkt1 bits 31..16
        (pwi->mac_addr[2] << 8) | pwi->mac_addr[3]);
    phy_bus_c45_write32(phy_dev, 0x1d806,      // set wol pkt1 bits 47..32
        (pwi->mac_addr[0] << 8) | pwi->mac_addr[1]);
    phy_bus_c45_write32(phy_dev, 0x1d807, 0);  // set wol pkt2 bits 15..0
    phy_bus_c45_write32(phy_dev, 0x1d808, 0);  // set wol pkt2 bits 31..16
    phy_bus_c45_write32(phy_dev, 0x1d809, 0);  // set wol pkt2 bits 47..32

    phy_bus_c45_write32(phy_dev, 0x1a83b, 0xa000);  // set led4 on based on source
    phy_bus_c45_write32(phy_dev, 0x1a838, 0x100);   // set led4 source to wake-on-lan
    phy_bus_c45_write32(phy_dev, 0x1a839, 0);       // set led4 high mask
    phy_bus_c45_write32(phy_dev, 0x1a8f3, 0);       // set led4 mask source

    phy_dev_for_pm_power_off = phy_dev;
    pm_power_off_saved = pm_power_off;
    pm_power_off = dsl_cl45phy_pm_power_off;
    printk("%s:%d orderly powering off the system\n", __func__, __LINE__);
    orderly_poweroff(true);
    return 0;
}
