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

/*
 *******************************************************************************
 * File Name : fpi_spu.c
 *
 *******************************************************************************
 */

#include <linux/netdevice.h>
#include <linux/blog.h>
#include <linux/bcm_log.h>
#include <net/xfrm.h>
#include "fpi.h"

static int fpi_set_blog_esptx_dst_p(Blog_t *blog_p)
{
	struct dst_entry *dst_p;
	struct net *net;
	struct flowi4 fl4;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
	struct sec_path *sp = blog_p->esprx.secPath_p;
	struct xfrm_state *x = sp->xvec[sp->len-1];
#else
	struct xfrm_state *x = blog_p->esprx.xfrm_st;
#endif

	net = dev_net(blog_p->tx_dev_p);
	flowi4_init_output(&fl4, 0, 0, 0, RT_SCOPE_UNIVERSE,
			blog_p->key.protocol, FLOWI_FLAG_ANYSRC,
			blog_p->tx.tuple.daddr, blog_p->tx.tuple.saddr,
			0, 0, sock_net_uid(net, NULL));
	dst_p = (struct dst_entry *)ip_route_output_flow(net, &fl4, NULL);
	if (IS_ERR(dst_p))
		return -1;
	if (!dst_p || dst_p->xfrm != x) {
		dst_release(dst_p);
		return -1;
	}

	blog_p->esptx.dst_p = dst_p;
	return 0;
}

int fpi_ctx_to_blog_spu(fpi_flow_t *flow_p, Blog_t *blog_p)
{
	fpi_l3l4_key_t *l3l4_key_p = &flow_p->key.l3l4_key;
	fpi_context_t *ctx_p = &flow_p->context;
	xfrm_address_t daddr = {};
	int i;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
	struct sec_path *sp;
#endif
	struct xfrm_state *x;

	if (l3l4_key_p->esp_spi_mode == FPI_ESP_IGNORED)
		return -EINVAL;

	if (blog_p->tx.info.phyHdrType == BLOG_SPU_US) {
		for (i = 0; i < 4; i++)
			daddr.a6[i] = htonl(ctx_p->dst_ip[i]);
	} else {
		daddr.a4 = blog_p->tx.tuple.daddr;
	}

	x = xfrm_state_lookup(dev_net(blog_p->tx_dev_p), 0, &daddr,
			htonl(l3l4_key_p->esp_spi), IPPROTO_ESP, 
			l3l4_key_p->is_ipv6 ? AF_INET6 : AF_INET);

	if (!x) {
		BCM_LOG_ERROR(BCM_LOG_ID_FPI,
			"no xfrm state for daddr 0x%x esp_spi 0x%08x\n",
			ntohl(daddr.a4), l3l4_key_p->esp_spi);
		return -EINVAL;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
	sp = secpath_dup(NULL); 
	if (!sp) {
		xfrm_state_put(x);
		return -ENOMEM;
	}
	sp->xvec[sp->len++] = x;
	blog_p->esprx.secPath_p = sp;
#else
	blog_p->esprx.xfrm_st = x;
#endif

	if (blog_p->tx.info.phyHdrType == BLOG_SPU_US) {
		if (fpi_set_blog_esptx_dst_p(blog_p)) {
			xfrm_state_put(x);
			BCM_LOG_ERROR(BCM_LOG_ID_FPI,
				"invalid daddr 0x%x esp_spi 0x%08x\n",
				ntohl(daddr.a4), l3l4_key_p->esp_spi);
			return -EINVAL;
		}
		blog_p->esptx_tuple_p = &blog_p->deltx_tuple;
		blog_p->esptx_tuple_p->saddr = htonl(ctx_p->src_ip[0]);
		blog_p->esptx_tuple_p->daddr = htonl(ctx_p->dst_ip[0]);
		if (l3l4_key_p->esp_spi_mode == FPI_ESP_IN_UDP) {
			blog_p->esptx_tuple_p->port.source =
				htons(ctx_p->src_port);
			blog_p->esptx_tuple_p->port.dest =
				htons(ctx_p->dst_port);
		} else {
			blog_p->esptx_tuple_p->esp_spi =
				htonl(l3l4_key_p->esp_spi);
		}
		blog_p->esptx_tuple_p->tos = l3l4_key_p->packet_priority;
		blog_p->esptx_tuple_p->ttl = 32;
		blog_p->tx.info.bmap.DEL_IPv4 = 1;
		blog_p->tx.info.bmap.ESP = 1;
	} else {
		blog_p->rx.info.bmap.ESP = 1;
	}

	if (l3l4_key_p->esp_spi_mode == FPI_ESP_IN_UDP) {
		blog_p->esp_over_udp_spi = htonl(l3l4_key_p->esp_spi);
		blog_p->tx.info.bmap.ESPoUDP = 1;
		blog_p->rx.info.bmap.ESPoUDP = 1;
	}

	return 0;
}

void fpi_cleanup_blog_spu(Blog_t *blog_p)
{
	if (!blog_p)
		return;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
	secpath_put(blog_p->esprx.secPath_p);
#else
	if (blog_p->esprx.xfrm_st)
		xfrm_state_put(blog_p->esprx.xfrm_st);
#endif
	dst_release(blog_p->esptx.dst_p);
}
