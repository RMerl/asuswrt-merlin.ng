/*
 * +--------------------------------------------------------------------------+
 * mac_cpudbg.h
 *
 * External Interface for MAC CPU DBG entry transfer and decode, shared
 * between lower MAC (producer) and host driver (consumer).
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: mac_cpudbg.h 831913 2023-10-27 20:17:35Z $
 *
 * vim: set ts=4 noet sw=4 tw=80:
 * -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * +--------------------------------------------------------------------------+
 */
#ifndef __mac_cpudbg_h__
#define __mac_cpudbg_h__

#if defined(BCMDRIVER) && (defined(BCMPCIEDEV) || defined(BCMPCIE) || \
	defined(CPUDBG_XFER))
#include <hndsoc.h>
#include <bcmpcie.h>

/** Section: BCM_CPUDBG PCIE IPC Shared Structure */
/** Preamble's write and read index, must be cacheline aligned */
#define CPUDBG_ALIGNMENT            (64)
#define __cpudbg_aligned            __attribute__ ((aligned (CPUDBG_ALIGNMENT)))

/** CPUDBG HME decode strings info */
typedef struct cpudbg_dbgstrs_info {
	uint32              mlo_dbgstrs_size;      /* MLO debug decode strings size */
	daddr32_t           mlo_dbgstrs_daddr32;   /* device address of MLO debug decode strings */
	uint32              smac_dbgstrs_size;     /* SMAC debug decode strings size */
	daddr32_t           smac_dbgstrs_daddr32;  /* device address of SMAC debug decode strings */
	uint32              psmr_label_size;       /* PSMr debug decode label strings size */
	daddr32_t           psmr_label_daddr32;    /* device address of PSMr debug decode strings */
} cpudbg_dbgstrs_info_t;

/** CPUDBG HME info */
struct cpudbg_hme {
	uint32    wridx;                           /* CPUDBG current WrIdx value */
	uint32    cpudbg_config;                   /* CPUDBG m2mdma setup config */
	haddr64_t cpudbg_dmamem_haddr64;           /* CPUDBG entry m2mdma hostmem */
	cpudbg_dbgstrs_info_t dbgstrs_info;        /* CPUDBG dbg strings info */
	uint32    hwevt_msglevel;                  /* CPUDBG hw event msglevel */
	daddr32_t rdidx_daddr32;                   /* CPUDBG M2M DMA RdIdx reg addr */
} __cpudbg_aligned;
typedef struct cpudbg_hme cpudbg_hme_t;

#define PSM_LABEL_INPUT_LINE_LEN	46 /* fixed len, includes \n */
#define PSM_LABEL_MAX			44 /* size to make struct 48 bytes */
typedef struct psm_label {
	uint32 addr;			/* PSM instruction address */
	char   label[PSM_LABEL_MAX];	/* Label name at the address */
} psm_label_t;

/* CPUDBG DMA header dropped entry counts */
typedef struct cpudbg_ctrs {
	uint64 cpu0_entry_ct;
	uint64 cpu1_entry_ct;
	uint64 cpu2_entry_ct;
	uint64 cpu3_entry_ct;
	uint64 cpu0_drop_ct;
	uint64 cpu1_drop_ct;
	uint64 cpu2_drop_ct;
	uint64 cpu3_drop_ct;
} cpudbg_ctrs_t;

#define MAX_HW_EVT	32

static const char *hw_evts[MAX_HW_EVT] = {
	"pktproc_state[0]",
	"pktproc_state[1]",
	"pktproc_state[2]",
	"pktproc_state[3]",
	"pktproc_state[4]",
	"pktproc_state[5]",
	"TXOPP_ACT",
	"TXOPS_ACT",
	"TXE_UFLO",
	"TX_PST",
	"TX_CRS",
	"RXSTART_TIMEOUT",
	"PHY_RXSTAT_VAL",
	"RX_START",
	"RX_FRAME",
	"RX_FRAG",
	"RSP_TMT",
	"PSMTXD_RDY",
	"PRETBTT",
	"PIFS",
	"PHYCRS/sec",
	"PHYCRS/pri",
	"PHY_TXERR",
	"NAV_ACT",
	"IPC_INT[3:0]",
	"ED/sec",
	"ED/pri",
	"CMNINFO_PHYSTS_VLD",
	"BFERPT_RDY",
	"BFMRPTXFER_DN",
	"BFERPTSTATS_RDY",
	"BTCX_RF_ACT"
};

#define MAX_PKTPROC_ST	31

static const char *pktproc_states[MAX_PKTPROC_ST] = {
	"PKT_RESET",
	"CARRIER_SEARCH",
	"WAIT_FOR_NB_PWR",
	"WAIT_FOR_W1_PWR",
	"WAIT_FOR_W2_PWR",
	"OFDM_PHY",
	"TIMING_SEARCH",
	"CHAN_EST_1",
	"LEG_SIG_DECODE",
	"SIG_DECODE_1",
	"SIG_DECODE_2",
	"HT_AGC",
	"CHAN_EST_2",
	"PAY_DECODE",
	"DSSS_CCK_PHY",
	"WAIT_ENERGY_DROP",
	"WAIT_NCLKS",
	"TXFRAME",
	"SAMPLE",
	"RIFS_CRS_SEARCH",
	"BOARD_SWITCH_DIV_SEARCH",
	"DSSS_CCK_BOARD_SWITCH_DIV_SEARCH",
	"UNDEF_22",
	"UNDEF_23",
	"FINE_TIMING_SEARCH",
	"SET_CLIP_GAIN",
	"NAPPING",
	"UNDEF_27",
	"VHTSIGB",
	"PKT_ABORT",
	"PAY_DEC_EXT"
};

static uint32 prev_hw_evts = 0;

/*
 * Return the PSMr label matching the CPU DBG entry or the
 * label immediately prior. The label addresses start at 0
 * (INT_PROC), and the last label is UCODE_END, after all code, so
 * it is not valid to have an addr < INT_PROC || addr > UCODE_END.
 */
static inline char *
cpudbg_nearest_psmr_label(psm_label_t *labels, uint num_labels, uint32 addr)
{
	uint i;

	for (i = 0; i < num_labels; ++i) {
		if (labels[i].addr > addr) {
			/* Sanity: the first PSMr label is at addr 0 */
			if (i) {
				return labels[i - 1].label;
			}
		}
		if (labels[i].addr == addr) {
			return labels[i].label;
		}
	}

	return NULL;
}

#define DBGLOG_BUFSIZE 256
#define HWEVTS_BUFSIZE 128

static inline void
cpudbg_dbglog_decode(char *outbuf, uint outbuflen, wl_cpudbg_entry_t *cpudbg_entry,
	const char *str_p, const char *tag, const char *pfx, uint32 hwevt_mask)
{
	char dbglogfmt[DBGLOG_BUFSIZE];
	char hw_evt_buf[HWEVTS_BUFSIZE];
	const char *fmt, *main_fmt, *label = NULL;
	uint32 param_ct = 0, i, extra_uint, param3 = 0, param4 = 0;
	char *cpu_str = NULL, *trace_str = NULL;
	size_t cpysize;
	dbgstr_id_t stringId = cpudbg_entry->stringId;
	uint32 param1 = cpudbg_entry->param1;
	uint32 param2 = cpudbg_entry->param2;
	uint32 ts = cpudbg_entry->ts;
	uint8 cpu_id = stringId.cpu_id;

	outbuf[0] = '\0';
	if (cpu_id == CPUC_ID_PHY) {
		cpu_str = "PHY";
		trace_str = "TRACE";
		extra_uint = stringId.offset;	/* For PHY it is the pc */
		fmt = "0x%x 0x%x";
	} else if (cpu_id == CPUC_ID_HWEVT) {
		uint32 pktproc_state, evt_bit;
		uint32 prev_evts = prev_hw_evts;
		int ret, n = 0;

		cpu_str = "HW";
		trace_str = "EVENT";
		prev_hw_evts = extra_uint = (param1 & hwevt_mask);
		pktproc_state = extra_uint & PKTPROC_ST_MASK;
		if (pktproc_state >= MAX_PKTPROC_ST) {
			snprintf(outbuf, outbuflen - 1, "%s: HWEVT PKTPROC_STATE %u out of range\n",
				__FUNCTION__, pktproc_state);
		    return;
		}

		fmt = &hw_evt_buf[0];
		if (pktproc_state != (prev_evts & PKTPROC_ST_MASK)) {
			n = snprintf(hw_evt_buf, HWEVTS_BUFSIZE - 1, "pktproc_state->%s ",
				pktproc_states[pktproc_state]);
			if (n < 0) {
				snprintf(outbuf, outbuflen - 1,
					"%s: pktproc_state decode snprintf error\n",
					__FUNCTION__);
				return;
			}
		}
		for (i = HWEVT_SIG_START; i <= HWEVT_SIG_END; ++i) {
			evt_bit = ((extra_uint >> i) & 0x1);
			if (evt_bit != ((prev_evts >> i) & 0x1)) {
				if ((HWEVTS_BUFSIZE - n - 1) < 0) {
					snprintf(outbuf, outbuflen - 1, "%s: HWEVT buf too small\n",
						__FUNCTION__);
					return;
				}
				ret = snprintf(hw_evt_buf + n, HWEVTS_BUFSIZE - n - 1, "%s->%u ",
					hw_evts[i], evt_bit);
				if (ret < 0) {
					snprintf(outbuf, outbuflen - 1,
						"%s: hw_evt decode snprintf error\n",
						__FUNCTION__);
					return;
				}
				n += ret;
			}
		}
	} else if (cpu_id > CPUC_ID_MLO) {
		bool utrace = FALSE;
		extra_uint = stringId.offset;	/* For PSMR0/1 it is the pc */

		label = str_p;
		if (extra_uint & UTRACE_FLAG_BIT) {
			utrace = TRUE;
			extra_uint &= ~UTRACE_FLAG_BIT;
		}
		if (cpu_id == CPUC_ID_PSMR0) {
			cpu_str = "PSMR0";
			if (utrace) {
				trace_str = "UTRACE";
			} else {
				trace_str = "TRACE";
			}
		} if (cpu_id == CPUC_ID_PSMR1) {
			cpu_str = "PSMR1";
			if (utrace) {
				trace_str = "UTRACE";
			} else {
				trace_str = "TRACE";
			}
		}

		if (utrace) {
			param1 = param1 & 0xffff;
			if (extra_uint & UCODE_FAULT_BIT) {
				fmt = "#### UCODE FAULT:";
				extra_uint &= ~UCODE_FAULT_BIT;
			} else {
				fmt = "%04x";
			}
		} else {
			param4 = (param2 >> 16) & 0xffff;
			param3 = param2 & 0xffff;
			param2 = (param1 >> 16) & 0xffff;
			param1 = param1 & 0xffff;
			fmt = "%04x %04x %04x %04x";
		}
	} else {
		switch (stringId.msglevel) {
		case CPU_DBG_HIGH:
			trace_str = "HIGH";
			break;
		case CPU_DBG_MED:
			trace_str = "MED";
			break;
		case CPU_DBG_INFO:
			trace_str = "INFO";
			break;
		case CPU_DBG_TRACE:
			trace_str = "TRACE";
			break;
		default:
			trace_str = "<unknown>";
			break;
		}
		fmt = str_p;
		for (i = 0; i < strlen(fmt); ++i) {
			/* check for escaped '%' */
			if (fmt[i] == '%') {
				if (fmt[i+1] == '%') {
					++i;
				} else if (fmt[i+1] == 's') {
					/* build should have flagged invalid %s and its variants */
					snprintf(outbuf, outbuflen - 1, "%s *** invalid %%s fmt "
					       "conversion found in %s dbgstrs file: %s\n",
					       (*pfx ? pfx : tag),
					       (cpu_id == CPUC_ID_SMAC ? "SMAC" : "MLO"),
					       fmt);
					return;
				} else {
					param_ct++;
				}
			}
		}
		if (cpu_id == CPUC_ID_SMAC) {
			cpu_str = "SMAC";
		} else {
			cpu_str = "MLO";
		}
		if (param_ct > 2) {
			snprintf(outbuf, outbuflen - 1, "%s *** param count %d exceeds maximum of 2"
				" in %s dbgstrs fmt %s\n", (*pfx ? pfx : tag), param_ct, cpu_str,
				fmt);
			return;
		}
		extra_uint = stringId.u32;
	}

	main_fmt = "%010u %u % 5s %6s: (%08x): ";
	cpysize = strlen(pfx) + strlen(main_fmt) + strlen(fmt) + 2; /* fmt strings + \n\0 */
	if (label) {
		cpysize += strlen(label) + 1; /* label + \t */
	}
	if (cpysize <= DBGLOG_BUFSIZE) {
		size_t n;
#if defined(BCMDONGLEHOST)
		/* can be done more efficiently using strlcpy() on the dongle host */
		n = strlcpy(dbglogfmt, pfx, DBGLOG_BUFSIZE);
		n += strlcpy(dbglogfmt + n, main_fmt, DBGLOG_BUFSIZE - n);
		n += strlcpy(dbglogfmt + n, fmt, DBGLOG_BUFSIZE - n);
		if (label) {
			n += strlcpy(dbglogfmt + n, "\t", DBGLOG_BUFSIZE - n);
			n += strlcpy(dbglogfmt + n, label, DBGLOG_BUFSIZE - n);
		}
#else
		strncpy(dbglogfmt, pfx, DBGLOG_BUFSIZE - 1);
		n = strlen(dbglogfmt);
		strncpy(dbglogfmt + n, main_fmt, DBGLOG_BUFSIZE - n);
		n += strlen(dbglogfmt + n);
		strncpy(dbglogfmt + n, fmt, DBGLOG_BUFSIZE - n);
		n += strlen(dbglogfmt + n);
		if (label) {
			strncpy(dbglogfmt + n, "\t", DBGLOG_BUFSIZE - n);
			n += strlen(dbglogfmt + n);
			strncpy(dbglogfmt + n, label, DBGLOG_BUFSIZE - n);
			n += strlen(dbglogfmt + n);
		}
#endif /* defined(BCMDONGLEHOST) */
		strncpy(dbglogfmt + n, "\n", DBGLOG_BUFSIZE - n);
		snprintf(outbuf, outbuflen - 1, dbglogfmt, ts, stringId.link_id,
			cpu_str, trace_str, extra_uint, param1, param2, param3, param4);
	} else {
		snprintf(outbuf, outbuflen - 1, "%s buffer size %u too big for dbglog buf "
			"size %u\n", (*pfx ? pfx : tag), (uint32)cpysize, outbuflen);
	}
}
#endif /* BCMDRIVER && (BCMPCIEDEV || BCMPCIE  || CPUDBG_XFER) */

#endif /* __mac_cpudbg_h__ */
