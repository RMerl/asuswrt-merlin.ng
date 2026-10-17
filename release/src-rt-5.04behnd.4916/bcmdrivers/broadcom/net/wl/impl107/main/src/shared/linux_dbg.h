/*
 * DHD & NIC debugability header file
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
 * $Id$
 */

#ifndef _linux_dbg_h_
#define _linux_dbg_h_
#include <osl.h>
#include <bcmutils.h>
#include <event_log_payload.h>
#include <eldbg_ring.h>

#define DBG_ERROR(args)		do { printf args; } while (0)

/* if necessary, add printf to DBG_INFO */
#define DBG_INFO(args)

/* control_logtrace bit control
 * bit 0: string log on/off : 0 - disable, 1 - enable
 * bit 1: string log type: 0 - raw encoded, 1 - parsed
 * (NIC doesn't refer bit 1 and type is string itself)
 * bit 2: chatty string log on/off : 0 - disable, 1 - enable
 * bit 3: chatty string log type : 0 - raw encoded, 1 - parsed
 * (NIC doesn't refer bit 3 and type is string itself)
 * bit 4: binary data on/off: 0 - disable, 1 - enable
 */

#define CTL_LT_STRING_ENABLE		(0x01u)
#define CTL_LT_STRING_TYPE		(0x02u)
#define CTL_LT_STRING_CHATTY_ENABLE	(0x04u)
#define CTL_LT_STRING_CHATTY_TYPE	(0x08u)
#define CTL_LT_BINARY_ENABLE		(0x10u)

#define CTL_LT_IS_ON(x)		(x & (CTL_LT_STRING_ENABLE | CTL_LT_STRING_CHATTY_ENABLE | \
					CTL_LT_BINARY_ENABLE))

#define CTL_LT_STR_IS_ON(x)		(x & CTL_LT_STRING_ENABLE)
#define CTL_LT_STR_IS_PARSING(x)	(x & CTL_LT_STRING_TYPE)
#define CTL_LT_STR_CHATTY_IS_ON(x)	(x & CTL_LT_STRING_CHATTY_ENABLE)
#define CTL_LT_STR_CHATTY_IS_PARSING(x)	(x & CTL_LT_STRING_CHATTY_TYPE)
#define CTL_LT_BIN_IS_ON(x)		(x & CTL_LT_BINARY_ENABLE)

#define CTL_LT_ALL_STR_IS_ON(ring_id, control_logtrace)	\
	(((ring_id == EVENT_LOG_STR_RING_ID) &&	CTL_LT_STR_IS_ON(control_logtrace)) ||	\
	((ring_id == EVENT_LOG_STR_CHATTY_RING_ID) && CTL_LT_STR_CHATTY_IS_ON(control_logtrace)))

#define CTL_LT_ALL_STR_IS_PARSING_ON(ring_id, control_logtrace)	\
	(((ring_id == EVENT_LOG_STR_RING_ID) && CTL_LT_STR_IS_PARSING(control_logtrace)) ||	\
	((ring_id == EVENT_LOG_STR_CHATTY_RING_ID) &&	\
		CTL_LT_STR_CHATTY_IS_PARSING(control_logtrace)))

#define DISABLE_CONTROL_LOGTRACE	(0)
#define ENABLE_CTL_LT_STR_RAW_ENCODED	CTL_LT_STRING_ENABLE
#define ENABLE_CTL_LT_STR_PARSED	(CTL_LT_STRING_ENABLE | CTL_LT_STRING_TYPE)
#define ENABLE_CTL_LT_BINARY		CTL_LT_BINARY_ENABLE

#define DEFAULT_CONTROL_LOGTRACE	DISABLE_CONTROL_LOGTRACE
#ifndef CUSTOM_CONTROL_LOGTRACE
#define CUSTOM_CONTROL_LOGTRACE		DEFAULT_CONTROL_LOGTRACE
#endif

/* --------------------------------------------------------- */

#ifdef EVENT_LOG_NIC
#if defined(__linux__)
	/* (u64)result = (u64)dividend / (u64)divisor */
#define DIV_U64_BY_U64(dividend, divisor)	div64_u64(dividend, divisor)

	/* (u64)result = (u64)dividend / (u32)divisor */
#define DIV_U64_BY_U32(dividend, divisor)	div_u64(dividend, divisor)

	/* Be careful while using this, as it divides dividend also
	 * (u32)remainder = (u64)dividend % (u32)divisor
	 * (u64)dividend = (u64)dividend / (u32)divisor
	 */
#define DIV_AND_MOD_U64_BY_U32(dividend, divisor)	do_div(dividend, divisor)

	/* (u32)remainder = (u64)dividend % (u32)divisor */
#define MOD_U64_BY_U32(dividend, divisor) ({					\
		uint64 temp_dividend = (dividend);				\
		uint32 rem = DIV_AND_MOD_U64_BY_U32(temp_dividend, (divisor));	\
		rem;								\
	})

#define SEC_USEC_FMT \
		"%5llu.%06u"
#else
	/* (u64)result = (u64)dividend / (u64)divisor */
#define DIV_U64_BY_U64(dividend, divisor)	(uint64)(dividend) / (uint64)(divisor)

	/* (u64)result = (u64)dividend / (u32)divisor */
#define DIV_U64_BY_U32(dividend, divisor)	(uint64)(dividend) / (uint32)(divisor)

	/* Be careful while using this, as it divides dividend also
	 * (u32)remainder = (u64)dividend % (u32)divisor
	 * (u64)dividend = (u64)dividend / (u32)divisor
	 */
#define DIV_AND_MOD_U64_BY_U32(dividend, divisor) ({			\
		uint32 rem = (uint64)(dividend) % (uint32)(divisor);	\
		(dividend) = (uint64)(dividend) / (uint32)(divisor);	\
		rem;							\
	})

	/* (u32)remainder = (u64)dividend % (u32)divisor */
#define MOD_U64_BY_U32(dividend, divisor)	(uint32)((uint64)(dividend) % (uint32)(divisor))

#define SEC_USEC_FMT \
		"%015llu.%06u"
#endif /* __linux__ */
#endif /* EVENT_LOG_NIC */

#define TLV_LOG_SIZE(tlv) ((tlv) ? (sizeof(tlv_log) + (tlv)->len) : 0)

#define TLV_LOG_NEXT(tlv) \
	((tlv) ? ((tlv_log *)((uint8 *)tlv + TLV_LOG_SIZE(tlv))) : 0)

#define PTM_FW_TIME_LEN 64u
#define  ENHANCED_TIMESTAMP_V2_MSG_LEN	(sizeof(ets_msg_t) + sizeof(ets_msg_v2_t))

typedef uint32 (*dbg_el_block_parser_fn_t)(void *dbg, char *data,
	uint32 datalen, ring_parser_info_t *el_ring_parser);

/* raw data info of event log parser for logstrs.bin, rodata, pageable rodatastr */
typedef struct {
	char *raw_fmts;		/* buffer for raw formats payload in logstrs.bin */
	uint32 raw_fmts_size;	/* size of raw formats payload in logstrs.bin */
	uint  num_fmts;		/* number of formats in logstrs.bin */
	uint32 fmts_size;	/* size of pointer array: num_fmts  * sizeof(char *) */
	char **fmts;		/* array pointer to indicate format string based on index */

	uint32 text_start;	/* text_start address */

	char *raw_rodata;	/* buffer for raw read-only data section in rtecdc.bin */
	uint32 rodata_start;	/* rodata_start address */
	uint32 rodata_end;	/* rodata_end address */
	uint32 raw_rodata_size;	/* size of raw read-only data section */

	char *raw_rodatastr;	/* buffer for raw pageable read-only string data section */
	uint32 rodatastr_size;	/* size of raw pageable read-only string data section */
	uint32 rodatastr_start;	/* pageable_rodatastr_start address */
	uint32 rodatastr_end;	/* pageable_rodatastr_end address */

	char *str_buf;		/* buffer to store str data of arg in rodata during parsing */
	uint32 str_buf_size;	/* total buf size for str_buf */
	char *fmted_buf;	/* buffer to make formated string */
	uint32 fmted_buf_size;	/* total buf size for fmted_buf */
} el_raw_info_t;

typedef struct eldbg_cmn {
	osl_t	*osh;	/* pointer to os handler */
	uint	unit;	/* device instance number */
#ifdef SHOW_LOGTRACE
	/* raw data info of event log parser for logstrs.bin, rodata, pageable rodatastr */
	el_raw_info_t el_raw_info;
#endif /* SHOW_LOGTRACE */
	eldbg_ring_cmn_t dbg_rings[DEBUG_RING_ID_MAX];
#ifdef DBG_PKT_MON
	dhd_dbg_pkt_mon_t pkt_mon;	/* packet monitoring structure */
	void	*pkt_mon_lock;		/* spin lock for packet monitoring */
#endif /* DBG_PKT_MON */
#ifdef DHD_PKT_LOGGING
	struct dhd_pktlog *pktlog;
	char	debug_dump_time_pktlog_str[DEBUG_DUMP_TIME_BUF_LEN];
	bool	pktlog_debug;
#endif /* DHD_PKT_LOGGING */
	uint32	event_log_ts_ver;	/* event log timestamp version being supported */
	uint8	ets_msg[ENHANCED_TIMESTAMP_V2_MSG_LEN];	/* snapshot of the latest ETS V2 message */
	dbg_el_block_parser_fn_t func_parser;	/* function to parse event log block */
} eldbg_cmn_t;

eldbg_cmn_t *dbg_module_attach(void *hdl, void *osh, uint unit, bool isnic,
	dbg_el_block_parser_fn_t paser_fn);
void dbg_module_detach(eldbg_cmn_t *dbg, bool isnic);

int dbg_push_to_ring(eldbg_cmn_t *dbg, int ring_id, eldbg_ring_entry_t *hdr, void *data);
int dbg_push_to_ring_by_tag(eldbg_cmn_t *dbg, int tag,
	eldbg_ring_entry_t *hdr, void *data);

extern void eldbg_ring_proc_create(eldbg_cmn_t *dbg, uint unit, bool isnic);
extern void eldbg_ring_proc_destroy(eldbg_cmn_t *dbg, uint unit, bool isnic);
extern int eldbg_ring_attach(eldbg_cmn_t *dbg, bool isnic);
extern void eldbg_ring_detach(eldbg_cmn_t *dbg);

/* below functions should be removed from dhd_debug.c & dhd_debug.h after porting */
eldbg_ring_cmn_t *dbg_get_ring_from_ring_id(eldbg_cmn_t *dbg, int ring_id);
int dbg_get_str_ring_id_from_set_num(int set_num);
#endif /* _linux_dbg_h_ */
