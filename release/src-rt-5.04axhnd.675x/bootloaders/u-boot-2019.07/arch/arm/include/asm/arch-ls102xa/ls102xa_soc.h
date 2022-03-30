/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#ifndef __FSL_LS102XA_SOC_H
#define __FSL_LS102XA_SOC_H

unsigned int get_soc_major_rev(void);
int arch_soc_init(void);
int ls102xa_smmu_stream_id_init(void);

void erratum_a008850_post(void);

#ifdef CONFIG_SYS_FSL_ERRATUM_A010315
void erratum_a010315(void);
#endif

#endif /* __FSL_LS102XA_SOC_H */
