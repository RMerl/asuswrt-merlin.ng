// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#include <command.h>
#include <dm.h>
#include <hang.h>
#include <i2c.h>
#include <ram.h>
#include <time.h>

#include <linux/bitops.h>
#include <linux/io.h>

#include <mach/octeon_ddr.h>

/* Random number generator stuff */

#define CVMX_OCT_DID_RNG	8ULL

static u64 cvmx_rng_get_random64(void)
{
	return csr_rd(cvmx_build_io_address(CVMX_OCT_DID_RNG, 0));
}

static void cvmx_rng_enable(void)
{
	u64 val;

	val = csr_rd(CVMX_RNM_CTL_STATUS);
	val |= BIT(0) | BIT(1);
	csr_wr(CVMX_RNM_CTL_STATUS, val);
}

#define RLEVEL_PRINTALL_DEFAULT		1
#define WLEVEL_PRINTALL_DEFAULT		1

/*
 * Define how many HW WL samples to take for majority voting.
 * MUST BE odd!!
 * Assume there should only be 2 possible values that will show up,
 * so treat ties as a problem!!!
 * NOTE: Do not change this without checking the code!!!
 */
#define WLEVEL_LOOPS_DEFAULT		5

#define ENABLE_COMPUTED_VREF_ADJUSTMENT	1
#define SW_WLEVEL_HW_DEFAULT		1
#define DEFAULT_BEST_RANK_SCORE		9999999
#define MAX_RANK_SCORE_LIMIT		99

/*
 * Define how many HW RL samples per rank to take multiple samples will
 * allow looking for the best sample score
 */
#define RLEVEL_SAMPLES_DEFAULT		3

#define ddr_seq_print(format, ...) do {} while (0)

struct wlevel_bitcnt {
	int bitcnt[4];
};

static void display_dac_dbi_settings(int lmc, int dac_or_dbi,
				     int ecc_ena, int *settings, char *title);

static unsigned short load_dac_override(struct ddr_priv *priv, int if_num,
					int dac_value, int byte);

/* "mode" arg */
#define DBTRAIN_TEST 0
#define DBTRAIN_DBI  1
#define DBTRAIN_LFSR 2

static int run_best_hw_patterns(struct ddr_priv *priv, int lmc, u64 phys_addr,
				int mode, u64 *xor_data);

#define LMC_DDR3_RESET_ASSERT   0
#define LMC_DDR3_RESET_DEASSERT 1

static void cn7xxx_lmc_ddr3_reset(struct ddr_priv *priv, int if_num, int reset)
{
	union cvmx_lmcx_reset_ctl reset_ctl;

	/*
	 * 4. Deassert DDRn_RESET_L pin by writing
	 *    LMC(0..3)_RESET_CTL[DDR3RST] = 1
	 *    without modifying any other LMC(0..3)_RESET_CTL fields.
	 * 5. Read LMC(0..3)_RESET_CTL and wait for the result.
	 * 6. Wait a minimum of 500us. This guarantees the necessary T = 500us
	 *    delay between DDRn_RESET_L deassertion and DDRn_DIMM*_CKE*
	 *    assertion.
	 */
	debug("LMC%d %s DDR_RESET_L\n", if_num,
	      (reset ==
	       LMC_DDR3_RESET_DEASSERT) ? "De-asserting" : "Asserting");

	reset_ctl.u64 = lmc_rd(priv, CVMX_LMCX_RESET_CTL(if_num));
	reset_ctl.cn78xx.ddr3rst = reset;
	lmc_wr(priv, CVMX_LMCX_RESET_CTL(if_num), reset_ctl.u64);

	lmc_rd(priv, CVMX_LMCX_RESET_CTL(if_num));

	udelay(500);
}

static void perform_lmc_reset(struct ddr_priv *priv, int node, int if_num)
{
	/*
	 * 5.9.6 LMC RESET Initialization
	 *
	 * The purpose of this step is to assert/deassert the RESET# pin at the
	 * DDR3/DDR4 parts.
	 *
	 * This LMC RESET step is done for all enabled LMCs.
	 *
	 * It may be appropriate to skip this step if the DDR3/DDR4 DRAM parts
	 * are in self refresh and are currently preserving their
	 * contents. (Software can determine this via
	 * LMC(0..3)_RESET_CTL[DDR3PSV] in some circumstances.) The remainder of
	 * this section assumes that the DRAM contents need not be preserved.
	 *
	 * The remainder of this section assumes that the CN78XX DDRn_RESET_L
	 * pin is attached to the RESET# pin of the attached DDR3/DDR4 parts,
	 * as will be appropriate in many systems.
	 *
	 * (In other systems, such as ones that can preserve DDR3/DDR4 part
	 * contents while CN78XX is powered down, it will not be appropriate to
	 * directly attach the CN78XX DDRn_RESET_L pin to DRESET# of the
	 * DDR3/DDR4 parts, and this section may not apply.)
	 *
	 * The remainder of this section describes the sequence for LMCn.
	 *
	 * Perform the following six substeps for LMC reset initialization:
	 *
	 * 1. If not done already, assert DDRn_RESET_L pin by writing
	 * LMC(0..3)_RESET_ CTL[DDR3RST] = 0 without modifying any other
	 * LMC(0..3)_RESET_CTL fields.
	 */

	if (!ddr_memory_preserved(priv)) {
		/*
		 * 2. Read LMC(0..3)_RESET_CTL and wait for the result.
		 */

		lmc_rd(priv, CVMX_LMCX_RESET_CTL(if_num));

		/*
		 * 3. Wait until RESET# assertion-time requirement from JEDEC
		 * DDR3/DDR4 specification is satisfied (200 us during a
		 * power-on ramp, 100ns when power is already stable).
		 */

		udelay(200);

		/*
		 * 4. Deassert DDRn_RESET_L pin by writing
		 *    LMC(0..3)_RESET_CTL[DDR3RST] = 1
		 *    without modifying any other LMC(0..3)_RESET_CTL fields.
		 * 5. Read LMC(0..3)_RESET_CTL and wait for the result.
		 * 6. Wait a minimum of 500us. This guarantees the necessary
		 *    T = 500us delay between DDRn_RESET_L deassertion and
		 *    DDRn_DIMM*_CKE* assertion.
		 */
		cn7xxx_lmc_ddr3_reset(priv, if_num, LMC_DDR3_RESET_DEASSERT);

		/* Toggle Reset Again */
		/* That is, assert, then de-assert, one more time */
		cn7xxx_lmc_ddr3_reset(priv, if_num, LMC_DDR3_RESET_ASSERT);
		cn7xxx_lmc_ddr3_reset(priv, if_num, LMC_DDR3_RESET_DEASSERT);
	}
}

void oct3_ddr3_seq(struct ddr_priv *priv, int rank_mask, int if_num,
		   int sequence)
{
	/*
	 * 3. Without changing any other fields in LMC(0)_CONFIG, write
	 *    LMC(0)_CONFIG[RANKMASK] then write both
	 *    LMC(0)_SEQ_CTL[SEQ_SEL,INIT_START] = 1 with a single CSR write
	 *    operation. LMC(0)_CONFIG[RANKMASK] bits should be set to indicate
	 *    the ranks that will participate in the sequence.
	 *
	 *    The LMC(0)_SEQ_CTL[SEQ_SEL] value should select power-up/init or
	 *    selfrefresh exit, depending on whether the DRAM parts are in
	 *    self-refresh and whether their contents should be preserved. While
	 *    LMC performs these sequences, it will not perform any other DDR3
	 *    transactions. When the sequence is complete, hardware sets the
	 *    LMC(0)_CONFIG[INIT_STATUS] bits for the ranks that have been
	 *    initialized.
	 *
	 *    If power-up/init is selected immediately following a DRESET
	 *    assertion, LMC executes the sequence described in the "Reset and
	 *    Initialization Procedure" section of the JEDEC DDR3
	 *    specification. This includes activating CKE, writing all four DDR3
	 *    mode registers on all selected ranks, and issuing the required
	 *    ZQCL
	 *    command. The LMC(0)_CONFIG[RANKMASK] value should select all ranks
	 *    with attached DRAM in this case. If LMC(0)_CONTROL[RDIMM_ENA] = 1,
	 *    LMC writes the JEDEC standard SSTE32882 control words selected by
	 *    LMC(0)_DIMM_CTL[DIMM*_WMASK] between DDR_CKE* signal assertion and
	 *    the first DDR3 mode register write operation.
	 *    LMC(0)_DIMM_CTL[DIMM*_WMASK] should be cleared to 0 if the
	 *    corresponding DIMM is not present.
	 *
	 *    If self-refresh exit is selected, LMC executes the required SRX
	 *    command followed by a refresh and ZQ calibration. Section 4.5
	 *    describes behavior of a REF + ZQCS.  LMC does not write the DDR3
	 *    mode registers as part of this sequence, and the mode register
	 *    parameters must match at self-refresh entry and exit times.
	 *
	 * 4. Read LMC(0)_SEQ_CTL and wait for LMC(0)_SEQ_CTL[SEQ_COMPLETE]
	 *    to be set.
	 *
	 * 5. Read LMC(0)_CONFIG[INIT_STATUS] and confirm that all ranks have
	 *    been initialized.
	 */

	union cvmx_lmcx_seq_ctl seq_ctl;
	union cvmx_lmcx_config lmc_config;
	int timeout;

	lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
	lmc_config.s.rankmask = rank_mask;
	lmc_wr(priv, CVMX_LMCX_CONFIG(if_num), lmc_config.u64);

	seq_ctl.u64 = 0;

	seq_ctl.s.init_start = 1;
	seq_ctl.s.seq_sel = sequence;

	ddr_seq_print
	    ("Performing LMC sequence: rank_mask=0x%02x, sequence=0x%x, %s\n",
	     rank_mask, sequence, sequence_str[sequence]);

	if (seq_ctl.s.seq_sel == 3)
		debug("LMC%d: Exiting Self-refresh Rank_mask:%x\n", if_num,
		      rank_mask);

	lmc_wr(priv, CVMX_LMCX_SEQ_CTL(if_num), seq_ctl.u64);
	lmc_rd(priv, CVMX_LMCX_SEQ_CTL(if_num));

	timeout = 100;
	do {
		udelay(100);	/* Wait a while */
		seq_ctl.u64 = lmc_rd(priv, CVMX_LMCX_SEQ_CTL(if_num));
		if (--timeout == 0) {
			printf("Sequence %d timed out\n", sequence);
			break;
		}
	} while (seq_ctl.s.seq_complete != 1);

	ddr_seq_print("           LMC sequence=%x: Completed.\n", sequence);
}

#define bdk_numa_get_address(n, p)	((p) | ((u64)n) << CVMX_NODE_MEM_SHIFT)
#define AREA_BASE_OFFSET		BIT_ULL(26)

static int test_dram_byte64(struct ddr_priv *priv, int lmc, u64 p,
			    u64 bitmask, u64 *xor_data)
{
	u64 p1, p2, d1, d2;
	u64 v, v1;
	u64 p2offset = (1ULL << 26);	// offset to area 2
	u64 datamask;
	u64 xor;
	u64 i, j, k;
	u64 ii;
	int errors = 0;
	//u64 index;
	u64 pattern1 = cvmx_rng_get_random64();
	u64 pattern2 = 0;
	u64 bad_bits[2] = { 0, 0 };
	int kbitno = (octeon_is_cpuid(OCTEON_CN7XXX)) ? 20 : 18;
	union cvmx_l2c_ctl l2c_ctl;
	int burst;
	int saved_dissblkdty;
	int node = 0;

	// Force full cacheline write-backs to boost traffic
	l2c_ctl.u64 = l2c_rd(priv, CVMX_L2C_CTL_REL);
	saved_dissblkdty = l2c_ctl.cn78xx.dissblkdty;
	l2c_ctl.cn78xx.dissblkdty = 1;
	l2c_wr(priv, CVMX_L2C_CTL_REL, l2c_ctl.u64);

	if (octeon_is_cpuid(OCTEON_CN73XX) || octeon_is_cpuid(OCTEON_CNF75XX))
		kbitno = 18;

	// Byte lanes may be clear in the mask to indicate no testing on that
	//lane.
	datamask = bitmask;

	/*
	 * Add offset to both test regions to not clobber boot stuff
	 * when running from L2 for NAND boot.
	 */
	p += AREA_BASE_OFFSET;	// make sure base is out of the way of boot

	// final address must include LMC and node
	p |= (lmc << 7);	/* Map address into proper interface */
	p = bdk_numa_get_address(node, p);	/* Map to node */
	p |= 1ull << 63;

#define II_INC BIT_ULL(22)
#define II_MAX BIT_ULL(22)
#define K_INC  BIT_ULL(14)
#define K_MAX  BIT_ULL(kbitno)
#define J_INC  BIT_ULL(9)
#define J_MAX  BIT_ULL(12)
#define I_INC  BIT_ULL(3)
#define I_MAX  BIT_ULL(7)

	debug("N%d.LMC%d: %s: phys_addr=0x%llx/0x%llx (0x%llx)\n",
	      node, lmc, __func__, p, p + p2offset, 1ULL << kbitno);

	// loops are ordered so that only a single 64-bit slot is written to
	// each cacheline at one time, then the cachelines are forced out;
	// this should maximize read/write traffic

	// FIXME? extend the range of memory tested!!
	for (ii = 0; ii < II_MAX; ii += II_INC) {
		for (i = 0; i < I_MAX; i += I_INC) {
			for (k = 0; k < K_MAX; k += K_INC) {
				for (j = 0; j < J_MAX; j += J_INC) {
					p1 = p + ii + k + j;
					p2 = p1 + p2offset;

					v = pattern1 * (p1 + i);
					// write the same thing to both areas
					v1 = v;

					cvmx_write64_uint64(p1 + i, v);
					cvmx_write64_uint64(p2 + i, v1);

					CVMX_CACHE_WBIL2(p1, 0);
					CVMX_CACHE_WBIL2(p2, 0);
				}
			}
		}
	}

	CVMX_DCACHE_INVALIDATE;

	debug("N%d.LMC%d: dram_tuning_mem_xor: done INIT loop\n", node, lmc);

	/* Make a series of passes over the memory areas. */

	for (burst = 0; burst < 1 /* was: dram_tune_use_bursts */ ; burst++) {
		u64 this_pattern = cvmx_rng_get_random64();

		pattern2 ^= this_pattern;

		/*
		 * XOR the data with a random value, applying the change to both
		 * memory areas.
		 */

		// FIXME? extend the range of memory tested!!
		for (ii = 0; ii < II_MAX; ii += II_INC) {
			// FIXME: rearranged, did not make much difference?
			for (i = 0; i < I_MAX; i += I_INC) {
				for (k = 0; k < K_MAX; k += K_INC) {
					for (j = 0; j < J_MAX; j += J_INC) {
						p1 = p + ii + k + j;
						p2 = p1 + p2offset;

						v = cvmx_read64_uint64(p1 +
								      i) ^
						    this_pattern;
						v1 = cvmx_read64_uint64(p2 +
								       i) ^
						    this_pattern;

						cvmx_write64_uint64(p1 + i, v);
						cvmx_write64_uint64(p2 + i, v1);

						CVMX_CACHE_WBIL2(p1, 0);
						CVMX_CACHE_WBIL2(p2, 0);
					}
				}
			}
		}

		CVMX_DCACHE_INVALIDATE;

		debug("N%d.LMC%d: dram_tuning_mem_xor: done MODIFY loop\n",
		      node, lmc);

		/*
		 * Look for differences in the areas. If there is a mismatch,
		 * reset both memory locations with the same pattern. Failing
		 * to do so means that on all subsequent passes the pair of
		 * locations remain out of sync giving spurious errors.
		 */

		// FIXME: Change the loop order so that an entire cache line
		//        is compared at one time. This is so that a read
		//        error that occurs *anywhere* on the cacheline will
		//        be caught, rather than comparing only 1 cacheline
		//        slot at a time, where an error on a different
		//        slot will be missed that time around
		// Does the above make sense?

		// FIXME? extend the range of memory tested!!
		for (ii = 0; ii < II_MAX; ii += II_INC) {
			for (k = 0; k < K_MAX; k += K_INC) {
				for (j = 0; j < J_MAX; j += J_INC) {
					p1 = p + ii + k + j;
					p2 = p1 + p2offset;

					// process entire cachelines in the
					//innermost loop
					for (i = 0; i < I_MAX; i += I_INC) {
						int bybit = 1;
						// start in byte lane 0
						u64 bymsk = 0xffULL;

						// FIXME: this should predict
						// what we find...???
						v = ((p1 + i) * pattern1) ^
							pattern2;
						d1 = cvmx_read64_uint64(p1 + i);
						d2 = cvmx_read64_uint64(p2 + i);

						// union of error bits only in
						// active byte lanes
						xor = ((d1 ^ v) | (d2 ^ v)) &
							datamask;

						if (!xor)
							continue;

						// accumulate bad bits
						bad_bits[0] |= xor;

						while (xor != 0) {
							debug("ERROR(%03d): [0x%016llX] [0x%016llX]  expected 0x%016llX d1 %016llX d2 %016llX\n",
							      burst, p1, p2, v,
							      d1, d2);
							// error(s) in this lane
							if (xor & bymsk) {
								// set the byte
								// error bit
								errors |= bybit;
								// clear byte
								// lane in
								// error bits
								xor &= ~bymsk;
								// clear the
								// byte lane in
								// the mask
								datamask &= ~bymsk;
#if EXIT_WHEN_ALL_LANES_HAVE_ERRORS
								// nothing
								// left to do
								if (datamask == 0) {
									return errors;
								}
#endif /* EXIT_WHEN_ALL_LANES_HAVE_ERRORS */
							}
							// move mask into
							// next byte lane
							bymsk <<= 8;
							// move bit into next
							// byte position
							bybit <<= 1;
						}
					}
					CVMX_CACHE_WBIL2(p1, 0);
					CVMX_CACHE_WBIL2(p2, 0);
				}
			}
		}

		debug("N%d.LMC%d: dram_tuning_mem_xor: done TEST loop\n",
		      node, lmc);
	}

	if (xor_data) {		// send the bad bits back...
		xor_data[0] = bad_bits[0];
		xor_data[1] = bad_bits[1];	// let it be zeroed
	}

	// Restore original setting that could enable partial cacheline writes
	l2c_ctl.u64 = l2c_rd(priv, CVMX_L2C_CTL_REL);
	l2c_ctl.cn78xx.dissblkdty = saved_dissblkdty;
	l2c_wr(priv, CVMX_L2C_CTL_REL, l2c_ctl.u64);

	return errors;
}

static void ddr4_mrw(struct ddr_priv *priv, int if_num, int rank,
		     int mr_wr_addr, int mr_wr_sel, int mr_wr_bg1)
{
	union cvmx_lmcx_mr_mpr_ctl lmc_mr_mpr_ctl;

	lmc_mr_mpr_ctl.u64 = 0;
	lmc_mr_mpr_ctl.cn78xx.mr_wr_addr = (mr_wr_addr == -1) ? 0 : mr_wr_addr;
	lmc_mr_mpr_ctl.cn78xx.mr_wr_sel = mr_wr_sel;
	lmc_mr_mpr_ctl.cn78xx.mr_wr_rank = rank;
	lmc_mr_mpr_ctl.cn78xx.mr_wr_use_default_value =
		(mr_wr_addr == -1) ? 1 : 0;
	lmc_mr_mpr_ctl.cn78xx.mr_wr_bg1 = mr_wr_bg1;
	lmc_wr(priv, CVMX_LMCX_MR_MPR_CTL(if_num), lmc_mr_mpr_ctl.u64);

	/* Mode Register Write */
	oct3_ddr3_seq(priv, 1 << rank, if_num, 0x8);
}

#define INV_A0_17(x)	((x) ^ 0x22bf8)

static void set_mpr_mode(struct ddr_priv *priv, int rank_mask,
			 int if_num, int dimm_count, int mpr, int bg1)
{
	int rankx;

	debug("All Ranks: Set mpr mode = %x %c-side\n",
	      mpr, (bg1 == 0) ? 'A' : 'B');

	for (rankx = 0; rankx < dimm_count * 4; rankx++) {
		if (!(rank_mask & (1 << rankx)))
			continue;
		if (bg1 == 0) {
			/* MR3 A-side */
			ddr4_mrw(priv, if_num, rankx, mpr << 2, 3, bg1);
		} else {
			/* MR3 B-side */
			ddr4_mrw(priv, if_num, rankx, INV_A0_17(mpr << 2), ~3,
				 bg1);
		}
	}
}

static void do_ddr4_mpr_read(struct ddr_priv *priv, int if_num,
			     int rank, int page, int location)
{
	union cvmx_lmcx_mr_mpr_ctl lmc_mr_mpr_ctl;

	lmc_mr_mpr_ctl.u64 = lmc_rd(priv, CVMX_LMCX_MR_MPR_CTL(if_num));
	lmc_mr_mpr_ctl.cn70xx.mr_wr_addr = 0;
	lmc_mr_mpr_ctl.cn70xx.mr_wr_sel = page;	/* Page */
	lmc_mr_mpr_ctl.cn70xx.mr_wr_rank = rank;
	lmc_mr_mpr_ctl.cn70xx.mpr_loc = location;
	lmc_mr_mpr_ctl.cn70xx.mpr_wr = 0;	/* Read=0, Write=1 */
	lmc_wr(priv, CVMX_LMCX_MR_MPR_CTL(if_num), lmc_mr_mpr_ctl.u64);

	/* MPR register access sequence */
	oct3_ddr3_seq(priv, 1 << rank, if_num, 0x9);

	debug("LMC_MR_MPR_CTL                  : 0x%016llx\n",
	      lmc_mr_mpr_ctl.u64);
	debug("lmc_mr_mpr_ctl.cn70xx.mr_wr_addr: 0x%02x\n",
	      lmc_mr_mpr_ctl.cn70xx.mr_wr_addr);
	debug("lmc_mr_mpr_ctl.cn70xx.mr_wr_sel : 0x%02x\n",
	      lmc_mr_mpr_ctl.cn70xx.mr_wr_sel);
	debug("lmc_mr_mpr_ctl.cn70xx.mpr_loc   : 0x%02x\n",
	      lmc_mr_mpr_ctl.cn70xx.mpr_loc);
	debug("lmc_mr_mpr_ctl.cn70xx.mpr_wr    : 0x%02x\n",
	      lmc_mr_mpr_ctl.cn70xx.mpr_wr);
}

static int set_rdimm_mode(struct ddr_priv *priv, int if_num, int enable)
{
	union cvmx_lmcx_control lmc_control;
	int save_rdimm_mode;

	lmc_control.u64 = lmc_rd(priv, CVMX_LMCX_CONTROL(if_num));
	save_rdimm_mode = lmc_control.s.rdimm_ena;
	lmc_control.s.rdimm_ena = enable;
	debug("Setting RDIMM_ENA = %x\n", enable);
	lmc_wr(priv, CVMX_LMCX_CONTROL(if_num), lmc_control.u64);

	return save_rdimm_mode;
}

static void ddr4_mpr_read(struct ddr_priv *priv, int if_num, int rank,
			  int page, int location, u64 *mpr_data)
{
	do_ddr4_mpr_read(priv, if_num, rank, page, location);

	mpr_data[0] = lmc_rd(priv, CVMX_LMCX_MPR_DATA0(if_num));
}

/* Display MPR values for Page */
static void display_mpr_page(struct ddr_priv *priv, int rank_mask,
			     int if_num, int page)
{
	int rankx, location;
	u64 mpr_data[3];

	for (rankx = 0; rankx < 4; rankx++) {
		if (!(rank_mask & (1 << rankx)))
			continue;

		debug("N0.LMC%d.R%d: MPR Page %d loc [0:3]: ",
		      if_num, rankx, page);
		for (location = 0; location < 4; location++) {
			ddr4_mpr_read(priv, if_num, rankx, page, location,
				      mpr_data);
			debug("0x%02llx ", mpr_data[0] & 0xFF);
		}
		debug("\n");

	}			/* for (rankx = 0; rankx < 4; rankx++) */
}

static void ddr4_mpr_write(struct ddr_priv *priv, int if_num, int rank,
			   int page, int location, u8 mpr_data)
{
	union cvmx_lmcx_mr_mpr_ctl lmc_mr_mpr_ctl;

	lmc_mr_mpr_ctl.u64 = 0;
	lmc_mr_mpr_ctl.cn70xx.mr_wr_addr = mpr_data;
	lmc_mr_mpr_ctl.cn70xx.mr_wr_sel = page;	/* Page */
	lmc_mr_mpr_ctl.cn70xx.mr_wr_rank = rank;
	lmc_mr_mpr_ctl.cn70xx.mpr_loc = location;
	lmc_mr_mpr_ctl.cn70xx.mpr_wr = 1;	/* Read=0, Write=1 */
	lmc_wr(priv, CVMX_LMCX_MR_MPR_CTL(if_num), lmc_mr_mpr_ctl.u64);

	/* MPR register access sequence */
	oct3_ddr3_seq(priv, 1 << rank, if_num, 0x9);

	debug("LMC_MR_MPR_CTL                  : 0x%016llx\n",
	      lmc_mr_mpr_ctl.u64);
	debug("lmc_mr_mpr_ctl.cn70xx.mr_wr_addr: 0x%02x\n",
	      lmc_mr_mpr_ctl.cn70xx.mr_wr_addr);
	debug("lmc_mr_mpr_ctl.cn70xx.mr_wr_sel : 0x%02x\n",
	      lmc_mr_mpr_ctl.cn70xx.mr_wr_sel);
	debug("lmc_mr_mpr_ctl.cn70xx.mpr_loc   : 0x%02x\n",
	      lmc_mr_mpr_ctl.cn70xx.mpr_loc);
	debug("lmc_mr_mpr_ctl.cn70xx.mpr_wr    : 0x%02x\n",
	      lmc_mr_mpr_ctl.cn70xx.mpr_wr);
}

static void set_vref(struct ddr_priv *priv, int if_num, int rank,
		     int range, int value)
{
	union cvmx_lmcx_mr_mpr_ctl lmc_mr_mpr_ctl;
	union cvmx_lmcx_modereg_params3 lmc_modereg_params3;
	int mr_wr_addr = 0;

	lmc_mr_mpr_ctl.u64 = 0;
	lmc_modereg_params3.u64 = lmc_rd(priv,
					 CVMX_LMCX_MODEREG_PARAMS3(if_num));

	/* A12:A10 tCCD_L */
	mr_wr_addr |= lmc_modereg_params3.s.tccd_l << 10;
	mr_wr_addr |= 1 << 7;	/* A7 1 = Enable(Training Mode) */
	mr_wr_addr |= range << 6;	/* A6 vrefDQ Training Range */
	mr_wr_addr |= value << 0;	/* A5:A0 vrefDQ Training Value */

	lmc_mr_mpr_ctl.cn70xx.mr_wr_addr = mr_wr_addr;
	lmc_mr_mpr_ctl.cn70xx.mr_wr_sel = 6;	/* Write MR6 */
	lmc_mr_mpr_ctl.cn70xx.mr_wr_rank = rank;
	lmc_wr(priv, CVMX_LMCX_MR_MPR_CTL(if_num), lmc_mr_mpr_ctl.u64);

	/* 0x8 = Mode Register Write */
	oct3_ddr3_seq(priv, 1 << rank, if_num, 0x8);

	/*
	 * It is vendor specific whether vref_value is captured with A7=1.
	 * A subsequent MRS might be necessary.
	 */
	oct3_ddr3_seq(priv, 1 << rank, if_num, 0x8);

	mr_wr_addr &= ~(1 << 7);	/* A7 0 = Disable(Training Mode) */
	lmc_mr_mpr_ctl.cn70xx.mr_wr_addr = mr_wr_addr;
	lmc_wr(priv, CVMX_LMCX_MR_MPR_CTL(if_num), lmc_mr_mpr_ctl.u64);
}

static void set_dram_output_inversion(struct ddr_priv *priv, int if_num,
				      int dimm_count, int rank_mask,
				      int inversion)
{
	union cvmx_lmcx_ddr4_dimm_ctl lmc_ddr4_dimm_ctl;
	union cvmx_lmcx_dimmx_params lmc_dimmx_params;
	union cvmx_lmcx_dimm_ctl lmc_dimm_ctl;
	int dimm_no;

	/* Don't touch extenced register control words */
	lmc_ddr4_dimm_ctl.u64 = 0;
	lmc_wr(priv, CVMX_LMCX_DDR4_DIMM_CTL(if_num), lmc_ddr4_dimm_ctl.u64);

	debug("All DIMMs: Register Control Word          RC0 : %x\n",
	      (inversion & 1));

	for (dimm_no = 0; dimm_no < dimm_count; ++dimm_no) {
		lmc_dimmx_params.u64 =
		    lmc_rd(priv, CVMX_LMCX_DIMMX_PARAMS(dimm_no, if_num));
		lmc_dimmx_params.s.rc0 =
		    (lmc_dimmx_params.s.rc0 & ~1) | (inversion & 1);

		lmc_wr(priv,
		       CVMX_LMCX_DIMMX_PARAMS(dimm_no, if_num),
		       lmc_dimmx_params.u64);
	}

	/* LMC0_DIMM_CTL */
	lmc_dimm_ctl.u64 = lmc_rd(priv, CVMX_LMCX_DIMM_CTL(if_num));
	lmc_dimm_ctl.s.dimm0_wmask = 0x1;
	lmc_dimm_ctl.s.dimm1_wmask = (dimm_count > 1) ? 0x0001 : 0x0000;

	debug("LMC DIMM_CTL                                  : 0x%016llx\n",
	      lmc_dimm_ctl.u64);
	lmc_wr(priv, CVMX_LMCX_DIMM_CTL(if_num), lmc_dimm_ctl.u64);

	oct3_ddr3_seq(priv, rank_mask, if_num, 0x7);	/* Init RCW */
}

static void write_mpr_page0_pattern(struct ddr_priv *priv, int rank_mask,
				    int if_num, int dimm_count, int pattern,
				    int location_mask)
{
	int rankx;
	int location;

	for (rankx = 0; rankx < dimm_count * 4; rankx++) {
		if (!(rank_mask & (1 << rankx)))
			continue;
		for (location = 0; location < 4; ++location) {
			if (!(location_mask & (1 << location)))
				continue;

			ddr4_mpr_write(priv, if_num, rankx,
				       /* page */ 0, /* location */ location,
				       pattern);
		}
	}
}

static void change_rdimm_mpr_pattern(struct ddr_priv *priv, int rank_mask,
				     int if_num, int dimm_count)
{
	int save_ref_zqcs_int;
	union cvmx_lmcx_config lmc_config;

	/*
	 * Okay, here is the latest sequence.  This should work for all
	 * chips and passes (78,88,73,etc).  This sequence should be run
	 * immediately after DRAM INIT.  The basic idea is to write the
	 * same pattern into each of the 4 MPR locations in the DRAM, so
	 * that the same value is returned when doing MPR reads regardless
	 * of the inversion state.  My advice is to put this into a
	 * function, change_rdimm_mpr_pattern or something like that, so
	 * that it can be called multiple times, as I think David wants a
	 * clock-like pattern for OFFSET training, but does not want a
	 * clock pattern for Bit-Deskew.  You should then be able to call
	 * this at any point in the init sequence (after DRAM init) to
	 * change the pattern to a new value.
	 * Mike
	 *
	 * A correction: PHY doesn't need any pattern during offset
	 * training, but needs clock like pattern for internal vref and
	 * bit-dskew training.  So for that reason, these steps below have
	 * to be conducted before those trainings to pre-condition
	 * the pattern.  David
	 *
	 * Note: Step 3, 4, 8 and 9 have to be done through RDIMM
	 * sequence. If you issue MRW sequence to do RCW write (in o78 pass
	 * 1 at least), LMC will still do two commands because
	 * CONTROL[RDIMM_ENA] is still set high. We don't want it to have
	 * any unintentional mode register write so it's best to do what
	 * Mike is doing here.
	 * Andrew
	 */

	/* 1) Disable refresh (REF_ZQCS_INT = 0) */

	debug("1) Disable refresh (REF_ZQCS_INT = 0)\n");

	lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
	save_ref_zqcs_int = lmc_config.cn78xx.ref_zqcs_int;
	lmc_config.cn78xx.ref_zqcs_int = 0;
	lmc_wr(priv, CVMX_LMCX_CONFIG(if_num), lmc_config.u64);

	/*
	 * 2) Put all devices in MPR mode (Run MRW sequence (sequence=8)
	 * with MODEREG_PARAMS0[MPRLOC]=0,
	 * MODEREG_PARAMS0[MPR]=1, MR_MPR_CTL[MR_WR_SEL]=3, and
	 * MR_MPR_CTL[MR_WR_USE_DEFAULT_VALUE]=1)
	 */

	debug("2) Put all devices in MPR mode (Run MRW sequence (sequence=8)\n");

	/* A-side */
	set_mpr_mode(priv, rank_mask, if_num, dimm_count, 1, 0);
	/* B-side */
	set_mpr_mode(priv, rank_mask, if_num, dimm_count, 1, 1);

	/*
	 * a. Or you can set MR_MPR_CTL[MR_WR_USE_DEFAULT_VALUE]=0 and set
	 * the value you would like directly into
	 * MR_MPR_CTL[MR_WR_ADDR]
	 */

	/*
	 * 3) Disable RCD Parity (if previously enabled) - parity does not
	 * work if inversion disabled
	 */

	debug("3) Disable RCD Parity\n");

	/*
	 * 4) Disable Inversion in the RCD.
	 * a. I did (3&4) via the RDIMM sequence (seq_sel=7), but it
	 * may be easier to use the MRW sequence (seq_sel=8).  Just set
	 * MR_MPR_CTL[MR_WR_SEL]=7, MR_MPR_CTL[MR_WR_ADDR][3:0]=data,
	 * MR_MPR_CTL[MR_WR_ADDR][7:4]=RCD reg
	 */

	debug("4) Disable Inversion in the RCD.\n");

	set_dram_output_inversion(priv, if_num, dimm_count, rank_mask, 1);

	/*
	 * 5) Disable CONTROL[RDIMM_ENA] so that MR sequence goes out
	 * non-inverted.
	 */

	debug("5) Disable CONTROL[RDIMM_ENA]\n");

	set_rdimm_mode(priv, if_num, 0);

	/*
	 * 6) Write all 4 MPR registers with the desired pattern (have to
	 * do this for all enabled ranks)
	 * a. MR_MPR_CTL.MPR_WR=1, MR_MPR_CTL.MPR_LOC=0..3,
	 * MR_MPR_CTL.MR_WR_SEL=0, MR_MPR_CTL.MR_WR_ADDR[7:0]=pattern
	 */

	debug("6) Write all 4 MPR page 0 Training Patterns\n");

	write_mpr_page0_pattern(priv, rank_mask, if_num, dimm_count, 0x55, 0x8);

	/* 7) Re-enable RDIMM_ENA */

	debug("7) Re-enable RDIMM_ENA\n");

	set_rdimm_mode(priv, if_num, 1);

	/* 8) Re-enable RDIMM inversion */

	debug("8) Re-enable RDIMM inversion\n");

	set_dram_output_inversion(priv, if_num, dimm_count, rank_mask, 0);

	/* 9) Re-enable RDIMM parity (if desired) */

	debug("9) Re-enable RDIMM parity (if desired)\n");

	/*
	 * 10)Take B-side devices out of MPR mode (Run MRW sequence
	 * (sequence=8) with MODEREG_PARAMS0[MPRLOC]=0,
	 * MODEREG_PARAMS0[MPR]=0, MR_MPR_CTL[MR_WR_SEL]=3, and
	 * MR_MPR_CTL[MR_WR_USE_DEFAULT_VALUE]=1)
	 */

	debug("10)Take B-side devices out of MPR mode\n");

	set_mpr_mode(priv, rank_mask, if_num, dimm_count,
		     /* mpr */ 0, /* bg1 */ 1);

	/*
	 * a. Or you can set MR_MPR_CTL[MR_WR_USE_DEFAULT_VALUE]=0 and
	 * set the value you would like directly into MR_MPR_CTL[MR_WR_ADDR]
	 */

	/* 11)Re-enable refresh (REF_ZQCS_INT=previous value) */

	debug("11)Re-enable refresh (REF_ZQCS_INT=previous value)\n");

	lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
	lmc_config.cn78xx.ref_zqcs_int = save_ref_zqcs_int;
	lmc_wr(priv, CVMX_LMCX_CONFIG(if_num), lmc_config.u64);
}

static int validate_hwl_seq(int *wl, int *seq)
{
	// sequence index, step through the sequence array
	int seqx;
	int bitnum;

	seqx = 0;

	while (seq[seqx + 1] >= 0) {	// stop on next seq entry == -1
		// but now, check current versus next
		bitnum = (wl[seq[seqx]] << 2) | wl[seq[seqx + 1]];
		// magic validity number (see matrix above)
		if (!((1 << bitnum) & 0xBDE7))
			return 1;
		seqx++;
	}

	return 0;
}

static int validate_hw_wl_settings(int if_num,
				   union cvmx_lmcx_wlevel_rankx
				   *lmc_wlevel_rank, int is_rdimm, int ecc_ena)
{
	int wl[9], byte, errors;

	// arrange the sequences so
	// index 0 has byte 0, etc, ECC in middle
	int useq[] = { 0, 1, 2, 3, 8, 4, 5, 6, 7, -1 };
	// index 0 is ECC, then go down
	int rseq1[] = { 8, 3, 2, 1, 0, -1 };
	// index 0 has byte 4, then go up
	int rseq2[] = { 4, 5, 6, 7, -1 };
	// index 0 has byte 0, etc, no ECC
	int useqno[] = { 0, 1, 2, 3, 4, 5, 6, 7, -1 };
	// index 0 is byte 3, then go down, no ECC
	int rseq1no[] = { 3, 2, 1, 0, -1 };

	// in the CSR, bytes 0-7 are always data, byte 8 is ECC
	for (byte = 0; byte < (8 + ecc_ena); byte++) {
		// preprocess :-)
		wl[byte] = (get_wl_rank(lmc_wlevel_rank, byte) >>
			    1) & 3;
	}

	errors = 0;
	if (is_rdimm) {		// RDIMM order
		errors = validate_hwl_seq(wl, (ecc_ena) ? rseq1 : rseq1no);
		errors += validate_hwl_seq(wl, rseq2);
	} else {		// UDIMM order
		errors = validate_hwl_seq(wl, (ecc_ena) ? useq : useqno);
	}

	return errors;
}

static unsigned int extr_wr(u64 u, int x)
{
	return (unsigned int)(((u >> (x * 12 + 5)) & 0x3ULL) |
			      ((u >> (51 + x - 2)) & 0x4ULL));
}

static void insrt_wr(u64 *up, int x, int v)
{
	u64 u = *up;

	u &= ~(((0x3ULL) << (x * 12 + 5)) | ((0x1ULL) << (51 + x)));
	*up = (u | ((v & 0x3ULL) << (x * 12 + 5)) |
	       ((v & 0x4ULL) << (51 + x - 2)));
}

/* Read out Deskew Settings for DDR */

struct deskew_bytes {
	u16 bits[8];
};

struct deskew_data {
	struct deskew_bytes bytes[9];
};

struct dac_data {
	int bytes[9];
};

// T88 pass 1, skip 4=DAC
static const u8 dsk_bit_seq_p1[8] = { 0, 1, 2, 3, 5, 6, 7, 8 };
// T88 Pass 2, skip 4=DAC and 5=DBI
static const u8 dsk_bit_seq_p2[8] = { 0, 1, 2, 3, 6, 7, 8, 9 };

static void get_deskew_settings(struct ddr_priv *priv, int if_num,
				struct deskew_data *dskdat)
{
	union cvmx_lmcx_phy_ctl phy_ctl;
	union cvmx_lmcx_config lmc_config;
	int bit_index;
	int byte_lane, byte_limit;
	// NOTE: these are for pass 2.x
	int is_o78p2 = !octeon_is_cpuid(OCTEON_CN78XX_PASS1_X);
	const u8 *bit_seq = (is_o78p2) ? dsk_bit_seq_p2 : dsk_bit_seq_p1;

	lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
	byte_limit = ((!lmc_config.s.mode32b) ? 8 : 4) + lmc_config.s.ecc_ena;

	memset(dskdat, 0, sizeof(*dskdat));

	phy_ctl.u64 = lmc_rd(priv, CVMX_LMCX_PHY_CTL(if_num));
	phy_ctl.s.dsk_dbg_clk_scaler = 3;

	for (byte_lane = 0; byte_lane < byte_limit; byte_lane++) {
		phy_ctl.s.dsk_dbg_byte_sel = byte_lane;	// set byte lane

		for (bit_index = 0; bit_index < 8; ++bit_index) {
			// set bit number and start read sequence
			phy_ctl.s.dsk_dbg_bit_sel = bit_seq[bit_index];
			phy_ctl.s.dsk_dbg_rd_start = 1;
			lmc_wr(priv, CVMX_LMCX_PHY_CTL(if_num), phy_ctl.u64);

			// poll for read sequence to complete
			do {
				phy_ctl.u64 =
					lmc_rd(priv, CVMX_LMCX_PHY_CTL(if_num));
			} while (phy_ctl.s.dsk_dbg_rd_complete != 1);

			// record the data
			dskdat->bytes[byte_lane].bits[bit_index] =
				phy_ctl.s.dsk_dbg_rd_data & 0x3ff;
		}
	}
}

static void display_deskew_settings(struct ddr_priv *priv, int if_num,
				    struct deskew_data *dskdat,
				    int print_enable)
{
	int byte_lane;
	int bit_num;
	u16 flags, deskew;
	union cvmx_lmcx_config lmc_config;
	int byte_limit;
	const char *fc = " ?-=+*#&";

	lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
	byte_limit = ((lmc_config.s.mode32b) ? 4 : 8) + lmc_config.s.ecc_ena;

	if (print_enable) {
		debug("N0.LMC%d: Deskew Data:              Bit =>      :",
		      if_num);
		for (bit_num = 7; bit_num >= 0; --bit_num)
			debug(" %3d  ", bit_num);
		debug("\n");
	}

	for (byte_lane = 0; byte_lane < byte_limit; byte_lane++) {
		if (print_enable)
			debug("N0.LMC%d: Bit Deskew Byte %d %s               :",
			      if_num, byte_lane,
			      (print_enable >= 3) ? "FINAL" : "     ");

		for (bit_num = 7; bit_num >= 0; --bit_num) {
			flags = dskdat->bytes[byte_lane].bits[bit_num] & 7;
			deskew = dskdat->bytes[byte_lane].bits[bit_num] >> 3;

			if (print_enable)
				debug(" %3d %c", deskew, fc[flags ^ 1]);

		}		/* for (bit_num = 7; bit_num >= 0; --bit_num) */

		if (print_enable)
			debug("\n");
	}
}

static void override_deskew_settings(struct ddr_priv *priv, int if_num,
				     struct deskew_data *dskdat)
{
	union cvmx_lmcx_phy_ctl phy_ctl;
	union cvmx_lmcx_config lmc_config;

	int bit, byte_lane, byte_limit;
	u64 csr_data;

	lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
	byte_limit = ((lmc_config.s.mode32b) ? 4 : 8) + lmc_config.s.ecc_ena;

	phy_ctl.u64 = lmc_rd(priv, CVMX_LMCX_PHY_CTL(if_num));

	phy_ctl.s.phy_reset = 0;
	phy_ctl.s.dsk_dbg_num_bits_sel = 1;
	phy_ctl.s.dsk_dbg_offset = 0;
	phy_ctl.s.dsk_dbg_clk_scaler = 3;

	phy_ctl.s.dsk_dbg_wr_mode = 1;
	phy_ctl.s.dsk_dbg_load_dis = 0;
	phy_ctl.s.dsk_dbg_overwrt_ena = 0;

	phy_ctl.s.phy_dsk_reset = 0;

	lmc_wr(priv, CVMX_LMCX_PHY_CTL(if_num), phy_ctl.u64);
	lmc_rd(priv, CVMX_LMCX_PHY_CTL(if_num));

	for (byte_lane = 0; byte_lane < byte_limit; byte_lane++) {
		csr_data = 0;
		// FIXME: can we ignore DBI?
		for (bit = 0; bit < 8; ++bit) {
			// fetch input and adjust
			u64 bits = (dskdat->bytes[byte_lane].bits[bit] >> 3) &
				0x7F;

			/*
			 * lmc_general_purpose0.data[6:0]    // DQ0
			 * lmc_general_purpose0.data[13:7]   // DQ1
			 * lmc_general_purpose0.data[20:14]  // DQ2
			 * lmc_general_purpose0.data[27:21]  // DQ3
			 * lmc_general_purpose0.data[34:28]  // DQ4
			 * lmc_general_purpose0.data[41:35]  // DQ5
			 * lmc_general_purpose0.data[48:42]  // DQ6
			 * lmc_general_purpose0.data[55:49]  // DQ7
			 * lmc_general_purpose0.data[62:56]  // DBI
			 */
			csr_data |= (bits << (7 * bit));

		} /* for (bit = 0; bit < 8; ++bit) */

		// update GP0 with the bit data for this byte lane
		lmc_wr(priv, CVMX_LMCX_GENERAL_PURPOSE0(if_num), csr_data);
		lmc_rd(priv, CVMX_LMCX_GENERAL_PURPOSE0(if_num));

		// start the deskew load sequence
		phy_ctl.s.dsk_dbg_byte_sel = byte_lane;
		phy_ctl.s.dsk_dbg_rd_start = 1;
		lmc_wr(priv, CVMX_LMCX_PHY_CTL(if_num), phy_ctl.u64);

		// poll for read sequence to complete
		do {
			udelay(100);
			phy_ctl.u64 = lmc_rd(priv, CVMX_LMCX_PHY_CTL(if_num));
		} while (phy_ctl.s.dsk_dbg_rd_complete != 1);
	}

	// tell phy to use the new settings
	phy_ctl.s.dsk_dbg_overwrt_ena = 1;
	phy_ctl.s.dsk_dbg_rd_start = 0;
	lmc_wr(priv, CVMX_LMCX_PHY_CTL(if_num), phy_ctl.u64);

	phy_ctl.s.dsk_dbg_wr_mode = 0;
	lmc_wr(priv, CVMX_LMCX_PHY_CTL(if_num), phy_ctl.u64);
}

static void process_by_rank_dac(struct ddr_priv *priv, int if_num,
				int rank_mask, struct dac_data *dacdat)
{
	union cvmx_lmcx_config lmc_config;
	int rankx, byte_lane;
	int byte_limit;
	int rank_count;
	struct dac_data dacsum;
	int lane_probs;

	lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
	byte_limit = ((lmc_config.s.mode32b) ? 4 : 8) + lmc_config.s.ecc_ena;

	memset((void *)&dacsum, 0, sizeof(dacsum));
	rank_count = 0;
	lane_probs = 0;

	for (rankx = 0; rankx < 4; rankx++) {
		if (!(rank_mask & (1 << rankx)))
			continue;
		rank_count++;

		display_dac_dbi_settings(if_num, /*dac */ 1,
					 lmc_config.s.ecc_ena,
					 &dacdat[rankx].bytes[0],
					 "By-Ranks VREF");
		// sum
		for (byte_lane = 0; byte_lane < byte_limit; byte_lane++) {
			if (rank_count == 2) {
				int ranks_diff =
				    abs((dacsum.bytes[byte_lane] -
					 dacdat[rankx].bytes[byte_lane]));

				// FIXME: is 19 a good number?
				if (ranks_diff > 19)
					lane_probs |= (1 << byte_lane);
			}
			dacsum.bytes[byte_lane] +=
			    dacdat[rankx].bytes[byte_lane];
		}
	}

	// average
	for (byte_lane = 0; byte_lane < byte_limit; byte_lane++)
		dacsum.bytes[byte_lane] /= rank_count;	// FIXME: nint?

	display_dac_dbi_settings(if_num, /*dac */ 1, lmc_config.s.ecc_ena,
				 &dacsum.bytes[0], "All-Rank VREF");

	if (lane_probs) {
		debug("N0.LMC%d: All-Rank VREF DAC Problem Bytelane(s): 0x%03x\n",
		      if_num, lane_probs);
	}

	// finally, write the averaged DAC values
	for (byte_lane = 0; byte_lane < byte_limit; byte_lane++) {
		load_dac_override(priv, if_num, dacsum.bytes[byte_lane],
				  byte_lane);
	}
}

static void process_by_rank_dsk(struct ddr_priv *priv, int if_num,
				int rank_mask, struct deskew_data *dskdat)
{
	union cvmx_lmcx_config lmc_config;
	int rankx, lane, bit;
	int byte_limit;
	struct deskew_data dsksum, dskcnt;
	u16 deskew;

	lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
	byte_limit = ((lmc_config.s.mode32b) ? 4 : 8) + lmc_config.s.ecc_ena;

	memset((void *)&dsksum, 0, sizeof(dsksum));
	memset((void *)&dskcnt, 0, sizeof(dskcnt));

	for (rankx = 0; rankx < 4; rankx++) {
		if (!(rank_mask & (1 << rankx)))
			continue;

		// sum ranks
		for (lane = 0; lane < byte_limit; lane++) {
			for (bit = 0; bit < 8; ++bit) {
				deskew = dskdat[rankx].bytes[lane].bits[bit];
				// if flags indicate sat hi or lo, skip it
				if (deskew & 6)
					continue;

				// clear flags
				dsksum.bytes[lane].bits[bit] +=
					deskew & ~7;
				// count entries
				dskcnt.bytes[lane].bits[bit] += 1;
			}
		}
	}

	// average ranks
	for (lane = 0; lane < byte_limit; lane++) {
		for (bit = 0; bit < 8; ++bit) {
			int div = dskcnt.bytes[lane].bits[bit];

			if (div > 0) {
				dsksum.bytes[lane].bits[bit] /= div;
				// clear flags
				dsksum.bytes[lane].bits[bit] &= ~7;
				// set LOCK
				dsksum.bytes[lane].bits[bit] |= 1;
			} else {
				// FIXME? use reset value?
				dsksum.bytes[lane].bits[bit] =
					(64 << 3) | 1;
			}
		}
	}

	// TME for FINAL version
	display_deskew_settings(priv, if_num, &dsksum, /*VBL_TME */ 3);

	// finally, write the averaged DESKEW values
	override_deskew_settings(priv, if_num, &dsksum);
}

struct deskew_counts {
	int saturated;		// number saturated
	int unlocked;		// number unlocked
	int nibrng_errs;	// nibble range errors
	int nibunl_errs;	// nibble unlocked errors
	int bitval_errs;	// bit value errors
};

#define MIN_BITVAL  17
#define MAX_BITVAL 110

static void validate_deskew_training(struct ddr_priv *priv, int rank_mask,
				     int if_num, struct deskew_counts *counts,
				     int print_flags)
{
	int byte_lane, bit_index, nib_num;
	int nibrng_errs, nibunl_errs, bitval_errs;
	union cvmx_lmcx_config lmc_config;
	s16 nib_min[2], nib_max[2], nib_unl[2];
	int byte_limit;
	int print_enable = print_flags & 1;
	struct deskew_data dskdat;
	s16 flags, deskew;
	const char *fc = " ?-=+*#&";
	int bit_last;

	lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
	byte_limit = ((!lmc_config.s.mode32b) ? 8 : 4) + lmc_config.s.ecc_ena;

	memset(counts, 0, sizeof(struct deskew_counts));

	get_deskew_settings(priv, if_num, &dskdat);

	if (print_enable) {
		debug("N0.LMC%d: Deskew Settings:          Bit =>      :",
		      if_num);
		for (bit_index = 7; bit_index >= 0; --bit_index)
			debug(" %3d  ", bit_index);
		debug("\n");
	}

	for (byte_lane = 0; byte_lane < byte_limit; byte_lane++) {
		if (print_enable)
			debug("N0.LMC%d: Bit Deskew Byte %d %s               :",
			      if_num, byte_lane,
			      (print_flags & 2) ? "FINAL" : "     ");

		nib_min[0] = 127;
		nib_min[1] = 127;
		nib_max[0] = 0;
		nib_max[1] = 0;
		nib_unl[0] = 0;
		nib_unl[1] = 0;

		if (lmc_config.s.mode32b == 1 && byte_lane == 4) {
			bit_last = 3;
			if (print_enable)
				debug("                        ");
		} else {
			bit_last = 7;
		}

		for (bit_index = bit_last; bit_index >= 0; --bit_index) {
			nib_num = (bit_index > 3) ? 1 : 0;

			flags = dskdat.bytes[byte_lane].bits[bit_index] & 7;
			deskew = dskdat.bytes[byte_lane].bits[bit_index] >> 3;

			counts->saturated += !!(flags & 6);

			// Do range calc even when locked; it could happen
			// that a bit is still unlocked after final retry,
			// and we want to have an external retry if a RANGE
			// error is present at exit...
			nib_min[nib_num] = min(nib_min[nib_num], deskew);
			nib_max[nib_num] = max(nib_max[nib_num], deskew);

			if (!(flags & 1)) {	// only when not locked
				counts->unlocked += 1;
				nib_unl[nib_num] += 1;
			}

			if (print_enable)
				debug(" %3d %c", deskew, fc[flags ^ 1]);
		}

		/*
		 * Now look for nibble errors
		 *
		 * For bit 55, it looks like a bit deskew problem. When the
		 * upper nibble of byte 6 needs to go to saturation, bit 7
		 * of byte 6 locks prematurely at 64. For DIMMs with raw
		 * card A and B, can we reset the deskew training when we
		 * encounter this case? The reset criteria should be looking
		 * at one nibble at a time for raw card A and B; if the
		 * bit-deskew setting within a nibble is different by > 33,
		 * we'll issue a reset to the bit deskew training.
		 *
		 * LMC0 Bit Deskew Byte(6): 64 0 - 0 - 0 - 26 61 35 64
		 */
		// upper nibble range, then lower nibble range
		nibrng_errs = ((nib_max[1] - nib_min[1]) > 33) ? 1 : 0;
		nibrng_errs |= ((nib_max[0] - nib_min[0]) > 33) ? 1 : 0;

		// check for nibble all unlocked
		nibunl_errs = ((nib_unl[0] == 4) || (nib_unl[1] == 4)) ? 1 : 0;

		// check for bit value errors, ie < 17 or > 110
		// FIXME? assume max always > MIN_BITVAL and min < MAX_BITVAL
		bitval_errs = ((nib_max[1] > MAX_BITVAL) ||
			       (nib_max[0] > MAX_BITVAL)) ? 1 : 0;
		bitval_errs |= ((nib_min[1] < MIN_BITVAL) ||
				(nib_min[0] < MIN_BITVAL)) ? 1 : 0;

		if ((nibrng_errs != 0 || nibunl_errs != 0 ||
		     bitval_errs != 0) && print_enable) {
			debug(" %c%c%c",
			      (nibrng_errs) ? 'R' : ' ',
			      (nibunl_errs) ? 'U' : ' ',
			      (bitval_errs) ? 'V' : ' ');
		}

		if (print_enable)
			debug("\n");

		counts->nibrng_errs |= (nibrng_errs << byte_lane);
		counts->nibunl_errs |= (nibunl_errs << byte_lane);
		counts->bitval_errs |= (bitval_errs << byte_lane);
	}
}

static unsigned short load_dac_override(struct ddr_priv *priv, int if_num,
					int dac_value, int byte)
{
	union cvmx_lmcx_dll_ctl3 ddr_dll_ctl3;
	// single bytelanes incr by 1; A is for ALL
	int bytex = (byte == 0x0A) ? byte : byte + 1;

	ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(if_num));

	SET_DDR_DLL_CTL3(byte_sel, bytex);
	SET_DDR_DLL_CTL3(offset, dac_value >> 1);

	ddr_dll_ctl3.cn73xx.bit_select = 0x9;	/* No-op */
	lmc_wr(priv, CVMX_LMCX_DLL_CTL3(if_num), ddr_dll_ctl3.u64);

	ddr_dll_ctl3.cn73xx.bit_select = 0xC;	/* vref bypass setting load */
	lmc_wr(priv, CVMX_LMCX_DLL_CTL3(if_num), ddr_dll_ctl3.u64);

	ddr_dll_ctl3.cn73xx.bit_select = 0xD;	/* vref bypass on. */
	lmc_wr(priv, CVMX_LMCX_DLL_CTL3(if_num), ddr_dll_ctl3.u64);

	ddr_dll_ctl3.cn73xx.bit_select = 0x9;	/* No-op */
	lmc_wr(priv, CVMX_LMCX_DLL_CTL3(if_num), ddr_dll_ctl3.u64);

	lmc_rd(priv, CVMX_LMCX_DLL_CTL3(if_num));	// flush writes

	return (unsigned short)GET_DDR_DLL_CTL3(offset);
}

// arg dac_or_dbi is 1 for DAC, 0 for DBI
// returns 9 entries (bytelanes 0 through 8) in settings[]
// returns 0 if OK, -1 if a problem
static int read_dac_dbi_settings(struct ddr_priv *priv, int if_num,
				 int dac_or_dbi, int *settings)
{
	union cvmx_lmcx_phy_ctl phy_ctl;
	int byte_lane, bit_num;
	int deskew;
	int dac_value;
	int new_deskew_layout = 0;

	new_deskew_layout = octeon_is_cpuid(OCTEON_CN73XX) ||
		octeon_is_cpuid(OCTEON_CNF75XX);
	new_deskew_layout |= (octeon_is_cpuid(OCTEON_CN78XX) &&
			      !octeon_is_cpuid(OCTEON_CN78XX_PASS1_X));

	phy_ctl.u64 = lmc_rd(priv, CVMX_LMCX_PHY_CTL(if_num));
	phy_ctl.s.dsk_dbg_clk_scaler = 3;
	lmc_wr(priv, CVMX_LMCX_PHY_CTL(if_num), phy_ctl.u64);

	bit_num = (dac_or_dbi) ? 4 : 5;
	// DBI not available
	if (bit_num == 5 && !new_deskew_layout)
		return -1;

	// FIXME: always assume ECC is available
	for (byte_lane = 8; byte_lane >= 0; --byte_lane) {
		//set byte lane and bit to read
		phy_ctl.s.dsk_dbg_bit_sel = bit_num;
		phy_ctl.s.dsk_dbg_byte_sel = byte_lane;
		lmc_wr(priv, CVMX_LMCX_PHY_CTL(if_num), phy_ctl.u64);

		//start read sequence
		phy_ctl.u64 = lmc_rd(priv, CVMX_LMCX_PHY_CTL(if_num));
		phy_ctl.s.dsk_dbg_rd_start = 1;
		lmc_wr(priv, CVMX_LMCX_PHY_CTL(if_num), phy_ctl.u64);

		//poll for read sequence to complete
		do {
			phy_ctl.u64 = lmc_rd(priv, CVMX_LMCX_PHY_CTL(if_num));
		} while (phy_ctl.s.dsk_dbg_rd_complete != 1);

		// keep the flag bits where they are for DBI
		deskew = phy_ctl.s.dsk_dbg_rd_data; /* >> 3 */
		dac_value = phy_ctl.s.dsk_dbg_rd_data & 0xff;

		settings[byte_lane] = (dac_or_dbi) ? dac_value : deskew;
	}

	return 0;
}

// print out the DBI settings array
// arg dac_or_dbi is 1 for DAC, 0 for DBI
static void display_dac_dbi_settings(int lmc, int dac_or_dbi,
				     int ecc_ena, int *settings, char *title)
{
	int byte;
	int flags;
	int deskew;
	const char *fc = " ?-=+*#&";

	debug("N0.LMC%d: %s %s Settings %d:0 :",
	      lmc, title, (dac_or_dbi) ? "DAC" : "DBI", 7 + ecc_ena);
	// FIXME: what about 32-bit mode?
	for (byte = (7 + ecc_ena); byte >= 0; --byte) {
		if (dac_or_dbi) {	// DAC
			flags = 1;	// say its locked to get blank
			deskew = settings[byte] & 0xff;
		} else {	// DBI
			flags = settings[byte] & 7;
			deskew = (settings[byte] >> 3) & 0x7f;
		}
		debug(" %3d %c", deskew, fc[flags ^ 1]);
	}
	debug("\n");
}

// Find a HWL majority
static int find_wl_majority(struct wlevel_bitcnt *bc, int *mx, int *mc,
			    int *xc, int *cc)
{
	int ix, ic;

	*mx = -1;
	*mc = 0;
	*xc = 0;
	*cc = 0;

	for (ix = 0; ix < 4; ix++) {
		ic = bc->bitcnt[ix];

		// make a bitmask of the ones with a count
		if (ic > 0) {
			*mc |= (1 << ix);
			*cc += 1;	// count how many had non-zero counts
		}

		// find the majority
		if (ic > *xc) {	// new max?
			*xc = ic;	// yes
			*mx = ix;	// set its index
		}
	}

	return (*mx << 1);
}

// Evaluate the DAC settings array
static int evaluate_dac_settings(int if_64b, int ecc_ena, int *settings)
{
	int byte, lane, dac, comp;
	int last = (if_64b) ? 7 : 3;

	// FIXME: change the check...???
	// this looks only for sets of DAC values whose max/min differ by a lot
	// let any EVEN go so long as it is within range...
	for (byte = (last + ecc_ena); byte >= 0; --byte) {
		dac = settings[byte] & 0xff;

		for (lane = (last + ecc_ena); lane >= 0; --lane) {
			comp = settings[lane] & 0xff;
			if (abs((dac - comp)) > 25)
				return 1;
		}
	}

	return 0;
}

static void perform_offset_training(struct ddr_priv *priv, int rank_mask,
				    int if_num)
{
	union cvmx_lmcx_phy_ctl lmc_phy_ctl;
	u64 orig_phy_ctl;
	const char *s;

	/*
	 * 4.8.6 LMC Offset Training
	 *
	 * LMC requires input-receiver offset training.
	 *
	 * 1. Write LMC(0)_PHY_CTL[DAC_ON] = 1
	 */
	lmc_phy_ctl.u64 = lmc_rd(priv, CVMX_LMCX_PHY_CTL(if_num));
	orig_phy_ctl = lmc_phy_ctl.u64;
	lmc_phy_ctl.s.dac_on = 1;

	// allow full CSR override
	s = lookup_env_ull(priv, "ddr_phy_ctl");
	if (s)
		lmc_phy_ctl.u64 = strtoull(s, NULL, 0);

	// do not print or write if CSR does not change...
	if (lmc_phy_ctl.u64 != orig_phy_ctl) {
		debug("PHY_CTL                                       : 0x%016llx\n",
		      lmc_phy_ctl.u64);
		lmc_wr(priv, CVMX_LMCX_PHY_CTL(if_num), lmc_phy_ctl.u64);
	}

	/*
	 * 2. Write LMC(0)_SEQ_CTL[SEQ_SEL] = 0x0B and
	 *    LMC(0)_SEQ_CTL[INIT_START] = 1.
	 *
	 * 3. Wait for LMC(0)_SEQ_CTL[SEQ_COMPLETE] to be set to 1.
	 */
	/* Start Offset training sequence */
	oct3_ddr3_seq(priv, rank_mask, if_num, 0x0B);
}

static void perform_internal_vref_training(struct ddr_priv *priv,
					   int rank_mask, int if_num)
{
	union cvmx_lmcx_ext_config ext_config;
	union cvmx_lmcx_dll_ctl3 ddr_dll_ctl3;

	// First, make sure all byte-lanes are out of VREF bypass mode
	ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(if_num));

	ddr_dll_ctl3.cn78xx.byte_sel = 0x0A;	/* all byte-lanes */
	ddr_dll_ctl3.cn78xx.bit_select = 0x09;	/* No-op */
	lmc_wr(priv, CVMX_LMCX_DLL_CTL3(if_num), ddr_dll_ctl3.u64);

	ddr_dll_ctl3.cn78xx.bit_select = 0x0E;	/* vref bypass off. */
	lmc_wr(priv, CVMX_LMCX_DLL_CTL3(if_num), ddr_dll_ctl3.u64);

	ddr_dll_ctl3.cn78xx.bit_select = 0x09;	/* No-op */
	lmc_wr(priv, CVMX_LMCX_DLL_CTL3(if_num), ddr_dll_ctl3.u64);

	/*
	 * 4.8.7 LMC Internal vref Training
	 *
	 * LMC requires input-reference-voltage training.
	 *
	 * 1. Write LMC(0)_EXT_CONFIG[VREFINT_SEQ_DESKEW] = 0.
	 */
	ext_config.u64 = lmc_rd(priv, CVMX_LMCX_EXT_CONFIG(if_num));
	ext_config.s.vrefint_seq_deskew = 0;

	ddr_seq_print("Performing LMC sequence: vrefint_seq_deskew = %d\n",
		      ext_config.s.vrefint_seq_deskew);

	lmc_wr(priv, CVMX_LMCX_EXT_CONFIG(if_num), ext_config.u64);

	/*
	 * 2. Write LMC(0)_SEQ_CTL[SEQ_SEL] = 0x0a and
	 *    LMC(0)_SEQ_CTL[INIT_START] = 1.
	 *
	 * 3. Wait for LMC(0)_SEQ_CTL[SEQ_COMPLETE] to be set to 1.
	 */
	/* Start LMC Internal vref Training */
	oct3_ddr3_seq(priv, rank_mask, if_num, 0x0A);
}

#define dbg_avg(format, ...)	// debug(format, ##__VA_ARGS__)

static int process_samples_average(s16 *bytes, int num_samples,
				   int lmc, int lane_no)
{
	int i, sadj, sum = 0, ret, asum, trunc;
	s16 smin = 32767, smax = -32768;
	int nmin, nmax;
	//int rng;

	dbg_avg("DBG_AVG%d.%d: ", lmc, lane_no);

	for (i = 0; i < num_samples; i++) {
		sum += bytes[i];
		if (bytes[i] < smin)
			smin = bytes[i];
		if (bytes[i] > smax)
			smax = bytes[i];
		dbg_avg(" %3d", bytes[i]);
	}

	nmin = 0;
	nmax = 0;
	for (i = 0; i < num_samples; i++) {
		if (bytes[i] == smin)
			nmin += 1;
		if (bytes[i] == smax)
			nmax += 1;
	}
	dbg_avg(" (min=%3d/%d, max=%3d/%d, range=%2d, samples=%2d)",
		smin, nmin, smax, nmax, rng, num_samples);

	asum = sum - smin - smax;

	sadj = divide_nint(asum * 10, (num_samples - 2));

	trunc = asum / (num_samples - 2);

	dbg_avg(" [%3d.%d, %3d]", sadj / 10, sadj % 10, trunc);

	sadj = divide_nint(sadj, 10);
	if (trunc & 1)
		ret = trunc;
	else if (sadj & 1)
		ret = sadj;
	else
		ret = trunc + 1;

	dbg_avg(" -> %3d\n", ret);

	return ret;
}

#define DEFAULT_SAT_RETRY_LIMIT    11	// 1 + 10 retries

#define default_lock_retry_limit   20	// 20 retries
#define deskew_validation_delay    10000	// 10 millisecs

static int perform_deskew_training(struct ddr_priv *priv, int rank_mask,
				   int if_num, int spd_rawcard_aorb)
{
	int unsaturated, locked;
	int sat_retries, sat_retries_limit;
	int lock_retries, lock_retries_total, lock_retries_limit;
	int print_first;
	int print_them_all;
	struct deskew_counts dsk_counts;
	union cvmx_lmcx_phy_ctl phy_ctl;
	char *s;
	int has_no_sat = octeon_is_cpuid(OCTEON_CN78XX_PASS2_X) ||
		octeon_is_cpuid(OCTEON_CNF75XX);
	int disable_bitval_retries = 1;	// default to disabled

	debug("N0.LMC%d: Performing Deskew Training.\n", if_num);

	sat_retries = 0;
	sat_retries_limit = (has_no_sat) ? 5 : DEFAULT_SAT_RETRY_LIMIT;

	lock_retries_total = 0;
	unsaturated = 0;
	print_first = 1;	// print the first one
	// set to true for printing all normal deskew attempts
	print_them_all = 0;

	// provide override for bitval_errs causing internal VREF retries
	s = env_get("ddr_disable_bitval_retries");
	if (s)
		disable_bitval_retries = !!simple_strtoul(s, NULL, 0);

	lock_retries_limit = default_lock_retry_limit;
	if ((octeon_is_cpuid(OCTEON_CN78XX_PASS2_X)) ||
	    (octeon_is_cpuid(OCTEON_CN73XX)) ||
	    (octeon_is_cpuid(OCTEON_CNF75XX)))
		lock_retries_limit *= 2;	// give new chips twice as many

	do {			/* while (sat_retries < sat_retry_limit) */
		/*
		 * 4.8.8 LMC Deskew Training
		 *
		 * LMC requires input-read-data deskew training.
		 *
		 * 1. Write LMC(0)_EXT_CONFIG[VREFINT_SEQ_DESKEW] = 1.
		 */

		union cvmx_lmcx_ext_config ext_config;

		ext_config.u64 = lmc_rd(priv, CVMX_LMCX_EXT_CONFIG(if_num));
		ext_config.s.vrefint_seq_deskew = 1;

		ddr_seq_print
		    ("Performing LMC sequence: vrefint_seq_deskew = %d\n",
		     ext_config.s.vrefint_seq_deskew);

		lmc_wr(priv, CVMX_LMCX_EXT_CONFIG(if_num), ext_config.u64);

		/*
		 * 2. Write LMC(0)_SEQ_CTL[SEQ_SEL] = 0x0A and
		 *    LMC(0)_SEQ_CTL[INIT_START] = 1.
		 *
		 * 3. Wait for LMC(0)_SEQ_CTL[SEQ_COMPLETE] to be set to 1.
		 */

		phy_ctl.u64 = lmc_rd(priv, CVMX_LMCX_PHY_CTL(if_num));
		phy_ctl.s.phy_dsk_reset = 1;	/* RESET Deskew sequence */
		lmc_wr(priv, CVMX_LMCX_PHY_CTL(if_num), phy_ctl.u64);

		/* LMC Deskew Training */
		oct3_ddr3_seq(priv, rank_mask, if_num, 0x0A);

		lock_retries = 0;

perform_deskew_training:

		phy_ctl.u64 = lmc_rd(priv, CVMX_LMCX_PHY_CTL(if_num));
		phy_ctl.s.phy_dsk_reset = 0;	/* Normal Deskew sequence */
		lmc_wr(priv, CVMX_LMCX_PHY_CTL(if_num), phy_ctl.u64);

		/* LMC Deskew Training */
		oct3_ddr3_seq(priv, rank_mask, if_num, 0x0A);

		// Moved this from validate_deskew_training
		/* Allow deskew results to stabilize before evaluating them. */
		udelay(deskew_validation_delay);

		// Now go look at lock and saturation status...
		validate_deskew_training(priv, rank_mask, if_num, &dsk_counts,
					 print_first);
		// after printing the first and not doing them all, no more
		if (print_first && !print_them_all)
			print_first = 0;

		unsaturated = (dsk_counts.saturated == 0);
		locked = (dsk_counts.unlocked == 0);

		// only do locking retries if unsaturated or rawcard A or B,
		// otherwise full SAT retry
		if (unsaturated || (spd_rawcard_aorb && !has_no_sat)) {
			if (!locked) {	// and not locked
				lock_retries++;
				lock_retries_total++;
				if (lock_retries <= lock_retries_limit) {
					goto perform_deskew_training;
				} else {
					debug("N0.LMC%d: LOCK RETRIES failed after %d retries\n",
					      if_num, lock_retries_limit);
				}
			} else {
				// only print if we did try
				if (lock_retries_total > 0)
					debug("N0.LMC%d: LOCK RETRIES successful after %d retries\n",
					      if_num, lock_retries);
			}
		}		/* if (unsaturated || spd_rawcard_aorb) */

		++sat_retries;

		/*
		 * At this point, check for a DDR4 RDIMM that will not
		 * benefit from SAT retries; if so, exit
		 */
		if (spd_rawcard_aorb && !has_no_sat) {
			debug("N0.LMC%d: Deskew Training Loop: Exiting for RAWCARD == A or B.\n",
			      if_num);
			break;	// no sat or lock retries
		}

	} while (!unsaturated && (sat_retries < sat_retries_limit));

	debug("N0.LMC%d: Deskew Training %s. %d sat-retries, %d lock-retries\n",
	      if_num, (sat_retries >= DEFAULT_SAT_RETRY_LIMIT) ?
	      "Timed Out" : "Completed", sat_retries - 1, lock_retries_total);

	// FIXME? add saturation to reasons for fault return - give it a
	// chance via Internal VREF
	// FIXME? add OPTIONAL bit value to reasons for fault return -
	// give it a chance via Internal VREF
	if (dsk_counts.nibrng_errs != 0 || dsk_counts.nibunl_errs != 0 ||
	    (dsk_counts.bitval_errs != 0 && !disable_bitval_retries) ||
	    !unsaturated) {
		debug("N0.LMC%d: Nibble or Saturation Error(s) found, returning FAULT\n",
		      if_num);
		// FIXME: do we want this output always for errors?
		validate_deskew_training(priv, rank_mask, if_num,
					 &dsk_counts, 1);
		return -1;	// we did retry locally, they did not help
	}

	// NOTE: we (currently) always print one last training validation
	// before starting Read Leveling...

	return 0;
}

#define SCALING_FACTOR (1000)

// NOTE: this gets called for 1-rank and 2-rank DIMMs in single-slot config
static int compute_vref_1slot_2rank(int rtt_wr, int rtt_park, int dqx_ctl,
				    int rank_count, int dram_connection)
{
	u64 reff_s;
	u64 rser_s = (dram_connection) ? 0 : 15;
	u64 vdd = 1200;
	u64 vref;
	// 99 == HiZ
	u64 rtt_wr_s = (((rtt_wr == 0) || rtt_wr == 99) ?
			1 * 1024 * 1024 : rtt_wr);
	u64 rtt_park_s = (((rtt_park == 0) || ((rank_count == 1) &&
					       (rtt_wr != 0))) ?
			  1 * 1024 * 1024 : rtt_park);
	u64 dqx_ctl_s = (dqx_ctl == 0 ? 1 * 1024 * 1024 : dqx_ctl);
	int vref_value;
	u64 rangepc = 6000;	// range1 base
	u64 vrefpc;
	int vref_range = 0;

	reff_s = divide_nint((rtt_wr_s * rtt_park_s), (rtt_wr_s + rtt_park_s));

	vref = (((rser_s + dqx_ctl_s) * SCALING_FACTOR) /
		(rser_s + dqx_ctl_s + reff_s)) + SCALING_FACTOR;

	vref = (vref * vdd) / 2 / SCALING_FACTOR;

	vrefpc = (vref * 100 * 100) / vdd;

	if (vrefpc < rangepc) {	// < range1 base, use range2
		vref_range = 1 << 6;	// set bit A6 for range2
		rangepc = 4500;	// range2 base is 45%
	}

	vref_value = divide_nint(vrefpc - rangepc, 65);
	if (vref_value < 0)
		vref_value = vref_range;	// set to base of range
	else
		vref_value |= vref_range;

	debug("rtt_wr: %d, rtt_park: %d, dqx_ctl: %d, rank_count: %d\n",
	      rtt_wr, rtt_park, dqx_ctl, rank_count);
	debug("rtt_wr_s: %lld, rtt_park_s: %lld, dqx_ctl_s: %lld, vref_value: 0x%x, range: %d\n",
	      rtt_wr_s, rtt_park_s, dqx_ctl_s, vref_value ^ vref_range,
	      vref_range ? 2 : 1);

	return vref_value;
}

// NOTE: this gets called for 1-rank and 2-rank DIMMs in two-slot configs
static int compute_vref_2slot_2rank(int rtt_wr, int rtt_park_00,
				    int rtt_park_01,
				    int dqx_ctl, int rtt_nom,
				    int dram_connection)
{
	u64 rser = (dram_connection) ? 0 : 15;
	u64 vdd = 1200;
	u64 vl, vlp, vcm;
	u64 rd0, rd1, rpullup;
	// 99 == HiZ
	u64 rtt_wr_s = (((rtt_wr == 0) || rtt_wr == 99) ?
			1 * 1024 * 1024 : rtt_wr);
	u64 rtt_park_00_s = (rtt_park_00 == 0 ? 1 * 1024 * 1024 : rtt_park_00);
	u64 rtt_park_01_s = (rtt_park_01 == 0 ? 1 * 1024 * 1024 : rtt_park_01);
	u64 dqx_ctl_s = (dqx_ctl == 0 ? 1 * 1024 * 1024 : dqx_ctl);
	u64 rtt_nom_s = (rtt_nom == 0 ? 1 * 1024 * 1024 : rtt_nom);
	int vref_value;
	u64 rangepc = 6000;	// range1 base
	u64 vrefpc;
	int vref_range = 0;

	// rd0 = (RTT_NOM (parallel) RTT_WR) +  =
	// ((RTT_NOM * RTT_WR) / (RTT_NOM + RTT_WR)) + RSER
	rd0 = divide_nint((rtt_nom_s * rtt_wr_s),
			  (rtt_nom_s + rtt_wr_s)) + rser;

	// rd1 = (RTT_PARK_00 (parallel) RTT_PARK_01) + RSER =
	// ((RTT_PARK_00 * RTT_PARK_01) / (RTT_PARK_00 + RTT_PARK_01)) + RSER
	rd1 = divide_nint((rtt_park_00_s * rtt_park_01_s),
			  (rtt_park_00_s + rtt_park_01_s)) + rser;

	// rpullup = rd0 (parallel) rd1 = (rd0 * rd1) / (rd0 + rd1)
	rpullup = divide_nint((rd0 * rd1), (rd0 + rd1));

	// vl = (DQX_CTL / (DQX_CTL + rpullup)) * 1.2
	vl = divide_nint((dqx_ctl_s * vdd), (dqx_ctl_s + rpullup));

	// vlp = ((RSER / rd0) * (1.2 - vl)) + vl
	vlp = divide_nint((rser * (vdd - vl)), rd0) + vl;

	// vcm = (vlp + 1.2) / 2
	vcm = divide_nint((vlp + vdd), 2);

	// vrefpc = (vcm / 1.2) * 100
	vrefpc = divide_nint((vcm * 100 * 100), vdd);

	if (vrefpc < rangepc) {	// < range1 base, use range2
		vref_range = 1 << 6;	// set bit A6 for range2
		rangepc = 4500;	// range2 base is 45%
	}

	vref_value = divide_nint(vrefpc - rangepc, 65);
	if (vref_value < 0)
		vref_value = vref_range;	// set to base of range
	else
		vref_value |= vref_range;

	debug("rtt_wr:%d, rtt_park_00:%d, rtt_park_01:%d, dqx_ctl:%d, rtt_nom:%d, vref_value:%d (0x%x)\n",
	      rtt_wr, rtt_park_00, rtt_park_01, dqx_ctl, rtt_nom, vref_value,
	      vref_value);

	return vref_value;
}

// NOTE: only call this for DIMMs with 1 or 2 ranks, not 4.
static int compute_vref_val(struct ddr_priv *priv, int if_num, int rankx,
			    int dimm_count, int rank_count,
			    struct impedence_values *imp_values,
			    int is_stacked_die, int dram_connection)
{
	int computed_final_vref_value = 0;
	int enable_adjust = ENABLE_COMPUTED_VREF_ADJUSTMENT;
	const char *s;
	int rtt_wr, dqx_ctl, rtt_nom, index;
	union cvmx_lmcx_modereg_params1 lmc_modereg_params1;
	union cvmx_lmcx_modereg_params2 lmc_modereg_params2;
	union cvmx_lmcx_comp_ctl2 comp_ctl2;
	int rtt_park;
	int rtt_park_00;
	int rtt_park_01;

	debug("N0.LMC%d.R%d: %s(...dram_connection = %d)\n",
	      if_num, rankx, __func__, dram_connection);

	// allow some overrides...
	s = env_get("ddr_adjust_computed_vref");
	if (s) {
		enable_adjust = !!simple_strtoul(s, NULL, 0);
		if (!enable_adjust) {
			debug("N0.LMC%d.R%d: DISABLE adjustment of computed VREF\n",
			      if_num, rankx);
		}
	}

	s = env_get("ddr_set_computed_vref");
	if (s) {
		int new_vref = simple_strtoul(s, NULL, 0);

		debug("N0.LMC%d.R%d: OVERRIDE computed VREF to 0x%x (%d)\n",
		      if_num, rankx, new_vref, new_vref);
		return new_vref;
	}

	/*
	 * Calculate an alternative to the measured vref value
	 * but only for configurations we know how to...
	 */
	// We have code for 2-rank DIMMs in both 1-slot or 2-slot configs,
	// and can use the 2-rank 1-slot code for 1-rank DIMMs in 1-slot
	// configs, and can use the 2-rank 2-slot code for 1-rank DIMMs
	// in 2-slot configs.

	lmc_modereg_params1.u64 =
	    lmc_rd(priv, CVMX_LMCX_MODEREG_PARAMS1(if_num));
	lmc_modereg_params2.u64 =
	    lmc_rd(priv, CVMX_LMCX_MODEREG_PARAMS2(if_num));
	comp_ctl2.u64 = lmc_rd(priv, CVMX_LMCX_COMP_CTL2(if_num));
	dqx_ctl = imp_values->dqx_strength[comp_ctl2.s.dqx_ctl];

	// WR always comes from the current rank
	index = (lmc_modereg_params1.u64 >> (rankx * 12 + 5)) & 0x03;
	if (!octeon_is_cpuid(OCTEON_CN78XX_PASS1_X))
		index |= lmc_modereg_params1.u64 >> (51 + rankx - 2) & 0x04;
	rtt_wr = imp_values->rtt_wr_ohms[index];

	// separate calculations for 1 vs 2 DIMMs per LMC
	if (dimm_count == 1) {
		// PARK comes from this rank if 1-rank, otherwise other rank
		index =
		    (lmc_modereg_params2.u64 >>
		     ((rankx ^ (rank_count - 1)) * 10 + 0)) & 0x07;
		rtt_park = imp_values->rtt_nom_ohms[index];
		computed_final_vref_value =
		    compute_vref_1slot_2rank(rtt_wr, rtt_park, dqx_ctl,
					     rank_count, dram_connection);
	} else {
		// get both PARK values from the other DIMM
		index =
		    (lmc_modereg_params2.u64 >> ((rankx ^ 0x02) * 10 + 0)) &
		    0x07;
		rtt_park_00 = imp_values->rtt_nom_ohms[index];
		index =
		    (lmc_modereg_params2.u64 >> ((rankx ^ 0x03) * 10 + 0)) &
		    0x07;
		rtt_park_01 = imp_values->rtt_nom_ohms[index];
		// NOM comes from this rank if 1-rank, otherwise other rank
		index =
		    (lmc_modereg_params1.u64 >>
		     ((rankx ^ (rank_count - 1)) * 12 + 9)) & 0x07;
		rtt_nom = imp_values->rtt_nom_ohms[index];
		computed_final_vref_value =
		    compute_vref_2slot_2rank(rtt_wr, rtt_park_00, rtt_park_01,
					     dqx_ctl, rtt_nom, dram_connection);
	}

	if (enable_adjust) {
		union cvmx_lmcx_config lmc_config;
		union cvmx_lmcx_control lmc_control;

		lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
		lmc_control.u64 = lmc_rd(priv, CVMX_LMCX_CONTROL(if_num));

		/*
		 *  New computed vref = existing computed vref - X
		 *
		 * The value of X is depending on different conditions.
		 * Both #122 and #139 are 2Rx4 RDIMM, while #124 is stacked
		 * die 2Rx4, so I conclude the results into two conditions:
		 *
		 * 1. Stacked Die: 2Rx4
		 * 1-slot: offset = 7. i, e New computed vref = existing
		 * computed vref - 7
		 * 2-slot: offset = 6
		 *
		 * 2. Regular: 2Rx4
		 * 1-slot: offset = 3
		 * 2-slot:  offset = 2
		 */
		// we know we never get called unless DDR4, so test just
		// the other conditions
		if (lmc_control.s.rdimm_ena == 1 &&
		    rank_count == 2 && lmc_config.s.mode_x4dev) {
			// it must first be RDIMM and 2-rank and x4
			int adj;

			// now do according to stacked die or not...
			if (is_stacked_die)
				adj = (dimm_count == 1) ? -7 : -6;
			else
				adj = (dimm_count == 1) ? -3 : -2;

			// we must have adjusted it, so print it out if
			// verbosity is right
			debug("N0.LMC%d.R%d: adjusting computed vref from %2d (0x%02x) to %2d (0x%02x)\n",
			      if_num, rankx, computed_final_vref_value,
			      computed_final_vref_value,
			      computed_final_vref_value + adj,
			      computed_final_vref_value + adj);
			computed_final_vref_value += adj;
		}
	}

	return computed_final_vref_value;
}

static void unpack_rlevel_settings(int if_bytemask, int ecc_ena,
				   struct rlevel_byte_data *rlevel_byte,
				   union cvmx_lmcx_rlevel_rankx lmc_rlevel_rank)
{
	if ((if_bytemask & 0xff) == 0xff) {
		if (ecc_ena) {
			rlevel_byte[8].delay = lmc_rlevel_rank.s.byte7;
			rlevel_byte[7].delay = lmc_rlevel_rank.s.byte6;
			rlevel_byte[6].delay = lmc_rlevel_rank.s.byte5;
			rlevel_byte[5].delay = lmc_rlevel_rank.s.byte4;
			/* ECC */
			rlevel_byte[4].delay = lmc_rlevel_rank.s.byte8;
		} else {
			rlevel_byte[7].delay = lmc_rlevel_rank.s.byte7;
			rlevel_byte[6].delay = lmc_rlevel_rank.s.byte6;
			rlevel_byte[5].delay = lmc_rlevel_rank.s.byte5;
			rlevel_byte[4].delay = lmc_rlevel_rank.s.byte4;
		}
	} else {
		rlevel_byte[8].delay = lmc_rlevel_rank.s.byte8;	/* unused */
		rlevel_byte[7].delay = lmc_rlevel_rank.s.byte7;	/* unused */
		rlevel_byte[6].delay = lmc_rlevel_rank.s.byte6;	/* unused */
		rlevel_byte[5].delay = lmc_rlevel_rank.s.byte5;	/* unused */
		rlevel_byte[4].delay = lmc_rlevel_rank.s.byte4;	/* ECC */
	}

	rlevel_byte[3].delay = lmc_rlevel_rank.s.byte3;
	rlevel_byte[2].delay = lmc_rlevel_rank.s.byte2;
	rlevel_byte[1].delay = lmc_rlevel_rank.s.byte1;
	rlevel_byte[0].delay = lmc_rlevel_rank.s.byte0;
}

static void pack_rlevel_settings(int if_bytemask, int ecc_ena,
				 struct rlevel_byte_data *rlevel_byte,
				 union cvmx_lmcx_rlevel_rankx
				 *final_rlevel_rank)
{
	union cvmx_lmcx_rlevel_rankx lmc_rlevel_rank = *final_rlevel_rank;

	if ((if_bytemask & 0xff) == 0xff) {
		if (ecc_ena) {
			lmc_rlevel_rank.s.byte7 = rlevel_byte[8].delay;
			lmc_rlevel_rank.s.byte6 = rlevel_byte[7].delay;
			lmc_rlevel_rank.s.byte5 = rlevel_byte[6].delay;
			lmc_rlevel_rank.s.byte4 = rlevel_byte[5].delay;
			/* ECC */
			lmc_rlevel_rank.s.byte8 = rlevel_byte[4].delay;
		} else {
			lmc_rlevel_rank.s.byte7 = rlevel_byte[7].delay;
			lmc_rlevel_rank.s.byte6 = rlevel_byte[6].delay;
			lmc_rlevel_rank.s.byte5 = rlevel_byte[5].delay;
			lmc_rlevel_rank.s.byte4 = rlevel_byte[4].delay;
		}
	} else {
		lmc_rlevel_rank.s.byte8 = rlevel_byte[8].delay;
		lmc_rlevel_rank.s.byte7 = rlevel_byte[7].delay;
		lmc_rlevel_rank.s.byte6 = rlevel_byte[6].delay;
		lmc_rlevel_rank.s.byte5 = rlevel_byte[5].delay;
		lmc_rlevel_rank.s.byte4 = rlevel_byte[4].delay;
	}

	lmc_rlevel_rank.s.byte3 = rlevel_byte[3].delay;
	lmc_rlevel_rank.s.byte2 = rlevel_byte[2].delay;
	lmc_rlevel_rank.s.byte1 = rlevel_byte[1].delay;
	lmc_rlevel_rank.s.byte0 = rlevel_byte[0].delay;

	*final_rlevel_rank = lmc_rlevel_rank;
}

/////////////////// These are the RLEVEL settings display routines

// flags
#define WITH_NOTHING 0
#define WITH_SCORE   1
#define WITH_AVERAGE 2
#define WITH_FINAL   4
#define WITH_COMPUTE 8

static void do_display_rl(int if_num,
			  union cvmx_lmcx_rlevel_rankx lmc_rlevel_rank,
			  int rank, int flags, int score)
{
	char score_buf[16];
	char *msg_buf;
	char hex_buf[20];

	if (flags & WITH_SCORE) {
		snprintf(score_buf, sizeof(score_buf), "(%d)", score);
	} else {
		score_buf[0] = ' ';
		score_buf[1] = 0;
	}

	if (flags & WITH_AVERAGE) {
		msg_buf = "  DELAY AVERAGES  ";
	} else if (flags & WITH_FINAL) {
		msg_buf = "  FINAL SETTINGS  ";
	} else if (flags & WITH_COMPUTE) {
		msg_buf = "  COMPUTED DELAYS ";
	} else {
		snprintf(hex_buf, sizeof(hex_buf), "0x%016llX",
			 (unsigned long long)lmc_rlevel_rank.u64);
		msg_buf = hex_buf;
	}

	debug("N0.LMC%d.R%d: Rlevel Rank %#4x, %s  : %5d %5d %5d %5d %5d %5d %5d %5d %5d %s\n",
	      if_num, rank, lmc_rlevel_rank.s.status, msg_buf,
	      lmc_rlevel_rank.s.byte8, lmc_rlevel_rank.s.byte7,
	      lmc_rlevel_rank.s.byte6, lmc_rlevel_rank.s.byte5,
	      lmc_rlevel_rank.s.byte4, lmc_rlevel_rank.s.byte3,
	      lmc_rlevel_rank.s.byte2, lmc_rlevel_rank.s.byte1,
	      lmc_rlevel_rank.s.byte0, score_buf);
}

static void display_rl(int if_num,
		       union cvmx_lmcx_rlevel_rankx lmc_rlevel_rank, int rank)
{
	do_display_rl(if_num, lmc_rlevel_rank, rank, 0, 0);
}

static void display_rl_with_score(int if_num,
				  union cvmx_lmcx_rlevel_rankx lmc_rlevel_rank,
				  int rank, int score)
{
	do_display_rl(if_num, lmc_rlevel_rank, rank, 1, score);
}

static void display_rl_with_final(int if_num,
				  union cvmx_lmcx_rlevel_rankx lmc_rlevel_rank,
				  int rank)
{
	do_display_rl(if_num, lmc_rlevel_rank, rank, 4, 0);
}

static void display_rl_with_computed(int if_num,
				     union cvmx_lmcx_rlevel_rankx
				     lmc_rlevel_rank, int rank, int score)
{
	do_display_rl(if_num, lmc_rlevel_rank, rank, 9, score);
}

// flag values
#define WITH_RODT_BLANK      0
#define WITH_RODT_SKIPPING   1
#define WITH_RODT_BESTROW    2
#define WITH_RODT_BESTSCORE  3
// control
#define SKIP_SKIPPING 1

static const char *with_rodt_canned_msgs[4] = {
	"          ", "SKIPPING  ", "BEST ROW  ", "BEST SCORE"
};

static void display_rl_with_rodt(int if_num,
				 union cvmx_lmcx_rlevel_rankx lmc_rlevel_rank,
				 int rank, int score,
				 int nom_ohms, int rodt_ohms, int flag)
{
	const char *msg_buf;
	char set_buf[20];

#if SKIP_SKIPPING
	if (flag == WITH_RODT_SKIPPING)
		return;
#endif

	msg_buf = with_rodt_canned_msgs[flag];
	if (nom_ohms < 0) {
		snprintf(set_buf, sizeof(set_buf), "    RODT %3d    ",
			 rodt_ohms);
	} else {
		snprintf(set_buf, sizeof(set_buf), "NOM %3d RODT %3d", nom_ohms,
			 rodt_ohms);
	}

	debug("N0.LMC%d.R%d: Rlevel %s   %s  : %5d %5d %5d %5d %5d %5d %5d %5d %5d (%d)\n",
	      if_num, rank, set_buf, msg_buf, lmc_rlevel_rank.s.byte8,
	      lmc_rlevel_rank.s.byte7, lmc_rlevel_rank.s.byte6,
	      lmc_rlevel_rank.s.byte5, lmc_rlevel_rank.s.byte4,
	      lmc_rlevel_rank.s.byte3, lmc_rlevel_rank.s.byte2,
	      lmc_rlevel_rank.s.byte1, lmc_rlevel_rank.s.byte0, score);
}

static void do_display_wl(int if_num,
			  union cvmx_lmcx_wlevel_rankx lmc_wlevel_rank,
			  int rank, int flags)
{
	char *msg_buf;
	char hex_buf[20];

	if (flags & WITH_FINAL) {
		msg_buf = "  FINAL SETTINGS  ";
	} else {
		snprintf(hex_buf, sizeof(hex_buf), "0x%016llX",
			 (unsigned long long)lmc_wlevel_rank.u64);
		msg_buf = hex_buf;
	}

	debug("N0.LMC%d.R%d: Wlevel Rank %#4x, %s  : %5d %5d %5d %5d %5d %5d %5d %5d %5d\n",
	      if_num, rank, lmc_wlevel_rank.s.status, msg_buf,
	      lmc_wlevel_rank.s.byte8, lmc_wlevel_rank.s.byte7,
	      lmc_wlevel_rank.s.byte6, lmc_wlevel_rank.s.byte5,
	      lmc_wlevel_rank.s.byte4, lmc_wlevel_rank.s.byte3,
	      lmc_wlevel_rank.s.byte2, lmc_wlevel_rank.s.byte1,
	      lmc_wlevel_rank.s.byte0);
}

static void display_wl(int if_num,
		       union cvmx_lmcx_wlevel_rankx lmc_wlevel_rank, int rank)
{
	do_display_wl(if_num, lmc_wlevel_rank, rank, WITH_NOTHING);
}

static void display_wl_with_final(int if_num,
				  union cvmx_lmcx_wlevel_rankx lmc_wlevel_rank,
				  int rank)
{
	do_display_wl(if_num, lmc_wlevel_rank, rank, WITH_FINAL);
}

// pretty-print bitmask adjuster
static u64 ppbm(u64 bm)
{
	if (bm != 0ul) {
		while ((bm & 0x0fful) == 0ul)
			bm >>= 4;
	}

	return bm;
}

// xlate PACKED index to UNPACKED index to use with rlevel_byte
#define XPU(i, e) (((i) < 4) ? (i) : (((i) < 8) ? (i) + (e) : 4))
// xlate UNPACKED index to PACKED index to use with rlevel_bitmask
#define XUP(i, e) (((i) < 4) ? (i) : (e) ? (((i) > 4) ? (i) - 1 : 8) : (i))

// flag values
#define WITH_WL_BITMASKS      0
#define WITH_RL_BITMASKS      1
#define WITH_RL_MASK_SCORES   2
#define WITH_RL_SEQ_SCORES    3

static void do_display_bm(int if_num, int rank, void *bm,
			  int flags, int ecc)
{
	if (flags == WITH_WL_BITMASKS) {
		// wlevel_bitmask array in PACKED index order, so just
		// print them
		int *bitmasks = (int *)bm;

		debug("N0.LMC%d.R%d: Wlevel Debug Bitmasks                 : %05x %05x %05x %05x %05x %05x %05x %05x %05x\n",
		      if_num, rank, bitmasks[8], bitmasks[7], bitmasks[6],
		      bitmasks[5], bitmasks[4], bitmasks[3], bitmasks[2],
		      bitmasks[1], bitmasks[0]
			);
	} else if (flags == WITH_RL_BITMASKS) {
		// rlevel_bitmask array in PACKED index order, so just
		// print them
		struct rlevel_bitmask *rlevel_bitmask =
			(struct rlevel_bitmask *)bm;

		debug("N0.LMC%d.R%d: Rlevel Debug Bitmasks        8:0      : %05llx %05llx %05llx %05llx %05llx %05llx %05llx %05llx %05llx\n",
		      if_num, rank, ppbm(rlevel_bitmask[8].bm),
		      ppbm(rlevel_bitmask[7].bm), ppbm(rlevel_bitmask[6].bm),
		      ppbm(rlevel_bitmask[5].bm), ppbm(rlevel_bitmask[4].bm),
		      ppbm(rlevel_bitmask[3].bm), ppbm(rlevel_bitmask[2].bm),
		      ppbm(rlevel_bitmask[1].bm), ppbm(rlevel_bitmask[0].bm)
			);
	} else if (flags == WITH_RL_MASK_SCORES) {
		// rlevel_bitmask array in PACKED index order, so just
		// print them
		struct rlevel_bitmask *rlevel_bitmask =
			(struct rlevel_bitmask *)bm;

		debug("N0.LMC%d.R%d: Rlevel Debug Bitmask Scores  8:0      : %5d %5d %5d %5d %5d %5d %5d %5d %5d\n",
		      if_num, rank, rlevel_bitmask[8].errs,
		      rlevel_bitmask[7].errs, rlevel_bitmask[6].errs,
		      rlevel_bitmask[5].errs, rlevel_bitmask[4].errs,
		      rlevel_bitmask[3].errs, rlevel_bitmask[2].errs,
		      rlevel_bitmask[1].errs, rlevel_bitmask[0].errs);
	} else if (flags == WITH_RL_SEQ_SCORES) {
		// rlevel_byte array in UNPACKED index order, so xlate
		// and print them
		struct rlevel_byte_data *rlevel_byte =
			(struct rlevel_byte_data *)bm;

		debug("N0.LMC%d.R%d: Rlevel Debug Non-seq Scores  8:0      : %5d %5d %5d %5d %5d %5d %5d %5d %5d\n",
		      if_num, rank, rlevel_byte[XPU(8, ecc)].sqerrs,
		      rlevel_byte[XPU(7, ecc)].sqerrs,
		      rlevel_byte[XPU(6, ecc)].sqerrs,
		      rlevel_byte[XPU(5, ecc)].sqerrs,
		      rlevel_byte[XPU(4, ecc)].sqerrs,
		      rlevel_byte[XPU(3, ecc)].sqerrs,
		      rlevel_byte[XPU(2, ecc)].sqerrs,
		      rlevel_byte[XPU(1, ecc)].sqerrs,
		      rlevel_byte[XPU(0, ecc)].sqerrs);
	}
}

static void display_wl_bm(int if_num, int rank, int *bitmasks)
{
	do_display_bm(if_num, rank, (void *)bitmasks, WITH_WL_BITMASKS, 0);
}

static void display_rl_bm(int if_num, int rank,
			  struct rlevel_bitmask *bitmasks, int ecc_ena)
{
	do_display_bm(if_num, rank, (void *)bitmasks, WITH_RL_BITMASKS,
		      ecc_ena);
}

static void display_rl_bm_scores(int if_num, int rank,
				 struct rlevel_bitmask *bitmasks, int ecc_ena)
{
	do_display_bm(if_num, rank, (void *)bitmasks, WITH_RL_MASK_SCORES,
		      ecc_ena);
}

static void display_rl_seq_scores(int if_num, int rank,
				  struct rlevel_byte_data *bytes, int ecc_ena)
{
	do_display_bm(if_num, rank, (void *)bytes, WITH_RL_SEQ_SCORES, ecc_ena);
}

#define RODT_OHMS_COUNT        8
#define RTT_NOM_OHMS_COUNT     8
#define RTT_NOM_TABLE_COUNT    8
#define RTT_WR_OHMS_COUNT      8
#define DIC_OHMS_COUNT         3
#define DRIVE_STRENGTH_COUNT  15

static unsigned char ddr4_rodt_ohms[RODT_OHMS_COUNT] = {
	0, 40, 60, 80, 120, 240, 34, 48 };
static unsigned char ddr4_rtt_nom_ohms[RTT_NOM_OHMS_COUNT] = {
	0, 60, 120, 40, 240, 48, 80, 34 };
static unsigned char ddr4_rtt_nom_table[RTT_NOM_TABLE_COUNT] = {
	0, 4, 2, 6, 1, 5, 3, 7 };
// setting HiZ ohms to 99 for computed vref
static unsigned char ddr4_rtt_wr_ohms[RTT_WR_OHMS_COUNT] = {
	0, 120, 240, 99, 80 };
static unsigned char ddr4_dic_ohms[DIC_OHMS_COUNT] = { 34, 48 };
static short ddr4_drive_strength[DRIVE_STRENGTH_COUNT] = {
	0, 0, 26, 30, 34, 40, 48, 68, 0, 0, 0, 0, 0, 0, 0 };
static short ddr4_dqx_strength[DRIVE_STRENGTH_COUNT] = {
	0, 24, 27, 30, 34, 40, 48, 60, 0, 0, 0, 0, 0, 0, 0 };
struct impedence_values ddr4_impedence_val = {
	.rodt_ohms = ddr4_rodt_ohms,
	.rtt_nom_ohms = ddr4_rtt_nom_ohms,
	.rtt_nom_table = ddr4_rtt_nom_table,
	.rtt_wr_ohms = ddr4_rtt_wr_ohms,
	.dic_ohms = ddr4_dic_ohms,
	.drive_strength = ddr4_drive_strength,
	.dqx_strength = ddr4_dqx_strength,
};

static unsigned char ddr3_rodt_ohms[RODT_OHMS_COUNT] = {
	0, 20, 30, 40, 60, 120, 0, 0 };
static unsigned char ddr3_rtt_nom_ohms[RTT_NOM_OHMS_COUNT] = {
	0, 60, 120, 40, 20, 30, 0, 0 };
static unsigned char ddr3_rtt_nom_table[RTT_NOM_TABLE_COUNT] = {
	0, 2, 1, 3, 5, 4, 0, 0 };
static unsigned char ddr3_rtt_wr_ohms[RTT_WR_OHMS_COUNT] = { 0, 60, 120 };
static unsigned char ddr3_dic_ohms[DIC_OHMS_COUNT] = { 40, 34 };
static short ddr3_drive_strength[DRIVE_STRENGTH_COUNT] = {
	0, 24, 27, 30, 34, 40, 48, 60, 0, 0, 0, 0, 0, 0, 0 };
static struct impedence_values ddr3_impedence_val = {
	.rodt_ohms = ddr3_rodt_ohms,
	.rtt_nom_ohms = ddr3_rtt_nom_ohms,
	.rtt_nom_table = ddr3_rtt_nom_table,
	.rtt_wr_ohms = ddr3_rtt_wr_ohms,
	.dic_ohms = ddr3_dic_ohms,
	.drive_strength = ddr3_drive_strength,
	.dqx_strength = ddr3_drive_strength,
};

static u64 hertz_to_psecs(u64 hertz)
{
	/* Clock in psecs */
	return divide_nint((u64)1000 * 1000 * 1000 * 1000, hertz);
}

#define DIVIDEND_SCALE 1000	/* Scale to avoid rounding error. */

static u64 psecs_to_mts(u64 psecs)
{
	return divide_nint(divide_nint((u64)(2 * 1000000 * DIVIDEND_SCALE),
				       psecs), DIVIDEND_SCALE);
}

#define WITHIN(v, b, m) (((v) >= ((b) - (m))) && ((v) <= ((b) + (m))))

static unsigned long pretty_psecs_to_mts(u64 psecs)
{
	u64 ret = 0;		// default to error

	if (WITHIN(psecs, 2500, 1))
		ret = 800;
	else if (WITHIN(psecs, 1875, 1))
		ret = 1066;
	else if (WITHIN(psecs, 1500, 1))
		ret = 1333;
	else if (WITHIN(psecs, 1250, 1))
		ret = 1600;
	else if (WITHIN(psecs, 1071, 1))
		ret = 1866;
	else if (WITHIN(psecs, 937, 1))
		ret = 2133;
	else if (WITHIN(psecs, 833, 1))
		ret = 2400;
	else if (WITHIN(psecs, 750, 1))
		ret = 2666;
	return ret;
}

static u64 mts_to_hertz(u64 mts)
{
	return ((mts * 1000 * 1000) / 2);
}

static int compute_rc3x(int64_t tclk_psecs)
{
	long speed;
	long tclk_psecs_min, tclk_psecs_max;
	long data_rate_mhz, data_rate_mhz_min, data_rate_mhz_max;
	int rc3x;

#define ENCODING_BASE 1240

	data_rate_mhz = psecs_to_mts(tclk_psecs);

	/*
	 * 2400 MT/s is a special case. Using integer arithmetic it rounds
	 * from 833 psecs to 2401 MT/s. Force it to 2400 to pick the
	 * proper setting from the table.
	 */
	if (tclk_psecs == 833)
		data_rate_mhz = 2400;

	for (speed = ENCODING_BASE; speed < 3200; speed += 20) {
		int error = 0;

		/* Clock in psecs */
		tclk_psecs_min = hertz_to_psecs(mts_to_hertz(speed + 00));
		/* Clock in psecs */
		tclk_psecs_max = hertz_to_psecs(mts_to_hertz(speed + 18));

		data_rate_mhz_min = psecs_to_mts(tclk_psecs_min);
		data_rate_mhz_max = psecs_to_mts(tclk_psecs_max);

		/* Force alingment to multiple to avound rounding errors. */
		data_rate_mhz_min = ((data_rate_mhz_min + 18) / 20) * 20;
		data_rate_mhz_max = ((data_rate_mhz_max + 18) / 20) * 20;

		error += (speed + 00 != data_rate_mhz_min);
		error += (speed + 20 != data_rate_mhz_max);

		rc3x = (speed - ENCODING_BASE) / 20;

		if (data_rate_mhz <= (speed + 20))
			break;
	}

	return rc3x;
}

/*
 * static global variables needed, so that functions (loops) can be
 * restructured from the main huge function. Its not elegant, but the
 * only way to break the original functions like init_octeon3_ddr3_interface()
 * into separate logical smaller functions with less indentation levels.
 */
static int if_num __section(".data");
static u32 if_mask __section(".data");
static int ddr_hertz __section(".data");

static struct ddr_conf *ddr_conf __section(".data");
static const struct dimm_odt_config *odt_1rank_config __section(".data");
static const struct dimm_odt_config *odt_2rank_config __section(".data");
static const struct dimm_odt_config *odt_4rank_config __section(".data");
static struct dimm_config *dimm_config_table __section(".data");
static const struct dimm_odt_config *odt_config __section(".data");
static const struct ddr3_custom_config *c_cfg __section(".data");

static int odt_idx __section(".data");

static ulong tclk_psecs __section(".data");
static ulong eclk_psecs __section(".data");

static int row_bits __section(".data");
static int col_bits __section(".data");
static int num_banks __section(".data");
static int num_ranks __section(".data");
static int dram_width __section(".data");
static int dimm_count __section(".data");
/* Accumulate and report all the errors before giving up */
static int fatal_error __section(".data");
/* Flag that indicates safe DDR settings should be used */
static int safe_ddr_flag __section(".data");
/* Octeon II Default: 64bit interface width */
static int if_64b __section(".data");
static int if_bytemask __section(".data");
static u32 mem_size_mbytes __section(".data");
static unsigned int didx __section(".data");
static int bank_bits __section(".data");
static int bunk_enable __section(".data");
static int rank_mask __section(".data");
static int column_bits_start __section(".data");
static int row_lsb __section(".data");
static int pbank_lsb __section(".data");
static int use_ecc __section(".data");
static int mtb_psec __section(".data");
static short ftb_dividend __section(".data");
static short ftb_divisor __section(".data");
static int taamin __section(".data");
static int tckmin __section(".data");
static int cl __section(".data");
static int min_cas_latency __section(".data");
static int max_cas_latency __section(".data");
static int override_cas_latency __section(".data");
static int ddr_rtt_nom_auto __section(".data");
static int ddr_rodt_ctl_auto __section(".data");

static int spd_addr __section(".data");
static int spd_org __section(".data");
static int spd_banks __section(".data");
static int spd_rdimm __section(".data");
static int spd_dimm_type __section(".data");
static int spd_ecc __section(".data");
static u32 spd_cas_latency __section(".data");
static int spd_mtb_dividend __section(".data");
static int spd_mtb_divisor __section(".data");
static int spd_tck_min __section(".data");
static int spd_taa_min __section(".data");
static int spd_twr __section(".data");
static int spd_trcd __section(".data");
static int spd_trrd __section(".data");
static int spd_trp __section(".data");
static int spd_tras __section(".data");
static int spd_trc __section(".data");
static int spd_trfc __section(".data");
static int spd_twtr __section(".data");
static int spd_trtp __section(".data");
static int spd_tfaw __section(".data");
static int spd_addr_mirror __section(".data");
static int spd_package __section(".data");
static int spd_rawcard __section(".data");
static int spd_rawcard_aorb __section(".data");
static int spd_rdimm_registers __section(".data");
static int spd_thermal_sensor __section(".data");

static int is_stacked_die __section(".data");
static int is_3ds_dimm __section(".data");
// 3DS: logical ranks per package rank
static int lranks_per_prank __section(".data");
// 3DS: logical ranks bits
static int lranks_bits __section(".data");
// in Mbits; only used for 3DS
static int die_capacity __section(".data");

static enum ddr_type ddr_type __section(".data");

static int twr __section(".data");
static int trcd __section(".data");
static int trrd __section(".data");
static int trp __section(".data");
static int tras __section(".data");
static int trc __section(".data");
static int trfc __section(".data");
static int twtr __section(".data");
static int trtp __section(".data");
static int tfaw __section(".data");

static int ddr4_tckavgmin __section(".data");
static int ddr4_tckavgmax __section(".data");
static int ddr4_trdcmin __section(".data");
static int ddr4_trpmin __section(".data");
static int ddr4_trasmin __section(".data");
static int ddr4_trcmin __section(".data");
static int ddr4_trfc1min __section(".data");
static int ddr4_trfc2min __section(".data");
static int ddr4_trfc4min __section(".data");
static int ddr4_tfawmin __section(".data");
static int ddr4_trrd_smin __section(".data");
static int ddr4_trrd_lmin __section(".data");
static int ddr4_tccd_lmin __section(".data");

static int wl_mask_err __section(".data");
static int wl_loops __section(".data");
static int default_rtt_nom[4] __section(".data");
static int dyn_rtt_nom_mask __section(".data");
static struct impedence_values *imp_val __section(".data");
static char default_rodt_ctl __section(".data");
// default to disabled (ie, try LMC restart, not chip reset)
static int ddr_disable_chip_reset __section(".data");
static const char *dimm_type_name __section(".data");
static int match_wl_rtt_nom __section(".data");

struct hwl_alt_by_rank {
	u16 hwl_alt_mask;	// mask of bytelanes with alternate
	u16 hwl_alt_delay[9];	// bytelane alternate avail if mask=1
};

static struct hwl_alt_by_rank hwl_alts[4] __section(".data");

#define DEFAULT_INTERNAL_VREF_TRAINING_LIMIT 3	// was: 5
static int internal_retries __section(".data");

static int deskew_training_errors __section(".data");
static struct deskew_counts deskew_training_results __section(".data");
static int disable_deskew_training __section(".data");
static int restart_if_dsk_incomplete __section(".data");
static int dac_eval_retries __section(".data");
static int dac_settings[9] __section(".data");
static int num_samples __section(".data");
static int sample __section(".data");
static int lane __section(".data");
static int last_lane __section(".data");
static int total_dac_eval_retries __section(".data");
static int dac_eval_exhausted __section(".data");

#define DEFAULT_DAC_SAMPLES 7	// originally was 5
#define DAC_RETRIES_LIMIT   2

struct bytelane_sample {
	s16 bytes[DEFAULT_DAC_SAMPLES];
};

static struct bytelane_sample lanes[9] __section(".data");

static char disable_sequential_delay_check __section(".data");
static int wl_print __section(".data");

static int enable_by_rank_init __section(".data");
static int saved_rank_mask __section(".data");
static int by_rank __section(".data");
static struct deskew_data rank_dsk[4] __section(".data");
static struct dac_data rank_dac[4] __section(".data");

// todo: perhaps remove node at some time completely?
static int node __section(".data");
static int base_cl __section(".data");

/* Parameters from DDR3 Specifications */
#define DDR3_TREFI         7800000	/* 7.8 us */
#define DDR3_ZQCS          80000ull	/* 80 ns */
#define DDR3_ZQCS_INTERNAL 1280000000ull	/* 128ms/100 */
#define DDR3_TCKE          5000	/* 5 ns */
#define DDR3_TMRD          4	/* 4 nCK */
#define DDR3_TDLLK         512	/* 512 nCK */
#define DDR3_TMPRR         1	/* 1 nCK */
#define DDR3_TWLMRD        40	/* 40 nCK */
#define DDR3_TWLDQSEN      25	/* 25 nCK */

/* Parameters from DDR4 Specifications */
#define DDR4_TMRD          8	/* 8 nCK */
#define DDR4_TDLLK         768	/* 768 nCK */

static void lmc_config(struct ddr_priv *priv)
{
	union cvmx_lmcx_config cfg;
	char *s;

	cfg.u64 = 0;

	cfg.cn78xx.ecc_ena = use_ecc;
	cfg.cn78xx.row_lsb = encode_row_lsb_ddr3(row_lsb);
	cfg.cn78xx.pbank_lsb = encode_pbank_lsb_ddr3(pbank_lsb);

	cfg.cn78xx.idlepower = 0;	/* Disabled */

	s = lookup_env(priv, "ddr_idlepower");
	if (s)
		cfg.cn78xx.idlepower = simple_strtoul(s, NULL, 0);

	cfg.cn78xx.forcewrite = 0;	/* Disabled */
	/* Include memory reference address in the ECC */
	cfg.cn78xx.ecc_adr = 1;

	s = lookup_env(priv, "ddr_ecc_adr");
	if (s)
		cfg.cn78xx.ecc_adr = simple_strtoul(s, NULL, 0);

	cfg.cn78xx.reset = 0;

	/*
	 * Program LMC0_CONFIG[24:18], ref_zqcs_int(6:0) to
	 * RND-DN(tREFI/clkPeriod/512) Program LMC0_CONFIG[36:25],
	 * ref_zqcs_int(18:7) to
	 * RND-DN(ZQCS_Interval/clkPeriod/(512*128)). Note that this
	 * value should always be greater than 32, to account for
	 * resistor calibration delays.
	 */

	cfg.cn78xx.ref_zqcs_int = ((DDR3_TREFI / tclk_psecs / 512) & 0x7f);
	cfg.cn78xx.ref_zqcs_int |=
		((max(33ull, (DDR3_ZQCS_INTERNAL / (tclk_psecs / 100) /
			      (512 * 128))) & 0xfff) << 7);

	cfg.cn78xx.early_dqx = 1;	/* Default to enabled */

	s = lookup_env(priv, "ddr_early_dqx");
	if (!s)
		s = lookup_env(priv, "ddr%d_early_dqx", if_num);

	if (s)
		cfg.cn78xx.early_dqx = simple_strtoul(s, NULL, 0);

	cfg.cn78xx.sref_with_dll = 0;

	cfg.cn78xx.rank_ena = bunk_enable;
	cfg.cn78xx.rankmask = rank_mask;	/* Set later */
	cfg.cn78xx.mirrmask = (spd_addr_mirror << 1 | spd_addr_mirror << 3) &
		rank_mask;
	/* Set once and don't change it. */
	cfg.cn78xx.init_status = rank_mask;
	cfg.cn78xx.early_unload_d0_r0 = 0;
	cfg.cn78xx.early_unload_d0_r1 = 0;
	cfg.cn78xx.early_unload_d1_r0 = 0;
	cfg.cn78xx.early_unload_d1_r1 = 0;
	cfg.cn78xx.scrz = 0;
	if (octeon_is_cpuid(OCTEON_CN70XX))
		cfg.cn78xx.mode32b = 1;	/* Read-only. Always 1. */
	cfg.cn78xx.mode_x4dev = (dram_width == 4) ? 1 : 0;
	cfg.cn78xx.bg2_enable = ((ddr_type == DDR4_DRAM) &&
				 (dram_width == 16)) ? 0 : 1;

	s = lookup_env_ull(priv, "ddr_config");
	if (s)
		cfg.u64 = simple_strtoull(s, NULL, 0);
	debug("LMC_CONFIG                                    : 0x%016llx\n",
	      cfg.u64);
	lmc_wr(priv, CVMX_LMCX_CONFIG(if_num), cfg.u64);
}

static void lmc_control(struct ddr_priv *priv)
{
	union cvmx_lmcx_control ctrl;
	char *s;

	ctrl.u64 = lmc_rd(priv, CVMX_LMCX_CONTROL(if_num));
	ctrl.s.rdimm_ena = spd_rdimm;
	ctrl.s.bwcnt = 0;	/* Clear counter later */
	if (spd_rdimm)
		ctrl.s.ddr2t = (safe_ddr_flag ? 1 : c_cfg->ddr2t_rdimm);
	else
		ctrl.s.ddr2t = (safe_ddr_flag ? 1 : c_cfg->ddr2t_udimm);
	ctrl.s.pocas = 0;
	ctrl.s.fprch2 = (safe_ddr_flag ? 2 : c_cfg->fprch2);
	ctrl.s.throttle_rd = safe_ddr_flag ? 1 : 0;
	ctrl.s.throttle_wr = safe_ddr_flag ? 1 : 0;
	ctrl.s.inorder_rd = safe_ddr_flag ? 1 : 0;
	ctrl.s.inorder_wr = safe_ddr_flag ? 1 : 0;
	ctrl.s.elev_prio_dis = safe_ddr_flag ? 1 : 0;
	/* discards writes to addresses that don't exist in the DRAM */
	ctrl.s.nxm_write_en = 0;
	ctrl.s.max_write_batch = 8;
	ctrl.s.xor_bank = 1;
	ctrl.s.auto_dclkdis = 1;
	ctrl.s.int_zqcs_dis = 0;
	ctrl.s.ext_zqcs_dis = 0;
	ctrl.s.bprch = 1;
	ctrl.s.wodt_bprch = 1;
	ctrl.s.rodt_bprch = 1;

	s = lookup_env(priv, "ddr_xor_bank");
	if (s)
		ctrl.s.xor_bank = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_2t");
	if (s)
		ctrl.s.ddr2t = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_fprch2");
	if (s)
		ctrl.s.fprch2 = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_bprch");
	if (s)
		ctrl.s.bprch = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_wodt_bprch");
	if (s)
		ctrl.s.wodt_bprch = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_rodt_bprch");
	if (s)
		ctrl.s.rodt_bprch = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_int_zqcs_dis");
	if (s)
		ctrl.s.int_zqcs_dis = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_ext_zqcs_dis");
	if (s)
		ctrl.s.ext_zqcs_dis = simple_strtoul(s, NULL, 0);

	s = lookup_env_ull(priv, "ddr_control");
	if (s)
		ctrl.u64 = simple_strtoull(s, NULL, 0);

	debug("LMC_CONTROL                                   : 0x%016llx\n",
	      ctrl.u64);
	lmc_wr(priv, CVMX_LMCX_CONTROL(if_num), ctrl.u64);
}

static void lmc_timing_params0(struct ddr_priv *priv)
{
	union cvmx_lmcx_timing_params0 tp0;
	unsigned int trp_value;
	char *s;

	tp0.u64 = lmc_rd(priv, CVMX_LMCX_TIMING_PARAMS0(if_num));

	trp_value = divide_roundup(trp, tclk_psecs) - 1;
	debug("TIMING_PARAMS0[TRP]: NEW 0x%x, OLD 0x%x\n", trp_value,
	      trp_value +
	      (unsigned int)(divide_roundup(max(4ull * tclk_psecs, 7500ull),
					    tclk_psecs)) - 4);
	s = lookup_env_ull(priv, "ddr_use_old_trp");
	if (s) {
		if (!!simple_strtoull(s, NULL, 0)) {
			trp_value +=
			    divide_roundup(max(4ull * tclk_psecs, 7500ull),
					   tclk_psecs) - 4;
			debug("TIMING_PARAMS0[trp]: USING OLD 0x%x\n",
			      trp_value);
		}
	}

	tp0.cn78xx.txpr =
	    divide_roundup(max(5ull * tclk_psecs, trfc + 10000ull),
			   16 * tclk_psecs);
	tp0.cn78xx.trp = trp_value & 0x1f;
	tp0.cn78xx.tcksre =
	    divide_roundup(max(5ull * tclk_psecs, 10000ull), tclk_psecs) - 1;

	if (ddr_type == DDR4_DRAM) {
		int tzqinit = 4;	// Default to 4, for all DDR4 speed bins

		s = lookup_env(priv, "ddr_tzqinit");
		if (s)
			tzqinit = simple_strtoul(s, NULL, 0);

		tp0.cn78xx.tzqinit = tzqinit;
		/* Always 8. */
		tp0.cn78xx.tzqcs = divide_roundup(128 * tclk_psecs,
						  (16 * tclk_psecs));
		tp0.cn78xx.tcke =
		    divide_roundup(max(3 * tclk_psecs, (ulong)DDR3_TCKE),
				   tclk_psecs) - 1;
		tp0.cn78xx.tmrd =
		    divide_roundup((DDR4_TMRD * tclk_psecs), tclk_psecs) - 1;
		tp0.cn78xx.tmod = 25;	/* 25 is the max allowed */
		tp0.cn78xx.tdllk = divide_roundup(DDR4_TDLLK, 256);
	} else {
		tp0.cn78xx.tzqinit =
		    divide_roundup(max(512ull * tclk_psecs, 640000ull),
				   (256 * tclk_psecs));
		tp0.cn78xx.tzqcs =
		    divide_roundup(max(64ull * tclk_psecs, DDR3_ZQCS),
				   (16 * tclk_psecs));
		tp0.cn78xx.tcke = divide_roundup(DDR3_TCKE, tclk_psecs) - 1;
		tp0.cn78xx.tmrd =
		    divide_roundup((DDR3_TMRD * tclk_psecs), tclk_psecs) - 1;
		tp0.cn78xx.tmod =
		    divide_roundup(max(12ull * tclk_psecs, 15000ull),
				   tclk_psecs) - 1;
		tp0.cn78xx.tdllk = divide_roundup(DDR3_TDLLK, 256);
	}

	s = lookup_env_ull(priv, "ddr_timing_params0");
	if (s)
		tp0.u64 = simple_strtoull(s, NULL, 0);
	debug("TIMING_PARAMS0                                : 0x%016llx\n",
	      tp0.u64);
	lmc_wr(priv, CVMX_LMCX_TIMING_PARAMS0(if_num), tp0.u64);
}

static void lmc_timing_params1(struct ddr_priv *priv)
{
	union cvmx_lmcx_timing_params1 tp1;
	unsigned int txp, temp_trcd, trfc_dlr;
	char *s;

	tp1.u64 = lmc_rd(priv, CVMX_LMCX_TIMING_PARAMS1(if_num));

	/* .cn70xx. */
	tp1.s.tmprr = divide_roundup(DDR3_TMPRR * tclk_psecs, tclk_psecs) - 1;

	tp1.cn78xx.tras = divide_roundup(tras, tclk_psecs) - 1;

	temp_trcd = divide_roundup(trcd, tclk_psecs);
	if (temp_trcd > 15) {
		debug("TIMING_PARAMS1[trcd]: need extension bit for 0x%x\n",
		      temp_trcd);
	}
	if (octeon_is_cpuid(OCTEON_CN78XX_PASS1_X) && temp_trcd > 15) {
		/*
		 * Let .trcd=0 serve as a flag that the field has
		 * overflowed. Must use Additive Latency mode as a
		 * workaround.
		 */
		temp_trcd = 0;
	}
	tp1.cn78xx.trcd = (temp_trcd >> 0) & 0xf;
	tp1.cn78xx.trcd_ext = (temp_trcd >> 4) & 0x1;

	tp1.cn78xx.twtr = divide_roundup(twtr, tclk_psecs) - 1;
	tp1.cn78xx.trfc = divide_roundup(trfc, 8 * tclk_psecs);

	if (ddr_type == DDR4_DRAM) {
		/* Workaround bug 24006. Use Trrd_l. */
		tp1.cn78xx.trrd =
		    divide_roundup(ddr4_trrd_lmin, tclk_psecs) - 2;
	} else {
		tp1.cn78xx.trrd = divide_roundup(trrd, tclk_psecs) - 2;
	}

	/*
	 * tXP = max( 3nCK, 7.5 ns)     DDR3-800   tCLK = 2500 psec
	 * tXP = max( 3nCK, 7.5 ns)     DDR3-1066  tCLK = 1875 psec
	 * tXP = max( 3nCK, 6.0 ns)     DDR3-1333  tCLK = 1500 psec
	 * tXP = max( 3nCK, 6.0 ns)     DDR3-1600  tCLK = 1250 psec
	 * tXP = max( 3nCK, 6.0 ns)     DDR3-1866  tCLK = 1071 psec
	 * tXP = max( 3nCK, 6.0 ns)     DDR3-2133  tCLK =  937 psec
	 */
	txp = (tclk_psecs < 1875) ? 6000 : 7500;
	txp = divide_roundup(max((unsigned int)(3 * tclk_psecs), txp),
			     tclk_psecs) - 1;
	if (txp > 7) {
		debug("TIMING_PARAMS1[txp]: need extension bit for 0x%x\n",
		      txp);
	}
	if (octeon_is_cpuid(OCTEON_CN78XX_PASS1_X) && txp > 7)
		txp = 7;	// max it out
	tp1.cn78xx.txp = (txp >> 0) & 7;
	tp1.cn78xx.txp_ext = (txp >> 3) & 1;

	tp1.cn78xx.twlmrd = divide_roundup(DDR3_TWLMRD * tclk_psecs,
					   4 * tclk_psecs);
	tp1.cn78xx.twldqsen = divide_roundup(DDR3_TWLDQSEN * tclk_psecs,
					     4 * tclk_psecs);
	tp1.cn78xx.tfaw = divide_roundup(tfaw, 4 * tclk_psecs);
	tp1.cn78xx.txpdll = divide_roundup(max(10ull * tclk_psecs, 24000ull),
					   tclk_psecs) - 1;

	if (ddr_type == DDR4_DRAM && is_3ds_dimm) {
		/*
		 * 4 Gb: tRFC_DLR = 90 ns
		 * 8 Gb: tRFC_DLR = 120 ns
		 * 16 Gb: tRFC_DLR = 190 ns FIXME?
		 */
		if (die_capacity == 0x1000)	// 4 Gbit
			trfc_dlr = 90;
		else if (die_capacity == 0x2000)	// 8 Gbit
			trfc_dlr = 120;
		else if (die_capacity == 0x4000)	// 16 Gbit
			trfc_dlr = 190;
		else
			trfc_dlr = 0;

		if (trfc_dlr == 0) {
			debug("N%d.LMC%d: ERROR: tRFC_DLR: die_capacity %u Mbit is illegal\n",
			      node, if_num, die_capacity);
		} else {
			tp1.cn78xx.trfc_dlr =
			    divide_roundup(trfc_dlr * 1000UL, 8 * tclk_psecs);
			debug("N%d.LMC%d: TIMING_PARAMS1[trfc_dlr] set to %u\n",
			      node, if_num, tp1.cn78xx.trfc_dlr);
		}
	}

	s = lookup_env_ull(priv, "ddr_timing_params1");
	if (s)
		tp1.u64 = simple_strtoull(s, NULL, 0);

	debug("TIMING_PARAMS1                                : 0x%016llx\n",
	      tp1.u64);
	lmc_wr(priv, CVMX_LMCX_TIMING_PARAMS1(if_num), tp1.u64);
}

static void lmc_timing_params2(struct ddr_priv *priv)
{
	if (ddr_type == DDR4_DRAM) {
		union cvmx_lmcx_timing_params1 tp1;
		union cvmx_lmcx_timing_params2 tp2;
		int temp_trrd_l;

		tp1.u64 = lmc_rd(priv, CVMX_LMCX_TIMING_PARAMS1(if_num));
		tp2.u64 = lmc_rd(priv, CVMX_LMCX_TIMING_PARAMS2(if_num));
		debug("TIMING_PARAMS2                                : 0x%016llx\n",
		      tp2.u64);

		temp_trrd_l = divide_roundup(ddr4_trrd_lmin, tclk_psecs) - 2;
		if (temp_trrd_l > 7)
			debug("TIMING_PARAMS2[trrd_l]: need extension bit for 0x%x\n",
			      temp_trrd_l);
		if (octeon_is_cpuid(OCTEON_CN78XX_PASS1_X) && temp_trrd_l > 7)
			temp_trrd_l = 7;	// max it out
		tp2.cn78xx.trrd_l = (temp_trrd_l >> 0) & 7;
		tp2.cn78xx.trrd_l_ext = (temp_trrd_l >> 3) & 1;

		// correct for 1600-2400
		tp2.s.twtr_l = divide_nint(max(4ull * tclk_psecs, 7500ull),
					   tclk_psecs) - 1;
		tp2.s.t_rw_op_max = 7;
		tp2.s.trtp = divide_roundup(max(4ull * tclk_psecs, 7500ull),
					    tclk_psecs) - 1;

		debug("TIMING_PARAMS2                                : 0x%016llx\n",
		      tp2.u64);
		lmc_wr(priv, CVMX_LMCX_TIMING_PARAMS2(if_num), tp2.u64);

		/*
		 * Workaround Errata 25823 - LMC: Possible DDR4 tWTR_L not met
		 * for Write-to-Read operations to the same Bank Group
		 */
		if (tp1.cn78xx.twtr < (tp2.s.twtr_l - 4)) {
			tp1.cn78xx.twtr = tp2.s.twtr_l - 4;
			debug("ERRATA 25823: NEW: TWTR: %d, TWTR_L: %d\n",
			      tp1.cn78xx.twtr, tp2.s.twtr_l);
			debug("TIMING_PARAMS1                                : 0x%016llx\n",
			      tp1.u64);
			lmc_wr(priv, CVMX_LMCX_TIMING_PARAMS1(if_num), tp1.u64);
		}
	}
}

static void lmc_modereg_params0(struct ddr_priv *priv)
{
	union cvmx_lmcx_modereg_params0 mp0;
	int param;
	char *s;

	mp0.u64 = lmc_rd(priv, CVMX_LMCX_MODEREG_PARAMS0(if_num));

	if (ddr_type == DDR4_DRAM) {
		mp0.s.cwl = 0;	/* 1600 (1250ps) */
		if (tclk_psecs < 1250)
			mp0.s.cwl = 1;	/* 1866 (1072ps) */
		if (tclk_psecs < 1072)
			mp0.s.cwl = 2;	/* 2133 (938ps) */
		if (tclk_psecs < 938)
			mp0.s.cwl = 3;	/* 2400 (833ps) */
		if (tclk_psecs < 833)
			mp0.s.cwl = 4;	/* 2666 (750ps) */
		if (tclk_psecs < 750)
			mp0.s.cwl = 5;	/* 3200 (625ps) */
	} else {
		/*
		 ** CSR   CWL         CAS write Latency
		 ** ===   ===   =================================
		 **  0      5   (           tCK(avg) >=   2.5 ns)
		 **  1      6   (2.5 ns   > tCK(avg) >= 1.875 ns)
		 **  2      7   (1.875 ns > tCK(avg) >= 1.5   ns)
		 **  3      8   (1.5 ns   > tCK(avg) >= 1.25  ns)
		 **  4      9   (1.25 ns  > tCK(avg) >= 1.07  ns)
		 **  5     10   (1.07 ns  > tCK(avg) >= 0.935 ns)
		 **  6     11   (0.935 ns > tCK(avg) >= 0.833 ns)
		 **  7     12   (0.833 ns > tCK(avg) >= 0.75  ns)
		 */

		mp0.s.cwl = 0;
		if (tclk_psecs < 2500)
			mp0.s.cwl = 1;
		if (tclk_psecs < 1875)
			mp0.s.cwl = 2;
		if (tclk_psecs < 1500)
			mp0.s.cwl = 3;
		if (tclk_psecs < 1250)
			mp0.s.cwl = 4;
		if (tclk_psecs < 1070)
			mp0.s.cwl = 5;
		if (tclk_psecs < 935)
			mp0.s.cwl = 6;
		if (tclk_psecs < 833)
			mp0.s.cwl = 7;
	}

	s = lookup_env(priv, "ddr_cwl");
	if (s)
		mp0.s.cwl = simple_strtoul(s, NULL, 0) - 5;

	if (ddr_type == DDR4_DRAM) {
		debug("%-45s : %d, [0x%x]\n", "CAS Write Latency CWL, [CSR]",
		      mp0.s.cwl + 9
		      + ((mp0.s.cwl > 2) ? (mp0.s.cwl - 3) * 2 : 0), mp0.s.cwl);
	} else {
		debug("%-45s : %d, [0x%x]\n", "CAS Write Latency CWL, [CSR]",
		      mp0.s.cwl + 5, mp0.s.cwl);
	}

	mp0.s.mprloc = 0;
	mp0.s.mpr = 0;
	mp0.s.dll = (ddr_type == DDR4_DRAM);	/* 0 for DDR3 and 1 for DDR4 */
	mp0.s.al = 0;
	mp0.s.wlev = 0;		/* Read Only */
	if (octeon_is_cpuid(OCTEON_CN70XX) || ddr_type == DDR4_DRAM)
		mp0.s.tdqs = 0;
	else
		mp0.s.tdqs = 1;
	mp0.s.qoff = 0;

	s = lookup_env(priv, "ddr_cl");
	if (s) {
		cl = simple_strtoul(s, NULL, 0);
		debug("CAS Latency                                   : %6d\n",
		      cl);
	}

	if (ddr_type == DDR4_DRAM) {
		mp0.s.cl = 0x0;
		if (cl > 9)
			mp0.s.cl = 0x1;
		if (cl > 10)
			mp0.s.cl = 0x2;
		if (cl > 11)
			mp0.s.cl = 0x3;
		if (cl > 12)
			mp0.s.cl = 0x4;
		if (cl > 13)
			mp0.s.cl = 0x5;
		if (cl > 14)
			mp0.s.cl = 0x6;
		if (cl > 15)
			mp0.s.cl = 0x7;
		if (cl > 16)
			mp0.s.cl = 0x8;
		if (cl > 18)
			mp0.s.cl = 0x9;
		if (cl > 20)
			mp0.s.cl = 0xA;
		if (cl > 24)
			mp0.s.cl = 0xB;
	} else {
		mp0.s.cl = 0x2;
		if (cl > 5)
			mp0.s.cl = 0x4;
		if (cl > 6)
			mp0.s.cl = 0x6;
		if (cl > 7)
			mp0.s.cl = 0x8;
		if (cl > 8)
			mp0.s.cl = 0xA;
		if (cl > 9)
			mp0.s.cl = 0xC;
		if (cl > 10)
			mp0.s.cl = 0xE;
		if (cl > 11)
			mp0.s.cl = 0x1;
		if (cl > 12)
			mp0.s.cl = 0x3;
		if (cl > 13)
			mp0.s.cl = 0x5;
		if (cl > 14)
			mp0.s.cl = 0x7;
		if (cl > 15)
			mp0.s.cl = 0x9;
	}

	mp0.s.rbt = 0;		/* Read Only. */
	mp0.s.tm = 0;
	mp0.s.dllr = 0;

	param = divide_roundup(twr, tclk_psecs);

	if (ddr_type == DDR4_DRAM) {	/* DDR4 */
		mp0.s.wrp = 1;
		if (param > 12)
			mp0.s.wrp = 2;
		if (param > 14)
			mp0.s.wrp = 3;
		if (param > 16)
			mp0.s.wrp = 4;
		if (param > 18)
			mp0.s.wrp = 5;
		if (param > 20)
			mp0.s.wrp = 6;
		if (param > 24)	/* RESERVED in DDR4 spec */
			mp0.s.wrp = 7;
	} else {		/* DDR3 */
		mp0.s.wrp = 1;
		if (param > 5)
			mp0.s.wrp = 2;
		if (param > 6)
			mp0.s.wrp = 3;
		if (param > 7)
			mp0.s.wrp = 4;
		if (param > 8)
			mp0.s.wrp = 5;
		if (param > 10)
			mp0.s.wrp = 6;
		if (param > 12)
			mp0.s.wrp = 7;
	}

	mp0.s.ppd = 0;

	s = lookup_env(priv, "ddr_wrp");
	if (s)
		mp0.s.wrp = simple_strtoul(s, NULL, 0);

	debug("%-45s : %d, [0x%x]\n",
	      "Write recovery for auto precharge WRP, [CSR]", param, mp0.s.wrp);

	s = lookup_env_ull(priv, "ddr_modereg_params0");
	if (s)
		mp0.u64 = simple_strtoull(s, NULL, 0);

	debug("MODEREG_PARAMS0                               : 0x%016llx\n",
	      mp0.u64);
	lmc_wr(priv, CVMX_LMCX_MODEREG_PARAMS0(if_num), mp0.u64);
}

static void lmc_modereg_params1(struct ddr_priv *priv)
{
	union cvmx_lmcx_modereg_params1 mp1;
	char *s;
	int i;

	mp1.u64 = odt_config[odt_idx].modereg_params1.u64;

	/*
	 * Special request: mismatched DIMM support. Slot 0: 2-Rank,
	 * Slot 1: 1-Rank
	 */
	if (rank_mask == 0x7) {	/* 2-Rank, 1-Rank */
		mp1.s.rtt_nom_00 = 0;
		mp1.s.rtt_nom_01 = 3;	/* rttnom_40ohm */
		mp1.s.rtt_nom_10 = 3;	/* rttnom_40ohm */
		mp1.s.rtt_nom_11 = 0;
		dyn_rtt_nom_mask = 0x6;
	}

	s = lookup_env(priv, "ddr_rtt_nom_mask");
	if (s)
		dyn_rtt_nom_mask = simple_strtoul(s, NULL, 0);

	/*
	 * Save the original rtt_nom settings before sweeping through
	 * settings.
	 */
	default_rtt_nom[0] = mp1.s.rtt_nom_00;
	default_rtt_nom[1] = mp1.s.rtt_nom_01;
	default_rtt_nom[2] = mp1.s.rtt_nom_10;
	default_rtt_nom[3] = mp1.s.rtt_nom_11;

	ddr_rtt_nom_auto = c_cfg->ddr_rtt_nom_auto;

	for (i = 0; i < 4; ++i) {
		u64 value;

		s = lookup_env(priv, "ddr_rtt_nom_%1d%1d", !!(i & 2),
			       !!(i & 1));
		if (!s)
			s = lookup_env(priv, "ddr%d_rtt_nom_%1d%1d", if_num,
				       !!(i & 2), !!(i & 1));
		if (s) {
			value = simple_strtoul(s, NULL, 0);
			mp1.u64 &= ~((u64)0x7 << (i * 12 + 9));
			mp1.u64 |= ((value & 0x7) << (i * 12 + 9));
			default_rtt_nom[i] = value;
			ddr_rtt_nom_auto = 0;
		}
	}

	s = lookup_env(priv, "ddr_rtt_nom");
	if (!s)
		s = lookup_env(priv, "ddr%d_rtt_nom", if_num);
	if (s) {
		u64 value;

		value = simple_strtoul(s, NULL, 0);

		if (dyn_rtt_nom_mask & 1) {
			default_rtt_nom[0] = value;
			mp1.s.rtt_nom_00 = value;
		}
		if (dyn_rtt_nom_mask & 2) {
			default_rtt_nom[1] = value;
			mp1.s.rtt_nom_01 = value;
		}
		if (dyn_rtt_nom_mask & 4) {
			default_rtt_nom[2] = value;
			mp1.s.rtt_nom_10 = value;
		}
		if (dyn_rtt_nom_mask & 8) {
			default_rtt_nom[3] = value;
			mp1.s.rtt_nom_11 = value;
		}

		ddr_rtt_nom_auto = 0;
	}

	for (i = 0; i < 4; ++i) {
		u64 value;

		s = lookup_env(priv, "ddr_rtt_wr_%1d%1d", !!(i & 2), !!(i & 1));
		if (!s)
			s = lookup_env(priv, "ddr%d_rtt_wr_%1d%1d", if_num,
				       !!(i & 2), !!(i & 1));
		if (s) {
			value = simple_strtoul(s, NULL, 0);
			insrt_wr(&mp1.u64, i, value);
		}
	}

	// Make sure 78XX pass 1 has valid RTT_WR settings, because
	// configuration files may be set-up for later chips, and
	// 78XX pass 1 supports no RTT_WR extension bits
	if (octeon_is_cpuid(OCTEON_CN78XX_PASS1_X)) {
		for (i = 0; i < 4; ++i) {
			// if 80 or undefined
			if (extr_wr(mp1.u64, i) > 3) {
				// FIXME? always insert 120
				insrt_wr(&mp1.u64, i, 1);
				debug("RTT_WR_%d%d set to 120 for CN78XX pass 1\n",
				      !!(i & 2), i & 1);
			}
		}
	}

	s = lookup_env(priv, "ddr_dic");
	if (s) {
		u64 value = simple_strtoul(s, NULL, 0);

		for (i = 0; i < 4; ++i) {
			mp1.u64 &= ~((u64)0x3 << (i * 12 + 7));
			mp1.u64 |= ((value & 0x3) << (i * 12 + 7));
		}
	}

	for (i = 0; i < 4; ++i) {
		u64 value;

		s = lookup_env(priv, "ddr_dic_%1d%1d", !!(i & 2), !!(i & 1));
		if (s) {
			value = simple_strtoul(s, NULL, 0);
			mp1.u64 &= ~((u64)0x3 << (i * 12 + 7));
			mp1.u64 |= ((value & 0x3) << (i * 12 + 7));
		}
	}

	s = lookup_env_ull(priv, "ddr_modereg_params1");
	if (s)
		mp1.u64 = simple_strtoull(s, NULL, 0);

	debug("RTT_NOM     %3d, %3d, %3d, %3d ohms           :  %x,%x,%x,%x\n",
	      imp_val->rtt_nom_ohms[mp1.s.rtt_nom_11],
	      imp_val->rtt_nom_ohms[mp1.s.rtt_nom_10],
	      imp_val->rtt_nom_ohms[mp1.s.rtt_nom_01],
	      imp_val->rtt_nom_ohms[mp1.s.rtt_nom_00],
	      mp1.s.rtt_nom_11,
	      mp1.s.rtt_nom_10, mp1.s.rtt_nom_01, mp1.s.rtt_nom_00);

	debug("RTT_WR      %3d, %3d, %3d, %3d ohms           :  %x,%x,%x,%x\n",
	      imp_val->rtt_wr_ohms[extr_wr(mp1.u64, 3)],
	      imp_val->rtt_wr_ohms[extr_wr(mp1.u64, 2)],
	      imp_val->rtt_wr_ohms[extr_wr(mp1.u64, 1)],
	      imp_val->rtt_wr_ohms[extr_wr(mp1.u64, 0)],
	      extr_wr(mp1.u64, 3),
	      extr_wr(mp1.u64, 2), extr_wr(mp1.u64, 1), extr_wr(mp1.u64, 0));

	debug("DIC         %3d, %3d, %3d, %3d ohms           :  %x,%x,%x,%x\n",
	      imp_val->dic_ohms[mp1.s.dic_11],
	      imp_val->dic_ohms[mp1.s.dic_10],
	      imp_val->dic_ohms[mp1.s.dic_01],
	      imp_val->dic_ohms[mp1.s.dic_00],
	      mp1.s.dic_11, mp1.s.dic_10, mp1.s.dic_01, mp1.s.dic_00);

	debug("MODEREG_PARAMS1                               : 0x%016llx\n",
	      mp1.u64);
	lmc_wr(priv, CVMX_LMCX_MODEREG_PARAMS1(if_num), mp1.u64);
}

static void lmc_modereg_params2(struct ddr_priv *priv)
{
	char *s;
	int i;

	if (ddr_type == DDR4_DRAM) {
		union cvmx_lmcx_modereg_params2 mp2;

		mp2.u64 = odt_config[odt_idx].modereg_params2.u64;

		s = lookup_env(priv, "ddr_rtt_park");
		if (s) {
			u64 value = simple_strtoul(s, NULL, 0);

			for (i = 0; i < 4; ++i) {
				mp2.u64 &= ~((u64)0x7 << (i * 10 + 0));
				mp2.u64 |= ((value & 0x7) << (i * 10 + 0));
			}
		}

		for (i = 0; i < 4; ++i) {
			u64 value;

			s = lookup_env(priv, "ddr_rtt_park_%1d%1d", !!(i & 2),
				       !!(i & 1));
			if (s) {
				value = simple_strtoul(s, NULL, 0);
				mp2.u64 &= ~((u64)0x7 << (i * 10 + 0));
				mp2.u64 |= ((value & 0x7) << (i * 10 + 0));
			}
		}

		s = lookup_env_ull(priv, "ddr_modereg_params2");
		if (s)
			mp2.u64 = simple_strtoull(s, NULL, 0);

		debug("RTT_PARK    %3d, %3d, %3d, %3d ohms           :  %x,%x,%x,%x\n",
		      imp_val->rtt_nom_ohms[mp2.s.rtt_park_11],
		      imp_val->rtt_nom_ohms[mp2.s.rtt_park_10],
		      imp_val->rtt_nom_ohms[mp2.s.rtt_park_01],
		      imp_val->rtt_nom_ohms[mp2.s.rtt_park_00],
		      mp2.s.rtt_park_11, mp2.s.rtt_park_10, mp2.s.rtt_park_01,
		      mp2.s.rtt_park_00);

		debug("%-45s :  0x%x,0x%x,0x%x,0x%x\n", "VREF_RANGE",
		      mp2.s.vref_range_11,
		      mp2.s.vref_range_10,
		      mp2.s.vref_range_01, mp2.s.vref_range_00);

		debug("%-45s :  0x%x,0x%x,0x%x,0x%x\n", "VREF_VALUE",
		      mp2.s.vref_value_11,
		      mp2.s.vref_value_10,
		      mp2.s.vref_value_01, mp2.s.vref_value_00);

		debug("MODEREG_PARAMS2                               : 0x%016llx\n",
		      mp2.u64);
		lmc_wr(priv, CVMX_LMCX_MODEREG_PARAMS2(if_num), mp2.u64);
	}
}

static void lmc_modereg_params3(struct ddr_priv *priv)
{
	char *s;

	if (ddr_type == DDR4_DRAM) {
		union cvmx_lmcx_modereg_params3 mp3;

		mp3.u64 = lmc_rd(priv, CVMX_LMCX_MODEREG_PARAMS3(if_num));
		/* Disable as workaround to Errata 20547 */
		mp3.s.rd_dbi = 0;
		mp3.s.tccd_l = max(divide_roundup(ddr4_tccd_lmin, tclk_psecs),
				   5ull) - 4;

		s = lookup_env(priv, "ddr_rd_preamble");
		if (s)
			mp3.s.rd_preamble = !!simple_strtoul(s, NULL, 0);

		if (!octeon_is_cpuid(OCTEON_CN78XX_PASS1_X)) {
			int delay = 0;

			if (lranks_per_prank == 4 && ddr_hertz >= 1000000000)
				delay = 1;

			mp3.s.xrank_add_tccd_l = delay;
			mp3.s.xrank_add_tccd_s = delay;
		}

		lmc_wr(priv, CVMX_LMCX_MODEREG_PARAMS3(if_num), mp3.u64);
		debug("MODEREG_PARAMS3                               : 0x%016llx\n",
		      mp3.u64);
	}
}

static void lmc_nxm(struct ddr_priv *priv)
{
	union cvmx_lmcx_nxm lmc_nxm;
	int num_bits = row_lsb + row_bits + lranks_bits - 26;
	char *s;

	lmc_nxm.u64 = lmc_rd(priv, CVMX_LMCX_NXM(if_num));

	/* .cn78xx. */
	if (rank_mask & 0x1)
		lmc_nxm.cn78xx.mem_msb_d0_r0 = num_bits;
	if (rank_mask & 0x2)
		lmc_nxm.cn78xx.mem_msb_d0_r1 = num_bits;
	if (rank_mask & 0x4)
		lmc_nxm.cn78xx.mem_msb_d1_r0 = num_bits;
	if (rank_mask & 0x8)
		lmc_nxm.cn78xx.mem_msb_d1_r1 = num_bits;

	/* Set the mask for non-existent ranks. */
	lmc_nxm.cn78xx.cs_mask = ~rank_mask & 0xff;

	s = lookup_env_ull(priv, "ddr_nxm");
	if (s)
		lmc_nxm.u64 = simple_strtoull(s, NULL, 0);

	debug("LMC_NXM                                       : 0x%016llx\n",
	      lmc_nxm.u64);
	lmc_wr(priv, CVMX_LMCX_NXM(if_num), lmc_nxm.u64);
}

static void lmc_wodt_mask(struct ddr_priv *priv)
{
	union cvmx_lmcx_wodt_mask wodt_mask;
	char *s;

	wodt_mask.u64 = odt_config[odt_idx].odt_mask;

	s = lookup_env_ull(priv, "ddr_wodt_mask");
	if (s)
		wodt_mask.u64 = simple_strtoull(s, NULL, 0);

	debug("WODT_MASK                                     : 0x%016llx\n",
	      wodt_mask.u64);
	lmc_wr(priv, CVMX_LMCX_WODT_MASK(if_num), wodt_mask.u64);
}

static void lmc_rodt_mask(struct ddr_priv *priv)
{
	union cvmx_lmcx_rodt_mask rodt_mask;
	int rankx;
	char *s;

	rodt_mask.u64 = odt_config[odt_idx].rodt_ctl;

	s = lookup_env_ull(priv, "ddr_rodt_mask");
	if (s)
		rodt_mask.u64 = simple_strtoull(s, NULL, 0);

	debug("%-45s : 0x%016llx\n", "RODT_MASK", rodt_mask.u64);
	lmc_wr(priv, CVMX_LMCX_RODT_MASK(if_num), rodt_mask.u64);

	dyn_rtt_nom_mask = 0;
	for (rankx = 0; rankx < dimm_count * 4; rankx++) {
		if (!(rank_mask & (1 << rankx)))
			continue;
		dyn_rtt_nom_mask |= ((rodt_mask.u64 >> (8 * rankx)) & 0xff);
	}
	if (num_ranks == 4) {
		/*
		 * Normally ODT1 is wired to rank 1. For quad-ranked DIMMs
		 * ODT1 is wired to the third rank (rank 2).  The mask,
		 * dyn_rtt_nom_mask, is used to indicate for which ranks
		 * to sweep RTT_NOM during read-leveling. Shift the bit
		 * from the ODT1 position over to the "ODT2" position so
		 * that the read-leveling analysis comes out right.
		 */
		int odt1_bit = dyn_rtt_nom_mask & 2;

		dyn_rtt_nom_mask &= ~2;
		dyn_rtt_nom_mask |= odt1_bit << 1;
	}
	debug("%-45s : 0x%02x\n", "DYN_RTT_NOM_MASK", dyn_rtt_nom_mask);
}

static void lmc_comp_ctl2(struct ddr_priv *priv)
{
	union cvmx_lmcx_comp_ctl2 cc2;
	char *s;

	cc2.u64 = lmc_rd(priv, CVMX_LMCX_COMP_CTL2(if_num));

	cc2.cn78xx.dqx_ctl = odt_config[odt_idx].odt_ena;
	/* Default 4=34.3 ohm */
	cc2.cn78xx.ck_ctl = (c_cfg->ck_ctl == 0) ? 4 : c_cfg->ck_ctl;
	/* Default 4=34.3 ohm */
	cc2.cn78xx.cmd_ctl = (c_cfg->cmd_ctl == 0) ? 4 : c_cfg->cmd_ctl;
	/* Default 4=34.3 ohm */
	cc2.cn78xx.control_ctl = (c_cfg->ctl_ctl == 0) ? 4 : c_cfg->ctl_ctl;

	ddr_rodt_ctl_auto = c_cfg->ddr_rodt_ctl_auto;
	s = lookup_env(priv, "ddr_rodt_ctl_auto");
	if (s)
		ddr_rodt_ctl_auto = !!simple_strtoul(s, NULL, 0);

	default_rodt_ctl = odt_config[odt_idx].qs_dic;
	s = lookup_env(priv, "ddr_rodt_ctl");
	if (!s)
		s = lookup_env(priv, "ddr%d_rodt_ctl", if_num);
	if (s) {
		default_rodt_ctl = simple_strtoul(s, NULL, 0);
		ddr_rodt_ctl_auto = 0;
	}

	cc2.cn70xx.rodt_ctl = default_rodt_ctl;

	// if DDR4, force CK_CTL to 26 ohms if it is currently 34 ohms,
	// and DCLK speed is 1 GHz or more...
	if (ddr_type == DDR4_DRAM && cc2.s.ck_ctl == ddr4_driver_34_ohm &&
	    ddr_hertz >= 1000000000) {
		// lowest for DDR4 is 26 ohms
		cc2.s.ck_ctl = ddr4_driver_26_ohm;
		debug("N%d.LMC%d: Forcing DDR4 COMP_CTL2[CK_CTL] to %d, %d ohms\n",
		      node, if_num, cc2.s.ck_ctl,
		      imp_val->drive_strength[cc2.s.ck_ctl]);
	}

	// if DDR4, 2DPC, UDIMM, force CONTROL_CTL and CMD_CTL to 26 ohms,
	// if DCLK speed is 1 GHz or more...
	if (ddr_type == DDR4_DRAM && dimm_count == 2 &&
	    (spd_dimm_type == 2 || spd_dimm_type == 6) &&
	    ddr_hertz >= 1000000000) {
		// lowest for DDR4 is 26 ohms
		cc2.cn78xx.control_ctl = ddr4_driver_26_ohm;
		// lowest for DDR4 is 26 ohms
		cc2.cn78xx.cmd_ctl = ddr4_driver_26_ohm;
		debug("N%d.LMC%d: Forcing DDR4 COMP_CTL2[CONTROL_CTL,CMD_CTL] to %d, %d ohms\n",
		      node, if_num, ddr4_driver_26_ohm,
		      imp_val->drive_strength[ddr4_driver_26_ohm]);
	}

	s = lookup_env(priv, "ddr_ck_ctl");
	if (s)
		cc2.cn78xx.ck_ctl = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_cmd_ctl");
	if (s)
		cc2.cn78xx.cmd_ctl = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_control_ctl");
	if (s)
		cc2.cn70xx.control_ctl = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_dqx_ctl");
	if (s)
		cc2.cn78xx.dqx_ctl = simple_strtoul(s, NULL, 0);

	debug("%-45s : %d, %d ohms\n", "DQX_CTL           ", cc2.cn78xx.dqx_ctl,
	      imp_val->drive_strength[cc2.cn78xx.dqx_ctl]);
	debug("%-45s : %d, %d ohms\n", "CK_CTL            ", cc2.cn78xx.ck_ctl,
	      imp_val->drive_strength[cc2.cn78xx.ck_ctl]);
	debug("%-45s : %d, %d ohms\n", "CMD_CTL           ", cc2.cn78xx.cmd_ctl,
	      imp_val->drive_strength[cc2.cn78xx.cmd_ctl]);
	debug("%-45s : %d, %d ohms\n", "CONTROL_CTL       ",
	      cc2.cn78xx.control_ctl,
	      imp_val->drive_strength[cc2.cn78xx.control_ctl]);
	debug("Read ODT_CTL                                  : 0x%x (%d ohms)\n",
	      cc2.cn78xx.rodt_ctl, imp_val->rodt_ohms[cc2.cn78xx.rodt_ctl]);

	debug("%-45s : 0x%016llx\n", "COMP_CTL2", cc2.u64);
	lmc_wr(priv, CVMX_LMCX_COMP_CTL2(if_num), cc2.u64);
}

static void lmc_phy_ctl(struct ddr_priv *priv)
{
	union cvmx_lmcx_phy_ctl phy_ctl;

	phy_ctl.u64 = lmc_rd(priv, CVMX_LMCX_PHY_CTL(if_num));
	phy_ctl.s.ts_stagger = 0;
	// FIXME: are there others TBD?
	phy_ctl.s.dsk_dbg_overwrt_ena = 0;

	if (!octeon_is_cpuid(OCTEON_CN78XX_PASS1_X) && lranks_per_prank > 1) {
		// C0 is TEN, C1 is A17
		phy_ctl.s.c0_sel = 2;
		phy_ctl.s.c1_sel = 2;
		debug("N%d.LMC%d: 3DS: setting PHY_CTL[cx_csel] = %d\n",
		      node, if_num, phy_ctl.s.c1_sel);
	}

	debug("PHY_CTL                                       : 0x%016llx\n",
	      phy_ctl.u64);
	lmc_wr(priv, CVMX_LMCX_PHY_CTL(if_num), phy_ctl.u64);
}

static void lmc_ext_config(struct ddr_priv *priv)
{
	union cvmx_lmcx_ext_config ext_cfg;
	char *s;

	ext_cfg.u64 = lmc_rd(priv, CVMX_LMCX_EXT_CONFIG(if_num));
	ext_cfg.s.vrefint_seq_deskew = 0;
	ext_cfg.s.read_ena_bprch = 1;
	ext_cfg.s.read_ena_fprch = 1;
	ext_cfg.s.drive_ena_fprch = 1;
	ext_cfg.s.drive_ena_bprch = 1;
	// make sure this is OFF for all current chips
	ext_cfg.s.invert_data = 0;

	s = lookup_env(priv, "ddr_read_fprch");
	if (s)
		ext_cfg.s.read_ena_fprch = strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_read_bprch");
	if (s)
		ext_cfg.s.read_ena_bprch = strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_drive_fprch");
	if (s)
		ext_cfg.s.drive_ena_fprch = strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_drive_bprch");
	if (s)
		ext_cfg.s.drive_ena_bprch = strtoul(s, NULL, 0);

	if (!octeon_is_cpuid(OCTEON_CN78XX_PASS1_X) && lranks_per_prank > 1) {
		ext_cfg.s.dimm0_cid = lranks_bits;
		ext_cfg.s.dimm1_cid = lranks_bits;
		debug("N%d.LMC%d: 3DS: setting EXT_CONFIG[dimmx_cid] = %d\n",
		      node, if_num, ext_cfg.s.dimm0_cid);
	}

	lmc_wr(priv, CVMX_LMCX_EXT_CONFIG(if_num), ext_cfg.u64);
	debug("%-45s : 0x%016llx\n", "EXT_CONFIG", ext_cfg.u64);
}

static void lmc_ext_config2(struct ddr_priv *priv)
{
	char *s;

	// NOTE: all chips have this register, but not necessarily the
	// fields we modify...
	if (!octeon_is_cpuid(OCTEON_CN78XX_PASS1_X) &&
	    !octeon_is_cpuid(OCTEON_CN73XX)) {
		union cvmx_lmcx_ext_config2 ext_cfg2;
		int value = 1;	// default to 1

		ext_cfg2.u64 = lmc_rd(priv, CVMX_LMCX_EXT_CONFIG2(if_num));

		s = lookup_env(priv, "ddr_ext2_delay_unload");
		if (s)
			value = !!simple_strtoul(s, NULL, 0);

		ext_cfg2.s.delay_unload_r0 = value;
		ext_cfg2.s.delay_unload_r1 = value;
		ext_cfg2.s.delay_unload_r2 = value;
		ext_cfg2.s.delay_unload_r3 = value;

		lmc_wr(priv, CVMX_LMCX_EXT_CONFIG2(if_num), ext_cfg2.u64);
		debug("%-45s : 0x%016llx\n", "EXT_CONFIG2", ext_cfg2.u64);
	}
}

static void lmc_dimm01_params_loop(struct ddr_priv *priv)
{
	union cvmx_lmcx_dimmx_params dimm_p;
	int dimmx = didx;
	char *s;
	int rc;
	int i;

	dimm_p.u64 = lmc_rd(priv, CVMX_LMCX_DIMMX_PARAMS(dimmx, if_num));

	if (ddr_type == DDR4_DRAM) {
		union cvmx_lmcx_dimmx_ddr4_params0 ddr4_p0;
		union cvmx_lmcx_dimmx_ddr4_params1 ddr4_p1;
		union cvmx_lmcx_ddr4_dimm_ctl ddr4_ctl;

		dimm_p.s.rc0 = 0;
		dimm_p.s.rc1 = 0;
		dimm_p.s.rc2 = 0;

		rc = read_spd(&dimm_config_table[didx], 0,
			      DDR4_SPD_RDIMM_REGISTER_DRIVE_STRENGTH_CTL);
		dimm_p.s.rc3 = (rc >> 4) & 0xf;
		dimm_p.s.rc4 = ((rc >> 0) & 0x3) << 2;
		dimm_p.s.rc4 |= ((rc >> 2) & 0x3) << 0;

		rc = read_spd(&dimm_config_table[didx], 0,
			      DDR4_SPD_RDIMM_REGISTER_DRIVE_STRENGTH_CK);
		dimm_p.s.rc5 = ((rc >> 0) & 0x3) << 2;
		dimm_p.s.rc5 |= ((rc >> 2) & 0x3) << 0;

		dimm_p.s.rc6 = 0;
		dimm_p.s.rc7 = 0;
		dimm_p.s.rc8 = 0;
		dimm_p.s.rc9 = 0;

		/*
		 * rc10               DDR4 RDIMM Operating Speed
		 * ===  ===================================================
		 *  0               tclk_psecs >= 1250 psec DDR4-1600 (1250 ps)
		 *  1   1250 psec > tclk_psecs >= 1071 psec DDR4-1866 (1071 ps)
		 *  2   1071 psec > tclk_psecs >=  938 psec DDR4-2133 ( 938 ps)
		 *  3    938 psec > tclk_psecs >=  833 psec DDR4-2400 ( 833 ps)
		 *  4    833 psec > tclk_psecs >=  750 psec DDR4-2666 ( 750 ps)
		 *  5    750 psec > tclk_psecs >=  625 psec DDR4-3200 ( 625 ps)
		 */
		dimm_p.s.rc10 = 0;
		if (tclk_psecs < 1250)
			dimm_p.s.rc10 = 1;
		if (tclk_psecs < 1071)
			dimm_p.s.rc10 = 2;
		if (tclk_psecs < 938)
			dimm_p.s.rc10 = 3;
		if (tclk_psecs < 833)
			dimm_p.s.rc10 = 4;
		if (tclk_psecs < 750)
			dimm_p.s.rc10 = 5;

		dimm_p.s.rc11 = 0;
		dimm_p.s.rc12 = 0;
		/* 0=LRDIMM, 1=RDIMM */
		dimm_p.s.rc13 = (spd_dimm_type == 4) ? 0 : 4;
		dimm_p.s.rc13 |= (ddr_type == DDR4_DRAM) ?
			(spd_addr_mirror << 3) : 0;
		dimm_p.s.rc14 = 0;
		dimm_p.s.rc15 = 0;	/* 1 nCK latency adder */

		ddr4_p0.u64 = 0;

		ddr4_p0.s.rc8x = 0;
		ddr4_p0.s.rc7x = 0;
		ddr4_p0.s.rc6x = 0;
		ddr4_p0.s.rc5x = 0;
		ddr4_p0.s.rc4x = 0;

		ddr4_p0.s.rc3x = compute_rc3x(tclk_psecs);

		ddr4_p0.s.rc2x = 0;
		ddr4_p0.s.rc1x = 0;

		ddr4_p1.u64 = 0;

		ddr4_p1.s.rcbx = 0;
		ddr4_p1.s.rcax = 0;
		ddr4_p1.s.rc9x = 0;

		ddr4_ctl.u64 = 0;
		ddr4_ctl.cn70xx.ddr4_dimm0_wmask = 0x004;
		ddr4_ctl.cn70xx.ddr4_dimm1_wmask =
		    (dimm_count > 1) ? 0x004 : 0x0000;

		/*
		 * Handle any overrides from envvars here...
		 */
		s = lookup_env(priv, "ddr_ddr4_params0");
		if (s)
			ddr4_p0.u64 = simple_strtoul(s, NULL, 0);

		s = lookup_env(priv, "ddr_ddr4_params1");
		if (s)
			ddr4_p1.u64 = simple_strtoul(s, NULL, 0);

		s = lookup_env(priv, "ddr_ddr4_dimm_ctl");
		if (s)
			ddr4_ctl.u64 = simple_strtoul(s, NULL, 0);

		for (i = 0; i < 11; ++i) {
			u64 value;

			s = lookup_env(priv, "ddr_ddr4_rc%1xx", i + 1);
			if (s) {
				value = simple_strtoul(s, NULL, 0);
				if (i < 8) {
					ddr4_p0.u64 &= ~((u64)0xff << (i * 8));
					ddr4_p0.u64 |= (value << (i * 8));
				} else {
					ddr4_p1.u64 &=
					    ~((u64)0xff << ((i - 8) * 8));
					ddr4_p1.u64 |= (value << ((i - 8) * 8));
				}
			}
		}

		/*
		 * write the final CSR values
		 */
		lmc_wr(priv, CVMX_LMCX_DIMMX_DDR4_PARAMS0(dimmx, if_num),
		       ddr4_p0.u64);

		lmc_wr(priv, CVMX_LMCX_DDR4_DIMM_CTL(if_num), ddr4_ctl.u64);

		lmc_wr(priv, CVMX_LMCX_DIMMX_DDR4_PARAMS1(dimmx, if_num),
		       ddr4_p1.u64);

		debug("DIMM%d Register Control Words        RCBx:RC1x : %x %x %x %x %x %x %x %x %x %x %x\n",
		      dimmx, ddr4_p1.s.rcbx, ddr4_p1.s.rcax,
		      ddr4_p1.s.rc9x, ddr4_p0.s.rc8x,
		      ddr4_p0.s.rc7x, ddr4_p0.s.rc6x,
		      ddr4_p0.s.rc5x, ddr4_p0.s.rc4x,
		      ddr4_p0.s.rc3x, ddr4_p0.s.rc2x, ddr4_p0.s.rc1x);

	} else {
		rc = read_spd(&dimm_config_table[didx], 0, 69);
		dimm_p.s.rc0 = (rc >> 0) & 0xf;
		dimm_p.s.rc1 = (rc >> 4) & 0xf;

		rc = read_spd(&dimm_config_table[didx], 0, 70);
		dimm_p.s.rc2 = (rc >> 0) & 0xf;
		dimm_p.s.rc3 = (rc >> 4) & 0xf;

		rc = read_spd(&dimm_config_table[didx], 0, 71);
		dimm_p.s.rc4 = (rc >> 0) & 0xf;
		dimm_p.s.rc5 = (rc >> 4) & 0xf;

		rc = read_spd(&dimm_config_table[didx], 0, 72);
		dimm_p.s.rc6 = (rc >> 0) & 0xf;
		dimm_p.s.rc7 = (rc >> 4) & 0xf;

		rc = read_spd(&dimm_config_table[didx], 0, 73);
		dimm_p.s.rc8 = (rc >> 0) & 0xf;
		dimm_p.s.rc9 = (rc >> 4) & 0xf;

		rc = read_spd(&dimm_config_table[didx], 0, 74);
		dimm_p.s.rc10 = (rc >> 0) & 0xf;
		dimm_p.s.rc11 = (rc >> 4) & 0xf;

		rc = read_spd(&dimm_config_table[didx], 0, 75);
		dimm_p.s.rc12 = (rc >> 0) & 0xf;
		dimm_p.s.rc13 = (rc >> 4) & 0xf;

		rc = read_spd(&dimm_config_table[didx], 0, 76);
		dimm_p.s.rc14 = (rc >> 0) & 0xf;
		dimm_p.s.rc15 = (rc >> 4) & 0xf;

		s = ddr_getenv_debug(priv, "ddr_clk_drive");
		if (s) {
			if (strcmp(s, "light") == 0)
				dimm_p.s.rc5 = 0x0;	/* Light Drive */
			if (strcmp(s, "moderate") == 0)
				dimm_p.s.rc5 = 0x5;	/* Moderate Drive */
			if (strcmp(s, "strong") == 0)
				dimm_p.s.rc5 = 0xA;	/* Strong Drive */
			printf("Parameter found in environment. ddr_clk_drive = %s\n",
			       s);
		}

		s = ddr_getenv_debug(priv, "ddr_cmd_drive");
		if (s) {
			if (strcmp(s, "light") == 0)
				dimm_p.s.rc3 = 0x0;	/* Light Drive */
			if (strcmp(s, "moderate") == 0)
				dimm_p.s.rc3 = 0x5;	/* Moderate Drive */
			if (strcmp(s, "strong") == 0)
				dimm_p.s.rc3 = 0xA;	/* Strong Drive */
			printf("Parameter found in environment. ddr_cmd_drive = %s\n",
			       s);
		}

		s = ddr_getenv_debug(priv, "ddr_ctl_drive");
		if (s) {
			if (strcmp(s, "light") == 0)
				dimm_p.s.rc4 = 0x0;	/* Light Drive */
			if (strcmp(s, "moderate") == 0)
				dimm_p.s.rc4 = 0x5;	/* Moderate Drive */
			printf("Parameter found in environment. ddr_ctl_drive = %s\n",
			       s);
		}

		/*
		 * rc10               DDR3 RDIMM Operating Speed
		 * ==   =====================================================
		 *  0               tclk_psecs >= 2500 psec DDR3/DDR3L-800 def
		 *  1   2500 psec > tclk_psecs >= 1875 psec DDR3/DDR3L-1066
		 *  2   1875 psec > tclk_psecs >= 1500 psec DDR3/DDR3L-1333
		 *  3   1500 psec > tclk_psecs >= 1250 psec DDR3/DDR3L-1600
		 *  4   1250 psec > tclk_psecs >= 1071 psec DDR3-1866
		 */
		dimm_p.s.rc10 = 0;
		if (tclk_psecs < 2500)
			dimm_p.s.rc10 = 1;
		if (tclk_psecs < 1875)
			dimm_p.s.rc10 = 2;
		if (tclk_psecs < 1500)
			dimm_p.s.rc10 = 3;
		if (tclk_psecs < 1250)
			dimm_p.s.rc10 = 4;
	}

	s = lookup_env(priv, "ddr_dimmx_params", i);
	if (s)
		dimm_p.u64 = simple_strtoul(s, NULL, 0);

	for (i = 0; i < 16; ++i) {
		u64 value;

		s = lookup_env(priv, "ddr_rc%d", i);
		if (s) {
			value = simple_strtoul(s, NULL, 0);
			dimm_p.u64 &= ~((u64)0xf << (i * 4));
			dimm_p.u64 |= (value << (i * 4));
		}
	}

	lmc_wr(priv, CVMX_LMCX_DIMMX_PARAMS(dimmx, if_num), dimm_p.u64);

	debug("DIMM%d Register Control Words         RC15:RC0 : %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
	      dimmx, dimm_p.s.rc15, dimm_p.s.rc14, dimm_p.s.rc13,
	      dimm_p.s.rc12, dimm_p.s.rc11, dimm_p.s.rc10,
	      dimm_p.s.rc9, dimm_p.s.rc8, dimm_p.s.rc7,
	      dimm_p.s.rc6, dimm_p.s.rc5, dimm_p.s.rc4,
	      dimm_p.s.rc3, dimm_p.s.rc2, dimm_p.s.rc1, dimm_p.s.rc0);

	// FIXME: recognize a DDR3 RDIMM with 4 ranks and 2 registers,
	// and treat it specially
	if (ddr_type == DDR3_DRAM && num_ranks == 4 &&
	    spd_rdimm_registers == 2 && dimmx == 0) {
		debug("DDR3: Copying DIMM0_PARAMS to DIMM1_PARAMS for pseudo-DIMM #1...\n");
		lmc_wr(priv, CVMX_LMCX_DIMMX_PARAMS(1, if_num), dimm_p.u64);
	}
}

static void lmc_dimm01_params(struct ddr_priv *priv)
{
	union cvmx_lmcx_dimm_ctl dimm_ctl;
	char *s;

	if (spd_rdimm) {
		for (didx = 0; didx < (unsigned int)dimm_count; ++didx)
			lmc_dimm01_params_loop(priv);

		if (ddr_type == DDR4_DRAM) {
			/* LMC0_DIMM_CTL */
			dimm_ctl.u64 = lmc_rd(priv, CVMX_LMCX_DIMM_CTL(if_num));
			dimm_ctl.s.dimm0_wmask = 0xdf3f;
			dimm_ctl.s.dimm1_wmask =
			    (dimm_count > 1) ? 0xdf3f : 0x0000;
			dimm_ctl.s.tcws = 0x4e0;
			dimm_ctl.s.parity = c_cfg->parity;

			s = lookup_env(priv, "ddr_dimm0_wmask");
			if (s) {
				dimm_ctl.s.dimm0_wmask =
				    simple_strtoul(s, NULL, 0);
			}

			s = lookup_env(priv, "ddr_dimm1_wmask");
			if (s) {
				dimm_ctl.s.dimm1_wmask =
				    simple_strtoul(s, NULL, 0);
			}

			s = lookup_env(priv, "ddr_dimm_ctl_parity");
			if (s)
				dimm_ctl.s.parity = simple_strtoul(s, NULL, 0);

			s = lookup_env(priv, "ddr_dimm_ctl_tcws");
			if (s)
				dimm_ctl.s.tcws = simple_strtoul(s, NULL, 0);

			debug("LMC DIMM_CTL                                  : 0x%016llx\n",
			      dimm_ctl.u64);
			lmc_wr(priv, CVMX_LMCX_DIMM_CTL(if_num), dimm_ctl.u64);

			/* Init RCW */
			oct3_ddr3_seq(priv, rank_mask, if_num, 0x7);

			/* Write RC0D last */
			dimm_ctl.s.dimm0_wmask = 0x2000;
			dimm_ctl.s.dimm1_wmask = (dimm_count > 1) ?
				0x2000 : 0x0000;
			debug("LMC DIMM_CTL                                  : 0x%016llx\n",
			      dimm_ctl.u64);
			lmc_wr(priv, CVMX_LMCX_DIMM_CTL(if_num), dimm_ctl.u64);

			/*
			 * Don't write any extended registers the second time
			 */
			lmc_wr(priv, CVMX_LMCX_DDR4_DIMM_CTL(if_num), 0);

			/* Init RCW */
			oct3_ddr3_seq(priv, rank_mask, if_num, 0x7);
		} else {
			/* LMC0_DIMM_CTL */
			dimm_ctl.u64 = lmc_rd(priv, CVMX_LMCX_DIMM_CTL(if_num));
			dimm_ctl.s.dimm0_wmask = 0xffff;
			// FIXME: recognize a DDR3 RDIMM with 4 ranks and 2
			// registers, and treat it specially
			if (num_ranks == 4 && spd_rdimm_registers == 2) {
				debug("DDR3: Activating DIMM_CTL[dimm1_mask] bits...\n");
				dimm_ctl.s.dimm1_wmask = 0xffff;
			} else {
				dimm_ctl.s.dimm1_wmask =
				    (dimm_count > 1) ? 0xffff : 0x0000;
			}
			dimm_ctl.s.tcws = 0x4e0;
			dimm_ctl.s.parity = c_cfg->parity;

			s = lookup_env(priv, "ddr_dimm0_wmask");
			if (s) {
				dimm_ctl.s.dimm0_wmask =
				    simple_strtoul(s, NULL, 0);
			}

			s = lookup_env(priv, "ddr_dimm1_wmask");
			if (s) {
				dimm_ctl.s.dimm1_wmask =
				    simple_strtoul(s, NULL, 0);
			}

			s = lookup_env(priv, "ddr_dimm_ctl_parity");
			if (s)
				dimm_ctl.s.parity = simple_strtoul(s, NULL, 0);

			s = lookup_env(priv, "ddr_dimm_ctl_tcws");
			if (s)
				dimm_ctl.s.tcws = simple_strtoul(s, NULL, 0);

			debug("LMC DIMM_CTL                                  : 0x%016llx\n",
			      dimm_ctl.u64);
			lmc_wr(priv, CVMX_LMCX_DIMM_CTL(if_num), dimm_ctl.u64);

			/* Init RCW */
			oct3_ddr3_seq(priv, rank_mask, if_num, 0x7);
		}

	} else {
		/* Disable register control writes for unbuffered */
		union cvmx_lmcx_dimm_ctl dimm_ctl;

		dimm_ctl.u64 = lmc_rd(priv, CVMX_LMCX_DIMM_CTL(if_num));
		dimm_ctl.s.dimm0_wmask = 0;
		dimm_ctl.s.dimm1_wmask = 0;
		lmc_wr(priv, CVMX_LMCX_DIMM_CTL(if_num), dimm_ctl.u64);
	}
}

static int lmc_rank_init(struct ddr_priv *priv)
{
	char *s;

	if (enable_by_rank_init) {
		by_rank = 3;
		saved_rank_mask = rank_mask;
	}

start_by_rank_init:

	if (enable_by_rank_init) {
		rank_mask = (1 << by_rank);
		if (!(rank_mask & saved_rank_mask))
			goto end_by_rank_init;
		if (by_rank == 0)
			rank_mask = saved_rank_mask;

		debug("\n>>>>> BY_RANK: starting rank %d with mask 0x%02x\n\n",
		      by_rank, rank_mask);
	}

	/*
	 * Comments (steps 3 through 5) continue in oct3_ddr3_seq()
	 */
	union cvmx_lmcx_modereg_params0 mp0;

	if (ddr_memory_preserved(priv)) {
		/*
		 * Contents are being preserved. Take DRAM out of self-refresh
		 * first. Then init steps can procede normally
		 */
		/* self-refresh exit */
		oct3_ddr3_seq(priv, rank_mask, if_num, 3);
	}

	mp0.u64 = lmc_rd(priv, CVMX_LMCX_MODEREG_PARAMS0(if_num));
	mp0.s.dllr = 1;		/* Set during first init sequence */
	lmc_wr(priv, CVMX_LMCX_MODEREG_PARAMS0(if_num), mp0.u64);

	ddr_init_seq(priv, rank_mask, if_num);

	mp0.s.dllr = 0;		/* Clear for normal operation */
	lmc_wr(priv, CVMX_LMCX_MODEREG_PARAMS0(if_num), mp0.u64);

	if (spd_rdimm && ddr_type == DDR4_DRAM &&
	    octeon_is_cpuid(OCTEON_CN7XXX)) {
		debug("Running init sequence 1\n");
		change_rdimm_mpr_pattern(priv, rank_mask, if_num, dimm_count);
	}

	memset(lanes, 0, sizeof(lanes));
	for (lane = 0; lane < last_lane; lane++) {
		// init all lanes to reset value
		dac_settings[lane] = 127;
	}

	// FIXME: disable internal VREF if deskew is disabled?
	if (disable_deskew_training) {
		debug("N%d.LMC%d: internal VREF Training disabled, leaving them in RESET.\n",
		      node, if_num);
		num_samples = 0;
	} else if (ddr_type == DDR4_DRAM &&
		   !octeon_is_cpuid(OCTEON_CN78XX_PASS1_X)) {
		num_samples = DEFAULT_DAC_SAMPLES;
	} else {
		// if DDR3 or no ability to write DAC values
		num_samples = 1;
	}

perform_internal_vref_training:

	total_dac_eval_retries = 0;
	dac_eval_exhausted = 0;

	for (sample = 0; sample < num_samples; sample++) {
		dac_eval_retries = 0;

		// make offset and internal vref training repeatable
		do {
			/*
			 * 6.9.8 LMC Offset Training
			 * LMC requires input-receiver offset training.
			 */
			perform_offset_training(priv, rank_mask, if_num);

			/*
			 * 6.9.9 LMC Internal vref Training
			 * LMC requires input-reference-voltage training.
			 */
			perform_internal_vref_training(priv, rank_mask, if_num);

			// read and maybe display the DAC values for a sample
			read_dac_dbi_settings(priv, if_num, /*DAC*/ 1,
					      dac_settings);
			if (num_samples == 1 || ddr_verbose(priv)) {
				display_dac_dbi_settings(if_num, /*DAC*/ 1,
							 use_ecc, dac_settings,
							 "Internal VREF");
			}

			// for DDR4, evaluate the DAC settings and retry
			// if any issues
			if (ddr_type == DDR4_DRAM) {
				if (evaluate_dac_settings
				    (if_64b, use_ecc, dac_settings)) {
					dac_eval_retries += 1;
					if (dac_eval_retries >
					    DAC_RETRIES_LIMIT) {
						debug("N%d.LMC%d: DDR4 internal VREF DAC settings: retries exhausted; continuing...\n",
						      node, if_num);
						dac_eval_exhausted += 1;
					} else {
						debug("N%d.LMC%d: DDR4 internal VREF DAC settings inconsistent; retrying....\n",
						      node, if_num);
						total_dac_eval_retries += 1;
						// try another sample
						continue;
					}
				}

				// taking multiple samples, otherwise do nothing
				if (num_samples > 1) {
					// good sample or exhausted retries,
					// record it
					for (lane = 0; lane < last_lane;
					     lane++) {
						lanes[lane].bytes[sample] =
						    dac_settings[lane];
					}
				}
			}
			// done if DDR3, or good sample, or exhausted retries
			break;
		} while (1);
	}

	if (ddr_type == DDR4_DRAM && dac_eval_exhausted > 0) {
		debug("N%d.LMC%d: DDR internal VREF DAC settings: total retries %d, exhausted %d\n",
		      node, if_num, total_dac_eval_retries, dac_eval_exhausted);
	}

	if (num_samples > 1) {
		debug("N%d.LMC%d: DDR4 internal VREF DAC settings: processing multiple samples...\n",
		      node, if_num);

		for (lane = 0; lane < last_lane; lane++) {
			dac_settings[lane] =
			    process_samples_average(&lanes[lane].bytes[0],
						    num_samples, if_num, lane);
		}
		display_dac_dbi_settings(if_num, /*DAC*/ 1, use_ecc,
					 dac_settings, "Averaged VREF");

		// finally, write the final DAC values
		for (lane = 0; lane < last_lane; lane++) {
			load_dac_override(priv, if_num, dac_settings[lane],
					  lane);
		}
	}

	// allow override of any byte-lane internal VREF
	int overrode_vref_dac = 0;

	for (lane = 0; lane < last_lane; lane++) {
		s = lookup_env(priv, "ddr%d_vref_dac_byte%d", if_num, lane);
		if (s) {
			dac_settings[lane] = simple_strtoul(s, NULL, 0);
			overrode_vref_dac = 1;
			// finally, write the new DAC value
			load_dac_override(priv, if_num, dac_settings[lane],
					  lane);
		}
	}
	if (overrode_vref_dac) {
		display_dac_dbi_settings(if_num, /*DAC*/ 1, use_ecc,
					 dac_settings, "Override VREF");
	}

	// as a second step, after internal VREF training, before starting
	// deskew training:
	// for DDR3 and OCTEON3 not O78 pass 1.x, override the DAC setting
	// to 127
	if (ddr_type == DDR3_DRAM && !octeon_is_cpuid(OCTEON_CN78XX_PASS1_X) &&
	    !disable_deskew_training) {
		load_dac_override(priv, if_num, 127, /* all */ 0x0A);
		debug("N%d.LMC%d: Overriding DDR3 internal VREF DAC settings to 127.\n",
		      node, if_num);
	}

	/*
	 * 4.8.8 LMC Deskew Training
	 *
	 * LMC requires input-read-data deskew training.
	 */
	if (!disable_deskew_training) {
		deskew_training_errors =
		    perform_deskew_training(priv, rank_mask, if_num,
					    spd_rawcard_aorb);

		// All the Deskew lock and saturation retries (may) have
		// been done, but we ended up with nibble errors; so,
		// as a last ditch effort, try the Internal vref
		// Training again...
		if (deskew_training_errors) {
			if (internal_retries <
			    DEFAULT_INTERNAL_VREF_TRAINING_LIMIT) {
				internal_retries++;
				debug("N%d.LMC%d: Deskew training results still unsettled - retrying internal vref training (%d)\n",
				      node, if_num, internal_retries);
				goto perform_internal_vref_training;
			} else {
				if (restart_if_dsk_incomplete) {
					debug("N%d.LMC%d: INFO: Deskew training incomplete - %d retries exhausted, Restarting LMC init...\n",
					      node, if_num, internal_retries);
					return -EAGAIN;
				}
				debug("N%d.LMC%d: Deskew training incomplete - %d retries exhausted, but continuing...\n",
				      node, if_num, internal_retries);
			}
		}		/* if (deskew_training_errors) */

		// FIXME: treat this as the final DSK print from now on,
		// and print if VBL_NORM or above also, save the results
		// of the original training in case we want them later
		validate_deskew_training(priv, rank_mask, if_num,
					 &deskew_training_results, 1);
	} else {		/* if (! disable_deskew_training) */
		debug("N%d.LMC%d: Deskew Training disabled, printing settings before HWL.\n",
		      node, if_num);
		validate_deskew_training(priv, rank_mask, if_num,
					 &deskew_training_results, 1);
	}			/* if (! disable_deskew_training) */

	if (enable_by_rank_init) {
		read_dac_dbi_settings(priv, if_num, /*dac */ 1,
				      &rank_dac[by_rank].bytes[0]);
		get_deskew_settings(priv, if_num, &rank_dsk[by_rank]);
		debug("\n>>>>> BY_RANK: ending rank %d\n\n", by_rank);
	}

end_by_rank_init:

	if (enable_by_rank_init) {
		//debug("\n>>>>> BY_RANK: ending rank %d\n\n", by_rank);

		by_rank--;
		if (by_rank >= 0)
			goto start_by_rank_init;

		rank_mask = saved_rank_mask;
		ddr_init_seq(priv, rank_mask, if_num);

		process_by_rank_dac(priv, if_num, rank_mask, rank_dac);
		process_by_rank_dsk(priv, if_num, rank_mask, rank_dsk);

		// FIXME: set this to prevent later checking!!!
		disable_deskew_training = 1;

		debug("\n>>>>> BY_RANK: FINISHED!!\n\n");
	}

	return 0;
}

static void lmc_config_2(struct ddr_priv *priv)
{
	union cvmx_lmcx_config lmc_config;
	int save_ref_zqcs_int;
	u64 temp_delay_usecs;

	lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));

	/*
	 * Temporarily select the minimum ZQCS interval and wait
	 * long enough for a few ZQCS calibrations to occur.  This
	 * should ensure that the calibration circuitry is
	 * stabilized before read/write leveling occurs.
	 */
	if (octeon_is_cpuid(OCTEON_CN7XXX)) {
		save_ref_zqcs_int = lmc_config.cn78xx.ref_zqcs_int;
		/* set smallest interval */
		lmc_config.cn78xx.ref_zqcs_int = 1 | (32 << 7);
	} else {
		save_ref_zqcs_int = lmc_config.cn63xx.ref_zqcs_int;
		/* set smallest interval */
		lmc_config.cn63xx.ref_zqcs_int = 1 | (32 << 7);
	}
	lmc_wr(priv, CVMX_LMCX_CONFIG(if_num), lmc_config.u64);
	lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));

	/*
	 * Compute an appropriate delay based on the current ZQCS
	 * interval. The delay should be long enough for the
	 * current ZQCS delay counter to expire plus ten of the
	 * minimum intarvals to ensure that some calibrations
	 * occur.
	 */
	temp_delay_usecs = (((u64)save_ref_zqcs_int >> 7) * tclk_psecs *
			    100 * 512 * 128) / (10000 * 10000) + 10 *
		((u64)32 * tclk_psecs * 100 * 512 * 128) / (10000 * 10000);

	debug("Waiting %lld usecs for ZQCS calibrations to start\n",
	      temp_delay_usecs);
	udelay(temp_delay_usecs);

	if (octeon_is_cpuid(OCTEON_CN7XXX)) {
		/* Restore computed interval */
		lmc_config.cn78xx.ref_zqcs_int = save_ref_zqcs_int;
	} else {
		/* Restore computed interval */
		lmc_config.cn63xx.ref_zqcs_int = save_ref_zqcs_int;
	}

	lmc_wr(priv, CVMX_LMCX_CONFIG(if_num), lmc_config.u64);
	lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
}

static union cvmx_lmcx_wlevel_ctl wl_ctl __section(".data");
static union cvmx_lmcx_wlevel_rankx wl_rank __section(".data");
static union cvmx_lmcx_modereg_params1 mp1 __section(".data");

static int wl_mask[9] __section(".data");
static int byte_idx __section(".data");
static int ecc_ena __section(".data");
static int wl_roundup __section(".data");
static int save_mode32b __section(".data");
static int disable_hwl_validity __section(".data");
static int default_wl_rtt_nom __section(".data");
static int wl_pbm_pump __section(".data");

static void lmc_write_leveling_loop(struct ddr_priv *priv, int rankx)
{
	int wloop = 0;
	// retries per sample for HW-related issues with bitmasks or values
	int wloop_retries = 0;
	int wloop_retries_total = 0;
	int wloop_retries_exhausted = 0;
#define WLOOP_RETRIES_DEFAULT 5
	int wl_val_err;
	int wl_mask_err_rank = 0;
	int wl_val_err_rank = 0;
	// array to collect counts of byte-lane values
	// assume low-order 3 bits and even, so really only 2-bit values
	struct wlevel_bitcnt wl_bytes[9], wl_bytes_extra[9];
	int extra_bumps, extra_mask;
	int rank_nom = 0;

	if (!(rank_mask & (1 << rankx)))
		return;

	if (match_wl_rtt_nom) {
		if (rankx == 0)
			rank_nom = mp1.s.rtt_nom_00;
		if (rankx == 1)
			rank_nom = mp1.s.rtt_nom_01;
		if (rankx == 2)
			rank_nom = mp1.s.rtt_nom_10;
		if (rankx == 3)
			rank_nom = mp1.s.rtt_nom_11;

		debug("N%d.LMC%d.R%d: Setting WLEVEL_CTL[rtt_nom] to %d (%d)\n",
		      node, if_num, rankx, rank_nom,
		      imp_val->rtt_nom_ohms[rank_nom]);
	}

	memset(wl_bytes, 0, sizeof(wl_bytes));
	memset(wl_bytes_extra, 0, sizeof(wl_bytes_extra));

	// restructure the looping so we can keep trying until we get the
	// samples we want
	while (wloop < wl_loops) {
		wl_ctl.u64 = lmc_rd(priv, CVMX_LMCX_WLEVEL_CTL(if_num));

		wl_ctl.cn78xx.rtt_nom =
		    (default_wl_rtt_nom > 0) ? (default_wl_rtt_nom - 1) : 7;

		if (match_wl_rtt_nom) {
			wl_ctl.cn78xx.rtt_nom =
			    (rank_nom > 0) ? (rank_nom - 1) : 7;
		}

		/* Clear write-level delays */
		lmc_wr(priv, CVMX_LMCX_WLEVEL_RANKX(rankx, if_num), 0);

		wl_mask_err = 0;	/* Reset error counters */
		wl_val_err = 0;

		for (byte_idx = 0; byte_idx < 9; ++byte_idx)
			wl_mask[byte_idx] = 0;	/* Reset bitmasks */

		// do all the byte-lanes at the same time
		wl_ctl.cn78xx.lanemask = 0x1ff;

		lmc_wr(priv, CVMX_LMCX_WLEVEL_CTL(if_num), wl_ctl.u64);

		/*
		 * Read and write values back in order to update the
		 * status field. This insures that we read the updated
		 * values after write-leveling has completed.
		 */
		lmc_wr(priv, CVMX_LMCX_WLEVEL_RANKX(rankx, if_num),
		       lmc_rd(priv, CVMX_LMCX_WLEVEL_RANKX(rankx, if_num)));

		/* write-leveling */
		oct3_ddr3_seq(priv, 1 << rankx, if_num, 6);

		do {
			wl_rank.u64 = lmc_rd(priv,
					     CVMX_LMCX_WLEVEL_RANKX(rankx,
								    if_num));
		} while (wl_rank.cn78xx.status != 3);

		wl_rank.u64 = lmc_rd(priv, CVMX_LMCX_WLEVEL_RANKX(rankx,
								  if_num));

		for (byte_idx = 0; byte_idx < (8 + ecc_ena); ++byte_idx) {
			wl_mask[byte_idx] = lmc_ddr3_wl_dbg_read(priv,
								 if_num,
								 byte_idx);
			if (wl_mask[byte_idx] == 0)
				++wl_mask_err;
		}

		// check validity only if no bitmask errors
		if (wl_mask_err == 0) {
			if ((spd_dimm_type == 1 || spd_dimm_type == 2) &&
			    dram_width != 16 && if_64b &&
			    !disable_hwl_validity) {
				// bypass if [mini|SO]-[RU]DIMM or x16 or
				// 32-bit
				wl_val_err =
				    validate_hw_wl_settings(if_num,
							    &wl_rank,
							    spd_rdimm, ecc_ena);
				wl_val_err_rank += (wl_val_err != 0);
			}
		} else {
			wl_mask_err_rank++;
		}

		// before we print, if we had bitmask or validity errors,
		// do a retry...
		if (wl_mask_err != 0 || wl_val_err != 0) {
			if (wloop_retries < WLOOP_RETRIES_DEFAULT) {
				wloop_retries++;
				wloop_retries_total++;
				// this printout is per-retry: only when VBL
				// is high enough (DEV?)
				// FIXME: do we want to show the bad bitmaps
				// or delays here also?
				debug("N%d.LMC%d.R%d: H/W Write-Leveling had %s errors - retrying...\n",
				      node, if_num, rankx,
				      (wl_mask_err) ? "Bitmask" : "Validity");
				// this takes us back to the top without
				// counting a sample
				return;
			}

			// retries exhausted, do not print at normal VBL
			debug("N%d.LMC%d.R%d: H/W Write-Leveling issues: %s errors\n",
			      node, if_num, rankx,
			      (wl_mask_err) ? "Bitmask" : "Validity");
			wloop_retries_exhausted++;
		}
		// no errors or exhausted retries, use this sample
		wloop_retries = 0;	//reset for next sample

		// when only 1 sample or forced, print the bitmasks then
		// current HW WL
		if (wl_loops == 1 || wl_print) {
			if (wl_print > 1)
				display_wl_bm(if_num, rankx, wl_mask);
			display_wl(if_num, wl_rank, rankx);
		}

		if (wl_roundup) {	/* Round up odd bitmask delays */
			for (byte_idx = 0; byte_idx < (8 + ecc_ena);
			     ++byte_idx) {
				if (!(if_bytemask & (1 << byte_idx)))
					return;
				upd_wl_rank(&wl_rank, byte_idx,
					    roundup_ddr3_wlevel_bitmask
					    (wl_mask[byte_idx]));
			}
			lmc_wr(priv, CVMX_LMCX_WLEVEL_RANKX(rankx, if_num),
			       wl_rank.u64);
			display_wl(if_num, wl_rank, rankx);
		}

		// OK, we have a decent sample, no bitmask or validity errors
		extra_bumps = 0;
		extra_mask = 0;
		for (byte_idx = 0; byte_idx < (8 + ecc_ena); ++byte_idx) {
			int ix;

			if (!(if_bytemask & (1 << byte_idx)))
				return;

			// increment count of byte-lane value
			// only 4 values
			ix = (get_wl_rank(&wl_rank, byte_idx) >> 1) & 3;
			wl_bytes[byte_idx].bitcnt[ix]++;
			wl_bytes_extra[byte_idx].bitcnt[ix]++;
			// if perfect...
			if (__builtin_popcount(wl_mask[byte_idx]) == 4) {
				wl_bytes_extra[byte_idx].bitcnt[ix] +=
				    wl_pbm_pump;
				extra_bumps++;
				extra_mask |= 1 << byte_idx;
			}
		}

		if (extra_bumps) {
			if (wl_print > 1) {
				debug("N%d.LMC%d.R%d: HWL sample had %d bumps (0x%02x).\n",
				      node, if_num, rankx, extra_bumps,
				      extra_mask);
			}
		}

		// if we get here, we have taken a decent sample
		wloop++;

	}			/* while (wloop < wl_loops) */

	// if we did sample more than once, try to pick a majority vote
	if (wl_loops > 1) {
		// look for the majority in each byte-lane
		for (byte_idx = 0; byte_idx < (8 + ecc_ena); ++byte_idx) {
			int mx, mc, xc, cc;
			int ix, alts;
			int maj, xmaj, xmx, xmc, xxc, xcc;

			if (!(if_bytemask & (1 << byte_idx)))
				return;
			maj = find_wl_majority(&wl_bytes[byte_idx], &mx,
					       &mc, &xc, &cc);
			xmaj = find_wl_majority(&wl_bytes_extra[byte_idx],
						&xmx, &xmc, &xxc, &xcc);
			if (maj != xmaj) {
				if (wl_print) {
					debug("N%d.LMC%d.R%d: Byte %d: HWL maj %d(%d), USING xmaj %d(%d)\n",
					      node, if_num, rankx,
					      byte_idx, maj, xc, xmaj, xxc);
				}
				mx = xmx;
				mc = xmc;
				xc = xxc;
				cc = xcc;
			}

			// see if there was an alternate
			// take out the majority choice
			alts = (mc & ~(1 << mx));
			if (alts != 0) {
				for (ix = 0; ix < 4; ix++) {
					// FIXME: could be done multiple times?
					// bad if so
					if (alts & (1 << ix)) {
						// set the mask
						hwl_alts[rankx].hwl_alt_mask |=
							(1 << byte_idx);
						// record the value
						hwl_alts[rankx].hwl_alt_delay[byte_idx] =
							ix << 1;
						if (wl_print > 1) {
							debug("N%d.LMC%d.R%d: SWL_TRY_HWL_ALT: Byte %d maj %d (%d) alt %d (%d).\n",
							      node,
							      if_num,
							      rankx,
							      byte_idx,
							      mx << 1,
							      xc,
							      ix << 1,
							      wl_bytes
							      [byte_idx].bitcnt
							      [ix]);
						}
					}
				}
			}

			if (cc > 2) {	// unlikely, but...
				// assume: counts for 3 indices are all 1
				// possiblities are: 0/2/4, 2/4/6, 0/4/6, 0/2/6
				// and the desired?:   2  ,   4  ,     6, 0
				// we choose the middle, assuming one of the
				// outliers is bad
				// NOTE: this is an ugly hack at the moment;
				// there must be a better way
				switch (mc) {
				case 0x7:
					mx = 1;
					break;	// was 0/2/4, choose 2
				case 0xb:
					mx = 0;
					break;	// was 0/2/6, choose 0
				case 0xd:
					mx = 3;
					break;	// was 0/4/6, choose 6
				case 0xe:
					mx = 2;
					break;	// was 2/4/6, choose 4
				default:
				case 0xf:
					mx = 1;
					break;	// was 0/2/4/6, choose 2?
				}
				printf("N%d.LMC%d.R%d: HW WL MAJORITY: bad byte-lane %d (0x%x), using %d.\n",
				       node, if_num, rankx, byte_idx, mc,
				       mx << 1);
			}
			upd_wl_rank(&wl_rank, byte_idx, mx << 1);
		}

		lmc_wr(priv, CVMX_LMCX_WLEVEL_RANKX(rankx, if_num),
		       wl_rank.u64);
		display_wl_with_final(if_num, wl_rank, rankx);

		// FIXME: does this help make the output a little easier
		// to focus?
		if (wl_print > 0)
			debug("-----------\n");

	}			/* if (wl_loops > 1) */

	// maybe print an error summary for the rank
	if (wl_mask_err_rank != 0 || wl_val_err_rank != 0) {
		debug("N%d.LMC%d.R%d: H/W Write-Leveling errors - %d bitmask, %d validity, %d retries, %d exhausted\n",
		      node, if_num, rankx, wl_mask_err_rank,
		      wl_val_err_rank, wloop_retries_total,
		      wloop_retries_exhausted);
	}
}

static void lmc_write_leveling(struct ddr_priv *priv)
{
	union cvmx_lmcx_config cfg;
	int rankx;
	char *s;

	/*
	 * 4.8.9 LMC Write Leveling
	 *
	 * LMC supports an automatic write leveling like that described in the
	 * JEDEC DDR3 specifications separately per byte-lane.
	 *
	 * All of DDR PLL, LMC CK, LMC DRESET, and early LMC initializations
	 * must be completed prior to starting this LMC write-leveling sequence.
	 *
	 * There are many possible procedures that will write-level all the
	 * attached DDR3 DRAM parts. One possibility is for software to simply
	 * write the desired values into LMC(0)_WLEVEL_RANK(0..3). This section
	 * describes one possible sequence that uses LMC's autowrite-leveling
	 * capabilities.
	 *
	 * 1. If the DQS/DQ delays on the board may be more than the ADD/CMD
	 *    delays, then ensure that LMC(0)_CONFIG[EARLY_DQX] is set at this
	 *    point.
	 *
	 * Do the remaining steps 2-7 separately for each rank i with attached
	 * DRAM.
	 *
	 * 2. Write LMC(0)_WLEVEL_RANKi = 0.
	 *
	 * 3. For x8 parts:
	 *
	 *    Without changing any other fields in LMC(0)_WLEVEL_CTL, write
	 *    LMC(0)_WLEVEL_CTL[LANEMASK] to select all byte lanes with attached
	 *    DRAM.
	 *
	 *    For x16 parts:
	 *
	 *    Without changing any other fields in LMC(0)_WLEVEL_CTL, write
	 *    LMC(0)_WLEVEL_CTL[LANEMASK] to select all even byte lanes with
	 *    attached DRAM.
	 *
	 * 4. Without changing any other fields in LMC(0)_CONFIG,
	 *
	 *    o write LMC(0)_SEQ_CTL[SEQ_SEL] to select write-leveling
	 *
	 *    o write LMC(0)_CONFIG[RANKMASK] = (1 << i)
	 *
	 *    o write LMC(0)_SEQ_CTL[INIT_START] = 1
	 *
	 *    LMC will initiate write-leveling at this point. Assuming
	 *    LMC(0)_WLEVEL_CTL [SSET] = 0, LMC first enables write-leveling on
	 *    the selected DRAM rank via a DDR3 MR1 write, then sequences
	 *    through
	 *    and accumulates write-leveling results for eight different delay
	 *    settings twice, starting at a delay of zero in this case since
	 *    LMC(0)_WLEVEL_RANKi[BYTE*<4:3>] = 0, increasing by 1/8 CK each
	 *    setting, covering a total distance of one CK, then disables the
	 *    write-leveling via another DDR3 MR1 write.
	 *
	 *    After the sequence through 16 delay settings is complete:
	 *
	 *    o LMC sets LMC(0)_WLEVEL_RANKi[STATUS] = 3
	 *
	 *    o LMC sets LMC(0)_WLEVEL_RANKi[BYTE*<2:0>] (for all ranks selected
	 *      by LMC(0)_WLEVEL_CTL[LANEMASK]) to indicate the first write
	 *      leveling result of 1 that followed result of 0 during the
	 *      sequence, except that the LMC always writes
	 *      LMC(0)_WLEVEL_RANKi[BYTE*<0>]=0.
	 *
	 *    o Software can read the eight write-leveling results from the
	 *      first pass through the delay settings by reading
	 *      LMC(0)_WLEVEL_DBG[BITMASK] (after writing
	 *      LMC(0)_WLEVEL_DBG[BYTE]). (LMC does not retain the writeleveling
	 *      results from the second pass through the eight delay
	 *      settings. They should often be identical to the
	 *      LMC(0)_WLEVEL_DBG[BITMASK] results, though.)
	 *
	 * 5. Wait until LMC(0)_WLEVEL_RANKi[STATUS] != 2.
	 *
	 *    LMC will have updated LMC(0)_WLEVEL_RANKi[BYTE*<2:0>] for all byte
	 *    lanes selected by LMC(0)_WLEVEL_CTL[LANEMASK] at this point.
	 *    LMC(0)_WLEVEL_RANKi[BYTE*<4:3>] will still be the value that
	 *    software wrote in substep 2 above, which is 0.
	 *
	 * 6. For x16 parts:
	 *
	 *    Without changing any other fields in LMC(0)_WLEVEL_CTL, write
	 *    LMC(0)_WLEVEL_CTL[LANEMASK] to select all odd byte lanes with
	 *    attached DRAM.
	 *
	 *    Repeat substeps 4 and 5 with this new LMC(0)_WLEVEL_CTL[LANEMASK]
	 *    setting. Skip to substep 7 if this has already been done.
	 *
	 *    For x8 parts:
	 *
	 *    Skip this substep. Go to substep 7.
	 *
	 * 7. Calculate LMC(0)_WLEVEL_RANKi[BYTE*<4:3>] settings for all byte
	 *    lanes on all ranks with attached DRAM.
	 *
	 *    At this point, all byte lanes on rank i with attached DRAM should
	 *    have been write-leveled, and LMC(0)_WLEVEL_RANKi[BYTE*<2:0>] has
	 *    the result for each byte lane.
	 *
	 *    But note that the DDR3 write-leveling sequence will only determine
	 *    the delay modulo the CK cycle time, and cannot determine how many
	 *    additional CK cycles of delay are present. Software must calculate
	 *    the number of CK cycles, or equivalently, the
	 *    LMC(0)_WLEVEL_RANKi[BYTE*<4:3>] settings.
	 *
	 *    This BYTE*<4:3> calculation is system/board specific.
	 *
	 * Many techniques can be used to calculate write-leveling BYTE*<4:3>
	 * values, including:
	 *
	 *    o Known values for some byte lanes.
	 *
	 *    o Relative values for some byte lanes relative to others.
	 *
	 *    For example, suppose lane X is likely to require a larger
	 *    write-leveling delay than lane Y. A BYTEX<2:0> value that is much
	 *    smaller than the BYTEY<2:0> value may then indicate that the
	 *    required lane X delay wrapped into the next CK, so BYTEX<4:3>
	 *    should be set to BYTEY<4:3>+1.
	 *
	 *    When ECC DRAM is not present (i.e. when DRAM is not attached to
	 *    the DDR_CBS_0_* and DDR_CB<7:0> chip signals, or the
	 *    DDR_DQS_<4>_* and DDR_DQ<35:32> chip signals), write
	 *    LMC(0)_WLEVEL_RANK*[BYTE8] = LMC(0)_WLEVEL_RANK*[BYTE0],
	 *    using the final calculated BYTE0 value.
	 *    Write LMC(0)_WLEVEL_RANK*[BYTE4] = LMC(0)_WLEVEL_RANK*[BYTE0],
	 *    using the final calculated BYTE0 value.
	 *
	 * 8. Initialize LMC(0)_WLEVEL_RANK* values for all unused ranks.
	 *
	 *    Let rank i be a rank with attached DRAM.
	 *
	 *    For all ranks j that do not have attached DRAM, set
	 *    LMC(0)_WLEVEL_RANKj = LMC(0)_WLEVEL_RANKi.
	 */

	rankx = 0;
	wl_roundup = 0;
	disable_hwl_validity = 0;

	// wl_pbm_pump: weight for write-leveling PBMs...
	// 0 causes original behavior
	// 1 allows a minority of 2 pbms to outscore a majority of 3 non-pbms
	// 4 would allow a minority of 1 pbm to outscore a majority of 4
	// non-pbms
	wl_pbm_pump = 4;	// FIXME: is 4 too much?

	if (wl_loops) {
		debug("N%d.LMC%d: Performing Hardware Write-Leveling\n", node,
		      if_num);
	} else {
		/* Force software write-leveling to run */
		wl_mask_err = 1;
		debug("N%d.LMC%d: Forcing software Write-Leveling\n", node,
		      if_num);
	}

	default_wl_rtt_nom = (ddr_type == DDR3_DRAM) ?
		rttnom_20ohm : ddr4_rttnom_40ohm;

	cfg.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
	ecc_ena = cfg.s.ecc_ena;
	save_mode32b = cfg.cn78xx.mode32b;
	cfg.cn78xx.mode32b = (!if_64b);
	lmc_wr(priv, CVMX_LMCX_CONFIG(if_num), cfg.u64);
	debug("%-45s : %d\n", "MODE32B", cfg.cn78xx.mode32b);

	s = lookup_env(priv, "ddr_wlevel_roundup");
	if (s)
		wl_roundup = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_wlevel_printall");
	if (s)
		wl_print = strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_wlevel_pbm_bump");
	if (s)
		wl_pbm_pump = strtoul(s, NULL, 0);

	// default to disable when RL sequential delay check is disabled
	disable_hwl_validity = disable_sequential_delay_check;
	s = lookup_env(priv, "ddr_disable_hwl_validity");
	if (s)
		disable_hwl_validity = !!strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_wl_rtt_nom");
	if (s)
		default_wl_rtt_nom = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_match_wl_rtt_nom");
	if (s)
		match_wl_rtt_nom = !!simple_strtoul(s, NULL, 0);

	if (match_wl_rtt_nom)
		mp1.u64 = lmc_rd(priv, CVMX_LMCX_MODEREG_PARAMS1(if_num));

	// For DDR3, we do not touch WLEVEL_CTL fields OR_DIS or BITMASK
	// For DDR4, we touch WLEVEL_CTL fields OR_DIS or BITMASK here
	if (ddr_type == DDR4_DRAM) {
		int default_or_dis = 1;
		int default_bitmask = 0xff;

		// when x4, use only the lower nibble
		if (dram_width == 4) {
			default_bitmask = 0x0f;
			if (wl_print) {
				debug("N%d.LMC%d: WLEVEL_CTL: default bitmask is 0x%02x for DDR4 x4\n",
				      node, if_num, default_bitmask);
			}
		}

		wl_ctl.u64 = lmc_rd(priv, CVMX_LMCX_WLEVEL_CTL(if_num));
		wl_ctl.s.or_dis = default_or_dis;
		wl_ctl.s.bitmask = default_bitmask;

		// allow overrides
		s = lookup_env(priv, "ddr_wlevel_ctl_or_dis");
		if (s)
			wl_ctl.s.or_dis = !!strtoul(s, NULL, 0);

		s = lookup_env(priv, "ddr_wlevel_ctl_bitmask");
		if (s)
			wl_ctl.s.bitmask = simple_strtoul(s, NULL, 0);

		// print only if not defaults
		if (wl_ctl.s.or_dis != default_or_dis ||
		    wl_ctl.s.bitmask != default_bitmask) {
			debug("N%d.LMC%d: WLEVEL_CTL: or_dis=%d, bitmask=0x%02x\n",
			      node, if_num, wl_ctl.s.or_dis, wl_ctl.s.bitmask);
		}

		// always write
		lmc_wr(priv, CVMX_LMCX_WLEVEL_CTL(if_num), wl_ctl.u64);
	}

	// Start the hardware write-leveling loop per rank
	for (rankx = 0; rankx < dimm_count * 4; rankx++)
		lmc_write_leveling_loop(priv, rankx);

	cfg.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
	cfg.cn78xx.mode32b = save_mode32b;
	lmc_wr(priv, CVMX_LMCX_CONFIG(if_num), cfg.u64);
	debug("%-45s : %d\n", "MODE32B", cfg.cn78xx.mode32b);

	// At the end of HW Write Leveling, check on some DESKEW things...
	if (!disable_deskew_training) {
		struct deskew_counts dsk_counts;
		int retry_count = 0;

		debug("N%d.LMC%d: Check Deskew Settings before Read-Leveling.\n",
		      node, if_num);

		do {
			validate_deskew_training(priv, rank_mask, if_num,
						 &dsk_counts, 1);

			// only RAWCARD A or B will not benefit from
			// retraining if there's only saturation
			// or any rawcard if there is a nibble error
			if ((!spd_rawcard_aorb && dsk_counts.saturated > 0) ||
			    (dsk_counts.nibrng_errs != 0 ||
			     dsk_counts.nibunl_errs != 0)) {
				retry_count++;
				debug("N%d.LMC%d: Deskew Status indicates saturation or nibble errors - retry %d Training.\n",
				      node, if_num, retry_count);
				perform_deskew_training(priv, rank_mask, if_num,
							spd_rawcard_aorb);
			} else {
				break;
			}
		} while (retry_count < 5);
	}
}

static void lmc_workaround(struct ddr_priv *priv)
{
	/* Workaround Trcd overflow by using Additive latency. */
	if (octeon_is_cpuid(OCTEON_CN78XX_PASS1_X)) {
		union cvmx_lmcx_modereg_params0 mp0;
		union cvmx_lmcx_timing_params1 tp1;
		union cvmx_lmcx_control ctrl;
		int rankx;

		tp1.u64 = lmc_rd(priv, CVMX_LMCX_TIMING_PARAMS1(if_num));
		mp0.u64 = lmc_rd(priv, CVMX_LMCX_MODEREG_PARAMS0(if_num));
		ctrl.u64 = lmc_rd(priv, CVMX_LMCX_CONTROL(if_num));

		if (tp1.cn78xx.trcd == 0) {
			debug("Workaround Trcd overflow by using Additive latency.\n");
			/* Hard code this to 12 and enable additive latency */
			tp1.cn78xx.trcd = 12;
			mp0.s.al = 2;	/* CL-2 */
			ctrl.s.pocas = 1;

			debug("MODEREG_PARAMS0                               : 0x%016llx\n",
			      mp0.u64);
			lmc_wr(priv, CVMX_LMCX_MODEREG_PARAMS0(if_num),
			       mp0.u64);
			debug("TIMING_PARAMS1                                : 0x%016llx\n",
			      tp1.u64);
			lmc_wr(priv, CVMX_LMCX_TIMING_PARAMS1(if_num), tp1.u64);

			debug("LMC_CONTROL                                   : 0x%016llx\n",
			      ctrl.u64);
			lmc_wr(priv, CVMX_LMCX_CONTROL(if_num), ctrl.u64);

			for (rankx = 0; rankx < dimm_count * 4; rankx++) {
				if (!(rank_mask & (1 << rankx)))
					continue;

				/* MR1 */
				ddr4_mrw(priv, if_num, rankx, -1, 1, 0);
			}
		}
	}

	// this is here just for output, to allow check of the Deskew
	// settings one last time...
	if (!disable_deskew_training) {
		struct deskew_counts dsk_counts;

		debug("N%d.LMC%d: Check Deskew Settings before software Write-Leveling.\n",
		      node, if_num);
		validate_deskew_training(priv, rank_mask, if_num, &dsk_counts,
					 3);
	}

	/*
	 * Workaround Errata 26304 (T88@2.0, O75@1.x, O78@2.x)
	 *
	 * When the CSRs LMCX_DLL_CTL3[WR_DESKEW_ENA] = 1 AND
	 * LMCX_PHY_CTL2[DQS[0..8]_DSK_ADJ] > 4, set
	 * LMCX_EXT_CONFIG[DRIVE_ENA_BPRCH] = 1.
	 */
	if (octeon_is_cpuid(OCTEON_CN78XX_PASS2_X) ||
	    octeon_is_cpuid(OCTEON_CNF75XX_PASS1_X)) {
		union cvmx_lmcx_dll_ctl3 dll_ctl3;
		union cvmx_lmcx_phy_ctl2 phy_ctl2;
		union cvmx_lmcx_ext_config ext_cfg;
		int increased_dsk_adj = 0;
		int byte;

		phy_ctl2.u64 = lmc_rd(priv, CVMX_LMCX_PHY_CTL2(if_num));
		ext_cfg.u64 = lmc_rd(priv, CVMX_LMCX_EXT_CONFIG(if_num));
		dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(if_num));

		for (byte = 0; byte < 8; ++byte) {
			if (!(if_bytemask & (1 << byte)))
				continue;
			increased_dsk_adj |=
			    (((phy_ctl2.u64 >> (byte * 3)) & 0x7) > 4);
		}

		if (dll_ctl3.s.wr_deskew_ena == 1 && increased_dsk_adj) {
			ext_cfg.s.drive_ena_bprch = 1;
			lmc_wr(priv, CVMX_LMCX_EXT_CONFIG(if_num), ext_cfg.u64);
			debug("LMC%d: Forcing DRIVE_ENA_BPRCH for Workaround Errata 26304.\n",
			      if_num);
		}
	}
}

// Software Write-Leveling block

#define VREF_RANGE1_LIMIT 0x33	// range1 is valid for 0x00 - 0x32
#define VREF_RANGE2_LIMIT 0x18	// range2 is valid for 0x00 - 0x17
// full window is valid for 0x00 to 0x4A
// let 0x00 - 0x17 be range2, 0x18 - 0x4a be range 1
#define VREF_LIMIT        (VREF_RANGE1_LIMIT + VREF_RANGE2_LIMIT)
#define VREF_FINAL        (VREF_LIMIT - 1)

enum sw_wl_status {
	WL_ESTIMATED = 0, /* HW/SW wleveling failed. Reslt estimated */
	WL_HARDWARE = 1,	/* H/W wleveling succeeded */
	WL_SOFTWARE = 2, /* S/W wleveling passed 2 contiguous setting */
	WL_SOFTWARE1 = 3, /* S/W wleveling passed 1 marginal setting */
};

static u64 rank_addr __section(".data");
static int vref_val __section(".data");
static int final_vref_val __section(".data");
static int final_vref_range __section(".data");
static int start_vref_val __section(".data");
static int computed_final_vref_val __section(".data");
static char best_vref_val_count __section(".data");
static char vref_val_count __section(".data");
static char best_vref_val_start __section(".data");
static char vref_val_start __section(".data");
static int bytes_failed __section(".data");
static enum sw_wl_status byte_test_status[9] __section(".data");
static enum sw_wl_status sw_wl_rank_status __section(".data");
static int sw_wl_failed __section(".data");
static int sw_wl_hw __section(".data");
static int measured_vref_flag __section(".data");

static void ddr4_vref_loop(struct ddr_priv *priv, int rankx)
{
	char *s;

	if (vref_val < VREF_FINAL) {
		int vrange, vvalue;

		if (vref_val < VREF_RANGE2_LIMIT) {
			vrange = 1;
			vvalue = vref_val;
		} else {
			vrange = 0;
			vvalue = vref_val - VREF_RANGE2_LIMIT;
		}

		set_vref(priv, if_num, rankx, vrange, vvalue);
	} else {		/* if (vref_val < VREF_FINAL) */
		/* Print the final vref value first. */

		/* Always print the computed first if its valid */
		if (computed_final_vref_val >= 0) {
			debug("N%d.LMC%d.R%d: vref Computed Summary                 :              %2d (0x%02x)\n",
			      node, if_num, rankx,
			      computed_final_vref_val, computed_final_vref_val);
		}

		if (!measured_vref_flag) {	// setup to use the computed
			best_vref_val_count = 1;
			final_vref_val = computed_final_vref_val;
		} else {	// setup to use the measured
			if (best_vref_val_count > 0) {
				best_vref_val_count =
				    max(best_vref_val_count, (char)2);
				final_vref_val = best_vref_val_start +
					divide_nint(best_vref_val_count - 1, 2);

				if (final_vref_val < VREF_RANGE2_LIMIT) {
					final_vref_range = 1;
				} else {
					final_vref_range = 0;
					final_vref_val -= VREF_RANGE2_LIMIT;
				}

				int vvlo = best_vref_val_start;
				int vrlo;
				int vvhi = best_vref_val_start +
					best_vref_val_count - 1;
				int vrhi;

				if (vvlo < VREF_RANGE2_LIMIT) {
					vrlo = 2;
				} else {
					vrlo = 1;
					vvlo -= VREF_RANGE2_LIMIT;
				}

				if (vvhi < VREF_RANGE2_LIMIT) {
					vrhi = 2;
				} else {
					vrhi = 1;
					vvhi -= VREF_RANGE2_LIMIT;
				}
				debug("N%d.LMC%d.R%d: vref Training Summary                 :  0x%02x/%1d <----- 0x%02x/%1d -----> 0x%02x/%1d, range: %2d\n",
				      node, if_num, rankx, vvlo, vrlo,
				      final_vref_val,
				      final_vref_range + 1, vvhi, vrhi,
				      best_vref_val_count - 1);

			} else {
				/*
				 * If nothing passed use the default vref
				 * value for this rank
				 */
				union cvmx_lmcx_modereg_params2 mp2;

				mp2.u64 =
					lmc_rd(priv,
					       CVMX_LMCX_MODEREG_PARAMS2(if_num));
				final_vref_val = (mp2.u64 >>
						  (rankx * 10 + 3)) & 0x3f;
				final_vref_range = (mp2.u64 >>
						    (rankx * 10 + 9)) & 0x01;

				debug("N%d.LMC%d.R%d: vref Using Default                    :    %2d <----- %2d (0x%02x) -----> %2d, range%1d\n",
				      node, if_num, rankx, final_vref_val,
				      final_vref_val, final_vref_val,
				      final_vref_val, final_vref_range + 1);
			}
		}

		// allow override
		s = lookup_env(priv, "ddr%d_vref_val_%1d%1d",
			       if_num, !!(rankx & 2), !!(rankx & 1));
		if (s)
			final_vref_val = strtoul(s, NULL, 0);

		set_vref(priv, if_num, rankx, final_vref_range, final_vref_val);
	}
}

#define WL_MIN_NO_ERRORS_COUNT 3	// FIXME? three passes without errors

static int errors __section(".data");
static int byte_delay[9] __section(".data");
static u64 bytemask __section(".data");
static int bytes_todo __section(".data");
static int no_errors_count __section(".data");
static u64 bad_bits[2] __section(".data");
static u64 sum_dram_dclk __section(".data");
static u64 sum_dram_ops __section(".data");
static u64 start_dram_dclk __section(".data");
static u64 stop_dram_dclk __section(".data");
static u64 start_dram_ops __section(".data");
static u64 stop_dram_ops __section(".data");

static void lmc_sw_write_leveling_loop(struct ddr_priv *priv, int rankx)
{
	int delay;
	int b;

	// write the current set of WL delays
	lmc_wr(priv, CVMX_LMCX_WLEVEL_RANKX(rankx, if_num), wl_rank.u64);
	wl_rank.u64 = lmc_rd(priv, CVMX_LMCX_WLEVEL_RANKX(rankx, if_num));

	// do the test
	if (sw_wl_hw) {
		errors = run_best_hw_patterns(priv, if_num, rank_addr,
					      DBTRAIN_TEST, bad_bits);
		errors &= bytes_todo;	// keep only the ones we are still doing
	} else {
		start_dram_dclk = lmc_rd(priv, CVMX_LMCX_DCLK_CNT(if_num));
		start_dram_ops = lmc_rd(priv, CVMX_LMCX_OPS_CNT(if_num));
		errors = test_dram_byte64(priv, if_num, rank_addr, bytemask,
					  bad_bits);

		stop_dram_dclk = lmc_rd(priv, CVMX_LMCX_DCLK_CNT(if_num));
		stop_dram_ops = lmc_rd(priv, CVMX_LMCX_OPS_CNT(if_num));
		sum_dram_dclk += stop_dram_dclk - start_dram_dclk;
		sum_dram_ops += stop_dram_ops - start_dram_ops;
	}

	debug("WL pass1: test_dram_byte returned 0x%x\n", errors);

	// remember, errors will not be returned for byte-lanes that have
	// maxxed out...
	if (errors == 0) {
		no_errors_count++;	// bump
		// bypass check/update completely
		if (no_errors_count > 1)
			return;	// to end of do-while
	} else {
		no_errors_count = 0;	// reset
	}

	// check errors by byte
	for (b = 0; b < 9; ++b) {
		if (!(bytes_todo & (1 << b)))
			continue;

		delay = byte_delay[b];
		// yes, an error in this byte lane
		if (errors & (1 << b)) {
			debug("        byte %d delay %2d Errors\n", b, delay);
			// since this byte had an error, we move to the next
			// delay value, unless done with it
			delay += 8;	// incr by 8 to do delay high-order bits
			if (delay < 32) {
				upd_wl_rank(&wl_rank, b, delay);
				debug("        byte %d delay %2d New\n",
				      b, delay);
				byte_delay[b] = delay;
			} else {
				// reached max delay, maybe really done with
				// this byte
				// consider an alt only for computed VREF and
				if (!measured_vref_flag &&
				    (hwl_alts[rankx].hwl_alt_mask & (1 << b))) {
					// if an alt exists...
					// just orig low-3 bits
					int bad_delay = delay & 0x6;

					// yes, use it
					delay =	hwl_alts[rankx].hwl_alt_delay[b];
					// clear that flag
					hwl_alts[rankx].hwl_alt_mask &=
						~(1 << b);
					upd_wl_rank(&wl_rank, b, delay);
					byte_delay[b] = delay;
					debug("        byte %d delay %2d ALTERNATE\n",
					      b, delay);
					debug("N%d.LMC%d.R%d: SWL: Byte %d: %d FAIL, trying ALTERNATE %d\n",
					      node, if_num,
					      rankx, b, bad_delay, delay);

				} else {
					unsigned int bits_bad;

					if (b < 8) {
						// test no longer, remove from
						// byte mask
						bytemask &=
							~(0xffULL << (8 * b));
						bits_bad = (unsigned int)
							((bad_bits[0] >>
							  (8 * b)) & 0xffUL);
					} else {
						bits_bad = (unsigned int)
						    (bad_bits[1] & 0xffUL);
					}

					// remove from bytes to do
					bytes_todo &= ~(1 << b);
					// make sure this is set for this case
					byte_test_status[b] = WL_ESTIMATED;
					debug("        byte %d delay %2d Exhausted\n",
					      b, delay);
					if (!measured_vref_flag) {
						// this is too noisy when doing
						// measured VREF
						debug("N%d.LMC%d.R%d: SWL: Byte %d (0x%02x): delay %d EXHAUSTED\n",
						      node, if_num, rankx,
						      b, bits_bad, delay);
					}
				}
			}
		} else {
			// no error, stay with current delay, but keep testing
			// it...
			debug("        byte %d delay %2d Passed\n", b, delay);
			byte_test_status[b] = WL_HARDWARE;	// change status
		}
	}			/* for (b = 0; b < 9; ++b) */
}

static void sw_write_lvl_use_ecc(struct ddr_priv *priv, int rankx)
{
	int save_byte8 = wl_rank.s.byte8;

	byte_test_status[8] = WL_HARDWARE;	/* H/W delay value */

	if (save_byte8 != wl_rank.s.byte3 &&
	    save_byte8 != wl_rank.s.byte4) {
		int test_byte8 = save_byte8;
		int test_byte8_error;
		int byte8_error = 0x1f;
		int adder;
		int avg_bytes = divide_nint(wl_rank.s.byte3 + wl_rank.s.byte4,
					    2);

		for (adder = 0; adder <= 32; adder += 8) {
			test_byte8_error = abs((adder + save_byte8) -
					       avg_bytes);
			if (test_byte8_error < byte8_error) {
				byte8_error = test_byte8_error;
				test_byte8 = save_byte8 + adder;
			}
		}

		// only do the check if we are not using measured VREF
		if (!measured_vref_flag) {
			/* Use only even settings, rounding down... */
			test_byte8 &= ~1;

			// do validity check on the calculated ECC delay value
			// this depends on the DIMM type
			if (spd_rdimm) {	// RDIMM
				// but not mini-RDIMM
				if (spd_dimm_type != 5) {
					// it can be > byte4, but should never
					// be > byte3
					if (test_byte8 > wl_rank.s.byte3) {
						/* say it is still estimated */
						byte_test_status[8] =
							WL_ESTIMATED;
					}
				}
			} else {	// UDIMM
				if (test_byte8 < wl_rank.s.byte3 ||
				    test_byte8 > wl_rank.s.byte4) {
					// should never be outside the
					// byte 3-4 range
					/* say it is still estimated */
					byte_test_status[8] = WL_ESTIMATED;
				}
			}
			/*
			 * Report whenever the calculation appears bad.
			 * This happens if some of the original values were off,
			 * or unexpected geometry from DIMM type, or custom
			 * circuitry (NIC225E, I am looking at you!).
			 * We will trust the calculated value, and depend on
			 * later testing to catch any instances when that
			 * value is truly bad.
			 */
			// ESTIMATED means there may be an issue
			if (byte_test_status[8] == WL_ESTIMATED) {
				debug("N%d.LMC%d.R%d: SWL: (%cDIMM): calculated ECC delay unexpected (%d/%d/%d)\n",
				      node, if_num, rankx,
				      (spd_rdimm ? 'R' : 'U'), wl_rank.s.byte4,
				      test_byte8, wl_rank.s.byte3);
				byte_test_status[8] = WL_HARDWARE;
			}
		}
		/* Use only even settings */
		wl_rank.s.byte8 = test_byte8 & ~1;
	}

	if (wl_rank.s.byte8 != save_byte8) {
		/* Change the status if s/w adjusted the delay */
		byte_test_status[8] = WL_SOFTWARE;	/* Estimated delay */
	}
}

static __maybe_unused void parallel_wl_block_delay(struct ddr_priv *priv,
						   int rankx)
{
	int errors;
	int byte_delay[8];
	int byte_passed[8];
	u64 bytemask;
	u64 bitmask;
	int wl_offset;
	int bytes_todo;
	int sw_wl_offset = 1;
	int delay;
	int b;

	for (b = 0; b < 8; ++b)
		byte_passed[b] = 0;

	bytes_todo = if_bytemask;

	for (wl_offset = sw_wl_offset; wl_offset >= 0; --wl_offset) {
		debug("Starting wl_offset for-loop: %d\n", wl_offset);

		bytemask = 0;

		for (b = 0; b < 8; ++b) {
			byte_delay[b] = 0;
			// this does not contain fully passed bytes
			if (!(bytes_todo & (1 << b)))
				continue;

			// reset across passes if not fully passed
			byte_passed[b] = 0;
			upd_wl_rank(&wl_rank, b, 0);	// all delays start at 0
			bitmask = ((!if_64b) && (b == 4)) ? 0x0f : 0xff;
			// set the bytes bits in the bytemask
			bytemask |= bitmask << (8 * b);
		}		/* for (b = 0; b < 8; ++b) */

		// start a pass if there is any byte lane to test
		while (bytemask != 0) {
			debug("Starting bytemask while-loop: 0x%llx\n",
			      bytemask);

			// write this set of WL delays
			lmc_wr(priv, CVMX_LMCX_WLEVEL_RANKX(rankx, if_num),
			       wl_rank.u64);
			wl_rank.u64 = lmc_rd(priv,
					     CVMX_LMCX_WLEVEL_RANKX(rankx,
								    if_num));

			// do the test
			if (sw_wl_hw) {
				errors = run_best_hw_patterns(priv, if_num,
							      rank_addr,
							      DBTRAIN_TEST,
							      NULL) & 0xff;
			} else {
				errors = test_dram_byte64(priv, if_num,
							  rank_addr, bytemask,
							  NULL);
			}

			debug("test_dram_byte returned 0x%x\n", errors);

			// check errors by byte
			for (b = 0; b < 8; ++b) {
				if (!(bytes_todo & (1 << b)))
					continue;

				delay = byte_delay[b];
				if (errors & (1 << b)) {	// yes, an error
					debug("        byte %d delay %2d Errors\n",
					      b, delay);
					byte_passed[b] = 0;
				} else {	// no error
					byte_passed[b] += 1;
					// Look for consecutive working settings
					if (byte_passed[b] == (1 + wl_offset)) {
						debug("        byte %d delay %2d FULLY Passed\n",
						      b, delay);
						if (wl_offset == 1) {
							byte_test_status[b] =
								WL_SOFTWARE;
						} else if (wl_offset == 0) {
							byte_test_status[b] =
								WL_SOFTWARE1;
						}

						// test no longer, remove
						// from byte mask this pass
						bytemask &= ~(0xffULL <<
							      (8 * b));
						// remove completely from
						// concern
						bytes_todo &= ~(1 << b);
						// on to the next byte, bypass
						// delay updating!!
						continue;
					} else {
						debug("        byte %d delay %2d Passed\n",
						      b, delay);
					}
				}

				// error or no, here we move to the next delay
				// value for this byte, unless done all delays
				// only a byte that has "fully passed" will
				// bypass around this,
				delay += 2;
				if (delay < 32) {
					upd_wl_rank(&wl_rank, b, delay);
					debug("        byte %d delay %2d New\n",
					      b, delay);
					byte_delay[b] = delay;
				} else {
					// reached max delay, done with this
					// byte
					debug("        byte %d delay %2d Exhausted\n",
					      b, delay);
					// test no longer, remove from byte
					// mask this pass
					bytemask &= ~(0xffULL << (8 * b));
				}
			}	/* for (b = 0; b < 8; ++b) */
			debug("End of for-loop: bytemask 0x%llx\n", bytemask);
		}		/* while (bytemask != 0) */
	}

	for (b = 0; b < 8; ++b) {
		// any bytes left in bytes_todo did not pass
		if (bytes_todo & (1 << b)) {
			union cvmx_lmcx_rlevel_rankx lmc_rlevel_rank;

			/*
			 * Last resort. Use Rlevel settings to estimate
			 * Wlevel if software write-leveling fails
			 */
			debug("Using RLEVEL as WLEVEL estimate for byte %d\n",
			      b);
			lmc_rlevel_rank.u64 =
				lmc_rd(priv, CVMX_LMCX_RLEVEL_RANKX(rankx,
								    if_num));
			rlevel_to_wlevel(&lmc_rlevel_rank, &wl_rank, b);
		}
	}			/* for (b = 0; b < 8; ++b) */
}

static int lmc_sw_write_leveling(struct ddr_priv *priv)
{
	/* Try to determine/optimize write-level delays experimentally. */
	union cvmx_lmcx_wlevel_rankx wl_rank_hw_res;
	union cvmx_lmcx_config cfg;
	int rankx;
	int byte;
	char *s;
	int i;

	int active_rank;
	int sw_wl_enable = 1;	/* FIX... Should be customizable. */
	int interfaces;

	static const char * const wl_status_strings[] = {
		"(e)",
		"   ",
		"   ",
		"(1)"
	};

	// FIXME: make HW-assist the default now?
	int sw_wl_hw_default = SW_WLEVEL_HW_DEFAULT;
	int dram_connection = c_cfg->dram_connection;

	s = lookup_env(priv, "ddr_sw_wlevel_hw");
	if (s)
		sw_wl_hw_default = !!strtoul(s, NULL, 0);
	if (!if_64b)		// must use SW algo if 32-bit mode
		sw_wl_hw_default = 0;

	// can never use hw-assist
	if (octeon_is_cpuid(OCTEON_CN78XX_PASS1_X))
		sw_wl_hw_default = 0;

	s = lookup_env(priv, "ddr_software_wlevel");
	if (s)
		sw_wl_enable = strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr%d_dram_connection", if_num);
	if (s)
		dram_connection = !!strtoul(s, NULL, 0);

	cvmx_rng_enable();

	/*
	 * Get the measured_vref setting from the config, check for an
	 * override...
	 */
	/* NOTE: measured_vref=1 (ON) means force use of MEASURED vref... */
	// NOTE: measured VREF can only be done for DDR4
	if (ddr_type == DDR4_DRAM) {
		measured_vref_flag = c_cfg->measured_vref;
		s = lookup_env(priv, "ddr_measured_vref");
		if (s)
			measured_vref_flag = !!strtoul(s, NULL, 0);
	} else {
		measured_vref_flag = 0;	// OFF for DDR3
	}

	/*
	 * Ensure disabled ECC for DRAM tests using the SW algo, else leave
	 * it untouched
	 */
	if (!sw_wl_hw_default) {
		cfg.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
		cfg.cn78xx.ecc_ena = 0;
		lmc_wr(priv, CVMX_LMCX_CONFIG(if_num), cfg.u64);
	}

	/*
	 * We need to track absolute rank number, as well as how many
	 * active ranks we have.  Two single rank DIMMs show up as
	 * ranks 0 and 2, but only 2 ranks are active.
	 */
	active_rank = 0;

	interfaces = __builtin_popcount(if_mask);

	for (rankx = 0; rankx < dimm_count * 4; rankx++) {
		final_vref_range = 0;
		start_vref_val = 0;
		computed_final_vref_val = -1;
		sw_wl_rank_status = WL_HARDWARE;
		sw_wl_failed = 0;
		sw_wl_hw = sw_wl_hw_default;

		if (!sw_wl_enable)
			break;

		if (!(rank_mask & (1 << rankx)))
			continue;

		debug("N%d.LMC%d.R%d: Performing Software Write-Leveling %s\n",
		      node, if_num, rankx,
		      (sw_wl_hw) ? "with H/W assist" :
		      "with S/W algorithm");

		if (ddr_type == DDR4_DRAM && num_ranks != 4) {
			// always compute when we can...
			computed_final_vref_val =
			    compute_vref_val(priv, if_num, rankx, dimm_count,
					     num_ranks, imp_val,
					     is_stacked_die, dram_connection);

			// but only use it if allowed
			if (!measured_vref_flag) {
				// skip all the measured vref processing,
				// just the final setting
				start_vref_val = VREF_FINAL;
			}
		}

		/* Save off the h/w wl results */
		wl_rank_hw_res.u64 = lmc_rd(priv,
					    CVMX_LMCX_WLEVEL_RANKX(rankx,
								   if_num));

		vref_val_count = 0;
		vref_val_start = 0;
		best_vref_val_count = 0;
		best_vref_val_start = 0;

		/* Loop one extra time using the Final vref value. */
		for (vref_val = start_vref_val; vref_val < VREF_LIMIT;
		     ++vref_val) {
			if (ddr_type == DDR4_DRAM)
				ddr4_vref_loop(priv, rankx);

			/* Restore the saved value */
			wl_rank.u64 = wl_rank_hw_res.u64;

			for (byte = 0; byte < 9; ++byte)
				byte_test_status[byte] = WL_ESTIMATED;

			if (wl_mask_err == 0) {
				/*
				 * Determine address of DRAM to test for
				 * pass 1 of software write leveling.
				 */
				rank_addr = active_rank *
					(1ull << (pbank_lsb - bunk_enable +
						  (interfaces / 2)));

				/*
				 * Adjust address for boot bus hole in memory
				 * map.
				 */
				if (rank_addr > 0x10000000)
					rank_addr += 0x10000000;

				debug("N%d.LMC%d.R%d: Active Rank %d Address: 0x%llx\n",
				      node, if_num, rankx, active_rank,
				      rank_addr);

				// start parallel write-leveling block for
				// delay high-order bits
				errors = 0;
				no_errors_count = 0;
				sum_dram_dclk = 0;
				sum_dram_ops = 0;

				if (if_64b) {
					bytes_todo = (sw_wl_hw) ?
						if_bytemask : 0xFF;
					bytemask = ~0ULL;
				} else {
					// 32-bit, must be using SW algo,
					// only data bytes
					bytes_todo = 0x0f;
					bytemask = 0x00000000ffffffffULL;
				}

				for (byte = 0; byte < 9; ++byte) {
					if (!(bytes_todo & (1 << byte))) {
						byte_delay[byte] = 0;
					} else {
						byte_delay[byte] =
						    get_wl_rank(&wl_rank, byte);
					}
				}	/* for (byte = 0; byte < 9; ++byte) */

				do {
					lmc_sw_write_leveling_loop(priv, rankx);
				} while (no_errors_count <
					 WL_MIN_NO_ERRORS_COUNT);

				if (!sw_wl_hw) {
					u64 percent_x10;

					if (sum_dram_dclk == 0)
						sum_dram_dclk = 1;
					percent_x10 = sum_dram_ops * 1000 /
						sum_dram_dclk;
					debug("N%d.LMC%d.R%d: ops %llu, cycles %llu, used %llu.%llu%%\n",
					      node, if_num, rankx, sum_dram_ops,
					      sum_dram_dclk, percent_x10 / 10,
					      percent_x10 % 10);
				}
				if (errors) {
					debug("End WLEV_64 while loop: vref_val %d(0x%x), errors 0x%02x\n",
					      vref_val, vref_val, errors);
				}
				// end parallel write-leveling block for
				// delay high-order bits

				// if we used HW-assist, we did the ECC byte
				// when approp.
				if (sw_wl_hw) {
					if (wl_print) {
						debug("N%d.LMC%d.R%d: HW-assisted SWL - ECC estimate not needed.\n",
						      node, if_num, rankx);
					}
					goto no_ecc_estimate;
				}

				if ((if_bytemask & 0xff) == 0xff) {
					if (use_ecc) {
						sw_write_lvl_use_ecc(priv,
								     rankx);
					} else {
						/* H/W delay value */
						byte_test_status[8] =
							WL_HARDWARE;
						/* ECC is not used */
						wl_rank.s.byte8 =
							wl_rank.s.byte0;
					}
				} else {
					if (use_ecc) {
						/* Estimate the ECC byte dly */
						// add hi-order to b4
						wl_rank.s.byte4 |=
							(wl_rank.s.byte3 &
							 0x38);
						if ((wl_rank.s.byte4 & 0x06) <
						    (wl_rank.s.byte3 & 0x06)) {
							// must be next clock
							wl_rank.s.byte4 += 8;
						}
					} else {
						/* ECC is not used */
						wl_rank.s.byte4 =
							wl_rank.s.byte0;
					}

					/*
					 * Change the status if s/w adjusted
					 * the delay
					 */
					/* Estimated delay */
					byte_test_status[4] = WL_SOFTWARE;
				}	/* if ((if_bytemask & 0xff) == 0xff) */
			}	/* if (wl_mask_err == 0) */

no_ecc_estimate:

			bytes_failed = 0;
			for (byte = 0; byte < 9; ++byte) {
				/* Don't accumulate errors for untested bytes */
				if (!(if_bytemask & (1 << byte)))
					continue;
				bytes_failed +=
				    (byte_test_status[byte] == WL_ESTIMATED);
			}

			/* vref training loop is only used for DDR4  */
			if (ddr_type != DDR4_DRAM)
				break;

			if (bytes_failed == 0) {
				if (vref_val_count == 0)
					vref_val_start = vref_val;

				++vref_val_count;
				if (vref_val_count > best_vref_val_count) {
					best_vref_val_count = vref_val_count;
					best_vref_val_start = vref_val_start;
					debug("N%d.LMC%d.R%d: vref Training                    (%2d) :    0x%02x <----- ???? -----> 0x%02x\n",
					      node, if_num, rankx, vref_val,
					      best_vref_val_start,
					      best_vref_val_start +
					      best_vref_val_count - 1);
				}
			} else {
				vref_val_count = 0;
				debug("N%d.LMC%d.R%d: vref Training                    (%2d) :    failed\n",
				      node, if_num, rankx, vref_val);
			}
		}

		/*
		 * Determine address of DRAM to test for software write
		 * leveling.
		 */
		rank_addr = active_rank * (1ull << (pbank_lsb - bunk_enable +
						    (interfaces / 2)));
		/* Adjust address for boot bus hole in memory map. */
		if (rank_addr > 0x10000000)
			rank_addr += 0x10000000;

		debug("Rank Address: 0x%llx\n", rank_addr);

		if (bytes_failed) {
			// FIXME? the big hammer, did not even try SW WL pass2,
			// assume only chip reset will help
			debug("N%d.LMC%d.R%d: S/W write-leveling pass 1 failed\n",
			      node, if_num, rankx);
			sw_wl_failed = 1;
		} else {	/* if (bytes_failed) */
			// SW WL pass 1 was OK, write the settings
			lmc_wr(priv, CVMX_LMCX_WLEVEL_RANKX(rankx, if_num),
			       wl_rank.u64);
			wl_rank.u64 = lmc_rd(priv,
					     CVMX_LMCX_WLEVEL_RANKX(rankx,
								    if_num));

			// do validity check on the delay values by running
			// the test 1 more time...
			// FIXME: we really need to check the ECC byte setting
			// here as well, so we need to enable ECC for this test!
			// if there are any errors, claim SW WL failure
			u64 datamask = (if_64b) ? 0xffffffffffffffffULL :
				0x00000000ffffffffULL;
			int errors;

			// do the test
			if (sw_wl_hw) {
				errors = run_best_hw_patterns(priv, if_num,
							      rank_addr,
							      DBTRAIN_TEST,
							      NULL) & 0xff;
			} else {
				errors = test_dram_byte64(priv, if_num,
							  rank_addr, datamask,
							  NULL);
			}

			if (errors) {
				debug("N%d.LMC%d.R%d: Wlevel Rank Final Test errors 0x%03x\n",
				      node, if_num, rankx, errors);
				sw_wl_failed = 1;
			}
		}		/* if (bytes_failed) */

		// FIXME? dump the WL settings, so we get more of a clue
		// as to what happened where
		debug("N%d.LMC%d.R%d: Wlevel Rank %#4x, 0x%016llX  : %2d%3s %2d%3s %2d%3s %2d%3s %2d%3s %2d%3s %2d%3s %2d%3s %2d%3s %s\n",
		      node, if_num, rankx, wl_rank.s.status, wl_rank.u64,
		      wl_rank.s.byte8, wl_status_strings[byte_test_status[8]],
		      wl_rank.s.byte7, wl_status_strings[byte_test_status[7]],
		      wl_rank.s.byte6, wl_status_strings[byte_test_status[6]],
		      wl_rank.s.byte5, wl_status_strings[byte_test_status[5]],
		      wl_rank.s.byte4, wl_status_strings[byte_test_status[4]],
		      wl_rank.s.byte3, wl_status_strings[byte_test_status[3]],
		      wl_rank.s.byte2, wl_status_strings[byte_test_status[2]],
		      wl_rank.s.byte1, wl_status_strings[byte_test_status[1]],
		      wl_rank.s.byte0, wl_status_strings[byte_test_status[0]],
		      (sw_wl_rank_status == WL_HARDWARE) ? "" : "(s)");

		// finally, check for fatal conditions: either chip reset
		// right here, or return error flag
		if ((ddr_type == DDR4_DRAM && best_vref_val_count == 0) ||
		    sw_wl_failed) {
			if (!ddr_disable_chip_reset) {	// do chip RESET
				printf("N%d.LMC%d.R%d: INFO: Short memory test indicates a retry is needed. Resetting node...\n",
				       node, if_num, rankx);
				mdelay(500);
				do_reset(NULL, 0, 0, NULL);
			} else {
				// return error flag so LMC init can be retried.
				debug("N%d.LMC%d.R%d: INFO: Short memory test indicates a retry is needed. Restarting LMC init...\n",
				      node, if_num, rankx);
				return -EAGAIN;	// 0 indicates restart possible.
			}
		}
		active_rank++;
	}

	for (rankx = 0; rankx < dimm_count * 4; rankx++) {
		int parameter_set = 0;
		u64 value;

		if (!(rank_mask & (1 << rankx)))
			continue;

		wl_rank.u64 = lmc_rd(priv, CVMX_LMCX_WLEVEL_RANKX(rankx,
								  if_num));

		for (i = 0; i < 9; ++i) {
			s = lookup_env(priv, "ddr%d_wlevel_rank%d_byte%d",
				       if_num, rankx, i);
			if (s) {
				parameter_set |= 1;
				value = strtoul(s, NULL, 0);

				upd_wl_rank(&wl_rank, i, value);
			}
		}

		s = lookup_env_ull(priv, "ddr%d_wlevel_rank%d", if_num, rankx);
		if (s) {
			parameter_set |= 1;
			value = strtoull(s, NULL, 0);
			wl_rank.u64 = value;
		}

		if (parameter_set) {
			lmc_wr(priv, CVMX_LMCX_WLEVEL_RANKX(rankx, if_num),
			       wl_rank.u64);
			wl_rank.u64 =
			    lmc_rd(priv, CVMX_LMCX_WLEVEL_RANKX(rankx, if_num));
			display_wl(if_num, wl_rank, rankx);
		}
		// if there are unused entries to be filled
		if ((rank_mask & 0x0F) != 0x0F) {
			if (rankx < 3) {
				debug("N%d.LMC%d.R%d: checking for WLEVEL_RANK unused entries.\n",
				      node, if_num, rankx);

				// if rank 0, write ranks 1 and 2 here if empty
				if (rankx == 0) {
					// check that rank 1 is empty
					if (!(rank_mask & (1 << 1))) {
						debug("N%d.LMC%d.R%d: writing WLEVEL_RANK unused entry R%d.\n",
						      node, if_num, rankx, 1);
						lmc_wr(priv,
						       CVMX_LMCX_WLEVEL_RANKX(1,
								if_num),
						       wl_rank.u64);
					}

					// check that rank 2 is empty
					if (!(rank_mask & (1 << 2))) {
						debug("N%d.LMC%d.R%d: writing WLEVEL_RANK unused entry R%d.\n",
						      node, if_num, rankx, 2);
						lmc_wr(priv,
						       CVMX_LMCX_WLEVEL_RANKX(2,
								if_num),
						       wl_rank.u64);
					}
				}

				// if rank 0, 1 or 2, write rank 3 here if empty
				// check that rank 3 is empty
				if (!(rank_mask & (1 << 3))) {
					debug("N%d.LMC%d.R%d: writing WLEVEL_RANK unused entry R%d.\n",
					      node, if_num, rankx, 3);
					lmc_wr(priv,
					       CVMX_LMCX_WLEVEL_RANKX(3,
								      if_num),
					       wl_rank.u64);
				}
			}
		}
	}

	/* Enable 32-bit mode if required. */
	cfg.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
	cfg.cn78xx.mode32b = (!if_64b);
	debug("%-45s : %d\n", "MODE32B", cfg.cn78xx.mode32b);

	/* Restore the ECC configuration */
	if (!sw_wl_hw_default)
		cfg.cn78xx.ecc_ena = use_ecc;

	lmc_wr(priv, CVMX_LMCX_CONFIG(if_num), cfg.u64);

	return 0;
}

static void lmc_dll(struct ddr_priv *priv)
{
	union cvmx_lmcx_dll_ctl3 ddr_dll_ctl3;
	int setting[9];
	int i;

	ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(if_num));

	for (i = 0; i < 9; ++i) {
		SET_DDR_DLL_CTL3(dll90_byte_sel, ENCODE_DLL90_BYTE_SEL(i));
		lmc_wr(priv, CVMX_LMCX_DLL_CTL3(if_num), ddr_dll_ctl3.u64);
		lmc_rd(priv, CVMX_LMCX_DLL_CTL3(if_num));
		ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(if_num));
		setting[i] = GET_DDR_DLL_CTL3(dll90_setting);
		debug("%d. LMC%d_DLL_CTL3[%d] = %016llx %d\n", i, if_num,
		      GET_DDR_DLL_CTL3(dll90_byte_sel), ddr_dll_ctl3.u64,
		      setting[i]);
	}

	debug("N%d.LMC%d: %-36s : %5d %5d %5d %5d %5d %5d %5d %5d %5d\n",
	      node, if_num, "DLL90 Setting 8:0",
	      setting[8], setting[7], setting[6], setting[5], setting[4],
	      setting[3], setting[2], setting[1], setting[0]);

	process_custom_dll_offsets(priv, if_num, "ddr_dll_write_offset",
				   c_cfg->dll_write_offset,
				   "ddr%d_dll_write_offset_byte%d", 1);
	process_custom_dll_offsets(priv, if_num, "ddr_dll_read_offset",
				   c_cfg->dll_read_offset,
				   "ddr%d_dll_read_offset_byte%d", 2);
}

#define SLOT_CTL_INCR(csr, chip, field, incr)				\
	csr.chip.field = (csr.chip.field < (64 - incr)) ?		\
		(csr.chip.field + incr) : 63

#define INCR(csr, chip, field, incr)                                    \
	csr.chip.field = (csr.chip.field < (64 - incr)) ?		\
		(csr.chip.field + incr) : 63

static void lmc_workaround_2(struct ddr_priv *priv)
{
	/* Workaround Errata 21063 */
	if (octeon_is_cpuid(OCTEON_CN78XX) ||
	    octeon_is_cpuid(OCTEON_CN70XX_PASS1_X)) {
		union cvmx_lmcx_slot_ctl0 slot_ctl0;
		union cvmx_lmcx_slot_ctl1 slot_ctl1;
		union cvmx_lmcx_slot_ctl2 slot_ctl2;
		union cvmx_lmcx_ext_config ext_cfg;

		slot_ctl0.u64 = lmc_rd(priv, CVMX_LMCX_SLOT_CTL0(if_num));
		slot_ctl1.u64 = lmc_rd(priv, CVMX_LMCX_SLOT_CTL1(if_num));
		slot_ctl2.u64 = lmc_rd(priv, CVMX_LMCX_SLOT_CTL2(if_num));

		ext_cfg.u64 = lmc_rd(priv, CVMX_LMCX_EXT_CONFIG(if_num));

		/* When ext_cfg.s.read_ena_bprch is set add 1 */
		if (ext_cfg.s.read_ena_bprch) {
			SLOT_CTL_INCR(slot_ctl0, cn78xx, r2w_init, 1);
			SLOT_CTL_INCR(slot_ctl0, cn78xx, r2w_l_init, 1);
			SLOT_CTL_INCR(slot_ctl1, cn78xx, r2w_xrank_init, 1);
			SLOT_CTL_INCR(slot_ctl2, cn78xx, r2w_xdimm_init, 1);
		}

		/* Always add 2 */
		SLOT_CTL_INCR(slot_ctl1, cn78xx, w2r_xrank_init, 2);
		SLOT_CTL_INCR(slot_ctl2, cn78xx, w2r_xdimm_init, 2);

		lmc_wr(priv, CVMX_LMCX_SLOT_CTL0(if_num), slot_ctl0.u64);
		lmc_wr(priv, CVMX_LMCX_SLOT_CTL1(if_num), slot_ctl1.u64);
		lmc_wr(priv, CVMX_LMCX_SLOT_CTL2(if_num), slot_ctl2.u64);
	}

	/* Workaround Errata 21216 */
	if (octeon_is_cpuid(OCTEON_CN78XX_PASS1_X) ||
	    octeon_is_cpuid(OCTEON_CN70XX_PASS1_X)) {
		union cvmx_lmcx_slot_ctl1 slot_ctl1;
		union cvmx_lmcx_slot_ctl2 slot_ctl2;

		slot_ctl1.u64 = lmc_rd(priv, CVMX_LMCX_SLOT_CTL1(if_num));
		slot_ctl1.cn78xx.w2w_xrank_init =
		    max(10, (int)slot_ctl1.cn78xx.w2w_xrank_init);
		lmc_wr(priv, CVMX_LMCX_SLOT_CTL1(if_num), slot_ctl1.u64);

		slot_ctl2.u64 = lmc_rd(priv, CVMX_LMCX_SLOT_CTL2(if_num));
		slot_ctl2.cn78xx.w2w_xdimm_init =
		    max(10, (int)slot_ctl2.cn78xx.w2w_xdimm_init);
		lmc_wr(priv, CVMX_LMCX_SLOT_CTL2(if_num), slot_ctl2.u64);
	}
}

static void lmc_final(struct ddr_priv *priv)
{
	/*
	 * 4.8.11 Final LMC Initialization
	 *
	 * Early LMC initialization, LMC write-leveling, and LMC read-leveling
	 * must be completed prior to starting this final LMC initialization.
	 *
	 * LMC hardware updates the LMC(0)_SLOT_CTL0, LMC(0)_SLOT_CTL1,
	 * LMC(0)_SLOT_CTL2 CSRs with minimum values based on the selected
	 * readleveling and write-leveling settings. Software should not write
	 * the final LMC(0)_SLOT_CTL0, LMC(0)_SLOT_CTL1, and LMC(0)_SLOT_CTL2
	 * values until after the final read-leveling and write-leveling
	 * settings are written.
	 *
	 * Software must ensure the LMC(0)_SLOT_CTL0, LMC(0)_SLOT_CTL1, and
	 * LMC(0)_SLOT_CTL2 CSR values are appropriate for this step. These CSRs
	 * select the minimum gaps between read operations and write operations
	 * of various types.
	 *
	 * Software must not reduce the values in these CSR fields below the
	 * values previously selected by the LMC hardware (during write-leveling
	 * and read-leveling steps above).
	 *
	 * All sections in this chapter may be used to derive proper settings
	 * for these registers.
	 *
	 * For minimal read latency, L2C_CTL[EF_ENA,EF_CNT] should be programmed
	 * properly. This should be done prior to the first read.
	 */

	/* Clear any residual ECC errors */
	int num_tads = 1;
	int tad;
	int num_mcis = 1;
	int mci;

	if (octeon_is_cpuid(OCTEON_CN78XX)) {
		num_tads = 8;
		num_mcis = 4;
	} else if (octeon_is_cpuid(OCTEON_CN70XX)) {
		num_tads = 1;
		num_mcis = 1;
	} else if (octeon_is_cpuid(OCTEON_CN73XX) ||
		   octeon_is_cpuid(OCTEON_CNF75XX)) {
		num_tads = 4;
		num_mcis = 3;
	}

	lmc_wr(priv, CVMX_LMCX_INT(if_num), -1ULL);
	lmc_rd(priv, CVMX_LMCX_INT(if_num));

	for (tad = 0; tad < num_tads; tad++) {
		l2c_wr(priv, CVMX_L2C_TADX_INT_REL(tad),
		       l2c_rd(priv, CVMX_L2C_TADX_INT_REL(tad)));
		debug("%-45s : (%d) 0x%08llx\n", "CVMX_L2C_TAD_INT", tad,
		      l2c_rd(priv, CVMX_L2C_TADX_INT_REL(tad)));
	}

	for (mci = 0; mci < num_mcis; mci++) {
		l2c_wr(priv, CVMX_L2C_MCIX_INT_REL(mci),
		       l2c_rd(priv, CVMX_L2C_MCIX_INT_REL(mci)));
		debug("%-45s : (%d) 0x%08llx\n", "L2C_MCI_INT", mci,
		      l2c_rd(priv, CVMX_L2C_MCIX_INT_REL(mci)));
	}

	debug("%-45s : 0x%08llx\n", "LMC_INT",
	      lmc_rd(priv, CVMX_LMCX_INT(if_num)));
}

static void lmc_scrambling(struct ddr_priv *priv)
{
	// Make sure scrambling is disabled during init...
	union cvmx_lmcx_control ctrl;
	union cvmx_lmcx_scramble_cfg0 lmc_scramble_cfg0;
	union cvmx_lmcx_scramble_cfg1 lmc_scramble_cfg1;
	union cvmx_lmcx_scramble_cfg2 lmc_scramble_cfg2;
	union cvmx_lmcx_ns_ctl lmc_ns_ctl;
	int use_scramble = 0;	// default OFF
	char *s;

	ctrl.u64 = lmc_rd(priv, CVMX_LMCX_CONTROL(if_num));
	lmc_scramble_cfg0.u64 = lmc_rd(priv, CVMX_LMCX_SCRAMBLE_CFG0(if_num));
	lmc_scramble_cfg1.u64 = lmc_rd(priv, CVMX_LMCX_SCRAMBLE_CFG1(if_num));
	lmc_scramble_cfg2.u64 = 0;	// quiet compiler
	if (!octeon_is_cpuid(OCTEON_CN78XX_PASS1_X)) {
		lmc_scramble_cfg2.u64 =
		    lmc_rd(priv, CVMX_LMCX_SCRAMBLE_CFG2(if_num));
	}
	lmc_ns_ctl.u64 = lmc_rd(priv, CVMX_LMCX_NS_CTL(if_num));

	s = lookup_env_ull(priv, "ddr_use_scramble");
	if (s)
		use_scramble = simple_strtoull(s, NULL, 0);

	/* Generate random values if scrambling is needed */
	if (use_scramble) {
		lmc_scramble_cfg0.u64 = cvmx_rng_get_random64();
		lmc_scramble_cfg1.u64 = cvmx_rng_get_random64();
		lmc_scramble_cfg2.u64 = cvmx_rng_get_random64();
		lmc_ns_ctl.s.ns_scramble_dis = 0;
		lmc_ns_ctl.s.adr_offset = 0;
		ctrl.s.scramble_ena = 1;
	}

	s = lookup_env_ull(priv, "ddr_scramble_cfg0");
	if (s) {
		lmc_scramble_cfg0.u64 = simple_strtoull(s, NULL, 0);
		ctrl.s.scramble_ena = 1;
	}
	debug("%-45s : 0x%016llx\n", "LMC_SCRAMBLE_CFG0",
	      lmc_scramble_cfg0.u64);

	lmc_wr(priv, CVMX_LMCX_SCRAMBLE_CFG0(if_num), lmc_scramble_cfg0.u64);

	s = lookup_env_ull(priv, "ddr_scramble_cfg1");
	if (s) {
		lmc_scramble_cfg1.u64 = simple_strtoull(s, NULL, 0);
		ctrl.s.scramble_ena = 1;
	}
	debug("%-45s : 0x%016llx\n", "LMC_SCRAMBLE_CFG1",
	      lmc_scramble_cfg1.u64);
	lmc_wr(priv, CVMX_LMCX_SCRAMBLE_CFG1(if_num), lmc_scramble_cfg1.u64);

	if (!octeon_is_cpuid(OCTEON_CN78XX_PASS1_X)) {
		s = lookup_env_ull(priv, "ddr_scramble_cfg2");
		if (s) {
			lmc_scramble_cfg2.u64 = simple_strtoull(s, NULL, 0);
			ctrl.s.scramble_ena = 1;
		}
		debug("%-45s : 0x%016llx\n", "LMC_SCRAMBLE_CFG2",
		      lmc_scramble_cfg1.u64);
		lmc_wr(priv, CVMX_LMCX_SCRAMBLE_CFG2(if_num),
		       lmc_scramble_cfg2.u64);
	}

	s = lookup_env_ull(priv, "ddr_ns_ctl");
	if (s)
		lmc_ns_ctl.u64 = simple_strtoull(s, NULL, 0);
	debug("%-45s : 0x%016llx\n", "LMC_NS_CTL", lmc_ns_ctl.u64);
	lmc_wr(priv, CVMX_LMCX_NS_CTL(if_num), lmc_ns_ctl.u64);

	lmc_wr(priv, CVMX_LMCX_CONTROL(if_num), ctrl.u64);
}

struct rl_score {
	u64 setting;
	int score;
};

static union cvmx_lmcx_rlevel_rankx rl_rank __section(".data");
static union cvmx_lmcx_rlevel_ctl rl_ctl __section(".data");
static unsigned char rodt_ctl __section(".data");

static int rl_rodt_err __section(".data");
static unsigned char rtt_nom __section(".data");
static unsigned char rtt_idx __section(".data");
static char min_rtt_nom_idx __section(".data");
static char max_rtt_nom_idx __section(".data");
static char min_rodt_ctl __section(".data");
static char max_rodt_ctl __section(".data");
static int rl_dbg_loops __section(".data");
static unsigned char save_ddr2t __section(".data");
static int rl_samples __section(".data");
static char rl_compute __section(".data");
static char saved_ddr__ptune __section(".data");
static char saved_ddr__ntune __section(".data");
static char rl_comp_offs __section(".data");
static char saved_int_zqcs_dis __section(".data");
static int max_adj_rl_del_inc __section(".data");
static int print_nom_ohms __section(".data");
static int rl_print __section(".data");

#ifdef ENABLE_HARDCODED_RLEVEL
static char part_number[21] __section(".data");
#endif /* ENABLE_HARDCODED_RLEVEL */

struct perfect_counts {
	u16 count[9][32]; // 8+ECC by 64 values
	u32 mask[9];      // 8+ECC, bitmask of perfect delays
};

static struct perfect_counts rank_perf[4] __section(".data");
static struct perfect_counts rodt_perfect_counts __section(".data");
static int pbm_lowsum_limit __section(".data");
// FIXME: PBM skip for RODT 240 and 34
static u32 pbm_rodt_skip __section(".data");

// control rank majority processing
static int disable_rank_majority __section(".data");

// default to mask 11b ODDs for DDR4 (except 73xx), else DISABLE
// for DDR3
static int enable_rldelay_bump __section(".data");
static int rldelay_bump_incr __section(".data");
static int disable_rlv_bump_this_byte __section(".data");
static u64 value_mask __section(".data");

static struct rlevel_byte_data rl_byte[9] __section(".data");
static int sample_loops __section(".data");
static int max_samples __section(".data");
static int rl_rank_errors __section(".data");
static int rl_mask_err __section(".data");
static int rl_nonseq_err __section(".data");
static struct rlevel_bitmask rl_mask[9] __section(".data");
static int rl_best_rank_score __section(".data");

static int rodt_row_skip_mask __section(".data");

static void rodt_loop(struct ddr_priv *priv, int rankx, struct rl_score
		      rl_score[RTT_NOM_OHMS_COUNT][RODT_OHMS_COUNT][4])
{
	union cvmx_lmcx_comp_ctl2 cc2;
	const int rl_separate_ab = 1;
	int i;

	rl_best_rank_score = DEFAULT_BEST_RANK_SCORE;
	rl_rodt_err = 0;
	cc2.u64 = lmc_rd(priv, CVMX_LMCX_COMP_CTL2(if_num));
	cc2.cn78xx.rodt_ctl = rodt_ctl;
	lmc_wr(priv, CVMX_LMCX_COMP_CTL2(if_num), cc2.u64);
	cc2.u64 = lmc_rd(priv, CVMX_LMCX_COMP_CTL2(if_num));
	udelay(1); /* Give it a little time to take affect */
	if (rl_print > 1) {
		debug("Read ODT_CTL                                  : 0x%x (%d ohms)\n",
		      cc2.cn78xx.rodt_ctl,
		      imp_val->rodt_ohms[cc2.cn78xx.rodt_ctl]);
	}

	memset(rl_byte, 0, sizeof(rl_byte));
	memset(&rodt_perfect_counts, 0, sizeof(rodt_perfect_counts));

	// when iter RODT is the target RODT, take more samples...
	max_samples = rl_samples;
	if (rodt_ctl == default_rodt_ctl)
		max_samples += rl_samples + 1;

	for (sample_loops = 0; sample_loops < max_samples; sample_loops++) {
		int redoing_nonseq_errs = 0;

		rl_mask_err = 0;

		if (!(rl_separate_ab && spd_rdimm &&
		      ddr_type == DDR4_DRAM)) {
			/* Clear read-level delays */
			lmc_wr(priv, CVMX_LMCX_RLEVEL_RANKX(rankx, if_num), 0);

			/* read-leveling */
			oct3_ddr3_seq(priv, 1 << rankx, if_num, 1);

			do {
				rl_rank.u64 =
					lmc_rd(priv,
					       CVMX_LMCX_RLEVEL_RANKX(rankx,
								      if_num));
			} while (rl_rank.cn78xx.status != 3);
		}

		rl_rank.u64 =
			lmc_rd(priv, CVMX_LMCX_RLEVEL_RANKX(rankx, if_num));

		// start bitmask interpretation block

		memset(rl_mask, 0, sizeof(rl_mask));

		if (rl_separate_ab && spd_rdimm && ddr_type == DDR4_DRAM) {
			union cvmx_lmcx_rlevel_rankx rl_rank_aside;
			union cvmx_lmcx_modereg_params0 mp0;

			/* A-side */
			mp0.u64 =
				lmc_rd(priv, CVMX_LMCX_MODEREG_PARAMS0(if_num));
			mp0.s.mprloc = 0; /* MPR Page 0 Location 0 */
			lmc_wr(priv,
			       CVMX_LMCX_MODEREG_PARAMS0(if_num),
			       mp0.u64);

			/* Clear read-level delays */
			lmc_wr(priv, CVMX_LMCX_RLEVEL_RANKX(rankx, if_num), 0);

			/* read-leveling */
			oct3_ddr3_seq(priv, 1 << rankx, if_num, 1);

			do {
				rl_rank.u64 =
					lmc_rd(priv,
					       CVMX_LMCX_RLEVEL_RANKX(rankx,
								      if_num));
			} while (rl_rank.cn78xx.status != 3);

			rl_rank.u64 =
				lmc_rd(priv, CVMX_LMCX_RLEVEL_RANKX(rankx,
								    if_num));

			rl_rank_aside.u64 = rl_rank.u64;

			rl_mask[0].bm = lmc_ddr3_rl_dbg_read(priv, if_num, 0);
			rl_mask[1].bm = lmc_ddr3_rl_dbg_read(priv, if_num, 1);
			rl_mask[2].bm = lmc_ddr3_rl_dbg_read(priv, if_num, 2);
			rl_mask[3].bm = lmc_ddr3_rl_dbg_read(priv, if_num, 3);
			rl_mask[8].bm = lmc_ddr3_rl_dbg_read(priv, if_num, 8);
			/* A-side complete */

			/* B-side */
			mp0.u64 =
				lmc_rd(priv, CVMX_LMCX_MODEREG_PARAMS0(if_num));
			mp0.s.mprloc = 3; /* MPR Page 0 Location 3 */
			lmc_wr(priv, CVMX_LMCX_MODEREG_PARAMS0(if_num),
			       mp0.u64);

			/* Clear read-level delays */
			lmc_wr(priv, CVMX_LMCX_RLEVEL_RANKX(rankx, if_num), 0);

			/* read-leveling */
			oct3_ddr3_seq(priv, 1 << rankx, if_num, 1);

			do {
				rl_rank.u64 =
					lmc_rd(priv,
					       CVMX_LMCX_RLEVEL_RANKX(rankx,
								      if_num));
			} while (rl_rank.cn78xx.status != 3);

			rl_rank.u64 =
				lmc_rd(priv, CVMX_LMCX_RLEVEL_RANKX(rankx,
								    if_num));

			rl_mask[4].bm = lmc_ddr3_rl_dbg_read(priv, if_num, 4);
			rl_mask[5].bm = lmc_ddr3_rl_dbg_read(priv, if_num, 5);
			rl_mask[6].bm = lmc_ddr3_rl_dbg_read(priv, if_num, 6);
			rl_mask[7].bm = lmc_ddr3_rl_dbg_read(priv, if_num, 7);
			/* B-side complete */

			upd_rl_rank(&rl_rank, 0, rl_rank_aside.s.byte0);
			upd_rl_rank(&rl_rank, 1, rl_rank_aside.s.byte1);
			upd_rl_rank(&rl_rank, 2, rl_rank_aside.s.byte2);
			upd_rl_rank(&rl_rank, 3, rl_rank_aside.s.byte3);
			/* ECC A-side */
			upd_rl_rank(&rl_rank, 8, rl_rank_aside.s.byte8);

			mp0.u64 =
				lmc_rd(priv, CVMX_LMCX_MODEREG_PARAMS0(if_num));
			mp0.s.mprloc = 0; /* MPR Page 0 Location 0 */
			lmc_wr(priv, CVMX_LMCX_MODEREG_PARAMS0(if_num),
			       mp0.u64);
		}

		/*
		 * Evaluate the quality of the read-leveling delays from the
		 * bitmasks. Also save off a software computed read-leveling
		 * mask that may be used later to qualify the delay results
		 * from Octeon.
		 */
		for (i = 0; i < (8 + ecc_ena); ++i) {
			int bmerr;

			if (!(if_bytemask & (1 << i)))
				continue;
			if (!(rl_separate_ab && spd_rdimm &&
			      ddr_type == DDR4_DRAM)) {
				rl_mask[i].bm =
					lmc_ddr3_rl_dbg_read(priv, if_num, i);
			}
			bmerr = validate_ddr3_rlevel_bitmask(&rl_mask[i],
							     ddr_type);
			rl_mask[i].errs = bmerr;
			rl_mask_err += bmerr;
			// count only the "perfect" bitmasks
			if (ddr_type == DDR4_DRAM && !bmerr) {
				int delay;
				// FIXME: for now, simple filtering:
				// do NOT count PBMs for RODTs in skip mask
				if ((1U << rodt_ctl) & pbm_rodt_skip)
					continue;
				// FIXME: could optimize this a bit?
				delay = get_rl_rank(&rl_rank, i);
				rank_perf[rankx].count[i][delay] += 1;
				rank_perf[rankx].mask[i] |=
					(1ULL << delay);
				rodt_perfect_counts.count[i][delay] += 1;
				rodt_perfect_counts.mask[i] |= (1ULL << delay);
			}
		}

		/* Set delays for unused bytes to match byte 0. */
		for (i = 0; i < 9; ++i) {
			if (if_bytemask & (1 << i))
				continue;
			upd_rl_rank(&rl_rank, i, rl_rank.s.byte0);
		}

		/*
		 * Save a copy of the byte delays in physical
		 * order for sequential evaluation.
		 */
		unpack_rlevel_settings(if_bytemask, ecc_ena, rl_byte, rl_rank);

	redo_nonseq_errs:

		rl_nonseq_err  = 0;
		if (!disable_sequential_delay_check) {
			for (i = 0; i < 9; ++i)
				rl_byte[i].sqerrs = 0;

			if ((if_bytemask & 0xff) == 0xff) {
				/*
				 * Evaluate delay sequence across the whole
				 * range of bytes for standard dimms.
				 */
				/* 1=RDIMM, 5=Mini-RDIMM */
				if (spd_dimm_type == 1 || spd_dimm_type == 5) {
					int reg_adj_del = abs(rl_byte[4].delay -
							      rl_byte[5].delay);

					/*
					 * Registered dimm topology routes
					 * from the center.
					 */
					rl_nonseq_err +=
						nonseq_del(rl_byte, 0,
							   3 + ecc_ena,
							   max_adj_rl_del_inc);
					rl_nonseq_err +=
						nonseq_del(rl_byte, 5,
							   7 + ecc_ena,
							   max_adj_rl_del_inc);
					// byte 5 sqerrs never gets cleared
					// for RDIMMs
					rl_byte[5].sqerrs = 0;
					if (reg_adj_del > 1) {
						/*
						 * Assess proximity of bytes on
						 * opposite sides of register
						 */
						rl_nonseq_err += (reg_adj_del -
								  1) *
							RLEVEL_ADJACENT_DELAY_ERROR;
						// update byte 5 error
						rl_byte[5].sqerrs +=
							(reg_adj_del - 1) *
							RLEVEL_ADJACENT_DELAY_ERROR;
					}
				}

				/* 2=UDIMM, 6=Mini-UDIMM */
				if (spd_dimm_type == 2 || spd_dimm_type == 6) {
					/*
					 * Unbuffered dimm topology routes
					 * from end to end.
					 */
					rl_nonseq_err += nonseq_del(rl_byte, 0,
								    7 + ecc_ena,
								    max_adj_rl_del_inc);
				}
			} else {
				rl_nonseq_err += nonseq_del(rl_byte, 0,
							    3 + ecc_ena,
							    max_adj_rl_del_inc);
			}
		} /* if (! disable_sequential_delay_check) */

		rl_rank_errors = rl_mask_err + rl_nonseq_err;

		// print original sample here only if we are not really
		// averaging or picking best
		// also do not print if we were redoing the NONSEQ score
		// for using COMPUTED
		if (!redoing_nonseq_errs && rl_samples < 2) {
			if (rl_print > 1) {
				display_rl_bm(if_num, rankx, rl_mask, ecc_ena);
				display_rl_bm_scores(if_num, rankx, rl_mask,
						     ecc_ena);
				display_rl_seq_scores(if_num, rankx, rl_byte,
						      ecc_ena);
			}
			display_rl_with_score(if_num, rl_rank, rankx,
					      rl_rank_errors);
		}

		if (rl_compute) {
			if (!redoing_nonseq_errs) {
				/* Recompute the delays based on the bitmask */
				for (i = 0; i < (8 + ecc_ena); ++i) {
					if (!(if_bytemask & (1 << i)))
						continue;

					upd_rl_rank(&rl_rank, i,
						    compute_ddr3_rlevel_delay(
							    rl_mask[i].mstart,
							    rl_mask[i].width,
							    rl_ctl));
				}

				/*
				 * Override the copy of byte delays with the
				 * computed results.
				 */
				unpack_rlevel_settings(if_bytemask, ecc_ena,
						       rl_byte, rl_rank);

				redoing_nonseq_errs = 1;
				goto redo_nonseq_errs;

			} else {
				/*
				 * now print this if already printed the
				 * original sample
				 */
				if (rl_samples < 2 || rl_print) {
					display_rl_with_computed(if_num,
								 rl_rank, rankx,
								 rl_rank_errors);
				}
			}
		} /* if (rl_compute) */

		// end bitmask interpretation block

		// if it is a better (lower) score, then  keep it
		if (rl_rank_errors < rl_best_rank_score) {
			rl_best_rank_score = rl_rank_errors;

			// save the new best delays and best errors
			for (i = 0; i < (8 + ecc_ena); ++i) {
				rl_byte[i].best = rl_byte[i].delay;
				rl_byte[i].bestsq = rl_byte[i].sqerrs;
				// save bitmasks and their scores as well
				// xlate UNPACKED index to PACKED index to
				// get from rl_mask
				rl_byte[i].bm = rl_mask[XUP(i, !!ecc_ena)].bm;
				rl_byte[i].bmerrs =
					rl_mask[XUP(i, !!ecc_ena)].errs;
			}
		}

		rl_rodt_err += rl_rank_errors;
	}

	/* We recorded the best score across the averaging loops */
	rl_score[rtt_nom][rodt_ctl][rankx].score = rl_best_rank_score;

	/*
	 * Restore the delays from the best fields that go with the best
	 * score
	 */
	for (i = 0; i < 9; ++i) {
		rl_byte[i].delay = rl_byte[i].best;
		rl_byte[i].sqerrs = rl_byte[i].bestsq;
	}

	rl_rank.u64 = lmc_rd(priv, CVMX_LMCX_RLEVEL_RANKX(rankx, if_num));

	pack_rlevel_settings(if_bytemask, ecc_ena, rl_byte, &rl_rank);

	if (rl_samples > 1) {
		// restore the "best" bitmasks and their scores for printing
		for (i = 0; i < 9; ++i) {
			if ((if_bytemask & (1 << i)) == 0)
				continue;
			// xlate PACKED index to UNPACKED index to get from
			// rl_byte
			rl_mask[i].bm   = rl_byte[XPU(i, !!ecc_ena)].bm;
			rl_mask[i].errs = rl_byte[XPU(i, !!ecc_ena)].bmerrs;
		}

		// maybe print bitmasks/scores here
		if (rl_print > 1) {
			display_rl_bm(if_num, rankx, rl_mask, ecc_ena);
			display_rl_bm_scores(if_num, rankx, rl_mask, ecc_ena);
			display_rl_seq_scores(if_num, rankx, rl_byte, ecc_ena);

			display_rl_with_rodt(if_num, rl_rank, rankx,
					     rl_score[rtt_nom][rodt_ctl][rankx].score,
					     print_nom_ohms,
					     imp_val->rodt_ohms[rodt_ctl],
					     WITH_RODT_BESTSCORE);

			debug("-----------\n");
		}
	}

	rl_score[rtt_nom][rodt_ctl][rankx].setting = rl_rank.u64;

	// print out the PBMs for the current RODT
	if (ddr_type == DDR4_DRAM && rl_print > 1) { // verbosity?
		// FIXME: change verbosity level after debug complete...

		for (i = 0; i < 9; i++) {
			u64 temp_mask;
			int num_values;

			// FIXME: PBM skip for RODTs in mask
			if ((1U << rodt_ctl) & pbm_rodt_skip)
				continue;

			temp_mask = rodt_perfect_counts.mask[i];
			num_values = __builtin_popcountll(temp_mask);
			i = __builtin_ffsll(temp_mask) - 1;

			debug("N%d.LMC%d.R%d: PERFECT: RODT %3d: Byte %d: mask 0x%02llx (%d): ",
			      node, if_num, rankx,
			      imp_val->rodt_ohms[rodt_ctl],
			      i, temp_mask >> i, num_values);

			while (temp_mask != 0) {
				i = __builtin_ffsll(temp_mask) - 1;
				debug("%2d(%2d) ", i,
				      rodt_perfect_counts.count[i][i]);
				temp_mask &= ~(1UL << i);
			} /* while (temp_mask != 0) */
			debug("\n");
		}
	}
}

static void rank_major_loop(struct ddr_priv *priv, int rankx, struct rl_score
			    rl_score[RTT_NOM_OHMS_COUNT][RODT_OHMS_COUNT][4])
{
	/* Start with an arbitrarily high score */
	int best_rank_score = DEFAULT_BEST_RANK_SCORE;
	int best_rank_rtt_nom = 0;
	int best_rank_ctl = 0;
	int best_rank_ohms = 0;
	int best_rankx = 0;
	int dimm_rank_mask;
	int max_rank_score;
	union cvmx_lmcx_rlevel_rankx saved_rl_rank;
	int next_ohms;
	int orankx;
	int next_score = 0;
	int best_byte, new_byte, temp_byte, orig_best_byte;
	int rank_best_bytes[9];
	int byte_sh;
	int avg_byte;
	int avg_diff;
	int i;

	if (!(rank_mask & (1 << rankx)))
		return;

	// some of the rank-related loops below need to operate only on
	// the ranks of a single DIMM,
	// so create a mask for their use here
	if (num_ranks == 4) {
		dimm_rank_mask = rank_mask; // should be 1111
	} else {
		dimm_rank_mask = rank_mask & 3; // should be 01 or 11
		if (rankx >= 2) {
			// doing a rank on the second DIMM, should be
			// 0100 or 1100
			dimm_rank_mask <<= 2;
		}
	}
	debug("DIMM rank mask: 0x%x, rank mask: 0x%x, rankx: %d\n",
	      dimm_rank_mask, rank_mask, rankx);

	// this is the start of the BEST ROW SCORE LOOP

	for (rtt_idx = min_rtt_nom_idx; rtt_idx <= max_rtt_nom_idx; ++rtt_idx) {
		rtt_nom = imp_val->rtt_nom_table[rtt_idx];

		debug("N%d.LMC%d.R%d: starting RTT_NOM %d (%d)\n",
		      node, if_num, rankx, rtt_nom,
		      imp_val->rtt_nom_ohms[rtt_nom]);

		for (rodt_ctl = max_rodt_ctl; rodt_ctl >= min_rodt_ctl;
		     --rodt_ctl) {
			next_ohms = imp_val->rodt_ohms[rodt_ctl];

			// skip RODT rows in mask, but *NOT* rows with too
			// high a score;
			// we will not use the skipped ones for printing or
			// evaluating, but we need to allow all the
			// non-skipped ones to be candidates for "best"
			if (((1 << rodt_ctl) & rodt_row_skip_mask) != 0) {
				debug("N%d.LMC%d.R%d: SKIPPING rodt:%d (%d) with rank_score:%d\n",
				      node, if_num, rankx, rodt_ctl,
				      next_ohms, next_score);
				continue;
			}

			// this is ROFFIX-0528
			for (orankx = 0; orankx < dimm_count * 4; orankx++) {
				// stay on the same DIMM
				if (!(dimm_rank_mask & (1 << orankx)))
					continue;

				next_score = rl_score[rtt_nom][rodt_ctl][orankx].score;

				// always skip a higher score
				if (next_score > best_rank_score)
					continue;

				// if scores are equal
				if (next_score == best_rank_score) {
					// always skip lower ohms
					if (next_ohms < best_rank_ohms)
						continue;

					// if same ohms
					if (next_ohms == best_rank_ohms) {
						// always skip the other rank(s)
						if (orankx != rankx)
							continue;
					}
					// else next_ohms are greater,
					// always choose it
				}
				// else next_score is less than current best,
				// so always choose it
				debug("N%d.LMC%d.R%d: new best score: rank %d, rodt %d(%3d), new best %d, previous best %d(%d)\n",
				      node, if_num, rankx, orankx, rodt_ctl, next_ohms, next_score,
				      best_rank_score, best_rank_ohms);
				best_rank_score	    = next_score;
				best_rank_rtt_nom   = rtt_nom;
				//best_rank_nom_ohms  = rtt_nom_ohms;
				best_rank_ctl       = rodt_ctl;
				best_rank_ohms      = next_ohms;
				best_rankx          = orankx;
				rl_rank.u64 =
					rl_score[rtt_nom][rodt_ctl][orankx].setting;
			}
		}
	}

	// this is the end of the BEST ROW SCORE LOOP

	// DANGER, Will Robinson!! Abort now if we did not find a best
	// score at all...
	if (best_rank_score == DEFAULT_BEST_RANK_SCORE) {
		printf("N%d.LMC%d.R%d: WARNING: no best rank score found - resetting node...\n",
		       node, if_num, rankx);
		mdelay(500);
		do_reset(NULL, 0, 0, NULL);
	}

	// FIXME: relative now, but still arbitrary...
	max_rank_score = best_rank_score;
	if (ddr_type == DDR4_DRAM) {
		// halve the range if 2 DIMMs unless they are single rank...
		max_rank_score += (MAX_RANK_SCORE_LIMIT / ((num_ranks > 1) ?
							   dimm_count : 1));
	} else {
		// Since DDR3 typically has a wider score range,
		// keep more of them always
		max_rank_score += MAX_RANK_SCORE_LIMIT;
	}

	if (!ecc_ena) {
		/* ECC is not used */
		rl_rank.s.byte8 = rl_rank.s.byte0;
	}

	// at the end, write the best row settings to the current rank
	lmc_wr(priv, CVMX_LMCX_RLEVEL_RANKX(rankx, if_num), rl_rank.u64);
	rl_rank.u64 = lmc_rd(priv, CVMX_LMCX_RLEVEL_RANKX(rankx, if_num));

	saved_rl_rank.u64 = rl_rank.u64;

	// this is the start of the PRINT LOOP
	int pass;

	// for pass==0, print current rank, pass==1 print other rank(s)
	// this is done because we want to show each ranks RODT values
	// together, not interlaced
	// keep separates for ranks - pass=0 target rank, pass=1 other
	// rank on DIMM
	int mask_skipped[2] = {0, 0};
	int score_skipped[2] = {0, 0};
	int selected_rows[2] = {0, 0};
	int zero_scores[2] = {0, 0};
	for (pass = 0; pass < 2; pass++) {
		for (orankx = 0; orankx < dimm_count * 4; orankx++) {
			// stay on the same DIMM
			if (!(dimm_rank_mask & (1 << orankx)))
				continue;

			if ((pass == 0 && orankx != rankx) ||
			    (pass != 0 && orankx == rankx))
				continue;

			for (rtt_idx = min_rtt_nom_idx;
			     rtt_idx <= max_rtt_nom_idx; ++rtt_idx) {
				rtt_nom = imp_val->rtt_nom_table[rtt_idx];
				if (dyn_rtt_nom_mask == 0) {
					print_nom_ohms = -1;
				} else {
					print_nom_ohms =
						imp_val->rtt_nom_ohms[rtt_nom];
				}

				// cycle through all the RODT values...
				for (rodt_ctl = max_rodt_ctl;
				     rodt_ctl >= min_rodt_ctl; --rodt_ctl) {
					union cvmx_lmcx_rlevel_rankx
						temp_rl_rank;
					int temp_score =
						rl_score[rtt_nom][rodt_ctl][orankx].score;
					int skip_row;

					temp_rl_rank.u64 =
						rl_score[rtt_nom][rodt_ctl][orankx].setting;

					// skip RODT rows in mask, or rows
					// with too high a score;
					// we will not use them for printing
					// or evaluating...
					if ((1 << rodt_ctl) &
					    rodt_row_skip_mask) {
						skip_row = WITH_RODT_SKIPPING;
						++mask_skipped[pass];
					} else if (temp_score >
						   max_rank_score) {
						skip_row = WITH_RODT_SKIPPING;
						++score_skipped[pass];
					} else {
						skip_row = WITH_RODT_BLANK;
						++selected_rows[pass];
						if (temp_score == 0)
							++zero_scores[pass];
					}

					// identify and print the BEST ROW
					// when it comes up
					if (skip_row == WITH_RODT_BLANK &&
					    best_rankx == orankx &&
					    best_rank_rtt_nom == rtt_nom &&
					    best_rank_ctl == rodt_ctl)
						skip_row = WITH_RODT_BESTROW;

					if (rl_print) {
						display_rl_with_rodt(if_num,
								     temp_rl_rank, orankx, temp_score,
								     print_nom_ohms,
								     imp_val->rodt_ohms[rodt_ctl],
								     skip_row);
					}
				}
			}
		}
	}
	debug("N%d.LMC%d.R%d: RLROWS: selected %d+%d, zero_scores %d+%d, mask_skipped %d+%d, score_skipped %d+%d\n",
	      node, if_num, rankx, selected_rows[0], selected_rows[1],
	      zero_scores[0], zero_scores[1], mask_skipped[0], mask_skipped[1],
	      score_skipped[0], score_skipped[1]);
	// this is the end of the PRINT LOOP

	// now evaluate which bytes need adjusting
	// collect the new byte values; first init with current best for
	// neighbor use
	for (i = 0, byte_sh = 0; i < 8 + ecc_ena; i++, byte_sh += 6) {
		rank_best_bytes[i] = (int)(rl_rank.u64 >> byte_sh) &
			RLEVEL_BYTE_MSK;
	}

	// this is the start of the BEST BYTE LOOP

	for (i = 0, byte_sh = 0; i < 8 + ecc_ena; i++, byte_sh += 6) {
		int sum = 0, count = 0;
		int count_less = 0, count_same = 0, count_more = 0;
		int count_byte; // save the value we counted around
		// for rank majority use
		int rank_less = 0, rank_same = 0, rank_more = 0;
		int neighbor;
		int neigh_byte;

		best_byte = rank_best_bytes[i];
		orig_best_byte = rank_best_bytes[i];

		// this is the start of the BEST BYTE AVERAGING LOOP

		// validate the initial "best" byte by looking at the
		// average of the unskipped byte-column entries
		// we want to do this before we go further, so we can
		// try to start with a better initial value
		// this is the so-called "BESTBUY" patch set

		for (rtt_idx = min_rtt_nom_idx; rtt_idx <= max_rtt_nom_idx;
		     ++rtt_idx) {
			rtt_nom = imp_val->rtt_nom_table[rtt_idx];

			for (rodt_ctl = max_rodt_ctl; rodt_ctl >= min_rodt_ctl;
			     --rodt_ctl) {
				union cvmx_lmcx_rlevel_rankx temp_rl_rank;
				int temp_score;

				// average over all the ranks
				for (orankx = 0; orankx < dimm_count * 4;
				     orankx++) {
					// stay on the same DIMM
					if (!(dimm_rank_mask & (1 << orankx)))
						continue;

					temp_score =
						rl_score[rtt_nom][rodt_ctl][orankx].score;
					// skip RODT rows in mask, or rows with
					// too high a score;
					// we will not use them for printing or
					// evaluating...

					if (!((1 << rodt_ctl) &
					      rodt_row_skip_mask) &&
					    temp_score <= max_rank_score) {
						temp_rl_rank.u64 =
							rl_score[rtt_nom][rodt_ctl][orankx].setting;
						temp_byte =
							(int)(temp_rl_rank.u64 >> byte_sh) &
							RLEVEL_BYTE_MSK;
						sum += temp_byte;
						count++;
					}
				}
			}
		}

		// this is the end of the BEST BYTE AVERAGING LOOP

		// FIXME: validate count and sum??
		avg_byte = (int)divide_nint(sum, count);
		avg_diff = best_byte - avg_byte;
		new_byte = best_byte;
		if (avg_diff != 0) {
			// bump best up/dn by 1, not necessarily all the
			// way to avg
			new_byte = best_byte + ((avg_diff > 0) ? -1 : 1);
		}

		if (rl_print) {
			debug("N%d.LMC%d.R%d: START:   Byte %d: best %d is different by %d from average %d, using %d.\n",
			      node, if_num, rankx,
			      i, best_byte, avg_diff, avg_byte, new_byte);
		}
		best_byte = new_byte;
		count_byte = new_byte; // save the value we will count around

		// At this point best_byte is either:
		// 1. the original byte-column value from the best scoring
		//    RODT row, OR
		// 2. that value bumped toward the average of all the
		//    byte-column values
		//
		// best_byte will not change from here on...

		// this is the start of the BEST BYTE COUNTING LOOP

		// NOTE: we do this next loop separately from above, because
		// we count relative to "best_byte"
		// which may have been modified by the above averaging
		// operation...

		for (rtt_idx = min_rtt_nom_idx; rtt_idx <= max_rtt_nom_idx;
		     ++rtt_idx) {
			rtt_nom = imp_val->rtt_nom_table[rtt_idx];

			for (rodt_ctl = max_rodt_ctl; rodt_ctl >= min_rodt_ctl;
			     --rodt_ctl) {
				union cvmx_lmcx_rlevel_rankx temp_rl_rank;
				int temp_score;

				for (orankx = 0; orankx < dimm_count * 4;
				     orankx++) { // count over all the ranks
					// stay on the same DIMM
					if (!(dimm_rank_mask & (1 << orankx)))
						continue;

					temp_score =
						rl_score[rtt_nom][rodt_ctl][orankx].score;
					// skip RODT rows in mask, or rows
					// with too high a score;
					// we will not use them for printing
					// or evaluating...
					if (((1 << rodt_ctl) &
					     rodt_row_skip_mask) ||
					    temp_score > max_rank_score)
						continue;

					temp_rl_rank.u64 =
						rl_score[rtt_nom][rodt_ctl][orankx].setting;
					temp_byte = (temp_rl_rank.u64 >>
						     byte_sh) & RLEVEL_BYTE_MSK;

					if (temp_byte == 0)
						;  // do not count it if illegal
					else if (temp_byte == best_byte)
						count_same++;
					else if (temp_byte == best_byte - 1)
						count_less++;
					else if (temp_byte == best_byte + 1)
						count_more++;
					// else do not count anything more
					// than 1 away from the best

					// no rank counting if disabled
					if (disable_rank_majority)
						continue;

					// FIXME? count is relative to
					// best_byte; should it be rank-based?
					// rank counts only on main rank
					if (orankx != rankx)
						continue;
					else if (temp_byte == best_byte)
						rank_same++;
					else if (temp_byte == best_byte - 1)
						rank_less++;
					else if (temp_byte == best_byte + 1)
						rank_more++;
				}
			}
		}

		if (rl_print) {
			debug("N%d.LMC%d.R%d: COUNT:   Byte %d: orig %d now %d, more %d same %d less %d (%d/%d/%d)\n",
			      node, if_num, rankx,
			      i, orig_best_byte, best_byte,
			      count_more, count_same, count_less,
			      rank_more, rank_same, rank_less);
		}

		// this is the end of the BEST BYTE COUNTING LOOP

		// choose the new byte value
		// we need to check that there is no gap greater than 2
		// between adjacent bytes (adjacency depends on DIMM type)
		// use the neighbor value to help decide
		// initially, the rank_best_bytes[] will contain values from
		// the chosen lowest score rank
		new_byte = 0;

		// neighbor is index-1 unless we are index 0 or index 8 (ECC)
		neighbor = (i == 8) ? 3 : ((i == 0) ? 1 : i - 1);
		neigh_byte = rank_best_bytes[neighbor];

		// can go up or down or stay the same, so look at a numeric
		// average to help
		new_byte = (int)divide_nint(((count_more * (best_byte + 1)) +
					     (count_same * (best_byte + 0)) +
					     (count_less * (best_byte - 1))),
					    max(1, (count_more + count_same +
						    count_less)));

		// use neighbor to help choose with average
		if (i > 0 && (abs(neigh_byte - new_byte) > 2) &&
		    !disable_sequential_delay_check) {
			// but not for byte 0
			int avg_pick = new_byte;

			if ((new_byte - best_byte) != 0) {
				// back to best, average did not get better
				new_byte = best_byte;
			} else {
				// avg was the same, still too far, now move
				// it towards the neighbor
				new_byte += (neigh_byte > new_byte) ? 1 : -1;
			}

			if (rl_print) {
				debug("N%d.LMC%d.R%d: AVERAGE: Byte %d: neighbor %d too different %d from average %d, picking %d.\n",
				      node, if_num, rankx,
				      i, neighbor, neigh_byte, avg_pick,
				      new_byte);
			}
		} else {
			// NOTE:
			// For now, we let the neighbor processing above trump
			// the new simple majority processing here.
			// This is mostly because we have seen no smoking gun
			// for a neighbor bad choice (yet?).
			// Also note that we will ALWAYS be using byte 0
			// majority, because of the if clause above.

			// majority is dependent on the counts, which are
			// relative to best_byte, so start there
			int maj_byte = best_byte;
			int rank_maj;
			int rank_sum;

			if (count_more > count_same &&
			    count_more > count_less) {
				maj_byte++;
			} else if (count_less > count_same &&
				   count_less > count_more) {
				maj_byte--;
			}

			if (maj_byte != new_byte) {
				// print only when majority choice is
				// different from average
				if (rl_print) {
					debug("N%d.LMC%d.R%d: MAJORTY: Byte %d: picking majority of %d over average %d.\n",
					      node, if_num, rankx, i, maj_byte,
					      new_byte);
				}
				new_byte = maj_byte;
			} else {
				if (rl_print) {
					debug("N%d.LMC%d.R%d: AVERAGE: Byte %d: picking average of %d.\n",
					      node, if_num, rankx, i, new_byte);
				}
			}

			if (!disable_rank_majority) {
				// rank majority is dependent on the rank
				// counts, which are relative to best_byte,
				// so start there, and adjust according to the
				// rank counts majority
				rank_maj = best_byte;
				if (rank_more > rank_same &&
				    rank_more > rank_less) {
					rank_maj++;
				} else if (rank_less > rank_same &&
					   rank_less > rank_more) {
					rank_maj--;
				}
				rank_sum = rank_more + rank_same + rank_less;

				// now, let rank majority possibly rule over
				// the current new_byte however we got it
				if (rank_maj != new_byte) { // only if different
					// Here is where we decide whether to
					// completely apply RANK_MAJORITY or not
					// ignore if less than
					if (rank_maj < new_byte) {
						if (rl_print) {
							debug("N%d.LMC%d.R%d: RANKMAJ: Byte %d: LESS: NOT using %d over %d.\n",
							      node, if_num,
							      rankx, i,
							      rank_maj,
							      new_byte);
						}
					} else {
						// For the moment, we do it
						// ONLY when running 2-slot
						// configs
						//  OR when rank_sum is big
						// enough
						if (dimm_count > 1 ||
						    rank_sum > 2) {
							// print only when rank
							// majority choice is
							// selected
							if (rl_print) {
								debug("N%d.LMC%d.R%d: RANKMAJ: Byte %d: picking %d over %d.\n",
								      node,
								      if_num,
								      rankx,
								      i,
								      rank_maj,
								      new_byte);
							}
							new_byte = rank_maj;
						} else {
							// FIXME: print some
							// info when we could
							// have chosen RANKMAJ
							// but did not
							if (rl_print) {
								debug("N%d.LMC%d.R%d: RANKMAJ: Byte %d: NOT using %d over %d (best=%d,sum=%d).\n",
								      node,
								      if_num,
								      rankx,
								      i,
								      rank_maj,
								      new_byte,
								      best_byte,
								      rank_sum);
							}
						}
					}
				}
			} /* if (!disable_rank_majority) */
		}
		// one last check:
		// if new_byte is still count_byte, BUT there was no count
		// for that value, DO SOMETHING!!!
		// FIXME: go back to original best byte from the best row
		if (new_byte == count_byte && count_same == 0) {
			new_byte = orig_best_byte;
			if (rl_print) {
				debug("N%d.LMC%d.R%d: FAILSAF: Byte %d: going back to original %d.\n",
				      node, if_num, rankx, i, new_byte);
			}
		}
		// Look at counts for "perfect" bitmasks (PBMs) if we had
		// any for this byte-lane.
		// Remember, we only counted for DDR4, so zero means none
		// or DDR3, and we bypass this...
		value_mask = rank_perf[rankx].mask[i];
		disable_rlv_bump_this_byte = 0;

		if (value_mask != 0 && rl_ctl.cn78xx.offset == 1) {
			int i, delay_count, delay_max = 0, del_val = 0;
			int num_values = __builtin_popcountll(value_mask);
			int sum_counts = 0;
			u64 temp_mask = value_mask;

			disable_rlv_bump_this_byte = 1;
			i = __builtin_ffsll(temp_mask) - 1;
			if (rl_print)
				debug("N%d.LMC%d.R%d: PERFECT: Byte %d: OFF1: mask 0x%02llx (%d): ",
				      node, if_num, rankx, i, value_mask >> i,
				      num_values);

			while (temp_mask != 0) {
				i = __builtin_ffsll(temp_mask) - 1;
				delay_count = rank_perf[rankx].count[i][i];
				sum_counts += delay_count;
				if (rl_print)
					debug("%2d(%2d) ", i, delay_count);
				if (delay_count >= delay_max) {
					delay_max = delay_count;
					del_val = i;
				}
				temp_mask &= ~(1UL << i);
			} /* while (temp_mask != 0) */

			// if sum_counts is small, just use NEW_BYTE
			if (sum_counts < pbm_lowsum_limit) {
				if (rl_print)
					debug(": LOWSUM (%2d), choose ORIG ",
					      sum_counts);
				del_val = new_byte;
				delay_max = rank_perf[rankx].count[i][del_val];
			}

			// finish printing here...
			if (rl_print) {
				debug(": USING %2d (%2d) D%d\n", del_val,
				      delay_max, disable_rlv_bump_this_byte);
			}

			new_byte = del_val; // override with best PBM choice

		} else if ((value_mask != 0) && (rl_ctl.cn78xx.offset == 2)) {
			//                        if (value_mask != 0) {
			int i, delay_count, del_val;
			int num_values = __builtin_popcountll(value_mask);
			int sum_counts = 0;
			u64 temp_mask = value_mask;

			i = __builtin_ffsll(temp_mask) - 1;
			if (rl_print)
				debug("N%d.LMC%d.R%d: PERFECT: Byte %d: mask 0x%02llx (%d): ",
				      node, if_num, rankx, i, value_mask >> i,
				      num_values);
			while (temp_mask != 0) {
				i = __builtin_ffsll(temp_mask) - 1;
				delay_count = rank_perf[rankx].count[i][i];
				sum_counts += delay_count;
				if (rl_print)
					debug("%2d(%2d) ", i, delay_count);
				temp_mask &= ~(1UL << i);
			} /* while (temp_mask != 0) */

			del_val = __builtin_ffsll(value_mask) - 1;
			delay_count =
				rank_perf[rankx].count[i][del_val];

			// overkill, normally only 1-4 bits
			i = (value_mask >> del_val) & 0x1F;

			// if sum_counts is small, treat as special and use
			// NEW_BYTE
			if (sum_counts < pbm_lowsum_limit) {
				if (rl_print)
					debug(": LOWSUM (%2d), choose ORIG",
					      sum_counts);
				i = 99; // SPECIAL case...
			}

			switch (i) {
			case 0x01 /* 00001b */:
				// allow BUMP
				break;

			case 0x13 /* 10011b */:
			case 0x0B /* 01011b */:
			case 0x03 /* 00011b */:
				del_val += 1; // take the second
				disable_rlv_bump_this_byte = 1; // allow no BUMP
				break;

			case 0x0D /* 01101b */:
			case 0x05 /* 00101b */:
				// test count of lowest and all
				if (delay_count >= 5 || sum_counts <= 5)
					del_val += 1; // take the hole
				else
					del_val += 2; // take the next set
				disable_rlv_bump_this_byte = 1; // allow no BUMP
				break;

			case 0x0F /* 01111b */:
			case 0x17 /* 10111b */:
			case 0x07 /* 00111b */:
				del_val += 1; // take the second
				if (delay_count < 5) { // lowest count is small
					int second =
						rank_perf[rankx].count[i][del_val];
					int third =
						rank_perf[rankx].count[i][del_val + 1];
					// test if middle is more than 1 OR
					// top is more than 1;
					// this means if they are BOTH 1,
					// then we keep the second...
					if (second > 1 || third > 1) {
						// if middle is small OR top
						// is large
						if (second < 5 ||
						    third > 1) {
							// take the top
							del_val += 1;
							if (rl_print)
								debug(": TOP7 ");
						}
					}
				}
				disable_rlv_bump_this_byte = 1; // allow no BUMP
				break;

			default: // all others...
				if (rl_print)
					debug(": ABNORMAL, choose ORIG");

			case 99: // special
				 // FIXME: choose original choice?
				del_val = new_byte;
				disable_rlv_bump_this_byte = 1; // allow no BUMP
				break;
			}
			delay_count =
				rank_perf[rankx].count[i][del_val];

			// finish printing here...
			if (rl_print)
				debug(": USING %2d (%2d) D%d\n", del_val,
				      delay_count, disable_rlv_bump_this_byte);
			new_byte = del_val; // override with best PBM choice
		} else {
			if (ddr_type == DDR4_DRAM) { // only report when DDR4
				// FIXME: remove or increase VBL for this
				// output...
				if (rl_print)
					debug("N%d.LMC%d.R%d: PERFECT: Byte %d: ZERO PBMs, USING %d\n",
					      node, if_num, rankx, i,
					      new_byte);
				// prevent ODD bump, rely on original
				disable_rlv_bump_this_byte = 1;
			}
		} /* if (value_mask != 0) */

		// optionally bump the delay value
		if (enable_rldelay_bump && !disable_rlv_bump_this_byte) {
			if ((new_byte & enable_rldelay_bump) ==
			    enable_rldelay_bump) {
				int bump_value = new_byte + rldelay_bump_incr;

				if (rl_print) {
					debug("N%d.LMC%d.R%d: RLVBUMP: Byte %d: CHANGING %d to %d (%s)\n",
					      node, if_num, rankx, i,
					      new_byte, bump_value,
					      (value_mask &
					       (1 << bump_value)) ?
					      "PBM" : "NOPBM");
				}
				new_byte = bump_value;
			}
		}

		// last checks for count-related purposes
		if (new_byte == best_byte && count_more > 0 &&
		    count_less == 0) {
			// we really should take best_byte + 1
			if (rl_print) {
				debug("N%d.LMC%d.R%d: CADJMOR: Byte %d: CHANGING %d to %d\n",
				      node, if_num, rankx, i,
				      new_byte, best_byte + 1);
				new_byte = best_byte + 1;
			}
		} else if ((new_byte < best_byte) && (count_same > 0)) {
			// we really should take best_byte
			if (rl_print) {
				debug("N%d.LMC%d.R%d: CADJSAM: Byte %d: CHANGING %d to %d\n",
				      node, if_num, rankx, i,
				      new_byte, best_byte);
				new_byte = best_byte;
			}
		} else if (new_byte > best_byte) {
			if ((new_byte == (best_byte + 1)) &&
			    count_more == 0 && count_less > 0) {
				// we really should take best_byte
				if (rl_print) {
					debug("N%d.LMC%d.R%d: CADJLE1: Byte %d: CHANGING %d to %d\n",
					      node, if_num, rankx, i,
					      new_byte, best_byte);
					new_byte = best_byte;
				}
			} else if ((new_byte >= (best_byte + 2)) &&
				   ((count_more > 0) || (count_same > 0))) {
				if (rl_print) {
					debug("N%d.LMC%d.R%d: CADJLE2: Byte %d: CHANGING %d to %d\n",
					      node, if_num, rankx, i,
					      new_byte, best_byte + 1);
					new_byte = best_byte + 1;
				}
			}
		}

		if (rl_print) {
			debug("N%d.LMC%d.R%d: SUMMARY: Byte %d: orig %d now %d, more %d same %d less %d, using %d\n",
			      node, if_num, rankx, i, orig_best_byte,
			      best_byte, count_more, count_same, count_less,
			      new_byte);
		}

		// update the byte with the new value (NOTE: orig value in
		// the CSR may not be current "best")
		upd_rl_rank(&rl_rank, i, new_byte);

		// save new best for neighbor use
		rank_best_bytes[i] = new_byte;
	} /* for (i = 0; i < 8+ecc_ena; i++) */

	////////////////// this is the end of the BEST BYTE LOOP

	if (saved_rl_rank.u64 != rl_rank.u64) {
		lmc_wr(priv, CVMX_LMCX_RLEVEL_RANKX(rankx, if_num),
		       rl_rank.u64);
		rl_rank.u64 = lmc_rd(priv,
				     CVMX_LMCX_RLEVEL_RANKX(rankx, if_num));
		debug("Adjusting Read-Leveling per-RANK settings.\n");
	} else {
		debug("Not Adjusting Read-Leveling per-RANK settings.\n");
	}
	display_rl_with_final(if_num, rl_rank, rankx);

	// FIXME: does this help make the output a little easier to focus?
	if (rl_print > 0)
		debug("-----------\n");

#define RLEVEL_RANKX_EXTRAS_INCR  0
	// if there are unused entries to be filled
	if ((rank_mask & 0x0f) != 0x0f) {
		// copy the current rank
		union cvmx_lmcx_rlevel_rankx temp_rl_rank = rl_rank;

		if (rankx < 3) {
#if RLEVEL_RANKX_EXTRAS_INCR > 0
			int byte, delay;

			// modify the copy in prep for writing to empty slot(s)
			for (byte = 0; byte < 9; byte++) {
				delay = get_rl_rank(&temp_rl_rank, byte) +
					RLEVEL_RANKX_EXTRAS_INCR;
				if (delay > RLEVEL_BYTE_MSK)
					delay = RLEVEL_BYTE_MSK;
				upd_rl_rank(&temp_rl_rank, byte, delay);
			}
#endif

			// if rank 0, write rank 1 and rank 2 here if empty
			if (rankx == 0) {
				// check that rank 1 is empty
				if (!(rank_mask & (1 << 1))) {
					debug("N%d.LMC%d.R%d: writing RLEVEL_RANK unused entry R%d.\n",
					      node, if_num, rankx, 1);
					lmc_wr(priv,
					       CVMX_LMCX_RLEVEL_RANKX(1,
								      if_num),
					       temp_rl_rank.u64);
				}

				// check that rank 2 is empty
				if (!(rank_mask & (1 << 2))) {
					debug("N%d.LMC%d.R%d: writing RLEVEL_RANK unused entry R%d.\n",
					      node, if_num, rankx, 2);
					lmc_wr(priv,
					       CVMX_LMCX_RLEVEL_RANKX(2,
								      if_num),
					       temp_rl_rank.u64);
				}
			}

			// if ranks 0, 1 or 2, write rank 3 here if empty
			// check that rank 3 is empty
			if (!(rank_mask & (1 << 3))) {
				debug("N%d.LMC%d.R%d: writing RLEVEL_RANK unused entry R%d.\n",
				      node, if_num, rankx, 3);
				lmc_wr(priv, CVMX_LMCX_RLEVEL_RANKX(3, if_num),
				       temp_rl_rank.u64);
			}
		}
	}
}

static void lmc_read_leveling(struct ddr_priv *priv)
{
	struct rl_score rl_score[RTT_NOM_OHMS_COUNT][RODT_OHMS_COUNT][4];
	union cvmx_lmcx_control ctl;
	union cvmx_lmcx_config cfg;
	int rankx;
	char *s;
	int i;

	/*
	 * 4.8.10 LMC Read Leveling
	 *
	 * LMC supports an automatic read-leveling separately per byte-lane
	 * using the DDR3 multipurpose register predefined pattern for system
	 * calibration defined in the JEDEC DDR3 specifications.
	 *
	 * All of DDR PLL, LMC CK, and LMC DRESET, and early LMC initializations
	 * must be completed prior to starting this LMC read-leveling sequence.
	 *
	 * Software could simply write the desired read-leveling values into
	 * LMC(0)_RLEVEL_RANK(0..3). This section describes a sequence that uses
	 * LMC's autoread-leveling capabilities.
	 *
	 * When LMC does the read-leveling sequence for a rank, it first enables
	 * the DDR3 multipurpose register predefined pattern for system
	 * calibration on the selected DRAM rank via a DDR3 MR3 write, then
	 * executes 64 RD operations at different internal delay settings, then
	 * disables the predefined pattern via another DDR3 MR3 write
	 * operation. LMC determines the pass or fail of each of the 64 settings
	 * independently for each byte lane, then writes appropriate
	 * LMC(0)_RLEVEL_RANK(0..3)[BYTE*] values for the rank.
	 *
	 * After read-leveling for a rank, software can read the 64 pass/fail
	 * indications for one byte lane via LMC(0)_RLEVEL_DBG[BITMASK].
	 * Software can observe all pass/fail results for all byte lanes in a
	 * rank via separate read-leveling sequences on the rank with different
	 * LMC(0)_RLEVEL_CTL[BYTE] values.
	 *
	 * The 64 pass/fail results will typically have failures for the low
	 * delays, followed by a run of some passing settings, followed by more
	 * failures in the remaining high delays.  LMC sets
	 * LMC(0)_RLEVEL_RANK(0..3)[BYTE*] to one of the passing settings.
	 * First, LMC selects the longest run of successes in the 64 results.
	 * (In the unlikely event that there is more than one longest run, LMC
	 * selects the first one.) Then if LMC(0)_RLEVEL_CTL[OFFSET_EN] = 1 and
	 * the selected run has more than LMC(0)_RLEVEL_CTL[OFFSET] successes,
	 * LMC selects the last passing setting in the run minus
	 * LMC(0)_RLEVEL_CTL[OFFSET]. Otherwise LMC selects the middle setting
	 * in the run (rounding earlier when necessary). We expect the
	 * read-leveling sequence to produce good results with the reset values
	 * LMC(0)_RLEVEL_CTL [OFFSET_EN]=1, LMC(0)_RLEVEL_CTL[OFFSET] = 2.
	 *
	 * The read-leveling sequence has the following steps:
	 *
	 * 1. Select desired LMC(0)_RLEVEL_CTL[OFFSET_EN,OFFSET,BYTE] settings.
	 *    Do the remaining substeps 2-4 separately for each rank i with
	 *    attached DRAM.
	 *
	 * 2. Without changing any other fields in LMC(0)_CONFIG,
	 *
	 *    o write LMC(0)_SEQ_CTL[SEQ_SEL] to select read-leveling
	 *
	 *    o write LMC(0)_CONFIG[RANKMASK] = (1 << i)
	 *
	 *    o write LMC(0)_SEQ_CTL[INIT_START] = 1
	 *
	 *    This initiates the previously-described read-leveling.
	 *
	 * 3. Wait until LMC(0)_RLEVEL_RANKi[STATUS] != 2
	 *
	 *    LMC will have updated LMC(0)_RLEVEL_RANKi[BYTE*] for all byte
	 *    lanes at this point.
	 *
	 *    If ECC DRAM is not present (i.e. when DRAM is not attached to the
	 *    DDR_CBS_0_* and DDR_CB<7:0> chip signals, or the DDR_DQS_<4>_* and
	 *    DDR_DQ<35:32> chip signals), write LMC(0)_RLEVEL_RANK*[BYTE8] =
	 *    LMC(0)_RLEVEL_RANK*[BYTE0]. Write LMC(0)_RLEVEL_RANK*[BYTE4] =
	 *    LMC(0)_RLEVEL_RANK*[BYTE0].
	 *
	 * 4. If desired, consult LMC(0)_RLEVEL_DBG[BITMASK] and compare to
	 *    LMC(0)_RLEVEL_RANKi[BYTE*] for the lane selected by
	 *    LMC(0)_RLEVEL_CTL[BYTE]. If desired, modify
	 *    LMC(0)_RLEVEL_CTL[BYTE] to a new value and repeat so that all
	 *    BITMASKs can be observed.
	 *
	 * 5. Initialize LMC(0)_RLEVEL_RANK* values for all unused ranks.
	 *
	 *    Let rank i be a rank with attached DRAM.
	 *
	 *    For all ranks j that do not have attached DRAM, set
	 *    LMC(0)_RLEVEL_RANKj = LMC(0)_RLEVEL_RANKi.
	 *
	 * This read-leveling sequence can help select the proper CN70XX ODT
	 * resistance value (LMC(0)_COMP_CTL2[RODT_CTL]). A hardware-generated
	 * LMC(0)_RLEVEL_RANKi[BYTEj] value (for a used byte lane j) that is
	 * drastically different from a neighboring LMC(0)_RLEVEL_RANKi[BYTEk]
	 * (for a used byte lane k) can indicate that the CN70XX ODT value is
	 * bad. It is possible to simultaneously optimize both
	 * LMC(0)_COMP_CTL2[RODT_CTL] and LMC(0)_RLEVEL_RANKn[BYTE*] values by
	 * performing this read-leveling sequence for several
	 * LMC(0)_COMP_CTL2[RODT_CTL] values and selecting the one with the
	 * best LMC(0)_RLEVEL_RANKn[BYTE*] profile for the ranks.
	 */

	rl_rodt_err = 0;
	rl_dbg_loops = 1;
	saved_int_zqcs_dis = 0;
	max_adj_rl_del_inc = 0;
	rl_print = RLEVEL_PRINTALL_DEFAULT;

#ifdef ENABLE_HARDCODED_RLEVEL
	part_number[21] = {0};
#endif /* ENABLE_HARDCODED_RLEVEL */

	pbm_lowsum_limit = 5; // FIXME: is this a good default?
	// FIXME: PBM skip for RODT 240 and 34
	pbm_rodt_skip = (1U << ddr4_rodt_ctl_240_ohm) |
		(1U << ddr4_rodt_ctl_34_ohm);

	disable_rank_majority = 0; // control rank majority processing

	// default to mask 11b ODDs for DDR4 (except 73xx), else DISABLE
	// for DDR3
	rldelay_bump_incr = 0;
	disable_rlv_bump_this_byte = 0;

	enable_rldelay_bump = (ddr_type == DDR4_DRAM) ?
		((octeon_is_cpuid(OCTEON_CN73XX)) ? 1 : 3) : 0;

	s = lookup_env(priv, "ddr_disable_rank_majority");
	if (s)
		disable_rank_majority = !!simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_pbm_lowsum_limit");
	if (s)
		pbm_lowsum_limit = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_pbm_rodt_skip");
	if (s)
		pbm_rodt_skip = simple_strtoul(s, NULL, 0);
	memset(rank_perf, 0, sizeof(rank_perf));

	ctl.u64 = lmc_rd(priv, CVMX_LMCX_CONTROL(if_num));
	save_ddr2t = ctl.cn78xx.ddr2t;

	cfg.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
	ecc_ena = cfg.cn78xx.ecc_ena;

	s = lookup_env(priv, "ddr_rlevel_2t");
	if (s)
		ctl.cn78xx.ddr2t = simple_strtoul(s, NULL, 0);

	lmc_wr(priv, CVMX_LMCX_CONTROL(if_num), ctl.u64);

	debug("LMC%d: Performing Read-Leveling\n", if_num);

	rl_ctl.u64 = lmc_rd(priv, CVMX_LMCX_RLEVEL_CTL(if_num));

	rl_samples = c_cfg->rlevel_average_loops;
	if (rl_samples == 0) {
		rl_samples = RLEVEL_SAMPLES_DEFAULT;
		// up the samples for these cases
		if (dimm_count == 1 || num_ranks == 1)
			rl_samples = rl_samples * 2 + 1;
	}

	rl_compute = c_cfg->rlevel_compute;
	rl_ctl.cn78xx.offset_en = c_cfg->offset_en;
	rl_ctl.cn78xx.offset    = spd_rdimm
		? c_cfg->offset_rdimm
		: c_cfg->offset_udimm;

	int value = 1; // should ALWAYS be set

	s = lookup_env(priv, "ddr_rlevel_delay_unload");
	if (s)
		value = !!simple_strtoul(s, NULL, 0);
	rl_ctl.cn78xx.delay_unload_0 = value;
	rl_ctl.cn78xx.delay_unload_1 = value;
	rl_ctl.cn78xx.delay_unload_2 = value;
	rl_ctl.cn78xx.delay_unload_3 = value;

	// use OR_DIS=1 to try for better results
	rl_ctl.cn78xx.or_dis = 1;

	/*
	 * If we will be switching to 32bit mode level based on only
	 * four bits because there are only 4 ECC bits.
	 */
	rl_ctl.cn78xx.bitmask = (if_64b) ? 0xFF : 0x0F;

	// allow overrides
	s = lookup_env(priv, "ddr_rlevel_ctl_or_dis");
	if (s)
		rl_ctl.cn78xx.or_dis = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_rlevel_ctl_bitmask");
	if (s)
		rl_ctl.cn78xx.bitmask = simple_strtoul(s, NULL, 0);

	rl_comp_offs = spd_rdimm
		? c_cfg->rlevel_comp_offset_rdimm
		: c_cfg->rlevel_comp_offset_udimm;
	s = lookup_env(priv, "ddr_rlevel_comp_offset");
	if (s)
		rl_comp_offs = strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_rlevel_offset");
	if (s)
		rl_ctl.cn78xx.offset   = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_rlevel_offset_en");
	if (s)
		rl_ctl.cn78xx.offset_en   = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_rlevel_ctl");
	if (s)
		rl_ctl.u64   = simple_strtoul(s, NULL, 0);

	lmc_wr(priv,
	       CVMX_LMCX_RLEVEL_CTL(if_num),
	       rl_ctl.u64);

	// do this here so we can look at final RLEVEL_CTL[offset] setting...
	s = lookup_env(priv, "ddr_enable_rldelay_bump");
	if (s) {
		// also use as mask bits
		enable_rldelay_bump = strtoul(s, NULL, 0);
	}

	if (enable_rldelay_bump != 0)
		rldelay_bump_incr = (rl_ctl.cn78xx.offset == 1) ? -1 : 1;

	s = lookup_env(priv, "ddr%d_rlevel_debug_loops", if_num);
	if (s)
		rl_dbg_loops = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_rtt_nom_auto");
	if (s)
		ddr_rtt_nom_auto = !!simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_rlevel_average");
	if (s)
		rl_samples = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_rlevel_compute");
	if (s)
		rl_compute = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_rlevel_printall");
	if (s)
		rl_print = simple_strtoul(s, NULL, 0);

	debug("RLEVEL_CTL                                    : 0x%016llx\n",
	      rl_ctl.u64);
	debug("RLEVEL_OFFSET                                 : %6d\n",
	      rl_ctl.cn78xx.offset);
	debug("RLEVEL_OFFSET_EN                              : %6d\n",
	      rl_ctl.cn78xx.offset_en);

	/*
	 * The purpose for the indexed table is to sort the settings
	 * by the ohm value to simplify the testing when incrementing
	 * through the settings.  (index => ohms) 1=120, 2=60, 3=40,
	 * 4=30, 5=20
	 */
	min_rtt_nom_idx = (c_cfg->min_rtt_nom_idx == 0) ?
		1 : c_cfg->min_rtt_nom_idx;
	max_rtt_nom_idx = (c_cfg->max_rtt_nom_idx == 0) ?
		5 : c_cfg->max_rtt_nom_idx;

	min_rodt_ctl = (c_cfg->min_rodt_ctl == 0) ? 1 : c_cfg->min_rodt_ctl;
	max_rodt_ctl = (c_cfg->max_rodt_ctl == 0) ? 5 : c_cfg->max_rodt_ctl;

	s = lookup_env(priv, "ddr_min_rodt_ctl");
	if (s)
		min_rodt_ctl = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_max_rodt_ctl");
	if (s)
		max_rodt_ctl = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_min_rtt_nom_idx");
	if (s)
		min_rtt_nom_idx = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_max_rtt_nom_idx");
	if (s)
		max_rtt_nom_idx = simple_strtoul(s, NULL, 0);

#ifdef ENABLE_HARDCODED_RLEVEL
	if (c_cfg->rl_tbl) {
		/* Check for hard-coded read-leveling settings */
		get_dimm_part_number(part_number, &dimm_config_table[0],
				     0, ddr_type);
		for (rankx = 0; rankx < dimm_count * 4; rankx++) {
			if (!(rank_mask & (1 << rankx)))
				continue;

			rl_rank.u64 = lmc_rd(priv,
					     CVMX_LMCX_RLEVEL_RANKX(rankx,
								    if_num));

			i = 0;
			while (c_cfg->rl_tbl[i].part) {
				debug("DIMM part number:\"%s\", SPD: \"%s\"\n",
				      c_cfg->rl_tbl[i].part, part_number);
				if ((strcmp(part_number,
					    c_cfg->rl_tbl[i].part) == 0) &&
				    (abs(c_cfg->rl_tbl[i].speed -
					 2 * ddr_hertz / (1000 * 1000)) < 10)) {
					debug("Using hard-coded read leveling for DIMM part number: \"%s\"\n",
					      part_number);
					rl_rank.u64 =
						c_cfg->rl_tbl[i].rl_rank[if_num][rankx];
					lmc_wr(priv,
					       CVMX_LMCX_RLEVEL_RANKX(rankx,
								      if_num),
					       rl_rank.u64);
					rl_rank.u64 =
						lmc_rd(priv,
						       CVMX_LMCX_RLEVEL_RANKX(rankx,
									      if_num));
					display_rl(if_num, rl_rank, rankx);
					/* Disable h/w read-leveling */
					rl_dbg_loops = 0;
					break;
				}
				++i;
			}
		}
	}
#endif /* ENABLE_HARDCODED_RLEVEL */

	max_adj_rl_del_inc = c_cfg->maximum_adjacent_rlevel_delay_increment;
	s = lookup_env(priv, "ddr_maximum_adjacent_rlevel_delay_increment");
	if (s)
		max_adj_rl_del_inc = strtoul(s, NULL, 0);

	while (rl_dbg_loops--) {
		union cvmx_lmcx_modereg_params1 mp1;
		union cvmx_lmcx_comp_ctl2 cc2;

		/* Initialize the error scoreboard */
		memset(rl_score, 0, sizeof(rl_score));

		cc2.u64 = lmc_rd(priv, CVMX_LMCX_COMP_CTL2(if_num));
		saved_ddr__ptune = cc2.cn78xx.ddr__ptune;
		saved_ddr__ntune = cc2.cn78xx.ddr__ntune;

		/* Disable dynamic compensation settings */
		if (rl_comp_offs != 0) {
			cc2.cn78xx.ptune = saved_ddr__ptune;
			cc2.cn78xx.ntune = saved_ddr__ntune;

			/*
			 * Round up the ptune calculation to bias the odd
			 * cases toward ptune
			 */
			cc2.cn78xx.ptune += divide_roundup(rl_comp_offs, 2);
			cc2.cn78xx.ntune -= rl_comp_offs / 2;

			ctl.u64 = lmc_rd(priv, CVMX_LMCX_CONTROL(if_num));
			saved_int_zqcs_dis = ctl.s.int_zqcs_dis;
			/* Disable ZQCS while in bypass. */
			ctl.s.int_zqcs_dis = 1;
			lmc_wr(priv, CVMX_LMCX_CONTROL(if_num), ctl.u64);

			cc2.cn78xx.byp = 1; /* Enable bypass mode */
			lmc_wr(priv, CVMX_LMCX_COMP_CTL2(if_num), cc2.u64);
			lmc_rd(priv, CVMX_LMCX_COMP_CTL2(if_num));
			/* Read again */
			cc2.u64 = lmc_rd(priv, CVMX_LMCX_COMP_CTL2(if_num));
			debug("DDR__PTUNE/DDR__NTUNE                         : %d/%d\n",
			      cc2.cn78xx.ddr__ptune, cc2.cn78xx.ddr__ntune);
		}

		mp1.u64 = lmc_rd(priv, CVMX_LMCX_MODEREG_PARAMS1(if_num));

		for (rtt_idx = min_rtt_nom_idx; rtt_idx <= max_rtt_nom_idx;
		     ++rtt_idx) {
			rtt_nom = imp_val->rtt_nom_table[rtt_idx];

			/*
			 * When the read ODT mask is zero the dyn_rtt_nom_mask
			 * is zero than RTT_NOM will not be changing during
			 * read-leveling.  Since the value is fixed we only need
			 * to test it once.
			 */
			if (dyn_rtt_nom_mask == 0) {
				// flag not to print NOM ohms
				print_nom_ohms = -1;
			} else {
				if (dyn_rtt_nom_mask & 1)
					mp1.s.rtt_nom_00 = rtt_nom;
				if (dyn_rtt_nom_mask & 2)
					mp1.s.rtt_nom_01 = rtt_nom;
				if (dyn_rtt_nom_mask & 4)
					mp1.s.rtt_nom_10 = rtt_nom;
				if (dyn_rtt_nom_mask & 8)
					mp1.s.rtt_nom_11 = rtt_nom;
				// FIXME? rank 0 ohms always?
				print_nom_ohms =
					imp_val->rtt_nom_ohms[mp1.s.rtt_nom_00];
			}

			lmc_wr(priv, CVMX_LMCX_MODEREG_PARAMS1(if_num),
			       mp1.u64);

			if (print_nom_ohms >= 0 && rl_print > 1) {
				debug("\n");
				debug("RTT_NOM     %3d, %3d, %3d, %3d ohms           :  %x,%x,%x,%x\n",
				      imp_val->rtt_nom_ohms[mp1.s.rtt_nom_11],
				      imp_val->rtt_nom_ohms[mp1.s.rtt_nom_10],
				      imp_val->rtt_nom_ohms[mp1.s.rtt_nom_01],
				      imp_val->rtt_nom_ohms[mp1.s.rtt_nom_00],
				      mp1.s.rtt_nom_11,
				      mp1.s.rtt_nom_10,
				      mp1.s.rtt_nom_01,
				      mp1.s.rtt_nom_00);
			}

			ddr_init_seq(priv, rank_mask, if_num);

			// Try RANK outside RODT to rearrange the output...
			for (rankx = 0; rankx < dimm_count * 4; rankx++) {
				if (!(rank_mask & (1 << rankx)))
					continue;

				for (rodt_ctl = max_rodt_ctl;
				     rodt_ctl >= min_rodt_ctl; --rodt_ctl)
					rodt_loop(priv, rankx, rl_score);
			}
		}

		/* Re-enable dynamic compensation settings. */
		if (rl_comp_offs != 0) {
			cc2.u64 = lmc_rd(priv, CVMX_LMCX_COMP_CTL2(if_num));

			cc2.cn78xx.ptune = 0;
			cc2.cn78xx.ntune = 0;
			cc2.cn78xx.byp = 0; /* Disable bypass mode */
			lmc_wr(priv, CVMX_LMCX_COMP_CTL2(if_num), cc2.u64);
			/* Read once */
			lmc_rd(priv, CVMX_LMCX_COMP_CTL2(if_num));

			/* Read again */
			cc2.u64 = lmc_rd(priv, CVMX_LMCX_COMP_CTL2(if_num));
			debug("DDR__PTUNE/DDR__NTUNE                         : %d/%d\n",
			      cc2.cn78xx.ddr__ptune, cc2.cn78xx.ddr__ntune);

			ctl.u64 = lmc_rd(priv, CVMX_LMCX_CONTROL(if_num));
			/* Restore original setting */
			ctl.s.int_zqcs_dis = saved_int_zqcs_dis;
			lmc_wr(priv, CVMX_LMCX_CONTROL(if_num), ctl.u64);
		}

		int override_compensation = 0;

		s = lookup_env(priv, "ddr__ptune");
		if (s)
			saved_ddr__ptune = strtoul(s, NULL, 0);

		s = lookup_env(priv, "ddr__ntune");
		if (s) {
			saved_ddr__ntune = strtoul(s, NULL, 0);
			override_compensation = 1;
		}

		if (override_compensation) {
			cc2.cn78xx.ptune = saved_ddr__ptune;
			cc2.cn78xx.ntune = saved_ddr__ntune;

			ctl.u64 = lmc_rd(priv, CVMX_LMCX_CONTROL(if_num));
			saved_int_zqcs_dis = ctl.s.int_zqcs_dis;
			/* Disable ZQCS while in bypass. */
			ctl.s.int_zqcs_dis = 1;
			lmc_wr(priv, CVMX_LMCX_CONTROL(if_num), ctl.u64);

			cc2.cn78xx.byp = 1; /* Enable bypass mode */
			lmc_wr(priv, CVMX_LMCX_COMP_CTL2(if_num), cc2.u64);
			/* Read again */
			cc2.u64 = lmc_rd(priv, CVMX_LMCX_COMP_CTL2(if_num));

			debug("DDR__PTUNE/DDR__NTUNE                         : %d/%d\n",
			      cc2.cn78xx.ptune, cc2.cn78xx.ntune);
		}

		/* Evaluation block */
		/* Still at initial value? */
		int best_rodt_score = DEFAULT_BEST_RANK_SCORE;
		int auto_rodt_ctl = 0;
		int auto_rtt_nom  = 0;
		int rodt_score;

		rodt_row_skip_mask = 0;

		// just add specific RODT rows to the skip mask for DDR4
		// at this time...
		if (ddr_type == DDR4_DRAM) {
			// skip RODT row 34 ohms for all DDR4 types
			rodt_row_skip_mask |= (1 << ddr4_rodt_ctl_34_ohm);
			// skip RODT row 40 ohms for all DDR4 types
			rodt_row_skip_mask |= (1 << ddr4_rodt_ctl_40_ohm);
			// For now, do not skip RODT row 40 or 48 ohm when
			// ddr_hertz is above 1075 MHz
			if (ddr_hertz > 1075000000) {
				// noskip RODT row 40 ohms
				rodt_row_skip_mask &=
					~(1 << ddr4_rodt_ctl_40_ohm);
				// noskip RODT row 48 ohms
				rodt_row_skip_mask &=
					~(1 << ddr4_rodt_ctl_48_ohm);
			}
			// For now, do not skip RODT row 48 ohm for 2Rx4
			// stacked die DIMMs
			if (is_stacked_die && num_ranks == 2 &&
			    dram_width == 4) {
				// noskip RODT row 48 ohms
				rodt_row_skip_mask &=
					~(1 << ddr4_rodt_ctl_48_ohm);
			}
			// for now, leave all rows eligible when we have
			// mini-DIMMs...
			if (spd_dimm_type == 5 || spd_dimm_type == 6)
				rodt_row_skip_mask = 0;
			// for now, leave all rows eligible when we have
			// a 2-slot 1-rank config
			if (dimm_count == 2 && num_ranks == 1)
				rodt_row_skip_mask = 0;

			debug("Evaluating Read-Leveling Scoreboard for AUTO settings.\n");
			for (rtt_idx = min_rtt_nom_idx;
			     rtt_idx <= max_rtt_nom_idx; ++rtt_idx) {
				rtt_nom = imp_val->rtt_nom_table[rtt_idx];

				for (rodt_ctl = max_rodt_ctl;
				     rodt_ctl >= min_rodt_ctl; --rodt_ctl) {
					rodt_score = 0;
					for (rankx = 0; rankx < dimm_count * 4;
					     rankx++) {
						if (!(rank_mask & (1 << rankx)))
							continue;

						debug("rl_score[rtt_nom=%d][rodt_ctl=%d][rankx=%d].score:%d\n",
						      rtt_nom, rodt_ctl, rankx,
						      rl_score[rtt_nom][rodt_ctl][rankx].score);
						rodt_score +=
							rl_score[rtt_nom][rodt_ctl][rankx].score;
					}
					// FIXME: do we need to skip RODT rows
					// here, like we do below in the
					// by-RANK settings?

					/*
					 * When using automatic ODT settings use
					 * the ODT settings associated with the
					 * best score for all of the tested ODT
					 * combinations.
					 */

					if (rodt_score < best_rodt_score ||
					    (rodt_score == best_rodt_score &&
					     (imp_val->rodt_ohms[rodt_ctl] >
					      imp_val->rodt_ohms[auto_rodt_ctl]))) {
						debug("AUTO: new best score for rodt:%d (%d), new score:%d, previous score:%d\n",
						      rodt_ctl,
						      imp_val->rodt_ohms[rodt_ctl],
						      rodt_score,
						      best_rodt_score);
						best_rodt_score = rodt_score;
						auto_rodt_ctl   = rodt_ctl;
						auto_rtt_nom    = rtt_nom;
					}
				}
			}

			mp1.u64 = lmc_rd(priv,
					 CVMX_LMCX_MODEREG_PARAMS1(if_num));

			if (ddr_rtt_nom_auto) {
				/* Store the automatically set RTT_NOM value */
				if (dyn_rtt_nom_mask & 1)
					mp1.s.rtt_nom_00 = auto_rtt_nom;
				if (dyn_rtt_nom_mask & 2)
					mp1.s.rtt_nom_01 = auto_rtt_nom;
				if (dyn_rtt_nom_mask & 4)
					mp1.s.rtt_nom_10 = auto_rtt_nom;
				if (dyn_rtt_nom_mask & 8)
					mp1.s.rtt_nom_11 = auto_rtt_nom;
			} else {
				/*
				 * restore the manual settings to the register
				 */
				mp1.s.rtt_nom_00 = default_rtt_nom[0];
				mp1.s.rtt_nom_01 = default_rtt_nom[1];
				mp1.s.rtt_nom_10 = default_rtt_nom[2];
				mp1.s.rtt_nom_11 = default_rtt_nom[3];
			}

			lmc_wr(priv, CVMX_LMCX_MODEREG_PARAMS1(if_num),
			       mp1.u64);
			debug("RTT_NOM     %3d, %3d, %3d, %3d ohms           :  %x,%x,%x,%x\n",
			      imp_val->rtt_nom_ohms[mp1.s.rtt_nom_11],
			      imp_val->rtt_nom_ohms[mp1.s.rtt_nom_10],
			      imp_val->rtt_nom_ohms[mp1.s.rtt_nom_01],
			      imp_val->rtt_nom_ohms[mp1.s.rtt_nom_00],
			      mp1.s.rtt_nom_11,
			      mp1.s.rtt_nom_10,
			      mp1.s.rtt_nom_01,
			      mp1.s.rtt_nom_00);

			debug("RTT_WR      %3d, %3d, %3d, %3d ohms           :  %x,%x,%x,%x\n",
			      imp_val->rtt_wr_ohms[extr_wr(mp1.u64, 3)],
			      imp_val->rtt_wr_ohms[extr_wr(mp1.u64, 2)],
			      imp_val->rtt_wr_ohms[extr_wr(mp1.u64, 1)],
			      imp_val->rtt_wr_ohms[extr_wr(mp1.u64, 0)],
			      extr_wr(mp1.u64, 3),
			      extr_wr(mp1.u64, 2),
			      extr_wr(mp1.u64, 1),
			      extr_wr(mp1.u64, 0));

			debug("DIC         %3d, %3d, %3d, %3d ohms           :  %x,%x,%x,%x\n",
			      imp_val->dic_ohms[mp1.s.dic_11],
			      imp_val->dic_ohms[mp1.s.dic_10],
			      imp_val->dic_ohms[mp1.s.dic_01],
			      imp_val->dic_ohms[mp1.s.dic_00],
			      mp1.s.dic_11,
			      mp1.s.dic_10,
			      mp1.s.dic_01,
			      mp1.s.dic_00);

			if (ddr_type == DDR4_DRAM) {
				union cvmx_lmcx_modereg_params2 mp2;
				/*
				 * We must read the CSR, and not depend on
				 * odt_config[odt_idx].odt_mask2, since we could
				 * have overridden values with envvars.
				 * NOTE: this corrects the printout, since the
				 * CSR is not written with the old values...
				 */
				mp2.u64 = lmc_rd(priv,
						 CVMX_LMCX_MODEREG_PARAMS2(if_num));

				debug("RTT_PARK    %3d, %3d, %3d, %3d ohms           :  %x,%x,%x,%x\n",
				      imp_val->rtt_nom_ohms[mp2.s.rtt_park_11],
				      imp_val->rtt_nom_ohms[mp2.s.rtt_park_10],
				      imp_val->rtt_nom_ohms[mp2.s.rtt_park_01],
				      imp_val->rtt_nom_ohms[mp2.s.rtt_park_00],
				      mp2.s.rtt_park_11,
				      mp2.s.rtt_park_10,
				      mp2.s.rtt_park_01,
				      mp2.s.rtt_park_00);

				debug("%-45s :  0x%x,0x%x,0x%x,0x%x\n",
				      "VREF_RANGE",
				      mp2.s.vref_range_11,
				      mp2.s.vref_range_10,
				      mp2.s.vref_range_01,
				      mp2.s.vref_range_00);

				debug("%-45s :  0x%x,0x%x,0x%x,0x%x\n",
				      "VREF_VALUE",
				      mp2.s.vref_value_11,
				      mp2.s.vref_value_10,
				      mp2.s.vref_value_01,
				      mp2.s.vref_value_00);
			}

			cc2.u64 = lmc_rd(priv, CVMX_LMCX_COMP_CTL2(if_num));
			if (ddr_rodt_ctl_auto) {
				cc2.cn78xx.rodt_ctl = auto_rodt_ctl;
			} else {
				// back to the original setting
				cc2.cn78xx.rodt_ctl = default_rodt_ctl;
			}
			lmc_wr(priv, CVMX_LMCX_COMP_CTL2(if_num), cc2.u64);
			cc2.u64 = lmc_rd(priv, CVMX_LMCX_COMP_CTL2(if_num));
			debug("Read ODT_CTL                                  : 0x%x (%d ohms)\n",
			      cc2.cn78xx.rodt_ctl,
			      imp_val->rodt_ohms[cc2.cn78xx.rodt_ctl]);

			/*
			 * Use the delays associated with the best score for
			 * each individual rank
			 */
			debug("Evaluating Read-Leveling Scoreboard for per-RANK settings.\n");

			// this is the the RANK MAJOR LOOP
			for (rankx = 0; rankx < dimm_count * 4; rankx++)
				rank_major_loop(priv, rankx, rl_score);
		}  /* Evaluation block */
	} /* while(rl_dbg_loops--) */

	ctl.cn78xx.ddr2t = save_ddr2t;
	lmc_wr(priv, CVMX_LMCX_CONTROL(if_num), ctl.u64);
	ctl.u64 = lmc_rd(priv, CVMX_LMCX_CONTROL(if_num));
	/* Display final 2T value */
	debug("DDR2T                                         : %6d\n",
	      ctl.cn78xx.ddr2t);

	ddr_init_seq(priv, rank_mask, if_num);

	for (rankx = 0; rankx < dimm_count * 4; rankx++) {
		u64 value;
		int parameter_set = 0;

		if (!(rank_mask & (1 << rankx)))
			continue;

		rl_rank.u64 = lmc_rd(priv, CVMX_LMCX_RLEVEL_RANKX(rankx,
								  if_num));

		for (i = 0; i < 9; ++i) {
			s = lookup_env(priv, "ddr%d_rlevel_rank%d_byte%d",
				       if_num, rankx, i);
			if (s) {
				parameter_set |= 1;
				value = simple_strtoul(s, NULL, 0);

				upd_rl_rank(&rl_rank, i, value);
			}
		}

		s = lookup_env_ull(priv, "ddr%d_rlevel_rank%d", if_num, rankx);
		if (s) {
			parameter_set |= 1;
			value = simple_strtoull(s, NULL, 0);
			rl_rank.u64 = value;
		}

		if (parameter_set) {
			lmc_wr(priv,
			       CVMX_LMCX_RLEVEL_RANKX(rankx, if_num),
			       rl_rank.u64);
			rl_rank.u64 = lmc_rd(priv,
					     CVMX_LMCX_RLEVEL_RANKX(rankx,
								    if_num));
			display_rl(if_num, rl_rank, rankx);
		}
	}
}

int init_octeon3_ddr3_interface(struct ddr_priv *priv,
				struct ddr_conf *_ddr_conf, u32 _ddr_hertz,
				u32 cpu_hertz, u32 ddr_ref_hertz, int _if_num,
				u32 _if_mask)
{
	union cvmx_lmcx_control ctrl;
	int ret;
	char *s;
	int i;

	if_num = _if_num;
	ddr_hertz = _ddr_hertz;
	ddr_conf = _ddr_conf;
	if_mask = _if_mask;
	odt_1rank_config = ddr_conf->odt_1rank_config;
	odt_2rank_config = ddr_conf->odt_2rank_config;
	odt_4rank_config = ddr_conf->odt_4rank_config;
	dimm_config_table = ddr_conf->dimm_config_table;
	c_cfg = &ddr_conf->custom_lmc_config;

	/*
	 * Compute clock rates to the nearest picosecond.
	 */
	tclk_psecs = hertz_to_psecs(ddr_hertz);	/* Clock in psecs */
	eclk_psecs = hertz_to_psecs(cpu_hertz);	/* Clock in psecs */

	dimm_count = 0;
	/* Accumulate and report all the errors before giving up */
	fatal_error = 0;

	/* Flag that indicates safe DDR settings should be used */
	safe_ddr_flag = 0;
	if_64b = 1;		/* Octeon II Default: 64bit interface width */
	mem_size_mbytes = 0;
	bank_bits = 0;
	column_bits_start = 1;
	use_ecc = 1;
	min_cas_latency = 0, max_cas_latency = 0, override_cas_latency = 0;
	spd_package = 0;
	spd_rawcard = 0;
	spd_rawcard_aorb = 0;
	spd_rdimm_registers = 0;
	is_stacked_die = 0;
	is_3ds_dimm = 0;	// 3DS
	lranks_per_prank = 1;	// 3DS: logical ranks per package rank
	lranks_bits = 0;	// 3DS: logical ranks bits
	die_capacity = 0;	// in Mbits; only used for 3DS

	wl_mask_err = 0;
	dyn_rtt_nom_mask = 0;
	ddr_disable_chip_reset = 1;
	match_wl_rtt_nom = 0;

	internal_retries = 0;

	disable_deskew_training = 0;
	restart_if_dsk_incomplete = 0;
	last_lane = ((if_64b) ? 8 : 4) + use_ecc;

	disable_sequential_delay_check = 0;
	wl_print = WLEVEL_PRINTALL_DEFAULT;

	enable_by_rank_init = 1;	// FIXME: default by-rank ON
	saved_rank_mask = 0;

	node = 0;

	memset(hwl_alts, 0, sizeof(hwl_alts));

	/*
	 * Initialize these to shut up the compiler. They are configured
	 * and used only for DDR4
	 */
	ddr4_trrd_lmin = 6000;
	ddr4_tccd_lmin = 6000;

	debug("\nInitializing node %d DDR interface %d, DDR Clock %d, DDR Reference Clock %d, CPUID 0x%08x\n",
	      node, if_num, ddr_hertz, ddr_ref_hertz, read_c0_prid());

	if (dimm_config_table[0].spd_addrs[0] == 0 &&
	    !dimm_config_table[0].spd_ptrs[0]) {
		printf("ERROR: No dimms specified in the dimm_config_table.\n");
		return -1;
	}

	// allow some overrides to be done

	// this one controls several things related to DIMM geometry: HWL and RL
	disable_sequential_delay_check = c_cfg->disable_sequential_delay_check;
	s = lookup_env(priv, "ddr_disable_sequential_delay_check");
	if (s)
		disable_sequential_delay_check = strtoul(s, NULL, 0);

	// this one controls whether chip RESET is done, or LMC init restarted
	// from step 6.9.6
	s = lookup_env(priv, "ddr_disable_chip_reset");
	if (s)
		ddr_disable_chip_reset = !!strtoul(s, NULL, 0);

	// this one controls whether Deskew Training is performed
	s = lookup_env(priv, "ddr_disable_deskew_training");
	if (s)
		disable_deskew_training = !!strtoul(s, NULL, 0);

	if (ddr_verbose(priv)) {
		printf("DDR SPD Table:");
		for (didx = 0; didx < DDR_CFG_T_MAX_DIMMS; ++didx) {
			if (dimm_config_table[didx].spd_addrs[0] == 0)
				break;

			printf(" --ddr%dspd=0x%02x", if_num,
			       dimm_config_table[didx].spd_addrs[0]);
			if (dimm_config_table[didx].spd_addrs[1] != 0)
				printf(",0x%02x",
				       dimm_config_table[didx].spd_addrs[1]);
		}
		printf("\n");
	}

	/*
	 * Walk the DRAM Socket Configuration Table to see what is installed.
	 */
	for (didx = 0; didx < DDR_CFG_T_MAX_DIMMS; ++didx) {
		/* Check for lower DIMM socket populated */
		if (validate_dimm(priv, &dimm_config_table[didx], 0)) {
			if (ddr_verbose(priv))
				report_dimm(&dimm_config_table[didx], 0,
					    dimm_count, if_num);
			++dimm_count;
		} else {
			break;
		}		/* Finished when there is no lower DIMM */
	}

	initialize_ddr_clock(priv, ddr_conf, cpu_hertz, ddr_hertz,
			     ddr_ref_hertz, if_num, if_mask);

	if (!odt_1rank_config)
		odt_1rank_config = disable_odt_config;
	if (!odt_2rank_config)
		odt_2rank_config = disable_odt_config;
	if (!odt_4rank_config)
		odt_4rank_config = disable_odt_config;

	s = env_get("ddr_safe");
	if (s) {
		safe_ddr_flag = !!simple_strtoul(s, NULL, 0);
		printf("Parameter found in environment. ddr_safe = %d\n",
		       safe_ddr_flag);
	}

	if (dimm_count == 0) {
		printf("ERROR: DIMM 0 not detected.\n");
		return (-1);
	}

	if (c_cfg->mode32b)
		if_64b = 0;

	s = lookup_env(priv, "if_64b");
	if (s)
		if_64b = !!simple_strtoul(s, NULL, 0);

	if (if_64b == 1) {
		if (octeon_is_cpuid(OCTEON_CN70XX)) {
			printf("64-bit interface width is not supported for this Octeon model\n");
			++fatal_error;
		}
	}

	/* ddr_type only indicates DDR4 or DDR3 */
	ddr_type = (read_spd(&dimm_config_table[0], 0,
			     DDR4_SPD_KEY_BYTE_DEVICE_TYPE) == 0x0C) ? 4 : 3;
	debug("DRAM Device Type: DDR%d\n", ddr_type);

	if (ddr_type == DDR4_DRAM) {
		int spd_module_type;
		int asymmetric;
		const char *signal_load[4] = { "", "MLS", "3DS", "RSV" };

		imp_val = &ddr4_impedence_val;

		spd_addr =
		    read_spd(&dimm_config_table[0], 0,
			     DDR4_SPD_ADDRESSING_ROW_COL_BITS);
		spd_org =
		    read_spd(&dimm_config_table[0], 0,
			     DDR4_SPD_MODULE_ORGANIZATION);
		spd_banks =
		    0xFF & read_spd(&dimm_config_table[0], 0,
				    DDR4_SPD_DENSITY_BANKS);

		bank_bits =
		    (2 + ((spd_banks >> 4) & 0x3)) + ((spd_banks >> 6) & 0x3);
		/* Controller can only address 4 bits. */
		bank_bits = min((int)bank_bits, 4);

		spd_package =
		    0XFF & read_spd(&dimm_config_table[0], 0,
				    DDR4_SPD_PACKAGE_TYPE);
		if (spd_package & 0x80) {	// non-monolithic device
			is_stacked_die = ((spd_package & 0x73) == 0x11);
			debug("DDR4: Package Type 0x%02x (%s), %d die\n",
			      spd_package, signal_load[(spd_package & 3)],
			      ((spd_package >> 4) & 7) + 1);
			is_3ds_dimm = ((spd_package & 3) == 2);	// is it 3DS?
			if (is_3ds_dimm) {	// is it 3DS?
				lranks_per_prank = ((spd_package >> 4) & 7) + 1;
				// FIXME: should make sure it is only 2H or 4H
				// or 8H?
				lranks_bits = lranks_per_prank >> 1;
				if (lranks_bits == 4)
					lranks_bits = 3;
			}
		} else if (spd_package != 0) {
			// FIXME: print non-zero monolithic device definition
			debug("DDR4: Package Type MONOLITHIC: %d die, signal load %d\n",
			      ((spd_package >> 4) & 7) + 1, (spd_package & 3));
		}

		asymmetric = (spd_org >> 6) & 1;
		if (asymmetric) {
			int spd_secondary_pkg =
			    read_spd(&dimm_config_table[0], 0,
				     DDR4_SPD_SECONDARY_PACKAGE_TYPE);
			debug("DDR4: Module Organization: ASYMMETRICAL: Secondary Package Type 0x%02x\n",
			      spd_secondary_pkg);
		} else {
			u64 bus_width =
				8 << (0x07 &
				read_spd(&dimm_config_table[0], 0,
					 DDR4_SPD_MODULE_MEMORY_BUS_WIDTH));
			u64 ddr_width = 4 << ((spd_org >> 0) & 0x7);
			u64 module_cap;
			int shift = (spd_banks & 0x0F);

			die_capacity = (shift < 8) ? (256UL << shift) :
				((12UL << (shift & 1)) << 10);
			debug("DDR4: Module Organization: SYMMETRICAL: capacity per die %d %cbit\n",
			      (die_capacity > 512) ? (die_capacity >> 10) :
			      die_capacity, (die_capacity > 512) ? 'G' : 'M');
			module_cap = ((u64)die_capacity << 20) / 8UL *
				bus_width / ddr_width *
				(1UL + ((spd_org >> 3) & 0x7));

			// is it 3DS?
			if (is_3ds_dimm) {
				module_cap *= (u64)(((spd_package >> 4) & 7) +
						    1);
			}
			debug("DDR4: Module Organization: SYMMETRICAL: capacity per module %lld GB\n",
			      module_cap >> 30);
		}

		spd_rawcard =
		    0xFF & read_spd(&dimm_config_table[0], 0,
				    DDR4_SPD_REFERENCE_RAW_CARD);
		debug("DDR4: Reference Raw Card 0x%02x\n", spd_rawcard);

		spd_module_type =
		    read_spd(&dimm_config_table[0], 0,
			     DDR4_SPD_KEY_BYTE_MODULE_TYPE);
		if (spd_module_type & 0x80) {	// HYBRID module
			debug("DDR4: HYBRID module, type %s\n",
			      ((spd_module_type & 0x70) ==
			       0x10) ? "NVDIMM" : "UNKNOWN");
		}
		spd_thermal_sensor =
		    read_spd(&dimm_config_table[0], 0,
			     DDR4_SPD_MODULE_THERMAL_SENSOR);
		spd_dimm_type = spd_module_type & 0x0F;
		spd_rdimm = (spd_dimm_type == 1) || (spd_dimm_type == 5) ||
			(spd_dimm_type == 8);
		if (spd_rdimm) {
			u16 spd_mfgr_id, spd_register_rev, spd_mod_attr;
			static const u16 manu_ids[4] = {
				0xb380, 0x3286, 0x9780, 0xb304
			};
			static const char *manu_names[4] = {
				"XXX", "XXXXXXX", "XX", "XXXXX"
			};
			int mc;

			spd_mfgr_id =
			    (0xFFU &
			     read_spd(&dimm_config_table[0], 0,
				      DDR4_SPD_REGISTER_MANUFACTURER_ID_LSB)) |
			    ((0xFFU &
			      read_spd(&dimm_config_table[0], 0,
				       DDR4_SPD_REGISTER_MANUFACTURER_ID_MSB))
			     << 8);
			spd_register_rev =
			    0xFFU & read_spd(&dimm_config_table[0], 0,
					     DDR4_SPD_REGISTER_REVISION_NUMBER);
			for (mc = 0; mc < 4; mc++)
				if (manu_ids[mc] == spd_mfgr_id)
					break;

			debug("DDR4: RDIMM Register Manufacturer ID: %s, Revision: 0x%02x\n",
			      (mc >= 4) ? "UNKNOWN" : manu_names[mc],
			      spd_register_rev);

			// RAWCARD A or B must be bit 7=0 and bits 4-0
			// either 00000(A) or 00001(B)
			spd_rawcard_aorb = ((spd_rawcard & 0x9fUL) <= 1);
			// RDIMM Module Attributes
			spd_mod_attr =
			    0xFFU & read_spd(&dimm_config_table[0], 0,
					DDR4_SPD_UDIMM_ADDR_MAPPING_FROM_EDGE);
			spd_rdimm_registers = ((1 << (spd_mod_attr & 3)) >> 1);
			debug("DDR4: RDIMM Module Attributes (0x%02x): Register Type DDR4RCD%02d, DRAM rows %d, Registers %d\n",
			      spd_mod_attr, (spd_mod_attr >> 4) + 1,
			      ((1 << ((spd_mod_attr >> 2) & 3)) >> 1),
			      spd_rdimm_registers);
		}
		dimm_type_name = ddr4_dimm_types[spd_dimm_type];
	} else {		/* if (ddr_type == DDR4_DRAM) */
		const char *signal_load[4] = { "UNK", "MLS", "SLS", "RSV" };

		imp_val = &ddr3_impedence_val;

		spd_addr =
		    read_spd(&dimm_config_table[0], 0,
			     DDR3_SPD_ADDRESSING_ROW_COL_BITS);
		spd_org =
		    read_spd(&dimm_config_table[0], 0,
			     DDR3_SPD_MODULE_ORGANIZATION);
		spd_banks =
		    read_spd(&dimm_config_table[0], 0,
			     DDR3_SPD_DENSITY_BANKS) & 0xff;

		bank_bits = 3 + ((spd_banks >> 4) & 0x7);
		/* Controller can only address 3 bits. */
		bank_bits = min((int)bank_bits, 3);
		spd_dimm_type =
		    0x0f & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_KEY_BYTE_MODULE_TYPE);
		spd_rdimm = (spd_dimm_type == 1) || (spd_dimm_type == 5) ||
			(spd_dimm_type == 9);

		spd_package =
		    0xFF & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_SDRAM_DEVICE_TYPE);
		if (spd_package & 0x80) {	// non-standard device
			debug("DDR3: Device Type 0x%02x (%s), %d die\n",
			      spd_package, signal_load[(spd_package & 3)],
			      ((1 << ((spd_package >> 4) & 7)) >> 1));
		} else if (spd_package != 0) {
			// FIXME: print non-zero monolithic device definition
			debug("DDR3: Device Type MONOLITHIC: %d die, signal load %d\n",
			      ((1 << (spd_package >> 4) & 7) >> 1),
			      (spd_package & 3));
		}

		spd_rawcard =
		    0xFF & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_REFERENCE_RAW_CARD);
		debug("DDR3: Reference Raw Card 0x%02x\n", spd_rawcard);
		spd_thermal_sensor =
		    read_spd(&dimm_config_table[0], 0,
			     DDR3_SPD_MODULE_THERMAL_SENSOR);

		if (spd_rdimm) {
			int spd_mfgr_id, spd_register_rev, spd_mod_attr;

			spd_mfgr_id =
			    (0xFFU &
			     read_spd(&dimm_config_table[0], 0,
				      DDR3_SPD_REGISTER_MANUFACTURER_ID_LSB)) |
			    ((0xFFU &
			      read_spd(&dimm_config_table[0], 0,
				       DDR3_SPD_REGISTER_MANUFACTURER_ID_MSB))
			     << 8);
			spd_register_rev =
			    0xFFU & read_spd(&dimm_config_table[0], 0,
					     DDR3_SPD_REGISTER_REVISION_NUMBER);
			debug("DDR3: RDIMM Register Manufacturer ID 0x%x Revision 0x%02x\n",
			      spd_mfgr_id, spd_register_rev);
			// Module Attributes
			spd_mod_attr =
			    0xFFU & read_spd(&dimm_config_table[0], 0,
					     DDR3_SPD_ADDRESS_MAPPING);
			spd_rdimm_registers = ((1 << (spd_mod_attr & 3)) >> 1);
			debug("DDR3: RDIMM Module Attributes (0x%02x): DRAM rows %d, Registers %d\n",
			      spd_mod_attr,
			      ((1 << ((spd_mod_attr >> 2) & 3)) >> 1),
			      spd_rdimm_registers);
		}
		dimm_type_name = ddr3_dimm_types[spd_dimm_type];
	}

	if (spd_thermal_sensor & 0x80) {
		debug("DDR%d: SPD: Thermal Sensor PRESENT\n",
		      (ddr_type == DDR4_DRAM) ? 4 : 3);
	}

	debug("spd_addr        : %#06x\n", spd_addr);
	debug("spd_org         : %#06x\n", spd_org);
	debug("spd_banks       : %#06x\n", spd_banks);

	row_bits = 12 + ((spd_addr >> 3) & 0x7);
	col_bits = 9 + ((spd_addr >> 0) & 0x7);

	num_ranks = 1 + ((spd_org >> 3) & 0x7);
	dram_width = 4 << ((spd_org >> 0) & 0x7);
	num_banks = 1 << bank_bits;

	s = lookup_env(priv, "ddr_num_ranks");
	if (s)
		num_ranks = simple_strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_enable_by_rank_init");
	if (s)
		enable_by_rank_init = !!simple_strtoul(s, NULL, 0);

	// FIXME: for now, we can only handle a DDR4 2rank-1slot config
	// FIXME: also, by-rank init does not work correctly if 32-bit mode...
	if (enable_by_rank_init && (ddr_type != DDR4_DRAM ||
				    dimm_count != 1 || if_64b != 1 ||
				    num_ranks != 2))
		enable_by_rank_init = 0;

	if (enable_by_rank_init) {
		struct dimm_odt_config *odt_config;
		union cvmx_lmcx_modereg_params1 mp1;
		union cvmx_lmcx_modereg_params2 modereg_params2;
		int by_rank_rodt, by_rank_wr, by_rank_park;

		// Do ODT settings changes which work best for 2R-1S configs
		debug("DDR4: 2R-1S special BY-RANK init ODT settings updated\n");

		// setup for modifying config table values - 2 ranks and 1 DIMM
		odt_config =
		    (struct dimm_odt_config *)&ddr_conf->odt_2rank_config[0];

		// original was 80, first try was 60
		by_rank_rodt = ddr4_rodt_ctl_48_ohm;
		s = lookup_env(priv, "ddr_by_rank_rodt");
		if (s)
			by_rank_rodt = strtoul(s, NULL, 0);

		odt_config->qs_dic = /*RODT_CTL */ by_rank_rodt;

		// this is for MODEREG_PARAMS1 fields
		// fetch the original settings
		mp1.u64 = odt_config->modereg_params1.u64;

		by_rank_wr = ddr4_rttwr_80ohm;	// originals were 240
		s = lookup_env(priv, "ddr_by_rank_wr");
		if (s)
			by_rank_wr = simple_strtoul(s, NULL, 0);

		// change specific settings here...
		insrt_wr(&mp1.u64, /*rank */ 00, by_rank_wr);
		insrt_wr(&mp1.u64, /*rank */ 01, by_rank_wr);

		// save final settings
		odt_config->modereg_params1.u64 = mp1.u64;

		// this is for MODEREG_PARAMS2 fields
		// fetch the original settings
		modereg_params2.u64 = odt_config->modereg_params2.u64;

		by_rank_park = ddr4_rttpark_none;	// originals were 120
		s = lookup_env(priv, "ddr_by_rank_park");
		if (s)
			by_rank_park = simple_strtoul(s, NULL, 0);

		// change specific settings here...
		modereg_params2.s.rtt_park_00 = by_rank_park;
		modereg_params2.s.rtt_park_01 = by_rank_park;

		// save final settings
		odt_config->modereg_params2.u64 = modereg_params2.u64;
	}

	/*
	 * FIX
	 * Check that values are within some theoretical limits.
	 * col_bits(min) = row_lsb(min) - bank_bits(max) - bus_bits(max) =
	 *   14 - 3 - 4 = 7
	 * col_bits(max) = row_lsb(max) - bank_bits(min) - bus_bits(min) =
	 *   18 - 2 - 3 = 13
	 */
	if (col_bits > 13 || col_bits < 7) {
		printf("Unsupported number of Col Bits: %d\n", col_bits);
		++fatal_error;
	}

	/*
	 * FIX
	 * Check that values are within some theoretical limits.
	 * row_bits(min) = pbank_lsb(min) - row_lsb(max) - rank_bits =
	 *   26 - 18 - 1 = 7
	 * row_bits(max) = pbank_lsb(max) - row_lsb(min) - rank_bits =
	 *   33 - 14 - 1 = 18
	 */
	if (row_bits > 18 || row_bits < 7) {
		printf("Unsupported number of Row Bits: %d\n", row_bits);
		++fatal_error;
	}

	s = lookup_env(priv, "ddr_rdimm_ena");
	if (s)
		spd_rdimm = !!simple_strtoul(s, NULL, 0);

	wl_loops = WLEVEL_LOOPS_DEFAULT;
	// accept generic or interface-specific override
	s = lookup_env(priv, "ddr_wlevel_loops");
	if (!s)
		s = lookup_env(priv, "ddr%d_wlevel_loops", if_num);

	if (s)
		wl_loops = strtoul(s, NULL, 0);

	s = lookup_env(priv, "ddr_ranks");
	if (s)
		num_ranks = simple_strtoul(s, NULL, 0);

	bunk_enable = (num_ranks > 1);

	if (octeon_is_cpuid(OCTEON_CN7XXX))
		column_bits_start = 3;
	else
		printf("ERROR: Unsupported Octeon model: 0x%x\n",
		       read_c0_prid());

	row_lsb = column_bits_start + col_bits + bank_bits - (!if_64b);
	debug("row_lsb = column_bits_start + col_bits + bank_bits = %d\n",
	      row_lsb);

	pbank_lsb = row_lsb + row_bits + bunk_enable;
	debug("pbank_lsb = row_lsb + row_bits + bunk_enable = %d\n", pbank_lsb);

	if (lranks_per_prank > 1) {
		pbank_lsb = row_lsb + row_bits + lranks_bits + bunk_enable;
		debug("DDR4: 3DS: pbank_lsb = (%d row_lsb) + (%d row_bits) + (%d lranks_bits) + (%d bunk_enable) = %d\n",
		      row_lsb, row_bits, lranks_bits, bunk_enable, pbank_lsb);
	}

	mem_size_mbytes = dimm_count * ((1ull << pbank_lsb) >> 20);
	if (num_ranks == 4) {
		/*
		 * Quad rank dimm capacity is equivalent to two dual-rank
		 * dimms.
		 */
		mem_size_mbytes *= 2;
	}

	/*
	 * Mask with 1 bits set for for each active rank, allowing 2 bits
	 * per dimm. This makes later calculations simpler, as a variety
	 * of CSRs use this layout. This init needs to be updated for dual
	 * configs (ie non-identical DIMMs).
	 *
	 * Bit 0 = dimm0, rank 0
	 * Bit 1 = dimm0, rank 1
	 * Bit 2 = dimm1, rank 0
	 * Bit 3 = dimm1, rank 1
	 * ...
	 */
	rank_mask = 0x1;
	if (num_ranks > 1)
		rank_mask = 0x3;
	if (num_ranks > 2)
		rank_mask = 0xf;

	for (i = 1; i < dimm_count; i++)
		rank_mask |= ((rank_mask & 0x3) << (2 * i));

	/*
	 * If we are booting from RAM, the DRAM controller is
	 * already set up.  Just return the memory size
	 */
	if (priv->flags & FLAG_RAM_RESIDENT) {
		debug("Ram Boot: Skipping LMC config\n");
		return mem_size_mbytes;
	}

	if (ddr_type == DDR4_DRAM) {
		spd_ecc =
		    !!(read_spd
		       (&dimm_config_table[0], 0,
			DDR4_SPD_MODULE_MEMORY_BUS_WIDTH) & 8);
	} else {
		spd_ecc =
		    !!(read_spd
		       (&dimm_config_table[0], 0,
			DDR3_SPD_MEMORY_BUS_WIDTH) & 8);
	}

	char rank_spec[8];

	printable_rank_spec(rank_spec, num_ranks, dram_width, spd_package);
	debug("Summary: %d %s%s %s %s, row bits=%d, col bits=%d, bank bits=%d\n",
	      dimm_count, dimm_type_name, (dimm_count > 1) ? "s" : "",
	      rank_spec,
	      (spd_ecc) ? "ECC" : "non-ECC", row_bits, col_bits, bank_bits);

	if (ddr_type == DDR4_DRAM) {
		spd_cas_latency =
		    ((0xff &
		      read_spd(&dimm_config_table[0], 0,
			       DDR4_SPD_CAS_LATENCIES_BYTE0)) << 0);
		spd_cas_latency |=
		    ((0xff &
		      read_spd(&dimm_config_table[0], 0,
			       DDR4_SPD_CAS_LATENCIES_BYTE1)) << 8);
		spd_cas_latency |=
		    ((0xff &
		      read_spd(&dimm_config_table[0], 0,
			       DDR4_SPD_CAS_LATENCIES_BYTE2)) << 16);
		spd_cas_latency |=
		    ((0xff &
		      read_spd(&dimm_config_table[0], 0,
			       DDR4_SPD_CAS_LATENCIES_BYTE3)) << 24);
	} else {
		spd_cas_latency =
		    0xff & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_CAS_LATENCIES_LSB);
		spd_cas_latency |=
		    ((0xff &
		      read_spd(&dimm_config_table[0], 0,
			       DDR3_SPD_CAS_LATENCIES_MSB)) << 8);
	}
	debug("spd_cas_latency : %#06x\n", spd_cas_latency);

	if (ddr_type == DDR4_DRAM) {
		/*
		 * No other values for DDR4 MTB and FTB are specified at the
		 * current time so don't bother reading them. Can't speculate
		 * how new values will be represented.
		 */
		int spdmtb = 125;
		int spdftb = 1;

		taamin = spdmtb * read_spd(&dimm_config_table[0], 0,
					   DDR4_SPD_MIN_CAS_LATENCY_TAAMIN) +
			 spdftb * (signed char)read_spd(&dimm_config_table[0],
			 0, DDR4_SPD_MIN_CAS_LATENCY_FINE_TAAMIN);

		ddr4_tckavgmin = spdmtb * read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MINIMUM_CYCLE_TIME_TCKAVGMIN) +
			spdftb * (signed char)read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_CYCLE_TIME_FINE_TCKAVGMIN);

		ddr4_tckavgmax = spdmtb * read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MAXIMUM_CYCLE_TIME_TCKAVGMAX) +
			spdftb * (signed char)read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MAX_CYCLE_TIME_FINE_TCKAVGMAX);

		ddr4_trdcmin = spdmtb * read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_RAS_CAS_DELAY_TRCDMIN) +
			spdftb * (signed char)read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_RAS_TO_CAS_DELAY_FINE_TRCDMIN);

		ddr4_trpmin = spdmtb * read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_ROW_PRECHARGE_DELAY_TRPMIN) +
			spdftb * (signed char)read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_ROW_PRECHARGE_DELAY_FINE_TRPMIN);

		ddr4_trasmin = spdmtb *
			(((read_spd
			   (&dimm_config_table[0], 0,
			    DDR4_SPD_UPPER_NIBBLES_TRAS_TRC) & 0xf) << 8) +
			 (read_spd
			  (&dimm_config_table[0], 0,
			   DDR4_SPD_MIN_ACTIVE_PRECHARGE_LSB_TRASMIN) & 0xff));

		ddr4_trcmin = spdmtb *
			((((read_spd
			    (&dimm_config_table[0], 0,
			     DDR4_SPD_UPPER_NIBBLES_TRAS_TRC) >> 4) & 0xf) <<
			  8) + (read_spd
				(&dimm_config_table[0], 0,
				 DDR4_SPD_MIN_ACTIVE_REFRESH_LSB_TRCMIN) &
				0xff))
			+ spdftb * (signed char)read_spd(&dimm_config_table[0],
							 0,
			DDR4_SPD_MIN_ACT_TO_ACT_REFRESH_DELAY_FINE_TRCMIN);

		ddr4_trfc1min = spdmtb * (((read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_REFRESH_RECOVERY_MSB_TRFC1MIN) & 0xff) <<
			8) + (read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_REFRESH_RECOVERY_LSB_TRFC1MIN) & 0xff));

		ddr4_trfc2min = spdmtb * (((read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_REFRESH_RECOVERY_MSB_TRFC2MIN) & 0xff) <<
			8) + (read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_REFRESH_RECOVERY_LSB_TRFC2MIN) & 0xff));

		ddr4_trfc4min = spdmtb * (((read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_REFRESH_RECOVERY_MSB_TRFC4MIN) & 0xff) <<
			8) + (read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_REFRESH_RECOVERY_LSB_TRFC4MIN) & 0xff));

		ddr4_tfawmin = spdmtb * (((read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_FOUR_ACTIVE_WINDOW_MSN_TFAWMIN) & 0xf) <<
			8) + (read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_FOUR_ACTIVE_WINDOW_LSB_TFAWMIN) & 0xff));

		ddr4_trrd_smin = spdmtb * read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_ROW_ACTIVE_DELAY_SAME_TRRD_SMIN) +
			spdftb * (signed char)read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_ACT_TO_ACT_DELAY_DIFF_FINE_TRRD_SMIN);

		ddr4_trrd_lmin = spdmtb * read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_ROW_ACTIVE_DELAY_DIFF_TRRD_LMIN) +
			spdftb * (signed char)read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_ACT_TO_ACT_DELAY_SAME_FINE_TRRD_LMIN);

		ddr4_tccd_lmin = spdmtb * read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_CAS_TO_CAS_DELAY_TCCD_LMIN) +
			spdftb * (signed char)read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_MIN_CAS_TO_CAS_DELAY_FINE_TCCD_LMIN);

		debug("%-45s : %6d ps\n", "Medium Timebase (MTB)", spdmtb);
		debug("%-45s : %6d ps\n", "Fine Timebase   (FTB)", spdftb);

		debug("%-45s : %6d ps (%ld MT/s)\n",
		      "SDRAM Minimum Cycle Time (tCKAVGmin)", ddr4_tckavgmin,
		      pretty_psecs_to_mts(ddr4_tckavgmin));
		debug("%-45s : %6d ps\n",
		      "SDRAM Maximum Cycle Time (tCKAVGmax)", ddr4_tckavgmax);
		debug("%-45s : %6d ps\n", "Minimum CAS Latency Time (taamin)",
		      taamin);
		debug("%-45s : %6d ps\n",
		      "Minimum RAS to CAS Delay Time (tRCDmin)", ddr4_trdcmin);
		debug("%-45s : %6d ps\n",
		      "Minimum Row Precharge Delay Time (tRPmin)", ddr4_trpmin);
		debug("%-45s : %6d ps\n",
		      "Minimum Active to Precharge Delay (tRASmin)",
		      ddr4_trasmin);
		debug("%-45s : %6d ps\n",
		      "Minimum Active to Active/Refr. Delay (tRCmin)",
		      ddr4_trcmin);
		debug("%-45s : %6d ps\n",
		      "Minimum Refresh Recovery Delay (tRFC1min)",
		      ddr4_trfc1min);
		debug("%-45s : %6d ps\n",
		      "Minimum Refresh Recovery Delay (tRFC2min)",
		      ddr4_trfc2min);
		debug("%-45s : %6d ps\n",
		      "Minimum Refresh Recovery Delay (tRFC4min)",
		      ddr4_trfc4min);
		debug("%-45s : %6d ps\n",
		      "Minimum Four Activate Window Time (tFAWmin)",
		      ddr4_tfawmin);
		debug("%-45s : %6d ps\n",
		      "Minimum Act. to Act. Delay (tRRD_Smin)", ddr4_trrd_smin);
		debug("%-45s : %6d ps\n",
		      "Minimum Act. to Act. Delay (tRRD_Lmin)", ddr4_trrd_lmin);
		debug("%-45s : %6d ps\n",
		      "Minimum CAS to CAS Delay Time (tCCD_Lmin)",
		      ddr4_tccd_lmin);

#define DDR4_TWR 15000
#define DDR4_TWTR_S 2500

		tckmin = ddr4_tckavgmin;
		twr = DDR4_TWR;
		trcd = ddr4_trdcmin;
		trrd = ddr4_trrd_smin;
		trp = ddr4_trpmin;
		tras = ddr4_trasmin;
		trc = ddr4_trcmin;
		trfc = ddr4_trfc1min;
		twtr = DDR4_TWTR_S;
		tfaw = ddr4_tfawmin;

		if (spd_rdimm) {
			spd_addr_mirror = read_spd(&dimm_config_table[0], 0,
			DDR4_SPD_RDIMM_ADDR_MAPPING_FROM_REGISTER_TO_DRAM) &
			0x1;
		} else {
			spd_addr_mirror = read_spd(&dimm_config_table[0], 0,
				DDR4_SPD_UDIMM_ADDR_MAPPING_FROM_EDGE) & 0x1;
		}
		debug("spd_addr_mirror : %#06x\n", spd_addr_mirror);
	} else {
		spd_mtb_dividend =
		    0xff & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_MEDIUM_TIMEBASE_DIVIDEND);
		spd_mtb_divisor =
		    0xff & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_MEDIUM_TIMEBASE_DIVISOR);
		spd_tck_min =
		    0xff & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_MINIMUM_CYCLE_TIME_TCKMIN);
		spd_taa_min =
		    0xff & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_MIN_CAS_LATENCY_TAAMIN);

		spd_twr =
		    0xff & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_MIN_WRITE_RECOVERY_TWRMIN);
		spd_trcd =
		    0xff & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_MIN_RAS_CAS_DELAY_TRCDMIN);
		spd_trrd =
		    0xff & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_MIN_ROW_ACTIVE_DELAY_TRRDMIN);
		spd_trp =
		    0xff & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_MIN_ROW_PRECHARGE_DELAY_TRPMIN);
		spd_tras =
		    0xff & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_MIN_ACTIVE_PRECHARGE_LSB_TRASMIN);
		spd_tras |=
		    ((0xff &
		      read_spd(&dimm_config_table[0], 0,
			       DDR3_SPD_UPPER_NIBBLES_TRAS_TRC) & 0xf) << 8);
		spd_trc =
		    0xff & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_MIN_ACTIVE_REFRESH_LSB_TRCMIN);
		spd_trc |=
		    ((0xff &
		      read_spd(&dimm_config_table[0], 0,
			       DDR3_SPD_UPPER_NIBBLES_TRAS_TRC) & 0xf0) << 4);
		spd_trfc =
		    0xff & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_MIN_REFRESH_RECOVERY_LSB_TRFCMIN);
		spd_trfc |=
		    ((0xff &
		      read_spd(&dimm_config_table[0], 0,
			       DDR3_SPD_MIN_REFRESH_RECOVERY_MSB_TRFCMIN)) <<
		     8);
		spd_twtr =
		    0xff & read_spd(&dimm_config_table[0], 0,
				DDR3_SPD_MIN_INTERNAL_WRITE_READ_CMD_TWTRMIN);
		spd_trtp =
		    0xff & read_spd(&dimm_config_table[0], 0,
			DDR3_SPD_MIN_INTERNAL_READ_PRECHARGE_CMD_TRTPMIN);
		spd_tfaw =
		    0xff & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_MIN_FOUR_ACTIVE_WINDOW_TFAWMIN);
		spd_tfaw |=
		    ((0xff &
		      read_spd(&dimm_config_table[0], 0,
			       DDR3_SPD_UPPER_NIBBLE_TFAW) & 0xf) << 8);
		spd_addr_mirror =
		    0xff & read_spd(&dimm_config_table[0], 0,
				    DDR3_SPD_ADDRESS_MAPPING) & 0x1;
		/* Only address mirror unbuffered dimms.  */
		spd_addr_mirror = spd_addr_mirror && !spd_rdimm;
		ftb_dividend =
		    read_spd(&dimm_config_table[0], 0,
			     DDR3_SPD_FINE_TIMEBASE_DIVIDEND_DIVISOR) >> 4;
		ftb_divisor =
		    read_spd(&dimm_config_table[0], 0,
			     DDR3_SPD_FINE_TIMEBASE_DIVIDEND_DIVISOR) & 0xf;
		/* Make sure that it is not 0 */
		ftb_divisor = (ftb_divisor == 0) ? 1 : ftb_divisor;

		debug("spd_twr         : %#06x\n", spd_twr);
		debug("spd_trcd        : %#06x\n", spd_trcd);
		debug("spd_trrd        : %#06x\n", spd_trrd);
		debug("spd_trp         : %#06x\n", spd_trp);
		debug("spd_tras        : %#06x\n", spd_tras);
		debug("spd_trc         : %#06x\n", spd_trc);
		debug("spd_trfc        : %#06x\n", spd_trfc);
		debug("spd_twtr        : %#06x\n", spd_twtr);
		debug("spd_trtp        : %#06x\n", spd_trtp);
		debug("spd_tfaw        : %#06x\n", spd_tfaw);
		debug("spd_addr_mirror : %#06x\n", spd_addr_mirror);

		mtb_psec = spd_mtb_dividend * 1000 / spd_mtb_divisor;
		taamin = mtb_psec * spd_taa_min;
		taamin += ftb_dividend *
			(signed char)read_spd(&dimm_config_table[0],
				0, DDR3_SPD_MIN_CAS_LATENCY_FINE_TAAMIN) /
			ftb_divisor;
		tckmin = mtb_psec * spd_tck_min;
		tckmin += ftb_dividend *
			(signed char)read_spd(&dimm_config_table[0],
				0, DDR3_SPD_MINIMUM_CYCLE_TIME_FINE_TCKMIN) /
			ftb_divisor;

		twr = spd_twr * mtb_psec;
		trcd = spd_trcd * mtb_psec;
		trrd = spd_trrd * mtb_psec;
		trp = spd_trp * mtb_psec;
		tras = spd_tras * mtb_psec;
		trc = spd_trc * mtb_psec;
		trfc = spd_trfc * mtb_psec;
		if (octeon_is_cpuid(OCTEON_CN78XX_PASS2_X) && trfc < 260000) {
			// default to this - because it works...
			int new_trfc = 260000;

			s = env_get("ddr_trfc");
			if (s) {
				new_trfc = simple_strtoul(s, NULL, 0);
				printf("Parameter found in environment. ddr_trfc = %d\n",
				       new_trfc);
				if (new_trfc < 160000 || new_trfc > 260000) {
					// back to default if out of range
					new_trfc = 260000;
				}
			}
			debug("N%d.LMC%d: Adjusting tRFC from %d to %d, for CN78XX Pass 2.x\n",
			      node, if_num, trfc, new_trfc);
			trfc = new_trfc;
		}

		twtr = spd_twtr * mtb_psec;
		trtp = spd_trtp * mtb_psec;
		tfaw = spd_tfaw * mtb_psec;

		debug("Medium Timebase (MTB)                         : %6d ps\n",
		      mtb_psec);
		debug("Minimum Cycle Time (tckmin)                   : %6d ps (%ld MT/s)\n",
		      tckmin, pretty_psecs_to_mts(tckmin));
		debug("Minimum CAS Latency Time (taamin)             : %6d ps\n",
		      taamin);
		debug("Write Recovery Time (tWR)                     : %6d ps\n",
		      twr);
		debug("Minimum RAS to CAS delay (tRCD)               : %6d ps\n",
		      trcd);
		debug("Minimum Row Active to Row Active delay (tRRD) : %6d ps\n",
		      trrd);
		debug("Minimum Row Precharge Delay (tRP)             : %6d ps\n",
		      trp);
		debug("Minimum Active to Precharge (tRAS)            : %6d ps\n",
		      tras);
		debug("Minimum Active to Active/Refresh Delay (tRC)  : %6d ps\n",
		      trc);
		debug("Minimum Refresh Recovery Delay (tRFC)         : %6d ps\n",
		      trfc);
		debug("Internal write to read command delay (tWTR)   : %6d ps\n",
		      twtr);
		debug("Min Internal Rd to Precharge Cmd Delay (tRTP) : %6d ps\n",
		      trtp);
		debug("Minimum Four Activate Window Delay (tFAW)     : %6d ps\n",
		      tfaw);
	}

	/*
	 * When the cycle time is within 1 psec of the minimum accept it
	 * as a slight rounding error and adjust it to exactly the minimum
	 * cycle time. This avoids an unnecessary warning.
	 */
	if (abs(tclk_psecs - tckmin) < 2)
		tclk_psecs = tckmin;

	if (tclk_psecs < (u64)tckmin) {
		printf("WARNING!!!!: DDR Clock Rate (tCLK: %ld) exceeds DIMM specifications (tckmin: %ld)!!!!\n",
		       tclk_psecs, (ulong)tckmin);
	}

	debug("DDR Clock Rate (tCLK)                         : %6ld ps\n",
	      tclk_psecs);
	debug("Core Clock Rate (eCLK)                        : %6ld ps\n",
	      eclk_psecs);

	s = env_get("ddr_use_ecc");
	if (s) {
		use_ecc = !!simple_strtoul(s, NULL, 0);
		printf("Parameter found in environment. ddr_use_ecc = %d\n",
		       use_ecc);
	}
	use_ecc = use_ecc && spd_ecc;

	if_bytemask = if_64b ? (use_ecc ? 0x1ff : 0xff)
	    : (use_ecc ? 0x01f : 0x0f);

	debug("DRAM Interface width: %d bits %s bytemask 0x%03x\n",
	      if_64b ? 64 : 32, use_ecc ? "+ECC" : "", if_bytemask);

	debug("\n------ Board Custom Configuration Settings ------\n");
	debug("%-45s : %d\n", "MIN_RTT_NOM_IDX   ", c_cfg->min_rtt_nom_idx);
	debug("%-45s : %d\n", "MAX_RTT_NOM_IDX   ", c_cfg->max_rtt_nom_idx);
	debug("%-45s : %d\n", "MIN_RODT_CTL      ", c_cfg->min_rodt_ctl);
	debug("%-45s : %d\n", "MAX_RODT_CTL      ", c_cfg->max_rodt_ctl);
	debug("%-45s : %d\n", "MIN_CAS_LATENCY   ", c_cfg->min_cas_latency);
	debug("%-45s : %d\n", "OFFSET_EN         ", c_cfg->offset_en);
	debug("%-45s : %d\n", "OFFSET_UDIMM      ", c_cfg->offset_udimm);
	debug("%-45s : %d\n", "OFFSET_RDIMM      ", c_cfg->offset_rdimm);
	debug("%-45s : %d\n", "DDR_RTT_NOM_AUTO  ", c_cfg->ddr_rtt_nom_auto);
	debug("%-45s : %d\n", "DDR_RODT_CTL_AUTO ", c_cfg->ddr_rodt_ctl_auto);
	if (spd_rdimm)
		debug("%-45s : %d\n", "RLEVEL_COMP_OFFSET",
		      c_cfg->rlevel_comp_offset_rdimm);
	else
		debug("%-45s : %d\n", "RLEVEL_COMP_OFFSET",
		      c_cfg->rlevel_comp_offset_udimm);
	debug("%-45s : %d\n", "RLEVEL_COMPUTE    ", c_cfg->rlevel_compute);
	debug("%-45s : %d\n", "DDR2T_UDIMM       ", c_cfg->ddr2t_udimm);
	debug("%-45s : %d\n", "DDR2T_RDIMM       ", c_cfg->ddr2t_rdimm);
	debug("%-45s : %d\n", "FPRCH2            ", c_cfg->fprch2);
	debug("%-45s : %d\n", "PTUNE_OFFSET      ", c_cfg->ptune_offset);
	debug("%-45s : %d\n", "NTUNE_OFFSET      ", c_cfg->ntune_offset);
	debug("-------------------------------------------------\n");

	cl = divide_roundup(taamin, tclk_psecs);

	debug("Desired CAS Latency                           : %6d\n", cl);

	min_cas_latency = c_cfg->min_cas_latency;

	s = lookup_env(priv, "ddr_min_cas_latency");
	if (s)
		min_cas_latency = simple_strtoul(s, NULL, 0);

	debug("CAS Latencies supported in DIMM               :");
	base_cl = (ddr_type == DDR4_DRAM) ? 7 : 4;
	for (i = 0; i < 32; ++i) {
		if ((spd_cas_latency >> i) & 1) {
			debug(" %d", i + base_cl);
			max_cas_latency = i + base_cl;
			if (min_cas_latency == 0)
				min_cas_latency = i + base_cl;
		}
	}
	debug("\n");

	/*
	 * Use relaxed timing when running slower than the minimum
	 * supported speed.  Adjust timing to match the smallest supported
	 * CAS Latency.
	 */
	if (min_cas_latency > cl) {
		ulong adjusted_tclk = taamin / min_cas_latency;

		cl = min_cas_latency;
		debug("Slow clock speed. Adjusting timing: tClk = %ld, Adjusted tClk = %ld\n",
		      tclk_psecs, adjusted_tclk);
		tclk_psecs = adjusted_tclk;
	}

	s = env_get("ddr_cas_latency");
	if (s) {
		override_cas_latency = simple_strtoul(s, NULL, 0);
		printf("Parameter found in environment. ddr_cas_latency = %d\n",
		       override_cas_latency);
	}

	/* Make sure that the selected cas latency is legal */
	for (i = (cl - base_cl); i < 32; ++i) {
		if ((spd_cas_latency >> i) & 1) {
			cl = i + base_cl;
			break;
		}
	}

	if (max_cas_latency < cl)
		cl = max_cas_latency;

	if (override_cas_latency != 0)
		cl = override_cas_latency;

	debug("CAS Latency                                   : %6d\n", cl);

	if ((cl * tckmin) > 20000) {
		debug("(CLactual * tckmin) = %d exceeds 20 ns\n",
		      (cl * tckmin));
	}

	if (tclk_psecs < (ulong)tckmin) {
		printf("WARNING!!!!!!: DDR3 Clock Rate (tCLK: %ld) exceeds DIMM specifications (tckmin:%ld)!!!!!!!!\n",
		       tclk_psecs, (ulong)tckmin);
	}

	if (num_banks != 4 && num_banks != 8 && num_banks != 16) {
		printf("Unsupported number of banks %d. Must be 4 or 8.\n",
		       num_banks);
		++fatal_error;
	}

	if (num_ranks != 1 && num_ranks != 2 && num_ranks != 4) {
		printf("Unsupported number of ranks: %d\n", num_ranks);
		++fatal_error;
	}

	if (octeon_is_cpuid(OCTEON_CN78XX) ||
	    octeon_is_cpuid(OCTEON_CN73XX) ||
	    octeon_is_cpuid(OCTEON_CNF75XX)) {
		if (dram_width != 8 && dram_width != 16 && dram_width != 4) {
			printf("Unsupported SDRAM Width, %d.  Must be 4, 8 or 16.\n",
			       dram_width);
			++fatal_error;
		}
	} else if (dram_width != 8 && dram_width != 16) {
		printf("Unsupported SDRAM Width, %d.  Must be 8 or 16.\n",
		       dram_width);
		++fatal_error;
	}

	/*
	 ** Bail out here if things are not copasetic.
	 */
	if (fatal_error)
		return (-1);

	/*
	 * 4.8.4 LMC RESET Initialization
	 *
	 * The purpose of this step is to assert/deassert the RESET# pin at the
	 * DDR3/DDR4 parts.
	 *
	 * This LMC RESET step is done for all enabled LMCs.
	 */
	perform_lmc_reset(priv, node, if_num);

	// Make sure scrambling is disabled during init...
	ctrl.u64 = lmc_rd(priv, CVMX_LMCX_CONTROL(if_num));
	ctrl.s.scramble_ena = 0;
	lmc_wr(priv, CVMX_LMCX_CONTROL(if_num), ctrl.u64);

	lmc_wr(priv, CVMX_LMCX_SCRAMBLE_CFG0(if_num), 0);
	lmc_wr(priv, CVMX_LMCX_SCRAMBLE_CFG1(if_num), 0);
	if (!octeon_is_cpuid(OCTEON_CN78XX_PASS1_X))
		lmc_wr(priv, CVMX_LMCX_SCRAMBLE_CFG2(if_num), 0);

	odt_idx = min(dimm_count - 1, 3);

	switch (num_ranks) {
	case 1:
		odt_config = odt_1rank_config;
		break;
	case 2:
		odt_config = odt_2rank_config;
		break;
	case 4:
		odt_config = odt_4rank_config;
		break;
	default:
		odt_config = disable_odt_config;
		printf("Unsupported number of ranks: %d\n", num_ranks);
		++fatal_error;
	}

	/*
	 * 4.8.5 Early LMC Initialization
	 *
	 * All of DDR PLL, LMC CK, and LMC DRESET initializations must be
	 * completed prior to starting this LMC initialization sequence.
	 *
	 * Perform the following five substeps for early LMC initialization:
	 *
	 * 1. Software must ensure there are no pending DRAM transactions.
	 *
	 * 2. Write LMC(0)_CONFIG, LMC(0)_CONTROL, LMC(0)_TIMING_PARAMS0,
	 *    LMC(0)_TIMING_PARAMS1, LMC(0)_MODEREG_PARAMS0,
	 *    LMC(0)_MODEREG_PARAMS1, LMC(0)_DUAL_MEMCFG, LMC(0)_NXM,
	 *    LMC(0)_WODT_MASK, LMC(0)_RODT_MASK, LMC(0)_COMP_CTL2,
	 *    LMC(0)_PHY_CTL, LMC(0)_DIMM0/1_PARAMS, and LMC(0)_DIMM_CTL with
	 *    appropriate values. All sections in this chapter can be used to
	 *    derive proper register settings.
	 */

	/* LMC(0)_CONFIG */
	lmc_config(priv);

	/* LMC(0)_CONTROL */
	lmc_control(priv);

	/* LMC(0)_TIMING_PARAMS0 */
	lmc_timing_params0(priv);

	/* LMC(0)_TIMING_PARAMS1 */
	lmc_timing_params1(priv);

	/* LMC(0)_TIMING_PARAMS2 */
	lmc_timing_params2(priv);

	/* LMC(0)_MODEREG_PARAMS0 */
	lmc_modereg_params0(priv);

	/* LMC(0)_MODEREG_PARAMS1 */
	lmc_modereg_params1(priv);

	/* LMC(0)_MODEREG_PARAMS2 */
	lmc_modereg_params2(priv);

	/* LMC(0)_MODEREG_PARAMS3 */
	lmc_modereg_params3(priv);

	/* LMC(0)_NXM */
	lmc_nxm(priv);

	/* LMC(0)_WODT_MASK */
	lmc_wodt_mask(priv);

	/* LMC(0)_RODT_MASK */
	lmc_rodt_mask(priv);

	/* LMC(0)_COMP_CTL2 */
	lmc_comp_ctl2(priv);

	/* LMC(0)_PHY_CTL */
	lmc_phy_ctl(priv);

	/* LMC(0)_EXT_CONFIG */
	lmc_ext_config(priv);

	/* LMC(0)_EXT_CONFIG2 */
	lmc_ext_config2(priv);

	/* LMC(0)_DIMM0/1_PARAMS */
	lmc_dimm01_params(priv);

	ret = lmc_rank_init(priv);
	if (ret < 0)
		return 0;	/* 0 indicates problem */

	lmc_config_2(priv);

	lmc_write_leveling(priv);

	lmc_read_leveling(priv);

	lmc_workaround(priv);

	ret = lmc_sw_write_leveling(priv);
	if (ret < 0)
		return 0;	/* 0 indicates problem */

	// this sometimes causes stack overflow crashes..
	// display only for DDR4 RDIMMs.
	if (ddr_type == DDR4_DRAM && spd_rdimm) {
		int i;

		for (i = 0; i < 3; i += 2)	// just pages 0 and 2 for now..
			display_mpr_page(priv, rank_mask, if_num, i);
	}

	lmc_dll(priv);

	lmc_workaround_2(priv);

	lmc_final(priv);

	lmc_scrambling(priv);

	return mem_size_mbytes;
}

/////    HW-assist byte DLL offset tuning   //////

static int cvmx_dram_get_num_lmc(struct ddr_priv *priv)
{
	union cvmx_lmcx_dll_ctl2 lmcx_dll_ctl2;

	if (octeon_is_cpuid(OCTEON_CN70XX))
		return 1;

	if (octeon_is_cpuid(OCTEON_CN73XX) || octeon_is_cpuid(OCTEON_CNF75XX)) {
		// sample LMC1
		lmcx_dll_ctl2.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL2(1));
		if (lmcx_dll_ctl2.cn78xx.intf_en)
			return 2;
		else
			return 1;
	}

	// for CN78XX, LMCs are always active in pairs, and always LMC0/1
	// so, we sample LMC2 to see if 2 and 3 are active
	lmcx_dll_ctl2.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL2(2));
	if (lmcx_dll_ctl2.cn78xx.intf_en)
		return 4;
	else
		return 2;
}

// got to do these here, even though already defined in BDK

// all DDR3, and DDR4 x16 today, use only 3 bank bits;
// DDR4 x4 and x8 always have 4 bank bits
// NOTE: this will change in the future, when DDR4 x16 devices can
// come with 16 banks!! FIXME!!
static int cvmx_dram_get_num_bank_bits(struct ddr_priv *priv, int lmc)
{
	union cvmx_lmcx_dll_ctl2 lmcx_dll_ctl2;
	union cvmx_lmcx_config lmcx_config;
	union cvmx_lmcx_ddr_pll_ctl lmcx_ddr_pll_ctl;
	int bank_width;

	// can always read this
	lmcx_dll_ctl2.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL2(lmc));

	if (lmcx_dll_ctl2.cn78xx.dreset)	// check LMCn
		return 0;

	lmcx_config.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL2(lmc));
	lmcx_ddr_pll_ctl.u64 = lmc_rd(priv, CVMX_LMCX_DDR_PLL_CTL(lmc));

	bank_width = ((lmcx_ddr_pll_ctl.s.ddr4_mode != 0) &&
		      (lmcx_config.s.bg2_enable)) ? 4 : 3;

	return bank_width;
}

#define EXTRACT(v, lsb, width) (((v) >> (lsb)) & ((1ull << (width)) - 1))
#define ADDRESS_HOLE 0x10000000ULL

static void cvmx_dram_address_extract_info(struct ddr_priv *priv, u64 address,
					   int *node, int *lmc, int *dimm,
					   int *prank, int *lrank, int *bank,
					   int *row, int *col)
{
	int bank_lsb, xbits;
	union cvmx_l2c_ctl l2c_ctl;
	union cvmx_lmcx_config lmcx_config;
	union cvmx_lmcx_control lmcx_control;
	union cvmx_lmcx_ext_config ext_config;
	int bitno = (octeon_is_cpuid(OCTEON_CN7XXX)) ? 20 : 18;
	int bank_width;
	int dimm_lsb;
	int dimm_width;
	int prank_lsb, lrank_lsb;
	int prank_width, lrank_width;
	int row_lsb;
	int row_width;
	int col_hi_lsb;
	int col_hi_width;
	int col_hi;

	if (octeon_is_cpuid(OCTEON_CN73XX) || octeon_is_cpuid(OCTEON_CNF75XX))
		bitno = 18;

	*node = EXTRACT(address, 40, 2);	/* Address bits [41:40] */

	address &= (1ULL << 40) - 1;	// lop off any node bits or above
	if (address >= ADDRESS_HOLE)	// adjust down if at HOLE or above
		address -= ADDRESS_HOLE;

	/* Determine the LMC controllers */
	l2c_ctl.u64 = l2c_rd(priv, CVMX_L2C_CTL_REL);

	/* xbits depends on number of LMCs */
	xbits = cvmx_dram_get_num_lmc(priv) >> 1;	// 4->2, 2->1, 1->0
	bank_lsb = 7 + xbits;

	/* LMC number is probably aliased */
	if (l2c_ctl.s.disidxalias) {
		*lmc = EXTRACT(address, 7, xbits);
	}  else {
		*lmc = EXTRACT(address, 7, xbits) ^
			EXTRACT(address, bitno, xbits) ^
			EXTRACT(address, 12, xbits);
	}

	/* Figure out the bank field width */
	lmcx_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(*lmc));
	ext_config.u64 = lmc_rd(priv, CVMX_LMCX_EXT_CONFIG(*lmc));
	bank_width = cvmx_dram_get_num_bank_bits(priv, *lmc);

	/* Extract additional info from the LMC_CONFIG CSR */
	dimm_lsb = 28 + lmcx_config.s.pbank_lsb + xbits;
	dimm_width = 40 - dimm_lsb;
	prank_lsb = dimm_lsb - lmcx_config.s.rank_ena;
	prank_width = dimm_lsb - prank_lsb;
	lrank_lsb = prank_lsb - ext_config.s.dimm0_cid;
	lrank_width = prank_lsb - lrank_lsb;
	row_lsb = 14 + lmcx_config.s.row_lsb + xbits;
	row_width = lrank_lsb - row_lsb;
	col_hi_lsb = bank_lsb + bank_width;
	col_hi_width = row_lsb - col_hi_lsb;

	/* Extract the parts of the address */
	*dimm = EXTRACT(address, dimm_lsb, dimm_width);
	*prank = EXTRACT(address, prank_lsb, prank_width);
	*lrank = EXTRACT(address, lrank_lsb, lrank_width);
	*row = EXTRACT(address, row_lsb, row_width);

	/* bank calculation may be aliased... */
	lmcx_control.u64 = lmc_rd(priv, CVMX_LMCX_CONTROL(*lmc));
	if (lmcx_control.s.xor_bank) {
		*bank = EXTRACT(address, bank_lsb, bank_width) ^
			EXTRACT(address, 12 + xbits, bank_width);
	} else {
		*bank = EXTRACT(address, bank_lsb, bank_width);
	}

	/* LMC number already extracted */
	col_hi = EXTRACT(address, col_hi_lsb, col_hi_width);
	*col = EXTRACT(address, 3, 4) | (col_hi << 4);
	/* Bus byte is address bits [2:0]. Unused here */
}

// end of added workarounds

// NOTE: "mode" argument:
//         DBTRAIN_TEST: for testing using GP patterns, includes ECC
//         DBTRAIN_DBI:  for DBI deskew training behavior (uses GP patterns)
//         DBTRAIN_LFSR: for testing using LFSR patterns, includes ECC
// NOTE: trust the caller to specify the correct/supported mode
//
static int test_dram_byte_hw(struct ddr_priv *priv, int if_num, u64 p,
			     int mode, u64 *xor_data)
{
	u64 p1;
	u64 k;
	int errors = 0;

	u64 mpr_data0, mpr_data1;
	u64 bad_bits[2] = { 0, 0 };

	int node_address, lmc, dimm;
	int prank, lrank;
	int bank, row, col;
	int save_or_dis;
	int byte;
	int ba_loop, ba_bits;

	union cvmx_lmcx_rlevel_ctl rlevel_ctl;
	union cvmx_lmcx_dbtrain_ctl dbtrain_ctl;
	union cvmx_lmcx_phy_ctl phy_ctl;

	int biter_errs;

	// FIXME: K iterations set to 4 for now.
	// FIXME: decrement to increase interations.
	// FIXME: must be no less than 22 to stay above an LMC hash field.
	int kshift = 27;

	const char *s;
	int node = 0;

	// allow override default setting for kshift
	s = env_get("ddr_tune_set_kshift");
	if (s) {
		int temp = simple_strtoul(s, NULL, 0);

		if (temp < 22 || temp > 28) {
			debug("N%d.LMC%d: ILLEGAL override of kshift to %d, using default %d\n",
			      node, if_num, temp, kshift);
		} else {
			debug("N%d.LMC%d: overriding kshift (%d) to %d\n",
			      node, if_num, kshift, temp);
			kshift = temp;
		}
	}

	/*
	 * 1) Make sure that RLEVEL_CTL[OR_DIS] = 0.
	 */
	rlevel_ctl.u64 = lmc_rd(priv, CVMX_LMCX_RLEVEL_CTL(if_num));
	save_or_dis = rlevel_ctl.s.or_dis;
	/* or_dis must be disabled for this sequence */
	rlevel_ctl.s.or_dis = 0;
	lmc_wr(priv, CVMX_LMCX_RLEVEL_CTL(if_num), rlevel_ctl.u64);

	/*
	 * NOTE: this step done in the calling routine(s)...
	 * 3) Setup GENERAL_PURPOSE[0-2] registers with the data pattern
	 * of choice.
	 * a. GENERAL_PURPOSE0[DATA<63:0>] - sets the initial lower
	 * (rising edge) 64 bits of data.
	 * b. GENERAL_PURPOSE1[DATA<63:0>] - sets the initial upper
	 * (falling edge) 64 bits of data.
	 * c. GENERAL_PURPOSE2[DATA<15:0>] - sets the initial lower
	 * (rising edge <7:0>) and upper (falling edge <15:8>) ECC data.
	 */

	// final address must include LMC and node
	p |= (if_num << 7);	/* Map address into proper interface */
	p |= (u64)node << CVMX_NODE_MEM_SHIFT;	// map to node

	/*
	 * Add base offset to both test regions to not clobber u-boot stuff
	 * when running from L2 for NAND boot.
	 */
	p += 0x20000000;	// offset to 512MB, ie above THE HOLE!!!
	p |= 1ull << 63;	// needed for OCTEON

	errors = 0;

	cvmx_dram_address_extract_info(priv, p, &node_address, &lmc, &dimm,
				       &prank, &lrank, &bank, &row, &col);
	debug("%s: START at A:0x%012llx, N%d L%d D%d/%d R%d B%1x Row:%05x Col:%05x\n",
	      __func__, p, node_address, lmc, dimm, prank, lrank, bank,
	      row, col);

	// only check once per call, and ignore if no match...
	if ((int)node != node_address) {
		printf("ERROR: Node address mismatch\n");
		return 0;
	}
	if (lmc != if_num) {
		printf("ERROR: LMC address mismatch\n");
		return 0;
	}

	/*
	 * 7) Set PHY_CTL[PHY_RESET] = 1 (LMC automatically clears this as
	 * it's a one-shot operation). This is to get into the habit of
	 * resetting PHY's SILO to the original 0 location.
	 */
	phy_ctl.u64 = lmc_rd(priv, CVMX_LMCX_PHY_CTL(if_num));
	phy_ctl.s.phy_reset = 1;
	lmc_wr(priv, CVMX_LMCX_PHY_CTL(if_num), phy_ctl.u64);

	/*
	 * Walk through a range of addresses avoiding bits that alias
	 * interfaces on the CN88XX.
	 */

	// FIXME: want to try to keep the K increment from affecting the
	// LMC via hash, so keep it above bit 21 we also want to keep k
	// less than the base offset of bit 29 (512MB)

	for (k = 0; k < (1UL << 29); k += (1UL << kshift)) {
		// FIXME: the sequence will interate over 1/2 cacheline
		// FIXME: for each unit specified in "read_cmd_count",
		// FIXME: so, we setup each sequence to do the max cachelines
		// it can

		p1 = p + k;

		cvmx_dram_address_extract_info(priv, p1, &node_address, &lmc,
					       &dimm, &prank, &lrank, &bank,
					       &row, &col);

		/*
		 * 2) Setup the fields of the CSR DBTRAIN_CTL as follows:
		 * a. COL, ROW, BA, BG, PRANK points to the starting point
		 * of the address.
		 * You can just set them to all 0.
		 * b. RW_TRAIN - set this to 1.
		 * c. TCCD_L - set this to 0.
		 * d. READ_CMD_COUNT - instruct the sequence to the how many
		 * writes/reads.
		 * It is 5 bits field, so set to 31 of maximum # of r/w.
		 */
		dbtrain_ctl.u64 = lmc_rd(priv, CVMX_LMCX_DBTRAIN_CTL(if_num));
		dbtrain_ctl.s.column_a = col;
		dbtrain_ctl.s.row_a = row;
		dbtrain_ctl.s.bg = (bank >> 2) & 3;
		dbtrain_ctl.s.prank = (dimm * 2) + prank;	// FIXME?
		dbtrain_ctl.s.lrank = lrank;	// FIXME?
		dbtrain_ctl.s.activate = (mode == DBTRAIN_DBI);
		dbtrain_ctl.s.write_ena = 1;
		dbtrain_ctl.s.read_cmd_count = 31;	// max count pass 1.x
		if (octeon_is_cpuid(OCTEON_CN78XX_PASS2_X) ||
		    octeon_is_cpuid(OCTEON_CNF75XX)) {
			// max count on chips that support it
			dbtrain_ctl.s.cmd_count_ext = 3;
		} else {
			// max count pass 1.x
			dbtrain_ctl.s.cmd_count_ext = 0;
		}

		dbtrain_ctl.s.rw_train = 1;
		dbtrain_ctl.s.tccd_sel = (mode == DBTRAIN_DBI);
		// LFSR should only be on when chip supports it...
		dbtrain_ctl.s.lfsr_pattern_sel = (mode == DBTRAIN_LFSR) ? 1 : 0;

		biter_errs = 0;

		// for each address, iterate over the 4 "banks" in the BA
		for (ba_loop = 0, ba_bits = bank & 3;
		     ba_loop < 4; ba_loop++, ba_bits = (ba_bits + 1) & 3) {
			dbtrain_ctl.s.ba = ba_bits;
			lmc_wr(priv, CVMX_LMCX_DBTRAIN_CTL(if_num),
			       dbtrain_ctl.u64);

			/*
			 * We will use the RW_TRAINING sequence (14) for
			 * this task.
			 *
			 * 4) Kick off the sequence (SEQ_CTL[SEQ_SEL] = 14,
			 *    SEQ_CTL[INIT_START] = 1).
			 * 5) Poll on SEQ_CTL[SEQ_COMPLETE] for completion.
			 */
			oct3_ddr3_seq(priv, prank, if_num, 14);

			/*
			 * 6) Read MPR_DATA0 and MPR_DATA1 for results.
			 * a. MPR_DATA0[MPR_DATA<63:0>] - comparison results
			 *    for DQ63:DQ0. (1 means MATCH, 0 means FAIL).
			 * b. MPR_DATA1[MPR_DATA<7:0>] - comparison results
			 *    for ECC bit7:0.
			 */
			mpr_data0 = lmc_rd(priv, CVMX_LMCX_MPR_DATA0(if_num));
			mpr_data1 = lmc_rd(priv, CVMX_LMCX_MPR_DATA1(if_num));

			/*
			 * 7) Set PHY_CTL[PHY_RESET] = 1 (LMC automatically
			 * clears this as it's a one-shot operation).
			 * This is to get into the habit of resetting PHY's
			 * SILO to the original 0 location.
			 */
			phy_ctl.u64 = lmc_rd(priv, CVMX_LMCX_PHY_CTL(if_num));
			phy_ctl.s.phy_reset = 1;
			lmc_wr(priv, CVMX_LMCX_PHY_CTL(if_num), phy_ctl.u64);

			// bypass any error checking or updating when DBI mode
			if (mode == DBTRAIN_DBI)
				continue;

			// data bytes
			if (~mpr_data0) {
				for (byte = 0; byte < 8; byte++) {
					if ((~mpr_data0 >> (8 * byte)) & 0xffUL)
						biter_errs |= (1 << byte);
				}
				// accumulate bad bits
				bad_bits[0] |= ~mpr_data0;
			}

			// include ECC byte errors
			if (~mpr_data1 & 0xffUL) {
				biter_errs |= (1 << 8);
				bad_bits[1] |= ~mpr_data1 & 0xffUL;
			}
		}

		errors |= biter_errs;
	}			/* end for (k=...) */

	rlevel_ctl.s.or_dis = save_or_dis;
	lmc_wr(priv, CVMX_LMCX_RLEVEL_CTL(if_num), rlevel_ctl.u64);

	// send the bad bits back...
	if (mode != DBTRAIN_DBI && xor_data) {
		xor_data[0] = bad_bits[0];
		xor_data[1] = bad_bits[1];
	}

	return errors;
}

// setup default for byte test pattern array
// take these from the HRM section 6.9.13
static const u64 byte_pattern_0[] = {
	0xFFAAFFFFFF55FFFFULL,	// GP0
	0x55555555AAAAAAAAULL,	// GP1
	0xAA55AAAAULL,		// GP2
};

static const u64 byte_pattern_1[] = {
	0xFBF7EFDFBF7FFEFDULL,	// GP0
	0x0F1E3C78F0E1C387ULL,	// GP1
	0xF0E1BF7FULL,		// GP2
};

// this is from Andrew via LFSR with PRBS=0xFFFFAAAA
static const u64 byte_pattern_2[] = {
	0xEE55AADDEE55AADDULL,	// GP0
	0x55AADDEE55AADDEEULL,	// GP1
	0x55EEULL,		// GP2
};

// this is from Mike via LFSR with PRBS=0x4A519909
static const u64 byte_pattern_3[] = {
	0x0088CCEE0088CCEEULL,	// GP0
	0xBB552211BB552211ULL,	// GP1
	0xBB00ULL,		// GP2
};

static const u64 *byte_patterns[4] = {
	byte_pattern_0, byte_pattern_1, byte_pattern_2, byte_pattern_3
};

static const u32 lfsr_patterns[4] = {
	0xFFFFAAAAUL, 0x06000000UL, 0xAAAAFFFFUL, 0x4A519909UL
};

#define NUM_BYTE_PATTERNS 4

#define DEFAULT_BYTE_BURSTS 32	// compromise between time and rigor

static void setup_hw_pattern(struct ddr_priv *priv, int lmc,
			     const u64 *pattern_p)
{
	/*
	 * 3) Setup GENERAL_PURPOSE[0-2] registers with the data pattern
	 * of choice.
	 * a. GENERAL_PURPOSE0[DATA<63:0>] - sets the initial lower
	 *    (rising edge) 64 bits of data.
	 * b. GENERAL_PURPOSE1[DATA<63:0>] - sets the initial upper
	 *    (falling edge) 64 bits of data.
	 * c. GENERAL_PURPOSE2[DATA<15:0>] - sets the initial lower
	 *    (rising edge <7:0>) and upper
	 * (falling edge <15:8>) ECC data.
	 */
	lmc_wr(priv, CVMX_LMCX_GENERAL_PURPOSE0(lmc), pattern_p[0]);
	lmc_wr(priv, CVMX_LMCX_GENERAL_PURPOSE1(lmc), pattern_p[1]);
	lmc_wr(priv, CVMX_LMCX_GENERAL_PURPOSE2(lmc), pattern_p[2]);
}

static void setup_lfsr_pattern(struct ddr_priv *priv, int lmc, u32 data)
{
	union cvmx_lmcx_char_ctl char_ctl;
	u32 prbs;
	const char *s;

	s = env_get("ddr_lfsr_prbs");
	if (s)
		prbs = simple_strtoul(s, NULL, 0);
	else
		prbs = data;

	/*
	 * 2) DBTRAIN_CTL[LFSR_PATTERN_SEL] = 1
	 * here data comes from the LFSR generating a PRBS pattern
	 * CHAR_CTL.EN = 0
	 * CHAR_CTL.SEL = 0; // for PRBS
	 * CHAR_CTL.DR = 1;
	 * CHAR_CTL.PRBS = setup for whatever type of PRBS to send
	 * CHAR_CTL.SKEW_ON = 1;
	 */
	char_ctl.u64 = lmc_rd(priv, CVMX_LMCX_CHAR_CTL(lmc));
	char_ctl.s.en = 0;
	char_ctl.s.sel = 0;
	char_ctl.s.dr = 1;
	char_ctl.s.prbs = prbs;
	char_ctl.s.skew_on = 1;
	lmc_wr(priv, CVMX_LMCX_CHAR_CTL(lmc), char_ctl.u64);
}

static int choose_best_hw_patterns(int lmc, int mode)
{
	int new_mode = mode;
	const char *s;

	switch (mode) {
	case DBTRAIN_TEST:	// always choose LFSR if chip supports it
		if (octeon_is_cpuid(OCTEON_CN78XX_PASS2_X)) {
			int lfsr_enable = 1;

			s = env_get("ddr_allow_lfsr");
			if (s) {
				// override?
				lfsr_enable = !!strtoul(s, NULL, 0);
			}

			if (lfsr_enable)
				new_mode = DBTRAIN_LFSR;
		}
		break;

	case DBTRAIN_DBI:	// possibly can allow LFSR use?
		break;

	case DBTRAIN_LFSR:	// forced already
		if (!octeon_is_cpuid(OCTEON_CN78XX_PASS2_X)) {
			debug("ERROR: illegal HW assist mode %d\n", mode);
			new_mode = DBTRAIN_TEST;
		}
		break;

	default:
		debug("ERROR: unknown HW assist mode %d\n", mode);
	}

	if (new_mode != mode)
		debug("%s: changing mode %d to %d\n", __func__, mode, new_mode);

	return new_mode;
}

int run_best_hw_patterns(struct ddr_priv *priv, int lmc, u64 phys_addr,
			 int mode, u64 *xor_data)
{
	int pattern;
	const u64 *pattern_p;
	int errs, errors = 0;

	// FIXME? always choose LFSR if chip supports it???
	mode = choose_best_hw_patterns(lmc, mode);

	for (pattern = 0; pattern < NUM_BYTE_PATTERNS; pattern++) {
		if (mode == DBTRAIN_LFSR) {
			setup_lfsr_pattern(priv, lmc, lfsr_patterns[pattern]);
		} else {
			pattern_p = byte_patterns[pattern];
			setup_hw_pattern(priv, lmc, pattern_p);
		}
		errs = test_dram_byte_hw(priv, lmc, phys_addr, mode, xor_data);

		debug("%s: PATTERN %d at A:0x%012llx errors 0x%x\n",
		      __func__, pattern, phys_addr, errs);

		errors |= errs;
	}

	return errors;
}

static void hw_assist_test_dll_offset(struct ddr_priv *priv,
				      int dll_offset_mode, int lmc,
				      int bytelane,
				      int if_64b,
				      u64 dram_tune_rank_offset,
				      int dram_tune_byte_bursts)
{
	int byte_offset, new_best_offset[9];
	int rank_delay_start[4][9];
	int rank_delay_count[4][9];
	int rank_delay_best_start[4][9];
	int rank_delay_best_count[4][9];
	int errors[4], off_errors, tot_errors;
	int rank_mask, rankx, active_ranks;
	int pattern;
	const u64 *pattern_p;
	int byte;
	char *mode_str = (dll_offset_mode == 2) ? "Read" : "Write";
	int pat_best_offset[9];
	u64 phys_addr;
	int pat_beg, pat_end;
	int rank_beg, rank_end;
	int byte_lo, byte_hi;
	union cvmx_lmcx_config lmcx_config;
	u64 hw_rank_offset;
	int num_lmcs = cvmx_dram_get_num_lmc(priv);
	// FIXME? always choose LFSR if chip supports it???
	int mode = choose_best_hw_patterns(lmc, DBTRAIN_TEST);
	int node = 0;

	if (bytelane == 0x0A) {	// all bytelanes
		byte_lo = 0;
		byte_hi = 8;
	} else {		// just 1
		byte_lo = bytelane;
		byte_hi = bytelane;
	}

	lmcx_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(lmc));
	rank_mask = lmcx_config.s.init_status;

	// this should be correct for 1 or 2 ranks, 1 or 2 DIMMs
	hw_rank_offset =
	    1ull << (28 + lmcx_config.s.pbank_lsb - lmcx_config.s.rank_ena +
		     (num_lmcs / 2));

	debug("N%d: %s: starting LMC%d with rank offset 0x%016llx\n",
	      node, __func__, lmc, (unsigned long long)hw_rank_offset);

	// start of pattern loop
	// we do the set of tests for each pattern supplied...

	memset(new_best_offset, 0, sizeof(new_best_offset));
	for (pattern = 0; pattern < NUM_BYTE_PATTERNS; pattern++) {
		memset(pat_best_offset, 0, sizeof(pat_best_offset));

		if (mode == DBTRAIN_TEST) {
			pattern_p = byte_patterns[pattern];
			setup_hw_pattern(priv, lmc, pattern_p);
		} else {
			setup_lfsr_pattern(priv, lmc, lfsr_patterns[pattern]);
		}

		// now loop through all legal values for the DLL byte offset...

#define BYTE_OFFSET_INCR 3	// FIXME: make this tunable?

		tot_errors = 0;

		memset(rank_delay_count, 0, sizeof(rank_delay_count));
		memset(rank_delay_start, 0, sizeof(rank_delay_start));
		memset(rank_delay_best_count, 0, sizeof(rank_delay_best_count));
		memset(rank_delay_best_start, 0, sizeof(rank_delay_best_start));

		for (byte_offset = -63; byte_offset < 64;
		     byte_offset += BYTE_OFFSET_INCR) {
			// do the setup on the active LMC
			// set the bytelanes DLL offsets
			change_dll_offset_enable(priv, lmc, 0);
			// FIXME? bytelane?
			load_dll_offset(priv, lmc, dll_offset_mode,
					byte_offset, bytelane);
			change_dll_offset_enable(priv, lmc, 1);

			//bdk_watchdog_poke();

			// run the test on each rank
			// only 1 call per rank should be enough, let the
			// bursts, loops, etc, control the load...

			// errors for this byte_offset, all ranks
			off_errors = 0;

			active_ranks = 0;

			for (rankx = 0; rankx < 4; rankx++) {
				if (!(rank_mask & (1 << rankx)))
					continue;

				phys_addr = hw_rank_offset * active_ranks;
				// FIXME: now done by test_dram_byte_hw()
				//phys_addr |= (lmc << 7);
				//phys_addr |= (u64)node << CVMX_NODE_MEM_SHIFT;

				active_ranks++;

				// NOTE: return is a now a bitmask of the
				// erroring bytelanes.
				errors[rankx] =
				    test_dram_byte_hw(priv, lmc, phys_addr,
						      mode, NULL);

				// process any errors in the bytelane(s) that
				// are being tested
				for (byte = byte_lo; byte <= byte_hi; byte++) {
					// check errors
					// yes, an error in the byte lane in
					// this rank
					if (errors[rankx] & (1 << byte)) {
						off_errors |= (1 << byte);

						debug("N%d.LMC%d.R%d: Bytelane %d DLL %s Offset Test %3d: Address 0x%012llx errors\n",
						      node, lmc, rankx, byte,
						      mode_str, byte_offset,
						      phys_addr);

						// had started run
						if (rank_delay_count
						    [rankx][byte] > 0) {
							debug("N%d.LMC%d.R%d: Bytelane %d DLL %s Offset Test %3d: stopping a run here\n",
							      node, lmc, rankx,
							      byte, mode_str,
							      byte_offset);
							// stop now
							rank_delay_count
								[rankx][byte] =
								0;
						}
						// FIXME: else had not started
						// run - nothing else to do?
					} else {
						// no error in the byte lane
						// first success, set run start
						if (rank_delay_count[rankx]
						    [byte] == 0) {
							debug("N%d.LMC%d.R%d: Bytelane %d DLL %s Offset Test %3d: starting a run here\n",
							      node, lmc, rankx,
							      byte, mode_str,
							      byte_offset);
							rank_delay_start[rankx]
								[byte] =
								byte_offset;
						}
						// bump run length
						rank_delay_count[rankx][byte]
							+= BYTE_OFFSET_INCR;

						// is this now the biggest
						// window?
						if (rank_delay_count[rankx]
						    [byte] >
						    rank_delay_best_count[rankx]
						    [byte]) {
							rank_delay_best_count
							    [rankx][byte] =
							    rank_delay_count
							    [rankx][byte];
							rank_delay_best_start
							    [rankx][byte] =
							    rank_delay_start
							    [rankx][byte];
							debug("N%d.LMC%d.R%d: Bytelane %d DLL %s Offset Test %3d: updating best to %d/%d\n",
							      node, lmc, rankx,
							      byte, mode_str,
							      byte_offset,
							      rank_delay_best_start
							      [rankx][byte],
							      rank_delay_best_count
							      [rankx][byte]);
						}
					}
				}
			} /* for (rankx = 0; rankx < 4; rankx++) */

			tot_errors |= off_errors;
		}

		// set the bytelanes DLL offsets all back to 0
		change_dll_offset_enable(priv, lmc, 0);
		load_dll_offset(priv, lmc, dll_offset_mode, 0, bytelane);
		change_dll_offset_enable(priv, lmc, 1);

		// now choose the best byte_offsets for this pattern
		// according to the best windows of the tested ranks
		// calculate offset by constructing an average window
		// from the rank windows
		for (byte = byte_lo; byte <= byte_hi; byte++) {
			pat_beg = -999;
			pat_end = 999;

			for (rankx = 0; rankx < 4; rankx++) {
				if (!(rank_mask & (1 << rankx)))
					continue;

				rank_beg = rank_delay_best_start[rankx][byte];
				pat_beg = max(pat_beg, rank_beg);
				rank_end = rank_beg +
					rank_delay_best_count[rankx][byte] -
					BYTE_OFFSET_INCR;
				pat_end = min(pat_end, rank_end);

				debug("N%d.LMC%d.R%d: Bytelane %d DLL %s Offset Test:  Rank Window %3d:%3d\n",
				      node, lmc, rankx, byte, mode_str,
				      rank_beg, rank_end);

			}	/* for (rankx = 0; rankx < 4; rankx++) */

			pat_best_offset[byte] = (pat_end + pat_beg) / 2;

			// sum the pattern averages
			new_best_offset[byte] += pat_best_offset[byte];
		}

		// now print them on 1 line, descending order...
		debug("N%d.LMC%d: HW DLL %s Offset Pattern %d :",
		      node, lmc, mode_str, pattern);
		for (byte = byte_hi; byte >= byte_lo; --byte)
			debug(" %4d", pat_best_offset[byte]);
		debug("\n");
	}
	// end of pattern loop

	debug("N%d.LMC%d: HW DLL %s Offset Average  : ", node, lmc, mode_str);

	// print in decending byte index order
	for (byte = byte_hi; byte >= byte_lo; --byte) {
		// create the new average NINT
		new_best_offset[byte] = divide_nint(new_best_offset[byte],
						    NUM_BYTE_PATTERNS);

		// print the best offsets from all patterns

		// print just the offset of all the bytes
		if (bytelane == 0x0A)
			debug("%4d ", new_best_offset[byte]);
		else		// print the bytelanes also
			debug("(byte %d) %4d ", byte, new_best_offset[byte]);

		// done with testing, load up the best offsets we found...
		// disable offsets while we load...
		change_dll_offset_enable(priv, lmc, 0);
		load_dll_offset(priv, lmc, dll_offset_mode,
				new_best_offset[byte], byte);
		// re-enable the offsets now that we are done loading
		change_dll_offset_enable(priv, lmc, 1);
	}

	debug("\n");
}

/*
 * Automatically adjust the DLL offset for the selected bytelane using
 * hardware-assist
 */
static int perform_HW_dll_offset_tuning(struct ddr_priv *priv,
					int dll_offset_mode, int bytelane)
{
	int if_64b;
	int save_ecc_ena[4];
	union cvmx_lmcx_config lmc_config;
	int lmc, num_lmcs = cvmx_dram_get_num_lmc(priv);
	const char *s;
	int loops = 1, loop;
	int by;
	u64 dram_tune_rank_offset;
	int dram_tune_byte_bursts = DEFAULT_BYTE_BURSTS;
	int node = 0;

	// see if we want to do the tuning more than once per LMC...
	s = env_get("ddr_tune_ecc_loops");
	if (s)
		loops = strtoul(s, NULL, 0);

	// allow override of the test repeats (bursts)
	s = env_get("ddr_tune_byte_bursts");
	if (s)
		dram_tune_byte_bursts = strtoul(s, NULL, 10);

	// print current working values
	debug("N%d: H/W Tuning for bytelane %d will use %d loops, %d bursts, and %d patterns.\n",
	      node, bytelane, loops, dram_tune_byte_bursts, NUM_BYTE_PATTERNS);

	// FIXME? get flag from LMC0 only
	lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(0));
	if_64b = !lmc_config.s.mode32b;

	// this should be correct for 1 or 2 ranks, 1 or 2 DIMMs
	dram_tune_rank_offset =
	    1ull << (28 + lmc_config.s.pbank_lsb - lmc_config.s.rank_ena +
		     (num_lmcs / 2));

	// do once for each active LMC

	for (lmc = 0; lmc < num_lmcs; lmc++) {
		debug("N%d: H/W Tuning: starting LMC%d bytelane %d tune.\n",
		      node, lmc, bytelane);

		/* Enable ECC for the HW tests */
		// NOTE: we do enable ECC, but the HW tests used will not
		// generate "visible" errors
		lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(lmc));
		save_ecc_ena[lmc] = lmc_config.s.ecc_ena;
		lmc_config.s.ecc_ena = 1;
		lmc_wr(priv, CVMX_LMCX_CONFIG(lmc), lmc_config.u64);
		lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(lmc));

		// testing is done on a single LMC at a time
		// FIXME: for now, loop here to show what happens multiple times
		for (loop = 0; loop < loops; loop++) {
			/* Perform DLL offset tuning */
			hw_assist_test_dll_offset(priv, 2 /* 2=read */, lmc,
						  bytelane,
						  if_64b, dram_tune_rank_offset,
						  dram_tune_byte_bursts);
		}

		// perform cleanup on active LMC
		debug("N%d: H/W Tuning: finishing LMC%d bytelane %d tune.\n",
		      node, lmc, bytelane);

		/* Restore ECC for DRAM tests */
		lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(lmc));
		lmc_config.s.ecc_ena = save_ecc_ena[lmc];
		lmc_wr(priv, CVMX_LMCX_CONFIG(lmc), lmc_config.u64);
		lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(lmc));

		// finally, see if there are any read offset overrides
		// after tuning
		for (by = 0; by < 9; by++) {
			s = lookup_env(priv, "ddr%d_tune_byte%d", lmc, by);
			if (s) {
				int dllro = strtoul(s, NULL, 10);

				change_dll_offset_enable(priv, lmc, 0);
				load_dll_offset(priv, lmc, 2, dllro, by);
				change_dll_offset_enable(priv, lmc, 1);
			}
		}

	}			/* for (lmc = 0; lmc < num_lmcs; lmc++) */

	// finish up...

	return 0;

}				/* perform_HW_dll_offset_tuning */

// this routine simply makes the calls to the tuning routine and returns
// any errors
static int cvmx_tune_node(struct ddr_priv *priv)
{
	int errs, tot_errs;
	int do_dllwo = 0;	// default to NO
	const char *str;
	int node = 0;

	// Automatically tune the data and ECC byte DLL read offsets
	debug("N%d: Starting DLL Read Offset Tuning for LMCs\n", node);
	errs = perform_HW_dll_offset_tuning(priv, 2, 0x0A /* all bytelanes */);
	debug("N%d: Finished DLL Read Offset Tuning for LMCs, %d errors\n",
	      node, errs);
	tot_errs = errs;

	// disabled by default for now, does not seem to be needed?
	// Automatically tune the data and ECC byte DLL write offsets
	// allow override of default setting
	str = env_get("ddr_tune_write_offsets");
	if (str)
		do_dllwo = !!strtoul(str, NULL, 0);
	if (do_dllwo) {
		debug("N%d: Starting DLL Write Offset Tuning for LMCs\n", node);
		errs =
		    perform_HW_dll_offset_tuning(priv, 1,
						 0x0A /* all bytelanes */);
		debug("N%d: Finished DLL Write Offset Tuning for LMCs, %d errors\n",
		      node, errs);
		tot_errs += errs;
	}

	return tot_errs;
}

// this routine makes the calls to the tuning routines when criteria are met
// intended to be called for automated tuning, to apply filtering...

#define IS_DDR4  1
#define IS_DDR3  0
#define IS_RDIMM 1
#define IS_UDIMM 0
#define IS_1SLOT 1
#define IS_2SLOT 0

// FIXME: DDR3 is not tuned
static const u32 ddr_speed_filter[2][2][2] = {
	[IS_DDR4] = {
		     [IS_RDIMM] = {
				   [IS_1SLOT] = 940,
				   [IS_2SLOT] = 800},
		     [IS_UDIMM] = {
				   [IS_1SLOT] = 1050,
				   [IS_2SLOT] = 940},
		      },
	[IS_DDR3] = {
		     [IS_RDIMM] = {
				   [IS_1SLOT] = 0,	// disabled
				   [IS_2SLOT] = 0	// disabled
				   },
		     [IS_UDIMM] = {
				   [IS_1SLOT] = 0,	// disabled
				   [IS_2SLOT] = 0	// disabled
				}
		}
};

void cvmx_maybe_tune_node(struct ddr_priv *priv, u32 ddr_speed)
{
	const char *s;
	union cvmx_lmcx_config lmc_config;
	union cvmx_lmcx_control lmc_control;
	union cvmx_lmcx_ddr_pll_ctl lmc_ddr_pll_ctl;
	int is_ddr4;
	int is_rdimm;
	int is_1slot;
	int do_tune = 0;
	u32 ddr_min_speed;
	int node = 0;

	// scale it down from Hz to MHz
	ddr_speed = divide_nint(ddr_speed, 1000000);

	// FIXME: allow an override here so that all configs can be tuned
	// or none
	// If the envvar is defined, always either force it or avoid it
	// accordingly
	s = env_get("ddr_tune_all_configs");
	if (s) {
		do_tune = !!strtoul(s, NULL, 0);
		printf("N%d: DRAM auto-tuning %s.\n", node,
		       (do_tune) ? "forced" : "disabled");
		if (do_tune)
			cvmx_tune_node(priv);

		return;
	}

	// filter the tuning calls here...
	// determine if we should/can run automatically for this configuration
	//
	// FIXME: tune only when the configuration indicates it will help:
	//    DDR type, RDIMM or UDIMM, 1-slot or 2-slot, and speed
	//
	lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(0));	// sample LMC0
	lmc_control.u64 = lmc_rd(priv, CVMX_LMCX_CONTROL(0));	// sample LMC0
	// sample LMC0
	lmc_ddr_pll_ctl.u64 = lmc_rd(priv, CVMX_LMCX_DDR_PLL_CTL(0));

	is_ddr4 = (lmc_ddr_pll_ctl.s.ddr4_mode != 0);
	is_rdimm = (lmc_control.s.rdimm_ena != 0);
	// HACK, should do better
	is_1slot = (lmc_config.s.init_status < 4);

	ddr_min_speed = ddr_speed_filter[is_ddr4][is_rdimm][is_1slot];
	do_tune = ((ddr_min_speed != 0) && (ddr_speed > ddr_min_speed));

	debug("N%d: DDR%d %cDIMM %d-slot at %d MHz %s eligible for auto-tuning.\n",
	      node, (is_ddr4) ? 4 : 3, (is_rdimm) ? 'R' : 'U',
	      (is_1slot) ? 1 : 2, ddr_speed, (do_tune) ? "is" : "is not");

	// call the tuning routine, filtering is done...
	if (do_tune)
		cvmx_tune_node(priv);
}

/*
 * first pattern example:
 * GENERAL_PURPOSE0.DATA == 64'h00ff00ff00ff00ff;
 * GENERAL_PURPOSE1.DATA == 64'h00ff00ff00ff00ff;
 * GENERAL_PURPOSE0.DATA == 16'h0000;
 */

static const u64 dbi_pattern[3] = {
	0x00ff00ff00ff00ffULL, 0x00ff00ff00ff00ffULL, 0x0000ULL };

// Perform switchover to DBI
static void cvmx_dbi_switchover_interface(struct ddr_priv *priv, int lmc)
{
	union cvmx_lmcx_modereg_params0 modereg_params0;
	union cvmx_lmcx_modereg_params3 modereg_params3;
	union cvmx_lmcx_phy_ctl phy_ctl;
	union cvmx_lmcx_config lmcx_config;
	union cvmx_lmcx_ddr_pll_ctl ddr_pll_ctl;
	int rank_mask, rankx, active_ranks;
	u64 phys_addr, rank_offset;
	int num_lmcs, errors;
	int dbi_settings[9], byte, unlocked, retries;
	int ecc_ena;
	int rank_max = 1;	// FIXME: make this 4 to try all the ranks
	int node = 0;

	ddr_pll_ctl.u64 = lmc_rd(priv, CVMX_LMCX_DDR_PLL_CTL(0));

	lmcx_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(lmc));
	rank_mask = lmcx_config.s.init_status;
	ecc_ena = lmcx_config.s.ecc_ena;

	// FIXME: must filter out any non-supported configs
	//        ie, no DDR3, no x4 devices
	if (ddr_pll_ctl.s.ddr4_mode == 0 || lmcx_config.s.mode_x4dev == 1) {
		debug("N%d.LMC%d: DBI switchover: inappropriate device; EXITING...\n",
		      node, lmc);
		return;
	}

	// this should be correct for 1 or 2 ranks, 1 or 2 DIMMs
	num_lmcs = cvmx_dram_get_num_lmc(priv);
	rank_offset = 1ull << (28 + lmcx_config.s.pbank_lsb -
			       lmcx_config.s.rank_ena + (num_lmcs / 2));

	debug("N%d.LMC%d: DBI switchover: rank mask 0x%x, rank size 0x%016llx.\n",
	      node, lmc, rank_mask, (unsigned long long)rank_offset);

	/*
	 * 1. conduct the current init sequence as usual all the way
	 * after software write leveling.
	 */

	read_dac_dbi_settings(priv, lmc, /*DBI*/ 0, dbi_settings);

	display_dac_dbi_settings(lmc, /*DBI*/ 0, ecc_ena, dbi_settings,
				 " INIT");

	/*
	 * 2. set DBI related CSRs as below and issue MR write.
	 * MODEREG_PARAMS3.WR_DBI=1
	 * MODEREG_PARAMS3.RD_DBI=1
	 * PHY_CTL.DBI_MODE_ENA=1
	 */
	modereg_params0.u64 = lmc_rd(priv, CVMX_LMCX_MODEREG_PARAMS0(lmc));

	modereg_params3.u64 = lmc_rd(priv, CVMX_LMCX_MODEREG_PARAMS3(lmc));
	modereg_params3.s.wr_dbi = 1;
	modereg_params3.s.rd_dbi = 1;
	lmc_wr(priv, CVMX_LMCX_MODEREG_PARAMS3(lmc), modereg_params3.u64);

	phy_ctl.u64 = lmc_rd(priv, CVMX_LMCX_PHY_CTL(lmc));
	phy_ctl.s.dbi_mode_ena = 1;
	lmc_wr(priv, CVMX_LMCX_PHY_CTL(lmc), phy_ctl.u64);

	/*
	 * there are two options for data to send.  Lets start with (1)
	 * and could move to (2) in the future:
	 *
	 * 1) DBTRAIN_CTL[LFSR_PATTERN_SEL] = 0 (or for older chips where
	 * this does not exist) set data directly in these reigsters.
	 * this will yield a clk/2 pattern:
	 * GENERAL_PURPOSE0.DATA == 64'h00ff00ff00ff00ff;
	 * GENERAL_PURPOSE1.DATA == 64'h00ff00ff00ff00ff;
	 * GENERAL_PURPOSE0.DATA == 16'h0000;
	 * 2) DBTRAIN_CTL[LFSR_PATTERN_SEL] = 1
	 * here data comes from the LFSR generating a PRBS pattern
	 * CHAR_CTL.EN = 0
	 * CHAR_CTL.SEL = 0; // for PRBS
	 * CHAR_CTL.DR = 1;
	 * CHAR_CTL.PRBS = setup for whatever type of PRBS to send
	 * CHAR_CTL.SKEW_ON = 1;
	 */
	lmc_wr(priv, CVMX_LMCX_GENERAL_PURPOSE0(lmc), dbi_pattern[0]);
	lmc_wr(priv, CVMX_LMCX_GENERAL_PURPOSE1(lmc), dbi_pattern[1]);
	lmc_wr(priv, CVMX_LMCX_GENERAL_PURPOSE2(lmc), dbi_pattern[2]);

	/*
	 * 3. adjust cas_latency (only necessary if RD_DBI is set).
	 * here is my code for doing this:
	 *
	 * if (csr_model.MODEREG_PARAMS3.RD_DBI.value == 1) begin
	 * case (csr_model.MODEREG_PARAMS0.CL.value)
	 * 0,1,2,3,4: csr_model.MODEREG_PARAMS0.CL.value += 2;
	 * // CL 9-13 -> 11-15
	 * 5: begin
	 * // CL=14, CWL=10,12 gets +2, CLW=11,14 gets +3
	 * if((csr_model.MODEREG_PARAMS0.CWL.value==1 ||
	 * csr_model.MODEREG_PARAMS0.CWL.value==3))
	 * csr_model.MODEREG_PARAMS0.CL.value = 7; // 14->16
	 * else
	 * csr_model.MODEREG_PARAMS0.CL.value = 13; // 14->17
	 * end
	 * 6: csr_model.MODEREG_PARAMS0.CL.value = 8; // 15->18
	 * 7: csr_model.MODEREG_PARAMS0.CL.value = 14; // 16->19
	 * 8: csr_model.MODEREG_PARAMS0.CL.value = 15; // 18->21
	 * default:
	 * `cn_fatal(("Error mem_cfg (%s) CL (%d) with RD_DBI=1,
	 * I am not sure what to do.",
	 * mem_cfg, csr_model.MODEREG_PARAMS3.RD_DBI.value))
	 * endcase
	 * end
	 */

	if (modereg_params3.s.rd_dbi == 1) {
		int old_cl, new_cl, old_cwl;

		old_cl = modereg_params0.s.cl;
		old_cwl = modereg_params0.s.cwl;

		switch (old_cl) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			new_cl = old_cl + 2;
			break;	// 9-13->11-15
			// CL=14, CWL=10,12 gets +2, CLW=11,14 gets +3
		case 5:
			new_cl = ((old_cwl == 1) || (old_cwl == 3)) ? 7 : 13;
			break;
		case 6:
			new_cl = 8;
			break;	// 15->18
		case 7:
			new_cl = 14;
			break;	// 16->19
		case 8:
			new_cl = 15;
			break;	// 18->21
		default:
			printf("ERROR: Bad CL value (%d) for DBI switchover.\n",
			       old_cl);
			// FIXME: need to error exit here...
			old_cl = -1;
			new_cl = -1;
			break;
		}
		debug("N%d.LMC%d: DBI switchover: CL ADJ: old_cl 0x%x, old_cwl 0x%x, new_cl 0x%x.\n",
		      node, lmc, old_cl, old_cwl, new_cl);
		modereg_params0.s.cl = new_cl;
		lmc_wr(priv, CVMX_LMCX_MODEREG_PARAMS0(lmc),
		       modereg_params0.u64);
	}

	/*
	 * 4. issue MRW to MR0 (CL) and MR5 (DBI), using LMC sequence
	 * SEQ_CTL[SEQ_SEL] = MRW.
	 */
	// Use the default values, from the CSRs fields
	// also, do B-sides for RDIMMs...

	for (rankx = 0; rankx < 4; rankx++) {
		if (!(rank_mask & (1 << rankx)))
			continue;

		// for RDIMMs, B-side writes should get done automatically
		// when the A-side is written
		ddr4_mrw(priv, lmc, rankx, -1 /* use_default */,
			 0 /*MRreg */, 0 /*A-side */);	/* MR0 */
		ddr4_mrw(priv, lmc, rankx, -1 /* use_default */,
			 5 /*MRreg */, 0 /*A-side */);	/* MR5 */
	}

	/*
	 * 5. conduct DBI bit deskew training via the General Purpose
	 * R/W sequence (dbtrain). may need to run this over and over to get
	 * a lock (I need up to 5 in simulation):
	 * SEQ_CTL[SEQ_SEL] = RW_TRAINING (15)
	 * DBTRAIN_CTL.CMD_COUNT_EXT = all 1's
	 * DBTRAIN_CTL.READ_CMD_COUNT = all 1's
	 * DBTRAIN_CTL.TCCD_SEL = set according to MODEREG_PARAMS3[TCCD_L]
	 * DBTRAIN_CTL.RW_TRAIN = 1
	 * DBTRAIN_CTL.READ_DQ_COUNT = dont care
	 * DBTRAIN_CTL.WRITE_ENA = 1;
	 * DBTRAIN_CTL.ACTIVATE = 1;
	 * DBTRAIN_CTL LRANK, PRANK, ROW_A, BG, BA, COLUMN_A = set to a
	 * valid address
	 */

	// NOW - do the training
	debug("N%d.LMC%d: DBI switchover: TRAINING begins...\n", node, lmc);

	active_ranks = 0;
	for (rankx = 0; rankx < rank_max; rankx++) {
		if (!(rank_mask & (1 << rankx)))
			continue;

		phys_addr = rank_offset * active_ranks;
		// FIXME: now done by test_dram_byte_hw()

		active_ranks++;

		retries = 0;

restart_training:

		// NOTE: return is a bitmask of the erroring bytelanes -
		// we only print it
		errors =
		    test_dram_byte_hw(priv, lmc, phys_addr, DBTRAIN_DBI, NULL);

		debug("N%d.LMC%d: DBI switchover: TEST: rank %d, phys_addr 0x%llx, errors 0x%x.\n",
		      node, lmc, rankx, (unsigned long long)phys_addr, errors);

		// NEXT - check for locking
		unlocked = 0;
		read_dac_dbi_settings(priv, lmc, /*DBI*/ 0, dbi_settings);

		for (byte = 0; byte < (8 + ecc_ena); byte++)
			unlocked += (dbi_settings[byte] & 1) ^ 1;

		// FIXME: print out the DBI settings array after each rank?
		if (rank_max > 1)	// only when doing more than 1 rank
			display_dac_dbi_settings(lmc, /*DBI*/ 0, ecc_ena,
						 dbi_settings, " RANK");

		if (unlocked > 0) {
			debug("N%d.LMC%d: DBI switchover: LOCK: %d still unlocked.\n",
			      node, lmc, unlocked);
			retries++;
			if (retries < 10) {
				goto restart_training;
			} else {
				debug("N%d.LMC%d: DBI switchover: LOCK: %d retries exhausted.\n",
				      node, lmc, retries);
			}
		}
	}			/* for (rankx = 0; rankx < 4; rankx++) */

	// print out the final DBI settings array
	display_dac_dbi_settings(lmc, /*DBI*/ 0, ecc_ena, dbi_settings,
				 "FINAL");
}

void cvmx_dbi_switchover(struct ddr_priv *priv)
{
	int lmc;
	int num_lmcs = cvmx_dram_get_num_lmc(priv);

	for (lmc = 0; lmc < num_lmcs; lmc++)
		cvmx_dbi_switchover_interface(priv, lmc);
}
