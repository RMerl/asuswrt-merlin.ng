// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   */
/*
 * xport_drv.c
 *
 */

//includes
#include <linux/delay.h>
#include "xport_ag.h"
#include "xport_drv.h"
#include "xport_intr.h"

#define UDELAY udelay

void __iomem *xport_phys_base;
void __iomem *xport_virt_base;
void __iomem *xlif_virt_base;

static void remap_ru_block_single_virtbase(const ru_block_rec *ru_block, uintptr_t virt_offset)
{
    uint32_t addr_itr;
    uintptr_t phys_addr, virt_addr;

    for (addr_itr = 0; addr_itr < ru_block->addr_count; addr_itr++)
    {
        phys_addr = ru_block->addr[addr_itr];
        virt_addr = ru_block->addr[addr_itr] + virt_offset;
        ru_block->addr[addr_itr] = virt_addr;

        pr_debug("%-11s %1d Remapped physical 0x%lx to virtual 0x%lx\n",
            ru_block->name, addr_itr, phys_addr, virt_addr);
    }
}

/* This function is a utility to fix virtual address of the RU/HAL based driver.
 * After calling this function all RU addresses will turn virtual and no translation is needed during read/write
 * ru_blocks[] is the main ru object of the xport block
 */
static void remap_ru_block_addrs(void)
{
    const ru_block_rec **ru_blocks = RU_XPORT_BLOCKS;
    uint32_t blk;
    uintptr_t virt_offset = xport_virt_base - xport_phys_base;

    for (blk = 0; ru_blocks[blk]; blk++)
    {
        remap_ru_block_single_virtbase(ru_blocks[blk], virt_offset);
    }
}

static int xport_probe(dt_device_t *pdev)
{
    int ret;

    xport_virt_base = dt_dev_remap(pdev, 0);
    if (IS_ERR(xport_virt_base))
    {
        ret = PTR_ERR(xport_virt_base);
        xport_virt_base = NULL;
        goto Exit;
    }

    xlif_virt_base = dt_dev_remap(pdev, 1);
    if (IS_ERR(xlif_virt_base))
    {
        ret = PTR_ERR(xlif_virt_base);
        xlif_virt_base = NULL;
        goto Exit;
    }

    xport_phys_base = dt_dev_read_addr(pdev ,0);
    if (!xport_phys_base)
    {
        ret = -1;
        goto Exit;
    }

    remap_ru_block_addrs();

    dev_info(&pdev->dev, "registered\n");

    return 0;

Exit:
    return ret;
}

static const struct udevice_id xport_ids[] = {
    { .compatible = "brcm,xport" },
    { /* end of list */ },
};

U_BOOT_DRIVER(brcm_xport) = {
    .name	= "brcm-xport",
    .id	= UCLASS_MISC,
    .of_match = xport_ids,
    .probe = xport_probe,
};

typedef struct
{
    uint32_t if_enable; 
    uint32_t read_credits; 
    uint32_t set_credits; 
    uint32_t out_ctrl; 
    uint32_t urun_port_enable;
    uint32_t tx_threshold;
} xlif_tx_if_t;

typedef struct
{
    uint32_t xlif_rx_if_channel[3];             // 0x00 - 0x0b
    uint32_t xlif_rsvd_1[5];                    // 0x0c - 0x1f
    uint32_t xlif_rx_flow_control_channel[2];   // 0x20 - 0x27
    uint32_t xlif_rsvd_2[6];                    // 0x28 - 0x3f
    xlif_tx_if_t xlif_tx_if_channel;            // 0x40 - 0x57
    uint32_t xlif_rsvd_3[2];                    // 0x58 - 0x5f
    uint32_t xlif_tx_flow_control_channel[2];   // 0x60 - 0x67
    uint32_t xlif_rsvd_4[2];                    // 0x68 - 0x6f
    uint32_t debug_bus_channel[1];              // 0x70 - 0x73
    uint32_t xlif_rsvd_5[1];                    // 0x74 - 0x77
    uint32_t xlif_eee_channel[1];               // 0x78 - 0x7b
    uint32_t q_off_channel[1];                  // 0x7c - 0x7f
    uint32_t xlif_rsvd_6[96];                   // 0x80 - 0x67f

} xlif_ch_t;

#define XLIF_REG ((volatile xlif_ch_t *)xlif_virt_base)

static int validate_xport_configuration(xport_xlmac_port_info_s *init_params)
{
    if (XPORT_PORT_VALID(init_params->xport_port_id))
    {
        return XPORT_ERR_OK;
    }
    return XPORT_ERR_INVALID;
}

static int xport_speed_to_xlmac(XPORT_PORT_RATE xport_speed)
{
    int ret = 0;

    switch (xport_speed)
    {
    case XPORT_RATE_10MB:
        ret = XLMAC_PORT_SPEED_10MB;
        break;
    case XPORT_RATE_100MB:
        ret = XLMAC_PORT_SPEED_100MB;
        break;
    case XPORT_RATE_1000MB:
        ret = XLMAC_PORT_SPEED_1000MB;
        break;
    case XPORT_RATE_2500MB:
        ret = XLMAC_PORT_SPEED_2500MB;
        break;
    case XPORT_RATE_5G:
#if !defined(CONFIG_BCM963158)
        ret = XLMAC_PORT_SPEED_5G;
#else
        /* 5G LED does not work, so borrow 10M LED color which is not used in real world */
        ret = XLMAC_PORT_SPEED_10MB;
#endif
        break;
    case XPORT_RATE_10G:
        ret = XLMAC_PORT_SPEED_10G;
        break;
    case XPORT_RATE_UNKNOWN:
    default:
        __xportError("Wrong xport speed %d\n", xport_speed);
    }

    return ret;
}

static XPORT_PORT_RATE xport_speed_get(uint8_t portid)
{
    uint8_t speed_mode, no_sop_for_crc_hg, hdr_mode;
    XPORT_PORT_RATE xport_speed = XPORT_RATE_UNKNOWN;
    
    if (ag_drv_xport_xlmac_core_mode_get(portid, &speed_mode, &no_sop_for_crc_hg, &hdr_mode))
        return XPORT_RATE_UNKNOWN;
    
    switch (speed_mode)
    {
    case XLMAC_PORT_SPEED_10MB: /* 10M */
        xport_speed = XPORT_RATE_5G; /* Since we are reading LED register, 10M means 5G speed */
        break;

    case XLMAC_PORT_SPEED_100MB: /* 100Mbps */
        xport_speed = XPORT_RATE_100MB;
        break;

    case XLMAC_PORT_SPEED_1000MB: /* 1000Mbps */
        xport_speed = XPORT_RATE_1000MB;
        break;

    case XLMAC_PORT_SPEED_2500MB: /* 2500Mbps */
        xport_speed = XPORT_RATE_2500MB;
        break;

    case XLMAC_PORT_SPEED_5G: /* 5Gbps */
        xport_speed = XPORT_RATE_5G;
        break;

    case XLMAC_PORT_SPEED_10G: /* 10G */
        xport_speed = XPORT_RATE_10G;
        break;

    default:
        xport_speed = XPORT_RATE_UNKNOWN;
    }
    
    return xport_speed;
}

#if !defined(CONFIG_BCM963158)
static int xport_msbus_reset(xport_xlmac_port_info_s *init_params)
{
    uint8_t tx_credit_disab, tx_fifo_rst, tx_port_rst, rx_port_rst;
    int rc = XPORT_ERR_OK;
    int xlmac_num = PID_XLMAC_NUM(init_params->xport_port_id);
    int xport_num = PID_XPORT_NUM(init_params->xport_port_id);

    rc = rc ? rc : ag_drv_xport_mab_ctrl_get(xlmac_num, &tx_credit_disab, &tx_fifo_rst, &tx_port_rst, &rx_port_rst);

    rx_port_rst |= (1 << xport_num);
    tx_port_rst |= (1 << xport_num);

    rc = rc ? rc : ag_drv_xport_mab_ctrl_set(xlmac_num, tx_credit_disab, tx_fifo_rst, tx_port_rst, rx_port_rst);

    __xportDebug("DONE (%d)\n", rc);

    return rc;
}
#else
static int xport_msbus_reset(xport_xlmac_port_info_s *init_params)
{
    xport_mab_ctrl ctrl_xlmac;
    int rc = XPORT_ERR_OK;
    XPORT_PORT_ID xlmac_port = init_params->xport_port_id;

    rc = rc ? rc : ag_drv_xport_mab_ctrl_get(&ctrl_xlmac);

    if (xlmac_port == XPORT_PORT_ID_AE)
    {
        ctrl_xlmac.xgmii_rx_rst = 1;
        ctrl_xlmac.xgmii_tx_rst = 1;
    }

    ctrl_xlmac.gmii_rx_rst |= (1 << xlmac_port);
    ctrl_xlmac.gmii_tx_rst |= (1 << xlmac_port);

    rc = rc ? rc : ag_drv_xport_mab_ctrl_set(&ctrl_xlmac);

    __xportDebug("DONE (%d)\n", rc);

    return rc;
}
#endif

static int xport_xlmac_reset(xport_xlmac_port_info_s *init_params)
{
    xport_xlmac_core_ctrl ctrl;
    int rc = XPORT_ERR_INVALID;
    if (XPORT_PORT_VALID(init_params->xport_port_id))
    {
        rc = ag_drv_xport_xlmac_core_ctrl_get(init_params->xport_port_id, &ctrl);
        ctrl.soft_reset = 1; /* Keep the XLMAC Port in soft reset */
        rc = rc ? rc : ag_drv_xport_xlmac_core_ctrl_set(init_params->xport_port_id, &ctrl);
    }
    __xportDebug("DONE (%d)\n", rc);

    return rc;
}

static int xlif_reset_credit(int channel)
{
    volatile xlif_ch_t *p_xlif_ch = XLIF_REG;
    p_xlif_ch[channel].xlif_tx_if_channel.set_credits = (1<<12);
    __xportDebug("DONE (channel = %d)\n", channel);
    return 0;
}

static int xport_xlif_reset(xport_xlmac_port_info_s *init_params)
{
    int rc = XPORT_ERR_INVALID;
    if (XPORT_PORT_VALID(init_params->xport_port_id))
    {
        rc = xlif_reset_credit(init_params->xport_port_id);
    }
    __xportDebug("DONE (%d)\n", rc);
    return rc;
}

static int xlif_release_credit(int channel)
{
    volatile xlif_ch_t *p_xlif_ch = XLIF_REG;
    p_xlif_ch[channel].xlif_tx_if_channel.set_credits &= ~(1<<12);
    __xportDebug("DONE (channel = %d)\n", channel);
    return 0;
}

static int xport_xlif_release(xport_xlmac_port_info_s *init_params)
{
    int rc = XPORT_ERR_INVALID;
    if (XPORT_PORT_VALID(init_params->xport_port_id))
    {
        rc = xlif_release_credit(init_params->xport_port_id);
    }
    __xportDebug("DONE (%d)\n", rc);
    return rc;
}

static int xport_xlmac_release_reset(xport_xlmac_port_info_s *init_params)
{
    xport_xlmac_core_ctrl ctrl;
    int rc = XPORT_ERR_INVALID;
    if (XPORT_PORT_VALID(init_params->xport_port_id))
    {
        rc = ag_drv_xport_xlmac_core_ctrl_get(init_params->xport_port_id, &ctrl);
        ctrl.soft_reset = 0; /* Release the XLMAC Port soft reset */
        rc = rc ? rc : ag_drv_xport_xlmac_core_ctrl_set(init_params->xport_port_id, &ctrl);
    }
    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

static int xport_xlmac_init(xport_xlmac_port_info_s *init_params)
{
    xport_xlmac_core_ctrl     ctrl;
    xport_xlmac_core_tx_ctrl  tx_ctrl;
    xport_xlmac_core_rx_ctrl  rx_ctrl;
    xport_xlmac_core_pfc_ctrl pfc_ctrl;

    uint32_t port_id;
    int rc = XPORT_ERR_OK;
    int xgmii = init_params->xgmii_mode;

    /* XPORT-XLMAC should be in reset during init */

    if (!XPORT_PORT_VALID(init_params->xport_port_id))
        return XPORT_ERR_INVALID;

    /* Only initialize the port in use */
    port_id = init_params->xport_port_id;

    /* Enable 2.5G/10G AE PFC_STATS_EN for Hardware work around */
    rc = rc? rc: ag_drv_xport_xlmac_core_pfc_ctrl_get(port_id, &pfc_ctrl);
    pfc_ctrl.pfc_stats_en = 1;  /* Work around for HW issue */
    rc = rc? rc: ag_drv_xport_xlmac_core_pfc_ctrl_set(port_id, &pfc_ctrl);

#if !defined(CONFIG_BCM963158)
    {
        int xlmac_num = PID_XLMAC_NUM(init_params->xport_port_id);
        uint8_t p3_mode, p2_mode, p1_mode, p0_mode;
        rc = rc? rc: ag_drv_xport_top_ctrl_get(xlmac_num, &p3_mode, &p2_mode, &p1_mode, &p0_mode);
        switch (PID_XPORT_NUM(init_params->xport_port_id))
        {
        case 0: p0_mode = xgmii; break;
        case 1: p1_mode = xgmii; break;
        case 2: p2_mode = xgmii; break;
        case 3: p3_mode = xgmii; break;
        }
        rc = rc? rc: ag_drv_xport_top_ctrl_set(xlmac_num, p3_mode, p2_mode, p1_mode, p0_mode);
    }
#else
    /* XPORT_REG_XPORT_CNTRL_1 */
    rc = rc ? rc : ag_drv_xport_reg_xport_cntrl_1_set(
            (init_params->xport_port_id == XPORT_PORT_ID_AE) ? 1 : 0 /* msbus_clk_sel only uses 644MHz if p0 is in use */,
                                                      0 /* wan_led0_sel - keep default. Need update if design share LED for both wan ports*/, 
                                                      0 /* timeout_rst_disable */, 
                                                      xgmii?1:0 /* p0_mode; 0=GMII, 1=XGMII */);
#endif


    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_ctrl_get(port_id, &tx_ctrl);

    tx_ctrl.crc_mode = XLMAC_TX_CTRL_CRC_MODE_PER_PKT /* = 3 (not defined in RDB) */;
    tx_ctrl.pad_en = 1;
    tx_ctrl.tx_threshold = 2; /* TX_THRESHOLD depends on how "fast" XRDP writes into XPORT. 
                                 If any underflow in XLMAC we can increase the value. */
    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_ctrl_set(port_id, &tx_ctrl);


    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_ctrl_get(port_id, &rx_ctrl);
    rx_ctrl.rx_any_start = 0;
    rx_ctrl.strip_crc = 0; /* CRC will be validated by the BBH */
    rx_ctrl.strict_preamble = 0; /* Keep this disabled otherwise interop issue with some bad equipment */
    rx_ctrl.runt_threshold = 0x40;
    rx_ctrl.rx_pass_ctrl = 1;
    rx_ctrl.rx_pass_pause = 0; /* By default do not pass RX pause to XRDP */
    rx_ctrl.rx_pass_pfc = 0;   /* By default do not pass RX PFC to XRDP */
    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_ctrl_set(port_id, &rx_ctrl);

    rc = ag_drv_xport_xlmac_core_ctrl_get(port_id, &ctrl);
    ctrl.extended_hig2_en = 0; /* By default - do not enable hig2 */
    rc = rc ? rc : ag_drv_xport_xlmac_core_ctrl_set(port_id, &ctrl);
    {
        uint8_t speed_mode, no_sop_for_crc_hg, hdr_mode;
        rc = rc ? rc : ag_drv_xport_xlmac_core_mode_get(port_id, &speed_mode, &no_sop_for_crc_hg, &hdr_mode);
        no_sop_for_crc_hg = 0;
        hdr_mode = 0;
        /* Configure the SPEED_MODE in XLMAC_MODE register based on link speed detected */
        speed_mode = xport_speed_to_xlmac(init_params->port_rate);
        rc = rc ? rc : ag_drv_xport_xlmac_core_mode_set(port_id, speed_mode, no_sop_for_crc_hg, hdr_mode);
    }
    /* Release XPORT reset  */
    xport_xlmac_release_reset(init_params);

    __xportDebug("rc = %d; port = %d spd = %s dup = %d\n",
                  rc, init_params->xport_port_id,
                  xport_rate_to_str(init_params->port_rate),init_params->port_duplex);
    return rc;
}

#if !defined(CONFIG_BCM963158)
static int xport_msbus_init(xport_xlmac_port_info_s *init_params)
{
    uint8_t tx_credit_disab, tx_fifo_rst, tx_port_rst, rx_port_rst;
    xport_mab_tx_wrr_ctrl tx_wrr_ctrl;
    int xlmac_num = PID_XLMAC_NUM(init_params->xport_port_id);
    int xport_num = PID_XPORT_NUM(init_params->xport_port_id);
    int rc = XPORT_ERR_OK;

    /* Keep MSBUS TX arbitrater ARB_Mode to HW defaults
     * Set unused port weight to ZERO */
    rc = rc ? rc : ag_drv_xport_mab_tx_wrr_ctrl_get(xlmac_num, &tx_wrr_ctrl);
    tx_wrr_ctrl.arb_mode  = 0; /* Fixed mode */
    tx_wrr_ctrl.p0_weight = 1;
    tx_wrr_ctrl.p1_weight = 1;
    tx_wrr_ctrl.p2_weight = 1;
    tx_wrr_ctrl.p3_weight = 1;
    rc = rc ? rc : ag_drv_xport_mab_tx_wrr_ctrl_set(xlmac_num, &tx_wrr_ctrl);

    rc = rc ? rc : ag_drv_xport_mab_ctrl_get(xlmac_num, &tx_credit_disab, &tx_fifo_rst, &tx_port_rst, &rx_port_rst);

    tx_credit_disab &= ~(1 << xport_num);
    tx_fifo_rst &= ~(1 << xport_num);
    rx_port_rst &= ~(1 << xport_num);
    tx_port_rst &= ~(1 << xport_num);

    rc = rc ? rc : ag_drv_xport_mab_ctrl_set(xlmac_num, tx_credit_disab, tx_fifo_rst, tx_port_rst, rx_port_rst);

    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}
#else
static int xport_msbus_init(xport_xlmac_port_info_s *init_params)
{
    xport_mab_ctrl ctrl_xlmac;
    xport_mab_tx_wrr_ctrl tx_wrr_ctrl;
    XPORT_PORT_ID xlmac_port = init_params->xport_port_id;
    int rc = XPORT_ERR_OK;

    /* Keep MSBUS TX arbitrater ARB_Mode to HW defaults
     * Set unused port weight to ZERO */
    rc = rc ? rc : ag_drv_xport_mab_tx_wrr_ctrl_get(&tx_wrr_ctrl);
    tx_wrr_ctrl.arb_mode  = 0; /* Fixed mode */
    tx_wrr_ctrl.p0_weight = 3; /* potentially 10G */
    tx_wrr_ctrl.p1_weight = 1;
    tx_wrr_ctrl.p2_weight = 0;
    tx_wrr_ctrl.p3_weight = 0;
    rc = rc ? rc : ag_drv_xport_mab_tx_wrr_ctrl_set(&tx_wrr_ctrl);


    rc = rc ? rc : ag_drv_xport_mab_ctrl_get(&ctrl_xlmac);
    if (xlmac_port == XPORT_PORT_ID_AE)
    {
        ctrl_xlmac.xgmii_rx_rst = 0;
        ctrl_xlmac.xgmii_tx_rst = 0;
    }

    ctrl_xlmac.gmii_rx_rst &= ~(1 << xlmac_port);
    ctrl_xlmac.gmii_tx_rst &= ~(1 << xlmac_port);

    rc = rc ? rc : ag_drv_xport_mab_ctrl_set(&ctrl_xlmac);

    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}
#endif

static int xport_reset(xport_xlmac_port_info_s *init_params)
{
    int rc;
    rc = xport_msbus_reset(init_params);
    rc = rc ? rc : xport_xlmac_reset(init_params);
    rc = rc ? rc : xport_xlif_reset(init_params);
    __xportInfo("rc = %d; port = %d spd = %s dup = %d\n",
                  rc, init_params->xport_port_id,
                  xport_rate_to_str(init_params->port_rate),init_params->port_duplex);
    return rc;
}

#if !defined(CONFIG_BCM963158)
static int xport_portreset_ctrl_set(int xport_port_id, int reset)
{
    int xlmac_num = PID_XLMAC_NUM(xport_port_id);
    int rc = 0;

    switch (PID_XPORT_NUM(xport_port_id))
    {
        case 0: rc = rc ? rc : ag_drv_xport_portreset_p0_ctrl_set(xlmac_num, reset); break;
        case 1: rc = rc ? rc : ag_drv_xport_portreset_p1_ctrl_set(xlmac_num, reset); break;
        case 2: rc = rc ? rc : ag_drv_xport_portreset_p2_ctrl_set(xlmac_num, reset); break;
        case 3: rc = rc ? rc : ag_drv_xport_portreset_p3_ctrl_set(xlmac_num, reset); break;
    }
    return rc;
}
#endif

static int xport_init(xport_xlmac_port_info_s *init_params)
{
    int rc = 0;

#if !defined(CONFIG_BCM963158)
    rc = xport_portreset_ctrl_set(init_params->xport_port_id, 1); /* Reset XPORT state machine by software with hw automatic reset disabled */
#endif

    rc = rc ? rc : xport_xlif_release(init_params);
    rc = rc ? rc : xport_xlmac_init(init_params);
    rc = rc ? rc : xport_msbus_init(init_params);

#if !defined(CONFIG_BCM963158)
    {
        int xlmac_num = PID_XLMAC_NUM(init_params->xport_port_id);
        int xport_num = PID_XPORT_NUM(init_params->xport_port_id);
        uint8_t link_status = 0;
        uint32_t retries = 1000;

        do
        {
            UDELAY(1000);
            ag_drv_xport_top_status_get(xlmac_num, &link_status);
        } while (!(link_status & (1 << xport_num)) && --retries);

        if (!link_status)
        {
            __xportError("Time out waiting for link up on port %d\n", init_params->xport_port_id);
            return -1;
        }
	    rc = rc ? rc : xport_portreset_ctrl_set(init_params->xport_port_id, 0);
    }
#endif

    __xportInfo("rc = %d; port = %d spd = %s dup = %d\n",
                  rc, init_params->xport_port_id,
                  xport_rate_to_str(init_params->port_rate),init_params->port_duplex);
    return rc;
}

#if !defined(CONFIG_BCM963158)
static int xport_platform_init(xport_xlmac_port_info_s *init_params)
{
    int rc = 0;
    int xlmac_num = PID_XLMAC_NUM(init_params->xport_port_id);
    int xport_num = PID_XPORT_NUM(init_params->xport_port_id);

    // do per port init 
    if (xport_num == 0)
    {
        uint8_t link_down_rst_en, enable_sm_run;
        uint16_t tick_timer_ndiv;
        xport_portreset_sig_en sig_en;

        //reset SM setting
        link_down_rst_en = 0x0; /* Disable HW automatic state machine reset to avoid link flipping failure */
        enable_sm_run = 0; tick_timer_ndiv = 0;
        rc = ag_drv_xport_portreset_config_set(xlmac_num, link_down_rst_en, enable_sm_run, tick_timer_ndiv);
        UDELAY(1000);
        tick_timer_ndiv = 0xfa;
        rc = rc ? rc : ag_drv_xport_portreset_config_set(xlmac_num, link_down_rst_en, enable_sm_run, tick_timer_ndiv);

        sig_en.enable_xlmac_rx_disab = sig_en.enable_xlmac_tx_disab = sig_en.enable_xlmac_tx_discard = sig_en.enable_xlmac_soft_reset = 1;
        sig_en.enable_mab_rx_port_init = sig_en.enable_mab_tx_port_init = sig_en.enable_mab_tx_credit_disab = sig_en.enable_mab_tx_fifo_init = 1;
        sig_en.enable_port_is_under_reset = 1;
        sig_en.enable_xlmac_ep_discard = 0;
        rc = rc ? rc : ag_drv_xport_portreset_p0_sig_en_set(xlmac_num, &sig_en);
        rc = rc ? rc : ag_drv_xport_portreset_p1_sig_en_set(xlmac_num, &sig_en);
        rc = rc ? rc : ag_drv_xport_portreset_p2_sig_en_set(xlmac_num, &sig_en);
        rc = rc ? rc : ag_drv_xport_portreset_p3_sig_en_set(xlmac_num, &sig_en);
        UDELAY(5000);

        enable_sm_run = 0xf;
        rc = rc ? rc : ag_drv_xport_portreset_config_set(xlmac_num, link_down_rst_en, enable_sm_run, tick_timer_ndiv);
    }
    // do per channel init
    {   xport_xlmac_core_rx_lss_ctrl rx_lss_ctrl;
        rc = rc ? rc : ag_drv_xport_xlmac_core_rx_lss_ctrl_get(init_params->xport_port_id, &rx_lss_ctrl);
        rx_lss_ctrl.local_fault_disable = 1;
        rc = rc ? rc : ag_drv_xport_xlmac_core_rx_lss_ctrl_set(init_params->xport_port_id, &rx_lss_ctrl);
    }
    {   xport_xlmac_core_tx_ctrl ctrl;
        rc = rc ? rc : ag_drv_xport_xlmac_core_tx_ctrl_get(init_params->xport_port_id, &ctrl);
        ctrl.average_ipg = 11;
        rc = rc ? rc : ag_drv_xport_xlmac_core_tx_ctrl_set(init_params->xport_port_id, &ctrl);
    }
    {   uint32_t prog_tx_crc; uint8_t tx_crc_corruption_mode; uint8_t tx_crc_corrupt_en; uint8_t tx_err_corrupts_crc;
        rc = rc ? rc : ag_drv_xport_xlmac_core_tx_crc_corrupt_ctrl_get(init_params->xport_port_id,&prog_tx_crc,&tx_crc_corruption_mode,&tx_crc_corrupt_en,&tx_err_corrupts_crc);
        tx_crc_corrupt_en = 0; tx_err_corrupts_crc = 0;
        rc = rc ? rc : ag_drv_xport_xlmac_core_tx_crc_corrupt_ctrl_set(init_params->xport_port_id, prog_tx_crc, tx_crc_corruption_mode, tx_crc_corrupt_en, tx_err_corrupts_crc);
    }
#if 0    
    {   xport_xlmac_core_rx_ctrl rx_ctrl;
        rc = rc ? rc : ag_drv_xport_xlmac_core_rx_ctrl_get(init_params->xport_port_id,&rx_ctrl);
        rx_ctrl.rx_pass_pfc = 1; rx_ctrl.rx_pass_pause = 1; 
        rc = rc ? rc : ag_drv_xport_xlmac_core_rx_ctrl_set(init_params->xport_port_id,&rx_ctrl);
    }
#endif

    return rc;
}
#endif

int xport_init_driver(xport_xlmac_port_info_s *init_params)
{
    if (validate_xport_configuration(init_params))
    {
        pr_err("XPORT configuration validation failed\n");
        return XPORT_ERR_PARAM;
    }

#if !defined(CONFIG_BCM963158)
    xport_platform_init(init_params);
#endif

    /* Serdes insert/remove, PHY/RGMII link up/down interrupts could be registered here */

    if (xport_reset(init_params))
        return XPORT_ERR_PARAM;

    __xportDebug("DONE \n");
    return XPORT_ERR_OK;
}

int xport_handle_link_up(xport_xlmac_port_info_s *p_info)
{
    int rc = 0;

    rc = rc ? rc : xport_init(p_info);
    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

int xport_handle_link_dn(xport_xlmac_port_info_s *p_info)
{
    int rc = 0;
    rc = rc ? rc : xport_reset(p_info);
    UDELAY(1000);
    __xportDebug("DONE (rc = %d)\n", rc);
    //return xport_reset(&local_xport_cfg);
    return rc;
}
int xport_get_port_configuration(uint32_t portid, xport_port_cfg_s *port_conf)
{
    int rc;
    xport_xlmac_core_ctrl xlmac_ctrl;
    xport_xlmac_core_tx_ctrl xlmac_tx_ctrl;
    xport_xlmac_core_rx_ctrl xlmac_rx_ctrl;

    rc = ag_drv_xport_xlmac_core_ctrl_get(portid, &xlmac_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_ctrl_get(portid, &xlmac_tx_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_ctrl_get(portid, &xlmac_rx_ctrl);
    port_conf->average_igp = xlmac_tx_ctrl.average_ipg;
    port_conf->pad_en = xlmac_tx_ctrl.pad_en;
    port_conf->local_loopback = xlmac_ctrl.local_lpbk;
    port_conf->pad_threashold = xlmac_tx_ctrl.pad_threshold;
    port_conf->tx_preamble_len = xlmac_tx_ctrl.tx_preamble_length;
    port_conf->tx_threshold = xlmac_tx_ctrl.tx_threshold;
    port_conf->speed = xport_speed_get((uint8_t)portid);
    __xportDebug("DONE \n");
    return rc;
}

int xport_set_port_configuration(uint32_t portid, xport_port_cfg_s *port_conf)
{
    int rc = 0;
    xport_xlmac_core_ctrl xlmac_ctrl;
    xport_xlmac_core_tx_ctrl xlmac_tx_ctrl;

    rc = rc ? rc : ag_drv_xport_xlmac_core_ctrl_get(portid, &xlmac_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_ctrl_get(portid, &xlmac_tx_ctrl);

    xlmac_tx_ctrl.average_ipg = port_conf->average_igp;
    xlmac_tx_ctrl.pad_en = port_conf->pad_en;
    xlmac_ctrl.local_lpbk = port_conf->local_loopback;
    xlmac_tx_ctrl.pad_threshold = port_conf->pad_threashold;
    xlmac_tx_ctrl.tx_preamble_length = port_conf->tx_preamble_len;

    rc = rc ? rc : ag_drv_xport_xlmac_core_ctrl_set(portid, &xlmac_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_ctrl_set(portid, &xlmac_tx_ctrl);

    /* link speed related configuration should be done during link-up/xport_init */
    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

int xport_get_pause_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl)
{
    int rc;
    xport_xlmac_core_rx_ctrl xlmac_rx_ctrl;
    xport_xlmac_core_pause_ctrl pause_ctrl;

    rc = ag_drv_xport_xlmac_core_rx_ctrl_get(portid, &xlmac_rx_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_mac_sa_get(portid, &flow_ctrl->tx_ctrl_sa);
    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_mac_sa_get(portid, &flow_ctrl->rx_ctrl_sa);
    rc = rc ? rc : ag_drv_xport_xlmac_core_pause_ctrl_get(portid, &pause_ctrl);

    flow_ctrl->pause_refresh_en = pause_ctrl.pause_refresh_en;
    flow_ctrl->pause_refresh_timer = pause_ctrl.pause_refresh_timer;
    flow_ctrl->pause_xoff_timer = pause_ctrl.pause_xoff_timer;
    flow_ctrl->rx_pass_ctrl = xlmac_rx_ctrl.rx_pass_ctrl;
    flow_ctrl->rx_pass_pause = xlmac_rx_ctrl.rx_pass_pause;
    flow_ctrl->rx_pause_en = pause_ctrl.rx_pause_en;
    flow_ctrl->tx_pause_en = pause_ctrl.tx_pause_en;

    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

int xport_set_pfc_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl)
{
    int rc;

    xport_xlmac_core_rx_ctrl xlmac_rx_ctrl;

    rc = ag_drv_xport_xlmac_core_rx_ctrl_get(portid, &xlmac_rx_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_mac_sa_set(portid, flow_ctrl->tx_ctrl_sa);
    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_mac_sa_set(portid, flow_ctrl->rx_ctrl_sa);

    xlmac_rx_ctrl.rx_pass_ctrl = flow_ctrl->rx_pass_ctrl;

    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_ctrl_set(portid, &xlmac_rx_ctrl);

    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

int xport_get_pfc_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl)
{
    int rc;
    xport_xlmac_core_rx_ctrl xlmac_rx_ctrl;

    rc = ag_drv_xport_xlmac_core_rx_ctrl_get(portid, &xlmac_rx_ctrl);
    flow_ctrl->rx_pass_ctrl = xlmac_rx_ctrl.rx_pass_ctrl;

    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

int xport_set_pause_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl)
{
    int rc;
    xport_xlmac_core_rx_ctrl xlmac_rx_ctrl;
    xport_xlmac_core_pause_ctrl pause_ctrl;

    rc = ag_drv_xport_xlmac_core_rx_ctrl_get(portid, &xlmac_rx_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_mac_sa_set(portid, flow_ctrl->tx_ctrl_sa);
    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_mac_sa_set(portid, flow_ctrl->rx_ctrl_sa);
    rc = rc ? rc : ag_drv_xport_xlmac_core_pause_ctrl_get(portid, &pause_ctrl);

    pause_ctrl.pause_refresh_en = flow_ctrl->pause_refresh_en;
    pause_ctrl.pause_refresh_timer = flow_ctrl->pause_refresh_timer;
    pause_ctrl.pause_xoff_timer = flow_ctrl->pause_xoff_timer;
    xlmac_rx_ctrl.rx_pass_ctrl = flow_ctrl->rx_pass_ctrl;
    xlmac_rx_ctrl.rx_pass_pause = flow_ctrl->rx_pass_pause;
    pause_ctrl.rx_pause_en = flow_ctrl->rx_pause_en;
    pause_ctrl.tx_pause_en = flow_ctrl->tx_pause_en;

    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_ctrl_set(portid, &xlmac_rx_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_pause_ctrl_set(portid, &pause_ctrl);

    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

int xport_get_port_rxtx_enable(uint32_t portid, uint8_t *rx_en, uint8_t *tx_en)
{
    int rc;
    xport_xlmac_core_ctrl xlmac_ctrl;

    rc = ag_drv_xport_xlmac_core_ctrl_get(portid, &xlmac_ctrl);

    *rx_en = xlmac_ctrl.rx_en;
    *tx_en = xlmac_ctrl.tx_en;

    __xportDebug("DONE (rc = %d)\n", rc);
    return rc ;
}

int xport_set_port_rxtx_enable(uint32_t portid, uint8_t rx_en, uint8_t tx_en)
{
    int rc;
    xport_xlmac_core_ctrl xlmac_ctrl;

    rc = ag_drv_xport_xlmac_core_ctrl_get(portid, &xlmac_ctrl);

    xlmac_ctrl.rx_en = rx_en;
    xlmac_ctrl.tx_en = tx_en;

    return rc ? rc : ag_drv_xport_xlmac_core_ctrl_set(portid, &xlmac_ctrl);
}

int  xport_get_phyid(uint32_t portid, uint16_t *phyid)
{
    __xportError("Not implemented\n");
    return 0;
}

int xport_reset_phy_cfg(uint32_t portid, xport_port_phycfg_s *phycfg)
{
    __xportError("Not implemented\n");
    return 0;
}

/*This status analysis is for Broadcom's PHYs only as it described in the EGPHY register specification */
int xport_get_brcm_phy_status(uint16_t phyid,xport_port_status_s *port_status)
{
    __xportError("Not implemented\n");
    return 0;
}

int xport_get_port_status(uint32_t portid, xport_port_status_s *port_status)
{
    int rc = XPORT_ERR_OK;
    xport_xlmac_core_ctrl ctrl;
    ag_drv_xport_xlmac_core_ctrl_get(portid, &ctrl);
    port_status->mac_lpbk = ctrl.local_lpbk;
    port_status->mac_rx_en = ctrl.rx_en;
    port_status->mac_tx_en = ctrl.tx_en;
    port_status->rate = xport_speed_get(portid);

    if (portid == 0) /* XGMII 10G Serdes */
    {
        port_status->duplex = XPORT_FULL_DUPLEX;
    }
#if defined(CONFIG_BCM963158)
    else
    {
        xport_reg_crossbar_status crossbar_status;
        ag_drv_xport_reg_crossbar_status_get(&crossbar_status);
        port_status->duplex = crossbar_status.full_duplex ? XPORT_FULL_DUPLEX : XPORT_HALF_DUPLEX;
        port_status->port_up = crossbar_status.link_status;
    }
#endif
    {
        xport_xlmac_core_pause_ctrl pause_ctrl;
        ag_drv_xport_xlmac_core_pause_ctrl_get(portid, &pause_ctrl);
        port_status->rx_pause_en = pause_ctrl.rx_pause_en;
        port_status->tx_pause_en = pause_ctrl.tx_pause_en;
    }

#if !defined(CONFIG_BCM963158)
    {
        int xlmac_num = PID_XLMAC_NUM(portid);
        int xport_num = PID_XPORT_NUM(portid);
        uint8_t link_status = 0;

        ag_drv_xport_top_status_get(xlmac_num, &link_status);
        port_status->port_up = link_status & (1 << xport_num);
        port_status->duplex = XPORT_FULL_DUPLEX;
    }
#endif

    return rc;
}

int xport_mtu_get(uint32_t portid, uint16_t *port_mtu)
{
    return ag_drv_xport_xlmac_core_rx_max_size_get(portid, port_mtu);
}

#if !defined(CONFIG_BCM963158)
int xport_mtu_set(uint32_t portid, uint16_t port_mtu)
{
    int xlmac_num = PID_XLMAC_NUM(portid);
    int xport_num = PID_XPORT_NUM(portid);

    /* set the XLMAC MTU */
    ag_drv_xport_xlmac_core_rx_max_size_set(portid, 0x3fff);

    /* align the MIB max packet size for accurate accounting */
    switch (xport_num)
    {
    case 0:
        return ag_drv_xport_mib_reg_gport0_max_pkt_size_set(xlmac_num, port_mtu);
    case 1:
        return ag_drv_xport_mib_reg_gport1_max_pkt_size_set(xlmac_num, port_mtu);
    case 2:
        return ag_drv_xport_mib_reg_gport2_max_pkt_size_set(xlmac_num, port_mtu);
    case 3:
        return ag_drv_xport_mib_reg_gport3_max_pkt_size_set(xlmac_num, port_mtu);
    default:
        __xportError("(%d):Wrong portid %d",__LINE__, portid);
        return XPORT_ERR_PARAM;
    }
}
#else
int xport_mtu_set(uint32_t portid, uint16_t port_mtu)
{
    uint8_t instance = portid % 4;

    /* set the XLMAC MTU */
    ag_drv_xport_xlmac_core_rx_max_size_set(portid, port_mtu);

    /* align the MIB max packet size for accurate accounting */
    switch (instance)
    {
    case 0:
        return ag_drv_xport_mib_reg_gport0_max_pkt_size_set(port_mtu);
    case 1:
        return ag_drv_xport_mib_reg_gport1_max_pkt_size_set(port_mtu);
    case 2:
        return ag_drv_xport_mib_reg_gport2_max_pkt_size_set(port_mtu);
    case 3:
        return ag_drv_xport_mib_reg_gport3_max_pkt_size_set(port_mtu);
    default:
        __xportError("(%d):Wrong portid %d",__LINE__, portid);
        return XPORT_ERR_PARAM;
    }
}
#endif

int xport_get_port_link_status(uint32_t portid, uint8_t *link_up)
{
    __xportError("Not implemented\n");
    return XPORT_ERR_OK;
}

int xport_eee_set(uint32_t portid, uint8_t enable)
{
    XPORT_PORT_RATE xport_speed;
    uint16_t ref_count = 645;
    uint16_t wake_timer;
    uint16_t delay_entry_timer = 34; // wait 34 us in EMPTY state before moving to LPI 
    int rc = 0;

    if (enable)
        xport_speed = xport_speed_get(portid);
    else
        xport_speed = XPORT_RATE_UNKNOWN;

    /* set wait frim from LPI to active based on connection speed, numbers came from IEEE 802.3az */
    /* extra 1us buffer is added since the timer is not accurate */
    switch (xport_speed)
    {
        case XPORT_RATE_10G:
            /* 
             * 7.36us for 10Gb/s 10GBase-T
             * 4.48us for 10Gb/s 10GBase-T 
             * 12.38us for 10Gb/s XFI
             * 15.38us for 10Gb/s KR, no FEC
             * 17.38us for 10Gb/s KR with FEC
             */
            wake_timer = 19;
        break;
        case XPORT_RATE_5G:
            /* 
             * 14.72us for 5GBase-T.
             */
            wake_timer = 16;
        break;
        case XPORT_RATE_2500MB:
            /* 
             * 29.44us for 2.5GBase-T
             */
            wake_timer = 31;
        break;
        case XPORT_RATE_1000MB:
            /* 
             * 16.5us for 1Gb/s
             */
            wake_timer = 18;
        break;
        case XPORT_RATE_100MB:
            /* 
             * 30us for 100 Mb/s
             */
            wake_timer = 31;
        break;
        case XPORT_RATE_10MB:
            __xportNotice("EEE for connection speed %s is not supported; "
                          "ignore setting eee to %d for xport mac %d\n",
                          xport_rate_to_str(xport_speed), enable, portid);
            return 0;
        break;
        default:
            wake_timer = 0;
        break;
    }

#if defined(CONFIG_BCM963158)
    if (rc == 0)
    {
        uint8_t msbus_clk_sel, wan_led0_sel, timeout_rst_disable, p0_mode;
        /* obtain reference clock for timers */
        rc = ag_drv_xport_reg_xport_cntrl_1_get(&msbus_clk_sel, &wan_led0_sel, &timeout_rst_disable, &p0_mode);
        if (rc == 0)
        {
            if (msbus_clk_sel == 0)
            {
                ref_count = 500; // clock running at 500MHz, 500 count for 1 us
            }
            else
            {
                ref_count = 645; // clock running at 644.5MHz, 645 count for 1 us
            }
        }
    }
#endif

    if (rc == 0)
    {
        // enable EEE on XPORT XLMAC
        ag_drv_xport_xlmac_core_eee_ctrl_set (portid, 0, enable);
        // set EEE timer in XPORT XLMAC
        ag_drv_xport_xlmac_core_eee_timers_set (portid, ref_count, wake_timer, delay_entry_timer);

        // Note : even this function is called 1 second after the PHY is enabled,
        // the 1 second link status timer will be set since this is how HW was validated
        ag_drv_xport_xlmac_core_eee_1_sec_link_status_timer_set (portid, 1000000);
    }
    return rc;
}

int xport_wol_enable_port(uint32_t portid, wol_params_t *wol_params)
{
    int ret = 0;
#if defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
    int xlmac_num = PID_XLMAC_NUM(portid);
    int xport_num = PID_XPORT_NUM(portid);

    if (!wol_params->en_mpd && !wol_params->en_ard)
        return 0;

    /* Configure the XLMAC port for WOL detection */
    ret |= ag_drv_xport_xlmac_reg_wol_cfg_set(xlmac_num, 1, xport_num);
#endif

    return ret;
}

int xport_wol_enable_mpd(uint32_t portid, wol_params_t *wol_params)
{
    int ret = 0;
#if defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
    int xlmac_num = PID_XLMAC_NUM(portid);
    uint32_t mac_31_0;
    uint16_t mac_47_32;
    uint32_t psw_31_0;
    uint16_t psw_47_32;

    if (!wol_params->en_mpd)
        return 0;

    /* Configure MPD destination address */
    mac_31_0 = (wol_params->mac_addr[2] << 24) |
        (wol_params->mac_addr[3] << 16) |
        (wol_params->mac_addr[4] << 8) |
        wol_params->mac_addr[5];

    mac_47_32 = (wol_params->mac_addr[0] << 8) |
        wol_params->mac_addr[1];

    ret |= ag_drv_xport_wol_mpd_mseq_mac_da_low_set(xlmac_num, mac_31_0);
    ret |= ag_drv_xport_wol_mpd_mseq_mac_da_hi_set(xlmac_num, mac_47_32);

    /* Configure MPD password */
    psw_31_0 = (wol_params->password[2] << 24) |
        (wol_params->password[3] << 16) |
        (wol_params->password[4] << 8) |
        wol_params->password[5];

    psw_47_32 = (wol_params->password[0] << 8) |
        wol_params->password[1];

    ret |= ag_drv_xport_wol_mpd_psw_low_set(xlmac_num, psw_31_0);
    ret |= ag_drv_xport_wol_mpd_psw_hi_set(xlmac_num, psw_47_32);

    /* Enable password and configure number of repetitions */
    ret |= ag_drv_xport_wol_mpd_config_set(xlmac_num, wol_params->en_psw ? 1 : 0, wol_params->repetitions ? : 16);

    /* Enable Magic packet detection in XPORT */
    ret |= ag_drv_xport_wol_mpd_control_set(xlmac_num, 1);
#endif

    return ret;
}

int xport_wol_enable_ard(uint32_t portid, wol_params_t *wol_params)
{
    int ret = 0;
#if defined(CONFIG_BCM96837)
    int xlmac_num = PID_XLMAC_NUM(portid);

    if (!wol_params->en_ard)
        return 0;

    /* Don't look for Broadcom tag */
    ret |= ag_drv_xport_wol_ard_config_set(xlmac_num, 0);

    /* Enable ARP detection in XPORT */
    ret |= ag_drv_xport_wol_ard_control_set(xlmac_num, 1);
#endif

    return ret;
}

int xport_wol_enable(uint32_t portid, wol_params_t *wol_params)
{
    int ret = 0;

    ret |= xport_wol_enable_port(portid, wol_params);
    ret |= xport_wol_enable_mpd(portid, wol_params);
    ret |= xport_wol_enable_ard(portid, wol_params);

    return ret;
}

char *xport_rate_to_str(XPORT_PORT_RATE rate)
{
    switch (rate)
    {
    case XPORT_RATE_10MB: return "10M";
    case XPORT_RATE_100MB: return "100M";
    case XPORT_RATE_1000MB: return "1G";
    case XPORT_RATE_2500MB: return "2.5G";
    case XPORT_RATE_5G: return "5G";
    case XPORT_RATE_10G: return "10G";
    default: return "Unknown";
    }
}

int xport_str_to_rate(char *str)
{
    if (!strcmp(str, "10M"))
        return XPORT_RATE_10MB;
    if (!strcmp(str, "100M"))
        return XPORT_RATE_100MB;
    if (!strcmp(str, "1G"))
        return XPORT_RATE_1000MB;
    if (!strcmp(str, "2.5G"))
        return XPORT_RATE_2500MB;
    if (!strcmp(str, "5G"))
        return XPORT_RATE_5G;
    if (!strcmp(str, "10G"))
        return XPORT_RATE_10G;

    return XPORT_RATE_UNKNOWN;
}

