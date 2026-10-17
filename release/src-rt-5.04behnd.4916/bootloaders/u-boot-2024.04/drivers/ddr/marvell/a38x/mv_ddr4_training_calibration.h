/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _MV_DDR4_TRAINING_CALIBRATION_H
#define _MV_DDR4_TRAINING_CALIBRATION_H

/* vref subphy calibration state */
enum mv_ddr4_vref_subphy_cal_state {
	MV_DDR4_VREF_SUBPHY_CAL_ABOVE,
	MV_DDR4_VREF_SUBPHY_CAL_UNDER,
	MV_DDR4_VREF_SUBPHY_CAL_INSIDE,
	MV_DDR4_VREF_SUBPHY_CAL_END
};

/* calibrate DDR4 dq vref (tx) */
int mv_ddr4_dq_vref_calibration(u8 dev_num, u16 (*pbs_tap_factor)[MAX_BUS_NUM][BUS_WIDTH_IN_BITS]);

/* calibrate receiver (receiver duty cycle) */
int mv_ddr4_receiver_calibration(u8 dev_num);

/* tune dm signal */
int mv_ddr4_dm_tuning(u32 cs, u16 (*pbs_tap_factor)[MAX_BUS_NUM][BUS_WIDTH_IN_BITS]);

#endif /* _MV_DDR4_TRAINING_CALIBRATION_H */
