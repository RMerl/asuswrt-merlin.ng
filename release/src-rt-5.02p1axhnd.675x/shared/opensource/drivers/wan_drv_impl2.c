/*
    Copyright 2015 Broadcom Corporation

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

#include "bcm_map_part.h"
#include "mdio_drv_impl2.h"
#include "access_macros.h"
#include "bl_os_wraper.h"
#include "boardparms.h"
#include <bcmsfp_i2c.h>
#include "pmc_drv.h"
#include "BPCM.h"
#include "bcm_pinmux.h"
#include "bcm_gpio.h"
#include "board.h"
#include "wan_drv.h"
#include <linux/delay.h>
#include "rdp_gpon.h"

static serdes_wan_type_t wan_serdes_type = SERDES_WAN_TYPE_NONE;
static unsigned short irq_test;

PMD_DEV_ENABLE_PRBS pmd_prbs_callback;

#pragma pack(push,1)
typedef struct
{
    uint32_t tx_lbe_bit_order:1;
    uint32_t tx_rd_pointer:5;
    uint32_t tx_wr_pointer:5;
    uint32_t tx_pointer_distances_max:5;
    uint32_t tx_pointer_distances_min:5;
    uint32_t clear_txfifo_drifted:1;
    uint32_t clear_txfifo_collision:1;
    uint32_t reserved2:7;
    uint32_t txfifo_reset:1;
    uint32_t reserved1:1;
} gearbox_cfg_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t reserved3:23;
    uint32_t tx_pointer_distances:5;
    uint32_t reserved2:1;
    uint32_t txfifo_drifted:1;
    uint32_t txfifo_collision:1;
    uint32_t reserved1:1;
} gearbox_status_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t reserved3:4;
    uint32_t ntr_sync_period_sel:4;
    uint32_t reserved2:4;
    uint32_t wan_debug_sel:4;
    uint32_t epon_debug_sel:8;
    uint32_t reserved1:1;
    uint32_t laser_oe:1;
    uint32_t mem_reb:1;
    uint32_t laser_invert:1;
    uint32_t laser_mode:2;
    uint32_t wan_interface_select:2;
} wan_cfg_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t pmd_core_mode:16;
    uint32_t onu2g_prtad:5;
    uint32_t pmd_ln_dp_h_rstb:1;
    uint32_t pmd_ln_h_rstb:1;
    uint32_t pmd_core_dp_h_rstb:1;
    uint32_t pmd_por_h_rstb:1;
    uint32_t mdio_mode:1;
    uint32_t refout_en:1;
    uint32_t refin_en:1;
    uint32_t pmd_ext_los:1;
    uint32_t mdio_fast_mode:1;
    uint32_t pmd_ln_tx_h_pwrdn:1;
    uint32_t pmd_ln_rx_h_pwrdn:1;
} onu2g_misc_ctrl_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t pm_lane_mode:16;
    uint32_t reserved:4;
    uint32_t pmd_tx_disable:1;
    uint32_t pmd_rx_mode:1;
    uint32_t pmd_tx_mode:2;
    uint32_t pmd_rx_osr_mode:4;
    uint32_t pmd_tx_osr_mode:4;
} onu2g_pmd_lane_ctrl_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t reserved:29;
    uint32_t pmi_lp_vldclr:1;
    uint32_t pmi_lp_write:1;
    uint32_t pmi_lp_en:1;
} onu2g_lp_ctrl_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t reserved:14;
    uint32_t pmi_lp_err_stat:1;
    uint32_t pmi_lp_ack_stat:1;
    uint32_t pmi_lp_rddata:16;
} onu2g_lp_return_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t reserved:22;
    uint32_t pmd_tx_burst_en_out:1;
    uint32_t pmi_lp_error:1;
    uint32_t pmi_lp_ack:1;
    uint32_t pmd_signal_detect:1;
    uint32_t pmd_energy_detect:1;
    uint32_t pmd_rx_lock:1;
    uint32_t pmd_tx_clk_vld:1;
    uint32_t pmd_rx_clk_vld:1;
    uint32_t pmd_pll_lock_loss:1;
    uint32_t pmd_pll_lock:1;
} onu2g_pmd_status_t;
#pragma pack(pop)

static void onu2g_pmd_wait_for_pll_lock(void)
{
    uint32_t retry = 2000;
    onu2g_pmd_status_t onu2g_pmd_status;

    do {
        READ_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_PMD_STATUS, onu2g_pmd_status);
        if (onu2g_pmd_status.pmd_pll_lock)
            break;
        udelay(10);
    } while (--retry);

    if (!retry)
        printk("ONU2G Error: onu2g_pmd_wait_for_pll_lock() reached maximum retries.\n");
}

#define USE_MDIO
#ifndef USE_MDIO
static void onu2g_lp_wait_for_ack_stat(void)
{
    uint32_t retry = 2000;
    onu2g_lp_return_t onu2g_lp_return;

    do {
        READ_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_LP_RETURN, onu2g_lp_return);
        if (onu2g_lp_return.pmi_lp_ack_stat)
            break;
        udelay(10);
    } while (--retry);

    if (!retry)
        printk("ONU2G Error: onu2g_lp_wait_for_ack_stat() reached maximum retries.\n");
}
#endif

void onu2g_write(uint32_t addr, uint16_t data)
{
#ifdef USE_MDIO
    mdio_write_c45_register(MDIO_INT, 0x00, 0x01, addr & 0xffff, data);
#else
    onu2g_lp_ctrl_t onu2g_lp_ctrl = {};

    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_LP_ADDR, addr);
    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_LP_WR_DATA, data);

    onu2g_lp_ctrl.pmi_lp_en = 1;
    onu2g_lp_ctrl.pmi_lp_write = 1;
    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_LP_CTRL, onu2g_lp_ctrl);
#endif
}
EXPORT_SYMBOL(onu2g_write);

void onu2g_read(uint32_t addr, uint16_t *data)
{
#ifdef USE_MDIO
    mdio_read_c45_register(MDIO_INT, 0x00, 0x01, addr & 0xffff, data);
#else
    onu2g_lp_ctrl_t onu2g_lp_ctrl = {};
    onu2g_lp_return_t onu2g_lp_return;

    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_LP_ADDR, addr);

    onu2g_lp_ctrl.pmi_lp_en = 1;
    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_LP_CTRL, onu2g_lp_ctrl);

    onu2g_lp_wait_for_ack_stat();

    READ_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_LP_RETURN, onu2g_lp_return);
    *data = onu2g_lp_return.pmi_lp_rddata;

    onu2g_lp_ctrl.pmi_lp_en = 0;
    onu2g_lp_ctrl.pmi_lp_vldclr = 1;
    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_LP_CTRL, onu2g_lp_ctrl);
#endif
}
EXPORT_SYMBOL(onu2g_read);

static void onu2g_init(serdes_wan_type_t wan_type)
{
    uint16_t data;
    uint32_t rescal;
    onu2g_misc_ctrl_t onu2g_misc_ctrl;
    onu2g_pmd_lane_ctrl_t onu2g_pmd_lane_ctrl;
    wan_cfg_t wan_cfg;

    /* Initialize rescal */
    rescal = 3;
    WRITE_32(0xb085a03c, rescal);
    rescal = 2;
    WRITE_32(0xb085a03c, rescal);
    udelay(500);
    READ_32(0xb085a050, rescal);
    if ((rescal & 1) != 1)
    {
      printk("%s: +++ rescal not initialized correctly: %x\n", __FUNCTION__, rescal);
      printk("+++ after rescal : %x\n", *((uint32_t *)0xb085a04c));
    }

    /* Assert POR reset by forcing pmd_por_h_rstb pin to 1'b0. Optionally if out of POR then core_s_rstb can be asserted by writing to 1'b0 to reset the whole core. */
    READ_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_MISC_CTRL ,onu2g_misc_ctrl);
    onu2g_misc_ctrl.pmd_ln_dp_h_rstb = 0;
    onu2g_misc_ctrl.pmd_core_dp_h_rstb = 0;
    onu2g_misc_ctrl.pmd_ln_h_rstb = 0;
    onu2g_misc_ctrl.pmd_por_h_rstb = 0;
    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_MISC_CTRL ,onu2g_misc_ctrl);

    /* Wait for stable refclk from crystal */
    udelay(50);
    
    /* De-assert pmd_por_h_rstb pin */
    READ_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_MISC_CTRL ,onu2g_misc_ctrl);
    onu2g_misc_ctrl.pmd_ln_dp_h_rstb = 1;
    onu2g_misc_ctrl.pmd_core_dp_h_rstb = 1;
    onu2g_misc_ctrl.pmd_ln_h_rstb = 1;
    onu2g_misc_ctrl.pmd_por_h_rstb = 1;
    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_MISC_CTRL ,onu2g_misc_ctrl);

    switch (wan_type)
    {
    case SERDES_WAN_TYPE_EPON_1G:
        onu2g_write(0x0800d004, 0x4020);
        onu2g_write(0x0800d004, 0x4220);
        onu2g_write(0x0800d0e4, 0x0001);

    case SERDES_WAN_TYPE_EPON_2G:
        onu2g_write(0x0800d127, 0x7005);
        break;
    case SERDES_WAN_TYPE_AE:
        onu2g_write(0x0800d127, 0x7005);
        onu2g_write(0x0800d004, 0x4020);
        onu2g_write(0x0800d004, 0x4220);
        break;
    case SERDES_WAN_TYPE_GPON:
        onu2g_write(0x0800d0b8, 0x0c70);
        onu2g_write(0x0800d0b7, 0x432C);
        onu2g_write(0x0800d0b8, 0xcc70);
        onu2g_write(0x0800d0e4, 0x0001);

        onu2g_read(0x0800d070, &data);
        data |= 0x0002;
        onu2g_write(0x0800d070, data);
        data |= 0x0001;
        onu2g_write(0x0800d070, data);
        break;
    default:
        break;
    }

    onu2g_write(0x0800d0b1, 0x8081);
    onu2g_write(0x0800d0b4, 0x6377);

    if (!irq_test)
    {
        onu2g_write(0x0800d094, 0xf000);
        onu2g_write(0x0800d093, 0x8);
        onu2g_write(0x0800d091, 0x2);
        onu2g_write(0x0800d0b3, 0x4);
        onu2g_write(0x0800d0b2, 0x0196);
        onu2g_write(0x0800d092, 0x2830);
    }
    else
    {
        onu2g_write(0x0800d094, 0x0);
        onu2g_write(0x0800d093, 0x0);
        onu2g_write(0x0800d091, 0x0);
        onu2g_write(0x0800d0b3, 0x0);
        onu2g_write(0x0800d0b2, 0x16);
        onu2g_write(0x0800d092, 0x2800);
    }

    switch (wan_type)
    {
    case SERDES_WAN_TYPE_EPON_2G:
        onu2g_write(0x0800d080, 0xc010);
        break;
    case SERDES_WAN_TYPE_EPON_1G:
    case SERDES_WAN_TYPE_AE:
        onu2g_write(0x0800d080, 0xc011);
        READ_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_PMD_LANE_CTRL, onu2g_pmd_lane_ctrl);
        onu2g_pmd_lane_ctrl.pmd_rx_osr_mode = 0x1;
        onu2g_pmd_lane_ctrl.pmd_tx_osr_mode = 0x1;
        WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_PMD_LANE_CTRL, onu2g_pmd_lane_ctrl);
        break;
    case SERDES_WAN_TYPE_GPON:
        onu2g_write(0x0800d080, 0xc010);
        break;
    default:
        break;
    }

    if (!irq_test)
    {
        onu2g_write(0x0800d074, 0x0);
        onu2g_write(0x0800d010, 0x208);
    }

    onu2g_read(0x0800d040, &data);
    onu2g_write(0x0800d040, data | 0x0010);
    onu2g_write(0x0800d0f4, 0x2271); /* Start PLL calibration */

    onu2g_pmd_wait_for_pll_lock(); 

    udelay(50);
    onu2g_write(0x0800d081, 0x0002);
    udelay(50);

    onu2g_write(0x0800d0a2, 0x000c); /* Correcting TX Amplitude to 1v */
    onu2g_write(0x0800d111, 0x7f);  /* Correcting amplitude */
    onu2g_write(0x0800d119, 0x00c0); /* best pre-emphasys */

    READ_32(WAN_MISC_WAN_TOP_WAN_MISC_WAN_CFG, wan_cfg);
    wan_cfg.mem_reb = 0;
    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_WAN_CFG, wan_cfg);

    wan_cfg.laser_oe = 1;
    switch (wan_type)
    {
    case SERDES_WAN_TYPE_EPON_2G:
    case SERDES_WAN_TYPE_EPON_1G:
        wan_cfg.laser_mode = 0x2;
        wan_cfg.wan_interface_select = 0x1;
        break;
    case SERDES_WAN_TYPE_AE:
        wan_cfg.laser_mode = 0x0;
        wan_cfg.wan_interface_select = 0x2;
        break;
    case SERDES_WAN_TYPE_GPON:
        wan_cfg.laser_mode = 0x1;
        wan_cfg.wan_interface_select = 0x0;
        break;
    default:
        break;
    }

    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_WAN_CFG, wan_cfg);
}

#define MAX_DISTANCE 27
#define MIN_DISTANCE 5

static void pon_reset_fifo(void)
{
    gearbox_cfg_t gc;

    wd_log("Reset FIFO\n");

    /* TX FIFO reset */
    READ_32(WAN_MISC_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG, gc);
    gc.tx_rd_pointer = 16; /* This will give us an actual distance of ~16 */
    gc.tx_wr_pointer = 0;
    gc.tx_pointer_distances_max = MAX_DISTANCE;
    gc.tx_pointer_distances_min = MIN_DISTANCE;
    /* 96848 specific */
    gc.clear_txfifo_drifted = gc.clear_txfifo_collision = gc.txfifo_reset = 1;
    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG, gc);
    gc.clear_txfifo_drifted = gc.clear_txfifo_collision = gc.txfifo_reset = 0;
    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG, gc);
    gc.clear_txfifo_drifted = 1;
    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG, gc);
    gc.clear_txfifo_drifted = 0;
    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG, gc);
}

static void pon_gearbox_drift_sample_check(void)
{
#define GB_FIFO_SIZE 32
#define SAMPLES 30
    gearbox_status_t st;
    int is_drift = FALSE, i, dist = 0, distances[GB_FIFO_SIZE] = {};
    static int prev_dist = 2 * GB_FIFO_SIZE; /* make sure first distance is reported */
    
    for (i = 0; i < SAMPLES; i++)
    {
        /* wd_log("i %d distance %d\n", i, st.tx_pointer_distances); */
        READ_32(WAN_MISC_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_STATUS, st);
        distances[st.tx_pointer_distances]++;
    }
    
    for (i = 0; i < GB_FIFO_SIZE; i++)
    {
        if (distances[i] > distances[dist])
            dist = i;
        if ((i < MIN_DISTANCE || i > MAX_DISTANCE) && distances[i])
        {
            wd_log("FIFO drift distance %d (%d)\n", i, distances[i]);
            is_drift = TRUE;
    }
    }

    if (abs(dist - prev_dist) > 1)
    {
        wd_log("FIFO distance changed %d (%d,%d,%d), previous=%d\n", dist,
            distances[ (dist - 1) % GB_FIFO_SIZE], distances[dist], distances[ (dist + 1) % GB_FIFO_SIZE], prev_dist);
        prev_dist = dist;
    }

    return st.txfifo_drifted;
}

static int pon_gearbox_drift_test(const int just_synced)
{
    gearbox_status_t st;

    wd_log_debug("fifo drift test\n");

    if (just_synced)
    {
        pon_reset_fifo();
        return FALSE;
    }

    READ_32(WAN_MISC_WAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_STATUS, st);
    if (st.txfifo_drifted)
    {
        wd_log("tx fifo drift indicationd\n");
    }

    pon_gearbox_drift_sample_check();

    return st.txfifo_drifted;
}

static void pon_reset_cdr(void)
{
    uint16_t data;

    wd_log_debug("Reset CDR\n");

    /* Reset CDR */
    onu2g_write(0x800d002, 0x690);
    onu2g_read(0x800d001, &data);
    onu2g_write(0x800d001, data | 1 << 7);
    udelay(100);
    onu2g_write(0x800d001, data);
}

static void onu2g_polarity_invert(void)
{
    uint16_t data;
    unsigned short polarity;

    /* rx_pmd_dp_invert */
    BpGetPmdInvSerdesRxPol(&polarity);

    if (polarity == pmd_polarity_invert)
    {
        onu2g_read(0x0800d0d3, &data);
        onu2g_write(0x0800d0d3, data | 0x1);
    }

    /* tx_pmd_dp_invert */
    BpGetPmdInvSerdesTxPol(&polarity);
    if (polarity == pmd_polarity_invert)
    {
        onu2g_read(0x0800d0e3, &data);
        onu2g_write(0x0800d0e3, data | 0x1);
    }
}

static void wan_burst_config(serdes_wan_type_t wan_type)
{
    if (wan_type == SERDES_WAN_TYPE_AE)
    {
        unsigned short polarity;

        bcm_set_pinmux(62,5);
        bcm_gpio_set_dir(62,1);
        if (BpGetAePolarity(&polarity) == BP_SUCCESS)
            bcm_gpio_set_data(62, polarity);
    }
    else
    {
        bcm_set_pinmux(62,1);
    }
}

static void clk_25mhz_disable(void)
{
    uint32_t data;
    int ret;

    ret = ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(xtal_control), (uint32 *)&data);
    if (ret)
        printk("Failed to ReadBPCMRegister CHIP_CLKRST block CLKRST_XTAL_CNTL. Error=%d\n", ret);

    data |= (0x1<<15);

    ret = WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST, CLKRSTBPCMRegOffset(xtal_control), data);
    if (ret)
        printk("Failed to writeBPCMRegister CHIP_CLKRST block CLKRST_XTAL_CNTL. Error=%d\n", ret);
}


void config_wan_ewake(uint8_t toff_time, uint8_t setup_time, uint8_t hold_time)
{
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_EARLY_TXEN_CFG, 0, 1 ,0 );            //  EARLY_TXEN_BYPASS
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_EARLY_TXEN_CFG, 8, 8 ,toff_time );    //  TOFF_TIME
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_EARLY_TXEN_CFG, 16, 8 ,setup_time );  //  SETUP_TIME
    FIELD_MWRITE_32(WAN_MISC_RDP_WAN_TOP_WAN_MISC_EARLY_TXEN_CFG, 24, 8 ,hold_time );   //  HOLD_TIME
}

void enable_jitter_filter (int filter_state)
{
    uint16_t data ;

  
    onu2g_read(0x0800d070, &data);
    if (filter_state)
    {
        data |= 0x2;
    }
    else
    {
        data &= ~0x2;
    }
    onu2g_write(0x0800d070, data);
}

void wan_reset_rx_and_tx(void)
{
    enable_jitter_filter(0);
    pon_reset_cdr();
    udelay(2000);
    pon_reset_fifo();
    enable_jitter_filter(1);
}
EXPORT_SYMBOL(wan_reset_rx_and_tx);


int wan_serdes_config(serdes_wan_type_t wan_type)
{
    unsigned short optics_type;

    irq_test = !(((*(uint32_t *)0xb0800000) & 0xff) == 160);
    if (wan_type == SERDES_WAN_TYPE_GPON)
    {
        pon_serdes_lof_fixup_fifo_t fifo_cb =
        {
            .gearbox_drift_test = &pon_gearbox_drift_test,
            .reset_fifo = &wan_reset_rx_and_tx,
        };
        pon_serdes_lof_fixup_cfg(&fifo_cb, &pon_reset_cdr);
    }

    onu2g_init(wan_type);
    wan_burst_config(wan_type);

    if (bcm_i2c_pon_optics_type_get(&optics_type) == BP_SUCCESS && optics_type == BCM_I2C_PON_OPTICS_TYPE_PMD)
    {
        onu2g_polarity_invert();
        config_wan_ewake(PMD_EWAKE_OFF_TIME, PMD_EWAKE_SETUP_TIME, PMD_EWAKE_HOLD_TIME);
    }
    else
        clk_25mhz_disable();

    if (wan_type == SERDES_WAN_TYPE_AE)
        bcm_i2c_optics_tx_control(BCM_I2C_OPTICS_ENABLE);

    wan_serdes_type = wan_type;

    return 0;
}
EXPORT_SYMBOL(wan_serdes_config);

serdes_wan_type_t wan_serdes_type_get(void)
{
    return wan_serdes_type;
}
EXPORT_SYMBOL(wan_serdes_type_get);

void pon_wan_top_reset_gearbox_tx()
{
    //TODO
    return;
}
EXPORT_SYMBOL(pon_wan_top_reset_gearbox_tx);

void wan_register_pmd_prbs_callback(PMD_DEV_ENABLE_PRBS callback)
{
    pmd_prbs_callback = callback;
}
EXPORT_SYMBOL(wan_register_pmd_prbs_callback);

void wan_prbs_status(bdmf_boolean *valid, uint32_t *errors)
{
    uint16_t data;
    uint16_t lock_lost;
    uint16_t lock;

    /* Check for PRBS lock */
    onu2g_read(0x0800d0d9, &data);
    lock = data & 0x1;

    /* Check for PRBS lock lost*/
    onu2g_read(0x0800d0da, &data);
    lock_lost = (data & 0x8000) >> 15;

    *errors = data & 0x7fff;
    onu2g_read(0x0800d0db, &data);
    *errors = ((*errors << 16 )| data) / 3;

    if (lock && !lock_lost && *errors == 0)
        *valid = 1;
    else
        *valid = 0;
}
EXPORT_SYMBOL(wan_prbs_status);

void wan_prbs_gen(uint32_t enable, int enable_host_tracking, int mode, serdes_wan_type_t wan_type, bdmf_boolean *valid)
{
    uint32_t data = 0x0;
    uint32_t prbs_chk_mode_sel;
    uint16_t    transceiver;
    uint32_t errors;
    onu2g_pmd_lane_ctrl_t onu2g_pmd_lane_ctrl;

    bcm_i2c_optics_tx_control(BCM_I2C_OPTICS_ENABLE);

    bcm_i2c_pon_optics_type_get(&transceiver);
    if (transceiver == BCM_I2C_PON_OPTICS_TYPE_PMD && enable)
    {   
        if (pmd_prbs_callback)
            pmd_prbs_callback((uint16_t)enable, 1);
    }   

    if (wan_type != SERDES_WAN_TYPE_AE)
    {
        if (enable)
        {
            bcm_set_pinmux(62, 5);
            bcm_gpio_set_dir(62, 1);
            bcm_gpio_set_data(62, 1);
        }
        else
        {
            bcm_set_pinmux(62, 1);
        }
    }

    if (!enable)
    {
        /* Disable PRBS */
        onu2g_write(0x0800d0e1, 0x0);
        onu2g_write(0x0800d0d1, 0x0);
        *valid = 0;

        if (transceiver == BCM_I2C_PON_OPTICS_TYPE_PMD)

        {   
            if (pmd_prbs_callback)
                pmd_prbs_callback((uint16_t)enable, 1);
        }   

        /* recover to init value, onu2g_init, in order to resync to OLT after prbs test */
        if ((wan_type == SERDES_WAN_TYPE_EPON_1G) ||
            (wan_type == SERDES_WAN_TYPE_AE))
        {
            READ_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_PMD_LANE_CTRL, onu2g_pmd_lane_ctrl);
            onu2g_pmd_lane_ctrl.pmd_rx_osr_mode = 0x1;
            onu2g_pmd_lane_ctrl.pmd_tx_osr_mode = 0x1;
            WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_PMD_LANE_CTRL, onu2g_pmd_lane_ctrl);
        }
        return;
    }

    if (mode == 0) /* PRBS 7 */
        prbs_chk_mode_sel = 0;
    else if (mode == 1) /* PRBS 15 */
        prbs_chk_mode_sel = 3;
    else if (mode == 2) /* PRBS 23 */
        prbs_chk_mode_sel = 4;
    else if (mode == 3) /* PRBS 31 */
        prbs_chk_mode_sel = 5;
    else
    {
        printk("Error: wan_prbs_gen unknown mode %d\n", mode);
        return;
    }

    WRITE_32(WAN_MISC_WAN_TOP_WAN_MISC_ONU2G_PMD_LANE_CTRL, data);

    /* Enable PRBS */
    onu2g_write(0x0800d0e1, 0x0001 | prbs_chk_mode_sel<<1);

    /* Enable PRBS checker */
    onu2g_write(0x0800d0d1, 0x0021 | prbs_chk_mode_sel<<1);
    udelay(100);

    wan_prbs_status(valid, &errors);
}
EXPORT_SYMBOL(wan_prbs_gen);

/* Should be called before optical module TX power is enabled / MAC is set to transmit */
void wan_top_transmitter_control(enum transmitter_control mode)
{
}
EXPORT_SYMBOL(wan_top_transmitter_control);

