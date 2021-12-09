/*
   <:copyright-BRCM:2016:DUAL/GPL:standard

      Copyright (c) 2016 Broadcom
      All Rights Reserved

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
 * GMAC driver for Star Fighter 2
 */

#include "mac_drv.h"
#include "mac_drv_sf2.h"
#include <board.h>
#include "bcm_map_part.h"
#include "bcm_misc_hw_init.h"

/* Only necessary for ether_gphy_reset */
#include <linux/delay.h>
#include <linux/slab.h>
#include "bcm_gpio.h"

#include <bcm/bcmswapitypes.h>
#include "bcmmii.h"
#include "bcmmii_xtn.h"

#include <linux/spinlock.h>
static DEFINE_SPINLOCK(sf2_reg_access);
static DEFINE_SPINLOCK(sf2_stat_access);

#define SF2_REG_SHIFT (sizeof(((EthernetSwitchCore *)0)->port_traffic_ctrl[0])/4)
void sf2_rreg_mmap(int page, int reg, void *data_out, int len)
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
EXPORT_SYMBOL(sf2_rreg_mmap);

void sf2_wreg_mmap(int page, int reg, void *data_in, int len)
{
    // based on impl5\bcmsw.c:extsw_2reg_mmap()
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
EXPORT_SYMBOL(sf2_wreg_mmap);



/********** MAC API **********/

static int port_sf2mac_stats_clear(mac_dev_t *mac_dev);


static int port_sf2mac_init(mac_dev_t *mac_dev)
{
    sf2_mac_dev_priv_data_t *p_priv = (sf2_mac_dev_priv_data_t *)mac_dev->priv;

    if (p_priv->priv_flags & SF2MAC_DRV_PRIV_FLAG_SHRINK_IPG)
    {
        uint32_t val32;
        sf2_rreg_mmap(PAGE_MANAGEMENT, REG_IPG_SHRNK_CTRL, &val32, 4);
        val32 &= ~IPG_SHRNK_MASK(mac_dev->mac_id);
        val32 |= IPG_SHRNK_VAL(mac_dev->mac_id, IPG_4BYTE_SHRNK);
        sf2_wreg_mmap(PAGE_MANAGEMENT, REG_IPG_SHRNK_CTRL, &val32, 4);
    }

    return 0;
}

static int port_sf2mac_enable(mac_dev_t *mac_dev)
{
    // based on impl5\bcmsw.c:bcmsw_mac_rxtx_op()
    uint8_t v8;
    
    /* Clear MIB counters */
    port_sf2mac_stats_clear(mac_dev);

    sf2_rreg_mmap(PAGE_CONTROL, REG_PORT_CTRL + mac_dev->mac_id, &v8, 1);
    v8 &= ~REG_PORT_CTRL_DISABLE;
    sf2_wreg_mmap(PAGE_CONTROL, REG_PORT_CTRL + mac_dev->mac_id, &v8, 1);

    return 0;
}

static int port_sf2mac_disable(mac_dev_t *mac_dev)
{
    // based on impl5\bcmsw.c:bcmsw_mac_rxtx_op()
    uint8_t v8;
    
    sf2_rreg_mmap(PAGE_CONTROL, REG_PORT_CTRL + mac_dev->mac_id, &v8, 1);
    v8 |= REG_PORT_CTRL_DISABLE;
    sf2_wreg_mmap(PAGE_CONTROL, REG_PORT_CTRL + mac_dev->mac_id, &v8, 1);
    return 0;
}

static int port_sf2mac_pause_get(mac_dev_t *mac_dev, int *rx_enable, int *tx_enable)
{
    // return 0 (no error) only when override is enabled
    uint32_t val;

    sf2_rreg_mmap(PAUSE_CAP_PAGE, PAUSE_CAP_REG, (uint8_t *)&val, 4);
    if (val & REG_PAUSE_CAPBILITY_OVERRIDE) {
        *rx_enable = (val & (1 << (mac_dev->mac_id + TOTAL_SWITCH_PORTS))) ? 1 : 0;
        *tx_enable = (val & (1 << mac_dev->mac_id)) ? 1 : 0;
        return 0;
    }

    sf2_rreg_mmap(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(mac_dev->mac_id), (uint8_t *)&val, 4);
    if (val & REG_PORT_STATE_OVERRIDE) {
        *rx_enable = (val & REG_PORT_STATE_RX_FLOWCTL) ? 1 : 0;
        *tx_enable = (val & REG_PORT_STATE_TX_FLOWCTL) ? 1 : 0;
        return 0;
    }

    return -1;  // need to get from phy instead
}

static int port_sf2mac_pause_set(mac_dev_t *mac_dev, int rx_enable, int tx_enable, char *src_addr)
{
    uint32_t val;

    // if PAUSE_CAP_REG REG_PAUSE_CAPBILITY_OVERRIDE is enabled, set in this register
    // else if PORT_OVERIDE_REG REG_PORT_STATE_OVERRIDE is enabled, set in this register
    // otherwise, just set phy only
    sf2_rreg_mmap(PAUSE_CAP_PAGE, PAUSE_CAP_REG, (uint8_t *)&val, 4);
    if (val & REG_PAUSE_CAPBILITY_OVERRIDE) {
        val &= ~((1 << mac_dev->mac_id) | (1 << (mac_dev->mac_id + TOTAL_SWITCH_PORTS)));
        if (tx_enable) val |= 1 << mac_dev->mac_id;
        if (rx_enable) val |= 1 << (mac_dev->mac_id + TOTAL_SWITCH_PORTS);
        sf2_wreg_mmap(PAUSE_CAP_PAGE, PAUSE_CAP_REG, (uint8_t *)&val, 4);
        return 0;
    }

    sf2_rreg_mmap(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(mac_dev->mac_id), (uint8_t *)&val, 4);
    if (val & REG_PORT_STATE_OVERRIDE) {
        val &= ~(REG_PORT_STATE_RX_FLOWCTL | REG_PORT_STATE_TX_FLOWCTL);
        if (tx_enable) val |= REG_PORT_STATE_TX_FLOWCTL;
        if (rx_enable) val |= REG_PORT_STATE_RX_FLOWCTL;
        sf2_wreg_mmap(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(mac_dev->mac_id), (uint8_t *)&val, 4);
        return 0;
    }

    return 0;
}

static int port_sf2mac_cfg_get(mac_dev_t *mac_dev, mac_cfg_t *mac_cfg)
{
    uint8_t v8;

    // read from hw
    sf2_rreg_mmap(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(mac_dev->mac_id), &v8, 1);
    if (v8 & REG_PORT_GMII_SPEED_UP_2G)
        mac_cfg->speed = MAC_SPEED_2500;
    else if (v8 & REG_PORT_STATE_1000)
        mac_cfg->speed = MAC_SPEED_1000;
    else if (v8 & REG_PORT_STATE_100)
        mac_cfg->speed = MAC_SPEED_100;
    else
        mac_cfg->speed = MAC_SPEED_10;

    mac_cfg->duplex = (v8 & REG_PORT_STATE_FDX)? MAC_DUPLEX_FULL : MAC_DUPLEX_HALF;

    return 0;
}

static int port_sf2mac_cfg_set(mac_dev_t *mac_dev, mac_cfg_t *mac_cfg)
{
    // based on impl5\bcmsw.c:bcmsw_set_mac_port_state()
    uint8_t v8;
    
    sf2_rreg_mmap(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(mac_dev->mac_id), &v8, 1);
    v8 &= (REG_PORT_STATE_TX_FLOWCTL | REG_PORT_STATE_RX_FLOWCTL);  // save FC

    if (mac_cfg->speed == MAC_SPEED_10)
        v8 |= REG_PORT_STATE_LNK;
    else if (mac_cfg->speed == MAC_SPEED_100)
        v8 |= REG_PORT_STATE_100 | REG_PORT_STATE_LNK;
    else if (mac_cfg->speed == MAC_SPEED_1000)
        v8 |= REG_PORT_STATE_1000 | REG_PORT_STATE_LNK;
    else if (mac_cfg->speed == MAC_SPEED_2500)
        v8 |= REG_PORT_GMII_SPEED_UP_2G | REG_PORT_STATE_1000 | REG_PORT_STATE_LNK;
    
    v8 |= REG_PORT_STATE_OVERRIDE | 
          ((mac_cfg->duplex == MAC_DUPLEX_FULL)? REG_PORT_STATE_FDX : 0);
    
    sf2_wreg_mmap(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(mac_dev->mac_id), &v8, 1);
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
    uint32 add_uint_max = 0;    
 
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
    
    {
        spin_lock_bh(&sf2_stat_access);
        sf2_rreg_mmap(PAGE_CONTROL, REG_LOW_POWER_EXP1, &ctr32, 4);
        if (ctr32 & (1<<mac_dev->mac_id)) {
            spin_unlock_bh(&sf2_stat_access);
            printk("Err: port=%d is in low power mode - mib counters not accessible!!\n", mac_dev->mac_id);    // SLEEP_SYSCLK_PORT for specified port is set
            return -1;
        }
        sf2_rreg_mmap(PAGE_CONTROL, REG_SW_RESET, &ctr32, 4);
        if (ctr32 & REG_SW_RST) {
            spin_unlock_bh(&sf2_stat_access);
            printk("Err: sf2 switch in reset - mib counters not accessible!!\n");
            return -1;
        }
    }
    // track RX byte count
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXOCTETS, (uint8_t*)&ctr64, 8);
    mac_stats->rx_byte = ctr64;
    // track RX unicast, multicast, and broadcast packets
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXBPKTS, (uint8_t*)&ctr32, 4);
    ctr64 = ctr32;
    mac_stats->rx_broadcast_packet = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXMPKTS, (uint8_t*)&ctr32, 4);
    ctr64 += ctr32;
    mac_stats->rx_multicast_packet = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXUPKTS, (uint8_t*)&ctr32, 4);
    ctr64 += ctr32;
    mac_stats->rx_unicast_packet = ctr32;
    mac_stats->rx_packet = ctr64;
    
    // track RX packets of different sizes
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RX64OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_frame_64 = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RX127OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_frame_65_127 = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RX255OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_frame_128_255 = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RX511OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_frame_256_511 = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RX1023OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_frame_512_1023 = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXMAXOCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_frame_1024_1518 = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXOVERSIZE, (uint8_t*)&ctr32, 4);
    mac_stats->rx_frame_1519_mtu = ctr32;
    
    // track RX packet errors
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXALIGNERRORS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_alignment_error = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXSYMBOLERRORS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_code_error = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXFCSERRORS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_fcs_error = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXUNDERSIZEPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_undersize_packet = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXFRAGMENTS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_fragments = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXJABBERS, (uint8_t*)&ctr32, 4);
    mac_stats->rx_jabber = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXJUMBOPKT, (uint8_t*)&ctr32, 4);
    mac_stats->rx_oversize_packet = ctr32;

    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXDROPS, &ctr32, 4);
    mac_stats->rx_dropped = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_RXDISCARD, &ctr32, 4);
    mac_stats->rx_dropped += ctr32;

    // track TX byte count
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXOCTETS, (uint8_t*)&ctr64, 8);
    mac_stats->tx_byte = ctr64;
    // track TX unicast, multicast, and broadcast packets
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXBPKTS, (uint8_t*)&ctr32, 4);
    ctr64 = ctr32;
    mac_stats->tx_broadcast_packet = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXMPKTS, (uint8_t*)&ctr32, 4);
    ctr64 += ctr32;
    mac_stats->tx_multicast_packet = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXUPKTS, (uint8_t*)&ctr32, 4);
    ctr64 += ctr32;
    mac_stats->tx_unicast_packet = ctr32;
    mac_stats->tx_packet = ctr64;

    // track TX packets of different sizes
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, SF2_REG_MIB_P0_TX64OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->tx_frame_64 = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, SF2_REG_MIB_P0_TX127OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->tx_frame_65_127 = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, SF2_REG_MIB_P0_TX255OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->tx_frame_128_255 = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, SF2_REG_MIB_P0_TX511OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->tx_frame_256_511 = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, SF2_REG_MIB_P0_TX1023OCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->tx_frame_512_1023 = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, SF2_REG_MIB_P0_TXMAXOCTPKTS, (uint8_t*)&ctr32, 4);
    mac_stats->tx_frame_1024_1518 = ctr32;
    
    // track TX packet errors
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXCOL, (uint8_t*)&ctr32, 4);
    mac_stats->tx_total_collision = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXSINGLECOL, (uint8_t*)&ctr32, 4);
    mac_stats->tx_single_collision = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXMULTICOL, (uint8_t*)&ctr32, 4);
    mac_stats->tx_multiple_collision = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXLATECOL, (uint8_t*)&ctr32, 4);
    mac_stats->tx_late_collision = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXEXCESSCOL, (uint8_t*)&ctr32, 4);
    mac_stats->tx_excessive_collision = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXDEFERREDTX, (uint8_t*)&ctr32, 4);
    mac_stats->tx_deferral_packet = ctr32;

    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXDROPS, &ctr32, 4);
    mac_stats->tx_dropped = ctr32;
    sf2_rreg_mmap(PAGE_MIB_P0 + mac_dev->mac_id, REG_MIB_P0_EXT_TXFRAMEINDISC, &ctr32, 4);
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

    // read reset mib enable mask
    sf2_rreg_mmap(PAGE_MANAGEMENT, REG_RST_MIB_CNT_EN, (uint8_t*)&rst_mib_en, 4);
    rst_mib_en = (rst_mib_en & ~REG_RST_MIB_CNT_EN_PORT_M) | 1 << mac_dev->mac_id;
    // only enable clearing of this port
    sf2_wreg_mmap(PAGE_MANAGEMENT, REG_RST_MIB_CNT_EN, (uint8_t*)&rst_mib_en, 4);

    // toggle global reset mib bit
    sf2_rreg_mmap(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &global_cfg, 4);
    global_cfg |= GLOBAL_CFG_RESET_MIB;
    sf2_wreg_mmap(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, (uint8_t*)&global_cfg, 4);
    global_cfg &= ~GLOBAL_CFG_RESET_MIB;
    sf2_wreg_mmap(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, (uint8_t*)&global_cfg, 4);

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
    sf2_wreg_mmap(PAGE_JUMBO, REG_JUMBO_FRAME_SIZE, (uint8_t *)&max, 4);
#else
    sf2_rreg_mmap(PAGE_JUMBO, REG_JUMBO_FRAME_SIZE, (uint8_t *)&max, 4);
    max &= 0x3fff;
#endif

    sf2_rreg_mmap(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8_t *)&mask, 4);
    if (mtu > max)
        mask |= 1<<mac_dev->mac_id;
    else
        mask &= ~(1<<mac_dev->mac_id);
    
    sf2_wreg_mmap(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8_t *)&mask, 4);
    return 0;
}

static int port_sf2mac_eee_set(mac_dev_t *mac_dev, int enable)
{
    uint16 v16;

    sf2_rreg_mmap (PAGE_EEE, REG_EEE_EN_CTRL, (uint8_t *)&v16, 2);

    /* enable / disable the corresponding port */
    if (enable)
        v16 |= (1 << mac_dev->mac_id);
    else
        v16 &= ~(1 << mac_dev->mac_id);

    sf2_wreg_mmap (PAGE_EEE, REG_EEE_EN_CTRL, (uint8_t*)&v16, 2);
   
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
    //TODO_DSL? port_sf2mac_drv_init() anything here?
    mac_drv->initialized = 1;
    return 0;
}


mac_drv_t mac_drv_sf2 =
{
    .mac_type = MAC_TYPE_SF2,
    .name = "SF2MAC",
    .init = port_sf2mac_init,
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
};
