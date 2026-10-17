// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 */

#include <errno.h>
#include <i2c.h>
#include <log.h>
#include <malloc.h>
#include <linux/delay.h>
#include <display_options.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/cvmx-bootmem.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/cvmx-pcie.h>
#include <mach/cvmx-coremask.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-fdt.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-gpio.h>
#include <mach/cvmx-helper-util.h>

extern void octeon_i2c_unblock(int bus);

static struct cvmx_fdt_sfp_info *sfp_list;

/**
 * Local allocator to handle both SE and U-Boot that also zeroes out memory
 *
 * @param	size	number of bytes to allocate
 *
 * @return	pointer to allocated memory or NULL if out of memory.
 *		Alignment is set to 8-bytes.
 */
static void *cvm_sfp_alloc(size_t size)
{
	return calloc(size, 1);
}

/**
 * Free allocated memory.
 *
 * @param	ptr	pointer to memory to free
 *
 * NOTE: This only works in U-Boot since SE does not really have a freeing
 *	 mechanism.  In SE the memory is zeroed out and not freed so this
 *	 is a memory leak if errors occur.
 */
static inline void cvm_sfp_free(void *ptr, size_t size)
{
	free(ptr);
}

/**
 * Select a QSFP device before accessing the EEPROM
 *
 * @param	sfp	handle for sfp/qsfp connector
 * @param	enable	Set true to select, false to deselect
 *
 * @return	0 on success or if SFP or no select GPIO, -1 on GPIO error
 */
static int cvmx_qsfp_select(const struct cvmx_fdt_sfp_info *sfp, bool enable)
{
	/* Select is only needed for QSFP modules */
	if (!sfp->is_qsfp) {
		debug("%s(%s, %d): not QSFP\n", __func__, sfp->name, enable);
		return 0;
	}

	if (dm_gpio_is_valid(&sfp->select)) {
		/* Note that select is active low */
		return dm_gpio_set_value(&sfp->select, !enable);
	}

	debug("%s: select GPIO unknown\n", __func__);
	return 0;
}

static int cvmx_sfp_parse_sfp_buffer(struct cvmx_sfp_mod_info *sfp_info,
				     const uint8_t *buffer)
{
	u8 csum = 0;
	bool csum_good = false;
	int i;

	/* Validate the checksum */
	for (i = 0; i < 0x3f; i++)
		csum += buffer[i];
	csum_good = csum == buffer[0x3f];
	debug("%s: Lower checksum: 0x%02x, expected: 0x%02x\n", __func__, csum,
	      buffer[0x3f]);
	csum = 0;
	for (i = 0x40; i < 0x5f; i++)
		csum += buffer[i];
	debug("%s: Upper checksum: 0x%02x, expected: 0x%02x\n", __func__, csum,
	      buffer[0x5f]);
	if (csum != buffer[0x5f] || !csum_good) {
		debug("Error: SFP EEPROM checksum information is incorrect\n");
		return -1;
	}

	sfp_info->conn_type = buffer[0];
	if (buffer[1] < 1 || buffer[1] > 7) { /* Extended ID */
		debug("Error: Unknown SFP extended identifier 0x%x\n",
		      buffer[1]);
		return -1;
	}
	if (buffer[1] != 4) {
		debug("Module is not SFP/SFP+/SFP28/QSFP+\n");
		return -1;
	}
	sfp_info->mod_type = buffer[2];
	sfp_info->eth_comp = buffer[3] & 0xf0;
	sfp_info->cable_comp = buffer[0x24];

	/* There are several ways a cable can be marked as active or
	 * passive.  8.[2-3] specify the SFP+ cable technology.  Some
	 * modules also use 3.[0-1] for Infiniband, though it's
	 * redundant.
	 */
	if ((buffer[8] & 0x0C) == 0x08) {
		sfp_info->limiting = true;
		sfp_info->active_cable = true;
	} else if ((buffer[8] & 0xC) == 0x4) {
		sfp_info->limiting = false;
		sfp_info->active_cable = false;
	}
	if ((buffer[3] & 3) == 2) {
		sfp_info->active_cable = true;
		sfp_info->limiting = true;
	}

	switch (sfp_info->mod_type) {
	case CVMX_SFP_MOD_OPTICAL_LC:
	case CVMX_SFP_MOD_OPTICAL_PIGTAIL:
		sfp_info->copper_cable = false;
		break;
	case CVMX_SFP_MOD_COPPER_PIGTAIL:
		sfp_info->copper_cable = true;
		break;
	case CVMX_SFP_MOD_NO_SEP_CONN:
		switch (sfp_info->cable_comp) {
		case CVMX_SFP_CABLE_100G_25GAUI_C2M_AOC_HIGH_BER:
		case CVMX_SFP_CABLE_100G_25GAUI_C2M_AOC_LOW_BER:
		case CVMX_SFP_CABLE_100G_25GAUI_C2M_ACC_LOW_BER:
			sfp_info->copper_cable = false;
			sfp_info->limiting = true;
			sfp_info->active_cable = true;
			break;

		case CVMX_SFP_CABLE_100G_SR4_25G_SR:
		case CVMX_SFP_CABLE_100G_LR4_25G_LR:
		case CVMX_SFP_CABLE_100G_ER4_25G_ER:
		case CVMX_SFP_CABLE_100G_SR10:
		case CVMX_SFP_CABLE_100G_CWDM4_MSA:
		case CVMX_SFP_CABLE_100G_PSM4:
		case CVMX_SFP_CABLE_100G_CWDM4:
		case CVMX_SFP_CABLE_40G_ER4:
		case CVMX_SFP_CABLE_4X10G_SR:
		case CVMX_SFP_CABLE_G959_1_P1I1_2D1:
		case CVMX_SFP_CABLE_G959_1_P1S1_2D2:
		case CVMX_SFP_CABLE_G959_1_P1L1_2D2:
		case CVMX_SFP_CABLE_100G_CLR4:
		case CVMX_SFP_CABLE_100G_2_LAMBDA_DWDM:
		case CVMX_SFP_CABLE_40G_SWDM4:
		case CVMX_SFP_CABLE_100G_SWDM4:
		case CVMX_SFP_CABLE_100G_PAM4_BIDI:
			sfp_info->copper_cable = false;
			break;

		case CVMX_SFP_CABLE_100G_25GAUI_C2M_ACC_HIGH_BER:
		case CVMX_SFP_CABLE_10GBASE_T:
		case CVMX_SFP_CABLE_10GBASE_T_SR:
		case CVMX_SFP_CABLE_5GBASE_T:
		case CVMX_SFP_CABLE_2_5GBASE_T:
			sfp_info->copper_cable = true;
			sfp_info->limiting = true;
			sfp_info->active_cable = true;
			break;

		case CVMX_SFP_CABLE_100G_CR4_25G_CR_CA_L:
		case CVMX_SFP_CABLE_25G_CR_CA_S:
		case CVMX_SFP_CABLE_25G_CR_CA_N:
		case CVMX_SFP_CABLE_40G_PSM4:
			sfp_info->copper_cable = true;
			break;

		default:
			switch (sfp_info->eth_comp) {
			case CVMX_SFP_CABLE_10GBASE_ER:
			case CVMX_SFP_CABLE_10GBASE_LRM:
			case CVMX_SFP_CABLE_10GBASE_LR:
			case CVMX_SFP_CABLE_10GBASE_SR:
				sfp_info->copper_cable = false;
				break;
			}
			break;
		}
		break;

	case CVMX_SFP_MOD_RJ45:
		debug("%s: RJ45 adapter\n", __func__);
		sfp_info->copper_cable = true;
		sfp_info->active_cable = true;
		sfp_info->limiting = true;
		break;
	case CVMX_SFP_MOD_UNKNOWN:
		/* The Avago 1000Base-X to 1000Base-T module reports that it
		 * is an unknown module type but the Ethernet compliance code
		 * says it is 1000Base-T.  We'll change the reporting to RJ45.
		 */
		if (buffer[6] & 8) {
			debug("RJ45 gigabit module detected\n");
			sfp_info->mod_type = CVMX_SFP_MOD_RJ45;
			sfp_info->copper_cable = false;
			sfp_info->limiting = true;
			sfp_info->active_cable = true;
			sfp_info->max_copper_cable_len = buffer[0x12];
			sfp_info->rate = CVMX_SFP_RATE_1G;
		} else {
			debug("Unknown module type 0x%x\n", sfp_info->mod_type);
		}
		sfp_info->limiting = true;
		break;
	case CVMX_SFP_MOD_MXC_2X16:
		debug("%s: MXC 2X16\n", __func__);
		break;
	default:
		sfp_info->limiting = true;
		break;
	}

	if (sfp_info->copper_cable)
		sfp_info->max_copper_cable_len = buffer[0x12];
	else
		sfp_info->max_50um_om4_cable_length = buffer[0x12] * 10;

	if (buffer[0xe])
		sfp_info->max_single_mode_cable_length = buffer[0xe] * 1000;
	else
		sfp_info->max_single_mode_cable_length = buffer[0xf] * 100000;

	sfp_info->max_50um_om2_cable_length = buffer[0x10] * 10;
	sfp_info->max_62_5um_om1_cable_length = buffer[0x11] * 10;
	sfp_info->max_50um_om3_cable_length = buffer[0x13] * 10;

	if (buffer[0xc] == 0xff) {
		if (buffer[0x42] >= 255)
			sfp_info->rate = CVMX_SFP_RATE_100G;
		else if (buffer[0x42] >= 160)
			sfp_info->rate = CVMX_SFP_RATE_40G;
		else if (buffer[0x42] >= 100)
			sfp_info->rate = CVMX_SFP_RATE_25G;
		else
			sfp_info->rate = CVMX_SFP_RATE_UNKNOWN;
	} else if (buffer[0xc] >= 100) {
		sfp_info->rate = CVMX_SFP_RATE_10G;
	} else if (buffer[0xc] >= 10) {
		sfp_info->rate = CVMX_SFP_RATE_1G;
	} else {
		sfp_info->rate = CVMX_SFP_RATE_UNKNOWN;
	}

	if (sfp_info->rate == CVMX_SFP_RATE_UNKNOWN) {
		switch (sfp_info->cable_comp) {
		case CVMX_SFP_CABLE_100G_SR10:
		case CVMX_SFP_CABLE_100G_CWDM4_MSA:
		case CVMX_SFP_CABLE_100G_PSM4:
		case CVMX_SFP_CABLE_100G_CWDM4:
		case CVMX_SFP_CABLE_100G_CLR4:
		case CVMX_SFP_CABLE_100G_2_LAMBDA_DWDM:
		case CVMX_SFP_CABLE_100G_SWDM4:
		case CVMX_SFP_CABLE_100G_PAM4_BIDI:
			sfp_info->rate = CVMX_SFP_RATE_100G;
			break;
		case CVMX_SFP_CABLE_100G_25GAUI_C2M_AOC_HIGH_BER:
		case CVMX_SFP_CABLE_100G_SR4_25G_SR:
		case CVMX_SFP_CABLE_100G_LR4_25G_LR:
		case CVMX_SFP_CABLE_100G_ER4_25G_ER:
		case CVMX_SFP_CABLE_100G_25GAUI_C2M_ACC_HIGH_BER:
		case CVMX_SFP_CABLE_100G_CR4_25G_CR_CA_L:
		case CVMX_SFP_CABLE_25G_CR_CA_S:
		case CVMX_SFP_CABLE_25G_CR_CA_N:
		case CVMX_SFP_CABLE_100G_25GAUI_C2M_AOC_LOW_BER:
		case CVMX_SFP_CABLE_100G_25GAUI_C2M_ACC_LOW_BER:
			sfp_info->rate = CVMX_SFP_RATE_25G;
			break;
		case CVMX_SFP_CABLE_40G_ER4:
		case CVMX_SFP_CABLE_4X10G_SR:
		case CVMX_SFP_CABLE_40G_PSM4:
		case CVMX_SFP_CABLE_40G_SWDM4:
			sfp_info->rate = CVMX_SFP_RATE_40G;
			break;
		case CVMX_SFP_CABLE_G959_1_P1I1_2D1:
		case CVMX_SFP_CABLE_G959_1_P1S1_2D2:
		case CVMX_SFP_CABLE_G959_1_P1L1_2D2:
		case CVMX_SFP_CABLE_10GBASE_T:
		case CVMX_SFP_CABLE_10GBASE_T_SR:
		case CVMX_SFP_CABLE_5GBASE_T:
		case CVMX_SFP_CABLE_2_5GBASE_T:
			sfp_info->rate = CVMX_SFP_RATE_10G;
			break;
		default:
			switch (sfp_info->eth_comp) {
			case CVMX_SFP_CABLE_10GBASE_ER:
			case CVMX_SFP_CABLE_10GBASE_LRM:
			case CVMX_SFP_CABLE_10GBASE_LR:
			case CVMX_SFP_CABLE_10GBASE_SR:
				sfp_info->rate = CVMX_SFP_RATE_10G;
				break;
			default:
				sfp_info->rate = CVMX_SFP_RATE_UNKNOWN;
				break;
			}
			break;
		}
	}

	if (buffer[0xc] < 0xff)
		sfp_info->bitrate_max = buffer[0xc] * 100;
	else
		sfp_info->bitrate_max = buffer[0x42] * 250;

	if ((buffer[8] & 0xc) == 8) {
		if (buffer[0x3c] & 0x4)
			sfp_info->limiting = true;
	}

	/* Currently we only set this for 25G.  FEC is required for CA-S cables
	 * and for cable lengths >= 5M as of this writing.
	 */
	if ((sfp_info->rate == CVMX_SFP_RATE_25G &&
	     sfp_info->copper_cable) &&
	    (sfp_info->cable_comp == CVMX_SFP_CABLE_25G_CR_CA_S ||
	     sfp_info->max_copper_cable_len >= 5))
		sfp_info->fec_required = true;

	/* copy strings and vendor info, strings will be automatically NUL
	 * terminated.
	 */
	memcpy(sfp_info->vendor_name, &buffer[0x14], 16);
	memcpy(sfp_info->vendor_oui, &buffer[0x25], 3);
	memcpy(sfp_info->vendor_pn, &buffer[0x28], 16);
	memcpy(sfp_info->vendor_rev, &buffer[0x38], 4);
	memcpy(sfp_info->vendor_sn, &buffer[0x44], 16);
	memcpy(sfp_info->date_code, &buffer[0x54], 8);

	sfp_info->cooled_laser = !!(buffer[0x40] & 4);
	sfp_info->internal_cdr = !!(buffer[0x40] & 8);

	if (buffer[0x40] & 0x20)
		sfp_info->power_level = 3;
	else
		sfp_info->power_level = (buffer[0x40] & 2) ? 2 : 1;

	sfp_info->diag_paging = !!(buffer[0x40] & 0x10);
	sfp_info->linear_rx_output = !(buffer[0x40] & 1);
	sfp_info->los_implemented = !!(buffer[0x41] & 2);
	sfp_info->los_inverted = !!(buffer[0x41] & 4);
	sfp_info->tx_fault_implemented = !!(buffer[0x41] & 8);
	sfp_info->tx_disable_implemented = !!(buffer[0x41] & 0x10);
	sfp_info->rate_select_implemented = !!(buffer[0x41] & 0x20);
	sfp_info->tuneable_transmitter = !!(buffer[0x41] & 0x40);
	sfp_info->rx_decision_threshold_implemented = !!(buffer[0x41] & 0x80);

	sfp_info->diag_monitoring = !!(buffer[0x5c] & 0x40);
	sfp_info->diag_rx_power_averaged = !!(buffer[0x5c] & 0x8);
	sfp_info->diag_externally_calibrated = !!(buffer[0x5c] & 0x10);
	sfp_info->diag_internally_calibrated = !!(buffer[0x5c] & 0x20);
	sfp_info->diag_addr_change_required = !!(buffer[0x5c] & 0x4);
	sfp_info->diag_soft_rate_select_control = !!(buffer[0x5d] & 2);
	sfp_info->diag_app_select_control = !!(buffer[0x5d] & 4);
	sfp_info->diag_soft_rate_select_control = !!(buffer[0x5d] & 8);
	sfp_info->diag_soft_rx_los_implemented = !!(buffer[0x5d] & 0x10);
	sfp_info->diag_soft_tx_fault_implemented = !!(buffer[0x5d] & 0x20);
	sfp_info->diag_soft_tx_disable_implemented = !!(buffer[0x5d] & 0x40);
	sfp_info->diag_alarm_warning_flags_implemented =
		!!(buffer[0x5d] & 0x80);
	sfp_info->diag_rev = buffer[0x5e];

	return 0;
}

static int cvmx_sfp_parse_qsfp_buffer(struct cvmx_sfp_mod_info *sfp_info,
				      const uint8_t *buffer)
{
	u8 csum = 0;
	bool csum_good = false;
	int i;

	/* Validate the checksum */
	for (i = 0x80; i < 0xbf; i++)
		csum += buffer[i];
	csum_good = csum == buffer[0xbf];
	debug("%s: Lower checksum: 0x%02x, expected: 0x%02x\n", __func__, csum,
	      buffer[0xbf]);
	csum = 0;
	for (i = 0xc0; i < 0xdf; i++)
		csum += buffer[i];
	debug("%s: Upper checksum: 0x%02x, expected: 0x%02x\n", __func__, csum,
	      buffer[0xdf]);
	if (csum != buffer[0xdf] || !csum_good) {
		debug("Error: SFP EEPROM checksum information is incorrect\n");
		return -1;
	}

	sfp_info->conn_type = buffer[0x80];
	sfp_info->mod_type = buffer[0x82];
	sfp_info->eth_comp = buffer[0x83] & 0xf0;
	sfp_info->cable_comp = buffer[0xa4];

	switch (sfp_info->mod_type) {
	case CVMX_SFP_MOD_COPPER_PIGTAIL:
	case CVMX_SFP_MOD_NO_SEP_CONN:
		debug("%s: copper pigtail or no separable cable\n", __func__);
		/* There are several ways a cable can be marked as active or
		 * passive.  8.[2-3] specify the SFP+ cable technology.  Some
		 * modules also use 3.[0-1] for Infiniband, though it's
		 * redundant.
		 */
		sfp_info->copper_cable = true;
		if ((buffer[0x88] & 0x0C) == 0x08) {
			sfp_info->limiting = true;
			sfp_info->active_cable = true;
		} else if ((buffer[0x88] & 0xC) == 0x4) {
			sfp_info->limiting = false;
			sfp_info->active_cable = false;
		}
		if ((buffer[0x83] & 3) == 2) {
			sfp_info->active_cable = true;
			sfp_info->limiting = true;
		}
		break;
	case CVMX_SFP_MOD_RJ45:
		debug("%s: RJ45 adapter\n", __func__);
		sfp_info->copper_cable = true;
		sfp_info->active_cable = true;
		sfp_info->limiting = true;
		break;
	case CVMX_SFP_MOD_UNKNOWN:
		debug("Unknown module type\n");
		/* The Avago 1000Base-X to 1000Base-T module reports that it
		 * is an unknown module type but the Ethernet compliance code
		 * says it is 1000Base-T.  We'll change the reporting to RJ45.
		 */
		if (buffer[0x86] & 8) {
			sfp_info->mod_type = CVMX_SFP_MOD_RJ45;
			sfp_info->copper_cable = false;
			sfp_info->limiting = true;
			sfp_info->active_cable = true;
			sfp_info->max_copper_cable_len = buffer[0x92];
			sfp_info->rate = CVMX_SFP_RATE_1G;
		}
		fallthrough;
	default:
		sfp_info->limiting = true;
		break;
	}

	if (sfp_info->copper_cable)
		sfp_info->max_copper_cable_len = buffer[0x92];
	else
		sfp_info->max_50um_om4_cable_length = buffer[0x92] * 10;

	debug("%s: copper cable: %d, max copper cable len: %d\n", __func__,
	      sfp_info->copper_cable, sfp_info->max_copper_cable_len);
	if (buffer[0xe])
		sfp_info->max_single_mode_cable_length = buffer[0x8e] * 1000;
	else
		sfp_info->max_single_mode_cable_length = buffer[0x8f] * 100000;

	sfp_info->max_50um_om2_cable_length = buffer[0x90] * 10;
	sfp_info->max_62_5um_om1_cable_length = buffer[0x91] * 10;
	sfp_info->max_50um_om3_cable_length = buffer[0x93] * 10;

	if (buffer[0x8c] == 12) {
		sfp_info->rate = CVMX_SFP_RATE_1G;
	} else if (buffer[0x8c] == 103) {
		sfp_info->rate = CVMX_SFP_RATE_10G;
	} else if (buffer[0x8c] == 0xff) {
		if (buffer[0xc2] == 103)
			sfp_info->rate = CVMX_SFP_RATE_100G;
	}

	if (buffer[0x8c] < 0xff)
		sfp_info->bitrate_max = buffer[0x8c] * 100;
	else
		sfp_info->bitrate_max = buffer[0xc2] * 250;

	if ((buffer[0x88] & 0xc) == 8) {
		if (buffer[0xbc] & 0x4)
			sfp_info->limiting = true;
	}

	/* Currently we only set this for 25G.  FEC is required for CA-S cables
	 * and for cable lengths >= 5M as of this writing.
	 */
	/* copy strings and vendor info, strings will be automatically NUL
	 * terminated.
	 */
	memcpy(sfp_info->vendor_name, &buffer[0x94], 16);
	memcpy(sfp_info->vendor_oui, &buffer[0xa5], 3);
	memcpy(sfp_info->vendor_pn, &buffer[0xa8], 16);
	memcpy(sfp_info->vendor_rev, &buffer[0xb8], 4);
	memcpy(sfp_info->vendor_sn, &buffer[0xc4], 16);
	memcpy(sfp_info->date_code, &buffer[0xd4], 8);

	sfp_info->linear_rx_output = !!(buffer[0xc0] & 1);
	sfp_info->cooled_laser = !!(buffer[0xc0] & 4);
	sfp_info->internal_cdr = !!(buffer[0xc0] & 8);

	if (buffer[0xc0] & 0x20)
		sfp_info->power_level = 3;
	else
		sfp_info->power_level = (buffer[0xc0] & 2) ? 2 : 1;

	sfp_info->diag_paging = !!(buffer[0xc0] & 0x10);
	sfp_info->los_implemented = !!(buffer[0xc1] & 2);
	sfp_info->los_inverted = !!(buffer[0xc1] & 4);
	sfp_info->tx_fault_implemented = !!(buffer[0xc1] & 8);
	sfp_info->tx_disable_implemented = !!(buffer[0xc1] & 0x10);
	sfp_info->rate_select_implemented = !!(buffer[0xc1] & 0x20);
	sfp_info->tuneable_transmitter = !!(buffer[0xc1] & 0x40);
	sfp_info->rx_decision_threshold_implemented = !!(buffer[0xc1] & 0x80);

	sfp_info->diag_monitoring = !!(buffer[0xdc] & 0x40);
	sfp_info->diag_rx_power_averaged = !!(buffer[0xdc] & 0x8);
	sfp_info->diag_externally_calibrated = !!(buffer[0xdc] & 0x10);
	sfp_info->diag_internally_calibrated = !!(buffer[0xdc] & 0x20);
	sfp_info->diag_addr_change_required = !!(buffer[0xdc] & 0x4);
	sfp_info->diag_soft_rate_select_control = !!(buffer[0xdd] & 2);
	sfp_info->diag_app_select_control = !!(buffer[0xdd] & 4);
	sfp_info->diag_soft_rate_select_control = !!(buffer[0xdd] & 8);
	sfp_info->diag_soft_rx_los_implemented = !!(buffer[0xdd] & 0x10);
	sfp_info->diag_soft_tx_fault_implemented = !!(buffer[0xdd] & 0x20);
	sfp_info->diag_soft_tx_disable_implemented = !!(buffer[0xdd] & 0x40);
	sfp_info->diag_alarm_warning_flags_implemented =
		!!(buffer[0xdd] & 0x80);
	sfp_info->diag_rev = buffer[0xde];

	return 0;
}

static bool sfp_verify_checksum(const uint8_t *buffer)
{
	u8 csum = 0;
	u8 offset;
	bool csum_good = false;
	int i;

	switch (buffer[0]) {
	case CVMX_SFP_CONN_QSFP:
	case CVMX_SFP_CONN_QSFPP:
	case CVMX_SFP_CONN_QSFP28:
	case CVMX_SFP_CONN_MICRO_QSFP:
	case CVMX_SFP_CONN_QSFP_DD:
		offset = 0x80;
		break;
	default:
		offset = 0;
		break;
	}
	for (i = offset; i < offset + 0x3f; i++)
		csum += buffer[i];
	csum_good = csum == buffer[offset + 0x3f];
	if (!csum_good) {
		debug("%s: Lower checksum bad, got 0x%x, expected 0x%x\n",
		      __func__, csum, buffer[offset + 0x3f]);
		return false;
	}
	csum = 0;
	for (i = offset + 0x40; i < offset + 0x5f; i++)
		csum += buffer[i];
	if (csum != buffer[offset + 0x5f]) {
		debug("%s: Upper checksum bad, got 0x%x, expected 0x%x\n",
		      __func__, csum, buffer[offset + 0x5f]);
		return false;
	}
	return true;
}

/**
 * Reads and parses SFP/QSFP EEPROM
 *
 * @param	sfp	sfp handle to read
 *
 * @return	0 for success, -1 on error.
 */
int cvmx_sfp_read_i2c_eeprom(struct cvmx_fdt_sfp_info *sfp)
{
	const struct cvmx_fdt_i2c_bus_info *bus = sfp->i2c_bus;
	int oct_bus = cvmx_fdt_i2c_get_root_bus(bus);
	struct udevice *dev;
	u8 buffer[256];
	bool is_qsfp;
	int retry;
	int err;

	if (!bus) {
		debug("%s(%s): Error: i2c bus undefined for eeprom\n", __func__,
		      sfp->name);
		return -1;
	}

	is_qsfp = (sfp->sfp_info.conn_type == CVMX_SFP_CONN_QSFP ||
		   sfp->sfp_info.conn_type == CVMX_SFP_CONN_QSFPP ||
		   sfp->sfp_info.conn_type == CVMX_SFP_CONN_QSFP28 ||
		   sfp->sfp_info.conn_type == CVMX_SFP_CONN_MICRO_QSFP) ||
		  sfp->is_qsfp;

	err = cvmx_qsfp_select(sfp, true);
	if (err) {
		debug("%s: Error selecting SFP/QSFP slot\n", __func__);
		return err;
	}

	debug("%s: Reading eeprom from i2c address %d:0x%x\n", __func__,
	      oct_bus, sfp->i2c_eeprom_addr);
	for (retry = 0; retry < 3; retry++) {
		err = i2c_get_chip(bus->i2c_bus, sfp->i2c_eeprom_addr, 1, &dev);
		if (err) {
			debug("Cannot find I2C device: %d\n", err);
			goto error;
		}

		err = dm_i2c_read(dev, 0, buffer, 256);
		if (err || !sfp_verify_checksum(buffer)) {
			debug("%s: Error %d reading eeprom at 0x%x, bus %d\n",
			      __func__, err, sfp->i2c_eeprom_addr, oct_bus);
			debug("%s: Retry %d\n", __func__, retry + 1);
			mdelay(1000);
		} else {
			break;
		}
	}
	if (err) {
		debug("%s: Error reading eeprom from SFP %s\n", __func__,
		      sfp->name);
		return -1;
	}
#ifdef DEBUG
	print_buffer(0, buffer, 1, 256, 0);
#endif
	memset(&sfp->sfp_info, 0, sizeof(struct cvmx_sfp_mod_info));

	switch (buffer[0]) {
	case CVMX_SFP_CONN_SFP:
		err = cvmx_sfp_parse_sfp_buffer(&sfp->sfp_info, buffer);
		break;
	case CVMX_SFP_CONN_QSFP:
	case CVMX_SFP_CONN_QSFPP:
	case CVMX_SFP_CONN_QSFP28:
	case CVMX_SFP_CONN_MICRO_QSFP:
		err = cvmx_sfp_parse_qsfp_buffer(&sfp->sfp_info, buffer);
		break;
	default:
		debug("%s: Unknown SFP transceiver type 0x%x\n", __func__,
		      buffer[0]);
		err = -1;
		break;
	}

error:
	if (is_qsfp)
		err |= cvmx_qsfp_select(sfp, false);

	if (!err) {
		sfp->valid = true;
		sfp->sfp_info.valid = true;
	} else {
		sfp->valid = false;
		sfp->sfp_info.valid = false;
	}

	return err;
}

/**
 * Function called to check and return the status of the mod_abs pin or
 * mod_pres pin for QSFPs.
 *
 * @param	sfp	Handle to SFP information.
 * @param	data	User-defined data passed to the function
 *
 * @return	0 if absent, 1 if present, -1 on error
 */
int cvmx_sfp_check_mod_abs(struct cvmx_fdt_sfp_info *sfp, void *data)
{
	int val;
	int err = 0;
	int mode;

	if (!dm_gpio_is_valid(&sfp->mod_abs)) {
		debug("%s: Error: mod_abs not set for %s\n", __func__,
		      sfp->name);
		return -1;
	}
	val = dm_gpio_get_value(&sfp->mod_abs);
	debug("%s(%s, %p) mod_abs: %d\n", __func__, sfp->name, data, val);
	if (val >= 0 && val != sfp->last_mod_abs && sfp->mod_abs_changed) {
		err = 0;
		if (!val) {
			err = cvmx_sfp_read_i2c_eeprom(sfp);
			if (err)
				debug("%s: Error reading SFP %s EEPROM\n",
				      __func__, sfp->name);
		}
		err = sfp->mod_abs_changed(sfp, val, sfp->mod_abs_changed_data);
	}
	debug("%s(%s (%p)): Last mod_abs: %d, current: %d, changed: %p, rc: %d, next: %p, caller: %p\n",
	      __func__, sfp->name, sfp, sfp->last_mod_abs, val,
	      sfp->mod_abs_changed, err, sfp->next_iface_sfp,
	      __builtin_return_address(0));

	if (err >= 0) {
		sfp->last_mod_abs = val;
		mode = cvmx_helper_interface_get_mode(sfp->xiface);
		cvmx_sfp_validate_module(sfp, mode);
	} else {
		debug("%s: mod_abs_changed for %s returned error\n", __func__,
		      sfp->name);
	}

	return err < 0 ? err : val;
}

/**
 * Reads the EEPROMs of all SFP modules.
 *
 * @return 0 for success
 */
int cvmx_sfp_read_all_modules(void)
{
	struct cvmx_fdt_sfp_info *sfp;
	int val;
	bool error = false;
	int rc;

	for (sfp = sfp_list; sfp; sfp = sfp->next) {
		if (dm_gpio_is_valid(&sfp->mod_abs)) {
			/* Check if module absent */
			val = dm_gpio_get_value(&sfp->mod_abs);
			sfp->last_mod_abs = val;
			if (val)
				continue;
		}
		rc = cvmx_sfp_read_i2c_eeprom(sfp);
		if (rc) {
			debug("%s: Error reading eeprom from SFP %s\n",
			      __func__, sfp->name);
			error = true;
		}
	}

	return error ? -1 : 0;
}

/**
 * Registers a function to be called whenever the mod_abs/mod_pres signal
 * changes.
 *
 * @param	sfp		Handle to SFP data structure
 * @param	mod_abs_changed	Function called whenever mod_abs is changed
 *				or NULL to remove.
 * @param	mod_abs_changed_data	User-defined data passed to
 *					mod_abs_changed
 *
 * @return	0 for success
 *
 * @NOTE: If multiple SFP slots are linked together, all subsequent slots
 *	  will also be registered for the same handler.
 */
int cvmx_sfp_register_mod_abs_changed(struct cvmx_fdt_sfp_info *sfp,
				      int (*mod_abs_changed)(struct cvmx_fdt_sfp_info *sfp,
							     int val, void *data),
				      void *mod_abs_changed_data)
{
	sfp->mod_abs_changed = mod_abs_changed;
	sfp->mod_abs_changed_data = mod_abs_changed_data;

	sfp->last_mod_abs = -2; /* undefined */

	return 0;
}

/**
 * Parses a SFP slot from the device tree
 *
 * @param	sfp		SFP handle to store data in
 * @param	fdt_addr	Address of flat device tree
 * @param	of_offset	Node in device tree for SFP slot
 *
 * @return	0 on success, -1 on error
 */
static int cvmx_sfp_parse_sfp(struct cvmx_fdt_sfp_info *sfp, ofnode node)
{
	struct ofnode_phandle_args phandle;
	int err;

	sfp->name = ofnode_get_name(node);
	sfp->of_offset = ofnode_to_offset(node);

	err = gpio_request_by_name_nodev(node, "tx_disable", 0,
					 &sfp->tx_disable, GPIOD_IS_OUT);
	if (err) {
		printf("%s: tx_disable not found in DT!\n", __func__);
		return -ENODEV;
	}
	dm_gpio_set_value(&sfp->tx_disable, 0);

	err = gpio_request_by_name_nodev(node, "mod_abs", 0,
					 &sfp->mod_abs, GPIOD_IS_IN);
	if (err) {
		printf("%s: mod_abs not found in DT!\n", __func__);
		return -ENODEV;
	}

	err = gpio_request_by_name_nodev(node, "tx_error", 0,
					 &sfp->tx_error, GPIOD_IS_IN);
	if (err) {
		printf("%s: tx_error not found in DT!\n", __func__);
		return -ENODEV;
	}

	err = gpio_request_by_name_nodev(node, "rx_los", 0,
					 &sfp->rx_los, GPIOD_IS_IN);
	if (err) {
		printf("%s: rx_los not found in DT!\n", __func__);
		return -ENODEV;
	}

	err = ofnode_parse_phandle_with_args(node, "eeprom", NULL, 0, 0,
					     &phandle);
	if (!err) {
		sfp->i2c_eeprom_addr = ofnode_get_addr(phandle.node);
		debug("%s: eeprom address: 0x%x\n", __func__,
		      sfp->i2c_eeprom_addr);

		debug("%s: Getting eeprom i2c bus for %s\n", __func__,
		      sfp->name);
		sfp->i2c_bus = cvmx_ofnode_get_i2c_bus(ofnode_get_parent(phandle.node));
	}

	err = ofnode_parse_phandle_with_args(node, "diag", NULL, 0, 0,
					     &phandle);
	if (!err) {
		sfp->i2c_diag_addr = ofnode_get_addr(phandle.node);
		if (!sfp->i2c_bus)
			sfp->i2c_bus = cvmx_ofnode_get_i2c_bus(ofnode_get_parent(phandle.node));
	}

	sfp->last_mod_abs = -2;
	sfp->last_rx_los = -2;

	if (!sfp->i2c_bus) {
		debug("%s(%s): Error: could not get i2c bus from device tree\n",
		      __func__, sfp->name);
		err = -1;
	}

	if (err) {
		dm_gpio_free(sfp->tx_disable.dev, &sfp->tx_disable);
		dm_gpio_free(sfp->mod_abs.dev, &sfp->mod_abs);
		dm_gpio_free(sfp->tx_error.dev, &sfp->tx_error);
		dm_gpio_free(sfp->rx_los.dev, &sfp->rx_los);
	} else {
		sfp->valid = true;
	}

	return err;
}

/**
 * Parses a QSFP slot from the device tree
 *
 * @param	sfp		SFP handle to store data in
 * @param	fdt_addr	Address of flat device tree
 * @param	of_offset	Node in device tree for SFP slot
 *
 * @return	0 on success, -1 on error
 */
static int cvmx_sfp_parse_qsfp(struct cvmx_fdt_sfp_info *sfp, ofnode node)
{
	struct ofnode_phandle_args phandle;
	int err;

	sfp->is_qsfp = true;
	sfp->name = ofnode_get_name(node);
	sfp->of_offset = ofnode_to_offset(node);

	err = gpio_request_by_name_nodev(node, "lp_mode", 0,
					 &sfp->lp_mode, GPIOD_IS_OUT);
	if (err) {
		printf("%s: lp_mode not found in DT!\n", __func__);
		return -ENODEV;
	}

	err = gpio_request_by_name_nodev(node, "mod_prs", 0,
					 &sfp->mod_abs, GPIOD_IS_IN);
	if (err) {
		printf("%s: mod_prs not found in DT!\n", __func__);
		return -ENODEV;
	}

	err = gpio_request_by_name_nodev(node, "select", 0,
					 &sfp->select, GPIOD_IS_IN);
	if (err) {
		printf("%s: select not found in DT!\n", __func__);
		return -ENODEV;
	}

	err = gpio_request_by_name_nodev(node, "reset", 0,
					 &sfp->reset, GPIOD_IS_OUT);
	if (err) {
		printf("%s: reset not found in DT!\n", __func__);
		return -ENODEV;
	}

	err = gpio_request_by_name_nodev(node, "interrupt", 0,
					 &sfp->interrupt, GPIOD_IS_IN);
	if (err) {
		printf("%s: interrupt not found in DT!\n", __func__);
		return -ENODEV;
	}

	err = ofnode_parse_phandle_with_args(node, "eeprom", NULL, 0, 0,
					     &phandle);
	if (!err) {
		sfp->i2c_eeprom_addr = ofnode_get_addr(phandle.node);
		sfp->i2c_bus = cvmx_ofnode_get_i2c_bus(ofnode_get_parent(phandle.node));
	}

	err = ofnode_parse_phandle_with_args(node, "diag", NULL, 0, 0,
					     &phandle);
	if (!err) {
		sfp->i2c_diag_addr = ofnode_get_addr(phandle.node);
		if (!sfp->i2c_bus)
			sfp->i2c_bus = cvmx_ofnode_get_i2c_bus(ofnode_get_parent(phandle.node));
	}

	sfp->last_mod_abs = -2;
	sfp->last_rx_los = -2;

	if (!sfp->i2c_bus) {
		cvmx_printf("%s(%s): Error: could not get i2c bus from device tree\n",
			    __func__, sfp->name);
		err = -1;
	}

	if (err) {
		dm_gpio_free(sfp->lp_mode.dev, &sfp->lp_mode);
		dm_gpio_free(sfp->mod_abs.dev, &sfp->mod_abs);
		dm_gpio_free(sfp->select.dev, &sfp->select);
		dm_gpio_free(sfp->reset.dev, &sfp->reset);
		dm_gpio_free(sfp->interrupt.dev, &sfp->interrupt);
	} else {
		sfp->valid = true;
	}

	return err;
}

/**
 * Parses the device tree for SFP and QSFP slots
 *
 * @param	fdt_addr	Address of flat device-tree
 *
 * @return	0 for success, -1 on error
 */
int cvmx_sfp_parse_device_tree(const void *fdt_addr)
{
	struct cvmx_fdt_sfp_info *sfp, *first_sfp = NULL, *last_sfp = NULL;
	ofnode node;
	int err = 0;
	int reg;
	static bool parsed;

	debug("%s(%p): Parsing...\n", __func__, fdt_addr);
	if (parsed) {
		debug("%s(%p): Already parsed\n", __func__, fdt_addr);
		return 0;
	}

	ofnode_for_each_compatible_node(node, "ethernet,sfp-slot") {
		if (!ofnode_valid(node))
			continue;

		sfp = cvm_sfp_alloc(sizeof(*sfp));
		if (!sfp)
			return -1;

		err = cvmx_sfp_parse_sfp(sfp, node);
		if (!err) {
			if (!sfp_list)
				sfp_list = sfp;
			if (last_sfp)
				last_sfp->next = sfp;
			sfp->prev = last_sfp;
			last_sfp = sfp;
			debug("%s: parsed %s\n", __func__, sfp->name);
		} else {
			debug("%s: Error parsing SFP at node %s\n",
			      __func__, ofnode_get_name(node));
			return err;
		}
	}

	ofnode_for_each_compatible_node(node, "ethernet,qsfp-slot") {
		if (!ofnode_valid(node))
			continue;

		sfp = cvm_sfp_alloc(sizeof(*sfp));
		if (!sfp)
			return -1;

		err = cvmx_sfp_parse_qsfp(sfp, node);
		if (!err) {
			if (!sfp_list)
				sfp_list = sfp;
			if (last_sfp)
				last_sfp->next = sfp;
			sfp->prev = last_sfp;
			last_sfp = sfp;
			debug("%s: parsed %s\n", __func__, sfp->name);
		} else {
			debug("%s: Error parsing QSFP at node %s\n",
			      __func__, ofnode_get_name(node));
			return err;
		}
	}

	if (!octeon_has_feature(OCTEON_FEATURE_BGX))
		return 0;

	err = 0;
	ofnode_for_each_compatible_node(node, "cavium,octeon-7890-bgx-port") {
		int sfp_nodes[4];
		ofnode sfp_ofnodes[4];
		int num_sfp_nodes;
		u64 reg_addr;
		struct cvmx_xiface xi;
		int xiface, index;
		cvmx_helper_interface_mode_t mode;
		int i;
		int rc;

		if (!ofnode_valid(node))
			break;

		num_sfp_nodes = ARRAY_SIZE(sfp_nodes);
		rc = cvmx_ofnode_lookup_phandles(node, "sfp-slot",
						 &num_sfp_nodes, sfp_ofnodes);
		if (rc != 0 || num_sfp_nodes < 1)
			rc = cvmx_ofnode_lookup_phandles(node, "qsfp-slot",
							 &num_sfp_nodes,
							 sfp_ofnodes);
		/* If no SFP or QSFP slot found, go to next port */
		if (rc < 0)
			continue;

		last_sfp = NULL;
		for (i = 0; i < num_sfp_nodes; i++) {
			sfp = cvmx_sfp_find_slot_by_fdt_node(ofnode_to_offset(sfp_ofnodes[i]));
			debug("%s: Adding sfp %s (%p) to BGX port\n",
			      __func__, sfp->name, sfp);
			if (last_sfp)
				last_sfp->next_iface_sfp = sfp;
			else
				first_sfp = sfp;
			last_sfp = sfp;
		}
		if (!first_sfp) {
			debug("%s: Error: could not find SFP slot for BGX port %s\n",
			      __func__,
			      fdt_get_name(fdt_addr, sfp_nodes[0],
					   NULL));
			err = -1;
			break;
		}

		/* Get the port index */
		reg = ofnode_get_addr(node);
		if (reg < 0) {
			debug("%s: Error: could not get BGX port reg value\n",
			      __func__);
			err = -1;
			break;
		}
		index = reg;

		/* Get BGX node and address */
		reg_addr = ofnode_get_addr(ofnode_get_parent(node));
		/* Extrace node */
		xi.node = cvmx_csr_addr_to_node(reg_addr);
		/* Extract reg address */
		reg_addr = cvmx_csr_addr_strip_node(reg_addr);
		if ((reg_addr & 0xFFFFFFFFF0000000) !=
		    0x00011800E0000000) {
			debug("%s: Invalid BGX address 0x%llx\n",
			      __func__, (unsigned long long)reg_addr);
			xi.node = -1;
			err = -1;
			break;
		}

		/* Extract interface from address */
		xi.interface = (reg_addr >> 24) & 0x0F;
		/* Convert to xiface */
		xiface = cvmx_helper_node_interface_to_xiface(xi.node,
							      xi.interface);
		debug("%s: Parsed %d SFP slots for interface 0x%x, index %d\n",
		      __func__, num_sfp_nodes, xiface, index);

		mode = cvmx_helper_interface_get_mode(xiface);
		for (sfp = first_sfp; sfp; sfp = sfp->next_iface_sfp) {
			sfp->xiface = xiface;
			sfp->index = index;
			/* Convert to IPD port */
			sfp->ipd_port[0] =
				cvmx_helper_get_ipd_port(xiface, index);
			debug("%s: sfp %s (%p) xi: 0x%x, index: 0x%x, node: %d, mode: 0x%x, next: %p\n",
			      __func__, sfp->name, sfp, sfp->xiface,
			      sfp->index, xi.node, mode,
			      sfp->next_iface_sfp);
			if (mode == CVMX_HELPER_INTERFACE_MODE_XLAUI ||
			    mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4)
				for (i = 1; i < 4; i++)
					sfp->ipd_port[i] = -1;
			else
				for (i = 1; i < 4; i++)
					sfp->ipd_port[i] =
						cvmx_helper_get_ipd_port(
							xiface, i);
		}
		cvmx_helper_cfg_set_sfp_info(xiface, index, first_sfp);
	}

	if (!err) {
		parsed = true;
		cvmx_sfp_read_all_modules();
	}

	return err;
}

/**
 * Given a fdt node offset find the corresponding SFP or QSFP slot
 *
 * @param	of_offset	flat device tree node offset
 *
 * @return	pointer to SFP data structure or NULL if not found
 */
struct cvmx_fdt_sfp_info *cvmx_sfp_find_slot_by_fdt_node(int of_offset)
{
	struct cvmx_fdt_sfp_info *sfp = sfp_list;

	while (sfp) {
		if (sfp->of_offset == of_offset)
			return sfp;
		sfp = sfp->next;
	}
	return NULL;
}

static bool cvmx_sfp_validate_quad(struct cvmx_fdt_sfp_info *sfp,
				   struct cvmx_phy_gpio_leds *leds)
{
	bool multi_led = leds && (leds->next);
	bool error = false;
	int mod_abs;

	do {
		/* Skip missing modules */
		if (dm_gpio_is_valid(&sfp->mod_abs))
			mod_abs = dm_gpio_get_value(&sfp->mod_abs);
		else
			mod_abs = 0;
		if (!mod_abs) {
			if (cvmx_sfp_read_i2c_eeprom(sfp)) {
				debug("%s: Error reading eeprom for %s\n",
				      __func__, sfp->name);
			}
			if (sfp->sfp_info.rate < CVMX_SFP_RATE_10G) {
				cvmx_helper_leds_show_error(leds, true);
				error = true;
			} else if (sfp->sfp_info.rate >= CVMX_SFP_RATE_10G) {
				/* We don't support 10GBase-T modules in
				 * this mode.
				 */
				switch (sfp->sfp_info.cable_comp) {
				case CVMX_SFP_CABLE_10GBASE_T:
				case CVMX_SFP_CABLE_10GBASE_T_SR:
				case CVMX_SFP_CABLE_5GBASE_T:
				case CVMX_SFP_CABLE_2_5GBASE_T:
					cvmx_helper_leds_show_error(leds, true);
					error = true;
					break;
				default:
					break;
				}
			}
		} else if (multi_led) {
			cvmx_helper_leds_show_error(leds, false);
		}

		if (multi_led && leds->next)
			leds = leds->next;
		sfp = sfp->next_iface_sfp;
	} while (sfp);

	if (!multi_led)
		cvmx_helper_leds_show_error(leds, error);

	return error;
}

/**
 * Validates if the module is correct for the specified port
 *
 * @param[in]	sfp	SFP port to check
 * @param	xiface	interface
 * @param	index	port index
 * @param	speed	link speed, -1 if unknown
 * @param	mode	interface mode
 *
 * @return	true if module is valid, false if invalid
 * NOTE: This will also toggle the error LED, if present
 */
bool cvmx_sfp_validate_module(struct cvmx_fdt_sfp_info *sfp, int mode)
{
	const struct cvmx_sfp_mod_info *mod_info = &sfp->sfp_info;
	int xiface = sfp->xiface;
	int index = sfp->index;
	struct cvmx_phy_gpio_leds *leds;
	bool error = false;
	bool quad_mode = false;

	debug("%s(%s, 0x%x, 0x%x, 0x%x)\n", __func__, sfp->name, xiface, index,
	      mode);
	if (!sfp) {
		debug("%s: Error: sfp is NULL\n", __func__);
		return false;
	}
	/* No module is valid */
	leds = cvmx_helper_get_port_phy_leds(xiface, index);
	if (!leds)
		debug("%s: No leds for 0x%x:0x%x\n", __func__, xiface, index);

	if (mode != CVMX_HELPER_INTERFACE_MODE_XLAUI &&
	    mode != CVMX_HELPER_INTERFACE_MODE_40G_KR4 && !sfp->is_qsfp &&
	    sfp->last_mod_abs && leds) {
		cvmx_helper_leds_show_error(leds, false);
		debug("%s: %s: last_mod_abs: %d, no error\n", __func__,
		      sfp->name, sfp->last_mod_abs);
		return true;
	}

	switch (mode) {
	case CVMX_HELPER_INTERFACE_MODE_RGMII:
	case CVMX_HELPER_INTERFACE_MODE_GMII:
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
	case CVMX_HELPER_INTERFACE_MODE_QSGMII:
	case CVMX_HELPER_INTERFACE_MODE_AGL:
	case CVMX_HELPER_INTERFACE_MODE_SPI:
		if ((mod_info->active_cable &&
		     mod_info->rate != CVMX_SFP_RATE_1G) ||
		    mod_info->rate < CVMX_SFP_RATE_1G)
			error = true;
		break;
	case CVMX_HELPER_INTERFACE_MODE_RXAUI:
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
	case CVMX_HELPER_INTERFACE_MODE_10G_KR:
	case CVMX_HELPER_INTERFACE_MODE_XFI:
		if ((mod_info->active_cable &&
		     mod_info->rate != CVMX_SFP_RATE_10G) ||
		    mod_info->rate < CVMX_SFP_RATE_10G)
			error = true;
		break;
	case CVMX_HELPER_INTERFACE_MODE_XLAUI:
	case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
		if (!sfp->is_qsfp) {
			quad_mode = true;
			error = cvmx_sfp_validate_quad(sfp, leds);
		} else {
			if ((mod_info->active_cable &&
			     mod_info->rate != CVMX_SFP_RATE_40G) ||
			    mod_info->rate < CVMX_SFP_RATE_25G)
				error = true;
		}
		break;
	default:
		debug("%s: Unsupported interface mode %d on xiface 0x%x\n",
		      __func__, mode, xiface);
		return false;
	}
	debug("%s: %s: error: %d\n", __func__, sfp->name, error);
	if (leds && !quad_mode)
		cvmx_helper_leds_show_error(leds, error);

	return !error;
}
