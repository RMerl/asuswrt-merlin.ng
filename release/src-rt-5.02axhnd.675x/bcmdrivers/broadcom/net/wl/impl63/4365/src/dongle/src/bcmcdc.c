/*
 * CDC protocol support
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: bcmcdc.c 500185 2014-09-03 05:05:27Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <proto/ethernet.h>
#include <trxhdr.h>
#include <bcmnvram.h>

#include <osl.h>
#include <wlioctl.h>
#include <etioctl.h>
#include <bcmendian.h>
#include <epivers.h>
#include <sflash.h>
#include <sbchipc.h>
#include <dngl_dbg.h>
#include <bcmcdc.h>
#include <oidencap.h>
#include <dngl_bus.h>
#include <dngl_api.h>
#include <dngl_protocol.h>
#ifdef EXT_STA
#include <wlc_pub.h>
#endif /* EXT_STA */

static bool autoshrink = TRUE;

#define CDC_IOC_RESP_CHUNK	(MAXPKTBUFSZ - LBUFSZ) /* cdc ioctl response fragment size */
#define REBOOT_DELAY	1000		/* delay to allow WLC_REBOOT IOCTL to complete */

#define RESP_CHUNKSZ	0
#define REBOOT_DLY		2
#define DONGLEOVERHEAD	3
#define DONGLEHDRSZ		4
#define BDCHEADERLEN	5
#define MAX_TUNABLE		6

void *cdc_proto_pkt_header_push(void *proto, void *p);
int cdc_proto_pkt_header_pull(void *proto, void *p);
void cdc_proto_dev_event(void *proto, void *data);
void *cdc_proto_attach(osl_t *osh, struct dngl *dngl, struct dngl_bus *bus,
                       char *name, bool link_init);
void cdc_proto_detach(void *proto);
void cdc_proto_ctrldispatch(void *proto, void *p, uchar *ext_buf);

typedef struct {
	void *dngl;			/* per-port private data struct */
	void *bus;			/* bus private context struct */
	osl_t *osh;
	char *name;			/* name of network interface */
	dngl_stats_t stats;	/* Linux network device statistics */
	uint32 tunables[MAX_TUNABLE];
#ifdef BCMUSBDEV
	bool pr46794WAR;
#endif // endif
} cdc_info_t;

struct dngl_proto_ops_t cdc_proto_ops = {
	proto_attach_fn:	cdc_proto_attach,
	proto_detach_fn:	cdc_proto_detach,
	proto_ctrldispatch_fn:	cdc_proto_ctrldispatch,
	proto_pkt_header_push_fn: cdc_proto_pkt_header_push,
	proto_pkt_header_pull_fn: cdc_proto_pkt_header_pull,
	proto_dev_event_fn: cdc_proto_dev_event
};

static void cdc_indicate(cdc_info_t *cdc, uint32 cmd, uint32 flags,
                         uint32 status, uchar *buf, ulong len, void *p);

static void
cdc_indicate(cdc_info_t *cdc, uint32 cmd, uint32 flags, uint32 status,
             uchar *buf, ulong len, void *p)
{
	uint32 chunk, id;
	void *p0, *p1, *p2;
	cdc_ioctl_t *cdc_ioc;
	uint32 needed;

	trace("%s", cdc->name);

	needed = len;
	/*
	 * In the case of an error, no data needs to be sent up
	 */
	if (status != 0)
		len = 0;

	/* If given the results in a packet, just use that */
	if (p != NULL) {
		ASSERT(PKTHEADROOM(cdc->osh, p) >=
		       (sizeof(cdc_ioctl_t) + cdc->tunables[DONGLEHDRSZ] + sizeof(uint32)));
		if (PKTHEADROOM(cdc->osh, p) < (sizeof(cdc_ioctl_t) +
		                                cdc->tunables[DONGLEHDRSZ] + sizeof(uint32))) {
			err("bad return buffer!");
			PKTFREE(cdc->osh, p, TRUE);
			return;
		}
		p0 = p;
		p = NULL;
		PKTPUSH(cdc->osh, p0, sizeof(cdc_ioctl_t));
		cdc_ioc = (cdc_ioctl_t *)PKTDATA(cdc->osh, p0);
	} else {
		/*
		 * If the response buffer is greater than cdc->tunables[RESP_CHUNKSZ]
		 * then break the reponse into multiple pkts which will
		 * be dma chained into one transfer
		 */

		/* first chunk has cdc_ioctl_t, and space for dongle msg header */
		if ((len + sizeof(cdc_ioctl_t) + cdc->tunables[DONGLEOVERHEAD]) >=
		    cdc->tunables[RESP_CHUNKSZ])
			chunk = cdc->tunables[RESP_CHUNKSZ] - sizeof(cdc_ioctl_t) -
			        cdc->tunables[DONGLEOVERHEAD];
		else
			chunk = len;

		if (!(p0 = p1 = PKTGET(cdc->osh, chunk+sizeof(cdc_ioctl_t) +
		                       cdc->tunables[DONGLEOVERHEAD], TRUE))) {
			ASSERT(0);
			err("out of txbufs");
			return;
		}

		/* reserve space for dongle msg header */
		PKTSETLEN(cdc->osh, p0, chunk+sizeof(cdc_ioctl_t) +
		          cdc->tunables[DONGLEHDRSZ]);
		PKTPULL(cdc->osh, p0, cdc->tunables[DONGLEHDRSZ]);

		cdc_ioc = (cdc_ioctl_t *)(PKTDATA(cdc->osh, p0));

		if (buf && chunk) {
			/* copy in the data fragment */
			bcopy(buf, (char*)&cdc_ioc[1], chunk);

			buf += chunk;
			len -= chunk;
		}

		/* add more pkts if necessary */
		while (buf && (len > 0)) {
			if (len > cdc->tunables[RESP_CHUNKSZ])
				chunk = cdc->tunables[RESP_CHUNKSZ];
			else
				chunk = len;

			if (!(p2 = PKTGET(cdc->osh, chunk, TRUE))) {
				err("out of txbufs");
				PKTFREE(cdc->osh, p0, TRUE);
				return;
			}

			/* copy in the data fragments */
			bcopy(buf, (char*)PKTDATA(cdc->osh, p2), chunk);

			buf += chunk;
			len -= chunk;

			/* chain them */
			PKTSETNEXT(cdc->osh, p1, p2);
			p1 = p2;
		}
	}

	/* form the response hdr */
	id = (flags & CDCF_IOC_ID_MASK) >> CDCF_IOC_ID_SHIFT;
	cdc_ioc->cmd = htol32(cmd);
	cdc_ioc->len = htol32(needed);
	flags = id << CDCF_IOC_ID_SHIFT;
	if (status) {
		flags |= CDCF_IOC_ERROR;
		cdc_ioc->status = htol32(status);
	} else
		cdc_ioc->status = 0;

	cdc_ioc->flags = htol32(flags);

	/* Send the pkt chain */
	bus_ops->sendctl(cdc->bus, p0);
}

/* XXX This function could and should be BCMATTACH but there seems to be
 * an issue in the special ram stub scripts/lds process.  Because this
 * function is called from a non-discarded function, dngl_binddev of
 * dngl_rte.c, there is a build error.  Analysis of the code shows that
 * proto_attach can only be called on the first call to dngl_bindev which
 * occurs during attach.  But the ram script/lds process seems to be
 * based on a more static analysis.
 */
void *
BCMINITFN(cdc_proto_attach)(osl_t *osh, struct dngl *dngl, struct dngl_bus *bus,
                          char *name, bool link_init)
{
	cdc_info_t *cdc;

	trace("%s", name);

	if ((cdc = MALLOC(osh, sizeof(cdc_info_t))) == NULL)
		return NULL;

	memset(cdc, 0, sizeof(cdc_info_t));
	cdc->name = name;
	cdc->dngl = dngl;
	cdc->bus = bus;
	cdc->osh = osh;
	cdc->tunables[RESP_CHUNKSZ] = CDC_IOC_RESP_CHUNK;
	cdc->tunables[REBOOT_DLY] = REBOOT_DELAY;
	cdc->tunables[DONGLEOVERHEAD] = BCMDONGLEOVERHEAD;
	cdc->tunables[DONGLEHDRSZ] = BCMDONGLEHDRSZ;
	cdc->tunables[BDCHEADERLEN] = BDC_HEADER_LEN;

	return (void *) cdc;
}

void
BCMATTACHFN(cdc_proto_detach)(void *proto)
{
	cdc_info_t *cdc = (cdc_info_t *) proto;

	trace("%s", cdc->name);
	MFREE(cdc->osh, cdc, sizeof(cdc_info_t));
}

/* dispatcher for all control messages received from the host */
void
cdc_proto_ctrldispatch(void *proto, void *p, uchar *ext_buf)
{
	cdc_info_t *cdc = (cdc_info_t *) proto;
	cdc_ioctl_t *cdc_ioc = (cdc_ioctl_t *)PKTDATA(cdc->osh, p);
	uint32 flags = ltoh32(cdc_ioc->flags);
	uint32 cmd =  ltoh32(cdc_ioc->cmd);
	uint32 savecmd;
	uchar *buf = NULL, * savebuf = NULL;
	uint32 inlen = 0, outlen = 0, savelen = 0;
	uint32 reserve = 0;
	int ret = 0;
	int pktlen = PKTLEN(cdc->osh, p);
	setinformation_t *sinfo = NULL;
	uint32 needed = 0, used = 0;
	int ifindex = CDC_IOC_IF_IDX(flags);
	bool reuse_pkt = FALSE;
	bool shrunk = FALSE;
	void *newpkt = NULL;
	void *currpkt = NULL;
	uint32 newlen = 0;
	uint32 padsz;

	trace("%s", cdc->name);

	padsz = cdc->tunables[DONGLEOVERHEAD] - cdc->tunables[DONGLEHDRSZ];

	if (pktlen < sizeof(cdc_ioctl_t)) {
		err("bad packet length %d", pktlen);
#ifdef BCMDBG
		prpkt("bad pkt len", cdc->osh, p);
#endif // endif
		ret = BCME_BADLEN;
		goto indicate;
	}

	outlen = ltoh32(cdc_ioc->len);
	if (outlen & CDCL_IOC_INLEN_MASK) {
		inlen = (outlen & CDCL_IOC_INLEN_MASK) >> CDCL_IOC_INLEN_SHIFT;
		outlen &= CDCL_IOC_OUTLEN_MASK;
		if (pktlen < inlen) {
			err("bad message length %d; pktlen %d", inlen, pktlen);
#ifdef BCMDBG
			prpkt("bad message len", cdc->osh, p);
#endif // endif
#ifdef BCMUSBDEV
			if (cdc->pr46794WAR && pktlen == 12) {
				pktlen = inlen;
				PKTSETLEN(cdc->osh, p, inlen + sizeof(cdc_ioctl_t));
				bus_ops->pr46794WAR(cdc->bus);
				cdc->pr46794WAR = FALSE;
				err("proto_pr46794WAR OFF");
			} else
#endif // endif
			{
				ret = BCME_BUFTOOSHORT;
				outlen = 0;
				goto indicate;
			}
		}
	} else
		inlen = outlen;

	/* Check if max outlen is exceeded */
	if (outlen > WLC_IOCTL_MAXLEN) {
		ret = BCME_BADLEN;
		outlen = 0; /* No data to return */
		goto indicate;
	}

	/* Valid input data should not exceed available packet data */
	if (inlen > (pktlen - sizeof(cdc_ioctl_t)))
		inlen = (pktlen - sizeof(cdc_ioctl_t));

	/* Reuse request packet as much as possible: incorporate tailroom. */
	PKTSETLEN(cdc->osh, p, (PKTLEN(cdc->osh, p) + PKTTAILROOM(cdc->osh, p)));

	/* Add available headroom, w/alignment pad.  (May be overkill!) */
	reserve = cdc->tunables[DONGLEHDRSZ] + sizeof(uint32);
	if (PKTHEADROOM(cdc->osh, p) > reserve) {
		PKTPUSH(cdc->osh, p, (PKTHEADROOM(cdc->osh, p) - reserve));
	}

	/* Adjust to account only for available data space */
	PKTPULL(cdc->osh, p, sizeof(cdc_ioctl_t));
	pktlen = PKTLEN(cdc->osh, p);

	/* We expect (and don't handle!) input chains, but may want to output */
	ASSERT(PKTNEXT(cdc->osh, p) == NULL);
	if (PKTNEXT(cdc->osh, p) != NULL) {
		PKTFREE(cdc->osh, PKTNEXT(cdc->osh, p), FALSE);
		PKTSETNEXT(cdc->osh, p, NULL);
	}

	/* From here on, assume p will be reused as part of the response */
	reuse_pkt = TRUE;

	/* If results fit, reuse this packet as buffer; otherwise alloc a new one */
	if (((outlen + padsz) <= pktlen) &&
	    (PKTHEADROOM(cdc->osh, p) >= (reserve + sizeof(cdc_ioctl_t)))) {
		buf = PKTDATA(cdc->osh, p);
		PKTSETLEN(cdc->osh, p, outlen);
	} else {
		newlen = outlen + sizeof(cdc_ioctl_t) +
		        cdc->tunables[DONGLEOVERHEAD] + sizeof(uint32);
		newpkt = PKTGET(cdc->osh, newlen, TRUE);
		if (newpkt) {
			PKTPULL(cdc->osh, newpkt, (cdc->tunables[DONGLEHDRSZ] +
			                           sizeof(uint32) + sizeof(cdc_ioctl_t)));
			PKTSETLEN(cdc->osh, newpkt, outlen);
			buf = PKTDATA(cdc->osh, newpkt);
			reuse_pkt = FALSE;
		}
	}

	/* If buf is NULL, packet (old or new) can't be used directly.
	 * Malloc a buffer for the ioctl, and prepare a return packet chain.
	 * If no memory for malloc or packet buffers, autoshrink or fail.
	 */
	while (buf == NULL) {
		savelen = MAX(inlen, outlen);
		if (savelen) {
			savebuf = MALLOC(cdc->osh, savelen);
			if (savebuf != NULL) {
				int segsize, remlen = (int)savelen - pktlen;
				ASSERT(newpkt == NULL);
				for (newpkt = p; remlen > 0; remlen -= segsize) {
					segsize = MIN(cdc->tunables[RESP_CHUNKSZ], remlen);
					currpkt = PKTGET(cdc->osh, segsize, TRUE);
					PKTSETNEXT(cdc->osh, newpkt, currpkt);
					newpkt = PKTNEXT(cdc->osh, newpkt);
					if (newpkt == NULL)
						break;
				}

				/* On packet failure, release whatever we did get */
				if (remlen > 0) {
					newpkt = PKTNEXT(cdc->osh, p);
					PKTSETNEXT(cdc->osh, p, NULL);
					PKTFREE(cdc->osh, newpkt, TRUE);
					newpkt = NULL;

					MFREE(cdc->osh, savebuf, savelen);
					savebuf = NULL;
				}
				newpkt = NULL;
			}

			/* If no buffer (something failed) try autoshrink or fail */
			if (savebuf == NULL) {
				if (autoshrink) {
					if (((outlen + padsz) > inlen) &&
					    ((outlen > (pktlen - padsz) + 1024))) {
						outlen -= 1024;
					} else {
						outlen = pktlen - padsz;
						buf = PKTDATA(cdc->osh, p);
					}
					shrunk = TRUE;
				} else {
					ret = BCME_NOMEM;
					outlen = 0;
					goto indicate;
				}
			} else {
				buf = savebuf;
			}
		} else {
			ASSERT(0);
			ret = BCME_ERROR;
			outlen = 0;
			goto indicate;
		}
	}

	/* At this point buf is the io buf, pointing to the original packet p on reuse
	 * (including autoshrink), a newly allocated packet newpkt, or a malloced buffer
	 * savebuf.  If the latter, p has been set up with a packet chain the result can
	 * be copied to.
	 */

	/* Copy valid input data into the buffer */
	if (inlen) {
		bcopy((char *)&cdc_ioc[1], buf, inlen);
	}

	/* On big Windows (NOT CE) 'SetInformationHandler' calls comes through
	   'QueryInformationHandler' since we use 'IOCTL_NDIS_QUERY_GLOBAL_STATS' (CE uses NDISUIO)
	   from the wl.exe. 'SetInformationHandler' is encapsulated with
	   BCM_SETINFORMATION package and sent across using 'IOCTL_NDIS_QUERY_GLOBAL_STATS'
	   which gets intercepted in dongle code and piped to 'SetInformationHandler' call
	*/

	if (cmd == (OID_BCM_SETINFORMATION))
	{
		/* Check packet */
		if (inlen < SETINFORMATION_SIZE) {
			ret = BCME_BADLEN;
			goto indicate;
		}

		sinfo = (setinformation_t *)buf;
		/* Change OID */
		cmd = sinfo->oid;
		/* Adjust buffer */
		buf += SETINFORMATION_SIZE;
		inlen -= SETINFORMATION_SIZE;
		if (savebuf == NULL)
			PKTPULL(cdc->osh, (newpkt ? newpkt : p), SETINFORMATION_SIZE);

		flags |= CDCF_IOC_SET;
	}
	/*
	 * Intercepting selected wlc IOCTL's requires
	 * unwrapping of the cmd in case of NDIS OIDS's
	 */
	savecmd = cmd;

	if ((cmd >= WL_OID_BASE) && (cmd < (WL_OID_BASE + WLC_LAST)))
		cmd -= WL_OID_BASE;

	switch (cmd) {

#ifdef DONGLEBUILD
	case WLC_REBOOT:
		ret = dngl_schedule_work(cdc->dngl, NULL, _dngl_reboot,
		                         cdc->tunables[REBOOT_DLY]);
		goto indicate;
#endif // endif

	case WLC_KEEPALIVE: {
		uint32 ul;
		ul = ltoh_ua((uint32 *) buf);
		dngl_keepalive(cdc->dngl, ul);
		goto indicate;
	}

#ifdef FLASH_UPGRADE
	case WLC_UPGRADE:
		ret = dngl_upgrade(cdc->dngl, buf, inlen);
		goto indicate;
#endif // endif

	case WLC_SET_VAR:
		if (!strncmp((char *)buf, "bus:", 4)) {
			ret = bus_ops->iovar(cdc->bus, (char *)buf, inlen, NULL, TRUE);
			goto indicate;
		}
		break;

	case WLC_GET_VAR:
		if (!strncmp((char *)buf, "bus:", 4)) {
			ret = bus_ops->iovar(cdc->bus, (char *)buf, outlen, &outlen, FALSE);
			goto indicate;
		}
		break;

#ifdef BCMDBG
	case WLC_SET_MSGLEVEL:
		dngl_msglevel = ltoh_ua((uint32 *) buf);
		break;
#endif // endif

#ifdef BCMET
	case ETCUP:
#else
	case WLC_UP:
#endif /* BCMET */
		/* make sure the attached device has been opened */
		ret = dngl_opendev(cdc->dngl);
		goto indicate;
	}
	cmd = savecmd;
	if (flags & CDCF_IOC_SET)
		ret = dngl_dev_set_oid(cdc->dngl, ifindex, cmd, buf, inlen, (int *)&used,
		                       (int *)&needed);
	else
		ret = dngl_dev_query_oid(cdc->dngl, ifindex, cmd, buf, outlen, (int *)&outlen,
		                         (int *)&needed);

	/* If we truncated outlen, map too-short errors to NOMEM */
	if ((ret == BCME_BUFTOOSHORT) && shrunk) {
		outlen = 0;
		ret = BCME_NOMEM;
	}

indicate:
	/* Copy result from buffer, and/or update packet length */
	if (savebuf)
		pktfrombuf(cdc->osh, p, 0, outlen, buf);
	else
		PKTSETLEN(cdc->osh, (newpkt ? newpkt : p), outlen);

	if (ret)
		outlen = needed;

	if (!reuse_pkt) {
		PKTFREE(cdc->osh, p, FALSE);
		p = newpkt;
	}
	cdc_indicate(cdc, cmd, flags, ret, buf, outlen, p);

	if (savebuf)
		MFREE(cdc->osh, savebuf, savelen);

}

void *
cdc_proto_pkt_header_push(void *proto, void *p)
{
#if defined(WLC_LOW) && defined(WLC_HIGH)
	cdc_info_t *cdc = proto;
	struct bdc_header *h;
	osl_t *osh = cdc->osh;

	if (PKTHEADROOM(osh, p) < cdc->tunables[BDCHEADERLEN]) {
		void *p1;
		int plen = PKTLEN(osh, p);

		/* Alloc a packet that will fit all the data; chaining the header won't work */
		if ((p1 = PKTGET(osh, plen + cdc->tunables[BDCHEADERLEN], TRUE)) == NULL) {
			err("PKTGET pkt size %d + headroom %d failed\n", plen,
			    cdc->tunables[BDCHEADERLEN]);
			PKTFREE(osh, p, TRUE);
			return NULL;
		}

		/* Transfer other fields */
		PKTSETPRIO(p1, PKTPRIO(p));
		PKTSETSUMGOOD(p1, PKTSUMGOOD(p));
		PKTSETDATAOFFSET(p1, PKTDATAOFFSET(p));
		bcopy(PKTDATA(osh, p), PKTDATA(osh, p1) + cdc->tunables[BDCHEADERLEN], plen);
		PKTFREE(osh, p, TRUE);

		p = p1;
	} else
		PKTPUSH(osh, p, cdc->tunables[BDCHEADERLEN]);

	h = (struct bdc_header *)PKTDATA(osh, p);

	h->flags = (BDC_PROTO_VER << BDC_FLAG_VER_SHIFT);
	if (PKTSUMGOOD(p))
		h->flags |= BDC_FLAG_SUM_GOOD;
	h->priority = PKTPRIO(p);
	h->flags2 = 0;
	h->dataOffset = PKTDATAOFFSET(p);
	return p;
#else
	return p;
#endif /* WLC_LOW && WLC_HIGH */
}

int
cdc_proto_pkt_header_pull(void *proto, void *p)
{
#if defined(WLC_LOW) && defined(WLC_HIGH)
	cdc_info_t *cdc = proto;
	struct bdc_header *h;
	osl_t *osh = cdc->osh;
	int ver;

	if (PKTLEN(osh, p) < cdc->tunables[BDCHEADERLEN]) {
		err("bad packet length %d", PKTLEN(osh, p));
		goto drop;
	}

	h = (struct bdc_header *)PKTDATA(osh, p);

	ver = (h->flags & BDC_FLAG_VER_MASK) >> BDC_FLAG_VER_SHIFT;

	if (!((ver == BDC_PROTO_VER) || (ver == BDC_PROTO_VER_1))) {
		err("unsupported bdc protocol version (%d)\n", ver);
		goto drop;
	}

	PKTSETPRIO(p, h->priority & BDC_PRIORITY_MASK);
	if (h->flags & BDC_FLAG_SUM_NEEDED)
		PKTSETSUMNEEDED(p, TRUE);

#ifdef EXT_STA
	if (h->flags & BDC_FLAG_EXEMPT) {
		WLPKTFLAG_EXEMPT_SET(WLPKTTAG(p), (h->flags & BDC_FLAG_EXEMPT));
	}
#endif /* EXT_STA */

	if (ver == BDC_PROTO_VER_1)
		PKTSETDATAOFFSET(p, 0);
	else
		PKTSETDATAOFFSET(p, h->dataOffset);

	PKTPULL(osh, p, cdc->tunables[BDCHEADERLEN]);

	return 0;

drop:
	PKTFREE(osh, p, FALSE);
	return 1;
#else
	return 0;
#endif /* WLC_LOW && WLC_HIGH */
}

void
cdc_proto_dev_event(void *proto, void *data)
{
#ifdef NOTYET
	cdc_info_t *cdc = (cdc_info_t *) proto;
	bcm_event_t *pkt_data = (bcm_event_t *) data;
	uchar *buf = NULL;
	ulong Status = 0;
	uint len = 0;
	uint msg, datalen;
	uint32 flags;
	struct ether_addr *addr;

	trace("%s", cdc->name);

	if (bcmp(BRCM_OUI, &pkt_data->bcm_hdr.oui[0], DOT11_OUI_LEN)) {
		printf("%s: proto_dev_event: Not BRCM_OUI\n", cdc->name);
		return;
	}

	if (ntoh16(pkt_data->bcm_hdr.usr_subtype) != BCMILCP_BCM_SUBTYPE_EVENT) {
		printf("%s: proto_dev_event: Not BCMILCP_BCM_SUBTYPE_MSG\n", cdc->name);
		return;
	}

	msg = ntoh32(pkt_data->event.event_type);
	addr = (struct ether_addr *)&(pkt_data->event.addr);
	datalen = ntoh32(pkt_data->event.datalen);
	flags = (uint32) ntoh16(pkt_data->event.flags);

	/* Generic MAC event */
	len = sizeof(bcm_event_t) + datalen;
	buf = (uchar *) pkt_data;
	Status = BCM_MAC_STATUS_INDICATION;

	cdc_indicate(cdc, cmd, flags, BCM_MAC_STATUS_INDICATION, buf, len, NULL);
#endif /* NOTYET */
}

#ifdef BCMUSBDEV
void
proto_pr46794WAR(struct dngl *dngl)
{
	cdc_info_t *cdc = (cdc_info_t *) dngl_proto(dngl);
	cdc->pr46794WAR = TRUE;
	err("ON");
	/* punt for now */
}
#endif /* BCMUSBDEV */
