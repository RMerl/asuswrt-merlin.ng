// SPDX-License-Identifier: GPL-2.0
/*
 * From Coreboot src/southbridge/intel/bd82x6x/me_status.c
 *
 * Copyright (C) 2011 The Chromium OS Authors. All rights reserved.
 */

#include <common.h>
#include <asm/arch/me.h>

/* HFS1[3:0] Current Working State Values */
static const char *const me_cws_values[] = {
	[ME_HFS_CWS_RESET]	= "Reset",
	[ME_HFS_CWS_INIT]	= "Initializing",
	[ME_HFS_CWS_REC]	= "Recovery",
	[ME_HFS_CWS_NORMAL]	= "Normal",
	[ME_HFS_CWS_WAIT]	= "Platform Disable Wait",
	[ME_HFS_CWS_TRANS]	= "OP State Transition",
	[ME_HFS_CWS_INVALID]	= "Invalid CPU Plugged In"
};

/* HFS1[8:6] Current Operation State Values */
static const char *const me_opstate_values[] = {
	[ME_HFS_STATE_PREBOOT]	= "Preboot",
	[ME_HFS_STATE_M0_UMA]	= "M0 with UMA",
	[ME_HFS_STATE_M3]	= "M3 without UMA",
	[ME_HFS_STATE_M0]	= "M0 without UMA",
	[ME_HFS_STATE_BRINGUP]	= "Bring up",
	[ME_HFS_STATE_ERROR]	= "M0 without UMA but with error"
};

/* HFS[19:16] Current Operation Mode Values */
static const char *const me_opmode_values[] = {
	[ME_HFS_MODE_NORMAL]	= "Normal",
	[ME_HFS_MODE_DEBUG]	= "Debug",
	[ME_HFS_MODE_DIS]	= "Soft Temporary Disable",
	[ME_HFS_MODE_OVER_JMPR]	= "Security Override via Jumper",
	[ME_HFS_MODE_OVER_MEI]	= "Security Override via MEI Message"
};

/* HFS[15:12] Error Code Values */
static const char *const me_error_values[] = {
	[ME_HFS_ERROR_NONE]	= "No Error",
	[ME_HFS_ERROR_UNCAT]	= "Uncategorized Failure",
	[ME_HFS_ERROR_IMAGE]	= "Image Failure",
	[ME_HFS_ERROR_DEBUG]	= "Debug Failure"
};

/* GMES[31:28] ME Progress Code */
static const char *const me_progress_values[] = {
	[ME_GMES_PHASE_ROM]	= "ROM Phase",
	[ME_GMES_PHASE_BUP]	= "BUP Phase",
	[ME_GMES_PHASE_UKERNEL]	= "uKernel Phase",
	[ME_GMES_PHASE_POLICY]	= "Policy Module",
	[ME_GMES_PHASE_MODULE]	= "Module Loading",
	[ME_GMES_PHASE_UNKNOWN]	= "Unknown",
	[ME_GMES_PHASE_HOST]	= "Host Communication"
};

/* GMES[27:24] Power Management Event */
static const char *const me_pmevent_values[] = {
	[0x00] = "Clean Moff->Mx wake",
	[0x01] = "Moff->Mx wake after an error",
	[0x02] = "Clean global reset",
	[0x03] = "Global reset after an error",
	[0x04] = "Clean Intel ME reset",
	[0x05] = "Intel ME reset due to exception",
	[0x06] = "Pseudo-global reset",
	[0x07] = "S0/M0->Sx/M3",
	[0x08] = "Sx/M3->S0/M0",
	[0x09] = "Non-power cycle reset",
	[0x0a] = "Power cycle reset through M3",
	[0x0b] = "Power cycle reset through Moff",
	[0x0c] = "Sx/Mx->Sx/Moff"
};

/* Progress Code 0 states */
static const char *const me_progress_rom_values[] = {
	[0x00] = "BEGIN",
	[0x06] = "DISABLE"
};

/* Progress Code 1 states */
static const char *const me_progress_bup_values[] = {
	[0x00] = "Initialization starts",
	[0x01] = "Disable the host wake event",
	[0x04] = "Flow determination start process",
	[0x08] = "Error reading/matching the VSCC table in the descriptor",
	[0x0a] = "Check to see if straps say ME DISABLED",
	[0x0b] = "Timeout waiting for PWROK",
	[0x0d] = "Possibly handle BUP manufacturing override strap",
	[0x11] = "Bringup in M3",
	[0x12] = "Bringup in M0",
	[0x13] = "Flow detection error",
	[0x15] = "M3 clock switching error",
	[0x18] = "M3 kernel load",
	[0x1c] = "T34 missing - cannot program ICC",
	[0x1f] = "Waiting for DID BIOS message",
	[0x20] = "Waiting for DID BIOS message failure",
	[0x21] = "DID reported an error",
	[0x22] = "Enabling UMA",
	[0x23] = "Enabling UMA error",
	[0x24] = "Sending DID Ack to BIOS",
	[0x25] = "Sending DID Ack to BIOS error",
	[0x26] = "Switching clocks in M0",
	[0x27] = "Switching clocks in M0 error",
	[0x28] = "ME in temp disable",
	[0x32] = "M0 kernel load",
};

/* Progress Code 3 states */
static const char *const me_progress_policy_values[] = {
	[0x00] = "Entery into Policy Module",
	[0x03] = "Received S3 entry",
	[0x04] = "Received S4 entry",
	[0x05] = "Received S5 entry",
	[0x06] = "Received UPD entry",
	[0x07] = "Received PCR entry",
	[0x08] = "Received NPCR entry",
	[0x09] = "Received host wake",
	[0x0a] = "Received AC<>DC switch",
	[0x0b] = "Received DRAM Init Done",
	[0x0c] = "VSCC Data not found for flash device",
	[0x0d] = "VSCC Table is not valid",
	[0x0e] = "Flash Partition Boundary is outside address space",
	[0x0f] = "ME cannot access the chipset descriptor region",
	[0x10] = "Required VSCC values for flash parts do not match",
};


/**
 * _intel_me_status() - Check Intel Management Engine status
 *
 * struct hfs:	Firmware status
 * struct gmes:	Management engine status
 */
static void _intel_me_status(struct me_hfs *hfs, struct me_gmes *gmes)
{
	/* Check Current States */
	debug("ME: FW Partition Table      : %s\n",
	      hfs->fpt_bad ? "BAD" : "OK");
	debug("ME: Bringup Loader Failure  : %s\n",
	      hfs->ft_bup_ld_flr ? "YES" : "NO");
	debug("ME: Firmware Init Complete  : %s\n",
	      hfs->fw_init_complete ? "YES" : "NO");
	debug("ME: Manufacturing Mode      : %s\n",
	      hfs->mfg_mode ? "YES" : "NO");
	debug("ME: Boot Options Present    : %s\n",
	      hfs->boot_options_present ? "YES" : "NO");
	debug("ME: Update In Progress      : %s\n",
	      hfs->update_in_progress ? "YES" : "NO");
	debug("ME: Current Working State   : %s\n",
	      me_cws_values[hfs->working_state]);
	debug("ME: Current Operation State : %s\n",
	      me_opstate_values[hfs->operation_state]);
	debug("ME: Current Operation Mode  : %s\n",
	      me_opmode_values[hfs->operation_mode]);
	debug("ME: Error Code              : %s\n",
	      me_error_values[hfs->error_code]);
	debug("ME: Progress Phase          : %s\n",
	      me_progress_values[gmes->progress_code]);
	debug("ME: Power Management Event  : %s\n",
	      me_pmevent_values[gmes->current_pmevent]);

	debug("ME: Progress Phase State    : ");
	switch (gmes->progress_code) {
	case ME_GMES_PHASE_ROM:		/* ROM Phase */
		debug("%s", me_progress_rom_values[gmes->current_state]);
		break;

	case ME_GMES_PHASE_BUP:		/* Bringup Phase */
		if (gmes->current_state < ARRAY_SIZE(me_progress_bup_values) &&
		    me_progress_bup_values[gmes->current_state])
			debug("%s",
			      me_progress_bup_values[gmes->current_state]);
		else
			debug("0x%02x", gmes->current_state);
		break;

	case ME_GMES_PHASE_POLICY:	/* Policy Module Phase */
		if (gmes->current_state <
				ARRAY_SIZE(me_progress_policy_values) &&
		    me_progress_policy_values[gmes->current_state])
			debug("%s",
			      me_progress_policy_values[gmes->current_state]);
		else
			debug("0x%02x", gmes->current_state);
		break;

	case ME_GMES_PHASE_HOST:	/* Host Communication Phase */
		if (!gmes->current_state)
			debug("Host communication established");
		else
			debug("0x%02x", gmes->current_state);
		break;

	default:
		debug("Unknown 0x%02x", gmes->current_state);
	}
	debug("\n");
}

void intel_me_status(struct udevice *me_dev)
{
	struct me_hfs hfs;
	struct me_gmes gmes;

	pci_read_dword_ptr(me_dev, &hfs, PCI_ME_HFS);
	pci_read_dword_ptr(me_dev, &gmes, PCI_ME_GMES);

	_intel_me_status(&hfs, &gmes);
}
