// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008, 2010-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2018 NXP Semiconductor
 */

#include <common.h>
#include <hwconfig.h>
#include <fsl_ddr_sdram.h>

#include <fsl_ddr.h>
#if defined(CONFIG_FSL_LSCH2) || defined(CONFIG_FSL_LSCH3) || \
	defined(CONFIG_ARM)
#include <asm/arch/clock.h>
#endif

/*
 * Use our own stack based buffer before relocation to allow accessing longer
 * hwconfig strings that might be in the environment before we've relocated.
 * This is pretty fragile on both the use of stack and if the buffer is big
 * enough. However we will get a warning from env_get_f() for the latter.
 */

/* Board-specific functions defined in each board's ddr.c */
void __weak fsl_ddr_board_options(memctl_options_t *popts,
				  dimm_params_t *pdimm,
				  unsigned int ctrl_num)
{
	return;
}

struct dynamic_odt {
	unsigned int odt_rd_cfg;
	unsigned int odt_wr_cfg;
	unsigned int odt_rtt_norm;
	unsigned int odt_rtt_wr;
};

#ifdef CONFIG_SYS_FSL_DDR4
/* Quad rank is not verified yet due availability.
 * Replacing 20 OHM with 34 OHM since DDR4 doesn't have 20 OHM option
 */
static __maybe_unused const struct dynamic_odt single_Q[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS_AND_OTHER_DIMM,
		DDR4_RTT_34_OHM,	/* unverified */
		DDR4_RTT_120_OHM
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR4_RTT_OFF,
		DDR4_RTT_120_OHM
	},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS_AND_OTHER_DIMM,
		DDR4_RTT_34_OHM,
		DDR4_RTT_120_OHM
	},
	{	/* cs3 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,	/* tied high */
		DDR4_RTT_OFF,
		DDR4_RTT_120_OHM
	}
};

static __maybe_unused const struct dynamic_odt single_D[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_ALL,
		DDR4_RTT_40_OHM,
		DDR4_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR4_RTT_OFF,
		DDR4_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

static __maybe_unused const struct dynamic_odt single_S[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_ALL,
		DDR4_RTT_40_OHM,
		DDR4_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
};

static __maybe_unused const struct dynamic_odt dual_DD[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR4_RTT_120_OHM,
		DDR4_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR4_RTT_34_OHM,
		DDR4_RTT_OFF
	},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR4_RTT_120_OHM,
		DDR4_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR4_RTT_34_OHM,
		DDR4_RTT_OFF
	}
};

static __maybe_unused const struct dynamic_odt dual_DS[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR4_RTT_120_OHM,
		DDR4_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR4_RTT_34_OHM,
		DDR4_RTT_OFF
	},
	{	/* cs2 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_ALL,
		DDR4_RTT_34_OHM,
		DDR4_RTT_120_OHM
	},
	{0, 0, 0, 0}
};
static __maybe_unused const struct dynamic_odt dual_SD[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_ALL,
		DDR4_RTT_34_OHM,
		DDR4_RTT_120_OHM
	},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR4_RTT_120_OHM,
		DDR4_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR4_RTT_34_OHM,
		DDR4_RTT_OFF
	}
};

static __maybe_unused const struct dynamic_odt dual_SS[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_ALL,
		DDR4_RTT_34_OHM,
		DDR4_RTT_120_OHM
	},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_ALL,
		DDR4_RTT_34_OHM,
		DDR4_RTT_120_OHM
	},
	{0, 0, 0, 0}
};

static __maybe_unused const struct dynamic_odt dual_D0[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR4_RTT_40_OHM,
		DDR4_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR4_RTT_OFF,
		DDR4_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

static __maybe_unused const struct dynamic_odt dual_0D[4] = {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR4_RTT_40_OHM,
		DDR4_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR4_RTT_OFF,
		DDR4_RTT_OFF
	}
};

static __maybe_unused const struct dynamic_odt dual_S0[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR4_RTT_40_OHM,
		DDR4_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}

};

static __maybe_unused const struct dynamic_odt dual_0S[4] = {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR4_RTT_40_OHM,
		DDR4_RTT_OFF
	},
	{0, 0, 0, 0}

};

static __maybe_unused const struct dynamic_odt odt_unknown[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR4_RTT_120_OHM,
		DDR4_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR4_RTT_120_OHM,
		DDR4_RTT_OFF
	},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR4_RTT_120_OHM,
		DDR4_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR4_RTT_120_OHM,
		DDR4_RTT_OFF
	}
};
#elif defined(CONFIG_SYS_FSL_DDR3)
static __maybe_unused const struct dynamic_odt single_Q[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS_AND_OTHER_DIMM,
		DDR3_RTT_20_OHM,
		DDR3_RTT_120_OHM
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,	/* tied high */
		DDR3_RTT_OFF,
		DDR3_RTT_120_OHM
	},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS_AND_OTHER_DIMM,
		DDR3_RTT_20_OHM,
		DDR3_RTT_120_OHM
	},
	{	/* cs3 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,	/* tied high */
		DDR3_RTT_OFF,
		DDR3_RTT_120_OHM
	}
};

static __maybe_unused const struct dynamic_odt single_D[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_ALL,
		DDR3_RTT_40_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR3_RTT_OFF,
		DDR3_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

static __maybe_unused const struct dynamic_odt single_S[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_ALL,
		DDR3_RTT_40_OHM,
		DDR3_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
};

static __maybe_unused const struct dynamic_odt dual_DD[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR3_RTT_30_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR3_RTT_30_OHM,
		DDR3_RTT_OFF
	}
};

static __maybe_unused const struct dynamic_odt dual_DS[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR3_RTT_30_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs2 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_ALL,
		DDR3_RTT_20_OHM,
		DDR3_RTT_120_OHM
	},
	{0, 0, 0, 0}
};
static __maybe_unused const struct dynamic_odt dual_SD[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_ALL,
		DDR3_RTT_20_OHM,
		DDR3_RTT_120_OHM
	},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR3_RTT_20_OHM,
		DDR3_RTT_OFF
	}
};

static __maybe_unused const struct dynamic_odt dual_SS[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_ALL,
		DDR3_RTT_30_OHM,
		DDR3_RTT_120_OHM
	},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_ALL,
		DDR3_RTT_30_OHM,
		DDR3_RTT_120_OHM
	},
	{0, 0, 0, 0}
};

static __maybe_unused const struct dynamic_odt dual_D0[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR3_RTT_40_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR3_RTT_OFF,
		DDR3_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

static __maybe_unused const struct dynamic_odt dual_0D[4] = {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR3_RTT_40_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR3_RTT_OFF,
		DDR3_RTT_OFF
	}
};

static __maybe_unused const struct dynamic_odt dual_S0[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR3_RTT_40_OHM,
		DDR3_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}

};

static __maybe_unused const struct dynamic_odt dual_0S[4] = {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR3_RTT_40_OHM,
		DDR3_RTT_OFF
	},
	{0, 0, 0, 0}

};

static __maybe_unused const struct dynamic_odt odt_unknown[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	}
};
#else	/* CONFIG_SYS_FSL_DDR3 */
static __maybe_unused const struct dynamic_odt single_Q[4] = {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

static __maybe_unused const struct dynamic_odt single_D[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_ALL,
		DDR2_RTT_150_OHM,
		DDR2_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR2_RTT_OFF,
		DDR2_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

static __maybe_unused const struct dynamic_odt single_S[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_ALL,
		DDR2_RTT_150_OHM,
		DDR2_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
};

static __maybe_unused const struct dynamic_odt dual_DD[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR2_RTT_75_OHM,
		DDR2_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR2_RTT_OFF,
		DDR2_RTT_OFF
	},
	{	/* cs2 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR2_RTT_75_OHM,
		DDR2_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR2_RTT_OFF,
		DDR2_RTT_OFF
	}
};

static __maybe_unused const struct dynamic_odt dual_DS[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR2_RTT_75_OHM,
		DDR2_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR2_RTT_OFF,
		DDR2_RTT_OFF
	},
	{	/* cs2 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR2_RTT_75_OHM,
		DDR2_RTT_OFF
	},
	{0, 0, 0, 0}
};

static __maybe_unused const struct dynamic_odt dual_SD[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR2_RTT_75_OHM,
		DDR2_RTT_OFF
	},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR2_RTT_75_OHM,
		DDR2_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR2_RTT_OFF,
		DDR2_RTT_OFF
	}
};

static __maybe_unused const struct dynamic_odt dual_SS[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR2_RTT_75_OHM,
		DDR2_RTT_OFF
	},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR2_RTT_75_OHM,
		DDR2_RTT_OFF
	},
	{0, 0, 0, 0}
};

static __maybe_unused const struct dynamic_odt dual_D0[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_ALL,
		DDR2_RTT_150_OHM,
		DDR2_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR2_RTT_OFF,
		DDR2_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

static __maybe_unused const struct dynamic_odt dual_0D[4] = {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_ALL,
		DDR2_RTT_150_OHM,
		DDR2_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR2_RTT_OFF,
		DDR2_RTT_OFF
	}
};

static __maybe_unused const struct dynamic_odt dual_S0[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR2_RTT_150_OHM,
		DDR2_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}

};

static __maybe_unused const struct dynamic_odt dual_0S[4] = {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR2_RTT_150_OHM,
		DDR2_RTT_OFF
	},
	{0, 0, 0, 0}

};

static __maybe_unused const struct dynamic_odt odt_unknown[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR2_RTT_75_OHM,
		DDR2_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR2_RTT_OFF,
		DDR2_RTT_OFF
	},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR2_RTT_75_OHM,
		DDR2_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR2_RTT_OFF,
		DDR2_RTT_OFF
	}
};
#endif

/*
 * Automatically seleect bank interleaving mode based on DIMMs
 * in this order: cs0_cs1_cs2_cs3, cs0_cs1, null.
 * This function only deal with one or two slots per controller.
 */
static inline unsigned int auto_bank_intlv(dimm_params_t *pdimm)
{
#if (CONFIG_DIMM_SLOTS_PER_CTLR == 1)
	if (pdimm[0].n_ranks == 4)
		return FSL_DDR_CS0_CS1_CS2_CS3;
	else if (pdimm[0].n_ranks == 2)
		return FSL_DDR_CS0_CS1;
#elif (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
#ifdef CONFIG_FSL_DDR_FIRST_SLOT_QUAD_CAPABLE
	if (pdimm[0].n_ranks == 4)
		return FSL_DDR_CS0_CS1_CS2_CS3;
#endif
	if (pdimm[0].n_ranks == 2) {
		if (pdimm[1].n_ranks == 2)
			return FSL_DDR_CS0_CS1_CS2_CS3;
		else
			return FSL_DDR_CS0_CS1;
	}
#endif
	return 0;
}

unsigned int populate_memctl_options(const common_timing_params_t *common_dimm,
			memctl_options_t *popts,
			dimm_params_t *pdimm,
			unsigned int ctrl_num)
{
	unsigned int i;
	char buf[HWCONFIG_BUFFER_SIZE];
#if defined(CONFIG_SYS_FSL_DDR3) || \
	defined(CONFIG_SYS_FSL_DDR2) || \
	defined(CONFIG_SYS_FSL_DDR4)
	const struct dynamic_odt *pdodt = odt_unknown;
#endif
#if (CONFIG_FSL_SDRAM_TYPE != SDRAM_TYPE_DDR4)
	ulong ddr_freq;
#endif

	/*
	 * Extract hwconfig from environment since we have not properly setup
	 * the environment but need it for ddr config params
	 */
	if (env_get_f("hwconfig", buf, sizeof(buf)) < 0)
		buf[0] = '\0';

#if defined(CONFIG_SYS_FSL_DDR3) || \
	defined(CONFIG_SYS_FSL_DDR2) || \
	defined(CONFIG_SYS_FSL_DDR4)
	/* Chip select options. */
#if (CONFIG_DIMM_SLOTS_PER_CTLR == 1)
	switch (pdimm[0].n_ranks) {
	case 1:
		pdodt = single_S;
		break;
	case 2:
		pdodt = single_D;
		break;
	case 4:
		pdodt = single_Q;
		break;
	}
#elif (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
	switch (pdimm[0].n_ranks) {
#ifdef CONFIG_FSL_DDR_FIRST_SLOT_QUAD_CAPABLE
	case 4:
		pdodt = single_Q;
		if (pdimm[1].n_ranks)
			printf("Error: Quad- and Dual-rank DIMMs cannot be used together\n");
		break;
#endif
	case 2:
		switch (pdimm[1].n_ranks) {
		case 2:
			pdodt = dual_DD;
			break;
		case 1:
			pdodt = dual_DS;
			break;
		case 0:
			pdodt = dual_D0;
			break;
		}
		break;
	case 1:
		switch (pdimm[1].n_ranks) {
		case 2:
			pdodt = dual_SD;
			break;
		case 1:
			pdodt = dual_SS;
			break;
		case 0:
			pdodt = dual_S0;
			break;
		}
		break;
	case 0:
		switch (pdimm[1].n_ranks) {
		case 2:
			pdodt = dual_0D;
			break;
		case 1:
			pdodt = dual_0S;
			break;
		}
		break;
	}
#endif	/* CONFIG_DIMM_SLOTS_PER_CTLR */
#endif	/* CONFIG_SYS_FSL_DDR2, 3, 4 */

	/* Pick chip-select local options. */
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
#if defined(CONFIG_SYS_FSL_DDR3) || \
	defined(CONFIG_SYS_FSL_DDR2) || \
	defined(CONFIG_SYS_FSL_DDR4)
		popts->cs_local_opts[i].odt_rd_cfg = pdodt[i].odt_rd_cfg;
		popts->cs_local_opts[i].odt_wr_cfg = pdodt[i].odt_wr_cfg;
		popts->cs_local_opts[i].odt_rtt_norm = pdodt[i].odt_rtt_norm;
		popts->cs_local_opts[i].odt_rtt_wr = pdodt[i].odt_rtt_wr;
#else
		popts->cs_local_opts[i].odt_rd_cfg = FSL_DDR_ODT_NEVER;
		popts->cs_local_opts[i].odt_wr_cfg = FSL_DDR_ODT_CS;
#endif
		popts->cs_local_opts[i].auto_precharge = 0;
	}

	/* Pick interleaving mode. */

	/*
	 * 0 = no interleaving
	 * 1 = interleaving between 2 controllers
	 */
	popts->memctl_interleaving = 0;

	/*
	 * 0 = cacheline
	 * 1 = page
	 * 2 = (logical) bank
	 * 3 = superbank (only if CS interleaving is enabled)
	 */
	popts->memctl_interleaving_mode = 0;

	/*
	 * 0: cacheline: bit 30 of the 36-bit physical addr selects the memctl
	 * 1: page:      bit to the left of the column bits selects the memctl
	 * 2: bank:      bit to the left of the bank bits selects the memctl
	 * 3: superbank: bit to the left of the chip select selects the memctl
	 *
	 * NOTE: ba_intlv (rank interleaving) is independent of memory
	 * controller interleaving; it is only within a memory controller.
	 * Must use superbank interleaving if rank interleaving is used and
	 * memory controller interleaving is enabled.
	 */

	/*
	 * 0 = no
	 * 0x40 = CS0,CS1
	 * 0x20 = CS2,CS3
	 * 0x60 = CS0,CS1 + CS2,CS3
	 * 0x04 = CS0,CS1,CS2,CS3
	 */
	popts->ba_intlv_ctl = 0;

	/* Memory Organization Parameters */
	popts->registered_dimm_en = common_dimm->all_dimms_registered;

	/* Operational Mode Paramters */

	/* Pick ECC modes */
	popts->ecc_mode = 0;		  /* 0 = disabled, 1 = enabled */
#ifdef CONFIG_DDR_ECC
	if (hwconfig_sub_f("fsl_ddr", "ecc", buf)) {
		if (hwconfig_subarg_cmp_f("fsl_ddr", "ecc", "on", buf))
			popts->ecc_mode = 1;
	} else
		popts->ecc_mode = 1;
#endif
	/* 1 = use memory controler to init data */
	popts->ecc_init_using_memctl = popts->ecc_mode ? 1 : 0;

	/*
	 * Choose DQS config
	 * 0 for DDR1
	 * 1 for DDR2
	 */
#if defined(CONFIG_SYS_FSL_DDR1)
	popts->dqs_config = 0;
#elif defined(CONFIG_SYS_FSL_DDR2) || defined(CONFIG_SYS_FSL_DDR3)
	popts->dqs_config = 1;
#endif

	/* Choose self-refresh during sleep. */
	popts->self_refresh_in_sleep = 1;

	/* Choose dynamic power management mode. */
	popts->dynamic_power = 0;

	/*
	 * check first dimm for primary sdram width
	 * presuming all dimms are similar
	 * 0 = 64-bit, 1 = 32-bit, 2 = 16-bit
	 */
#if defined(CONFIG_SYS_FSL_DDR1) || defined(CONFIG_SYS_FSL_DDR2)
	if (pdimm[0].n_ranks != 0) {
		if ((pdimm[0].data_width >= 64) && \
			(pdimm[0].data_width <= 72))
			popts->data_bus_width = 0;
		else if ((pdimm[0].data_width >= 32) && \
			(pdimm[0].data_width <= 40))
			popts->data_bus_width = 1;
		else {
			panic("Error: data width %u is invalid!\n",
				pdimm[0].data_width);
		}
	}
#else
	if (pdimm[0].n_ranks != 0) {
		if (pdimm[0].primary_sdram_width == 64)
			popts->data_bus_width = 0;
		else if (pdimm[0].primary_sdram_width == 32)
			popts->data_bus_width = 1;
		else if (pdimm[0].primary_sdram_width == 16)
			popts->data_bus_width = 2;
		else {
			panic("Error: primary sdram width %u is invalid!\n",
				pdimm[0].primary_sdram_width);
		}
	}
#endif

	popts->x4_en = (pdimm[0].device_width == 4) ? 1 : 0;

	/* Choose burst length. */
#if defined(CONFIG_SYS_FSL_DDR3) || defined(CONFIG_SYS_FSL_DDR4)
#if defined(CONFIG_E500MC)
	popts->otf_burst_chop_en = 0;	/* on-the-fly burst chop disable */
	popts->burst_length = DDR_BL8;	/* Fixed 8-beat burst len */
#else
	if ((popts->data_bus_width == 1) || (popts->data_bus_width == 2)) {
		/* 32-bit or 16-bit bus */
		popts->otf_burst_chop_en = 0;
		popts->burst_length = DDR_BL8;
	} else {
		popts->otf_burst_chop_en = 1;	/* on-the-fly burst chop */
		popts->burst_length = DDR_OTF;	/* on-the-fly BC4 and BL8 */
	}
#endif
#else
	popts->burst_length = DDR_BL4;	/* has to be 4 for DDR2 */
#endif

	/* Choose ddr controller address mirror mode */
#if defined(CONFIG_SYS_FSL_DDR3) || defined(CONFIG_SYS_FSL_DDR4)
	for (i = 0; i < CONFIG_DIMM_SLOTS_PER_CTLR; i++) {
		if (pdimm[i].n_ranks) {
			popts->mirrored_dimm = pdimm[i].mirrored_dimm;
			break;
		}
	}
#endif

	/* Global Timing Parameters. */
	debug("mclk_ps = %u ps\n", get_memory_clk_period_ps(ctrl_num));

	/* Pick a caslat override. */
	popts->cas_latency_override = 0;
	popts->cas_latency_override_value = 3;
	if (popts->cas_latency_override) {
		debug("using caslat override value = %u\n",
		       popts->cas_latency_override_value);
	}

	/* Decide whether to use the computed derated latency */
	popts->use_derated_caslat = 0;

	/* Choose an additive latency. */
	popts->additive_latency_override = 0;
	popts->additive_latency_override_value = 3;
	if (popts->additive_latency_override) {
		debug("using additive latency override value = %u\n",
		       popts->additive_latency_override_value);
	}

	/*
	 * 2T_EN setting
	 *
	 * Factors to consider for 2T_EN:
	 *	- number of DIMMs installed
	 *	- number of components, number of active ranks
	 *	- how much time you want to spend playing around
	 */
	popts->twot_en = 0;
	popts->threet_en = 0;

	/* for RDIMM and DDR4 UDIMM/discrete memory, address parity enable */
	if (popts->registered_dimm_en)
		popts->ap_en = 1; /* 0 = disable,  1 = enable */
	else
		popts->ap_en = 0; /* disabled for DDR4 UDIMM/discrete default */

	if (hwconfig_sub_f("fsl_ddr", "parity", buf)) {
		if (hwconfig_subarg_cmp_f("fsl_ddr", "parity", "on", buf)) {
			if (popts->registered_dimm_en ||
			    (CONFIG_FSL_SDRAM_TYPE == SDRAM_TYPE_DDR4))
				popts->ap_en = 1;
		}
	}

	/*
	 * BSTTOPRE precharge interval
	 *
	 * Set this to 0 for global auto precharge
	 * The value of 0x100 has been used for DDR1, DDR2, DDR3.
	 * It is not wrong. Any value should be OK. The performance depends on
	 * applications. There is no one good value for all. One way to set
	 * is to use 1/4 of refint value.
	 */
	popts->bstopre = picos_to_mclk(ctrl_num, common_dimm->refresh_rate_ps)
			 >> 2;

	/*
	 * Window for four activates -- tFAW
	 *
	 * FIXME: UM: applies only to DDR2/DDR3 with eight logical banks only
	 * FIXME: varies depending upon number of column addresses or data
	 * FIXME: width, was considering looking at pdimm->primary_sdram_width
	 */
#if defined(CONFIG_SYS_FSL_DDR1)
	popts->tfaw_window_four_activates_ps = mclk_to_picos(ctrl_num, 1);

#elif defined(CONFIG_SYS_FSL_DDR2)
	/*
	 * x4/x8;  some datasheets have 35000
	 * x16 wide columns only?  Use 50000?
	 */
	popts->tfaw_window_four_activates_ps = 37500;

#else
	popts->tfaw_window_four_activates_ps = pdimm[0].tfaw_ps;
#endif
	popts->zq_en = 0;
	popts->wrlvl_en = 0;
#if defined(CONFIG_SYS_FSL_DDR3) || defined(CONFIG_SYS_FSL_DDR4)
	/*
	 * due to ddr3 dimm is fly-by topology
	 * we suggest to enable write leveling to
	 * meet the tQDSS under different loading.
	 */
	popts->wrlvl_en = 1;
	popts->zq_en = 1;
	popts->wrlvl_override = 0;
#endif

	/*
	 * Check interleaving configuration from environment.
	 * Please refer to doc/README.fsl-ddr for the detail.
	 *
	 * If memory controller interleaving is enabled, then the data
	 * bus widths must be programmed identically for all memory controllers.
	 *
	 * Attempt to set all controllers to the same chip select
	 * interleaving mode. It will do a best effort to get the
	 * requested ranks interleaved together such that the result
	 * should be a subset of the requested configuration.
	 *
	 * if CONFIG_SYS_FSL_DDR_INTLV_256B is defined, mandatory interleaving
	 * with 256 Byte is enabled.
	 */
#if (CONFIG_SYS_NUM_DDR_CTLRS > 1)
	if (!hwconfig_sub_f("fsl_ddr", "ctlr_intlv", buf))
#ifdef CONFIG_SYS_FSL_DDR_INTLV_256B
		;
#else
		goto done;
#endif
	if (pdimm[0].n_ranks == 0) {
		printf("There is no rank on CS0 for controller %d.\n", ctrl_num);
		popts->memctl_interleaving = 0;
		goto done;
	}
	popts->memctl_interleaving = 1;
#ifdef CONFIG_SYS_FSL_DDR_INTLV_256B
	popts->memctl_interleaving_mode = FSL_DDR_256B_INTERLEAVING;
	popts->memctl_interleaving = 1;
	debug("256 Byte interleaving\n");
#else
	/*
	 * test null first. if CONFIG_HWCONFIG is not defined
	 * hwconfig_arg_cmp returns non-zero
	 */
	if (hwconfig_subarg_cmp_f("fsl_ddr", "ctlr_intlv",
				    "null", buf)) {
		popts->memctl_interleaving = 0;
		debug("memory controller interleaving disabled.\n");
	} else if (hwconfig_subarg_cmp_f("fsl_ddr",
					"ctlr_intlv",
					"cacheline", buf)) {
		popts->memctl_interleaving_mode =
			((CONFIG_SYS_NUM_DDR_CTLRS == 3) && ctrl_num == 2) ?
			0 : FSL_DDR_CACHE_LINE_INTERLEAVING;
		popts->memctl_interleaving =
			((CONFIG_SYS_NUM_DDR_CTLRS == 3) && ctrl_num == 2) ?
			0 : 1;
	} else if (hwconfig_subarg_cmp_f("fsl_ddr",
					"ctlr_intlv",
					"page", buf)) {
		popts->memctl_interleaving_mode =
			((CONFIG_SYS_NUM_DDR_CTLRS == 3) && ctrl_num == 2) ?
			0 : FSL_DDR_PAGE_INTERLEAVING;
		popts->memctl_interleaving =
			((CONFIG_SYS_NUM_DDR_CTLRS == 3) && ctrl_num == 2) ?
			0 : 1;
	} else if (hwconfig_subarg_cmp_f("fsl_ddr",
					"ctlr_intlv",
					"bank", buf)) {
		popts->memctl_interleaving_mode =
			((CONFIG_SYS_NUM_DDR_CTLRS == 3) && ctrl_num == 2) ?
			0 : FSL_DDR_BANK_INTERLEAVING;
		popts->memctl_interleaving =
			((CONFIG_SYS_NUM_DDR_CTLRS == 3) && ctrl_num == 2) ?
			0 : 1;
	} else if (hwconfig_subarg_cmp_f("fsl_ddr",
					"ctlr_intlv",
					"superbank", buf)) {
		popts->memctl_interleaving_mode =
			((CONFIG_SYS_NUM_DDR_CTLRS == 3) && ctrl_num == 2) ?
			0 : FSL_DDR_SUPERBANK_INTERLEAVING;
		popts->memctl_interleaving =
			((CONFIG_SYS_NUM_DDR_CTLRS == 3) && ctrl_num == 2) ?
			0 : 1;
#if (CONFIG_SYS_NUM_DDR_CTLRS == 3)
	} else if (hwconfig_subarg_cmp_f("fsl_ddr",
					"ctlr_intlv",
					"3way_1KB", buf)) {
		popts->memctl_interleaving_mode =
			FSL_DDR_3WAY_1KB_INTERLEAVING;
	} else if (hwconfig_subarg_cmp_f("fsl_ddr",
					"ctlr_intlv",
					"3way_4KB", buf)) {
		popts->memctl_interleaving_mode =
			FSL_DDR_3WAY_4KB_INTERLEAVING;
	} else if (hwconfig_subarg_cmp_f("fsl_ddr",
					"ctlr_intlv",
					"3way_8KB", buf)) {
		popts->memctl_interleaving_mode =
			FSL_DDR_3WAY_8KB_INTERLEAVING;
#elif (CONFIG_SYS_NUM_DDR_CTLRS == 4)
	} else if (hwconfig_subarg_cmp_f("fsl_ddr",
					"ctlr_intlv",
					"4way_1KB", buf)) {
		popts->memctl_interleaving_mode =
			FSL_DDR_4WAY_1KB_INTERLEAVING;
	} else if (hwconfig_subarg_cmp_f("fsl_ddr",
					"ctlr_intlv",
					"4way_4KB", buf)) {
		popts->memctl_interleaving_mode =
			FSL_DDR_4WAY_4KB_INTERLEAVING;
	} else if (hwconfig_subarg_cmp_f("fsl_ddr",
					"ctlr_intlv",
					"4way_8KB", buf)) {
		popts->memctl_interleaving_mode =
			FSL_DDR_4WAY_8KB_INTERLEAVING;
#endif
	} else {
		popts->memctl_interleaving = 0;
		printf("hwconfig has unrecognized parameter for ctlr_intlv.\n");
	}
#endif	/* CONFIG_SYS_FSL_DDR_INTLV_256B */
done:
#endif /* CONFIG_SYS_NUM_DDR_CTLRS > 1 */
	if ((hwconfig_sub_f("fsl_ddr", "bank_intlv", buf)) &&
		(CONFIG_CHIP_SELECTS_PER_CTRL > 1)) {
		/* test null first. if CONFIG_HWCONFIG is not defined,
		 * hwconfig_subarg_cmp_f returns non-zero */
		if (hwconfig_subarg_cmp_f("fsl_ddr", "bank_intlv",
					    "null", buf))
			debug("bank interleaving disabled.\n");
		else if (hwconfig_subarg_cmp_f("fsl_ddr", "bank_intlv",
						 "cs0_cs1", buf))
			popts->ba_intlv_ctl = FSL_DDR_CS0_CS1;
		else if (hwconfig_subarg_cmp_f("fsl_ddr", "bank_intlv",
						 "cs2_cs3", buf))
			popts->ba_intlv_ctl = FSL_DDR_CS2_CS3;
		else if (hwconfig_subarg_cmp_f("fsl_ddr", "bank_intlv",
						 "cs0_cs1_and_cs2_cs3", buf))
			popts->ba_intlv_ctl = FSL_DDR_CS0_CS1_AND_CS2_CS3;
		else if (hwconfig_subarg_cmp_f("fsl_ddr", "bank_intlv",
						 "cs0_cs1_cs2_cs3", buf))
			popts->ba_intlv_ctl = FSL_DDR_CS0_CS1_CS2_CS3;
		else if (hwconfig_subarg_cmp_f("fsl_ddr", "bank_intlv",
						"auto", buf))
			popts->ba_intlv_ctl = auto_bank_intlv(pdimm);
		else
			printf("hwconfig has unrecognized parameter for bank_intlv.\n");
		switch (popts->ba_intlv_ctl & FSL_DDR_CS0_CS1_CS2_CS3) {
		case FSL_DDR_CS0_CS1_CS2_CS3:
#if (CONFIG_DIMM_SLOTS_PER_CTLR == 1)
			if (pdimm[0].n_ranks < 4) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(chip-select) for "
					"CS0+CS1+CS2+CS3 on controller %d, "
					"interleaving disabled!\n", ctrl_num);
			}
#elif (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
#ifdef CONFIG_FSL_DDR_FIRST_SLOT_QUAD_CAPABLE
			if (pdimm[0].n_ranks == 4)
				break;
#endif
			if ((pdimm[0].n_ranks < 2) && (pdimm[1].n_ranks < 2)) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(chip-select) for "
					"CS0+CS1+CS2+CS3 on controller %d, "
					"interleaving disabled!\n", ctrl_num);
			}
			if (pdimm[0].capacity != pdimm[1].capacity) {
				popts->ba_intlv_ctl = 0;
				printf("Not identical DIMM size for "
					"CS0+CS1+CS2+CS3 on controller %d, "
					"interleaving disabled!\n", ctrl_num);
			}
#endif
			break;
		case FSL_DDR_CS0_CS1:
			if (pdimm[0].n_ranks < 2) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(chip-select) for "
					"CS0+CS1 on controller %d, "
					"interleaving disabled!\n", ctrl_num);
			}
			break;
		case FSL_DDR_CS2_CS3:
#if (CONFIG_DIMM_SLOTS_PER_CTLR == 1)
			if (pdimm[0].n_ranks < 4) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(chip-select) for CS2+CS3 "
					"on controller %d, interleaving disabled!\n", ctrl_num);
			}
#elif (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
			if (pdimm[1].n_ranks < 2) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(chip-select) for CS2+CS3 "
					"on controller %d, interleaving disabled!\n", ctrl_num);
			}
#endif
			break;
		case FSL_DDR_CS0_CS1_AND_CS2_CS3:
#if (CONFIG_DIMM_SLOTS_PER_CTLR == 1)
			if (pdimm[0].n_ranks < 4) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(CS) for CS0+CS1 and "
					"CS2+CS3 on controller %d, "
					"interleaving disabled!\n", ctrl_num);
			}
#elif (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
			if ((pdimm[0].n_ranks < 2) || (pdimm[1].n_ranks < 2)) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(CS) for CS0+CS1 and "
					"CS2+CS3 on controller %d, "
					"interleaving disabled!\n", ctrl_num);
			}
#endif
			break;
		default:
			popts->ba_intlv_ctl = 0;
			break;
		}
	}

	if (hwconfig_sub_f("fsl_ddr", "addr_hash", buf)) {
		if (hwconfig_subarg_cmp_f("fsl_ddr", "addr_hash", "null", buf))
			popts->addr_hash = 0;
		else if (hwconfig_subarg_cmp_f("fsl_ddr", "addr_hash",
					       "true", buf))
			popts->addr_hash = 1;
	}

	if (pdimm[0].n_ranks == 4)
		popts->quad_rank_present = 1;

	popts->package_3ds = pdimm->package_3ds;

#if (CONFIG_FSL_SDRAM_TYPE != SDRAM_TYPE_DDR4)
	ddr_freq = get_ddr_freq(ctrl_num) / 1000000;
	if (popts->registered_dimm_en) {
		popts->rcw_override = 1;
		popts->rcw_1 = 0x000a5a00;
		if (ddr_freq <= 800)
			popts->rcw_2 = 0x00000000;
		else if (ddr_freq <= 1066)
			popts->rcw_2 = 0x00100000;
		else if (ddr_freq <= 1333)
			popts->rcw_2 = 0x00200000;
		else
			popts->rcw_2 = 0x00300000;
	}
#endif

	fsl_ddr_board_options(popts, pdimm, ctrl_num);

	return 0;
}

void check_interleaving_options(fsl_ddr_info_t *pinfo)
{
	int i, j, k, check_n_ranks, intlv_invalid = 0;
	unsigned int check_intlv, check_n_row_addr, check_n_col_addr;
	unsigned long long check_rank_density;
	struct dimm_params_s *dimm;
	int first_ctrl = pinfo->first_ctrl;
	int last_ctrl = first_ctrl + pinfo->num_ctrls - 1;

	/*
	 * Check if all controllers are configured for memory
	 * controller interleaving. Identical dimms are recommended. At least
	 * the size, row and col address should be checked.
	 */
	j = 0;
	check_n_ranks = pinfo->dimm_params[first_ctrl][0].n_ranks;
	check_rank_density = pinfo->dimm_params[first_ctrl][0].rank_density;
	check_n_row_addr =  pinfo->dimm_params[first_ctrl][0].n_row_addr;
	check_n_col_addr = pinfo->dimm_params[first_ctrl][0].n_col_addr;
	check_intlv = pinfo->memctl_opts[first_ctrl].memctl_interleaving_mode;
	for (i = first_ctrl; i <= last_ctrl; i++) {
		dimm = &pinfo->dimm_params[i][0];
		if (!pinfo->memctl_opts[i].memctl_interleaving) {
			continue;
		} else if (((check_rank_density != dimm->rank_density) ||
		     (check_n_ranks != dimm->n_ranks) ||
		     (check_n_row_addr != dimm->n_row_addr) ||
		     (check_n_col_addr != dimm->n_col_addr) ||
		     (check_intlv !=
			pinfo->memctl_opts[i].memctl_interleaving_mode))){
			intlv_invalid = 1;
			break;
		} else {
			j++;
		}

	}
	if (intlv_invalid) {
		for (i = first_ctrl; i <= last_ctrl; i++)
			pinfo->memctl_opts[i].memctl_interleaving = 0;
		printf("Not all DIMMs are identical. "
			"Memory controller interleaving disabled.\n");
	} else {
		switch (check_intlv) {
		case FSL_DDR_256B_INTERLEAVING:
		case FSL_DDR_CACHE_LINE_INTERLEAVING:
		case FSL_DDR_PAGE_INTERLEAVING:
		case FSL_DDR_BANK_INTERLEAVING:
		case FSL_DDR_SUPERBANK_INTERLEAVING:
#if (3 == CONFIG_SYS_NUM_DDR_CTLRS)
				k = 2;
#else
				k = CONFIG_SYS_NUM_DDR_CTLRS;
#endif
			break;
		case FSL_DDR_3WAY_1KB_INTERLEAVING:
		case FSL_DDR_3WAY_4KB_INTERLEAVING:
		case FSL_DDR_3WAY_8KB_INTERLEAVING:
		case FSL_DDR_4WAY_1KB_INTERLEAVING:
		case FSL_DDR_4WAY_4KB_INTERLEAVING:
		case FSL_DDR_4WAY_8KB_INTERLEAVING:
		default:
			k = CONFIG_SYS_NUM_DDR_CTLRS;
			break;
		}
		debug("%d of %d controllers are interleaving.\n", j, k);
		if (j && (j != k)) {
			for (i = first_ctrl; i <= last_ctrl; i++)
				pinfo->memctl_opts[i].memctl_interleaving = 0;
			if ((last_ctrl - first_ctrl) > 1)
				puts("Not all controllers have compatible interleaving mode. All disabled.\n");
		}
	}
	debug("Checking interleaving options completed\n");
}

int fsl_use_spd(void)
{
	int use_spd = 0;

#ifdef CONFIG_DDR_SPD
	char buf[HWCONFIG_BUFFER_SIZE];

	/*
	 * Extract hwconfig from environment since we have not properly setup
	 * the environment but need it for ddr config params
	 */
	if (env_get_f("hwconfig", buf, sizeof(buf)) < 0)
		buf[0] = '\0';

	/* if hwconfig is not enabled, or "sdram" is not defined, use spd */
	if (hwconfig_sub_f("fsl_ddr", "sdram", buf)) {
		if (hwconfig_subarg_cmp_f("fsl_ddr", "sdram", "spd", buf))
			use_spd = 1;
		else if (hwconfig_subarg_cmp_f("fsl_ddr", "sdram",
					       "fixed", buf))
			use_spd = 0;
		else
			use_spd = 1;
	} else
		use_spd = 1;
#endif

	return use_spd;
}
