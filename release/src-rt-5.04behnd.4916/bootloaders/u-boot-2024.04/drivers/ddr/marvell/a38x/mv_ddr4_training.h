/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _MV_DDR4_TRAINING_H
#define _MV_DDR4_TRAINING_H

#include "ddr3_training_ip.h"

/* configure DDR4 SDRAM */
int mv_ddr4_sdram_config(u32 dev_num);

/* configure phy */
int mv_ddr4_phy_config(u32 dev_num);

/*
 * configure sstl for manual calibration and pod for automatic one
 * assumes subphy configured to pod ealier
 */
int mv_ddr4_calibration_adjust(u32 dev_num, u8 vref_en, u8 pod_only);

/*
 * validates calibration values
 * soc dependent; TODO: check it
 */
int mv_ddr4_calibration_validate(u32 dev_num);

u16 mv_ddr4_rtt_nom_to_odt(u16 rtt_nom);
u16 mv_ddr4_rtt_wr_to_odt(u16 rtt_wr);

#endif /* _MV_DDR4_TRAINING_H */
