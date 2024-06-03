/* D11 macdbg functions for Broadcom 802.11abgn
 * Networking Adapter Device Drivers.
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
 * $Id: dhd_macdbg.c 832015 2023-10-31 18:21:56Z $
 */

#include <typedefs.h>
#include <osl.h>

#include <bcmutils.h>
#include <bcmendian.h>
#include <dhd_dbg.h>
#include <dhd_macdbg.h>
#include "d11reglist_proto.h"
#include "dhdioctl.h"
#include <sdiovar.h>
#include <dhd_linux.h>

#ifdef BCMPCIE
/**
 * For dump/debug purpose: definitions of certain d11 core registers
 */
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <sbchipc.h>
#include <hndpmu.h>
#include <hndsoc.h>
#include <bcmendian.h>
#include <dngl_stats.h>
#include <bcmdevs.h>
#include <dhd.h>
#include <dhd_bus.h>
#include <dhd_proto.h>
#include <pcicfg.h>
#endif /* BCMPCIE */

#define MACDBG_RLM_MAX_ENTRIES		256
#define MACDBG_RLM_BCMC_ENTRY		63
#define MACDBG_RLM_VBSS_START		236
#define MACDBG_RLM_VBSS_END		251
#define MACDBG_RLM_TWT_START		252
#define MACDBG_RLM_TWT_END		255
#define MACDBG_RLM_ULOFDMA_START	218
#define MACDBG_RLM_ULOFDMA_END		249
#define MACDBG_RLM_DLOFDMA_START	250
#define MACDBG_RLM_DLOFDMA_END		255

#define MACREG_PHY_REG_ADDR		0x3FC
#define MACREG_PHY_REG_DATA		0x3FE
#define PHYREG_TABLE_ID			0xD
#define PHYREG_TABLE_OFFSET		0xE
#define PHYREG_TABLE_DATA_LO		0xF
#define PHYREG_TABLE_DATA_HI		0x10
#define PHYREG_TABLE_DATA_WIDE		0x11
#define MACREG_PSM_RATEMEM_DBG(_d11rev)	((_d11rev) >= 132 ? 0x7f8 : 0x872)

typedef struct _macdbg_info_t {
	dhd_pub_t *dhdp;
	uint8 *dumpbuf, *dumpbufcurp;
	int dumpbuflen;
	uint8 *pd11regxtlv;
	uint16 d11regxtlv_bsz;
	uint32 corerev;
	uint32 phyrev;
	int rlm_max_index;
	const char *active_dumpname;
	sdreg_t last_access;
	uint32 si_enum_base;
	uint32 max_dumpnamelen;
	bool aer_enab;
} macdbg_info_t;

static int dhd_macdbg_dumppsm(dhd_pub_t *dhdp, char *buf, int buflen,
	int *outbuflen, uint16 xtlv_type);
static int dhd_macdbg_dumpsampcol(dhd_pub_t *dhdp, char *buf, int buflen,
	int *outbuflen, uint8 sctype);
static int dhd_macdbg_dumpphysvmp(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen);
static int dhd_macdbg_dumpphy(macdbg_info_t *mi, struct bcmstrbuf *b);
static int dhd_macdbg_dumpsvmp(macdbg_info_t *mi, struct bcmstrbuf *b);
static int dhd_macdbg_dumpratelinkmem(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen);
static int dhd_macdbg_dumpbmc(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen);
static int dhd_macdbg_dumpperuser(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen);
static int dhd_macdbg_dumpaqmsts(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen);
static int dhd_macdbg_dumpcpudbg(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen);
static int dhd_macdbg_dumpcpudbgfifo(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen);
static int dhd_macdbg_dumpcpudbgstate(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen);

/* Sample capture related definitions */
#define UCREVMAJOR_OFFSET	0x0
#define UCREVMINOR_OFFSET	0x2
#define UCFEATURES_OFFSET	0xa

#define	PHY_TYPE_AC		11
#define	PHY_TYPE_AX		13

#define MCTL1_OFFSET		0x1a0

#define DUMP_SC_TYPE_TPL	0
#define DUMP_SC_TYPE_UTSHM	1
#define DUMP_SC_TYPE_UTSHMX	2
#define DUMP_SC_TYPE_PSM	3

typedef struct _d11scregs_addr_t {
	uint16 ctrl;
	uint16 ctrl2;
	uint16 start_ptr;
	uint16 stop_ptr;
	uint16 cur_ptr;
	uint16 wr_ptr;
	uint16 wr_data;
} d11scregs_addr_t;

/* per user dump */
#define MPDUS_DUMP_NUM		0x8
#define MAX_NUSR(_d11rev) \
		(((_d11rev) == 129 || (_d11rev) == 132 || \
		 ((_d11rev) == 133) || (_d11rev) == 134) ? 16 : \
		 (((_d11rev) == 130 || (_d11rev) == 131) ? 4 : 0))
#define NUM_TXDBG(corerev)	((corerev < 133) ? (9) : ((corerev == 133) ? (11) : (47)))

#define BMC_TPL_IDX(_d11rev) \
		(((_d11rev) == 133) ? 86 : ((_d11rev == 135) ? 38 :  \
		(((_d11rev) == 129 || (_d11rev) == 132 || ((_d11rev) == 134)) ? 70 : \
		 ((_d11rev) == 130 ? 38 : 22))))

int
dhd_macdbg_attach(dhd_pub_t *dhdp)
{
	macdbg_info_t *mi = MALLOCZ(dhdp->osh, sizeof(*mi));

	if (mi == NULL) {
		return BCME_NOMEM;
	}

	mi->dhdp = dhdp;
	mi->dumpbuf = mi->dumpbufcurp = NULL;
	mi->dumpbuflen = 0;
	mi->rlm_max_index = -1;
	mi->active_dumpname = NULL;
#ifdef LINUX
	mi->aer_enab = IS_ENABLED(CONFIG_ACPI_APEI_PCIEAER);
#endif
	dhdp->macdbg_info = mi;

	/* Record length of longest dump name string */
#define MACDBGDUMP_ENUMDEF(_id, _str) \
	if (strlen(#_str) > mi->max_dumpnamelen) { \
		mi->max_dumpnamelen = strlen(#_str); \
	}

	MACDBGDUMP_ENUMDEF_LIST

#undef MACDBGDUMP_ENUMDEF

	return BCME_OK;
}

void
dhd_macdbg_detach(dhd_pub_t *dhdp)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	ASSERT(mi);

	dhdp->macdbg_info = NULL;
	if (mi->pd11regxtlv) {
		ASSERT(mi->d11regxtlv_bsz > 0);
		MFREE(dhdp->osh, mi->pd11regxtlv, mi->d11regxtlv_bsz);
		mi->d11regxtlv_bsz = 0;
	}
	if (mi->dumpbuf) {
		MFREE(dhdp->osh, mi->dumpbuf, DUMPMAC_BUF_SZ);
		mi->dumpbuf = mi->dumpbufcurp = NULL;
	}

	MFREE(dhdp->osh, mi, sizeof(*mi));
}

static void
dhd_macdbg_rlm_event_handler(dhd_pub_t *dhdp, void *data, int data_len)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	wlc_rlm_event_t *rlm_evdata;
	uint16 rlm_index, version;

	if (mi->corerev < 128)
		return;

	if (data_len < sizeof(wlc_rlm_event_t)) {
		DHD_ERROR(("%s: invalid length\n", __FUNCTION__));
		return;
	}

	rlm_evdata = data;
	version = ltoh16(rlm_evdata->version);

	if (version != WLC_E_MACDBG_RLM_VERSION) {
		DHD_ERROR(("%s: invalid version: %u != %u\n", __FUNCTION__, version,
			WLC_E_MACDBG_RLM_VERSION));
		return;
	}

	rlm_index = ltoh16(rlm_evdata->entry);

	/* ignore invalid/special index */
	if (rlm_index >= MACDBG_RLM_MAX_ENTRIES || rlm_index == MACDBG_RLM_BCMC_ENTRY)
		return;

	if ((int)rlm_index > mi->rlm_max_index)
		mi->rlm_max_index = rlm_index;
}

static int
dhd_macdbg_dumpmac(dhd_pub_t *dhdp, char *buf, int buflen, int *remain_len)
{
	return dhd_macdbg_dumppsm(dhdp, buf, buflen, NULL, D11REG_XTLV_PSMR);
}

static int
dhd_macdbg_dumpmac1(dhd_pub_t *dhdp, char *buf, int buflen, int *remain_len)
{
	return dhd_macdbg_dumppsm(dhdp, buf, buflen, NULL, D11REG_XTLV_PSMR1);
}

static int
dhd_macdbg_dumpmacx(dhd_pub_t *dhdp, char *buf, int buflen, int *remain_len)
{
	return dhd_macdbg_dumppsm(dhdp, buf, buflen, NULL, D11REG_XTLV_PSMX);
}

static int
dhd_macdbg_dumplmac(dhd_pub_t *dhdp, char *buf, int buflen, int *remain_len)
{
	return dhd_macdbg_dumppsm(dhdp, buf, buflen, NULL, D11REG_XTLV_LMAC);
}

static int
dhd_macdbg_dumpphydtcm(dhd_pub_t *dhdp, char *buf, int buflen, int *remain_len)
{
	return dhd_macdbg_dumppsm(dhdp, buf, buflen, NULL, D11REG_XTLV_PHYDTCM);
}

static int
dhd_macdbg_dumpsctpl(dhd_pub_t *dhdp, char *buf, int buflen, int *remain_len)
{
	return dhd_macdbg_dumpsampcol(dhdp, buf, buflen, NULL, DUMP_SC_TYPE_TPL);
}

static int
dhd_macdbg_dumpscpsm(dhd_pub_t *dhdp, char *buf, int buflen, int *remain_len)
{
	return dhd_macdbg_dumpsampcol(dhdp, buf, buflen, NULL, DUMP_SC_TYPE_PSM);
}

void
dhd_macdbg_event_handler(dhd_pub_t *dhdp, uint32 reason,
		uint8 *event_data, uint32 datalen)
{
	macdbg_info_t *mi = dhdp->macdbg_info;

	DHD_TRACE(("%s: reason %d datalen %d\n", __FUNCTION__, reason, datalen));
	switch (reason) {
		case WLC_E_MACDBG_DUMPALL:
#ifdef LINUX
			DHD_ERROR(("%s: Schedule trap log dump\n", __FUNCTION__));
			/* Schedule to work queue as this context could be ISR */
			dhd_schedule_trap_log_dump(dhdp, FWTRAP_DUMP_FULL);
#else
			/*
			 * No need to update this for new dumps. The pre-processor
			 * expansion will produce code as defined by MACDBGDUMP_ENUMDEF
			 * for each dump as listed in MACDBGDUMP_ENUMDEF_LIST.
			 *
			 * MACDBGDUMP_ENUMDEF is defined below to produce the dump call.
			 */
			/* call each dump function to dump to console */
#define MACDBGDUMP_ENUMDEF(_id, _str) \
			(void) dhd_macdbg_dump ## _str(dhdp, NULL, 0, NULL);

			MACDBGDUMP_ENUMDEF_LIST

#undef MACDBGDUMP_ENUMDEF

#endif /* LINUX */
			break;
		case WLC_E_MACDBG_LISTALL:
			if (mi->pd11regxtlv == NULL && mi->d11regxtlv_bsz == 0) {
				mi->pd11regxtlv = MALLOCZ(dhdp->osh, datalen);
				if (mi->pd11regxtlv == NULL) {
					DHD_ERROR(("%s: NOMEM for len %d\n",
						__FUNCTION__, datalen));
					return;
				}
				mi->d11regxtlv_bsz = datalen;
				memcpy(mi->pd11regxtlv, event_data, datalen);
				DHD_TRACE(("%s: d11regxtlv_bsz %d\n", __FUNCTION__,
					mi->d11regxtlv_bsz));
			}
			break;
		case WLC_E_MACDBG_RATELINKMEM:
			dhd_macdbg_rlm_event_handler(dhdp, event_data, datalen);
			break;
		case WLC_E_MACDBG_DTRACE:
			dhd_schedule_dtrace(dhdp, event_data, datalen);
			break;
		default:
			DHD_ERROR(("%s: Unknown reason %d\n",
				__FUNCTION__, reason));
	}
	return;
}

void
dhd_macdbg_upd_revinfo(dhd_pub_t *dhdp, wlc_rev_info_t *revinfo)
{
	uint32 val;

	macdbg_info_t *mi = dhdp->macdbg_info;
	if (mi == NULL) {
		DHD_ERROR(("%s: mi is NULL!\n", __FUNCTION__));
		return;
	}
	mi->corerev = revinfo->corerev;

	if (revinfo->phytype == PHY_TYPE_AC ||
		revinfo->phytype == PHY_TYPE_AX) {
		mi->phyrev = revinfo->phyrev;
	}
	if (BUS_IOVAR_OP(dhdp, "si_enum_base", NULL, 0, &val,
			sizeof(val), IOV_GET) == BCME_OK) {
		mi->si_enum_base = val;
	}
}

int
dhd_macdbg_dump(dhd_pub_t *dhdp, char *buf, int buflen, const char *name)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	int res = BCME_BADARG;

	if (!dhd_d11_iscoreup(dhdp)) {
		DHD_ERROR(("%s: Cannot do registers dump due to d11 core is down\n", __FUNCTION__));
		res = BCME_NOTUP;
		goto exit;
	}

	mi->active_dumpname = name;

	/*
	 * No need to update this for new dumps. The pre-processor expansion
	 * will produce code as defined by MACDBGDUMP_ENUMDEF for each dump
	 * as listed in MACDBGDUMP_ENUMDEF_LIST.
	 *
	 * MACDBGDUMP_ENUMDEF is defined below to produce comparison and\
	 * function call for each dump entry.
	 */
#define MACDBGDUMP_ENUMDEF(_id, _str) \
	if (strncmp(name, #_str, mi->max_dumpnamelen) == 0) {	\
		res = dhd_macdbg_dump ## _str(dhdp, buf, buflen, NULL); \
		goto exit; \
	}

	MACDBGDUMP_ENUMDEF_LIST

#undef MACDBGDUMP_ENUMDEF

exit:
	mi->active_dumpname = NULL;
	return res;
}

int
dhd_macdbg_dumpiov(dhd_pub_t *dhdp, char *params, int plen, char *buf, int buflen)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	char *dumpname = params;
	int copylen;

	if (strncmp("cont", dumpname, 4)) {
		int res;
		if (mi->dumpbuf) {
			DHD_ERROR(("%s: unexpected dangling dumpbuf\n", __FUNCTION__));
		} else {
			mi->dumpbuf = MALLOCZ(dhdp->osh, DUMPMAC_BUF_SZ);
		}
		res = dhd_macdbg_dump(dhdp, mi->dumpbuf, DUMPMAC_BUF_SZ, dumpname);
		if (res != BCME_OK) {
			MFREE(dhdp->osh, mi->dumpbuf, DUMPMAC_BUF_SZ);
			mi->dumpbuf = mi->dumpbufcurp = NULL;
			return res;
		}
		mi->dumpbufcurp = mi->dumpbuf;
		mi->dumpbuflen = (int)strlen(mi->dumpbuf);
	} else if (!mi->dumpbuf) {
		/* if continuous mode and no dumpbuffer to return, return null string */
		memset(buf, 0, buflen);
		return BCME_OK;
	}
	ASSERT(mi->dumpbufcurp != NULL);
	copylen = MIN(buflen, mi->dumpbuflen);
	memcpy(buf, mi->dumpbufcurp, copylen);
	mi->dumpbufcurp += copylen;
	mi->dumpbuflen -= copylen;

	if (mi->dumpbuflen == 0) {
		MFREE(dhdp->osh, mi->dumpbuf, DUMPMAC_BUF_SZ);
		mi->dumpbuf = mi->dumpbufcurp = NULL;
	}
	return BCME_OK;
}

static uint32
_dhd_get_sbreg(macdbg_info_t *mi, uint32 addr, uint32 size)
{
	sdreg_t sdreg;
	uint32 val;

	sdreg.func = size;
	sdreg.offset = addr;
	BUS_IOVAR_OP(mi->dhdp, "sbreg",
		&sdreg, sizeof(sdreg), &val, sizeof(val), IOV_GET);
	return val;
};

static void
_dhd_set_sbreg(macdbg_info_t *mi, uint32 addr, uint32 size, uint32 val)
{
	sdreg_t sdreg;

	sdreg.func = size;
	sdreg.offset = addr;
	sdreg.value = val;
	BUS_IOVAR_OP(mi->dhdp, "sbreg",
		NULL, 0, &sdreg, sizeof(sdreg), IOV_SET);
};

static uint16
_dhd_get_ihr16(macdbg_info_t *mi, uint16 addr, struct bcmstrbuf *b, bool verbose)
{
	uint16 val;

	addr += 0x1000;
	val = _dhd_get_sbreg(mi, addr, 2);
#ifdef BCMDBG
	if (verbose) {
		if (b) {
			bcm_bprintf(b, "DEBUG: IHR16: read 0x%08x, size 2, value 0x%04x\n",
				(addr + mi->si_enum_base), val);
		} else {
			printf("DEBUG: IHR16: read 0x%08x, size 2, value 0x%04x\n",
				(addr + mi->si_enum_base), val);
		}
	}
#else
	BCM_REFERENCE(verbose);
#endif /* BCMDBG */
	return val;
}

static uint32
_dhd_get_ihr32(macdbg_info_t *mi, uint16 addr, struct bcmstrbuf *b, bool verbose)
{
	uint32 val;

	addr += 0x1000;
	val = _dhd_get_sbreg(mi, addr, 4);
#ifdef BCMDBG
	if (verbose) {
		if (b) {
			bcm_bprintf(b, "DEBUG: IHR32: read 0x%08x, size 4, value 0x%08x\n",
				(addr + mi->si_enum_base), val);
		} else {
			printf("DEBUG: IHR32: read 0x%08x, size 4, value 0x%08x\n",
				(addr + mi->si_enum_base), val);
		}
	}
#else
	BCM_REFERENCE(verbose);
#endif /* BCMDBG */
	return val;
}

static void
_dhd_set_ihr16(macdbg_info_t *mi, uint16 addr, uint16 val,
	struct bcmstrbuf *b, bool verbose)
{
	addr += 0x1000;
#ifdef BCMDBG
	if (verbose) {
		if (b) {
			bcm_bprintf(b, "DEBUG: IHR16: write 0x%08x, size 2, value 0x%04x\n",
				(addr + mi->si_enum_base), val);
		} else {
			printf("DEBUG: IHR16: write 0x%08x, size 2, value 0x%04x\n",
				(addr + mi->si_enum_base), val);
		}
	}
#else
	BCM_REFERENCE(verbose);
#endif /* BCMDBG */
	_dhd_set_sbreg(mi, addr, 2, val);
}

static void
_dhd_set_ihr32(macdbg_info_t *mi, uint16 addr, uint32 val,
	struct bcmstrbuf *b, bool verbose)
{
	addr += 0x1000;
#ifdef BCMDBG
	if (verbose) {
		if (b) {
			bcm_bprintf(b, "DEBUG: IHR32: write 0x%08x, size 4, value 0x%08x\n",
				(addr + mi->si_enum_base), val);
		} else {
			printf("DEBUG: IHR32: write 0x%08x, size 4, value 0x%08x\n",
				(addr + mi->si_enum_base), val);
		}
	}
#else
	BCM_REFERENCE(verbose);
#endif /* BCMDBG */
	_dhd_set_sbreg(mi, addr, 4, val);
}

static uint32
_dhd_get_d11obj32(macdbg_info_t *mi, uint16 objaddr, uint32 sel,
	struct bcmstrbuf *b, bool verbose)
{
#ifdef BCMDBG
	const char *sel_str = (sel & 0x00020000) ? "SCR":"SHM";
#endif
	uint32 val, objval;
	sdreg_t sdreg;

	BCM_REFERENCE(verbose);

	sdreg.func = 4; /* 4bytes by default */
	sdreg.offset = 0x1160;

	if (objaddr == 0xffff) {
		/* autoincrement so no need to write indirect address */
		goto objaddr_read;
	}

	if (objaddr & 0x3) {
		printf("%s: ERROR! Invalid addr 0x%x\n", __FUNCTION__, objaddr);
	}

	sdreg.value = (sel | (objaddr >> 2));
#ifdef BCMDBG
	if (verbose) {
		if (b) {
			bcm_bprintf(b, "DEBUG: %s: Indirect: write 0x%08x, size %d, value 0x%08x\n",
				sel_str, (sdreg.offset + mi->si_enum_base),
				sdreg.func, sdreg.value);
		} else {
			printf("DEBUG: %s: Indirect: write 0x%08x, size %d, value 0x%08x\n",
				sel_str, (sdreg.offset + mi->si_enum_base),
				sdreg.func, sdreg.value);
		}
	}
#endif /* BCMDBG */
	BUS_IOVAR_OP(mi->dhdp, "sbreg", NULL, 0, &sdreg, sizeof(sdreg), IOV_SET);

objaddr_read:
	/* Give some time to obj addr register */
	BUS_IOVAR_OP(mi->dhdp, "sbreg", &sdreg, sizeof(sdreg), &objval, sizeof(objval), IOV_GET);
#ifdef BCMDBG
	if (verbose) {
		if (b) {
			bcm_bprintf(b, "DEBUG: %s: Indirect: Read 0x%08x, size %d, value 0x%08x\n",
				sel_str, (sdreg.offset + mi->si_enum_base), sdreg.func, objval);
		} else {
			printf("DEBUG: %s: Indirect: Read 0x%08x, size %d, value 0x%08x\n",
				sel_str, (sdreg.offset + mi->si_enum_base), sdreg.func, objval);
		}
	}
#endif /* BCMDBG */

	/* read indirect register */
	sdreg.offset = 0x1164;
	BUS_IOVAR_OP(mi->dhdp, "sbreg", &sdreg, sizeof(sdreg), &val, sizeof(val), IOV_GET);

#ifdef BCMDBG
	if (verbose) {
		if (b) {
			bcm_bprintf(b, "DEBUG: %s: Indirect: Read 0x%08x, size %d, value 0x%04x\n",
				sel_str, (sdreg.offset + mi->si_enum_base), sdreg.func, val);
		} else {
			printf("DEBUG: %s: Indirect: Read 0x%08x, size %d, value 0x%04x\n",
				sel_str, (sdreg.offset + mi->si_enum_base), sdreg.func, val);
		}
	}
#endif /* BCMDBG */
	return val;
}

static uint16
_dhd_get_d11obj16(macdbg_info_t *mi, uint16 objaddr,
	uint32 sel, d11obj_cache_t *obj_cache, struct bcmstrbuf *b, bool verbose)
{
	uint32 val;
	if (obj_cache && obj_cache->cache_valid && ((obj_cache->sel ^ sel) & (0xffffff)) == 0) {
		if (obj_cache->addr32 == (objaddr & ~0x3)) {
#ifdef BCMDBG
			if (verbose) {
				if (b) {
					bcm_bprintf(b, "DEBUG: %s: Read cache value: "
						"addr32 0x%04x, sel 0x%08x, value 0x%08x\n",
						(sel & 0x00020000) ? "SCR":"SHM",
						obj_cache->addr32, obj_cache->sel, obj_cache->val);
				} else {
					printf("DEBUG: %s: Read cache value: "
						"addr32 0x%04x, sel 0x%08x, value 0x%08x\n",
						(sel & 0x00020000) ? "SCR":"SHM",
						obj_cache->addr32, obj_cache->sel, obj_cache->val);
				}
			}
#else
			BCM_REFERENCE(verbose);
#endif /* BCMDBG */
			val = obj_cache->val;
			goto exit;
		} else if ((obj_cache->sel & 0x02000000) &&
			(obj_cache->addr32 + 4 == (objaddr & ~0x3))) {
#ifdef BCMDBG
			if (verbose) {
				if (b) {
					bcm_bprintf(b, "DEBUG: %s: Read objdata only: "
						"addr32 0x%04x, sel 0x%08x, value 0x%08x\n",
						(sel & 0x00020000) ? "SCR":"SHM",
						obj_cache->addr32, obj_cache->sel, obj_cache->val);
				} else {
					printf("DEBUG: %s: Read objdata only: "
						"addr32 0x%04x, sel 0x%08x, value 0x%08x\n",
						(sel & 0x00020000) ? "SCR":"SHM",
						obj_cache->addr32, obj_cache->sel, obj_cache->val);
				}
			}
#endif /* BCMDBG */
			val = _dhd_get_d11obj32(mi, 0xffff, sel, b, verbose);
			goto exit;
		}
	}
	val = _dhd_get_d11obj32(mi, (objaddr & ~0x2), sel, b, verbose);
exit:
	if (obj_cache) {
		obj_cache->addr32 = (objaddr & ~0x3);
		obj_cache->sel = sel;
		obj_cache->val = val;
		obj_cache->cache_valid = TRUE;
	}
	return (uint16)((objaddr & 0x2) ? (val >> 16) : val);
}

static uint16
_dhd_get_cm7reg(macdbg_info_t *mi, uint16 addr, uint16 axi_addr, struct bcmstrbuf *b,
	uint32 size, bool verbose)
{
	uint16 val;

	val = _dhd_get_sbreg(mi, (axi_addr << 16 | addr), size);
	BCM_REFERENCE(verbose);
	return val;
}

static uint32
_dhd_get_cm7reg32(macdbg_info_t *mi, uint16 addr, uint16 axi_addr, struct bcmstrbuf *b,
	uint32 size, bool verbose)
{
	uint32 val32;

	val32 = _dhd_get_sbreg(mi, (axi_addr << 16 | addr), size);
	BCM_REFERENCE(verbose);
	return val32;
}

static const char *regname[D11REG_TYPE_MAX] = D11REGTYPENAME;

static void dhd_audit_d11reg_value(macdbg_info_t *mi, int type, uint16 addr, uint32 val)
{
	if (mi->aer_enab && val == 0xffffffff)
	{
		printf("wl%d: FAILED: dump <%s> access <%s:0x%04x>\n", mi->dhdp->unit,
			mi->active_dumpname, regname[type], addr);
	}
}

static int
_dhd_print_d11reg(macdbg_info_t *mi, int idx, int type, uint16 addr, struct bcmstrbuf *b,
	d11obj_cache_t *obj_cache, bool verbose)
{
	uint32 val;

	/* store access */
	mi->last_access.func = type;
	mi->last_access.offset = addr;

	if (type == D11REG_TYPE_IHR32) {
		if ((addr & 0x3)) {
			printf("%s: ERROR! Invalid addr 0x%x\n", __FUNCTION__, addr);
			addr &= ~0x3;
		}
		val = _dhd_get_ihr32(mi, addr, b, verbose);
		if (b) {
			bcm_bprintf(b, "%-3d %s 0x%-4x = 0x%-8x\n",
				idx, regname[type], addr, val);
		} else {
			printf("%-3d %s 0x%-4x = 0x%-8x\n",
				idx, regname[type], addr, val);
		}
	} else {
		switch (type) {
		case D11REG_TYPE_IHR16: {
			if ((addr & 0x1)) {
				printf("%s: ERROR! Invalid addr 0x%x\n", __FUNCTION__, addr);
				addr &= ~0x1;
			}
			val = _dhd_get_ihr16(mi, addr, b, verbose);
			break;
		}
		case D11REG_TYPE_IHRX16:
			// TODO for rev133
			val = _dhd_get_d11obj16(mi, (addr - 0x400) << 1, 0x020b0000,
				obj_cache, b, verbose);
			break;
		case D11REG_TYPE_IHR116:
			val = _dhd_get_d11obj16(mi, (addr - 0x400) << 1, 0x02130000,
				obj_cache, b, verbose);
			break;
		case D11REG_TYPE_SCR:
			val = _dhd_get_d11obj16(mi, addr << 2, 0x02020000,
				obj_cache, b, verbose);
			break;
		case D11REG_TYPE_SCR1:
			val = _dhd_get_d11obj16(mi, addr << 2, 0x02120000,
				obj_cache, b, verbose);
			break;
		case D11REG_TYPE_SCRX:
			val = _dhd_get_d11obj16(mi, addr << 2, 0x020a0000,
				obj_cache, b, verbose);
			break;
		case D11REG_TYPE_SHM:
			val = _dhd_get_d11obj16(mi, addr, 0x02010000,
				obj_cache, b, verbose);
			break;
		case D11REG_TYPE_SHMX:
			val = _dhd_get_d11obj16(mi, addr, 0x02090000,
				obj_cache, b, verbose);
			break;
		case D11REG_TYPE_SHM1:
			val = _dhd_get_d11obj16(mi, addr, 0x02110000,
				obj_cache, b, verbose);
			break;
		case D11REG_TYPE_IHR16CM7:
			val = _dhd_get_cm7reg(mi, (addr - 0x400), 0x3a0, b, 2, verbose);
			break;
		case D11REG_TYPE_SHMCM7:
			val = _dhd_get_cm7reg(mi, addr, 0x250, b, 2, verbose);
			break;
		case D11REG_TYPE_IPC:
			val = _dhd_get_cm7reg(mi, addr, 0x241, b, 4, verbose);
			break;
		case D11REG_TYPE_ICC:
			val = _dhd_get_cm7reg(mi, addr, 0x2b0, b, 4, verbose);
			break;
		case D11REG_TYPE_MLOAMGR:
			val = _dhd_get_cm7reg(mi, addr, 0x280, b, 4, verbose);
			break;
		case D11REG_TYPE_DTCMCM7:
			val = _dhd_get_cm7reg(mi, addr, 0x20c, b, 4, verbose);
			break;
		case D11REG_TYPE_DTCMCM4:
			val = _dhd_get_cm7reg(mi, addr, 0x214, b, 4, verbose);
			break;
		default:
			printf("Unrecognized type %d!\n", type);
			return 0;
		}

		if (type == D11REG_TYPE_DTCMCM7 || type == D11REG_TYPE_DTCMCM4) {
		    if (b) {
				bcm_bprintf(b, "%-4x\n", val);
		    } else {
				printf("%-4x\n", val);
		    }
		} else {
		    if (b) {
				bcm_bprintf(b, "%-3d %s 0x%-4x = 0x%-4x\n",
				idx, regname[type], addr, val);
		    } else {
				printf("%-3d %s 0x%-4x = 0x%-4x\n",
				idx, regname[type], addr, val);
		    }
		}
	}

	dhd_audit_d11reg_value(mi, type, addr, val);

	/* success; clear offset */
	mi->last_access.offset = 0;

	return 1;
}

static int
_dhd_print_d11regs(macdbg_info_t *mi, d11regs_list_t *pregs,
	int start_idx, struct bcmstrbuf *b, bool verbose)
{
	int idx = 0;
	d11obj_cache_t obj_cache = {0, 0, 0, FALSE};
	uint16 byte_offset = pregs->byte_offset;
	uint32 bitmap = pregs->bitmap;

	if (pregs->type >= D11REG_TYPE_MAX) {
		printf("%s: wrong type %d\n", __FUNCTION__, pregs->type);
		return 0;
	}

	if (mi->corerev >= 133) {
		/* switch psmx types to cm7 types */
		if (pregs->type == D11REG_TYPE_IHRX16) {
			pregs->type = D11REG_TYPE_IHR16CM7;
		} else if (pregs->type == D11REG_TYPE_SHMX) {
			pregs->type = D11REG_TYPE_SHMCM7;
		}
	}

	if (bitmap) {
		while (bitmap) {
			if (bitmap && (bitmap & 0x1)) {
				_dhd_print_d11reg(mi, (idx + start_idx), pregs->type,
					byte_offset, b, &obj_cache, verbose);
				idx++;
			}
			bitmap = bitmap >> 1;
			byte_offset += pregs->step;
		}
	} else {
		/* skip these since they are written to .bin files */
		if (pregs->type == D11REG_TYPE_DTCMCM7 || pregs->type == D11REG_TYPE_DTCMCM4 ||
			pregs->type == D11REG_TYPE_SHMCM7 || pregs->type == D11REG_TYPE_MLO_AMGR ||
			pregs->type == D11REG_TYPE_ICC_OUT) {
			/* ensure idx is non-zero so dump cmd does not error out */
			++idx;
		} else {
			for (; idx < pregs->cnt; idx++) {
				_dhd_print_d11reg(mi, (idx + start_idx), pregs->type,
					byte_offset, b, &obj_cache, verbose);
				byte_offset += pregs->step;
			}
		}
	}
	return idx;
}

static int
_dhd_print_phydtcm(macdbg_info_t *mi, d11regs_list_t *pregs,
	int start_idx, struct bcmstrbuf *b, bool verbose)
{
	int idx, k;
	uint16 byte_offset = pregs->byte_offset;
	uint16 byte_size = pregs->step;
	uint32 val32;

	if (b) {
		bcm_bprintf(b, "phycm7_dtcm size=0x%x*%d\n", pregs->cnt, pregs->step);
		bcm_bprintf(b, "byte_addr: val32[8n+0] ... val32[8n+7]\n");
	} else {
		printf("phycm7 dtcm\n");
	}
	DHD_ERROR(("%s: start_idx=%d, type=%d, offset=%d, cnt=%d, step=%d\n",
		__FUNCTION__, start_idx,
		pregs->type, pregs->byte_offset, pregs->cnt, pregs->step));

	//for (; idx < pregs->cnt; idx++) {
	//	_dhd_print_d11reg(mi, (idx + start_idx), pregs->type,
	//		byte_offset, b, &obj_cache, verbose);
	//	byte_offset += pregs->step;
	//}

	for (idx = 0; idx < pregs->cnt; idx += 8) {
		if (b) {
			bcm_bprintf(b, "0x%04x:", byte_offset);
		} else {
			printf("0x%04x:", byte_offset);
		}
		for (k = 0; k < 8; k++) {
			//val32 = _dhd_get_sbreg(mi, ((0x208 << 16) | byte_offset), byte_size);
			//BCM_REFERENCE(verbose);
			val32 = _dhd_get_cm7reg32(mi, byte_offset, 0x208, b, byte_size, verbose);
			if (b) {
				bcm_bprintf(b, " %08x", val32);
			} else {
				printf(" %08x", val32);
			}
			byte_offset += byte_size;
		}
		if (b) {
			bcm_bprintf(b, "\n");
		} else {
			printf("\n");
		}
	}

	return idx;
}

static int
_dhd_pd11regs_bylist(macdbg_info_t *mi, d11regs_list_t *reglist,
	uint16 reglist_sz, struct bcmstrbuf *b)
{
	uint i, idx = 0;

	if (reglist != NULL && reglist_sz > 0) {
		for (i = 0; i < reglist_sz; i++) {
			DHD_TRACE(("%s type %d byte_offset 0x%x bmp 0x%x step %d cnt %d\n",
				__FUNCTION__, reglist[i].type, reglist[i].byte_offset,
				reglist[i].bitmap, reglist[i].step, reglist[i].cnt));
			if (reglist[i].type == D11REG_TYPE_DTCMPHY) {
				idx += _dhd_print_phydtcm(mi, &reglist[i], idx, b, FALSE);
			} else {
				idx += _dhd_print_d11regs(mi, &reglist[i], idx, b, FALSE);
			}
		}
	}

	return idx;
}

static int
dhd_macdbg_dumppsm(dhd_pub_t *dhdp, char *buf, int buflen,
	int *outbuflen, uint16 xtlv_type)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	d11regs_list_t *reglist = NULL;
	uint16 reglist_sz, cnt = 0;
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;

	if (buf && buflen > 0) {
		bcm_binit(&bcmstrbuf, buf, buflen);
		b = &bcmstrbuf;
	}

	reglist = (d11regs_list_t *)bcm_get_data_from_xtlv_buf(mi->pd11regxtlv,
		mi->d11regxtlv_bsz, xtlv_type, &reglist_sz, BCM_XTLV_OPTION_ALIGN32);
	reglist_sz /= sizeof(*reglist); /* byte size to array size */

	cnt += _dhd_pd11regs_bylist(mi, reglist, reglist_sz, b);
	DHD_TRACE(("%s reglist_sz %d cnt %d\n", __FUNCTION__, reglist_sz, cnt));

	if (b && outbuflen) {
		if (buflen > BCMSTRBUF_LEN(b)) {
			*outbuflen = buflen - BCMSTRBUF_LEN(b);
		} else {
			DHD_ERROR(("%s: buflen insufficient!\n", __FUNCTION__));
			*outbuflen = buflen;
			/* Do not return buftooshort to allow printing macregs we have got */
		}
	}

	return ((cnt > 0) ? BCME_OK : BCME_UNSUPPORTED);
}

int
dhd_macdbg_dumpsampcol(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen, uint8 sctype)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;
	int corerev;
	int res = BCME_OK;
	uint32 val_l, val_h;
	uint32 gpio_sel, byt_offset, len, i;
	uint16 scpctl, addr0, addr1, curptr;
	d11scregs_addr_t d11scregs_addr;

	if (buf && buflen > 0) {
		bcm_binit(&bcmstrbuf, buf, buflen);
		b = &bcmstrbuf;
	}
	corerev = mi->corerev;

	/* validation check */
	if (corerev < 40 || (corerev < 129 && sctype == DUMP_SC_TYPE_PSM) ||
		sctype == DUMP_SC_TYPE_UTSHM || sctype == DUMP_SC_TYPE_UTSHMX) {
		DHD_ERROR(("%s: unsupported corerev %d or type %d\n",
			__FUNCTION__, corerev, sctype));
		res = BCME_UNSUPPORTED;
		goto exit;
	}

	/* load reg address per corerev and sctype */
	if (sctype == DUMP_SC_TYPE_PSM) {
		if (corerev >= 133) {
			d11scregs_addr.ctrl = 0x1706;
			d11scregs_addr.ctrl2 = 0x172e;
			d11scregs_addr.start_ptr = 0x172a;
			d11scregs_addr.stop_ptr = 0x172c;
			d11scregs_addr.cur_ptr = 0x1728;
		} else if (corerev >= 132) {
			d11scregs_addr.ctrl = 0xf26;
			d11scregs_addr.ctrl2 = 0xf4e;
			d11scregs_addr.start_ptr = 0xf4a;
			d11scregs_addr.stop_ptr = 0xf4c;
			d11scregs_addr.cur_ptr = 0xf48;
		} else { /* corerev >= 129 */
			d11scregs_addr.ctrl = 0xf06;
			d11scregs_addr.ctrl2 = 0xf2e;
			d11scregs_addr.start_ptr = 0xf2a;
			d11scregs_addr.stop_ptr = 0xf2c;
			d11scregs_addr.cur_ptr = 0xf28;
		}
		d11scregs_addr.wr_ptr = 0x36c;
		d11scregs_addr.wr_data = 0x370;
	} else { /* sctype == DUMP_SC_TYPE_TPL */
		if (corerev >= 129) {
			/* exists for certain corerevs only */
			d11scregs_addr.ctrl2 = 0xa28;
		} else {
			d11scregs_addr.ctrl2 = (uint16)-1;
		}

		if (corerev >= 128) {
			d11scregs_addr.ctrl = 0xade;
			d11scregs_addr.start_ptr = 0xaa6;
			d11scregs_addr.stop_ptr = 0xaa8;
			d11scregs_addr.cur_ptr = 0xaa2;
		} else { /* corerev >= 40 */
			d11scregs_addr.ctrl = (corerev >= 54 || corerev == 50) ? 0xb2e : 0x492;
			d11scregs_addr.start_ptr = 0x552;
			d11scregs_addr.stop_ptr = 0x554;
			d11scregs_addr.cur_ptr = 0x556;
		}
		d11scregs_addr.wr_ptr = 0x130;
		d11scregs_addr.wr_data = 0x134;
	}

	gpio_sel = _dhd_get_ihr32(mi, MCTL1_OFFSET, b, FALSE);
	scpctl = _dhd_get_ihr16(mi, d11scregs_addr.ctrl, b, FALSE);
	addr0 = _dhd_get_ihr16(mi, d11scregs_addr.start_ptr, b, FALSE);
	addr1 = _dhd_get_ihr16(mi, d11scregs_addr.stop_ptr, b, FALSE);

	/* stop sample capture */
	_dhd_set_ihr16(mi, d11scregs_addr.ctrl, 0, b, FALSE);

	curptr = _dhd_get_ihr16(mi, d11scregs_addr.cur_ptr, b, FALSE);
	byt_offset = 4 * (curptr - addr0);
	len = (addr1 - addr0 + 1) * 4;
	/* set template area offset with auto increment */
	while ((_dhd_get_ihr32(mi, d11scregs_addr.wr_ptr, b, FALSE) & 3));
	_dhd_set_ihr32(mi, d11scregs_addr.wr_ptr, ((addr0 * 4) | 2), b, FALSE);

	if (b) {
		bcm_bprintf(b, "corerev: %d ucode revision %d.%d features 0x%04x\n",
			corerev,
			_dhd_get_d11obj16(mi, UCREVMAJOR_OFFSET,
				0x00010000, NULL, b, FALSE),
			_dhd_get_d11obj16(mi, UCREVMINOR_OFFSET,
				0x00010000, NULL, b, FALSE),
			_dhd_get_d11obj16(mi, UCFEATURES_OFFSET,
				0x00010000, NULL, b, FALSE));
		bcm_bprintf(b, "Capture mode: maccontrol1 0x%02x scpctl 0x%02x", gpio_sel,
			scpctl);
		if (d11scregs_addr.ctrl2 != (uint16)-1) {
			bcm_bprintf(b, " scpctl2 0x%02x\n",
				_dhd_get_ihr16(mi, d11scregs_addr.ctrl2, b, FALSE));
		} else {
			bcm_bprintf(b, "\n");
		}
		bcm_bprintf(b, "Start/stop/cur 0x%04x 0x%04x 0x%04x byt_offset 0x%05x entries %u\n",
		       addr0, addr1, curptr, byt_offset, len >> 2);
		bcm_bprintf(b, "offset: low high\n");
	} else {
		printf("corerev: %d ucode revision %d.%d features 0x%04x\n",
			corerev,
			_dhd_get_d11obj16(mi, UCREVMAJOR_OFFSET,
				0x00010000, NULL, b, FALSE),
			_dhd_get_d11obj16(mi, UCREVMINOR_OFFSET,
				0x00010000, NULL, b, FALSE),
			_dhd_get_d11obj16(mi, UCFEATURES_OFFSET,
				0x00010000, NULL, b, FALSE));
		printf("Capture mode: maccontrol1 0x%02x scpctl 0x%02x", gpio_sel, scpctl);
		if (d11scregs_addr.ctrl2 != (uint16)-1) {
			printf("scpctl2 0x%02x\n",
				_dhd_get_ihr16(mi, d11scregs_addr.ctrl2, b, FALSE));
		} else {
			printf("\n");
		}
		printf("Start/stop/cur 0x%04x 0x%04x 0x%04x byt_offset 0x%05x entries %u\n",
		       addr0, addr1, curptr, byt_offset, len >> 2);
		printf("offset: low high\n");
	}

	for (i = 0; i < len; i += 4) {
		val_l = _dhd_get_ihr32(mi, d11scregs_addr.wr_data, b, FALSE);
		val_h = val_l >> 16;
		val_l = val_l & 0xffff;
		if (b) {
			bcm_bprintf(b, "%05X: %04X %04X\n", i, val_l, val_h);
		} else {
			printf("%05X: %04X %04X\n", i, val_l, val_h);
		}
	}
exit:
	if (b && outbuflen) {
		if (buflen > BCMSTRBUF_LEN(b)) {
			*outbuflen = buflen - BCMSTRBUF_LEN(b);
		} else {
			DHD_ERROR(("%s: buflen insufficient!\n", __FUNCTION__));
			*outbuflen = buflen;
			/* Do not return buftooshort to allow printing macregs we have got */
		}
	}

	return res;
}

int
dhd_macdbg_pd11regs(dhd_pub_t *dhdp, char *params, int plen, char *buf, int buflen)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	dhd_pd11regs_param_t *pd11regs = (void *)params;
	dhd_pd11regs_buf_t *pd11regs_buf = (void *)buf;
	uint16 start_idx;
	bool verbose;
	d11regs_list_t reglist;
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;

	start_idx = pd11regs->start_idx;
	verbose = pd11regs->verbose;
	memcpy(&reglist, pd11regs->plist, sizeof(reglist));
	memset(buf, '\0', buflen);
	bcm_binit(&bcmstrbuf, pd11regs_buf->pbuf,
		(buflen - OFFSETOF(dhd_pd11regs_buf_t, pbuf)));
	b = &bcmstrbuf;
	pd11regs_buf->idx = _dhd_print_d11regs(mi, &reglist, start_idx, b, verbose);

	return ((pd11regs_buf->idx > 0) ? BCME_OK : BCME_ERROR);
}

static int
_dhd_print_svmps(macdbg_info_t *mi, svmp_list_t *psvmp,
	int start_idx, struct bcmstrbuf *b, bool verbose)
{
	int idx;
	uint32 addr, mem_id, offset, prev_mem_id, prev_offset;
	uint16 cnt, val;

	BCM_REFERENCE(start_idx);

	/* Set tbl ID and tbl offset. */
	_dhd_set_ihr32(mi, MACREG_PHY_REG_ADDR, (0x300000 | PHYREG_TABLE_ID), b, verbose);
	_dhd_set_ihr32(mi, MACREG_PHY_REG_ADDR, (0x80000000 | PHYREG_TABLE_OFFSET),
			b, verbose);

	addr = psvmp->addr;
	cnt = psvmp->cnt;

	/* In validate previous mem_id and offset */
	prev_mem_id = (uint32)(-1);
	prev_offset = (uint32)(-1);

	for (idx = 0; idx < cnt; idx++, addr++) {
		mem_id = (addr >> 15);
		offset = (addr & 0x7fff) >> 1;

		if (mem_id != prev_mem_id) {
			/* Set mem_id */
			_dhd_set_ihr32(mi, MACREG_PHY_REG_ADDR,
				((mem_id & 0xffff0000) | PHYREG_TABLE_DATA_HI), b, verbose);
			_dhd_set_ihr32(mi, MACREG_PHY_REG_ADDR,
				((mem_id << 16) | PHYREG_TABLE_DATA_LO), b, verbose);
		}

		if (offset != prev_offset) {
			/* svmp offset */
			_dhd_set_ihr32(mi, MACREG_PHY_REG_ADDR, ((offset << 16) | 0xe),
				b, verbose);
		}
		/* Read hi or lo */
		_dhd_set_ihr16(mi, MACREG_PHY_REG_ADDR,
			((addr & 0x1) ?  PHYREG_TABLE_DATA_HI : PHYREG_TABLE_DATA_LO), b, verbose);
		val = _dhd_get_ihr16(mi, MACREG_PHY_REG_DATA, b, verbose);
		if (b) {
			bcm_bprintf(b, "svmp   0x%-4x   0x%-4x\n",
				addr, val);

		} else {
			printf("svmp   0x%-4x   0x%-4x\n",
				addr, val);
		}
		prev_mem_id = mem_id;
		prev_offset = offset;
	}
	return idx;
}

static int
_dhd_psvmps_bylist(macdbg_info_t *mi, svmp_list_t *svmplist,
	uint16 svmplist_sz, struct bcmstrbuf *b)
{
	uint i, j, idx = 0, step, svmp_len, offset_size;
	uint svmp_block_len;
	uint32 svmp_addr;
	uint32 compr_rpt_start, compr_rpt_end, compr_rpt_step;
	uint32 cqi_rpt_start, cqi_rpt_end, cqi_rpt_step;
	svmp_list_t tmp;

	if (mi->corerev == 133 || mi->corerev == 134) {
		// 6717
		compr_rpt_step  = 32;
		compr_rpt_start = 0x0;
		compr_rpt_end   = 0x20000;
		if (mi->corerev == 134) {
			cqi_rpt_step = 19;
		} else {
			cqi_rpt_step = 11;
		}
		cqi_rpt_start   = 0x30000;
		cqi_rpt_end     = 0x34000;
	} else {
		// mi->corerev >= 128
		// 43684 & 6715
		compr_rpt_step  = 16;
		compr_rpt_start = 0x10000;
		compr_rpt_end   = 0x20000;
		cqi_rpt_step    = 11;
		cqi_rpt_start   = 0xe000;
		cqi_rpt_end     = 0x10000;
	}

	if (svmplist != NULL && svmplist_sz > 0) {
		for (i = 0; i < svmplist_sz; i++) {
			svmp_addr = svmplist[i].addr;
			DHD_TRACE(("%s %p %d\n", __FUNCTION__, &svmplist[i], svmplist_sz));

			if ((svmp_addr >= compr_rpt_start) && (svmp_addr < compr_rpt_end)) {
				step = compr_rpt_step, svmp_len = 16, offset_size = 32;
			} else if ((svmp_addr >= cqi_rpt_start) && (svmp_addr < cqi_rpt_end)) {
				step = cqi_rpt_step, svmp_len = 16, offset_size = 16;
			} else {
				step = 1, svmp_len = 32, offset_size = 32;
			}

			svmp_block_len =  svmplist[i].cnt/32;
			for (j = 0; j < svmp_block_len; j += step) {
				tmp = (svmp_list_t) {svmp_addr + offset_size*j, svmp_len};
				idx += _dhd_print_svmps(mi, &tmp, idx, b, FALSE);
			}
		}
	}

	return idx;
}

static int
_dhd_print_phytable(macdbg_info_t *mi, int tableID, int tableOffset,
	int tableLen, int tableWide, struct bcmstrbuf *b, bool verbose)
{
	int idx, m;
	uint16 val[4];

	/* Set tbl ID and tbl offset. */
	_dhd_set_ihr32(mi, MACREG_PHY_REG_ADDR,
		(tableID << 16) | PHYREG_TABLE_ID, b, verbose);

	for (idx = tableOffset; idx < (tableOffset + tableLen); idx++) {
		_dhd_set_ihr32(mi, MACREG_PHY_REG_ADDR,
			((idx << 16) | PHYREG_TABLE_OFFSET), b, verbose);

		if (tableWide == 32) {
			_dhd_set_ihr16(mi, MACREG_PHY_REG_ADDR,
				PHYREG_TABLE_DATA_LO, b, verbose);
			val[0] = _dhd_get_ihr16(mi, MACREG_PHY_REG_DATA, b, verbose);

			_dhd_set_ihr16(mi, MACREG_PHY_REG_ADDR,
				PHYREG_TABLE_DATA_HI, b, verbose);
			val[1] = _dhd_get_ihr16(mi, MACREG_PHY_REG_DATA, b, verbose);
			if (b) {
				bcm_bprintf(b, "0x%-4x   0x%-4x   0x%-8x\n",
					tableID, idx, ((val[1] << 16) | val[0]));
			} else {
				printf("0x%-4x   0x%-4x   0x%-8x\n",
					tableID, idx, ((val[1] << 16) | val[0]));
			}
		} else if (tableWide == 64) {
			for (m = 0; m < 4; m++) {
				_dhd_set_ihr16(mi, MACREG_PHY_REG_ADDR,
					PHYREG_TABLE_DATA_WIDE, b, verbose);
				val[m] = _dhd_get_ihr16(mi, MACREG_PHY_REG_DATA,
					b, verbose);
			}

			if (b) {
				bcm_bprintf(b, "0x%-4x   0x%-4x   0x%-8x   0x%-8x\n", tableID, idx,
					((val[1] << 16) | val[0]), ((val[3] << 16) | val[2]));
			} else {
				printf("0x%-4x   0x%-4x   0x%-8x   0x%-8x\n", tableID, idx,
					((val[1] << 16) | val[0]), ((val[3] << 16) | val[2]));
			}
		}
	}
	return idx;
}

static int
dhd_macdbg_dumpphysvmp(dhd_pub_t *dhdp, char *buf, int buflen,
	int *outbuflen)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;
	int cnt0, cnt1;

	if (buf && buflen > 0) {
		bcm_binit(&bcmstrbuf, buf, buflen);
		b = &bcmstrbuf;
	}

	cnt0 = dhd_macdbg_dumpphy(mi, b);
	cnt1 = dhd_macdbg_dumpsvmp(mi, b);

	if (b && outbuflen) {
		if (buflen > BCMSTRBUF_LEN(b)) {
			*outbuflen = buflen - BCMSTRBUF_LEN(b);
		} else {
			DHD_ERROR(("%s: buflen insufficient!\n", __FUNCTION__));
			*outbuflen = buflen;
			/* Do not return buftooshort to allow printing macregs we have got */
		}
	}

	return (((cnt0+cnt1) > 0) ? BCME_OK : BCME_UNSUPPORTED);
}

static bool
dhd_macdbg_is_svmp_ready(macdbg_info_t *mi)
{
	uint16 val;

	if (mi->corerev >= 129) {
		/* check for simultaneous access */
		_dhd_set_ihr16(mi, MACREG_PHY_REG_ADDR, 0x12a5, NULL, FALSE);
		val = _dhd_get_ihr16(mi, MACREG_PHY_REG_DATA, NULL, FALSE) & 0x200;
		return (val == 0);
	}
	/* 4366 has SVMP, but no simultaneous access */
	return (mi->corerev == 65);
}

static int
dhd_macdbg_dumpsvmp(macdbg_info_t *mi, struct bcmstrbuf *b)
{
	svmp_list_t *svmplist = NULL;
	uint16 svmplist_sz, cnt = 0;

	if (dhd_macdbg_is_svmp_ready(mi)) {
		if (b) {
			bcm_bprintf(b, "\n===== Important SVMP =====\n");
		} else {
			printf("\n===== Important SVMP =====\n");
		}

		svmplist = (svmp_list_t *)bcm_get_data_from_xtlv_buf(mi->pd11regxtlv,
			mi->d11regxtlv_bsz, D11REG_XTLV_SVMP,
			&svmplist_sz, BCM_XTLV_OPTION_ALIGN32);
		svmplist_sz /= sizeof(*svmplist); /* byte size to array size */

		cnt += _dhd_psvmps_bylist(mi, svmplist, svmplist_sz, b);
		DHD_TRACE(("%s svmplist_sz %d cnt %d\n", __FUNCTION__, svmplist_sz, cnt));
	} else {
		printf("SVMP not available/skipped\n");
	}

	return cnt;
}

static int
_dhd_print_phyreg(macdbg_info_t *mi, int reg_addr, struct bcmstrbuf *b, bool verbose)
{
	uint16 val;

	_dhd_set_ihr16(mi, MACREG_PHY_REG_ADDR, reg_addr, b, verbose);
	val = _dhd_get_ihr16(mi, MACREG_PHY_REG_DATA, b, verbose);

	if (b) {
		bcm_bprintf(b, "0x%-4x   0x%-4x\n", reg_addr, val);
	} else {
		printf("0x%-4x   0x%-4x\n", reg_addr, val);
	}
	return 1;
}

typedef struct _d11phyregs_list_t {
	uint16 addr;
	uint16 cnt;
} d11phyregs_list_t;

static CONST d11phyregs_list_t phyreg_list_all[] = {
	{0x1F0, 2},
	{0x60D, 1},
	{0x80D, 1},
	{0xA0D, 1},
	{0x601, 1},
	{0x801, 1},
	{0xA01, 1},
	{0x3EC, 1},
	{0x003, 1},
};

static CONST d11phyregs_list_t phyreg_list_r32_r33[] = {
	{0xC01, 1},
	{0xC0D, 1},
	{0x1049, 10},
	{0x10EC,  4},
	{0x11C0, 3},
};

static CONST d11phyregs_list_t phyreg_list_lt132[] = {
	{0xC0D, 1},
	{0x1301, 1},
	{0x1311, 1},
	{0xC01, 1},
	{0x107D, 1},
	{0x1049, 10},
	{0x10EC,  4},
	{0x1110,  9},
	{0x11C0, 16},
	{0x1290, 77},
	{0x1400,  4},
	{0x1405, 17},
	{0x39F,   2},
	{0x1368,  6},
	{0x55C,   3},
	{0x439,  22},
	{0x168E,  2},
	{0x1696,  2},
	{0x34A,   1}
};

static CONST d11phyregs_list_t phyreg_list_r47[] = {
	{ 0x0000, 0x0006 }, /* 0x0000 - 0x0005 */
	{ 0x0009, 0x0004 }, /* 0x0009 - 0x000c */
	{ 0x0012, 0x0025 }, /* 0x0012 - 0x0036 */
	{ 0x0038, 0x0019 }, /* 0x0038 - 0x0050 */
	{ 0x0052, 0x0016 }, /* 0x0052 - 0x0067 */
	{ 0x006f, 0x000f }, /* 0x006f - 0x007d */
	{ 0x007f, 0x002f }, /* 0x007f - 0x00ad */
	{ 0x00b0, 0x0069 }, /* 0x00b0 - 0x0118 */
	{ 0x0120, 0x000b }, /* 0x0120 - 0x012a */
	{ 0x0130, 0x000c }, /* 0x0130 - 0x013b */
	{ 0x0140, 0x001f }, /* 0x0140 - 0x015e */
	{ 0x0160, 0x001a }, /* 0x0160 - 0x0179 */
	{ 0x017c, 0x0005 }, /* 0x017c - 0x0180 */
	{ 0x0195, 0x0011 }, /* 0x0195 - 0x01a5 */
	{ 0x01a7, 0x0004 }, /* 0x01a7 - 0x01aa */
	{ 0x01ad, 0x0060 }, /* 0x01ad - 0x020c */
	{ 0x0210, 0x0003 }, /* 0x0210 - 0x0212 */
	{ 0x0218, 0x0010 }, /* 0x0218 - 0x0227 */
	{ 0x0230, 0x000a }, /* 0x0230 - 0x0239 */
	{ 0x0240, 0x000f }, /* 0x0240 - 0x024e */
	{ 0x0250, 0x0029 }, /* 0x0250 - 0x0278 */
	{ 0x027e, 0x006c }, /* 0x027e - 0x02e9 */
	{ 0x02ee, 0x0013 }, /* 0x02ee - 0x0300 */
	{ 0x030e, 0x000a }, /* 0x030e - 0x0317 */
	{ 0x0320, 0x001b }, /* 0x0320 - 0x033a */
	{ 0x034a, 0x0001 }, /* 0x034a - 0x034a */
	{ 0x034d, 0x0018 }, /* 0x034d - 0x0364 */
	{ 0x0366, 0x0004 }, /* 0x0366 - 0x0369 */
	{ 0x0370, 0x000e }, /* 0x0370 - 0x037d */
	{ 0x0380, 0x0006 }, /* 0x0380 - 0x0385 */
	{ 0x0390, 0x000b }, /* 0x0390 - 0x039a */
	{ 0x039c, 0x005e }, /* 0x039c - 0x03f9 */
	{ 0x0400, 0x001c }, /* 0x0400 - 0x041b */
	{ 0x041e, 0x0010 }, /* 0x041e - 0x042d */
	{ 0x0430, 0x0054 }, /* 0x0430 - 0x0483 */
	{ 0x048e, 0x0001 }, /* 0x048e - 0x048e */
	{ 0x0490, 0x0001 }, /* 0x0490 - 0x0490 */
	{ 0x0496, 0x0001 }, /* 0x0496 - 0x0496 */
	{ 0x049d, 0x0001 }, /* 0x049d - 0x049d */
	{ 0x04a0, 0x0001 }, /* 0x04a0 - 0x04a0 */
	{ 0x04b8, 0x0001 }, /* 0x04b8 - 0x04b8 */
	{ 0x04d6, 0x0005 }, /* 0x04d6 - 0x04da */
	{ 0x04dd, 0x0004 }, /* 0x04dd - 0x04e0 */
	{ 0x04e2, 0x0003 }, /* 0x04e2 - 0x04e4 */
	{ 0x04e6, 0x0001 }, /* 0x04e6 - 0x04e6 */
	{ 0x04f4, 0x0003 }, /* 0x04f4 - 0x04f6 */
	{ 0x04f8, 0x0003 }, /* 0x04f8 - 0x04fa */
	{ 0x0500, 0x0003 }, /* 0x0500 - 0x0502 */
	{ 0x0510, 0x0001 }, /* 0x0510 - 0x0510 */
	{ 0x0520, 0x000d }, /* 0x0520 - 0x052c */
	{ 0x0550, 0x000f }, /* 0x0550 - 0x055e */
	{ 0x0560, 0x0007 }, /* 0x0560 - 0x0566 */
	{ 0x0570, 0x0005 }, /* 0x0570 - 0x0574 */
	{ 0x0578, 0x003e }, /* 0x0578 - 0x05b5 */
	{ 0x05c0, 0x000c }, /* 0x05c0 - 0x05cb */
	{ 0x05d0, 0x000a }, /* 0x05d0 - 0x05d9 */
	{ 0x05f0, 0x0009 }, /* 0x05f0 - 0x05f8 */
	{ 0x0600, 0x002e }, /* 0x0600 - 0x062d */
	{ 0x062f, 0x000a }, /* 0x062f - 0x0638 */
	{ 0x063f, 0x0014 }, /* 0x063f - 0x0652 */
	{ 0x0660, 0x0003 }, /* 0x0660 - 0x0662 */
	{ 0x0665, 0x0001 }, /* 0x0665 - 0x0665 */
	{ 0x066a, 0x0002 }, /* 0x066a - 0x066b */
	{ 0x0670, 0x0016 }, /* 0x0670 - 0x0685 */
	{ 0x0688, 0x0003 }, /* 0x0688 - 0x068a */
	{ 0x0690, 0x0007 }, /* 0x0690 - 0x0696 */
	{ 0x0698, 0x0021 }, /* 0x0698 - 0x06b8 */
	{ 0x06ba, 0x0014 }, /* 0x06ba - 0x06cd */
	{ 0x06d0, 0x003e }, /* 0x06d0 - 0x070d */
	{ 0x0710, 0x001a }, /* 0x0710 - 0x0729 */
	{ 0x0730, 0x0010 }, /* 0x0730 - 0x073f */
	{ 0x0741, 0x001f }, /* 0x0741 - 0x075f */
	{ 0x0767, 0x0001 }, /* 0x0767 - 0x0767 */
	{ 0x0769, 0x0001 }, /* 0x0769 - 0x0769 */
	{ 0x0770, 0x000e }, /* 0x0770 - 0x077d */
	{ 0x0780, 0x001a }, /* 0x0780 - 0x0799 */
	{ 0x079d, 0x0001 }, /* 0x079d - 0x079d */
	{ 0x07a0, 0x0040 }, /* 0x07a0 - 0x07df */
	{ 0x07e2, 0x0001 }, /* 0x07e2 - 0x07e2 */
	{ 0x07ec, 0x0024 }, /* 0x07ec - 0x080f */
	{ 0x083f, 0x0014 }, /* 0x083f - 0x0852 */
	{ 0x0860, 0x0003 }, /* 0x0860 - 0x0862 */
	{ 0x0865, 0x0001 }, /* 0x0865 - 0x0865 */
	{ 0x086a, 0x0002 }, /* 0x086a - 0x086b */
	{ 0x0870, 0x0016 }, /* 0x0870 - 0x0885 */
	{ 0x0888, 0x0003 }, /* 0x0888 - 0x088a */
	{ 0x0890, 0x0007 }, /* 0x0890 - 0x0896 */
	{ 0x0898, 0x0021 }, /* 0x0898 - 0x08b8 */
	{ 0x08ba, 0x0014 }, /* 0x08ba - 0x08cd */
	{ 0x08d0, 0x003e }, /* 0x08d0 - 0x090d */
	{ 0x0910, 0x001a }, /* 0x0910 - 0x0929 */
	{ 0x0930, 0x0010 }, /* 0x0930 - 0x093f */
	{ 0x0941, 0x001f }, /* 0x0941 - 0x095f */
	{ 0x0967, 0x0001 }, /* 0x0967 - 0x0967 */
	{ 0x0969, 0x0001 }, /* 0x0969 - 0x0969 */
	{ 0x0970, 0x000e }, /* 0x0970 - 0x097d */
	{ 0x0980, 0x001a }, /* 0x0980 - 0x0999 */
	{ 0x099d, 0x0001 }, /* 0x099d - 0x099d */
	{ 0x09a0, 0x0040 }, /* 0x09a0 - 0x09df */
	{ 0x09e2, 0x0001 }, /* 0x09e2 - 0x09e2 */
	{ 0x09ec, 0x0024 }, /* 0x09ec - 0x0a0f */
	{ 0x0a3f, 0x0014 }, /* 0x0a3f - 0x0a52 */
	{ 0x0a60, 0x0003 }, /* 0x0a60 - 0x0a62 */
	{ 0x0a65, 0x0001 }, /* 0x0a65 - 0x0a65 */
	{ 0x0a6a, 0x0002 }, /* 0x0a6a - 0x0a6b */
	{ 0x0a70, 0x0016 }, /* 0x0a70 - 0x0a85 */
	{ 0x0a88, 0x0003 }, /* 0x0a88 - 0x0a8a */
	{ 0x0a90, 0x0007 }, /* 0x0a90 - 0x0a96 */
	{ 0x0a98, 0x0021 }, /* 0x0a98 - 0x0ab8 */
	{ 0x0aba, 0x0014 }, /* 0x0aba - 0x0acd */
	{ 0x0ad0, 0x003e }, /* 0x0ad0 - 0x0b0d */
	{ 0x0b10, 0x001a }, /* 0x0b10 - 0x0b29 */
	{ 0x0b30, 0x0010 }, /* 0x0b30 - 0x0b3f */
	{ 0x0b41, 0x001f }, /* 0x0b41 - 0x0b5f */
	{ 0x0b67, 0x0001 }, /* 0x0b67 - 0x0b67 */
	{ 0x0b69, 0x0001 }, /* 0x0b69 - 0x0b69 */
	{ 0x0b70, 0x000e }, /* 0x0b70 - 0x0b7d */
	{ 0x0b80, 0x001a }, /* 0x0b80 - 0x0b99 */
	{ 0x0b9d, 0x0001 }, /* 0x0b9d - 0x0b9d */
	{ 0x0ba0, 0x0040 }, /* 0x0ba0 - 0x0bdf */
	{ 0x0be2, 0x0001 }, /* 0x0be2 - 0x0be2 */
	{ 0x0bec, 0x0024 }, /* 0x0bec - 0x0c0f */
	{ 0x0c3f, 0x0014 }, /* 0x0c3f - 0x0c52 */
	{ 0x0c60, 0x0003 }, /* 0x0c60 - 0x0c62 */
	{ 0x0c65, 0x0001 }, /* 0x0c65 - 0x0c65 */
	{ 0x0c6a, 0x0002 }, /* 0x0c6a - 0x0c6b */
	{ 0x0c70, 0x0016 }, /* 0x0c70 - 0x0c85 */
	{ 0x0c88, 0x0003 }, /* 0x0c88 - 0x0c8a */
	{ 0x0c90, 0x0007 }, /* 0x0c90 - 0x0c96 */
	{ 0x0c98, 0x0021 }, /* 0x0c98 - 0x0cb8 */
	{ 0x0cba, 0x0014 }, /* 0x0cba - 0x0ccd */
	{ 0x0cd0, 0x003e }, /* 0x0cd0 - 0x0d0d */
	{ 0x0d10, 0x001a }, /* 0x0d10 - 0x0d29 */
	{ 0x0d30, 0x0010 }, /* 0x0d30 - 0x0d3f */
	{ 0x0d41, 0x001f }, /* 0x0d41 - 0x0d5f */
	{ 0x0d67, 0x0001 }, /* 0x0d67 - 0x0d67 */
	{ 0x0d69, 0x0001 }, /* 0x0d69 - 0x0d69 */
	{ 0x0d70, 0x000e }, /* 0x0d70 - 0x0d7d */
	{ 0x0d80, 0x001a }, /* 0x0d80 - 0x0d99 */
	{ 0x0d9d, 0x0001 }, /* 0x0d9d - 0x0d9d */
	{ 0x0da0, 0x0040 }, /* 0x0da0 - 0x0ddf */
	{ 0x0de2, 0x0001 }, /* 0x0de2 - 0x0de2 */
	{ 0x0dec, 0x0014 }, /* 0x0dec - 0x0dff */
	{ 0x0e50, 0x0001 }, /* 0x0e50 - 0x0e50 */
	{ 0x0e52, 0x000a }, /* 0x0e52 - 0x0e5b */
	{ 0x0eee, 0x000a }, /* 0x0eee - 0x0ef7 */
	{ 0x0efe, 0x0001 }, /* 0x0efe - 0x0efe */
	{ 0x1000, 0x0010 }, /* 0x1000 - 0x100f */
	{ 0x1020, 0x000e }, /* 0x1020 - 0x102d */
	{ 0x1030, 0x0023 }, /* 0x1030 - 0x1052 */
	{ 0x1060, 0x0009 }, /* 0x1060 - 0x1068 */
	{ 0x1070, 0x000f }, /* 0x1070 - 0x107e */
	{ 0x1081, 0x000e }, /* 0x1081 - 0x108e */
	{ 0x1090, 0x0018 }, /* 0x1090 - 0x10a7 */
	{ 0x10b0, 0x0043 }, /* 0x10b0 - 0x10f2 */
	{ 0x10f4, 0x0026 }, /* 0x10f4 - 0x1119 */
	{ 0x111c, 0x0002 }, /* 0x111c - 0x111d */
	{ 0x1120, 0x0013 }, /* 0x1120 - 0x1132 */
	{ 0x1138, 0x0006 }, /* 0x1138 - 0x113d */
	{ 0x113f, 0x0037 }, /* 0x113f - 0x1175 */
	{ 0x1178, 0x0008 }, /* 0x1178 - 0x117f */
	{ 0x11b0, 0x000c }, /* 0x11b0 - 0x11bb */
	{ 0x11c0, 0x0035 }, /* 0x11c0 - 0x11f4 */
	{ 0x1205, 0x0002 }, /* 0x1205 - 0x1206 */
	{ 0x120a, 0x0005 }, /* 0x120a - 0x120e */
	{ 0x1210, 0x0009 }, /* 0x1210 - 0x1218 */
	{ 0x1240, 0x000d }, /* 0x1240 - 0x124c */
	{ 0x1259, 0x0013 }, /* 0x1259 - 0x126b */
	{ 0x126f, 0x0008 }, /* 0x126f - 0x1276 */
	{ 0x1278, 0x0003 }, /* 0x1278 - 0x127a */
	{ 0x1280, 0x000e }, /* 0x1280 - 0x128d */
	{ 0x128f, 0x004e }, /* 0x128f - 0x12dc */
	{ 0x12e0, 0x0005 }, /* 0x12e0 - 0x12e4 */
	{ 0x12f4, 0x000a }, /* 0x12f4 - 0x12fd */
	{ 0x1300, 0x0014 }, /* 0x1300 - 0x1313 */
	{ 0x1315, 0x0005 }, /* 0x1315 - 0x1319 */
	{ 0x1320, 0x001c }, /* 0x1320 - 0x133b */
	{ 0x1340, 0x0019 }, /* 0x1340 - 0x1358 */
	{ 0x1368, 0x0006 }, /* 0x1368 - 0x136d */
	{ 0x1400, 0x0004 }, /* 0x1400 - 0x1403 */
	{ 0x1405, 0x0011 }, /* 0x1405 - 0x1415 */
	{ 0x1610, 0x0005 }, /* 0x1610 - 0x1614 */
	{ 0x1620, 0x000d }, /* 0x1620 - 0x162c */
	{ 0x1630, 0x0001 }, /* 0x1630 - 0x1630 */
	{ 0x1632, 0x0001 }, /* 0x1632 - 0x1632 */
	{ 0x1636, 0x0002 }, /* 0x1636 - 0x1637 */
	{ 0x1640, 0x000d }, /* 0x1640 - 0x164c */
	{ 0x1650, 0x004c }, /* 0x1650 - 0x169b */
	{ 0x16a0, 0x000a }, /* 0x16a0 - 0x16a9 */
	{ 0x16b0, 0x000d }, /* 0x16b0 - 0x16bc */
	{ 0x16c0, 0x001a }, /* 0x16c0 - 0x16d9 */
	{ 0x16de, 0x0005 }, /* 0x16de - 0x16e2 */
	{ 0x16f0, 0x0023 }, /* 0x16f0 - 0x1712 */
	{ 0x1720, 0x001b }, /* 0x1720 - 0x173a */
	{ 0x1740, 0x0018 }, /* 0x1740 - 0x1757 */
	{ 0x1760, 0x0009 }, /* 0x1760 - 0x1768 */
	{ 0x1770, 0x0009 }, /* 0x1770 - 0x1778 */
	{ 0x17a0, 0x002f }, /* 0x17a0 - 0x17ce */
	{ 0x17d0, 0x0030 }, /* 0x17d0 - 0x17ff */
	{ 0x1810, 0x0005 }, /* 0x1810 - 0x1814 */
	{ 0x1820, 0x000d }, /* 0x1820 - 0x182c */
	{ 0x1830, 0x0001 }, /* 0x1830 - 0x1830 */
	{ 0x1832, 0x0001 }, /* 0x1832 - 0x1832 */
	{ 0x1836, 0x0002 }, /* 0x1836 - 0x1837 */
	{ 0x1840, 0x000d }, /* 0x1840 - 0x184c */
	{ 0x1850, 0x004c }, /* 0x1850 - 0x189b */
	{ 0x18a0, 0x000a }, /* 0x18a0 - 0x18a9 */
	{ 0x18b0, 0x000d }, /* 0x18b0 - 0x18bc */
	{ 0x18c0, 0x001a }, /* 0x18c0 - 0x18d9 */
	{ 0x18de, 0x0005 }, /* 0x18de - 0x18e2 */
	{ 0x18f0, 0x0023 }, /* 0x18f0 - 0x1912 */
	{ 0x1920, 0x001b }, /* 0x1920 - 0x193a */
	{ 0x1940, 0x0018 }, /* 0x1940 - 0x1957 */
	{ 0x1960, 0x0009 }, /* 0x1960 - 0x1968 */
	{ 0x1970, 0x0009 }, /* 0x1970 - 0x1978 */
	{ 0x19a0, 0x002f }, /* 0x19a0 - 0x19ce */
	{ 0x19d0, 0x0030 }, /* 0x19d0 - 0x19ff */
	{ 0x1a10, 0x0005 }, /* 0x1a10 - 0x1a14 */
	{ 0x1a20, 0x000d }, /* 0x1a20 - 0x1a2c */
	{ 0x1a30, 0x0001 }, /* 0x1a30 - 0x1a30 */
	{ 0x1a32, 0x0001 }, /* 0x1a32 - 0x1a32 */
	{ 0x1a36, 0x0002 }, /* 0x1a36 - 0x1a37 */
	{ 0x1a40, 0x000d }, /* 0x1a40 - 0x1a4c */
	{ 0x1a50, 0x004c }, /* 0x1a50 - 0x1a9b */
	{ 0x1aa0, 0x000a }, /* 0x1aa0 - 0x1aa9 */
	{ 0x1ab0, 0x000d }, /* 0x1ab0 - 0x1abc */
	{ 0x1ac0, 0x001a }, /* 0x1ac0 - 0x1ad9 */
	{ 0x1ade, 0x0005 }, /* 0x1ade - 0x1ae2 */
	{ 0x1af0, 0x0023 }, /* 0x1af0 - 0x1b12 */
	{ 0x1b20, 0x001b }, /* 0x1b20 - 0x1b3a */
	{ 0x1b40, 0x0018 }, /* 0x1b40 - 0x1b57 */
	{ 0x1b60, 0x0009 }, /* 0x1b60 - 0x1b68 */
	{ 0x1b70, 0x0009 }, /* 0x1b70 - 0x1b78 */
	{ 0x1ba0, 0x002f }, /* 0x1ba0 - 0x1bce */
	{ 0x1bd0, 0x0030 }, /* 0x1bd0 - 0x1bff */
	{ 0x1c10, 0x0005 }, /* 0x1c10 - 0x1c14 */
	{ 0x1c20, 0x000d }, /* 0x1c20 - 0x1c2c */
	{ 0x1c30, 0x0001 }, /* 0x1c30 - 0x1c30 */
	{ 0x1c32, 0x0001 }, /* 0x1c32 - 0x1c32 */
	{ 0x1c36, 0x0002 }, /* 0x1c36 - 0x1c37 */
	{ 0x1c40, 0x000d }, /* 0x1c40 - 0x1c4c */
	{ 0x1c50, 0x004c }, /* 0x1c50 - 0x1c9b */
	{ 0x1ca0, 0x000a }, /* 0x1ca0 - 0x1ca9 */
	{ 0x1cb0, 0x000d }, /* 0x1cb0 - 0x1cbc */
	{ 0x1cc0, 0x001a }, /* 0x1cc0 - 0x1cd9 */
	{ 0x1cde, 0x0005 }, /* 0x1cde - 0x1ce2 */
	{ 0x1cf0, 0x0023 }, /* 0x1cf0 - 0x1d12 */
	{ 0x1d20, 0x001b }, /* 0x1d20 - 0x1d3a */
	{ 0x1d40, 0x0018 }, /* 0x1d40 - 0x1d57 */
	{ 0x1d60, 0x0009 }, /* 0x1d60 - 0x1d68 */
	{ 0x1d70, 0x0009 }, /* 0x1d70 - 0x1d78 */
	{ 0x1da0, 0x002f }, /* 0x1da0 - 0x1dce */
	{ 0x1dd0, 0x0030 }, /* 0x1dd0 - 0x1dff */
	{ 0x1e10, 0x0002 }, /* 0x1e10 - 0x1e11 */
	{ 0x2600, 0x000f }, /* 0x2600 - 0x260e */
	{ 0x2800, 0x000f }, /* 0x2800 - 0x280e */
	{ 0x2a00, 0x000f }, /* 0x2a00 - 0x2a0e */
	{ 0x2c00, 0x000f }, /* 0x2c00 - 0x2c0e */
	{ 0x3590, 0x0002 }, /* 0x3590 - 0x3591 */
	{ 0x3790, 0x0002 }, /* 0x3790 - 0x3791 */
	{ 0x3990, 0x0002 }, /* 0x3990 - 0x3991 */
	{ 0x3b90, 0x0002 }  /* 0x3b90 - 0x3b91 */
};

static CONST d11phyregs_list_t phyreg_list_r130[] = {
	{ 0x0000, 0x0006 }, /* 0x0000 - 0x0005 */
	{ 0x0009, 0x0004 }, /* 0x0009 - 0x000c */
	{ 0x0012, 0x0025 }, /* 0x0012 - 0x0036 */
	{ 0x0038, 0x0035 }, /* 0x0038 - 0x006c */
	{ 0x0070, 0x000e }, /* 0x0070 - 0x007d */
	{ 0x007f, 0x0031 }, /* 0x007f - 0x00af */
	{ 0x010a, 0x0003 }, /* 0x010a - 0x010c */
	{ 0x0110, 0x0002 }, /* 0x0110 - 0x0111 */
	{ 0x0119, 0x0001 }, /* 0x0119 - 0x0119 */
	{ 0x0120, 0x005a }, /* 0x0120 - 0x0179 */
	{ 0x017c, 0x0005 }, /* 0x017c - 0x0180 */
	{ 0x0195, 0x0016 }, /* 0x0195 - 0x01aa */
	{ 0x01ad, 0x0062 }, /* 0x01ad - 0x020e */
	{ 0x0210, 0x0003 }, /* 0x0210 - 0x0212 */
	{ 0x0218, 0x0022 }, /* 0x0218 - 0x0239 */
	{ 0x0240, 0x00ab }, /* 0x0240 - 0x02ea */
	{ 0x02ee, 0x0019 }, /* 0x02ee - 0x0306 */
	{ 0x030e, 0x000c }, /* 0x030e - 0x0319 */
	{ 0x031e, 0x001d }, /* 0x031e - 0x033a */
	{ 0x034a, 0x0001 }, /* 0x034a - 0x034a */
	{ 0x034d, 0x0018 }, /* 0x034d - 0x0364 */
	{ 0x0366, 0x0020 }, /* 0x0366 - 0x0385 */
	{ 0x038a, 0x0011 }, /* 0x038a - 0x039a */
	{ 0x039c, 0x0080 }, /* 0x039c - 0x041b */
	{ 0x041e, 0x0010 }, /* 0x041e - 0x042d */
	{ 0x0430, 0x0054 }, /* 0x0430 - 0x0483 */
	{ 0x048e, 0x0001 }, /* 0x048e - 0x048e */
	{ 0x0490, 0x0001 }, /* 0x0490 - 0x0490 */
	{ 0x0496, 0x0001 }, /* 0x0496 - 0x0496 */
	{ 0x049d, 0x0001 }, /* 0x049d - 0x049d */
	{ 0x04a0, 0x0001 }, /* 0x04a0 - 0x04a0 */
	{ 0x04b0, 0x0006 }, /* 0x04b0 - 0x04b5 */
	{ 0x04b8, 0x0003 }, /* 0x04b8 - 0x04ba */
	{ 0x04c0, 0x0010 }, /* 0x04c0 - 0x04cf */
	{ 0x04d6, 0x0011 }, /* 0x04d6 - 0x04e6 */
	{ 0x04f4, 0x0003 }, /* 0x04f4 - 0x04f6 */
	{ 0x04f8, 0x0003 }, /* 0x04f8 - 0x04fa */
	{ 0x0500, 0x0003 }, /* 0x0500 - 0x0502 */
	{ 0x0510, 0x000c }, /* 0x0510 - 0x051b */
	{ 0x0520, 0x000d }, /* 0x0520 - 0x052c */
	{ 0x0550, 0x000f }, /* 0x0550 - 0x055e */
	{ 0x0560, 0x0007 }, /* 0x0560 - 0x0566 */
	{ 0x0570, 0x0005 }, /* 0x0570 - 0x0574 */
	{ 0x0578, 0x003e }, /* 0x0578 - 0x05b5 */
	{ 0x05bc, 0x0001 }, /* 0x05bc - 0x05bc */
	{ 0x05c0, 0x000c }, /* 0x05c0 - 0x05cb */
	{ 0x05d0, 0x000a }, /* 0x05d0 - 0x05d9 */
	{ 0x05e0, 0x000c }, /* 0x05e0 - 0x05eb */
	{ 0x05f0, 0x0009 }, /* 0x05f0 - 0x05f8 */
	{ 0x0600, 0x0010 }, /* 0x0600 - 0x060f */
	{ 0x063f, 0x0015 }, /* 0x063f - 0x0653 */
	{ 0x0660, 0x0003 }, /* 0x0660 - 0x0662 */
	{ 0x0665, 0x0001 }, /* 0x0665 - 0x0665 */
	{ 0x066a, 0x0002 }, /* 0x066a - 0x066b */
	{ 0x0670, 0x0016 }, /* 0x0670 - 0x0685 */
	{ 0x0688, 0x0003 }, /* 0x0688 - 0x068a */
	{ 0x0690, 0x0007 }, /* 0x0690 - 0x0696 */
	{ 0x0698, 0x0021 }, /* 0x0698 - 0x06b8 */
	{ 0x06ba, 0x0015 }, /* 0x06ba - 0x06ce */
	{ 0x06d0, 0x003e }, /* 0x06d0 - 0x070d */
	{ 0x0710, 0x001a }, /* 0x0710 - 0x0729 */
	{ 0x0730, 0x0010 }, /* 0x0730 - 0x073f */
	{ 0x0741, 0x001f }, /* 0x0741 - 0x075f */
	{ 0x0767, 0x0001 }, /* 0x0767 - 0x0767 */
	{ 0x0769, 0x0001 }, /* 0x0769 - 0x0769 */
	{ 0x0770, 0x000e }, /* 0x0770 - 0x077d */
	{ 0x0780, 0x001a }, /* 0x0780 - 0x0799 */
	{ 0x079d, 0x0001 }, /* 0x079d - 0x079d */
	{ 0x07a0, 0x0040 }, /* 0x07a0 - 0x07df */
	{ 0x07e2, 0x0001 }, /* 0x07e2 - 0x07e2 */
	{ 0x07eb, 0x0025 }, /* 0x07eb - 0x080f */
	{ 0x0820, 0x000d }, /* 0x0820 - 0x082c */
	{ 0x083f, 0x0015 }, /* 0x083f - 0x0853 */
	{ 0x0860, 0x0003 }, /* 0x0860 - 0x0862 */
	{ 0x0865, 0x0001 }, /* 0x0865 - 0x0865 */
	{ 0x086a, 0x0002 }, /* 0x086a - 0x086b */
	{ 0x0870, 0x0016 }, /* 0x0870 - 0x0885 */
	{ 0x0888, 0x0003 }, /* 0x0888 - 0x088a */
	{ 0x0890, 0x0007 }, /* 0x0890 - 0x0896 */
	{ 0x0898, 0x0021 }, /* 0x0898 - 0x08b8 */
	{ 0x08ba, 0x0015 }, /* 0x08ba - 0x08ce */
	{ 0x08d0, 0x003e }, /* 0x08d0 - 0x090d */
	{ 0x0910, 0x001a }, /* 0x0910 - 0x0929 */
	{ 0x0930, 0x0010 }, /* 0x0930 - 0x093f */
	{ 0x0941, 0x001f }, /* 0x0941 - 0x095f */
	{ 0x0967, 0x0001 }, /* 0x0967 - 0x0967 */
	{ 0x0969, 0x0001 }, /* 0x0969 - 0x0969 */
	{ 0x0970, 0x000e }, /* 0x0970 - 0x097d */
	{ 0x0980, 0x001a }, /* 0x0980 - 0x0999 */
	{ 0x099d, 0x0001 }, /* 0x099d - 0x099d */
	{ 0x09a0, 0x0040 }, /* 0x09a0 - 0x09df */
	{ 0x09e2, 0x0001 }, /* 0x09e2 - 0x09e2 */
	{ 0x09eb, 0x0025 }, /* 0x09eb - 0x0a0f */
	{ 0x0a3f, 0x0015 }, /* 0x0a3f - 0x0a53 */
	{ 0x0a60, 0x0003 }, /* 0x0a60 - 0x0a62 */
	{ 0x0a65, 0x0001 }, /* 0x0a65 - 0x0a65 */
	{ 0x0a6a, 0x0002 }, /* 0x0a6a - 0x0a6b */
	{ 0x0a70, 0x0016 }, /* 0x0a70 - 0x0a85 */
	{ 0x0a88, 0x0003 }, /* 0x0a88 - 0x0a8a */
	{ 0x0a90, 0x0007 }, /* 0x0a90 - 0x0a96 */
	{ 0x0a98, 0x0021 }, /* 0x0a98 - 0x0ab8 */
	{ 0x0aba, 0x0015 }, /* 0x0aba - 0x0ace */
	{ 0x0ad0, 0x003e }, /* 0x0ad0 - 0x0b0d */
	{ 0x0b10, 0x001a }, /* 0x0b10 - 0x0b29 */
	{ 0x0b30, 0x0010 }, /* 0x0b30 - 0x0b3f */
	{ 0x0b41, 0x001f }, /* 0x0b41 - 0x0b5f */
	{ 0x0b67, 0x0001 }, /* 0x0b67 - 0x0b67 */
	{ 0x0b69, 0x0001 }, /* 0x0b69 - 0x0b69 */
	{ 0x0b70, 0x000e }, /* 0x0b70 - 0x0b7d */
	{ 0x0b80, 0x001a }, /* 0x0b80 - 0x0b99 */
	{ 0x0b9d, 0x0001 }, /* 0x0b9d - 0x0b9d */
	{ 0x0ba0, 0x0040 }, /* 0x0ba0 - 0x0bdf */
	{ 0x0be2, 0x0001 }, /* 0x0be2 - 0x0be2 */
	{ 0x0beb, 0x0025 }, /* 0x0beb - 0x0c0f */
	{ 0x0c3f, 0x0015 }, /* 0x0c3f - 0x0c53 */
	{ 0x0c60, 0x0003 }, /* 0x0c60 - 0x0c62 */
	{ 0x0c65, 0x0001 }, /* 0x0c65 - 0x0c65 */
	{ 0x0c6a, 0x0002 }, /* 0x0c6a - 0x0c6b */
	{ 0x0c70, 0x0016 }, /* 0x0c70 - 0x0c85 */
	{ 0x0c88, 0x0003 }, /* 0x0c88 - 0x0c8a */
	{ 0x0c90, 0x0007 }, /* 0x0c90 - 0x0c96 */
	{ 0x0c98, 0x0021 }, /* 0x0c98 - 0x0cb8 */
	{ 0x0cba, 0x0015 }, /* 0x0cba - 0x0cce */
	{ 0x0cd0, 0x003e }, /* 0x0cd0 - 0x0d0d */
	{ 0x0d10, 0x001a }, /* 0x0d10 - 0x0d29 */
	{ 0x0d30, 0x0010 }, /* 0x0d30 - 0x0d3f */
	{ 0x0d41, 0x001f }, /* 0x0d41 - 0x0d5f */
	{ 0x0d67, 0x0001 }, /* 0x0d67 - 0x0d67 */
	{ 0x0d69, 0x0001 }, /* 0x0d69 - 0x0d69 */
	{ 0x0d70, 0x000e }, /* 0x0d70 - 0x0d7d */
	{ 0x0d80, 0x001a }, /* 0x0d80 - 0x0d99 */
	{ 0x0d9d, 0x0001 }, /* 0x0d9d - 0x0d9d */
	{ 0x0da0, 0x0040 }, /* 0x0da0 - 0x0ddf */
	{ 0x0de2, 0x0001 }, /* 0x0de2 - 0x0de2 */
	{ 0x0deb, 0x0015 }, /* 0x0deb - 0x0dff */
	{ 0x0e50, 0x0001 }, /* 0x0e50 - 0x0e50 */
	{ 0x0e52, 0x000b }, /* 0x0e52 - 0x0e5c */
	{ 0x0eee, 0x000a }, /* 0x0eee - 0x0ef7 */
	{ 0x0efe, 0x0001 }, /* 0x0efe - 0x0efe */
	{ 0x1000, 0x0010 }, /* 0x1000 - 0x100f */
	{ 0x1020, 0x000e }, /* 0x1020 - 0x102d */
	{ 0x1030, 0x0023 }, /* 0x1030 - 0x1052 */
	{ 0x1060, 0x0009 }, /* 0x1060 - 0x1068 */
	{ 0x1070, 0x0010 }, /* 0x1070 - 0x107f */
	{ 0x1081, 0x000e }, /* 0x1081 - 0x108e */
	{ 0x1090, 0x0018 }, /* 0x1090 - 0x10a7 */
	{ 0x10b0, 0x0043 }, /* 0x10b0 - 0x10f2 */
	{ 0x10f4, 0x002a }, /* 0x10f4 - 0x111d */
	{ 0x1120, 0x0013 }, /* 0x1120 - 0x1132 */
	{ 0x1138, 0x003e }, /* 0x1138 - 0x1175 */
	{ 0x1178, 0x0008 }, /* 0x1178 - 0x117f */
	{ 0x11ac, 0x0003 }, /* 0x11ac - 0x11ae */
	{ 0x11b0, 0x0046 }, /* 0x11b0 - 0x11f5 */
	{ 0x1205, 0x0002 }, /* 0x1205 - 0x1206 */
	{ 0x120a, 0x000f }, /* 0x120a - 0x1218 */
	{ 0x1240, 0x000d }, /* 0x1240 - 0x124c */
	{ 0x1259, 0x0013 }, /* 0x1259 - 0x126b */
	{ 0x126f, 0x000f }, /* 0x126f - 0x127d */
	{ 0x1280, 0x000e }, /* 0x1280 - 0x128d */
	{ 0x128f, 0x004f }, /* 0x128f - 0x12dd */
	{ 0x12e0, 0x0013 }, /* 0x12e0 - 0x12f2 */
	{ 0x12f4, 0x000a }, /* 0x12f4 - 0x12fd */
	{ 0x1300, 0x001c }, /* 0x1300 - 0x131b */
	{ 0x1320, 0x001c }, /* 0x1320 - 0x133b */
	{ 0x1340, 0x001f }, /* 0x1340 - 0x135e */
	{ 0x1360, 0x0004 }, /* 0x1360 - 0x1363 */
	{ 0x1368, 0x0007 }, /* 0x1368 - 0x136e */
	{ 0x1370, 0x0009 }, /* 0x1370 - 0x1378 */
	{ 0x1380, 0x001a }, /* 0x1380 - 0x1399 */
	{ 0x13a0, 0x0004 }, /* 0x13a0 - 0x13a3 */
	{ 0x13e0, 0x0001 }, /* 0x13e0 - 0x13e0 */
	{ 0x13f3, 0x0003 }, /* 0x13f3 - 0x13f5 */
	{ 0x1400, 0x0004 }, /* 0x1400 - 0x1403 */
	{ 0x1405, 0x0011 }, /* 0x1405 - 0x1415 */
	{ 0x1430, 0x0002 }, /* 0x1430 - 0x1431 */
	{ 0x1439, 0x0004 }, /* 0x1439 - 0x143c */
	{ 0x143f, 0x0009 }, /* 0x143f - 0x1447 */
	{ 0x1600, 0x000e }, /* 0x1600 - 0x160d */
	{ 0x1610, 0x0005 }, /* 0x1610 - 0x1614 */
	{ 0x1620, 0x000d }, /* 0x1620 - 0x162c */
	{ 0x1630, 0x0001 }, /* 0x1630 - 0x1630 */
	{ 0x1632, 0x000d }, /* 0x1632 - 0x163e */
	{ 0x1640, 0x000d }, /* 0x1640 - 0x164c */
	{ 0x1650, 0x004c }, /* 0x1650 - 0x169b */
	{ 0x16a0, 0x000a }, /* 0x16a0 - 0x16a9 */
	{ 0x16b0, 0x000d }, /* 0x16b0 - 0x16bc */
	{ 0x16c0, 0x001a }, /* 0x16c0 - 0x16d9 */
	{ 0x16de, 0x0007 }, /* 0x16de - 0x16e4 */
	{ 0x16f0, 0x0023 }, /* 0x16f0 - 0x1712 */
	{ 0x1720, 0x0038 }, /* 0x1720 - 0x1757 */
	{ 0x1760, 0x000c }, /* 0x1760 - 0x176b */
	{ 0x1770, 0x0009 }, /* 0x1770 - 0x1778 */
	{ 0x1780, 0x0015 }, /* 0x1780 - 0x1794 */
	{ 0x17a0, 0x002f }, /* 0x17a0 - 0x17ce */
	{ 0x17d0, 0x003e }, /* 0x17d0 - 0x180d */
	{ 0x1810, 0x0005 }, /* 0x1810 - 0x1814 */
	{ 0x1820, 0x000d }, /* 0x1820 - 0x182c */
	{ 0x1830, 0x0001 }, /* 0x1830 - 0x1830 */
	{ 0x1832, 0x000d }, /* 0x1832 - 0x183e */
	{ 0x1840, 0x000d }, /* 0x1840 - 0x184c */
	{ 0x1850, 0x004c }, /* 0x1850 - 0x189b */
	{ 0x18a0, 0x000a }, /* 0x18a0 - 0x18a9 */
	{ 0x18b0, 0x000d }, /* 0x18b0 - 0x18bc */
	{ 0x18c0, 0x001a }, /* 0x18c0 - 0x18d9 */
	{ 0x18de, 0x0007 }, /* 0x18de - 0x18e4 */
	{ 0x18f0, 0x0023 }, /* 0x18f0 - 0x1912 */
	{ 0x1920, 0x0038 }, /* 0x1920 - 0x1957 */
	{ 0x1960, 0x000c }, /* 0x1960 - 0x196b */
	{ 0x1970, 0x0009 }, /* 0x1970 - 0x1978 */
	{ 0x1980, 0x0015 }, /* 0x1980 - 0x1994 */
	{ 0x19a0, 0x002f }, /* 0x19a0 - 0x19ce */
	{ 0x19d0, 0x003e }, /* 0x19d0 - 0x1a0d */
	{ 0x1a10, 0x0005 }, /* 0x1a10 - 0x1a14 */
	{ 0x1a20, 0x000d }, /* 0x1a20 - 0x1a2c */
	{ 0x1a30, 0x0001 }, /* 0x1a30 - 0x1a30 */
	{ 0x1a32, 0x000d }, /* 0x1a32 - 0x1a3e */
	{ 0x1a40, 0x000d }, /* 0x1a40 - 0x1a4c */
	{ 0x1a50, 0x004c }, /* 0x1a50 - 0x1a9b */
	{ 0x1aa0, 0x000a }, /* 0x1aa0 - 0x1aa9 */
	{ 0x1ab0, 0x000d }, /* 0x1ab0 - 0x1abc */
	{ 0x1ac0, 0x001a }, /* 0x1ac0 - 0x1ad9 */
	{ 0x1ade, 0x0007 }, /* 0x1ade - 0x1ae4 */
	{ 0x1af0, 0x0023 }, /* 0x1af0 - 0x1b12 */
	{ 0x1b20, 0x0038 }, /* 0x1b20 - 0x1b57 */
	{ 0x1b60, 0x000c }, /* 0x1b60 - 0x1b6b */
	{ 0x1b70, 0x0009 }, /* 0x1b70 - 0x1b78 */
	{ 0x1b80, 0x0015 }, /* 0x1b80 - 0x1b94 */
	{ 0x1ba0, 0x002f }, /* 0x1ba0 - 0x1bce */
	{ 0x1bd0, 0x003e }, /* 0x1bd0 - 0x1c0d */
	{ 0x1c10, 0x0005 }, /* 0x1c10 - 0x1c14 */
	{ 0x1c20, 0x000d }, /* 0x1c20 - 0x1c2c */
	{ 0x1c30, 0x0001 }, /* 0x1c30 - 0x1c30 */
	{ 0x1c32, 0x000d }, /* 0x1c32 - 0x1c3e */
	{ 0x1c40, 0x000d }, /* 0x1c40 - 0x1c4c */
	{ 0x1c50, 0x004c }, /* 0x1c50 - 0x1c9b */
	{ 0x1ca0, 0x000a }, /* 0x1ca0 - 0x1ca9 */
	{ 0x1cb0, 0x000d }, /* 0x1cb0 - 0x1cbc */
	{ 0x1cc0, 0x001a }, /* 0x1cc0 - 0x1cd9 */
	{ 0x1cde, 0x0007 }, /* 0x1cde - 0x1ce4 */
	{ 0x1cf0, 0x0023 }, /* 0x1cf0 - 0x1d12 */
	{ 0x1d20, 0x0038 }, /* 0x1d20 - 0x1d57 */
	{ 0x1d60, 0x000c }, /* 0x1d60 - 0x1d6b */
	{ 0x1d70, 0x0009 }, /* 0x1d70 - 0x1d78 */
	{ 0x1d80, 0x0015 }, /* 0x1d80 - 0x1d94 */
	{ 0x1da0, 0x002f }, /* 0x1da0 - 0x1dce */
	{ 0x1dd0, 0x0030 }, /* 0x1dd0 - 0x1dff */
	{ 0x1e10, 0x0002 }, /* 0x1e10 - 0x1e11 */
	{ 0x2600, 0x0029 }, /* 0x2600 - 0x2628 */
	{ 0x2639, 0x0007 }, /* 0x2639 - 0x263f */
	{ 0x26a7, 0x0005 }, /* 0x26a7 - 0x26ab */
	{ 0x26d6, 0x0002 }, /* 0x26d6 - 0x26d7 */
	{ 0x2700, 0x0003 }, /* 0x2700 - 0x2702 */
	{ 0x270f, 0x000f }, /* 0x270f - 0x271d */
	{ 0x2720, 0x000e }, /* 0x2720 - 0x272d */
	{ 0x2732, 0x0004 }, /* 0x2732 - 0x2735 */
	{ 0x2800, 0x000f }, /* 0x2800 - 0x280e */
	{ 0x28a7, 0x0005 }, /* 0x28a7 - 0x28ab */
	{ 0x28d6, 0x0002 }, /* 0x28d6 - 0x28d7 */
	{ 0x2900, 0x0003 }, /* 0x2900 - 0x2902 */
	{ 0x2932, 0x0004 }, /* 0x2932 - 0x2935 */
	{ 0x2a00, 0x000f }, /* 0x2a00 - 0x2a0e */
	{ 0x2aa7, 0x0005 }, /* 0x2aa7 - 0x2aab */
	{ 0x2ad6, 0x0002 }, /* 0x2ad6 - 0x2ad7 */
	{ 0x2b00, 0x0003 }, /* 0x2b00 - 0x2b02 */
	{ 0x2b32, 0x0004 }, /* 0x2b32 - 0x2b35 */
	{ 0x2c00, 0x000f }, /* 0x2c00 - 0x2c0e */
	{ 0x2ca7, 0x0005 }, /* 0x2ca7 - 0x2cab */
	{ 0x2cd6, 0x0002 }, /* 0x2cd6 - 0x2cd7 */
	{ 0x2d00, 0x0003 }, /* 0x2d00 - 0x2d02 */
	{ 0x2d32, 0x0004 }, /* 0x2d32 - 0x2d35 */
	{ 0x3580, 0x0007 }, /* 0x3580 - 0x3586 */
	{ 0x3590, 0x0003 }, /* 0x3590 - 0x3592 */
	{ 0x3780, 0x0007 }, /* 0x3780 - 0x3786 */
	{ 0x3790, 0x0003 }, /* 0x3790 - 0x3792 */
	{ 0x3980, 0x0007 }, /* 0x3980 - 0x3986 */
	{ 0x3990, 0x0003 }, /* 0x3990 - 0x3992 */
	{ 0x3b80, 0x0007 }, /* 0x3b80 - 0x3b86 */
	{ 0x3b90, 0x0003 }  /* 0x3b90 - 0x3b92 */
};

static CONST d11phyregs_list_t phyreg_list_r132[] = {
	{ 0x0000, 0x0006 }, /* 0x0000 - 0x0005 */
	{ 0x0009, 0x0004 }, /* 0x0009 - 0x000c */
	{ 0x0012, 0x0025 }, /* 0x0012 - 0x0036 */
	{ 0x0038, 0x00e6 }, /* 0x0038 - 0x011d */
	{ 0x0120, 0x005a }, /* 0x0120 - 0x0179 */
	{ 0x017c, 0x0005 }, /* 0x017c - 0x0180 */
	{ 0x0195, 0x0016 }, /* 0x0195 - 0x01aa */
	{ 0x01ad, 0x0066 }, /* 0x01ad - 0x0212 */
	{ 0x0218, 0x0025 }, /* 0x0218 - 0x023c */
	{ 0x0240, 0x00ac }, /* 0x0240 - 0x02eb */
	{ 0x02ee, 0x001b }, /* 0x02ee - 0x0308 */
	{ 0x030e, 0x000c }, /* 0x030e - 0x0319 */
	{ 0x031e, 0x001f }, /* 0x031e - 0x033c */
	{ 0x033f, 0x0005 }, /* 0x033f - 0x0343 */
	{ 0x034d, 0x0018 }, /* 0x034d - 0x0364 */
	{ 0x0366, 0x0021 }, /* 0x0366 - 0x0386 */
	{ 0x038a, 0x0011 }, /* 0x038a - 0x039a */
	{ 0x039c, 0x0080 }, /* 0x039c - 0x041b */
	{ 0x041e, 0x0066 }, /* 0x041e - 0x0483 */
	{ 0x048e, 0x0001 }, /* 0x048e - 0x048e */
	{ 0x0490, 0x0001 }, /* 0x0490 - 0x0490 */
	{ 0x0496, 0x0001 }, /* 0x0496 - 0x0496 */
	{ 0x049d, 0x0001 }, /* 0x049d - 0x049d */
	{ 0x04a0, 0x0003 }, /* 0x04a0 - 0x04a2 */
	{ 0x04a4, 0x0006 }, /* 0x04a4 - 0x04a9 */
	{ 0x04b0, 0x0006 }, /* 0x04b0 - 0x04b5 */
	{ 0x04b8, 0x0004 }, /* 0x04b8 - 0x04bb */
	{ 0x04c0, 0x000c }, /* 0x04c0 - 0x04cb */
	{ 0x04d6, 0x0011 }, /* 0x04d6 - 0x04e6 */
	{ 0x04f4, 0x0003 }, /* 0x04f4 - 0x04f6 */
	{ 0x04f8, 0x0003 }, /* 0x04f8 - 0x04fa */
	{ 0x0500, 0x0003 }, /* 0x0500 - 0x0502 */
	{ 0x0510, 0x001d }, /* 0x0510 - 0x052c */
	{ 0x0550, 0x0018 }, /* 0x0550 - 0x0567 */
	{ 0x0570, 0x0005 }, /* 0x0570 - 0x0574 */
	{ 0x0578, 0x003e }, /* 0x0578 - 0x05b5 */
	{ 0x05bc, 0x0003 }, /* 0x05bc - 0x05be */
	{ 0x05c0, 0x000c }, /* 0x05c0 - 0x05cb */
	{ 0x05d0, 0x000a }, /* 0x05d0 - 0x05d9 */
	{ 0x05e0, 0x0019 }, /* 0x05e0 - 0x05f8 */
	{ 0x0600, 0x0010 }, /* 0x0600 - 0x060f */
	{ 0x0629, 0x0004 }, /* 0x0629 - 0x062c */
	{ 0x063f, 0x0015 }, /* 0x063f - 0x0653 */
	{ 0x0660, 0x0003 }, /* 0x0660 - 0x0662 */
	{ 0x0665, 0x0001 }, /* 0x0665 - 0x0665 */
	{ 0x066a, 0x0002 }, /* 0x066a - 0x066b */
	{ 0x0670, 0x001e }, /* 0x0670 - 0x068d */
	{ 0x0690, 0x0007 }, /* 0x0690 - 0x0696 */
	{ 0x0698, 0x0021 }, /* 0x0698 - 0x06b8 */
	{ 0x06ba, 0x0015 }, /* 0x06ba - 0x06ce */
	{ 0x06d0, 0x003e }, /* 0x06d0 - 0x070d */
	{ 0x0710, 0x001e }, /* 0x0710 - 0x072d */
	{ 0x0730, 0x0010 }, /* 0x0730 - 0x073f */
	{ 0x0741, 0x001f }, /* 0x0741 - 0x075f */
	{ 0x0767, 0x0001 }, /* 0x0767 - 0x0767 */
	{ 0x0769, 0x0001 }, /* 0x0769 - 0x0769 */
	{ 0x0770, 0x000e }, /* 0x0770 - 0x077d */
	{ 0x0780, 0x001a }, /* 0x0780 - 0x0799 */
	{ 0x079d, 0x0001 }, /* 0x079d - 0x079d */
	{ 0x07a0, 0x0040 }, /* 0x07a0 - 0x07df */
	{ 0x07e2, 0x0001 }, /* 0x07e2 - 0x07e2 */
	{ 0x07eb, 0x0052 }, /* 0x07eb - 0x083c */
	{ 0x083f, 0x0015 }, /* 0x083f - 0x0853 */
	{ 0x0858, 0x0002 }, /* 0x0858 - 0x0859 */
	{ 0x0860, 0x0003 }, /* 0x0860 - 0x0862 */
	{ 0x0865, 0x0001 }, /* 0x0865 - 0x0865 */
	{ 0x086a, 0x0002 }, /* 0x086a - 0x086b */
	{ 0x0870, 0x001e }, /* 0x0870 - 0x088d */
	{ 0x0890, 0x0007 }, /* 0x0890 - 0x0896 */
	{ 0x0898, 0x0021 }, /* 0x0898 - 0x08b8 */
	{ 0x08ba, 0x0015 }, /* 0x08ba - 0x08ce */
	{ 0x08d0, 0x003e }, /* 0x08d0 - 0x090d */
	{ 0x0910, 0x001e }, /* 0x0910 - 0x092d */
	{ 0x0930, 0x0010 }, /* 0x0930 - 0x093f */
	{ 0x0941, 0x001f }, /* 0x0941 - 0x095f */
	{ 0x0967, 0x0001 }, /* 0x0967 - 0x0967 */
	{ 0x0969, 0x0001 }, /* 0x0969 - 0x0969 */
	{ 0x0970, 0x000e }, /* 0x0970 - 0x097d */
	{ 0x0980, 0x001a }, /* 0x0980 - 0x0999 */
	{ 0x099d, 0x0001 }, /* 0x099d - 0x099d */
	{ 0x09a0, 0x0040 }, /* 0x09a0 - 0x09df */
	{ 0x09e2, 0x0001 }, /* 0x09e2 - 0x09e2 */
	{ 0x09eb, 0x0035 }, /* 0x09eb - 0x0a1f */
	{ 0x0a29, 0x0004 }, /* 0x0a29 - 0x0a2c */
	{ 0x0a30, 0x0003 }, /* 0x0a30 - 0x0a32 */
	{ 0x0a3f, 0x0015 }, /* 0x0a3f - 0x0a53 */
	{ 0x0a60, 0x0003 }, /* 0x0a60 - 0x0a62 */
	{ 0x0a65, 0x0001 }, /* 0x0a65 - 0x0a65 */
	{ 0x0a6a, 0x0002 }, /* 0x0a6a - 0x0a6b */
	{ 0x0a70, 0x001e }, /* 0x0a70 - 0x0a8d */
	{ 0x0a90, 0x0007 }, /* 0x0a90 - 0x0a96 */
	{ 0x0a98, 0x0021 }, /* 0x0a98 - 0x0ab8 */
	{ 0x0aba, 0x0015 }, /* 0x0aba - 0x0ace */
	{ 0x0ad0, 0x003e }, /* 0x0ad0 - 0x0b0d */
	{ 0x0b10, 0x001e }, /* 0x0b10 - 0x0b2d */
	{ 0x0b30, 0x0010 }, /* 0x0b30 - 0x0b3f */
	{ 0x0b41, 0x001f }, /* 0x0b41 - 0x0b5f */
	{ 0x0b67, 0x0001 }, /* 0x0b67 - 0x0b67 */
	{ 0x0b69, 0x0001 }, /* 0x0b69 - 0x0b69 */
	{ 0x0b70, 0x000e }, /* 0x0b70 - 0x0b7d */
	{ 0x0b80, 0x001a }, /* 0x0b80 - 0x0b99 */
	{ 0x0b9d, 0x0001 }, /* 0x0b9d - 0x0b9d */
	{ 0x0ba0, 0x0040 }, /* 0x0ba0 - 0x0bdf */
	{ 0x0be2, 0x0001 }, /* 0x0be2 - 0x0be2 */
	{ 0x0beb, 0x0035 }, /* 0x0beb - 0x0c1f */
	{ 0x0c29, 0x0004 }, /* 0x0c29 - 0x0c2c */
	{ 0x0c30, 0x0003 }, /* 0x0c30 - 0x0c32 */
	{ 0x0c3f, 0x0015 }, /* 0x0c3f - 0x0c53 */
	{ 0x0c60, 0x0003 }, /* 0x0c60 - 0x0c62 */
	{ 0x0c65, 0x0001 }, /* 0x0c65 - 0x0c65 */
	{ 0x0c6a, 0x0002 }, /* 0x0c6a - 0x0c6b */
	{ 0x0c70, 0x001e }, /* 0x0c70 - 0x0c8d */
	{ 0x0c90, 0x0007 }, /* 0x0c90 - 0x0c96 */
	{ 0x0c98, 0x0021 }, /* 0x0c98 - 0x0cb8 */
	{ 0x0cba, 0x0015 }, /* 0x0cba - 0x0cce */
	{ 0x0cd0, 0x003e }, /* 0x0cd0 - 0x0d0d */
	{ 0x0d10, 0x001e }, /* 0x0d10 - 0x0d2d */
	{ 0x0d30, 0x0010 }, /* 0x0d30 - 0x0d3f */
	{ 0x0d41, 0x001f }, /* 0x0d41 - 0x0d5f */
	{ 0x0d67, 0x0001 }, /* 0x0d67 - 0x0d67 */
	{ 0x0d69, 0x0001 }, /* 0x0d69 - 0x0d69 */
	{ 0x0d70, 0x000e }, /* 0x0d70 - 0x0d7d */
	{ 0x0d80, 0x001a }, /* 0x0d80 - 0x0d99 */
	{ 0x0d9d, 0x0001 }, /* 0x0d9d - 0x0d9d */
	{ 0x0da0, 0x0040 }, /* 0x0da0 - 0x0ddf */
	{ 0x0de2, 0x0001 }, /* 0x0de2 - 0x0de2 */
	{ 0x0deb, 0x0015 }, /* 0x0deb - 0x0dff */
	{ 0x0e10, 0x0010 }, /* 0x0e10 - 0x0e1f */
	{ 0x0e30, 0x0003 }, /* 0x0e30 - 0x0e32 */
	{ 0x0e50, 0x0001 }, /* 0x0e50 - 0x0e50 */
	{ 0x0e52, 0x000b }, /* 0x0e52 - 0x0e5c */
	{ 0x0eb0, 0x001d }, /* 0x0eb0 - 0x0ecc */
	{ 0x0eee, 0x000a }, /* 0x0eee - 0x0ef7 */
	{ 0x0efe, 0x000c }, /* 0x0efe - 0x0f09 */
	{ 0x1000, 0x0010 }, /* 0x1000 - 0x100f */
	{ 0x1020, 0x000f }, /* 0x1020 - 0x102e */
	{ 0x1030, 0x0024 }, /* 0x1030 - 0x1053 */
	{ 0x1060, 0x0009 }, /* 0x1060 - 0x1068 */
	{ 0x1070, 0x0010 }, /* 0x1070 - 0x107f */
	{ 0x1081, 0x000e }, /* 0x1081 - 0x108e */
	{ 0x1090, 0x0018 }, /* 0x1090 - 0x10a7 */
	{ 0x10b0, 0x0043 }, /* 0x10b0 - 0x10f2 */
	{ 0x10f4, 0x003f }, /* 0x10f4 - 0x1132 */
	{ 0x1136, 0x0001 }, /* 0x1136 - 0x1136 */
	{ 0x1138, 0x003e }, /* 0x1138 - 0x1175 */
	{ 0x1178, 0x0008 }, /* 0x1178 - 0x117f */
	{ 0x11ac, 0x004a }, /* 0x11ac - 0x11f5 */
	{ 0x1205, 0x0002 }, /* 0x1205 - 0x1206 */
	{ 0x120a, 0x000f }, /* 0x120a - 0x1218 */
	{ 0x123f, 0x000e }, /* 0x123f - 0x124c */
	{ 0x1259, 0x0015 }, /* 0x1259 - 0x126d */
	{ 0x126f, 0x0010 }, /* 0x126f - 0x127e */
	{ 0x1280, 0x000e }, /* 0x1280 - 0x128d */
	{ 0x128f, 0x004f }, /* 0x128f - 0x12dd */
	{ 0x12df, 0x0014 }, /* 0x12df - 0x12f2 */
	{ 0x12f4, 0x006b }, /* 0x12f4 - 0x135e */
	{ 0x1360, 0x005a }, /* 0x1360 - 0x13b9 */
	{ 0x13c0, 0x0005 }, /* 0x13c0 - 0x13c4 */
	{ 0x13c6, 0x0005 }, /* 0x13c6 - 0x13ca */
	{ 0x13d0, 0x0006 }, /* 0x13d0 - 0x13d5 */
	{ 0x13e0, 0x0001 }, /* 0x13e0 - 0x13e0 */
	{ 0x13f3, 0x0003 }, /* 0x13f3 - 0x13f5 */
	{ 0x1400, 0x0001 }, /* 0x1400 - 0x1400 */
	{ 0x140b, 0x0001 }, /* 0x140b - 0x140b */
	{ 0x1410, 0x0003 }, /* 0x1410 - 0x1412 */
	{ 0x1416, 0x0009 }, /* 0x1416 - 0x141e */
	{ 0x1430, 0x0002 }, /* 0x1430 - 0x1431 */
	{ 0x1439, 0x0004 }, /* 0x1439 - 0x143c */
	{ 0x143f, 0x0017 }, /* 0x143f - 0x1455 */
	{ 0x1460, 0x0005 }, /* 0x1460 - 0x1464 */
	{ 0x1468, 0x0005 }, /* 0x1468 - 0x146c */
	{ 0x1470, 0x0007 }, /* 0x1470 - 0x1476 */
	{ 0x1480, 0x000d }, /* 0x1480 - 0x148c */
	{ 0x1490, 0x0002 }, /* 0x1490 - 0x1491 */
	{ 0x14a0, 0x000b }, /* 0x14a0 - 0x14aa */
	{ 0x14ff, 0x000d }, /* 0x14ff - 0x150b */
	{ 0x1600, 0x0015 }, /* 0x1600 - 0x1614 */
	{ 0x1620, 0x000d }, /* 0x1620 - 0x162c */
	{ 0x1630, 0x001f }, /* 0x1630 - 0x164e */
	{ 0x1650, 0x005f }, /* 0x1650 - 0x16ae */
	{ 0x16b0, 0x002a }, /* 0x16b0 - 0x16d9 */
	{ 0x16de, 0x0007 }, /* 0x16de - 0x16e4 */
	{ 0x16f0, 0x0023 }, /* 0x16f0 - 0x1712 */
	{ 0x1720, 0x0039 }, /* 0x1720 - 0x1758 */
	{ 0x1760, 0x000e }, /* 0x1760 - 0x176d */
	{ 0x1770, 0x0009 }, /* 0x1770 - 0x1778 */
	{ 0x1780, 0x0015 }, /* 0x1780 - 0x1794 */
	{ 0x17a0, 0x002f }, /* 0x17a0 - 0x17ce */
	{ 0x17d0, 0x0045 }, /* 0x17d0 - 0x1814 */
	{ 0x1820, 0x000d }, /* 0x1820 - 0x182c */
	{ 0x1830, 0x001f }, /* 0x1830 - 0x184e */
	{ 0x1850, 0x005f }, /* 0x1850 - 0x18ae */
	{ 0x18b0, 0x002a }, /* 0x18b0 - 0x18d9 */
	{ 0x18de, 0x0007 }, /* 0x18de - 0x18e4 */
	{ 0x18f0, 0x0023 }, /* 0x18f0 - 0x1912 */
	{ 0x1920, 0x0039 }, /* 0x1920 - 0x1958 */
	{ 0x1960, 0x000e }, /* 0x1960 - 0x196d */
	{ 0x1970, 0x0009 }, /* 0x1970 - 0x1978 */
	{ 0x1980, 0x0015 }, /* 0x1980 - 0x1994 */
	{ 0x19a0, 0x002f }, /* 0x19a0 - 0x19ce */
	{ 0x19d0, 0x0045 }, /* 0x19d0 - 0x1a14 */
	{ 0x1a20, 0x000d }, /* 0x1a20 - 0x1a2c */
	{ 0x1a30, 0x001f }, /* 0x1a30 - 0x1a4e */
	{ 0x1a50, 0x005f }, /* 0x1a50 - 0x1aae */
	{ 0x1ab0, 0x002a }, /* 0x1ab0 - 0x1ad9 */
	{ 0x1ade, 0x0007 }, /* 0x1ade - 0x1ae4 */
	{ 0x1af0, 0x0023 }, /* 0x1af0 - 0x1b12 */
	{ 0x1b20, 0x0039 }, /* 0x1b20 - 0x1b58 */
	{ 0x1b60, 0x000e }, /* 0x1b60 - 0x1b6d */
	{ 0x1b70, 0x0009 }, /* 0x1b70 - 0x1b78 */
	{ 0x1b80, 0x0015 }, /* 0x1b80 - 0x1b94 */
	{ 0x1ba0, 0x002f }, /* 0x1ba0 - 0x1bce */
	{ 0x1bd0, 0x0045 }, /* 0x1bd0 - 0x1c14 */
	{ 0x1c20, 0x000d }, /* 0x1c20 - 0x1c2c */
	{ 0x1c30, 0x001f }, /* 0x1c30 - 0x1c4e */
	{ 0x1c50, 0x005f }, /* 0x1c50 - 0x1cae */
	{ 0x1cb0, 0x002a }, /* 0x1cb0 - 0x1cd9 */
	{ 0x1cde, 0x0007 }, /* 0x1cde - 0x1ce4 */
	{ 0x1cf0, 0x0023 }, /* 0x1cf0 - 0x1d12 */
	{ 0x1d20, 0x0039 }, /* 0x1d20 - 0x1d58 */
	{ 0x1d60, 0x000e }, /* 0x1d60 - 0x1d6d */
	{ 0x1d70, 0x0009 }, /* 0x1d70 - 0x1d78 */
	{ 0x1d80, 0x0015 }, /* 0x1d80 - 0x1d94 */
	{ 0x1da0, 0x002f }, /* 0x1da0 - 0x1dce */
	{ 0x1dd0, 0x0030 }, /* 0x1dd0 - 0x1dff */
	{ 0x1e10, 0x0002 }, /* 0x1e10 - 0x1e11 */
	{ 0x1e20, 0x0080 }, /* 0x1e20 - 0x1e9f */
	{ 0x1f00, 0x0018 }, /* 0x1f00 - 0x1f17 */
	{ 0x1f20, 0x0018 }, /* 0x1f20 - 0x1f37 */
	{ 0x1ff0, 0x0002 }, /* 0x1ff0 - 0x1ff1 */
	{ 0x2000, 0x0001 }, /* 0x2000 - 0x2000 */
	{ 0x2020, 0x0080 }, /* 0x2020 - 0x209f */
	{ 0x20b0, 0x000e }, /* 0x20b0 - 0x20bd */
	{ 0x20c0, 0x0002 }, /* 0x20c0 - 0x20c1 */
	{ 0x20d0, 0x0008 }, /* 0x20d0 - 0x20d7 */
	{ 0x20e0, 0x0014 }, /* 0x20e0 - 0x20f3 */
	{ 0x21f0, 0x0002 }, /* 0x21f0 - 0x21f1 */
	{ 0x2220, 0x0080 }, /* 0x2220 - 0x229f */
	{ 0x22b0, 0x000e }, /* 0x22b0 - 0x22bd */
	{ 0x22c0, 0x0002 }, /* 0x22c0 - 0x22c1 */
	{ 0x22e0, 0x0014 }, /* 0x22e0 - 0x22f3 */
	{ 0x23f0, 0x0002 }, /* 0x23f0 - 0x23f1 */
	{ 0x2420, 0x0080 }, /* 0x2420 - 0x249f */
	{ 0x24b0, 0x000e }, /* 0x24b0 - 0x24bd */
	{ 0x24c0, 0x0002 }, /* 0x24c0 - 0x24c1 */
	{ 0x24e0, 0x0014 }, /* 0x24e0 - 0x24f3 */
	{ 0x25f0, 0x0002 }, /* 0x25f0 - 0x25f1 */
	{ 0x2600, 0x0029 }, /* 0x2600 - 0x2628 */
	{ 0x2639, 0x0009 }, /* 0x2639 - 0x2641 */
	{ 0x26a7, 0x0005 }, /* 0x26a7 - 0x26ab */
	{ 0x26b0, 0x000e }, /* 0x26b0 - 0x26bd */
	{ 0x26c0, 0x0002 }, /* 0x26c0 - 0x26c1 */
	{ 0x26d6, 0x0002 }, /* 0x26d6 - 0x26d7 */
	{ 0x26e0, 0x0014 }, /* 0x26e0 - 0x26f3 */
	{ 0x2700, 0x0003 }, /* 0x2700 - 0x2702 */
	{ 0x270f, 0x000f }, /* 0x270f - 0x271d */
	{ 0x2720, 0x000f }, /* 0x2720 - 0x272e */
	{ 0x2730, 0x000e }, /* 0x2730 - 0x273d */
	{ 0x2800, 0x000f }, /* 0x2800 - 0x280e */
	{ 0x28a7, 0x0005 }, /* 0x28a7 - 0x28ab */
	{ 0x28d6, 0x0002 }, /* 0x28d6 - 0x28d7 */
	{ 0x2900, 0x0003 }, /* 0x2900 - 0x2902 */
	{ 0x2920, 0x000f }, /* 0x2920 - 0x292e */
	{ 0x2a00, 0x000f }, /* 0x2a00 - 0x2a0e */
	{ 0x2aa7, 0x0005 }, /* 0x2aa7 - 0x2aab */
	{ 0x2ad6, 0x0002 }, /* 0x2ad6 - 0x2ad7 */
	{ 0x2b00, 0x0003 }, /* 0x2b00 - 0x2b02 */
	{ 0x2b20, 0x000f }, /* 0x2b20 - 0x2b2e */
	{ 0x2c00, 0x000f }, /* 0x2c00 - 0x2c0e */
	{ 0x2ca7, 0x0005 }, /* 0x2ca7 - 0x2cab */
	{ 0x2cd6, 0x0002 }, /* 0x2cd6 - 0x2cd7 */
	{ 0x2d00, 0x0003 }, /* 0x2d00 - 0x2d02 */
	{ 0x2d20, 0x000f }, /* 0x2d20 - 0x2d2e */
	{ 0x3580, 0x000a }, /* 0x3580 - 0x3589 */
	{ 0x3590, 0x0003 }, /* 0x3590 - 0x3592 */
	{ 0x3780, 0x000a }, /* 0x3780 - 0x3789 */
	{ 0x3790, 0x0003 }, /* 0x3790 - 0x3792 */
	{ 0x3980, 0x000a }, /* 0x3980 - 0x3989 */
	{ 0x3990, 0x0003 }, /* 0x3990 - 0x3992 */
	{ 0x3b80, 0x000a }, /* 0x3b80 - 0x3b89 */
	{ 0x3b90, 0x0003 }  /* 0x3b90 - 0x3b92 */
};

static CONST d11phyregs_list_t phyreg_list_r133[] = {
	{ 0x0000, 0x0006 }, /* 0x0000 - 0x0005 */
	{ 0x0009, 0x0004 }, /* 0x0009 - 0x000c */
	{ 0x0012, 0x0025 }, /* 0x0012 - 0x0036 */
	{ 0x0038, 0x0142 }, /* 0x0038 - 0x0179 */
	{ 0x017c, 0x0008 }, /* 0x017c - 0x0183 */
	{ 0x0189, 0x0002 }, /* 0x0189 - 0x018a */
	{ 0x0195, 0x0016 }, /* 0x0195 - 0x01aa */
	{ 0x01ad, 0x0057 }, /* 0x01ad - 0x0203 */
	{ 0x020d, 0x0006 }, /* 0x020d - 0x0212 */
	{ 0x0219, 0x0019 }, /* 0x0219 - 0x0231 */
	{ 0x023a, 0x0004 }, /* 0x023a - 0x023d */
	{ 0x0240, 0x0002 }, /* 0x0240 - 0x0241 */
	{ 0x024f, 0x009d }, /* 0x024f - 0x02eb */
	{ 0x02ed, 0x0019 }, /* 0x02ed - 0x0305 */
	{ 0x0307, 0x0002 }, /* 0x0307 - 0x0308 */
	{ 0x030e, 0x000c }, /* 0x030e - 0x0319 */
	{ 0x031d, 0x0020 }, /* 0x031d - 0x033c */
	{ 0x033f, 0x0009 }, /* 0x033f - 0x0347 */
	{ 0x034d, 0x0018 }, /* 0x034d - 0x0364 */
	{ 0x0366, 0x0021 }, /* 0x0366 - 0x0386 */
	{ 0x038a, 0x0011 }, /* 0x038a - 0x039a */
	{ 0x039c, 0x0080 }, /* 0x039c - 0x041b */
	{ 0x041e, 0x0066 }, /* 0x041e - 0x0483 */
	{ 0x048e, 0x0001 }, /* 0x048e - 0x048e */
	{ 0x0490, 0x0001 }, /* 0x0490 - 0x0490 */
	{ 0x0496, 0x0001 }, /* 0x0496 - 0x0496 */
	{ 0x049d, 0x0001 }, /* 0x049d - 0x049d */
	{ 0x04a0, 0x0003 }, /* 0x04a0 - 0x04a2 */
	{ 0x04a4, 0x0006 }, /* 0x04a4 - 0x04a9 */
	{ 0x04b0, 0x0006 }, /* 0x04b0 - 0x04b5 */
	{ 0x04b8, 0x0004 }, /* 0x04b8 - 0x04bb */
	{ 0x04c0, 0x000d }, /* 0x04c0 - 0x04cc */
	{ 0x04d6, 0x0011 }, /* 0x04d6 - 0x04e6 */
	{ 0x04f4, 0x0003 }, /* 0x04f4 - 0x04f6 */
	{ 0x04f8, 0x0003 }, /* 0x04f8 - 0x04fa */
	{ 0x0500, 0x0003 }, /* 0x0500 - 0x0502 */
	{ 0x0510, 0x001d }, /* 0x0510 - 0x052c */
	{ 0x054a, 0x0002 }, /* 0x054a - 0x054b */
	{ 0x0550, 0x0018 }, /* 0x0550 - 0x0567 */
	{ 0x0570, 0x0005 }, /* 0x0570 - 0x0574 */
	{ 0x0578, 0x003e }, /* 0x0578 - 0x05b5 */
	{ 0x05bc, 0x0010 }, /* 0x05bc - 0x05cb */
	{ 0x05d0, 0x000a }, /* 0x05d0 - 0x05d9 */
	{ 0x05e0, 0x0019 }, /* 0x05e0 - 0x05f8 */
	{ 0x0600, 0x0010 }, /* 0x0600 - 0x060f */
	{ 0x0629, 0x0004 }, /* 0x0629 - 0x062c */
	{ 0x063f, 0x0015 }, /* 0x063f - 0x0653 */
	{ 0x0660, 0x0003 }, /* 0x0660 - 0x0662 */
	{ 0x0665, 0x0001 }, /* 0x0665 - 0x0665 */
	{ 0x066a, 0x0002 }, /* 0x066a - 0x066b */
	{ 0x0670, 0x001e }, /* 0x0670 - 0x068d */
	{ 0x0690, 0x0007 }, /* 0x0690 - 0x0696 */
	{ 0x0698, 0x0021 }, /* 0x0698 - 0x06b8 */
	{ 0x06ba, 0x0015 }, /* 0x06ba - 0x06ce */
	{ 0x06d0, 0x003a }, /* 0x06d0 - 0x0709 */
	{ 0x070d, 0x0021 }, /* 0x070d - 0x072d */
	{ 0x0730, 0x0010 }, /* 0x0730 - 0x073f */
	{ 0x0741, 0x001f }, /* 0x0741 - 0x075f */
	{ 0x0767, 0x0001 }, /* 0x0767 - 0x0767 */
	{ 0x0769, 0x0001 }, /* 0x0769 - 0x0769 */
	{ 0x0770, 0x000e }, /* 0x0770 - 0x077d */
	{ 0x0780, 0x001a }, /* 0x0780 - 0x0799 */
	{ 0x079d, 0x0001 }, /* 0x079d - 0x079d */
	{ 0x07a0, 0x0040 }, /* 0x07a0 - 0x07df */
	{ 0x07e2, 0x0001 }, /* 0x07e2 - 0x07e2 */
	{ 0x07eb, 0x0052 }, /* 0x07eb - 0x083c */
	{ 0x083f, 0x0015 }, /* 0x083f - 0x0853 */
	{ 0x0858, 0x000b }, /* 0x0858 - 0x0862 */
	{ 0x0865, 0x0001 }, /* 0x0865 - 0x0865 */
	{ 0x086a, 0x0002 }, /* 0x086a - 0x086b */
	{ 0x0870, 0x001e }, /* 0x0870 - 0x088d */
	{ 0x0890, 0x0007 }, /* 0x0890 - 0x0896 */
	{ 0x0898, 0x0021 }, /* 0x0898 - 0x08b8 */
	{ 0x08ba, 0x0015 }, /* 0x08ba - 0x08ce */
	{ 0x08d0, 0x003a }, /* 0x08d0 - 0x0909 */
	{ 0x090d, 0x0021 }, /* 0x090d - 0x092d */
	{ 0x0930, 0x0010 }, /* 0x0930 - 0x093f */
	{ 0x0941, 0x001f }, /* 0x0941 - 0x095f */
	{ 0x0967, 0x0001 }, /* 0x0967 - 0x0967 */
	{ 0x0969, 0x0001 }, /* 0x0969 - 0x0969 */
	{ 0x0970, 0x000e }, /* 0x0970 - 0x097d */
	{ 0x0980, 0x001a }, /* 0x0980 - 0x0999 */
	{ 0x099d, 0x0001 }, /* 0x099d - 0x099d */
	{ 0x09a0, 0x0040 }, /* 0x09a0 - 0x09df */
	{ 0x09e2, 0x0001 }, /* 0x09e2 - 0x09e2 */
	{ 0x09eb, 0x003a }, /* 0x09eb - 0x0a24 */
	{ 0x0a29, 0x0004 }, /* 0x0a29 - 0x0a2c */
	{ 0x0a30, 0x0003 }, /* 0x0a30 - 0x0a32 */
	{ 0x0a3f, 0x0015 }, /* 0x0a3f - 0x0a53 */
	{ 0x0a60, 0x0003 }, /* 0x0a60 - 0x0a62 */
	{ 0x0a65, 0x0001 }, /* 0x0a65 - 0x0a65 */
	{ 0x0a6a, 0x0002 }, /* 0x0a6a - 0x0a6b */
	{ 0x0a70, 0x001e }, /* 0x0a70 - 0x0a8d */
	{ 0x0a90, 0x0007 }, /* 0x0a90 - 0x0a96 */
	{ 0x0a98, 0x0021 }, /* 0x0a98 - 0x0ab8 */
	{ 0x0aba, 0x0015 }, /* 0x0aba - 0x0ace */
	{ 0x0ad0, 0x003a }, /* 0x0ad0 - 0x0b09 */
	{ 0x0b0d, 0x0021 }, /* 0x0b0d - 0x0b2d */
	{ 0x0b30, 0x0010 }, /* 0x0b30 - 0x0b3f */
	{ 0x0b41, 0x001f }, /* 0x0b41 - 0x0b5f */
	{ 0x0b67, 0x0001 }, /* 0x0b67 - 0x0b67 */
	{ 0x0b69, 0x0001 }, /* 0x0b69 - 0x0b69 */
	{ 0x0b70, 0x000e }, /* 0x0b70 - 0x0b7d */
	{ 0x0b80, 0x001a }, /* 0x0b80 - 0x0b99 */
	{ 0x0b9d, 0x0001 }, /* 0x0b9d - 0x0b9d */
	{ 0x0ba0, 0x0040 }, /* 0x0ba0 - 0x0bdf */
	{ 0x0be2, 0x0001 }, /* 0x0be2 - 0x0be2 */
	{ 0x0beb, 0x0035 }, /* 0x0beb - 0x0c1f */
	{ 0x0c29, 0x0004 }, /* 0x0c29 - 0x0c2c */
	{ 0x0c30, 0x0003 }, /* 0x0c30 - 0x0c32 */
	{ 0x0c3f, 0x0015 }, /* 0x0c3f - 0x0c53 */
	{ 0x0c60, 0x0003 }, /* 0x0c60 - 0x0c62 */
	{ 0x0c65, 0x0001 }, /* 0x0c65 - 0x0c65 */
	{ 0x0c6a, 0x0002 }, /* 0x0c6a - 0x0c6b */
	{ 0x0c70, 0x001e }, /* 0x0c70 - 0x0c8d */
	{ 0x0c90, 0x0007 }, /* 0x0c90 - 0x0c96 */
	{ 0x0c98, 0x0021 }, /* 0x0c98 - 0x0cb8 */
	{ 0x0cba, 0x0015 }, /* 0x0cba - 0x0cce */
	{ 0x0cd0, 0x003a }, /* 0x0cd0 - 0x0d09 */
	{ 0x0d0d, 0x0021 }, /* 0x0d0d - 0x0d2d */
	{ 0x0d30, 0x0010 }, /* 0x0d30 - 0x0d3f */
	{ 0x0d41, 0x001f }, /* 0x0d41 - 0x0d5f */
	{ 0x0d67, 0x0001 }, /* 0x0d67 - 0x0d67 */
	{ 0x0d69, 0x0001 }, /* 0x0d69 - 0x0d69 */
	{ 0x0d70, 0x000e }, /* 0x0d70 - 0x0d7d */
	{ 0x0d80, 0x001a }, /* 0x0d80 - 0x0d99 */
	{ 0x0d9d, 0x0001 }, /* 0x0d9d - 0x0d9d */
	{ 0x0da0, 0x0040 }, /* 0x0da0 - 0x0ddf */
	{ 0x0de2, 0x0001 }, /* 0x0de2 - 0x0de2 */
	{ 0x0deb, 0x001f }, /* 0x0deb - 0x0e09 */
	{ 0x0e10, 0x0010 }, /* 0x0e10 - 0x0e1f */
	{ 0x0e30, 0x0003 }, /* 0x0e30 - 0x0e32 */
	{ 0x0e34, 0x0016 }, /* 0x0e34 - 0x0e49 */
	{ 0x0e50, 0x0001 }, /* 0x0e50 - 0x0e50 */
	{ 0x0e52, 0x000b }, /* 0x0e52 - 0x0e5c */
	{ 0x0e60, 0x000f }, /* 0x0e60 - 0x0e6e */
	{ 0x0e70, 0x0006 }, /* 0x0e70 - 0x0e75 */
	{ 0x0eb0, 0x001d }, /* 0x0eb0 - 0x0ecc */
	{ 0x0eee, 0x000a }, /* 0x0eee - 0x0ef7 */
	{ 0x0efe, 0x000c }, /* 0x0efe - 0x0f09 */
	{ 0x0f10, 0x0014 }, /* 0x0f10 - 0x0f23 */
	{ 0x0f25, 0x000f }, /* 0x0f25 - 0x0f33 */
	{ 0x0f40, 0x000f }, /* 0x0f40 - 0x0f4e */
	{ 0x0f50, 0x0003 }, /* 0x0f50 - 0x0f52 */
	{ 0x0ff7, 0x0008 }, /* 0x0ff7 - 0x0ffe */
	{ 0x1000, 0x0010 }, /* 0x1000 - 0x100f */
	{ 0x1020, 0x000f }, /* 0x1020 - 0x102e */
	{ 0x1030, 0x0024 }, /* 0x1030 - 0x1053 */
	{ 0x1060, 0x0009 }, /* 0x1060 - 0x1068 */
	{ 0x1070, 0x0010 }, /* 0x1070 - 0x107f */
	{ 0x1081, 0x000e }, /* 0x1081 - 0x108e */
	{ 0x1090, 0x0018 }, /* 0x1090 - 0x10a7 */
	{ 0x10b0, 0x0087 }, /* 0x10b0 - 0x1136 */
	{ 0x1138, 0x003e }, /* 0x1138 - 0x1175 */
	{ 0x1178, 0x0008 }, /* 0x1178 - 0x117f */
	{ 0x11ac, 0x0053 }, /* 0x11ac - 0x11fe */
	{ 0x1205, 0x0002 }, /* 0x1205 - 0x1206 */
	{ 0x120a, 0x000f }, /* 0x120a - 0x1218 */
	{ 0x123f, 0x000e }, /* 0x123f - 0x124c */
	{ 0x1258, 0x0016 }, /* 0x1258 - 0x126d */
	{ 0x126f, 0x0010 }, /* 0x126f - 0x127e */
	{ 0x1280, 0x000e }, /* 0x1280 - 0x128d */
	{ 0x128f, 0x00d0 }, /* 0x128f - 0x135e */
	{ 0x1360, 0x0065 }, /* 0x1360 - 0x13c4 */
	{ 0x13c6, 0x0005 }, /* 0x13c6 - 0x13ca */
	{ 0x13d0, 0x0006 }, /* 0x13d0 - 0x13d5 */
	{ 0x13da, 0x0004 }, /* 0x13da - 0x13dd */
	{ 0x13e0, 0x0001 }, /* 0x13e0 - 0x13e0 */
	{ 0x13f3, 0x000c }, /* 0x13f3 - 0x13fe */
	{ 0x1400, 0x0001 }, /* 0x1400 - 0x1400 */
	{ 0x140b, 0x0001 }, /* 0x140b - 0x140b */
	{ 0x1410, 0x0003 }, /* 0x1410 - 0x1412 */
	{ 0x1416, 0x0009 }, /* 0x1416 - 0x141e */
	{ 0x1420, 0x0013 }, /* 0x1420 - 0x1432 */
	{ 0x1439, 0x0004 }, /* 0x1439 - 0x143c */
	{ 0x143f, 0x001b }, /* 0x143f - 0x1459 */
	{ 0x145b, 0x000a }, /* 0x145b - 0x1464 */
	{ 0x1468, 0x0011 }, /* 0x1468 - 0x1478 */
	{ 0x1480, 0x000d }, /* 0x1480 - 0x148c */
	{ 0x1490, 0x0002 }, /* 0x1490 - 0x1491 */
	{ 0x14a0, 0x000b }, /* 0x14a0 - 0x14aa */
	{ 0x14b0, 0x0002 }, /* 0x14b0 - 0x14b1 */
	{ 0x14b7, 0x000f }, /* 0x14b7 - 0x14c5 */
	{ 0x14e5, 0x000b }, /* 0x14e5 - 0x14ef */
	{ 0x14ff, 0x000e }, /* 0x14ff - 0x150c */
	{ 0x1513, 0x000d }, /* 0x1513 - 0x151f */
	{ 0x15f7, 0x0008 }, /* 0x15f7 - 0x15fe */
	{ 0x1600, 0x0015 }, /* 0x1600 - 0x1614 */
	{ 0x1620, 0x000d }, /* 0x1620 - 0x162c */
	{ 0x1630, 0x001f }, /* 0x1630 - 0x164e */
	{ 0x1650, 0x005f }, /* 0x1650 - 0x16ae */
	{ 0x16b0, 0x002a }, /* 0x16b0 - 0x16d9 */
	{ 0x16de, 0x007b }, /* 0x16de - 0x1758 */
	{ 0x1760, 0x0019 }, /* 0x1760 - 0x1778 */
	{ 0x1780, 0x0015 }, /* 0x1780 - 0x1794 */
	{ 0x17a0, 0x002f }, /* 0x17a0 - 0x17ce */
	{ 0x17d0, 0x0045 }, /* 0x17d0 - 0x1814 */
	{ 0x1820, 0x000d }, /* 0x1820 - 0x182c */
	{ 0x1830, 0x001f }, /* 0x1830 - 0x184e */
	{ 0x1850, 0x005f }, /* 0x1850 - 0x18ae */
	{ 0x18b0, 0x002a }, /* 0x18b0 - 0x18d9 */
	{ 0x18de, 0x007b }, /* 0x18de - 0x1958 */
	{ 0x1960, 0x0019 }, /* 0x1960 - 0x1978 */
	{ 0x1980, 0x0015 }, /* 0x1980 - 0x1994 */
	{ 0x19a0, 0x002f }, /* 0x19a0 - 0x19ce */
	{ 0x19d0, 0x0045 }, /* 0x19d0 - 0x1a14 */
	{ 0x1a20, 0x000d }, /* 0x1a20 - 0x1a2c */
	{ 0x1a30, 0x001f }, /* 0x1a30 - 0x1a4e */
	{ 0x1a50, 0x005f }, /* 0x1a50 - 0x1aae */
	{ 0x1ab0, 0x002a }, /* 0x1ab0 - 0x1ad9 */
	{ 0x1ade, 0x007b }, /* 0x1ade - 0x1b58 */
	{ 0x1b60, 0x0019 }, /* 0x1b60 - 0x1b78 */
	{ 0x1b80, 0x0015 }, /* 0x1b80 - 0x1b94 */
	{ 0x1ba0, 0x002f }, /* 0x1ba0 - 0x1bce */
	{ 0x1bd0, 0x0045 }, /* 0x1bd0 - 0x1c14 */
	{ 0x1c20, 0x000d }, /* 0x1c20 - 0x1c2c */
	{ 0x1c30, 0x001f }, /* 0x1c30 - 0x1c4e */
	{ 0x1c50, 0x005f }, /* 0x1c50 - 0x1cae */
	{ 0x1cb0, 0x002a }, /* 0x1cb0 - 0x1cd9 */
	{ 0x1cde, 0x0007 }, /* 0x1cde - 0x1ce4 */
	{ 0x1cf0, 0x0023 }, /* 0x1cf0 - 0x1d12 */
	{ 0x1d20, 0x0039 }, /* 0x1d20 - 0x1d58 */
	{ 0x1d60, 0x0019 }, /* 0x1d60 - 0x1d78 */
	{ 0x1d80, 0x0015 }, /* 0x1d80 - 0x1d94 */
	{ 0x1da0, 0x002f }, /* 0x1da0 - 0x1dce */
	{ 0x1dd0, 0x0030 }, /* 0x1dd0 - 0x1dff */
	{ 0x1e10, 0x0002 }, /* 0x1e10 - 0x1e11 */
	{ 0x1e20, 0x0090 }, /* 0x1e20 - 0x1eaf */
	{ 0x1f00, 0x0018 }, /* 0x1f00 - 0x1f17 */
	{ 0x1f20, 0x0018 }, /* 0x1f20 - 0x1f37 */
	{ 0x1fc0, 0x000d }, /* 0x1fc0 - 0x1fcc */
	{ 0x1ff0, 0x0006 }, /* 0x1ff0 - 0x1ff5 */
	{ 0x1ff7, 0x0015 }, /* 0x1ff7 - 0x200b */
	{ 0x2020, 0x009e }, /* 0x2020 - 0x20bd */
	{ 0x20c0, 0x0002 }, /* 0x20c0 - 0x20c1 */
	{ 0x20d0, 0x0008 }, /* 0x20d0 - 0x20d7 */
	{ 0x20e0, 0x0020 }, /* 0x20e0 - 0x20ff */
	{ 0x21b0, 0x001a }, /* 0x21b0 - 0x21c9 */
	{ 0x21f0, 0x0006 }, /* 0x21f0 - 0x21f5 */
	{ 0x2220, 0x009e }, /* 0x2220 - 0x22bd */
	{ 0x22c0, 0x0002 }, /* 0x22c0 - 0x22c1 */
	{ 0x22e0, 0x0020 }, /* 0x22e0 - 0x22ff */
	{ 0x23b0, 0x001a }, /* 0x23b0 - 0x23c9 */
	{ 0x23f0, 0x0006 }, /* 0x23f0 - 0x23f5 */
	{ 0x2420, 0x009e }, /* 0x2420 - 0x24bd */
	{ 0x24c0, 0x0002 }, /* 0x24c0 - 0x24c1 */
	{ 0x24e0, 0x0020 }, /* 0x24e0 - 0x24ff */
	{ 0x25b0, 0x001a }, /* 0x25b0 - 0x25c9 */
	{ 0x25f0, 0x0006 }, /* 0x25f0 - 0x25f5 */
	{ 0x2600, 0x0029 }, /* 0x2600 - 0x2628 */
	{ 0x2639, 0x0009 }, /* 0x2639 - 0x2641 */
	{ 0x26a0, 0x000f }, /* 0x26a0 - 0x26ae */
	{ 0x26b0, 0x000e }, /* 0x26b0 - 0x26bd */
	{ 0x26c0, 0x000e }, /* 0x26c0 - 0x26cd */
	{ 0x26d6, 0x0002 }, /* 0x26d6 - 0x26d7 */
	{ 0x26e0, 0x0023 }, /* 0x26e0 - 0x2702 */
	{ 0x270f, 0x000f }, /* 0x270f - 0x271d */
	{ 0x2720, 0x001e }, /* 0x2720 - 0x273d */
	{ 0x2747, 0x0001 }, /* 0x2747 - 0x2747 */
	{ 0x2749, 0x0001 }, /* 0x2749 - 0x2749 */
	{ 0x2751, 0x000c }, /* 0x2751 - 0x275c */
	{ 0x275e, 0x0027 }, /* 0x275e - 0x2784 */
	{ 0x27b0, 0x001a }, /* 0x27b0 - 0x27c9 */
	{ 0x2800, 0x000f }, /* 0x2800 - 0x280e */
	{ 0x28a0, 0x000f }, /* 0x28a0 - 0x28ae */
	{ 0x28c2, 0x000c }, /* 0x28c2 - 0x28cd */
	{ 0x28d6, 0x0002 }, /* 0x28d6 - 0x28d7 */
	{ 0x2900, 0x0003 }, /* 0x2900 - 0x2902 */
	{ 0x2920, 0x0010 }, /* 0x2920 - 0x292f */
	{ 0x295b, 0x0002 }, /* 0x295b - 0x295c */
	{ 0x2963, 0x0018 }, /* 0x2963 - 0x297a */
	{ 0x2a00, 0x000f }, /* 0x2a00 - 0x2a0e */
	{ 0x2aa0, 0x000f }, /* 0x2aa0 - 0x2aae */
	{ 0x2ac2, 0x000c }, /* 0x2ac2 - 0x2acd */
	{ 0x2ad6, 0x0002 }, /* 0x2ad6 - 0x2ad7 */
	{ 0x2b00, 0x0003 }, /* 0x2b00 - 0x2b02 */
	{ 0x2b20, 0x0010 }, /* 0x2b20 - 0x2b2f */
	{ 0x2b5b, 0x0002 }, /* 0x2b5b - 0x2b5c */
	{ 0x2b63, 0x0018 }, /* 0x2b63 - 0x2b7a */
	{ 0x2c00, 0x000f }, /* 0x2c00 - 0x2c0e */
	{ 0x2ca0, 0x000f }, /* 0x2ca0 - 0x2cae */
	{ 0x2cc2, 0x000c }, /* 0x2cc2 - 0x2ccd */
	{ 0x2cd6, 0x0002 }, /* 0x2cd6 - 0x2cd7 */
	{ 0x2d00, 0x0003 }, /* 0x2d00 - 0x2d02 */
	{ 0x2d20, 0x0010 }, /* 0x2d20 - 0x2d2f */
	{ 0x2d5b, 0x0002 }, /* 0x2d5b - 0x2d5c */
	{ 0x2d63, 0x0018 }, /* 0x2d63 - 0x2d7a */
	{ 0x3580, 0x000a }, /* 0x3580 - 0x3589 */
	{ 0x3590, 0x0003 }, /* 0x3590 - 0x3592 */
	{ 0x35a0, 0x0007 }, /* 0x35a0 - 0x35a6 */
	{ 0x3780, 0x000a }, /* 0x3780 - 0x3789 */
	{ 0x3790, 0x0003 }, /* 0x3790 - 0x3792 */
	{ 0x37a0, 0x0007 }, /* 0x37a0 - 0x37a6 */
	{ 0x3980, 0x000a }, /* 0x3980 - 0x3989 */
	{ 0x3990, 0x0003 }, /* 0x3990 - 0x3992 */
	{ 0x39a0, 0x0007 }, /* 0x39a0 - 0x39a6 */
	{ 0x3b80, 0x000a }, /* 0x3b80 - 0x3b89 */
	{ 0x3b90, 0x0003 }, /* 0x3b90 - 0x3b92 */
	{ 0x3ba0, 0x0007 }, /* 0x3ba0 - 0x3ba6 */
	{ 0x3c00, 0x0001 }, /* 0x3c00 - 0x3c00 */
	{ 0x3c02, 0x0014 }  /* 0x3c02 - 0x3c15 */
};

static int
dhd_macdbg_dumpphy(macdbg_info_t *mi, struct bcmstrbuf *b)
{
	uint16 m, n, val, cnt = 0;

	if (mi->phyrev == 0) {
		DHD_ERROR(("%s: unsupported for non-ac phy\n", __FUNCTION__));
		return cnt;
	}

	/* dump important registers */
	if (b) {
		bcm_bprintf(b, "===== Important Register =====\n");
	} else {
		printf("===== Important Register =====\n");
	}

	/* since r47, phy will dump all phyreg */
	if (mi->phyrev == 47) {
		for (m = 0; m < ARRAYSIZE(phyreg_list_r47); m++) {
			for (n = 0; n < phyreg_list_r47[m].cnt; n++) {
				cnt += _dhd_print_phyreg(mi, phyreg_list_r47[m].addr + n,
						b, FALSE);
			}
		}
	} else if (mi->phyrev == 130) {
		for (m = 0; m < ARRAYSIZE(phyreg_list_r130); m++) {
			for (n = 0; n < phyreg_list_r130[m].cnt; n++) {
				cnt += _dhd_print_phyreg(mi, phyreg_list_r130[m].addr + n,
						b, FALSE);
			}
		}
	} else if (mi->phyrev == 132) {
		for (m = 0; m < ARRAYSIZE(phyreg_list_r132); m++) {
			for (n = 0; n < phyreg_list_r132[m].cnt; n++) {
				cnt += _dhd_print_phyreg(mi, phyreg_list_r132[m].addr + n,
						b, FALSE);
			}
		}
	} else if (mi->phyrev == 133) {
		for (m = 0; m < ARRAYSIZE(phyreg_list_r133); m++) {
			for (n = 0; n < phyreg_list_r133[m].cnt; n++) {
				cnt += _dhd_print_phyreg(mi, phyreg_list_r133[m].addr + n,
						b, FALSE);
			}
		}
	} else {
		for (m = 0; m < ARRAYSIZE(phyreg_list_all); m++) {
			for (n = 0; n < phyreg_list_all[m].cnt; n++) {
				cnt += _dhd_print_phyreg(mi, phyreg_list_all[m].addr + n, b, FALSE);
			}
		}

		if (mi->phyrev == 32 || mi->phyrev == 33) {
			for (m = 0; m < ARRAYSIZE(phyreg_list_r32_r33); m++) {
				for (n = 0; n < phyreg_list_r32_r33[m].cnt; n++) {
					cnt += _dhd_print_phyreg(mi,
							phyreg_list_r32_r33[m].addr + n, b, FALSE);
				}
			}
			if (mi->phyrev == 33) {
				for (n = 0x1110; n < 0x1119; n++) {
					cnt += _dhd_print_phyreg(mi, n, b, FALSE);
				}
			}
		} else if (mi->phyrev > 47) {
			for (m = 0; m < ARRAYSIZE(phyreg_list_lt132); m++) {
				for (n = 0; n < phyreg_list_lt132[m].cnt; n++) {
					cnt += _dhd_print_phyreg(mi,
							phyreg_list_lt132[m].addr + n,
							b, FALSE);
				}
			}
		}
	}

	if (mi->corerev < 129) {
		DHD_ERROR(("%s: table dumps unsupported for corerev %d\n",
			__FUNCTION__, mi->corerev));
		return cnt;
	}

	/* dump imporant table */
	if (b) {
		bcm_bprintf(b, "\n===== Important TABLE =====\n");
	} else {
		printf("\n===== Important TABLE =====\n");
	}

	/* dump axmacphyIfTbl */
	/* axmacphyIfTbl has 128 entries and each has 64 bits */
	printf("Print axmacphyIfTbl\n");
	for (m = 0; m < 4; m++)
		cnt += _dhd_print_phytable(mi, 229, 32*m, 32, 64,  b, FALSE);

	/* dump warEngMemTbl */
	printf("Print warEngMemTbl\n");
	if (mi->corerev >= 133) {
		for (m = 0; m < 8; m++)
			cnt += _dhd_print_phytable(mi, 236, 0x4800+32*m, 32, 32, b, FALSE);
	}

	if (mi->corerev < 133) {
		/* dump vasipregister */
		printf("Print vasipregister\n");
		cnt += _dhd_print_phytable(mi, 41, 0x2c >> 2, 1, 32,  b, FALSE);
		cnt += _dhd_print_phytable(mi, 41, 0x30 >> 2, 1, 32,  b, FALSE);
		cnt += _dhd_print_phytable(mi, 41, 0xd0 >> 2, 1, 32,  b, FALSE);
		cnt += _dhd_print_phytable(mi, 41, 0xd4 >> 2, 1, 32,  b, FALSE);
		cnt += _dhd_print_phytable(mi, 41, 0xe0 >> 2, 1, 32,  b, FALSE);
		cnt += _dhd_print_phytable(mi, 41, 0xe4 >> 2, 1, 32,  b, FALSE);

		/* dump vasippchistory 16 index */
		printf("Print vasippchistory\n");
		cnt += _dhd_print_phytable(mi, 61, 0, 16, 32,  b, FALSE);

		/* dump smctable */
		printf("Print smctable\n");
		for (m = 0; m < 2; m++)
			cnt += _dhd_print_phytable(mi, 45, 0x34, 1, 32, b, FALSE);
		for (m = 0; m < 2; m++)
			cnt += _dhd_print_phytable(mi, 45, 0x40, 1, 32, b, FALSE);
		for (m = 0; m < 2; m++)
			cnt += _dhd_print_phytable(mi, 45, 0x44, 1, 32, b, FALSE);

		/* dump smctinytable */
		_dhd_set_ihr16(mi, MACREG_PHY_REG_ADDR, 0x140d, b, FALSE);
		val = (_dhd_get_ihr16(mi, MACREG_PHY_REG_DATA, b, FALSE) >> 1) & 0x1;
		if (val == 0) {
			printf("Print smctinytable\n");
			cnt += _dhd_print_phytable(mi, 46, 0x901, 2, 32, b, FALSE);
			cnt += _dhd_print_phytable(mi, 46, 0x9DA, 2, 32, b, FALSE);
			/* SMC M0 table */
			cnt += _dhd_print_phytable(mi, 46, 0, 32, 32, b, FALSE);
			/* SMC M1 table */
			cnt += _dhd_print_phytable(mi, 46, 0x800, 32, 32, b, FALSE);
		} else {
			printf("SMC is running\n");
		}
	} else {
		printf("Skip vasipregister, vasippchistory, smctable, smctinytable\n");
	}

	/* dump BFM header */
	_dhd_set_ihr16(mi, MACREG_PHY_REG_ADDR, 0x10f4, b, FALSE);
	val = _dhd_get_ihr16(mi, MACREG_PHY_REG_DATA, b, FALSE) & 0x1;
	if (val == 0) {
		printf("Print BFM table\n");
		cnt += _dhd_print_phytable(mi, 230, 0, 8, 32,  b, FALSE);
	} else {
		printf("BFM is running\n");
	}

	_dhd_set_ihr16(mi, MACREG_PHY_REG_ADDR, 0x12a5, b, FALSE);
	val = _dhd_get_ihr16(mi, MACREG_PHY_REG_DATA, b, FALSE) & 0x200;
	if (val == 0) {
		/* dump bfdIndexLUT 128 index */
		printf("Print bfdIndexLUT\n");
		for (m = 0; m < 4; m++)
			cnt += _dhd_print_phytable(mi, 63, 32*m, 32, 32,  b, FALSE);
	} else {
		printf("BFD is running\n");
	}
	return cnt;
}

int
dhd_macdbg_psvmpmems(dhd_pub_t *dhdp, char *params, int plen, char *buf, int buflen)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	dhd_pd11regs_param_t *pd11regs = (void *)params;
	dhd_pd11regs_buf_t *pd11regs_buf = (void *)buf;
	uint16 start_idx;
	bool verbose;
	svmp_list_t reglist;
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;

	start_idx = pd11regs->start_idx;
	verbose = pd11regs->verbose;
	memcpy(&reglist, pd11regs->plist, sizeof(reglist));
	memset(buf, '\0', buflen);
	bcm_binit(&bcmstrbuf, pd11regs_buf->pbuf,
		(buflen - OFFSETOF(dhd_pd11regs_buf_t, pbuf)));
	b = &bcmstrbuf;
	pd11regs_buf->idx = _dhd_print_svmps(mi, &reglist, start_idx, b, verbose);

	return ((pd11regs_buf->idx > 0) ? BCME_OK : BCME_ERROR);
}

/* convenience macros for entry sizes */
#define MACDBG_RLM_RATE_ENTRY_SIZE		(256)
#define MACDBG_RLM_LINK_ENTRY_SIZE		(64)
#define MACDBG_RLM_LINK_ENTRY_SIZE_REV132	(128)
#define MACDBG_RATE_OFFSET(idx)			(0x8c0000 + ((idx) * 256))
#define MACDBG_RLM_LINK_BASE_ADDR(_d11rev)	((_d11rev) >= 132 ? 0x8e0000 : 0x8d0000)
#define MACDBG_LINK_OFFSET(_d11rev, idx, size) \
		(MACDBG_RLM_LINK_BASE_ADDR(_d11rev) + ((idx) * (size)))

static void
_dhd_print_rlmemblk(macdbg_info_t *mi, struct bcmstrbuf *b, int32 idx, bool rmem,
	char *pingpong_str)
{
	uint32 addr, val, val1, offset = 0, amt_offset = 0;
	int i, j, size;

	if (rmem) {
		size = MACDBG_RLM_RATE_ENTRY_SIZE;
		addr = MACDBG_RATE_OFFSET(idx);
		bcm_bprintf(b, "RateMem Block%s: ", pingpong_str);
		bcm_bprintf(b, "%d Size:%dB\n", idx, size);
	} else {
		if (mi->corerev >= 132) {
			size = MACDBG_RLM_LINK_ENTRY_SIZE_REV132;
		} else {
			size = MACDBG_RLM_LINK_ENTRY_SIZE;
		}
		addr = MACDBG_LINK_OFFSET(mi->corerev, idx, size);
		bcm_bprintf(b, "LinkMem Block: ");
		amt_offset = (idx * 2) << 2;
		val = _dhd_get_d11obj32(mi, amt_offset, 0x02040000, b, FALSE);
		val1 = _dhd_get_d11obj32(mi, amt_offset+4, 0x02040000, b, FALSE);
		bcm_bprintf(b, "%d Size:%dB MAC:%02x:%02x:%02x:%02x:%02x:%02x attr:0x%04x\n",
			idx, size, val & 0xff, (val >> 8) & 0xff, (val >> 16) & 0xff,
			(val >> 24) & 0xff, val1 & 0xff, (val1 >> 8) & 0xff,
			val1 >> 16);
	}

	for (i = 0; i < (size + 15) / 16; i++) {
		bcm_bprintf(b, "0x%08x:", (addr + offset));
		for (j = 0; j < 4 && offset < size; j++) {
			val = _dhd_get_sbreg(mi, (addr + offset), 4);
			bcm_bprintf(b, " %02x %02x %02x %02x",
				((uint8 *)&val)[0], ((uint8 *)&val)[1],
				((uint8 *)&val)[2], ((uint8 *)&val)[3]);
			offset += 4;
		}
		bcm_bprintf(b, "\n");
	}
}

static void
dhd_macdbg_read_mpm(macdbg_info_t *mi, struct bcmstrbuf *b, uint16 addr, uint16 len)
{
	int j;
	_dhd_set_ihr16(mi, 0xa70, addr, b, FALSE);
	for (j = 0; j < len; j++) {
		if (b) {
			bcm_bprintf(b, "%04x", hton16(_dhd_get_ihr16(mi, 0xa72, b, FALSE)));
		} else {
			printf("%04x", hton16(_dhd_get_ihr16(mi, 0xa72, b, FALSE)));
		}
	}
	if (b) {
		bcm_bprintf(b, "\n");
	} else {
		printf("\n");
	}
}

#define D11_MAX_DMATX_ENGINES_REV129			24
#define D11_MAX_DMATX_ENGINES_REV130			12
#define D11_MAX_DMATX_ENGINES_REV131			6
#define D11_MAX_DMATX_ENGINES_REV132			24
#define D11_MAX_DMATX_ENGINES_REV133			24
#define D11_MAX_DMATX_ENGINES_REV134			16
#define D11_MAX_DMATX_ENGINES_REV135			8
#define D11_MAX_DMATX_ENGINES_REV136			16
#define D11_MAX_DMATX_ENGINES_REV_GE137			8

#ifndef D11_MAX_DMATX_ENGINES  /* This needs to be fixed in a better way probably */
/** #dma engines that can be addressed through d11 register D11_TXE_XMTDMA_DBG_CTL */
#define D11_MAX_DMATX_ENGINES(corerev)	\
	((corerev >= 137) ? D11_MAX_DMATX_ENGINES_REV_GE137 : \
	(corerev == 136) ? D11_MAX_DMATX_ENGINES_REV136 : \
	(corerev == 135) ? D11_MAX_DMATX_ENGINES_REV135 : \
	(corerev == 134) ? D11_MAX_DMATX_ENGINES_REV134 : \
	(corerev >= 132) ? D11_MAX_DMATX_ENGINES_REV132 : \
	(corerev >= 131) ? D11_MAX_DMATX_ENGINES_REV131 : \
	(corerev >= 130) ? D11_MAX_DMATX_ENGINES_REV130 : \
	D11_MAX_DMATX_ENGINES_REV129)
#endif

static int
dhd_macdbg_dumpperuser(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	uint16 reglist_sz, cnt = 0;
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;
	uint16 regmux, csb_req, txe_ctl2, qmap, cclen, cc1_len, cc2_len;
	int i, j, k, num_mpdus;

	uint16 txdbg_sel[] = {
		0x0000, 0x0001, 0x0002,
		0x0100, 0x0110, // 43684/6710/6717: data2mpm/txebytes/txebytes
		0x0200, 0x0210, // all: fcs2mpm
		0x0300, 0x0310, // all: wep2fcs
		0x0400, 0x0410, //  6717 only: data2mpm
		// below are 6716 only
		0x0500, 0x0510, // split2serial
		0x0600, 0x0610, // serial2wep
		0x0700, 0x0710, // fcs_nmpdu_in
		0x0800, 0x0810, // fcs_nmpdu_out
		0x0900, 0x0910, // fcs_state
		0x0a00, 0x0a10, // txe_rcvd
		0x0b00, 0x0b10, // mpmfifo_notempty
		0x0c00, 0x0c10, // mpmfifo_lastfifo
		0x0d00, 0x0d10, // mpmfifo_full
		0x0e00, 0x0e10, // bfe_sendcnt
		0x0f00, 0x0e10, // mpm_startaddr
		0x1000, 0x1010, // mpm_end_addr
		0x1100, 0x1110, // mpm_wptr
		0x1200, 0x1210, // mpm_rptr
		0x1300, 0x1310, // data_bytes
		0x1400, 0x1410, // bytes_enables
		0x1500, 0x1510, // last_ptr
		0x1600, 0x1610 // last_ppduid
	};

	d11regs_list_t reglist[] = {
		{D11REG_TYPE_IHR16, 0x540, 0x00000013, 2, 0},
		{D11REG_TYPE_IHR16, 0x5d0, 0x00000001, 2, 0},
		{D11REG_TYPE_IHR16, 0x7b0, 0x00000001, 2, 0},
		{D11REG_TYPE_IHR16, 0x922, 0x00000017, 2, 0},
		{D11REG_TYPE_IHR16, 0xa7c, 0x00000003, 2, 0},
		{D11REG_TYPE_IHR16, 0xb14, 0x00000129, 2, 0},
		{D11REG_TYPE_IHR16, 0xb70, 0x00400001, 2, 0},
		{D11REG_TYPE_IHR16, 0xbce, 0x00002003, 2, 0},
		{D11REG_TYPE_IHR16, 0xc12, 0x7f80705f, 2, 0},
		{D11REG_TYPE_IHR16, 0xc52, 0x0078010f, 2, 0},
		{D11REG_TYPE_IHR16, 0xcd4, 0x00000001, 2, 0},
		{D11REG_TYPE_IHR16, 0xd66, 0x0000007f, 2, 0},
		{D11REG_TYPE_IHR16, 0xe0a, 0xf18f801f, 2, 0},
		{D11REG_TYPE_IHR16, 0xe4c, 0x00000003, 2, 0}
	};

	d11regs_list_t per_mpdu_list[] = {
		{D11REG_TYPE_IHR16, 0xd64, 0x00000fff, 2, 0}
	};

	d11regs_list_t csb_reglist0[] = {
		{D11REG_TYPE_IHR16, 0xd60, 0x00040001, 2, 0}
	};
	d11regs_list_t csb_reglist0_ge133[] = {
		{D11REG_TYPE_IHR16, 0xd60, 0x00008001, 2, 0},
		{D11REG_TYPE_IHR16, 0xce6, 0x0000003f, 2, 0}	// autobqc
	};
	/* rx regs valid only for users 0-7 for rev 133 */
	d11regs_list_t csb_rx_reglist0_ge133[] = {
		{D11REG_TYPE_IHR16, 0x4ca, 0x00000001, 2, 0},
		{D11REG_TYPE_IHR16, 0x510, 0x00020470, 2, 0},
		{D11REG_TYPE_IHR16, 0x5f0, 0x00000020, 2, 0},
		{D11REG_TYPE_IHR16, 0x1278, 0x00000001, 2, 0},
		{D11REG_TYPE_IHR16, 0x1600, 0x00002001, 2, 0}	// wep_ctl, _err_msk
	};
	d11regs_list_t wep_rx[] = {
		/* wep_stat, wep_err_sts, wep_cs_ctl */
		{D11REG_TYPE_IHR16, 0x1600, 0x00005002, 2, 0}
	};
	d11regs_list_t wep_tx[] = {
		/* wep_stat, wep_err_sts, wep_cs_ctl, wep_psdulen */
		{D11REG_TYPE_IHR16, 0x1600, 0x0000500a, 2, 0}
	};

	/* registers available starting from 6717 */
	d11regs_list_t ge133[] = {
		{D11REG_TYPE_IHR16, 0x4de, 0x00000001, 2, 0},
		{D11REG_TYPE_IHR16, 0x500, 0x001b0068, 2, 0},
		{D11REG_TYPE_IHR16, 0x9de, 0x00000001, 2, 0},
		{D11REG_TYPE_IHR16, 0x1000, 0x00000033, 2, 0},
		{D11REG_TYPE_IHR16, 0x11b4, 0x0006663f, 2, 0},
		{D11REG_TYPE_IHR16, 0x122e, 0x00000003, 2, 0},
		{D11REG_TYPE_IHR16, 0x12ba, 0x00000007, 2, 0},
		{D11REG_TYPE_IHR16, 0x13e4, 0x00000003, 2, 0}
	};

	d11regs_list_t ge134[] = {
		{D11REG_TYPE_IHR16, 0x132e, 0x00000003, 2, 0}, // pre/postbm
		{D11REG_TYPE_IHR16, 0x1786, 0x00000007, 2, 0} // bmc
	};

	d11regs_list_t csb_reglist2_3[] = {
		{D11REG_TYPE_IHR16, 0xd60, 0x01e00001, 2, 0}
	};

	d11regs_list_t csb_reglist5[] = {
		{D11REG_TYPE_IHR16, 0xd60, 0x40000001, 2, 0}
	};

	d11regs_list_t seqmon_reglist[] = {
		{D11REG_TYPE_IHR16, 0x1742, 0x00003fff, 2, 0}
	};

	reglist_sz = ARRAYSIZE(reglist);

	if (mi->corerev < 129) {
		DHD_ERROR(("%s: unsupported corerev %d\n",
			__FUNCTION__, mi->corerev));
		return BCME_UNSUPPORTED;
	}

	if (buf && buflen > 0) {
		bcm_binit(&bcmstrbuf, buf, buflen);
		b = &bcmstrbuf;
	}

	regmux = _dhd_get_ihr16(mi, 0x7b0, b, FALSE);
	txe_ctl2 = _dhd_get_ihr16(mi, 0x904, b, FALSE);

	for (i = 0; i < MAX_NUSR(mi->corerev); i++) {
		_dhd_set_ihr16(mi, 0x7b0, (0xff00 | i), b, FALSE);
		if (mi->corerev >= 134) {
			// TXE_XMTDMA_RUWT_QSEL: user id in revid >= 134
			_dhd_set_ihr16(mi, 0xbce, i, b, FALSE);
		}

		cnt += _dhd_pd11regs_bylist(mi, reglist, reglist_sz, b);

		/* per mpdu */
		num_mpdus = (_dhd_get_ihr16(mi, 0xcc8, b, FALSE)) & 0x3f;
		if (b) {
			bcm_bprintf(b, "0   ihr 0xcc8  = 0x%x\n", num_mpdus);
		} else {
			printf("0   ihr 0xcc8  = 0x%x\n", num_mpdus);
		}
		num_mpdus = MIN(num_mpdus, MPDUS_DUMP_NUM);
		for (j = 0; j < num_mpdus; j++) {
			_dhd_set_ihr16(mi, 0xd62, j, b, FALSE);
			while (!(_dhd_get_ihr16(mi, 0xd62, b, FALSE) & 0x8000));
			cnt += _dhd_pd11regs_bylist(mi, per_mpdu_list, ARRAYSIZE(per_mpdu_list), b);
		}

		/* txdbg_sel */
		for (j = 0; j < NUM_TXDBG(mi->corerev); j++) {
			if (j < 3) {
				_dhd_set_ihr16(mi, 0xa86, txdbg_sel[j], b, FALSE);
			} else {
				_dhd_set_ihr16(mi, 0xa86, txdbg_sel[j] | i, b, FALSE);
			}
			if (b) {
				bcm_bprintf(b, "0x%x\n", _dhd_get_ihr16(mi, 0xa88, b, FALSE));
			} else {
				printf("0x%x\n", _dhd_get_ihr16(mi, 0xa88, b, FALSE));
			}
		}

		/* rx frame data for ctrl/mgmt frames
		 * config changes needed for data frame
		 */
		if (mi->corerev >= 133) {
			for (j = 0; j < 32; j++) {
				_dhd_set_ihr16(mi, 0x1342, j, b, FALSE);
					if (b) {
						bcm_bprintf(b, "0x%x\n",
							_dhd_get_ihr16(mi, 0x1344, b, FALSE));
					} else {
						printf("0x%x\n",
							_dhd_get_ihr16(mi, 0x1344, b, FALSE));
					}
			}
		}

		if (mi->corerev >= 133) {
			/* rxq_dbg_sts */
			for (j = 0; j < 4; j++) {
				for (k = 0; k < 10; k++) {
					_dhd_set_ihr16(mi, 0x514, (j << 14) | k, b, FALSE);
					if (b) {
						bcm_bprintf(b, "0x%x\n",
							_dhd_get_ihr16(mi, 0x516, b, FALSE));
					} else {
						printf("0x%x\n",
							_dhd_get_ihr16(mi, 0x516, b, FALSE));
					}
				}
			}

			/* postBM */
			for (j = 0; j < 21; j++) {
				_dhd_set_ihr16(mi, 0x132c, (uint16) j, b, FALSE);
				if (b) {
					bcm_bprintf(b, "0x%x\n",
						_dhd_get_ihr16(mi, 0x132e, b, FALSE));
				} else {
					printf("0x%x\n",
						_dhd_get_ihr16(mi, 0x132e, b, FALSE));
				}
			}
		}

		/* aqmcsb */
		qmap = _dhd_get_ihr16(mi, 0xc12, b, FALSE);
		csb_req = _dhd_get_ihr16(mi, 0xd60, b, FALSE);
		_dhd_set_ihr16(mi, 0xd88, 0, b, FALSE);
		_dhd_set_ihr16(mi, 0xd60, (csb_req & 0xff80) | qmap, b, FALSE);
		for (j = 0; j <= 5; j++) {
			if (j == 1 || j == 4) {
				continue;
			}
			csb_req = _dhd_get_ihr16(mi, 0xd60, b, FALSE);
			_dhd_set_ihr16(mi, 0xd60, (csb_req & 0xfcff) | (2 << 8), b, FALSE);
			csb_req = _dhd_get_ihr16(mi, 0xd60, b, FALSE);
			_dhd_set_ihr16(mi, 0xd60, (csb_req & 0xc3ff) | (j << 10), b, FALSE);
			while (!(_dhd_get_ihr16(mi, 0xd60, b, FALSE) & 0x8000));
			if (j == 0) {
				if (mi->corerev >= 133) {
					cnt += _dhd_pd11regs_bylist(mi, csb_reglist0_ge133,
						ARRAYSIZE(csb_reglist0_ge133), b);
					if (mi->corerev == 133 && i >= 8) {
						/* rev133 only supports 8 RX users */
						continue;
					}
					cnt += _dhd_pd11regs_bylist(mi, csb_rx_reglist0_ge133,
						ARRAYSIZE(csb_rx_reglist0_ge133), b);
				} else {
					cnt += _dhd_pd11regs_bylist(mi, csb_reglist0,
						ARRAYSIZE(csb_reglist0), b);
				}
			} else if (j == 2 || j == 3) {
				cnt += _dhd_pd11regs_bylist(mi, csb_reglist2_3,
						ARRAYSIZE(csb_reglist2_3), b);
			} else {
				cnt += _dhd_pd11regs_bylist(mi, csb_reglist5,
						ARRAYSIZE(csb_reglist5), b);
			}
		}

		/* bmc, rxe */
		if (mi->corerev >= 133) {
			_dhd_set_ihr16(mi, 0x1008, qmap, b, FALSE);
			cnt += _dhd_pd11regs_bylist(mi, ge133, ARRAYSIZE(ge133), b);
		}

		/* wep */
		if (mi->corerev >= 133) {
			if (i < 8 || mi->corerev > 133) {
				_dhd_set_ihr16(mi, 0x904, txe_ctl2 | (1 << 4), b, FALSE);
				cnt += _dhd_pd11regs_bylist(mi, wep_rx, ARRAYSIZE(wep_rx), b);
			}
			_dhd_set_ihr16(mi, 0x904, txe_ctl2 & ~(1 << 4), b, FALSE);
			cnt += _dhd_pd11regs_bylist(mi, wep_tx, ARRAYSIZE(wep_tx), b);
		}

		if (mi->corerev >= 134) {
			/* seqmon */
			for (j = 0; j < 3; j++) {
				_dhd_set_ihr16(mi, 0x1740, (j << 4) | i, b, FALSE);
				if (j == 2) {
					if (b) {
						bcm_bprintf(b, "0   ihr 0x1742  = 0x%x\n",
							_dhd_get_ihr16(mi, 0x1742, b, FALSE));
					} else {
						printf("0   ihr 0x1742  = 0x%x\n",
							_dhd_get_ihr16(mi, 0x1742, b, FALSE));
					}
					break;
				} else {
					cnt += _dhd_pd11regs_bylist(mi, seqmon_reglist,
							ARRAYSIZE(seqmon_reglist), b);
				}
			}

			_dhd_set_ihr16(mi, 0x132c, (i << 12) | i, b, FALSE);
			/* additional bmc and pre/postbm */
			cnt += _dhd_pd11regs_bylist(mi, ge134, ARRAYSIZE(ge134), b);
		}
	}

	/* additional pre/post bm regs */
	if (mi->corerev >= 134) {
		_dhd_set_ihr16(mi, 0x132c, 0x7000, b, FALSE);
		_dhd_set_ihr16(mi, 0x502, 0x4500, b, FALSE);
		if (b) {
			bcm_bprintf(b, "0   ihr 0x4500  = 0x%x\n",
				_dhd_get_ihr16(mi, 0x1330, b, FALSE));
		} else {
			printf("0   ihr 0x4500  = 0x%x\n",
				_dhd_get_ihr16(mi, 0x1330, b, FALSE));
		}
		_dhd_set_ihr16(mi, 0x502, 0x5600, b, FALSE);
		if (b) {
			bcm_bprintf(b, "0   ihr 0x5600  = 0x%x\n",
				_dhd_get_ihr16(mi, 0x1330, b, FALSE));
		} else {
			printf("0   ihr 0x5600  = 0x%x\n",
				_dhd_get_ihr16(mi, 0x1330, b, FALSE));
		}
		_dhd_set_ihr16(mi, 0x502, 0x6000, b, FALSE);
		if (b) {
			bcm_bprintf(b, "0   ihr 0x6000  = 0x%x\n",
				_dhd_get_ihr16(mi, 0x1330, b, FALSE));
		} else {
			printf("0   ihr 0x6000  = 0x%x\n",
				_dhd_get_ihr16(mi, 0x1330, b, FALSE));
		}
	}

	/* dump mpm macphy contents */
	if (mi->corerev >= 133) {
		cclen = _dhd_get_ihr16(mi, 0xec8, b, FALSE);
		cc1_len = ((cclen & 0xff) + 1)/2;
		cc2_len = (((cclen >> 8) & 0xff) + 1)/2;
		dhd_macdbg_read_mpm(mi, b, 0xc200, 28);
		dhd_macdbg_read_mpm(mi, b, 0xc400, cc1_len);
		dhd_macdbg_read_mpm(mi, b, 0xc480, cc2_len);
		/* restore txe_ctl2 */
		_dhd_set_ihr16(mi, 0x904, txe_ctl2, b, FALSE);
	}

	if (b && outbuflen) {
		if (buflen > BCMSTRBUF_LEN(b)) {
			*outbuflen = buflen - BCMSTRBUF_LEN(b);
		} else {
			DHD_ERROR(("%s: buflen insufficient!\n", __FUNCTION__));
			*outbuflen = buflen;
			/* Do not return buftooshort to allow printing macregs we have got */
		}
	}

	/* Need to restore before exiting */
	_dhd_set_ihr16(mi, 0x7b0, regmux, b, FALSE);
	return (cnt > 0) ? BCME_OK : BCME_UNSUPPORTED;
}

static void
dhd_macdbg_dumpratemem_pingpong(dhd_pub_t *dhdp, int rtidx, struct bcmstrbuf *b)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	int corerev = mi->corerev;

	/* set rmem to ping */
	_dhd_set_ihr16(mi, MACREG_PSM_RATEMEM_DBG(corerev), 4, b, FALSE);
	_dhd_print_rlmemblk(mi, b, rtidx, TRUE, " (Ping)");
	/* set rmem to pong */
	_dhd_set_ihr16(mi, MACREG_PSM_RATEMEM_DBG(corerev), 0xc, b, FALSE);
	_dhd_print_rlmemblk(mi, b, rtidx, TRUE, " (Pong)");
}

static int
dhd_macdbg_dumpratelinkmem(dhd_pub_t *dhdp, char *buf, int buflen,
	int *outbuflen)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;
	int idx;

	/* validation check */
	if (mi->corerev < 128) {
		DHD_ERROR(("%s: unsupported corerev %d\n",
			__FUNCTION__, mi->corerev));
		return BCME_UNSUPPORTED;
	}

	if (buf == NULL || buflen <= 0) {
		return BCME_ERROR;
	}

	bcm_binit(&bcmstrbuf, buf, buflen);
	b = &bcmstrbuf;

	/* BCMC entry */
	dhd_macdbg_dumpratemem_pingpong(dhdp, MACDBG_RLM_BCMC_ENTRY, b);
	_dhd_print_rlmemblk(mi, b, MACDBG_RLM_BCMC_ENTRY, FALSE, "");
	bcm_bprintf(b, "\n");

	/* VBSS + TWT entries in linkmem */
	for (idx = MACDBG_RLM_VBSS_START; idx <= MACDBG_RLM_TWT_END; idx++) {
		_dhd_print_rlmemblk(mi, b, idx, FALSE, "");
		bcm_bprintf(b, "\n");
	}

	/* ULMU user blocks + RUCFG table in ratemem */
	for (idx = MACDBG_RLM_ULOFDMA_START; idx <= MACDBG_RLM_DLOFDMA_END; idx++) {
		/* print ping only */
		_dhd_set_ihr16(mi, MACREG_PSM_RATEMEM_DBG(mi->corerev), 4, b, FALSE);
		_dhd_print_rlmemblk(mi, b, idx, TRUE, " (Ping)");
		bcm_bprintf(b, "\n");
	}

	for (idx = 0; idx <= mi->rlm_max_index; idx++) {
		if (idx == MACDBG_RLM_BCMC_ENTRY) {
			continue;
		}

		if (idx < MACDBG_RLM_ULOFDMA_START || idx > MACDBG_RLM_DLOFDMA_END) {
			dhd_macdbg_dumpratemem_pingpong(dhdp, idx, b);
		}

		if (idx < MACDBG_RLM_VBSS_START || idx > MACDBG_RLM_TWT_END) {
			_dhd_print_rlmemblk(mi, b, idx, FALSE, "");
		}
		bcm_bprintf(b, "\n");
	}

	return BCME_OK;
}

#define RXBMDUMPSZ		0xc000
#define RXBM_ADDR_REV129	0x580000	/* starting offset: 5.5MB */
#define RXBM_ADDR_REV132	0x380000	/* starting offset: 3.5MB */
#define RXBM_ADDR_REV133	0x800000	/* starting offset: 8MB */
#define RXBM_ADDR_REV134	0x480000
#define RXBM_SZ			0x180000	/* length: 1.5MB */
#define RXBM_SZ_REV133		0x200000	/* length: 2MB */
#define RXBM_SZ_REV134		0x1c0000

/* called under DHD_LOCK */
int
dhd_macdbg_dumprxbm(dhd_pub_t *dhdp, char *buf, int buflen, const char *filename)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	uint32 params[2] = {0, RXBMDUMPSZ};
	uint i, j, rem_sz, membytes_sz;
	char *p;
	uint8 *membytes_buf;
	int res;
	uint32 saved_addr = 0;

	/* validation check */
	rem_sz = RXBM_SZ; /* remaining byte size */
	if (mi->corerev == 129) {
		params[0] = RXBM_ADDR_REV129;
	} else if (mi->corerev == 132) {
		params[0] = RXBM_ADDR_REV132;
	} else if (mi->corerev == 133) {
		params[0] = 0;
		rem_sz = RXBM_SZ_REV133;
	} else if (mi->corerev == 134) {
		params[0] = RXBM_ADDR_REV134;
		rem_sz = RXBM_SZ_REV134;
	} else {
		DHD_ERROR(("%s: unsupported corerev %d\n",
			__FUNCTION__, mi->corerev));
		return BCME_UNSUPPORTED;
	}
	membytes_buf = MALLOCZ(dhdp->osh, RXBMDUMPSZ);
	if (!membytes_buf) {
		DHD_ERROR(("%s: NOMEM for len %d\n", __FUNCTION__, RXBMDUMPSZ));
		return BCME_NOMEM;
	}

	while (rem_sz > 0) {
		memset(membytes_buf, '\0', RXBMDUMPSZ);
		memset(buf, '\0', buflen);
		membytes_sz = MIN(RXBMDUMPSZ, rem_sz);
		params[1] = membytes_sz;

		if (mi->corerev == 133) {
			/* Remap the BAR1 window to the MAC BM at RXBM_ADDR_REV133 */
			saved_addr = OSL_PCI_READ_CONFIG(dhdp->osh, PCI_BAR1_WIN, sizeof(uint32));
			OSL_PCI_WRITE_CONFIG(dhdp->osh, PCI_BAR1_WIN, sizeof(uint32),
				RXBM_ADDR_REV133);
		}

		res = BUS_IOVAR_OP(dhdp, "membytes",
			params, sizeof(params), membytes_buf, membytes_sz, IOV_GET);

		if (mi->corerev == 133) {
			/* Restore the BAR1 window to the beginning of sysmem */
			OSL_PCI_WRITE_CONFIG(dhdp->osh, PCI_BAR1_WIN, sizeof(uint32), saved_addr);
		}

		if (res != BCME_OK) {
			break;
		}

		p = buf;
		for (i = 0; i < membytes_sz; i++) {
			if (i % 16 == 0) {
				p += sprintf(p, "%08x: ", params[0] + i); /* line prefix */
			}
			p += sprintf(p, "%02x ", membytes_buf[i]);
			if (i % 16 == 15) {
				p += sprintf(p, "  ");
				for (j = i-15; j <= i; j++)
					p += sprintf(p, "%c",
						((membytes_buf[j] >= 0x20 &&
						membytes_buf[j] <= 0x7f) ?
						membytes_buf[j] : '.'));
				p += sprintf(p, "\n"); /* flush line */
			}
		}

#ifdef LINUX
		if (filename) {
			DHD_UNLOCK(dhdp);
			DHD_OS_WAKE_UNLOCK(dhdp);
			dhd_write_file(dhdp, filename, buf, strlen(buf));
			DHD_OS_WAKE_LOCK(dhdp);
			DHD_LOCK(dhdp);
		} else
#endif /* LINUX */
		{
			printf("%s\n", buf);
		}

		rem_sz -= membytes_sz;
		params[0] += membytes_sz;
	}

	MFREE(dhdp->osh, membytes_buf, RXBMDUMPSZ);

	return res;
}

static int
dhd_macdbg_dumpbmc(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen)
{
#define D11MAC_BMC_MAXFIFOS_GE128		89 /* 86+1+2 */
	macdbg_info_t *mi = dhdp->macdbg_info;
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;
	int corerev;
	uint16 *pbuf = NULL, *p;
	int i, j, num_of_fifo;
	/* specify dump which queues */
	uint32 fifo_bmp[D11MAC_BMC_MAXFIFOS_GE128/32 + 1];
	int fifo_phys;
	uint tpl_idx, rxq0_idx, rxq1_idx, fifoid;
	int num_statsel;
	uint8 statsel_nbit, bmask_fifosel, seltype;

	if (buf && buflen > 0) {
		bcm_binit(&bcmstrbuf, buf, buflen);
		b = &bcmstrbuf;
	}
	corerev = mi->corerev;
	tpl_idx = BMC_TPL_IDX(corerev);
	rxq0_idx = tpl_idx + 1;
	rxq1_idx = tpl_idx + 2;
	num_statsel = (corerev >= 134) ? 14 : 13;
	statsel_nbit = 9;
	bmask_fifosel = 0x7f;

	/* validation check */
	if (corerev < 129) {
		DHD_ERROR(("%s: unsupported corerev %d\n", __FUNCTION__, corerev));
		return BCME_UNSUPPORTED;
	}

	if ((pbuf = (uint16*) MALLOCZ(dhdp->osh, num_statsel*sizeof(uint16))) == NULL) {
		DHD_ERROR(("wl: %s: MALLOC failure\n", __FUNCTION__));
		return BCME_NOMEM;
	}

	num_of_fifo = BMC_TPL_IDX(corerev) + 3;
	for (i = 0; i < num_of_fifo/32+1; i++) {
		fifo_bmp[i] = -1;
	}

	if (b) {
		bcm_bprintf(b, "AQQMAP 0x%x AQMF_STATUS 0x%x AQMCT_PRIFIFO 0x%x\n",
			_dhd_get_ihr16(mi, 0xc12, b, FALSE),
			_dhd_get_ihr16(mi, 0xcc8, b, FALSE),
			_dhd_get_ihr16(mi, 0xd36, b, FALSE));
		bcm_bprintf(b, "BMC stats: 0-nfrm 1-nbufRecvd 2-nbuf2DMA 3-nbufMax "
		"4-nbufUse 5-nbufMin 6-nFree\n"
		"%11s7-nDrqPush 8-nDrqPop 9-nDdqPush 10-nDdqPop "
		"11-nOccupied 12-nAvalTid %s\n", "",
			(corerev >= 134) ? "13-TailIndex" : "");
		for (i = 0; i < num_of_fifo; i++) {
			/* skip template */
			if (i == tpl_idx) {
				continue;
			}
			p = pbuf;

			seltype = 0;
			fifoid = i;
			fifo_phys = i;
			if (corerev >= 128) {
				if ((fifo_bmp[i/32] & (1<<(i%32))) == 0) {
					continue;
				}
				if (i == rxq0_idx || i == rxq1_idx) {
					//rx fifo
					seltype = 1;
					fifoid = i - rxq0_idx;
				} else {
					//tx fifio
					seltype = 0;
					fifoid = i;
					fifo_phys = (i < 6 ? (i + 64) : (i - 6));
				}
			}

			for (j = 0; j < num_statsel; j++, p++) {
				_dhd_set_ihr16(mi, 0xb9e,
				(j << statsel_nbit) | (fifoid & (bmask_fifosel)) |
				(seltype << 7), b, FALSE);
				*p = _dhd_get_ihr16(mi, 0xba0, b, FALSE);
			}
			bcm_bprintf(b, "fifo-%02d(p%02d) :", i, fifo_phys);
			p = pbuf;
			for (j = 0; j < num_statsel; j++, p++) {
				bcm_bprintf(b, " %04x", *p);
			}
			bcm_bprintf(b, "\n");
		}
	}

	MFREE(dhdp->osh, pbuf, num_statsel * sizeof(uint16));

	return BCME_OK;
}

#define MLO_AQMQ_OFFSET 48
#define MAX_MLO_AQMQ 32
#define MLO_DBG_OPCODE_RDBMPST 2
#define MLO_DBG_OPCODE_RDTXCNT 4
#define MLO_AMGR_OFFSET (0x280 << 16) // AMGR_OFFSET 0x280_0000
#define MLO_STDBG_CTL0_DONE (1 << 31)

#ifndef MLO_STDBG_CTL0
#define MLO_STDBG_CTL0 0x1E0
#endif
#ifndef MLO_STDBG_DAT0
#define MLO_STDBG_DAT0 0x1E8
#endif
typedef union {
	uint32 u32;
	struct {
		uint32 dbg_opcode      : 4; /* b[3:0] */
		uint32 mlo_cmdq_rdptr  : 4; /* b[7:4] */
		uint32 rw_offset       : 4; /* b[11:8] */
		uint32 reserved_1      : 4; /* b[15:12] */
		uint32 exp_qnum        : 7; /* b[22:16] */
		uint32 exp_qnum_vld    : 1; /* b[23] */
		uint32 reserved_0      : 7; /* b[30:24] */
		uint32 done            : 1; /* b[31] */
	};
} mlo_stdbg_ctrl0_t;

typedef union {
	uint32 u32;
	struct {
		uint8 data[4];
	};
} mlo_stdbg_data_t;

static void
dhd_macdbg_get_aqmsts(macdbg_info_t *mi, uint16 aqmq, uint opcode,
	uint offset, mlo_stdbg_data_t *results)
{
	mlo_stdbg_ctrl0_t data;
	uint i;

	// setup to read the bmp status
	data.u32 = 0;
	data.exp_qnum_vld = 1;
	data.exp_qnum = aqmq;
	data.rw_offset = offset;
	data.dbg_opcode = opcode;

	// for pcie, sbreg access via SI_ENUM_BASE: 0x2800_0000
	_dhd_set_sbreg(mi, (MLO_AMGR_OFFSET + MLO_STDBG_CTL0), 4, data.u32);
	// wait for done bit = 1
	while (!(_dhd_get_sbreg(mi, (MLO_AMGR_OFFSET+MLO_STDBG_CTL0), 4) & MLO_STDBG_CTL0_DONE));
	// now we can read the bmp from DAT0~DAT7
	for (i = 0; i < 8; i++) {
		// each MLO_STDBG_DATx is 4 bytes word
		results[i].u32 =
			_dhd_get_sbreg(mi, ((MLO_AMGR_OFFSET + MLO_STDBG_DAT0) + (i << 2)), 4);
	}
}

//#define AQMSTS_DEBUG
#ifdef AQMSTS_DEBUG
static int
dhd_macdbg_dumpaqmsts(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;
	int corerev;
	int aqmq, i, j, k, state;
	mlo_stdbg_data_t results[8];
	char decode[6] = {'F', 'A', '0', '1', '2', '3'};

	if (buf == NULL || buflen <= 0) {
		DHD_ERROR(("%s: Invalid Input Params\n", __FUNCTION__));
		return BCME_ERROR;
	}

	bcm_binit(&bcmstrbuf, buf, buflen);
	b = &bcmstrbuf;

	corerev = mi->corerev;

	// only support for revid >= 133
	if (corerev < 133) {
		DHD_ERROR(("%s: unsupported corerev %d\n", __FUNCTION__, corerev));
		return BCME_UNSUPPORTED;
	}

	for (aqmq = MLO_AQMQ_OFFSET; aqmq < (MLO_AQMQ_OFFSET + MAX_MLO_AQMQ); aqmq++) {
		bcm_bprintf(b, "AQM[%d] BMP: encoding F=Free, A=Ack, 0:1:2:3=Link, *=SUSP\n", aqmq);
		for (i = 0; i < 16; i++) {
			dhd_macdbg_get_aqmsts(mi, aqmq, MLO_DBG_OPCODE_RDBMPST, i, results);
			bcm_bprintf(b, "  idx[%03d~%03d]", (i*32), ((i*32)+31));
			for (j = 0; j < 8; j++) {
				for (k = 0; k < 4; k++) {
					state = results[j].data[k] & 0xf;
					if (state == 6 || state == 7 || state == 14 || state == 15)
						continue;
					bcm_bprintf(b, "%c%c", (state < 8) ? ' ':'*',
						decode[state & 7]);
				}
			}
			bcm_bprintf(b, "\n");
		}
	}
	for (aqmq = MLO_AQMQ_OFFSET; aqmq < (MLO_AQMQ_OFFSET + MAX_MLO_AQMQ); aqmq++) {
		bcm_bprintf(b, "AQM[%d] Tx Cnt:\n", aqmq);
		for (i = 0; i < 16; i++) {
			dhd_macdbg_get_aqmsts(mi, aqmq, MLO_DBG_OPCODE_RDTXCNT, i, results);
			bcm_bprintf(b, "  idx[%03d~%03d]", (i*32), ((i*32)+31));
			for (j = 0; j < 8; j++) {
				for (k = 0; k < 4; k++) {
					bcm_bprintf(b, " %3d", results[j].data[k]);
				}
			}
			bcm_bprintf(b, "\n");
		}
	}
	if (outbuflen) {
		if (buflen > BCMSTRBUF_LEN(b)) {
			*outbuflen = buflen - BCMSTRBUF_LEN(b);
		} else {
			DHD_ERROR(("%s: buflen insufficient!\n", __FUNCTION__));
			*outbuflen = buflen;
			/* Do not return buftooshort to allow printing macregs we have got */
		}
	}
	return BCME_OK;
}
#else
static int
dhd_macdbg_dumpaqmsts(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;
	int corerev;
	int aqmq, i, j, k, st_free;
	mlo_stdbg_data_t results[8];
	uint32 bmp[16], bit;
	char *decode[6] = {"FR", "AK", "L0", "L1", "L2", "L3"};

	if (buf == NULL || buflen <= 0) {
		DHD_ERROR(("%s: Invalid Input Params\n", __FUNCTION__));
		return BCME_ERROR;
	}

	bcm_binit(&bcmstrbuf, buf, buflen);
	b = &bcmstrbuf;

	corerev = mi->corerev;

	// only support for revid >= 133
	if (corerev != 133) {
		DHD_ERROR(("%s: unsupported corerev %d\n", __FUNCTION__, corerev));
		return BCME_UNSUPPORTED;
	}

	bcm_bprintf(b, "MPDU State Legend: FR=Free AK=ACK L0/L1/L2/L3=Link *=SUSP\n");
	for (aqmq = MLO_AQMQ_OFFSET; aqmq < (MLO_AQMQ_OFFSET + MAX_MLO_AQMQ); aqmq++) {
		st_free = 1;
		bcm_bprintf(b, "AQM[%d]:\n", aqmq);
		for (i = 0; i < 16; i++) {
			memset(bmp, '\0', sizeof(bmp));
			bit = 1;
			dhd_macdbg_get_aqmsts(mi, aqmq, MLO_DBG_OPCODE_RDBMPST, i, results);
			for (j = 0; j < 8; j++) {
				for (k = 0; k < 4; k++) {
					bmp[results[j].data[k] & 0xf] |= bit;
					bit <<= 1;
				}
			}
			// print the results only if is not ALL Free state
			if ((bmp[0] | bmp[8]) != 0xffffffff) {
				bcm_bprintf(b, "  [%03d:%03d]:", ((i*32)+31), (i*32));
				for (j = 0; j < 16; j++) {
					// skip unsupported state and if bmp is all zero
					if (j == 6 || j == 7 || j == 14 || j == 15 || bmp[j] == 0)
						continue;
					bcm_bprintf(b, " %c%s[%04x_%04x]", (j < 8)? ' ':'*',
						decode[j % 8], bmp[j] >> 16, bmp[j] & 0xffff);
				}
				bcm_bprintf(b, "\n");
				st_free = 0;
			}
		}
		if (st_free)
			bcm_bprintf(b, "  All MPDU states are Free\n");
	}
	if (outbuflen) {
		if (buflen > BCMSTRBUF_LEN(b)) {
			*outbuflen = buflen - BCMSTRBUF_LEN(b);
		} else {
			DHD_ERROR(("%s: buflen insufficient!\n", __FUNCTION__));
			*outbuflen = buflen;
			/* Do not return buftooshort to allow printing macregs we have got */
		}
	}
	return BCME_OK;
}
#endif /* AQMSTS_DEBUG */

static int
dhd_macdbg_dumpcpudbgfifo(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen)
{
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;

	if (outbuflen) {
		*outbuflen = 0;
	}

	if (buf == NULL || buflen <= 0) {
		return BCME_ERROR;
	}

	bcm_binit(&bcmstrbuf, buf, buflen);
	b = &bcmstrbuf;

#if defined(BCMPCIE) && defined(LINUX)
	if (dhdp->cpudbg_wl_support) {
		uint32 val, read_idx, write_idx, i, global_ctl, dma_ctl;
		wl_cpudbg_entry_t wl_cpudbg_entry;
		uint32 entry_ct, len = 0;
		uint32 written, logged_entries = 0, skipped_entries = 0, invalid_entries = 0;
		macdbg_info_t *mi = dhdp->macdbg_info;
		uint32 entry[4];
		bool wraparound;
		int res;
		uint32 subbuf_entries;
		uint32 cpudbg_regsbase;

		/*
		 * Only do FIFO dump for a dongle trap since the hw needs a reset to work afterward
		 */
		if (!dhdp->dongle_trap_occured) {
			bcm_bprintf(b, "CPU DBG FIFO dump only supported for crashdumps\n");
			return BCME_OK;
		}

		if (mi->corerev > 134 && mi->corerev != 138) {
			cpudbg_regsbase = CM_CLUSTER_HADDR_BASE_REV135 + CM_CPU_DBG_REGS_OFFSET;
		} else {
			cpudbg_regsbase = CM_CLUSTER_HADDR_BASE + CM_CPU_DBG_REGS_OFFSET;
		}

		/* Disable timeout-based DMA transfers */
		dma_ctl = _dhd_get_sbreg(mi, cpudbg_regsbase + CPU_DBG_DMA_CTL, 4);
		_dhd_set_sbreg(mi, cpudbg_regsbase + CPU_DBG_DMA_CTL, 4,
			(dma_ctl & ~CPU_DBG_DMA_CTL_TO_EN));

		/* Set RDEN in CPU_DBG_GLOBAL_CTL to prevent further updates by CPUs */
		global_ctl = _dhd_get_sbreg(mi, cpudbg_regsbase + CPU_DBG_GLOBAL_CTL, 4);
		val = global_ctl | CPU_DBG_GLOBAL_CTL_RDEN;
		_dhd_set_sbreg(mi, cpudbg_regsbase + CPU_DBG_GLOBAL_CTL, 4, val);

		/* get the RdIdx and WrIdx values */
		val = _dhd_get_sbreg(mi, cpudbg_regsbase + CPU_DBG_STS0, 4);
		read_idx = val & 0x7ff;
		write_idx = (val >> 16) & 0x7ff;

		/*
		 * Must write CPU_DBG_RD_CTL before accessing entries. If the FIFO writes wrapped
		 * around, start reading at the current WrIdx to dump the oldest entries first.
		 */
		if (global_ctl & CPU_DBG_GLOBAL_CTL_DMA) {
			wraparound = dhdp->cpudbg_wraparound;
		} else {
			val = _dhd_get_sbreg(mi, cpudbg_regsbase + CPU_DBG_STS4, 4);
			wraparound = ((val & CPU_DBG_STS4_WRAPAROUND) == CPU_DBG_STS4_WRAPAROUND);
		}

		if (wraparound) {
			val = write_idx;
			entry_ct = CM_CPU_DBG_BUF_SIZE / CM_CPU_DBG_ENTRY_SIZE;
		} else {
			val = 0;
			entry_ct = write_idx;
		}
		_dhd_set_sbreg(mi, cpudbg_regsbase + CPU_DBG_RD_CTL, 4, val);

		subbuf_entries = (1 << (dhdp->cpudbg_config & CM_CPU_DBG_CFG_SZ_MASK)) /
			CM_CPU_DBG_ENTRY_SIZE;
		for (i = 0; i < entry_ct; ++i) {
			/* Skip 16-byte header if in DMA mode */
			if ((global_ctl & CPU_DBG_GLOBAL_CTL_DMA) && (i % subbuf_entries) == 0) {
				continue;
			}
			entry[0] = _dhd_get_sbreg(mi, cpudbg_regsbase + CPU_DBG_RD0, 4);
			entry[1] = _dhd_get_sbreg(mi, cpudbg_regsbase + CPU_DBG_RD1, 4);
			entry[2] = _dhd_get_sbreg(mi, cpudbg_regsbase + CPU_DBG_RD2, 4);
			entry[3] = _dhd_get_sbreg(mi, cpudbg_regsbase + CPU_DBG_RD3, 4);

			wl_cpudbg_entry.stringId.u32 = entry[0];
			wl_cpudbg_entry.param1 = entry[1];
			wl_cpudbg_entry.param2 = entry[2];
			wl_cpudbg_entry.ts = entry[3];

			if (wl_cpudbg_entry.stringId.u32 != 0) {
				written = 0;
				res = dhd_cpudbg_write_entry(dhdp, &entry[0], &written, NULL, b);
				if (res == BCME_ERROR) {
					goto done;
				} else if (res == BCME_NOTFOUND) {
					++invalid_entries;
				} else {
					len += written;
					++logged_entries;
				}
			} else {
				++skipped_entries;
			}
		}
done:
		DHD_PRINT(("wl%d: %s: dumped %u FIFO entries (wa %u), skipped %u, "
			"inval %u len %u bytes; RdIdx 0x%x; WrIdx 0x%x\n", dhdp->unit,
			__FUNCTION__, logged_entries, wraparound, skipped_entries, invalid_entries,
			len, read_idx, write_idx));
		/* restore original CTL values */
		_dhd_set_sbreg(mi, cpudbg_regsbase + CPU_DBG_GLOBAL_CTL, 4, global_ctl);
		_dhd_set_sbreg(mi, cpudbg_regsbase + CPU_DBG_DMA_CTL, 4, dma_ctl);

		if (outbuflen) {
			*outbuflen = len;
		}
		return BCME_OK;
	}
#endif /* defined(BCMPCIE) && defined(LINUX) */

	/* Intentionally return BCME_OK in this case */
	bcm_bprintf(b, "CPU DBG FIFO dump not supported\n");
	return BCME_OK;
}

static uint32
dhd_macdbg_dumpcpudbg_hostdma(dhd_pub_t *dhdp, struct bcmstrbuf *b, uint32 cpudbg_regsbase,
	uint32 m2mdma1_regsbase)
{
	uint32 val, global_ctl, dma_ctl, len = 0;
	uint16 orig_rdidx, orig_wridx;
	macdbg_info_t *mi = dhdp->macdbg_info;
	uint32 cpudbg_xfer_sz;
	uint32 skipped = 0;

	global_ctl = _dhd_get_sbreg(mi, cpudbg_regsbase + CPU_DBG_GLOBAL_CTL, 4);

	if (!(global_ctl & CPU_DBG_GLOBAL_CTL_EN) || !(global_ctl & CPU_DBG_GLOBAL_CTL_DMA)) {
		return 0;
	}

	/* Disable timeout-based DMA transfers */
	dma_ctl = _dhd_get_sbreg(mi, cpudbg_regsbase + CPU_DBG_DMA_CTL, 4);
	_dhd_set_sbreg(mi, cpudbg_regsbase + CPU_DBG_DMA_CTL, 4,
		(dma_ctl & ~CPU_DBG_DMA_CTL_TO_EN));

	/* Disable DMA & set RDEN in CPU_DBG_GLOBAL_CTL to prevent further updates */
	val = (global_ctl | CPU_DBG_GLOBAL_CTL_RDEN) & ~CPU_DBG_GLOBAL_CTL_DMA;
	_dhd_set_sbreg(mi, cpudbg_regsbase + CPU_DBG_GLOBAL_CTL, 4, val);

	orig_rdidx = dhdp->cpudbg_rdidx;
	orig_wridx = dhdp->cpudbg_wridx;
	/* Get the latest WrIdx from hw */
	dhdp->cpudbg_wridx = _dhd_get_sbreg(mi,
		m2mdma1_regsbase + M2MDMA1_CPU_DBG_WRIDX_OFFSET, 4) & 0xffff;

	cpudbg_xfer_sz = (1 << (dhdp->cpudbg_config & CM_CPU_DBG_CFG_SZ_MASK));
	if (dhdp->cpudbg_wraparound &&
		(dhdp->cpudbg_wridx != ((CPUDBG_HOSTBUF_SZ / cpudbg_xfer_sz) - 1))) {
		dhdp->cpudbg_rdidx = (dhdp->cpudbg_wridx + 1);
	} else {
		dhdp->cpudbg_rdidx = 0;
	}

	len = dhd_cpudbg_write_entries(dhdp, NULL, b, &skipped);

	DHD_PRINT(("wl%d: %s: dumped CPU DBG DMA entries (wa %u), len %u bytes; skipped %u; "
		"RdIdx %x; WrIdx %x\n", dhdp->unit, __FUNCTION__, dhdp->cpudbg_wraparound,
		len, skipped, dhdp->cpudbg_rdidx, dhdp->cpudbg_wridx));

	dhdp->cpudbg_rdidx = orig_rdidx;
	dhdp->cpudbg_wridx = orig_wridx;

	/* restore original CTL values */
	_dhd_set_sbreg(mi, cpudbg_regsbase + CPU_DBG_GLOBAL_CTL, 4, global_ctl);
	_dhd_set_sbreg(mi, cpudbg_regsbase + CPU_DBG_DMA_CTL, 4, dma_ctl);

	return len;
}

static int
dhd_macdbg_dumpcpudbg(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen)
{
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;

	if (buf == NULL || buflen <= 0) {
		return BCME_ERROR;
	}

	bcm_binit(&bcmstrbuf, buf, buflen);
	b = &bcmstrbuf;

	/*
	 * Only do DMA dump for a dongle trap since it would disrupt ongoing CPUDBG logging
	 */
	if (!dhdp->dongle_trap_occured) {
		bcm_bprintf(b, "CPU host DMA dump only supported for crashdumps\n");
		return BCME_OK;
	}

#if defined(BCMPCIE) && defined(LINUX)
	if (dhdp->cpudbg_wl_support) {
		uint32 cpudbg_regsbase;
		uint32 m2mdma1_regsbase;
		macdbg_info_t *mi = dhdp->macdbg_info;
		uint32 len;

		if (mi->corerev > 134 && mi->corerev != 138) {
			cpudbg_regsbase = CM_CLUSTER_HADDR_BASE_REV135 + CM_CPU_DBG_REGS_OFFSET;
			m2mdma1_regsbase = M2MDMA1_AXI_REGS_BASE_REV135;
		} else {
			cpudbg_regsbase = CM_CLUSTER_HADDR_BASE + CM_CPU_DBG_REGS_OFFSET;
			m2mdma1_regsbase = M2MDMA1_AXI_REGS_BASE;
		}

		len = dhd_macdbg_dumpcpudbg_hostdma(dhdp, b, cpudbg_regsbase, m2mdma1_regsbase);

		if (outbuflen) {
			*outbuflen = len;
		}
		return BCME_OK;
	}
#endif /* defined(BCMPCIE) && defined(LINUX) */

	/* Intentionally return BCME_OK in this case */
	bcm_bprintf(b, "CPU DBG dump not supported\n");
	return BCME_OK;
}

static int
dhd_macdbg_dumpcpudbgstate(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen)
{
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;

	if (buf == NULL || buflen <= 0) {
		return BCME_ERROR;
	}

	bcm_binit(&bcmstrbuf, buf, buflen);
	b = &bcmstrbuf;

#if defined(BCMPCIE) && defined(LINUX)
	if (dhdp->cpudbg_wl_support) {
		uint32 val, global_ctl, offset;
		uint32 cpudbg_regsbase;
		macdbg_info_t *mi = dhdp->macdbg_info;
		int ret;
		int len = 0;

		if (mi->corerev > 134 && mi->corerev != 138) {
			cpudbg_regsbase = CM_CLUSTER_HADDR_BASE_REV135 + CM_CPU_DBG_REGS_OFFSET;
		} else {
			cpudbg_regsbase = CM_CLUSTER_HADDR_BASE + CM_CPU_DBG_REGS_OFFSET;
		}

		global_ctl = _dhd_get_sbreg(mi, cpudbg_regsbase + CPU_DBG_GLOBAL_CTL, 4);

		if (!(global_ctl & CPU_DBG_GLOBAL_CTL_EN)) {
			return 0;
		}

		for (offset = 0; offset < CPU_DBG_REGS_MAX; offset += 4) {
			if (mi->corerev <  134 &&
			    (offset == CPU_DBG_REGS_SKIP1_REV133 ||
			     offset == CPU_DBG_REGS_SKIP2_REV133)) {
				continue;
			}
			if (offset == CPU_DBG_REGS_SKIP_REV134) {
				continue;
			}
			val = _dhd_get_sbreg(mi, cpudbg_regsbase + offset, 4);
			ret = bcm_bprintf(b, "%2u cpudbg_reg 0x%02x = 0x%08x\n", offset >> 2,
				offset, val);
			if (ret < 0) {
				break;
			}
			len += ret;
		}

		ret = bcm_bprintf(b, "\ncpu0_entry_ct %llu cpu1_entry_ct %llu cpu2_entry_ct %llu "
			"cpu3_entry_ct %llu\n"
			"cpu0_drop_ct %llu cpu1_drop_ct %llu cpu2_drop_ct %llu cpu3_drop_ct %llu\n",
			dhdp->cpudbg_ctrs.cpu0_entry_ct, dhdp->cpudbg_ctrs.cpu1_entry_ct,
			dhdp->cpudbg_ctrs.cpu2_entry_ct, dhdp->cpudbg_ctrs.cpu3_entry_ct,
			dhdp->cpudbg_ctrs.cpu0_drop_ct, dhdp->cpudbg_ctrs.cpu1_drop_ct,
			dhdp->cpudbg_ctrs.cpu2_drop_ct, dhdp->cpudbg_ctrs.cpu3_drop_ct);

		if (ret > 0) {
			len += ret;
		}

		if (outbuflen) {
			*outbuflen = len;
		}
		return BCME_OK;
	}
#endif /* defined(BCMPCIE) && defined(LINUX) */

	/* Intentionally return BCME_OK in this case */
	bcm_bprintf(b, "CPU DBG regs dump not supported\n");
	return BCME_OK;
}

void
dhd_macdbg_cpudbg_updt_rdidx(dhd_pub_t *dhdp)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	_dhd_set_sbreg(mi, dhdp->cpudbg_rdidx_daddr32, 4, htol32(dhdp->cpudbg_rdidx));
}

#ifdef BCMPCIE
#define CC_WATCHDOGCOUNTER  (0x80 / sizeof(uint32))

#define D11_MACCONTROL      (0x120 / sizeof(uint32))
#define D11_MACINTMASK      (0x12c / sizeof(uint32))
#define D11_ALTMACINTMASK   (0x16c / sizeof(uint32))
#define D11_MACCONTROLX     (0x1b4 / sizeof(uint32)) /* for PSMx */
#define D11_MACINTMASKX     (0x1c4 / sizeof(uint32))
#define D11_ALTMACINTMASKX  (0x1cc / sizeof(uint32))
#define D11_MACCONTROLR1    (0x324 / sizeof(uint32)) /* for PSMr1 */
#define D11_MACINTMASKR1    (0x334 / sizeof(uint32))
#define D11_ALTMACINTMASKR1 (0x33c / sizeof(uint32))
#define D11_OBJ_ADDR_OFFSET (0x160 / sizeof(uint32))
#define D11_OBJ_ADDR_DATA   (0x164 / sizeof(uint32))
#define D11_MACHWCAP1       (0x1A4 / sizeof(uint32))
#define D11_PHYREG_ADDR     (0x3FC / sizeof(uint16))
#define D11_PHYREG_DATA     (0x3FE / sizeof(uint16))
#define GET_SHM_SIZE(v)     ((v >> 1) & 0x7) /* in [uint32] units */

#define MCTL_PSM_RUN        (1 << 1)  /**< bitfield in MACCONTROL register */
#define MCTL_IHR_EN         (1 << 10) /**< bitfield in MACCONTROL register */
#define MCTL_WAKE	    (1 << 26)

#define OBJ_ADDR_SL_SHIFT	16 /** bitfield in d11 object access register */
#define OBJ_ADDR_SL_UCODE	(0x0 << OBJ_ADDR_SL_SHIFT)
#define OBJ_ADDR_SL_SHM		(0x1 << OBJ_ADDR_SL_SHIFT)
#define OBJ_ADDR_SL_SCR		(0x2 << OBJ_ADDR_SL_SHIFT)
#define OBJ_ADDR_SL_IHR		(0x3 << OBJ_ADDR_SL_SHIFT)
#define OBJ_ADDR_SL_AMT		(0x4 << OBJ_ADDR_SL_SHIFT)
#define OBJ_ADDR_SL_SHMX	(0x9 << OBJ_ADDR_SL_SHIFT)
#define OBJ_ADDR_SL_SCRX	(0xa << OBJ_ADDR_SL_SHIFT)
#define OBJ_ADDR_SL_IHRX	(0xb << OBJ_ADDR_SL_SHIFT)

/**
 * When dumping registers, the registers that have not been HW implemented need to be skipped.
 */
static bool
is_reg_in_list(uint16 reg_offset, CONST d11regs_list_t *regs_list, int n_reglist_entries)
{
	int i;
	uint32 byte_offset;  /**< for short hand */

	for (i = 0; i < n_reglist_entries; i++) {
		byte_offset = regs_list[i].byte_offset;
		if (reg_offset < byte_offset) {
			continue;
		}
		if (regs_list[i].bitmap == 0) {
			if (reg_offset < byte_offset + regs_list[i].cnt * regs_list[i].step) {
				return TRUE;
			}
		} else if (reg_offset < byte_offset + sizeof(uint32) * 8 * regs_list[i].step) {
			byte_offset = (reg_offset - byte_offset) / regs_list[i].step;
			if ((regs_list[i].bitmap >> byte_offset) & 1) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

/* chipcommon revs 48, 58, and 66 for 43602a3, 4366c0, and 43684b0/b1 respectively */
CONST d11regs_list_t cc_regs_revs48_58_66[] = {
	/*  type   addr      bitmap     step  cnt */
	{     0,   0x0, 0xFFC0FF0F,       4,    0   }, /*   0x0 -  0x7C */
	{     0,  0x80, 0xDF700E0F,       4,    0   }, /*  0x80 - 0x100 */
	{     0, 0x190, 0x00500007,       4,    0   }, /* 0x190 - 0x20C */
};

CONST d11regs_list_t d11_regs_rev49[] = { /* in e.g. 43602a3 */
	/*  type   addr      bitmap     step  cnt */
	{     0,  0x18, 0x00000000,       4,   14   }, /*  0x18 -  0x4C */
	{     0, 0x100, 0xFDF73F03,       4,    0   }, /* 0x100 - 0x17C */
	{     0, 0x180, 0x0700077F,       4,    0   }, /* 0x180 - 0x1FC */
	{     0, 0x200, 0x003F3F3F,       4,    0   }, /* 0x200 - 0x27C */
	{     0, 0x280, 0x0000003F,       4,    0   }, /* 0x280 - 0x28C */
	{     0, 0x300, 0x003F003F,       4,    6   }, /* 0x300 - 0x354 */
};

CONST d11regs_list_t d11_regs_rev65[] = { /* in e.g. 4366c0 */
	/*  type   addr      bitmap     step  cnt */
	{     0,  0x18, 0x00000000,       4,   14   }, /*  0x18 -  0x4C */
	{     0, 0x100, 0xF0F73F03,       4,    0   }, /* 0x100 - 0x17C */
	{     0, 0x180, 0x0700077F,       4,    0   }, /* 0x180 - 0x1FC */
	{     0, 0x200, 0x003F3F3F,       4,    0   }, /* 0x200 - 0x27C */
	{     0, 0x280, 0x0000003F,       4,    0   }, /* 0x280 - 0x28C */
	{     0, 0x300, 0x003F003F,       4,    6   }, /* 0x300 - 0x354 */
};

CONST d11regs_list_t pcieg2_regs_rev9[] = { /* in e.g. 43602a3 */
	/*  type   addr      bitmap     step  cnt */
	{     0,   0x0, 0x00FF3F3F,       4,    0   }, /*   0x0 -  0x7C */
	{     0, 0x100, 0x000F1F07,       4,    0   }, /* 0x100 - 0x17C */
	{     0, 0x180, 0x07000FFD,       4,    0   }, /* 0x180 - 0x19C */
	{     0, 0x200, 0x3F3F3F3F,       4,    0   }, /* 0x200 - 0x27C */
};

CONST d11regs_list_t pcieg2_regs_revs22_27[] = { /* in e.g. 4366c0 */
	/*  type   addr      bitmap     step  cnt */
	{     0,   0x0, 0x07FF3F7F,       4,    0   }, /*   0x0 -  0x7C */
	{     0, 0x100, 0x000F1F1F,       4,    0   }, /* 0x100 - 0x17C */
	{     0, 0x180, 0x070E0FFD,       4,    0   }, /* 0x180 - 0x19C */
	{     0, 0x200, 0x3F3F3F3F,       4,    0   }, /* 0x200 - 0x27C */
};

CONST d11regs_list_t pmu_regs_rev24[] = { /* in e.g. 43602a3 */
	/*  type   addr      bitmap     step  cnt */
	{    0,  0x600, 0xFFFEFFFF,       4,    0   }, /* 0x600 -  0x67C */
	{    0,  0x680, 0x0FFC800C,       4,    0   }, /* 0x680 -  0x6FC */
	{    0,  0x700, 0x33FF0303,       4,    0   }, /* 0x700 -  0x7FC */
};

CONST d11regs_list_t pmu_regs_rev31[] = { /* in e.g. 4366c0 */
	/*  type   addr      bitmap     step  cnt */
	{    0,  0x600, 0xFFFEFFFF,       4,    0   }, /* 0x600 -  0x67C */
	{    0,  0x680, 0x0FFC800C,       4,    0   }, /* 0x680 -  0x6FC */
	{    0,  0x700, 0x33FF0303,       4,    0   }, /* 0x700 -  0x7FC */
};

CONST d11regs_list_t ca7_regs_rev2[] = { /* in e.g. 4366c0 */
	/*  type   addr      bitmap     step  cnt */
	{     0,   0x0, 0x0000001F,       4,    0   }, /*   0x0 -  0x7C */
	{     0, 0x100, 0x00000001,       4,    0   }, /* 0x100 - 0x17C */
};

CONST d11regs_list_t cr4_regs_rev7[] = { /* in e.g. 43602a3 */
	/*  type   addr      bitmap     step  cnt */
	{     0,   0x0, 0x7C0F3FF7,       4,    0   }, /*   0x0 -  0x7C */
	{     0, 0x1e0, 0x00000005,       4,    0   }, /* 0x1e0 - 0x1eC */
};

CONST d11regs_list_t sysmem_regs_rev3[] = { /* in e.g. 4366c0 */
	/*  type   addr      bitmap     step  cnt */
	{     0,   0x0, 0x000F3FF5,       4,    0   }, /*   0x0 -  0x7C */
	{     0, 0x1e0, 0x00000000,       4,    3   }, /* 0x1e0 - 0x1e8 */
};

CONST d11regs_list_t gci_regs_rev7[] = { /* in e.g. 4366c0 */
	/*  type   addr      bitmap     step  cnt */
	{     0,   0x0, 0xFFFFFFFF,       4,    0   }, /*   0x0 -  0x7C */
	{     0, 0x100, 0xFFFFFFFF,       4,    0   }, /* 0x100 - 0x17C */
	{     0, 0x200, 0xFFFFFFFF,       4,    0   }, /* 0x200 - 0x27C */
};

CONST struct core_regs_s {
	uint core_id;               /**< id of core to dump */
	uint core_rev;              /**< rev of core to dump */
	uint n_regs_list_entries;   /**< #entries in struct member 'regs' */
	CONST d11regs_list_t *regs; /**< contains registers to dump */
} core_regs[] = {
	/* 43602a3 */
	{CC_CORE_ID,    48, ARRAYSIZE(cc_regs_revs48_58_66), cc_regs_revs48_58_66},
	{PMU_CORE_ID,   24, ARRAYSIZE(pmu_regs_rev24), pmu_regs_rev24},
	{PCIE2_CORE_ID,  9, ARRAYSIZE(pcieg2_regs_rev9), pcieg2_regs_rev9},
	{ARMCR4_CORE_ID, 7, ARRAYSIZE(cr4_regs_rev7), cr4_regs_rev7},
	{SYSMEM_CORE_ID, 3, ARRAYSIZE(sysmem_regs_rev3), sysmem_regs_rev3},
	{D11_CORE_ID,   49, ARRAYSIZE(d11_regs_rev49), d11_regs_rev49},
	/* 4366c0 */
	{CC_CORE_ID,    58, ARRAYSIZE(cc_regs_revs48_58_66), cc_regs_revs48_58_66},
	{PMU_CORE_ID,   31, ARRAYSIZE(pmu_regs_rev31), pmu_regs_rev31},
	{PCIE2_CORE_ID, 22, ARRAYSIZE(pcieg2_regs_revs22_27), pcieg2_regs_revs22_27},
	{ARMCA7_CORE_ID, 2, ARRAYSIZE(ca7_regs_rev2), ca7_regs_rev2},
	{D11_CORE_ID,   65, ARRAYSIZE(d11_regs_rev65), d11_regs_rev65},
	{GCI_CORE_ID,    7, ARRAYSIZE(gci_regs_rev7), gci_regs_rev7},
	/* 43684b0 */
	{CC_CORE_ID,    66, ARRAYSIZE(cc_regs_revs48_58_66), cc_regs_revs48_58_66},
	{PMU_CORE_ID,   35, ARRAYSIZE(pmu_regs_rev31), pmu_regs_rev31},
	{PCIE2_CORE_ID, 27, ARRAYSIZE(pcieg2_regs_revs22_27), pcieg2_regs_revs22_27},
	{ARMCA7_CORE_ID, 6, ARRAYSIZE(ca7_regs_rev2), ca7_regs_rev2},
	{D11_CORE_ID,  129, ARRAYSIZE(d11_regs_rev65), d11_regs_rev65},
	{GCI_CORE_ID,   14, ARRAYSIZE(gci_regs_rev7), gci_regs_rev7},
};

/**
 * Invoked by './dhd upload' command line. This function obtains core register dumps and d11 dumps
 * from the dongle and forwards those to user space. It does not require 'debug' enhanced firmware,
 * it can be used with 'production' firmware, and thus consumes no precious dongle memory (important
 * for e.g. the 43602). It is however only suited for PCIe and SDIO buses.
 *
 * @param outlen Length of output buffer pointed at by 'ddo' in [bytes]
 * @param ddi    Utility -> DHD direction
 * @param ddo    DHD -> Utility direction
 */
extern int
dhd_macdbg_dump_dongle(struct si_pub *sih,
	dump_dongle_in_t ddi, int outlen, dump_dongle_out_t *ddo)
{
	uint32 *p32 = ddo->u.ui32;
	uint64 *p64 = ddo->u.ui64;
	uint32 ui32;
	uint64 ui64;
	uint max_read_offset = 0; /**< in [bytes] */
	volatile uint32 *pv = NULL;
	uint obj_addr_idx = 0; /**< reg bitfield ObjAddr::Index */
	int bcmerror = BCME_OK;
	CONST struct core_regs_s *crl = NULL;  /**< points at a core register list */

	ddo->element_width = sizeof(uint16);
	ddo->address = 0; // local memory address at which this dump starts
	ddo->n_bytes = 0;
	ddo->id = 0; /**< 0: core is not available */

	switch (ddi.type) {
	case DUMP_DONGLE_COREREG:
		max_read_offset = 4096 - 1; /* one core contains max 4096/4 registers */
		ddo->element_width = sizeof(uint32);
		if (si_setcoreidx(sih, ddi.index) == NULL) {
			goto exit; // beyond last core: core enumeration ended
		}
		for (ui32 = 0; ui32 < ARRAYSIZE(core_regs); ui32++) {
			crl = &core_regs[ui32];
			if (crl->core_id == si_coreid(sih) && crl->core_rev == si_corerev(sih)) {
				break;
			}
		}
		if (ui32 == ARRAYSIZE(core_regs)) {
			crl = NULL; /* register list for given core was not found */
		}

		// backplane address at which this dump starts
		ddo->address = si_addrspace(sih, CORE_SLAVE_PORT_0, CORE_BASE_ADDR_0);
		ddo->id = si_coreid(sih);
		ddo->rev = si_corerev(sih);
		if (!si_iscoreup(sih) && ddo->id != PMU_CORE_ID) {
			printf("dump: core id 0x%x is down\n", ddo->id);
			goto exit;
		}
		break;
	case DUMP_DONGLE_D11MEM_AMT:
		pv = si_setcore(sih, D11_CORE_ID, 0);
		if (pv == NULL || !si_iscoreup(sih)) {
			printf("dump: d11 core is down\n");
			bcmerror = BCME_NOTUP;
			goto exit;
		}

		if (si_corerev(sih) < 64 && ddi.index > 0) { // d11rev < 64 has only one PSM
			goto exit;
		}

		max_read_offset = 0x100;
		ddo->element_width = sizeof(uint64);

		obj_addr_idx = ddi.byte_offset / ddo->element_width;
		break;
	default:
		printf("dump failed: unknown type %d\n", ddi.type);
		bcmerror = BCME_BADARG;
		goto exit;
	}

	ddo->address += ddi.byte_offset; // start address of this dump

	while (ddi.byte_offset < max_read_offset &&
		sizeof(dump_dongle_out_t) + ddo->n_bytes + ddo->element_width < (uint)outlen) {
		switch (ddi.type) {
		case DUMP_DONGLE_COREREG:    /* core register entries are 32 bits wide */
			if (crl && is_reg_in_list(ddi.byte_offset, crl->regs,
					crl->n_regs_list_entries)) {
				*p32++ = si_corereg(sih, ddi.index, ddi.byte_offset, 0, 0);
			} else {
				*p32++ = 0xDEADBEEF;
			}
			break;
		case DUMP_DONGLE_D11MEM_AMT: /* AMT entries are 64 bits wide */
			// ObjData accesses the 32 low bits of AMT[index/2] for even values of index
			*(pv + D11_OBJ_ADDR_OFFSET) = (obj_addr_idx | OBJ_ADDR_SL_AMT);
			ui64 = *(pv + D11_OBJ_ADDR_DATA);
			// ... and the 32 high bits of AMT[(Index-1)/2] for odd values of index
			*(pv + D11_OBJ_ADDR_OFFSET) = (++obj_addr_idx | OBJ_ADDR_SL_AMT);
			ui32 = *(pv + D11_OBJ_ADDR_DATA);
			ui64 |= ((uint64)ui32 << 32);
			*p64++ = ui64;
			break;
		}
		ddi.byte_offset += ddo->element_width;
		ddo->n_bytes += ddo->element_width;
		++obj_addr_idx;
	}

exit:
	return bcmerror;
} /* dhd_macdbg_dump_dongle */

static bool dhd_macdbg_psmx_hw_supported(uint corerev)
{
	return corerev == 65 || corerev >= 128;
}

static bool dhd_macdbg_psmr1_hw_supported(uint corerev)
{
	return corerev >= 129;
}

/**
 * PSM(s) in dongle have to be halted for consistent d11 memory (e.g. shm) dumps.
 *
 * param sih	silicon interface handle
 *
 * @return  BCME_OK on success
 */
extern int
dhd_macdbg_halt_psms(struct si_pub *sih)
{
	volatile uint32 *pv = NULL;        /**< pointer to read/write core registers */
	uint corerev;

	pv = si_setcore(sih, D11_CORE_ID, 0);
	if (pv == NULL || !si_iscoreup(sih)) {
		printf("dump failed: d11 core is down\n");
		return BCME_NOTUP;
	}

	corerev = si_corerev(sih);

	/* disable d11 irqs to avoid watchdog reset */
	*(pv + D11_MACINTMASK) = 0;
	*(pv + D11_ALTMACINTMASK) = 0;

	/* reset mac control */
	*(pv + D11_MACCONTROL) = MCTL_IHR_EN | MCTL_WAKE;

	if (dhd_macdbg_psmx_hw_supported(corerev)) {
		*(pv + D11_MACINTMASKX) = 0;
		*(pv + D11_ALTMACINTMASKX) = 0;
		*(pv + D11_MACCONTROLX) = MCTL_IHR_EN;
	}

	if (dhd_macdbg_psmr1_hw_supported(corerev)) {
		*(pv + D11_MACINTMASKR1) = 0;
		*(pv + D11_ALTMACINTMASKR1) = 0;
		*(pv + D11_MACCONTROLR1) = MCTL_IHR_EN | MCTL_WAKE;
	}

	return BCME_OK;
}
#endif /* BCMPCIE */

bool
dhd_d11_iscoreup(dhd_pub_t *dhdp)
{
	bool isup = FALSE;
	si_t *sih;
	int idx;

	sih = dhd_bus_sih(dhdp->bus);
	if (sih == NULL)
		return FALSE;

	idx = si_coreidx(sih);
	if (si_setcore(sih, D11_CORE_ID, 0) != NULL) {
		if (si_iscoreup(sih)) {
			isup = TRUE;
		}
		si_setcoreidx(sih, idx);
	}

	return isup;
}

uint32
dhd_macdbg_corerev(dhd_pub_t *dhdp)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	return mi->corerev;
}

uint32
dhd_macdbg_bmc_tpl_idx(uint32 corerev)
{
	return BMC_TPL_IDX(corerev);
}

#define PUQ_TX_FIFOS(_d11rev)	256

uint32
dhd_macdbg_puq_tx_fifos(uint32 corerev)
{
	return PUQ_TX_FIFOS(corerev);
}

void
dhd_macdbg_panic_dump(dhd_pub_t *dhdp)
{
	macdbg_info_t *mi = dhdp->macdbg_info;

	if (!mi || !mi->active_dumpname)
		return;

	DHD_ERROR(("[%d] MACDBG: ACTIVE <%s>\n", dhdp->unit, mi->active_dumpname));
	if (mi->last_access.offset) {
		DHD_ERROR(("       ACCESS <%s:0x%04x>\n", regname[mi->last_access.func],
			mi->last_access.offset));
	}
}
