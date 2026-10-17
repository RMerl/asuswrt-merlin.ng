/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef LPDDR4_AM6X_SANITY_H
#define LPDDR4_AM6X_SANITY_H

#include <errno.h>
#include <linux/types.h>
#include <lpddr4_if.h>
#include <lpddr4_if.h>
#include <lpddr4_if.h>
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
		(intr != LPDDR4_INTR_TIMEOUT_ZQ_CAL_INIT) &&
		(intr != LPDDR4_INTR_TIMEOUT_ZQ_CALLATCH) &&
		(intr != LPDDR4_INTR_TIMEOUT_ZQ_CALSTART) &&
		(intr != LPDDR4_INTR_TIMEOUT_MRR_TEMP) &&
		(intr != LPDDR4_INTR_TIMEOUT_DQS_OSC_REQ) &&
		(intr != LPDDR4_INTR_TIMEOUT_DFI_UPDATE) &&
		(intr != LPDDR4_INTR_TIMEOUT_LP_WAKEUP) &&
		(intr != LPDDR4_INTR_TIMEOUT_AUTO_REFRESH_MAX) &&
		(intr != LPDDR4_INTR_ECC_ERROR) &&
		(intr != LPDDR4_INTR_LP_DONE) &&
		(intr != LPDDR4_INTR_LP_TIMEOUT) &&
		(intr != LPDDR4_INTR_PORT_TIMEOUT) &&
		(intr != LPDDR4_INTR_RFIFO_TIMEOUT) &&
		(intr != LPDDR4_INTR_TRAINING_ZQ_STATUS) &&
		(intr != LPDDR4_INTR_TRAINING_DQS_OSC_DONE) &&
		(intr != LPDDR4_INTR_TRAINING_DQS_OSC_UPDATE_DONE) &&
		(intr != LPDDR4_INTR_TRAINING_DQS_OSC_OVERFLOW) &&
		(intr != LPDDR4_INTR_TRAINING_DQS_OSC_VAR_OUT) &&
		(intr != LPDDR4_INTR_USERIF_OUTSIDE_MEM_ACCESS) &&
		(intr != LPDDR4_INTR_USERIF_MULTI_OUTSIDE_MEM_ACCESS) &&
		(intr != LPDDR4_INTR_USERIF_PORT_CMD_ERROR) &&
		(intr != LPDDR4_INTR_USERIF_WRAP) &&
		(intr != LPDDR4_INTR_USERIF_INVAL_SETTING) &&
		(intr != LPDDR4_INTR_MISC_MRR_TRAFFIC) &&
		(intr != LPDDR4_INTR_MISC_SW_REQ_MODE) &&
		(intr != LPDDR4_INTR_MISC_CHANGE_TEMP_REFRESH) &&
		(intr != LPDDR4_INTR_MISC_TEMP_ALERT) &&
		(intr != LPDDR4_INTR_MISC_REFRESH_STATUS) &&
		(intr != LPDDR4_INTR_BIST_DONE) &&
		(intr != LPDDR4_INTR_CRC) &&
		(intr != LPDDR4_INTR_DFI_UPDATE_ERROR) &&
		(intr != LPDDR4_INTR_DFI_PHY_ERROR) &&
		(intr != LPDDR4_INTR_DFI_BUS_ERROR) &&
		(intr != LPDDR4_INTR_DFI_STATE_CHANGE) &&
		(intr != LPDDR4_INTR_DFI_DLL_SYNC_DONE) &&
		(intr != LPDDR4_INTR_DFI_TIMEOUT) &&
		(intr != LPDDR4_INTR_DIMM) &&
		(intr != LPDDR4_INTR_FREQ_DFS_REQ_HW_IGNORE) &&
		(intr != LPDDR4_INTR_FREQ_DFS_HW_TERMINATE) &&
		(intr != LPDDR4_INTR_FREQ_DFS_HW_DONE) &&
		(intr != LPDDR4_INTR_FREQ_DFS_REQ_SW_IGNORE) &&
		(intr != LPDDR4_INTR_FREQ_DFS_SW_TERMINATE) &&
		(intr != LPDDR4_INTR_FREQ_DFS_SW_DONE) &&
		(intr != LPDDR4_INTR_INIT_MEM_RESET_DONE) &&
		(intr != LPDDR4_INTR_MC_INIT_DONE) &&
		(intr != LPDDR4_INTR_INIT_POWER_ON_STATE) &&
		(intr != LPDDR4_INTR_MRR_ERROR) &&
		(intr != LPDDR4_INTR_MR_READ_DONE) &&
		(intr != LPDDR4_INTR_MR_WRITE_DONE) &&
		(intr != LPDDR4_INTR_PARITY_ERROR) &&
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
		(intr != LPDDR4_INTR_TIMEOUT_ZQ_CAL_INIT) &&
		(intr != LPDDR4_INTR_TIMEOUT_ZQ_CALLATCH) &&
		(intr != LPDDR4_INTR_TIMEOUT_ZQ_CALSTART) &&
		(intr != LPDDR4_INTR_TIMEOUT_MRR_TEMP) &&
		(intr != LPDDR4_INTR_TIMEOUT_DQS_OSC_REQ) &&
		(intr != LPDDR4_INTR_TIMEOUT_DFI_UPDATE) &&
		(intr != LPDDR4_INTR_TIMEOUT_LP_WAKEUP) &&
		(intr != LPDDR4_INTR_TIMEOUT_AUTO_REFRESH_MAX) &&
		(intr != LPDDR4_INTR_ECC_ERROR) &&
		(intr != LPDDR4_INTR_LP_DONE) &&
		(intr != LPDDR4_INTR_LP_TIMEOUT) &&
		(intr != LPDDR4_INTR_PORT_TIMEOUT) &&
		(intr != LPDDR4_INTR_RFIFO_TIMEOUT) &&
		(intr != LPDDR4_INTR_TRAINING_ZQ_STATUS) &&
		(intr != LPDDR4_INTR_TRAINING_DQS_OSC_DONE) &&
		(intr != LPDDR4_INTR_TRAINING_DQS_OSC_UPDATE_DONE) &&
		(intr != LPDDR4_INTR_TRAINING_DQS_OSC_OVERFLOW) &&
		(intr != LPDDR4_INTR_TRAINING_DQS_OSC_VAR_OUT) &&
		(intr != LPDDR4_INTR_USERIF_OUTSIDE_MEM_ACCESS) &&
		(intr != LPDDR4_INTR_USERIF_MULTI_OUTSIDE_MEM_ACCESS) &&
		(intr != LPDDR4_INTR_USERIF_PORT_CMD_ERROR) &&
		(intr != LPDDR4_INTR_USERIF_WRAP) &&
		(intr != LPDDR4_INTR_USERIF_INVAL_SETTING) &&
		(intr != LPDDR4_INTR_MISC_MRR_TRAFFIC) &&
		(intr != LPDDR4_INTR_MISC_SW_REQ_MODE) &&
		(intr != LPDDR4_INTR_MISC_CHANGE_TEMP_REFRESH) &&
		(intr != LPDDR4_INTR_MISC_TEMP_ALERT) &&
		(intr != LPDDR4_INTR_MISC_REFRESH_STATUS) &&
		(intr != LPDDR4_INTR_BIST_DONE) &&
		(intr != LPDDR4_INTR_CRC) &&
		(intr != LPDDR4_INTR_DFI_UPDATE_ERROR) &&
		(intr != LPDDR4_INTR_DFI_PHY_ERROR) &&
		(intr != LPDDR4_INTR_DFI_BUS_ERROR) &&
		(intr != LPDDR4_INTR_DFI_STATE_CHANGE) &&
		(intr != LPDDR4_INTR_DFI_DLL_SYNC_DONE) &&
		(intr != LPDDR4_INTR_DFI_TIMEOUT) &&
		(intr != LPDDR4_INTR_DIMM) &&
		(intr != LPDDR4_INTR_FREQ_DFS_REQ_HW_IGNORE) &&
		(intr != LPDDR4_INTR_FREQ_DFS_HW_TERMINATE) &&
		(intr != LPDDR4_INTR_FREQ_DFS_HW_DONE) &&
		(intr != LPDDR4_INTR_FREQ_DFS_REQ_SW_IGNORE) &&
		(intr != LPDDR4_INTR_FREQ_DFS_SW_TERMINATE) &&
		(intr != LPDDR4_INTR_FREQ_DFS_SW_DONE) &&
		(intr != LPDDR4_INTR_INIT_MEM_RESET_DONE) &&
		(intr != LPDDR4_INTR_MC_INIT_DONE) &&
		(intr != LPDDR4_INTR_INIT_POWER_ON_STATE) &&
		(intr != LPDDR4_INTR_MRR_ERROR) &&
		(intr != LPDDR4_INTR_MR_READ_DONE) &&
		(intr != LPDDR4_INTR_MR_WRITE_DONE) &&
		(intr != LPDDR4_INTR_PARITY_ERROR) &&
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
		(intr != LPDDR4_INTR_PHY_INDEP_DLL_LOCK_STATE_CHANGE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_MEM_RST_VALID_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_ZQ_STATUS_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_PERIPHERAL_MRR_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_WRITE_NODEREG_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_FREQ_CHANGE_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_RDLVL_G_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_RDLVL_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_WRLVL_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_CALVL_DONE__BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_WDQLVL_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_VREF_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_ANY_VALID_BIT)
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
		(intr != LPDDR4_INTR_PHY_INDEP_DLL_LOCK_STATE_CHANGE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_MEM_RST_VALID_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_ZQ_STATUS_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_PERIPHERAL_MRR_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_WRITE_NODEREG_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_FREQ_CHANGE_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_RDLVL_G_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_RDLVL_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_WRLVL_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_CALVL_DONE__BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_WDQLVL_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_VREF_DONE_BIT) &&
		(intr != LPDDR4_INTR_PHY_INDEP_ANY_VALID_BIT)
		) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

#endif  /* LPDDR4_AM6X_SANITY_H */
