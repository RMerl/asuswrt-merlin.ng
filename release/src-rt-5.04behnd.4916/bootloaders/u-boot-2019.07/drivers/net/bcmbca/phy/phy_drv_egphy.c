// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved


*/

/*
 *  Created on: Aug 2016
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Phy driver for 68xx internal quad 1G PHY block
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "phy_drv_brcm.h"
#include "memory_access.h"
#include "dt_access.h"
#include "pmc_qgphy.h"
#include <linux/delay.h>

static void __iomem * qegphy_base;
static uint16_t base_addr = 1;
static int _phy_cfg(uint32_t port_map);

static int egphy_probe(dt_device_t *pdev)
{
    int ret;
    dt_handle_t node = dt_dev_get_handle(pdev);

    qegphy_base = dt_dev_remap(pdev, 0);
    if (IS_ERR(qegphy_base))
    {
        ret = PTR_ERR(qegphy_base);
        qegphy_base = NULL;
        dev_err(&pdev->dev, "Missing qegphy_base entry\n");
        goto Exit;
    }

    dev_dbg(&pdev->dev, "qegphy_base=0x%px\n", qegphy_base);
    dev_dbg(&pdev->dev, "base_addr=%d\n", base_addr);
    dev_info(&pdev->dev, "registered\n");

    base_addr = dt_property_read_u32_default(node, "base-addr", base_addr);
    _phy_cfg(0); /* put the EGPHY block in reset */

    return 0;

Exit:
    return ret;
}

static const struct udevice_id egphy_ids[] = {
    { .compatible = "brcm,egphy" },
    { /* end of list */ },
};

U_BOOT_DRIVER(brcm_egphy) = {
    .name	= "brcm-egphy",
    .id	= UCLASS_MISC,
    .of_match = egphy_ids,
    .probe = egphy_probe,
};

#define QEGPHY_BASE             (void *)qegphy_base
#define QEGPHY_TEST_CTRL_REG    QEGPHY_BASE + 0x0000
#define QEGPHY_CTRL_REG         QEGPHY_BASE + 0x0004
#define QEGPHY_STATUS_REG       QEGPHY_BASE + 0x0008
static uint32_t enabled_addrs = 0;

#pragma pack(push,1)
typedef struct
{
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96855)
    uint32_t PHY_TEST_EN:1;
    uint32_t RESERVED1:31;
#endif
#if defined(CONFIG_BCM96858)
    uint32_t PHY_TEST_EN:1;
    uint32_t PLL_CLK125_250_SEL:1;
    uint32_t PLL_SEL_DIV5:2;
    uint32_t PLL_REFCLK_SEL:2;
    uint32_t RESERVED1:26;
#endif
#if defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
    uint32_t PHY_TEST_EN:1;
    uint32_t IDDQ_TEST_MODE:1;
    uint32_t RESERVED1:30;
#endif
} qegphy_test_ctrl_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96855)
    uint32_t IDDQ_BIAS:1;
    uint32_t EXT_PWR_DOWN:4;
    uint32_t FORCE_DLL_EN:1;
    uint32_t IDDQ_GLOBAL_PWR:1;
    uint32_t CK25_DIS:1;
    uint32_t PHY_RESET:1;
    uint32_t RESERVED1:3;
    uint32_t PHY_PHYAD:5;
    uint32_t PLL_REFCLK_SEL:2;
    uint32_t PLL_SEL_DIV5:2;
    uint32_t PLL_CLK125_250_SEL:1;
    uint32_t RESERVED2:10;
#endif
#if defined(CONFIG_BCM96858)
    uint32_t IDDQ_BIAS:1;
    uint32_t EXT_PWR_DOWN:4;
    uint32_t FORCE_DLL_EN:1;
    uint32_t IDDQ_GLOBAL_PWR:1;
    uint32_t CK25_EN:1;
    uint32_t PHY_RESET:1;
    uint32_t CLK_MUX_MODE:1;
    uint32_t RESERVED1:2;
    uint32_t PHY_PHYAD:5;
    uint32_t RESERVED2:15;
#endif
#if defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
    uint32_t IDDQ_BIAS:1;
    uint32_t EXT_PWR_DOWN:4;
    uint32_t FORCE_DLL_EN:1;
    uint32_t IDDQ_GLOBAL_PWR:4;
    uint32_t CK25_DIS:1;
    uint32_t PHY_RESET:1;
    uint32_t PHY_PHYAD:5;
    uint32_t REF_CLK_FREQ_SEL:2;
    uint32_t PLL_CLK125_250_SEL:1;
    uint32_t RESERVED1:12;
#endif
} qegphy_ctrl_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878) || defined(CONFIG_BCM96856) || \
    defined(CONFIG_BCM96855) || defined(CONFIG_BCM96858) || defined(CONFIG_BCM96888) || \
    defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
    uint32_t ENERGY_DET_MASKED:4;
    uint32_t ENERGY_DET_APD:4;
    uint32_t PLL_LOCK:1;
    uint32_t RECOVERED_CLK_LOCK:4;
    uint32_t GPHY_TEST_STATUS:1;
    uint32_t RESERVED1:18;
#endif
} qegphy_status_t;
#pragma pack(pop)

#define CORE_SHD18_000          0x0028 /* Auxiliary Control Register */
#define CORE_EXPB0              0x00b0 /* Bias Control 0 */
#define DSP_TAP10               0x0125 /* PLL Bandwidth Control */
#define DSP_TAP33_C2            0x0152 /* EEE LPI Timers */
#define DSP_TAP34_C2            0x0156 /* EEE 100TX Mode BW control */
#define DSP_FFE_ZERO_DET_CTL    0x0166 /* FFE Zero Detection Control */
#define AFE_RXCONFIG_2          0x01e2 /* AFE RXCONFIG 2 */
#define AFE_TX_IQ_RX_LP         0x01e4 /* AFE_TX_IQ_RX_LP */
#define AFE_TX_CONFIG_0         0x01e5 /* AFE_TX_CONFIG_0 */
#define AFE_HPF_TRIM_OTHERS     0x01e8 /* HPF trim and RXCONFIG 49:48 and reserved bits */
#define AFE_TX_CONFIG_1         0x01ea /* AFE_TX_CONFIG_1 */
#define AFE_TX_CONFIG_2         0x01eb /* AFE_TX_CONFIG_2 */
#define AFE_TEMPSEN_OTHERS      0x01ec /* AFE_TEMPSEN_OTHERS */

#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96878)
static int _phy_afe(phy_dev_t *phy_dev)
{
    int ret;

    /* Enable the DSP clock */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD18_000, 0x0c30)))
        goto Exit;

    /* Turn off AOF */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TX_CONFIG_0, 0x0000)))
        goto Exit;

    /* 1g AB symmetry Iq */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TX_CONFIG_1, 0x0bcc)))
        goto Exit;

    /* LPF BW */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TX_IQ_RX_LP, 0x233f)))
        goto Exit;

    /* RCAL +6LSB to make impedance from 112 to 100ohm */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TEMPSEN_OTHERS, 0xad40)))
        goto Exit;

    /* since rcal make R smaller, make master current -4% */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP10, 0x091b)))
        goto Exit;

    /* rx_on_tune 8 -> 0xf */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP33_C2, 0x87f6)))
        goto Exit;

    /* From EEE excel config file for Vitesse fix */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP34_C2, 0x017d)))
        goto Exit;

    /* Enable ffe zero det for Vitesse interop */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_FFE_ZERO_DET_CTL, 0x0015)))
        goto Exit;

    /* Reset R_CAL/RC_CAL Engine */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_EXPB0, 0x0010)))
        goto Exit;

    /* Disable Reset R_CAL/RC_CAL Engine */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_EXPB0, 0x0000)))
        goto Exit;

Exit:
    return ret;
}
#endif

#if defined(CONFIG_BCM96855)
static int _phy_afe(phy_dev_t *phy_dev)
{
    int ret;

    /* Enable the DSP clock */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD18_000, 0x0c30)))
        goto Exit;

    /* Turn off AOF */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TX_CONFIG_0, 0x0000)))
        goto Exit;

    /* 1g AB symmetry Iq */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TX_CONFIG_1, 0x0bcc)))
        goto Exit;

    /* 10BT no change(net increase +2.2%) and 100BT decrease by -4.8% (net increase -2.6%) */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TX_CONFIG_2, 0x005e))) /* TODO: 0x005f for 17x17 */
        goto Exit;

    /* LPF BW */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TX_IQ_RX_LP, 0x233f)))
        goto Exit;

    /* RCAL +6LSB to make impedance from 112 to 100ohm */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TEMPSEN_OTHERS, 0xad40)))
        goto Exit;

    /* since rcal make R smaller, make master current +2.2% */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP10, 0x211b)))
        goto Exit;

    /* rx_on_tune 8 -> 0xf */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP33_C2, 0x87f6)))
        goto Exit;

    /* From EEE excel config file for Vitesse fix */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP34_C2, 0x017d)))
        goto Exit;

    /* Enable ffe zero det for Vitesse interop */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_FFE_ZERO_DET_CTL, 0x0015)))
        goto Exit;

    /* Reset R_CAL/RC_CAL Engine */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_EXPB0, 0x0010)))
        goto Exit;

    /* Disable Reset R_CAL/RC_CAL Engine */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_EXPB0, 0x0000)))
        goto Exit;

Exit:
    return ret;
}
#endif

#if defined(CONFIG_BCM96858)
static int _phy_afe(phy_dev_t *phy_dev)
{
    int ret;

    /* Enable the dsp clock */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD18_000, 0x0c30)))
        goto Exit;

    /* +1 RCAL codes for RL centering for both LT and HT conditions; default was -2 */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_RXCONFIG_2, 0xd003)))
        goto Exit;

    /* Cut master bias current by 2% to compensate for RCAL code offset */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP10, 0x791b)))
        goto Exit;

    /* Improve hybrid leakage */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_HPF_TRIM_OTHERS, 0x10e3)))
        goto Exit;

    /* rx_on_tune 8 -> 0xf */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP33_C2, 0x87f6)))
        goto Exit;

    /* 100tx EEE bandwidth */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP34_C2, 0x017d)))
        goto Exit;

    /* Enable ffe zero det for Vitesse interop */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_FFE_ZERO_DET_CTL, 0x0015)))
        goto Exit;

    /* Reset R_CAL/RC_CAL Engine */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_EXPB0, 0x0010)))
        goto Exit;

    /* Disable Reset R_CAL/RC_CAL Engine */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_EXPB0, 0x0000)))
        goto Exit;

Exit:
    return ret;
}
#endif

#if defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
static void _phy_run_cal(phy_dev_t *phy_dev, uint16_t *rcalnewcode11_p)
{
    uint16_t expa9, rcalcode, rcalnewcodelp;

    // Write PLL/AFE control registers
    brcm_misc_write(phy_dev, 0x30, 1, 0x0000);      // reset PLL
    brcm_misc_write(phy_dev, 0x31, 0, 0x0050);      // ndiv_integer
    brcm_misc_write(phy_dev, 0x33, 2, 0x0001);      // pdiv=0; auto-configured
    brcm_misc_write(phy_dev, 0x31, 1, 0x0000);      // ndiv_fraction
    brcm_misc_write(phy_dev, 0x31, 2, 0x0000);      // frequency nudge factor
    brcm_misc_write(phy_dev, 0x30, 3, 0x0032);      // reference frequency, mode 0
    brcm_misc_write(phy_dev, 0x32, 3, 0x0000);      // init bypass mode - default setting
    brcm_misc_write(phy_dev, 0x33, 0, 0x0002);      // bypass code - default, vcoclk enabled
    brcm_misc_write(phy_dev, 0x30, 2, 0x01c0);      // LDOs at default settings
    brcm_misc_write(phy_dev, 0x30, 1, 0x0001);      // release PLL reset

    // AFE_BG_CONFIG
    brcm_misc_write(phy_dev, 0x38, 0, 0x0010);      // Bandgap curvature correction to correct default -- Erol

    // Run RCAL on AFE_CAL_CONFIG_2 = 0x3
    brcm_misc_write(phy_dev, 0x39, 0x3, 0x38);      // enable max averaging
    brcm_misc_write(phy_dev, 0x39, 0x3, 0x3b);      // no reset, analog powerup
    udelay(1000);
    brcm_misc_write(phy_dev, 0x39, 0x3, 0x3f);      // start calibration
    udelay(1000);

    // Run RCCAL on AFE_CAL_CONFIG_0 = 0x1
    brcm_misc_write(phy_dev, 0x39, 0x1, 0x1c82);    // Vref=1000, Target=10, averaging enabled
    brcm_misc_write(phy_dev, 0x39, 0x1, 0x9e82);    // no reset, analog powerup
    udelay(1000);
    brcm_misc_write(phy_dev, 0x39, 0x1, 0x9f82);    // start calibration
    udelay(1000);
    brcm_misc_write(phy_dev, 0x39, 0x1, 0x9e86);    // clear start calibration, set HiBW
    udelay(1000);
    brcm_misc_write(phy_dev, 0x39, 0x1, 0x9f86);    // start calibration with hi BW mode set
    udelay(1000);

    // TX Amplitude finetune
    brcm_misc_write(phy_dev, 0x38, 0x1, 0xe7ea);    // AFE_BIAS_CONFIG_0, Adjust 10BT amplitude additional +7% and 100BT +2%
    brcm_misc_write(phy_dev, 0x38, 0x2, 0xede0);    // AFE_BIAS_CONFIG_1, Adjust 1G mode amplitude and 1G testmode1

    // Adjust 10BT bias and RCAL settings
    // read CORE_EXPA9
    brcm_exp_read(phy_dev, 0xa9, &expa9);           // read CORE_EXPA9
    rcalcode = (expa9 & 0x007e) >> 1;               // expA9<6:1> is rcalcode<5:0>
    rcalnewcodelp = rcalcode + 16;
    *rcalnewcode11_p = rcalcode + 10;
    if (rcalnewcodelp > 0x003f)                     // saturate RCAL code if necessary
        rcalnewcodelp = 0x003f;
    if (*rcalnewcode11_p > 0x003f)                  // saturate RCAL code if necessary
        *rcalnewcode11_p = 0x003f;

    // AFE_BIAS_CONFIG_0
    brcm_misc_write(phy_dev, 0x39, 0x3, (rcalnewcodelp << 8) + 0xf8);   // REXT=1 BYP=1 RCAL_st1<5:0>=new rcal code
    brcm_misc_write(phy_dev, 0x38, 0x1, 0xe7e4);    // AFE_BIAS_CONFIG_0 10BT bias code, bias = 0xe4
}

static int _phy_afe_cfg(phy_dev_t *phy_dev, uint16_t rcalnewcode11)
{
    uint16_t txcfgch0 = 0 , txcfg2 = 0;

    // AFE_RXCONFIG_3
    brcm_misc_write(phy_dev, 0x3b, 0x00, 0x8002);   // invert adc clock output and 'adc refp ldo current To correct default
    // AFE_TX_CONFIG_1
    brcm_misc_write(phy_dev, 0x3c, 0x03, 0xf882);   // 100BT stair case, high BW, 1G stair case, alternate encode
    // AFE_TX_CONFIG_2
    brcm_misc_write(phy_dev, 0x3d, 0x00, 0x3201);   // 1000BT DAC transition method per Erol, bits[32], DAC Shuffle sequence 1 + 10BT imp adjust bits
    /// AFE_RXCONFIG_1
    brcm_misc_write(phy_dev, 0x3a, 0x02, 0x0c00);   // some non-overlap fix per Erol

    // RX Full scale calibration
    // pwdb override (rxconfig<5>) to turn on RX LDO indpendent of pwdb controls from DSP */
    brcm_misc_write(phy_dev, 0x3a, 0x01, 0x0020);   // AFE_RXCONFIG_0

    // Specify target offset for each channel
    brcm_misc_write(phy_dev, 0x3b, 0x02, 0x0000);   // AFE_RXCONFIG_CH0
    brcm_misc_write(phy_dev, 0x3b, 0x03, 0x0000);   // AFE_RXCONFIG_CH1
    brcm_misc_write(phy_dev, 0x3c, 0x00, 0x0000);   // AFE_RXCONFIG_CH2
    brcm_misc_write(phy_dev, 0x3c, 0x01, 0x0000);   // AFE_RXCONFIG_CH3

    // Set cal_bypassb bit rxconfig<43>
    brcm_misc_write(phy_dev, 0x003a, 0x03, 0x0800);   // AFE_RXCONFIG_2

    // At least 2us delay needed  before this line is executed.
    udelay(1000);

    // Revert pwdb_override (rxconfig<5>) to 0 so that the RX pwr is controlled by DSP.
    brcm_misc_write(phy_dev, 0x003a, 0x01, 0x0000);   // AFE_RXCONFIG_0

    // Adjust 10BT bias and RCAL settings
    brcm_misc_read(phy_dev, 0x3d, 0x1, &txcfgch0);    // read AFE_TX_CONFIG_CH0
    // clear bits <11:5>, set txcfg_ch0<5>=1 (enable + set local rcal)
    txcfgch0 = (txcfgch0 & ~(0xfe0)) | 0x0020 | ((rcalnewcode11 &0xfffe) << 5);
    brcm_misc_write(phy_dev, 0x3d, 0x1, txcfgch0);    // write AFE_TX_CONFIG_CH0
    brcm_misc_write(phy_dev, 0x3d, 0x2, txcfgch0);    // write AFE_TX_CONFIG_CH1 for MDIX

    // AFE_TX_CONFIG_2
    brcm_misc_read(phy_dev, 0x3d, 0x0, &txcfg2);      // read AFE_TX_CONFIG_2
    //* set txcfg<45:44>=11 (enable Rextra + invert fullscaledetect)
    txcfg2 = (txcfg2 & ~(0x3000)) | 0x3000;
    brcm_misc_write(phy_dev, 0x3d, 0x0, txcfg2);      // write AFE_TX_CONFIG_2

    return 0;
}

static int _phy_afe_reset(phy_dev_t *phy_dev)
{
   // AFE reset
    brcm_exp_write(phy_dev, 0x03, 0x00006);    // reset AFE and PLL
    udelay(300);
    brcm_exp_write(phy_dev, 0x03, 0x00000);    // release Reset

    return 0;
}

static int _phy_afe(phy_dev_t *phy_dev)
{
    int i;
    uint16_t rcalnewcode11;
    phy_dev_t tmp_phy = *phy_dev;

    for (i = 0; i < 4; i++)
    {
        tmp_phy.addr = base_addr + i;
        _phy_afe_reset(&tmp_phy);
    }

    tmp_phy.addr = base_addr;
    _phy_run_cal(&tmp_phy, &rcalnewcode11);

    for (i = 0; i < 4; i++)
    {
        tmp_phy.addr = base_addr + i;
        _phy_afe_cfg(&tmp_phy, rcalnewcode11);
    }

    return 0;
}
#endif

static int _phy_init(phy_dev_t *phy_dev)
{
    int ret;

    if ((ret = mii_init(phy_dev)))
        goto Exit;

    /* AFE workaround */
    if ((ret = _phy_afe(phy_dev)))
        goto Exit;

    if ((ret = brcm_egphy_force_auto_mdix_set(phy_dev, 1)))
        goto Exit;

    if ((ret = brcm_egphy_eth_wirespeed_set(phy_dev, 1)))
        goto Exit;

#if defined(CONFIG_BCM_JUMBO_FRAME)
    if ((ret = brcm_shadow_18_ext_pkt_len_set(phy_dev, 1)))
        goto Exit;
#endif

Exit:
    return ret;
}

#if defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
static int _phy_cfg(uint32_t port_map)
{
    qegphy_test_ctrl_t qegphy_test_ctrl;
    qegphy_ctrl_t qegphy_ctrl;
    qegphy_status_t qegphy_status;

    if (!port_map)
        return 0;

    if (pmc_qgphy_power_up())
        return -1;

    READ_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    READ_32(QEGPHY_TEST_CTRL_REG, qegphy_test_ctrl);

    /* Assert reset_n (active low) */
    qegphy_ctrl.PHY_RESET = 1;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    qegphy_ctrl.PLL_CLK125_250_SEL = 1;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    /* Assert iddq_bias */
    qegphy_ctrl.IDDQ_BIAS = 1;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    /* Power only enabled ports */
    qegphy_ctrl.EXT_PWR_DOWN = ~port_map & 0xf;
    qegphy_ctrl.IDDQ_GLOBAL_PWR = ~port_map & 0xf;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    /* Set QGPHY base address */
    qegphy_ctrl.PHY_PHYAD = base_addr;

    /* Deassert iddq_bias */
    qegphy_ctrl.IDDQ_BIAS = 0;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    /* Set GPHY's pll_clk125_250_sel to 125MHz */
    qegphy_ctrl.PLL_CLK125_250_SEL = 0;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    /* Deassert reset_n */
    qegphy_ctrl.PHY_RESET = 0;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    /* Check for PLL Lock */
    READ_32(QEGPHY_STATUS_REG, qegphy_status);

    if (!qegphy_status.PLL_LOCK)
    {
        pr_info("EGPHY PLL is not Locked");
        return -1;
    }

    return 0;
}
#else
static int _phy_cfg(uint32_t port_map)
{
    qegphy_test_ctrl_t qegphy_test_ctrl;
    qegphy_ctrl_t qegphy_ctrl;
    qegphy_status_t qegphy_status;

    /* To ensure minimum power consumption, the following steps should be
     * applied to the GPHY control signals. This should be done immediately
     * upon startup, before the decision is made to enable the GPHY or not,
     * in order to cover all cases.
     */

    READ_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    READ_32(QEGPHY_TEST_CTRL_REG, qegphy_test_ctrl);

    /* Assert reset_n (active low) */
    qegphy_ctrl.PHY_RESET = 1;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    /* Set GPHY's pll_clk125_250_sel to 250MHz */
#if defined(CONFIG_BCM96858)
    qegphy_test_ctrl.PLL_CLK125_250_SEL = 1;
    WRITE_32(QEGPHY_TEST_CTRL_REG, qegphy_test_ctrl);
#else
    qegphy_ctrl.PLL_CLK125_250_SEL = 1;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
#endif
    udelay(1000);

    /* Deassert iddq_global_pwr and iddq_bias */
    qegphy_ctrl.IDDQ_GLOBAL_PWR = 0;
    qegphy_ctrl.IDDQ_BIAS = 0;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    if (!port_map)
    {
        /* Assert iddq_global_pwr and iddq_bias */
#if defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
        qegphy_ctrl.IDDQ_GLOBAL_PWR = 0xf;
#else
        qegphy_ctrl.IDDQ_GLOBAL_PWR = 0x1;
#endif
        qegphy_ctrl.IDDQ_BIAS = 1;
        WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
        udelay(1000);

        /* Deassert reset_n */
        qegphy_ctrl.PHY_RESET = 0;
        WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
        udelay(1000);

       /* Set GPHY's pll_clk125_250_sel to 125MHz */
#if defined(CONFIG_BCM96858)
        qegphy_test_ctrl.PLL_CLK125_250_SEL = 0;
        WRITE_32(QEGPHY_TEST_CTRL_REG, qegphy_test_ctrl);
#else
        qegphy_ctrl.PLL_CLK125_250_SEL = 0;
        WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
#endif
        udelay(1000);

        /* Assert reset_n (active low) */
        qegphy_ctrl.PHY_RESET = 1;
        WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
        udelay(1000);

        return 0;
    }

    /* Set QGPHY base address */
    qegphy_ctrl.PHY_PHYAD = base_addr;

    /* Power only enabled ports */
    qegphy_ctrl.EXT_PWR_DOWN = ~port_map & 0xf;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    /* Set GPHY's pll_clk125_250_sel to 125MHz */
#if defined(CONFIG_BCM96858)
    qegphy_test_ctrl.PLL_CLK125_250_SEL = 0;
    WRITE_32(QEGPHY_TEST_CTRL_REG, qegphy_test_ctrl);
#else
    qegphy_ctrl.PLL_CLK125_250_SEL = 0;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
#endif
    udelay(1000);

    /* Deassert reset_n */
    qegphy_ctrl.PHY_RESET = 0;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    /* Check for PLL Lock */
    READ_32(QEGPHY_STATUS_REG, qegphy_status);

    if (!qegphy_status.PLL_LOCK)
    {
        pr_info("EGPHY PLL is not Locked");
        return -1;
    }

    return 0;
}
#endif

extern phy_drv_t phy_drv_egphy;

static int _phy_afe_all(void)
{
    int i, ret = 0;
    phy_dev_t phy_dev = {};
    phy_dev.phy_drv = &phy_drv_egphy;

    for (i = 0; i < 4; i++)
    {
        phy_dev.addr = base_addr + i;
        ret |= _phy_afe(&phy_dev);
    }

    return ret;
}

static int _phy_dev_add(phy_dev_t *phy_dev)
{
    enabled_addrs |= (1 << phy_dev->addr);

    return 0;
}

static int _phy_dev_del(phy_dev_t *phy_dev)
{
    enabled_addrs &= ~(1 << phy_dev->addr);

    return 0;
}

static int _phy_drv_init(phy_drv_t *phy_drv)
{
    if (_phy_cfg((enabled_addrs >> base_addr) & 0xf))
    {
        printk("Failed to initialize the EGPHY driver\n");
        return -1;
    }

    if (_phy_afe_all())
    {
        printk("Failed to initialize the phy AFE settings\n");
        return -1;
    }

    phy_drv->initialized = 1;

    return 0;
}

#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)
#include "bcm_bca_leds_dt_bindings.h"
#include "bcm_bca_leds.h"

#define MAX_LEDS_PER_PORT 2

#define CORE_SHD1C_02			0x0012 /* LED Control0 */
#define CORE_SHD1C_09			0x0019 /* LED Control */
#define CORE_SHD1C_0D			0x001d /* LED Selector 1 */
#define CORE_SHD1C_0E			0x001e /* LED Selector 2 */
#define CORE_EXP04              0x0034 /* Bicollor Led Selector */

static int _phy_leds_init_51XX(phy_dev_t *phy_dev, void *_leds_info, uint8_t is_shifted)
{
    bca_leds_info_t *leds_info = (bca_leds_info_t *)_leds_info;
    int ret = 0;
    int j;
    uint16_t led_shd1c_09 = 0;
    uint16_t led_shd1c_02 = 0;
    uint16_t led_core_exp_04 = 0;
    uint16_t led_shd1c_0d = 0, led_shd1c_0e = 0;

    if (((leds_info->link[0] == LED_SPEED_GBE || leds_info->link[0] == LED_SPEED_ALL) &&
        leds_info->link[0] == leds_info->activity[0]) &&
        ((leds_info->link[1] == LED_SPEED_1G) && leds_info->activity[1] == 0))
    {
        led_shd1c_02 = 0x206;
        led_shd1c_09 = 0;
        led_shd1c_0d = 0xaa;
        led_shd1c_0e = 0x00;
        led_core_exp_04 = 0x102;
    }
    else if(((leds_info->link[1] == LED_SPEED_GBE || leds_info->link[1] == LED_SPEED_ALL) &&
        leds_info->link[1] == leds_info->activity[1]) &&
        ((leds_info->link[0] == LED_SPEED_1G) && leds_info->activity[0] == 0))
    {
        led_shd1c_02 = 0x206;
        led_shd1c_09 = 0;
        led_shd1c_0d = 0x00;
        led_shd1c_0e = 0xaa;
        led_core_exp_04 = 0x120;
    }
    else
    {

        for (j = 0; j < MAX_LEDS_PER_PORT; j++)
        {
            uint32_t led_mux = leds_info->link[j];
            uint32_t led_activity = leds_info->activity[j];
            uint32_t val = 0;

            if (led_mux == led_activity)
            {
                if (led_mux == LED_SPEED_ALL || led_mux == LED_SPEED_GBE)
                {
                    val = 0x18;
                }
                else if (led_mux == (LED_SPEED_1G | LED_SPEED_100))
                {
                    val = 0x108;
                }
                else if (led_mux == (LED_SPEED_1G | LED_SPEED_10))
                {
                    val = 0x108;
                }
                else if (led_mux == LED_SPEED_1G)
                {
                    val = 0x118;
                }
                else if (led_mux == LED_SPEED_100)
                {
                    val = 0x118;
                }
                else if (led_mux == LED_SPEED_10)
                {
                    val = 0x118;
                }
                else if (led_mux == (LED_SPEED_100 | LED_SPEED_10))
                {
                    val = 0x118;
                }

                if( val > led_shd1c_09)
                    led_shd1c_09 = val;
            }
        }

        for (j = 0; j < MAX_LEDS_PER_PORT; j++)
        {
            uint16_t led_sel = 0;
            uint32_t led_mux = leds_info->link[j];
            uint32_t led_activity = leds_info->activity[j];
            uint16_t val, val2;

            if (led_mux == led_activity)
            {
                if (led_mux == LED_SPEED_ALL || led_mux == LED_SPEED_GBE)
                {
                    val = 0x3;
                    if (led_shd1c_09 == 0x118)
                    {
                        val = 0xa;
                        led_core_exp_04 |= 0x500;
                    }
                }
                else if (led_mux == (LED_SPEED_1G | LED_SPEED_100))
                {
                    val = 0x1;
                }
                else if (led_mux == (LED_SPEED_1G | LED_SPEED_10))
                {
                    val = 0x0;
                }
                else if (led_mux == LED_SPEED_1G)
                {
                    val = 0x3;
                }
                else if (led_mux == LED_SPEED_100)
                {
                    val = 0x1;
                }
                else if (led_mux == LED_SPEED_10)
                {
                    val = 0x0;
                }
                else if (led_mux == (LED_SPEED_100 | LED_SPEED_10))
                {
                    val = 0xa;
                    led_core_exp_04 |= 0x504;
                }
                else
                {
                    val = 0xe;
                }
            }
            else
            {
                if (led_mux == LED_SPEED_ALL || led_mux == LED_SPEED_GBE)
                {
                    val = 0xa;
                    val2 = 0x2;
                }
                else if (led_activity == LED_SPEED_ALL || led_activity == LED_SPEED_GBE)
                {
                    val = 0xa;
                    val2 = 0x8;
                }
                else
                {
                    val=0xe;
                    val2 = 0x0;
                }
                led_core_exp_04 |= (0x100 | val2<<(4*(j%2)));
            }


            led_sel = (val<<(4*((j+is_shifted)%2)));

            if (j < (2 - is_shifted))
                led_shd1c_0d |= led_sel;
            else
                led_shd1c_0e |= led_sel;
        }
    }

    ret = ret ? ret : phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_0D, led_shd1c_0d);
    ret = ret ? ret : phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_0E, led_shd1c_0e);
    if (led_shd1c_02 && !ret)
        ret = ret ? ret : phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_02, led_shd1c_02);
    if (led_shd1c_09 && !ret)
        ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_09, led_shd1c_09);
    if (led_core_exp_04 && !ret)
        ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_EXP04, led_core_exp_04);

    printk("CORE_SHD1C_02: 0x%x CORE_SHD1C_09: 0x%x CORE_SHD1C_0D: 0x%x CORE_SHD1C_0E: 0x%x CORE_EXP04: 0x%x\n",
        led_shd1c_02, led_shd1c_09, led_shd1c_0d, led_shd1c_0e, led_core_exp_04);

    return ret;
}
#endif

#if defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855)
int xrdp_leds_init(void *leds_info);
#endif
#if defined(CONFIG_BCM96858)
int lport_led_init(void *leds_info);
#endif
#if defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
int ephy_leds_init(void *leds_info);
#endif

static int _phy_leds_init(phy_dev_t *phy_dev, void *leds_info)
{
#if defined(CONFIG_BCM96846)
    return _phy_leds_init_51XX(phy_dev, leds_info, 0);
#endif
#if defined(CONFIG_BCM96856)
    return _phy_leds_init_51XX(phy_dev, leds_info, 1);
#endif
#if defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855)
    return xrdp_leds_init(leds_info);
#endif
#if defined(CONFIG_BCM96858)
    return lport_led_init(leds_info);
#endif
#if defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
    return ephy_leds_init(leds_info);
#endif
}

phy_drv_t phy_drv_egphy =
{
    .phy_type = PHY_TYPE_EGPHY,
    .name = "EGPHY",
    .read = brcm_egphy_read,
    .write = brcm_egphy_write,
    .power_get = mii_power_get,
    .power_set = mii_power_set,
    .apd_get = brcm_egphy_apd_get,
    .apd_set = brcm_egphy_apd_set,
    .eee_get = brcm_egphy_eee_get,
    .eee_set = brcm_egphy_eee_set,
    .eee_resolution_get = brcm_egphy_eee_resolution_get,
    .read_status = brcm_read_status,
    .speed_set = mii_speed_set,
    .caps_get = mii_caps_get,
    .caps_set = mii_caps_set,
    .phyid_get = mii_phyid_get,
    .auto_mdix_set = brcm_egphy_force_auto_mdix_set,
    .auto_mdix_get = brcm_egphy_force_auto_mdix_get,
    .wirespeed_set = brcm_egphy_eth_wirespeed_set,
    .wirespeed_get = brcm_egphy_eth_wirespeed_get,
    .init = _phy_init,
    .dev_add = _phy_dev_add,
    .dev_del = _phy_dev_del,
    .drv_init = _phy_drv_init,
    .leds_init = _phy_leds_init,
    .cable_diag_run = brcm_cable_diag_run,
};
