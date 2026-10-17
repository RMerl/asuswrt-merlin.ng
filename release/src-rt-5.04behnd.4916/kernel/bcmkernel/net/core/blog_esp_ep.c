/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom 
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

#include <linux/version.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5,15,0))
//#define DEBUG
#define pr_fmt(fmt) "blog_esp_ep: " fmt
#include <linux/nbuff.h>
#include <net/xfrm.h>
#include <net/esp.h>
#include <crypto/aead.h>
#include <pktHdr.h>
#include <linux/debugfs.h>

#define pr_dbg(fmt, ...)                                   \
	pr_debug("cpu%d:%pSR->%s:%d " fmt, smp_processor_id(), \
		__builtin_return_address(0),__func__,__LINE__,	   \
		## __VA_ARGS__);

#ifdef DEBUG
#define pr_buf(pre, buf, len)        do {                  \
	if (blog_esp_endpoint > 1)                             \
		print_hex_dump_debug(pre, DUMP_PREFIX_OFFSET,      \
				16, 1, buf, len, true);                    \
} while (0)
#else
#define pr_buf(pre, buf, len)        do {                  \
} while (0)
#endif

// copied from esp4.c
struct esp_skb_cb {
	struct xfrm_skb_cb xfrm;
	void *tmp;
};
#define ESP_SKB_CB(__skb) ((struct esp_skb_cb *)&((__skb)->cb[0]))

#define BLOG_ESP_US BLOG_SPU_US
#define BLOG_ESP_DS BLOG_SPU_DS

enum {
	BLOG_ESP_DIR_US, // upstream
	BLOG_ESP_DIR_DS  // downstream
};

#define MAX_IV_SIZE	32
struct blog_esp_aead_req_data {
	pNBuff_t pnbuf;
	union {
		struct dst_entry *dst; // for US
		struct xfrm_state *xfrm_st; // for DS
	};
	struct net_device *rx_dev_p;
	struct {
		uint16_t ipv6 :1;
	} flags;
	uint16_t esph_offset;
	uint16_t *ppre_spi;
	__be32 pre_spi, spi, seq_no;
	uint8_t iv[MAX_IV_SIZE];
	struct scatterlist sg;
	struct aead_request req;	// must be the last field
};

static struct net_device  *blog_esp_us_dev;
static struct net_device  *blog_esp_ds_dev;

static const char* action_names[] = {
	"0-done", "1-norm", "2-blog", "3-drop"
};

static void blog_esp_flush_flow(struct net_device *dev, struct xfrm_state *xs,
		BlogFlushMetadataFunc_t bfmd_fn)
{
	BlogFlushParams_t pa = {
		.flush_dev = 1,
		.flush_meta = 1,
		.devid = dev->ifindex,
		.metadata_p = xs,
		.func_p = bfmd_fn,
	};

	blog_lock();
	blog_notify(FLUSH, xs_net(xs), (unsigned long)&pa, 0);
	blog_unlock();
}

static int blog_esp_us_bfmd_fn(void *metadata_p, const Blog_t *const blog_p)
{
	struct dst_entry *dst = blog_p->esptx.dst_p;

	return dst && (dst->xfrm == metadata_p);
}

static void blog_esp_output_done(struct crypto_async_request *base, int err)
{
	struct blog_esp_aead_req_data *ard = base->data;
	struct dst_entry *dst = ard->dst;
	struct xfrm_state *xs = dst->xfrm;
	struct crypto_aead *aead = xs->data;
	pNBuff_t pnbuf = ard->pnbuf;
	BlogFcArgs_t fcArgs = {};
	BlogAction_t action;
	struct sk_buff *skb;
	unsigned char *pdata;
	unsigned int pktlen, headroom;

	pr_dbg("pnbuf %px xs %px refcnt %d err %d ppre_spi %px\n",
			pnbuf, xs, refcount_read(&xs->refcnt), err, ard->ppre_spi);
	if (unlikely(err))
		goto done;

	if (ard->ppre_spi) {
		pr_buf("-ppre_spi:", ard->ppre_spi, 12);
		_write32_align16(ard->ppre_spi, ard->pre_spi);
		_write32_align16(ard->ppre_spi+2, ard->spi);
		pr_buf("+ppre_spi:", ard->ppre_spi, 12);
	}
	fcArgs.esp_ivsize = crypto_aead_ivsize(aead);
	fcArgs.esp_icvsize = crypto_aead_authsize(aead);
	fcArgs.esp_over_udp = !!xs->encap;
	fcArgs.esp_mode = (xs->props.mode == XFRM_MODE_TRANSPORT) ?
		BLOG_ESP_MODE_TRANSPORT : BLOG_ESP_MODE_TUNNEL;
	fcArgs.esp_spi = xs->id.spi;

	if (IS_SKBUFF_PTR(pnbuf)) {
		skb = (struct sk_buff *)PNBUFF_2_PBUF(pnbuf);
		pdata = skb->data;
		pktlen = skb->len;
		headroom = 0;
		action = blog_sinit(skb, blog_esp_us_dev, TYPE_IP, 0,
				BLOG_ESP_US, &fcArgs);
	} else {
		FkBuff_t *fkb = PNBUFF_2_FKBUFF(pnbuf);
		headroom = (int)(fkb->data - BCM_PFKBUFF_TO_PHEAD(fkb));
		pdata = fkb->data;
		pktlen = fkb->len;
		skb = NULL;
		action = blog_finit(fkb, blog_esp_us_dev, TYPE_IP, 0,
				BLOG_ESP_US, &fcArgs);
	}
	pr_dbg("pnbuf %px skb %px pdata %px pktlen %u headroom %u blog init action %s\n",
			pnbuf, skb, pdata, pktlen, headroom, action_names[action]);
	pr_buf("pdata:", pdata, pktlen);
	switch (action) {
	case PKT_DONE:
		pnbuf = NULL;	// do not free the pnbuf here
		break;
	case PKT_NORM:		// fkb was released; initialize it again for xlate
		if (!skb)
			fkb_init(pdata, headroom, pdata, pktlen);
		fallthrough;
	case PKT_BLOG:
		if (!skb)
			skb = nbuff_xlate(pnbuf);
		skb_dst_set(skb, dst_clone(dst));
		skb_reset_network_header(skb);
		skb->dev = blog_esp_us_dev;
		xfrm_output_resume(skb, 0);
		break;
	case PKT_DROP:
	default:
		break;
	}

done:
	dst_release(dst);
	if (pnbuf)
		nbuff_free(pnbuf);
	kfree(ard);
}

static int blog_esp_us_dev_xmit_args(pNBuff_t pnbuf, struct net_device *dev, BlogFcArgs_t *args)
{
	struct dst_entry *dst = args->esptx_dst_p;
	struct xfrm_state *xs;
	struct sk_buff *skb;
	struct net *net;
	unsigned char *pdata, *iv;
	int nbuflen, esph_offset, alen, ivsize, datalen, padlen, assoclen;
	__be32 spi;
	uint32_t seqno, seqhi=0;
	__be64 seq64;
	struct crypto_aead *aead;
	struct blog_esp_aead_req_data *ard;
	int ret = -EOPNOTSUPP;

	if (unlikely(!dst)) {
		pr_dbg("esptx dst is NULL\n");
		goto discard_1;
	}
	xs = dst->xfrm;
	if (unlikely(!xs)) {
		pr_dbg("dst xfrm state is NULL\n");
		goto discard;
	}

	pr_dbg("pnbuf %px dev %s xs %px refcnt %d\n",
			pnbuf, dev ? dev->name : "null", xs,
			xs ? refcount_read(&xs->refcnt) : 0);

	if (IS_SKBUFF_PTR(pnbuf)) {
		skb = (struct sk_buff *)PNBUFF_2_PBUF(pnbuf);
		pdata = skb->data;
		nbuflen = skb->len;
		skb_shinfo(skb)->dirty_p = NULL; // make sure dirty_p is NULL
	} else {
		FkBuff_t *fkb = (FkBuff_t *)PNBUFF_2_PBUF(pnbuf);
		fkb = fkb_unshare(fkb);
		pdata = fkb->data;
		nbuflen = fkb->len;
		skb = NULL;
	}

	if (*pdata == 0x45) {
		struct iphdr *iph = (struct iphdr *)pdata;
		esph_offset = BLOG_IPV4_HDR_LEN;
		if (iph->protocol == BLOG_IPPROTO_UDP)
			esph_offset += BLOG_UDP_HDR_LEN;
	} else if (*pdata == 0x60) {
		struct ipv6hdr *ip6h = (struct ipv6hdr *)pdata;
		esph_offset = BLOG_IPV6_HDR_LEN;
		if (ip6h->nexthdr == BLOG_IPPROTO_UDP) {
			pr_dbg("esp over UDP in IPV6 is not supported\n");
			goto discard;
		}
	} else {
		pr_dbg("packet type 0x%x is not supported\n", *pdata);
		goto discard;
	}

	net = xs_net(xs);
	spi = _read32_align16((uint16_t *)&pdata[esph_offset]);
	spin_lock_bh(&xs->lock);
	/* A new key has genid = 0. When the key is renewed, genid is incremented.
	 * We want to evict the flow from fcache when the key has been renewed or
	 * expired.
	 */
	if (spi != xs->id.spi || xs->genid) {
		pr_dbg("spi 0x%08x xs id.spi 0x%08x genid 0x%08x\n", spi, xs->id.spi, xs->genid);
		ret = -1;
		goto unlock_xs;
	}

	if (unlikely(xs->km.state != XFRM_STATE_VALID)) {
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATEINVALID);
		ret = -EINVAL;
		goto unlock_xs;
	}

	ret = xfrm_state_check_expire(xs);
	if (ret) {
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATEEXPIRED);
		goto unlock_xs;
	}

	/* pass NULL skb since we have an fkb if there is an error then we will
	 * create an skb and call again so that the error is reported need to
	 * change relevant code in xfrm/xfrm_replay.c accordingly  */
	ret = xfrm_replay_overflow(xs, NULL);
	if (ret) {
		if (!skb) {
			/* nbuf is an fkb - translate to skb call replay function again for
			 * auditing
			 */
			skb = nbuff_xlate(pnbuf);
		}
		skb_reset_network_header(skb);
		xfrm_replay_overflow(xs, skb);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATESEQERROR);
		kfree_skb(skb);
		pnbuf = NULL;
		goto unlock_xs;
	}
	if (xs->replay_esn) {
		seqno = xs->replay_esn->oseq;
		seqhi = xs->replay_esn->oseq_hi;
	} else {
		seqno = xs->replay.oseq;
	}

	aead = xs->data;
	alen = crypto_aead_authsize(aead);
	ivsize = crypto_aead_ivsize(aead);
	/* only count inner ip header and data */
	padlen  = pdata[nbuflen - alen - 2];
	datalen = nbuflen - esph_offset - BLOG_ESP_SPI_LEN - BLOG_ESP_SEQNUM_LEN
		- ivsize - padlen - BLOG_ESP_PADLEN_LEN - BLOG_ESP_NEXT_PROTO_LEN
		- alen;
	xs->curlft.bytes += datalen;
	xs->curlft.packets++;

unlock_xs:
	spin_unlock_bh(&xs->lock);
	if (ret) {
		pr_dbg("ret %d discard pnbuf %px flush flows with xs %px\n",
				ret, pnbuf, xs);
		blog_esp_flush_flow(blog_esp_us_dev, xs, blog_esp_us_bfmd_fn);
		goto discard;
	}

	ard = kzalloc(sizeof(struct blog_esp_aead_req_data) +
			crypto_aead_reqsize(aead), GFP_ATOMIC);
	if (!ard) {
		pr_dbg("out of memory\n");
		goto discard;
	}
	ard->pnbuf = pnbuf;
	ard->dst = dst;
	_write32_align16((uint16_t *)&pdata[esph_offset+BLOG_ESP_SPI_LEN], htonl(seqno));
	assoclen = BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN;
	if (xs->replay_esn) {
		ard->ppre_spi = (uint16_t*)&pdata[esph_offset - BLOG_ESP_SPI_LEN];
		ard->pre_spi = _read32_align16(ard->ppre_spi);
		ard->spi = spi;
		pr_buf("-ppre_spi:", ard->ppre_spi, 12);
		_write32_align16(ard->ppre_spi, spi);
		_write32_align16((uint16_t *)&pdata[esph_offset], htonl(seqhi));
		pr_buf("+ppre_spi:", ard->ppre_spi, 12);
		sg_init_one(&ard->sg, ard->ppre_spi,
				nbuflen - esph_offset + sizeof(__be32));
		assoclen += sizeof(__be32);
	} else {
		sg_init_one(&ard->sg, &pdata[esph_offset], nbuflen - esph_offset);
	}

	aead_request_set_tfm(&ard->req, aead);
	aead_request_set_callback(&ard->req, 0, blog_esp_output_done, ard);
	iv = &pdata[esph_offset + sizeof(struct ip_esp_hdr)];
	memset(iv, 0, ivsize);
	iv = ard->iv;
	seq64 = cpu_to_be64(seqno + ((u64)seqhi << 32));
	memcpy(iv + ivsize - min(ivsize, 8),
			((u8 *)&seq64) + 8 - min(ivsize, 8), min(ivsize, 8));
	pr_buf("iv:", iv, ivsize);
	aead_request_set_crypt(&ard->req, &ard->sg, &ard->sg,
			ivsize + datalen + padlen + 2, iv);
	aead_request_set_ad(&ard->req, assoclen);
	pr_dbg("pnbuf %px pdata %px nbuflen %d esph_offset %d alen %d ivsize %d "
			"datalen %d padlen %d assoclen %d seqno %u seqhi %u req %px\n",
			pnbuf, pdata, nbuflen, esph_offset, alen, ivsize,
			datalen, padlen, assoclen, seqno, seqhi, &ard->req);
	pr_buf("pdata:", pdata, nbuflen);

	ret = crypto_aead_encrypt(&ard->req);
	pr_dbg("pnbuf %px encrypt ret %d\n", pnbuf, ret);
	switch (ret) {
	case 0:
		blog_esp_output_done(&ard->req.base, 0);
		return 0;
	case -EINPROGRESS:
		return 0;
	}

	kfree(ard);
discard:
	dst_release(dst);
discard_1:
	if (pnbuf)
		nbuff_free(pnbuf);
	return ret;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_esp_us_tx_dev_adjust
 * Description  : Calculates the SPU ESP tx_dev_adjust value based on the 
 *              : MTU, length of outgoing packet and the padding alignment.
 * blog_p       : Pointer to the Blog_t object
 * dev_p        : SPUU device
 * blksize      : padding alignment
 * pktlen       : packet length (without MAC header)
 *------------------------------------------------------------------------------
 */
static void blog_esp_us_tx_dev_adjust(Blog_t *blog_p, struct net_device *dev_p,
        BlogFcArgs_t *fcArgs, uint8_t tunl_overhead, uint8_t padlen,  uint16_t pktlen)
{
	int16_t len = pktlen;
	int16_t mtu = dev_p->mtu;
	uint16_t blksize = fcArgs->esp_blksize;
	int8_t adjust=0;

	if (len != dev_p->mtu) {
		if (len < dev_p->mtu) {
			while (len < dev_p->mtu)
				len += blksize;
			adjust = (len - blksize) - mtu;
		} else {
			while (len > dev_p->mtu)
				len -= blksize;
			adjust = len - mtu;
		}
	} else {
		adjust = 0;
	}

	/* reduce HDRs & trailer also */ 
	adjust -= (tunl_overhead + fcArgs->esp_icvsize);
	blog_p->tx_dev_mtu_padlen = padlen;
	blog_p->tx_dev_adjust = adjust;
}

void blog_esp_emit_aead_us(struct sk_buff *skb, void *esp_info)
{
	struct blog_t *blog_p = blog_ptr(skb);
	struct xfrm_state *xs = skb_dst(skb)->xfrm;
	struct esp_info *esp = esp_info;
	struct ip_esp_hdr *esph = esp->esph;
	BlogFcArgs_t fcArgs = {};
	struct crypto_aead *aead;
	struct net_device *skb_dev;
	int esp_mode, esp_over_udp = 0;
	uint16_t eth_proto;
	__be32 seqhi;
	BlogAction_t action;
	unsigned char *pdata;
	unsigned char padlen=0;
	unsigned char tunl_overhead;

	pr_dbg("skb %px dev %s len %d blog_p %px xs %px refcnt %d esph %px "
			"spi 0x%08x seq_no %u/0x%x seq64 0x%llx\n",
			skb, skb->dev ? skb->dev->name : "null", skb->len, blog_p, xs,
			refcount_read(&xs->refcnt), esph, ntohl(esph->spi),
			ntohl(esph->seq_no), ntohl(esph->seq_no), be64_to_cpu(esp->seqno));
	if (!blog_p || !xs || !blog_esp_us_dev)
		goto skip;

	/* ignore packets needing more than one transform */
	if (xfrm_dst_child(skb_dst(skb))->xfrm)
		goto skip;

	switch (xs->props.mode) {
	case XFRM_MODE_TRANSPORT:
		esp_mode = BLOG_ESP_MODE_TRANSPORT;
		break;
	case XFRM_MODE_TUNNEL:
		esp_mode = BLOG_ESP_MODE_TUNNEL;
		break;
	default:
		goto skip;
	}
	eth_proto = ntohs(skb->protocol);
	switch (eth_proto) {
	case ETH_P_IP:
		if (esp_mode != BLOG_ESP_MODE_TRANSPORT && esp->proto != IPPROTO_IPIP)
			goto skip;
		break;
	case ETH_P_IPV6:
		if (esp_mode != BLOG_ESP_MODE_TRANSPORT && esp->proto != IPPROTO_IPV6)
			goto skip;
		break;
	default:
		goto skip;
	}

	if (xs->encap) {
		if (xs->encap->encap_type != UDP_ENCAP_ESPINUDP
				|| eth_proto == ETH_P_IPV6)
			goto skip;
		esp_over_udp = 1;
	}

	blog_p->esptx.dst_p = dst_clone(skb_dst(skb));
	if (!(xs->props.flags & XFRM_STATE_NOPMTUDISC))
		blog_p->esptx.pmtudiscen = 1;
	blog_p->tx.info.bmap.ESP = 1;
	blog_p->esp_mode = esp_mode;
	blog_p->use_xmit_args = 1;
	aead = xs->data;
	fcArgs.esp_ivsize = crypto_aead_ivsize(aead);
	fcArgs.esp_icvsize = crypto_aead_authsize(aead);
	fcArgs.esp_blksize = crypto_aead_blocksize(aead);
	fcArgs.esp_over_udp = esp_over_udp;
	fcArgs.esp_spi = esph->spi;
	fcArgs.dev_xmit = (unsigned long)blog_esp_us_dev_xmit_args;
	skb_dev = skb->dev;
	skb->dev = blog_esp_us_dev;
	// adapt the packet to let fcache learn
	if (xs->props.flags & XFRM_STATE_ESN) {
		// esph{spi|seq_no} points to: |spi         | seqhi | seqno
		// changes into:               |extra->seqhi| spi   | seqno
		pr_buf("-esph:", esph, 12);
		seqhi = esph->seq_no;
		esph->seq_no = esph->spi;
		memcpy(&esph->spi, ESP_SKB_CB(skb)->tmp, sizeof(__be32));
		pr_buf("+esph:", esph, 12);
	}
	if (eth_proto == ETH_P_IP) {
		struct iphdr *iph = (struct iphdr *)skb->data;
		ip_send_check(iph);
	} else {
		struct ipv6hdr *ipv6h = (struct ipv6hdr *)skb->data;
		ipv6h->payload_len = htons(skb->len);
	}

	pdata= skb->data;
	padlen = pdata[skb->len - fcArgs.esp_icvsize - 2];

	/* in tunnel mode include len before ESP header also */
	if(esp_mode == BLOG_ESP_MODE_TUNNEL)
		tunl_overhead = ((unsigned char *)esph - skb->data);
	else
		tunl_overhead = 0;

	tunl_overhead += sizeof(struct ip_esp_hdr) + fcArgs.esp_ivsize
			+ BLOG_ESP_PADLEN_LEN + BLOG_ESP_NEXT_PROTO_LEN;

	blog_esp_us_tx_dev_adjust(blog_p, blog_esp_us_dev,
			&fcArgs, tunl_overhead, padlen, skb->len);
	action = blog_emit_args(skb, blog_esp_us_dev, TYPE_IP, 0,
			BLOG_ESP_US, &fcArgs);
	pr_dbg("skb %px esp_spi 0x%08x blog emit action %s\n",
			skb, ntohl(fcArgs.esp_spi), action_names[action]);
	if (xs->props.flags & XFRM_STATE_ESN) {
		pr_buf("-esph:", esph, 12);
		esph->spi = fcArgs.esp_spi;
		esph->seq_no = seqhi;
		pr_buf("+esph:", esph, 12);
	}
	BUG_ON(action == PKT_DONE);
	skb->dev = skb_dev;
	return;
skip:
	blog_skip(skb, blog_skip_reason_blog_esp_ep);
}

static void blog_esp_input_done(struct crypto_async_request *base, int err)
{
	struct blog_esp_aead_req_data *ard = base->data;
	struct xfrm_state *xs = ard->xfrm_st;
	struct crypto_aead *aead = xs->data;
	pNBuff_t pnbuf = ard->pnbuf;
	BlogFcArgs_t fcArgs = {};
	BlogAction_t action;
	struct sk_buff *skb;
	FkBuff_t *fkb;
	unsigned char *pdata;
	struct iphdr *iph;
	unsigned int pktlen, headroom, hlen, padlen, tlen, tot_len = 0;
	uint8_t nexthdr;
	struct net_device *skb_dev = NULL;

	pr_dbg("pnbuf %px xs %px refcnt %d err %d ipv6 %d xfrm_mode %d "
			"ppre_spi %px\n",
			pnbuf, xs, refcount_read(&xs->refcnt), err,
			ard->flags.ipv6, xs->props.mode, ard->ppre_spi);
	if (unlikely(err))
		goto done;

	if (ard->flags.ipv6 && xs->props.mode == XFRM_MODE_TRANSPORT)
		goto done;

	if (IS_SKBUFF_PTR(pnbuf)) {
		skb = (struct sk_buff *)PNBUFF_2_PBUF(pnbuf);
		pdata = skb->data;
		pktlen = skb->len;
	} else {
		fkb = PNBUFF_2_FKBUFF(pnbuf);
		pdata = fkb->data;
		pktlen = fkb->len;
	}

	fcArgs.esp_inner_pkt = 1;
	// fcArgs.esp_spi = 0;
	// fcArgs.esp_over_udp = !!xs->encap;
	fcArgs.esp_mode = (xs->props.mode == XFRM_MODE_TRANSPORT) ?
		BLOG_ESP_MODE_TRANSPORT : BLOG_ESP_MODE_TUNNEL;
	if (ard->ppre_spi) {
		pr_buf("-ppre_spi:", ard->ppre_spi, 12);
		_write32_align16(ard->ppre_spi, ard->pre_spi);
		_write32_align16(ard->ppre_spi+2, ard->spi);
		pr_buf("+ppre_spi:", ard->ppre_spi, 12);
	}
	fcArgs.esp_ivsize = crypto_aead_ivsize(aead);
	fcArgs.esp_icvsize = crypto_aead_authsize(aead);

	hlen = ard->esph_offset + sizeof(struct ip_esp_hdr) + fcArgs.esp_ivsize;
	padlen = pdata[pktlen - fcArgs.esp_icvsize - 2];
	nexthdr = pdata[pktlen - fcArgs.esp_icvsize - 1];
	tlen = fcArgs.esp_icvsize + padlen + 2;

	pr_dbg("pnbuf %px pdata %px pktlen %u hlen %u padlen %u nexthdr %u "
			"tlen %u ivsize %u icvsize %u esp_mode %d esph_offset %u\n",
			pnbuf, pdata, pktlen, hlen, padlen, nexthdr,
			tlen, fcArgs.esp_ivsize, fcArgs.esp_icvsize, fcArgs.esp_mode,
			ard->esph_offset);
	pr_buf("pdata:", pdata, pktlen);

	if (IS_SKBUFF_PTR(pnbuf)) {
		__skb_trim(skb, skb->len - tlen);
		__skb_pull(skb, hlen);

		if (fcArgs.esp_mode) {
			iph = (struct iphdr*) pdata;
			tot_len = ntohs(iph->tot_len);
			__skb_push(skb, BLOG_IPV4_HDR_LEN);
			tot_len += (BLOG_IPV4_HDR_LEN - hlen - tlen);
			iph->tot_len = htons(tot_len);
			iph->protocol = nexthdr;
			ip_send_check(iph);
			memmove(skb->data, iph, BLOG_IPV4_HDR_LEN);
		}

		skb_reset_network_header(skb);
		skb_reset_mac_header(skb);
		skb_set_transport_header(skb, 0);
		pktlen = skb->len;
		pdata = skb->data;
		action = blog_sinit(skb, blog_esp_ds_dev, TYPE_IP, 0,
				BLOG_ESP_DS, &fcArgs);
		headroom = 0;
	} else {
		fkb->len -= tlen;
		_fkb_pull(fkb, hlen);

		if (fcArgs.esp_mode) {
			iph = (struct iphdr*) pdata;
			tot_len = ntohs(iph->tot_len);
			_fkb_push(fkb, BLOG_IPV4_HDR_LEN);
			tot_len += (BLOG_IPV4_HDR_LEN - hlen - tlen);
			iph->tot_len = htons(tot_len);
			iph->protocol = nexthdr;
			ip_send_check(iph);
			memmove(fkb->data, iph, BLOG_IPV4_HDR_LEN);
		}

		headroom = (int)(fkb->data - BCM_PFKBUFF_TO_PHEAD(fkb));
		pktlen = fkb->len;
		pdata = fkb->data;
		action = blog_finit(fkb, blog_esp_ds_dev, TYPE_IP, 0,
				BLOG_ESP_DS, &fcArgs);
		skb = NULL;
	}
	pr_dbg("pnbuf %px skb %px pdata %px pktlen %u headroom %u hlen %u padlen %u "
			"tlen %u tot_len %u blog init action %s\n",
			pnbuf, skb, pdata, pktlen, headroom, hlen, padlen,
			tlen, tot_len, action_names[action]);
	pr_buf("pdata:", pdata, pktlen);

	switch (action) {
	case PKT_DONE:
		spin_lock_bh(&xs->lock);
		xfrm_replay_advance(xs, ard->seq_no);
		xs->curlft.bytes += pktlen;
		xs->curlft.packets++;
		spin_unlock_bh(&xs->lock);
		pnbuf = NULL;	// do not free the pnbuf here
		break;
	case PKT_NORM:		// fkb was released; initialize it again for xlate
		if (!skb)
			fkb_init(pdata, headroom, pdata, pktlen);
		skb_dev = ard->rx_dev_p;
		fallthrough;
	case PKT_BLOG:
		if (!skb)
			skb = nbuff_xlate(pnbuf);

		{
			struct sec_path *sp = secpath_set(skb);

			if (!sp)
				goto done;
			xfrm_state_hold(xs);
			sp->xvec[sp->len++] = xs;
			pr_dbg("held xs %px refcnt %d skb %px skb_dev %s\n", xs,
				refcount_read(&xs->refcnt), skb, skb_dev ? skb_dev->name : "null");
		}
		skb->dev = skb_dev ? : blog_esp_ds_dev;
		dev_hold(skb->dev);
		XFRM_SKB_CB(skb)->seq.input.low = ard->seq_no;
		XFRM_SKB_CB(skb)->seq.input.hi =
			htonl(xfrm_replay_seqhi(xs, ard->seq_no));
		if (fcArgs.esp_mode) {
			skb_reset_network_header(skb);
			skb_reset_transport_header(skb);
			__skb_pull(skb, BLOG_IPV4_HDR_LEN);
		}
		xfrm_input_resume(skb, nexthdr);
		pnbuf = NULL;	// do not free the pnbuf here
		break;
	case PKT_DROP:
	default:
		break;
	}

done:
	xfrm_state_put(xs);
	if (pnbuf)
		nbuff_free(pnbuf);
	kfree(ard);
}

static int blog_esp_ds_dev_xmit_args(pNBuff_t pnbuf, struct net_device *dev, BlogFcArgs_t *args)
{
	struct sk_buff *skb;
	unsigned char *pdata, *iv;
	struct xfrm_state *xs = args->esprx_xfrm_st;
	struct net *net;
	struct crypto_aead *aead;
	int nbuflen, esph_offset, hlen, assoclen;
	__be32 spi, seqno;
	struct blog_esp_aead_req_data *ard;
	int is_ipv6 = 0;
	int ret = -1;

	if (unlikely(!xs)) {
		pr_dbg("discard pnbuf %px dev %s xs null\n",
				pnbuf, dev ? dev->name : "null");
		goto discard_1;
	}
	net = xs_net(xs);

	if (IS_SKBUFF_PTR(pnbuf)) {
		skb = (struct sk_buff *)PNBUFF_2_PBUF(pnbuf);
		pdata = skb->data;
		nbuflen = skb->len;
		skb_shinfo(skb)->dirty_p = NULL; // make sure dirty_p is NULL
	} else {
		FkBuff_t *fkb = (FkBuff_t *)PNBUFF_2_PBUF(pnbuf);
		fkb = fkb_unshare(fkb);
		pdata = fkb->data;
		nbuflen = fkb->len;
		skb = NULL;
	}

	pr_dbg("pnbuf %px len %u dev %s xs %px refcnt %d\n",
			pnbuf, nbuflen, dev ? dev->name : "null", xs,
			refcount_read(&xs->refcnt));
	if (*pdata == 0x45) {
		struct iphdr *iph = (struct iphdr *)pdata;
		esph_offset = BLOG_IPV4_HDR_LEN;
		if (iph->protocol == BLOG_IPPROTO_UDP)
			esph_offset += BLOG_UDP_HDR_LEN;
	} else if (*pdata == 0x60) {
		struct ipv6hdr *ip6h = (struct ipv6hdr *)pdata;
		esph_offset = BLOG_IPV6_HDR_LEN;
		if (ip6h->nexthdr == BLOG_IPPROTO_UDP) {
			pr_dbg("esp over UDP in IPV6 is not supported\n");
			goto discard;
		}
		is_ipv6 = 1;
	} else {
		pr_dbg("packet type 0x%x is not supported\n", *pdata);
		goto discard;
	}

	spi = _read32_align16((uint16_t *)&pdata[esph_offset]);
	hlen = esph_offset + BLOG_ESP_SPI_LEN;
	seqno = _read32_align16((uint16_t *)&pdata[hlen]);
	spin_lock_bh(&xs->lock);
	if (spi != xs->id.spi || xs->genid) {
		pr_dbg("spi 0x%08x xs id.spi 0x%08x genid 0x%08x\n", spi, xs->id.spi, xs->genid);
		goto unlock_xs;
	}
	ret = xfrm_replay_check(xs, NULL, seqno);
	if (ret) {
		if (!skb)
			skb = nbuff_xlate(pnbuf);
		skb_reset_network_header(skb);
		xfrm_replay_check(xs, skb, seqno);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATESEQERROR);
		kfree_skb(skb);
		pnbuf = NULL;
		goto unlock_xs;
	}
	ret = xfrm_state_check_expire(xs);
	if (ret)
		XFRM_INC_STATS(net, LINUX_MIB_XFRMINSTATEEXPIRED);

unlock_xs:
	spin_unlock_bh(&xs->lock);
	if (ret)
		goto discard;

	aead = xs->data;
	ard = kzalloc(sizeof(struct blog_esp_aead_req_data) +
			crypto_aead_reqsize(aead), GFP_ATOMIC);
	if (!ard) {
		pr_dbg("out of memory\n");
		goto discard;
	}
	ard->pnbuf = pnbuf;
	ard->xfrm_st = xs;
	ard->rx_dev_p = args->esprx_dev_p;
	assoclen = BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN;
	if (xs->props.flags & XFRM_STATE_ESN) {
		ard->ppre_spi = (uint16_t*)&pdata[esph_offset - BLOG_ESP_SPI_LEN];
		ard->pre_spi = _read32_align16(ard->ppre_spi);
		ard->spi = spi;
		_write32_align16(ard->ppre_spi, spi);
		_write32_align16((uint16_t *)&pdata[esph_offset],
				htonl(xfrm_replay_seqhi(xs, seqno)));
		sg_init_one(&ard->sg, ard->ppre_spi, nbuflen - esph_offset + sizeof(__be32));
		assoclen += sizeof(__be32);
	} else {
		sg_init_one(&ard->sg, &pdata[esph_offset], nbuflen - esph_offset);
	}
	ard->flags.ipv6 = is_ipv6;
	ard->esph_offset = esph_offset;
	ard->seq_no = seqno;

	aead_request_set_tfm(&ard->req, aead);
	aead_request_set_callback(&ard->req, 0, blog_esp_input_done, ard);
	iv = &pdata[esph_offset + sizeof(struct ip_esp_hdr)];
	aead_request_set_crypt(&ard->req, &ard->sg, &ard->sg,
			nbuflen - esph_offset - sizeof(struct ip_esp_hdr), iv);
	aead_request_set_ad(&ard->req, assoclen);
	pr_dbg("pnbuf %px pdata %px nbuflen %d esph_offset %d hlen %d "
			"assoclen %d is_ipv6 %d spi 0x%08x seqno %u req %px\n",
			pnbuf, pdata, nbuflen, esph_offset, hlen,
			assoclen, is_ipv6, ntohl(spi), ntohl(seqno), &ard->req);
	pr_buf("pdata:", pdata, nbuflen);

	ret = crypto_aead_decrypt(&ard->req);
	pr_dbg("pnbuf %px decrypt ret %d\n", pnbuf, ret);
	switch (ret) {
	case 0:
		blog_esp_input_done(&ard->req.base, 0);
		return 0;
	case -EINPROGRESS:
		return 0;
	}

	kfree(ard);
discard:
	xfrm_state_put(xs);
discard_1:
	if (pnbuf)
		nbuff_free(pnbuf);
	return ret;
}

void blog_esp_emit_aead_ds(struct sk_buff *skb)
{
	struct blog_t *blog_p = blog_ptr(skb);
	struct xfrm_state *xs = xfrm_input_state(skb);
	struct net_device *skb_dev = skb->dev;
	int esp_mode, esp_over_udp = 0;
	uint16_t eth_proto, esph_offset;
	BlogEsp_t *esprx;
	struct ip_esp_hdr *esph;
	__be32 esp_spi;
	BlogFcArgs_t fcArgs = {};
	struct crypto_aead *aead;
	BlogAction_t action;
	uint8_t nexthdr;

	pr_dbg("skb %px len %u dev %s blog_p %px xs %px refcnt %d\n",
			skb, skb->len, skb_dev ? skb_dev->name : "null", blog_p, xs,
			xs ? refcount_read(&xs->refcnt) : 0);

	if (unlikely(!skb_dev || !xs || !blog_esp_ds_dev))
		goto skip;

	if (!blog_p) {
		pr_dbg("blog null skip skb %px len %u headlen %u xs %px\n",
				skb, skb->len, skb_headlen(skb), xs);
		pr_buf("skb data:", skb->data, skb_headlen(skb));
		goto skip;
	}

	switch (xs->props.mode) {
	case XFRM_MODE_TRANSPORT:
		esp_mode = BLOG_ESP_MODE_TRANSPORT;
		break;
	case XFRM_MODE_TUNNEL:
		esp_mode = BLOG_ESP_MODE_TUNNEL;
		break;
	default:
		goto skip;
	}

	aead = xs->data;
	fcArgs.esp_icvsize = crypto_aead_authsize(aead);
	if (skb_copy_bits(skb, skb->len - fcArgs.esp_icvsize - 1, &nexthdr, 1))
		goto skip;

	eth_proto = ntohs(skb->protocol);
	switch (eth_proto) {
	case ETH_P_IP:
		if (esp_mode != BLOG_ESP_MODE_TRANSPORT && nexthdr != IPPROTO_IPIP)
			goto skip;
		esph_offset = BLOG_IPV4_HDR_LEN;
		break;
	case ETH_P_IPV6:
		if (esp_mode == BLOG_ESP_MODE_TRANSPORT || nexthdr != IPPROTO_IPV6)
			goto skip;
		esph_offset = BLOG_IPV6_HDR_LEN;
		break;
	default:
		goto skip;
	}

	if (xs->encap) {
		if (xs->encap->encap_type != UDP_ENCAP_ESPINUDP
				|| eth_proto == ETH_P_IPV6)
			goto skip;
		esp_over_udp = 1;
		esph_offset += BLOG_UDP_HDR_LEN;
	}

	esprx = &blog_p->esprx;
	if (xs != esprx->xfrm_st) {
		pr_dbg("blog_p %px xs old %px cur %px\n", blog_p, esprx->xfrm_st, xs);
		if (esprx->xfrm_st)
			xfrm_state_put(esprx->xfrm_st);
		xfrm_state_hold(xs);
		esprx->xfrm_st = xs;
		pr_dbg("blog_p %px held xs %px refcnt %d\n",
				blog_p, xs, refcount_read(&xs->refcnt));
	}
	blog_p->rx.info.bmap.ESP = 1;
	blog_p->esp_mode = esp_mode;
	esph = (struct ip_esp_hdr *)skb->data;
	esp_spi = _read32_align16((uint16_t *)&esph->spi);
	if (esp_over_udp && (!esp_spi || esp_spi == htonl(0xFF00000)))
		goto skip;
	blog_p->use_xmit_args = 1;
	fcArgs.esp_ivsize = crypto_aead_ivsize(aead);
	fcArgs.esp_blksize = crypto_aead_blocksize(aead);
	fcArgs.esp_over_udp = esp_over_udp;
	fcArgs.esp_spi = esp_spi;
	fcArgs.dev_xmit = (unsigned long)blog_esp_ds_dev_xmit_args;
	skb->dev = blog_esp_ds_dev;
	skb_push(skb, esph_offset);
	pr_dbg("skb %px data %px len %d nxth %u esp_over_udp %d esph_offset %u\n",
			skb, skb->data, skb->len, nexthdr, esp_over_udp, esph_offset);
	action = blog_emit_args(skb, blog_esp_ds_dev, TYPE_IP, 0,
			BLOG_ESP_DS, &fcArgs);
	pr_dbg("skb %px esp_spi 0x%08x blog emit action %s\n",
			skb, ntohl(esp_spi), action_names[action]);
	BUG_ON(action == PKT_DONE);
	skb->dev = skb_dev;
	skb_pull(skb, esph_offset);
	return;
skip:
	blog_skip(skb, blog_skip_reason_blog_esp_ep);
}

static netdev_tx_t blog_esp_ndo_start_xmit(struct sk_buff *skb,
		struct net_device *dev) {
	pr_dbg("sending skb %p from dev %s\n", skb, dev->name);
	kfree_skb(skb);
	return NETDEV_TX_OK;
}

static struct net_device_ops blog_esp_dev_ops = {
	.ndo_start_xmit = blog_esp_ndo_start_xmit,
};

static void __init blog_esp_register_netdev(int dir)
{
	struct net_device *dev;

	dev = alloc_netdev(0, dir ? "blog_esp_ds_dev" : "blog_esp_us_dev",
			NET_NAME_UNKNOWN, ether_setup);
	if (!dev) {
		pr_err("failed to allocate netdev for dir %s\n", dir ? "ds" : "us");
		return;
	}

	netdev_bcm_dev_set(dev);
	netdev_path_set_hw_port_type(dev, dir ? BLOG_ESP_DS : BLOG_ESP_US);
	netdev_dummy_dev_set(dev);
	bcm_netdev_ext_field_set(dev, dev_xmit_args,
			dir ? blog_esp_ds_dev_xmit_args : blog_esp_us_dev_xmit_args);
	dev->type = ARPHRD_NONE;
	dev->tx_queue_len = 100;
	dev->flags = IFF_NOARP;
	/* Set mtu to a huge value so that fcache will not fragment packets
	 * destined to this device.
	 */
	dev->mtu = BCM_MAX_MTU_PAYLOAD_SIZE;
	dev->netdev_ops = &blog_esp_dev_ops;
	if (register_netdev(dev)) {
		pr_err("failed to register netdev for dir %s\n", dir ? "ds" : "us");
		free_netdev(dev);
		return;
	}
	rtnl_lock();
	dev_open(dev, NULL); // bring it up
	rtnl_unlock();
	if (dir)
		blog_esp_ds_dev = dev;
	else
		blog_esp_us_dev = dev;
}

uint32_t blog_esp_endpoint = 1;
static int __init blog_esp_ep_init(void)
{
	blog_esp_register_netdev(BLOG_ESP_DIR_US);
	blog_esp_register_netdev(BLOG_ESP_DIR_DS);
	debugfs_create_u32("blog_esp_endpoint", 0600, NULL, &blog_esp_endpoint);
	return 0;
}

device_initcall(blog_esp_ep_init);
#endif // #if (LINUX_VERSION_CODE > KERNEL_VERSION(5,15,0))
