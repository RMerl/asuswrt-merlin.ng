/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _NPCM_RNG_H_
#define _NPCM_RNG_H_

struct npcm_rng_regs {
	unsigned int rngcs;
	unsigned int rngd;
	unsigned int rngmode;
};

#define RNGCS_RNGE              (1 << 0)
#define RNGCS_DVALID            (1 << 1)
#define RNGCS_CLKP(range)       ((0x0f & (range)) << 2)
#define RNGMODE_M1ROSEL_VAL     (0x02) /* Ring Oscillator Select for Method I */

/*----------------------------------------------------------------------------*/
/* Core Domain Clock Frequency Range for the selected value is higher         */
/* than or equal to the actual Core domain clock frequency                    */
/*----------------------------------------------------------------------------*/
enum {
	RNG_CLKP_80_100_MHZ = 0x00, /*default */
	RNG_CLKP_60_80_MHZ  = 0x01,
	RNG_CLKP_50_60_MHZ  = 0x02,
	RNG_CLKP_40_50_MHZ  = 0x03,
	RNG_CLKP_30_40_MHZ  = 0x04,
	RNG_CLKP_25_30_MHZ  = 0x05,
	RNG_CLKP_20_25_MHZ  = 0x06,
	RNG_CLKP_5_20_MHZ   = 0x07,
	RNG_CLKP_2_15_MHZ   = 0x08,
	RNG_CLKP_9_12_MHZ   = 0x09,
	RNG_CLKP_7_9_MHZ    = 0x0A,
	RNG_CLKP_6_7_MHZ    = 0x0B,
	RNG_CLKP_5_6_MHZ    = 0x0C,
	RNG_CLKP_4_5_MHZ    = 0x0D,
	RNG_CLKP_3_4_MHZ    = 0x0E,
	RNG_NUM_OF_CLKP
};

void npcm_rng_init(void);
void npcm_rng_disable(void);

#endif
