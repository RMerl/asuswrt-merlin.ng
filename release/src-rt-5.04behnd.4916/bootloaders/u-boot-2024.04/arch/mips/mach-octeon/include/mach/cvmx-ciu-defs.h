/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon ciu.
 */

#ifndef __CVMX_CIU_DEFS_H__
#define __CVMX_CIU_DEFS_H__

#define CVMX_CIU_BIST				  (0x0001070000000730ull)
#define CVMX_CIU_BLOCK_INT			  (0x00010700000007C0ull)
#define CVMX_CIU_CIB_L2C_ENX(offset)		  (0x000107000000E100ull)
#define CVMX_CIU_CIB_L2C_RAWX(offset)		  (0x000107000000E000ull)
#define CVMX_CIU_CIB_LMCX_ENX(offset, block_id)	  (0x000107000000E300ull)
#define CVMX_CIU_CIB_LMCX_RAWX(offset, block_id)  (0x000107000000E200ull)
#define CVMX_CIU_CIB_OCLAX_ENX(offset, block_id)  (0x000107000000EE00ull)
#define CVMX_CIU_CIB_OCLAX_RAWX(offset, block_id) (0x000107000000EC00ull)
#define CVMX_CIU_CIB_RST_ENX(offset)		  (0x000107000000E500ull)
#define CVMX_CIU_CIB_RST_RAWX(offset)		  (0x000107000000E400ull)
#define CVMX_CIU_CIB_SATA_ENX(offset)		  (0x000107000000E700ull)
#define CVMX_CIU_CIB_SATA_RAWX(offset)		  (0x000107000000E600ull)
#define CVMX_CIU_CIB_USBDRDX_ENX(offset, block_id)                                                 \
	(0x000107000000EA00ull + ((block_id) & 1) * 0x100ull)
#define CVMX_CIU_CIB_USBDRDX_RAWX(offset, block_id)                                                \
	(0x000107000000E800ull + ((block_id) & 1) * 0x100ull)
#define CVMX_CIU_DINT CVMX_CIU_DINT_FUNC()
static inline u64 CVMX_CIU_DINT_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000720ull;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001010000000180ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001010000000180ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001010000000180ull;
	}
	return 0x0001010000000180ull;
}

#define CVMX_CIU_EN2_IOX_INT(offset)	 (0x000107000000A600ull + ((offset) & 1) * 8)
#define CVMX_CIU_EN2_IOX_INT_W1C(offset) (0x000107000000CE00ull + ((offset) & 1) * 8)
#define CVMX_CIU_EN2_IOX_INT_W1S(offset) (0x000107000000AE00ull + ((offset) & 1) * 8)
#define CVMX_CIU_EN2_PPX_IP2(offset)	 (0x000107000000A000ull + ((offset) & 15) * 8)
#define CVMX_CIU_EN2_PPX_IP2_W1C(offset) (0x000107000000C800ull + ((offset) & 15) * 8)
#define CVMX_CIU_EN2_PPX_IP2_W1S(offset) (0x000107000000A800ull + ((offset) & 15) * 8)
#define CVMX_CIU_EN2_PPX_IP3(offset)	 (0x000107000000A200ull + ((offset) & 15) * 8)
#define CVMX_CIU_EN2_PPX_IP3_W1C(offset) (0x000107000000CA00ull + ((offset) & 15) * 8)
#define CVMX_CIU_EN2_PPX_IP3_W1S(offset) (0x000107000000AA00ull + ((offset) & 15) * 8)
#define CVMX_CIU_EN2_PPX_IP4(offset)	 (0x000107000000A400ull + ((offset) & 15) * 8)
#define CVMX_CIU_EN2_PPX_IP4_W1C(offset) (0x000107000000CC00ull + ((offset) & 15) * 8)
#define CVMX_CIU_EN2_PPX_IP4_W1S(offset) (0x000107000000AC00ull + ((offset) & 15) * 8)
#define CVMX_CIU_FUSE			 CVMX_CIU_FUSE_FUNC()
static inline u64 CVMX_CIU_FUSE_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000728ull;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00010100000001A0ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00010100000001A0ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00010100000001A0ull;
	}
	return 0x00010100000001A0ull;
}

#define CVMX_CIU_GSTOP			(0x0001070000000710ull)
#define CVMX_CIU_INT33_SUM0		(0x0001070000000110ull)
#define CVMX_CIU_INTR_SLOWDOWN		(0x00010700000007D0ull)
#define CVMX_CIU_INTX_EN0(offset)	(0x0001070000000200ull + ((offset) & 63) * 16)
#define CVMX_CIU_INTX_EN0_W1C(offset)	(0x0001070000002200ull + ((offset) & 63) * 16)
#define CVMX_CIU_INTX_EN0_W1S(offset)	(0x0001070000006200ull + ((offset) & 63) * 16)
#define CVMX_CIU_INTX_EN1(offset)	(0x0001070000000208ull + ((offset) & 63) * 16)
#define CVMX_CIU_INTX_EN1_W1C(offset)	(0x0001070000002208ull + ((offset) & 63) * 16)
#define CVMX_CIU_INTX_EN1_W1S(offset)	(0x0001070000006208ull + ((offset) & 63) * 16)
#define CVMX_CIU_INTX_EN4_0(offset)	(0x0001070000000C80ull + ((offset) & 15) * 16)
#define CVMX_CIU_INTX_EN4_0_W1C(offset) (0x0001070000002C80ull + ((offset) & 15) * 16)
#define CVMX_CIU_INTX_EN4_0_W1S(offset) (0x0001070000006C80ull + ((offset) & 15) * 16)
#define CVMX_CIU_INTX_EN4_1(offset)	(0x0001070000000C88ull + ((offset) & 15) * 16)
#define CVMX_CIU_INTX_EN4_1_W1C(offset) (0x0001070000002C88ull + ((offset) & 15) * 16)
#define CVMX_CIU_INTX_EN4_1_W1S(offset) (0x0001070000006C88ull + ((offset) & 15) * 16)
#define CVMX_CIU_INTX_SUM0(offset)	(0x0001070000000000ull + ((offset) & 63) * 8)
#define CVMX_CIU_INTX_SUM4(offset)	(0x0001070000000C00ull + ((offset) & 15) * 8)
#define CVMX_CIU_INT_DBG_SEL		(0x00010700000007D0ull)
#define CVMX_CIU_INT_SUM1		(0x0001070000000108ull)
static inline u64 CVMX_CIU_MBOX_CLRX(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000680ull + (offset) * 8;
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000680ull + (offset) * 8;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000680ull + (offset) * 8;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001070100100600ull + (offset) * 8;
	}
	return 0x0001070000000680ull + (offset) * 8;
}

static inline u64 CVMX_CIU_MBOX_SETX(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000600ull + (offset) * 8;
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000600ull + (offset) * 8;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000600ull + (offset) * 8;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001070100100400ull + (offset) * 8;
	}
	return 0x0001070000000600ull + (offset) * 8;
}

#define CVMX_CIU_NMI	      (0x0001070000000718ull)
#define CVMX_CIU_PCI_INTA     (0x0001070000000750ull)
#define CVMX_CIU_PP_BIST_STAT (0x00010700000007E0ull)
#define CVMX_CIU_PP_DBG	      CVMX_CIU_PP_DBG_FUNC()
static inline u64 CVMX_CIU_PP_DBG_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000708ull;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001010000000120ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001010000000120ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001010000000120ull;
	}
	return 0x0001010000000120ull;
}

static inline u64 CVMX_CIU_PP_POKEX(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000580ull + (offset) * 8;
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000580ull + (offset) * 8;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000580ull + (offset) * 8;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001010000030000ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001010000030000ull + (offset) * 8;

	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001010000030000ull + (offset) * 8;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001070100100200ull + (offset) * 8;
	}
	return 0x0001010000030000ull + (offset) * 8;
}

#define CVMX_CIU_PP_RST CVMX_CIU_PP_RST_FUNC()
static inline u64 CVMX_CIU_PP_RST_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000700ull;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001010000000100ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001010000000100ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001010000000100ull;
	}
	return 0x0001010000000100ull;
}

#define CVMX_CIU_PP_RST_PENDING CVMX_CIU_PP_RST_PENDING_FUNC()
static inline u64 CVMX_CIU_PP_RST_PENDING_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001010000000110ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001010000000110ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001010000000110ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000740ull;
	}
	return 0x0001010000000110ull;
}

#define CVMX_CIU_QLM0		      (0x0001070000000780ull)
#define CVMX_CIU_QLM1		      (0x0001070000000788ull)
#define CVMX_CIU_QLM2		      (0x0001070000000790ull)
#define CVMX_CIU_QLM3		      (0x0001070000000798ull)
#define CVMX_CIU_QLM4		      (0x00010700000007A0ull)
#define CVMX_CIU_QLM_DCOK	      (0x0001070000000760ull)
#define CVMX_CIU_QLM_JTGC	      (0x0001070000000768ull)
#define CVMX_CIU_QLM_JTGD	      (0x0001070000000770ull)
#define CVMX_CIU_SOFT_BIST	      (0x0001070000000738ull)
#define CVMX_CIU_SOFT_PRST	      (0x0001070000000748ull)
#define CVMX_CIU_SOFT_PRST1	      (0x0001070000000758ull)
#define CVMX_CIU_SOFT_PRST2	      (0x00010700000007D8ull)
#define CVMX_CIU_SOFT_PRST3	      (0x00010700000007E0ull)
#define CVMX_CIU_SOFT_RST	      (0x0001070000000740ull)
#define CVMX_CIU_SUM1_IOX_INT(offset) (0x0001070000008600ull + ((offset) & 1) * 8)
#define CVMX_CIU_SUM1_PPX_IP2(offset) (0x0001070000008000ull + ((offset) & 15) * 8)
#define CVMX_CIU_SUM1_PPX_IP3(offset) (0x0001070000008200ull + ((offset) & 15) * 8)
#define CVMX_CIU_SUM1_PPX_IP4(offset) (0x0001070000008400ull + ((offset) & 15) * 8)
#define CVMX_CIU_SUM2_IOX_INT(offset) (0x0001070000008E00ull + ((offset) & 1) * 8)
#define CVMX_CIU_SUM2_PPX_IP2(offset) (0x0001070000008800ull + ((offset) & 15) * 8)
#define CVMX_CIU_SUM2_PPX_IP3(offset) (0x0001070000008A00ull + ((offset) & 15) * 8)
#define CVMX_CIU_SUM2_PPX_IP4(offset) (0x0001070000008C00ull + ((offset) & 15) * 8)
#define CVMX_CIU_TIMX(offset)	      (0x0001070000000480ull + ((offset) & 15) * 8)
#define CVMX_CIU_TIM_MULTI_CAST	      CVMX_CIU_TIM_MULTI_CAST_FUNC()
static inline u64 CVMX_CIU_TIM_MULTI_CAST_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00010700000004F0ull;
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x000107000000C200ull;
	}
	return 0x000107000000C200ull;
}

static inline u64 CVMX_CIU_WDOGX(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000500ull + (offset) * 8;
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000500ull + (offset) * 8;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001070000000500ull + (offset) * 8;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001010000020000ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001010000020000ull + (offset) * 8;

	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001010000020000ull + (offset) * 8;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001070100100000ull + (offset) * 8;
	}
	return 0x0001010000020000ull + (offset) * 8;
}

/**
 * cvmx_ciu_bist
 */
union cvmx_ciu_bist {
	u64 u64;
	struct cvmx_ciu_bist_s {
		u64 reserved_7_63 : 57;
		u64 bist : 7;
	} s;
	struct cvmx_ciu_bist_cn30xx {
		u64 reserved_4_63 : 60;
		u64 bist : 4;
	} cn30xx;
	struct cvmx_ciu_bist_cn30xx cn31xx;
	struct cvmx_ciu_bist_cn30xx cn38xx;
	struct cvmx_ciu_bist_cn30xx cn38xxp2;
	struct cvmx_ciu_bist_cn50xx {
		u64 reserved_2_63 : 62;
		u64 bist : 2;
	} cn50xx;
	struct cvmx_ciu_bist_cn52xx {
		u64 reserved_3_63 : 61;
		u64 bist : 3;
	} cn52xx;
	struct cvmx_ciu_bist_cn52xx cn52xxp1;
	struct cvmx_ciu_bist_cn30xx cn56xx;
	struct cvmx_ciu_bist_cn30xx cn56xxp1;
	struct cvmx_ciu_bist_cn30xx cn58xx;
	struct cvmx_ciu_bist_cn30xx cn58xxp1;
	struct cvmx_ciu_bist_cn61xx {
		u64 reserved_6_63 : 58;
		u64 bist : 6;
	} cn61xx;
	struct cvmx_ciu_bist_cn63xx {
		u64 reserved_5_63 : 59;
		u64 bist : 5;
	} cn63xx;
	struct cvmx_ciu_bist_cn63xx cn63xxp1;
	struct cvmx_ciu_bist_cn61xx cn66xx;
	struct cvmx_ciu_bist_s cn68xx;
	struct cvmx_ciu_bist_s cn68xxp1;
	struct cvmx_ciu_bist_cn52xx cn70xx;
	struct cvmx_ciu_bist_cn52xx cn70xxp1;
	struct cvmx_ciu_bist_cn61xx cnf71xx;
};

typedef union cvmx_ciu_bist cvmx_ciu_bist_t;

/**
 * cvmx_ciu_block_int
 *
 * CIU_BLOCK_INT = CIU Blocks Interrupt
 *
 * The interrupt lines from the various chip blocks.
 */
union cvmx_ciu_block_int {
	u64 u64;
	struct cvmx_ciu_block_int_s {
		u64 reserved_62_63 : 2;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_43_59 : 17;
		u64 ptp : 1;
		u64 dpi : 1;
		u64 dfm : 1;
		u64 reserved_34_39 : 6;
		u64 srio1 : 1;
		u64 srio0 : 1;
		u64 reserved_31_31 : 1;
		u64 iob : 1;
		u64 reserved_29_29 : 1;
		u64 agl : 1;
		u64 reserved_27_27 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 reserved_24_24 : 1;
		u64 asxpcs1 : 1;
		u64 asxpcs0 : 1;
		u64 reserved_21_21 : 1;
		u64 pip : 1;
		u64 reserved_18_19 : 2;
		u64 lmc0 : 1;
		u64 l2c : 1;
		u64 reserved_15_15 : 1;
		u64 rad : 1;
		u64 usb : 1;
		u64 pow : 1;
		u64 tim : 1;
		u64 pko : 1;
		u64 ipd : 1;
		u64 reserved_8_8 : 1;
		u64 zip : 1;
		u64 dfa : 1;
		u64 fpa : 1;
		u64 key : 1;
		u64 sli : 1;
		u64 gmx1 : 1;
		u64 gmx0 : 1;
		u64 mio : 1;
	} s;
	struct cvmx_ciu_block_int_cn61xx {
		u64 reserved_43_63 : 21;
		u64 ptp : 1;
		u64 dpi : 1;
		u64 reserved_31_40 : 10;
		u64 iob : 1;
		u64 reserved_29_29 : 1;
		u64 agl : 1;
		u64 reserved_27_27 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 reserved_24_24 : 1;
		u64 asxpcs1 : 1;
		u64 asxpcs0 : 1;
		u64 reserved_21_21 : 1;
		u64 pip : 1;
		u64 reserved_18_19 : 2;
		u64 lmc0 : 1;
		u64 l2c : 1;
		u64 reserved_15_15 : 1;
		u64 rad : 1;
		u64 usb : 1;
		u64 pow : 1;
		u64 tim : 1;
		u64 pko : 1;
		u64 ipd : 1;
		u64 reserved_8_8 : 1;
		u64 zip : 1;
		u64 dfa : 1;
		u64 fpa : 1;
		u64 key : 1;
		u64 sli : 1;
		u64 gmx1 : 1;
		u64 gmx0 : 1;
		u64 mio : 1;
	} cn61xx;
	struct cvmx_ciu_block_int_cn63xx {
		u64 reserved_43_63 : 21;
		u64 ptp : 1;
		u64 dpi : 1;
		u64 dfm : 1;
		u64 reserved_34_39 : 6;
		u64 srio1 : 1;
		u64 srio0 : 1;
		u64 reserved_31_31 : 1;
		u64 iob : 1;
		u64 reserved_29_29 : 1;
		u64 agl : 1;
		u64 reserved_27_27 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 reserved_23_24 : 2;
		u64 asxpcs0 : 1;
		u64 reserved_21_21 : 1;
		u64 pip : 1;
		u64 reserved_18_19 : 2;
		u64 lmc0 : 1;
		u64 l2c : 1;
		u64 reserved_15_15 : 1;
		u64 rad : 1;
		u64 usb : 1;
		u64 pow : 1;
		u64 tim : 1;
		u64 pko : 1;
		u64 ipd : 1;
		u64 reserved_8_8 : 1;
		u64 zip : 1;
		u64 dfa : 1;
		u64 fpa : 1;
		u64 key : 1;
		u64 sli : 1;
		u64 reserved_2_2 : 1;
		u64 gmx0 : 1;
		u64 mio : 1;
	} cn63xx;
	struct cvmx_ciu_block_int_cn63xx cn63xxp1;
	struct cvmx_ciu_block_int_cn66xx {
		u64 reserved_62_63 : 2;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_43_59 : 17;
		u64 ptp : 1;
		u64 dpi : 1;
		u64 dfm : 1;
		u64 reserved_33_39 : 7;
		u64 srio0 : 1;
		u64 reserved_31_31 : 1;
		u64 iob : 1;
		u64 reserved_29_29 : 1;
		u64 agl : 1;
		u64 reserved_27_27 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 reserved_24_24 : 1;
		u64 asxpcs1 : 1;
		u64 asxpcs0 : 1;
		u64 reserved_21_21 : 1;
		u64 pip : 1;
		u64 reserved_18_19 : 2;
		u64 lmc0 : 1;
		u64 l2c : 1;
		u64 reserved_15_15 : 1;
		u64 rad : 1;
		u64 usb : 1;
		u64 pow : 1;
		u64 tim : 1;
		u64 pko : 1;
		u64 ipd : 1;
		u64 reserved_8_8 : 1;
		u64 zip : 1;
		u64 dfa : 1;
		u64 fpa : 1;
		u64 key : 1;
		u64 sli : 1;
		u64 gmx1 : 1;
		u64 gmx0 : 1;
		u64 mio : 1;
	} cn66xx;
	struct cvmx_ciu_block_int_cnf71xx {
		u64 reserved_43_63 : 21;
		u64 ptp : 1;
		u64 dpi : 1;
		u64 reserved_31_40 : 10;
		u64 iob : 1;
		u64 reserved_27_29 : 3;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 reserved_23_24 : 2;
		u64 asxpcs0 : 1;
		u64 reserved_21_21 : 1;
		u64 pip : 1;
		u64 reserved_18_19 : 2;
		u64 lmc0 : 1;
		u64 l2c : 1;
		u64 reserved_15_15 : 1;
		u64 rad : 1;
		u64 usb : 1;
		u64 pow : 1;
		u64 tim : 1;
		u64 pko : 1;
		u64 ipd : 1;
		u64 reserved_6_8 : 3;
		u64 fpa : 1;
		u64 key : 1;
		u64 sli : 1;
		u64 reserved_2_2 : 1;
		u64 gmx0 : 1;
		u64 mio : 1;
	} cnf71xx;
};

typedef union cvmx_ciu_block_int cvmx_ciu_block_int_t;

/**
 * cvmx_ciu_cib_l2c_en#
 */
union cvmx_ciu_cib_l2c_enx {
	u64 u64;
	struct cvmx_ciu_cib_l2c_enx_s {
		u64 reserved_23_63 : 41;
		u64 cbcx_int_ioccmddbe : 1;
		u64 cbcx_int_ioccmdsbe : 1;
		u64 cbcx_int_rsddbe : 1;
		u64 cbcx_int_rsdsbe : 1;
		u64 mcix_int_vbfdbe : 1;
		u64 mcix_int_vbfsbe : 1;
		u64 tadx_int_rtgdbe : 1;
		u64 tadx_int_rtgsbe : 1;
		u64 tadx_int_rddislmc : 1;
		u64 tadx_int_wrdislmc : 1;
		u64 tadx_int_bigrd : 1;
		u64 tadx_int_bigwr : 1;
		u64 tadx_int_holerd : 1;
		u64 tadx_int_holewr : 1;
		u64 tadx_int_noway : 1;
		u64 tadx_int_tagdbe : 1;
		u64 tadx_int_tagsbe : 1;
		u64 tadx_int_fbfdbe : 1;
		u64 tadx_int_fbfsbe : 1;
		u64 tadx_int_sbfdbe : 1;
		u64 tadx_int_sbfsbe : 1;
		u64 tadx_int_l2ddbe : 1;
		u64 tadx_int_l2dsbe : 1;
	} s;
	struct cvmx_ciu_cib_l2c_enx_s cn70xx;
	struct cvmx_ciu_cib_l2c_enx_s cn70xxp1;
};

typedef union cvmx_ciu_cib_l2c_enx cvmx_ciu_cib_l2c_enx_t;

/**
 * cvmx_ciu_cib_l2c_raw#
 */
union cvmx_ciu_cib_l2c_rawx {
	u64 u64;
	struct cvmx_ciu_cib_l2c_rawx_s {
		u64 reserved_23_63 : 41;
		u64 cbcx_int_ioccmddbe : 1;
		u64 cbcx_int_ioccmdsbe : 1;
		u64 cbcx_int_rsddbe : 1;
		u64 cbcx_int_rsdsbe : 1;
		u64 mcix_int_vbfdbe : 1;
		u64 mcix_int_vbfsbe : 1;
		u64 tadx_int_rtgdbe : 1;
		u64 tadx_int_rtgsbe : 1;
		u64 tadx_int_rddislmc : 1;
		u64 tadx_int_wrdislmc : 1;
		u64 tadx_int_bigrd : 1;
		u64 tadx_int_bigwr : 1;
		u64 tadx_int_holerd : 1;
		u64 tadx_int_holewr : 1;
		u64 tadx_int_noway : 1;
		u64 tadx_int_tagdbe : 1;
		u64 tadx_int_tagsbe : 1;
		u64 tadx_int_fbfdbe : 1;
		u64 tadx_int_fbfsbe : 1;
		u64 tadx_int_sbfdbe : 1;
		u64 tadx_int_sbfsbe : 1;
		u64 tadx_int_l2ddbe : 1;
		u64 tadx_int_l2dsbe : 1;
	} s;
	struct cvmx_ciu_cib_l2c_rawx_s cn70xx;
	struct cvmx_ciu_cib_l2c_rawx_s cn70xxp1;
};

typedef union cvmx_ciu_cib_l2c_rawx cvmx_ciu_cib_l2c_rawx_t;

/**
 * cvmx_ciu_cib_lmc#_en#
 */
union cvmx_ciu_cib_lmcx_enx {
	u64 u64;
	struct cvmx_ciu_cib_lmcx_enx_s {
		u64 reserved_12_63 : 52;
		u64 int_ddr_err : 1;
		u64 int_dlc_ded : 1;
		u64 int_dlc_sec : 1;
		u64 int_ded_errx : 4;
		u64 int_sec_errx : 4;
		u64 int_nxm_wr_err : 1;
	} s;
	struct cvmx_ciu_cib_lmcx_enx_s cn70xx;
	struct cvmx_ciu_cib_lmcx_enx_s cn70xxp1;
};

typedef union cvmx_ciu_cib_lmcx_enx cvmx_ciu_cib_lmcx_enx_t;

/**
 * cvmx_ciu_cib_lmc#_raw#
 */
union cvmx_ciu_cib_lmcx_rawx {
	u64 u64;
	struct cvmx_ciu_cib_lmcx_rawx_s {
		u64 reserved_12_63 : 52;
		u64 int_ddr_err : 1;
		u64 int_dlc_ded : 1;
		u64 int_dlc_sec : 1;
		u64 int_ded_errx : 4;
		u64 int_sec_errx : 4;
		u64 int_nxm_wr_err : 1;
	} s;
	struct cvmx_ciu_cib_lmcx_rawx_s cn70xx;
	struct cvmx_ciu_cib_lmcx_rawx_s cn70xxp1;
};

typedef union cvmx_ciu_cib_lmcx_rawx cvmx_ciu_cib_lmcx_rawx_t;

/**
 * cvmx_ciu_cib_ocla#_en#
 */
union cvmx_ciu_cib_oclax_enx {
	u64 u64;
	struct cvmx_ciu_cib_oclax_enx_s {
		u64 reserved_15_63 : 49;
		u64 state_ddrfull : 1;
		u64 state_wmark : 1;
		u64 state_overfull : 1;
		u64 state_trigfull : 1;
		u64 state_captured : 1;
		u64 state_fsm1_int : 1;
		u64 state_fsm0_int : 1;
		u64 state_mcdx : 3;
		u64 state_trig : 1;
		u64 state_ovflx : 4;
	} s;
	struct cvmx_ciu_cib_oclax_enx_s cn70xx;
	struct cvmx_ciu_cib_oclax_enx_s cn70xxp1;
};

typedef union cvmx_ciu_cib_oclax_enx cvmx_ciu_cib_oclax_enx_t;

/**
 * cvmx_ciu_cib_ocla#_raw#
 */
union cvmx_ciu_cib_oclax_rawx {
	u64 u64;
	struct cvmx_ciu_cib_oclax_rawx_s {
		u64 reserved_15_63 : 49;
		u64 state_ddrfull : 1;
		u64 state_wmark : 1;
		u64 state_overfull : 1;
		u64 state_trigfull : 1;
		u64 state_captured : 1;
		u64 state_fsm1_int : 1;
		u64 state_fsm0_int : 1;
		u64 state_mcdx : 3;
		u64 state_trig : 1;
		u64 state_ovflx : 4;
	} s;
	struct cvmx_ciu_cib_oclax_rawx_s cn70xx;
	struct cvmx_ciu_cib_oclax_rawx_s cn70xxp1;
};

typedef union cvmx_ciu_cib_oclax_rawx cvmx_ciu_cib_oclax_rawx_t;

/**
 * cvmx_ciu_cib_rst_en#
 */
union cvmx_ciu_cib_rst_enx {
	u64 u64;
	struct cvmx_ciu_cib_rst_enx_s {
		u64 reserved_6_63 : 58;
		u64 int_perstx : 3;
		u64 int_linkx : 3;
	} s;
	struct cvmx_ciu_cib_rst_enx_s cn70xx;
	struct cvmx_ciu_cib_rst_enx_s cn70xxp1;
};

typedef union cvmx_ciu_cib_rst_enx cvmx_ciu_cib_rst_enx_t;

/**
 * cvmx_ciu_cib_rst_raw#
 */
union cvmx_ciu_cib_rst_rawx {
	u64 u64;
	struct cvmx_ciu_cib_rst_rawx_s {
		u64 reserved_6_63 : 58;
		u64 int_perstx : 3;
		u64 int_linkx : 3;
	} s;
	struct cvmx_ciu_cib_rst_rawx_s cn70xx;
	struct cvmx_ciu_cib_rst_rawx_s cn70xxp1;
};

typedef union cvmx_ciu_cib_rst_rawx cvmx_ciu_cib_rst_rawx_t;

/**
 * cvmx_ciu_cib_sata_en#
 */
union cvmx_ciu_cib_sata_enx {
	u64 u64;
	struct cvmx_ciu_cib_sata_enx_s {
		u64 reserved_4_63 : 60;
		u64 uahc_pme_req_ip : 1;
		u64 uahc_intrq_ip : 1;
		u64 intstat_xm_bad_dma : 1;
		u64 intstat_xs_ncb_oob : 1;
	} s;
	struct cvmx_ciu_cib_sata_enx_s cn70xx;
	struct cvmx_ciu_cib_sata_enx_s cn70xxp1;
};

typedef union cvmx_ciu_cib_sata_enx cvmx_ciu_cib_sata_enx_t;

/**
 * cvmx_ciu_cib_sata_raw#
 */
union cvmx_ciu_cib_sata_rawx {
	u64 u64;
	struct cvmx_ciu_cib_sata_rawx_s {
		u64 reserved_4_63 : 60;
		u64 uahc_pme_req_ip : 1;
		u64 uahc_intrq_ip : 1;
		u64 intstat_xm_bad_dma : 1;
		u64 intstat_xs_ncb_oob : 1;
	} s;
	struct cvmx_ciu_cib_sata_rawx_s cn70xx;
	struct cvmx_ciu_cib_sata_rawx_s cn70xxp1;
};

typedef union cvmx_ciu_cib_sata_rawx cvmx_ciu_cib_sata_rawx_t;

/**
 * cvmx_ciu_cib_usbdrd#_en#
 */
union cvmx_ciu_cib_usbdrdx_enx {
	u64 u64;
	struct cvmx_ciu_cib_usbdrdx_enx_s {
		u64 reserved_11_63 : 53;
		u64 uahc_dev_int : 1;
		u64 uahc_imanx_ip : 1;
		u64 uahc_usbsts_hse : 1;
		u64 intstat_ram2_dbe : 1;
		u64 intstat_ram2_sbe : 1;
		u64 intstat_ram1_dbe : 1;
		u64 intstat_ram1_sbe : 1;
		u64 intstat_ram0_dbe : 1;
		u64 intstat_ram0_sbe : 1;
		u64 intstat_xm_bad_dma : 1;
		u64 intstat_xs_ncb_oob : 1;
	} s;
	struct cvmx_ciu_cib_usbdrdx_enx_s cn70xx;
	struct cvmx_ciu_cib_usbdrdx_enx_s cn70xxp1;
};

typedef union cvmx_ciu_cib_usbdrdx_enx cvmx_ciu_cib_usbdrdx_enx_t;

/**
 * cvmx_ciu_cib_usbdrd#_raw#
 */
union cvmx_ciu_cib_usbdrdx_rawx {
	u64 u64;
	struct cvmx_ciu_cib_usbdrdx_rawx_s {
		u64 reserved_11_63 : 53;
		u64 uahc_dev_int : 1;
		u64 uahc_imanx_ip : 1;
		u64 uahc_usbsts_hse : 1;
		u64 intstat_ram2_dbe : 1;
		u64 intstat_ram2_sbe : 1;
		u64 intstat_ram1_dbe : 1;
		u64 intstat_ram1_sbe : 1;
		u64 intstat_ram0_dbe : 1;
		u64 intstat_ram0_sbe : 1;
		u64 intstat_xm_bad_dma : 1;
		u64 intstat_xs_ncb_oob : 1;
	} s;
	struct cvmx_ciu_cib_usbdrdx_rawx_s cn70xx;
	struct cvmx_ciu_cib_usbdrdx_rawx_s cn70xxp1;
};

typedef union cvmx_ciu_cib_usbdrdx_rawx cvmx_ciu_cib_usbdrdx_rawx_t;

/**
 * cvmx_ciu_dint
 */
union cvmx_ciu_dint {
	u64 u64;
	struct cvmx_ciu_dint_s {
		u64 reserved_48_63 : 16;
		u64 dint : 48;
	} s;
	struct cvmx_ciu_dint_cn30xx {
		u64 reserved_1_63 : 63;
		u64 dint : 1;
	} cn30xx;
	struct cvmx_ciu_dint_cn31xx {
		u64 reserved_2_63 : 62;
		u64 dint : 2;
	} cn31xx;
	struct cvmx_ciu_dint_cn38xx {
		u64 reserved_16_63 : 48;
		u64 dint : 16;
	} cn38xx;
	struct cvmx_ciu_dint_cn38xx cn38xxp2;
	struct cvmx_ciu_dint_cn31xx cn50xx;
	struct cvmx_ciu_dint_cn52xx {
		u64 reserved_4_63 : 60;
		u64 dint : 4;
	} cn52xx;
	struct cvmx_ciu_dint_cn52xx cn52xxp1;
	struct cvmx_ciu_dint_cn56xx {
		u64 reserved_12_63 : 52;
		u64 dint : 12;
	} cn56xx;
	struct cvmx_ciu_dint_cn56xx cn56xxp1;
	struct cvmx_ciu_dint_cn38xx cn58xx;
	struct cvmx_ciu_dint_cn38xx cn58xxp1;
	struct cvmx_ciu_dint_cn52xx cn61xx;
	struct cvmx_ciu_dint_cn63xx {
		u64 reserved_6_63 : 58;
		u64 dint : 6;
	} cn63xx;
	struct cvmx_ciu_dint_cn63xx cn63xxp1;
	struct cvmx_ciu_dint_cn66xx {
		u64 reserved_10_63 : 54;
		u64 dint : 10;
	} cn66xx;
	struct cvmx_ciu_dint_cn68xx {
		u64 reserved_32_63 : 32;
		u64 dint : 32;
	} cn68xx;
	struct cvmx_ciu_dint_cn68xx cn68xxp1;
	struct cvmx_ciu_dint_cn52xx cn70xx;
	struct cvmx_ciu_dint_cn52xx cn70xxp1;
	struct cvmx_ciu_dint_cn38xx cn73xx;
	struct cvmx_ciu_dint_s cn78xx;
	struct cvmx_ciu_dint_s cn78xxp1;
	struct cvmx_ciu_dint_cn52xx cnf71xx;
	struct cvmx_ciu_dint_cn38xx cnf75xx;
};

typedef union cvmx_ciu_dint cvmx_ciu_dint_t;

/**
 * cvmx_ciu_en2_io#_int
 *
 * CIU_EN2_IO0_INT is for PEM0, CIU_EN2_IO1_INT is reserved.
 *
 */
union cvmx_ciu_en2_iox_int {
	u64 u64;
	struct cvmx_ciu_en2_iox_int_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_en2_iox_int_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_en2_iox_int_cn61xx cn66xx;
	struct cvmx_ciu_en2_iox_int_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_en2_iox_int_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_iox_int_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_en2_iox_int cvmx_ciu_en2_iox_int_t;

/**
 * cvmx_ciu_en2_io#_int_w1c
 *
 * CIU_EN2_IO0_INT_W1C is for PEM0, CIU_EN2_IO1_INT_W1C is reserved.
 *
 */
union cvmx_ciu_en2_iox_int_w1c {
	u64 u64;
	struct cvmx_ciu_en2_iox_int_w1c_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_en2_iox_int_w1c_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_en2_iox_int_w1c_cn61xx cn66xx;
	struct cvmx_ciu_en2_iox_int_w1c_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_en2_iox_int_w1c_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_iox_int_w1c_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_en2_iox_int_w1c cvmx_ciu_en2_iox_int_w1c_t;

/**
 * cvmx_ciu_en2_io#_int_w1s
 *
 * CIU_EN2_IO0_INT_W1S is for PEM0, CIU_EN2_IO1_INT_W1S is reserved.
 *
 */
union cvmx_ciu_en2_iox_int_w1s {
	u64 u64;
	struct cvmx_ciu_en2_iox_int_w1s_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_en2_iox_int_w1s_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_en2_iox_int_w1s_cn61xx cn66xx;
	struct cvmx_ciu_en2_iox_int_w1s_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_en2_iox_int_w1s_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_iox_int_w1s_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_en2_iox_int_w1s cvmx_ciu_en2_iox_int_w1s_t;

/**
 * cvmx_ciu_en2_pp#_ip2
 *
 * Notes:
 * These SUM2 CSR's did not exist prior to pass 1.2. CIU_TIM4-9 did not exist prior to pass 1.2.
 *
 */
union cvmx_ciu_en2_ppx_ip2 {
	u64 u64;
	struct cvmx_ciu_en2_ppx_ip2_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_en2_ppx_ip2_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip2_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip2_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip2_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip2_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_en2_ppx_ip2 cvmx_ciu_en2_ppx_ip2_t;

/**
 * cvmx_ciu_en2_pp#_ip2_w1c
 *
 * Write-1-to-clear version of the CIU_EN2_PP(IO)X_IPx(INT) register, read back corresponding
 * CIU_EN2_PP(IO)X_IPx(INT) value.
 */
union cvmx_ciu_en2_ppx_ip2_w1c {
	u64 u64;
	struct cvmx_ciu_en2_ppx_ip2_w1c_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_en2_ppx_ip2_w1c_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip2_w1c_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip2_w1c_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip2_w1c_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip2_w1c_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_en2_ppx_ip2_w1c cvmx_ciu_en2_ppx_ip2_w1c_t;

/**
 * cvmx_ciu_en2_pp#_ip2_w1s
 *
 * Write-1-to-set version of the CIU_EN2_PP(IO)X_IPx(INT) register, read back corresponding
 * CIU_EN2_PP(IO)X_IPx(INT) value.
 */
union cvmx_ciu_en2_ppx_ip2_w1s {
	u64 u64;
	struct cvmx_ciu_en2_ppx_ip2_w1s_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_en2_ppx_ip2_w1s_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip2_w1s_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip2_w1s_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip2_w1s_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip2_w1s_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_en2_ppx_ip2_w1s cvmx_ciu_en2_ppx_ip2_w1s_t;

/**
 * cvmx_ciu_en2_pp#_ip3
 *
 * Notes:
 * These SUM2 CSR's did not exist prior to pass 1.2. CIU_TIM4-9 did not exist prior to pass 1.2.
 *
 */
union cvmx_ciu_en2_ppx_ip3 {
	u64 u64;
	struct cvmx_ciu_en2_ppx_ip3_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_en2_ppx_ip3_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip3_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip3_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip3_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip3_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_en2_ppx_ip3 cvmx_ciu_en2_ppx_ip3_t;

/**
 * cvmx_ciu_en2_pp#_ip3_w1c
 *
 * Notes:
 * Write-1-to-clear version of the CIU_EN2_PP(IO)X_IPx(INT) register, read back corresponding
 * CIU_EN2_PP(IO)X_IPx(INT) value.
 */
union cvmx_ciu_en2_ppx_ip3_w1c {
	u64 u64;
	struct cvmx_ciu_en2_ppx_ip3_w1c_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_en2_ppx_ip3_w1c_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip3_w1c_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip3_w1c_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip3_w1c_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip3_w1c_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_en2_ppx_ip3_w1c cvmx_ciu_en2_ppx_ip3_w1c_t;

/**
 * cvmx_ciu_en2_pp#_ip3_w1s
 *
 * Notes:
 * Write-1-to-set version of the CIU_EN2_PP(IO)X_IPx(INT) register, read back corresponding
 * CIU_EN2_PP(IO)X_IPx(INT) value.
 */
union cvmx_ciu_en2_ppx_ip3_w1s {
	u64 u64;
	struct cvmx_ciu_en2_ppx_ip3_w1s_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_en2_ppx_ip3_w1s_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip3_w1s_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip3_w1s_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip3_w1s_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip3_w1s_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_en2_ppx_ip3_w1s cvmx_ciu_en2_ppx_ip3_w1s_t;

/**
 * cvmx_ciu_en2_pp#_ip4
 *
 * Notes:
 * These SUM2 CSR's did not exist prior to pass 1.2. CIU_TIM4-9 did not exist prior to pass 1.2.
 *
 */
union cvmx_ciu_en2_ppx_ip4 {
	u64 u64;
	struct cvmx_ciu_en2_ppx_ip4_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_en2_ppx_ip4_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip4_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip4_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip4_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip4_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_en2_ppx_ip4 cvmx_ciu_en2_ppx_ip4_t;

/**
 * cvmx_ciu_en2_pp#_ip4_w1c
 *
 * Notes:
 * Write-1-to-clear version of the CIU_EN2_PP(IO)X_IPx(INT) register, read back corresponding
 * CIU_EN2_PP(IO)X_IPx(INT) value.
 */
union cvmx_ciu_en2_ppx_ip4_w1c {
	u64 u64;
	struct cvmx_ciu_en2_ppx_ip4_w1c_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_en2_ppx_ip4_w1c_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip4_w1c_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip4_w1c_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip4_w1c_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip4_w1c_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_en2_ppx_ip4_w1c cvmx_ciu_en2_ppx_ip4_w1c_t;

/**
 * cvmx_ciu_en2_pp#_ip4_w1s
 *
 * Notes:
 * Write-1-to-set version of the CIU_EN2_PP(IO)X_IPx(INT) register, read back corresponding
 * CIU_EN2_PP(IO)X_IPx(INT) value.
 */
union cvmx_ciu_en2_ppx_ip4_w1s {
	u64 u64;
	struct cvmx_ciu_en2_ppx_ip4_w1s_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_en2_ppx_ip4_w1s_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip4_w1s_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip4_w1s_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip4_w1s_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip4_w1s_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_en2_ppx_ip4_w1s cvmx_ciu_en2_ppx_ip4_w1s_t;

/**
 * cvmx_ciu_fuse
 */
union cvmx_ciu_fuse {
	u64 u64;
	struct cvmx_ciu_fuse_s {
		u64 reserved_48_63 : 16;
		u64 fuse : 48;
	} s;
	struct cvmx_ciu_fuse_cn30xx {
		u64 reserved_1_63 : 63;
		u64 fuse : 1;
	} cn30xx;
	struct cvmx_ciu_fuse_cn31xx {
		u64 reserved_2_63 : 62;
		u64 fuse : 2;
	} cn31xx;
	struct cvmx_ciu_fuse_cn38xx {
		u64 reserved_16_63 : 48;
		u64 fuse : 16;
	} cn38xx;
	struct cvmx_ciu_fuse_cn38xx cn38xxp2;
	struct cvmx_ciu_fuse_cn31xx cn50xx;
	struct cvmx_ciu_fuse_cn52xx {
		u64 reserved_4_63 : 60;
		u64 fuse : 4;
	} cn52xx;
	struct cvmx_ciu_fuse_cn52xx cn52xxp1;
	struct cvmx_ciu_fuse_cn56xx {
		u64 reserved_12_63 : 52;
		u64 fuse : 12;
	} cn56xx;
	struct cvmx_ciu_fuse_cn56xx cn56xxp1;
	struct cvmx_ciu_fuse_cn38xx cn58xx;
	struct cvmx_ciu_fuse_cn38xx cn58xxp1;
	struct cvmx_ciu_fuse_cn52xx cn61xx;
	struct cvmx_ciu_fuse_cn63xx {
		u64 reserved_6_63 : 58;
		u64 fuse : 6;
	} cn63xx;
	struct cvmx_ciu_fuse_cn63xx cn63xxp1;
	struct cvmx_ciu_fuse_cn66xx {
		u64 reserved_10_63 : 54;
		u64 fuse : 10;
	} cn66xx;
	struct cvmx_ciu_fuse_cn68xx {
		u64 reserved_32_63 : 32;
		u64 fuse : 32;
	} cn68xx;
	struct cvmx_ciu_fuse_cn68xx cn68xxp1;
	struct cvmx_ciu_fuse_cn52xx cn70xx;
	struct cvmx_ciu_fuse_cn52xx cn70xxp1;
	struct cvmx_ciu_fuse_cn38xx cn73xx;
	struct cvmx_ciu_fuse_s cn78xx;
	struct cvmx_ciu_fuse_s cn78xxp1;
	struct cvmx_ciu_fuse_cn52xx cnf71xx;
	struct cvmx_ciu_fuse_cn38xx cnf75xx;
};

typedef union cvmx_ciu_fuse cvmx_ciu_fuse_t;

/**
 * cvmx_ciu_gstop
 */
union cvmx_ciu_gstop {
	u64 u64;
	struct cvmx_ciu_gstop_s {
		u64 reserved_1_63 : 63;
		u64 gstop : 1;
	} s;
	struct cvmx_ciu_gstop_s cn30xx;
	struct cvmx_ciu_gstop_s cn31xx;
	struct cvmx_ciu_gstop_s cn38xx;
	struct cvmx_ciu_gstop_s cn38xxp2;
	struct cvmx_ciu_gstop_s cn50xx;
	struct cvmx_ciu_gstop_s cn52xx;
	struct cvmx_ciu_gstop_s cn52xxp1;
	struct cvmx_ciu_gstop_s cn56xx;
	struct cvmx_ciu_gstop_s cn56xxp1;
	struct cvmx_ciu_gstop_s cn58xx;
	struct cvmx_ciu_gstop_s cn58xxp1;
	struct cvmx_ciu_gstop_s cn61xx;
	struct cvmx_ciu_gstop_s cn63xx;
	struct cvmx_ciu_gstop_s cn63xxp1;
	struct cvmx_ciu_gstop_s cn66xx;
	struct cvmx_ciu_gstop_s cn68xx;
	struct cvmx_ciu_gstop_s cn68xxp1;
	struct cvmx_ciu_gstop_s cn70xx;
	struct cvmx_ciu_gstop_s cn70xxp1;
	struct cvmx_ciu_gstop_s cnf71xx;
};

typedef union cvmx_ciu_gstop cvmx_ciu_gstop_t;

/**
 * cvmx_ciu_int#_en0
 *
 * CIU_INT0_EN0:  PP0/IP2
 * CIU_INT1_EN0:  PP0/IP3
 * CIU_INT2_EN0:  PP1/IP2
 * CIU_INT3_EN0:  PP1/IP3
 * CIU_INT4_EN0:  PP2/IP2
 * CIU_INT5_EN0:  PP2/IP3
 * CIU_INT6_EN0:  PP3/IP2
 * CIU_INT7_EN0:  PP3/IP3
 * - .....
 * (hole)
 * CIU_INT32_EN0: IO 0 (PEM0)
 * CIU_INT33_EN0: IO 1 (reserved in o70).
 */
union cvmx_ciu_intx_en0 {
	u64 u64;
	struct cvmx_ciu_intx_en0_s {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} s;
	struct cvmx_ciu_intx_en0_cn30xx {
		u64 reserved_59_63 : 5;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 reserved_47_47 : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn30xx;
	struct cvmx_ciu_intx_en0_cn31xx {
		u64 reserved_59_63 : 5;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn31xx;
	struct cvmx_ciu_intx_en0_cn38xx {
		u64 reserved_56_63 : 8;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn38xx;
	struct cvmx_ciu_intx_en0_cn38xx cn38xxp2;
	struct cvmx_ciu_intx_en0_cn30xx cn50xx;
	struct cvmx_ciu_intx_en0_cn52xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn52xx;
	struct cvmx_ciu_intx_en0_cn52xx cn52xxp1;
	struct cvmx_ciu_intx_en0_cn56xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn56xx;
	struct cvmx_ciu_intx_en0_cn56xx cn56xxp1;
	struct cvmx_ciu_intx_en0_cn38xx cn58xx;
	struct cvmx_ciu_intx_en0_cn38xx cn58xxp1;
	struct cvmx_ciu_intx_en0_cn61xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn61xx;
	struct cvmx_ciu_intx_en0_cn52xx cn63xx;
	struct cvmx_ciu_intx_en0_cn52xx cn63xxp1;
	struct cvmx_ciu_intx_en0_cn66xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 reserved_57_57 : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn66xx;
	struct cvmx_ciu_intx_en0_cn70xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 reserved_56_56 : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 reserved_46_47 : 2;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn70xx;
	struct cvmx_ciu_intx_en0_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en0_cnf71xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cnf71xx;
};

typedef union cvmx_ciu_intx_en0 cvmx_ciu_intx_en0_t;

/**
 * cvmx_ciu_int#_en0_w1c
 *
 * Write-1-to-clear version of the CIU_INTx_EN0 register, read back corresponding CIU_INTx_EN0
 * value.
 * CIU_INT33_EN0_W1C is reserved.
 */
union cvmx_ciu_intx_en0_w1c {
	u64 u64;
	struct cvmx_ciu_intx_en0_w1c_s {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} s;
	struct cvmx_ciu_intx_en0_w1c_cn52xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn52xx;
	struct cvmx_ciu_intx_en0_w1c_cn56xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn56xx;
	struct cvmx_ciu_intx_en0_w1c_cn58xx {
		u64 reserved_56_63 : 8;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn58xx;
	struct cvmx_ciu_intx_en0_w1c_cn61xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn61xx;
	struct cvmx_ciu_intx_en0_w1c_cn52xx cn63xx;
	struct cvmx_ciu_intx_en0_w1c_cn52xx cn63xxp1;
	struct cvmx_ciu_intx_en0_w1c_cn66xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 reserved_57_57 : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn66xx;
	struct cvmx_ciu_intx_en0_w1c_cn70xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 reserved_56_56 : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 reserved_46_47 : 2;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn70xx;
	struct cvmx_ciu_intx_en0_w1c_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en0_w1c_cnf71xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cnf71xx;
};

typedef union cvmx_ciu_intx_en0_w1c cvmx_ciu_intx_en0_w1c_t;

/**
 * cvmx_ciu_int#_en0_w1s
 *
 * Write-1-to-set version of the CIU_INTx_EN0 register, read back corresponding CIU_INTx_EN0
 * value.
 * CIU_INT33_EN0_W1S is reserved.
 */
union cvmx_ciu_intx_en0_w1s {
	u64 u64;
	struct cvmx_ciu_intx_en0_w1s_s {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} s;
	struct cvmx_ciu_intx_en0_w1s_cn52xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn52xx;
	struct cvmx_ciu_intx_en0_w1s_cn56xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn56xx;
	struct cvmx_ciu_intx_en0_w1s_cn58xx {
		u64 reserved_56_63 : 8;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn58xx;
	struct cvmx_ciu_intx_en0_w1s_cn61xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn61xx;
	struct cvmx_ciu_intx_en0_w1s_cn52xx cn63xx;
	struct cvmx_ciu_intx_en0_w1s_cn52xx cn63xxp1;
	struct cvmx_ciu_intx_en0_w1s_cn66xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 reserved_57_57 : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn66xx;
	struct cvmx_ciu_intx_en0_w1s_cn70xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 reserved_56_56 : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 reserved_46_47 : 2;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn70xx;
	struct cvmx_ciu_intx_en0_w1s_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en0_w1s_cnf71xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cnf71xx;
};

typedef union cvmx_ciu_intx_en0_w1s cvmx_ciu_intx_en0_w1s_t;

/**
 * cvmx_ciu_int#_en1
 *
 * Enables for CIU_SUM1_PPX_IPx  or CIU_SUM1_IOX_INT
 * CIU_INT0_EN1:  PP0/IP2
 * CIU_INT1_EN1:  PP0/IP3
 * CIU_INT2_EN1:  PP1/IP2
 * CIU_INT3_EN1:  PP1/IP3
 * CIU_INT4_EN1:  PP2/IP2
 * CIU_INT5_EN1:  PP2/IP3
 * CIU_INT6_EN1:  PP3/IP2
 * CIU_INT7_EN1:  PP3/IP3
 * - .....
 * (hole)
 * CIU_INT32_EN1: IO0 (PEM0)
 * CIU_INT33_EN1: IO1 (Reserved for o70)
 *
 * PPx/IP2 will be raised when...
 *
 * n = x*2
 * PPx/IP2 = |([CIU_SUM2_PPx_IP2,CIU_SUM1_PPx_IP2, CIU_INTn_SUM0] &
 * [CIU_EN2_PPx_IP2,CIU_INTn_EN1, CIU_INTn_EN0])
 *
 * PPx/IP3 will be raised when...
 *
 * n = x*2 + 1
 * PPx/IP3 =  |([CIU_SUM2_PPx_IP3,CIU_SUM1_PPx_IP3, CIU_INTn_SUM0] &
 * [CIU_EN2_PPx_IP3,CIU_INTn_EN1, CIU_INTn_EN0])
 *
 * PPx/IP4 will be raised when...
 * PPx/IP4 = |([CIU_SUM1_PPx_IP4, CIU_INTx_SUM4] & [CIU_INTx_EN4_1, CIU_INTx_EN4_0])
 *
 * PCI/INT will be raised when...
 *
 * PCI/INT0 (PEM0)
 * PCI/INT0 = |([CIU_SUM2_IO0_INT,CIU_SUM1_IO0_INT, CIU_INT32_SUM0] &
 * [CIU_EN2_IO0_INT,CIU_INT32_EN1, CIU_INT32_EN0])
 *
 * PCI/INT1 is reserved for o70.
 * PCI/INT1 = |([CIU_SUM2_IO1_INT,CIU_SUM1_IO1_INT, CIU_INT33_SUM0] &
 * [CIU_EN2_IO1_INT,CIU_INT33_EN1, CIU_INT33_EN0])
 */
union cvmx_ciu_intx_en1 {
	u64 u64;
	struct cvmx_ciu_intx_en1_s {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 srio1 : 1;
		u64 reserved_50_50 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 wdog : 16;
	} s;
	struct cvmx_ciu_intx_en1_cn30xx {
		u64 reserved_1_63 : 63;
		u64 wdog : 1;
	} cn30xx;
	struct cvmx_ciu_intx_en1_cn31xx {
		u64 reserved_2_63 : 62;
		u64 wdog : 2;
	} cn31xx;
	struct cvmx_ciu_intx_en1_cn38xx {
		u64 reserved_16_63 : 48;
		u64 wdog : 16;
	} cn38xx;
	struct cvmx_ciu_intx_en1_cn38xx cn38xxp2;
	struct cvmx_ciu_intx_en1_cn31xx cn50xx;
	struct cvmx_ciu_intx_en1_cn52xx {
		u64 reserved_20_63 : 44;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 reserved_4_15 : 12;
		u64 wdog : 4;
	} cn52xx;
	struct cvmx_ciu_intx_en1_cn52xxp1 {
		u64 reserved_19_63 : 45;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 reserved_4_15 : 12;
		u64 wdog : 4;
	} cn52xxp1;
	struct cvmx_ciu_intx_en1_cn56xx {
		u64 reserved_12_63 : 52;
		u64 wdog : 12;
	} cn56xx;
	struct cvmx_ciu_intx_en1_cn56xx cn56xxp1;
	struct cvmx_ciu_intx_en1_cn38xx cn58xx;
	struct cvmx_ciu_intx_en1_cn38xx cn58xxp1;
	struct cvmx_ciu_intx_en1_cn61xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_4_17 : 14;
		u64 wdog : 4;
	} cn61xx;
	struct cvmx_ciu_intx_en1_cn63xx {
		u64 rst : 1;
		u64 reserved_57_62 : 6;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 srio1 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_37_45 : 9;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_6_17 : 12;
		u64 wdog : 6;
	} cn63xx;
	struct cvmx_ciu_intx_en1_cn63xx cn63xxp1;
	struct cvmx_ciu_intx_en1_cn66xx {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_38_45 : 8;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_10_17 : 8;
		u64 wdog : 10;
	} cn66xx;
	struct cvmx_ciu_intx_en1_cn70xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 pem2 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_39_38 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_18_18 : 1;
		u64 usb1 : 1;
		u64 reserved_4_16 : 13;
		u64 wdog : 4;
	} cn70xx;
	struct cvmx_ciu_intx_en1_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en1_cnf71xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 reserved_41_46 : 6;
		u64 dpi_dma : 1;
		u64 reserved_37_39 : 3;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 reserved_32_32 : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_4_18 : 15;
		u64 wdog : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_intx_en1 cvmx_ciu_intx_en1_t;

/**
 * cvmx_ciu_int#_en1_w1c
 *
 * Write-1-to-clear version of the CIU_INTX_EN1 register, read back corresponding CIU_INTX_EN1
 * value.
 * CIU_INT33_EN1_W1C is reserved.
 */
union cvmx_ciu_intx_en1_w1c {
	u64 u64;
	struct cvmx_ciu_intx_en1_w1c_s {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 srio1 : 1;
		u64 reserved_50_50 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 wdog : 16;
	} s;
	struct cvmx_ciu_intx_en1_w1c_cn52xx {
		u64 reserved_20_63 : 44;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 reserved_4_15 : 12;
		u64 wdog : 4;
	} cn52xx;
	struct cvmx_ciu_intx_en1_w1c_cn56xx {
		u64 reserved_12_63 : 52;
		u64 wdog : 12;
	} cn56xx;
	struct cvmx_ciu_intx_en1_w1c_cn58xx {
		u64 reserved_16_63 : 48;
		u64 wdog : 16;
	} cn58xx;
	struct cvmx_ciu_intx_en1_w1c_cn61xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_4_17 : 14;
		u64 wdog : 4;
	} cn61xx;
	struct cvmx_ciu_intx_en1_w1c_cn63xx {
		u64 rst : 1;
		u64 reserved_57_62 : 6;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 srio1 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_37_45 : 9;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_6_17 : 12;
		u64 wdog : 6;
	} cn63xx;
	struct cvmx_ciu_intx_en1_w1c_cn63xx cn63xxp1;
	struct cvmx_ciu_intx_en1_w1c_cn66xx {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_38_45 : 8;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_10_17 : 8;
		u64 wdog : 10;
	} cn66xx;
	struct cvmx_ciu_intx_en1_w1c_cn70xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 pem2 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_18_18 : 1;
		u64 usb1 : 1;
		u64 reserved_4_16 : 13;
		u64 wdog : 4;
	} cn70xx;
	struct cvmx_ciu_intx_en1_w1c_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en1_w1c_cnf71xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 reserved_41_46 : 6;
		u64 dpi_dma : 1;
		u64 reserved_37_39 : 3;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 reserved_32_32 : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_4_18 : 15;
		u64 wdog : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_intx_en1_w1c cvmx_ciu_intx_en1_w1c_t;

/**
 * cvmx_ciu_int#_en1_w1s
 *
 * Write-1-to-set version of the CIU_INTX_EN1 register, read back corresponding CIU_INTX_EN1
 * value.
 * CIU_INT33_EN1_W1S is reserved.
 */
union cvmx_ciu_intx_en1_w1s {
	u64 u64;
	struct cvmx_ciu_intx_en1_w1s_s {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 srio1 : 1;
		u64 reserved_50_50 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 wdog : 16;
	} s;
	struct cvmx_ciu_intx_en1_w1s_cn52xx {
		u64 reserved_20_63 : 44;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 reserved_4_15 : 12;
		u64 wdog : 4;
	} cn52xx;
	struct cvmx_ciu_intx_en1_w1s_cn56xx {
		u64 reserved_12_63 : 52;
		u64 wdog : 12;
	} cn56xx;
	struct cvmx_ciu_intx_en1_w1s_cn58xx {
		u64 reserved_16_63 : 48;
		u64 wdog : 16;
	} cn58xx;
	struct cvmx_ciu_intx_en1_w1s_cn61xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_4_17 : 14;
		u64 wdog : 4;
	} cn61xx;
	struct cvmx_ciu_intx_en1_w1s_cn63xx {
		u64 rst : 1;
		u64 reserved_57_62 : 6;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 srio1 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_37_45 : 9;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_6_17 : 12;
		u64 wdog : 6;
	} cn63xx;
	struct cvmx_ciu_intx_en1_w1s_cn63xx cn63xxp1;
	struct cvmx_ciu_intx_en1_w1s_cn66xx {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_38_45 : 8;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_10_17 : 8;
		u64 wdog : 10;
	} cn66xx;
	struct cvmx_ciu_intx_en1_w1s_cn70xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 pem2 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_18_18 : 1;
		u64 usb1 : 1;
		u64 reserved_4_16 : 13;
		u64 wdog : 4;
	} cn70xx;
	struct cvmx_ciu_intx_en1_w1s_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en1_w1s_cnf71xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 reserved_41_46 : 6;
		u64 dpi_dma : 1;
		u64 reserved_37_39 : 3;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 reserved_32_32 : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_4_18 : 15;
		u64 wdog : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_intx_en1_w1s cvmx_ciu_intx_en1_w1s_t;

/**
 * cvmx_ciu_int#_en4_0
 *
 * CIU_INT0_EN4_0:   PP0  /IP4
 * CIU_INT1_EN4_0:   PP1  /IP4
 * - ...
 * CIU_INT3_EN4_0:   PP3  /IP4
 */
union cvmx_ciu_intx_en4_0 {
	u64 u64;
	struct cvmx_ciu_intx_en4_0_s {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} s;
	struct cvmx_ciu_intx_en4_0_cn50xx {
		u64 reserved_59_63 : 5;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 reserved_47_47 : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn50xx;
	struct cvmx_ciu_intx_en4_0_cn52xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn52xx;
	struct cvmx_ciu_intx_en4_0_cn52xx cn52xxp1;
	struct cvmx_ciu_intx_en4_0_cn56xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn56xx;
	struct cvmx_ciu_intx_en4_0_cn56xx cn56xxp1;
	struct cvmx_ciu_intx_en4_0_cn58xx {
		u64 reserved_56_63 : 8;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn58xx;
	struct cvmx_ciu_intx_en4_0_cn58xx cn58xxp1;
	struct cvmx_ciu_intx_en4_0_cn61xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn61xx;
	struct cvmx_ciu_intx_en4_0_cn52xx cn63xx;
	struct cvmx_ciu_intx_en4_0_cn52xx cn63xxp1;
	struct cvmx_ciu_intx_en4_0_cn66xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 reserved_57_57 : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn66xx;
	struct cvmx_ciu_intx_en4_0_cn70xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 reserved_56_56 : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 reserved_46_47 : 2;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn70xx;
	struct cvmx_ciu_intx_en4_0_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en4_0_cnf71xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cnf71xx;
};

typedef union cvmx_ciu_intx_en4_0 cvmx_ciu_intx_en4_0_t;

/**
 * cvmx_ciu_int#_en4_0_w1c
 *
 * Write-1-to-clear version of the CIU_INTx_EN4_0 register, read back corresponding
 * CIU_INTx_EN4_0 value.
 */
union cvmx_ciu_intx_en4_0_w1c {
	u64 u64;
	struct cvmx_ciu_intx_en4_0_w1c_s {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} s;
	struct cvmx_ciu_intx_en4_0_w1c_cn52xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn52xx;
	struct cvmx_ciu_intx_en4_0_w1c_cn56xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn56xx;
	struct cvmx_ciu_intx_en4_0_w1c_cn58xx {
		u64 reserved_56_63 : 8;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn58xx;
	struct cvmx_ciu_intx_en4_0_w1c_cn61xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn61xx;
	struct cvmx_ciu_intx_en4_0_w1c_cn52xx cn63xx;
	struct cvmx_ciu_intx_en4_0_w1c_cn52xx cn63xxp1;
	struct cvmx_ciu_intx_en4_0_w1c_cn66xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 reserved_57_57 : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn66xx;
	struct cvmx_ciu_intx_en4_0_w1c_cn70xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 reserved_56_56 : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 reserved_46_47 : 2;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn70xx;
	struct cvmx_ciu_intx_en4_0_w1c_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en4_0_w1c_cnf71xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cnf71xx;
};

typedef union cvmx_ciu_intx_en4_0_w1c cvmx_ciu_intx_en4_0_w1c_t;

/**
 * cvmx_ciu_int#_en4_0_w1s
 *
 * Write-1-to-set version of the CIU_INTX_EN4_0 register, read back corresponding CIU_INTX_EN4_0
 * value.
 */
union cvmx_ciu_intx_en4_0_w1s {
	u64 u64;
	struct cvmx_ciu_intx_en4_0_w1s_s {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} s;
	struct cvmx_ciu_intx_en4_0_w1s_cn52xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn52xx;
	struct cvmx_ciu_intx_en4_0_w1s_cn56xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn56xx;
	struct cvmx_ciu_intx_en4_0_w1s_cn58xx {
		u64 reserved_56_63 : 8;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn58xx;
	struct cvmx_ciu_intx_en4_0_w1s_cn61xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn61xx;
	struct cvmx_ciu_intx_en4_0_w1s_cn52xx cn63xx;
	struct cvmx_ciu_intx_en4_0_w1s_cn52xx cn63xxp1;
	struct cvmx_ciu_intx_en4_0_w1s_cn66xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 reserved_57_57 : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn66xx;
	struct cvmx_ciu_intx_en4_0_w1s_cn70xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 reserved_56_56 : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 reserved_46_47 : 2;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn70xx;
	struct cvmx_ciu_intx_en4_0_w1s_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en4_0_w1s_cnf71xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 reserved_44_44 : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cnf71xx;
};

typedef union cvmx_ciu_intx_en4_0_w1s cvmx_ciu_intx_en4_0_w1s_t;

/**
 * cvmx_ciu_int#_en4_1
 *
 * PPx/IP4 will be raised when...
 * PPx/IP4 = |([CIU_SUM1_PPx_IP4, CIU_INTx_SUM4] & [CIU_INTx_EN4_1, CIU_INTx_EN4_0])
 */
union cvmx_ciu_intx_en4_1 {
	u64 u64;
	struct cvmx_ciu_intx_en4_1_s {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 srio1 : 1;
		u64 reserved_50_50 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 wdog : 16;
	} s;
	struct cvmx_ciu_intx_en4_1_cn50xx {
		u64 reserved_2_63 : 62;
		u64 wdog : 2;
	} cn50xx;
	struct cvmx_ciu_intx_en4_1_cn52xx {
		u64 reserved_20_63 : 44;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 reserved_4_15 : 12;
		u64 wdog : 4;
	} cn52xx;
	struct cvmx_ciu_intx_en4_1_cn52xxp1 {
		u64 reserved_19_63 : 45;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 reserved_4_15 : 12;
		u64 wdog : 4;
	} cn52xxp1;
	struct cvmx_ciu_intx_en4_1_cn56xx {
		u64 reserved_12_63 : 52;
		u64 wdog : 12;
	} cn56xx;
	struct cvmx_ciu_intx_en4_1_cn56xx cn56xxp1;
	struct cvmx_ciu_intx_en4_1_cn58xx {
		u64 reserved_16_63 : 48;
		u64 wdog : 16;
	} cn58xx;
	struct cvmx_ciu_intx_en4_1_cn58xx cn58xxp1;
	struct cvmx_ciu_intx_en4_1_cn61xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_4_17 : 14;
		u64 wdog : 4;
	} cn61xx;
	struct cvmx_ciu_intx_en4_1_cn63xx {
		u64 rst : 1;
		u64 reserved_57_62 : 6;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 srio1 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_37_45 : 9;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_6_17 : 12;
		u64 wdog : 6;
	} cn63xx;
	struct cvmx_ciu_intx_en4_1_cn63xx cn63xxp1;
	struct cvmx_ciu_intx_en4_1_cn66xx {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_38_45 : 8;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_10_17 : 8;
		u64 wdog : 10;
	} cn66xx;
	struct cvmx_ciu_intx_en4_1_cn70xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 pem2 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_39_38 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_18_18 : 1;
		u64 usb1 : 1;
		u64 reserved_4_16 : 13;
		u64 wdog : 4;
	} cn70xx;
	struct cvmx_ciu_intx_en4_1_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en4_1_cnf71xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 reserved_41_46 : 6;
		u64 dpi_dma : 1;
		u64 reserved_37_39 : 3;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 reserved_32_32 : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_4_18 : 15;
		u64 wdog : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_intx_en4_1 cvmx_ciu_intx_en4_1_t;

/**
 * cvmx_ciu_int#_en4_1_w1c
 *
 * Write-1-to-clear version of the CIU_INTX_EN4_1 register, read back corresponding
 * CIU_INTX_EN4_1 value.
 */
union cvmx_ciu_intx_en4_1_w1c {
	u64 u64;
	struct cvmx_ciu_intx_en4_1_w1c_s {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 srio1 : 1;
		u64 reserved_50_50 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 wdog : 16;
	} s;
	struct cvmx_ciu_intx_en4_1_w1c_cn52xx {
		u64 reserved_20_63 : 44;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 reserved_4_15 : 12;
		u64 wdog : 4;
	} cn52xx;
	struct cvmx_ciu_intx_en4_1_w1c_cn56xx {
		u64 reserved_12_63 : 52;
		u64 wdog : 12;
	} cn56xx;
	struct cvmx_ciu_intx_en4_1_w1c_cn58xx {
		u64 reserved_16_63 : 48;
		u64 wdog : 16;
	} cn58xx;
	struct cvmx_ciu_intx_en4_1_w1c_cn61xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_4_17 : 14;
		u64 wdog : 4;
	} cn61xx;
	struct cvmx_ciu_intx_en4_1_w1c_cn63xx {
		u64 rst : 1;
		u64 reserved_57_62 : 6;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 srio1 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_37_45 : 9;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_6_17 : 12;
		u64 wdog : 6;
	} cn63xx;
	struct cvmx_ciu_intx_en4_1_w1c_cn63xx cn63xxp1;
	struct cvmx_ciu_intx_en4_1_w1c_cn66xx {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_38_45 : 8;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_10_17 : 8;
		u64 wdog : 10;
	} cn66xx;
	struct cvmx_ciu_intx_en4_1_w1c_cn70xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 pem2 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_18_18 : 1;
		u64 usb1 : 1;
		u64 reserved_4_16 : 13;
		u64 wdog : 4;
	} cn70xx;
	struct cvmx_ciu_intx_en4_1_w1c_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en4_1_w1c_cnf71xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 reserved_41_46 : 6;
		u64 dpi_dma : 1;
		u64 reserved_37_39 : 3;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 reserved_32_32 : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_4_18 : 15;
		u64 wdog : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_intx_en4_1_w1c cvmx_ciu_intx_en4_1_w1c_t;

/**
 * cvmx_ciu_int#_en4_1_w1s
 *
 * Write-1-to-set version of the CIU_INTX_EN4_1 register, read back corresponding CIU_INTX_EN4_1
 * value.
 */
union cvmx_ciu_intx_en4_1_w1s {
	u64 u64;
	struct cvmx_ciu_intx_en4_1_w1s_s {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 srio1 : 1;
		u64 reserved_50_50 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 wdog : 16;
	} s;
	struct cvmx_ciu_intx_en4_1_w1s_cn52xx {
		u64 reserved_20_63 : 44;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 reserved_4_15 : 12;
		u64 wdog : 4;
	} cn52xx;
	struct cvmx_ciu_intx_en4_1_w1s_cn56xx {
		u64 reserved_12_63 : 52;
		u64 wdog : 12;
	} cn56xx;
	struct cvmx_ciu_intx_en4_1_w1s_cn58xx {
		u64 reserved_16_63 : 48;
		u64 wdog : 16;
	} cn58xx;
	struct cvmx_ciu_intx_en4_1_w1s_cn61xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_4_17 : 14;
		u64 wdog : 4;
	} cn61xx;
	struct cvmx_ciu_intx_en4_1_w1s_cn63xx {
		u64 rst : 1;
		u64 reserved_57_62 : 6;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 srio1 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_37_45 : 9;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_6_17 : 12;
		u64 wdog : 6;
	} cn63xx;
	struct cvmx_ciu_intx_en4_1_w1s_cn63xx cn63xxp1;
	struct cvmx_ciu_intx_en4_1_w1s_cn66xx {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_38_45 : 8;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_10_17 : 8;
		u64 wdog : 10;
	} cn66xx;
	struct cvmx_ciu_intx_en4_1_w1s_cn70xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 pem2 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_18_18 : 1;
		u64 usb1 : 1;
		u64 reserved_4_16 : 13;
		u64 wdog : 4;
	} cn70xx;
	struct cvmx_ciu_intx_en4_1_w1s_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en4_1_w1s_cnf71xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 reserved_41_46 : 6;
		u64 dpi_dma : 1;
		u64 reserved_37_39 : 3;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 reserved_32_32 : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_4_18 : 15;
		u64 wdog : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_intx_en4_1_w1s cvmx_ciu_intx_en4_1_w1s_t;

/**
 * cvmx_ciu_int#_sum0
 *
 * The remaining IP4 summary bits will be CIU_INTX_SUM4.
 * CIU_INT0_SUM0:  PP0/IP2
 * CIU_INT1_SUM0:  PP0/IP3
 * CIU_INT2_SUM0:  PP1/IP2
 * CIU_INT3_SUM0:  PP1/IP3
 * CIU_INT4_SUM0:  PP2/IP2
 * CIU_INT5_SUM0:  PP2/IP3
 * CIU_INT6_SUM0:  PP3/IP2
 * CIU_INT7_SUM0:  PP3/IP3
 *  - .....
 * (hole)
 * CIU_INT32_SUM0: IO 0 (PEM0).
 * CIU_INT33_SUM0: IO 1 (Reserved in o70, in separate address group)
 */
union cvmx_ciu_intx_sum0 {
	u64 u64;
	struct cvmx_ciu_intx_sum0_s {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} s;
	struct cvmx_ciu_intx_sum0_cn30xx {
		u64 reserved_59_63 : 5;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 reserved_47_47 : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn30xx;
	struct cvmx_ciu_intx_sum0_cn31xx {
		u64 reserved_59_63 : 5;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn31xx;
	struct cvmx_ciu_intx_sum0_cn38xx {
		u64 reserved_56_63 : 8;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn38xx;
	struct cvmx_ciu_intx_sum0_cn38xx cn38xxp2;
	struct cvmx_ciu_intx_sum0_cn30xx cn50xx;
	struct cvmx_ciu_intx_sum0_cn52xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn52xx;
	struct cvmx_ciu_intx_sum0_cn52xx cn52xxp1;
	struct cvmx_ciu_intx_sum0_cn56xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn56xx;
	struct cvmx_ciu_intx_sum0_cn56xx cn56xxp1;
	struct cvmx_ciu_intx_sum0_cn38xx cn58xx;
	struct cvmx_ciu_intx_sum0_cn38xx cn58xxp1;
	struct cvmx_ciu_intx_sum0_cn61xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 sum2 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn61xx;
	struct cvmx_ciu_intx_sum0_cn52xx cn63xx;
	struct cvmx_ciu_intx_sum0_cn52xx cn63xxp1;
	struct cvmx_ciu_intx_sum0_cn66xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 reserved_57_57 : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 sum2 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn66xx;
	struct cvmx_ciu_intx_sum0_cn70xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 reserved_56_56 : 1;
		u64 timer : 4;
		u64 sum2 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 reserved_46_47 : 2;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn70xx;
	struct cvmx_ciu_intx_sum0_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_sum0_cnf71xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 sum2 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cnf71xx;
};

typedef union cvmx_ciu_intx_sum0 cvmx_ciu_intx_sum0_t;

/**
 * cvmx_ciu_int#_sum4
 *
 * CIU_INT0_SUM4:   PP0  /IP4
 * CIU_INT1_SUM4:   PP1  /IP4
 * - ...
 * CIU_INT3_SUM4:   PP3  /IP4
 */
union cvmx_ciu_intx_sum4 {
	u64 u64;
	struct cvmx_ciu_intx_sum4_s {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} s;
	struct cvmx_ciu_intx_sum4_cn50xx {
		u64 reserved_59_63 : 5;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 reserved_47_47 : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn50xx;
	struct cvmx_ciu_intx_sum4_cn52xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn52xx;
	struct cvmx_ciu_intx_sum4_cn52xx cn52xxp1;
	struct cvmx_ciu_intx_sum4_cn56xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn56xx;
	struct cvmx_ciu_intx_sum4_cn56xx cn56xxp1;
	struct cvmx_ciu_intx_sum4_cn58xx {
		u64 reserved_56_63 : 8;
		u64 timer : 4;
		u64 key_zero : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn58xx;
	struct cvmx_ciu_intx_sum4_cn58xx cn58xxp1;
	struct cvmx_ciu_intx_sum4_cn61xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 sum2 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn61xx;
	struct cvmx_ciu_intx_sum4_cn52xx cn63xx;
	struct cvmx_ciu_intx_sum4_cn52xx cn63xxp1;
	struct cvmx_ciu_intx_sum4_cn66xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 reserved_57_57 : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 sum2 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn66xx;
	struct cvmx_ciu_intx_sum4_cn70xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 reserved_56_56 : 1;
		u64 timer : 4;
		u64 sum2 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 reserved_46_47 : 2;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn70xx;
	struct cvmx_ciu_intx_sum4_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_sum4_cnf71xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 sum2 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cnf71xx;
};

typedef union cvmx_ciu_intx_sum4 cvmx_ciu_intx_sum4_t;

/**
 * cvmx_ciu_int33_sum0
 *
 * This bit is associated with CIU_INTX_SUM0. Reserved for o70 for future expansion.
 *
 */
union cvmx_ciu_int33_sum0 {
	u64 u64;
	struct cvmx_ciu_int33_sum0_s {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 sum2 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} s;
	struct cvmx_ciu_int33_sum0_s cn61xx;
	struct cvmx_ciu_int33_sum0_cn63xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 reserved_57_58 : 2;
		u64 usb : 1;
		u64 timer : 4;
		u64 reserved_51_51 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn63xx;
	struct cvmx_ciu_int33_sum0_cn63xx cn63xxp1;
	struct cvmx_ciu_int33_sum0_cn66xx {
		u64 bootdma : 1;
		u64 mii : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 reserved_57_57 : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 sum2 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn66xx;
	struct cvmx_ciu_int33_sum0_cn70xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 reserved_56_56 : 1;
		u64 timer : 4;
		u64 sum2 : 1;
		u64 ipd_drp : 1;
		u64 gmx_drp : 2;
		u64 reserved_47_46 : 2;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cn70xx;
	struct cvmx_ciu_int33_sum0_cn70xx cn70xxp1;
	struct cvmx_ciu_int33_sum0_cnf71xx {
		u64 bootdma : 1;
		u64 reserved_62_62 : 1;
		u64 ipdppthr : 1;
		u64 powiq : 1;
		u64 twsi2 : 1;
		u64 mpi : 1;
		u64 pcm : 1;
		u64 usb : 1;
		u64 timer : 4;
		u64 sum2 : 1;
		u64 ipd_drp : 1;
		u64 reserved_49_49 : 1;
		u64 gmx_drp : 1;
		u64 trace : 1;
		u64 rml : 1;
		u64 twsi : 1;
		u64 wdog_sum : 1;
		u64 pci_msi : 4;
		u64 pci_int : 4;
		u64 uart : 2;
		u64 mbox : 2;
		u64 gpio : 16;
		u64 workq : 16;
	} cnf71xx;
};

typedef union cvmx_ciu_int33_sum0 cvmx_ciu_int33_sum0_t;

/**
 * cvmx_ciu_int_dbg_sel
 */
union cvmx_ciu_int_dbg_sel {
	u64 u64;
	struct cvmx_ciu_int_dbg_sel_s {
		u64 reserved_19_63 : 45;
		u64 sel : 3;
		u64 reserved_10_15 : 6;
		u64 irq : 2;
		u64 reserved_5_7 : 3;
		u64 pp : 5;
	} s;
	struct cvmx_ciu_int_dbg_sel_cn61xx {
		u64 reserved_19_63 : 45;
		u64 sel : 3;
		u64 reserved_10_15 : 6;
		u64 irq : 2;
		u64 reserved_4_7 : 4;
		u64 pp : 4;
	} cn61xx;
	struct cvmx_ciu_int_dbg_sel_cn63xx {
		u64 reserved_19_63 : 45;
		u64 sel : 3;
		u64 reserved_10_15 : 6;
		u64 irq : 2;
		u64 reserved_3_7 : 5;
		u64 pp : 3;
	} cn63xx;
	struct cvmx_ciu_int_dbg_sel_cn61xx cn66xx;
	struct cvmx_ciu_int_dbg_sel_s cn68xx;
	struct cvmx_ciu_int_dbg_sel_s cn68xxp1;
	struct cvmx_ciu_int_dbg_sel_cn61xx cnf71xx;
};

typedef union cvmx_ciu_int_dbg_sel cvmx_ciu_int_dbg_sel_t;

/**
 * cvmx_ciu_int_sum1
 *
 * CIU_INT_SUM1 is kept to keep backward compatible.
 * Refer to CIU_SUM1_PPX_IPx which is the one should use.
 */
union cvmx_ciu_int_sum1 {
	u64 u64;
	struct cvmx_ciu_int_sum1_s {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 srio1 : 1;
		u64 reserved_50_50 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_38_45 : 8;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 wdog : 16;
	} s;
	struct cvmx_ciu_int_sum1_cn30xx {
		u64 reserved_1_63 : 63;
		u64 wdog : 1;
	} cn30xx;
	struct cvmx_ciu_int_sum1_cn31xx {
		u64 reserved_2_63 : 62;
		u64 wdog : 2;
	} cn31xx;
	struct cvmx_ciu_int_sum1_cn38xx {
		u64 reserved_16_63 : 48;
		u64 wdog : 16;
	} cn38xx;
	struct cvmx_ciu_int_sum1_cn38xx cn38xxp2;
	struct cvmx_ciu_int_sum1_cn31xx cn50xx;
	struct cvmx_ciu_int_sum1_cn52xx {
		u64 reserved_20_63 : 44;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 reserved_4_15 : 12;
		u64 wdog : 4;
	} cn52xx;
	struct cvmx_ciu_int_sum1_cn52xxp1 {
		u64 reserved_19_63 : 45;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 uart2 : 1;
		u64 reserved_4_15 : 12;
		u64 wdog : 4;
	} cn52xxp1;
	struct cvmx_ciu_int_sum1_cn56xx {
		u64 reserved_12_63 : 52;
		u64 wdog : 12;
	} cn56xx;
	struct cvmx_ciu_int_sum1_cn56xx cn56xxp1;
	struct cvmx_ciu_int_sum1_cn38xx cn58xx;
	struct cvmx_ciu_int_sum1_cn38xx cn58xxp1;
	struct cvmx_ciu_int_sum1_cn61xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_38_45 : 8;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_4_17 : 14;
		u64 wdog : 4;
	} cn61xx;
	struct cvmx_ciu_int_sum1_cn63xx {
		u64 rst : 1;
		u64 reserved_57_62 : 6;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 srio1 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_37_45 : 9;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_6_17 : 12;
		u64 wdog : 6;
	} cn63xx;
	struct cvmx_ciu_int_sum1_cn63xx cn63xxp1;
	struct cvmx_ciu_int_sum1_cn66xx {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_38_45 : 8;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_10_17 : 8;
		u64 wdog : 10;
	} cn66xx;
	struct cvmx_ciu_int_sum1_cn70xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 pem2 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_38_45 : 8;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_18_18 : 1;
		u64 usb1 : 1;
		u64 reserved_4_16 : 13;
		u64 wdog : 4;
	} cn70xx;
	struct cvmx_ciu_int_sum1_cn70xx cn70xxp1;
	struct cvmx_ciu_int_sum1_cnf71xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 reserved_37_46 : 10;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 reserved_32_32 : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_4_18 : 15;
		u64 wdog : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_int_sum1 cvmx_ciu_int_sum1_t;

/**
 * cvmx_ciu_intr_slowdown
 */
union cvmx_ciu_intr_slowdown {
	u64 u64;
	struct cvmx_ciu_intr_slowdown_s {
		u64 reserved_3_63 : 61;
		u64 ctl : 3;
	} s;
	struct cvmx_ciu_intr_slowdown_s cn70xx;
	struct cvmx_ciu_intr_slowdown_s cn70xxp1;
};

typedef union cvmx_ciu_intr_slowdown cvmx_ciu_intr_slowdown_t;

/**
 * cvmx_ciu_mbox_clr#
 */
union cvmx_ciu_mbox_clrx {
	u64 u64;
	struct cvmx_ciu_mbox_clrx_s {
		u64 reserved_32_63 : 32;
		u64 bits : 32;
	} s;
	struct cvmx_ciu_mbox_clrx_s cn30xx;
	struct cvmx_ciu_mbox_clrx_s cn31xx;
	struct cvmx_ciu_mbox_clrx_s cn38xx;
	struct cvmx_ciu_mbox_clrx_s cn38xxp2;
	struct cvmx_ciu_mbox_clrx_s cn50xx;
	struct cvmx_ciu_mbox_clrx_s cn52xx;
	struct cvmx_ciu_mbox_clrx_s cn52xxp1;
	struct cvmx_ciu_mbox_clrx_s cn56xx;
	struct cvmx_ciu_mbox_clrx_s cn56xxp1;
	struct cvmx_ciu_mbox_clrx_s cn58xx;
	struct cvmx_ciu_mbox_clrx_s cn58xxp1;
	struct cvmx_ciu_mbox_clrx_s cn61xx;
	struct cvmx_ciu_mbox_clrx_s cn63xx;
	struct cvmx_ciu_mbox_clrx_s cn63xxp1;
	struct cvmx_ciu_mbox_clrx_s cn66xx;
	struct cvmx_ciu_mbox_clrx_s cn68xx;
	struct cvmx_ciu_mbox_clrx_s cn68xxp1;
	struct cvmx_ciu_mbox_clrx_s cn70xx;
	struct cvmx_ciu_mbox_clrx_s cn70xxp1;
	struct cvmx_ciu_mbox_clrx_s cnf71xx;
};

typedef union cvmx_ciu_mbox_clrx cvmx_ciu_mbox_clrx_t;

/**
 * cvmx_ciu_mbox_set#
 */
union cvmx_ciu_mbox_setx {
	u64 u64;
	struct cvmx_ciu_mbox_setx_s {
		u64 reserved_32_63 : 32;
		u64 bits : 32;
	} s;
	struct cvmx_ciu_mbox_setx_s cn30xx;
	struct cvmx_ciu_mbox_setx_s cn31xx;
	struct cvmx_ciu_mbox_setx_s cn38xx;
	struct cvmx_ciu_mbox_setx_s cn38xxp2;
	struct cvmx_ciu_mbox_setx_s cn50xx;
	struct cvmx_ciu_mbox_setx_s cn52xx;
	struct cvmx_ciu_mbox_setx_s cn52xxp1;
	struct cvmx_ciu_mbox_setx_s cn56xx;
	struct cvmx_ciu_mbox_setx_s cn56xxp1;
	struct cvmx_ciu_mbox_setx_s cn58xx;
	struct cvmx_ciu_mbox_setx_s cn58xxp1;
	struct cvmx_ciu_mbox_setx_s cn61xx;
	struct cvmx_ciu_mbox_setx_s cn63xx;
	struct cvmx_ciu_mbox_setx_s cn63xxp1;
	struct cvmx_ciu_mbox_setx_s cn66xx;
	struct cvmx_ciu_mbox_setx_s cn68xx;
	struct cvmx_ciu_mbox_setx_s cn68xxp1;
	struct cvmx_ciu_mbox_setx_s cn70xx;
	struct cvmx_ciu_mbox_setx_s cn70xxp1;
	struct cvmx_ciu_mbox_setx_s cnf71xx;
};

typedef union cvmx_ciu_mbox_setx cvmx_ciu_mbox_setx_t;

/**
 * cvmx_ciu_nmi
 */
union cvmx_ciu_nmi {
	u64 u64;
	struct cvmx_ciu_nmi_s {
		u64 reserved_32_63 : 32;
		u64 nmi : 32;
	} s;
	struct cvmx_ciu_nmi_cn30xx {
		u64 reserved_1_63 : 63;
		u64 nmi : 1;
	} cn30xx;
	struct cvmx_ciu_nmi_cn31xx {
		u64 reserved_2_63 : 62;
		u64 nmi : 2;
	} cn31xx;
	struct cvmx_ciu_nmi_cn38xx {
		u64 reserved_16_63 : 48;
		u64 nmi : 16;
	} cn38xx;
	struct cvmx_ciu_nmi_cn38xx cn38xxp2;
	struct cvmx_ciu_nmi_cn31xx cn50xx;
	struct cvmx_ciu_nmi_cn52xx {
		u64 reserved_4_63 : 60;
		u64 nmi : 4;
	} cn52xx;
	struct cvmx_ciu_nmi_cn52xx cn52xxp1;
	struct cvmx_ciu_nmi_cn56xx {
		u64 reserved_12_63 : 52;
		u64 nmi : 12;
	} cn56xx;
	struct cvmx_ciu_nmi_cn56xx cn56xxp1;
	struct cvmx_ciu_nmi_cn38xx cn58xx;
	struct cvmx_ciu_nmi_cn38xx cn58xxp1;
	struct cvmx_ciu_nmi_cn52xx cn61xx;
	struct cvmx_ciu_nmi_cn63xx {
		u64 reserved_6_63 : 58;
		u64 nmi : 6;
	} cn63xx;
	struct cvmx_ciu_nmi_cn63xx cn63xxp1;
	struct cvmx_ciu_nmi_cn66xx {
		u64 reserved_10_63 : 54;
		u64 nmi : 10;
	} cn66xx;
	struct cvmx_ciu_nmi_s cn68xx;
	struct cvmx_ciu_nmi_s cn68xxp1;
	struct cvmx_ciu_nmi_cn52xx cn70xx;
	struct cvmx_ciu_nmi_cn52xx cn70xxp1;
	struct cvmx_ciu_nmi_cn52xx cnf71xx;
};

typedef union cvmx_ciu_nmi cvmx_ciu_nmi_t;

/**
 * cvmx_ciu_pci_inta
 */
union cvmx_ciu_pci_inta {
	u64 u64;
	struct cvmx_ciu_pci_inta_s {
		u64 reserved_2_63 : 62;
		u64 intr : 2;
	} s;
	struct cvmx_ciu_pci_inta_s cn30xx;
	struct cvmx_ciu_pci_inta_s cn31xx;
	struct cvmx_ciu_pci_inta_s cn38xx;
	struct cvmx_ciu_pci_inta_s cn38xxp2;
	struct cvmx_ciu_pci_inta_s cn50xx;
	struct cvmx_ciu_pci_inta_s cn52xx;
	struct cvmx_ciu_pci_inta_s cn52xxp1;
	struct cvmx_ciu_pci_inta_s cn56xx;
	struct cvmx_ciu_pci_inta_s cn56xxp1;
	struct cvmx_ciu_pci_inta_s cn58xx;
	struct cvmx_ciu_pci_inta_s cn58xxp1;
	struct cvmx_ciu_pci_inta_s cn61xx;
	struct cvmx_ciu_pci_inta_s cn63xx;
	struct cvmx_ciu_pci_inta_s cn63xxp1;
	struct cvmx_ciu_pci_inta_s cn66xx;
	struct cvmx_ciu_pci_inta_s cn68xx;
	struct cvmx_ciu_pci_inta_s cn68xxp1;
	struct cvmx_ciu_pci_inta_s cn70xx;
	struct cvmx_ciu_pci_inta_s cn70xxp1;
	struct cvmx_ciu_pci_inta_s cnf71xx;
};

typedef union cvmx_ciu_pci_inta cvmx_ciu_pci_inta_t;

/**
 * cvmx_ciu_pp_bist_stat
 */
union cvmx_ciu_pp_bist_stat {
	u64 u64;
	struct cvmx_ciu_pp_bist_stat_s {
		u64 reserved_32_63 : 32;
		u64 pp_bist : 32;
	} s;
	struct cvmx_ciu_pp_bist_stat_s cn68xx;
	struct cvmx_ciu_pp_bist_stat_s cn68xxp1;
};

typedef union cvmx_ciu_pp_bist_stat cvmx_ciu_pp_bist_stat_t;

/**
 * cvmx_ciu_pp_dbg
 */
union cvmx_ciu_pp_dbg {
	u64 u64;
	struct cvmx_ciu_pp_dbg_s {
		u64 reserved_48_63 : 16;
		u64 ppdbg : 48;
	} s;
	struct cvmx_ciu_pp_dbg_cn30xx {
		u64 reserved_1_63 : 63;
		u64 ppdbg : 1;
	} cn30xx;
	struct cvmx_ciu_pp_dbg_cn31xx {
		u64 reserved_2_63 : 62;
		u64 ppdbg : 2;
	} cn31xx;
	struct cvmx_ciu_pp_dbg_cn38xx {
		u64 reserved_16_63 : 48;
		u64 ppdbg : 16;
	} cn38xx;
	struct cvmx_ciu_pp_dbg_cn38xx cn38xxp2;
	struct cvmx_ciu_pp_dbg_cn31xx cn50xx;
	struct cvmx_ciu_pp_dbg_cn52xx {
		u64 reserved_4_63 : 60;
		u64 ppdbg : 4;
	} cn52xx;
	struct cvmx_ciu_pp_dbg_cn52xx cn52xxp1;
	struct cvmx_ciu_pp_dbg_cn56xx {
		u64 reserved_12_63 : 52;
		u64 ppdbg : 12;
	} cn56xx;
	struct cvmx_ciu_pp_dbg_cn56xx cn56xxp1;
	struct cvmx_ciu_pp_dbg_cn38xx cn58xx;
	struct cvmx_ciu_pp_dbg_cn38xx cn58xxp1;
	struct cvmx_ciu_pp_dbg_cn52xx cn61xx;
	struct cvmx_ciu_pp_dbg_cn63xx {
		u64 reserved_6_63 : 58;
		u64 ppdbg : 6;
	} cn63xx;
	struct cvmx_ciu_pp_dbg_cn63xx cn63xxp1;
	struct cvmx_ciu_pp_dbg_cn66xx {
		u64 reserved_10_63 : 54;
		u64 ppdbg : 10;
	} cn66xx;
	struct cvmx_ciu_pp_dbg_cn68xx {
		u64 reserved_32_63 : 32;
		u64 ppdbg : 32;
	} cn68xx;
	struct cvmx_ciu_pp_dbg_cn68xx cn68xxp1;
	struct cvmx_ciu_pp_dbg_cn52xx cn70xx;
	struct cvmx_ciu_pp_dbg_cn52xx cn70xxp1;
	struct cvmx_ciu_pp_dbg_cn38xx cn73xx;
	struct cvmx_ciu_pp_dbg_s cn78xx;
	struct cvmx_ciu_pp_dbg_s cn78xxp1;
	struct cvmx_ciu_pp_dbg_cn52xx cnf71xx;
	struct cvmx_ciu_pp_dbg_cn38xx cnf75xx;
};

typedef union cvmx_ciu_pp_dbg cvmx_ciu_pp_dbg_t;

/**
 * cvmx_ciu_pp_poke#
 *
 * CIU_PP_POKE for CIU_WDOG
 *
 */
union cvmx_ciu_pp_pokex {
	u64 u64;
	struct cvmx_ciu_pp_pokex_s {
		u64 poke : 64;
	} s;
	struct cvmx_ciu_pp_pokex_s cn30xx;
	struct cvmx_ciu_pp_pokex_s cn31xx;
	struct cvmx_ciu_pp_pokex_s cn38xx;
	struct cvmx_ciu_pp_pokex_s cn38xxp2;
	struct cvmx_ciu_pp_pokex_s cn50xx;
	struct cvmx_ciu_pp_pokex_s cn52xx;
	struct cvmx_ciu_pp_pokex_s cn52xxp1;
	struct cvmx_ciu_pp_pokex_s cn56xx;
	struct cvmx_ciu_pp_pokex_s cn56xxp1;
	struct cvmx_ciu_pp_pokex_s cn58xx;
	struct cvmx_ciu_pp_pokex_s cn58xxp1;
	struct cvmx_ciu_pp_pokex_s cn61xx;
	struct cvmx_ciu_pp_pokex_s cn63xx;
	struct cvmx_ciu_pp_pokex_s cn63xxp1;
	struct cvmx_ciu_pp_pokex_s cn66xx;
	struct cvmx_ciu_pp_pokex_s cn68xx;
	struct cvmx_ciu_pp_pokex_s cn68xxp1;
	struct cvmx_ciu_pp_pokex_s cn70xx;
	struct cvmx_ciu_pp_pokex_s cn70xxp1;
	struct cvmx_ciu_pp_pokex_cn73xx {
		u64 reserved_1_63 : 63;
		u64 poke : 1;
	} cn73xx;
	struct cvmx_ciu_pp_pokex_cn73xx cn78xx;
	struct cvmx_ciu_pp_pokex_cn73xx cn78xxp1;
	struct cvmx_ciu_pp_pokex_s cnf71xx;
	struct cvmx_ciu_pp_pokex_cn73xx cnf75xx;
};

typedef union cvmx_ciu_pp_pokex cvmx_ciu_pp_pokex_t;

/**
 * cvmx_ciu_pp_rst
 *
 * This register contains the reset control for each core. A 1 holds a core in reset, 0 release
 * from reset. It resets to all ones when REMOTE_BOOT is enabled or all ones excluding bit 0 when
 * REMOTE_BOOT is disabled. Writes to this register should occur only if the CIU_PP_RST_PENDING
 * register is cleared.
 */
union cvmx_ciu_pp_rst {
	u64 u64;
	struct cvmx_ciu_pp_rst_s {
		u64 reserved_48_63 : 16;
		u64 rst : 47;
		u64 rst0 : 1;
	} s;
	struct cvmx_ciu_pp_rst_cn30xx {
		u64 reserved_1_63 : 63;
		u64 rst0 : 1;
	} cn30xx;
	struct cvmx_ciu_pp_rst_cn31xx {
		u64 reserved_2_63 : 62;
		u64 rst : 1;
		u64 rst0 : 1;
	} cn31xx;
	struct cvmx_ciu_pp_rst_cn38xx {
		u64 reserved_16_63 : 48;
		u64 rst : 15;
		u64 rst0 : 1;
	} cn38xx;
	struct cvmx_ciu_pp_rst_cn38xx cn38xxp2;
	struct cvmx_ciu_pp_rst_cn31xx cn50xx;
	struct cvmx_ciu_pp_rst_cn52xx {
		u64 reserved_4_63 : 60;
		u64 rst : 3;
		u64 rst0 : 1;
	} cn52xx;
	struct cvmx_ciu_pp_rst_cn52xx cn52xxp1;
	struct cvmx_ciu_pp_rst_cn56xx {
		u64 reserved_12_63 : 52;
		u64 rst : 11;
		u64 rst0 : 1;
	} cn56xx;
	struct cvmx_ciu_pp_rst_cn56xx cn56xxp1;
	struct cvmx_ciu_pp_rst_cn38xx cn58xx;
	struct cvmx_ciu_pp_rst_cn38xx cn58xxp1;
	struct cvmx_ciu_pp_rst_cn52xx cn61xx;
	struct cvmx_ciu_pp_rst_cn63xx {
		u64 reserved_6_63 : 58;
		u64 rst : 5;
		u64 rst0 : 1;
	} cn63xx;
	struct cvmx_ciu_pp_rst_cn63xx cn63xxp1;
	struct cvmx_ciu_pp_rst_cn66xx {
		u64 reserved_10_63 : 54;
		u64 rst : 9;
		u64 rst0 : 1;
	} cn66xx;
	struct cvmx_ciu_pp_rst_cn68xx {
		u64 reserved_32_63 : 32;
		u64 rst : 31;
		u64 rst0 : 1;
	} cn68xx;
	struct cvmx_ciu_pp_rst_cn68xx cn68xxp1;
	struct cvmx_ciu_pp_rst_cn52xx cn70xx;
	struct cvmx_ciu_pp_rst_cn52xx cn70xxp1;
	struct cvmx_ciu_pp_rst_cn38xx cn73xx;
	struct cvmx_ciu_pp_rst_s cn78xx;
	struct cvmx_ciu_pp_rst_s cn78xxp1;
	struct cvmx_ciu_pp_rst_cn52xx cnf71xx;
	struct cvmx_ciu_pp_rst_cn38xx cnf75xx;
};

typedef union cvmx_ciu_pp_rst cvmx_ciu_pp_rst_t;

/**
 * cvmx_ciu_pp_rst_pending
 *
 * This register contains the reset status for each core.
 *
 */
union cvmx_ciu_pp_rst_pending {
	u64 u64;
	struct cvmx_ciu_pp_rst_pending_s {
		u64 reserved_48_63 : 16;
		u64 pend : 48;
	} s;
	struct cvmx_ciu_pp_rst_pending_s cn70xx;
	struct cvmx_ciu_pp_rst_pending_s cn70xxp1;
	struct cvmx_ciu_pp_rst_pending_cn73xx {
		u64 reserved_16_63 : 48;
		u64 pend : 16;
	} cn73xx;
	struct cvmx_ciu_pp_rst_pending_s cn78xx;
	struct cvmx_ciu_pp_rst_pending_s cn78xxp1;
	struct cvmx_ciu_pp_rst_pending_cn73xx cnf75xx;
};

typedef union cvmx_ciu_pp_rst_pending cvmx_ciu_pp_rst_pending_t;

/**
 * cvmx_ciu_qlm0
 *
 * Notes:
 * This register is only reset by cold reset.
 *
 */
union cvmx_ciu_qlm0 {
	u64 u64;
	struct cvmx_ciu_qlm0_s {
		u64 g2bypass : 1;
		u64 reserved_53_62 : 10;
		u64 g2deemph : 5;
		u64 reserved_45_47 : 3;
		u64 g2margin : 5;
		u64 reserved_32_39 : 8;
		u64 txbypass : 1;
		u64 reserved_21_30 : 10;
		u64 txdeemph : 5;
		u64 reserved_13_15 : 3;
		u64 txmargin : 5;
		u64 reserved_4_7 : 4;
		u64 lane_en : 4;
	} s;
	struct cvmx_ciu_qlm0_s cn61xx;
	struct cvmx_ciu_qlm0_s cn63xx;
	struct cvmx_ciu_qlm0_cn63xxp1 {
		u64 reserved_32_63 : 32;
		u64 txbypass : 1;
		u64 reserved_20_30 : 11;
		u64 txdeemph : 4;
		u64 reserved_13_15 : 3;
		u64 txmargin : 5;
		u64 reserved_4_7 : 4;
		u64 lane_en : 4;
	} cn63xxp1;
	struct cvmx_ciu_qlm0_s cn66xx;
	struct cvmx_ciu_qlm0_cn68xx {
		u64 reserved_32_63 : 32;
		u64 txbypass : 1;
		u64 reserved_21_30 : 10;
		u64 txdeemph : 5;
		u64 reserved_13_15 : 3;
		u64 txmargin : 5;
		u64 reserved_4_7 : 4;
		u64 lane_en : 4;
	} cn68xx;
	struct cvmx_ciu_qlm0_cn68xx cn68xxp1;
	struct cvmx_ciu_qlm0_s cnf71xx;
};

typedef union cvmx_ciu_qlm0 cvmx_ciu_qlm0_t;

/**
 * cvmx_ciu_qlm1
 *
 * Notes:
 * This register is only reset by cold reset.
 *
 */
union cvmx_ciu_qlm1 {
	u64 u64;
	struct cvmx_ciu_qlm1_s {
		u64 g2bypass : 1;
		u64 reserved_53_62 : 10;
		u64 g2deemph : 5;
		u64 reserved_45_47 : 3;
		u64 g2margin : 5;
		u64 reserved_32_39 : 8;
		u64 txbypass : 1;
		u64 reserved_21_30 : 10;
		u64 txdeemph : 5;
		u64 reserved_13_15 : 3;
		u64 txmargin : 5;
		u64 reserved_4_7 : 4;
		u64 lane_en : 4;
	} s;
	struct cvmx_ciu_qlm1_s cn61xx;
	struct cvmx_ciu_qlm1_s cn63xx;
	struct cvmx_ciu_qlm1_cn63xxp1 {
		u64 reserved_32_63 : 32;
		u64 txbypass : 1;
		u64 reserved_20_30 : 11;
		u64 txdeemph : 4;
		u64 reserved_13_15 : 3;
		u64 txmargin : 5;
		u64 reserved_4_7 : 4;
		u64 lane_en : 4;
	} cn63xxp1;
	struct cvmx_ciu_qlm1_s cn66xx;
	struct cvmx_ciu_qlm1_s cn68xx;
	struct cvmx_ciu_qlm1_s cn68xxp1;
	struct cvmx_ciu_qlm1_s cnf71xx;
};

typedef union cvmx_ciu_qlm1 cvmx_ciu_qlm1_t;

/**
 * cvmx_ciu_qlm2
 *
 * Notes:
 * This register is only reset by cold reset.
 *
 */
union cvmx_ciu_qlm2 {
	u64 u64;
	struct cvmx_ciu_qlm2_s {
		u64 g2bypass : 1;
		u64 reserved_53_62 : 10;
		u64 g2deemph : 5;
		u64 reserved_45_47 : 3;
		u64 g2margin : 5;
		u64 reserved_32_39 : 8;
		u64 txbypass : 1;
		u64 reserved_21_30 : 10;
		u64 txdeemph : 5;
		u64 reserved_13_15 : 3;
		u64 txmargin : 5;
		u64 reserved_4_7 : 4;
		u64 lane_en : 4;
	} s;
	struct cvmx_ciu_qlm2_cn61xx {
		u64 reserved_32_63 : 32;
		u64 txbypass : 1;
		u64 reserved_21_30 : 10;
		u64 txdeemph : 5;
		u64 reserved_13_15 : 3;
		u64 txmargin : 5;
		u64 reserved_4_7 : 4;
		u64 lane_en : 4;
	} cn61xx;
	struct cvmx_ciu_qlm2_cn61xx cn63xx;
	struct cvmx_ciu_qlm2_cn63xxp1 {
		u64 reserved_32_63 : 32;
		u64 txbypass : 1;
		u64 reserved_20_30 : 11;
		u64 txdeemph : 4;
		u64 reserved_13_15 : 3;
		u64 txmargin : 5;
		u64 reserved_4_7 : 4;
		u64 lane_en : 4;
	} cn63xxp1;
	struct cvmx_ciu_qlm2_cn61xx cn66xx;
	struct cvmx_ciu_qlm2_s cn68xx;
	struct cvmx_ciu_qlm2_s cn68xxp1;
	struct cvmx_ciu_qlm2_cn61xx cnf71xx;
};

typedef union cvmx_ciu_qlm2 cvmx_ciu_qlm2_t;

/**
 * cvmx_ciu_qlm3
 *
 * Notes:
 * This register is only reset by cold reset.
 *
 */
union cvmx_ciu_qlm3 {
	u64 u64;
	struct cvmx_ciu_qlm3_s {
		u64 g2bypass : 1;
		u64 reserved_53_62 : 10;
		u64 g2deemph : 5;
		u64 reserved_45_47 : 3;
		u64 g2margin : 5;
		u64 reserved_32_39 : 8;
		u64 txbypass : 1;
		u64 reserved_21_30 : 10;
		u64 txdeemph : 5;
		u64 reserved_13_15 : 3;
		u64 txmargin : 5;
		u64 reserved_4_7 : 4;
		u64 lane_en : 4;
	} s;
	struct cvmx_ciu_qlm3_s cn68xx;
	struct cvmx_ciu_qlm3_s cn68xxp1;
};

typedef union cvmx_ciu_qlm3 cvmx_ciu_qlm3_t;

/**
 * cvmx_ciu_qlm4
 *
 * Notes:
 * This register is only reset by cold reset.
 *
 */
union cvmx_ciu_qlm4 {
	u64 u64;
	struct cvmx_ciu_qlm4_s {
		u64 g2bypass : 1;
		u64 reserved_53_62 : 10;
		u64 g2deemph : 5;
		u64 reserved_45_47 : 3;
		u64 g2margin : 5;
		u64 reserved_32_39 : 8;
		u64 txbypass : 1;
		u64 reserved_21_30 : 10;
		u64 txdeemph : 5;
		u64 reserved_13_15 : 3;
		u64 txmargin : 5;
		u64 reserved_4_7 : 4;
		u64 lane_en : 4;
	} s;
	struct cvmx_ciu_qlm4_s cn68xx;
	struct cvmx_ciu_qlm4_s cn68xxp1;
};

typedef union cvmx_ciu_qlm4 cvmx_ciu_qlm4_t;

/**
 * cvmx_ciu_qlm_dcok
 */
union cvmx_ciu_qlm_dcok {
	u64 u64;
	struct cvmx_ciu_qlm_dcok_s {
		u64 reserved_4_63 : 60;
		u64 qlm_dcok : 4;
	} s;
	struct cvmx_ciu_qlm_dcok_cn52xx {
		u64 reserved_2_63 : 62;
		u64 qlm_dcok : 2;
	} cn52xx;
	struct cvmx_ciu_qlm_dcok_cn52xx cn52xxp1;
	struct cvmx_ciu_qlm_dcok_s cn56xx;
	struct cvmx_ciu_qlm_dcok_s cn56xxp1;
};

typedef union cvmx_ciu_qlm_dcok cvmx_ciu_qlm_dcok_t;

/**
 * cvmx_ciu_qlm_jtgc
 */
union cvmx_ciu_qlm_jtgc {
	u64 u64;
	struct cvmx_ciu_qlm_jtgc_s {
		u64 reserved_17_63 : 47;
		u64 bypass_ext : 1;
		u64 reserved_11_15 : 5;
		u64 clk_div : 3;
		u64 reserved_7_7 : 1;
		u64 mux_sel : 3;
		u64 bypass : 4;
	} s;
	struct cvmx_ciu_qlm_jtgc_cn52xx {
		u64 reserved_11_63 : 53;
		u64 clk_div : 3;
		u64 reserved_5_7 : 3;
		u64 mux_sel : 1;
		u64 reserved_2_3 : 2;
		u64 bypass : 2;
	} cn52xx;
	struct cvmx_ciu_qlm_jtgc_cn52xx cn52xxp1;
	struct cvmx_ciu_qlm_jtgc_cn56xx {
		u64 reserved_11_63 : 53;
		u64 clk_div : 3;
		u64 reserved_6_7 : 2;
		u64 mux_sel : 2;
		u64 bypass : 4;
	} cn56xx;
	struct cvmx_ciu_qlm_jtgc_cn56xx cn56xxp1;
	struct cvmx_ciu_qlm_jtgc_cn61xx {
		u64 reserved_11_63 : 53;
		u64 clk_div : 3;
		u64 reserved_6_7 : 2;
		u64 mux_sel : 2;
		u64 reserved_3_3 : 1;
		u64 bypass : 3;
	} cn61xx;
	struct cvmx_ciu_qlm_jtgc_cn61xx cn63xx;
	struct cvmx_ciu_qlm_jtgc_cn61xx cn63xxp1;
	struct cvmx_ciu_qlm_jtgc_cn61xx cn66xx;
	struct cvmx_ciu_qlm_jtgc_s cn68xx;
	struct cvmx_ciu_qlm_jtgc_s cn68xxp1;
	struct cvmx_ciu_qlm_jtgc_cn61xx cnf71xx;
};

typedef union cvmx_ciu_qlm_jtgc cvmx_ciu_qlm_jtgc_t;

/**
 * cvmx_ciu_qlm_jtgd
 */
union cvmx_ciu_qlm_jtgd {
	u64 u64;
	struct cvmx_ciu_qlm_jtgd_s {
		u64 capture : 1;
		u64 shift : 1;
		u64 update : 1;
		u64 reserved_45_60 : 16;
		u64 select : 5;
		u64 reserved_37_39 : 3;
		u64 shft_cnt : 5;
		u64 shft_reg : 32;
	} s;
	struct cvmx_ciu_qlm_jtgd_cn52xx {
		u64 capture : 1;
		u64 shift : 1;
		u64 update : 1;
		u64 reserved_42_60 : 19;
		u64 select : 2;
		u64 reserved_37_39 : 3;
		u64 shft_cnt : 5;
		u64 shft_reg : 32;
	} cn52xx;
	struct cvmx_ciu_qlm_jtgd_cn52xx cn52xxp1;
	struct cvmx_ciu_qlm_jtgd_cn56xx {
		u64 capture : 1;
		u64 shift : 1;
		u64 update : 1;
		u64 reserved_44_60 : 17;
		u64 select : 4;
		u64 reserved_37_39 : 3;
		u64 shft_cnt : 5;
		u64 shft_reg : 32;
	} cn56xx;
	struct cvmx_ciu_qlm_jtgd_cn56xxp1 {
		u64 capture : 1;
		u64 shift : 1;
		u64 update : 1;
		u64 reserved_37_60 : 24;
		u64 shft_cnt : 5;
		u64 shft_reg : 32;
	} cn56xxp1;
	struct cvmx_ciu_qlm_jtgd_cn61xx {
		u64 capture : 1;
		u64 shift : 1;
		u64 update : 1;
		u64 reserved_43_60 : 18;
		u64 select : 3;
		u64 reserved_37_39 : 3;
		u64 shft_cnt : 5;
		u64 shft_reg : 32;
	} cn61xx;
	struct cvmx_ciu_qlm_jtgd_cn61xx cn63xx;
	struct cvmx_ciu_qlm_jtgd_cn61xx cn63xxp1;
	struct cvmx_ciu_qlm_jtgd_cn61xx cn66xx;
	struct cvmx_ciu_qlm_jtgd_s cn68xx;
	struct cvmx_ciu_qlm_jtgd_s cn68xxp1;
	struct cvmx_ciu_qlm_jtgd_cn61xx cnf71xx;
};

typedef union cvmx_ciu_qlm_jtgd cvmx_ciu_qlm_jtgd_t;

/**
 * cvmx_ciu_soft_bist
 */
union cvmx_ciu_soft_bist {
	u64 u64;
	struct cvmx_ciu_soft_bist_s {
		u64 reserved_1_63 : 63;
		u64 soft_bist : 1;
	} s;
	struct cvmx_ciu_soft_bist_s cn30xx;
	struct cvmx_ciu_soft_bist_s cn31xx;
	struct cvmx_ciu_soft_bist_s cn38xx;
	struct cvmx_ciu_soft_bist_s cn38xxp2;
	struct cvmx_ciu_soft_bist_s cn50xx;
	struct cvmx_ciu_soft_bist_s cn52xx;
	struct cvmx_ciu_soft_bist_s cn52xxp1;
	struct cvmx_ciu_soft_bist_s cn56xx;
	struct cvmx_ciu_soft_bist_s cn56xxp1;
	struct cvmx_ciu_soft_bist_s cn58xx;
	struct cvmx_ciu_soft_bist_s cn58xxp1;
	struct cvmx_ciu_soft_bist_s cn61xx;
	struct cvmx_ciu_soft_bist_s cn63xx;
	struct cvmx_ciu_soft_bist_s cn63xxp1;
	struct cvmx_ciu_soft_bist_s cn66xx;
	struct cvmx_ciu_soft_bist_s cn68xx;
	struct cvmx_ciu_soft_bist_s cn68xxp1;
	struct cvmx_ciu_soft_bist_s cn70xx;
	struct cvmx_ciu_soft_bist_s cn70xxp1;
	struct cvmx_ciu_soft_bist_s cnf71xx;
};

typedef union cvmx_ciu_soft_bist cvmx_ciu_soft_bist_t;

/**
 * cvmx_ciu_soft_prst
 */
union cvmx_ciu_soft_prst {
	u64 u64;
	struct cvmx_ciu_soft_prst_s {
		u64 reserved_3_63 : 61;
		u64 host64 : 1;
		u64 npi : 1;
		u64 soft_prst : 1;
	} s;
	struct cvmx_ciu_soft_prst_s cn30xx;
	struct cvmx_ciu_soft_prst_s cn31xx;
	struct cvmx_ciu_soft_prst_s cn38xx;
	struct cvmx_ciu_soft_prst_s cn38xxp2;
	struct cvmx_ciu_soft_prst_s cn50xx;
	struct cvmx_ciu_soft_prst_cn52xx {
		u64 reserved_1_63 : 63;
		u64 soft_prst : 1;
	} cn52xx;
	struct cvmx_ciu_soft_prst_cn52xx cn52xxp1;
	struct cvmx_ciu_soft_prst_cn52xx cn56xx;
	struct cvmx_ciu_soft_prst_cn52xx cn56xxp1;
	struct cvmx_ciu_soft_prst_s cn58xx;
	struct cvmx_ciu_soft_prst_s cn58xxp1;
	struct cvmx_ciu_soft_prst_cn52xx cn61xx;
	struct cvmx_ciu_soft_prst_cn52xx cn63xx;
	struct cvmx_ciu_soft_prst_cn52xx cn63xxp1;
	struct cvmx_ciu_soft_prst_cn52xx cn66xx;
	struct cvmx_ciu_soft_prst_cn52xx cn68xx;
	struct cvmx_ciu_soft_prst_cn52xx cn68xxp1;
	struct cvmx_ciu_soft_prst_cn52xx cnf71xx;
};

typedef union cvmx_ciu_soft_prst cvmx_ciu_soft_prst_t;

/**
 * cvmx_ciu_soft_prst1
 */
union cvmx_ciu_soft_prst1 {
	u64 u64;
	struct cvmx_ciu_soft_prst1_s {
		u64 reserved_1_63 : 63;
		u64 soft_prst : 1;
	} s;
	struct cvmx_ciu_soft_prst1_s cn52xx;
	struct cvmx_ciu_soft_prst1_s cn52xxp1;
	struct cvmx_ciu_soft_prst1_s cn56xx;
	struct cvmx_ciu_soft_prst1_s cn56xxp1;
	struct cvmx_ciu_soft_prst1_s cn61xx;
	struct cvmx_ciu_soft_prst1_s cn63xx;
	struct cvmx_ciu_soft_prst1_s cn63xxp1;
	struct cvmx_ciu_soft_prst1_s cn66xx;
	struct cvmx_ciu_soft_prst1_s cn68xx;
	struct cvmx_ciu_soft_prst1_s cn68xxp1;
	struct cvmx_ciu_soft_prst1_s cnf71xx;
};

typedef union cvmx_ciu_soft_prst1 cvmx_ciu_soft_prst1_t;

/**
 * cvmx_ciu_soft_prst2
 */
union cvmx_ciu_soft_prst2 {
	u64 u64;
	struct cvmx_ciu_soft_prst2_s {
		u64 reserved_1_63 : 63;
		u64 soft_prst : 1;
	} s;
	struct cvmx_ciu_soft_prst2_s cn66xx;
};

typedef union cvmx_ciu_soft_prst2 cvmx_ciu_soft_prst2_t;

/**
 * cvmx_ciu_soft_prst3
 */
union cvmx_ciu_soft_prst3 {
	u64 u64;
	struct cvmx_ciu_soft_prst3_s {
		u64 reserved_1_63 : 63;
		u64 soft_prst : 1;
	} s;
	struct cvmx_ciu_soft_prst3_s cn66xx;
};

typedef union cvmx_ciu_soft_prst3 cvmx_ciu_soft_prst3_t;

/**
 * cvmx_ciu_soft_rst
 */
union cvmx_ciu_soft_rst {
	u64 u64;
	struct cvmx_ciu_soft_rst_s {
		u64 reserved_1_63 : 63;
		u64 soft_rst : 1;
	} s;
	struct cvmx_ciu_soft_rst_s cn30xx;
	struct cvmx_ciu_soft_rst_s cn31xx;
	struct cvmx_ciu_soft_rst_s cn38xx;
	struct cvmx_ciu_soft_rst_s cn38xxp2;
	struct cvmx_ciu_soft_rst_s cn50xx;
	struct cvmx_ciu_soft_rst_s cn52xx;
	struct cvmx_ciu_soft_rst_s cn52xxp1;
	struct cvmx_ciu_soft_rst_s cn56xx;
	struct cvmx_ciu_soft_rst_s cn56xxp1;
	struct cvmx_ciu_soft_rst_s cn58xx;
	struct cvmx_ciu_soft_rst_s cn58xxp1;
	struct cvmx_ciu_soft_rst_s cn61xx;
	struct cvmx_ciu_soft_rst_s cn63xx;
	struct cvmx_ciu_soft_rst_s cn63xxp1;
	struct cvmx_ciu_soft_rst_s cn66xx;
	struct cvmx_ciu_soft_rst_s cn68xx;
	struct cvmx_ciu_soft_rst_s cn68xxp1;
	struct cvmx_ciu_soft_rst_s cnf71xx;
};

typedef union cvmx_ciu_soft_rst cvmx_ciu_soft_rst_t;

/**
 * cvmx_ciu_sum1_io#_int
 *
 * CIU_SUM1_IO0_INT is for PEM0, CIU_SUM1_IO1_INT is reserved.
 *
 */
union cvmx_ciu_sum1_iox_int {
	u64 u64;
	struct cvmx_ciu_sum1_iox_int_s {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 reserved_10_16 : 7;
		u64 wdog : 10;
	} s;
	struct cvmx_ciu_sum1_iox_int_cn61xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_4_17 : 14;
		u64 wdog : 4;
	} cn61xx;
	struct cvmx_ciu_sum1_iox_int_cn66xx {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_38_45 : 8;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_10_17 : 8;
		u64 wdog : 10;
	} cn66xx;
	struct cvmx_ciu_sum1_iox_int_cn70xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 pem2 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_18_18 : 1;
		u64 usb1 : 1;
		u64 reserved_4_16 : 13;
		u64 wdog : 4;
	} cn70xx;
	struct cvmx_ciu_sum1_iox_int_cn70xx cn70xxp1;
	struct cvmx_ciu_sum1_iox_int_cnf71xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 reserved_41_46 : 6;
		u64 dpi_dma : 1;
		u64 reserved_37_39 : 3;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 reserved_32_32 : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_4_18 : 15;
		u64 wdog : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_sum1_iox_int cvmx_ciu_sum1_iox_int_t;

/**
 * cvmx_ciu_sum1_pp#_ip2
 *
 * SUM1 becomes per IPx in o65/6 and afterwards. Only Field <40> DPI_DMA will have
 * different value per PP(IP) for  $CIU_SUM1_PPx_IPy, and <40> DPI_DMA will always
 * be zero for  $CIU_SUM1_IOX_INT. All other fields ([63:41] and [39:0]) values  are idential for
 * different PPs, same value as $CIU_INT_SUM1.
 * Write to any IRQ's PTP fields will clear PTP for all IRQ's PTP field.
 */
union cvmx_ciu_sum1_ppx_ip2 {
	u64 u64;
	struct cvmx_ciu_sum1_ppx_ip2_s {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 reserved_10_16 : 7;
		u64 wdog : 10;
	} s;
	struct cvmx_ciu_sum1_ppx_ip2_cn61xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_4_17 : 14;
		u64 wdog : 4;
	} cn61xx;
	struct cvmx_ciu_sum1_ppx_ip2_cn66xx {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_38_45 : 8;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_10_17 : 8;
		u64 wdog : 10;
	} cn66xx;
	struct cvmx_ciu_sum1_ppx_ip2_cn70xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 pem2 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_18_18 : 1;
		u64 usb1 : 1;
		u64 reserved_4_16 : 13;
		u64 wdog : 4;
	} cn70xx;
	struct cvmx_ciu_sum1_ppx_ip2_cn70xx cn70xxp1;
	struct cvmx_ciu_sum1_ppx_ip2_cnf71xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 reserved_41_46 : 6;
		u64 dpi_dma : 1;
		u64 reserved_37_39 : 3;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 reserved_32_32 : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_4_18 : 15;
		u64 wdog : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_sum1_ppx_ip2 cvmx_ciu_sum1_ppx_ip2_t;

/**
 * cvmx_ciu_sum1_pp#_ip3
 *
 * Notes:
 * SUM1 becomes per IPx in o65/6 and afterwards. Only Field <40> DPI_DMA will have
 * different value per PP(IP) for  $CIU_SUM1_PPx_IPy, and <40> DPI_DMA will always
 * be zero for  $CIU_SUM1_IOX_INT. All other fields ([63:41] and [39:0]) values  are idential for
 * different PPs, same value as $CIU_INT_SUM1.
 * Write to any IRQ's PTP fields will clear PTP for all IRQ's PTP field.
 */
union cvmx_ciu_sum1_ppx_ip3 {
	u64 u64;
	struct cvmx_ciu_sum1_ppx_ip3_s {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 reserved_10_16 : 7;
		u64 wdog : 10;
	} s;
	struct cvmx_ciu_sum1_ppx_ip3_cn61xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_4_17 : 14;
		u64 wdog : 4;
	} cn61xx;
	struct cvmx_ciu_sum1_ppx_ip3_cn66xx {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_38_45 : 8;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_10_17 : 8;
		u64 wdog : 10;
	} cn66xx;
	struct cvmx_ciu_sum1_ppx_ip3_cn70xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 pem2 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_18_18 : 1;
		u64 usb1 : 1;
		u64 reserved_4_16 : 13;
		u64 wdog : 4;
	} cn70xx;
	struct cvmx_ciu_sum1_ppx_ip3_cn70xx cn70xxp1;
	struct cvmx_ciu_sum1_ppx_ip3_cnf71xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 reserved_41_46 : 6;
		u64 dpi_dma : 1;
		u64 reserved_37_39 : 3;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 reserved_32_32 : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_4_18 : 15;
		u64 wdog : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_sum1_ppx_ip3 cvmx_ciu_sum1_ppx_ip3_t;

/**
 * cvmx_ciu_sum1_pp#_ip4
 *
 * Notes:
 * SUM1 becomes per IPx in o65/6 and afterwards. Only Field <40> DPI_DMA will have
 * different value per PP(IP) for  $CIU_SUM1_PPx_IPy, and <40> DPI_DMA will always
 * be zero for  $CIU_SUM1_IOX_INT. All other fields ([63:41] and [39:0]) values  are idential for
 * different PPs, same value as $CIU_INT_SUM1.
 * Write to any IRQ's PTP fields will clear PTP for all IRQ's PTP field.
 */
union cvmx_ciu_sum1_ppx_ip4 {
	u64 u64;
	struct cvmx_ciu_sum1_ppx_ip4_s {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 usb1 : 1;
		u64 reserved_10_16 : 7;
		u64 wdog : 10;
	} s;
	struct cvmx_ciu_sum1_ppx_ip4_cn61xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_4_17 : 14;
		u64 wdog : 4;
	} cn61xx;
	struct cvmx_ciu_sum1_ppx_ip4_cn66xx {
		u64 rst : 1;
		u64 reserved_62_62 : 1;
		u64 srio3 : 1;
		u64 srio2 : 1;
		u64 reserved_57_59 : 3;
		u64 dfm : 1;
		u64 reserved_53_55 : 3;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 srio0 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_38_45 : 8;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 zip : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 mii1 : 1;
		u64 reserved_10_17 : 8;
		u64 wdog : 10;
	} cn66xx;
	struct cvmx_ciu_sum1_ppx_ip4_cn70xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_51_51 : 1;
		u64 pem2 : 1;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 agl : 1;
		u64 reserved_41_45 : 5;
		u64 dpi_dma : 1;
		u64 reserved_38_39 : 2;
		u64 agx1 : 1;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 dfa : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_18_18 : 1;
		u64 usb1 : 1;
		u64 reserved_4_16 : 13;
		u64 wdog : 4;
	} cn70xx;
	struct cvmx_ciu_sum1_ppx_ip4_cn70xx cn70xxp1;
	struct cvmx_ciu_sum1_ppx_ip4_cnf71xx {
		u64 rst : 1;
		u64 reserved_53_62 : 10;
		u64 lmc0 : 1;
		u64 reserved_50_51 : 2;
		u64 pem1 : 1;
		u64 pem0 : 1;
		u64 ptp : 1;
		u64 reserved_41_46 : 6;
		u64 dpi_dma : 1;
		u64 reserved_37_39 : 3;
		u64 agx0 : 1;
		u64 dpi : 1;
		u64 sli : 1;
		u64 usb : 1;
		u64 reserved_32_32 : 1;
		u64 key : 1;
		u64 rad : 1;
		u64 tim : 1;
		u64 reserved_28_28 : 1;
		u64 pko : 1;
		u64 pip : 1;
		u64 ipd : 1;
		u64 l2c : 1;
		u64 pow : 1;
		u64 fpa : 1;
		u64 iob : 1;
		u64 mio : 1;
		u64 nand : 1;
		u64 reserved_4_18 : 15;
		u64 wdog : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_sum1_ppx_ip4 cvmx_ciu_sum1_ppx_ip4_t;

/**
 * cvmx_ciu_sum2_io#_int
 *
 * CIU_SUM2_IO0_INT is for PEM0, CIU_SUM2_IO1_INT is reserved.
 *
 */
union cvmx_ciu_sum2_iox_int {
	u64 u64;
	struct cvmx_ciu_sum2_iox_int_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_sum2_iox_int_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_sum2_iox_int_cn61xx cn66xx;
	struct cvmx_ciu_sum2_iox_int_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_sum2_iox_int_cn70xx cn70xxp1;
	struct cvmx_ciu_sum2_iox_int_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_sum2_iox_int cvmx_ciu_sum2_iox_int_t;

/**
 * cvmx_ciu_sum2_pp#_ip2
 *
 * Only TIMER field may have different value per PP(IP).
 * All other fields  values  are idential for different PPs.
 */
union cvmx_ciu_sum2_ppx_ip2 {
	u64 u64;
	struct cvmx_ciu_sum2_ppx_ip2_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_sum2_ppx_ip2_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_sum2_ppx_ip2_cn61xx cn66xx;
	struct cvmx_ciu_sum2_ppx_ip2_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_sum2_ppx_ip2_cn70xx cn70xxp1;
	struct cvmx_ciu_sum2_ppx_ip2_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_sum2_ppx_ip2 cvmx_ciu_sum2_ppx_ip2_t;

/**
 * cvmx_ciu_sum2_pp#_ip3
 *
 * Notes:
 * These SUM2 CSR's did not exist prior to pass 1.2. CIU_TIM4-9 did not exist prior to pass 1.2.
 *
 */
union cvmx_ciu_sum2_ppx_ip3 {
	u64 u64;
	struct cvmx_ciu_sum2_ppx_ip3_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_sum2_ppx_ip3_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_sum2_ppx_ip3_cn61xx cn66xx;
	struct cvmx_ciu_sum2_ppx_ip3_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_sum2_ppx_ip3_cn70xx cn70xxp1;
	struct cvmx_ciu_sum2_ppx_ip3_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_sum2_ppx_ip3 cvmx_ciu_sum2_ppx_ip3_t;

/**
 * cvmx_ciu_sum2_pp#_ip4
 *
 * Notes:
 * These SUM2 CSR's did not exist prior to pass 1.2. CIU_TIM4-9 did not exist prior to pass 1.2.
 *
 */
union cvmx_ciu_sum2_ppx_ip4 {
	u64 u64;
	struct cvmx_ciu_sum2_ppx_ip4_s {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_15_15 : 1;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_ciu_sum2_ppx_ip4_cn61xx {
		u64 reserved_10_63 : 54;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn61xx;
	struct cvmx_ciu_sum2_ppx_ip4_cn61xx cn66xx;
	struct cvmx_ciu_sum2_ppx_ip4_cn70xx {
		u64 reserved_20_63 : 44;
		u64 bch : 1;
		u64 agl_drp : 1;
		u64 ocla : 1;
		u64 sata : 1;
		u64 reserved_10_15 : 6;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cn70xx;
	struct cvmx_ciu_sum2_ppx_ip4_cn70xx cn70xxp1;
	struct cvmx_ciu_sum2_ppx_ip4_cnf71xx {
		u64 reserved_15_63 : 49;
		u64 endor : 2;
		u64 eoi : 1;
		u64 reserved_10_11 : 2;
		u64 timer : 6;
		u64 reserved_0_3 : 4;
	} cnf71xx;
};

typedef union cvmx_ciu_sum2_ppx_ip4 cvmx_ciu_sum2_ppx_ip4_t;

/**
 * cvmx_ciu_tim#
 *
 * Notes:
 * CIU_TIM4-9 did not exist prior to pass 1.2
 *
 */
union cvmx_ciu_timx {
	u64 u64;
	struct cvmx_ciu_timx_s {
		u64 reserved_37_63 : 27;
		u64 one_shot : 1;
		u64 len : 36;
	} s;
	struct cvmx_ciu_timx_s cn30xx;
	struct cvmx_ciu_timx_s cn31xx;
	struct cvmx_ciu_timx_s cn38xx;
	struct cvmx_ciu_timx_s cn38xxp2;
	struct cvmx_ciu_timx_s cn50xx;
	struct cvmx_ciu_timx_s cn52xx;
	struct cvmx_ciu_timx_s cn52xxp1;
	struct cvmx_ciu_timx_s cn56xx;
	struct cvmx_ciu_timx_s cn56xxp1;
	struct cvmx_ciu_timx_s cn58xx;
	struct cvmx_ciu_timx_s cn58xxp1;
	struct cvmx_ciu_timx_s cn61xx;
	struct cvmx_ciu_timx_s cn63xx;
	struct cvmx_ciu_timx_s cn63xxp1;
	struct cvmx_ciu_timx_s cn66xx;
	struct cvmx_ciu_timx_s cn68xx;
	struct cvmx_ciu_timx_s cn68xxp1;
	struct cvmx_ciu_timx_s cn70xx;
	struct cvmx_ciu_timx_s cn70xxp1;
	struct cvmx_ciu_timx_s cnf71xx;
};

typedef union cvmx_ciu_timx cvmx_ciu_timx_t;

/**
 * cvmx_ciu_tim_multi_cast
 *
 * Notes:
 * This register does not exist prior to pass 1.2 silicon. Those earlier chip passes operate as if
 * EN==0.
 */
union cvmx_ciu_tim_multi_cast {
	u64 u64;
	struct cvmx_ciu_tim_multi_cast_s {
		u64 reserved_1_63 : 63;
		u64 en : 1;
	} s;
	struct cvmx_ciu_tim_multi_cast_s cn61xx;
	struct cvmx_ciu_tim_multi_cast_s cn66xx;
	struct cvmx_ciu_tim_multi_cast_s cn70xx;
	struct cvmx_ciu_tim_multi_cast_s cn70xxp1;
	struct cvmx_ciu_tim_multi_cast_s cnf71xx;
};

typedef union cvmx_ciu_tim_multi_cast cvmx_ciu_tim_multi_cast_t;

/**
 * cvmx_ciu_wdog#
 */
union cvmx_ciu_wdogx {
	u64 u64;
	struct cvmx_ciu_wdogx_s {
		u64 reserved_46_63 : 18;
		u64 gstopen : 1;
		u64 dstop : 1;
		u64 cnt : 24;
		u64 len : 16;
		u64 state : 2;
		u64 mode : 2;
	} s;
	struct cvmx_ciu_wdogx_s cn30xx;
	struct cvmx_ciu_wdogx_s cn31xx;
	struct cvmx_ciu_wdogx_s cn38xx;
	struct cvmx_ciu_wdogx_s cn38xxp2;
	struct cvmx_ciu_wdogx_s cn50xx;
	struct cvmx_ciu_wdogx_s cn52xx;
	struct cvmx_ciu_wdogx_s cn52xxp1;
	struct cvmx_ciu_wdogx_s cn56xx;
	struct cvmx_ciu_wdogx_s cn56xxp1;
	struct cvmx_ciu_wdogx_s cn58xx;
	struct cvmx_ciu_wdogx_s cn58xxp1;
	struct cvmx_ciu_wdogx_s cn61xx;
	struct cvmx_ciu_wdogx_s cn63xx;
	struct cvmx_ciu_wdogx_s cn63xxp1;
	struct cvmx_ciu_wdogx_s cn66xx;
	struct cvmx_ciu_wdogx_s cn68xx;
	struct cvmx_ciu_wdogx_s cn68xxp1;
	struct cvmx_ciu_wdogx_s cn70xx;
	struct cvmx_ciu_wdogx_s cn70xxp1;
	struct cvmx_ciu_wdogx_s cn73xx;
	struct cvmx_ciu_wdogx_s cn78xx;
	struct cvmx_ciu_wdogx_s cn78xxp1;
	struct cvmx_ciu_wdogx_s cnf71xx;
	struct cvmx_ciu_wdogx_s cnf75xx;
};

typedef union cvmx_ciu_wdogx cvmx_ciu_wdogx_t;

#endif
