/*
 * Initialization and support routines for self-booting compressed image.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: rtecdc.c 405571 2013-06-03 20:03:49Z $
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
#include <bcmmsgbuf.h>
#include <oidencap.h>
#include <dngl_bus.h>
#include <dngl_api.h>
#include <dngl_protocol.h>

static void *msgbuf_proto_attach(osl_t *osh, struct dngl *dngl, struct dngl_bus *bus,
                          char *name, bool link_init);
static void msgbuf_proto_detach(void *proto);
static void msgbuf_proto_ctrldispatch(void *proto, void *p, uchar *ioct_resp_buf);
static void *msgbuf_proto_pkt_header_push(void *proto, void *p);
static int msgbuf_proto_pkt_header_pull(void *proto, void *p);
static void msgbuf_proto_dev_event(void *proto, void *data);

typedef struct {
	struct dngl	*dngl;			/* per-port private data struct */
	struct dngl_bus	*bus;			/* bus private context struct */
	osl_t	*osh;
	char	*name;			/* name of network interface */
	dngl_stats_t	stats;	/* Linux network device statistics */
	void	* pkt_cache;
} dngl_proto_info_t;

struct dngl_proto_ops_t msgbuf_proto_ops = {
	proto_attach_fn:	msgbuf_proto_attach,
	proto_detach_fn:	msgbuf_proto_detach,
	proto_ctrldispatch_fn:	msgbuf_proto_ctrldispatch,
	proto_pkt_header_push_fn: msgbuf_proto_pkt_header_push,
	proto_pkt_header_pull_fn: msgbuf_proto_pkt_header_pull,
	proto_dev_event_fn: msgbuf_proto_dev_event
};

static void msgbuf_indicate(dngl_proto_info_t *dngl_proto, uint32 cmd, uint16 xt_id,
	uint32 flags, uint32 status,  uchar *buf, ulong len, void *p, ret_buf_t * ret_buf,
                            uint16 retlen, bool use_altinterface);

void *msgbuf_proto_attach(osl_t *osh, struct dngl *dngl, struct dngl_bus *bus,
                           char *name, bool link_init)
{
	dngl_proto_info_t * dngl_proto;

	if ((dngl_proto = MALLOC(osh, sizeof(dngl_proto_info_t))) == NULL)
		return NULL;

	memset(dngl_proto, 0, sizeof(dngl_proto_info_t));
	dngl_proto->name = name;
	dngl_proto->dngl = dngl;
	dngl_proto->bus = bus;
	dngl_proto->osh = osh;

	STATIC_ASSERT(sizeof(union ctrl_submit_item) == H2DRING_CTRL_SUB_ITEMSIZE);
	STATIC_ASSERT(sizeof(union ctrl_completion_item) == D2HRING_CTRL_CMPLT_ITEMSIZE);
	STATIC_ASSERT(sizeof(union rxbuf_submit_item) == H2DRING_RXPOST_ITEMSIZE);
	STATIC_ASSERT(sizeof(union rxbuf_complete_item) == D2HRING_RXCMPLT_ITEMSIZE);
	STATIC_ASSERT(sizeof(union txbuf_submit_item) == H2DRING_TXPOST_ITEMSIZE);
	STATIC_ASSERT(sizeof(union txbuf_complete_item) == D2HRING_TXCMPLT_ITEMSIZE);

	return (void *) dngl_proto;
}

void BCMATTACHFN(msgbuf_proto_detach)(void *proto)
{
	dngl_proto_info_t *dngl_proto = (dngl_proto_info_t *) proto;

	trace("%s", dngl_proto->name);
	MFREE(dngl_proto->osh, dngl_proto, sizeof(dngl_proto_info_t));

}
void msgbuf_proto_ctrldispatch(void *proto, void *p, uchar *ioct_resp_buf)
{
	dngl_proto_info_t *dngl_proto = (dngl_proto_info_t *)proto;
	cmn_msg_hdr_t *msg = NULL;

	/* msg buf hdr info */
	uint32 cmd = 0;
	uint16 xt_id = 0;
	uint32 flags = 0;
	uint16 retbuf_len = 0;
	uint16 msgbuf_hdr_sz = 0;

	int pktlen = PKTLEN(dngl_proto->osh, p);
	int ret = 0;
	uint32 savecmd;
	uchar *buf = NULL;
	uint32 inlen = 0, outlen = 0;
	setinformation_t *sinfo = NULL;
	int ifindex = 0;
	uint32 needed = 0, used = 0;
	void* payloadptr = NULL;
	bool use_altinterface = PKTALTINTF(p);

	trace("%s", dngl_proto->name);

	msg = (cmn_msg_hdr_t *)PKTDATA(dngl_proto->osh, p);

	if (msg->msg_type == MSG_TYPE_IOCTLPTR_REQ) {
		/* Non Inline ioctl request */
		ioctl_req_msg_t * ioct_rqst;

		/* ioctl payload DMA ed separately */
		ioct_rqst = (ioctl_req_msg_t*)PKTDATA(dngl_proto->osh, p);
		msgbuf_hdr_sz = sizeof(ioctl_req_msg_t);

		/* Extract out ioct header */
		cmd = ltoh32(ioct_rqst->cmd);
		xt_id = ltoh16(ioct_rqst->trans_id);
		retbuf_len = ltoh16(ioct_rqst->output_buf_len);
	}
	else {
		err("bad packet type %d", msg->msg_type);
		ret = BCME_BADARG;
		ASSERT(0);
		goto indicate;
	}

	/* interface idx */
	ifindex = BCMMSGBUF_API_IFIDX(msg);

	if (pktlen < msgbuf_hdr_sz) {
		err("bad packet length %d", pktlen);
#ifdef BCMDBG
		prpkt("bad pkt len", dngl_proto->osh, p);
#endif
		ret = BCME_BADLEN;
		ASSERT(0);
		goto indicate;
	}

	/* remove ret buf info */
	PKTPULL(dngl_proto->osh, p, msgbuf_hdr_sz);
	inlen = pktlen - msgbuf_hdr_sz;

	payloadptr = PKTDATA(dngl_proto->osh, p);

	/* Check if max outlen is exceeded */
	outlen = retbuf_len;
	if (outlen > WLC_IOCTL_MAXLEN) {
		ret = BCME_BADLEN;
		outlen = 0; /* No data to return */
		retbuf_len = 0; /* No data to return */
		goto indicate;
	}

	/* Reuse request packet as much as possible: incorporate tailroom. */
	PKTSETLEN(dngl_proto->osh, p, (PKTLEN(dngl_proto->osh, p) +
		PKTTAILROOM(dngl_proto->osh, p)));

	if (ioct_resp_buf) {
		/* Separate buffer passed for ioctl response. We do not use the packet data ptr. */
		buf = ioct_resp_buf;
		/* Copy valid input data into the buffer */
		if (inlen) {
			bcopy(payloadptr, buf, inlen);
		}
	} else {
		/* From here on, p will be reused as part of the response */
		buf = payloadptr;
	}
	/* at this point packet has enough headroom and response buffer to handle the requst */
	if (cmd == (OID_BCM_SETINFORMATION))
	{
		/* Check packet */
		if (inlen < SETINFORMATION_SIZE) {
			ret = BCME_BADLEN;
			goto indicate;
		}

		PKTPULL(dngl_proto->osh, p, SETINFORMATION_SIZE);
		sinfo = (setinformation_t *)buf;
		/* Change OID */
		cmd = sinfo->oid;
		/* Adjust buffer */
		buf += SETINFORMATION_SIZE;
		inlen -= SETINFORMATION_SIZE;
		flags |= MSGBUF_IOC_ACTION_MASK;
	}

	/*
	 * Intercepting selected wlc IOCTL's requires
	 * unwrapping of the cmd in case of NDIS OIDS's
	 */
	savecmd = cmd;

	if ((cmd >= WL_OID_BASE) && (cmd < (WL_OID_BASE + WLC_LAST)))
		cmd -= WL_OID_BASE;

	switch (cmd) {

	case WLC_SET_VAR:
		if (!strncmp((char *)buf, "bus:", 4)) {
			ret = bus_ops->iovar(dngl_proto->bus, (char *)buf, inlen, NULL, TRUE);
			goto indicate;
		}
		break;

	case WLC_GET_VAR:
		if (!strncmp((char *)buf, "bus:", 4)) {
			ret = bus_ops->iovar(dngl_proto->bus, (char *)buf, outlen, &outlen, FALSE);
			goto indicate;
		}
		break;

	case WLC_ECHO:
		/* No action necessary, simply reflect content back to the host */
		goto indicate;

#ifdef BCMDBG
	case WLC_SET_MSGLEVEL:
		dngl_msglevel = ltoh_ua((uint32 *) buf);
		break;
#endif

#ifdef BCMET
	case ETCUP:
#else
	case WLC_UP:
#endif /* BCMET */
		/* make sure the attached device has been opened */
		ret = dngl_opendev(dngl_proto->dngl);
		goto indicate;
	}
	cmd = savecmd;
	if (flags & MSGBUF_IOC_ACTION_MASK) {
		ret = dngl_dev_set_oid(dngl_proto->dngl, ifindex, cmd, buf, inlen, (int *)&used,
		                       (int *)&needed);
	} else {
		ret = dngl_dev_query_oid(dngl_proto->dngl, ifindex, cmd, buf, outlen,
			(int *)&outlen, (int *)&needed);
	}

indicate:
	if (ioct_resp_buf == NULL) {
		/* Update packet length if the ioctl response was written to the packet */
		PKTSETLEN(dngl_proto->osh, p, outlen);
	}
	retbuf_len = outlen;

	if (ret)
		outlen = needed;

	msgbuf_indicate(dngl_proto, cmd, xt_id, flags, ret, buf, outlen, p,
	                NULL, retbuf_len, use_altinterface);
}

/* Fn to send out ioctl responses to host */
/* From protocol layer form only single packet with both response and completion message */
/* Bus layer will separate out both and send response first and then cmplt message */
static void
msgbuf_indicate(dngl_proto_info_t *dngl_proto, uint32 cmd, uint16 xt_id,
	uint32 flags, uint32 status, uchar *buf, ulong len, void *p, ret_buf_t * ret_buf,
                uint16 retbuf_len, bool use_altinterface)
{
	ioctl_comp_resp_msg_t *ioct_resp;
	trace("%s", dngl_proto->name);

	ASSERT(p != NULL);

	if (PKTHEADROOM(dngl_proto->osh, p) < (sizeof(ioctl_comp_resp_msg_t))) {
		err("bad return buffer!");
		return;
	}
	PKTPUSH(dngl_proto->osh, p, sizeof(ioctl_comp_resp_msg_t));
	ioct_resp = (ioctl_comp_resp_msg_t *)PKTDATA(dngl_proto->osh, p);

	/* cmn hdr */
	ioct_resp->cmn_hdr.msg_type = MSG_TYPE_IOCT_PYLD;
	ioct_resp->cmn_hdr.if_id = 0;
	ioct_resp->cmn_hdr.flags = 0;
	ioct_resp->compl_hdr.flow_ring_id = 1;
	ioct_resp->trans_id = htol16(xt_id);
	ioct_resp->cmd = htol32(cmd);

	/* status */
	ioct_resp->compl_hdr.status = htol16((uint16)status);

	ioct_resp->resp_len = htol16(retbuf_len);

	/* if it was received on the alternate interface response must be on same */
	if (use_altinterface)
		PKTSETALTINTF(p, TRUE);

	/* Send the pkt chain */
	bus_ops->sendctl(dngl_proto->bus, p);
}

/* Adds message buf header to all packets going from dongle to host */
/* wl event packets require special header */
/* RX packets need a dummy cmn hdr which will be stripped of in bus layer */
void *msgbuf_proto_pkt_header_push(void *proto, void *p)
{
	dngl_proto_info_t *dngl_proto = (dngl_proto_info_t *)proto;
	cmn_msg_hdr_t *msg = NULL;
	uint16 hdrlen;
	bool brcm_specialpkt;
	struct bdc_header *h;
	uint8	bdc_len = 0;
	bool metadata_needed = TRUE;

	osl_t *osh = dngl_proto->osh;

	brcm_specialpkt = PKTTYPEEVENT(osh, p) || PKTMSGTRACE(p);

	hdrlen = sizeof(cmn_msg_hdr_t);
	/* dont add metadata if host has not added metadata addr/len info */
	if ((PKTISRXFRAG(osh, p)) && (PKTFRAGMETADATALEN(osh, p) == 0))
		metadata_needed = FALSE;

	if (!brcm_specialpkt) {
		/* Make sure we have min 8 bytes for rxmeta data */
		if (metadata_needed) {
			if (!PKTDATAOFFSET(p))
				bdc_len = BCMPCIE_D2H_METADATA_MINLEN;
			else
				bdc_len = BCMPCIE_D2H_METADATA_HDRLEN;
			PKTSETDATAOFFSET(p, PKTDATAOFFSET(p) + (bdc_len/4));
			hdrlen += bdc_len;
		}
	}

	if (PKTHEADROOM(osh, p) < hdrlen) {
		void *p1;
		int plen = PKTLEN(osh, p);

		/* Alloc a packet that will fit all the data; chaining the header won't work */
		if ((p1 = PKTGET(osh, plen + hdrlen, TRUE)) == NULL) {
			err("PKTGET pkt size %d + headroom %d failed\n", plen,
			    hdrlen);
			PKTFREE(osh, p, TRUE);
			return NULL;
		}

		/* Transfer other fields */
		PKTSETPRIO(p1, PKTPRIO(p));
		PKTSETSUMGOOD(p1, PKTSUMGOOD(p));
		PKTSETDATAOFFSET(p1, PKTDATAOFFSET(p));
		if (PKT80211(p))
			PKTSET80211(p1);
		bcopy(PKTDATA(osh, p), PKTDATA(osh, p1) + hdrlen, plen);
		PKTFREE(osh, p, TRUE);

		p = p1;
	} else
			PKTPUSH(osh, p, hdrlen);

	msg = (cmn_msg_hdr_t *)PKTDATA(osh, p);

	/* Common msg hdr, [Length, Type] */
	if (brcm_specialpkt) {
		msg->msg_type = MSG_TYPE_WL_EVENT;
	} else {
		msg->msg_type = MSG_TYPE_RX_PYLD;
		if (metadata_needed) {
			/* update BDC header for rx meta info */
			h = (struct bdc_header *)((char *)msg + sizeof(cmn_msg_hdr_t));
			bzero(h, bdc_len);
			h->priority = PKTPRIO(p);
			if (PKT80211(p))
				h->flags2 |= BDC_FLAG_80211_PKT;
		}
	}

	/* Init the metadata header portion when the spec comes in */

	return p;
}

/* removes cmn message hdr added by host */
/* this fn will be modified once wplit hdr functionality is up */
int msgbuf_proto_pkt_header_pull(void *proto, void *p)
{
	return 0;

	/* Nothing to be populated here */
}
void msgbuf_proto_dev_event(void *proto, void *data)
{

}
