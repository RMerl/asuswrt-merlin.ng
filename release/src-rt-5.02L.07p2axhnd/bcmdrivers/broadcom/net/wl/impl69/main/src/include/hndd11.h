/*
 * Generic functions for d11 access
 *
 * Copyright (C) 2021, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: hndd11.h 787082 2020-05-18 09:21:51Z $
 */

#ifndef	_hndd11_h_
#define	_hndd11_h_

#include <typedefs.h>
#include <osl_decl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <d11.h>
#include <bcm_mpool_pub.h>
/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

#ifndef WL_RSSI_ANT_MAX
#define WL_RSSI_ANT_MAX		4	/**< max possible rx antennas */
#elif WL_RSSI_ANT_MAX != 4
#error "WL_RSSI_ANT_MAX does not match"
#endif // endif

/* SW RXHDR + HW RXHDR */
typedef struct wlc_d11rxhdr wlc_d11rxhdr_t;
BWL_PRE_PACKED_STRUCT struct wlc_d11rxhdr {
	/* SW header */
	uint32	tsf_l;		/**< TSF_L reading */
	int8	rssi;		/**< computed instantaneous rssi */
	int8	rssi_qdb;	/**< qdB portion of the computed rssi */
	uint8	pad[2];
	int8	rxpwr[ROUNDUP(WL_RSSI_ANT_MAX,2)];	/**< rssi for supported antennas */
#if defined(STS_XFER)
	d11phyrxsts_t *d11phyrxsts;	/** PhyRx Status buffer for rev GE129 */
#endif /* STS_XFER */
	/**
	 * Even though rxhdr can be in short or long format, always declare it here
	 * to be in long format. So the offsets for the other fields are always the same.
	 */
	d11rxhdr_t rxhdr;
} BWL_POST_PACKED_STRUCT;

/* Length of SW header */
#define WLC_RXHDR_LEN		(OFFSETOF(wlc_d11rxhdr_t, rxhdr))
/* Length of RX headers - SW header + HW/ucode/PHY RX status */
#define WL_RXHDR_LEN(corerev)	(WLC_RXHDR_LEN + D11_RXHDR_LEN(corerev))
/* RxOffset used by fifo-0 in splitrxmode-4.
 * we need minimum 8 bytes of rxoffset for DMA engines to work, even though
 * 4 bytes was enough for header coversion.
 */
#define WL_DATA_FIFO_OFFSET	8	/* used by fifo-0 in splitrxmode-4 */

extern uintptr sts_mp_base_addr[];
typedef struct sts_buff sts_buff_t;

/* for rev 129, each phyrxsts is DMA'ed into an 8-byte aligned buffer */
#define D11_GE129_STS_MP_ALIGN_BYTES 8
/* the size of each array element also needs to be 8-byte aligned buffer */
#define PHYRXSTS_ELM_SZ_GE129 ALIGN_SIZE(sizeof(d11phyrxsts_t), D11_GE129_STS_MP_ALIGN_BYTES)

/** this d11rev >=129 struct contains meta-data related to a particular phyrx status */
BWL_PRE_PACKED_STRUCT struct sts_buff {
	d11phyrxsts_t	*phystshdr; /** sizeof(phystshdr) = 160B */
	union {
#ifdef DONGLEBUILD
		/* 8-byte padding to align-size to 8x */
		uint8	pad0[8];
		struct {
			union {
				uint16	dma_idx;
				int16	bmebuf_idx;
			};
			uint16	ref_cnt;
			void	*link;
		};
#else
		uint8	pad_0[32];
		struct {
			uint8	pad_1[20];
			union {
				uint16	dma_idx;
				int16	bmebuf_idx;
			};
			uint16	ref_cnt;
			void	*link;
		};
#endif /* DONGLEBUILD */
	};
} BWL_POST_PACKED_STRUCT;

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

/** a pool that, in contrast with bcm_mp_pool_h, does not modify object on STSBUF_FREE() */
struct sts_buff_pool {
	struct sts_buff *top; /**< top of a stack consisting of free status buffers */
	int n_items;          /**< number of sts_buff in pool */
	int n_fail;           /**< number of times a STS_ALLOC() returned NULL */
	int high_watermark;   /**< max number of sts_buff items in pool at any time */
	void *ctx;         /**< control/config block */
		              /**< circular buffer release callback function */
	void (*free_sts_fn)(void *ctx, int16 bufidx);
};

#define STSBUF_RECV_OFFSET	        8
#define	STSBUF_DATA(sts)		(((sts))->phystshdr)
#define	STSBUF_LEN(sts)			(((sts))->phystshdr->len)
#define	STSBUF_SEQ(sts)		    (((((sts))->phystshdr->seq & 0xf000) >> 4) |\
								(((sts))->phystshdr->seq & 0xff))
#define	STSBUF_LINK(sts)		(((sts))->link)
#define	STSBUF_SETLINK(sts, x)		(((sts))->link = (x))
#define	STSBUF_DMAIDX(sts)		(((sts))->dma_idx)
#define	STSBUF_SETDMAIDX(sts, x)	(((sts))->dma_idx = (x))
#define	STSBUF_BMEBUFIDX(sts)		(((sts))->bmebuf_idx)

#define STSBUF_ALLOC(sts_mempool)		stsbuf_alloc((sts_mempool))
#define STSBUF_FREE(sts_buff, sts_mempool)	stsbuf_free((sts_buff), (sts_mempool))
#define STSBUF_BME_INVALID_IDX			(-1)	/* valid index start from 0 */

/** @ret  a statusbuffer taken from a stack of free status buffers */
static INLINE sts_buff_t *
stsbuf_alloc(struct sts_buff_pool *sts_pool)
{
	struct sts_buff *sts_buff = sts_pool->top;

	if (sts_pool->top != NULL) {
		sts_pool->top = STSBUF_LINK(sts_pool->top);
		STSBUF_SETLINK(sts_buff, NULL);
		sts_pool->n_items--;
	} else {
		sts_pool->n_fail++;
	}

	return sts_buff;
}

/** @param[in] sts_buf   A linked list of status buffers to free, they are added to a stack */
static INLINE void
stsbuf_free(sts_buff_t *sts_buff, struct sts_buff_pool *sts_pool)
{
	struct sts_buff *sts_last, *sts_curr ; /* last sts in caller provided list */

	ASSERT(sts_buff != NULL);
	sts_curr = sts_last = sts_buff;

	while (sts_curr) {
		/* Need to call free function if this it is set (used by WLC_OFFLOADS_RXSTS) */
		if (sts_pool->free_sts_fn != NULL &&
		    STSBUF_BMEBUFIDX(sts_curr) != STSBUF_BME_INVALID_IDX) {
			sts_pool->free_sts_fn(sts_pool->ctx, STSBUF_BMEBUFIDX(sts_curr));
			STSBUF_BMEBUFIDX(sts_curr) = STSBUF_BME_INVALID_IDX;
		}
		sts_pool->n_items++;

		sts_last = sts_curr;
		sts_curr = STSBUF_LINK(sts_curr);
	}

	sts_pool->high_watermark = MAX(sts_pool->high_watermark, sts_pool->n_items);

	STSBUF_SETLINK(sts_last, sts_pool->top);
	sts_pool->top = sts_buff;
}

/* ulp dbg macro */
#define HNDD11_DBG(x)
#define HNDD11_ERR(x) printf x

extern void hndd11_read_shm(si_t *sih, uint coreunit, uint offset, void* buf);
extern void hndd11_write_shm(si_t *sih, uint coreunit, uint offset, const void* buf);

extern void hndd11_copyto_shm(si_t *sih, uint coreunit, uint offset,
		const void* buf, int len);

extern void hndd11_copyfrom_shm(si_t *sih, uint coreunit, uint offset, void* buf, int len);
extern void hndd11_copyto_shm(si_t *sih, uint coreunit, uint offset, const void* buf, int len);

extern uint32 hndd11_write_reg32(osl_t *osh, volatile uint32 *reg, uint32 mask, uint32 val);

extern uint32 hndd11_bm_read(osl_t *osh, d11regs_info_t *regsinfo, uint32 offset, uint32 len,
	uint32 *buf);
extern uint32 hndd11_bm_write(osl_t *osh, d11regs_info_t *regsinfo, uint32 offset, uint32 len,
	const uint32 *buf);
extern void hndd11_bm_dump(osl_t *osh, d11regs_info_t *regsinfo, uint32 offset, uint32 len);

extern int hndd11_get_reginfo(si_t *sih, d11regs_info_t *regsinfo, uint coreunit);

#if defined(STS_XFER)
/*
 * ------------------------------------------------------------------------------------------------
 *  Section: D11 PhyRx Status
 *
 *  For MAC rev >= 129, PhyRxStatus is send to a seperate PhyRx Status buffer asynchronous to
 *  ucode statuss
 *  WLAN driver will link PhyRx Status buffer to Ucode status using wlc_d11rxhdr_t::d11phyrxsts
 * ------------------------------------------------------------------------------------------------
 */

/* Accessor MACROS for PhyRx Status (d11phyrxsts_t) bufferss */
#define HNDD11_PHYRXSTS_GE129_ACCESS_REF(rxh, member)						\
({												\
	wlc_d11rxhdr_t *_wrxh = CONTAINEROF(rxh, wlc_d11rxhdr_t, rxhdr);			\
	(&(_wrxh->d11phyrxsts->member));							\
})

#define HNDD11_PHYRXSTS_GE129_ACCESS_VAL(rxh, member)						\
({												\
	wlc_d11rxhdr_t *_wrxh = CONTAINEROF(rxh, wlc_d11rxhdr_t, rxhdr);			\
	(_wrxh->d11phyrxsts->member);								\
})

#endif /* STS_XFER */

#endif	/* _hndd11_h_ */
/*
 * Generic functions for d11 access
 *
 * Copyright (C) 2021, Broadcom. All Rights Reserved.
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
 * $Id: hndd11.h 787082 2020-05-18 09:21:51Z $
 */
