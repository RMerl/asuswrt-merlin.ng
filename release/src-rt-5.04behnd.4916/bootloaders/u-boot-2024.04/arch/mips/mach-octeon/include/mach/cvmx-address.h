/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Typedefs and defines for working with Octeon physical addresses.
 */

#ifndef __CVMX_ADDRESS_H__
#define __CVMX_ADDRESS_H__

typedef enum {
	CVMX_MIPS_SPACE_XKSEG = 3LL,
	CVMX_MIPS_SPACE_XKPHYS = 2LL,
	CVMX_MIPS_SPACE_XSSEG = 1LL,
	CVMX_MIPS_SPACE_XUSEG = 0LL
} cvmx_mips_space_t;

typedef enum {
	CVMX_MIPS_XKSEG_SPACE_KSEG0 = 0LL,
	CVMX_MIPS_XKSEG_SPACE_KSEG1 = 1LL,
	CVMX_MIPS_XKSEG_SPACE_SSEG = 2LL,
	CVMX_MIPS_XKSEG_SPACE_KSEG3 = 3LL
} cvmx_mips_xkseg_space_t;

/* decodes <14:13> of a kseg3 window address */
typedef enum {
	CVMX_ADD_WIN_SCR = 0L,
	CVMX_ADD_WIN_DMA = 1L,
	CVMX_ADD_WIN_UNUSED = 2L,
	CVMX_ADD_WIN_UNUSED2 = 3L
} cvmx_add_win_dec_t;

/* decode within DMA space */
typedef enum {
	CVMX_ADD_WIN_DMA_ADD = 0L,
	CVMX_ADD_WIN_DMA_SENDMEM = 1L,
	/* store data must be normal DRAM memory space address in this case */
	CVMX_ADD_WIN_DMA_SENDDMA = 2L,
	/* see CVMX_ADD_WIN_DMA_SEND_DEC for data contents */
	CVMX_ADD_WIN_DMA_SENDIO = 3L,
	/* store data must be normal IO space address in this case */
	CVMX_ADD_WIN_DMA_SENDSINGLE = 4L,
	/* no write buffer data needed/used */
} cvmx_add_win_dma_dec_t;

/**
 *   Physical Address Decode
 *
 * Octeon-I HW never interprets this X (<39:36> reserved
 * for future expansion), software should set to 0.
 *
 *  - 0x0 XXX0 0000 0000 to      DRAM         Cached
 *  - 0x0 XXX0 0FFF FFFF
 *
 *  - 0x0 XXX0 1000 0000 to      Boot Bus     Uncached  (Converted to 0x1 00X0 1000 0000
 *  - 0x0 XXX0 1FFF FFFF         + EJTAG                           to 0x1 00X0 1FFF FFFF)
 *
 *  - 0x0 XXX0 2000 0000 to      DRAM         Cached
 *  - 0x0 XXXF FFFF FFFF
 *
 *  - 0x1 00X0 0000 0000 to      Boot Bus     Uncached
 *  - 0x1 00XF FFFF FFFF
 *
 *  - 0x1 01X0 0000 0000 to      Other NCB    Uncached
 *  - 0x1 FFXF FFFF FFFF         devices
 *
 * Decode of all Octeon addresses
 */
typedef union {
	u64 u64;
	struct {
		cvmx_mips_space_t R : 2;
		u64 offset : 62;
	} sva;

	struct {
		u64 zeroes : 33;
		u64 offset : 31;
	} suseg;

	struct {
		u64 ones : 33;
		cvmx_mips_xkseg_space_t sp : 2;
		u64 offset : 29;
	} sxkseg;

	struct {
		cvmx_mips_space_t R : 2;
		u64 cca : 3;
		u64 mbz : 10;
		u64 pa : 49;
	} sxkphys;

	struct {
		u64 mbz : 15;
		u64 is_io : 1;
		u64 did : 8;
		u64 unaddr : 4;
		u64 offset : 36;
	} sphys;

	struct {
		u64 zeroes : 24;
		u64 unaddr : 4;
		u64 offset : 36;
	} smem;

	struct {
		u64 mem_region : 2;
		u64 mbz : 13;
		u64 is_io : 1;
		u64 did : 8;
		u64 unaddr : 4;
		u64 offset : 36;
	} sio;

	struct {
		u64 ones : 49;
		cvmx_add_win_dec_t csrdec : 2;
		u64 addr : 13;
	} sscr;

	/* there should only be stores to IOBDMA space, no loads */
	struct {
		u64 ones : 49;
		cvmx_add_win_dec_t csrdec : 2;
		u64 unused2 : 3;
		cvmx_add_win_dma_dec_t type : 3;
		u64 addr : 7;
	} sdma;

	struct {
		u64 didspace : 24;
		u64 unused : 40;
	} sfilldidspace;
} cvmx_addr_t;

/* These macros for used by 32 bit applications */

#define CVMX_MIPS32_SPACE_KSEG0	     1l
#define CVMX_ADD_SEG32(segment, add) (((s32)segment << 31) | (s32)(add))

/*
 * Currently all IOs are performed using XKPHYS addressing. Linux uses the
 * CvmMemCtl register to enable XKPHYS addressing to IO space from user mode.
 * Future OSes may need to change the upper bits of IO addresses. The
 * following define controls the upper two bits for all IO addresses generated
 * by the simple executive library
 */
#define CVMX_IO_SEG CVMX_MIPS_SPACE_XKPHYS

/* These macros simplify the process of creating common IO addresses */
#define CVMX_ADD_SEG(segment, add) ((((u64)segment) << 62) | (add))

#define CVMX_ADD_IO_SEG(add) (add)

#define CVMX_ADDR_DIDSPACE(did)	   (((CVMX_IO_SEG) << 22) | ((1ULL) << 8) | (did))
#define CVMX_ADDR_DID(did)	   (CVMX_ADDR_DIDSPACE(did) << 40)
#define CVMX_FULL_DID(did, subdid) (((did) << 3) | (subdid))

/* from include/ncb_rsl_id.v */
#define CVMX_OCT_DID_MIS  0ULL /* misc stuff */
#define CVMX_OCT_DID_GMX0 1ULL
#define CVMX_OCT_DID_GMX1 2ULL
#define CVMX_OCT_DID_PCI  3ULL
#define CVMX_OCT_DID_KEY  4ULL
#define CVMX_OCT_DID_FPA  5ULL
#define CVMX_OCT_DID_DFA  6ULL
#define CVMX_OCT_DID_ZIP  7ULL
#define CVMX_OCT_DID_RNG  8ULL
#define CVMX_OCT_DID_IPD  9ULL
#define CVMX_OCT_DID_PKT  10ULL
#define CVMX_OCT_DID_TIM  11ULL
#define CVMX_OCT_DID_TAG  12ULL
/* the rest are not on the IO bus */
#define CVMX_OCT_DID_L2C  16ULL
#define CVMX_OCT_DID_LMC  17ULL
#define CVMX_OCT_DID_SPX0 18ULL
#define CVMX_OCT_DID_SPX1 19ULL
#define CVMX_OCT_DID_PIP  20ULL
#define CVMX_OCT_DID_ASX0 22ULL
#define CVMX_OCT_DID_ASX1 23ULL
#define CVMX_OCT_DID_IOB  30ULL

#define CVMX_OCT_DID_PKT_SEND	 CVMX_FULL_DID(CVMX_OCT_DID_PKT, 2ULL)
#define CVMX_OCT_DID_TAG_SWTAG	 CVMX_FULL_DID(CVMX_OCT_DID_TAG, 0ULL)
#define CVMX_OCT_DID_TAG_TAG1	 CVMX_FULL_DID(CVMX_OCT_DID_TAG, 1ULL)
#define CVMX_OCT_DID_TAG_TAG2	 CVMX_FULL_DID(CVMX_OCT_DID_TAG, 2ULL)
#define CVMX_OCT_DID_TAG_TAG3	 CVMX_FULL_DID(CVMX_OCT_DID_TAG, 3ULL)
#define CVMX_OCT_DID_TAG_NULL_RD CVMX_FULL_DID(CVMX_OCT_DID_TAG, 4ULL)
#define CVMX_OCT_DID_TAG_TAG5	 CVMX_FULL_DID(CVMX_OCT_DID_TAG, 5ULL)
#define CVMX_OCT_DID_TAG_CSR	 CVMX_FULL_DID(CVMX_OCT_DID_TAG, 7ULL)
#define CVMX_OCT_DID_FAU_FAI	 CVMX_FULL_DID(CVMX_OCT_DID_IOB, 0ULL)
#define CVMX_OCT_DID_TIM_CSR	 CVMX_FULL_DID(CVMX_OCT_DID_TIM, 0ULL)
#define CVMX_OCT_DID_KEY_RW	 CVMX_FULL_DID(CVMX_OCT_DID_KEY, 0ULL)
#define CVMX_OCT_DID_PCI_6	 CVMX_FULL_DID(CVMX_OCT_DID_PCI, 6ULL)
#define CVMX_OCT_DID_MIS_BOO	 CVMX_FULL_DID(CVMX_OCT_DID_MIS, 0ULL)
#define CVMX_OCT_DID_PCI_RML	 CVMX_FULL_DID(CVMX_OCT_DID_PCI, 0ULL)
#define CVMX_OCT_DID_IPD_CSR	 CVMX_FULL_DID(CVMX_OCT_DID_IPD, 7ULL)
#define CVMX_OCT_DID_DFA_CSR	 CVMX_FULL_DID(CVMX_OCT_DID_DFA, 7ULL)
#define CVMX_OCT_DID_MIS_CSR	 CVMX_FULL_DID(CVMX_OCT_DID_MIS, 7ULL)
#define CVMX_OCT_DID_ZIP_CSR	 CVMX_FULL_DID(CVMX_OCT_DID_ZIP, 0ULL)

/* Cast to unsigned long long, mainly for use in printfs. */
#define CAST_ULL(v) ((unsigned long long)(v))

#define UNMAPPED_PTR(x) ((1ULL << 63) | (x))

#endif /* __CVMX_ADDRESS_H__ */
