#ifndef __BURSTBANK_H_INCLUDED__
#define __BURSTBANK_H_INCLUDED__
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

/* Burstbanks compilation: define BMIPS4355_DMAE */
#if defined(CONFIG_BCM96838) || defined (CONFIG_BCM96848)
#define BMIPS4355_DMAE      /* DSL SoC that uses the BMIPS4355 with DMA-E     */
#endif /*  CONFIG_BCM96838 || CONFIG_BCM96848 */

#if defined(BMIPS4355_DMAE)
/*
 * ----------------------------------------------------------------------------*
 *
 * File Name   : burstbank.h
 *
 * Provides APIs for Burst Bank manipulation in the BMIPS4355 DMA-Engine
 *  - Read and Write transactions
 *  - Copy max 16B and transfer large blocks of data
 *  - Reservation of burst bank channels
 *  - Debug, statistics and audit support
 *
 * History:
 *      1.0: Initial version.
 *           (Note): Derived for use in Host to Device mailbox transactions over
 *                   PCIe between Host 802.3 driver and Device 802.11 driver.
 *
 * -----------------------------------------------------------------------------
 */

/*
 * =============================================================================
 * Section: Conditional Compiles for LAB/Design development
 * =============================================================================
 *  Designer Note: Modifications to Burst Bank must be regressed for performance
 *               and debug/statistics/audit builds.
 * =============================================================================
 */

#define CC_BB_FNINL     /* Enable burstbank builds with inlined APIs          */
#define CC_BB_COLOR     /* Enable build with color coded debug/audit output   */
#define CC_BB_STATS     /* Enable build with statistics collection            */
#define CC_BB_DEBUG 5   /* Enable build with debugging at a specific level    */
#define CC_BB_AUDIT     /* Enable build with state audit                      */
#define CC_BB_BENCH     /* Enable build with pmon benchmarking (pmon_enable)  */
#if 0
#define CC_BB_UNITT     /* Enable build with Unit Test section                */
#endif

/*
 * =============================================================================
 * Section: Includes and Defines
 * =============================================================================
 */
#include <asm/atomic.h>                 /* atomic_t */
#include <linux/kernel.h>               /* printk, CPHYSADDR() */

struct bb;                              /* A hardware Burst Bank channel */

#define BB_VERSION                  "1.0"
#define BB_VER_STR                  "v" BB_VERSION

#define BB_SUCCESS                  0
#define BB_ERROR                    (~BB_SUCCESS)
#define BB_NULL                     ((struct bb *)NULL)

#define BB_NULL_STMT                do { /* noop */ } while (0)

/* Logical to physical address conversion */
#define BB_LOG2PHY(addr)            ((uint32_t *)CPHYSADDR(addr))

#define BB_PRINT                    printk

/*
 * Align an address/len down or up to the next specified alignment boundary
 * E.g.
 * unsigned long d = BB_ROUND_DN( (unsigned long)addr, L1_CACHE_BYTES );
 * unsigned long u = BB_ROUND_UP( (unsigned long)addr, L1_CACHE_BYTES );
 *
 */
#define BB_ROUND_DN(p, align)       ((p) & ~((align) - 1))
#define BB_ROUND_UP(p, align)       (((p) + (align) - 1) & ~((align) - 1))

#define BB_U16(bytes)               ((bytes) / sizeof(uint16_t))
#define BB_U32(bytes)               ((bytes) / sizeof(uint32_t))


/*
 * =============================================================================
 * Section: INLINE Macros
 * =============================================================================
 *
 * API inline or callable version definitions, example:
 *
 *  static inline void _bb_foo(void)
 *  { ... do foo statements ... }
 *  BB_DECLARE_FN( bb_foo, void bb_foo(void), _bb_foo() )
 *
 * =============================================================================
 */
#if defined(CC_BB_FNINL)
#   define BB_DECLARE_FN(fn_name, fn_signature, fn_body)                       \
    static inline fn_signature { fn_body; } /* APIs inlined */
#else  /* ! CC_BB_FNINL */
#   if defined(BB_IMPLEMENTATION)           /* pragma in burst bank.c */
#       define BB_DECLARE_FN(fn_name, fn_signature, fn_body)                   \
		fn_signature { fn_body; }                                              \
		EXPORT_SYMBOL(fn_name);             /* APIs declared in burstbank.c */
#   else  /* ! BB_IMPLEMENTATION */
#       define BB_DECLARE_FN(fn_name, fn_signature, fn_body)                   \
		extern fn_signature;                /* APIs exported to others */
#   endif /* ! BB_IMPLEMENTATION */
#endif /* ! CC_BB_FNINL */

/*
 * =============================================================================
 * Section: COLOR Macros
 * =============================================================================
 */
#if defined(CC_BB_COLOR)
#   define BB_COLOR(clr_code)       clr_code
#else  /* ! CC_BB_COLOR */
#   define BB_COLOR(clr_code)
#endif /* ! CC_BB_COLOR */

#define _BBh_   BB_COLOR("\e[0;36;44m")     /* Highlight color */
#define _BBr_   BB_COLOR("\e[0;31m")        /* Error     color */
#define _BBg_   BB_COLOR("\e[0;32m")        /* Debug     color */
#define _BBn_   BB_COLOR("\e[0m")           /* Reset     color */
#define _BBb_   BB_COLOR("\e[0;34m")        /* Bold      color */
#define _BBnl_  _BBn_ "\n"                  /* Reset with newline */

/*
 * =============================================================================
 * Section: STATISTICS Collection Macros
 * =============================================================================
 */
#if defined(CC_BB_STATS)
#   define BB_STATS(code)           code
#else
#   define BB_STATS(code)           BB_NULL_STMT
#endif /* ! CC_BB_STATS */

/*
 * =============================================================================
 * Section: DEBUG Macros
 *  BB_DBGL: Enables printk when the burst bank global debug level is greater
 *           or equal to the requested debug level.
 *  BB_DBG : Embed debug code segment when burst bank debug builds enabled.
 * =============================================================================
 */
#if defined(CC_BB_DEBUG)
#   define BB_DBG(code)             code
#   define BB_DBGL(lvl, fmt, arg...)                                           \
		if (bb_g.debug >= lvl)                                                 \
			BB_PRINT(_BBg_ "BB %pS: " _BBn_ fmt "\n", (void*)_THIS_IP_, ##arg)
#else  /* ! CC_BB_DEBUG */
#   define BB_DBG(code)             BB_NULL_STMT
#   define BB_DBGL(lvl, fmt, arg...)    BB_NULL_STMT
#endif /* ! CC_BB_DEBUG */

/*
 * =============================================================================
 * Section: AUDIT and ASSERT macros
 * =============================================================================
 */
#if defined(CC_BB_AUDIT)
#   define BB_AUDIT(code)           code
#else  /* ! CC_BB_AUDIT */
#   define BB_AUDIT(code)           BB_NULL_STMT
#endif /* ! CC_BB_AUDIT */

#define BB_ASSERTV(cond)                                                       \
		BB_AUDIT(if (!cond) {                                                  \
			        BB_PRINT(_BBr_ "BB %s ASSERT :" #cond _BBnl_,              \
			                 __FUNCTION__);                                    \
					return;                                                    \
				})

#define BB_ASSERTR(cond, rtn)                                                  \
		BB_AUDIT(if (!cond) {                                                  \
					BB_PRINT(_BBr_ "BB %s ASSERT :" #cond _BBnl_,              \
					         __FUNCTION__);                                    \
					return rtn;                                                \
				})

/*
 * =============================================================================
 * Section: BMIPS 4355 DMA-Engine Burst Bank Hardware Specification
 *
 * Designer Note:
 * Per shared/include/6xxx_cpu.h, a #define MIPS_BASE 0xff400000 is defined. The
 * BMIPS_DMA_CONTROL_EN_REG is essentially (MIPS_BASE + 0x34).
 * PS. BCM6368 is BMIPS4350 based and does not include a DMA-E (burst banks).
 *
 * =============================================================================
 */

#define BMIPS_DMA_CONTROL_EN_REG    0xFF400034	/* MIPS_BASE + 0x34 */
#define BMIPS_DMA_BB_BASE           0xFF500000

#define BB_SIZE_WORDS               16U
#define BB_SIZE_SHORT               (BB_SIZE_WORDS * sizeof(uint16_t))
#define BB_SIZE_BYTES               (BB_SIZE_WORDS * sizeof(uint32_t))

#define BB_MAX_CHANNELS             8U

#define __bb_aligned                __attribute__ ((__aligned__ (4)))

/*
 * -----------------------------------------------------------------------------
 *
 * BMIPS4355 Layout of Burst Banks
 *
 * Addressing DMA_Engine:
 *   31                        10 9     7 6       0
 *    |                          |       |         |
 *    ----------------------------------------------
 *    |   BMIPS_DMA_BB_BASE      |  BB#  |  Field  |
 *    ----------------------------------------------
 *
 * Each Burst bank is 32 words in size with 16+3 words. Last bank#7 carries
 * 3 extra words of global status of each of the 8 burst banks(4bit status).
 *
 * Data in a burst bank may only be accessed as an aligned word (lw/sw).
 * All burst bank DMA transactions are in multiple of word sizes.
 *
 * CAUTION:
 * 1. A user may only access the data in a burst bank as words (lw/sw).
 * 2. The data sections of adjacent burst banks are NOT contiguous.
 * 3. Burst Read/Write transactions are not through the L1-Cache. Cache/DDR
 *    coherency is the callers responsibility.
 * 4. What about Read Ahead Cache?
 *
 * -----------------------------------------------------------------------------
 */

union bb_data   /* Each burst bank can transfer 16B of 4B aligned data */
{
	uint8_t   u8[BB_SIZE_BYTES];
	uint16_t u16[BB_SIZE_SHORT];
	uint32_t u32[BB_SIZE_WORDS];
} __bb_aligned;
typedef union bb_data bb_data_t;


struct bb_block /* Block of memory to serve as src or dst of a BB transaction */
{
	uint8_t u8[0];
} __bb_aligned;
typedef struct bb_block bb_block_t;


typedef /* Hardware Specification of a Burst Bank Channel */
struct bb
{
	bb_data_t   data;           /* READ/WRITE as u32 ONLY !!!                 */
	uint32_t    * memory_p;     /* Destination start address in DMA-M         */
	uint32_t    burst_words;    /* Number of words in transaction             */
	uint32_t    transaction;    /* Type of transaction RD or WR               */

	union
	{
		int     padding06[13];  /* Banks #0 to Bank #6, do not have status    */

		struct
		{
			    /* Global status registers in last burst bank, 4bits per bank */
			uint32_t reg_DMAE;  /* Outstanding DMA transactions               */
			uint32_t reg_UBUS;  /* Outstanding DMA transactions to UBUS       */
			uint32_t reg_MISB;  /* Outstanding DMA transactions to SMISB      */
			uint32_t pad7[10];  /* Bank #7                                    */
		} status;
	};
} bb_t;


/* Macro: Access a burst bank data. Only aligned word access permitted */
#define BB_DATA(bb_p, i)            (bb_p)->data.u32[ i ]

/* Macro: Access the base pointer of a burst bank from channel number */
#define BB_POINTER(channel)         ((bb_t*)(BMIPS_DMA_BB_BASE) + (channel))

/* Macro: Access the channel number from a burst bank pointer */
#define BB_CHANNEL(pointer)                                                    \
	(((uint32_t)(pointer) - BMIPS_DMA_BB_BASE) / sizeof(bb_t))

/* Macro: validate a burst bank pointer */
#define BB_CHECK(pointer) \
	(((bb_t*)(pointer) != BB_NULL) && \
	 ((((uint32_t)(pointer) - BMIPS_DMA_BB_BASE) % sizeof(bb_t)) == 0) && \
	 (BB_CHANNEL(pointer) < BB_MAX_CHANNELS))


/*
 * -----------------------------------------------------------------------------
 * Burst Bank Status (DMAE, UBUS, MISB) (global register in last Burst Bank)
 * -----------------------------------------------------------------------------
 */

/* A status word contains 4bits per channel */
#define BB_REG_STATUS_BITS          4

#define BB_REG_STATUS(rname)                                                   \
	&(((bb_t*)(BMIPS_DMA_BB_BASE)) + (BB_MAX_CHANNELS - 1))->status.reg_##rname

#define BB_REG_STATUS_DMAE          BB_REG_STATUS(DMAE)
#define BB_REG_STATUS_UBUS          BB_REG_STATUS(UBUS)
#define BB_REG_STATUS_MISB          BB_REG_STATUS(MISB)

typedef /* Status reported by hardware on a transaction issuance */
enum bb_status
{
	bb_read_pending = (1 << 0U),        /* Outstanding read transaction       */
	bb_write_pending = (1 << 1U),       /* Outstanding write transaction      */
	bb_error_condition = (1 << 2U)      /* Error condition                    */
	/* bit #4 is reserved */
} bb_status_t;


#define BB_RD_TRANS(bb_p)		    ((bb_p)->transaction & bb_read_pending)
#define BB_WR_TRANS(bb_p)		    ((bb_p)->transaction & bb_write_pending)
#define BB_ERR_COND(bb_p)		    ((bb_p)->transaction & bb_error_condition)

/* Pending read or write transaction */
#define BB_TRANSACTION              (bb_read_pending | bb_write_pending)

/* Pending read or write transaction, or error constition */
#define BB_STATUS_USED              ((1 << BB_REG_STATUS_BITS) - 1U)

/* Macro: Access the status 4bits for a channel given a status word */
#define BB_STATUS_GBL(status_word, channel)                                    \
	((status_word) >> ((channel) * BB_REG_STATUS_BITS))

/*
 * -----------------------------------------------------------------------------
 *
 * Read a status register and test for pending transaction or error condition
 * Returns a 4 bit value containing outstanding transactions
 *
 * Accessor APIS:
 *      bb_DMAE(channel, status)
 *      bb_UBUS(channel, status)
 *      bb_MISB(channel, status)
 *
 *    channel - bb_channel_t
 *    status - 3 bit value described in bb_status_t
 *
 * -----------------------------------------------------------------------------
 */

#define __BB_BUILD_STATUS_ACCESSOR(TYPE)                                       \
static inline uint32_t                                                         \
bb_##TYPE(const uint32_t channel, const uint32_t status)                       \
{                                                                              \
	register volatile uint32_t status##TYPE;                                   \
	BB_ASSERTR((channel < BB_MAX_CHANNELS), BB_ERROR);                         \
	BB_ASSERTR((status <= BB_STATUS_USED), BB_ERROR);                          \
	status##TYPE = (*(volatile uint32_t *)(BB_REG_STATUS_##TYPE));             \
	return (BB_STATUS_GBL(status##TYPE, channel) & status);                    \
}
__BB_BUILD_STATUS_ACCESSOR(DMAE)        /* function: bb_DMAE(channel, status) */
__BB_BUILD_STATUS_ACCESSOR(UBUS)        /* function: bb_UBUS(channel, status) */
__BB_BUILD_STATUS_ACCESSOR(MISB)        /* function: bb_MISB(channel, status) */


/*
 * =============================================================================
 * Section: Pre-assignment of channels usage
 * =============================================================================
 *
 * Burst Bank DMA Channels are dedicated for specific access function
 * Instead of using a reservation mechanism, for datapath channels reserved for
 * specific purposes in datapath may be used as an alternative.
 *
 * Alternatively, reserving 4 banks per CPU core is another option.
 *
 * While a named reservation of a burst bank breaks the abstraction model, it
 * serves the performance requirement for datapath use.
 *
 * =============================================================================
 */
typedef /* Named allocation, callers responsibility for mutual access to bank */
enum bb_channel
{
	bb_channel_xfer0,   /* Four banks reserved for xfer of >64B data chunks   */
	bb_channel_xfer1,
	bb_channel_xfer2,
	bb_channel_xfer3,

	bb_channel_copy,    /* DMA access to copy <= 64 Bytes                     */

	bb_channel_rxbd,    /* DMA access to receive buffer descriptors           */
	bb_channel_txbd,    /* DMA access to transmit buffer descriptors          */
	bb_channel_reclbd,  /* DMA access to reclaim transmit buffer descriptors  */

	bb_channel_max = BB_MAX_CHANNELS
} bb_channel_t;

typedef /* Allocation modes supported by API */
enum bb_alloc_mode
{
	bb_alloc_check_status,
	bb_alloc_ignore_status,     /* Ignore error or pending transaction, clear */
} bb_alloc_mode_t;

/*
 * =============================================================================
 * Section: Burst Bank Channel state
 * Allocation state and statistics per channel
 * =============================================================================
 */
typedef /* Software state per burst bank channel */
struct bb_state
{
	atomic_t    in_use;             /* Allocation via compile time delegation */
#if defined(CC_BB_STATS)
	uint32_t    reserves, releases, copies;
	struct { uint32_t ops, polls, errors; } RD, WR;
#endif /*   CC_BB_STATS */
} bb_state_t;


/*
 * =============================================================================
 * Section: Global Burst Bank System State
 *  ch_state: per channel, in use and statistics
 * =============================================================================
 */
typedef /* Global instance of burst bank software state, debug, statistics */
struct bb_global
{
	uint32_t    debug;
	bb_state_t  ch_state[BB_MAX_CHANNELS];
} bb_global_t;

extern bb_global_t bb_g;    /* Burst Bank Global State of all channels */

/*
 * =============================================================================
 * Section: Channel Reservation
 *  Burst bank allocation/reservation.
 *  For datapath, statically reserving channels for specific purposes is
 *  preferrable for performance.
 * =============================================================================
 */

/*
 * -----------------------------------------------------------------------------
 * API bb_status
 * Description: Determine on the requested burst bank,
 *              if any read or write transaction is pending, or,
 *              if an error condition exists.
 * Parameters:
 *      bb_p    : pointer to a burst bank channel
 *   transaction: pending read, write and/or error condition test
 * -----------------------------------------------------------------------------
 */
static inline uint32_t
_bb_status(volatile bb_t * const bb_p, const uint32_t transaction)
{
	BB_ASSERTR(BB_CHECK(bb_p), BB_ERROR);
	BB_ASSERTR((transaction <= bb_error_condition), BB_ERROR);

	return (bb_p->transaction & transaction);
}
BB_DECLARE_FN(
	bb_status,      /* Determine the status of a channel */
	uint32_t bb_status(volatile bb_t * const bb_p, const uint32_t transaction),
	return _bb_status(bb_p, transaction))


/*
 * -----------------------------------------------------------------------------
 * API bb_clear
 * Description: Clear error or pending transactions, and memories.
 * Parameters:
 *   bb_p       : pointer to a burst bank channel
 *
 * Design Note: No mention on how to clear error condition.
 * -----------------------------------------------------------------------------
 */
static inline void
_bb_clear(volatile bb_t * const bb_p)
{
	BB_ASSERTV(BB_CHECK(bb_p));

	bb_p->memory_p = (uint32_t*)0x0;
	bb_p->burst_words = 0U;	        /* Hmmm ... 0 implies at least 1 */
	bb_p->transaction = 0U;         /* Force a clear of pending transaction ? */

	BB_DBG({
		uint32_t wordIx;
		for (wordIx = 0U; wordIx < BB_SIZE_WORDS; wordIx++) {
			BB_DATA(bb_p, wordIx) = 0U;
		}
	});
}
BB_DECLARE_FN(
	bb_clear,       /* Zero out a burst bank channel */
	void bb_clear(bb_t * const bb_p),
	_bb_clear(bb_p))

/*
 * -----------------------------------------------------------------------------
 * API bb_reset
 * Description: Resets status of outstanding transaction or error conditions.
 * Parameters:
 *   channel    : index of a burst bank 0..7
 * -----------------------------------------------------------------------------
 */
static inline void
_bb_reset(const uint32_t channel)
{
	volatile bb_t * bb_p;

	BB_ASSERTV((channel < BB_MAX_CHANNELS));

	bb_p = BB_POINTER(channel);

	_bb_clear(bb_p);

	BB_DBG(/* udelay(1);  Need to wait for the bank to complete transaction ? */
	    if (bb_DMAE(channel, BB_STATUS_USED)) {
			BB_DBGL(0U, "bb_reset channel %d failure", channel);
		});

	BB_ASSERTV((bb_status(bb_p, BB_TRANSACTION) == 0));
}
BB_DECLARE_FN(
	bb_reset,     /* Reset pending transaction|error */
	void bb_reset(const uint32_t channel),
	_bb_reset(channel))

/*
 * -----------------------------------------------------------------------------
 * API bb_reserve
 * Description: Reserve a channel for exclusive use.
 * Parameters:
 *   bb_p       : pointer to a burst bank channel
 *   alloc_condition: ignore or check for status before allocation.
 * -----------------------------------------------------------------------------
 */
static inline bb_t *
_bb_reserve(const uint32_t channel, const int alloc_condition)
{
	atomic_t * channel_in_use_p;
	volatile bb_t * bb_p;

	BB_STATS(bb_state_t * state_p);

	BB_ASSERTR((channel < BB_MAX_CHANNELS), BB_NULL);
	BB_ASSERTR((alloc_condition <= bb_alloc_ignore_status), BB_NULL);

	channel_in_use_p = & bb_g.ch_state[channel].in_use;
	BB_ASSERTR((atomic_read(channel_in_use_p) == 0U), BB_NULL);

	bb_p = BB_POINTER(channel);

	BB_STATS(state_p = &bb_g.ch_state[channel]);

	/* Check if reserved by another user */
	if (unlikely(atomic_read(channel_in_use_p))) {
		BB_DBGL(0U, "channel<%u> bb_p<0x%08x> in_use transaction<0x%08x>"
		            "memory_p<0x%08x> burst_words<%u>",
		            channel, (int)bb_p, (int)bb_p->transaction,
		            (int)bb_p->memory_p, bb_p->burst_words);
		return BB_NULL;
	}

	/* Check if outstanding transaction or error condition */
	if (unlikely(bb_DMAE(channel, BB_STATUS_USED))) {
		if (unlikely(alloc_condition == bb_alloc_ignore_status)) {
			_bb_reset(channel);     /* Clear condition */
		} else {
			BB_DBGL(0U, "channel<%u> bb_p<0x%08x> pending transaction<0x%08x>"
			            "memory_p<0x%08x> burst_words<%u>",
			            channel, (int)bb_p, (int)bb_p->transaction,
			            (int)bb_p->memory_p, bb_p->burst_words);
			return BB_NULL;
		}
	}

	/* Reserve channel */
	atomic_inc(channel_in_use_p);
	BB_ASSERTR((atomic_read(channel_in_use_p) == 1U), BB_NULL);

	BB_STATS(state_p->reserves++);

	return (bb_t *)bb_p;
}
BB_DECLARE_FN(
	bb_reserve,     /* Reserve a burst bank channel */
	bb_t * bb_reserve(const uint32_t channel, const int alloc_condition),
	return _bb_reserve(channel, alloc_condition))


/*
 * -----------------------------------------------------------------------------
 * API bb_release
 * Description: Release a previously reserved channel.
 * Parameters:
 *   channel    : index of a burst bank 0..7
 * -----------------------------------------------------------------------------
 */
static inline
void _bb_release(const uint32_t channel)
{
	atomic_t * channel_in_use_p;

	BB_STATS(bb_state_t * state_p);

	BB_ASSERTV((channel < BB_MAX_CHANNELS));

	channel_in_use_p = &bb_g.ch_state[channel].in_use;
	BB_ASSERTV((atomic_read(channel_in_use_p) == 1U));

	BB_STATS(state_p = &bb_g.ch_state[channel]);

	/* Ensure no pending transaction or error condition exists */
	_bb_reset(channel);

	/* Unreserve channel */
	atomic_dec(channel_in_use_p);

	BB_ASSERTV((atomic_read(channel_in_use_p) == 0U));

	BB_STATS(state_p->releases++);
	return;
}
BB_DECLARE_FN(
	bb_release,     /* Release a burst bank channel */
	void bb_release(const uint32_t channel),
	_bb_release(channel))


/*
 * =============================================================================
 * Section: Bursting Read & Write transactions, with wait for completion
 * =============================================================================
 */

/*
 * -----------------------------------------------------------------------------
 * API bb_wait
 * Description: Wait for a transaction to complete.
 * Parameters:
 *   bb_p       : pointer to a burst bank channel
 *   transaction: read or write transaction
 * -----------------------------------------------------------------------------
 */
static inline uint32_t
_bb_wait(volatile bb_t * const bb_p, const uint32_t transaction)
{
	uint32_t not_done;
	BB_STATS(int channel = BB_CHANNEL(bb_p);
	         bb_state_t * state_p = &bb_g.ch_state[channel]);

	BB_ASSERTR(BB_CHECK(bb_p), BB_ERROR);
	BB_ASSERTR(((transaction == bb_read_pending) ||
	           (transaction == bb_write_pending)), BB_ERROR);

	/* Wait on transaction to complete (or error) */
	not_done = bb_p->transaction;

	while (likely(not_done)) {

		if (unlikely(not_done & bb_error_condition)) {
			BB_STATS((transaction == bb_read_pending)
			         ? state_p->RD.errors++ : state_p->WR.errors++);
			BB_DBGL(0U, "bb_p<0x%08x> error condition "
			            "memory_p<0x%08x> burst_words<%u>",
			            (int)bb_p, (int)bb_p->memory_p, bb_p->burst_words);
			return BB_ERROR;
		}

		BB_STATS((transaction == bb_read_pending)
		         ? state_p->RD.polls++ : state_p->WR.polls++);

		not_done = bb_p->transaction;
	}

	return BB_SUCCESS;
}
BB_DECLARE_FN(
	bb_wait,        /* Wait on transaction to complete on a channel */
	uint32_t bb_wait(volatile bb_t * const bb_p, const uint32_t transaction),
	return _bb_wait(bb_p, transaction))

/*
 * -----------------------------------------------------------------------------
 * API bb_issue
 * Description: Issue a read or write transaction and return ASYNCHRONOUSLY.
 * Parameters:
 *   bb_p       : pointer to a burst bank channel
 *   memory_p   : physical memory address to which the bank will be mapped
 *   burst_words : number of words 0..15.
 *   transaction: read or write transaction
 *
 * CAUTION: memory_p must be a non zero word aligned physical address
 *          burst_words is total words - 1
 *
 * -----------------------------------------------------------------------------
 */
static inline void
_bb_issue(volatile bb_t * const bb_p, uint32_t * memory_p,
	      const uint32_t burst_words, const uint32_t transaction)
{
	BB_STATS(int channel = BB_CHANNEL(bb_p);
	         bb_state_t * state_p = &bb_g.ch_state[channel]);

	BB_ASSERTV(BB_CHECK(bb_p));
	BB_ASSERTV((bb_status(bb_p, BB_TRANSACTION) == 0));

	BB_ASSERTV((memory_p != (uint32_t*)NULL));
	BB_ASSERTV((((uint32_t)memory_p & 0x3) == 0U));
	BB_ASSERTV(((uint32_t)memory_p == (uint32_t)BB_LOG2PHY(memory_p)));
	BB_ASSERTV((burst_words < BB_SIZE_WORDS));
	BB_ASSERTV(((transaction == bb_read_pending) ||
	           (transaction == bb_write_pending)));

	bb_p->memory_p = memory_p;          /* physical memory pointer */
	bb_p->burst_words = burst_words;    /* num_words - 1 */

	bb_p->transaction = transaction;    /* read or write initiated to HW */

	BB_STATS((transaction == bb_read_pending)
	         ? state_p->RD.ops++ : state_p->WR.ops++);
}
BB_DECLARE_FN(
	bb_issue,       /* Issue a read|write transaction on a channel */
	void bb_issue(volatile bb_t * const bb_p, uint32_t * memory_p,
	              const uint32_t burst_words ,  /* num_words - 1 */
	              const uint32_t transaction),
	_bb_issue(bb_p, memory_p, burst_words, transaction))

/*
 * -----------------------------------------------------------------------------
 *
 * API bb_copy
 * Description: A maximum of 64Bytes of data will be copied from a word aligned
 *		location to a word-aligned location.
 *
 * Parameters:
 *   src_p      : source physical memory address: read into bank
 *   dst_p      : destination physical memory address: written from bank
 *   burst_words : number of words 0..15
 *   channel    : index of a burst bank 0..7
 *
 * CAUTION: local_p and memory_p must be non-zero 4B aligned physical addresses
 *          burst_words is number of words - 1
 *          An error on read, results in garbage write.
 * -----------------------------------------------------------------------------
 */
static inline uint32_t
_bb_copy(const uint32_t * src_p, uint32_t * dst_p,
	     const uint32_t burst_words, const uint32_t channel)
{
	uint32_t ret;
	volatile bb_t * bb_p = BB_POINTER(channel);

	BB_STATS(bb_state_t * state_p = &bb_g.ch_state[channel]);

	BB_ASSERTR(BB_CHECK(bb_p), BB_ERROR);
	BB_ASSERTR((bb_p->transaction == 0U), BB_ERROR);

	BB_ASSERTR((dst_p != (uint32_t *)NULL), BB_ERROR);
	BB_ASSERTR((src_p != (uint32_t *)NULL), BB_ERROR);
	BB_ASSERTR((((uint32_t)src_p & 0x3) == 0U), BB_ERROR);
	BB_ASSERTR((((uint32_t)dst_p & 0x3) == 0U), BB_ERROR);
	BB_ASSERTR((burst_words < BB_SIZE_WORDS), BB_ERROR);
	BB_ASSERTR((channel < BB_MAX_CHANNELS), BB_ERROR);

	/* First read from source memory into burst bank */
	_bb_issue(bb_p, (uint32_t *)src_p, burst_words, bb_read_pending);
	ret = _bb_wait(bb_p, bb_read_pending);

	/* Then copy to destination memory, "ignoring read error" */
	_bb_issue(bb_p, dst_p, burst_words, bb_write_pending);
	ret |= _bb_wait(bb_p, bb_write_pending);

	BB_STATS(state_p->copies++);

	BB_ASSERTR((ret == BB_SUCCESS), ret);
	return ret;
}
BB_DECLARE_FN(
	bb_copy,        /* Copy from|to memory to|from local */
	uint32_t bb_copy(const uint32_t * src_p, uint32_t * dst_p,
	                 const uint32_t burst_words,   /* num_words - 1 */
	                 const uint32_t channel),
	return _bb_copy(src_p, dst_p, burst_words, channel))

/*
 * -----------------------------------------------------------------------------
 *
 * API bb_xfer
 * Description: Copy data from one location to another (larger than 64B)
 *		Source and destination location must be word-aligned.
 *
 * Parameters:
 *   src_p      : physical memory address to be read into bank
 *   dst_p      : physical memory address to be written from bank
 *   num_words   : number of words
 *
 * CAUTION: local_p and memory_p must be non-zero 4B aligned  physical addresses
 *          An error on read, results in garbage write.
 * -----------------------------------------------------------------------------
 */
static inline uint32_t
_bb_xfer(const uint32_t * src_p, uint32_t * dst_p, const uint32_t num_words)
{
	uint32_t ret, i, bbs;
	volatile bb_t * bb_p = BB_POINTER(bb_channel_xfer0);
	uint32_t words = (uint32_t) num_words;

	BB_STATS(bb_state_t * state_p = &bb_g.ch_state[bb_channel_xfer0]);

	BB_ASSERTR(BB_CHECK(bb_p), BB_ERROR);
	BB_ASSERTR(((bb_p + 0)->transaction == 0U), BB_ERROR);
	BB_ASSERTR(((bb_p + 1)->transaction == 0U), BB_ERROR);
	BB_ASSERTR(((bb_p + 2)->transaction == 0U), BB_ERROR);
	BB_ASSERTR(((bb_p + 3)->transaction == 0U), BB_ERROR);

	BB_ASSERTR((dst_p != (uint32_t *)NULL), BB_ERROR);
	BB_ASSERTR((src_p != (uint32_t *)NULL), BB_ERROR);
	BB_ASSERTR((((uint32_t)src_p & 0x3) == 0U), BB_ERROR);
	BB_ASSERTR((((uint32_t)dst_p & 0x3) == 0U), BB_ERROR);

	while (likely(words > BB_SIZE_WORDS)) {     /* Copy using multiple banks */

		if (words >= (BB_SIZE_WORDS * 4))
			bbs = 4;                            /* Use 4 banks in parallel */
		else if (words >= (BB_SIZE_WORDS * 3))
			bbs = 3;                            /* Use 3 banks in parallel */
		else if (words >= (BB_SIZE_WORDS * 2))
			bbs = 2;                            /* Use 2 banks in parallel */
		else
			bbs = 1;

		/* Wait on previous write(s) to complete, then read */
		for (i = 0; likely(i < bbs); i++) {
			ret |= _bb_wait(bb_p + i, bb_write_pending);
			_bb_issue(bb_p + i, (uint32_t *)src_p, BB_SIZE_WORDS - 1,
			          bb_read_pending);
			src_p += BB_SIZE_WORDS;
		}

		/* Wait on previous read(s) to complete, then write */
		for (i = 0; likely(i < bbs); i++) {
			ret |= _bb_wait(bb_p + i, bb_read_pending);
			_bb_issue(bb_p + i, dst_p, BB_SIZE_WORDS - 1, bb_write_pending);
			dst_p += BB_SIZE_WORDS;
		}

		words -= BB_SIZE_WORDS * bbs;
		BB_STATS(state_p->copies += bbs);
	}

	if (likely(words > 0)) {    /* Copy leftover words */
		ret |= bb_copy(src_p, dst_p, words - 1, bb_channel_copy);
	}

	/* Wait on last set of parralel write(s) to complete */
	for (i = 0; likely(i < 4); i++) {
		ret |= _bb_wait(bb_p + i, bb_write_pending);
	}

	BB_ASSERTR((ret == BB_SUCCESS), ret);
	return ret;
}
BB_DECLARE_FN(
	bb_xfer,        /* Bulk xfer from|to memory to|from local */
	uint32_t bb_xfer(const uint32_t * src_p, uint32_t * dst_p,
	                 const uint32_t num_words),
	return _bb_xfer(src_p, dst_p, num_words))

/*
 * =============================================================================
 * Section: Debug APIs
 * =============================================================================
 */
#if defined(CC_BB_DEBUG)
	extern void bb_dump(volatile bb_t * const bb_p, int verbose);
	extern void bb_show(int verbose);
#else   /* !CC_BB_DEBUG */
#	define bb_dump(bb_p, verbose)		BB_NULL_STMT
#	define bb_show(verbose)				BB_NULL_STMT
#endif  /* !CC_BB_DEBUG */

#endif  /*  BMIPS4355 */

#endif /* defined(__BURSTBANK_H_INCLUDED__) */
