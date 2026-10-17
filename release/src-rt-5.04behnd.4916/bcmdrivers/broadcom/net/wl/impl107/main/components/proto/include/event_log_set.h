/*
 * EVENT_LOG system definitions
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Dual:>>
 *
 * $Id: $
 */

#ifndef _EVENT_LOG_SET_H_
#define _EVENT_LOG_SET_H_

#ifdef EVENT_LOG_SET_BCA_NEW_DEFINITION

#define EVENT_LOG_SET_NUM_BLOCKS_DEFAULT	(2u)

/* Set assignments for BCA */

/* set 0 String Log Level 0 */
#define EVENT_LOG_SET_STR_LOG_LVL_0		(0u)
#ifndef EVENT_LOG_SET_STR_LOG_LVL_0_NUM_BLOCKS
#define EVENT_LOG_SET_STR_LOG_LVL_0_NUM_BLOCKS	EVENT_LOG_SET_NUM_BLOCKS_DEFAULT
#endif
#ifndef EVENT_LOG_SET_STR_LOG_LVL_0_BLOCK_SIZE
#define EVENT_LOG_SET_STR_LOG_LVL_0_BLOCK_SIZE	(EVENT_LOG_BLOCK_SIZE_1K)
#endif

/* set 1 String Log Level 1 */
#define EVENT_LOG_SET_STR_LOG_LVL_1		(1u)
#ifndef EVENT_LOG_SET_STR_LOG_LVL_1_NUM_BLOCKS
#define EVENT_LOG_SET_STR_LOG_LVL_1_NUM_BLOCKS	EVENT_LOG_SET_NUM_BLOCKS_DEFAULT
#endif
#ifndef EVENT_LOG_SET_STR_LOG_LVL_1_BLOCK_SIZE
#define EVENT_LOG_SET_STR_LOG_LVL_1_BLOCK_SIZE	(EVENT_LOG_BLOCK_SIZE_1K)
#endif

/* set 2 String Log Level 2 */
#define EVENT_LOG_SET_STR_LOG_LVL_2		(2u)
#ifndef EVENT_LOG_SET_STR_LOG_LVL_2_NUM_BLOCKS
#define EVENT_LOG_SET_STR_LOG_LVL_2_NUM_BLOCKS	EVENT_LOG_SET_NUM_BLOCKS_DEFAULT
#endif
#ifndef EVENT_LOG_SET_STR_LOG_LVL_2_BLOCK_SIZE
#define EVENT_LOG_SET_STR_LOG_LVL_2_BLOCK_SIZE	(EVENT_LOG_BLOCK_SIZE_1K)
#endif

/* set 3 String Log Level 3 */
#define EVENT_LOG_SET_STR_LOG_LVL_3		(3u)
#ifndef EVENT_LOG_SET_STR_LOG_LVL_3_NUM_BLOCKS
#define EVENT_LOG_SET_STR_LOG_LVL_3_NUM_BLOCKS	EVENT_LOG_SET_NUM_BLOCKS_DEFAULT
#endif
#ifndef EVENT_LOG_SET_STR_LOG_LVL_3_BLOCK_SIZE
#define EVENT_LOG_SET_STR_LOG_LVL_3_BLOCK_SIZE	(EVENT_LOG_BLOCK_SIZE_1K)
#endif

/* set 4 Debug purpose for Sring log Level 0 */
#define EVENT_LOG_SET_DBG_STR_LOG_LVL_0		(4u)

/* set 5 Debug purpose for Sring log Level 1 */
#define EVENT_LOG_SET_DBG_STR_LOG_LVL_1		(5u)

/* set 6 Debug purpose for Sring log Level 2 */
#define EVENT_LOG_SET_DBG_STR_LOG_LVL_2		(6u)

/* Important: Please do not add any SET for string log usage after EVENT_LOG_SET_BINARY_DBG
 * EVENT_LOG_IS_STR_SETNUM(setnum)	check setnum < EVENT_LOG_SET_BINARY_DBG
 */
/* set 7 Debug purpose for Binary data */
#define EVENT_LOG_SET_DBG_BINARY_DATA		(7u)

/* set 8 ECOUNTERS */
#define EVENT_LOG_SET_ECOUNTERS			(8u)
#ifndef EVENT_LOG_SET_ECOUNTERS_NUM_BLOCKS
#define EVENT_LOG_SET_ECOUNTERS_NUM_BLOCKS	EVENT_LOG_SET_NUM_BLOCKS_DEFAULT
#endif
#ifndef EVENT_LOG_SET_ECOUNTERS_BLOCK_SIZE
#define EVENT_LOG_SET_ECOUNTERS_BLOCK_SIZE	(EVENT_LOG_BLOCK_SIZE_1K)
#endif

/* set 9 Phy Periodic log feature */
#define EVENT_LOG_SET_PHY_PERIODIC		(9u)
#ifndef EVENT_LOG_SET_PHY_PERIODIC_NUM_BLOCKS
#define EVENT_LOG_SET_PHY_PERIODIC_NUM_BLOCKS	EVENT_LOG_SET_NUM_BLOCKS_DEFAULT
#endif
#ifndef EVENT_LOG_SET_PHY_PERIODIC_BLOCK_SIZE
#define EVENT_LOG_SET_PHY_PERIODIC_BLOCK_SIZE	(EVENT_LOG_BLOCK_SIZE_1K)
#endif

/* set 10 Phy Calibiration log feature */
#define EVENT_LOG_SET_PHY_CAL			(10u)
#ifndef EVENT_LOG_SET_PHY_CAL_NUM_BLOCKS
#define EVENT_LOG_SET_PHY_CAL_NUM_BLOCKS	EVENT_LOG_SET_NUM_BLOCKS_DEFAULT
#endif
#ifndef EVENT_LOG_SET_PHY_CAL_BLOCK_SIZE
#define EVENT_LOG_SET_PHY_CAL_BLOCK_SIZE	(EVENT_LOG_BLOCK_SIZE_512B)
#endif

#ifndef NUM_EVENT_LOG_SETS
/* Set a maximum number of sets here.  It is not dynamic for
 * efficiency of the EVENT_LOG calls. Old branches could define
 * this to an appropriat enumber in their makefiles to reduce
 * ROM invalidation
 */
#ifdef NUM_EVENT_LOG_SETS_V2
/* for v2, everything has became unsigned */
#define NUM_EVENT_LOG_SETS (11u)
#else /* NUM_EVENT_LOG_SETS_V2 */
#define NUM_EVENT_LOG_SETS (11)
#endif /* NUM_EVENT_LOG_SETS_V2 */
#endif /* NUM_EVENT_LOG_SETS */

#else /* !EVENT_LOG_SET_BCA_NEW_DEFINITION */

/* Set assignments */
#define EVENT_LOG_SET_BUS		(0u)
#define EVENT_LOG_SET_WL		(1u)
#define EVENT_LOG_SET_PSM		(2u)
#define EVENT_LOG_SET_ERROR		(3u)

/* MSCH logging */
#define EVENT_LOG_SET_MSCH_PROFILER	(4u)

/* A particular customer uses sets 5, 6, and 7. There is a request
 * to not name these log sets as that could limit their ability to
 * use different log sets in future.
 * Sets 5, 6, and 7 are instantiated by host
 * In such case, ecounters could be mapped to any set that host
 * configures. They may or may not use set 5.
 */
#define EVENT_LOG_SET_5			(5u)
#define EVENT_LOG_SET_ECOUNTERS		(EVENT_LOG_SET_5)
#define EVENT_LOG_SET_6			(6u)
#define EVENT_LOG_SET_7			(7u)

/* Temporary change to satisfy compilation across branches
 * Will be removed after checkin
 */
#define EVENT_LOG_SET_8			(8u)
#define EVENT_LOG_SET_PRSRV		(EVENT_LOG_SET_8)

#define EVENT_LOG_SET_9			(9u)
/* General purpose preserve chatty.
 * EVENT_LOG_SET_PRSRV_CHATTY log set should not be used by FW as it is
 * used by customer host. FW should use EVENT_LOG_SET_GP_PRSRV_CHATTY
 * for general purpose preserve chatty logs.
 */
#define EVENT_LOG_SET_GP_PRSRV_CHATTY	(EVENT_LOG_SET_9)
#define EVENT_LOG_SET_PRSRV_CHATTY	(EVENT_LOG_SET_6)

/* BUS preserve */
#define EVENT_LOG_SET_PRSRV_BUS		(10u)

/* WL preserve */
#define EVENT_LOG_SET_PRSRV_WL		(11u)

/* Slotted BSS set */
#define EVENT_LOG_SET_WL_SLOTTED_BSS    (12u)

/* PHY entity logging */
#define EVENT_LOG_SET_PHY		(13u)

/* PHY preserve */
#define EVENT_LOG_SET_PRSRV_PHY		(14u)
/* PHY periodic */
/* Use the newer name in future */
#define EVENT_LOG_SET_PERIODIC_PHY	(14u)

/* RTE entity */
#define EVENT_LOG_SET_RTE		(15u)

/* Malloc and free logging */
#define EVENT_LOG_SET_MEM_API		(16u)

/* Console buffer */
#define EVENT_LOG_SET_RTE_CONS_BUF	(17u)

/* three log sets for general debug purposes */
#define EVENT_LOG_SET_GENERAL_DBG_1	(18u)
#define EVENT_LOG_SET_GENERAL_DBG_2	(19u)
#define EVENT_LOG_SET_GENERAL_DBG_3	(20u)

/* Log sets for capturing power related logs. Note that these sets
 * are to be used across entire system and not just WL.
 */
#define EVENT_LOG_SET_POWER_1		(21u)
#define EVENT_LOG_SET_POWER_2		(22u)

/* Used for timestamp plotting, TS_LOG() */
#define EVENT_LOG_SET_TS_LOG		(23u)

/* BUS preserve chatty */
#define EVENT_LOG_SET_PRSRV_BUS_CHATTY	(24u)

/* PRESERVE_PREIODIC_LOG_SET */
/* flush if host is in D0 at every period */
#define EVENT_LOG_SET_PRSV_PERIODIC	(25u)

/* AMT logging and other related information */
#define EVENT_LOG_SET_AMT		(26u)

/* State machine logging. Part of preserve logs */
#define EVENT_LOG_SET_FSM		(27u)

/* wbus related logging */
#define EVENT_LOG_SET_WBUS		(28u)

/* bcm trace logging */
#define EVENT_LOG_SET_BCM_TRACE		(29u)

/* For PM alert related logging */
#define EVENT_LOG_SET_WL_PS_LOG		(30u)

/* For SIB co-ex logging */
#define EVENT_LOG_SET_WL_SIB_LOG	(31u)

/* For EWP HW Init logging */
#define EVENT_LOG_SET_EWP_HW_INIT_LOG	(32u)

// MOG-ON: CHRE
#ifdef CHRE
/* CHRE related logging */
#define EVENT_LOG_SET_CHRE              (33u)
#define EVENT_LOG_SET_CHRE_CHATTY       (34u)
#endif /* CHRE */
// MOG-OFF: CHRE

// MOG-ON: COEX_CPU
#ifdef COEX_CPU
/* Shdow log sets for coex cpu */
#define EVENT_LOG_SET_COEX_SHADOW_INFO  (35u)
#define EVENT_LOG_SET_COEX_SHADOW_ERR   (36u)
#define EVENT_LOG_SET_COEX_SHADOW_PRSRV (37u)
#endif /* COEX_CPU */
// MOG-OFF: COEX_CPU

#ifndef NUM_EVENT_LOG_SETS
/* Set a maximum number of sets here.  It is not dynamic for
 * efficiency of the EVENT_LOG calls. Old branches could define
 * this to an appropriat enumber in their makefiles to reduce
 * ROM invalidation
 */
#ifdef NUM_EVENT_LOG_SETS_V2
/* for v2, everything has became unsigned */
#define NUM_EVENT_LOG_SETS (38u)
#else /* NUM_EVENT_LOG_SETS_V2 */
#define NUM_EVENT_LOG_SETS (38)
#endif /* NUM_EVENT_LOG_SETS_V2 */
#endif /* NUM_EVENT_LOG_SETS */

#endif /* EVENT_LOG_SET_BCA_NEW_DEFINITION */

/* send delayed logs when >= 50% of buffer is full */
#ifndef ECOUNTERS_DELAYED_FLUSH_PERCENTAGE
#define ECOUNTERS_DELAYED_FLUSH_PERCENTAGE	(50)
#endif

#endif /* _EVENT_LOG_SET_H_ */
