/*
<:copyright-BRCM:2015:DUAL/GPL:standard 

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/*

 * BMIPS 4355 Burstbank API
 * $Copyright Open Broadcom Corporation$
 * $Id$
 *
 *
 */

/*
 * -----------------------------------------------------------------------------
 *
 * File Name   : burstbank.c
 *
 * Provides implementation for
 * - initialization (enabling) BMIPS4355 DMA-E (burst bank) feature
 * - non-inlined callable versions of Burst Bank APIs
 * - debug APIs to dump a burst bank channel or the entire system.
 * - performance benchmarking using pmontool
 * - unit testing
 *
 * -----------------------------------------------------------------------------
 */
#define BB_IMPLEMENTATION   /* Non-inlined implementation of Burst Bank APIs  */

#include <linux/module.h>
#include <burstbank.h>

#if defined(BMIPS4355_DMAE)
bb_global_t bb_g;

#if defined(CC_BB_DEBUG)
/*
 * =============================================================================
 * Section: Implementation of Debug APIs
 * =============================================================================
 */

/*
 * -----------------------------------------------------------------------------
 * bb_dump: Dump a Burst Bank, with data and statistics based on verbosity
 * -----------------------------------------------------------------------------
 */
void
bb_dump(volatile bb_t * const bb_p, int verbose)
{
	uint32_t channel = BB_CHANNEL(bb_p);
	bb_state_t * const state_p = &bb_g.ch_state[channel];
	uint32_t status_dmae = bb_DMAE(channel, BB_STATUS_USED);

	BB_PRINT("\nBurstBank %s%p%s channel %s%u%s in_use %u GblDMA-E %u\n"
	         "\tmemory_p %p burst_words %u RD %u WR %u %sError %u%s\n",
	         _BBg_, bb_p, _BBn_, _BBg_, channel, _BBn_,
	         state_p->in_use.counter, status_dmae,
	         bb_p->memory_p, bb_p->burst_words,
	         BB_RD_TRANS(bb_p) ? 1 : 0, BB_WR_TRANS(bb_p) ? 1 : 0,
	         BB_ERR_COND(bb_p) ? _BBr_ : _BBn_,
	         BB_ERR_COND(bb_p) ? 1 : 0, _BBn_);

	if (verbose == 1) {
#		undef  _BBD
#		define _BBD(i)	BB_DATA(bb_p, i)
		BB_PRINT("\t\tdata00..03  0x%08x  0x%08x  0x%08x  0x%08x\n"
		         "\t\tdata04..07  0x%08x  0x%08x  0x%08x  0x%08x\n"
		         "\t\tdata08..11  0x%08x  0x%08x  0x%08x  0x%08x\n"
		         "\t\tdata12..15  0x%08x  0x%08x  0x%08x  0x%08x\n",
		         _BBD(0),  _BBD(1),  _BBD(2),  _BBD(3),
		         _BBD(4),  _BBD(5),  _BBD(6),  _BBD(7),
		         _BBD(8),  _BBD(9),  _BBD(10), _BBD(11),
		         _BBD(12), _BBD(13), _BBD(14), _BBD(15));

#if defined(CC_BB_STATS)
		BB_PRINT("\tStats:  reserves %u releases %u copies %u\n"
		         "\tReads:  %u polls %u %serrors %u%s\n"
		         "\tWrites: %u polls %u %serrors %u%s\n",
		         state_p->reserves, state_p->releases, state_p->copies,
		         state_p->RD.ops, state_p->RD.polls,
		         (state_p->RD.errors != 0) ? _BBr_ : _BBn_,
		         state_p->RD.errors, _BBn_,
		         state_p->WR.ops, state_p->WR.polls,
		         (state_p->WR.errors != 0) ? _BBr_ : _BBn_,
		         state_p->WR.errors, _BBn_);
#endif	/*  CC_BB_STATS */
	}
}

/*
 * -----------------------------------------------------------------------------
 * Show the entire BursBank system.
 * -----------------------------------------------------------------------------
 */
void
bb_show(int verbose)
{
	int channel_ix;

	for (channel_ix = 0U; channel_ix < BB_MAX_CHANNELS; channel_ix++) {
		volatile bb_t * const bb_p = BB_POINTER(channel_ix);
		bb_dump(bb_p, verbose);
	}
}

EXPORT_SYMBOL(bb_dump);
EXPORT_SYMBOL(bb_show);
#endif /*   CC_BB_DEBUG */

#if defined(CC_BB_BENCH) && defined(CONFIG_PMON)

/*
 * =============================================================================
 * Section: Benchmarking Burst bank using pmontool
 * =============================================================================
 */
#include <asm/pmonapi.h>
#include <linux/nbuff.h>

uint32_t memory[16];

typedef struct bb_dataA
{
	uint32_t u32[BB_SIZE_WORDS];
} bb_dataA_t ____cacheline_aligned;

bb_dataA_t data1;   /* Burst bank access */
bb_dataA_t data2;   /* Uncached access   */
bb_dataA_t data3;   /* Cached access     */
bb_dataA_t data4;

#define BB_XFER_SZ              (256 * 1024)
#define BB_XFER_U32             (BB_XFER_SZ / sizeof(uint32_t))
uint32_t src_mem[BB_XFER_U32];
uint32_t dst_mem[BB_XFER_U32];

#define DTCM_ADDR               0xb1080000
void
bb_bench(void)
{
	uint32_t ix, loops;
	bb_t * bb_p;
	uint32_t ret;

	uint32_t * const from_phy_p = BB_LOG2PHY(memory);
	uint32_t * const to_phy_p   = BB_LOG2PHY(&data1.u32[0]);
	uint32_t * const local_p    = BB_LOG2PHY(&data4.u32[0]);
	uint32_t * const kseg0_p    = (uint32_t*)CKSEG0ADDR(memory);
	uint32_t * const kseg1_p    = (uint32_t*)CKSEG1ADDR(memory);

	pmon_reg(1, "Burst Bank ASYNC  READ 16 words");
	pmon_reg(2, "Burst Bank  SYNC  READ 16 words");

	pmon_reg(3, "Burst Bank ASYNC WRITE 16 words");
	pmon_reg(4, "Burst Bank  SYNC WRITE 16 words");

	pmon_reg(5, "Burst Bank  SYNC  COPY 16 words");
	pmon_reg(6, "Burst Bank  SYNC  XFER 32K words");
	pmon_reg(7, "Burst Bank  SYNC  XFER 256K bytes");
	pmon_reg(8, "Burst Bank  WRITE TCM  256K bytes");
	pmon_reg(9, "Burst Bank  READ  TCM  256K bytes");
	pmon_reg(10, "Libc memcpy WRITE TCM  256K bytes");
	pmon_reg(11, "Libc memcpy READ  TCM  256K bytes");

	pmon_reg(12, "UNCACHED   SYNC  READ 16 words");
	pmon_reg(13, "UNCACHED   SYNC WRITE 16 words");

	pmon_reg(14, "INVALIDATE CACHE Mem  16 words");

	pmon_reg(15, "  CACHED   SYNC  READ 16 words");
	pmon_reg(16, "  CACHED   SYNC WRITE 16 words");

	pmon_reg(17, "FLUSH/INV  CACHE Mem  16 words");

	for (ix = 0; ix < 16; ix++) {
		memory[ix] = ix;
		data1.u32[ix] = data2.u32[ix] = data3.u32[ix] = data4.u32[ix] = 0;
	}

	for (ix = 0; ix < BB_XFER_U32; ix++) {
		src_mem[ix] = 0x12345678;
		dst_mem[ix] = 0x00000000;
	}

	/* Flush memory */
	cache_flush_len(memory, sizeof(uint32_t)*16);
	cache_flush_len(&data1, sizeof(uint32_t)*16);
	cache_flush_len(&data2, sizeof(uint32_t)*16);
	cache_flush_len(&data3, sizeof(uint32_t)*16);
	cache_flush_len(&data4, sizeof(uint32_t)*16);

	cache_flush_len(&src_mem, BB_XFER_SZ);
	cache_flush_len(&dst_mem, BB_XFER_SZ);

	cache_flush_len(&data4, sizeof(uint32_t)*16);

	bb_p = bb_reserve(bb_channel_copy, bb_alloc_ignore_status);

	ret = 0;

	for (loops = 0; likely(loops < 100); loops++) {
		if (unlikely(loops >= 10))
			pmon_bgn();

		/* Setup and Issue READ: MEMORY ---> BB */
		bb_issue(bb_p, from_phy_p, BB_SIZE_WORDS - 1, bb_read_pending);
		pmon_log(1);    /* ISSUE READ */

		/* Sync until READ completes: MEMORY ---> BB */
		ret |= bb_wait(bb_p, bb_read_pending);
		pmon_log(2);    /* WAIT FOR READ TO COMPLETE */

		/* Setup and Issue WRITE: BB ---> MEMORY */
		bb_issue(bb_p, to_phy_p, BB_SIZE_WORDS - 1, bb_write_pending);
		pmon_log(3);    /* ISSUE WRITE */

		/* Sync until WRITE completes: BB ---> MEMORY */
		ret |= bb_wait(bb_p, bb_write_pending);
		pmon_log(4);    /* WAIT FOR WRITE TO COMPLETE */

		/* Copy Memory ---> BB ---> Memory */
		ret |= bb_copy(from_phy_p, local_p, BB_SIZE_WORDS-1, bb_channel_copy);
		pmon_log(5);    /* Copy 64B */

		/* Copy Memory ---> BB ---> Memory */
		ret |= bb_xfer(BB_LOG2PHY(src_mem), BB_LOG2PHY(dst_mem), 32*1024);
		pmon_log(6);    /* XFER 128K Bytes */

		/* Copy Memory ---> BB ---> Memory */
		ret |= bb_xfer(BB_LOG2PHY(src_mem), BB_LOG2PHY(dst_mem), BB_XFER_U32);
		pmon_log(7);    /* XFER 256 KBytes */


		/* Copy DDR to TCM 256KBytes */
		ret |= bb_xfer(BB_LOG2PHY(src_mem), BB_LOG2PHY(DTCM_ADDR), BB_XFER_U32);
		pmon_log(8);

		/* Copy TCM to DDR 256KBytes */
		ret |= bb_xfer(BB_LOG2PHY(DTCM_ADDR), BB_LOG2PHY(dst_mem), BB_XFER_U32);
		pmon_log(9);

		memcpy((void*)DTCM_ADDR, (void*)src_mem, BB_XFER_SZ);
		pmon_log(10);

		memcpy((void*)src_mem, (void*)DTCM_ADDR, BB_XFER_SZ);
		pmon_log(11);

		/* READ from Uncached memory */
		data2.u32[0]  = *(kseg1_p + 0);  data2.u32[1]  = *(kseg1_p + 1);
		data2.u32[2]  = *(kseg1_p + 2);  data2.u32[3]  = *(kseg1_p + 3);
		data2.u32[4]  = *(kseg1_p + 4);  data2.u32[5]  = *(kseg1_p + 5);
		data2.u32[6]  = *(kseg1_p + 6);  data2.u32[7]  = *(kseg1_p + 7);
		data2.u32[8]  = *(kseg1_p + 8);  data2.u32[9]  = *(kseg1_p + 9);
		data2.u32[10] = *(kseg1_p + 10); data2.u32[11] = *(kseg1_p + 11);
		data2.u32[12] = *(kseg1_p + 12); data2.u32[13] = *(kseg1_p + 13);
		data2.u32[14] = *(kseg1_p + 14); data2.u32[15] = *(kseg1_p + 15);
		pmon_log(12);    /* UNCACHED READ ACCESS */

		/* WRITE to Uncached memory */
		*(kseg1_p + 0)  = data2.u32[0];  *(kseg1_p + 1)  = data2.u32[1];
		*(kseg1_p + 2)  = data2.u32[2];  *(kseg1_p + 3)  = data2.u32[3];
		*(kseg1_p + 4)  = data2.u32[4];  *(kseg1_p + 5)  = data2.u32[5];
		*(kseg1_p + 6)  = data2.u32[6];  *(kseg1_p + 7)  = data2.u32[7];
		*(kseg1_p + 8)  = data2.u32[8];  *(kseg1_p + 9)  = data2.u32[9];
		*(kseg1_p + 10) = data2.u32[10]; *(kseg1_p + 11) = data2.u32[11];
		*(kseg1_p + 12) = data2.u32[12]; *(kseg1_p + 13) = data2.u32[13];
		*(kseg1_p + 14) = data2.u32[14]; *(kseg1_p + 15) = data2.u32[15];
		pmon_log(13);    /* UNCACHED WRITE ACCESS */

		/* D-Cache invalidate (not dirty in cache, hence flush ignored */
		cache_flush_len(memory, sizeof(uint32_t) * 16);
		pmon_log(14);    /* cache invalidate */

		/* READ from Cached memory, 16B Cacheline */
		data3.u32[0]  = *(kseg0_p + 0);  data3.u32[1]  = *(kseg0_p + 1);
		data3.u32[2]  = *(kseg0_p + 2);  data3.u32[3]  = *(kseg0_p + 3);
		data3.u32[4]  = *(kseg0_p + 4);  data3.u32[5]  = *(kseg0_p + 5);
		data3.u32[6]  = *(kseg0_p + 6);  data3.u32[7]  = *(kseg0_p + 7);
		data3.u32[8]  = *(kseg0_p + 8);  data3.u32[9]  = *(kseg0_p + 9);
		data3.u32[10] = *(kseg0_p + 10); data3.u32[11] = *(kseg0_p + 11);
		data3.u32[12] = *(kseg0_p + 12); data3.u32[13] = *(kseg0_p + 13);
		data3.u32[14] = *(kseg0_p + 14); data3.u32[15] = *(kseg0_p + 15);
		pmon_log(15);   /* CACHED READ ACCESS */

		/* WRITE from Cached memory, 16B Cacheline, Buffered write */
		*(kseg0_p + 0)  = data3.u32[0];  *(kseg0_p + 1)  = data3.u32[1];
		*(kseg0_p + 2)  = data3.u32[2];  *(kseg0_p + 3)  = data3.u32[3];
		*(kseg0_p + 4)  = data3.u32[4];  *(kseg0_p + 5)  = data3.u32[5];
		*(kseg0_p + 6)  = data3.u32[6];  *(kseg0_p + 7)  = data3.u32[7];
		*(kseg0_p + 8)  = data3.u32[8];  *(kseg0_p + 9)  = data3.u32[9];
		*(kseg0_p + 10) = data3.u32[10]; *(kseg0_p + 11) = data3.u32[11];
		*(kseg0_p + 12) = data3.u32[12]; *(kseg0_p + 13) = data3.u32[13];
		*(kseg0_p + 14) = data3.u32[14]; *(kseg0_p + 15) = data3.u32[15];
		pmon_log(16);    /* CACHED WRITE ACCESS */

		/* D-Cache fluash (dirty) and invalidate */
		cache_flush_len(memory, sizeof(uint32_t) * 16);
		pmon_log(17);    /* cache flush */

		if (ret == 0) {
			pmon_end(17);
		} else {
			BB_PRINT(_BBr_ "ret error %u" _BBnl_, ret);
			pmon_clr();
			ret = 0;
		}
	}

	BB_DBG(
		for (ix = 0; ix < 16; ix++)
			BB_PRINT(_BBg_ " %u %u %u" _BBnl_, memory[ix],
			        data1.u32[ix], data2.u32[ix]);
		bb_show(1));

	bb_release(bb_channel_copy);
}
EXPORT_SYMBOL(bb_bench);

/*
	Evt:  Cycles-Count Nano-secs : EventName
	  1:            20        50 : Burst Bank ASYNC  READ 16 words
	  2:           137       342 : Burst Bank  SYNC  READ 16 words

	  3:            68       170 : Burst Bank ASYNC WRITE 16 words
	  4:            74       185 : Burst Bank  SYNC WRITE 16 words

	  5:           668      1670 : Burst Bank  SYNC  COPY 16 words
	  6:       1326676   3316690 : Burst Bank  SYNC  XFER 256K words

	  7:          1678      4195 : UNCACHED   SYNC  READ 16 words
	  8:           561      1402 : UNCACHED   SYNC WRITE 16 words

	  9:            84       210 : INVALIDATE CACHE Mem  16 words

	 10:           190       475 :   CACHED   SYNC  READ 16 words
	 11:            92       230 :   CACHED   SYNC WRITE 16 words

	 12:           237       592 : FLUSH/INV  CACHE Mem  16 words
 */

#endif  /*  CC_BB_BENCH  &&  CONFIG_PMON  */

#if defined(CC_BB_UNITT)
void bb_unittest(void)
{
	/* 1.  Verify burst bank properly enabled */
	/* 2.  Verify burst bank reservation scheme */
	/* 3.  Verify status clear, by causing an error condition first */
	/* 4.  Verify READ transaction with sync */
	/* 5.  Verify WRITE transaction with sync */
	/* 6.  Verify COPY transaction */
	/* 7.  Verify XFER transaction */

	/* 8.  Regress SHOW and DUMP */
	/* 9.  Regress STATS, DEBUG, AUDIT builds */
}
#endif  /*  CC_BB_UNITT */

/*
 * -----------------------------------------------------------------------------
 * Enable the BMIPS 4355 Burst Bank System.
 * -----------------------------------------------------------------------------
 */
void
bb_enable(void)
{
	uint32_t channel;
	volatile uint32_t * const dma_control_en
		= (uint32_t *)BMIPS_DMA_CONTROL_EN_REG;

	memset(&bb_g, 0, sizeof(bb_global_t));

	BB_DBG(bb_g.debug = (uint32_t)CC_BB_DEBUG);

	/* Enable BMIPS 4350 DMA-Engine */
	*dma_control_en |= 1U;      /* Enable */

	__asm__ volatile ("sync");  /* sync first */

	/*       (   *(0xFF400034)   == ( 0xFF500000       | 0x1) ) */
	BB_ASSERTV((*dma_control_en == (BMIPS_DMA_BB_BASE | 0x1)));

	for (channel = 0; channel < BB_MAX_CHANNELS; channel++) {
		bb_t * bb_p;
		uint32_t word_ix;

		bb_reset(channel);              /* clear error condition if any */
		bb_p = BB_POINTER(channel);
		for (word_ix = 0U; word_ix < BB_SIZE_WORDS; word_ix++)
			BB_DATA(bb_p, word_ix) = 0U; /* clear burst bank memories */
	}

	/* BB_DBG(bb_show(1));                *//* dump all burst banks */
}

static int
__init __init_burstbank(void)
{
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848)
	/* Although BCM96368's CPU is BMIPS4350, it does not have burst banks */
	struct cpuinfo_mips *c = & current_cpu_data;    /* cpu-info.h */

	if (likely(c->cputype == CPU_BMIPS4350))
		bb_enable();

	BB_PRINT(_BBh_ "BMIPS 4350 BurstBank %s" _BBn_
		     ": bb_g.debug %p = %d\n\n",
		     BB_VER_STR, &bb_g.debug, bb_g.debug);

#if defined(CC_BB_BENCH) && defined(CONFIG_PMON)
	pmon_reg(0, (void *)bb_bench);
#endif  /* CC_BB_BENCH && CONFIG_PMON */

#if defined(CC_BB_UNITT)
	bb_unittest();
#endif  /*  CC_BB_UNITT */

#endif  /* CONFIG_BCM96838 || CONFIG_BCM96848 */

	return 0;
}

subsys_initcall(__init_burstbank);

EXPORT_SYMBOL(bb_g);

#endif  /*  BMIPS4355_DMAE */
