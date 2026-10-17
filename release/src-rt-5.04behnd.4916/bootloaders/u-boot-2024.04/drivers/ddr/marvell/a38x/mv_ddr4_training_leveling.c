// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#if defined(CONFIG_DDR4)

#include "ddr3_init.h"
#include "mv_ddr_regs.h"

static int mv_ddr4_dynamic_pb_wl_supp(u32 dev_num, enum mv_wl_supp_mode ecc_mode);

/* compare test for ddr4 write leveling supplementary */
#define MV_DDR4_COMP_TEST_NO_RESULT	0
#define MV_DDR4_COMP_TEST_RESULT_0	1
#define MV_DDR4_XSB_COMP_PATTERNS_NUM	8

static u8 mv_ddr4_xsb_comp_test(u32 dev_num, u32 subphy_num, u32 if_id,
				enum mv_wl_supp_mode ecc_mode)
{
	u32 wl_invert;
	u8 pb_key, bit, bit_max, word;
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 subphy_max = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	uint64_t read_pattern_64[MV_DDR4_XSB_COMP_PATTERNS_NUM] = {0};
	/*
	 * FIXME: the pattern below is used for writing to the memory
	 * by the cpu. it was changed to be written through the odpg.
	 * for a workaround
	 * uint64_t pattern_test_table_64[MV_DDR4_XSB_COMP_PATTERNS_NUM] = {
	 *	0xffffffffffffffff,
	 *	0xffffffffffffffff,
	 *	0x0000000000000000,
	 *	0x0000000000000000,
	 *	0x0000000000000000,
	 *	0x0000000000000000,
	 *	0xffffffffffffffff,
	 *	0xffffffffffffffff};
	 */
	u32 read_pattern[MV_DDR4_XSB_COMP_PATTERNS_NUM];
	/*u32 pattern_test_table[MV_DDR4_XSB_COMP_PATTERNS_NUM] = {
		0xffffffff,
		0xffffffff,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0xffffffff,
		0xffffffff};	TODO: use pattern_table_get_word */
	int i, status;
	uint64_t data64;
	uintptr_t addr64;
	int ecc_running = 0;
	u32 ecc_read_subphy_num = 0; /* FIXME: change ecc read subphy num to be configurable */
	u8 bit_counter = 0;
	int edge = 0;
	/* write and read data */
	if (MV_DDR_IS_64BIT_DRAM_MODE(tm->bus_act_mask)) {
		status = ddr3_tip_if_write(0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ODPG_DATA_CTRL_REG,
					   effective_cs << ODPG_DATA_CS_OFFS,
					   ODPG_DATA_CS_MASK << ODPG_DATA_CS_OFFS);
		if (status != MV_OK)
			return status;

		addr64 = (uintptr_t)pattern_table[PATTERN_TEST].start_addr;
		/*
		 * FIXME: changed the load pattern to memory through the odpg
		 * this change is needed to be validate
		 * this change is done due to un calibrated dm at this stage
		 * the below code is the code for loading the pattern directly
		 * to the memory
		 *
		 * for (i = 0; i < MV_DDR4_XSB_COMP_PATTERNS_NUM; i++) {
		 *	data64 = pattern_test_table_64[i];
		 *	writeq(addr64, data64);
		 *	addr64 +=  sizeof(uint64_t);
		 *}
		 * FIXME: the below code loads the pattern to the memory through the odpg
		 * it loads it twice to due supplementary failure, need to check it
		 */
		int j;
		for (j = 0; j < 2; j++)
			ddr3_tip_load_pattern_to_mem(dev_num, PATTERN_TEST);

	} else if (MV_DDR_IS_32BIT_IN_64BIT_DRAM_MODE(tm->bus_act_mask, subphy_max)) {
		status = ddr3_tip_if_write(0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ODPG_DATA_CTRL_REG,
					       effective_cs << ODPG_DATA_CS_OFFS,
					       ODPG_DATA_CS_MASK << ODPG_DATA_CS_OFFS);
		if (status != MV_OK)
			return status;

		/*
		 * FIXME: changed the load pattern to memory through the odpg
		 * this change is needed to be validate
		 * this change is done due to un calibrated dm at this stage
		 * the below code is the code for loading the pattern directly
		 * to the memory
		 */
		int j;
		for (j = 0; j < 2; j++)
			ddr3_tip_load_pattern_to_mem(dev_num, PATTERN_TEST);
	} else {
		/*
		 * FIXME: changed the load pattern to memory through the odpg
		 * this change is needed to be validate
		 * this change is done due to un calibrated dm at this stage
		 * the below code is the code for loading the pattern directly
		 * to the memory
		 */
		int j;
		for (j = 0; j < 2; j++)
			ddr3_tip_load_pattern_to_mem(dev_num, PATTERN_TEST);
	}

	if ((ecc_mode == WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP4) ||
	    (ecc_mode == WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP3 ||
	     ecc_mode == WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP8)) {
		/* disable ecc write mux */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
					   TRAINING_SW_2_REG, 0x0, 0x100);
		if (status != MV_OK)
			return status;

		/* enable read data ecc mux */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
					   TRAINING_SW_2_REG, 0x3, 0x3);
		if (status != MV_OK)
			return status;

		/* unset training start bit */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
					   TRAINING_REG, 0x80000000, 0x80000000);
		if (status != MV_OK)
			return status;

		ecc_running = 1;
		ecc_read_subphy_num = ECC_READ_BUS_0;
	}

	if (MV_DDR_IS_64BIT_DRAM_MODE(tm->bus_act_mask)) {
		status = ddr3_tip_if_write(0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ODPG_DATA_CTRL_REG,
					       effective_cs << ODPG_DATA_CS_OFFS,
					       ODPG_DATA_CS_MASK << ODPG_DATA_CS_OFFS);
		if (status != MV_OK)
			return status;
		/*
		 * in case of reading the pattern read it from the address x 8
		 * the odpg multiply by 8 the addres to read from
		 */
		addr64 = ((uintptr_t)pattern_table[PATTERN_TEST].start_addr) << 3;
		for (i = 0; i < MV_DDR4_XSB_COMP_PATTERNS_NUM; i++) {
			data64 = readq(addr64);
			addr64 +=  sizeof(uint64_t);
			read_pattern_64[i] = data64;
		}

		DEBUG_LEVELING(DEBUG_LEVEL_INFO, ("xsb comp: if %d bus id %d\n", 0, subphy_num));
		for (edge = 0; edge < 8; edge++)
			DEBUG_LEVELING(DEBUG_LEVEL_INFO, ("0x%16llx\n", (unsigned long long)read_pattern_64[edge]));
		DEBUG_LEVELING(DEBUG_LEVEL_INFO, ("\n"));
	} else if (MV_DDR_IS_32BIT_IN_64BIT_DRAM_MODE(tm->bus_act_mask, subphy_max)) {
		status = ddr3_tip_if_write(0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ODPG_DATA_CTRL_REG,
					       effective_cs << ODPG_DATA_CS_OFFS,
					       ODPG_DATA_CS_MASK << ODPG_DATA_CS_OFFS);
		if (status != MV_OK)
			return status;

		status = ddr3_tip_ext_read(dev_num, if_id, pattern_table[PATTERN_TEST].start_addr << 3,
					   1, read_pattern);
		if (status != MV_OK)
			return status;

		DEBUG_LEVELING(DEBUG_LEVEL_INFO, ("xsb comp: if %d bus id %d\n", 0, subphy_num));
		for (edge = 0; edge < 8; edge++)
			DEBUG_LEVELING(DEBUG_LEVEL_INFO, ("0x%16x\n", read_pattern[edge]));
		DEBUG_LEVELING(DEBUG_LEVEL_INFO, ("\n"));
	} else {
		status = ddr3_tip_ext_read(dev_num, if_id, ((pattern_table[PATTERN_TEST].start_addr << 3) +
					    ((SDRAM_CS_SIZE + 1) * effective_cs)), 1, read_pattern);
		if (status != MV_OK)
			return status;

		DEBUG_LEVELING(DEBUG_LEVEL_INFO, ("xsb comp: if %d bus id %d\n", 0, subphy_num));
		for (edge = 0; edge < 8; edge++)
			DEBUG_LEVELING(DEBUG_LEVEL_INFO, ("0x%16x\n", read_pattern[edge]));
		DEBUG_LEVELING(DEBUG_LEVEL_INFO, ("\n"));
	}

	/* read centralization result to decide on half phase by inverse bit */
	status = ddr3_tip_bus_read(dev_num, if_id, ACCESS_TYPE_UNICAST, subphy_num, DDR_PHY_DATA,
				   CTX_PHY_REG(0), &wl_invert);
	if (status != MV_OK)
		return status;

	if ((wl_invert & 0x20) != 0)
		wl_invert = 1;
	else
		wl_invert = 0;

	/* for ecc, read from the "read" subphy (usualy subphy 0) */
	if (ecc_running)
		subphy_num = ecc_read_subphy_num;

	/* per bit loop*/
	bit_max = subphy_num * BUS_WIDTH_IN_BITS + BUS_WIDTH_IN_BITS;
	for (bit = subphy_num * BUS_WIDTH_IN_BITS; bit < bit_max; bit++) {
		/* get per bit pattern key (value of the same bit in the pattern) */
		pb_key = 0;
		for (word = 0; word < MV_DDR4_XSB_COMP_PATTERNS_NUM; word++) {
			if (MV_DDR_IS_64BIT_DRAM_MODE(tm->bus_act_mask)) {
				if ((read_pattern_64[word] & ((uint64_t)1 << bit)) != 0)
					pb_key |= (1 << word);
			} else {
				if ((read_pattern[word] & (1 << bit)) != 0)
					pb_key |= (1 << word);
			}
		}

		/* find the key value and make decision */
		switch (pb_key) {
		/* case(s) for 0 */
		case 0b11000011:	/* nominal */
		case 0b10000011:	/* sample at start of UI sample at the dqvref TH */
		case 0b10000111:	/* sample at start of UI sample at the dqvref TH */
		case 0b11000001:	/* sample at start of UI sample at the dqvref TH */
		case 0b11100001:	/* sample at start of UI sample at the dqvref TH */
		case 0b11100011:	/* sample at start of UI sample at the dqvref TH */
		case 0b11000111:	/* sample at start of UI sample at the dqvref TH */
			bit_counter++;
			break;
		} /* end of switch */
	} /* end of per bit loop */

	/* check all bits in the current subphy has met the switch condition above */
	if (bit_counter == BUS_WIDTH_IN_BITS)
		return MV_DDR4_COMP_TEST_RESULT_0;
	else {
		DEBUG_LEVELING(
			       DEBUG_LEVEL_INFO,
			       ("different supplementary results (%d -> %d)\n",
			       MV_DDR4_COMP_TEST_NO_RESULT, MV_DDR4_COMP_TEST_RESULT_0));
		return MV_DDR4_COMP_TEST_NO_RESULT;
	}
}

int mv_ddr4_dynamic_wl_supp(u32 dev_num)
{
	int status = MV_OK;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (DDR3_IS_ECC_PUP4_MODE(tm->bus_act_mask) ||
	    DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask) ||
	    DDR3_IS_ECC_PUP8_MODE(tm->bus_act_mask)) {
		if (DDR3_IS_ECC_PUP4_MODE(tm->bus_act_mask))
			status = mv_ddr4_dynamic_pb_wl_supp(dev_num, WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP4);
		else if (DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask))
			status = mv_ddr4_dynamic_pb_wl_supp(dev_num, WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP3);
		else /* WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP8 */
			status = mv_ddr4_dynamic_pb_wl_supp(dev_num, WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP8);
		if (status != MV_OK)
			return status;
		status = mv_ddr4_dynamic_pb_wl_supp(dev_num, WRITE_LEVELING_SUPP_ECC_MODE_DATA_PUPS);
	} else { /* regular supplementary for data subphys in non-ecc mode */
		status = mv_ddr4_dynamic_pb_wl_supp(dev_num, WRITE_LEVELING_SUPP_REG_MODE);
	}

	return status;
}

/* dynamic per bit write leveling supplementary */
static int mv_ddr4_dynamic_pb_wl_supp(u32 dev_num, enum mv_wl_supp_mode ecc_mode)
{
	u32 if_id;
	u32 subphy_start, subphy_end;
	u32 subphy_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	u8 compare_result = 0;
	u32 orig_phase;
	u32 rd_data, wr_data = 0;
	u32 flag, step;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 ecc_phy_access_id;
	int status;

	if (ecc_mode == WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP4 ||
	    ecc_mode == WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP3 ||
	    ecc_mode == WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP8) {
		/* enable ecc write mux */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
					   TRAINING_SW_2_REG, 0x100, 0x100);
		if (status != MV_OK)
			return status;

		/* disable read data ecc mux */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
					   TRAINING_SW_2_REG, 0x0, 0x3);
		if (status != MV_OK)
			return status;

		/* unset training start bit */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
					   TRAINING_REG, 0x0, 0x80000000);
		if (status != MV_OK)
			return status;

		if (ecc_mode == WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP3)
			ecc_phy_access_id = ECC_PHY_ACCESS_3;
		else if (ecc_mode == WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP4)
			ecc_phy_access_id = ECC_PHY_ACCESS_4;
		else /* ecc_mode == WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP8 */
			ecc_phy_access_id = ECC_PHY_ACCESS_8;

		subphy_start = ecc_phy_access_id;
		subphy_end = subphy_start + 1;
	} else if (ecc_mode == WRITE_LEVELING_SUPP_ECC_MODE_DATA_PUPS) {
		/* disable ecc write mux */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
					   TRAINING_SW_2_REG, 0x0, 0x100);
		if (status != MV_OK)
			return status;

		/* disable ecc mode*/
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
					   SDRAM_CFG_REG, 0, 0x40000);
		if (status != MV_OK)
			return status;

		subphy_start = 0;
		if (MV_DDR_IS_HALF_BUS_DRAM_MODE(tm->bus_act_mask, subphy_num))
			subphy_end = (subphy_num - 1) / 2;
		else
			subphy_end = subphy_num - 1;
	} else { /* ecc_mode == WRITE_LEVELING_SUPP_REG_MODE */
		subphy_start = 0;
		/* remove ecc subphy prior to algorithm's start */
		subphy_end = subphy_num - 1; /* TODO: check it */
	}

	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (subphy_num = subphy_start; subphy_num < subphy_end; subphy_num++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, subphy_num);
			flag = 1;
			step = 0;
			status = ddr3_tip_bus_read(dev_num, if_id, ACCESS_TYPE_UNICAST, subphy_num, DDR_PHY_DATA,
						   WL_PHY_REG(effective_cs), &rd_data);
			if (status != MV_OK)
				return status;
			orig_phase = (rd_data >> 6) & 0x7;
			while (flag != 0) {
				/* get decision for subphy */
				compare_result = mv_ddr4_xsb_comp_test(dev_num, subphy_num, if_id, ecc_mode);
				if (compare_result == MV_DDR4_COMP_TEST_RESULT_0) {
					flag = 0;
				} else { /* shift phase to -1 */
					step++;
					if (step == 1) { /* set phase (0x0[6-8]) to -2 */
						if (orig_phase > 1)
							wr_data = (rd_data & ~0x1c0) | ((orig_phase - 2) << 6);
						else if (orig_phase == 1)
							wr_data = (rd_data & ~0x1df);
						if (orig_phase >= 1)
							ddr3_tip_bus_write(dev_num, ACCESS_TYPE_UNICAST, if_id,
									   ACCESS_TYPE_UNICAST, subphy_num,
									   DDR_PHY_DATA,
									   WL_PHY_REG(effective_cs), wr_data);
					} else if (step == 2) { /* shift phase to +1 */
						if (orig_phase <= 5) {
							wr_data = (rd_data & ~0x1c0) | ((orig_phase + 2) << 6);
							ddr3_tip_bus_write(dev_num, ACCESS_TYPE_UNICAST, if_id,
									   ACCESS_TYPE_UNICAST, subphy_num,
									   DDR_PHY_DATA,
									   WL_PHY_REG(effective_cs), wr_data);
						}
					} else if (step == 3) {
						if (orig_phase <= 3) {
							wr_data = (rd_data & ~0x1c0) | ((orig_phase + 4) << 6);
							ddr3_tip_bus_write(dev_num, ACCESS_TYPE_UNICAST, if_id,
									   ACCESS_TYPE_UNICAST, subphy_num,
									   DDR_PHY_DATA,
									   WL_PHY_REG(effective_cs), wr_data);
						}
					} else { /* error */
						flag = 0;
						compare_result = MV_DDR4_COMP_TEST_NO_RESULT;
						training_result[training_stage][if_id] = TEST_FAILED;
					}
				}
			}
		}
		if ((training_result[training_stage][if_id] == NO_TEST_DONE) ||
		    (training_result[training_stage][if_id] == TEST_SUCCESS))
			training_result[training_stage][if_id] = TEST_SUCCESS;
	}

	if (ecc_mode == WRITE_LEVELING_SUPP_ECC_MODE_DATA_PUPS) {
		/* enable ecc write mux */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
					   TRAINING_SW_2_REG, 0x100, 0x100);
		if (status != MV_OK)
			return status;

		/* enable ecc mode*/
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
					   SDRAM_CFG_REG, 0x40000, 0x40000);
		if (status != MV_OK)
			return status;
	} else if (ecc_mode == WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP4 ||
		   ecc_mode == WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP3 ||
		   ecc_mode == WRITE_LEVELING_SUPP_ECC_MODE_ECC_PUP8) {
			/* enable ecc write mux */
			status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
						   TRAINING_SW_2_REG, 0x100, 0x100);
			if (status != MV_OK)
				return status;

			/* disable read data ecc mux */
			status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
						   TRAINING_SW_2_REG, 0x0, 0x3);
			if (status != MV_OK)
				return status;

			/* unset training start bit */
			status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
						   TRAINING_REG, 0x0, 0x80000000);
			if (status != MV_OK)
				return status;

			status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
						   TRAINING_SW_1_REG, 0x1 << 16, 0x1 << 16);
			if (status != MV_OK)
				return status;
	} else {
		/* do nothing for WRITE_LEVELING_SUPP_REG_MODE */;
	}
	if (training_result[training_stage][0] == TEST_SUCCESS)
		return MV_OK;
	else
		return MV_FAIL;
}
#endif /* CONFIG_DDR4 */
