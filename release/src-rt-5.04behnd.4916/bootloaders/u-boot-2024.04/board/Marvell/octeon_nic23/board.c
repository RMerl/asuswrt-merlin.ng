// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021-2022 Stefan Roese <sr@denx.de>
 */

#include <cyclic.h>
#include <dm.h>
#include <event.h>
#include <ram.h>
#include <time.h>
#include <asm/gpio.h>

#include <mach/octeon_ddr.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/octeon_fdt.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-bgxx-defs.h>
#include <mach/cvmx-dtx-defs.h>

#include "board_ddr.h"

/**
 * cvmx_spem#_cfg_rd
 *
 * This register allows read access to the configuration in the PCIe core.
 *
 */
union cvmx_spemx_cfg_rd {
	u64 u64;
	struct cvmx_spemx_cfg_rd_s {
		u64 data                         : 32;
		u64 addr                         : 32;
	} s;
	struct cvmx_spemx_cfg_rd_s            cn73xx;
};

/**
 * cvmx_spem#_cfg_wr
 *
 * This register allows write access to the configuration in the PCIe core.
 *
 */
union cvmx_spemx_cfg_wr {
	u64 u64;
	struct cvmx_spemx_cfg_wr_s {
		u64 data                         : 32;
		u64 addr                         : 32;
	} s;
	struct cvmx_spemx_cfg_wr_s            cn73xx;
};

/**
 * cvmx_spem#_flr_pf_stopreq
 *
 * PF function level reset stop outbound requests register.
 * Hardware automatically sets the STOPREQ bit for the PF when it enters a
 * function level reset (FLR).  Software is responsible for clearing the STOPREQ
 * bit but must not do so prior to hardware taking down the FLR, which could be
 * as long as 100ms.  It may be appropriate for software to wait longer before clearing
 * STOPREQ, software may need to drain deep DPI queues for example.
 * Whenever SPEM receives a PF or child VF request mastered by CNXXXX over S2M (i.e. P or NP),
 * when STOPREQ is set for the function, SPEM will discard the outgoing request
 * before sending it to the PCIe core.  If a NP, SPEM will schedule an immediate
 * SWI_RSP_ERROR completion for the request - no timeout is required.
 * In both cases, SPEM()_DBG_PF()_INFO[P()_BMD_E] will be set and a error
 * interrupt is generated.
 *
 * STOPREQ mimics the behavior of PCIEEP()_CFG001[ME] for outbound requests that will
 * master the PCIe bus (P and NP).
 *
 * STOPREQ will have no effect on completions returned by CNXXXX over the S2M,
 * nor on M2S traffic.
 *
 * When a PF()_STOPREQ is set, none of the associated
 * PEM()_FLR_PF()_VF_STOPREQ[VF_STOPREQ] will be set.
 *
 * STOPREQ is reset when the MAC is reset, and is not reset after a chip soft reset.
 */
union cvmx_spemx_flr_pf_stopreq {
	u64 u64;
	struct cvmx_spemx_flr_pf_stopreq_s {
		u64 reserved_3_63                : 61;
		u64 pf2_stopreq                  : 1;
		u64 pf1_stopreq                  : 1;
		u64 pf0_stopreq                  : 1;
	} s;
	struct cvmx_spemx_flr_pf_stopreq_s    cn73xx;
};

#define CVMX_SPEMX_CFG_WR(offset)		0x00011800C0000028ull
#define CVMX_SPEMX_CFG_RD(offset)		0x00011800C0000030ull
#define CVMX_SPEMX_FLR_PF_STOPREQ(offset)	0x00011800C0000218ull

#define DTX_SELECT_LTSSM		0x0
#define DTX_SELECT_LTSSM_ENA		0x3ff
#define LTSSM_L0			0x11

#define NIC23_DEF_DRAM_FREQ		800

static u32 pci_cfgspace_reg0[2] = { 0, 0 };

static u8 octeon_nic23_cfg0_spd_values[512] = {
	OCTEON_NIC23_CFG0_SPD_VALUES
};

static struct ddr_conf board_ddr_conf[] = {
	 OCTEON_NIC23_DDR_CONFIGURATION
};

struct ddr_conf *octeon_ddr_conf_table_get(int *count, int *def_ddr_freq)
{
	*count = ARRAY_SIZE(board_ddr_conf);
	*def_ddr_freq = NIC23_DEF_DRAM_FREQ;

	return board_ddr_conf;
}

int board_fix_fdt(void *fdt)
{
	u32 range_data[5 * 8];
	bool rev4;
	int node;
	int rc;

	/*
	 * ToDo:
	 * Read rev4 info from EEPROM or where the original U-Boot does
	 * and don't hard-code it here.
	 */
	rev4 = true;

	debug("%s() rev4: %s\n", __func__, rev4 ? "true" : "false");
	/* Patch the PHY configuration based on board revision */
	rc = octeon_fdt_patch_rename(fdt,
				     rev4 ? "4,nor-flash" : "4,no-nor-flash",
				     "cavium,board-trim", false, NULL, NULL);
	if (!rev4) {
		/* Modify the ranges for CS 0 */
		node = fdt_node_offset_by_compatible(fdt, -1,
						     "cavium,octeon-3860-bootbus");
		if (node < 0) {
			printf("%s: Error: cannot find boot bus in device tree!\n",
			       __func__);
			return -1;
		}

		rc = fdtdec_get_int_array(fdt, node, "ranges",
					  range_data, 5 * 8);
		if (rc) {
			printf("%s: Error reading ranges from boot bus FDT\n",
			       __func__);
			return -1;
		}
		range_data[2] = cpu_to_fdt32(0x10000);
		range_data[3] = 0;
		range_data[4] = 0;
		rc = fdt_setprop(fdt, node, "ranges", range_data,
				 sizeof(range_data));
		if (rc) {
			printf("%s: Error updating boot bus ranges in fdt\n",
			       __func__);
		}
	}
	return rc;
}

int board_early_init_f(void)
{
	struct gpio_desc gpio = {};
	ofnode node;

	/* Initial GPIO configuration */

	/* GPIO 7: Vitesse reset */
	node = ofnode_by_compatible(ofnode_null(), "vitesse,vsc7224");
	if (ofnode_valid(node)) {
		gpio_request_by_name_nodev(node, "los", 0, &gpio, GPIOD_IS_IN);
		dm_gpio_free(gpio.dev, &gpio);
		gpio_request_by_name_nodev(node, "reset", 0, &gpio,
					   GPIOD_IS_OUT);
		if (dm_gpio_is_valid(&gpio)) {
			/* Vitesse reset */
			debug("%s: Setting GPIO 7 to 1\n", __func__);
			dm_gpio_set_value(&gpio, 1);
		}
		dm_gpio_free(gpio.dev, &gpio);
	}

	/* SFP+ transmitters */
	ofnode_for_each_compatible_node(node, "ethernet,sfp-slot") {
		gpio_request_by_name_nodev(node, "tx_disable", 0,
					   &gpio, GPIOD_IS_OUT);
		if (dm_gpio_is_valid(&gpio)) {
			debug("%s: Setting GPIO %d to 1\n", __func__,
			      gpio.offset);
			dm_gpio_set_value(&gpio, 1);
		}
		dm_gpio_free(gpio.dev, &gpio);
		gpio_request_by_name_nodev(node, "mod_abs", 0, &gpio,
					   GPIOD_IS_IN);
		dm_gpio_free(gpio.dev, &gpio);
		gpio_request_by_name_nodev(node, "tx_error", 0, &gpio,
					   GPIOD_IS_IN);
		dm_gpio_free(gpio.dev, &gpio);
		gpio_request_by_name_nodev(node, "rx_los", 0, &gpio,
					   GPIOD_IS_IN);
		dm_gpio_free(gpio.dev, &gpio);
	}

	return 0;
}

void board_configure_qlms(void)
{
	octeon_configure_qlm(4, 3000, CVMX_QLM_MODE_SATA_2X1, 0, 0, 0, 0);
	octeon_configure_qlm(5, 103125, CVMX_QLM_MODE_XFI_1X2, 0, 0, 2, 0);
	/* Apply amplitude tuning to 10G interface */
	octeon_qlm_tune_v3(0, 4, 3000, -1, -1, 7, -1);
	octeon_qlm_tune_v3(0, 5, 103125, 0x19, 0x0, -1, -1);
	octeon_qlm_set_channel_v3(0, 5, 0);
	octeon_qlm_dfe_disable(0, 5, -1, 103125, CVMX_QLM_MODE_XFI_1X2);
	debug("QLM 4 reference clock: %d\n"
	      "DLM 5 reference clock: %d\n",
	      cvmx_qlm_measure_clock(4), cvmx_qlm_measure_clock(5));
}

/**
 * If there is a PF FLR then the PCI EEPROM is not re-read.  In this case
 * we need to re-program the vendor and device ID immediately after hardware
 * completes FLR.
 *
 * PCI spec requires FLR to be completed within 100ms.  The user who triggered
 * FLR expects hardware to finish FLR within 100ms, otherwise the user will
 * end up reading DEVICE_ID incorrectly from the reset value.
 * CN23XX exits FLR at any point between 66 and 99ms, so U-Boot has to wait
 * 99ms to let hardware finish its part, then finish reprogramming the
 * correct device ID before the end of 100ms.
 *
 * Note: this solution only works properly when there is no other activity
 * within U-Boot for 100ms from the time FLR is triggered.
 *
 * This function gets called every 100usec.  If FLR happens during any
 * other activity like bootloader/image update then it is possible that
 * this function does not get called for more than the FLR period which will
 * cause the PF device ID restore to happen after whoever initiated the FLR to
 * read the incorrect device ID 0x9700 (reset value) instead of 0x9702
 * (restored value).
 */
static void octeon_board_restore_pf(void *ctx)
{
	union cvmx_spemx_flr_pf_stopreq stopreq;
	static bool start_initialized[2] = {false, false};
	bool pf0_flag, pf1_flag;
	u64 ltssm_bits;
	const u64 pf_flr_wait_usecs = 99700;
	u64 elapsed_usecs;
	union cvmx_spemx_cfg_wr cfg_wr;
	union cvmx_spemx_cfg_rd cfg_rd;
	static u64 start_us[2];
	int pf_num;

	csr_wr(CVMX_DTX_SPEM_SELX(0), DTX_SELECT_LTSSM);
	csr_rd(CVMX_DTX_SPEM_SELX(0));
	csr_wr(CVMX_DTX_SPEM_ENAX(0), DTX_SELECT_LTSSM_ENA);
	csr_rd(CVMX_DTX_SPEM_ENAX(0));
	ltssm_bits = csr_rd(CVMX_DTX_SPEM_DATX(0));
	if (((ltssm_bits >> 3) & 0x3f) != LTSSM_L0)
		return;

	stopreq.u64 = csr_rd(CVMX_SPEMX_FLR_PF_STOPREQ(0));
	pf0_flag = stopreq.s.pf0_stopreq;
	pf1_flag = stopreq.s.pf1_stopreq;
	/* See if PF interrupt happened */
	if (!(pf0_flag || pf1_flag))
		return;

	if (pf0_flag && !start_initialized[0]) {
		start_initialized[0] = true;
		start_us[0] = get_timer_us(0);
	}

	/* Store programmed PCIe DevID SPEM0 PF0 */
	if (pf0_flag && !pci_cfgspace_reg0[0]) {
		cfg_rd.s.addr = (0 << 24) | 0x0;
		csr_wr(CVMX_SPEMX_CFG_RD(0), cfg_rd.u64);
		cfg_rd.u64 = csr_rd(CVMX_SPEMX_CFG_RD(0));
		pci_cfgspace_reg0[0] = cfg_rd.s.data;
	}

	if (pf1_flag && !start_initialized[1]) {
		start_initialized[1] = true;
		start_us[1] = get_timer_us(0);
	}

	/* Store programmed PCIe DevID SPEM0 PF1 */
	if (pf1_flag && !pci_cfgspace_reg0[1]) {
		cfg_rd.s.addr = (1 << 24) | 0x0;
		csr_wr(CVMX_SPEMX_CFG_RD(0), cfg_rd.u64);
		cfg_rd.u64 = csr_rd(CVMX_SPEMX_CFG_RD(0));
		pci_cfgspace_reg0[1] = cfg_rd.s.data;
	}

	/* For PF, rewrite pci config space reg 0 */
	for (pf_num = 0; pf_num < 2; pf_num++) {
		if (!start_initialized[pf_num])
			continue;

		elapsed_usecs = get_timer_us(0) - start_us[pf_num];

		if (elapsed_usecs > pf_flr_wait_usecs) {
			/* Here, our measured FLR duration has passed;
			 * check if device ID has been reset,
			 * which indicates FLR completion (per MA team).
			 */
			cfg_rd.s.addr = (pf_num << 24) | 0x0;
			csr_wr(CVMX_SPEMX_CFG_RD(0), cfg_rd.u64);
			cfg_rd.u64 = csr_rd(CVMX_SPEMX_CFG_RD(0));
			/* if DevID has NOT been reset, FLR is not yet
			 * complete
			 */
			if (cfg_rd.s.data != pci_cfgspace_reg0[pf_num]) {
				stopreq.s.pf0_stopreq = (pf_num == 0) ? 1 : 0;
				stopreq.s.pf1_stopreq = (pf_num == 1) ? 1 : 0;
				csr_wr(CVMX_SPEMX_FLR_PF_STOPREQ(0), stopreq.u64);

				cfg_wr.u64 = 0;
				cfg_wr.s.addr = (pf_num << 24) | 0;
				cfg_wr.s.data = pci_cfgspace_reg0[pf_num];
				csr_wr(CVMX_SPEMX_CFG_WR(0), cfg_wr.u64);
				start_initialized[pf_num] = false;
			}
		}
	}
}

int board_late_init(void)
{
	struct cyclic_info *cyclic;
	struct gpio_desc gpio = {};
	ofnode node;

	/* Turn on SFP+ transmitters */
	ofnode_for_each_compatible_node(node, "ethernet,sfp-slot") {
		gpio_request_by_name_nodev(node, "tx_disable", 0,
					   &gpio, GPIOD_IS_OUT);
		if (dm_gpio_is_valid(&gpio)) {
			debug("%s: Setting GPIO %d to 0\n", __func__,
			      gpio.offset);
			dm_gpio_set_value(&gpio, 0);
		}
		dm_gpio_free(gpio.dev, &gpio);
	}

	board_configure_qlms();

	/* Register cyclic function for PCIe FLR fixup */
	cyclic = cyclic_register(octeon_board_restore_pf, 100,
				 "pcie_flr_fix", NULL);
	if (!cyclic)
		printf("Registering of cyclic function failed\n");

	return 0;
}

static int last_stage_init(void)
{
	struct gpio_desc gpio = {};
	ofnode node;

	node = ofnode_by_compatible(ofnode_null(), "vitesse,vsc7224");
	if (!ofnode_valid(node)) {
		printf("Vitesse SPF DT node not found!");
		return 0;
	}

	gpio_request_by_name_nodev(node, "reset", 0, &gpio, GPIOD_IS_OUT);
	if (dm_gpio_is_valid(&gpio)) {
		/* Take Vitesse retimer out of reset */
		debug("%s: Setting GPIO 7 to 0\n", __func__);
		dm_gpio_set_value(&gpio, 0);
		mdelay(50);
	}
	dm_gpio_free(gpio.dev, &gpio);

	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, last_stage_init);
