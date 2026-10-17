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

void fpi_key_to_blog_spu(fpi_key_t *key_p, Blog_t *blog_p)
{
	fpi_l3l4_key_t *l3l4_key_p = &key_p->l3l4_key;

	if ((l3l4_key_p->l4_proto == IPPROTO_ESP) &&
	    ((l3l4_key_p->esp_flow_type == ESP_US_FWD) ||
	     (l3l4_key_p->esp_flow_type == ESP_DS))) {
		if (l3l4_key_p->is_ipv6)
			blog_p->tupleV6.ports = htonl(l3l4_key_p->esp_spi);
		else
			blog_p->rx.tuple.ports = htonl(l3l4_key_p->esp_spi);
	}

	if (l3l4_key_p->esp_flow_type == ESP_DS) {
		blog_p->rx.info.bmap.ESP = 1;
		if (l3l4_key_p->l4_proto == IPPROTO_UDP) {
			blog_p->esp_over_udp_spi = htonl(l3l4_key_p->esp_spi);
			blog_p->rx.tuple.esp_spi = htonl(l3l4_key_p->esp_spi);
			blog_p->tx.info.bmap.ESPoUDP = 1;
			if (l3l4_key_p->is_ipv6) {
				blog_p->rx.info.bmap.PLD_IPv6 = 1;
			} else {
				blog_p->rx.info.bmap.PLD_IPv4 = 1;
			}

			blog_p->is_inner_flow = 1;
			blog_p->tunnel.info.flowid = 0;
			blog_p->tunnel.info.key = 0;
		} else
			blog_p->rx.tuple.esp_spi = htonl(l3l4_key_p->esp_spi);
	}

	if (l3l4_key_p->esp_flow_type == ESP_DS_UDP) {
		blog_p->rx.info.bmap.ESP = 1;

		/* the outermost of ESPoUDP downstream flow has to be
		 * set as a tunnel flow, while ESP itself is not
		 * considered as tunnel, because it terminates at host.
		 */
		blog_p->is_tunnel = 1;
		blog_p->tunnel.match.protocol = blog_p->key.protocol;
		blog_p->tunnel.info.flags.is_inner_hdr_l3 = 1;

		/* TODO! as of today, it is not defined to have 2 or
		 * more tunnel-related flows to be bound together
		 * with a ID.  Therefore, we hardcode the flowid
		 * to 0 as always.
		 */
		blog_p->tunnel.info.flowid = 0;
		blog_p->tx.info.bmap.ESPoUDP = 1;

		if (l3l4_key_p->is_ipv6) {
			blog_p->rx.info.bmap.PLD_IPv6 = 1;
		} else {
			blog_p->rx.info.bmap.PLD_IPv4 = 1;
		}
	}

	/* for packet coming in from SPU, there is no L2 header */
	if ((blog_p->rx.info.phyHdrType == BLOG_SPU_DS) ||
	    (blog_p->rx.info.phyHdrType == BLOG_SPU_US)) {
		blog_p->rx.length = 0;
		blog_p->tuple_offset = blog_p->rx.length;
	}

}

int fpi_ctx_to_blog_spu(fpi_flow_t *flow_p, Blog_t *blog_p, uint8_t ipv6_tun)
{
	fpi_l3l4_key_t *l3l4_key_p = &flow_p->key.l3l4_key;
	fpi_context_t *ctx_p = &flow_p->context;
	uint8_t tos;

	if (blog_p->tx.info.phyHdrType == BLOG_SPU_US) {
		tos = BLOG_IPDSCP2TOS(l3l4_key_p->packet_priority);

		if (ipv6_tun == 0) {
			/* in case ESP (with/without over UDP) with GRE, triple
			 * tunnel cases, GRE will have its outer tunnel info
			 * stored in deltx_tuple.  ESP will use delrx_tuple.
			 */
			blog_p->esptx_tuple_p = &blog_p->delrx_tuple;

			blog_p->esptx_tuple_p->saddr = htonl(ctx_p->src_ip[0]);
			blog_p->esptx_tuple_p->daddr = htonl(ctx_p->dst_ip[0]);
			if (l3l4_key_p->esp_flow_type == ESP_US_UDP) {
				blog_p->esptx_tuple_p->port.source =
						htons(ctx_p->src_port);
				blog_p->esptx_tuple_p->port.dest =
						htons(ctx_p->dst_port);
				blog_p->esp_over_udp_spi =
						htonl(ctx_p->esp_spi);
				blog_p->tx.info.bmap.ESPoUDP = 1;

			} else /* if (l3l4_key_p->esp_flow_type == ESP_US) */ {
				blog_p->esptx_tuple_p->esp_spi =
						htonl(ctx_p->esp_spi);
			}
			blog_p->esptx_tuple_p->tos = tos;
			blog_p->esptx_tuple_p->ttl = 32;
			blog_p->tx.info.bmap.DEL_IPv4 = 1;
		} else {
			int i;

			/* TODO!! Temporarily use delrx_tuple. Per blog_t,
			 * 80 bytes from blog->delrx_tuple should be free to
			 * use in this scenario.
			 */
			blog_p->esp6tx_tuple_p = (BlogTupleV6_t *)
					&blog_p->delrx_tuple;

			blog_p->esp6tx_tuple_p->word0 |= htonl(0x60000000);
			PKT_IPV6_SET_TOS_WORD(blog_p->esp6tx_tuple_p->word0,
					      tos);
			for (i = 0; i < 4; i++) {
				blog_p->esp6tx_tuple_p->saddr.p32[i] =
						htonl(ctx_p->src_ip[i]);
				blog_p->esp6tx_tuple_p->daddr.p32[i] =
						htonl(ctx_p->dst_ip[i]);
			}
			blog_p->esp6tx_tuple_p->tx_hop_limit = 32;

			blog_p->esp6tx_tuple_p->esp_spi = htonl(ctx_p->esp_spi);
			blog_p->tx.info.bmap.DEL_IPv6 = 1;
		}
		blog_p->tx.info.bmap.ESP = 1;
	}

	return 0;
}
