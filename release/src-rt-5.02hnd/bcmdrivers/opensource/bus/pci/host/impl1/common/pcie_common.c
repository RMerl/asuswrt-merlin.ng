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

#include <pcie_common.h>
#include <pcie-bcm963xx.h>


/**************************************
  *
  *  Defines
  *
  **************************************/

/**************************************
 *
 *  Macros
 *
 **************************************/
#define HCD_HC_CORE_CFG(config, core)              \
	(((0xF << ((core)*4)) & (config)) >> ((core)*4))


/**************************************
 *
 *  Structures
 *
 **************************************/

/**************************************
 *
 *  external Functions prototypes and variables
 *
 **************************************/
uint16 bcm963xx_pcie_mdio_read (struct bcm963xx_pcie_hcd *pdrv,
	uint16 phyad, uint16 regad);
int bcm963xx_pcie_mdio_write (struct bcm963xx_pcie_hcd *pdrv,
	uint16 phyad, uint16 regad, uint16 wrdata);


/**************************************
 *
 *  Global variables
 *
 **************************************/
/*
 * config_ssc values
 *
 *     0 - disable
 *     1 - Enable
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16][15-12] [11-08] [07-04] [03-00]
 *                                                                        [core 3] [core 2] [core 1]
 */
u32 pcie_ssc_cfg = 0x0000;
module_param(pcie_ssc_cfg, int, S_IRUGO);

/*
 * config_speed values
 *
 *     0 - default (keep reset value)
 *     1 - 2.5Gbps
 *     2 - 5Gbps
 *     3 - 8 Gbps
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16][15-12] [11-08] [07-04] [03-00]
 *                                                                        [core 3] [core 2] [core 1]
 */
u32 pcie_speed_cfg = 0x0000;
module_param(pcie_speed_cfg, int, S_IRUGO);

/***********
  * HCD Logging
  ***********/
#ifdef HCD_DEBUG
int hcd_log_level = HCD_LOG_LVL_ERROR;
#endif

/**************************************
 *
 *  Global Function definitions
 *
 **************************************/

/*
  *
  * Function bcm963xx_pcie_hcd_init_hc_cfg (pdrv)
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
void bcm963xx_pcie_hcd_init_hc_cfg(struct bcm963xx_pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	pdrv->hc_cfg.ssc = HCD_HC_CORE_CFG(pcie_ssc_cfg, pdrv->core_id) ? TRUE : FALSE;
	pdrv->hc_cfg.speed = HCD_HC_CORE_CFG(pcie_speed_cfg, pdrv->core_id);

	HCD_FN_EXT();
}

/*
  *
  * Function bcm963xx_pcie_phy_config_ssc (pdrv)
  *
  *
  *   Parameters:
  *    pdrv ... pointer to pcie core hcd data structure
  *    enable...flag to specify enable or disable SSC
  *
  *   Description:
  *     Configure PCIe PHY through MDIO interface. The settings
  *     largely comes from ASIC design team
  *
  *  Return: None
  */
void bcm963xx_pcie_phy_config_ssc(struct bcm963xx_pcie_hcd *pdrv)
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
	 * Workaround:
	 * Block 0x1100, Register 0xA = 0xea3c
	 * Block 0x1100, Register 0xB = 0x04e7
	 * Block 0x1100, Register 0xC = 0x0039
	 * Block 0x2200, Register 5 = 0x5044    // VCO parameters for fractional mode, -175ppm
	 * Block 0x2200, Register 6 = 0xfef1    // VCO parameters for fractional mode, -175ppm
	 * Block 0x2200, Register 7 = 0xe818    // VCO parameters for fractional mode, -175ppm
	 * Notes:
	 * -Only need to apply these fixes when enabling Spread Spectrum Clocking (SSC),
	 *   which would likely be a flash option
	 * -Block 0x1100 fixed in 63148A0, 63381B0, 63138B0 but ok to write anyway
	 */
	bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x1100);
	bcm963xx_pcie_mdio_write(pdrv, 0, 0x0a, 0xea3c);
	bcm963xx_pcie_mdio_write(pdrv, 0, 0x0b, 0x04e7);
	bcm963xx_pcie_mdio_write(pdrv, 0, 0x0c, 0x0039);

	bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x2200);
	bcm963xx_pcie_mdio_write(pdrv, 0, 0x05, 0x5044);
	bcm963xx_pcie_mdio_write(pdrv, 0, 0x06, 0xfef1);
	bcm963xx_pcie_mdio_write(pdrv, 0, 0x07, 0xe818);

	HCD_FN_EXT();

	return;
}


/*
  *
  * Function bcm963xx_pcie_phy_enable_ssc (pdrv,enable)
  *
  *
  *   Parameters:
  *    pdrv ... pointer to pcie core hcd data structure
  *    enable...flag to specify enable or disable SSC
  *
  *   Description:
  *   Enable/disable SSC. Assumed that SSC is configured before enabling the SSC
  *
  *  Return: 0:     on success or no action.
  *              -1:   on failure or timeout
  */
int bcm963xx_pcie_phy_enable_ssc(struct bcm963xx_pcie_hcd *pdrv,
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
	data = bcm963xx_pcie_mdio_read (pdrv, 0, 0x02);
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
