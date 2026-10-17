// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Marvell International Ltd.
 */

#include <common.h>
#include <fdtdec.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/ptrace.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/delay.h>
#include <linux/printk.h>

#include "comphy_core.h"
#include "sata.h"
#include "utmi_phy.h"

DECLARE_GLOBAL_DATA_PTR;

/* Firmware related definitions used for SMC calls */
#define MV_SIP_COMPHY_POWER_ON	0x82000001
#define MV_SIP_COMPHY_POWER_OFF	0x82000002
#define MV_SIP_COMPHY_PLL_LOCK	0x82000003
#define MV_SIP_COMPHY_XFI_TRAIN	0x82000004

/* Used to distinguish between different possible callers (U-Boot/Linux) */
#define COMPHY_CALLER_UBOOT			(0x1 << 21)

#define COMPHY_FW_MODE_FORMAT(mode)		((mode) << 12)
#define COMPHY_FW_FORMAT(mode, idx, speeds)	\
			(((mode) << 12) | ((idx) << 8) | ((speeds) << 2))

#define COMPHY_FW_PCIE_FORMAT(pcie_width, clk_src, mode, speeds)	\
			(COMPHY_CALLER_UBOOT | ((pcie_width) << 18) |	\
			((clk_src) << 17) | COMPHY_FW_FORMAT(mode, 0, speeds))

/* Invert polarity are bits 1-0 of the mode */
#define COMPHY_FW_SATA_FORMAT(mode, invert)	\
			((invert) | COMPHY_FW_MODE_FORMAT(mode))

#define COMPHY_SATA_MODE	0x1
#define COMPHY_SGMII_MODE	0x2	/* SGMII 1G */
#define COMPHY_HS_SGMII_MODE	0x3	/* SGMII 2.5G */
#define COMPHY_USB3H_MODE	0x4
#define COMPHY_USB3D_MODE	0x5
#define COMPHY_PCIE_MODE	0x6
#define COMPHY_RXAUI_MODE	0x7
#define COMPHY_XFI_MODE		0x8
#define COMPHY_SFI_MODE		0x9
#define COMPHY_USB3_MODE	0xa
#define COMPHY_AP_MODE		0xb

/* Comphy unit index macro */
#define COMPHY_UNIT_ID0		0
#define COMPHY_UNIT_ID1		1
#define COMPHY_UNIT_ID2		2
#define COMPHY_UNIT_ID3		3

struct utmi_phy_data {
	void __iomem *utmi_pll_addr;
	void __iomem *utmi_base_addr;
	void __iomem *usb_cfg_addr;
	void __iomem *utmi_cfg_addr;
	u32 utmi_phy_port;
};

static u32 polling_with_timeout(void __iomem *addr, u32 val,
				u32 mask, unsigned long usec_timout)
{
	u32 data;

	do {
		udelay(1);
		data = readl(addr) & mask;
	} while (data != val  && --usec_timout > 0);

	if (usec_timout == 0)
		return data;

	return 0;
}

static int comphy_smc(u32 function_id, void __iomem *comphy_base_addr,
		      u32 lane, u32 mode)
{
	struct pt_regs pregs = {0};

	pregs.regs[0] = function_id;
	pregs.regs[1] = (unsigned long)comphy_base_addr;
	pregs.regs[2] = lane;
	pregs.regs[3] = mode;

	smc_call(&pregs);

	/*
	 * TODO: Firmware return 0 on success, temporary map it to u-boot
	 * convention, but after all comphy will be reworked the convention in
	 * u-boot should be change and this conversion removed
	 */
	return pregs.regs[0] ? 0 : 1;
}

/* This function performs RX training for all FFE possible values.
 * We get the result for each FFE and eventually the best FFE will
 * be used and set to the HW.
 *
 * Return '1' on succsess.
 * Return '0' on failure.
 */
int comphy_cp110_sfi_rx_training(struct chip_serdes_phy_config *ptr_chip_cfg,
				 u32 lane)
{
	int ret;
	u32 type = ptr_chip_cfg->comphy_map_data[lane].type;

	debug_enter();

	if (type != COMPHY_TYPE_SFI0 && type != COMPHY_TYPE_SFI1) {
		pr_err("Comphy %d isn't configured to SFI\n", lane);
		return 0;
	}

	/* Mode is not relevant for xfi training */
	ret = comphy_smc(MV_SIP_COMPHY_XFI_TRAIN,
			 ptr_chip_cfg->comphy_base_addr, lane, 0);

	debug_exit();

	return ret;
}

static int comphy_sata_power_up(u32 lane, void __iomem *hpipe_base,
				void __iomem *comphy_base_addr, int cp_index,
				u32 type)
{
	u32 mask, data, i, ret = 1;
	void __iomem *sata_base = NULL;
	int sata_node = -1; /* Set to -1 in order to read the first sata node */

	debug_enter();

	/*
	 * Assumption - each CP has only one SATA controller
	 * Calling fdt_node_offset_by_compatible first time (with sata_node = -1
	 * will return the first node always.
	 * In order to parse each CPs SATA node, fdt_node_offset_by_compatible
	 * must be called again (according to the CP id)
	 */
	for (i = 0; i < (cp_index + 1); i++)
		sata_node = fdt_node_offset_by_compatible(
			gd->fdt_blob, sata_node, "marvell,armada-8k-ahci");

	if (sata_node == 0) {
		pr_err("SATA node not found in FDT\n");
		return 0;
	}

	sata_base = (void __iomem *)fdtdec_get_addr_size_auto_noparent(
		gd->fdt_blob, sata_node, "reg", 0, NULL, true);
	if (sata_base == NULL) {
		pr_err("SATA address not found in FDT\n");
		return 0;
	}

	debug("SATA address found in FDT %p\n", sata_base);

	debug("stage: MAC configuration - power down comphy\n");
	/*
	 * MAC configuration powe down comphy use indirect address for
	 * vendor spesific SATA control register
	 */
	reg_set(sata_base + SATA3_VENDOR_ADDRESS,
		SATA_CONTROL_REG << SATA3_VENDOR_ADDR_OFSSET,
		SATA3_VENDOR_ADDR_MASK);
	/* SATA 0 power down */
	mask = SATA3_CTRL_SATA0_PD_MASK;
	data = 0x1 << SATA3_CTRL_SATA0_PD_OFFSET;
	/* SATA 1 power down */
	mask |= SATA3_CTRL_SATA1_PD_MASK;
	data |= 0x1 << SATA3_CTRL_SATA1_PD_OFFSET;
	/* SATA SSU disable */
	mask |= SATA3_CTRL_SATA1_ENABLE_MASK;
	data |= 0x0 << SATA3_CTRL_SATA1_ENABLE_OFFSET;
	/* SATA port 1 disable */
	mask |= SATA3_CTRL_SATA_SSU_MASK;
	data |= 0x0 << SATA3_CTRL_SATA_SSU_OFFSET;
	reg_set(sata_base + SATA3_VENDOR_DATA, data, mask);

	ret = comphy_smc(MV_SIP_COMPHY_POWER_ON, comphy_base_addr, lane, type);

	/*
	 * MAC configuration power up comphy - power up PLL/TX/RX
	 * use indirect address for vendor spesific SATA control register
	 */
	reg_set(sata_base + SATA3_VENDOR_ADDRESS,
		SATA_CONTROL_REG << SATA3_VENDOR_ADDR_OFSSET,
		SATA3_VENDOR_ADDR_MASK);
	/* SATA 0 power up */
	mask = SATA3_CTRL_SATA0_PD_MASK;
	data = 0x0 << SATA3_CTRL_SATA0_PD_OFFSET;
	/* SATA 1 power up */
	mask |= SATA3_CTRL_SATA1_PD_MASK;
	data |= 0x0 << SATA3_CTRL_SATA1_PD_OFFSET;
	/* SATA SSU enable */
	mask |= SATA3_CTRL_SATA1_ENABLE_MASK;
	data |= 0x1 << SATA3_CTRL_SATA1_ENABLE_OFFSET;
	/* SATA port 1 enable */
	mask |= SATA3_CTRL_SATA_SSU_MASK;
	data |= 0x1 << SATA3_CTRL_SATA_SSU_OFFSET;
	reg_set(sata_base + SATA3_VENDOR_DATA, data, mask);

	/* MBUS request size and interface select register */
	reg_set(sata_base + SATA3_VENDOR_ADDRESS,
		SATA_MBUS_SIZE_SELECT_REG << SATA3_VENDOR_ADDR_OFSSET,
		SATA3_VENDOR_ADDR_MASK);
	/* Mbus regret enable */
	reg_set(sata_base + SATA3_VENDOR_DATA,
		0x1 << SATA_MBUS_REGRET_EN_OFFSET, SATA_MBUS_REGRET_EN_MASK);

	ret = comphy_smc(MV_SIP_COMPHY_PLL_LOCK, comphy_base_addr, lane, type);

	debug_exit();
	return ret;
}

static void comphy_utmi_power_down(u32 utmi_index, void __iomem *utmi_base_addr,
				   void __iomem *usb_cfg_addr,
				   void __iomem *utmi_cfg_addr,
				   u32 utmi_phy_port)
{
	u32 mask, data;

	debug_enter();
	debug("stage:  UTMI %d - Power down transceiver (power down Phy), Power down PLL, and SuspendDM\n",
	      utmi_index);
	/* Power down UTMI PHY */
	reg_set(utmi_cfg_addr, 0x0 << UTMI_PHY_CFG_PU_OFFSET,
		UTMI_PHY_CFG_PU_MASK);

	/*
	 * If UTMI connected to USB Device, configure mux prior to PHY init
	 * (Device can be connected to UTMI0 or to UTMI1)
	 */
	if (utmi_phy_port == UTMI_PHY_TO_USB3_DEVICE0) {
		debug("stage:  UTMI %d - Enable Device mode and configure UTMI mux\n",
		      utmi_index);
		/* USB3 Device UTMI enable */
		mask = UTMI_USB_CFG_DEVICE_EN_MASK;
		data = 0x1 << UTMI_USB_CFG_DEVICE_EN_OFFSET;
		/* USB3 Device UTMI MUX */
		mask |= UTMI_USB_CFG_DEVICE_MUX_MASK;
		data |= utmi_index << UTMI_USB_CFG_DEVICE_MUX_OFFSET;
		reg_set(usb_cfg_addr,  data, mask);
	}

	/* Set Test suspendm mode */
	mask = UTMI_CTRL_STATUS0_SUSPENDM_MASK;
	data = 0x1 << UTMI_CTRL_STATUS0_SUSPENDM_OFFSET;
	/* Enable Test UTMI select */
	mask |= UTMI_CTRL_STATUS0_TEST_SEL_MASK;
	data |= 0x1 << UTMI_CTRL_STATUS0_TEST_SEL_OFFSET;
	reg_set(utmi_base_addr + UTMI_CTRL_STATUS0_REG, data, mask);

	/* Wait for UTMI power down */
	mdelay(1);

	debug_exit();
	return;
}

static void comphy_utmi_phy_config(u32 utmi_index, void __iomem *utmi_pll_addr,
				   void __iomem *utmi_base_addr,
				   void __iomem *usb_cfg_addr,
				   void __iomem *utmi_cfg_addr,
				   u32 utmi_phy_port)
{
	u32 mask, data;

	debug_exit();
	debug("stage: Configure UTMI PHY %d registers\n", utmi_index);
	/* Reference Clock Divider Select */
	mask = UTMI_PLL_CTRL_REFDIV_MASK;
	data = 0x5 << UTMI_PLL_CTRL_REFDIV_OFFSET;
	/* Feedback Clock Divider Select - 90 for 25Mhz*/
	mask |= UTMI_PLL_CTRL_FBDIV_MASK;
	data |= 0x60 << UTMI_PLL_CTRL_FBDIV_OFFSET;
	/* Select LPFR - 0x0 for 25Mhz/5=5Mhz*/
	mask |= UTMI_PLL_CTRL_SEL_LPFR_MASK;
	data |= 0x0 << UTMI_PLL_CTRL_SEL_LPFR_OFFSET;
	reg_set(utmi_pll_addr + UTMI_PLL_CTRL_REG, data, mask);

	/* Impedance Calibration Threshold Setting */
	mask = UTMI_CALIB_CTRL_IMPCAL_VTH_MASK;
	data = 0x7 << UTMI_CALIB_CTRL_IMPCAL_VTH_OFFSET;
	reg_set(utmi_pll_addr + UTMI_CALIB_CTRL_REG, data, mask);

	/* Start Impedance and PLL Calibration */
	mask = UTMI_CALIB_CTRL_PLLCAL_START_MASK;
	data = (0x1 << UTMI_CALIB_CTRL_PLLCAL_START_OFFSET);
	mask |= UTMI_CALIB_CTRL_IMPCAL_START_MASK;
	data |= (0x1 << UTMI_CALIB_CTRL_IMPCAL_START_OFFSET);
	reg_set(utmi_pll_addr + UTMI_CALIB_CTRL_REG, data, mask);

	/* Set LS TX driver strength coarse control */
	mask = UTMI_TX_CH_CTRL_AMP_MASK;
	data = 0x4 << UTMI_TX_CH_CTRL_AMP_OFFSET;
	mask |= UTMI_TX_CH_CTRL_IMP_SEL_LS_MASK;
	data |= 0x3 << UTMI_TX_CH_CTRL_IMP_SEL_LS_OFFSET;
	mask |= UTMI_TX_CH_CTRL_DRV_EN_LS_MASK;
	data |= 0x3 << UTMI_TX_CH_CTRL_DRV_EN_LS_OFFSET;
	reg_set(utmi_base_addr + UTMI_TX_CH_CTRL_REG, data, mask);

	/* Enable SQ */
	mask = UTMI_RX_CH_CTRL0_SQ_DET_MASK;
	data = 0x1 << UTMI_RX_CH_CTRL0_SQ_DET_OFFSET;
	/* Enable analog squelch detect */
	mask |= UTMI_RX_CH_CTRL0_SQ_ANA_DTC_MASK;
	data |= 0x0 << UTMI_RX_CH_CTRL0_SQ_ANA_DTC_OFFSET;
	mask |= UTMI_RX_CH_CTRL0_DISCON_THRESH_MASK;
	data |= 0x0 << UTMI_RX_CH_CTRL0_DISCON_THRESH_OFFSET;
	reg_set(utmi_base_addr + UTMI_RX_CH_CTRL0_REG, data, mask);

	/* Set External squelch calibration number */
	mask = UTMI_RX_CH_CTRL1_SQ_AMP_CAL_MASK;
	data = 0x1 << UTMI_RX_CH_CTRL1_SQ_AMP_CAL_OFFSET;
	/* Enable the External squelch calibration */
	mask |= UTMI_RX_CH_CTRL1_SQ_AMP_CAL_EN_MASK;
	data |= 0x1 << UTMI_RX_CH_CTRL1_SQ_AMP_CAL_EN_OFFSET;
	reg_set(utmi_base_addr + UTMI_RX_CH_CTRL1_REG, data, mask);

	/* Set Control VDAT Reference Voltage - 0.325V */
	mask = UTMI_CHGDTC_CTRL_VDAT_MASK;
	data = 0x1 << UTMI_CHGDTC_CTRL_VDAT_OFFSET;
	/* Set Control VSRC Reference Voltage - 0.6V */
	mask |= UTMI_CHGDTC_CTRL_VSRC_MASK;
	data |= 0x1 << UTMI_CHGDTC_CTRL_VSRC_OFFSET;
	reg_set(utmi_base_addr + UTMI_CHGDTC_CTRL_REG, data, mask);

	debug_exit();
	return;
}

static int comphy_utmi_power_up(u32 utmi_index, void __iomem *utmi_pll_addr,
				void __iomem *utmi_base_addr,
				void __iomem *usb_cfg_addr,
				void __iomem *utmi_cfg_addr, u32 utmi_phy_port)
{
	u32 data, mask, ret = 1;
	void __iomem *addr;

	debug_enter();
	debug("stage: UTMI %d - Power up transceiver(Power up Phy), and exit SuspendDM\n",
	      utmi_index);
	/* Power UP UTMI PHY */
	reg_set(utmi_cfg_addr, 0x1 << UTMI_PHY_CFG_PU_OFFSET,
		UTMI_PHY_CFG_PU_MASK);
	/* Disable Test UTMI select */
	reg_set(utmi_base_addr + UTMI_CTRL_STATUS0_REG,
		0x0 << UTMI_CTRL_STATUS0_TEST_SEL_OFFSET,
		UTMI_CTRL_STATUS0_TEST_SEL_MASK);

	debug("stage: Polling for PLL and impedance calibration done, and PLL ready done\n");
	addr = utmi_pll_addr + UTMI_CALIB_CTRL_REG;
	data = UTMI_CALIB_CTRL_IMPCAL_DONE_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask, 100);
	if (data != 0) {
		pr_err("Impedance calibration is not done\n");
		debug("Read from reg = %p - value = 0x%x\n", addr, data);
		ret = 0;
	}

	data = UTMI_CALIB_CTRL_PLLCAL_DONE_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask, 100);
	if (data != 0) {
		pr_err("PLL calibration is not done\n");
		debug("Read from reg = %p - value = 0x%x\n", addr, data);
		ret = 0;
	}

	addr = utmi_pll_addr + UTMI_PLL_CTRL_REG;
	data = UTMI_PLL_CTRL_PLL_RDY_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask, 100);
	if (data != 0) {
		pr_err("PLL is not ready\n");
		debug("Read from reg = %p - value = 0x%x\n", addr, data);
		ret = 0;
	}

	if (ret)
		debug("Passed\n");
	else
		debug("\n");

	debug_exit();
	return ret;
}

/*
 * comphy_utmi_phy_init initialize the UTMI PHY
 * the init split in 3 parts:
 * 1. Power down transceiver and PLL
 * 2. UTMI PHY configure
 * 3. Power up transceiver and PLL
 * Note: - Power down/up should be once for both UTMI PHYs
 *       - comphy_dedicated_phys_init call this function if at least there is
 *         one UTMI PHY exists in FDT blob. access to cp110_utmi_data[0] is
 *         legal
 */
static void comphy_utmi_phy_init(u32 utmi_phy_count,
				 struct utmi_phy_data *cp110_utmi_data)
{
	u32 i;

	debug_enter();
	/* UTMI Power down */
	for (i = 0; i < utmi_phy_count; i++) {
		comphy_utmi_power_down(i, cp110_utmi_data[i].utmi_base_addr,
				       cp110_utmi_data[i].usb_cfg_addr,
				       cp110_utmi_data[i].utmi_cfg_addr,
				       cp110_utmi_data[i].utmi_phy_port);
	}
	/* PLL Power down */
	debug("stage: UTMI PHY power down PLL\n");
	for (i = 0; i < utmi_phy_count; i++) {
		reg_set(cp110_utmi_data[i].usb_cfg_addr,
			0x0 << UTMI_USB_CFG_PLL_OFFSET, UTMI_USB_CFG_PLL_MASK);
	}
	/* UTMI configure */
	for (i = 0; i < utmi_phy_count; i++) {
		comphy_utmi_phy_config(i, cp110_utmi_data[i].utmi_pll_addr,
				       cp110_utmi_data[i].utmi_base_addr,
				       cp110_utmi_data[i].usb_cfg_addr,
				       cp110_utmi_data[i].utmi_cfg_addr,
				       cp110_utmi_data[i].utmi_phy_port);
	}
	/* UTMI Power up */
	for (i = 0; i < utmi_phy_count; i++) {
		if (!comphy_utmi_power_up(i, cp110_utmi_data[i].utmi_pll_addr,
					  cp110_utmi_data[i].utmi_base_addr,
					  cp110_utmi_data[i].usb_cfg_addr,
					  cp110_utmi_data[i].utmi_cfg_addr,
					  cp110_utmi_data[i].utmi_phy_port)) {
			pr_err("Failed to initialize UTMI PHY %d\n", i);
			continue;
		}
		printf("UTMI PHY %d initialized to ", i);
		if (cp110_utmi_data[i].utmi_phy_port ==
		    UTMI_PHY_TO_USB3_DEVICE0)
			printf("USB Device\n");
		else
			printf("USB Host%d\n",
			       cp110_utmi_data[i].utmi_phy_port);
	}
	/* PLL Power up */
	debug("stage: UTMI PHY power up PLL\n");
	for (i = 0; i < utmi_phy_count; i++) {
		reg_set(cp110_utmi_data[i].usb_cfg_addr,
			0x1 << UTMI_USB_CFG_PLL_OFFSET, UTMI_USB_CFG_PLL_MASK);
	}

	debug_exit();
	return;
}

/*
 * comphy_dedicated_phys_init initialize the dedicated PHYs
 * - not muxed SerDes lanes e.g. UTMI PHY
 */
void comphy_dedicated_phys_init(void)
{
	struct utmi_phy_data cp110_utmi_data[MAX_UTMI_PHY_COUNT];
	int node = -1;
	int node_idx;
	int parent = -1;

	debug_enter();
	debug("Initialize USB UTMI PHYs\n");

	for (node_idx = 0; node_idx < MAX_UTMI_PHY_COUNT;) {
		/* Find the UTMI phy node in device tree */
		node = fdt_node_offset_by_compatible(gd->fdt_blob, node,
						     "marvell,mvebu-utmi-2.6.0");
		if (node <= 0)
			break;

		/* check if node is enabled */
		if (!fdtdec_get_is_enabled(gd->fdt_blob, node))
			continue;

		parent = fdt_parent_offset(gd->fdt_blob, node);
		if (parent <= 0)
			break;

		/* get base address of UTMI PLL */
		cp110_utmi_data[node_idx].utmi_pll_addr =
			(void __iomem *)fdtdec_get_addr_size_auto_noparent(
				gd->fdt_blob, parent, "reg", 0, NULL, true);
		if (!cp110_utmi_data[node_idx].utmi_pll_addr) {
			pr_err("UTMI PHY PLL address is invalid\n");
			continue;
		}

		/* get base address of UTMI phy */
		cp110_utmi_data[node_idx].utmi_base_addr =
			(void __iomem *)fdtdec_get_addr_size_auto_noparent(
				gd->fdt_blob, node, "reg", 0, NULL, true);
		if (!cp110_utmi_data[node_idx].utmi_base_addr) {
			pr_err("UTMI PHY base address is invalid\n");
			continue;
		}

		/* get usb config address */
		cp110_utmi_data[node_idx].usb_cfg_addr =
			(void __iomem *)fdtdec_get_addr_size_auto_noparent(
				gd->fdt_blob, node, "reg", 1, NULL, true);
		if (!cp110_utmi_data[node_idx].usb_cfg_addr) {
			pr_err("UTMI PHY base address is invalid\n");
			continue;
		}

		/* get UTMI config address */
		cp110_utmi_data[node_idx].utmi_cfg_addr =
			(void __iomem *)fdtdec_get_addr_size_auto_noparent(
				gd->fdt_blob, node, "reg", 2, NULL, true);
		if (!cp110_utmi_data[node_idx].utmi_cfg_addr) {
			pr_err("UTMI PHY base address is invalid\n");
			continue;
		}

		/*
		 * get the port number (to check if the utmi connected to
		 * host/device)
		 */
		cp110_utmi_data[node_idx].utmi_phy_port = fdtdec_get_int(
			gd->fdt_blob, node, "utmi-port", UTMI_PHY_INVALID);
		if (cp110_utmi_data[node_idx].utmi_phy_port ==
							UTMI_PHY_INVALID) {
			pr_err("UTMI PHY port type is invalid\n");
			continue;
		}

		/* count valid UTMI unit */
		node_idx++;
	}

	if (node_idx > 0)
		comphy_utmi_phy_init(node_idx, cp110_utmi_data);

	debug_exit();
}

int comphy_cp110_init_serdes_map(int node, struct chip_serdes_phy_config *cfg)
{
	int lane, subnode;

	cfg->comphy_lanes_count = fdtdec_get_int(gd->fdt_blob, node,
						 "max-lanes", 0);
	if (cfg->comphy_lanes_count <= 0) {
		printf("comphy max lanes is wrong\n");
		return -EINVAL;
	}

	cfg->comphy_mux_bitcount = fdtdec_get_int(gd->fdt_blob, node,
						  "mux-bitcount", 0);
	if (cfg->comphy_mux_bitcount <= 0) {
		printf("comphy mux bit count is wrong\n");
		return -EINVAL;
	}

	cfg->comphy_mux_lane_order = fdtdec_locate_array(gd->fdt_blob, node,
							 "mux-lane-order",
							 cfg->comphy_lanes_count);

	lane = 0;
	fdt_for_each_subnode(subnode, gd->fdt_blob, node) {
		/* Skip disabled ports */
		if (!fdtdec_get_is_enabled(gd->fdt_blob, subnode))
			continue;

		cfg->comphy_map_data[lane].type =
			fdtdec_get_int(gd->fdt_blob, subnode, "phy-type",
				       COMPHY_TYPE_INVALID);

		if (cfg->comphy_map_data[lane].type == COMPHY_TYPE_INVALID) {
			printf("no phy type for lane %d, setting lane as unconnected\n",
			       lane + 1);
			continue;
		}

		cfg->comphy_map_data[lane].speed =
			fdtdec_get_int(gd->fdt_blob, subnode, "phy-speed",
				       COMPHY_SPEED_INVALID);

		cfg->comphy_map_data[lane].invert =
			fdtdec_get_int(gd->fdt_blob, subnode, "phy-invert",
				       COMPHY_POLARITY_NO_INVERT);

		cfg->comphy_map_data[lane].clk_src =
			fdtdec_get_bool(gd->fdt_blob, subnode, "clk-src");

		cfg->comphy_map_data[lane].end_point =
			fdtdec_get_bool(gd->fdt_blob, subnode, "end_point");

		lane++;
	}

	return 0;
}

int comphy_cp110_init(struct chip_serdes_phy_config *ptr_chip_cfg,
		      struct comphy_map *serdes_map)
{
	struct comphy_map *ptr_comphy_map;
	void __iomem *comphy_base_addr, *hpipe_base_addr;
	u32 comphy_max_count, lane, id, ret = 0;
	u32 pcie_width = 0;
	u32 mode;

	debug_enter();

	comphy_max_count = ptr_chip_cfg->comphy_lanes_count;
	comphy_base_addr = ptr_chip_cfg->comphy_base_addr;
	hpipe_base_addr = ptr_chip_cfg->hpipe3_base_addr;

	/* Check if the first 4 lanes configured as By-4 */
	for (lane = 0, ptr_comphy_map = serdes_map; lane < 4;
	     lane++, ptr_comphy_map++) {
		if (ptr_comphy_map->type != COMPHY_TYPE_PEX0)
			break;
		pcie_width++;
	}

	for (lane = 0, ptr_comphy_map = serdes_map; lane < comphy_max_count;
	     lane++, ptr_comphy_map++) {
		debug("Initialize serdes number %d\n", lane);
		debug("Serdes type = 0x%x\n", ptr_comphy_map->type);
		if (lane == 4) {
			/*
			 * PCIe lanes above the first 4 lanes, can be only
			 * by1
			 */
			pcie_width = 1;
		}
		switch (ptr_comphy_map->type) {
		case COMPHY_TYPE_UNCONNECTED:
			mode = COMPHY_TYPE_UNCONNECTED | COMPHY_CALLER_UBOOT;
			ret = comphy_smc(MV_SIP_COMPHY_POWER_OFF,
					 ptr_chip_cfg->comphy_base_addr,
					 lane, mode);
		case COMPHY_TYPE_IGNORE:
			continue;
			break;
		case COMPHY_TYPE_PEX0:
		case COMPHY_TYPE_PEX1:
		case COMPHY_TYPE_PEX2:
		case COMPHY_TYPE_PEX3:
			mode = COMPHY_FW_PCIE_FORMAT(pcie_width,
						     ptr_comphy_map->clk_src,
						     COMPHY_PCIE_MODE,
						     ptr_comphy_map->speed);
			ret = comphy_smc(MV_SIP_COMPHY_POWER_ON,
					 ptr_chip_cfg->comphy_base_addr, lane,
					 mode);
			break;
		case COMPHY_TYPE_SATA0:
		case COMPHY_TYPE_SATA1:
			mode = COMPHY_FW_SATA_FORMAT(COMPHY_SATA_MODE,
						     serdes_map[lane].invert);
			ret = comphy_sata_power_up(lane, hpipe_base_addr,
						   comphy_base_addr,
						   ptr_chip_cfg->cp_index,
						   mode);
			break;
		case COMPHY_TYPE_USB3_HOST0:
		case COMPHY_TYPE_USB3_HOST1:
			mode = COMPHY_FW_MODE_FORMAT(COMPHY_USB3H_MODE);
			ret = comphy_smc(MV_SIP_COMPHY_POWER_ON,
					 ptr_chip_cfg->comphy_base_addr, lane,
					 mode);
			break;
		case COMPHY_TYPE_USB3_DEVICE:
			mode = COMPHY_FW_MODE_FORMAT(COMPHY_USB3D_MODE);
			ret = comphy_smc(MV_SIP_COMPHY_POWER_ON,
					 ptr_chip_cfg->comphy_base_addr, lane,
					 mode);
			break;
		case COMPHY_TYPE_SGMII0:
		case COMPHY_TYPE_SGMII1:
		case COMPHY_TYPE_SGMII2:
			/* Calculate SGMII ID */
			id = ptr_comphy_map->type - COMPHY_TYPE_SGMII0;

			if (ptr_comphy_map->speed == COMPHY_SPEED_INVALID) {
				debug("Warning: SGMII PHY speed in lane %d is invalid, set PHY speed to 1.25G\n",
				      lane);
				ptr_comphy_map->speed = COMPHY_SPEED_1_25G;
			}

			mode = COMPHY_FW_FORMAT(COMPHY_SGMII_MODE, id,
						ptr_comphy_map->speed);
			ret = comphy_smc(MV_SIP_COMPHY_POWER_ON,
					 ptr_chip_cfg->comphy_base_addr, lane,
					 mode);
			break;
		case COMPHY_TYPE_SFI0:
		case COMPHY_TYPE_SFI1:
			/* Calculate SFI id */
			id = ptr_comphy_map->type - COMPHY_TYPE_SFI0;
			mode = COMPHY_FW_FORMAT(COMPHY_SFI_MODE, id,
						ptr_comphy_map->speed);
			ret = comphy_smc(MV_SIP_COMPHY_POWER_ON,
				ptr_chip_cfg->comphy_base_addr, lane, mode);
			break;
		case COMPHY_TYPE_RXAUI0:
		case COMPHY_TYPE_RXAUI1:
			mode = COMPHY_FW_MODE_FORMAT(COMPHY_RXAUI_MODE);
			ret = comphy_smc(MV_SIP_COMPHY_POWER_ON,
					 ptr_chip_cfg->comphy_base_addr, lane,
					 mode);
			break;
		default:
			debug("Unknown SerDes type, skip initialize SerDes %d\n",
			      lane);
			break;
		}
		if (ret == 0) {
			/*
			 * If interface wans't initialized, set the lane to
			 * COMPHY_TYPE_UNCONNECTED state.
			 */
			ptr_comphy_map->type = COMPHY_TYPE_UNCONNECTED;
			pr_err("PLL is not locked - Failed to initialize lane %d\n",
			      lane);
		}
	}

	debug_exit();
	return 0;
}
