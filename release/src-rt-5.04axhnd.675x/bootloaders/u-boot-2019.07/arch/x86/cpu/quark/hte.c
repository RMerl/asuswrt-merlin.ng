// SPDX-License-Identifier: Intel
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Ported from Intel released Quark UEFI BIOS
 * QuarkSocPkg/QuarkNorthCluster/MemoryInit/Pei
 */

#include <common.h>
#include <asm/arch/mrc.h>
#include <asm/arch/msg_port.h>
#include "mrc_util.h"
#include "hte.h"

/**
 * Enable HTE to detect all possible errors for the given training parameters
 * (per-bit or full byte lane).
 */
static void hte_enable_all_errors(void)
{
	msg_port_write(HTE, 0x000200a2, 0xffffffff);
	msg_port_write(HTE, 0x000200a3, 0x000000ff);
	msg_port_write(HTE, 0x000200a4, 0x00000000);
}

/**
 * Go and read the HTE register in order to find any error
 *
 * @return: The errors detected in the HTE status register
 */
static u32 hte_check_errors(void)
{
	return msg_port_read(HTE, 0x000200a7);
}

/**
 * Wait until HTE finishes
 */
static void hte_wait_for_complete(void)
{
	u32 tmp;

	ENTERFN();

	do {} while ((msg_port_read(HTE, 0x00020012) & (1 << 30)) != 0);

	tmp = msg_port_read(HTE, 0x00020011);
	tmp |= (1 << 9);
	tmp &= ~((1 << 12) | (1 << 13));
	msg_port_write(HTE, 0x00020011, tmp);

	LEAVEFN();
}

/**
 * Clear registers related with errors in the HTE
 */
static void hte_clear_error_regs(void)
{
	u32 tmp;

	/*
	 * Clear all HTE errors and enable error checking
	 * for burst and chunk.
	 */
	tmp = msg_port_read(HTE, 0x000200a1);
	tmp |= (1 << 8);
	msg_port_write(HTE, 0x000200a1, tmp);
}

/**
 * Execute a basic single-cache-line memory write/read/verify test using simple
 * constant pattern, different for READ_TRAIN and WRITE_TRAIN modes.
 *
 * See hte_basic_write_read() which is the external visible wrapper.
 *
 * @mrc_params: host structure for all MRC global data
 * @addr: memory adress being tested (must hit specific channel/rank)
 * @first_run: if set then the HTE registers are configured, otherwise it is
 *             assumed configuration is done and we just re-run the test
 * @mode: READ_TRAIN or WRITE_TRAIN (the difference is in the pattern)
 *
 * @return: byte lane failure on each bit (for Quark only bit0 and bit1)
 */
static u16 hte_basic_data_cmp(struct mrc_params *mrc_params, u32 addr,
			      u8 first_run, u8 mode)
{
	u32 pattern;
	u32 offset;

	if (first_run) {
		msg_port_write(HTE, 0x00020020, 0x01b10021);
		msg_port_write(HTE, 0x00020021, 0x06000000);
		msg_port_write(HTE, 0x00020022, addr >> 6);
		msg_port_write(HTE, 0x00020062, 0x00800015);
		msg_port_write(HTE, 0x00020063, 0xaaaaaaaa);
		msg_port_write(HTE, 0x00020064, 0xcccccccc);
		msg_port_write(HTE, 0x00020065, 0xf0f0f0f0);
		msg_port_write(HTE, 0x00020061, 0x00030008);

		if (mode == WRITE_TRAIN)
			pattern = 0xc33c0000;
		else /* READ_TRAIN */
			pattern = 0xaa5555aa;

		for (offset = 0x80; offset <= 0x8f; offset++)
			msg_port_write(HTE, offset, pattern);
	}

	msg_port_write(HTE, 0x000200a1, 0xffff1000);
	msg_port_write(HTE, 0x00020011, 0x00011000);
	msg_port_write(HTE, 0x00020011, 0x00011100);

	hte_wait_for_complete();

	/*
	 * Return bits 15:8 of HTE_CH0_ERR_XSTAT to check for
	 * any bytelane errors.
	 */
	return (hte_check_errors() >> 8) & 0xff;
}

/**
 * Examine a single-cache-line memory with write/read/verify test using multiple
 * data patterns (victim-aggressor algorithm).
 *
 * See hte_write_stress_bit_lanes() which is the external visible wrapper.
 *
 * @mrc_params: host structure for all MRC global data
 * @addr: memory adress being tested (must hit specific channel/rank)
 * @loop_cnt: number of test iterations
 * @seed_victim: victim data pattern seed
 * @seed_aggressor: aggressor data pattern seed
 * @victim_bit: should be 0 as auto-rotate feature is in use
 * @first_run: if set then the HTE registers are configured, otherwise it is
 *             assumed configuration is done and we just re-run the test
 *
 * @return: byte lane failure on each bit (for Quark only bit0 and bit1)
 */
static u16 hte_rw_data_cmp(struct mrc_params *mrc_params, u32 addr,
			   u8 loop_cnt, u32 seed_victim, u32 seed_aggressor,
			   u8 victim_bit, u8 first_run)
{
	u32 offset;
	u32 tmp;

	if (first_run) {
		msg_port_write(HTE, 0x00020020, 0x00910024);
		msg_port_write(HTE, 0x00020023, 0x00810024);
		msg_port_write(HTE, 0x00020021, 0x06070000);
		msg_port_write(HTE, 0x00020024, 0x06070000);
		msg_port_write(HTE, 0x00020022, addr >> 6);
		msg_port_write(HTE, 0x00020025, addr >> 6);
		msg_port_write(HTE, 0x00020062, 0x0000002a);
		msg_port_write(HTE, 0x00020063, seed_victim);
		msg_port_write(HTE, 0x00020064, seed_aggressor);
		msg_port_write(HTE, 0x00020065, seed_victim);

		/*
		 * Write the pattern buffers to select the victim bit
		 *
		 * Start with bit0
		 */
		for (offset = 0x80; offset <= 0x8f; offset++) {
			if ((offset % 8) == victim_bit)
				msg_port_write(HTE, offset, 0x55555555);
			else
				msg_port_write(HTE, offset, 0xcccccccc);
		}

		msg_port_write(HTE, 0x00020061, 0x00000000);
		msg_port_write(HTE, 0x00020066, 0x03440000);
		msg_port_write(HTE, 0x000200a1, 0xffff1000);
	}

	tmp = 0x10001000 | (loop_cnt << 16);
	msg_port_write(HTE, 0x00020011, tmp);
	msg_port_write(HTE, 0x00020011, tmp | (1 << 8));

	hte_wait_for_complete();

	/*
	 * Return bits 15:8 of HTE_CH0_ERR_XSTAT to check for
	 * any bytelane errors.
	 */
	return (hte_check_errors() >> 8) & 0xff;
}

/**
 * Use HW HTE engine to initialize or test all memory attached to a given DUNIT.
 * If flag is MRC_MEM_INIT, this routine writes 0s to all memory locations to
 * initialize ECC. If flag is MRC_MEM_TEST, this routine will send an 5AA55AA5
 * pattern to all memory locations on the RankMask and then read it back.
 * Then it sends an A55AA55A pattern to all memory locations on the RankMask
 * and reads it back.
 *
 * @mrc_params: host structure for all MRC global data
 * @flag: MRC_MEM_INIT or MRC_MEM_TEST
 *
 * @return: errors register showing HTE failures. Also prints out which rank
 *          failed the HTE test if failure occurs. For rank detection to work,
 *          the address map must be left in its default state. If MRC changes
 *          the address map, this function must be modified to change it back
 *          to default at the beginning, then restore it at the end.
 */
u32 hte_mem_init(struct mrc_params *mrc_params, u8 flag)
{
	u32 offset;
	int test_num;
	int i;

	/*
	 * Clear out the error registers at the start of each memory
	 * init or memory test run.
	 */
	hte_clear_error_regs();

	msg_port_write(HTE, 0x00020062, 0x00000015);

	for (offset = 0x80; offset <= 0x8f; offset++)
		msg_port_write(HTE, offset, ((offset & 1) ? 0xa55a : 0x5aa5));

	msg_port_write(HTE, 0x00020021, 0x00000000);
	msg_port_write(HTE, 0x00020022, (mrc_params->mem_size >> 6) - 1);
	msg_port_write(HTE, 0x00020063, 0xaaaaaaaa);
	msg_port_write(HTE, 0x00020064, 0xcccccccc);
	msg_port_write(HTE, 0x00020065, 0xf0f0f0f0);
	msg_port_write(HTE, 0x00020066, 0x03000000);

	switch (flag) {
	case MRC_MEM_INIT:
		/*
		 * Only 1 write pass through memory is needed
		 * to initialize ECC
		 */
		test_num = 1;
		break;
	case MRC_MEM_TEST:
		/* Write/read then write/read with inverted pattern */
		test_num = 4;
		break;
	default:
		DPF(D_INFO, "Unknown parameter for flag: %d\n", flag);
		return 0xffffffff;
	}

	DPF(D_INFO, "hte_mem_init");

	for (i = 0; i < test_num; i++) {
		DPF(D_INFO, ".");

		if (i == 0) {
			msg_port_write(HTE, 0x00020061, 0x00000000);
			msg_port_write(HTE, 0x00020020, 0x00110010);
		} else if (i == 1) {
			msg_port_write(HTE, 0x00020061, 0x00000000);
			msg_port_write(HTE, 0x00020020, 0x00010010);
		} else if (i == 2) {
			msg_port_write(HTE, 0x00020061, 0x00010100);
			msg_port_write(HTE, 0x00020020, 0x00110010);
		} else {
			msg_port_write(HTE, 0x00020061, 0x00010100);
			msg_port_write(HTE, 0x00020020, 0x00010010);
		}

		msg_port_write(HTE, 0x00020011, 0x00111000);
		msg_port_write(HTE, 0x00020011, 0x00111100);

		hte_wait_for_complete();

		/* If this is a READ pass, check for errors at the end */
		if ((i % 2) == 1) {
			/* Return immediately if error */
			if (hte_check_errors())
				break;
		}
	}

	DPF(D_INFO, "done\n");

	return hte_check_errors();
}

/**
 * Execute a basic single-cache-line memory write/read/verify test using simple
 * constant pattern, different for READ_TRAIN and WRITE_TRAIN modes.
 *
 * @mrc_params: host structure for all MRC global data
 * @addr: memory adress being tested (must hit specific channel/rank)
 * @first_run: if set then the HTE registers are configured, otherwise it is
 *             assumed configuration is done and we just re-run the test
 * @mode: READ_TRAIN or WRITE_TRAIN (the difference is in the pattern)
 *
 * @return: byte lane failure on each bit (for Quark only bit0 and bit1)
 */
u16 hte_basic_write_read(struct mrc_params *mrc_params, u32 addr,
			 u8 first_run, u8 mode)
{
	u16 errors;

	ENTERFN();

	/* Enable all error reporting in preparation for HTE test */
	hte_enable_all_errors();
	hte_clear_error_regs();

	errors = hte_basic_data_cmp(mrc_params, addr, first_run, mode);

	LEAVEFN();

	return errors;
}

/**
 * Examine a single-cache-line memory with write/read/verify test using multiple
 * data patterns (victim-aggressor algorithm).
 *
 * @mrc_params: host structure for all MRC global data
 * @addr: memory adress being tested (must hit specific channel/rank)
 * @first_run: if set then the HTE registers are configured, otherwise it is
 *             assumed configuration is done and we just re-run the test
 *
 * @return: byte lane failure on each bit (for Quark only bit0 and bit1)
 */
u16 hte_write_stress_bit_lanes(struct mrc_params *mrc_params,
			       u32 addr, u8 first_run)
{
	u16 errors;
	u8 victim_bit = 0;

	ENTERFN();

	/* Enable all error reporting in preparation for HTE test */
	hte_enable_all_errors();
	hte_clear_error_regs();

	/*
	 * Loop through each bit in the bytelane.
	 *
	 * Each pass creates a victim bit while keeping all other bits the same
	 * as aggressors. AVN HTE adds an auto-rotate feature which allows us
	 * to program the entire victim/aggressor sequence in 1 step.
	 *
	 * The victim bit rotates on each pass so no need to have software
	 * implement a victim bit loop like on VLV.
	 */
	errors = hte_rw_data_cmp(mrc_params, addr, HTE_LOOP_CNT,
				 HTE_LFSR_VICTIM_SEED, HTE_LFSR_AGRESSOR_SEED,
				 victim_bit, first_run);

	LEAVEFN();

	return errors;
}

/**
 * Execute a basic single-cache-line memory write or read.
 * This is just for receive enable / fine write-levelling purpose.
 *
 * @addr: memory adress being tested (must hit specific channel/rank)
 * @first_run: if set then the HTE registers are configured, otherwise it is
 *             assumed configuration is done and we just re-run the test
 * @is_write: when non-zero memory write operation executed, otherwise read
 */
void hte_mem_op(u32 addr, u8 first_run, u8 is_write)
{
	u32 offset;
	u32 tmp;

	hte_enable_all_errors();
	hte_clear_error_regs();

	if (first_run) {
		tmp = is_write ? 0x01110021 : 0x01010021;
		msg_port_write(HTE, 0x00020020, tmp);

		msg_port_write(HTE, 0x00020021, 0x06000000);
		msg_port_write(HTE, 0x00020022, addr >> 6);
		msg_port_write(HTE, 0x00020062, 0x00800015);
		msg_port_write(HTE, 0x00020063, 0xaaaaaaaa);
		msg_port_write(HTE, 0x00020064, 0xcccccccc);
		msg_port_write(HTE, 0x00020065, 0xf0f0f0f0);
		msg_port_write(HTE, 0x00020061, 0x00030008);

		for (offset = 0x80; offset <= 0x8f; offset++)
			msg_port_write(HTE, offset, 0xc33c0000);
	}

	msg_port_write(HTE, 0x000200a1, 0xffff1000);
	msg_port_write(HTE, 0x00020011, 0x00011000);
	msg_port_write(HTE, 0x00020011, 0x00011100);

	hte_wait_for_complete();
}
