/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Ported from Intel released Quark UEFI BIOS
 * QuarkSocPkg/QuarkNorthCluster/MemoryInit/Pei
 */

#ifndef _HTE_H_
#define _HTE_H_

enum {
	MRC_MEM_INIT,
	MRC_MEM_TEST
};

enum {
	READ_TRAIN,
	WRITE_TRAIN
};

/*
 * EXP_LOOP_CNT field of HTE_CMD_CTL
 *
 * This CANNOT be less than 4!
 */
#define HTE_LOOP_CNT		5

/* random seed for victim */
#define HTE_LFSR_VICTIM_SEED	0xf294ba21

/* random seed for aggressor */
#define HTE_LFSR_AGRESSOR_SEED	0xeba7492d

u32 hte_mem_init(struct mrc_params *mrc_params, u8 flag);
u16 hte_basic_write_read(struct mrc_params *mrc_params, u32 addr,
			 u8 first_run, u8 mode);
u16 hte_write_stress_bit_lanes(struct mrc_params *mrc_params,
			       u32 addr, u8 first_run);
void hte_mem_op(u32 addr, u8 first_run, u8 is_write);

#endif /* _HTE_H_ */
