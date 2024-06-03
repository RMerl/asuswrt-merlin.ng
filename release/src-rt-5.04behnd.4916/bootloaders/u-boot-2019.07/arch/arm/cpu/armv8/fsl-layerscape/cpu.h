/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014-2015, Freescale Semiconductor
 */

int fsl_qoriq_core_to_cluster(unsigned int core);
u32 initiator_type(u32 cluster, int init_id);
u32 cpu_mask(void);
