/* D11 macdbg functions for Broadcom 802.11abgn
 * Networking Adapter Device Drivers.
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 * $Id: dhd_macdbg.c 806979 2022-01-10 22:48:59Z $
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

#define MACDBG_RLM_MAX_ENTRIES		256
#define MACDBG_RLM_BCMC_ENTRY		63
#define MACDBG_RLM_VBSS_START		236
#define MACDBG_RLM_VBSS_END		251
#define MACDBG_RLM_TWT_START		252
#define MACDBG_RLM_TWT_END		255
#define MACDBG_RLM_ULOFDMA_START	214
#define MACDBG_RLM_ULOFDMA_END		245
#define MACDBG_RLM_DLOFDMA_START	250
#define MACDBG_RLM_DLOFDMA_END		255
#define MACDBG_RLM_VHTMU_START		246
#define MACDBG_RLM_VHTMU_END		249

#define MACREG_PHY_REG_ADDR		0x3FC
#define MACREG_PHY_REG_DATA		0x3FE
#define PHYREG_TABLE_ID			0xD
#define PHYREG_TABLE_OFFSET		0xE
#define PHYREG_TABLE_DATA_LO		0xF
#define PHYREG_TABLE_DATA_HI		0x10
#define PHYREG_TABLE_DATA_WIDE		0x11
#define MACREG_PSM_RATEMEM_DBG		0x872

typedef struct _macdbg_info_t {
	dhd_pub_t *dhdp;
	uint8 *dumpbuf, *dumpbufcurp;
	int dumpbuflen;
	uint8 *pd11regxtlv;
	uint16 d11regxtlv_bsz;
	uint32 corerev;
	uint32 phyrev;
	int rlm_max_index;
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

/* Sample capture related definitions */
#define UCREVMAJOR_OFFSET	0x0
#define UCREVMINOR_OFFSET	0x2
#define UCFEATURES_OFFSET	0xa

#define	PHY_TYPE_AC		11
#define	PHY_TYPE_AX		13

#define MCTL1_OFFSET		0x1a0
#define TPL_SCPCTL_OFFSET(_d11rev) \
		((_d11rev) >= 132 ? 0xf26 : \
		 ((_d11rev) >= 128 ? 0xade : \
		  (((_d11rev) >= 54 || (_d11rev) == 50) ? 0xb2e : 0x492)))
#define TPL_STRTPTR_OFFSET(_d11rev) \
		((_d11rev) >= 132 ? 0xf4a : \
		 ((_d11rev) >= 128 ? 0xaa6 : 0x552))
#define TPL_STOPPTR_OFFSET(_d11rev) \
		((_d11rev) >= 132 ? 0xf4c : \
		 ((_d11rev) >= 128 ? 0xaa8 : 0x554))
#define TPL_CURPTR_OFFSET(_d11rev) \
		((_d11rev) >= 132 ? 0xf48 : \
		 ((_d11rev) >= 128 ? 0xaa2 : 0x556))
#define TPL_WRPTR_OFFSET(_d11rev)	((_d11rev) >= 132 ? 0x36c : 0x130)
#define TPL_WRDATA_OFFSET(_d11rev)	((_d11rev) >= 132 ? 0x370 : 0x134)

/* corerev >= 129 specific regs */
#define TPL_SCPCTL2_OFFSET	0xa28
#define PSM_SCPCTL_OFFSET	0xf06
#define PSM_SCPCTL2_OFFSET	0xf2e
#define PSM_STRTPTR_OFFSET	0xf2a
#define PSM_STOPPTR_OFFSET	0xf2c
#define PSM_CURPTR_OFFSET	0xf28
#define PSM_WRPTR_OFFSET	0x36c
#define PSM_WRDATA_OFFSET	0x370

/* per user dump */
#define MPDUS_DUMP_NUM		0x8
#define MAX_NUSR(_d11rev) \
		(((_d11rev) == 129 || (_d11rev) == 132) ? 16 : \
		 (((_d11rev) == 130 || (_d11rev) == 131) ? 4 : 0))

#define SCPCTL_OFFSET(_sct, _d11rev) \
	((_sct) == DUMP_SC_TYPE_PSM ? PSM_SCPCTL_OFFSET : TPL_SCPCTL_OFFSET(_d11rev))
#define SCPCTL2_OFFSET(_sct) \
	((_sct) == DUMP_SC_TYPE_PSM ? PSM_SCPCTL2_OFFSET : TPL_SCPCTL2_OFFSET)
#define STRTPTR_OFFSET(_sct, _d11rev) \
	((_sct) == DUMP_SC_TYPE_PSM ? PSM_STRTPTR_OFFSET : TPL_STRTPTR_OFFSET(_d11rev))
#define STOPPTR_OFFSET(_sct, _d11rev) \
	((_sct) == DUMP_SC_TYPE_PSM ? PSM_STOPPTR_OFFSET : TPL_STOPPTR_OFFSET(_d11rev))
#define CURPTR_OFFSET(_sct, _d11rev) \
	((_sct) == DUMP_SC_TYPE_PSM ? PSM_CURPTR_OFFSET : TPL_CURPTR_OFFSET(_d11rev))
#define WRPTR_OFFSET(_sct, _d11rev) \
	((_sct) == DUMP_SC_TYPE_PSM ? PSM_WRPTR_OFFSET : TPL_WRPTR_OFFSET(_d11rev))
#define WRDATA_OFFSET(_sct, _d11rev) \
	((_sct) == DUMP_SC_TYPE_PSM ? PSM_WRDATA_OFFSET : TPL_WRDATA_OFFSET(_d11rev))

#define DUMP_SC_TYPE_TPL	0
#define DUMP_SC_TYPE_UTSHM	1
#define DUMP_SC_TYPE_UTSHMX	2
#define DUMP_SC_TYPE_PSM	3

/* IHR dump */
#define PIHR_BASE		0x400
#define PIHR2K_BASE		(PIHR_BASE + 0x600*2)

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

	dhdp->macdbg_info = mi;
	return BCME_OK;
}

void
dhd_macdbg_detach(dhd_pub_t *dhdp)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	ASSERT(mi);

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
			dhd_schedule_trap_log_dump(dhdp, false);
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
}

int
dhd_macdbg_dump(dhd_pub_t *dhdp, char *buf, int buflen, const char *name)
{
	int res = BCME_BADARG;

#ifdef BCMPCIE
	if (!dhd_macdbg_iscoreup(dhdp)) {
		DHD_ERROR(("%s: Cannot do registers dump due to d11 core is down\n", __FUNCTION__));
		res = BCME_NOTUP;
		goto exit;
	}
#endif /* BCMPCIE */

	/*
	 * No need to update this for new dumps. The pre-processor expansion
	 * will produce code as defined by MACDBGDUMP_ENUMDEF for each dump
	 * as listed in MACDBGDUMP_ENUMDEF_LIST.
	 *
	 * MACDBGDUMP_ENUMDEF is defined below to produce comparison and\
	 * function call for each dump entry.
	 */
#define MACDBGDUMP_ENUMDEF(_id, _str) \
	if (strncmp(name, #_str, strlen(name)) == 0) { \
		res = dhd_macdbg_dump ## _str(dhdp, buf, buflen, NULL); \
		goto exit; \
	}

	MACDBGDUMP_ENUMDEF_LIST

#undef MACDBGDUMP_ENUMDEF

exit:
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

	val = _dhd_get_sbreg(mi, (0x1000 | addr), 2);
#ifdef BCMDBG
	if (verbose) {
		if (b) {
			bcm_bprintf(b, "DEBUG: IHR16: read 0x%08x, size 2, value 0x%04x\n",
				(addr + 0x18001000), val);
		} else {
			printf("DEBUG: IHR16: read 0x%08x, size 2, value 0x%04x\n",
				(addr + 0x18001000), val);
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

	val = _dhd_get_sbreg(mi, (0x1000 | addr), 4);
#ifdef BCMDBG
	if (verbose) {
		if (b) {
			bcm_bprintf(b, "DEBUG: IHR32: read 0x%08x, size 4, value 0x%08x\n",
				(addr + 0x18001000), val);
		} else {
			printf("DEBUG: IHR32: read 0x%08x, size 4, value 0x%08x\n",
				(addr + 0x18001000), val);
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
#ifdef BCMDBG
	if (verbose) {
		if (b) {
			bcm_bprintf(b, "DEBUG: IHR16: write 0x%08x, size 2, value 0x%04x\n",
				(addr + 0x18001000), val);
		} else {
			printf("DEBUG: IHR16: write 0x%08x, size 2, value 0x%04x\n",
				(addr + 0x18001000), val);
		}
	}
#else
	BCM_REFERENCE(verbose);
#endif /* BCMDBG */
	_dhd_set_sbreg(mi, (0x1000 | addr), 2, val);
}

static void
_dhd_set_ihr32(macdbg_info_t *mi, uint16 addr, uint32 val,
	struct bcmstrbuf *b, bool verbose)
{
#ifdef BCMDBG
	if (verbose) {
		if (b) {
			bcm_bprintf(b, "DEBUG: IHR32: write 0x%08x, size 4, value 0x%08x\n",
				(addr + 0x18001000), val);
		} else {
			printf("DEBUG: IHR32: write 0x%08x, size 4, value 0x%08x\n",
				(addr + 0x18001000), val);
		}
	}
#else
	BCM_REFERENCE(verbose);
#endif /* BCMDBG */
	_dhd_set_sbreg(mi, (0x1000 | addr), 4, val);
}

static uint32
_dhd_get_d11obj32(macdbg_info_t *mi, uint16 objaddr, uint32 sel,
	struct bcmstrbuf *b, bool verbose)
{
	uint32 val;
	sdreg_t sdreg;
	sdreg.func = 4; // 4bytes by default.
	sdreg.offset = 0x1160;

	if (objaddr == 0xffff) {
#ifdef BCMDBG
		if (verbose) {
			goto objaddr_read;
		} else {
			goto objdata_read;
		}
#else
		BCM_REFERENCE(verbose);
		goto objdata_read;
#endif /* BCMDBG */
	}

	if (objaddr & 0x3) {
		printf("%s: ERROR! Invalid addr 0x%x\n", __FUNCTION__, objaddr);
	}

	sdreg.value = (sel | (objaddr >> 2));
#ifdef BCMDBG
	if (verbose) {
		if (b) {
			bcm_bprintf(b, "DEBUG: %s: Indirect: write 0x%08x, size %d, value 0x%08x\n",
				(sel & 0x00020000) ? "SCR":"SHM",
				(sdreg.offset + 0x18000000), sdreg.func, sdreg.value);
		} else {
			printf("DEBUG: %s: Indirect: write 0x%08x, size %d, value 0x%08x\n",
				(sel & 0x00020000) ? "SCR":"SHM",
				(sdreg.offset + 0x18000000), sdreg.func, sdreg.value);
		}
	}
#endif /* BCMDBG */
	BUS_IOVAR_OP(mi->dhdp, "sbreg",
		NULL, 0, &sdreg, sizeof(sdreg), IOV_SET);

#ifdef BCMDBG
objaddr_read:
#endif /* BCMDBG */
	/* Give some time to obj addr register */
	BUS_IOVAR_OP(mi->dhdp, "sbreg",
		&sdreg, sizeof(sdreg), &val, sizeof(val), IOV_GET);
#ifdef BCMDBG
	if (verbose) {
		if (b) {
			bcm_bprintf(b, "DEBUG: %s: Indirect: Read 0x%08x, size %d, value 0x%08x\n",
				(sel & 0x00020000) ? "SCR":"SHM",
				(sdreg.offset + 0x18000000), sdreg.func, val);
		} else {
			printf("DEBUG: %s: Indirect: Read 0x%08x, size %d, value 0x%08x\n",
				(sel & 0x00020000) ? "SCR":"SHM",
				(sdreg.offset + 0x18000000), sdreg.func, val);
		}
	}
#endif /* BCMDBG */

objdata_read:
	sdreg.offset = 0x1164;
	BUS_IOVAR_OP(mi->dhdp, "sbreg",
		&sdreg, sizeof(sdreg), &val, sizeof(val), IOV_GET);
#ifdef BCMDBG
	if (verbose) {
		if (b) {
			bcm_bprintf(b, "DEBUG: %s: Indirect: Read 0x%08x, size %d, value 0x%04x\n",
				(sel & 0x00020000) ? "SCR":"SHM",
				(sdreg.offset + 0x18000000), sdreg.func, val);
		} else {
			printf("DEBUG: %s: Indirect: Read 0x%08x, size %d, value 0x%04x\n",
				(sel & 0x00020000) ? "SCR":"SHM",
				(sdreg.offset + 0x18000000), sdreg.func, val);
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

static int
_dhd_print_d11reg(macdbg_info_t *mi, int idx, int type, uint16 addr, struct bcmstrbuf *b,
	d11obj_cache_t *obj_cache, bool verbose)
{
	const char *regname[D11REG_TYPE_MAX] = D11REGTYPENAME;
	uint32 val;

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
			if (addr < PIHR2K_BASE) {
				val = _dhd_get_ihr16(mi, addr, b, verbose);
			} else {
				val = _dhd_get_d11obj16(mi, (addr - PIHR_BASE) << 1, 0x02030000,
					obj_cache, b, verbose);
			}
			break;
		}
		case D11REG_TYPE_IHRX16:
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
		default:
			printf("Unrecognized type %d!\n", type);
			return 0;
		}
		if (b) {
			bcm_bprintf(b, "%-3d %s 0x%-4x = 0x%-4x\n",
				idx, regname[type], addr, val);
		} else {
			printf("%-3d %s 0x%-4x = 0x%-4x\n",
				idx, regname[type], addr, val);
		}
	}
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
		for (; idx < pregs->cnt; idx++) {
			_dhd_print_d11reg(mi, (idx + start_idx), pregs->type,
				byte_offset, b, &obj_cache, verbose);
			byte_offset += pregs->step;
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
			idx += _dhd_print_d11regs(mi, &reglist[i], idx, b, FALSE);
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

	gpio_sel = _dhd_get_ihr32(mi, MCTL1_OFFSET, b, FALSE);
	scpctl = _dhd_get_ihr16(mi, SCPCTL_OFFSET(sctype, corerev), b, FALSE);
	addr0 = _dhd_get_ihr16(mi, STRTPTR_OFFSET(sctype, corerev), b, FALSE);
	addr1 = _dhd_get_ihr16(mi, STOPPTR_OFFSET(sctype, corerev), b, FALSE);
	/* stop sample capture */
	_dhd_set_ihr16(mi, SCPCTL_OFFSET(sctype, corerev), 0, b, FALSE);
	curptr = _dhd_get_ihr16(mi, CURPTR_OFFSET(sctype, corerev), b, FALSE);
	byt_offset = 4 * (curptr - addr0);
	len = (addr1 - addr0 + 1) * 4;
	/* set template area offset with auto increment */
	while ((_dhd_get_ihr32(mi, WRPTR_OFFSET(sctype, corerev), b, FALSE) & 3));
	_dhd_set_ihr32(mi, WRPTR_OFFSET(sctype, corerev), ((addr0 * 4) | 2), b, FALSE);

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
		if (corerev >= 129) {
			bcm_bprintf(b, " scpctl2 0x%02x\n",
				_dhd_get_ihr16(mi, SCPCTL2_OFFSET(sctype), b, FALSE));
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
		if (corerev >= 129) {
			printf("scpctl2 0x%02x\n",
				_dhd_get_ihr16(mi, SCPCTL2_OFFSET(sctype), b, FALSE));
		} else {
			printf("\n");
		}
		printf("Start/stop/cur 0x%04x 0x%04x 0x%04x byt_offset 0x%05x entries %u\n",
		       addr0, addr1, curptr, byt_offset, len >> 2);
		printf("offset: low high\n");
	}

	for (i = 0; i < len; i += 4) {
		val_l = _dhd_get_ihr32(mi, WRDATA_OFFSET(sctype, corerev), b, FALSE);
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
			/* XXX: Is this needed?
			 * _dhd_set_ihr32(mi, MACREG_PHY_REG_ADDR, 0x30000d, b, verbose);
			 */
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
	svmp_list_t tmp;

	if (svmplist != NULL && svmplist_sz > 0) {
		for (i = 0; i < svmplist_sz; i++) {
			svmp_addr = svmplist[i].addr;
			DHD_TRACE(("%s %p %d\n", __FUNCTION__, &svmplist[i], svmplist_sz));

			if (svmp_addr >= 0x10000 && svmp_addr < 0x20000) {
				step = 16, svmp_len = 16, offset_size = 32;
			} else if (svmp_addr >= 0xe000 && svmp_addr < 0x10000) {
				step = 11, svmp_len = 16, offset_size = 16;
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

typedef struct _d11phyregs_list {
	uint16 addr;
	uint16 cnt;
} d11phyregs_list_t;

static CONST d11phyregs_list_t phyreg_list_all[] = {
	{0x1f0, 2},
	{0x60d, 1},
	{0x80d, 1},
	{0xa0d, 1},
	{0x601, 1},
	{0x801, 1},
	{0xa01, 1},
	{0x3ec, 1},
	{0x003, 1},
};

static CONST d11phyregs_list_t phyreg_list_r32_r33[] = {
	{0xc01, 1},
	{0xc0d, 1},
	{0x1049, 10},
	{0x10EC,  4},
	{0x11C0, 3},
};

static CONST d11phyregs_list_t phyreg_list_r47[] = {
	{0xc0d, 1},
	{0x1301, 1},
	{0x1311, 1},
	{0xc01, 1},
	{0x107d, 1},
	{0x1049, 10},
	{0x10EC,  4},
	{0x1110,  9},
	{0x11C0, 16},
	{0x1290, 77},
	{0x1400,  4},
	{0x1405, 17},
	{0x39f,   2},
	{0x1368,  6},
	{0x55c,   3},
	{0x439,  22}
};

static CONST d11phyregs_list_t phyreg_list_r130[] = {
	{0x1380, 26}
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

	for (m = 0; m < ARRAYSIZE(phyreg_list_all); m++) {
		for (n = 0; n < phyreg_list_all[m].cnt; n++) {
			cnt += _dhd_print_phyreg(mi, phyreg_list_all[m].addr + n, b, FALSE);
		}
	}

	if (mi->phyrev == 32 || mi->phyrev == 33) {
		for (m = 0; m < ARRAYSIZE(phyreg_list_r32_r33); m++) {
			for (n = 0; n < phyreg_list_r32_r33[m].cnt; n++) {
				cnt += _dhd_print_phyreg(mi, phyreg_list_r32_r33[m].addr + n,
						b, FALSE);
			}
		}
		if (mi->phyrev == 33) {
			for (n = 0x1110; n < 0x1119; n++) {
				cnt += _dhd_print_phyreg(mi, n, b, FALSE);
			}
		}
	}
	if (mi->phyrev >= 47) {
		for (m = 0; m < ARRAYSIZE(phyreg_list_r47); m++) {
			for (n = 0; n < phyreg_list_r47[m].cnt; n++) {
				cnt += _dhd_print_phyreg(mi, phyreg_list_r47[m].addr + n,
						b, FALSE);
			}
		}
	}

	if (mi->phyrev == 130) {
		for (m = 0; m < ARRAYSIZE(phyreg_list_r130); m++) {
			for (n = 0; n < phyreg_list_r130[m].cnt; n++) {
				cnt += _dhd_print_phyreg(mi, phyreg_list_r130[m].addr + n,
						b, FALSE);
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
		cnt += _dhd_print_phytable(mi, 46, 0x800, 32, 32,  b, FALSE);
	} else {
		printf("SMC is running\n");
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
	uint32 addr, val, offset = 0;
	int i, j, size;

	if (rmem) {
		size = MACDBG_RLM_RATE_ENTRY_SIZE;
		addr = MACDBG_RATE_OFFSET(idx);
		bcm_bprintf(b, "RateMem Block%s: ", pingpong_str);
	} else {
		if (mi->corerev >= 132) {
			size = MACDBG_RLM_LINK_ENTRY_SIZE_REV132;
		} else {
			size = MACDBG_RLM_LINK_ENTRY_SIZE;
		}
		addr = MACDBG_LINK_OFFSET(mi->corerev, idx, size);
		bcm_bprintf(b, "LinkMem Block: ");
	}
	bcm_bprintf(b, "%d Size:%dB\n", idx, size);

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

static int
dhd_macdbg_dumpperuser(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen)
{
	macdbg_info_t *mi = dhdp->macdbg_info;
	uint16 reglist_sz, cnt = 0;
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;
	uint16 regmux, csb_req, qmap;
	int i, j, num_txdbg, num_mpdus;

	uint16 txdbg_sel[] = {
		0x0000, 0x0001, 0x0002,
		0x0100, 0x0110, 0x2000,
		0x0210, 0x0300, 0x0310
	};

	d11regs_list_t reglist[] = {
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
		{D11REG_TYPE_IHR16, 0xe0a, 0x0180001f, 2, 0}
	};

	d11regs_list_t per_mpdu_list[] = {
		{D11REG_TYPE_IHR16, 0xd64, 0x00000fff, 2, 0}
	};

	d11regs_list_t csb_reglist0[] = {
		{D11REG_TYPE_IHR16, 0xd60, 0x00040001, 2, 0}
	};

	d11regs_list_t csb_reglist2_3[] = {
		{D11REG_TYPE_IHR16, 0xd60, 0x01e00001, 2, 0}
	};

	d11regs_list_t csb_reglist5[] = {
		{D11REG_TYPE_IHR16, 0xd60, 0x40000001, 2, 0}
	};

	num_txdbg = ARRAYSIZE(txdbg_sel);
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

	for (i = 0; i < MAX_NUSR(mi->corerev); i++) {
		_dhd_set_ihr16(mi, 0x7b0, (0xff00 | i), b, FALSE);

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
			cnt += _dhd_pd11regs_bylist(mi, per_mpdu_list, 1, b);
		}

		/* txdbg_sel */
		for (j = 0; j < num_txdbg; j++) {
			_dhd_set_ihr16(mi, 0xa86, txdbg_sel[j], b, FALSE);
			if (b) {
				bcm_bprintf(b, "0x%x\n", _dhd_get_ihr16(mi, 0xa88, b, FALSE));
			} else {
				printf("0x%x\n", _dhd_get_ihr16(mi, 0xa88, b, FALSE));
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
				cnt += _dhd_pd11regs_bylist(mi, csb_reglist0, 1, b);
			} else if (j == 2 || j == 3) {
				cnt += _dhd_pd11regs_bylist(mi, csb_reglist2_3, 1, b);
			} else {
				cnt += _dhd_pd11regs_bylist(mi, csb_reglist5, 1, b);
			}
		}
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

	/* set rmem to ping */
	_dhd_set_ihr16(mi, MACREG_PSM_RATEMEM_DBG, 4, b, FALSE);
	_dhd_print_rlmemblk(mi, b, rtidx, TRUE, " (Ping)");
	/* set rmem to pong */
	_dhd_set_ihr16(mi, MACREG_PSM_RATEMEM_DBG, 0xc, b, FALSE);
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

	_dhd_print_rlmemblk(mi, b, MACDBG_RLM_BCMC_ENTRY, TRUE, "");
	_dhd_print_rlmemblk(mi, b, MACDBG_RLM_BCMC_ENTRY, FALSE, "");
	bcm_bprintf(b, "\n");

	/* Always dumping TWT linkmem; Note that these entries may not be in use and dump
	 * may show unexpected values
	 */
	for (idx = MACDBG_RLM_TWT_START; idx <= MACDBG_RLM_TWT_END; idx++) {
		_dhd_print_rlmemblk(mi, b, idx, FALSE, "");
		bcm_bprintf(b, "\n");
	}
	/* Always dumping VBSS rate/linkmem; Note that these entries may not be in use and dump
	 * may show unexpected values
	 */
	for (idx = MACDBG_RLM_VBSS_START; idx <= MACDBG_RLM_VBSS_END; idx++) {
		_dhd_print_rlmemblk(mi, b, idx, TRUE, "");
		_dhd_print_rlmemblk(mi, b, idx, FALSE, "");
		bcm_bprintf(b, "\n");
	}

	for (idx = 0; idx <= mi->rlm_max_index; idx++) {
		_dhd_print_rlmemblk(mi, b, idx, TRUE, "");
		_dhd_print_rlmemblk(mi, b, idx, FALSE, "");
		bcm_bprintf(b, "\n");
	}

	/* Print the ping and pong of ratemem */
	dhd_macdbg_dumpratemem_pingpong(dhdp, MACDBG_RLM_BCMC_ENTRY, b);
	for (idx = 0; idx <= mi->rlm_max_index; idx++) {
		dhd_macdbg_dumpratemem_pingpong(dhdp, idx, b);
	}
	for (idx = MACDBG_RLM_ULOFDMA_START; idx <= MACDBG_RLM_ULOFDMA_END; idx++) {
		dhd_macdbg_dumpratemem_pingpong(dhdp, idx, b);
	}
	for (idx = MACDBG_RLM_VHTMU_START; idx <= MACDBG_RLM_VHTMU_END; idx++) {
		dhd_macdbg_dumpratemem_pingpong(dhdp, idx, b);
	}
	for (idx = MACDBG_RLM_DLOFDMA_START; idx <= MACDBG_RLM_DLOFDMA_END; idx++) {
		dhd_macdbg_dumpratemem_pingpong(dhdp, idx, b);
	}

	return BCME_OK;
}

#define RXBMDUMPSZ		0xc000
#define RXBM_ADDR_REV129	0x580000	/* starting offset: 5.5MB */
#define RXBM_ADDR_REV132	0x380000	/* starting offset: 3.5MB */
#define RXBM_SZ			0x180000	/* length: 1.5MB */

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

	/* validation check */
	if (mi->corerev == 129) {
		params[0] = RXBM_ADDR_REV129;
	} else if (mi->corerev == 132) {
		params[0] = RXBM_ADDR_REV132;
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

	rem_sz = RXBM_SZ; /* remaining byte size */
	while (rem_sz > 0) {
		memset(membytes_buf, '\0', RXBMDUMPSZ);
		memset(buf, '\0', buflen);
		membytes_sz = MIN(RXBMDUMPSZ, rem_sz);

		res = BUS_IOVAR_OP(dhdp, "membytes",
			params, sizeof(params), membytes_buf, membytes_sz, IOV_GET);
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
			dhd_write_file(dhdp, filename, buf);
			DHD_OS_WAKE_LOCK(dhdp);
			DHD_LOCK(dhdp);
		} else
#endif /* LINUX */
		{
			printf("%s\n", buf);
		}

		rem_sz -= RXBMDUMPSZ;
		params[0] += RXBMDUMPSZ;
	}

	MFREE(dhdp->osh, membytes_buf, RXBMDUMPSZ);

	return res;
}
static int
dhd_macdbg_dumpbmc(dhd_pub_t *dhdp, char *buf, int buflen, int *outbuflen)
{
#define D11MAC_BMC_MAXFIFOS_GE128		73
	macdbg_info_t *mi = dhdp->macdbg_info;
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;
	int corerev;
	uint16 *pbuf = NULL, *p;
	int i, j, num_of_fifo;
	/* specify dump which queues */
	uint32 fifo_bmp[D11MAC_BMC_MAXFIFOS_GE128/32 + 1];
	int fifo_phys;
	int *bmc_fifo_list;
	uint tpl_idx, rxq0_idx, rxq1_idx, fifoid;
	int num_statsel;
	uint8 statsel_nbit, bmask_fifosel, seltype;
	static int bmc_fifo_list_ge128[73] =
		{70, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
		31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46,
		47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
		63, 64, 65, 66, 67, 68, 69, 71, 72};

	if (buf && buflen > 0) {
		bcm_binit(&bcmstrbuf, buf, buflen);
		b = &bcmstrbuf;
	}
	corerev = mi->corerev;
	tpl_idx = 70;
	rxq0_idx = 71;
	rxq1_idx = 72;
	num_statsel = 13;
	statsel_nbit = 9;
	bmask_fifosel = 0x7f;
	bmc_fifo_list = bmc_fifo_list_ge128;

	/* validation check */
	if (corerev < 129) {
		DHD_ERROR(("%s: unsupported corerev %d\n", __FUNCTION__, corerev));
		return BCME_UNSUPPORTED;
	}

	if ((pbuf = (uint16*) MALLOCZ(dhdp->osh, num_statsel*sizeof(uint16))) == NULL) {
		DHD_ERROR(("wl: %s: MALLOC failure\n", __FUNCTION__));
		return BCME_NOMEM;
	}

	num_of_fifo = D11MAC_BMC_MAXFIFOS_GE128;
	for (i = 0; i < D11MAC_BMC_MAXFIFOS_GE128/32+1; i++) {
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
		"11-nOccupied 12-nAvalTid\n", "");
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

#define OBJ_ADDR_SL_SHIFT 16 /** bitfield in d11 object access register */
#define OBJ_ADDR_SL_UCODE (0 << OBJ_ADDR_SL_SHIFT)
#define OBJ_ADDR_SL_SHM   (1 << OBJ_ADDR_SL_SHIFT)
#define OBJ_ADDR_SL_SCR   (2 << OBJ_ADDR_SL_SHIFT)
#define OBJ_ADDR_SL_IHR   (3 << OBJ_ADDR_SL_SHIFT)
#define OBJ_ADDR_SL_AMT   (4 << OBJ_ADDR_SL_SHIFT)
#define OBJ_ADDR_SL_SHMx  (9 << OBJ_ADDR_SL_SHIFT)
#define OBJ_ADDR_SL_SCRx  (10 << OBJ_ADDR_SL_SHIFT)
#define OBJ_ADDR_SL_IHRx  (11 << OBJ_ADDR_SL_SHIFT)
#endif /* BCMPCIE */

#ifdef BCMPCIE
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
		/*
		 * XXX no idea why for psmx MCTL_WAKE is not set but that
		 * is what is in wlc_mctrlx_reset() so following that.
		 */
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

bool
dhd_macdbg_iscoreup(dhd_pub_t *dhdp)
{
	bool isup = FALSE;
	si_t *sih;
	int idx;

	sih = dhd_bus_sih(dhdp->bus);
	ASSERT(sih);

	idx = si_coreidx(sih);
	if (si_setcore(sih, D11_CORE_ID, 0) != NULL) {
		if (si_iscoreup(sih)) {
			isup = TRUE;
		}
		si_setcoreidx(sih, idx);
	}

	return isup;
}
#endif /* BCMPCIE */
