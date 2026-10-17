/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef LPDDR4_J721E_SANITY_H
#define LPDDR4_J721E_SANITY_H

#include <errno.h>
#include <linux/types.h>
#include <lpddr4_if.h>

static inline u32 lpddr4_intr_sanityfunction1(const lpddr4_privatedata *pd, const lpddr4_intr_ctlinterrupt intr, const bool *irqstatus);
static inline u32 lpddr4_intr_sanityfunction2(const lpddr4_privatedata *pd, const lpddr4_intr_ctlinterrupt intr);
static inline u32 lpddr4_intr_sanityfunction3(const lpddr4_privatedata *pd, const lpddr4_intr_phyindepinterrupt intr, const bool *irqstatus);
static inline u32 lpddr4_intr_sanityfunction4(const lpddr4_privatedata *pd, const lpddr4_intr_phyindepinterrupt intr);

#define LPDDR4_INTR_CheckCtlIntSF lpddr4_intr_sanityfunction1
#define LPDDR4_INTR_AckCtlIntSF lpddr4_intr_sanityfunction2
#define LPDDR4_INTR_CheckPhyIndepIntSF lpddr4_intr_sanityfunction3
#define LPDDR4_INTR_AckPhyIndepIntSF lpddr4_intr_sanityfunction4

static inline u32 lpddr4_intr_sanityfunction1(const lpddr4_privatedata *pd, const lpddr4_intr_ctlinterrupt intr, const bool *irqstatus)
{
	u32 ret = 0;

	if (pd == NULL) {
		ret = EINVAL;
	} else if (irqstatus == NULL) {
		ret = EINVAL;
	} else if (
		(intr != LPDDR4_INTR_RESET_DONE) &&
		(intr != LPDDR4_INTR_BUS_ACCESS_ERROR) &&
		(intr != LPDDR4_INTR_MULTIPLE_BUS_ACCESS_ERROR) &&
		(intr != LPDDR4_INTR_ECC_MULTIPLE_CORR_ERROR) &&
		(intr != LPDDR4_INTR_ECC_MULTIPLE_UNCORR_ERROR) &&
		(intr != LPDDR4_INTR_ECC_WRITEBACK_EXEC_ERROR) &&
		(intr != LPDDR4_INTR_ECC_SCRUB_DONE) &&
		(intr != LPDDR4_INTR_ECC_SCRUB_ERROR) &&
		(intr != LPDDR4_INTR_PORT_COMMAND_ERROR) &&
		(intr != LPDDR4_INTR_MC_INIT_DONE) &&
		(intr != LPDDR4_INTR_LP_DONE) &&
		(intr != LPDDR4_INTR_BIST_DONE) &&
		(intr != LPDDR4_INTR_WRAP_ERROR) &&
		(intr != LPDDR4_INTR_INVALID_BURST_ERROR) &&
		(intr != LPDDR4_INTR_RDLVL_ERROR) &&
		(intr != LPDDR4_INTR_RDLVL_GATE_ERROR) &&
		(intr != LPDDR4_INTR_WRLVL_ERROR) &&
		(intr != LPDDR4_INTR_CA_TRAINING_ERROR) &&
		(intr != LPDDR4_INTR_DFI_UPDATE_ERROR) &&
		(intr != LPDDR4_INTR_MRR_ERROR) &&
		(intr != LPDDR4_INTR_PHY_MASTER_ERROR) &&
		(intr != LPDDR4_INTR_WRLVL_REQ) &&
		(intr != LPDDR4_INTR_RDLVL_REQ) &&
		(intr != LPDDR4_INTR_RDLVL_GATE_REQ) &&
		(intr != LPDDR4_INTR_CA_TRAINING_REQ) &&
		(intr != LPDDR4_INTR_LEVELING_DONE) &&
		(intr != LPDDR4_INTR_PHY_ERROR) &&
		(intr != LPDDR4_INTR_MR_READ_DONE) &&
		(intr != LPDDR4_INTR_TEMP_CHANGE) &&
		(intr != LPDDR4_INTR_TEMP_ALERT) &&
		(intr != LPDDR4_INTR_SW_DQS_COMPLETE) &&
		(intr != LPDDR4_INTR_DQS_OSC_BV_UPDATED) &&
		(intr != LPDDR4_INTR_DQS_OSC_OVERFLOW) &&
		(intr != LPDDR4_INTR_DQS_OSC_VAR_OUT) &&
		(intr != LPDDR4_INTR_MR_WRITE_DONE) &&
		(intr != LPDDR4_INTR_INHIBIT_DRAM_DONE) &&
		(intr != LPDDR4_INTR_DFI_INIT_STATE) &&
		(intr != LPDDR4_INTR_DLL_RESYNC_DONE) &&
		(intr != LPDDR4_INTR_TDFI_TO) &&
		(intr != LPDDR4_INTR_DFS_DONE) &&
		(intr != LPDDR4_INTR_DFS_STATUS) &&
		(intr != LPDDR4_INTR_REFRESH_STATUS) &&
		(intr != LPDDR4_INTR_ZQ_STATUS) &&
		(intr != LPDDR4_INTR_SW_REQ_MODE) &&
		(intr != LPDDR4_INTR_LOR_BITS)
		) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_intr_sanityfunction2(const lpddr4_privatedata *pd, const lpddr4_intr_ctlinterrupt intr)
{
	u32 ret = 0;

	if (pd == NULL) {
		ret = EINVAL;
	} else if (
		(intr != LPDDR4_INTR_RESET_DONE) &&
		(intr != LPDDR4_INTR_BUS_ACCESS_ERROR) &&
		(intr != LPDDR4_INTR_MULTIPLE_BUS_ACCESS_ERROR) &&
		(intr != LPDDR4_INTR_ECC_MULTIPLE_CORR_ERROR) &&
		(intr != LPDDR4_INTR_ECC_MULTIPLE_UNCORR_ERROR) &&
		(intr != LPDDR4_INTR_ECC_WRITEBACK_EXEC_ERROR) &&
		(intr != LPDDR4_INTR_ECC_SCRUB_DONE) &&
		(intr != LPDDR4_INTR_ECC_SCRUB_ERROR) &&
		(intr != LPDDR4_INTR_PORT_COMMAND_ERROR) &&
		(intr != LPDDR4_INTR_MC_INIT_DONE) &&
		(intr != LPDDR4_INTR_LP_DONE) &&
		(intr != LPDDR4_INTR_BIST_DONE) &&
		(intr != LPDDR4_INTR_WRAP_ERROR) &&
		(intr != LPDDR4_INTR_INVALID_BURST_ERROR) &&
		(intr != LPDDR4_INTR_RDLVL_ERROR) &&
		(intr != LPDDR4_INTR_RDLVL_GATE_ERROR) &&
		(intr != LPDDR4_INTR_WRLVL_ERROR) &&
		(intr != LPDDR4_INTR_CA_TRAINING_ERROR) &&
		(intr != LPDDR4_INTR_DFI_UPDATE_ERROR) &&
		(intr != LPDDR4_INTR_MRR_ERROR) &&
		(intr != LPDDR4_INTR_PHY_MASTER_ERROR) &&
		(intr != LPDDR4_INTR_WRLVL_REQ) &&
		(intr != LPDDR4_INTR_RDLVL_REQ) &&
		(intr != LPDDR4_INTR_RDLVL_GATE_REQ) &&
		(intr != LPDDR4_INTR_CA_TRAINING_REQ) &&
		(intr != LPDDR4_INTR_LEVELING_DONE) &&
		(intr != LPDDR4_INTR_PHY_ERROR) &&
		(intr != LPDDR4_INTR_MR_READ_DONE) &&
		(intr != LPDDR4_INTR_TEMP_CHANGE) &&
		(intr != LPDDR4_INTR_TEMP_ALERT) &&
		(intr != LPDDR4_INTR_SW_DQS_COMPLETE) &&
		(intr != LPDDR4_INTR_DQS_OSC_BV_UPDATED) &&
		(intr != LPDDR4_INTR_DQS_OSC_OVERFLOW) &&
		(intr != LPDDR4_INTR_DQS_OSC_VAR_OUT) &&
		(intr != LPDDR4_INTR_MR_WRITE_DONE) &&
		(intr != LPDDR4_INTR_INHIBIT_DRAM_DONE) &&
		(intr != LPDDR4_INTR_DFI_INIT_STATE) &&
		(intr != LPDDR4_INTR_DLL_RESYNC_DONE) &&
		(intr != LPDDR4_INTR_TDFI_TO) &&
		(intr != LPDDR4_INTR_DFS_DONE) &&
		(intr != LPDDR4_INTR_DFS_STATUS) &&
		(intr != LPDDR4_INTR_REFRESH_STATUS) &&
		(intr != LPDDR4_INTR_ZQ_STATUS) &&
		(intr != LPDDR4_INTR_SW_REQ_MODE) &&
		(intr != LPDDR4_INTR_LOR_BITS)
		) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_intr_sanityfunction3(const lpddr4_privatedata *pd, const lpddr4_intr_phyindepinterrupt intr, const bool *irqstatus)
{
	u32 ret = 0;

	if (pd == NULL) {
		ret = EINVAL;
	} else if (irqstatus == NULL) {
		ret = EINVAL;
	} else if (
		(intr != LPDDR4_INTR_PHY_INDEP_INIT_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_CONTROL_ERROR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_CA_PARITY_ERR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_RDLVL_ERROR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_RDLVL_G_ERROR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_WRLVL_ERROR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_CALVL_ERROR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_WDQLVL_ERROR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_UPDATE_ERROR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_RDLVL_REQ_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_RDLVL_GATE_REQ_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_WRLVL_REQ_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_CALVL_REQ_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_WDQLVL_REQ_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_LVL_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_BIST_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_TDFI_INIT_TIME_OUT_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_DLL_LOCK_STATE_CHANGE_BIT)
		) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_intr_sanityfunction4(const lpddr4_privatedata *pd, const lpddr4_intr_phyindepinterrupt intr)
{
	u32 ret = 0;

	if (pd == NULL) {
		ret = EINVAL;
	} else if (
		(intr != LPDDR4_INTR_PHY_INDEP_INIT_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_CONTROL_ERROR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_CA_PARITY_ERR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_RDLVL_ERROR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_RDLVL_G_ERROR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_WRLVL_ERROR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_CALVL_ERROR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_WDQLVL_ERROR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_UPDATE_ERROR_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_RDLVL_REQ_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_RDLVL_GATE_REQ_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_WRLVL_REQ_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_CALVL_REQ_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_WDQLVL_REQ_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_LVL_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_BIST_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_TDFI_INIT_TIME_OUT_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_DLL_LOCK_STATE_CHANGE_BIT)
		) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

#endif  /* LPDDR4_J721E_SANITY_H */
