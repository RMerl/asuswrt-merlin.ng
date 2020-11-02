/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom
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
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/module.h>
#include <linux/module.h>
#include <linux/delay.h>


#include <bcm_map_part.h>
#include <pmc_drv.h>

#include <pcie_common.h>
#include <pcie-bcm963xx.h>

/* kernel nvram */
#include <bcm_nvram.h>


/*
 *
 *  Defines
 *
 */

/*
 *
 *  Macros
 *
 */
#define HCD_HC_CORE_CFG(config, core)              \
	(((0xF << ((core)*4)) & (config)) >> ((core)*4))


/*
 *
 *  Structures
 *
 */

/*
 * +-----------------------------------------------------
 *
 *  Local Function prototype
 *
 * +-----------------------------------------------------
 */
static int bcm963xx_pcie_nvram_get_u32(char *key, uint32 *pval);

/*
 *
 *  external Functions prototypes and variables
 *
 */
uint16 bcm963xx_pcie_mdio_read(struct bcm963xx_pcie_hcd *pdrv,
	uint16 phyad, uint16 regad);
int bcm963xx_pcie_mdio_write(struct bcm963xx_pcie_hcd *pdrv,
	uint16 phyad, uint16 regad, uint16 wrdata);


/*
 *
 *  Global variables
 *
 */
/*
 * config_ssc values
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - disable
 *     1 - Enable
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
u32 pcie_ssc_cfg = 0x0000;
module_param(pcie_ssc_cfg, int, S_IRUGO);

/*
 * config_speed values
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - default (keep reset value)
 *     1 - 2.5Gbps
 *     2 - 5Gbps
 *     3 - 8 Gbps
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
u32 pcie_speed_cfg = 0x0000;
module_param(pcie_speed_cfg, int, S_IRUGO);

/*
 * core bring up order (right -> left)
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0x000 - default ( 0, 1, 2, ..... )
 *     0x210 - boot order 0, 1, 2
 *     0x012 - boot order 2, 1, 0
 *
 *     Each 4bit's corresponds to a PCIe core id
 *     [31-28] [27-24][23-20] [19-16][15-12] [11-08] [07-04] [03-00]
 *                                           [third] [second][first]
 */
u32 pcie_boot_order = 0x0000;
module_param(pcie_boot_order, int, S_IRUGO);

/*
 * Always power on
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - Power Down PCIe core if no Link (default)
 *     1 - Keep PCIe core Powered up even if there is no Link
 *     2 - Keep PCIe core Powered Off even if there is Link
 *     3 - Keep PCIe core Powered Off with domain even if there is Link
 *
 *     Each 4bit's corresponds to a PCIe core id
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 *
 *     For Virtual Core the nibble is divided as below
 *     [03-02] [01-00]
 *     [dev2]  [dev1]
 */
u32 pcie_apon = 0x0000;
module_param(pcie_apon, int, S_IRUGO);

/*
 * phy power mode values
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - default (normal power)
 *     1 - low power
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
u32 pcie_phy_pwrmode = 0x0000;
module_param(pcie_phy_pwrmode, int, S_IRUGO);

/*
 * PCIe bus error logging
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - No PCIe bus error logging
 *     1 - enable PCIe bus error logging
 *
 *     Each 4bit's corresponds to a PCIe core id
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
u32 pcie_errlog = 0x1111;
module_param(pcie_errlog, int, S_IRUGO);

/*
 * HCD Logging
 */
#ifdef HCD_DEBUG
int hcd_log_level = HCD_LOG_LVL_ERROR;
#endif


/*
 *
 *  Local Function definitions
 *
 */
/*
 *
 * Function bcm963xx_pcie_nvram_get_u32 (key, pval)
 *
 *
 *   Parameters:
 *    key ... nvram key string
 *    pval ... pointer to u32 value to be returned
 *
 *   Description:
 *     Query the key value from NVRAM and convert to u32 value
 *     value exptected in hex format with a 0xprefix
 *
 *  Return: 0 on success, -ve on failure
 */
int bcm963xx_pcie_nvram_get_u32(char *key, uint32 *pval)
{
#if defined(CONFIG_BCM_NVRAM) || defined(CONFIG_BCM_NVRAM_MODULE)
	char *valstr;
	uint32 val;

	if ((valstr = nvram_k_get(key)) != NULL) {
	    if (sscanf(valstr, "0x%x", &val) == 1) {
	        if (pval) *pval = val;
			return 0;
	    }
		return -EINVAL;
	}
#endif /* CONFIG_BCM_NVRAM */

	return -ENOENT;
}


/*
 *
 *  Global Function definitions
 *
 */
/*
 *
 * Function bcm963xx_pcie_common_init (void)
 *
 *
 *	 Parameters:
 *	   None
 *
 *	 Description:
 *	   Query the PCIe NVRAM parameters and update the global configuration
 *     parameters
 *
 *	Return: 0 on success, -ve on failure
 */
int bcm963xx_pcie_common_init(void)
{
#if defined(MODULE)
	uint32 val;

	HCD_FN_ENT();

	if (bcm963xx_pcie_nvram_get_u32("pcie_ssc_cfg", &val) == 0) {
		pcie_ssc_cfg = val;
	}

	if (bcm963xx_pcie_nvram_get_u32("pcie_speed_cfg", &val) == 0) {
		pcie_speed_cfg = val;
	}

	if (bcm963xx_pcie_nvram_get_u32("pcie_boot_order", &val) == 0) {
		pcie_boot_order = val;
	}

	if (bcm963xx_pcie_nvram_get_u32("pcie_apon", &val) == 0) {
	    pcie_apon = val;
	}

	if (bcm963xx_pcie_nvram_get_u32("pcie_phy_pwrmode", &val) == 0) {
		pcie_phy_pwrmode = val;
	}

	if (bcm963xx_pcie_nvram_get_u32("pcie_errlog", &val) == 0) {
	    pcie_errlog = val;
	}

#ifdef HCD_DEBUG
	if (bcm963xx_pcie_nvram_get_u32("pcie_log_level", &val) == 0) {
		hcd_log_level = val;
	}
#endif /* HCD_DEBUG */

	HCD_FN_EXT();
	return 0;

#else /* !MODULE */
	HCD_FN_ENT();

	HCD_ERROR("nvram is not supported in built-in mode\r\n");

	HCD_FN_EXT();
	return -1;
#endif /* !MODULE */
}

/*
 *
 * Function bcm963xx_pcie_init_hc_cfg (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *     Initialize the port hc_cfg parameters from global storage area
 *
 *  Return: None
 */
void bcm963xx_pcie_init_hc_cfg(struct bcm963xx_pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	pdrv->hc_cfg.ssc = HCD_HC_CORE_CFG(pcie_ssc_cfg, pdrv->core_id) ? TRUE : FALSE;
	pdrv->hc_cfg.speed = HCD_HC_CORE_CFG(pcie_speed_cfg, pdrv->core_id);
	pdrv->hc_cfg.apon = HCD_HC_CORE_CFG(pcie_apon, pdrv->core_id);
	pdrv->hc_cfg.phypwrmode = HCD_HC_CORE_CFG(pcie_phy_pwrmode, pdrv->core_id);
	pdrv->hc_cfg.errlog = HCD_HC_CORE_CFG(pcie_errlog, pdrv->core_id);

	HCD_FN_EXT();
}

/*
 *
 * Function bcm963xx_pcie_get_boot_order_core (order)
 *
 *
 *   Parameters:
 *    order ... index of boot order
 *
 *   Description:
 *     Initialize the port hc_cfg parameters from global storage area
 *
 *  Return: None
 */
int bcm963xx_pcie_get_boot_order_core(int index)
{
	int core;

	HCD_FN_ENT();

	if (pcie_boot_order)
	    core = ((pcie_boot_order >> (index*4)) &  0xF);
	else
	    core = index;

	HCD_FN_EXT();

	return core;
}

/*
 *
 * Function bcm963xx_pcie_gen2_phy_config_ssc (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *     Configure PCIe SSC through MDIO interface. The settings
 *     largely comes from ASIC design team
 *
 *  Return: None
 */
void bcm963xx_pcie_gen2_phy_config_ssc(struct bcm963xx_pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	/* Nothing to do, if SSC is not configured */
	if (pdrv->hc_cfg.ssc == FALSE) {
	    HCD_FN_EXT();
	    return;
	}

	/* set the SSC parameters
	 *
	 * SSC Parameters
	 * Workaround (for early gen2 cards):
	 * Block 0x1100, Register 0xA = 0xea3c
	 * Block 0x1100, Register 0xB = 0x04e7
	 * Block 0x1100, Register 0xC = 0x0039
	 * -Block 0x1100 fixed in 63148A0, 63381B0, 63138B0 but ok to write anyway
	 */
	if (pdrv->core_rev < 0x303) {
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x1100);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x0a, 0xea3c);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x0b, 0x04e7);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x0c, 0x0039);
	}

	/* set the SSC parameters
	 *
	 * SSC Parameters
	 * Block 0x2200, Register 5 = 0x5044    // VCO parameters for fractional mode, -175ppm
	 * Block 0x2200, Register 6 = 0xfef1    // VCO parameters for fractional mode, -175ppm
	 * Block 0x2200, Register 7 = 0xe818    // VCO parameters for fractional mode, -175ppm
	 * Notes:
	 * -Only need to apply these fixes when enabling Spread Spectrum Clocking (SSC),
	 *   which would likely be a flash option
	 */

	bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x2200);
	bcm963xx_pcie_mdio_write(pdrv, 0, 0x05, 0x5044);
	bcm963xx_pcie_mdio_write(pdrv, 0, 0x06, 0xfef1);
	bcm963xx_pcie_mdio_write(pdrv, 0, 0x07, 0xe818);

	HCD_FN_EXT();

	return;
}


/*
 *
 * Function bcm963xx_pcie_gen2_phy_enable_ssc (pdrv,enable)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *    enable...flag to specify enable or disable SSC
 *
 *   Description:
 *   Enable/disable SSC for GEN2 cores. Assumed that SSC is configured before enabling the SSC
 *
 *  Return: 0:     on success or no action.
 *              -1:   on failure or timeout
 */
int bcm963xx_pcie_gen2_phy_enable_ssc(struct bcm963xx_pcie_hcd *pdrv,
	bool enable)
{
	uint16 data = 0;
	int timeout = 40;
	int ret = 0;

	HCD_FN_ENT();

	/* Nothing to do, if SSC is not configured */
	if (pdrv->hc_cfg.ssc == FALSE) {
	    HCD_FN_EXT();
	    return ret;
	}

	/*
	 * SSC disabled when PCIe core comes out of reset to allow PLL sync to happen
	 * write sscControl0 register ssc_mode_enable_ovrd & ssc_mode_enable_ovrd_val
	 */
	bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x1100);
	data = bcm963xx_pcie_mdio_read(pdrv, 0, 0x02);
	if (enable == TRUE)
	    data |= 0xc000;     /* bit 15:14 11'b to enable SSC */
	else
	    data &= ~0xc000;    /* bit 15:14 00'b to disable SSC */
	bcm963xx_pcie_mdio_write(pdrv, 0, 0x02, data);

	/* TODO: Check the status to see if SSC is set or not */
	while (timeout-- > 0) {
	    data = bcm963xx_pcie_mdio_read(pdrv, 0, 1);
	    /* Bit-31=1 is DONE */
	    if (((data & (1<<10)) >> 10) == enable)
	        break;
	    timeout = timeout - 1;
	    udelay(1000);
	}

	if (timeout == 0) {
	    HCD_ERROR("[%d] failed to %s SSC\n", pdrv->core_id,
	        (enable) ? "Enable" : "Disable");
	    ret = -1;
	} else {
	    HCD_LOG("[%d] SSC %s\n", pdrv->core_id,
	       (enable) ? "Enabled" : "Disabled");
	}

	HCD_FN_EXT();
	return ret;
}

/*
 *
 * Function bcm963xx_pcie_phy_config_rescal (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *     Configure PCIe PHY rescal through MDIO interface. The settings
 *     largely comes from ASIC design team
 *
 *  Return: None
 */
void bcm963xx_pcie_phy_config_rescal(struct bcm963xx_pcie_hcd *pdrv)
{
#if defined(RCAL_1UM_VERT)
	uint16 rddata, wrdata;
	int val;
#endif /* RCAL_1UM_VERT */

	HCD_FN_ENT();

#if defined(RCAL_1UM_VERT)
	if (GetRCalSetting(RCAL_1UM_VERT, &val) == kPMC_NO_ERROR)
	{
	    HCD_LOG("Core [%d] setting resistor calibration value to 0x%x\n",
	        pdrv->core_id, val);

	    if (pdrv->core_gen == PCIE_LINK_SPEED_GEN3) {
	        /*
	         *    ' Block 0x1000, Register 0, bit8=enable, bits 7:4=val
	         *    tmp = pcie_mdio_write (0, &h1f&, &hffd0&) ' AER block
	         *    tmp = pcie_mdio_write (0, &h1e&, &h0800&) ' PMA_PMD
	         *
	         *    tmp = pcie_mdio_write(0, &h1f&, &h1000&)
	         *    data = pcie_mdio_read(0, &h00&)
	         *    data = (data And Not(lshift(&h1f,4))) Or lshift(1,8) Or lshift(val,4)
	         *    tmp = pcie_mdio_write(0, &h00&, data)
	         */

	        /*
	         * For Gen3, first Select the lane
	         *
	         * Block:0xffd0 (AER), Register:0x1e (AER)
	         * Value:0x09ff bit[15:11] 1(PMA_PMD) bit[10:0] 0x000 (lane0)
	         */
	        bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0xffd0);
	        bcm963xx_pcie_mdio_write(pdrv, 0, 0x1e, 0x0800);

	        /*
	         * Block:0x1000 (PCIE_BLK0), Register:0x00 (PCIE_BLK0_ctrl1)
	         * bit[7:4] val (rescal_force_val) bit[8] 1 (rescal_force)
	         */
	        bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x1000);
	        rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x00);
	        wrdata = ((rddata & 0xff0f) | ((val & 0xf) << 4) | (1 << 8)); /* enable */
	        bcm963xx_pcie_mdio_write(pdrv, 0, 0x00, wrdata);
	    } else {
	        /*
	         * Rcal Calibration Timers
	         *	 Block 0x1000, Register 1, bit 4(enable), and 3:0 (value)
	         */
	        bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x1000);
	        rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 1);
	        wrdata = ((rddata & 0xffe0) | (val & 0xf) | (1 << 4)); /* enable */
	        bcm963xx_pcie_mdio_write(pdrv, 0, 1, wrdata);
	    }
	}
#endif /* RCAL_1UM_VERT */

	HCD_FN_EXT();

	return;
}
