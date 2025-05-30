// SPDX-License-Identifier: GPL-2.0+
/*
      Copyright (c) 2016 Broadcom
      All Rights Reserved

 */


/*
 * GMAC driver for Star Fighter 2
 */

#include "mac_drv.h"
#include "mac_drv_sf2.h"
#include <board.h>
#include <linux/delay.h>
//#include "bcm_map_part.h"
//#include "bcm_misc_hw_init.h"

/* Only necessary for ether_gphy_reset */
//#include <linux/delay.h>
//#include <linux/slab.h>
//#include "bcm_gpio.h"

//#include <bcm/bcmswapitypes.h>
#include "../mii_shared.h"
#include "bcmmii.h"
#include "bcmmii_xtn.h"
//#include <linux/spinlock.h>
//#include "bcmswshared.h"

#define IMP_PORT_ID     8

static uintptr_t sf2_reg_base;
static uintptr_t sf2_core_base;

#define SWITCH_BASE                 (void *)sf2_core_base
#define SWITCH_REG_BASE             (void *)sf2_reg_base
#define SWITCH_DIRECT_DATA_RD_REG   (SWITCH_REG_BASE + 0x0000cUL)
#define SWITCH_DIRECT_DATA_WR_REG   (SWITCH_REG_BASE + 0x00008UL)

#if defined(CONFIG_BCM94908)
#define SF2_REG_SHIFT               1  
#else
#define SF2_REG_SHIFT               2  /* for 63158 & after */
#endif

void sf2_base_init(uintptr_t reg_base, uintptr_t core_base)
{
   sf2_reg_base = reg_base;
   sf2_core_base = core_base; 
}

#include "mdio_drv_sf2.h"
#define MDIO_RD(addr,reg,val) mdio_read_c22_register(addr, reg, val)
#define MDIO_WR(addr,reg,val) mdio_write_c22_register(addr, reg, val)

static DEFINE_SPINLOCK(sf2_reg_access);
static DEFINE_SPINLOCK(sf2_stat_access);
DEFINE_SPINLOCK(extsw_reg_config);
EXPORT_SYMBOL(extsw_reg_config);

void sf2_pseudo_mdio_switch_read(int page, int reg, void *data_out, int len)   //mdio
{
    // based on impl5\bcmsw_dma.c:bcmsw_pmdio_rreg()
    uint16_t val;
    uint64_t data64 = 0;
    uint16_t *data = (uint16_t *)&data64;
    int i;
    spin_lock_bh(&sf2_reg_access);

    val = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, val);

    val = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_READ;
    MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, val);

    for (i = 0; i < 20; i++) {
        MDIO_RD(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, &val);
        if ((val & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ)) == REG_PPM_REG17_OP_DONE)
            break;
        udelay(10);
    }

    if (i >= 20) {
        printk("sf2_rreg: mdio timeout!\n");
        spin_unlock_bh(&sf2_reg_access);
        return;
    }

    switch (len) {
    case 1:
        MDIO_RD(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &val);
        data[0] = (uint8_t)val; break;
    case 2:
        MDIO_RD(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &val);
        data[0] = val; break;
    case 4:
        MDIO_RD(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &val);
        data[0] = val;
        MDIO_RD(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &val);
        data[1] = val; break;
    case 6:
        MDIO_RD(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &val);
        data[0] = val;
        MDIO_RD(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &val);
        data[1] = val;
        MDIO_RD(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &val);
        data[2] = val; break;
    case 8:
        MDIO_RD(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &val);
        data[0] = val;
        MDIO_RD(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &val);
        data[1] = val;
        MDIO_RD(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &val);
        data[2] = val;
        MDIO_RD(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG27, &val);
        data[3] = val; break;
    default:
        printk("%s: len = %d NOT Handled !! \n", __FUNCTION__, len);
        break;
    }
    //printk("sf2_rreg(page=%x reg=%x len=%d %04x %04x %04x %04x)\n", page, reg, len, data[0], data[1], data[2],data[3]);
    spin_unlock_bh(&sf2_reg_access);
    memcpy(data_out, (void *)data, len);
}
EXPORT_SYMBOL(sf2_pseudo_mdio_switch_read);

void sf2_pseudo_mdio_switch_write(int page, int reg, void *data_in, int len)    //mdio
{
    // based on impl5\bcmsw_dma.c:bcmsw_pmdio_wreg()
    uint16_t val;
    uint64_t data64;
    uint16_t *data = (uint16_t *)&data64;
    int i;

    memcpy((void*)data, data_in, len);
    
    spin_lock_bh(&sf2_reg_access);

    val = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, val);

    switch (len) {
    case 1:
        val = (uint8_t)(data[0]);
        MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, val); break;
    case 2:
        MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, data[0]); break;
    case 4:
        MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, data[0]);
        MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, data[1]); break;
    case 6:
        MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, data[0]);
        MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, data[1]);
        MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, data[2]); break;
    case 8:
        MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, data[0]);
        MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, data[1]);
        MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, data[2]);
        MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG27, data[3]); break;
    default:
        spin_unlock_bh(&sf2_reg_access);
        printk("%s: len = %d NOT Handled !! \n", __FUNCTION__, len);
        return;
    }

    val = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_WRITE;
    MDIO_WR(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, val);

    for (i = 0; i < 20; i++) {
        MDIO_RD(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, &val);
        if ((val & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ)) == REG_PPM_REG17_OP_DONE)
            break;
        udelay(10);
    }

    spin_unlock_bh(&sf2_reg_access);

    if (i >= 20)
        printk("sf2_wreg: mdio timeout!\n");
}
EXPORT_SYMBOL(sf2_pseudo_mdio_switch_write);

#if !defined(MAC_SF2_EXTERNAL)
void sf2_mmap_rreg(int page, int reg, void *data_out, int len)   //mmap
{
    // based on impl5\bcmsw.c:extsw_rreg_mmap()
    uint32_t val;
    uint64_t data64 = 0;
    volatile unsigned *switch_directReadReg   = (unsigned int *) (SWITCH_DIRECT_DATA_RD_REG);
    void *data = &data64;
    volatile uint32_t *base = (volatile uint32_t *) (SWITCH_BASE + (((page << 8) + reg)*4*SF2_REG_SHIFT));
    
    spin_lock_bh(&sf2_reg_access);
    val = *base;
    switch (len) {
    case 1:
        *(uint32_t *)data = (uint8_t)val; break;
    case 2:
        *(uint32_t *)data = (uint16_t)val; break;
    case 4:
        *(uint32_t *)data = val; break;
    case 6:
        *(uint64_t *)data = val | ((uint64_t)(*switch_directReadReg & 0xffff)) << 32;  break;
    case 8:
        *(uint64_t *)data = val | ((uint64_t)(*switch_directReadReg)) << 32;  break;
    default:
        printk("%s: len = %d NOT Handled !! \n", __FUNCTION__, len);
        break;
    }
    spin_unlock_bh(&sf2_reg_access);
    memcpy(data_out, data, len);
}
EXPORT_SYMBOL(sf2_mmap_rreg);

void sf2_mmap_wreg(int page, int reg, void *data_in, int len)    //mmap
{
    // based on impl5\bcmsw.c:extsw_wreg_mmap()
    uint32_t val;
    uint64_t data64;
    volatile unsigned *switch_directWriteReg   = (unsigned int *) (SWITCH_DIRECT_DATA_WR_REG);
    void *data = &data64;
    
    volatile uint32_t *base = (volatile uint32_t *) (SWITCH_BASE + (((page << 8) + reg)*4*SF2_REG_SHIFT));
    memcpy(data, data_in, len);
    
    spin_lock_bh(&sf2_reg_access);
    val = *base;
    switch (len) {
    case 1:
        val = *(uint8_t *)data; break;
    case 2:
        val = *(uint16_t *)data; break;
    case 4:
        val = *(uint32_t *)data; break;
    case 6:
        *switch_directWriteReg = (data64 >> 32) & 0xffff;
        val = (uint32_t) data64; break;
    case 8:
        *switch_directWriteReg = data64 >> 32;
        val = (uint32_t) data64; break;
    default:
        spin_unlock_bh(&sf2_reg_access);
        printk("%s: len = %d NOT Handled !! \n", __FUNCTION__, len);
        return;
    }
    *base = val;
    spin_unlock_bh(&sf2_reg_access);
}
EXPORT_SYMBOL(sf2_mmap_wreg);
#endif //!MAC_SF2_EXTERNAL

#if defined(MAC_SF2_DUAL)
void sf2_sw_rreg(int unit, int page, int reg, void *data_out, int len)
{
    if (unit)
        sf2_pseudo_mdio_switch_read(page, reg, data_out, len);
    else
        sf2_mmap_rreg(page, reg, data_out, len);
}

void sf2_sw_wreg(int unit, int page, int reg, void *data_in, int len)
{
    if (unit)
        sf2_pseudo_mdio_switch_write(page, reg, data_in, len);
    else
        sf2_mmap_wreg(page, reg, data_in, len);
}

#elif defined(MAC_SF2_EXTERNAL)   // 47622: use mdio base
void sf2_sw_rreg(int unit, int page, int reg, void *data_out, int len) { sf2_pseudo_mdio_switch_read(page, reg, data_out, len); }
void sf2_sw_wreg(int unit, int page, int reg, void *data_in, int len)  { sf2_pseudo_mdio_switch_write(page, reg, data_in, len); }

#else // !MAC_SF2_EXTERNAL - use memory mapped
void sf2_sw_rreg(int unit, int page, int reg, void *data_out, int len) { sf2_mmap_rreg(page, reg, data_out, len); }
void sf2_sw_wreg(int unit, int page, int reg, void *data_in, int len)  { sf2_mmap_wreg(page, reg, data_in, len); }

#endif // !MAC_SF2_EXTERNAL

EXPORT_SYMBOL(sf2_sw_rreg);
EXPORT_SYMBOL(sf2_sw_wreg);

#define sf2_rreg(p,r,d,l) ((sf2_mac_dev_priv_data_t *)mac_dev->priv)->rreg(p,r,d,l)
#define sf2_wreg(p,r,d,l) ((sf2_mac_dev_priv_data_t *)mac_dev->priv)->wreg(p,r,d,l)


/********** MAC API **********/

static int port_sf2mac_stats_clear(mac_dev_t *mac_dev);


static int port_sf2mac_init(mac_dev_t *mac_dev)
{
    sf2_mac_dev_priv_data_t *p_priv = (sf2_mac_dev_priv_data_t *)mac_dev->priv;

    if (p_priv->priv_flags & SF2MAC_DRV_PRIV_FLAG_SHRINK_IPG)
    {
        uint32_t val32;
        spin_lock_bh(&extsw_reg_config);
        sf2_rreg(PAGE_MANAGEMENT, REG_IPG_SHRNK_CTRL, &val32, 4);
        val32 &= ~IPG_SHRNK_MASK(mac_dev->mac_id);
        val32 |= IPG_SHRNK_VAL(mac_dev->mac_id, IPG_4BYTE_SHRNK);
        sf2_wreg(PAGE_MANAGEMENT, REG_IPG_SHRNK_CTRL, &val32, 4);
        spin_unlock_bh(&extsw_reg_config);
    }

    return 0;
}

static int port_sf2mac_enable(mac_dev_t *mac_dev)
{
    // based on impl5\bcmsw.c:bcmsw_mac_rxtx_op()
    uint8_t v8;
    
    /* Clear MIB counters */
    port_sf2mac_stats_clear(mac_dev);

    spin_lock_bh(&extsw_reg_config);
    sf2_rreg(PAGE_CONTROL, REG_PORT_CTRL + mac_dev->mac_id, &v8, 1);
    v8 &= ~REG_PORT_CTRL_DISABLE;
    sf2_wreg(PAGE_CONTROL, REG_PORT_CTRL + mac_dev->mac_id, &v8, 1);
    spin_unlock_bh(&extsw_reg_config);

    return 0;
}

static int port_sf2mac_disable(mac_dev_t *mac_dev)
{
    // based on impl5\bcmsw.c:bcmsw_mac_rxtx_op()
    uint8_t v8;
    
    spin_lock_bh(&extsw_reg_config);
    sf2_rreg(PAGE_CONTROL, REG_PORT_CTRL + mac_dev->mac_id, &v8, 1);
    v8 |= REG_PORT_CTRL_DISABLE;
    sf2_wreg(PAGE_CONTROL, REG_PORT_CTRL + mac_dev->mac_id, &v8, 1);
    spin_unlock_bh(&extsw_reg_config);
    return 0;
}

static int port_sf2mac_pause_get(mac_dev_t *mac_dev, int *rx_enable, int *tx_enable)
{
    // return 0 (no error) only when override is enabled
    sf2_mac_dev_priv_data_t *p_priv = (sf2_mac_dev_priv_data_t*)mac_dev->priv;
    int page = PORT_OVERIDE_PAGE;
    int reg  = PORT_OVERIDE_REG(mac_dev->mac_id);
    int pause_page = PAUSE_CAP_PAGE;
    int pause_reg = PAUSE_CAP_REG;
    uint32_t val;

    if (p_priv->priv_flags & SF2MAC_DRV_PRIV_FLAG_SW_EXT) {
        // external switch override is in control page
        page = PAGE_CONTROL;
        reg = CTL_OVERIDE_REG(mac_dev->mac_id);
        pause_page = PAGE_CONTROL;
        pause_reg = REG_PAUSE_CAPBILITY;
    }

    sf2_rreg(pause_page, pause_reg, (uint8_t *)&val, 4);
    if (val & REG_PAUSE_CAPBILITY_OVERRIDE) {
        *rx_enable = (val & (1 << (mac_dev->mac_id + TOTAL_SWITCH_PORTS))) ? 1 : 0;
        *tx_enable = (val & (1 << mac_dev->mac_id)) ? 1 : 0;
        return 0;
    }

    sf2_rreg(page, reg, (uint8_t *)&val, 4);
    if (val & REG_PORT_STATE_OVERRIDE) {
        *rx_enable = (val & REG_PORT_STATE_RX_FLOWCTL) ? 1 : 0;
        *tx_enable = (val & REG_PORT_STATE_TX_FLOWCTL) ? 1 : 0;
        return 0;
    }

    return -1;  // need to get from phy instead
}

static int port_sf2mac_pause_set(mac_dev_t *mac_dev, int rx_enable, int tx_enable, char *src_addr)
{
    sf2_mac_dev_priv_data_t *p_priv = (sf2_mac_dev_priv_data_t*)mac_dev->priv;
    int page = PORT_OVERIDE_PAGE;
    int reg  = PORT_OVERIDE_REG(mac_dev->mac_id);
    int pause_page = PAUSE_CAP_PAGE;
    int pause_reg = PAUSE_CAP_REG;
    uint32_t val;

    if (p_priv->priv_flags & SF2MAC_DRV_PRIV_FLAG_SW_EXT) {
        // external switch override is in control page
        page = PAGE_CONTROL;
        reg = CTL_OVERIDE_REG(mac_dev->mac_id);
        pause_page = PAGE_CONTROL;
        pause_reg = REG_PAUSE_CAPBILITY;
    }

    spin_lock_bh(&extsw_reg_config);
    // if PAUSE_CAP_REG REG_PAUSE_CAPBILITY_OVERRIDE is enabled, set in this register
    // else if PORT_OVERIDE_REG REG_PORT_STATE_OVERRIDE is enabled, set in this register
    // otherwise, just set phy only
    sf2_rreg(pause_page, pause_reg, (uint8_t *)&val, 4);
    if (val & REG_PAUSE_CAPBILITY_OVERRIDE) {
        val &= ~((1 << mac_dev->mac_id) | (1 << (mac_dev->mac_id + TOTAL_SWITCH_PORTS)));
        if (tx_enable) val |= 1 << mac_dev->mac_id;
        if (rx_enable) val |= 1 << (mac_dev->mac_id + TOTAL_SWITCH_PORTS);
        sf2_wreg(pause_page, pause_reg, (uint8_t *)&val, 4);
        spin_unlock_bh(&extsw_reg_config);
        return 0;
    }

    sf2_rreg(page, reg, (uint8_t *)&val, 4);
    if (val & REG_PORT_STATE_OVERRIDE) {
        val &= ~(REG_PORT_STATE_RX_FLOWCTL | REG_PORT_STATE_TX_FLOWCTL);
        if (tx_enable) val |= REG_PORT_STATE_TX_FLOWCTL;
        if (rx_enable) val |= REG_PORT_STATE_RX_FLOWCTL;
        sf2_wreg(page, reg, (uint8_t *)&val, 4);
        spin_unlock_bh(&extsw_reg_config);
        return 0;
    }
    spin_unlock_bh(&extsw_reg_config);

    return 0;
}

#if defined(MAC_SF2_DUAL)
#define HIGH_SPEED_STATE    1       // chips support speed state > 1G
#endif

static int port_s2fmac_read_status(mac_dev_t *mac_dev, mac_status_t *mac_status)
{
    uint32_t link_st, spd_st, dup_st, pause_st;
    
    sf2_rreg(PAGE_STATUS, REG_LNKSTS, &link_st, 4);    
    sf2_rreg(PAGE_STATUS, REG_DUPSTS, &dup_st, 4);    
    sf2_rreg(PAGE_STATUS, REG_PAUSESTS, &pause_st, 4);    
    
    mac_status->link = LNKSTS_UP(link_st, mac_dev->mac_id) ? 1 : 0;
    mac_status->duplex = DUPSTS(dup_st, mac_dev->mac_id) ? MAC_DUPLEX_FULL : MAC_DUPLEX_HALF;
    mac_status->pause_rx = PAUSERXSTS(pause_st, mac_dev->mac_id)? 1 : 0;
    mac_status->pause_tx = PAUSETXSTS(pause_st, mac_dev->mac_id)? 1 : 0;

#if defined(HIGH_SPEED_STATE)
    sf2_rreg(PAGE_STATUS, REG_NEW_SPDSTS, &spd_st, 4);
    spd_st = NSPDSTS(spd_st, mac_dev->mac_id);
    switch (spd_st) {
        case 0: mac_status->speed = MAC_SPEED_10; break;
        case 1: mac_status->speed = MAC_SPEED_100; break;
        case 2: mac_status->speed = MAC_SPEED_1000; break;
        case 3: mac_status->speed = MAC_SPEED_2500; break;
        case 4: mac_status->speed = MAC_SPEED_10000; break;
        case 5: mac_status->speed = MAC_SPEED_5000; break;
        default: mac_status->speed = MAC_SPEED_UNKNOWN;
    }
#else
    sf2_rreg(PAGE_STATUS, REG_SPDSTS, &spd_st, 4);
    spd_st = SPDSTS(spd_st, mac_dev->mac_id);
    switch (spd_st) {
        case 0: mac_status->speed = MAC_SPEED_10; break;
        case 1: mac_status->speed = MAC_SPEED_100; break;
        case 2: mac_status->speed = MAC_SPEED_1000; break;
        default: mac_status->speed = MAC_SPEED_2500;
    }
#endif
    return 0;
}

static int port_sf2mac_cfg_get(mac_dev_t *mac_dev, mac_cfg_t *mac_cfg)
{
    sf2_mac_dev_priv_data_t *p_priv = (sf2_mac_dev_priv_data_t*)mac_dev->priv;
    int page = PORT_OVERIDE_PAGE;
    int reg  = PORT_OVERIDE_REG(mac_dev->mac_id);
    uint8_t v8;

    if (p_priv->priv_flags & SF2MAC_DRV_PRIV_FLAG_SW_EXT) {
        // external switch override is in control page
        page = PAGE_CONTROL;
        reg = CTL_OVERIDE_REG(mac_dev->mac_id);
    }

    // read from hw
    sf2_rreg(page, reg, &v8, 1);
    switch (v8 & ((mac_dev->mac_id == IMP_PORT_ID) ? REG_PORT_STATE_IMP_SPD_MSK : REG_PORT_STATE_EXT_SPD_MSK)) {
#if defined(HIGH_SPEED_STATE)
    case REG_CONTROL_MPSO_SPEED10G:
    case REG_PORT_STATE_10000:      mac_cfg->speed = MAC_SPEED_10000; break;
    case REG_CONTROL_MPSO_SPEED5G:
    case REG_PORT_STATE_5000:       mac_cfg->speed = MAC_SPEED_5000; break;
    case REG_PORT_STATE_2500:       mac_cfg->speed = MAC_SPEED_2500; break;
#else
    case REG_PORT_GMII_SPEED_UP_2G: mac_cfg->speed = MAC_SPEED_2500; break;
#endif
    case REG_PORT_STATE_1000:       mac_cfg->speed = MAC_SPEED_1000; break;
    case REG_PORT_STATE_100:        mac_cfg->speed = MAC_SPEED_100; break;
    default:                        mac_cfg->speed = MAC_SPEED_10; break;
    }

    mac_cfg->duplex = (v8 & REG_PORT_STATE_FDX)? MAC_DUPLEX_FULL : MAC_DUPLEX_HALF;

    return 0;
}

static int port_sf2mac_cfg_set(mac_dev_t *mac_dev, mac_cfg_t *mac_cfg)
{
    sf2_mac_dev_priv_data_t *p_priv = (sf2_mac_dev_priv_data_t*)mac_dev->priv;
    int page = PORT_OVERIDE_PAGE;
    int reg  = PORT_OVERIDE_REG(mac_dev->mac_id);
    uint16_t v16;

    if (p_priv->priv_flags & SF2MAC_DRV_PRIV_FLAG_SW_EXT) {
        // external switch override is in control page
        page = PAGE_CONTROL;
        reg = CTL_OVERIDE_REG(mac_dev->mac_id);
    }

    spin_lock_bh(&extsw_reg_config);
    sf2_rreg(page, reg, &v16, 2);
    v16 &= (REG_PORT_STATE_TX_FLOWCTL | REG_PORT_STATE_RX_FLOWCTL);  // save FC
    v16 |= ((mac_cfg->flag & MAC_FLAG_XGMII)? REG_PORT_STATE_XGMII_MODE : 0)|REG_PORT_STATE_LNK;

    switch (mac_cfg->speed) {
    case MAC_SPEED_100:     v16 |= REG_PORT_STATE_100; break;
    case MAC_SPEED_1000:    v16 |= REG_PORT_STATE_1000; break;
#if defined(HIGH_SPEED_STATE)
    case MAC_SPEED_2500:    v16 |= REG_PORT_STATE_2500; break;
    case MAC_SPEED_5000:    v16 |= (mac_dev->mac_id == IMP_PORT_ID) ? REG_CONTROL_MPSO_SPEED5G : REG_PORT_STATE_5000; break;
    case MAC_SPEED_10000:   v16 |= (mac_dev->mac_id == IMP_PORT_ID) ? REG_CONTROL_MPSO_SPEED10G: REG_PORT_STATE_10000; break;
#else
    case MAC_SPEED_2500:    v16 |= REG_PORT_STATE_1000 | REG_PORT_GMII_SPEED_UP_2G; break;
#endif
    case MAC_SPEED_UNKNOWN: v16 &= ~REG_PORT_STATE_LNK;
    default: break;
    }
    
    v16 |= ((mac_dev->mac_id == IMP_PORT_ID) ? REG_CONTROL_MPSO_MII_SW_OVERRIDE : REG_PORT_STATE_OVERRIDE) | 
          ((mac_cfg->duplex == MAC_DUPLEX_FULL)? REG_PORT_STATE_FDX : 0);
    
    sf2_wreg(page, reg, &v16, 2);
    spin_unlock_bh(&extsw_reg_config);
    return 0;
}

static void port_sf2mac_fold_stats(mac_stats_t *dstats, 
                                   mac_stats_t *lstats, 
                                   mac_stats_t *cstats)
{
	int i;
    int num_flds = sizeof(*cstats) / sizeof(uint64_t);    /* All fields are of same data-type */
	const uint64_t *cur = (const uint64_t *)cstats;       /* Current stats from HW */
	const uint64_t *last = (const uint64_t *)lstats;      /* Last snapshot of HW stats */
	uint64_t *now = (uint64_t *)dstats;                   /* Device stats now */
    uint32_t add_uint_max = 0;    
 
	for (i = 0; i < num_flds; i++)
    {
        if (last[i] > cur[i]) /* wrap around */
        {
            add_uint_max = UINT_MAX; 
        }
        now[i] += (cur[i] + (add_uint_max -last[i]) ); /* Add the difference from last time */ 
        add_uint_max = 0;
    }
    /* Store the current stats from HW into last stats snapshot */
    memcpy(lstats, cstats, sizeof(*lstats));
}

// if mac_stats is null, just read counter to clear counter
static int port_sf2mac_stats_read(mac_dev_t *mac_dev, mac_stats_t *mac_stats)
{
    // based on enet\impl5\bcmsw.c::bcmsw_get_hw_stats()
    // counters are not clear on read, we need to write to clear.
    uint32_t ctr32;
    uint64_t ctr64;

    memset(mac_stats, 0, sizeof(*mac_stats));
    
    {
        spin_lock_bh(&sf2_stat_access);
        sf2_rreg(PAGE_CONTROL, REG_LOW_POWER_EXP1, &ctr32, 4);
        if (ctr32 & (1<<mac_dev->mac_id)) {
            spin_unlock_bh(&sf2_stat_access);
            printk("Err: port=%d is in low power mode - mib counters not accessible!!\n", mac_dev->mac_id);    // SLEEP_SYSCLK_PORT for specified port is set
            return -1;
        }
        sf2_rreg(PAGE_CONTROL, REG_SW_RESET, &ctr32, 4);
        if (ctr32 & REG_SW_RST) {
            spin_unlock_bh(&sf2_stat_access);
            printk("Err: sf2 switch in reset - mib counters not accessible!!\n");
            return -1;
        }
    }
    // track RX byte count
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXOCTETS, (uint8_t*)&ctr64, 8);
    mac_stats->rx_byte = ctr64;
    // track RX unicast, multicast, and broadcast packets
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXBPKTS, (uint8_t*)&ctr32, 4);
    ctr64 = ctr32;
    mac_stats->rx_broadcast_packet = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXMPKTS, (uint8_t*)&ctr32, 4);
    ctr64 += ctr32;
    mac_stats->rx_multicast_packet = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXUPKTS, (uint8_t*)&ctr32, 4);
    ctr64 += ctr32;
    mac_stats->rx_unicast_packet = ctr32;
    mac_stats->rx_packet = ctr64;

    // track RX packets of different sizes
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RX64OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_frame_64 = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RX127OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_frame_65_127 = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RX255OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_frame_128_255 = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RX511OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_frame_256_511 = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RX1023OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_frame_512_1023 = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXMAXOCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_frame_1024_1518 = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXOVERSIZE, (uint8_t*)&ctr32, 4);
    mac_stats->rx_frame_1519_mtu = ctr32;
    
    // track RX packet errors
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXALIGNERRORS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_alignment_error = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXSYMBOLERRORS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_code_error = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXFCSERRORS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_fcs_error = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXUNDERSIZEPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_undersize_packet = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXFRAGMENTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_fragments = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXJABBERS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_jabber = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXJUMBOPKT, (uint8_t*)&ctr32, 4);
    mac_stats->rx_oversize_packet = ctr32;

    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXDROPS, &ctr32, 4);
    mac_stats->rx_dropped = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXDISCARD, &ctr32, 4);
    mac_stats->rx_dropped += ctr32;

    // track TX byte count
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXOCTETS, (uint8_t*)&ctr64, 8);
    mac_stats->tx_byte = ctr64;
    // track TX unicast, multicast, and broadcast packets
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXBPKTS, (uint8_t*)&ctr32, 4);
    ctr64 = ctr32;
    mac_stats->tx_broadcast_packet = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXMPKTS, (uint8_t*)&ctr32, 4);
    ctr64 += ctr32;
    mac_stats->tx_multicast_packet = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXUPKTS, (uint8_t*)&ctr32, 4);
    ctr64 += ctr32;
    mac_stats->tx_unicast_packet = ctr32;
    mac_stats->tx_packet = ctr64;

    // track TX packets of different sizes
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, SF2_REG_MIB_P0_TX64OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->tx_frame_64 = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, SF2_REG_MIB_P0_TX127OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->tx_frame_65_127 = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, SF2_REG_MIB_P0_TX255OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->tx_frame_128_255 = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, SF2_REG_MIB_P0_TX511OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->tx_frame_256_511 = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, SF2_REG_MIB_P0_TX1023OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->tx_frame_512_1023 = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, SF2_REG_MIB_P0_TXMAXOCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->tx_frame_1024_1518 = ctr32;

    // track TX packet errors
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXCOL, (uint8_t*)&ctr32, 4);
    mac_stats->tx_total_collision = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXSINGLECOL, (uint8_t*)&ctr32, 4);
    mac_stats->tx_single_collision = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXMULTICOL, (uint8_t*)&ctr32, 4);
    mac_stats->tx_multiple_collision = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXLATECOL, (uint8_t*)&ctr32, 4);
    mac_stats->tx_late_collision = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXEXCESSCOL, (uint8_t*)&ctr32, 4);
    mac_stats->tx_excessive_collision = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXDEFERREDTX, (uint8_t*)&ctr32, 4);
    mac_stats->tx_deferral_packet = ctr32;

    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXDROPS, &ctr32, 4);
    mac_stats->tx_dropped = ctr32;
    sf2_rreg(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXFRAMEINDISC, &ctr32, 4);
    mac_stats->tx_dropped += ctr32;
    spin_unlock_bh(&sf2_stat_access);

    return 0;
}
static int port_sf2mac_stats_get(mac_dev_t *mac_dev, mac_stats_t *mac_stats)
{
    sf2_mac_dev_priv_data_t *p_priv = (sf2_mac_dev_priv_data_t*)mac_dev->priv;
    port_sf2mac_stats_read(mac_dev, mac_stats);
    port_sf2mac_fold_stats(&p_priv->mac_stats, &p_priv->last_mac_stats, mac_stats);
    memcpy(mac_stats, &p_priv->mac_stats, sizeof(*mac_stats));
    return 0;
}
static int port_sf2mac_stats_clear(mac_dev_t *mac_dev)
{
    uint32_t global_cfg, rst_mib_en;
    sf2_mac_dev_priv_data_t *p_priv = (sf2_mac_dev_priv_data_t*)mac_dev->priv;

    spin_lock_bh(&extsw_reg_config);
    // read reset mib enable mask
    sf2_rreg(PAGE_MANAGEMENT, REG_RST_MIB_CNT_EN, (uint8_t*)&rst_mib_en, 4);
    rst_mib_en = (rst_mib_en & ~REG_RST_MIB_CNT_EN_PORT_M) | 1 << mac_dev->mac_id;
    // only enable clearing of this port
    sf2_wreg(PAGE_MANAGEMENT, REG_RST_MIB_CNT_EN, (uint8_t*)&rst_mib_en, 4);

    // toggle global reset mib bit
    sf2_rreg(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &global_cfg, 4);
    global_cfg |= GLOBAL_CFG_RESET_MIB;
    sf2_wreg(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, (uint8_t*)&global_cfg, 4);
    global_cfg &= ~GLOBAL_CFG_RESET_MIB;
    sf2_wreg(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, (uint8_t*)&global_cfg, 4);
    spin_unlock_bh(&extsw_reg_config);

    udelay(50);  // hw need time to clear mibs
    memset(&p_priv->mac_stats, 0, sizeof(p_priv->mac_stats));
    memset(&p_priv->last_mac_stats, 0, sizeof(p_priv->last_mac_stats));
    return 0;
}

static int port_sf2mac_mtu_set(mac_dev_t *mac_dev, int mtu)
{
    uint32_t mask, max;
    
#if defined(CONFIG_BCM_JUMBO_FRAME)
    /* 
       Set MIB values for jumbo frames to reflect our maximum frame size.
       Need to set size to hardware max size, otherwise byte counter and
       frame counters in hardware will be inconsistent
     */
    max = MAX_HW_JUMBO_FRAME_SIZE;
    sf2_wreg(PAGE_JUMBO, REG_JUMBO_FRAME_SIZE, (uint8_t *)&max, 4);
#else
    sf2_rreg(PAGE_JUMBO, REG_JUMBO_FRAME_SIZE, (uint8_t *)&max, 4);
    max &= 0x3fff;
#endif

    spin_lock_bh(&extsw_reg_config);
    sf2_rreg(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8_t *)&mask, 4);
    if (mtu > max)
        mask |= 1<<mac_dev->mac_id;
    else
        mask &= ~(1<<mac_dev->mac_id);
    
    sf2_wreg(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8_t *)&mask, 4);
    spin_unlock_bh(&extsw_reg_config);
    return 0;
}

static int port_sf2mac_eee_set(mac_dev_t *mac_dev, int enable)
{
    uint16_t v16;

    spin_lock_bh(&extsw_reg_config);
    sf2_rreg (PAGE_EEE, REG_EEE_EN_CTRL, (uint8_t *)&v16, 2);

    /* enable / disable the corresponding port */
    if (enable)
        v16 |= (1 << mac_dev->mac_id);
    else
        v16 &= ~(1 << mac_dev->mac_id);

    sf2_wreg (PAGE_EEE, REG_EEE_EN_CTRL, (uint8_t*)&v16, 2);
    spin_unlock_bh(&extsw_reg_config);
   
    return 0;
}

static int port_sf2mac_dev_add(mac_dev_t *mac_dev)
{
    /* allocate the private data */
    sf2_mac_dev_priv_data_t *p_priv = kmalloc(sizeof(sf2_mac_dev_priv_data_t), GFP_ATOMIC);;
    /* set private data members - priv_flags are passed in priv pointer */
    p_priv->priv_flags = (unsigned long)mac_dev->priv;
    memset(&p_priv->mac_stats, 0, sizeof(p_priv->mac_stats));
    memset(&p_priv->last_mac_stats, 0, sizeof(p_priv->last_mac_stats));
#if defined(MAC_SF2_DUAL)
    p_priv->rreg = (p_priv->priv_flags & SF2MAC_DRV_PRIV_FLAG_SW_EXT) ? sf2_pseudo_mdio_switch_read : sf2_mmap_rreg;
    p_priv->wreg = (p_priv->priv_flags & SF2MAC_DRV_PRIV_FLAG_SW_EXT) ? sf2_pseudo_mdio_switch_write : sf2_mmap_wreg;
#elif defined(MAC_SF2_EXTERNAL)
    p_priv->rreg = sf2_pseudo_mdio_switch_read;
    p_priv->wreg = sf2_pseudo_mdio_switch_write;
#else
    p_priv->rreg = sf2_mmap_rreg;
    p_priv->wreg = sf2_mmap_wreg;
#endif

    /* assign private data */
    mac_dev->priv = p_priv;
    return 0;
}

static int port_sf2mac_dev_del(mac_dev_t *mac_dev)
{
    /* Release allocated memory */
    kfree(mac_dev->priv);
    mac_dev->priv = NULL;
    return 0;
}

static int port_sf2mac_drv_init(mac_drv_t *mac_drv)
{
#if !defined(MAC_SF2_EXTERNAL)          // TODO47622: temp comment out 
    // Init EEE Wake delay per spec for 2.5G port #6 (0x1e instead of 0x11)
    // If port is used for 1G Ethernet, 0x1e is OK too.
    uint32_t wake_delay = 0x1e;
    sf2_mmap_wreg(PAGE_EEE, REG_EEE_WAKE_TIMER_G+6*2, (uint8_t*)&wake_delay, 2);
#endif

    mac_drv->initialized = 1;
    return 0;
}

static int port_sf2mac_dt_priv(const dt_handle_t handle, int mac_id, void **priv)
{
    unsigned long priv_flags = 0;

    if (dt_property_read_bool(handle, "shrink-ipg"))
        priv_flags |= SF2MAC_DRV_PRIV_FLAG_SHRINK_IPG;

#if defined(MAC_SF2_DUAL) || defined(MAC_SF2_EXTERNAL)
    if (dt_property_read_u32_default(dt_parent(dt_parent(handle)),"unit",0))
        priv_flags |= SF2MAC_DRV_PRIV_FLAG_SW_EXT;
#endif

   *priv = (void *)(unsigned long)priv_flags;

    return 0;
}

mac_drv_t mac_drv_sf2 =
{
    .mac_type = MAC_TYPE_SF2,
    .name = "SF2MAC",
    .init = port_sf2mac_init,
    .read_status = port_s2fmac_read_status,
    .enable = port_sf2mac_enable,
    .disable = port_sf2mac_disable,
    .cfg_get = port_sf2mac_cfg_get,
    .cfg_set = port_sf2mac_cfg_set,   /* MACs are handled by hardware in SF2 ? */
    .pause_get = port_sf2mac_pause_get,
    .pause_set = port_sf2mac_pause_set,
    .stats_get = port_sf2mac_stats_get,
    .stats_clear = port_sf2mac_stats_clear,
    .mtu_set = port_sf2mac_mtu_set,
    .eee_set = port_sf2mac_eee_set,
    .dev_add = port_sf2mac_dev_add,
    .dev_del = port_sf2mac_dev_del,
    .drv_init = port_sf2mac_drv_init,
    .dt_priv = port_sf2mac_dt_priv,
};
