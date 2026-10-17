/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon dtx.
 */

#ifndef __CVMX_DTX_DEFS_H__
#define __CVMX_DTX_DEFS_H__

#define CVMX_DTX_AGL_BCST_RSP	       (0x00011800FE700080ull)
#define CVMX_DTX_AGL_CTL	       (0x00011800FE700060ull)
#define CVMX_DTX_AGL_DATX(offset)      (0x00011800FE700040ull + ((offset) & 1) * 8)
#define CVMX_DTX_AGL_ENAX(offset)      (0x00011800FE700020ull + ((offset) & 1) * 8)
#define CVMX_DTX_AGL_SELX(offset)      (0x00011800FE700000ull + ((offset) & 1) * 8)
#define CVMX_DTX_ASE_BCST_RSP	       (0x00011800FE6E8080ull)
#define CVMX_DTX_ASE_CTL	       (0x00011800FE6E8060ull)
#define CVMX_DTX_ASE_DATX(offset)      (0x00011800FE6E8040ull + ((offset) & 1) * 8)
#define CVMX_DTX_ASE_ENAX(offset)      (0x00011800FE6E8020ull + ((offset) & 1) * 8)
#define CVMX_DTX_ASE_SELX(offset)      (0x00011800FE6E8000ull + ((offset) & 1) * 8)
#define CVMX_DTX_BBX1I_BCST_RSP	       (0x00011800FED78080ull)
#define CVMX_DTX_BBX1I_CTL	       (0x00011800FED78060ull)
#define CVMX_DTX_BBX1I_DATX(offset)    (0x00011800FED78040ull + ((offset) & 1) * 8)
#define CVMX_DTX_BBX1I_ENAX(offset)    (0x00011800FED78020ull + ((offset) & 1) * 8)
#define CVMX_DTX_BBX1I_SELX(offset)    (0x00011800FED78000ull + ((offset) & 1) * 8)
#define CVMX_DTX_BBX2I_BCST_RSP	       (0x00011800FED80080ull)
#define CVMX_DTX_BBX2I_CTL	       (0x00011800FED80060ull)
#define CVMX_DTX_BBX2I_DATX(offset)    (0x00011800FED80040ull + ((offset) & 1) * 8)
#define CVMX_DTX_BBX2I_ENAX(offset)    (0x00011800FED80020ull + ((offset) & 1) * 8)
#define CVMX_DTX_BBX2I_SELX(offset)    (0x00011800FED80000ull + ((offset) & 1) * 8)
#define CVMX_DTX_BBX3I_BCST_RSP	       (0x00011800FED88080ull)
#define CVMX_DTX_BBX3I_CTL	       (0x00011800FED88060ull)
#define CVMX_DTX_BBX3I_DATX(offset)    (0x00011800FED88040ull + ((offset) & 1) * 8)
#define CVMX_DTX_BBX3I_ENAX(offset)    (0x00011800FED88020ull + ((offset) & 1) * 8)
#define CVMX_DTX_BBX3I_SELX(offset)    (0x00011800FED88000ull + ((offset) & 1) * 8)
#define CVMX_DTX_BCH_BCST_RSP	       (0x00011800FE388080ull)
#define CVMX_DTX_BCH_CTL	       (0x00011800FE388060ull)
#define CVMX_DTX_BCH_DATX(offset)      (0x00011800FE388040ull + ((offset) & 1) * 8)
#define CVMX_DTX_BCH_ENAX(offset)      (0x00011800FE388020ull + ((offset) & 1) * 8)
#define CVMX_DTX_BCH_SELX(offset)      (0x00011800FE388000ull + ((offset) & 1) * 8)
#define CVMX_DTX_BGXX_BCST_RSP(offset) (0x00011800FE700080ull + ((offset) & 7) * 32768)
#define CVMX_DTX_BGXX_CTL(offset)      (0x00011800FE700060ull + ((offset) & 7) * 32768)
#define CVMX_DTX_BGXX_DATX(offset, block_id)                                                       \
	(0x00011800FE700040ull + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8)
#define CVMX_DTX_BGXX_ENAX(offset, block_id)                                                       \
	(0x00011800FE700020ull + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8)
#define CVMX_DTX_BGXX_SELX(offset, block_id)                                                       \
	(0x00011800FE700000ull + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8)
#define CVMX_DTX_BROADCAST_CTL		(0x00011800FE7F0060ull)
#define CVMX_DTX_BROADCAST_ENAX(offset) (0x00011800FE7F0020ull + ((offset) & 1) * 8)
#define CVMX_DTX_BROADCAST_SELX(offset) (0x00011800FE7F0000ull + ((offset) & 1) * 8)
#define CVMX_DTX_BTS_BCST_RSP		(0x00011800FE5B0080ull)
#define CVMX_DTX_BTS_CTL		(0x00011800FE5B0060ull)
#define CVMX_DTX_BTS_DATX(offset)	(0x00011800FE5B0040ull + ((offset) & 1) * 8)
#define CVMX_DTX_BTS_ENAX(offset)	(0x00011800FE5B0020ull + ((offset) & 1) * 8)
#define CVMX_DTX_BTS_SELX(offset)	(0x00011800FE5B0000ull + ((offset) & 1) * 8)
#define CVMX_DTX_CIU_BCST_RSP		(0x00011800FE808080ull)
#define CVMX_DTX_CIU_CTL		(0x00011800FE808060ull)
#define CVMX_DTX_CIU_DATX(offset)	(0x00011800FE808040ull + ((offset) & 1) * 8)
#define CVMX_DTX_CIU_ENAX(offset)	(0x00011800FE808020ull + ((offset) & 1) * 8)
#define CVMX_DTX_CIU_SELX(offset)	(0x00011800FE808000ull + ((offset) & 1) * 8)
#define CVMX_DTX_DENC_BCST_RSP		(0x00011800FED48080ull)
#define CVMX_DTX_DENC_CTL		(0x00011800FED48060ull)
#define CVMX_DTX_DENC_DATX(offset)	(0x00011800FED48040ull + ((offset) & 1) * 8)
#define CVMX_DTX_DENC_ENAX(offset)	(0x00011800FED48020ull + ((offset) & 1) * 8)
#define CVMX_DTX_DENC_SELX(offset)	(0x00011800FED48000ull + ((offset) & 1) * 8)
#define CVMX_DTX_DFA_BCST_RSP		(0x00011800FE1B8080ull)
#define CVMX_DTX_DFA_CTL		(0x00011800FE1B8060ull)
#define CVMX_DTX_DFA_DATX(offset)	(0x00011800FE1B8040ull + ((offset) & 1) * 8)
#define CVMX_DTX_DFA_ENAX(offset)	(0x00011800FE1B8020ull + ((offset) & 1) * 8)
#define CVMX_DTX_DFA_SELX(offset)	(0x00011800FE1B8000ull + ((offset) & 1) * 8)
#define CVMX_DTX_DLFE_BCST_RSP		(0x00011800FED18080ull)
#define CVMX_DTX_DLFE_CTL		(0x00011800FED18060ull)
#define CVMX_DTX_DLFE_DATX(offset)	(0x00011800FED18040ull + ((offset) & 1) * 8)
#define CVMX_DTX_DLFE_ENAX(offset)	(0x00011800FED18020ull + ((offset) & 1) * 8)
#define CVMX_DTX_DLFE_SELX(offset)	(0x00011800FED18000ull + ((offset) & 1) * 8)
#define CVMX_DTX_DPI_BCST_RSP		(0x00011800FEEF8080ull)
#define CVMX_DTX_DPI_CTL		(0x00011800FEEF8060ull)
#define CVMX_DTX_DPI_DATX(offset)	(0x00011800FEEF8040ull + ((offset) & 1) * 8)
#define CVMX_DTX_DPI_ENAX(offset)	(0x00011800FEEF8020ull + ((offset) & 1) * 8)
#define CVMX_DTX_DPI_SELX(offset)	(0x00011800FEEF8000ull + ((offset) & 1) * 8)
#define CVMX_DTX_FDEQX_BCST_RSP(offset) (0x00011800FED30080ull + ((offset) & 1) * 0x20000ull)
#define CVMX_DTX_FDEQX_CTL(offset)	(0x00011800FED30060ull + ((offset) & 1) * 0x20000ull)
#define CVMX_DTX_FDEQX_DATX(offset, block_id)                                                      \
	(0x00011800FED30040ull + (((offset) & 1) + ((block_id) & 1) * 0x4000ull) * 8)
#define CVMX_DTX_FDEQX_ENAX(offset, block_id)                                                      \
	(0x00011800FED30020ull + (((offset) & 1) + ((block_id) & 1) * 0x4000ull) * 8)
#define CVMX_DTX_FDEQX_SELX(offset, block_id)                                                      \
	(0x00011800FED30000ull + (((offset) & 1) + ((block_id) & 1) * 0x4000ull) * 8)
#define CVMX_DTX_FPA_BCST_RSP CVMX_DTX_FPA_BCST_RSP_FUNC()
static inline u64 CVMX_DTX_FPA_BCST_RSP_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FE940080ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FE940080ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE940080ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE140080ull;
	}
	return 0x00011800FE940080ull;
}

#define CVMX_DTX_FPA_CTL CVMX_DTX_FPA_CTL_FUNC()
static inline u64 CVMX_DTX_FPA_CTL_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FE940060ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FE940060ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE940060ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE140060ull;
	}
	return 0x00011800FE940060ull;
}

static inline u64 CVMX_DTX_FPA_DATX(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FE940040ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FE940040ull + (offset) * 8;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE940040ull + (offset) * 8;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE140040ull + (offset) * 8;
	}
	return 0x00011800FE940040ull + (offset) * 8;
}

static inline u64 CVMX_DTX_FPA_ENAX(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FE940020ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FE940020ull + (offset) * 8;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE940020ull + (offset) * 8;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE140020ull + (offset) * 8;
	}
	return 0x00011800FE940020ull + (offset) * 8;
}

static inline u64 CVMX_DTX_FPA_SELX(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FE940000ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FE940000ull + (offset) * 8;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE940000ull + (offset) * 8;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE140000ull + (offset) * 8;
	}
	return 0x00011800FE940000ull + (offset) * 8;
}

#define CVMX_DTX_GMXX_BCST_RSP(offset) (0x00011800FE040080ull + ((offset) & 1) * 0x40000ull)
#define CVMX_DTX_GMXX_CTL(offset)      (0x00011800FE040060ull + ((offset) & 1) * 0x40000ull)
#define CVMX_DTX_GMXX_DATX(offset, block_id)                                                       \
	(0x00011800FE040040ull + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8)
#define CVMX_DTX_GMXX_ENAX(offset, block_id)                                                       \
	(0x00011800FE040020ull + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8)
#define CVMX_DTX_GMXX_SELX(offset, block_id)                                                       \
	(0x00011800FE040000ull + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8)
#define CVMX_DTX_GSERX_BCST_RSP(offset) (0x00011800FE480080ull + ((offset) & 15) * 32768)
#define CVMX_DTX_GSERX_CTL(offset)	(0x00011800FE480060ull + ((offset) & 15) * 32768)
#define CVMX_DTX_GSERX_DATX(offset, block_id)                                                      \
	(0x00011800FE480040ull + (((offset) & 1) + ((block_id) & 15) * 0x1000ull) * 8)
#define CVMX_DTX_GSERX_ENAX(offset, block_id)                                                      \
	(0x00011800FE480020ull + (((offset) & 1) + ((block_id) & 15) * 0x1000ull) * 8)
#define CVMX_DTX_GSERX_SELX(offset, block_id)                                                      \
	(0x00011800FE480000ull + (((offset) & 1) + ((block_id) & 15) * 0x1000ull) * 8)
#define CVMX_DTX_HNA_BCST_RSP		   (0x00011800FE238080ull)
#define CVMX_DTX_HNA_CTL		   (0x00011800FE238060ull)
#define CVMX_DTX_HNA_DATX(offset)	   (0x00011800FE238040ull + ((offset) & 1) * 8)
#define CVMX_DTX_HNA_ENAX(offset)	   (0x00011800FE238020ull + ((offset) & 1) * 8)
#define CVMX_DTX_HNA_SELX(offset)	   (0x00011800FE238000ull + ((offset) & 1) * 8)
#define CVMX_DTX_ILA_BCST_RSP		   (0x00011800FE0B8080ull)
#define CVMX_DTX_ILA_CTL		   (0x00011800FE0B8060ull)
#define CVMX_DTX_ILA_DATX(offset)	   (0x00011800FE0B8040ull + ((offset) & 1) * 8)
#define CVMX_DTX_ILA_ENAX(offset)	   (0x00011800FE0B8020ull + ((offset) & 1) * 8)
#define CVMX_DTX_ILA_SELX(offset)	   (0x00011800FE0B8000ull + ((offset) & 1) * 8)
#define CVMX_DTX_ILK_BCST_RSP		   (0x00011800FE0A0080ull)
#define CVMX_DTX_ILK_CTL		   (0x00011800FE0A0060ull)
#define CVMX_DTX_ILK_DATX(offset)	   (0x00011800FE0A0040ull + ((offset) & 1) * 8)
#define CVMX_DTX_ILK_ENAX(offset)	   (0x00011800FE0A0020ull + ((offset) & 1) * 8)
#define CVMX_DTX_ILK_SELX(offset)	   (0x00011800FE0A0000ull + ((offset) & 1) * 8)
#define CVMX_DTX_IOBN_BCST_RSP		   (0x00011800FE780080ull)
#define CVMX_DTX_IOBN_CTL		   (0x00011800FE780060ull)
#define CVMX_DTX_IOBN_DATX(offset)	   (0x00011800FE780040ull + ((offset) & 1) * 8)
#define CVMX_DTX_IOBN_ENAX(offset)	   (0x00011800FE780020ull + ((offset) & 1) * 8)
#define CVMX_DTX_IOBN_SELX(offset)	   (0x00011800FE780000ull + ((offset) & 1) * 8)
#define CVMX_DTX_IOBP_BCST_RSP		   (0x00011800FE7A0080ull)
#define CVMX_DTX_IOBP_CTL		   (0x00011800FE7A0060ull)
#define CVMX_DTX_IOBP_DATX(offset)	   (0x00011800FE7A0040ull + ((offset) & 1) * 8)
#define CVMX_DTX_IOBP_ENAX(offset)	   (0x00011800FE7A0020ull + ((offset) & 1) * 8)
#define CVMX_DTX_IOBP_SELX(offset)	   (0x00011800FE7A0000ull + ((offset) & 1) * 8)
#define CVMX_DTX_IOB_BCST_RSP		   (0x00011800FE780080ull)
#define CVMX_DTX_IOB_CTL		   (0x00011800FE780060ull)
#define CVMX_DTX_IOB_DATX(offset)	   (0x00011800FE780040ull + ((offset) & 1) * 8)
#define CVMX_DTX_IOB_ENAX(offset)	   (0x00011800FE780020ull + ((offset) & 1) * 8)
#define CVMX_DTX_IOB_SELX(offset)	   (0x00011800FE780000ull + ((offset) & 1) * 8)
#define CVMX_DTX_IPD_BCST_RSP		   (0x00011800FE278080ull)
#define CVMX_DTX_IPD_CTL		   (0x00011800FE278060ull)
#define CVMX_DTX_IPD_DATX(offset)	   (0x00011800FE278040ull + ((offset) & 1) * 8)
#define CVMX_DTX_IPD_ENAX(offset)	   (0x00011800FE278020ull + ((offset) & 1) * 8)
#define CVMX_DTX_IPD_SELX(offset)	   (0x00011800FE278000ull + ((offset) & 1) * 8)
#define CVMX_DTX_KEY_BCST_RSP		   (0x00011800FE100080ull)
#define CVMX_DTX_KEY_CTL		   (0x00011800FE100060ull)
#define CVMX_DTX_KEY_DATX(offset)	   (0x00011800FE100040ull + ((offset) & 1) * 8)
#define CVMX_DTX_KEY_ENAX(offset)	   (0x00011800FE100020ull + ((offset) & 1) * 8)
#define CVMX_DTX_KEY_SELX(offset)	   (0x00011800FE100000ull + ((offset) & 1) * 8)
#define CVMX_DTX_L2C_CBCX_BCST_RSP(offset) (0x00011800FE420080ull + ((offset) & 3) * 32768)
#define CVMX_DTX_L2C_CBCX_CTL(offset)	   (0x00011800FE420060ull + ((offset) & 3) * 32768)
#define CVMX_DTX_L2C_CBCX_DATX(offset, block_id)                                                   \
	(0x00011800FE420040ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_L2C_CBCX_ENAX(offset, block_id)                                                   \
	(0x00011800FE420020ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_L2C_CBCX_SELX(offset, block_id)                                                   \
	(0x00011800FE420000ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_L2C_MCIX_BCST_RSP(offset) (0x00011800FE2E0080ull + ((offset) & 3) * 32768)
#define CVMX_DTX_L2C_MCIX_CTL(offset)	   (0x00011800FE2E0060ull + ((offset) & 3) * 32768)
#define CVMX_DTX_L2C_MCIX_DATX(offset, block_id)                                                   \
	(0x00011800FE2E0040ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_L2C_MCIX_ENAX(offset, block_id)                                                   \
	(0x00011800FE2E0020ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_L2C_MCIX_SELX(offset, block_id)                                                   \
	(0x00011800FE2E0000ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_L2C_TADX_BCST_RSP(offset) (0x00011800FE240080ull + ((offset) & 7) * 32768)
#define CVMX_DTX_L2C_TADX_CTL(offset)	   (0x00011800FE240060ull + ((offset) & 7) * 32768)
#define CVMX_DTX_L2C_TADX_DATX(offset, block_id)                                                   \
	(0x00011800FE240040ull + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8)
#define CVMX_DTX_L2C_TADX_ENAX(offset, block_id)                                                   \
	(0x00011800FE240020ull + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8)
#define CVMX_DTX_L2C_TADX_SELX(offset, block_id)                                                   \
	(0x00011800FE240000ull + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8)
#define CVMX_DTX_LAPX_BCST_RSP(offset) (0x00011800FE060080ull + ((offset) & 1) * 32768)
#define CVMX_DTX_LAPX_CTL(offset)      (0x00011800FE060060ull + ((offset) & 1) * 32768)
#define CVMX_DTX_LAPX_DATX(offset, block_id)                                                       \
	(0x00011800FE060040ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_LAPX_ENAX(offset, block_id)                                                       \
	(0x00011800FE060020ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_LAPX_SELX(offset, block_id)                                                       \
	(0x00011800FE060000ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_LBK_BCST_RSP	       (0x00011800FE090080ull)
#define CVMX_DTX_LBK_CTL	       (0x00011800FE090060ull)
#define CVMX_DTX_LBK_DATX(offset)      (0x00011800FE090040ull + ((offset) & 1) * 8)
#define CVMX_DTX_LBK_ENAX(offset)      (0x00011800FE090020ull + ((offset) & 1) * 8)
#define CVMX_DTX_LBK_SELX(offset)      (0x00011800FE090000ull + ((offset) & 1) * 8)
#define CVMX_DTX_LMCX_BCST_RSP(offset) (0x00011800FE440080ull + ((offset) & 3) * 32768)
#define CVMX_DTX_LMCX_CTL(offset)      (0x00011800FE440060ull + ((offset) & 3) * 32768)
#define CVMX_DTX_LMCX_DATX(offset, block_id)                                                       \
	(0x00011800FE440040ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_LMCX_ENAX(offset, block_id)                                                       \
	(0x00011800FE440020ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_LMCX_SELX(offset, block_id)                                                       \
	(0x00011800FE440000ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_MDBX_BCST_RSP(offset) (0x00011800FEC00080ull + ((offset) & 31) * 32768)
#define CVMX_DTX_MDBX_CTL(offset)      (0x00011800FEC00060ull + ((offset) & 31) * 32768)
#define CVMX_DTX_MDBX_DATX(offset, block_id)                                                       \
	(0x00011800FEC00040ull + (((offset) & 1) + ((block_id) & 31) * 0x1000ull) * 8)
#define CVMX_DTX_MDBX_ENAX(offset, block_id)                                                       \
	(0x00011800FEC00020ull + (((offset) & 1) + ((block_id) & 31) * 0x1000ull) * 8)
#define CVMX_DTX_MDBX_SELX(offset, block_id)                                                       \
	(0x00011800FEC00000ull + (((offset) & 1) + ((block_id) & 31) * 0x1000ull) * 8)
#define CVMX_DTX_MHBW_BCST_RSP		   (0x00011800FE598080ull)
#define CVMX_DTX_MHBW_CTL		   (0x00011800FE598060ull)
#define CVMX_DTX_MHBW_DATX(offset)	   (0x00011800FE598040ull + ((offset) & 1) * 8)
#define CVMX_DTX_MHBW_ENAX(offset)	   (0x00011800FE598020ull + ((offset) & 1) * 8)
#define CVMX_DTX_MHBW_SELX(offset)	   (0x00011800FE598000ull + ((offset) & 1) * 8)
#define CVMX_DTX_MIO_BCST_RSP		   (0x00011800FE000080ull)
#define CVMX_DTX_MIO_CTL		   (0x00011800FE000060ull)
#define CVMX_DTX_MIO_DATX(offset)	   (0x00011800FE000040ull + ((offset) & 1) * 8)
#define CVMX_DTX_MIO_ENAX(offset)	   (0x00011800FE000020ull + ((offset) & 1) * 8)
#define CVMX_DTX_MIO_SELX(offset)	   (0x00011800FE000000ull + ((offset) & 1) * 8)
#define CVMX_DTX_OCX_BOT_BCST_RSP	   (0x00011800FE198080ull)
#define CVMX_DTX_OCX_BOT_CTL		   (0x00011800FE198060ull)
#define CVMX_DTX_OCX_BOT_DATX(offset)	   (0x00011800FE198040ull + ((offset) & 1) * 8)
#define CVMX_DTX_OCX_BOT_ENAX(offset)	   (0x00011800FE198020ull + ((offset) & 1) * 8)
#define CVMX_DTX_OCX_BOT_SELX(offset)	   (0x00011800FE198000ull + ((offset) & 1) * 8)
#define CVMX_DTX_OCX_LNKX_BCST_RSP(offset) (0x00011800FE180080ull + ((offset) & 3) * 32768)
#define CVMX_DTX_OCX_LNKX_CTL(offset)	   (0x00011800FE180060ull + ((offset) & 3) * 32768)
#define CVMX_DTX_OCX_LNKX_DATX(offset, block_id)                                                   \
	(0x00011800FE180040ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_OCX_LNKX_ENAX(offset, block_id)                                                   \
	(0x00011800FE180020ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_OCX_LNKX_SELX(offset, block_id)                                                   \
	(0x00011800FE180000ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_OCX_OLEX_BCST_RSP(offset) (0x00011800FE1A0080ull + ((offset) & 3) * 32768)
#define CVMX_DTX_OCX_OLEX_CTL(offset)	   (0x00011800FE1A0060ull + ((offset) & 3) * 32768)
#define CVMX_DTX_OCX_OLEX_DATX(offset, block_id)                                                   \
	(0x00011800FE1A0040ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_OCX_OLEX_ENAX(offset, block_id)                                                   \
	(0x00011800FE1A0020ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_OCX_OLEX_SELX(offset, block_id)                                                   \
	(0x00011800FE1A0000ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_OCX_TOP_BCST_RSP     (0x00011800FE088080ull)
#define CVMX_DTX_OCX_TOP_CTL	      (0x00011800FE088060ull)
#define CVMX_DTX_OCX_TOP_DATX(offset) (0x00011800FE088040ull + ((offset) & 1) * 8)
#define CVMX_DTX_OCX_TOP_ENAX(offset) (0x00011800FE088020ull + ((offset) & 1) * 8)
#define CVMX_DTX_OCX_TOP_SELX(offset) (0x00011800FE088000ull + ((offset) & 1) * 8)
#define CVMX_DTX_OSM_BCST_RSP	      CVMX_DTX_OSM_BCST_RSP_FUNC()
static inline u64 CVMX_DTX_OSM_BCST_RSP_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FE6E0080ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FE6E0080ull;

	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FEEE0080ull;
	}
	return 0x00011800FE6E0080ull;
}

#define CVMX_DTX_OSM_CTL CVMX_DTX_OSM_CTL_FUNC()
static inline u64 CVMX_DTX_OSM_CTL_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FE6E0060ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FE6E0060ull;

	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FEEE0060ull;
	}
	return 0x00011800FE6E0060ull;
}

static inline u64 CVMX_DTX_OSM_DATX(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FE6E0040ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FE6E0040ull + (offset) * 8;

	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FEEE0040ull + (offset) * 8;
	}
	return 0x00011800FE6E0040ull + (offset) * 8;
}

static inline u64 CVMX_DTX_OSM_ENAX(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FE6E0020ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FE6E0020ull + (offset) * 8;

	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FEEE0020ull + (offset) * 8;
	}
	return 0x00011800FE6E0020ull + (offset) * 8;
}

static inline u64 CVMX_DTX_OSM_SELX(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FE6E0000ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FE6E0000ull + (offset) * 8;

	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FEEE0000ull + (offset) * 8;
	}
	return 0x00011800FE6E0000ull + (offset) * 8;
}

#define CVMX_DTX_PCSX_BCST_RSP(offset) (0x00011800FE580080ull + ((offset) & 1) * 0x40000ull)
#define CVMX_DTX_PCSX_CTL(offset)      (0x00011800FE580060ull + ((offset) & 1) * 0x40000ull)
#define CVMX_DTX_PCSX_DATX(offset, block_id)                                                       \
	(0x00011800FE580040ull + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8)
#define CVMX_DTX_PCSX_ENAX(offset, block_id)                                                       \
	(0x00011800FE580020ull + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8)
#define CVMX_DTX_PCSX_SELX(offset, block_id)                                                       \
	(0x00011800FE580000ull + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8)
#define CVMX_DTX_PEMX_BCST_RSP(offset) (0x00011800FE600080ull + ((offset) & 3) * 32768)
#define CVMX_DTX_PEMX_CTL(offset)      (0x00011800FE600060ull + ((offset) & 3) * 32768)
#define CVMX_DTX_PEMX_DATX(offset, block_id)                                                       \
	(0x00011800FE600040ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_PEMX_ENAX(offset, block_id)                                                       \
	(0x00011800FE600020ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_PEMX_SELX(offset, block_id)                                                       \
	(0x00011800FE600000ull + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#define CVMX_DTX_PIP_BCST_RSP	      (0x00011800FE500080ull)
#define CVMX_DTX_PIP_CTL	      (0x00011800FE500060ull)
#define CVMX_DTX_PIP_DATX(offset)     (0x00011800FE500040ull + ((offset) & 1) * 8)
#define CVMX_DTX_PIP_ENAX(offset)     (0x00011800FE500020ull + ((offset) & 1) * 8)
#define CVMX_DTX_PIP_SELX(offset)     (0x00011800FE500000ull + ((offset) & 1) * 8)
#define CVMX_DTX_PKI_PBE_BCST_RSP     (0x00011800FE228080ull)
#define CVMX_DTX_PKI_PBE_CTL	      (0x00011800FE228060ull)
#define CVMX_DTX_PKI_PBE_DATX(offset) (0x00011800FE228040ull + ((offset) & 1) * 8)
#define CVMX_DTX_PKI_PBE_ENAX(offset) (0x00011800FE228020ull + ((offset) & 1) * 8)
#define CVMX_DTX_PKI_PBE_SELX(offset) (0x00011800FE228000ull + ((offset) & 1) * 8)
#define CVMX_DTX_PKI_PFE_BCST_RSP     (0x00011800FE220080ull)
#define CVMX_DTX_PKI_PFE_CTL	      (0x00011800FE220060ull)
#define CVMX_DTX_PKI_PFE_DATX(offset) (0x00011800FE220040ull + ((offset) & 1) * 8)
#define CVMX_DTX_PKI_PFE_ENAX(offset) (0x00011800FE220020ull + ((offset) & 1) * 8)
#define CVMX_DTX_PKI_PFE_SELX(offset) (0x00011800FE220000ull + ((offset) & 1) * 8)
#define CVMX_DTX_PKI_PIX_BCST_RSP     (0x00011800FE230080ull)
#define CVMX_DTX_PKI_PIX_CTL	      (0x00011800FE230060ull)
#define CVMX_DTX_PKI_PIX_DATX(offset) (0x00011800FE230040ull + ((offset) & 1) * 8)
#define CVMX_DTX_PKI_PIX_ENAX(offset) (0x00011800FE230020ull + ((offset) & 1) * 8)
#define CVMX_DTX_PKI_PIX_SELX(offset) (0x00011800FE230000ull + ((offset) & 1) * 8)
#define CVMX_DTX_PKO_BCST_RSP	      CVMX_DTX_PKO_BCST_RSP_FUNC()
static inline u64 CVMX_DTX_PKO_BCST_RSP_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FEAA0080ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FEAA0080ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FEAA0080ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE280080ull;
	}
	return 0x00011800FEAA0080ull;
}

#define CVMX_DTX_PKO_CTL CVMX_DTX_PKO_CTL_FUNC()
static inline u64 CVMX_DTX_PKO_CTL_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FEAA0060ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FEAA0060ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FEAA0060ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE280060ull;
	}
	return 0x00011800FEAA0060ull;
}

static inline u64 CVMX_DTX_PKO_DATX(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FEAA0040ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FEAA0040ull + (offset) * 8;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FEAA0040ull + (offset) * 8;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE280040ull + (offset) * 8;
	}
	return 0x00011800FEAA0040ull + (offset) * 8;
}

static inline u64 CVMX_DTX_PKO_ENAX(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FEAA0020ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FEAA0020ull + (offset) * 8;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FEAA0020ull + (offset) * 8;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE280020ull + (offset) * 8;
	}
	return 0x00011800FEAA0020ull + (offset) * 8;
}

static inline u64 CVMX_DTX_PKO_SELX(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800FEAA0000ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800FEAA0000ull + (offset) * 8;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800FEAA0000ull + (offset) * 8;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800FE280000ull + (offset) * 8;
	}
	return 0x00011800FEAA0000ull + (offset) * 8;
}

#define CVMX_DTX_PNBDX_BCST_RSP(offset) (0x00011800FED90080ull + ((offset) & 1) * 32768)
#define CVMX_DTX_PNBDX_CTL(offset)	(0x00011800FED90060ull + ((offset) & 1) * 32768)
#define CVMX_DTX_PNBDX_DATX(offset, block_id)                                                      \
	(0x00011800FED90040ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_PNBDX_ENAX(offset, block_id)                                                      \
	(0x00011800FED90020ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_PNBDX_SELX(offset, block_id)                                                      \
	(0x00011800FED90000ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_PNBX_BCST_RSP(offset) (0x00011800FE580080ull + ((offset) & 1) * 32768)
#define CVMX_DTX_PNBX_CTL(offset)      (0x00011800FE580060ull + ((offset) & 1) * 32768)
#define CVMX_DTX_PNBX_DATX(offset, block_id)                                                       \
	(0x00011800FE580040ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_PNBX_ENAX(offset, block_id)                                                       \
	(0x00011800FE580020ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_PNBX_SELX(offset, block_id)                                                       \
	(0x00011800FE580000ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_POW_BCST_RSP		(0x00011800FE338080ull)
#define CVMX_DTX_POW_CTL		(0x00011800FE338060ull)
#define CVMX_DTX_POW_DATX(offset)	(0x00011800FE338040ull + ((offset) & 1) * 8)
#define CVMX_DTX_POW_ENAX(offset)	(0x00011800FE338020ull + ((offset) & 1) * 8)
#define CVMX_DTX_POW_SELX(offset)	(0x00011800FE338000ull + ((offset) & 1) * 8)
#define CVMX_DTX_PRCH_BCST_RSP		(0x00011800FED00080ull)
#define CVMX_DTX_PRCH_CTL		(0x00011800FED00060ull)
#define CVMX_DTX_PRCH_DATX(offset)	(0x00011800FED00040ull + ((offset) & 1) * 8)
#define CVMX_DTX_PRCH_ENAX(offset)	(0x00011800FED00020ull + ((offset) & 1) * 8)
#define CVMX_DTX_PRCH_SELX(offset)	(0x00011800FED00000ull + ((offset) & 1) * 8)
#define CVMX_DTX_PSM_BCST_RSP		(0x00011800FEEA0080ull)
#define CVMX_DTX_PSM_CTL		(0x00011800FEEA0060ull)
#define CVMX_DTX_PSM_DATX(offset)	(0x00011800FEEA0040ull + ((offset) & 1) * 8)
#define CVMX_DTX_PSM_ENAX(offset)	(0x00011800FEEA0020ull + ((offset) & 1) * 8)
#define CVMX_DTX_PSM_SELX(offset)	(0x00011800FEEA0000ull + ((offset) & 1) * 8)
#define CVMX_DTX_RAD_BCST_RSP		(0x00011800FE380080ull)
#define CVMX_DTX_RAD_CTL		(0x00011800FE380060ull)
#define CVMX_DTX_RAD_DATX(offset)	(0x00011800FE380040ull + ((offset) & 1) * 8)
#define CVMX_DTX_RAD_ENAX(offset)	(0x00011800FE380020ull + ((offset) & 1) * 8)
#define CVMX_DTX_RAD_SELX(offset)	(0x00011800FE380000ull + ((offset) & 1) * 8)
#define CVMX_DTX_RDEC_BCST_RSP		(0x00011800FED68080ull)
#define CVMX_DTX_RDEC_CTL		(0x00011800FED68060ull)
#define CVMX_DTX_RDEC_DATX(offset)	(0x00011800FED68040ull + ((offset) & 1) * 8)
#define CVMX_DTX_RDEC_ENAX(offset)	(0x00011800FED68020ull + ((offset) & 1) * 8)
#define CVMX_DTX_RDEC_SELX(offset)	(0x00011800FED68000ull + ((offset) & 1) * 8)
#define CVMX_DTX_RFIF_BCST_RSP		(0x00011800FE6A8080ull)
#define CVMX_DTX_RFIF_CTL		(0x00011800FE6A8060ull)
#define CVMX_DTX_RFIF_DATX(offset)	(0x00011800FE6A8040ull + ((offset) & 1) * 8)
#define CVMX_DTX_RFIF_ENAX(offset)	(0x00011800FE6A8020ull + ((offset) & 1) * 8)
#define CVMX_DTX_RFIF_SELX(offset)	(0x00011800FE6A8000ull + ((offset) & 1) * 8)
#define CVMX_DTX_RMAP_BCST_RSP		(0x00011800FED40080ull)
#define CVMX_DTX_RMAP_CTL		(0x00011800FED40060ull)
#define CVMX_DTX_RMAP_DATX(offset)	(0x00011800FED40040ull + ((offset) & 1) * 8)
#define CVMX_DTX_RMAP_ENAX(offset)	(0x00011800FED40020ull + ((offset) & 1) * 8)
#define CVMX_DTX_RMAP_SELX(offset)	(0x00011800FED40000ull + ((offset) & 1) * 8)
#define CVMX_DTX_RNM_BCST_RSP		(0x00011800FE200080ull)
#define CVMX_DTX_RNM_CTL		(0x00011800FE200060ull)
#define CVMX_DTX_RNM_DATX(offset)	(0x00011800FE200040ull + ((offset) & 1) * 8)
#define CVMX_DTX_RNM_ENAX(offset)	(0x00011800FE200020ull + ((offset) & 1) * 8)
#define CVMX_DTX_RNM_SELX(offset)	(0x00011800FE200000ull + ((offset) & 1) * 8)
#define CVMX_DTX_RST_BCST_RSP		(0x00011800FE030080ull)
#define CVMX_DTX_RST_CTL		(0x00011800FE030060ull)
#define CVMX_DTX_RST_DATX(offset)	(0x00011800FE030040ull + ((offset) & 1) * 8)
#define CVMX_DTX_RST_ENAX(offset)	(0x00011800FE030020ull + ((offset) & 1) * 8)
#define CVMX_DTX_RST_SELX(offset)	(0x00011800FE030000ull + ((offset) & 1) * 8)
#define CVMX_DTX_SATA_BCST_RSP		(0x00011800FE360080ull)
#define CVMX_DTX_SATA_CTL		(0x00011800FE360060ull)
#define CVMX_DTX_SATA_DATX(offset)	(0x00011800FE360040ull + ((offset) & 1) * 8)
#define CVMX_DTX_SATA_ENAX(offset)	(0x00011800FE360020ull + ((offset) & 1) * 8)
#define CVMX_DTX_SATA_SELX(offset)	(0x00011800FE360000ull + ((offset) & 1) * 8)
#define CVMX_DTX_SLI_BCST_RSP		(0x00011800FE8F8080ull)
#define CVMX_DTX_SLI_CTL		(0x00011800FE8F8060ull)
#define CVMX_DTX_SLI_DATX(offset)	(0x00011800FE8F8040ull + ((offset) & 1) * 8)
#define CVMX_DTX_SLI_ENAX(offset)	(0x00011800FE8F8020ull + ((offset) & 1) * 8)
#define CVMX_DTX_SLI_SELX(offset)	(0x00011800FE8F8000ull + ((offset) & 1) * 8)
#define CVMX_DTX_SPEM_BCST_RSP		(0x00011800FE600080ull)
#define CVMX_DTX_SPEM_CTL		(0x00011800FE600060ull)
#define CVMX_DTX_SPEM_DATX(offset)	(0x00011800FE600040ull + ((offset) & 1) * 8)
#define CVMX_DTX_SPEM_ENAX(offset)	(0x00011800FE600020ull + ((offset) & 1) * 8)
#define CVMX_DTX_SPEM_SELX(offset)	(0x00011800FE600000ull + ((offset) & 1) * 8)
#define CVMX_DTX_SRIOX_BCST_RSP(offset) (0x00011800FE640080ull + ((offset) & 1) * 32768)
#define CVMX_DTX_SRIOX_CTL(offset)	(0x00011800FE640060ull + ((offset) & 1) * 32768)
#define CVMX_DTX_SRIOX_DATX(offset, block_id)                                                      \
	(0x00011800FE640040ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_SRIOX_ENAX(offset, block_id)                                                      \
	(0x00011800FE640020ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_SRIOX_SELX(offset, block_id)                                                      \
	(0x00011800FE640000ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_SSO_BCST_RSP		  (0x00011800FEB38080ull)
#define CVMX_DTX_SSO_CTL		  (0x00011800FEB38060ull)
#define CVMX_DTX_SSO_DATX(offset)	  (0x00011800FEB38040ull + ((offset) & 1) * 8)
#define CVMX_DTX_SSO_ENAX(offset)	  (0x00011800FEB38020ull + ((offset) & 1) * 8)
#define CVMX_DTX_SSO_SELX(offset)	  (0x00011800FEB38000ull + ((offset) & 1) * 8)
#define CVMX_DTX_TDEC_BCST_RSP		  (0x00011800FED60080ull)
#define CVMX_DTX_TDEC_CTL		  (0x00011800FED60060ull)
#define CVMX_DTX_TDEC_DATX(offset)	  (0x00011800FED60040ull + ((offset) & 1) * 8)
#define CVMX_DTX_TDEC_ENAX(offset)	  (0x00011800FED60020ull + ((offset) & 1) * 8)
#define CVMX_DTX_TDEC_SELX(offset)	  (0x00011800FED60000ull + ((offset) & 1) * 8)
#define CVMX_DTX_TIM_BCST_RSP		  (0x00011800FE2C0080ull)
#define CVMX_DTX_TIM_CTL		  (0x00011800FE2C0060ull)
#define CVMX_DTX_TIM_DATX(offset)	  (0x00011800FE2C0040ull + ((offset) & 1) * 8)
#define CVMX_DTX_TIM_ENAX(offset)	  (0x00011800FE2C0020ull + ((offset) & 1) * 8)
#define CVMX_DTX_TIM_SELX(offset)	  (0x00011800FE2C0000ull + ((offset) & 1) * 8)
#define CVMX_DTX_ULFE_BCST_RSP		  (0x00011800FED08080ull)
#define CVMX_DTX_ULFE_CTL		  (0x00011800FED08060ull)
#define CVMX_DTX_ULFE_DATX(offset)	  (0x00011800FED08040ull + ((offset) & 1) * 8)
#define CVMX_DTX_ULFE_ENAX(offset)	  (0x00011800FED08020ull + ((offset) & 1) * 8)
#define CVMX_DTX_ULFE_SELX(offset)	  (0x00011800FED08000ull + ((offset) & 1) * 8)
#define CVMX_DTX_USBDRDX_BCST_RSP(offset) (0x00011800FE340080ull + ((offset) & 1) * 32768)
#define CVMX_DTX_USBDRDX_CTL(offset)	  (0x00011800FE340060ull + ((offset) & 1) * 32768)
#define CVMX_DTX_USBDRDX_DATX(offset, block_id)                                                    \
	(0x00011800FE340040ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_USBDRDX_ENAX(offset, block_id)                                                    \
	(0x00011800FE340020ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_USBDRDX_SELX(offset, block_id)                                                    \
	(0x00011800FE340000ull + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#define CVMX_DTX_USBHX_BCST_RSP(offset) (0x00011800FE340080ull)
#define CVMX_DTX_USBHX_CTL(offset)	(0x00011800FE340060ull)
#define CVMX_DTX_USBHX_DATX(offset, block_id)                                                      \
	(0x00011800FE340040ull + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8)
#define CVMX_DTX_USBHX_ENAX(offset, block_id)                                                      \
	(0x00011800FE340020ull + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8)
#define CVMX_DTX_USBHX_SELX(offset, block_id)                                                      \
	(0x00011800FE340000ull + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8)
#define CVMX_DTX_VDEC_BCST_RSP	   (0x00011800FED70080ull)
#define CVMX_DTX_VDEC_CTL	   (0x00011800FED70060ull)
#define CVMX_DTX_VDEC_DATX(offset) (0x00011800FED70040ull + ((offset) & 1) * 8)
#define CVMX_DTX_VDEC_ENAX(offset) (0x00011800FED70020ull + ((offset) & 1) * 8)
#define CVMX_DTX_VDEC_SELX(offset) (0x00011800FED70000ull + ((offset) & 1) * 8)
#define CVMX_DTX_WPSE_BCST_RSP	   (0x00011800FED10080ull)
#define CVMX_DTX_WPSE_CTL	   (0x00011800FED10060ull)
#define CVMX_DTX_WPSE_DATX(offset) (0x00011800FED10040ull + ((offset) & 1) * 8)
#define CVMX_DTX_WPSE_ENAX(offset) (0x00011800FED10020ull + ((offset) & 1) * 8)
#define CVMX_DTX_WPSE_SELX(offset) (0x00011800FED10000ull + ((offset) & 1) * 8)
#define CVMX_DTX_WRCE_BCST_RSP	   (0x00011800FED38080ull)
#define CVMX_DTX_WRCE_CTL	   (0x00011800FED38060ull)
#define CVMX_DTX_WRCE_DATX(offset) (0x00011800FED38040ull + ((offset) & 1) * 8)
#define CVMX_DTX_WRCE_ENAX(offset) (0x00011800FED38020ull + ((offset) & 1) * 8)
#define CVMX_DTX_WRCE_SELX(offset) (0x00011800FED38000ull + ((offset) & 1) * 8)
#define CVMX_DTX_WRDE_BCST_RSP	   (0x00011800FED58080ull)
#define CVMX_DTX_WRDE_CTL	   (0x00011800FED58060ull)
#define CVMX_DTX_WRDE_DATX(offset) (0x00011800FED58040ull + ((offset) & 1) * 8)
#define CVMX_DTX_WRDE_ENAX(offset) (0x00011800FED58020ull + ((offset) & 1) * 8)
#define CVMX_DTX_WRDE_SELX(offset) (0x00011800FED58000ull + ((offset) & 1) * 8)
#define CVMX_DTX_WRSE_BCST_RSP	   (0x00011800FED28080ull)
#define CVMX_DTX_WRSE_CTL	   (0x00011800FED28060ull)
#define CVMX_DTX_WRSE_DATX(offset) (0x00011800FED28040ull + ((offset) & 1) * 8)
#define CVMX_DTX_WRSE_ENAX(offset) (0x00011800FED28020ull + ((offset) & 1) * 8)
#define CVMX_DTX_WRSE_SELX(offset) (0x00011800FED28000ull + ((offset) & 1) * 8)
#define CVMX_DTX_WTXE_BCST_RSP	   (0x00011800FED20080ull)
#define CVMX_DTX_WTXE_CTL	   (0x00011800FED20060ull)
#define CVMX_DTX_WTXE_DATX(offset) (0x00011800FED20040ull + ((offset) & 1) * 8)
#define CVMX_DTX_WTXE_ENAX(offset) (0x00011800FED20020ull + ((offset) & 1) * 8)
#define CVMX_DTX_WTXE_SELX(offset) (0x00011800FED20000ull + ((offset) & 1) * 8)
#define CVMX_DTX_XCV_BCST_RSP	   (0x00011800FE6D8080ull)
#define CVMX_DTX_XCV_CTL	   (0x00011800FE6D8060ull)
#define CVMX_DTX_XCV_DATX(offset)  (0x00011800FE6D8040ull + ((offset) & 1) * 8)
#define CVMX_DTX_XCV_ENAX(offset)  (0x00011800FE6D8020ull + ((offset) & 1) * 8)
#define CVMX_DTX_XCV_SELX(offset)  (0x00011800FE6D8000ull + ((offset) & 1) * 8)
#define CVMX_DTX_XSX_BCST_RSP	   (0x00011800FE5A8080ull)
#define CVMX_DTX_XSX_CTL	   (0x00011800FE5A8060ull)
#define CVMX_DTX_XSX_DATX(offset)  (0x00011800FE5A8040ull + ((offset) & 1) * 8)
#define CVMX_DTX_XSX_ENAX(offset)  (0x00011800FE5A8020ull + ((offset) & 1) * 8)
#define CVMX_DTX_XSX_SELX(offset)  (0x00011800FE5A8000ull + ((offset) & 1) * 8)
#define CVMX_DTX_ZIP_BCST_RSP	   (0x00011800FE1C0080ull)
#define CVMX_DTX_ZIP_CTL	   (0x00011800FE1C0060ull)
#define CVMX_DTX_ZIP_DATX(offset)  (0x00011800FE1C0040ull + ((offset) & 1) * 8)
#define CVMX_DTX_ZIP_ENAX(offset)  (0x00011800FE1C0020ull + ((offset) & 1) * 8)
#define CVMX_DTX_ZIP_SELX(offset)  (0x00011800FE1C0000ull + ((offset) & 1) * 8)

/**
 * cvmx_dtx_agl_bcst_rsp
 */
union cvmx_dtx_agl_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_agl_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_agl_bcst_rsp_s cn70xx;
	struct cvmx_dtx_agl_bcst_rsp_s cn70xxp1;
};

typedef union cvmx_dtx_agl_bcst_rsp cvmx_dtx_agl_bcst_rsp_t;

/**
 * cvmx_dtx_agl_ctl
 */
union cvmx_dtx_agl_ctl {
	u64 u64;
	struct cvmx_dtx_agl_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_agl_ctl_s cn70xx;
	struct cvmx_dtx_agl_ctl_s cn70xxp1;
};

typedef union cvmx_dtx_agl_ctl cvmx_dtx_agl_ctl_t;

/**
 * cvmx_dtx_agl_dat#
 */
union cvmx_dtx_agl_datx {
	u64 u64;
	struct cvmx_dtx_agl_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_agl_datx_s cn70xx;
	struct cvmx_dtx_agl_datx_s cn70xxp1;
};

typedef union cvmx_dtx_agl_datx cvmx_dtx_agl_datx_t;

/**
 * cvmx_dtx_agl_ena#
 */
union cvmx_dtx_agl_enax {
	u64 u64;
	struct cvmx_dtx_agl_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_agl_enax_s cn70xx;
	struct cvmx_dtx_agl_enax_s cn70xxp1;
};

typedef union cvmx_dtx_agl_enax cvmx_dtx_agl_enax_t;

/**
 * cvmx_dtx_agl_sel#
 */
union cvmx_dtx_agl_selx {
	u64 u64;
	struct cvmx_dtx_agl_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_agl_selx_s cn70xx;
	struct cvmx_dtx_agl_selx_s cn70xxp1;
};

typedef union cvmx_dtx_agl_selx cvmx_dtx_agl_selx_t;

/**
 * cvmx_dtx_ase_bcst_rsp
 */
union cvmx_dtx_ase_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_ase_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_ase_bcst_rsp_s cn78xx;
	struct cvmx_dtx_ase_bcst_rsp_s cn78xxp1;
};

typedef union cvmx_dtx_ase_bcst_rsp cvmx_dtx_ase_bcst_rsp_t;

/**
 * cvmx_dtx_ase_ctl
 */
union cvmx_dtx_ase_ctl {
	u64 u64;
	struct cvmx_dtx_ase_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_ase_ctl_s cn78xx;
	struct cvmx_dtx_ase_ctl_s cn78xxp1;
};

typedef union cvmx_dtx_ase_ctl cvmx_dtx_ase_ctl_t;

/**
 * cvmx_dtx_ase_dat#
 */
union cvmx_dtx_ase_datx {
	u64 u64;
	struct cvmx_dtx_ase_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_ase_datx_s cn78xx;
	struct cvmx_dtx_ase_datx_s cn78xxp1;
};

typedef union cvmx_dtx_ase_datx cvmx_dtx_ase_datx_t;

/**
 * cvmx_dtx_ase_ena#
 */
union cvmx_dtx_ase_enax {
	u64 u64;
	struct cvmx_dtx_ase_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_ase_enax_s cn78xx;
	struct cvmx_dtx_ase_enax_s cn78xxp1;
};

typedef union cvmx_dtx_ase_enax cvmx_dtx_ase_enax_t;

/**
 * cvmx_dtx_ase_sel#
 */
union cvmx_dtx_ase_selx {
	u64 u64;
	struct cvmx_dtx_ase_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_ase_selx_s cn78xx;
	struct cvmx_dtx_ase_selx_s cn78xxp1;
};

typedef union cvmx_dtx_ase_selx cvmx_dtx_ase_selx_t;

/**
 * cvmx_dtx_bbx1i_bcst_rsp
 */
union cvmx_dtx_bbx1i_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_bbx1i_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_bbx1i_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_bbx1i_bcst_rsp cvmx_dtx_bbx1i_bcst_rsp_t;

/**
 * cvmx_dtx_bbx1i_ctl
 */
union cvmx_dtx_bbx1i_ctl {
	u64 u64;
	struct cvmx_dtx_bbx1i_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_bbx1i_ctl_s cnf75xx;
};

typedef union cvmx_dtx_bbx1i_ctl cvmx_dtx_bbx1i_ctl_t;

/**
 * cvmx_dtx_bbx1i_dat#
 */
union cvmx_dtx_bbx1i_datx {
	u64 u64;
	struct cvmx_dtx_bbx1i_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_bbx1i_datx_s cnf75xx;
};

typedef union cvmx_dtx_bbx1i_datx cvmx_dtx_bbx1i_datx_t;

/**
 * cvmx_dtx_bbx1i_ena#
 */
union cvmx_dtx_bbx1i_enax {
	u64 u64;
	struct cvmx_dtx_bbx1i_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_bbx1i_enax_s cnf75xx;
};

typedef union cvmx_dtx_bbx1i_enax cvmx_dtx_bbx1i_enax_t;

/**
 * cvmx_dtx_bbx1i_sel#
 */
union cvmx_dtx_bbx1i_selx {
	u64 u64;
	struct cvmx_dtx_bbx1i_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_bbx1i_selx_s cnf75xx;
};

typedef union cvmx_dtx_bbx1i_selx cvmx_dtx_bbx1i_selx_t;

/**
 * cvmx_dtx_bbx2i_bcst_rsp
 */
union cvmx_dtx_bbx2i_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_bbx2i_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_bbx2i_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_bbx2i_bcst_rsp cvmx_dtx_bbx2i_bcst_rsp_t;

/**
 * cvmx_dtx_bbx2i_ctl
 */
union cvmx_dtx_bbx2i_ctl {
	u64 u64;
	struct cvmx_dtx_bbx2i_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_bbx2i_ctl_s cnf75xx;
};

typedef union cvmx_dtx_bbx2i_ctl cvmx_dtx_bbx2i_ctl_t;

/**
 * cvmx_dtx_bbx2i_dat#
 */
union cvmx_dtx_bbx2i_datx {
	u64 u64;
	struct cvmx_dtx_bbx2i_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_bbx2i_datx_s cnf75xx;
};

typedef union cvmx_dtx_bbx2i_datx cvmx_dtx_bbx2i_datx_t;

/**
 * cvmx_dtx_bbx2i_ena#
 */
union cvmx_dtx_bbx2i_enax {
	u64 u64;
	struct cvmx_dtx_bbx2i_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_bbx2i_enax_s cnf75xx;
};

typedef union cvmx_dtx_bbx2i_enax cvmx_dtx_bbx2i_enax_t;

/**
 * cvmx_dtx_bbx2i_sel#
 */
union cvmx_dtx_bbx2i_selx {
	u64 u64;
	struct cvmx_dtx_bbx2i_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_bbx2i_selx_s cnf75xx;
};

typedef union cvmx_dtx_bbx2i_selx cvmx_dtx_bbx2i_selx_t;

/**
 * cvmx_dtx_bbx3i_bcst_rsp
 */
union cvmx_dtx_bbx3i_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_bbx3i_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_bbx3i_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_bbx3i_bcst_rsp cvmx_dtx_bbx3i_bcst_rsp_t;

/**
 * cvmx_dtx_bbx3i_ctl
 */
union cvmx_dtx_bbx3i_ctl {
	u64 u64;
	struct cvmx_dtx_bbx3i_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_bbx3i_ctl_s cnf75xx;
};

typedef union cvmx_dtx_bbx3i_ctl cvmx_dtx_bbx3i_ctl_t;

/**
 * cvmx_dtx_bbx3i_dat#
 */
union cvmx_dtx_bbx3i_datx {
	u64 u64;
	struct cvmx_dtx_bbx3i_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_bbx3i_datx_s cnf75xx;
};

typedef union cvmx_dtx_bbx3i_datx cvmx_dtx_bbx3i_datx_t;

/**
 * cvmx_dtx_bbx3i_ena#
 */
union cvmx_dtx_bbx3i_enax {
	u64 u64;
	struct cvmx_dtx_bbx3i_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_bbx3i_enax_s cnf75xx;
};

typedef union cvmx_dtx_bbx3i_enax cvmx_dtx_bbx3i_enax_t;

/**
 * cvmx_dtx_bbx3i_sel#
 */
union cvmx_dtx_bbx3i_selx {
	u64 u64;
	struct cvmx_dtx_bbx3i_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_bbx3i_selx_s cnf75xx;
};

typedef union cvmx_dtx_bbx3i_selx cvmx_dtx_bbx3i_selx_t;

/**
 * cvmx_dtx_bch_bcst_rsp
 */
union cvmx_dtx_bch_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_bch_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_bch_bcst_rsp_s cn73xx;
	struct cvmx_dtx_bch_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_bch_bcst_rsp cvmx_dtx_bch_bcst_rsp_t;

/**
 * cvmx_dtx_bch_ctl
 */
union cvmx_dtx_bch_ctl {
	u64 u64;
	struct cvmx_dtx_bch_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_bch_ctl_s cn73xx;
	struct cvmx_dtx_bch_ctl_s cnf75xx;
};

typedef union cvmx_dtx_bch_ctl cvmx_dtx_bch_ctl_t;

/**
 * cvmx_dtx_bch_dat#
 */
union cvmx_dtx_bch_datx {
	u64 u64;
	struct cvmx_dtx_bch_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_bch_datx_s cn73xx;
	struct cvmx_dtx_bch_datx_s cnf75xx;
};

typedef union cvmx_dtx_bch_datx cvmx_dtx_bch_datx_t;

/**
 * cvmx_dtx_bch_ena#
 */
union cvmx_dtx_bch_enax {
	u64 u64;
	struct cvmx_dtx_bch_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_bch_enax_s cn73xx;
	struct cvmx_dtx_bch_enax_s cnf75xx;
};

typedef union cvmx_dtx_bch_enax cvmx_dtx_bch_enax_t;

/**
 * cvmx_dtx_bch_sel#
 */
union cvmx_dtx_bch_selx {
	u64 u64;
	struct cvmx_dtx_bch_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_bch_selx_s cn73xx;
	struct cvmx_dtx_bch_selx_s cnf75xx;
};

typedef union cvmx_dtx_bch_selx cvmx_dtx_bch_selx_t;

/**
 * cvmx_dtx_bgx#_bcst_rsp
 */
union cvmx_dtx_bgxx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_bgxx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_bgxx_bcst_rsp_s cn73xx;
	struct cvmx_dtx_bgxx_bcst_rsp_s cn78xx;
	struct cvmx_dtx_bgxx_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_bgxx_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_bgxx_bcst_rsp cvmx_dtx_bgxx_bcst_rsp_t;

/**
 * cvmx_dtx_bgx#_ctl
 */
union cvmx_dtx_bgxx_ctl {
	u64 u64;
	struct cvmx_dtx_bgxx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_bgxx_ctl_s cn73xx;
	struct cvmx_dtx_bgxx_ctl_s cn78xx;
	struct cvmx_dtx_bgxx_ctl_s cn78xxp1;
	struct cvmx_dtx_bgxx_ctl_s cnf75xx;
};

typedef union cvmx_dtx_bgxx_ctl cvmx_dtx_bgxx_ctl_t;

/**
 * cvmx_dtx_bgx#_dat#
 */
union cvmx_dtx_bgxx_datx {
	u64 u64;
	struct cvmx_dtx_bgxx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_bgxx_datx_s cn73xx;
	struct cvmx_dtx_bgxx_datx_s cn78xx;
	struct cvmx_dtx_bgxx_datx_s cn78xxp1;
	struct cvmx_dtx_bgxx_datx_s cnf75xx;
};

typedef union cvmx_dtx_bgxx_datx cvmx_dtx_bgxx_datx_t;

/**
 * cvmx_dtx_bgx#_ena#
 */
union cvmx_dtx_bgxx_enax {
	u64 u64;
	struct cvmx_dtx_bgxx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_bgxx_enax_s cn73xx;
	struct cvmx_dtx_bgxx_enax_s cn78xx;
	struct cvmx_dtx_bgxx_enax_s cn78xxp1;
	struct cvmx_dtx_bgxx_enax_s cnf75xx;
};

typedef union cvmx_dtx_bgxx_enax cvmx_dtx_bgxx_enax_t;

/**
 * cvmx_dtx_bgx#_sel#
 */
union cvmx_dtx_bgxx_selx {
	u64 u64;
	struct cvmx_dtx_bgxx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_bgxx_selx_s cn73xx;
	struct cvmx_dtx_bgxx_selx_s cn78xx;
	struct cvmx_dtx_bgxx_selx_s cn78xxp1;
	struct cvmx_dtx_bgxx_selx_s cnf75xx;
};

typedef union cvmx_dtx_bgxx_selx cvmx_dtx_bgxx_selx_t;

/**
 * cvmx_dtx_broadcast_ctl
 */
union cvmx_dtx_broadcast_ctl {
	u64 u64;
	struct cvmx_dtx_broadcast_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_broadcast_ctl_s cn70xx;
	struct cvmx_dtx_broadcast_ctl_s cn70xxp1;
	struct cvmx_dtx_broadcast_ctl_s cn73xx;
	struct cvmx_dtx_broadcast_ctl_s cn78xx;
	struct cvmx_dtx_broadcast_ctl_s cn78xxp1;
	struct cvmx_dtx_broadcast_ctl_s cnf75xx;
};

typedef union cvmx_dtx_broadcast_ctl cvmx_dtx_broadcast_ctl_t;

/**
 * cvmx_dtx_broadcast_ena#
 */
union cvmx_dtx_broadcast_enax {
	u64 u64;
	struct cvmx_dtx_broadcast_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_broadcast_enax_s cn70xx;
	struct cvmx_dtx_broadcast_enax_s cn70xxp1;
	struct cvmx_dtx_broadcast_enax_s cn73xx;
	struct cvmx_dtx_broadcast_enax_s cn78xx;
	struct cvmx_dtx_broadcast_enax_s cn78xxp1;
	struct cvmx_dtx_broadcast_enax_s cnf75xx;
};

typedef union cvmx_dtx_broadcast_enax cvmx_dtx_broadcast_enax_t;

/**
 * cvmx_dtx_broadcast_sel#
 */
union cvmx_dtx_broadcast_selx {
	u64 u64;
	struct cvmx_dtx_broadcast_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_broadcast_selx_s cn70xx;
	struct cvmx_dtx_broadcast_selx_s cn70xxp1;
	struct cvmx_dtx_broadcast_selx_s cn73xx;
	struct cvmx_dtx_broadcast_selx_s cn78xx;
	struct cvmx_dtx_broadcast_selx_s cn78xxp1;
	struct cvmx_dtx_broadcast_selx_s cnf75xx;
};

typedef union cvmx_dtx_broadcast_selx cvmx_dtx_broadcast_selx_t;

/**
 * cvmx_dtx_bts_bcst_rsp
 */
union cvmx_dtx_bts_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_bts_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_bts_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_bts_bcst_rsp cvmx_dtx_bts_bcst_rsp_t;

/**
 * cvmx_dtx_bts_ctl
 */
union cvmx_dtx_bts_ctl {
	u64 u64;
	struct cvmx_dtx_bts_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_bts_ctl_s cnf75xx;
};

typedef union cvmx_dtx_bts_ctl cvmx_dtx_bts_ctl_t;

/**
 * cvmx_dtx_bts_dat#
 */
union cvmx_dtx_bts_datx {
	u64 u64;
	struct cvmx_dtx_bts_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_bts_datx_s cnf75xx;
};

typedef union cvmx_dtx_bts_datx cvmx_dtx_bts_datx_t;

/**
 * cvmx_dtx_bts_ena#
 */
union cvmx_dtx_bts_enax {
	u64 u64;
	struct cvmx_dtx_bts_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_bts_enax_s cnf75xx;
};

typedef union cvmx_dtx_bts_enax cvmx_dtx_bts_enax_t;

/**
 * cvmx_dtx_bts_sel#
 */
union cvmx_dtx_bts_selx {
	u64 u64;
	struct cvmx_dtx_bts_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_bts_selx_s cnf75xx;
};

typedef union cvmx_dtx_bts_selx cvmx_dtx_bts_selx_t;

/**
 * cvmx_dtx_ciu_bcst_rsp
 */
union cvmx_dtx_ciu_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_ciu_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_ciu_bcst_rsp_s cn73xx;
	struct cvmx_dtx_ciu_bcst_rsp_s cn78xx;
	struct cvmx_dtx_ciu_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_ciu_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_ciu_bcst_rsp cvmx_dtx_ciu_bcst_rsp_t;

/**
 * cvmx_dtx_ciu_ctl
 */
union cvmx_dtx_ciu_ctl {
	u64 u64;
	struct cvmx_dtx_ciu_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_ciu_ctl_s cn73xx;
	struct cvmx_dtx_ciu_ctl_s cn78xx;
	struct cvmx_dtx_ciu_ctl_s cn78xxp1;
	struct cvmx_dtx_ciu_ctl_s cnf75xx;
};

typedef union cvmx_dtx_ciu_ctl cvmx_dtx_ciu_ctl_t;

/**
 * cvmx_dtx_ciu_dat#
 */
union cvmx_dtx_ciu_datx {
	u64 u64;
	struct cvmx_dtx_ciu_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_ciu_datx_s cn73xx;
	struct cvmx_dtx_ciu_datx_s cn78xx;
	struct cvmx_dtx_ciu_datx_s cn78xxp1;
	struct cvmx_dtx_ciu_datx_s cnf75xx;
};

typedef union cvmx_dtx_ciu_datx cvmx_dtx_ciu_datx_t;

/**
 * cvmx_dtx_ciu_ena#
 */
union cvmx_dtx_ciu_enax {
	u64 u64;
	struct cvmx_dtx_ciu_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_ciu_enax_s cn73xx;
	struct cvmx_dtx_ciu_enax_s cn78xx;
	struct cvmx_dtx_ciu_enax_s cn78xxp1;
	struct cvmx_dtx_ciu_enax_s cnf75xx;
};

typedef union cvmx_dtx_ciu_enax cvmx_dtx_ciu_enax_t;

/**
 * cvmx_dtx_ciu_sel#
 */
union cvmx_dtx_ciu_selx {
	u64 u64;
	struct cvmx_dtx_ciu_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_ciu_selx_s cn73xx;
	struct cvmx_dtx_ciu_selx_s cn78xx;
	struct cvmx_dtx_ciu_selx_s cn78xxp1;
	struct cvmx_dtx_ciu_selx_s cnf75xx;
};

typedef union cvmx_dtx_ciu_selx cvmx_dtx_ciu_selx_t;

/**
 * cvmx_dtx_denc_bcst_rsp
 */
union cvmx_dtx_denc_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_denc_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_denc_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_denc_bcst_rsp cvmx_dtx_denc_bcst_rsp_t;

/**
 * cvmx_dtx_denc_ctl
 */
union cvmx_dtx_denc_ctl {
	u64 u64;
	struct cvmx_dtx_denc_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_denc_ctl_s cnf75xx;
};

typedef union cvmx_dtx_denc_ctl cvmx_dtx_denc_ctl_t;

/**
 * cvmx_dtx_denc_dat#
 */
union cvmx_dtx_denc_datx {
	u64 u64;
	struct cvmx_dtx_denc_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_denc_datx_s cnf75xx;
};

typedef union cvmx_dtx_denc_datx cvmx_dtx_denc_datx_t;

/**
 * cvmx_dtx_denc_ena#
 */
union cvmx_dtx_denc_enax {
	u64 u64;
	struct cvmx_dtx_denc_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_denc_enax_s cnf75xx;
};

typedef union cvmx_dtx_denc_enax cvmx_dtx_denc_enax_t;

/**
 * cvmx_dtx_denc_sel#
 */
union cvmx_dtx_denc_selx {
	u64 u64;
	struct cvmx_dtx_denc_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_denc_selx_s cnf75xx;
};

typedef union cvmx_dtx_denc_selx cvmx_dtx_denc_selx_t;

/**
 * cvmx_dtx_dfa_bcst_rsp
 */
union cvmx_dtx_dfa_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_dfa_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_dfa_bcst_rsp_s cn70xx;
	struct cvmx_dtx_dfa_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_dfa_bcst_rsp_s cn73xx;
	struct cvmx_dtx_dfa_bcst_rsp_s cn78xx;
	struct cvmx_dtx_dfa_bcst_rsp_s cn78xxp1;
};

typedef union cvmx_dtx_dfa_bcst_rsp cvmx_dtx_dfa_bcst_rsp_t;

/**
 * cvmx_dtx_dfa_ctl
 */
union cvmx_dtx_dfa_ctl {
	u64 u64;
	struct cvmx_dtx_dfa_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_dfa_ctl_s cn70xx;
	struct cvmx_dtx_dfa_ctl_s cn70xxp1;
	struct cvmx_dtx_dfa_ctl_s cn73xx;
	struct cvmx_dtx_dfa_ctl_s cn78xx;
	struct cvmx_dtx_dfa_ctl_s cn78xxp1;
};

typedef union cvmx_dtx_dfa_ctl cvmx_dtx_dfa_ctl_t;

/**
 * cvmx_dtx_dfa_dat#
 */
union cvmx_dtx_dfa_datx {
	u64 u64;
	struct cvmx_dtx_dfa_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_dfa_datx_s cn70xx;
	struct cvmx_dtx_dfa_datx_s cn70xxp1;
	struct cvmx_dtx_dfa_datx_s cn73xx;
	struct cvmx_dtx_dfa_datx_s cn78xx;
	struct cvmx_dtx_dfa_datx_s cn78xxp1;
};

typedef union cvmx_dtx_dfa_datx cvmx_dtx_dfa_datx_t;

/**
 * cvmx_dtx_dfa_ena#
 */
union cvmx_dtx_dfa_enax {
	u64 u64;
	struct cvmx_dtx_dfa_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_dfa_enax_s cn70xx;
	struct cvmx_dtx_dfa_enax_s cn70xxp1;
	struct cvmx_dtx_dfa_enax_s cn73xx;
	struct cvmx_dtx_dfa_enax_s cn78xx;
	struct cvmx_dtx_dfa_enax_s cn78xxp1;
};

typedef union cvmx_dtx_dfa_enax cvmx_dtx_dfa_enax_t;

/**
 * cvmx_dtx_dfa_sel#
 */
union cvmx_dtx_dfa_selx {
	u64 u64;
	struct cvmx_dtx_dfa_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_dfa_selx_s cn70xx;
	struct cvmx_dtx_dfa_selx_s cn70xxp1;
	struct cvmx_dtx_dfa_selx_s cn73xx;
	struct cvmx_dtx_dfa_selx_s cn78xx;
	struct cvmx_dtx_dfa_selx_s cn78xxp1;
};

typedef union cvmx_dtx_dfa_selx cvmx_dtx_dfa_selx_t;

/**
 * cvmx_dtx_dlfe_bcst_rsp
 */
union cvmx_dtx_dlfe_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_dlfe_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_dlfe_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_dlfe_bcst_rsp cvmx_dtx_dlfe_bcst_rsp_t;

/**
 * cvmx_dtx_dlfe_ctl
 */
union cvmx_dtx_dlfe_ctl {
	u64 u64;
	struct cvmx_dtx_dlfe_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_dlfe_ctl_s cnf75xx;
};

typedef union cvmx_dtx_dlfe_ctl cvmx_dtx_dlfe_ctl_t;

/**
 * cvmx_dtx_dlfe_dat#
 */
union cvmx_dtx_dlfe_datx {
	u64 u64;
	struct cvmx_dtx_dlfe_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_dlfe_datx_s cnf75xx;
};

typedef union cvmx_dtx_dlfe_datx cvmx_dtx_dlfe_datx_t;

/**
 * cvmx_dtx_dlfe_ena#
 */
union cvmx_dtx_dlfe_enax {
	u64 u64;
	struct cvmx_dtx_dlfe_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_dlfe_enax_s cnf75xx;
};

typedef union cvmx_dtx_dlfe_enax cvmx_dtx_dlfe_enax_t;

/**
 * cvmx_dtx_dlfe_sel#
 */
union cvmx_dtx_dlfe_selx {
	u64 u64;
	struct cvmx_dtx_dlfe_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_dlfe_selx_s cnf75xx;
};

typedef union cvmx_dtx_dlfe_selx cvmx_dtx_dlfe_selx_t;

/**
 * cvmx_dtx_dpi_bcst_rsp
 */
union cvmx_dtx_dpi_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_dpi_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_dpi_bcst_rsp_s cn70xx;
	struct cvmx_dtx_dpi_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_dpi_bcst_rsp_s cn73xx;
	struct cvmx_dtx_dpi_bcst_rsp_s cn78xx;
	struct cvmx_dtx_dpi_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_dpi_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_dpi_bcst_rsp cvmx_dtx_dpi_bcst_rsp_t;

/**
 * cvmx_dtx_dpi_ctl
 */
union cvmx_dtx_dpi_ctl {
	u64 u64;
	struct cvmx_dtx_dpi_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_dpi_ctl_s cn70xx;
	struct cvmx_dtx_dpi_ctl_s cn70xxp1;
	struct cvmx_dtx_dpi_ctl_s cn73xx;
	struct cvmx_dtx_dpi_ctl_s cn78xx;
	struct cvmx_dtx_dpi_ctl_s cn78xxp1;
	struct cvmx_dtx_dpi_ctl_s cnf75xx;
};

typedef union cvmx_dtx_dpi_ctl cvmx_dtx_dpi_ctl_t;

/**
 * cvmx_dtx_dpi_dat#
 */
union cvmx_dtx_dpi_datx {
	u64 u64;
	struct cvmx_dtx_dpi_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_dpi_datx_s cn70xx;
	struct cvmx_dtx_dpi_datx_s cn70xxp1;
	struct cvmx_dtx_dpi_datx_s cn73xx;
	struct cvmx_dtx_dpi_datx_s cn78xx;
	struct cvmx_dtx_dpi_datx_s cn78xxp1;
	struct cvmx_dtx_dpi_datx_s cnf75xx;
};

typedef union cvmx_dtx_dpi_datx cvmx_dtx_dpi_datx_t;

/**
 * cvmx_dtx_dpi_ena#
 */
union cvmx_dtx_dpi_enax {
	u64 u64;
	struct cvmx_dtx_dpi_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_dpi_enax_s cn70xx;
	struct cvmx_dtx_dpi_enax_s cn70xxp1;
	struct cvmx_dtx_dpi_enax_s cn73xx;
	struct cvmx_dtx_dpi_enax_s cn78xx;
	struct cvmx_dtx_dpi_enax_s cn78xxp1;
	struct cvmx_dtx_dpi_enax_s cnf75xx;
};

typedef union cvmx_dtx_dpi_enax cvmx_dtx_dpi_enax_t;

/**
 * cvmx_dtx_dpi_sel#
 */
union cvmx_dtx_dpi_selx {
	u64 u64;
	struct cvmx_dtx_dpi_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_dpi_selx_s cn70xx;
	struct cvmx_dtx_dpi_selx_s cn70xxp1;
	struct cvmx_dtx_dpi_selx_s cn73xx;
	struct cvmx_dtx_dpi_selx_s cn78xx;
	struct cvmx_dtx_dpi_selx_s cn78xxp1;
	struct cvmx_dtx_dpi_selx_s cnf75xx;
};

typedef union cvmx_dtx_dpi_selx cvmx_dtx_dpi_selx_t;

/**
 * cvmx_dtx_fdeq#_bcst_rsp
 */
union cvmx_dtx_fdeqx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_fdeqx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_fdeqx_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_fdeqx_bcst_rsp cvmx_dtx_fdeqx_bcst_rsp_t;

/**
 * cvmx_dtx_fdeq#_ctl
 */
union cvmx_dtx_fdeqx_ctl {
	u64 u64;
	struct cvmx_dtx_fdeqx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_fdeqx_ctl_s cnf75xx;
};

typedef union cvmx_dtx_fdeqx_ctl cvmx_dtx_fdeqx_ctl_t;

/**
 * cvmx_dtx_fdeq#_dat#
 */
union cvmx_dtx_fdeqx_datx {
	u64 u64;
	struct cvmx_dtx_fdeqx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_fdeqx_datx_s cnf75xx;
};

typedef union cvmx_dtx_fdeqx_datx cvmx_dtx_fdeqx_datx_t;

/**
 * cvmx_dtx_fdeq#_ena#
 */
union cvmx_dtx_fdeqx_enax {
	u64 u64;
	struct cvmx_dtx_fdeqx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_fdeqx_enax_s cnf75xx;
};

typedef union cvmx_dtx_fdeqx_enax cvmx_dtx_fdeqx_enax_t;

/**
 * cvmx_dtx_fdeq#_sel#
 */
union cvmx_dtx_fdeqx_selx {
	u64 u64;
	struct cvmx_dtx_fdeqx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_fdeqx_selx_s cnf75xx;
};

typedef union cvmx_dtx_fdeqx_selx cvmx_dtx_fdeqx_selx_t;

/**
 * cvmx_dtx_fpa_bcst_rsp
 */
union cvmx_dtx_fpa_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_fpa_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_fpa_bcst_rsp_s cn70xx;
	struct cvmx_dtx_fpa_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_fpa_bcst_rsp_s cn73xx;
	struct cvmx_dtx_fpa_bcst_rsp_s cn78xx;
	struct cvmx_dtx_fpa_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_fpa_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_fpa_bcst_rsp cvmx_dtx_fpa_bcst_rsp_t;

/**
 * cvmx_dtx_fpa_ctl
 */
union cvmx_dtx_fpa_ctl {
	u64 u64;
	struct cvmx_dtx_fpa_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_fpa_ctl_s cn70xx;
	struct cvmx_dtx_fpa_ctl_s cn70xxp1;
	struct cvmx_dtx_fpa_ctl_s cn73xx;
	struct cvmx_dtx_fpa_ctl_s cn78xx;
	struct cvmx_dtx_fpa_ctl_s cn78xxp1;
	struct cvmx_dtx_fpa_ctl_s cnf75xx;
};

typedef union cvmx_dtx_fpa_ctl cvmx_dtx_fpa_ctl_t;

/**
 * cvmx_dtx_fpa_dat#
 */
union cvmx_dtx_fpa_datx {
	u64 u64;
	struct cvmx_dtx_fpa_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_fpa_datx_s cn70xx;
	struct cvmx_dtx_fpa_datx_s cn70xxp1;
	struct cvmx_dtx_fpa_datx_s cn73xx;
	struct cvmx_dtx_fpa_datx_s cn78xx;
	struct cvmx_dtx_fpa_datx_s cn78xxp1;
	struct cvmx_dtx_fpa_datx_s cnf75xx;
};

typedef union cvmx_dtx_fpa_datx cvmx_dtx_fpa_datx_t;

/**
 * cvmx_dtx_fpa_ena#
 */
union cvmx_dtx_fpa_enax {
	u64 u64;
	struct cvmx_dtx_fpa_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_fpa_enax_s cn70xx;
	struct cvmx_dtx_fpa_enax_s cn70xxp1;
	struct cvmx_dtx_fpa_enax_s cn73xx;
	struct cvmx_dtx_fpa_enax_s cn78xx;
	struct cvmx_dtx_fpa_enax_s cn78xxp1;
	struct cvmx_dtx_fpa_enax_s cnf75xx;
};

typedef union cvmx_dtx_fpa_enax cvmx_dtx_fpa_enax_t;

/**
 * cvmx_dtx_fpa_sel#
 */
union cvmx_dtx_fpa_selx {
	u64 u64;
	struct cvmx_dtx_fpa_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_fpa_selx_s cn70xx;
	struct cvmx_dtx_fpa_selx_s cn70xxp1;
	struct cvmx_dtx_fpa_selx_s cn73xx;
	struct cvmx_dtx_fpa_selx_s cn78xx;
	struct cvmx_dtx_fpa_selx_s cn78xxp1;
	struct cvmx_dtx_fpa_selx_s cnf75xx;
};

typedef union cvmx_dtx_fpa_selx cvmx_dtx_fpa_selx_t;

/**
 * cvmx_dtx_gmx#_bcst_rsp
 */
union cvmx_dtx_gmxx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_gmxx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_gmxx_bcst_rsp_s cn70xx;
	struct cvmx_dtx_gmxx_bcst_rsp_s cn70xxp1;
};

typedef union cvmx_dtx_gmxx_bcst_rsp cvmx_dtx_gmxx_bcst_rsp_t;

/**
 * cvmx_dtx_gmx#_ctl
 */
union cvmx_dtx_gmxx_ctl {
	u64 u64;
	struct cvmx_dtx_gmxx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_gmxx_ctl_s cn70xx;
	struct cvmx_dtx_gmxx_ctl_s cn70xxp1;
};

typedef union cvmx_dtx_gmxx_ctl cvmx_dtx_gmxx_ctl_t;

/**
 * cvmx_dtx_gmx#_dat#
 */
union cvmx_dtx_gmxx_datx {
	u64 u64;
	struct cvmx_dtx_gmxx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_gmxx_datx_s cn70xx;
	struct cvmx_dtx_gmxx_datx_s cn70xxp1;
};

typedef union cvmx_dtx_gmxx_datx cvmx_dtx_gmxx_datx_t;

/**
 * cvmx_dtx_gmx#_ena#
 */
union cvmx_dtx_gmxx_enax {
	u64 u64;
	struct cvmx_dtx_gmxx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_gmxx_enax_s cn70xx;
	struct cvmx_dtx_gmxx_enax_s cn70xxp1;
};

typedef union cvmx_dtx_gmxx_enax cvmx_dtx_gmxx_enax_t;

/**
 * cvmx_dtx_gmx#_sel#
 */
union cvmx_dtx_gmxx_selx {
	u64 u64;
	struct cvmx_dtx_gmxx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_gmxx_selx_s cn70xx;
	struct cvmx_dtx_gmxx_selx_s cn70xxp1;
};

typedef union cvmx_dtx_gmxx_selx cvmx_dtx_gmxx_selx_t;

/**
 * cvmx_dtx_gser#_bcst_rsp
 */
union cvmx_dtx_gserx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_gserx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_gserx_bcst_rsp_s cn73xx;
	struct cvmx_dtx_gserx_bcst_rsp_s cn78xx;
	struct cvmx_dtx_gserx_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_gserx_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_gserx_bcst_rsp cvmx_dtx_gserx_bcst_rsp_t;

/**
 * cvmx_dtx_gser#_ctl
 */
union cvmx_dtx_gserx_ctl {
	u64 u64;
	struct cvmx_dtx_gserx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_gserx_ctl_s cn73xx;
	struct cvmx_dtx_gserx_ctl_s cn78xx;
	struct cvmx_dtx_gserx_ctl_s cn78xxp1;
	struct cvmx_dtx_gserx_ctl_s cnf75xx;
};

typedef union cvmx_dtx_gserx_ctl cvmx_dtx_gserx_ctl_t;

/**
 * cvmx_dtx_gser#_dat#
 */
union cvmx_dtx_gserx_datx {
	u64 u64;
	struct cvmx_dtx_gserx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_gserx_datx_s cn73xx;
	struct cvmx_dtx_gserx_datx_s cn78xx;
	struct cvmx_dtx_gserx_datx_s cn78xxp1;
	struct cvmx_dtx_gserx_datx_s cnf75xx;
};

typedef union cvmx_dtx_gserx_datx cvmx_dtx_gserx_datx_t;

/**
 * cvmx_dtx_gser#_ena#
 */
union cvmx_dtx_gserx_enax {
	u64 u64;
	struct cvmx_dtx_gserx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_gserx_enax_s cn73xx;
	struct cvmx_dtx_gserx_enax_s cn78xx;
	struct cvmx_dtx_gserx_enax_s cn78xxp1;
	struct cvmx_dtx_gserx_enax_s cnf75xx;
};

typedef union cvmx_dtx_gserx_enax cvmx_dtx_gserx_enax_t;

/**
 * cvmx_dtx_gser#_sel#
 */
union cvmx_dtx_gserx_selx {
	u64 u64;
	struct cvmx_dtx_gserx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_gserx_selx_s cn73xx;
	struct cvmx_dtx_gserx_selx_s cn78xx;
	struct cvmx_dtx_gserx_selx_s cn78xxp1;
	struct cvmx_dtx_gserx_selx_s cnf75xx;
};

typedef union cvmx_dtx_gserx_selx cvmx_dtx_gserx_selx_t;

/**
 * cvmx_dtx_hna_bcst_rsp
 */
union cvmx_dtx_hna_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_hna_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_hna_bcst_rsp_s cn73xx;
	struct cvmx_dtx_hna_bcst_rsp_s cn78xx;
	struct cvmx_dtx_hna_bcst_rsp_s cn78xxp1;
};

typedef union cvmx_dtx_hna_bcst_rsp cvmx_dtx_hna_bcst_rsp_t;

/**
 * cvmx_dtx_hna_ctl
 */
union cvmx_dtx_hna_ctl {
	u64 u64;
	struct cvmx_dtx_hna_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_hna_ctl_s cn73xx;
	struct cvmx_dtx_hna_ctl_s cn78xx;
	struct cvmx_dtx_hna_ctl_s cn78xxp1;
};

typedef union cvmx_dtx_hna_ctl cvmx_dtx_hna_ctl_t;

/**
 * cvmx_dtx_hna_dat#
 */
union cvmx_dtx_hna_datx {
	u64 u64;
	struct cvmx_dtx_hna_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_hna_datx_s cn73xx;
	struct cvmx_dtx_hna_datx_s cn78xx;
	struct cvmx_dtx_hna_datx_s cn78xxp1;
};

typedef union cvmx_dtx_hna_datx cvmx_dtx_hna_datx_t;

/**
 * cvmx_dtx_hna_ena#
 */
union cvmx_dtx_hna_enax {
	u64 u64;
	struct cvmx_dtx_hna_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_hna_enax_s cn73xx;
	struct cvmx_dtx_hna_enax_s cn78xx;
	struct cvmx_dtx_hna_enax_s cn78xxp1;
};

typedef union cvmx_dtx_hna_enax cvmx_dtx_hna_enax_t;

/**
 * cvmx_dtx_hna_sel#
 */
union cvmx_dtx_hna_selx {
	u64 u64;
	struct cvmx_dtx_hna_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_hna_selx_s cn73xx;
	struct cvmx_dtx_hna_selx_s cn78xx;
	struct cvmx_dtx_hna_selx_s cn78xxp1;
};

typedef union cvmx_dtx_hna_selx cvmx_dtx_hna_selx_t;

/**
 * cvmx_dtx_ila_bcst_rsp
 */
union cvmx_dtx_ila_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_ila_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_ila_bcst_rsp_s cn78xx;
	struct cvmx_dtx_ila_bcst_rsp_s cn78xxp1;
};

typedef union cvmx_dtx_ila_bcst_rsp cvmx_dtx_ila_bcst_rsp_t;

/**
 * cvmx_dtx_ila_ctl
 */
union cvmx_dtx_ila_ctl {
	u64 u64;
	struct cvmx_dtx_ila_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_ila_ctl_s cn78xx;
	struct cvmx_dtx_ila_ctl_s cn78xxp1;
};

typedef union cvmx_dtx_ila_ctl cvmx_dtx_ila_ctl_t;

/**
 * cvmx_dtx_ila_dat#
 */
union cvmx_dtx_ila_datx {
	u64 u64;
	struct cvmx_dtx_ila_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_ila_datx_s cn78xx;
	struct cvmx_dtx_ila_datx_s cn78xxp1;
};

typedef union cvmx_dtx_ila_datx cvmx_dtx_ila_datx_t;

/**
 * cvmx_dtx_ila_ena#
 */
union cvmx_dtx_ila_enax {
	u64 u64;
	struct cvmx_dtx_ila_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_ila_enax_s cn78xx;
	struct cvmx_dtx_ila_enax_s cn78xxp1;
};

typedef union cvmx_dtx_ila_enax cvmx_dtx_ila_enax_t;

/**
 * cvmx_dtx_ila_sel#
 */
union cvmx_dtx_ila_selx {
	u64 u64;
	struct cvmx_dtx_ila_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_ila_selx_s cn78xx;
	struct cvmx_dtx_ila_selx_s cn78xxp1;
};

typedef union cvmx_dtx_ila_selx cvmx_dtx_ila_selx_t;

/**
 * cvmx_dtx_ilk_bcst_rsp
 */
union cvmx_dtx_ilk_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_ilk_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_ilk_bcst_rsp_s cn78xx;
	struct cvmx_dtx_ilk_bcst_rsp_s cn78xxp1;
};

typedef union cvmx_dtx_ilk_bcst_rsp cvmx_dtx_ilk_bcst_rsp_t;

/**
 * cvmx_dtx_ilk_ctl
 */
union cvmx_dtx_ilk_ctl {
	u64 u64;
	struct cvmx_dtx_ilk_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_ilk_ctl_s cn78xx;
	struct cvmx_dtx_ilk_ctl_s cn78xxp1;
};

typedef union cvmx_dtx_ilk_ctl cvmx_dtx_ilk_ctl_t;

/**
 * cvmx_dtx_ilk_dat#
 */
union cvmx_dtx_ilk_datx {
	u64 u64;
	struct cvmx_dtx_ilk_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_ilk_datx_s cn78xx;
	struct cvmx_dtx_ilk_datx_s cn78xxp1;
};

typedef union cvmx_dtx_ilk_datx cvmx_dtx_ilk_datx_t;

/**
 * cvmx_dtx_ilk_ena#
 */
union cvmx_dtx_ilk_enax {
	u64 u64;
	struct cvmx_dtx_ilk_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_ilk_enax_s cn78xx;
	struct cvmx_dtx_ilk_enax_s cn78xxp1;
};

typedef union cvmx_dtx_ilk_enax cvmx_dtx_ilk_enax_t;

/**
 * cvmx_dtx_ilk_sel#
 */
union cvmx_dtx_ilk_selx {
	u64 u64;
	struct cvmx_dtx_ilk_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_ilk_selx_s cn78xx;
	struct cvmx_dtx_ilk_selx_s cn78xxp1;
};

typedef union cvmx_dtx_ilk_selx cvmx_dtx_ilk_selx_t;

/**
 * cvmx_dtx_iob_bcst_rsp
 */
union cvmx_dtx_iob_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_iob_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_iob_bcst_rsp_s cn70xx;
	struct cvmx_dtx_iob_bcst_rsp_s cn70xxp1;
};

typedef union cvmx_dtx_iob_bcst_rsp cvmx_dtx_iob_bcst_rsp_t;

/**
 * cvmx_dtx_iob_ctl
 */
union cvmx_dtx_iob_ctl {
	u64 u64;
	struct cvmx_dtx_iob_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_iob_ctl_s cn70xx;
	struct cvmx_dtx_iob_ctl_s cn70xxp1;
};

typedef union cvmx_dtx_iob_ctl cvmx_dtx_iob_ctl_t;

/**
 * cvmx_dtx_iob_dat#
 */
union cvmx_dtx_iob_datx {
	u64 u64;
	struct cvmx_dtx_iob_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_iob_datx_s cn70xx;
	struct cvmx_dtx_iob_datx_s cn70xxp1;
};

typedef union cvmx_dtx_iob_datx cvmx_dtx_iob_datx_t;

/**
 * cvmx_dtx_iob_ena#
 */
union cvmx_dtx_iob_enax {
	u64 u64;
	struct cvmx_dtx_iob_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_iob_enax_s cn70xx;
	struct cvmx_dtx_iob_enax_s cn70xxp1;
};

typedef union cvmx_dtx_iob_enax cvmx_dtx_iob_enax_t;

/**
 * cvmx_dtx_iob_sel#
 */
union cvmx_dtx_iob_selx {
	u64 u64;
	struct cvmx_dtx_iob_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_iob_selx_s cn70xx;
	struct cvmx_dtx_iob_selx_s cn70xxp1;
};

typedef union cvmx_dtx_iob_selx cvmx_dtx_iob_selx_t;

/**
 * cvmx_dtx_iobn_bcst_rsp
 */
union cvmx_dtx_iobn_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_iobn_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_iobn_bcst_rsp_s cn73xx;
	struct cvmx_dtx_iobn_bcst_rsp_s cn78xx;
	struct cvmx_dtx_iobn_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_iobn_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_iobn_bcst_rsp cvmx_dtx_iobn_bcst_rsp_t;

/**
 * cvmx_dtx_iobn_ctl
 */
union cvmx_dtx_iobn_ctl {
	u64 u64;
	struct cvmx_dtx_iobn_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_iobn_ctl_s cn73xx;
	struct cvmx_dtx_iobn_ctl_s cn78xx;
	struct cvmx_dtx_iobn_ctl_s cn78xxp1;
	struct cvmx_dtx_iobn_ctl_s cnf75xx;
};

typedef union cvmx_dtx_iobn_ctl cvmx_dtx_iobn_ctl_t;

/**
 * cvmx_dtx_iobn_dat#
 */
union cvmx_dtx_iobn_datx {
	u64 u64;
	struct cvmx_dtx_iobn_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_iobn_datx_s cn73xx;
	struct cvmx_dtx_iobn_datx_s cn78xx;
	struct cvmx_dtx_iobn_datx_s cn78xxp1;
	struct cvmx_dtx_iobn_datx_s cnf75xx;
};

typedef union cvmx_dtx_iobn_datx cvmx_dtx_iobn_datx_t;

/**
 * cvmx_dtx_iobn_ena#
 */
union cvmx_dtx_iobn_enax {
	u64 u64;
	struct cvmx_dtx_iobn_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_iobn_enax_s cn73xx;
	struct cvmx_dtx_iobn_enax_s cn78xx;
	struct cvmx_dtx_iobn_enax_s cn78xxp1;
	struct cvmx_dtx_iobn_enax_s cnf75xx;
};

typedef union cvmx_dtx_iobn_enax cvmx_dtx_iobn_enax_t;

/**
 * cvmx_dtx_iobn_sel#
 */
union cvmx_dtx_iobn_selx {
	u64 u64;
	struct cvmx_dtx_iobn_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_iobn_selx_s cn73xx;
	struct cvmx_dtx_iobn_selx_s cn78xx;
	struct cvmx_dtx_iobn_selx_s cn78xxp1;
	struct cvmx_dtx_iobn_selx_s cnf75xx;
};

typedef union cvmx_dtx_iobn_selx cvmx_dtx_iobn_selx_t;

/**
 * cvmx_dtx_iobp_bcst_rsp
 */
union cvmx_dtx_iobp_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_iobp_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_iobp_bcst_rsp_s cn73xx;
	struct cvmx_dtx_iobp_bcst_rsp_s cn78xx;
	struct cvmx_dtx_iobp_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_iobp_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_iobp_bcst_rsp cvmx_dtx_iobp_bcst_rsp_t;

/**
 * cvmx_dtx_iobp_ctl
 */
union cvmx_dtx_iobp_ctl {
	u64 u64;
	struct cvmx_dtx_iobp_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_iobp_ctl_s cn73xx;
	struct cvmx_dtx_iobp_ctl_s cn78xx;
	struct cvmx_dtx_iobp_ctl_s cn78xxp1;
	struct cvmx_dtx_iobp_ctl_s cnf75xx;
};

typedef union cvmx_dtx_iobp_ctl cvmx_dtx_iobp_ctl_t;

/**
 * cvmx_dtx_iobp_dat#
 */
union cvmx_dtx_iobp_datx {
	u64 u64;
	struct cvmx_dtx_iobp_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_iobp_datx_s cn73xx;
	struct cvmx_dtx_iobp_datx_s cn78xx;
	struct cvmx_dtx_iobp_datx_s cn78xxp1;
	struct cvmx_dtx_iobp_datx_s cnf75xx;
};

typedef union cvmx_dtx_iobp_datx cvmx_dtx_iobp_datx_t;

/**
 * cvmx_dtx_iobp_ena#
 */
union cvmx_dtx_iobp_enax {
	u64 u64;
	struct cvmx_dtx_iobp_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_iobp_enax_s cn73xx;
	struct cvmx_dtx_iobp_enax_s cn78xx;
	struct cvmx_dtx_iobp_enax_s cn78xxp1;
	struct cvmx_dtx_iobp_enax_s cnf75xx;
};

typedef union cvmx_dtx_iobp_enax cvmx_dtx_iobp_enax_t;

/**
 * cvmx_dtx_iobp_sel#
 */
union cvmx_dtx_iobp_selx {
	u64 u64;
	struct cvmx_dtx_iobp_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_iobp_selx_s cn73xx;
	struct cvmx_dtx_iobp_selx_s cn78xx;
	struct cvmx_dtx_iobp_selx_s cn78xxp1;
	struct cvmx_dtx_iobp_selx_s cnf75xx;
};

typedef union cvmx_dtx_iobp_selx cvmx_dtx_iobp_selx_t;

/**
 * cvmx_dtx_ipd_bcst_rsp
 */
union cvmx_dtx_ipd_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_ipd_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_ipd_bcst_rsp_s cn70xx;
	struct cvmx_dtx_ipd_bcst_rsp_s cn70xxp1;
};

typedef union cvmx_dtx_ipd_bcst_rsp cvmx_dtx_ipd_bcst_rsp_t;

/**
 * cvmx_dtx_ipd_ctl
 */
union cvmx_dtx_ipd_ctl {
	u64 u64;
	struct cvmx_dtx_ipd_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_ipd_ctl_s cn70xx;
	struct cvmx_dtx_ipd_ctl_s cn70xxp1;
};

typedef union cvmx_dtx_ipd_ctl cvmx_dtx_ipd_ctl_t;

/**
 * cvmx_dtx_ipd_dat#
 */
union cvmx_dtx_ipd_datx {
	u64 u64;
	struct cvmx_dtx_ipd_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_ipd_datx_s cn70xx;
	struct cvmx_dtx_ipd_datx_s cn70xxp1;
};

typedef union cvmx_dtx_ipd_datx cvmx_dtx_ipd_datx_t;

/**
 * cvmx_dtx_ipd_ena#
 */
union cvmx_dtx_ipd_enax {
	u64 u64;
	struct cvmx_dtx_ipd_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_ipd_enax_s cn70xx;
	struct cvmx_dtx_ipd_enax_s cn70xxp1;
};

typedef union cvmx_dtx_ipd_enax cvmx_dtx_ipd_enax_t;

/**
 * cvmx_dtx_ipd_sel#
 */
union cvmx_dtx_ipd_selx {
	u64 u64;
	struct cvmx_dtx_ipd_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_ipd_selx_s cn70xx;
	struct cvmx_dtx_ipd_selx_s cn70xxp1;
};

typedef union cvmx_dtx_ipd_selx cvmx_dtx_ipd_selx_t;

/**
 * cvmx_dtx_key_bcst_rsp
 */
union cvmx_dtx_key_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_key_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_key_bcst_rsp_s cn70xx;
	struct cvmx_dtx_key_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_key_bcst_rsp_s cn73xx;
	struct cvmx_dtx_key_bcst_rsp_s cn78xx;
	struct cvmx_dtx_key_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_key_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_key_bcst_rsp cvmx_dtx_key_bcst_rsp_t;

/**
 * cvmx_dtx_key_ctl
 */
union cvmx_dtx_key_ctl {
	u64 u64;
	struct cvmx_dtx_key_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_key_ctl_s cn70xx;
	struct cvmx_dtx_key_ctl_s cn70xxp1;
	struct cvmx_dtx_key_ctl_s cn73xx;
	struct cvmx_dtx_key_ctl_s cn78xx;
	struct cvmx_dtx_key_ctl_s cn78xxp1;
	struct cvmx_dtx_key_ctl_s cnf75xx;
};

typedef union cvmx_dtx_key_ctl cvmx_dtx_key_ctl_t;

/**
 * cvmx_dtx_key_dat#
 */
union cvmx_dtx_key_datx {
	u64 u64;
	struct cvmx_dtx_key_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_key_datx_s cn70xx;
	struct cvmx_dtx_key_datx_s cn70xxp1;
	struct cvmx_dtx_key_datx_s cn73xx;
	struct cvmx_dtx_key_datx_s cn78xx;
	struct cvmx_dtx_key_datx_s cn78xxp1;
	struct cvmx_dtx_key_datx_s cnf75xx;
};

typedef union cvmx_dtx_key_datx cvmx_dtx_key_datx_t;

/**
 * cvmx_dtx_key_ena#
 */
union cvmx_dtx_key_enax {
	u64 u64;
	struct cvmx_dtx_key_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_key_enax_s cn70xx;
	struct cvmx_dtx_key_enax_s cn70xxp1;
	struct cvmx_dtx_key_enax_s cn73xx;
	struct cvmx_dtx_key_enax_s cn78xx;
	struct cvmx_dtx_key_enax_s cn78xxp1;
	struct cvmx_dtx_key_enax_s cnf75xx;
};

typedef union cvmx_dtx_key_enax cvmx_dtx_key_enax_t;

/**
 * cvmx_dtx_key_sel#
 */
union cvmx_dtx_key_selx {
	u64 u64;
	struct cvmx_dtx_key_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_key_selx_s cn70xx;
	struct cvmx_dtx_key_selx_s cn70xxp1;
	struct cvmx_dtx_key_selx_s cn73xx;
	struct cvmx_dtx_key_selx_s cn78xx;
	struct cvmx_dtx_key_selx_s cn78xxp1;
	struct cvmx_dtx_key_selx_s cnf75xx;
};

typedef union cvmx_dtx_key_selx cvmx_dtx_key_selx_t;

/**
 * cvmx_dtx_l2c_cbc#_bcst_rsp
 */
union cvmx_dtx_l2c_cbcx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_l2c_cbcx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_l2c_cbcx_bcst_rsp_s cn70xx;
	struct cvmx_dtx_l2c_cbcx_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_l2c_cbcx_bcst_rsp_s cn73xx;
	struct cvmx_dtx_l2c_cbcx_bcst_rsp_s cn78xx;
	struct cvmx_dtx_l2c_cbcx_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_l2c_cbcx_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_l2c_cbcx_bcst_rsp cvmx_dtx_l2c_cbcx_bcst_rsp_t;

/**
 * cvmx_dtx_l2c_cbc#_ctl
 */
union cvmx_dtx_l2c_cbcx_ctl {
	u64 u64;
	struct cvmx_dtx_l2c_cbcx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_l2c_cbcx_ctl_s cn70xx;
	struct cvmx_dtx_l2c_cbcx_ctl_s cn70xxp1;
	struct cvmx_dtx_l2c_cbcx_ctl_s cn73xx;
	struct cvmx_dtx_l2c_cbcx_ctl_s cn78xx;
	struct cvmx_dtx_l2c_cbcx_ctl_s cn78xxp1;
	struct cvmx_dtx_l2c_cbcx_ctl_s cnf75xx;
};

typedef union cvmx_dtx_l2c_cbcx_ctl cvmx_dtx_l2c_cbcx_ctl_t;

/**
 * cvmx_dtx_l2c_cbc#_dat#
 */
union cvmx_dtx_l2c_cbcx_datx {
	u64 u64;
	struct cvmx_dtx_l2c_cbcx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_l2c_cbcx_datx_s cn70xx;
	struct cvmx_dtx_l2c_cbcx_datx_s cn70xxp1;
	struct cvmx_dtx_l2c_cbcx_datx_s cn73xx;
	struct cvmx_dtx_l2c_cbcx_datx_s cn78xx;
	struct cvmx_dtx_l2c_cbcx_datx_s cn78xxp1;
	struct cvmx_dtx_l2c_cbcx_datx_s cnf75xx;
};

typedef union cvmx_dtx_l2c_cbcx_datx cvmx_dtx_l2c_cbcx_datx_t;

/**
 * cvmx_dtx_l2c_cbc#_ena#
 */
union cvmx_dtx_l2c_cbcx_enax {
	u64 u64;
	struct cvmx_dtx_l2c_cbcx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_l2c_cbcx_enax_s cn70xx;
	struct cvmx_dtx_l2c_cbcx_enax_s cn70xxp1;
	struct cvmx_dtx_l2c_cbcx_enax_s cn73xx;
	struct cvmx_dtx_l2c_cbcx_enax_s cn78xx;
	struct cvmx_dtx_l2c_cbcx_enax_s cn78xxp1;
	struct cvmx_dtx_l2c_cbcx_enax_s cnf75xx;
};

typedef union cvmx_dtx_l2c_cbcx_enax cvmx_dtx_l2c_cbcx_enax_t;

/**
 * cvmx_dtx_l2c_cbc#_sel#
 */
union cvmx_dtx_l2c_cbcx_selx {
	u64 u64;
	struct cvmx_dtx_l2c_cbcx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_l2c_cbcx_selx_s cn70xx;
	struct cvmx_dtx_l2c_cbcx_selx_s cn70xxp1;
	struct cvmx_dtx_l2c_cbcx_selx_s cn73xx;
	struct cvmx_dtx_l2c_cbcx_selx_s cn78xx;
	struct cvmx_dtx_l2c_cbcx_selx_s cn78xxp1;
	struct cvmx_dtx_l2c_cbcx_selx_s cnf75xx;
};

typedef union cvmx_dtx_l2c_cbcx_selx cvmx_dtx_l2c_cbcx_selx_t;

/**
 * cvmx_dtx_l2c_mci#_bcst_rsp
 */
union cvmx_dtx_l2c_mcix_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_l2c_mcix_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_l2c_mcix_bcst_rsp_s cn70xx;
	struct cvmx_dtx_l2c_mcix_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_l2c_mcix_bcst_rsp_s cn73xx;
	struct cvmx_dtx_l2c_mcix_bcst_rsp_s cn78xx;
	struct cvmx_dtx_l2c_mcix_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_l2c_mcix_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_l2c_mcix_bcst_rsp cvmx_dtx_l2c_mcix_bcst_rsp_t;

/**
 * cvmx_dtx_l2c_mci#_ctl
 */
union cvmx_dtx_l2c_mcix_ctl {
	u64 u64;
	struct cvmx_dtx_l2c_mcix_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_l2c_mcix_ctl_s cn70xx;
	struct cvmx_dtx_l2c_mcix_ctl_s cn70xxp1;
	struct cvmx_dtx_l2c_mcix_ctl_s cn73xx;
	struct cvmx_dtx_l2c_mcix_ctl_s cn78xx;
	struct cvmx_dtx_l2c_mcix_ctl_s cn78xxp1;
	struct cvmx_dtx_l2c_mcix_ctl_s cnf75xx;
};

typedef union cvmx_dtx_l2c_mcix_ctl cvmx_dtx_l2c_mcix_ctl_t;

/**
 * cvmx_dtx_l2c_mci#_dat#
 */
union cvmx_dtx_l2c_mcix_datx {
	u64 u64;
	struct cvmx_dtx_l2c_mcix_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_l2c_mcix_datx_s cn70xx;
	struct cvmx_dtx_l2c_mcix_datx_s cn70xxp1;
	struct cvmx_dtx_l2c_mcix_datx_s cn73xx;
	struct cvmx_dtx_l2c_mcix_datx_s cn78xx;
	struct cvmx_dtx_l2c_mcix_datx_s cn78xxp1;
	struct cvmx_dtx_l2c_mcix_datx_s cnf75xx;
};

typedef union cvmx_dtx_l2c_mcix_datx cvmx_dtx_l2c_mcix_datx_t;

/**
 * cvmx_dtx_l2c_mci#_ena#
 */
union cvmx_dtx_l2c_mcix_enax {
	u64 u64;
	struct cvmx_dtx_l2c_mcix_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_l2c_mcix_enax_s cn70xx;
	struct cvmx_dtx_l2c_mcix_enax_s cn70xxp1;
	struct cvmx_dtx_l2c_mcix_enax_s cn73xx;
	struct cvmx_dtx_l2c_mcix_enax_s cn78xx;
	struct cvmx_dtx_l2c_mcix_enax_s cn78xxp1;
	struct cvmx_dtx_l2c_mcix_enax_s cnf75xx;
};

typedef union cvmx_dtx_l2c_mcix_enax cvmx_dtx_l2c_mcix_enax_t;

/**
 * cvmx_dtx_l2c_mci#_sel#
 */
union cvmx_dtx_l2c_mcix_selx {
	u64 u64;
	struct cvmx_dtx_l2c_mcix_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_l2c_mcix_selx_s cn70xx;
	struct cvmx_dtx_l2c_mcix_selx_s cn70xxp1;
	struct cvmx_dtx_l2c_mcix_selx_s cn73xx;
	struct cvmx_dtx_l2c_mcix_selx_s cn78xx;
	struct cvmx_dtx_l2c_mcix_selx_s cn78xxp1;
	struct cvmx_dtx_l2c_mcix_selx_s cnf75xx;
};

typedef union cvmx_dtx_l2c_mcix_selx cvmx_dtx_l2c_mcix_selx_t;

/**
 * cvmx_dtx_l2c_tad#_bcst_rsp
 */
union cvmx_dtx_l2c_tadx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_l2c_tadx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_l2c_tadx_bcst_rsp_s cn70xx;
	struct cvmx_dtx_l2c_tadx_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_l2c_tadx_bcst_rsp_s cn73xx;
	struct cvmx_dtx_l2c_tadx_bcst_rsp_s cn78xx;
	struct cvmx_dtx_l2c_tadx_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_l2c_tadx_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_l2c_tadx_bcst_rsp cvmx_dtx_l2c_tadx_bcst_rsp_t;

/**
 * cvmx_dtx_l2c_tad#_ctl
 */
union cvmx_dtx_l2c_tadx_ctl {
	u64 u64;
	struct cvmx_dtx_l2c_tadx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_l2c_tadx_ctl_s cn70xx;
	struct cvmx_dtx_l2c_tadx_ctl_s cn70xxp1;
	struct cvmx_dtx_l2c_tadx_ctl_s cn73xx;
	struct cvmx_dtx_l2c_tadx_ctl_s cn78xx;
	struct cvmx_dtx_l2c_tadx_ctl_s cn78xxp1;
	struct cvmx_dtx_l2c_tadx_ctl_s cnf75xx;
};

typedef union cvmx_dtx_l2c_tadx_ctl cvmx_dtx_l2c_tadx_ctl_t;

/**
 * cvmx_dtx_l2c_tad#_dat#
 */
union cvmx_dtx_l2c_tadx_datx {
	u64 u64;
	struct cvmx_dtx_l2c_tadx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_l2c_tadx_datx_s cn70xx;
	struct cvmx_dtx_l2c_tadx_datx_s cn70xxp1;
	struct cvmx_dtx_l2c_tadx_datx_s cn73xx;
	struct cvmx_dtx_l2c_tadx_datx_s cn78xx;
	struct cvmx_dtx_l2c_tadx_datx_s cn78xxp1;
	struct cvmx_dtx_l2c_tadx_datx_s cnf75xx;
};

typedef union cvmx_dtx_l2c_tadx_datx cvmx_dtx_l2c_tadx_datx_t;

/**
 * cvmx_dtx_l2c_tad#_ena#
 */
union cvmx_dtx_l2c_tadx_enax {
	u64 u64;
	struct cvmx_dtx_l2c_tadx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_l2c_tadx_enax_s cn70xx;
	struct cvmx_dtx_l2c_tadx_enax_s cn70xxp1;
	struct cvmx_dtx_l2c_tadx_enax_s cn73xx;
	struct cvmx_dtx_l2c_tadx_enax_s cn78xx;
	struct cvmx_dtx_l2c_tadx_enax_s cn78xxp1;
	struct cvmx_dtx_l2c_tadx_enax_s cnf75xx;
};

typedef union cvmx_dtx_l2c_tadx_enax cvmx_dtx_l2c_tadx_enax_t;

/**
 * cvmx_dtx_l2c_tad#_sel#
 */
union cvmx_dtx_l2c_tadx_selx {
	u64 u64;
	struct cvmx_dtx_l2c_tadx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_l2c_tadx_selx_s cn70xx;
	struct cvmx_dtx_l2c_tadx_selx_s cn70xxp1;
	struct cvmx_dtx_l2c_tadx_selx_s cn73xx;
	struct cvmx_dtx_l2c_tadx_selx_s cn78xx;
	struct cvmx_dtx_l2c_tadx_selx_s cn78xxp1;
	struct cvmx_dtx_l2c_tadx_selx_s cnf75xx;
};

typedef union cvmx_dtx_l2c_tadx_selx cvmx_dtx_l2c_tadx_selx_t;

/**
 * cvmx_dtx_lap#_bcst_rsp
 */
union cvmx_dtx_lapx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_lapx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_lapx_bcst_rsp_s cn78xx;
	struct cvmx_dtx_lapx_bcst_rsp_s cn78xxp1;
};

typedef union cvmx_dtx_lapx_bcst_rsp cvmx_dtx_lapx_bcst_rsp_t;

/**
 * cvmx_dtx_lap#_ctl
 */
union cvmx_dtx_lapx_ctl {
	u64 u64;
	struct cvmx_dtx_lapx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_lapx_ctl_s cn78xx;
	struct cvmx_dtx_lapx_ctl_s cn78xxp1;
};

typedef union cvmx_dtx_lapx_ctl cvmx_dtx_lapx_ctl_t;

/**
 * cvmx_dtx_lap#_dat#
 */
union cvmx_dtx_lapx_datx {
	u64 u64;
	struct cvmx_dtx_lapx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_lapx_datx_s cn78xx;
	struct cvmx_dtx_lapx_datx_s cn78xxp1;
};

typedef union cvmx_dtx_lapx_datx cvmx_dtx_lapx_datx_t;

/**
 * cvmx_dtx_lap#_ena#
 */
union cvmx_dtx_lapx_enax {
	u64 u64;
	struct cvmx_dtx_lapx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_lapx_enax_s cn78xx;
	struct cvmx_dtx_lapx_enax_s cn78xxp1;
};

typedef union cvmx_dtx_lapx_enax cvmx_dtx_lapx_enax_t;

/**
 * cvmx_dtx_lap#_sel#
 */
union cvmx_dtx_lapx_selx {
	u64 u64;
	struct cvmx_dtx_lapx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_lapx_selx_s cn78xx;
	struct cvmx_dtx_lapx_selx_s cn78xxp1;
};

typedef union cvmx_dtx_lapx_selx cvmx_dtx_lapx_selx_t;

/**
 * cvmx_dtx_lbk_bcst_rsp
 */
union cvmx_dtx_lbk_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_lbk_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_lbk_bcst_rsp_s cn73xx;
	struct cvmx_dtx_lbk_bcst_rsp_s cn78xx;
	struct cvmx_dtx_lbk_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_lbk_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_lbk_bcst_rsp cvmx_dtx_lbk_bcst_rsp_t;

/**
 * cvmx_dtx_lbk_ctl
 */
union cvmx_dtx_lbk_ctl {
	u64 u64;
	struct cvmx_dtx_lbk_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_lbk_ctl_s cn73xx;
	struct cvmx_dtx_lbk_ctl_s cn78xx;
	struct cvmx_dtx_lbk_ctl_s cn78xxp1;
	struct cvmx_dtx_lbk_ctl_s cnf75xx;
};

typedef union cvmx_dtx_lbk_ctl cvmx_dtx_lbk_ctl_t;

/**
 * cvmx_dtx_lbk_dat#
 */
union cvmx_dtx_lbk_datx {
	u64 u64;
	struct cvmx_dtx_lbk_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_lbk_datx_s cn73xx;
	struct cvmx_dtx_lbk_datx_s cn78xx;
	struct cvmx_dtx_lbk_datx_s cn78xxp1;
	struct cvmx_dtx_lbk_datx_s cnf75xx;
};

typedef union cvmx_dtx_lbk_datx cvmx_dtx_lbk_datx_t;

/**
 * cvmx_dtx_lbk_ena#
 */
union cvmx_dtx_lbk_enax {
	u64 u64;
	struct cvmx_dtx_lbk_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_lbk_enax_s cn73xx;
	struct cvmx_dtx_lbk_enax_s cn78xx;
	struct cvmx_dtx_lbk_enax_s cn78xxp1;
	struct cvmx_dtx_lbk_enax_s cnf75xx;
};

typedef union cvmx_dtx_lbk_enax cvmx_dtx_lbk_enax_t;

/**
 * cvmx_dtx_lbk_sel#
 */
union cvmx_dtx_lbk_selx {
	u64 u64;
	struct cvmx_dtx_lbk_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_lbk_selx_s cn73xx;
	struct cvmx_dtx_lbk_selx_s cn78xx;
	struct cvmx_dtx_lbk_selx_s cn78xxp1;
	struct cvmx_dtx_lbk_selx_s cnf75xx;
};

typedef union cvmx_dtx_lbk_selx cvmx_dtx_lbk_selx_t;

/**
 * cvmx_dtx_lmc#_bcst_rsp
 */
union cvmx_dtx_lmcx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_lmcx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_lmcx_bcst_rsp_s cn70xx;
	struct cvmx_dtx_lmcx_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_lmcx_bcst_rsp_s cn73xx;
	struct cvmx_dtx_lmcx_bcst_rsp_s cn78xx;
	struct cvmx_dtx_lmcx_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_lmcx_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_lmcx_bcst_rsp cvmx_dtx_lmcx_bcst_rsp_t;

/**
 * cvmx_dtx_lmc#_ctl
 */
union cvmx_dtx_lmcx_ctl {
	u64 u64;
	struct cvmx_dtx_lmcx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_lmcx_ctl_s cn70xx;
	struct cvmx_dtx_lmcx_ctl_s cn70xxp1;
	struct cvmx_dtx_lmcx_ctl_s cn73xx;
	struct cvmx_dtx_lmcx_ctl_s cn78xx;
	struct cvmx_dtx_lmcx_ctl_s cn78xxp1;
	struct cvmx_dtx_lmcx_ctl_s cnf75xx;
};

typedef union cvmx_dtx_lmcx_ctl cvmx_dtx_lmcx_ctl_t;

/**
 * cvmx_dtx_lmc#_dat#
 */
union cvmx_dtx_lmcx_datx {
	u64 u64;
	struct cvmx_dtx_lmcx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_lmcx_datx_s cn70xx;
	struct cvmx_dtx_lmcx_datx_s cn70xxp1;
	struct cvmx_dtx_lmcx_datx_s cn73xx;
	struct cvmx_dtx_lmcx_datx_s cn78xx;
	struct cvmx_dtx_lmcx_datx_s cn78xxp1;
	struct cvmx_dtx_lmcx_datx_s cnf75xx;
};

typedef union cvmx_dtx_lmcx_datx cvmx_dtx_lmcx_datx_t;

/**
 * cvmx_dtx_lmc#_ena#
 */
union cvmx_dtx_lmcx_enax {
	u64 u64;
	struct cvmx_dtx_lmcx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_lmcx_enax_s cn70xx;
	struct cvmx_dtx_lmcx_enax_s cn70xxp1;
	struct cvmx_dtx_lmcx_enax_s cn73xx;
	struct cvmx_dtx_lmcx_enax_s cn78xx;
	struct cvmx_dtx_lmcx_enax_s cn78xxp1;
	struct cvmx_dtx_lmcx_enax_s cnf75xx;
};

typedef union cvmx_dtx_lmcx_enax cvmx_dtx_lmcx_enax_t;

/**
 * cvmx_dtx_lmc#_sel#
 */
union cvmx_dtx_lmcx_selx {
	u64 u64;
	struct cvmx_dtx_lmcx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_lmcx_selx_s cn70xx;
	struct cvmx_dtx_lmcx_selx_s cn70xxp1;
	struct cvmx_dtx_lmcx_selx_s cn73xx;
	struct cvmx_dtx_lmcx_selx_s cn78xx;
	struct cvmx_dtx_lmcx_selx_s cn78xxp1;
	struct cvmx_dtx_lmcx_selx_s cnf75xx;
};

typedef union cvmx_dtx_lmcx_selx cvmx_dtx_lmcx_selx_t;

/**
 * cvmx_dtx_mdb#_bcst_rsp
 */
union cvmx_dtx_mdbx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_mdbx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_mdbx_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_mdbx_bcst_rsp cvmx_dtx_mdbx_bcst_rsp_t;

/**
 * cvmx_dtx_mdb#_ctl
 */
union cvmx_dtx_mdbx_ctl {
	u64 u64;
	struct cvmx_dtx_mdbx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_mdbx_ctl_s cnf75xx;
};

typedef union cvmx_dtx_mdbx_ctl cvmx_dtx_mdbx_ctl_t;

/**
 * cvmx_dtx_mdb#_dat#
 */
union cvmx_dtx_mdbx_datx {
	u64 u64;
	struct cvmx_dtx_mdbx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_mdbx_datx_s cnf75xx;
};

typedef union cvmx_dtx_mdbx_datx cvmx_dtx_mdbx_datx_t;

/**
 * cvmx_dtx_mdb#_ena#
 */
union cvmx_dtx_mdbx_enax {
	u64 u64;
	struct cvmx_dtx_mdbx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_mdbx_enax_s cnf75xx;
};

typedef union cvmx_dtx_mdbx_enax cvmx_dtx_mdbx_enax_t;

/**
 * cvmx_dtx_mdb#_sel#
 */
union cvmx_dtx_mdbx_selx {
	u64 u64;
	struct cvmx_dtx_mdbx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_mdbx_selx_s cnf75xx;
};

typedef union cvmx_dtx_mdbx_selx cvmx_dtx_mdbx_selx_t;

/**
 * cvmx_dtx_mhbw_bcst_rsp
 */
union cvmx_dtx_mhbw_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_mhbw_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_mhbw_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_mhbw_bcst_rsp cvmx_dtx_mhbw_bcst_rsp_t;

/**
 * cvmx_dtx_mhbw_ctl
 */
union cvmx_dtx_mhbw_ctl {
	u64 u64;
	struct cvmx_dtx_mhbw_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_mhbw_ctl_s cnf75xx;
};

typedef union cvmx_dtx_mhbw_ctl cvmx_dtx_mhbw_ctl_t;

/**
 * cvmx_dtx_mhbw_dat#
 */
union cvmx_dtx_mhbw_datx {
	u64 u64;
	struct cvmx_dtx_mhbw_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_mhbw_datx_s cnf75xx;
};

typedef union cvmx_dtx_mhbw_datx cvmx_dtx_mhbw_datx_t;

/**
 * cvmx_dtx_mhbw_ena#
 */
union cvmx_dtx_mhbw_enax {
	u64 u64;
	struct cvmx_dtx_mhbw_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_mhbw_enax_s cnf75xx;
};

typedef union cvmx_dtx_mhbw_enax cvmx_dtx_mhbw_enax_t;

/**
 * cvmx_dtx_mhbw_sel#
 */
union cvmx_dtx_mhbw_selx {
	u64 u64;
	struct cvmx_dtx_mhbw_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_mhbw_selx_s cnf75xx;
};

typedef union cvmx_dtx_mhbw_selx cvmx_dtx_mhbw_selx_t;

/**
 * cvmx_dtx_mio_bcst_rsp
 */
union cvmx_dtx_mio_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_mio_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_mio_bcst_rsp_s cn70xx;
	struct cvmx_dtx_mio_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_mio_bcst_rsp_s cn73xx;
	struct cvmx_dtx_mio_bcst_rsp_s cn78xx;
	struct cvmx_dtx_mio_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_mio_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_mio_bcst_rsp cvmx_dtx_mio_bcst_rsp_t;

/**
 * cvmx_dtx_mio_ctl
 */
union cvmx_dtx_mio_ctl {
	u64 u64;
	struct cvmx_dtx_mio_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_mio_ctl_s cn70xx;
	struct cvmx_dtx_mio_ctl_s cn70xxp1;
	struct cvmx_dtx_mio_ctl_s cn73xx;
	struct cvmx_dtx_mio_ctl_s cn78xx;
	struct cvmx_dtx_mio_ctl_s cn78xxp1;
	struct cvmx_dtx_mio_ctl_s cnf75xx;
};

typedef union cvmx_dtx_mio_ctl cvmx_dtx_mio_ctl_t;

/**
 * cvmx_dtx_mio_dat#
 */
union cvmx_dtx_mio_datx {
	u64 u64;
	struct cvmx_dtx_mio_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_mio_datx_s cn70xx;
	struct cvmx_dtx_mio_datx_s cn70xxp1;
	struct cvmx_dtx_mio_datx_s cn73xx;
	struct cvmx_dtx_mio_datx_s cn78xx;
	struct cvmx_dtx_mio_datx_s cn78xxp1;
	struct cvmx_dtx_mio_datx_s cnf75xx;
};

typedef union cvmx_dtx_mio_datx cvmx_dtx_mio_datx_t;

/**
 * cvmx_dtx_mio_ena#
 */
union cvmx_dtx_mio_enax {
	u64 u64;
	struct cvmx_dtx_mio_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_mio_enax_s cn70xx;
	struct cvmx_dtx_mio_enax_s cn70xxp1;
	struct cvmx_dtx_mio_enax_s cn73xx;
	struct cvmx_dtx_mio_enax_s cn78xx;
	struct cvmx_dtx_mio_enax_s cn78xxp1;
	struct cvmx_dtx_mio_enax_s cnf75xx;
};

typedef union cvmx_dtx_mio_enax cvmx_dtx_mio_enax_t;

/**
 * cvmx_dtx_mio_sel#
 */
union cvmx_dtx_mio_selx {
	u64 u64;
	struct cvmx_dtx_mio_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_mio_selx_s cn70xx;
	struct cvmx_dtx_mio_selx_s cn70xxp1;
	struct cvmx_dtx_mio_selx_s cn73xx;
	struct cvmx_dtx_mio_selx_s cn78xx;
	struct cvmx_dtx_mio_selx_s cn78xxp1;
	struct cvmx_dtx_mio_selx_s cnf75xx;
};

typedef union cvmx_dtx_mio_selx cvmx_dtx_mio_selx_t;

/**
 * cvmx_dtx_ocx_bot_bcst_rsp
 */
union cvmx_dtx_ocx_bot_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_ocx_bot_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_ocx_bot_bcst_rsp_s cn78xx;
	struct cvmx_dtx_ocx_bot_bcst_rsp_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_bot_bcst_rsp cvmx_dtx_ocx_bot_bcst_rsp_t;

/**
 * cvmx_dtx_ocx_bot_ctl
 */
union cvmx_dtx_ocx_bot_ctl {
	u64 u64;
	struct cvmx_dtx_ocx_bot_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_ocx_bot_ctl_s cn78xx;
	struct cvmx_dtx_ocx_bot_ctl_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_bot_ctl cvmx_dtx_ocx_bot_ctl_t;

/**
 * cvmx_dtx_ocx_bot_dat#
 */
union cvmx_dtx_ocx_bot_datx {
	u64 u64;
	struct cvmx_dtx_ocx_bot_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_ocx_bot_datx_s cn78xx;
	struct cvmx_dtx_ocx_bot_datx_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_bot_datx cvmx_dtx_ocx_bot_datx_t;

/**
 * cvmx_dtx_ocx_bot_ena#
 */
union cvmx_dtx_ocx_bot_enax {
	u64 u64;
	struct cvmx_dtx_ocx_bot_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_ocx_bot_enax_s cn78xx;
	struct cvmx_dtx_ocx_bot_enax_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_bot_enax cvmx_dtx_ocx_bot_enax_t;

/**
 * cvmx_dtx_ocx_bot_sel#
 */
union cvmx_dtx_ocx_bot_selx {
	u64 u64;
	struct cvmx_dtx_ocx_bot_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_ocx_bot_selx_s cn78xx;
	struct cvmx_dtx_ocx_bot_selx_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_bot_selx cvmx_dtx_ocx_bot_selx_t;

/**
 * cvmx_dtx_ocx_lnk#_bcst_rsp
 */
union cvmx_dtx_ocx_lnkx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_ocx_lnkx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_ocx_lnkx_bcst_rsp_s cn78xx;
	struct cvmx_dtx_ocx_lnkx_bcst_rsp_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_lnkx_bcst_rsp cvmx_dtx_ocx_lnkx_bcst_rsp_t;

/**
 * cvmx_dtx_ocx_lnk#_ctl
 */
union cvmx_dtx_ocx_lnkx_ctl {
	u64 u64;
	struct cvmx_dtx_ocx_lnkx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_ocx_lnkx_ctl_s cn78xx;
	struct cvmx_dtx_ocx_lnkx_ctl_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_lnkx_ctl cvmx_dtx_ocx_lnkx_ctl_t;

/**
 * cvmx_dtx_ocx_lnk#_dat#
 */
union cvmx_dtx_ocx_lnkx_datx {
	u64 u64;
	struct cvmx_dtx_ocx_lnkx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_ocx_lnkx_datx_s cn78xx;
	struct cvmx_dtx_ocx_lnkx_datx_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_lnkx_datx cvmx_dtx_ocx_lnkx_datx_t;

/**
 * cvmx_dtx_ocx_lnk#_ena#
 */
union cvmx_dtx_ocx_lnkx_enax {
	u64 u64;
	struct cvmx_dtx_ocx_lnkx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_ocx_lnkx_enax_s cn78xx;
	struct cvmx_dtx_ocx_lnkx_enax_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_lnkx_enax cvmx_dtx_ocx_lnkx_enax_t;

/**
 * cvmx_dtx_ocx_lnk#_sel#
 */
union cvmx_dtx_ocx_lnkx_selx {
	u64 u64;
	struct cvmx_dtx_ocx_lnkx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_ocx_lnkx_selx_s cn78xx;
	struct cvmx_dtx_ocx_lnkx_selx_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_lnkx_selx cvmx_dtx_ocx_lnkx_selx_t;

/**
 * cvmx_dtx_ocx_ole#_bcst_rsp
 */
union cvmx_dtx_ocx_olex_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_ocx_olex_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_ocx_olex_bcst_rsp_s cn78xx;
	struct cvmx_dtx_ocx_olex_bcst_rsp_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_olex_bcst_rsp cvmx_dtx_ocx_olex_bcst_rsp_t;

/**
 * cvmx_dtx_ocx_ole#_ctl
 */
union cvmx_dtx_ocx_olex_ctl {
	u64 u64;
	struct cvmx_dtx_ocx_olex_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_ocx_olex_ctl_s cn78xx;
	struct cvmx_dtx_ocx_olex_ctl_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_olex_ctl cvmx_dtx_ocx_olex_ctl_t;

/**
 * cvmx_dtx_ocx_ole#_dat#
 */
union cvmx_dtx_ocx_olex_datx {
	u64 u64;
	struct cvmx_dtx_ocx_olex_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_ocx_olex_datx_s cn78xx;
	struct cvmx_dtx_ocx_olex_datx_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_olex_datx cvmx_dtx_ocx_olex_datx_t;

/**
 * cvmx_dtx_ocx_ole#_ena#
 */
union cvmx_dtx_ocx_olex_enax {
	u64 u64;
	struct cvmx_dtx_ocx_olex_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_ocx_olex_enax_s cn78xx;
	struct cvmx_dtx_ocx_olex_enax_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_olex_enax cvmx_dtx_ocx_olex_enax_t;

/**
 * cvmx_dtx_ocx_ole#_sel#
 */
union cvmx_dtx_ocx_olex_selx {
	u64 u64;
	struct cvmx_dtx_ocx_olex_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_ocx_olex_selx_s cn78xx;
	struct cvmx_dtx_ocx_olex_selx_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_olex_selx cvmx_dtx_ocx_olex_selx_t;

/**
 * cvmx_dtx_ocx_top_bcst_rsp
 */
union cvmx_dtx_ocx_top_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_ocx_top_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_ocx_top_bcst_rsp_s cn78xx;
	struct cvmx_dtx_ocx_top_bcst_rsp_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_top_bcst_rsp cvmx_dtx_ocx_top_bcst_rsp_t;

/**
 * cvmx_dtx_ocx_top_ctl
 */
union cvmx_dtx_ocx_top_ctl {
	u64 u64;
	struct cvmx_dtx_ocx_top_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_ocx_top_ctl_s cn78xx;
	struct cvmx_dtx_ocx_top_ctl_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_top_ctl cvmx_dtx_ocx_top_ctl_t;

/**
 * cvmx_dtx_ocx_top_dat#
 */
union cvmx_dtx_ocx_top_datx {
	u64 u64;
	struct cvmx_dtx_ocx_top_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_ocx_top_datx_s cn78xx;
	struct cvmx_dtx_ocx_top_datx_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_top_datx cvmx_dtx_ocx_top_datx_t;

/**
 * cvmx_dtx_ocx_top_ena#
 */
union cvmx_dtx_ocx_top_enax {
	u64 u64;
	struct cvmx_dtx_ocx_top_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_ocx_top_enax_s cn78xx;
	struct cvmx_dtx_ocx_top_enax_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_top_enax cvmx_dtx_ocx_top_enax_t;

/**
 * cvmx_dtx_ocx_top_sel#
 */
union cvmx_dtx_ocx_top_selx {
	u64 u64;
	struct cvmx_dtx_ocx_top_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_ocx_top_selx_s cn78xx;
	struct cvmx_dtx_ocx_top_selx_s cn78xxp1;
};

typedef union cvmx_dtx_ocx_top_selx cvmx_dtx_ocx_top_selx_t;

/**
 * cvmx_dtx_osm_bcst_rsp
 */
union cvmx_dtx_osm_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_osm_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_osm_bcst_rsp_s cn73xx;
	struct cvmx_dtx_osm_bcst_rsp_s cn78xx;
	struct cvmx_dtx_osm_bcst_rsp_s cn78xxp1;
};

typedef union cvmx_dtx_osm_bcst_rsp cvmx_dtx_osm_bcst_rsp_t;

/**
 * cvmx_dtx_osm_ctl
 */
union cvmx_dtx_osm_ctl {
	u64 u64;
	struct cvmx_dtx_osm_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_osm_ctl_s cn73xx;
	struct cvmx_dtx_osm_ctl_s cn78xx;
	struct cvmx_dtx_osm_ctl_s cn78xxp1;
};

typedef union cvmx_dtx_osm_ctl cvmx_dtx_osm_ctl_t;

/**
 * cvmx_dtx_osm_dat#
 */
union cvmx_dtx_osm_datx {
	u64 u64;
	struct cvmx_dtx_osm_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_osm_datx_s cn73xx;
	struct cvmx_dtx_osm_datx_s cn78xx;
	struct cvmx_dtx_osm_datx_s cn78xxp1;
};

typedef union cvmx_dtx_osm_datx cvmx_dtx_osm_datx_t;

/**
 * cvmx_dtx_osm_ena#
 */
union cvmx_dtx_osm_enax {
	u64 u64;
	struct cvmx_dtx_osm_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_osm_enax_s cn73xx;
	struct cvmx_dtx_osm_enax_s cn78xx;
	struct cvmx_dtx_osm_enax_s cn78xxp1;
};

typedef union cvmx_dtx_osm_enax cvmx_dtx_osm_enax_t;

/**
 * cvmx_dtx_osm_sel#
 */
union cvmx_dtx_osm_selx {
	u64 u64;
	struct cvmx_dtx_osm_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_osm_selx_s cn73xx;
	struct cvmx_dtx_osm_selx_s cn78xx;
	struct cvmx_dtx_osm_selx_s cn78xxp1;
};

typedef union cvmx_dtx_osm_selx cvmx_dtx_osm_selx_t;

/**
 * cvmx_dtx_pcs#_bcst_rsp
 */
union cvmx_dtx_pcsx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_pcsx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_pcsx_bcst_rsp_s cn70xx;
	struct cvmx_dtx_pcsx_bcst_rsp_s cn70xxp1;
};

typedef union cvmx_dtx_pcsx_bcst_rsp cvmx_dtx_pcsx_bcst_rsp_t;

/**
 * cvmx_dtx_pcs#_ctl
 */
union cvmx_dtx_pcsx_ctl {
	u64 u64;
	struct cvmx_dtx_pcsx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_pcsx_ctl_s cn70xx;
	struct cvmx_dtx_pcsx_ctl_s cn70xxp1;
};

typedef union cvmx_dtx_pcsx_ctl cvmx_dtx_pcsx_ctl_t;

/**
 * cvmx_dtx_pcs#_dat#
 */
union cvmx_dtx_pcsx_datx {
	u64 u64;
	struct cvmx_dtx_pcsx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_pcsx_datx_s cn70xx;
	struct cvmx_dtx_pcsx_datx_s cn70xxp1;
};

typedef union cvmx_dtx_pcsx_datx cvmx_dtx_pcsx_datx_t;

/**
 * cvmx_dtx_pcs#_ena#
 */
union cvmx_dtx_pcsx_enax {
	u64 u64;
	struct cvmx_dtx_pcsx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_pcsx_enax_s cn70xx;
	struct cvmx_dtx_pcsx_enax_s cn70xxp1;
};

typedef union cvmx_dtx_pcsx_enax cvmx_dtx_pcsx_enax_t;

/**
 * cvmx_dtx_pcs#_sel#
 */
union cvmx_dtx_pcsx_selx {
	u64 u64;
	struct cvmx_dtx_pcsx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_pcsx_selx_s cn70xx;
	struct cvmx_dtx_pcsx_selx_s cn70xxp1;
};

typedef union cvmx_dtx_pcsx_selx cvmx_dtx_pcsx_selx_t;

/**
 * cvmx_dtx_pem#_bcst_rsp
 */
union cvmx_dtx_pemx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_pemx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_pemx_bcst_rsp_s cn70xx;
	struct cvmx_dtx_pemx_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_pemx_bcst_rsp_s cn73xx;
	struct cvmx_dtx_pemx_bcst_rsp_s cn78xx;
	struct cvmx_dtx_pemx_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_pemx_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_pemx_bcst_rsp cvmx_dtx_pemx_bcst_rsp_t;

/**
 * cvmx_dtx_pem#_ctl
 */
union cvmx_dtx_pemx_ctl {
	u64 u64;
	struct cvmx_dtx_pemx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_pemx_ctl_s cn70xx;
	struct cvmx_dtx_pemx_ctl_s cn70xxp1;
	struct cvmx_dtx_pemx_ctl_s cn73xx;
	struct cvmx_dtx_pemx_ctl_s cn78xx;
	struct cvmx_dtx_pemx_ctl_s cn78xxp1;
	struct cvmx_dtx_pemx_ctl_s cnf75xx;
};

typedef union cvmx_dtx_pemx_ctl cvmx_dtx_pemx_ctl_t;

/**
 * cvmx_dtx_pem#_dat#
 */
union cvmx_dtx_pemx_datx {
	u64 u64;
	struct cvmx_dtx_pemx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_pemx_datx_s cn70xx;
	struct cvmx_dtx_pemx_datx_s cn70xxp1;
	struct cvmx_dtx_pemx_datx_s cn73xx;
	struct cvmx_dtx_pemx_datx_s cn78xx;
	struct cvmx_dtx_pemx_datx_s cn78xxp1;
	struct cvmx_dtx_pemx_datx_s cnf75xx;
};

typedef union cvmx_dtx_pemx_datx cvmx_dtx_pemx_datx_t;

/**
 * cvmx_dtx_pem#_ena#
 */
union cvmx_dtx_pemx_enax {
	u64 u64;
	struct cvmx_dtx_pemx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_pemx_enax_s cn70xx;
	struct cvmx_dtx_pemx_enax_s cn70xxp1;
	struct cvmx_dtx_pemx_enax_s cn73xx;
	struct cvmx_dtx_pemx_enax_s cn78xx;
	struct cvmx_dtx_pemx_enax_s cn78xxp1;
	struct cvmx_dtx_pemx_enax_s cnf75xx;
};

typedef union cvmx_dtx_pemx_enax cvmx_dtx_pemx_enax_t;

/**
 * cvmx_dtx_pem#_sel#
 */
union cvmx_dtx_pemx_selx {
	u64 u64;
	struct cvmx_dtx_pemx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_pemx_selx_s cn70xx;
	struct cvmx_dtx_pemx_selx_s cn70xxp1;
	struct cvmx_dtx_pemx_selx_s cn73xx;
	struct cvmx_dtx_pemx_selx_s cn78xx;
	struct cvmx_dtx_pemx_selx_s cn78xxp1;
	struct cvmx_dtx_pemx_selx_s cnf75xx;
};

typedef union cvmx_dtx_pemx_selx cvmx_dtx_pemx_selx_t;

/**
 * cvmx_dtx_pip_bcst_rsp
 */
union cvmx_dtx_pip_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_pip_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_pip_bcst_rsp_s cn70xx;
	struct cvmx_dtx_pip_bcst_rsp_s cn70xxp1;
};

typedef union cvmx_dtx_pip_bcst_rsp cvmx_dtx_pip_bcst_rsp_t;

/**
 * cvmx_dtx_pip_ctl
 */
union cvmx_dtx_pip_ctl {
	u64 u64;
	struct cvmx_dtx_pip_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_pip_ctl_s cn70xx;
	struct cvmx_dtx_pip_ctl_s cn70xxp1;
};

typedef union cvmx_dtx_pip_ctl cvmx_dtx_pip_ctl_t;

/**
 * cvmx_dtx_pip_dat#
 */
union cvmx_dtx_pip_datx {
	u64 u64;
	struct cvmx_dtx_pip_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_pip_datx_s cn70xx;
	struct cvmx_dtx_pip_datx_s cn70xxp1;
};

typedef union cvmx_dtx_pip_datx cvmx_dtx_pip_datx_t;

/**
 * cvmx_dtx_pip_ena#
 */
union cvmx_dtx_pip_enax {
	u64 u64;
	struct cvmx_dtx_pip_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_pip_enax_s cn70xx;
	struct cvmx_dtx_pip_enax_s cn70xxp1;
};

typedef union cvmx_dtx_pip_enax cvmx_dtx_pip_enax_t;

/**
 * cvmx_dtx_pip_sel#
 */
union cvmx_dtx_pip_selx {
	u64 u64;
	struct cvmx_dtx_pip_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_pip_selx_s cn70xx;
	struct cvmx_dtx_pip_selx_s cn70xxp1;
};

typedef union cvmx_dtx_pip_selx cvmx_dtx_pip_selx_t;

/**
 * cvmx_dtx_pki_pbe_bcst_rsp
 */
union cvmx_dtx_pki_pbe_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_pki_pbe_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_pki_pbe_bcst_rsp_s cn73xx;
	struct cvmx_dtx_pki_pbe_bcst_rsp_s cn78xx;
	struct cvmx_dtx_pki_pbe_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_pki_pbe_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_pki_pbe_bcst_rsp cvmx_dtx_pki_pbe_bcst_rsp_t;

/**
 * cvmx_dtx_pki_pbe_ctl
 */
union cvmx_dtx_pki_pbe_ctl {
	u64 u64;
	struct cvmx_dtx_pki_pbe_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_pki_pbe_ctl_s cn73xx;
	struct cvmx_dtx_pki_pbe_ctl_s cn78xx;
	struct cvmx_dtx_pki_pbe_ctl_s cn78xxp1;
	struct cvmx_dtx_pki_pbe_ctl_s cnf75xx;
};

typedef union cvmx_dtx_pki_pbe_ctl cvmx_dtx_pki_pbe_ctl_t;

/**
 * cvmx_dtx_pki_pbe_dat#
 */
union cvmx_dtx_pki_pbe_datx {
	u64 u64;
	struct cvmx_dtx_pki_pbe_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_pki_pbe_datx_s cn73xx;
	struct cvmx_dtx_pki_pbe_datx_s cn78xx;
	struct cvmx_dtx_pki_pbe_datx_s cn78xxp1;
	struct cvmx_dtx_pki_pbe_datx_s cnf75xx;
};

typedef union cvmx_dtx_pki_pbe_datx cvmx_dtx_pki_pbe_datx_t;

/**
 * cvmx_dtx_pki_pbe_ena#
 */
union cvmx_dtx_pki_pbe_enax {
	u64 u64;
	struct cvmx_dtx_pki_pbe_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_pki_pbe_enax_s cn73xx;
	struct cvmx_dtx_pki_pbe_enax_s cn78xx;
	struct cvmx_dtx_pki_pbe_enax_s cn78xxp1;
	struct cvmx_dtx_pki_pbe_enax_s cnf75xx;
};

typedef union cvmx_dtx_pki_pbe_enax cvmx_dtx_pki_pbe_enax_t;

/**
 * cvmx_dtx_pki_pbe_sel#
 */
union cvmx_dtx_pki_pbe_selx {
	u64 u64;
	struct cvmx_dtx_pki_pbe_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_pki_pbe_selx_s cn73xx;
	struct cvmx_dtx_pki_pbe_selx_s cn78xx;
	struct cvmx_dtx_pki_pbe_selx_s cn78xxp1;
	struct cvmx_dtx_pki_pbe_selx_s cnf75xx;
};

typedef union cvmx_dtx_pki_pbe_selx cvmx_dtx_pki_pbe_selx_t;

/**
 * cvmx_dtx_pki_pfe_bcst_rsp
 */
union cvmx_dtx_pki_pfe_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_pki_pfe_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_pki_pfe_bcst_rsp_s cn73xx;
	struct cvmx_dtx_pki_pfe_bcst_rsp_s cn78xx;
	struct cvmx_dtx_pki_pfe_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_pki_pfe_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_pki_pfe_bcst_rsp cvmx_dtx_pki_pfe_bcst_rsp_t;

/**
 * cvmx_dtx_pki_pfe_ctl
 */
union cvmx_dtx_pki_pfe_ctl {
	u64 u64;
	struct cvmx_dtx_pki_pfe_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_pki_pfe_ctl_s cn73xx;
	struct cvmx_dtx_pki_pfe_ctl_s cn78xx;
	struct cvmx_dtx_pki_pfe_ctl_s cn78xxp1;
	struct cvmx_dtx_pki_pfe_ctl_s cnf75xx;
};

typedef union cvmx_dtx_pki_pfe_ctl cvmx_dtx_pki_pfe_ctl_t;

/**
 * cvmx_dtx_pki_pfe_dat#
 */
union cvmx_dtx_pki_pfe_datx {
	u64 u64;
	struct cvmx_dtx_pki_pfe_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_pki_pfe_datx_s cn73xx;
	struct cvmx_dtx_pki_pfe_datx_s cn78xx;
	struct cvmx_dtx_pki_pfe_datx_s cn78xxp1;
	struct cvmx_dtx_pki_pfe_datx_s cnf75xx;
};

typedef union cvmx_dtx_pki_pfe_datx cvmx_dtx_pki_pfe_datx_t;

/**
 * cvmx_dtx_pki_pfe_ena#
 */
union cvmx_dtx_pki_pfe_enax {
	u64 u64;
	struct cvmx_dtx_pki_pfe_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_pki_pfe_enax_s cn73xx;
	struct cvmx_dtx_pki_pfe_enax_s cn78xx;
	struct cvmx_dtx_pki_pfe_enax_s cn78xxp1;
	struct cvmx_dtx_pki_pfe_enax_s cnf75xx;
};

typedef union cvmx_dtx_pki_pfe_enax cvmx_dtx_pki_pfe_enax_t;

/**
 * cvmx_dtx_pki_pfe_sel#
 */
union cvmx_dtx_pki_pfe_selx {
	u64 u64;
	struct cvmx_dtx_pki_pfe_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_pki_pfe_selx_s cn73xx;
	struct cvmx_dtx_pki_pfe_selx_s cn78xx;
	struct cvmx_dtx_pki_pfe_selx_s cn78xxp1;
	struct cvmx_dtx_pki_pfe_selx_s cnf75xx;
};

typedef union cvmx_dtx_pki_pfe_selx cvmx_dtx_pki_pfe_selx_t;

/**
 * cvmx_dtx_pki_pix_bcst_rsp
 */
union cvmx_dtx_pki_pix_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_pki_pix_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_pki_pix_bcst_rsp_s cn73xx;
	struct cvmx_dtx_pki_pix_bcst_rsp_s cn78xx;
	struct cvmx_dtx_pki_pix_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_pki_pix_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_pki_pix_bcst_rsp cvmx_dtx_pki_pix_bcst_rsp_t;

/**
 * cvmx_dtx_pki_pix_ctl
 */
union cvmx_dtx_pki_pix_ctl {
	u64 u64;
	struct cvmx_dtx_pki_pix_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_pki_pix_ctl_s cn73xx;
	struct cvmx_dtx_pki_pix_ctl_s cn78xx;
	struct cvmx_dtx_pki_pix_ctl_s cn78xxp1;
	struct cvmx_dtx_pki_pix_ctl_s cnf75xx;
};

typedef union cvmx_dtx_pki_pix_ctl cvmx_dtx_pki_pix_ctl_t;

/**
 * cvmx_dtx_pki_pix_dat#
 */
union cvmx_dtx_pki_pix_datx {
	u64 u64;
	struct cvmx_dtx_pki_pix_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_pki_pix_datx_s cn73xx;
	struct cvmx_dtx_pki_pix_datx_s cn78xx;
	struct cvmx_dtx_pki_pix_datx_s cn78xxp1;
	struct cvmx_dtx_pki_pix_datx_s cnf75xx;
};

typedef union cvmx_dtx_pki_pix_datx cvmx_dtx_pki_pix_datx_t;

/**
 * cvmx_dtx_pki_pix_ena#
 */
union cvmx_dtx_pki_pix_enax {
	u64 u64;
	struct cvmx_dtx_pki_pix_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_pki_pix_enax_s cn73xx;
	struct cvmx_dtx_pki_pix_enax_s cn78xx;
	struct cvmx_dtx_pki_pix_enax_s cn78xxp1;
	struct cvmx_dtx_pki_pix_enax_s cnf75xx;
};

typedef union cvmx_dtx_pki_pix_enax cvmx_dtx_pki_pix_enax_t;

/**
 * cvmx_dtx_pki_pix_sel#
 */
union cvmx_dtx_pki_pix_selx {
	u64 u64;
	struct cvmx_dtx_pki_pix_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_pki_pix_selx_s cn73xx;
	struct cvmx_dtx_pki_pix_selx_s cn78xx;
	struct cvmx_dtx_pki_pix_selx_s cn78xxp1;
	struct cvmx_dtx_pki_pix_selx_s cnf75xx;
};

typedef union cvmx_dtx_pki_pix_selx cvmx_dtx_pki_pix_selx_t;

/**
 * cvmx_dtx_pko_bcst_rsp
 */
union cvmx_dtx_pko_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_pko_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_pko_bcst_rsp_s cn70xx;
	struct cvmx_dtx_pko_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_pko_bcst_rsp_s cn73xx;
	struct cvmx_dtx_pko_bcst_rsp_s cn78xx;
	struct cvmx_dtx_pko_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_pko_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_pko_bcst_rsp cvmx_dtx_pko_bcst_rsp_t;

/**
 * cvmx_dtx_pko_ctl
 */
union cvmx_dtx_pko_ctl {
	u64 u64;
	struct cvmx_dtx_pko_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_pko_ctl_s cn70xx;
	struct cvmx_dtx_pko_ctl_s cn70xxp1;
	struct cvmx_dtx_pko_ctl_s cn73xx;
	struct cvmx_dtx_pko_ctl_s cn78xx;
	struct cvmx_dtx_pko_ctl_s cn78xxp1;
	struct cvmx_dtx_pko_ctl_s cnf75xx;
};

typedef union cvmx_dtx_pko_ctl cvmx_dtx_pko_ctl_t;

/**
 * cvmx_dtx_pko_dat#
 */
union cvmx_dtx_pko_datx {
	u64 u64;
	struct cvmx_dtx_pko_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_pko_datx_s cn70xx;
	struct cvmx_dtx_pko_datx_s cn70xxp1;
	struct cvmx_dtx_pko_datx_s cn73xx;
	struct cvmx_dtx_pko_datx_s cn78xx;
	struct cvmx_dtx_pko_datx_s cn78xxp1;
	struct cvmx_dtx_pko_datx_s cnf75xx;
};

typedef union cvmx_dtx_pko_datx cvmx_dtx_pko_datx_t;

/**
 * cvmx_dtx_pko_ena#
 */
union cvmx_dtx_pko_enax {
	u64 u64;
	struct cvmx_dtx_pko_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_pko_enax_s cn70xx;
	struct cvmx_dtx_pko_enax_s cn70xxp1;
	struct cvmx_dtx_pko_enax_s cn73xx;
	struct cvmx_dtx_pko_enax_s cn78xx;
	struct cvmx_dtx_pko_enax_s cn78xxp1;
	struct cvmx_dtx_pko_enax_s cnf75xx;
};

typedef union cvmx_dtx_pko_enax cvmx_dtx_pko_enax_t;

/**
 * cvmx_dtx_pko_sel#
 */
union cvmx_dtx_pko_selx {
	u64 u64;
	struct cvmx_dtx_pko_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_pko_selx_s cn70xx;
	struct cvmx_dtx_pko_selx_s cn70xxp1;
	struct cvmx_dtx_pko_selx_s cn73xx;
	struct cvmx_dtx_pko_selx_s cn78xx;
	struct cvmx_dtx_pko_selx_s cn78xxp1;
	struct cvmx_dtx_pko_selx_s cnf75xx;
};

typedef union cvmx_dtx_pko_selx cvmx_dtx_pko_selx_t;

/**
 * cvmx_dtx_pnb#_bcst_rsp
 */
union cvmx_dtx_pnbx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_pnbx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_pnbx_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_pnbx_bcst_rsp cvmx_dtx_pnbx_bcst_rsp_t;

/**
 * cvmx_dtx_pnb#_ctl
 */
union cvmx_dtx_pnbx_ctl {
	u64 u64;
	struct cvmx_dtx_pnbx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_pnbx_ctl_s cnf75xx;
};

typedef union cvmx_dtx_pnbx_ctl cvmx_dtx_pnbx_ctl_t;

/**
 * cvmx_dtx_pnb#_dat#
 */
union cvmx_dtx_pnbx_datx {
	u64 u64;
	struct cvmx_dtx_pnbx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_pnbx_datx_s cnf75xx;
};

typedef union cvmx_dtx_pnbx_datx cvmx_dtx_pnbx_datx_t;

/**
 * cvmx_dtx_pnb#_ena#
 */
union cvmx_dtx_pnbx_enax {
	u64 u64;
	struct cvmx_dtx_pnbx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_pnbx_enax_s cnf75xx;
};

typedef union cvmx_dtx_pnbx_enax cvmx_dtx_pnbx_enax_t;

/**
 * cvmx_dtx_pnb#_sel#
 */
union cvmx_dtx_pnbx_selx {
	u64 u64;
	struct cvmx_dtx_pnbx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_pnbx_selx_s cnf75xx;
};

typedef union cvmx_dtx_pnbx_selx cvmx_dtx_pnbx_selx_t;

/**
 * cvmx_dtx_pnbd#_bcst_rsp
 */
union cvmx_dtx_pnbdx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_pnbdx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_pnbdx_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_pnbdx_bcst_rsp cvmx_dtx_pnbdx_bcst_rsp_t;

/**
 * cvmx_dtx_pnbd#_ctl
 */
union cvmx_dtx_pnbdx_ctl {
	u64 u64;
	struct cvmx_dtx_pnbdx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_pnbdx_ctl_s cnf75xx;
};

typedef union cvmx_dtx_pnbdx_ctl cvmx_dtx_pnbdx_ctl_t;

/**
 * cvmx_dtx_pnbd#_dat#
 */
union cvmx_dtx_pnbdx_datx {
	u64 u64;
	struct cvmx_dtx_pnbdx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_pnbdx_datx_s cnf75xx;
};

typedef union cvmx_dtx_pnbdx_datx cvmx_dtx_pnbdx_datx_t;

/**
 * cvmx_dtx_pnbd#_ena#
 */
union cvmx_dtx_pnbdx_enax {
	u64 u64;
	struct cvmx_dtx_pnbdx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_pnbdx_enax_s cnf75xx;
};

typedef union cvmx_dtx_pnbdx_enax cvmx_dtx_pnbdx_enax_t;

/**
 * cvmx_dtx_pnbd#_sel#
 */
union cvmx_dtx_pnbdx_selx {
	u64 u64;
	struct cvmx_dtx_pnbdx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_pnbdx_selx_s cnf75xx;
};

typedef union cvmx_dtx_pnbdx_selx cvmx_dtx_pnbdx_selx_t;

/**
 * cvmx_dtx_pow_bcst_rsp
 */
union cvmx_dtx_pow_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_pow_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_pow_bcst_rsp_s cn70xx;
	struct cvmx_dtx_pow_bcst_rsp_s cn70xxp1;
};

typedef union cvmx_dtx_pow_bcst_rsp cvmx_dtx_pow_bcst_rsp_t;

/**
 * cvmx_dtx_pow_ctl
 */
union cvmx_dtx_pow_ctl {
	u64 u64;
	struct cvmx_dtx_pow_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_pow_ctl_s cn70xx;
	struct cvmx_dtx_pow_ctl_s cn70xxp1;
};

typedef union cvmx_dtx_pow_ctl cvmx_dtx_pow_ctl_t;

/**
 * cvmx_dtx_pow_dat#
 */
union cvmx_dtx_pow_datx {
	u64 u64;
	struct cvmx_dtx_pow_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_pow_datx_s cn70xx;
	struct cvmx_dtx_pow_datx_s cn70xxp1;
};

typedef union cvmx_dtx_pow_datx cvmx_dtx_pow_datx_t;

/**
 * cvmx_dtx_pow_ena#
 */
union cvmx_dtx_pow_enax {
	u64 u64;
	struct cvmx_dtx_pow_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_pow_enax_s cn70xx;
	struct cvmx_dtx_pow_enax_s cn70xxp1;
};

typedef union cvmx_dtx_pow_enax cvmx_dtx_pow_enax_t;

/**
 * cvmx_dtx_pow_sel#
 */
union cvmx_dtx_pow_selx {
	u64 u64;
	struct cvmx_dtx_pow_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_pow_selx_s cn70xx;
	struct cvmx_dtx_pow_selx_s cn70xxp1;
};

typedef union cvmx_dtx_pow_selx cvmx_dtx_pow_selx_t;

/**
 * cvmx_dtx_prch_bcst_rsp
 */
union cvmx_dtx_prch_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_prch_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_prch_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_prch_bcst_rsp cvmx_dtx_prch_bcst_rsp_t;

/**
 * cvmx_dtx_prch_ctl
 */
union cvmx_dtx_prch_ctl {
	u64 u64;
	struct cvmx_dtx_prch_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_prch_ctl_s cnf75xx;
};

typedef union cvmx_dtx_prch_ctl cvmx_dtx_prch_ctl_t;

/**
 * cvmx_dtx_prch_dat#
 */
union cvmx_dtx_prch_datx {
	u64 u64;
	struct cvmx_dtx_prch_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_prch_datx_s cnf75xx;
};

typedef union cvmx_dtx_prch_datx cvmx_dtx_prch_datx_t;

/**
 * cvmx_dtx_prch_ena#
 */
union cvmx_dtx_prch_enax {
	u64 u64;
	struct cvmx_dtx_prch_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_prch_enax_s cnf75xx;
};

typedef union cvmx_dtx_prch_enax cvmx_dtx_prch_enax_t;

/**
 * cvmx_dtx_prch_sel#
 */
union cvmx_dtx_prch_selx {
	u64 u64;
	struct cvmx_dtx_prch_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_prch_selx_s cnf75xx;
};

typedef union cvmx_dtx_prch_selx cvmx_dtx_prch_selx_t;

/**
 * cvmx_dtx_psm_bcst_rsp
 */
union cvmx_dtx_psm_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_psm_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_psm_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_psm_bcst_rsp cvmx_dtx_psm_bcst_rsp_t;

/**
 * cvmx_dtx_psm_ctl
 */
union cvmx_dtx_psm_ctl {
	u64 u64;
	struct cvmx_dtx_psm_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_psm_ctl_s cnf75xx;
};

typedef union cvmx_dtx_psm_ctl cvmx_dtx_psm_ctl_t;

/**
 * cvmx_dtx_psm_dat#
 */
union cvmx_dtx_psm_datx {
	u64 u64;
	struct cvmx_dtx_psm_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_psm_datx_s cnf75xx;
};

typedef union cvmx_dtx_psm_datx cvmx_dtx_psm_datx_t;

/**
 * cvmx_dtx_psm_ena#
 */
union cvmx_dtx_psm_enax {
	u64 u64;
	struct cvmx_dtx_psm_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_psm_enax_s cnf75xx;
};

typedef union cvmx_dtx_psm_enax cvmx_dtx_psm_enax_t;

/**
 * cvmx_dtx_psm_sel#
 */
union cvmx_dtx_psm_selx {
	u64 u64;
	struct cvmx_dtx_psm_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_psm_selx_s cnf75xx;
};

typedef union cvmx_dtx_psm_selx cvmx_dtx_psm_selx_t;

/**
 * cvmx_dtx_rad_bcst_rsp
 */
union cvmx_dtx_rad_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_rad_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_rad_bcst_rsp_s cn73xx;
	struct cvmx_dtx_rad_bcst_rsp_s cn78xx;
	struct cvmx_dtx_rad_bcst_rsp_s cn78xxp1;
};

typedef union cvmx_dtx_rad_bcst_rsp cvmx_dtx_rad_bcst_rsp_t;

/**
 * cvmx_dtx_rad_ctl
 */
union cvmx_dtx_rad_ctl {
	u64 u64;
	struct cvmx_dtx_rad_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_rad_ctl_s cn73xx;
	struct cvmx_dtx_rad_ctl_s cn78xx;
	struct cvmx_dtx_rad_ctl_s cn78xxp1;
};

typedef union cvmx_dtx_rad_ctl cvmx_dtx_rad_ctl_t;

/**
 * cvmx_dtx_rad_dat#
 */
union cvmx_dtx_rad_datx {
	u64 u64;
	struct cvmx_dtx_rad_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_rad_datx_s cn73xx;
	struct cvmx_dtx_rad_datx_s cn78xx;
	struct cvmx_dtx_rad_datx_s cn78xxp1;
};

typedef union cvmx_dtx_rad_datx cvmx_dtx_rad_datx_t;

/**
 * cvmx_dtx_rad_ena#
 */
union cvmx_dtx_rad_enax {
	u64 u64;
	struct cvmx_dtx_rad_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_rad_enax_s cn73xx;
	struct cvmx_dtx_rad_enax_s cn78xx;
	struct cvmx_dtx_rad_enax_s cn78xxp1;
};

typedef union cvmx_dtx_rad_enax cvmx_dtx_rad_enax_t;

/**
 * cvmx_dtx_rad_sel#
 */
union cvmx_dtx_rad_selx {
	u64 u64;
	struct cvmx_dtx_rad_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_rad_selx_s cn73xx;
	struct cvmx_dtx_rad_selx_s cn78xx;
	struct cvmx_dtx_rad_selx_s cn78xxp1;
};

typedef union cvmx_dtx_rad_selx cvmx_dtx_rad_selx_t;

/**
 * cvmx_dtx_rdec_bcst_rsp
 */
union cvmx_dtx_rdec_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_rdec_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_rdec_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_rdec_bcst_rsp cvmx_dtx_rdec_bcst_rsp_t;

/**
 * cvmx_dtx_rdec_ctl
 */
union cvmx_dtx_rdec_ctl {
	u64 u64;
	struct cvmx_dtx_rdec_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_rdec_ctl_s cnf75xx;
};

typedef union cvmx_dtx_rdec_ctl cvmx_dtx_rdec_ctl_t;

/**
 * cvmx_dtx_rdec_dat#
 */
union cvmx_dtx_rdec_datx {
	u64 u64;
	struct cvmx_dtx_rdec_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_rdec_datx_s cnf75xx;
};

typedef union cvmx_dtx_rdec_datx cvmx_dtx_rdec_datx_t;

/**
 * cvmx_dtx_rdec_ena#
 */
union cvmx_dtx_rdec_enax {
	u64 u64;
	struct cvmx_dtx_rdec_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_rdec_enax_s cnf75xx;
};

typedef union cvmx_dtx_rdec_enax cvmx_dtx_rdec_enax_t;

/**
 * cvmx_dtx_rdec_sel#
 */
union cvmx_dtx_rdec_selx {
	u64 u64;
	struct cvmx_dtx_rdec_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_rdec_selx_s cnf75xx;
};

typedef union cvmx_dtx_rdec_selx cvmx_dtx_rdec_selx_t;

/**
 * cvmx_dtx_rfif_bcst_rsp
 */
union cvmx_dtx_rfif_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_rfif_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_rfif_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_rfif_bcst_rsp cvmx_dtx_rfif_bcst_rsp_t;

/**
 * cvmx_dtx_rfif_ctl
 */
union cvmx_dtx_rfif_ctl {
	u64 u64;
	struct cvmx_dtx_rfif_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_rfif_ctl_s cnf75xx;
};

typedef union cvmx_dtx_rfif_ctl cvmx_dtx_rfif_ctl_t;

/**
 * cvmx_dtx_rfif_dat#
 */
union cvmx_dtx_rfif_datx {
	u64 u64;
	struct cvmx_dtx_rfif_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_rfif_datx_s cnf75xx;
};

typedef union cvmx_dtx_rfif_datx cvmx_dtx_rfif_datx_t;

/**
 * cvmx_dtx_rfif_ena#
 */
union cvmx_dtx_rfif_enax {
	u64 u64;
	struct cvmx_dtx_rfif_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_rfif_enax_s cnf75xx;
};

typedef union cvmx_dtx_rfif_enax cvmx_dtx_rfif_enax_t;

/**
 * cvmx_dtx_rfif_sel#
 */
union cvmx_dtx_rfif_selx {
	u64 u64;
	struct cvmx_dtx_rfif_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_rfif_selx_s cnf75xx;
};

typedef union cvmx_dtx_rfif_selx cvmx_dtx_rfif_selx_t;

/**
 * cvmx_dtx_rmap_bcst_rsp
 */
union cvmx_dtx_rmap_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_rmap_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_rmap_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_rmap_bcst_rsp cvmx_dtx_rmap_bcst_rsp_t;

/**
 * cvmx_dtx_rmap_ctl
 */
union cvmx_dtx_rmap_ctl {
	u64 u64;
	struct cvmx_dtx_rmap_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_rmap_ctl_s cnf75xx;
};

typedef union cvmx_dtx_rmap_ctl cvmx_dtx_rmap_ctl_t;

/**
 * cvmx_dtx_rmap_dat#
 */
union cvmx_dtx_rmap_datx {
	u64 u64;
	struct cvmx_dtx_rmap_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_rmap_datx_s cnf75xx;
};

typedef union cvmx_dtx_rmap_datx cvmx_dtx_rmap_datx_t;

/**
 * cvmx_dtx_rmap_ena#
 */
union cvmx_dtx_rmap_enax {
	u64 u64;
	struct cvmx_dtx_rmap_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_rmap_enax_s cnf75xx;
};

typedef union cvmx_dtx_rmap_enax cvmx_dtx_rmap_enax_t;

/**
 * cvmx_dtx_rmap_sel#
 */
union cvmx_dtx_rmap_selx {
	u64 u64;
	struct cvmx_dtx_rmap_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_rmap_selx_s cnf75xx;
};

typedef union cvmx_dtx_rmap_selx cvmx_dtx_rmap_selx_t;

/**
 * cvmx_dtx_rnm_bcst_rsp
 */
union cvmx_dtx_rnm_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_rnm_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_rnm_bcst_rsp_s cn73xx;
	struct cvmx_dtx_rnm_bcst_rsp_s cn78xx;
	struct cvmx_dtx_rnm_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_rnm_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_rnm_bcst_rsp cvmx_dtx_rnm_bcst_rsp_t;

/**
 * cvmx_dtx_rnm_ctl
 */
union cvmx_dtx_rnm_ctl {
	u64 u64;
	struct cvmx_dtx_rnm_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_rnm_ctl_s cn73xx;
	struct cvmx_dtx_rnm_ctl_s cn78xx;
	struct cvmx_dtx_rnm_ctl_s cn78xxp1;
	struct cvmx_dtx_rnm_ctl_s cnf75xx;
};

typedef union cvmx_dtx_rnm_ctl cvmx_dtx_rnm_ctl_t;

/**
 * cvmx_dtx_rnm_dat#
 */
union cvmx_dtx_rnm_datx {
	u64 u64;
	struct cvmx_dtx_rnm_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_rnm_datx_s cn73xx;
	struct cvmx_dtx_rnm_datx_s cn78xx;
	struct cvmx_dtx_rnm_datx_s cn78xxp1;
	struct cvmx_dtx_rnm_datx_s cnf75xx;
};

typedef union cvmx_dtx_rnm_datx cvmx_dtx_rnm_datx_t;

/**
 * cvmx_dtx_rnm_ena#
 */
union cvmx_dtx_rnm_enax {
	u64 u64;
	struct cvmx_dtx_rnm_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_rnm_enax_s cn73xx;
	struct cvmx_dtx_rnm_enax_s cn78xx;
	struct cvmx_dtx_rnm_enax_s cn78xxp1;
	struct cvmx_dtx_rnm_enax_s cnf75xx;
};

typedef union cvmx_dtx_rnm_enax cvmx_dtx_rnm_enax_t;

/**
 * cvmx_dtx_rnm_sel#
 */
union cvmx_dtx_rnm_selx {
	u64 u64;
	struct cvmx_dtx_rnm_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_rnm_selx_s cn73xx;
	struct cvmx_dtx_rnm_selx_s cn78xx;
	struct cvmx_dtx_rnm_selx_s cn78xxp1;
	struct cvmx_dtx_rnm_selx_s cnf75xx;
};

typedef union cvmx_dtx_rnm_selx cvmx_dtx_rnm_selx_t;

/**
 * cvmx_dtx_rst_bcst_rsp
 */
union cvmx_dtx_rst_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_rst_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_rst_bcst_rsp_s cn70xx;
	struct cvmx_dtx_rst_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_rst_bcst_rsp_s cn73xx;
	struct cvmx_dtx_rst_bcst_rsp_s cn78xx;
	struct cvmx_dtx_rst_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_rst_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_rst_bcst_rsp cvmx_dtx_rst_bcst_rsp_t;

/**
 * cvmx_dtx_rst_ctl
 */
union cvmx_dtx_rst_ctl {
	u64 u64;
	struct cvmx_dtx_rst_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_rst_ctl_s cn70xx;
	struct cvmx_dtx_rst_ctl_s cn70xxp1;
	struct cvmx_dtx_rst_ctl_s cn73xx;
	struct cvmx_dtx_rst_ctl_s cn78xx;
	struct cvmx_dtx_rst_ctl_s cn78xxp1;
	struct cvmx_dtx_rst_ctl_s cnf75xx;
};

typedef union cvmx_dtx_rst_ctl cvmx_dtx_rst_ctl_t;

/**
 * cvmx_dtx_rst_dat#
 */
union cvmx_dtx_rst_datx {
	u64 u64;
	struct cvmx_dtx_rst_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_rst_datx_s cn70xx;
	struct cvmx_dtx_rst_datx_s cn70xxp1;
	struct cvmx_dtx_rst_datx_s cn73xx;
	struct cvmx_dtx_rst_datx_s cn78xx;
	struct cvmx_dtx_rst_datx_s cn78xxp1;
	struct cvmx_dtx_rst_datx_s cnf75xx;
};

typedef union cvmx_dtx_rst_datx cvmx_dtx_rst_datx_t;

/**
 * cvmx_dtx_rst_ena#
 */
union cvmx_dtx_rst_enax {
	u64 u64;
	struct cvmx_dtx_rst_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_rst_enax_s cn70xx;
	struct cvmx_dtx_rst_enax_s cn70xxp1;
	struct cvmx_dtx_rst_enax_s cn73xx;
	struct cvmx_dtx_rst_enax_s cn78xx;
	struct cvmx_dtx_rst_enax_s cn78xxp1;
	struct cvmx_dtx_rst_enax_s cnf75xx;
};

typedef union cvmx_dtx_rst_enax cvmx_dtx_rst_enax_t;

/**
 * cvmx_dtx_rst_sel#
 */
union cvmx_dtx_rst_selx {
	u64 u64;
	struct cvmx_dtx_rst_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_rst_selx_s cn70xx;
	struct cvmx_dtx_rst_selx_s cn70xxp1;
	struct cvmx_dtx_rst_selx_s cn73xx;
	struct cvmx_dtx_rst_selx_s cn78xx;
	struct cvmx_dtx_rst_selx_s cn78xxp1;
	struct cvmx_dtx_rst_selx_s cnf75xx;
};

typedef union cvmx_dtx_rst_selx cvmx_dtx_rst_selx_t;

/**
 * cvmx_dtx_sata_bcst_rsp
 */
union cvmx_dtx_sata_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_sata_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_sata_bcst_rsp_s cn70xx;
	struct cvmx_dtx_sata_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_sata_bcst_rsp_s cn73xx;
};

typedef union cvmx_dtx_sata_bcst_rsp cvmx_dtx_sata_bcst_rsp_t;

/**
 * cvmx_dtx_sata_ctl
 */
union cvmx_dtx_sata_ctl {
	u64 u64;
	struct cvmx_dtx_sata_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_sata_ctl_s cn70xx;
	struct cvmx_dtx_sata_ctl_s cn70xxp1;
	struct cvmx_dtx_sata_ctl_s cn73xx;
};

typedef union cvmx_dtx_sata_ctl cvmx_dtx_sata_ctl_t;

/**
 * cvmx_dtx_sata_dat#
 */
union cvmx_dtx_sata_datx {
	u64 u64;
	struct cvmx_dtx_sata_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_sata_datx_s cn70xx;
	struct cvmx_dtx_sata_datx_s cn70xxp1;
	struct cvmx_dtx_sata_datx_s cn73xx;
};

typedef union cvmx_dtx_sata_datx cvmx_dtx_sata_datx_t;

/**
 * cvmx_dtx_sata_ena#
 */
union cvmx_dtx_sata_enax {
	u64 u64;
	struct cvmx_dtx_sata_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_sata_enax_s cn70xx;
	struct cvmx_dtx_sata_enax_s cn70xxp1;
	struct cvmx_dtx_sata_enax_s cn73xx;
};

typedef union cvmx_dtx_sata_enax cvmx_dtx_sata_enax_t;

/**
 * cvmx_dtx_sata_sel#
 */
union cvmx_dtx_sata_selx {
	u64 u64;
	struct cvmx_dtx_sata_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_sata_selx_s cn70xx;
	struct cvmx_dtx_sata_selx_s cn70xxp1;
	struct cvmx_dtx_sata_selx_s cn73xx;
};

typedef union cvmx_dtx_sata_selx cvmx_dtx_sata_selx_t;

/**
 * cvmx_dtx_sli_bcst_rsp
 */
union cvmx_dtx_sli_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_sli_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_sli_bcst_rsp_s cn70xx;
	struct cvmx_dtx_sli_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_sli_bcst_rsp_s cn73xx;
	struct cvmx_dtx_sli_bcst_rsp_s cn78xx;
	struct cvmx_dtx_sli_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_sli_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_sli_bcst_rsp cvmx_dtx_sli_bcst_rsp_t;

/**
 * cvmx_dtx_sli_ctl
 */
union cvmx_dtx_sli_ctl {
	u64 u64;
	struct cvmx_dtx_sli_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_sli_ctl_s cn70xx;
	struct cvmx_dtx_sli_ctl_s cn70xxp1;
	struct cvmx_dtx_sli_ctl_s cn73xx;
	struct cvmx_dtx_sli_ctl_s cn78xx;
	struct cvmx_dtx_sli_ctl_s cn78xxp1;
	struct cvmx_dtx_sli_ctl_s cnf75xx;
};

typedef union cvmx_dtx_sli_ctl cvmx_dtx_sli_ctl_t;

/**
 * cvmx_dtx_sli_dat#
 */
union cvmx_dtx_sli_datx {
	u64 u64;
	struct cvmx_dtx_sli_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_sli_datx_s cn70xx;
	struct cvmx_dtx_sli_datx_s cn70xxp1;
	struct cvmx_dtx_sli_datx_s cn73xx;
	struct cvmx_dtx_sli_datx_s cn78xx;
	struct cvmx_dtx_sli_datx_s cn78xxp1;
	struct cvmx_dtx_sli_datx_s cnf75xx;
};

typedef union cvmx_dtx_sli_datx cvmx_dtx_sli_datx_t;

/**
 * cvmx_dtx_sli_ena#
 */
union cvmx_dtx_sli_enax {
	u64 u64;
	struct cvmx_dtx_sli_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_sli_enax_s cn70xx;
	struct cvmx_dtx_sli_enax_s cn70xxp1;
	struct cvmx_dtx_sli_enax_s cn73xx;
	struct cvmx_dtx_sli_enax_s cn78xx;
	struct cvmx_dtx_sli_enax_s cn78xxp1;
	struct cvmx_dtx_sli_enax_s cnf75xx;
};

typedef union cvmx_dtx_sli_enax cvmx_dtx_sli_enax_t;

/**
 * cvmx_dtx_sli_sel#
 */
union cvmx_dtx_sli_selx {
	u64 u64;
	struct cvmx_dtx_sli_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_sli_selx_s cn70xx;
	struct cvmx_dtx_sli_selx_s cn70xxp1;
	struct cvmx_dtx_sli_selx_s cn73xx;
	struct cvmx_dtx_sli_selx_s cn78xx;
	struct cvmx_dtx_sli_selx_s cn78xxp1;
	struct cvmx_dtx_sli_selx_s cnf75xx;
};

typedef union cvmx_dtx_sli_selx cvmx_dtx_sli_selx_t;

/**
 * cvmx_dtx_spem_bcst_rsp
 */
union cvmx_dtx_spem_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_spem_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_spem_bcst_rsp_s cn73xx;
};

typedef union cvmx_dtx_spem_bcst_rsp cvmx_dtx_spem_bcst_rsp_t;

/**
 * cvmx_dtx_spem_ctl
 */
union cvmx_dtx_spem_ctl {
	u64 u64;
	struct cvmx_dtx_spem_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_spem_ctl_s cn73xx;
};

typedef union cvmx_dtx_spem_ctl cvmx_dtx_spem_ctl_t;

/**
 * cvmx_dtx_spem_dat#
 */
union cvmx_dtx_spem_datx {
	u64 u64;
	struct cvmx_dtx_spem_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_spem_datx_s cn73xx;
};

typedef union cvmx_dtx_spem_datx cvmx_dtx_spem_datx_t;

/**
 * cvmx_dtx_spem_ena#
 */
union cvmx_dtx_spem_enax {
	u64 u64;
	struct cvmx_dtx_spem_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_spem_enax_s cn73xx;
};

typedef union cvmx_dtx_spem_enax cvmx_dtx_spem_enax_t;

/**
 * cvmx_dtx_spem_sel#
 */
union cvmx_dtx_spem_selx {
	u64 u64;
	struct cvmx_dtx_spem_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_spem_selx_s cn73xx;
};

typedef union cvmx_dtx_spem_selx cvmx_dtx_spem_selx_t;

/**
 * cvmx_dtx_srio#_bcst_rsp
 */
union cvmx_dtx_sriox_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_sriox_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_sriox_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_sriox_bcst_rsp cvmx_dtx_sriox_bcst_rsp_t;

/**
 * cvmx_dtx_srio#_ctl
 */
union cvmx_dtx_sriox_ctl {
	u64 u64;
	struct cvmx_dtx_sriox_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_sriox_ctl_s cnf75xx;
};

typedef union cvmx_dtx_sriox_ctl cvmx_dtx_sriox_ctl_t;

/**
 * cvmx_dtx_srio#_dat#
 */
union cvmx_dtx_sriox_datx {
	u64 u64;
	struct cvmx_dtx_sriox_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_sriox_datx_s cnf75xx;
};

typedef union cvmx_dtx_sriox_datx cvmx_dtx_sriox_datx_t;

/**
 * cvmx_dtx_srio#_ena#
 */
union cvmx_dtx_sriox_enax {
	u64 u64;
	struct cvmx_dtx_sriox_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_sriox_enax_s cnf75xx;
};

typedef union cvmx_dtx_sriox_enax cvmx_dtx_sriox_enax_t;

/**
 * cvmx_dtx_srio#_sel#
 */
union cvmx_dtx_sriox_selx {
	u64 u64;
	struct cvmx_dtx_sriox_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_sriox_selx_s cnf75xx;
};

typedef union cvmx_dtx_sriox_selx cvmx_dtx_sriox_selx_t;

/**
 * cvmx_dtx_sso_bcst_rsp
 */
union cvmx_dtx_sso_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_sso_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_sso_bcst_rsp_s cn73xx;
	struct cvmx_dtx_sso_bcst_rsp_s cn78xx;
	struct cvmx_dtx_sso_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_sso_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_sso_bcst_rsp cvmx_dtx_sso_bcst_rsp_t;

/**
 * cvmx_dtx_sso_ctl
 */
union cvmx_dtx_sso_ctl {
	u64 u64;
	struct cvmx_dtx_sso_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_sso_ctl_s cn73xx;
	struct cvmx_dtx_sso_ctl_s cn78xx;
	struct cvmx_dtx_sso_ctl_s cn78xxp1;
	struct cvmx_dtx_sso_ctl_s cnf75xx;
};

typedef union cvmx_dtx_sso_ctl cvmx_dtx_sso_ctl_t;

/**
 * cvmx_dtx_sso_dat#
 */
union cvmx_dtx_sso_datx {
	u64 u64;
	struct cvmx_dtx_sso_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_sso_datx_s cn73xx;
	struct cvmx_dtx_sso_datx_s cn78xx;
	struct cvmx_dtx_sso_datx_s cn78xxp1;
	struct cvmx_dtx_sso_datx_s cnf75xx;
};

typedef union cvmx_dtx_sso_datx cvmx_dtx_sso_datx_t;

/**
 * cvmx_dtx_sso_ena#
 */
union cvmx_dtx_sso_enax {
	u64 u64;
	struct cvmx_dtx_sso_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_sso_enax_s cn73xx;
	struct cvmx_dtx_sso_enax_s cn78xx;
	struct cvmx_dtx_sso_enax_s cn78xxp1;
	struct cvmx_dtx_sso_enax_s cnf75xx;
};

typedef union cvmx_dtx_sso_enax cvmx_dtx_sso_enax_t;

/**
 * cvmx_dtx_sso_sel#
 */
union cvmx_dtx_sso_selx {
	u64 u64;
	struct cvmx_dtx_sso_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_sso_selx_s cn73xx;
	struct cvmx_dtx_sso_selx_s cn78xx;
	struct cvmx_dtx_sso_selx_s cn78xxp1;
	struct cvmx_dtx_sso_selx_s cnf75xx;
};

typedef union cvmx_dtx_sso_selx cvmx_dtx_sso_selx_t;

/**
 * cvmx_dtx_tdec_bcst_rsp
 */
union cvmx_dtx_tdec_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_tdec_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_tdec_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_tdec_bcst_rsp cvmx_dtx_tdec_bcst_rsp_t;

/**
 * cvmx_dtx_tdec_ctl
 */
union cvmx_dtx_tdec_ctl {
	u64 u64;
	struct cvmx_dtx_tdec_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_tdec_ctl_s cnf75xx;
};

typedef union cvmx_dtx_tdec_ctl cvmx_dtx_tdec_ctl_t;

/**
 * cvmx_dtx_tdec_dat#
 */
union cvmx_dtx_tdec_datx {
	u64 u64;
	struct cvmx_dtx_tdec_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_tdec_datx_s cnf75xx;
};

typedef union cvmx_dtx_tdec_datx cvmx_dtx_tdec_datx_t;

/**
 * cvmx_dtx_tdec_ena#
 */
union cvmx_dtx_tdec_enax {
	u64 u64;
	struct cvmx_dtx_tdec_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_tdec_enax_s cnf75xx;
};

typedef union cvmx_dtx_tdec_enax cvmx_dtx_tdec_enax_t;

/**
 * cvmx_dtx_tdec_sel#
 */
union cvmx_dtx_tdec_selx {
	u64 u64;
	struct cvmx_dtx_tdec_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_tdec_selx_s cnf75xx;
};

typedef union cvmx_dtx_tdec_selx cvmx_dtx_tdec_selx_t;

/**
 * cvmx_dtx_tim_bcst_rsp
 */
union cvmx_dtx_tim_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_tim_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_tim_bcst_rsp_s cn70xx;
	struct cvmx_dtx_tim_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_tim_bcst_rsp_s cn73xx;
	struct cvmx_dtx_tim_bcst_rsp_s cn78xx;
	struct cvmx_dtx_tim_bcst_rsp_s cn78xxp1;
	struct cvmx_dtx_tim_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_tim_bcst_rsp cvmx_dtx_tim_bcst_rsp_t;

/**
 * cvmx_dtx_tim_ctl
 */
union cvmx_dtx_tim_ctl {
	u64 u64;
	struct cvmx_dtx_tim_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_tim_ctl_s cn70xx;
	struct cvmx_dtx_tim_ctl_s cn70xxp1;
	struct cvmx_dtx_tim_ctl_s cn73xx;
	struct cvmx_dtx_tim_ctl_s cn78xx;
	struct cvmx_dtx_tim_ctl_s cn78xxp1;
	struct cvmx_dtx_tim_ctl_s cnf75xx;
};

typedef union cvmx_dtx_tim_ctl cvmx_dtx_tim_ctl_t;

/**
 * cvmx_dtx_tim_dat#
 */
union cvmx_dtx_tim_datx {
	u64 u64;
	struct cvmx_dtx_tim_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_tim_datx_s cn70xx;
	struct cvmx_dtx_tim_datx_s cn70xxp1;
	struct cvmx_dtx_tim_datx_s cn73xx;
	struct cvmx_dtx_tim_datx_s cn78xx;
	struct cvmx_dtx_tim_datx_s cn78xxp1;
	struct cvmx_dtx_tim_datx_s cnf75xx;
};

typedef union cvmx_dtx_tim_datx cvmx_dtx_tim_datx_t;

/**
 * cvmx_dtx_tim_ena#
 */
union cvmx_dtx_tim_enax {
	u64 u64;
	struct cvmx_dtx_tim_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_tim_enax_s cn70xx;
	struct cvmx_dtx_tim_enax_s cn70xxp1;
	struct cvmx_dtx_tim_enax_s cn73xx;
	struct cvmx_dtx_tim_enax_s cn78xx;
	struct cvmx_dtx_tim_enax_s cn78xxp1;
	struct cvmx_dtx_tim_enax_s cnf75xx;
};

typedef union cvmx_dtx_tim_enax cvmx_dtx_tim_enax_t;

/**
 * cvmx_dtx_tim_sel#
 */
union cvmx_dtx_tim_selx {
	u64 u64;
	struct cvmx_dtx_tim_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_tim_selx_s cn70xx;
	struct cvmx_dtx_tim_selx_s cn70xxp1;
	struct cvmx_dtx_tim_selx_s cn73xx;
	struct cvmx_dtx_tim_selx_s cn78xx;
	struct cvmx_dtx_tim_selx_s cn78xxp1;
	struct cvmx_dtx_tim_selx_s cnf75xx;
};

typedef union cvmx_dtx_tim_selx cvmx_dtx_tim_selx_t;

/**
 * cvmx_dtx_ulfe_bcst_rsp
 */
union cvmx_dtx_ulfe_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_ulfe_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_ulfe_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_ulfe_bcst_rsp cvmx_dtx_ulfe_bcst_rsp_t;

/**
 * cvmx_dtx_ulfe_ctl
 */
union cvmx_dtx_ulfe_ctl {
	u64 u64;
	struct cvmx_dtx_ulfe_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_ulfe_ctl_s cnf75xx;
};

typedef union cvmx_dtx_ulfe_ctl cvmx_dtx_ulfe_ctl_t;

/**
 * cvmx_dtx_ulfe_dat#
 */
union cvmx_dtx_ulfe_datx {
	u64 u64;
	struct cvmx_dtx_ulfe_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_ulfe_datx_s cnf75xx;
};

typedef union cvmx_dtx_ulfe_datx cvmx_dtx_ulfe_datx_t;

/**
 * cvmx_dtx_ulfe_ena#
 */
union cvmx_dtx_ulfe_enax {
	u64 u64;
	struct cvmx_dtx_ulfe_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_ulfe_enax_s cnf75xx;
};

typedef union cvmx_dtx_ulfe_enax cvmx_dtx_ulfe_enax_t;

/**
 * cvmx_dtx_ulfe_sel#
 */
union cvmx_dtx_ulfe_selx {
	u64 u64;
	struct cvmx_dtx_ulfe_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_ulfe_selx_s cnf75xx;
};

typedef union cvmx_dtx_ulfe_selx cvmx_dtx_ulfe_selx_t;

/**
 * cvmx_dtx_usbdrd#_bcst_rsp
 */
union cvmx_dtx_usbdrdx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_usbdrdx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_usbdrdx_bcst_rsp_s cn70xx;
	struct cvmx_dtx_usbdrdx_bcst_rsp_s cn70xxp1;
	struct cvmx_dtx_usbdrdx_bcst_rsp_s cn73xx;
	struct cvmx_dtx_usbdrdx_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_usbdrdx_bcst_rsp cvmx_dtx_usbdrdx_bcst_rsp_t;

/**
 * cvmx_dtx_usbdrd#_ctl
 */
union cvmx_dtx_usbdrdx_ctl {
	u64 u64;
	struct cvmx_dtx_usbdrdx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_usbdrdx_ctl_s cn70xx;
	struct cvmx_dtx_usbdrdx_ctl_s cn70xxp1;
	struct cvmx_dtx_usbdrdx_ctl_s cn73xx;
	struct cvmx_dtx_usbdrdx_ctl_s cnf75xx;
};

typedef union cvmx_dtx_usbdrdx_ctl cvmx_dtx_usbdrdx_ctl_t;

/**
 * cvmx_dtx_usbdrd#_dat#
 */
union cvmx_dtx_usbdrdx_datx {
	u64 u64;
	struct cvmx_dtx_usbdrdx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_usbdrdx_datx_s cn70xx;
	struct cvmx_dtx_usbdrdx_datx_s cn70xxp1;
	struct cvmx_dtx_usbdrdx_datx_s cn73xx;
	struct cvmx_dtx_usbdrdx_datx_s cnf75xx;
};

typedef union cvmx_dtx_usbdrdx_datx cvmx_dtx_usbdrdx_datx_t;

/**
 * cvmx_dtx_usbdrd#_ena#
 */
union cvmx_dtx_usbdrdx_enax {
	u64 u64;
	struct cvmx_dtx_usbdrdx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_usbdrdx_enax_s cn70xx;
	struct cvmx_dtx_usbdrdx_enax_s cn70xxp1;
	struct cvmx_dtx_usbdrdx_enax_s cn73xx;
	struct cvmx_dtx_usbdrdx_enax_s cnf75xx;
};

typedef union cvmx_dtx_usbdrdx_enax cvmx_dtx_usbdrdx_enax_t;

/**
 * cvmx_dtx_usbdrd#_sel#
 */
union cvmx_dtx_usbdrdx_selx {
	u64 u64;
	struct cvmx_dtx_usbdrdx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_usbdrdx_selx_s cn70xx;
	struct cvmx_dtx_usbdrdx_selx_s cn70xxp1;
	struct cvmx_dtx_usbdrdx_selx_s cn73xx;
	struct cvmx_dtx_usbdrdx_selx_s cnf75xx;
};

typedef union cvmx_dtx_usbdrdx_selx cvmx_dtx_usbdrdx_selx_t;

/**
 * cvmx_dtx_usbh#_bcst_rsp
 */
union cvmx_dtx_usbhx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_usbhx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_usbhx_bcst_rsp_s cn78xx;
	struct cvmx_dtx_usbhx_bcst_rsp_s cn78xxp1;
};

typedef union cvmx_dtx_usbhx_bcst_rsp cvmx_dtx_usbhx_bcst_rsp_t;

/**
 * cvmx_dtx_usbh#_ctl
 */
union cvmx_dtx_usbhx_ctl {
	u64 u64;
	struct cvmx_dtx_usbhx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_usbhx_ctl_s cn78xx;
	struct cvmx_dtx_usbhx_ctl_s cn78xxp1;
};

typedef union cvmx_dtx_usbhx_ctl cvmx_dtx_usbhx_ctl_t;

/**
 * cvmx_dtx_usbh#_dat#
 */
union cvmx_dtx_usbhx_datx {
	u64 u64;
	struct cvmx_dtx_usbhx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_usbhx_datx_s cn78xx;
	struct cvmx_dtx_usbhx_datx_s cn78xxp1;
};

typedef union cvmx_dtx_usbhx_datx cvmx_dtx_usbhx_datx_t;

/**
 * cvmx_dtx_usbh#_ena#
 */
union cvmx_dtx_usbhx_enax {
	u64 u64;
	struct cvmx_dtx_usbhx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_usbhx_enax_s cn78xx;
	struct cvmx_dtx_usbhx_enax_s cn78xxp1;
};

typedef union cvmx_dtx_usbhx_enax cvmx_dtx_usbhx_enax_t;

/**
 * cvmx_dtx_usbh#_sel#
 */
union cvmx_dtx_usbhx_selx {
	u64 u64;
	struct cvmx_dtx_usbhx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_usbhx_selx_s cn78xx;
	struct cvmx_dtx_usbhx_selx_s cn78xxp1;
};

typedef union cvmx_dtx_usbhx_selx cvmx_dtx_usbhx_selx_t;

/**
 * cvmx_dtx_vdec_bcst_rsp
 */
union cvmx_dtx_vdec_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_vdec_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_vdec_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_vdec_bcst_rsp cvmx_dtx_vdec_bcst_rsp_t;

/**
 * cvmx_dtx_vdec_ctl
 */
union cvmx_dtx_vdec_ctl {
	u64 u64;
	struct cvmx_dtx_vdec_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_vdec_ctl_s cnf75xx;
};

typedef union cvmx_dtx_vdec_ctl cvmx_dtx_vdec_ctl_t;

/**
 * cvmx_dtx_vdec_dat#
 */
union cvmx_dtx_vdec_datx {
	u64 u64;
	struct cvmx_dtx_vdec_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_vdec_datx_s cnf75xx;
};

typedef union cvmx_dtx_vdec_datx cvmx_dtx_vdec_datx_t;

/**
 * cvmx_dtx_vdec_ena#
 */
union cvmx_dtx_vdec_enax {
	u64 u64;
	struct cvmx_dtx_vdec_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_vdec_enax_s cnf75xx;
};

typedef union cvmx_dtx_vdec_enax cvmx_dtx_vdec_enax_t;

/**
 * cvmx_dtx_vdec_sel#
 */
union cvmx_dtx_vdec_selx {
	u64 u64;
	struct cvmx_dtx_vdec_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_vdec_selx_s cnf75xx;
};

typedef union cvmx_dtx_vdec_selx cvmx_dtx_vdec_selx_t;

/**
 * cvmx_dtx_wpse_bcst_rsp
 */
union cvmx_dtx_wpse_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_wpse_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_wpse_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_wpse_bcst_rsp cvmx_dtx_wpse_bcst_rsp_t;

/**
 * cvmx_dtx_wpse_ctl
 */
union cvmx_dtx_wpse_ctl {
	u64 u64;
	struct cvmx_dtx_wpse_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_wpse_ctl_s cnf75xx;
};

typedef union cvmx_dtx_wpse_ctl cvmx_dtx_wpse_ctl_t;

/**
 * cvmx_dtx_wpse_dat#
 */
union cvmx_dtx_wpse_datx {
	u64 u64;
	struct cvmx_dtx_wpse_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_wpse_datx_s cnf75xx;
};

typedef union cvmx_dtx_wpse_datx cvmx_dtx_wpse_datx_t;

/**
 * cvmx_dtx_wpse_ena#
 */
union cvmx_dtx_wpse_enax {
	u64 u64;
	struct cvmx_dtx_wpse_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_wpse_enax_s cnf75xx;
};

typedef union cvmx_dtx_wpse_enax cvmx_dtx_wpse_enax_t;

/**
 * cvmx_dtx_wpse_sel#
 */
union cvmx_dtx_wpse_selx {
	u64 u64;
	struct cvmx_dtx_wpse_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_wpse_selx_s cnf75xx;
};

typedef union cvmx_dtx_wpse_selx cvmx_dtx_wpse_selx_t;

/**
 * cvmx_dtx_wrce_bcst_rsp
 */
union cvmx_dtx_wrce_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_wrce_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_wrce_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_wrce_bcst_rsp cvmx_dtx_wrce_bcst_rsp_t;

/**
 * cvmx_dtx_wrce_ctl
 */
union cvmx_dtx_wrce_ctl {
	u64 u64;
	struct cvmx_dtx_wrce_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_wrce_ctl_s cnf75xx;
};

typedef union cvmx_dtx_wrce_ctl cvmx_dtx_wrce_ctl_t;

/**
 * cvmx_dtx_wrce_dat#
 */
union cvmx_dtx_wrce_datx {
	u64 u64;
	struct cvmx_dtx_wrce_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_wrce_datx_s cnf75xx;
};

typedef union cvmx_dtx_wrce_datx cvmx_dtx_wrce_datx_t;

/**
 * cvmx_dtx_wrce_ena#
 */
union cvmx_dtx_wrce_enax {
	u64 u64;
	struct cvmx_dtx_wrce_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_wrce_enax_s cnf75xx;
};

typedef union cvmx_dtx_wrce_enax cvmx_dtx_wrce_enax_t;

/**
 * cvmx_dtx_wrce_sel#
 */
union cvmx_dtx_wrce_selx {
	u64 u64;
	struct cvmx_dtx_wrce_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_wrce_selx_s cnf75xx;
};

typedef union cvmx_dtx_wrce_selx cvmx_dtx_wrce_selx_t;

/**
 * cvmx_dtx_wrde_bcst_rsp
 */
union cvmx_dtx_wrde_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_wrde_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_wrde_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_wrde_bcst_rsp cvmx_dtx_wrde_bcst_rsp_t;

/**
 * cvmx_dtx_wrde_ctl
 */
union cvmx_dtx_wrde_ctl {
	u64 u64;
	struct cvmx_dtx_wrde_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_wrde_ctl_s cnf75xx;
};

typedef union cvmx_dtx_wrde_ctl cvmx_dtx_wrde_ctl_t;

/**
 * cvmx_dtx_wrde_dat#
 */
union cvmx_dtx_wrde_datx {
	u64 u64;
	struct cvmx_dtx_wrde_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_wrde_datx_s cnf75xx;
};

typedef union cvmx_dtx_wrde_datx cvmx_dtx_wrde_datx_t;

/**
 * cvmx_dtx_wrde_ena#
 */
union cvmx_dtx_wrde_enax {
	u64 u64;
	struct cvmx_dtx_wrde_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_wrde_enax_s cnf75xx;
};

typedef union cvmx_dtx_wrde_enax cvmx_dtx_wrde_enax_t;

/**
 * cvmx_dtx_wrde_sel#
 */
union cvmx_dtx_wrde_selx {
	u64 u64;
	struct cvmx_dtx_wrde_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_wrde_selx_s cnf75xx;
};

typedef union cvmx_dtx_wrde_selx cvmx_dtx_wrde_selx_t;

/**
 * cvmx_dtx_wrse_bcst_rsp
 */
union cvmx_dtx_wrse_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_wrse_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_wrse_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_wrse_bcst_rsp cvmx_dtx_wrse_bcst_rsp_t;

/**
 * cvmx_dtx_wrse_ctl
 */
union cvmx_dtx_wrse_ctl {
	u64 u64;
	struct cvmx_dtx_wrse_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_wrse_ctl_s cnf75xx;
};

typedef union cvmx_dtx_wrse_ctl cvmx_dtx_wrse_ctl_t;

/**
 * cvmx_dtx_wrse_dat#
 */
union cvmx_dtx_wrse_datx {
	u64 u64;
	struct cvmx_dtx_wrse_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_wrse_datx_s cnf75xx;
};

typedef union cvmx_dtx_wrse_datx cvmx_dtx_wrse_datx_t;

/**
 * cvmx_dtx_wrse_ena#
 */
union cvmx_dtx_wrse_enax {
	u64 u64;
	struct cvmx_dtx_wrse_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_wrse_enax_s cnf75xx;
};

typedef union cvmx_dtx_wrse_enax cvmx_dtx_wrse_enax_t;

/**
 * cvmx_dtx_wrse_sel#
 */
union cvmx_dtx_wrse_selx {
	u64 u64;
	struct cvmx_dtx_wrse_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_wrse_selx_s cnf75xx;
};

typedef union cvmx_dtx_wrse_selx cvmx_dtx_wrse_selx_t;

/**
 * cvmx_dtx_wtxe_bcst_rsp
 */
union cvmx_dtx_wtxe_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_wtxe_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_wtxe_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_wtxe_bcst_rsp cvmx_dtx_wtxe_bcst_rsp_t;

/**
 * cvmx_dtx_wtxe_ctl
 */
union cvmx_dtx_wtxe_ctl {
	u64 u64;
	struct cvmx_dtx_wtxe_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_wtxe_ctl_s cnf75xx;
};

typedef union cvmx_dtx_wtxe_ctl cvmx_dtx_wtxe_ctl_t;

/**
 * cvmx_dtx_wtxe_dat#
 */
union cvmx_dtx_wtxe_datx {
	u64 u64;
	struct cvmx_dtx_wtxe_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_wtxe_datx_s cnf75xx;
};

typedef union cvmx_dtx_wtxe_datx cvmx_dtx_wtxe_datx_t;

/**
 * cvmx_dtx_wtxe_ena#
 */
union cvmx_dtx_wtxe_enax {
	u64 u64;
	struct cvmx_dtx_wtxe_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_wtxe_enax_s cnf75xx;
};

typedef union cvmx_dtx_wtxe_enax cvmx_dtx_wtxe_enax_t;

/**
 * cvmx_dtx_wtxe_sel#
 */
union cvmx_dtx_wtxe_selx {
	u64 u64;
	struct cvmx_dtx_wtxe_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_wtxe_selx_s cnf75xx;
};

typedef union cvmx_dtx_wtxe_selx cvmx_dtx_wtxe_selx_t;

/**
 * cvmx_dtx_xcv_bcst_rsp
 */
union cvmx_dtx_xcv_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_xcv_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_xcv_bcst_rsp_s cn73xx;
	struct cvmx_dtx_xcv_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_xcv_bcst_rsp cvmx_dtx_xcv_bcst_rsp_t;

/**
 * cvmx_dtx_xcv_ctl
 */
union cvmx_dtx_xcv_ctl {
	u64 u64;
	struct cvmx_dtx_xcv_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_xcv_ctl_s cn73xx;
	struct cvmx_dtx_xcv_ctl_s cnf75xx;
};

typedef union cvmx_dtx_xcv_ctl cvmx_dtx_xcv_ctl_t;

/**
 * cvmx_dtx_xcv_dat#
 */
union cvmx_dtx_xcv_datx {
	u64 u64;
	struct cvmx_dtx_xcv_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_xcv_datx_s cn73xx;
	struct cvmx_dtx_xcv_datx_s cnf75xx;
};

typedef union cvmx_dtx_xcv_datx cvmx_dtx_xcv_datx_t;

/**
 * cvmx_dtx_xcv_ena#
 */
union cvmx_dtx_xcv_enax {
	u64 u64;
	struct cvmx_dtx_xcv_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_xcv_enax_s cn73xx;
	struct cvmx_dtx_xcv_enax_s cnf75xx;
};

typedef union cvmx_dtx_xcv_enax cvmx_dtx_xcv_enax_t;

/**
 * cvmx_dtx_xcv_sel#
 */
union cvmx_dtx_xcv_selx {
	u64 u64;
	struct cvmx_dtx_xcv_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_xcv_selx_s cn73xx;
	struct cvmx_dtx_xcv_selx_s cnf75xx;
};

typedef union cvmx_dtx_xcv_selx cvmx_dtx_xcv_selx_t;

/**
 * cvmx_dtx_xsx_bcst_rsp
 */
union cvmx_dtx_xsx_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_xsx_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_xsx_bcst_rsp_s cnf75xx;
};

typedef union cvmx_dtx_xsx_bcst_rsp cvmx_dtx_xsx_bcst_rsp_t;

/**
 * cvmx_dtx_xsx_ctl
 */
union cvmx_dtx_xsx_ctl {
	u64 u64;
	struct cvmx_dtx_xsx_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_xsx_ctl_s cnf75xx;
};

typedef union cvmx_dtx_xsx_ctl cvmx_dtx_xsx_ctl_t;

/**
 * cvmx_dtx_xsx_dat#
 */
union cvmx_dtx_xsx_datx {
	u64 u64;
	struct cvmx_dtx_xsx_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_xsx_datx_s cnf75xx;
};

typedef union cvmx_dtx_xsx_datx cvmx_dtx_xsx_datx_t;

/**
 * cvmx_dtx_xsx_ena#
 */
union cvmx_dtx_xsx_enax {
	u64 u64;
	struct cvmx_dtx_xsx_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_xsx_enax_s cnf75xx;
};

typedef union cvmx_dtx_xsx_enax cvmx_dtx_xsx_enax_t;

/**
 * cvmx_dtx_xsx_sel#
 */
union cvmx_dtx_xsx_selx {
	u64 u64;
	struct cvmx_dtx_xsx_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_xsx_selx_s cnf75xx;
};

typedef union cvmx_dtx_xsx_selx cvmx_dtx_xsx_selx_t;

/**
 * cvmx_dtx_zip_bcst_rsp
 */
union cvmx_dtx_zip_bcst_rsp {
	u64 u64;
	struct cvmx_dtx_zip_bcst_rsp_s {
		u64 reserved_1_63 : 63;
		u64 ena : 1;
	} s;
	struct cvmx_dtx_zip_bcst_rsp_s cn73xx;
	struct cvmx_dtx_zip_bcst_rsp_s cn78xx;
	struct cvmx_dtx_zip_bcst_rsp_s cn78xxp1;
};

typedef union cvmx_dtx_zip_bcst_rsp cvmx_dtx_zip_bcst_rsp_t;

/**
 * cvmx_dtx_zip_ctl
 */
union cvmx_dtx_zip_ctl {
	u64 u64;
	struct cvmx_dtx_zip_ctl_s {
		u64 reserved_5_63 : 59;
		u64 active : 1;
		u64 reserved_2_3 : 2;
		u64 echoen : 1;
		u64 swap : 1;
	} s;
	struct cvmx_dtx_zip_ctl_s cn73xx;
	struct cvmx_dtx_zip_ctl_s cn78xx;
	struct cvmx_dtx_zip_ctl_s cn78xxp1;
};

typedef union cvmx_dtx_zip_ctl cvmx_dtx_zip_ctl_t;

/**
 * cvmx_dtx_zip_dat#
 */
union cvmx_dtx_zip_datx {
	u64 u64;
	struct cvmx_dtx_zip_datx_s {
		u64 reserved_36_63 : 28;
		u64 raw : 36;
	} s;
	struct cvmx_dtx_zip_datx_s cn73xx;
	struct cvmx_dtx_zip_datx_s cn78xx;
	struct cvmx_dtx_zip_datx_s cn78xxp1;
};

typedef union cvmx_dtx_zip_datx cvmx_dtx_zip_datx_t;

/**
 * cvmx_dtx_zip_ena#
 */
union cvmx_dtx_zip_enax {
	u64 u64;
	struct cvmx_dtx_zip_enax_s {
		u64 reserved_36_63 : 28;
		u64 ena : 36;
	} s;
	struct cvmx_dtx_zip_enax_s cn73xx;
	struct cvmx_dtx_zip_enax_s cn78xx;
	struct cvmx_dtx_zip_enax_s cn78xxp1;
};

typedef union cvmx_dtx_zip_enax cvmx_dtx_zip_enax_t;

/**
 * cvmx_dtx_zip_sel#
 */
union cvmx_dtx_zip_selx {
	u64 u64;
	struct cvmx_dtx_zip_selx_s {
		u64 reserved_24_63 : 40;
		u64 value : 24;
	} s;
	struct cvmx_dtx_zip_selx_s cn73xx;
	struct cvmx_dtx_zip_selx_s cn78xx;
	struct cvmx_dtx_zip_selx_s cn78xxp1;
};

typedef union cvmx_dtx_zip_selx cvmx_dtx_zip_selx_t;

#endif
