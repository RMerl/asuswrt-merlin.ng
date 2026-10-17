/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon gmxx.
 */

#ifndef __CVMX_GMXX_DEFS_H__
#define __CVMX_GMXX_DEFS_H__

static inline u64 CVMX_GMXX_BAD_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000518ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000518ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000518ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000518ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_BIST(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000400ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000400ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000400ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000400ull + (offset) * 0x8000000ull;
}

#define CVMX_GMXX_BPID_MAPX(offset, block_id)                                                      \
	(0x0001180008000680ull + (((offset) & 15) + ((block_id) & 7) * 0x200000ull) * 8)
#define CVMX_GMXX_BPID_MSK(offset) (0x0001180008000700ull + ((offset) & 7) * 0x1000000ull)
static inline u64 CVMX_GMXX_CLK_EN(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800080007F0ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800080007F0ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080007F0ull + (offset) * 0x1000000ull;
	}
	return 0x00011800080007F0ull + (offset) * 0x8000000ull;
}

#define CVMX_GMXX_EBP_DIS(offset) (0x0001180008000608ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_GMXX_EBP_MSK(offset) (0x0001180008000600ull + ((offset) & 7) * 0x1000000ull)
static inline u64 CVMX_GMXX_HG2_CONTROL(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000550ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000550ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000550ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000550ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_INF_MODE(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800080007F8ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800080007F8ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080007F8ull + (offset) * 0x1000000ull;
	}
	return 0x00011800080007F8ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_NXA_ADR(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000510ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000510ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000510ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000510ull + (offset) * 0x8000000ull;
}

#define CVMX_GMXX_PIPE_STATUS(offset) (0x0001180008000760ull + ((offset) & 7) * 0x1000000ull)
static inline u64 CVMX_GMXX_PRTX_CBFC_CTL(unsigned long __attribute__((unused)) offset,
					  unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000580ull + (block_id) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000580ull + (block_id) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000580ull + (block_id) * 0x1000000ull;
	}
	return 0x0001180008000580ull + (block_id) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_PRTX_CFG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000010ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000010ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000010ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

#define CVMX_GMXX_QSGMII_CTL(offset) (0x0001180008000760ull + ((offset) & 1) * 0x8000000ull)
static inline u64 CVMX_GMXX_RXAUI_CTL(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000740ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000740ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000740ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_RXX_ADR_CAM0(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000180ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000180ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000180ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_ADR_CAM1(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000188ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000188ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000188ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_ADR_CAM2(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000190ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000190ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000190ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_ADR_CAM3(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000198ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000198ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000198ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_ADR_CAM4(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800080001A0ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080001A0ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x00011800080001A0ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_ADR_CAM5(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800080001A8ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080001A8ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x00011800080001A8ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_ADR_CAM_ALL_EN(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000110ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000110ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000110ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000110ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_ADR_CAM_EN(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000108ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000108ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000108ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_ADR_CTL(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000100ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000100ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000100ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_DECISION(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000040ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000040ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000040ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_FRM_CHK(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000020ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000020ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000020ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_FRM_CTL(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000018ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000018ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000018ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

#define CVMX_GMXX_RXX_FRM_MAX(offset, block_id)                                                    \
	(0x0001180008000030ull + (((offset) & 3) + ((block_id) & 1) * 0x10000ull) * 2048)
#define CVMX_GMXX_RXX_FRM_MIN(offset, block_id)                                                    \
	(0x0001180008000028ull + (((offset) & 3) + ((block_id) & 1) * 0x10000ull) * 2048)
static inline u64 CVMX_GMXX_RXX_IFG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000058ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000058ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000058ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_INT_EN(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000008ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000008ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000008ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_INT_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000000ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000000ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000000ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_JABBER(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000038ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000038ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000038ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_PAUSE_DROP_TIME(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000068ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000068ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000068ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000068ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000068ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

#define CVMX_GMXX_RXX_RX_INBND(offset, block_id)                                                   \
	(0x0001180008000060ull + (((offset) & 3) + ((block_id) & 1) * 0x10000ull) * 2048)
static inline u64 CVMX_GMXX_RXX_STATS_CTL(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000050ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000050ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000050ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_STATS_OCTS(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000088ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000088ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000088ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_STATS_OCTS_CTL(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000098ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000098ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000098ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_STATS_OCTS_DMAC(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800080000A8ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080000A8ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x00011800080000A8ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_STATS_OCTS_DRP(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800080000B8ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080000B8ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x00011800080000B8ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_STATS_PKTS(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000080ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000080ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000080ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_STATS_PKTS_BAD(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800080000C0ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080000C0ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x00011800080000C0ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_STATS_PKTS_CTL(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000090ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000090ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000090ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_STATS_PKTS_DMAC(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800080000A0ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080000A0ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x00011800080000A0ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_STATS_PKTS_DRP(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800080000B0ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080000B0ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x00011800080000B0ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RXX_UDD_SKP(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000048ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000048ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000048ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_RX_BP_DROPX(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000420ull + ((offset) + (block_id) * 0x1000000ull) * 8;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000420ull + ((offset) + (block_id) * 0x200000ull) * 8;
	}
	return 0x0001180008000420ull + ((offset) + (block_id) * 0x1000000ull) * 8;
}

static inline u64 CVMX_GMXX_RX_BP_OFFX(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000460ull + ((offset) + (block_id) * 0x1000000ull) * 8;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000460ull + ((offset) + (block_id) * 0x200000ull) * 8;
	}
	return 0x0001180008000460ull + ((offset) + (block_id) * 0x1000000ull) * 8;
}

static inline u64 CVMX_GMXX_RX_BP_ONX(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000440ull + ((offset) + (block_id) * 0x1000000ull) * 8;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000440ull + ((offset) + (block_id) * 0x200000ull) * 8;
	}
	return 0x0001180008000440ull + ((offset) + (block_id) * 0x1000000ull) * 8;
}

static inline u64 CVMX_GMXX_RX_HG2_STATUS(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000548ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000548ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000548ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000548ull + (offset) * 0x8000000ull;
}

#define CVMX_GMXX_RX_PASS_EN(offset) (0x00011800080005F8ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_GMXX_RX_PASS_MAPX(offset, block_id)                                                   \
	(0x0001180008000600ull + (((offset) & 15) + ((block_id) & 1) * 0x1000000ull) * 8)
static inline u64 CVMX_GMXX_RX_PRTS(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000410ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000410ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000410ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000410ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_RX_PRT_INFO(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004E8ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004E8ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004E8ull + (offset) * 0x1000000ull;
	}
	return 0x00011800080004E8ull + (offset) * 0x8000000ull;
}

#define CVMX_GMXX_RX_TX_STATUS(offset) (0x00011800080007E8ull)
static inline u64 CVMX_GMXX_RX_XAUI_BAD_COL(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000538ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000538ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000538ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000538ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_RX_XAUI_CTL(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000530ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000530ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000530ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000530ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_SMACX(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000230ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000230ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000230ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_SOFT_BIST(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800080007E8ull + (offset) * 0x8000000ull;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800080007E8ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080007E8ull + (offset) * 0x1000000ull;
	}
	return 0x00011800080007E8ull + (offset) * 0x1000000ull;
}

static inline u64 CVMX_GMXX_STAT_BP(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000520ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000520ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000520ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000520ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TB_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800080007E0ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800080007E0ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080007E0ull + (offset) * 0x1000000ull;
	}
	return 0x00011800080007E0ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TXX_APPEND(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000218ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000218ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000218ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

#define CVMX_GMXX_TXX_BCK_CRDT(offset, block_id)                                                   \
	(0x0001180008000388ull + (((offset) & 3) + ((block_id) & 1) * 0x10000ull) * 2048)
static inline u64 CVMX_GMXX_TXX_BURST(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000228ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000228ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000228ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_CBFC_XOFF(unsigned long __attribute__((unused)) offset,
					  unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800080005A0ull + (block_id) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800080005A0ull + (block_id) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080005A0ull + (block_id) * 0x1000000ull;
	}
	return 0x00011800080005A0ull + (block_id) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TXX_CBFC_XON(unsigned long __attribute__((unused)) offset,
					 unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800080005C0ull + (block_id) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800080005C0ull + (block_id) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080005C0ull + (block_id) * 0x1000000ull;
	}
	return 0x00011800080005C0ull + (block_id) * 0x8000000ull;
}

#define CVMX_GMXX_TXX_CLK(offset, block_id)                                                        \
	(0x0001180008000208ull + (((offset) & 3) + ((block_id) & 1) * 0x10000ull) * 2048)
static inline u64 CVMX_GMXX_TXX_CTL(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000270ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000270ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000270ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

#define CVMX_GMXX_TXX_JAM_MODE(offset, block_id)                                                   \
	(0x0001180008000380ull + (((offset) & 3) + ((block_id) & 1) * 0x10000ull) * 2048)
static inline u64 CVMX_GMXX_TXX_MIN_PKT(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000240ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000240ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000240ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_PAUSE_PKT_INTERVAL(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000248ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000248ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000248ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_PAUSE_PKT_TIME(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000238ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000238ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000238ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_PAUSE_TOGO(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000258ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000258ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000258ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_PAUSE_ZERO(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000260ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000260ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000260ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

#define CVMX_GMXX_TXX_PIPE(offset, block_id)                                                       \
	(0x0001180008000310ull + (((offset) & 3) + ((block_id) & 7) * 0x2000ull) * 2048)
static inline u64 CVMX_GMXX_TXX_SGMII_CTL(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000300ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000300ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000300ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000300ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_SLOT(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000220ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000220ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000220ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_SOFT_PAUSE(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000250ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000250ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000250ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_STAT0(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000280ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000280ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000280ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_STAT1(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000288ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000288ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000288ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_STAT2(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000290ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000290ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000290ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_STAT3(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000298ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000298ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000298ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_STAT4(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800080002A0ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080002A0ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x00011800080002A0ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_STAT5(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800080002A8ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080002A8ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x00011800080002A8ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_STAT6(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800080002B0ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080002B0ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x00011800080002B0ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_STAT7(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800080002B8ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080002B8ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x00011800080002B8ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_STAT8(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800080002C0ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080002C0ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x00011800080002C0ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_STAT9(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800080002C8ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080002C8ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x00011800080002C8ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_STATS_CTL(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000268ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000268ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000268ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TXX_THRESH(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000210ull + ((offset) + (block_id) * 0x10000ull) * 2048;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000210ull + ((offset) + (block_id) * 0x2000ull) * 2048;
	}
	return 0x0001180008000210ull + ((offset) + (block_id) * 0x10000ull) * 2048;
}

static inline u64 CVMX_GMXX_TX_BP(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004D0ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004D0ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004D0ull + (offset) * 0x1000000ull;
	}
	return 0x00011800080004D0ull + (offset) * 0x8000000ull;
}

#define CVMX_GMXX_TX_CLK_MSKX(offset, block_id)                                                    \
	(0x0001180008000780ull + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8)
static inline u64 CVMX_GMXX_TX_COL_ATTEMPT(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000498ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000498ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000498ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000498ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TX_CORRUPT(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004D8ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004D8ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004D8ull + (offset) * 0x1000000ull;
	}
	return 0x00011800080004D8ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TX_HG2_REG1(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000558ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000558ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000558ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000558ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TX_HG2_REG2(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000560ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000560ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000560ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000560ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TX_IFG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000488ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000488ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000488ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000488ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TX_INT_EN(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000508ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000508ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000508ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000508ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TX_INT_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000500ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000500ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000500ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000500ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TX_JAM(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000490ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000490ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000490ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000490ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TX_LFSR(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004F8ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004F8ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004F8ull + (offset) * 0x1000000ull;
	}
	return 0x00011800080004F8ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TX_OVR_BP(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004C8ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004C8ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004C8ull + (offset) * 0x1000000ull;
	}
	return 0x00011800080004C8ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TX_PAUSE_PKT_DMAC(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004A0ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004A0ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004A0ull + (offset) * 0x1000000ull;
	}
	return 0x00011800080004A0ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TX_PAUSE_PKT_TYPE(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004A8ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004A8ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800080004A8ull + (offset) * 0x1000000ull;
	}
	return 0x00011800080004A8ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_GMXX_TX_PRTS(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000480ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000480ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000480ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000480ull + (offset) * 0x8000000ull;
}

#define CVMX_GMXX_TX_SPI_CTL(offset)   (0x00011800080004C0ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_GMXX_TX_SPI_DRAIN(offset) (0x00011800080004E0ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_GMXX_TX_SPI_MAX(offset)   (0x00011800080004B0ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_GMXX_TX_SPI_ROUNDX(offset, block_id)                                                  \
	(0x0001180008000680ull + (((offset) & 31) + ((block_id) & 1) * 0x1000000ull) * 8)
#define CVMX_GMXX_TX_SPI_THRESH(offset) (0x00011800080004B8ull + ((offset) & 1) * 0x8000000ull)
static inline u64 CVMX_GMXX_TX_XAUI_CTL(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000528ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000528ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000528ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000528ull + (offset) * 0x8000000ull;
}

#define CVMX_GMXX_WOL_CTL(offset) (0x0001180008000780ull + ((offset) & 1) * 0x8000000ull)
static inline u64 CVMX_GMXX_XAUI_EXT_LOOPBACK(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000540ull + (offset) * 0x8000000ull;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000540ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180008000540ull + (offset) * 0x1000000ull;
	}
	return 0x0001180008000540ull + (offset) * 0x8000000ull;
}

/**
 * cvmx_gmx#_bad_reg
 *
 * GMX_BAD_REG = A collection of things that have gone very, very wrong
 *
 *
 * Notes:
 * In XAUI mode, only the lsb (corresponding to port0) of INB_NXA, LOSTSTAT, OUT_OVR, are used.
 *
 */
union cvmx_gmxx_bad_reg {
	u64 u64;
	struct cvmx_gmxx_bad_reg_s {
		u64 reserved_31_63 : 33;
		u64 inb_nxa : 4;
		u64 statovr : 1;
		u64 loststat : 4;
		u64 reserved_18_21 : 4;
		u64 out_ovr : 16;
		u64 ncb_ovr : 1;
		u64 out_col : 1;
	} s;
	struct cvmx_gmxx_bad_reg_cn30xx {
		u64 reserved_31_63 : 33;
		u64 inb_nxa : 4;
		u64 statovr : 1;
		u64 reserved_25_25 : 1;
		u64 loststat : 3;
		u64 reserved_5_21 : 17;
		u64 out_ovr : 3;
		u64 reserved_0_1 : 2;
	} cn30xx;
	struct cvmx_gmxx_bad_reg_cn30xx cn31xx;
	struct cvmx_gmxx_bad_reg_s cn38xx;
	struct cvmx_gmxx_bad_reg_s cn38xxp2;
	struct cvmx_gmxx_bad_reg_cn30xx cn50xx;
	struct cvmx_gmxx_bad_reg_cn52xx {
		u64 reserved_31_63 : 33;
		u64 inb_nxa : 4;
		u64 statovr : 1;
		u64 loststat : 4;
		u64 reserved_6_21 : 16;
		u64 out_ovr : 4;
		u64 reserved_0_1 : 2;
	} cn52xx;
	struct cvmx_gmxx_bad_reg_cn52xx cn52xxp1;
	struct cvmx_gmxx_bad_reg_cn52xx cn56xx;
	struct cvmx_gmxx_bad_reg_cn52xx cn56xxp1;
	struct cvmx_gmxx_bad_reg_s cn58xx;
	struct cvmx_gmxx_bad_reg_s cn58xxp1;
	struct cvmx_gmxx_bad_reg_cn52xx cn61xx;
	struct cvmx_gmxx_bad_reg_cn52xx cn63xx;
	struct cvmx_gmxx_bad_reg_cn52xx cn63xxp1;
	struct cvmx_gmxx_bad_reg_cn52xx cn66xx;
	struct cvmx_gmxx_bad_reg_cn52xx cn68xx;
	struct cvmx_gmxx_bad_reg_cn52xx cn68xxp1;
	struct cvmx_gmxx_bad_reg_cn52xx cn70xx;
	struct cvmx_gmxx_bad_reg_cn52xx cn70xxp1;
	struct cvmx_gmxx_bad_reg_cn52xx cnf71xx;
};

typedef union cvmx_gmxx_bad_reg cvmx_gmxx_bad_reg_t;

/**
 * cvmx_gmx#_bist
 *
 * GMX_BIST = GMX BIST Results
 *
 */
union cvmx_gmxx_bist {
	u64 u64;
	struct cvmx_gmxx_bist_s {
		u64 reserved_25_63 : 39;
		u64 status : 25;
	} s;
	struct cvmx_gmxx_bist_cn30xx {
		u64 reserved_10_63 : 54;
		u64 status : 10;
	} cn30xx;
	struct cvmx_gmxx_bist_cn30xx cn31xx;
	struct cvmx_gmxx_bist_cn30xx cn38xx;
	struct cvmx_gmxx_bist_cn30xx cn38xxp2;
	struct cvmx_gmxx_bist_cn50xx {
		u64 reserved_12_63 : 52;
		u64 status : 12;
	} cn50xx;
	struct cvmx_gmxx_bist_cn52xx {
		u64 reserved_16_63 : 48;
		u64 status : 16;
	} cn52xx;
	struct cvmx_gmxx_bist_cn52xx cn52xxp1;
	struct cvmx_gmxx_bist_cn52xx cn56xx;
	struct cvmx_gmxx_bist_cn52xx cn56xxp1;
	struct cvmx_gmxx_bist_cn58xx {
		u64 reserved_17_63 : 47;
		u64 status : 17;
	} cn58xx;
	struct cvmx_gmxx_bist_cn58xx cn58xxp1;
	struct cvmx_gmxx_bist_s cn61xx;
	struct cvmx_gmxx_bist_s cn63xx;
	struct cvmx_gmxx_bist_s cn63xxp1;
	struct cvmx_gmxx_bist_s cn66xx;
	struct cvmx_gmxx_bist_s cn68xx;
	struct cvmx_gmxx_bist_s cn68xxp1;
	struct cvmx_gmxx_bist_s cn70xx;
	struct cvmx_gmxx_bist_s cn70xxp1;
	struct cvmx_gmxx_bist_s cnf71xx;
};

typedef union cvmx_gmxx_bist cvmx_gmxx_bist_t;

/**
 * cvmx_gmx#_bpid_map#
 *
 * Notes:
 * GMX will build BPID_VECTOR<15:0> using the 16 GMX_BPID_MAP entries and the BPID
 * state from IPD.  In XAUI/RXAUI mode when PFC/CBFC/HiGig2 is used, the
 * BPID_VECTOR becomes the logical backpressure.  In XAUI/RXAUI mode when
 * PFC/CBFC/HiGig2 is not used or when in 4xSGMII mode, the BPID_VECTOR can be used
 * with the GMX_BPID_MSK register to determine the physical backpressure.
 *
 * In XAUI/RXAUI mode, the entire BPID_VECTOR<15:0> is available determining physical
 * backpressure for the single XAUI/RXAUI interface.
 *
 * In SGMII mode, BPID_VECTOR is broken up as follows:
 *    SGMII interface0 uses BPID_VECTOR<3:0>
 *    SGMII interface1 uses BPID_VECTOR<7:4>
 *    SGMII interface2 uses BPID_VECTOR<11:8>
 *    SGMII interface3 uses BPID_VECTOR<15:12>
 *
 * In all SGMII configurations, and in some XAUI/RXAUI configurations, the
 * interface protocols only support physical backpressure. In these cases, a single
 * BPID will commonly drive the physical backpressure for the physical
 * interface. We provide example programmings for these simple cases.
 *
 * In XAUI/RXAUI mode where PFC/CBFC/HiGig2 is not used, an example programming
 * would be as follows:
 *
 *    @verbatim
 *    GMX_BPID_MAP0[VAL]    = 1;
 *    GMX_BPID_MAP0[BPID]   = xaui_bpid;
 *    GMX_BPID_MSK[MSK_OR]  = 1;
 *    GMX_BPID_MSK[MSK_AND] = 0;
 *    @endverbatim
 *
 * In SGMII mode, an example programming would be as follows:
 *
 *    @verbatim
 *    for (i=0; i<4; i++) [
 *       if (GMX_PRTi_CFG[EN]) [
 *          GMX_BPID_MAP(i*4)[VAL]    = 1;
 *          GMX_BPID_MAP(i*4)[BPID]   = sgmii_bpid(i);
 *          GMX_BPID_MSK[MSK_OR]      = (1 << (i*4)) | GMX_BPID_MSK[MSK_OR];
 *       ]
 *    ]
 *    GMX_BPID_MSK[MSK_AND] = 0;
 *    @endverbatim
 */
union cvmx_gmxx_bpid_mapx {
	u64 u64;
	struct cvmx_gmxx_bpid_mapx_s {
		u64 reserved_17_63 : 47;
		u64 status : 1;
		u64 reserved_9_15 : 7;
		u64 val : 1;
		u64 reserved_6_7 : 2;
		u64 bpid : 6;
	} s;
	struct cvmx_gmxx_bpid_mapx_s cn68xx;
	struct cvmx_gmxx_bpid_mapx_s cn68xxp1;
};

typedef union cvmx_gmxx_bpid_mapx cvmx_gmxx_bpid_mapx_t;

/**
 * cvmx_gmx#_bpid_msk
 */
union cvmx_gmxx_bpid_msk {
	u64 u64;
	struct cvmx_gmxx_bpid_msk_s {
		u64 reserved_48_63 : 16;
		u64 msk_or : 16;
		u64 reserved_16_31 : 16;
		u64 msk_and : 16;
	} s;
	struct cvmx_gmxx_bpid_msk_s cn68xx;
	struct cvmx_gmxx_bpid_msk_s cn68xxp1;
};

typedef union cvmx_gmxx_bpid_msk cvmx_gmxx_bpid_msk_t;

/**
 * cvmx_gmx#_clk_en
 *
 * DON'T PUT IN HRM*
 *
 */
union cvmx_gmxx_clk_en {
	u64 u64;
	struct cvmx_gmxx_clk_en_s {
		u64 reserved_1_63 : 63;
		u64 clk_en : 1;
	} s;
	struct cvmx_gmxx_clk_en_s cn52xx;
	struct cvmx_gmxx_clk_en_s cn52xxp1;
	struct cvmx_gmxx_clk_en_s cn56xx;
	struct cvmx_gmxx_clk_en_s cn56xxp1;
	struct cvmx_gmxx_clk_en_s cn61xx;
	struct cvmx_gmxx_clk_en_s cn63xx;
	struct cvmx_gmxx_clk_en_s cn63xxp1;
	struct cvmx_gmxx_clk_en_s cn66xx;
	struct cvmx_gmxx_clk_en_s cn68xx;
	struct cvmx_gmxx_clk_en_s cn68xxp1;
	struct cvmx_gmxx_clk_en_s cn70xx;
	struct cvmx_gmxx_clk_en_s cn70xxp1;
	struct cvmx_gmxx_clk_en_s cnf71xx;
};

typedef union cvmx_gmxx_clk_en cvmx_gmxx_clk_en_t;

/**
 * cvmx_gmx#_ebp_dis
 */
union cvmx_gmxx_ebp_dis {
	u64 u64;
	struct cvmx_gmxx_ebp_dis_s {
		u64 reserved_16_63 : 48;
		u64 dis : 16;
	} s;
	struct cvmx_gmxx_ebp_dis_s cn68xx;
	struct cvmx_gmxx_ebp_dis_s cn68xxp1;
};

typedef union cvmx_gmxx_ebp_dis cvmx_gmxx_ebp_dis_t;

/**
 * cvmx_gmx#_ebp_msk
 */
union cvmx_gmxx_ebp_msk {
	u64 u64;
	struct cvmx_gmxx_ebp_msk_s {
		u64 reserved_16_63 : 48;
		u64 msk : 16;
	} s;
	struct cvmx_gmxx_ebp_msk_s cn68xx;
	struct cvmx_gmxx_ebp_msk_s cn68xxp1;
};

typedef union cvmx_gmxx_ebp_msk cvmx_gmxx_ebp_msk_t;

/**
 * cvmx_gmx#_hg2_control
 *
 * Notes:
 * The HiGig2 TX and RX enable would normally be both set together for HiGig2 messaging. However
 * setting just the TX or RX bit will result in only the HG2 message transmit or the receive
 * capability.
 * PHYS_EN and LOGL_EN bits when 1, allow link pause or back pressure to PKO as per received
 * HiGig2 message. When 0, link pause and back pressure to PKO in response to received messages
 * are disabled.
 *
 * GMX*_TX_XAUI_CTL[HG_EN] must be set to one(to enable HiGig) whenever either HG2TX_EN or HG2RX_EN
 * are set.
 *
 * GMX*_RX0_UDD_SKP[LEN] must be set to 16 (to select HiGig2) whenever either HG2TX_EN or HG2RX_EN
 * are set.
 *
 * GMX*_TX_OVR_BP[EN<0>] must be set to one and GMX*_TX_OVR_BP[BP<0>] must be cleared to zero
 * (to forcibly disable HW-automatic 802.3 pause packet generation) with the HiGig2 Protocol when
 * GMX*_HG2_CONTROL[HG2TX_EN]=0. (The HiGig2 protocol is indicated by GMX*_TX_XAUI_CTL[HG_EN]=1
 * and GMX*_RX0_UDD_SKP[LEN]=16.) The HW can only auto-generate backpressure via HiGig2 messages
 * (optionally, when HG2TX_EN=1) with the HiGig2 protocol.
 */
union cvmx_gmxx_hg2_control {
	u64 u64;
	struct cvmx_gmxx_hg2_control_s {
		u64 reserved_19_63 : 45;
		u64 hg2tx_en : 1;
		u64 hg2rx_en : 1;
		u64 phys_en : 1;
		u64 logl_en : 16;
	} s;
	struct cvmx_gmxx_hg2_control_s cn52xx;
	struct cvmx_gmxx_hg2_control_s cn52xxp1;
	struct cvmx_gmxx_hg2_control_s cn56xx;
	struct cvmx_gmxx_hg2_control_s cn61xx;
	struct cvmx_gmxx_hg2_control_s cn63xx;
	struct cvmx_gmxx_hg2_control_s cn63xxp1;
	struct cvmx_gmxx_hg2_control_s cn66xx;
	struct cvmx_gmxx_hg2_control_s cn68xx;
	struct cvmx_gmxx_hg2_control_s cn68xxp1;
	struct cvmx_gmxx_hg2_control_s cn70xx;
	struct cvmx_gmxx_hg2_control_s cn70xxp1;
	struct cvmx_gmxx_hg2_control_s cnf71xx;
};

typedef union cvmx_gmxx_hg2_control cvmx_gmxx_hg2_control_t;

/**
 * cvmx_gmx#_inf_mode
 *
 * GMX_INF_MODE = Interface Mode
 *
 */
union cvmx_gmxx_inf_mode {
	u64 u64;
	struct cvmx_gmxx_inf_mode_s {
		u64 reserved_20_63 : 44;
		u64 rate : 4;
		u64 reserved_12_15 : 4;
		u64 speed : 4;
		u64 reserved_7_7 : 1;
		u64 mode : 3;
		u64 reserved_3_3 : 1;
		u64 p0mii : 1;
		u64 en : 1;
		u64 type : 1;
	} s;
	struct cvmx_gmxx_inf_mode_cn30xx {
		u64 reserved_3_63 : 61;
		u64 p0mii : 1;
		u64 en : 1;
		u64 type : 1;
	} cn30xx;
	struct cvmx_gmxx_inf_mode_cn31xx {
		u64 reserved_2_63 : 62;
		u64 en : 1;
		u64 type : 1;
	} cn31xx;
	struct cvmx_gmxx_inf_mode_cn31xx cn38xx;
	struct cvmx_gmxx_inf_mode_cn31xx cn38xxp2;
	struct cvmx_gmxx_inf_mode_cn30xx cn50xx;
	struct cvmx_gmxx_inf_mode_cn52xx {
		u64 reserved_10_63 : 54;
		u64 speed : 2;
		u64 reserved_6_7 : 2;
		u64 mode : 2;
		u64 reserved_2_3 : 2;
		u64 en : 1;
		u64 type : 1;
	} cn52xx;
	struct cvmx_gmxx_inf_mode_cn52xx cn52xxp1;
	struct cvmx_gmxx_inf_mode_cn52xx cn56xx;
	struct cvmx_gmxx_inf_mode_cn52xx cn56xxp1;
	struct cvmx_gmxx_inf_mode_cn31xx cn58xx;
	struct cvmx_gmxx_inf_mode_cn31xx cn58xxp1;
	struct cvmx_gmxx_inf_mode_cn61xx {
		u64 reserved_12_63 : 52;
		u64 speed : 4;
		u64 reserved_5_7 : 3;
		u64 mode : 1;
		u64 reserved_2_3 : 2;
		u64 en : 1;
		u64 type : 1;
	} cn61xx;
	struct cvmx_gmxx_inf_mode_cn61xx cn63xx;
	struct cvmx_gmxx_inf_mode_cn61xx cn63xxp1;
	struct cvmx_gmxx_inf_mode_cn66xx {
		u64 reserved_20_63 : 44;
		u64 rate : 4;
		u64 reserved_12_15 : 4;
		u64 speed : 4;
		u64 reserved_5_7 : 3;
		u64 mode : 1;
		u64 reserved_2_3 : 2;
		u64 en : 1;
		u64 type : 1;
	} cn66xx;
	struct cvmx_gmxx_inf_mode_cn68xx {
		u64 reserved_12_63 : 52;
		u64 speed : 4;
		u64 reserved_7_7 : 1;
		u64 mode : 3;
		u64 reserved_2_3 : 2;
		u64 en : 1;
		u64 type : 1;
	} cn68xx;
	struct cvmx_gmxx_inf_mode_cn68xx cn68xxp1;
	struct cvmx_gmxx_inf_mode_cn70xx {
		u64 reserved_6_63 : 58;
		u64 mode : 2;
		u64 reserved_2_3 : 2;
		u64 en : 1;
		u64 reserved_0_0 : 1;
	} cn70xx;
	struct cvmx_gmxx_inf_mode_cn70xx cn70xxp1;
	struct cvmx_gmxx_inf_mode_cn61xx cnf71xx;
};

typedef union cvmx_gmxx_inf_mode cvmx_gmxx_inf_mode_t;

/**
 * cvmx_gmx#_nxa_adr
 *
 * GMX_NXA_ADR = NXA Port Address
 *
 */
union cvmx_gmxx_nxa_adr {
	u64 u64;
	struct cvmx_gmxx_nxa_adr_s {
		u64 reserved_23_63 : 41;
		u64 pipe : 7;
		u64 reserved_6_15 : 10;
		u64 prt : 6;
	} s;
	struct cvmx_gmxx_nxa_adr_cn30xx {
		u64 reserved_6_63 : 58;
		u64 prt : 6;
	} cn30xx;
	struct cvmx_gmxx_nxa_adr_cn30xx cn31xx;
	struct cvmx_gmxx_nxa_adr_cn30xx cn38xx;
	struct cvmx_gmxx_nxa_adr_cn30xx cn38xxp2;
	struct cvmx_gmxx_nxa_adr_cn30xx cn50xx;
	struct cvmx_gmxx_nxa_adr_cn30xx cn52xx;
	struct cvmx_gmxx_nxa_adr_cn30xx cn52xxp1;
	struct cvmx_gmxx_nxa_adr_cn30xx cn56xx;
	struct cvmx_gmxx_nxa_adr_cn30xx cn56xxp1;
	struct cvmx_gmxx_nxa_adr_cn30xx cn58xx;
	struct cvmx_gmxx_nxa_adr_cn30xx cn58xxp1;
	struct cvmx_gmxx_nxa_adr_cn30xx cn61xx;
	struct cvmx_gmxx_nxa_adr_cn30xx cn63xx;
	struct cvmx_gmxx_nxa_adr_cn30xx cn63xxp1;
	struct cvmx_gmxx_nxa_adr_cn30xx cn66xx;
	struct cvmx_gmxx_nxa_adr_s cn68xx;
	struct cvmx_gmxx_nxa_adr_s cn68xxp1;
	struct cvmx_gmxx_nxa_adr_cn30xx cn70xx;
	struct cvmx_gmxx_nxa_adr_cn30xx cn70xxp1;
	struct cvmx_gmxx_nxa_adr_cn30xx cnf71xx;
};

typedef union cvmx_gmxx_nxa_adr cvmx_gmxx_nxa_adr_t;

/**
 * cvmx_gmx#_pipe_status
 *
 * DON'T PUT IN HRM*
 *
 */
union cvmx_gmxx_pipe_status {
	u64 u64;
	struct cvmx_gmxx_pipe_status_s {
		u64 reserved_20_63 : 44;
		u64 ovr : 4;
		u64 reserved_12_15 : 4;
		u64 bp : 4;
		u64 reserved_4_7 : 4;
		u64 stop : 4;
	} s;
	struct cvmx_gmxx_pipe_status_s cn68xx;
	struct cvmx_gmxx_pipe_status_s cn68xxp1;
};

typedef union cvmx_gmxx_pipe_status cvmx_gmxx_pipe_status_t;

/**
 * cvmx_gmx#_prt#_cbfc_ctl
 *
 * ** HG2 message CSRs end
 *
 */
union cvmx_gmxx_prtx_cbfc_ctl {
	u64 u64;
	struct cvmx_gmxx_prtx_cbfc_ctl_s {
		u64 phys_en : 16;
		u64 logl_en : 16;
		u64 phys_bp : 16;
		u64 reserved_4_15 : 12;
		u64 bck_en : 1;
		u64 drp_en : 1;
		u64 tx_en : 1;
		u64 rx_en : 1;
	} s;
	struct cvmx_gmxx_prtx_cbfc_ctl_s cn52xx;
	struct cvmx_gmxx_prtx_cbfc_ctl_s cn56xx;
	struct cvmx_gmxx_prtx_cbfc_ctl_s cn61xx;
	struct cvmx_gmxx_prtx_cbfc_ctl_s cn63xx;
	struct cvmx_gmxx_prtx_cbfc_ctl_s cn63xxp1;
	struct cvmx_gmxx_prtx_cbfc_ctl_s cn66xx;
	struct cvmx_gmxx_prtx_cbfc_ctl_s cn68xx;
	struct cvmx_gmxx_prtx_cbfc_ctl_s cn68xxp1;
	struct cvmx_gmxx_prtx_cbfc_ctl_s cn70xx;
	struct cvmx_gmxx_prtx_cbfc_ctl_s cn70xxp1;
	struct cvmx_gmxx_prtx_cbfc_ctl_s cnf71xx;
};

typedef union cvmx_gmxx_prtx_cbfc_ctl cvmx_gmxx_prtx_cbfc_ctl_t;

/**
 * cvmx_gmx#_prt#_cfg
 *
 * GMX_PRT_CFG = Port description
 *
 */
union cvmx_gmxx_prtx_cfg {
	u64 u64;
	struct cvmx_gmxx_prtx_cfg_s {
		u64 reserved_22_63 : 42;
		u64 pknd : 6;
		u64 reserved_14_15 : 2;
		u64 tx_idle : 1;
		u64 rx_idle : 1;
		u64 reserved_9_11 : 3;
		u64 speed_msb : 1;
		u64 reserved_4_7 : 4;
		u64 slottime : 1;
		u64 duplex : 1;
		u64 speed : 1;
		u64 en : 1;
	} s;
	struct cvmx_gmxx_prtx_cfg_cn30xx {
		u64 reserved_4_63 : 60;
		u64 slottime : 1;
		u64 duplex : 1;
		u64 speed : 1;
		u64 en : 1;
	} cn30xx;
	struct cvmx_gmxx_prtx_cfg_cn30xx cn31xx;
	struct cvmx_gmxx_prtx_cfg_cn30xx cn38xx;
	struct cvmx_gmxx_prtx_cfg_cn30xx cn38xxp2;
	struct cvmx_gmxx_prtx_cfg_cn30xx cn50xx;
	struct cvmx_gmxx_prtx_cfg_cn52xx {
		u64 reserved_14_63 : 50;
		u64 tx_idle : 1;
		u64 rx_idle : 1;
		u64 reserved_9_11 : 3;
		u64 speed_msb : 1;
		u64 reserved_4_7 : 4;
		u64 slottime : 1;
		u64 duplex : 1;
		u64 speed : 1;
		u64 en : 1;
	} cn52xx;
	struct cvmx_gmxx_prtx_cfg_cn52xx cn52xxp1;
	struct cvmx_gmxx_prtx_cfg_cn52xx cn56xx;
	struct cvmx_gmxx_prtx_cfg_cn52xx cn56xxp1;
	struct cvmx_gmxx_prtx_cfg_cn30xx cn58xx;
	struct cvmx_gmxx_prtx_cfg_cn30xx cn58xxp1;
	struct cvmx_gmxx_prtx_cfg_cn52xx cn61xx;
	struct cvmx_gmxx_prtx_cfg_cn52xx cn63xx;
	struct cvmx_gmxx_prtx_cfg_cn52xx cn63xxp1;
	struct cvmx_gmxx_prtx_cfg_cn52xx cn66xx;
	struct cvmx_gmxx_prtx_cfg_s cn68xx;
	struct cvmx_gmxx_prtx_cfg_s cn68xxp1;
	struct cvmx_gmxx_prtx_cfg_cn52xx cn70xx;
	struct cvmx_gmxx_prtx_cfg_cn52xx cn70xxp1;
	struct cvmx_gmxx_prtx_cfg_cn52xx cnf71xx;
};

typedef union cvmx_gmxx_prtx_cfg cvmx_gmxx_prtx_cfg_t;

/**
 * cvmx_gmx#_qsgmii_ctl
 */
union cvmx_gmxx_qsgmii_ctl {
	u64 u64;
	struct cvmx_gmxx_qsgmii_ctl_s {
		u64 reserved_1_63 : 63;
		u64 disparity : 1;
	} s;
	struct cvmx_gmxx_qsgmii_ctl_s cn70xx;
	struct cvmx_gmxx_qsgmii_ctl_s cn70xxp1;
};

typedef union cvmx_gmxx_qsgmii_ctl cvmx_gmxx_qsgmii_ctl_t;

/**
 * cvmx_gmx#_rx#_adr_cam0
 *
 * GMX_RX_ADR_CAM = Address Filtering Control
 *
 */
union cvmx_gmxx_rxx_adr_cam0 {
	u64 u64;
	struct cvmx_gmxx_rxx_adr_cam0_s {
		u64 adr : 64;
	} s;
	struct cvmx_gmxx_rxx_adr_cam0_s cn30xx;
	struct cvmx_gmxx_rxx_adr_cam0_s cn31xx;
	struct cvmx_gmxx_rxx_adr_cam0_s cn38xx;
	struct cvmx_gmxx_rxx_adr_cam0_s cn38xxp2;
	struct cvmx_gmxx_rxx_adr_cam0_s cn50xx;
	struct cvmx_gmxx_rxx_adr_cam0_s cn52xx;
	struct cvmx_gmxx_rxx_adr_cam0_s cn52xxp1;
	struct cvmx_gmxx_rxx_adr_cam0_s cn56xx;
	struct cvmx_gmxx_rxx_adr_cam0_s cn56xxp1;
	struct cvmx_gmxx_rxx_adr_cam0_s cn58xx;
	struct cvmx_gmxx_rxx_adr_cam0_s cn58xxp1;
	struct cvmx_gmxx_rxx_adr_cam0_s cn61xx;
	struct cvmx_gmxx_rxx_adr_cam0_s cn63xx;
	struct cvmx_gmxx_rxx_adr_cam0_s cn63xxp1;
	struct cvmx_gmxx_rxx_adr_cam0_s cn66xx;
	struct cvmx_gmxx_rxx_adr_cam0_s cn68xx;
	struct cvmx_gmxx_rxx_adr_cam0_s cn68xxp1;
	struct cvmx_gmxx_rxx_adr_cam0_s cn70xx;
	struct cvmx_gmxx_rxx_adr_cam0_s cn70xxp1;
	struct cvmx_gmxx_rxx_adr_cam0_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_adr_cam0 cvmx_gmxx_rxx_adr_cam0_t;

/**
 * cvmx_gmx#_rx#_adr_cam1
 *
 * GMX_RX_ADR_CAM = Address Filtering Control
 *
 */
union cvmx_gmxx_rxx_adr_cam1 {
	u64 u64;
	struct cvmx_gmxx_rxx_adr_cam1_s {
		u64 adr : 64;
	} s;
	struct cvmx_gmxx_rxx_adr_cam1_s cn30xx;
	struct cvmx_gmxx_rxx_adr_cam1_s cn31xx;
	struct cvmx_gmxx_rxx_adr_cam1_s cn38xx;
	struct cvmx_gmxx_rxx_adr_cam1_s cn38xxp2;
	struct cvmx_gmxx_rxx_adr_cam1_s cn50xx;
	struct cvmx_gmxx_rxx_adr_cam1_s cn52xx;
	struct cvmx_gmxx_rxx_adr_cam1_s cn52xxp1;
	struct cvmx_gmxx_rxx_adr_cam1_s cn56xx;
	struct cvmx_gmxx_rxx_adr_cam1_s cn56xxp1;
	struct cvmx_gmxx_rxx_adr_cam1_s cn58xx;
	struct cvmx_gmxx_rxx_adr_cam1_s cn58xxp1;
	struct cvmx_gmxx_rxx_adr_cam1_s cn61xx;
	struct cvmx_gmxx_rxx_adr_cam1_s cn63xx;
	struct cvmx_gmxx_rxx_adr_cam1_s cn63xxp1;
	struct cvmx_gmxx_rxx_adr_cam1_s cn66xx;
	struct cvmx_gmxx_rxx_adr_cam1_s cn68xx;
	struct cvmx_gmxx_rxx_adr_cam1_s cn68xxp1;
	struct cvmx_gmxx_rxx_adr_cam1_s cn70xx;
	struct cvmx_gmxx_rxx_adr_cam1_s cn70xxp1;
	struct cvmx_gmxx_rxx_adr_cam1_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_adr_cam1 cvmx_gmxx_rxx_adr_cam1_t;

/**
 * cvmx_gmx#_rx#_adr_cam2
 *
 * GMX_RX_ADR_CAM = Address Filtering Control
 *
 */
union cvmx_gmxx_rxx_adr_cam2 {
	u64 u64;
	struct cvmx_gmxx_rxx_adr_cam2_s {
		u64 adr : 64;
	} s;
	struct cvmx_gmxx_rxx_adr_cam2_s cn30xx;
	struct cvmx_gmxx_rxx_adr_cam2_s cn31xx;
	struct cvmx_gmxx_rxx_adr_cam2_s cn38xx;
	struct cvmx_gmxx_rxx_adr_cam2_s cn38xxp2;
	struct cvmx_gmxx_rxx_adr_cam2_s cn50xx;
	struct cvmx_gmxx_rxx_adr_cam2_s cn52xx;
	struct cvmx_gmxx_rxx_adr_cam2_s cn52xxp1;
	struct cvmx_gmxx_rxx_adr_cam2_s cn56xx;
	struct cvmx_gmxx_rxx_adr_cam2_s cn56xxp1;
	struct cvmx_gmxx_rxx_adr_cam2_s cn58xx;
	struct cvmx_gmxx_rxx_adr_cam2_s cn58xxp1;
	struct cvmx_gmxx_rxx_adr_cam2_s cn61xx;
	struct cvmx_gmxx_rxx_adr_cam2_s cn63xx;
	struct cvmx_gmxx_rxx_adr_cam2_s cn63xxp1;
	struct cvmx_gmxx_rxx_adr_cam2_s cn66xx;
	struct cvmx_gmxx_rxx_adr_cam2_s cn68xx;
	struct cvmx_gmxx_rxx_adr_cam2_s cn68xxp1;
	struct cvmx_gmxx_rxx_adr_cam2_s cn70xx;
	struct cvmx_gmxx_rxx_adr_cam2_s cn70xxp1;
	struct cvmx_gmxx_rxx_adr_cam2_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_adr_cam2 cvmx_gmxx_rxx_adr_cam2_t;

/**
 * cvmx_gmx#_rx#_adr_cam3
 *
 * GMX_RX_ADR_CAM = Address Filtering Control
 *
 */
union cvmx_gmxx_rxx_adr_cam3 {
	u64 u64;
	struct cvmx_gmxx_rxx_adr_cam3_s {
		u64 adr : 64;
	} s;
	struct cvmx_gmxx_rxx_adr_cam3_s cn30xx;
	struct cvmx_gmxx_rxx_adr_cam3_s cn31xx;
	struct cvmx_gmxx_rxx_adr_cam3_s cn38xx;
	struct cvmx_gmxx_rxx_adr_cam3_s cn38xxp2;
	struct cvmx_gmxx_rxx_adr_cam3_s cn50xx;
	struct cvmx_gmxx_rxx_adr_cam3_s cn52xx;
	struct cvmx_gmxx_rxx_adr_cam3_s cn52xxp1;
	struct cvmx_gmxx_rxx_adr_cam3_s cn56xx;
	struct cvmx_gmxx_rxx_adr_cam3_s cn56xxp1;
	struct cvmx_gmxx_rxx_adr_cam3_s cn58xx;
	struct cvmx_gmxx_rxx_adr_cam3_s cn58xxp1;
	struct cvmx_gmxx_rxx_adr_cam3_s cn61xx;
	struct cvmx_gmxx_rxx_adr_cam3_s cn63xx;
	struct cvmx_gmxx_rxx_adr_cam3_s cn63xxp1;
	struct cvmx_gmxx_rxx_adr_cam3_s cn66xx;
	struct cvmx_gmxx_rxx_adr_cam3_s cn68xx;
	struct cvmx_gmxx_rxx_adr_cam3_s cn68xxp1;
	struct cvmx_gmxx_rxx_adr_cam3_s cn70xx;
	struct cvmx_gmxx_rxx_adr_cam3_s cn70xxp1;
	struct cvmx_gmxx_rxx_adr_cam3_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_adr_cam3 cvmx_gmxx_rxx_adr_cam3_t;

/**
 * cvmx_gmx#_rx#_adr_cam4
 *
 * GMX_RX_ADR_CAM = Address Filtering Control
 *
 */
union cvmx_gmxx_rxx_adr_cam4 {
	u64 u64;
	struct cvmx_gmxx_rxx_adr_cam4_s {
		u64 adr : 64;
	} s;
	struct cvmx_gmxx_rxx_adr_cam4_s cn30xx;
	struct cvmx_gmxx_rxx_adr_cam4_s cn31xx;
	struct cvmx_gmxx_rxx_adr_cam4_s cn38xx;
	struct cvmx_gmxx_rxx_adr_cam4_s cn38xxp2;
	struct cvmx_gmxx_rxx_adr_cam4_s cn50xx;
	struct cvmx_gmxx_rxx_adr_cam4_s cn52xx;
	struct cvmx_gmxx_rxx_adr_cam4_s cn52xxp1;
	struct cvmx_gmxx_rxx_adr_cam4_s cn56xx;
	struct cvmx_gmxx_rxx_adr_cam4_s cn56xxp1;
	struct cvmx_gmxx_rxx_adr_cam4_s cn58xx;
	struct cvmx_gmxx_rxx_adr_cam4_s cn58xxp1;
	struct cvmx_gmxx_rxx_adr_cam4_s cn61xx;
	struct cvmx_gmxx_rxx_adr_cam4_s cn63xx;
	struct cvmx_gmxx_rxx_adr_cam4_s cn63xxp1;
	struct cvmx_gmxx_rxx_adr_cam4_s cn66xx;
	struct cvmx_gmxx_rxx_adr_cam4_s cn68xx;
	struct cvmx_gmxx_rxx_adr_cam4_s cn68xxp1;
	struct cvmx_gmxx_rxx_adr_cam4_s cn70xx;
	struct cvmx_gmxx_rxx_adr_cam4_s cn70xxp1;
	struct cvmx_gmxx_rxx_adr_cam4_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_adr_cam4 cvmx_gmxx_rxx_adr_cam4_t;

/**
 * cvmx_gmx#_rx#_adr_cam5
 *
 * GMX_RX_ADR_CAM = Address Filtering Control
 *
 */
union cvmx_gmxx_rxx_adr_cam5 {
	u64 u64;
	struct cvmx_gmxx_rxx_adr_cam5_s {
		u64 adr : 64;
	} s;
	struct cvmx_gmxx_rxx_adr_cam5_s cn30xx;
	struct cvmx_gmxx_rxx_adr_cam5_s cn31xx;
	struct cvmx_gmxx_rxx_adr_cam5_s cn38xx;
	struct cvmx_gmxx_rxx_adr_cam5_s cn38xxp2;
	struct cvmx_gmxx_rxx_adr_cam5_s cn50xx;
	struct cvmx_gmxx_rxx_adr_cam5_s cn52xx;
	struct cvmx_gmxx_rxx_adr_cam5_s cn52xxp1;
	struct cvmx_gmxx_rxx_adr_cam5_s cn56xx;
	struct cvmx_gmxx_rxx_adr_cam5_s cn56xxp1;
	struct cvmx_gmxx_rxx_adr_cam5_s cn58xx;
	struct cvmx_gmxx_rxx_adr_cam5_s cn58xxp1;
	struct cvmx_gmxx_rxx_adr_cam5_s cn61xx;
	struct cvmx_gmxx_rxx_adr_cam5_s cn63xx;
	struct cvmx_gmxx_rxx_adr_cam5_s cn63xxp1;
	struct cvmx_gmxx_rxx_adr_cam5_s cn66xx;
	struct cvmx_gmxx_rxx_adr_cam5_s cn68xx;
	struct cvmx_gmxx_rxx_adr_cam5_s cn68xxp1;
	struct cvmx_gmxx_rxx_adr_cam5_s cn70xx;
	struct cvmx_gmxx_rxx_adr_cam5_s cn70xxp1;
	struct cvmx_gmxx_rxx_adr_cam5_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_adr_cam5 cvmx_gmxx_rxx_adr_cam5_t;

/**
 * cvmx_gmx#_rx#_adr_cam_all_en
 *
 * GMX_RX_ADR_CAM_ALL_EN = Address Filtering Control Enable
 *
 */
union cvmx_gmxx_rxx_adr_cam_all_en {
	u64 u64;
	struct cvmx_gmxx_rxx_adr_cam_all_en_s {
		u64 reserved_32_63 : 32;
		u64 en : 32;
	} s;
	struct cvmx_gmxx_rxx_adr_cam_all_en_s cn61xx;
	struct cvmx_gmxx_rxx_adr_cam_all_en_s cn66xx;
	struct cvmx_gmxx_rxx_adr_cam_all_en_s cn68xx;
	struct cvmx_gmxx_rxx_adr_cam_all_en_s cn70xx;
	struct cvmx_gmxx_rxx_adr_cam_all_en_s cn70xxp1;
	struct cvmx_gmxx_rxx_adr_cam_all_en_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_adr_cam_all_en cvmx_gmxx_rxx_adr_cam_all_en_t;

/**
 * cvmx_gmx#_rx#_adr_cam_en
 *
 * GMX_RX_ADR_CAM_EN = Address Filtering Control Enable
 *
 */
union cvmx_gmxx_rxx_adr_cam_en {
	u64 u64;
	struct cvmx_gmxx_rxx_adr_cam_en_s {
		u64 reserved_8_63 : 56;
		u64 en : 8;
	} s;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn30xx;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn31xx;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn38xx;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn38xxp2;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn50xx;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn52xx;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn52xxp1;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn56xx;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn56xxp1;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn58xx;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn58xxp1;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn61xx;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn63xx;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn63xxp1;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn66xx;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn68xx;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn68xxp1;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn70xx;
	struct cvmx_gmxx_rxx_adr_cam_en_s cn70xxp1;
	struct cvmx_gmxx_rxx_adr_cam_en_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_adr_cam_en cvmx_gmxx_rxx_adr_cam_en_t;

/**
 * cvmx_gmx#_rx#_adr_ctl
 *
 * GMX_RX_ADR_CTL = Address Filtering Control
 *
 *
 * Notes:
 * * ALGORITHM
 *   Here is some pseudo code that represents the address filter behavior.
 *
 *      @verbatim
 *      bool dmac_addr_filter(uint8 prt, uint48 dmac) [
 *        ASSERT(prt >= 0 && prt <= 3);
 *        if (is_bcst(dmac))                               // broadcast accept
 *          return (GMX_RX[prt]_ADR_CTL[BCST] ? ACCEPT : REJECT);
 *        if (is_mcst(dmac) & GMX_RX[prt]_ADR_CTL[MCST] == 1)   // multicast reject
 *          return REJECT;
 *        if (is_mcst(dmac) & GMX_RX[prt]_ADR_CTL[MCST] == 2)   // multicast accept
 *          return ACCEPT;
 *
 *        cam_hit = 0;
 *
 *        for (i=0; i<32; i++) [
 *          if (GMX_RX[prt]_ADR_CAM_ALL_EN[EN<i>] == 0)
 *            continue;
 *          uint48 unswizzled_mac_adr = 0x0;
 *          for (j=5; j>=0; j--) [
 *             unswizzled_mac_adr = (unswizzled_mac_adr << 8) | GMX_RX[i>>3]_ADR_CAM[j][ADR<(i&7)*8+7:(i&7)*8>];
 *          ]
 *          if (unswizzled_mac_adr == dmac) [
 *            cam_hit = 1;
 *            break;
 *          ]
 *        ]
 *
 *        if (cam_hit)
 *          return (GMX_RX[prt]_ADR_CTL[CAM_MODE] ? ACCEPT : REJECT);
 *        else
 *          return (GMX_RX[prt]_ADR_CTL[CAM_MODE] ? REJECT : ACCEPT);
 *      ]
 *      @endverbatim
 *
 * * XAUI Mode
 *
 *   In XAUI mode, only GMX_RX0_ADR_CTL is used.  GMX_RX[1,2,3]_ADR_CTL should not be used.
 */
union cvmx_gmxx_rxx_adr_ctl {
	u64 u64;
	struct cvmx_gmxx_rxx_adr_ctl_s {
		u64 reserved_4_63 : 60;
		u64 cam_mode : 1;
		u64 mcst : 2;
		u64 bcst : 1;
	} s;
	struct cvmx_gmxx_rxx_adr_ctl_s cn30xx;
	struct cvmx_gmxx_rxx_adr_ctl_s cn31xx;
	struct cvmx_gmxx_rxx_adr_ctl_s cn38xx;
	struct cvmx_gmxx_rxx_adr_ctl_s cn38xxp2;
	struct cvmx_gmxx_rxx_adr_ctl_s cn50xx;
	struct cvmx_gmxx_rxx_adr_ctl_s cn52xx;
	struct cvmx_gmxx_rxx_adr_ctl_s cn52xxp1;
	struct cvmx_gmxx_rxx_adr_ctl_s cn56xx;
	struct cvmx_gmxx_rxx_adr_ctl_s cn56xxp1;
	struct cvmx_gmxx_rxx_adr_ctl_s cn58xx;
	struct cvmx_gmxx_rxx_adr_ctl_s cn58xxp1;
	struct cvmx_gmxx_rxx_adr_ctl_s cn61xx;
	struct cvmx_gmxx_rxx_adr_ctl_s cn63xx;
	struct cvmx_gmxx_rxx_adr_ctl_s cn63xxp1;
	struct cvmx_gmxx_rxx_adr_ctl_s cn66xx;
	struct cvmx_gmxx_rxx_adr_ctl_s cn68xx;
	struct cvmx_gmxx_rxx_adr_ctl_s cn68xxp1;
	struct cvmx_gmxx_rxx_adr_ctl_s cn70xx;
	struct cvmx_gmxx_rxx_adr_ctl_s cn70xxp1;
	struct cvmx_gmxx_rxx_adr_ctl_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_adr_ctl cvmx_gmxx_rxx_adr_ctl_t;

/**
 * cvmx_gmx#_rx#_decision
 *
 * GMX_RX_DECISION = The byte count to decide when to accept or filter a packet
 *
 *
 * Notes:
 * As each byte in a packet is received by GMX, the L2 byte count is compared
 * against the GMX_RX_DECISION[CNT].  The L2 byte count is the number of bytes
 * from the beginning of the L2 header (DMAC).  In normal operation, the L2
 * header begins after the PREAMBLE+SFD (GMX_RX_FRM_CTL[PRE_CHK]=1) and any
 * optional UDD skip data (GMX_RX_UDD_SKP[LEN]).
 *
 * When GMX_RX_FRM_CTL[PRE_CHK] is clear, PREAMBLE+SFD are prepended to the
 * packet and would require UDD skip length to account for them.
 *
 *                                                 L2 Size
 * Port Mode             <GMX_RX_DECISION bytes (default=24)       >=GMX_RX_DECISION bytes (default=24)
 *
 * Full Duplex           accept packet                             apply filters
 *                       no filtering is applied                   accept packet based on DMAC and PAUSE packet filters
 *
 * Half Duplex           drop packet                               apply filters
 *                       packet is unconditionally dropped         accept packet based on DMAC
 *
 * where l2_size = MAX(0, total_packet_size - GMX_RX_UDD_SKP[LEN] - ((GMX_RX_FRM_CTL[PRE_CHK]==1)*8)
 */
union cvmx_gmxx_rxx_decision {
	u64 u64;
	struct cvmx_gmxx_rxx_decision_s {
		u64 reserved_5_63 : 59;
		u64 cnt : 5;
	} s;
	struct cvmx_gmxx_rxx_decision_s cn30xx;
	struct cvmx_gmxx_rxx_decision_s cn31xx;
	struct cvmx_gmxx_rxx_decision_s cn38xx;
	struct cvmx_gmxx_rxx_decision_s cn38xxp2;
	struct cvmx_gmxx_rxx_decision_s cn50xx;
	struct cvmx_gmxx_rxx_decision_s cn52xx;
	struct cvmx_gmxx_rxx_decision_s cn52xxp1;
	struct cvmx_gmxx_rxx_decision_s cn56xx;
	struct cvmx_gmxx_rxx_decision_s cn56xxp1;
	struct cvmx_gmxx_rxx_decision_s cn58xx;
	struct cvmx_gmxx_rxx_decision_s cn58xxp1;
	struct cvmx_gmxx_rxx_decision_s cn61xx;
	struct cvmx_gmxx_rxx_decision_s cn63xx;
	struct cvmx_gmxx_rxx_decision_s cn63xxp1;
	struct cvmx_gmxx_rxx_decision_s cn66xx;
	struct cvmx_gmxx_rxx_decision_s cn68xx;
	struct cvmx_gmxx_rxx_decision_s cn68xxp1;
	struct cvmx_gmxx_rxx_decision_s cn70xx;
	struct cvmx_gmxx_rxx_decision_s cn70xxp1;
	struct cvmx_gmxx_rxx_decision_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_decision cvmx_gmxx_rxx_decision_t;

/**
 * cvmx_gmx#_rx#_frm_chk
 *
 * GMX_RX_FRM_CHK = Which frame errors will set the ERR bit of the frame
 *
 *
 * Notes:
 * If GMX_RX_UDD_SKP[LEN] != 0, then LENERR will be forced to zero in HW.
 *
 * In XAUI mode prt0 is used for checking.
 */
union cvmx_gmxx_rxx_frm_chk {
	u64 u64;
	struct cvmx_gmxx_rxx_frm_chk_s {
		u64 reserved_10_63 : 54;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} s;
	struct cvmx_gmxx_rxx_frm_chk_s cn30xx;
	struct cvmx_gmxx_rxx_frm_chk_s cn31xx;
	struct cvmx_gmxx_rxx_frm_chk_s cn38xx;
	struct cvmx_gmxx_rxx_frm_chk_s cn38xxp2;
	struct cvmx_gmxx_rxx_frm_chk_cn50xx {
		u64 reserved_10_63 : 54;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_6_6 : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 reserved_2_2 : 1;
		u64 carext : 1;
		u64 reserved_0_0 : 1;
	} cn50xx;
	struct cvmx_gmxx_rxx_frm_chk_cn52xx {
		u64 reserved_9_63 : 55;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_5_6 : 2;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 reserved_2_2 : 1;
		u64 carext : 1;
		u64 reserved_0_0 : 1;
	} cn52xx;
	struct cvmx_gmxx_rxx_frm_chk_cn52xx cn52xxp1;
	struct cvmx_gmxx_rxx_frm_chk_cn52xx cn56xx;
	struct cvmx_gmxx_rxx_frm_chk_cn52xx cn56xxp1;
	struct cvmx_gmxx_rxx_frm_chk_s cn58xx;
	struct cvmx_gmxx_rxx_frm_chk_s cn58xxp1;
	struct cvmx_gmxx_rxx_frm_chk_cn61xx {
		u64 reserved_9_63 : 55;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_5_6 : 2;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 reserved_2_2 : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} cn61xx;
	struct cvmx_gmxx_rxx_frm_chk_cn61xx cn63xx;
	struct cvmx_gmxx_rxx_frm_chk_cn61xx cn63xxp1;
	struct cvmx_gmxx_rxx_frm_chk_cn61xx cn66xx;
	struct cvmx_gmxx_rxx_frm_chk_cn61xx cn68xx;
	struct cvmx_gmxx_rxx_frm_chk_cn61xx cn68xxp1;
	struct cvmx_gmxx_rxx_frm_chk_cn61xx cn70xx;
	struct cvmx_gmxx_rxx_frm_chk_cn61xx cn70xxp1;
	struct cvmx_gmxx_rxx_frm_chk_cn61xx cnf71xx;
};

typedef union cvmx_gmxx_rxx_frm_chk cvmx_gmxx_rxx_frm_chk_t;

/**
 * cvmx_gmx#_rx#_frm_ctl
 *
 * GMX_RX_FRM_CTL = Frame Control
 *
 *
 * Notes:
 * * PRE_STRP
 *   When PRE_CHK is set (indicating that the PREAMBLE will be sent), PRE_STRP
 *   determines if the PREAMBLE+SFD bytes are thrown away or sent to the Octane
 *   core as part of the packet.
 *
 *   In either mode, the PREAMBLE+SFD bytes are not counted toward the packet
 *   size when checking against the MIN and MAX bounds.  Furthermore, the bytes
 *   are skipped when locating the start of the L2 header for DMAC and Control
 *   frame recognition.
 *
 * * CTL_BCK/CTL_DRP
 *   These bits control how the HW handles incoming PAUSE packets.  Here are
 *   the most common modes of operation:
 *     CTL_BCK=1,CTL_DRP=1   - HW does it all
 *     CTL_BCK=0,CTL_DRP=0   - SW sees all pause frames
 *     CTL_BCK=0,CTL_DRP=1   - all pause frames are completely ignored
 *
 *   These control bits should be set to CTL_BCK=0,CTL_DRP=0 in halfdup mode.
 *   Since PAUSE packets only apply to fulldup operation, any PAUSE packet
 *   would constitute an exception which should be handled by the processing
 *   cores.  PAUSE packets should not be forwarded.
 */
union cvmx_gmxx_rxx_frm_ctl {
	u64 u64;
	struct cvmx_gmxx_rxx_frm_ctl_s {
		u64 reserved_13_63 : 51;
		u64 ptp_mode : 1;
		u64 reserved_11_11 : 1;
		u64 null_dis : 1;
		u64 pre_align : 1;
		u64 pad_len : 1;
		u64 vlan_len : 1;
		u64 pre_free : 1;
		u64 ctl_smac : 1;
		u64 ctl_mcst : 1;
		u64 ctl_bck : 1;
		u64 ctl_drp : 1;
		u64 pre_strp : 1;
		u64 pre_chk : 1;
	} s;
	struct cvmx_gmxx_rxx_frm_ctl_cn30xx {
		u64 reserved_9_63 : 55;
		u64 pad_len : 1;
		u64 vlan_len : 1;
		u64 pre_free : 1;
		u64 ctl_smac : 1;
		u64 ctl_mcst : 1;
		u64 ctl_bck : 1;
		u64 ctl_drp : 1;
		u64 pre_strp : 1;
		u64 pre_chk : 1;
	} cn30xx;
	struct cvmx_gmxx_rxx_frm_ctl_cn31xx {
		u64 reserved_8_63 : 56;
		u64 vlan_len : 1;
		u64 pre_free : 1;
		u64 ctl_smac : 1;
		u64 ctl_mcst : 1;
		u64 ctl_bck : 1;
		u64 ctl_drp : 1;
		u64 pre_strp : 1;
		u64 pre_chk : 1;
	} cn31xx;
	struct cvmx_gmxx_rxx_frm_ctl_cn30xx cn38xx;
	struct cvmx_gmxx_rxx_frm_ctl_cn31xx cn38xxp2;
	struct cvmx_gmxx_rxx_frm_ctl_cn50xx {
		u64 reserved_11_63 : 53;
		u64 null_dis : 1;
		u64 pre_align : 1;
		u64 reserved_7_8 : 2;
		u64 pre_free : 1;
		u64 ctl_smac : 1;
		u64 ctl_mcst : 1;
		u64 ctl_bck : 1;
		u64 ctl_drp : 1;
		u64 pre_strp : 1;
		u64 pre_chk : 1;
	} cn50xx;
	struct cvmx_gmxx_rxx_frm_ctl_cn50xx cn52xx;
	struct cvmx_gmxx_rxx_frm_ctl_cn50xx cn52xxp1;
	struct cvmx_gmxx_rxx_frm_ctl_cn50xx cn56xx;
	struct cvmx_gmxx_rxx_frm_ctl_cn56xxp1 {
		u64 reserved_10_63 : 54;
		u64 pre_align : 1;
		u64 reserved_7_8 : 2;
		u64 pre_free : 1;
		u64 ctl_smac : 1;
		u64 ctl_mcst : 1;
		u64 ctl_bck : 1;
		u64 ctl_drp : 1;
		u64 pre_strp : 1;
		u64 pre_chk : 1;
	} cn56xxp1;
	struct cvmx_gmxx_rxx_frm_ctl_cn58xx {
		u64 reserved_11_63 : 53;
		u64 null_dis : 1;
		u64 pre_align : 1;
		u64 pad_len : 1;
		u64 vlan_len : 1;
		u64 pre_free : 1;
		u64 ctl_smac : 1;
		u64 ctl_mcst : 1;
		u64 ctl_bck : 1;
		u64 ctl_drp : 1;
		u64 pre_strp : 1;
		u64 pre_chk : 1;
	} cn58xx;
	struct cvmx_gmxx_rxx_frm_ctl_cn30xx cn58xxp1;
	struct cvmx_gmxx_rxx_frm_ctl_cn61xx {
		u64 reserved_13_63 : 51;
		u64 ptp_mode : 1;
		u64 reserved_11_11 : 1;
		u64 null_dis : 1;
		u64 pre_align : 1;
		u64 reserved_7_8 : 2;
		u64 pre_free : 1;
		u64 ctl_smac : 1;
		u64 ctl_mcst : 1;
		u64 ctl_bck : 1;
		u64 ctl_drp : 1;
		u64 pre_strp : 1;
		u64 pre_chk : 1;
	} cn61xx;
	struct cvmx_gmxx_rxx_frm_ctl_cn61xx cn63xx;
	struct cvmx_gmxx_rxx_frm_ctl_cn61xx cn63xxp1;
	struct cvmx_gmxx_rxx_frm_ctl_cn61xx cn66xx;
	struct cvmx_gmxx_rxx_frm_ctl_cn61xx cn68xx;
	struct cvmx_gmxx_rxx_frm_ctl_cn61xx cn68xxp1;
	struct cvmx_gmxx_rxx_frm_ctl_cn61xx cn70xx;
	struct cvmx_gmxx_rxx_frm_ctl_cn61xx cn70xxp1;
	struct cvmx_gmxx_rxx_frm_ctl_cn61xx cnf71xx;
};

typedef union cvmx_gmxx_rxx_frm_ctl cvmx_gmxx_rxx_frm_ctl_t;

/**
 * cvmx_gmx#_rx#_frm_max
 *
 * GMX_RX_FRM_MAX = Frame Max length
 *
 *
 * Notes:
 * In spi4 mode, all spi4 ports use prt0 for checking.
 *
 * When changing the LEN field, be sure that LEN does not exceed
 * GMX_RX_JABBER[CNT]. Failure to meet this constraint will cause packets that
 * are within the maximum length parameter to be rejected because they exceed
 * the GMX_RX_JABBER[CNT] limit.
 */
union cvmx_gmxx_rxx_frm_max {
	u64 u64;
	struct cvmx_gmxx_rxx_frm_max_s {
		u64 reserved_16_63 : 48;
		u64 len : 16;
	} s;
	struct cvmx_gmxx_rxx_frm_max_s cn30xx;
	struct cvmx_gmxx_rxx_frm_max_s cn31xx;
	struct cvmx_gmxx_rxx_frm_max_s cn38xx;
	struct cvmx_gmxx_rxx_frm_max_s cn38xxp2;
	struct cvmx_gmxx_rxx_frm_max_s cn58xx;
	struct cvmx_gmxx_rxx_frm_max_s cn58xxp1;
};

typedef union cvmx_gmxx_rxx_frm_max cvmx_gmxx_rxx_frm_max_t;

/**
 * cvmx_gmx#_rx#_frm_min
 *
 * GMX_RX_FRM_MIN = Frame Min length
 *
 *
 * Notes:
 * In spi4 mode, all spi4 ports use prt0 for checking.
 *
 */
union cvmx_gmxx_rxx_frm_min {
	u64 u64;
	struct cvmx_gmxx_rxx_frm_min_s {
		u64 reserved_16_63 : 48;
		u64 len : 16;
	} s;
	struct cvmx_gmxx_rxx_frm_min_s cn30xx;
	struct cvmx_gmxx_rxx_frm_min_s cn31xx;
	struct cvmx_gmxx_rxx_frm_min_s cn38xx;
	struct cvmx_gmxx_rxx_frm_min_s cn38xxp2;
	struct cvmx_gmxx_rxx_frm_min_s cn58xx;
	struct cvmx_gmxx_rxx_frm_min_s cn58xxp1;
};

typedef union cvmx_gmxx_rxx_frm_min cvmx_gmxx_rxx_frm_min_t;

/**
 * cvmx_gmx#_rx#_ifg
 *
 * GMX_RX_IFG = RX Min IFG
 *
 */
union cvmx_gmxx_rxx_ifg {
	u64 u64;
	struct cvmx_gmxx_rxx_ifg_s {
		u64 reserved_4_63 : 60;
		u64 ifg : 4;
	} s;
	struct cvmx_gmxx_rxx_ifg_s cn30xx;
	struct cvmx_gmxx_rxx_ifg_s cn31xx;
	struct cvmx_gmxx_rxx_ifg_s cn38xx;
	struct cvmx_gmxx_rxx_ifg_s cn38xxp2;
	struct cvmx_gmxx_rxx_ifg_s cn50xx;
	struct cvmx_gmxx_rxx_ifg_s cn52xx;
	struct cvmx_gmxx_rxx_ifg_s cn52xxp1;
	struct cvmx_gmxx_rxx_ifg_s cn56xx;
	struct cvmx_gmxx_rxx_ifg_s cn56xxp1;
	struct cvmx_gmxx_rxx_ifg_s cn58xx;
	struct cvmx_gmxx_rxx_ifg_s cn58xxp1;
	struct cvmx_gmxx_rxx_ifg_s cn61xx;
	struct cvmx_gmxx_rxx_ifg_s cn63xx;
	struct cvmx_gmxx_rxx_ifg_s cn63xxp1;
	struct cvmx_gmxx_rxx_ifg_s cn66xx;
	struct cvmx_gmxx_rxx_ifg_s cn68xx;
	struct cvmx_gmxx_rxx_ifg_s cn68xxp1;
	struct cvmx_gmxx_rxx_ifg_s cn70xx;
	struct cvmx_gmxx_rxx_ifg_s cn70xxp1;
	struct cvmx_gmxx_rxx_ifg_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_ifg cvmx_gmxx_rxx_ifg_t;

/**
 * cvmx_gmx#_rx#_int_en
 *
 * GMX_RX_INT_EN = Interrupt Enable
 *
 *
 * Notes:
 * In XAUI mode prt0 is used for checking.
 *
 */
union cvmx_gmxx_rxx_int_en {
	u64 u64;
	struct cvmx_gmxx_rxx_int_en_s {
		u64 reserved_30_63 : 34;
		u64 wol : 1;
		u64 hg2cc : 1;
		u64 hg2fld : 1;
		u64 undat : 1;
		u64 uneop : 1;
		u64 unsop : 1;
		u64 bad_term : 1;
		u64 bad_seq : 1;
		u64 rem_fault : 1;
		u64 loc_fault : 1;
		u64 pause_drp : 1;
		u64 phy_dupx : 1;
		u64 phy_spd : 1;
		u64 phy_link : 1;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} s;
	struct cvmx_gmxx_rxx_int_en_cn30xx {
		u64 reserved_19_63 : 45;
		u64 phy_dupx : 1;
		u64 phy_spd : 1;
		u64 phy_link : 1;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} cn30xx;
	struct cvmx_gmxx_rxx_int_en_cn30xx cn31xx;
	struct cvmx_gmxx_rxx_int_en_cn30xx cn38xx;
	struct cvmx_gmxx_rxx_int_en_cn30xx cn38xxp2;
	struct cvmx_gmxx_rxx_int_en_cn50xx {
		u64 reserved_20_63 : 44;
		u64 pause_drp : 1;
		u64 phy_dupx : 1;
		u64 phy_spd : 1;
		u64 phy_link : 1;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_6_6 : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 reserved_2_2 : 1;
		u64 carext : 1;
		u64 reserved_0_0 : 1;
	} cn50xx;
	struct cvmx_gmxx_rxx_int_en_cn52xx {
		u64 reserved_29_63 : 35;
		u64 hg2cc : 1;
		u64 hg2fld : 1;
		u64 undat : 1;
		u64 uneop : 1;
		u64 unsop : 1;
		u64 bad_term : 1;
		u64 bad_seq : 1;
		u64 rem_fault : 1;
		u64 loc_fault : 1;
		u64 pause_drp : 1;
		u64 reserved_16_18 : 3;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 reserved_9_9 : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_5_6 : 2;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 reserved_2_2 : 1;
		u64 carext : 1;
		u64 reserved_0_0 : 1;
	} cn52xx;
	struct cvmx_gmxx_rxx_int_en_cn52xx cn52xxp1;
	struct cvmx_gmxx_rxx_int_en_cn52xx cn56xx;
	struct cvmx_gmxx_rxx_int_en_cn56xxp1 {
		u64 reserved_27_63 : 37;
		u64 undat : 1;
		u64 uneop : 1;
		u64 unsop : 1;
		u64 bad_term : 1;
		u64 bad_seq : 1;
		u64 rem_fault : 1;
		u64 loc_fault : 1;
		u64 pause_drp : 1;
		u64 reserved_16_18 : 3;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 reserved_9_9 : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_5_6 : 2;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 reserved_2_2 : 1;
		u64 carext : 1;
		u64 reserved_0_0 : 1;
	} cn56xxp1;
	struct cvmx_gmxx_rxx_int_en_cn58xx {
		u64 reserved_20_63 : 44;
		u64 pause_drp : 1;
		u64 phy_dupx : 1;
		u64 phy_spd : 1;
		u64 phy_link : 1;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} cn58xx;
	struct cvmx_gmxx_rxx_int_en_cn58xx cn58xxp1;
	struct cvmx_gmxx_rxx_int_en_cn61xx {
		u64 reserved_29_63 : 35;
		u64 hg2cc : 1;
		u64 hg2fld : 1;
		u64 undat : 1;
		u64 uneop : 1;
		u64 unsop : 1;
		u64 bad_term : 1;
		u64 bad_seq : 1;
		u64 rem_fault : 1;
		u64 loc_fault : 1;
		u64 pause_drp : 1;
		u64 reserved_16_18 : 3;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 reserved_9_9 : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_5_6 : 2;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 reserved_2_2 : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} cn61xx;
	struct cvmx_gmxx_rxx_int_en_cn61xx cn63xx;
	struct cvmx_gmxx_rxx_int_en_cn61xx cn63xxp1;
	struct cvmx_gmxx_rxx_int_en_cn61xx cn66xx;
	struct cvmx_gmxx_rxx_int_en_cn61xx cn68xx;
	struct cvmx_gmxx_rxx_int_en_cn61xx cn68xxp1;
	struct cvmx_gmxx_rxx_int_en_cn70xx {
		u64 reserved_30_63 : 34;
		u64 wol : 1;
		u64 hg2cc : 1;
		u64 hg2fld : 1;
		u64 undat : 1;
		u64 uneop : 1;
		u64 unsop : 1;
		u64 bad_term : 1;
		u64 bad_seq : 1;
		u64 rem_fault : 1;
		u64 loc_fault : 1;
		u64 pause_drp : 1;
		u64 reserved_16_18 : 3;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 reserved_9_9 : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_5_6 : 2;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 reserved_2_2 : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} cn70xx;
	struct cvmx_gmxx_rxx_int_en_cn70xx cn70xxp1;
	struct cvmx_gmxx_rxx_int_en_cn61xx cnf71xx;
};

typedef union cvmx_gmxx_rxx_int_en cvmx_gmxx_rxx_int_en_t;

/**
 * cvmx_gmx#_rx#_int_reg
 *
 * GMX_RX_INT_REG = Interrupt Register
 *
 *
 * Notes:
 * (1) exceptions will only be raised to the control processor if the
 *     corresponding bit in the GMX_RX_INT_EN register is set.
 *
 * (2) exception conditions 10:0 can also set the rcv/opcode in the received
 *     packet's workQ entry.  The GMX_RX_FRM_CHK register provides a bit mask
 *     for configuring which conditions set the error.
 *
 * (3) in half duplex operation, the expectation is that collisions will appear
 *     as either MINERR o r CAREXT errors.
 *
 * (4) JABBER - An RX Jabber error indicates that a packet was received which
 *              is longer than the maximum allowed packet as defined by the
 *              system.  GMX will truncate the packet at the JABBER count.
 *              Failure to do so could lead to system instabilty.
 *
 * (5) NIBERR - This error is illegal at 1000Mbs speeds
 *              (GMX_RX_PRT_CFG[SPEED]==0) and will never assert.
 *
 * (6) MAXERR - for untagged frames, the total frame DA+SA+TL+DATA+PAD+FCS >
 *              GMX_RX_FRM_MAX.  For tagged frames, DA+SA+VLAN+TL+DATA+PAD+FCS
 *              > GMX_RX_FRM_MAX + 4*VLAN_VAL + 4*VLAN_STACKED.
 *
 * (7) MINERR - total frame DA+SA+TL+DATA+PAD+FCS < 64
 *
 * (8) ALNERR - Indicates that the packet received was not an integer number of
 *              bytes.  If FCS checking is enabled, ALNERR will only assert if
 *              the FCS is bad.  If FCS checking is disabled, ALNERR will
 *              assert in all non-integer frame cases.
 *
 * (9) Collisions - Collisions can only occur in half-duplex mode.  A collision
 *                  is assumed by the receiver when the slottime
 *                  (GMX_PRT_CFG[SLOTTIME]) is not satisfied.  In 10/100 mode,
 *                  this will result in a frame < SLOTTIME.  In 1000 mode, it
 *                  could result either in frame < SLOTTIME or a carrier extend
 *                  error with the SLOTTIME.  These conditions are visible by...
 *
 *                  . transfer ended before slottime - COLDET
 *                  . carrier extend error           - CAREXT
 *
 * (A) LENERR - Length errors occur when the received packet does not match the
 *              length field.  LENERR is only checked for packets between 64
 *              and 1500 bytes.  For untagged frames, the length must exact
 *              match.  For tagged frames the length or length+4 must match.
 *
 * (B) PCTERR - checks that the frame begins with a valid PREAMBLE sequence.
 *              Does not check the number of PREAMBLE cycles.
 *
 * (C) OVRERR -
 *
 *              OVRERR is an architectural assertion check internal to GMX to
 *              make sure no assumption was violated.  In a correctly operating
 *              system, this interrupt can never fire.
 *
 *              GMX has an internal arbiter which selects which of 4 ports to
 *              buffer in the main RX FIFO.  If we normally buffer 8 bytes,
 *              then each port will typically push a tick every 8 cycles - if
 *              the packet interface is going as fast as possible.  If there
 *              are four ports, they push every two cycles.  So that's the
 *              assumption.  That the inbound module will always be able to
 *              consume the tick before another is produced.  If that doesn't
 *              happen - that's when OVRERR will assert.
 *
 * (D) In XAUI mode prt0 is used for interrupt logging.
 */
union cvmx_gmxx_rxx_int_reg {
	u64 u64;
	struct cvmx_gmxx_rxx_int_reg_s {
		u64 reserved_30_63 : 34;
		u64 wol : 1;
		u64 hg2cc : 1;
		u64 hg2fld : 1;
		u64 undat : 1;
		u64 uneop : 1;
		u64 unsop : 1;
		u64 bad_term : 1;
		u64 bad_seq : 1;
		u64 rem_fault : 1;
		u64 loc_fault : 1;
		u64 pause_drp : 1;
		u64 phy_dupx : 1;
		u64 phy_spd : 1;
		u64 phy_link : 1;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} s;
	struct cvmx_gmxx_rxx_int_reg_cn30xx {
		u64 reserved_19_63 : 45;
		u64 phy_dupx : 1;
		u64 phy_spd : 1;
		u64 phy_link : 1;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} cn30xx;
	struct cvmx_gmxx_rxx_int_reg_cn30xx cn31xx;
	struct cvmx_gmxx_rxx_int_reg_cn30xx cn38xx;
	struct cvmx_gmxx_rxx_int_reg_cn30xx cn38xxp2;
	struct cvmx_gmxx_rxx_int_reg_cn50xx {
		u64 reserved_20_63 : 44;
		u64 pause_drp : 1;
		u64 phy_dupx : 1;
		u64 phy_spd : 1;
		u64 phy_link : 1;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_6_6 : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 reserved_2_2 : 1;
		u64 carext : 1;
		u64 reserved_0_0 : 1;
	} cn50xx;
	struct cvmx_gmxx_rxx_int_reg_cn52xx {
		u64 reserved_29_63 : 35;
		u64 hg2cc : 1;
		u64 hg2fld : 1;
		u64 undat : 1;
		u64 uneop : 1;
		u64 unsop : 1;
		u64 bad_term : 1;
		u64 bad_seq : 1;
		u64 rem_fault : 1;
		u64 loc_fault : 1;
		u64 pause_drp : 1;
		u64 reserved_16_18 : 3;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 reserved_9_9 : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_5_6 : 2;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 reserved_2_2 : 1;
		u64 carext : 1;
		u64 reserved_0_0 : 1;
	} cn52xx;
	struct cvmx_gmxx_rxx_int_reg_cn52xx cn52xxp1;
	struct cvmx_gmxx_rxx_int_reg_cn52xx cn56xx;
	struct cvmx_gmxx_rxx_int_reg_cn56xxp1 {
		u64 reserved_27_63 : 37;
		u64 undat : 1;
		u64 uneop : 1;
		u64 unsop : 1;
		u64 bad_term : 1;
		u64 bad_seq : 1;
		u64 rem_fault : 1;
		u64 loc_fault : 1;
		u64 pause_drp : 1;
		u64 reserved_16_18 : 3;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 reserved_9_9 : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_5_6 : 2;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 reserved_2_2 : 1;
		u64 carext : 1;
		u64 reserved_0_0 : 1;
	} cn56xxp1;
	struct cvmx_gmxx_rxx_int_reg_cn58xx {
		u64 reserved_20_63 : 44;
		u64 pause_drp : 1;
		u64 phy_dupx : 1;
		u64 phy_spd : 1;
		u64 phy_link : 1;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} cn58xx;
	struct cvmx_gmxx_rxx_int_reg_cn58xx cn58xxp1;
	struct cvmx_gmxx_rxx_int_reg_cn61xx {
		u64 reserved_29_63 : 35;
		u64 hg2cc : 1;
		u64 hg2fld : 1;
		u64 undat : 1;
		u64 uneop : 1;
		u64 unsop : 1;
		u64 bad_term : 1;
		u64 bad_seq : 1;
		u64 rem_fault : 1;
		u64 loc_fault : 1;
		u64 pause_drp : 1;
		u64 reserved_16_18 : 3;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 reserved_9_9 : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_5_6 : 2;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 reserved_2_2 : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} cn61xx;
	struct cvmx_gmxx_rxx_int_reg_cn61xx cn63xx;
	struct cvmx_gmxx_rxx_int_reg_cn61xx cn63xxp1;
	struct cvmx_gmxx_rxx_int_reg_cn61xx cn66xx;
	struct cvmx_gmxx_rxx_int_reg_cn61xx cn68xx;
	struct cvmx_gmxx_rxx_int_reg_cn61xx cn68xxp1;
	struct cvmx_gmxx_rxx_int_reg_cn70xx {
		u64 reserved_30_63 : 34;
		u64 wol : 1;
		u64 hg2cc : 1;
		u64 hg2fld : 1;
		u64 undat : 1;
		u64 uneop : 1;
		u64 unsop : 1;
		u64 bad_term : 1;
		u64 bad_seq : 1;
		u64 rem_fault : 1;
		u64 loc_fault : 1;
		u64 pause_drp : 1;
		u64 reserved_16_18 : 3;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 reserved_9_9 : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_5_6 : 2;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 reserved_2_2 : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} cn70xx;
	struct cvmx_gmxx_rxx_int_reg_cn70xx cn70xxp1;
	struct cvmx_gmxx_rxx_int_reg_cn61xx cnf71xx;
};

typedef union cvmx_gmxx_rxx_int_reg cvmx_gmxx_rxx_int_reg_t;

/**
 * cvmx_gmx#_rx#_jabber
 *
 * GMX_RX_JABBER = The max size packet after which GMX will truncate
 *
 *
 * Notes:
 * CNT must be 8-byte aligned such that CNT[2:0] == 0
 *
 * The packet that will be sent to the packet input logic will have an
 * additionl 8 bytes if GMX_RX_FRM_CTL[PRE_CHK] is set and
 * GMX_RX_FRM_CTL[PRE_STRP] is clear.  The max packet that will be sent is
 * defined as...
 *
 *      max_sized_packet = GMX_RX_JABBER[CNT]+((GMX_RX_FRM_CTL[PRE_CHK] & !GMX_RX_FRM_CTL[PRE_STRP])*8)
 *
 * In XAUI mode prt0 is used for checking.
 */
union cvmx_gmxx_rxx_jabber {
	u64 u64;
	struct cvmx_gmxx_rxx_jabber_s {
		u64 reserved_16_63 : 48;
		u64 cnt : 16;
	} s;
	struct cvmx_gmxx_rxx_jabber_s cn30xx;
	struct cvmx_gmxx_rxx_jabber_s cn31xx;
	struct cvmx_gmxx_rxx_jabber_s cn38xx;
	struct cvmx_gmxx_rxx_jabber_s cn38xxp2;
	struct cvmx_gmxx_rxx_jabber_s cn50xx;
	struct cvmx_gmxx_rxx_jabber_s cn52xx;
	struct cvmx_gmxx_rxx_jabber_s cn52xxp1;
	struct cvmx_gmxx_rxx_jabber_s cn56xx;
	struct cvmx_gmxx_rxx_jabber_s cn56xxp1;
	struct cvmx_gmxx_rxx_jabber_s cn58xx;
	struct cvmx_gmxx_rxx_jabber_s cn58xxp1;
	struct cvmx_gmxx_rxx_jabber_s cn61xx;
	struct cvmx_gmxx_rxx_jabber_s cn63xx;
	struct cvmx_gmxx_rxx_jabber_s cn63xxp1;
	struct cvmx_gmxx_rxx_jabber_s cn66xx;
	struct cvmx_gmxx_rxx_jabber_s cn68xx;
	struct cvmx_gmxx_rxx_jabber_s cn68xxp1;
	struct cvmx_gmxx_rxx_jabber_s cn70xx;
	struct cvmx_gmxx_rxx_jabber_s cn70xxp1;
	struct cvmx_gmxx_rxx_jabber_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_jabber cvmx_gmxx_rxx_jabber_t;

/**
 * cvmx_gmx#_rx#_pause_drop_time
 *
 * GMX_RX_PAUSE_DROP_TIME = The TIME field in a PAUSE Packet which was dropped due to GMX RX FIFO full condition
 *
 */
union cvmx_gmxx_rxx_pause_drop_time {
	u64 u64;
	struct cvmx_gmxx_rxx_pause_drop_time_s {
		u64 reserved_16_63 : 48;
		u64 status : 16;
	} s;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn50xx;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn52xx;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn52xxp1;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn56xx;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn56xxp1;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn58xx;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn58xxp1;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn61xx;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn63xx;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn63xxp1;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn66xx;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn68xx;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn68xxp1;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn70xx;
	struct cvmx_gmxx_rxx_pause_drop_time_s cn70xxp1;
	struct cvmx_gmxx_rxx_pause_drop_time_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_pause_drop_time cvmx_gmxx_rxx_pause_drop_time_t;

/**
 * cvmx_gmx#_rx#_rx_inbnd
 *
 * GMX_RX_INBND = RGMII InBand Link Status
 *
 *
 * Notes:
 * These fields are only valid if the attached PHY is operating in RGMII mode
 * and supports the optional in-band status (see section 3.4.1 of the RGMII
 * specification, version 1.3 for more information).
 */
union cvmx_gmxx_rxx_rx_inbnd {
	u64 u64;
	struct cvmx_gmxx_rxx_rx_inbnd_s {
		u64 reserved_4_63 : 60;
		u64 duplex : 1;
		u64 speed : 2;
		u64 status : 1;
	} s;
	struct cvmx_gmxx_rxx_rx_inbnd_s cn30xx;
	struct cvmx_gmxx_rxx_rx_inbnd_s cn31xx;
	struct cvmx_gmxx_rxx_rx_inbnd_s cn38xx;
	struct cvmx_gmxx_rxx_rx_inbnd_s cn38xxp2;
	struct cvmx_gmxx_rxx_rx_inbnd_s cn50xx;
	struct cvmx_gmxx_rxx_rx_inbnd_s cn58xx;
	struct cvmx_gmxx_rxx_rx_inbnd_s cn58xxp1;
};

typedef union cvmx_gmxx_rxx_rx_inbnd cvmx_gmxx_rxx_rx_inbnd_t;

/**
 * cvmx_gmx#_rx#_stats_ctl
 *
 * GMX_RX_STATS_CTL = RX Stats Control register
 *
 */
union cvmx_gmxx_rxx_stats_ctl {
	u64 u64;
	struct cvmx_gmxx_rxx_stats_ctl_s {
		u64 reserved_1_63 : 63;
		u64 rd_clr : 1;
	} s;
	struct cvmx_gmxx_rxx_stats_ctl_s cn30xx;
	struct cvmx_gmxx_rxx_stats_ctl_s cn31xx;
	struct cvmx_gmxx_rxx_stats_ctl_s cn38xx;
	struct cvmx_gmxx_rxx_stats_ctl_s cn38xxp2;
	struct cvmx_gmxx_rxx_stats_ctl_s cn50xx;
	struct cvmx_gmxx_rxx_stats_ctl_s cn52xx;
	struct cvmx_gmxx_rxx_stats_ctl_s cn52xxp1;
	struct cvmx_gmxx_rxx_stats_ctl_s cn56xx;
	struct cvmx_gmxx_rxx_stats_ctl_s cn56xxp1;
	struct cvmx_gmxx_rxx_stats_ctl_s cn58xx;
	struct cvmx_gmxx_rxx_stats_ctl_s cn58xxp1;
	struct cvmx_gmxx_rxx_stats_ctl_s cn61xx;
	struct cvmx_gmxx_rxx_stats_ctl_s cn63xx;
	struct cvmx_gmxx_rxx_stats_ctl_s cn63xxp1;
	struct cvmx_gmxx_rxx_stats_ctl_s cn66xx;
	struct cvmx_gmxx_rxx_stats_ctl_s cn68xx;
	struct cvmx_gmxx_rxx_stats_ctl_s cn68xxp1;
	struct cvmx_gmxx_rxx_stats_ctl_s cn70xx;
	struct cvmx_gmxx_rxx_stats_ctl_s cn70xxp1;
	struct cvmx_gmxx_rxx_stats_ctl_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_stats_ctl cvmx_gmxx_rxx_stats_ctl_t;

/**
 * cvmx_gmx#_rx#_stats_octs
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when GMX_RX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 */
union cvmx_gmxx_rxx_stats_octs {
	u64 u64;
	struct cvmx_gmxx_rxx_stats_octs_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_gmxx_rxx_stats_octs_s cn30xx;
	struct cvmx_gmxx_rxx_stats_octs_s cn31xx;
	struct cvmx_gmxx_rxx_stats_octs_s cn38xx;
	struct cvmx_gmxx_rxx_stats_octs_s cn38xxp2;
	struct cvmx_gmxx_rxx_stats_octs_s cn50xx;
	struct cvmx_gmxx_rxx_stats_octs_s cn52xx;
	struct cvmx_gmxx_rxx_stats_octs_s cn52xxp1;
	struct cvmx_gmxx_rxx_stats_octs_s cn56xx;
	struct cvmx_gmxx_rxx_stats_octs_s cn56xxp1;
	struct cvmx_gmxx_rxx_stats_octs_s cn58xx;
	struct cvmx_gmxx_rxx_stats_octs_s cn58xxp1;
	struct cvmx_gmxx_rxx_stats_octs_s cn61xx;
	struct cvmx_gmxx_rxx_stats_octs_s cn63xx;
	struct cvmx_gmxx_rxx_stats_octs_s cn63xxp1;
	struct cvmx_gmxx_rxx_stats_octs_s cn66xx;
	struct cvmx_gmxx_rxx_stats_octs_s cn68xx;
	struct cvmx_gmxx_rxx_stats_octs_s cn68xxp1;
	struct cvmx_gmxx_rxx_stats_octs_s cn70xx;
	struct cvmx_gmxx_rxx_stats_octs_s cn70xxp1;
	struct cvmx_gmxx_rxx_stats_octs_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_stats_octs cvmx_gmxx_rxx_stats_octs_t;

/**
 * cvmx_gmx#_rx#_stats_octs_ctl
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when GMX_RX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 */
union cvmx_gmxx_rxx_stats_octs_ctl {
	u64 u64;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn30xx;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn31xx;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn38xx;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn38xxp2;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn50xx;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn52xx;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn52xxp1;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn56xx;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn56xxp1;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn58xx;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn58xxp1;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn61xx;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn63xx;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn63xxp1;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn66xx;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn68xx;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn68xxp1;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn70xx;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cn70xxp1;
	struct cvmx_gmxx_rxx_stats_octs_ctl_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_stats_octs_ctl cvmx_gmxx_rxx_stats_octs_ctl_t;

/**
 * cvmx_gmx#_rx#_stats_octs_dmac
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when GMX_RX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 */
union cvmx_gmxx_rxx_stats_octs_dmac {
	u64 u64;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn30xx;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn31xx;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn38xx;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn38xxp2;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn50xx;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn52xx;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn52xxp1;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn56xx;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn56xxp1;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn58xx;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn58xxp1;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn61xx;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn63xx;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn63xxp1;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn66xx;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn68xx;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn68xxp1;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn70xx;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cn70xxp1;
	struct cvmx_gmxx_rxx_stats_octs_dmac_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_stats_octs_dmac cvmx_gmxx_rxx_stats_octs_dmac_t;

/**
 * cvmx_gmx#_rx#_stats_octs_drp
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when GMX_RX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 */
union cvmx_gmxx_rxx_stats_octs_drp {
	u64 u64;
	struct cvmx_gmxx_rxx_stats_octs_drp_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn30xx;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn31xx;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn38xx;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn38xxp2;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn50xx;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn52xx;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn52xxp1;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn56xx;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn56xxp1;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn58xx;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn58xxp1;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn61xx;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn63xx;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn63xxp1;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn66xx;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn68xx;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn68xxp1;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn70xx;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cn70xxp1;
	struct cvmx_gmxx_rxx_stats_octs_drp_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_stats_octs_drp cvmx_gmxx_rxx_stats_octs_drp_t;

/**
 * cvmx_gmx#_rx#_stats_pkts
 *
 * Count of good received packets - packets that are not recognized as PAUSE
 * packets, dropped due the DMAC filter, dropped due FIFO full status, or
 * have any other OPCODE (FCS, Length, etc).
 */
union cvmx_gmxx_rxx_stats_pkts {
	u64 u64;
	struct cvmx_gmxx_rxx_stats_pkts_s {
		u64 reserved_32_63 : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_gmxx_rxx_stats_pkts_s cn30xx;
	struct cvmx_gmxx_rxx_stats_pkts_s cn31xx;
	struct cvmx_gmxx_rxx_stats_pkts_s cn38xx;
	struct cvmx_gmxx_rxx_stats_pkts_s cn38xxp2;
	struct cvmx_gmxx_rxx_stats_pkts_s cn50xx;
	struct cvmx_gmxx_rxx_stats_pkts_s cn52xx;
	struct cvmx_gmxx_rxx_stats_pkts_s cn52xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_s cn56xx;
	struct cvmx_gmxx_rxx_stats_pkts_s cn56xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_s cn58xx;
	struct cvmx_gmxx_rxx_stats_pkts_s cn58xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_s cn61xx;
	struct cvmx_gmxx_rxx_stats_pkts_s cn63xx;
	struct cvmx_gmxx_rxx_stats_pkts_s cn63xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_s cn66xx;
	struct cvmx_gmxx_rxx_stats_pkts_s cn68xx;
	struct cvmx_gmxx_rxx_stats_pkts_s cn68xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_s cn70xx;
	struct cvmx_gmxx_rxx_stats_pkts_s cn70xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_stats_pkts cvmx_gmxx_rxx_stats_pkts_t;

/**
 * cvmx_gmx#_rx#_stats_pkts_bad
 *
 * Count of all packets received with some error that were not dropped
 * either due to the dmac filter or lack of room in the receive FIFO.
 */
union cvmx_gmxx_rxx_stats_pkts_bad {
	u64 u64;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s {
		u64 reserved_32_63 : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn30xx;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn31xx;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn38xx;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn38xxp2;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn50xx;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn52xx;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn52xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn56xx;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn56xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn58xx;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn58xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn61xx;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn63xx;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn63xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn66xx;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn68xx;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn68xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn70xx;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cn70xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_bad_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_stats_pkts_bad cvmx_gmxx_rxx_stats_pkts_bad_t;

/**
 * cvmx_gmx#_rx#_stats_pkts_ctl
 *
 * Count of all packets received that were recognized as Flow Control or
 * PAUSE packets.  PAUSE packets with any kind of error are counted in
 * GMX_RX_STATS_PKTS_BAD.  Pause packets can be optionally dropped or
 * forwarded based on the GMX_RX_FRM_CTL[CTL_DRP] bit.  This count
 * increments regardless of whether the packet is dropped.  Pause packets
 * will never be counted in GMX_RX_STATS_PKTS.  Packets dropped due the dmac
 * filter will be counted in GMX_RX_STATS_PKTS_DMAC and not here.
 */
union cvmx_gmxx_rxx_stats_pkts_ctl {
	u64 u64;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s {
		u64 reserved_32_63 : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn30xx;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn31xx;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn38xx;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn38xxp2;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn50xx;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn52xx;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn52xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn56xx;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn56xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn58xx;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn58xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn61xx;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn63xx;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn63xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn66xx;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn68xx;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn68xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn70xx;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cn70xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_ctl_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_stats_pkts_ctl cvmx_gmxx_rxx_stats_pkts_ctl_t;

/**
 * cvmx_gmx#_rx#_stats_pkts_dmac
 *
 * Count of all packets received that were dropped by the dmac filter.
 * Packets that match the DMAC will be dropped and counted here regardless
 * of if they were bad packets.  These packets will never be counted in
 * GMX_RX_STATS_PKTS.
 * Some packets that were not able to satisify the DECISION_CNT may not
 * actually be dropped by Octeon, but they will be counted here as if they
 * were dropped.
 */
union cvmx_gmxx_rxx_stats_pkts_dmac {
	u64 u64;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s {
		u64 reserved_32_63 : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn30xx;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn31xx;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn38xx;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn38xxp2;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn50xx;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn52xx;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn52xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn56xx;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn56xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn58xx;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn58xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn61xx;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn63xx;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn63xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn66xx;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn68xx;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn68xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn70xx;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cn70xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_dmac_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_stats_pkts_dmac cvmx_gmxx_rxx_stats_pkts_dmac_t;

/**
 * cvmx_gmx#_rx#_stats_pkts_drp
 *
 * Count of all packets received that were dropped due to a full receive FIFO.
 * This counts both partial packets in which there was enough space in the RX
 * FIFO to begin to buffer and the packet and total drops in which no packet was
 * sent to PKI.  This counts good and bad packets received - all packets dropped
 * by the FIFO.  It does not count packets dropped by the dmac or pause packet
 * filters.
 */
union cvmx_gmxx_rxx_stats_pkts_drp {
	u64 u64;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s {
		u64 reserved_32_63 : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn30xx;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn31xx;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn38xx;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn38xxp2;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn50xx;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn52xx;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn52xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn56xx;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn56xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn58xx;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn58xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn61xx;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn63xx;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn63xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn66xx;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn68xx;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn68xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn70xx;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cn70xxp1;
	struct cvmx_gmxx_rxx_stats_pkts_drp_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_stats_pkts_drp cvmx_gmxx_rxx_stats_pkts_drp_t;

/**
 * cvmx_gmx#_rx#_udd_skp
 *
 * GMX_RX_UDD_SKP = Amount of User-defined data before the start of the L2 data
 *
 *
 * Notes:
 * (1) The skip bytes are part of the packet and will be sent down the NCB
 *     packet interface and will be handled by PKI.
 *
 * (2) The system can determine if the UDD bytes are included in the FCS check
 *     by using the FCSSEL field - if the FCS check is enabled.
 *
 * (3) Assume that the preamble/sfd is always at the start of the frame - even
 *     before UDD bytes.  In most cases, there will be no preamble in these
 *     cases since it will be packet interface in direct communication to
 *     another packet interface (MAC to MAC) without a PHY involved.
 *
 * (4) We can still do address filtering and control packet filtering is the
 *     user desires.
 *
 * (5) UDD_SKP must be 0 in half-duplex operation unless
 *     GMX_RX_FRM_CTL[PRE_CHK] is clear.  If GMX_RX_FRM_CTL[PRE_CHK] is clear,
 *     then UDD_SKP will normally be 8.
 *
 * (6) In all cases, the UDD bytes will be sent down the packet interface as
 *     part of the packet.  The UDD bytes are never stripped from the actual
 *     packet.
 *
 * (7) If LEN != 0, then GMX_RX_FRM_CHK[LENERR] will be disabled and GMX_RX_INT_REG[LENERR] will be zero
 */
union cvmx_gmxx_rxx_udd_skp {
	u64 u64;
	struct cvmx_gmxx_rxx_udd_skp_s {
		u64 reserved_9_63 : 55;
		u64 fcssel : 1;
		u64 reserved_7_7 : 1;
		u64 len : 7;
	} s;
	struct cvmx_gmxx_rxx_udd_skp_s cn30xx;
	struct cvmx_gmxx_rxx_udd_skp_s cn31xx;
	struct cvmx_gmxx_rxx_udd_skp_s cn38xx;
	struct cvmx_gmxx_rxx_udd_skp_s cn38xxp2;
	struct cvmx_gmxx_rxx_udd_skp_s cn50xx;
	struct cvmx_gmxx_rxx_udd_skp_s cn52xx;
	struct cvmx_gmxx_rxx_udd_skp_s cn52xxp1;
	struct cvmx_gmxx_rxx_udd_skp_s cn56xx;
	struct cvmx_gmxx_rxx_udd_skp_s cn56xxp1;
	struct cvmx_gmxx_rxx_udd_skp_s cn58xx;
	struct cvmx_gmxx_rxx_udd_skp_s cn58xxp1;
	struct cvmx_gmxx_rxx_udd_skp_s cn61xx;
	struct cvmx_gmxx_rxx_udd_skp_s cn63xx;
	struct cvmx_gmxx_rxx_udd_skp_s cn63xxp1;
	struct cvmx_gmxx_rxx_udd_skp_s cn66xx;
	struct cvmx_gmxx_rxx_udd_skp_s cn68xx;
	struct cvmx_gmxx_rxx_udd_skp_s cn68xxp1;
	struct cvmx_gmxx_rxx_udd_skp_s cn70xx;
	struct cvmx_gmxx_rxx_udd_skp_s cn70xxp1;
	struct cvmx_gmxx_rxx_udd_skp_s cnf71xx;
};

typedef union cvmx_gmxx_rxx_udd_skp cvmx_gmxx_rxx_udd_skp_t;

/**
 * cvmx_gmx#_rx_bp_drop#
 *
 * GMX_RX_BP_DROP = FIFO mark for packet drop
 *
 *
 * Notes:
 * The actual watermark is dynamic with respect to the GMX_RX_PRTS
 * register.  The GMX_RX_PRTS controls the depth of the port's
 * FIFO so as ports are added or removed, the drop point may change.
 *
 * In XAUI mode prt0 is used for checking.
 */
union cvmx_gmxx_rx_bp_dropx {
	u64 u64;
	struct cvmx_gmxx_rx_bp_dropx_s {
		u64 reserved_6_63 : 58;
		u64 mark : 6;
	} s;
	struct cvmx_gmxx_rx_bp_dropx_s cn30xx;
	struct cvmx_gmxx_rx_bp_dropx_s cn31xx;
	struct cvmx_gmxx_rx_bp_dropx_s cn38xx;
	struct cvmx_gmxx_rx_bp_dropx_s cn38xxp2;
	struct cvmx_gmxx_rx_bp_dropx_s cn50xx;
	struct cvmx_gmxx_rx_bp_dropx_s cn52xx;
	struct cvmx_gmxx_rx_bp_dropx_s cn52xxp1;
	struct cvmx_gmxx_rx_bp_dropx_s cn56xx;
	struct cvmx_gmxx_rx_bp_dropx_s cn56xxp1;
	struct cvmx_gmxx_rx_bp_dropx_s cn58xx;
	struct cvmx_gmxx_rx_bp_dropx_s cn58xxp1;
	struct cvmx_gmxx_rx_bp_dropx_s cn61xx;
	struct cvmx_gmxx_rx_bp_dropx_s cn63xx;
	struct cvmx_gmxx_rx_bp_dropx_s cn63xxp1;
	struct cvmx_gmxx_rx_bp_dropx_s cn66xx;
	struct cvmx_gmxx_rx_bp_dropx_s cn68xx;
	struct cvmx_gmxx_rx_bp_dropx_s cn68xxp1;
	struct cvmx_gmxx_rx_bp_dropx_s cn70xx;
	struct cvmx_gmxx_rx_bp_dropx_s cn70xxp1;
	struct cvmx_gmxx_rx_bp_dropx_s cnf71xx;
};

typedef union cvmx_gmxx_rx_bp_dropx cvmx_gmxx_rx_bp_dropx_t;

/**
 * cvmx_gmx#_rx_bp_off#
 *
 * GMX_RX_BP_OFF = Lowater mark for packet drop
 *
 *
 * Notes:
 * In XAUI mode, prt0 is used for checking.
 *
 */
union cvmx_gmxx_rx_bp_offx {
	u64 u64;
	struct cvmx_gmxx_rx_bp_offx_s {
		u64 reserved_6_63 : 58;
		u64 mark : 6;
	} s;
	struct cvmx_gmxx_rx_bp_offx_s cn30xx;
	struct cvmx_gmxx_rx_bp_offx_s cn31xx;
	struct cvmx_gmxx_rx_bp_offx_s cn38xx;
	struct cvmx_gmxx_rx_bp_offx_s cn38xxp2;
	struct cvmx_gmxx_rx_bp_offx_s cn50xx;
	struct cvmx_gmxx_rx_bp_offx_s cn52xx;
	struct cvmx_gmxx_rx_bp_offx_s cn52xxp1;
	struct cvmx_gmxx_rx_bp_offx_s cn56xx;
	struct cvmx_gmxx_rx_bp_offx_s cn56xxp1;
	struct cvmx_gmxx_rx_bp_offx_s cn58xx;
	struct cvmx_gmxx_rx_bp_offx_s cn58xxp1;
	struct cvmx_gmxx_rx_bp_offx_s cn61xx;
	struct cvmx_gmxx_rx_bp_offx_s cn63xx;
	struct cvmx_gmxx_rx_bp_offx_s cn63xxp1;
	struct cvmx_gmxx_rx_bp_offx_s cn66xx;
	struct cvmx_gmxx_rx_bp_offx_s cn68xx;
	struct cvmx_gmxx_rx_bp_offx_s cn68xxp1;
	struct cvmx_gmxx_rx_bp_offx_s cn70xx;
	struct cvmx_gmxx_rx_bp_offx_s cn70xxp1;
	struct cvmx_gmxx_rx_bp_offx_s cnf71xx;
};

typedef union cvmx_gmxx_rx_bp_offx cvmx_gmxx_rx_bp_offx_t;

/**
 * cvmx_gmx#_rx_bp_on#
 *
 * GMX_RX_BP_ON = Hiwater mark for port/interface backpressure
 *
 *
 * Notes:
 * In XAUI mode, prt0 is used for checking.
 *
 */
union cvmx_gmxx_rx_bp_onx {
	u64 u64;
	struct cvmx_gmxx_rx_bp_onx_s {
		u64 reserved_11_63 : 53;
		u64 mark : 11;
	} s;
	struct cvmx_gmxx_rx_bp_onx_cn30xx {
		u64 reserved_9_63 : 55;
		u64 mark : 9;
	} cn30xx;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn31xx;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn38xx;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn38xxp2;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn50xx;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn52xx;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn52xxp1;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn56xx;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn56xxp1;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn58xx;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn58xxp1;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn61xx;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn63xx;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn63xxp1;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn66xx;
	struct cvmx_gmxx_rx_bp_onx_s cn68xx;
	struct cvmx_gmxx_rx_bp_onx_s cn68xxp1;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn70xx;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cn70xxp1;
	struct cvmx_gmxx_rx_bp_onx_cn30xx cnf71xx;
};

typedef union cvmx_gmxx_rx_bp_onx cvmx_gmxx_rx_bp_onx_t;

/**
 * cvmx_gmx#_rx_hg2_status
 *
 * ** HG2 message CSRs
 *
 */
union cvmx_gmxx_rx_hg2_status {
	u64 u64;
	struct cvmx_gmxx_rx_hg2_status_s {
		u64 reserved_48_63 : 16;
		u64 phtim2go : 16;
		u64 xof : 16;
		u64 lgtim2go : 16;
	} s;
	struct cvmx_gmxx_rx_hg2_status_s cn52xx;
	struct cvmx_gmxx_rx_hg2_status_s cn52xxp1;
	struct cvmx_gmxx_rx_hg2_status_s cn56xx;
	struct cvmx_gmxx_rx_hg2_status_s cn61xx;
	struct cvmx_gmxx_rx_hg2_status_s cn63xx;
	struct cvmx_gmxx_rx_hg2_status_s cn63xxp1;
	struct cvmx_gmxx_rx_hg2_status_s cn66xx;
	struct cvmx_gmxx_rx_hg2_status_s cn68xx;
	struct cvmx_gmxx_rx_hg2_status_s cn68xxp1;
	struct cvmx_gmxx_rx_hg2_status_s cn70xx;
	struct cvmx_gmxx_rx_hg2_status_s cn70xxp1;
	struct cvmx_gmxx_rx_hg2_status_s cnf71xx;
};

typedef union cvmx_gmxx_rx_hg2_status cvmx_gmxx_rx_hg2_status_t;

/**
 * cvmx_gmx#_rx_pass_en
 *
 * GMX_RX_PASS_EN = Packet pass through mode enable
 *
 * When both Octane ports are running in Spi4 mode, packets can be directly
 * passed from one SPX interface to the other without being processed by the
 * core or PP's.  The register has one bit for each port to enable the pass
 * through feature.
 *
 * Notes:
 * (1) Can only be used in dual Spi4 configs
 *
 * (2) The mapped pass through output port cannot be the destination port for
 *     any Octane core traffic.
 */
union cvmx_gmxx_rx_pass_en {
	u64 u64;
	struct cvmx_gmxx_rx_pass_en_s {
		u64 reserved_16_63 : 48;
		u64 en : 16;
	} s;
	struct cvmx_gmxx_rx_pass_en_s cn38xx;
	struct cvmx_gmxx_rx_pass_en_s cn38xxp2;
	struct cvmx_gmxx_rx_pass_en_s cn58xx;
	struct cvmx_gmxx_rx_pass_en_s cn58xxp1;
};

typedef union cvmx_gmxx_rx_pass_en cvmx_gmxx_rx_pass_en_t;

/**
 * cvmx_gmx#_rx_pass_map#
 *
 * GMX_RX_PASS_MAP = Packet pass through port map
 *
 */
union cvmx_gmxx_rx_pass_mapx {
	u64 u64;
	struct cvmx_gmxx_rx_pass_mapx_s {
		u64 reserved_4_63 : 60;
		u64 dprt : 4;
	} s;
	struct cvmx_gmxx_rx_pass_mapx_s cn38xx;
	struct cvmx_gmxx_rx_pass_mapx_s cn38xxp2;
	struct cvmx_gmxx_rx_pass_mapx_s cn58xx;
	struct cvmx_gmxx_rx_pass_mapx_s cn58xxp1;
};

typedef union cvmx_gmxx_rx_pass_mapx cvmx_gmxx_rx_pass_mapx_t;

/**
 * cvmx_gmx#_rx_prt_info
 *
 * GMX_RX_PRT_INFO = Report the RX status for port
 *
 *
 * Notes:
 * In XAUI mode, only the lsb (corresponding to port0) of DROP and COMMIT are used.
 *
 */
union cvmx_gmxx_rx_prt_info {
	u64 u64;
	struct cvmx_gmxx_rx_prt_info_s {
		u64 reserved_32_63 : 32;
		u64 drop : 16;
		u64 commit : 16;
	} s;
	struct cvmx_gmxx_rx_prt_info_cn30xx {
		u64 reserved_19_63 : 45;
		u64 drop : 3;
		u64 reserved_3_15 : 13;
		u64 commit : 3;
	} cn30xx;
	struct cvmx_gmxx_rx_prt_info_cn30xx cn31xx;
	struct cvmx_gmxx_rx_prt_info_s cn38xx;
	struct cvmx_gmxx_rx_prt_info_cn30xx cn50xx;
	struct cvmx_gmxx_rx_prt_info_cn52xx {
		u64 reserved_20_63 : 44;
		u64 drop : 4;
		u64 reserved_4_15 : 12;
		u64 commit : 4;
	} cn52xx;
	struct cvmx_gmxx_rx_prt_info_cn52xx cn52xxp1;
	struct cvmx_gmxx_rx_prt_info_cn52xx cn56xx;
	struct cvmx_gmxx_rx_prt_info_cn52xx cn56xxp1;
	struct cvmx_gmxx_rx_prt_info_s cn58xx;
	struct cvmx_gmxx_rx_prt_info_s cn58xxp1;
	struct cvmx_gmxx_rx_prt_info_cn52xx cn61xx;
	struct cvmx_gmxx_rx_prt_info_cn52xx cn63xx;
	struct cvmx_gmxx_rx_prt_info_cn52xx cn63xxp1;
	struct cvmx_gmxx_rx_prt_info_cn52xx cn66xx;
	struct cvmx_gmxx_rx_prt_info_cn52xx cn68xx;
	struct cvmx_gmxx_rx_prt_info_cn52xx cn68xxp1;
	struct cvmx_gmxx_rx_prt_info_cn52xx cn70xx;
	struct cvmx_gmxx_rx_prt_info_cn52xx cn70xxp1;
	struct cvmx_gmxx_rx_prt_info_cnf71xx {
		u64 reserved_18_63 : 46;
		u64 drop : 2;
		u64 reserved_2_15 : 14;
		u64 commit : 2;
	} cnf71xx;
};

typedef union cvmx_gmxx_rx_prt_info cvmx_gmxx_rx_prt_info_t;

/**
 * cvmx_gmx#_rx_prts
 *
 * GMX_RX_PRTS = Number of FIFOs to carve the RX buffer into
 *
 *
 * Notes:
 * GMX_RX_PRTS[PRTS] must be set to '1' in XAUI mode.
 *
 */
union cvmx_gmxx_rx_prts {
	u64 u64;
	struct cvmx_gmxx_rx_prts_s {
		u64 reserved_3_63 : 61;
		u64 prts : 3;
	} s;
	struct cvmx_gmxx_rx_prts_s cn30xx;
	struct cvmx_gmxx_rx_prts_s cn31xx;
	struct cvmx_gmxx_rx_prts_s cn38xx;
	struct cvmx_gmxx_rx_prts_s cn38xxp2;
	struct cvmx_gmxx_rx_prts_s cn50xx;
	struct cvmx_gmxx_rx_prts_s cn52xx;
	struct cvmx_gmxx_rx_prts_s cn52xxp1;
	struct cvmx_gmxx_rx_prts_s cn56xx;
	struct cvmx_gmxx_rx_prts_s cn56xxp1;
	struct cvmx_gmxx_rx_prts_s cn58xx;
	struct cvmx_gmxx_rx_prts_s cn58xxp1;
	struct cvmx_gmxx_rx_prts_s cn61xx;
	struct cvmx_gmxx_rx_prts_s cn63xx;
	struct cvmx_gmxx_rx_prts_s cn63xxp1;
	struct cvmx_gmxx_rx_prts_s cn66xx;
	struct cvmx_gmxx_rx_prts_s cn68xx;
	struct cvmx_gmxx_rx_prts_s cn68xxp1;
	struct cvmx_gmxx_rx_prts_s cn70xx;
	struct cvmx_gmxx_rx_prts_s cn70xxp1;
	struct cvmx_gmxx_rx_prts_s cnf71xx;
};

typedef union cvmx_gmxx_rx_prts cvmx_gmxx_rx_prts_t;

/**
 * cvmx_gmx#_rx_tx_status
 *
 * GMX_RX_TX_STATUS = GMX RX/TX Status
 *
 */
union cvmx_gmxx_rx_tx_status {
	u64 u64;
	struct cvmx_gmxx_rx_tx_status_s {
		u64 reserved_7_63 : 57;
		u64 tx : 3;
		u64 reserved_3_3 : 1;
		u64 rx : 3;
	} s;
	struct cvmx_gmxx_rx_tx_status_s cn30xx;
	struct cvmx_gmxx_rx_tx_status_s cn31xx;
	struct cvmx_gmxx_rx_tx_status_s cn50xx;
};

typedef union cvmx_gmxx_rx_tx_status cvmx_gmxx_rx_tx_status_t;

/**
 * cvmx_gmx#_rx_xaui_bad_col
 */
union cvmx_gmxx_rx_xaui_bad_col {
	u64 u64;
	struct cvmx_gmxx_rx_xaui_bad_col_s {
		u64 reserved_40_63 : 24;
		u64 val : 1;
		u64 state : 3;
		u64 lane_rxc : 4;
		u64 lane_rxd : 32;
	} s;
	struct cvmx_gmxx_rx_xaui_bad_col_s cn52xx;
	struct cvmx_gmxx_rx_xaui_bad_col_s cn52xxp1;
	struct cvmx_gmxx_rx_xaui_bad_col_s cn56xx;
	struct cvmx_gmxx_rx_xaui_bad_col_s cn56xxp1;
	struct cvmx_gmxx_rx_xaui_bad_col_s cn61xx;
	struct cvmx_gmxx_rx_xaui_bad_col_s cn63xx;
	struct cvmx_gmxx_rx_xaui_bad_col_s cn63xxp1;
	struct cvmx_gmxx_rx_xaui_bad_col_s cn66xx;
	struct cvmx_gmxx_rx_xaui_bad_col_s cn68xx;
	struct cvmx_gmxx_rx_xaui_bad_col_s cn68xxp1;
	struct cvmx_gmxx_rx_xaui_bad_col_s cn70xx;
	struct cvmx_gmxx_rx_xaui_bad_col_s cn70xxp1;
	struct cvmx_gmxx_rx_xaui_bad_col_s cnf71xx;
};

typedef union cvmx_gmxx_rx_xaui_bad_col cvmx_gmxx_rx_xaui_bad_col_t;

/**
 * cvmx_gmx#_rx_xaui_ctl
 */
union cvmx_gmxx_rx_xaui_ctl {
	u64 u64;
	struct cvmx_gmxx_rx_xaui_ctl_s {
		u64 reserved_2_63 : 62;
		u64 status : 2;
	} s;
	struct cvmx_gmxx_rx_xaui_ctl_s cn52xx;
	struct cvmx_gmxx_rx_xaui_ctl_s cn52xxp1;
	struct cvmx_gmxx_rx_xaui_ctl_s cn56xx;
	struct cvmx_gmxx_rx_xaui_ctl_s cn56xxp1;
	struct cvmx_gmxx_rx_xaui_ctl_s cn61xx;
	struct cvmx_gmxx_rx_xaui_ctl_s cn63xx;
	struct cvmx_gmxx_rx_xaui_ctl_s cn63xxp1;
	struct cvmx_gmxx_rx_xaui_ctl_s cn66xx;
	struct cvmx_gmxx_rx_xaui_ctl_s cn68xx;
	struct cvmx_gmxx_rx_xaui_ctl_s cn68xxp1;
	struct cvmx_gmxx_rx_xaui_ctl_s cn70xx;
	struct cvmx_gmxx_rx_xaui_ctl_s cn70xxp1;
	struct cvmx_gmxx_rx_xaui_ctl_s cnf71xx;
};

typedef union cvmx_gmxx_rx_xaui_ctl cvmx_gmxx_rx_xaui_ctl_t;

/**
 * cvmx_gmx#_rxaui_ctl
 */
union cvmx_gmxx_rxaui_ctl {
	u64 u64;
	struct cvmx_gmxx_rxaui_ctl_s {
		u64 reserved_1_63 : 63;
		u64 disparity : 1;
	} s;
	struct cvmx_gmxx_rxaui_ctl_s cn68xx;
	struct cvmx_gmxx_rxaui_ctl_s cn68xxp1;
	struct cvmx_gmxx_rxaui_ctl_s cn70xx;
	struct cvmx_gmxx_rxaui_ctl_s cn70xxp1;
};

typedef union cvmx_gmxx_rxaui_ctl cvmx_gmxx_rxaui_ctl_t;

/**
 * cvmx_gmx#_smac#
 *
 * GMX_SMAC = Packet SMAC
 *
 */
union cvmx_gmxx_smacx {
	u64 u64;
	struct cvmx_gmxx_smacx_s {
		u64 reserved_48_63 : 16;
		u64 smac : 48;
	} s;
	struct cvmx_gmxx_smacx_s cn30xx;
	struct cvmx_gmxx_smacx_s cn31xx;
	struct cvmx_gmxx_smacx_s cn38xx;
	struct cvmx_gmxx_smacx_s cn38xxp2;
	struct cvmx_gmxx_smacx_s cn50xx;
	struct cvmx_gmxx_smacx_s cn52xx;
	struct cvmx_gmxx_smacx_s cn52xxp1;
	struct cvmx_gmxx_smacx_s cn56xx;
	struct cvmx_gmxx_smacx_s cn56xxp1;
	struct cvmx_gmxx_smacx_s cn58xx;
	struct cvmx_gmxx_smacx_s cn58xxp1;
	struct cvmx_gmxx_smacx_s cn61xx;
	struct cvmx_gmxx_smacx_s cn63xx;
	struct cvmx_gmxx_smacx_s cn63xxp1;
	struct cvmx_gmxx_smacx_s cn66xx;
	struct cvmx_gmxx_smacx_s cn68xx;
	struct cvmx_gmxx_smacx_s cn68xxp1;
	struct cvmx_gmxx_smacx_s cn70xx;
	struct cvmx_gmxx_smacx_s cn70xxp1;
	struct cvmx_gmxx_smacx_s cnf71xx;
};

typedef union cvmx_gmxx_smacx cvmx_gmxx_smacx_t;

/**
 * cvmx_gmx#_soft_bist
 *
 * GMX_SOFT_BIST = Software BIST Control
 *
 */
union cvmx_gmxx_soft_bist {
	u64 u64;
	struct cvmx_gmxx_soft_bist_s {
		u64 reserved_2_63 : 62;
		u64 start_bist : 1;
		u64 clear_bist : 1;
	} s;
	struct cvmx_gmxx_soft_bist_s cn63xx;
	struct cvmx_gmxx_soft_bist_s cn63xxp1;
	struct cvmx_gmxx_soft_bist_s cn66xx;
	struct cvmx_gmxx_soft_bist_s cn68xx;
	struct cvmx_gmxx_soft_bist_s cn68xxp1;
};

typedef union cvmx_gmxx_soft_bist cvmx_gmxx_soft_bist_t;

/**
 * cvmx_gmx#_stat_bp
 *
 * GMX_STAT_BP = Number of cycles that the TX/Stats block has help up operation
 *
 *
 * Notes:
 * It has no relationship with the TX FIFO per se.  The TX engine sends packets
 * from PKO and upon completion, sends a command to the TX stats block for an
 * update based on the packet size.  The stats operation can take a few cycles -
 * normally not enough to be visible considering the 64B min packet size that is
 * ethernet convention.
 *
 * In the rare case in which SW attempted to schedule really, really, small packets
 * or the sclk (6xxx) is running ass-slow, then the stats updates may not happen in
 * real time and can back up the TX engine.
 *
 * This counter is the number of cycles in which the TX engine was stalled.  In
 * normal operation, it should always be zeros.
 */
union cvmx_gmxx_stat_bp {
	u64 u64;
	struct cvmx_gmxx_stat_bp_s {
		u64 reserved_17_63 : 47;
		u64 bp : 1;
		u64 cnt : 16;
	} s;
	struct cvmx_gmxx_stat_bp_s cn30xx;
	struct cvmx_gmxx_stat_bp_s cn31xx;
	struct cvmx_gmxx_stat_bp_s cn38xx;
	struct cvmx_gmxx_stat_bp_s cn38xxp2;
	struct cvmx_gmxx_stat_bp_s cn50xx;
	struct cvmx_gmxx_stat_bp_s cn52xx;
	struct cvmx_gmxx_stat_bp_s cn52xxp1;
	struct cvmx_gmxx_stat_bp_s cn56xx;
	struct cvmx_gmxx_stat_bp_s cn56xxp1;
	struct cvmx_gmxx_stat_bp_s cn58xx;
	struct cvmx_gmxx_stat_bp_s cn58xxp1;
	struct cvmx_gmxx_stat_bp_s cn61xx;
	struct cvmx_gmxx_stat_bp_s cn63xx;
	struct cvmx_gmxx_stat_bp_s cn63xxp1;
	struct cvmx_gmxx_stat_bp_s cn66xx;
	struct cvmx_gmxx_stat_bp_s cn68xx;
	struct cvmx_gmxx_stat_bp_s cn68xxp1;
	struct cvmx_gmxx_stat_bp_s cn70xx;
	struct cvmx_gmxx_stat_bp_s cn70xxp1;
	struct cvmx_gmxx_stat_bp_s cnf71xx;
};

typedef union cvmx_gmxx_stat_bp cvmx_gmxx_stat_bp_t;

/**
 * cvmx_gmx#_tb_reg
 *
 * DON'T PUT IN HRM*
 *
 */
union cvmx_gmxx_tb_reg {
	u64 u64;
	struct cvmx_gmxx_tb_reg_s {
		u64 reserved_1_63 : 63;
		u64 wr_magic : 1;
	} s;
	struct cvmx_gmxx_tb_reg_s cn61xx;
	struct cvmx_gmxx_tb_reg_s cn66xx;
	struct cvmx_gmxx_tb_reg_s cn68xx;
	struct cvmx_gmxx_tb_reg_s cn70xx;
	struct cvmx_gmxx_tb_reg_s cn70xxp1;
	struct cvmx_gmxx_tb_reg_s cnf71xx;
};

typedef union cvmx_gmxx_tb_reg cvmx_gmxx_tb_reg_t;

/**
 * cvmx_gmx#_tx#_append
 *
 * GMX_TX_APPEND = Packet TX Append Control
 *
 */
union cvmx_gmxx_txx_append {
	u64 u64;
	struct cvmx_gmxx_txx_append_s {
		u64 reserved_4_63 : 60;
		u64 force_fcs : 1;
		u64 fcs : 1;
		u64 pad : 1;
		u64 preamble : 1;
	} s;
	struct cvmx_gmxx_txx_append_s cn30xx;
	struct cvmx_gmxx_txx_append_s cn31xx;
	struct cvmx_gmxx_txx_append_s cn38xx;
	struct cvmx_gmxx_txx_append_s cn38xxp2;
	struct cvmx_gmxx_txx_append_s cn50xx;
	struct cvmx_gmxx_txx_append_s cn52xx;
	struct cvmx_gmxx_txx_append_s cn52xxp1;
	struct cvmx_gmxx_txx_append_s cn56xx;
	struct cvmx_gmxx_txx_append_s cn56xxp1;
	struct cvmx_gmxx_txx_append_s cn58xx;
	struct cvmx_gmxx_txx_append_s cn58xxp1;
	struct cvmx_gmxx_txx_append_s cn61xx;
	struct cvmx_gmxx_txx_append_s cn63xx;
	struct cvmx_gmxx_txx_append_s cn63xxp1;
	struct cvmx_gmxx_txx_append_s cn66xx;
	struct cvmx_gmxx_txx_append_s cn68xx;
	struct cvmx_gmxx_txx_append_s cn68xxp1;
	struct cvmx_gmxx_txx_append_s cn70xx;
	struct cvmx_gmxx_txx_append_s cn70xxp1;
	struct cvmx_gmxx_txx_append_s cnf71xx;
};

typedef union cvmx_gmxx_txx_append cvmx_gmxx_txx_append_t;

/**
 * cvmx_gmx#_tx#_bck_crdt
 *
 * gmi_tx_bck to gmi_tx_out credit count register
 *
 */
union cvmx_gmxx_txx_bck_crdt {
	u64 u64;
	struct cvmx_gmxx_txx_bck_crdt_s {
		u64 reserved_4_63 : 60;
		u64 cnt : 4;
	} s;
	struct cvmx_gmxx_txx_bck_crdt_s cn70xx;
	struct cvmx_gmxx_txx_bck_crdt_s cn70xxp1;
};

typedef union cvmx_gmxx_txx_bck_crdt cvmx_gmxx_txx_bck_crdt_t;

/**
 * cvmx_gmx#_tx#_burst
 *
 * GMX_TX_BURST = Packet TX Burst Counter
 *
 */
union cvmx_gmxx_txx_burst {
	u64 u64;
	struct cvmx_gmxx_txx_burst_s {
		u64 reserved_16_63 : 48;
		u64 burst : 16;
	} s;
	struct cvmx_gmxx_txx_burst_s cn30xx;
	struct cvmx_gmxx_txx_burst_s cn31xx;
	struct cvmx_gmxx_txx_burst_s cn38xx;
	struct cvmx_gmxx_txx_burst_s cn38xxp2;
	struct cvmx_gmxx_txx_burst_s cn50xx;
	struct cvmx_gmxx_txx_burst_s cn52xx;
	struct cvmx_gmxx_txx_burst_s cn52xxp1;
	struct cvmx_gmxx_txx_burst_s cn56xx;
	struct cvmx_gmxx_txx_burst_s cn56xxp1;
	struct cvmx_gmxx_txx_burst_s cn58xx;
	struct cvmx_gmxx_txx_burst_s cn58xxp1;
	struct cvmx_gmxx_txx_burst_s cn61xx;
	struct cvmx_gmxx_txx_burst_s cn63xx;
	struct cvmx_gmxx_txx_burst_s cn63xxp1;
	struct cvmx_gmxx_txx_burst_s cn66xx;
	struct cvmx_gmxx_txx_burst_s cn68xx;
	struct cvmx_gmxx_txx_burst_s cn68xxp1;
	struct cvmx_gmxx_txx_burst_s cn70xx;
	struct cvmx_gmxx_txx_burst_s cn70xxp1;
	struct cvmx_gmxx_txx_burst_s cnf71xx;
};

typedef union cvmx_gmxx_txx_burst cvmx_gmxx_txx_burst_t;

/**
 * cvmx_gmx#_tx#_cbfc_xoff
 */
union cvmx_gmxx_txx_cbfc_xoff {
	u64 u64;
	struct cvmx_gmxx_txx_cbfc_xoff_s {
		u64 reserved_16_63 : 48;
		u64 xoff : 16;
	} s;
	struct cvmx_gmxx_txx_cbfc_xoff_s cn52xx;
	struct cvmx_gmxx_txx_cbfc_xoff_s cn56xx;
	struct cvmx_gmxx_txx_cbfc_xoff_s cn61xx;
	struct cvmx_gmxx_txx_cbfc_xoff_s cn63xx;
	struct cvmx_gmxx_txx_cbfc_xoff_s cn63xxp1;
	struct cvmx_gmxx_txx_cbfc_xoff_s cn66xx;
	struct cvmx_gmxx_txx_cbfc_xoff_s cn68xx;
	struct cvmx_gmxx_txx_cbfc_xoff_s cn68xxp1;
	struct cvmx_gmxx_txx_cbfc_xoff_s cn70xx;
	struct cvmx_gmxx_txx_cbfc_xoff_s cn70xxp1;
	struct cvmx_gmxx_txx_cbfc_xoff_s cnf71xx;
};

typedef union cvmx_gmxx_txx_cbfc_xoff cvmx_gmxx_txx_cbfc_xoff_t;

/**
 * cvmx_gmx#_tx#_cbfc_xon
 */
union cvmx_gmxx_txx_cbfc_xon {
	u64 u64;
	struct cvmx_gmxx_txx_cbfc_xon_s {
		u64 reserved_16_63 : 48;
		u64 xon : 16;
	} s;
	struct cvmx_gmxx_txx_cbfc_xon_s cn52xx;
	struct cvmx_gmxx_txx_cbfc_xon_s cn56xx;
	struct cvmx_gmxx_txx_cbfc_xon_s cn61xx;
	struct cvmx_gmxx_txx_cbfc_xon_s cn63xx;
	struct cvmx_gmxx_txx_cbfc_xon_s cn63xxp1;
	struct cvmx_gmxx_txx_cbfc_xon_s cn66xx;
	struct cvmx_gmxx_txx_cbfc_xon_s cn68xx;
	struct cvmx_gmxx_txx_cbfc_xon_s cn68xxp1;
	struct cvmx_gmxx_txx_cbfc_xon_s cn70xx;
	struct cvmx_gmxx_txx_cbfc_xon_s cn70xxp1;
	struct cvmx_gmxx_txx_cbfc_xon_s cnf71xx;
};

typedef union cvmx_gmxx_txx_cbfc_xon cvmx_gmxx_txx_cbfc_xon_t;

/**
 * cvmx_gmx#_tx#_clk
 *
 * Per Port
 *
 *
 * GMX_TX_CLK = RGMII TX Clock Generation Register
 *
 * Notes:
 * Programming Restrictions:
 *  (1) In RGMII mode, if GMX_PRT_CFG[SPEED]==0, then CLK_CNT must be > 1.
 *  (2) In MII mode, CLK_CNT == 1
 *  (3) In RGMII or GMII mode, if CLK_CNT==0, Octeon will not generate a tx clock.
 *
 * RGMII Example:
 *  Given a 125MHz PLL reference clock...
 *   CLK_CNT ==  1 ==> 125.0MHz TXC clock period (8ns* 1)
 *   CLK_CNT ==  5 ==>  25.0MHz TXC clock period (8ns* 5)
 *   CLK_CNT == 50 ==>   2.5MHz TXC clock period (8ns*50)
 */
union cvmx_gmxx_txx_clk {
	u64 u64;
	struct cvmx_gmxx_txx_clk_s {
		u64 reserved_6_63 : 58;
		u64 clk_cnt : 6;
	} s;
	struct cvmx_gmxx_txx_clk_s cn30xx;
	struct cvmx_gmxx_txx_clk_s cn31xx;
	struct cvmx_gmxx_txx_clk_s cn38xx;
	struct cvmx_gmxx_txx_clk_s cn38xxp2;
	struct cvmx_gmxx_txx_clk_s cn50xx;
	struct cvmx_gmxx_txx_clk_s cn58xx;
	struct cvmx_gmxx_txx_clk_s cn58xxp1;
};

typedef union cvmx_gmxx_txx_clk cvmx_gmxx_txx_clk_t;

/**
 * cvmx_gmx#_tx#_ctl
 *
 * GMX_TX_CTL = TX Control register
 *
 */
union cvmx_gmxx_txx_ctl {
	u64 u64;
	struct cvmx_gmxx_txx_ctl_s {
		u64 reserved_2_63 : 62;
		u64 xsdef_en : 1;
		u64 xscol_en : 1;
	} s;
	struct cvmx_gmxx_txx_ctl_s cn30xx;
	struct cvmx_gmxx_txx_ctl_s cn31xx;
	struct cvmx_gmxx_txx_ctl_s cn38xx;
	struct cvmx_gmxx_txx_ctl_s cn38xxp2;
	struct cvmx_gmxx_txx_ctl_s cn50xx;
	struct cvmx_gmxx_txx_ctl_s cn52xx;
	struct cvmx_gmxx_txx_ctl_s cn52xxp1;
	struct cvmx_gmxx_txx_ctl_s cn56xx;
	struct cvmx_gmxx_txx_ctl_s cn56xxp1;
	struct cvmx_gmxx_txx_ctl_s cn58xx;
	struct cvmx_gmxx_txx_ctl_s cn58xxp1;
	struct cvmx_gmxx_txx_ctl_s cn61xx;
	struct cvmx_gmxx_txx_ctl_s cn63xx;
	struct cvmx_gmxx_txx_ctl_s cn63xxp1;
	struct cvmx_gmxx_txx_ctl_s cn66xx;
	struct cvmx_gmxx_txx_ctl_s cn68xx;
	struct cvmx_gmxx_txx_ctl_s cn68xxp1;
	struct cvmx_gmxx_txx_ctl_s cn70xx;
	struct cvmx_gmxx_txx_ctl_s cn70xxp1;
	struct cvmx_gmxx_txx_ctl_s cnf71xx;
};

typedef union cvmx_gmxx_txx_ctl cvmx_gmxx_txx_ctl_t;

/**
 * cvmx_gmx#_tx#_jam_mode
 */
union cvmx_gmxx_txx_jam_mode {
	u64 u64;
	struct cvmx_gmxx_txx_jam_mode_s {
		u64 reserved_1_63 : 63;
		u64 mode : 1;
	} s;
	struct cvmx_gmxx_txx_jam_mode_s cn70xx;
	struct cvmx_gmxx_txx_jam_mode_s cn70xxp1;
};

typedef union cvmx_gmxx_txx_jam_mode cvmx_gmxx_txx_jam_mode_t;

/**
 * cvmx_gmx#_tx#_min_pkt
 *
 * GMX_TX_MIN_PKT = Packet TX Min Size Packet (PAD upto min size)
 *
 */
union cvmx_gmxx_txx_min_pkt {
	u64 u64;
	struct cvmx_gmxx_txx_min_pkt_s {
		u64 reserved_8_63 : 56;
		u64 min_size : 8;
	} s;
	struct cvmx_gmxx_txx_min_pkt_s cn30xx;
	struct cvmx_gmxx_txx_min_pkt_s cn31xx;
	struct cvmx_gmxx_txx_min_pkt_s cn38xx;
	struct cvmx_gmxx_txx_min_pkt_s cn38xxp2;
	struct cvmx_gmxx_txx_min_pkt_s cn50xx;
	struct cvmx_gmxx_txx_min_pkt_s cn52xx;
	struct cvmx_gmxx_txx_min_pkt_s cn52xxp1;
	struct cvmx_gmxx_txx_min_pkt_s cn56xx;
	struct cvmx_gmxx_txx_min_pkt_s cn56xxp1;
	struct cvmx_gmxx_txx_min_pkt_s cn58xx;
	struct cvmx_gmxx_txx_min_pkt_s cn58xxp1;
	struct cvmx_gmxx_txx_min_pkt_s cn61xx;
	struct cvmx_gmxx_txx_min_pkt_s cn63xx;
	struct cvmx_gmxx_txx_min_pkt_s cn63xxp1;
	struct cvmx_gmxx_txx_min_pkt_s cn66xx;
	struct cvmx_gmxx_txx_min_pkt_s cn68xx;
	struct cvmx_gmxx_txx_min_pkt_s cn68xxp1;
	struct cvmx_gmxx_txx_min_pkt_s cn70xx;
	struct cvmx_gmxx_txx_min_pkt_s cn70xxp1;
	struct cvmx_gmxx_txx_min_pkt_s cnf71xx;
};

typedef union cvmx_gmxx_txx_min_pkt cvmx_gmxx_txx_min_pkt_t;

/**
 * cvmx_gmx#_tx#_pause_pkt_interval
 *
 * GMX_TX_PAUSE_PKT_INTERVAL = Packet TX Pause Packet transmission interval - how often PAUSE packets will be sent
 *
 *
 * Notes:
 * Choosing proper values of GMX_TX_PAUSE_PKT_TIME[TIME] and
 * GMX_TX_PAUSE_PKT_INTERVAL[INTERVAL] can be challenging to the system
 * designer.  It is suggested that TIME be much greater than INTERVAL and
 * GMX_TX_PAUSE_ZERO[SEND] be set.  This allows a periodic refresh of the PAUSE
 * count and then when the backpressure condition is lifted, a PAUSE packet
 * with TIME==0 will be sent indicating that Octane is ready for additional
 * data.
 *
 * If the system chooses to not set GMX_TX_PAUSE_ZERO[SEND], then it is
 * suggested that TIME and INTERVAL are programmed such that they satisify the
 * following rule...
 *
 *    INTERVAL <= TIME - (largest_pkt_size + IFG + pause_pkt_size)
 *
 * where largest_pkt_size is that largest packet that the system can send
 * (normally 1518B), IFG is the interframe gap and pause_pkt_size is the size
 * of the PAUSE packet (normally 64B).
 */
union cvmx_gmxx_txx_pause_pkt_interval {
	u64 u64;
	struct cvmx_gmxx_txx_pause_pkt_interval_s {
		u64 reserved_16_63 : 48;
		u64 interval : 16;
	} s;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn30xx;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn31xx;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn38xx;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn38xxp2;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn50xx;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn52xx;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn52xxp1;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn56xx;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn56xxp1;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn58xx;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn58xxp1;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn61xx;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn63xx;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn63xxp1;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn66xx;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn68xx;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn68xxp1;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn70xx;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cn70xxp1;
	struct cvmx_gmxx_txx_pause_pkt_interval_s cnf71xx;
};

typedef union cvmx_gmxx_txx_pause_pkt_interval cvmx_gmxx_txx_pause_pkt_interval_t;

/**
 * cvmx_gmx#_tx#_pause_pkt_time
 *
 * GMX_TX_PAUSE_PKT_TIME = Packet TX Pause Packet pause_time field
 *
 *
 * Notes:
 * Choosing proper values of GMX_TX_PAUSE_PKT_TIME[TIME] and
 * GMX_TX_PAUSE_PKT_INTERVAL[INTERVAL] can be challenging to the system
 * designer.  It is suggested that TIME be much greater than INTERVAL and
 * GMX_TX_PAUSE_ZERO[SEND] be set.  This allows a periodic refresh of the PAUSE
 * count and then when the backpressure condition is lifted, a PAUSE packet
 * with TIME==0 will be sent indicating that Octane is ready for additional
 * data.
 *
 * If the system chooses to not set GMX_TX_PAUSE_ZERO[SEND], then it is
 * suggested that TIME and INTERVAL are programmed such that they satisify the
 * following rule...
 *
 *    INTERVAL <= TIME - (largest_pkt_size + IFG + pause_pkt_size)
 *
 * where largest_pkt_size is that largest packet that the system can send
 * (normally 1518B), IFG is the interframe gap and pause_pkt_size is the size
 * of the PAUSE packet (normally 64B).
 */
union cvmx_gmxx_txx_pause_pkt_time {
	u64 u64;
	struct cvmx_gmxx_txx_pause_pkt_time_s {
		u64 reserved_16_63 : 48;
		u64 time : 16;
	} s;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn30xx;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn31xx;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn38xx;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn38xxp2;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn50xx;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn52xx;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn52xxp1;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn56xx;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn56xxp1;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn58xx;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn58xxp1;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn61xx;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn63xx;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn63xxp1;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn66xx;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn68xx;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn68xxp1;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn70xx;
	struct cvmx_gmxx_txx_pause_pkt_time_s cn70xxp1;
	struct cvmx_gmxx_txx_pause_pkt_time_s cnf71xx;
};

typedef union cvmx_gmxx_txx_pause_pkt_time cvmx_gmxx_txx_pause_pkt_time_t;

/**
 * cvmx_gmx#_tx#_pause_togo
 *
 * GMX_TX_PAUSE_TOGO = Packet TX Amount of time remaining to backpressure
 *
 */
union cvmx_gmxx_txx_pause_togo {
	u64 u64;
	struct cvmx_gmxx_txx_pause_togo_s {
		u64 reserved_32_63 : 32;
		u64 msg_time : 16;
		u64 time : 16;
	} s;
	struct cvmx_gmxx_txx_pause_togo_cn30xx {
		u64 reserved_16_63 : 48;
		u64 time : 16;
	} cn30xx;
	struct cvmx_gmxx_txx_pause_togo_cn30xx cn31xx;
	struct cvmx_gmxx_txx_pause_togo_cn30xx cn38xx;
	struct cvmx_gmxx_txx_pause_togo_cn30xx cn38xxp2;
	struct cvmx_gmxx_txx_pause_togo_cn30xx cn50xx;
	struct cvmx_gmxx_txx_pause_togo_s cn52xx;
	struct cvmx_gmxx_txx_pause_togo_s cn52xxp1;
	struct cvmx_gmxx_txx_pause_togo_s cn56xx;
	struct cvmx_gmxx_txx_pause_togo_cn30xx cn56xxp1;
	struct cvmx_gmxx_txx_pause_togo_cn30xx cn58xx;
	struct cvmx_gmxx_txx_pause_togo_cn30xx cn58xxp1;
	struct cvmx_gmxx_txx_pause_togo_s cn61xx;
	struct cvmx_gmxx_txx_pause_togo_s cn63xx;
	struct cvmx_gmxx_txx_pause_togo_s cn63xxp1;
	struct cvmx_gmxx_txx_pause_togo_s cn66xx;
	struct cvmx_gmxx_txx_pause_togo_s cn68xx;
	struct cvmx_gmxx_txx_pause_togo_s cn68xxp1;
	struct cvmx_gmxx_txx_pause_togo_s cn70xx;
	struct cvmx_gmxx_txx_pause_togo_s cn70xxp1;
	struct cvmx_gmxx_txx_pause_togo_s cnf71xx;
};

typedef union cvmx_gmxx_txx_pause_togo cvmx_gmxx_txx_pause_togo_t;

/**
 * cvmx_gmx#_tx#_pause_zero
 *
 * GMX_TX_PAUSE_ZERO = Packet TX Amount of time remaining to backpressure
 *
 */
union cvmx_gmxx_txx_pause_zero {
	u64 u64;
	struct cvmx_gmxx_txx_pause_zero_s {
		u64 reserved_1_63 : 63;
		u64 send : 1;
	} s;
	struct cvmx_gmxx_txx_pause_zero_s cn30xx;
	struct cvmx_gmxx_txx_pause_zero_s cn31xx;
	struct cvmx_gmxx_txx_pause_zero_s cn38xx;
	struct cvmx_gmxx_txx_pause_zero_s cn38xxp2;
	struct cvmx_gmxx_txx_pause_zero_s cn50xx;
	struct cvmx_gmxx_txx_pause_zero_s cn52xx;
	struct cvmx_gmxx_txx_pause_zero_s cn52xxp1;
	struct cvmx_gmxx_txx_pause_zero_s cn56xx;
	struct cvmx_gmxx_txx_pause_zero_s cn56xxp1;
	struct cvmx_gmxx_txx_pause_zero_s cn58xx;
	struct cvmx_gmxx_txx_pause_zero_s cn58xxp1;
	struct cvmx_gmxx_txx_pause_zero_s cn61xx;
	struct cvmx_gmxx_txx_pause_zero_s cn63xx;
	struct cvmx_gmxx_txx_pause_zero_s cn63xxp1;
	struct cvmx_gmxx_txx_pause_zero_s cn66xx;
	struct cvmx_gmxx_txx_pause_zero_s cn68xx;
	struct cvmx_gmxx_txx_pause_zero_s cn68xxp1;
	struct cvmx_gmxx_txx_pause_zero_s cn70xx;
	struct cvmx_gmxx_txx_pause_zero_s cn70xxp1;
	struct cvmx_gmxx_txx_pause_zero_s cnf71xx;
};

typedef union cvmx_gmxx_txx_pause_zero cvmx_gmxx_txx_pause_zero_t;

/**
 * cvmx_gmx#_tx#_pipe
 */
union cvmx_gmxx_txx_pipe {
	u64 u64;
	struct cvmx_gmxx_txx_pipe_s {
		u64 reserved_33_63 : 31;
		u64 ign_bp : 1;
		u64 reserved_21_31 : 11;
		u64 nump : 5;
		u64 reserved_7_15 : 9;
		u64 base : 7;
	} s;
	struct cvmx_gmxx_txx_pipe_s cn68xx;
	struct cvmx_gmxx_txx_pipe_s cn68xxp1;
};

typedef union cvmx_gmxx_txx_pipe cvmx_gmxx_txx_pipe_t;

/**
 * cvmx_gmx#_tx#_sgmii_ctl
 */
union cvmx_gmxx_txx_sgmii_ctl {
	u64 u64;
	struct cvmx_gmxx_txx_sgmii_ctl_s {
		u64 reserved_1_63 : 63;
		u64 align : 1;
	} s;
	struct cvmx_gmxx_txx_sgmii_ctl_s cn52xx;
	struct cvmx_gmxx_txx_sgmii_ctl_s cn52xxp1;
	struct cvmx_gmxx_txx_sgmii_ctl_s cn56xx;
	struct cvmx_gmxx_txx_sgmii_ctl_s cn56xxp1;
	struct cvmx_gmxx_txx_sgmii_ctl_s cn61xx;
	struct cvmx_gmxx_txx_sgmii_ctl_s cn63xx;
	struct cvmx_gmxx_txx_sgmii_ctl_s cn63xxp1;
	struct cvmx_gmxx_txx_sgmii_ctl_s cn66xx;
	struct cvmx_gmxx_txx_sgmii_ctl_s cn68xx;
	struct cvmx_gmxx_txx_sgmii_ctl_s cn68xxp1;
	struct cvmx_gmxx_txx_sgmii_ctl_s cn70xx;
	struct cvmx_gmxx_txx_sgmii_ctl_s cn70xxp1;
	struct cvmx_gmxx_txx_sgmii_ctl_s cnf71xx;
};

typedef union cvmx_gmxx_txx_sgmii_ctl cvmx_gmxx_txx_sgmii_ctl_t;

/**
 * cvmx_gmx#_tx#_slot
 *
 * GMX_TX_SLOT = Packet TX Slottime Counter
 *
 */
union cvmx_gmxx_txx_slot {
	u64 u64;
	struct cvmx_gmxx_txx_slot_s {
		u64 reserved_10_63 : 54;
		u64 slot : 10;
	} s;
	struct cvmx_gmxx_txx_slot_s cn30xx;
	struct cvmx_gmxx_txx_slot_s cn31xx;
	struct cvmx_gmxx_txx_slot_s cn38xx;
	struct cvmx_gmxx_txx_slot_s cn38xxp2;
	struct cvmx_gmxx_txx_slot_s cn50xx;
	struct cvmx_gmxx_txx_slot_s cn52xx;
	struct cvmx_gmxx_txx_slot_s cn52xxp1;
	struct cvmx_gmxx_txx_slot_s cn56xx;
	struct cvmx_gmxx_txx_slot_s cn56xxp1;
	struct cvmx_gmxx_txx_slot_s cn58xx;
	struct cvmx_gmxx_txx_slot_s cn58xxp1;
	struct cvmx_gmxx_txx_slot_s cn61xx;
	struct cvmx_gmxx_txx_slot_s cn63xx;
	struct cvmx_gmxx_txx_slot_s cn63xxp1;
	struct cvmx_gmxx_txx_slot_s cn66xx;
	struct cvmx_gmxx_txx_slot_s cn68xx;
	struct cvmx_gmxx_txx_slot_s cn68xxp1;
	struct cvmx_gmxx_txx_slot_s cn70xx;
	struct cvmx_gmxx_txx_slot_s cn70xxp1;
	struct cvmx_gmxx_txx_slot_s cnf71xx;
};

typedef union cvmx_gmxx_txx_slot cvmx_gmxx_txx_slot_t;

/**
 * cvmx_gmx#_tx#_soft_pause
 *
 * GMX_TX_SOFT_PAUSE = Packet TX Software Pause
 *
 */
union cvmx_gmxx_txx_soft_pause {
	u64 u64;
	struct cvmx_gmxx_txx_soft_pause_s {
		u64 reserved_16_63 : 48;
		u64 time : 16;
	} s;
	struct cvmx_gmxx_txx_soft_pause_s cn30xx;
	struct cvmx_gmxx_txx_soft_pause_s cn31xx;
	struct cvmx_gmxx_txx_soft_pause_s cn38xx;
	struct cvmx_gmxx_txx_soft_pause_s cn38xxp2;
	struct cvmx_gmxx_txx_soft_pause_s cn50xx;
	struct cvmx_gmxx_txx_soft_pause_s cn52xx;
	struct cvmx_gmxx_txx_soft_pause_s cn52xxp1;
	struct cvmx_gmxx_txx_soft_pause_s cn56xx;
	struct cvmx_gmxx_txx_soft_pause_s cn56xxp1;
	struct cvmx_gmxx_txx_soft_pause_s cn58xx;
	struct cvmx_gmxx_txx_soft_pause_s cn58xxp1;
	struct cvmx_gmxx_txx_soft_pause_s cn61xx;
	struct cvmx_gmxx_txx_soft_pause_s cn63xx;
	struct cvmx_gmxx_txx_soft_pause_s cn63xxp1;
	struct cvmx_gmxx_txx_soft_pause_s cn66xx;
	struct cvmx_gmxx_txx_soft_pause_s cn68xx;
	struct cvmx_gmxx_txx_soft_pause_s cn68xxp1;
	struct cvmx_gmxx_txx_soft_pause_s cn70xx;
	struct cvmx_gmxx_txx_soft_pause_s cn70xxp1;
	struct cvmx_gmxx_txx_soft_pause_s cnf71xx;
};

typedef union cvmx_gmxx_txx_soft_pause cvmx_gmxx_txx_soft_pause_t;

/**
 * cvmx_gmx#_tx#_stat0
 *
 * GMX_TX_STAT0 = GMX_TX_STATS_XSDEF / GMX_TX_STATS_XSCOL
 *
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 */
union cvmx_gmxx_txx_stat0 {
	u64 u64;
	struct cvmx_gmxx_txx_stat0_s {
		u64 xsdef : 32;
		u64 xscol : 32;
	} s;
	struct cvmx_gmxx_txx_stat0_s cn30xx;
	struct cvmx_gmxx_txx_stat0_s cn31xx;
	struct cvmx_gmxx_txx_stat0_s cn38xx;
	struct cvmx_gmxx_txx_stat0_s cn38xxp2;
	struct cvmx_gmxx_txx_stat0_s cn50xx;
	struct cvmx_gmxx_txx_stat0_s cn52xx;
	struct cvmx_gmxx_txx_stat0_s cn52xxp1;
	struct cvmx_gmxx_txx_stat0_s cn56xx;
	struct cvmx_gmxx_txx_stat0_s cn56xxp1;
	struct cvmx_gmxx_txx_stat0_s cn58xx;
	struct cvmx_gmxx_txx_stat0_s cn58xxp1;
	struct cvmx_gmxx_txx_stat0_s cn61xx;
	struct cvmx_gmxx_txx_stat0_s cn63xx;
	struct cvmx_gmxx_txx_stat0_s cn63xxp1;
	struct cvmx_gmxx_txx_stat0_s cn66xx;
	struct cvmx_gmxx_txx_stat0_s cn68xx;
	struct cvmx_gmxx_txx_stat0_s cn68xxp1;
	struct cvmx_gmxx_txx_stat0_s cn70xx;
	struct cvmx_gmxx_txx_stat0_s cn70xxp1;
	struct cvmx_gmxx_txx_stat0_s cnf71xx;
};

typedef union cvmx_gmxx_txx_stat0 cvmx_gmxx_txx_stat0_t;

/**
 * cvmx_gmx#_tx#_stat1
 *
 * GMX_TX_STAT1 = GMX_TX_STATS_SCOL  / GMX_TX_STATS_MCOL
 *
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 */
union cvmx_gmxx_txx_stat1 {
	u64 u64;
	struct cvmx_gmxx_txx_stat1_s {
		u64 scol : 32;
		u64 mcol : 32;
	} s;
	struct cvmx_gmxx_txx_stat1_s cn30xx;
	struct cvmx_gmxx_txx_stat1_s cn31xx;
	struct cvmx_gmxx_txx_stat1_s cn38xx;
	struct cvmx_gmxx_txx_stat1_s cn38xxp2;
	struct cvmx_gmxx_txx_stat1_s cn50xx;
	struct cvmx_gmxx_txx_stat1_s cn52xx;
	struct cvmx_gmxx_txx_stat1_s cn52xxp1;
	struct cvmx_gmxx_txx_stat1_s cn56xx;
	struct cvmx_gmxx_txx_stat1_s cn56xxp1;
	struct cvmx_gmxx_txx_stat1_s cn58xx;
	struct cvmx_gmxx_txx_stat1_s cn58xxp1;
	struct cvmx_gmxx_txx_stat1_s cn61xx;
	struct cvmx_gmxx_txx_stat1_s cn63xx;
	struct cvmx_gmxx_txx_stat1_s cn63xxp1;
	struct cvmx_gmxx_txx_stat1_s cn66xx;
	struct cvmx_gmxx_txx_stat1_s cn68xx;
	struct cvmx_gmxx_txx_stat1_s cn68xxp1;
	struct cvmx_gmxx_txx_stat1_s cn70xx;
	struct cvmx_gmxx_txx_stat1_s cn70xxp1;
	struct cvmx_gmxx_txx_stat1_s cnf71xx;
};

typedef union cvmx_gmxx_txx_stat1 cvmx_gmxx_txx_stat1_t;

/**
 * cvmx_gmx#_tx#_stat2
 *
 * GMX_TX_STAT2 = GMX_TX_STATS_OCTS
 *
 *
 * Notes:
 * - Octect counts are the sum of all data transmitted on the wire including
 *   packet data, pad bytes, fcs bytes, pause bytes, and jam bytes.  The octect
 *   counts do not include PREAMBLE byte or EXTEND cycles.
 * - Cleared either by a write (of any value) or a read when GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 */
union cvmx_gmxx_txx_stat2 {
	u64 u64;
	struct cvmx_gmxx_txx_stat2_s {
		u64 reserved_48_63 : 16;
		u64 octs : 48;
	} s;
	struct cvmx_gmxx_txx_stat2_s cn30xx;
	struct cvmx_gmxx_txx_stat2_s cn31xx;
	struct cvmx_gmxx_txx_stat2_s cn38xx;
	struct cvmx_gmxx_txx_stat2_s cn38xxp2;
	struct cvmx_gmxx_txx_stat2_s cn50xx;
	struct cvmx_gmxx_txx_stat2_s cn52xx;
	struct cvmx_gmxx_txx_stat2_s cn52xxp1;
	struct cvmx_gmxx_txx_stat2_s cn56xx;
	struct cvmx_gmxx_txx_stat2_s cn56xxp1;
	struct cvmx_gmxx_txx_stat2_s cn58xx;
	struct cvmx_gmxx_txx_stat2_s cn58xxp1;
	struct cvmx_gmxx_txx_stat2_s cn61xx;
	struct cvmx_gmxx_txx_stat2_s cn63xx;
	struct cvmx_gmxx_txx_stat2_s cn63xxp1;
	struct cvmx_gmxx_txx_stat2_s cn66xx;
	struct cvmx_gmxx_txx_stat2_s cn68xx;
	struct cvmx_gmxx_txx_stat2_s cn68xxp1;
	struct cvmx_gmxx_txx_stat2_s cn70xx;
	struct cvmx_gmxx_txx_stat2_s cn70xxp1;
	struct cvmx_gmxx_txx_stat2_s cnf71xx;
};

typedef union cvmx_gmxx_txx_stat2 cvmx_gmxx_txx_stat2_t;

/**
 * cvmx_gmx#_tx#_stat3
 *
 * GMX_TX_STAT3 = GMX_TX_STATS_PKTS
 *
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 */
union cvmx_gmxx_txx_stat3 {
	u64 u64;
	struct cvmx_gmxx_txx_stat3_s {
		u64 reserved_32_63 : 32;
		u64 pkts : 32;
	} s;
	struct cvmx_gmxx_txx_stat3_s cn30xx;
	struct cvmx_gmxx_txx_stat3_s cn31xx;
	struct cvmx_gmxx_txx_stat3_s cn38xx;
	struct cvmx_gmxx_txx_stat3_s cn38xxp2;
	struct cvmx_gmxx_txx_stat3_s cn50xx;
	struct cvmx_gmxx_txx_stat3_s cn52xx;
	struct cvmx_gmxx_txx_stat3_s cn52xxp1;
	struct cvmx_gmxx_txx_stat3_s cn56xx;
	struct cvmx_gmxx_txx_stat3_s cn56xxp1;
	struct cvmx_gmxx_txx_stat3_s cn58xx;
	struct cvmx_gmxx_txx_stat3_s cn58xxp1;
	struct cvmx_gmxx_txx_stat3_s cn61xx;
	struct cvmx_gmxx_txx_stat3_s cn63xx;
	struct cvmx_gmxx_txx_stat3_s cn63xxp1;
	struct cvmx_gmxx_txx_stat3_s cn66xx;
	struct cvmx_gmxx_txx_stat3_s cn68xx;
	struct cvmx_gmxx_txx_stat3_s cn68xxp1;
	struct cvmx_gmxx_txx_stat3_s cn70xx;
	struct cvmx_gmxx_txx_stat3_s cn70xxp1;
	struct cvmx_gmxx_txx_stat3_s cnf71xx;
};

typedef union cvmx_gmxx_txx_stat3 cvmx_gmxx_txx_stat3_t;

/**
 * cvmx_gmx#_tx#_stat4
 *
 * GMX_TX_STAT4 = GMX_TX_STATS_HIST1 (64) / GMX_TX_STATS_HIST0 (<64)
 *
 *
 * Notes:
 * - Packet length is the sum of all data transmitted on the wire for the given
 *   packet including packet data, pad bytes, fcs bytes, pause bytes, and jam
 *   bytes.  The octect counts do not include PREAMBLE byte or EXTEND cycles.
 * - Cleared either by a write (of any value) or a read when GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 */
union cvmx_gmxx_txx_stat4 {
	u64 u64;
	struct cvmx_gmxx_txx_stat4_s {
		u64 hist1 : 32;
		u64 hist0 : 32;
	} s;
	struct cvmx_gmxx_txx_stat4_s cn30xx;
	struct cvmx_gmxx_txx_stat4_s cn31xx;
	struct cvmx_gmxx_txx_stat4_s cn38xx;
	struct cvmx_gmxx_txx_stat4_s cn38xxp2;
	struct cvmx_gmxx_txx_stat4_s cn50xx;
	struct cvmx_gmxx_txx_stat4_s cn52xx;
	struct cvmx_gmxx_txx_stat4_s cn52xxp1;
	struct cvmx_gmxx_txx_stat4_s cn56xx;
	struct cvmx_gmxx_txx_stat4_s cn56xxp1;
	struct cvmx_gmxx_txx_stat4_s cn58xx;
	struct cvmx_gmxx_txx_stat4_s cn58xxp1;
	struct cvmx_gmxx_txx_stat4_s cn61xx;
	struct cvmx_gmxx_txx_stat4_s cn63xx;
	struct cvmx_gmxx_txx_stat4_s cn63xxp1;
	struct cvmx_gmxx_txx_stat4_s cn66xx;
	struct cvmx_gmxx_txx_stat4_s cn68xx;
	struct cvmx_gmxx_txx_stat4_s cn68xxp1;
	struct cvmx_gmxx_txx_stat4_s cn70xx;
	struct cvmx_gmxx_txx_stat4_s cn70xxp1;
	struct cvmx_gmxx_txx_stat4_s cnf71xx;
};

typedef union cvmx_gmxx_txx_stat4 cvmx_gmxx_txx_stat4_t;

/**
 * cvmx_gmx#_tx#_stat5
 *
 * GMX_TX_STAT5 = GMX_TX_STATS_HIST3 (128- 255) / GMX_TX_STATS_HIST2 (65- 127)
 *
 *
 * Notes:
 * - Packet length is the sum of all data transmitted on the wire for the given
 *   packet including packet data, pad bytes, fcs bytes, pause bytes, and jam
 *   bytes.  The octect counts do not include PREAMBLE byte or EXTEND cycles.
 * - Cleared either by a write (of any value) or a read when GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 */
union cvmx_gmxx_txx_stat5 {
	u64 u64;
	struct cvmx_gmxx_txx_stat5_s {
		u64 hist3 : 32;
		u64 hist2 : 32;
	} s;
	struct cvmx_gmxx_txx_stat5_s cn30xx;
	struct cvmx_gmxx_txx_stat5_s cn31xx;
	struct cvmx_gmxx_txx_stat5_s cn38xx;
	struct cvmx_gmxx_txx_stat5_s cn38xxp2;
	struct cvmx_gmxx_txx_stat5_s cn50xx;
	struct cvmx_gmxx_txx_stat5_s cn52xx;
	struct cvmx_gmxx_txx_stat5_s cn52xxp1;
	struct cvmx_gmxx_txx_stat5_s cn56xx;
	struct cvmx_gmxx_txx_stat5_s cn56xxp1;
	struct cvmx_gmxx_txx_stat5_s cn58xx;
	struct cvmx_gmxx_txx_stat5_s cn58xxp1;
	struct cvmx_gmxx_txx_stat5_s cn61xx;
	struct cvmx_gmxx_txx_stat5_s cn63xx;
	struct cvmx_gmxx_txx_stat5_s cn63xxp1;
	struct cvmx_gmxx_txx_stat5_s cn66xx;
	struct cvmx_gmxx_txx_stat5_s cn68xx;
	struct cvmx_gmxx_txx_stat5_s cn68xxp1;
	struct cvmx_gmxx_txx_stat5_s cn70xx;
	struct cvmx_gmxx_txx_stat5_s cn70xxp1;
	struct cvmx_gmxx_txx_stat5_s cnf71xx;
};

typedef union cvmx_gmxx_txx_stat5 cvmx_gmxx_txx_stat5_t;

/**
 * cvmx_gmx#_tx#_stat6
 *
 * GMX_TX_STAT6 = GMX_TX_STATS_HIST5 (512-1023) / GMX_TX_STATS_HIST4 (256-511)
 *
 *
 * Notes:
 * - Packet length is the sum of all data transmitted on the wire for the given
 *   packet including packet data, pad bytes, fcs bytes, pause bytes, and jam
 *   bytes.  The octect counts do not include PREAMBLE byte or EXTEND cycles.
 * - Cleared either by a write (of any value) or a read when GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 */
union cvmx_gmxx_txx_stat6 {
	u64 u64;
	struct cvmx_gmxx_txx_stat6_s {
		u64 hist5 : 32;
		u64 hist4 : 32;
	} s;
	struct cvmx_gmxx_txx_stat6_s cn30xx;
	struct cvmx_gmxx_txx_stat6_s cn31xx;
	struct cvmx_gmxx_txx_stat6_s cn38xx;
	struct cvmx_gmxx_txx_stat6_s cn38xxp2;
	struct cvmx_gmxx_txx_stat6_s cn50xx;
	struct cvmx_gmxx_txx_stat6_s cn52xx;
	struct cvmx_gmxx_txx_stat6_s cn52xxp1;
	struct cvmx_gmxx_txx_stat6_s cn56xx;
	struct cvmx_gmxx_txx_stat6_s cn56xxp1;
	struct cvmx_gmxx_txx_stat6_s cn58xx;
	struct cvmx_gmxx_txx_stat6_s cn58xxp1;
	struct cvmx_gmxx_txx_stat6_s cn61xx;
	struct cvmx_gmxx_txx_stat6_s cn63xx;
	struct cvmx_gmxx_txx_stat6_s cn63xxp1;
	struct cvmx_gmxx_txx_stat6_s cn66xx;
	struct cvmx_gmxx_txx_stat6_s cn68xx;
	struct cvmx_gmxx_txx_stat6_s cn68xxp1;
	struct cvmx_gmxx_txx_stat6_s cn70xx;
	struct cvmx_gmxx_txx_stat6_s cn70xxp1;
	struct cvmx_gmxx_txx_stat6_s cnf71xx;
};

typedef union cvmx_gmxx_txx_stat6 cvmx_gmxx_txx_stat6_t;

/**
 * cvmx_gmx#_tx#_stat7
 *
 * GMX_TX_STAT7 = GMX_TX_STATS_HIST7 (1024-1518) / GMX_TX_STATS_HIST6 (>1518)
 *
 *
 * Notes:
 * - Packet length is the sum of all data transmitted on the wire for the given
 *   packet including packet data, pad bytes, fcs bytes, pause bytes, and jam
 *   bytes.  The octect counts do not include PREAMBLE byte or EXTEND cycles.
 * - Cleared either by a write (of any value) or a read when GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 */
union cvmx_gmxx_txx_stat7 {
	u64 u64;
	struct cvmx_gmxx_txx_stat7_s {
		u64 hist7 : 32;
		u64 hist6 : 32;
	} s;
	struct cvmx_gmxx_txx_stat7_s cn30xx;
	struct cvmx_gmxx_txx_stat7_s cn31xx;
	struct cvmx_gmxx_txx_stat7_s cn38xx;
	struct cvmx_gmxx_txx_stat7_s cn38xxp2;
	struct cvmx_gmxx_txx_stat7_s cn50xx;
	struct cvmx_gmxx_txx_stat7_s cn52xx;
	struct cvmx_gmxx_txx_stat7_s cn52xxp1;
	struct cvmx_gmxx_txx_stat7_s cn56xx;
	struct cvmx_gmxx_txx_stat7_s cn56xxp1;
	struct cvmx_gmxx_txx_stat7_s cn58xx;
	struct cvmx_gmxx_txx_stat7_s cn58xxp1;
	struct cvmx_gmxx_txx_stat7_s cn61xx;
	struct cvmx_gmxx_txx_stat7_s cn63xx;
	struct cvmx_gmxx_txx_stat7_s cn63xxp1;
	struct cvmx_gmxx_txx_stat7_s cn66xx;
	struct cvmx_gmxx_txx_stat7_s cn68xx;
	struct cvmx_gmxx_txx_stat7_s cn68xxp1;
	struct cvmx_gmxx_txx_stat7_s cn70xx;
	struct cvmx_gmxx_txx_stat7_s cn70xxp1;
	struct cvmx_gmxx_txx_stat7_s cnf71xx;
};

typedef union cvmx_gmxx_txx_stat7 cvmx_gmxx_txx_stat7_t;

/**
 * cvmx_gmx#_tx#_stat8
 *
 * GMX_TX_STAT8 = GMX_TX_STATS_MCST  / GMX_TX_STATS_BCST
 *
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Note, GMX determines if the packet is MCST or BCST from the DMAC of the
 *   packet.  GMX assumes that the DMAC lies in the first 6 bytes of the packet
 *   as per the 802.3 frame definition.  If the system requires additional data
 *   before the L2 header, then the MCST and BCST counters may not reflect
 *   reality and should be ignored by software.
 */
union cvmx_gmxx_txx_stat8 {
	u64 u64;
	struct cvmx_gmxx_txx_stat8_s {
		u64 mcst : 32;
		u64 bcst : 32;
	} s;
	struct cvmx_gmxx_txx_stat8_s cn30xx;
	struct cvmx_gmxx_txx_stat8_s cn31xx;
	struct cvmx_gmxx_txx_stat8_s cn38xx;
	struct cvmx_gmxx_txx_stat8_s cn38xxp2;
	struct cvmx_gmxx_txx_stat8_s cn50xx;
	struct cvmx_gmxx_txx_stat8_s cn52xx;
	struct cvmx_gmxx_txx_stat8_s cn52xxp1;
	struct cvmx_gmxx_txx_stat8_s cn56xx;
	struct cvmx_gmxx_txx_stat8_s cn56xxp1;
	struct cvmx_gmxx_txx_stat8_s cn58xx;
	struct cvmx_gmxx_txx_stat8_s cn58xxp1;
	struct cvmx_gmxx_txx_stat8_s cn61xx;
	struct cvmx_gmxx_txx_stat8_s cn63xx;
	struct cvmx_gmxx_txx_stat8_s cn63xxp1;
	struct cvmx_gmxx_txx_stat8_s cn66xx;
	struct cvmx_gmxx_txx_stat8_s cn68xx;
	struct cvmx_gmxx_txx_stat8_s cn68xxp1;
	struct cvmx_gmxx_txx_stat8_s cn70xx;
	struct cvmx_gmxx_txx_stat8_s cn70xxp1;
	struct cvmx_gmxx_txx_stat8_s cnf71xx;
};

typedef union cvmx_gmxx_txx_stat8 cvmx_gmxx_txx_stat8_t;

/**
 * cvmx_gmx#_tx#_stat9
 *
 * GMX_TX_STAT9 = GMX_TX_STATS_UNDFLW / GMX_TX_STATS_CTL
 *
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 */
union cvmx_gmxx_txx_stat9 {
	u64 u64;
	struct cvmx_gmxx_txx_stat9_s {
		u64 undflw : 32;
		u64 ctl : 32;
	} s;
	struct cvmx_gmxx_txx_stat9_s cn30xx;
	struct cvmx_gmxx_txx_stat9_s cn31xx;
	struct cvmx_gmxx_txx_stat9_s cn38xx;
	struct cvmx_gmxx_txx_stat9_s cn38xxp2;
	struct cvmx_gmxx_txx_stat9_s cn50xx;
	struct cvmx_gmxx_txx_stat9_s cn52xx;
	struct cvmx_gmxx_txx_stat9_s cn52xxp1;
	struct cvmx_gmxx_txx_stat9_s cn56xx;
	struct cvmx_gmxx_txx_stat9_s cn56xxp1;
	struct cvmx_gmxx_txx_stat9_s cn58xx;
	struct cvmx_gmxx_txx_stat9_s cn58xxp1;
	struct cvmx_gmxx_txx_stat9_s cn61xx;
	struct cvmx_gmxx_txx_stat9_s cn63xx;
	struct cvmx_gmxx_txx_stat9_s cn63xxp1;
	struct cvmx_gmxx_txx_stat9_s cn66xx;
	struct cvmx_gmxx_txx_stat9_s cn68xx;
	struct cvmx_gmxx_txx_stat9_s cn68xxp1;
	struct cvmx_gmxx_txx_stat9_s cn70xx;
	struct cvmx_gmxx_txx_stat9_s cn70xxp1;
	struct cvmx_gmxx_txx_stat9_s cnf71xx;
};

typedef union cvmx_gmxx_txx_stat9 cvmx_gmxx_txx_stat9_t;

/**
 * cvmx_gmx#_tx#_stats_ctl
 *
 * GMX_TX_STATS_CTL = TX Stats Control register
 *
 */
union cvmx_gmxx_txx_stats_ctl {
	u64 u64;
	struct cvmx_gmxx_txx_stats_ctl_s {
		u64 reserved_1_63 : 63;
		u64 rd_clr : 1;
	} s;
	struct cvmx_gmxx_txx_stats_ctl_s cn30xx;
	struct cvmx_gmxx_txx_stats_ctl_s cn31xx;
	struct cvmx_gmxx_txx_stats_ctl_s cn38xx;
	struct cvmx_gmxx_txx_stats_ctl_s cn38xxp2;
	struct cvmx_gmxx_txx_stats_ctl_s cn50xx;
	struct cvmx_gmxx_txx_stats_ctl_s cn52xx;
	struct cvmx_gmxx_txx_stats_ctl_s cn52xxp1;
	struct cvmx_gmxx_txx_stats_ctl_s cn56xx;
	struct cvmx_gmxx_txx_stats_ctl_s cn56xxp1;
	struct cvmx_gmxx_txx_stats_ctl_s cn58xx;
	struct cvmx_gmxx_txx_stats_ctl_s cn58xxp1;
	struct cvmx_gmxx_txx_stats_ctl_s cn61xx;
	struct cvmx_gmxx_txx_stats_ctl_s cn63xx;
	struct cvmx_gmxx_txx_stats_ctl_s cn63xxp1;
	struct cvmx_gmxx_txx_stats_ctl_s cn66xx;
	struct cvmx_gmxx_txx_stats_ctl_s cn68xx;
	struct cvmx_gmxx_txx_stats_ctl_s cn68xxp1;
	struct cvmx_gmxx_txx_stats_ctl_s cn70xx;
	struct cvmx_gmxx_txx_stats_ctl_s cn70xxp1;
	struct cvmx_gmxx_txx_stats_ctl_s cnf71xx;
};

typedef union cvmx_gmxx_txx_stats_ctl cvmx_gmxx_txx_stats_ctl_t;

/**
 * cvmx_gmx#_tx#_thresh
 *
 * Per Port
 * GMX_TX_THRESH = Packet TX Threshold
 */
union cvmx_gmxx_txx_thresh {
	u64 u64;
	struct cvmx_gmxx_txx_thresh_s {
		u64 reserved_10_63 : 54;
		u64 cnt : 10;
	} s;
	struct cvmx_gmxx_txx_thresh_cn30xx {
		u64 reserved_7_63 : 57;
		u64 cnt : 7;
	} cn30xx;
	struct cvmx_gmxx_txx_thresh_cn30xx cn31xx;
	struct cvmx_gmxx_txx_thresh_cn38xx {
		u64 reserved_9_63 : 55;
		u64 cnt : 9;
	} cn38xx;
	struct cvmx_gmxx_txx_thresh_cn38xx cn38xxp2;
	struct cvmx_gmxx_txx_thresh_cn30xx cn50xx;
	struct cvmx_gmxx_txx_thresh_cn38xx cn52xx;
	struct cvmx_gmxx_txx_thresh_cn38xx cn52xxp1;
	struct cvmx_gmxx_txx_thresh_cn38xx cn56xx;
	struct cvmx_gmxx_txx_thresh_cn38xx cn56xxp1;
	struct cvmx_gmxx_txx_thresh_cn38xx cn58xx;
	struct cvmx_gmxx_txx_thresh_cn38xx cn58xxp1;
	struct cvmx_gmxx_txx_thresh_cn38xx cn61xx;
	struct cvmx_gmxx_txx_thresh_cn38xx cn63xx;
	struct cvmx_gmxx_txx_thresh_cn38xx cn63xxp1;
	struct cvmx_gmxx_txx_thresh_cn38xx cn66xx;
	struct cvmx_gmxx_txx_thresh_s cn68xx;
	struct cvmx_gmxx_txx_thresh_s cn68xxp1;
	struct cvmx_gmxx_txx_thresh_cn38xx cn70xx;
	struct cvmx_gmxx_txx_thresh_cn38xx cn70xxp1;
	struct cvmx_gmxx_txx_thresh_cn38xx cnf71xx;
};

typedef union cvmx_gmxx_txx_thresh cvmx_gmxx_txx_thresh_t;

/**
 * cvmx_gmx#_tx_bp
 *
 * GMX_TX_BP = Packet Interface TX BackPressure Register
 *
 *
 * Notes:
 * In XAUI mode, only the lsb (corresponding to port0) of BP is used.
 *
 */
union cvmx_gmxx_tx_bp {
	u64 u64;
	struct cvmx_gmxx_tx_bp_s {
		u64 reserved_4_63 : 60;
		u64 bp : 4;
	} s;
	struct cvmx_gmxx_tx_bp_cn30xx {
		u64 reserved_3_63 : 61;
		u64 bp : 3;
	} cn30xx;
	struct cvmx_gmxx_tx_bp_cn30xx cn31xx;
	struct cvmx_gmxx_tx_bp_s cn38xx;
	struct cvmx_gmxx_tx_bp_s cn38xxp2;
	struct cvmx_gmxx_tx_bp_cn30xx cn50xx;
	struct cvmx_gmxx_tx_bp_s cn52xx;
	struct cvmx_gmxx_tx_bp_s cn52xxp1;
	struct cvmx_gmxx_tx_bp_s cn56xx;
	struct cvmx_gmxx_tx_bp_s cn56xxp1;
	struct cvmx_gmxx_tx_bp_s cn58xx;
	struct cvmx_gmxx_tx_bp_s cn58xxp1;
	struct cvmx_gmxx_tx_bp_s cn61xx;
	struct cvmx_gmxx_tx_bp_s cn63xx;
	struct cvmx_gmxx_tx_bp_s cn63xxp1;
	struct cvmx_gmxx_tx_bp_s cn66xx;
	struct cvmx_gmxx_tx_bp_s cn68xx;
	struct cvmx_gmxx_tx_bp_s cn68xxp1;
	struct cvmx_gmxx_tx_bp_s cn70xx;
	struct cvmx_gmxx_tx_bp_s cn70xxp1;
	struct cvmx_gmxx_tx_bp_cnf71xx {
		u64 reserved_2_63 : 62;
		u64 bp : 2;
	} cnf71xx;
};

typedef union cvmx_gmxx_tx_bp cvmx_gmxx_tx_bp_t;

/**
 * cvmx_gmx#_tx_clk_msk#
 *
 * GMX_TX_CLK_MSK = GMX Clock Select
 *
 */
union cvmx_gmxx_tx_clk_mskx {
	u64 u64;
	struct cvmx_gmxx_tx_clk_mskx_s {
		u64 reserved_1_63 : 63;
		u64 msk : 1;
	} s;
	struct cvmx_gmxx_tx_clk_mskx_s cn30xx;
	struct cvmx_gmxx_tx_clk_mskx_s cn50xx;
};

typedef union cvmx_gmxx_tx_clk_mskx cvmx_gmxx_tx_clk_mskx_t;

/**
 * cvmx_gmx#_tx_col_attempt
 *
 * GMX_TX_COL_ATTEMPT = Packet TX collision attempts before dropping frame
 *
 */
union cvmx_gmxx_tx_col_attempt {
	u64 u64;
	struct cvmx_gmxx_tx_col_attempt_s {
		u64 reserved_5_63 : 59;
		u64 limit : 5;
	} s;
	struct cvmx_gmxx_tx_col_attempt_s cn30xx;
	struct cvmx_gmxx_tx_col_attempt_s cn31xx;
	struct cvmx_gmxx_tx_col_attempt_s cn38xx;
	struct cvmx_gmxx_tx_col_attempt_s cn38xxp2;
	struct cvmx_gmxx_tx_col_attempt_s cn50xx;
	struct cvmx_gmxx_tx_col_attempt_s cn52xx;
	struct cvmx_gmxx_tx_col_attempt_s cn52xxp1;
	struct cvmx_gmxx_tx_col_attempt_s cn56xx;
	struct cvmx_gmxx_tx_col_attempt_s cn56xxp1;
	struct cvmx_gmxx_tx_col_attempt_s cn58xx;
	struct cvmx_gmxx_tx_col_attempt_s cn58xxp1;
	struct cvmx_gmxx_tx_col_attempt_s cn61xx;
	struct cvmx_gmxx_tx_col_attempt_s cn63xx;
	struct cvmx_gmxx_tx_col_attempt_s cn63xxp1;
	struct cvmx_gmxx_tx_col_attempt_s cn66xx;
	struct cvmx_gmxx_tx_col_attempt_s cn68xx;
	struct cvmx_gmxx_tx_col_attempt_s cn68xxp1;
	struct cvmx_gmxx_tx_col_attempt_s cn70xx;
	struct cvmx_gmxx_tx_col_attempt_s cn70xxp1;
	struct cvmx_gmxx_tx_col_attempt_s cnf71xx;
};

typedef union cvmx_gmxx_tx_col_attempt cvmx_gmxx_tx_col_attempt_t;

/**
 * cvmx_gmx#_tx_corrupt
 *
 * GMX_TX_CORRUPT = TX - Corrupt TX packets with the ERR bit set
 *
 *
 * Notes:
 * Packets sent from PKO with the ERR wire asserted will be corrupted by
 * the transmitter if CORRUPT[prt] is set (XAUI uses prt==0).
 *
 * Corruption means that GMX will send a bad FCS value.  If GMX_TX_APPEND[FCS]
 * is clear then no FCS is sent and the GMX cannot corrupt it.  The corrupt FCS
 * value is 0xeeeeeeee for SGMII/1000Base-X and 4 bytes of the error
 * propagation code in XAUI mode.
 */
union cvmx_gmxx_tx_corrupt {
	u64 u64;
	struct cvmx_gmxx_tx_corrupt_s {
		u64 reserved_4_63 : 60;
		u64 corrupt : 4;
	} s;
	struct cvmx_gmxx_tx_corrupt_cn30xx {
		u64 reserved_3_63 : 61;
		u64 corrupt : 3;
	} cn30xx;
	struct cvmx_gmxx_tx_corrupt_cn30xx cn31xx;
	struct cvmx_gmxx_tx_corrupt_s cn38xx;
	struct cvmx_gmxx_tx_corrupt_s cn38xxp2;
	struct cvmx_gmxx_tx_corrupt_cn30xx cn50xx;
	struct cvmx_gmxx_tx_corrupt_s cn52xx;
	struct cvmx_gmxx_tx_corrupt_s cn52xxp1;
	struct cvmx_gmxx_tx_corrupt_s cn56xx;
	struct cvmx_gmxx_tx_corrupt_s cn56xxp1;
	struct cvmx_gmxx_tx_corrupt_s cn58xx;
	struct cvmx_gmxx_tx_corrupt_s cn58xxp1;
	struct cvmx_gmxx_tx_corrupt_s cn61xx;
	struct cvmx_gmxx_tx_corrupt_s cn63xx;
	struct cvmx_gmxx_tx_corrupt_s cn63xxp1;
	struct cvmx_gmxx_tx_corrupt_s cn66xx;
	struct cvmx_gmxx_tx_corrupt_s cn68xx;
	struct cvmx_gmxx_tx_corrupt_s cn68xxp1;
	struct cvmx_gmxx_tx_corrupt_s cn70xx;
	struct cvmx_gmxx_tx_corrupt_s cn70xxp1;
	struct cvmx_gmxx_tx_corrupt_cnf71xx {
		u64 reserved_2_63 : 62;
		u64 corrupt : 2;
	} cnf71xx;
};

typedef union cvmx_gmxx_tx_corrupt cvmx_gmxx_tx_corrupt_t;

/**
 * cvmx_gmx#_tx_hg2_reg1
 *
 * Notes:
 * The TX_XOF[15:0] field in GMX(0)_TX_HG2_REG1 and the TX_XON[15:0] field in
 * GMX(0)_TX_HG2_REG2 register map to the same 16 physical flops. When written with address of
 * GMX(0)_TX_HG2_REG1, it will exhibit write 1 to set behavior and when written with address of
 * GMX(0)_TX_HG2_REG2, it will exhibit write 1 to clear behavior.
 * For reads, either address will return the $GMX(0)_TX_HG2_REG1 values.
 */
union cvmx_gmxx_tx_hg2_reg1 {
	u64 u64;
	struct cvmx_gmxx_tx_hg2_reg1_s {
		u64 reserved_16_63 : 48;
		u64 tx_xof : 16;
	} s;
	struct cvmx_gmxx_tx_hg2_reg1_s cn52xx;
	struct cvmx_gmxx_tx_hg2_reg1_s cn52xxp1;
	struct cvmx_gmxx_tx_hg2_reg1_s cn56xx;
	struct cvmx_gmxx_tx_hg2_reg1_s cn61xx;
	struct cvmx_gmxx_tx_hg2_reg1_s cn63xx;
	struct cvmx_gmxx_tx_hg2_reg1_s cn63xxp1;
	struct cvmx_gmxx_tx_hg2_reg1_s cn66xx;
	struct cvmx_gmxx_tx_hg2_reg1_s cn68xx;
	struct cvmx_gmxx_tx_hg2_reg1_s cn68xxp1;
	struct cvmx_gmxx_tx_hg2_reg1_s cn70xx;
	struct cvmx_gmxx_tx_hg2_reg1_s cn70xxp1;
	struct cvmx_gmxx_tx_hg2_reg1_s cnf71xx;
};

typedef union cvmx_gmxx_tx_hg2_reg1 cvmx_gmxx_tx_hg2_reg1_t;

/**
 * cvmx_gmx#_tx_hg2_reg2
 *
 * Notes:
 * The TX_XOF[15:0] field in GMX(0)_TX_HG2_REG1 and the TX_XON[15:0] field in
 * GMX(0)_TX_HG2_REG2 register map to the same 16 physical flops. When written with address  of
 * GMX(0)_TX_HG2_REG1, it will exhibit write 1 to set behavior and when written with address of
 * GMX(0)_TX_HG2_REG2, it will exhibit write 1 to clear behavior.
 * For reads, either address will return the $GMX(0)_TX_HG2_REG1 values.
 */
union cvmx_gmxx_tx_hg2_reg2 {
	u64 u64;
	struct cvmx_gmxx_tx_hg2_reg2_s {
		u64 reserved_16_63 : 48;
		u64 tx_xon : 16;
	} s;
	struct cvmx_gmxx_tx_hg2_reg2_s cn52xx;
	struct cvmx_gmxx_tx_hg2_reg2_s cn52xxp1;
	struct cvmx_gmxx_tx_hg2_reg2_s cn56xx;
	struct cvmx_gmxx_tx_hg2_reg2_s cn61xx;
	struct cvmx_gmxx_tx_hg2_reg2_s cn63xx;
	struct cvmx_gmxx_tx_hg2_reg2_s cn63xxp1;
	struct cvmx_gmxx_tx_hg2_reg2_s cn66xx;
	struct cvmx_gmxx_tx_hg2_reg2_s cn68xx;
	struct cvmx_gmxx_tx_hg2_reg2_s cn68xxp1;
	struct cvmx_gmxx_tx_hg2_reg2_s cn70xx;
	struct cvmx_gmxx_tx_hg2_reg2_s cn70xxp1;
	struct cvmx_gmxx_tx_hg2_reg2_s cnf71xx;
};

typedef union cvmx_gmxx_tx_hg2_reg2 cvmx_gmxx_tx_hg2_reg2_t;

/**
 * cvmx_gmx#_tx_ifg
 *
 * GMX_TX_IFG = Packet TX Interframe Gap
 *
 *
 * Notes:
 * * Programming IFG1 and IFG2.
 *
 * For 10/100/1000Mbs half-duplex systems that require IEEE 802.3
 * compatibility, IFG1 must be in the range of 1-8, IFG2 must be in the range
 * of 4-12, and the IFG1+IFG2 sum must be 12.
 *
 * For 10/100/1000Mbs full-duplex systems that require IEEE 802.3
 * compatibility, IFG1 must be in the range of 1-11, IFG2 must be in the range
 * of 1-11, and the IFG1+IFG2 sum must be 12.
 *
 * For XAUI/10Gbs systems that require IEEE 802.3 compatibility, the
 * IFG1+IFG2 sum must be 12.  IFG1[1:0] and IFG2[1:0] must be zero.
 *
 * For all other systems, IFG1 and IFG2 can be any value in the range of
 * 1-15.  Allowing for a total possible IFG sum of 2-30.
 */
union cvmx_gmxx_tx_ifg {
	u64 u64;
	struct cvmx_gmxx_tx_ifg_s {
		u64 reserved_8_63 : 56;
		u64 ifg2 : 4;
		u64 ifg1 : 4;
	} s;
	struct cvmx_gmxx_tx_ifg_s cn30xx;
	struct cvmx_gmxx_tx_ifg_s cn31xx;
	struct cvmx_gmxx_tx_ifg_s cn38xx;
	struct cvmx_gmxx_tx_ifg_s cn38xxp2;
	struct cvmx_gmxx_tx_ifg_s cn50xx;
	struct cvmx_gmxx_tx_ifg_s cn52xx;
	struct cvmx_gmxx_tx_ifg_s cn52xxp1;
	struct cvmx_gmxx_tx_ifg_s cn56xx;
	struct cvmx_gmxx_tx_ifg_s cn56xxp1;
	struct cvmx_gmxx_tx_ifg_s cn58xx;
	struct cvmx_gmxx_tx_ifg_s cn58xxp1;
	struct cvmx_gmxx_tx_ifg_s cn61xx;
	struct cvmx_gmxx_tx_ifg_s cn63xx;
	struct cvmx_gmxx_tx_ifg_s cn63xxp1;
	struct cvmx_gmxx_tx_ifg_s cn66xx;
	struct cvmx_gmxx_tx_ifg_s cn68xx;
	struct cvmx_gmxx_tx_ifg_s cn68xxp1;
	struct cvmx_gmxx_tx_ifg_s cn70xx;
	struct cvmx_gmxx_tx_ifg_s cn70xxp1;
	struct cvmx_gmxx_tx_ifg_s cnf71xx;
};

typedef union cvmx_gmxx_tx_ifg cvmx_gmxx_tx_ifg_t;

/**
 * cvmx_gmx#_tx_int_en
 *
 * GMX_TX_INT_EN = Interrupt Enable
 *
 *
 * Notes:
 * In XAUI mode, only the lsb (corresponding to port0) of UNDFLW is used.
 *
 */
union cvmx_gmxx_tx_int_en {
	u64 u64;
	struct cvmx_gmxx_tx_int_en_s {
		u64 reserved_25_63 : 39;
		u64 xchange : 1;
		u64 ptp_lost : 4;
		u64 late_col : 4;
		u64 xsdef : 4;
		u64 xscol : 4;
		u64 reserved_6_7 : 2;
		u64 undflw : 4;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} s;
	struct cvmx_gmxx_tx_int_en_cn30xx {
		u64 reserved_19_63 : 45;
		u64 late_col : 3;
		u64 reserved_15_15 : 1;
		u64 xsdef : 3;
		u64 reserved_11_11 : 1;
		u64 xscol : 3;
		u64 reserved_5_7 : 3;
		u64 undflw : 3;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cn30xx;
	struct cvmx_gmxx_tx_int_en_cn31xx {
		u64 reserved_15_63 : 49;
		u64 xsdef : 3;
		u64 reserved_11_11 : 1;
		u64 xscol : 3;
		u64 reserved_5_7 : 3;
		u64 undflw : 3;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cn31xx;
	struct cvmx_gmxx_tx_int_en_cn38xx {
		u64 reserved_20_63 : 44;
		u64 late_col : 4;
		u64 xsdef : 4;
		u64 xscol : 4;
		u64 reserved_6_7 : 2;
		u64 undflw : 4;
		u64 ncb_nxa : 1;
		u64 pko_nxa : 1;
	} cn38xx;
	struct cvmx_gmxx_tx_int_en_cn38xxp2 {
		u64 reserved_16_63 : 48;
		u64 xsdef : 4;
		u64 xscol : 4;
		u64 reserved_6_7 : 2;
		u64 undflw : 4;
		u64 ncb_nxa : 1;
		u64 pko_nxa : 1;
	} cn38xxp2;
	struct cvmx_gmxx_tx_int_en_cn30xx cn50xx;
	struct cvmx_gmxx_tx_int_en_cn52xx {
		u64 reserved_20_63 : 44;
		u64 late_col : 4;
		u64 xsdef : 4;
		u64 xscol : 4;
		u64 reserved_6_7 : 2;
		u64 undflw : 4;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cn52xx;
	struct cvmx_gmxx_tx_int_en_cn52xx cn52xxp1;
	struct cvmx_gmxx_tx_int_en_cn52xx cn56xx;
	struct cvmx_gmxx_tx_int_en_cn52xx cn56xxp1;
	struct cvmx_gmxx_tx_int_en_cn38xx cn58xx;
	struct cvmx_gmxx_tx_int_en_cn38xx cn58xxp1;
	struct cvmx_gmxx_tx_int_en_s cn61xx;
	struct cvmx_gmxx_tx_int_en_cn63xx {
		u64 reserved_24_63 : 40;
		u64 ptp_lost : 4;
		u64 late_col : 4;
		u64 xsdef : 4;
		u64 xscol : 4;
		u64 reserved_6_7 : 2;
		u64 undflw : 4;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cn63xx;
	struct cvmx_gmxx_tx_int_en_cn63xx cn63xxp1;
	struct cvmx_gmxx_tx_int_en_s cn66xx;
	struct cvmx_gmxx_tx_int_en_cn68xx {
		u64 reserved_25_63 : 39;
		u64 xchange : 1;
		u64 ptp_lost : 4;
		u64 late_col : 4;
		u64 xsdef : 4;
		u64 xscol : 4;
		u64 reserved_6_7 : 2;
		u64 undflw : 4;
		u64 pko_nxp : 1;
		u64 pko_nxa : 1;
	} cn68xx;
	struct cvmx_gmxx_tx_int_en_cn68xx cn68xxp1;
	struct cvmx_gmxx_tx_int_en_s cn70xx;
	struct cvmx_gmxx_tx_int_en_s cn70xxp1;
	struct cvmx_gmxx_tx_int_en_cnf71xx {
		u64 reserved_25_63 : 39;
		u64 xchange : 1;
		u64 reserved_22_23 : 2;
		u64 ptp_lost : 2;
		u64 reserved_18_19 : 2;
		u64 late_col : 2;
		u64 reserved_14_15 : 2;
		u64 xsdef : 2;
		u64 reserved_10_11 : 2;
		u64 xscol : 2;
		u64 reserved_4_7 : 4;
		u64 undflw : 2;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cnf71xx;
};

typedef union cvmx_gmxx_tx_int_en cvmx_gmxx_tx_int_en_t;

/**
 * cvmx_gmx#_tx_int_reg
 *
 * GMX_TX_INT_REG = Interrupt Register
 *
 *
 * Notes:
 * In XAUI mode, only the lsb (corresponding to port0) of UNDFLW is used.
 *
 */
union cvmx_gmxx_tx_int_reg {
	u64 u64;
	struct cvmx_gmxx_tx_int_reg_s {
		u64 reserved_25_63 : 39;
		u64 xchange : 1;
		u64 ptp_lost : 4;
		u64 late_col : 4;
		u64 xsdef : 4;
		u64 xscol : 4;
		u64 reserved_6_7 : 2;
		u64 undflw : 4;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} s;
	struct cvmx_gmxx_tx_int_reg_cn30xx {
		u64 reserved_19_63 : 45;
		u64 late_col : 3;
		u64 reserved_15_15 : 1;
		u64 xsdef : 3;
		u64 reserved_11_11 : 1;
		u64 xscol : 3;
		u64 reserved_5_7 : 3;
		u64 undflw : 3;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cn30xx;
	struct cvmx_gmxx_tx_int_reg_cn31xx {
		u64 reserved_15_63 : 49;
		u64 xsdef : 3;
		u64 reserved_11_11 : 1;
		u64 xscol : 3;
		u64 reserved_5_7 : 3;
		u64 undflw : 3;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cn31xx;
	struct cvmx_gmxx_tx_int_reg_cn38xx {
		u64 reserved_20_63 : 44;
		u64 late_col : 4;
		u64 xsdef : 4;
		u64 xscol : 4;
		u64 reserved_6_7 : 2;
		u64 undflw : 4;
		u64 ncb_nxa : 1;
		u64 pko_nxa : 1;
	} cn38xx;
	struct cvmx_gmxx_tx_int_reg_cn38xxp2 {
		u64 reserved_16_63 : 48;
		u64 xsdef : 4;
		u64 xscol : 4;
		u64 reserved_6_7 : 2;
		u64 undflw : 4;
		u64 ncb_nxa : 1;
		u64 pko_nxa : 1;
	} cn38xxp2;
	struct cvmx_gmxx_tx_int_reg_cn30xx cn50xx;
	struct cvmx_gmxx_tx_int_reg_cn52xx {
		u64 reserved_20_63 : 44;
		u64 late_col : 4;
		u64 xsdef : 4;
		u64 xscol : 4;
		u64 reserved_6_7 : 2;
		u64 undflw : 4;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cn52xx;
	struct cvmx_gmxx_tx_int_reg_cn52xx cn52xxp1;
	struct cvmx_gmxx_tx_int_reg_cn52xx cn56xx;
	struct cvmx_gmxx_tx_int_reg_cn52xx cn56xxp1;
	struct cvmx_gmxx_tx_int_reg_cn38xx cn58xx;
	struct cvmx_gmxx_tx_int_reg_cn38xx cn58xxp1;
	struct cvmx_gmxx_tx_int_reg_s cn61xx;
	struct cvmx_gmxx_tx_int_reg_cn63xx {
		u64 reserved_24_63 : 40;
		u64 ptp_lost : 4;
		u64 late_col : 4;
		u64 xsdef : 4;
		u64 xscol : 4;
		u64 reserved_6_7 : 2;
		u64 undflw : 4;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cn63xx;
	struct cvmx_gmxx_tx_int_reg_cn63xx cn63xxp1;
	struct cvmx_gmxx_tx_int_reg_s cn66xx;
	struct cvmx_gmxx_tx_int_reg_cn68xx {
		u64 reserved_25_63 : 39;
		u64 xchange : 1;
		u64 ptp_lost : 4;
		u64 late_col : 4;
		u64 xsdef : 4;
		u64 xscol : 4;
		u64 reserved_6_7 : 2;
		u64 undflw : 4;
		u64 pko_nxp : 1;
		u64 pko_nxa : 1;
	} cn68xx;
	struct cvmx_gmxx_tx_int_reg_cn68xx cn68xxp1;
	struct cvmx_gmxx_tx_int_reg_s cn70xx;
	struct cvmx_gmxx_tx_int_reg_s cn70xxp1;
	struct cvmx_gmxx_tx_int_reg_cnf71xx {
		u64 reserved_25_63 : 39;
		u64 xchange : 1;
		u64 reserved_22_23 : 2;
		u64 ptp_lost : 2;
		u64 reserved_18_19 : 2;
		u64 late_col : 2;
		u64 reserved_14_15 : 2;
		u64 xsdef : 2;
		u64 reserved_10_11 : 2;
		u64 xscol : 2;
		u64 reserved_4_7 : 4;
		u64 undflw : 2;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cnf71xx;
};

typedef union cvmx_gmxx_tx_int_reg cvmx_gmxx_tx_int_reg_t;

/**
 * cvmx_gmx#_tx_jam
 *
 * GMX_TX_JAM = Packet TX Jam Pattern
 *
 */
union cvmx_gmxx_tx_jam {
	u64 u64;
	struct cvmx_gmxx_tx_jam_s {
		u64 reserved_8_63 : 56;
		u64 jam : 8;
	} s;
	struct cvmx_gmxx_tx_jam_s cn30xx;
	struct cvmx_gmxx_tx_jam_s cn31xx;
	struct cvmx_gmxx_tx_jam_s cn38xx;
	struct cvmx_gmxx_tx_jam_s cn38xxp2;
	struct cvmx_gmxx_tx_jam_s cn50xx;
	struct cvmx_gmxx_tx_jam_s cn52xx;
	struct cvmx_gmxx_tx_jam_s cn52xxp1;
	struct cvmx_gmxx_tx_jam_s cn56xx;
	struct cvmx_gmxx_tx_jam_s cn56xxp1;
	struct cvmx_gmxx_tx_jam_s cn58xx;
	struct cvmx_gmxx_tx_jam_s cn58xxp1;
	struct cvmx_gmxx_tx_jam_s cn61xx;
	struct cvmx_gmxx_tx_jam_s cn63xx;
	struct cvmx_gmxx_tx_jam_s cn63xxp1;
	struct cvmx_gmxx_tx_jam_s cn66xx;
	struct cvmx_gmxx_tx_jam_s cn68xx;
	struct cvmx_gmxx_tx_jam_s cn68xxp1;
	struct cvmx_gmxx_tx_jam_s cn70xx;
	struct cvmx_gmxx_tx_jam_s cn70xxp1;
	struct cvmx_gmxx_tx_jam_s cnf71xx;
};

typedef union cvmx_gmxx_tx_jam cvmx_gmxx_tx_jam_t;

/**
 * cvmx_gmx#_tx_lfsr
 *
 * GMX_TX_LFSR = LFSR used to implement truncated binary exponential backoff
 *
 */
union cvmx_gmxx_tx_lfsr {
	u64 u64;
	struct cvmx_gmxx_tx_lfsr_s {
		u64 reserved_16_63 : 48;
		u64 lfsr : 16;
	} s;
	struct cvmx_gmxx_tx_lfsr_s cn30xx;
	struct cvmx_gmxx_tx_lfsr_s cn31xx;
	struct cvmx_gmxx_tx_lfsr_s cn38xx;
	struct cvmx_gmxx_tx_lfsr_s cn38xxp2;
	struct cvmx_gmxx_tx_lfsr_s cn50xx;
	struct cvmx_gmxx_tx_lfsr_s cn52xx;
	struct cvmx_gmxx_tx_lfsr_s cn52xxp1;
	struct cvmx_gmxx_tx_lfsr_s cn56xx;
	struct cvmx_gmxx_tx_lfsr_s cn56xxp1;
	struct cvmx_gmxx_tx_lfsr_s cn58xx;
	struct cvmx_gmxx_tx_lfsr_s cn58xxp1;
	struct cvmx_gmxx_tx_lfsr_s cn61xx;
	struct cvmx_gmxx_tx_lfsr_s cn63xx;
	struct cvmx_gmxx_tx_lfsr_s cn63xxp1;
	struct cvmx_gmxx_tx_lfsr_s cn66xx;
	struct cvmx_gmxx_tx_lfsr_s cn68xx;
	struct cvmx_gmxx_tx_lfsr_s cn68xxp1;
	struct cvmx_gmxx_tx_lfsr_s cn70xx;
	struct cvmx_gmxx_tx_lfsr_s cn70xxp1;
	struct cvmx_gmxx_tx_lfsr_s cnf71xx;
};

typedef union cvmx_gmxx_tx_lfsr cvmx_gmxx_tx_lfsr_t;

/**
 * cvmx_gmx#_tx_ovr_bp
 *
 * GMX_TX_OVR_BP = Packet Interface TX Override BackPressure
 *
 *
 * Notes:
 * In XAUI mode, only the lsb (corresponding to port0) of EN, BP, and IGN_FULL are used.
 *
 * GMX*_TX_OVR_BP[EN<0>] must be set to one and GMX*_TX_OVR_BP[BP<0>] must be cleared to zero
 * (to forcibly disable HW-automatic 802.3 pause packet generation) with the HiGig2 Protocol
 * when GMX*_HG2_CONTROL[HG2TX_EN]=0. (The HiGig2 protocol is indicated by
 * GMX*_TX_XAUI_CTL[HG_EN]=1 and GMX*_RX0_UDD_SKP[LEN]=16.) HW can only auto-generate backpressure
 * through HiGig2 messages (optionally, when GMX*_HG2_CONTROL[HG2TX_EN]=1) with the HiGig2
 * protocol.
 */
union cvmx_gmxx_tx_ovr_bp {
	u64 u64;
	struct cvmx_gmxx_tx_ovr_bp_s {
		u64 reserved_48_63 : 16;
		u64 tx_prt_bp : 16;
		u64 reserved_12_31 : 20;
		u64 en : 4;
		u64 bp : 4;
		u64 ign_full : 4;
	} s;
	struct cvmx_gmxx_tx_ovr_bp_cn30xx {
		u64 reserved_11_63 : 53;
		u64 en : 3;
		u64 reserved_7_7 : 1;
		u64 bp : 3;
		u64 reserved_3_3 : 1;
		u64 ign_full : 3;
	} cn30xx;
	struct cvmx_gmxx_tx_ovr_bp_cn30xx cn31xx;
	struct cvmx_gmxx_tx_ovr_bp_cn38xx {
		u64 reserved_12_63 : 52;
		u64 en : 4;
		u64 bp : 4;
		u64 ign_full : 4;
	} cn38xx;
	struct cvmx_gmxx_tx_ovr_bp_cn38xx cn38xxp2;
	struct cvmx_gmxx_tx_ovr_bp_cn30xx cn50xx;
	struct cvmx_gmxx_tx_ovr_bp_s cn52xx;
	struct cvmx_gmxx_tx_ovr_bp_s cn52xxp1;
	struct cvmx_gmxx_tx_ovr_bp_s cn56xx;
	struct cvmx_gmxx_tx_ovr_bp_s cn56xxp1;
	struct cvmx_gmxx_tx_ovr_bp_cn38xx cn58xx;
	struct cvmx_gmxx_tx_ovr_bp_cn38xx cn58xxp1;
	struct cvmx_gmxx_tx_ovr_bp_s cn61xx;
	struct cvmx_gmxx_tx_ovr_bp_s cn63xx;
	struct cvmx_gmxx_tx_ovr_bp_s cn63xxp1;
	struct cvmx_gmxx_tx_ovr_bp_s cn66xx;
	struct cvmx_gmxx_tx_ovr_bp_s cn68xx;
	struct cvmx_gmxx_tx_ovr_bp_s cn68xxp1;
	struct cvmx_gmxx_tx_ovr_bp_s cn70xx;
	struct cvmx_gmxx_tx_ovr_bp_s cn70xxp1;
	struct cvmx_gmxx_tx_ovr_bp_cnf71xx {
		u64 reserved_48_63 : 16;
		u64 tx_prt_bp : 16;
		u64 reserved_10_31 : 22;
		u64 en : 2;
		u64 reserved_6_7 : 2;
		u64 bp : 2;
		u64 reserved_2_3 : 2;
		u64 ign_full : 2;
	} cnf71xx;
};

typedef union cvmx_gmxx_tx_ovr_bp cvmx_gmxx_tx_ovr_bp_t;

/**
 * cvmx_gmx#_tx_pause_pkt_dmac
 *
 * GMX_TX_PAUSE_PKT_DMAC = Packet TX Pause Packet DMAC field
 *
 */
union cvmx_gmxx_tx_pause_pkt_dmac {
	u64 u64;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s {
		u64 reserved_48_63 : 16;
		u64 dmac : 48;
	} s;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn30xx;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn31xx;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn38xx;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn38xxp2;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn50xx;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn52xx;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn52xxp1;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn56xx;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn56xxp1;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn58xx;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn58xxp1;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn61xx;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn63xx;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn63xxp1;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn66xx;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn68xx;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn68xxp1;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn70xx;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cn70xxp1;
	struct cvmx_gmxx_tx_pause_pkt_dmac_s cnf71xx;
};

typedef union cvmx_gmxx_tx_pause_pkt_dmac cvmx_gmxx_tx_pause_pkt_dmac_t;

/**
 * cvmx_gmx#_tx_pause_pkt_type
 *
 * GMX_TX_PAUSE_PKT_TYPE = Packet Interface TX Pause Packet TYPE field
 *
 */
union cvmx_gmxx_tx_pause_pkt_type {
	u64 u64;
	struct cvmx_gmxx_tx_pause_pkt_type_s {
		u64 reserved_16_63 : 48;
		u64 type : 16;
	} s;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn30xx;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn31xx;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn38xx;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn38xxp2;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn50xx;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn52xx;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn52xxp1;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn56xx;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn56xxp1;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn58xx;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn58xxp1;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn61xx;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn63xx;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn63xxp1;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn66xx;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn68xx;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn68xxp1;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn70xx;
	struct cvmx_gmxx_tx_pause_pkt_type_s cn70xxp1;
	struct cvmx_gmxx_tx_pause_pkt_type_s cnf71xx;
};

typedef union cvmx_gmxx_tx_pause_pkt_type cvmx_gmxx_tx_pause_pkt_type_t;

/**
 * cvmx_gmx#_tx_prts
 *
 * Common
 * GMX_TX_PRTS = TX Ports
 */
union cvmx_gmxx_tx_prts {
	u64 u64;
	struct cvmx_gmxx_tx_prts_s {
		u64 reserved_5_63 : 59;
		u64 prts : 5;
	} s;
	struct cvmx_gmxx_tx_prts_s cn30xx;
	struct cvmx_gmxx_tx_prts_s cn31xx;
	struct cvmx_gmxx_tx_prts_s cn38xx;
	struct cvmx_gmxx_tx_prts_s cn38xxp2;
	struct cvmx_gmxx_tx_prts_s cn50xx;
	struct cvmx_gmxx_tx_prts_s cn52xx;
	struct cvmx_gmxx_tx_prts_s cn52xxp1;
	struct cvmx_gmxx_tx_prts_s cn56xx;
	struct cvmx_gmxx_tx_prts_s cn56xxp1;
	struct cvmx_gmxx_tx_prts_s cn58xx;
	struct cvmx_gmxx_tx_prts_s cn58xxp1;
	struct cvmx_gmxx_tx_prts_s cn61xx;
	struct cvmx_gmxx_tx_prts_s cn63xx;
	struct cvmx_gmxx_tx_prts_s cn63xxp1;
	struct cvmx_gmxx_tx_prts_s cn66xx;
	struct cvmx_gmxx_tx_prts_s cn68xx;
	struct cvmx_gmxx_tx_prts_s cn68xxp1;
	struct cvmx_gmxx_tx_prts_s cn70xx;
	struct cvmx_gmxx_tx_prts_s cn70xxp1;
	struct cvmx_gmxx_tx_prts_s cnf71xx;
};

typedef union cvmx_gmxx_tx_prts cvmx_gmxx_tx_prts_t;

/**
 * cvmx_gmx#_tx_spi_ctl
 *
 * GMX_TX_SPI_CTL = Spi4 TX ModesSpi4
 *
 */
union cvmx_gmxx_tx_spi_ctl {
	u64 u64;
	struct cvmx_gmxx_tx_spi_ctl_s {
		u64 reserved_2_63 : 62;
		u64 tpa_clr : 1;
		u64 cont_pkt : 1;
	} s;
	struct cvmx_gmxx_tx_spi_ctl_s cn38xx;
	struct cvmx_gmxx_tx_spi_ctl_s cn38xxp2;
	struct cvmx_gmxx_tx_spi_ctl_s cn58xx;
	struct cvmx_gmxx_tx_spi_ctl_s cn58xxp1;
};

typedef union cvmx_gmxx_tx_spi_ctl cvmx_gmxx_tx_spi_ctl_t;

/**
 * cvmx_gmx#_tx_spi_drain
 *
 * GMX_TX_SPI_DRAIN = Drain out Spi TX FIFO
 *
 */
union cvmx_gmxx_tx_spi_drain {
	u64 u64;
	struct cvmx_gmxx_tx_spi_drain_s {
		u64 reserved_16_63 : 48;
		u64 drain : 16;
	} s;
	struct cvmx_gmxx_tx_spi_drain_s cn38xx;
	struct cvmx_gmxx_tx_spi_drain_s cn58xx;
	struct cvmx_gmxx_tx_spi_drain_s cn58xxp1;
};

typedef union cvmx_gmxx_tx_spi_drain cvmx_gmxx_tx_spi_drain_t;

/**
 * cvmx_gmx#_tx_spi_max
 *
 * GMX_TX_SPI_MAX = RGMII TX Spi4 MAX
 *
 */
union cvmx_gmxx_tx_spi_max {
	u64 u64;
	struct cvmx_gmxx_tx_spi_max_s {
		u64 reserved_23_63 : 41;
		u64 slice : 7;
		u64 max2 : 8;
		u64 max1 : 8;
	} s;
	struct cvmx_gmxx_tx_spi_max_cn38xx {
		u64 reserved_16_63 : 48;
		u64 max2 : 8;
		u64 max1 : 8;
	} cn38xx;
	struct cvmx_gmxx_tx_spi_max_cn38xx cn38xxp2;
	struct cvmx_gmxx_tx_spi_max_s cn58xx;
	struct cvmx_gmxx_tx_spi_max_s cn58xxp1;
};

typedef union cvmx_gmxx_tx_spi_max cvmx_gmxx_tx_spi_max_t;

/**
 * cvmx_gmx#_tx_spi_round#
 *
 * GMX_TX_SPI_ROUND = Controls SPI4 TX Arbitration
 *
 */
union cvmx_gmxx_tx_spi_roundx {
	u64 u64;
	struct cvmx_gmxx_tx_spi_roundx_s {
		u64 reserved_16_63 : 48;
		u64 round : 16;
	} s;
	struct cvmx_gmxx_tx_spi_roundx_s cn58xx;
	struct cvmx_gmxx_tx_spi_roundx_s cn58xxp1;
};

typedef union cvmx_gmxx_tx_spi_roundx cvmx_gmxx_tx_spi_roundx_t;

/**
 * cvmx_gmx#_tx_spi_thresh
 *
 * GMX_TX_SPI_THRESH = RGMII TX Spi4 Transmit Threshold
 *
 *
 * Notes:
 * Note: zero will map to 0x20
 *
 * This will normally creates Spi4 traffic bursts at least THRESH in length.
 * If dclk > eclk, then this rule may not always hold and Octeon may split
 * transfers into smaller bursts - some of which could be as short as 16B.
 * Octeon will never violate the Spi4.2 spec and send a non-EOP burst that is
 * not a multiple of 16B.
 */
union cvmx_gmxx_tx_spi_thresh {
	u64 u64;
	struct cvmx_gmxx_tx_spi_thresh_s {
		u64 reserved_6_63 : 58;
		u64 thresh : 6;
	} s;
	struct cvmx_gmxx_tx_spi_thresh_s cn38xx;
	struct cvmx_gmxx_tx_spi_thresh_s cn38xxp2;
	struct cvmx_gmxx_tx_spi_thresh_s cn58xx;
	struct cvmx_gmxx_tx_spi_thresh_s cn58xxp1;
};

typedef union cvmx_gmxx_tx_spi_thresh cvmx_gmxx_tx_spi_thresh_t;

/**
 * cvmx_gmx#_tx_xaui_ctl
 */
union cvmx_gmxx_tx_xaui_ctl {
	u64 u64;
	struct cvmx_gmxx_tx_xaui_ctl_s {
		u64 reserved_11_63 : 53;
		u64 hg_pause_hgi : 2;
		u64 hg_en : 1;
		u64 reserved_7_7 : 1;
		u64 ls_byp : 1;
		u64 ls : 2;
		u64 reserved_2_3 : 2;
		u64 uni_en : 1;
		u64 dic_en : 1;
	} s;
	struct cvmx_gmxx_tx_xaui_ctl_s cn52xx;
	struct cvmx_gmxx_tx_xaui_ctl_s cn52xxp1;
	struct cvmx_gmxx_tx_xaui_ctl_s cn56xx;
	struct cvmx_gmxx_tx_xaui_ctl_s cn56xxp1;
	struct cvmx_gmxx_tx_xaui_ctl_s cn61xx;
	struct cvmx_gmxx_tx_xaui_ctl_s cn63xx;
	struct cvmx_gmxx_tx_xaui_ctl_s cn63xxp1;
	struct cvmx_gmxx_tx_xaui_ctl_s cn66xx;
	struct cvmx_gmxx_tx_xaui_ctl_s cn68xx;
	struct cvmx_gmxx_tx_xaui_ctl_s cn68xxp1;
	struct cvmx_gmxx_tx_xaui_ctl_s cn70xx;
	struct cvmx_gmxx_tx_xaui_ctl_s cn70xxp1;
	struct cvmx_gmxx_tx_xaui_ctl_s cnf71xx;
};

typedef union cvmx_gmxx_tx_xaui_ctl cvmx_gmxx_tx_xaui_ctl_t;

/**
 * cvmx_gmx#_wol_ctl
 */
union cvmx_gmxx_wol_ctl {
	u64 u64;
	struct cvmx_gmxx_wol_ctl_s {
		u64 reserved_36_63 : 28;
		u64 magic_en : 4;
		u64 reserved_20_31 : 12;
		u64 direct_en : 4;
		u64 reserved_1_15 : 15;
		u64 en : 1;
	} s;
	struct cvmx_gmxx_wol_ctl_s cn70xx;
	struct cvmx_gmxx_wol_ctl_s cn70xxp1;
};

typedef union cvmx_gmxx_wol_ctl cvmx_gmxx_wol_ctl_t;

/**
 * cvmx_gmx#_xaui_ext_loopback
 */
union cvmx_gmxx_xaui_ext_loopback {
	u64 u64;
	struct cvmx_gmxx_xaui_ext_loopback_s {
		u64 reserved_5_63 : 59;
		u64 en : 1;
		u64 thresh : 4;
	} s;
	struct cvmx_gmxx_xaui_ext_loopback_s cn52xx;
	struct cvmx_gmxx_xaui_ext_loopback_s cn52xxp1;
	struct cvmx_gmxx_xaui_ext_loopback_s cn56xx;
	struct cvmx_gmxx_xaui_ext_loopback_s cn56xxp1;
	struct cvmx_gmxx_xaui_ext_loopback_s cn61xx;
	struct cvmx_gmxx_xaui_ext_loopback_s cn63xx;
	struct cvmx_gmxx_xaui_ext_loopback_s cn63xxp1;
	struct cvmx_gmxx_xaui_ext_loopback_s cn66xx;
	struct cvmx_gmxx_xaui_ext_loopback_s cn68xx;
	struct cvmx_gmxx_xaui_ext_loopback_s cn68xxp1;
	struct cvmx_gmxx_xaui_ext_loopback_s cn70xx;
	struct cvmx_gmxx_xaui_ext_loopback_s cn70xxp1;
	struct cvmx_gmxx_xaui_ext_loopback_s cnf71xx;
};

typedef union cvmx_gmxx_xaui_ext_loopback cvmx_gmxx_xaui_ext_loopback_t;

#endif
