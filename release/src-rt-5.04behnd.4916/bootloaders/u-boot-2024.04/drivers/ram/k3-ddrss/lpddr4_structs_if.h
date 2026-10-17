/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef LPDDR4_STRUCTS_IF_H
#define LPDDR4_STRUCTS_IF_H

#include <linux/types.h>
#include "lpddr4_if.h"

struct lpddr4_config_s {
	struct lpddr4_ctlregs_s *ctlbase;
	lpddr4_infocallback infohandler;
	lpddr4_ctlcallback ctlinterrupthandler;
	lpddr4_phyindepcallback phyindepinterrupthandler;
};

struct lpddr4_privatedata_s {
	struct lpddr4_ctlregs_s *ctlbase;
	lpddr4_infocallback infohandler;
	lpddr4_ctlcallback ctlinterrupthandler;
	lpddr4_phyindepcallback phyindepinterrupthandler;
	void *ddr_instance;
};

struct lpddr4_debuginfo_s {
	u8 pllerror;
	u8 iocaliberror;
	u8 rxoffseterror;
	u8 catraingerror;
	u8 wrlvlerror;
	u8 gatelvlerror;
	u8 readlvlerror;
	u8 dqtrainingerror;
};

struct lpddr4_fspmoderegs_s {
	u8 mr1data_fn[LPDDR4_INTR_MAX_CS];
	u8 mr2data_fn[LPDDR4_INTR_MAX_CS];
	u8 mr3data_fn[LPDDR4_INTR_MAX_CS];
	u8 mr11data_fn[LPDDR4_INTR_MAX_CS];
	u8 mr12data_fn[LPDDR4_INTR_MAX_CS];
	u8 mr13data_fn[LPDDR4_INTR_MAX_CS];
	u8 mr14data_fn[LPDDR4_INTR_MAX_CS];
	u8 mr22data_fn[LPDDR4_INTR_MAX_CS];
};

#endif  /* LPDDR4_STRUCTS_IF_H */
